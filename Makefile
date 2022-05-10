# CoreFreq
# Copyright (C) 2015-2022 CYRIL INGENIERIE
# Licenses: GPL2

CC ?= cc
WARNING = -Wall -Wfatal-errors
PWD ?= $(shell pwd)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PREFIX ?= /usr
UBENCH = 0
CORE_COUNT ?= 256
TASK_ORDER = 5
MAX_FREQ_HZ ?= 6575000000
MSR_CORE_PERF_UCC ?= MSR_IA32_APERF
MSR_CORE_PERF_URC ?= MSR_IA32_MPERF
ARCH_PMC ?=

obj-m := corefreqk.o
ccflags-y :=	-D CORE_COUNT=$(CORE_COUNT) \
		-D TASK_ORDER=$(TASK_ORDER) \
		-D MAX_FREQ_HZ=$(MAX_FREQ_HZ)
ccflags-y += $(WARNING)

ifeq ($(OPTIM_LVL),0)
OPTIM_FLG = -O$(OPTIM_LVL)
ccflags-y += -fno-inline
else ifneq ($(OPTIM_LVL),)
OPTIM_FLG = -O$(OPTIM_LVL)
ccflags-y += -D OPTIM_LVL=$(OPTIM_LVL)
ccflags-y += $(OPTIM_FLG)
endif

DEFINITIONS =	-D CORE_COUNT=$(CORE_COUNT) -D TASK_ORDER=$(TASK_ORDER) \
		-D MAX_FREQ_HZ=$(MAX_FREQ_HZ) -D UBENCH=$(UBENCH)

ifneq ($(FEAT_DBG),)
DEFINITIONS += -D FEAT_DBG=$(FEAT_DBG)
ccflags-y += -D FEAT_DBG=$(FEAT_DBG)
endif

ifneq ($(LEGACY),)
DEFINITIONS += -D LEGACY=$(LEGACY)
ccflags-y += -D LEGACY=$(LEGACY)
endif

ifneq ($(DELAY_TSC),)
DEFINITIONS += -D DELAY_TSC=$(DELAY_TSC)
ccflags-y += -D DELAY_TSC=$(DELAY_TSC)
endif

ifneq ($(ARCH_PMC),)
DEFINITIONS += -D ARCH_PMC=$(ARCH_PMC)
ccflags-y += -D ARCH_PMC=$(ARCH_PMC)
endif

ccflags-y += -D MSR_CORE_PERF_UCC=$(MSR_CORE_PERF_UCC)
ccflags-y += -D MSR_CORE_PERF_URC=$(MSR_CORE_PERF_URC)

ifneq ($(HWM_CHIPSET),)
	ccflags-y += -D HWM_CHIPSET=$(HWM_CHIPSET)
endif

ifneq ($(NO_HEADER),)
LAYOUT += -D NO_HEADER=$(NO_HEADER)
endif

ifneq ($(NO_FOOTER),)
LAYOUT += -D NO_FOOTER=$(NO_FOOTER)
endif

ifneq ($(NO_UPPER),)
LAYOUT += -D NO_UPPER=$(NO_UPPER)
endif

ifneq ($(NO_LOWER),)
LAYOUT += -D NO_LOWER=$(NO_LOWER)
endif

ifneq ($(UI_TRANSPARENCY),)
LAYOUT += -D UI_TRANSPARENCY=$(UI_TRANSPARENCY)
endif

.PHONY: all
all: corefreqd corefreq-cli
	$(MAKE) -j1 -C $(KERNELDIR) M=$(PWD) modules

.PHONY: install
install: module-install
	install -Dm 0755 corefreq-cli corefreqd -t $(PREFIX)/bin
	install -Dm 0644 corefreqd.service \
		$(PREFIX)/lib/systemd/system/corefreqd.service

.PHONY: module-install
module-install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

.PHONY: clean
clean:
	rm -f corefreqd corefreq-cli
	$(MAKE) -j1 -C $(KERNELDIR) M=$(PWD) clean

