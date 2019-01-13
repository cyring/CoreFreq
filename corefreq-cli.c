/*
 * CoreFreq
 * Copyright (C) 2015-2019 CYRIL INGENIERIE
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
#include "corefreq-cli-rsc.h"
#include "corefreq-cli-json.h"

SHM_STRUCT *Shm = NULL;

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

void TrapSignal(int operation)
{
	if (operation == 0) {
		Shm->AppCli = 0;
	} else {
		Shm->AppCli = getpid();
		signal(SIGINT,  Emergency);
		signal(SIGQUIT, Emergency);
		signal(SIGTERM, Emergency);
		signal(SIGTSTP, Emergency);	/* [CTRL] + [Z]		*/
		signal(SIGCHLD, Emergency);
	}
}

ATTRIBUTE *StateToSymbol(short int state, char stateStr[])
{
	ATTRIBUTE *symbAttr[14] = {
	/* R */ RSC(RUN_STATE_COLOR).ATTR(),
	/* S */ RSC(SLEEP_STATE_COLOR).ATTR(),
	/* D */ RSC(UNINT_STATE_COLOR).ATTR(),
	/* T */ RSC(WAIT_STATE_COLOR).ATTR(),
	/* t */ RSC(WAIT_STATE_COLOR).ATTR(),
	/* X */ RSC(WAIT_STATE_COLOR).ATTR(),
	/* Z */ RSC(ZOMBIE_STATE_COLOR).ATTR(),
	/* P */ RSC(WAIT_STATE_COLOR).ATTR(),
	/* I */ RSC(WAIT_STATE_COLOR).ATTR(),
	/* K */ RSC(SLEEP_STATE_COLOR).ATTR(),
	/* W */ RSC(RUN_STATE_COLOR).ATTR(),
	/* i */ RSC(WAIT_STATE_COLOR).ATTR(),
	/* N */ RSC(RUN_STATE_COLOR).ATTR(),
	/* m */ RSC(OTHER_STATE_COLOR).ATTR()
	}, *stateAttr = RSC(OTHER_STATE_COLOR).ATTR();
	const char symbol[14] = "RSDTtXZPIKWiNm";
	unsigned short idx, jdx = 0;

	if (BITBSR(state, idx) == 1) {
		stateStr[jdx++] = symbol[0];
		stateAttr = symbAttr[0];
	} else
		do {
			BITCLR(LOCKLESS, state, idx);
			stateStr[jdx++] = symbol[1 + idx];
			stateAttr = symbAttr[1 + idx];
		} while (!BITBSR(state, idx));
	stateStr[jdx] = '\0';
	return(stateAttr);
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

void Print_v1(	CELL_FUNC OutFunc,
		Window *win,
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
  if (line != NULL)
  {
	va_list ap;
	va_start(ap, fmt);
	vsprintf(line, fmt, ap);

    if (OutFunc == NULL)
	printf("%s%s%.*s\n", indent[0][tab], line,
		(int)(width - strlen(line) - strlen(indent[0][tab])), hSpace);
    else {
	ASCII *item = malloc(width + 1);
      if (item != NULL) {
	sprintf((char *)item, "%s%s%.*s", indent[1][tab], line,
		(int)(width - strlen(line) - strlen(indent[1][tab])), hSpace);
	OutFunc(win, key, attrib, item);
	free(item);
      }
    }
	va_end(ap);
	free(line);
  }
}

void Print_v2(	CELL_FUNC OutFunc,
		Window *win,
		CUINT *nl,
		ATTRIBUTE *attrib, ...)
{
	ASCII *item = malloc(32);
    if (item != NULL)
    {
	char *fmt;
	va_list ap;
	va_start(ap, attrib);
	if ((fmt = va_arg(ap, char*)) != NULL)
	{
		vsprintf((char*) item, fmt, ap);
		if (OutFunc == NULL) {
			(*nl)--;
			if ((*nl) == 0) {
				(*nl) = win->matrix.size.wth;
				printf("%s\n", item);
			} else
				printf("%s", item);
		} else
			OutFunc(win, SCANKEY_NULL, attrib, item);
	}
	va_end(ap);
	free(item);
    }
}

void Print_v3(	CELL_FUNC OutFunc,
		Window *win,
		CUINT *nl,
		ATTRIBUTE *attrib, ...)
{
	ASCII *item = malloc(32);
    if (item != NULL)
    {
	char *fmt;
	va_list ap;
	va_start(ap, attrib);
	if ((fmt = va_arg(ap, char*)) != NULL)
	{
		vsprintf((char*) item, fmt, ap);
		if (OutFunc == NULL) {
			(*nl)--;
			if ((*nl) == (win->matrix.size.wth - 1))
				printf("|-%s", item);
			else if ((*nl) == 0) {
				(*nl) = win->matrix.size.wth;
				printf("%s\n", item);
			} else
				printf("%s", item);
		} else
			OutFunc(win, SCANKEY_NULL, attrib, item);
	}
	va_end(ap);
	free(item);
    }
}

#define PUT(key, attrib, width, tab, fmt, ...)	\
	Print_v1(OutFunc, win, key, attrib, width, tab, fmt, __VA_ARGS__)

#define Print_REG	Print_v2
#define Print_MAP	Print_v2
#define Print_IMC	Print_v2
#define Print_ISA	Print_v3

#define PRT(FUN, attrib, ...)	\
	Print_##FUN(OutFunc, win, &nl, attrib, __VA_ARGS__)

void SysInfoCPUID(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_CPUID_COND0).ATTR(),
		RSC(SYSINFO_CPUID_COND1).ATTR(),
		RSC(SYSINFO_CPUID_COND2).ATTR(),
		RSC(SYSINFO_CPUID_COND3).ATTR()
	};
	char format[] = "%08x:%08x%.*s%08x     %08x     %08x     %08x";
	unsigned int cpu;
	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	    if (OutFunc == NULL) {
		PUT(SCANKEY_NULL, attrib[0], width, 0,
			"CPU #%-2u function"				\
			"          EAX          EBX          ECX          EDX",
			cpu);
	    } else {
		PUT(SCANKEY_NULL,
			attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
			width, 0, "CPU #%-2u", cpu);
	    }
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PUT(SCANKEY_NULL, attrib[3], width, 2, format,
			0x00000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc,
			Shm->Cpu[cpu].Query.StdFunc.BX,
			Shm->Cpu[cpu].Query.StdFunc.CX,
			Shm->Cpu[cpu].Query.StdFunc.DX);

		PUT(SCANKEY_NULL, attrib[2], width, 3,
			"%.*s""=%08x",
			25, RSC(LARGEST_STD_FUNC).CODE(),
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc);

		PUT(SCANKEY_NULL, attrib[3], width, 2, format,
			0x80000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc,
			Shm->Cpu[cpu].Query.ExtFunc.EBX,
			Shm->Cpu[cpu].Query.ExtFunc.ECX,
			Shm->Cpu[cpu].Query.ExtFunc.EDX);

		PUT(SCANKEY_NULL, attrib[2], width, 3,
			"%.*s""=%08x",
			25, RSC(LARGEST_EXT_FUNC).CODE(),
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc);

		int i;
		for (i = 0; i < CPUID_MAX_FUNC; i++)
		    if (Shm->Cpu[cpu].CpuID[i].func) {
			PUT(SCANKEY_NULL, attrib[3], width, 2,
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

void SystemRegisters(Window *win, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[3] = {
		RSC(SYSTEM_REGISTERS_COND0).ATTR(),
		RSC(SYSTEM_REGISTERS_COND1).ATTR(),
		RSC(SYSTEM_REGISTERS_COND2).ATTR()
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
	CUINT nl = win->matrix.size.wth;

/* Section Mark */
	PRT(REG, attrib[0], "CPU ");
	PRT(REG, attrib[0], "FLAG");
    for (idx = tabRFLAGS.Start; idx < tabRFLAGS.Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
	PRT(REG, attrib[0], "    ");
	PRT(REG, attrib[0], "CR3:");
    for (idx = tabCR3.Start; idx < tabCR3.Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
	PRT(REG, attrib[0], "    ");
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
		"#%-2u ", cpu);

	PRT(REG, attrib[0], "    ");
	for (idx = tabRFLAGS.Start; idx < tabRFLAGS.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.RFLAGS,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
	PRT(REG, attrib[0], "    ");
	PRT(REG, attrib[0], "    ");
	for (idx = tabCR3.Start; idx < tabCR3.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR3,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
	PRT(REG, attrib[0], "    ");
    }
/* Section Mark */
	PRT(REG, attrib[0], "CR0:");
    for (idx = tabCR0.Start; idx < tabCR0.Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
	PRT(REG, attrib[0], "CR4:");
    for (idx = tabCR4[0].Start; idx < tabCR4[0].Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
		"#%-2u ", cpu);

	for (idx = tabCR0.Start; idx < tabCR0.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR0,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
	PRT(REG, attrib[0], "    ");
	for (idx = tabCR4[0].Start; idx < tabCR4[0].Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR4,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
    }
/* Section Mark */
	PRT(REG, attrib[0], "CR4:");
    for (idx = tabCR4[1].Start; idx < tabCR4[1].Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
		"#%-2u ", cpu);
	for (idx = tabCR4[1].Start; idx < tabCR4[1].Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR4,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
    }
/* Section Mark */
	PRT(REG, attrib[0], "EFCR");
	PRT(REG, attrib[0], "    ");
    for (idx = tabEFCR.Start; idx < tabEFCR.Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
	PRT(REG, attrib[0], "    ");
	PRT(REG, attrib[0], "EFER");
    for (idx = tabEFER.Start; idx < tabEFER.Stop; idx++) {
	PRT(REG, attrib[0], "%s", SR[idx].flag);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
		"#%-2u ", cpu);
	PRT(REG, attrib[0], "    ");

	for (idx = tabEFCR.Start; idx < tabEFCR.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)
	    &&  ((Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.EFCR,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
	PRT(REG, attrib[0], "    ");
	PRT(REG, attrib[0], "    ");
	for (idx = tabEFER.Start; idx < tabEFER.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.EFER,
				SR[idx].bit, SR[idx].len));
	    else
		PRT(REG, attrib[1], "  - ");
	}
    }
}

void PrintCoreBoost(	Window *win, struct FLIP_FLOP *CFlop,
			char *pfx, int _boost, int syc, unsigned long long _key,
			CUINT width, CELL_FUNC OutFunc, ATTRIBUTE attrib[])
{
	char symb[2][2] = {{'[', ']'}, {'<', '>'}};

    if (Shm->Proc.Boost[_boost] > 0) {
	PUT(	_key, attrib, width, 0,
		"%.*s""%s""%.*s""%7.2f""%.*s""%c%4d %c",
	(int) (20 - strlen(pfx)), hSpace, pfx, 3, hSpace,
	(double) ( Shm->Proc.Boost[_boost] * CFlop->Clock.Hz) / 1000000.0,
		20, hSpace,
		symb[syc][0],
		Shm->Proc.Boost[_boost],
		symb[syc][1]);
    }
}

void PrintUncoreBoost(	Window *win, struct FLIP_FLOP *CFlop,
			char *pfx, int _boost, int syc, unsigned long long _key,
			CUINT width, CELL_FUNC OutFunc, ATTRIBUTE attrib[])
{
	char symb[2][2] = {{'[', ']'}, {'<', '>'}};

    if (Shm->Uncore.Boost[_boost] > 0) {
	PUT(	_key, attrib, width, 0,
		"%.*s""%s""%.*s""%7.2f""%.*s""%c%4d %c",
	(int) (20 - strlen(pfx)), hSpace, pfx, 3, hSpace,
	(double) ( Shm->Uncore.Boost[_boost] * CFlop->Clock.Hz) / 1000000.0,
		20, hSpace,
		symb[syc][0],
		Shm->Uncore.Boost[_boost],
		symb[syc][1]);
    }
}

void SysInfoProc(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_PROC_COND0).ATTR(),
		RSC(SYSINFO_PROC_COND1).ATTR(),
		RSC(SYSINFO_PROC_COND2).ATTR(),
		RSC(SYSINFO_PROC_COND3).ATTR()
	};
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
			.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core].Toggle];

	unsigned int activeCores, boost = 0;

	PUT(SCANKEY_NULL, attrib[0], width, 0,
		"%s""%.*s[%s]", RSC(PROCESSOR).CODE(),
		width - 2 - RSZ(PROCESSOR) - strlen(Shm->Proc.Brand),
		hSpace, Shm->Proc.Brand);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%s]", RSC(ARCHITECTURE).CODE(),
		width - 5 - RSZ(ARCHITECTURE) - strlen(Shm->Proc.Architecture),
		hSpace, Shm->Proc.Architecture);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%s]", RSC(VENDOR_ID).CODE(),
	width - 5 - RSZ(VENDOR_ID) - strlen(Shm->Proc.Features.Info.Vendor.ID),
		hSpace, Shm->Proc.Features.Info.Vendor.ID);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%10u]", RSC(MICROCODE).CODE(),
		width - 15 - RSZ(MICROCODE), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].Query.Microcode);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%2X%1X_%1X%1X]", RSC(SIGNATURE).CODE(),
		width - 11 - RSZ(SIGNATURE), hSpace,
		Shm->Proc.Features.Std.EAX.ExtFamily,
		Shm->Proc.Features.Std.EAX.Family,
		Shm->Proc.Features.Std.EAX.ExtModel,
		Shm->Proc.Features.Std.EAX.Model);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%6u]", RSC(STEPPING).CODE(),
		width - 11 - RSZ(STEPPING), hSpace,
		Shm->Proc.Features.Std.EAX.Stepping);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[ %2u/%-2u]", RSC(ONLINE_CPU).CODE(),
		width - 11 - RSZ(ONLINE_CPU), hSpace,
		Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%6.2f]", RSC(BASE_CLOCK).CODE(),
		width - 11 - RSZ(BASE_CLOCK), hSpace,
		CFlop->Clock.Hz / 1000000.0);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s(MHz)%.*s""%s", RSC(FREQUENCY).CODE(),
		21 - RSZ(FREQUENCY), hSpace,
		23 - (OutFunc == NULL), hSpace,
		RSC(RATIO).CODE());

    if (Shm->Proc.Features.MinRatio_Unlock) {
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_MIN;

	PrintCoreBoost(win, CFlop,
			"Min", BOOST(MIN), 1, coreClock.sllong,
			width, OutFunc, attrib[3]);
    } else {
	PrintCoreBoost(win, CFlop,
			"Min", BOOST(MIN), 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);
    }
    if (Shm->Proc.Features.MaxRatio_Unlock) {
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_MAX;

	PrintCoreBoost(win, CFlop,
			"Max", BOOST(MAX), 1, coreClock.sllong,
			width, OutFunc, attrib[3]);
    } else {
	PrintCoreBoost(win, CFlop,
			"Max", BOOST(MAX), 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);
    }

	PUT(SCANKEY_NULL, attrib[0], width, 2, "%s""%.*s[%6.2f]",
		RSC(FACTORY).CODE(),
		(OutFunc == NULL ? 69 : 65) - RSZ(FACTORY), hSpace,
			Shm->Proc.Features.Factory.Clock.Hz / 1000000.0);

	PUT(SCANKEY_NULL, attrib[3], width, 0,
		"%.*s""%5u""%.*s""[%4d ]",
		22, hSpace, Shm->Proc.Features.Factory.Freq,
		23, hSpace, Shm->Proc.Features.Factory.Ratio);

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.Turbo_Unlock],
		width, 2,
		"Turbo Boost%.*s[%.*s]", width - 22, hSpace, 6,
			Shm->Proc.Features.Turbo_Unlock ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

    if (Shm->Proc.Features.Turbo_Unlock)
      for (boost = BOOST(1C), activeCores = 1;
		boost > BOOST(1C) - Shm->Proc.Features.SpecTurboRatio;
			boost--, activeCores++)
	{
	CLOCK_ARG clockMod={.NC=BOXKEY_TURBO_CLOCK_NC | activeCores,.Offset=0};
	char pfx[4];
	sprintf(pfx, "%2uC", activeCores);
	PrintCoreBoost(win, CFlop,
			pfx, boost, 1, clockMod.sllong,
			width, OutFunc, attrib[3]);
      }
    else
      for (boost = BOOST(1C), activeCores = 1;
		boost > BOOST(1C) - Shm->Proc.Features.SpecTurboRatio;
			boost--, activeCores++) {
	char pfx[4];
	sprintf(pfx, "%2uC", activeCores);
	PrintCoreBoost(win, CFlop,
			pfx, boost, 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);
      }

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.Uncore_Unlock],
		width, 2, "Uncore%.*s[%.*s]", width - 17, hSpace, 6,
			Shm->Proc.Features.Uncore_Unlock ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

    if (Shm->Proc.Features.Uncore_Unlock) {
	CLOCK_ARG uncoreClock = {.NC = 0, .Offset = 0};

	uncoreClock.NC = BOXKEY_UNCORE_CLOCK_OR | CLOCK_MOD_MIN;
	PrintUncoreBoost(win, CFlop,
			"Min", UNCORE_BOOST(MIN), 1, uncoreClock.sllong,
			width, OutFunc, attrib[3]);

	uncoreClock.NC = BOXKEY_UNCORE_CLOCK_OR | CLOCK_MOD_MAX;
	PrintUncoreBoost(win, CFlop,
			"Max", UNCORE_BOOST(MAX), 1, uncoreClock.sllong,
			width, OutFunc, attrib[3]);
    } else {
	PrintUncoreBoost(win, CFlop,
			"Min", UNCORE_BOOST(MIN), 0,SCANKEY_NULL,
			width, OutFunc, attrib[3]);
	PrintUncoreBoost(win, CFlop,
			"Max", UNCORE_BOOST(MAX), 0,SCANKEY_NULL,
			width, OutFunc, attrib[3]);
    }

    if (Shm->Proc.Features.TDP_Levels > 0) {
	const size_t len = RSZ(LEVEL) + 1 + 1;
	char *pfx = malloc(len);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"TDP%.*s""%s"" [%3d:%-2d]",
		width - 15 - RSZ(LEVEL), hSpace, RSC(LEVEL).CODE(),
		Shm->Proc.Features.TDP_Cfg_Level,Shm->Proc.Features.TDP_Levels);

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.TDP_Unlock],
		width, 3, "%s%.*s[%.*s]", RSC(PROGRAMMABLE).CODE(),
		width - (OutFunc == NULL ? 14:12) - RSZ(PROGRAMMABLE), hSpace,
			6, Shm->Proc.Features.TDP_Unlock ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

	PUT(SCANKEY_NULL, attrib[!Shm->Proc.Features.TDP_Cfg_Lock],
		width, 3, "%s%.*s[%.*s]", RSC(CONFIGURATION).CODE(),
		width - (OutFunc == NULL ? 14:12) - RSZ(CONFIGURATION),hSpace,
			6, Shm->Proc.Features.TDP_Cfg_Lock ?
				RSC(LOCK).CODE() : RSC(UNLOCK).CODE());

	PUT(SCANKEY_NULL, attrib[!Shm->Proc.Features.TurboActivation],
		width, 3, "%s%.*s[%.*s]", RSC(TURBO_ACTIVATION).CODE(),
		width - (OutFunc == NULL ? 14:12)-RSZ(TURBO_ACTIVATION),hSpace,
			6, Shm->Proc.Features.TurboActivation ?
				RSC(LOCK).CODE() : RSC(UNLOCK).CODE());

	PrintCoreBoost(win, CFlop, (char*) RSC(NOMINAL).CODE(),
			BOOST(TDP), 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);

	sprintf(pfx, "%s""1", RSC(LEVEL).CODE());
	PrintCoreBoost(win, CFlop,
			pfx, BOOST(TDP1), 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);

	sprintf(pfx, "%s""2", RSC(LEVEL).CODE());
	PrintCoreBoost(win, CFlop,
			pfx, BOOST(TDP2), 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);

	PrintCoreBoost(win, CFlop,
			"Turbo", BOOST(ACT), 0, SCANKEY_NULL,
			width, OutFunc, attrib[3]);
	free(pfx);
    }
}

