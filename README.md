
# Description

----
# Prerequisites
- `git`
- GNU ARM toolchain
- `cmake`

# Development  
## Building source
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

### gdbgui
A browser-based [GUI](https://www.gdbgui.com) for GDB.  

## Debugging  

OpenOCD + GDB may be used for debugging via the CMSIS-DAP interface provided by the ADICUP3029.  

1. In a terminal window:  
`$ ./ocd.sh`  

1. In a new terminal window:  
`$ arm-none-eabi-gdb`  
`(gdb) target remote localhost:3333`  # connect to openocd instance  
`(gdb) file build/ad5940_app.elf`  # specify binary with debug symbols  
`(gdb) load`  # flash the target device  
`(gdb) monitor reset halt`  # reset and halt target  
`(gdb) step`  # execute next instruction, step into function calls  
`(gdb) next`  # execute next instruction, step over function calls  
`(gdb) continue`  
`(gdb) Ctrl+c`  # interrupt execution  
`(gdb) list`  # display several lines of code at debugger position, repeated calls show more lines  
`(gdb) break <function>`  # set a breakpoint at the specified function within the current source file  
`(gdb) b <filename>:<function>`  # set a breakpoint at the specified function within the specified source file  
`(gdb) b <line number>`  # set breakpoint at line within current source file  
`(gdb) quit`  # exit GDB
The GDB Text User Interface (TUI) mode may be enabled when invoking GDB as:  
`$ arm-none-eabi-gdb -tui`  
...or from the CLI within an existing instance of GDB:  
`(gdb) tui`  
If installed, gdbgui may be used instead of the default CLI:  
`$ gdbgui -g arm-none-eabi-gdb`  

1. In yet another terminal window:  
`$ screen -L /dev/cu.usbmodem14502 230400 -L`  
`Ctrl+a,k` to kill screen session.  
