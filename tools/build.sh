#!/bin/sh

git clone --single-branch -b latest-binary https://github.com/limine-bootloader/limine
git clone https://github.com/managarm/cxxshim.git

PREFIX="$(pwd)"
TARGET=x86_64-elf

BINUTILSVERSION=2.35
GCCVERSION=10.2.0

export PATH="$PREFIX/bin:$PATH"

if [ ! -f binutils-$BINUTILSVERSION.tar.gz ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILSVERSION.tar.gz
fi

if [ ! -f gcc-$GCCVERSION.tar.gz ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCCVERSION/gcc-$GCCVERSION.tar.gz
fi

rm -rf build
mkdir build
cd build

tar -xf ../binutils-$BINUTILSVERSION.tar.gz
tar -xf ../gcc-$GCCVERSION.tar.gz

mkdir build-binutils
cd build-binutils
../binutils-$BINUTILSVERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror --enable-64-bit-bfd
make -j"$(nproc)"
make install -j"$(nproc)"
cd ..

mkdir build-gcc
cd build-gcc
../gcc-$GCCVERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c++ --without-headers
make all-gcc -j"$(nproc)"
make -j"$(nproc)" all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone' || true
sed -i 's/PICFLAG/DISABLED_PICFLAG/g' $TARGET/libgcc/Makefile
make -j"$(nproc)" all-target-libgcc CFLAGS_FOR_TARGET='-g -O2 -mcmodel=kernel -mno-red-zone'
make install-gcc -j"$(nproc)"
make install-target-libgcc -j"$(nproc)"

cd ..