void SysInfoISA(Window *win, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[4][5] = {
		{
			RSC(SYSINFO_ISA_COND_0_0).ATTR(),
			RSC(SYSINFO_ISA_COND_0_1).ATTR(),
			RSC(SYSINFO_ISA_COND_0_2).ATTR(),
			RSC(SYSINFO_ISA_COND_0_3).ATTR(),
			RSC(SYSINFO_ISA_COND_0_4).ATTR()
		}, {
			RSC(SYSINFO_ISA_COND_1_0).ATTR(),
			RSC(SYSINFO_ISA_COND_1_1).ATTR(),
			RSC(SYSINFO_ISA_COND_1_2).ATTR(),
			RSC(SYSINFO_ISA_COND_1_3).ATTR(),
			RSC(SYSINFO_ISA_COND_1_4).ATTR()
		}, {
			RSC(SYSINFO_ISA_COND_2_0).ATTR(),
			RSC(SYSINFO_ISA_COND_2_1).ATTR(),
			RSC(SYSINFO_ISA_COND_2_2).ATTR(),
			RSC(SYSINFO_ISA_COND_2_3).ATTR(),
			RSC(SYSINFO_ISA_COND_2_4).ATTR()
		}, {
			RSC(SYSINFO_ISA_COND_3_0).ATTR(),
			RSC(SYSINFO_ISA_COND_3_1).ATTR(),
			RSC(SYSINFO_ISA_COND_3_2).ATTR(),
			RSC(SYSINFO_ISA_COND_3_3).ATTR(),
			RSC(SYSINFO_ISA_COND_3_4).ATTR()
		}
	};
	CUINT nl = win->matrix.size.wth;
/* Row Mark */
	PRT(ISA, attrib[0][2 * (Shm->Proc.Features.ExtInfo.EDX._3DNow
				|  Shm->Proc.Features.ExtInfo.EDX._3DNowEx)
				+ (Shm->Proc.Features.ExtInfo.EDX._3DNow
				<< Shm->Proc.Features.ExtInfo.EDX._3DNowEx)],
		" 3DNow!/Ext [%c,%c]",
		Shm->Proc.Features.ExtInfo.EDX._3DNow ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.EDX._3DNowEx ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.ExtFeature.EBX.ADX],
		"          ADX [%c]",
		Shm->Proc.Features.ExtFeature.EBX.ADX ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.AES],
		"          AES [%c]",
		Shm->Proc.Features.Std.ECX.AES ? 'Y' : 'N');

	PRT(ISA, attrib[3][2 * (Shm->Proc.Features.Std.ECX.AVX
			|  Shm->Proc.Features.ExtFeature.EBX.AVX2)
			+ (Shm->Proc.Features.Std.ECX.AVX
			<< Shm->Proc.Features.ExtFeature.EBX.AVX2)],
		"  AVX/AVX2 [%c/%c] ",
		Shm->Proc.Features.Std.ECX.AVX ? 'Y' : 'N',
		Shm->Proc.Features.ExtFeature.EBX.AVX2 ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.ExtFeature.EBX.AVX_512F],
		" AVX-512      [%c]",
		Shm->Proc.Features.ExtFeature.EBX.AVX_512F ? 'Y' : 'N');

	PRT(ISA, attrib[0][2 * (Shm->Proc.Features.ExtFeature.EBX.BMI1
				|  Shm->Proc.Features.ExtFeature.EBX.BMI2)
				+ (Shm->Proc.Features.ExtFeature.EBX.BMI1
				<< Shm->Proc.Features.ExtFeature.EBX.BMI2)],
		"  BMI1/BMI2 [%c/%c]",
		Shm->Proc.Features.ExtFeature.EBX.BMI1 ? 'Y' : 'N',
		Shm->Proc.Features.ExtFeature.EBX.BMI2 ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.EDX.CLFSH],
		"        CLFSH [%c]",
		Shm->Proc.Features.Std.EDX.CLFSH ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.Std.EDX.CMOV],
		"        CMOV [%c] ",
		Shm->Proc.Features.Std.EDX.CMOV ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.Std.EDX.CMPXCH8],
		" CMPXCH8      [%c]",
		Shm->Proc.Features.Std.EDX.CMPXCH8 ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.CMPXCH16],
		"     CMPXCH16 [%c]",
		Shm->Proc.Features.Std.ECX.CMPXCH16 ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.F16C],
		"         F16C [%c]",
		Shm->Proc.Features.Std.ECX.F16C ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.Std.EDX.FPU],
		"         FPU [%c] ",
		Shm->Proc.Features.Std.EDX.FPU ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.Std.EDX.FXSR],
		" FXSR         [%c]",
		Shm->Proc.Features.Std.EDX.FXSR ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF],
		"    LAHF/SAHF [%c]",
		Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF ? 'Y' : 'N');

	PRT(ISA, attrib[0][2 * (Shm->Proc.Features.Std.EDX.MMX
			|  Shm->Proc.Features.ExtInfo.EDX.MMX_Ext)
			+ (Shm->Proc.Features.Std.EDX.MMX
			<< Shm->Proc.Features.ExtInfo.EDX.MMX_Ext)],
		"    MMX/Ext [%c/%c]",
		Shm->Proc.Features.Std.EDX.MMX ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.EDX.MMX_Ext ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.Std.ECX.MONITOR],
		"     MONITOR [%c] ",
		Shm->Proc.Features.Std.ECX.MONITOR ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.MOVBE],
		" MOVBE        [%c]",
		Shm->Proc.Features.Std.ECX.MOVBE ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.ExtFeature.EBX.MPX],
		"          MPX [%c]",
		Shm->Proc.Features.ExtFeature.EBX.MPX ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.PCLMULDQ],
		"     PCLMULDQ [%c]",
		Shm->Proc.Features.Std.ECX.PCLMULDQ ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.Std.ECX.POPCNT],
		"      POPCNT [%c] ",
		Shm->Proc.Features.Std.ECX.POPCNT ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.RDRAND],
		" RDRAND       [%c]",
		Shm->Proc.Features.Std.ECX.RDRAND ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.ExtFeature.EBX.RDSEED],
		"       RDSEED [%c]",
		Shm->Proc.Features.ExtFeature.EBX.RDSEED ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.ExtInfo.EDX.RDTSCP],
		"       RDTSCP [%c]",
		Shm->Proc.Features.ExtInfo.EDX.RDTSCP ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.Std.EDX.SEP],
		"         SEP [%c] ",
		Shm->Proc.Features.Std.EDX.SEP ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.ExtFeature.EBX.SGX],
		" SGX          [%c]",
		Shm->Proc.Features.ExtFeature.EBX.SGX ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.EDX.SSE],
		"          SSE [%c]",
		Shm->Proc.Features.Std.EDX.SSE ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.EDX.SSE2],
		"         SSE2 [%c]",
		Shm->Proc.Features.Std.EDX.SSE2 ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.Std.ECX.SSE3],
		"        SSE3 [%c] ",
		Shm->Proc.Features.Std.ECX.SSE3 ? 'Y' : 'N');
/* Row Mark */
	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.SSSE3],
		" SSSE3        [%c]",
		Shm->Proc.Features.Std.ECX.SSSE3 ? 'Y' : 'N');

	PRT(ISA, attrib[0][2 * (Shm->Proc.Features.Std.ECX.SSE41
			|  Shm->Proc.Features.ExtInfo.ECX.SSE4A)
			+ (Shm->Proc.Features.Std.ECX.SSE41
			<< Shm->Proc.Features.ExtInfo.ECX.SSE4A)],
		"  SSE4.1/4A [%c/%c]",
		Shm->Proc.Features.Std.ECX.SSE41 ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.ECX.SSE4A ? 'Y' : 'N');

	PRT(ISA, attrib[0][Shm->Proc.Features.Std.ECX.SSE42],
		"       SSE4.2 [%c]",
		Shm->Proc.Features.Std.ECX.SSE42 ? 'Y' : 'N');

	PRT(ISA, attrib[3][Shm->Proc.Features.ExtInfo.EDX.SYSCALL],
		"     SYSCALL [%c] ",
		Shm->Proc.Features.ExtInfo.EDX.SYSCALL ? 'Y' : 'N');
}

void SysInfoFeatures(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[3] = {
		RSC(SYSINFO_FEATURES_COND0).ATTR(),
		RSC(SYSINFO_FEATURES_COND1).ATTR(),
		RSC(SYSINFO_FEATURES_COND2).ATTR()
	};
	const ASCII *TSC[] = {
		RSC(MISSING).CODE(),
		RSC(VARIANT).CODE(),
		RSC(INVARIANT).CODE()
	};
	const ASCII *x2APIC[] = {
		RSC(MISSING).CODE(),
		(ASCII*) "  xAPIC",
		(ASCII*) " x2APIC"
	};
	int bix;
/* Section Mark */
	bix = Shm->Proc.Features.ExtInfo.EDX.PG_1GB == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"1 GB Pages Support%.*s1GB-PAGES   [%7s]",
		width - 42, hSpace, powered(bix));

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD) {
	bix = Shm->Proc.Features.AdvPower.EDX._100MHz == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"100 MHz multiplier Control%.*s100MHzSteps   [%7s]",
		width - 52, hSpace, powered(bix));
    }

	bix = (Shm->Proc.Features.Std.EDX.ACPI == 1)		/* Intel */
	   || (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1);	/* AMD   */
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Advanced Configuration & Power Interface%.*sACPI   [%7s]",
		width - 59, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.APIC == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Advanced Programmable Interrupt Controller%.*sAPIC   [%7s]",
		width - 61, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtInfo.ECX.MP_Mode == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Core Multi-Processing%.*sCMP Legacy   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.CNXT_ID == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"L1 Data Cache Context ID%.*sCNXT-ID   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.DCA == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Direct Cache Access%.*sDCA   [%7s]",
		width - 37, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.DE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Debugging Extension%.*sDE   [%7s]",
		width - 36, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.DS_PEBS == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Debug Store & Precise Event Based Sampling"
					"%.*sDS, PEBS   [%7s]",
		width - 65, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.DS_CPL == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"CPL Qualified Debug Store%.*sDS-CPL   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.DTES64 == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"64-Bit Debug Store%.*sDTES64   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtFeature.EBX.FastStrings == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Fast-String Operation%.*sFast-Strings   [%7s]",
		width - 48, hSpace, powered(bix));

	bix = (Shm->Proc.Features.Std.ECX.FMA == 1)
	   || (Shm->Proc.Features.ExtInfo.ECX.FMA4 == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Fused Multiply Add%.*sFMA|FMA4   [%7s]",
		width - 41, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtFeature.EBX.HLE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Hardware Lock Elision%.*sHLE   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtInfo.EDX.IA64 == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Long Mode 64 bits%.*sIA64|LM   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtInfo.ECX.LWP == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"LightWeight Profiling%.*sLWP   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.MCA == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Machine-Check Architecture%.*sMCA   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.MSR == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Model Specific Registers%.*sMSR   [%7s]",
		width - 42, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.MTRR == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Memory Type Range Registers%.*sMTRR   [%7s]",
		width - 46, hSpace, powered(bix));

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD) {
	bix = Shm->Proc.Features.ExtInfo.EDX.NX == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"No-Execute Page Protection%.*sNX   [%7s]",
		width - 43, hSpace, powered(bix));
    }

	bix = Shm->Proc.Features.Std.ECX.OSXSAVE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"OS-Enabled Ext. State Management%.*sOSXSAVE   [%7s]",
		width - 54,hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PAE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Physical Address Extension%.*sPAE   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PAT == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Page Attribute Table%.*sPAT   [%7s]",
		width - 38, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PBE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Pending Break Enable%.*sPBE   [%7s]",
		width - 38, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.PCID == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Process Context Identifiers%.*sPCID   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.PDCM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Perfmon and Debug Capability%.*sPDCM   [%7s]",
		width - 47, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PGE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Page Global Enable%.*sPGE   [%7s]",
		width - 36, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PSE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Page Size Extension%.*sPSE   [%7s]",
		width - 37, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PSE36 == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"36-bit Page Size Extension%.*sPSE36   [%7s]",
		width - 46, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.PSN == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Processor Serial Number%.*sPSN   [%7s]",
		width - 41, hSpace, powered(bix));

	bix = Shm->Proc.Features.ExtFeature.EBX.RTM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Restricted Transactional Memory%.*sRTM   [%7s]",
		width - 49, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.SMX == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Safer Mode Extensions%.*sSMX   [%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.SS == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Self-Snoop%.*sSS   [%7s]",
		width - 27, hSpace, powered(bix));

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.InvariantTSC],
		width, 2,
		"Time Stamp Counter%.*sTSC [%9s]",
		width - 36, hSpace, TSC[Shm->Proc.Features.InvariantTSC]);

	bix = Shm->Proc.Features.Std.ECX.TSCDEAD == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Time Stamp Counter Deadline%.*sTSC-DEADLINE   [%7s]",
		width - 54,hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.EDX.VME == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Virtual Mode Extension%.*sVME   [%7s]",
		width - 40, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.VMX == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Virtual Machine Extensions%.*sVMX   [%7s]",
		width - 44, hSpace, powered(bix));

	bix = Shm->Cpu[Shm->Proc.Service.Core].Topology.MP.x2APIC > 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Extended xAPIC Support%.*sx2APIC   [%7s]",
		width - 43, hSpace,
		x2APIC[Shm->Cpu[Shm->Proc.Service.Core].Topology.MP.x2APIC]);

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL) {
	bix = Shm->Proc.Features.ExtInfo.EDX.XD_Bit == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Execution Disable Bit Support%.*sXD-Bit   [%7s]",
		width - 50, hSpace, powered(bix));
    }

	bix = Shm->Proc.Features.Std.ECX.XSAVE == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"XSAVE/XSTOR States%.*sXSAVE   [%7s]",
		width - 38, hSpace, powered(bix));

	bix = Shm->Proc.Features.Std.ECX.xTPR == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"xTPR Update Control%.*sxTPR   [%7s]",
		width - 38, hSpace, powered(bix));
}

char *Hypervisor[HYPERVISORS] = {
	[HYPERV_BARE]	= "    ",
	[HYPERV_XEN]	= " Xen",
	[HYPERV_KVM]	= " KVM",
	[HYPERV_VBOX]	= "VBOX"
};

void SysInfoTech(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[2] = {
		RSC(SYSINFO_TECH_COND0).ATTR(),
		RSC(SYSINFO_TECH_COND1).ATTR()
	};
	int bix;
/* Section Mark */
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Technology.SMM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sSMM-Dual       [%3s]", RSC(TECHNOLOGIES_SMM).CODE(),
		width - 23 - RSZ(TECHNOLOGIES_SMM), hSpace, enabled(bix));

	bix = Shm->Proc.Features.HyperThreading == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sHTT       [%3s]", RSC(TECHNOLOGIES_HTT).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_HTT), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.EIST == 1;
	PUT(BOXKEY_EIST, attrib[bix], width, 2,
		"%s%.*sEIST       <%3s>", RSC(TECHNOLOGIES_EIST).CODE(),
		width - 19 - RSZ(TECHNOLOGIES_EIST), hSpace, enabled(bix));

	bix = Shm->Proc.Features.Power.EAX.TurboIDA == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sIDA       [%3s]", RSC(TECHNOLOGIES_IDA).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_IDA), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.Turbo == 1;
	PUT(BOXKEY_TURBO, attrib[bix], width, 2,
		"%s%.*sTURBO       <%3s>", RSC(TECHNOLOGIES_TURBO).CODE(),
		width - 20 - RSZ(TECHNOLOGIES_TURBO), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.VM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sVMX       [%3s]", RSC(TECHNOLOGIES_VM).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_VM), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.IOMMU == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 3,
		"%s%.*sVT-d       [%3s]", RSC(TECHNOLOGIES_IOMMU).CODE(),
		width - (OutFunc ? 20 : 22) - RSZ(TECHNOLOGIES_IOMMU),
		hSpace, enabled(bix));
    }
    else if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    {
	bix = Shm->Proc.Technology.SMM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sSMM-Lock       [%3s]", RSC(TECHNOLOGIES_SMM).CODE(),
		width - 23 - RSZ(TECHNOLOGIES_SMM), hSpace, enabled(bix));

	bix = Shm->Proc.Features.HyperThreading == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sSMT       [%3s]", RSC(TECHNOLOGIES_SMT).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_SMT), hSpace, enabled(bix));

	bix = Shm->Proc.PowerNow == 0b11;	/*	VID + FID	*/
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sCnQ       [%3s]", RSC(TECHNOLOGIES_CNQ).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_CNQ), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.Turbo == 1;
	PUT(BOXKEY_TURBO, attrib[bix], width, 2,
		"%s%.*sCPB       <%3s>", RSC(TECHNOLOGIES_CPB).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_CPB), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.VM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sSVM       [%3s]", RSC(TECHNOLOGIES_VM).CODE(),
		width - 18 - RSZ(TECHNOLOGIES_VM), hSpace, enabled(bix));

	bix = Shm->Proc.Technology.IOMMU == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 3,
		"%s%.*sAMD-V       [%3s]", RSC(TECHNOLOGIES_IOMMU).CODE(),
		width - (OutFunc? 21 : 23) - RSZ(TECHNOLOGIES_IOMMU),
		hSpace, enabled(bix));
    }
	bix = Shm->Proc.Features.Std.ECX.Hyperv == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 3,
		"%s%.*s""%s       [%3s]", RSC(TECHNOLOGIES_HYPERV).CODE(),
		width - (OutFunc? 20 : 22) - RSZ(TECHNOLOGIES_HYPERV), hSpace,
		Hypervisor[Shm->Proc.HypervisorID], enabled(bix));
}

void SysInfoPerfMon(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_PERFMON_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_COND1).ATTR(),
		RSC(SYSINFO_PERFMON_COND2).ATTR(),
		RSC(SYSINFO_PERFMON_COND3).ATTR()
	};
	int bix;
/* Section Mark */
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sPM       [%3d]", RSC(VERSION).CODE(),
		width - 17 - RSZ(VERSION), hSpace, Shm->Proc.PM_version);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s:%.*s%s%.*s%s",
		RSC(COUNTERS).CODE(),10, hSpace, RSC(GENERAL_CTRS).CODE(),
		width - 61, hSpace, RSC(FIXED_CTRS).CODE());

    if (OutFunc == NULL) {
	PUT(SCANKEY_NULL, attrib[0], width, 1,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.EAX.MonCtrs,
				Shm->Proc.Features.PerfMon.EAX.MonWidth,
		11, hSpace,	Shm->Proc.Features.PerfMon.EDX.FixCtrs,
				Shm->Proc.Features.PerfMon.EDX.FixWidth);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 0,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.EAX.MonCtrs,
				Shm->Proc.Features.PerfMon.EAX.MonWidth,
		5, hSpace,	Shm->Proc.Features.PerfMon.EDX.FixCtrs,
				Shm->Proc.Features.PerfMon.EDX.FixWidth);
    }
	bix = Shm->Proc.Technology.C1E == 1;
	PUT(BOXKEY_C1E, attrib[bix], width, 2,
		"Enhanced Halt State%.*sC1E       <%3s>",
		width - 37, hSpace, enabled(bix));

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Technology.C1A == 1;
	PUT(BOXKEY_C1A, attrib[bix], width, 2,
		"C1 Auto Demotion%.*sC1A       <%3s>",
		width - 34, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.C3A == 1;
	PUT(BOXKEY_C3A, attrib[bix], width, 2,
		"C3 Auto Demotion%.*sC3A       <%3s>",
		width - 34, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.C1U == 1;
	PUT(BOXKEY_C1U, attrib[bix], width, 2,
		"C1 UnDemotion%.*sC1U       <%3s>",
		width - 31, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.C3U == 1;
	PUT(BOXKEY_C3U, attrib[bix], width, 2,
		"C3 UnDemotion%.*sC3U       <%3s>",
		width - 31, hSpace, enabled(bix));
    }
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    {
	bix = Shm->Proc.Technology.CC6 == 1;
	PUT(BOXKEY_CC6, attrib[bix], width, 2,
		"Core C6 State%.*sCC6       <%3s>",
		width - 31, hSpace, enabled(bix));

	bix = Shm->Proc.Technology.PC6 == 1;
	PUT(BOXKEY_PC6, attrib[bix], width, 2,
		"Package C6 State%.*sPC6       <%3s>",
		width - 34, hSpace, enabled(bix));
    }
	bix = (Shm->Proc.Features.AdvPower.EDX.FID == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Frequency ID control%.*sFID       [%3s]",
		width - 38, hSpace, enabled(bix));

	bix = (Shm->Proc.Features.AdvPower.EDX.VID == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Voltage ID control%.*sVID       [%3s]",
		width - 36, hSpace, enabled(bix));

	bix = (Shm->Proc.Features.Power.ECX.HCF_Cap == 1)
	   || ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		&& (Shm->Proc.Features.AdvPower.EDX.EffFrqRO == 1));
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"P-State Hardware Coordination Feedback"	\
			"%.*sMPERF/APERF       [%3s]",
		width - 64, hSpace, enabled(bix));

	bix = (Shm->Proc.Features.Power.EAX.HWP_Reg == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Hardware-Controlled Performance States%.*sHWP       [%3s]",
		width - 56, hSpace, enabled(bix));

	bix = Shm->Proc.Features.Power.EAX.HDC_Reg == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Hardware Duty Cycling%.*sHDC       [%3s]",
		width - 39, hSpace, enabled(bix));

	PUT(SCANKEY_NULL, attrib[0], width, 2,"Package C-State",NULL);

	bix = Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock == 0 ? 3 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 3,
		"Configuration Control%.*sCONFIG   [%7s]",
		width - (OutFunc == NULL ? 45 : 43), hSpace,
		!Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock ?
			RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

	if (!Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock) {
		PUT(BOXKEY_PKGCST, attrib[0], width, 3,
			"Lowest C-State%.*sLIMIT   <%7d>",
			width - (OutFunc == NULL ? 37 : 35), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit);

		bix = Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir == 1 ? 3:2;
		PUT(BOXKEY_IOMWAIT, attrib[bix], width, 3,
			"I/O MWAIT Redirection%.*sIOMWAIT   <%7s>",
			width - (OutFunc == NULL ? 46 : 44), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE());

		PUT(BOXKEY_IORCST, attrib[0], width, 3,
			"Max C-State Inclusion%.*sRANGE   <%7d>",
			width - (OutFunc == NULL ? 44 : 42), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude);
	} else {
		PUT(SCANKEY_NULL, attrib[0], width, 3,
			"Lowest C-State%.*sLIMIT   [%7d]",
			width - (OutFunc == NULL ? 37 : 35), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit);

		PUT(SCANKEY_NULL, attrib[0], width, 3,
			"I/O MWAIT Redirection%.*sIOMWAIT   [%7s]",
			width - (OutFunc == NULL ? 46 : 44), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE());

		PUT(SCANKEY_NULL, attrib[0], width, 3,
			"Max C-State Inclusion%.*sRANGE   [%7d]",
			width - (OutFunc == NULL ? 44 : 42), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude);
	}
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"MWAIT States:%.*sC0    C1    C2    C3    C4    C5    C6    C7",
		04, hSpace);

	PUT(SCANKEY_NULL, attrib[0], width, (OutFunc == NULL) ? 1 : 0,
		"%.*s%2d    %2d    %2d    %2d    %2d    %2d    %2d    %2d",
		19, hSpace,
		Shm->Proc.Features.MWait.EDX.Num_C0_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C1_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C2_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C3_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C4_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C5_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C6_MWAIT,
		Shm->Proc.Features.MWait.EDX.Num_C7_MWAIT);

	bix = Shm->Proc.Features.PerfMon.EBX.CoreCycles == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Core Cycles%.*s[%7s]",
		width - 23, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.InstrRetired == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Instructions Retired%.*s[%7s]",
		width - 32, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.RefCycles == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Reference Cycles%.*s[%7s]",
		width - 28, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.LLC_Ref == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Last Level Cache References%.*s[%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.LLC_Misses == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Last Level Cache Misses%.*s[%7s]",
		width - 35, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.BranchRetired == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Branch Instructions Retired%.*s[%7s]",
		width - 39, hSpace, powered(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.BranchMispred == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"Branch Mispredicts Retired%.*s[%7s]",
		width - 38, hSpace, powered(bix));
}

void SysInfoPwrThermal(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_PWR_THERMAL_COND0).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND1).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND2).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND3).ATTR()
	};
	const ASCII *TM[] = {
		RSC(MISSING).CODE(),
		RSC(PRESENT).CODE(),
		RSC(DISABLE).CODE(),
		RSC(ENABLE).CODE(),
	}, *Unlock[] = {
		RSC(LOCK).CODE(),
		RSC(UNLOCK).CODE()
	};
	int bix;
