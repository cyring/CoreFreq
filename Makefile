obj-m := intelfreq.o
KVERSION = $(shell uname -r)
DESTDIR = $(HOME)

all:	corefreqd corefreq-cli
	make -C /lib/modules/$(KVERSION)/build M=${PWD} modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=${PWD} clean
	rm corefreqd corefreq-cli

corefreqd:	corefreqd.o
	cc -g -lpthread -lrt -o corefreqd corefreqd.c
corefreqd.o:	corefreqd.c
	cc -g -c corefreqd.c -o corefreqd.o

corefreq-cli:	corefreq-cli.o
	cc -g -lrt -o corefreq-cli corefreq-cli.c
corefreq-cli.o: corefreq-cli.c
	cc -g -c corefreq-cli.c -o corefreq-cli.o