corefreqm.o: corefreqm.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreqm.c \
	  $(DEFINITIONS) \
	  -o corefreqm.o

corefreqd.o: corefreqd.c
	$(CC) $(OPTIM_FLG) $(WARNING) -pthread -c corefreqd.c \
	  $(DEFINITIONS) \
	  -o corefreqd.o

corefreqd: corefreqd.o corefreqm.o
	$(CC) $(OPTIM_FLG) $(WARNING) corefreqd.c corefreqm.c \
	  $(DEFINITIONS) \
	  -o corefreqd -lpthread -lm -lrt

corefreq-ui.o: corefreq-ui.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-ui.c \
	  $(DEFINITIONS) \
	  -o corefreq-ui.o

corefreq-cli.o: corefreq-cli.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli.c \
	  $(DEFINITIONS) $(LAYOUT) \
	  -o corefreq-cli.o

corefreq-cli-rsc.o: corefreq-cli-rsc.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli-rsc.c \
	  $(DEFINITIONS) $(LAYOUT) \
	  -o corefreq-cli-rsc.o

corefreq-cli-json.o: corefreq-cli-json.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli-json.c \
	  $(DEFINITIONS) \
	  -o corefreq-cli-json.o

corefreq-cli-extra.o: corefreq-cli-extra.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli-extra.c \
	  $(DEFINITIONS) \
	  -o corefreq-cli-extra.o

corefreq-cli: corefreq-cli.o corefreq-ui.o corefreq-cli-rsc.o \
		corefreq-cli-json.o corefreq-cli-extra.o
	$(CC) $(OPTIM_FLG) $(WARNING) \
	  corefreq-cli.c corefreq-ui.c corefreq-cli-rsc.c \
	  corefreq-cli-json.c corefreq-cli-extra.c \
	  $(DEFINITIONS) $(LAYOUT) \
	  -o corefreq-cli -lm -lrt

.PHONY: info
info:
	$(info CC [$(shell whereis -b $(CC))])
	$(info WARNING [$(WARNING)])
	$(info PWD [$(PWD)])
	$(info KERNELDIR [$(KERNELDIR)])
	$(info PREFIX [$(PREFIX)])
	$(info LEGACY [$(LEGACY)])
	$(info UBENCH [$(UBENCH)])
	$(info FEAT_DBG [$(FEAT_DBG)])
	$(info DELAY_TSC [$(DELAY_TSC)])
	$(info OPTIM_LVL [$(OPTIM_LVL)])
	$(info MSR_CORE_PERF_UCC [$(MSR_CORE_PERF_UCC)])
	$(info MSR_CORE_PERF_URC [$(MSR_CORE_PERF_URC)])
	$(info ARCH_PMC [$(ARCH_PMC)])
	$(info NO_HEADER [$(NO_HEADER)])
	$(info NO_FOOTER [$(NO_FOOTER)])
	$(info NO_UPPER [$(NO_UPPER)])
	$(info NO_LOWER [$(NO_LOWER)])

