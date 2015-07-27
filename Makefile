obj-m := intelfreq.o
KVERSION = $(shell uname -r)
DESTDIR = $(HOME)

all:	corefreqd corefreq-cli
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
	rm corefreqd corefreq-cli

corefreqd:	corefreqd.o
	cc -lpthread -lrt -o corefreqd corefreqd.c
corefreqd.o:	corefreqd.c
	cc -c corefreqd.c -o corefreqd.o

corefreq-cli:	corefreq-cli.o
	cc -lrt -o corefreq-cli corefreq-cli.c
corefreq-cli.o: corefreq-cli.c
	cc -c corefreq-cli.c -o corefreq-cli.o
