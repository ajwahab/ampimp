# ad5940  
`$ mkdir ad5940`..
`$ cd ad5940`..
`$ git clone git@github.com:ninfinity/ad5940.git`  
----  
# OpenOCD  
Prerequisites:
`sudo apt-get install make libtool pkg-config autoconf automake texinfo libusb-dev libftdi-dev libhidapi-dev`
Special build with support for ADuCM302x...  
`$ git clone git@github.com:ninfinity/openocd.git`  
`$ cd openocd`  
`$ ./bootstrap`  
`$ ./configure`  
`$ make`  
`$ sudo make install`  
`$ cd ..`  
`$ rm -rf ./openocd`  
----  