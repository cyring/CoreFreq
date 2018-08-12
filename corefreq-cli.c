/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <math.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreq-ui.h"
#include "corefreq-cli.h"
#include "corefreq-cli-json.h"

char hSpace[] = "        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""    ";
char hBar[] =	"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||";
char hLine[] =	"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""----";

static Bit64 Shutdown __attribute__ ((aligned (64))) = 0x0;

SERVICE_PROC localService = {.Proc = -1};

int ClientFollowService(SERVICE_PROC *pSlave, SERVICE_PROC *pMaster, pid_t pid)
{
	if (pSlave->Proc != pMaster->Proc) {
		pSlave->Proc = pMaster->Proc;

		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(pSlave->Core, &cpuset);
		if (pSlave->Thread != -1)
			CPU_SET(pSlave->Thread, &cpuset);

		return(sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset));
	}
	return(0);
}

void Emergency(int caught)
{
	switch (caught) {
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
	case SIGTSTP:
	case SIGCHLD:
		BITSET(LOCKLESS, Shutdown, 0);
		break;
	}
}

void TrapSignal(SHM_STRUCT *Shm, int operation)
{
	if (operation == 0) {
		Shm->AppCli = 0;
	} else {
		Shm->AppCli = getpid();
		signal(SIGINT,  Emergency);
		signal(SIGQUIT, Emergency);
		signal(SIGTERM, Emergency);
		signal(SIGTSTP, Emergency);	// [CTRL] + [Z]
		signal(SIGCHLD, Emergency);
	}
}

unsigned int Dec2Digit(unsigned int decimal, unsigned int thisDigit[])
{
	memset(thisDigit, 0, 9 * sizeof(unsigned int));
	unsigned int j = 9;
	while (decimal > 0) {
		thisDigit[--j] = decimal % 10;
		decimal /= 10;
	}
	return(9 - j);
}

void printv(	CELL_FUNC OutFunc,
		unsigned long long key,
		ATTRIBUTE *attrib,
		CUINT width,
		int tab,
		char *fmt, ...)
{
	const char *indent[2][4] = {
		{"",	"|",	"|- ",	"   |- "},
		{"",	" ",	"  ",	"   "}
	};
	char *line = malloc(width + 1);

	va_list ap;
	va_start(ap, fmt);
	vsprintf(line, fmt, ap);

    if (OutFunc == NULL)
	printf("%s%s%.*s\n", indent[0][tab], line,
		(int)(width - strlen(line) - strlen(indent[0][tab])), hSpace);
    else {
	ASCII *item = malloc(width + 1);
	sprintf((char *)item, "%s%s%.*s", indent[1][tab], line,
		(int)(width - strlen(line) - strlen(indent[1][tab])), hSpace);
	OutFunc(key, attrib, item);
	free(item);
    }
	va_end(ap);
	free(line);
}

void SysInfoCPUID(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[4][74] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LCK,LCK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK
	    },
	    {
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK
	    },
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK
	    },
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,
		HWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,
		HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,HWK,
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,
		LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,
		LWK,LWK,LWK,LWK
	    }
	};
	char format[] = "%08x:%08x%.*s%08x     %08x     %08x     %08x";
	unsigned int cpu;
	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	    if (OutFunc == NULL) {
		printv(OutFunc, SCANKEY_NULL, attrib[0], width, 0,
			"CPU #%-2u function"				\
			"          EAX          EBX          ECX          EDX",
			cpu);
	    } else {
		printv(OutFunc, SCANKEY_NULL,
			attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)], width, 0,
			"CPU #%-2u", cpu);
	    }
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		printv(OutFunc, SCANKEY_NULL, attrib[3], width, 2, format,
			0x00000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc,
			Shm->Cpu[cpu].Query.StdFunc.BX,
			Shm->Cpu[cpu].Query.StdFunc.CX,
			Shm->Cpu[cpu].Query.StdFunc.DX);

		printv(OutFunc, SCANKEY_NULL, attrib[2], width, 3,
			"Largest Standard Function=%08x",
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc);

		printv(OutFunc, SCANKEY_NULL, attrib[3], width, 2, format,
			0x80000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc,
			Shm->Cpu[cpu].Query.ExtFunc.EBX,
			Shm->Cpu[cpu].Query.ExtFunc.ECX,
			Shm->Cpu[cpu].Query.ExtFunc.EDX);

		printv(OutFunc, SCANKEY_NULL, attrib[2], width, 3,
			"Largest Extended Function=%08x",
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc);

		int i;
		for (i = 0; i < CPUID_MAX_FUNC; i++)
		    if (Shm->Cpu[cpu].CpuID[i].func) {
			printv(OutFunc, SCANKEY_NULL, attrib[3], width, 2,
				format,
				Shm->Cpu[cpu].CpuID[i].func,
				Shm->Cpu[cpu].CpuID[i].sub,
				4, hSpace,
				Shm->Cpu[cpu].CpuID[i].reg[0],
				Shm->Cpu[cpu].CpuID[i].reg[1],
				Shm->Cpu[cpu].CpuID[i].reg[2],
				Shm->Cpu[cpu].CpuID[i].reg[3]);
		    }
	    }
	}
}

void SystemRegisters(SHM_STRUCT *Shm, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[3][4] = {
	    {
		LWK,LWK,LWK,LWK
	    },{
		HBK,HBK,HBK,HBK
	    },{
		HWK,HWK,HWK,HWK
	    }
	};
	ASCII item[8];
	const struct {
		enum SYS_REG bit;
		unsigned int len;
		const char *flag;
	} SR[] = {
		{RFLAG_TF,	1,	" TF "},
		{RFLAG_IF,	1,	" IF "},
		{RFLAG_IOPL,	2,	"IOPL"},
		{RFLAG_NT,	1,	" NT "},
		{RFLAG_RF,	1,	" RF "},
		{RFLAG_VM,	1,	" VM "},
		{RFLAG_AC,	1,	" AC "},
		{RFLAG_VIF,	1,	" VIF"},
		{RFLAG_VIP,	1,	" VIP"},
		{RFLAG_ID,	1,	" ID "},

		{CR0_PE,	1,	" PE "},
		{CR0_MP,	1,	" MP "},
		{CR0_EM,	1,	" EM "},
		{CR0_TS,	1,	" TS "},
		{CR0_ET,	1,	" ET "},
		{CR0_NE,	1,	" NE "},
		{CR0_WP,	1,	" WP "},
		{CR0_AM,	1,	" AM "},
		{CR0_NW,	1,	" NW "},
		{CR0_CD,	1,	" CD "},
		{CR0_PG,	1,	" PG "},

		{CR3_PWT,	1,	" PWT"},
		{CR3_PCD,	1,	" PCD"},

		{CR4_VME,	1,	" VME"},
		{CR4_PVI,	1,	" PVI"},
		{CR4_TSD,	1,	" TSD"},
		{CR4_DE,	1,	" DE "},
		{CR4_PSE,	1,	" PSE"},
		{CR4_PAE,	1,	" PAE"},
		{CR4_MCE,	1,	" MCE"},
		{CR4_PGE,	1,	" PGE"},
		{CR4_PCE,	1,	" PCE"},
		{CR4_OSFXSR,	1,	" FX "},
		{CR4_OSXMMEXCPT,1,	"XMM "},
		{CR4_UMIP,	1,	"UMIP"},
		{CR4_VMXE,	1,	" VMX"},
		{CR4_SMXE,	1,	" SMX"},
		{CR4_FSGSBASE,	1,	" FS "},
		{CR4_PCIDE,	1,	"PCID"},
		{CR4_OSXSAVE,	1,	" SAV"},
		{CR4_SMEP,	1,	" SME"},
		{CR4_SMAP,	1,	" SMA"},
		{CR4_PKE,	1,	" PKE"},

		{EXFCR_LOCK,	1,	"LCK "},
		{EXFCR_VMX_IN_SMX,1,	"VMX^"},
		{EXFCR_VMXOUT_SMX,1,	"SGX "},
		{EXFCR_SENTER_LEN,6,	"[SEN"},
		{EXFCR_SENTER_GEN,1,	"TER]"},
		{EXFCR_SGX_LCE, 1,	" [ S"},
		{EXFCR_SGX_GEN, 1,	"GX ]"},
		{EXFCR_LMCE,	1,	" LMC"},

		{EXFER_SCE,	1,	" SCE"},
		{EXFER_LME,	1,	" LME"},
		{EXFER_LMA,	1,	" LMA"},
		{EXFER_NXE,	1,	" NXE"},
		{EXFER_SVME,	1,	" SVM"}
	};
	const struct {
		unsigned int Start, Stop;
	} tabRFLAGS = {0, 10},
	tabCR0 = {tabRFLAGS.Stop, tabRFLAGS.Stop + 11},
	tabCR3 = {tabCR0.Stop, tabCR0.Stop + 2},
	tabCR4[2] = {	{tabCR3.Stop, tabCR3.Stop + 4},
			{tabCR4[0].Stop, tabCR4[0].Stop + 16}
	},
	tabEFCR = {tabCR4[1].Stop, tabCR4[1].Stop + 8},
	tabEFER = {tabEFCR.Stop, tabEFCR.Stop + 5};
	unsigned int cpu, idx = 0;
	unsigned int nl = 17;

	void printm(ATTRIBUTE *attrib, char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf((char*) item, fmt, ap);
		if (OutFunc == NULL) {
			nl--;
			if (nl == 0) {
				nl = 17;
				printf("%s\n", item);
			} else
				printf("%s", item);
		} else
			OutFunc(SCANKEY_NULL, attrib, item);
		va_end(ap);
	}

/* Section Mark */
	printm(attrib[0], "CPU "); printm(attrib[0], "FLAG");
    for (idx = tabRFLAGS.Start; idx < tabRFLAGS.Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
	printm(attrib[0], "    ");
	printm(attrib[0], "CR3:");
    for (idx = tabCR3.Start; idx < tabCR3.Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
	printm(attrib[0], "    ");
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)], "#%-2u ", cpu);

	printm(attrib[0], "    ");
	for (idx = tabRFLAGS.Start; idx < tabRFLAGS.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.RFLAGS,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
	printm(attrib[0], "    ");
	printm(attrib[0], "    ");
	for (idx = tabCR3.Start; idx < tabCR3.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR3,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
	printm(attrib[0], "    ");
    }
/* Section Mark */
	printm(attrib[0], "CR0:");
    for (idx = tabCR0.Start; idx < tabCR0.Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
	printm(attrib[0], "CR4:");
    for (idx = tabCR4[0].Start; idx < tabCR4[0].Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)], "#%-2u ", cpu);

	for (idx = tabCR0.Start; idx < tabCR0.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR0,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
	printm(attrib[0], "    ");
	for (idx = tabCR4[0].Start; idx < tabCR4[0].Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR4,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
    }
/* Section Mark */
	printm(attrib[0], "CR4:");
    for (idx = tabCR4[1].Start; idx < tabCR4[1].Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)], "#%-2u ", cpu);
	for (idx = tabCR4[1].Start; idx < tabCR4[1].Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR4,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
    }
/* Section Mark */
	printm(attrib[0], "EFCR");
	printm(attrib[0], "    ");
    for (idx = tabEFCR.Start; idx < tabEFCR.Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
	printm(attrib[0], "    ");
	printm(attrib[0], "EFER");
    for (idx = tabEFER.Start; idx < tabEFER.Stop; idx++) {
	printm(attrib[0], "%s", SR[idx].flag);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)], "#%-2u ", cpu);
	printm(attrib[0], "    ");

	for (idx = tabEFCR.Start; idx < tabEFCR.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)
	    &&  ((Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.EFCR,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
	printm(attrib[0], "    ");
	printm(attrib[0], "    ");
	for (idx = tabEFER.Start; idx < tabEFER.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.EFER,
				SR[idx].bit, SR[idx].len));
	    else
		printm(attrib[1], "  - ");
	}
    }
}

void SysInfoProc(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[4][76] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HGK,HGK,
		HGK,HGK,HGK,HGK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,
		HWK,HWK,HWK,HWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,
		HWK,HWK,HWK,HWK,LWK,LWK
	    }
	};
	char	*str = malloc(width + 1), symb[2][2] = {{'[', ']'}, {'<', '>'}};
	int	activeCores, boost = 0;

  void PrintCoreBoost(char *pfx, int _boost, int syc, unsigned long long _key)
  {
    if (Shm->Proc.Boost[_boost] > 0) {
	sprintf(str, "%.*s""%s""%.*s""%7.2f""%.*s""%c%4d %c",
	(int) (20 - strlen(pfx)), hSpace, pfx, 3, hSpace,
	(double) ( Shm->Proc.Boost[_boost]
		* Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz) / 1000000.0,
		20, hSpace,
		symb[syc][0],
		Shm->Proc.Boost[_boost],
		symb[syc][1]);
	printv(OutFunc, _key, attrib[3], width, 0, str);
    }
  }

  void PrintUncoreBoost(char *pfx, int _boost, int syc, unsigned long long _key)
  {
    if (Shm->Uncore.Boost[_boost] > 0) {
	sprintf(str, "%.*s""%s""%.*s""%7.2f""%.*s""%c%4d %c",
	(int) (20 - strlen(pfx)), hSpace, pfx, 3, hSpace,
	(double) ( Shm->Uncore.Boost[_boost]
		* Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz) / 1000000.0,
		20, hSpace,
		symb[syc][0],
		Shm->Uncore.Boost[_boost],
		symb[syc][1]);
	printv(OutFunc, _key, attrib[3], width, 0, str);
    }
  }

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 0,
		"Processor%.*s[%s]",
		width - 11 - strlen(Shm->Proc.Brand), hSpace, Shm->Proc.Brand);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Architecture%.*s[%s]",
		width - 17 - strlen(Shm->Proc.Architecture), hSpace,
		Shm->Proc.Architecture);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Vendor ID%.*s[%s]",
		width - 14 - strlen(Shm->Proc.Features.Info.Vendor.ID), hSpace,
		Shm->Proc.Features.Info.Vendor.ID);

	printv(OutFunc, SCANKEY_NULL, attrib[2], width, 2,
		"Signature%.*s[%2X%1X_%1X%1X]",
		width - 20, hSpace,
		Shm->Proc.Features.Std.EAX.ExtFamily,
		Shm->Proc.Features.Std.EAX.Family,
		Shm->Proc.Features.Std.EAX.ExtModel,
		Shm->Proc.Features.Std.EAX.Model);

	printv(OutFunc, SCANKEY_NULL, attrib[2], width, 2,
		"Stepping%.*s[%6u]",
		width - 19, hSpace, Shm->Proc.Features.Std.EAX.Stepping);

	printv(OutFunc, SCANKEY_NULL, attrib[2], width, 2,
		"Microcode%.*s[%6u]",
		width - 20, hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].Query.Microcode);

	printv(OutFunc, SCANKEY_NULL, attrib[2], width, 2,
		"Online CPU%.*s[ %2u/%-2u]",
		width - 21, hSpace, Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	printv(OutFunc, SCANKEY_NULL, attrib[2], width, 2,
		"Base Clock%.*s[%6.2f]",
		width - 21, hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz / 1000000.0);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Frequency%.*s(Mhz)%.*sRatio",
		12, hSpace, 23 - (OutFunc == NULL), hSpace);

	PrintCoreBoost("Min", BOOST(MIN), 0, SCANKEY_NULL);
	PrintCoreBoost("Max", BOOST(MAX), 0, SCANKEY_NULL);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2, "Factory%.*s[%6.2f]",
		(OutFunc == NULL) ? 62 : 58, hSpace,
			Shm->Proc.Features.Factory.Clock.Hz / 1000000.0);

	printv(OutFunc, SCANKEY_NULL, attrib[3], width, 0,
		"%.*s""%5u""%.*s""[%4d ]",
		22, hSpace, Shm->Proc.Features.Factory.Freq,
		23, hSpace, Shm->Proc.Features.Factory.Ratio);

	printv(OutFunc, SCANKEY_NULL, attrib[Shm->Proc.Features.Ratio_Unlock],
		width, 2,
		"Turbo Boost%.*s[%6s]", width - 22, hSpace,
		Shm->Proc.Features.Ratio_Unlock ? "UNLOCK" : "LOCK");

    if (Shm->Proc.Features.Ratio_Unlock)
      for (boost = BOOST(1C), activeCores = 1;
		boost > BOOST(1C) - Shm->Proc.Features.SpecTurboRatio;
			boost--, activeCores++)
	{
	CLOCK_ARG clockMod={.Ratio=BOXKEY_TURBO_CLOCK_NC|activeCores,.Offset=0};
	char pfx[4];
	sprintf(pfx, "%2dC", activeCores);
	PrintCoreBoost(pfx, boost, 1, clockMod.sllong);
      }
    else
      for (boost = BOOST(1C), activeCores = 1;
		boost > BOOST(1C) - Shm->Proc.Features.SpecTurboRatio;
			boost--, activeCores++) {
	char pfx[4];
	sprintf(pfx, "%2dC", activeCores);
	PrintCoreBoost(pfx, boost, 0, SCANKEY_NULL);
      }

	printv(OutFunc, SCANKEY_NULL, attrib[Shm->Proc.Features.Uncore_Unlock],
		width, 2, "Uncore%.*s[%6s]", width - 17, hSpace,
		Shm->Proc.Features.Uncore_Unlock ? "UNLOCK" : "LOCK");

    if (Shm->Proc.Features.Uncore_Unlock) {
	CLOCK_ARG uncoreClock = {.Ratio = 0, .Offset = 0};

	uncoreClock.Ratio = BOXKEY_UNCORE_CLOCK_OR | 2;
	PrintUncoreBoost("Min", UNCORE_BOOST(MIN), 1, uncoreClock.sllong);

	uncoreClock.Ratio = BOXKEY_UNCORE_CLOCK_OR | 1;
	PrintUncoreBoost("Max", UNCORE_BOOST(MAX), 1, uncoreClock.sllong);
    } else {
	PrintUncoreBoost("Min", UNCORE_BOOST(MIN), 0,SCANKEY_NULL);
	PrintUncoreBoost("Max", UNCORE_BOOST(MAX), 0,SCANKEY_NULL);
    }

    if (Shm->Proc.Features.TDP_Levels > 0) {
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"TDP%.*sLevel [%3d:%-2d]",
		width - 20, hSpace,
		Shm->Proc.Features.TDP_Cfg_Level,Shm->Proc.Features.TDP_Levels);

	printv(OutFunc, SCANKEY_NULL, attrib[Shm->Proc.Features.TDP_Unlock],
		width, 3, "Programmable%.*s[%6s]",
		width - (OutFunc == NULL ? 26 : 24), hSpace,
		Shm->Proc.Features.TDP_Unlock ? "UNLOCK" : "LOCK");

	printv(OutFunc, SCANKEY_NULL, attrib[!Shm->Proc.Features.TDP_Cfg_Lock],
		width, 3, "Configuration%.*s[%6s]",
		width - (OutFunc == NULL ? 27 : 25), hSpace,
		Shm->Proc.Features.TDP_Cfg_Lock ? "LOCK" : "UNLOCK");

	printv(OutFunc,SCANKEY_NULL,attrib[!Shm->Proc.Features.TurboRatio_Lock],
		width, 3, "Turbo Activation%.*s[%6s]",
		width - (OutFunc == NULL ? 30 : 28), hSpace,
		Shm->Proc.Features.TurboRatio_Lock ? "LOCK" : "UNLOCK");

	PrintCoreBoost("Nominal", BOOST(TDP), 0, SCANKEY_NULL);
	PrintCoreBoost("Level1", BOOST(TDP1), 0, SCANKEY_NULL);
	PrintCoreBoost("Level2", BOOST(TDP2), 0, SCANKEY_NULL);
	PrintCoreBoost("Turbo", BOOST(ACT), 0, SCANKEY_NULL);
    }
	free(str);
}

void SysInfoISA(SHM_STRUCT *Shm, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[4][5][17] = {
	  {
	    {	// [N] & [N/N]	2*(0|0)+(0<<0)
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },{ // [Y]
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,HGK,LWK
	    },{ // [N/Y]	2*(0|1)+(0<<1)
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,HGK,LWK
	    },{ // [Y/N]	2*(1|0)+(1<<0)
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HGK,LWK,LWK,LWK
	    },{ // [Y/Y]	2*(1|1)+(1<<1)
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HGK,LWK,HGK,LWK
	    }
	  },
	  {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HGK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,HGK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,HGK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,HGK,LWK,HGK,LWK,LWK
	    }
	  },
	  {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HGK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HGK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,HGK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,HGK,LWK,HGK,LWK,LWK,LWK
	    }
	  },
	  {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,HGK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,HGK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,HGK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,HGK,LWK,HGK,LWK,LWK
	    }
	  }
	};
	ASCII item[18];
	unsigned int nl = 4;

	void printm(ATTRIBUTE *attrib, char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf((char*) item, fmt, ap);
		if (OutFunc == NULL) {
			nl--;
			if (nl == 3)
				printf("|-%s", item);
			else if (nl == 0) {
				nl = 4;
				printf("%s\n", item);
			} else
				printf("%s", item);
		} else
			OutFunc(SCANKEY_NULL, attrib, item);
		va_end(ap);
	}
// Row Mark
	printm(attrib[0][2 * (Shm->Proc.Features.ExtInfo.EDX._3DNow
			   |  Shm->Proc.Features.ExtInfo.EDX._3DNowEx)
			   + (Shm->Proc.Features.ExtInfo.EDX._3DNow
			   << Shm->Proc.Features.ExtInfo.EDX._3DNowEx)],
		" 3DNow!/Ext [%c,%c]",
		Shm->Proc.Features.ExtInfo.EDX._3DNow ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.EDX._3DNowEx ? 'Y' : 'N');

	printm(attrib[1][Shm->Proc.Features.Std.ECX.AES],
		"        AES [%c]  ",
		Shm->Proc.Features.Std.ECX.AES ? 'Y' : 'N');

	printm(attrib[2][2 * (Shm->Proc.Features.Std.ECX.AVX
			   |  Shm->Proc.Features.ExtFeature.EBX.AVX2)
			   + (Shm->Proc.Features.Std.ECX.AVX
			   << Shm->Proc.Features.ExtFeature.EBX.AVX2)],
		" AVX/AVX2 [%c/%c]  ",
		Shm->Proc.Features.Std.ECX.AVX ? 'Y' : 'N',
		Shm->Proc.Features.ExtFeature.EBX.AVX2 ? 'Y' : 'N');

	printm(attrib[3][2 * (Shm->Proc.Features.ExtFeature.EBX.BMI1
			   |  Shm->Proc.Features.ExtFeature.EBX.BMI2)
			   + (Shm->Proc.Features.ExtFeature.EBX.BMI1
			   << Shm->Proc.Features.ExtFeature.EBX.BMI2)],
		" BMI1/BMI2 [%c/%c] ",
		Shm->Proc.Features.ExtFeature.EBX.BMI1 ? 'Y' : 'N',
		Shm->Proc.Features.ExtFeature.EBX.BMI2 ? 'Y' : 'N');
// Row Mark
	printm(attrib[0][Shm->Proc.Features.Std.EDX.CLFSH],
		" CLFSH        [%c]",
		Shm->Proc.Features.Std.EDX.CLFSH ? 'Y' : 'N');

	printm(attrib[1][Shm->Proc.Features.Std.EDX.CMOV],
		"       CMOV [%c]  ",
		Shm->Proc.Features.Std.EDX.CMOV ? 'Y' : 'N');

	printm(attrib[2][Shm->Proc.Features.Std.EDX.CMPXCH8],
		"    CMPXCH8 [%c]  ",
		Shm->Proc.Features.Std.EDX.CMPXCH8 ? 'Y' : 'N');

	printm(attrib[3][Shm->Proc.Features.Std.ECX.CMPXCH16],
		"    CMPXCH16 [%c] ",
		Shm->Proc.Features.Std.ECX.CMPXCH16 ? 'Y' : 'N');
// Row Mark
	printm(attrib[0][Shm->Proc.Features.Std.ECX.F16C],
		" F16C         [%c]",
		Shm->Proc.Features.Std.ECX.F16C ? 'Y' : 'N');

	printm(attrib[1][Shm->Proc.Features.Std.EDX.FPU],
		"        FPU [%c]  ",
		Shm->Proc.Features.Std.EDX.FPU ? 'Y' : 'N');

	printm(attrib[2][Shm->Proc.Features.Std.EDX.FXSR],
		"       FXSR [%c]  ",
		Shm->Proc.Features.Std.EDX.FXSR ? 'Y' : 'N');

	printm(attrib[3][Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF],
		"   LAHF/SAHF [%c] ",
		Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF ? 'Y' : 'N');
// Row Mark
	printm(attrib[0][2 * (Shm->Proc.Features.Std.EDX.MMX
			   |  Shm->Proc.Features.ExtInfo.EDX.MMX_Ext)
			   + (Shm->Proc.Features.Std.EDX.MMX
			   << Shm->Proc.Features.ExtInfo.EDX.MMX_Ext)],
		" MMX/Ext    [%c/%c]",
		Shm->Proc.Features.Std.EDX.MMX ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.EDX.MMX_Ext ? 'Y' : 'N');

	printm(attrib[1][Shm->Proc.Features.Std.ECX.MONITOR],
		"    MONITOR [%c]  ",
		Shm->Proc.Features.Std.ECX.MONITOR ? 'Y' : 'N');

	printm(attrib[2][Shm->Proc.Features.Std.ECX.MOVBE],
		"      MOVBE [%c]  ",
		Shm->Proc.Features.Std.ECX.MOVBE ? 'Y' : 'N');

	printm(attrib[3][Shm->Proc.Features.Std.ECX.PCLMULDQ],
		"    PCLMULDQ [%c] ",
		Shm->Proc.Features.Std.ECX.PCLMULDQ ? 'Y' : 'N');
// Row Mark
	printm(attrib[0][Shm->Proc.Features.Std.ECX.POPCNT],
		" POPCNT       [%c]",
		Shm->Proc.Features.Std.ECX.POPCNT ? 'Y' : 'N');

	printm(attrib[1][Shm->Proc.Features.Std.ECX.RDRAND],
		"     RDRAND [%c]  ",
		Shm->Proc.Features.Std.ECX.RDRAND ? 'Y' : 'N');

	printm(attrib[2][Shm->Proc.Features.ExtInfo.EDX.RDTSCP],
		"     RDTSCP [%c]  ",
		Shm->Proc.Features.ExtInfo.EDX.RDTSCP ? 'Y' : 'N');

	printm(attrib[3][Shm->Proc.Features.Std.EDX.SEP],
		"         SEP [%c] ",
		Shm->Proc.Features.Std.EDX.SEP ? 'Y' : 'N');
// Row Mark
	printm(attrib[0][Shm->Proc.Features.ExtFeature.EBX.SGX],
		" SGX          [%c]",
		Shm->Proc.Features.ExtFeature.EBX.SGX ? 'Y' : 'N');

	printm(attrib[1][Shm->Proc.Features.Std.EDX.SSE],
		"        SSE [%c]  ",
		Shm->Proc.Features.Std.EDX.SSE ? 'Y' : 'N');

	printm(attrib[2][Shm->Proc.Features.Std.EDX.SSE2],
		"       SSE2 [%c]  ",
		Shm->Proc.Features.Std.EDX.SSE2 ? 'Y' : 'N');

	printm(attrib[3][Shm->Proc.Features.Std.ECX.SSE3],
		"        SSE3 [%c] ",
		Shm->Proc.Features.Std.ECX.SSE3 ? 'Y' : 'N');
// Row Mark
	printm(attrib[0][Shm->Proc.Features.Std.ECX.SSSE3],
		" SSSE3        [%c]",
		Shm->Proc.Features.Std.ECX.SSSE3 ? 'Y' : 'N');

	printm(attrib[1][2 * (Shm->Proc.Features.Std.ECX.SSE41
			   |  Shm->Proc.Features.ExtInfo.ECX.SSE4A)
			   + (Shm->Proc.Features.Std.ECX.SSE41
			   << Shm->Proc.Features.ExtInfo.ECX.SSE4A)],
		" SSE4.1/4A [%c/%c] ",
		Shm->Proc.Features.Std.ECX.SSE41 ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.ECX.SSE4A ? 'Y' : 'N');

	printm(attrib[2][Shm->Proc.Features.Std.ECX.SSE42],
		"     SSE4.2 [%c]  ",
		Shm->Proc.Features.Std.ECX.SSE42 ? 'Y' : 'N');

	printm(attrib[3][Shm->Proc.Features.ExtInfo.EDX.SYSCALL],
		"     SYSCALL [%c] ",
		Shm->Proc.Features.ExtInfo.EDX.SYSCALL ? 'Y' : 'N');
}

