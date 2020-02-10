#!/bin/bash

# Create build directory
if [ -d build ]
then
    rm -rf build
fi
mkdir build
cd build

# Configure build environment
# set -DTOOLCHAIN_PREFIX to the install directory of gcc-arm-non-eabi (e.g. /usr/local/gcc-arm-none-eabi-6-2017-q1-update)
#-DTOOLCHAIN_PREFIX=/Users/awahab/Library/xPacks/@xpack-dev-tools/arm-none-eabi-gcc/9.2.1-1.1.1/.content     \
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake          \
      -DTOOLCHAIN_PREFIX=/usr/local     \
        ..

# Build
make

# Resulting binary can be flashed using Segger Ozone, openocd, etc.
