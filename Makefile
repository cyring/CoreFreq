# CoreFreq
# Copyright (C) 2015-2018 CYRIL INGENIERIE
# Licenses: GPL2

obj-m := corefreqk.o
KVERSION = $(shell uname -r)
DESTDIR = $(HOME)
CC = cc

all: corefreqd corefreq-cli
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
	rm corefreqd corefreq-cli

corefreqd: corefreqd.o
	$(CC) corefreqd.c -o corefreqd -lpthread -lrt
corefreqd.o: corefreqd.c
	$(CC) -Wall -pthread -c corefreqd.c -o corefreqd.o

corefreq-cli: corefreq-cli.o
	$(CC) corefreq-cli.c -o corefreq-cli -lm -lrt
corefreq-cli.o: corefreq-cli.c
	$(CC) -Wall -c corefreq-cli.c -o corefreq-cli.o
