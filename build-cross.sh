#!/bin/bash

_sysroot=$(realpath ../sysroot)

PKG_CONFIG_PATH=$(realpath $_sysroot/lib/pkgconfig) \
QNX_TARGET="$_sysroot" \
CC="ntoaarch64-gcc" \
CXX="ntoaarch64-c++" \
CPP="ntoaarch64-cpp" \
PKG_CONFIG_SYSROOT_DIR=$_sysroot \
./configure \
  --prefix=/usr/local/lib/jvm/java-25-openjdk \
	--openjdk-target=aarch64-unknown-qnx \
	--with-boot-jdk=../boot-jdk \
  --with-build-jdk=./build/linux-aarch64-server-release/images/jdk \
	--with-sysroot="$_sysroot" \
	--with-extra-cflags="-D_QNX_SOURCE -I$_sysroot/usr/include -I$_sysroot/usr/include/shims -D_LARGEFILE64_SOURCE" \
	--with-extra-cxxflags="-D_QNX_SOURCE -I$_sysroot/usr/include/c++/v1  -I$_sysroot/usr/include -D_LARGEFILE64_SOURCE" \
	--with-extra-ldflags="-L$_sysroot/usr/lib -shared-libgcc -L$_sysroot/usr/lib -lsocket -lepoll -leventfd -lsysv-ipc -lasound" \
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
	--with-native-debug-symbols=none