void SysInfoFeatures(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[3][72] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,
		LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,
		LWK,LWK
	    }
	};
	const char *TSC[] = {
		"Missing",
		"Variant",
		"Invariant"
	};
	const char *x2APIC[] = {
		"Missing",
		"  xAPIC",
		" x2APIC"
	};
	int bix;
// Section Mark
	bix = Shm->Proc.Features.ExtInfo.EDX.PG_1GB == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"1 GB Pages Support%.*s1GB-PAGES   [%7s]",
		width - 42, hSpace, powered(bix));

	bix = Shm->Proc.Features.AdvPower.EDX._100MHz == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"100 MHz multiplier Control%.*s100MHzSteps   [%7s]",
		width - 52, hSpace, powered(bix));

	bix = (Shm->Proc.Features.Std.EDX.ACPI == 1)		// Intel
	   || (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1);	// AMD
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Advanced Configuration & Power Interface%.*sACPI   [%7s]",
		width - 59, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.APIC == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Advanced Programmable Interrupt Controller%.*sAPIC   [%7s]",
		width - 61, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtInfo.ECX.MP_Mode == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Core Multi-Processing%.*sCMP Legacy   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.CNXT_ID == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"L1 Data Cache Context ID%.*sCNXT-ID   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.DCA == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Direct Cache Access%.*sDCA   [%7s]",
		width - 37, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.DE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Debugging Extension%.*sDE   [%7s]",
		width - 36, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.DS_PEBS == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Debug Store & Precise Event Based Sampling"
					"%.*sDS, PEBS   [%7s]",
		width - 65, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.DS_CPL == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"CPL Qualified Debug Store%.*sDS-CPL   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.DTES64 == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"64-Bit Debug Store%.*sDTES64   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtFeature.EBX.FastStrings == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Fast-String Operation%.*sFast-Strings   [%7s]",
		width - 48, hSpace, powered(bix));

	bix = (Shm->Proc.Features.Std.ECX.FMA == 1)
	   || (Shm->Proc.Features.ExtInfo.ECX.FMA4 == 1);
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Fused Multiply Add%.*sFMA|FMA4   [%7s]",
		width - 41, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtFeature.EBX.HLE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Hardware Lock Elision%.*sHLE   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtInfo.EDX.IA64 == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Long Mode 64 bits%.*sIA64|LM   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtInfo.ECX.LWP == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"LightWeight Profiling%.*sLWP   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.MCA == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Machine-Check Architecture%.*sMCA   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.MSR == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Model Specific Registers%.*sMSR   [%7s]",
		width - 42, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.MTRR == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Memory Type Range Registers%.*sMTRR   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.OSXSAVE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"OS-Enabled Ext. State Management%.*sOSXSAVE   [%7s]",
		width - 54,hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PAE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Physical Address Extension%.*sPAE   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PAT == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Page Attribute Table%.*sPAT   [%7s]",
		width - 38, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PBE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Pending Break Enable%.*sPBE   [%7s]",
		width - 38, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.PCID == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Process Context Identifiers%.*sPCID   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.PDCM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Perfmon and Debug Capability%.*sPDCM   [%7s]",
		width - 47, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PGE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Page Global Enable%.*sPGE   [%7s]",
		width - 36, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PSE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Page Size Extension%.*sPSE   [%7s]",
		width - 37, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PSE36 == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"36-bit Page Size Extension%.*sPSE36   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PSN == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Processor Serial Number%.*sPSN   [%7s]",
		width - 41, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtFeature.EBX.RTM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Restricted Transactional Memory%.*sRTM   [%7s]",
		width - 49, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.SMX == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Safer Mode Extensions%.*sSMX   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.SS == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Self-Snoop%.*sSS   [%7s]",
		width - 27, hSpace, powered(bix));

	printv(OutFunc, SCANKEY_NULL, attrib[Shm->Proc.Features.InvariantTSC],
		width, 2,
		"Time Stamp Counter%.*sTSC [%9s]",
		width - 36, hSpace, TSC[Shm->Proc.Features.InvariantTSC]);

	bix = Shm->Proc.Features.Std.ECX.TSCDEAD == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Time Stamp Counter Deadline%.*sTSC-DEADLINE   [%7s]",
		width - 54,hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.VME == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Virtual Mode Extension%.*sVME   [%7s]",
		width - 40, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.VMX == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Virtual Machine Extensions%.*sVMX   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Cpu[Shm->Proc.Service.Core].Topology.MP.x2APIC > 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Extended xAPIC Support%.*sx2APIC   [%7s]",
		width - 43, hSpace,
		x2APIC[Shm->Cpu[Shm->Proc.Service.Core].Topology.MP.x2APIC]);

	bix = Shm->Proc.Features.ExtInfo.EDX.XD_Bit == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Execution Disable Bit Support%.*sXD-Bit   [%7s]",
		width - 50, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.XSAVE == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"XSAVE/XSTOR States%.*sXSAVE   [%7s]",
		width - 38, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.xTPR == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"xTPR Update Control%.*sxTPR   [%7s]",
		width - 38, hSpace, powered(bix));
}

void SysInfoTech(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[2][50] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,HGK,HGK,HGK,LWK,LWK
	    }
	};
	int bix;
// Section Mark
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Technology.SMM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"System Management Mode%.*sSMM-Dual       [%3s]",
		width - 45, hSpace, enabled(bix));

	bix = Shm->Proc.Features.HyperThreading == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Hyper-Threading%.*sHTT       [%3s]",
		width - 33, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.EIST == 1;
	printv(OutFunc, BOXKEY_EIST, attrib[bix], width, 2,
		"SpeedStep%.*sEIST       <%3s>",
		width - 28, hSpace, enabled(bix));

	bix = Shm->Proc.Features.Power.EAX.TurboIDA == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Dynamic Acceleration%.*sIDA       [%3s]",
		width - 38, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.Turbo == 1;
	printv(OutFunc, BOXKEY_TURBO, attrib[bix], width, 2,
		"Turbo Boost%.*sTURBO       <%3s>",
		width - 31, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.VM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Virtualization%.*sVMX       [%3s]",
		width - 32, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.IOMMU == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 3,
		"I/O MMU%.*sVT-d       [%3s]",
		width - (OutFunc ? 27 : 29), hSpace, enabled(bix));
    }
    else if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    {
	bix = Shm->Proc.Technology.SMM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"System Management Mode%.*sSMM-Lock       [%3s]",
		width - 45, hSpace, enabled(bix));

	bix = Shm->Proc.Features.HyperThreading == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Simultaneous Multithreading%.*sSMT       [%3s]",
		width - 45, hSpace, enabled(bix));

	bix = Shm->Proc.PowerNow == 0b11;	// VID + FID
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"PowerNow!%.*sCnQ       [%3s]",
		width - 27, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.Turbo == 1;
	printv(OutFunc, BOXKEY_TURBO, attrib[bix], width, 2,
		"Core Performance Boost%.*sCPB       <%3s>",
		width - 40, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.VM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Virtualization%.*sSVM       [%3s]",
		width - 32, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.IOMMU == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 3,
		"I/O MMU%.*sAMD-V       [%3s]",
		width - (OutFunc? 28 : 30), hSpace, enabled(bix));
    }
	bix = Shm->Proc.Features.Std.ECX.Hyperv == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 3,
		"Hypervisor%.*s[%3s]",
		width - (OutFunc? 19 : 21), hSpace, enabled(bix));
}

void SysInfoPerfMon(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[4][74] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HGK,
		HGK,HGK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,HBK,HBK,HBK,HBK,HBK,
		HBK,HBK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,
		HGK,HGK,LWK,LWK
	    }
	};
	int bix;
// Section Mark
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Version%.*sPM       [%3d]",
		width - 24, hSpace, Shm->Proc.PM_version);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Counters:%.*sGeneral%.*sFixed",
		10, hSpace, width - 61, hSpace);

    if (OutFunc == NULL) {
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 1,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.EAX.MonCtrs,
				Shm->Proc.Features.PerfMon.EAX.MonWidth,
		11, hSpace,	Shm->Proc.Features.PerfMon.EDX.FixCtrs,
				Shm->Proc.Features.PerfMon.EDX.FixWidth);
    } else {
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 0,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.EAX.MonCtrs,
				Shm->Proc.Features.PerfMon.EAX.MonWidth,
		5, hSpace,	Shm->Proc.Features.PerfMon.EDX.FixCtrs,
				Shm->Proc.Features.PerfMon.EDX.FixWidth);
    }
	bix = Shm->Proc.Technology.C1E == 1;
	printv(OutFunc, BOXKEY_C1E, attrib[bix], width, 2,
		"Enhanced Halt State%.*sC1E       <%3s>",
		width - 37, hSpace, enabled(bix));

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Technology.C1A == 1;
	printv(OutFunc, BOXKEY_C1A, attrib[bix], width, 2,
		"C1 Auto Demotion%.*sC1A       <%3s>",
		width - 34, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.C3A == 1;
	printv(OutFunc, BOXKEY_C3A, attrib[bix], width, 2,
		"C3 Auto Demotion%.*sC3A       <%3s>",
		width - 34, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.C1U == 1;
	printv(OutFunc, BOXKEY_C1U, attrib[bix], width, 2,
		"C1 UnDemotion%.*sC1U       <%3s>",
		width - 31, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.C3U == 1;
	printv(OutFunc, BOXKEY_C3U, attrib[bix], width, 2,
		"C3 UnDemotion%.*sC3U       <%3s>",
		width - 31, hSpace, enabled(bix));
    }
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    {
	bix = Shm->Proc.Technology.CC6 == 1;
	printv(OutFunc, BOXKEY_CC6, attrib[bix], width, 2,
		"Core C6 State%.*sCC6       <%3s>",
		width - 31, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.PC6 == 1;
	printv(OutFunc, BOXKEY_PC6, attrib[bix], width, 2,
		"Package C6 State%.*sPC6       <%3s>",
		width - 34, hSpace, enabled(bix));
    }
	bix = Shm->Proc.Features.AdvPower.EDX.FID == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Frequency ID control%.*sFID       [%3s]",
		width - 38, hSpace, enabled(bix));

	bix = Shm->Proc.Features.AdvPower.EDX.VID == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Voltage ID control%.*sVID       [%3s]",
		width - 36, hSpace, enabled(bix));

	bix = (Shm->Proc.Features.Power.ECX.HCF_Cap == 1)
	   || ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		&& (Shm->Proc.Features.AdvPower.EDX.EffFrqRO == 1));
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"P-State Hardware Coordination Feedback"	\
			"%.*sMPERF/APERF       [%3s]",
		width - 64, hSpace, enabled(bix));

	bix = (Shm->Proc.Features.Power.EAX.HWP_Reg == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1);
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Hardware-Controlled Performance States%.*sHWP       [%3s]",
		width - 56, hSpace, enabled(bix));

	bix = Shm->Proc.Features.Power.EAX.HDC_Reg == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Hardware Duty Cycling%.*sHDC       [%3s]",
		width - 39, hSpace, enabled(bix));

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2, "Package C-State");

	bix = Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock == 0 ? 3 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 3,
		"Configuration Control%.*sCONFIG   [%7s]",
		width - (OutFunc == NULL ? 45 : 43), hSpace,
		!Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock ?
			"UNLOCK" : "LOCK");

	if (!Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock) {
		printv(OutFunc, BOXKEY_PKGCST, attrib[0], width, 3,
			"Lowest C-State%.*sLIMIT   <%7d>",
			width - (OutFunc == NULL ? 37 : 35), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit);

		bix = Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir == 1 ? 3:2;
		printv(OutFunc, BOXKEY_IOMWAIT, attrib[bix], width, 3,
			"I/O MWAIT Redirection%.*sIOMWAIT   <%7s>",
			width - (OutFunc == NULL ? 46 : 44), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir ?
				" ENABLE" : "DISABLE");

		printv(OutFunc, BOXKEY_IORCST, attrib[0], width, 3,
			"Max C-State Inclusion%.*sRANGE   <%7d>",
			width - (OutFunc == NULL ? 44 : 42), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude);
	} else {
		printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
			"Lowest C-State%.*sLIMIT   [%7d]",
			width - (OutFunc == NULL ? 37 : 35), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit);

		printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
			"I/O MWAIT Redirection%.*sIOMWAIT   [%7s]",
			width - (OutFunc == NULL ? 46 : 44), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir ?
				" ENABLE" : "DISABLE");

		printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
			"Max C-State Inclusion%.*sRANGE   [%7d]",
			width - (OutFunc == NULL ? 44 : 42), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude);
	}
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"MWAIT States:%.*sC0      C1      C2      C3      C4",
		06, hSpace);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, (OutFunc == NULL) ? 1:0,
		"%.*s%2d      %2d      %2d      %2d      %2d",
		21, hSpace,
		Shm->Proc.Features.MWait.EDX.Num_C0_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C1_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C2_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C3_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C4_MWAIT);

	bix = Shm->Proc.Features.PerfMon.EBX.CoreCycles == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Core Cycles%.*s[%7s]",
		width - 23, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.InstrRetired == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Instructions Retired%.*s[%7s]",
		width - 32, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.RefCycles == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Reference Cycles%.*s[%7s]",
		width - 28, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.LLC_Ref == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Last Level Cache References%.*s[%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.LLC_Misses == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Last Level Cache Misses%.*s[%7s]",
		width - 35, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.BranchRetired == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Branch Instructions Retired%.*s[%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.BranchMispred == 0 ? 2 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Branch Mispredicts Retired%.*s[%7s]",
		width - 38, hSpace, powered(bix));
}

void SysInfoPwrThermal(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[4][50] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LWK,LWK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,LWK,LWK
	    }
	};
	const char *TM[] = {
		"Missing",
		"Present",
		"Disable",
		" Enable",
	}, *Unlock[] = {
		"  LOCK",
		"UNLOCK",
	};
	int bix;
// Section Mark
	bix = Shm->Proc.Technology.ODCM == 1 ? 3 : 1;
	printv(OutFunc, BOXKEY_ODCM, attrib[bix], width, 2,
		"Clock Modulation%.*sODCM   <%7s>", width - 35, hSpace,
		Shm->Proc.Technology.ODCM ? " Enable" : "Disable");

	printv(OutFunc, BOXKEY_DUTYCYCLE, attrib[0], width, 3,
	"DutyCycle%.*s<%6.2f%%>", width - (OutFunc == NULL ? 24: 22), hSpace,
	(Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.Extended ?
		6.25f : 12.5f
	* Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod));

	bix = Shm->Proc.Technology.PowerMgmt == 1 ? 3 : 0;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Power Management%.*sPWR MGMT   [%7s]",
		width - 39, hSpace, Unlock[Shm->Proc.Technology.PowerMgmt]);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
		"Energy Policy%.*sBias Hint   [%7u]",
		width - (OutFunc == NULL ? 40 : 38), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.PowerPolicy);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Junction Temperature%.*sTjMax   [%7u]", width - 40, hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.Target);

	bix = (Shm->Proc.Features.Power.EAX.DTS == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.TS == 1);
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Digital Thermal Sensor%.*sDTS   [%7s]",
		width - 40, hSpace, powered(bix));

	bix = Shm->Proc.Features.Power.EAX.PLN == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Power Limit Notification%.*sPLN   [%7s]",
		width - 42, hSpace, powered(bix));

	bix = Shm->Proc.Features.Power.EAX.PTM == 1;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Package Thermal Management%.*sPTM   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1
	    | Shm->Proc.Features.AdvPower.EDX.TTP;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Thermal Monitor 1%.*sTM1|TTP   [%7s]",
		width - 39, hSpace,
		TM[  Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1
			| Shm->Proc.Features.AdvPower.EDX.TTP ]);

	bix = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2
	    | Shm->Proc.Features.AdvPower.EDX.TM;
	printv(OutFunc, SCANKEY_NULL, attrib[bix], width, 2,
		"Thermal Monitor 2%.*sTM2|HTC   [%7s]",
		width - 39, hSpace,
		TM[  Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2
			| Shm->Proc.Features.AdvPower.EDX.TM ]);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2, "Units");

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
		"Power%.*swatt   [%13.9f]",
		width - (OutFunc == NULL ? 33 : 31), hSpace,
		Shm->Proc.Power.Unit.Watts);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
		"Energy%.*sjoule   [%13.9f]",
		width - (OutFunc == NULL ? 35 : 33), hSpace,
		Shm->Proc.Power.Unit.Joules);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3,
		"Window%.*ssecond   [%13.9f]",
		width - (OutFunc == NULL ? 36 : 34), hSpace,
		Shm->Proc.Power.Unit.Times);
}

void SysInfoKernel(SHM_STRUCT *Shm, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[1][76] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK
	    }
	};
	size_t	len = 0, sln;
	char	*row = malloc(width + 1),
		*str = malloc(width + 1);
	int	idx = 0;
// Section Mark
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 0,
		"%s:", Shm->SysGate.sysname);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Release%.*s[%s]", width - 12 - strlen(Shm->SysGate.release),
		hSpace, Shm->SysGate.release);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Version%.*s[%s]", width - 12 - strlen(Shm->SysGate.version),
		hSpace, Shm->SysGate.version);

	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Machine%.*s[%s]", width - 12 - strlen(Shm->SysGate.machine),
		hSpace, Shm->SysGate.machine);
// Section Mark
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 0,
		"Memory:%.*s", width - 7, hSpace);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.totalram);
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Total RAM" "%.*s" "%s KB", width - 15 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.sharedram);
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Shared RAM" "%.*s" "%s KB", width - 16 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.freeram);
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Free RAM" "%.*s" "%s KB", width - 14 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.bufferram);
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Buffer RAM" "%.*s" "%s KB", width - 16 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.totalhigh);
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Total High" "%.*s" "%s KB", width - 16 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.freehigh);
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 2,
		"Free High" "%.*s" "%s KB", width - 15 - len, hSpace, str);
// Section Mark
  if ((len = strlen(Shm->SysGate.IdleDriver.Name)
		+ strlen(Shm->SysGate.IdleDriver.Governor)) > 0)
  {
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 0,
		"Idle driver%.*s[%s@%s]", width - 14 - len, hSpace,
		Shm->SysGate.IdleDriver.Governor, Shm->SysGate.IdleDriver.Name);
// Row Mark
	len = sprintf(row, "States:%.*s", 9, hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
					idx++, len += sln, strcat(row, str))
    {
	sln = sprintf(str, "%-8s", Shm->SysGate.IdleDriver.State[idx].Name);
    }
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3, row);
// Row Mark
	len = sprintf(row, "Power:%.*s", 10, hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
					idx++, len += sln, strcat(row, str))
    {
	sln=sprintf(str,"%-8d",Shm->SysGate.IdleDriver.State[idx].powerUsage);
    }
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3, row);
// Row Mark
	len = sprintf(row, "Latency:%.*s", 8, hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
				idx++, len += sln, strcat(row, str))
    {
	sln=sprintf(str,"%-8u",Shm->SysGate.IdleDriver.State[idx].exitLatency);
    }
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3, row);
// Row Mark
	len = sprintf(row, "Residency:%.*s", 6, hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
					idx++, len += sln, strcat(row, str))
    {
    sln=sprintf(str,"%-8u",Shm->SysGate.IdleDriver.State[idx].targetResidency);
    }
	printv(OutFunc, SCANKEY_NULL, attrib[0], width, 3, row);
  }
	free(row);
	free(str);
}

void Package(SHM_STRUCT *Shm)
{
    while (!BITVAL(Shutdown, 0)) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !BITVAL(Shutdown, 0))
		nanosleep(&Shm->Sleep.pollingWait, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

	ClientFollowService(&localService, &Shm->Proc.Service, 0);

	struct PKG_FLIP_FLOP *Pkg = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	printf( "\t\t" "Cycles" "\t\t" "State(%%)" "\n"	\
		"PC02" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC03" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC06" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC07" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC08" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC09" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC10" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PTSC" "\t" "%18llu" "\n"		\
		"UNCORE" "\t" "%18llu" "\n\n",
		Pkg->Delta.PC02, 100.f * Shm->Proc.State.PC02,
		Pkg->Delta.PC03, 100.f * Shm->Proc.State.PC03,
		Pkg->Delta.PC06, 100.f * Shm->Proc.State.PC06,
		Pkg->Delta.PC07, 100.f * Shm->Proc.State.PC07,
		Pkg->Delta.PC08, 100.f * Shm->Proc.State.PC08,
		Pkg->Delta.PC09, 100.f * Shm->Proc.State.PC09,
		Pkg->Delta.PC10, 100.f * Shm->Proc.State.PC10,
		Pkg->Delta.PTSC,
		Pkg->Uncore.FC0);
    }
}

void Counters(SHM_STRUCT *Shm)
{
    unsigned int cpu = 0;
    while (!BITVAL(Shutdown, 0)) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !BITVAL(Shutdown, 0))
		nanosleep(&Shm->Sleep.pollingWait, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

	ClientFollowService(&localService, &Shm->Proc.Service, 0);

		printf("CPU Freq(MHz) Ratio  Turbo"			\
			"  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
			"  Min TMP:TS  Max\n");
	for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown,0); cpu++)
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW)) {
	    struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printf("#%02u %7.2f (%5.2f)"				\
			" %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"		\
			"  %-3u/%3u:%-3u/%3u\n",
			cpu,
			Flop->Relative.Freq,
			Flop->Relative.Ratio,
			100.f * Flop->State.Turbo,
			100.f * Flop->State.C0,
			100.f * Flop->State.C1,
			100.f * Flop->State.C3,
			100.f * Flop->State.C6,
			100.f * Flop->State.C7,
			Shm->Cpu[cpu].PowerThermal.Limit[0],
			Flop->Thermal.Temp,
			Flop->Thermal.Sensor,
			Shm->Cpu[cpu].PowerThermal.Limit[1]);
	    else
		printf("#%02u        OFF\n", cpu);

	  }
		printf("\n"						\
		"%.*s" "Averages:"					\
		"%.*s" "Turbo  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
		"%.*s" "TjMax:\n"					\
		"%.*s" "%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"		\
		"%.*s" "%3u C\n\n",
			4, hSpace,
			8, hSpace,
			7, hSpace,
			20, hSpace,
			100.f * Shm->Proc.Avg.Turbo,
			100.f * Shm->Proc.Avg.C0,
			100.f * Shm->Proc.Avg.C1,
			100.f * Shm->Proc.Avg.C3,
			100.f * Shm->Proc.Avg.C6,
			100.f * Shm->Proc.Avg.C7,
			8, hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.Target);
    }
}

void Voltage(SHM_STRUCT *Shm)
{
    enum PWR_DOMAIN pw;
    unsigned int cpu = 0;
    while (!BITVAL(Shutdown, 0)) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !BITVAL(Shutdown, 0))
		nanosleep(&Shm->Sleep.pollingWait, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

	ClientFollowService(&localService, &Shm->Proc.Service, 0);

		printf("CPU Freq(MHz) VID  Vcore\n");
	for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown,0); cpu++)
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW)) {
	    struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printf("#%02u %7.2f %5d  %5.4f\n",
			cpu,
			Flop->Relative.Freq,
			Flop->Voltage.VID,
			Flop->Voltage.Vcore);
	    else
		printf("#%02u        OFF\n", cpu);
	  }

	printf("\n" "%.*sPackage%.*sCores%.*sUncore%.*sMemory",
		14, hSpace, 8, hSpace, 10, hSpace, 9, hSpace);

	printf("\n" "Energy(J):");
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
		printf("%.*s" "%13.9f", 2, hSpace, Shm->Proc.State.Energy[pw]);

	printf("\n" "Power(W) :");
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
		printf("%.*s" "%13.9f", 2, hSpace, Shm->Proc.State.Power[pw]);

	printf("\n\n");
    }
}

void Instructions(SHM_STRUCT *Shm)
{
	unsigned int cpu = 0;

    while (!BITVAL(Shutdown, 0)) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !BITVAL(Shutdown, 0))
		nanosleep(&Shm->Sleep.pollingWait, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

	ClientFollowService(&localService, &Shm->Proc.Service, 0);

		printf("CPU     IPS            IPC            CPI\n");

	for (cpu=0; (cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown,0); cpu++)
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW)) {
		struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printf("#%02u %12.6f/s %12.6f/c %12.6f/i\n",
			cpu,
			Flop->State.IPS,
			Flop->State.IPC,
			Flop->State.CPI);
	    else
		printf("#%02u\n", cpu);
	  }
		printf("\n");
	}
}

void Topology(SHM_STRUCT *Shm, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[3][13] = {
	    {
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,
		HWK,HWK,HWK
	    },{
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,
		HBK,HBK,HBK
	    },{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK
	    }
	};
	ASCII item[16];
	unsigned int cpu = 0, level = 0, nl = 6;

	void printm(ATTRIBUTE *attrib, char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf((char *) item, fmt, ap);
		if (OutFunc == NULL)
			if (!--nl) {
				nl = 6;
				printf("%s\n", item);
			}
			else
				printf("%s", item);
		else
			OutFunc(SCANKEY_NULL, attrib, item);
		va_end(ap);
	}

	printm(attrib[2], "CPU Pkg  Apic"); printm(attrib[2], "  Core Thread");
	printm(attrib[2], "  Caches     "); printm(attrib[2], " (w)rite-Back");
	printm(attrib[2], " (i)nclusive "); printm(attrib[2], "             ");
	printm(attrib[2], " #   ID   ID "); printm(attrib[2], "   ID     ID ");
	printm(attrib[2], " L1-Inst Way "); printm(attrib[2], " L1-Data Way ");
	printm(attrib[2], "     L2  Way "); printm(attrib[2], "     L3  Way ");

	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
		if (Shm->Cpu[cpu].Topology.MP.BSP)
			printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
				"%02u: BSP%6d",
				cpu,
				Shm->Cpu[cpu].Topology.ApicID);
		else
			printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
				"%02u:%4d%6d",
				cpu,
				Shm->Cpu[cpu].Topology.PackageID,
				Shm->Cpu[cpu].Topology.ApicID);

		printm(attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
			"%6d %6d",
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);

	  for (level = 0; level < CACHE_MAX_LEVEL; level++)
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printm(attrib[0], "%8u%3u%c%c",
			Shm->Cpu[cpu].Topology.Cache[level].Size,
			Shm->Cpu[cpu].Topology.Cache[level].Way,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack ?
				'w' : 0x20,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive ?
				'i' : 0x20);
	    else
		printm(attrib[1], "       -  -  ");
	}
}

