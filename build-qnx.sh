#!/bin/bash
#--with-extra-ldflags="-lsocket -lepoll -leventfd -lsysv-ipc" \

#LD_LIBRARY_PATH="$(realpath ../jdk/lib):LD_LIBRARY_PATH" PATH="$(realpath ../jdk/bin):$PATH" \


./configure \
        --build=aarch64-unknown-qnx \
  	--prefix=/usr/local/lib/jvm/java-25-openjdk \
	--openjdk-target=aarch64-unknown-qnx \
	--with-boot-jdk=../jdk \
	--with-extra-cflags="-D_LARGEFILE64_SOURCE" \
	--with-extra-cxxflags="-D_LARGEFILE64_SOURCE" \
	--with-extra-ldflags="-lsocket" \
	--with-zlib=system \
	--with-libjpeg=system \
	--with-giflib=system \
	--with-libpng=system \
	--with-lcms=system \
	--with-jobs=$(nproc) \
	--with-test-jobs=$(nproc) \
	--disable-warnings-as-errors \
	--disable-precompiled-headers \
	--enable-dtrace=no \
	--with-debug-level=release \
	--with-native-debug-symbols=none \
	--with-toolchain-type=clang \
	--with-gtest=../googletest-qnx_v1.14.0 \
	--with-jtreg=no