/* Section Mark */
	bix = Shm->Proc.Technology.ODCM == 1 ? 3 : 1;
	PUT(BOXKEY_ODCM, attrib[bix], width, 2,
		"%s%.*sODCM   <%7s>", RSC(POWER_THERMAL_ODCM).CODE(),
		width - 19 - RSZ(POWER_THERMAL_ODCM), hSpace,
		Shm->Proc.Technology.ODCM ?
			RSC(ENABLE).CODE() : RSC(DISABLE).CODE());

	PUT(BOXKEY_DUTYCYCLE, attrib[0], width, 3,
		"%s%.*s<%6.2f%%>", RSC(POWER_THERMAL_DUTY).CODE(),
	width - (OutFunc == NULL ? 15: 13) - RSZ(POWER_THERMAL_DUTY), hSpace,
	(Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.Extended ?
		6.25f : 12.5f
	* Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod));

	bix = Shm->Proc.Technology.PowerMgmt == 1 ? 3 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sPWR MGMT   [%7s]", RSC(POWER_THERMAL_MGMT).CODE(),
		width - 23 - RSZ(POWER_THERMAL_MGMT), hSpace,
		Unlock[Shm->Proc.Technology.PowerMgmt]);

	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*sBias Hint   [%7u]", RSC(POWER_THERMAL_BIAS).CODE(),
	width - (OutFunc == NULL ? 27 : 25) - RSZ(POWER_THERMAL_BIAS), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.PowerPolicy);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sTjMax   [%3u:%3u]", RSC(POWER_THERMAL_TJMAX).CODE(),
		width - 20 - RSZ(POWER_THERMAL_TJMAX), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.Param.Offset[1],
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.Param.Offset[0]);

	bix = (Shm->Proc.Features.Power.EAX.DTS == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.TS == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sDTS   [%7s]", RSC(POWER_THERMAL_DTS).CODE(),
		width - 18 - RSZ(POWER_THERMAL_DTS), hSpace, powered(bix));

	bix = Shm->Proc.Features.Power.EAX.PLN == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sPLN   [%7s]", RSC(POWER_THERMAL_PLN).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PLN), hSpace, powered(bix));

	bix = Shm->Proc.Features.Power.EAX.PTM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sPTM   [%7s]", RSC(POWER_THERMAL_PTM).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PTM), hSpace, powered(bix));

	bix = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1
	    | Shm->Proc.Features.AdvPower.EDX.TTP;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sTM1|TTP   [%7s]", RSC(POWER_THERMAL_TM1).CODE(),
		width - 22 - RSZ(POWER_THERMAL_TM1), hSpace,
		TM[  Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1
			| Shm->Proc.Features.AdvPower.EDX.TTP ]);

	bix = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2
	    | Shm->Proc.Features.AdvPower.EDX.TM;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sTM2|HTC   [%7s]", RSC(POWER_THERMAL_TM2).CODE(),
		width - 22 - RSZ(POWER_THERMAL_TM2), hSpace,
		TM[  Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2
			| Shm->Proc.Features.AdvPower.EDX.TM ]);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		(char*) RSC(POWER_THERMAL_UNITS).CODE(), NULL);

	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]",
		RSC(POWER_THERMAL_POWER).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		 - RSZ(POWER_THERMAL_POWER) - RSZ(POWER_THERMAL_WATT), hSpace,
		RSC(POWER_THERMAL_WATT).CODE(), Shm->Proc.Power.Unit.Watts);

	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]", RSC(POWER_THERMAL_ENERGY).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_ENERGY) - RSZ(POWER_THERMAL_JOULE), hSpace,
		RSC(POWER_THERMAL_JOULE).CODE(), Shm->Proc.Power.Unit.Joules);

	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]", RSC(POWER_THERMAL_WINDOW).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_WINDOW) - RSZ(POWER_THERMAL_SECOND), hSpace,
		RSC(POWER_THERMAL_SECOND).CODE(), Shm->Proc.Power.Unit.Times);
}

void SysInfoKernel(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	size_t	len = 0, sln;
	char	*item = malloc(width + 1),
		*str = malloc(width + 1);
	int	idx = 0;
/* Section Mark */
	PUT(SCANKEY_NULL, RSC(SYSINFO_KERNEL).ATTR(), width, 0,
		"%s:", Shm->SysGate.sysname);

	PUT(SCANKEY_NULL, RSC(KERNEL_RELEASE).ATTR(), width, 2,
		"%s%.*s[%s]", RSC(KERNEL_RELEASE).CODE(),
		width - 5 - RSZ(KERNEL_RELEASE)- strlen(Shm->SysGate.release),
		hSpace, Shm->SysGate.release);

	PUT(SCANKEY_NULL, RSC(KERNEL_VERSION).ATTR(), width, 2,
		"%s%.*s[%s]", RSC(KERNEL_VERSION).CODE(),
		width - 5 - RSZ(KERNEL_VERSION) - strlen(Shm->SysGate.version),
		hSpace, Shm->SysGate.version);

	PUT(SCANKEY_NULL, RSC(KERNEL_MACHINE).ATTR(), width, 2,
		"%s%.*s[%s]", RSC(KERNEL_MACHINE).CODE(),
		width - 5 - RSZ(KERNEL_MACHINE) - strlen(Shm->SysGate.machine),
		hSpace, Shm->SysGate.machine);
/* Section Mark */
	PUT(SCANKEY_NULL, RSC(KERNEL_MEMORY).ATTR(), width, 0,
		"%s:%.*s", RSC(KERNEL_MEMORY).CODE(),
		width - 1 - RSZ(KERNEL_MEMORY), hSpace);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.totalram);
	PUT(SCANKEY_NULL, RSC(KERNEL_TOTAL_RAM).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_TOTAL_RAM).CODE(),
		width - 6 - RSZ(KERNEL_TOTAL_RAM) - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.sharedram);
	PUT(SCANKEY_NULL, RSC(KERNEL_SHARED_RAM).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_SHARED_RAM).CODE(),
		width - 6 - RSZ(KERNEL_SHARED_RAM) - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.freeram);
	PUT(SCANKEY_NULL, RSC(KERNEL_FREE_RAM).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_FREE_RAM).CODE(),
		width - 6 - RSZ(KERNEL_FREE_RAM) - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.bufferram);
	PUT(SCANKEY_NULL, RSC(KERNEL_BUFFER_RAM).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_BUFFER_RAM).CODE(),
		width - 6 - RSZ(KERNEL_BUFFER_RAM) - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.totalhigh);
	PUT(SCANKEY_NULL, RSC(KERNEL_TOTAL_HIGH).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_TOTAL_HIGH).CODE(),
		width - 6 - RSZ(KERNEL_TOTAL_HIGH) - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.freehigh);
	PUT(SCANKEY_NULL, RSC(KERNEL_FREE_HIGH).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_FREE_HIGH).CODE(),
		width - 6 - RSZ(KERNEL_FREE_HIGH) - len, hSpace, str);
/* Section Mark */
  if ((len = strlen(Shm->SysGate.IdleDriver.Name)
		+ strlen(Shm->SysGate.IdleDriver.Governor)) > 0)
  {
	PUT(SCANKEY_NULL, RSC(KERNEL_IDLE_DRIVER).ATTR(), width, 0,
		"%s%.*s[%s@%s]", RSC(KERNEL_IDLE_DRIVER).CODE(),
		width - 3 - RSZ(KERNEL_IDLE_DRIVER) - len, hSpace,
		Shm->SysGate.IdleDriver.Governor, Shm->SysGate.IdleDriver.Name);
/* Row Mark */
	len = sprintf(item, "%s:%.*s", RSC(KERNEL_STATE).CODE(),
			15 - (int) RSZ(KERNEL_STATE), hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
				idx++, len += sln, strncat(item, str, sln))
    {
	sln = sprintf(str, "%-8s", Shm->SysGate.IdleDriver.State[idx].Name);
    }
	PUT(SCANKEY_NULL, RSC(KERNEL_STATE).ATTR(), width, 3, item, NULL);
/* Row Mark */
	len = sprintf(item, "%s:%.*s", RSC(KERNEL_POWER).CODE(),
			15 - (int) RSZ(KERNEL_POWER), hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
				idx++, len += sln, strncat(item, str, sln))
    {
	sln=sprintf(str,"%-8d",Shm->SysGate.IdleDriver.State[idx].powerUsage);
    }
	PUT(SCANKEY_NULL, RSC(KERNEL_POWER).ATTR(), width, 3, item, NULL);
/* Row Mark */
	len = sprintf(item, "%s:%.*s", RSC(KERNEL_LATENCY).CODE(),
			15 - (int) RSZ(KERNEL_LATENCY), hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
				idx++, len += sln, strncat(item, str, sln))
    {
	sln=sprintf(str,"%-8u",Shm->SysGate.IdleDriver.State[idx].exitLatency);
    }
	PUT(SCANKEY_NULL, RSC(KERNEL_LATENCY).ATTR(), width, 3, item, NULL);
/* Row Mark */
	len = sprintf(item, "%s:%.*s", RSC(KERNEL_RESIDENCY).CODE(),
			15 - (int) RSZ(KERNEL_RESIDENCY), hSpace);
    for (idx = 0, sln = 0; (idx < Shm->SysGate.IdleDriver.stateCount)
			 && (3 + len + sln <= width);
				idx++, len += sln, strncat(item, str, sln))
    {
    sln=sprintf(str,"%-8u",Shm->SysGate.IdleDriver.State[idx].targetResidency);
    }
	PUT(SCANKEY_NULL, RSC(KERNEL_RESIDENCY).ATTR(), width, 3, item, NULL);
  }
	free(item);
	free(str);
}

void Package(void)
{
    while (!BITVAL(Shutdown, 0)) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !BITVAL(Shutdown, 0))
		nanosleep(&Shm->Sleep.pollingWait, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

	ClientFollowService(&localService, &Shm->Proc.Service, 0);

	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
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
		PFlop->Delta.PC02, 100.f * Shm->Proc.State.PC02,
		PFlop->Delta.PC03, 100.f * Shm->Proc.State.PC03,
		PFlop->Delta.PC06, 100.f * Shm->Proc.State.PC06,
		PFlop->Delta.PC07, 100.f * Shm->Proc.State.PC07,
		PFlop->Delta.PC08, 100.f * Shm->Proc.State.PC08,
		PFlop->Delta.PC09, 100.f * Shm->Proc.State.PC09,
		PFlop->Delta.PC10, 100.f * Shm->Proc.State.PC10,
		PFlop->Delta.PTSC,
		PFlop->Uncore.FC0);
    }
}

void Counters(void)
{
    unsigned int cpu = 0;
    while (!BITVAL(Shutdown, 0)) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !BITVAL(Shutdown, 0))
		nanosleep(&Shm->Sleep.pollingWait, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

	ClientFollowService(&localService, &Shm->Proc.Service, 0);

		printf( "CPU Freq(MHz) Ratio  Turbo"			\
			"  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
			"  Min TMP:TS  Max\n");
	for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown,0); cpu++)
	{
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW)) {
	    struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printf( "#%02u %7.2f (%5.2f)"				\
			" %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"		\
			"  %-3u/%3u:%-3u/%3u\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Relative.Ratio,
			100.f * CFlop->State.Turbo,
			100.f * CFlop->State.C0,
			100.f * CFlop->State.C1,
			100.f * CFlop->State.C3,
			100.f * CFlop->State.C6,
			100.f * CFlop->State.C7,
			Shm->Cpu[cpu].PowerThermal.Limit[0],
			CFlop->Thermal.Temp,
			CFlop->Thermal.Sensor,
			Shm->Cpu[cpu].PowerThermal.Limit[1]);
	    else
		printf("#%02u        OFF\n", cpu);
	  }
	}
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	printf( "\n"							\
		"%.*s" "Averages:"					\
		"%.*s" "Turbo  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
		"%.*s" "TjMax:" "%.*s" "Pkg:\n"				\
		"%.*s" "%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"		\
		"%.*s" "%3u C" "%.*s" "%3u C\n\n",
		4, hSpace,
		8, hSpace,
		4, hSpace,
		4, hSpace,
		20, hSpace,
		100.f * Shm->Proc.Avg.Turbo,
		100.f * Shm->Proc.Avg.C0,
		100.f * Shm->Proc.Avg.C1,
		100.f * Shm->Proc.Avg.C3,
		100.f * Shm->Proc.Avg.C6,
		100.f * Shm->Proc.Avg.C7,
		5, hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.Param.Offset[0],
		3, hSpace,
		PFlop->Thermal.Temp);
    }
}

void Voltage(void)
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
	    struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
		printf("#%02u %7.2f %5d  %5.4f\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Voltage.VID,
			CFlop->Voltage.Vcore);
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

void Instructions(void)
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
		struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
			printf("#%02u %12.6f/s %12.6f/c %12.6f/i\n",
				cpu,
				CFlop->State.IPS,
				CFlop->State.IPC,
				CFlop->State.CPI);
		else
			printf("#%02u\n", cpu);
	    }
		printf("\n");
    }
}

void Topology(Window *win, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[3] = {
		RSC(TOPOLOGY_COND0).ATTR(),
		RSC(TOPOLOGY_COND1).ATTR(),
		RSC(TOPOLOGY_COND2).ATTR()
	};
	unsigned int cpu = 0, level = 0;
	CUINT nl = win->matrix.size.wth;

	PRT(MAP, attrib[2], "CPU Pkg  Apic");
	PRT(MAP, attrib[2], "  Core Thread");
	PRT(MAP, attrib[2], "  Caches     ");
	PRT(MAP, attrib[2], " (w)rite-Back");
	PRT(MAP, attrib[2], " (i)nclusive ");
	PRT(MAP, attrib[2], "             ");
	PRT(MAP, attrib[2], " #   ID   ID ");
	PRT(MAP, attrib[2], "   ID     ID ");
	PRT(MAP, attrib[2], " L1-Inst Way ");
	PRT(MAP, attrib[2], " L1-Data Way ");
	PRT(MAP, attrib[2], "     L2  Way ");
	PRT(MAP, attrib[2], "     L3  Way ");

    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	if (Shm->Cpu[cpu].Topology.MP.BSP)
		PRT(MAP,attrib[BITVAL(Shm->Cpu[cpu].OffLine,OS)],
			"%02u: BSP%6d",
			cpu,
			Shm->Cpu[cpu].Topology.ApicID);
	else
		PRT(MAP,attrib[BITVAL(Shm->Cpu[cpu].OffLine,OS)],
			"%02u:%4d%6d",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.ApicID);

	PRT(MAP, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
		"%6d %6d",
		Shm->Cpu[cpu].Topology.CoreID,
		Shm->Cpu[cpu].Topology.ThreadID);

	for (level = 0; level < CACHE_MAX_LEVEL; level++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(MAP, attrib[0], "%8u%3u%c%c",
		Shm->Cpu[cpu].Topology.Cache[level].Size,
		Shm->Cpu[cpu].Topology.Cache[level].Way,
		Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack ?
			'w' : 0x20,
		Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive ?
			'i' : 0x20);
	    } else
		PRT(MAP, attrib[1], "       -  -  ");
	}
    }
}

void iSplit(unsigned int sInt, char hInt[]) {
	char fInt[16];
	sprintf(fInt, "%10u", sInt);
	memcpy((hInt + 0), (fInt + 0), 5); *(hInt + 0 + 5) = '\0';
	memcpy((hInt + 8), (fInt + 5), 5); *(hInt + 8 + 5) = '\0';
}

void MemoryController(Window *win, CELL_FUNC OutFunc)
{
	#define MATX 14
	#define MATY 5
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	size_t len = strlen(Shm->Uncore.Chipset.CodeName);
	CUINT nl = win->matrix.size.wth, ni, nr, nc;
	unsigned short mc, cha, slot;
	char hInt[16], **codeName = malloc(MATX * sizeof(char *));

  if (codeName != NULL) {
	for (nc = 0; nc < MATX; nc++) {
		if ((codeName[nc] = malloc(MATY + 1)) != NULL) {
			memset(codeName[nc], 0x20, MATY);
			codeName[nc][MATY] = '\0';
		} else
			goto IMC_FREE;
	}
	for (ni = 0; ni < len; ni++) {
		nc = (MATX + len + ni) / MATY;
		nr = (MATX + len + ni) % MATY;
		codeName[nc][nr] = Shm->Uncore.Chipset.CodeName[ni];
	}
	for (nc = 0; nc < MATX; nc++)
		PRT(IMC, attrib[1], codeName[nc]);
IMC_FREE:
	while (nc > 0)
		free(codeName[--nc]);
	free(codeName);
  }
  for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
  {
	PRT(IMC, attrib[0], "Contr");		PRT(IMC, attrib[0], "oller");
	PRT(IMC, attrib[1], " #%-3u",mc);	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");		PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");		PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");		PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");

	switch (Shm->Uncore.MC[mc].ChannelCount) {
	case 1:
		PRT(IMC, attrib[1], "Singl");
		PRT(IMC, attrib[1], "e Cha");
		PRT(IMC, attrib[1], "nnel ");
		break;
	case 2:
		PRT(IMC, attrib[1], " Dual");
		PRT(IMC, attrib[1], " Chan");
		PRT(IMC, attrib[1], "nel  ");
		break;
	case 3:
		PRT(IMC, attrib[1], "Tripl");
		PRT(IMC, attrib[1], "e Cha");
		PRT(IMC, attrib[1], "nnel ");
		break;
	case 4:
		PRT(IMC, attrib[1], " Quad");
		PRT(IMC, attrib[1], " Chan");
		PRT(IMC, attrib[1], "nel  ");
		break;
	case 6:
		PRT(IMC, attrib[1], "  Six");
		PRT(IMC, attrib[1], " Chan");
		PRT(IMC, attrib[1], "nel  ");
		break;
	case 8:
		PRT(IMC, attrib[1], "Eight");
		PRT(IMC, attrib[1], " Chan");
		PRT(IMC, attrib[1], "nel  ");
		break;
	default:
		PRT(IMC, attrib[0], "     ");
		PRT(IMC, attrib[0], "     ");
		PRT(IMC, attrib[0], "     ");
		break;
	}

	PRT(IMC, attrib[0], " Bus ");	PRT(IMC, attrib[0], "Rate ");
	PRT(IMC, attrib[1], "%5llu", Shm->Uncore.Bus.Rate);

	switch (Shm->Uncore.Unit.Bus_Rate) {
	case 0b00:
		PRT(IMC, attrib[0], " MHz ");
		break;
	case 0b01:
		PRT(IMC, attrib[0], " MT/s");
		break;
	case 0b10:
		PRT(IMC, attrib[0], " MB/s");
		break;
	case 0b11:
		PRT(IMC, attrib[0], "     ");
		break;
	}
	PRT(IMC, attrib[0], "     ");

	PRT(IMC, attrib[0], " Bus ");	PRT(IMC, attrib[0], "Speed");
	PRT(IMC, attrib[1], "%5llu", Shm->Uncore.Bus.Speed);

	switch (Shm->Uncore.Unit.BusSpeed) {
	case 0b00:
		PRT(IMC, attrib[0], " MHz ");
		break;
	case 0b01:
		PRT(IMC, attrib[0], " MT/s");
		break;
	case 0b10:
		PRT(IMC, attrib[0], " MB/s");
		break;
	case 0b11:
		PRT(IMC, attrib[0], "     ");
		break;
	}
	PRT(IMC, attrib[0], "     ");

	PRT(IMC, attrib[0], "DRAM ");	PRT(IMC, attrib[0], "Speed");
	PRT(IMC, attrib[1], "%5llu", Shm->Uncore.CtrlSpeed);

	switch (Shm->Uncore.Unit.DDRSpeed) {
	case 0b00:
		PRT(IMC, attrib[0], " MHz ");
		break;
	case 0b01:
		PRT(IMC, attrib[0], " MT/s");
		break;
	case 0b10:
		PRT(IMC, attrib[0], " MB/s");
		break;
	case 0b11:
		PRT(IMC, attrib[0], "     ");
		break;
	}

	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");

	PRT(IMC, attrib[0], " Cha ");
	PRT(IMC, attrib[0], "   CL");	PRT(IMC, attrib[0], "  RCD");
	PRT(IMC, attrib[0], "   RP");	PRT(IMC, attrib[0], "  RAS");
	PRT(IMC, attrib[0], "  RRD");	PRT(IMC, attrib[0], "  RFC");
	PRT(IMC, attrib[0], "   WR");	PRT(IMC, attrib[0], " RTPr");
	PRT(IMC, attrib[0], " WTPr");	PRT(IMC, attrib[0], "  FAW");
	PRT(IMC, attrib[0], "  B2B");	PRT(IMC, attrib[0], "  CWL");
	PRT(IMC, attrib[0], " Rate");

    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	PRT(IMC, attrib[1], "\x20\x20#%-2u", cha);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tCL);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRP);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tWR);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.B2B);
	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL);
	PRT(IMC, attrib[1], "%4uN",
			Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);
    }
	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], " ddWR");	PRT(IMC, attrib[0], " drWR");
	PRT(IMC, attrib[0], " srWR");	PRT(IMC, attrib[0], " ddRW");
	PRT(IMC, attrib[0], " drRW");	PRT(IMC, attrib[0], " srRW");
	PRT(IMC, attrib[0], " ddRR");	PRT(IMC, attrib[0], " drRR");
	PRT(IMC, attrib[0], " srRR");	PRT(IMC, attrib[0], " ddWW");
	PRT(IMC, attrib[0], " drWW");	PRT(IMC, attrib[0], " srWW");
	PRT(IMC, attrib[0], "  ECC");

    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
      PRT(IMC, attrib[1],"\x20\x20#%-2u", cha);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd);

      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr);

      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd);

      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr);
      PRT(IMC, attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr);

      PRT(IMC, attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.ECC);
    }
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");

    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
    {
	PRT(IMC, attrib[0], " DIMM");	PRT(IMC, attrib[0], " Geom");
	PRT(IMC, attrib[0], "etry ");	PRT(IMC, attrib[0], "for c");
	PRT(IMC, attrib[0], "hanne");	PRT(IMC, attrib[0], "l #%-2u", cha);
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");

	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], " Slot");
	PRT(IMC, attrib[0], " Bank");	PRT(IMC, attrib[0], " Rank");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "Rows ");
	PRT(IMC, attrib[0], "  Col");	PRT(IMC, attrib[0], "umns ");
	PRT(IMC, attrib[0], "   Me");	PRT(IMC, attrib[0], "mory ");
	PRT(IMC, attrib[0], "Size ");	PRT(IMC, attrib[0], "(MB) ");
	PRT(IMC, attrib[0], "     ");	PRT(IMC, attrib[0], "     ");

      for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[1], "\x20\x20#%-2u", slot);
	PRT(IMC, attrib[1],
		"%5u",Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks);
	PRT(IMC, attrib[1],
		"%5u",Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);
	iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows, hInt);
	PRT(IMC, attrib[1], "%5s", &hInt[0]);
	PRT(IMC, attrib[1], "%5s", &hInt[8]);
	iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols, hInt);
	PRT(IMC, attrib[1], "%5s", &hInt[0]);
	PRT(IMC, attrib[1], "%5s", &hInt[8]);
	PRT(IMC, attrib[0], "     ");
	iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size, hInt);
	PRT(IMC, attrib[1], "%5s", &hInt[0]);
	PRT(IMC, attrib[1], "%5s", &hInt[8]);
	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");
	PRT(IMC, attrib[0], "     ");
      }
    }
  }
}

