/*
 * Copyright (c) 2008, 2022, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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

#include "jni.h"
#include "jni_util.h"
#include "jvm.h"
#include "jlong.h"

#include <sys/param.h>
#include <sys/mount.h>
#ifdef __QNX__
#include <sys/types.h>
#endif
#ifdef ST_RDONLY
#define statfs statvfs
#define getfsstat getvfsstat
#define f_flags f_flag
#define ISREADONLY ST_RDONLY
#else
#define ISREADONLY MNT_RDONLY
#endif

#include <stdlib.h>
#include <string.h>

static jfieldID entry_name;
static jfieldID entry_dir;
static jfieldID entry_fstype;
static jfieldID entry_options;

struct fsstat_iter {
    struct statfs *buf;
    int pos;
    int nentries;
};

#include "sun_nio_fs_QnxNativeDispatcher.h"


/**
 * Initialize jfieldIDs
 */
JNIEXPORT void JNICALL
Java_sun_nio_fs_BsdNativeDispatcher_initIDs(JNIEnv* env, jclass this)
{
    jclass clazz;

    clazz = (*env)->FindClass(env, "sun/nio/fs/UnixMountEntry");
    CHECK_NULL(clazz);
    entry_name = (*env)->GetFieldID(env, clazz, "name", "[B");
    CHECK_NULL(entry_name);
    entry_dir = (*env)->GetFieldID(env, clazz, "dir", "[B");
    CHECK_NULL(entry_dir);
    entry_fstype = (*env)->GetFieldID(env, clazz, "fstype", "[B");
    CHECK_NULL(entry_fstype);
    entry_options = (*env)->GetFieldID(env, clazz, "opts", "[B");
    CHECK_NULL(entry_options);
}
