#include "com_sun_management_internal_OperatingSystemImpl.h"
#include "management_ext.h"

#include <unistd.h>

#include "jvm.h"

#if !defined(MB)
#define MB      (1024UL * 1024UL)
#endif

JNIEXPORT jdouble JNICALL
Java_com_sun_management_internal_OperatingSystemImpl_getCpuLoad0
(JNIEnv *env, jobject dummy)
{
        return -1.0;
}

JNIEXPORT jdouble JNICALL
Java_com_sun_management_internal_OperatingSystemImpl_getProcessCpuLoad0
(JNIEnv *env, jobject dummy)
{
	return -1.0;
}


JNIEXPORT jdouble JNICALL
Java_com_sun_management_internal_OperatingSystemImpl_getSingleCpuLoad0
(JNIEnv *env, jobject dummy, jint cpu_number)
{
    // Not implemented yet
    return -1.0;
}

JNIEXPORT jlong JNICALL
Java_com_sun_management_internal_OperatingSystemImpl_getHostTotalCpuTicks0
(JNIEnv *env, jobject mbean)
{
    return -1.0;
}

JNIEXPORT jint JNICALL
Java_com_sun_management_internal_OperatingSystemImpl_getHostConfiguredCpuCount0
(JNIEnv *env, jobject mbean)
{
    return JVM_ActiveProcessorCount();
}

JNIEXPORT jlong JNICALL
Java_com_sun_management_internal_OperatingSystemImpl_getCommittedVirtualMemorySize0
  (JNIEnv *env, jobject mbean)
{
// TODO: for now just return some "sain" value
    return (64 * MB);	
}