/* >>> GLOBALS >>> */
static char *buffer = NULL;

Coordinate *cTask = NULL;

CardList cardList = {.head = NULL, .tail = NULL};

struct {
	double		TopFreq,
			TopLoad;
	unsigned long	FreeRAM;
	int		TaskCount;
} previous = {
	.TopFreq = 0.0,
	.TopLoad = 0.0,
	.FreeRAM = 0,
	.TaskCount = 0
};

struct {
	double		Minimum,
			Maximum,
			Median;
	unsigned int	Uniq[BOOST(SIZE)],
			Count;
} ratio = {
	.Count = 0
};

struct {
	struct {
	unsigned int
		layout	:  1-0,		/* Draw layout			*/
		clear	:  2-1,		/* Clear screen			*/
		height	:  3-2,		/* Valid height			*/
		width	:  4-3,		/* Valid width			*/
		daemon	:  5-4,		/* Draw dynamic			*/
		taskVal :  6-5,		/* Display task's value		*/
		avgOrPC :  7-6,		/* C-states average || % pkg states */
		clkOrLd :  8-7,		/* Relative freq. || % load	*/
		_padding: 32-8;
	} Flag;
	enum VIEW	View;
	enum DISPOSAL	Disposal;
	SCREEN_SIZE	Size;
	struct {
		CUINT	MinHeight;
		CUINT	MaxRows;
		CUINT	LoadWidth;
	} Area;
	unsigned int	iClock,
			cpuScroll;
} draw = {
	.Flag = {
		.layout = 0,
		.clear	= 0,
		.height = 0,
		.width	= 0,
		.daemon = 0,
		.taskVal= 0,
		.avgOrPC= 0,
		.clkOrLd= 0
	},
	.View		= V_FREQ,
	.Disposal	= D_MAINVIEW,
	.Size		= {.width = 0, .height = 0},
	.Area		= {.MinHeight = 0, .MaxRows = 0, .LoadWidth = 0},
	.iClock 	= 0,
	.cpuScroll	= 0
};

enum THERM_PWR_EVENTS processorEvents = EVENT_THERM_NONE;
/* <<< GLOBALS <<< */

void SortUniqRatio()
{
	unsigned int idx, jdx;
	ratio.Minimum = ratio.Maximum = (double) Shm->Proc.Boost[BOOST(MIN)];
	ratio.Count = 0;
	for (idx = BOOST(MIN); idx < BOOST(SIZE); idx++)
	    if (Shm->Proc.Boost[idx] > 0) {
		for (jdx = BOOST(MIN); jdx < ratio.Count; jdx++)
		    if (Shm->Proc.Boost[idx] == ratio.Uniq[jdx])
			break;
		if (jdx == ratio.Count) {
			ratio.Uniq[ratio.Count] = Shm->Proc.Boost[idx];
			if ((double) ratio.Uniq[ratio.Count] > ratio.Maximum)
				ratio.Maximum =(double) ratio.Uniq[ratio.Count];
			ratio.Count++;
		}
	}
	for (idx = BOOST(MAX); idx < ratio.Count; idx++) {
		unsigned int tmpRatio = ratio.Uniq[idx];
		jdx = idx;
		while (jdx > BOOST(MIN) && tmpRatio < ratio.Uniq[jdx - 1]) {
			ratio.Uniq[jdx] = ratio.Uniq[jdx - 1];
			--jdx;
		}
		ratio.Uniq[jdx] = tmpRatio;
	}
	ratio.Median = (Shm->Proc.Boost[BOOST(ACT)] > 0) ?
		Shm->Proc.Boost[BOOST(ACT)]
		: (ratio.Minimum + ratio.Maximum) / 2;
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
	    Threshold(value2,ratio.Minimum,ratio.Median,_GREEN,_YELLOW,_RED));\
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

#define Sys2LCD(layer, col, row, value) 				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/6.6f, 50.0, _RED,_YELLOW,_WHITE))

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

    if ((len = draw.Size.width-win->lazyComp.rowLen-win->matrix.origin.col) > 0)
	LayerFillAt(	win->layer,
			(win->matrix.origin.col + win->lazyComp.rowLen),
			win->matrix.origin.row,
			len, hSpace,
			win->hook.color[0].title);
}

