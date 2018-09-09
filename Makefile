# CoreFreq
# Copyright (C) 2015-2018 CYRIL INGENIERIE
# Licenses: GPL2

CC = cc
FEAT_DBG = 1
OPTIM_LVL =
WARNING = -Wall

obj-m := corefreqk.o
ccflags-y := -D FEAT_DBG=${FEAT_DBG}

ifneq ($(OPTIM_LVL),)
	OPTIM_FLG = -O${OPTIM_LVL}
	ccflags-y += -D OPTIM_LVL=${OPTIM_LVL}
	ccflags-y += ${OPTIM_FLG}
endif

KVERSION = $(shell uname -r)
DESTDIR = ${HOME}

all: corefreqd corefreq-cli
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules

clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
	rm corefreqd corefreq-cli

corefreqm.o: corefreqm.c
	${CC} ${OPTIM_FLG} ${WARNING} -c corefreqm.c -o corefreqm.o

corefreqd.o: corefreqd.c
	${CC} ${OPTIM_FLG} ${WARNING} -pthread -c corefreqd.c \
		-D FEAT_DBG=${FEAT_DBG} -o corefreqd.o

corefreqd: corefreqd.o corefreqm.o
	${CC} ${OPTIM_FLG} ${WARNING} corefreqd.c corefreqm.c \
		-D FEAT_DBG=${FEAT_DBG} -o corefreqd -lpthread -lm -lrt

corefreq-ui.o: corefreq-ui.c
	${CC} ${OPTIM_FLG} ${WARNING} -c corefreq-ui.c -o corefreq-ui.o

corefreq-cli.o: corefreq-cli.c
	${CC} ${OPTIM_FLG} ${WARNING} -c corefreq-cli.c -o corefreq-cli.o

corefreq-cli-json.o: corefreq-cli-json.c
	${CC} ${OPTIM_FLG} ${WARNING} -c corefreq-cli-json.c \
		-o corefreq-cli-json.o

corefreq-cli-extra.o: corefreq-cli-extra.c
	${CC} ${OPTIM_FLG} ${WARNING} -c corefreq-cli-extra.c \
		-o corefreq-cli-extra.o

corefreq-cli: corefreq-cli.o corefreq-ui.o \
		corefreq-cli-json.o corefreq-cli-extra.o
	${CC} ${OPTIM_FLG} ${WARNING} \
		corefreq-cli.c corefreq-ui.c \
		corefreq-cli-json.c corefreq-cli-extra.c \
		-o corefreq-cli -lm -lrt
