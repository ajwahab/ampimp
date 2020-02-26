

# Description  
- Custom application for the [EVAL-AD5940ELCZ](https://wiki.analog.com/resources/eval/user-guides/eval-ad5940/hardware/eval-ad5940elcz).  
- Automatically cycles between amperometric measurements and impedance measurements.  
- Python application provided for logging of measurements and control.  

----
# Installation  
> **Note:** Instructions assume that the working copy of the `ampimp` repository will be located in `~/repos/ampimp`.  

- If `~/repos` does not exist, then create it:  
`$ mkdir ~/repos`
- Navigate to directory that will store repository working copies:  
`$ cd ~/repos`  
- Clone `ampimp` repository:  
`$ git clone --depth 1 --recurse-submodules git@github.com:ninfinity/ampimp.git`  

## Docker container  
Automated setup of portable, self-contained image that includes necessary tools and source code.  
**INCOMPLETE**  
- Starting from ampimp root directory:  
`$ cd docker`  

----
# Usage  
> **Note:** Instructions assume that the root directory of the working copy of the `ampimp` repository is located in `~/repos/ampimp`.  

## Serial terminal  
- From Linux terminal:  
`$ screen -L /dev/ttyS0 230400 -L`  
- Commands:  
    - `help`: print supported commands  
    - `start`: start selected application  
    - `stop`: stop selected application  
    - `switch <appid>`: stop current app and switch to new application set by `<appid>`  
        `<appid>`: 0 = amperometric, 1 = impedance  

## Python app  
Prerequisites:  
- [Python 3](https://www.python.org)  
- [pySerial](https://pyserial.readthedocs.io/en/latest/pyserial.html)  

- Starting from `ampimp` root directory:  
`$ cd py`  
`$ pipenv install`  
`$ pipenv shell`  
`$ python3 ampimp_app.py`  
- Ctrl+c terminates the app and stops the device.  

----
# Development  
> **Note:** Instructions assume the following:  
> - The root directory of the working copy of the `ampimp` repository is located in `~/repos/ampimp`.  
> - The user's SSH public key has been [added](https://help.github.com/en/github/authenticating-to-github/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent) to their GitHub account.  

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

#### OpenOCD  
- Install prerequisites:  
`$ sudo apt-get install make libtool pkg-config autoconf automake texinfo libusb-dev libftdi-dev libhidapi-dev`  
- Navigate to `repos` directory:  
`$ cd ~/repos`  
- Clone OpenOCD:  
`$ git clone --depth 1 --recurse-submodules git@github.com:ntfreak/openocd.git`  
`$ cd openocd`  
- Apply patch provided by `ampimp` to add support for ADuCM302x microcontrollers:  
`$ git apply ~repos/ampimp/openocd/aducm302x-200226.patch`  
- Proceed to configure, build, and install of OpenOCD:  
`$ ./bootstrap`  
`$ ./configure`  
`$ make`  
`$ sudo make install`  
- Working copy of OpenOCD is no longer needed, so remove it:  
`$ cd ..`  
`$ rm -rf ./openocd`  

## Debugging  
OpenOCD + GDB may be used for debugging via the CMSIS-DAP interface provided by the ADICUP3029.  

1. In terminal:  
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

1. From another terminal window:  
`$ screen -L /dev/ttyS0 230400 -L`  
Useful `screen` commands:  
`Ctrl+a k`: kill screen session  
`Ctrl+a d`: detach from screen session  
`$ screen -ls`: list existing screen sessions  
`$ screen -r`: reattach to existing screen session  

