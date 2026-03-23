/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/*
 * Test if a file exists in the file system cache
 */
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#include "jni.h"


// Qnx does not have a mincore impl this is copied from os_qnx.cpp to satisfy the test
// KEEP IN SYNC
#ifdef __QNX__
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/procfs.h>
#include <devctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define CHUNK_SIZE 512 

int mincore(void *addr, size_t length, unsigned char *vec) {
    int fd;
    uintptr_t current_vaddr = (uintptr_t)addr;
    long page_size = sysconf(_SC_PAGESIZE);
    size_t total_pages = (length + page_size - 1) / page_size;

    // Make sure we are aligned to page_size
    if (current_vaddr % page_size != 0) {
        errno = EINVAL;
        return -1;
    }

    fd = open("/proc/self/as", O_RDONLY);
    if (fd == -1) return -1;

    procfs_mapinfo states[CHUNK_SIZE];
    size_t pages_processed = 0;

    while (pages_processed < total_pages) {
        size_t pages_to_query = total_pages - pages_processed;
        if (pages_to_query > CHUNK_SIZE) {
            pages_to_query = CHUNK_SIZE;
        }

        memset(states, 0, sizeof(states));
        // DCMD_PROC_PTINFO the first entry vaddr is used to denote the start
        states[0].vaddr = current_vaddr;

        int status = devctl(fd, DCMD_PROC_PTINFO, states,
                            sizeof(procfs_mapinfo) * pages_to_query, NULL);

        if (status != EOK) {
            close(fd);
            // If the range isn't fully mapped, mincore returns ENOMEM
            errno = (status == ENOENT) ? ENOMEM : status;
            return -1;
        }

        // Transfer flags to the output vector
        for (size_t i = 0; i < pages_to_query; i++) {
            vec[pages_processed + i] = (states[i].flags & PG_HWMAPPED) ? 1 : 0;
        }

        pages_processed += pages_to_query;
        current_vaddr += (pages_to_query * page_size);
    }

    close(fd);
    return 0;
}
#endif /* __QNX__ */
/*
 * Throws an exception with the given class name and detail message
 */
static void ThrowException(JNIEnv *env, const char *name, const char *msg) {
    jclass cls = (*env)->FindClass(env, name);
    if (cls != NULL) {
        (*env)->ThrowNew(env, cls, msg);
    }
}

/*
 * Class:     DirectIO
 * Method:    flushFileCache
 * Signature: (II;)V
 */
JNIEXPORT void Java_DirectIOTest_flushFileCache(JNIEnv *env,
                                                jclass cls,
                                                jint file_size,
                                                jint fd) {
#if defined(__linux__) || defined(__QNX__)
    posix_fadvise(fd, 0, file_size, POSIX_FADV_DONTNEED);
#endif
}

/*
 * Class:     DirectIO
 * Method:    isFileInCache
 * Signature: (II;)Z
 */
JNIEXPORT jboolean Java_DirectIOTest_isFileInCache(JNIEnv *env,
                                                jclass cls,
                                                jint file_size,
                                                jint fd) {
    void *f_mmap;
#ifdef __linux__
    unsigned char *f_seg;
#else
    char *f_seg;
#endif

#ifdef __APPLE__
    size_t mask = MINCORE_INCORE;
#else
    size_t mask = 0x1;
#endif

    size_t page_size = getpagesize();
    size_t index = (file_size + page_size - 1) /page_size;
    jboolean result = JNI_FALSE;

    f_mmap = mmap(0, file_size, PROT_NONE, MAP_SHARED, fd, 0);
    if (f_mmap == MAP_FAILED) {
        ThrowException(env, "java/io/IOException",
            "test of whether file exists in cache failed: mmap failed");
    }
    f_seg = malloc(index);
    if (f_seg != NULL) {
        if(mincore(f_mmap, file_size, f_seg) == 0) {
            size_t i;
            for (i = 0; i < index; i++) {
                if (f_seg[i] & mask) {
                    result = JNI_TRUE;
                    break;
                }
            }
        }
        free(f_seg);
    } else {
        ThrowException(env, "java/io/IOException",
            "test of whether file exists in cache failed: malloc failed");
    }
    munmap(f_mmap, file_size);
    return result;
}