void MemoryController(SHM_STRUCT *Shm, CELL_FUNC OutFunc)
{
	ATTRIBUTE attrib[2][5] = {
	    {
		LWK,LWK,LWK,LWK,LWK
	    },{
		HWK,HWK,HWK,HWK,HWK
	    }
	};
	ASCII item[8];
	char fInt[16], hInt[2][8];
	unsigned int nl = 14;
	unsigned short mc, cha, slot;

	void printm(ATTRIBUTE *attrib, char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf((char *) item, fmt, ap);
		if (OutFunc == NULL)
			if (!--nl) {
				nl = 14;
				printf("%s\n", item);
			}
			else
				printf("%s", item);
		else
			OutFunc(SCANKEY_NULL, attrib, item);
		va_end(ap);
	}

	void iSplit(unsigned int sInt) {
		sprintf(fInt, "%10u", sInt);
		strncpy(hInt[0], &fInt[0], 5); hInt[0][5] = '\0';
		strncpy(hInt[1], &fInt[5], 5); hInt[1][5] = '\0';
	}

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {
	printm(attrib[0], "Contr");	printm(attrib[0], "oller");
	printm(attrib[1], " #%-3u",mc); printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");

	switch (Shm->Uncore.MC[mc].ChannelCount) {
	case 1:
		printm(attrib[1], "Singl");
		printm(attrib[1], "e Cha");
		printm(attrib[1], "nnel ");
		break;
	case 2:
		printm(attrib[1], " Dual");
		printm(attrib[1], " Chan");
		printm(attrib[1], "nel  ");
		break;
	case 3:
		printm(attrib[1], "Tripl");
		printm(attrib[1], "e Cha");
		printm(attrib[1], "nnel ");
		break;
	case 4:
		printm(attrib[1], " Quad");
		printm(attrib[1], " Chan");
		printm(attrib[1], "nel  ");
		break;
	case 6:
		printm(attrib[1], "  Six");
		printm(attrib[1], " Chan");
		printm(attrib[1], "nel  ");
		break;
	case 8:
		printm(attrib[1], "Eight");
		printm(attrib[1], " Chan");
		printm(attrib[1], "nel  ");
		break;
	default:
		printm(attrib[0], "     ");
		printm(attrib[0], "     ");
		printm(attrib[0], "     ");
		break;
	}

	printm(attrib[0], " Bus ");	printm(attrib[0], "Rate ");
	printm(attrib[1], "%5llu", Shm->Uncore.Bus.Rate);

	switch (Shm->Uncore.Unit.Bus_Rate) {
	case 0b00:
		printm(attrib[0], " MHz ");
		break;
	case 0b01:
		printm(attrib[0], " MT/s");
		break;
	case 0b10:
		printm(attrib[0], " MB/s");
		break;
	case 0b11:
		printm(attrib[0], "     ");
		break;
	}
	printm(attrib[0], "     ");

	printm(attrib[0], " Bus ");	printm(attrib[0], "Speed");
	printm(attrib[1], "%5llu", Shm->Uncore.Bus.Speed);

	switch (Shm->Uncore.Unit.BusSpeed) {
	case 0b00:
		printm(attrib[0], " MHz ");
		break;
	case 0b01:
		printm(attrib[0], " MT/s");
		break;
	case 0b10:
		printm(attrib[0], " MB/s");
		break;
	case 0b11:
		printm(attrib[0], "     ");
		break;
	}
	printm(attrib[0], "     ");

	printm(attrib[0], "DRAM "); printm(attrib[0], "Speed");
	printm(attrib[1], "%5llu", Shm->Uncore.CtrlSpeed);

	switch (Shm->Uncore.Unit.DDRSpeed) {
	case 0b00:
		printm(attrib[0], " MHz ");
		break;
	case 0b01:
		printm(attrib[0], " MT/s");
		break;
	case 0b10:
		printm(attrib[0], " MB/s");
		break;
	case 0b11:
		printm(attrib[0], "     ");
		break;
	}

	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");

	printm(attrib[0], " Cha ");
	printm(attrib[0], "   CL");	printm(attrib[0], "  RCD");
	printm(attrib[0], "   RP");	printm(attrib[0], "  RAS");
	printm(attrib[0], "  RRD");	printm(attrib[0], "  RFC");
	printm(attrib[0], "   WR");	printm(attrib[0], " RTPr");
	printm(attrib[0], " WTPr");	printm(attrib[0], "  FAW");
	printm(attrib[0], "  B2B");	printm(attrib[0], "  CWL");
	printm(attrib[0], " Rate");

    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	printm(attrib[1], "\x20\x20#%-2u", cha);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tCL);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRP);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tWR);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.B2B);
	printm(attrib[1], "%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL);
	printm(attrib[1], "%4uN",
			Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);
    }
	printm(attrib[0], "     ");
	printm(attrib[0], " ddWR");	printm(attrib[0], " drWR");
	printm(attrib[0], " srWR");	printm(attrib[0], " ddRW");
	printm(attrib[0], " drRW");	printm(attrib[0], " srRW");
	printm(attrib[0], " ddRR");	printm(attrib[0], " drRR");
	printm(attrib[0], " srRR");	printm(attrib[0], " ddWW");
	printm(attrib[0], " drWW");	printm(attrib[0], " srWW");
	printm(attrib[0], "  ECC");

    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	printm(attrib[1],"\x20\x20#%-2u", cha);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd);

	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr);

	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd);

	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr);
	printm(attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr);

	printm(attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.ECC);
    }
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");
	printm(attrib[0], "     ");	printm(attrib[0], "     ");

	for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
		printm(attrib[0], " DIMM"); printm(attrib[0], " Geom");
		printm(attrib[0], "etry "); printm(attrib[0], "for c");
		printm(attrib[0], "hanne"); printm(attrib[0], "l #%-2u", cha);
		printm(attrib[0], "     "); printm(attrib[0], "     ");
		printm(attrib[0], "     "); printm(attrib[0], "     ");
		printm(attrib[0], "     "); printm(attrib[0], "     ");
		printm(attrib[0], "     "); printm(attrib[0], "     ");

		printm(attrib[0], "     "); printm(attrib[0], " Slot");
		printm(attrib[0], " Bank"); printm(attrib[0], " Rank");
		printm(attrib[0], "     "); printm(attrib[0], "Rows ");
		printm(attrib[0], "  Col"); printm(attrib[0], "umns ");
		printm(attrib[0], "   Me"); printm(attrib[0], "mory ");
		printm(attrib[0], "Size "); printm(attrib[0], "(MB) ");
		printm(attrib[0], "     "); printm(attrib[0], "     ");

	    for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		printm(attrib[0], "     ");
		printm(attrib[1], "\x20\x20#%-2u", slot);
		printm(attrib[1], "%5u",
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks);
		printm(attrib[1], "%5u",
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows);
		printm(attrib[1], "%5s", hInt[0]);
		printm(attrib[1], "%5s", hInt[1]);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols);
		printm(attrib[1], "%5s", hInt[0]);
		printm(attrib[1], "%5s", hInt[1]);
		printm(attrib[0], "     ");
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size);
		printm(attrib[1], "%5s", hInt[0]);
		printm(attrib[1], "%5s", hInt[1]);
		printm(attrib[0], "     ");
		printm(attrib[0], "     ");
		printm(attrib[0], "     ");
	    }
	}
    }
}

int ByteReDim(unsigned long ival, int constraint, unsigned long *oval)
{
	int base = 1 + (int) log10(ival);

	(*oval) = ival;
	if (base > constraint) {
		(*oval) = (*oval) >> 10;
		return(1 + ByteReDim((*oval), constraint, oval));
	} else
		return(0);
}

Card *CreateCard(void)
{
	Card *card = calloc(1, sizeof(Card));
	if (card != NULL) {
		card->next = NULL;
	}
	return(card);
}

void AppendCard(Card *card, CardList *list)
{
	if (card != NULL) {
		if (list->head == NULL) {
			list->head = list->tail = card;
		} else {
			list->tail->next = card;
			list->tail = card;
		}
	}
}

void DestroyAllCards(CardList *list)
{
	Card *card = list->head;
	while (card != NULL) {
		Card *next = card->next;
		free(card);
		card = next;
	}
	list->head = list->tail = NULL;
}


void Top(SHM_STRUCT *Shm, char option)
{
/*
           SCREEN
 __________________________
|           MENU           |
|                       T  |
|  L       HEADER          |
|                       R  |
|--E ----------------------|
|                       A  |
|  A        LOAD           |
|                       I  |
|--D ----------------------|
|                       L  |
|  I       MONITOR         |
|                       I  |
|--N ----------------------|
|                       N  |
|  G       FOOTER          |
|                       G  |
`__________________________'
*/

    CardList	cardList = {.head = NULL, .tail = NULL};

    struct {
	struct {
	unsigned int
		layout	:  1-0,		// Draw layout
		clear	:  2-1,		// Clear screen
		height	:  3-2,		// Valid height
		width	:  4-3,		// Valid width
		daemon	:  5-4,		// Draw dynamic
		taskVal :  6-5,		// Display task's value
		avgOrPC :  7-6,		// C-states average || % pkg states
		clkOrLd :  8-7,		// Relative freq. || % load
		_padding: 32-8;
	};
	enum VIEW view;
	enum DISPOSAL disposal;
    } drawFlag = {
	.layout = 0,
	.clear	= 0,
	.height = 0,
	.width	= 0,
	.daemon = 0,
	.taskVal= 0,
	.avgOrPC= 0,
	.clkOrLd= 0,
	.view	= V_FREQ,
	.disposal= (option == 'd') ? D_DASHBOARD : D_MAINVIEW
    };

    SCREEN_SIZE drawSize = {.width = 0, .height = 0};

    double prevTopFreq = 0.0, prevTopLoad = 0.0;
    unsigned long prevFreeRAM = 0;
    unsigned int cpu = 0, cpuScroll = 0, digit[9], iClock = 0, ratioCount = 0;
    int prevTaskCount = 0;

    CUINT loadWidth = 0, MIN_HEIGHT = 0, MAX_ROWS = 0;

    HBCLK *hBClk;

    char *buffer = NULL;

    Coordinate *cTask;

    double minRatio, maxRatio, medianRatio, availRatio[BOOST(SIZE)];

    void SortAvailableRatio()
    {
	int AscendingSort(const void *p1, const void *p2)
	{
		const double *r1 = p1, *r2 = p2;
		int sort = (*r1) < (*r2) ? -1 : (*r1) == (*r2) ? 0 : +1;
		return(sort);
	}

	unsigned int rdx;
	minRatio = Shm->Proc.Boost[BOOST(MIN)];
	maxRatio = Shm->Proc.Boost[BOOST(MIN)];
	ratioCount = 0;
	memset(availRatio, 0, BOOST(SIZE) * sizeof(double));
	for (rdx = BOOST(MIN); rdx < BOOST(SIZE); rdx++)
	    if (Shm->Proc.Boost[rdx] != 0) {
		unsigned int idx, unik = 1;
		for (idx = 0; (idx < ratioCount) && unik; idx++) {
		    if (Shm->Proc.Boost[rdx] == availRatio[idx])
			unik = 0;	// ignore duplicates.
		}
		if (unik) {
			maxRatio=CUMAX(maxRatio, Shm->Proc.Boost[rdx]);
			// Update the ruller tab ratios.
			availRatio[ratioCount++] = Shm->Proc.Boost[rdx];
		}
	    }
	medianRatio = (Shm->Proc.Boost[BOOST(ACT)] > 0) ?
			Shm->Proc.Boost[BOOST(ACT)] : (minRatio + maxRatio) / 2;

	qsort(availRatio, ratioCount, sizeof(*availRatio), AscendingSort);
    }

    void HookCardFunc(CARDFUNC *with, CARDFUNC what) { *with=what; }

#define StoreCard(card, with, what)					\
(									\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(card->hook with), typeof(CARDFUNC)), HookCardFunc,	\
    (void)0)							\
	(&(card->hook with), what)					\
)

#define Threshold(value, threshold1, threshold2, _low, _medium, _high)	\
({									\
	enum PALETTE _ret;						\
	if (value > threshold2)						\
		_ret = _high;						\
	else if (value > threshold1)					\
		_ret = _medium;						\
	else								\
		_ret = _low;						\
	_ret;								\
})

#define frtostr(r, d, pStr)						\
({									\
	int p = d - ((int) log10(fabs(r))) - 2;				\
	sprintf(pStr, "%*.*f", d, p, r);				\
	pStr;								\
})

#define Clock2LCD(layer, col, row, value1, value2)			\
({									\
	sprintf(buffer, "%04.0f", value1);				\
	PrintLCD(layer, col, row, 4, buffer,				\
	    Threshold(value2,minRatio,medianRatio,_GREEN,_YELLOW,_RED));\
})

#define Counter2LCD(layer, col, row, value)				\
({									\
	sprintf(buffer, "%04.0f", value);				\
	PrintLCD(layer, col, row, 4, buffer,				\
		Threshold(value, 0.f, 1.f, _RED,_YELLOW,_WHITE));	\
})

#define Load2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/3.f, 100.f/1.5f, _WHITE,_YELLOW,_RED))

#define Idle2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/3.f, 100.f/1.5f, _YELLOW,_WHITE,_GREEN))

#define Sys2LCD(layer, col, row, value)					\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/6.6f, 50.0, _RED,_YELLOW,_WHITE))

