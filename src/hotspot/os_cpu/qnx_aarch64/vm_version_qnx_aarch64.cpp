/*
 * Copyright (c) 2006, 2025, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2014, 2019, Red Hat Inc. All rights reserved.
 * Copyright (c) 2021, Azul Systems, Inc. All rights reserved.
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
 *
 */

#include "runtime/java.hpp"
#include "runtime/os.hpp"
#include "runtime/vm_version.hpp"

#include <sys/sysctl.h>

int VM_Version::get_current_sve_vector_length() {
  ShouldNotCallThis();
  return -1;
}

int VM_Version::set_and_get_current_sve_vector_length(int length) {
  ShouldNotCallThis();
  return -1;
}

void VM_Version::get_os_cpu_info() {
  size_t sysctllen;

  _features = CPU_FP | CPU_ASIMD;

  // All Apple-darwin Arm processors have AES, PMULL, SHA1 and SHA2.
  // See https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/arm/commpage/commpage.c#L412
  // Note that we ought to add assertions to check sysctlbyname parameters for
  // these four CPU features, e.g., "hw.optional.arm.FEAT_AES", but the
  // corresponding string names are not available before xnu-8019 version.
  // Hence, assertions are omitted considering backward compatibility.
  _features |= CPU_AES | CPU_PMULL | CPU_SHA1 | CPU_SHA2;

  // TODO move this over to syspage entry

  uint64_t ctr_el0;
  uint64_t dczid_el0;
  __asm__ (
    "mrs %0, CTR_EL0\n"
    "mrs %1, DCZID_EL0\n"
    : "=r"(ctr_el0), "=r"(dczid_el0)
  );

  _icache_line_size = (1 << (ctr_el0 & 0x0f)) * 4;
  _dcache_line_size = (1 << ((ctr_el0 >> 16) & 0x0f)) * 4;

  if (!(dczid_el0 & 0x10)) {
    _zva_length = 4 << (dczid_el0 & 0xf);
  }

  _cpu = CPU_ARM;
}

void VM_Version::get_compatible_board(char *buf, int buflen) {
  assert(buf != nullptr, "invalid argument");
  assert(buflen >= 1, "invalid argument");
  *buf = '\0';
}

