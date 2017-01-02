# CoreFreq
# Copyright (C) 2015-2017 CYRIL INGENIERIE
# Licenses: GPL2

obj-m := corefreqk.o
KVERSION = $(shell uname -r)
DESTDIR = $(HOME)

all: corefreqd corefreq-cli
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
	rm corefreqd corefreq-cli

corefreqd: corefreqd.o
	cc corefreqd.c -o corefreqd -lpthread -lrt
corefreqd.o: corefreqd.c
	cc -Wall -pthread -c corefreqd.c -o corefreqd.o

corefreq-cli: corefreq-cli.o
	cc corefreq-cli.c -o corefreq-cli -lm -lrt
corefreq-cli.o: corefreq-cli.c
	cc -Wall -c corefreq-cli.c -o corefreq-cli.o