#define EraseTCell_Menu(win)						\
    ({									\
	CoordShift shift = {						\
		.horz = win->matrix.scroll.horz + win->matrix.select.col,\
		.vert = win->matrix.scroll.vert + row			\
	};								\
	Coordinate cell = {						\
		.col =	(win->matrix.origin.col				\
			+ (win->matrix.select.col			\
			* TCellAt(win, shift.horz, shift.vert).length)),\
			(win->matrix.origin.row + row),			\
		.row =	win->matrix.origin.row + row			\
	};								\
	memset(&LayerAt(win->layer, attr, cell.col, cell.row), 0,	\
		TCellAt(win, shift.horz, shift.vert).length);		\
	memset(&LayerAt(win->layer, code, cell.col, cell.row), 0,	\
		TCellAt(win, shift.horz, shift.vert).length);		\
    })

    void ForEachCellPrint_Menu(Window *win, void *plist)
    {
	WinList *list = (WinList *) plist;
	CUINT col, row;
	size_t len;

	if (win->lazyComp.rowLen == 0)
	  for (col = 0; col < win->matrix.size.wth; col++)
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;

	if (win->matrix.origin.col > 0)
		LayerFillAt(	win->layer,
				0,
				win->matrix.origin.row,
				win->matrix.origin.col, hSpace,
				win->hook.color[0].title);

	for (col = 0; col < win->matrix.size.wth; col++)
		PrintContent(win, list, col, 0);
	for (row = 1; row < win->matrix.size.hth; row++)
	    if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			PrintContent(win, list, win->matrix.select.col, row);

	if((len=drawSize.width-win->lazyComp.rowLen-win->matrix.origin.col) > 0)
		LayerFillAt(	win->layer,
				(win->matrix.origin.col + win->lazyComp.rowLen),
				win->matrix.origin.row,
				len, hSpace,
				win->hook.color[0].title);
    }

    void ForEachCellPrint_Drop(Window *win, void *plist)
    {
	WinList *list = (WinList *) plist;
	CUINT col, row;

	if (win->lazyComp.rowLen == 0)
	  for (col = 0; col < win->matrix.size.wth; col++)
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;

	for (row = 0; row < win->matrix.size.hth; row++)
	    if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			PrintContent(win, list, win->matrix.select.col, row);
    }

    int MotionEnter_Cell(SCANKEY *scan, Window *win)
    {
	if ((scan->key = TCellAt(win,
				( win->matrix.select.col
				+ win->matrix.scroll.horz),
				( win->matrix.select.row
				+ win->matrix.scroll.vert)
				).quick.key) != SCANKEY_NULL) {
					SCANKEY closeKey = {.key = SCANKEY_ESC};
					Motion_Trigger(&closeKey, win,&winList);
					return(1);
				} else
					return(0);
    }

    void MotionEnd_Cell(Window *win)
    {
	win->matrix.scroll.vert = win->lazyComp.bottomRow;
	win->matrix.select.row  = win->matrix.size.hth - 1;
    }

    void MotionLeft_Menu(Window *win)
    {
	CUINT row;
	for (row = 1; row < win->matrix.size.hth; row++)
		EraseTCell_Menu(win);

	if (win->matrix.select.col > 0)
		win->matrix.select.col--;
	else
		win->matrix.select.col = win->matrix.size.wth - 1;

	win->matrix.select.row = 0;
    }

    void MotionRight_Menu(Window *win)
    {
	CUINT row;
	for (row = 1; row < win->matrix.size.hth; row++)
		EraseTCell_Menu(win);

	if (win->matrix.select.col < win->matrix.size.wth - 1)
		win->matrix.select.col++;
	else
		win->matrix.select.col = 0;

	win->matrix.select.row = 0;
    }

    void MotionUp_Menu(Window *win)
    {
	CUINT row = win->matrix.select.row;

	if (win->matrix.select.row > 0)
		row--;

	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			win->matrix.select.row = row;
    }

    void MotionDown_Menu(Window *win)
    {
	CUINT row = win->matrix.select.row;

	if (row < win->matrix.size.hth - 1)
		row++;

	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			win->matrix.select.row = row;
    }

    void MotionHome_Menu(Window *win)
    {
	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + 1)).quick.key != SCANKEY_VOID)
			win->matrix.select.row = 1;
	else
			win->matrix.select.row = 0;
    }

    void MotionEnd_Menu(Window *win)
    {
	CUINT row = 0;
	for (row = win->matrix.size.hth - 1; row > 1; row--)
	    if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			break;

	win->matrix.select.row = row;
    }

    Window *CreateMenu(unsigned long long id)
    {
      Window *wMenu = CreateWindow(wLayer, id, 3, 12, 3, 0);
      if (wMenu != NULL) {
	ATTRIBUTE sameAttr = {.fg = BLACK, .bg = WHITE, .bf = 0},
		voidAttr = {.value = 0}, gateAttr[24], ctrlAttr[24],
		stopAttr[24] = {
			HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,
			HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW
		},
		fkeyAttr[24] = {
			LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW, LKW,LKW,LKW,LKW,
			LKW,LKW,LKW,LKW,LKW,LKW,LKW,HKW,_LKW,_LKW,HKW,LKW
		},
		skeyAttr[24] = {
			LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW, LKW,LKW,LKW,
			LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,HKW,_LKW,HKW,LKW
		};

	memcpy(gateAttr, BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) ?
			skeyAttr : stopAttr, 24);
	memcpy(ctrlAttr, (Shm->Uncore.CtrlCount > 0) ? skeyAttr : stopAttr, 24);

	StoreTCell(wMenu, SCANKEY_NULL,   "          Menu          ", sameAttr);
	StoreTCell(wMenu, SCANKEY_NULL,   "          View          ", sameAttr);
	StoreTCell(wMenu, SCANKEY_NULL,   "         Window         ", sameAttr);

	StoreTCell(wMenu, SCANKEY_s,      " Settings           [s] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_d,      " Dashboard          [d] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_p,      " Processor          [p] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_k,      " Kernel Data        [k] ", gateAttr);
	StoreTCell(wMenu, SCANKEY_f,      " Frequency          [f] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_m,      " Topology           [m] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_HASH,   " HotPlug CPU        [#] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_i,      " Inst cycles        [i] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_e,      " Features           [e] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_F3,     " Tools             [F3] ", fkeyAttr);
	StoreTCell(wMenu, SCANKEY_c,      " Core cycles        [c] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_SHIFT_i," ISA Extensions     [I] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_a,      " About              [a] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_l,      " Idle C-States      [l] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_t,      " Technologies       [t] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_h,      " Help               [h] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_g,      " Package cycles     [g] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_o,      " Perf. Monitoring   [o] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_F1,     " Keys              [F1] ", fkeyAttr);
	StoreTCell(wMenu, SCANKEY_x,      " Tasks Monitoring   [x] ", gateAttr);
	StoreTCell(wMenu, SCANKEY_w,      " Power & Thermal    [w] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_F4,     " Quit              [F4] ", fkeyAttr);
	StoreTCell(wMenu, SCANKEY_q,      " System Interrupts  [q] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_u,      " CPUID Hexa Dump    [u] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_VOID,   "", voidAttr);
	StoreTCell(wMenu, SCANKEY_SHIFT_v," Power & Voltage    [V] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_SHIFT_r," System Registers   [R] ", skeyAttr);

	StoreTCell(wMenu, SCANKEY_VOID,   "", voidAttr);
	StoreTCell(wMenu, SCANKEY_SHIFT_t," Slice counters     [T] ", skeyAttr);
	StoreTCell(wMenu, SCANKEY_SHIFT_m," Memory Controller  [M] ", ctrlAttr);

	StoreTCell(wMenu, SCANKEY_VOID,   "", voidAttr);
	StoreTCell(wMenu, SCANKEY_VOID,   "", voidAttr);
	StoreTCell(wMenu, SCANKEY_VOID,   "", voidAttr);

	StoreWindow(wMenu, .color[0].select,	MakeAttr(BLACK, 0, WHITE, 0));
	StoreWindow(wMenu, .color[0].title,	MakeAttr(BLACK, 0, WHITE, 0));
	StoreWindow(wMenu, .color[1].title,	MakeAttr(BLACK, 0, WHITE, 1));

	StoreWindow(wMenu,	.Print,		ForEachCellPrint_Menu);
	StoreWindow(wMenu,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wMenu,	.key.Left,	MotionLeft_Menu);
	StoreWindow(wMenu,	.key.Right,	MotionRight_Menu);
	StoreWindow(wMenu,	.key.Down,	MotionDown_Menu);
	StoreWindow(wMenu,	.key.Up,	MotionUp_Menu);
	StoreWindow(wMenu,	.key.Home,	MotionHome_Menu);
	StoreWindow(wMenu,	.key.End,	MotionEnd_Menu);
      }
      return(wMenu);
    }

    Window *CreateSettings(unsigned long long id)
    {
      Window *wSet = CreateWindow(wLayer, id, 1, 13,
				8, (TOP_HEADER_ROW + 13 + 3 < drawSize.height) ?
					TOP_HEADER_ROW + 3 : 1);
      if (wSet != NULL) {
	char	intervStr[16], tickStr[16], pollStr[16], ringStr[16],
		childStr[16], sliceStr[16],
		experStr[16], cpuhpStr[16], pciRegStr[16], nmiRegStr[16];
	int intervLen = sprintf(intervStr,"       <%4u>E6",Shm->Sleep.Interval),
	    tickLen = sprintf(tickStr, "%13uE6",
				Shm->Sleep.Interval * Shm->SysGate.tickReset),
	    pollLen = sprintf(pollStr, "%13ldE6",
				Shm->Sleep.pollingWait.tv_nsec / 1000000L),
	    ringLen = sprintf(ringStr, "%13ldE6",
				Shm->Sleep.ringWaiting.tv_nsec / 1000000L),
	    childLen = sprintf(childStr, "%13ldE6",
				Shm->Sleep.childWaiting.tv_nsec / 1000000L),
	    sliceLen = sprintf(sliceStr, "%13ldE6",
				Shm->Sleep.sliceWaiting.tv_nsec / 1000000L),
	    experLen = sprintf(experStr, "<%3s>",
				enabled((Shm->Registration.Experimental != 0))),
	    cpuhpLen = sprintf(cpuhpStr, "[%3s]",
				enabled(!(Shm->Registration.hotplug < 0))),
	    pciRegLen = sprintf(pciRegStr, "[%3s]",
				enabled((Shm->Registration.pci == 1))),
	    nmiRegLen = sprintf(nmiRegStr, "<%3s>",
				enabled(Shm->Registration.nmi));
	size_t appLen = strlen(Shm->ShmName);
	ATTRIBUTE attribute[2][32] = {
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK
	    },
	    {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HGK,HGK,HGK,HDK,LWK
	    }
	};

	StoreTCell(wSet, SCANKEY_NULL, "                                ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Daemon gate                    ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, OPS_INTERVAL, " Interval(ns)                   ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Sys. Tick(ns)                  ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Poll Wait(ns)                  ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Ring Wait(ns)                  ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Child Wait(ns)                 ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Slice Wait(ns)                 ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet,OPS_EXPERIMENTAL," Experimental                   ",
				attribute[Shm->Registration.Experimental != 0]);

	StoreTCell(wSet, SCANKEY_NULL, " CPU Hot-Plug                   ",
				attribute[!(Shm->Registration.hotplug < 0)]);

	StoreTCell(wSet, SCANKEY_NULL, " PCI enablement                 ",
				attribute[(Shm->Registration.pci == 1)]);

	StoreTCell(wSet, OPS_INTERRUPTS," NMI registered                 ",
					attribute[Shm->Registration.nmi]);

	StoreTCell(wSet, SCANKEY_NULL, "                                ",
							MAKE_PRINT_UNFOCUS);

	memcpy(&TCellAt(wSet, 0, 1).item[31 - appLen],    Shm->ShmName, appLen);
	memcpy(&TCellAt(wSet, 0, 2).item[31 - intervLen], intervStr, intervLen);
	memcpy(&TCellAt(wSet, 0, 3).item[31 - tickLen],   tickStr,   tickLen);
	memcpy(&TCellAt(wSet, 0, 4).item[31 - pollLen],   pollStr,   pollLen);
	memcpy(&TCellAt(wSet, 0, 5).item[31 - ringLen],   ringStr,   ringLen);
	memcpy(&TCellAt(wSet, 0, 6).item[31 - childLen],  childStr,  childLen);
	memcpy(&TCellAt(wSet, 0, 7).item[31 - sliceLen],  sliceStr,  sliceLen);
	memcpy(&TCellAt(wSet, 0, 8).item[31 - experLen],  experStr,  experLen);
	memcpy(&TCellAt(wSet, 0, 9).item[31 - cpuhpLen],  cpuhpStr,  cpuhpLen);
	memcpy(&TCellAt(wSet, 0,10).item[31 - pciRegLen], pciRegStr, pciRegLen);
	memcpy(&TCellAt(wSet, 0,11).item[31 - nmiRegLen], nmiRegStr, nmiRegLen);

	StoreWindow(wSet, .title, " Settings ");

	StoreWindow(wSet,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wSet,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wSet,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wSet,	.key.WinUp,	MotionOriginUp_Win);
	StoreWindow(wSet,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wSet,	.key.Down,	MotionDown_Win);
	StoreWindow(wSet,	.key.Up,	MotionUp_Win);
	StoreWindow(wSet,	.key.Home,	MotionReset_Win);
	StoreWindow(wSet,	.key.End,	MotionEnd_Cell);
      }
      return(wSet);
    }

    Window *CreateHelp(unsigned long long id)
    {
      Window *wHelp = CreateWindow(wLayer, id, 2, 19, 2,
				(TOP_HEADER_ROW + 19 + 1 < drawSize.height) ?
					TOP_HEADER_ROW + 1 : 1);
      if (wHelp != NULL) {
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [F2]             ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"             Menu ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Escape]         ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"     Close window ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Shift]+[Tab]    ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"  Previous window ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Tab]            ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"      Next window ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"       [A|Z]      ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [W|Q]  [S]  [D]  ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"      Move window ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"       [Up]       ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Left]    [Right]",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"   Move selection ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"      [Down]      ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [End]            ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"        Last cell ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Home]           ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"       First cell ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Enter]          ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"Trigger selection ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Page-Up]        ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"    Previous page ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Page-Dw]        ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"        Next page ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Minus]          ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"  Scroll CPU down ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL," [Plus]           ",MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"    Scroll CPU up ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL,"                  ",MAKE_PRINT_UNFOCUS);

	StoreWindow(wHelp, .title, " Help ");
	StoreWindow(wHelp, .color[0].select, MAKE_PRINT_UNFOCUS);
	StoreWindow(wHelp, .color[1].select, MAKE_PRINT_UNFOCUS);

	StoreWindow(wHelp,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wHelp,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wHelp,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wHelp,	.key.WinUp,	MotionOriginUp_Win);
      }
      return(wHelp);
    }

    Window *CreateAdvHelp(unsigned long long id)
    {
      ATTRIBUTE attribute[2][38] = {
		{
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		{
		LWK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		}
      };
      struct ADV_HELP_ST {
	short theme;
	char *item;
	SCANKEY quick;
      } advHelp[] = {
	{0,"                                      ", {SCANKEY_NULL}	},
	{0," Frequency view:                      ", {SCANKEY_NULL}	},
	{1," %        Averages or Package C-States", {SCANKEY_PERCENT}	},
	{0,"                                      ", {SCANKEY_NULL}	},
	{0," Task Monitoring view:                ", {SCANKEY_NULL}	},
	{1," b                Sorting tasks order ", {SCANKEY_b}	},
	{1," n               Select task tracking ", {SCANKEY_n}	},
	{1," r              Reverse tasks sorting ", {SCANKEY_r}	},
	{1," v         Show|Hide contextual value ", {SCANKEY_v}	},
	{0,"                                      ", {SCANKEY_NULL}	},
	{0," Any view:                            ", {SCANKEY_NULL}	},
	{1," .             Top frequency or Usage ", {SCANKEY_DOT}	},
	{1," {             Start CoreFreq Machine ", {SCANKEY_OPEN_BRACE}},
	{1," }              Stop CoreFreq Machine ", {SCANKEY_CLOSE_BRACE}},
	{1," F10            Stop tools processing ", {SCANKEY_F10}	},
	{0,"                                      ", {SCANKEY_NULL}	},
	{1,"  Up  PgUp                     Scroll ", {SCANKEY_NULL}	},
	{1," Down PgDw                       CPU  ", {SCANKEY_NULL}	},
	{0,"                                      ", {SCANKEY_NULL}	},
      };
	const size_t nmemb = sizeof(advHelp) / sizeof(struct ADV_HELP_ST);
	Window *wHelp = CreateWindow(wLayer, id, 1, nmemb, 41,
				(TOP_HEADER_ROW + nmemb + 1 < drawSize.height) ?
					TOP_HEADER_ROW + 1 : 1);
      if (wHelp != NULL) {
	unsigned int idx;
	for (idx = 0; idx < nmemb; idx++)
		StoreTCell(	wHelp,
				advHelp[idx].quick.key,
				advHelp[idx].item,
				attribute[advHelp[idx].theme] );

	StoreWindow(wHelp, .title, " Keys ");

	StoreWindow(wHelp,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wHelp,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wHelp,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wHelp,	.key.WinUp,	MotionOriginUp_Win);
	StoreWindow(wHelp,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wHelp,	.key.Down,	MotionDown_Win);
	StoreWindow(wHelp,	.key.Up,	MotionUp_Win);
	StoreWindow(wHelp,	.key.Home,	MotionReset_Win);
	StoreWindow(wHelp,	.key.End,	MotionEnd_Cell);
      }
      return(wHelp);
    }

    Window *CreateAbout(unsigned long long id)
    {
      char *C[] = {
	"   ""   ______                ______               ""   ",
	"   ""  / ____/___  ________  / ____/_______  ____ _""   ",
	"   "" / /   / __ \\/ ___/ _ \\/ /_  / ___/ _ \\/ __ `/""   ",
	"   ""/ /___/ /_/ / /  /  __/ __/ / /  /  __/ /_/ / ""   ",
	"   ""\\____/\\____/_/   \\___/_/   /_/   \\___/\\__, /  ""   ",
	"   ""                                        /_/   ""   "
      };
      char *F[] = {
	"   ""   by CyrIng                                  ""   ",
	"   ""                                              ""   ",
	"   ""         (C)2015-2018 CYRIL INGENIERIE        ""   "
      };
	size_t	c = sizeof(C) / sizeof(C[0]),
		f = sizeof(F) / sizeof(F[0]),
		l = strlen(C[0]), v = strlen(COREFREQ_VERSION);

	CUINT cHeight=c + f, oCol=(drawSize.width-l)/2, oRow=TOP_HEADER_ROW + 4;
	if (cHeight >= (drawSize.height - 1)) {
		cHeight = drawSize.height - 2;
	}
	if (oRow + cHeight >= drawSize.height) {
		oRow = abs(drawSize.height - (1 + cHeight));
	}
	Window *wAbout = CreateWindow(wLayer, id, 1, cHeight, oCol, oRow);
	if (wAbout != NULL) {
		unsigned int i;

		for (i = 0; i < c; i++)
			StoreTCell(wAbout,SCANKEY_NULL, C[i], MAKE_PRINT_FOCUS);
		for (i = 0; i < f; i++)
			StoreTCell(wAbout,SCANKEY_NULL, F[i], MAKE_PRINT_FOCUS);

		size_t p = TCellAt(wAbout, 1, 5).length - 3 - v;
		memcpy(&TCellAt(wAbout, 1, 5).item[p], COREFREQ_VERSION, v);

		wAbout->matrix.select.row = wAbout->matrix.size.hth - 1;

		StoreWindow(wAbout, .title, " CoreFreq ");
		StoreWindow(wAbout, .color[0].select, MAKE_PRINT_UNFOCUS);
		StoreWindow(wAbout, .color[1].select, MAKE_PRINT_FOCUS);

		StoreWindow(wAbout,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wAbout,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wAbout,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wAbout,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wAbout);
    }

    Window *_CreateBox(unsigned long long id,
			Coordinate origin,
			Coordinate select,
			char *title,
			ASCII *button, ...)
    {
	struct PBOX {
		int cnt;
		struct SBOX {
			unsigned long long key;
			ASCII item[MIN_WIDTH];
			ATTRIBUTE attr;
		} btn[];
	} *pBox = NULL;
	int cnt = 0;

	va_list ap;
	va_start(ap, button);
	ASCII *item = button;
	ATTRIBUTE attr = va_arg(ap, ATTRIBUTE);
	unsigned long long aKey = va_arg(ap, unsigned long long);
	do {
	    if (item != NULL) {
		cnt = (pBox == NULL) ? 1: pBox->cnt + 1;
		if ((pBox = realloc(pBox,
					sizeof(struct PBOX)
					+ cnt * sizeof(struct SBOX))) != NULL)
		{
			strcpy((char *) pBox->btn[cnt - 1].item, (char *) item);
			pBox->btn[cnt - 1].attr = attr;
			pBox->btn[cnt - 1].key = aKey;
			pBox->cnt = cnt;
		}
		item = va_arg(ap, ASCII*);
		attr = va_arg(ap, ATTRIBUTE);
		aKey = va_arg(ap, unsigned long long);
	    }
	} while (item != NULL) ;
	va_end(ap);

	Window *wBox = NULL;
	if (pBox != NULL) {
	    CUINT cHeight = pBox->cnt, oRow = origin.row;
	    if (oRow + cHeight >= drawSize.height) {
		cHeight = CUMIN(cHeight, (drawSize.height - 2));
		oRow = 1;
	    }
	    wBox = CreateWindow(wLayer, id,
				1, cHeight,
				origin.col, oRow);
	    if (wBox != NULL) {
		wBox->matrix.select.col = select.col;
		wBox->matrix.select.row = select.row;

		for (cnt = 0; cnt < pBox->cnt; cnt++)
			StoreTCell(	wBox,
					pBox->btn[cnt].key,
					pBox->btn[cnt].item,
					pBox->btn[cnt].attr);
		if (title != NULL)
			StoreWindow(wBox, .title, title);

		StoreWindow(wBox,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wBox,	.key.Down,	MotionDown_Win);
		StoreWindow(wBox,	.key.Up,	MotionUp_Win);
		StoreWindow(wBox,	.key.Home,	MotionReset_Win);
		StoreWindow(wBox,	.key.End,	MotionEnd_Cell);
	    }
	    free(pBox);
	}
	return(wBox);
    }

#define CreateBox(id, origin, select, title, button, ...)		\
	_CreateBox(id, origin, select, title, button, __VA_ARGS__,NULL)

    Window *CreateSysInfo(unsigned long long id)
    {
	CoordSize matrixSize = {
		.wth = 1,
		.hth = CUMIN(18, (drawSize.height - TOP_HEADER_ROW - 2))
	};
	Coordinate winOrigin = {.col = 3, .row = TOP_HEADER_ROW + 2};
	CUINT winWidth = 74;
	void (*SysInfoFunc)(SHM_STRUCT*, CUINT, CELL_FUNC OutFunc) = NULL;
	char *title = NULL;

	switch (id) {
	case SCANKEY_p:
		{
		winOrigin.col = 2;
		winWidth = 76;
		SysInfoFunc = SysInfoProc;
		title = " Processor ";
		}
		break;
	case SCANKEY_e:
		{
		winOrigin.col = 4;
		winWidth = 72;
		SysInfoFunc = SysInfoFeatures;
		title = " Features ";
		}
		break;
	case SCANKEY_t:
		{
		if (TOP_HEADER_ROW + 11 + 6 < drawSize.height) {
			winOrigin.col = 23;
			matrixSize.hth = 7;
			winOrigin.row = TOP_HEADER_ROW + 11;
		} else {
			winOrigin.col = 18;
			matrixSize.hth = CUMIN((drawSize.height - 2), 7);
			winOrigin.row = 1;
		}
		winWidth = 50;
		SysInfoFunc = SysInfoTech;
		title = " Technologies ";
		}
		break;
	case SCANKEY_o:
		{
		SysInfoFunc = SysInfoPerfMon;
		title = " Performance Monitoring ";
		}
		break;
	case SCANKEY_w:
		{
		winOrigin.col = 25;
		if (TOP_HEADER_ROW + 2 + 14 < drawSize.height) {
			matrixSize.hth = 14;
			winOrigin.row = TOP_HEADER_ROW + 2;
		} else {
			matrixSize.hth = CUMIN((drawSize.height - 2), 14);
			winOrigin.row = 1;
		}
		winWidth = 50;
		SysInfoFunc = SysInfoPwrThermal;
		title = " Power & Thermal ";
		}
		break;
	case SCANKEY_u:
		{
		winWidth = 74;
		SysInfoFunc = SysInfoCPUID;
		title = " function           "				\
			"EAX          EBX          ECX          EDX ";
		}
		break;
	case SCANKEY_k:
		{
		winOrigin.col = 2;
		winWidth = 76;
		if (TOP_HEADER_ROW + 8 + 11 < drawSize.height) {
			matrixSize.hth = 11;
			winOrigin.row = TOP_HEADER_ROW + 8;
		} else {
			matrixSize.hth = CUMIN((drawSize.height - 2), 11);
			winOrigin.row = 1;
		}
		SysInfoFunc = SysInfoKernel;
		title = " Kernel ";
		}
		break;
	}

	int pad = 0;

	Window *wSysInfo = CreateWindow(wLayer, id,
					matrixSize.wth, matrixSize.hth,
					winOrigin.col, winOrigin.row);

	void AddSysInfoCell(CELL_ARGS)
	{
		pad++;
		StoreTCell(wSysInfo, key, item, attrib);
	}

	if (wSysInfo != NULL) {
		if (SysInfoFunc != NULL)
			SysInfoFunc(Shm, winWidth, AddSysInfoCell);

		while (pad < matrixSize.hth) {	// Pad with blank rows.
			pad++;
			StoreTCell(wSysInfo,
				SCANKEY_NULL,
				&hSpace[MAX_WIDTH - winWidth],
				MAKE_PRINT_FOCUS);
		}

		switch (id) {
		case SCANKEY_u:
			wSysInfo->matrix.select.row = 1;
			StoreWindow(wSysInfo,	.color[1].title,
						wSysInfo->hook.color[1].border);
			break;
		default:
			break;
		}
		StoreWindow(wSysInfo,	.title,		title);

		StoreWindow(wSysInfo,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wSysInfo,	.key.Left,	MotionLeft_Win);
		StoreWindow(wSysInfo,	.key.Right,	MotionRight_Win);
		StoreWindow(wSysInfo,	.key.Down,	MotionDown_Win);
		StoreWindow(wSysInfo,	.key.Up,	MotionUp_Win);
		StoreWindow(wSysInfo,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wSysInfo,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wSysInfo,	.key.Home,	MotionReset_Win);
		StoreWindow(wSysInfo,	.key.End,	MotionEnd_Cell);

		StoreWindow(wSysInfo,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wSysInfo,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wSysInfo,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wSysInfo,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wSysInfo);
    }

    Window *CreateTopology(unsigned long long id)
    {
	Window *wTopology = CreateWindow(wLayer, id,
					6, CUMIN(2 + Shm->Proc.CPU.Count,
					    (drawSize.height-TOP_HEADER_ROW-5)),
					1, TOP_HEADER_ROW + 3);
		wTopology->matrix.select.row = 2;

	void AddTopologyCell(CELL_ARGS)
	{
		StoreTCell(wTopology, key, item, attrib);
	}

	if (wTopology != NULL) {
		Topology(Shm, AddTopologyCell);

		StoreWindow(wTopology,	.title, " Topology ");

		StoreWindow(wTopology,	.key.Left,	MotionLeft_Win);
		StoreWindow(wTopology,	.key.Right,	MotionRight_Win);
		StoreWindow(wTopology,	.key.Down,	MotionDown_Win);
		StoreWindow(wTopology,	.key.Up,	MotionUp_Win);
		StoreWindow(wTopology,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wTopology,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wTopology,	.key.Home,	MotionReset_Win);
		StoreWindow(wTopology,	.key.End,	MotionEnd_Cell);

		StoreWindow(wTopology,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wTopology,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wTopology,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wTopology,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wTopology);
    }

    Window *CreateISA(unsigned long long id)
    {
	Window *wISA = CreateWindow(wLayer, id,
				4, CUMIN(7, (drawSize.height-TOP_HEADER_ROW-3)),
				6, TOP_HEADER_ROW + 2);

	void AddISACell(CELL_ARGS)
	{
		StoreTCell(wISA, key, item, attrib);
	}

	if (wISA != NULL) {
		SysInfoISA(Shm, AddISACell);

		StoreWindow(wISA,	.title, " Instruction Set Extensions ");

		StoreWindow(wISA,	.key.Left,	MotionLeft_Win);
		StoreWindow(wISA,	.key.Right,	MotionRight_Win);
		StoreWindow(wISA,	.key.Down,	MotionDown_Win);
		StoreWindow(wISA,	.key.Up,	MotionUp_Win);
		StoreWindow(wISA,	.key.Home,	MotionHome_Win);
		StoreWindow(wISA,	.key.End,	MotionEnd_Win);

		StoreWindow(wISA,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wISA,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wISA,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wISA,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wISA);
    }

    Window *CreateSysRegs(unsigned long long id)
    {
	Window *wSR = CreateWindow(wLayer, id,
				17,CUMIN((2 * (1 + Shm->Proc.CPU.Count)),
					(drawSize.height - TOP_HEADER_ROW - 3)),
				6, TOP_HEADER_ROW + 2);

	void AddSysRegsCell(CELL_ARGS)
	{
		StoreTCell(wSR, key, item, attrib);
	}

	if (wSR != NULL) {
		SystemRegisters(Shm, AddSysRegsCell);

		StoreWindow(wSR,	.title, " System Registers ");

		StoreWindow(wSR,	.key.Left,	MotionLeft_Win);
		StoreWindow(wSR,	.key.Right,	MotionRight_Win);
		StoreWindow(wSR,	.key.Down,	MotionDown_Win);
		StoreWindow(wSR,	.key.Up,	MotionUp_Win);
		StoreWindow(wSR,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wSR,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wSR,	.key.Home,	MotionHome_Win);
		StoreWindow(wSR,	.key.End,	MotionEnd_Win);

		StoreWindow(wSR,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wSR,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wSR,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wSR,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wSR);
    }

    Window *CreateMemCtrl(unsigned long long id)
    {
	unsigned short mc, cha, slot, rows = 0;
	for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {
		rows+=7;
	    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
		rows++;
	    for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
		rows++;
	}
	if (rows > 0) {
	    Window *wIMC = CreateWindow(wLayer, id,
					14, CUMIN((rows + 2),
					    (drawSize.height-TOP_HEADER_ROW-3)),
					1, TOP_HEADER_ROW + 2);
		wIMC->matrix.select.row = 4;

	    void AddMemoryControllerCell(CELL_ARGS)
	    {
		StoreTCell(wIMC, key, item, attrib);
	    }

	    if (wIMC != NULL) {
		MemoryController(Shm, AddMemoryControllerCell);

		StoreWindow(wIMC, .title, " Memory Controller ");

		StoreWindow(wIMC,	.key.Left,	MotionLeft_Win);
		StoreWindow(wIMC,	.key.Right,	MotionRight_Win);
		StoreWindow(wIMC,	.key.Down,	MotionDown_Win);
		StoreWindow(wIMC,	.key.Up,	MotionUp_Win);
		StoreWindow(wIMC,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wIMC,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wIMC,	.key.Home,	MotionHome_Win);
		StoreWindow(wIMC,	.key.End,	MotionEnd_Win);

		StoreWindow(wIMC,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wIMC,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wIMC,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wIMC,	.key.WinUp,	MotionOriginUp_Win);
	    }
	    return(wIMC);
	}
	else
	    return(NULL);
    }

    Window *CreateSortByField(unsigned long long id)
    {
	Window *wSortBy = CreateWindow( wLayer, id,
					1, SORTBYCOUNT,
					33, TOP_HEADER_ROW + MAX_ROWS + 2);
	if (wSortBy != NULL) {
		StoreTCell(wSortBy,SORTBY_STATE, " State    ", MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_RTIME, " RunTime  ", MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_UTIME, " UserTime ", MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_STIME, " SysTime  ", MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_PID,   " PID      ", MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_COMM,  " Command  ", MAKE_PRINT_DROP);

		wSortBy->matrix.select.row = Shm->SysGate.sortByField;

		StoreWindow(wSortBy, .color[0].select, MAKE_PRINT_DROP);
		StoreWindow(wSortBy, .color[0].title, MAKE_PRINT_DROP);
		StoreWindow(wSortBy, .color[1].title,MakeAttr(BLACK,0,WHITE,1));

		StoreWindow(wSortBy,	.Print,		ForEachCellPrint_Drop);

		StoreWindow(wSortBy,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wSortBy,	.key.Down,	MotionDown_Win);
		StoreWindow(wSortBy,	.key.Up,	MotionUp_Win);
		StoreWindow(wSortBy,	.key.Home,	MotionReset_Win);
		StoreWindow(wSortBy,	.key.End,	MotionEnd_Cell);
	}
	return(wSortBy);
    }

    Window *CreateTracking(unsigned long long id)
    {
	int SortByForest(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;

		if (task1->ppid < task2->ppid)
			return(-1);
		else if (task1->ppid > task2->ppid)
			return(1);

		else if (task1->tgid < task2->tgid)
			return(-1);
		else if (task1->tgid > task2->tgid)
			return(1);

		else if (task1->pid < task2->pid)
			return(-1);
		else if (task1->pid > task2->pid)
			return(1);

		else
			return(1);
	}

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
	  size_t tc = Shm->SysGate.taskCount;
	  if (tc > 0) {
	    const CUINT margin = 12;	// @ "Freq(MHz)"
	    int padding = drawSize.width - margin - TASK_COMM_LEN - 7;

	    Window *wTrack = CreateWindow(wLayer, id,
					1, TOP_HEADER_ROW + MAX_ROWS * 2,
					margin, TOP_HEADER_ROW);
	    if (wTrack != NULL) {
		char *item = malloc(MAX_WIDTH);
		TASK_MCB *trackList = malloc(tc * sizeof(TASK_MCB));

		memcpy(trackList, Shm->SysGate.taskList, tc * sizeof(TASK_MCB));
		qsort(trackList, tc, sizeof(TASK_MCB), SortByForest);

		unsigned int ti, si = 0, qi = 0;
		pid_t previd = (pid_t) -1;

		for (ti = 0; ti < tc; ti++) {
			if (trackList[ti].ppid == previd) {
				si += (si < padding - 2) ? 1 : 0;
			} else if (trackList[ti].tgid != previd) {
				si -= (si > 0) ? 1 : 0;
			}
			previd = trackList[ti].tgid;

			if (trackList[ti].pid == trackList[ti].tgid)
				qi = si + 1;
			else
				qi = si + 2;

			sprintf(item,
				"%.*s" "%-16s" "%.*s" "(%5d)",
				qi,
				hSpace,
				trackList[ti].comm,
				padding - qi,
				hSpace,
				trackList[ti].pid);

			StoreTCell(wTrack,
				(TRACK_TASK | trackList[ti].pid),
				item,
				(trackList[ti].pid == trackList[ti].tgid) ?
					  MAKE_PRINT_DROP
					: MakeAttr(BLACK, 0, WHITE, 1));
		}
		StoreWindow(wTrack, .color[0].select, MAKE_PRINT_DROP);
		StoreWindow(wTrack, .color[0].title, MAKE_PRINT_DROP);
		StoreWindow(wTrack, .color[1].title, MakeAttr(BLACK,0,WHITE,1));

		StoreWindow(wTrack,	.Print,		ForEachCellPrint_Drop);
		StoreWindow(wTrack,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wTrack,	.key.Down,	MotionDown_Win);
		StoreWindow(wTrack,	.key.Up,	MotionUp_Win);
		StoreWindow(wTrack,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wTrack,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wTrack,	.key.Home,	MotionReset_Win);
		StoreWindow(wTrack,	.key.End,	MotionEnd_Cell);

		free(trackList);
		free(item);
	    }
	    return(wTrack);
	  }
	  else
	    return(NULL);
	}
	else
	  return(NULL);
    }

    Window *CreateHotPlugCPU(unsigned long long id)
    {
	Window *wCPU = CreateWindow(	wLayer, id, 2, MAX_ROWS,
					LOAD_LEAD + 1, TOP_HEADER_ROW + 1);
	if (wCPU != NULL) {
		ATTRIBUTE enAttr[12] = {
			LWK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LWK,LDK,LDK,LDK
		}, disAttr[12] = {
			LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,LWK,LDK,LDK,LDK
		};
		ASCII item[12];
		unsigned int cpu;

	  for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	    if (BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		sprintf((char*) item, " %02u  Off ", cpu);
		StoreTCell(wCPU, SCANKEY_NULL, item, MakeAttr(BLUE,0,BLACK,1));
		StoreTCell(wCPU, CPU_ONLINE | cpu , "[ ENABLE]", enAttr);
	    } else {
		sprintf((char*) item, " %02u  On  ", cpu);
		StoreTCell(wCPU, SCANKEY_NULL, item, MakeAttr(WHITE,0,BLACK,0));
		StoreTCell(wCPU, CPU_OFFLINE | cpu, "[DISABLE]", disAttr);
	    }
	  }
		wCPU->matrix.select.col = 1;

		StoreWindow(wCPU,	.title,		" CPU ");
		StoreWindow(wCPU, .color[1].title, MakeAttr(WHITE, 0, BLUE, 1));

		StoreWindow(wCPU,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wCPU,	.key.Down,	MotionDown_Win);
		StoreWindow(wCPU,	.key.Up,	MotionUp_Win);
		StoreWindow(wCPU,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wCPU,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wCPU,	.key.Home,	MotionTop_Win);
		StoreWindow(wCPU,	.key.End,	MotionBottom_Win);

		StoreWindow(wCPU,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wCPU,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wCPU,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wCPU,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wCPU);
    }

    Window *CreateTurboClock(unsigned long long id)
    {
	ATTRIBUTE attribute[3][28] = {
		{
		LWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HDK,HDK,LWK,LWK,
		LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		{
		LWK,HBK,HBK,HBK,HBK,LBK,HBK,HBK,LWK,HDK,HDK,HDK,LWK,LWK,
		LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		{
		LWK,HRK,HRK,HRK,HRK,LRK,HRK,HRK,LWK,HDK,HDK,HDK,LWK,LWK,
		LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		}
	};
	ASCII item[32];
	CLOCK_ARG clockMod  = {.sllong = id};
	unsigned int ratio = clockMod.Ratio & CLOCKMOD_RATIO_MASK, multiplier;
	signed int offset,
	lowestOperatingShift = abs(Shm->Proc.Boost[BOOST(SIZE) - ratio]
					- Shm->Proc.Boost[BOOST(MIN)]),
	highestOperatingShift = MAXCLOCK_TO_RATIO(
				Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz
				) - Shm->Proc.Boost[BOOST(SIZE) - ratio],
	medianColdZone =( Shm->Proc.Boost[BOOST(MIN)]
			+ Shm->Proc.Features.Factory.Ratio ) >> 1,
	startingHotZone = Shm->Proc.Features.Factory.Ratio
			+ ( ( MAXCLOCK_TO_RATIO(
				Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz
				) - Shm->Proc.Features.Factory.Ratio ) >> 1);
	const CUINT	hthMin = 16,
			hthMax = 1+lowestOperatingShift + highestOperatingShift,
			hthWin = CUMIN(hthMin, hthMax);

	Window *wTC = CreateWindow(wLayer, id, 1, hthWin, 34,
				(TOP_HEADER_ROW + hthWin+2 < drawSize.height) ?
					TOP_HEADER_ROW + 2 : 1);
      if (wTC != NULL) {
	for (offset = -lowestOperatingShift;
		offset <= highestOperatingShift;
			offset++)
		{
		clockMod.Ratio = ratio | BOXKEY_TURBO_CLOCK;
		clockMod.Offset = offset;
		multiplier = Shm->Proc.Boost[BOOST(SIZE) - ratio] + offset;

		sprintf((char*) item, " %7.2f MHz   [%4d ]  %+3d ",
			(double)(multiplier
				* Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz)
				/ 1000000.0,
			multiplier, offset);

		StoreTCell(wTC, clockMod.sllong, item,
			attribute[multiplier < medianColdZone ?
					1 : multiplier > startingHotZone ?
						2 : 0]);
		}
	sprintf((char*) item, " Turbo Clock %1dC ", ratio);
	StoreWindow(wTC, .title, (char*) item);

	if (lowestOperatingShift >= hthWin) {
		wTC->matrix.scroll.vert = hthMax
					- hthWin * (1 + (highestOperatingShift
							/ hthWin));
		wTC->matrix.select.row  = lowestOperatingShift
					- wTC->matrix.scroll.vert;
	} else {
		wTC->matrix.select.row  = lowestOperatingShift;
	}
	StoreWindow(wTC,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wTC,	.key.Down,	MotionDown_Win);
	StoreWindow(wTC,	.key.Up,	MotionUp_Win);
	StoreWindow(wTC,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wTC,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wTC,	.key.Home,	MotionTop_Win);
	StoreWindow(wTC,	.key.End,	MotionBottom_Win);

	StoreWindow(wTC,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wTC,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wTC,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wTC,	.key.WinUp,	MotionOriginUp_Win);
      }
	return(wTC);
    }

    Window *CreateUncoreClock(unsigned long long id)
    {
	ATTRIBUTE attribute[2][28] = {
		{
		LWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HDK,HDK,LWK,LWK,
		LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		{
		LWK,HRK,HRK,HRK,HRK,LRK,HRK,HRK,LWK,HDK,HDK,HDK,LWK,LWK,
		LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		}
	};
	ASCII item[32];
	CLOCK_ARG clockMod  = {.sllong = id};
	unsigned int ratio = clockMod.Ratio & CLOCKMOD_RATIO_MASK, multiplier;
	signed int offset,
	lowestOperatingShift = abs(Shm->Uncore.Boost[UNCORE_BOOST(SIZE) - ratio]
				 - Shm->Proc.Boost[BOOST(MIN)]),
	highestOperatingShift = MAXCLOCK_TO_RATIO(
				Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz
				) - Shm->Uncore.Boost[UNCORE_BOOST(SIZE)-ratio],
	startingHotZone = Shm->Proc.Features.Factory.Ratio
			+ ( ( MAXCLOCK_TO_RATIO(
				Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz
				) - Shm->Proc.Features.Factory.Ratio ) >> 1);
	const CUINT	hthMin = 16,
			hthMax = 1+lowestOperatingShift + highestOperatingShift,
			hthWin = CUMIN(hthMin, hthMax);

	Window *wUC = CreateWindow(wLayer, id, 1, hthWin, 40,
				(TOP_HEADER_ROW + hthWin+2 < drawSize.height) ?
					TOP_HEADER_ROW + 2 : 1);
      if (wUC != NULL) {
	for (offset = -lowestOperatingShift;
		offset <= highestOperatingShift;
			offset++)
		{
		clockMod.Ratio = ratio | BOXKEY_UNCORE_CLOCK;
		clockMod.Offset = offset;
		multiplier = Shm->Uncore.Boost[UNCORE_BOOST(SIZE) - ratio];
		multiplier += offset;

		sprintf((char*) item, " %7.2f MHz   [%4d ]  %+3d ",
			(double)(multiplier
				* Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz)
				/ 1000000.0,
			multiplier, offset);

		StoreTCell(wUC, clockMod.sllong, item,
			attribute[multiplier > startingHotZone ? 1 : 0]);
		}
	sprintf((char*) item, " %s Clock Uncore ", ratio == 1 ? "Max" : "Min");
	StoreWindow(wUC, .title, (char*) item);

	if (lowestOperatingShift >= hthWin) {
		wUC->matrix.scroll.vert = hthMax
					- hthWin * (1 + (highestOperatingShift
							/ hthWin));
		wUC->matrix.select.row  = lowestOperatingShift
					- wUC->matrix.scroll.vert;
	} else {
		wUC->matrix.select.row  = lowestOperatingShift;
	}
	StoreWindow(wUC,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wUC,	.key.Down,	MotionDown_Win);
	StoreWindow(wUC,	.key.Up,	MotionUp_Win);
	StoreWindow(wUC,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wUC,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wUC,	.key.Home,	MotionTop_Win);
	StoreWindow(wUC,	.key.End,	MotionBottom_Win);

	StoreWindow(wUC,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wUC,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wUC,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wUC,	.key.WinUp,	MotionOriginUp_Win);
      }
	return(wUC);
    }

    void TrapScreenSize(int caught)
    {
      if (caught == SIGWINCH) {
	SCREEN_SIZE currentSize = GetScreenSize();

	if (currentSize.height != drawSize.height) {
		if (currentSize.height > MAX_HEIGHT)
			drawSize.height = MAX_HEIGHT;
		else
			drawSize.height = currentSize.height;

	    switch (drawFlag.disposal) {
	    case D_MAINVIEW:
		switch (drawFlag.view) {
		case V_FREQ:
		case V_INST:
		case V_CYCLES:
		case V_CSTATES:
		case V_TASKS:
		case V_INTR:
		case V_SLICE:
			MIN_HEIGHT = 2 + TOP_HEADER_ROW
					+ TOP_SEPARATOR + TOP_FOOTER_ROW;
			break;
		case V_PACKAGE:
		case V_VOLTAGE:
			MIN_HEIGHT = 8 + TOP_HEADER_ROW
					+ TOP_SEPARATOR + TOP_FOOTER_ROW;
			break;
		}
		break;
	    default:
		MIN_HEIGHT = LEADING_TOP + 2 * (MARGIN_HEIGHT + INTER_HEIGHT);
		break;
	    }

		drawFlag.clear  = 1;
		drawFlag.height = !(drawSize.height < MIN_HEIGHT);
	}
	if (currentSize.width != drawSize.width) {
		if (currentSize.width > MAX_WIDTH)
			drawSize.width = MAX_WIDTH;
		else
			drawSize.width = currentSize.width;

		drawFlag.clear = 1;
		drawFlag.width = !(drawSize.width < MIN_WIDTH);
	}
	MAX_ROWS = CUMIN(Shm->Proc.CPU.Count, ( drawSize.height
						- TOP_HEADER_ROW
						- TOP_SEPARATOR
						- TOP_FOOTER_ROW ) / 2);

	cpuScroll = 0;
      }
    }

  int Shortcut(SCANKEY *scan)
  {
	ATTRIBUTE stateAttr[2] = {
		MakeAttr(WHITE, 0, BLACK, 0),
		MakeAttr(CYAN , 0, BLACK, 1)
	},
	blankAttr = MakeAttr(BLACK, 0, BLACK, 1),
	descAttr =  MakeAttr(CYAN , 0, BLACK, 0);
	ASCII *stateStr[2][2] = {
		{
			(ASCII*)"              Disable               ",
			(ASCII*)"            < Disable >             "
		},{
			(ASCII*)"              Enable                ",
			(ASCII*)"            < Enable >              "
		}
	},
	*blankStr =	(ASCII*)"                                    ",
	*descStr[] = {
			(ASCII*)"             SpeedStep              ",
			(ASCII*)"        Enhanced Halt State         ",
			(ASCII*)" Turbo Boost/Core Performance Boost ",
			(ASCII*)"          C1 Auto Demotion          ",
			(ASCII*)"          C3 Auto Demotion          ",
			(ASCII*)"            C1 UnDemotion           ",
			(ASCII*)"            C3 UnDemotion           ",
			(ASCII*)"        I/O MWAIT Redirection       ",
			(ASCII*)"          Clock Modulation          ",
			(ASCII*)"           Core C6 State            ",
			(ASCII*)"          Package C6 State          "
	};

    switch (scan->key) {
    case SCANKEY_DOWN:
	if (!IsDead(&winList))
		return(-1);
	// Fallthrough
    case SCANKEY_PLUS:
	if ((drawFlag.disposal == D_MAINVIEW)
	&&  (cpuScroll < (Shm->Proc.CPU.Count - MAX_ROWS))) {
		cpuScroll++;
		drawFlag.layout = 1;
	}
    break;
    case SCANKEY_UP:
	if (!IsDead(&winList))
		return(-1);
	// Fallthrough
    case SCANKEY_MINUS:
	if ((drawFlag.disposal == D_MAINVIEW) && (cpuScroll > 0)) {
		cpuScroll--;
		drawFlag.layout = 1;
	}
    break;
    case SCANKEY_HOME:
    case SCANCON_HOME:
	if (!IsDead(&winList))
		return(-1);
	else if (drawFlag.disposal == D_MAINVIEW) {
		cpuScroll = 0;
		drawFlag.layout = 1;
	}
    break;
    case SCANKEY_END:
    case SCANCON_END:
	if (!IsDead(&winList))
		return(-1);
	else if (drawFlag.disposal == D_MAINVIEW) {
		cpuScroll = Shm->Proc.CPU.Count - MAX_ROWS;
		drawFlag.layout = 1;
	}
    break;
    case SCANKEY_PGDW:
	if (!IsDead(&winList))
		return(-1);
	else if (drawFlag.disposal == D_MAINVIEW) {
		CUINT offset = Shm->Proc.CPU.Count / 4;
		if ((cpuScroll + offset) < (Shm->Proc.CPU.Count - MAX_ROWS)) {
			cpuScroll += offset;
			drawFlag.layout = 1;
		}
	}
    break;
    case SCANKEY_PGUP:
	if (!IsDead(&winList))
		return(-1);
	else if (drawFlag.disposal == D_MAINVIEW) {
		CUINT offset = Shm->Proc.CPU.Count / 4;
		if (cpuScroll >= offset) {
			cpuScroll -= offset;
			drawFlag.layout = 1;
		}
	}
    break;
    case SCANKEY_OPEN_BRACE:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_MACHINE, COREFREQ_TOGGLE_ON);
    }
    break;
    case SCANKEY_CLOSE_BRACE:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_MACHINE, COREFREQ_TOGGLE_OFF);
    }
    break;
    case SCANKEY_F2:
    case SCANCON_F2:
    {
	Window *win = SearchWinListById(SCANKEY_F2, &winList);
	if (win == NULL)
		AppendWindow(CreateMenu(SCANKEY_F2), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_F4:
    case SCANCON_F4:
	BITSET(LOCKLESS, Shutdown, 0);
	break;
    case OPS_INTERVAL:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = 43,
		.row = TOP_HEADER_ROW + 4
	}, select = {
		.col = 0,
		.row = 5
	};
	AppendWindow(CreateBox(scan->key, origin, select, " Interval ",
	(ASCII*)"    100   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_100,
	(ASCII*)"    150   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_150,
	(ASCII*)"    250   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_250,
	(ASCII*)"    500   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_500,
	(ASCII*)"    750   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_750,
	(ASCII*)"   1000   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_1000,
	(ASCII*)"   1500   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_1500,
	(ASCII*)"   2000   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_2000,
	(ASCII*)"   2500   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_2500,
	(ASCII*)"   3000   ", MakeAttr(WHITE, 0, BLACK, 0), OPS_INTERVAL_3000),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case OPS_INTERVAL_100:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 100);
    }
    break;
    case OPS_INTERVAL_150:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 150);
    }
    break;
    case OPS_INTERVAL_250:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 250);
    }
    break;
    case OPS_INTERVAL_500:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 500);
    }
    break;
    case OPS_INTERVAL_750:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 750);
    }
    break;
    case OPS_INTERVAL_1000:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 1000);
    }
    break;
    case OPS_INTERVAL_1500:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 1500);
    }
    break;
    case OPS_INTERVAL_2000:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 2000);
    }
    break;
    case OPS_INTERVAL_2500:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 2500);
    }
    break;
    case OPS_INTERVAL_3000:
    {
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERVAL, 3000);
    }
    break;
    case OPS_EXPERIMENTAL:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	ATTRIBUTE exp_Attr[2] = {
		MakeAttr(RED , 0, BLACK, 1),
		MakeAttr(CYAN, 0, BLACK, 1)
	};
	ASCII *ops_Str[2][2] = {
		{
			(ASCII*)"       Nominal operating mode       ",
			(ASCII*)"     < Nominal operating mode >     ",
		},{
			(ASCII*)"     Experimental operating mode    ",
			(ASCII*)"   < Experimental operating mode >  "
		}
	};
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " Experimental ",
		blankStr, blankAttr, SCANKEY_NULL,
		"       CoreFreq Operation Mode       ", descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		ops_Str[0][Shm->Registration.Experimental == 0],
			stateAttr[Shm->Registration.Experimental == 0],
			OPS_EXPERIMENTAL_OFF,
		ops_Str[1][Shm->Registration.Experimental != 0] ,
			exp_Attr[Shm->Registration.Experimental != 0],
			OPS_EXPERIMENTAL_ON,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case OPS_EXPERIMENTAL_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
       RING_WRITE(Shm->Ring[0],COREFREQ_IOCTL_EXPERIMENTAL,COREFREQ_TOGGLE_OFF);
    }
    break;
    case OPS_EXPERIMENTAL_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0],COREFREQ_IOCTL_EXPERIMENTAL,COREFREQ_TOGGLE_ON);
    }
    break;
    case OPS_INTERRUPTS:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	ASCII *ops_Str[2][2] = {
		{
			(ASCII*)"              Register              ",
			(ASCII*)"            < Register >            ",
		},{
			(ASCII*)"             Unregister             ",
			(ASCII*)"           < Unregister >           "
		}
	};
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 5
	}, select = {
		.col = 0,
		.row = Shm->Registration.nmi == 0 ? 1 : 2
	};
	AppendWindow(CreateBox(scan->key, origin, select, " NMI Interrupts ",
		blankStr, blankAttr, SCANKEY_NULL,
		ops_Str[0][Shm->Registration.nmi != 0],
			stateAttr[Shm->Registration.nmi != 0],
			OPS_INTERRUPTS_ON,
		ops_Str[1][Shm->Registration.nmi == 0] ,
			stateAttr[Shm->Registration.nmi == 0],
			OPS_INTERRUPTS_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case OPS_INTERRUPTS_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERRUPTS,COREFREQ_TOGGLE_OFF);
    }
    break;
    case OPS_INTERRUPTS_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_INTERRUPTS, COREFREQ_TOGGLE_ON);
    }
    break;
    case SCANKEY_HASH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateHotPlugCPU(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
	break;
    case SCANKEY_PERCENT:
	if ((drawFlag.view == V_FREQ) && (drawFlag.disposal == D_MAINVIEW)) {
		drawFlag.avgOrPC = !drawFlag.avgOrPC;
		drawFlag.clear = 1;
	}
    break;
    case SCANKEY_DOT:
	if (drawFlag.disposal == D_MAINVIEW) {
		drawFlag.clkOrLd = !drawFlag.clkOrLd;
		drawFlag.clear = 1;
	}
    break;
    case 0x000000000000007e:
    {
	drawFlag.disposal = D_ASCIITEST;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_a:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateAbout(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_b:
	if ((drawFlag.view == V_TASKS) && (drawFlag.disposal == D_MAINVIEW)) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateSortByField(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
    break;
    case SCANKEY_c:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_CYCLES;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_d:
    {
	drawFlag.disposal = D_DASHBOARD;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_f:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_FREQ;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_n:
	if ((drawFlag.view == V_TASKS) && (drawFlag.disposal == D_MAINVIEW)) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateTracking(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
    break;
    case SCANKEY_g:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_PACKAGE;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_F1:
    case SCANCON_F1:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateAdvHelp(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_h:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateHelp(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_i:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_INST;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_l:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_CSTATES;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_m:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateTopology(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_SHIFT_i:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateISA(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_SHIFT_m:
	if (Shm->Uncore.CtrlCount > 0) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateMemCtrl(scan->key),&winList);
		else
			SetHead(&winList, win);
	}
    break;
    case SCANKEY_SHIFT_r:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateSysRegs(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_q:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_INTR;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_SHIFT_v:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_VOLTAGE;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_SHIFT_t:
    {
	drawFlag.disposal = D_MAINVIEW;
	drawFlag.view = V_SLICE;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_s:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateSettings(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_r:
	if ((drawFlag.view == V_TASKS) && (drawFlag.disposal == D_MAINVIEW)) {
		Shm->SysGate.reverseOrder = !Shm->SysGate.reverseOrder;
		drawFlag.layout = 1;
	}
    break;
    case SCANKEY_v:
	if ((drawFlag.view == V_TASKS) && (drawFlag.disposal == D_MAINVIEW)) {
		drawFlag.taskVal = !drawFlag.taskVal;
		drawFlag.layout = 1;
	}
    break;
    case SCANKEY_x:
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		Shm->SysGate.trackTask = 0;
		drawFlag.disposal = D_MAINVIEW;
		drawFlag.view = V_TASKS;
		drawSize.height = 0;
		TrapScreenSize(SIGWINCH);
	}
    break;
    case SORTBY_STATE:
    {
	Shm->SysGate.sortByField = F_STATE;
	drawFlag.layout = 1;
    }
    break;
    case SORTBY_RTIME:
    {
	Shm->SysGate.sortByField = F_RTIME;
	drawFlag.layout = 1;
    }
    break;
    case SORTBY_UTIME:
    {
	Shm->SysGate.sortByField = F_UTIME;
	drawFlag.layout = 1;
    }
    break;
    case SORTBY_STIME:
    {
	Shm->SysGate.sortByField = F_STIME;
	drawFlag.layout = 1;
    }
    break;
    case SORTBY_PID:
    {
	Shm->SysGate.sortByField = F_PID;
	drawFlag.layout = 1;
    }
    break;
    case SORTBY_COMM:
    {
	Shm->SysGate.sortByField = F_COMM;
	drawFlag.layout = 1;
    }
    break;
    case BOXKEY_EIST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
	{
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.EIST ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " EIST ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[0], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.EIST],
			stateAttr[Shm->Proc.Technology.EIST] , BOXKEY_EIST_ON,
		stateStr[0][!Shm->Proc.Technology.EIST],
			stateAttr[!Shm->Proc.Technology.EIST], BOXKEY_EIST_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_EIST_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_EIST, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_EIST_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_EIST, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_C1E:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
	{
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 2
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.C1E ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " C1E ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[1], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C1E],
			stateAttr[Shm->Proc.Technology.C1E] , BOXKEY_C1E_ON,
		stateStr[0][!Shm->Proc.Technology.C1E],
			stateAttr[!Shm->Proc.Technology.C1E], BOXKEY_C1E_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_C1E_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C1E, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_C1E_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C1E, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_TURBO:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
	{
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 4
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.Turbo ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " Turbo ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[2], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.Turbo],
			stateAttr[Shm->Proc.Technology.Turbo] , BOXKEY_TURBO_ON,
		stateStr[0][!Shm->Proc.Technology.Turbo],
			stateAttr[!Shm->Proc.Technology.Turbo], BOXKEY_TURBO_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	} else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_TURBO_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_TURBO, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_TURBO_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_TURBO, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_C1A:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 5
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.C1A ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " C1A ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[3], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C1A],
			stateAttr[Shm->Proc.Technology.C1A] , BOXKEY_C1A_ON,
		stateStr[0][!Shm->Proc.Technology.C1A],
			stateAttr[!Shm->Proc.Technology.C1A], BOXKEY_C1A_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_C1A_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C1A, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_C1A_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C1A, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_C3A:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 6
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.C3A ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " C3A ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[4], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C3A] ,
			stateAttr[Shm->Proc.Technology.C3A] , BOXKEY_C3A_ON,
		stateStr[0][!Shm->Proc.Technology.C3A],
			stateAttr[!Shm->Proc.Technology.C3A], BOXKEY_C3A_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	} else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_C3A_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C3A, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_C3A_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C3A, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_C1U:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 7
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.C1U ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " C1U ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[5], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C1U] ,
			stateAttr[Shm->Proc.Technology.C1U] , BOXKEY_C1U_ON,
		stateStr[0][!Shm->Proc.Technology.C1U],
			stateAttr[!Shm->Proc.Technology.C1U], BOXKEY_C1U_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_C1U_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C1U, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_C1U_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C1U, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_C3U:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 8
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.C3U ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " C3U ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[6], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C3U] ,
			stateAttr[Shm->Proc.Technology.C3U] , BOXKEY_C3U_ON,
		stateStr[0][!Shm->Proc.Technology.C3U],
			stateAttr[!Shm->Proc.Technology.C3U], BOXKEY_C3U_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_C3U_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C3U, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_C3U_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_C3U, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_CC6:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 9
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.CC6 ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " CC6 ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[9], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.CC6] ,
			stateAttr[Shm->Proc.Technology.CC6] , BOXKEY_CC6_ON,
		stateStr[0][!Shm->Proc.Technology.CC6],
			stateAttr[!Shm->Proc.Technology.CC6], BOXKEY_CC6_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_CC6_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CC6, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_CC6_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CC6, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_PC6:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 10
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.PC6 ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " PC6 ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[10], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.PC6] ,
			stateAttr[Shm->Proc.Technology.PC6] , BOXKEY_PC6_ON,
		stateStr[0][!Shm->Proc.Technology.PC6],
			stateAttr[!Shm->Proc.Technology.PC6], BOXKEY_PC6_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_PC6_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_PC6, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_PC6_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_PC6, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_PKGCST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const CSINT thisCST[] = {5, 4, 3, 2, -1, -1, 1, 0}; // Row indexes
	const Coordinate origin = {
	.col = (drawSize.width - (44 - 17)) / 2,
	.row = TOP_HEADER_ROW + 2
	}, select = {
	.col=0,
	.row=thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit] != -1 ?
		thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit] : 0
	};
		Window *wBox = CreateBox(scan->key, origin, select,
					" Package C-State Limit ",
	(ASCII*)"             C7            ", stateAttr[0], BOXKEY_PKGCST_C7,
	(ASCII*)"             C6            ", stateAttr[0], BOXKEY_PKGCST_C6,
	(ASCII*)"             C3            ", stateAttr[0], BOXKEY_PKGCST_C3,
	(ASCII*)"             C2            ", stateAttr[0], BOXKEY_PKGCST_C2,
	(ASCII*)"             C1            ", stateAttr[0], BOXKEY_PKGCST_C1,
	(ASCII*)"             C0            ", stateAttr[0], BOXKEY_PKGCST_C0);
		if (wBox != NULL) {
			TCellAt(wBox, 0, select.row).attr[11] =		\
			TCellAt(wBox, 0, select.row).attr[12] =		\
			TCellAt(wBox, 0, select.row).attr[13] =		\
			TCellAt(wBox, 0, select.row).attr[14] =		\
			TCellAt(wBox, 0, select.row).attr[15] =		\
			TCellAt(wBox, 0, select.row).attr[16] =		\
								stateAttr[1];
			TCellAt(wBox, 0, select.row).item[11] = '<';
			TCellAt(wBox, 0, select.row).item[16] = '>';

			AppendWindow(wBox, &winList);
		} else
			SetHead(&winList, win);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_PKGCST_C7:
    case BOXKEY_PKGCST_C6:
    case BOXKEY_PKGCST_C3:
    case BOXKEY_PKGCST_C2:
    case BOXKEY_PKGCST_C1:
    case BOXKEY_PKGCST_C0:
    {
	const unsigned long newCST = (scan->key - BOXKEY_PKGCST_C0) >> 4;
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_PKGCST, newCST);
    }
    break;
    case BOXKEY_IOMWAIT:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const unsigned int isIORedir = (
			Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir == 1
	);
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 9
	}, select = {
		.col = 0,
		.row = isIORedir ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " I/O MWAIT ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[7], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
	stateStr[1][isIORedir] , stateAttr[isIORedir] , BOXKEY_IOMWAIT_ON,
	stateStr[0][!isIORedir], stateAttr[!isIORedir], BOXKEY_IOMWAIT_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_IOMWAIT_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_IOMWAIT, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_IOMWAIT_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_IOMWAIT, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_IORCST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const CSINT thisCST[]={-1, -1, -1, 3, 2, -1, 1, 0}; // Row indexes
	const Coordinate origin = {
	.col = (drawSize.width - (44 - 17)) / 2,
	.row = TOP_HEADER_ROW + 3
	}, select = {
	.col = 0,
	.row=thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude]!=-1 ?
		thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude]:0
	};
		Window *wBox = CreateBox(scan->key, origin, select,
					" I/O MWAIT Max C-State ",
	(ASCII*)"             C7            ", stateAttr[0], BOXKEY_IORCST_C7,
	(ASCII*)"             C6            ", stateAttr[0], BOXKEY_IORCST_C6,
	(ASCII*)"             C4            ", stateAttr[0], BOXKEY_IORCST_C4,
	(ASCII*)"             C3            ", stateAttr[0], BOXKEY_IORCST_C3);
		if (wBox != NULL) {
			TCellAt(wBox, 0, select.row).attr[11] =		\
			TCellAt(wBox, 0, select.row).attr[12] =		\
			TCellAt(wBox, 0, select.row).attr[13] =		\
			TCellAt(wBox, 0, select.row).attr[14] =		\
			TCellAt(wBox, 0, select.row).attr[15] =		\
			TCellAt(wBox, 0, select.row).attr[16] =		\
								stateAttr[1];
			TCellAt(wBox, 0, select.row).item[11] = '<';
			TCellAt(wBox, 0, select.row).item[16] = '>';

			AppendWindow(wBox, &winList);
		} else
			SetHead(&winList, win);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_IORCST_C3:
    case BOXKEY_IORCST_C4:
    case BOXKEY_IORCST_C6:
    case BOXKEY_IORCST_C7:
    {
	const unsigned long newCST = (scan->key - BOXKEY_IORCST_C0) >> 4;
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_IORCST, newCST);
    }
    break;
    case BOXKEY_ODCM:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 6
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.ODCM ? 4 : 3
	};
	AppendWindow(CreateBox(scan->key, origin, select, " ODCM ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[8], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.ODCM] ,
			stateAttr[Shm->Proc.Technology.ODCM] , BOXKEY_ODCM_ON,
		stateStr[0][!Shm->Proc.Technology.ODCM],
			stateAttr[!Shm->Proc.Technology.ODCM],BOXKEY_ODCM_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_ODCM_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_ODCM, COREFREQ_TOGGLE_OFF);
    }
    break;
    case BOXKEY_ODCM_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_ODCM, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_DUTYCYCLE:
    {
	Window *win = SearchWinListById(scan->key, &winList);
     if (win == NULL)
     {
	const CSINT maxCM = 7 << Shm->Cpu[Shm->Proc.Service.Core]	\
					.PowerThermal.DutyCycle.Extended;
	const Coordinate origin = {
		.col = (drawSize.width - (44 - 17)) / 2,
		.row = TOP_HEADER_ROW + 3
     }, select = {
	.col = 0, .row = (
	Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod >= 0
	) && (
	Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod <=maxCM
	) ? Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod : 1
     };
	Window *wBox = NULL;

      if (Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.Extended)
		wBox = CreateBox(scan->key, origin, select,
		" Extended Duty Cycle ",
	(ASCII*)"          Reserved         ", blankAttr,    BOXKEY_ODCM_DC00,
	(ASCII*)"            6.25%          ", stateAttr[0], BOXKEY_ODCM_DC01,
	(ASCII*)"           12.50%          ", stateAttr[0], BOXKEY_ODCM_DC02,
	(ASCII*)"           18.75%          ", stateAttr[0], BOXKEY_ODCM_DC03,
	(ASCII*)"           25.00%          ", stateAttr[0], BOXKEY_ODCM_DC04,
	(ASCII*)"           31.25%          ", stateAttr[0], BOXKEY_ODCM_DC05,
	(ASCII*)"           37.50%          ", stateAttr[0], BOXKEY_ODCM_DC06,
	(ASCII*)"           43.75%          ", stateAttr[0], BOXKEY_ODCM_DC07,
	(ASCII*)"           50.00%          ", stateAttr[0], BOXKEY_ODCM_DC08,
	(ASCII*)"           56.25%          ", stateAttr[0], BOXKEY_ODCM_DC09,
	(ASCII*)"           63.50%          ", stateAttr[0], BOXKEY_ODCM_DC10,
	(ASCII*)"           68.75%          ", stateAttr[0], BOXKEY_ODCM_DC11,
	(ASCII*)"           75.00%          ", stateAttr[0], BOXKEY_ODCM_DC12,
	(ASCII*)"           81.25%          ", stateAttr[0], BOXKEY_ODCM_DC13,
	(ASCII*)"           87.50%          ", stateAttr[0], BOXKEY_ODCM_DC14);
      else
		wBox = CreateBox(scan->key, origin, select,
			" Duty Cycle ",
	(ASCII*)"          Reserved         ", blankAttr,    BOXKEY_ODCM_DC00,
	(ASCII*)"           12.50%          ", stateAttr[0], BOXKEY_ODCM_DC01,
	(ASCII*)"           25.00%          ", stateAttr[0], BOXKEY_ODCM_DC02,
	(ASCII*)"           37.50%          ", stateAttr[0], BOXKEY_ODCM_DC03,
	(ASCII*)"           50.00%          ", stateAttr[0], BOXKEY_ODCM_DC04,
	(ASCII*)"           62.50%          ", stateAttr[0], BOXKEY_ODCM_DC05,
	(ASCII*)"           75.00%          ", stateAttr[0], BOXKEY_ODCM_DC06,
	(ASCII*)"           87.50%          ", stateAttr[0], BOXKEY_ODCM_DC07);

	if (wBox != NULL) {
		TCellAt(wBox, 0, select.row).attr[ 8] = 	\
		TCellAt(wBox, 0, select.row).attr[ 9] = 	\
		TCellAt(wBox, 0, select.row).attr[10] = 	\
		TCellAt(wBox, 0, select.row).attr[11] = 	\
		TCellAt(wBox, 0, select.row).attr[12] = 	\
		TCellAt(wBox, 0, select.row).attr[13] = 	\
		TCellAt(wBox, 0, select.row).attr[14] = 	\
		TCellAt(wBox, 0, select.row).attr[15] = 	\
		TCellAt(wBox, 0, select.row).attr[16] = 	\
		TCellAt(wBox, 0, select.row).attr[17] = 	\
		TCellAt(wBox, 0, select.row).attr[18] = 	\
		TCellAt(wBox, 0, select.row).attr[19] = stateAttr[1];
		TCellAt(wBox, 0, select.row).item[ 8] = '<';
		TCellAt(wBox, 0, select.row).item[19] = '>';

		AppendWindow(wBox, &winList);
	} else
		SetHead(&winList, win);
     } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_ODCM_DC00:
    case BOXKEY_ODCM_DC01:
    case BOXKEY_ODCM_DC02:
    case BOXKEY_ODCM_DC03:
    case BOXKEY_ODCM_DC04:
    case BOXKEY_ODCM_DC05:
    case BOXKEY_ODCM_DC06:
    case BOXKEY_ODCM_DC07:
    case BOXKEY_ODCM_DC08:
    case BOXKEY_ODCM_DC09:
    case BOXKEY_ODCM_DC10:
    case BOXKEY_ODCM_DC11:
    case BOXKEY_ODCM_DC12:
    case BOXKEY_ODCM_DC13:
    case BOXKEY_ODCM_DC14:
    {
	const unsigned long newDC = (scan->key - BOXKEY_ODCM_DC00) >> 4;
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_ODCM_DC, newDC);
    }
    break;
    case BOXKEY_TURBO_CLOCK_1C:
    case BOXKEY_TURBO_CLOCK_2C:
    case BOXKEY_TURBO_CLOCK_3C:
    case BOXKEY_TURBO_CLOCK_4C:
    case BOXKEY_TURBO_CLOCK_5C:
    case BOXKEY_TURBO_CLOCK_6C:
    case BOXKEY_TURBO_CLOCK_7C:
    case BOXKEY_TURBO_CLOCK_8C:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateTurboClock(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_UNCORE_CLOCK_MAX:
    case BOXKEY_UNCORE_CLOCK_MIN:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateUncoreClock(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_F10:
    case BOXKEY_TOOLS_MACHINE:
    {
      if (!RING_FULL(Shm->Ring[1]))
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_MACHINE, COREFREQ_TOGGLE_OFF);
    }
    break;
    case SCANKEY_F3:
    case SCANCON_F3:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = 13,
		.row = TOP_HEADER_ROW + 2
	}, select = {
		.col = 0,
		.row = 0
	};
	Window *wBox = CreateBox(scan->key, origin, select,
			" Tools ",
    (ASCII*)"            STOP           ", BITVAL(Shm->Proc.Sync, 31) ?
					   MakeAttr(RED,0,BLACK,0) : blankAttr,
					   BITVAL(Shm->Proc.Sync, 31) ?
					   BOXKEY_TOOLS_MACHINE : SCANKEY_NULL,
    (ASCII*)"        Atomic Burn        ", stateAttr[0], BOXKEY_TOOLS_ATOMIC,
    (ASCII*)"       CRC32 Compute       ", stateAttr[0], BOXKEY_TOOLS_CRC32,
    (ASCII*)"       Conic Compute...    ", stateAttr[0], BOXKEY_TOOLS_CONIC,
    (ASCII*)"      Turbo Random CPU     ", stateAttr[0], BOXKEY_TOOLS_TURBO_RND,
    (ASCII*)"      Turbo Round Robin    ", stateAttr[0], BOXKEY_TOOLS_TURBO_RR);
	if (wBox != NULL) {
		AppendWindow(wBox, &winList);
	} else
		SetHead(&winList, win);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_TOOLS_ATOMIC:
    {
      if (!RING_FULL(Shm->Ring[1]))
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_ATOMIC, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_TOOLS_CRC32:
    {
      if (!RING_FULL(Shm->Ring[1]))
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CRC32, COREFREQ_TOGGLE_ON);
    }
    break;
    case BOXKEY_TOOLS_CONIC:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = 13 + 27 + 3,
		.row = TOP_HEADER_ROW + 2 + 4
	}, select = {
		.col = 0,
		.row = 0
	};
	Window *wBox = CreateBox(scan->key, origin, select,
		" Conic variations ",
    (ASCII*)"         Ellipsoid         ", stateAttr[0], BOXKEY_TOOLS_CONIC0,
    (ASCII*)" Hyperboloid of one sheet  ", stateAttr[0], BOXKEY_TOOLS_CONIC1,
    (ASCII*)" Hyperboloid of two sheets ", stateAttr[0], BOXKEY_TOOLS_CONIC2,
    (ASCII*)"    Elliptical cylinder    ", stateAttr[0], BOXKEY_TOOLS_CONIC3,
    (ASCII*)"    Hyperbolic cylinder    ", stateAttr[0], BOXKEY_TOOLS_CONIC4,
    (ASCII*)"    Two parallel planes    ", stateAttr[0], BOXKEY_TOOLS_CONIC5);
	if (wBox != NULL) {
		AppendWindow(wBox, &winList);
	} else
		SetHead(&winList, win);
      } else
		SetHead(&winList, win);
    }
    break;
    case BOXKEY_TOOLS_CONIC0:
    {
    if (!RING_FULL(Shm->Ring[1]))
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CONIC, CONIC_ELLIPSOID);
    }
    break;
    case BOXKEY_TOOLS_CONIC1:
    {
  if (!RING_FULL(Shm->Ring[1]))
    RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CONIC, CONIC_HYPERBOLOID_ONE_SHEET);
    }
    break;
    case BOXKEY_TOOLS_CONIC2:
    {
  if (!RING_FULL(Shm->Ring[1]))
   RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CONIC, CONIC_HYPERBOLOID_TWO_SHEETS);
    }
    break;
    case BOXKEY_TOOLS_CONIC3:
    {
  if (!RING_FULL(Shm->Ring[1]))
      RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CONIC, CONIC_ELLIPTICAL_CYLINDER);
    }
    break;
    case BOXKEY_TOOLS_CONIC4:
    {
  if (!RING_FULL(Shm->Ring[1]))
      RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CONIC, CONIC_HYPERBOLIC_CYLINDER);
    }
    break;
    case BOXKEY_TOOLS_CONIC5:
    {
  if (!RING_FULL(Shm->Ring[1]))
      RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_CONIC, CONIC_TWO_PARALLEL_PLANES);
    }
    break;
    case BOXKEY_TOOLS_TURBO_RND:
    {
      if (!RING_FULL(Shm->Ring[1]))
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_TURBO, RAND_SMT);
    }
    break;
    case BOXKEY_TOOLS_TURBO_RR:
    {
      if (!RING_FULL(Shm->Ring[1]))
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_TURBO, RR_SMT);
    }
    break;
    case SCANKEY_k:
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) == 0)
		break;
	// fallthrough
    case SCANKEY_e:
    case SCANKEY_o:
    case SCANKEY_p:
    case SCANKEY_t:
    case SCANKEY_u:
    case SCANKEY_w:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateSysInfo(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    default:
      if (scan->key & TRACK_TASK) {
		Shm->SysGate.trackTask = scan->key & TRACK_MASK;
		drawFlag.layout = 1;
      }
      else if (scan->key & CPU_ONLINE) {
		const unsigned long cpu = scan->key & CPUID_MASK;
		if (!RING_FULL(Shm->Ring[0]))
			RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CPU_ON, cpu);
      }
      else if (scan->key & CPU_OFFLINE) {
		const unsigned long cpu = scan->key & CPUID_MASK;
		if (!RING_FULL(Shm->Ring[0]))
			RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CPU_OFF, cpu);
      }
      else {
	CLOCK_ARG clockMod  = {.sllong = scan->key};
	if (clockMod.Ratio & BOXKEY_TURBO_CLOCK)
	{
	  clockMod.Ratio &= CLOCKMOD_RATIO_MASK;

	 if (!RING_FULL(Shm->Ring[0]))
	   RING_WRITE(Shm->Ring[0],COREFREQ_IOCTL_TURBO_CLOCK, clockMod.sllong);
	}
	else if (clockMod.Ratio & BOXKEY_UNCORE_CLOCK)
	{
	  clockMod.Ratio &= CLOCKMOD_RATIO_MASK;

	 if (!RING_FULL(Shm->Ring[0]))
	  RING_WRITE(Shm->Ring[0],COREFREQ_IOCTL_UNCORE_CLOCK, clockMod.sllong);
	}
	else
		return(-1);
      }
    }
	return(0);
  }

    void PrintTaskMemory(Layer *layer, CUINT row,
			int taskCount,
			unsigned long freeRAM,
			unsigned long totalRAM)
    {
	sprintf(buffer, "%6u" "%9lu" "%-9lu", taskCount, freeRAM, totalRAM);

	memcpy(&LayerAt(layer, code, (drawSize.width -35), row), &buffer[0], 6);
	memcpy(&LayerAt(layer, code, (drawSize.width -22), row), &buffer[6], 9);
	memcpy(&LayerAt(layer, code, (drawSize.width -12), row), &buffer[15],9);
    }

    void Layout_Header(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = NULL;
	size_t len;

	// Reset the Top Frequency
	Flop=&Shm->Cpu[Shm->Proc.Top].FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];

	if (!drawFlag.clkOrLd) {
		Clock2LCD(layer,0,row,Flop->Relative.Freq,Flop->Relative.Ratio);
	} else {
		double percent = 100.f * Shm->Proc.Avg.C0;

		Load2LCD(layer, 0, row, percent);
	}
	LayerDeclare(12) hProc0 = {
		.origin = {.col = 12, .row = row}, .length = 12,
		.attr = {LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HRK,HDK},
		.code = {' ','P','r','o','c','e','s','s','o','r',' ','['}
	};

	LayerDeclare(11) hProc1 = {
		.origin = {.col = drawSize.width - 11, .row = row},.length = 11,
		.attr = {HDK,HWK,HWK,HWK,HDK,HWK,HWK,HWK,LWK,LWK,LWK},
		.code = {']',' ',' ',' ','/',' ',' ',' ','C','P','U'}
	};

	row++;

	LayerDeclare(15) hArch0 = {
	    .origin = {.col = 12, .row = row}, .length = 15,
	    .attr={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK},
	    .code={' ','A','r','c','h','i','t','e','c','t','u','r','e',' ','['}
	};

	LayerDeclare(30) hArch1 = {
		.origin = {.col = drawSize.width - 30, .row = row},.length = 30,
		.attr ={HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,	\
			LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,LWK,LWK
		},
		.code ={']',' ','C','a','c','h','e','s',' ',		\
			'L','1',' ','I','n','s','t','=',' ',' ',' ',	\
			'D','a','t','a','=',' ',' ',' ','K','B'
		}
	};

	row++;

	LayerDeclare(28) hBClk0 = {
		.origin = {.col = 12, .row = row}, .length = 28,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
			HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,\
			LWK,LWK,LWK
			},
		.code ={' ','B','a','s','e',' ','C','l','o','c','k',' ',\
			'~',' ','0','0','0',' ','0','0','0',' ','0','0','0',\
			' ','H','z'
		}
	};

	LayerDeclare(21) hArch2 = {
		.origin = {.col = drawSize.width - 21, .row = row},.length = 21,
		.attr ={LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,	\
			LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK
		},
		.code ={'L','2','=',' ',' ',' ',' ',' ',' ',' ',	\
			'L','3','=',' ',' ',' ',' ',' ',' ','K','B'
		}
	};

	row++;

	sprintf(buffer, "%2u" "%-2u",
		Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	hProc1.code[2] = buffer[0];
	hProc1.code[3] = buffer[1];
	hProc1.code[5] = buffer[2];
	hProc1.code[6] = buffer[3];

	unsigned int L1I_Size = 0, L1D_Size = 0, L2U_Size = 0, L3U_Size = 0;
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL) {
	L1I_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[0].Size / 1024;
	L1D_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[1].Size / 1024;
	L2U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[2].Size / 1024;
	L3U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[3].Size / 1024;
    } else if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD) {
	L1I_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[0].Size;
	L1D_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[1].Size;
	L2U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[2].Size;
	L3U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[3].Size;
    }
	sprintf(buffer, "%-3u" "%-3u", L1I_Size, L1D_Size);

	hArch1.code[17] = buffer[0];
	hArch1.code[18] = buffer[1];
	hArch1.code[19] = buffer[2];
	hArch1.code[25] = buffer[3];
	hArch1.code[26] = buffer[4];
	hArch1.code[27] = buffer[5];

	sprintf(buffer, "%-4u" "%-5u", L2U_Size, L3U_Size);

	hArch2.code[ 3] = buffer[0];
	hArch2.code[ 4] = buffer[1];
	hArch2.code[ 5] = buffer[2];
	hArch2.code[ 6] = buffer[3];
	hArch2.code[13] = buffer[4];
	hArch2.code[14] = buffer[5];
	hArch2.code[15] = buffer[6];
	hArch2.code[16] = buffer[7];
	hArch2.code[17] = buffer[8];

	len = strlen(Shm->Proc.Brand);

	if (BITVAL(Shm->Proc.Sync, 31))
		hProc0.code[10] = '.';

	LayerCopyAt(layer, hProc0.origin.col, hProc0.origin.row,
			hProc0.length, hProc0.attr, hProc0.code);

	LayerFillAt(layer,(hProc0.origin.col + hProc0.length),hProc0.origin.row,
			len, Shm->Proc.Brand,
			MakeAttr(CYAN, 0, BLACK, 1));

	if ((hProc1.origin.col - len) > 0)
	    LayerFillAt(layer, (hProc0.origin.col + hProc0.length + len),
			hProc0.origin.row,
			(hProc1.origin.col - len), hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hProc1.origin.col, hProc1.origin.row,
			hProc1.length, hProc1.attr, hProc1.code);

	len = strlen(Shm->Proc.Architecture);

	LayerCopyAt(layer, hArch0.origin.col, hArch0.origin.row,
			hArch0.length, hArch0.attr, hArch0.code);

	LayerFillAt(layer,(hArch0.origin.col + hArch0.length),hArch0.origin.row,
			len, Shm->Proc.Architecture,
			MakeAttr(CYAN, 0, BLACK, 1));

	if ((hArch1.origin.col - len) > 0)
	    LayerFillAt(layer, (hArch0.origin.col + hArch0.length + len),
			hArch0.origin.row,
			(hArch1.origin.col - len), hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hArch1.origin.col, hArch1.origin.row,
			hArch1.length, hArch1.attr, hArch1.code);

	LayerCopyAt(layer, hBClk0.origin.col, hBClk0.origin.row,
			hBClk0.length, hBClk0.attr, hBClk0.code);

	LayerFillAt(layer,(hBClk0.origin.col + hBClk0.length),hBClk0.origin.row,
			(hArch2.origin.col - hBClk0.origin.col + hBClk0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hArch2.origin.col, hArch2.origin.row,
			hArch2.length, hArch2.attr, hArch2.code);
    }

    void Layout_Ruller_Load(Layer *layer, CUINT row)
    {
	LayerDeclare(MAX_WIDTH) hLoad0 = {
		.origin = {.col = 0, .row = row}, .length = drawSize.width,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK
		},
		.code ={'-','-','-',' ','R','a','t','i',		\
			'o',' ','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-','-','-','-','-',		\
			'-','-','-','-'
		}
	};
	// Alternate the color of the frequency ratios
	unsigned int idx;
	int bright = 0;

	for (idx = 0; idx < ratioCount; idx++) {
		char tabStop[] = "00";
		int hPos = availRatio[idx] * loadWidth / maxRatio;
		sprintf(tabStop, "%2.0f", availRatio[idx]);

		if (tabStop[0] != 0x20) {
			hLoad0.code[hPos + 2]=tabStop[0];
			hLoad0.attr[hPos + 2]=MakeAttr(CYAN, 0, BLACK, bright);
		}
		hLoad0.code[hPos + 3] = tabStop[1];
		hLoad0.attr[hPos + 3] = MakeAttr(CYAN, 0, BLACK, bright);

		bright = !bright;
	}
	LayerCopyAt(layer, hLoad0.origin.col, hLoad0.origin.row,
			hLoad0.length, hLoad0.attr, hLoad0.code);
    }

    CUINT Layout_Monitor_Frequency(Layer *layer, CUINT row)
    {
	LayerDeclare(77) hMon0 = {
		.origin = {	.col = (LOAD_LEAD - 1),
				.row = (row + MAX_ROWS + 1)
		},
		.length = 77,
		.attr ={HWK,						\
			HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,		\
			HDK,HWK,HWK,LWK,HWK,HWK,HDK,LWK,		\
			HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,LWK,		\
			HBK,HBK,HBK,HDK,				\
			LWK,LWK,LWK,HDK,				\
			LYK,LYK,LYK					\
		},
		.code ={' ',						\
			' ',' ',' ',' ',0x0,' ',' ',' ',		\
			0x0,' ',' ',0x0,' ',' ',0x0,' ',		\
			' ',' ',' ',0x0,' ',' ',0x0,' ',		\
			' ',' ',' ',0x0,' ',' ',0x0,' ',		\
			' ',' ',' ',0x0,' ',' ',0x0,' ',		\
			' ',' ',' ',0x0,' ',' ',0x0,' ',		\
			' ',' ',' ',0x0,' ',' ',0x0,' ',		\
			' ',' ',' ',0x0,' ',' ',0x0,' ',' ',		\
			' ',' ',' ',0x0,				\
			' ',' ',' ',0x0,				\
			' ',' ',' '					\
		}
	};
	LayerCopyAt(layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	LayerFillAt(layer, (hMon0.origin.col + hMon0.length),
			hMon0.origin.row,
			(drawSize.width - hMon0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	return(0);
    }

    CUINT Layout_Monitor_Instructions(Layer *layer, CUINT row)
    {
	LayerDeclare(76) hMon0 = {
		.origin = {	.col = (LOAD_LEAD - 1),
				.row = (row + MAX_ROWS + 1)
		},
		.length = 76,
		.attr ={HWK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code ={' ',						\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,	\
			' ',' ',' ',' ',' ',' ',0x0,0x0,		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,	\
			' ',' ',' ',' ',' ',' ',0x0,0x0,		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,	\
			' ',' ',' ',' ',' ',' ',0x0,0x0,		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' '
		}
	};
	LayerCopyAt(layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	LayerFillAt(layer, (hMon0.origin.col + hMon0.length),
			hMon0.origin.row,
			(drawSize.width - hMon0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	return(0);
    }

    CUINT Layout_Monitor_Common(Layer *layer, CUINT row)
    {
	LayerDeclare(77) hMon0 = {
		.origin = {	.col = (LOAD_LEAD - 1),
				.row = (row + MAX_ROWS + 1)
		},
		.length = 77,
		.attr ={HWK,HWK,HWK,HWK,HWK,				\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code ={' ',' ',' ',' ',' ',				\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' '
		}
	};
	LayerCopyAt(layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	LayerFillAt(layer, (hMon0.origin.col + hMon0.length),
			hMon0.origin.row,
			(drawSize.width - hMon0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	return(0);
    }

    CUINT Layout_Monitor_Package(Layer *layer, CUINT row)
    {
	return(0);
    }

    CUINT Layout_Monitor_Tasks(Layer *layer, CUINT row)
    {
	LayerDeclare( (MAX_WIDTH - LOAD_LEAD + 1) ) hMon0 = {
		.origin = {	.col = (LOAD_LEAD - 1),
				.row = (row + MAX_ROWS + 1)
		},
		.length = (MAX_WIDTH - LOAD_LEAD + 1),
		.attr ={HWK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
		},
		.code ={' ',						\
			' ',' ',' ',' ',0x0,' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' 	\
		}
	};
	LayerCopyAt(layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	cTask[cpu].col = LOAD_LEAD + 8;
	cTask[cpu].row = hMon0.origin.row;

	return(0);
    }

    CUINT Layout_Ruller_Frequency(Layer *layer, CUINT row)
    {
	LayerDeclare(MAX_WIDTH) hFreq0 = {
		.origin = {
			.col = 0,
			.row = row
		},
		.length = drawSize.width,
		.attr = {
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK, \
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code = "--- Freq(MHz) Ratio - Turbo --- "		\
			"C0 ---- C1 ---- C3 ---- C6 ---- C7 --"		\
			"Min TMP Max "					\
			"---------------------------------------------------",
	};

	LayerCopyAt(layer, hFreq0.origin.col, hFreq0.origin.row,
			hFreq0.length, hFreq0.attr, hFreq0.code);

	if (!drawFlag.avgOrPC) {
	    LayerDeclare(MAX_WIDTH) hAvg0 = {
		.origin = {
			.col = 0,
			.row = (row + MAX_ROWS + 1)
		},
		.length = drawSize.width,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code ={'-','-','-','-','-','-',' ','%',		\
			' ','A','v','e','r','a','g','e','s',' ','[',	\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,' ',']',' ',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-'
		}
	    };

	    LayerCopyAt(layer, hAvg0.origin.col, hAvg0.origin.row,
			hAvg0.length, hAvg0.attr, hAvg0.code);
	} else {
	    LayerDeclare(MAX_WIDTH) hPkg0 = {
		.origin = {
			.col = 0,
			.row = (row + MAX_ROWS + 1)
		},
		.length = drawSize.width,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code ={'-','-','-','-','-',' ','%',' ','P','k','g',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-','-','-','-','-',	\
			'-','-','-','-','-','-','-'
		}
	    };

	    LayerCopyAt(layer, hPkg0.origin.col, hPkg0.origin.row,
			hPkg0.length, hPkg0.attr, hPkg0.code);
	}
	row += MAX_ROWS + 2;
	return(row);
    }

    CUINT Layout_Ruller_Instructions(Layer *layer, CUINT row)
    {
	LayerFillAt(layer, 0, row, drawSize.width,
		"------------ IPS -------------- IPC ----"		\
		"---------- CPI ------------------ INST -"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	LayerFillAt(layer, 0, (row + MAX_ROWS + 1),
			drawSize.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));

	row += MAX_ROWS + 2;
	return(row);
    }

    CUINT Layout_Ruller_Cycles(Layer *layer, CUINT row)
    {
	LayerFillAt(layer, 0, row, drawSize.width,
		"-------------- C0:UCC ---------- C0:URC "		\
		"------------ C1 ------------- TSC ------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	LayerFillAt(layer, 0, (row + MAX_ROWS + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += MAX_ROWS + 2;
	return(row);
    }

    CUINT Layout_Ruller_CStates(Layer *layer, CUINT row)
    {
	LayerFillAt(layer, 0, row, drawSize.width,
		"---------------- C1 -------------- C3 --"		\
		"------------ C6 -------------- C7 ------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	LayerFillAt(layer, 0, (row + MAX_ROWS + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += MAX_ROWS + 2;
	return(row);
    }

    CUINT Layout_Ruller_Interrupts(Layer *layer, CUINT row)
    {
	LayerDeclare(MAX_WIDTH) hIntr0 = {
		.origin = {
			.col = 0,
			.row = row
		},
		.length = drawSize.width,
		.attr = {
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			 LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code = "---------- SMI ------------ NMI[ LOCAL   UNKNOWN"\
			"  PCI_SERR#  IO_CHECK] -------------------------"\
			"------------------------------------",
	};
	LayerCopyAt(layer, hIntr0.origin.col, hIntr0.origin.row,
			hIntr0.length, hIntr0.attr, hIntr0.code);

	LayerFillAt(layer, 0, (row + MAX_ROWS + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += MAX_ROWS + 2;
	return(row);
    }

    CUINT Layout_Ruller_Package(Layer *layer, CUINT row)
    {
	LayerFillAt(layer, 0, row, drawSize.width,
		"------------ Cycles ---- State ---------"		\
		"----------- TSC Ratio ------------------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	ATTRIBUTE hPCnnAttr[MAX_WIDTH] = {
		LWK,LWK,LWK,LWK,HDK,					\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,			\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,			\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,			\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
		HBK,HBK,HBK,HBK
	};
	ASCII	hPCnnCode[MAX_WIDTH] = {
		'P','C','0','0',':',					\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',	\
		' ',' ',' ',' '
	};
	ASCII	hCState[7][2] = {
		{'0', '2'},
		{'0', '3'},
		{'0', '6'},
		{'0', '7'},
		{'0', '8'},
		{'0', '9'},
		{'1', '0'}
	};

	unsigned int idx;
	for (idx = 0; idx < 7; idx++)
	{
		hPCnnCode[2] = hCState[idx][0];
		hPCnnCode[3] = hCState[idx][1];
		LayerCopyAt(	layer, 0, (row + idx + 1),
				drawSize.width, hPCnnAttr, hPCnnCode);
	}

	LayerDeclare(MAX_WIDTH) hUncore = {
	    .origin = {
		.col = 0,
		.row = row + 8
	    },
	    .length = drawSize.width,
	    .attr = {
		LWK,LWK,LWK,LWK,HDK,				\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,					\
		LWK,LWK,LWK,LWK,LWK,LWK,HDK,			\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
		HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },
	    .code = {
		' ','T','S','C',':',				\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
		' ',' ',' ',					\
		'U','N','C','O','R','E',':',			\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
		' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
		' ',' ',' ',' ',' ',' ',' ',' ',' '
	    }
	};

	LayerCopyAt(layer, hUncore.origin.col, hUncore.origin.row,
		hUncore.length, hUncore.attr, hUncore.code);

	LayerFillAt(layer, 0, (row + 9),
		drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += 2 + 8;
	return(row);
    }

    CUINT Layout_Ruller_Tasks(Layer *layer, CUINT row)
    {
	LayerDeclare(MAX_WIDTH) hTask0 = {
	    .origin = {
		.col = 0,
		.row = row
	    },
	    .length = drawSize.width,
	    .attr = {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK, \
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LDK, \
		LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK, \
		LDK,LDK,LDK,LDK,LDK,LDK,LDK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
	    },
	    .code =
		"--- Freq(MHz) --- Tasks                 "	\
		"   -------------------------------------"	\
		"----------------------------------------"	\
		"------------"
	};

	LayerDeclare(21) hTask1 = {
		.origin = {.col = 23, .row = row},
		.length = 21,
	};

	struct {
		ATTRIBUTE attr[21];
		ASCII code[21];
	} hSort[SORTBYCOUNT] = {
	  {
	    .attr = {
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
		LWK,LCK,LCK,LCK,LCK,LCK,HDK,LWK, LWK,LWK,LWK
	    },
	    .code = {
		'(','s','o','r','t','e','d',' ', 'b','y',	\
		' ','S','t','a','t','e',')',' ', '-','-','-'
	    }
	  },
	  {
	    .attr = {
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
		LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK
	    },
	  .code = {
		'(','s','o','r','t','e','d',' ', 'b','y',	\
		' ','R','u','n','T','i','m','e', ')',' ','-'
	    }
	  },
	  {
	    .attr = {
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
		LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, LCK,HDK,LWK
	    },
	    .code = {
		'(','s','o','r','t','e','d',' ', 'b','y',	\
		' ','U','s','e','r','T','i','m', 'e',')',' '
	    }
	  },
	  {
	    .attr = {
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
		LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK
	    },
	    .code = {
		'(','s','o','r','t','e','d',' ', 'b','y',	\
		' ','S','y','s','T','i','m','e', ')',' ','-'
	    }
	  },
	  {
	    .attr = {
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
		LWK,LCK,LCK,LCK,HDK,LWK,LWK,LWK, LWK,LWK,LWK
	    },
	    .code = {
		'(','s','o','r','t','e','d',' ', 'b','y',	\
		' ','P','I','D',')',' ','-','-', '-','-','-'
	    }
	  },
	  {
	    .attr = {
		HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
		LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK
	    },
	    .code = {
		'(','s','o','r','t','e','d',' ', 'b','y',	\
		' ','C','o','m','m','a','n','d', ')',' ','-'
	    }
	  }
	};

	memcpy(hTask1.attr, hSort[Shm->SysGate.sortByField].attr,hTask1.length);
	memcpy(hTask1.code, hSort[Shm->SysGate.sortByField].code,hTask1.length);

	LayerDeclare(15) hTask2 = {
		.origin = {
			.col = (drawSize.width - 18),
			.row = (row + MAX_ROWS + 1)
		},
		.length = 15,
	};

	struct {
		ATTRIBUTE attr[15];
		ASCII code[15];
	} hReverse[2] = {
	  {
	    .attr = {
		LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK
	    },
	  .code = {
		' ', 'R','e','v','e','r','s','e',' ','[','O','F','F',']',' '
	    }
	  },
	  {
	    .attr = {
		LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LCK,LCK,LCK,HDK,LWK
	    },
	  .code = {
		' ', 'R','e','v','e','r','s','e',' ','[',' ','O','N',']',' '
	    }
	  }
	};

	memcpy(hTask2.attr, hReverse[Shm->SysGate.reverseOrder].attr,
			hTask2.length);
	memcpy(hTask2.code, hReverse[Shm->SysGate.reverseOrder].code,
			hTask2.length);

	LayerDeclare(13) hTask3 = {
	    .origin = {
		.col = (drawSize.width - 34),
		.row = (row + MAX_ROWS + 1)
	    },
	    .length = 13,
	    .attr = {
		LWK,_HCK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK
	    },
	    .code = {
		' ', 'V','a','l','u','e',' ','[',' ',' ',' ',']',' '
	    }
	};

	struct {
		ATTRIBUTE attr[3];
		ASCII code[3];
	} hTaskVal[2] = {
		{
			.attr = {
				LWK,LWK,LWK
			},
			.code = {
				'O','F','F'
			}
		},
		{
			.attr = {
				LCK,LCK,LCK
			},
			.code = {
				' ','O','N'
			}
		}
	};

	memcpy(&hTask3.attr[8], hTaskVal[drawFlag.taskVal].attr, 3);
	memcpy(&hTask3.code[8], hTaskVal[drawFlag.taskVal].code, 3);

	LayerDeclare(22) hTrack0 = {
		.origin = {
			.col = 55,
			.row = row
		},
		.length = 22,
		.attr = {
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,HDK,LWK, LWK,LWK,	\
			LWK,LWK,HDK,LWK
		},
		.code = {
			' ','T','r','a','c','k','i', 'n','g',	\
			' ','P','I','D',' ','[',' ', 'O','F',	\
			'F',' ',']',' '
		}
	};
	if (Shm->SysGate.trackTask) {
		memset(&hTrack0.attr[15], MakeAttr(CYAN, 0, BLACK, 0).value, 5);
		sprintf(buffer, "%5d", Shm->SysGate.trackTask);
		memcpy(&hTrack0.code[15], buffer, 5);
	}

	LayerCopyAt(layer, hTask0.origin.col, hTask0.origin.row,
			hTask0.length, hTask0.attr, hTask0.code);

	LayerCopyAt(layer, hTask1.origin.col, hTask1.origin.row,
			hTask1.length, hTask1.attr, hTask1.code);

	LayerFillAt(layer, 0, (row + MAX_ROWS + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	LayerCopyAt(layer, hTask2.origin.col, hTask2.origin.row,
			hTask2.length, hTask2.attr, hTask2.code);

	LayerCopyAt(layer, hTask3.origin.col, hTask3.origin.row,
			hTask3.length, hTask3.attr, hTask3.code);

	LayerCopyAt(layer, hTrack0.origin.col, hTrack0.origin.row,
			hTrack0.length, hTrack0.attr, hTrack0.code);

	row += MAX_ROWS + 2;
	return(row);
    }

    CUINT Layout_Ruller_Voltage(Layer *layer, CUINT row)
    {
	const CUINT tab = LOAD_LEAD + 24 + 6;
	CUINT vsh = row + PWR_DOMAIN(SIZE) + 1;

	LayerDeclare(MAX_WIDTH) hVolt0 = {
		.origin = {
			.col = 0,
			.row = row
		},
		.length = drawSize.width,
		.attr = {
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK, \
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code = "--- Freq(MHz) - VID - Vcore ------------"	\
			"------ Energy(J) ----- Power(W) --------"	\
			"----------------------------------------"	\
			"------------"
	};
	LayerCopyAt(layer, hVolt0.origin.col, hVolt0.origin.row,
			hVolt0.length, hVolt0.attr, hVolt0.code);

	ASCII hDomain[PWR_DOMAIN(SIZE)][7] = {
		{'P','a','c','k','a','g','e'},
		{'C','o','r','e','s',' ',' '},
		{'U','n','c','o','r','e',' '},
		{'M','e','m','o','r','y',' '}
	};
	enum PWR_DOMAIN pw;
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
	{
		LayerDeclare(39) hPower0 = {
			.origin = {
				.col = tab,
				.row = row + pw + 1
			},
			.length = 39,
			.attr = {
				LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
				HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK, \
				HWK,HWK,HWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK, \
				HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
			},
			.code = "            0.000000000     0.000000000"
		};
		memcpy(&hPower0.code[0], hDomain[pw], 7);

		LayerCopyAt(layer, hPower0.origin.col, hPower0.origin.row,
				hPower0.length, hPower0.attr, hPower0.code);
	}
	if (MAX_ROWS > 4)
		vsh += MAX_ROWS - 4;

	LayerFillAt(layer,0,vsh,drawSize.width,hLine,MakeAttr(WHITE,0,BLACK,0));

	row += CUMAX(MAX_ROWS, 4) + 2;
	return(row);
    }

    CUINT Layout_Ruller_Slice(Layer *layer, CUINT row)
    {
	LayerDeclare(MAX_WIDTH) hSlice0 = {
		.origin = {
			.col = 0,
			.row = row
		},
		.length = drawSize.width,
		.attr = {
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK, \
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code = "--- Freq(MHz) ------ Cycles -- Instructi"	\
			"ons ------------ TSC ------------ PMC0 -"	\
			"----------------------------------------"	\
			"------------"
	};
	LayerCopyAt(layer, hSlice0.origin.col, hSlice0.origin.row,
			hSlice0.length, hSlice0.attr, hSlice0.code);

	LayerFillAt(layer, 0, (row + MAX_ROWS + 1),
			drawSize.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));

	row += MAX_ROWS + 2;
	return(row);
    }

    void Layout_Footer(Layer *layer, CUINT row, signed int *processorHot)
    {
	CUINT col = 0;
	size_t len;

	LayerDeclare(61) hTech0 = {
		.origin = {.col = 0, .row = row}, .length = 14,
		.attr={LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK},
		.code={'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',','},
	};

	const ATTRIBUTE Pwr[] = {
		MakeAttr(BLACK, 0, BLACK, 1),
		MakeAttr(GREEN, 0, BLACK, 1),
		MakeAttr(BLUE,  0, BLACK, 1)
	};
	const struct { ASCII *code; ATTRIBUTE attr; } TSC[] = {
		{(ASCII *) "  TSC  ",  MakeAttr(BLACK, 0, BLACK, 1)},
		{(ASCII *) "TSC-VAR" , MakeAttr(BLUE,  0, BLACK, 1)},
		{(ASCII *) "TSC-INV" , MakeAttr(GREEN, 0, BLACK, 1)}
	};

	hTech0.code[ 6] = TSC[Shm->Proc.Features.InvariantTSC].code[0];
	hTech0.code[ 7] = TSC[Shm->Proc.Features.InvariantTSC].code[1];
	hTech0.code[ 8] = TSC[Shm->Proc.Features.InvariantTSC].code[2];
	hTech0.code[ 9] = TSC[Shm->Proc.Features.InvariantTSC].code[3];
	hTech0.code[10] = TSC[Shm->Proc.Features.InvariantTSC].code[4];
	hTech0.code[11] = TSC[Shm->Proc.Features.InvariantTSC].code[5];
	hTech0.code[12] = TSC[Shm->Proc.Features.InvariantTSC].code[6];

	hTech0.attr[ 6] = hTech0.attr[ 7]=hTech0.attr[ 8] =
	hTech0.attr[ 9] = hTech0.attr[10]=hTech0.attr[11] =
	hTech0.attr[12] = TSC[Shm->Proc.Features.InvariantTSC].attr;

	LayerCopyAt(layer, hTech0.origin.col, hTech0.origin.row,
			hTech0.length, hTech0.attr, hTech0.code);

	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
	    LayerDeclare(69) hTech1 = {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=55,
		.attr ={HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK, \
			HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,	\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK
		},
		.code ={'H','T','T',',','E','I','S','T',',','I','D','A',',', \
			'T','U','R','B','O',',','C','1','E',',',	\
			' ','P','M',',','C','3','A',',','C','1','A',',',\
			'C','3','U',',','C','1','U',',',		\
			'T','M','1',',','T','M','2',',','H','O','T',']'
		},
	    };

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =
					Pwr[Shm->Proc.Features.HyperThreading];

		const ATTRIBUTE TM1[] = {
			MakeAttr(BLACK, 0, BLACK, 1),
			MakeAttr(BLUE,  0, BLACK, 1),
			MakeAttr(WHITE, 0, BLACK, 1),
			MakeAttr(GREEN, 0, BLACK, 1)
		};
		const ATTRIBUTE TM2[] = {
			MakeAttr(BLACK, 0, BLACK, 1),
			MakeAttr(BLUE,  0, BLACK, 1),
			MakeAttr(WHITE, 0, BLACK, 1),
			MakeAttr(GREEN, 0, BLACK, 1)
		};

	    hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] =		\
	    hTech1.attr[7] = Pwr[Shm->Proc.Technology.EIST];

	    hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] =	\
				Pwr[Shm->Proc.Features.Power.EAX.TurboIDA];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =	\
	    hTech1.attr[16] = hTech1.attr[17] = Pwr[Shm->Proc.Technology.Turbo];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] =	\
						Pwr[Shm->Proc.Technology.C1E];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[23] = buffer[0];
	    hTech1.code[24] = buffer[1];
	    hTech1.code[25] = buffer[2];

	    hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =	\
						Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] =	\
						Pwr[Shm->Proc.Technology.C3A];

	    hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] =	\
						Pwr[Shm->Proc.Technology.C1A];

	    hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] =	\
						Pwr[Shm->Proc.Technology.C3U];

	    hTech1.attr[39] = hTech1.attr[40] = hTech1.attr[41] =	\
						Pwr[Shm->Proc.Technology.C1U];

	    hTech1.attr[43] = hTech1.attr[44] = hTech1.attr[45] =	\
			TM1[Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1];

	    hTech1.attr[47] = hTech1.attr[48] = hTech1.attr[49] =	\
			TM2[Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2];

	    if ( (*processorHot) != -1 ) {
		hTech1.attr[51] = hTech1.attr[52] = hTech1.attr[53] =	\
						MakeAttr(RED, 0, BLACK, 1);
	    }
	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
				hTech1.origin.row,
				(drawSize.width - hTech0.length-hTech1.length),
				hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	} else {
	  if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD) {
	    LayerDeclare(65) hTech1 = {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=39,
		.attr={HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK,\
		       HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,\
		       HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK
		},
		.code={'S','M','T',',','P','o','w','e','r','N','o','w',',',\
		       'B','O','O','S','T',',','C','1','E',',',' ','P','M',',',\
		       'D','T','S',',','T','T','P',',','H','O','T',']'
		},
	    };

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =		\
					Pwr[Shm->Proc.Features.HyperThreading];

	  hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[ 6] = hTech1.attr[ 7]= \
	  hTech1.attr[8] = hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11]= \
					Pwr[(Shm->Proc.PowerNow == 0b11)];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =	\
	    hTech1.attr[16] = hTech1.attr[17] = Pwr[Shm->Proc.Technology.Turbo];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] =	\
						Pwr[Shm->Proc.Technology.C1E];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[23] = buffer[0];
	    hTech1.code[24] = buffer[1];
	    hTech1.code[25] = buffer[2];

	    hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =	\
						Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] =	\
				Pwr[(Shm->Proc.Features.AdvPower.EDX.TS != 0)];

	    hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] =	\
				Pwr[(Shm->Proc.Features.AdvPower.EDX.TTP != 0)];

	    if ( (*processorHot) != -1 ) {
		hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] =	\
						MakeAttr(RED, 0, BLACK, 1);
	    }
	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
				hTech1.origin.row,
				(drawSize.width - hTech0.length-hTech1.length),
				hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	  }
	}
	row++;

	len = sprintf(  buffer, "%s",
			BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) ?
			Shm->SysGate.sysname : "SysGate");

	LayerFillAt(	layer, col, row,
			len, buffer,
			MakeAttr(CYAN, 0, BLACK, 0));
	col += len;

	LayerAt(layer, attr, col, row) = MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, col, row) = 0x20;

	col++;

	LayerAt(layer, attr, col, row) = MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, col, row) = '[';

	col++;

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		len = sprintf(buffer, "%hu.%hu.%hu",
				Shm->SysGate.kernel.version,
				Shm->SysGate.kernel.major,
				Shm->SysGate.kernel.minor);

		LayerFillAt(	layer, col, row,
				len, buffer,
				MakeAttr(WHITE, 0, BLACK, 1));
		col += len;
	} else {
		LayerFillAt(	layer, col, row,
				3, "OFF",
				MakeAttr(RED, 0, BLACK, 0));
		col += 3;
	}
	LayerAt(layer, attr, col, row) = MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, col, row) = ']';

	col++;

	LayerDeclare(42) hSys1 = {
	    .origin = {.col = (drawSize.width - 42), .row = row}, .length = 42,
	    .attr = {
		LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HDK, \
		LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK, \
		HWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,HDK
		},
	    .code = {
		'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',']', \
		' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ', \
		' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ','K','B',']'
		},
	};

	len = hSys1.origin.col - col;
	if ((signed int)len  > 0)
	    LayerFillAt(layer, col, hSys1.origin.row,
			len, hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hSys1.origin.col, hSys1.origin.row,
			hSys1.length, hSys1.attr, hSys1.code);

	// Reset Tasks count & Memory usage
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		PrintTaskMemory(layer, row,
				Shm->SysGate.taskCount,
				Shm->SysGate.memInfo.freeram,
				Shm->SysGate.memInfo.totalram);
    }

    void Layout_Load_UpperView(Layer *layer, CUINT row)
    {
	sprintf(buffer, "%-2u", cpu);
	LayerAt(layer, attr, 0, row) = MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, 0, row) = '#';
	LayerAt(layer, code, 1, row) = buffer[0];
	LayerAt(layer, code, 2, row) = buffer[1];

	LayerAt(layer, attr, 3, row) = MakeAttr(YELLOW, 0, BLACK, 1);
	LayerAt(layer, code, 3, row) = 0x20;
    }

    void Layout_Load_LowerView(Layer *layer, CUINT row)
    {
	LayerAt(layer, attr, 0, (1 + row + MAX_ROWS))=MakeAttr(WHITE,0,BLACK,0);
	LayerAt(layer, code, 0, (1 + row + MAX_ROWS)) = '#';
	LayerAt(layer, code, 1, (1 + row + MAX_ROWS)) = buffer[0];
	LayerAt(layer, code, 2, (1 + row + MAX_ROWS)) = buffer[1];
    }

    void Draw_Load(Layer *layer, CUINT row)
    {
	if (!BITVAL(Shm->Cpu[cpu].OffLine, HW))
	{
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
		// Upper view area
		CUINT	bar0 =((Flop->Relative.Ratio > maxRatio ?
				maxRatio:Flop->Relative.Ratio)
				* loadWidth) / maxRatio,
			bar1 = loadWidth - bar0;
		// Print the Per Core BCLK indicator (yellow)
		LayerAt(layer, code, (LOAD_LEAD - 1), row) =		\
				(iClock == (cpu - cpuScroll)) ? '~' : 0x20;
		// Draw the relative Core frequency ratio
		LayerFillAt(layer, LOAD_LEAD, row,
				bar0, hBar,
				MakeAttr((Flop->Relative.Ratio > medianRatio ?
					RED : Flop->Relative.Ratio > minRatio ?
						YELLOW : GREEN),
					0, BLACK, 1));
		// Pad with blank characters
		LayerFillAt(layer, (bar0 + LOAD_LEAD), row,
				bar1, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}
    }

    CUINT Draw_Monitor_Frequency(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer,
		"%7.2f" " (" "%5.2f" ") "			\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
		"%-3u" "/" "%3u" "/" "%3u",
		Flop->Relative.Freq,
		Flop->Relative.Ratio,
		100.f * Flop->State.Turbo,
		100.f * Flop->State.C0,
		100.f * Flop->State.C1,
		100.f * Flop->State.C3,
		100.f * Flop->State.C6,
		100.f * Flop->State.C7,
		Shm->Cpu[cpu].PowerThermal.Limit[0],
		Flop->Thermal.Temp,
		Shm->Cpu[cpu].PowerThermal.Limit[1]);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	ATTRIBUTE warning ={.fg=WHITE, .un=0, .bg=BLACK, .bf=1};

	if (Flop->Thermal.Temp <= Shm->Cpu[cpu].PowerThermal.Limit[0])
			warning = MakeAttr(BLUE, 0, BLACK, 1);
	else {
		if (Flop->Thermal.Temp >= Shm->Cpu[cpu].PowerThermal.Limit[1])
			warning = MakeAttr(YELLOW, 0, BLACK, 0);
	}
	if (Flop->Thermal.Trip) {
		warning = MakeAttr(RED, 0, BLACK, 1);
	}
	LayerAt(layer, attr, (LOAD_LEAD + 69), row) =		\
	LayerAt(layer, attr, (LOAD_LEAD + 70), row) =		\
	LayerAt(layer, attr, (LOAD_LEAD + 71), row) = warning;

	return(0);
    }

    CUINT Draw_Monitor_Instructions(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer,
			"%17.6f" "/s"				\
			"%17.6f" "/c"				\
			"%17.6f" "/i"				\
			"%18llu",
			Flop->State.IPS,
			Flop->State.IPC,
			Flop->State.CPI,
			Flop->Delta.INST);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
    }

    CUINT Draw_Monitor_Cycles(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer,
			"%18llu%18llu%18llu%18llu",
			Flop->Delta.C0.UCC,
			Flop->Delta.C0.URC,
			Flop->Delta.C1,
			Flop->Delta.TSC);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
    }

    CUINT Draw_Monitor_CStates(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer,
			"%18llu%18llu%18llu%18llu",
			Flop->Delta.C1,
			Flop->Delta.C3,
			Flop->Delta.C6,
			Flop->Delta.C7);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
    }

    CUINT Draw_Monitor_Package(Layer *layer, CUINT row)
    {
	return(0);
    }

	ATTRIBUTE runColor[] = {
		HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
		HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
		HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
		HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
		HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK
	}, unintColor[] = {
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
		HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK
	}, zombieColor[] = {
		LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,\
		LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,\
		LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,\
		LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,\
		LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW
	}, sleepColor[] = {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
	}, waitColor[] = {
		HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
		HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
		HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
		HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
		HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK
	}, otherColor[] = {
		LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,\
		LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,\
		LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,\
		LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,\
		LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK
	}, trackerColor[] = {
		LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
		LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
		LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
		LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
		LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC
	};

    ATTRIBUTE *stateToSymbol(short int state, char stateStr[])
    {
	ATTRIBUTE *attrib[14] = {
	/* R */runColor,/* S */sleepColor,/* D */unintColor,/* T */waitColor,
	/* t */waitColor,/* X */waitColor,/* Z */zombieColor,/* P*/waitColor,
	/* I */waitColor,/* K */sleepColor,/* W */runColor,/* i */waitColor,
	/* N */runColor,/* m */otherColor
	}, *stateAttr = otherColor;
	const char symbol[14] = "RSDTtXZPIKWiNm";
	unsigned short idx, jdx = 0;

	if (BITBSR(state, idx) == 1) {
		stateStr[jdx++] = symbol[0];
		stateAttr = attrib[0];
	} else
		do {
			BITCLR(LOCKLESS, state, idx);
			stateStr[jdx++] = symbol[1 + idx];
			stateAttr = attrib[1 + idx];
		} while (!BITBSR(state, idx));
	stateStr[jdx] = '\0';
	return(stateAttr);
    }

    CUINT Draw_Monitor_Tasks(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer, "%7.2f", Flop->Relative.Freq);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	cTask[cpu].col = LOAD_LEAD + 8;

	return(0);
    }

    CUINT Draw_Monitor_Interrupts(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer, "%10u", Flop->Counter.SMI);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	if (Shm->Registration.nmi) {
		len = sprintf(buffer,
				"%10u%10u%10u%10u",
				Flop->Counter.NMI.LOCAL,
				Flop->Counter.NMI.UNKNOWN,
				Flop->Counter.NMI.PCISERR,
				Flop->Counter.NMI.IOCHECK);
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 24),row), buffer, len);
	}
	return(0);
    }

    CUINT Draw_Monitor_Voltage(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	if (Flop->Voltage.VID != 0)
		len = sprintf(buffer,	"%7.2f "	\
					"%7d   %5.4f",
					Flop->Relative.Freq,
					Flop->Voltage.VID,
					Flop->Voltage.Vcore);
	else
		len = sprintf(buffer, "%7.2f ", Flop->Relative.Freq);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
    }

    CUINT Draw_Monitor_Slice(Layer *layer, CUINT row)
    {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer,
			"%7.2f "				\
			"%16llu%16llu%18llu%18llu",
			Flop->Relative.Freq,
			Shm->Cpu[cpu].Slice.Delta.TSC,
			Shm->Cpu[cpu].Slice.Delta.INST,
			Shm->Cpu[cpu].Slice.Counter[1].TSC,
			Shm->Cpu[cpu].Slice.Counter[1].INST);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
    }

    CUINT Draw_AltMonitor_Frequency(Layer *layer, CUINT row)
    {
	size_t len;

	row += 2 + MAX_ROWS;
	if (!drawFlag.avgOrPC) {
		len = sprintf(buffer,
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%",
			100.f * Shm->Proc.Avg.Turbo,
			100.f * Shm->Proc.Avg.C0,
			100.f * Shm->Proc.Avg.C1,
			100.f * Shm->Proc.Avg.C3,
			100.f * Shm->Proc.Avg.C6,
			100.f * Shm->Proc.Avg.C7);
		memcpy(&LayerAt(layer, code, 20, row), buffer, len);
	} else {
		len = sprintf(buffer,
			"  c2:%-5.1f" "  c3:%-5.1f" "  c6:%-5.1f"	\
			"  c7:%-5.1f" "  c8:%-5.1f" "  c9:%-5.1f"	\
			" c10:%-5.1f",
			100.f * Shm->Proc.State.PC02,
			100.f * Shm->Proc.State.PC03,
			100.f * Shm->Proc.State.PC06,
			100.f * Shm->Proc.State.PC07,
			100.f * Shm->Proc.State.PC08,
			100.f * Shm->Proc.State.PC09,
			100.f * Shm->Proc.State.PC10);
		memcpy(&LayerAt(layer, code, 11, row), buffer, len);
	}
	row += 2;
	return(row);
    }

    CUINT Draw_AltMonitor_Common(Layer *layer, CUINT row)
    {
	row += 2 + TOP_FOOTER_ROW + MAX_ROWS;
	return(row);
    }

    CUINT Draw_AltMonitor_Package(Layer *layer, CUINT row)
    {
	struct PKG_FLIP_FLOP *Pkg = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	CUINT bar0, bar1, margin = loadWidth - 28;
	size_t len;

	row += 2;
/* PC02 */
	bar0 = Shm->Proc.State.PC02 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC02, 100.f * Shm->Proc.State.PC02,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, row), buffer, len);
