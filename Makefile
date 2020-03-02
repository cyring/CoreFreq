# CoreFreq
# Copyright (C) 2015-2020 CYRIL INGENIERIE
# Licenses: GPL2

CC ?= cc
WARNING = -Wall
PWD ?= $(shell pwd)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PREFIX ?= /usr
UBENCH = 0
FEAT_DBG = 0
TASK_ORDER = 5
MAX_FREQ_HZ = 5250000000
MSR_CORE_PERF_UCC ?= MSR_IA32_APERF
MSR_CORE_PERF_URC ?= MSR_IA32_MPERF

obj-m := corefreqk.o
ccflags-y := -D FEAT_DBG=$(FEAT_DBG) -D TASK_ORDER=$(TASK_ORDER)

ifneq ($(OPTIM_LVL),)
	OPTIM_FLG = -O$(OPTIM_LVL)
	ccflags-y += -D OPTIM_LVL=$(OPTIM_LVL)
	ccflags-y += $(OPTIM_FLG)
endif

DEFINITIONS =	-D FEAT_DBG=$(FEAT_DBG) -D UBENCH=$(UBENCH) \
		-D TASK_ORDER=$(TASK_ORDER) -D MAX_FREQ_HZ=$(MAX_FREQ_HZ)

ccflags-y += -D MSR_CORE_PERF_UCC=$(MSR_CORE_PERF_UCC)
ccflags-y += -D MSR_CORE_PERF_URC=$(MSR_CORE_PERF_URC)

ifneq ($(HWM_CHIPSET),)
	ccflags-y += -D HWM_CHIPSET=$(HWM_CHIPSET)
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
	rm -f corefreqd corefreq-cli corefreq-gui
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
		$(DEFINITIONS) \
		-o corefreq-cli.o

corefreq-cli-rsc.o: corefreq-cli-rsc.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli-rsc.c \
		$(DEFINITIONS) \
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
		$(DEFINITIONS) \
		-o corefreq-cli -lm -lrt

corefreq-gui.o: corefreq-gui.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-gui.c \
		$(DEFINITIONS) \
		-o corefreq-gui.o

corefreq-gui: corefreq-gui.o
	$(CC) $(OPTIM_FLG) $(WARNING) \
		corefreq-gui.c \
		$(DEFINITIONS) \
		-o corefreq-gui -lX11 -lpthread -lrt

.PHONY: info
info:
	$(info CC [$(CC)])
	$(info WARNING [$(WARNING)])
	$(info PWD [$(PWD)])
	$(info KERNELDIR [$(KERNELDIR)])
	$(info PREFIX [$(PREFIX)])
	$(info UBENCH [$(UBENCH)])
	$(info FEAT_DBG [$(FEAT_DBG)])
	$(info OPTIM_LVL [$(OPTIM_LVL)])
	$(info MSR_CORE_PERF_UCC [$(MSR_CORE_PERF_UCC)])
	$(info MSR_CORE_PERF_URC [$(MSR_CORE_PERF_URC)])

.PHONY: help
help:
	@echo -e \
	"o---------------------------------------------------------------o\n"\
	"|  make [all] [clean] [info] [help] [install] [module-install]  |\n"\
	"|                                                               |\n"\
	"|  CC=<COMPILER>                                                |\n"\
	"|    where <COMPILER> is cc, gcc        [clang is unsupported]  |\n"\
	"|                                                               |\n"\
	"|  WARNING=<ARG>                                                |\n"\
	"|    where default argument is -Wall                            |\n"\
	"|                                                               |\n"\
	"|  KERNELDIR=<PATH>                                             |\n"\
	"|    where <PATH> is the Kernel source directory                |\n"\
	"|                                                               |\n"\
	"|  UBENCH=<N>                                                   |\n"\
	"|    where <N> is 0 to disable or 1 to enable micro-benchmark   |\n"\
	"|                                                               |\n"\
	"|  TASK_ORDER=<N>                                               |\n"\
	"|    where <N> is the memory page unit of kernel allocation     |\n"\
	"|                                                               |\n"\
	"|  FEAT_DBG=<N>                                                 |\n"\
	"|    where <N> is 0 or 1 for FEATURE DEBUG level                |\n"\
	"|                                                               |\n"\
	"|  OPTIM_LVL=<N>                                                |\n"\
	"|    where <N> is 0,1,2, or 3 for OPTIMIZATION level            |\n"\
	"|                                                               |\n"\
	"|  MAX_FREQ_HZ=<freq>                                           |\n"\
	"|    where <freq> is at least 4050000000 Hz                     |\n"\
	"|                                                               |\n"\
	"|  HWM_CHIPSET=<chipset>                                        |\n"\
	"|    where <chipset> is W83627 or COMPATIBLE                    |\n"\
	"|                                                               |\n"\
	"|  Performance Counters:                                        |\n"\
	"|    -------------------------------------------------------    |\n"\
	"|   |     MSR_CORE_PERF_UCC     |     MSR_CORE_PERF_URC     |   |\n"\
	"|   |----------- REG -----------|----------- REG -----------|   |\n"\
	"|   | MSR_IA32_APERF            |  MSR_IA32_MPERF           |   |\n"\
	"|   | MSR_CORE_PERF_FIXED_CTR1  |  MSR_CORE_PERF_FIXED_CTR2 |   |\n"\
	"|   | MSR_PPERF                 |  MSR_PPERF                |   |\n"\
	"|    -------------------------------------------------------    |\n"\
	"|                                                               |\n"\
	"|  Example:                                                     |\n"\
	"|    make CC=gcc OPTIM_LVL=3 FEAT_DBG=1                         |\n"\
	"|         MSR_CORE_PERF_UCC=MSR_CORE_PERF_FIXED_CTR1            |\n"\
	"|         MSR_CORE_PERF_URC=MSR_CORE_PERF_FIXED_CTR2            |\n"\
	"|         HWM_CHIPSET=W83627 MAX_FREQ_HZ=5350000000             |\n"\
	"|         clean all                                             |\n"\
	"o---------------------------------------------------------------o"

