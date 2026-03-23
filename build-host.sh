#!/bin/bash


./configure \
  --prefix=/usr/local/lib/jvm/java-25-openjdk \
	--with-boot-jdk=../boot-jdk \
	--with-sysroot="$_sysroot" \
	--with-extra-cflags="-D_LARGEFILE64_SOURCE" \
	--with-extra-cxxflags="-D_LARGEFILE64_SOURCE" \
	--with-jobs=$(nproc) \
	--with-test-jobs=$(nproc) \
	--disable-warnings-as-errors \
	--disable-precompiled-headers \
	--enable-dtrace=no \
	--with-debug-level=release \
	--with-native-debug-symbols=none