Window *CreateMenu(unsigned long long id)
{
	Window *wMenu = CreateWindow(wLayer, id, 3, 12, 3, 0);
    if (wMenu != NULL) {
	ATTRIBUTE voidAttr = {.value = 0},
		sameAttr = {.fg = BLACK, .bg = WHITE, .bf = 0},
		*fkeyAttr = RSC(CREATE_MENU_FN_KEY).ATTR(),
		*skeyAttr = RSC(CREATE_MENU_SHORTKEY).ATTR(),
		*gateAttr = BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) ?
				RSC(CREATE_MENU_SHORTKEY).ATTR()
				: RSC(CREATE_MENU_STOP).ATTR(),
		*ctrlAttr = (Shm->Uncore.CtrlCount > 0) ?
				RSC(CREATE_MENU_SHORTKEY).ATTR()
				: RSC(CREATE_MENU_STOP).ATTR();

  StoreTCell(wMenu, SCANKEY_NULL, RSC(MENU_ITEM_MENU).CODE()	, sameAttr);
  StoreTCell(wMenu, SCANKEY_NULL, RSC(MENU_ITEM_VIEW).CODE()	, sameAttr);
  StoreTCell(wMenu, SCANKEY_NULL, RSC(MENU_ITEM_WINDOW).CODE()	, sameAttr);

  StoreTCell(wMenu, SCANKEY_s, RSC(MENU_ITEM_SETTINGS).CODE()	, skeyAttr);
  StoreTCell(wMenu, SCANKEY_d, RSC(MENU_ITEM_DASHBOARD).CODE()	, skeyAttr);
  StoreTCell(wMenu, SCANKEY_p, RSC(MENU_ITEM_PROCESSOR).CODE()	, skeyAttr);

  StoreTCell(wMenu, SCANKEY_k,	RSC(MENU_ITEM_KERNEL).CODE()	, gateAttr);
  StoreTCell(wMenu, SCANKEY_f, RSC(MENU_ITEM_FREQUENCY).CODE()	, skeyAttr);
  StoreTCell(wMenu, SCANKEY_m, RSC(MENU_ITEM_TOPOLOGY).CODE()	, skeyAttr);

  StoreTCell(wMenu, SCANKEY_HASH,RSC(MENU_ITEM_HOTPLUG).CODE()	,skeyAttr);
  StoreTCell(wMenu, SCANKEY_i, RSC(MENU_ITEM_INST_CYCLES).CODE(), skeyAttr);
  StoreTCell(wMenu, SCANKEY_e, RSC(MENU_ITEM_FEATURES).CODE()	, skeyAttr);

  StoreTCell(wMenu, SCANKEY_F3, RSC(MENU_ITEM_TOOLS).CODE()	, fkeyAttr);
  StoreTCell(wMenu, SCANKEY_c, RSC(MENU_ITEM_CORE_CYCLES).CODE(), skeyAttr);
  StoreTCell(wMenu, SCANKEY_SHIFT_i,RSC(MENU_ITEM_ISA_EXT).CODE(), skeyAttr);

  StoreTCell(wMenu, SCANKEY_a,	RSC(MENU_ITEM_ABOUT).CODE()	, skeyAttr);
  StoreTCell(wMenu, SCANKEY_l, RSC(MENU_ITEM_IDLE_STATES).CODE(), skeyAttr);
  StoreTCell(wMenu, SCANKEY_t,	RSC(MENU_ITEM_TECH).CODE()	, skeyAttr);

  StoreTCell(wMenu, SCANKEY_h,	RSC(MENU_ITEM_HELP).CODE()	, skeyAttr);
  StoreTCell(wMenu, SCANKEY_g,	RSC(MENU_ITEM_PKG_CYCLES).CODE(), skeyAttr);
  StoreTCell(wMenu, SCANKEY_o,	RSC(MENU_ITEM_PERF_MON).CODE()	, skeyAttr);

  StoreTCell(wMenu, SCANKEY_F1, RSC(MENU_ITEM_KEYS).CODE()	, fkeyAttr);
  StoreTCell(wMenu, SCANKEY_x,	RSC(MENU_ITEM_TASKS_MON).CODE() , gateAttr);
  StoreTCell(wMenu, SCANKEY_w,	RSC(MENU_ITEM_POW_THERM).CODE() , skeyAttr);

  StoreTCell(wMenu, SCANKEY_SHIFT_l, RSC(MENU_ITEM_LANG).CODE() , skeyAttr);
  StoreTCell(wMenu, SCANKEY_q,	RSC(MENU_ITEM_SYS_INTER).CODE() , skeyAttr);
  StoreTCell(wMenu, SCANKEY_u,	RSC(MENU_ITEM_CPUID).CODE()	, skeyAttr);

  StoreTCell(wMenu, SCANKEY_F4, RSC(MENU_ITEM_QUIT).CODE()	, fkeyAttr);
  StoreTCell(wMenu, SCANKEY_SHIFT_v,RSC(MENU_ITEM_POW_VOLT).CODE(),skeyAttr);
  StoreTCell(wMenu, SCANKEY_SHIFT_r,RSC(MENU_ITEM_SYS_REGS).CODE(),skeyAttr);

  StoreTCell(wMenu, SCANKEY_VOID,			""	, voidAttr);
  StoreTCell(wMenu, SCANKEY_SHIFT_t,RSC(MENU_ITEM_SLICE_CTRS).CODE(),skeyAttr);
  StoreTCell(wMenu, SCANKEY_SHIFT_m,RSC(MENU_ITEM_MEM_CTRL).CODE(),ctrlAttr);

  StoreTCell(wMenu, SCANKEY_VOID,	""			, voidAttr);
  StoreTCell(wMenu, SCANKEY_VOID,	""			, voidAttr);
  StoreTCell(wMenu, SCANKEY_VOID,	""			, voidAttr);

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
	Window *wSet = CreateWindow(wLayer, id, 1, 14,
				8, (TOP_HEADER_ROW + 14 + 3 < draw.Size.height)?
					TOP_HEADER_ROW + 3 : 1);
    if (wSet != NULL) {
	ATTRIBUTE *attrib[2] = {
		RSC(CREATE_SETTINGS_COND0).ATTR(),
		RSC(CREATE_SETTINGS_COND1).ATTR()
	};
	size_t subLen = strlen(Shm->ShmName);
	char subStr[16];

	StoreTCell(wSet, SCANKEY_NULL,   RSC(CREATE_SETTINGS_COND0).CODE(),
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,   RSC(SETTINGS_DAEMON).CODE(),
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, OPS_INTERVAL,   RSC(SETTINGS_INTERVAL).CODE(),
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,   " Sys. Tick(ms)                  ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,   " Poll Wait(ms)                  ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,   " Ring Wait(ms)                  ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,   " Child Wait(ms)                 ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,   " Slice Wait(ms)                 ",
							MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, OPS_AUTOCLOCK,  RSC(SETTINGS_AUTO_CLOCK).CODE(),
			attrib[((Shm->Registration.AutoClock & 0b10) != 0)]);

	StoreTCell(wSet,OPS_EXPERIMENTAL,RSC(SETTINGS_EXPERIMENTAL).CODE(),
				attrib[Shm->Registration.Experimental != 0]);

	StoreTCell(wSet, SCANKEY_NULL,   RSC(SETTINGS_CPU_HOTPLUG).CODE(),
				attrib[!(Shm->Registration.hotplug < 0)]);

	StoreTCell(wSet, SCANKEY_NULL,   RSC(SETTINGS_PCI_ENABLED).CODE(),
					attrib[(Shm->Registration.pci == 1)]);

	StoreTCell(wSet, OPS_INTERRUPTS, RSC(SETTINGS_NMI_REGISTERED).CODE(),
						attrib[Shm->Registration.nmi]);

	StoreTCell(wSet, SCANKEY_NULL,   RSC(CREATE_SETTINGS_COND0).CODE(),
							MAKE_PRINT_UNFOCUS);

	memcpy(&TCellAt(wSet, 0, 1).item[31 - subLen], Shm->ShmName, subLen);

	subLen = sprintf(subStr, "%.*s<%4u>",
				9, hSpace, Shm->Sleep.Interval);;
	memcpy(&TCellAt(wSet, 0, 2).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "%14u ",
				Shm->Sleep.Interval * Shm->SysGate.tickReset);
	memcpy(&TCellAt(wSet, 0, 3).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "%14ld ",
				Shm->Sleep.pollingWait.tv_nsec / 1000000L);
	memcpy(&TCellAt(wSet, 0, 4).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "%14ld ",
				Shm->Sleep.ringWaiting.tv_nsec / 1000000L);
	memcpy(&TCellAt(wSet, 0, 5).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "%14ld ",
				Shm->Sleep.childWaiting.tv_nsec / 1000000L);
	memcpy(&TCellAt(wSet, 0, 6).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "%14ld ",
				Shm->Sleep.sliceWaiting.tv_nsec / 1000000L);
	memcpy(&TCellAt(wSet, 0, 7).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "<%3s>",
			enabled((Shm->Registration.AutoClock & 0b10) != 0));
	memcpy(&TCellAt(wSet, 0, 8).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "<%3s>",
				enabled((Shm->Registration.Experimental != 0)));
	memcpy(&TCellAt(wSet, 0, 9).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "[%3s]",
				enabled(!(Shm->Registration.hotplug < 0)));
	memcpy(&TCellAt(wSet, 0,10).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "[%3s]",
				enabled((Shm->Registration.pci == 1)));
	memcpy(&TCellAt(wSet, 0,11).item[31 - subLen], subStr, subLen);

	subLen = sprintf(subStr, "<%3s>",
				enabled(Shm->Registration.nmi));
	memcpy(&TCellAt(wSet, 0,12).item[31 - subLen], subStr, subLen);

	StoreWindow(wSet, .title, (char*) RSC(SETTINGS_TITLE).CODE());

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
				(TOP_HEADER_ROW + 19 + 1 < draw.Size.height) ?
					TOP_HEADER_ROW + 1 : 1);
    if (wHelp != NULL) {
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [F2]             ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MENU).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Escape]         ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_CLOSE_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Shift]+[Tab]    ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_PREV_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Tab]            ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_NEXT_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       [A|Z]      ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [W|Q]  [S]  [D]  ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MOVE_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       [Up]       ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Left]    [Right]",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MOVE_SELECT).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "      [Down]      ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [End]            ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_LAST_CELL).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Home]           ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_FIRST_CELL).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Enter]          ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_TRIGGER_SELECT).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Page-Up]        ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_PREV_PAGE).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Page-Dw]        ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_NEXT_PAGE).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Minus]          ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_SCROLL_DOWN).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Plus]           ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_SCROLL_UP).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);

	StoreWindow(wHelp, .title, (char*) RSC(HELP_TITLE).CODE());
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
    ATTRIBUTE *attrib[2] = {
	RSC(CREATE_ADV_HELP_COND0).ATTR(),
	RSC(CREATE_ADV_HELP_COND1).ATTR()
    };
    struct ADV_HELP_ST {
	short theme;
	ASCII *item;
	SCANKEY quick;
    } advHelp[] = {
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_ITEM_1).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_2).CODE(),	{SCANKEY_PERCENT}},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_ITEM_3).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_4).CODE(),	{SCANKEY_b}	},
	{1, RSC(ADV_HELP_ITEM_5).CODE(),	{SCANKEY_n}	},
	{1, RSC(ADV_HELP_ITEM_6).CODE(),	{SCANKEY_r}	},
	{1, RSC(ADV_HELP_ITEM_7).CODE(),	{SCANKEY_v}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_ITEM_8).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_9).CODE(),	{SCANKEY_DOT}	},
	{1, RSC(ADV_HELP_ITEM_10).CODE(),	{SCANKEY_OPEN_BRACE}},
	{1, RSC(ADV_HELP_ITEM_11).CODE(),	{SCANKEY_CLOSE_BRACE}},
	{1, RSC(ADV_HELP_ITEM_12).CODE(),	{SCANKEY_F10}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_13).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_14).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
    };
	const size_t nmemb = sizeof(advHelp) / sizeof(struct ADV_HELP_ST);
	Window *wHelp = CreateWindow(wLayer, id, 1, nmemb, 41,
				(TOP_HEADER_ROW + nmemb + 1 < draw.Size.height)?
					TOP_HEADER_ROW + 1 : 1);
    if (wHelp != NULL) {
	unsigned int idx;
	for (idx = 0; idx < nmemb; idx++)
		StoreTCell(	wHelp,
				advHelp[idx].quick.key,
				advHelp[idx].item,
				attrib[advHelp[idx].theme] );

	StoreWindow(wHelp, .title, (char*) RSC(ADV_HELP_TITLE).CODE());

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
	"   ""         (C)2015-2019 CYRIL INGENIERIE        ""   "
      };
	size_t	c = sizeof(C) / sizeof(C[0]),
		f = sizeof(F) / sizeof(F[0]),
		l = strlen(C[0]), v = strlen(COREFREQ_VERSION);

	CUINT	cHeight = c + f,
		oCol = (draw.Size.width-l)/2,
		oRow = TOP_HEADER_ROW + 4;

	if (cHeight >= (draw.Size.height - 1)) {
		cHeight = draw.Size.height - 2;
	}
	if (oRow + cHeight >= draw.Size.height) {
		oRow = abs(draw.Size.height - (1 + cHeight));
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

/* >>> GLOBALS >>> */
int SysInfoCellPadding;
/* <<< GLOBALS <<< */

void AddSysInfoCell(CELL_ARGS)
{
	SysInfoCellPadding++;
	StoreTCell(win, key, item, attrib);
}

Window *CreateSysInfo(unsigned long long id)
{
	void (*SysInfoFunc)(Window*, CUINT, CELL_FUNC OutFunc) = NULL;
	ASCII *title = NULL;
	CoordSize matrixSize = {
		.wth = 1,
		.hth = CUMIN(18, (draw.Size.height - TOP_HEADER_ROW - 2))
	};
	Coordinate winOrigin = {.col = 3, .row = TOP_HEADER_ROW + 2};
	CUINT winWidth = 74;

	switch (id) {
	case SCANKEY_p:
		{
		if (TOP_HEADER_ROW + 2 + matrixSize.hth >= draw.Size.height)
			winOrigin.row = TOP_HEADER_ROW + 1;
		winOrigin.col = 2;
		winWidth = 76;
		SysInfoFunc = SysInfoProc;
		title = RSC(PROCESSOR_TITLE).CODE();
		}
		break;
	case SCANKEY_e:
		{
		if (TOP_HEADER_ROW + 2 + matrixSize.hth >= draw.Size.height)
			winOrigin.row = TOP_HEADER_ROW + 1;
		winOrigin.col = 4;
		winWidth = 72;
		SysInfoFunc = SysInfoFeatures;
		title = RSC(FEATURES_TITLE).CODE();
		}
		break;
	case SCANKEY_t:
		{
		if (TOP_HEADER_ROW + 11 + 6 < draw.Size.height) {
			winOrigin.col = 23;
			matrixSize.hth = 7;
			winOrigin.row = TOP_HEADER_ROW + 11;
		} else {
			winOrigin.col = 18;
			matrixSize.hth = CUMIN((draw.Size.height - 2), 7);
			winOrigin.row = 1;
		}
		winWidth = 50;
		SysInfoFunc = SysInfoTech;
		title = RSC(TECHNOLOGIES_TITLE).CODE();
		}
		break;
	case SCANKEY_o:
		{
		if (TOP_HEADER_ROW + 2 + matrixSize.hth >= draw.Size.height)
			winOrigin.row = TOP_HEADER_ROW + 1;
		SysInfoFunc = SysInfoPerfMon;
		title = RSC(PERF_MON_TITLE).CODE();
		}
		break;
	case SCANKEY_w:
		{
		winOrigin.col = 25;
		if (TOP_HEADER_ROW + 2 + 14 < draw.Size.height) {
			matrixSize.hth = 14;
			winOrigin.row = TOP_HEADER_ROW + 2;
		} else {
			matrixSize.hth = CUMIN((draw.Size.height - 2), 14);
			winOrigin.row = 1;
		}
		winWidth = 50;
		SysInfoFunc = SysInfoPwrThermal;
		title = RSC(POWER_THERMAL_TITLE).CODE();
		}
		break;
	case SCANKEY_u:
		{
		if (TOP_HEADER_ROW + 2 + matrixSize.hth >= draw.Size.height)
			winOrigin.row = TOP_HEADER_ROW + 1;
		winWidth = 74;
		SysInfoFunc = SysInfoCPUID;
		title = RSC(CPUID_TITLE).CODE();
		}
		break;
	case SCANKEY_k:
		{
		winOrigin.col = 2;
		winWidth = 76;
		if (TOP_HEADER_ROW + 8 + 11 < draw.Size.height) {
			matrixSize.hth = 11;
			winOrigin.row = TOP_HEADER_ROW + 8;
		} else {
			matrixSize.hth = CUMIN((draw.Size.height - 2), 11);
			winOrigin.row = 1;
		}
		SysInfoFunc = SysInfoKernel;
		title = RSC(KERNEL_TITLE).CODE();
		}
		break;
	}

	SysInfoCellPadding = 0;

	Window *wSysInfo = CreateWindow(wLayer, id,
					matrixSize.wth, matrixSize.hth,
					winOrigin.col, winOrigin.row);

	if (wSysInfo != NULL) {
		if (SysInfoFunc != NULL)
			SysInfoFunc(wSysInfo, winWidth, AddSysInfoCell);
		/* Pad with blank rows.					*/
		while (SysInfoCellPadding < matrixSize.hth) {
			SysInfoCellPadding++;
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
		if (title != NULL) {
			StoreWindow(wSysInfo, .title, (char*) title);
		}
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

void AddCell(CELL_ARGS)
{
	StoreTCell(win, key, item, attrib);
}

Window *CreateTopology(unsigned long long id)
{
	Window *wTopology = CreateWindow(wLayer, id,
					6, CUMIN(2 + Shm->Proc.CPU.Count,
					  (draw.Size.height-TOP_HEADER_ROW-5)),
					1, TOP_HEADER_ROW + 3);
		wTopology->matrix.select.row = 2;

	if (wTopology != NULL) {
		Topology(wTopology, AddCell);

		StoreWindow(wTopology,.title,(char*)RSC(TOPOLOGY_TITLE).CODE());

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
				4, CUMIN(8,(draw.Size.height-TOP_HEADER_ROW-3)),
				6, TOP_HEADER_ROW + 2);

	if (wISA != NULL) {
		SysInfoISA(wISA, AddCell);

		StoreWindow(wISA,	.title, (char*) RSC(ISA_TITLE).CODE());

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
				(draw.Size.height - TOP_HEADER_ROW - 3)),
			6, TOP_HEADER_ROW + 2);

	if (wSR != NULL) {
		SystemRegisters(wSR, AddCell);

		StoreWindow(wSR, .title, (char*) RSC(SYS_REGS_TITLE).CODE());

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
		rows += 7;
	    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
		rows++;
	    for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
		rows++;
	}
	if (rows > 0) {
	    Window *wIMC = CreateWindow(wLayer, id,
					14, CUMIN((rows + 3),
					  (draw.Size.height-TOP_HEADER_ROW-3)),
					1, TOP_HEADER_ROW + 2);
		wIMC->matrix.select.row = 5;

	    if (wIMC != NULL) {
		MemoryController(wIMC, AddCell);

		StoreWindow(wIMC, .title, (char*) RSC(MEM_CTRL_TITLE).CODE());

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
				33, TOP_HEADER_ROW + draw.Area.MaxRows + 2);
	if (wSortBy != NULL) {
		StoreTCell(wSortBy,SORTBY_STATE,RSC(TASKS_SORTBY_STATE).CODE(),
						MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_RTIME,RSC(TASKS_SORTBY_RTIME).CODE(),
						MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_UTIME,RSC(TASKS_SORTBY_UTIME).CODE(),
						MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_STIME,RSC(TASKS_SORTBY_STIME).CODE(),
						MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_PID,  RSC(TASKS_SORTBY_PID).CODE(),
						MAKE_PRINT_DROP);
		StoreTCell(wSortBy,SORTBY_COMM, RSC(TASKS_SORTBY_COMM).CODE(),
						MAKE_PRINT_DROP);

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

int SortTaskListByForest(const void *p1, const void *p2)
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

Window *CreateTracking(unsigned long long id)
{
    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
	size_t tc = Shm->SysGate.taskCount;
	if (tc > 0) {
		const CUINT margin = 12;	/*	@ "Freq(MHz)"	*/
		int padding = draw.Size.width - margin - TASK_COMM_LEN - 7;

		Window *wTrack = CreateWindow(wLayer, id,
				1, TOP_HEADER_ROW + draw.Area.MaxRows * 2,
				margin, TOP_HEADER_ROW);
	    if (wTrack != NULL) {
		char *item = malloc(MAX_WIDTH);
		TASK_MCB *trackList = malloc(tc * sizeof(TASK_MCB));

		memcpy(trackList, Shm->SysGate.taskList, tc * sizeof(TASK_MCB));
		qsort(trackList, tc, sizeof(TASK_MCB), SortTaskListByForest);

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

		StoreWindow(wTrack,	.Print, 	ForEachCellPrint_Drop);
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
	Window *wCPU = CreateWindow(	wLayer, id, 2, draw.Area.MaxRows,
					LOAD_LEAD + 1, TOP_HEADER_ROW + 1);
    if (wCPU != NULL) {
	ASCII *item = malloc(12 + 1);
	unsigned int cpu;
	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	    if (BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		sprintf((char*) item, " %02u   Off   ", cpu);
		StoreTCell(wCPU, SCANKEY_NULL, item, MakeAttr(BLUE,0,BLACK,1));

		StoreTCell(wCPU, CPU_ONLINE | cpu ,
					RSC(CREATE_HOTPLUG_CPU_ENABLE).CODE(),
					RSC(CREATE_HOTPLUG_CPU_ENABLE).ATTR());
	    } else {
		sprintf((char*) item, " %02u    On   ", cpu);
		StoreTCell(wCPU, SCANKEY_NULL, item, MakeAttr(WHITE,0,BLACK,0));

		StoreTCell(wCPU, CPU_OFFLINE | cpu,
					RSC(CREATE_HOTPLUG_CPU_DISABLE).CODE(),
					RSC(CREATE_HOTPLUG_CPU_DISABLE).ATTR());
	    }
	}
	wCPU->matrix.select.col = 1;

	StoreWindow(wCPU,	.title, 	" CPU ");
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

	free(item);
    }
	return(wCPU);
}

typedef void (*TITLE_CALLBACK)(unsigned int, char *);

Window *CreateCoreClock(unsigned long long id,
			unsigned int boostBase,
			unsigned long long boxKey,
			TITLE_CALLBACK TitleCallback,
			CUINT oCol)
{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
				.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core] \
					.Toggle];

	ATTRIBUTE *attrib[3] = {
		RSC(CREATE_CORE_CLOCK_COND0).ATTR(),
		RSC(CREATE_CORE_CLOCK_COND1).ATTR(),
		RSC(CREATE_CORE_CLOCK_COND2).ATTR()
	};
	CLOCK_ARG clockMod  = {.sllong = id};
	unsigned int nc = clockMod.NC & CLOCKMOD_RATIO_MASK, multiplier;
	signed int offset,
	lowestOperating = abs((int)Shm->Proc.Boost[boostBase - nc]
			- (signed) Shm->Proc.Boost[BOOST(MIN)]),
	highestOperating = MAXCLOCK_TO_RATIO(CFlop->Clock.Hz)
			 - Shm->Proc.Boost[boostBase - nc],
	medianColdZone =( Shm->Proc.Boost[BOOST(MIN)]
			+ Shm->Proc.Features.Factory.Ratio ) >> 1,
	startingHotZone = Shm->Proc.Features.Factory.Ratio
			+ ( ( MAXCLOCK_TO_RATIO(CFlop->Clock.Hz)
			- Shm->Proc.Features.Factory.Ratio ) >> 1);
	CUINT hthMin, hthMax = 1 + lowestOperating + highestOperating, hthWin;
	CUINT oRow;

	if (TOP_HEADER_ROW + TOP_FOOTER_ROW + 8 < draw.Size.height) {
		hthMin = draw.Size.height - 	( TOP_HEADER_ROW
						+ TOP_FOOTER_ROW
						+ TOP_SEPARATOR);
		oRow = TOP_HEADER_ROW + TOP_FOOTER_ROW;
	} else {
		hthMin = draw.Size.height - 2;
		oRow = 1;
	}
	hthWin = CUMIN(hthMin, hthMax);

	Window *wCK = CreateWindow(wLayer, id, 1, hthWin, oCol, oRow);

    if (wCK != NULL) {
	ASCII *item = malloc(32);
	for (offset = -lowestOperating; offset <= highestOperating; offset++)
	{
		clockMod.NC = nc | boxKey;
		clockMod.Offset = offset;
		multiplier = Shm->Proc.Boost[boostBase - nc] + offset;

		sprintf((char*) item, " %7.2f MHz   [%4d ]  %+3d ",
			(double)(multiplier * CFlop->Clock.Hz) / 1000000.0,
			multiplier, offset);

		StoreTCell(wCK, clockMod.sllong, item,
			attrib[multiplier < medianColdZone ?
				1 : multiplier > startingHotZone ?
					2 : 0]);
	}

	TitleCallback(nc, (char*) item);
	StoreWindow(wCK, .title, (char*) item);

	if (lowestOperating >= hthWin) {
		wCK->matrix.scroll.vert = hthMax
					- hthWin * (1 + (highestOperating
							/ hthWin));
		wCK->matrix.select.row  = lowestOperating
					- wCK->matrix.scroll.vert;
	} else {
		wCK->matrix.select.row  = lowestOperating;
	}
	StoreWindow(wCK,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wCK,	.key.Down,	MotionDown_Win);
	StoreWindow(wCK,	.key.Up,	MotionUp_Win);
	StoreWindow(wCK,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wCK,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wCK,	.key.Home,	MotionTop_Win);
	StoreWindow(wCK,	.key.End,	MotionBottom_Win);

	StoreWindow(wCK,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wCK,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wCK,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wCK,	.key.WinUp,	MotionOriginUp_Win);

	free(item);
    }
	return(wCK);
}

void TitleForTurboClock(unsigned int nc, char *title)
{
	sprintf(title, " Turbo Clock %1dC ", nc);
}

Window *CreateTurboClock(unsigned long long id)
{
	return(CreateCoreClock(id, BOOST(SIZE), BOXKEY_TURBO_CLOCK,
					TitleForTurboClock, 34));
}

void TitleForRatioClock(unsigned int nc, char *title)
{
	sprintf(title, " %s Clock Ratio ", nc == 1 ? "Max" : "Min");
}

Window *CreateRatioClock(unsigned long long id)
{
	return(CreateCoreClock(id, BOOST(ACT), BOXKEY_RATIO_CLOCK,
					TitleForRatioClock, 38));
}

Window *CreateUncoreClock(unsigned long long id)
{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
				.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core] \
					.Toggle];

	ATTRIBUTE *attrib[2] = {
		RSC(CREATE_UNCORE_CLOCK_COND0).ATTR(),
		RSC(CREATE_UNCORE_CLOCK_COND1).ATTR()
	};
	CLOCK_ARG clockMod  = {.sllong = id};
	unsigned int nc = clockMod.NC & CLOCKMOD_RATIO_MASK, multiplier;
	signed int offset,
	lowestOperating = abs((int)Shm->Uncore.Boost[UNCORE_BOOST(SIZE) - nc]
			- (int) Shm->Proc.Boost[BOOST(MIN)]),
	highestOperating = MAXCLOCK_TO_RATIO(CFlop->Clock.Hz)
				- Shm->Uncore.Boost[UNCORE_BOOST(SIZE) - nc],
	startingHotZone = Shm->Proc.Features.Factory.Ratio
			+ ( ( MAXCLOCK_TO_RATIO(CFlop->Clock.Hz)
			- Shm->Proc.Features.Factory.Ratio ) >> 1);
	CUINT hthMin, hthMax = 1 + lowestOperating + highestOperating, hthWin;
	CUINT oRow;

	if (TOP_HEADER_ROW + TOP_FOOTER_ROW + 8 < draw.Size.height) {
		hthMin = draw.Size.height -	( TOP_HEADER_ROW
						+ TOP_FOOTER_ROW
						+ TOP_SEPARATOR );
		oRow = TOP_HEADER_ROW + TOP_FOOTER_ROW;
	} else {
		hthMin = draw.Size.height - 2;
		oRow = 1;
	}
	hthWin = CUMIN(hthMin, hthMax);

	Window *wUC = CreateWindow(wLayer, id, 1, hthWin, 42, oRow);

    if (wUC != NULL) {
	ASCII *item = malloc(32);
	for (offset = -lowestOperating; offset <= highestOperating; offset++)
	{
		clockMod.NC = nc | BOXKEY_UNCORE_CLOCK;
		clockMod.Offset = offset;
		multiplier = Shm->Uncore.Boost[UNCORE_BOOST(SIZE) - nc];
		multiplier += offset;

		sprintf((char*) item, " %7.2f MHz   [%4d ]  %+3d ",
			(double)(multiplier * CFlop->Clock.Hz) / 1000000.0,
			multiplier, offset);

		StoreTCell(wUC, clockMod.sllong, item,
			attrib[multiplier > startingHotZone ? 1 : 0]);
	}
	sprintf((char*) item, " %s Clock Uncore ", nc == 1 ? "Max" : "Min");
	StoreWindow(wUC, .title, (char*) item);

	if (lowestOperating >= hthWin) {
		wUC->matrix.scroll.vert = hthMax
					- hthWin * (1 + (highestOperating
							/ hthWin));
		wUC->matrix.select.row  = lowestOperating
					- wUC->matrix.scroll.vert;
	} else {
		wUC->matrix.select.row  = lowestOperating;
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

	free(item);
    }
	return(wUC);
}

Window *_CreateBox(	unsigned long long id,
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
	ATTRIBUTE attrib = va_arg(ap, ATTRIBUTE);
	unsigned long long aKey = va_arg(ap, unsigned long long);
	do {
	    if (item != NULL) {
		cnt = (pBox == NULL) ? 1: pBox->cnt + 1;
		if ((pBox = realloc(pBox,
					sizeof(struct PBOX)
					+ cnt * sizeof(struct SBOX))) != NULL)
		{
			size_t len = KMIN(strlen((char *) item), MIN_WIDTH);
			memcpy((char*)pBox->btn[cnt - 1].item,(char *)item,len);
			pBox->btn[cnt - 1].item[len] = '\0';
			pBox->btn[cnt - 1].attr = attrib;
			pBox->btn[cnt - 1].key = aKey;
			pBox->cnt = cnt;
		}
		item = va_arg(ap, ASCII*);
		attrib = va_arg(ap, ATTRIBUTE);
		aKey = va_arg(ap, unsigned long long);
	    }
	} while (item != NULL) ;
	va_end(ap);

	Window *wBox = NULL;
    if (pBox != NULL) {
	CUINT cHeight = pBox->cnt, oRow = origin.row;
	if (oRow + cHeight >= draw.Size.height) {
		cHeight = CUMIN(cHeight, (draw.Size.height - 2));
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

void TrapScreenSize(int caught)
{
    if (caught == SIGWINCH) {
	SCREEN_SIZE currentSize = GetScreenSize();

	if (currentSize.height != draw.Size.height) {
		if (currentSize.height > MAX_HEIGHT)
			draw.Size.height = MAX_HEIGHT;
		else
			draw.Size.height = currentSize.height;

	    switch (draw.Disposal) {
	    case D_MAINVIEW:
		switch (draw.View) {
		case V_FREQ:
		case V_INST:
		case V_CYCLES:
		case V_CSTATES:
		case V_TASKS:
		case V_INTR:
		case V_SLICE:
	/*10*/		draw.Area.MinHeight = 2 + TOP_HEADER_ROW
						+ TOP_SEPARATOR
						+ TOP_FOOTER_ROW;
			break;
		case V_VOLTAGE:
	/*16*/		draw.Area.MinHeight = 8 + TOP_HEADER_ROW
						+ TOP_SEPARATOR
						+ TOP_FOOTER_ROW;
			break;
		case V_PACKAGE:
	/*24*/		draw.Area.MinHeight =16 + TOP_HEADER_ROW
						+ TOP_SEPARATOR
						+ TOP_FOOTER_ROW;
			break;
		}
		break;
	    default:
	/*11*/	draw.Area.MinHeight = LEADING_TOP
					+ 2 * (MARGIN_HEIGHT + INTER_HEIGHT);
		break;
	    }

		draw.Flag.clear  = 1;
		draw.Flag.height = !(draw.Size.height < draw.Area.MinHeight);
	}
	if (currentSize.width != draw.Size.width) {
		if (currentSize.width > MAX_WIDTH)
			draw.Size.width = MAX_WIDTH;
		else
			draw.Size.width = currentSize.width;

		draw.Flag.clear = 1;
		draw.Flag.width = !(draw.Size.width < MIN_WIDTH);
	}
	draw.Area.MaxRows = CUMIN(Shm->Proc.CPU.Count, ( draw.Size.height
						- TOP_HEADER_ROW
						- TOP_SEPARATOR
						- TOP_FOOTER_ROW ) / 2);

	draw.cpuScroll = 0;
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
			RSC(BOX_DISABLE_COND0).CODE(),
			RSC(BOX_DISABLE_COND1).CODE()
		},{
			RSC(BOX_ENABLE_COND0).CODE(),
			RSC(BOX_ENABLE_COND1).CODE()
		}
	},
	*blankStr =	RSC(BOX_BLANK_DESC).CODE(),
	*descStr[] = {
			RSC(BOX_EIST_DESC).CODE(),
			RSC(BOX_C1E_DESC).CODE(),
			RSC(BOX_TURBO_DESC).CODE(),
			RSC(BOX_C1A_DESC).CODE(),
			RSC(BOX_C3A_DESC).CODE(),
			RSC(BOX_C1U_DESC).CODE(),
			RSC(BOX_C3U_DESC).CODE(),
			RSC(BOX_IO_MWAIT_DESC).CODE(),
			RSC(BOX_ODCM_DESC).CODE(),
			RSC(BOX_CC6_DESC).CODE(),
			RSC(BOX_PC6_DESC).CODE()
	};

    switch (scan->key) {
    case SCANKEY_DOWN:
	if (!IsDead(&winList))
		return(-1);
	/* Fallthrough */
    case SCANKEY_PLUS:
	if ((draw.Disposal == D_MAINVIEW)
	&&  (draw.cpuScroll < (Shm->Proc.CPU.Count - draw.Area.MaxRows))) {
		draw.cpuScroll++;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_UP:
	if (!IsDead(&winList))
		return(-1);
	/* Fallthrough */
    case SCANKEY_MINUS:
	if ((draw.Disposal == D_MAINVIEW) && (draw.cpuScroll > 0)) {
		draw.cpuScroll--;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_HOME:
    case SCANCON_HOME:
	if (!IsDead(&winList))
		return(-1);
	else if (draw.Disposal == D_MAINVIEW) {
		draw.cpuScroll = 0;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_END:
    case SCANCON_END:
	if (!IsDead(&winList))
		return(-1);
	else if (draw.Disposal == D_MAINVIEW) {
		draw.cpuScroll = Shm->Proc.CPU.Count - draw.Area.MaxRows;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_PGDW:
	if (!IsDead(&winList))
		return(-1);
	else if (draw.Disposal == D_MAINVIEW) {
		CUINT offset = Shm->Proc.CPU.Count / 4;
		if((draw.cpuScroll + offset) < ( Shm->Proc.CPU.Count
						- draw.Area.MaxRows) )
		{
			draw.cpuScroll += offset;
			draw.Flag.layout = 1;
		}
	}
    break;
    case SCANKEY_PGUP:
	if (!IsDead(&winList))
		return(-1);
	else if (draw.Disposal == D_MAINVIEW) {
		CUINT offset = Shm->Proc.CPU.Count / 4;
		if (draw.cpuScroll >= offset) {
			draw.cpuScroll -= offset;
			draw.Flag.layout = 1;
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

	AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_INTERVAL_TITLE).CODE(),
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
    case OPS_AUTOCLOCK:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const int bON = ((Shm->Registration.AutoClock & 0b10) != 0);
		const Coordinate origin = {
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
			.row = TOP_HEADER_ROW + 4
		}, select = {
			.col = 0,
			.row = bON ? 2 : 1
		};

	AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_AUTO_CLOCK_TITLE).CODE(),
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][bON], stateAttr[bON] , OPS_AUTOCLOCK_ON,
		stateStr[0][!bON],stateAttr[!bON], OPS_AUTOCLOCK_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	} else
		SetHead(&winList, win);
    }
    break;
    case OPS_AUTOCLOCK_OFF:
    {
      if (!RING_FULL(Shm->Ring[0]))
       RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_AUTOCLOCK, COREFREQ_TOGGLE_OFF);
    }
    break;
    case OPS_AUTOCLOCK_ON:
    {
      if (!RING_FULL(Shm->Ring[0]))
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_AUTOCLOCK, COREFREQ_TOGGLE_ON);
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
				RSC(BOX_NOMINAL_MODE_COND0).CODE(),
				RSC(BOX_NOMINAL_MODE_COND1).CODE(),
			},{
				RSC(BOX_EXPERIMENT_MODE_COND0).CODE(),
				RSC(BOX_EXPERIMENT_MODE_COND1).CODE()
			}
		};
		const Coordinate origin = {
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = 3
		};

	AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_MODE_TITLE).CODE(),
				blankStr, blankAttr, SCANKEY_NULL,
			RSC(BOX_MODE_DESC).CODE(), descAttr, SCANKEY_NULL,
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
				RSC(BOX_INT_REGISTER_COND0).CODE(),
				RSC(BOX_INT_REGISTER_COND1).CODE(),
			},{
				RSC(BOX_INT_UNREGISTER_COND0).CODE(),
				RSC(BOX_INT_UNREGISTER_COND1).CODE()
			}
		};
		const Coordinate origin = {
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
			.row = TOP_HEADER_ROW + 5
		}, select = {
			.col = 0,
			.row = Shm->Registration.nmi == 0 ? 1 : 2
		};

	AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_INTERRUPT_TITLE).CODE(),
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
	if ((draw.View == V_FREQ) && (draw.Disposal == D_MAINVIEW)) {
		draw.Flag.avgOrPC = !draw.Flag.avgOrPC;
		draw.Flag.clear = 1;
	}
    break;
    case SCANKEY_DOT:
	if (draw.Disposal == D_MAINVIEW) {
		draw.Flag.clkOrLd = !draw.Flag.clkOrLd;
		draw.Flag.clear = 1;
	}
    break;
    case 0x000000000000007e:
    {
	draw.Disposal = D_ASCIITEST;
	draw.Size.height = 0;
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
	if ((draw.View == V_TASKS) && (draw.Disposal == D_MAINVIEW)) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateSortByField(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
    break;
    case SCANKEY_c:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_CYCLES;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_d:
    {
	draw.Disposal = D_DASHBOARD;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_f:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_FREQ;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_n:
	if ((draw.View == V_TASKS) && (draw.Disposal == D_MAINVIEW)) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateTracking(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
    break;
    case SCANKEY_g:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_PACKAGE;
	draw.Size.height = 0;
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
	draw.Disposal = D_MAINVIEW;
	draw.View = V_INST;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_l:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_CSTATES;
	draw.Size.height = 0;
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
    case SCANKEY_SHIFT_h:
    {
	ATTRIBUTE eventAttr[3] = {
		MakeAttr(WHITE,   0, BLACK, 0),
		MakeAttr(MAGENTA, 0, BLACK, 0),
		MakeAttr(YELLOW,  0, BLACK, 1)
	};
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col = 53,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = 0
		};

	Window *wBox = CreateBox(scan->key, origin, select,
		(char*) RSC(BOX_EVENT_TITLE).CODE(),
		RSC(BOX_EVENT_THERMAL_SENSOR).CODE(),
		eventAttr[(processorEvents & EVENT_THERM_SENSOR) ? 1 : 0],
		BOXKEY_CLR_THM_SENSOR,
		RSC(BOX_EVENT_PROCHOT_AGENT).CODE(),
		eventAttr[(processorEvents & EVENT_THERM_PROCHOT) ? 1 : 0],
		BOXKEY_CLR_THM_PROCHOT,
		RSC(BOX_EVENT_CRITICAL_TEMP).CODE(),
		eventAttr[(processorEvents & EVENT_THERM_CRIT) ? 1 : 0],
		BOXKEY_CLR_THM_CRIT,
		RSC(BOX_EVENT_THERMAL_THRESHOLD).CODE(),
		eventAttr[(processorEvents & EVENT_THERM_THOLD) ? 1 : 0],
		BOXKEY_CLR_THM_THOLD,
		RSC(BOX_EVENT_POWER_LIMITATION).CODE(),
		eventAttr[(processorEvents & EVENT_POWER_LIMIT) ? 2 : 0],
		BOXKEY_CLR_PWR_LIMIT,
		RSC(BOX_EVENT_CURRENT_LIMITATION).CODE(),
		eventAttr[(processorEvents & EVENT_CURRENT_LIMIT) ? 2 : 0],
		BOXKEY_CLR_CUR_LIMIT,
		RSC(BOX_EVENT_CROSS_DOMAIN_LIMIT).CODE(),
		eventAttr[(processorEvents & EVENT_CROSS_DOMAIN) ? 1 : 0],
		BOXKEY_CLR_X_DOMAIN);

		if (wBox != NULL) {
			AppendWindow(wBox, &winList);
		} else
			SetHead(&winList, win);
	} else
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
    case SCANKEY_SHIFT_l:
    {
	if (++locale >= LOC_CNT) {
		locale = LOC_EN;
	}
	draw.Flag.layout = 1;
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
	draw.Disposal = D_MAINVIEW;
	draw.View = V_INTR;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_SHIFT_v:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_VOLTAGE;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_SHIFT_t:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_SLICE;
	draw.Size.height = 0;
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
	if ((draw.View == V_TASKS) && (draw.Disposal == D_MAINVIEW)) {
		Shm->SysGate.reverseOrder = !Shm->SysGate.reverseOrder;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_v:
	if ((draw.View == V_TASKS) && (draw.Disposal == D_MAINVIEW)) {
		draw.Flag.taskVal = !draw.Flag.taskVal;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_x:
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		Shm->SysGate.trackTask = 0;
		draw.Disposal = D_MAINVIEW;
		draw.View = V_TASKS;
		draw.Size.height = 0;
		TrapScreenSize(SIGWINCH);
	}
    break;
    case SORTBY_STATE:
    {
	Shm->SysGate.sortByField = F_STATE;
	draw.Flag.layout = 1;
    }
    break;
    case SORTBY_RTIME:
    {
	Shm->SysGate.sortByField = F_RTIME;
	draw.Flag.layout = 1;
    }
    break;
    case SORTBY_UTIME:
    {
	Shm->SysGate.sortByField = F_UTIME;
	draw.Flag.layout = 1;
    }
    break;
    case SORTBY_STIME:
    {
	Shm->SysGate.sortByField = F_STIME;
	draw.Flag.layout = 1;
    }
    break;
    case SORTBY_PID:
    {
	Shm->SysGate.sortByField = F_PID;
	draw.Flag.layout = 1;
    }
    break;
    case SORTBY_COMM:
    {
	Shm->SysGate.sortByField = F_COMM;
	draw.Flag.layout = 1;
    }
    break;
    case BOXKEY_EIST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			stateAttr[Shm->Proc.Technology.Turbo] ,BOXKEY_TURBO_ON,
		stateStr[0][!Shm->Proc.Technology.Turbo],
			stateAttr[!Shm->Proc.Technology.Turbo],BOXKEY_TURBO_OFF,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
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
	{				/* Row indexes */
		const CSINT thisCST[] = {5, 4, 3, 2, -1, -1, 1, 0};
		const Coordinate origin = {
		.col = (draw.Size.width - (44 - 17)) / 2,
		.row = TOP_HEADER_ROW + 2
		}, select = {
		.col=0,
	.row=thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit] != -1 ?
		thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit] : 0
		};

		Window *wBox = CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_PACKAGE_STATE_LIMIT_TITLE).CODE(),
	(ASCII*)"             C7            ", stateAttr[0], BOXKEY_PKGCST_C7,
	(ASCII*)"             C6            ", stateAttr[0], BOXKEY_PKGCST_C6,
	(ASCII*)"             C3            ", stateAttr[0], BOXKEY_PKGCST_C3,
	(ASCII*)"             C2            ", stateAttr[0], BOXKEY_PKGCST_C2,
	(ASCII*)"             C1            ", stateAttr[0], BOXKEY_PKGCST_C1,
	(ASCII*)"             C0            ", stateAttr[0], BOXKEY_PKGCST_C0);
		if (wBox != NULL) {
			TCellAt(wBox, 0, select.row).attr[11] = 	\
			TCellAt(wBox, 0, select.row).attr[12] = 	\
			TCellAt(wBox, 0, select.row).attr[13] = 	\
			TCellAt(wBox, 0, select.row).attr[14] = 	\
			TCellAt(wBox, 0, select.row).attr[15] = 	\
			TCellAt(wBox, 0, select.row).attr[16] = stateAttr[1];
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
			.col=(draw.Size.width - strlen((char *) blankStr)) / 2,
			.row = TOP_HEADER_ROW + 9
		}, select = {
			.col = 0,
			.row = isIORedir ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_IO_MWAIT_TITLE).CODE(),
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
	{				/* Row indexes */
		const CSINT thisCST[] = {-1, -1, -1, 3, 2, -1, 1, 0};
		const Coordinate origin = {
			.col = (draw.Size.width - (44 - 17)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
	.row=thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude]!=-1 ?
		thisCST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude]:0
		};

		Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_MWAIT_MAX_STATE_TITLE).CODE(),
	(ASCII*)"             C7            ", stateAttr[0], BOXKEY_IORCST_C7,
	(ASCII*)"             C6            ", stateAttr[0], BOXKEY_IORCST_C6,
	(ASCII*)"             C4            ", stateAttr[0], BOXKEY_IORCST_C4,
	(ASCII*)"             C3            ", stateAttr[0], BOXKEY_IORCST_C3);

		if (wBox != NULL) {
			TCellAt(wBox, 0, select.row).attr[11] = 	\
			TCellAt(wBox, 0, select.row).attr[12] = 	\
			TCellAt(wBox, 0, select.row).attr[13] = 	\
			TCellAt(wBox, 0, select.row).attr[14] = 	\
			TCellAt(wBox, 0, select.row).attr[15] = 	\
			TCellAt(wBox, 0, select.row).attr[16] = stateAttr[1];
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
			.col =(draw.Size.width - strlen((char *) blankStr)) / 2,
			.row = TOP_HEADER_ROW + 6
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.ODCM ? 4 : 3
		};

		AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_ODCM_TITLE).CODE(),
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
		const CSINT maxCM = 7 << Shm->Cpu[Shm->Proc.Service.Core] \
					.PowerThermal.DutyCycle.Extended;
		const Coordinate origin = {
			.col = (draw.Size.width - (44 - 17)) / 2,
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
	  {
		wBox = CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_EXTENDED_DUTY_CYCLE_TITLE).CODE(),
		RSC(BOX_DUTY_CYCLE_RESERVED).CODE(), blankAttr,BOXKEY_ODCM_DC00,
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
	  } else {
		wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_DUTY_CYCLE_TITLE).CODE(),
		RSC(BOX_DUTY_CYCLE_RESERVED).CODE(), blankAttr,BOXKEY_ODCM_DC00,
	(ASCII*)"           12.50%          ", stateAttr[0], BOXKEY_ODCM_DC01,
	(ASCII*)"           25.00%          ", stateAttr[0], BOXKEY_ODCM_DC02,
	(ASCII*)"           37.50%          ", stateAttr[0], BOXKEY_ODCM_DC03,
	(ASCII*)"           50.00%          ", stateAttr[0], BOXKEY_ODCM_DC04,
	(ASCII*)"           62.50%          ", stateAttr[0], BOXKEY_ODCM_DC05,
	(ASCII*)"           75.00%          ", stateAttr[0], BOXKEY_ODCM_DC06,
	(ASCII*)"           87.50%          ", stateAttr[0], BOXKEY_ODCM_DC07);
	  }
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
    case BOXKEY_CLR_THM_SENSOR:
    case BOXKEY_CLR_THM_PROCHOT:
    case BOXKEY_CLR_THM_CRIT:
    case BOXKEY_CLR_THM_THOLD:
    case BOXKEY_CLR_PWR_LIMIT:
    case BOXKEY_CLR_CUR_LIMIT:
    case BOXKEY_CLR_X_DOMAIN:
    {
	const enum THERM_PWR_EVENTS events=(scan->key & CLEAR_EVENT_MASK) >> 4;
	if (!RING_FULL(Shm->Ring[0]))
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CLEAR_EVENTS, events);
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
    case BOXKEY_RATIO_CLOCK_MAX:
    case BOXKEY_RATIO_CLOCK_MIN:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateRatioClock(scan->key), &winList);
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
				(char*) RSC(BOX_TOOLS_TITLE).CODE(),
		RSC(BOX_TOOLS_STOP_BURN).CODE(), BITVAL(Shm->Proc.Sync, 31) ?
					MakeAttr(YELLOW,0,BLACK,0):blankAttr,
				BITVAL(Shm->Proc.Sync, 31) ?
					BOXKEY_TOOLS_MACHINE : SCANKEY_NULL,
	RSC(BOX_TOOLS_ATOMIC_BURN).CODE(), stateAttr[0],BOXKEY_TOOLS_ATOMIC,
	RSC(BOX_TOOLS_CRC32_BURN).CODE(), stateAttr[0], BOXKEY_TOOLS_CRC32,
	RSC(BOX_TOOLS_CONIC_BURN).CODE(), stateAttr[0], BOXKEY_TOOLS_CONIC,
	RSC(BOX_TOOLS_RANDOM_CPU).CODE(), stateAttr[0], BOXKEY_TOOLS_TURBO_RND,
    RSC(BOX_TOOLS_ROUND_ROBIN_CPU).CODE(), stateAttr[0],BOXKEY_TOOLS_TURBO_RR);

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
				(char*) RSC(BOX_CONIC_TITLE).CODE(),
		RSC(BOX_CONIC_ITEM_1).CODE(), stateAttr[0],BOXKEY_TOOLS_CONIC0,
		RSC(BOX_CONIC_ITEM_2).CODE(), stateAttr[0],BOXKEY_TOOLS_CONIC1,
		RSC(BOX_CONIC_ITEM_3).CODE(), stateAttr[0],BOXKEY_TOOLS_CONIC2,
		RSC(BOX_CONIC_ITEM_4).CODE(), stateAttr[0],BOXKEY_TOOLS_CONIC3,
		RSC(BOX_CONIC_ITEM_5).CODE(), stateAttr[0],BOXKEY_TOOLS_CONIC4,
		RSC(BOX_CONIC_ITEM_6).CODE(), stateAttr[0],BOXKEY_TOOLS_CONIC5);

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
	/* fallthrough */
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
		draw.Flag.layout = 1;
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
	if (clockMod.NC & BOXKEY_TURBO_CLOCK)
	{
	  clockMod.NC &= CLOCKMOD_RATIO_MASK;

	 if (!RING_FULL(Shm->Ring[0]))
	   RING_WRITE(Shm->Ring[0],COREFREQ_IOCTL_TURBO_CLOCK, clockMod.sllong);
	}
	else if (clockMod.NC & BOXKEY_RATIO_CLOCK)
	{
	  clockMod.NC &= CLOCKMOD_RATIO_MASK;

	 if (!RING_FULL(Shm->Ring[0]))
	   RING_WRITE(Shm->Ring[0],COREFREQ_IOCTL_RATIO_CLOCK, clockMod.sllong);
	}
	else if (clockMod.NC & BOXKEY_UNCORE_CLOCK)
	{
	  clockMod.NC &= CLOCKMOD_RATIO_MASK;

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

	memcpy(&LayerAt(layer, code, (draw.Size.width-35), row), &buffer[0], 6);
	memcpy(&LayerAt(layer, code, (draw.Size.width-22), row), &buffer[6], 9);
	memcpy(&LayerAt(layer, code, (draw.Size.width-12), row), &buffer[15],9);
}

