

# Description  
- Custom application for the [EVAL-AD5940ELCZ](https://wiki.analog.com/resources/eval/user-guides/eval-ad5940/hardware/eval-ad5940elcz).  
- Automatically cycles between amperometric measurements and impedance measurements.  
- Python application provided for logging of measurements and control.  
----
# Installation  
## Docker container  

----
# Usage  
## Serial terminal  
- From Linux terminal: `$ screen -L /dev/cu.usbmodem14502 230400 -L`  
- Commands:  
    - `help`: print supported commands  
    - `start`: start selected application  
    - `stop`: stop selected application  
    - `switch <appid>`: stop current app and switch to new application set by `<appid>`  
        `<appid>`: 0 = amperometric, 1 = impedance  

## Python  
Prerequisites: 
- [Python 3](https://www.python.org)  
- [pySerial](https://pyserial.readthedocs.io/en/latest/pyserial.html)  

`$ cd py`  
`$ pipenv install`
`$ pipenv shell`  
`$ python3 ampimp_app.py`  
Ctrl+c terminates the app and stops the device.  

----
# Development  
## Building source  
Prerequisites:  
- [Git](https://git-scm.com)  
- [GNU ARM toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)  
- [CMake](https://cmake.org)  

`$ mkdir ad5940`  
`$ cd ad5940`  
`$ git clone --recurse-submodules git@github.com:ninfinity/ad5940.git`  
`$ cd ad5940/ad5940`  
`$ ./build.sh`  

### Optional  
#### OpenOCD  
`sudo apt-get install make libtool pkg-config autoconf automake texinfo libusb-dev libftdi-dev libhidapi-dev`  
We'll use a special fork of OpenOCD with support for ADuCM302x...  
`$ git clone --recurse-submodules git@github.com:ninfinity/openocd.git`  
`$ cd openocd`  
`$ ./bootstrap`  
`$ ./configure`  
`$ make`  
`$ sudo make install`  
`$ cd ..`  
`$ rm -rf ./openocd`  

## Debugging  
OpenOCD + GDB may be used for debugging via the CMSIS-DAP interface provided by the ADICUP3029.  

1. In a terminal window:  
`$ ./debug.sh`  
`(gdb) c`    # continue execution of code  
Useful commands:  
`(gdb) step`  # execute next instruction, step into function calls  
`(gdb) next`  # execute next instruction, step over function calls  
`(gdb) continue`  # continue running code  
`(gdb) load`  # flash the target device; useful for updating device without disconnecting after building new firmware
`(gdb) Ctrl+c`  # interrupt execution  
`(gdb) list`  # display several lines of code at debugger position, repeated calls show more lines  
`(gdb) break <function>`  # set a breakpoint at the specified function within the current source file  
`(gdb) b <filename>:<function>`  # set a breakpoint at the specified function within the specified source file  
`(gdb) b <line number>`  # set breakpoint at line within current source file  
`(gdb) tui`  #launch the Text User Interface
`(gdb) q`  # exit GDB  

1. (optional) In another terminal window:  
`$ screen -L /dev/cu.usbmodem14502 230400 -L`  
Useful `screen` commands:  
`Ctrl+a k`: kill screen session  
`Ctrl+a d`: detach from screen session  
`$ screen -ls`: list existing screen sessions  
`$ screen -r`: reattach to existing screen session  