/* PC03 */
	bar0 = Shm->Proc.State.PC03 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC03, 100.f * Shm->Proc.State.PC03,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5,(row+1)), buffer, len);
/* PC06 */
	bar0 = Shm->Proc.State.PC06 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC06, 100.f * Shm->Proc.State.PC06,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5,(row+2)), buffer, len);
/* PC07 */
	bar0 = Shm->Proc.State.PC07 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC07, 100.f * Shm->Proc.State.PC07,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5,(row+3)), buffer, len);
/* PC08 */
	bar0 = Shm->Proc.State.PC08 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC08, 100.f * Shm->Proc.State.PC08,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5,(row+4)), buffer, len);
/* PC09 */
	bar0 = Shm->Proc.State.PC09 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC09, 100.f * Shm->Proc.State.PC09,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5,(row+5)), buffer, len);
/* PC10 */
	bar0 = Shm->Proc.State.PC10 * margin;
	bar1 = margin - bar0;

	len = sprintf(buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			Pkg->Delta.PC10, 100.f * Shm->Proc.State.PC10,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5,(row+6)), buffer, len);
/* TSC & UNCORE */
	len = sprintf(buffer, "%18llu", Pkg->Delta.PTSC);
	memcpy(&LayerAt(layer, code, 5,(row+7)), buffer, len);
	len = sprintf(buffer, "UNCORE:%18llu", Pkg->Uncore.FC0);
	memcpy(&LayerAt(layer, code,50,(row+7)), buffer, len);

	row += 2 + 8;
	return(row);
    }

  CUINT Draw_AltMonitor_Tasks(Layer *layer, CUINT row)
  {
    row += 2 + MAX_ROWS;
    if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
	size_t len;
	unsigned int idx;
	char stateStr[16];
	ATTRIBUTE *stateAttr;

     for (idx = 0; idx < Shm->SysGate.taskCount; idx++)
      if (!BITVAL(Shm->Cpu[Shm->SysGate.taskList[idx].wake_cpu].OffLine, OS)
	&& (Shm->SysGate.taskList[idx].wake_cpu >= cpuScroll)
	&& (Shm->SysGate.taskList[idx].wake_cpu < (cpuScroll + MAX_ROWS)))
	{
	unsigned int ldx = 2;
	CSINT dif=drawSize.width-cTask[Shm->SysGate.taskList[idx].wake_cpu].col;

	if (dif > 0)
	{
	  stateAttr = stateToSymbol(Shm->SysGate.taskList[idx].state, stateStr);

	  if (Shm->SysGate.taskList[idx].pid == Shm->SysGate.trackTask) {
		stateAttr = trackerColor;
	  }
	  if (!drawFlag.taskVal) {
		len = sprintf(buffer, "%s",
				Shm->SysGate.taskList[idx].comm);
	    } else {
	      switch (Shm->SysGate.sortByField) {
	      case F_STATE:
		len = sprintf(buffer, "%s(%s)",
				Shm->SysGate.taskList[idx].comm,
				stateStr);
		break;
	      case F_RTIME:
		len = sprintf(buffer, "%s(%llu)",
				Shm->SysGate.taskList[idx].comm,
				Shm->SysGate.taskList[idx].runtime);
		break;
	      case F_UTIME:
		len = sprintf(buffer, "%s(%llu)",
				Shm->SysGate.taskList[idx].comm,
				Shm->SysGate.taskList[idx].usertime);
		break;
	      case F_STIME:
		len = sprintf(buffer, "%s(%llu)",
				Shm->SysGate.taskList[idx].comm,
				Shm->SysGate.taskList[idx].systime);
		break;
	      case F_PID:
		// fallthrough
	      case F_COMM:
		// fallthrough
		len = sprintf(buffer, "%s(%d)",
				Shm->SysGate.taskList[idx].comm,
				Shm->SysGate.taskList[idx].pid);
		break;
	      }
	    }
	    if (dif >= len) {
		LayerCopyAt(layer,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row,
			len, stateAttr, buffer);

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col += len;
	    } else if (dif > 0) {
		LayerCopyAt(layer,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row,
			dif, stateAttr, buffer);

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col += dif;
	    }
	    while ((dif = drawSize.width
			- cTask[Shm->SysGate.taskList[idx].wake_cpu].col) > 0
		&& ldx--)
	    {
		LayerAt(layer, attr,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row) = \
						MakeAttr(WHITE, 0, BLACK, 0);
		LayerAt(layer, code,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row) = 0x20;

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col++;
	    }
	}
      }
    }
    row += 2;
    return(row);
  }

    CUINT Draw_AltMonitor_Power(Layer *layer, CUINT row)
    {
	const CUINT col = LOAD_LEAD + 24 + 16;
	const CUINT tab = 13 + 3;

	row += 2;
	sprintf(buffer,
		"%13.9f" "%13.9f" "%13.9f" "%13.9f"		\
		"%13.9f" "%13.9f" "%13.9f" "%13.9f",
		Shm->Proc.State.Energy[PWR_DOMAIN(PKG)],
		Shm->Proc.State.Energy[PWR_DOMAIN(CORES)],
		Shm->Proc.State.Energy[PWR_DOMAIN(UNCORE)],
		Shm->Proc.State.Energy[PWR_DOMAIN(RAM)],
		Shm->Proc.State.Power[PWR_DOMAIN(PKG)],
		Shm->Proc.State.Power[PWR_DOMAIN(CORES)],
		Shm->Proc.State.Power[PWR_DOMAIN(UNCORE)],
		Shm->Proc.State.Power[PWR_DOMAIN(RAM)]);
	memcpy(&LayerAt(layer, code, col     , row)   , &buffer[ 0], 13);
	memcpy(&LayerAt(layer, code,(col+tab), row)   , &buffer[52], 13);
	memcpy(&LayerAt(layer, code, col     ,(row+1)), &buffer[13], 13);
	memcpy(&LayerAt(layer, code,(col+tab),(row+1)), &buffer[65], 13);
	memcpy(&LayerAt(layer, code, col     ,(row+2)), &buffer[26], 13);
	memcpy(&LayerAt(layer, code,(col+tab),(row+2)), &buffer[78], 13);
	memcpy(&LayerAt(layer, code, col     ,(row+3)), &buffer[39], 13);
	memcpy(&LayerAt(layer, code,(col+tab),(row+3)), &buffer[91], 13);

	row += 2 + CUMAX(MAX_ROWS, 4);
	return(row);
    }

    void Draw_Footer(Layer *layer, CUINT row)
    {	// Update Footer view area
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)
	&& (Shm->SysGate.tickStep == Shm->SysGate.tickReset)) {
		if ((prevTaskCount != Shm->SysGate.taskCount)
		 || (prevFreeRAM != Shm->SysGate.memInfo.freeram)) {
			prevTaskCount = Shm->SysGate.taskCount;
			prevFreeRAM = Shm->SysGate.memInfo.freeram;

			PrintTaskMemory(layer, row,
					Shm->SysGate.taskCount,
					Shm->SysGate.memInfo.freeram,
					Shm->SysGate.memInfo.totalram);
		}
	}
    }

    void Draw_Header(Layer *layer, CUINT row)
    {	// Update Header view area
	struct FLIP_FLOP *Flop = \
	    &Shm->Cpu[Shm->Proc.Top].FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];
	// Print the Top value if delta exists with the previous one
	if (!drawFlag.clkOrLd) { // Frequency MHz
	    if (prevTopFreq != Flop->Relative.Freq) {
		prevTopFreq = Flop->Relative.Freq;

		Clock2LCD(layer,0,row,Flop->Relative.Freq,Flop->Relative.Ratio);
	    }
	} else { // C0 C-State % load
		if (prevTopLoad != Shm->Proc.Avg.C0) {
			double percent = 100.f * Shm->Proc.Avg.C0;
			prevTopLoad = Shm->Proc.Avg.C0;

			Load2LCD(layer, 0, row, percent);
		}
	}
	// Print the focus BCLK
	row += 2;
	memcpy(&LayerAt(layer, code, 26, row), hBClk[iClock + cpuScroll], 11);
    }

