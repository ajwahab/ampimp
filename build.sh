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
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake          \
      -DTOOLCHAIN_PREFIX=/usr/local/bin     \
        ..

# Build
make

# Resulting binary can be flashed using Segger Ozone, openocd, etc.
