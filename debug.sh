#!/bin/bash

SCRIPT_PATH="$( cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 ; pwd -P )"

pushd ${SCRIPT_PATH}

./ocd.sh &

sleep 2

pid=$(ps -ef | grep "[o]penocd" | awk '{print $2}')
echo "openocd is running with pid: $pid"

arm-none-eabi-gdb -se ../build/*.elf -x ${SCRIPT_PATH}/gdb_cmd.txt

kill $pid

popd