#define Illuminates_CPU(_layer, _row, fg, bg, hi)			\
({									\
	LayerAt(_layer, attr, 1, _row) =				\
		LayerAt(_layer, attr, 1, (1 + _row + MAX_ROWS)) =	\
						MakeAttr(fg, 0, bg, hi);\
									\
	LayerAt(_layer, attr, 2, _row) =				\
		LayerAt(_layer, attr, 2, (1 + _row + MAX_ROWS)) =	\
						MakeAttr(fg, 0, bg, hi);\
})

    VIEW_FUNC Matrix_Layout_Monitor[VIEW_SIZE] = {
	Layout_Monitor_Frequency,
	Layout_Monitor_Instructions,
	Layout_Monitor_Common,
	Layout_Monitor_Common,
	Layout_Monitor_Package,
	Layout_Monitor_Tasks,
	Layout_Monitor_Common,
	Layout_Monitor_Common,
	Layout_Monitor_Common
    };

    VIEW_FUNC Matrix_Layout_Ruller[VIEW_SIZE] = {
	Layout_Ruller_Frequency,
	Layout_Ruller_Instructions,
	Layout_Ruller_Cycles,
	Layout_Ruller_CStates,
	Layout_Ruller_Package,
	Layout_Ruller_Tasks,
	Layout_Ruller_Interrupts,
	Layout_Ruller_Voltage,
	Layout_Ruller_Slice
    };

  void Layout_Header_DualView_Footer(Layer *layer)
  {
	signed int processorHot = -1;
	CUINT row = 0;

	loadWidth = drawSize.width - LOAD_LEAD;

	Layout_Header(layer, row);

	row += TOP_HEADER_ROW;

	Layout_Ruller_Load(layer, row);

    for (cpu = cpuScroll; cpu < (cpuScroll + MAX_ROWS); cpu++)
    {
	row++;

	Layout_Load_UpperView(layer, row);

	if (drawFlag.view != V_PACKAGE)
		Layout_Load_LowerView(layer, row);

      if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
      {
	struct FLIP_FLOP *Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	// Store thermal throttling
	if (Flop->Thermal.Trip && (processorHot == -1)) {
		processorHot = cpu;
	}
	if (cpu == Shm->Proc.Service.Core)
		Illuminates_CPU(layer, row, CYAN, BLACK, 1);
	else if (cpu == Shm->Proc.Service.Thread)
		Illuminates_CPU(layer, row, CYAN, BLACK, 1);
	else
		Illuminates_CPU(layer, row, CYAN, BLACK, 0);

	Matrix_Layout_Monitor[drawFlag.view](layer, row);
      } else {
	Illuminates_CPU(layer, row, BLUE, BLACK, 0);

	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), row,
			(drawSize.width - LOAD_LEAD + 1));

	ClearGarbage(	dLayer, attr,
			(LOAD_LEAD - 1), (row + MAX_ROWS + 1),
			(drawSize.width - LOAD_LEAD + 1));

	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), (row + MAX_ROWS + 1),
			(drawSize.width - LOAD_LEAD + 1));
      }

	Dec2Digit(Shm->Cpu[cpu].Clock.Hz, digit);

	hBClk[cpu][ 0] = digit[0] + '0';
	hBClk[cpu][ 1] = digit[1] + '0';
	hBClk[cpu][ 2] = digit[2] + '0';
	hBClk[cpu][ 4] = digit[3] + '0';
	hBClk[cpu][ 5] = digit[4] + '0';
	hBClk[cpu][ 6] = digit[5] + '0';
	hBClk[cpu][ 8] = digit[6] + '0';
	hBClk[cpu][ 9] = digit[7] + '0';
	hBClk[cpu][10] = digit[8] + '0';
    }
	row++;

	row = Matrix_Layout_Ruller[drawFlag.view](layer, row);

	Layout_Footer(layer, row, &processorHot);
  }

    VIEW_FUNC Matrix_Draw_Monitor[VIEW_SIZE] = {
	Draw_Monitor_Frequency,
	Draw_Monitor_Instructions,
	Draw_Monitor_Cycles,
	Draw_Monitor_CStates,
	Draw_Monitor_Package,
	Draw_Monitor_Tasks,
	Draw_Monitor_Interrupts,
	Draw_Monitor_Voltage,
	Draw_Monitor_Slice
    };

    VIEW_FUNC Matrix_Draw_AltMon[VIEW_SIZE] = {
	Draw_AltMonitor_Frequency,
	Draw_AltMonitor_Common,
	Draw_AltMonitor_Common,
	Draw_AltMonitor_Common,
	Draw_AltMonitor_Package,
	Draw_AltMonitor_Tasks,
	Draw_AltMonitor_Common,
	Draw_AltMonitor_Power,
	Draw_AltMonitor_Common
    };

    void Dynamic_Header_DualView_Footer(Layer *layer)
    {
	CUINT row = 0;

	Draw_Header(layer, row);

	row += TOP_HEADER_ROW;

	for (cpu = cpuScroll; cpu < (cpuScroll + MAX_ROWS); cpu++) {
		row++;

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		Draw_Load(layer, row);

		Matrix_Draw_Monitor[drawFlag.view](layer, (1 + row + MAX_ROWS));
	    }
	}
	row = Matrix_Draw_AltMon[drawFlag.view](layer, row);

	Draw_Footer(layer, row);
    }

    void Layout_Card_Core(Layer *layer, Card* card)
    {
	unsigned int _cpu = card->data.dword.lo;

		Dec2Digit(_cpu, digit);
	if (!BITVAL(Shm->Cpu[_cpu].OffLine, OS)) {
		LayerDeclare(4 * INTER_WIDTH) hOnLine = {
			.origin = {
				.col = card->origin.col,
				.row = (card->origin.row + 3)
			},
			.length = (4 * INTER_WIDTH),
			.attr={HDK,HDK,HDK,LCK,LCK,HDK,HDK,HDK,HDK,HDK,HDK,HDK},
			.code={'[',' ','#',' ',' ',' ',' ',' ',' ','C',' ',']'}
		};

		LayerCopyAt(layer, hOnLine.origin.col, hOnLine.origin.row, \
				hOnLine.length, hOnLine.attr, hOnLine.code);
	} else {
		LayerDeclare(4 * INTER_WIDTH) hOffLine = {
			.origin = {
				.col = card->origin.col,
				.row = (card->origin.row + 3)
			},
			.length = (4 * INTER_WIDTH),
			.attr={HDK,HDK,HDK,LBK,LBK,HDK,HDK,LWK,LWK,LWK,HDK,HDK},
			.code={'[',' ','#',' ',' ',' ',' ','O','F','F',' ',']'}
		};

		card->data.dword.hi = RENDER_KO;

		LayerFillAt(layer, card->origin.col, (card->origin.row + 1), \
		(4 * INTER_WIDTH), " _  _  _  _ ", MakeAttr(BLACK,0,BLACK,1));

		LayerCopyAt(layer, hOffLine.origin.col, hOffLine.origin.row, \
				hOffLine.length, hOffLine.attr, hOffLine.code);
	}
	LayerAt(layer, code,
		(card->origin.col + 3),			\
		(card->origin.row + 3)) = digit[7] + '0';

	LayerAt(layer, code,			\
		(card->origin.col + 4),			\
		(card->origin.row + 3)) = digit[8] + '0';
    }

    void Layout_Card_CLK(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hCLK = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,HDK,HWK,HWK,HWK,LWK,HWK,HDK,HDK,HDK,HDK,HDK},
		.code={'[',' ','0','0','0','.','0',' ','M','H','z',']'}
	};
	LayerCopyAt(layer, hCLK.origin.col, hCLK.origin.row,	\
			hCLK.length, hCLK.attr, hCLK.code);
    }

    void Layout_Card_Uncore(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hUncore = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,LCK,LCK,HDK},
		.code={'[','U','N','C','O','R','E',' ',' ',' ',' ',']'}
	};

	sprintf(buffer, "x%2u", Shm->Uncore.Boost[UNCORE_BOOST(MAX)]);
	hUncore.code[ 8] = buffer[0];
	hUncore.code[ 9] = buffer[1];
	hUncore.code[10] = buffer[2];

	LayerCopyAt(layer, hUncore.origin.col, hUncore.origin.row,	\
			hUncore.length, hUncore.attr, hUncore.code);
    }

    void Layout_Card_Bus(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hBus = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,HDK},
		.code={'[','B','u','s',' ',' ',' ',' ',' ',' ',' ',']'}
	};

	sprintf(buffer, "%4u", Shm->Uncore.Bus.Rate);
	hBus.code[5] = buffer[0];
	hBus.code[6] = buffer[1];
	hBus.code[7] = buffer[2];
	hBus.code[8] = buffer[3];

	switch (Shm->Uncore.Unit.Bus_Rate) {
	case 0b00:
		hBus.code[ 9] = 'M';
		hBus.code[10] = 'H';
		break;
	case 0b01:
		hBus.code[ 9] = 'M';
		hBus.code[10] = 'T';
		break;
	case 0b10:
		hBus.code[ 9] = 'M';
		hBus.code[10] = 'B';
		break;
	}

	LayerCopyAt(layer, hBus.origin.col, hBus.origin.row,	\
			hBus.length, hBus.attr, hBus.code);

	Counter2LCD(layer, card->origin.col, card->origin.row,
			(double) Shm->Uncore.Bus.Speed);
    }

    void Layout_Card_MC(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hRAM = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK},
		.code={'[',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',']'}
	};
	unsigned int timings[4] = {
		Shm->Uncore.MC[0].Channel[0].Timing.tCL,
		Shm->Uncore.MC[0].Channel[0].Timing.tRCD,
		Shm->Uncore.MC[0].Channel[0].Timing.tRP,
		Shm->Uncore.MC[0].Channel[0].Timing.tRAS
	}, tdx, bdx = 0, ldx, sdx;
	char str[16];
	for (tdx = 0; tdx < 4; tdx++) {
		ldx = sprintf(str, "%u", timings[tdx]);
		sdx = 0;
		while (sdx < ldx)
			buffer[bdx++] = str[sdx++];
		if ((9 - bdx) > ldx) {
			if (tdx < 3)
				buffer[bdx++] = '-';
		} else
			break;
	}
	for (sdx = 0, ldx = 10 - bdx; sdx < bdx; sdx++, ldx++)
		hRAM.code[ldx] = buffer[sdx];

	LayerCopyAt(layer, hRAM.origin.col, hRAM.origin.row,	\
			hRAM.length, hRAM.attr, hRAM.code);
	if (Shm->Uncore.CtrlCount > 0) {
		Counter2LCD(layer, card->origin.col, card->origin.row,
				(double) Shm->Uncore.CtrlSpeed);
	}
    }

    void Layout_Card_Load(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hLoad = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,HDK,HDK,HDK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK},
		.code={'[',' ',' ','%','L','O','A','D',' ',' ',' ',']'}
	};
	LayerCopyAt(layer, hLoad.origin.col, hLoad.origin.row,	\
			hLoad.length, hLoad.attr, hLoad.code);
    }

    void Layout_Card_Idle(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hIdle = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,HDK,HDK,HDK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK},
		.code={'[',' ',' ','%','I','D','L','E',' ',' ',' ',']'}
	};
	LayerCopyAt(layer, hIdle.origin.col, hIdle.origin.row,	\
			hIdle.length, hIdle.attr, hIdle.code);
    }

  void Layout_Card_RAM(Layer *layer, Card* card)
  {
    LayerDeclare(4 * INTER_WIDTH) hMem = {
	.origin = {
		.col = card->origin.col,
		.row = (card->origin.row + 3)
	},
	.length = (4 * INTER_WIDTH),
	.attr={HDK,HWK,HWK,HWK,HWK,HWK,LWK,HDK,HWK,HWK,LWK,HDK},
	.code={'[',' ',' ',' ',' ',' ',' ','/',' ',' ',' ',']'}
    };

    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
	unsigned long totalRAM, totalDimm = 0;
	int unit;
	char symbol = 0x20;

      if (Shm->Uncore.CtrlCount > 0) {
	unsigned short mc, cha, slot;
	for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
	  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
	    for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		totalDimm += Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size;
	    }
	unit = ByteReDim(totalDimm, 2, &totalRAM);
	if ((unit >= 0) && (unit < 4)) {
		char symbols[4] = {'M', 'G', 'T', 'P'};
		symbol = symbols[unit];
	}
      } else {
	unit = ByteReDim(Shm->SysGate.memInfo.totalram, 2, &totalRAM);
	if ((unit >= 0) && (unit < 4)) {
		char symbols[4] = {'K', 'M', 'G', 'T'};
		symbol = symbols[unit];
	}
      }
	sprintf(buffer, "%2lu%c", totalRAM, symbol);
	memcpy(&hMem.code[8], buffer, 3);

	LayerCopyAt(layer, hMem.origin.col, hMem.origin.row,	\
			hMem.length, hMem.attr, hMem.code);
    } else
	card->data.dword.hi = RENDER_KO;
  }

    void Layout_Card_Task(Layer *layer, Card* card)
    {
	LayerDeclare(4 * INTER_WIDTH) hSystem = {
		.origin = {
			.col = card->origin.col,
			.row = (card->origin.row + 3)
		},
		.length = (4 * INTER_WIDTH),
		.attr={HDK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HDK},
		.code={'[','T','a','s','k','s',' ',' ',' ',' ',' ',']'}
	};

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		LayerCopyAt(layer, hSystem.origin.col, hSystem.origin.row, \
				hSystem.length, hSystem.attr, hSystem.code);
	else
		card->data.dword.hi = RENDER_KO;
    }

    void Layout_Dashboard(Layer *layer)
    {
	CUINT leadingLeft = LEADING_LEFT;
	CUINT leadingTop = LEADING_TOP;
	CUINT marginWidth = MARGIN_WIDTH + (4 * INTER_WIDTH);
	CUINT marginHeight = MARGIN_HEIGHT + INTER_HEIGHT;
	CUINT X = leadingLeft, Y = leadingTop;
	const CUINT	rightEdge = drawSize.width - marginWidth,
			bottomEdge = drawSize.height - leadingTop;

	unsigned int MoveCursorXY(void)
	{
		X += marginWidth;
		if (X > rightEdge) {
			X = leadingLeft;
			Y += marginHeight;
		}
		if (Y > bottomEdge) {
			return(RENDER_KO);
		}
		return(RENDER_OK);
	}

	Card *walker = cardList.head;
	while (walker != NULL) {
		walker->origin.col = X;
		walker->origin.row = Y;
		if ((walker->data.dword.hi = MoveCursorXY()) == RENDER_OK)
			walker->hook.Layout(layer, walker);
		walker = walker->next;
	}
    }

  void Draw_Card_Core(Layer *layer, Card* card)
  {
    if (card->data.dword.hi == RENDER_OK) {
	unsigned int _cpu = card->data.dword.lo;
	struct FLIP_FLOP *Flop=&Shm->Cpu[_cpu].FlipFlop[!Shm->Cpu[_cpu].Toggle];
	ATTRIBUTE warning = {.fg=WHITE, .un=0, .bg=BLACK, .bf=1};

	Clock2LCD(layer, card->origin.col, card->origin.row,
			Flop->Relative.Freq, Flop->Relative.Ratio);

	if (Flop->Thermal.Temp <= Shm->Cpu[_cpu].PowerThermal.Limit[0]) {
		warning = MakeAttr(BLUE, 0, BLACK, 1);
	} else if (Flop->Thermal.Temp >= Shm->Cpu[_cpu].PowerThermal.Limit[1]) {
		warning = MakeAttr(YELLOW, 0, BLACK, 0);
	}
	if (Flop->Thermal.Trip) {
		warning = MakeAttr(RED, 0, BLACK, 1);
	}
	Dec2Digit(Flop->Thermal.Temp, digit);

	LayerAt(layer, attr, (card->origin.col + 6), (card->origin.row + 3)) = \
	LayerAt(layer, attr, (card->origin.col + 7), (card->origin.row + 3)) = \
	LayerAt(layer, attr, (card->origin.col + 8), (card->origin.row + 3)) = \
									warning;

	LayerAt(layer, code, (card->origin.col + 6), (card->origin.row + 3)) = \
					digit[6] ? digit[6] + '0' : 0x20;

	LayerAt(layer, code, (card->origin.col + 7), (card->origin.row + 3)) = \
				(digit[6] | digit[7]) ? digit[7] + '0' : 0x20;

	LayerAt(layer, code, (card->origin.col + 8), (card->origin.row + 3)) = \
								digit[8] + '0';
    }
    else if (card->data.dword.hi == RENDER_KO) {
	CUINT row;

	card->data.dword.hi = RENDER_OFF;

      for (row = card->origin.row; row < card->origin.row + 4; row++) {
	memset(&LayerAt(layer, attr, card->origin.col, row), 0, 4*INTER_WIDTH);
	memset(&LayerAt(layer, code, card->origin.col, row), 0, 4*INTER_WIDTH);
      }
    }
  }

    void Draw_Card_CLK(Layer *layer, Card* card)
    {
	struct PKG_FLIP_FLOP *Pkg = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	double clock = Pkg->Delta.PTSC / 1000000.f;

	Counter2LCD(layer, card->origin.col, card->origin.row, clock);

	sprintf(buffer, "%5.1f",
			Shm->Cpu[Shm->Proc.Service.Core].Clock.Hz / 1000000.f);

	memcpy(&LayerAt(layer, code, (card->origin.col+2),(card->origin.row+3)),
		buffer, 5);
    }

    void Draw_Card_Uncore(Layer *layer, Card* card)
    {
	struct PKG_FLIP_FLOP *Pkg = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	double Uncore = Pkg->Uncore.FC0 / 1000000.f;

	Idle2LCD(layer, card->origin.col, card->origin.row, Uncore);
    }

    void Draw_Card_Load(Layer *layer, Card* card)
    {
	double percent = 100.f * Shm->Proc.Avg.C0;

	Load2LCD(layer, card->origin.col, card->origin.row, percent);
    }

    void Draw_Card_Idle(Layer *layer, Card* card)
    {
	double percent =( Shm->Proc.Avg.C1
			+ Shm->Proc.Avg.C3
			+ Shm->Proc.Avg.C6
			+ Shm->Proc.Avg.C7 ) * 100.f;

	Idle2LCD(layer, card->origin.col, card->origin.row, percent);
    }

  void Draw_Card_RAM(Layer *layer, Card* card)
  {
    if (card->data.dword.hi == RENDER_OK) {
      if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
	unsigned long freeRAM;
	int unit;
	char symbol[4] = {'K', 'M', 'G', 'T'};
	double percent = (100.f * Shm->SysGate.memInfo.freeram)
				/ Shm->SysGate.memInfo.totalram;

	Sys2LCD(layer, card->origin.col, card->origin.row, percent);

	unit = ByteReDim(Shm->SysGate.memInfo.freeram, 6, &freeRAM);
	sprintf(buffer, "%5lu%c", freeRAM, symbol[unit]);
	memcpy(&LayerAt(layer,code, (card->origin.col+1), (card->origin.row+3)),
		buffer, 6);
      }
    }
    else if (card->data.dword.hi == RENDER_KO) {
	CUINT row;

	card->data.dword.hi = RENDER_OFF;

      for (row = card->origin.row; row < card->origin.row + 4; row++) {
	memset(&LayerAt(layer, attr, card->origin.col, row), 0, 4*INTER_WIDTH);
	memset(&LayerAt(layer, code, card->origin.col, row), 0, 4*INTER_WIDTH);
      }
    }
  }

  void Draw_Card_Task(Layer *layer, Card* card)
  {
    if (card->data.dword.hi == RENDER_OK) {
      if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
	size_t len = strnlen(Shm->SysGate.taskList[0].comm, 12);
	int	hl = (12 - len) / 2, hr = hl + hl % 2;
	char	stateStr[16];
	ATTRIBUTE *stateAttr;
	stateAttr = stateToSymbol(Shm->SysGate.taskList[0].state, stateStr);

	sprintf(buffer, "%.*s%s%.*s",
			hl, hSpace,
			Shm->SysGate.taskList[0].comm,
			hr, hSpace);

	LayerCopyAt(layer,	(card->origin.col + 0),		\
				(card->origin.row + 1),		\
				12,				\
				stateAttr,			\
				buffer);

	len = sprintf(buffer, "%5u (%c)",
				Shm->SysGate.taskList[0].pid,
				stateStr[0]);

	LayerFillAt(layer,	(card->origin.col + 2),		\
				(card->origin.row + 2),		\
				len,				\
				buffer,				\
				MakeAttr(WHITE, 0, BLACK, 0));

	sprintf(buffer, "%5u", Shm->SysGate.taskCount);
	memcpy(&LayerAt(layer, code, (card->origin.col+6),(card->origin.row+3)),
		buffer, 5);
      }
    }
    else if (card->data.dword.hi == RENDER_KO) {
	CUINT row;

	card->data.dword.hi = RENDER_OFF;

      for (row = card->origin.row; row < card->origin.row + 4; row++) {
	memset(&LayerAt(layer, attr, card->origin.col, row), 0, 4*INTER_WIDTH);
	memset(&LayerAt(layer, code, card->origin.col, row), 0, 4*INTER_WIDTH);
      }
    }
  }

    void Dont_Draw_Card(Layer *layer, Card* card)
    {
    }

    void Draw_Dashboard(Layer *layer)
    {
	Card *walker = cardList.head;
	while (walker != NULL) {
		walker->hook.Draw(layer, walker);
		walker = walker->next;
	}
    }

    void AllocDashboard(void)
    {
	Card *card = NULL;
	for(cpu = 0; (cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, 0); cpu++)
	{
		if ((card = CreateCard()) != NULL) {
			card->data.dword.lo = cpu;
			card->data.dword.hi = RENDER_OK;

			AppendCard(card, &cardList);
			StoreCard(card, .Layout, Layout_Card_Core);
			StoreCard(card, .Draw, Draw_Card_Core);
		}
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_CLK);
		StoreCard(card, .Draw, Draw_Card_CLK);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_Uncore);
		StoreCard(card, .Draw, Draw_Card_Uncore);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_Bus);
		StoreCard(card, .Draw, Dont_Draw_Card);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_MC);
		StoreCard(card, .Draw, Dont_Draw_Card);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_Load);
		StoreCard(card, .Draw, Draw_Card_Load);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_Idle);
		StoreCard(card, .Draw, Draw_Card_Idle);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_RAM);
		StoreCard(card, .Draw, Draw_Card_RAM);
	}
	if ((card = CreateCard()) != NULL) {
		card->data.dword.lo = 0;
		card->data.dword.hi = RENDER_OK;

		AppendCard(card, &cardList);
		StoreCard(card, .Layout, Layout_Card_Task);
		StoreCard(card, .Draw, Draw_Card_Task);
	}
    }

    void Layout_NoHeader_SingleView_NoFooter(Layer *layer)
    {
	char *mir[] = {
		" ! \" # $ % & \' () * + , -./",
		"  0123456789 : ; < = > ?",
		"@ ABCDEFGHIJKLMNO",
		"  PQRSTUVWXYZ [ \\ ] ^ _",
		"` abcdefghijklmno",
		"  pqrstuvwxyz { | } ~ \x7f"
	};
	PrintLCD(layer, 0, 1, strlen(mir[0]), mir[0], _WHITE);
	PrintLCD(layer, 0, 5, strlen(mir[1]), mir[1], _WHITE);
	PrintLCD(layer, 0, 9, strlen(mir[2]), mir[2], _WHITE);
	PrintLCD(layer, 0,13, strlen(mir[3]), mir[3], _WHITE);
	PrintLCD(layer, 0,17, strlen(mir[4]), mir[4], _WHITE);
	PrintLCD(layer, 0,21, strlen(mir[5]), mir[5], _WHITE);
    }

    void Dynamic_NoHeader_SingleView_NoFooter(Layer *layer)
    {
    }

	/* Top starts here */
	SortAvailableRatio();

	TrapScreenSize(SIGWINCH);
	signal(SIGWINCH, TrapScreenSize);

	hBClk = calloc(Shm->Proc.CPU.Count, sizeof(HBCLK));
	cTask = calloc(Shm->Proc.CPU.Count, sizeof(Coordinate));

	AllocAll(&buffer);
	AllocDashboard();

	DISPOSAL_FUNC LayoutView[DISPOSAL_SIZE] = {
		Layout_Header_DualView_Footer,
		Layout_Dashboard,
		Layout_NoHeader_SingleView_NoFooter
	};
	DISPOSAL_FUNC DynamicView[DISPOSAL_SIZE] = {
		Dynamic_Header_DualView_Footer,
		Draw_Dashboard,
		Dynamic_NoHeader_SingleView_NoFooter
	};

  while (!BITVAL(Shutdown, 0))
  {
    do
    {
	SCANKEY scan = {.key = 0};

	if ((drawFlag.daemon = BITVAL(Shm->Proc.Sync, 0)) == 0) {
	    if (GetKey(&scan, &Shm->Sleep.pollingWait) > 0) {
		if (Shortcut(&scan) == -1) {
		  if (IsDead(&winList))
			AppendWindow(CreateMenu(SCANKEY_F2), &winList);
		  else
		    if (Motion_Trigger(&scan,GetFocus(&winList),&winList) > 0)
			Shortcut(&scan);
		}
		PrintWindowStack(&winList);

		break;
	    }
	} else {
		BITCLR(LOCKLESS, Shm->Proc.Sync, 0);
	}
	if (BITVAL(Shm->Proc.Sync, 62)) { // Compute required, clear the layout
		SortAvailableRatio();
		drawFlag.clear = 1;
		BITCLR(LOCKLESS, Shm->Proc.Sync, 62);
	}
	if (BITVAL(Shm->Proc.Sync, 63)) { // Platform changed, redraw the layout
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
		drawFlag.layout = 1;
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);
	}
    } while (	!BITVAL(Shutdown, 0)
		&& !drawFlag.daemon
		&& !drawFlag.layout
		&& !drawFlag.clear ) ;

    if (drawFlag.height & drawFlag.width)
    {
	if (drawFlag.clear) {
		drawFlag.clear  = 0;
		drawFlag.layout = 1;
		ResetLayer(dLayer);
	}
	if (drawFlag.layout) {
		drawFlag.layout = 0;
		ResetLayer(sLayer);
		FillLayerArea(sLayer, 0, 0, drawSize.width, drawSize.height,
				(ASCII*) hSpace, MakeAttr(BLACK, 0, BLACK, 1));

		LayoutView[drawFlag.disposal](sLayer);
	}
	if (drawFlag.daemon)
	{
		DynamicView[drawFlag.disposal](dLayer);

		// Increment the BCLK indicator (skip offline CPU)
		do {
			iClock++;
			if (iClock >= MAX_ROWS)
				iClock = 0;
		} while (BITVAL(Shm->Cpu[iClock].OffLine, OS)
			&& (iClock != Shm->Proc.Service.Core)) ;
	}
	// Write to the standard output
	WriteConsole(drawSize, buffer);
    } else
	printf( CUH RoK "Term(%u x %u) < View(%u x %u)\n",
		drawSize.width, drawSize.height, MIN_WIDTH, MIN_HEIGHT);
  }

  FreeAll(buffer);

  free(hBClk);
  free(cTask);

  DestroyAllCards(&cardList);
}

