In a terminal window:  
`$ ./ocd.sh`  

----  

In a new terminal window:  
`$ arm-none-eabi-gdb`  
`(gdb) target remote localhost:3333`  
`(gdb) file build/ad5940_app.elf`  
`(gdb) load`  
`(gdb) monitor reset halt`  
`(gdb) step`  
`(gdb) continue`  
`(gdb) Ctrl+c`  
`(gdb) list`  
`(gdb) break <function>`  
`(gdb) break <filename>:<function>`  
`(gdb) break <line number>`  

----  

In a new terminal window:  
`$ screen -L /dev/cu.usbmodem14502 230400 -L`  
`CTRL+a,k` to kill screen session.