.PHONY: help
help:
	@echo -e \
	"o---------------------------------------------------------------o\n"\
	"|  make [all] [clean] [info] [help] [install] [module-install]  |\n"\
	"|                                                               |\n"\
	"|  CC=<COMPILER>                                                |\n"\
	"|    where <COMPILER> is cc, gcc       [clang partial support]  |\n"\
	"|                                                               |\n"\
	"|  WARNING=<ARG>                                                |\n"\
	"|    where default argument is -Wall                            |\n"\
	"|                                                               |\n"\
	"|  KERNELDIR=<PATH>                                             |\n"\
	"|    where <PATH> is the Kernel source directory                |\n"\
	"|                                                               |\n"\
	"|  CORE_COUNT=<N>                                               |\n"\
	"|    where <N> is 64, 128, 256, 512 or 1024 builtin CPU         |\n"\
	"|                                                               |\n"\
	"|  LEGACY=<L>                                                   |\n"\
	"|    where level <L>                                            |\n"\
	"|    1: assembly level restriction such as CMPXCHG16            |\n"\
	"|                                                               |\n"\
	"|  UBENCH=<N>                                                   |\n"\
	"|    where <N> is 0 to disable or 1 to enable micro-benchmark   |\n"\
	"|                                                               |\n"\
	"|  TASK_ORDER=<N>                                               |\n"\
	"|    where <N> is the memory page unit of kernel allocation     |\n"\
	"|                                                               |\n"\
	"|  FEAT_DBG=<N>                                                 |\n"\
	"|    where <N> is 0 or N for FEATURE DEBUG level                |\n"\
	"|    3: XMM assembly in RING operations                         |\n"\
	"|                                                               |\n"\
	"|  DELAY_TSC=<N>                                                |\n"\
	"|    where <N> is 1 to build a TSC implementation of udelay()   |\n"\
	"|                                                               |\n"\
	"|  OPTIM_LVL=<N>                                                |\n"\
	"|    where <N> is 0, 1, 2 or 3 of the OPTIMIZATION level        |\n"\
	"|                                                               |\n"\
	"|  MAX_FREQ_HZ=<freq>                                           |\n"\
	"|    where <freq> is at least 4850000000 Hz                     |\n"\
	"|                                                               |\n"\
	"|  HWM_CHIPSET=<chipset>                                        |\n"\
	"|    where <chipset> is W83627 or IT8720 or COMPATIBLE          |\n"\
	"|                                                               |\n"\
	"|  Performance Counters:                                        |\n"\
	"|    -------------------------------------------------------    |\n"\
	"|   |     MSR_CORE_PERF_UCC     |     MSR_CORE_PERF_URC     |   |\n"\
	"|   |----------- REG -----------|----------- REG -----------|   |\n"\
	"|   | MSR_IA32_APERF            |  MSR_IA32_MPERF           |   |\n"\
	"|   | MSR_CORE_PERF_FIXED_CTR1  |  MSR_CORE_PERF_FIXED_CTR2 |   |\n"\
	"|   | MSR_PPERF                 |  MSR_PPERF                |   |\n"\
	"|   | MSR_AMD_F17H_APERF        |  MSR_AMD_F17H_MPERF       |   |\n"\
	"|    -------------------------------------------------------    |\n"\
	"|                                                               |\n"\
	"|  Architectural Counters:                                      |\n"\
	"|    -------------------------------------------------------    |\n"\
	"|   |           Intel           |            AMD            |   |\n"\
	"|   |----------- REG -----------|----------- REG -----------|   |\n"\
	"|   |       ARCH_PMC=PCU        |      ARCH_PMC=L3          |   |\n"\
	"|   |                           |      ARCH_PMC=PERF        |   |\n"\
	"|   |                           |      ARCH_PMC=UMC         |   |\n"\
	"|    -------------------------------------------------------    |\n"\
	"|                                                               |\n"\
	"|  User Interface Layout:                                       |\n"\
	"|    NO_HEADER=<F>  NO_FOOTER=<F>  NO_UPPER=<F>  NO_LOWER=<F>   |\n"\
	"|      when <F> is 1: don't build and display this area part    |\n"\
	"|    UI_TRANSPARENCY=<F>                                        |\n"\
	"|      when <F> is 1: build with background transparency        |\n"\
	"|                                                               |\n"\
	"|  Example:                                                     |\n"\
	"|    make CC=gcc OPTIM_LVL=3 FEAT_DBG=1 ARCH_PMC=PCU            |\n"\
	"|         MSR_CORE_PERF_UCC=MSR_CORE_PERF_FIXED_CTR1            |\n"\
	"|         MSR_CORE_PERF_URC=MSR_CORE_PERF_FIXED_CTR2            |\n"\
	"|         HWM_CHIPSET=W83627 MAX_FREQ_HZ=5350000000             |\n"\
	"|         CORE_COUNT=1024 NO_FOOTER=1 NO_UPPER=1                |\n"\
	"|         clean all                                             |\n"\
	"o---------------------------------------------------------------o"