int Help(char *appName)
{
	printf(	"CoreFreq."						\
		"  Copyright (C) 2015-2018 CYRIL INGENIERIE\n\n");
	printf(	"usage:\t%s [-option <arguments>]\n"			\
		"\t-t\tShow Top (default)\n"				\
		"\t-d\tShow Dashboard\n"				\
		"\t-V\tMonitor Power and Voltage\n"			\
		"\t-g\tMonitor Package\n"				\
		"\t-c\tMonitor Counters\n"				\
		"\t-i\tMonitor Instructions\n"				\
		"\t-s\tPrint System Information\n"			\
		"\t-j\tPrint System Information (json-encoded)\n" 	\
		"\t-M\tPrint Memory Controller\n"			\
		"\t-R\tPrint System Registers\n"			\
		"\t-m\tPrint Topology\n"				\
		"\t-u\tPrint CPUID\n"					\
		"\t-k\tPrint Kernel\n"					\
		"\t-h\tPrint out this message\n"			\
		"\nExit status:\n"					\
			"0\tif OK,\n"					\
			"1\tif problems,\n"				\
			">1\tif serious trouble.\n"			\
		"\nReport bugs to labs[at]cyring.fr\n", appName);
	return(1);
}

int main(int argc, char *argv[])
{
	struct stat shmStat = {0};
	SHM_STRUCT *Shm;
	int fd = -1, rc = EXIT_SUCCESS;

	char *program = strdup(argv[0]), *appName=basename(program);
	char option = 't';
  if ((argc >= 2) && (argv[1][0] == '-'))
	option = argv[1][1];
  if (option == 'h')
	Help(appName);
  else if ((fd = shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) !=-1)
  {
    if (fstat(fd, &shmStat) != -1)
    {
      if ((Shm = mmap(NULL, shmStat.st_size,
			PROT_READ|PROT_WRITE, MAP_SHARED,
			fd, 0)) != MAP_FAILED)
      {
	ClientFollowService(&localService, &Shm->Proc.Service, 0);

	switch (option) {
	case 'k':
		if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
			SysInfoKernel(Shm, 80, NULL);
		}
		break;
	case 'u':
		SysInfoCPUID(Shm, 80, NULL);
		break;
	case 's':
	{
	SysInfoProc(Shm, 80, NULL);
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"");
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"ISA Extensions:");
	SysInfoISA(Shm, NULL);
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"");
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"Features:");
	SysInfoFeatures(Shm, 80, NULL);
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"");
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"Technologies:");
	SysInfoTech(Shm, 80, NULL);
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"");
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"Performance Monitoring:");
	SysInfoPerfMon(Shm, 80, NULL);
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"");
	printv(NULL, SCANKEY_VOID, NULL, 80, 0,"Power & Thermal Monitoring:");
	SysInfoPwrThermal(Shm, 80, NULL);
	}
		break;
	case 'j':
		JsonSysInfo(Shm, NULL);
		break;
	case 'm':
		Topology(Shm, NULL);
		break;
	case 'M':
		MemoryController(Shm, NULL);
		break;
	case 'R':
		SystemRegisters(Shm, NULL);
		break;
	case 'i':
		TrapSignal(Shm, 1);
		Instructions(Shm);
		TrapSignal(Shm, 0);
		break;
	case 'c':
		TrapSignal(Shm, 1);
		Counters(Shm);
		TrapSignal(Shm, 0);
		break;
	case 'V':
		TrapSignal(Shm, 1);
		Voltage(Shm);
		TrapSignal(Shm, 0);
		break;
	case 'g':
		TrapSignal(Shm, 1);
		Package(Shm);
		TrapSignal(Shm, 0);
		break;
	case 'd':
		// Fallthrough
	case 't':
		{
		TERMINAL(IN);

		TrapSignal(Shm, 1);
		Top(Shm, option);
		TrapSignal(Shm, 0);

		TERMINAL(OUT);
		}
		break;
	default:
		rc = Help(appName);
		break;
	}
	munmap(Shm, shmStat.st_size);
	close(fd);
      } else
	rc = 4;
    } else
	rc = 3;
  } else
	rc = 2;

    free(program);
    return(rc);
}
