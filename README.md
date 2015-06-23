# CoreFreq
## Purpose
CoreFreq is made for the Intel 64-bits Processor, architecture Nehalem and above.

## Build & Run
 1- Download or clone the source code into a working directory.
 
 2- Build the programs.
```
make
```

```
cc -c corefreqd.c -o corefreqd.o
cc -lpthread -o corefreqd corefreqd.c
make -C /lib/modules/4.0.5-1-ARCH/build M=/home/anyuser/src/CoreFreq modules
make[1]: Entering directory '/usr/lib/modules/4.0.5-1-ARCH/build'
  CC [M]  /home/anyuser/src/CoreFreq/intelfreq.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/anyuser/src/CoreFreq/intelfreq.mod.o
  LD [M]  /home/anyuser/src/CoreFreq/intelfreq.ko
make[1]: Leaving directory '/usr/lib/modules/4.0.5-1-ARCH/build'
```

### Start

 3- Load the Kernel module, as root.
```
insmod intelfreq.ko
```
 4- Start the daemon, as root.
```
./corefreqd
```

### Stop

 5- Press [CTRL]+[C] to stop the daemon.

 6- Unload the kernel module with command
```
rmmod intelfreq.ko
```

## Screenshots
 * Use ```dmesg``` to check if the driver is started
```
IntelFreq [Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz] [8 x CPU]
[Clock @ 146 MHz]  Ratio={12,20,0,0,0,0,21,21,21,22}
```

![alt text](http://blog.cyring.free.fr/images/CoreFreq.png "CoreFreq")

# Regards
_`CyrIng`_