void Layout_Header(Layer *layer, CUINT row)
{
	size_t len;
	struct FLIP_FLOP *CFlop = \
	    &Shm->Cpu[Shm->Proc.Top].FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];

	/* Reset the Top Frequency					*/
	if (!draw.Flag.clkOrLd) {
	      Clock2LCD(layer,0,row,CFlop->Relative.Freq,CFlop->Relative.Ratio);
	} else {
		double percent = 100.f * Shm->Proc.Avg.C0;

		Load2LCD(layer, 0, row, percent);
	}
	LayerDeclare(LAYOUT_HEADER_PROC, 12, 12, row, hProc0);
	LayerDeclare(LAYOUT_HEADER_CPU, 11,(draw.Size.width - 11), row, hProc1);

	row++;

	LayerDeclare(LAYOUT_HEADER_ARCH, 15, 12, row, hArch0);
	LayerDeclare(LAYOUT_HEADER_CACHE_L1,30,(draw.Size.width-30),row,hArch1);

	row++;

	LayerDeclare(LAYOUT_HEADER_BCLK, 28, 12, row, hBClk0);
	LayerDeclare(LAYOUT_HEADER_CACHES,21,(draw.Size.width-21), row, hArch2);

	row++;

	sprintf(buffer,"%2u" "%-2u", Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

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
	/* RED DOT */
	hProc0.code[0] = BITVAL(Shm->Proc.Sync, 31) ? '.' : 0x20;

	LayerCopyAt(	layer, hProc0.origin.col, hProc0.origin.row,
			hProc0.length, hProc0.attr, hProc0.code);

	LayerFillAt(layer,(hProc0.origin.col + hProc0.length),hProc0.origin.row,
			len, Shm->Proc.Brand,
			MakeAttr(CYAN, 0, BLACK, 1));

	if ((hProc1.origin.col - len) > 0) {
		LayerFillAt(layer, (hProc0.origin.col + hProc0.length + len),
				hProc0.origin.row,
				(hProc1.origin.col - len), hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}
	LayerCopyAt(	layer, hProc1.origin.col, hProc1.origin.row,
			hProc1.length, hProc1.attr, hProc1.code);

	len = strlen(Shm->Proc.Architecture);

	LayerCopyAt(	layer, hArch0.origin.col, hArch0.origin.row,
			hArch0.length, hArch0.attr, hArch0.code);

	LayerFillAt(layer,(hArch0.origin.col + hArch0.length),hArch0.origin.row,
			len, Shm->Proc.Architecture,
			MakeAttr(CYAN, 0, BLACK, 1));

	if ((hArch1.origin.col - len) > 0) {
		LayerFillAt(layer, (hArch0.origin.col + hArch0.length + len),
				hArch0.origin.row,
				(hArch1.origin.col - len), hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}
	LayerCopyAt(	layer, hArch1.origin.col, hArch1.origin.row,
			hArch1.length, hArch1.attr, hArch1.code);

	LayerCopyAt(	layer, hBClk0.origin.col, hBClk0.origin.row,
			hBClk0.length, hBClk0.attr, hBClk0.code);

	LayerFillAt(layer,(hBClk0.origin.col + hBClk0.length),hBClk0.origin.row,
			(hArch2.origin.col - hBClk0.origin.col + hBClk0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(	layer, hArch2.origin.col, hArch2.origin.row,
			hArch2.length, hArch2.attr, hArch2.code);
}

void Layout_Ruller_Load(Layer *layer, CUINT row)
{
	LayerDeclare(LAYOUT_RULLER_LOAD, draw.Size.width, 0, row, hLoad0);
	LayerCopyAt(layer, hLoad0.origin.col, hLoad0.origin.row,
			hLoad0.length, hLoad0.attr, hLoad0.code);
	/* Alternate the color of the frequency ratios			*/
	int idx = ratio.Count, bright = 1;
	while (idx-- > 0) {
		char tabStop[] = "00";
		int hPos=ratio.Uniq[idx] * draw.Area.LoadWidth / ratio.Maximum;
		sprintf(tabStop, "%2u", ratio.Uniq[idx]);

	    if (tabStop[0] != 0x20) {
		LayerAt(layer, code,
			(hLoad0.origin.col + hPos + 2),
			hLoad0.origin.row) = tabStop[0];

		LayerAt(layer, attr,
			(hLoad0.origin.col + hPos + 2),
			hLoad0.origin.row) = MakeAttr(CYAN, 0, BLACK, bright);
	    }
		LayerAt(layer, code,
			(hLoad0.origin.col + hPos + 3),
			hLoad0.origin.row) = tabStop[1];

		LayerAt(layer, attr,
			(hLoad0.origin.col + hPos + 3),
			hLoad0.origin.row) = MakeAttr(CYAN, 0, BLACK, bright);

		bright = !bright;
	}
}

CUINT Layout_Monitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_FREQUENCY, 77,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			hMon0);

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	LayerFillAt(	layer, (hMon0.origin.col + hMon0.length),
			hMon0.origin.row,
			(draw.Size.width - hMon0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	return(0);
}

CUINT Layout_Monitor_Instructions(Layer *layer,const unsigned int cpu,CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_INST, 76,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			hMon0);

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	LayerFillAt(	layer, (hMon0.origin.col + hMon0.length),
			hMon0.origin.row,
			(draw.Size.width - hMon0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	return(0);
}

CUINT Layout_Monitor_Common(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_COMMON, 77,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			hMon0);

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	LayerFillAt(	layer, (hMon0.origin.col + hMon0.length),
			hMon0.origin.row,
			(draw.Size.width - hMon0.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	return(0);
}

CUINT Layout_Monitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	return(0);
}

CUINT Layout_Monitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_TASKS, (MAX_WIDTH - LOAD_LEAD + 1),
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			hMon0);

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

	cTask[cpu].col = LOAD_LEAD + 8;
	cTask[cpu].row = hMon0.origin.row;

	return(0);
}

CUINT Layout_Ruller_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULLER_FREQUENCY, draw.Size.width,
			0, row, hFreq0);

	LayerCopyAt(	layer, hFreq0.origin.col, hFreq0.origin.row,
			hFreq0.length, hFreq0.attr, hFreq0.code);

	if (!draw.Flag.avgOrPC) {
		LayerDeclare(	LAYOUT_RULLER_FREQUENCY_AVG, draw.Size.width,
				0, (row + draw.Area.MaxRows + 1),
				hAvg0);

		LayerCopyAt(	layer, hAvg0.origin.col, hAvg0.origin.row,
				hAvg0.length, hAvg0.attr, hAvg0.code);
	} else {
		LayerDeclare(	LAYOUT_RULLER_FREQUENCY_PKG, draw.Size.width,
				0, (row + draw.Area.MaxRows + 1),
				hPkg0);

		LayerCopyAt(	layer, hPkg0.origin.col, hPkg0.origin.row,
				hPkg0.length, hPkg0.attr, hPkg0.code);
	}
	row += draw.Area.MaxRows + 2;
	return(row);
}

CUINT Layout_Ruller_Instructions(Layer *layer,const unsigned int cpu,CUINT row)
{
	LayerFillAt(	layer, 0, row, draw.Size.width,
			RSC(LAYOUT_RULLER_INST).CODE(),
			RSC(LAYOUT_RULLER_INST).ATTR()[0]);

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));

	row += draw.Area.MaxRows + 2;
	return(row);
}

CUINT Layout_Ruller_Cycles(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerFillAt(	layer, 0, row, draw.Size.width,
			RSC(LAYOUT_RULLER_CYCLES).CODE(),
			RSC(LAYOUT_RULLER_CYCLES).ATTR()[0]);

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += draw.Area.MaxRows + 2;
	return(row);
}

CUINT Layout_Ruller_CStates(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerFillAt(	layer, 0, row, draw.Size.width,
			RSC(LAYOUT_RULLER_CSTATES).CODE(),
			RSC(LAYOUT_RULLER_CSTATES).ATTR()[0]);

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += draw.Area.MaxRows + 2;
	return(row);
}

CUINT Layout_Ruller_Interrupts(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULLER_INTERRUPTS, draw.Size.width,
			0, row, hIntr0);

	LayerCopyAt(	layer, hIntr0.origin.col, hIntr0.origin.row,
			hIntr0.length, hIntr0.attr, hIntr0.code);

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += draw.Area.MaxRows + 2;
	return(row);
}

CUINT Layout_Ruller_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	ASCII	hCState[7][2] = {
		{'0', '2'},
		{'0', '3'},
		{'0', '6'},
		{'0', '7'},
		{'0', '8'},
		{'0', '9'},
		{'1', '0'}
	};

	LayerFillAt(	layer, 0, row, draw.Size.width,
			RSC(LAYOUT_RULLER_PACKAGE).CODE(),
			RSC(LAYOUT_RULLER_PACKAGE).ATTR()[0]);

	unsigned int idx;
	for (idx = 0; idx < 7; idx++)
	{
		LayerCopyAt(	layer, 0, (row + idx + 1),
				draw.Size.width,
				RSC(LAYOUT_PACKAGE_PC).ATTR(),
				RSC(LAYOUT_PACKAGE_PC).CODE());

		LayerAt(layer, code, 2, (row + idx + 1)) = hCState[idx][0];
		LayerAt(layer, code, 3, (row + idx + 1)) = hCState[idx][1];
	}

	LayerDeclare(	LAYOUT_PACKAGE_UNCORE, draw.Size.width,
			0, (row + 8), hUncore);

	LayerCopyAt(	layer, hUncore.origin.col, hUncore.origin.row,
			hUncore.length, hUncore.attr, hUncore.code);

	LayerFillAt	(layer, 0, (row + 9),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	row += 2 + 8;
	return(row);
}

