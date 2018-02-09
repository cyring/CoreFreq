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

corefreqm.o: corefreqm.c
	$(CC) -Wall -c corefreqm.c -o corefreqm.o

corefreqd.o: corefreqd.c
	$(CC) -Wall -pthread -c corefreqd.c -o corefreqd.o

corefreqd: corefreqd.o corefreqm.o
	$(CC) corefreqd.c corefreqm.c -o corefreqd -lpthread -lm -lrt

corefreq-ui.o: corefreq-ui.c
	$(CC) -Wall -c corefreq-ui.c -o corefreq-ui.o

corefreq-cli.o: corefreq-cli.c
	$(CC) -Wall -c corefreq-cli.c -o corefreq-cli.o

corefreq-cli: corefreq-cli.o corefreq-ui.o
	$(CC) corefreq-cli.c corefreq-ui.c -o corefreq-cli -lm -lrt
