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
 * Use ```dmesg``` or ```journalctl -kf``` to check if the driver is started
```
kernel: IntelFreq [Intel(R) Core(TM) i7 CPU 920 @ 2.67GHz]
        Signature [06_1A] Architecture [Nehalem/Bloomfield]
        8/8 CPU Online , Clock @ {146/2939877074} MHz
        Ratio={12,20,0,0,0,0,21,21,21,22}
kernel: Topology(0) Apic[  0] Core[  0] Thread[  0]
kernel: Topology(1) Apic[  2] Core[  1] Thread[  0]
kernel: Topology(2) Apic[  4] Core[  2] Thread[  0]
kernel: Topology(3) Apic[  6] Core[  3] Thread[  0]
kernel: Topology(4) Apic[  1] Core[  0] Thread[  1]
kernel: Topology(5) Apic[  3] Core[  1] Thread[  1]
kernel: Topology(6) Apic[  5] Core[  2] Thread[  1]
kernel: Topology(7) Apic[  7] Core[  3] Thread[  1]
```

![alt text](http://blog.cyring.free.fr/images/CoreFreq.png "CoreFreq")

# Regards
_`CyrIng`_
