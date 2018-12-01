# CoreFreq
# Copyright (C) 2015-2018 CYRIL INGENIERIE
# Licenses: GPL2

PWD ?= $(shell pwd)
KVERSION ?= $(shell uname -r)
CC ?= cc
FEAT_DBG = 1
WARNING = -Wall

obj-m := corefreqk.o
ccflags-y := -D FEAT_DBG=$(FEAT_DBG)

ifneq ($(OPTIM_LVL),)
	OPTIM_FLG = -O$(OPTIM_LVL)
	ccflags-y += -D OPTIM_LVL=$(OPTIM_LVL)
	ccflags-y += $(OPTIM_FLG)
endif

ifneq ($(wildcard /dev/watchdog),)
	NMI = 1
else
	NMI = 0
endif

ifndef MSR_CORE_PERF_UCC
	ifeq ($(NMI), 1)
		MSR_CORE_PERF_UCC = MSR_IA32_APERF
	else
		MSR_CORE_PERF_UCC = MSR_CORE_PERF_FIXED_CTR1
	endif
else
	CHK1=$(filter $(MSR_CORE_PERF_UCC), \
			MSR_IA32_APERF MSR_CORE_PERF_FIXED_CTR1)
	ifeq ($(CHK1),)
        $(error MSR_IA32_APERF or MSR_CORE_PERF_FIXED_CTR1 expected)
	endif
endif

ifndef MSR_CORE_PERF_URC
	ifeq ($(NMI), 1)
		MSR_CORE_PERF_URC = MSR_IA32_MPERF
	else
		MSR_CORE_PERF_URC = MSR_CORE_PERF_FIXED_CTR2
	endif
else
	CHK2=$(filter $(MSR_CORE_PERF_URC), \
			MSR_IA32_MPERF MSR_CORE_PERF_FIXED_CTR2)
	ifeq ($(CHK2),)
        $(error MSR_IA32_MPERF or MSR_CORE_PERF_FIXED_CTR2 expected)
	endif
endif

ccflags-y += -D MSR_CORE_PERF_UCC=$(MSR_CORE_PERF_UCC)
ccflags-y += -D MSR_CORE_PERF_URC=$(MSR_CORE_PERF_URC)

all: corefreqd corefreq-cli
	$(MAKE) -j1 -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

.PHONY: clean
clean:
	rm -f corefreqd corefreq-cli
	$(MAKE) -j1 -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

corefreqm.o: corefreqm.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreqm.c -o corefreqm.o

corefreqd.o: corefreqd.c
	$(CC) $(OPTIM_FLG) $(WARNING) -pthread -c corefreqd.c \
		-D FEAT_DBG=$(FEAT_DBG) -o corefreqd.o

corefreqd: corefreqd.o corefreqm.o
	$(CC) $(OPTIM_FLG) $(WARNING) corefreqd.c corefreqm.c \
		-D FEAT_DBG=$(FEAT_DBG) -o corefreqd -lpthread -lm -lrt

corefreq-ui.o: corefreq-ui.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-ui.c -o corefreq-ui.o

corefreq-cli.o: corefreq-cli.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli.c -o corefreq-cli.o

corefreq-cli-json.o: corefreq-cli-json.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli-json.c \
		-o corefreq-cli-json.o

corefreq-cli-extra.o: corefreq-cli-extra.c
	$(CC) $(OPTIM_FLG) $(WARNING) -c corefreq-cli-extra.c \
		-o corefreq-cli-extra.o

corefreq-cli: corefreq-cli.o corefreq-ui.o \
		corefreq-cli-json.o corefreq-cli-extra.o
	$(CC) $(OPTIM_FLG) $(WARNING) \
		corefreq-cli.c corefreq-ui.c \
		corefreq-cli-json.c corefreq-cli-extra.c \
		-o corefreq-cli -lm -lrt

.PHONY: info
info:
	$(info NMI [$(NMI)])
	$(info MSR_CORE_PERF_UCC [$(MSR_CORE_PERF_UCC)])
	$(info MSR_CORE_PERF_URC [$(MSR_CORE_PERF_URC)])

.PHONY: help
help:
	@echo -e \
	"o---------------------------------------------------------------o\n"\
	"|  make [all] [clean] [info] [help]                             |\n"\
	"|                                                               |\n"\
	"|  CC=<COMPILER>                                                |\n"\
	"|    where <COMPILER> is cc, gcc or clang                       |\n"\
	"|                                                               |\n"\
	"|  WARNING=<ARG>                                                |\n"\
	"|    where default argument is -Wall                            |\n"\
	"|                                                               |\n"\
	"|  FEAT_DBG=<N>                                                 |\n"\
	"|    where <N> is 0 or 1 for FEATURE DEBUG level                |\n"\
	"|                                                               |\n"\
	"|  OPTIM_LVL=<N>                                                |\n"\
	"|    where <N> is 0,1,2, or 3 for OPTIMIZATION level            |\n"\
	"|                                                               |\n"\
	"|  MSR_CORE_PERF_UCC=<REG>                                      |\n"\
	"|    where <REG> is MSR_IA32_APERF or MSR_CORE_PERF_FIXED_CTR1  |\n"\
	"|                                                               |\n"\
	"|  MSR_CORE_PERF_URC=<REG>                                      |\n"\
	"|    where <REG> is MSR_IA32_MPERF or MSR_CORE_PERF_FIXED_CTR2  |\n"\
	"o---------------------------------------------------------------o"
