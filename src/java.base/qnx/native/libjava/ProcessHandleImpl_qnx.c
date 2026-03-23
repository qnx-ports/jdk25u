#include "jni.h"
#include "jni_util.h"
#include "java_lang_ProcessHandleImpl.h"
#include "java_lang_ProcessHandleImpl_Info.h"

#include "ProcessHandleImpl_unix.h"

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/debug.h>
#include <sys/procfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

static int get_pid_info(pid_t pid, procfs_info* info) {
  int fd;
  char fn[PATH_MAX];

  snprintf(fn, sizeof fn, "/proc/%d/as", pid);

  fd = open(fn, O_RDONLY);
  if (fd == NOFD) {
    return -1;              // fail, no such /proc/pid/as
  }

  errno = devctl(fd, DCMD_PROC_INFO, info, sizeof info, 0);
  close(fd);
  if (errno != EOK) {
    return -1;
  }

  return 1;
}


/**
 * Implementation of native ProcessHandleImpl functions for BSD's.
 * See ProcessHandleImpl_unix.c for more details.
 */

void os_initNative(JNIEnv *env, jclass clazz) {}

/*
 * Return pids of active processes, and optionally parent pids and
 * start times for each process.
 * For a specific non-zero pid jpid, only the direct children are returned.
 * If the pid jpid is zero, all active processes are returned.
 * Uses sysctl to accumulates any process following the rules above.
 * The resulting pids are stored into an array of longs named jarray.
 * The number of pids is returned if they all fit.
 * If the parentArray is non-null, store also the parent pid.
 * In this case the parentArray must have the same length as the result pid array.
 * Of course in the case of a given non-zero pid all entries in the parentArray
 * will contain this pid, so this array does only make sense in the case of a given
 * zero pid.
 * If the jstimesArray is non-null, store also the start time of the pid.
 * In this case the jstimesArray must have the same length as the result pid array.
 * If the array(s) (is|are) too short, excess pids are not stored and
 * the desired length is returned.
 */
jint os_getChildren(JNIEnv *env, jlong jpid, jlongArray jarray,
                    jlongArray jparentArray, jlongArray jstimesArray) {
  DIR* dir;
  struct dirent* ptr;
  pid_t pid = (pid_t) jpid;
    jlong* pids = NULL;
    jlong* ppids = NULL;
    jlong* stimes = NULL;
    jsize parentArraySize = 0;
    jsize arraySize = 0;
    jsize stimesSize = 0;
    jsize count = 0;

    // Boiler plate same as FreeBsd and Linux
    arraySize = (*env)->GetArrayLength(env, jarray);
    JNU_CHECK_EXCEPTION_RETURN(env, -1);
    if (jparentArray != NULL) {
        parentArraySize = (*env)->GetArrayLength(env, jparentArray);
        JNU_CHECK_EXCEPTION_RETURN(env, -1);

        if (arraySize != parentArraySize) {
            JNU_ThrowIllegalArgumentException(env, "array sizes not equal");
            return 0;
        }
    }
    if (jstimesArray != NULL) {
        stimesSize = (*env)->GetArrayLength(env, jstimesArray);
        JNU_CHECK_EXCEPTION_RETURN(env, -1);

        if (arraySize != stimesSize) {
            JNU_ThrowIllegalArgumentException(env, "array sizes not equal");
            return 0;
        }
    }
    if ((dir = opendir("/proc")) == NULL) {
        JNU_ThrowByNameWithMessageAndLastError(env,
            "java/lang/RuntimeException", "Unable to open /proc");
        return -1;
    }
    do { // Block to break out of on Exception
        pids = (*env)->GetLongArrayElements(env, jarray, NULL);
        if (pids == NULL) {
            break;
        }
        if (jparentArray != NULL) {
            ppids  = (*env)->GetLongArrayElements(env, jparentArray, NULL);
            if (ppids == NULL) {
                break;
            }
        }
        if (jstimesArray != NULL) {
            stimes  = (*env)->GetLongArrayElements(env, jstimesArray, NULL);
            if (stimes == NULL) {
                break;
            }
        }
        // TODO: add a happy path for when pid != 0 can be much faster than opening the whole /proc
        while ((ptr = readdir(dir)) != NULL) {
            pid_t ppid = 0;
            jlong totalTime = 0L;
            jlong startTime = 0L;

            /* skip files that aren't numbers */
            pid_t childpid = (pid_t) atoi(ptr->d_name);
            if ((int) childpid <= 0) {
                continue;
            }

            // Get the parent pid, and start time
            ppid = os_getParentPidAndTimings(env, childpid, &totalTime, &startTime);
            if (ppid >= 0 && (pid == 0 || ppid == pid)) {
                if (count < arraySize) {
                    // Only store if it fits
                    pids[count] = (jlong) childpid;

                    if (ppids != NULL) {
                        // Store the parentPid
                        ppids[count] = (jlong) ppid;
                    }
                    if (stimes != NULL) {
                        // Store the process start time
                        stimes[count] = startTime;
                    }
                }
                count++; // Count to tabulate size needed
            }
        }
            
            
    } while(0);

     if (pids != NULL) {
        (*env)->ReleaseLongArrayElements(env, jarray, pids, 0);
    }
    if (ppids != NULL) {
        (*env)->ReleaseLongArrayElements(env, jparentArray, ppids, 0);
    }
    if (stimes != NULL) {
        (*env)->ReleaseLongArrayElements(env, jstimesArray, stimes, 0);
    }

    closedir(dir);
    // If more pids than array had size for; count will be greater than array size
    return count;

}
/**
 * devctl /proc/<pid>/as and return the parent pid, total cputime and start time.
 * -1 is fail;  >=  0 is parent pid
 * 'total' will contain the running time of 'pid' in nanoseconds.
 * 'start' will contain the start time of 'pid' in milliseconds since epoch.
 */
