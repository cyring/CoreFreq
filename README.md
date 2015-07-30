# CoreFreq
## Purpose
CoreFreq is made for the Intel 64-bits Processor, architectures Atom, Core2, Nehalem, SandyBridge and above.

## Build & Run
 1- Download or clone the source code into a working directory.
 
 2- Build the programs.
```
make
```

```
cc -c corefreqd.c -o corefreqd.o
cc -lpthread -lrt -o corefreqd corefreqd.c
cc -c corefreq-cli.c -o corefreq-cli.o
cc -lrt -o corefreq-cli corefreq-cli.c
make -C /lib/modules/4.1.2-2-ARCH/build M=/workdir/CoreFreq modules
make[1]: Entering directory '/usr/lib/modules/4.1.2-2-ARCH/build'
  CC [M]  /workdir/CoreFreq/intelfreq.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /workdir/CoreFreq/intelfreq.mod.o
  LD [M]  /workdir/CoreFreq/intelfreq.ko
make[1]: Leaving directory '/usr/lib/modules/4.1.2-2-ARCH/build'
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
 5- Start the client, as user.
```
./corefreq-cli
```

### Stop

 6- Press [CTRL]+[C] to stop the client.

 7- Press [CTRL]+[C] to stop the daemon.

 8- Unload the kernel module with the ```rmmod``` command
```
rmmod intelfreq.ko
```

## Screenshots
 * Use ```dmesg``` or ```journalctl -k``` to check if the driver is started
```
kernel: IntelFreq [Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz]
        Signature [06_1A] Architecture [Nehalem/Bloomfield]
        8/8 CPU Online , Clock @ {146/1027545} MHz
```
```
CoreFreqd [Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz] , Clock @ 146.513772 MHz
```

![alt text](http://blog.cyring.free.fr/images/CoreFreq.png "CoreFreq")

## Algorithm
![alt text](http://blog.cyring.free.fr/images/CoreFreq-algorithm.png "CoreFreq algorithm")

# Regards
_`CyrIng`_