CUINT Layout_Ruller_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULLER_TASKS, draw.Size.width, 0, row, hTask0);

	struct {
		ATTRIBUTE *attr;
		ASCII	  *code;
	} hSort[SORTBYCOUNT] = {
		{
		.attr = RSC(LAYOUT_TASKS_STATE_SORTED).ATTR(),
		.code = RSC(LAYOUT_TASKS_STATE_SORTED).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_RUNTIME_SORTED).ATTR(),
		.code = RSC(LAYOUT_TASKS_RUNTIME_SORTED).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_USRTIME_SORTED).ATTR(),
		.code = RSC(LAYOUT_TASKS_USRTIME_SORTED).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_SYSTIME_SORTED).ATTR(),
		.code = RSC(LAYOUT_TASKS_SYSTIME_SORTED).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_PROCESS_SORTED).ATTR(),
		.code = RSC(LAYOUT_TASKS_PROCESS_SORTED).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_COMMAND_SORTED).ATTR(),
		.code = RSC(LAYOUT_TASKS_COMMAND_SORTED).CODE()
		}
	};

	struct {							\
		Coordinate	origin;					\
		CUINT		length;					\
		ATTRIBUTE	*attr;					\
		ASCII		*code;					\
	} hTask1 = {							\
		.origin = {						\
			.col = 23,					\
			.row = row					\
		},							\
		.length = 21,						\
		.attr = hSort[Shm->SysGate.sortByField].attr,		\
		.code = hSort[Shm->SysGate.sortByField].code		\
	};

	struct {
		ATTRIBUTE *attr;
		ASCII	  *code;
	} hReverse[2] = {
		{
		.attr = RSC(LAYOUT_TASKS_REVERSE_SORT_OFF).ATTR(),
		.code = RSC(LAYOUT_TASKS_REVERSE_SORT_OFF).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_REVERSE_SORT_ON).ATTR(),
		.code = RSC(LAYOUT_TASKS_REVERSE_SORT_ON).CODE()
		}
	};

	struct {							\
		Coordinate	origin;					\
		CUINT		length;					\
		ATTRIBUTE	*attr;					\
		ASCII		*code;					\
	} hTask2 = {							\
		.origin = {						\
			.col = (draw.Size.width - 18),			\
			.row = (row + draw.Area.MaxRows + 1)		\
		},							\
		.length = 15,						\
		.attr = hReverse[Shm->SysGate.reverseOrder].attr,	\
		.code = hReverse[Shm->SysGate.reverseOrder].code	\
	};

	LayerDeclare(	LAYOUT_TASKS_VALUE_SWITCH, 13,
			(draw.Size.width - 34), (row + draw.Area.MaxRows + 1),
			hTask3);

	struct {
		ATTRIBUTE *attr;
		ASCII	  *code;
	} hTaskVal[2] = {
		{
		.attr = RSC(LAYOUT_TASKS_VALUE_OFF).ATTR(),
		.code = RSC(LAYOUT_TASKS_VALUE_OFF).CODE()
		},
		{
		.attr = RSC(LAYOUT_TASKS_VALUE_ON).ATTR(),
		.code = RSC(LAYOUT_TASKS_VALUE_ON).CODE()
		}
	};

	memcpy(&hTask3.attr[8], hTaskVal[draw.Flag.taskVal].attr, 3);
	memcpy(&hTask3.code[8], hTaskVal[draw.Flag.taskVal].code, 3);

	LayerDeclare(LAYOUT_TASKS_TRACKING, 22, 55, row, hTrack0);

	if (Shm->SysGate.trackTask) {
		memset(&hTrack0.attr[15], MakeAttr(CYAN, 0, BLACK, 0).value, 5);
		sprintf(buffer, "%5d", Shm->SysGate.trackTask);
		memcpy(&hTrack0.code[15], buffer, 5);
	}

	LayerCopyAt(layer, hTask0.origin.col, hTask0.origin.row,
			hTask0.length, hTask0.attr, hTask0.code);

	LayerCopyAt(layer, hTask1.origin.col, hTask1.origin.row,
			hTask1.length, hTask1.attr, hTask1.code);

	LayerFillAt(layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	LayerCopyAt(layer, hTask2.origin.col, hTask2.origin.row,
			hTask2.length, hTask2.attr, hTask2.code);

	LayerCopyAt(layer, hTask3.origin.col, hTask3.origin.row,
			hTask3.length, hTask3.attr, hTask3.code);

	LayerCopyAt(layer, hTrack0.origin.col, hTrack0.origin.row,
			hTrack0.length, hTrack0.attr, hTrack0.code);

	row += draw.Area.MaxRows + 2;
	return(row);
}

ASCII	Layout_Power_Domain_Code[PWR_DOMAIN(SIZE)][7] = {
		{'P','a','c','k','a','g','e'},
		{'C','o','r','e','s',' ',' '},
		{'U','n','c','o','r','e',' '},
		{'M','e','m','o','r','y',' '}
	};

CUINT Layout_Ruller_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	const CUINT tab = LOAD_LEAD + 24 + 6;
	CUINT vsh = row + PWR_DOMAIN(SIZE) + 1;

	LayerDeclare(LAYOUT_RULLER_VOLTAGE, draw.Size.width, 0, row, hVolt0);

	LayerCopyAt(	layer, hVolt0.origin.col, hVolt0.origin.row,
			hVolt0.length, hVolt0.attr, hVolt0.code);

	enum PWR_DOMAIN pw;
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
	{
		LayerDeclare(	LAYOUT_POWER_MONITOR, 39,
				tab, (row + pw + 1),
				hPower0);

		memcpy(&hPower0.code[0], Layout_Power_Domain_Code[pw], 7);

		LayerCopyAt(	layer, hPower0.origin.col, hPower0.origin.row,
				hPower0.length, hPower0.attr, hPower0.code);
	}
	if (draw.Area.MaxRows > 4)
		vsh += draw.Area.MaxRows - 4;

    LayerFillAt(layer, 0, vsh,draw.Size.width, hLine,MakeAttr(WHITE,0,BLACK,0));

	row += CUMAX(draw.Area.MaxRows, 4) + 2;
	return(row);
}

CUINT Layout_Ruller_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULLER_SLICE, draw.Size.width, 0, row, hSlice0);

	LayerCopyAt(	layer, hSlice0.origin.col, hSlice0.origin.row,
			hSlice0.length, hSlice0.attr, hSlice0.code);

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));

	row += draw.Area.MaxRows + 2;
	return(row);
}

void Layout_Footer(Layer *layer, CUINT row)
{
	CUINT col = 0;
	size_t len;

	LayerDeclare(LAYOUT_FOOTER_TECH_X86, 14, 0, row, hTech0);

	const ATTRIBUTE Pwr[] = {
		MakeAttr(BLACK, 0, BLACK, 1),
		MakeAttr(GREEN, 0, BLACK, 1),
		MakeAttr(BLUE,  0, BLACK, 1)
	};
	const struct {  ASCII *code  ; ATTRIBUTE attr; } TSC[] = {
		{(ASCII *) "  TSC  " , MakeAttr(BLACK, 0, BLACK, 1)},
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

	LayerCopyAt(	layer, hTech0.origin.col, hTech0.origin.row,
			hTech0.length, hTech0.attr, hTech0.code);

	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
	    LayerDeclare(LAYOUT_FOOTER_TECH_INTEL, 66,
			hTech0.length, hTech0.origin.row,
			hTech1);

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =
					Pwr[Shm->Proc.Features.HyperThreading];

		const ATTRIBUTE TM[] = {
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

	    hTech1.attr[43] = hTech1.attr[44] = 			\
			TM[Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1
			  |Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2];

	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
			hTech1.origin.row,
			(draw.Size.width - hTech0.length-hTech1.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	} else {
	  if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  {
	    LayerDeclare(LAYOUT_FOOTER_TECH_AMD, 66,
			hTech0.length, hTech0.origin.row,
			hTech1);

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =		\
					Pwr[Shm->Proc.Features.HyperThreading];

	  hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[ 6] = hTech1.attr[ 7]= \
	  hTech1.attr[8] = hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11]= \
					Pwr[(Shm->Proc.PowerNow == 0b11)];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =	\
	    hTech1.attr[16] = hTech1.attr[17] = Pwr[Shm->Proc.Technology.Turbo];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] =	\
						Pwr[Shm->Proc.Technology.C1E];

	    hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =	\
						Pwr[Shm->Proc.Technology.CC6];

	    hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] =	\
						Pwr[Shm->Proc.Technology.PC6];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[31] = buffer[0];
	    hTech1.code[32] = buffer[1];
	    hTech1.code[33] = buffer[2];

	    hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] =	\
						Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] =	\
				Pwr[(Shm->Proc.Features.AdvPower.EDX.TS != 0)];

	    hTech1.attr[39] = hTech1.attr[40] = hTech1.attr[41] =	\
				Pwr[(Shm->Proc.Features.AdvPower.EDX.TTP != 0)];

	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
			hTech1.origin.row,
			(draw.Size.width - hTech0.length-hTech1.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
	  }
	}
	row++;

	len = sprintf(	buffer, "%s",
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
		len = sprintf(	buffer, "%hu.%hu.%hu",
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

	LayerDeclare(	LAYOUT_FOOTER_SYSTEM, 42,
			(draw.Size.width - 42), row, hSys1);

	len = hSys1.origin.col - col;
	if ((signed int) len  > 0) {
		LayerFillAt(	layer, col, hSys1.origin.row,
				len, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}
	LayerCopyAt(	layer, hSys1.origin.col, hSys1.origin.row,
			hSys1.length, hSys1.attr, hSys1.code);

	/* Reset Tasks count & Memory usage				*/
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		PrintTaskMemory(layer, row,
				Shm->SysGate.taskCount,
				Shm->SysGate.memInfo.freeram,
				Shm->SysGate.memInfo.totalram);
}

void Layout_Load_UpperView(Layer *layer, const unsigned int cpu, CUINT row)
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
	LayerAt(layer, attr, 0, (1 + row + draw.Area.MaxRows)) =	\
						MakeAttr(WHITE,0,BLACK,0);
	LayerAt(layer, code, 0, (1 + row + draw.Area.MaxRows)) = '#';
	LayerAt(layer, code, 1, (1 + row + draw.Area.MaxRows)) = buffer[0];
	LayerAt(layer, code, 2, (1 + row + draw.Area.MaxRows)) = buffer[1];
}

void Draw_Load(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (!BITVAL(Shm->Cpu[cpu].OffLine, HW))
	{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
		/* Upper view area					*/
		CUINT	bar0 =((CFlop->Relative.Ratio > ratio.Maximum ?
				ratio.Maximum : CFlop->Relative.Ratio)
				* draw.Area.LoadWidth) / ratio.Maximum,
			bar1 = draw.Area.LoadWidth - bar0;
		/* Print the Per Core BCLK indicator (yellow)		*/
		LayerAt(layer, code, (LOAD_LEAD - 1), row) =		\
			(draw.iClock == (cpu - draw.cpuScroll)) ? '~' : 0x20;
		/* Draw the relative Core frequency ratio		*/
		LayerFillAt(layer, LOAD_LEAD, row,
			bar0, hBar,
			MakeAttr((CFlop->Relative.Ratio > ratio.Median ?
				RED : CFlop->Relative.Ratio > ratio.Minimum ?
					YELLOW : GREEN),
				0, BLACK, 1));
		/* Pad with blank characters				*/
		LayerFillAt(layer, (bar0 + LOAD_LEAD), row,
				bar1, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}
}

CUINT Draw_Monitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len = 0;

	switch (Shm->Proc.thermalFormula) {
	case THERMAL_FORMULA_INTEL:
	case THERMAL_FORMULA_AMD:
	case THERMAL_FORMULA_AMD_0Fh:
		len = sprintf(buffer,
			"%7.2f" " (" "%5.2f" ") "			\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
			"%-3u" "/" "%3u" "/" "%3u",
			CFlop->Relative.Freq,
			CFlop->Relative.Ratio,
			100.f * CFlop->State.Turbo,
			100.f * CFlop->State.C0,
			100.f * CFlop->State.C1,
			100.f * CFlop->State.C3,
			100.f * CFlop->State.C6,
			100.f * CFlop->State.C7,
			Shm->Cpu[cpu].PowerThermal.Limit[0],
			CFlop->Thermal.Temp,
			Shm->Cpu[cpu].PowerThermal.Limit[1]);
		break;
	case THERMAL_FORMULA_AMD_15h:
	case THERMAL_FORMULA_AMD_17h:
	    if (cpu == Shm->Proc.Service.Core)
		len = sprintf(buffer,
			"%7.2f" " (" "%5.2f" ") "			\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
			"%-3u" "/" "%3u" "/" "%3u",
			CFlop->Relative.Freq,
			CFlop->Relative.Ratio,
			100.f * CFlop->State.Turbo,
			100.f * CFlop->State.C0,
			100.f * CFlop->State.C1,
			100.f * CFlop->State.C3,
			100.f * CFlop->State.C6,
			100.f * CFlop->State.C7,
			Shm->Cpu[cpu].PowerThermal.Limit[0],
			CFlop->Thermal.Temp,
			Shm->Cpu[cpu].PowerThermal.Limit[1]);
	    else
		len = sprintf(buffer,
			"%7.2f" " (" "%5.2f" ") "			\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
			"%.*s",
			CFlop->Relative.Freq,
			CFlop->Relative.Ratio,
			100.f * CFlop->State.Turbo,
			100.f * CFlop->State.C0,
			100.f * CFlop->State.C1,
			100.f * CFlop->State.C3,
			100.f * CFlop->State.C6,
			100.f * CFlop->State.C7,
			11, hSpace);
	    break;
	case THERMAL_FORMULA_NONE:
		len = sprintf(buffer,
			"%7.2f" " (" "%5.2f" ") "			\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
			"%.*s",
			CFlop->Relative.Freq,
			CFlop->Relative.Ratio,
			100.f * CFlop->State.Turbo,
			100.f * CFlop->State.C0,
			100.f * CFlop->State.C1,
			100.f * CFlop->State.C3,
			100.f * CFlop->State.C6,
			100.f * CFlop->State.C7,
			11, hSpace);
		break;
	}
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	ATTRIBUTE warning = {.fg = WHITE, .un = 0, .bg = BLACK, .bf = 1};

	if (CFlop->Thermal.Temp <= Shm->Cpu[cpu].PowerThermal.Limit[0])
			warning = MakeAttr(BLUE, 0, BLACK, 1);
	else {
		if (CFlop->Thermal.Temp >= Shm->Cpu[cpu].PowerThermal.Limit[1])
			warning = MakeAttr(YELLOW, 0, BLACK, 0);
	}
	if (CFlop->Thermal.Events & (	EVENT_THERM_SENSOR
				      |	EVENT_THERM_PROCHOT
				      |	EVENT_THERM_CRIT
				      |	EVENT_THERM_THOLD ))
	{
		warning = MakeAttr(RED, 0, BLACK, 1);
	}
	LayerAt(layer, attr, (LOAD_LEAD + 69), row) =			\
		LayerAt(layer, attr, (LOAD_LEAD + 70), row) =		\
			LayerAt(layer, attr, (LOAD_LEAD + 71), row) = warning;

	return(0);
}

CUINT Draw_Monitor_Instructions(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(	buffer,
			"%17.6f" "/s"					\
			"%17.6f" "/c"					\
			"%17.6f" "/i"					\
			"%18llu",
			CFlop->State.IPS,
			CFlop->State.IPC,
			CFlop->State.CPI,
			CFlop->Delta.INST);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
}

CUINT Draw_Monitor_Cycles(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(	buffer,
			"%18llu%18llu%18llu%18llu",
			CFlop->Delta.C0.UCC,
			CFlop->Delta.C0.URC,
			CFlop->Delta.C1,
			CFlop->Delta.TSC);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
}

CUINT Draw_Monitor_CStates(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(	buffer,
			"%18llu%18llu%18llu%18llu",
			CFlop->Delta.C1,
			CFlop->Delta.C3,
			CFlop->Delta.C6,
			CFlop->Delta.C7);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
}

CUINT Draw_Monitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	return(0);
}

CUINT Draw_Monitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer, "%7.2f", CFlop->Relative.Freq);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);
	/* Clear the trailing garbage left by last drawing.		*/
	if ((len = draw.Size.width - cTask[cpu].col) > 0) {
		LayerFillAt(	layer, cTask[cpu].col, row,
				len, hSpace, MakeAttr(BLACK, 0, BLACK, 0));
	}
	cTask[cpu].col = LOAD_LEAD + 8;

	return(0);
}

CUINT Draw_Monitor_Interrupts(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(buffer, "%10u", CFlop->Counter.SMI);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	if (Shm->Registration.nmi) {
		len = sprintf(	buffer,
				"%10u%10u%10u%10u",
				CFlop->Counter.NMI.LOCAL,
				CFlop->Counter.NMI.UNKNOWN,
				CFlop->Counter.NMI.PCISERR,
				CFlop->Counter.NMI.IOCHECK);
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 24),row), buffer, len);
	}
	return(0);
}

CUINT Draw_Monitor_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len = 0;

	switch (Shm->Proc.voltageFormula) {
	case VOLTAGE_FORMULA_INTEL:
	case VOLTAGE_FORMULA_INTEL_CORE2:
	case VOLTAGE_FORMULA_AMD:
	case VOLTAGE_FORMULA_AMD_0Fh:
		len = sprintf(	buffer,
				"%7.2f "				\
				"%7d   %5.4f",
				CFlop->Relative.Freq,
				CFlop->Voltage.VID,
				CFlop->Voltage.Vcore);
		break;
	case VOLTAGE_FORMULA_INTEL_SNB:
	case VOLTAGE_FORMULA_INTEL_SKL_X:
	case VOLTAGE_FORMULA_AMD_17h:
	    if (cpu == Shm->Proc.Service.Core)
		len = sprintf(	buffer,
				"%7.2f "				\
				"%7d   %5.4f",
				CFlop->Relative.Freq,
				CFlop->Voltage.VID,
				CFlop->Voltage.Vcore);
	    else
		len = sprintf(buffer, "%7.2f ", CFlop->Relative.Freq);
	    break;
	case VOLTAGE_FORMULA_NONE:
		len = sprintf(buffer, "%7.2f ", CFlop->Relative.Freq);
		break;
	}
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
}

CUINT Draw_Monitor_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = sprintf(	buffer,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu",
			CFlop->Relative.Freq,
			Shm->Cpu[cpu].Slice.Delta.TSC,
			Shm->Cpu[cpu].Slice.Delta.INST,
			Shm->Cpu[cpu].Slice.Counter[1].TSC,
			Shm->Cpu[cpu].Slice.Counter[1].INST);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), buffer, len);

	return(0);
}

CUINT Draw_AltMonitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	size_t len;

	row += 2 + draw.Area.MaxRows;
	if (!draw.Flag.avgOrPC) {
		len = sprintf(	buffer,
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% " \
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%",
				100.f * Shm->Proc.Avg.Turbo,
				100.f * Shm->Proc.Avg.C0,
				100.f * Shm->Proc.Avg.C1,
				100.f * Shm->Proc.Avg.C3,
				100.f * Shm->Proc.Avg.C6,
				100.f * Shm->Proc.Avg.C7);
		memcpy(&LayerAt(layer, code, 20, row), buffer, len);
	} else {
		len = sprintf(	buffer,
				"  c2:%-5.1f" "  c3:%-5.1f" "  c6:%-5.1f" \
				"  c7:%-5.1f" "  c8:%-5.1f" "  c9:%-5.1f" \
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
	row += 1;
	return(row);
}

CUINT Draw_AltMonitor_Common(Layer *layer, const unsigned int cpu, CUINT row)
{
	row += 1 + TOP_FOOTER_ROW + draw.Area.MaxRows;
	return(row);
}

CUINT Draw_AltMonitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	CUINT bar0, bar1, margin = draw.Area.LoadWidth - 28;
	size_t len;

	row += 2;
/* PC02 */
	bar0 = Shm->Proc.State.PC02 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC02, 100.f * Shm->Proc.State.PC02,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, row), buffer, len);
/* PC03 */
	bar0 = Shm->Proc.State.PC03 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC03, 100.f * Shm->Proc.State.PC03,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, (row + 1)), buffer, len);
/* PC06 */
	bar0 = Shm->Proc.State.PC06 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC06, 100.f * Shm->Proc.State.PC06,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, (row + 2)), buffer, len);
/* PC07 */
	bar0 = Shm->Proc.State.PC07 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC07, 100.f * Shm->Proc.State.PC07,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, (row + 3)), buffer, len);
/* PC08 */
	bar0 = Shm->Proc.State.PC08 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC08, 100.f * Shm->Proc.State.PC08,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, (row + 4)), buffer, len);
/* PC09 */
	bar0 = Shm->Proc.State.PC09 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC09, 100.f * Shm->Proc.State.PC09,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, (row + 5)), buffer, len);
/* PC10 */
	bar0 = Shm->Proc.State.PC10 * margin;
	bar1 = margin - bar0;

	len = sprintf(	buffer, "%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC10, 100.f * Shm->Proc.State.PC10,
			bar0, hBar, bar1, hSpace);
	memcpy(&LayerAt(layer, code, 5, (row + 6)), buffer, len);
/* TSC & UNCORE */
	len = sprintf(buffer, "%18llu", PFlop->Delta.PTSC);
	memcpy(&LayerAt(layer, code, 5,(row+7)), buffer, len);
	len = sprintf(buffer, "UNCORE:%18llu", PFlop->Uncore.FC0);
	memcpy(&LayerAt(layer, code, 50, (row + 7)), buffer, len);

	row += 1 + 8;
	return(row);
}