pid_t os_getParentPidAndTimings(JNIEnv *env, pid_t pid,
                                jlong *totalTime, jlong* startTime) {
  procfs_info info; 

  if (get_pid_info(pid, &info) < 0) {
    return -1;
  }

  *totalTime = info.utime + info.stime;
  *startTime = info.start_time;

  return info.parent;
}

void os_getCmdlineAndUserInfo(JNIEnv *env, jobject jinfo, pid_t pid) {
  int fd;
  procfs_info info;
  int cmdlen = 0;
  char *cmdline = NULL, *cmdEnd = NULL; // used for command line args and exe
  char *args = NULL;
  jstring cmdexe = NULL;
  if (get_pid_info(pid, &info) < 0) {
    return;
  }

   unix_getUserInfo(env, jinfo, info.uid);
   JNU_CHECK_EXCEPTION(env);

  struct {
		procfs_debuginfo        info;
		char                    buff[PATH_MAX];
	} name;

  snprintf(name.buff, sizeof(name.buff), "/proc/%u/ctl", pid);
  fd = open(name.buff, O_RDONLY);

  if (fd == NOFD) {
    return;              // fail, no such /proc/pid/ctl
  }

  do {
    int i = 0;
    int count;
    char *s;
    long argmax = sysconf( _SC_ARG_MAX );

    cmdline = (char*)malloc(argmax + 1);

    if (cmdline == NULL) {
      break;
    }

    strcpy(cmdline, name.info.path);
    // i think at this point cmdline is null terminataed?
    cmdexe = JNU_NewStringPlatform(env, cmdline);
    (*env)->ExceptionClear(env);        // unconditionally clear any exception

    cmdlen = 0;
    s = cmdline;
    while ((count = read(fd, s, argmax - cmdlen)) > 0) {
      cmdlen += count;
      s += count;
    }
    if (count < 0) {
      break;
    }
    // We have to null-terminate because the process may have changed argv[]
    cmdline[cmdlen] = '\0';
    
    // Count the arguments
    cmdEnd = &cmdline[cmdlen];
    for (s = cmdline; *s != '\0' && (s < cmdEnd); i++) {
        s += strnlen(s, (cmdEnd - s)) + 1;
    }
    unix_fillArgArray(env, jinfo, i, cmdline, cmdEnd, cmdexe, args);
    
  } while (0);

  if (cmdline != NULL) {
    free(cmdline);
  }
  if (args != NULL) {
    free(args);
  }
  if (fd >= 0) {
    close(fd);
  }
}
