#!/bin/bash

SCRIPT_PATH="$( cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 ; pwd -P )"

if [ -d ${SCRIPT_PATH}/../build ]
then
    rm -rf ${SCRIPT_PATH}/../build
fi

mkdir ${SCRIPT_PATH}/../build

pushd ${SCRIPT_PATH}/../build

cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake    \
      ${SCRIPT_PATH}
      #-DTOOLCHAIN_PREFIX=/usr/local   \
      #-G "Eclipse CDT4 - Unix Makefiles"    \
      #-DCMAKE_BUILD_TYPE=Debug    \
      #-DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE    \
      #-DCMAKE_ECLIPSE_MAKE_ARGUMENTS=-j8    \

make

popd
