# _CoreFreq_
## Purpose
_CoreFreq_, a CPU monitoring software with BIOS like functionalities, is designed for the 64-bits Processors of architecture Intel Atom, Core2, Nehalem, SandyBridge and superiors; AMD Families from 0Fh ... up to 17h (Zen , Zen+ , Zen 2), 18h (Hygon Dhyana), 19h (Zen 3, Zen 3+, Zen 4, Zen 4c); Arm A64  

![alt text](http://blog.cyring.free.fr/images/CoreFreq_Top.gif "CoreFreq Top")

_CoreFreq_ provides a framework to retrieve CPU data with a high degree of precision:

* Core frequencies & ratios; SpeedStep (EIST), Turbo Boost, Hyper-Threading (HTT) and Base Clock
* Performance counters including Time Stamp Counter (TSC), Unhalted Core Cycles (UCC), Unhalted Reference Cycles (URC)
* Number of instructions per cycle or second, IPS, IPC, or CPI
* CPU C-States C0 C1 C3 C6 C7 - C1E - Auto/UnDemotion of C1 C3
* DTS Temperature and Tjunction Max, Thermal Monitoring TM1 TM2 state, Vcore
* Topology map including Caches for boostrap & application CPU
* Processor features, brand & architecture strings
* In progress: Uncore, Memory Controller channels & geometry, DIMM timings,  
  Stress tools, Power & Energy (RAPL, P-State, HWP, TDP), Overclocking, cpuidle & cpufreq driver, ClockSource, Mitigation Mechanisms  


To reach this goal, _CoreFreq_ implements a Linux Kernel module which employs the followings:

* asm code to keep as near as possible the readings of the performance counters;
* per-CPU, implements slab data memory and high-resolution timer;
* compliant with suspend / resume and CPU Hot-Plug;
* a shared memory to protect kernel from the user-space part of the software;
* atomic synchronization of threads to avoid mutexes and deadlock.

## Build & Run

### Prerequisites

**a-** _Intel only_: For a better accuracy, *disable* the Kernel *NMI Watchdog*  

Add the below [parameter](https://github.com/torvalds/linux/blob/master/Documentation/admin-guide/kernel-parameters.txt) in the kernel boot loader { [Grub](https://www.gnu.org/software/grub/manual/grub/grub.html#GNU_002fLinux), [SysLinux](https://wiki.syslinux.org/wiki/index.php?title=Config#APPEND) } ...  

```
nmi_watchdog=0
```
... and build with the fixed performance counters  
```
make MSR_CORE_PERF_UC=MSR_CORE_PERF_FIXED_CTR1 MSR_CORE_PERF_URC=MSR_CORE_PERF_FIXED_CTR2
```

**b-** _AMD and Intel_: Some Virtualization  

VMs don't provide access to the registers that the _CoreFreq_ driver employs :  
* Fixed Performance Counters 
* Model Specific Registers 
* PCI Registers 

However _CoreFreq_ is making use of the virtualized performance counter :  
* HV_X64_MSR_VP_RUNTIME(0x40000010) 

**c-** Rendering  

The UI renders best with an ASCII console or a Xterm with VT100 support, ANSI **colors**; optionally transparency.  
If **bold** and **bright** colors are not rendered then use the following terminal options:  
#### Ubuntu Terminal
In the Preferences - Colors tab, select `Show bold text in bright colors`  
#### alacritty terminal
Uncomment and set `draw_bold_text_with_bright_colors: true` in `<config-file>`  

### Dependencies
* The Linux Kernel with a minimum version 3.3
* The GNU C Library

### Build
1. Software needed:  
* GNU C Compiler with GNU extensions
* GNU Make tool
* Linux Kernel Header files to build modules
  * Mandatory : `CONFIG_MODULES, CONFIG_SMP, CONFIG_X86_MSR`
  * Optionally: `CONFIG_HOTPLUG_CPU, CONFIG_CPU_IDLE, CONFIG_CPU_FREQ, CONFIG_PM_SLEEP, CONFIG_DMI, CONFIG_HAVE_NMI, CONFIG_XEN, CONFIG_AMD_NB, CONFIG_SCHED_MUQSS, CONFIG_SCHED_BMQ, CONFIG_SCHED_PDS, CONFIG_SCHED_ALT, CONFIG_SCHED_BORE, CONFIG_CACHY, CONFIG_ACPI, CONFIG_ACPI_CPPC_LIB`
  * Forbidden : `CONFIG_TRIM_UNUSED_KSYMS`

2. Clone the source code into a working directory.  
```sh
git clone https://github.com/cyring/CoreFreq.git
```

3. Build the programs.  
```sh
cd CoreFreq
make -j
```
```console
cc  -Wall -Wfatal-errors -Wno-unused-variable -pthread -c x86_64/corefreqd.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1 \
  -o build/corefreqd.o
cc  -Wall -Wfatal-errors -Wno-unused-variable -c x86_64/corefreqm.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1 \
  -o build/corefreqm.o
cc  -Wall -Wfatal-errors -Wno-unused-variable -c x86_64/corefreq-cli.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1  \
  -o build/corefreq-cli.o
cc  -Wall -Wfatal-errors -Wno-unused-variable -c x86_64/corefreq-ui.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1 \
  -o build/corefreq-ui.o
cc  -Wall -Wfatal-errors -Wno-unused-variable -c x86_64/corefreq-cli-rsc.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1  \
  -o build/corefreq-cli-rsc.o
cc  -Wall -Wfatal-errors -Wno-unused-variable -c x86_64/corefreq-cli-json.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1 \
  -o build/corefreq-cli-json.o
cc  -Wall -Wfatal-errors -Wno-unused-variable -c x86_64/corefreq-cli-extra.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1 \
  -o build/corefreq-cli-extra.o
cc  -Wall -Wfatal-errors -Wno-unused-variable x86_64/corefreqd.c x86_64/corefreqm.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1 \
  -o build/corefreqd -lpthread -lm -lrt
cc  -Wall -Wfatal-errors -Wno-unused-variable \
  x86_64/corefreq-cli.c x86_64/corefreq-ui.c x86_64/corefreq-cli-rsc.c \
  x86_64/corefreq-cli-json.c x86_64/corefreq-cli-extra.c \
  -D CORE_COUNT=256 -D TASK_ORDER=5 -D MAX_FREQ_HZ=7125000000 -D UBENCH=0 -D DELAY_TSC=1  \
  -o build/corefreq-cli -lm -lrt
make[1]: Entering directory '/usr/lib/modules/x.y.z/build'
  CC [M]  CoreFreq/build/module/corefreqk.o
  LD [M]  CoreFreq/build/corefreqk.o
  MODPOST CoreFreq/build/Module.symvers
  CC [M]  CoreFreq/build/corefreqk.mod.o
  LD [M]  CoreFreq/build/corefreqk.ko
  BTF [M] CoreFreq/build/corefreqk.ko
make[1]: Leaving directory '/usr/lib/modules/x.y.z/build
```
4. (Optionally) Sign the driver
If module signature verification is enabled into Kernel, you will have to sign the `corefreqk.ko` driver.  
* See [module-signing.rst](https://github.com/torvalds/linux/blob/master/Documentation/admin-guide/module-signing.rst) from the Kernel documentation
* See the [Gentoo Wiki](https://wiki.gentoo.org/wiki/Signed_kernel_module_support#Manually_signing_modules)

### Install

#### Manual
5. Copying _CoreFreq_ into the binaries directory  
```sh
make install
```

#### Distribution package
6. Although _CoreFreq_ is released in the ArchLinux AUR ; other sources of distribution may require to reload the systemd daemons:  
```sh
systemctl daemon-reload
```

### Start

7. When built from source code:

* Load the kernel module, from current directory, as root.  
```sh
insmod build/corefreqk.ko
```
* Start the daemon, as root.  
```sh
./build/corefreqd
```
* Start the client, as a user (_in another terminal or console_).  
```sh
./build/corefreq-cli
```

8. When manually installed or from a distribution package:  

* Load the kernel module, as root.  
```sh
modprobe corefreqk
```
* Start the daemon, as root.  
```sh
systemctl start corefreqd
```
* Start the client, as a user.  
```sh
corefreq-cli
```

### Stop

9. Press <kbd>Ctrl+x</kbd> or <kbd>Ctrl+c</kbd> to stop the client.

10. Press <kbd>Ctrl+c</kbd> to stop the daemon (in foreground) or kill its background job.

11. Unload the kernel module  
```sh
rmmod corefreqk.ko
```

### Try
Download the _CoreFreq_ Live CD from the [Wiki](http://github.com/cyring/CoreFreq/wiki/Live-CD)  
![alt text](http://blog.cyring.free.fr/images/CoreFreq_LiveCD_Step1.png "CoreFreq for ArchLinux")  

## Screenshots
### Linux kernel module
Use `lsmod`, `dmesg` or `journalctl -k` to check if the module is started:  
```console
CoreFreq(14:30:-1): Processor [ 8F_71] Architecture [Zen2/Matisse] SMT [32/32]
```

### Daemon

```console
CoreFreq Daemon #.##.#  Copyright (C) 2015-2024 CYRIL COURTIAT

  Processor [AMD Ryzen 9 3950X 16-Core Processor]
  Architecture [Zen2/Matisse] 32/32 CPU Online.
```

### Client

Without arguments, the corefreq-cli program displays Top Monitoring  
![alt text](http://blog.cyring.free.fr/images/CoreFreq_Tour_2017-12-06.gif "CoreFreq UI")  
  _Remark_: Drawing will stall if the terminal width is lower than 80 columns, or its height is less than required.  

* Memory Controller  

![alt text](http://blog.cyring.free.fr/images/CoreFreq_MC.png "IMC")

* With the option `-c`, the client traces counters  

![alt text](http://blog.cyring.free.fr/images/CoreFreq_Counters.gif "CoreFreq Counters")

* Using option `-m` corefreq-cli shows the CPU topology  

![alt text](http://blog.cyring.free.fr/images/CoreFreq_Topology.png "CoreFreq CPU & caches topology")

* With the option `-i` corefreq-cli traces the number of instructions per second / cycle  

```
CPU     IPS            IPC            CPI
#00     0.000579/s     0.059728/c    16.742698/i
#01     0.000334/s     0.150569/c     6.641471/i
#02     0.000598/s     0.161326/c     6.198641/i
#03     0.000294/s     0.233535/c     4.282013/i
#04     0.000240/s     0.042931/c    23.293141/i
#05     0.000284/s     0.158661/c     6.302765/i
#06     0.000128/s     0.128031/c     7.810631/i
#07     0.000088/s     0.150406/c     6.648674/i
```

* Use the option `-s` to show the Processor information (BSP)  

![alt text](http://blog.cyring.free.fr/images/CoreFreq_SysInfo.png "CoreFreq System Info")

## ArchLinux
* [![corefreq](https://img.shields.io/aur/version/corefreq-dkms?style=flat-square)](https://aur.archlinux.org/pkgbase/corefreq/) follows released tags

## Gentoo Linux
* In [GURU overlay](https://wiki.gentoo.org/wiki/Project:GURU/Information_for_End_Users) CoreFreq [package](https://github.com/gentoo/guru/tree/master/sys-apps/corefreq), please contact [vitaly-zdanevich](https://github.com/vitaly-zdanevich)  

## Debian, Ubuntu, TUXEDO
 * Installing the DKMS package will pull the Kernel development packages  
```sh
apt install dkms
```
 * Or install the kernel development prerequisites.  
```sh
apt list git build-essential gawk fakeroot linux-headers*
```

## Red Hat, CentOS
 * Development packages prerequisites.  
```sh
yum install kernel-devel
yum group install "Development Tools"
```

## AlmaLinux
```sh
## as root, install kernel development package and dependencies
dnf --assumeyes install kernel-devel gcc make git bc
```
```sh
## as a User, build CoreFreq
cd CoreFreq
make -j
```
```sh
## as root, install the binaries
make install
## and start Driver and Daemon
modprobe corefreqk
corefreqd
```
```sh
## as a User, start the Client
corefreq-cli
```
```sh
## Terminate Client, Daemon and unload Driver as root
modprobe -r corefreqk
## Proceed to uninstallation as root
cd CoreFreq
make uninstall
```

## openSUSE
 * Packages
1. [CoreFreq](https://software.opensuse.org/package/CoreFreq) official release
2. [CoreFreq-kmp-default](https://software.opensuse.org/package/CoreFreq-kmp-default) 

## ProxMox

Install Tools
```sh
apt-get install build-essential dkms git libpthread-stubs0-dev
```

Install headers related to your kernel
```sh
apt install pve-headers-`uname -r`

git clone https://github.com/cyring/CoreFreq.git
cd CoreFreq && make
```

Install the module in the system, refresh it and start it
```sh
make install
depmod
modprobe corefreqk
systemctl start corefreqd
```

## Unraid
 * Plugin
1. `corefreq.plg` from [ich777/unraid-corefreq](https://github.com/ich777/unraid-corefreq)
2. Based on latest developments, please contact [ich777](https://github.com/ich777)  

## Alpine
```sh
## Install the kernel development packages
apk add alpine-sdk sed installkernel bc nawk diffutils findutils pahole openssl-dev python3 linux-virt linux-virt-dev
```

## Chimera
```sh
## Install the kernel development packages
doas apk add git ckms gmake clang linux-headers linux-lts-devel
## Add the CoreFreq directory containing the ckms.ini file
doas ckms add CoreFreq/
## Build the CoreFreq version M.m.r
doas ckms build corefreqk=M.m.r
## Or manually if using the Clang compiler
gmake CC=clang
```

## [Buildroot](https://github.com/cyring/CoreFreq-buildroot)

## Q&A
* Q: How many CPUs are supported by _CoreFreq_ ?  

  A: Up to 1024 CPUs can be built using the `make` `CORE_COUNT` option.  256 as a default.  

* Q: Turbo Technology is activated however CPUs don't reach those frequencies ?  

* Q: The CPU ratio does not go above its minimum value ?  

* Q: The UI shows erratic counters values !  

  A: In the kernel boot command argument line, *disable the NMI Watchdog*  
`nmi_watchdog=0`  


* Q: The Processor does not enter the C-States ?  

  A1: Check if at least one Idle driver is running.  
  Accordingly to the Processor specs, provide a max_cstate value in the kernel argument as below.  
`intel_idle.max_cstate=value`  

  A2: _CoreFreq_ can also register itself as a cpuidle driver.  
  This time, any idle driver will have to be blacklisted in the kernel command line; such as:  
`modprobe.blacklist=intel_cstate idle=halt intel_idle.max_cstate=0`  
  Start the _CoreFreq_ driver with the `Register_CPU_Idle` parameter:  
`insmod corefreqk.ko Register_CPU_Idle=1`  


* Q: The _CoreFreq_ UI refreshes itself slowly, with a delay after the actual CPUs usage ?  

  A: The sampling time to read the counters can be reduced or increased using a _CoreFreq_ module argument:  
`insmod corefreqk.ko SleepInterval=value`  
  where `<value>` is supplied in milliseconds between a minimum of 100 ms and a maximum of 4500 ms. 1000 ms is the default value.  


* Q: The base clock reports a wrong frequency value ?  

  A: _CoreFreq_ uses various algorithms to estimate the base clock.  
  
  1. The delta of two TimeStamp counters during a defined interval  
  2. The value provided in the Processor brand string divided by the maximum ratio (without Turbo)  
  3. A static value advertised by the manufacturer specs.  
  4. The MSR_FSB_FREQ bits provided with the Core, Core2 and Atom architectures.  

  The _CoreFreq_ module can be started as follow to ignore the first algorithm (frequency estimation):  
`insmod corefreqk.ko AutoClock=0`  

  _Remark: algorithms # 2, 3 and 4 will not return any under/over-clock frequency._  


* Q: The CPU temperature is wrong ?  

  A: _CoreFreq_ employs two MSR registers to calculate the temperature.  
`MSR_IA32_TEMPERATURE_TARGET - MSR_IA32_THERM_STATUS [DTS]`  

  _Remark_: if the MSR_IA32_TEMPERATURE_TARGET is not provided by the Processor, a default value of 100 degree Celsius is considered as a target.  


* Q: The menu option "Memory Controller" does not open any window ?  

  A: Although Uncore and IMC features are under development, they can be activated with the Experimental driver argument:  
`insmod corefreqk.ko Experimental=1`  


* Q: The Instructions and PMC0 counters are stuck to zero ?  
* Q: The Daemon crashes whenever its stress tools are executing !  

  A: The `PCE` bit of control register `CR4` allows RDPMC in ring `3`  
`echo "2" > /sys/devices/cpu/rdpmc`  
  or using systemd, create file `/etc/tmpfiles.d/boot.conf` and add line:  
  `w /sys/devices/cpu/rdpmc - - - - 2`  

  Next, load the driver with the `RDPMC_Enable` argument to override the `CR4` register:  
`insmod corefreqk.ko RDPMC_Enable=1`  


* Q: How to solely control the P-States or the HWP Performance States ?  

  A1: Without the Kernel `cpufreq` framework (aka `CONFIG_CPU_FREQ`), _CoreFreq_ will take the full control over P-States.  
  This allow the User to select a _capped_ frequency from the UI, either per Core, either for the whole Processor.  

  A2: With `cpufreq` built into Kernel, allow _CoreFreq_ to register as a cpufreq driver.  
  In the Kernel boot command line, blacklist any P-state driver; such as:  
`modprobe.blacklist=acpi_cpufreq,pcc_cpufreq intel_pstate=disable`  

  * hardware CPPC (MSR registers)  
`initcall_blacklist=amd_pstate_init`

  * firmware CPPC (ACPI registers)  
`amd_pstate.shared_mem=0` and/or `initcall_blacklist=acpi_cpufreq_init`

 3. load the _CoreFreq_ driver with its `Register_CPU_Freq` parameter:  
`insmod corefreqk.ko Register_CPU_Freq=1`  

* Q: Governor is missing in Kernel window even after a successful registration.  

  A: When Registrations are done through the UI, they have  to be done in the following order:  

 1. Clock Source  
 2. Governor driver  
 3. CPU-FREQ driver  
 4. CPU-IDLE driver  
 5. CPU-IDLE route  

* Q: The CPU freezes or the System crashes.  

  A1: Changing the `Max` ratio frequency (aka P0 P-State) makes the Kernel TSC clock source unstable.  
  1. Boot the Kernel with these command line parameters `notsc nowatchdog`  
  2. Optionally, build the _CoreFreq_ driver with its `udelay()` TSC implementation  
`make DELAY_TSC=1`  
  3. Allow _CoreFreq_ to register a new TSC clock source using driver arguments:  
`insmod corefreqk.ko TurboBoost_Enable=0 Register_ClockSource=1`  
  4. Switch the current system clock source to `corefreq`  
`echo "corefreq" > /sys/devices/system/clocksource/clocksource0/current_clocksource`  

  A2: `[AMD][Zen]` SMU:  
  _CoreFreq_ CPU monitoring loops are executed in an interrupt context where any blocking call like Mutex will freeze the kernel.  
  As a recommendation, **make sure no other SMU driver is running**.  

  A3: This Processor is not or partially implemented in _CoreFreq_.  
  Please open an issue in the [CPU support](https://github.com/cyring/CoreFreq/wiki/CPU-support) Wiki page.  

* Q: No voltage is showing up with Nehalem or Westmere processors ?  

  A: Build _CoreFreq_ as below if one of those chips is present:  
`make HWM_CHIPSET=W83627`  
  or  
`make HWM_CHIPSET=IT8720`  

* Q: `[AMD][Zen]` How to read the idle states ?  

  A: As a workarround to the missing documentation of the hardware counters, _CoreFreq_ implements virtual counters based on the TSC  
  Those VPMC are estimated each time the Kernel is entering an idle state level.  
  The prerequisities are:  
  1. Boot the Kernel without its idle drivers and no `TSC` default clock source set  
  `modprobe.blacklist=acpi_cpufreq idle=halt tsc=unstable`  
  2. Build _CoreFreq_ with its `TSC` implementation  
  `make DELAY_TSC=1`
  3. Load and **register** the _CoreFreq_ kernel module as the system handler  
  `insmod corefreqk.ko Register_ClockSource=1 Register_Governor=1 Register_CPU_Freq=1 Register_CPU_Idle=1 Idle_Route=1`  
  4. Define _CoreFreq_ as the System clock source  
  `echo "corefreq" > /sys/devices/system/clocksource/clocksource0/current_clocksource`  
  5. Start the Daemon then the Client  
![alt text](http://blog.cyring.free.fr/images/CoreFreq_Zen_VPMC.png "CoreFreq for AMD Zen")  
  - The registration is confirmed into the `Settings` window  
  - The idle limit can be changed at any time in the `Kernel` window  
![alt text](http://blog.cyring.free.fr/images/CoreFreq_Idle_Limit.png "Idle Limit")  

* Q: How does _CoreFreq_ work with `cgroups` ?  

  A: The Daemon and the Client have to run in the `root cgroups cpugroup`, by using these commands:  
  ( _thanks to Conne Beest @connebeest_ )  
`cgexec -g cpuset:/ ./corefreqd`  
`cgexec -g cpuset:/ ./cofrefreq-cli`  

* Q: How to enable transparency in the User Interface ?  

  A: Transparency is a build option invoked by the compilation directive `UI_TRANSPARENCY`  
  1. Build the project with `UI_TRANSPARENCY` enabled  
`make UI_TRANSPARENCY=1`
  2. Start the Client with one of its transparency compatible colors theme  
`corefreq-cli -OE 2 -t`  
  3. Or switch to that theme from `Menu > Theme`, shortcut [`E`]  

* Q: How to screenshot the UI ?  

  A: Press `[Ctrl]+[p]` to save the screen to a rich ascii file.  Use the `cat` or `less -R` command to view the file saved with an `asc` extension.  

* Q: How to record the UI ?  

  A: Press `[Alt]+[p]` to record the screen for the duration set in Settings. A compatible [asciinema](https://github.com/asciinema/asciinema) file is saved in the current directory with a `cast` extension.  

* Q: What are the build options for _CoreFreq_ ?  

  A: Enter `make help` to display them:  

```
o---------------------------------------------------------------o
|  make [all] [clean] [info] [help] [install] [module-install]  |
|                                                               |
|  CC=<COMPILER>                                                |
|    where <COMPILER> is cc, gcc, clang                         |
|                                                               |
|  WARNING=<ARG>                                                |
|    where default argument is -Wall                            |
|                                                               |
|  KERNELDIR=<PATH>                                             |
|    where <PATH> is the Kernel source directory                |
|                                                               |
|  CORE_COUNT=<N>                                               |
|    where <N> is 64, 128, 256, 512 or 1024 builtin CPU         |
|                                                               |
|  LEGACY=<L>                                                   |
|    where level <L>                                            |
|    1: assembly level restriction such as CMPXCHG16            |
|                                                               |
|  UBENCH=<N>                                                   |
|    where <N> is 0 to disable or 1 to enable micro-benchmark   |
|                                                               |
|  TASK_ORDER=<N>                                               |
|    where <N> is the memory page unit of kernel allocation     |
|                                                               |
|  FEAT_DBG=<N>                                                 |
|    where <N> is 0 or N for FEATURE DEBUG level                |
|    3: XMM assembly in RING operations                         |
|                                                               |
|  DELAY_TSC=<N>                                                |
|    where <N> is 1 to build a TSC implementation of udelay()   |
|                                                               |
|  OPTIM_LVL=<N>                                                |
|    where <N> is 0, 1, 2 or 3 of the OPTIMIZATION level        |
|                                                               |
|  MAX_FREQ_HZ=<freq>                                           |
|    where <freq> is at least 4850000000 Hz                     |
|                                                               |
|  HWM_CHIPSET=<chipset>                                        |
|    where <chipset> is W83627 or IT8720 or COMPATIBLE          |
|                                                               |
|  Performance Counters:                                        |
|    -------------------------------------------------------    |
|   |     MSR_CORE_PERF_UCC     |     MSR_CORE_PERF_URC     |   |
|   |----------- REG -----------|----------- REG -----------|   |
|   | MSR_IA32_APERF            |  MSR_IA32_MPERF           |   |
|   | MSR_CORE_PERF_FIXED_CTR1  |  MSR_CORE_PERF_FIXED_CTR2 |   |
|   | MSR_PPERF                 |  MSR_PPERF                |   |
|   | MSR_AMD_F17H_APERF        |  MSR_AMD_F17H_MPERF       |   |
|    -------------------------------------------------------    |
|                                                               |
|  Architectural Counters:                                      |
|    -------------------------------------------------------    |
|   |           Intel           |            AMD            |   |
|   |----------- REG -----------|----------- REG -----------|   |
|   |       ARCH_PMC=PCU        |      ARCH_PMC=L3          |   |
|   |                           |      ARCH_PMC=PERF        |   |
|   |                           |      ARCH_PMC=UMC         |   |
|    -------------------------------------------------------    |
|                                                               |
|  User Interface Layout:                                       |
|    NO_HEADER=<F>  NO_FOOTER=<F>  NO_UPPER=<F>  NO_LOWER=<F>   |
|      when <F> is 1: don't build and display this area part    |
|    UI_TRANSPARENCY=<F>                                        |
|      when <F> is 1: build with background transparency        |
|    UI_RULER_MINIMUM=<N>, UI_RULER_MAXIMUM=<N>                 |
|      set ruler left or right bound to <N> frequency ratio     |
|                                                               |
|  Example:                                                     |
|    make CC=gcc OPTIM_LVL=3 FEAT_DBG=1 ARCH_PMC=PCU            |
|         MSR_CORE_PERF_UCC=MSR_CORE_PERF_FIXED_CTR1            |
|         MSR_CORE_PERF_URC=MSR_CORE_PERF_FIXED_CTR2            |
|         HWM_CHIPSET=W83627 MAX_FREQ_HZ=5350000000             |
|         CORE_COUNT=1024 NO_FOOTER=1 NO_UPPER=1                |
|         clean all                                             |
o---------------------------------------------------------------o
```

* Q: What are the parameters of the _CoreFreq_ driver ?  

  A: Use the `modinfo` command to list them:  

```
$ modinfo corefreqk.ko
parm:           ArchID:Force an architecture (ID) (int)
parm:           AutoClock:Estimate Clock Frequency 0:Spec; 1:Once; 2:Auto (int)
parm:           SleepInterval:Timer interval (ms) (uint)
parm:           TickInterval:System requested interval (ms) (uint)
parm:           Experimental:Enable features under development (int)
parm:           CPU_Count:-1:Kernel(default); 0:Hardware; >0: User value (int)
parm:           Target_Ratio_Unlock:1:Target Ratio Unlock; 0:Lock (short)
parm:           Clock_Ratio_Unlock:1:MinRatio; 2:MaxRatio; 3:Both Unlock (short)
parm:           Turbo_Ratio_Unlock:1:Turbo Ratio Unlock; 0:Lock (short)
parm:           Uncore_Ratio_Unlock:1:Uncore Ratio Unlock; 0:Lock (short)
parm:           ServiceProcessor:Select a CPU to run services with (int)
parm:           RDPMC_Enable:Enable RDPMC bit in CR4 register (ushort)
parm:           NMI_Disable:Disable the NMI Handler (ushort)
parm:           Override_SubCstate:Override Sub C-States (array of ushort)
parm:           PkgCStateLimit:Package C-State Limit (short)
parm:           IOMWAIT_Enable:I/O MWAIT Redirection Enable (short)
parm:           CStateIORedir:Power Mgmt IO Redirection C-State (short)
parm:           Config_TDP_Level:Config TDP Control Level (short)
parm:           Custom_TDP_Offset:TDP Limit Offset (watt) (array of short)
parm:           Activate_TDP_Limit:Activate TDP Limiting (array of short)
parm:           Activate_TDP_Clamp:Activate TDP Clamping (array of short)
parm:           Custom_TDC_Offset:TDC Limit Offset (amp) (short)
parm:           Activate_TDC_Limit:Activate TDC Limiting (short)
parm:           L1_HW_PREFETCH_Disable:Disable L1 HW Prefetcher (short)
parm:           L1_HW_IP_PREFETCH_Disable:Disable L1 HW IP Prefetcher (short)
parm:           L1_NPP_PREFETCH_Disable:Disable L1 NPP Prefetcher (short)
parm:           L1_Scrubbing_Enable:Enable L1 Scrubbing (short)
parm:           L2_HW_PREFETCH_Disable:Disable L2 HW Prefetcher (short)
parm:           L2_HW_CL_PREFETCH_Disable:Disable L2 HW CL Prefetcher (short)
parm:           L2_AMP_PREFETCH_Disable:Adaptive Multipath Probability (short)
parm:           L2_NLP_PREFETCH_Disable:Disable L2 NLP Prefetcher (short)
parm:           L1_STRIDE_PREFETCH_Disable:Disable L1 Stride Prefetcher (short)
parm:           L1_REGION_PREFETCH_Disable:Disable L1 Region Prefetcher (short)
parm:           L1_BURST_PREFETCH_Disable:Disable L1 Burst Prefetcher (short)
parm:           L2_STREAM_PREFETCH_Disable:Disable L2 Stream Prefetcher (short)
parm:           L2_UPDOWN_PREFETCH_Disable:Disable L2 Up/Down Prefetcher (short)
parm:           LLC_Streamer_Disable:Disable LLC Streamer (short)
parm:           SpeedStep_Enable:Enable SpeedStep (short)
parm:           C1E_Enable:Enable SpeedStep C1E (short)
parm:           TurboBoost_Enable:Enable Turbo Boost (array of short)
parm:           C3A_Enable:Enable C3 Auto Demotion (short)
parm:           C1A_Enable:Enable C3 Auto Demotion (short)
parm:           C3U_Enable:Enable C3 UnDemotion (short)
parm:           C1U_Enable:Enable C1 UnDemotion (short)
parm:           CC6_Enable:Enable Core C6 State (short)
parm:           PC6_Enable:Enable Package C6 State (short)
parm:           ODCM_Enable:Enable On-Demand Clock Modulation (short)
parm:           ODCM_DutyCycle:ODCM DutyCycle [0-7] | [0-14] (short)
parm:           PowerMGMT_Unlock:Unlock Power Management (short)
parm:           PowerPolicy:Power Policy Preference [0-15] (short)
parm:           Turbo_Activation_Ratio:Turbo Activation Ratio (short)
parm:           PState_FID:P-State Frequency Id (int)
parm:           PState_VID:P-State Voltage Id (int)
parm:           Ratio_Boost:Turbo Boost Frequency ratios (array of int)
parm:           Ratio_PPC:Target Performance ratio (int)
parm:           HWP_Enable:Hardware-Controlled Performance States (short)
parm:           HWP_EPP:Energy Performance Preference (short)
parm:           Ratio_HWP:Hardware-Controlled Performance ratios (array of int)
parm:           HDC_Enable:Hardware Duty Cycling (short)
parm:           EEO_Disable:Disable Energy Efficiency Optimization (short)
parm:           R2H_Disable:Disable Race to Halt (short)
parm:           Clear_Events:Clear Thermal and Power Events (ullong)
parm:           ThermalOffset:Thermal Offset (short)
parm:           ThermalScope:[0:None; 1:SMT; 2:Core; 3:Package] (int)
parm:           VoltageScope:[0:None; 1:SMT; 2:Core; 3:Package] (int)
parm:           PowerScope:[0:None; 1:SMT; 2:Core; 3:Package] (int)
parm:           Register_CPU_Idle:Register the Kernel cpuidle driver (short)
parm:           Register_CPU_Freq:Register the Kernel cpufreq driver (short)
parm:           Register_Governor:Register the Kernel governor (short)
parm:           Register_ClockSource:Register Clock Source driver (short)
parm:           Idle_Route:[0:Default; 1:I/O; 2:HALT; 3:MWAIT] (short)
parm:           Mech_IBRS:Mitigation Mechanism IBRS (short)
parm:           Mech_STIBP:Mitigation Mechanism STIBP (short)
parm:           Mech_SSBD:Mitigation Mechanism SSBD (short)
parm:           Mech_IBPB:Mitigation Mechanism IBPB (short)
parm:           Mech_L1D_FLUSH:Mitigation Mechanism Cache L1D Flush (short)
parm:           Mech_PSFD:Mitigation Mechanism PSFD (short)
parm:           Mech_BTC_NOBR:Mitigation Mechanism BTC-NOBR (short)
parm:           Mech_XPROC_LEAK:Mitigation Mech. Cross Processor Leak (short)
parm:           Mech_AGENPICK:Mitigation Mech. LsCfgDisAgenPick (short)
parm:           WDT_Enable:Watchdog Hardware Timer (short)
parm:           HSMP_Attempt:Attempt the HSMP interface (short)
```  

## Arm [AArch64]
### Screenshots

![alt text](http://blog.cyring.free.fr/images/CoreFreq_RK3588.png "RK3588 OPi 5 Plus")

### Q&A

* Q: Counters are stuck to zero  

  A: Add parameter `nohlt` to the kernel boot command line.  

----

## Algorithm
![alt text](http://blog.cyring.free.fr/images/CoreFreq-algorithm.png "CoreFreq algorithm")

# About
[CyrIng](https://github.com/cyring)

Copyright (C) 2015-2024 CYRIL COURTIAT  
 -------
