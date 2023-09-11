#!/bin/bash

set -o errexit
set -o nounset

TARGET=pdp11-aout
BINUTILS=binutils-2.41
GCC=gcc-13.2.0

HERE="$(dirname -- "$(readlink -f -- "$BASH_SOURCE")")"

PREFIX="${HERE}/cross-out"
mkdir -p "${PREFIX}"/bin
PATH="${PREFIX}/bin:${PATH}"

binutils() {
    mkdir -p "build-${BINUTILS}"
    cd "build-${BINUTILS}"
    "../${BINUTILS}/configure" \
	--prefix="${PREFIX}" \
	--target="${TARGET}" \
	--with-sysroot \
	--disable-nls \
	--disable-werror
    make -j
    make install
    cd ..
}

gcc() {
    mkdir -p "build-${GCC}"
    cd "build-${GCC}"
    "../${GCC}/configure" \
	--prefix="${PREFIX}" \
	--target="${TARGET}" \
	--disable-nls \
	--enable-languages=c \
	--without-headers
    make -j2 all-gcc
    make -j2 all-target-libgcc
    make install-gcc
    make install-target-libgcc
}

binutils
gcc





