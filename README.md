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
make -C /lib/modules/4.1.6-1-ARCH/build M=/workdir/CoreFreq modules
make[1]: Entering directory '/usr/lib/modules/4.1.6-1-ARCH/build'
  CC [M]  /workdir/CoreFreq/intelfreq.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /workdir/CoreFreq/intelfreq.mod.o
  LD [M]  /workdir/CoreFreq/intelfreq.ko
make[1]: Leaving directory '/usr/lib/modules/4.1.6-1-ARCH/build'
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
 5- Start the client, as a user.
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
	8/8 CPU Online , Clock @ {146/1200} MHz
```
```
CoreFreqd [Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz] , Clock @ 146.600000 MHz
```

![alt text](http://blog.cyring.free.fr/images/CoreFreq.png "CoreFreq")

 * Run the CoreFreq daemon with the option '-t' to display the Processor topology
```
CoreFreqd [Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz]
Signature [06_1A] Architecture [Nehalem/Bloomfield]
8/8 CPU Online , Clock @ 146.600000 MHz

CPU       ApicID CoreID ThreadID x2APIC Enable Caches L1 L2 L3
#00(BSP)       0      0        0    OFF    Y    32768 4626 262144
#01(AP)        2      1        0    OFF    Y    32768 4626 262144
#02(AP)        4      2        0    OFF    Y    32768 4626 262144
#03(AP)        6      3        0    OFF    Y    32768 4626 262144
#04(AP)        1      0        1    OFF    Y    32768 4626 262144
#05(AP)        3      1        1    OFF    Y    32768 4626 262144
#06(AP)        5      2        1    OFF    Y    32768 4626 262144
#07(AP)        7      3        1    OFF    Y    32768 4626 262144
```

## Algorithm
![alt text](http://blog.cyring.free.fr/images/CoreFreq-algorithm.png "CoreFreq algorithm")

# Regards
_`CyrIng`_
 -------