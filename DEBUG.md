
----  
In a new terminal window:  
`$ arm-none-eabi-gdb`  
`(gdb) target remote localhost:3333`  
`(gdb) ad5940_app.elf`  

----  
In a new terminal window:  
`$ screen -L /dev/cu.usbmodem14502 230400 -L`  
`CTRL+a,k` to kill screen session.