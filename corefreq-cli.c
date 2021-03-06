/*
 * CoreFreq
 * Copyright (C) 2015-2021 CYRIL INGENIERIE
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
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <pwd.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreq-ui.h"
#include "corefreq-cli.h"
#include "corefreq-cli-rsc.h"
#include "corefreq-cli-json.h"
#include "corefreq-cli-extra.h"

SHM_STRUCT *Shm = NULL;

static Bit64 Shutdown __attribute__ ((aligned (8))) = 0x0;

SERVICE_PROC localService = {.Proc = -1};

UBENCH_DECLARE()

struct SETTING_ST Setting = {
	.fahrCels = 0,
	.jouleWatt= 1,
	.secret   = 1,
	._padding = 0
};

char ConfigFQN[1+4095] = {[0] = 0};

char *BuildConfigFQN(char *dirPath)
{
    if (ConfigFQN[0] == 0)
    {
	char *homePath, *dotted;
	if ((homePath = secure_getenv("XDG_CONFIG_HOME")) == NULL)
	{
		struct stat cfgStat;

		if ((homePath = secure_getenv("HOME")) == NULL) {
			struct passwd *pwd = getpwuid(getuid());
			if (pwd != NULL) {
				homePath = pwd->pw_dir;
			} else {
				homePath = ".";
			}
		}
		snprintf(&ConfigFQN[1], 4095, "%s/.config", homePath);

		if ((stat(&ConfigFQN[1], &cfgStat) == 0)
		 && (cfgStat.st_mode & S_IFDIR))
		{
			dotted = "/.config/";
		} else {
			dotted = "/.";
		}
	} else {
		dotted = "/";
	}
	snprintf(&ConfigFQN[1], 4095, "%s%s%s/corefreq.cfg",
		homePath, dotted, dirPath);

	ConfigFQN[0] = 1;
    }
	return (&ConfigFQN[1]);
}

int ClientFollowService(SERVICE_PROC *pSlave, SERVICE_PROC *pMaster, pid_t pid)
{
	if (pSlave->Proc != pMaster->Proc) {
		pSlave->Proc = pMaster->Proc;

		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(pSlave->Core, &cpuset);
		if (pSlave->Thread != -1) {
			CPU_SET(pSlave->Thread, &cpuset);
		}
		return (sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset));
	}
	return (0);
}

struct RULER_ST Ruler = {
	.Count = 0
};

#define GetTopOfRuler() (Shm->Cpu[Ruler.TopOf.Top].Boost[Ruler.TopOf.Boost])

#define SetTopOfRuler(_cpu, _boost)					\
(									\
	Ruler.TopOf = (struct TOPOF) { .Top = _cpu , .Boost = _boost}	\
)

void AggregateRatio(void)
{
	enum RATIO_BOOST lt, rt;
	unsigned int cpu,
		lowest = Shm->Cpu[Shm->Proc.Service.Core].Boost[BOOST(MAX)],
		highest = Shm->Cpu[Shm->Proc.Service.Core].Boost[BOOST(MIN)];

	Ruler.Count = 0;
    for (lt = BOOST(MIN); lt < BOOST(SIZE); lt++) {
	Ruler.Top[lt] = Shm->Proc.Service.Core;
	Ruler.Uniq[lt] = 0.0;
    }
	SetTopOfRuler(Shm->Proc.Service.Core, BOOST(MIN));

    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
    {
      if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
      {
	for (lt = BOOST(MIN); lt < BOOST(SIZE); lt++)
	{
	  if (Shm->Cpu[cpu].Boost[lt] > 0)
	  {
	    switch (lt) {
	    case BOOST(HWP_MIN):
	    case BOOST(MIN):
	      if(Shm->Cpu[cpu].Boost[lt] < Shm->Cpu[ Ruler.Top[lt] ].Boost[lt])
	      {
		Ruler.Top[lt] = cpu;
	      }
	      if (Shm->Cpu[cpu].Boost[lt] < lowest)
	      {
		lowest = Shm->Cpu[cpu].Boost[lt];
		SetTopOfRuler(Ruler.Top[lt], lt);
	      }
		break;
	    default:
	      if(Shm->Cpu[cpu].Boost[lt] > Shm->Cpu[ Ruler.Top[lt] ].Boost[lt])
	      {
		Ruler.Top[lt] = cpu;
	      }
	      if (Shm->Cpu[cpu].Boost[lt] > highest)
	      {
		highest = Shm->Cpu[cpu].Boost[lt];
		SetTopOfRuler(Ruler.Top[lt], lt);
	      }
		break;
	    }
	    for (rt = BOOST(MIN); rt < Ruler.Count; rt++)
	    {
		if (Ruler.Uniq[rt] == Shm->Cpu[cpu].Boost[lt])
		{
			break;
		}
	    }
	    if (rt == Ruler.Count)
	    {
		Ruler.Uniq[Ruler.Count] = Shm->Cpu[cpu].Boost[lt];
		Ruler.Count++;
	    }
	  }
	}
      }
    }
	Ruler.Minimum = (double) lowest;
	Ruler.Maximum = (double) highest;
	Ruler.Median=(double) Shm->Cpu[Ruler.Top[BOOST(ACT)]].Boost[BOOST(ACT)];
	if (Ruler.Median == 0.0) {
		Ruler.Median = (Ruler.Minimum + Ruler.Maximum) / 2.0;
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
			BITCLR(LOCKLESS, state, (unsigned int) idx);
			stateStr[jdx++] = symbol[1 + idx];
			stateAttr = symbAttr[1 + idx];
		} while (!BITBSR(state, idx));
	stateStr[jdx] = '\0';
	return (stateAttr);
}

unsigned int Dec2Digit( const unsigned int length, unsigned int decimal,
			unsigned int thisDigit[] )
{
	memset(thisDigit, 0, length * sizeof(unsigned int));
	register unsigned int j = length;
	while (decimal > 0) {
		thisDigit[--j] = decimal % 10;
		decimal /= 10;
	}
	return (length - j);
}

#define Cels2Fahr(cels)	(((cels * 117965) >> 16) + 32)

const char *Indent[2][4] = {
	{"",	"|",	"|- ",	"   |- "},
	{"",	" ",	"  ",	"   "}
};

TGrid *Print_v1(CELL_FUNC OutFunc,
		Window *win,
		unsigned long long key,
		ATTRIBUTE *attrib,
		CUINT width,
		int tab,
		char *fmt, ...)
{
	TGrid *pGrid = NULL;
	char *line = malloc(width + 1);
  if (line != NULL)
  {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(line, width + 1, fmt, ap);

    if (OutFunc == NULL) {
	printf("%s%s%.*s\n", Indent[0][tab], line,
		(int)(width - strlen(line) - strlen(Indent[0][tab])), hSpace);
    } else {
	ASCII *item = malloc(width + 1);
      if (item != NULL) {
	snprintf((char *)item, width + 1, "%s%s%.*s", Indent[1][tab], line,
		(int)(width - strlen(line) - strlen(Indent[1][tab])), hSpace);

	pGrid = OutFunc(win, key, attrib, item);

	free(item);
      }
    }
	va_end(ap);
	free(line);
  }
	return (pGrid);
}

TGrid *Print_v2(CELL_FUNC OutFunc,
		Window *win,
		CUINT *nl,
		ATTRIBUTE *attrib, ...)
{
	TGrid *pGrid = NULL;
	ASCII *item = malloc(32);
    if (item != NULL)
    {
	char *fmt;
	va_list ap;
	va_start(ap, attrib);
	if ((fmt = va_arg(ap, char*)) != NULL)
	{
		vsnprintf((char*) item, 32, fmt, ap);
		if (OutFunc == NULL) {
			(*nl)--;
			if ((*nl) == 0) {
				(*nl) = win->matrix.size.wth;
				printf("%s\n", item);
			} else
				printf("%s", item);
		} else {
			pGrid = OutFunc(win, SCANKEY_NULL, attrib, item);
		}
	}
	va_end(ap);
	free(item);
    }
	return (pGrid);
}

TGrid *Print_v3(CELL_FUNC OutFunc,
		Window *win,
		CUINT *nl,
		ATTRIBUTE *attrib, ...)
{
	TGrid *pGrid = NULL;
	ASCII *item = malloc(32);
    if (item != NULL)
    {
	char *fmt;
	va_list ap;
	va_start(ap, attrib);
	if ((fmt = va_arg(ap, char*)) != NULL)
	{
		vsnprintf((char*) item, 32, fmt, ap);
		if (OutFunc == NULL) {
			(*nl)--;
			if ((*nl) == (win->matrix.size.wth - 1)) {
				printf("|-%s", item);
			} else if ((*nl) == 0) {
				(*nl) = win->matrix.size.wth;
				printf("%s\n", item);
			} else {
				printf("%s", item);
			}
		} else {
			pGrid = OutFunc(win, SCANKEY_NULL, attrib, item);
		}
	}
	va_end(ap);
	free(item);
    }
	return (pGrid);
}

#define PUT(key, attrib, width, tab, fmt, ...)				\
	Print_v1(OutFunc, win, key, attrib, width, tab, fmt, __VA_ARGS__)

#define Print_REG	Print_v2
#define Print_MAP	Print_v2
#define Print_IMC	Print_v2
#define Print_ISA	Print_v3

#define PRT(FUN, attrib, ...)						\
	Print_##FUN(OutFunc, win, nl, attrib, __VA_ARGS__)

REASON_CODE SysInfoCPUID(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
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
			"CPU #%-3u function"				\
			"         EAX          EBX          ECX          EDX",
			cpu);
	    } else {
		PUT(SCANKEY_NULL,
			attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS)],
			width, 0, "CPU #%-3u", cpu);
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

		enum CPUID_ENUM i;
		for (i = 0; i < CPUID_MAX_FUNC; i++) {
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
	return (reason);
}

const struct {
	enum SYS_REG bit;
	unsigned int len;
	const char *flag, *comm;
} SR[] = {
	{RFLAG_TF,	1, " TF ", " Trap Flag "},
	{RFLAG_IF,	1, " IF ", " Interrupt Flag "},
	{RFLAG_IOPL,	2, "IOPL", " I/O Privilege Level "},
	{RFLAG_NT,	1, " NT ", " Nested Task "},
	{RFLAG_RF,	1, " RF ", " Resume Flag "},
	{RFLAG_VM,	1, " VM ", " Virtual-8086 Mode "},
	{RFLAG_AC,	1, " AC ", " Alignment Check "},
	{RFLAG_VIF,	1, " VIF", " Virtual Interrupt Flag "},
	{RFLAG_VIP,	1, " VIP", " Virtual Interrupt Pending "},
	{RFLAG_ID,	1, " ID ", " Identification "},

	{CR0_PE,	1, " PE ", " Protection Enable "},
	{CR0_MP,	1, " MP ", " Monitor Coprocessor "},
	{CR0_EM,	1, " EM ", " FPU Emulation "},
	{CR0_TS,	1, " TS ", " Task Switched "},
	{CR0_ET,	1, " ET ", " Extension Type "},
	{CR0_NE,	1, " NE ", " Numeric Exception "},
	{CR0_WP,	1, " WP ", " Write Protect "},
	{CR0_AM,	1, " AM ", " Alignment Mask "},
	{CR0_NW,	1, " NW ", " Not Write-through "},
	{CR0_CD,	1, " CD ", " Cache Disable "},
	{CR0_PG,	1, " PG ", " Paging enable "},

	{CR3_PWT,	1, " PWT", " Page-level Write-Through "},
	{CR3_PCD,	1, " PCD", " Page-level Cache Disable "},

	{CR4_VME,	1, " VME", " Virtual-8086 Mode Extensions "},
	{CR4_PVI,	1, " PVI", " Protected-mode Virtual Interrupts "},
	{CR4_TSD,	1, " TSD", " Time-Stamp Disable "},
	{CR4_DE,	1, " DE ", " Debugging Extensions "},
	{CR4_PSE,	1, " PSE", " Page Size Extension "},
	{CR4_PAE,	1, " PAE", " Physical Address Extension "},
	{CR4_MCE,	1, " MCE", " Machine-Check Enable "},
	{CR4_PGE,	1, " PGE", " Page Global Enable "},
	{CR4_PCE,	1, " PCE", " Performance Counter Enable "},
	{CR4_OSFXSR,	1, " FX ", " OS Support for FXSAVE and FXRSTOR "},
	{CR4_OSXMMEXCPT,1, "XMM ", " OS Support for Unmasked SSE Exceptions "},
	{CR4_UMIP,	1, "UMIP", " User-Mode Instruction Prevention "},
	{CR4_VMXE,	1, " VMX", " Virtual Machine eXtension Enable "},
	{CR4_SMXE,	1, " SMX", " Safer Mode eXtension Enable "},
	{CR4_FSGSBASE,	1, " FS ", NULL},
	{CR4_PCIDE,	1, "PCID", " Process-Context Identifiers Enable "},
	{CR4_OSXSAVE,	1, " SAV", " XSAVE and Processor Extended States "},
	{CR4_SMEP,	1, " SME", " Supervisor-Mode Execution Prevention "},
	{CR4_SMAP,	1, " SMA", " Supervisor-Mode Access Prevention "},
	{CR4_PKE,	1, " PKE", " Protection Keys for user-mode pages "},
/*TODO(add CR bits)
	{CR4.CET,	1," CET"," Control-flow Enforcement Technology "},
	{CR4.PKS,	1," PKS"," Protection Keys for Supervisor-mode pages "},
*/
	{EXFCR_LOCK,	1,	"LCK ", " Lock bit "},
	{EXFCR_VMX_IN_SMX,1,	"VMX^", " VMX Inside SMX Operation "},
	{EXFCR_VMXOUT_SMX,1,	"SGX ", " VMX Outside SMX Operation "},
	{EXFCR_SENTER_LEN,6,	"[SEN", " SENTER Local Functions "},
	{EXFCR_SENTER_GEN,1,	"TER]", " SENTER Global Functions "},
	{EXFCR_SGX_LCE, 1,	" [ S", " SGX Launch Control "},
	{EXFCR_SGX_GEN, 1,	"GX ]", " SGX Global Functions "},
	{EXFCR_LMCE,	1,	" LMC", " Local Machine Check "},

	{EXFER_SCE,	1,	" SCE", " System-Call Extension "},
	{EXFER_LME,	1,	" LME", " Long Mode Enable "},
	{EXFER_LMA,	1,	" LMA", " Long Mode Active "},
	{EXFER_NXE,	1,	" NXE", " Execute-Disable Bit Enable "},
	{EXFER_SVME,	1,	" SVM", " Secure Virtual Machine Enable "}
};

enum {
	IX_CPU,
	IX_FLAG,
	IX_CR0,
	IX_CR3,
	IX_CR4,
	IX_EFCR,
	IX_EFER,
	IX_4SPC,
	IX_NA_3
};

const struct {
	const char *item, *comm;
} CR[] = {
	[IX_CPU]	= {"CPU ", NULL},
	[IX_FLAG]	= {"FLAG", NULL},
	[IX_CR0]	= {"CR0:", " Control Register 0 "},
	[IX_CR3]	= {"CR3:", " Control Register 3 "},
	[IX_CR4]	= {"CR4:", " Control Register 4 "},
	[IX_EFCR]	= {"EFCR", " Feature Control Bits Register "},
	[IX_EFER]	= {"EFER", " Extended-Feature-Enable Register "},
	[IX_4SPC]	= {"    ", NULL},
	[IX_NA_3] 	= {"  - ", NULL}
};

REASON_CODE SystemRegisters(Window *win, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[5] = {
		RSC(SYSTEM_REGISTERS_COND0).ATTR(),
		RSC(SYSTEM_REGISTERS_COND1).ATTR(),
		RSC(SYSTEM_REGISTERS_COND2).ATTR(),
		RSC(SYSTEM_REGISTERS_COND3).ATTR(),
		RSC(SYSTEM_REGISTERS_COND4).ATTR()
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
	CUINT cells_per_line = win->matrix.size.wth, *nl = &cells_per_line;

/* Section Mark */
	PRT(REG, attrib[0], CR[IX_CPU].item);
	PRT(REG, attrib[0], CR[IX_FLAG].item);
    for (idx = tabRFLAGS.Start; idx < tabRFLAGS.Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }
	PRT(REG, attrib[0], CR[IX_4SPC].item);
	GridHover(PRT(REG, attrib[0], CR[IX_CR3].item), CR[IX_CR3].comm);

    for (idx = tabCR3.Start; idx < tabCR3.Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }
	PRT(REG, attrib[0], CR[IX_4SPC].item);
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS) ? 4:3], "#%-2u ",cpu);

	PRT(REG, attrib[0], CR[IX_4SPC].item);
	for (idx = tabRFLAGS.Start; idx < tabRFLAGS.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.RFLAGS,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
	PRT(REG, attrib[0], CR[IX_4SPC].item);
	PRT(REG, attrib[0], CR[IX_4SPC].item);
	for (idx = tabCR3.Start; idx < tabCR3.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR3,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
	PRT(REG, attrib[0], CR[IX_4SPC].item);
    }
/* Section Mark */
	GridHover(PRT(REG, attrib[0], CR[IX_CR0].item), CR[IX_CR0].comm);

    for (idx = tabCR0.Start; idx < tabCR0.Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }

	GridHover(PRT(REG, attrib[0], CR[IX_CR4].item), CR[IX_CR4].comm);

    for (idx = tabCR4[0].Start; idx < tabCR4[0].Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS) ? 4:3], "#%-2u ",cpu);

	for (idx = tabCR0.Start; idx < tabCR0.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR0,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
	PRT(REG, attrib[0], CR[IX_4SPC].item);
	for (idx = tabCR4[0].Start; idx < tabCR4[0].Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR4,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
    }
/* Section Mark */
	GridHover(PRT(REG, attrib[0], CR[IX_CR4].item), CR[IX_CR4].comm);

    for (idx = tabCR4[1].Start; idx < tabCR4[1].Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS) ? 4:3], "#%-2u ",cpu);

	for (idx = tabCR4[1].Start; idx < tabCR4[1].Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.CR4,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
    }
/* Section Mark */
	GridHover(PRT(REG, attrib[0], CR[IX_EFCR].item), CR[IX_EFCR].comm);
	PRT(REG, attrib[0], CR[IX_4SPC].item);

    for (idx = tabEFCR.Start; idx < tabEFCR.Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }

	PRT(REG, attrib[0], CR[IX_4SPC].item);
	GridHover(PRT(REG, attrib[0], CR[IX_EFER].item), CR[IX_EFER].comm);

    for (idx = tabEFER.Start; idx < tabEFER.Stop; idx++) {
	GridHover(PRT(REG, attrib[0], "%s", SR[idx].flag), SR[idx].comm);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	PRT(REG, attrib[BITVAL(Shm->Cpu[cpu].OffLine, OS) ? 4:3], "#%-2u ",cpu);

	PRT(REG, attrib[0], CR[IX_4SPC].item);
	for (idx = tabEFCR.Start; idx < tabEFCR.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)
	    &&  ((Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL))) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.EFCR,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
	PRT(REG, attrib[0], CR[IX_4SPC].item);
	PRT(REG, attrib[0], CR[IX_4SPC].item);
	for (idx = tabEFER.Start; idx < tabEFER.Stop; idx++) {
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
		PRT(REG, attrib[2], "%3llx ",
				BITEXTRZ(Shm->Cpu[cpu].SystemRegister.EFER,
				SR[idx].bit, SR[idx].len));
	    } else {
		PRT(REG, attrib[1], CR[IX_NA_3].item);
	    }
	}
    }
	return (reason);
}

char SymbUnlock[2][2] = {{'[', ']'}, {'<', '>'}};

TGrid *PrintRatioFreq(	Window *win, struct FLIP_FLOP *CFlop,
			unsigned int zerobase, char *pfx, unsigned int *pRatio,
			int syc, unsigned long long _key,
			CUINT width, CELL_FUNC OutFunc, ATTRIBUTE attrib[])
{
	TGrid *pGrid = NULL;

    if ((( (*pRatio) > 0)  && !zerobase) || (zerobase))
    {
	double Freq_MHz = ABS_FREQ_MHz(double, (*pRatio), CFlop->Clock);

	if ((Freq_MHz > 0.0) && (Freq_MHz < CLOCK_MHz(double, UNIT_GHz(10.0))))
	{
		pGrid = PUT(_key, attrib, width, 0,
			"%.*s""%s""%.*s""%7.2f""%.*s""%c%4d %c",
		(int) (20 - strlen(pfx)), hSpace, pfx, 3, hSpace,
			Freq_MHz,
			20, hSpace,
			SymbUnlock[syc][0],
			(*pRatio),
			SymbUnlock[syc][1]);
	} else {
		pGrid = PUT(_key, attrib, width, 0,
			"%.*s""%s""%.*s""%7s""%.*s""%c%4d %c",
		(int) (20 - strlen(pfx)), hSpace, pfx, 3, hSpace,
			RSC(AUTOMATIC).CODE(),
			20, hSpace,
			SymbUnlock[syc][0],
			(*pRatio),
			SymbUnlock[syc][1]);
	}
    }
	return (pGrid);
}

void RefreshBaseClock(TGrid *grid, DATA_TYPE data)
{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
			.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core].Toggle];
	char item[8+1];
	UNUSED(data);

	snprintf(item, 8+1, "%7.3f", CLOCK_MHz(double, CFlop->Clock.Hz));

	memcpy(&grid->cell.item[grid->cell.length - 9], item, 7);
}

void RefreshFactoryClock(TGrid *grid, DATA_TYPE data)
{
	char item[8+1];
	UNUSED(data);

	snprintf(item, 8+1, "%7.3f",
		CLOCK_MHz(double, Shm->Proc.Features.Factory.Clock.Hz));

	memcpy(&grid->cell.item[grid->cell.length - 9], item, 7);
}

void RefreshFactoryFreq(TGrid *grid, DATA_TYPE data)
{
	char item[11+11+1];
	UNUSED(data);

	snprintf(item, 11+11+1, "%5u" "%4d",
		Shm->Proc.Features.Factory.Freq,
		Shm->Proc.Features.Factory.Ratio);

	memcpy(&grid->cell.item[22], &item[0], 5);
	memcpy(&grid->cell.item[51], &item[5], 4);
}

void RefreshItemFreq(TGrid *grid, unsigned int ratio, double Freq_MHz)
{
	char item[11+8+1];

    if ((Freq_MHz > 0.0) && (Freq_MHz < CLOCK_MHz(double, UNIT_GHz(10.0)))) {
	snprintf(item,11+8+1,"%4u%7.2f", ratio, Freq_MHz);
    } else {
	snprintf(item,11+7+1,"%4u%7s", ratio, RSC(AUTOMATIC).CODE());
    }
	memcpy(&grid->cell.item[23], &item[4], 7);
	memcpy(&grid->cell.item[51], &item[0], 4);
}

void RefreshRatioFreq(TGrid *grid, DATA_TYPE data)
{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[
					Shm->Proc.Service.Core
				].FlipFlop[
					!Shm->Cpu[Shm->Proc.Service.Core
				].Toggle];
	RefreshItemFreq(grid,
			(*data.puint),
			ABS_FREQ_MHz(double, (*data.puint), CFlop->Clock));
}

void RefreshTopFreq(TGrid *grid, DATA_TYPE data)
{
	enum RATIO_BOOST boost = data.uint[0];
	unsigned int top = Ruler.Top[boost];
	unsigned int ratio = Shm->Cpu[top].Boost[boost];

	struct FLIP_FLOP *CFlop = &Shm->Cpu[top] \
			.FlipFlop[!Shm->Cpu[top].Toggle];

	RefreshItemFreq(grid, ratio,
			ABS_FREQ_MHz(double, ratio, CFlop->Clock));
}

REASON_CODE SysInfoProc(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_PROC_COND0).ATTR(),
		RSC(SYSINFO_PROC_COND1).ATTR(),
		RSC(SYSINFO_PROC_COND2).ATTR(),
		RSC(SYSINFO_PROC_COND3).ATTR()
	};
	struct FLIP_FLOP *CFlop;
	unsigned int activeCores;
	enum RATIO_BOOST boost = 0;

	PUT(SCANKEY_NULL, attrib[0], width, 0,
		"%s""%.*s[%s]", RSC(PROCESSOR).CODE(),
		width - 2 - RSZ(PROCESSOR) - strlen(Shm->Proc.Brand),
		hSpace, Shm->Proc.Brand);

    if (Shm->Proc.Features.Factory.PPIN > 0)
    {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%16llx]", "PPIN#",
		width - 21 - __builtin_strlen("PPIN#"),
		hSpace, Shm->Proc.Features.Factory.PPIN);
    }
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%s]", RSC(ARCHITECTURE).CODE(),
		width - 5 - RSZ(ARCHITECTURE) - strlen(Shm->Proc.Architecture),
		hSpace, Shm->Proc.Architecture);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%s]", RSC(VENDOR_ID).CODE(),
	width - 5 - RSZ(VENDOR_ID) - strlen(Shm->Proc.Features.Info.Vendor.ID),
		hSpace, Shm->Proc.Features.Info.Vendor.ID);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[0x%08x]", RSC(MICROCODE).CODE(),
		width - 15 - RSZ(MICROCODE), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].Query.Microcode);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%3X%1X_%1X%1X]", RSC(SIGNATURE).CODE(),
		width - 12 - RSZ(SIGNATURE), hSpace,
		Shm->Proc.Features.Std.EAX.ExtFamily,
		Shm->Proc.Features.Std.EAX.Family,
		Shm->Proc.Features.Std.EAX.ExtModel,
		Shm->Proc.Features.Std.EAX.Model);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%7u]", RSC(STEPPING).CODE(),
		width - 12 - RSZ(STEPPING), hSpace,
		Shm->Proc.Features.Std.EAX.Stepping);

	PUT(SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%3u/%3u]", RSC(ONLINE_CPU).CODE(),
		width - 12 - RSZ(ONLINE_CPU), hSpace,
		Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	CFlop = &Shm->Cpu[
			Shm->Proc.Service.Core
			].FlipFlop[
			!Shm->Cpu[Shm->Proc.Service.Core].Toggle
			];

	GridCall(PUT(SCANKEY_NULL, attrib[2], width, 2,
			"%s""%.*s[%7.3f]", RSC(BASE_CLOCK).CODE(),
			width - 12 - RSZ(BASE_CLOCK), hSpace,
			CLOCK_MHz(double, CFlop->Clock.Hz)),
		RefreshBaseClock);

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s(MHz)%.*s""%s", RSC(FREQUENCY).CODE(),
		21 - RSZ(FREQUENCY), hSpace,
		23 - (OutFunc == NULL), hSpace,
		RSC(RATIO).CODE());

    {
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_MIN;

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(MIN) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(MIN) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, "Min",
				&Shm->Cpu[
					Ruler.Top[ BOOST(MIN) ]
					].Boost[ BOOST(MIN) ],
				1, coreClock.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(MIN) );
    }
    {
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_MAX;

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(MAX) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(MAX) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, "Max",
				&Shm->Cpu[
					Ruler.Top[ BOOST(MAX) ]
					].Boost[ BOOST(MAX) ],
				1, coreClock.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(MAX) );
    }

	GridCall(PUT(SCANKEY_NULL, attrib[0], width, 2, "%s""%.*s[%7.3f]",
			RSC(FACTORY).CODE(),
			(OutFunc == NULL ? 68 : 64) - RSZ(FACTORY), hSpace,
			CLOCK_MHz(double,Shm->Proc.Features.Factory.Clock.Hz)),
		RefreshFactoryClock);

	GridCall(PUT(SCANKEY_NULL, attrib[3], width, 0,
			"%.*s""%5u""%.*s""[%4d ]",
			22, hSpace, Shm->Proc.Features.Factory.Freq,
			23, hSpace, Shm->Proc.Features.Factory.Ratio),
		RefreshFactoryFreq);

	PUT(SCANKEY_NULL, attrib[0], width, 2, "%s", RSC(PERFORMANCE).CODE());

	PUT(SCANKEY_NULL, attrib[0], width, 3, "%s", "P-State");
    {
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_TGT;

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(TGT) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(TGT) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				1, "TGT",
				&Shm->Cpu[
					Ruler.Top[ BOOST(TGT) ]
					].Boost[ BOOST(TGT) ],
				1, coreClock.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TGT) );
    }

    if (Shm->Proc.Features.HWP_Enable) {
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};

	PUT(SCANKEY_NULL, attrib[0], width, 3, "%s", "HWP");

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_HWP_MIN;

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(HWP_MIN) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(HWP_MIN) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				1, "Min",
				&Shm->Cpu[
					Ruler.Top[ BOOST(HWP_MIN) ]
					].Boost[ BOOST(HWP_MIN) ],
				1, coreClock.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(HWP_MIN) );

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_HWP_MAX;

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(HWP_MAX) ]
			].FlipFlop[
			!Shm->Cpu[Ruler.Top[ BOOST(HWP_MAX) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				1, "Max",
				&Shm->Cpu[
					Ruler.Top[ BOOST(HWP_MAX) ]
				].Boost[ BOOST(HWP_MAX) ],
				1, coreClock.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(HWP_MAX) );

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_HWP_TGT;

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(HWP_TGT) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(HWP_TGT) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				1, "TGT",
				&Shm->Cpu[
					Ruler.Top[ BOOST(HWP_TGT) ]
				].Boost[ BOOST(HWP_TGT) ],
				1, coreClock.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(HWP_TGT) );
    }

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.Turbo_Unlock],
		width, 2,
		"Turbo Boost%.*s[%7.*s]", width - 23, hSpace, 6,
			Shm->Proc.Features.Turbo_Unlock ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

    if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
      if (Shm->Proc.Features.TDP_Levels >= 2)
      {
	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(XFR) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(XFR) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, "XFR",
				&Shm->Cpu[
					Ruler.Top[ BOOST(XFR) ]
				].Boost[ BOOST(XFR) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(XFR) );
      }
      if (Shm->Proc.Features.TDP_Levels >= 1)
      {
	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(CPB) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(CPB) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, "CPB",
			&Shm->Cpu[Ruler.Top[BOOST(CPB)]].Boost[BOOST(CPB)],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(CPB) );
      }
    }

    {
      for(boost = BOOST(1C), activeCores = 1;
	  boost > BOOST(1C)-(enum RATIO_BOOST)Shm->Proc.Features.SpecTurboRatio;
		boost--, activeCores++)
	{
	CLOCK_ARG clockMod={.NC=BOXKEY_TURBO_CLOCK_NC | activeCores,.Offset=0};
	char pfx[10+1+1];
	snprintf(pfx, 10+1+1, "%2uC", activeCores);

	CFlop = &Shm->Cpu[
			Ruler.Top[boost]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[boost] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, pfx, &Shm->Cpu[
						Ruler.Top[boost]
						].Boost[boost],
				1, clockMod.sllong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, boost );
      }
    }

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.Uncore_Unlock],
		width, 2, "Uncore%.*s[%7.*s]", width - 18, hSpace, 6,
			Shm->Proc.Features.Uncore_Unlock ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

    if (Shm->Proc.Features.Uncore_Unlock) {
	CLOCK_ARG uncoreClock = {.NC = 0, .Offset = 0};

	uncoreClock.NC = BOXKEY_UNCORE_CLOCK_OR | CLOCK_MOD_MIN;
	GridCall(PrintRatioFreq(win, CFlop,
				0, "Min", &Shm->Uncore.Boost[UNCORE_BOOST(MIN)],
				1, uncoreClock.sllong,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &Shm->Uncore.Boost[UNCORE_BOOST(MIN)]);

	uncoreClock.NC = BOXKEY_UNCORE_CLOCK_OR | CLOCK_MOD_MAX;
	GridCall(PrintRatioFreq(win, CFlop,
				0, "Max", &Shm->Uncore.Boost[UNCORE_BOOST(MAX)],
				1, uncoreClock.sllong,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &Shm->Uncore.Boost[UNCORE_BOOST(MAX)]);
    } else {
	GridCall(PrintRatioFreq(win, CFlop,
				0, "Min", &Shm->Uncore.Boost[UNCORE_BOOST(MIN)],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &Shm->Uncore.Boost[UNCORE_BOOST(MIN)]);

	GridCall(PrintRatioFreq(win, CFlop,
				0, "Max", &Shm->Uncore.Boost[UNCORE_BOOST(MAX)],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &Shm->Uncore.Boost[UNCORE_BOOST(MAX)]);
    }

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	const size_t len = RSZ(LEVEL) + 1 + 1;
	char *pfx = malloc(len);
      if (pfx != NULL)
      {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"TDP%.*s""%s"" [%3d:%-3d]",
		width - 16 - RSZ(LEVEL), hSpace, RSC(LEVEL).CODE(),
		Shm->Proc.Features.TDP_Cfg_Level,Shm->Proc.Features.TDP_Levels);

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.TDP_Unlock == 1],
		width, 3, "%s%.*s[%7.*s]", RSC(PROGRAMMABLE).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(PROGRAMMABLE), hSpace,
			6, Shm->Proc.Features.TDP_Unlock == 1 ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.TDP_Cfg_Lock == 0],
		width, 3, "%s%.*s[%7.*s]", RSC(CONFIGURATION).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(CONFIGURATION),hSpace,
			6, Shm->Proc.Features.TDP_Cfg_Lock == 1 ?
				RSC(LOCK).CODE() : RSC(UNLOCK).CODE());

	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.TurboActiv_Lock == 0],
		width, 3, "%s%.*s[%7.*s]", RSC(TURBO_ACTIVATION).CODE(),
		width - (OutFunc == NULL ? 15:13)-RSZ(TURBO_ACTIVATION),hSpace,
			6, Shm->Proc.Features.TurboActiv_Lock == 1 ?
				RSC(LOCK).CODE() : RSC(UNLOCK).CODE());

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(TDP) ]
			].FlipFlop[
			!Shm->Cpu[
			Ruler.Top[ BOOST(TDP) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, (char*) RSC(NOMINAL).CODE(),
				&Shm->Cpu[
					Ruler.Top[ BOOST(TDP) ]
					].Boost[ BOOST(TDP) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TDP) );

	snprintf(pfx, len, "%s" "1", RSC(LEVEL).CODE());

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(TDP1) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(TDP1) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, pfx, &Shm->Cpu[
						Ruler.Top[ BOOST(TDP1) ]
						].Boost[ BOOST(TDP1) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TDP1) );

	snprintf(pfx, len, "%s" "2", RSC(LEVEL).CODE());

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(TDP2) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(TDP2) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, pfx, &Shm->Cpu[
						Ruler.Top[ BOOST(TDP2) ]
						].Boost[ BOOST(TDP2) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TDP2) );

	CFlop = &Shm->Cpu[
			Ruler.Top[ BOOST(ACT) ]
			].FlipFlop[
			!Shm->Cpu[ Ruler.Top[ BOOST(ACT) ] ].Toggle
			];

	GridCall(PrintRatioFreq(win, CFlop,
				0, "Turbo", &Shm->Cpu[
						Ruler.Top[ BOOST(ACT) ]
						].Boost[ BOOST(ACT) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(ACT) );

	free(pfx);
      } else {
	REASON_SET(reason, RC_MEM_ERR);
      }
    }
	return (reason);
}

REASON_CODE SysInfoISA(Window *win, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[2][5] = {
		{
	/* [N],[N/N]*/	RSC(SYSINFO_ISA_COND_0_0).ATTR(),
	/* [Y]   */	RSC(SYSINFO_ISA_COND_0_1).ATTR(),
	/* [N/Y] */	RSC(SYSINFO_ISA_COND_0_2).ATTR(),
	/* [Y/N] */	RSC(SYSINFO_ISA_COND_0_3).ATTR(),
	/* [Y/Y] */	RSC(SYSINFO_ISA_COND_0_4).ATTR()
		}, {
			RSC(SYSINFO_ISA_COND_1_0).ATTR(),
			RSC(SYSINFO_ISA_COND_1_1).ATTR(),
			RSC(SYSINFO_ISA_COND_1_2).ATTR(),
			RSC(SYSINFO_ISA_COND_1_3).ATTR(),
			RSC(SYSINFO_ISA_COND_1_4).ATTR()
		}
	};
	const struct ISA_ST {
		unsigned int	*CRC;
		const char	*item, *comm;
		unsigned short	thm[2];
		unsigned short	*cond;
	} ISA[] = \
    {
/* Row Mark */
	{
		NULL,
		" 3DNow!/Ext [%c/%c]",
		" AMD 3DNow! SIMD instructions / 3DNow! Extensions ",
		{
		0,
		2 * ( Shm->Proc.Features.ExtInfo.EDX._3DNow
			|  Shm->Proc.Features.ExtInfo.EDX._3DNowEx )
		+ ( Shm->Proc.Features.ExtInfo.EDX._3DNow
			<< Shm->Proc.Features.ExtInfo.EDX._3DNowEx )
		},
		(unsigned short[])
		{
		Shm->Proc.Features.ExtInfo.EDX._3DNow,
		Shm->Proc.Features.ExtInfo.EDX._3DNowEx
		}
	},
	{
		NULL,
		"          ADX [%c]", " Multi-Precision Add-Carry ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.ADX },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.ADX }
	},
	{
		NULL,
		"          AES [%c]", " Advanced Encryption Standard ",
		{ 0, Shm->Proc.Features.Std.ECX.AES },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.AES },
	},
	{
		NULL,
		"  AVX/AVX2 [%c/%c] ", " Advanced Vector Extensions ",
		{
		1,
		2 * ( Shm->Proc.Features.Std.ECX.AVX
			|  Shm->Proc.Features.ExtFeature.EBX.AVX2 )
		+ ( Shm->Proc.Features.Std.ECX.AVX
			<< Shm->Proc.Features.ExtFeature.EBX.AVX2 )
		},
		(unsigned short[])
		{
		Shm->Proc.Features.Std.ECX.AVX,
		Shm->Proc.Features.ExtFeature.EBX.AVX2
		}
	},
/* Row Mark */
	{
		NULL,
		" AVX512-F     [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EBX.AVX_512F },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX_512F },
	},
	{
		NULL,
		"    AVX512-DQ [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EBX.AVX_512DQ },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX_512DQ },
	},
	{
		NULL,
		"  AVX512-IFMA [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EBX.AVX512_IFMA },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX512_IFMA },
	},
	{
		NULL,
		"   AVX512-PF [%c] ", NULL,
		{ 1, Shm->Proc.Features.ExtFeature.EBX.AVX512PF },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX512PF },
	},
/* Row Mark */
	{
		NULL,
		" AVX512-ER    [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EBX.AVX512ER },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX512ER },
	},
	{
		NULL,
		"    AVX512-CD [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EBX.AVX512CD },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX512CD },
	},
	{
		NULL,
		"    AVX512-BW [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EBX.AVX512BW },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX512BW },
	},
	{
		NULL,
		"   AVX512-VL [%c] ", NULL,
		{ 1, Shm->Proc.Features.ExtFeature.EBX.AVX512VL },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.AVX512VL },
	},
/* Row Mark */
	{
		NULL,
		" AVX512-VBMI  [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.ECX.AVX512_VBMI },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.ECX.AVX512_VBMI },
	},
	{
		NULL,
		" AVX512-VBMI2 [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.ECX.AVX512_VBMI2 },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.ECX.AVX512_VBMI2 },
	},
	{
		NULL,
		"  AVX512-VNMI [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.ECX.AVX512_VNNI },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.ECX.AVX512_VNNI },
	},
	{
		NULL,
		"  AVX512-ALG [%c] ", NULL,
		{ 1, Shm->Proc.Features.ExtFeature.ECX.AVX512_BITALG },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.ECX.AVX512_BITALG },
	},
/* Row Mark */
	{
		NULL,
		" AVX512-VPOP  [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.ECX.AVX512_VPOPCNTDQ },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.ECX.AVX512_VPOPCNTDQ },
	},
	{
		NULL,
		" AVX512-VNNIW [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EDX.AVX512_4VNNIW },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EDX.AVX512_4VNNIW },
	},
	{
		NULL,
		" AVX512-FMAPS [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature.EDX.AVX512_4FMAPS },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EDX.AVX512_4FMAPS },
	},
	{
		NULL,
		" AVX512-VP2I [%c] ", NULL,
		{ 1, Shm->Proc.Features.ExtFeature.EDX.AVX512_VP2INTER },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EDX.AVX512_VP2INTER },
	},
/* Row Mark */
	{
		NULL,
		" AVX512-BF16  [%c]", NULL,
		{ 0, Shm->Proc.Features.ExtFeature_Leaf1.EAX.AVX512_BF16 },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature_Leaf1.EAX.AVX512_BF16 },
	},
	{
		NULL,
		"  BMI1/BMI2 [%c/%c]", " Bit Manipulation Instructions ",
		{ 0, 2 * ( Shm->Proc.Features.ExtFeature.EBX.BMI1
				|  Shm->Proc.Features.ExtFeature.EBX.BMI2 )
			+ ( Shm->Proc.Features.ExtFeature.EBX.BMI1
				<< Shm->Proc.Features.ExtFeature.EBX.BMI2) },
		(unsigned short[])
		{
		Shm->Proc.Features.ExtFeature.EBX.BMI1,
		Shm->Proc.Features.ExtFeature.EBX.BMI2
		},
	},
	{
		NULL,
		"         CLWB [%c]", " Cache Line Write Back ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.CLWB },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.CLWB },
	},
	{
		NULL,
		" CLFLUSH/O [%c/%c] ", " Flush Cache Line / CLFLUSH Optimized ",
		{ 1, 2 * ( Shm->Proc.Features.Std.EDX.CLFLUSH
			|  Shm->Proc.Features.ExtFeature.EBX.CLFLUSHOPT )
			+ ( Shm->Proc.Features.Std.EDX.CLFLUSH
			<< Shm->Proc.Features.ExtFeature.EBX.CLFLUSHOPT ) },
		(unsigned short[])
		{
		Shm->Proc.Features.Std.EDX.CLFLUSH,
		Shm->Proc.Features.ExtFeature.EBX.CLFLUSHOPT
		},
	},
/* Row Mark */
	{
		NULL,
		" CLAC-STAC    [%c]",
		" Clear AC - Set AC Flag in EFLAGS Register ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.SMAP_CLAC_STAC },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.SMAP_CLAC_STAC },
	},
	{
		NULL,
		"         CMOV [%c]", " Conditional Move instructions ",
		{ 0, Shm->Proc.Features.Std.EDX.CMOV },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.CMOV },
	},
	{
		NULL,
		"    CMPXCHG8B [%c]", " Compare and Exchange 8 Bytes ",
		{ 0, Shm->Proc.Features.Std.EDX.CMPXCHG8 },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.CMPXCHG8 },
	},
	{
		NULL,
		"  CMPXCHG16B [%c] ", " Compare and Exchange 16 Bytes ",
		{ 1, Shm->Proc.Features.Std.ECX.CMPXCHG16 },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.CMPXCHG16 },
	},
/* Row Mark */
	{
		NULL,
		" F16C         [%c]",
		" 16-bit floating-point conversion instructions ",
		{ 0, Shm->Proc.Features.Std.ECX.F16C },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.F16C },
	},
	{
		NULL,
		"          FPU [%c]", " Floating Point Unit On-Chip ",
		{ 0, Shm->Proc.Features.Std.EDX.FPU },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.FPU },
	},
	{
		NULL,
		"         FXSR [%c]", " FXSAVE and FXRSTOR instructions ",
		{ 0, Shm->Proc.Features.Std.EDX.FXSR },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.FXSR },
	},
	{
		NULL,
		"   LAHF-SAHF [%c] ",
		" Load-Store Status Flags into AH register ",
		{ 1, Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF },
		(unsigned short[])
		{ Shm->Proc.Features.ExtInfo.ECX.LAHFSAHF },
	},
/* Row Mark */
	{
		NULL,
		" MMX/Ext    [%c/%c]", " MultiMedia eXtensions / Extended MMX ",
		{ 0, 2 * ( Shm->Proc.Features.Std.EDX.MMX
				|  Shm->Proc.Features.ExtInfo.EDX.MMX_Ext )
			+ ( Shm->Proc.Features.Std.EDX.MMX
				<< Shm->Proc.Features.ExtInfo.EDX.MMX_Ext ) },
		(unsigned short[])
		{
		Shm->Proc.Features.Std.EDX.MMX,
		Shm->Proc.Features.ExtInfo.EDX.MMX_Ext
		},
	},
	{
		NULL,
		" MON/MWAITX [%c/%c]", " Monitor Wait / MWAIT eXtensions ",
		{ 0, 2 * ( Shm->Proc.Features.Std.ECX.MONITOR
				|  Shm->Proc.Features.ExtInfo.ECX.MWaitExt )
			+ ( Shm->Proc.Features.Std.ECX.MONITOR
				<< Shm->Proc.Features.ExtInfo.ECX.MWaitExt ) },
		(unsigned short[])
		{
		Shm->Proc.Features.Std.ECX.MONITOR,
		Shm->Proc.Features.ExtInfo.ECX.MWaitExt
		},
	},
	{
		NULL,
		"        MOVBE [%c]", " Move Data After Swapping Bytes ",
		{ 0, Shm->Proc.Features.Std.ECX.MOVBE },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.MOVBE },
	},
	{
		NULL,
		"   PCLMULQDQ [%c] ", " Carryless Multiplication Quadword ",
		{ 1, Shm->Proc.Features.Std.ECX.PCLMULDQ },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.PCLMULDQ },
	},
/* Row Mark */
	{
		NULL,
		" POPCNT       [%c]", " Count of Number of Bits Set to 1 ",
		{ 0, Shm->Proc.Features.Std.ECX.POPCNT },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.POPCNT },
	},
	{
		NULL,
		"       RDRAND [%c]", " Read Random Number ",
		{ 0, Shm->Proc.Features.Std.ECX.RDRAND },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.RDRAND },
	},
	{
		NULL,
		"       RDSEED [%c]", " Read Random SEED ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.RDSEED },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.RDSEED },
	},
	{
		NULL,
		"      RDTSCP [%c] ",
		" Read Time-Stamp Counter and Processor ID ",
		{ 1, Shm->Proc.Features.ExtInfo.EDX.RDTSCP },
		(unsigned short[])
		{ Shm->Proc.Features.ExtInfo.EDX.RDTSCP },
	},
/* Row Mark */
	{
		NULL,
		" SEP          [%c]", " SYSENTER and SYSEXIT instructions ",
		{ 0, Shm->Proc.Features.Std.EDX.SEP },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.SEP },
	},
	{
		NULL,
		"          SHA [%c]", " Secure Hash Algorithms extensions ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.SHA },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.SHA },
	},
	{
		NULL,
		"          SSE [%c]", " Streaming SIMD Extensions ",
		{ 0, Shm->Proc.Features.Std.EDX.SSE },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.SSE },
	},
	{
		NULL,
		"        SSE2 [%c] ", " Streaming SIMD Extensions 2 ",
		{ 1, Shm->Proc.Features.Std.EDX.SSE2 },
		(unsigned short[])
		{ Shm->Proc.Features.Std.EDX.SSE2 },
	},
/* Row Mark */
	{
		NULL,
		" SSE3         [%c]", " Streaming SIMD Extensions 3 ",
		{ 0, Shm->Proc.Features.Std.ECX.SSE3 },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.SSE3 },
	},
	{
		NULL,
		"        SSSE3 [%c]",
		" Supplemental Streaming SIMD Extensions 3 ",
		{ 0, Shm->Proc.Features.Std.ECX.SSSE3 },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.SSSE3 },
	},
	{
		NULL,
		"  SSE4.1/4A [%c/%c]",
		" Streaming SIMD Extensions 4.1 / AMD SSE 4A ",
		{ 0, 2 * ( Shm->Proc.Features.Std.ECX.SSE41
				|  Shm->Proc.Features.ExtInfo.ECX.SSE4A )
			+ ( Shm->Proc.Features.Std.ECX.SSE41
				<< Shm->Proc.Features.ExtInfo.ECX.SSE4A ) },
		(unsigned short[])
		{
		Shm->Proc.Features.Std.ECX.SSE41,
		Shm->Proc.Features.ExtInfo.ECX.SSE4A
		},
	},
	{
		NULL,
		"      SSE4.2 [%c] ", " Streaming SIMD Extensions 4.2 ",
		{ 1, Shm->Proc.Features.Std.ECX.SSE42 },
		(unsigned short[])
		{ Shm->Proc.Features.Std.ECX.SSE42 },
	},
/* Row Mark */
	{
		NULL,
		" SERIALIZE    [%c]", " Serialize instruction ",
		{ 0, Shm->Proc.Features.ExtFeature.EDX.SERIALIZE },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EDX.SERIALIZE },
	},
	{
		NULL,
		"      SYSCALL [%c]",
		" Fast System Call and SYSRET - Return From SYSCALL ",
		{ 0, Shm->Proc.Features.ExtInfo.EDX.SYSCALL },
		(unsigned short[])
		{ Shm->Proc.Features.ExtInfo.EDX.SYSCALL },
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		"        RDPID [%c]", " Read Processor ID ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.RDPID },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.RDPID },
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		"        UMIP [%c] ", " User Mode Instruction Prevention ",
		{ 1, Shm->Proc.Features.ExtFeature.EBX.SGX_UMIP },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.SGX_UMIP },
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		"          SGX [%c]", " Intel Software Guard eXtensions ",
		{ 0, Shm->Proc.Features.ExtFeature.EBX.SGX_UMIP },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.EBX.SGX_UMIP },
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		"       RDPID [%c] ", " Read Processor ID ",
		{ 1, Shm->Proc.Features.ExtFeature.ECX.RDPID },
		(unsigned short[])
		{ Shm->Proc.Features.ExtFeature.ECX.RDPID },
	}
    };

	CUINT cells_per_line = win->matrix.size.wth, *nl = &cells_per_line;

	size_t idx;
    for (idx = 0; idx < sizeof(ISA) / sizeof(struct ISA_ST); idx++)
    {
	unsigned short capable = 0;
	if (ISA[idx].CRC == NULL) {
		capable = 1;
	} else {
		unsigned int *CRC;
	    for (CRC = ISA[idx].CRC; (*CRC) != 0 && capable == 0; CRC++)
	    {
		if ( (*CRC) == Shm->Proc.Features.Info.Vendor.CRC ) {
			capable = 1;
		}
	    }
	}
	if (capable) {
		GridHover(PRT(ISA, attrib[ ISA[idx].thm[0] ][ ISA[idx].thm[1] ],
				ISA[idx].item,
				ISA[idx].cond[0] ? 'Y' : 'N',
				ISA[idx].cond[1] ? 'Y' : 'N' ),
		ISA[idx].comm );
	}
    }
	return (reason);
}

REASON_CODE SysInfoFeatures(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attr_Feat[4] = {
		RSC(SYSINFO_FEATURES_COND0).ATTR(),
		RSC(SYSINFO_FEATURES_COND1).ATTR(),
		RSC(SYSINFO_FEATURES_COND2).ATTR(),
		RSC(SYSINFO_FEATURES_COND3).ATTR()
	};
	ATTRIBUTE *attr_TSC[3] = {
		RSC(SYSINFO_FEATURES_COND0).ATTR(),
		RSC(SYSINFO_FEATURES_COND1).ATTR(),
		RSC(SYSINFO_FEATURES_COND4).ATTR()
	};
	const ASCII *powered[] = {
		RSC(MISSING).CODE(),
		RSC(PRESENT).CODE()
	};
	const ASCII *code_TSC[] = {
		RSC(MISSING).CODE(),
		RSC(VARIANT).CODE(),
		RSC(INVARIANT).CODE()
	};
	const ASCII *x2APIC[] = {
		RSC(MISSING).CODE(),
		(ASCII*) "  xAPIC",
		(ASCII*) " x2APIC"
	};
	const ASCII *MECH[] = {
		RSC(MISSING).CODE(),
		RSC(PRESENT).CODE(),
		RSC(DISABLE).CODE(),
		RSC(ENABLE).CODE()
	};
	const struct FEAT_ST {
		unsigned int		*CRC;
		const unsigned short	cond;
		ATTRIBUTE		**attrib;
		const int		tab;
		char			*item;
		const ASCII		*code;
		const CUINT		spaces;
		const ASCII		**state;
	} FEAT[] = \
    {
/* Section Mark */
	{
		NULL,
		Shm->Proc.Features.ExtInfo.EDX.PG_1GB == 1,
		attr_Feat,
		2, "%s%.*s1GB-PAGES   [%7s]", RSC(FEATURES_1GB_PAGES).CODE(),
		width - 24 - RSZ(FEATURES_1GB_PAGES),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Features.AdvPower.EDX._100MHz == 1,
		attr_Feat,
		2, "%s%.*s100MHzSteps   [%7s]", RSC(FEATURES_100MHZ).CODE(),
		width - 26 - RSZ(FEATURES_100MHZ),
		NULL
	},
	{
		NULL,
		(Shm->Proc.Features.Std.EDX.ACPI == 1)
		|| (Shm->Proc.Features.AdvPower.EDX.HwPstate == 1),
		attr_Feat,
		2, "%s%.*sACPI   [%7s]", RSC(FEATURES_ACPI).CODE(),
		width - 19 - RSZ(FEATURES_ACPI),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.APIC == 1,
		attr_Feat,
		2, "%s%.*sAPIC   [%7s]", RSC(FEATURES_APIC).CODE(),
		width - 19 - RSZ(FEATURES_APIC),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtInfo.ECX.MP_Mode == 1,
		attr_Feat,
		2, "%s%.*sCMP Legacy   [%7s]", RSC(FEATURES_CORE_MP).CODE(),
		width - 25 - RSZ(FEATURES_CORE_MP),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.CNXT_ID == 1,
		attr_Feat,
		2, "%s%.*sCNXT-ID   [%7s]", RSC(FEATURES_CNXT_ID).CODE(),
		width - 22 - RSZ(FEATURES_CNXT_ID),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.DCA == 1,
		attr_Feat,
		2, "%s%.*sDCA   [%7s]", RSC(FEATURES_DCA).CODE(),
		width - 18 - RSZ(FEATURES_DCA),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.DE == 1,
		attr_Feat,
		2, "%s%.*sDE   [%7s]", RSC(FEATURES_DE).CODE(),
		width - 17 - RSZ(FEATURES_DE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.DS_PEBS == 1,
		attr_Feat,
		2, "%s%.*sDS, PEBS   [%7s]", RSC(FEATURES_DS_PEBS).CODE(),
		width - 23 - RSZ(FEATURES_DS_PEBS),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.DS_CPL == 1,
		attr_Feat,
		2, "%s%.*sDS-CPL   [%7s]", RSC(FEATURES_DS_CPL).CODE(),
		width - 21 - RSZ(FEATURES_DS_CPL),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.DTES64 == 1,
		attr_Feat,
		2, "%s%.*sDTES64   [%7s]", RSC(FEATURES_DTES_64).CODE(),
		width - 21 - RSZ(FEATURES_DTES_64),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.FastStrings == 1,
		attr_Feat,
		2, "%s%.*sFast-Strings   [%7s]", RSC(FEATURES_FAST_STR).CODE(),
		width - 27 - RSZ(FEATURES_FAST_STR),
		NULL
	},
	{
		NULL,
		(Shm->Proc.Features.Std.ECX.FMA == 1)
		|| (Shm->Proc.Features.ExtInfo.ECX.FMA4 == 1),
		attr_Feat,
		2, "%s%.*sFMA | FMA4   [%7s]", RSC(FEATURES_FMA).CODE(),
		width - 25 - RSZ(FEATURES_FMA),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.HLE == 1,
		attr_Feat,
		2, "%s%.*sHLE   [%7s]", RSC(FEATURES_HLE).CODE(),
		width - 18 - RSZ(FEATURES_HLE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtInfo.ECX.IBS == 1,
		attr_Feat,
		2, "%s%.*sIBS   [%7s]", RSC(FEATURES_IBS).CODE(),
		width - 18 - RSZ(FEATURES_IBS),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtInfo.EDX.IA64 == 1,
		attr_Feat,
		2, "%s%.*sIA64 | LM   [%7s]", RSC(FEATURES_LM).CODE(),
		width - 24 - RSZ(FEATURES_LM),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtInfo.ECX.LWP == 1,
		attr_Feat,
		2, "%s%.*sLWP   [%7s]", RSC(FEATURES_LWP).CODE(),
		width - 18 - RSZ(FEATURES_LWP),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.MCA == 1,
		attr_Feat,
		2, "%s%.*sMCA   [%7s]", RSC(FEATURES_MCA).CODE(),
		width - 18 - RSZ(FEATURES_MCA),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.MPX == 1,
		attr_Feat,
		2, "%s%.*sMPX   [%7s]", RSC(FEATURES_MPX).CODE(),
		width - 18 - RSZ(FEATURES_MPX),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.MSR == 1,
		attr_Feat,
		2, "%s%.*sMSR   [%7s]", RSC(FEATURES_MSR).CODE(),
		width - 18 - RSZ(FEATURES_MSR),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.MTRR == 1,
		attr_Feat,
		2, "%s%.*sMTRR   [%7s]", RSC(FEATURES_MTRR).CODE(),
		width - 19 - RSZ(FEATURES_MTRR),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Features.ExtInfo.EDX.NX == 1,
		attr_Feat,
		2, "%s%.*sNX   [%7s]", RSC(FEATURES_NX).CODE(),
		width - 17 - RSZ(FEATURES_NX),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.OSXSAVE == 1,
		attr_Feat,
		2, "%s%.*sOSXSAVE   [%7s]", RSC(FEATURES_OSXSAVE).CODE(),
		width - 22 - RSZ(FEATURES_OSXSAVE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PAE == 1,
		attr_Feat,
		2, "%s%.*sPAE   [%7s]", RSC(FEATURES_PAE).CODE(),
		width - 18 - RSZ(FEATURES_PAE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PAT == 1,
		attr_Feat,
		2, "%s%.*sPAT   [%7s]", RSC(FEATURES_PAT).CODE(),
		width - 18 - RSZ(FEATURES_PAT),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PBE == 1,
		attr_Feat,
		2, "%s%.*sPBE   [%7s]", RSC(FEATURES_PBE).CODE(),
		width - 18 - RSZ(FEATURES_PBE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.PCID == 1,
		attr_Feat,
		2, "%s%.*sPCID   [%7s]", RSC(FEATURES_PCID).CODE(),
		width - 19 - RSZ(FEATURES_PCID),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.PDCM == 1,
		attr_Feat,
		2, "%s%.*sPDCM   [%7s]", RSC(FEATURES_PDCM).CODE(),
		width - 19 - RSZ(FEATURES_PDCM),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PGE == 1,
		attr_Feat,
		2, "%s%.*sPGE   [%7s]", RSC(FEATURES_PGE).CODE(),
		width - 18 - RSZ(FEATURES_PGE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PSE == 1,
		attr_Feat,
		2, "%s%.*sPSE   [%7s]", RSC(FEATURES_PSE).CODE(),
		width - 18 - RSZ(FEATURES_PSE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PSE36 == 1,
		attr_Feat,
		2, "%s%.*sPSE36   [%7s]", RSC(FEATURES_PSE36).CODE(),
		width - 20 - RSZ(FEATURES_PSE36),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.PSN == 1,
		attr_Feat,
		2, "%s%.*sPSN   [%7s]", RSC(FEATURES_PSN).CODE(),
		width - 18 - RSZ(FEATURES_PSN),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.PQE == 1,
		attr_Feat,
		2, "%s%.*sRDT-A   [%7s]", RSC(FEATURES_RDT_PQE).CODE(),
		width - 20 - RSZ(FEATURES_RDT_PQE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.PQM == 1,
		attr_Feat,
		2, "%s%.*sRDT-M   [%7s]", RSC(FEATURES_RDT_PQM).CODE(),
		width - 20 - RSZ(FEATURES_RDT_PQM),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.RTM == 1,
		attr_Feat,
		2, "%s%.*sRTM   [%7s]", RSC(FEATURES_RTM).CODE(),
		width - 18 - RSZ(FEATURES_RTM),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.SMX == 1,
		attr_Feat,
		2, "%s%.*sSMX   [%7s]", RSC(FEATURES_SMX).CODE(),
		width - 18 - RSZ(FEATURES_SMX),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.SS == 1,
		attr_Feat,
		2, "%s%.*sSS   [%7s]", RSC(FEATURES_SELF_SNOOP).CODE(),
		width - 17 - RSZ(FEATURES_SELF_SNOOP),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.SMAP_CLAC_STAC == 1,
		attr_Feat,
		2, "%s%.*sSMAP   [%7s]", RSC(FEATURES_SMAP).CODE(),
		width - 19 - RSZ(FEATURES_SMAP),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EBX.SMEP == 1,
		attr_Feat,
		2, "%s%.*sSMEP   [%7s]", RSC(FEATURES_SMEP).CODE(),
		width - 19 - RSZ(FEATURES_SMEP),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.InvariantTSC,
		attr_TSC,
		2, "%s%.*sTSC [%9s]", RSC(FEATURES_TSC).CODE(),
		width - 18 - RSZ(FEATURES_TSC),
		code_TSC
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.TSCDEAD == 1,
		attr_Feat,
		2, "%s%.*sTSC-DEADLINE   [%7s]",RSC(FEATURES_TSC_DEADLN).CODE(),
		width - 27 - RSZ(FEATURES_TSC_DEADLN),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EDX.TSX_FORCE_ABORT == 1,
		attr_Feat,
		2, "%s%.*sTSX-ABORT   [%7s]", RSC(FEATURES_TSXABORT).CODE(),
		width - 24 - RSZ(FEATURES_TSXABORT),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.ExtFeature.EDX.TSXLDTRK == 1,
		attr_Feat,
		2, "%s%.*sTSX-LDTRK   [%7s]", RSC(FEATURES_TSXLDTRK).CODE(),
		width - 24 - RSZ(FEATURES_TSXLDTRK),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Features.ExtFeature.EBX.SGX_UMIP == 1,
		attr_Feat,
		2, "%s%.*sUMIP   [%7s]", RSC(FEATURES_UMIP).CODE(),
		width - 19 - RSZ(FEATURES_UMIP),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Features.ExtFeature.ECX.UMIP == 1,
		attr_Feat,
		2, "%s%.*sUMIP   [%7s]", RSC(FEATURES_UMIP).CODE(),
		width - 19 - RSZ(FEATURES_UMIP),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.EDX.VME == 1,
		attr_Feat,
		2, "%s%.*sVME   [%7s]", RSC(FEATURES_VME).CODE(),
		width - 18 - RSZ(FEATURES_VME),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.VMX == 1,
		attr_Feat,
		2, "%s%.*sVMX   [%7s]", RSC(FEATURES_VMX).CODE(),
		width - 18 - RSZ(FEATURES_VMX),
		NULL
	},
	{
		NULL,
		Shm->Cpu[Shm->Proc.Service.Core].Topology.MP.x2APIC,
		attr_Feat,
		2, "%s%.*sx2APIC   [%7s]", RSC(FEATURES_X2APIC).CODE(),
		width - 21 - RSZ(FEATURES_X2APIC),
		x2APIC
	},
	{
		(unsigned int[]) { CRC_INTEL, 0},
		Shm->Proc.Features.ExtInfo.EDX.XD_Bit == 1,
		attr_Feat,
		2, "%s%.*sXD-Bit   [%7s]", RSC(FEATURES_XD_BIT).CODE(),
		width - 21 - RSZ(FEATURES_XD_BIT),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.XSAVE == 1,
		attr_Feat,
		2, "%s%.*sXSAVE   [%7s]", RSC(FEATURES_XSAVE).CODE(),
		width - 20 - RSZ(FEATURES_XSAVE),
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.xTPR == 1,
		attr_Feat,
		2, "%s%.*sxTPR   [%7s]", RSC(FEATURES_XTPR).CODE(),
		width - 19 - RSZ(FEATURES_XTPR),
		NULL
	},
/* Section Mark */
	{
		NULL,
		0,
		attr_Feat,
		0, "%s", RSC(FEAT_SECTION_MECH).CODE(),
		0,
		NULL
	},
	{
		NULL,
		Shm->Proc.Mechanisms.IBRS,
		attr_Feat,
		2, "%s%.*sIBRS   [%7s]", RSC(MECH_IBRS).CODE(),
		width - 19 - RSZ(MECH_IBRS),
		MECH
	},
	{
		NULL,
		( Shm->Proc.Features.ExtFeature.EDX.IBRS_IBPB_Cap == 1 )
		|| ( Shm->Proc.Features.leaf80000008.EBX.IBPB == 1 ),
		attr_Feat,
		2, "%s%.*sIBPB   [%7s]", RSC(MECH_IBPB).CODE(),
		width - 19 - RSZ(MECH_IBPB),
		MECH
	},
	{
		NULL,
		Shm->Proc.Mechanisms.STIBP,
		attr_Feat,
		2, "%s%.*sSTIBP   [%7s]", RSC(MECH_STIBP).CODE(),
		width - 20 - RSZ(MECH_STIBP),
		MECH
	},
	{
		NULL,
		Shm->Proc.Mechanisms.SSBD,
		attr_Feat,
		2, "%s%.*sSSBD   [%7s]", RSC(MECH_SSBD).CODE(),
		width - 19 - RSZ(MECH_SSBD),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Features.ExtFeature.EDX.L1D_FLUSH_Cap == 1,
		attr_Feat,
		2, "%s%.*sL1D-FLUSH   [%7s]", RSC(MECH_L1D_FLUSH).CODE(),
		width - 24 - RSZ(MECH_L1D_FLUSH),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.L1DFL_VMENTRY_NO,
		attr_Feat,
		2, "%s%.*sL1DFL_VMENTRY_NO   [%7s]",
		RSC(MECH_L1DFL_VMENTRY_NO).CODE(),
		width - 31 - RSZ(MECH_L1DFL_VMENTRY_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Features.ExtFeature.EDX.MD_CLEAR_Cap == 1,
		attr_Feat,
		2, "%s%.*sMD-CLEAR   [%7s]", RSC(MECH_MD_CLEAR).CODE(),
		width - 23 - RSZ(MECH_MD_CLEAR),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.RDCL_NO,
		attr_Feat,
		2, "%s%.*sRDCL_NO   [%7s]", RSC(MECH_RDCL_NO).CODE(),
		width - 22 - RSZ(MECH_RDCL_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.IBRS_ALL,
		attr_Feat,
		2, "%s%.*sIBRS_ALL   [%7s]", RSC(MECH_IBRS_ALL).CODE(),
		width - 23 - RSZ(MECH_IBRS_ALL),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.RSBA,
		attr_Feat,
		2, "%s%.*sRSBA   [%7s]", RSC(MECH_RSBA).CODE(),
		width - 19 - RSZ(MECH_RSBA),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.SSB_NO,
		attr_Feat,
		2, "%s%.*sSSB_NO   [%7s]", RSC(MECH_SSB_NO).CODE(),
		width - 21 - RSZ(MECH_SSB_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.MDS_NO,
		attr_Feat,
		2, "%s%.*sMDS_NO   [%7s]", RSC(MECH_MDS_NO).CODE(),
		width - 21 - RSZ(MECH_MDS_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.TAA_NO,
		attr_Feat,
		2, "%s%.*sTAA_NO   [%7s]", RSC(MECH_TAA_NO).CODE(),
		width - 21 - RSZ(MECH_TAA_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.PSCHANGE_MC_NO,
		attr_Feat,
		2, "%s%.*sPSCHANGE_MC_NO   [%7s]",
		RSC(MECH_PSCHANGE_MC_NO).CODE(),
		width - 29 - RSZ(MECH_PSCHANGE_MC_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Mechanisms.SPLA,
		attr_Feat,
		2, "%s%.*sSPLA   [%7s]", RSC(MECH_SPLA).CODE(),
		width - 19 - RSZ(MECH_SPLA),
		MECH
	}
    };
	size_t idx;
    for (idx = 0; idx < sizeof(FEAT) / sizeof(struct FEAT_ST); idx++)
    {
	unsigned short capable = 0;
	if (FEAT[idx].CRC == NULL) {
		capable = 1;
	} else {
		unsigned int *CRC;
	    for (CRC = FEAT[idx].CRC; (*CRC) != 0 && capable == 0; CRC++)
	    {
		if ( (*CRC) == Shm->Proc.Features.Info.Vendor.CRC ) {
			capable = 1;
		}
	    }
	}
	if (capable)
	{
		PUT(	SCANKEY_NULL,
			FEAT[idx].attrib[ FEAT[idx].cond ],
			width,	FEAT[idx].tab,
			FEAT[idx].item, FEAT[idx].code,
			FEAT[idx].spaces, hSpace,
			(FEAT[idx].state == NULL) ? powered[ FEAT[idx].cond ]
			: FEAT[idx].state[ FEAT[idx].cond ] );
	}
    }
	return (reason);
}

void TechUpdate(TGrid *grid,	const int unsigned bix, const signed int pos,
				const size_t len, const char *item)
{
	ATTRIBUTE *attrib[2] = {
		RSC(SYSINFO_TECH_COND0).ATTR(),
		RSC(SYSINFO_TECH_COND1).ATTR()
	};
	memcpy(&grid->cell.attr[pos], &attrib[bix][pos], len);
	memcpy(&grid->cell.item[pos], item, len);
}

void L1_HW_Prefetch_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.L1_HW_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void L1_HW_IP_Prefetch_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.L1_HW_IP_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void L2_HW_Prefetch_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.L2_HW_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void L2_HW_CL_Prefetch_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.L2_HW_CL_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void SpeedStepUpdate(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.EIST == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void IDA_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Features.Power.EAX.TurboIDA == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void TurboUpdate(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.Turbo == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void Race2HaltUpdate(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Features.R2H_Disable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

char *Hypervisor[HYPERVISORS] = {
	[HYPERV_NONE]	= "          ",
	[BARE_METAL]	= "Bare-Metal",
	[HYPERV_XEN]	= "       Xen",
	[HYPERV_KVM]	= "       KVM",
	[HYPERV_VBOX]	= "VirtualBox",
	[HYPERV_KBOX]	= "  KVM/VBox",
	[HYPERV_VMWARE] = "    VMware",
	[HYPERV_HYPERV] = "MS Hyper-V"
};

REASON_CODE SysInfoTech(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[2] = {
		RSC(SYSINFO_TECH_COND0).ATTR(),
		RSC(SYSINFO_TECH_COND1).ATTR()
	};
	const struct TECH_ST {
		unsigned int		*CRC;
		const unsigned short	cond;
		const int		tab;
		char			*item;
		const ASCII		*code;
		const CUINT		spaces;
		const char		*context;
		const unsigned long long shortkey;
		void			(*Update)(struct _Grid*, DATA_TYPE);
	} TECH[] = \
    {
	{
		(unsigned int[]) { CRC_INTEL, CRC_AMD, CRC_HYGON, 0 },
		0,
		2, "%s%.*s",
		RSC(TECHNOLOGIES_DCU).CODE(),
		width - 3 - RSZ(TECHNOLOGIES_DCU),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Technology.L1_HW_Prefetch,
		3, "%s%.*sL1 HW   <%3s>",
		RSC(TECH_L1_HW_PREFETCH).CODE(),
		width - (OutFunc ? 17 : 19) - RSZ(TECH_L1_HW_PREFETCH),
		NULL,
		BOXKEY_L1_HW_PREFETCH,
		L1_HW_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.L1_HW_IP_Prefetch,
		3, "%s%.*sL1 HW IP   <%3s>",
		RSC(TECH_L1_HW_IP_PREFETCH).CODE(),
		width - (OutFunc ? 20 : 22) - RSZ(TECH_L1_HW_IP_PREFETCH),
		NULL,
		BOXKEY_L1_HW_IP_PREFETCH,
		L1_HW_IP_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Technology.L2_HW_Prefetch,
		3, "%s%.*sL2 HW   <%3s>",
		RSC(TECH_L2_HW_PREFETCH).CODE(),
		width - (OutFunc ? 17 : 19) - RSZ(TECH_L2_HW_PREFETCH),
		NULL,
		BOXKEY_L2_HW_PREFETCH,
		L2_HW_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.L2_HW_CL_Prefetch,
		3, "%s%.*sL2 HW CL   <%3s>",
		RSC(TECH_L2_HW_CL_PREFETCH).CODE(),
		width - (OutFunc ? 20 : 22) - RSZ(TECH_L2_HW_CL_PREFETCH),
		NULL,
		BOXKEY_L2_HW_CL_PREFETCH,
		L2_HW_CL_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.SMM == 1,
		2, "%s%.*sSMM-Dual   [%3s]",
		RSC(TECHNOLOGIES_SMM).CODE(),
		width - 19 - RSZ(TECHNOLOGIES_SMM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Features.HyperThreading == 1,
		2, "%s%.*sHTT   [%3s]",
		RSC(TECHNOLOGIES_HTT).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_HTT),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.EIST == 1,
		2, "%s%.*sEIST   <%3s>",
		RSC(TECHNOLOGIES_EIST).CODE(),
		width - 15 - RSZ(TECHNOLOGIES_EIST),
		NULL,
		BOXKEY_EIST,
		SpeedStepUpdate
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Features.Power.EAX.TurboIDA == 1,
		2, "%s%.*sIDA   [%3s]",
		RSC(TECHNOLOGIES_IDA).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_IDA),
		NULL,
		SCANKEY_NULL,
		IDA_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.Turbo == 1,
		2, "%s%.*sTURBO   <%3s>",
		RSC(TECHNOLOGIES_TURBO).CODE(),
		width - 16 - RSZ(TECHNOLOGIES_TURBO),
		NULL,
		BOXKEY_TURBO,
		TurboUpdate
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Features.R2H_Disable == 1,
		2, "%s%.*sR2H   <%3s>",
		RSC(TECHNOLOGIES_R2H).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_R2H),
		NULL,
		BOXKEY_R2H,
		Race2HaltUpdate
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.VM == 1,
		2, "%s%.*sVMX   [%3s]",
		RSC(TECHNOLOGIES_VM).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_VM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		Shm->Proc.Technology.IOMMU == 1,
		3, "%s%.*sVT-d   [%3s]",
		RSC(TECHNOLOGIES_IOMMU).CODE(),
		width - (OutFunc ? 16 : 18) - RSZ(TECHNOLOGIES_IOMMU),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Technology.SMM == 1,
		2, "%s%.*sSMM-Lock   [%3s]",
		RSC(TECHNOLOGIES_SMM).CODE(),
		width - 19 - RSZ(TECHNOLOGIES_SMM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Features.HyperThreading == 1,
		2, "%s%.*sSMT   [%3s]",
		RSC(TECHNOLOGIES_SMT).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_SMT),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.PowerNow == 0b11,	/*	VID + FID	*/
		2, "%s%.*sCnQ   [%3s]",
		RSC(TECHNOLOGIES_CNQ).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_CNQ),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Cpu[Shm->Proc.Service.Core].Query.CStateBaseAddr != 0,
		2, "%s%.*sCCx   [%3s]",
		RSC(PERF_MON_CORE_CSTATE).CODE(),
		width - 14 - RSZ(PERF_MON_CORE_CSTATE),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Technology.Turbo == 1,
		2, "%s%.*sCPB   <%3s>",
		RSC(TECHNOLOGIES_CPB).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_CPB),
		NULL,
		BOXKEY_TURBO,
		TurboUpdate
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Technology.VM == 1,
		2, "%s%.*sSVM   [%3s]",
		RSC(TECHNOLOGIES_VM).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_VM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		Shm->Proc.Technology.IOMMU == 1,
		3, "%s%.*sAMD-V   [%3s]",
		RSC(TECHNOLOGIES_IOMMU).CODE(),
		width - (OutFunc? 17 : 19) - RSZ(TECHNOLOGIES_IOMMU),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		NULL,
		Shm->Proc.Features.Std.ECX.Hyperv == 1,
		3, "%s%.*s""%10s   [%3s]",
		RSC(TECHNOLOGIES_HYPERV).CODE(),
		width - (OutFunc? 22 : 24) - RSZ(TECHNOLOGIES_HYPERV),
		Hypervisor[Shm->Proc.HypervisorID],
		SCANKEY_NULL,
		NULL
	},
	{
		NULL,
		0,
		3, "%s%.*s""   [%12s]",
		RSC(VENDOR_ID).CODE(),
		width - (OutFunc? 21 : 23) - RSZ(VENDOR_ID),
		strlen(Shm->Proc.Features.Info.Hypervisor.ID) > 0 ?
		Shm->Proc.Features.Info.Hypervisor.ID
		: (char*) RSC(NOT_AVAILABLE).CODE(),
		SCANKEY_NULL,
		NULL
	}
    };
	size_t idx;
  for (idx = 0; idx < sizeof(TECH) / sizeof(struct TECH_ST); idx++)
  {
	unsigned short capable = 0;
    if (TECH[idx].CRC == NULL) {
	capable = 1;
    } else {
	unsigned int *CRC;
	for (CRC = TECH[idx].CRC; (*CRC) != 0 && capable == 0; CRC++)
	{
		if ( (*CRC) == Shm->Proc.Features.Info.Vendor.CRC ) {
			capable = 1;
		}
	}
    }
    if (capable)
    {
	TGrid *grid=PUT((TECH[idx].shortkey == SCANKEY_NULL) ? SCANKEY_NULL
			: TECH[idx].shortkey,
			attrib[ TECH[idx].cond ],
			width,	TECH[idx].tab,
			TECH[idx].item, TECH[idx].code,
			TECH[idx].spaces, hSpace,
			(TECH[idx].context == NULL) ? ENABLED(TECH[idx].cond)
			: TECH[idx].context,
			ENABLED(TECH[idx].cond) );

	if (TECH[idx].Update != NULL)
	{
		GridCall(grid, TECH[idx].Update);
	}
    }
  }
	return (reason);
}

void PerfMonUpdate(TGrid *grid, const unsigned int bix, const signed int pos,
				const size_t len, const char *item)
{
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_PERFMON_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_COND1).ATTR(),
		RSC(SYSINFO_PERFMON_COND2).ATTR(),
		RSC(SYSINFO_PERFMON_COND3).ATTR()
	};
	memcpy(&grid->cell.attr[pos], &attrib[bix][pos], len);
	memcpy(&grid->cell.item[pos], item, len);
}

void C1E_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.C1E == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C1A_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.C1A == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C3A_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.C3A == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C1U_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.C1U == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C3U_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.C3U == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void CC6_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.CC6 == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void PC6_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.PC6 == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void HWP_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Features.HWP_Enable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void Refresh_HWP_Cap_Freq(TGrid *grid, DATA_TYPE data)
{
	ATTRIBUTE *HWP_Cap_Attr[2] = {
		RSC(SYSINFO_PERFMON_HWP_CAP_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_HWP_CAP_COND1).ATTR()
	};
	const unsigned int bix = Shm->Proc.Features.HWP_Enable == 1;

	memcpy(grid->cell.attr, HWP_Cap_Attr[bix], 76);

	RefreshRatioFreq(grid, data);
}

void HDC_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Features.HDC_Enable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void IOMWAIT_Update(TGrid *grid, DATA_TYPE data)
{
    const unsigned int bix=Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir == 1;
	const signed int pos = grid->cell.length - 9;
	UNUSED(data);

	PerfMonUpdate( grid, bix ? 3 : 2, pos, 7,
		(char *)(bix ? RSC(ENABLE).CODE() : RSC(DISABLE).CODE()) );
}

char *CST_Encoding[] = {
	[ _C0]	= " C0",
	[ _C1]	= " C1",
	[ _C2]	= " C2",
	[ _C3]	= " C3",
	[ _C4]	= " C4",
	[ _C6]	= " C6",
	[_C6R]	= "C6R",
	[ _C7]	= " C7",
	[_C7S]	= "C7S",
	[ _C8]	= " C8",
	[ _C9]	= " C9",
	[_C10]	= "C10",
	[_UNSPEC]="UNS"
};

void CStateLimit_Update(TGrid *grid, DATA_TYPE data)
{
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	memcpy(&grid->cell.item[pos],
	      CST_Encoding[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit],
		3);
}

void CStateRange_Update(TGrid *grid, DATA_TYPE data)
{
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	memcpy(&grid->cell.item[pos],
	    CST_Encoding[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude],
		3);
}

REASON_CODE SysInfoPerfMon(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[5] = {
		RSC(SYSINFO_PERFMON_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_COND1).ATTR(),
		RSC(SYSINFO_PERFMON_COND2).ATTR(),
		RSC(SYSINFO_PERFMON_COND3).ATTR(),
		RSC(SYSINFO_PERFMON_COND4).ATTR()
	};
	unsigned int bix;
/* Section Mark */
    if (Shm->Proc.PM_version > 0)
    {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sPM       [%3d]", RSC(VERSION).CODE(),
		width - 17 - RSZ(VERSION), hSpace, Shm->Proc.PM_version);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sPM       [%3s]", RSC(VERSION).CODE(),
		width - 17 - RSZ(VERSION), hSpace, RSC(NOT_AVAILABLE).CODE());
    }
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
	GridCall(PUT(BOXKEY_C1E, attrib[bix], width, 2,
			"%s%.*sC1E       <%3s>", RSC(PERF_MON_C1E).CODE(),
			width - 18 - RSZ(PERF_MON_C1E), hSpace, ENABLED(bix)),
		C1E_Update);

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Technology.C1A == 1;
	GridCall(PUT(BOXKEY_C1A, attrib[bix], width, 2,
			"%s%.*sC1A       <%3s>", RSC(PERF_MON_C1A).CODE(),
			width - 18 - RSZ(PERF_MON_C1A), hSpace, ENABLED(bix)),
		C1A_Update);

	bix = Shm->Proc.Technology.C3A == 1;
	GridCall(PUT(BOXKEY_C3A, attrib[bix], width, 2,
			"%s%.*sC3A       <%3s>", RSC(PERF_MON_C3A).CODE(),
			width - 18 - RSZ(PERF_MON_C3A), hSpace, ENABLED(bix)),
		C3A_Update);
    }

	bix = Shm->Proc.Technology.C1U == 1;

    if ( (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
     ||  (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	GridCall(PUT(BOXKEY_C1U, attrib[bix], width, 2,
			"%s%.*sC2U       <%3s>", RSC(PERF_MON_C2U).CODE(),
			width - 18 - RSZ(PERF_MON_C2U),
			hSpace, ENABLED(bix)),
		C1U_Update);
    } else {
	GridCall(PUT(BOXKEY_C1U, attrib[bix], width, 2,
			"%s%.*sC1U       <%3s>", RSC(PERF_MON_C1U).CODE(),
			width - 18 - RSZ(PERF_MON_C1U),
			hSpace, ENABLED(bix)),
		C1U_Update);
    }

	bix = Shm->Proc.Technology.C3U == 1;
	GridCall(PUT(BOXKEY_C3U, attrib[bix], width, 2,
			"%s%.*sC3U       <%3s>", RSC(PERF_MON_C3U).CODE(),
			width - 18 - RSZ(PERF_MON_C3U), hSpace, ENABLED(bix)),
		C3U_Update);

    if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	bix = Shm->Proc.Technology.CC6 == 1;
	GridCall(PUT(BOXKEY_CC6, attrib[bix], width, 2,
			"%s%.*sCC6       <%3s>", RSC(PERF_MON_CC6).CODE(),
			width - 18 - RSZ(PERF_MON_CC6), hSpace, ENABLED(bix)),
		CC6_Update);

	bix = Shm->Proc.Technology.PC6 == 1;
	GridCall(PUT(BOXKEY_PC6, attrib[bix], width, 2,
			"%s%.*sPC6       <%3s>", RSC(PERF_MON_PC6).CODE(),
			width - 18 - RSZ(PERF_MON_PC6), hSpace, ENABLED(bix)),
		PC6_Update);
    } else {
	bix = Shm->Proc.Technology.CC6 == 1;
	GridCall(PUT(BOXKEY_CC6, attrib[bix], width, 2,
			"%s%.*sCC6       <%3s>", RSC(PERF_MON_C6D).CODE(),
			width - 18 - RSZ(PERF_MON_C6D), hSpace, ENABLED(bix)),
		CC6_Update);

	bix = Shm->Proc.Technology.PC6 == 1;
	GridCall(PUT(BOXKEY_PC6, attrib[bix], width, 2,
			"%s%.*sMC6       <%3s>", RSC(PERF_MON_MC6).CODE(),
			width - 18 - RSZ(PERF_MON_MC6), hSpace, ENABLED(bix)),
		PC6_Update);
    }
	bix = Shm->Proc.Features.AdvPower.EDX.FID == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sFID       [%3s]", RSC(PERF_MON_FID).CODE(),
		width - 18 - RSZ(PERF_MON_FID), hSpace, ENABLED(bix));

	bix = (Shm->Proc.Features.AdvPower.EDX.VID == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sVID       [%3s]", RSC(PERF_MON_VID).CODE(),
		width - 18 - RSZ(PERF_MON_VID), hSpace, ENABLED(bix));

	bix = (Shm->Proc.Features.Power.ECX.HCF_Cap == 1)
	   || ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		&& (Shm->Proc.Features.AdvPower.EDX.EffFrqRO == 1))
	   || ((Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON)
		&& (Shm->Proc.Features.AdvPower.EDX.EffFrqRO == 1));
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sMPERF/APERF       [%3s]", RSC(PERF_MON_HWCF).CODE(),
		width - 26 - RSZ(PERF_MON_HWCF), hSpace, ENABLED(bix));

	bix = Shm->Proc.Features.Power.EAX.HWP_Reg == 1;	/* Intel */
    if (bix)
    {
	CPU_STRUCT *SProc = &Shm->Cpu[Shm->Proc.Service.Core];
	struct FLIP_FLOP *CFlop = &SProc->FlipFlop[
				!Shm->Cpu[Shm->Proc.Service.Core].Toggle
	];
	ATTRIBUTE *HWP_Cap_Attr[2] = {
		RSC(SYSINFO_PERFMON_HWP_CAP_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_HWP_CAP_COND1).ATTR()
	};
	bix = Shm->Proc.Features.HWP_Enable == 1;

	GridCall(PUT(BOXKEY_HWP, attrib[bix], width, 2,
			"%s%.*sHWP       <%3s>", RSC(PERF_MON_HWP).CODE(),
			width - 18 - RSZ(PERF_MON_HWP), hSpace, ENABLED(bix)),
		HWP_Update);

	PUT(SCANKEY_NULL, RSC(SYSINFO_PROC_COND0).ATTR(), width, 3,
		"%s""%.*s(MHz)%.*s""%s", RSC(CAPABILITIES).CODE(),
		21 - 3*(OutFunc == NULL) - RSZ(CAPABILITIES), hSpace,
		22, hSpace,
		RSC(RATIO).CODE());

	GridCall(PrintRatioFreq(win, CFlop,
			1, (char*) RSC(LOWEST).CODE(),
			&SProc->PowerThermal.HWP.Capabilities.Lowest,
			0, SCANKEY_NULL, width, OutFunc,
			HWP_Cap_Attr[bix]),
		Refresh_HWP_Cap_Freq,
			&SProc->PowerThermal.HWP.Capabilities.Lowest);

	GridCall(PrintRatioFreq(win, CFlop,
			1, (char*) RSC(EFFICIENT).CODE(),
			&SProc->PowerThermal.HWP.Capabilities.Most_Efficient,
			0, SCANKEY_NULL, width, OutFunc,
			HWP_Cap_Attr[bix]),
		Refresh_HWP_Cap_Freq,
			&SProc->PowerThermal.HWP.Capabilities.Most_Efficient);

	GridCall(PrintRatioFreq(win, CFlop,
			1, (char*) RSC(GUARANTEED).CODE(),
			&SProc->PowerThermal.HWP.Capabilities.Guaranteed,
			0, SCANKEY_NULL, width, OutFunc,
			HWP_Cap_Attr[bix]),
		Refresh_HWP_Cap_Freq,
			&SProc->PowerThermal.HWP.Capabilities.Guaranteed);

	GridCall(PrintRatioFreq(win, CFlop,
			1, (char*) RSC(HIGHEST).CODE(),
			&SProc->PowerThermal.HWP.Capabilities.Highest,
			0, SCANKEY_NULL, width, OutFunc,
			HWP_Cap_Attr[bix]),
		Refresh_HWP_Cap_Freq,
			&SProc->PowerThermal.HWP.Capabilities.Highest);
    } else {
	unsigned int cix;
	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
		bix = Shm->Proc.Features.HWP_Enable == 1;
		cix = Shm->Proc.Features.Power.EAX.HWP_Reg == 0 ?
			0 : Shm->Proc.Features.HWP_Enable == 1;
	}
	else if ( (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		||(Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
	{
		bix = Shm->Proc.Features.AdvPower.EDX.HwPstate == 1;
		cix = Shm->Proc.Features.AdvPower.EDX.HwPstate == 1;
	}
	else {
		bix = 0;
		cix = 4;
	}
	PUT(SCANKEY_NULL, attrib[cix], width, 2,
		"%s%.*sHWP       [%3s]", RSC(PERF_MON_HWP).CODE(),
		width - 18 - RSZ(PERF_MON_HWP), hSpace,
		ENABLED(bix));
    }
/* Section Mark */
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Features.HDC_Enable == 1;
      if (Shm->Proc.Features.Power.EAX.HDC_Reg)
      {
	GridCall(
		PUT(BOXKEY_HDC,
			attrib[bix], width, 2,
			"%s%.*sHDC       <%3s>", RSC(PERF_MON_HDC).CODE(),
			width - 18 - RSZ(PERF_MON_HDC), hSpace, ENABLED(bix)),
		HDC_Update );
      }
      else
      {
	PUT(	SCANKEY_NULL,
		attrib[0], width, 2,
		"%s%.*sHDC       [%3s]", RSC(PERF_MON_HDC).CODE(),
		width - 18 - RSZ(PERF_MON_HDC), hSpace, ENABLED(bix) );
      }
/* Section Mark */
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s", RSC(PERF_MON_PKG_CSTATE).CODE());

	bix = Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock == 0 ? 3 : 0;

	PUT(SCANKEY_NULL, attrib[bix], width, 3,
		"%s%.*sCONFIG   [%7s]", RSC(PERF_MON_CFG_CTRL).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(PERF_MON_CFG_CTRL), hSpace,
		!Shm->Cpu[Shm->Proc.Service.Core].Query.CfgLock ?
			RSC(UNLOCK).CODE() : RSC(LOCK).CODE());

	    GridCall(PUT(BOXKEY_PKGCST, attrib[0], width, 3,
			"%s%.*sLIMIT   <%7s>", RSC(PERF_MON_LOW_CSTATE).CODE(),
			width - (OutFunc == NULL ? 23 : 21)
			- RSZ(PERF_MON_LOW_CSTATE), hSpace,
	      CST_Encoding[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit]),
		CStateLimit_Update);

		bix = Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir == 1 ? 3:2;
	    GridCall(PUT(BOXKEY_IOMWAIT, attrib[bix], width, 3,
			"%s%.*sIOMWAIT   <%7s>", RSC(PERF_MON_IOMWAIT).CODE(),
			width - (OutFunc == NULL ? 25 : 23)
			- RSZ(PERF_MON_IOMWAIT), hSpace,
			Shm->Cpu[Shm->Proc.Service.Core].Query.IORedir ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE()),
		IOMWAIT_Update);

	    GridCall(PUT(BOXKEY_IORCST, attrib[0], width, 3,
			"%s%.*sRANGE   <%7s>", RSC(PERF_MON_MAX_CSTATE).CODE(),
			width - (OutFunc == NULL ? 23 : 21)
			- RSZ(PERF_MON_MAX_CSTATE), hSpace,
	    CST_Encoding[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude]),
		CStateRange_Update);
    }
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s", RSC(PERF_MON_CORE_CSTATE).CODE());

	PUT(SCANKEY_NULL,
	attrib[!Shm->Cpu[Shm->Proc.Service.Core].Query.CStateBaseAddr ? 0 : 2],
		width, 3,
		"%s%.*sBAR   [ 0x%-4X]", RSC(PERF_MON_CSTATE_BAR).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		- RSZ(PERF_MON_CSTATE_BAR), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].Query.CStateBaseAddr);
/* Section Mark */
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s", RSC(PERF_MON_MONITOR_MWAIT).CODE());

	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s:%.*s#0    #1    #2    #3    #4    #5    #6    #7",
		RSC(PERF_MON_MWAIT_IDX_CSTATE).CODE(), 04, hSpace);

	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s:%.*s%2d    %2d    %2d    %2d    %2d    %2d    %2d    %2d",
		RSC(PERF_MON_MWAIT_SUB_CSTATE).CODE(),
		15 - RSZ(PERF_MON_MWAIT_SUB_CSTATE), hSpace,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT0,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT1,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT2,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT3,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT4,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT5,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT6,
		Shm->Proc.Features.MWait.EDX.SubCstate_MWAIT7);
/* Section Mark */
	bix = Shm->Proc.Features.PerfMon.EBX.CoreCycles == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_CORE_CYCLE).CODE(),
		width - 12 - RSZ(PERF_MON_CORE_CYCLE), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.InstrRetired == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_INST_RET).CODE(),
		width - 12 - RSZ(PERF_MON_INST_RET), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.RefCycles == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_REF_CYCLE).CODE(),
		width - 12 - RSZ(PERF_MON_REF_CYCLE), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.LLC_Ref == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_REF_LLC).CODE(),
		width - 12 - RSZ(PERF_MON_REF_LLC), hSpace, POWERED(bix));

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = Shm->Proc.Features.PerfMon.EBX.LLC_Misses == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_MISS_LLC).CODE(),
		width - 12 - RSZ(PERF_MON_MISS_LLC), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.BranchRetired == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_BRANCH_RET).CODE(),
		width - 12 - RSZ(PERF_MON_BRANCH_RET), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.PerfMon.EBX.BranchMispred == 0 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_BRANCH_MIS).CODE(),
		width - 12 - RSZ(PERF_MON_BRANCH_MIS), hSpace, POWERED(bix));
    }
    else if( (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	bix = Shm->Proc.Features.ExtInfo.ECX.PerfTSC == 1 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_TSC).CODE(),
		width - 12 - RSZ(PERF_MON_TSC), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.ExtInfo.ECX.PerfNB == 1 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_NB_DF).CODE(),
		width - 12 - RSZ(PERF_MON_NB_DF), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.ExtInfo.ECX.PerfCore == 1 ? 2 : 0;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_CORE).CODE(),
		width - 12 - RSZ(PERF_MON_CORE), hSpace, POWERED(bix));
    }
	return (reason);
}

void PwrThermalUpdate(TGrid *grid, const unsigned int bix,const signed int pos,
				const size_t len, const char *item)
{
	ATTRIBUTE *attrib[4] = {
		RSC(SYSINFO_PWR_THERMAL_COND0).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND1).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND2).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND3).ATTR()
	};
	memcpy(&grid->cell.attr[pos], &attrib[bix][pos], len);
	memcpy(&grid->cell.item[pos], item, len);
}

void ODCM_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Proc.Technology.ODCM == 1;
	const signed int pos = grid->cell.length - 9;
	UNUSED(data);

	PwrThermalUpdate( grid, bix ? 3 : 1, pos, 7,
		(char *)(bix ? RSC(ENABLE).CODE() : RSC(DISABLE).CODE()) );
}

void DutyCycle_Update(TGrid *grid, DATA_TYPE data)
{
	const signed int pos = grid->cell.length - 10;
	const unsigned int bix = (Shm->Proc.Features.Std.EDX.ACPI == 1)
				&& (Shm->Proc.Technology.ODCM == 1);
	char item[10+1];
	UNUSED(data);

	snprintf(item, 10+1, "%c%6.2f%%%c",
		bix ? '<' : '[',
	(Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.Extended ?
		6.25f : 12.5f)
	* Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod,
		bix ? '>' : ']');

	memcpy(&grid->cell.item[pos], item, 9);
	grid->cell.quick.key = bix ? BOXKEY_DUTYCYCLE : SCANKEY_NULL;
}

void Hint_Update(TGrid *grid, DATA_TYPE data)
{
	const signed int pos = grid->cell.length - 9;
	char item[10+1];

	snprintf(item, 10+1, "%7u", (*data.puint));
	memcpy(&grid->cell.item[pos], item, 7);
}

void TjMax_Update(TGrid *grid, DATA_TYPE data)
{
	struct FLIP_FLOP *SFlop = &Shm->Cpu[
		Shm->Proc.Service.Core
	].FlipFlop[
		!Shm->Cpu[Shm->Proc.Service.Core].Toggle
	];
	const signed int pos = grid->cell.length - 9;
	char item[10+1+10+1];
	UNUSED(data);

	snprintf(item, 10+1+10+1, "%2u:%3u",
		SFlop->Thermal.Param.Offset[1],
		SFlop->Thermal.Param.Offset[0]);

	memcpy(&grid->cell.item[pos], item, 6);
}

REASON_CODE SysInfoPwrThermal(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[6] = {
		RSC(SYSINFO_PWR_THERMAL_COND0).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND1).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND2).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND3).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND4).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND5).ATTR()
	};
	const ASCII *TM[] = {
		RSC(MISSING).CODE(),
		RSC(PRESENT).CODE(),
		RSC(DISABLE).CODE(),
		RSC(ENABLE).CODE()
	}, *Unlock[] = {
		RSC(LOCK).CODE(),
		RSC(UNLOCK).CODE()
	};
	struct FLIP_FLOP *SFlop = &Shm->Cpu[
		Shm->Proc.Service.Core
	].FlipFlop[
		!Shm->Cpu[Shm->Proc.Service.Core].Toggle
	];
	unsigned int bix;
/* Section Mark */
  if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
	bix = Shm->Proc.Features.Std.EDX.ACPI == 1;
	GridCall(PUT(bix ? BOXKEY_ODCM : SCANKEY_NULL,
		attrib[ bix ? 3 : 1 ],
			width, 2,
			"%s%.*sODCM   %c%7s%c", RSC(POWER_THERMAL_ODCM).CODE(),
			width - 19 - RSZ(POWER_THERMAL_ODCM), hSpace,
			bix ? '<' : '[',
			(Shm->Proc.Technology.ODCM == 1) ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE(),
			bix ? '>' : ']'),
		ODCM_Update);

	bix = (Shm->Proc.Features.Std.EDX.ACPI == 1)
	   && (Shm->Proc.Technology.ODCM == 1);

      GridCall(PUT(bix ? BOXKEY_DUTYCYCLE : SCANKEY_NULL,
		attrib[0],
		width, 3,
		"%s%.*s%c%6.2f%%%c", RSC(POWER_THERMAL_DUTY).CODE(),
	width - (OutFunc == NULL ? 15: 13) - RSZ(POWER_THERMAL_DUTY), hSpace,
		bix ? '<' : '[',
	(Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.Extended ?
		6.25f : 12.5f)
	* Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod,
		bix ? '>' : ']'),
      DutyCycle_Update);

	bix = Shm->Proc.Technology.PowerMgmt == 1;
	PUT(SCANKEY_NULL,
		attrib[ bix ? 3 : 0 ],
		width, 2,
		"%s%.*sPWR MGMT   [%7s]", RSC(POWER_THERMAL_MGMT).CODE(),
		width - 23 - RSZ(POWER_THERMAL_MGMT), hSpace,
		Unlock[Shm->Proc.Technology.PowerMgmt]);

	bix = Shm->Proc.Features.Power.ECX.SETBH == 1;
    if (bix) {
      GridCall(PUT(BOXKEY_PWR_POLICY,
		attrib[0],
		width, 3,
		"%s%.*sBias Hint   <%7u>", RSC(POWER_THERMAL_BIAS).CODE(),
	width - (OutFunc == NULL ? 27 : 25) - RSZ(POWER_THERMAL_BIAS), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.PowerPolicy),
	Hint_Update,
		&Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.PowerPolicy);
    } else {
	PUT(SCANKEY_NULL,
		attrib[0],
		width, 3,
		"%s%.*sBias Hint   [%7u]", RSC(POWER_THERMAL_BIAS).CODE(),
	width - (OutFunc == NULL ? 27 : 25) - RSZ(POWER_THERMAL_BIAS), hSpace,
		Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.PowerPolicy);
    }

	bix = Shm->Proc.Features.HWP_Enable == 1;		/* Intel */
    if (bix) {
      GridCall(PUT(BOXKEY_HWP_EPP, attrib[0], width, 3,
		"%s%.*sHWP EPP   <%7u>", RSC(POWER_THERMAL_BIAS).CODE(),
	width - (OutFunc == NULL ? 25 : 23) - RSZ(POWER_THERMAL_BIAS), hSpace,
	Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.HWP.Request.Energy_Pref),
	Hint_Update,
	&Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.HWP.Request.Energy_Pref);
    } else {
	PUT(SCANKEY_NULL, attrib[Shm->Proc.Features.Power.EAX.HWP_Reg ? 0 : 4],
		width, 3,
		"%s%.*sHWP EPP   [%7u]", RSC(POWER_THERMAL_BIAS).CODE(),
	width - (OutFunc == NULL ? 25 : 23) - RSZ(POWER_THERMAL_BIAS), hSpace,
	Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.HWP.Request.Energy_Pref);
    }
  }
      GridCall(PUT(SCANKEY_NULL, attrib[5], width, 2,
		"%s%.*sTjMax   [%2u:%3uC]", RSC(POWER_THERMAL_TJMAX).CODE(),
		width - 20 - RSZ(POWER_THERMAL_TJMAX), hSpace,
		SFlop->Thermal.Param.Offset[1],
		SFlop->Thermal.Param.Offset[0]),
	TjMax_Update);

	bix = (Shm->Proc.Features.Power.EAX.DTS == 1)
	   || (Shm->Proc.Features.AdvPower.EDX.TS == 1);
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sDTS   [%7s]", RSC(POWER_THERMAL_DTS).CODE(),
		width - 18 - RSZ(POWER_THERMAL_DTS), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.Power.EAX.PLN == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sPLN   [%7s]", RSC(POWER_THERMAL_PLN).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PLN), hSpace, POWERED(bix));

	bix = Shm->Proc.Features.Power.EAX.PTM == 1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sPTM   [%7s]", RSC(POWER_THERMAL_PTM).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PTM), hSpace, POWERED(bix));

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL) {
	bix = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sTM1   [%7s]", RSC(POWER_THERMAL_TM1).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM1), hSpace, TM[bix]);

	bix = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sTM2   [%7s]", RSC(POWER_THERMAL_TM2).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM2), hSpace, TM[bix]);
    }
    else if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	 || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	bix = Shm->Proc.Features.AdvPower.EDX.TTP;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sTTP   [%7s]", RSC(POWER_THERMAL_TM1).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM1), hSpace, TM[bix]);

	bix = Shm->Proc.Features.AdvPower.EDX.TM;
	PUT(SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*sHTC   [%7s]", RSC(POWER_THERMAL_TM2).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM2), hSpace, TM[bix]);
    }

    if (Shm->Proc.Power.TDP > 0) {
	PUT(SCANKEY_NULL, attrib[5], width, 2,
		"%s%.*sTDP   [%5u W]", RSC(POWER_THERMAL_TDP).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TDP),hSpace,Shm->Proc.Power.TDP);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sTDP   [%7s]", RSC(POWER_THERMAL_TDP).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TDP), hSpace, POWERED(0));
    }
    if (Shm->Proc.Power.Min > 0) {
	PUT(SCANKEY_NULL, attrib[5], width, 3,
		"%s%.*sMin   [%5u W]", RSC(POWER_THERMAL_MIN).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MIN), hSpace,Shm->Proc.Power.Min);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*sMin   [%7s]", RSC(POWER_THERMAL_MIN).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MIN), hSpace, POWERED(0));
    }
    if (Shm->Proc.Power.Max > 0) {
	PUT(SCANKEY_NULL, attrib[5], width, 3,
		"%s%.*sMax   [%5u W]", RSC(POWER_THERMAL_MAX).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MAX), hSpace,Shm->Proc.Power.Max);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*sMax   [%7s]", RSC(POWER_THERMAL_MAX).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MAX), hSpace, POWERED(0));
    }
    if (Shm->Proc.Power.PPT > 0) {
	PUT(SCANKEY_NULL, attrib[5], width, 2,
		"%s%.*sPPT   [%5u W]", RSC(POWER_THERMAL_PPT).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PPT),hSpace,Shm->Proc.Power.PPT);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sPPT   [%7s]", RSC(POWER_THERMAL_PPT).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PPT), hSpace, POWERED(0));
    }
    if (Shm->Proc.Power.EDC > 0) {
	PUT(SCANKEY_NULL, attrib[5], width, 2,
		"%s%.*sEDC   [%5u A]", RSC(POWER_THERMAL_EDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_EDC),hSpace,Shm->Proc.Power.EDC);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sEDC   [%7s]", RSC(POWER_THERMAL_EDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_EDC), hSpace, POWERED(0));
    }
    if (Shm->Proc.Power.TDC > 0) {
	PUT(SCANKEY_NULL, attrib[5], width, 2,
		"%s%.*sTDC   [%5u A]", RSC(POWER_THERMAL_TDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TDC),hSpace,Shm->Proc.Power.TDC);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*sTDC   [%7s]", RSC(POWER_THERMAL_TDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TDC), hSpace, POWERED(0));
    }

	PUT(SCANKEY_NULL, attrib[0], width, 2,
		(char*) RSC(POWER_THERMAL_UNITS).CODE(), NULL);

    if (Shm->Proc.Power.Unit.Watts > 0.0) {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]",
		RSC(POWER_THERMAL_POWER).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		 - RSZ(POWER_THERMAL_POWER) - RSZ(POWER_THERMAL_WATT), hSpace,
		RSC(POWER_THERMAL_WATT).CODE(), Shm->Proc.Power.Unit.Watts);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13s]",
		RSC(POWER_THERMAL_POWER).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		 - RSZ(POWER_THERMAL_POWER) - RSZ(POWER_THERMAL_WATT), hSpace,
		RSC(POWER_THERMAL_WATT).CODE(), POWERED(0));
    }
    if (Shm->Proc.Power.Unit.Joules > 0.0) {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]", RSC(POWER_THERMAL_ENERGY).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_ENERGY) - RSZ(POWER_THERMAL_JOULE), hSpace,
		RSC(POWER_THERMAL_JOULE).CODE(), Shm->Proc.Power.Unit.Joules);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13s]", RSC(POWER_THERMAL_ENERGY).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_ENERGY) - RSZ(POWER_THERMAL_JOULE), hSpace,
		RSC(POWER_THERMAL_JOULE).CODE(), POWERED(0));
    }
    if (Shm->Proc.Power.Unit.Times > 0.0) {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]", RSC(POWER_THERMAL_WINDOW).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_WINDOW) - RSZ(POWER_THERMAL_SECOND), hSpace,
		RSC(POWER_THERMAL_SECOND).CODE(), Shm->Proc.Power.Unit.Times);
    } else {
	PUT(SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13s]", RSC(POWER_THERMAL_WINDOW).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_WINDOW) - RSZ(POWER_THERMAL_SECOND), hSpace,
		RSC(POWER_THERMAL_SECOND).CODE(), POWERED(0));
    }
	return (reason);
}

void KernelUpdate(TGrid *grid, DATA_TYPE data)
{
	char item[CPUFREQ_NAME_LEN+4+3];
	size_t len = snprintf(  item, CPUFREQ_NAME_LEN+4+3,
				"%18lu KB", (*data.pulong) );
	memcpy(&grid->cell.item[grid->cell.length - len - 1], item, len);
}

void IdleLimitUpdate(TGrid *grid, DATA_TYPE data)
{
	char item[CPUIDLE_NAME_LEN+1];
	unsigned int idx = (*data.psint) - 1;
	size_t len;
    if (Shm->SysGate.OS.IdleDriver.stateCount > 0)
    {
	len = snprintf(	item, CPUIDLE_NAME_LEN + 1,
				COREFREQ_FORMAT_STR(CPUIDLE_NAME_LEN),
				Shm->SysGate.OS.IdleDriver.State[idx].Name );
    }
    else
    {
	len = snprintf(	item, CPUIDLE_NAME_LEN + 1,
				COREFREQ_FORMAT_STR(CPUIDLE_NAME_LEN),
				RSC(NOT_AVAILABLE).CODE() );
    }
	memcpy(&grid->cell.item[grid->cell.length - len - 2], item, len);
}

REASON_CODE SysInfoKernel(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	size_t	len = (1 + width) * 5;
	char	*item[5], str[CPUFREQ_NAME_LEN+4+1];
	signed int idx;
	for (idx = 0; idx < 5; idx++) {
		if ((item[idx] = malloc(len)) != NULL) {
			continue;
		} else {
			do {
				free(item[idx]);
			} while (idx-- != 0);

			REASON_SET(reason, RC_MEM_ERR);
			return (reason);
		}
	}
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
		width - RSZ(KERNEL_MEMORY), hSpace);

	len = snprintf( str,CPUFREQ_NAME_LEN+4+1, "%lu",
			Shm->SysGate.memInfo.totalram );
	PUT(SCANKEY_NULL, RSC(KERNEL_TOTAL_RAM).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_TOTAL_RAM).CODE(),
		width - 6 - RSZ(KERNEL_TOTAL_RAM) - len, hSpace, str);

	len = snprintf( str, CPUFREQ_NAME_LEN+4+1, "%lu",
			Shm->SysGate.memInfo.sharedram );
	GridCall(PUT(SCANKEY_NULL, RSC(KERNEL_SHARED_RAM).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_SHARED_RAM).CODE(),
			width - 6 - RSZ(KERNEL_SHARED_RAM) - len, hSpace, str),
		KernelUpdate, &Shm->SysGate.memInfo.sharedram);

	len = snprintf( str, CPUFREQ_NAME_LEN+4+1,
			"%lu", Shm->SysGate.memInfo.freeram );
	GridCall(PUT(SCANKEY_NULL, RSC(KERNEL_FREE_RAM).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_FREE_RAM).CODE(),
			width - 6 - RSZ(KERNEL_FREE_RAM) - len, hSpace, str),
		KernelUpdate, &Shm->SysGate.memInfo.freeram);

	len = snprintf( str, CPUFREQ_NAME_LEN+4+1,
			"%lu", Shm->SysGate.memInfo.bufferram );
	GridCall(PUT(SCANKEY_NULL, RSC(KERNEL_BUFFER_RAM).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_BUFFER_RAM).CODE(),
			width - 6 - RSZ(KERNEL_BUFFER_RAM) - len, hSpace, str),
		KernelUpdate, &Shm->SysGate.memInfo.bufferram);

	len = snprintf( str, CPUFREQ_NAME_LEN+4+1,
			"%lu", Shm->SysGate.memInfo.totalhigh );
	GridCall(PUT(SCANKEY_NULL, RSC(KERNEL_TOTAL_HIGH).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_TOTAL_HIGH).CODE(),
			width - 6 - RSZ(KERNEL_TOTAL_HIGH) - len, hSpace, str),
		KernelUpdate, &Shm->SysGate.memInfo.totalhigh);

	len = snprintf( str, CPUFREQ_NAME_LEN+4+1,
			"%lu", Shm->SysGate.memInfo.freehigh );
	GridCall(PUT(SCANKEY_NULL, RSC(KERNEL_FREE_HIGH).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_FREE_HIGH).CODE(),
			width - 6 - RSZ(KERNEL_FREE_HIGH) - len, hSpace, str),
		KernelUpdate, &Shm->SysGate.memInfo.freehigh);
/* Section Mark */
	snprintf(item[0], 2+4+1+6+1+1, "%%s%%.*s[%%%d.*s]", CPUFREQ_NAME_LEN);

    len = KMIN(strlen(Shm->SysGate.OS.FreqDriver.Name), CPUFREQ_NAME_LEN);
    if (len > 0)
    {
	PUT(SCANKEY_NULL, RSC(KERNEL_FREQ_DRIVER).ATTR(), width, 0,
		item[0], RSC(KERNEL_FREQ_DRIVER).CODE(),
		width - (OutFunc == NULL ? 2 : 3)
		- RSZ(KERNEL_FREQ_DRIVER) - CPUFREQ_NAME_LEN, hSpace,
		len, Shm->SysGate.OS.FreqDriver.Name);
    } else {
	PUT(SCANKEY_NULL, RSC(KERNEL_FREQ_DRIVER).ATTR(), width, 0,
		item[0], RSC(KERNEL_FREQ_DRIVER).CODE(),
		width - (OutFunc == NULL ? 2 : 3)
		- RSZ(KERNEL_FREQ_DRIVER) - CPUFREQ_NAME_LEN, hSpace,
		CPUFREQ_NAME_LEN, RSC(MISSING).CODE());
    }
/* Row Mark */
    len = KMIN(strlen(Shm->SysGate.OS.FreqDriver.Governor), CPUFREQ_NAME_LEN);
    if (len > 0)
    {
	PUT(SCANKEY_NULL, RSC(KERNEL_GOVERNOR).ATTR(), width, 0,
		item[0], RSC(KERNEL_GOVERNOR).CODE(),
		width - (OutFunc == NULL ? 2 : 3) - RSZ(KERNEL_GOVERNOR)
		- CPUFREQ_NAME_LEN, hSpace,
		len, Shm->SysGate.OS.FreqDriver.Governor);
    } else {
	PUT(SCANKEY_NULL, RSC(KERNEL_GOVERNOR).ATTR(), width, 0,
		item[0], RSC(KERNEL_GOVERNOR).CODE(),
		width - (OutFunc == NULL ? 2 : 3) - RSZ(KERNEL_GOVERNOR)
		- CPUFREQ_NAME_LEN, hSpace,
		CPUFREQ_NAME_LEN, RSC(MISSING).CODE());
    }
/* Row Mark */
	snprintf(item[0], 2+4+1+6+1+1, "%%s%%.*s[%%%d.*s]", CPUIDLE_NAME_LEN);

    len = KMIN(strlen(Shm->SysGate.OS.IdleDriver.Name), CPUIDLE_NAME_LEN);
    if (len > 0)
    {
	PUT(SCANKEY_NULL, RSC(KERNEL_IDLE_DRIVER).ATTR(), width, 0,
		item[0], RSC(KERNEL_IDLE_DRIVER).CODE(),
		width - (OutFunc == NULL ? 2 : 3)
		- RSZ(KERNEL_IDLE_DRIVER) - CPUIDLE_NAME_LEN, hSpace,
		len, Shm->SysGate.OS.IdleDriver.Name);
    } else {
	PUT(SCANKEY_NULL, RSC(KERNEL_IDLE_DRIVER).ATTR(), width, 0,
		item[0], RSC(KERNEL_IDLE_DRIVER).CODE(),
		width - (OutFunc == NULL ? 2 : 3)
		- RSZ(KERNEL_IDLE_DRIVER) - CPUIDLE_NAME_LEN, hSpace,
		CPUIDLE_NAME_LEN, RSC(MISSING).CODE());
    }
/* Section Mark */
  if (Shm->SysGate.OS.IdleDriver.stateCount > 0)
  {
	snprintf(item[0], 2+4+1+4+1+1, "%%s%%.*s%c%%%ds%c",
	Shm->Registration.Driver.CPUidle & REGISTRATION_ENABLE ? '<' : '[',
		CPUIDLE_NAME_LEN,
	Shm->Registration.Driver.CPUidle & REGISTRATION_ENABLE ? '>' : ']');

	idx = Shm->SysGate.OS.IdleDriver.stateLimit - 1;
	GridCall(PUT(Shm->Registration.Driver.CPUidle & REGISTRATION_ENABLE ?
			BOXKEY_LIMIT_IDLE_STATE : SCANKEY_NULL,
			RSC(KERNEL_LIMIT).ATTR(), width, 2,
			item[0], RSC(KERNEL_LIMIT).CODE(),
			width - RSZ(KERNEL_LIMIT) - CPUIDLE_NAME_LEN-5, hSpace,
			Shm->SysGate.OS.IdleDriver.State[idx].Name),
		IdleLimitUpdate, &Shm->SysGate.OS.IdleDriver.stateLimit);
/* Row Mark */
	snprintf(item[0], 10+1, "%s%.*s" , RSC(KERNEL_STATE).CODE(),
				10 - (int) RSZ(KERNEL_STATE), hSpace);

	snprintf(item[1], 10+1, "%.*s", 10, hSpace);

	snprintf(item[2], 10+1, "%s%.*s" , RSC(KERNEL_POWER).CODE(),
				10 - (int) RSZ(KERNEL_POWER), hSpace);

	snprintf(item[3], 10+1, "%s%.*s" , RSC(KERNEL_LATENCY).CODE(),
				10 - (int) RSZ(KERNEL_LATENCY), hSpace);

	snprintf(item[4], 10+1, "%s%.*s" , RSC(KERNEL_RESIDENCY).CODE(),
				10 - (int) RSZ(KERNEL_RESIDENCY), hSpace);

    for (idx = 0; idx < CPUIDLE_STATE_MAX; idx++)
    {
      if (idx < Shm->SysGate.OS.IdleDriver.stateCount)
      {
	int n, cat;

	len = KMIN(strlen(Shm->SysGate.OS.IdleDriver.State[idx].Name), 7);
	cat = snprintf( str, 7+1,"%7.*s", (int) len,
			Shm->SysGate.OS.IdleDriver.State[idx].Name );
	len = strlen(item[0]);
	for (n = 0; n < cat; n++) {
		item[0][len + n] = str[n];
	}
	item[0][len + n] = '\0';

	len = KMIN(strlen(Shm->SysGate.OS.IdleDriver.State[idx].Desc), 7);
	cat = snprintf( str, 7+1, "%7.*s", (int) len,
			Shm->SysGate.OS.IdleDriver.State[idx].Desc );
	len = strlen(item[1]);
	for (n = 0; n < cat; n++) {
		item[1][len + n] = str[n];
	}
	item[1][len + n] = '\0';

	cat = snprintf( str, 10+1, "%7d",
			Shm->SysGate.OS.IdleDriver.State[idx].powerUsage );
	len = strlen(item[2]);
	for (n = 0; n < cat; n++) {
		item[2][len + n] = str[n];
	}
	item[2][len + n] = '\0';

	cat = snprintf( str, 10+1, "%7u",
			Shm->SysGate.OS.IdleDriver.State[idx].exitLatency );
	len = strlen(item[3]);
	for (n = 0; n < cat; n++) {
		item[3][len + n] = str[n];
	}
	item[3][len + n] = '\0';

	cat = snprintf( str, 10+1, "%7u",
			Shm->SysGate.OS.IdleDriver.State[idx].targetResidency );
	len = strlen(item[4]);
	for (n = 0; n < cat; n++) {
		item[4][len + n] = str[n];
	}
	item[4][len + n] = '\0';
      } else {
	int d;
	for (d = 0; d < 5; d++) {
		len = strlen(item[d]);
		item[d][len    ] = \
		item[d][len + 1] = \
		item[d][len + 2] = \
		item[d][len + 3] = '\x20';
		item[d][len + 4] = '\0';
	}
      }
      if (idx < (CPUIDLE_STATE_MAX - 1)) {
	int d;
	for (d = 0; d < 5; d++) {
		len = strlen(item[d]);
		item[d][len    ] = '\x20';
		item[d][len + 1] = '\0';
	}
      }
    }
	PUT(SCANKEY_NULL, RSC(KERNEL_STATE).ATTR(), width, 3,
		 "%.*s", width - (OutFunc == NULL ? 6 : 3), item[0]);

	PUT(SCANKEY_NULL, RSC(KERNEL_STATE).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[1]);

	PUT(SCANKEY_NULL, RSC(KERNEL_POWER).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[2]);

	PUT(SCANKEY_NULL, RSC(KERNEL_LATENCY).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[3]);

	PUT(SCANKEY_NULL, RSC(KERNEL_RESIDENCY).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[4]);
  }
/* Section Mark */
    for (idx = 0; idx < 5; idx++) {
	free(item[idx]);
    }
	return (reason);
}

char *ScrambleSMBIOS(enum SMB_STRING idx, int mod, char thing)
{
	struct {
		char		*pString;
		unsigned short	secret;
	} smb[SMB_STRING_COUNT] = {
		{.pString = Shm->SMB.BIOS.Vendor,	.secret = 0},
		{.pString = Shm->SMB.BIOS.Version,	.secret = 0},
		{.pString = Shm->SMB.BIOS.Release,	.secret = 0},
		{.pString = Shm->SMB.System.Vendor,	.secret = 0},
		{.pString = Shm->SMB.Product.Name,	.secret = 0},
		{.pString = Shm->SMB.Product.Version,	.secret = 0},
		{.pString = Shm->SMB.Product.Serial,	.secret = 1},
		{.pString = Shm->SMB.Product.SKU,	.secret = 0},
		{.pString = Shm->SMB.Product.Family,	.secret = 0},
		{.pString = Shm->SMB.Board.Vendor,	.secret = 0},
		{.pString = Shm->SMB.Board.Name,	.secret = 0},
		{.pString = Shm->SMB.Board.Version,	.secret = 0},
		{.pString = Shm->SMB.Board.Serial,	.secret = 1}
	};
	if (smb[idx].secret && Setting.secret) {
		static char outStr[MAX_UTS_LEN];
		ssize_t len = strlen(smb[idx].pString);
		int i;
		for (i = 0; i < len; i++) {
			outStr[i] = (i % mod) ? thing : smb[idx].pString[i];
		}
		outStr[i] = '\0';
		return (outStr);
	} else {
		return (smb[idx].pString);
	}
}

const char *SMB_Comment[SMB_STRING_COUNT] = {
	" "	COREFREQ_STRINGIFY(SMB_BIOS_VENDOR)	" ",
	" "	COREFREQ_STRINGIFY(SMB_BIOS_VERSION)	" ",
	" "	COREFREQ_STRINGIFY(SMB_BIOS_RELEASE)	" ",
	" "	COREFREQ_STRINGIFY(SMB_SYSTEM_VENDOR)	" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_NAME)	" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_VERSION) " ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_SERIAL)	" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_SKU)	" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_FAMILY)	" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_VENDOR)	" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_NAME)	" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_VERSION)	" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_SERIAL)	" "
};

REASON_CODE SysInfoSMBIOS(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	enum SMB_STRING idx;

	REASON_INIT(reason);

    for (idx = 0; idx < SMB_STRING_COUNT; idx ++)
    {
	GridHover( PUT( SMBIOS_STRING_INDEX|idx, RSC(SMBIOS_ITEM).ATTR(),
			width, 0, "[%2d] %s", idx, ScrambleSMBIOS(idx, 4, '-')),
		SMB_Comment[idx] );
    }
	return (reason);
}

void Package(void)
{
	char *out = malloc(8 + (Shm->Proc.CPU.Count + 10) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int sdx, idx;

	sdx = sprintf(out, "\t\t" "Cycles" "\t\t" "State(%%)" "\n");

    while (!BITVAL(Shutdown, SYNC)) {
	while (!BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
	}
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	idx = sdx;
	idx+= sprintf(&out[idx],
		"PC02" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC03" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC04" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC06" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC07" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC08" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC09" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PC10" "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"MC6"  "\t" "%18llu" "\t" "%7.2f" "\n"	\
		"PTSC" "\t" "%18llu" "\n"		\
		"UNCORE" "\t" "%18llu" "\n\n",
		PFlop->Delta.PC02, 100.f * Shm->Proc.State.PC02,
		PFlop->Delta.PC03, 100.f * Shm->Proc.State.PC03,
		PFlop->Delta.PC04, 100.f * Shm->Proc.State.PC04,
		PFlop->Delta.PC06, 100.f * Shm->Proc.State.PC06,
		PFlop->Delta.PC07, 100.f * Shm->Proc.State.PC07,
		PFlop->Delta.PC08, 100.f * Shm->Proc.State.PC08,
		PFlop->Delta.PC09, 100.f * Shm->Proc.State.PC09,
		PFlop->Delta.PC10, 100.f * Shm->Proc.State.PC10,
		PFlop->Delta.MC6,  100.f * Shm->Proc.State.MC6,
		PFlop->Delta.PTSC,
		PFlop->Uncore.FC0);

	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

unsigned int Core_Celsius(char *out, struct FLIP_FLOP *CFlop, unsigned int cpu)
{
	return(sprintf(out,
		"%03u %7.2f (%5.2f)"				\
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
		Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST],
		CFlop->Thermal.Temp,
		CFlop->Thermal.Sensor,
		Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST]));
}

unsigned int Core_Fahrenheit(char *out,struct FLIP_FLOP *CFlop,unsigned int cpu)
{
	return(sprintf(out,
		"%03u %7.2f (%5.2f)"				\
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
		Cels2Fahr(Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST]),
		Cels2Fahr(CFlop->Thermal.Temp),
		Cels2Fahr(CFlop->Thermal.Sensor),
		Cels2Fahr(Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST])));
}

unsigned int Pkg_Celsius(char *out, struct PKG_FLIP_FLOP *PFlop)
{
	struct FLIP_FLOP *SFlop = &Shm->Cpu[
		Shm->Proc.Service.Core
	].FlipFlop[
		!Shm->Cpu[Shm->Proc.Service.Core].Toggle
	];

	return(sprintf(out, "\n"					\
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
		SFlop->Thermal.Param.Offset[0],
		3, hSpace,
		PFlop->Thermal.Temp));
}

unsigned int Pkg_Fahrenheit(char *out, struct PKG_FLIP_FLOP *PFlop)
{
	struct FLIP_FLOP *SFlop = &Shm->Cpu[
		Shm->Proc.Service.Core
	].FlipFlop[
		!Shm->Cpu[Shm->Proc.Service.Core].Toggle
	];

	return(sprintf(out, "\n"					\
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
		SFlop->Thermal.Param.Offset[0],
		3, hSpace,
		Cels2Fahr(PFlop->Thermal.Temp)));
}

void Counters(void)
{
	unsigned int (*Core_Temp)(char *, struct FLIP_FLOP *, unsigned int) = \
		Setting.fahrCels ? Core_Fahrenheit : Core_Celsius;

	unsigned int (*Pkg_Temp)(char *, struct PKG_FLIP_FLOP *) =	\
		Setting.fahrCels ? Pkg_Fahrenheit : Pkg_Celsius;

	char *out = malloc(8 + (Shm->Proc.CPU.Count + 3) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int cpu, sdx, idx;

	sdx=sprintf(out,"CPU Freq(MHz) Ratio  Turbo"			\
			"  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
			"  Min TMP:TS  Max\n");

    while (!BITVAL(Shutdown, SYNC)) {
	while (!BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu=0;(cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
	{
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, HW)) {
		struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
			idx += Core_Temp(&out[idx], CFlop, cpu);
		} else {
			idx += sprintf(&out[idx], "%03u        OFF\n", cpu);
		}
	    }
	}
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];

	idx += Pkg_Temp(&out[idx], PFlop);

	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

void Sensors(void)
{
	char *out = malloc(8 + (Shm->Proc.CPU.Count + 4) * MIN_WIDTH);
	char *row = malloc(MIN_WIDTH + 16);
  if (out && row)
  {
	enum PWR_DOMAIN pw;
	unsigned int cpu, sdx, ldx, idx;

	sdx=sprintf(out,"CPU Freq(MHz) VID  Vcore  TMP(%c)"		\
			"    Accumulator       Energy(J)     Power(W)\n",
			Setting.fahrCels ? 'F' : 'C' );

	ldx=sprintf(row,"\n" "%.*sPackage%.*sCores%.*sUncore%.*sMemory" \
			"\n" "Energy(J):",
			14, hSpace, 8, hSpace, 10, hSpace, 9, hSpace);

    while (!BITVAL(Shutdown, SYNC)) {
	while (!BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu=0;(cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
	{
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW))
	  {
		struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	   if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
	    idx+=sprintf(&out[idx],
			"%03u %7.2f %5d  %5.4f  %3u"			\
			"  %018llu  %13.9f %13.9f\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Voltage.VID,
			CFlop->Voltage.Vcore,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			CFlop->Delta.Power.ACCU,
			CFlop->State.Energy,
			CFlop->State.Power);
	   } else {
		idx += sprintf(&out[idx], "%03u        OFF\n", cpu);
	   }
	  }
	}
	memcpy(&out[idx], row, ldx);
	idx += ldx;

	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++) {
		idx += sprintf(&out[idx], "%.*s" "%13.9f", 2, hSpace,
				Shm->Proc.State.Energy[pw].Current);
	}
	memcpy(&out[idx], "\n" "Power(W) :", 11);
	idx += 11;
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++) {
		idx += sprintf(&out[idx], "%.*s" "%13.9f", 2, hSpace,
				Shm->Proc.State.Power[pw].Current);
	}
	out[idx++] = '\n'; out[idx++] = '\n';

	fwrite(out, (size_t) idx, 1, stdout);
    }
  }
  if (out != NULL) {
	free(out);
  }
  if (row != NULL) {
	free(row);
  }
}

void Voltage(void)
{
	char *out = malloc(8 + (Shm->Proc.CPU.Count + 1) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int cpu, sdx, idx;

	sdx = sprintf(out, "CPU Freq(MHz) VID  Min     Vcore   Max\n");

    while (!BITVAL(Shutdown, SYNC)) {
	while (!BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu=0;(cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
	{
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW))
	  {
		struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	   if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
	    idx+=sprintf(&out[idx],
			"%03u %7.2f %5d  %5.4f  %5.4f  %5.4f\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Voltage.VID,
			Shm->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_LOWEST],
			CFlop->Voltage.Vcore,
			Shm->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_HIGHEST]);
	   } else {
		idx += sprintf(&out[idx], "%03u        OFF\n", cpu);
	   }
	  }
	}
	out[idx++] = '\n';

	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

void Power(void)
{
	char *out = malloc(8 + (Shm->Proc.CPU.Count + 5) * MIN_WIDTH);
	char *row = malloc(MIN_WIDTH + 8);
  if (out && row)
  {
	enum PWR_DOMAIN pw;
	unsigned int cpu, sdx, ldx, idx;

	sdx=sprintf(out,"CPU Freq(MHz)" 				\
			"    Accumulator      Min  Energy(J) Max"	\
			"    Min  Power(W)  Max\n" );

	ldx=sprintf(row,"\nEnergy(J)  Package%.*sCores%.*sUncore%.*sMemory\n",
			12, hSpace, 15, hSpace, 14, hSpace);

    while (!BITVAL(Shutdown, SYNC)) {
	while (!BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu=0;(cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
	{
	  if (!BITVAL(Shm->Cpu[cpu].OffLine, HW))
	  {
		struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	   if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
	    idx+=sprintf(&out[idx],
			"%03u %7.2f"					\
			"  %018llu  %6.2f %6.2f %6.2f  %6.2f %6.2f %6.2f\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Delta.Power.ACCU,
			Shm->Cpu[cpu].Sensors.Energy.Limit[SENSOR_LOWEST],
			CFlop->State.Energy,
			Shm->Cpu[cpu].Sensors.Energy.Limit[SENSOR_HIGHEST],
			Shm->Cpu[cpu].Sensors.Power.Limit[SENSOR_LOWEST],
			CFlop->State.Power,
			Shm->Cpu[cpu].Sensors.Power.Limit[SENSOR_HIGHEST]);
	   } else {
		idx += sprintf(&out[idx], "%03u        OFF\n", cpu);
	   }
	  }
	}
	memcpy(&out[idx], row, ldx);
	idx += ldx;

	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++) {
		idx+=sprintf(&out[idx], "%.*s" "%6.2f%6.2f%6.2f",
			pw == PWR_DOMAIN(PKG) ? 1 : 2, hSpace,
			Shm->Proc.State.Energy[pw].Limit[SENSOR_LOWEST],
			Shm->Proc.State.Energy[pw].Current,
			Shm->Proc.State.Energy[pw].Limit[SENSOR_HIGHEST]);
	}
	memcpy(&out[idx], "\n" "Power(W)\n", 11);
	idx += 11;
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++) {
		idx+=sprintf(&out[idx], "%.*s" "%6.2f%6.2f%6.2f",
			pw == PWR_DOMAIN(PKG) ? 1 : 2, hSpace,
			Shm->Proc.State.Power[pw].Limit[SENSOR_LOWEST],
			Shm->Proc.State.Power[pw].Current,
			Shm->Proc.State.Power[pw].Limit[SENSOR_HIGHEST]);
	}
	out[idx++] = '\n'; out[idx++] = '\n';

	fwrite(out, (size_t) idx, 1, stdout);
    }
  }
  if (out != NULL) {
	free(out);
  }
  if (row != NULL) {
	free(row);
  }
}

void Instructions(void)
{
	char *out = malloc(8 + (Shm->Proc.CPU.Count + 1) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int cpu, sdx, idx;

	sdx = sprintf(out, "CPU     IPS            IPC            CPI\n");

    while (!BITVAL(Shutdown, SYNC)) {
	while (!BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu=0;(cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
	{
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, HW)) {
		struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!BITVAL(Shm->Cpu[cpu].OffLine, OS)) {
			idx += sprintf(&out[idx],
					"%03u %12.6f/s %12.6f/c %12.6f/i\n",
					cpu,
					CFlop->State.IPS,
					CFlop->State.IPC,
					CFlop->State.CPI);
		} else {
			idx += sprintf(&out[idx], "%03u\n", cpu);
		}
	    }
	}
	out[idx++] = '\n';

	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

#define TOPO_MATX 13
#define TOPO_MATY 6

void Topology_Std(char *pStr, unsigned int cpu)
{
    if (Shm->Cpu[cpu].Topology.MP.BSP) {
	snprintf(&pStr[ 0], 4+(2*11)+1, "%03u:BSP%5d\x20",
			cpu,
			Shm->Cpu[cpu].Topology.ApicID);
    } else {
	snprintf(&pStr[ 0], 1+(3*11)+1, "%03u:%3d%5d\x20",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.ApicID);
    }
}

void Topology_SMT(char *pStr, unsigned int cpu)
{
	Topology_Std(pStr, cpu);

	snprintf(&pStr[TOPO_MATX+1], 1+(2*11)+1, "%5d\x20\x20%5d\x20",
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);
}

void Topology_CMP(char *pStr, unsigned int cpu)
{
	Topology_Std(pStr, cpu);

	snprintf(&pStr[TOPO_MATX+1], (3*11)+1, "%3u%4d%6d",
			Shm->Cpu[cpu].Topology.Cluster.CMP,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);
}

void Topology_CCD(char *pStr, unsigned int cpu)
{
	Topology_Std(pStr, cpu);

	snprintf(&pStr[TOPO_MATX+1], (4*11)+1, "%3u%3u%4d%3d",
			Shm->Cpu[cpu].Topology.Cluster.CCD,
			Shm->Cpu[cpu].Topology.Cluster.CCX,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);
}

char *TopologyStrOFF[] = {
	"\x20\x20\x20\x20-\x20\x20\x20\x20\x20\x20-\x20",
	"\x20\x20-\x20\x20\x20-\x20\x20\x20\x20\x20-",
	"\x20\x20-\x20\x20-\x20\x20\x20-\x20\x20-"
};

char *TopologyFmtOFF[] = {
	"%03u:\x20\x20-\x20\x20\x20\x20-\x20",
	"\x20\x20\x20\x20\x20\x20\x20-\x20\x20-\x20\x20"
};

char *TopologyHeader[TOPO_MATY] = {
	"CPU Pkg  Apic",
	"  Core/Thread",
	"  Caches     ",
	" (w)rite-Back",
	" (i)nclusive ",
	"             "
};

char *TopologySubHeader[TOPO_MATY] = {
	" #   ID   ID ",
	NULL,
	" L1-Inst Way ",
	" L1-Data Way ",
	"     L2  Way ",
	"     L3  Way "
};

char *TopologyAltSubHeader[] = {
	"   ID     ID ",
	" CMP ID    ID",
	"CCD CCX ID/ID"
};

void Topology(Window *win, CELL_FUNC OutFunc)
{
	ATTRIBUTE *attrib[5] = {
		RSC(TOPOLOGY_COND0).ATTR(),
		RSC(TOPOLOGY_COND1).ATTR(),
		RSC(TOPOLOGY_COND2).ATTR(),
		RSC(TOPOLOGY_COND3).ATTR(),
		RSC(TOPOLOGY_COND4).ATTR()
	};
	char	*strID = malloc(2 * ((4*11) + 1)), *pStrOFF = TopologyStrOFF[0];
	unsigned int cpu = 0, level = 0;
	CUINT cells_per_line = win->matrix.size.wth, *nl = &cells_per_line;

  if (strID != NULL)
  {
	void (*TopologyFunc)(char*, unsigned int) = Topology_SMT;
/* Row Mark */
	PRT(MAP, attrib[2], TopologyHeader[0]);
	PRT(MAP, attrib[2], TopologyHeader[1]);
	PRT(MAP, attrib[2], TopologyHeader[2]);
	PRT(MAP, attrib[2], TopologyHeader[3]);
	PRT(MAP, attrib[2], TopologyHeader[4]);
	PRT(MAP, attrib[2], TopologyHeader[5]);
/* Row Mark */
	PRT(MAP, attrib[2], TopologySubHeader[0]);
    if ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
     || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
      switch (Shm->Proc.ArchID) {
      case AMD_Family_11h:
      case AMD_Family_12h:
      case AMD_Family_14h:
      case AMD_Family_15h:
      case AMD_Family_16h:
      TOPOLOGY_CMP:
	TopologyFunc = Topology_CMP;
	pStrOFF = TopologyStrOFF[1];
	TopologySubHeader[1] = TopologyAltSubHeader[1];
	break;
      case AMD_Family_17h:
      case AMD_Family_18h:
      case AMD_Family_19h:
      case AMD_Zen:
      case AMD_Zen_APU:
      case AMD_ZenPlus:
      case AMD_ZenPlus_APU:
      case AMD_Zen_APU_Dali:
      case AMD_EPYC_Rome:
      case AMD_Zen2_CPK:
      case AMD_Zen2_APU:
      case AMD_Zen2_MTS:
      case AMD_Zen3_VMR:
      case AMD_Zen3_CZN:
	TopologyFunc = Topology_CCD;
	pStrOFF = TopologyStrOFF[2];
	TopologySubHeader[1] = TopologyAltSubHeader[2];
	break;
      default:
       if ( Shm->Proc.Features.Std.ECX.Hyperv ) { /* Virtualized ?	*/
	goto TOPOLOGY_CMP;
       }  else  {	/*	AMD_Family_0Fh and AMD_Family_10h	*/
	TopologySubHeader[1] = TopologyAltSubHeader[0];
       }
	break;
      }
    } else {
	TopologySubHeader[1] = TopologyAltSubHeader[0];
    }
	PRT(MAP, attrib[2], TopologySubHeader[1]);
	PRT(MAP, attrib[2], TopologySubHeader[2]);
	PRT(MAP, attrib[2], TopologySubHeader[3]);
	PRT(MAP, attrib[2], TopologySubHeader[4]);
	PRT(MAP, attrib[2], TopologySubHeader[5]);
/* Row Mark */
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
    {
	if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
	{
		TopologyFunc(strID, cpu);

		PRT(MAP, attrib[3], &strID[ 0]);
		PRT(MAP, attrib[0], &strID[14]);

	    for (level = 0; level < CACHE_MAX_LEVEL; level++)
	    {
		PRT(MAP, attrib[0], "%8u%3u%c%c",
			Shm->Cpu[cpu].Topology.Cache[level].Size,
			Shm->Cpu[cpu].Topology.Cache[level].Way,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack ?
				'w' : 0x20,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive ?
				'i' : 0x20);
	    }
	} else {
		PRT(MAP, attrib[4], TopologyFmtOFF[0], cpu);
		PRT(MAP, attrib[1], pStrOFF);

	    for (level = 0; level < CACHE_MAX_LEVEL; level++) {
		PRT(MAP,attrib[1], TopologyFmtOFF[1]);
	    }
	}
    }
	free(strID);
  }
}

#define MC_MATX 15	/*	Must be equal to or greater than 14	*/
#define MC_MATY 5	/*	Must be strictly equal to 5		*/

void iSplit(unsigned int sInt, char hInt[]) {
	char fInt[11+1];
	snprintf(fInt, 11+1, "%10u", sInt);
	memcpy((hInt + 0), (fInt + 0), 5); *(hInt + 0 + 5) = '\0';
	memcpy((hInt + 8), (fInt + 5), 5); *(hInt + 8 + 5) = '\0';
}

const char *Header_DDR3[2][MC_MATX] = {
	{
		" Cha ",
		"   CL", "  RCD", "   RP", "  RAS", "  RRD", "  RFC",
		"   WR", " RTPr", " WTPr", "  FAW", "  B2B", "  CWL",
		" CMD ", " REFI"
	},
	{
		"     ",
		" ddWR", " drWR", " srWR", " ddRW", " drRW", " srRW",
		" ddRR", " drRR", " srRR", " ddWW", " drWW", " srWW",
		" CKE ", "  ECC"
	}
};

const char *Footer_DDR3[2][MC_MATX] = {
	{
		NULL,
		" tCL ( CAS Latency ) ",
		" tRCD ( Activate to CAS ) ",
		" tRP ( RAS Precharge to Activate ) ",
		" tRAS ( Activate to Precharge ) ",
		" tRRD ( Activate to Activate, Same Rank ) ",
		" tRFC ( Refresh to Refresh ) ",
		" tWR ( Write Recovery ) ",
		" tRTPr ( Read CAS to Precharge ) ",
		" tWTPr ( Write CAS to Precharge ) ",
		" tFAW ( Four Activate Window ) ",
		" B2B ( CAS commands spacing ) ",
		" tCWL ( CAS Write Latency ) ",
		" CMD ( Command Rate ) ",
		" tREFI ( Refresh Interval ) "
	},
	{
		NULL,
		" tddWrTRd ( Write to Read, Different DIMM ) ",
		" tdrWrTRd ( Write to Read, Different Rank, Same DIMM ) ",
		" tsrWrTRd ( Write to Read, Same Rank ) ",
		" tddRdTWr ( Read to Write, Different DIMM ) ",
		" tdrRdTWr ( Read to Write, Different Rank, Same DIMM ) ",
		" tsrRdTWr ( Read to Write, Same Rank ) ",
		" tddRdTRd ( Read to Read, Different DIMM ) ",
		" tdrRdTRd ( Read to Read, Different Rank, Same DIMM ) ",
		" tsrRdTRd ( Read to Read, Same Rank ) ",
		" tddWrTWr ( Write to Write, Different DIMM ) ",
		" tdrWrTWr ( Write to Write, Different Rank, Same DIMM ) ",
		" tsrWrTWr ( Write to Write, Same Rank ) ",
		" tCKE ( ClocK Enable ) ",
		" ECC ( Error Correcting Code ) "
	}
};

const char *Header_DDR4[3][MC_MATX] = {
	{
		" Cha ",
		"   CL", "  RCD", "   RP", "  RAS", "  RRD", "  RFC",
		"   WR", " RTPr", " WTPr", "  FAW", "  B2B", "  CWL",
		" CMD ", " REFI"
	},
	{
		"     ",
		" sgRR", " dgRR", " drRR", " ddRR", "     ", " sgRW",
		" dgRW", " drRW", " ddRW", "     ", " sgWR", " dgWR",
		" drWR", " ddWR"
	},
	{
		"     ",
		" sgWW", " dgWW", " drWW", " ddWW", "     ", "     ",
		"     ", "     ", "     ", "     ", "     ", "     ",
		" CKE ", "  ECC"
	}
};

const char *Footer_DDR4[3][MC_MATX] = {
	{
		NULL,
		" tCL ( CAS Latency ) ",
		" tRCD ( Activate to CAS ) ",
		" tRP ( RAS Precharge to Activate ) ",
		" tRAS ( Activate to Precharge ) ",
		" tRRD ( Activate to Activate, Same Rank ) ",
		" tRFC ( Refresh to Refresh ) ",
		" tWR ( Write Recovery ) ",
		" tRTPr ( Read CAS to Precharge ) ",
		" tWTPr ( Write CAS to Precharge ) ",
		" tFAW ( Four Activate Window ) ",
		" B2B ( CAS commands spacing ) ",
		" tCWL ( CAS Write Latency ) ",
		" CMD ( Command Rate ) ",
		" tREFI ( Refresh Interval ) "
	},
	{
		NULL,
		" tRDRD ( Read to Read, Same Bank ) ",
		" tRDRD ( Read to Read, Different Bank ) ",
		" tRDRD ( Read to Read, Different Rank ) ",
		" tRDRD ( Read to Read, Different DIMM ) ",
		NULL,
		" tRDWR ( Read to Write, Same Bank ) ",
		" tRDWR ( Read to Write, Different Bank ) ",
		" tRDWR ( Read to Write, Different Rank ) ",
		" tRDWR ( Read to Write, Different DIMM ) ",
		NULL,
		" tWRRD ( Write to Read, Same Bank ) ",
		" tWRRD ( Write to Read, Different Bank ) ",
		" tWRRD ( Write to Read, Different Rank ) ",
		" tWRRD ( Write to Read, Different DIMM ) "
	},
	{
		NULL,
		" tWRWR ( Write to Write, Same Bank ) ",
		" tWRWR ( Write to Write, Different Bank ) ",
		" tWRWR ( Write to Write, Different Rank ) ",
		" tWRWR ( Write to Write, Different DIMM ) ",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		" tCKE ( ClocK Enable ) ",
		" ECC ( Error Correcting Code ) "
	}
};

const char *Header_DDR4_Zen[3][MC_MATX] = {
	{
		" Cha ",
		"  CL ", " RCDR", " RCDW", "  RP ", " RAS ", "  RC ",
		" RRDS", " RRDL", " FAW ", " WTRS", " WTRL", "  WR ",
		" clRR", " clWW"
	},
	{
		"     ",
		" CWL ", " RTP ", "RdWr ", "WrRd ", "scWW ", "sdWW ",
		"ddWW ", "scRR ", "sdRR ", "ddRR ", "drRR ", "drWW ",
		"drWR ", "drRRD"
	},
	{
		"     ",
		" REFI", " RFC1", " RFC2", " RFC4", " RCPB", " RPPB", " sFAW",
		" dFAW", " Ban ", " Page", "  CKE", "  CMD", "  GDM", "  ECC"
	}
};

const char *Footer_DDR4_Zen[3][MC_MATX] = {
	{
		NULL,
		" tCL ( CAS Latency ) ",
		" tRCD_R ( Activate to Read CAS ) ",
		" tRCD_W ( Activate to Write CAS ) ",
		" tRP ( RAS Precharge to Activate ) ",
		" tRAS ( Activate to Precharge ) ",
		" tRC ( Activate to Activate ) ",
		" tRRD_S ( Activate to Activate, Different Bank Group ) ",
		" tRRD_L ( Activate to Activate, Same Bank Group ) ",
		" tFAW ( Four Activate Window ) ",
		" tWTR_S ( Write to Read, Different Bank Group ) ",
		" tWTR_L ( Write to Read, Same Bank Group ) ",
		" tWR ( Write Recovery ) ",
		" tRDRD[SCL] ( Read to Read, Same Bank Group ) ",
		" tWRWR[SCL] ( Write to Write, Same Bank Group ) "
	},
	{
		NULL,
		" tCWL ( CAS Write Latency ) ",
		" tRTP ( Read To Precharge ) ",
		" tRDWR ( Read Write Command Spacing ) ",
		" tWRRD ( Write Read Command Spacing ) ",
		" tWRWR[SC] ( tWRWR, Different Bank Group ) ",
		" tWRWR[SD] ( tWRWR, Different Rank) ",
		" tWRWR[DD] ( tWRWR, Different DIMM ) ",
		" tRDRD[SC] ( tRDRD, Different Bank Group ) ",
		" tRDRD[SD] ( tRDRD, Different Rank ) ",
		" tRDRD[DD] ( tRDRD, Different DIMM ) ",
		" tRTR[DLR] ( tRTR, Different Logical Rank ) ",
		" tWTW[DLR] ( tWTR, Different Logical Rank ) ",
		" tWTR[DLR] ( tWTR, Different Logical Rank ) ",
		" tRRD[DLR] ( tRRD, Different Logical Rank ) "
	},
	{
		NULL,
		" tREFI ( Refresh Interval ) ",
		" tRFC1 ( Refresh to Refresh, 1X mode ) ",
		" tRFC2 ( Refresh to Refresh, 2X mode ) ",
		" tRFC4 ( Refresh to Refresh, 4X mode ) ",
		" tRCPB ( Row Cycle Time, Per-Bank ) ",
		" tRPPB ( Row Precharge Time, Per-Bank ) ",
		" tFAW[SLR] ( tFAW, Same Logical Rank ) ",
		" tFAW[DLR] ( tFAW, Different Logical Rank ) ",
		" tBAN ( Timing Ban, RTR | WTW ) ",
		" tRCPage ( Row Cycle Page Time ) ",
		" tCKE ( ClocK Enable ) ",
		" CMD ( Command Rate ) ",
		" GDM ( Gear Down Mode ) ",
		" ECC ( Error Correcting Code ) "
	}
};

typedef void (*TIMING_FUNC)(Window*, CELL_FUNC, CUINT*, unsigned short);

void Timing_DDR3(Window *win, CELL_FUNC OutFunc, CUINT *nl, unsigned short mc)
{
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	CUINT	nc;
	unsigned short cha;

  for (nc = 0; nc < MC_MATX; nc++) {
	GridHover(PRT(IMC, attrib[0], Header_DDR3[0][nc]), Footer_DDR3[0][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
	PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);
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

	PRT(IMC, attrib[1], "%3uT\x20",
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);

	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI);
  }
  for (nc = 0; nc < MC_MATX; nc++) {
	GridHover(PRT(IMC, attrib[0], Header_DDR3[1][nc]), Footer_DDR3[1][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
    PRT(IMC, attrib[0],"\x20\x20#%-2u", cha);
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

    PRT(IMC,attrib[1],"%4u\x20",Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE);

    PRT(IMC, attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.ECC);
  }
}

void Timing_DDR4(Window *win, CELL_FUNC OutFunc, CUINT *nl, unsigned short mc)
{
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	CUINT	nc;
	unsigned short cha;

  for (nc = 0; nc < MC_MATX; nc++) {
	GridHover(PRT(IMC, attrib[0], Header_DDR4[0][nc]), Footer_DDR4[0][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
	PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);
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

	PRT(IMC, attrib[1], "%3uT\x20",
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);

	PRT(IMC, attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI);
  }
  for (nc = 0; nc < MC_MATX; nc++) {
	GridHover(PRT(IMC, attrib[0], Header_DDR4[1][nc]), Footer_DDR4[1][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
    PRT(IMC,attrib[0],"\x20\x20#%-2u", cha);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_SG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DR);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DD);

	PRT(IMC,attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);

  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_SG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DR);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DD);

	PRT(IMC,attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);

  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_SG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DR);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DD);
  }
  for (nc = 0; nc < MC_MATX; nc++) {
	GridHover(PRT(IMC, attrib[0], Header_DDR4[2][nc]), Footer_DDR4[2][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
    PRT(IMC,attrib[0],"\x20\x20#%-2u", cha);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_SG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DG);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DR);
  PRT(IMC,attrib[1],"%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DD);

    for (nc = 0; nc < 8; nc++) {
	PRT(IMC,attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
    }

    PRT(IMC,attrib[1],"%4u\x20",Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE);

    PRT(IMC, attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.ECC);
  }
}

void Timing_DDR4_Zen(Window *win,CELL_FUNC OutFunc,CUINT *nl, unsigned short mc)
{
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	CUINT	nc;
	unsigned short cha;

  for (nc = 0; nc < MC_MATX; nc++) {
    GridHover(PRT(IMC,attrib[0],Header_DDR4_Zen[0][nc]),Footer_DDR4_Zen[0][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
    PRT(IMC,attrib[0], "\x20\x20#%-2u", cha);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tCL);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD_RD);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD_WR);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRP);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS);

    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRC);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRRDS);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRRDL);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW);

    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tWTRS);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tWTRL);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tWR);

    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRdRdScl);
    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrWrScl);
  }
  for (nc = 0; nc < MC_MATX; nc++) {
    GridHover(PRT(IMC,attrib[0],Header_DDR4_Zen[1][nc]),Footer_DDR4_Zen[1][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
    PRT(IMC,attrib[0],"\x20\x20#%-2u", cha);

    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL);
    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRTP);

  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddRdTWr);
  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddWrTRd);

  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tscWrTWr);
  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tsdWrTWr);
  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddWrTWr);

  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tscRdTRd);
  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tsdRdTRd);
  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddRdTRd);

PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRdRdScDLR);
PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrWrScDLR);
PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrRdScDLR);
  PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRRDDLR);
  }
  for (nc = 0; nc < MC_MATX; nc++) {
    GridHover(PRT(IMC,attrib[0],Header_DDR4_Zen[2][nc]),Footer_DDR4_Zen[2][nc]);
  }
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
    PRT(IMC,attrib[0], "\x20\x20#%-2u", cha);
    PRT(IMC,attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI);
    PRT(IMC,attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC1);
    PRT(IMC,attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC2);
    PRT(IMC,attrib[1], "%5u",Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC4);

    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRCPB);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRPPB);

    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tFAWSLR);
    PRT(IMC,attrib[1], "%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tFAWDLR);

    PRT(IMC,attrib[1], "\x20R%1uW%1u",
			Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRdRdBan,
			Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrWrBan);

    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tRCPage);

    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE);

    PRT(IMC,attrib[1],"%3uT ",Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);

    PRT(IMC,attrib[1],"\x20\x20%3s",
			ENABLED(Shm->Uncore.MC[mc].Channel[cha].Timing.GDM));

    PRT(IMC,attrib[1],"%4u ",Shm->Uncore.MC[mc].Channel[cha].Timing.ECC);
  }
}

void MemoryController(Window *win, CELL_FUNC OutFunc, TIMING_FUNC TimingFunc)
{
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	char *MC_Unit[4] =
	{
		[0b00] = " MHz ",
		[0b01] = " MT/s",
		[0b10] = " MB/s",
		[0b11] = (char *) RSC(MEM_CTRL_BLANK).CODE()
	};
	char	item[MC_MATY + 1], *str = malloc(CODENAME_LEN + 4 + 10),
		*chipStr = malloc((MC_MATX * MC_MATY) + 1);
  if ((str != NULL) && (chipStr != NULL))
  {
	CUINT	cells_per_line = win->matrix.size.wth, *nl = &cells_per_line,
		ni, nc, li;

	unsigned short mc, cha, slot;

	li = snprintf(	str, CODENAME_LEN + 4 + 10, "%s  [%4hX]",
			Shm->Uncore.Chipset.CodeName, Shm->Uncore.ChipID );

	nc = (MC_MATX * MC_MATY) - li;
	ni = nc >> 1;
	nc = ni + (nc & 0x1);

	li = snprintf(	chipStr, (MC_MATX * MC_MATY) + 1, "%*.s" "%s" "%*.s",
			nc, HSPACE, str, ni, HSPACE );

    for (nc = 0; nc < MC_MATX; nc++) {
	memcpy(item, &chipStr[nc * MC_MATY], MC_MATY);
	item[MC_MATY] = '\0';
	PRT(IMC, attrib[1], item);
    }
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT1_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT1_1).CODE());
	PRT(IMC, attrib[1], RSC(MEM_CTRL_SUBSECT1_2).CODE(), mc);

      for (nc = 0; nc < (MC_MATX - 6); nc++) {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
      }

	switch (Shm->Uncore.MC[mc].ChannelCount) {
	case 1:
		PRT(IMC, attrib[1], RSC(MEM_CTRL_SINGLE_CHA_0).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_SINGLE_CHA_1).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_SINGLE_CHA_2).CODE());
		break;
	case 2:
		PRT(IMC, attrib[1], RSC(MEM_CTRL_DUAL_CHA_0).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_DUAL_CHA_1).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_DUAL_CHA_2).CODE());
		break;
	case 3:
		PRT(IMC, attrib[1], RSC(MEM_CTRL_TRIPLE_CHA_0).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_TRIPLE_CHA_1).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_TRIPLE_CHA_2).CODE());
		break;
	case 4:
		PRT(IMC, attrib[1], RSC(MEM_CTRL_QUAD_CHA_0).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_QUAD_CHA_1).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_QUAD_CHA_2).CODE());
		break;
	case 6:
		PRT(IMC, attrib[1], RSC(MEM_CTRL_SIX_CHA_0).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_SIX_CHA_1).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_SIX_CHA_2).CODE());
		break;
	case 8:
		PRT(IMC, attrib[1], RSC(MEM_CTRL_EIGHT_CHA_0).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_EIGHT_CHA_1).CODE());
		PRT(IMC, attrib[1], RSC(MEM_CTRL_EIGHT_CHA_2).CODE());
		break;
	default:
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
		break;
	}

	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_RATE_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_RATE_1).CODE());
	PRT(IMC, attrib[1], "%5llu", Shm->Uncore.Bus.Rate);
	PRT(IMC, attrib[0], MC_Unit[Shm->Uncore.Unit.Bus_Rate], MC_MATY,HSPACE);
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);

	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_SPEED_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_SPEED_1).CODE());
	PRT(IMC, attrib[1], "%5llu", Shm->Uncore.Bus.Speed);
	PRT(IMC, attrib[0], MC_Unit[Shm->Uncore.Unit.BusSpeed], MC_MATY,HSPACE);

      for (nc = 0; nc < (MC_MATX - 13); nc++) {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
      }

	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_SPEED_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_SPEED_1).CODE());
	PRT(IMC, attrib[1], "%5llu", Shm->Uncore.CtrlSpeed);
	PRT(IMC, attrib[0], MC_Unit[Shm->Uncore.Unit.DDRSpeed], MC_MATY,HSPACE);


      for (nc = 0; nc < MC_MATX; nc++) {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
      }

	TimingFunc(win, OutFunc, nl, mc);

      for (nc = 0; nc < MC_MATX; nc++) {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
      }

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_1).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_2).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_3).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_4).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_5).CODE(), cha);
	for (nc = 0; nc < (MC_MATX - 6); nc++) {
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
	}

	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SLOT).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_BANK).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_RANK).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY, HSPACE);
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_ROW).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_COLUMN0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_COLUMN1).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_1).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_2).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_3).CODE());
	for (nc = 0; nc < (MC_MATX - 12); nc++) {
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
	}

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
		PRT(IMC, attrib[0], "\x20\x20#%-2u", slot);
	  if (Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size > 0)
	  {
		PRT(IMC, attrib[1],
		"%5u",Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks);
		PRT(IMC, attrib[1],
		"%5u",Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows, str);
		PRT(IMC, attrib[1], "%5s", &str[0]);
		PRT(IMC, attrib[1], "%5s", &str[8]);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols, str);
		PRT(IMC, attrib[1], "%5s", &str[0]);
		PRT(IMC, attrib[1], "%5s", &str[8]);
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size, str);
		PRT(IMC, attrib[1], "%5s", &str[0]);
		PRT(IMC, attrib[1], "%5s", &str[8]);
	  }
	  else for (nc = 0; nc < 9; nc++) {
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
	  }
	  for (nc = 0; nc < (MC_MATX - 11); nc++) {
		PRT(IMC, attrib[0], RSC(MEM_CTRL_BLANK).CODE(), MC_MATY,HSPACE);
	  }
	}
      }
    }
  }
  if (str != NULL) {
	free(str);
  }
  if (chipStr != NULL) {
	free(chipStr);
  }
}

/* >>> GLOBALS >>> */
static char *Buffer = NULL; /* Memory allocation for a mono-thread context */

Coordinate *cTask = NULL;

CardList cardList = {.head = NULL, .tail = NULL};

struct DRAW_ST draw = {
	.Cache = {
	    #ifndef NO_HEADER
		.TopAvg = 0.0,
	    #endif
	    #ifndef NO_FOOTER
		.FreeRAM= 0,
		.TaskCount = 0
	    #endif
	},
	.Flag = {
		.layout = 0,
		.clear	= 0,
		.height = 0,
		.width	= 0,
		.daemon = 0,
		.taskVal= 0,
		.avgOrPC= 0,
		.clkOrLd= 0,
	    #if defined(UBENCH) && UBENCH == 1
		.uBench = 0,
	    #endif
		._padding=0
	},
	.View		= V_FREQ,
	.Disposal	= D_MAINVIEW,
	.Size		= { .width = 0, .height = 0 },
	.Area		= { .MinHeight = 0, .MaxRows = 0, .LoadWidth = 0 },
	.iClock 	= 0,
	.cpuScroll	= 0,
	.Load		= 0,
	.Unit		= { .Memory = 0 },
	.SmbIndex	= SMB_BOARD_NAME
};

enum THERM_PWR_EVENTS processorEvents = EVENT_THERM_NONE;

struct RECORDER_ST Recorder = {
		.Reset = 0,
		.Select = 1,
		.Ratios = {0, 1, 2, 10, 20, 60, 90, 120, 240, 0}
};
/* <<< GLOBALS <<< */

int ByteReDim(unsigned long ival, int constraint, unsigned long *oval)
{
	int base = 1 + (int) log10(ival);

	(*oval) = ival;
	if (base > constraint) {
		(*oval) = (*oval) >> 10;
		return (1 + ByteReDim((*oval), constraint, oval));
	} else {
		return (0);
	}
}

#define Threshold(value, threshold1, threshold2, _low, _medium, _high)	\
({									\
	enum PALETTE _ret;						\
	if (value > threshold2) {					\
		_ret = _high;						\
	} else if (value > threshold1) {				\
		_ret = _medium;						\
	} else {							\
		_ret = _low;						\
	}								\
	_ret;								\
})

#define frtostr(r, d, pStr)						\
({									\
	int p = d - ((int) log10(fabs(r))) - 2;				\
	snprintf(pStr, d+1, "%*.*f", d, p, r);				\
	pStr;								\
})

#define Clock2LCD(layer, col, row, value1, value2)			\
({									\
	snprintf(Buffer, 4+1, "%04.0f", value1);			\
	PrintLCD(layer, col, row, 4, Buffer,				\
	    Threshold(value2,Ruler.Minimum,Ruler.Median,_GREEN,_YELLOW,_RED));\
})

#define Counter2LCD(layer, col, row, value)				\
({									\
	snprintf(Buffer, 4+1, "%04.0f" , value);			\
	PrintLCD(layer, col, row, 4, Buffer,				\
		Threshold(value, 0.f, 1.f, _RED,_YELLOW,_WHITE));	\
})

#define Load2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, Buffer),		\
		Threshold(value, 100.f/3.f, 100.f/1.5f, _WHITE,_YELLOW,_RED))

#define Idle2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, Buffer),		\
		Threshold(value, 100.f/3.f, 100.f/1.5f, _YELLOW,_WHITE,_GREEN))

#define Sys2LCD(layer, col, row, value) 				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, Buffer),		\
		Threshold(value, 100.f/6.6f, 50.0, _RED,_YELLOW,_WHITE))

void ForEachCellPrint_Menu(Window *win, void *plist)
{
	WinList *list = (WinList *) plist;
	CUINT col, row;
	size_t len;

    if (win->lazyComp.rowLen == 0) {
	for (col = 0; col < win->matrix.size.wth; col++) {
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;
	}
    }
    if (win->matrix.origin.col > 0) {
	LayerFillAt(	win->layer,
			0,
			win->matrix.origin.row,
			win->matrix.origin.col, hSpace,
			win->hook.color[0].title);
    }
    for (col = 0; col < win->matrix.size.wth; col++) {
	PrintContent(win, list, col, 0);
    }
    for (row = 1; row < win->matrix.size.hth; row++) {
	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID) {
			PrintContent(win, list, win->matrix.select.col, row);
		}
    }
    if ((len = draw.Size.width-win->lazyComp.rowLen-win->matrix.origin.col) > 0)
    {
	LayerFillAt(	win->layer,
			(win->matrix.origin.col + win->lazyComp.rowLen),
			win->matrix.origin.row,
			len, hSpace,
			win->hook.color[0].title);
    }
}

Window *CreateMenu(unsigned long long id, CUINT matrixSelectCol)
{
	Window *wMenu = CreateWindow(	wLayer, id,
					3, 13, 3, 0,
					  WINFLAG_NO_STOCK
					| WINFLAG_NO_SCALE
					| WINFLAG_NO_BORDER );
    if (wMenu != NULL)
    {
/* Top Menu */
	StoreTCell(wMenu, SCANKEY_NULL, RSC(MENU_ITEM_MENU).CODE(),
					RSC(MENU_ITEM_MENU).ATTR());

	StoreTCell(wMenu, SCANKEY_NULL, RSC(MENU_ITEM_VIEW).CODE(),
					RSC(MENU_ITEM_VIEW).ATTR());

	StoreTCell(wMenu, SCANKEY_NULL, RSC(MENU_ITEM_WINDOW).CODE(),
					RSC(MENU_ITEM_WINDOW).ATTR());
/* Row  1 */
	StoreTCell(wMenu, SCANKEY_F1,	RSC(MENU_ITEM_KEYS).CODE(),
					RSC(CREATE_MENU_FN_KEY).ATTR());

	StoreTCell(wMenu, SCANKEY_d,	RSC(MENU_ITEM_DASHBOARD).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_p,	RSC(MENU_ITEM_PROCESSOR).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  2 */
	StoreTCell(wMenu, SCANKEY_SHIFT_l, RSC(MENU_ITEM_LANG).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_f,	RSC(MENU_ITEM_FREQUENCY).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_m,	RSC(MENU_ITEM_TOPOLOGY).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  3 */
	StoreTCell(wMenu, SCANKEY_s,	RSC(MENU_ITEM_SETTINGS).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_i,	RSC(MENU_ITEM_INST_CYCLES).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_e,	RSC(MENU_ITEM_FEATURES).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  4 */
	StoreTCell(wMenu, SCANKEY_SHIFT_b, RSC(MENU_ITEM_SMBIOS).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_c,	RSC(MENU_ITEM_CORE_CYCLES).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_i, RSC(MENU_ITEM_ISA_EXT).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  5 */
	StoreTCell(wMenu, SCANKEY_k,	RSC(MENU_ITEM_KERNEL).CODE(),
			BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) ?
					RSC(CREATE_MENU_SHORTKEY).ATTR()
					: RSC(CREATE_MENU_DISABLE).ATTR());

	StoreTCell(wMenu, SCANKEY_l,	RSC(MENU_ITEM_IDLE_STATES).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_t,	RSC(MENU_ITEM_TECH).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  6 */
	StoreTCell(wMenu, SCANKEY_HASH, RSC(MENU_ITEM_HOTPLUG).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_g,	RSC(MENU_ITEM_PKG_CYCLES).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_o,	RSC(MENU_ITEM_PERF_MON).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  7 */
	StoreTCell(wMenu, SCANKEY_SHIFT_o, RSC(MENU_ITEM_TOOLS).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_x,	RSC(MENU_ITEM_TASKS_MON).CODE(),
			#ifndef NO_LOWER
			BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) ?
					RSC(CREATE_MENU_SHORTKEY).ATTR()
					: RSC(CREATE_MENU_DISABLE).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_w,	RSC(MENU_ITEM_POW_THERM).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  8 */
	StoreTCell(wMenu, SCANKEY_a,	RSC(MENU_ITEM_ABOUT).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_q,	RSC(MENU_ITEM_SYS_INTER).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_u,	RSC(MENU_ITEM_CPUID).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  9 */
	StoreTCell(wMenu, SCANKEY_h,	RSC(MENU_ITEM_HELP).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

	StoreTCell(wMenu, SCANKEY_SHIFT_c, RSC(MENU_ITEM_SENSORS).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_r, RSC(MENU_ITEM_SYS_REGS).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row 10 */
	StoreTCell(wMenu, SCANKEY_CTRL_x, RSC(MENU_ITEM_QUIT).CODE(),
					  RSC(CREATE_MENU_CTRL_KEY).ATTR());

	StoreTCell(wMenu, SCANKEY_SHIFT_v, RSC(MENU_ITEM_VOLTAGE).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_m, RSC(MENU_ITEM_MEM_CTRL).CODE(),
				Shm->Uncore.CtrlCount > 0 ?
					RSC(CREATE_MENU_SHORTKEY).ATTR()
					: RSC(CREATE_MENU_DISABLE).ATTR());
/* Row 11 */
	StoreTCell(wMenu, SCANKEY_VOID, "", vColor);

	StoreTCell(wMenu, SCANKEY_SHIFT_w, RSC(MENU_ITEM_POWER).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_VOID, "", vColor);
/* Row 12 */
	StoreTCell(wMenu, SCANKEY_VOID, "", vColor);

	StoreTCell(wMenu, SCANKEY_SHIFT_t, RSC(MENU_ITEM_SLICE_CTRS).CODE(),
			#ifndef NO_LOWER
					RSC(CREATE_MENU_SHORTKEY).ATTR());
			#else
					RSC(CREATE_MENU_DISABLE).ATTR());
			#endif

	StoreTCell(wMenu, SCANKEY_VOID, "", vColor);
/* Bottom Menu */
	StoreWindow(wMenu, .color[0].select,	MakeAttr(BLACK, 0, WHITE, 0));
	StoreWindow(wMenu, .color[0].title,	MakeAttr(BLACK, 0, WHITE, 0));
	StoreWindow(wMenu, .color[1].title,	MakeAttr(BLACK, 0, WHITE, 1));

	wMenu->matrix.select.col = matrixSelectCol;

	StoreWindow(wMenu,	.Print,		ForEachCellPrint_Menu);
	StoreWindow(wMenu,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wMenu,	.key.Left,	MotionLeft_Menu);
	StoreWindow(wMenu,	.key.Right,	MotionRight_Menu);
	StoreWindow(wMenu,	.key.Down,	MotionDown_Menu);
	StoreWindow(wMenu,	.key.Up,	MotionUp_Menu);
	StoreWindow(wMenu,	.key.Home,	MotionHome_Menu);
	StoreWindow(wMenu,	.key.End,	MotionEnd_Menu);
    }
	return (wMenu);
}

void IntervalUpdate(TGrid *grid, DATA_TYPE data)
{
	UNUSED(data);
	snprintf(Buffer, 10+1, "%4u", Shm->Sleep.Interval);
	memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
}

void SysTickUpdate(TGrid *grid, DATA_TYPE data)
{
	UNUSED(data);
	snprintf(Buffer, 10+1,"%4u",Shm->Sleep.Interval*Shm->SysGate.tickReset);
	memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
}

void SvrWaitUpdate(TGrid *grid, DATA_TYPE data)
{
	snprintf(Buffer, 21+1, "%4ld", (*data.pslong) / 1000000L);
	memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
}

void RecorderUpdate(TGrid *grid, DATA_TYPE data)
{
	int duration = RECORDER_SECONDS((*data.psint), Shm->Sleep.Interval);
	if (duration <= 9999) {
		snprintf(Buffer, 11+1, "%4d", duration);
		memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
	}
}

void SettingUpdate(TGrid *grid, const int bix, const int pos,
				const size_t len, const char *item)
{
	ATTRIBUTE *attrib[2] = {
		RSC(CREATE_SETTINGS_COND0).ATTR(),
		RSC(CREATE_SETTINGS_COND1).ATTR()
	};
	memcpy(&grid->cell.attr[pos], &attrib[bix][pos], len);
	memcpy(&grid->cell.item[pos], item, len);
}

void AutoClockUpdate(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = (Shm->Registration.AutoClock & 0b10) != 0;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void ExperimentalUpdate(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Registration.Experimental != 0;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void HotPlug_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = !(Shm->Registration.HotPlug < 0);
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void PCI_Probe_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Registration.PCI == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void NMI_Registration_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = BITWISEAND(	LOCKLESS,
						Shm->Registration.NMI,
						BIT_NMI_MASK ) != 0;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void CPU_Idle_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Registration.Driver.CPUidle
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void CPU_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Registration.Driver.CPUfreq
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void Governor_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Registration.Driver.Governor
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void ClockSource_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int bix = Shm->Registration.Driver.CS
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void ScopeUpdate(TGrid *grid, DATA_TYPE data)
{
	ASCII *code[] = {
		RSC(SCOPE_NONE).CODE(),
		RSC(SCOPE_THREAD).CODE(),
		RSC(SCOPE_CORE).CODE(),
		RSC(SCOPE_PACKAGE).CODE()
	};
	const enum FORMULA_SCOPE scope = SCOPE_OF_FORMULA(*data.psint);
	memcpy(&grid->cell.item[grid->cell.length - RSZ(SCOPE_NONE) - 2],
		code[scope], RSZ(SCOPE_NONE));
}

Window *CreateSettings(unsigned long long id)
{
	Window *wSet = CreateWindow(wLayer, id, 1, 24, 8, TOP_HEADER_ROW+2);
    if (wSet != NULL)
    {
	ATTRIBUTE *attrib[2] = {
		RSC(CREATE_SETTINGS_COND0).ATTR(),
		RSC(CREATE_SETTINGS_COND1).ATTR()
	};
	size_t length = strlen(Shm->ShmName);
	unsigned int bix;

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	StoreTCell(wSet, SCANKEY_NULL,  RSC(SETTINGS_DAEMON).CODE(),
					MAKE_PRINT_UNFOCUS);

	GridCall( StoreTCell(	wSet, OPS_INTERVAL,
				RSC(SETTINGS_INTERVAL).CODE(),
				MAKE_PRINT_UNFOCUS ),
		IntervalUpdate );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				" Sys. Tick(ms)                  ",
				MAKE_PRINT_UNFOCUS ),
		SysTickUpdate );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				" Poll Wait(ms)                  ",
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &Shm->Sleep.pollingWait.tv_nsec );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				" Ring Wait(ms)                  ",
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &Shm->Sleep.ringWaiting.tv_nsec );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				" Child Wait(ms)                 ",
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &Shm->Sleep.childWaiting.tv_nsec );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				" Slice Wait(ms)                 ",
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &Shm->Sleep.sliceWaiting.tv_nsec );

	GridCall( StoreTCell(	wSet, OPS_RECORDER,
				RSC(SETTINGS_RECORDER).CODE(),
				MAKE_PRINT_UNFOCUS ),
		RecorderUpdate, &Recorder.Reset );

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	bix = ((Shm->Registration.AutoClock & 0b10) != 0);
	GridCall( StoreTCell( wSet, OPS_AUTOCLOCK,
			RSC(SETTINGS_AUTO_CLOCK).CODE(),
			attrib[bix] ),
		AutoClockUpdate );

	bix = (Shm->Registration.Experimental != 0);
	GridCall( StoreTCell(	wSet, OPS_EXPERIMENTAL,
				RSC(SETTINGS_EXPERIMENTAL).CODE(),
				attrib[bix] ),
		ExperimentalUpdate );

	bix = !(Shm->Registration.HotPlug < 0);
	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_CPU_HOTPLUG).CODE(),
				attrib[bix] ),
		HotPlug_Update );

	bix = (Shm->Registration.PCI == 1);
	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_PCI_ENABLED).CODE(),
				attrib[bix] ),
		PCI_Probe_Update );

	bix = BITWISEAND(LOCKLESS, Shm->Registration.NMI, BIT_NMI_MASK) != 0;
	GridCall( StoreTCell(	wSet, OPS_INTERRUPTS,
				RSC(SETTINGS_NMI_REGISTERED).CODE(),
				attrib[bix] ),
		NMI_Registration_Update );

	bix = Shm->Registration.Driver.CPUidle & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_CPU_IDLE,
				RSC(SETTINGS_CPUIDLE_REGISTERED).CODE(),
				attrib[bix] ),
		CPU_Idle_Update );

	bix = Shm->Registration.Driver.CPUfreq & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_CPU_FREQ,
				RSC(SETTINGS_CPUFREQ_REGISTERED).CODE(),
				attrib[bix] ),
		CPU_Freq_Update );

	bix = Shm->Registration.Driver.Governor & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_GOVERNOR,
				RSC(SETTINGS_GOVERNOR_REGISTERED).CODE(),
				attrib[bix] ),
		Governor_Update );

	bix = Shm->Registration.Driver.CS & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_CLOCK_SOURCE,
				RSC(SETTINGS_CS_REGISTERED).CODE(),
				attrib[bix] ),
		ClockSource_Update );

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	GridCall( StoreTCell(	wSet, OPS_THERMAL_SCOPE,
				RSC(SETTINGS_THERMAL_SCOPE).CODE(),
				MAKE_PRINT_UNFOCUS ),
		ScopeUpdate, &Shm->Proc.thermalFormula );

	GridCall( StoreTCell(	wSet, OPS_VOLTAGE_SCOPE,
				RSC(SETTINGS_VOLTAGE_SCOPE).CODE(),
				MAKE_PRINT_UNFOCUS ),
		ScopeUpdate, &Shm->Proc.voltageFormula );

	GridCall( StoreTCell(	wSet, OPS_POWER_SCOPE,
				RSC(SETTINGS_POWER_SCOPE).CODE(),
				MAKE_PRINT_UNFOCUS ),
		ScopeUpdate, &Shm->Proc.powerFormula );

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	memcpy(&TCellAt(wSet, 0, 1).item[31 - length], Shm->ShmName, length);

	StoreWindow(wSet, .title, (char*) RSC(SETTINGS_TITLE).CODE());

	StoreWindow(wSet,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wSet,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wSet,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wSet,	.key.WinUp,	MotionOriginUp_Win);
	StoreWindow(wSet,	.key.Enter,	Enter_StickyCell);
	StoreWindow(wSet,	.key.Down,	MotionDown_Win);
	StoreWindow(wSet,	.key.Up,	MotionUp_Win);
	StoreWindow(wSet,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wSet,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wSet,	.key.Home,	MotionReset_Win);
	StoreWindow(wSet,	.key.End,	MotionEnd_Cell);
	StoreWindow(wSet,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wSet,	.key.Expand,	MotionExpand_Win);
    }
	return (wSet);
}

Window *CreateHelp(unsigned long long id)
{
	Window *wHelp = CreateWindow(wLayer, id, 2, 19, 2, TOP_HEADER_ROW+1);
    if (wHelp != NULL)
    {
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [F2]             ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MENU).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_ESCAPE).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_CLOSE_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_SHIFT_TAB).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_PREV_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_TAB).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_NEXT_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       [a|z]      ",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [w|q]  [s]  [d] +",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MOVE_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                 +",
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_SIZE_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_UP).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_LEFT_RIGHT).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MOVE_SELECT).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_DOWN).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_END).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_LAST_CELL).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_HOME).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_FIRST_CELL).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_ENTER).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_TRIGGER_SELECT).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_PAGE_UP).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_PREV_PAGE).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_PAGE_DOWN).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_NEXT_PAGE).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_MINUS).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_SCROLL_DOWN).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_PLUS).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_SCROLL_UP).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);

	StoreWindow(wHelp, .title, (char*) RSC(HELP_TITLE).CODE());
	StoreWindow(wHelp, .color[0].select, MAKE_PRINT_UNFOCUS);
	StoreWindow(wHelp, .color[1].select, MAKE_PRINT_FOCUS);

	StoreWindow(wHelp,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wHelp,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wHelp,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wHelp,	.key.WinUp,	MotionOriginUp_Win);
	StoreWindow(wHelp,	.key.Down,	MotionDown_Win);
	StoreWindow(wHelp,	.key.Up,	MotionUp_Win);
	StoreWindow(wHelp,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wHelp,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wHelp,	.key.Home,	MotionReset_Win);
	StoreWindow(wHelp,	.key.End,	MotionEnd_Cell);
	StoreWindow(wHelp,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wHelp,	.key.Expand,	MotionExpand_Win);
    }
	return (wHelp);
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
	{0, RSC(ADV_HELP_SECT_FREQ).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_AVG).CODE(),	{SCANKEY_PERCENT}},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_SECT_TASK).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_ORDER).CODE(),	{SCANKEY_b}	},
	{1, RSC(ADV_HELP_ITEM_SEL).CODE(),	{SCANKEY_n}	},
	{1, RSC(ADV_HELP_ITEM_REV).CODE(),	{SCANKEY_r}	},
	{1, RSC(ADV_HELP_ITEM_HIDE).CODE(),	{SCANKEY_v}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_SECT_ANY).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_POWER).CODE(),	{SCANKEY_DOLLAR}},
	{1, RSC(ADV_HELP_ITEM_TOP).CODE(),	{SCANKEY_DOT}	},
	{1, RSC(ADV_HELP_ITEM_FAHR_CELS).CODE(),{SCANKEY_SHIFT_f}},
	{1, RSC(ADV_HELP_ITEM_PROC_EVENT).CODE(),{SCANKEY_SHIFT_h}},
	{1, RSC(ADV_HELP_ITEM_SECRET).CODE(),	{SCANKEY_SHIFT_y}},
	{1, RSC(ADV_HELP_ITEM_UPD).CODE(),	{SCANKEY_AST}	},
	{1, RSC(ADV_HELP_ITEM_START).CODE(),	{SCANKEY_OPEN_BRACE}},
	{1, RSC(ADV_HELP_ITEM_STOP).CODE(),	{SCANKEY_CLOSE_BRACE}},
	{1, RSC(ADV_HELP_ITEM_TOOLS).CODE(),	{SCANKEY_F10}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_ITEM_TERMINAL).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_PRT_SCR).CODE(),	{SCANKEY_CTRL_p}},
	{1, RSC(ADV_HELP_ITEM_REC_SCR).CODE(),	{SCANKEY_ALT_p} },
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_GO_UP).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_GO_DW).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	}
    };
	const size_t nmemb = sizeof(advHelp) / sizeof(struct ADV_HELP_ST);
	Window *wHelp = CreateWindow(wLayer, id, 1,nmemb,41,TOP_HEADER_ROW+1);
    if (wHelp != NULL)
    {
	unsigned int idx;
	for (idx = 0; idx < nmemb; idx++) {
		StoreTCell(	wHelp,
				advHelp[idx].quick.key,
				advHelp[idx].item,
				attrib[advHelp[idx].theme] );
	}
	StoreWindow(wHelp, .title, (char*) RSC(ADV_HELP_TITLE).CODE());

	StoreWindow(wHelp,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wHelp,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wHelp,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wHelp,	.key.WinUp,	MotionOriginUp_Win);
	StoreWindow(wHelp,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wHelp,	.key.Down,	MotionDown_Win);
	StoreWindow(wHelp,	.key.Up,	MotionUp_Win);
	StoreWindow(wHelp,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wHelp,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wHelp,	.key.Home,	MotionReset_Win);
	StoreWindow(wHelp,	.key.End,	MotionEnd_Cell);
	StoreWindow(wHelp,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wHelp,	.key.Expand,	MotionExpand_Win);
    }
	return (wHelp);
}

Window *CreateAbout(unsigned long long id)
{
	ASCII *C[] = {
		RSC(LOGO_ROW_0).CODE(),
		RSC(LOGO_ROW_1).CODE(),
		RSC(LOGO_ROW_2).CODE(),
		RSC(LOGO_ROW_3).CODE(),
		RSC(LOGO_ROW_4).CODE(),
		RSC(LOGO_ROW_5).CODE()
	} , *F[] = {
		RSC(COPY_ROW_0).CODE(),
		RSC(COPY_ROW_1).CODE(),
		RSC(COPY_ROW_2).CODE()
	};
	size_t	c = sizeof(C) / sizeof(C[0]),
		f = sizeof(F) / sizeof(F[0]),
		v = strlen(COREFREQ_VERSION);
	CUINT	cHeight = c + f,
		oCol = (draw.Size.width - RSZ(LOGO_ROW_0)) / 2,
		oRow = TOP_HEADER_ROW + 4;

	if (cHeight >= (draw.Size.height - 1)) {
		cHeight = draw.Size.height - 2;
	}
	if (oRow + cHeight >= draw.Size.height) {
		oRow = abs(draw.Size.height - (1 + cHeight));
	}
	Window *wAbout = CreateWindow(	wLayer, id, 1, cHeight,
					oCol, oRow,
					WINFLAG_NO_SCALE );
	if (wAbout != NULL)
	{
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
	return (wAbout);
}

/* >>> GLOBALS >>> */
int SysInfoCellPadding;
/* <<< GLOBALS <<< */

TGrid *AddSysInfoCell(CELL_ARGS)
{
	SysInfoCellPadding++;
	return (StoreTCell(win, key, item, attrib));
}

Window *CreateSysInfo(unsigned long long id)
{
	REASON_CODE (*SysInfoFunc)(Window*, CUINT, CELL_FUNC OutFunc) = NULL;
	ASCII *title = NULL;
	CoordSize matrixSize = {
		.wth = 1,
		.hth = 18
	};
	Coordinate winOrigin = {.col = 3, .row = TOP_HEADER_ROW + 2};
	CUINT winWidth = 74;

	switch (id) {
	case SCANKEY_p:
		{
		winOrigin.row = TOP_HEADER_ROW + 2;
		winOrigin.col = 2;
		matrixSize.hth = 21;
		winWidth = 76;
		SysInfoFunc = SysInfoProc;
		title = RSC(PROCESSOR_TITLE).CODE();
		}
		break;
	case SCANKEY_e:
		{
		winOrigin.row = TOP_HEADER_ROW + 1;
		winOrigin.col = 4;
		matrixSize.hth = 24;
		winWidth = 72;
		SysInfoFunc = SysInfoFeatures;
		title = RSC(FEATURES_TITLE).CODE();
		}
		break;
	case SCANKEY_t:
		{
		winOrigin.col = 23;
		matrixSize.hth = 12;
		winOrigin.row = TOP_HEADER_ROW + 10;
		winWidth = 50;
		SysInfoFunc = SysInfoTech;
		title = RSC(TECHNOLOGIES_TITLE).CODE();
		}
		break;
	case SCANKEY_o:
		{
		matrixSize.hth = 24;
		winOrigin.row = TOP_HEADER_ROW + 1;
		SysInfoFunc = SysInfoPerfMon;
		title = RSC(PERF_MON_TITLE).CODE();
		}
		break;
	case SCANKEY_w:
		{
		winOrigin.col = 25;
		matrixSize.hth = 16;
		matrixSize.hth += (
			Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL
		) ? 5 : 0;
		winOrigin.row = TOP_HEADER_ROW + 2;
		winWidth = 50;
		SysInfoFunc = SysInfoPwrThermal;
		title = RSC(POWER_THERMAL_TITLE).CODE();
		}
		break;
	case SCANKEY_u:
		{
		winOrigin.row = 2;
		matrixSize.hth = 66;
		winWidth = 74;
		SysInfoFunc = SysInfoCPUID;
		title = RSC(CPUID_TITLE).CODE();
		}
		break;
	case SCANKEY_k:
		{
		CUINT height = 14
			+ ( Shm->SysGate.OS.IdleDriver.stateCount > 0 ) * 6;
		winOrigin.col = 1;
		winWidth = 78;
		matrixSize.hth = height;
		winOrigin.row = TOP_HEADER_ROW + 1;
		SysInfoFunc = SysInfoKernel;
		title = RSC(KERNEL_TITLE).CODE();
		}
		break;
	case SCANKEY_SHIFT_b:
		{
		winOrigin.col = 4;
		winWidth = MAX_UTS_LEN + 4 + 1;
		SysInfoFunc = SysInfoSMBIOS;
		title = RSC(SMBIOS_TITLE).CODE();
		}
		break;
	}

	SysInfoCellPadding = 0;

	Window *wSysInfo = CreateWindow(wLayer, id,
					matrixSize.wth, matrixSize.hth,
					winOrigin.col, winOrigin.row);
	if (wSysInfo != NULL)
	{
		if (SysInfoFunc != NULL) {
			SysInfoFunc(wSysInfo, winWidth, AddSysInfoCell);
		}
		/* Pad with blank rows.					*/
		while (SysInfoCellPadding < matrixSize.hth) {
			SysInfoCellPadding++;
			StoreTCell(wSysInfo,
				SCANKEY_NULL,
				&hSpace[MAX_WIDTH - winWidth],
				MAKE_PRINT_FOCUS);
		}

		switch (id) {
		case SCANKEY_p:
		case SCANKEY_t:
		case SCANKEY_o:
		case SCANKEY_w:
		case SCANKEY_k:
			StoreWindow(wSysInfo,	.key.Enter, Enter_StickyCell);
			break;
		case SCANKEY_u:
			wSysInfo->matrix.select.row = 1;
			StoreWindow(wSysInfo,	.color[1].title,
						wSysInfo->hook.color[1].border);
			StoreWindow(wSysInfo,	.key.Enter, MotionEnter_Cell);
			break;
		case SCANKEY_SHIFT_b:
			wSysInfo->matrix.select.row = draw.SmbIndex;
			/* fallthrough */
		default:
			StoreWindow(wSysInfo,	.key.Enter, MotionEnter_Cell);
			break;
		}
		if (title != NULL) {
			StoreWindow(wSysInfo, .title, (char*) title);
		}
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

		StoreWindow(wSysInfo,	.key.Shrink,	MotionShrink_Win);
		StoreWindow(wSysInfo,	.key.Expand,	MotionExpand_Win);
	}
	return (wSysInfo);
}

TGrid *AddCell(CELL_ARGS)
{
	return (StoreTCell(win, key, item, attrib));
}

Window *CreateTopology(unsigned long long id)
{
	Window *wTopology = CreateWindow(wLayer, id,
					TOPO_MATY, 2+Shm->Proc.CPU.Count,
					1, TOP_HEADER_ROW + 1);

	if (wTopology != NULL)
	{
		wTopology->matrix.select.row = 2;

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

		StoreWindow(wTopology,	.key.Shrink,	MotionShrink_Win);
		StoreWindow(wTopology,	.key.Expand,	MotionExpand_Win);
	}
	return (wTopology);
}

Window *CreateISA(unsigned long long id)
{
	Window *wISA = CreateWindow(wLayer, id, 4, 13, 6, TOP_HEADER_ROW+2);

	if (wISA != NULL)
	{
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
	return (wISA);
}

Window *CreateSysRegs(unsigned long long id)
{
	Window *wSR = CreateWindow(	wLayer, id,
					17, (2*(1+Shm->Proc.CPU.Count)),
					6, TOP_HEADER_ROW + 2 );
	if (wSR != NULL)
	{
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

		StoreWindow(wSR,	.key.Shrink,	MotionShrink_Win);
		StoreWindow(wSR,	.key.Expand,	MotionExpand_Win);
	}
	return (wSR);
}

Window *CreateMemCtrl(unsigned long long id)
{
	Window *wIMC;
	TIMING_FUNC pTimingFunc = Timing_DDR3;
	unsigned int mc, rows = 1, ctrlHeaders = 6, channelHeaders = 4;

	switch (Shm->Uncore.Unit.DDR_Ver) {
	case 4:
		ctrlHeaders = 7;
		channelHeaders = 5;
		if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
		{
			pTimingFunc = Timing_DDR4;
		}
		else if ( (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
			||(Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
		{
			pTimingFunc = Timing_DDR4_Zen;
		}
		break;
	case 3:
	case 2:
	default:
		ctrlHeaders = 6;
		channelHeaders = 4;
		pTimingFunc = Timing_DDR3;
		break;
	}
   for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {
	rows += ctrlHeaders * Shm->Uncore.CtrlCount;
	rows += channelHeaders * Shm->Uncore.MC[mc].ChannelCount;
	rows += Shm->Uncore.MC[mc].SlotCount * Shm->Uncore.MC[mc].ChannelCount;
    }
	wIMC = CreateWindow(wLayer, id, MC_MATX, rows, 4, TOP_HEADER_ROW + 2);
    if (wIMC != NULL)
    {
	MemoryController(wIMC, AddCell, pTimingFunc);

	wIMC->matrix.select.col = 2;
	wIMC->matrix.select.row = 1;

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

	StoreWindow(wIMC,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wIMC,	.key.Expand,	MotionExpand_Win);
    }
	return (wIMC);
}

Window *CreateSortByField(unsigned long long id)
{
	Window *wSortBy = CreateWindow( wLayer, id,
					1, SORTBYCOUNT,
					33,
					TOP_HEADER_ROW
				#ifndef NO_UPPER
					+ draw.Area.MaxRows
					+ 2,
				#else
					+ 1,
				#endif
					WINFLAG_NO_STOCK
					| WINFLAG_NO_SCALE
					| WINFLAG_NO_BORDER );
	if (wSortBy != NULL)
	{
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
	return (wSortBy);
}

int SortTaskListByForest(const void *p1, const void *p2)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;

	if (task1->ppid < task2->ppid) {
		return (-1);
	} else if (task1->ppid > task2->ppid) {
		return (1);
	} else if (task1->tgid < task2->tgid) {
		return (-1);
	} else if (task1->tgid > task2->tgid) {
		return (1);
	} else if (task1->pid < task2->pid) {
		return (-1);
	} else if (task1->pid > task2->pid) {
		return (1);
	} else {
		return (1);
	}
}

void UpdateTracker(TGrid *grid, DATA_TYPE data)
{
	const pid_t pid = data.sint[0];
	signed int idx;
  for (idx = 0; idx < Shm->SysGate.taskCount; idx++)
  {
    if (Shm->SysGate.taskList[idx].pid == pid)
    {
	double runtime, usertime, systime;
	/*
	 * Source: kernel/sched/cputime.c
	 * If either stime or utime are 0, assume all runtime is userspace.
	 */
	runtime = Shm->SysGate.taskList[idx].runtime;
	systime = Shm->SysGate.taskList[idx].systime;
	usertime= Shm->SysGate.taskList[idx].usertime;

	if (systime == 0.0)
	{
		usertime = runtime;
	}
	if (usertime == 0.0)
	{
		systime = runtime;
	}
	systime = (systime * runtime) / (systime + usertime);
	usertime= runtime - systime;

	const double fulltime = usertime + systime;
	if (fulltime > 0.0)
	{
		usertime= (100.0 * usertime)/ fulltime;
		systime = (100.0 * systime) / fulltime;
	} else {
		usertime= 0.0;
		systime = 0.0;
	}
	const size_t len = snprintf(	Buffer, 20+20+5+4+6+1,
					"User:%3.0f%%   Sys:%3.0f%% ",
					usertime, systime );

	memcpy( &grid->cell.item[grid->cell.length - len], Buffer, len);
	break;
    }
  }
  if (!(idx < Shm->SysGate.taskCount)) {
	memcpy(grid->cell.item, hSpace, grid->cell.length);
  }
}

Window *CreateTracking(unsigned long long id)
{
	Window *wTrack = NULL;
	const ssize_t tc = Shm->SysGate.taskCount;

  if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) && (tc > 0))
  {
	TASK_MCB *trackList;
	trackList = malloc(tc * sizeof(TASK_MCB));
    if (trackList != NULL)
    {
	memcpy(trackList, Shm->SysGate.taskList, tc * sizeof(TASK_MCB));
	qsort(trackList, tc, sizeof(TASK_MCB), SortTaskListByForest);

	const CUINT margin = 12;	/*	"--- Freq(MHz"		*/
	const CUINT height = TOP_SEPARATOR
		+ (draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER));
	const CUINT width = (draw.Size.width - margin) / 2;
	const int padding = width - TASK_COMM_LEN - (7 + 2);

	wTrack = CreateWindow( wLayer, id,
				2, height,
				margin, TOP_HEADER_ROW,
				WINFLAG_NO_STOCK
				| WINFLAG_NO_BORDER );
	if (wTrack != NULL)
	{
		signed int ti, si = 0, qi = 0;
		pid_t previd = (pid_t) -1;

	    for (ti = 0; ti < tc; ti++)
	    {
		if (trackList[ti].ppid == previd) {
			si += (si < padding - 2) ? 1 : 0;
		} else if (trackList[ti].tgid != previd) {
			si -= (si > 0) ? 1 : 0;
		}
		previd = trackList[ti].tgid;

		if (trackList[ti].pid == trackList[ti].tgid) {
			qi = si + 1;
		} else {
			qi = si + 2;
		}
		snprintf(Buffer, MAX_WIDTH-1,
			"%.*s" "%-16s" "%.*s" "(%7d)",
			qi,
			hSpace,
			trackList[ti].comm,
			padding - qi,
			hSpace,
			trackList[ti].pid);

		StoreTCell(wTrack,
			(TRACK_TASK | trackList[ti].pid),
			Buffer,
			(trackList[ti].pid == trackList[ti].tgid) ?
				  MAKE_PRINT_DROP
				: MakeAttr(BLACK, 0, WHITE, 1));

		snprintf(Buffer, MAX_WIDTH-1, "%.*s", width, hSpace);

		GridCall(StoreTCell(wTrack,SCANKEY_NULL,Buffer,MAKE_PRINT_DROP),
			UpdateTracker, (pid_t) trackList[ti].pid);
	    }
		StoreWindow(wTrack, .color[0].select, MAKE_PRINT_DROP);
		StoreWindow(wTrack, .color[0].title, MAKE_PRINT_DROP);
		StoreWindow(wTrack, .color[1].title, MakeAttr(BLACK,0,WHITE,1));

		StoreWindow(wTrack,	.Print, 	ForEachCellPrint_Drop);
		StoreWindow(wTrack,	.key.Enter,	MotionEnter_Cell);

		StoreWindow(wTrack,	.key.Left,	MotionLeft_Win);
		StoreWindow(wTrack,	.key.Right,	MotionRight_Win);
		StoreWindow(wTrack,	.key.Down,	MotionDown_Win);
		StoreWindow(wTrack,	.key.Up,	MotionUp_Win);

		StoreWindow(wTrack,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wTrack,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wTrack,	.key.Home,	MotionReset_Win);
		StoreWindow(wTrack,	.key.End,	MotionEnd_Cell);

		StoreWindow(wTrack,	.key.Shrink,	MotionShrink_Win);
		StoreWindow(wTrack,	.key.Expand,	MotionExpand_Win);
	}
	free(trackList);
    }
  }
	return (wTrack);
}

void UpdateHotPlugCPU(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;

	if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
	{
	    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
	    {
		snprintf(Buffer, 9+10+1,
			(char*) RSC(CREATE_HOTPLUG_CPU_OFFLINE).CODE(), cpu);

		memcpy(grid->cell.item, Buffer, grid->cell.length);
		memcpy(grid->cell.attr, RSC(CREATE_HOTPLUG_CPU_OFFLINE).ATTR(),
					grid->cell.length);

		grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
	    }
	}
	else
	{
	    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
	    {
		snprintf(Buffer, 9+10+1,
			(char*) RSC(CREATE_HOTPLUG_CPU_ONLINE).CODE(), cpu);

		memcpy(grid->cell.item, Buffer, grid->cell.length);
		memcpy(grid->cell.attr, RSC(CREATE_HOTPLUG_CPU_ONLINE).ATTR(),
					grid->cell.length);

		grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
	    }
	}
}

void UpdateHotPlugTrigger(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;

	if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
	{
	    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
	    {
		memcpy(	grid->cell.item,
			RSC(CREATE_HOTPLUG_CPU_ENABLE).CODE(),
			RSZ(CREATE_HOTPLUG_CPU_ENABLE) );

		memcpy( grid->cell.attr,
			RSC(CREATE_HOTPLUG_CPU_ENABLE).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = (unsigned long long) (CPU_ONLINE | cpu);
		grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
	    }
	}
	else
	{
	    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
	    {
		memcpy( grid->cell.item,
			RSC(CREATE_HOTPLUG_CPU_DISABLE).CODE(),
			RSZ(CREATE_HOTPLUG_CPU_DISABLE) );

		memcpy( grid->cell.attr,
			RSC(CREATE_HOTPLUG_CPU_DISABLE).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = (unsigned long long) (CPU_OFFLINE | cpu);
		grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
	    }
	}
}

Window *CreateHotPlugCPU(unsigned long long id)
{
	Window *wCPU = CreateWindow(	wLayer, id, 2, draw.Area.MaxRows,
					LOAD_LEAD + 1, TOP_HEADER_ROW + 1,
					WINFLAG_NO_STOCK );
  if (wCPU != NULL)
  {
	unsigned int cpu;
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
    {
      if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
      {
	snprintf(Buffer, 9+10+1,
		(char*) RSC(CREATE_HOTPLUG_CPU_OFFLINE).CODE(), cpu);

	GridCall( StoreTCell(	wCPU, SCANKEY_NULL, Buffer,
				RSC(CREATE_HOTPLUG_CPU_OFFLINE).ATTR() ),
		UpdateHotPlugCPU, (unsigned long long) (CPU_OFFLINE | cpu) );

	GridCall( StoreTCell(	wCPU, (unsigned long long) (CPU_ONLINE | cpu),
				RSC(CREATE_HOTPLUG_CPU_ENABLE).CODE(),
				RSC(CREATE_HOTPLUG_CPU_ENABLE).ATTR() ),
		UpdateHotPlugTrigger, (unsigned long long)(CPU_OFFLINE | cpu) );
      }
    else
      {
	snprintf(Buffer, 9+10+1,
		(char*) RSC(CREATE_HOTPLUG_CPU_ONLINE).CODE(), cpu);

	GridCall( StoreTCell(	wCPU, SCANKEY_NULL, Buffer,
				RSC(CREATE_HOTPLUG_CPU_ONLINE).ATTR() ),
		UpdateHotPlugCPU, (unsigned long long) (CPU_ONLINE | cpu) );

	GridCall( StoreTCell(	wCPU, (unsigned long long) (CPU_OFFLINE | cpu),
				RSC(CREATE_HOTPLUG_CPU_DISABLE).CODE(),
				RSC(CREATE_HOTPLUG_CPU_DISABLE).ATTR() ),
		UpdateHotPlugTrigger, (unsigned long long)(CPU_ONLINE | cpu) );
      }
    }
	wCPU->matrix.select.col = 1;

	StoreWindow(wCPU,	.title, 	" CPU ");
	StoreWindow(wCPU, .color[1].title, MakeAttr(WHITE, 0, BLUE, 1));

	StoreWindow(wCPU,	.key.Enter,	Enter_StickyCell);
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

	StoreWindow(wCPU,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wCPU,	.key.Expand,	MotionExpand_Win);
  }
	return (wCPU);
}

void UpdateRatioClock(TGrid *grid, DATA_TYPE data)
{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
				.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core] \
					.Toggle];

	if (data.uint[0] > MAXCLOCK_TO_RATIO(unsigned int, CFlop->Clock.Hz))
	{
		snprintf(Buffer, 1+6+1, " %-6s", RSC(NOT_AVAILABLE).CODE());
	} else {
		snprintf(Buffer, 8+1, "%7.2f",
			ABS_FREQ_MHz(double, data.uint[0], CFlop->Clock));
	}
	memcpy(&grid->cell.item[1], Buffer, 7);
}

void TitleForTurboClock(unsigned int NC, ASCII *title)
{
	snprintf((char *) title, 15+11+1,
		(char *) RSC(TURBO_CLOCK_TITLE).CODE(), NC);
}

void TitleForRatioClock(unsigned int NC, ASCII *title)
{
	snprintf((char *) title,14+6+1,(char *) RSC(RATIO_CLOCK_TITLE).CODE(),
			(NC == CLOCK_MOD_TGT) || (NC == CLOCK_MOD_HWP_TGT) ?
		(char *) RSC(TARGET).CODE() :
			(NC == CLOCK_MOD_MAX) || (NC == CLOCK_MOD_HWP_MAX) ?
		"Max" :
			(NC == CLOCK_MOD_MIN) || (NC == CLOCK_MOD_HWP_MIN) ?
		"Min" : "");
}

void TitleForUncoreClock(unsigned int NC, ASCII *title)
{
	snprintf((char *) title,15+3+1,(char *) RSC(UNCORE_CLOCK_TITLE).CODE(),
			(NC == CLOCK_MOD_MAX) ? "Max" :
			(NC == CLOCK_MOD_MIN) ? "Min" : "");
}

void ComputeRatioShifts(unsigned int COF,
			unsigned int lowestOperating,
			unsigned int highestOperating,
			signed int *lowestShift,
			signed int *highestShift)
{
	unsigned int minRatio, maxRatio;
	if (highestOperating < lowestOperating) {
		minRatio = highestOperating;
		maxRatio = lowestOperating;
	} else {
		minRatio = lowestOperating;
		maxRatio = highestOperating;
	}
	if (COF < minRatio) {
		minRatio = COF;
	}
	if (COF > maxRatio) {
		maxRatio = COF;
	}
	(*lowestShift)	= COF - minRatio;
	(*highestShift) = maxRatio - COF;
}

unsigned int MultiplierIsRatio(unsigned int cpu, unsigned int multiplier)
{
	enum RATIO_BOOST boost;
	for (boost = BOOST(MIN); boost < BOOST(SIZE); boost++)
	{
		switch (boost) {
		case BOOST(CPB):
		case BOOST(XFR):
			if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
			|| (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON)) {
				break;
			}
			/* Fallthrough */
		default:
			if (Shm->Cpu[cpu].Boost[boost] == multiplier) {
				return (1);
			}
			break;
		}
	}
	if (Shm->Proc.Features.Factory.Ratio == multiplier) {
		return (1);
	}
	return (0);
}

Window *CreateRatioClock(unsigned long long id,
			unsigned int COF,
			signed short any,
			unsigned int NC,
			signed int lowestShift,
			signed int highestShift,
			signed int medianColdZone,
			signed int startingHotZone,
			unsigned long long boxKey,
			CPU_ITEM_CALLBACK TitleCallback,
			CUINT oCol)
{
	unsigned int cpu = (any == -1) ? Ruler.Top[NC] : (unsigned int) any;
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	ATTRIBUTE *attrib[7] = {
		RSC(CREATE_RATIO_CLOCK_COND0).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND1).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND2).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND3).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND4).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND5).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND6).ATTR()
	};
	CLOCK_ARG clockMod = {.sllong = id};

	const CUINT hthMax = 1 + lowestShift + highestShift;
	CUINT hthWin = CUMIN( (draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER)),
				hthMax );

	Window *wCK = CreateWindow(	wLayer, id,
					1, hthWin,
					oCol,
					1 + TOP_HEADER_ROW + TOP_FOOTER_ROW,
					WINFLAG_NO_STOCK );
    if (wCK != NULL)
    {
	signed int multiplier, offset;
	for (offset = -lowestShift; offset <= highestShift; offset++)
	{
		ATTRIBUTE *attr = attrib[3];

		multiplier = COF + offset;

		clockMod.NC = NC | boxKey;
		clockMod.cpu = any;

	    if (any == -1) {
		clockMod.Ratio	= multiplier;
	    } else {
		clockMod.Offset = offset;
	    }

	  if (multiplier == 0) {
		snprintf(Buffer, 17+RSZ(AUTOMATIC)+11+11+1,
			"    %s       <%4d >  %+4d ",
			RSC(AUTOMATIC).CODE(), multiplier, offset);

		StoreTCell(wCK, clockMod.sllong, Buffer, attr);
	  } else {
	    if (multiplier > MAXCLOCK_TO_RATIO(signed int, CFlop->Clock.Hz))
	    {
		snprintf(Buffer, 15+6+11+11+1,
			"  %-6s MHz   <%4d >  %+4d ",
			RSC(NOT_AVAILABLE).CODE(), multiplier, offset);
	    } else {
		attr = attrib[
			multiplier < medianColdZone ?
				1 + 4 * MultiplierIsRatio(cpu, multiplier)
			: multiplier >= startingHotZone ?
				2 + 4 * MultiplierIsRatio(cpu, multiplier)
				: 0 + 4 * MultiplierIsRatio(cpu, multiplier) ];

		snprintf(Buffer, 14+8+11+11+1,
			" %7.2f MHz   <%4d >  %+4d ",
			ABS_FREQ_MHz(double, multiplier, CFlop->Clock),
			multiplier, offset);
	    }
		GridCall(StoreTCell(wCK, clockMod.sllong, Buffer, attr),
			UpdateRatioClock, multiplier);
	  }
	}

	TitleCallback(NC, (ASCII *) Buffer);
	StoreWindow(wCK, .title, Buffer);

	if (lowestShift >= hthWin) {
		wCK->matrix.scroll.vert = hthMax
					- hthWin * (1+(highestShift / hthWin));
		wCK->matrix.select.row  = lowestShift
					- wCK->matrix.scroll.vert;
	} else {
		wCK->matrix.select.row  = COF > 0 ? lowestShift : 0;
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

	StoreWindow(wCK,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wCK,	.key.Expand,	MotionExpand_Win);
    }
	return (wCK);
}

void UpdateRoomSchedule(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	const signed int pos = grid->cell.length - 8;

	if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
	{
	    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
	    {
		memcpy( grid->cell.attr,
			RSC(CREATE_SELECT_CPU_COND0).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = SCANKEY_NULL;
		grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
	    }
	}
	else
	{
	    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
	    {
		memcpy( grid->cell.attr,
			RSC(CREATE_SELECT_CPU_COND1).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = (unsigned long long) (CPU_SELECT | cpu);
		grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
	    }
	}

	const unsigned int run = BITVAL_CC(Shm->roomSched, cpu);
	unsigned int sched = BITVAL(data.ullong, 63);

	if (run != sched)
	{
		memcpy( &grid->cell.item[pos],
			ENABLED(run), __builtin_strlen(ENABLED(0)) );

		if (run) {
			BITSET(LOCKLESS, grid->data.ullong, 63);
		} else {
			BITCLR(LOCKLESS, grid->data.ullong, 63);
		}
	    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
	    {
		sched = BITVAL(grid->data.ullong, 63);
		if (sched) {
			memcpy( &grid->cell.attr[pos],
				&RSC(CREATE_SELECT_CPU_COND2).ATTR()[pos],
				__builtin_strlen(ENABLED(0)) );
		} else {
			memcpy( &grid->cell.attr[pos],
				&RSC(CREATE_SELECT_CPU_COND1).ATTR()[pos],
				__builtin_strlen(ENABLED(0)) );
		}
	    }
	}
}

Window *CreateSelectCPU(unsigned long long id)
{
	const CUINT _height = (ADD_UPPER & ADD_LOWER)
			+ (draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER));
	const CUINT height = CUMIN(Shm->Proc.CPU.Count, _height);

	Window *wUSR = CreateWindow(	wLayer, id,
					1, height,
					13 + 27 + 3, TOP_HEADER_ROW + 1,
					WINFLAG_NO_STOCK );
    if (wUSR != NULL)
    {
	unsigned int cpu, bix;
	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
	{
		bix = BITVAL_CC(Shm->roomSched, cpu);
		snprintf(Buffer, 7+10+11+11+11+1,
				"  %03u  %4d%6d%6d    <%3s>    ",
				cpu,
				Shm->Cpu[cpu].Topology.PackageID,
				Shm->Cpu[cpu].Topology.CoreID,
				Shm->Cpu[cpu].Topology.ThreadID,
				ENABLED(bix));

	    if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
	    {
		unsigned long long	data = BITVAL_CC(Shm->roomSched, cpu);
					data = data << 63;
					data = data | (CPU_OFFLINE | cpu);

		GridCall( StoreTCell(	wUSR, SCANKEY_NULL, Buffer,
					RSC(CREATE_SELECT_CPU_COND0).ATTR() ),
			UpdateRoomSchedule, data );
	    }
	  else
	    {
		unsigned long long	data = BITVAL_CC(Shm->roomSched, cpu);
					data = data << 63;
					data = data | (CPU_ONLINE | cpu);

		GridCall( StoreTCell(	wUSR, CPU_SELECT | cpu, Buffer,
				bix	? RSC(CREATE_SELECT_CPU_COND2).ATTR()
					: RSC(CREATE_SELECT_CPU_COND1).ATTR() ),
			UpdateRoomSchedule, data );
	    }
	}
	StoreWindow(wUSR,	.title, (char*) RSC(SELECT_CPU_TITLE).CODE());
	StoreWindow(wUSR,	.color[1].title, wUSR->hook.color[1].border);

	StoreWindow(wUSR,	.key.Enter,	Enter_StickyCell);
	StoreWindow(wUSR,	.key.Down,	MotionDown_Win);
	StoreWindow(wUSR,	.key.Up,	MotionUp_Win);
	StoreWindow(wUSR,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wUSR,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wUSR,	.key.Home,	MotionTop_Win);
	StoreWindow(wUSR,	.key.End,	MotionBottom_Win);
	StoreWindow(wUSR,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wUSR,	.key.Expand,	MotionExpand_Win);

	StoreWindow(wUSR,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wUSR,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wUSR,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wUSR,	.key.WinUp,	MotionOriginUp_Win);

	StoreWindow(wUSR,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wUSR,	.key.Expand,	MotionExpand_Win);
    }
	return (wUSR);
}

void Pkg_Fmt_Turbo(ASCII *item, CLOCK *clock, unsigned int ratio, char *NC)
{
    if (ratio == 0) {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
			(char *) RSC(CREATE_SELECT_AUTO_TURBO).CODE(),
			NC, RSC(AUTOMATIC).CODE(),
			Shm->Proc.Features.Turbo_Unlock ? '<' : '[',
			ratio,
			Shm->Proc.Features.Turbo_Unlock ? '>' : ']');
    } else {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1,
			(char *) RSC(CREATE_SELECT_FREQ_TURBO).CODE(),
			NC, ABS_FREQ_MHz(double, ratio, (*clock)),
			Shm->Proc.Features.Turbo_Unlock ? '<' : '[',
			ratio,
			Shm->Proc.Features.Turbo_Unlock ? '>' : ']');
    }
}

void Pkg_Fmt_Freq(	ASCII *item, ASCII *code, CLOCK *clock,
			unsigned int ratio, unsigned char unlock )
{
    if (ratio == 0) {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
			"%s" "   %s     %c%4u %c ",
			code, RSC(AUTOMATIC).CODE(),
			unlock ? '<' : '[', ratio, unlock ? '>' : ']');
    } else {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1,
			"%s" "%7.2f MHz %c%4u %c ",
			code, ABS_FREQ_MHz(double, ratio, (*clock)),
			unlock ? '<' : '[', ratio, unlock ? '>' : ']');
    }
}

void CPU_Item_Auto_Freq(unsigned int cpu, unsigned int ratio,
			unsigned char unlock, ASCII *item)
{
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+10+1,
			"  %03u  %4d%6d%6d   " "   %s     %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
			RSC(AUTOMATIC).CODE(),
			unlock ? '<' : '[',
			ratio,
			unlock ? '>' : ']');
}

#define DECLARE_Pkg_Item_Turbo(_NC)					\
void Pkg_Item_Turbo_##_NC(ASCII *item)					\
{									\
	unsigned int top = Ruler.Top[BOOST(_NC)];			\
									\
	Pkg_Fmt_Turbo(	item,						\
			&Shm->Cpu[top].FlipFlop[			\
					!Shm->Cpu[top].Toggle		\
					].Clock,			\
			Shm->Cpu[top].Boost[BOOST(_NC)],		\
			COREFREQ_STRINGIFY(_NC) );			\
}
DECLARE_Pkg_Item_Turbo( 1C)
DECLARE_Pkg_Item_Turbo( 2C)
DECLARE_Pkg_Item_Turbo( 3C)
DECLARE_Pkg_Item_Turbo( 4C)
DECLARE_Pkg_Item_Turbo( 5C)
DECLARE_Pkg_Item_Turbo( 6C)
DECLARE_Pkg_Item_Turbo( 7C)
DECLARE_Pkg_Item_Turbo( 8C)
DECLARE_Pkg_Item_Turbo( 9C)
DECLARE_Pkg_Item_Turbo(10C)
DECLARE_Pkg_Item_Turbo(11C)
DECLARE_Pkg_Item_Turbo(12C)
DECLARE_Pkg_Item_Turbo(13C)
DECLARE_Pkg_Item_Turbo(14C)
DECLARE_Pkg_Item_Turbo(15C)
DECLARE_Pkg_Item_Turbo(16C)
DECLARE_Pkg_Item_Turbo(17C)
DECLARE_Pkg_Item_Turbo(18C)
#undef DECLARE_Pkg_Item_Turbo

#define DECLARE_Pkg_Update_Turbo(_NC)					\
void Pkg_Update_Turbo_##_NC(TGrid *grid, DATA_TYPE data)		\
{									\
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];		\
	unsigned int top = Ruler.Top[BOOST(_NC)];			\
	UNUSED(data);							\
									\
	Pkg_Fmt_Turbo(	item,						\
			&Shm->Cpu[top].FlipFlop[			\
					!Shm->Cpu[top].Toggle		\
					].Clock,			\
			Shm->Cpu[top].Boost[BOOST(_NC)],		\
			COREFREQ_STRINGIFY(_NC) );			\
									\
	memcpy(grid->cell.item, item, grid->cell.length);		\
}

DECLARE_Pkg_Update_Turbo( 1C)
DECLARE_Pkg_Update_Turbo( 2C)
DECLARE_Pkg_Update_Turbo( 3C)
DECLARE_Pkg_Update_Turbo( 4C)
DECLARE_Pkg_Update_Turbo( 5C)
DECLARE_Pkg_Update_Turbo( 6C)
DECLARE_Pkg_Update_Turbo( 7C)
DECLARE_Pkg_Update_Turbo( 8C)
DECLARE_Pkg_Update_Turbo( 9C)
DECLARE_Pkg_Update_Turbo(10C)
DECLARE_Pkg_Update_Turbo(11C)
DECLARE_Pkg_Update_Turbo(12C)
DECLARE_Pkg_Update_Turbo(13C)
DECLARE_Pkg_Update_Turbo(14C)
DECLARE_Pkg_Update_Turbo(15C)
DECLARE_Pkg_Update_Turbo(16C)
DECLARE_Pkg_Update_Turbo(17C)
DECLARE_Pkg_Update_Turbo(18C)
#undef DECLARE_Pkg_Update_Turbo

#define DECLARE_CPU_Item_Turbo(_NC)					\
void CPU_Item_Turbo_##_NC(unsigned int cpu, ASCII *item)		\
{									\
    if (Shm->Cpu[cpu].Boost[BOOST(_NC)] == 0) { 			\
	CPU_Item_Auto_Freq(	cpu, Shm->Cpu[cpu].Boost[BOOST(_NC)],	\
				Shm->Proc.Features.Turbo_Unlock, item );\
    } else {								\
	struct FLIP_FLOP *CFlop = &Shm->Cpu[cpu].FlipFlop[		\
					!Shm->Cpu[cpu].Toggle		\
				];					\
	snprintf((char *) item ,					\
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,\
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",	\
			cpu,						\
			Shm->Cpu[cpu].Topology.PackageID,		\
			Shm->Cpu[cpu].Topology.CoreID,			\
			Shm->Cpu[cpu].Topology.ThreadID,		\
			ABS_FREQ_MHz(double,				\
					Shm->Cpu[cpu].Boost[BOOST(_NC)],\
					CFlop->Clock),			\
			Shm->Proc.Features.Turbo_Unlock ? '<' : '[',	\
			Shm->Cpu[cpu].Boost[BOOST(_NC)],		\
			Shm->Proc.Features.Turbo_Unlock ? '>' : ']');	\
    }									\
}
DECLARE_CPU_Item_Turbo( 1C)
DECLARE_CPU_Item_Turbo( 2C)
DECLARE_CPU_Item_Turbo( 3C)
DECLARE_CPU_Item_Turbo( 4C)
DECLARE_CPU_Item_Turbo( 5C)
DECLARE_CPU_Item_Turbo( 6C)
DECLARE_CPU_Item_Turbo( 7C)
DECLARE_CPU_Item_Turbo( 8C)
DECLARE_CPU_Item_Turbo( 9C)
DECLARE_CPU_Item_Turbo(10C)
DECLARE_CPU_Item_Turbo(11C)
DECLARE_CPU_Item_Turbo(12C)
DECLARE_CPU_Item_Turbo(13C)
DECLARE_CPU_Item_Turbo(14C)
DECLARE_CPU_Item_Turbo(15C)
DECLARE_CPU_Item_Turbo(16C)
DECLARE_CPU_Item_Turbo(17C)
DECLARE_CPU_Item_Turbo(18C)
#undef DECLARE_CPU_Item_Turbo

#define DECLARE_CPU_Update_Turbo(_NC)					\
void CPU_Update_Turbo_##_NC(TGrid *grid, DATA_TYPE data)		\
{									\
	const unsigned int cpu = data.ullong & CPU_MASK;		\
	ASCII item[ RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1 ];	\
  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))				\
  {									\
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)			\
    {									\
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,\
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);	\
									\
	memcpy(grid->cell.item, item, grid->cell.length);		\
									\
	memcpy( grid->cell.attr,					\
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),			\
		grid->cell.length );					\
									\
	grid->cell.quick.key = SCANKEY_NULL;				\
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);	\
    }									\
  }									\
  else									\
  {									\
	CPU_Item_Turbo_##_NC(cpu, item);				\
	memcpy(grid->cell.item, item, grid->cell.length);		\
    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)			\
    {									\
	memcpy( grid->cell.attr,					\
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),			\
		grid->cell.length );					\
									\
	grid->cell.quick.key = BOXKEY_TURBO_CLOCK_##_NC | (cpu ^ CORE_COUNT);\
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);	\
    }									\
  }									\
}
DECLARE_CPU_Update_Turbo( 1C)
DECLARE_CPU_Update_Turbo( 2C)
DECLARE_CPU_Update_Turbo( 3C)
DECLARE_CPU_Update_Turbo( 4C)
DECLARE_CPU_Update_Turbo( 5C)
DECLARE_CPU_Update_Turbo( 6C)
DECLARE_CPU_Update_Turbo( 7C)
DECLARE_CPU_Update_Turbo( 8C)
DECLARE_CPU_Update_Turbo( 9C)
DECLARE_CPU_Update_Turbo(10C)
DECLARE_CPU_Update_Turbo(11C)
DECLARE_CPU_Update_Turbo(12C)
DECLARE_CPU_Update_Turbo(13C)
DECLARE_CPU_Update_Turbo(14C)
DECLARE_CPU_Update_Turbo(15C)
DECLARE_CPU_Update_Turbo(16C)
DECLARE_CPU_Update_Turbo(17C)
DECLARE_CPU_Update_Turbo(18C)
#undef DECLARE_CPU_Update_Turbo

void Pkg_Item_Target_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(TGT)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_TGT).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(TGT)],
			Shm->Proc.Features.TgtRatio_Unlock );
}

void Pkg_Target_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(TGT)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_TGT).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(TGT)],
			Shm->Proc.Features.TgtRatio_Unlock );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_Target_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &Shm->Cpu[cpu].FlipFlop[ !Shm->Cpu[cpu].Toggle ];

    if (Shm->Cpu[cpu].Boost[BOOST(TGT)] == 0) {
	CPU_Item_Auto_Freq(	cpu, Shm->Cpu[cpu].Boost[BOOST(TGT)],
				Shm->Proc.Features.TgtRatio_Unlock, item );
    } else {
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d  %-3d" "[%3u ]%5.0f MHz %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
			CFlop->Absolute.Ratio.Perf,
			CFlop->Absolute.Freq,
			Shm->Proc.Features.TgtRatio_Unlock ? '<' : '[',
			Shm->Cpu[cpu].Boost[BOOST(TGT)],
			Shm->Proc.Features.TgtRatio_Unlock ? '>' : ']');
    }
}

void CPU_Target_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
  {
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_Target_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_TGT | (cpu ^ CORE_COUNT);
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_HWP_Target_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(HWP_TGT)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_TGT).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(HWP_TGT)],
			Shm->Proc.Features.HWP_Enable );
}

void Pkg_HWP_Target_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(HWP_TGT)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_TGT).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(HWP_TGT)],
			Shm->Proc.Features.HWP_Enable );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_HWP_Target_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &Shm->Cpu[cpu].FlipFlop[ !Shm->Cpu[cpu].Toggle ];

    if (Shm->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf == 0) {
	CPU_Item_Auto_Freq(	cpu,
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf,
				Shm->Proc.Features.HWP_Enable, item );
    } else {
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(double,
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf,
			CFlop->Clock
			),
			Shm->Proc.Features.HWP_Enable ? '<' : '[',
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf,
			Shm->Proc.Features.HWP_Enable ? '>' : ']');
    }
}

void CPU_HWP_Target_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
  {
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_HWP_Target_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_HWP_TGT | (cpu ^ CORE_COUNT);
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_HWP_Max_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(HWP_MAX)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MAX).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(HWP_MAX)],
			Shm->Proc.Features.HWP_Enable );
}

void Pkg_HWP_Max_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(HWP_MAX)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MAX).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(HWP_MAX)],
			Shm->Proc.Features.HWP_Enable );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_HWP_Max_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &Shm->Cpu[cpu].FlipFlop[ !Shm->Cpu[cpu].Toggle ];

    if (Shm->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf == 0) {
	CPU_Item_Auto_Freq(	cpu,
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf,
				Shm->Proc.Features.HWP_Enable, item );
    } else {
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(double,
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf,
			CFlop->Clock
			),
			Shm->Proc.Features.HWP_Enable ? '<' : '[',
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf,
			Shm->Proc.Features.HWP_Enable ? '>' : ']');
    }
}

void CPU_HWP_Max_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
  {
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_HWP_Max_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_HWP_MAX | (cpu ^ CORE_COUNT);
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_HWP_Min_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(HWP_MIN)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MIN).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(HWP_MIN)],
			Shm->Proc.Features.HWP_Enable );
}

void Pkg_HWP_Min_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(HWP_MIN)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MIN).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(HWP_MIN)],
			Shm->Proc.Features.HWP_Enable );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_HWP_Min_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &Shm->Cpu[cpu].FlipFlop[ !Shm->Cpu[cpu].Toggle ];

    if (Shm->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf == 0) {
	CPU_Item_Auto_Freq(	cpu,
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf,
				Shm->Proc.Features.HWP_Enable, item );
    } else {
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(double,
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf,
			CFlop->Clock
			),
			Shm->Proc.Features.HWP_Enable ? '<' : '[',
			Shm->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf,
			Shm->Proc.Features.HWP_Enable ? '>' : ']');
    }
}

void CPU_HWP_Min_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
  {
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_HWP_Min_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_HWP_MIN | (cpu ^ CORE_COUNT);
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_Max_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(MAX)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MAX).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(MAX)],
			(Shm->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10 );
}

void Pkg_Max_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(MAX)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MAX).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(MAX)],
			(Shm->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10 );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_Max_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &Shm->Cpu[cpu].FlipFlop[ !Shm->Cpu[cpu].Toggle ];

    if (Shm->Cpu[cpu].Boost[BOOST(MAX)] == 0) {
	CPU_Item_Auto_Freq(	cpu, Shm->Cpu[cpu].Boost[BOOST(MAX)],
			(Shm->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10,
				item );
    } else {
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
			ABS_FREQ_MHz(double,
					Shm->Cpu[cpu].Boost[BOOST(MAX)],
					CFlop->Clock),
		(Shm->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10 ? '<':'[',
			Shm->Cpu[cpu].Boost[BOOST(MAX)],
		(Shm->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10 ? '>':']');
    }
}

void CPU_Max_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
  {
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_Max_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_MAX | (cpu ^ CORE_COUNT);
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_Min_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(MIN)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MIN).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(MIN)],
			(Shm->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01 );
}

void Pkg_Min_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(MIN)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MIN).CODE(),
			&Shm->Cpu[top].FlipFlop[ !Shm->Cpu[top].Toggle ].Clock,
			Shm->Cpu[top].Boost[BOOST(MIN)],
			(Shm->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01 );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_Min_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &Shm->Cpu[cpu].FlipFlop[ !Shm->Cpu[cpu].Toggle ];

    if (Shm->Cpu[cpu].Boost[BOOST(MIN)] == 0) {
	CPU_Item_Auto_Freq(	cpu, Shm->Cpu[cpu].Boost[BOOST(MIN)],
			(Shm->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01,
				item );
    } else {
	snprintf((char *) item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			Shm->Cpu[cpu].Topology.PackageID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
			ABS_FREQ_MHz(double,
					Shm->Cpu[cpu].Boost[BOOST(MIN)],
					CFlop->Clock),
		(Shm->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01 ? '<':'[',
			Shm->Cpu[cpu].Boost[BOOST(MIN)],
		(Shm->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01 ? '>':']');
    }
}

void CPU_Min_Freq_Update(TGrid *grid, DATA_TYPE data)
{
	const unsigned int cpu = data.ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
  {
    if ((data.ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data.ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_Min_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data.ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_MIN | (cpu ^ CORE_COUNT);
	grid->data.ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

Window *CreateSelectFreq(unsigned long long id,
			PKG_ITEM_CALLBACK Pkg_Item_Callback,
			CPU_ITEM_CALLBACK CPU_Item_Callback,
			UPDATE_CALLBACK Pkg_Freq_Update,
			UPDATE_CALLBACK CPU_Freq_Update)
{
	const CUINT _height = (draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER))
			#ifndef NO_FOOTER
				+ (ADD_UPPER | ADD_LOWER)
			#endif
		, height = CUMIN( (Shm->Proc.CPU.Count + 1), _height );

	Window *wFreq = CreateWindow(	wLayer, id,
					1, height,
					30, (TOP_HEADER_ROW | 1) );
  if (wFreq != NULL)
  {
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];
	const unsigned long long all = id | (0xffff ^ CORE_COUNT);
	unsigned int cpu;

	Pkg_Item_Callback(item);

	GridCall(StoreTCell(wFreq, all, (char*) item,
					RSC(CREATE_SELECT_FREQ_PKG).ATTR()),
		Pkg_Freq_Update);

    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
    {
      if (BITVAL(Shm->Cpu[cpu].OffLine, OS))
      {
	snprintf((char *) item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
		(char *) RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	GridCall( StoreTCell(wFreq, SCANKEY_NULL,
			item, RSC(CREATE_SELECT_FREQ_COND1).ATTR()),
		CPU_Freq_Update, (unsigned long long) (CPU_OFFLINE | cpu) );
      }
    else
      {
	CPU_Item_Callback(cpu, item);

	GridCall( StoreTCell(wFreq, id | (cpu ^ CORE_COUNT),
			item, RSC(CREATE_SELECT_FREQ_COND0).ATTR()),
		CPU_Freq_Update, (unsigned long long) (CPU_ONLINE | cpu) );
      }
    }
	StoreWindow(wFreq,	.title, (char*) RSC(SELECT_FREQ_TITLE).CODE());
	StoreWindow(wFreq,	.color[1].title, wFreq->hook.color[1].border);

	StoreWindow(wFreq,	.key.Enter,	Enter_StickyCell);
	StoreWindow(wFreq,	.key.Down,	MotionDown_Win);
	StoreWindow(wFreq,	.key.Up,	MotionUp_Win);
	StoreWindow(wFreq,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wFreq,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wFreq,	.key.Home,	MotionTop_Win);
	StoreWindow(wFreq,	.key.End,	MotionBottom_Win);

	StoreWindow(wFreq,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wFreq,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wFreq,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wFreq,	.key.WinUp,	MotionOriginUp_Win);
  }
	return (wFreq);
}

Window *CreateSelectIdle(unsigned long long id)
{
	Window *wIdle = CreateWindow(wLayer, id,
			1, 1 + Shm->SysGate.OS.IdleDriver.stateCount,
			(draw.Size.width - (2 + RSZ(BOX_IDLE_LIMIT_TITLE))) / 2,
			TOP_HEADER_ROW + 2,
			WINFLAG_NO_STOCK);

    if (wIdle != NULL)
    {
	int idx;

	StoreTCell(wIdle, BOXKEY_LIMIT_IDLE_ST00,
			(ASCII*) "            0     RESET ",
			MakeAttr(WHITE, 0, BLACK, 0));

	for (idx = 0; idx < Shm->SysGate.OS.IdleDriver.stateCount; idx++)
	{
		snprintf(Buffer, 12+11+10+1,
				"           %2d%10.*s ", 1 + idx,
				10, Shm->SysGate.OS.IdleDriver.State[idx].Name);

		StoreTCell(wIdle, (BOXKEY_LIMIT_IDLE_ST00 | ((1 + idx) << 4)),
				Buffer, MakeAttr(WHITE, 0, BLACK, 0));
	}
	StoreWindow(wIdle, .title, (char*) RSC(BOX_IDLE_LIMIT_TITLE).CODE());

	wIdle->matrix.select.row  = Shm->SysGate.OS.IdleDriver.stateLimit;
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[ 8] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[ 9] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[10] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[11] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[12] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[13] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[14] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[15] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[16] =		\
						MakeAttr(CYAN , 0, BLACK, 1);
	TCellAt(wIdle, 0, wIdle->matrix.select.row).item[ 8] = '<';
	if (wIdle->matrix.select.row > 9) {
		TCellAt(wIdle, 0, wIdle->matrix.select.row).item[15] = '>';
	} else {
		TCellAt(wIdle, 0, wIdle->matrix.select.row).item[16] = '>';
	}
	StoreWindow(wIdle,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wIdle,	.key.Down,	MotionDown_Win);
	StoreWindow(wIdle,	.key.Up,	MotionUp_Win);
	StoreWindow(wIdle,	.key.Home,	MotionTop_Win);
	StoreWindow(wIdle,	.key.End,	MotionBottom_Win);

	StoreWindow(wIdle,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wIdle,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wIdle,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wIdle,	.key.WinUp,	MotionOriginUp_Win);

	StoreWindow(wIdle,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wIdle,	.key.Expand,	MotionExpand_Win);
    }
	return (wIdle);
}

Window *CreateRecorder(unsigned long long id)
{
	Window *wRec = CreateWindow(	wLayer, id,
					1, 8, 43, TOP_HEADER_ROW+4,
					WINFLAG_NO_STOCK );
    if (wRec != NULL)
    {
	unsigned int idx = 1;
	do {
		unsigned long long key = OPS_RECORDER | (idx << 4);
		int	duration = Recorder.Ratios[idx] * RECORDER_DEFAULT,
			remainder = duration % (60 * 60),
			hours	= duration / (60 * 60),
			minutes = remainder / 60,
			seconds = remainder % 60;

		snprintf(Buffer,6+11+11+11+1,
				"\x20\x20%02d:%02d:%02d\x20\x20",
				hours, minutes, seconds);

		StoreTCell(	wRec,
				key,
				Buffer,
				RSC(CREATE_RECORDER).ATTR() );

	} while (Recorder.Ratios[++idx] != 0);

	if (Recorder.Select > 0) {
		wRec->matrix.select.row  = Recorder.Select - 1;
	}
	StoreWindow(wRec, .title, (char*) RSC(BOX_RECORDER_TITLE).CODE());

	StoreWindow(wRec,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wRec,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wRec,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wRec,	.key.WinUp,	MotionOriginUp_Win);
	StoreWindow(wRec,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wRec,	.key.Down,	MotionDown_Win);
	StoreWindow(wRec,	.key.Up,	MotionUp_Win);
	StoreWindow(wRec,	.key.Home,	MotionReset_Win);
	StoreWindow(wRec,	.key.End,	MotionEnd_Cell);
	StoreWindow(wRec,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wRec,	.key.Expand,	MotionExpand_Win);
    }
	return (wRec);
}

#define POPUP_WIDTH (MIN_WIDTH - 2)
#define POPUP_ALLOC (POPUP_WIDTH + 1)

Window *PopUpMessage(ASCII *title, RING_CTRL *pCtrl)
{
	char *sysMsg, *item = NULL, *inStr = NULL, *outStr = NULL;
	Window *wMsg = NULL;

  if (((item = malloc(POPUP_ALLOC)) != NULL)
  && ((inStr = malloc(POPUP_ALLOC)) != NULL)
  && ((outStr = malloc(POPUP_ALLOC)) != NULL))
  {
	item[POPUP_WIDTH] = '\0';

	wMsg = CreateWindow(	wLayer, pCtrl->arg,
				1, 5,
				1, draw.Size.height - (1 + 5),
				WINFLAG_NO_STOCK );
    if (wMsg != NULL)
    {
	struct tm *brokTime, localTime;
	time_t execTime;
	size_t hdrLen, sysLen;

	execTime = Shm->StartedAt + pCtrl->tds;
	brokTime = localtime_r(&execTime, &localTime);

	switch ( GET_LOCALE() ) {
	case LOC_FR:	/* Convert the local time in ASCII		*/
		hdrLen = strftime(inStr, POPUP_ALLOC, "%c", brokTime);
		ISO_8859_To_Unicode((ASCII *) inStr, (ASCII *) outStr);
		break;
	case LOC_EN:	/* Keep the default language. No conversion.	*/
	default:
		hdrLen = strftime(outStr, POPUP_ALLOC, "%c", brokTime);
		break;
	}

	if (pCtrl->drc < RC_DRIVER_BASE)
	{
		sysMsg = strerror_r(pCtrl->drc, inStr, POPUP_ALLOC);
		switch ( GET_LOCALE() ) {
		case LOC_FR:	/* Convert the System message to locale */
			ISO_8859_To_Unicode((ASCII *) sysMsg, (ASCII *) inStr);
			sysMsg = inStr;
			break;
		case LOC_EN:
		default:	/*	No conversion required.		*/
			break;
		}
		sysLen = sysMsg != NULL ? strlen(sysMsg) : 0;
	} else {
	    struct {
		enum REASON_CLASS rc;
		ASCII		*code;
		size_t		len;
	    } drvReason[] = {
		{
		RC_UNIMPLEMENTED,
		RSC(ERROR_UNIMPLEMENTED).CODE() , RSZ(ERROR_UNIMPLEMENTED)
		},
		{
		RC_EXPERIMENTAL,
		RSC(ERROR_EXPERIMENTAL).CODE()	, RSZ(ERROR_EXPERIMENTAL)
		},
		{
		RC_TURBO_PREREQ,
		RSC(ERROR_TURBO_PREREQ).CODE()	, RSZ(ERROR_TURBO_PREREQ)
		},
		{
		RC_UNCORE_PREREQ,
		RSC(ERROR_UNCORE_PREREQ).CODE()	, RSZ(ERROR_UNCORE_PREREQ)
		},
		{
		RC_PSTATE_NOT_FOUND,
		RSC(ERROR_PSTATE_NOT_FOUND).CODE(), RSZ(ERROR_PSTATE_NOT_FOUND)
		}
	    };
		const size_t count = sizeof(drvReason) / sizeof(drvReason[0]);
		size_t idx;
		for (idx = 0; idx < count; idx++) {
			if (drvReason[idx].rc == pCtrl->drc) {
				break;
			}
		}
		if (idx < count) {
			sysMsg = (char *) drvReason[idx].code;
			sysLen = drvReason[idx].len;
		} else {
			sysMsg = NULL;
			sysLen = 0;
		}
	}
	memset(item, 0x20, POPUP_WIDTH);
	if ((hdrLen > 0) && (hdrLen < POPUP_WIDTH)) {
		memcpy(&item[POPUP_WIDTH - hdrLen], outStr, hdrLen);
	}
	hdrLen = sprintf(outStr, "[%x:%lx]", pCtrl->cmd, pCtrl->arg);
	if (hdrLen < POPUP_WIDTH) {
		memcpy(item, outStr, hdrLen);
	}
	StoreTCell(wMsg, SCANKEY_NULL, item,	MakeAttr(WHITE, 0, BLACK, 1));

	memset(item, 0x20, POPUP_WIDTH);
	StoreTCell(wMsg, SCANKEY_NULL, item,	MakeAttr(WHITE, 0, BLACK, 1));

	if ((sysLen > 0) && (sysLen < POPUP_WIDTH)) {
		memcpy(&item[(POPUP_WIDTH / 2) - (sysLen / 2)], sysMsg,sysLen);
	} else {
		memcpy(item, sysMsg, POPUP_WIDTH);
	}
	StoreTCell(wMsg, SCANKEY_NULL, item,	MakeAttr(WHITE, 0, BLACK, 1));

	memset(item, 0x20, POPUP_WIDTH);
	StoreTCell(wMsg, SCANKEY_NULL, item,	MakeAttr(WHITE, 0, BLACK, 1));
	StoreTCell(wMsg, SCANKEY_NULL, item,	MakeAttr(WHITE, 0, BLACK, 1));

	StoreWindow(wMsg, .color[0].select,	MakeAttr(WHITE, 0, BLACK, 0));
	StoreWindow(wMsg, .color[1].select,	MakeAttr(WHITE, 0, BLACK, 1));
	StoreWindow(wMsg, .color[0].border,	MakeAttr(WHITE, 0, RED	, 0));
	StoreWindow(wMsg, .color[1].border,	MakeAttr(WHITE, 0, RED	, 1));
	StoreWindow(wMsg, .color[0].title,	MakeAttr(WHITE, 0, RED	, 0));
	StoreWindow(wMsg, .color[1].title,	MakeAttr(BLACK, 0, WHITE, 0));
	StoreWindow(wMsg, .title, (char *) title);
    }
  }
  if (outStr != NULL) {
	free(outStr);
  }
  if (inStr != NULL) {
	free(inStr);
  }
  if (item != NULL) {
	free(item);
  }
	return (wMsg);
}
#undef POPUP_ALLOC
#undef POPUP_WIDTH

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
    if (pBox != NULL)
    {
	wBox = CreateWindow(	wLayer, id,
				1, pBox->cnt, origin.col, origin.row,
				WINFLAG_NO_STOCK );
	if (wBox != NULL) {
		wBox->matrix.select.col = select.col;
		wBox->matrix.select.row = select.row;

		for (cnt = 0; cnt < pBox->cnt; cnt++)
			StoreTCell(	wBox,
					pBox->btn[cnt].key,
					pBox->btn[cnt].item,
					pBox->btn[cnt].attr);
		if (title != NULL) {
			StoreWindow(wBox, .title, title);
		}
		StoreWindow(wBox,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wBox,	.key.Down,	MotionDown_Win);
		StoreWindow(wBox,	.key.Up,	MotionUp_Win);
		StoreWindow(wBox,	.key.Home,	MotionReset_Win);
		StoreWindow(wBox,	.key.End,	MotionEnd_Cell);
		StoreWindow(wBox,	.key.Shrink,	MotionShrink_Win);
		StoreWindow(wBox,	.key.Expand,	MotionExpand_Win);
	}
	free(pBox);
    }
	return (wBox);
}

#define CreateBox(id, origin, select, title, button, ...)		\
	_CreateBox(id, origin, select, title, button, __VA_ARGS__,NULL)

typedef struct {
	ASCII *item;
	CUINT length;
	ATTRIBUTE attrib;
	unsigned long long quick;
} IssueList;

IssueList *FindIssues(CUINT *wth, CUINT *hth)
{
	struct {
		IssueList	issue;
		unsigned short	state;
	} problem[] = {
	  {
	    {
		RSC(RECORDER).CODE(),
		RSZ(RECORDER),
		MakeAttr(CYAN, 0, BLACK, 1),
		SCANKEY_ALT_p
	    },
		DumpStatus()
	  },
	  {
	    {
		RSC(STRESS).CODE(),
		RSZ(STRESS),
		MakeAttr(CYAN, 0, BLACK, 1),
		BOXKEY_TOOLS_MACHINE
	    },
		BITVAL(Shm->Proc.Sync, BURN)
	  },
	  {
	    {
		RSC(KERNEL_IDLE_DRIVER).CODE(),
		RSZ(KERNEL_IDLE_DRIVER),
		MakeAttr(CYAN, 0, BLACK, 1),
		OPS_CPU_IDLE
	    },
		(Shm->Registration.Driver.CPUidle == REGISTRATION_ENABLE)
	  }
	};
	IssueList *list = NULL;
	CUINT idx;
	for (idx = 0, (*hth) = 0;
		idx < (CUINT) (sizeof(problem) / sizeof(problem[0])); idx++)
	{
	  if (problem[idx].state)
	  {
	    if ((list = realloc(list, (1+(*hth)) * sizeof(IssueList))) != NULL)
	    {
		list[(*hth)].item	= problem[idx].issue.item;
		list[(*hth)].length	= problem[idx].issue.length;
		list[(*hth)].attrib	= problem[idx].issue.attrib;
		list[(*hth)].quick	= problem[idx].issue.quick;

		(*wth) = CUMAX((*wth), list[(*hth)].length);
		(*hth) = (*hth) + 1;
	    }
	  }
	}
	return (list);
}

Window *CreateExit(unsigned long long id, IssueList *issue, CUINT wth,CUINT hth)
{
	Window *wExit;

	wExit = CreateWindow(	wLayer, id,
				1, (hth + 6),
				(draw.Size.width - wth) / 2,
				(hth + 6) > draw.Size.height ?
					1 : (draw.Size.height - (hth + 2)) / 2,
				WINFLAG_NO_STOCK );
    if (wExit != NULL)
    {
	CUINT idx;

	StoreTCell(wExit, SCANKEY_NULL, RSC(EXIT_HEADER).CODE(),
			MakeAttr(WHITE, 0, BLACK, 0));

	memset(Buffer, 0x20, wth);	Buffer[wth] = '\0';
	StoreTCell(wExit, SCANKEY_NULL, Buffer, MakeAttr(BLACK, 0, BLACK, 1));

	for (idx = 0; idx < hth; idx++)
	{
		const CUINT pos = (wth - issue[idx].length) / 2;
		memset(Buffer, 0x20, wth);
		memcpy(&Buffer[pos], issue[idx].item, issue[idx].length);
		Buffer[wth] = '\0';

		StoreTCell(wExit, issue[idx].quick, Buffer, issue[idx].attrib);
	};
	memset(Buffer, 0x20, wth);	Buffer[wth] = '\0';
	StoreTCell(wExit, SCANKEY_NULL, Buffer, MakeAttr(BLACK, 0, BLACK, 1));

	StoreTCell(wExit, SCANKEY_CTRL_ALT_x, RSC(EXIT_CONFIRM).CODE(),
			MakeAttr(WHITE, 0, BLACK, 1));

	memset(Buffer, 0x20, wth);	Buffer[wth] = '\0';
	StoreTCell(wExit, SCANKEY_NULL, Buffer, MakeAttr(BLACK, 0, BLACK, 1));

	StoreTCell(wExit, SCANKEY_NULL, RSC(EXIT_FOOTER).CODE(),
			MakeAttr(WHITE, 0, BLACK, 0));

	wExit->matrix.select.row = 2;

	StoreWindow(wExit,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wExit,	.key.Down,	MotionDown_Win);
	StoreWindow(wExit,	.key.Up,	MotionUp_Win);
	StoreWindow(wExit,	.key.Home,	MotionReset_Win);
	StoreWindow(wExit,	.key.End,	MotionEnd_Cell);
	StoreWindow(wExit, .title, (char*) RSC(EXIT_TITLE).CODE());
    }
	return (wExit);
}

void TrapScreenSize(int caught)
{
  if (caught == SIGWINCH)
  {
	SCREEN_SIZE currentSize = GetScreenSize();
	CUINT areaMaxRows = 1;

    if (currentSize.height != draw.Size.height)
    {
	if (currentSize.height > MAX_HEIGHT) {
		draw.Size.height = MAX_HEIGHT;
	} else {
		draw.Size.height = currentSize.height;
	}
	switch (draw.Disposal) {
	case D_MAINVIEW:
	draw.Area.MinHeight = (ADD_LOWER * (draw.View == V_PACKAGE ? 10 : 0))
				+ TOP_HEADER_ROW
				+ TOP_SEPARATOR
				+ TOP_FOOTER_ROW;
	break;
	default:
	draw.Area.MinHeight = LEADING_TOP + 2 * (MARGIN_HEIGHT + INTER_HEIGHT);
	break;
	}
	draw.Flag.clear  = 1;
	draw.Flag.height = !(draw.Size.height < draw.Area.MinHeight);
    }
    if (currentSize.width != draw.Size.width) {
	if (currentSize.width > MAX_WIDTH) {
		draw.Size.width = MAX_WIDTH;
	} else {
		draw.Size.width = currentSize.width;
	}
	draw.Flag.clear = 1;
	draw.Flag.width = !(draw.Size.width < MIN_WIDTH);
    }
    if ((ADD_UPPER | ADD_LOWER) && (draw.Size.height > draw.Area.MinHeight)) {
	areaMaxRows = draw.Size.height - draw.Area.MinHeight;
      if (draw.View != V_PACKAGE) {
	areaMaxRows = areaMaxRows >> (ADD_UPPER & ADD_LOWER);
      }
      if (!areaMaxRows) {
	areaMaxRows = 1;
      }
    }
	draw.Area.MaxRows = CUMIN(Shm->Proc.CPU.Count, areaMaxRows);
	draw.Area.LoadWidth = draw.Size.width - LOAD_LEAD;
	draw.cpuScroll = 0;

    if (draw.Flag.clear == 1 ) {
	ReScaleAllWindows(&winList);
    }
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
	};

    switch (scan->key) {
    case SCANKEY_DOWN:
	if (!IsDead(&winList)) {
		return (-1);
	}
	/* Fallthrough */
    case SCANKEY_PLUS:
	if ((draw.Disposal == D_MAINVIEW)
	&&  (draw.cpuScroll < (Shm->Proc.CPU.Count - draw.Area.MaxRows)))
	{
		draw.cpuScroll++;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_UP:
	if (!IsDead(&winList)) {
		return (-1);
	}
	/* Fallthrough */
    case SCANKEY_MINUS:
	if ((draw.Disposal == D_MAINVIEW) && (draw.cpuScroll > 0)) {
		draw.cpuScroll--;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_HOME:
    case SCANCON_HOME:
	if (!IsDead(&winList)) {
		return (-1);
	} else if (draw.Disposal == D_MAINVIEW) {
		draw.cpuScroll = 0;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_END:
    case SCANCON_END:
	if (!IsDead(&winList)) {
		return (-1);
	} else if (draw.Disposal == D_MAINVIEW) {
		draw.cpuScroll = Shm->Proc.CPU.Count - draw.Area.MaxRows;
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_PGDW:
	if (!IsDead(&winList)) {
		return (-1);
	} else if (draw.Disposal == D_MAINVIEW) {
	    if ( (draw.cpuScroll + draw.Area.MaxRows) < ( Shm->Proc.CPU.Count
							- draw.Area.MaxRows ) )
	    {
		draw.cpuScroll += draw.Area.MaxRows;
	    } else {
		draw.cpuScroll = Shm->Proc.CPU.Count - draw.Area.MaxRows;
	    }
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_PGUP:
	if (!IsDead(&winList)) {
		return (-1);
	} else if (draw.Disposal == D_MAINVIEW) {
	    if (draw.cpuScroll >= draw.Area.MaxRows) {
		draw.cpuScroll -= draw.Area.MaxRows;
	    } else {
		draw.cpuScroll = 0;
	    }
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_AST:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_SYSUPDT);
	}
    break;
    case SCANKEY_OPEN_BRACE:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CONTROLLER );
	}
    break;
    case SCANKEY_CLOSE_BRACE:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CONTROLLER );
	}
    break;
    case SCANKEY_F2:
    case SCANCON_F2:
    {
	Window *win = SearchWinListById(SCANKEY_F2, &winList);
	if (win == NULL) {
		AppendWindow(CreateMenu(SCANKEY_F2, 0), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_F3:
    case SCANCON_F3:
    {
	Window *win = SearchWinListById(SCANKEY_F2, &winList);
	if (win == NULL) {
		AppendWindow(CreateMenu(SCANKEY_F2, 1), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_F4:
    case SCANCON_F4:
    {
	Window *win = SearchWinListById(SCANKEY_F2, &winList);
	if (win == NULL) {
		AppendWindow(CreateMenu(SCANKEY_F2, 2), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_CTRL_u:
    #if defined(UBENCH) && UBENCH == 1
	draw.Flag.uBench = !draw.Flag.uBench;
    #endif
	break;
    case SCANKEY_CTRL_ALT_x:
    case SCANCON_CTRL_ALT_x:
	BITSET(LOCKLESS, Shutdown, SYNC);
    break;
    case SCANKEY_CTRL_x:
    {
	CUINT wth = RSZ(EXIT_HEADER), hth;
	IssueList *issueList = FindIssues(&wth, &hth);
	if (issueList != NULL) {
		Window *win = SearchWinListById(SCANKEY_CTRL_x, &winList);
	    if (win == NULL) {
		AppendWindow(	CreateExit(SCANKEY_CTRL_x, issueList, wth, hth),
				&winList );
	    } else {
		SetHead(&winList, win);
	    }
		free(issueList);
	} else {
		BITSET(LOCKLESS, Shutdown, SYNC);
	}
    }
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
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				100,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_150:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				150,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_250:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				250,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_500:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				500,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_750:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				750,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_1000:
	if (!RING_FULL( Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				1000,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_1500:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				1500,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_2000:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				2000,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_2500:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				2500,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_INTERVAL_3000:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				3000,
				MACHINE_INTERVAL );
	}
    break;
    case OPS_AUTOCLOCK:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const int bON = ((Shm->Registration.AutoClock & 0b10) != 0);
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 4
		}, select = {
			.col = 0,
			.row = bON ? 2 : 1
		};

	AppendWindow(CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_AUTO_CLOCK_TITLE).CODE(),
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][bON], stateAttr[bON] , OPS_AUTOCLOCK_ON,
		stateStr[0][!bON],stateAttr[!bON], OPS_AUTOCLOCK_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else
		SetHead(&winList, win);
    }
    break;
    case OPS_AUTOCLOCK_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_AUTOCLOCK );
	}
    break;
    case OPS_AUTOCLOCK_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_AUTOCLOCK );
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
				RSC(BOX_NOMINAL_MODE_COND1).CODE()
			},{
				RSC(BOX_EXPERIMENT_MODE_COND0).CODE(),
				RSC(BOX_EXPERIMENT_MODE_COND1).CODE()
			}
		};
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = 3
		};

	AppendWindow(CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_MODE_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
			RSC(BOX_MODE_DESC).CODE(), descAttr, SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
			ops_Str[0][Shm->Registration.Experimental == 0],
				stateAttr[Shm->Registration.Experimental == 0],
					OPS_EXPERIMENTAL_OFF,
			ops_Str[1][Shm->Registration.Experimental != 0] ,
				exp_Attr[Shm->Registration.Experimental != 0],
					OPS_EXPERIMENTAL_ON,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case OPS_EXPERIMENTAL_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_EXPERIMENTAL );
	}
    break;
    case OPS_EXPERIMENTAL_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_EXPERIMENTAL );
	}
    break;
    case OPS_INTERRUPTS:
    case OPS_CPU_IDLE:
    case OPS_CPU_FREQ:
    case OPS_GOVERNOR:
    case OPS_CLOCK_SOURCE:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		ASCII *ops_Str[2][2] = {
			{
				RSC(BOX_OPS_REGISTER_COND0).CODE(),
				RSC(BOX_OPS_REGISTER_COND1).CODE()
			},{
				RSC(BOX_OPS_UNREGISTER_COND0).CODE(),
				RSC(BOX_OPS_UNREGISTER_COND1).CODE()
			}
		}, *ops_title = NULL;
		unsigned long long	ops_key_on = SCANKEY_NULL,
					ops_key_off = SCANKEY_NULL;
		unsigned int bix = 0;

	    switch (scan->key) {
	    case OPS_INTERRUPTS:
		ops_title = RSC(BOX_INTERRUPT_TITLE).CODE();
		ops_key_on  = OPS_INTERRUPTS_ON;
		ops_key_off = OPS_INTERRUPTS_OFF;
		bix=BITWISEAND(LOCKLESS,Shm->Registration.NMI,BIT_NMI_MASK)!=0;
		break;
	    case OPS_CPU_IDLE:
		ops_title = RSC(BOX_CPU_IDLE_TITLE).CODE();
		ops_key_on  = OPS_CPU_IDLE_ON;
		ops_key_off = OPS_CPU_IDLE_OFF;
		bix = Shm->Registration.Driver.CPUidle & REGISTRATION_ENABLE;
		break;
	    case OPS_CPU_FREQ:
		ops_title = RSC(BOX_CPU_FREQ_TITLE).CODE();
		ops_key_on  = OPS_CPU_FREQ_ON;
		ops_key_off = OPS_CPU_FREQ_OFF;
		bix = Shm->Registration.Driver.CPUfreq & REGISTRATION_ENABLE;
		break;
	    case OPS_GOVERNOR:
		ops_title = RSC(BOX_GOVERNOR_TITLE).CODE();
		ops_key_on  = OPS_GOVERNOR_ON;
		ops_key_off = OPS_GOVERNOR_OFF;
		bix = Shm->Registration.Driver.Governor & REGISTRATION_ENABLE;
		break;
	    case OPS_CLOCK_SOURCE:
		ops_title = RSC(BOX_CLOCK_SOURCE_TITLE).CODE();
		ops_key_on  = OPS_CLOCK_SOURCE_ON;
		ops_key_off = OPS_CLOCK_SOURCE_OFF;
		bix = Shm->Registration.Driver.CS & REGISTRATION_ENABLE;
		break;
	    }
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 5
		}, select = {
			.col = 0,
			.row = bix == 0 ? 1 : 2
		};

	AppendWindow(CreateBox(scan->key, origin, select, (char*) ops_title,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
			ops_Str[0][bix != 0], stateAttr[bix != 0], ops_key_on,
			ops_Str[1][bix == 0], stateAttr[bix == 0], ops_key_off,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
			&winList);
	} else
		SetHead(&winList, win);
    }
    break;
    case OPS_INTERRUPTS_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_INTERRUPTS );
	}
    break;
    case OPS_INTERRUPTS_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_INTERRUPTS );
	}
    break;
    case OPS_CPU_IDLE_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CPU_IDLE );
	}
    break;
    case OPS_CPU_IDLE_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CPU_IDLE );
	}
    break;
    case OPS_CPU_FREQ_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CPU_FREQ );
	}
    break;
    case OPS_CPU_FREQ_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CPU_FREQ );
	}
    break;
    case OPS_GOVERNOR_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_GOVERNOR );
	}
    break;
    case OPS_GOVERNOR_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_GOVERNOR );
	}
    break;
    case OPS_CLOCK_SOURCE_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CLOCK_SOURCE );
	}
    break;
    case OPS_CLOCK_SOURCE_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CLOCK_SOURCE );
	}
    break;
    case OPS_THERMAL_SCOPE:
    case OPS_VOLTAGE_SCOPE:
    case OPS_POWER_SCOPE:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const ASCII *title[] = {
			RSC(BOX_SCOPE_THERMAL_TITLE).CODE(),
			RSC(BOX_SCOPE_VOLTAGE_TITLE).CODE(),
			RSC(BOX_SCOPE_POWER_TITLE).CODE()
		};
		const union {
			int			*pInteger;
			enum THERMAL_FORMULAS	*pThermal;
			enum VOLTAGE_FORMULAS	*pVoltage;
			enum POWER_FORMULAS	*pPower;
		} formula[] = {
			{ .pThermal	= &Shm->Proc.thermalFormula	},
			{ .pVoltage	= &Shm->Proc.voltageFormula	},
			{ .pPower	= &Shm->Proc.powerFormula	}
		};
		const int index = (scan->key & 0x000000000000f000) >> 12;
		const Coordinate origin = {
			.col = 43,
			.row = TOP_HEADER_ROW + 20
		}, select = {
			.col = 0,
			.row = SCOPE_OF_FORMULA((*formula[index].pInteger))
		};

		AppendWindow(CreateBox(scan->key, origin, select,
			(char*) title[index],
			RSC(BOX_SCOPE_NONE).CODE(), stateAttr[0],
		(scan->key & 0x100000000002f000) | (7 ^ FORMULA_SCOPE_NONE),
			RSC(BOX_SCOPE_THREAD).CODE(), stateAttr[0],
		(scan->key & 0x100000000002f000) | (7 ^ FORMULA_SCOPE_SMT),
			RSC(BOX_SCOPE_CORE).CODE(), stateAttr[0],
		(scan->key & 0x100000000002f000) | (7 ^ FORMULA_SCOPE_CORE),
			RSC(BOX_SCOPE_PACKAGE).CODE(), stateAttr[0],
		(scan->key & 0x100000000002f000) | (7 ^ FORMULA_SCOPE_PKG)),
			&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case OPS_THERMAL_SCOPE_NONE:
    case OPS_THERMAL_SCOPE_SMT:
    case OPS_THERMAL_SCOPE_CORE:
    case OPS_THERMAL_SCOPE_PKG:
    case OPS_VOLTAGE_SCOPE_NONE:
    case OPS_VOLTAGE_SCOPE_SMT:
    case OPS_VOLTAGE_SCOPE_CORE:
    case OPS_VOLTAGE_SCOPE_PKG:
    case OPS_POWER_SCOPE_NONE:
    case OPS_POWER_SCOPE_SMT:
    case OPS_POWER_SCOPE_CORE:
    case OPS_POWER_SCOPE_PKG:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				(scan->key & 0x000000000000f000) >> 12,
				MACHINE_FORMULA_SCOPE,
				(scan->key & 0x000000000000000f) ^ 7 );
	}
    break;
    case SCANKEY_HASH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateHotPlugCPU(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
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
	if (win == NULL) {
		AppendWindow(CreateAbout(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_b:
	if ((draw.View == V_TASKS) && (draw.Disposal == D_MAINVIEW)) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL) {
			AppendWindow(CreateSortByField(scan->key), &winList);
		} else {
			SetHead(&winList, win);
		}
	}
    break;
#ifndef NO_LOWER
    case SCANKEY_c:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_CYCLES;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
#endif
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
		if (win == NULL) {
			AppendWindow(CreateTracking(scan->key), &winList);
		} else {
			SetHead(&winList, win);
		}
	}
    break;
#ifndef NO_LOWER
    case SCANKEY_g:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_PACKAGE;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
#endif
    case SCANKEY_F1:
    case SCANCON_F1:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateAdvHelp(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_h:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateHelp(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
#ifndef NO_LOWER
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
#endif /* NO_LOWER */
    case SCANKEY_m:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateTopology(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_DOLLAR:
	Setting.jouleWatt = !Setting.jouleWatt;
	if (draw.Disposal == D_MAINVIEW) {
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_SHIFT_f:
	Setting.fahrCels = !Setting.fahrCels;
	draw.Flag.layout = 1;
    break;
    case SCANKEY_SHIFT_y:
	Setting.secret = !Setting.secret;
	draw.Flag.layout = 1;
    break;
    case SCANKEY_SHIFT_h:
    {
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
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_THERM_SENSOR) ? 1 : 0],
				BOXKEY_CLR_THM_SENSOR,
				RSC(BOX_EVENT_PROCHOT_AGENT).CODE(),
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_THERM_PROCHOT) ? 1 : 0],
				BOXKEY_CLR_THM_PROCHOT,
				RSC(BOX_EVENT_CRITICAL_TEMP).CODE(),
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_THERM_CRIT) ? 1 : 0],
				BOXKEY_CLR_THM_CRIT,
				RSC(BOX_EVENT_THERM_THRESHOLD).CODE(),
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_THERM_THOLD) ? 1 : 0],
				BOXKEY_CLR_THM_THOLD,
				RSC(BOX_EVENT_POWER_LIMIT).CODE(),
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_POWER_LIMIT) ? 2 : 0],
				BOXKEY_CLR_PWR_LIMIT,
				RSC(BOX_EVENT_CURRENT_LIMIT).CODE(),
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_CURRENT_LIMIT) ? 2 : 0],
				BOXKEY_CLR_CUR_LIMIT,
				RSC(BOX_EVENT_CROSS_DOM_LIMIT).CODE(),
	RSC(BOX_EVENT).ATTR()[(processorEvents & EVENT_CROSS_DOMAIN) ? 1 : 0],
				BOXKEY_CLR_X_DOMAIN);

	if (wBox != NULL) {
		AppendWindow(wBox, &winList);
	} else {
		SetHead(&winList, win);
	}
      } else {
		SetHead(&winList, win);
      }
    }
    break;
    case SCANKEY_SHIFT_i:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateISA(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_SHIFT_l:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col = 30,
			.row = TOP_HEADER_ROW + 6
		}, select = {
			.col = 0,
			.row = (GET_LOCALE() >= LOC_EN)
				&& (GET_LOCALE() < LOC_CNT) ?
					GET_LOCALE() : LOC_EN
		};

	Window *wBox = CreateBox(scan->key, origin, select,
		(char*) RSC(BOX_LANG_TITLE).CODE(),
		RSC(BOX_LANG_ENGLISH).CODE(), stateAttr[0], BOXKEY_LANG_ENGLISH,
		RSC(BOX_LANG_FRENCH).CODE(), stateAttr[0], BOXKEY_LANG_FRENCH);

		if (wBox != NULL) {
			AppendWindow(wBox, &winList);
		} else {
			SetHead(&winList, win);
		}
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_LANG_ENGLISH:
    {
	if (GET_LOCALE() != LOC_EN) {
		SET_LOCALE(LOC_EN);
		draw.Flag.layout = 1;
	}
    }
    break;
    case BOXKEY_LANG_FRENCH:
    {
	if (GET_LOCALE() != LOC_FR) {
		SET_LOCALE(LOC_FR);
		draw.Flag.layout = 1;
	}
    }
    break;
    case SCANKEY_SHIFT_m:
	if (Shm->Uncore.CtrlCount > 0) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL) {
			AppendWindow(CreateMemCtrl(scan->key),&winList);
		} else {
			SetHead(&winList, win);
		}
	}
    break;
    case SCANKEY_SHIFT_r:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateSysRegs(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
#ifndef NO_LOWER
    case SCANKEY_q:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_INTR;
	draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
    case SCANKEY_SHIFT_c:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_SENSORS;
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
    case SCANKEY_SHIFT_w:
    {
	draw.Disposal = D_MAINVIEW;
	draw.View = V_ENERGY;
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
#endif /* NO_LOWER */
    case SCANKEY_s:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateSettings(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
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
#ifndef NO_LOWER
    case SCANKEY_x:
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		Shm->SysGate.trackTask = 0;
		draw.Disposal = D_MAINVIEW;
		draw.View = V_TASKS;
		draw.Size.height = 0;
		TrapScreenSize(SIGWINCH);
	}
    break;
#endif
    case SCANKEY_EXCL:
	if (draw.Disposal == D_MAINVIEW) {
		draw.Load = !draw.Load;
		draw.Flag.layout = 1;
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
    case BOXKEY_L1_HW_PREFETCH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 2
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.L1_HW_Prefetch ? 2 : 1
		};

	AppendWindow(CreateBox(scan->key, origin, select,
			" DCU L1 Prefetcher ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.L1_HW_Prefetch],
			stateAttr[Shm->Proc.Technology.L1_HW_Prefetch],
				BOXKEY_L1_HW_PREFETCH_ON,
		stateStr[0][!Shm->Proc.Technology.L1_HW_Prefetch],
			stateAttr[!Shm->Proc.Technology.L1_HW_Prefetch],
				BOXKEY_L1_HW_PREFETCH_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_L1_HW_PREFETCH_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L1_HW_PREFETCH );
	}
    break;
    case BOXKEY_L1_HW_PREFETCH_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_L1_HW_PREFETCH );
	}
    break;
    case BOXKEY_L1_HW_IP_PREFETCH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.L1_HW_IP_Prefetch ? 2 : 1
		};

	AppendWindow(CreateBox(scan->key, origin, select,
			" DCU L1 IP Prefetcher ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.L1_HW_IP_Prefetch],
			stateAttr[Shm->Proc.Technology.L1_HW_IP_Prefetch],
				BOXKEY_L1_HW_IP_PREFETCH_ON,
		stateStr[0][!Shm->Proc.Technology.L1_HW_IP_Prefetch],
			stateAttr[!Shm->Proc.Technology.L1_HW_IP_Prefetch],
				BOXKEY_L1_HW_IP_PREFETCH_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_L1_HW_IP_PREFETCH_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L1_HW_IP_PREFETCH );
	}
    break;
    case BOXKEY_L1_HW_IP_PREFETCH_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_L1_HW_IP_PREFETCH );
	}
    break;
    case BOXKEY_L2_HW_PREFETCH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.L2_HW_Prefetch ? 2 : 1
		};

	AppendWindow(CreateBox(scan->key, origin, select,
			" DCU L2 Prefetcher ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.L2_HW_Prefetch],
			stateAttr[Shm->Proc.Technology.L2_HW_Prefetch],
				BOXKEY_L2_HW_PREFETCH_ON,
		stateStr[0][!Shm->Proc.Technology.L2_HW_Prefetch],
			stateAttr[!Shm->Proc.Technology.L2_HW_Prefetch],
				BOXKEY_L2_HW_PREFETCH_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_L2_HW_PREFETCH_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L2_HW_PREFETCH );
	}
    break;
    case BOXKEY_L2_HW_PREFETCH_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_L2_HW_PREFETCH );
	}
    break;
    case BOXKEY_L2_HW_CL_PREFETCH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 4
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.L2_HW_CL_Prefetch ? 2 : 1
		};

	AppendWindow(CreateBox(scan->key, origin, select,
			" DCU L2 CL Prefetcher ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.L2_HW_CL_Prefetch],
			stateAttr[Shm->Proc.Technology.L2_HW_CL_Prefetch],
				BOXKEY_L2_HW_CL_PREFETCH_ON,
		stateStr[0][!Shm->Proc.Technology.L2_HW_CL_Prefetch],
			stateAttr[!Shm->Proc.Technology.L2_HW_CL_Prefetch],
				BOXKEY_L2_HW_CL_PREFETCH_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_L2_HW_CL_PREFETCH_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L2_HW_CL_PREFETCH );
	}
    break;
    case BOXKEY_L2_HW_CL_PREFETCH_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_L2_HW_CL_PREFETCH );
	}
    break;
    case BOXKEY_EIST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 2
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.EIST ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " EIST ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_EIST_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.EIST],
			stateAttr[Shm->Proc.Technology.EIST] , BOXKEY_EIST_ON,
		stateStr[0][!Shm->Proc.Technology.EIST],
			stateAttr[!Shm->Proc.Technology.EIST], BOXKEY_EIST_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_EIST_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_EIST );
	}
    break;
    case BOXKEY_EIST_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_EIST );
	}
    break;
    case BOXKEY_C1E:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.C1E ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " C1E ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_C1E_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C1E],
			stateAttr[Shm->Proc.Technology.C1E] , BOXKEY_C1E_ON,
		stateStr[0][!Shm->Proc.Technology.C1E],
			stateAttr[!Shm->Proc.Technology.C1E], BOXKEY_C1E_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_C1E_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C1E );
	}
    break;
    case BOXKEY_C1E_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_C1E );
	}
    break;
    case BOXKEY_TURBO:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 2
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.Turbo ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " Turbo ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_TURBO_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.Turbo],
			stateAttr[Shm->Proc.Technology.Turbo] ,BOXKEY_TURBO_ON,
		stateStr[0][!Shm->Proc.Technology.Turbo],
			stateAttr[!Shm->Proc.Technology.Turbo],BOXKEY_TURBO_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_TURBO_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_TURBO );
	}
    break;
    case BOXKEY_TURBO_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_TURBO );
	}
    break;
    case BOXKEY_C1A:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 5
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.C1A ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " C1A ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_C1A_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C1A],
			stateAttr[Shm->Proc.Technology.C1A] , BOXKEY_C1A_ON,
		stateStr[0][!Shm->Proc.Technology.C1A],
			stateAttr[!Shm->Proc.Technology.C1A], BOXKEY_C1A_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_C1A_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C1A );
	}
    break;
    case BOXKEY_C1A_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_C1A );
	}
    break;
    case BOXKEY_C3A:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 6
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.C3A ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " C3A ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_C3A_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C3A] ,
			stateAttr[Shm->Proc.Technology.C3A] , BOXKEY_C3A_ON,
		stateStr[0][!Shm->Proc.Technology.C3A],
			stateAttr[!Shm->Proc.Technology.C3A], BOXKEY_C3A_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_C3A_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C3A );
	}
    break;
    case BOXKEY_C3A_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_C3A );
	}
    break;
    case BOXKEY_C1U:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 7
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.C1U ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select,
		(  (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		|| (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) ?
		" C2U " : " C1U ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		(  (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		|| (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) ?
		RSC(BOX_C2U_DESC).CODE() : RSC(BOX_C1U_DESC).CODE(),
			descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C1U] ,
			stateAttr[Shm->Proc.Technology.C1U] , BOXKEY_C1U_ON,
		stateStr[0][!Shm->Proc.Technology.C1U],
			stateAttr[!Shm->Proc.Technology.C1U], BOXKEY_C1U_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_C1U_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C1U );
	}
    break;
    case BOXKEY_C1U_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_C1U );
	}
    break;
    case BOXKEY_C3U:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 8
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.C3U ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " C3U ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_C3U_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.C3U] ,
			stateAttr[Shm->Proc.Technology.C3U] , BOXKEY_C3U_ON,
		stateStr[0][!Shm->Proc.Technology.C3U],
			stateAttr[!Shm->Proc.Technology.C3U], BOXKEY_C3U_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_C3U_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C3U );
	}
    break;
    case BOXKEY_C3U_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_C3U );
	}
    break;
    case BOXKEY_CC6:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	char *title;
	ASCII *descCode;
	const Coordinate origin = {
		.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 9
	}, select = {
		.col = 0,
		.row = Shm->Proc.Technology.CC6 ? 4 : 3
	};
	if ( (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) {
		title = " CC6 ";
		descCode = RSC(BOX_CC6_DESC).CODE();
	} else {
		title = " C6D ";
		descCode = RSC(BOX_C6D_DESC).CODE();
	}
	AppendWindow(CreateBox(scan->key, origin, select, title,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		descCode, descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.CC6] ,
			stateAttr[Shm->Proc.Technology.CC6] , BOXKEY_CC6_ON,
		stateStr[0][!Shm->Proc.Technology.CC6],
			stateAttr[!Shm->Proc.Technology.CC6], BOXKEY_CC6_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;
    case BOXKEY_CC6_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_CC6 );
	}
    break;
    case BOXKEY_CC6_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_CC6 );
	}
    break;
    case BOXKEY_PC6:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	char *title;
	ASCII *descCode;
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 10
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.PC6 ? 4 : 3
		};
	if ( (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) {
		title = " PC6 ";
		descCode = RSC(BOX_PC6_DESC).CODE();
	} else {
		title = " MC6 ";
		descCode = RSC(BOX_MC6_DESC).CODE();
	}
	AppendWindow(CreateBox(scan->key, origin, select, title,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		descCode, descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.PC6] ,
			stateAttr[Shm->Proc.Technology.PC6] , BOXKEY_PC6_ON,
		stateStr[0][!Shm->Proc.Technology.PC6],
			stateAttr[!Shm->Proc.Technology.PC6], BOXKEY_PC6_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;
    case BOXKEY_PC6_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_PC6 );
	}
    break;
    case BOXKEY_PC6_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_PC6 );
	}
    break;
    case BOXKEY_PKGCST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const CSINT CST[] = {
			[   _C0 ] = 12,
			[   _C1 ] = 11,
			[   _C2 ] = 10,
			[   _C3 ] = 9,
			[   _C4 ] = 8,
			[   _C6 ] = 7,
			[  _C6R ] = 6,
			[   _C7 ] = 5,
			[  _C7S ] = 4,
			[   _C8 ] = 3,
			[   _C9 ] = 2,
			[  _C10 ] = 1,
			[_UNSPEC] = 0
		};
		const Coordinate origin = {
			.col = (draw.Size.width - (44 - 17)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
		.col = 0,
		.row = CST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit]
		};

		Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_PKG_STATE_LIMIT_TITLE).CODE(),
/* 0 */ 	RSC(BOX_STATE_UNSPECIFIED).CODE(), stateAttr[0], SCANKEY_NULL,
/* 1 */ (ASCII*)"            C10            ", stateAttr[0], BOXKEY_PKGCST_C10,
/* 2 */ (ASCII*)"             C9            ", stateAttr[0], BOXKEY_PKGCST_C9,
/* 3 */ (ASCII*)"             C8            ", stateAttr[0], BOXKEY_PKGCST_C8,
/* 4 */ (ASCII*)"            C7S            ", stateAttr[0], BOXKEY_PKGCST_C7S,
/* 5 */ (ASCII*)"             C7            ", stateAttr[0], BOXKEY_PKGCST_C7,
/* 6 */ (ASCII*)"            C6R            ", stateAttr[0], BOXKEY_PKGCST_C6R,
/* 7 */ (ASCII*)"             C6            ", stateAttr[0], BOXKEY_PKGCST_C6,
/* 8 */ (ASCII*)"             C4            ", stateAttr[0], BOXKEY_PKGCST_C4,
/* 9 */ (ASCII*)"             C3            ", stateAttr[0], BOXKEY_PKGCST_C3,
/*10 */ (ASCII*)"             C2            ", stateAttr[0], BOXKEY_PKGCST_C2,
/*11 */ (ASCII*)"             C1            ", stateAttr[0], BOXKEY_PKGCST_C1,
/*12 */ (ASCII*)"             C0            ", stateAttr[0], BOXKEY_PKGCST_C0);

	    if (wBox != NULL)
	    {
	      if(Shm->Cpu[Shm->Proc.Service.Core].Query.CStateLimit != _UNSPEC)
		{
			TCellAt(wBox, 0, select.row).attr[11] = 	\
			TCellAt(wBox, 0, select.row).attr[12] = 	\
			TCellAt(wBox, 0, select.row).attr[13] = 	\
			TCellAt(wBox, 0, select.row).attr[14] = 	\
			TCellAt(wBox, 0, select.row).attr[15] = 	\
			TCellAt(wBox, 0, select.row).attr[16] = stateAttr[1];
			TCellAt(wBox, 0, select.row).item[11] = '<';
			TCellAt(wBox, 0, select.row).item[16] = '>';
		} else {
			TCellAt(wBox, 0, select.row).item[ 6] = '<';
			TCellAt(wBox, 0, select.row).item[20] = '>';
		}
		AppendWindow(wBox, &winList);
	    } else {
		SetHead(&winList, win);
	    }
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_PKGCST_C10:
    case BOXKEY_PKGCST_C9:
    case BOXKEY_PKGCST_C8:
    case BOXKEY_PKGCST_C7S:
    case BOXKEY_PKGCST_C7:
    case BOXKEY_PKGCST_C6:
    case BOXKEY_PKGCST_C6R:
    case BOXKEY_PKGCST_C4:
    case BOXKEY_PKGCST_C3:
    case BOXKEY_PKGCST_C2:
    case BOXKEY_PKGCST_C1:
    case BOXKEY_PKGCST_C0:
    {
	const unsigned long newCST = (scan->key & BOXKEY_CSTATE_MASK) >> 4;
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				newCST,
				TECHNOLOGY_PKG_CSTATE );
	}
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
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 9
		}, select = {
			.col = 0,
			.row = isIORedir ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_IO_MWAIT_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
			RSC(BOX_IO_MWAIT_DESC).CODE(), descAttr, SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
	stateStr[1][isIORedir] , stateAttr[isIORedir] , BOXKEY_IOMWAIT_ON,
	stateStr[0][!isIORedir], stateAttr[!isIORedir], BOXKEY_IOMWAIT_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
			&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_IOMWAIT_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_IO_MWAIT );
	}
    break;
    case BOXKEY_IOMWAIT_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_IO_MWAIT );
	}
    break;
    case BOXKEY_IORCST:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const CSINT CST[] = {
			[   _C0 ] = -1,
			[   _C1 ] = -1,
			[   _C2 ] = -1,
			[   _C3 ] =  5,
			[   _C4 ] =  4,
			[   _C6 ] =  3,
			[  _C6R ] = -1,
			[   _C7 ] =  2,
			[  _C7S ] = -1,
			[   _C8 ] =  1,
			[   _C9 ] = -1,
			[  _C10 ] = -1,
			[_UNSPEC] =  0
		};
		const Coordinate origin = {
			.col = (draw.Size.width - (44 - 17)) / 2,
			.row = TOP_HEADER_ROW + 4
		}, select = {
		.col = 0,
	.row = CST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude] != -1?
		CST[Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude] : 0
		};

		Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_MWAIT_MAX_STATE_TITLE).CODE(),
/* 0 */ 	RSC(BOX_STATE_UNSPECIFIED).CODE(), stateAttr[0], SCANKEY_NULL,
/* 1 */ (ASCII*)"             C8            ", stateAttr[0], BOXKEY_IORCST_C8,
/* 2 */ (ASCII*)"             C7            ", stateAttr[0], BOXKEY_IORCST_C7,
/* 3 */ (ASCII*)"             C6            ", stateAttr[0], BOXKEY_IORCST_C6,
/* 4 */ (ASCII*)"             C4            ", stateAttr[0], BOXKEY_IORCST_C4,
/* 5 */ (ASCII*)"             C3            ", stateAttr[0], BOXKEY_IORCST_C3);

	  if (wBox != NULL)
	  {
	    if(Shm->Cpu[Shm->Proc.Service.Core].Query.CStateInclude != _UNSPEC)
	    {
		TCellAt(wBox, 0, select.row).attr[11] = 	\
		TCellAt(wBox, 0, select.row).attr[12] = 	\
		TCellAt(wBox, 0, select.row).attr[13] = 	\
		TCellAt(wBox, 0, select.row).attr[14] = 	\
		TCellAt(wBox, 0, select.row).attr[15] = 	\
		TCellAt(wBox, 0, select.row).attr[16] = stateAttr[1];
		TCellAt(wBox, 0, select.row).item[11] = '<';
		TCellAt(wBox, 0, select.row).item[16] = '>';
	    } else {
		TCellAt(wBox, 0, select.row).item[ 6] = '<';
		TCellAt(wBox, 0, select.row).item[20] = '>';
	    }
		AppendWindow(wBox, &winList);
	  } else {
		SetHead(&winList, win);
	  }
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_IORCST_C3:
    case BOXKEY_IORCST_C4:
    case BOXKEY_IORCST_C6:
    case BOXKEY_IORCST_C7:
    case BOXKEY_IORCST_C8:
    {
	const unsigned long newCST = (scan->key & BOXKEY_CSTATE_MASK) >> 4;
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				newCST,
				TECHNOLOGY_IO_MWAIT_REDIR );
	}
    }
    break;
    case BOXKEY_ODCM:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col =(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 6
		}, select = {
			.col = 0,
			.row = Shm->Proc.Technology.ODCM ? 4 : 3
		};

		AppendWindow(CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_ODCM_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
			RSC(BOX_ODCM_DESC).CODE(),descAttr,SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Technology.ODCM] ,
			stateAttr[Shm->Proc.Technology.ODCM] , BOXKEY_ODCM_ON,
		stateStr[0][!Shm->Proc.Technology.ODCM],
			stateAttr[!Shm->Proc.Technology.ODCM],BOXKEY_ODCM_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
			&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_ODCM_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_ODCM );
	}
    break;
    case BOXKEY_ODCM_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_ODCM );
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
			.col = (draw.Size.width - 27) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0, .row = (
	Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod <=maxCM
	) ? Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.ClockMod : 1
		};
	Window *wBox = NULL;

	  if (Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.DutyCycle.Extended)
	  {
		wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_EXT_DUTY_CYCLE_TITLE).CODE(),
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
	(ASCII*)"           62.50%          ", stateAttr[0], BOXKEY_ODCM_DC10,
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
	  } else {
		SetHead(&winList, win);
	  }
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_PWR_POLICY:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
		.col = (draw.Size.width - (2 + RSZ(BOX_POWER_POLICY_LOW))) / 2,
		.row = TOP_HEADER_ROW + 3
	    }, select = {
		.col = 0,
		.row = Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.PowerPolicy
	    };
		Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_POWER_POLICY_TITLE).CODE(),
	RSC(BOX_POWER_POLICY_LOW).CODE()  , stateAttr[0], BOXKEY_PWR_POL00,
	(ASCII*)"            1           ", stateAttr[0], BOXKEY_PWR_POL01,
	(ASCII*)"            2           ", stateAttr[0], BOXKEY_PWR_POL02,
	(ASCII*)"            3           ", stateAttr[0], BOXKEY_PWR_POL03,
	(ASCII*)"            4           ", stateAttr[0], BOXKEY_PWR_POL04,
	(ASCII*)"            5           ", stateAttr[0], BOXKEY_PWR_POL05,
	(ASCII*)"            6           ", stateAttr[0], BOXKEY_PWR_POL06,
	(ASCII*)"            7           ", stateAttr[0], BOXKEY_PWR_POL07,
	(ASCII*)"            8           ", stateAttr[0], BOXKEY_PWR_POL08,
	(ASCII*)"            9           ", stateAttr[0], BOXKEY_PWR_POL09,
	(ASCII*)"           10           ", stateAttr[0], BOXKEY_PWR_POL10,
	(ASCII*)"           11           ", stateAttr[0], BOXKEY_PWR_POL11,
	(ASCII*)"           12           ", stateAttr[0], BOXKEY_PWR_POL12,
	(ASCII*)"           13           ", stateAttr[0], BOXKEY_PWR_POL13,
	(ASCII*)"           14           ", stateAttr[0], BOXKEY_PWR_POL14,
	RSC(BOX_POWER_POLICY_HIGH).CODE() , stateAttr[0], BOXKEY_PWR_POL15);

	    if (wBox != NULL) {
		TCellAt(wBox, 0, select.row).attr[ 8] = 	\
		TCellAt(wBox, 0, select.row).attr[ 9] = 	\
		TCellAt(wBox, 0, select.row).attr[10] = 	\
		TCellAt(wBox, 0, select.row).attr[11] = 	\
		TCellAt(wBox, 0, select.row).attr[12] = 	\
		TCellAt(wBox, 0, select.row).attr[13] = 	\
		TCellAt(wBox, 0, select.row).attr[14] = 	\
		TCellAt(wBox, 0, select.row).attr[15] = 	\
		TCellAt(wBox, 0, select.row).attr[16] = stateAttr[1];
		TCellAt(wBox, 0, select.row).item[ 8] = '<';
		if (select.row > 9) {
			TCellAt(wBox, 0, select.row).item[15] = '>';
		} else {
			TCellAt(wBox, 0, select.row).item[16] = '>';
		}
		AppendWindow(wBox, &winList);
	    } else {
		SetHead(&winList, win);
	    }
	} else {
		SetHead(&winList, win);
	}
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
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				newDC,
				TECHNOLOGY_ODCM_DUTYCYCLE );
	}
    }
    break;
    case BOXKEY_PWR_POL00:
    case BOXKEY_PWR_POL01:
    case BOXKEY_PWR_POL02:
    case BOXKEY_PWR_POL03:
    case BOXKEY_PWR_POL04:
    case BOXKEY_PWR_POL05:
    case BOXKEY_PWR_POL06:
    case BOXKEY_PWR_POL07:
    case BOXKEY_PWR_POL08:
    case BOXKEY_PWR_POL09:
    case BOXKEY_PWR_POL10:
    case BOXKEY_PWR_POL11:
    case BOXKEY_PWR_POL12:
    case BOXKEY_PWR_POL13:
    case BOXKEY_PWR_POL14:
    case BOXKEY_PWR_POL15:
    {
	const unsigned long newPolicy = (scan->key - BOXKEY_PWR_POL00) >> 4;
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				newPolicy,
				TECHNOLOGY_POWER_POLICY );
	}
    }
    break;
    case BOXKEY_HWP_EPP:
    {
	CPU_STRUCT *SProc = &Shm->Cpu[Shm->Proc.Service.Core];
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
		.col = (draw.Size.width - (2 + RSZ(BOX_POWER_POLICY_LOW))) / 2,
		.row = TOP_HEADER_ROW + 3
	    }, select = {
		.col = 0,
		.row = SProc->PowerThermal.HWP.Request.Energy_Pref == 0xff ?
		   3 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0xc0 ?
		   2 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0x80 ?
		   1 : 0
	    };
		Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_POWER_POLICY_TITLE).CODE(),
	RSC(BOX_HWP_POLICY_MIN).CODE(), stateAttr[0], BOXKEY_HWP_EPP_MIN,
	RSC(BOX_HWP_POLICY_MED).CODE(), stateAttr[0], BOXKEY_HWP_EPP_MED,
	RSC(BOX_HWP_POLICY_PWR).CODE(), stateAttr[0], BOXKEY_HWP_EPP_PWR,
	RSC(BOX_HWP_POLICY_MAX).CODE(), stateAttr[0], BOXKEY_HWP_EPP_MAX);

	    if (wBox != NULL) {
		TCellAt(wBox, 0, select.row).attr[ 6] = 	\
		TCellAt(wBox, 0, select.row).attr[ 7] = 	\
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
		TCellAt(wBox, 0, select.row).attr[18] = stateAttr[1];
		TCellAt(wBox, 0, select.row).item[ 6] = '<';
		TCellAt(wBox, 0, select.row).item[18] = '>';

		AppendWindow(wBox, &winList);
	    } else {
		SetHead(&winList, win);
	    }
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_HWP_EPP_MIN:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x0,
				TECHNOLOGY_HWP_EPP );
	}
    break;
    case BOXKEY_HWP_EPP_MED:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x80,
				TECHNOLOGY_HWP_EPP );
	}
    break;
    case BOXKEY_HWP_EPP_PWR:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0xc0,
				TECHNOLOGY_HWP_EPP );
	}
    break;
    case BOXKEY_HWP_EPP_MAX:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0xff,
				TECHNOLOGY_HWP_EPP );
	}
    break;
    case BOXKEY_HWP:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 11
		}, select = {
			.col = 0,
			.row = 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " HWP ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_HWP_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Features.HWP_Enable],
			stateAttr[Shm->Proc.Features.HWP_Enable],BOXKEY_HWP_ON,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_HWP_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_HWP );
	}
    break;
    case BOXKEY_HDC:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 12
		}, select = {
			.col = 0,
			.row = Shm->Proc.Features.HDC_Enable ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " HDC ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_HDC_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Features.HDC_Enable],
		stateAttr[Shm->Proc.Features.HDC_Enable],BOXKEY_HDC_ON,
		stateStr[0][!Shm->Proc.Features.HDC_Enable],
		stateAttr[!Shm->Proc.Features.HDC_Enable],BOXKEY_HDC_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_HDC_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_HDC );
	}
    break;
    case BOXKEY_HDC_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_HDC );
	}
    break;
    case BOXKEY_R2H:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col=(draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
			.row = TOP_HEADER_ROW + 12
		}, select = {
			.col = 0,
			.row = Shm->Proc.Features.R2H_Disable ? 4 : 3
		};

	AppendWindow(CreateBox(scan->key, origin, select, " R2H ",
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_R2H_DESC).CODE(), descAttr, SCANKEY_NULL,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
		stateStr[1][Shm->Proc.Features.R2H_Disable],
		stateAttr[Shm->Proc.Features.R2H_Disable],BOXKEY_R2H_ON,
		stateStr[0][!Shm->Proc.Features.R2H_Disable],
		stateAttr[!Shm->Proc.Features.R2H_Disable],BOXKEY_R2H_OFF,
		RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_R2H_OFF:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_R2H );
	}
    break;
    case BOXKEY_R2H_ON:
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_R2H );
	}
    break;
    case BOXKEY_LIMIT_IDLE_STATE:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateSelectIdle(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_LIMIT_IDLE_ST00:
    case BOXKEY_LIMIT_IDLE_ST01:
    case BOXKEY_LIMIT_IDLE_ST02:
    case BOXKEY_LIMIT_IDLE_ST03:
    case BOXKEY_LIMIT_IDLE_ST04:
    case BOXKEY_LIMIT_IDLE_ST05:
    case BOXKEY_LIMIT_IDLE_ST06:
    case BOXKEY_LIMIT_IDLE_ST07:
    case BOXKEY_LIMIT_IDLE_ST08:
    case BOXKEY_LIMIT_IDLE_ST09:
    case BOXKEY_LIMIT_IDLE_ST10:
    {
	const unsigned long newLim = (scan->key - BOXKEY_LIMIT_IDLE_ST00) >> 4;
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(	Shm->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				newLim,
				MACHINE_LIMIT_IDLE );
	}
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
      if (!RING_FULL(Shm->Ring[0])) {
	RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CLEAR_EVENTS, events);
      }
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
    case BOXKEY_TURBO_CLOCK_9C:
    case BOXKEY_TURBO_CLOCK_10C:
    case BOXKEY_TURBO_CLOCK_11C:
    case BOXKEY_TURBO_CLOCK_12C:
    case BOXKEY_TURBO_CLOCK_13C:
    case BOXKEY_TURBO_CLOCK_14C:
    case BOXKEY_TURBO_CLOCK_15C:
    case BOXKEY_TURBO_CLOCK_16C:
    case BOXKEY_TURBO_CLOCK_17C:
    case BOXKEY_TURBO_CLOCK_18C:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR,
				i18C = (((id >> 32) & RATIO_MASK) % 18) - 1;

		PKG_ITEM_CALLBACK Pkg_Item_Turbo_Freq[18] = {
				Pkg_Item_Turbo_1C,
				Pkg_Item_Turbo_2C,
				Pkg_Item_Turbo_3C,
				Pkg_Item_Turbo_4C,
				Pkg_Item_Turbo_5C,
				Pkg_Item_Turbo_6C,
				Pkg_Item_Turbo_7C,
				Pkg_Item_Turbo_8C,
				Pkg_Item_Turbo_9C,
				Pkg_Item_Turbo_10C,
				Pkg_Item_Turbo_11C,
				Pkg_Item_Turbo_12C,
				Pkg_Item_Turbo_13C,
				Pkg_Item_Turbo_14C,
				Pkg_Item_Turbo_15C,
				Pkg_Item_Turbo_16C,
				Pkg_Item_Turbo_17C,
				Pkg_Item_Turbo_18C
		};
		CPU_ITEM_CALLBACK CPU_Item_Turbo_Freq[18] = {
				CPU_Item_Turbo_1C,
				CPU_Item_Turbo_2C,
				CPU_Item_Turbo_3C,
				CPU_Item_Turbo_4C,
				CPU_Item_Turbo_5C,
				CPU_Item_Turbo_6C,
				CPU_Item_Turbo_7C,
				CPU_Item_Turbo_8C,
				CPU_Item_Turbo_9C,
				CPU_Item_Turbo_10C,
				CPU_Item_Turbo_11C,
				CPU_Item_Turbo_12C,
				CPU_Item_Turbo_13C,
				CPU_Item_Turbo_14C,
				CPU_Item_Turbo_15C,
				CPU_Item_Turbo_16C,
				CPU_Item_Turbo_17C,
				CPU_Item_Turbo_18C
		};
		UPDATE_CALLBACK Pkg_Turbo_Freq_Update[18] = {
				Pkg_Update_Turbo_1C,
				Pkg_Update_Turbo_2C,
				Pkg_Update_Turbo_3C,
				Pkg_Update_Turbo_4C,
				Pkg_Update_Turbo_5C,
				Pkg_Update_Turbo_6C,
				Pkg_Update_Turbo_7C,
				Pkg_Update_Turbo_8C,
				Pkg_Update_Turbo_9C,
				Pkg_Update_Turbo_10C,
				Pkg_Update_Turbo_11C,
				Pkg_Update_Turbo_12C,
				Pkg_Update_Turbo_13C,
				Pkg_Update_Turbo_14C,
				Pkg_Update_Turbo_15C,
				Pkg_Update_Turbo_16C,
				Pkg_Update_Turbo_17C,
				Pkg_Update_Turbo_18C
		};
		UPDATE_CALLBACK CPU_Turbo_Freq_Update[18] = {
				CPU_Update_Turbo_1C,
				CPU_Update_Turbo_2C,
				CPU_Update_Turbo_3C,
				CPU_Update_Turbo_4C,
				CPU_Update_Turbo_5C,
				CPU_Update_Turbo_6C,
				CPU_Update_Turbo_7C,
				CPU_Update_Turbo_8C,
				CPU_Update_Turbo_9C,
				CPU_Update_Turbo_10C,
				CPU_Update_Turbo_11C,
				CPU_Update_Turbo_12C,
				CPU_Update_Turbo_13C,
				CPU_Update_Turbo_14C,
				CPU_Update_Turbo_15C,
				CPU_Update_Turbo_16C,
				CPU_Update_Turbo_17C,
				CPU_Update_Turbo_18C
		};

		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_Turbo_Freq[i18C],
						CPU_Item_Turbo_Freq[i18C],
						Pkg_Turbo_Freq_Update[i18C],
						CPU_Turbo_Freq_Update[i18C] ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_RATIO_CLOCK_TGT:
    {
	unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR;
	Window *win = SearchWinListById(id, &winList);
	if (win == NULL) {
		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_Target_Freq,
						CPU_Item_Target_Freq,
						Pkg_Target_Freq_Update,
						CPU_Target_Freq_Update ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_RATIO_CLOCK_HWP_TGT:
    {
	unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR;
	Window *win = SearchWinListById(id, &winList);
	if (win == NULL) {
		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_HWP_Target_Freq,
						CPU_Item_HWP_Target_Freq,
						Pkg_HWP_Target_Freq_Update,
						CPU_HWP_Target_Freq_Update ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_RATIO_CLOCK_HWP_MAX:
    {
	unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR;
	Window *win = SearchWinListById(id, &winList);
	if (win == NULL) {
		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_HWP_Max_Freq,
						CPU_Item_HWP_Max_Freq,
						Pkg_HWP_Max_Freq_Update,
						CPU_HWP_Max_Freq_Update ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_RATIO_CLOCK_HWP_MIN:
    {
	unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR;
	Window *win = SearchWinListById(id, &winList);
	if (win == NULL) {
		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_HWP_Min_Freq,
						CPU_Item_HWP_Min_Freq,
						Pkg_HWP_Min_Freq_Update,
						CPU_HWP_Min_Freq_Update ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_RATIO_CLOCK_MAX:
    {
	unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR;
	Window *win = SearchWinListById(id, &winList);
	if (win == NULL) {
		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_Max_Freq,
						CPU_Item_Max_Freq,
						Pkg_Max_Freq_Update,
						CPU_Max_Freq_Update ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_RATIO_CLOCK_MIN:
    {
	unsigned long long id = scan->key | BOXKEY_RATIO_SELECT_OR;
	Window *win = SearchWinListById(id, &winList);
	if (win == NULL) {
		AppendWindow( CreateSelectFreq( id,
						Pkg_Item_Min_Freq,
						CPU_Item_Min_Freq,
						Pkg_Min_Freq_Update,
						CPU_Min_Freq_Update ),
				&winList );
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_UNCORE_CLOCK_MAX:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		CPU_STRUCT *SProc = &Shm->Cpu[Shm->Proc.Service.Core];
		struct FLIP_FLOP *CFlop = &SProc->FlipFlop[
					!Shm->Cpu[Shm->Proc.Service.Core].Toggle
		];
		CLOCK_ARG clockMod  = {.sllong = scan->key};
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;

		signed int lowestShift, highestShift;
		ComputeRatioShifts(Shm->Uncore.Boost[BOOST(MAX)],
				Shm->Uncore.Boost[BOOST(MIN)],
				MAXCLOCK_TO_RATIO(unsigned int,CFlop->Clock.Hz),
				&lowestShift,
				&highestShift);

		AppendWindow(CreateRatioClock(scan->key,
				Shm->Uncore.Boost[BOOST(MAX)],
				-1,
				NC,
				lowestShift,
				highestShift,

				( Shm->Uncore.Boost[BOOST(MIN)]
				+ Shm->Proc.Features.Factory.Ratio ) >> 1,

				Shm->Proc.Features.Factory.Ratio
			    + ((MAXCLOCK_TO_RATIO(signed int, CFlop->Clock.Hz)
				- Shm->Proc.Features.Factory.Ratio ) >> 1),

				BOXKEY_UNCORE_CLOCK,
				TitleForUncoreClock,
				36), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_UNCORE_CLOCK_MIN:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		CLOCK_ARG clockMod  = {.sllong = scan->key};
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;

		signed int lowestShift, highestShift;
		ComputeRatioShifts(Shm->Uncore.Boost[BOOST(MIN)],
				1,
				Shm->Proc.Features.Factory.Ratio,
				&lowestShift,
				&highestShift);

		AppendWindow(CreateRatioClock(scan->key,
				Shm->Uncore.Boost[BOOST(MIN)],
				-1,
				NC,
				lowestShift,
				highestShift,

				( Shm->Uncore.Boost[BOOST(MIN)]
				+ Shm->Proc.Features.Factory.Ratio ) >> 1,

				Shm->Proc.Features.Factory.Ratio - 1,

				BOXKEY_UNCORE_CLOCK,
				TitleForUncoreClock,
				37), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_F10:
    case BOXKEY_TOOLS_MACHINE:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE(Shm->Ring[1], COREFREQ_ORDER_MACHINE, COREFREQ_TOGGLE_OFF);
      }
    break;
    case SCANKEY_SHIFT_o:
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
		RSC(BOX_TOOLS_STOP_BURN).CODE(), BITVAL(Shm->Proc.Sync, BURN) ?
					MakeAttr(RED,0,BLACK,1) : blankAttr,
				BITVAL(Shm->Proc.Sync, BURN) ?
					BOXKEY_TOOLS_MACHINE : SCANKEY_NULL,
	RSC(BOX_TOOLS_ATOMIC_BURN).CODE(),stateAttr[0], BOXKEY_TOOLS_ATOMIC,
	RSC(BOX_TOOLS_CRC32_BURN).CODE() ,stateAttr[0], BOXKEY_TOOLS_CRC32,
	RSC(BOX_TOOLS_CONIC_BURN).CODE() ,stateAttr[0], BOXKEY_TOOLS_CONIC,
	RSC(BOX_TOOLS_RANDOM_CPU).CODE() ,stateAttr[0], BOXKEY_TOOLS_TURBO_RND,
    RSC(BOX_TOOLS_ROUND_ROBIN_CPU).CODE(),stateAttr[0], BOXKEY_TOOLS_TURBO_RR,
	RSC(BOX_TOOLS_USER_CPU).CODE()   ,stateAttr[0], BOXKEY_TOOLS_TURBO_CPU);

		if (wBox != NULL) {
			AppendWindow(wBox, &winList);
		} else {
			SetHead(&winList, win);
		}
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_TOOLS_ATOMIC:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE(	Shm->Ring[1],
			COREFREQ_ORDER_ATOMIC,
			Shm->Proc.Service.Core,
			COREFREQ_TOGGLE_ON);
      }
    break;
    case BOXKEY_TOOLS_CRC32:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE(	Shm->Ring[1],
			COREFREQ_ORDER_CRC32,
			Shm->Proc.Service.Core,
			COREFREQ_TOGGLE_ON);
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
		} else {
			SetHead(&winList, win);
		}
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case BOXKEY_TOOLS_CONIC0:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_ELLIPSOID,
				Shm->Ring[1],
				COREFREQ_ORDER_CONIC,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_CONIC1:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_HYPERBOLOID_ONE_SHEET,
				Shm->Ring[1],
				COREFREQ_ORDER_CONIC,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_CONIC2:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_HYPERBOLOID_TWO_SHEETS,
				Shm->Ring[1],
				COREFREQ_ORDER_CONIC,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_CONIC3:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_ELLIPTICAL_CYLINDER,
				Shm->Ring[1],
				COREFREQ_ORDER_CONIC,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_CONIC4:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_HYPERBOLIC_CYLINDER,
				Shm->Ring[1],
				COREFREQ_ORDER_CONIC,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_CONIC5:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_TWO_PARALLEL_PLANES,
				Shm->Ring[1],
				COREFREQ_ORDER_CONIC,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_TURBO_RND:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	RAND_SMT,
				Shm->Ring[1],
				COREFREQ_ORDER_TURBO,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_TURBO_RR:
      if (!RING_FULL(Shm->Ring[1])) {
	RING_WRITE_SUB_CMD(	RR_SMT,
				Shm->Ring[1],
				COREFREQ_ORDER_TURBO,
				Shm->Proc.Service.Core );
      }
    break;
    case BOXKEY_TOOLS_TURBO_CPU:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
		AppendWindow(CreateSelectCPU(scan->key), &winList);
	else
		SetHead(&winList, win);
    }
    break;
    case SCANKEY_k:
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) == 0) {
		break;
	}
	/* fallthrough */
    case SCANKEY_e:
    case SCANKEY_o:
    case SCANKEY_p:
    case SCANKEY_t:
    case SCANKEY_u:
    case SCANKEY_w:
    case SCANKEY_SHIFT_b:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateSysInfo(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    case SCANKEY_CTRL_p:
	if (DumpStatus())
	{
		AbortDump();
	}
	else if (StartDump("corefreq_%llx.asc", 0, DUMP_TO_ANSI) == 0)
	{
		draw.Flag.layout = 1;
	}
    break;
    case SCANKEY_ALT_p:
    case SCANCON_ALT_p:
	if (DumpStatus())
	{
		AbortDump();
	}
	else if (StartDump(	"corefreq_%llx.cast",
				Recorder.Reset - 1,
				DUMP_TO_JSON) == 0 )
	{
		draw.Flag.layout = 1;
	}
    break;
    case OPS_RECORDER_RESET:
    case OPS_RECORDER_X002:
    case OPS_RECORDER_X010:
    case OPS_RECORDER_X020:
    case OPS_RECORDER_X060:
    case OPS_RECORDER_X090:
    case OPS_RECORDER_X120:
    case OPS_RECORDER_X240:
    {
	__typeof__(Recorder.Select) select=(scan->key & OPS_RECORDER_MASK) >> 4;
	Recorder.Select = select;
	RECORDER_COMPUTE(Recorder, Shm->Sleep.Interval);
    }
    break;
    case OPS_RECORDER:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateRecorder(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;
    default:
      if (scan->key & TRACK_TASK) {
	Shm->SysGate.trackTask = scan->key & TRACK_MASK;
	draw.Flag.layout = 1;
      }
      else if (scan->key & SMBIOS_STRING_INDEX) {
	if (draw.Disposal == D_MAINVIEW) {
		enum SMB_STRING usrIdx = scan->key & SMBIOS_STRING_MASK;
		if (usrIdx < SMB_STRING_COUNT) {
			draw.SmbIndex = usrIdx;
			draw.Flag.layout = 1;
		}
	}
      }
      else if (scan->key & CPU_ONLINE) {
	const unsigned long cpu = scan->key & CPU_MASK;
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CPU_ON, cpu);
	}
      }
      else if (scan->key & CPU_OFFLINE) {
	const unsigned long cpu = scan->key & CPU_MASK;
	if (!RING_FULL(Shm->Ring[0])) {
		RING_WRITE(Shm->Ring[0], COREFREQ_IOCTL_CPU_OFF, cpu);
	}
      }
      else if (scan->key & CPU_SELECT) {
	const unsigned short cpu = scan->key & CPU_MASK;
	if (!RING_FULL(Shm->Ring[1])) {
		RING_WRITE_SUB_CMD(	USR_CPU,
					Shm->Ring[1],
					COREFREQ_ORDER_TURBO,
					cpu );
	}
      }
      else {
	switch (scan->key & (~BOXKEY_RATIO_SELECT_OR ^ RATIO_MASK))
	{
	case BOXKEY_TURBO_CLOCK_1C:
	case BOXKEY_TURBO_CLOCK_2C:
	case BOXKEY_TURBO_CLOCK_3C:
	case BOXKEY_TURBO_CLOCK_4C:
	case BOXKEY_TURBO_CLOCK_5C:
	case BOXKEY_TURBO_CLOCK_6C:
	case BOXKEY_TURBO_CLOCK_7C:
	case BOXKEY_TURBO_CLOCK_8C:
	case BOXKEY_TURBO_CLOCK_9C:
	case BOXKEY_TURBO_CLOCK_10C:
	case BOXKEY_TURBO_CLOCK_11C:
	case BOXKEY_TURBO_CLOCK_12C:
	case BOXKEY_TURBO_CLOCK_13C:
	case BOXKEY_TURBO_CLOCK_14C:
	case BOXKEY_TURBO_CLOCK_15C:
	case BOXKEY_TURBO_CLOCK_16C:
	case BOXKEY_TURBO_CLOCK_17C:
	case BOXKEY_TURBO_CLOCK_18C:
	  if (Shm->Proc.Features.Turbo_Unlock)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		struct FLIP_FLOP *CFlop;
		CLOCK_ARG clockMod = {.sllong = scan->key};
		unsigned int COF;
		const unsigned int lowestOperating = 1;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		const enum RATIO_BOOST boost = BOOST(SIZE) - NC;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	    if (cpu == -1) {
		COF = Shm->Cpu[ Ruler.Top[boost] ].Boost[boost];
		CFlop = &Shm->Cpu[ Ruler.Top[boost] ].FlipFlop[
				!Shm->Cpu[ Ruler.Top[boost] ].Toggle
			];
	    } else {
		COF = Shm->Cpu[cpu].Boost[boost];
		CFlop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	    }
		ComputeRatioShifts(COF,
				lowestOperating,
				MAXCLOCK_TO_RATIO(unsigned int,CFlop->Clock.Hz),
				&lowestShift,
				&highestShift);

		AppendWindow(
			CreateRatioClock(scan->key,
				COF,
				cpu,
				NC,
				lowestShift,
				highestShift,

				( lowestOperating
				+ Shm->Proc.Features.Factory.Ratio ) >> 1,

				Shm->Proc.Features.Factory.Ratio
			    + ((MAXCLOCK_TO_RATIO(signed int, CFlop->Clock.Hz)
				- Shm->Proc.Features.Factory.Ratio ) >> 1),

				BOXKEY_TURBO_CLOCK,
				TitleForTurboClock,
				34),
			&winList );
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;
	case BOXKEY_RATIO_CLOCK_TGT:
	  if (Shm->Proc.Features.TgtRatio_Unlock)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	   if (win == NULL)
	   {
		CLOCK_ARG clockMod = {.sllong = scan->key};
		unsigned int COF, lowestOperating, highestOperating;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	    if (cpu == -1) {
		COF = Shm->Cpu[ Ruler.Top[BOOST(TGT)] ].Boost[BOOST(TGT)];
		lowestOperating = Shm->Cpu[
					Ruler.Top[ BOOST(TGT) ]
					].Boost[ BOOST(MIN) ];
		highestOperating = Shm->Cpu[
					Ruler.Top[ BOOST(TGT) ]
					].Boost[ BOOST(MAX) ];
	    } else {
		COF = Shm->Cpu[cpu].Boost[BOOST(TGT)];
		lowestOperating = Shm->Cpu[cpu].Boost[BOOST(MIN)];
		highestOperating = Shm->Cpu[cpu].Boost[BOOST(MAX)];
	    }
		ComputeRatioShifts(	COF,
					0,	/*	AUTO Frequency	*/
					GetTopOfRuler(),
					&lowestShift,
					&highestShift );

		AppendWindow(
			CreateRatioClock(scan->key,
					COF,
					cpu,
					NC,
					lowestShift,
					highestShift,

					( lowestOperating
					+ highestOperating ) >> 1,

					CUMIN(Shm->Proc.Features.Factory.Ratio,
						GetTopOfRuler() ),

					BOXKEY_RATIO_CLOCK,
					TitleForRatioClock,
					35),
			&winList );
	   } else {
		SetHead(&winList, win);
	   }
	  }
	  break;
	case BOXKEY_RATIO_CLOCK_HWP_TGT:
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod = {.sllong = scan->key};
		struct HWP_STRUCT *pHWP;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	      if (cpu == -1) {
		pHWP = &Shm->Cpu[ Ruler.Top[BOOST(HWP_TGT)] ].PowerThermal.HWP;
		COF = pHWP->Request.Desired_Perf;
	      } else {
		pHWP = &Shm->Cpu[cpu].PowerThermal.HWP;
		COF = pHWP->Request.Desired_Perf;
	      }
		ComputeRatioShifts(	COF,
					0,
					pHWP->Capabilities.Highest,
					&lowestShift,
					&highestShift );

		AppendWindow(
			CreateRatioClock(scan->key,
					COF,
					cpu,
					NC,
					lowestShift,
					highestShift,

					( pHWP->Capabilities.Most_Efficient
					+ pHWP->Capabilities.Guaranteed ) >> 1,

					pHWP->Capabilities.Guaranteed,
					BOXKEY_RATIO_CLOCK,
					TitleForRatioClock,
					38),
			&winList );
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;
	case BOXKEY_RATIO_CLOCK_HWP_MAX:
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod = {.sllong = scan->key};
		struct HWP_STRUCT *pHWP;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	      if (cpu == -1) {
		pHWP = &Shm->Cpu[ Ruler.Top[BOOST(HWP_MAX)] ].PowerThermal.HWP;
		COF = pHWP->Request.Maximum_Perf;
	      } else {
		pHWP = &Shm->Cpu[cpu].PowerThermal.HWP;
		COF = pHWP->Request.Maximum_Perf;
	      }
		ComputeRatioShifts(	COF,
					0,
					pHWP->Capabilities.Highest,
					&lowestShift,
					&highestShift );

		AppendWindow(
			CreateRatioClock(scan->key,
					COF,
					cpu,
					NC,
					lowestShift,
					highestShift,

					( pHWP->Capabilities.Most_Efficient
					+ pHWP->Capabilities.Guaranteed ) >> 1,

					pHWP->Capabilities.Guaranteed,
					BOXKEY_RATIO_CLOCK,
					TitleForRatioClock,
					39),
			&winList );
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;
	case BOXKEY_RATIO_CLOCK_HWP_MIN:
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod = {.sllong = scan->key};
		struct HWP_STRUCT *pHWP;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	      if (cpu == -1) {
		pHWP = &Shm->Cpu[ Ruler.Top[BOOST(HWP_MIN)] ].PowerThermal.HWP;
		COF = pHWP->Request.Minimum_Perf;
	      } else {
		pHWP = &Shm->Cpu[cpu].PowerThermal.HWP;
		COF = pHWP->Request.Minimum_Perf;
	      }
		ComputeRatioShifts(	COF,
					0,
					pHWP->Capabilities.Highest,
					&lowestShift,
					&highestShift );

		AppendWindow(
			CreateRatioClock(scan->key,
					COF,
					cpu,
					NC,
					lowestShift,
					highestShift,

					( pHWP->Capabilities.Most_Efficient
					+ pHWP->Capabilities.Guaranteed ) >> 1,

					pHWP->Capabilities.Guaranteed,
					BOXKEY_RATIO_CLOCK,
					TitleForRatioClock,
					40),
			&winList );
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;
	case BOXKEY_RATIO_CLOCK_MAX:
	  if ((Shm->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		struct FLIP_FLOP *CFlop;
		CLOCK_ARG clockMod  = {.sllong = scan->key};
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF, lowestOperating;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	      if (cpu == -1) {
		COF = Shm->Cpu[ Ruler.Top[BOOST(MAX)] ].Boost[BOOST(MAX)];
		lowestOperating = KMIN( Shm->Cpu[
						Ruler.Top[BOOST(MAX)]
					].Boost[BOOST(MIN)],
					Shm->Proc.Features.Factory.Ratio );
		CFlop = &Shm->Cpu[Ruler.Top[ BOOST(MAX)] ].FlipFlop[
					!Shm->Cpu[Ruler.Top[BOOST(MAX)]
					].Toggle];
	      } else {
		COF = Shm->Cpu[cpu].Boost[BOOST(MAX)];
		lowestOperating = KMIN( Shm->Cpu[cpu].Boost[BOOST(MIN)],
					Shm->Proc.Features.Factory.Ratio );
		CFlop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	      }
		ComputeRatioShifts(COF,
				lowestOperating,
				MAXCLOCK_TO_RATIO(unsigned int,CFlop->Clock.Hz),
				&lowestShift,
				&highestShift);

		AppendWindow(
			CreateRatioClock(scan->key,
				COF,
				cpu,
				NC,
				lowestShift,
				highestShift,

				( lowestOperating
				+ Shm->Proc.Features.Factory.Ratio ) >> 1,

				Shm->Proc.Features.Factory.Ratio
			    + ((MAXCLOCK_TO_RATIO(signed int, CFlop->Clock.Hz)
				- Shm->Proc.Features.Factory.Ratio ) >> 1),

				BOXKEY_RATIO_CLOCK,
				TitleForRatioClock,
				36), &winList);
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;
	case BOXKEY_RATIO_CLOCK_MIN:
	  if ((Shm->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod  = {.sllong = scan->key};
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF, lowestOperating;
		signed int lowestShift, highestShift;

		const signed short cpu = (scan->key & RATIO_MASK) ^ CORE_COUNT;
	      if (cpu == -1) {
		COF = Shm->Cpu[ Ruler.Top[BOOST(MIN)] ].Boost[BOOST(MIN)];
		lowestOperating = KMIN( Shm->Cpu[
						Ruler.Top[BOOST(MIN)]
					].Boost[BOOST(MIN)],
					Shm->Proc.Features.Factory.Ratio );
	      } else {
		COF = Shm->Cpu[cpu].Boost[BOOST(MIN)];
		lowestOperating = KMIN( Shm->Cpu[cpu].Boost[BOOST(MIN)],
					Shm->Proc.Features.Factory.Ratio );
	      }
		ComputeRatioShifts(COF,
				0,
				Shm->Proc.Features.Factory.Ratio,
				&lowestShift,
				&highestShift);

		AppendWindow(
			CreateRatioClock(scan->key,
				COF,
				cpu,
				NC,
				lowestShift,
				highestShift,

				( lowestOperating
				+ Shm->Proc.Features.Factory.Ratio ) >> 1,

				Shm->Proc.Features.Factory.Ratio - 1,

				BOXKEY_RATIO_CLOCK,
				TitleForRatioClock,
				37), &winList);
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;
	default: {
		CLOCK_ARG clockMod = {.sllong = scan->key};
	    if (clockMod.NC & BOXKEY_TURBO_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(Shm->Ring[0])) {
		    RING_WRITE( Shm->Ring[0],
				COREFREQ_IOCTL_TURBO_CLOCK, clockMod.sllong );
		}
	    }
	    else if (clockMod.NC & BOXKEY_RATIO_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(Shm->Ring[0])) {
		    RING_WRITE( Shm->Ring[0],
				COREFREQ_IOCTL_RATIO_CLOCK, clockMod.sllong );
		}
	    }
	    else if (clockMod.NC & BOXKEY_UNCORE_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(Shm->Ring[0])) {
		    RING_WRITE( Shm->Ring[0],
				COREFREQ_IOCTL_UNCORE_CLOCK, clockMod.sllong );
		}
	    } else {
		return (-1);
	    }
	  }
	  break;
	}
      }
    }
	return (0);
}

#ifndef NO_FOOTER
void PrintTaskMemory(Layer *layer, CUINT row,
			int taskCount,
			unsigned long freeRAM,
			unsigned long totalRAM)
{
	snprintf(Buffer, 11+20+20+1,
			"%6d" "%9lu" "%-9lu",
			taskCount,
			freeRAM >> draw.Unit.Memory,
			totalRAM >> draw.Unit.Memory);

	memcpy(&LayerAt(layer, code, (draw.Size.width-35), row), &Buffer[0], 6);
	memcpy(&LayerAt(layer, code, (draw.Size.width-22), row), &Buffer[6], 9);
	memcpy(&LayerAt(layer, code, (draw.Size.width-12), row), &Buffer[15],9);
}
#endif

#ifndef NO_HEADER
void Layout_Header(Layer *layer, CUINT row)
{
	size_t len;
	const CUINT	lProc0 = RSZ(LAYOUT_HEADER_PROC),
			xProc0 = 12,
			lProc1 = RSZ(LAYOUT_HEADER_CPU),
			xProc1 = draw.Size.width - lProc1,
			lArch0 = RSZ(LAYOUT_HEADER_ARCH),
			xArch0 = 12,
			lArch1 = RSZ(LAYOUT_HEADER_CACHE_L1),
			xArch1 = draw.Size.width-lArch1,
			lBClk0 = RSZ(LAYOUT_HEADER_BCLK),
			xBClk0 = 12,
			lArch2 = RSZ(LAYOUT_HEADER_CACHES),
			xArch2 = draw.Size.width-lArch2;

	PrintLCD(layer, 0, row, 4, "::::", _CYAN);

	LayerDeclare(LAYOUT_HEADER_PROC, lProc0, xProc0, row, hProc0);
	LayerDeclare(LAYOUT_HEADER_CPU , lProc1, xProc1, row, hProc1);

	row++;

	LayerDeclare(LAYOUT_HEADER_ARCH, lArch0, xArch0, row, hArch0);
	LayerDeclare(LAYOUT_HEADER_CACHE_L1,lArch1,xArch1,row,hArch1);

	row++;

	LayerDeclare(LAYOUT_HEADER_BCLK, lBClk0, xBClk0, row, hBClk0);
	LayerDeclare(LAYOUT_HEADER_CACHES,lArch2,xArch2, row, hArch2);

	row++;

	snprintf(Buffer, 10+10+1,
			"%3u" "%-3u",
			Shm->Proc.CPU.OnLine,
			Shm->Proc.CPU.Count);

	hProc1.code[2] = Buffer[0];
	hProc1.code[3] = Buffer[1];
	hProc1.code[4] = Buffer[2];
	hProc1.code[6] = Buffer[3];
	hProc1.code[7] = Buffer[4];
	hProc1.code[8] = Buffer[5];

	unsigned int L1I_Size = 0, L1D_Size = 0, L2U_Size = 0, L3U_Size = 0;
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	L1I_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[0].Size / 1024;
	L1D_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[1].Size / 1024;
	L2U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[2].Size / 1024;
	L3U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[3].Size / 1024;
    }
    else if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	 || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	L1I_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[0].Size;
	L1D_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[1].Size;
	L2U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[2].Size;
	L3U_Size=Shm->Cpu[Shm->Proc.Service.Core].Topology.Cache[3].Size;
    }
	snprintf(Buffer, 10+10+1, "%-3u" "%-3u", L1I_Size, L1D_Size);

	hArch1.code[17] = Buffer[0];
	hArch1.code[18] = Buffer[1];
	hArch1.code[19] = Buffer[2];
	hArch1.code[25] = Buffer[3];
	hArch1.code[26] = Buffer[4];
	hArch1.code[27] = Buffer[5];

	snprintf(Buffer, 10+10+1, "%-4u" "%-5u", L2U_Size, L3U_Size);

	hArch2.code[ 3] = Buffer[0];
	hArch2.code[ 4] = Buffer[1];
	hArch2.code[ 5] = Buffer[2];
	hArch2.code[ 6] = Buffer[3];
	hArch2.code[13] = Buffer[4];
	hArch2.code[14] = Buffer[5];
	hArch2.code[15] = Buffer[6];
	hArch2.code[16] = Buffer[7];
	hArch2.code[17] = Buffer[8];

	len = CUMIN(xProc1 - (hProc0.origin.col + hProc0.length),
			(CUINT) strlen(Shm->Proc.Brand));
	/* RED DOT */
	hProc0.code[0] = BITVAL(Shm->Proc.Sync, BURN) ? '.' : 0x20;

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

	len = CUMIN(xArch1 - (hArch0.origin.col + hArch0.length),
			(CUINT) strlen(Shm->Proc.Architecture));
	/* DUMP DOT */
	hArch0.code[0] = DumpStatus() ? '.' : 0x20;

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
#endif /* NO_HEADER */

#ifndef NO_UPPER
void Layout_Ruler_Load(Layer *layer, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_LOAD, draw.Size.width, 0, row, hLoad0);

	struct {
		Coordinate	origin;
		CUINT		length;
		ATTRIBUTE	*attr[2];
		ASCII		*code[2];
	} hLoad1 = {
		.origin = {
			.col = hLoad0.origin.col + 6,
			.row = hLoad0.origin.row
		},
		.length = RSZ(LAYOUT_RULER_REL_LOAD),
		.attr = {
			RSC(LAYOUT_RULER_REL_LOAD).ATTR(),
			RSC(LAYOUT_RULER_ABS_LOAD).ATTR()
		},
		.code = {
			RSC(LAYOUT_RULER_REL_LOAD).CODE(),
			RSC(LAYOUT_RULER_ABS_LOAD).CODE()
		}
	};

	LayerCopyAt(layer, hLoad0.origin.col, hLoad0.origin.row,
			hLoad0.length, hLoad0.attr, hLoad0.code);

	LayerCopyAt(layer, hLoad1.origin.col, hLoad1.origin.row, hLoad1.length,
			hLoad1.attr[draw.Load], hLoad1.code[draw.Load]);

	/* Alternate the color of the frequency ratios			*/
	int idx = Ruler.Count, bright = 1;
    while (idx-- > 0)
    {
		int hPos=Ruler.Uniq[idx] * draw.Area.LoadWidth / Ruler.Maximum;
	if (((hPos+6) < hLoad1.origin.col)
	 || ((hLoad0.origin.col+hPos+3) > (hLoad1.origin.col+hLoad1.length)))
	{
		char tabStop[10+1] = "00";
		snprintf(tabStop, 10+1, "%2u", Ruler.Uniq[idx]);

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
}
#endif /* NO_UPPER */

#ifndef NO_LOWER
CUINT Layout_Monitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_FREQUENCY, RSZ(LAYOUT_MONITOR_FREQUENCY),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return (0);
}

CUINT Layout_Monitor_Instructions(Layer *layer,const unsigned int cpu,CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_INST, RSZ(LAYOUT_MONITOR_INST),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return (0);
}

CUINT Layout_Monitor_Common(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_COMMON, RSZ(LAYOUT_MONITOR_COMMON),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return (0);
}

CUINT Layout_Monitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(layer);
	UNUSED(cpu);
	UNUSED(row);

	return (0);
}

CUINT Layout_Monitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_TASKS, (MAX_WIDTH - LOAD_LEAD + 1),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );

	cTask[cpu].col = LOAD_LEAD + 8;
	cTask[cpu].row = hMon0.origin.row;

	return (0);
}

CUINT Layout_Monitor_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_SLICE, RSZ(LAYOUT_MONITOR_SLICE),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return (0);
}

CUINT Layout_Ruler_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_FREQUENCY, draw.Size.width,
			0, row, hFreq0 );

	LayerCopyAt(	layer, hFreq0.origin.col, hFreq0.origin.row,
			hFreq0.length, hFreq0.attr, hFreq0.code );
	UNUSED(cpu);

	if (!draw.Flag.avgOrPC) {
		LayerDeclare(	LAYOUT_RULER_FREQUENCY_AVG, draw.Size.width,
				0, (row + draw.Area.MaxRows + 1), hAvg0 );

		LayerCopyAt(	layer, hAvg0.origin.col, hAvg0.origin.row,
				hAvg0.length, hAvg0.attr, hAvg0.code );
	} else {
		LayerDeclare(	LAYOUT_RULER_FREQUENCY_PKG, draw.Size.width,
				0, (row + draw.Area.MaxRows + 1), hPkg0) ;

		LayerCopyAt(	layer, hPkg0.origin.col, hPkg0.origin.row,
				hPkg0.length, hPkg0.attr, hPkg0.code );
	}
	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Instructions(Layer *layer, const unsigned int cpu,CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_INST, draw.Size.width,
			0, row, hInstr );

	LayerCopyAt(	layer, hInstr.origin.col, hInstr.origin.row,
			hInstr.length, hInstr.attr, hInstr.code );

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0) );
	UNUSED(cpu);

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Cycles(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_CYCLES, draw.Size.width,
			0, row, hCycles );

	LayerCopyAt(	layer, hCycles.origin.col, hCycles.origin.row,
			hCycles.length, hCycles.attr, hCycles.code );

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0) );
	UNUSED(cpu);

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_CStates(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_CSTATES, draw.Size.width,
			0, row, hCStates );

	LayerCopyAt(	layer, hCStates.origin.col, hCStates.origin.row,
			hCStates.length, hCStates.attr, hCStates.code );

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0) );
	UNUSED(cpu);

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Interrupts(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_INTERRUPTS, draw.Size.width,
			0, row, hIntr0 );

	LayerCopyAt(	layer, hIntr0.origin.col, hIntr0.origin.row,
			hIntr0.length, hIntr0.attr, hIntr0.code );

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0) );
	UNUSED(cpu);

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	ASCII	hCState[9][4] = {
		{'P', 'C', '0', '2'},
		{'P', 'C', '0', '3'},
		{'P', 'C', '0', '4'},
		{'P', 'C', '0', '6'},
		{'P', 'C', '0', '7'},
		{'P', 'C', '0', '8'},
		{'P', 'C', '0', '9'},
		{'P', 'C', '1', '0'},
		{'M', 'C', '0', '6'}
	};

	LayerFillAt(	layer, 0, row, draw.Size.width,
			RSC(LAYOUT_RULER_PACKAGE).CODE(),
			RSC(LAYOUT_RULER_PACKAGE).ATTR()[0]);

	unsigned int idx;
	UNUSED(cpu);

	for (idx = 0; idx < 9; idx++)
	{
		LayerCopyAt(	layer, 0, (row + idx + 1),
				draw.Size.width,
				RSC(LAYOUT_PACKAGE_PC).ATTR(),
				RSC(LAYOUT_PACKAGE_PC).CODE());

		LayerAt(layer, code, 0, (row + idx + 1)) = hCState[idx][0];
		LayerAt(layer, code, 1, (row + idx + 1)) = hCState[idx][1];
		LayerAt(layer, code, 2, (row + idx + 1)) = hCState[idx][2];
		LayerAt(layer, code, 3, (row + idx + 1)) = hCState[idx][3];
	}

	LayerDeclare(	LAYOUT_PACKAGE_UNCORE, draw.Size.width,
			0, (row + 10), hUncore);

	LayerCopyAt(	layer, hUncore.origin.col, hUncore.origin.row,
			hUncore.length, hUncore.attr, hUncore.code);

	LayerFillAt(	layer, 0, (row + 11),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0) );

	row += 2 + 10;
	return (row);
}

CUINT Layout_Ruler_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_TASKS, draw.Size.width, 0, row, hTask0);

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

	struct {
		Coordinate	origin;
		CUINT		length;
		ATTRIBUTE	*attr;
		ASCII		*code;
	} hTask1 = {
		.origin = {
			.col = 23,
			.row = row
		},
		.length = 21,
		.attr = hSort[Shm->SysGate.sortByField].attr,
		.code = hSort[Shm->SysGate.sortByField].code
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

	struct {
		Coordinate	origin;
		CUINT		length;
		ATTRIBUTE	*attr;
		ASCII		*code;
	} hTask2 = {
		.origin = {
			.col = (draw.Size.width - 18),
			.row = (row + draw.Area.MaxRows + 1)
		},
		.length = 15,
		.attr = hReverse[Shm->SysGate.reverseOrder].attr,
		.code = hReverse[Shm->SysGate.reverseOrder].code
	};

	LayerDeclare(LAYOUT_TASKS_VALUE_SWITCH, RSZ(LAYOUT_TASKS_VALUE_SWITCH),
			(draw.Size.width - 34),
			(row + draw.Area.MaxRows + 1),
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
	UNUSED(cpu);

	memcpy(&hTask3.attr[8], hTaskVal[draw.Flag.taskVal].attr, 3);
	memcpy(&hTask3.code[8], hTaskVal[draw.Flag.taskVal].code, 3);

	LayerDeclare(	LAYOUT_TASKS_TRACKING, RSZ(LAYOUT_TASKS_TRACKING),
			53, row, hTrack0 );

	LayerCopyAt(	layer, hTask0.origin.col, hTask0.origin.row,
			hTask0.length, hTask0.attr, hTask0.code );

	LayerCopyAt(	layer, hTask1.origin.col, hTask1.origin.row,
			hTask1.length, hTask1.attr, hTask1.code );

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0) );

	LayerCopyAt(	layer, hTask2.origin.col, hTask2.origin.row,
			hTask2.length, hTask2.attr, hTask2.code );

	LayerCopyAt(	layer, hTask3.origin.col, hTask3.origin.row,
			hTask3.length, hTask3.attr, hTask3.code );

	LayerCopyAt(	layer, hTrack0.origin.col, hTrack0.origin.row,
			hTrack0.length, hTrack0.attr, hTrack0.code );

	if (Shm->SysGate.trackTask)
	{
		snprintf(Buffer, 11+1, "%7d", Shm->SysGate.trackTask);
		LayerFillAt(	layer,
				(hTrack0.origin.col + 15), hTrack0.origin.row,
				7, Buffer, MakeAttr(CYAN, 0, BLACK, 0) );
	}
	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Sensors(Layer *layer, const unsigned int cpu, CUINT row)
{
	CUINT oRow;

	LayerDeclare(LAYOUT_RULER_SENSORS, draw.Size.width, 0, row, hSensors);

	UNUSED(cpu);

	LayerCopyAt(	layer, hSensors.origin.col, hSensors.origin.row,
			hSensors.length, hSensors.attr, hSensors.code );

	LayerAt(layer,code,32,hSensors.origin.row)=Setting.fahrCels ? 'F':'C';

  if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
    switch (Shm->Proc.ArchID) {
    case Kabylake_UY:
    case Kabylake:
    case Cannonlake:
    case Icelake_UY:
    case Icelake:
    case Cometlake_UY:
    case Cometlake:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_PLATFORM, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hPwrPfm);

	LayerCopyAt(	layer, hPwrPfm.origin.col, hPwrPfm.origin.row,
			hPwrPfm.length, hPwrPfm.attr, hPwrPfm.code );

	oRow = hPwrPfm.origin.row;
	}
	break;
    default:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_UNCORE, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hPwrUncore );

	LayerCopyAt(	layer, hPwrUncore.origin.col, hPwrUncore.origin.row,
			hPwrUncore.length, hPwrUncore.attr, hPwrUncore.code );

	oRow = hPwrUncore.origin.row;
	}
	break;
    }
  } else {
	LayerDeclare(	LAYOUT_RULER_PWR_SOC, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hPwrSoC );

	LayerCopyAt(	layer, hPwrSoC.origin.col, hPwrSoC.origin.row,
			hPwrSoC.length, hPwrSoC.attr, hPwrSoC.code );

	oRow = hPwrSoC.origin.row;
  }
	LayerAt(layer,code, 14, oRow) = \
	LayerAt(layer,code, 35, oRow) = \
	LayerAt(layer,code, 57, oRow) = \
	LayerAt(layer,code, 77, oRow) = Setting.jouleWatt ? 'W':'J';

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_VOLTAGE, draw.Size.width, 0, row, hVolt );

	UNUSED(cpu);

	LayerCopyAt(	layer, hVolt.origin.col, hVolt.origin.row,
			hVolt.length, hVolt.attr, hVolt.code );

	LayerDeclare(	LAYOUT_RULER_VPKG_SOC, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hVPkg );

	LayerCopyAt(	layer, hVPkg.origin.col, hVPkg.origin.row,
			hVPkg.length, hVPkg.attr, hVPkg.code );

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Energy(Layer *layer, const unsigned int cpu, CUINT row)
{
	CUINT oRow;
	UNUSED(cpu);

    if (Setting.jouleWatt)
    {
	LayerDeclare(LAYOUT_RULER_POWER, draw.Size.width, 0, row,  hPwr0);

	LayerCopyAt(	layer,  hPwr0.origin.col,  hPwr0.origin.row,
			 hPwr0.length,  hPwr0.attr,  hPwr0.code );
    }
   else
    {
	LayerDeclare(LAYOUT_RULER_ENERGY, draw.Size.width, 0, row,  hPwr0);

	LayerCopyAt(	layer,  hPwr0.origin.col,  hPwr0.origin.row,
			 hPwr0.length,  hPwr0.attr,  hPwr0.code );
    }
	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine, MakeAttr(WHITE, 0, BLACK, 0) );

  if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
    switch (Shm->Proc.ArchID) {
    case Kabylake_UY:
    case Kabylake:
    case Cannonlake:
    case Icelake_UY:
    case Icelake:
    case Cometlake_UY:
    case Cometlake:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_PLATFORM, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hPwrPfm);

	LayerCopyAt(	layer, hPwrPfm.origin.col, hPwrPfm.origin.row,
			hPwrPfm.length, hPwrPfm.attr, hPwrPfm.code );

	oRow = hPwrPfm.origin.row;
	}
	break;
    default:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_UNCORE, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hPwrUncore);

	LayerCopyAt(	layer, hPwrUncore.origin.col, hPwrUncore.origin.row,
			hPwrUncore.length, hPwrUncore.attr, hPwrUncore.code );

	oRow = hPwrUncore.origin.row;
	}
	break;
    }
  } else {
	LayerDeclare(	LAYOUT_RULER_PWR_SOC, draw.Size.width,
			0, (row + draw.Area.MaxRows + 1), hPwrSoC);

	LayerCopyAt(	layer, hPwrSoC.origin.col, hPwrSoC.origin.row,
			hPwrSoC.length, hPwrSoC.attr, hPwrSoC.code );

	oRow = hPwrSoC.origin.row;
  }
	LayerAt(layer,code, 14, oRow) = \
	LayerAt(layer,code, 35, oRow) = \
	LayerAt(layer,code, 57, oRow) = \
	LayerAt(layer,code, 77, oRow) = Setting.jouleWatt ? 'W':'J';

	row += draw.Area.MaxRows + 2;
	return (row);
}

CUINT Layout_Ruler_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_SLICE, draw.Size.width, 0, row, hSlice0);

	UNUSED(cpu);

	LayerCopyAt(	layer, hSlice0.origin.col, hSlice0.origin.row,
			hSlice0.length, hSlice0.attr, hSlice0.code );

	LayerFillAt(	layer, 0, (row + draw.Area.MaxRows + 1),
			draw.Size.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0) );

	row += draw.Area.MaxRows + 2;
	return (row);
}
#endif /* NO_LOWER */

#ifndef NO_FOOTER
void Layout_Footer(Layer *layer, CUINT row)
{
	ssize_t len;
	CUINT col = 0;

	LayerDeclare(	LAYOUT_FOOTER_TECH_X86, RSZ(LAYOUT_FOOTER_TECH_X86),
			0, row, hTech0 );

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
			hTech0.length, hTech0.attr, hTech0.code );

    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	LayerDeclare(	LAYOUT_FOOTER_TECH_INTEL, RSZ(LAYOUT_FOOTER_TECH_INTEL),
			hTech0.length, hTech0.origin.row,
			hTech1 );

	hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =		\
					Pwr[Shm->Proc.Features.HyperThreading];

	const ATTRIBUTE TM[] = {
		MakeAttr(BLACK, 0, BLACK, 1),
		MakeAttr(BLUE,  0, BLACK, 1),
		MakeAttr(WHITE, 0, BLACK, 1),
		MakeAttr(GREEN, 0, BLACK, 1)
	};

	hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] =		\
	hTech1.attr[7] = Pwr[Shm->Proc.Technology.EIST];

	hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] =		\
				Pwr[Shm->Proc.Features.Power.EAX.TurboIDA];

	hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =		\
	hTech1.attr[16] = hTech1.attr[17] = Pwr[Shm->Proc.Technology.Turbo];

	hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] =		\
						Pwr[Shm->Proc.Technology.C1E];

	snprintf(Buffer, 2+10+1, "PM%1u", Shm->Proc.PM_version);

	hTech1.code[23] = Buffer[0];
	hTech1.code[24] = Buffer[1];
	hTech1.code[25] = Buffer[2];

	hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =		\
						Pwr[(Shm->Proc.PM_version > 0)];

	hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] =		\
						Pwr[Shm->Proc.Technology.C3A];

	hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] =		\
						Pwr[Shm->Proc.Technology.C1A];

	hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] =		\
						Pwr[Shm->Proc.Technology.C3U];

	hTech1.attr[39] = hTech1.attr[40] = hTech1.attr[41] =		\
						Pwr[Shm->Proc.Technology.C1U];

	hTech1.attr[43] = hTech1.attr[44] = 				\
			TM[Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM1
			  |Shm->Cpu[Shm->Proc.Service.Core].PowerThermal.TM2];

	LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
			hTech1.origin.row,
			(draw.Size.width - hTech0.length - hTech1.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
    }
    else
    {
      if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
      || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
      {
	LayerDeclare(	LAYOUT_FOOTER_TECH_AMD, RSZ(LAYOUT_FOOTER_TECH_AMD),
			hTech0.length, hTech0.origin.row,
			hTech1 );

	hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =		\
					Pwr[Shm->Proc.Features.HyperThreading];

	hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] =		\
					Pwr[(Shm->Proc.PowerNow == 0b11)];

	hTech1.attr[8] = hTech1.attr[9] = hTech1.attr[10] =		\
				Pwr[Shm->Proc.Features.AdvPower.EDX.HwPstate];

	hTech1.attr[12] = hTech1.attr[13] = hTech1.attr[14] =		\
	hTech1.attr[15] = hTech1.attr[16] = Pwr[Shm->Proc.Technology.Turbo];

	hTech1.attr[18] = hTech1.attr[19] = hTech1.attr[20] =		\
						Pwr[Shm->Proc.Technology.C1E];

	hTech1.attr[22] = hTech1.attr[23] = hTech1.attr[24] =		\
						Pwr[Shm->Proc.Technology.CC6];

	hTech1.attr[26] = hTech1.attr[27] = hTech1.attr[28] =		\
						Pwr[Shm->Proc.Technology.PC6];

	hTech1.attr[30] = hTech1.attr[31] = hTech1.attr[32] =		\
	Pwr[(Shm->Cpu[Shm->Proc.Service.Core].Query.CStateBaseAddr != 0)];

	hTech1.attr[34] = hTech1.attr[35] = hTech1.attr[36] =		\
				Pwr[(Shm->Proc.Features.AdvPower.EDX.TS != 0)];

	hTech1.attr[38] = hTech1.attr[39] = hTech1.attr[40] =		\
				Pwr[(Shm->Proc.Features.AdvPower.EDX.TTP != 0)];

	LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
			hTech1.origin.row,
			(draw.Size.width - hTech0.length - hTech1.length),
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));
      }
    }
	LayerAt(layer, code, 14+64, row) = Setting.fahrCels ? 'F' : 'C';

	row++;

	len = snprintf( Buffer, MAX_UTS_LEN, "%s",
			BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1) ?
			Shm->SysGate.sysname : "SysGate" );

	LayerFillAt(	layer, col, row,
			len, Buffer,
			MakeAttr(CYAN, 0, BLACK, 0));
	col += len;

	LayerAt(layer, attr, col, row) = MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, col, row) = 0x20;

	col++;

	LayerAt(layer, attr, col, row) = MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, col, row) = '[';

	col++;

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		len = snprintf( Buffer, 2+5+5+5+1, "%hu.%hu.%hu",
				Shm->SysGate.kernel.version,
				Shm->SysGate.kernel.major,
				Shm->SysGate.kernel.minor );

		LayerFillAt(	layer, col, row,
				len, Buffer,
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

	LayerDeclare(	LAYOUT_FOOTER_SYSTEM, RSZ(LAYOUT_FOOTER_SYSTEM),
			(draw.Size.width-RSZ(LAYOUT_FOOTER_SYSTEM)),row, hSys1);

	LayerCopyAt(	layer, hSys1.origin.col, hSys1.origin.row,
			hSys1.length, hSys1.attr, hSys1.code );
	/* Center the DMI string					*/
	if ((len = strlen(Shm->SMB.String[draw.SmbIndex])) > 0) {
		CSINT	can = CUMIN(hSys1.origin.col - col - 1, len),
			ctr = ((hSys1.origin.col + col) - can) / 2;
		LayerFillAt(	layer, ctr, hSys1.origin.row,
				can, ScrambleSMBIOS(draw.SmbIndex, 4, '-'),
				MakeAttr(BLUE, 0, BLACK, 1) );
	}
	/* Reset Tasks count & Memory usage				*/
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		PrintTaskMemory(layer, row,
				Shm->SysGate.taskCount,
				Shm->SysGate.memInfo.freeram,
				Shm->SysGate.memInfo.totalram);
	}
	/* Print the memory unit character				*/
	LayerAt(layer, code,
		hSys1.origin.col + (RSZ(LAYOUT_FOOTER_SYSTEM) - 3),
		hSys1.origin.row) =	draw.Unit.Memory ==  0 ? 'K'
				:	draw.Unit.Memory == 10 ? 'M'
				:	draw.Unit.Memory == 20 ? 'G' : 0x20;
}
#endif /* NO_FOOTER */

void Layout_CPU_To_String(const unsigned int cpu)
{
	snprintf(Buffer, 10+1, "%03u", cpu);
}

void Layout_CPU_To_View(Layer *layer, const CUINT col, const CUINT row)
{
	LayerAt(layer,code, col + 0, row) = Buffer[0];
	LayerAt(layer,code, col + 1, row) = Buffer[1];
	LayerAt(layer,code, col + 2, row) = Buffer[2];
}

void Layout_BCLK_To_View(Layer *layer, const CUINT col, const CUINT row)
{
	LayerAt(layer, attr, col + 3, row) = MakeAttr(YELLOW, 0, BLACK, 1);
	LayerAt(layer, code, col + 3, row) = 0x20;
}

#ifndef NO_UPPER
CUINT Draw_Frequency_Load(	Layer *layer, CUINT row,
				const unsigned int cpu, double ratio )
{	/*	Upper view area						*/
	const CUINT	bar0 = ((ratio > Ruler.Maximum ? Ruler.Maximum : ratio)
				* draw.Area.LoadWidth) / Ruler.Maximum,
			bar1 = draw.Area.LoadWidth - bar0;

	const ATTRIBUTE attr = MakeAttr( (ratio > Ruler.Median ? RED
					: ratio > Ruler.Minimum ? YELLOW:GREEN),
					0, BLACK, 1 );
	UNUSED(cpu);

	LayerFillAt(layer, LOAD_LEAD, row, bar0, hBar, attr);
	/*TODO( Clear garbage with transparency padding )		*/
	ClearGarbage(	layer, attr, (bar0 + LOAD_LEAD), row, bar1,
			MakeAttr(BLACK,0,BLACK,1).value );

	ClearGarbage(layer, code, (bar0 + LOAD_LEAD), row, bar1, 0x0);

	return (0);
}

CUINT Draw_Relative_Load(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	/*		Draw the relative Core frequency ratio		*/
	return (Draw_Frequency_Load(layer,row, cpu,CFlop->Relative.Ratio));
}

CUINT Draw_Absolute_Load(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	/*		Draw the absolute Core frequency ratio		*/
	return (Draw_Frequency_Load(layer,row, cpu,CFlop->Absolute.Ratio.Perf));
}
#endif /* NO_UPPER */

#ifndef NO_LOWER
size_t Draw_Relative_Freq_Spaces(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);
	UNUSED(cpu);

	return (snprintf(Buffer, 17+8+6+7+7+7+7+7+7+11+1,
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
		11, hSpace));
}

size_t Draw_Absolute_Freq_Spaces(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);
	UNUSED(cpu);

	return (snprintf(Buffer, 17+8+10+7+7+7+7+7+7+11+1,
		"%7.2f" " (" "%5u" ") "			\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
		"%.*s",
		CFlop->Absolute.Freq,
		CFlop->Absolute.Ratio.Perf,
		100.f * CFlop->State.Turbo,
		100.f * CFlop->State.C0,
		100.f * CFlop->State.C1,
		100.f * CFlop->State.C3,
		100.f * CFlop->State.C6,
		100.f * CFlop->State.C7,
		11, hSpace));
}

size_t (*Draw_Freq_Spaces_Matrix[2])(	struct FLIP_FLOP*,
					CPU_STRUCT*,
					const unsigned int ) = \
{
	Draw_Relative_Freq_Spaces,
	Draw_Absolute_Freq_Spaces
};

size_t Draw_Freq_Spaces(struct FLIP_FLOP *CFlop,
			CPU_STRUCT *Cpu,
			const unsigned int cpu)
{
	return (Draw_Freq_Spaces_Matrix[draw.Load](CFlop, Cpu, cpu));
}

size_t Draw_Relative_Freq_Celsius(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	return (snprintf(Buffer, 19+8+6+7+7+7+7+7+7+10+10+10+1,
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
		Cpu->PowerThermal.Limit[SENSOR_LOWEST],
		CFlop->Thermal.Temp,
		Cpu->PowerThermal.Limit[SENSOR_HIGHEST]));
}

size_t Draw_Absolute_Freq_Celsius(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	return (snprintf(Buffer, 19+8+10+7+7+7+7+7+7+10+10+10+1,
		"%7.2f" " (" "%5u" ") "			\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
		"%-3u" "/" "%3u" "/" "%3u",
		CFlop->Absolute.Freq,
		CFlop->Absolute.Ratio.Perf,
		100.f * CFlop->State.Turbo,
		100.f * CFlop->State.C0,
		100.f * CFlop->State.C1,
		100.f * CFlop->State.C3,
		100.f * CFlop->State.C6,
		100.f * CFlop->State.C7,
		Cpu->PowerThermal.Limit[SENSOR_LOWEST],
		CFlop->Thermal.Temp,
		Cpu->PowerThermal.Limit[SENSOR_HIGHEST]));
}

size_t (*Draw_Freq_Celsius_Matrix[2])(	struct FLIP_FLOP*,
					CPU_STRUCT*,
					const unsigned int ) = \
{
	Draw_Relative_Freq_Celsius,
	Draw_Absolute_Freq_Celsius
};

size_t Draw_Freq_Celsius(	struct FLIP_FLOP *CFlop,
				CPU_STRUCT *Cpu,
				const unsigned int cpu )
{
	return (Draw_Freq_Celsius_Matrix[draw.Load](CFlop, Cpu, cpu));
};

size_t Draw_Relative_Freq_Fahrenheit(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	return (snprintf(Buffer, 19+8+6+7+7+7+7+7+7+10+10+10+1,
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
		Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_LOWEST]),
		Cels2Fahr(CFlop->Thermal.Temp),
		Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_HIGHEST])));
}

size_t Draw_Absolute_Freq_Fahrenheit(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	return (snprintf(Buffer, 19+8+10+7+7+7+7+7+7+10+10+10+1,
		"%7.2f" " (" "%5u" ") "			\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
		"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  "	\
		"%-3u" "/" "%3u" "/" "%3u",
		CFlop->Absolute.Freq,
		CFlop->Absolute.Ratio.Perf,
		100.f * CFlop->State.Turbo,
		100.f * CFlop->State.C0,
		100.f * CFlop->State.C1,
		100.f * CFlop->State.C3,
		100.f * CFlop->State.C6,
		100.f * CFlop->State.C7,
		Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_LOWEST]),
		Cels2Fahr(CFlop->Thermal.Temp),
		Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_HIGHEST])));
}

size_t (*Draw_Freq_Fahrenheit_Matrix[2])(	struct FLIP_FLOP*,
						CPU_STRUCT*,
						const unsigned int ) = \
{
	Draw_Relative_Freq_Fahrenheit,
	Draw_Absolute_Freq_Fahrenheit
};

size_t Draw_Freq_Fahrenheit(	struct FLIP_FLOP *CFlop,
				CPU_STRUCT *Cpu,
				const unsigned int cpu )
{
	return (Draw_Freq_Fahrenheit_Matrix[draw.Load](CFlop, Cpu, cpu));
};

size_t Draw_Freq_Celsius_PerCore(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Freq_Celsius(CFlop, &Shm->Cpu[cpu], cpu));
	} else {
		return (Draw_Freq_Spaces(CFlop, NULL, cpu));
	}
}

size_t Draw_Freq_Fahrenheit_PerCore(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Freq_Fahrenheit(CFlop, &Shm->Cpu[cpu], cpu));
	} else {
		return (Draw_Freq_Spaces(CFlop, NULL, cpu));
	}
}

size_t Draw_Freq_Celsius_PerPkg(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Freq_Celsius(CFlop, &Shm->Cpu[cpu], cpu));
	} else {
		return (Draw_Freq_Spaces(CFlop, NULL, cpu));
	}
}

size_t Draw_Freq_Fahrenheit_PerPkg(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Freq_Fahrenheit(CFlop, &Shm->Cpu[cpu], cpu));
	} else {
		return (Draw_Freq_Spaces(CFlop, NULL, cpu));
	}
}

size_t (*Draw_Freq_Temp_Matrix[4][2])(	struct FLIP_FLOP*,
					CPU_STRUCT*,
					const unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = {
		[0] = Draw_Freq_Spaces,
		[1] = Draw_Freq_Spaces
	},
	[FORMULA_SCOPE_SMT ] = {
		[0] = Draw_Freq_Celsius,
		[1] = Draw_Freq_Fahrenheit
	},
	[FORMULA_SCOPE_CORE] = {
		[0] = Draw_Freq_Celsius_PerCore,
		[1] = Draw_Freq_Fahrenheit_PerCore
	},
	[FORMULA_SCOPE_PKG ] = {
		[0] = Draw_Freq_Celsius_PerPkg,
		[1] = Draw_Freq_Fahrenheit_PerPkg
	}
};

CUINT Draw_Monitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	const enum FORMULA_SCOPE
		thermalScope = SCOPE_OF_FORMULA(Shm->Proc.thermalFormula);

	size_t len = Draw_Freq_Temp_Matrix[thermalScope][Setting.fahrCels](
				CFlop, &Shm->Cpu[cpu], cpu
	);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	ATTRIBUTE warning = {.fg = WHITE, .un = 0, .bg = BLACK, .bf = 1};

  if (CFlop->Thermal.Temp <= Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST]) {
		warning = MakeAttr(BLUE, 0, BLACK, 1);
  } else {
    if (CFlop->Thermal.Temp >= Shm->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST])
		warning = MakeAttr(YELLOW, 0, BLACK, 0);
  }
	if ( CFlop->Thermal.Events & (	EVENT_THERM_SENSOR
				     |	EVENT_THERM_PROCHOT
				     |	EVENT_THERM_CRIT
				     |	EVENT_THERM_THOLD ) )
	{
		warning = MakeAttr(RED, 0, BLACK, 1);
	}
	LayerAt(layer, attr, (LOAD_LEAD + 69), row) =			\
		LayerAt(layer, attr, (LOAD_LEAD + 70), row) =		\
			LayerAt(layer, attr, (LOAD_LEAD + 71), row) = warning;

	return (0);
}

CUINT Draw_Monitor_Instructions(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = snprintf( Buffer, 6+18+18+18+20+1,
			"%17.6f" "/s"					\
			"%17.6f" "/c"					\
			"%17.6f" "/i"					\
			"%18llu",
			CFlop->State.IPS,
			CFlop->State.IPC,
			CFlop->State.CPI,
			CFlop->Delta.INST );
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

CUINT Draw_Monitor_Cycles(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = snprintf( Buffer, 20+20+20+20+1,
			"%18llu%18llu%18llu%18llu",
			CFlop->Delta.C0.UCC,
			CFlop->Delta.C0.URC,
			CFlop->Delta.C1,
			CFlop->Delta.TSC );
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

CUINT Draw_Monitor_CStates(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = snprintf( Buffer, 20+20+20+20+1,
			"%18llu%18llu%18llu%18llu",
			CFlop->Delta.C1,
			CFlop->Delta.C3,
			CFlop->Delta.C6,
			CFlop->Delta.C7 );
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

CUINT Draw_Monitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(layer);
	UNUSED(cpu);
	UNUSED(row);

	return (0);
}

size_t Draw_Monitor_Tasks_Relative_Freq(struct FLIP_FLOP *CFlop)
{
	return (snprintf(Buffer, 8+1, "%7.2f", CFlop->Relative.Freq));
}

size_t Draw_Monitor_Tasks_Absolute_Freq(struct FLIP_FLOP *CFlop)
{
	return (snprintf(Buffer, 8+1, "%7.2f", CFlop->Absolute.Freq));
}

size_t (*Draw_Monitor_Tasks_Matrix[2])(struct FLIP_FLOP *CFlop) = \
{
	Draw_Monitor_Tasks_Relative_Freq,
	Draw_Monitor_Tasks_Absolute_Freq
};

CUINT Draw_Monitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	size_t len = Draw_Monitor_Tasks_Matrix[draw.Load](CFlop);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);
	cTask[cpu].col = LOAD_LEAD + 8;

	return (0);
}

CUINT Draw_Monitor_Interrupts(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	size_t len;

	len = snprintf(Buffer, 10+1, "%10u", CFlop->Counter.SMI);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	if (BITVAL(Shm->Registration.NMI, BIT_NMI_LOCAL) == 1) {
		len = snprintf(Buffer, 10+1, "%10u", CFlop->Counter.NMI.LOCAL);
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 24),row), Buffer, len);
	} else {
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 24),row), hSpace, 10);
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_UNKNOWN) == 1) {
		len = snprintf(Buffer, 10+1,"%10u",CFlop->Counter.NMI.UNKNOWN);
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 34),row), Buffer, len);
	} else {
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 34),row), hSpace, 10);
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_SERR) == 1) {
		len = snprintf(Buffer, 10+1,"%10u",CFlop->Counter.NMI.PCISERR);
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 44),row), Buffer, len);
	} else {
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 44),row), hSpace, 10);
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_IO_CHECK) == 1) {
		len = snprintf(Buffer, 10+1,"%10u",CFlop->Counter.NMI.IOCHECK);
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 54),row), Buffer, len);
	} else {
		memcpy(&LayerAt(layer,code,(LOAD_LEAD + 54),row), hSpace, 10);
	}
	return (0);
}

size_t Draw_Sensors_V0_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			69, hSpace) );
}

size_t Draw_Sensors_V0_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f%.*s"					\
			"%8.4f\x20\x20\x20\x20\x20\x20" 	 	\
			"%8.4f%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			26, hSpace,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace) );
}

size_t Draw_Sensors_V0_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V0_T0_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V0_T0_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f%.*s"					\
			"%3u%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			17, hSpace,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			49, hSpace) );
}

size_t Draw_Sensors_V0_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f%.*s"		\
			"%3u\x20\x20\x20\x20\x20\x20"			\
			"%8.4f\x20\x20\x20\x20\x20\x20" 	 	\
			"%8.4f%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			17, hSpace,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace) );
}

size_t Draw_Sensors_V0_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V0_T1_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V0_T1_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V0_T1_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V0_T1_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V0_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V0_T1_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			56, hSpace) );
}

size_t Draw_Sensors_V1_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f\x20\x20\x20\x20" 			\
			"\x20\x20\x20\x20\x20\x20\x20\x20\x20"		\
			"%8.4f\x20\x20\x20\x20\x20\x20" 		\
			"%8.4f%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace) );
}

size_t Draw_Sensors_V1_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T0_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T0_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f\x20\x20\x20\x20" 			\
			"%3u%.*s",					\
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			49, hSpace) );
}

size_t Draw_Sensors_V1_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f\x20\x20\x20\x20" 			\
			"%3u\x20\x20\x20\x20\x20\x20"			\
			"%8.4f\x20\x20\x20\x20\x20\x20" 	 	\
			"%8.4f%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace) );
}

size_t Draw_Sensors_V1_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T1_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T1_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V1_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V1_T0_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T0_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T0_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T0_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T0_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T1_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T2_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T2_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T2_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T2_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T3_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T3_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T3_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V2_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Sensors_V1_T3_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T0_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T0_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T0_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T0_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T0_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T1_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T1_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T2_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T2_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T2_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T2_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T2_P3(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T3_P0(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P0(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T3_P1(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P1(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T3_P2(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P2(layer, cpu, row));
	}
}

size_t Draw_Sensors_V3_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Sensors_V1_T3_P3(layer, cpu, row));
	} else {
		return (Draw_Sensors_V0_T3_P3(layer, cpu, row));
	}
}

size_t (*Draw_Sensors_VTP_Matrix[4][4][4])(Layer*, const unsigned int, CUINT)=\
{
[FORMULA_SCOPE_NONE] = {
    [FORMULA_SCOPE_NONE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V0_T0_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V0_T0_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V0_T0_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V0_T0_P3
    },
    [FORMULA_SCOPE_SMT ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V0_T1_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V0_T1_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V0_T1_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V0_T1_P3
    },
    [FORMULA_SCOPE_CORE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V0_T2_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V0_T2_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V0_T2_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V0_T2_P3
    },
    [FORMULA_SCOPE_PKG ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V0_T3_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V0_T3_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V0_T3_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V0_T3_P3
    }
  },
[FORMULA_SCOPE_SMT ] = {
    [FORMULA_SCOPE_NONE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V1_T0_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V1_T0_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V1_T0_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V1_T0_P3
    },
    [FORMULA_SCOPE_SMT ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V1_T1_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V1_T1_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V1_T1_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V1_T1_P3
    },
    [FORMULA_SCOPE_CORE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V1_T2_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V1_T2_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V1_T2_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V1_T2_P3
    },
    [FORMULA_SCOPE_PKG ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V1_T3_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V1_T3_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V1_T3_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V1_T3_P3
    }
  },
[FORMULA_SCOPE_CORE] = {
    [FORMULA_SCOPE_NONE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V2_T0_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V2_T0_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V2_T0_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V2_T0_P3
    },
    [FORMULA_SCOPE_SMT ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V2_T1_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V2_T1_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V2_T1_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V2_T1_P3
    },
    [FORMULA_SCOPE_CORE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V2_T2_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V2_T2_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V2_T2_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V2_T2_P3
    },
    [FORMULA_SCOPE_PKG ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V2_T3_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V2_T3_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V2_T3_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V2_T3_P3
    }
  },
[FORMULA_SCOPE_PKG ] = {
    [FORMULA_SCOPE_NONE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V3_T0_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V3_T0_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V3_T0_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V3_T0_P3
    },
    [FORMULA_SCOPE_SMT ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V3_T1_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V3_T1_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V3_T1_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V3_T1_P3
    },
    [FORMULA_SCOPE_CORE] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V3_T2_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V3_T2_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V3_T2_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V3_T2_P3
    },
    [FORMULA_SCOPE_PKG ] = {
	[FORMULA_SCOPE_NONE]	= Draw_Sensors_V3_T3_P0,
	[FORMULA_SCOPE_SMT ]	= Draw_Sensors_V3_T3_P1,
	[FORMULA_SCOPE_CORE]	= Draw_Sensors_V3_T3_P2,
	[FORMULA_SCOPE_PKG ]	= Draw_Sensors_V3_T3_P3
    }
  }
};

CUINT Draw_Monitor_Sensors(Layer *layer, const unsigned int cpu, CUINT row)
{
	size_t len;
	const enum FORMULA_SCOPE
		voltageScope	= SCOPE_OF_FORMULA(Shm->Proc.voltageFormula),
		thermalScope	= SCOPE_OF_FORMULA(Shm->Proc.thermalFormula),
		powerScope	= SCOPE_OF_FORMULA(Shm->Proc.powerFormula);

	len = Draw_Sensors_VTP_Matrix	[voltageScope]
					[thermalScope]
					[powerScope]	(layer, cpu, row);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

#define Draw_Voltage_None	Draw_Sensors_V0_T0_P0

size_t Draw_Voltage_SMT(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20%7d"					\
			"\x20\x20\x20\x20\x20%5.4f"			\
			"\x20\x20\x20%5.4f"				\
			"\x20\x20\x20%5.4f"				\
			"%.*s",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.VID,
			Shm->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_LOWEST],
			CFlop->Voltage.Vcore,
			Shm->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_HIGHEST],
			32, hSpace) );
}

size_t Draw_Voltage_Core(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Voltage_SMT(layer, cpu, row));
	} else {
		return (Draw_Voltage_None(layer, cpu, row));
	}
}

size_t Draw_Voltage_Pkg(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Voltage_SMT(layer, cpu, row));
	} else {
		return (Draw_Voltage_None(layer, cpu, row));
	}
}

size_t (*Draw_Voltage_Matrix[4])(Layer*, const unsigned int, CUINT) = \
{
	[FORMULA_SCOPE_NONE] = Draw_Voltage_None,
	[FORMULA_SCOPE_SMT ] = Draw_Voltage_SMT,
	[FORMULA_SCOPE_CORE] = Draw_Voltage_Core,
	[FORMULA_SCOPE_PKG ] = Draw_Voltage_Pkg
};

CUINT Draw_Monitor_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	size_t len;
	const enum FORMULA_SCOPE
		voltageScope = SCOPE_OF_FORMULA(Shm->Proc.voltageFormula);

	len = Draw_Voltage_Matrix[voltageScope](layer, cpu, row);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

#define Draw_Energy_None	Draw_Sensors_V0_T0_P0

size_t Draw_Energy_Joule(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20\x20\x20%018llu\x20\x20\x20\x20"	\
			"%10.6f\x20\x20%10.6f\x20\x20%10.6f",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Delta.Power.ACCU,
			Shm->Cpu[cpu].Sensors.Energy.Limit[SENSOR_LOWEST],
			CFlop->State.Energy,
			Shm->Cpu[cpu].Sensors.Energy.Limit[SENSOR_HIGHEST]) );
}

size_t Draw_Power_Watt(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	return (snprintf(Buffer, 80+1,
			"%7.2f\x20\x20\x20%018llu\x20\x20\x20\x20"	\
			"%10.6f\x20\x20%10.6f\x20\x20%10.6f",
			draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Delta.Power.ACCU,
			Shm->Cpu[cpu].Sensors.Power.Limit[SENSOR_LOWEST],
			CFlop->State.Power,
			Shm->Cpu[cpu].Sensors.Power.Limit[SENSOR_HIGHEST]) );
}

size_t (*Draw_Energy_Power_Matrix[])(Layer*, const unsigned int, CUINT) = {
	Draw_Energy_Joule,
	Draw_Power_Watt
};

size_t Draw_Energy_SMT(Layer *layer, const unsigned int cpu, CUINT row)
{
	return (Draw_Energy_Power_Matrix[Setting.jouleWatt](layer, cpu, row));
}

size_t Draw_Energy_Core(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1)) {
		return (Draw_Energy_SMT(layer, cpu, row));
	} else {
		return (Draw_Energy_None(layer, cpu, row));
	}
}

size_t Draw_Energy_Pkg(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == Shm->Proc.Service.Core) {
		return (Draw_Energy_SMT(layer, cpu, row));
	} else {
		return (Draw_Energy_None(layer, cpu, row));
	}
}

size_t (*Draw_Energy_Matrix[4])(Layer*, const unsigned int, CUINT) = \
{
	[FORMULA_SCOPE_NONE] = Draw_Energy_None,
	[FORMULA_SCOPE_SMT ] = Draw_Energy_SMT,
	[FORMULA_SCOPE_CORE] = Draw_Energy_Core,
	[FORMULA_SCOPE_PKG ] = Draw_Energy_Pkg
};

CUINT Draw_Monitor_Energy(Layer *layer, const unsigned int cpu, CUINT row)
{
	size_t len;
	const enum FORMULA_SCOPE
		powerScope = SCOPE_OF_FORMULA(Shm->Proc.powerFormula);

	len = Draw_Energy_Matrix[powerScope](layer, cpu, row);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

size_t Draw_Monitor_Slice_Error_Relative_Freq(	struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice )
{
	return (snprintf(Buffer, 8+20+20+20+20+20+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%18llu",
			CFlop->Relative.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			pSlice->Error));
}

size_t Draw_Monitor_Slice_Error_Absolute_Freq(	struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice )
{
	return (snprintf(Buffer, 8+20+20+20+20+20+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%18llu",
			CFlop->Absolute.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			pSlice->Error));
}

size_t Draw_Monitor_Slice_NoError_Relative_Freq(struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice)
{
	return (snprintf(Buffer, 8+20+20+20+20+18+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%.*s",
			CFlop->Relative.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			18, hSpace));
}

size_t Draw_Monitor_Slice_NoError_Absolute_Freq(struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice)
{
	return (snprintf(Buffer, 8+20+20+20+20+18+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%.*s",
			CFlop->Absolute.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			18, hSpace));
}

size_t (*Draw_Monitor_Slice_Matrix[2][2])(struct FLIP_FLOP *CFlop,
					struct SLICE_STRUCT *pSlice) = \
{
	{
		Draw_Monitor_Slice_NoError_Relative_Freq,
		Draw_Monitor_Slice_NoError_Absolute_Freq
	},
	{
		Draw_Monitor_Slice_Error_Relative_Freq,
		Draw_Monitor_Slice_Error_Absolute_Freq
	}
};

CUINT Draw_Monitor_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];
	struct SLICE_STRUCT *pSlice = &Shm->Cpu[cpu].Slice;

	const unsigned int flagError = (Shm->Cpu[cpu].Slice.Error > 0);
	size_t len;
	len = Draw_Monitor_Slice_Matrix[flagError][draw.Load](CFlop, pSlice);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return (0);
}

CUINT Draw_AltMonitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	size_t len;
	UNUSED(cpu);

	row += 1 + draw.Area.MaxRows;
	if (!draw.Flag.avgOrPC) {
		len = snprintf( Buffer, ((5*2)+1)+(6*7)+1,
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% " \
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%",
				100.f * Shm->Proc.Avg.Turbo,
				100.f * Shm->Proc.Avg.C0,
				100.f * Shm->Proc.Avg.C1,
				100.f * Shm->Proc.Avg.C3,
				100.f * Shm->Proc.Avg.C6,
				100.f * Shm->Proc.Avg.C7 );
		memcpy(&LayerAt(layer, code, 20, row), Buffer, len);
	} else {
		len = snprintf( Buffer, ((6*4)+3+5)+(8*6)+1,
				"c2:%-5.1f"	" c3:%-5.1f"	" c4:%-5.1f" \
				" c6:%-5.1f"	" c7:%-5.1f"	" c8:%-5.1f" \
				" c9:%-5.1f"	" c10:%-5.1f",
				100.f * Shm->Proc.State.PC02,
				100.f * Shm->Proc.State.PC03,
				100.f * Shm->Proc.State.PC04,
				100.f * Shm->Proc.State.PC06,
				100.f * Shm->Proc.State.PC07,
				100.f * Shm->Proc.State.PC08,
				100.f * Shm->Proc.State.PC09,
				100.f * Shm->Proc.State.PC10 );
		memcpy(&LayerAt(layer, code, 7, row), Buffer, len);
	}
	row += 1;
	return (row);
}

CUINT Draw_AltMonitor_Common(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(layer);
	UNUSED(cpu);

	row += 2 + draw.Area.MaxRows;
	return (row);
}

CUINT Draw_AltMonitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	CUINT bar0, bar1, margin = draw.Area.LoadWidth - 28;
	size_t len;
	UNUSED(cpu);

	row += 1;
/* PC02 */
	bar0 = Shm->Proc.State.PC02 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC02, 100.f * Shm->Proc.State.PC02,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, row), Buffer, len);
/* PC03 */
	bar0 = Shm->Proc.State.PC03 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC03, 100.f * Shm->Proc.State.PC03,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 1)), Buffer, len);
/* PC04 */
	bar0 = Shm->Proc.State.PC04 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC04, 100.f * Shm->Proc.State.PC04,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 2)), Buffer, len);
/* PC06 */
	bar0 = Shm->Proc.State.PC06 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC06, 100.f * Shm->Proc.State.PC06,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 3)), Buffer, len);
/* PC07 */
	bar0 = Shm->Proc.State.PC07 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC07, 100.f * Shm->Proc.State.PC07,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 4)), Buffer, len);
/* PC08 */
	bar0 = Shm->Proc.State.PC08 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC08, 100.f * Shm->Proc.State.PC08,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 5)), Buffer, len);
/* PC09 */
	bar0 = Shm->Proc.State.PC09 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC09, 100.f * Shm->Proc.State.PC09,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 6)), Buffer, len);
/* PC10 */
	bar0 = Shm->Proc.State.PC10 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC10, 100.f * Shm->Proc.State.PC10,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 7)), Buffer, len);
/* MC6 */
	bar0 = Shm->Proc.State.MC6 * margin;
	bar1 = margin - bar0;

	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.MC6, 100.f * Shm->Proc.State.MC6,
			bar0, hBar, bar1, hSpace );
	memcpy(&LayerAt(layer, code, 5, (row + 8)), Buffer, len);
/* TSC & UNCORE */
	len = snprintf( Buffer, draw.Area.LoadWidth,
			"%18llu" "%.*s" "UNCORE:%18llu",
			PFlop->Delta.PTSC, 7+2+18, hSpace, PFlop->Uncore.FC0 );
	memcpy(&LayerAt(layer, code, 5, (row + 9)), Buffer, len);

	row += 1 + 10;
	return (row);
}

CUINT Draw_AltMonitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(cpu);

  if (Shm->SysGate.tickStep == Shm->SysGate.tickReset)
  {
	ssize_t len = 0;
	signed int idx;
	char stateStr[TASK_COMM_LEN];
	ATTRIBUTE *stateAttr;

	/* Clear the trailing garbage chars left by the previous drawing. */
	FillLayerArea(	layer, (LOAD_LEAD + 8), (row + 1),
			(draw.Size.width - (LOAD_LEAD + 8)), draw.Area.MaxRows,
			hSpace, MakeAttr(BLACK, 0, BLACK, 0) );

    for (idx = 0; idx < Shm->SysGate.taskCount; idx++)
    {
      if (!BITVAL(Shm->Cpu[Shm->SysGate.taskList[idx].wake_cpu].OffLine, OS)
	&& (Shm->SysGate.taskList[idx].wake_cpu >= (short int)draw.cpuScroll)
	&& (Shm->SysGate.taskList[idx].wake_cpu < (short int)(draw.cpuScroll
							+ draw.Area.MaxRows)))
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
		len = snprintf( Buffer, TASK_COMM_LEN, "%s",
				Shm->SysGate.taskList[idx].comm );
	    } else {
		switch (Shm->SysGate.sortByField) {
		case F_STATE:
			len = snprintf( Buffer, 2*TASK_COMM_LEN+2, "%s(%s)",
					Shm->SysGate.taskList[idx].comm,
					stateStr );
			break;
		case F_RTIME:
			len = snprintf( Buffer, TASK_COMM_LEN+20+2, "%s(%llu)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].runtime );
			break;
		case F_UTIME:
			len = snprintf( Buffer, TASK_COMM_LEN+20+2, "%s(%llu)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].usertime );
			break;
		case F_STIME:
			len = snprintf( Buffer, TASK_COMM_LEN+20+2, "%s(%llu)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].systime );
			break;
		case F_PID:
			/* fallthrough */
		case F_COMM:
			len = snprintf( Buffer, TASK_COMM_LEN+11+2, "%s(%d)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].pid );
			break;
		}
	    }
	    if (dif >= len) {
		LayerCopyAt(layer,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row,
			len, stateAttr, Buffer);

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col += len;
	      /* Add a blank spacing between items: up to (ldx) chars	*/
	      while ((dif = draw.Size.width
			- cTask[Shm->SysGate.taskList[idx].wake_cpu].col) > 0
		&& ldx--)
	      {
		LayerAt(layer, attr,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row) =
						MakeAttr(WHITE, 0, BLACK, 0);
		LayerAt(layer, code,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row) = 0x20;

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col++;
	      }
	    } else {
		LayerCopyAt(layer,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
			cTask[Shm->SysGate.taskList[idx].wake_cpu].row,
			dif, stateAttr, Buffer);

		cTask[Shm->SysGate.taskList[idx].wake_cpu].col += dif;
	    }
	}
      }
    }
  }
  row += 2 + draw.Area.MaxRows;
  return (row);
}

void Draw_AltMonitor_Energy_Joule(void)
{
	snprintf(Buffer, 9+9+9+9+1,
		"%8.4f" "%8.4f" "%8.4f" "%8.4f",
		Shm->Proc.State.Energy[PWR_DOMAIN(PKG)].Current,
		Shm->Proc.State.Energy[PWR_DOMAIN(CORES)].Current,
		Shm->Proc.State.Energy[PWR_DOMAIN(UNCORE)].Current,
		Shm->Proc.State.Energy[PWR_DOMAIN(RAM)].Current);
}

void Draw_AltMonitor_Power_Watt(void)
{
	snprintf(Buffer, 9+9+9+9+1,
		"%8.4f" "%8.4f" "%8.4f" "%8.4f",
		Shm->Proc.State.Power[PWR_DOMAIN(PKG)].Current,
		Shm->Proc.State.Power[PWR_DOMAIN(CORES)].Current,
		Shm->Proc.State.Power[PWR_DOMAIN(UNCORE)].Current,
		Shm->Proc.State.Power[PWR_DOMAIN(RAM)].Current);
}

void (*Draw_AltMonitor_Power_Matrix[])(void) = {
	Draw_AltMonitor_Energy_Joule,
	Draw_AltMonitor_Power_Watt
};

CUINT Draw_AltMonitor_Power(Layer *layer, const unsigned int cpu, CUINT row)
{
	const CUINT col = LOAD_LEAD;
	UNUSED(cpu);

	row += 1 + draw.Area.MaxRows;

	Draw_AltMonitor_Power_Matrix[Setting.jouleWatt]();

	memcpy(&LayerAt(layer, code, col +  1,	row), &Buffer[24], 8);
	memcpy(&LayerAt(layer, code, col + 22,	row), &Buffer[16], 8);
	memcpy(&LayerAt(layer, code, col + 44,	row), &Buffer[ 0], 8);
	memcpy(&LayerAt(layer, code, col + 64,	row), &Buffer[ 8], 8);

	row += 1;
	return (row);
}

CUINT Draw_AltMonitor_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	const CUINT col = LOAD_LEAD + 8;
	UNUSED(cpu);

	row += 1 + draw.Area.MaxRows;

	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];

	snprintf( Buffer, 2*11+4*6+1,
			"%7d" "%05.4f" "%05.4f" "%05.4f" "%7d" "%05.4f",
			PFlop->Voltage.VID.CPU,
			Shm->Proc.State.Voltage.Limit[SENSOR_LOWEST],
			PFlop->Voltage.CPU,
			Shm->Proc.State.Voltage.Limit[SENSOR_HIGHEST],
			PFlop->Voltage.VID.SOC,
			PFlop->Voltage.SOC );

	memcpy(&LayerAt(layer, code, col,	row), &Buffer [0], 7);
	memcpy(&LayerAt(layer, code, col + 12,	row), &Buffer [7], 6);
	memcpy(&LayerAt(layer, code, col + 21,	row), &Buffer[13], 6);
	memcpy(&LayerAt(layer, code, col + 30,	row), &Buffer[19], 6);
	memcpy(&LayerAt(layer, code, col + 49,	row), &Buffer[25], 7);
	memcpy(&LayerAt(layer, code, col + 59,	row), &Buffer[32], 6);

	row += 1;
	return (row);
}
#endif /* NO_LOWER */

#ifndef NO_FOOTER
/* >>> GLOBALS >>> */
char *Format_Temp_Voltage[2][2] = {
	[0]={ [0]="\x20\x20\x20\x20\x20\x20\x20", [1]="\x20\x20\x20%2$4.2f" },
	[1]={ [0]="%1$3u\x20\x20\x20\x20"	, [1]="%1$3u%2$4.2f" }
};
/* <<< GLOBALS <<< */

void Draw_Footer_Voltage_Fahrenheit(struct PKG_FLIP_FLOP *PFlop)
{
	const enum FORMULA_SCOPE fmt[2] = {
	(SCOPE_OF_FORMULA(Shm->Proc.thermalFormula) != FORMULA_SCOPE_NONE),
	(SCOPE_OF_FORMULA(Shm->Proc.voltageFormula) != FORMULA_SCOPE_NONE)
	};
	snprintf(Buffer, 10+5+1, Format_Temp_Voltage[ fmt[0] ] [ fmt[1] ],
			Cels2Fahr(PFlop->Thermal.Temp),
			PFlop->Voltage.CPU);
}

void Draw_Footer_Voltage_Celsius(struct PKG_FLIP_FLOP *PFlop)
{
	const enum FORMULA_SCOPE fmt[2] = {
	(SCOPE_OF_FORMULA(Shm->Proc.thermalFormula) != FORMULA_SCOPE_NONE),
	(SCOPE_OF_FORMULA(Shm->Proc.voltageFormula) != FORMULA_SCOPE_NONE)
	};
	snprintf(Buffer, 10+5+1, Format_Temp_Voltage[ fmt[0] ] [ fmt[1] ],
			PFlop->Thermal.Temp,
			PFlop->Voltage.CPU);
}

void (*Draw_Footer_Voltage_Temp[])(struct PKG_FLIP_FLOP*) = {
	Draw_Footer_Voltage_Celsius,
	Draw_Footer_Voltage_Fahrenheit
};

void Draw_Footer(Layer *layer, CUINT row)
{	/* Update Footer view area					*/
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];

	ATTRIBUTE *eventAttr[] = {
		RSC(HOT_EVENT_COND0).ATTR(),
		RSC(HOT_EVENT_COND1).ATTR(),
		RSC(HOT_EVENT_COND2).ATTR(),
		RSC(HOT_EVENT_COND3).ATTR(),
		RSC(HOT_EVENT_COND4).ATTR()
	};
	unsigned int _hot = 0, _tmp = 0;

	if (!processorEvents) {
		_hot = 0;
		_tmp = 3;
	} else {
	    if (processorEvents &	( EVENT_THERM_SENSOR
					| EVENT_THERM_PROCHOT
					| EVENT_THERM_CRIT
					| EVENT_THERM_THOLD ) )
	    {
		_hot = 4;
		_tmp = 1;
	    } else {
		_hot = 2;
		_tmp = 3;
	    }
	}

	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
		LayerAt(layer, attr, 14+46, row) = eventAttr[_hot][0];
		LayerAt(layer, attr, 14+47, row) = eventAttr[_hot][1];
		LayerAt(layer, attr, 14+48, row) = eventAttr[_hot][2];
	}
	else if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	     || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
	{
		LayerAt(layer, attr, 14+42, row) = eventAttr[_hot][0];
		LayerAt(layer, attr, 14+43, row) = eventAttr[_hot][1];
		LayerAt(layer, attr, 14+44, row) = eventAttr[_hot][2];
	}
	LayerAt(layer, attr, 14+61, row) = eventAttr[_tmp][0];
	LayerAt(layer, attr, 14+62, row) = eventAttr[_tmp][1];
	LayerAt(layer, attr, 14+63, row) = eventAttr[_tmp][2];

	Draw_Footer_Voltage_Temp[Setting.fahrCels](PFlop);

	LayerAt(layer, code, 75, row) = Buffer[0];
	LayerAt(layer, code, 76, row) = Buffer[1];
	LayerAt(layer, code, 77, row) = Buffer[2];

	LayerAt(layer, code, 67, row) = Buffer[3];
	LayerAt(layer, code, 68, row) = Buffer[4];
	LayerAt(layer, code, 69, row) = Buffer[5];
	LayerAt(layer, code, 70, row) = Buffer[6];

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)
	&& (Shm->SysGate.tickStep == Shm->SysGate.tickReset)) {
		if ((draw.Cache.TaskCount != Shm->SysGate.taskCount)
		 || (draw.Cache.FreeRAM != Shm->SysGate.memInfo.freeram)) {
			draw.Cache.TaskCount = Shm->SysGate.taskCount;
			draw.Cache.FreeRAM = Shm->SysGate.memInfo.freeram;

			PrintTaskMemory(layer, (row + 1),
					Shm->SysGate.taskCount,
					Shm->SysGate.memInfo.freeram,
					Shm->SysGate.memInfo.totalram);
		}
	}
}
#endif /* NO_FOOTER */

#ifndef NO_HEADER
void Draw_Header(Layer *layer, CUINT row)
{	/* Update Header view area					*/
	struct FLIP_FLOP *CFlop;
	unsigned int digit[9];

	/* Print the Top frequency in MHz Or the C0 C-State % load	*/
    if (!draw.Flag.clkOrLd)
    {
	if (draw.Load) {
		const unsigned int top = Shm->Proc.Top.Abs;
		CFlop = &Shm->Cpu[top].FlipFlop[!Shm->Cpu[top].Toggle];

		Clock2LCD(layer, 0,row, CFlop->Absolute.Freq,
					CFlop->Absolute.Ratio.Perf);
	} else {
		const unsigned int top = Shm->Proc.Top.Rel;
		CFlop = &Shm->Cpu[top].FlipFlop[!Shm->Cpu[top].Toggle];

		Clock2LCD(layer, 0,row, CFlop->Relative.Freq,
					CFlop->Relative.Ratio);
	}
    } else {
	if (draw.Cache.TopAvg != Shm->Proc.Avg.C0) {
		double percent = 100.f * Shm->Proc.Avg.C0;
		draw.Cache.TopAvg = Shm->Proc.Avg.C0;

		Load2LCD(layer, 0, row, percent);
	}
    }
	/* Print the focused BCLK					*/
	row += 2;

	CFlop = &Shm->Cpu[ draw.iClock + draw.cpuScroll ]		\
		.FlipFlop[ !Shm->Cpu[draw.iClock + draw.cpuScroll].Toggle ];

	Dec2Digit(9, CFlop->Clock.Hz, digit);

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
#endif /* NO_HEADER */

/* >>> GLOBALS >>> */
#ifndef NO_LOWER
VIEW_FUNC Matrix_Layout_Monitor[VIEW_SIZE] = {
	Layout_Monitor_Frequency,
	Layout_Monitor_Instructions,
	Layout_Monitor_Common,
	Layout_Monitor_Common,
	Layout_Monitor_Package,
	Layout_Monitor_Tasks,
	Layout_Monitor_Common,
	Layout_Monitor_Common,
	Layout_Monitor_Common,
	Layout_Monitor_Common,
	Layout_Monitor_Slice
};

VIEW_FUNC Matrix_Layout_Ruler[VIEW_SIZE] = {
	Layout_Ruler_Frequency,
	Layout_Ruler_Instructions,
	Layout_Ruler_Cycles,
	Layout_Ruler_CStates,
	Layout_Ruler_Package,
	Layout_Ruler_Tasks,
	Layout_Ruler_Interrupts,
	Layout_Ruler_Sensors,
	Layout_Ruler_Voltage,
	Layout_Ruler_Energy,
	Layout_Ruler_Slice
};
#endif /* NO_LOWER */

#ifndef NO_UPPER
VIEW_FUNC Matrix_Draw_Load[] = {
	Draw_Relative_Load,
	Draw_Absolute_Load
};
#endif /* NO_UPPER */

#ifndef NO_LOWER
VIEW_FUNC Matrix_Draw_Monitor[VIEW_SIZE] = {
	Draw_Monitor_Frequency,
	Draw_Monitor_Instructions,
	Draw_Monitor_Cycles,
	Draw_Monitor_CStates,
	Draw_Monitor_Package,
	Draw_Monitor_Tasks,
	Draw_Monitor_Interrupts,
	Draw_Monitor_Sensors,
	Draw_Monitor_Voltage,
	Draw_Monitor_Energy,
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
	Draw_AltMonitor_Voltage,
	Draw_AltMonitor_Power,
	Draw_AltMonitor_Common
};
#endif /* NO_LOWER */
/* <<< GLOBALS <<< */

#ifndef NO_UPPER
	#define Illuminates_Upper_CPU_At(_layer, _col, _row)		\
		LayerAt(_layer, attr, _col, _row) =
#else
	#define Illuminates_Upper_CPU_At(_layer, _col, _row)
#endif

#ifndef NO_LOWER
#ifndef NO_UPPER
	#define Illuminates_Lower_CPU_At(_layer, _col, _row)		\
		LayerAt(_layer, attr, _col, (1 + _row + draw.Area.MaxRows)) =
#else
	#define Illuminates_Lower_CPU_At(_layer, _col, _row)		\
		LayerAt(_layer, attr, _col, (_row)) =
#endif
#else
	#define Illuminates_Lower_CPU_At(_layer, _col, _row)
#endif

#define Illuminates_CPU(_layer, _row, fg, bg, hi)			\
({									\
	Illuminates_Upper_CPU_At(_layer, 0, _row)			\
	Illuminates_Lower_CPU_At(_layer, 0, _row)			\
	Illuminates_Upper_CPU_At(_layer, 1, _row)			\
	Illuminates_Lower_CPU_At(_layer, 1, _row)			\
	Illuminates_Upper_CPU_At(_layer, 2, _row)			\
	Illuminates_Lower_CPU_At(_layer, 2, _row)			\
									\
						MakeAttr(fg, 0, bg, hi);\
									\
})

void Layout_Header_DualView_Footer(Layer *layer)
{
	unsigned int cpu;
	CUINT row = 0;

#ifndef NO_HEADER
	Layout_Header(layer, row);
#endif
	row += TOP_HEADER_ROW;
#ifndef NO_UPPER
	Layout_Ruler_Load(layer, row);
#endif
  for(cpu = draw.cpuScroll; cpu < (draw.cpuScroll + draw.Area.MaxRows); cpu++)
  {
	Layout_CPU_To_String(cpu);
#ifndef NO_UPPER
	Layout_CPU_To_View(layer, 0, row + 1);

	Layout_BCLK_To_View(layer, 0, row + 1);
#endif
	row = row + 1;
#ifndef NO_LOWER
#ifndef NO_UPPER
    if (draw.View != V_PACKAGE) {
	Layout_CPU_To_View(layer, 0, row + draw.Area.MaxRows + 1);
    }
#else
    if (draw.View != V_PACKAGE) {
	Layout_CPU_To_View(layer, 0, row);
    }
#endif
#endif
    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
    {
	if (cpu == Shm->Proc.Service.Core) {
		Illuminates_CPU(layer, row, CYAN, BLACK, 1);
	} else if ((signed int) cpu == Shm->Proc.Service.Thread) {
		Illuminates_CPU(layer, row, CYAN, BLACK, 1);
	} else {
		Illuminates_CPU(layer, row, CYAN, BLACK, 0);
	}
#ifndef NO_LOWER
#ifndef NO_UPPER
	Matrix_Layout_Monitor[draw.View](layer,cpu,row + draw.Area.MaxRows + 1);
#else
	Matrix_Layout_Monitor[draw.View](layer,cpu,row);
#endif
#endif
    }
    else
    {
	Illuminates_CPU(layer, row, BLUE, BLACK, 0);

#ifndef NO_UPPER
	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), row,
			(draw.Size.width - LOAD_LEAD + 1), 0x0);
#endif
#ifndef NO_LOWER
#ifndef NO_UPPER
	ClearGarbage(	dLayer, attr,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			(draw.Size.width - LOAD_LEAD + 1),
			MakeAttr(BLACK,0,BLACK,0).value );

	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), (row + draw.Area.MaxRows + 1),
			(draw.Size.width - LOAD_LEAD + 1), 0x0 );
#else
	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), row,
			(draw.Size.width - LOAD_LEAD + 1), 0x0 );
#endif
#endif
    }
  }
	row++;
#ifndef NO_LOWER
#ifndef NO_UPPER
	row = Matrix_Layout_Ruler[draw.View](layer, 0, row);
#else
	row = Matrix_Layout_Ruler[draw.View](layer, 0, TOP_HEADER_ROW);
#endif
#endif
#ifndef NO_FOOTER
	Layout_Footer(layer, row);
#endif
}

void Dynamic_Header_DualView_Footer(Layer *layer)
{
	unsigned int cpu;
	CUINT row = 0;

	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	processorEvents = PFlop->Thermal.Events;
#ifndef NO_HEADER
	Draw_Header(layer, row);
#endif
	row += TOP_HEADER_ROW;

  for (cpu = draw.cpuScroll; cpu < (draw.cpuScroll + draw.Area.MaxRows); cpu++)
  {
	struct FLIP_FLOP *CFlop=&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	processorEvents |= CFlop->Thermal.Events;

	row++;

    if (!BITVAL(Shm->Cpu[cpu].OffLine, OS))
    {
      if (!BITVAL(Shm->Cpu[cpu].OffLine, HW))
      {
#ifndef NO_UPPER
	Matrix_Draw_Load[draw.Load](layer, cpu, row);
#endif
      }
	/*	Print the Per Core BCLK indicator (yellow)		*/
#ifdef NO_UPPER
    if (draw.View != V_PACKAGE) {
#endif
#if !defined(NO_LOWER) || !defined(NO_UPPER)
	LayerAt(layer, code, (LOAD_LEAD - 1), row) =
			(draw.iClock == (cpu - draw.cpuScroll)) ? '~' : 0x20;
#endif
#ifdef NO_UPPER
    }
#endif

#ifndef NO_LOWER
#ifndef NO_UPPER
	Matrix_Draw_Monitor[draw.View](layer, cpu, row + draw.Area.MaxRows + 1);
#else
	Matrix_Draw_Monitor[draw.View](layer, cpu, row);
#endif
#endif
    }
  }
	row++;
#ifndef NO_LOWER
#ifndef NO_UPPER
	row = Matrix_Draw_AltMon[draw.View](layer, 0, row);
#else
	row = Matrix_Draw_AltMon[draw.View](layer, 0, TOP_HEADER_ROW);
#endif
#endif
#ifndef NO_FOOTER
	Draw_Footer(layer, row);
#endif
}

void Layout_Card_Core(Layer *layer, Card *card)
{
	unsigned int digit[3];
	unsigned int _cpu = card->data.dword.lo;

	Dec2Digit(3, _cpu, digit);

	if (!BITVAL(Shm->Cpu[_cpu].OffLine, OS))
	{
	    if (Setting.fahrCels) {
		LayerDeclare(	LAYOUT_CARD_CORE_ONLINE_COND1,(4 * INTER_WIDTH),
				card->origin.col, (card->origin.row + 3),
				hOnLine);

		LayerCopyAt(layer, hOnLine.origin.col, hOnLine.origin.row,
				hOnLine.length, hOnLine.attr, hOnLine.code);
	    } else {
		LayerDeclare(	LAYOUT_CARD_CORE_ONLINE_COND0,(4 * INTER_WIDTH),
				card->origin.col, (card->origin.row + 3),
				hOnLine);

		LayerCopyAt(layer, hOnLine.origin.col, hOnLine.origin.row,
				hOnLine.length, hOnLine.attr, hOnLine.code);
	    }
	} else {
		LayerDeclare(	LAYOUT_CARD_CORE_OFFLINE, (4 * INTER_WIDTH),
				card->origin.col, (card->origin.row + 3),
				hOffLine);

		card->data.dword.hi = RENDER_KO;

		LayerFillAt(layer, card->origin.col, (card->origin.row + 1),
		(4 * INTER_WIDTH), " _  _  _  _ ", MakeAttr(BLACK,0,BLACK,1));

		LayerCopyAt(layer, hOffLine.origin.col, hOffLine.origin.row,
				hOffLine.length, hOffLine.attr, hOffLine.code);
	}
	LayerAt(layer, code,
		(card->origin.col + 2),
		(card->origin.row + 3)) = digit[0] + '0';

	LayerAt(layer, code,
		(card->origin.col + 3),
		(card->origin.row + 3)) = digit[1] + '0';

	LayerAt(layer, code,
		(card->origin.col + 4),
		(card->origin.row + 3)) = digit[2] + '0';
}

void Layout_Card_CLK(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_CLK, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hCLK);

	LayerCopyAt(	layer, hCLK.origin.col, hCLK.origin.row,
			hCLK.length, hCLK.attr, hCLK.code);
}

void Layout_Card_Uncore(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_UNCORE, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hUncore);

	snprintf(Buffer, 10+1, "x%2u", Shm->Uncore.Boost[UNCORE_BOOST(MAX)]);
	hUncore.code[ 8] = Buffer[0];
	hUncore.code[ 9] = Buffer[1];
	hUncore.code[10] = Buffer[2];

	LayerCopyAt(	layer, hUncore.origin.col, hUncore.origin.row,
			hUncore.length, hUncore.attr, hUncore.code);
}

void Layout_Card_Bus(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_BUS, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hBus);

	snprintf(Buffer, 10+1, "%4u", Shm->Uncore.Bus.Rate);
	hBus.code[5] = Buffer[0];
	hBus.code[6] = Buffer[1];
	hBus.code[7] = Buffer[2];
	hBus.code[8] = Buffer[3];

	switch (Shm->Uncore.Unit.Bus_Rate) {
	case 0b00:
		hBus.code[ 9] = 'H';
		hBus.code[10] = 'z';
		break;
	case 0b01:
		hBus.code[ 9] = 'M';
		hBus.code[10] = 'T';
		break;
	case 0b10:
		hBus.code[ 9] = 'B';
		hBus.code[10] = 's';
		break;
	}

	LayerCopyAt(	layer, hBus.origin.col, hBus.origin.row,
			hBus.length, hBus.attr, hBus.code);

	Counter2LCD(	layer, card->origin.col, card->origin.row,
			(double) Shm->Uncore.Bus.Speed);
}

static char Card_MC_Timing[11+(4*10)+5+1];

void Layout_Card_MC(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_MC, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hRAM);

	card->data.dword.lo = 0;
	card->data.dword.hi = snprintf(	Card_MC_Timing, 11+(4*10)+5+1,
					"% d-%u-%u-%u-%uT",
				Shm->Uncore.MC[0].Channel[0].Timing.tCL,
				Shm->Uncore.MC[0].Channel[0].Timing.tRCD,
				Shm->Uncore.MC[0].Channel[0].Timing.tRP,
				Shm->Uncore.MC[0].Channel[0].Timing.tRAS,
				Shm->Uncore.MC[0].Channel[0].Timing.CMD_Rate );

	LayerCopyAt(	layer, hRAM.origin.col, hRAM.origin.row,
			hRAM.length, hRAM.attr, hRAM.code);

	if (Shm->Uncore.CtrlCount > 0) {
		Counter2LCD(layer, card->origin.col, card->origin.row,
				(double) Shm->Uncore.CtrlSpeed);
	}
}

void Layout_Card_Load(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_LOAD, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hLoad);

	LayerCopyAt(	layer, hLoad.origin.col, hLoad.origin.row,
			hLoad.length, hLoad.attr, hLoad.code);
}

void Layout_Card_Idle(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_IDLE, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hIdle);

	LayerCopyAt(	layer, hIdle.origin.col, hIdle.origin.row,
			hIdle.length, hIdle.attr, hIdle.code);
}

void Layout_Card_RAM(Layer *layer, Card *card)
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
	unit = ByteReDim(totalDimm, 3, &totalRAM);
	if ((unit >= 0) && (unit < 4)) {
		char symbols[4] = {'M', 'G', 'T', 'P'};
		symbol = symbols[unit];
	}
      } else {
	unit = ByteReDim(Shm->SysGate.memInfo.totalram, 3, &totalRAM);
	if ((unit >= 0) && (unit < 4)) {
		char symbols[4] = {'K', 'M', 'G', 'T'};
		symbol = symbols[unit];
	}
      }
      if (totalRAM > 99) {
	hMem.attr[hMem.length-2] = MakeAttr(WHITE, 0, BLACK, 1);
	snprintf(Buffer, 20+1+1, "%3lu", totalRAM);
      } else {
	hMem.attr[hMem.length-2] = MakeAttr(WHITE, 0, BLACK, 0);
	snprintf(Buffer, 20+1+1, "%2lu%c", totalRAM, symbol);
      }
	memcpy(&hMem.code[8], Buffer, 3);

	LayerCopyAt(	layer, hMem.origin.col, hMem.origin.row,
			hMem.length, hMem.attr, hMem.code);
    } else {
	card->data.dword.hi = RENDER_KO;
    }
}

void Layout_Card_Task(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_TASK, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hSystem);

	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		LayerCopyAt(	layer, hSystem.origin.col, hSystem.origin.row,
				hSystem.length, hSystem.attr, hSystem.code);
	} else {
		card->data.dword.hi = RENDER_KO;
	}
}

unsigned int MoveDashboardCursor(Coordinate *coord)
{
	const CUINT	marginWidth = MARGIN_WIDTH + (4 * INTER_WIDTH);
	const CUINT	marginHeight = MARGIN_HEIGHT + INTER_HEIGHT;
	const CUINT	rightEdge = draw.Size.width - marginWidth,
			bottomEdge = draw.Size.height + marginHeight;

	coord->col += marginWidth;
	if (coord->col > rightEdge) {
		coord->col = LEADING_LEFT;
		coord->row += marginHeight;
	}
	if (coord->row > bottomEdge) {
		return (RENDER_KO);
	}
	return (RENDER_OK);
}

void Layout_Dashboard(Layer *layer)
{
	Coordinate coord = {.col = LEADING_LEFT, .row = LEADING_TOP};

	Card *walker = cardList.head;
	while (walker != NULL) {
		walker->origin.col = coord.col;
		walker->origin.row = coord.row;
		walker->data.dword.hi = MoveDashboardCursor(&coord);
		if (walker->data.dword.hi == RENDER_OK) {
			walker->hook.Layout(layer, walker);
		}
		walker = GetNext(walker);
	}
}

void Draw_Card_Core(Layer *layer, Card *card)
{
  if (card->data.dword.hi == RENDER_OK)
  {
	const enum FORMULA_SCOPE
		thermalScope = SCOPE_OF_FORMULA(Shm->Proc.thermalFormula);

	unsigned int _cpu = card->data.dword.lo;

	struct FLIP_FLOP *CFlop = \
			&Shm->Cpu[_cpu].FlipFlop[!Shm->Cpu[_cpu].Toggle];

	ATTRIBUTE warning = {.fg = WHITE, .un = 0, .bg = BLACK, .bf = 1};

	Clock2LCD(layer, card->origin.col, card->origin.row,
			CFlop->Relative.Freq, CFlop->Relative.Ratio);

   if(CFlop->Thermal.Temp <= Shm->Cpu[_cpu].PowerThermal.Limit[SENSOR_LOWEST]) {
		warning = MakeAttr(BLUE, 0, BLACK, 1);
   } else {
    if(CFlop->Thermal.Temp >= Shm->Cpu[_cpu].PowerThermal.Limit[SENSOR_HIGHEST])
		warning = MakeAttr(YELLOW, 0, BLACK, 0);
   }
	if (CFlop->Thermal.Events) {
		warning = MakeAttr(RED, 0, BLACK, 1);
	}

	LayerAt(layer, attr, (card->origin.col + 6), (card->origin.row + 3)) = \
	LayerAt(layer, attr, (card->origin.col + 7), (card->origin.row + 3)) = \
	LayerAt(layer, attr, (card->origin.col + 8), (card->origin.row + 3)) = \
									warning;
	LayerAt(layer, code, (card->origin.col + 6), (card->origin.row + 3)) = \
	LayerAt(layer, code, (card->origin.col + 7), (card->origin.row + 3)) = \
	LayerAt(layer, code, (card->origin.col + 8), (card->origin.row + 3)) = \
									0x20;
    switch (thermalScope) {
    case FORMULA_SCOPE_NONE:
	break;
    case FORMULA_SCOPE_PKG:
	if ( !(_cpu == Shm->Proc.Service.Core) ) {
		break;
	}
	/* Fallthrough */
    case FORMULA_SCOPE_CORE:
	if (	!( (Shm->Cpu[_cpu].Topology.ThreadID == 0)
		|| (Shm->Cpu[_cpu].Topology.ThreadID == -1)) ) {
		break;
	}
	/* Fallthrough */
    case FORMULA_SCOPE_SMT:
	{
	unsigned int digit[3];
	Dec2Digit( 3, Setting.fahrCels	? Cels2Fahr(CFlop->Thermal.Temp)
					: CFlop->Thermal.Temp, digit );

	LayerAt(layer, code, (card->origin.col + 6), (card->origin.row + 3)) = \
						digit[0] ? digit[0] + '0':0x20;

	LayerAt(layer, code, (card->origin.col + 7), (card->origin.row + 3)) = \
				(digit[0] | digit[1]) ? digit[1] + '0' : 0x20;

	LayerAt(layer, code, (card->origin.col + 8), (card->origin.row + 3)) = \
								digit[2] + '0';
	}
	break;
    }
  }
  else if (card->data.dword.hi == RENDER_KO)
  {
	CUINT row;

	card->data.dword.hi = RENDER_OFF;

    for (row = card->origin.row; row < card->origin.row + 4; row++) {
	memset(&LayerAt(layer, attr, card->origin.col, row), 0, 4*INTER_WIDTH);
	memset(&LayerAt(layer, code, card->origin.col, row), 0, 4*INTER_WIDTH);
    }
  }
}

void Draw_Card_CLK(Layer *layer, Card *card)
{
	struct FLIP_FLOP *CFlop = &Shm->Cpu[Shm->Proc.Service.Core] \
				.FlipFlop[!Shm->Cpu[Shm->Proc.Service.Core] \
					.Toggle];

	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];

	double clock = CLOCK_MHz(double, PFlop->Delta.PTSC);

	Counter2LCD(layer, card->origin.col, card->origin.row, clock);

	snprintf(Buffer, 6+1, "%5.1f", CLOCK_MHz(double, CFlop->Clock.Hz));

	memcpy(&LayerAt(layer, code, (card->origin.col+2),(card->origin.row+3)),
		Buffer, 5);
}

void Draw_Card_Uncore(Layer *layer, Card *card)
{
	struct PKG_FLIP_FLOP *PFlop = &Shm->Proc.FlipFlop[!Shm->Proc.Toggle];
	double Uncore = CLOCK_MHz(double, PFlop->Uncore.FC0);

	Idle2LCD(layer, card->origin.col, card->origin.row, Uncore);
}

void Draw_Card_MC(Layer *layer, Card *card)
{
	memcpy( &LayerAt(layer,code, (card->origin.col+1),(card->origin.row+3)),
		&Card_MC_Timing[card->data.dword.lo], 10);

	card->data.dword.lo++;
	if ((card->data.dword.lo + 10) > card->data.dword.hi) {
		card->data.dword.lo = 0;
	}
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

void Draw_Card_RAM(Layer *layer, Card *card)
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
	snprintf(Buffer, 20+1+1, "%5lu%c", freeRAM, symbol[unit]);
	memcpy(&LayerAt(layer,code, (card->origin.col+1), (card->origin.row+3)),
		Buffer, 6);
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

void Draw_Card_Task(Layer *layer, Card *card)
{
    if (card->data.dword.hi == RENDER_OK)
    {
      if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
	int	cl = (int) strnlen(Shm->SysGate.taskList[0].comm, 12),
		hl = (12 - cl) / 2, hr = hl + (cl & 1), pb, pe;
	char	stateStr[TASK_COMM_LEN];
	ATTRIBUTE *stateAttr;
	stateAttr = StateToSymbol(Shm->SysGate.taskList[0].state, stateStr);

	snprintf(Buffer, 12+10+3+10+1, "%.*s%.*s%.*s%n%7u(%c)%n%5u",
				hl, hSpace,
				cl, Shm->SysGate.taskList[0].comm,
				hr, hSpace,
				&pb,
				Shm->SysGate.taskList[0].pid,
				stateStr[0],
				&pe,
				Shm->SysGate.taskCount);

	LayerCopyAt(layer,	(card->origin.col + 0),
				(card->origin.row + 1),
				pb,
				stateAttr,
				Buffer);

	LayerFillAt(layer,	(card->origin.col + 1),
				(card->origin.row + 2),
				pe - pb,
				&Buffer[pb],
				MakeAttr(WHITE, 0, BLACK, 0));

	memcpy(&LayerAt(layer, code, (card->origin.col+6),(card->origin.row+3)),
				&Buffer[pe], 5);
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

void Dont_Draw_Card(Layer *layer, Card *card)
{
	UNUSED(layer);
	UNUSED(card);
}

void Draw_Dashboard(Layer *layer)
{
	Card *walker = cardList.head;
	while (walker != NULL) {
		walker->hook.Draw(layer, walker);
		walker = GetNext(walker);
	}
}

void AllocDashboard(void)
{
	unsigned int cpu;
	Card *card = NULL;
	for(cpu=0;(cpu < Shm->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
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
		StoreCard(card, .Draw, Draw_Card_MC);
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
	UNUSED(layer);
}

void Dynamic_NoHeader_SingleView_NoFooter(Layer *layer)
{
	UNUSED(layer);
}

REASON_CODE Top(char option)
{
/*
           SCREEN
 __________________________   __________________________
|           MENU           | |           MENU           |
|                       T  | |                          |
|  L       HEADER          | |        NO_HEADER         |
|                       R  | |                          |
|--E ----------------------| |                          |
|                       A  | |                          |
|  A        LOAD           | |         NO_UPPER         |
|                       I  | |                          |
|--D ----------------------| |                          |
|                       L  | |                          |
|  I       MONITOR         | |         NO_LOWER         |
|                       I  | |                          |
|--N ----------------------| |                          |
|                       N  | |                          |
|  G       FOOTER          | |        NO_FOOTER         |
|                       G  | |                          |
`__________________________' `__________________________'
*/

	REASON_INIT(reason);

	TrapScreenSize(SIGWINCH);

	if (signal(SIGWINCH, TrapScreenSize) == SIG_ERR) {
		REASON_SET(reason);
  } else if((cTask = calloc(Shm->Proc.CPU.Count, sizeof(Coordinate))) == NULL) {
		REASON_SET(reason, RC_MEM_ERR);
  } else if (AllocAll(&Buffer) == ENOMEM) {
		REASON_SET(reason, RC_MEM_ERR, ENOMEM);
  } else
  {
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

	RECORDER_COMPUTE(Recorder, Shm->Sleep.Interval);

	LoadGeometries(BuildConfigFQN("CoreFreq"));

	/* MAIN LOOP */
    while (!BITVAL(Shutdown, SYNC))
    {
      do
      {
	if ((draw.Flag.daemon = BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0)) == 0)
	{
		SCANKEY scan = {.key = 0};

	  if (GetKey(&scan, &Shm->Sleep.pollingWait) > 0) {
	    if (Shortcut(&scan) == -1) {
		if (IsDead(&winList)) {
			AppendWindow(CreateMenu(SCANKEY_F2, 0), &winList);
		}
		else if (Motion_Trigger(&scan,GetFocus(&winList),&winList) > 0)
		{
			Shortcut(&scan);
		}
	    }
		PrintWindowStack(&winList);

		break;
	  } else {
		WindowsUpdate(&winList);
	  }
	}
	if (!RING_NULL(Shm->Error))
	{
		RING_CTRL ctrl __attribute__ ((aligned(16)));
		RING_READ(Shm->Error, ctrl);

		AppendWindow(
			PopUpMessage(RSC(POPUP_DRIVER_TITLE).CODE(), &ctrl),
			&winList );
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, COMP0)) {
		AggregateRatio();
		draw.Flag.clear = 1;	/* Compute required,clear the layout */
	}
	if (BITCLR(LOCKLESS, Shm->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &Shm->Proc.Service, 0);
		RECORDER_COMPUTE(Recorder, Shm->Sleep.Interval);
		draw.Flag.layout = 1;	 /* Platform changed, redraw layout */
	}
      } while ( !BITVAL(Shutdown, SYNC)
		&& !draw.Flag.daemon
		&& !draw.Flag.layout
		&& !draw.Flag.clear ) ;

      if (draw.Flag.height & draw.Flag.width)
      {
	if (draw.Flag.clear) {
		draw.Flag.clear  = 0;
		draw.Flag.layout = 1;
		ResetLayer(dLayer, MakeAttr(BLACK,0,BLACK,0).value, 0x0);
	}
	if (draw.Flag.layout) {
		draw.Flag.layout = 0;
		ResetLayer(sLayer, MakeAttr(BLACK,0,BLACK,0).value, 0x0);
		LayoutView[draw.Disposal](sLayer);
	}
	if (draw.Flag.daemon)
	{
		DynamicView[draw.Disposal](dLayer);

		/* Increment the BCLK indicator (skip offline CPU)	*/
		do {
			draw.iClock++;
			if (draw.iClock >= draw.Area.MaxRows) {
				draw.iClock = 0;
			}
		} while (BITVAL(Shm->Cpu[draw.iClock].OffLine, OS)
			&& (draw.iClock != Shm->Proc.Service.Core)) ;
	}
	Draw_uBenchmark(dLayer);
	UBENCH_RDCOUNTER(1);

	/* Write to the standard output.				*/
	draw.Flag.layout = WriteConsole(draw.Size);

	UBENCH_RDCOUNTER(2);
	UBENCH_COMPUTE();
      } else {
	printf( CUH RoK "Term(%u x %u) < View(%u x %u)\n",
		draw.Size.width,draw.Size.height,MIN_WIDTH,draw.Area.MinHeight);
      }
    }
    SaveGeometries(BuildConfigFQN("CoreFreq"));
  }
  FreeAll(Buffer);

  if (cTask != NULL) {
	free(cTask);
  }
  DestroyAllCards(&cardList);

  return (reason);
}

REASON_CODE Help(REASON_CODE reason, ...)
{
	va_list ap;
	va_start(ap, reason);
	switch (reason.rc) {
	case RC_SUCCESS:
	case RC_OK_SYSGATE:
	case RC_OK_COMPUTE:
	case RC_DRIVER_BASE ... RC_DRIVER_LAST:
		break;
	case RC_PERM_ERR:
	case RC_MEM_ERR:
	case RC_EXEC_ERR:
		break;
	case RC_CMD_SYNTAX: {
		char *appName = va_arg(ap, char *);
		printf((char *) RSC(ERROR_CMD_SYNTAX).CODE(), appName,
				RC_SUCCESS,
				RC_CMD_SYNTAX,
				RC_SHM_FILE,
				RC_SHM_MMAP,
				RC_PERM_ERR,
				RC_MEM_ERR,
				RC_EXEC_ERR,
				RC_SYS_CALL);
		}
		break;
	case RC_SHM_FILE:
	case RC_SHM_MMAP: {
		char *shmFileName = va_arg(ap, char *);
		char *sysMsg = strerror_l(reason.no, SYS_LOCALE());
		fprintf(stderr, (char *) RSC(ERROR_SHARED_MEM).CODE(),
				reason.no, shmFileName, sysMsg, reason.ln);
		}
		break;
	case RC_SYS_CALL: {
		char *sysMsg = strerror(reason.no);
		fprintf(stderr, (char *) RSC(ERROR_SYS_CALL).CODE(),
				reason.no, sysMsg, reason.ln);
		}
		break;
	}
	va_end(ap);
	return (reason);
}

void Emergency(int caught)
{
	switch (caught) {
	case SIGINT:	/* Press [CTRL] + [C] twice to force quit.	*/
	  if (Buffer != NULL)
	  {	/*	UI is running if buffer is allocated		*/
		Window *win = SearchWinListById(SCANKEY_CTRL_x, &winList);
	    if (win == NULL) {
		    if (Shortcut(&(SCANKEY){.key = SCANKEY_CTRL_x}) == 0)
		    {
			PrintWindowStack(&winList);
			break;
		    }
	    }
	  }
		/* Fallthrough */
	case SIGBUS:
	case SIGFPE:
	case SIGHUP:	/* Terminal lost */
	case SIGILL:
	case SIGSYS:
	case SIGQUIT:
	case SIGTERM:
	case SIGSEGV:
	case SIGTSTP:	/* [CTRL] + [Z] */
	case SIGXCPU:
	case SIGXFSZ:
	case SIGSTKFLT:
		BITSET(LOCKLESS, Shutdown, SYNC);
		break;
	}
}

void TrapSignal(int operation)
{
	if (operation == 0) {
		Shm->App.Cli = 0;
	} else {
		const int ignored[] = {
			SIGUSR1, SIGUSR2, SIGTTIN, SIGTTOU, SIGPWR,
			SIGTRAP, SIGALRM, SIGPROF, SIGPIPE, SIGABRT,
			SIGVTALRM, SIGCHLD, SIGWINCH, SIGIO, SIGSEGV
		}, handled[] = {
			SIGBUS, SIGFPE, SIGHUP, SIGILL, SIGINT,
			SIGQUIT, SIGSYS, SIGTERM, SIGTSTP,
			SIGXCPU, SIGXFSZ, SIGSTKFLT
		};
		/* SIGKILL,SIGCONT,SIGSTOP,SIGURG	: Reserved	*/
		const ssize_t	ignoredCount = sizeof(ignored) / sizeof(int),
				handledCount = sizeof(handled) / sizeof(int);
		int signo;

		Shm->App.Cli = getpid();
		for (signo = SIGRTMIN; signo <= SIGRTMAX; signo++) {
			signal(signo, SIG_IGN);
		}
		for (signo = 0; signo < ignoredCount; signo++) {
			signal(ignored[signo], SIG_IGN);
		}
		for (signo = 0; signo < handledCount; signo++) {
			signal(handled[signo],  Emergency);
		}
	}
}

int main(int argc, char *argv[])
{
	struct stat shmStat = {0};
	int	fd = -1, idx = 0;
	char	*program = strdup(argv[0]),
		*appName = program != NULL ? basename(program) : argv[idx],
		option = 't';

	REASON_INIT(reason);

	LOCALE(IN);

  if ((argc >= 2) && (argv[++idx][0] == '-')) {
	option = argv[idx][1];
  }
  if (option == 'h') {
	REASON_SET(reason, RC_CMD_SYNTAX, 0);
	reason = Help(reason, appName);
  } else if (option == 'v') {
	printf(COREFREQ_VERSION"\n");
  } else if ((fd = shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) !=-1)
  {
    if (fstat(fd, &shmStat) != -1)
    {
      if ((Shm = mmap(NULL, shmStat.st_size,
			PROT_READ|PROT_WRITE, MAP_SHARED,
			fd, 0)) != MAP_FAILED)
      {
       if (CHK_FOOTPRINT(Shm->FootPrint,COREFREQ_MAJOR,
					COREFREQ_MINOR,
					COREFREQ_REV)	)
       {
	ClientFollowService(&localService, &Shm->Proc.Service, 0);

	AggregateRatio();

  #define CONDITION_RDTSCP()						\
	(  (Shm->Proc.Features.AdvPower.EDX.Inv_TSC == 1)		\
	|| (Shm->Proc.Features.ExtInfo.EDX.RDTSCP == 1) )

  #define CONDITION_RDPMC()						\
	(  (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)		\
	&& (Shm->Proc.PM_version >= 1)					\
	&& (BITVAL(Shm->Cpu[Shm->Proc.Service.Core].SystemRegister.CR4, \
							CR4_PCE) == 1) )

	UBENCH_SETUP(CONDITION_RDTSCP(), CONDITION_RDPMC());

	do {
	    switch (option) {
	    case '0' ... '2':
		draw.Unit.Memory = 10 * (option - '0');
		break;
	    case 'F':
		Setting.fahrCels = 1;
		break;
	    case 'J':
		if (++idx < argc) {
			enum SMB_STRING usrIdx = SMB_BOARD_NAME;
			char trailing =  '\0';
			if ((sscanf(argv[idx], "%u%c", &usrIdx, &trailing) != 1)
			 || (usrIdx >= SMB_STRING_COUNT)) {
				goto SYNTAX_ERROR;
			} else {
				draw.SmbIndex = usrIdx;
			}
		}
		break;
	    case 'Y':
		Setting.secret = 0;
		break;
	    case 'B':
		reason = SysInfoSMBIOS(NULL, 80, NULL);
		break;
	    case 'k':
		if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
			reason = SysInfoKernel(NULL, 80, NULL);
		}
		break;
	    case 'u':
		reason = SysInfoCPUID(NULL, 80, NULL);
		break;
	    case 's':
		{
		Window tty = {.matrix.size.wth = 4};

		reason = SysInfoProc(NULL, 80, NULL);
		if (IS_REASON_SUCCESSFUL(reason) == 0) { break; }

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, (char *) &(RSC(ISA_TITLE).CODE()[1]));
		reason = SysInfoISA(&tty, NULL);
		if (IS_REASON_SUCCESSFUL(reason) == 0) { break; }

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, (char *) &(RSC(FEATURES_TITLE).CODE()[1]));
		reason = SysInfoFeatures(NULL, 80, NULL);
		if (IS_REASON_SUCCESSFUL(reason) == 0) { break; }

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, (char *) &(RSC(TECHNOLOGIES_TITLE).CODE()[1]));
		reason = SysInfoTech(NULL, 80, NULL);
		if (IS_REASON_SUCCESSFUL(reason) == 0) { break; }

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, (char *) &(RSC(PERF_MON_TITLE).CODE()[1]));
		reason = SysInfoPerfMon(NULL, 80, NULL);
		if (IS_REASON_SUCCESSFUL(reason) == 0) { break; }

		Print_v1(NULL, NULL, SCANKEY_VOID, NULL, 80, 0, "");
		Print_v1(NULL, NULL, SCANKEY_VOID, NULL,
			80, 0, (char *) &(RSC(POWER_THERMAL_TITLE).CODE()[1]));
		reason = SysInfoPwrThermal(NULL, 80, NULL);
		if (IS_REASON_SUCCESSFUL(reason) == 0) { break; }
		}
		break;
	    case 'j':
		JsonSysInfo(Shm, NULL);
		break;
	    case 'm':
		{
		Window tty = {.matrix.size.wth = 6};
		Topology(&tty, NULL);
		}
		break;
	    case 'M':
	      {
		Window tty = {.matrix.size.wth = MC_MATX};

		switch (Shm->Uncore.Unit.DDR_Ver) {
		case 4:
		    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL) {
			MemoryController(&tty, NULL, Timing_DDR4);
		    }
		  else if ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
			|| (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
		    {
			MemoryController(&tty, NULL, Timing_DDR4_Zen);
		    }
			break;
		case 3:
		case 2:
		default:
			MemoryController(&tty, NULL, Timing_DDR3);
			break;
		}
	      }
		break;
	    case 'R':
		{
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
	    case 'C':
		TrapSignal(1);
		Sensors();
		TrapSignal(0);
		break;
	    case 'V':
		TrapSignal(1);
		Voltage();
		TrapSignal(0);
		break;
	    case 'W':
		TrapSignal(1);
		Power();
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
		if (++idx < argc) {
			struct {
				enum VIEW view;
				char *first;
			} opt[] = {
				{	V_FREQ		, "frequency"	},
				{	V_INST		, "instructions"},
				{	V_CYCLES	, "core"	},
				{	V_CSTATES	, "idle"	},
				{	V_PACKAGE	, "package"	},
				{	V_TASKS 	, "tasks"	},
				{	V_INTR		, "interrupts"	},
				{	V_SENSORS	, "sensors"	},
				{	V_VOLTAGE	, "voltage"	},
				{	V_ENERGY	, "power"	},
				{	V_SLICE 	, "slices"	}
			}, *pOpt, *lOpt = &opt[sizeof(opt)/sizeof(opt[0])];
		    for (pOpt = opt; pOpt < lOpt; pOpt++)
		    {
			const size_t llen = strlen(pOpt->first);
			const size_t rlen = strlen(argv[idx]);
			const ssize_t nlen = llen - rlen;
			if ((nlen >= 0)
			&& (strncmp(pOpt->first, argv[idx], rlen) == 0))
			{
				draw.View = pOpt->view;
				break;
			}
		    }
		    if (pOpt == lOpt) {
			goto SYNTAX_ERROR;
		    }
		}
		TERMINAL(IN);

		TrapSignal(1);
		reason = Top(option);
		TrapSignal(0);

		TERMINAL(OUT);
		break;
	    default:
	    SYNTAX_ERROR:
		{
		REASON_SET(reason, RC_CMD_SYNTAX, 0);
		reason = Help(reason, appName);
		idx = argc;
		}
		break;
	    }
	} while (  (++idx < argc)
		&& (argv[idx][0] == '-')
		&& ((option = argv[idx][1]) != '\0') );

	if (munmap(Shm, shmStat.st_size) == -1) {
		REASON_SET(reason, RC_SHM_MMAP);
		reason = Help(reason, SHM_FILENAME);
	}
       } else {
		char *wrongVersion = malloc(10+5+5+5+1);
		REASON_SET(reason, RC_SHM_MMAP, EPERM);
		if (wrongVersion != NULL) {
			snprintf(wrongVersion, 10+5+5+5+1,
				"Version %hu.%hu.%hu",
				Shm->FootPrint.major,
				Shm->FootPrint.minor,
				Shm->FootPrint.rev);
			reason = Help(reason, wrongVersion);
			free(wrongVersion);
		}
		munmap(Shm, shmStat.st_size);
       }
      } else {
		REASON_SET(reason, RC_SHM_MMAP);
		reason = Help(reason, SHM_FILENAME);
      }
    } else {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, SHM_FILENAME);
    }
    if (close(fd) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, SHM_FILENAME);
    }
  } else {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, SHM_FILENAME);
  }
    LOCALE(OUT);

    if (program != NULL) {
	free(program);
    }
    return (reason.rc);
}
