#!/bin/bash

# Create build directory
if [ -d ../build ]
then
    rm -rf ../build
fi
mkdir ../build
cd ../build

# Configure build environment
# set -DTOOLCHAIN_PREFIX to the install directory of gcc-arm-non-eabi (e.g. /usr/local/gcc-arm-none-eabi-6-2017-q1-update)
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake    \
      -DTOOLCHAIN_PREFIX=/usr/local   \
      -G "Eclipse CDT4 - Unix Makefiles"    \
      -DCMAKE_BUILD_TYPE=Debug    \
      -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE    \
      -DCMAKE_ECLIPSE_MAKE_ARGUMENTS=-j8    \
      ../ad5940

# Build
make

# Resulting binary can be flashed using Segger Ozone, openocd, etc.