CUINT Draw_AltMonitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
  row += 2 + draw.Area.MaxRows;
  if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
	size_t len = 0;
	unsigned int idx;
	char stateStr[16];
	ATTRIBUTE *stateAttr;

    for (idx = 0; idx < Shm->SysGate.taskCount; idx++)
    {
      if (!BITVAL(Shm->Cpu[Shm->SysGate.taskList[idx].wake_cpu].OffLine, OS)
	&& (Shm->SysGate.taskList[idx].wake_cpu >= draw.cpuScroll)
	&& (Shm->SysGate.taskList[idx].wake_cpu < (	draw.cpuScroll
							+ draw.Area.MaxRows) ))
      {
	unsigned int ldx = 2;
	CSINT dif = draw.Size.width
		  - cTask[Shm->SysGate.taskList[idx].wake_cpu].col;

	if (dif > 0)
	{
	  stateAttr = StateToSymbol(Shm->SysGate.taskList[idx].state, stateStr);

	    if (Shm->SysGate.taskList[idx].pid == Shm->SysGate.trackTask) {
		stateAttr = RSC(TRACKER_STATE_COLOR).ATTR();
	    }
	    if (!draw.Flag.taskVal) {
		len = sprintf(	buffer, "%s",
				Shm->SysGate.taskList[idx].comm);
	    } else {
		switch (Shm->SysGate.sortByField) {
		case F_STATE:
			len = sprintf(	buffer, "%s(%s)",
					Shm->SysGate.taskList[idx].comm,
					stateStr);
			break;
		case F_RTIME:
			len = sprintf(	buffer, "%s(%llu)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].runtime);
			break;
		case F_UTIME:
			len = sprintf(	buffer, "%s(%llu)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].usertime);
			break;
		case F_STIME:
			len = sprintf(	buffer, "%s(%llu)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].systime);
			break;
		case F_PID:
			/* fallthrough */
		case F_COMM:
			len = sprintf(	buffer, "%s(%d)",
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
	    } else {
		LayerCopyAt(layer,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row,
			dif, stateAttr, buffer);

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col += dif;
	    }
	    while ((dif = draw.Size.width
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
  }
  row += 1;
  return(row);
}

CUINT Draw_AltMonitor_Power(Layer *layer, const unsigned int cpu, CUINT row)
{
	const CUINT col = LOAD_LEAD + 24 + 16;
	const CUINT tab = 13 + 3;

	row += 2;
    switch (Shm->Proc.powerFormula) {
    case POWER_FORMULA_INTEL:
    case POWER_FORMULA_INTEL_ATOM:
    case POWER_FORMULA_AMD:
    case POWER_FORMULA_AMD_17h:
	sprintf(buffer,
		"%13.9f" "%13.9f" "%13.9f" "%13.9f"			\
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
	break;
    case POWER_FORMULA_NONE:
	break;
    }
	row += 1 + CUMAX(draw.Area.MaxRows, 4);
	return(row);
}

void Draw_Footer(Layer *layer, CUINT row)
{	/* Update Footer view area					*/
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	struct FLIP_FLOP *SProc = &Shm->Cpu[Shm->Proc.Service.Core]	\
			.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core].Toggle];

	ATTRIBUTE eventAttr[4] = {
		MakeAttr(BLACK,  0, BLACK, 1),
		MakeAttr(RED,    0, BLACK, 1),
		MakeAttr(YELLOW, 0, BLACK, 1),
		MakeAttr(WHITE,  0, BLACK, 1)
	};
	unsigned int _hot = 0, _tmp = 0;

	if (!processorEvents) {
		_hot = 0;
		_tmp = 3;
	} else {
	    if (processorEvents & (EVENT_POWER_LIMIT | EVENT_CURRENT_LIMIT)) {
		_hot = 2;
		_tmp = 3;
	    } else {
		_hot = 1;
		_tmp = 1;
	    }
	}

	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
		LayerAt(layer, attr, 14+46, row) = \
		LayerAt(layer, attr, 14+47, row) = \
		LayerAt(layer, attr, 14+48, row) = eventAttr[_hot];
	else if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		LayerAt(layer, attr, 14+43, row) = \
		LayerAt(layer, attr, 14+44, row) = \
		LayerAt(layer, attr, 14+45, row) = eventAttr[_hot];

	LayerAt(layer, attr, 14+62, row) = \
	LayerAt(layer, attr, 14+63, row) = \
	LayerAt(layer, attr, 14+64, row) = eventAttr[_tmp];

	sprintf(buffer, "%3u%4.2f", PFlop->Thermal.Temp, SProc->Voltage.Vcore);
	memcpy(&LayerAt(layer, code, 76, row), &buffer[0], 3);
	memcpy(&LayerAt(layer, code, 68, row), &buffer[3], 4);

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)
	&& (Shm->SysGate.tickStep == Shm->SysGate.tickReset)) {
		if ((previous.TaskCount != Shm->SysGate.taskCount)
		 || (previous.FreeRAM != Shm->SysGate.memInfo.freeram)) {
			previous.TaskCount = Shm->SysGate.taskCount;
			previous.FreeRAM = Shm->SysGate.memInfo.freeram;

			PrintTaskMemory(layer, (row + 1),
					Shm->SysGate.taskCount,
					Shm->SysGate.memInfo.freeram,
					Shm->SysGate.memInfo.totalram);
		}
	}
}

void Draw_Header(Layer *layer, CUINT row)
{	/* Update Header view area					*/
	struct FLIP_FLOP *CFlop = NULL;
	unsigned int digit[9];

	CFlop = &Shm->Cpu[Shm->Proc.Top] \
		.FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];

	/* Print the Top value if delta exists with the previous one	*/
	if (!draw.Flag.clkOrLd) { /* Frequency MHz			*/
	    if (previous.TopFreq != CFlop->Relative.Freq) {
		previous.TopFreq = CFlop->Relative.Freq;

	      Clock2LCD(layer,0,row,CFlop->Relative.Freq,CFlop->Relative.Ratio);
	    }
	} else { /* C0 C-State % load					*/
		if (previous.TopLoad != Shm->Proc.Avg.C0) {
			double percent = 100.f * Shm->Proc.Avg.C0;
			previous.TopLoad = Shm->Proc.Avg.C0;

			Load2LCD(layer, 0, row, percent);
		}
	}
	/* Print the focused BCLK					*/
	row += 2;

	CFlop = &Shm->Cpu[ draw.iClock + draw.cpuScroll ]		\
		.FlipFlop[ !Shm->Cpu[draw.iClock + draw.cpuScroll].Toggle ];

	Dec2Digit(CFlop->Clock.Hz, digit);

	LayerAt(layer, code, 26 +  0, row) = digit[0] + '0';
	LayerAt(layer, code, 26 +  1, row) = digit[1] + '0';
	LayerAt(layer, code, 26 +  2, row) = digit[2] + '0';
	LayerAt(layer, code, 26 +  4, row) = digit[3] + '0';
	LayerAt(layer, code, 26 +  5, row) = digit[4] + '0';
	LayerAt(layer, code, 26 +  6, row) = digit[5] + '0';
	LayerAt(layer, code, 26 +  8, row) = digit[6] + '0';
	LayerAt(layer, code, 26 +  9, row) = digit[7] + '0';
	LayerAt(layer, code, 26 + 10, row) = digit[8] + '0';
}

/* >>> GLOBALS >>> */
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
/* <<< GLOBALS <<< */

#define Illuminates_CPU(_layer, _row, fg, bg, hi)			\
({									\
	LayerAt(_layer, attr, 1, _row) =				\
		LayerAt(_layer, attr, 1, (1 + _row + draw.Area.MaxRows)) =\
						MakeAttr(fg, 0, bg, hi);\
									\
	LayerAt(_layer, attr, 2, _row) =				\
		LayerAt(_layer, attr, 2, (1 + _row + draw.Area.MaxRows)) =\
						MakeAttr(fg, 0, bg, hi);\
})

void Layout_Header_DualView_Footer(Layer *layer)
{
	unsigned int cpu;
	CUINT row = 0;

	draw.Area.LoadWidth = draw.Size.width - LOAD_LEAD;

	Layout_Header(layer, row);

	row += TOP_HEADER_ROW;

	Layout_Ruller_Load(layer, row);

    for(cpu = draw.cpuScroll; cpu < (draw.cpuScroll + draw.Area.MaxRows); cpu++)
    {
	row++;

	Layout_Load_UpperView(layer, cpu, row);

	if (draw.View != V_PACKAGE)
		Layout_Load_LowerView(layer, row);

      if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
      {
	if (cpu == Shm->Proc.Service.Core)
		Illuminates_CPU(layer, row, CYAN, BLACK, 1);
	else if (cpu == Shm->Proc.Service.Thread)
		Illuminates_CPU(layer, row, CYAN, BLACK, 1);
	else
		Illuminates_CPU(layer, row, CYAN, BLACK, 0);

	Matrix_Layout_Monitor[draw.View](layer, cpu, row);
      } else {
	Illuminates_CPU(layer, row, BLUE, BLACK, 0);

	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), row,
			(draw.Size.width - LOAD_LEAD + 1));

	ClearGarbage(	dLayer, attr,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			(draw.Size.width - LOAD_LEAD + 1));

	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			(draw.Size.width - LOAD_LEAD + 1));
      }
    }
	row++;

	row = Matrix_Layout_Ruller[draw.View](layer, 0, row);

	Layout_Footer(layer, row);
}

void Dynamic_Header_DualView_Footer(Layer *layer)
{
	unsigned int cpu;
	CUINT row = 0;

	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	processorEvents = PFlop->Thermal.Events;

	Draw_Header(layer, row);

	row += TOP_HEADER_ROW;

	for (cpu = draw.cpuScroll;
		cpu < (draw.cpuScroll + draw.Area.MaxRows); cpu++)
	{
		struct FLIP_FLOP *CFlop;
		CFlop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		processorEvents |= CFlop->Thermal.Events;

		row++;

	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
	    {
		Draw_Load(layer, cpu, row);

		Matrix_Draw_Monitor[draw.View](layer,
						cpu,
						(1 + row + draw.Area.MaxRows));
	    }
	}
	row = Matrix_Draw_AltMon[draw.View](layer, 0, row);

	Draw_Footer(layer, row);
}

void Layout_Card_Core(Layer *layer, Card* card)
{
	unsigned int digit[9];
	unsigned int _cpu = card->data.dword.lo;

	Dec2Digit(_cpu, digit);

	if (!BITVAL(Shm->Cpu[_cpu].OffLine, OS))
	{
		LayerDeclare(	LAYOUT_CARD_CORE_ONLINE, (4 * INTER_WIDTH),
				card->origin.col, (card->origin.row + 3),
				hOnLine);

		LayerCopyAt(layer, hOnLine.origin.col, hOnLine.origin.row, \
				hOnLine.length, hOnLine.attr, hOnLine.code);
	} else {
		LayerDeclare(	LAYOUT_CARD_CORE_OFFLINE, (4 * INTER_WIDTH),
				card->origin.col, (card->origin.row + 3),
				hOffLine);

		card->data.dword.hi = RENDER_KO;

		LayerFillAt(layer, card->origin.col, (card->origin.row + 1), \
		(4 * INTER_WIDTH), " _  _  _  _ ", MakeAttr(BLACK,0,BLACK,1));

		LayerCopyAt(layer, hOffLine.origin.col, hOffLine.origin.row, \
				hOffLine.length, hOffLine.attr, hOffLine.code);
	}
	LayerAt(layer, code,
		(card->origin.col + 3),			\
		(card->origin.row + 3)) = digit[7] + '0';

	LayerAt(layer, code,				\
		(card->origin.col + 4),			\
		(card->origin.row + 3)) = digit[8] + '0';
}

void Layout_Card_CLK(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_CLK, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hCLK);

	LayerCopyAt(	layer, hCLK.origin.col, hCLK.origin.row,	\
			hCLK.length, hCLK.attr, hCLK.code);
}

void Layout_Card_Uncore(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_UNCORE, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hUncore);

	sprintf(buffer, "x%2u", Shm->Uncore.Boost[UNCORE_BOOST(MAX)]);
	hUncore.code[ 8] = buffer[0];
	hUncore.code[ 9] = buffer[1];
	hUncore.code[10] = buffer[2];

	LayerCopyAt(	layer, hUncore.origin.col, hUncore.origin.row,	\
			hUncore.length, hUncore.attr, hUncore.code);
}

void Layout_Card_Bus(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_BUS, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hBus);

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

	LayerCopyAt(	layer, hBus.origin.col, hBus.origin.row,	\
			hBus.length, hBus.attr, hBus.code);

	Counter2LCD(	layer, card->origin.col, card->origin.row,
			(double) Shm->Uncore.Bus.Speed);
}

void Layout_Card_MC(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_MC, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hRAM);

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

	LayerCopyAt(	layer, hRAM.origin.col, hRAM.origin.row,	\
			hRAM.length, hRAM.attr, hRAM.code);
	if (Shm->Uncore.CtrlCount > 0) {
		Counter2LCD(layer, card->origin.col, card->origin.row,
				(double) Shm->Uncore.CtrlSpeed);
	}
}

void Layout_Card_Load(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_LOAD, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hLoad);

	LayerCopyAt(	layer, hLoad.origin.col, hLoad.origin.row,	\
			hLoad.length, hLoad.attr, hLoad.code);
}

void Layout_Card_Idle(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_IDLE, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hIdle);

	LayerCopyAt(	layer, hIdle.origin.col, hIdle.origin.row,	\
			hIdle.length, hIdle.attr, hIdle.code);
}

void Layout_Card_RAM(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_RAM, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hMem);

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

	LayerCopyAt(	layer, hMem.origin.col, hMem.origin.row,	\
			hMem.length, hMem.attr, hMem.code);
    } else
	card->data.dword.hi = RENDER_KO;
}

void Layout_Card_Task(Layer *layer, Card* card)
{
	LayerDeclare(	LAYOUT_CARD_TASK, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hSystem);

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		LayerCopyAt(	layer, hSystem.origin.col, hSystem.origin.row,\
				hSystem.length, hSystem.attr, hSystem.code);
	else
		card->data.dword.hi = RENDER_KO;
}

unsigned int MoveDashboardCursor(Coordinate *coord)
{
	const CUINT	marginWidth = MARGIN_WIDTH + (4 * INTER_WIDTH);
	const CUINT	marginHeight = MARGIN_HEIGHT + INTER_HEIGHT;
	const CUINT	rightEdge = draw.Size.width - marginWidth,
			bottomEdge = draw.Size.height - LEADING_TOP;

	coord->col += marginWidth;
	if (coord->col > rightEdge) {
		coord->col = LEADING_LEFT;
		coord->row += marginHeight;
	}
	if (coord->row > bottomEdge) {
		return(RENDER_KO);
	}
	return(RENDER_OK);
}

void Layout_Dashboard(Layer *layer)
{
	Coordinate coord = {.col = LEADING_LEFT, .row = LEADING_TOP};

	Card *walker = cardList.head;
	while (walker != NULL) {
		walker->origin.col = coord.col;
		walker->origin.row = coord.row;
		walker->data.dword.hi = MoveDashboardCursor(&coord);
		if (walker->data.dword.hi == RENDER_OK)
			walker->hook.Layout(layer, walker);
		walker = walker->next;
	}
}

void Draw_Card_Core(Layer *layer, Card* card)
{
    if (card->data.dword.hi == RENDER_OK)
    {
	unsigned int digit[9];
	unsigned int _cpu = card->data.dword.lo;

	struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[_cpu].FlipFlop[!Shm->Cpu[_cpu].Toggle];

	ATTRIBUTE warning = {.fg=WHITE, .un=0, .bg=BLACK, .bf=1};

	Clock2LCD(layer, card->origin.col, card->origin.row,
			CFlop->Relative.Freq, CFlop->Relative.Ratio);

	if (CFlop->Thermal.Temp <= Shm->Cpu[_cpu].PowerThermal.Limit[0]) {
		warning = MakeAttr(BLUE, 0, BLACK, 1);
	} else if(CFlop->Thermal.Temp >= Shm->Cpu[_cpu].PowerThermal.Limit[1]) {
		warning = MakeAttr(YELLOW, 0, BLACK, 0);
	}
	if (CFlop->Thermal.Events) {
		warning = MakeAttr(RED, 0, BLACK, 1);
	}

	Dec2Digit(CFlop->Thermal.Temp, digit);

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
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
				.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core] \
					.Toggle];

	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];

	double clock = PFlop->Delta.PTSC / 1000000.f;

	Counter2LCD(layer, card->origin.col, card->origin.row, clock);

	sprintf(buffer, "%5.1f", CFlop->Clock.Hz / 1000000.f);

	memcpy(&LayerAt(layer, code, (card->origin.col+2),(card->origin.row+3)),
		buffer, 5);
}

void Draw_Card_Uncore(Layer *layer, Card* card)
{
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	double Uncore = PFlop->Uncore.FC0 / 1000000.f;

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
    if (card->data.dword.hi == RENDER_OK)
    {
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
    if (card->data.dword.hi == RENDER_OK)
    {
      if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
	size_t len = strnlen(Shm->SysGate.taskList[0].comm, 12);
	int	hl = (12 - len) / 2, hr = hl + hl % 2;
	char	stateStr[16];
	ATTRIBUTE *stateAttr;
	stateAttr = StateToSymbol(Shm->SysGate.taskList[0].state, stateStr);

	sprintf(buffer, "%.*s%s%.*s",
			hl, hSpace,
			Shm->SysGate.taskList[0].comm,
			hr, hSpace);

	LayerCopyAt(layer,	(card->origin.col + 0) ,		\
				(card->origin.row + 1) ,		\
				12 ,					\
				stateAttr ,				\
				buffer);

	len = sprintf(buffer, "%5u (%c)",
				Shm->SysGate.taskList[0].pid,
				stateStr[0]);

	LayerFillAt(layer,	(card->origin.col + 2) ,		\
				(card->origin.row + 2) ,		\
				len ,					\
				buffer ,				\
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
	unsigned int cpu;
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
/*TODO: LCD test. To be removed.
	char *mir[] = {
		" ! \" # $ % & \' () * + , -./",
		"  0123456789 : ; < = > ?",
		"@ ABCDEFGHIJKLMNO",
		"  PQRSTUVWXYZ [ \\ ] ^ _",
		"` abcdefghijklmno",
		"  pqrstuvwxyz { | } ~ \x7f"
	};

void Layout_NoHeader_SingleView_NoFooter(Layer *layer)
{
	PrintLCD(layer, 0, 1, strlen(mir[0]), mir[0], _WHITE);
	PrintLCD(layer, 0, 5, strlen(mir[1]), mir[1], _WHITE);
	PrintLCD(layer, 0, 9, strlen(mir[2]), mir[2], _WHITE);
	PrintLCD(layer, 0,13, strlen(mir[3]), mir[3], _WHITE);
	PrintLCD(layer, 0,17, strlen(mir[4]), mir[4], _WHITE);
	PrintLCD(layer, 0,21, strlen(mir[5]), mir[5], _WHITE);
}
*/
void Layout_NoHeader_SingleView_NoFooter(Layer *layer)
{
}

void Dynamic_NoHeader_SingleView_NoFooter(Layer *layer)
{
}


void Top(char option)
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

	SortUniqRatio();

	TrapScreenSize(SIGWINCH);
	signal(SIGWINCH, TrapScreenSize);

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

	draw.Disposal = (option == 'd') ? D_DASHBOARD : D_MAINVIEW;

	/* MAIN LOOP */
  while (!BITVAL(Shutdown, 0))
  {
    do
    {
	if ((draw.Flag.daemon = BITVAL(Shm->Proc.Sync, 0)) == 0) {
	    SCANKEY scan = {.key = 0};

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
	if (BITVAL(Shm->Proc.Sync, 62)) {/* Compute required, clear the layout*/
		SortUniqRatio();
		draw.Flag.clear = 1;
		BITCLR(LOCKLESS, Shm->Proc.Sync, 62);
	}
	if (BITVAL(Shm->Proc.Sync, 63)) {/* Platform changed,redraw the layout*/
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
		draw.Flag.layout = 1;
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);
	}
    } while (	!BITVAL(Shutdown, 0)
		&& !draw.Flag.daemon
		&& !draw.Flag.layout
		&& !draw.Flag.clear ) ;

    if (draw.Flag.height & draw.Flag.width)
    {
	if (draw.Flag.clear) {
		draw.Flag.clear  = 0;
		draw.Flag.layout = 1;
		ResetLayer(dLayer);
	}
	if (draw.Flag.layout) {
		draw.Flag.layout = 0;
		ResetLayer(sLayer);
		FillLayerArea(sLayer, 0, 0, draw.Size.width, draw.Size.height,
				(ASCII*) hSpace, MakeAttr(BLACK, 0, BLACK, 1));

		LayoutView[draw.Disposal](sLayer);
	}
	if (draw.Flag.daemon)
	{
		DynamicView[draw.Disposal](dLayer);

		/* Increment the BCLK indicator (skip offline CPU)	*/
		do {
			draw.iClock++;
			if (draw.iClock >= draw.Area.MaxRows)
				draw.iClock = 0;
		} while (BITVAL(Shm->Cpu[draw.iClock].OffLine, OS)
			&& (draw.iClock != Shm->Proc.Service.Core)) ;
	}
	/* Write to the standard output					*/
	WriteConsole(draw.Size, buffer);
    } else
	printf( CUH RoK "Term(%u x %u) < View(%u x %u)\n",
		draw.Size.width,draw.Size.height,MIN_WIDTH,draw.Area.MinHeight);
  }

  FreeAll(buffer);

  free(cTask);

  DestroyAllCards(&cardList);
}

int Help(char *appName)
{
	printf( "CoreFreq."						\
		"  Copyright (C) 2015-2019 CYRIL INGENIERIE\n\n");
	printf( "usage:\t%s [-option <arguments>]\n"			\
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
			SysInfoKernel(NULL, 80, NULL);
		}
		break;
	case 'u':
		SysInfoCPUID(NULL, 80, NULL);
		break;
	case 's':
	{
		Window tty = {.matrix.size.wth = 4};

		SysInfoProc(NULL, 80, NULL);

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, "ISA Extensions:");
		SysInfoISA(&tty, NULL);

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, "Features:");
		SysInfoFeatures(NULL, 80, NULL);

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, "Technologies:");
		SysInfoTech(NULL, 80, NULL);

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, "Performance Monitoring:");
		SysInfoPerfMon(NULL, 80, NULL);

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, "Power & Thermal Monitoring:");
		SysInfoPwrThermal(NULL, 80, NULL);
	}
		break;
	case 'j':
		JsonSysInfo(Shm, NULL);
		break;
	case 'm': {
		Window tty = {.matrix.size.wth = 6};
		Topology(&tty, NULL);
		}
		break;
	case 'M': {
		Window tty = {.matrix.size.wth = 14};
		MemoryController(&tty, NULL);
		}
		break;
	case 'R': {
		Window tty = {.matrix.size.wth = 17};
		SystemRegisters(&tty, NULL);
		}
		break;
	case 'i':
		TrapSignal(1);
		Instructions();
		TrapSignal(0);
		break;
	case 'c':
		TrapSignal(1);
		Counters();
		TrapSignal(0);
		break;
	case 'V':
		TrapSignal(1);
		Voltage();
		TrapSignal(0);
		break;
	case 'g':
		TrapSignal(1);
		Package();
		TrapSignal(0);
		break;
	case 'd':
		/* Fallthrough */
	case 't':
		{
		TERMINAL(IN);

		TrapSignal(1);
		Top(option);
		TrapSignal(0);

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
