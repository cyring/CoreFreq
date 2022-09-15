/*
 * CoreFreq
 * Copyright (C) 2015-2022 CYRIL INGENIERIE
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
#include "corefreq-cli-rsc.h"
#include "corefreq-cli.h"
#include "corefreq-cli-json.h"
#include "corefreq-cli-extra.h"

RO(SHM_STRUCT)	*RO(Shm) = NULL;
RW(SHM_STRUCT)	*RW(Shm) = NULL;

static Bit64 Shutdown __attribute__ ((aligned (8))) = 0x0;

SERVICE_PROC localService = RESET_SERVICE;

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
		StrFormat(&ConfigFQN[1], 4095, "%s/.config", homePath);

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
	StrFormat(&ConfigFQN[1], 4095, "%s%s%s/corefreq.cfg",
		homePath, dotted, dirPath);

	ConfigFQN[0] = 1;
    }
	return &ConfigFQN[1];
}

int ClientFollowService(SERVICE_PROC *pSlave, SERVICE_PROC *pMaster, pid_t pid)
{
	if (pSlave->Proc != pMaster->Proc) {
		pSlave->Proc = pMaster->Proc;

		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(pSlave->Core, &cpuset);
		if (pSlave->Thread != -1) {
			const signed int cpu = pSlave->Thread;
			CPU_SET(cpu , &cpuset);
		}
		return sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
	}
	return 0;
}

struct RULER_ST Ruler = {
	.Count = 0
};

#define GetTopOfRuler() (RO(Shm)->Cpu[Ruler.TopOf.Top].Boost[Ruler.TopOf.Boost])

#define SetTopOfRuler(_cpu, _boost)					\
(									\
	Ruler.TopOf = (struct TOPOF) { .Top = _cpu , .Boost = _boost}	\
)

void SetTopOftheTop(	unsigned int cpu, enum RATIO_BOOST rb,
			unsigned int *lowest, unsigned int *highest )
{
    switch (rb) {
    case BOOST(HWP_MIN):
    case BOOST(MIN):
      if(RO(Shm)->Cpu[cpu].Boost[rb] < RO(Shm)->Cpu[ Ruler.Top[rb] ].Boost[rb])
      {
	Ruler.Top[rb] = cpu;
      }
      if (RO(Shm)->Cpu[cpu].Boost[rb] < (*lowest))
      {
	(*lowest) = RO(Shm)->Cpu[cpu].Boost[rb];
	SetTopOfRuler(Ruler.Top[rb], rb);
      }
	break;
    default:
      if(RO(Shm)->Cpu[cpu].Boost[rb] > RO(Shm)->Cpu[ Ruler.Top[rb] ].Boost[rb])
      {
	Ruler.Top[rb] = cpu;
      }
      if (RO(Shm)->Cpu[cpu].Boost[rb] > (*highest))
      {
	(*highest) = RO(Shm)->Cpu[cpu].Boost[rb];
	SetTopOfRuler(Ruler.Top[rb], rb);
      }
	break;
    }
}

void InsertionSortRuler(unsigned int base[],
			unsigned int cnt,
			enum RATIO_BOOST start)
{
	__typeof__(start) lt = start + 1, rt;
	while (lt < cnt)
	{
		rt = lt;
		while ((rt > start) && (base[rt - 1] > base[rt]))
		{
			__typeof__(base[0]) swap = base[rt - 1];
			base[rt - 1] = base[rt];
			base[rt] = swap;
			rt = rt - 1;
		}
		lt = lt + 1;
	}
}

void AggregateRatio(void)
{
	const size_t dimension = sizeof(Ruler.Uniq) / sizeof(Ruler.Uniq[0]);
	const unsigned int highestFactory = MAXCLOCK_TO_RATIO(
		unsigned int, RO(Shm)->Proc.Features.Factory.Clock.Hz
	);
	enum RATIO_BOOST lt, rt;
	unsigned int cpu,
	lowest = RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Boost[BOOST(MAX)],
	highest = RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Boost[BOOST(MIN)];

	Ruler.Count = 0;
	SetTopOfRuler(RO(Shm)->Proc.Service.Core, BOOST(MIN));

	lt = BOOST(MIN);
    while (lt < BOOST(SIZE))
    {
	Ruler.Top[lt] = RO(Shm)->Proc.Service.Core;

	for (cpu = 0;
			!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)
			&& (cpu < RO(Shm)->Proc.CPU.Count)
			&& (Ruler.Count < dimension);
		cpu++)
	{
	    if ((RO(Shm)->Cpu[cpu].Boost[lt] > 0)
	     && (RO(Shm)->Cpu[cpu].Boost[lt] <= highestFactory) )
	    {
		SetTopOftheTop(cpu, lt, &lowest, & highest);

		for (rt = BOOST(MIN); rt < Ruler.Count; rt++)
		{
			if (Ruler.Uniq[rt] == RO(Shm)->Cpu[cpu].Boost[lt])
			{
				break;
			}
		}
		if (rt == Ruler.Count) {
			Ruler.Uniq[Ruler.Count] = RO(Shm)->Cpu[cpu].Boost[lt];
			Ruler.Count++;
		}
	    }
	}
	lt = lt + 1;
    }
	InsertionSortRuler(Ruler.Uniq, Ruler.Count, BOOST(MIN));

	Ruler.Minimum = (double) lowest;
	Ruler.Maximum = (double) highest;
	Ruler.Median  = (double) RO(Shm)->Cpu[
					Ruler.Top[BOOST(ACT)]
				].Boost[BOOST(ACT)];

	if (Ruler.Median == 0.0) {
		Ruler.Median = (Ruler.Minimum + Ruler.Maximum) / 2.0;
	}
}

#undef InsertionSort

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
	return stateAttr;
}

unsigned int Dec2Digit( const unsigned int length, unsigned int decimal,
			unsigned int thisDigit[] )
{
	memset(thisDigit, 0, length * sizeof(unsigned int));
	register unsigned int j = length, dec = decimal;
	while (dec > 0) {
		thisDigit[--j] = dec % 10;
		dec /= 10;
	}
	return length - j;
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
	if (vsnprintf(line, width + 1, fmt, ap) < 0) {
		goto EXIT_v1;
	}
    if (OutFunc == NULL) {
	printf("%s%s%.*s\n", Indent[0][tab], line,
		(int)(width - strlen(line) - strlen(Indent[0][tab])), hSpace);
    } else {
	ASCII *item = malloc(width + 1);
      if (item != NULL) {
	if (0 < StrFormat(item, width + 1, "%s%s%.*s", Indent[1][tab], line,
		(int)(width - strlen(line) - strlen(Indent[1][tab])), hSpace))
	{
		pGrid = OutFunc(win, key, attrib, item);
	}
	free(item);
      }
    }
EXIT_v1:
	va_end(ap);
	free(line);
  }
	return pGrid;
}

TGrid *Print_v2(CELL_FUNC OutFunc,
		Window *win,
		CUINT *nl,
		ATTRIBUTE *attrib, ...)
{
	TGrid *pGrid = NULL;
	ASCII *item = malloc(MIN_WIDTH);
    if (item != NULL)
    {
	char *fmt;
	va_list ap;
	va_start(ap, attrib);
	if ((fmt = va_arg(ap, char*)) != NULL)
	{
		if (vsnprintf((char*) item, MIN_WIDTH, fmt, ap) < 0) {
			goto EXIT_v2;
		}
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
EXIT_v2:
	va_end(ap);
	free(item);
    }
	return pGrid;
}

TGrid *Print_v3(CELL_FUNC OutFunc,
		Window *win,
		CUINT *nl,
		ATTRIBUTE *attrib, ...)
{
	TGrid *pGrid = NULL;
	ASCII *item = malloc(MIN_WIDTH);
    if (item != NULL)
    {
	char *fmt;
	va_list ap;
	va_start(ap, attrib);
	if ((fmt = va_arg(ap, char*)) != NULL)
	{
		if (!(vsnprintf((char*) item, MIN_WIDTH, fmt, ap) < 0))
		{
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
	}
	va_end(ap);
	free(item);
    }
	return pGrid;
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
	for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++) {
	    if (OutFunc == NULL) {
		PUT(SCANKEY_NULL, attrib[0], width, 0,
			"CPU #%-3u function"				\
			"         EAX          EBX          ECX          EDX",
			cpu);
	    } else {
		PUT(SCANKEY_NULL,
			attrib[BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)],
			width, 0, "CPU #%-3u", cpu);
	    }
	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)) {
		PUT(SCANKEY_NULL, attrib[3], width, 2, format,
			0x00000000, 0x00000000,
			4, hSpace,
			RO(Shm)->Cpu[cpu].Query.StdFunc.LargestStdFunc,
			RO(Shm)->Cpu[cpu].Query.StdFunc.BX,
			RO(Shm)->Cpu[cpu].Query.StdFunc.CX,
			RO(Shm)->Cpu[cpu].Query.StdFunc.DX);

		PUT(SCANKEY_NULL, attrib[2], width, 3,
			"%.*s""=%08x",
			RSZ(LARGEST_STD_FUNC), RSC(LARGEST_STD_FUNC).CODE(),
			RO(Shm)->Cpu[cpu].Query.StdFunc.LargestStdFunc);

		PUT(SCANKEY_NULL, attrib[3], width, 2, format,
			0x80000000, 0x00000000,
			4, hSpace,
			RO(Shm)->Cpu[cpu].Query.ExtFunc.LargestExtFunc,
			RO(Shm)->Cpu[cpu].Query.ExtFunc.EBX,
			RO(Shm)->Cpu[cpu].Query.ExtFunc.ECX,
			RO(Shm)->Cpu[cpu].Query.ExtFunc.EDX);

		PUT(SCANKEY_NULL, attrib[2], width, 3,
			"%.*s""=%08x",
			RSZ(LARGEST_EXT_FUNC), RSC(LARGEST_EXT_FUNC).CODE(),
			RO(Shm)->Cpu[cpu].Query.ExtFunc.LargestExtFunc);

		enum CPUID_ENUM i;
		for (i = 0; i < CPUID_MAX_FUNC; i++) {
		    if (RO(Shm)->Cpu[cpu].CpuID[i].func) {
			PUT(SCANKEY_NULL, attrib[3], width, 2,
				format,
				RO(Shm)->Cpu[cpu].CpuID[i].func,
				RO(Shm)->Cpu[cpu].CpuID[i].sub,
				4, hSpace,
				RO(Shm)->Cpu[cpu].CpuID[i].reg[0],
				RO(Shm)->Cpu[cpu].CpuID[i].reg[1],
				RO(Shm)->Cpu[cpu].CpuID[i].reg[2],
				RO(Shm)->Cpu[cpu].CpuID[i].reg[3]);
		    }
		}
	    }
	}
	return reason;
}

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
	enum AUTOMAT {
		DO_END, DO_SPC, DO_CPU, DO_FLAG,
		DO_CR0, DO_CR3, DO_CR4, DO_CR8,
		DO_EFCR, DO_EFER
	};
	const struct SR_ST {
		struct SR_HDR {
			const ASCII	*flag,
					*comm;
		} *header;
		struct SR_BIT {
			enum AUTOMAT	automat;
			unsigned int	*CRC;
			enum SYS_REG	pos;
			unsigned int	len;
		} *flag;
	} SR[] = \
    {
      {
	.header = (struct SR_HDR[]) {
	[ 0] =	{RSC(SYS_REGS_HDR_CPU).CODE(),	NULL},
	[ 1] =	{RSC(SYS_REG_HDR_FLAGS).CODE(), NULL},
	[ 2] =	{RSC(SYS_REG_HDR_TF).CODE(),	RSC(SYS_REG_FLAGS_TF).CODE()},
	[ 3] =	{RSC(SYS_REG_HDR_IF).CODE(),	RSC(SYS_REG_FLAGS_IF).CODE()},
	[ 4] =	{RSC(SYS_REG_HDR_IOPL).CODE(),	RSC(SYS_REG_FLAGS_IOPL).CODE()},
	[ 5] =	{RSC(SYS_REG_HDR_NT).CODE(),	RSC(SYS_REG_FLAGS_NT).CODE()},
	[ 6] =	{RSC(SYS_REG_HDR_RF).CODE(),	RSC(SYS_REG_FLAGS_RF).CODE()},
	[ 7] =	{RSC(SYS_REG_HDR_VM).CODE(),	RSC(SYS_REG_FLAGS_VM).CODE()},
	[ 8] =	{RSC(SYS_REG_HDR_AC).CODE(),	RSC(SYS_REG_FLAGS_AC).CODE()},
	[ 9] =	{RSC(SYS_REG_HDR_VIF).CODE(),	RSC(SYS_REG_FLAGS_VIF).CODE()},
	[10] =	{RSC(SYS_REG_HDR_VIP).CODE(),	RSC(SYS_REG_FLAGS_VIP).CODE()},
	[11] =	{RSC(SYS_REG_HDR_ID).CODE(),	RSC(SYS_REG_FLAGS_ID).CODE()},
	[12] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[13] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[14] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[15] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[16] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
		{NULL, NULL}
	},
	.flag = (struct SR_BIT[]) {
	[ 0] =	{DO_CPU , NULL	, UNDEF_CR	, 0	},
	[ 1] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[ 2] =	{DO_FLAG, NULL	, RFLAG_TF	, 1	},
	[ 3] =	{DO_FLAG, NULL	, RFLAG_IF	, 1	},
	[ 4] =	{DO_FLAG, NULL	, RFLAG_IOPL	, 2	},
	[ 5] =	{DO_FLAG, NULL	, RFLAG_NT	, 1	},
	[ 6] =	{DO_FLAG, NULL	, RFLAG_RF	, 1	},
	[ 7] =	{DO_FLAG, NULL	, RFLAG_VM	, 1	},
	[ 8] =	{DO_FLAG, NULL	, RFLAG_AC	, 1	},
	[ 9] =	{DO_FLAG, NULL	, RFLAG_VIF	, 1	},
	[10] =	{DO_FLAG, NULL	, RFLAG_VIP	, 1	},
	[11] =	{DO_FLAG, NULL	, RFLAG_ID	, 1	},
	[12] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[13] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[14] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[15] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[16] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
		{DO_END , NULL	, UNDEF_CR	, 0	}
	}
      },
      {
	.header = (struct SR_HDR[]) {
	[ 0] =	{RSC(SYS_REG_HDR_CR0).CODE(),	RSC(SYS_REGS_CR0).CODE()},
	[ 1] =	{RSC(SYS_REG_HDR_CR0_PE).CODE(), RSC(SYS_REG_CR0_PE).CODE()},
	[ 2] =	{RSC(SYS_REG_HDR_CR0_MP).CODE(), RSC(SYS_REG_CR0_MP).CODE()},
	[ 3] =	{RSC(SYS_REG_HDR_CR0_EM).CODE(), RSC(SYS_REG_CR0_EM).CODE()},
	[ 4] =	{RSC(SYS_REG_HDR_CR0_TS).CODE(), RSC(SYS_REG_CR0_TS).CODE()},
	[ 5] =	{RSC(SYS_REG_HDR_CR0_ET).CODE(), RSC(SYS_REG_CR0_ET).CODE()},
	[ 6] =	{RSC(SYS_REG_HDR_CR0_NE).CODE(), RSC(SYS_REG_CR0_NE).CODE()},
	[ 7] =	{RSC(SYS_REG_HDR_CR0_WP).CODE(), RSC(SYS_REG_CR0_WP).CODE()},
	[ 8] =	{RSC(SYS_REG_HDR_CR0_AM).CODE(), RSC(SYS_REG_CR0_AM).CODE()},
	[ 9] =	{RSC(SYS_REG_HDR_CR0_NW).CODE(), RSC(SYS_REG_CR0_NW).CODE()},
	[10] =	{RSC(SYS_REG_HDR_CR0_CD).CODE(), RSC(SYS_REG_CR0_CD).CODE()},
	[11] =	{RSC(SYS_REG_HDR_CR0_PG).CODE(), RSC(SYS_REG_CR0_PG).CODE()},
	[12] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[13] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[14] =	{RSC(SYS_REG_HDR_CR3).CODE(), RSC(SYS_REGS_CR3).CODE()},
	[15] =	{RSC(SYS_REG_HDR_CR3_PWT).CODE(), RSC(SYS_REG_CR3_PWT).CODE()},
	[16] =	{RSC(SYS_REG_HDR_CR3_PCD).CODE(), RSC(SYS_REG_CR3_PCD).CODE()},
		{NULL, NULL}
	},
	.flag = (struct SR_BIT[]) {
	[ 0] =	{DO_CPU , NULL	, UNDEF_CR	, 0	},
	[ 1] =  {DO_CR0 , NULL	, CR0_PE	, 1	},
	[ 2] =	{DO_CR0 , NULL	, CR0_MP	, 1	},
	[ 3] =	{DO_CR0 , NULL	, CR0_EM	, 1	},
	[ 4] =	{DO_CR0 , NULL	, CR0_TS	, 1	},
	[ 5] =	{DO_CR0 , NULL	, CR0_ET	, 1	},
	[ 6] =	{DO_CR0 , NULL	, CR0_NE	, 1	},
	[ 7] =	{DO_CR0 , NULL	, CR0_WP	, 1	},
	[ 8] =	{DO_CR0 , NULL	, CR0_AM	, 1	},
	[ 9] =	{DO_CR0 , NULL	, CR0_NW	, 1	},
	[10] =	{DO_CR0 , NULL	, CR0_CD	, 1	},
	[11] =	{DO_CR0 , NULL	, CR0_PG	, 1	},
	[12] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[13] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[14] =	{DO_SPC , NULL	, UNDEF_CR	, 4	},
	[15] =	{DO_CR3 , NULL	, CR3_PWT	, 1	},
	[16] =	{DO_CR3 , NULL	, CR3_PCD	, 1	},
		{DO_END , NULL	, UNDEF_CR	, 0	}
	}
      },
      {
	.header = (struct SR_HDR[]) {
	[ 0] =	{RSC(SYS_REG_HDR_CR4).CODE(),	RSC(SYS_REGS_CR4).CODE()},
	[ 1] =	{RSC(SYS_REG_HDR_CR4_VME).CODE(),RSC(SYS_REG_CR4_VME).CODE()},
	[ 2] =	{RSC(SYS_REG_HDR_CR4_PVI).CODE(),RSC(SYS_REG_CR4_PVI).CODE()},
	[ 3] =	{RSC(SYS_REG_HDR_CR4_TSD).CODE(),RSC(SYS_REG_CR4_TSD).CODE()},
	[ 4] =	{RSC(SYS_REG_HDR_CR4_DE).CODE(), RSC(SYS_REG_CR4_DE).CODE()},
	[ 5] =	{RSC(SYS_REG_HDR_CR4_PSE).CODE(),RSC(SYS_REG_CR4_PSE).CODE()},
	[ 6] =	{RSC(SYS_REG_HDR_CR4_PAE).CODE(),RSC(SYS_REG_CR4_PAE).CODE()},
	[ 7] =	{RSC(SYS_REG_HDR_CR4_MCE).CODE(),RSC(SYS_REG_CR4_MCE).CODE()},
	[ 8] =	{RSC(SYS_REG_HDR_CR4_PGE).CODE(),RSC(SYS_REG_CR4_PGE).CODE()},
	[ 9] =	{RSC(SYS_REG_HDR_CR4_PCE).CODE(),RSC(SYS_REG_CR4_PCE).CODE()},
	[10] =	{RSC(SYS_REG_HDR_CR4_FX).CODE(), RSC(SYS_REG_CR4_FX).CODE()},
	[11] =	{RSC(SYS_REG_HDR_CR4_XMM).CODE(),RSC(SYS_REG_CR4_XMM).CODE()},
	[12] =	{RSC(SYS_REG_HDR_CR4_UMIP).CODE(),RSC(SYS_REG_CR4_UMIP).CODE()},
	[13] =	{RSC(SYS_REG_HDR_CR4_5LP).CODE(),RSC(SYS_REG_CR4_5LP).CODE()},
	[14] =	{RSC(SYS_REG_HDR_CR4_VMX).CODE(),RSC(SYS_REG_CR4_VMX).CODE()},
	[15] =	{RSC(SYS_REG_HDR_CR4_SMX).CODE(),RSC(SYS_REG_CR4_SMX).CODE()},
	[16] =	{RSC(SYS_REG_HDR_CR4_FS).CODE(), RSC(SYS_REG_CR4_FS).CODE()},
		{NULL, NULL}
	},
	.flag = (struct SR_BIT[]) {
	[ 0] =	{DO_CPU , NULL	, UNDEF_CR	, 0	},
	[ 1] =	{DO_CR4 , NULL	, CR4_VME	, 1	},
	[ 2] =	{DO_CR4 , NULL	, CR4_PVI	, 1	},
	[ 3] =	{DO_CR4 , NULL	, CR4_TSD	, 1	},
	[ 4] =	{DO_CR4 , NULL	, CR4_DE	, 1	},
	[ 5] =	{DO_CR4 , NULL	, CR4_PSE	, 1	},
	[ 6] =	{DO_CR4 , NULL	, CR4_PAE	, 1	},
	[ 7] =	{DO_CR4 , NULL	, CR4_MCE	, 1	},
	[ 8] =	{DO_CR4 , NULL	, CR4_PGE	, 1	},
	[ 9] =	{DO_CR4 , NULL	, CR4_PCE	, 1	},
	[10] =	{DO_CR4 , NULL	, CR4_OSFXSR	, 1	},
	[11] =	{DO_CR4 , NULL	, CR4_OSXMMEXCPT, 1	},
	[12] =	{DO_CR4 , NULL	, CR4_UMIP	, 1	},
	[13] =	{DO_CR4 , NULL	, CR4_LA57	, 1	},
	[14] =	{DO_CR4 , NULL	, CR4_VMXE	, 1	},
	[15] =	{DO_CR4 , NULL	, CR4_SMXE	, 1	},
	[16] =	{DO_CR4 , NULL	, CR4_FSGSBASE	, 1	},
		{DO_END , NULL	, UNDEF_CR	, 0	}
	}
      },
      {
	.header = (struct SR_HDR[]) {
	[ 0] =	{RSC(SYS_REG_HDR_CR4).CODE(),	RSC(SYS_REGS_CR4).CODE()},
	[ 1] =	{RSC(SYS_REG_HDR_CR4_PCID).CODE(),RSC(SYS_REG_CR4_PCID).CODE()},
	[ 2] =	{RSC(SYS_REG_HDR_CR4_SAV).CODE(),RSC(SYS_REG_CR4_SAV).CODE()},
	[ 3] =	{RSC(SYS_REG_HDR_CR4_KL).CODE(), RSC(SYS_REG_CR4_KL).CODE()},
	[ 4] =	{RSC(SYS_REG_HDR_CR4_SME).CODE(),RSC(SYS_REG_CR4_SME).CODE()},
	[ 5] =	{RSC(SYS_REG_HDR_CR4_SMA).CODE(),RSC(SYS_REG_CR4_SMA).CODE()},
	[ 6] =	{RSC(SYS_REG_HDR_CR4_PKE).CODE(),RSC(SYS_REG_CR4_PKE).CODE()},
	[ 7] =	{RSC(SYS_REG_HDR_CR4_CET).CODE(),RSC(SYS_REG_CR4_CET).CODE()},
	[ 8] =	{RSC(SYS_REG_HDR_CR4_PKS).CODE(),RSC(SYS_REG_CR4_PKS).CODE()},
	[ 9] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[10] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[11] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[12] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[13] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[14] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[15] =	{RSC(SYS_REG_HDR_CR8).CODE(), RSC(SYS_REGS_CR8).CODE()	},
	[16] =	{RSC(SYS_REG_HDR_CR8_TPL).CODE(), RSC(SYS_REG_CR8_TPL).CODE()},
		{NULL, NULL}
	},
	.flag = (struct SR_BIT[]) {
	[ 0] =	{DO_CPU , NULL	, UNDEF_CR	, 0	},
	[ 1] =	{DO_CR4 , NULL	, CR4_PCIDE	, 1	},
	[ 2] =	{DO_CR4 , NULL	, CR4_OSXSAVE	, 1	},
	[ 3] =	{DO_CR4 , NULL	, CR4_KL	, 1	},
	[ 4] =	{DO_CR4 , NULL	, CR4_SMEP	, 1	},
	[ 5] =	{DO_CR4 , NULL	, CR4_SMAP	, 1	},
	[ 6] =	{DO_CR4 , NULL	, CR4_PKE	, 1	},
	[ 7] =	{DO_CR4 , NULL	, CR4_CET	, 1	},
	[ 8] =	{DO_CR4 , NULL	, CR4_PKS	, 1	},
	[ 9] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[10] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[11] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[12] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[13] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[14] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[15] =	{DO_SPC , NULL	, UNDEF_CR	, 0	},
	[16] =	{DO_CR8 , NULL	, CR8_TPL	, 4	},
		{DO_END , NULL	, UNDEF_CR	, 0	}
	}
      },
      {
	.header = (struct SR_HDR[]) {
	[ 0] =	{RSC(SYS_REG_HDR_EFCR).CODE(), RSC(SYS_REGS_EFCR).CODE()},
	[ 1] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[ 2] =	{RSC(SYS_REG_HDR_EFCR_LCK).CODE(),RSC(SYS_REG_EFCR_LCK).CODE()},
	[ 3] =	{RSC(SYS_REG_HDR_EFCR_VMX).CODE(),RSC(SYS_REG_EFCR_VMX).CODE()},
	[ 4] =	{RSC(SYS_REG_HDR_EFCR_SGX).CODE(),RSC(SYS_REG_EFCR_SGX).CODE()},
	[ 5] =	{RSC(SYS_REG_HDR_EFCR_LSE).CODE(),RSC(SYS_REG_EFCR_LSE).CODE()},
	[ 6] =	{RSC(SYS_REG_HDR_EFCR_GSE).CODE(),RSC(SYS_REG_EFCR_GSE).CODE()},
	[ 7] ={RSC(SYS_REG_HDR_EFCR_LSGX).CODE(),RSC(SYS_REG_EFCR_LSGX).CODE()},
	[ 8] ={RSC(SYS_REG_HDR_EFCR_GSGX).CODE(),RSC(SYS_REG_EFCR_GSGX).CODE()},
	[ 9] =	{RSC(SYS_REG_HDR_EFCR_LMC).CODE(),RSC(SYS_REG_EFCR_LMC).CODE()},
	[10] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[11] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[12] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[13] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[14] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[15] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[16] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
		{NULL, NULL}
	},
	.flag = (struct SR_BIT[]) {
	[ 0] =	{DO_CPU , NULL	, UNDEF_CR		, 0		},
	[ 1] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[ 2] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_LOCK,	1},
	[ 3] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_VMX_IN_SMX, 1},
	[ 4] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_VMXOUT_SMX, 1},
	[ 5] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_SENTER_LEN, 6},
	[ 6] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_SENTER_GEN, 1},
	[ 7] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_SGX_LCE, 1},
	[ 8] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_SGX_GEN, 1},
	[ 9] =	{DO_EFCR,(unsigned int[]) {CRC_INTEL, 0}, EXFCR_LMCE,	1},
	[10] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[11] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[12] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[13] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[14] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[15] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[16] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
		{DO_END , NULL	, UNDEF_CR		, 0		}
	}
      },
      {
	.header = (struct SR_HDR[]) {
	[ 0] =	{RSC(SYS_REG_HDR_EFER).CODE(),	RSC(SYS_REGS_EFER).CODE()},
	[ 1] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[ 2] =	{RSC(SYS_REG_HDR_EFER_SCE).CODE(),RSC(SYS_REG_EFER_SCE).CODE()},
	[ 3] =	{RSC(SYS_REG_HDR_EFER_LME).CODE(),RSC(SYS_REG_EFER_LME).CODE()},
	[ 4] =	{RSC(SYS_REG_HDR_EFER_LMA).CODE(),RSC(SYS_REG_EFER_LMA).CODE()},
	[ 5] =	{RSC(SYS_REG_HDR_EFER_NXE).CODE(),RSC(SYS_REG_EFER_NXE).CODE()},
	[ 6] =	{RSC(SYS_REG_HDR_EFER_SVM).CODE(),RSC(SYS_REG_EFER_SVM).CODE()},
	[ 7] =	{RSC(SYS_REG_HDR_EFER_LMS).CODE(),RSC(SYS_REG_EFER_LMS).CODE()},
	[ 8] =	{RSC(SYS_REG_HDR_EFER_FFX).CODE(),RSC(SYS_REG_EFER_FFX).CODE()},
	[ 9] =	{RSC(SYS_REG_HDR_EFER_TCE).CODE(),RSC(SYS_REG_EFER_TCE).CODE()},
	[10] =	{RSC(SYS_REG_HDR_EFER_MCM).CODE(),RSC(SYS_REG_EFER_MCM).CODE()},
	[11] =	{RSC(SYS_REG_HDR_EFER_WBI).CODE(),RSC(SYS_REG_EFER_WBI).CODE()},
	[12] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[13] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[14] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[15] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
	[16] =	{RSC(SYS_REGS_SPACE).CODE(),	NULL},
		{NULL, NULL}
	},
	.flag = (struct SR_BIT[]) {
	[ 0] =	{DO_CPU , NULL	, UNDEF_CR		, 0		},
	[ 1] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[ 2] =	{DO_EFER, NULL	, EXFER_SCE		, 1		},
	[ 3] =	{DO_EFER, NULL	, EXFER_LME		, 1		},
	[ 4] =	{DO_EFER, NULL	, EXFER_LMA		, 1		},
	[ 5] =	{DO_EFER, NULL	, EXFER_NXE		, 1		},
	[ 6] =	{DO_EFER, NULL	, EXFER_SVME		, 1		},
	[ 7] =	{DO_EFER, NULL	, EXFER_LMSLE		, 1		},
	[ 8] =	{DO_EFER, NULL	, EXFER_FFXSE		, 1		},
	[ 9] =	{DO_EFER, NULL	, EXFER_TCE		, 1		},
	[10] =	{DO_EFER, NULL	, EXFER_MCOMMIT 	, 1		},
	[11] =	{DO_EFER, NULL	, EXFER_INT_WBINVD	, 1		},
	[12] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[13] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[14] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[15] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
	[16] =	{DO_SPC , NULL	, UNDEF_CR		, 0		},
		{DO_END , NULL	, UNDEF_CR		, 0		}
	}
      }
    };

	CUINT cells_per_line = win->matrix.size.wth, *nl = &cells_per_line;

	size_t idx;
  for (idx = 0; idx < sizeof(SR) / sizeof(struct SR_ST); idx++)
  {
	struct SR_HDR *pHdr;
	for (pHdr = SR[idx].header; pHdr->flag != NULL; pHdr++)
	{
		GridHover(	PRT(REG, attrib[0], "%s", pHdr->flag),
				(char *) pHdr->comm );
	}
	unsigned int cpu;
    for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++)
    {
	struct SR_BIT *pFlag;
      for (pFlag = SR[idx].flag; pFlag->automat != DO_END; pFlag++)
      {
	switch (pFlag->automat) {
	case DO_END:
	case DO_SPC:
		PRT(REG, attrib[0], RSC(SYS_REGS_SPACE).CODE());
		break;
	case DO_CPU:
		PRT(REG,attrib[BITVAL(RO(Shm)->Cpu[cpu].OffLine,OS) ? 4:3],
			"#%-2u ", cpu);
		break;
	default:
	    {
		unsigned short capable = 0;
		if (pFlag->CRC == NULL) {
			capable = 1;
		}
		else
		{
			unsigned int *CRC;
		    for (CRC = pFlag->CRC;
			(*CRC) != 0 && capable == 0; CRC++)
		    {
			if((*CRC) == RO(Shm)->Proc.Features.Info.Vendor.CRC)
			{
				capable = 1;
			}
		    }
		}
		if ((capable) && !BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
		{
		    switch (pFlag->automat) {
		    case DO_FLAG:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.RFLAGS,
					pFlag->pos, pFlag->len));
			break;
		    case DO_CR0:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.CR0,
					pFlag->pos, pFlag->len));
			break;
		    case DO_CR3:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.CR3,
					pFlag->pos, pFlag->len));
			break;
		    case DO_CR4:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.CR4,
					pFlag->pos, pFlag->len));
			break;
		    case DO_CR8:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.CR8,
					pFlag->pos, pFlag->len));
			break;
		    case DO_EFCR:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.EFCR,
					pFlag->pos, pFlag->len));
			break;
		    case DO_EFER:
			PRT(REG, attrib[2], "%3llx ",
			  BITEXTRZ(RO(Shm)->Cpu[cpu].SystemRegister.EFER,
					pFlag->pos, pFlag->len));
			break;
		    default:
			PRT(REG, attrib[1], RSC(SYS_REGS_NA).CODE());
			break;
		    }
		} else {
			PRT(REG, attrib[1], RSC(SYS_REGS_NA).CODE());
		}
	    }
		break;
	}
      }
    }
  }
	return reason;
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
	return pGrid;
}

void RefreshBaseClock(TGrid *grid, DATA_TYPE data[])
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
			!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
		];
	char item[8+1];
	UNUSED(data);

	StrFormat(item, 8+1, "%7.3f", CLOCK_MHz(double, CFlop->Clock.Hz));

	memcpy(&grid->cell.item[grid->cell.length - 9], item, 7);
}

void RefreshFactoryClock(TGrid *grid, DATA_TYPE data[])
{
	char item[8+1];
	UNUSED(data);

	StrFormat(item, 8+1, "%7.3f",
		CLOCK_MHz(double, RO(Shm)->Proc.Features.Factory.Clock.Hz));

	memcpy(&grid->cell.item[grid->cell.length - 9], item, 7);
}

void RefreshFactoryFreq(TGrid *grid, DATA_TYPE data[])
{
	char item[11+11+1];
	UNUSED(data);

	StrFormat(item, 11+11+1, "%5u" "%4u",
		RO(Shm)->Proc.Features.Factory.Freq,
		RO(Shm)->Proc.Features.Factory.Ratio);

	memcpy(&grid->cell.item[22], &item[0], 5);
	memcpy(&grid->cell.item[51], &item[5], 4);
}

void RefreshItemFreq(TGrid *grid, unsigned int ratio, double Freq_MHz)
{
	char item[11+8+1];

    if ((Freq_MHz > 0.0) && (Freq_MHz < CLOCK_MHz(double, UNIT_GHz(10.0)))) {
	StrFormat(item,11+8+1, "%4u%7.2f", ratio, Freq_MHz);
    } else {
	StrFormat(item,11+7+1, "%4u%7s", ratio, RSC(AUTOMATIC).CODE());
    }
	memcpy(&grid->cell.item[23], &item[4], 7);
	memcpy(&grid->cell.item[51], &item[0], 4);
}

void RefreshRatioFreq(TGrid *grid, DATA_TYPE data[])
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
			!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
		];
	RefreshItemFreq(grid,
			(*data[0].puint),
			ABS_FREQ_MHz(double, (*data[0].puint), CFlop->Clock));
}

void RefreshTopFreq(TGrid *grid, DATA_TYPE data[])
{
	enum RATIO_BOOST boost = data[0].uint[0];
	unsigned int top = Ruler.Top[boost];
	unsigned int ratio = RO(Shm)->Cpu[top].Boost[boost];

	struct FLIP_FLOP *CFlop = &RO(Shm)->Cpu[top].FlipFlop[
					!RO(Shm)->Cpu[top].Toggle
				];
	RefreshItemFreq(grid, ratio, ABS_FREQ_MHz(double, ratio, CFlop->Clock));
}

void RefreshHybridFreq(TGrid *grid, DATA_TYPE data[])
{
	enum RATIO_BOOST boost = data[0].uint[0];
	unsigned int ratio = RO(Shm)->Cpu[
					RO(Shm)->Proc.Service.Hybrid
				].Boost[boost];

	struct FLIP_FLOP *CFlop = &RO(Shm)->Cpu[
					RO(Shm)->Proc.Service.Hybrid
				].FlipFlop[
					!RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Hybrid
					].Toggle
				];
	RefreshItemFreq(grid, ratio, ABS_FREQ_MHz(double, ratio, CFlop->Clock));
}

void RefreshConfigTDP(TGrid *grid, DATA_TYPE data[])
{
	char item[11+11+1];
	UNUSED(data);

	StrFormat(item, 11+11+1, "%3d:%-3d",
		RO(Shm)->Proc.Features.TDP_Cfg_Level,
		RO(Shm)->Proc.Features.TDP_Levels);

	memcpy(&grid->cell.item[grid->cell.length - 9], item, 7);
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
	CLOCK_ARG coreClock = {.NC = 0, .Offset = 0};
	unsigned int activeCores;
	enum RATIO_BOOST boost = 0;

	PUT(	SCANKEY_NULL, attrib[0], width, 0,
		"%s""%.*s[%s]", RSC(PROCESSOR).CODE(),
		width - 2 - RSZ(PROCESSOR) - (int) strlen(RO(Shm)->Proc.Brand),
		hSpace, RO(Shm)->Proc.Brand );

    if (RO(Shm)->Proc.Features.Factory.PPIN > 0)
    {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%16llx]", RSC(PPIN).CODE(),
		width - 21 - RSZ(PPIN),
		hSpace, RO(Shm)->Proc.Features.Factory.PPIN );
    }
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%s]", RSC(ARCHITECTURE).CODE(),
		width - 5 - RSZ(ARCHITECTURE)
		- (int) strlen(RO(Shm)->Proc.Architecture),
		hSpace, RO(Shm)->Proc.Architecture );

	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[%s]", RSC(VENDOR_ID).CODE(),
		width - 5 - RSZ(VENDOR_ID)
		- (int) strlen(RO(Shm)->Proc.Features.Info.Vendor.ID),
		hSpace, RO(Shm)->Proc.Features.Info.Vendor.ID );

    if (RO(Shm)->Proc.Features.Factory.SMU.Version > 0)
    {
	char version[17+1];
	int len;
	StrLenFormat(len, version, 17+1, "[%3u.%u.%u-%u]",
			RO(Shm)->Proc.Features.Factory.SMU.Major,
			RO(Shm)->Proc.Features.Factory.SMU.Minor,
			RO(Shm)->Proc.Features.Factory.SMU.Revision,
			RO(Shm)->Proc.Features.Factory.SMU.Interface);

	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s%s", RSC(FIRMWARE).CODE(),
		width - 3 - RSZ(FIRMWARE) - len , hSpace, version );
    }
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s[0x%08x]", RSC(MICROCODE).CODE(),
		width - 15 - RSZ(MICROCODE), hSpace,
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.Microcode );

	PUT(	SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%3X%1X_%1X%1X]", RSC(SIGNATURE).CODE(),
		width - 12 - RSZ(SIGNATURE), hSpace,
		RO(Shm)->Proc.Features.Std.EAX.ExtFamily,
		RO(Shm)->Proc.Features.Std.EAX.Family,
		RO(Shm)->Proc.Features.Std.EAX.ExtModel,
		RO(Shm)->Proc.Features.Std.EAX.Model );

	PUT(	SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%7u]", RSC(STEPPING).CODE(),
		width - 12 - RSZ(STEPPING), hSpace,
		RO(Shm)->Proc.Features.Std.EAX.Stepping );

	PUT(	SCANKEY_NULL, attrib[2], width, 2,
		"%s""%.*s[%3u/%3u]", RSC(ONLINE_CPU).CODE(),
		width - 12 - RSZ(ONLINE_CPU), hSpace,
		RO(Shm)->Proc.CPU.OnLine, RO(Shm)->Proc.CPU.Count );

	CFlop = &RO(Shm)->Cpu[
			RO(Shm)->Proc.Service.Core
		].FlipFlop[
			!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
		];

	GridCall( PUT(	SCANKEY_NULL, attrib[2], width, 2,
			"%s""%.*s[%7.3f]", RSC(BASE_CLOCK).CODE(),
			width - 12 - RSZ(BASE_CLOCK), hSpace,
			CLOCK_MHz(double, CFlop->Clock.Hz) ),
		RefreshBaseClock );

	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s""%.*s%s%.*s""%s", RSC(FREQUENCY).CODE(),
		21 - RSZ(FREQUENCY), hSpace,
		RSC(FREQ_UNIT_MHZ).CODE(),
		23 - (OutFunc == NULL), hSpace,
		RSC(RATIO).CODE() );

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_MIN;

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(MIN) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(MIN) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) RSC(MIN).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(MIN) ]
				].Boost[ BOOST(MIN) ],
				1, coreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(MIN) );

	coreClock = (CLOCK_ARG) {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_MAX;

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(MAX) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(MAX) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) RSC(MAX).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(MAX) ]
				].Boost[ BOOST(MAX) ],
				1, coreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(MAX) );

	GridCall( PUT(	SCANKEY_NULL, attrib[0], width, 2,
			"%s""%.*s[%7.3f]", RSC(FACTORY).CODE(),
			(OutFunc == NULL ? 68 : 64) - RSZ(FACTORY), hSpace,
		    CLOCK_MHz(double,RO(Shm)->Proc.Features.Factory.Clock.Hz) ),
		RefreshFactoryClock );

	GridCall( PUT(	SCANKEY_NULL, attrib[3], width, 0,
			"%.*s""%5u""%.*s""[%4d ]",
			22, hSpace, RO(Shm)->Proc.Features.Factory.Freq,
			23, hSpace, RO(Shm)->Proc.Features.Factory.Ratio ),
		RefreshFactoryFreq );

	PUT(SCANKEY_NULL, attrib[0], width, 2, "%s", RSC(PERFORMANCE).CODE());

	PUT(SCANKEY_NULL, attrib[0], width, 3, "%s", RSC(PSTATE).CODE());

	coreClock = (CLOCK_ARG) {.NC = 0, .Offset = 0};

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_TGT;

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(TGT) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(TGT) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				1, (char*) RSC(TGT).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(TGT) ]
				].Boost[ BOOST(TGT) ],
				1, coreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TGT) );

    if ((RO(Shm)->Proc.Features.HWP_Enable == 1)
     || (RO(Shm)->Proc.Features.ACPI_CPPC == 1))
    {
	coreClock = (CLOCK_ARG) {.NC = 0, .Offset = 0};

      if ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	|| (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
      {
	PUT(SCANKEY_NULL, attrib[0], width, 3, "%s", RSC(CPPC).CODE());
      } else {
	PUT(SCANKEY_NULL, attrib[0], width, 3, "%s", RSC(HWP).CODE());
      }

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_HWP_MIN;

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(HWP_MIN) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(HWP_MIN) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				1, (char*) RSC(MIN).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(HWP_MIN) ]
				].Boost[ BOOST(HWP_MIN) ],
				1, coreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(HWP_MIN) );

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_HWP_MAX;

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(HWP_MAX) ]
		].FlipFlop[
			!RO(Shm)->Cpu[Ruler.Top[ BOOST(HWP_MAX) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				1, (char*) RSC(MAX).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(HWP_MAX) ]
				].Boost[ BOOST(HWP_MAX) ],
				1, coreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(HWP_MAX) );

	coreClock.NC = BOXKEY_RATIO_CLOCK_OR | CLOCK_MOD_HWP_TGT;

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(HWP_TGT) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(HWP_TGT) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				1, (char*) RSC(TGT).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(HWP_TGT) ]
				].Boost[ BOOST(HWP_TGT) ],
				1, coreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(HWP_TGT) );
    }
	PUT(	SCANKEY_NULL, attrib[RO(Shm)->Proc.Features.Turbo_Unlock],
		width, 2, "%s%.*s[%7.*s]", RSC(BOOST).CODE(),
		width - 12 - RSZ(BOOST), hSpace, 6,
		RO(Shm)->Proc.Features.Turbo_Unlock ?
			RSC(UNLOCK).CODE() : RSC(LOCK).CODE() );

    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
     || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
      if (RO(Shm)->Proc.Features.XtraCOF >= 2)
      {
	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(XFR) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(XFR) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) RSC(XFR).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(XFR) ]
				].Boost[ BOOST(XFR) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(XFR) );
      }
      if (RO(Shm)->Proc.Features.XtraCOF >= 1)
      {
	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(CPB) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(CPB) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) RSC(CPB).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(CPB) ]
				].Boost[BOOST(CPB)],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(CPB) );
      }
    }

      for(boost = BOOST(1C), activeCores = 1;
      boost > BOOST(1C)-(enum RATIO_BOOST)RO(Shm)->Proc.Features.SpecTurboRatio;
		boost--, activeCores++)
      {
	CLOCK_ARG clockMod={.NC=BOXKEY_TURBO_CLOCK_NC | activeCores,.Offset=0};
	char pfx[10+1+1];
	StrFormat(pfx, 10+1+1, "%2uC", activeCores);

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[boost]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[boost] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, pfx, &RO(Shm)->Cpu[
						Ruler.Top[boost]
					].Boost[boost],
				1, clockMod.ullong,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, boost );
      }
    if (RO(Shm)->Proc.Features.ExtFeature.EDX.Hybrid == 1)
    {
	PUT(	SCANKEY_NULL, attrib[RO(Shm)->Proc.Features.Turbo_Unlock],
		width, 2, "%s%.*s[%7.*s]", RSC(HYBRID).CODE(),
		width - 12 - RSZ(HYBRID), hSpace, 6,
		RO(Shm)->Proc.Features.Turbo_Unlock ?
			RSC(UNLOCK).CODE() : RSC(LOCK).CODE() );

      for(boost = BOOST(1C), activeCores = 1;
      boost > BOOST(1C)-(enum RATIO_BOOST)RO(Shm)->Proc.Features.SpecTurboRatio;
		boost--, activeCores++)
      {
	CLOCK_ARG clockMod={.NC=BOXKEY_TURBO_CLOCK_NC | activeCores,.Offset=0};
	char pfx[10+1+1];
	StrFormat(pfx, 10+1+1, "%2uC", activeCores);

	CFlop = &RO(Shm)->Cpu[
			RO(Shm)->Proc.Service.Hybrid
		].FlipFlop[
			!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Hybrid].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, pfx, &RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Hybrid
					].Boost[boost],
				1, clockMod.ullong,
				width, OutFunc, attrib[3] ),
		RefreshHybridFreq, boost );
      }
    }

	PUT(	SCANKEY_NULL, attrib[RO(Shm)->Proc.Features.Uncore_Unlock],
		width, 2, "%s%.*s[%7.*s]", RSC(UNCORE).CODE(),
		width - 18, hSpace, 6,
		RO(Shm)->Proc.Features.Uncore_Unlock ?
			RSC(UNLOCK).CODE() : RSC(LOCK).CODE() );

	ASCII *uncoreLabel[2];

	if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	 || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON)) {
		uncoreLabel[0] = RSC(UCLK).CODE();
		uncoreLabel[1] = RSC(MCLK).CODE();
	} else {
		uncoreLabel[0] = RSC(MIN).CODE();
		uncoreLabel[1] = RSC(MAX).CODE();
	}

      if (RO(Shm)->Proc.Features.Uncore_Unlock) {
	CLOCK_ARG uncoreClock = {.NC = 0, .Offset = 0};

	uncoreClock.NC = BOXKEY_UNCORE_CLOCK_OR | CLOCK_MOD_MIN;
	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) uncoreLabel[0],
				&RO(Shm)->Uncore.Boost[UNCORE_BOOST(MIN)],
				1, uncoreClock.ullong,
				width, OutFunc, attrib[3] ),
		RefreshRatioFreq, &RO(Shm)->Uncore.Boost[UNCORE_BOOST(MIN)] );

	uncoreClock.NC = BOXKEY_UNCORE_CLOCK_OR | CLOCK_MOD_MAX;
	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) uncoreLabel[1],
				&RO(Shm)->Uncore.Boost[UNCORE_BOOST(MAX)],
				1, uncoreClock.ullong,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &RO(Shm)->Uncore.Boost[UNCORE_BOOST(MAX)] );
      } else {
	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) uncoreLabel[0],
				&RO(Shm)->Uncore.Boost[UNCORE_BOOST(MIN)],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &RO(Shm)->Uncore.Boost[UNCORE_BOOST(MIN)] );

	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) uncoreLabel[1],
				&RO(Shm)->Uncore.Boost[UNCORE_BOOST(MAX)],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3]),
		RefreshRatioFreq, &RO(Shm)->Uncore.Boost[UNCORE_BOOST(MAX)] );
      }

    if (RO(Shm)->Proc.Features.TDP_Cfg_Lock) {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s""%s"" [%3d:%-3d]", RSC(TDP).CODE(),
		width - 16 - RSZ(LEVEL), hSpace, RSC(LEVEL).CODE(),
		RO(Shm)->Proc.Features.TDP_Cfg_Level,
		RO(Shm)->Proc.Features.TDP_Levels );
    } else {
	GridCall( PUT(	BOXKEY_CFG_TDP_LVL, attrib[0], width, 2,
			"%s%.*s""%s"" <%3d:%-3d>", RSC(TDP).CODE(),
			width - 16 - RSZ(LEVEL),
			hSpace, RSC(LEVEL).CODE(),
			RO(Shm)->Proc.Features.TDP_Cfg_Level,
			RO(Shm)->Proc.Features.TDP_Levels ),
		RefreshConfigTDP );
    }
	PUT(	SCANKEY_NULL, attrib[RO(Shm)->Proc.Features.TDP_Unlock == 1],
		width, 3, "%s%.*s[%7.*s]", RSC(PROGRAMMABLE).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(PROGRAMMABLE), hSpace,
			6, RO(Shm)->Proc.Features.TDP_Unlock == 1 ?
				RSC(UNLOCK).CODE() : RSC(LOCK).CODE() );

    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	const size_t len = (size_t) RSZ(LEVEL) + 1 + 1;
	char *pfx = malloc(len);
      if (pfx != NULL)
      {
	coreClock = (CLOCK_ARG) {.NC = 0, .Offset = 0};

	PUT( SCANKEY_NULL, attrib[RO(Shm)->Proc.Features.TDP_Cfg_Lock == 0],
		width, 3, "%s%.*s[%7.*s]", RSC(CONFIGURATION).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(CONFIGURATION), hSpace,
			6, RO(Shm)->Proc.Features.TDP_Cfg_Lock == 1 ?
				RSC(LOCK).CODE() : RSC(UNLOCK).CODE() );

	PUT( SCANKEY_NULL, attrib[RO(Shm)->Proc.Features.TurboActiv_Lock == 0],
		width, 3, "%s%.*s[%7.*s]", RSC(TURBO_ACTIVATION).CODE(),
		width - (OutFunc == NULL ? 15:13)-RSZ(TURBO_ACTIVATION), hSpace,
			6, RO(Shm)->Proc.Features.TurboActiv_Lock == 1 ?
				RSC(LOCK).CODE() : RSC(UNLOCK).CODE() );

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(TDP) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(TDP) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, (char*) RSC(NOMINAL).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(TDP) ]
				].Boost[ BOOST(TDP) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TDP) );

	StrFormat(pfx, len, "%s" "1", RSC(LEVEL).CODE());

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(TDP1) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(TDP1) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, pfx, &RO(Shm)->Cpu[
						Ruler.Top[ BOOST(TDP1) ]
					].Boost[ BOOST(TDP1) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TDP1) );

	StrFormat(pfx, len, "%s" "2", RSC(LEVEL).CODE());

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(TDP2) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(TDP2) ] ].Toggle
		];

	GridCall( PrintRatioFreq(win, CFlop,
				0, pfx, &RO(Shm)->Cpu[
						Ruler.Top[ BOOST(TDP2) ]
					].Boost[ BOOST(TDP2) ],
				0, SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(TDP2) );

	CFlop = &RO(Shm)->Cpu[
			Ruler.Top[ BOOST(ACT) ]
		].FlipFlop[
			!RO(Shm)->Cpu[ Ruler.Top[ BOOST(ACT) ] ].Toggle
		];

	coreClock.NC = BOXKEY_CFGTDP_CLOCK_OR | CLOCK_MOD_ACT;

	GridCall( PrintRatioFreq(win, CFlop,
				1, (char*) RSC(TURBO).CODE(),
				&RO(Shm)->Cpu[
					Ruler.Top[ BOOST(ACT) ]
				].Boost[ BOOST(ACT) ],
				(RO(Shm)->Proc.Features.TurboActiv_Lock == 0),
				(RO(Shm)->Proc.Features.TurboActiv_Lock == 0) ?
					coreClock.ullong : SCANKEY_NULL,
				width, OutFunc, attrib[3] ),
		RefreshTopFreq, BOOST(ACT) );

	free(pfx);
      } else {
	REASON_SET(reason, RC_MEM_ERR);
      }
    }
	return reason;
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
	struct {
		Bit32	FP128	:  1-0,
			FP256	:  2-1,
			_pad32	: 32-2;
	} AVX = {
	.FP128 = (RO(Shm)->Proc.Features.Info.LargestExtFunc >= 0x8000001a)
		&& BITVAL(RO(Shm)->Cpu[
					RO(Shm)->Proc.Service.Core
				].CpuID[
				CPUID_8000001A_00000000_PERF_OPTIMIZATION
				].reg[0], 0),
	.FP256 = (RO(Shm)->Proc.Features.Info.LargestExtFunc >= 0x8000001a)
		&& BITVAL(RO(Shm)->Cpu[
					RO(Shm)->Proc.Service.Core
				].CpuID[
				CPUID_8000001A_00000000_PERF_OPTIMIZATION
				].reg[0], 2)
	};
	const struct ISA_ST {
		unsigned int	*CRC;
		const ASCII	*item, *comm;
		unsigned short	thm[2];
		unsigned short	*cond;
	} ISA[] = \
    {
/* Row Mark */
	{
		NULL,
		RSC(ISA_3DNOW).CODE(), RSC(ISA_3DNOW_COMM).CODE(),
		{
		0,
		2 * ( RO(Shm)->Proc.Features.ExtInfo.EDX._3DNow
			|  RO(Shm)->Proc.Features.ExtInfo.EDX._3DNowEx )
		+ ( RO(Shm)->Proc.Features.ExtInfo.EDX._3DNow
			<< RO(Shm)->Proc.Features.ExtInfo.EDX._3DNowEx )
		},
		(unsigned short[])
		{
		RO(Shm)->Proc.Features.ExtInfo.EDX._3DNow,
		RO(Shm)->Proc.Features.ExtInfo.EDX._3DNowEx
		}
	},
	{
		NULL,
		RSC(ISA_ADX).CODE(), RSC(ISA_ADX_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.ADX },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.ADX }
	},
	{
		NULL,
		RSC(ISA_AES).CODE(), RSC(ISA_AES_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.AES },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.AES },
	},
	{
		NULL,
		RSC(ISA_AVX).CODE(), RSC(ISA_AVX_COMM).CODE(),
		{
		1,
		2 * ( RO(Shm)->Proc.Features.Std.ECX.AVX
			|  RO(Shm)->Proc.Features.ExtFeature.EBX.AVX2 )
		+ ( RO(Shm)->Proc.Features.Std.ECX.AVX
			<< RO(Shm)->Proc.Features.ExtFeature.EBX.AVX2 )
		},
		(unsigned short[])
		{
		RO(Shm)->Proc.Features.Std.ECX.AVX,
		RO(Shm)->Proc.Features.ExtFeature.EBX.AVX2
		}
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_AVX512_F).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX_512F },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX_512F },
	},
	{
		NULL,
		RSC(ISA_AVX512_DQ).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX_512DQ },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX_512DQ },
	},
	{
		NULL,
		RSC(ISA_AVX512_IFMA).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512_IFMA },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512_IFMA },
	},
	{
		NULL,
		RSC(ISA_AVX512_PF).CODE(), NULL,
		{ 1, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512PF },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512PF },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_AVX512_ER).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512ER },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512ER },
	},
	{
		NULL,
		RSC(ISA_AVX512_CD).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512CD },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512CD },
	},
	{
		NULL,
		RSC(ISA_AVX512_BW).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512BW },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512BW },
	},
	{
		NULL,
		RSC(ISA_AVX512_VL).CODE(), NULL,
		{ 1, RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512VL },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.AVX512VL },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_AVX512_VBMI).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VBMI },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VBMI },
	},
	{
		NULL,
		RSC(ISA_AVX512_VBMI2).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VBMI2 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VBMI2 },
	},
	{
		NULL,
		RSC(ISA_AVX512_VNNI).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VNNI },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VNNI },
	},
	{
		NULL,
		RSC(ISA_AVX512_ALG).CODE(), NULL,
		{ 1, RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_BITALG },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_BITALG },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_AVX512_VPOP).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VPOPCNTDQ },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.AVX512_VPOPCNTDQ },
	},
	{
		NULL,
		RSC(ISA_AVX512_VNNIW).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EDX.AVX512_4VNNIW },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EDX.AVX512_4VNNIW },
	},
	{
		NULL,
		RSC(ISA_AVX512_FMAPS).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EDX.AVX512_4FMAPS },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EDX.AVX512_4FMAPS },
	},
	{
		NULL,
		RSC(ISA_AVX512_VP2I).CODE(), NULL,
		{ 1, RO(Shm)->Proc.Features.ExtFeature.EDX.AVX512_VP2INTER },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EDX.AVX512_VP2INTER },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_AVX512_BF16).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.AVX512_BF16 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.AVX512_BF16 },
	},
	{
		NULL,
		RSC(ISA_AVX_VEX).CODE(), NULL,
		{ 0, RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.AVX_VNNI_VEX },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.AVX_VNNI_VEX },
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RSC(ISA_AVX_128).CODE(), NULL,
		{ 0, AVX.FP128 },
		(unsigned short[])
		{ AVX.FP128 },
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RSC(ISA_AVX_256).CODE(), NULL,
		{ 1, AVX.FP256 },
		(unsigned short[])
		{ AVX.FP256 },
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RSC(ISA_MOVDIRI).CODE(), RSC(ISA_MOVDIRI_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.ECX.MOVDIRI },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.MOVDIRI },
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RSC(ISA_MOVDIR64B).CODE(), RSC(ISA_MOVDIR64B_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.ExtFeature.ECX.MOVDIR64B },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.MOVDIR64B },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_BMI).CODE(), RSC(ISA_BMI_COMM).CODE(),
		{ 0, 2 * ( RO(Shm)->Proc.Features.ExtFeature.EBX.BMI1
				|  RO(Shm)->Proc.Features.ExtFeature.EBX.BMI2 )
			+ ( RO(Shm)->Proc.Features.ExtFeature.EBX.BMI1
				<< RO(Shm)->Proc.Features.ExtFeature.EBX.BMI2)},
		(unsigned short[])
		{
		RO(Shm)->Proc.Features.ExtFeature.EBX.BMI1,
		RO(Shm)->Proc.Features.ExtFeature.EBX.BMI2
		},
	},
	{
		NULL,
		RSC(ISA_CLWB).CODE(), RSC(ISA_CLWB_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.CLWB },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.CLWB },
	},
	{
		NULL,
		RSC(ISA_CLFLUSH).CODE(), RSC(ISA_CLFLUSH_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.CLFLUSH },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.CLFLUSH },
	},
	{
		NULL,
		RSC(ISA_CLFLUSHOPT).CODE(), RSC(ISA_CLFLUSHOPT_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.ExtFeature.EBX.CLFLUSHOPT },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.CLFLUSHOPT },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_AC_FLAG).CODE(), RSC(ISA_AC_FLAG_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.SMAP_CLAC_STAC },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.SMAP_CLAC_STAC },
	},
	{
		NULL,
		RSC(ISA_CMOV).CODE(), RSC(ISA_CMOV_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.CMOV },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.CMOV },
	},
	{
		NULL,
		RSC(ISA_XCHG8B).CODE(), RSC(ISA_XCHG8B_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.CMPXCHG8 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.CMPXCHG8 },
	},
	{
		NULL,
		RSC(ISA_XCHG16B).CODE(), RSC(ISA_XCHG16B_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.Std.ECX.CMPXCHG16 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.CMPXCHG16 },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_F16C).CODE(), RSC(ISA_F16C_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.F16C },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.F16C },
	},
	{
		NULL,
		RSC(ISA_FPU).CODE(), RSC(ISA_FPU_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.FPU },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.FPU },
	},
	{
		NULL,
		RSC(ISA_FXSR).CODE(), RSC(ISA_FXSR_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.FXSR },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.FXSR },
	},
	{
		NULL,
		RSC(ISA_LSHF).CODE(), RSC(ISA_LSHF_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.ExtInfo.ECX.LAHFSAHF },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtInfo.ECX.LAHFSAHF },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_MMX).CODE(), RSC(ISA_MMX_COMM).CODE(),
		{ 0, 2 * ( RO(Shm)->Proc.Features.Std.EDX.MMX
				|  RO(Shm)->Proc.Features.ExtInfo.EDX.MMX_Ext )
			+ ( RO(Shm)->Proc.Features.Std.EDX.MMX
				<< RO(Shm)->Proc.Features.ExtInfo.EDX.MMX_Ext)},
		(unsigned short[])
		{
		RO(Shm)->Proc.Features.Std.EDX.MMX,
		RO(Shm)->Proc.Features.ExtInfo.EDX.MMX_Ext
		},
	},
	{
		NULL,
		RSC(ISA_MWAITX).CODE(), RSC(ISA_MWAITX_COMM).CODE(),
		{ 0, 2 * ( RO(Shm)->Proc.Features.Std.ECX.MONITOR
				|  RO(Shm)->Proc.Features.ExtInfo.ECX.MWaitExt )
			+ ( RO(Shm)->Proc.Features.Std.ECX.MONITOR
				<<RO(Shm)->Proc.Features.ExtInfo.ECX.MWaitExt)},
		(unsigned short[])
		{
		RO(Shm)->Proc.Features.Std.ECX.MONITOR,
		RO(Shm)->Proc.Features.ExtInfo.ECX.MWaitExt
		},
	},
	{
		NULL,
		RSC(ISA_MOVBE).CODE(), RSC(ISA_MOVBE_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.MOVBE },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.MOVBE },
	},
	{
		NULL,
		RSC(ISA_PCLMULDQ).CODE(), RSC(ISA_PCLMULDQ_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.Std.ECX.PCLMULDQ },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.PCLMULDQ },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_POPCNT).CODE(), RSC(ISA_POPCNT_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.POPCNT },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.POPCNT },
	},
	{
		NULL,
		RSC(ISA_RDRAND).CODE(), RSC(ISA_RDRAND_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.RDRAND },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.RDRAND },
	},
	{
		NULL,
		RSC(ISA_RDSEED).CODE(), RSC(ISA_RDSEED_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.RDSEED },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.RDSEED },
	},
	{
		NULL,
		RSC(ISA_RDTSCP).CODE(), RSC(ISA_RDTSCP_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.ExtInfo.EDX.RDTSCP },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtInfo.EDX.RDTSCP },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_SEP).CODE(), RSC(ISA_SEP_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.SEP },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.SEP },
	},
	{
		NULL,
		RSC(ISA_SHA).CODE(), RSC(ISA_SHA_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.SHA },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.SHA },
	},
	{
		NULL,
		RSC(ISA_SSE).CODE(), RSC(ISA_SSE_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.EDX.SSE },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.SSE },
	},
	{
		NULL,
		RSC(ISA_SSE2).CODE(), RSC(ISA_SSE2_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.Std.EDX.SSE2 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.EDX.SSE2 },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_SSE3).CODE(), RSC(ISA_SSE3_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.SSE3 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.SSE3 },
	},
	{
		NULL,
		RSC(ISA_SSSE3).CODE(), RSC(ISA_SSSE3_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.Std.ECX.SSSE3 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.SSSE3 },
	},
	{
		NULL,
		RSC(ISA_SSE4_1).CODE(), RSC(ISA_SSE4_1_COMM).CODE(),
		{ 0, 2 * ( RO(Shm)->Proc.Features.Std.ECX.SSE41
				|  RO(Shm)->Proc.Features.ExtInfo.ECX.SSE4A )
			+ ( RO(Shm)->Proc.Features.Std.ECX.SSE41
				<< RO(Shm)->Proc.Features.ExtInfo.ECX.SSE4A ) },
		(unsigned short[])
		{
		RO(Shm)->Proc.Features.Std.ECX.SSE41,
		RO(Shm)->Proc.Features.ExtInfo.ECX.SSE4A
		},
	},
	{
		NULL,
		RSC(ISA_SSE4_2).CODE(), RSC(ISA_SSE4_2_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.Std.ECX.SSE42 },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.Std.ECX.SSE42 },
	},
/* Row Mark */
	{
		NULL,
		RSC(ISA_SERIALIZE).CODE(), RSC(ISA_SERIALIZE_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EDX.SERIALIZE },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EDX.SERIALIZE },
	},
	{
		NULL,
		RSC(ISA_SYSCALL).CODE(), RSC(ISA_SYSCALL_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtInfo.EDX.SYSCALL },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtInfo.EDX.SYSCALL },
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RSC(ISA_RDPID_FMT1).CODE(), RSC(ISA_RDPID_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.RDPID },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.RDPID },
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RSC(ISA_UMIP).CODE(), RSC(ISA_UMIP_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.ExtFeature.EBX.SGX_UMIP },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.SGX_UMIP },
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RSC(ISA_SGX).CODE(), RSC(ISA_SGX_COMM).CODE(),
		{ 0, RO(Shm)->Proc.Features.ExtFeature.EBX.SGX_UMIP },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.EBX.SGX_UMIP },
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RSC(ISA_RDPID_FMT2).CODE(), RSC(ISA_RDPID_COMM).CODE(),
		{ 1, RO(Shm)->Proc.Features.ExtFeature.ECX.RDPID },
		(unsigned short[])
		{ RO(Shm)->Proc.Features.ExtFeature.ECX.RDPID },
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
		if ( (*CRC) == RO(Shm)->Proc.Features.Info.Vendor.CRC ) {
			capable = 1;
		}
	    }
	}
	if (capable) {
		GridHover(PRT(ISA, attrib[ ISA[idx].thm[0] ][ ISA[idx].thm[1] ],
				ISA[idx].item,
				ISA[idx].cond[0] ? 'Y' : 'N',
				ISA[idx].cond[1] ? 'Y' : 'N' ),
			(char *)ISA[idx].comm );
	}
    }
	return reason;
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
		RSC(XAPIC).CODE(),
		RSC(X2APIC).CODE()
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
		RO(Shm)->Proc.Features.ExtInfo.EDX.PG_1GB == 1,
		attr_Feat,
		2, "%s%.*s1GB-PAGES   [%7s]", RSC(FEATURES_1GB_PAGES).CODE(),
		width - 24 - RSZ(FEATURES_1GB_PAGES),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.AdvPower.EDX._100MHz == 1,
		attr_Feat,
		2, "%s%.*s100MHzSteps   [%7s]", RSC(FEATURES_100MHZ).CODE(),
		width - 26 - RSZ(FEATURES_100MHZ),
		NULL
	},
	{
		NULL,
		(RO(Shm)->Proc.Features.Std.EDX.ACPI == 1)
		|| (RO(Shm)->Proc.Features.AdvPower.EDX.HwPstate == 1),
		attr_Feat,
		2, "%s%.*sACPI   [%7s]", RSC(FEATURES_ACPI).CODE(),
		width - 19 - RSZ(FEATURES_ACPI),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.APIC == 1,
		attr_Feat,
		2, "%s%.*sAPIC   [%7s]", RSC(FEATURES_APIC).CODE(),
		width - 19 - RSZ(FEATURES_APIC),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Power.EAX.ARAT == 1,
		attr_Feat,
		2, "%s%.*sARAT   [%7s]", RSC(FEATURES_ARAT).CODE(),
		width - 19 - RSZ(FEATURES_ARAT),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtInfo.ECX.MP_Mode == 1,
		attr_Feat,
		2, "%s%.*sCMP Legacy   [%7s]", RSC(FEATURES_CORE_MP).CODE(),
		width - 25 - RSZ(FEATURES_CORE_MP),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.CNXT_ID == 1,
		attr_Feat,
		2, "%s%.*sCNXT-ID   [%7s]", RSC(FEATURES_CNXT_ID).CODE(),
		width - 22 - RSZ(FEATURES_CNXT_ID),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.CPPC == 1,
		attr_Feat,
		2, "%s%.*sCPPC   [%7s]", RSC(FEATURES_CPPC).CODE(),
		width - 19 - RSZ(FEATURES_CPPC),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.DCA == 1,
		attr_Feat,
		2, "%s%.*sDCA   [%7s]", RSC(FEATURES_DCA).CODE(),
		width - 18 - RSZ(FEATURES_DCA),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.DE == 1,
		attr_Feat,
		2, "%s%.*sDE   [%7s]", RSC(FEATURES_DE).CODE(),
		width - 17 - RSZ(FEATURES_DE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.DS_PEBS == 1,
		attr_Feat,
		2, "%s%.*sDS, PEBS   [%7s]", RSC(FEATURES_DS_PEBS).CODE(),
		width - 23 - RSZ(FEATURES_DS_PEBS),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.DS_CPL == 1,
		attr_Feat,
		2, "%s%.*sDS-CPL   [%7s]", RSC(FEATURES_DS_CPL).CODE(),
		width - 21 - RSZ(FEATURES_DS_CPL),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.DTES64 == 1,
		attr_Feat,
		2, "%s%.*sDTES64   [%7s]", RSC(FEATURES_DTES_64).CODE(),
		width - 21 - RSZ(FEATURES_DTES_64),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.FSRC == 1,
		attr_Feat,
		2, "%s%.*sFSRC   [%7s]", RSC(FEATURES_FSRC).CODE(),
		width - 19 - RSZ(FEATURES_FSRC),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EDX.FSRM == 1,
		attr_Feat,
		2, "%s%.*sFSRM   [%7s]", RSC(FEATURES_FSRM).CODE(),
		width - 19 - RSZ(FEATURES_FSRM),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.FSRS == 1,
		attr_Feat,
		2, "%s%.*sFSRS   [%7s]", RSC(FEATURES_FSRS).CODE(),
		width - 19 - RSZ(FEATURES_FSRS),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.FZRM == 1,
		attr_Feat,
		2, "%s%.*sFZRM   [%7s]", RSC(FEATURES_FZRM).CODE(),
		width - 19 - RSZ(FEATURES_FZRM),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.ERMS == 1,
		attr_Feat,
		2, "%s%.*sERMS   [%7s]", RSC(FEATURES_ERMS).CODE(),
		width - 19 - RSZ(FEATURES_ERMS),
		NULL
	},
	{
		NULL,
		(RO(Shm)->Proc.Features.Std.ECX.FMA == 1)
		|| (RO(Shm)->Proc.Features.ExtInfo.ECX.FMA4 == 1),
		attr_Feat,
		2, "%s%.*sFMA | FMA4   [%7s]", RSC(FEATURES_FMA).CODE(),
		width - 25 - RSZ(FEATURES_FMA),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.Power.EAX.HWFB_Cap == 1,
		attr_Feat,
		2, "%s%.*sHFI   [%7s]", RSC(FEATURES_HFI).CODE(),
		width - 18 - RSZ(FEATURES_HFI),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.HLE == 1,
		attr_Feat,
		2, "%s%.*sHLE   [%7s]", RSC(FEATURES_HLE).CODE(),
		width - 18 - RSZ(FEATURES_HLE),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.AdvPower.EDX.HwPstate == 1,
		attr_Feat,
		2, "%s%.*sHwP   [%7s]", RSC(FEATURES_HwP).CODE(),
		width - 18 - RSZ(FEATURES_HwP),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature_Leaf1.EAX.HRESET == 1,
		attr_Feat,
		2, "%s%.*sHRESET   [%7s]", RSC(FEATURES_HRESET).CODE(),
		width - 21 - RSZ(FEATURES_HRESET),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature.EDX.Hybrid == 1,
		attr_Feat,
		2, "%s%.*sHYBRID   [%7s]", RSC(FEATURES_HYBRID).CODE(),
		width - 21 - RSZ(FEATURES_HYBRID),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtInfo.ECX.IBS == 1,
		attr_Feat,
		2, "%s%.*sIBS   [%7s]", RSC(FEATURES_IBS).CODE(),
		width - 18 - RSZ(FEATURES_IBS),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.INVLPGB == 1,
		attr_Feat,
		2, "%s%.*sINVLPGB   [%7s]", RSC(FEATURES_INVLPGB).CODE(),
		width - 22 - RSZ(FEATURES_INVLPGB),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.INVPCID == 1,
		attr_Feat,
		2, "%s%.*sINVPCID   [%7s]", RSC(FEATURES_INVPCID).CODE(),
		width - 22 - RSZ(FEATURES_INVPCID),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtInfo.EDX.IA64 == 1,
		attr_Feat,
		2, "%s%.*sIA64 | LM   [%7s]", RSC(FEATURES_LM).CODE(),
		width - 24 - RSZ(FEATURES_LM),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtInfo.ECX.LWP == 1,
		attr_Feat,
		2, "%s%.*sLWP   [%7s]", RSC(FEATURES_LWP).CODE(),
		width - 18 - RSZ(FEATURES_LWP),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.MBE == 1,
		attr_Feat,
		2, "%s%.*sMBE   [%7s]", RSC(FEATURES_MBE).CODE(),
		width - 18 - RSZ(FEATURES_MBE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.MCA == 1,
		attr_Feat,
		2, "%s%.*sMCA   [%7s]", RSC(FEATURES_MCA).CODE(),
		width - 18 - RSZ(FEATURES_MCA),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.MCOMMIT == 1,
		attr_Feat,
		2, "%s%.*sMCOMMIT   [%7s]", RSC(FEATURES_MCOMMIT).CODE(),
		width - 22 - RSZ(FEATURES_MCOMMIT),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.MPX == 1,
		attr_Feat,
		2, "%s%.*sMPX   [%7s]", RSC(FEATURES_MPX).CODE(),
		width - 18 - RSZ(FEATURES_MPX),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.MSR == 1,
		attr_Feat,
		2, "%s%.*sMSR   [%7s]", RSC(FEATURES_MSR).CODE(),
		width - 18 - RSZ(FEATURES_MSR),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.MTRR == 1,
		attr_Feat,
		2, "%s%.*sMTRR   [%7s]", RSC(FEATURES_MTRR).CODE(),
		width - 19 - RSZ(FEATURES_MTRR),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.ExtInfo.EDX.NX == 1,
		attr_Feat,
		2, "%s%.*sNX   [%7s]", RSC(FEATURES_NX).CODE(),
		width - 17 - RSZ(FEATURES_NX),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.OSXSAVE == 1,
		attr_Feat,
		2, "%s%.*sOSXSAVE   [%7s]", RSC(FEATURES_OSXSAVE).CODE(),
		width - 22 - RSZ(FEATURES_OSXSAVE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PAE == 1,
		attr_Feat,
		2, "%s%.*sPAE   [%7s]", RSC(FEATURES_PAE).CODE(),
		width - 18 - RSZ(FEATURES_PAE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PAT == 1,
		attr_Feat,
		2, "%s%.*sPAT   [%7s]", RSC(FEATURES_PAT).CODE(),
		width - 18 - RSZ(FEATURES_PAT),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PBE == 1,
		attr_Feat,
		2, "%s%.*sPBE   [%7s]", RSC(FEATURES_PBE).CODE(),
		width - 18 - RSZ(FEATURES_PBE),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature.EDX.PCONFIG == 1,
		attr_Feat,
		2, "%s%.*sPCONFIG   [%7s]", RSC(FEATURES_PCONFIG).CODE(),
		width - 22 - RSZ(FEATURES_PCONFIG),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.PCID == 1,
		attr_Feat,
		2, "%s%.*sPCID   [%7s]", RSC(FEATURES_PCID).CODE(),
		width - 19 - RSZ(FEATURES_PCID),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.PDCM == 1,
		attr_Feat,
		2, "%s%.*sPDCM   [%7s]", RSC(FEATURES_PDCM).CODE(),
		width - 19 - RSZ(FEATURES_PDCM),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PGE == 1,
		attr_Feat,
		2, "%s%.*sPGE   [%7s]", RSC(FEATURES_PGE).CODE(),
		width - 18 - RSZ(FEATURES_PGE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PSE == 1,
		attr_Feat,
		2, "%s%.*sPSE   [%7s]", RSC(FEATURES_PSE).CODE(),
		width - 18 - RSZ(FEATURES_PSE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PSE36 == 1,
		attr_Feat,
		2, "%s%.*sPSE36   [%7s]", RSC(FEATURES_PSE36).CODE(),
		width - 20 - RSZ(FEATURES_PSE36),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.PSN == 1,
		attr_Feat,
		2, "%s%.*sPSN   [%7s]", RSC(FEATURES_PSN).CODE(),
		width - 18 - RSZ(FEATURES_PSN),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.PQE == 1,
		attr_Feat,
		2, "%s%.*sRDT-A   [%7s]", RSC(FEATURES_RDT_PQE).CODE(),
		width - 20 - RSZ(FEATURES_RDT_PQE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.PQM == 1,
		attr_Feat,
		2, "%s%.*sRDT-M   [%7s]", RSC(FEATURES_RDT_PQM).CODE(),
		width - 20 - RSZ(FEATURES_RDT_PQM),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.RDPRU == 1,
		attr_Feat,
		2, "%s%.*sRDPRU   [%7s]", RSC(FEATURES_RDPRU).CODE(),
		width - 20 - RSZ(FEATURES_RDPRU),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.RTM == 1,
		attr_Feat,
		2, "%s%.*sRTM   [%7s]", RSC(FEATURES_RTM).CODE(),
		width - 18 - RSZ(FEATURES_RTM),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.SMX == 1,
		attr_Feat,
		2, "%s%.*sSMX   [%7s]", RSC(FEATURES_SMX).CODE(),
		width - 18 - RSZ(FEATURES_SMX),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.SS == 1,
		attr_Feat,
		2, "%s%.*sSS   [%7s]", RSC(FEATURES_SELF_SNOOP).CODE(),
		width - 17 - RSZ(FEATURES_SELF_SNOOP),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.SMAP_CLAC_STAC == 1,
		attr_Feat,
		2, "%s%.*sSMAP   [%7s]", RSC(FEATURES_SMAP).CODE(),
		width - 19 - RSZ(FEATURES_SMAP),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EBX.SMEP == 1,
		attr_Feat,
		2, "%s%.*sSMEP   [%7s]", RSC(FEATURES_SMEP).CODE(),
		width - 19 - RSZ(FEATURES_SMEP),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.Power.EAX.ITD_MSR == 1,
		attr_Feat,
		2, "%s%.*sTD   [%7s]", RSC(FEATURES_ITD).CODE(),
		width - 17 - RSZ(FEATURES_ITD),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.InvariantTSC,
		attr_TSC,
		2, "%s%.*sTSC [%9s]", RSC(FEATURES_TSC).CODE(),
		width - 18 - RSZ(FEATURES_TSC),
		code_TSC
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.TSC_DEADLINE == 1,
		attr_Feat,
		2, "%s%.*sTSC-DEADLINE   [%7s]",RSC(FEATURES_TSC_DEADLN).CODE(),
		width - 27 - RSZ(FEATURES_TSC_DEADLN),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EDX.TSX_FORCE_ABORT == 1,
		attr_Feat,
		2, "%s%.*sTSX-ABORT   [%7s]", RSC(FEATURES_TSXABORT).CODE(),
		width - 24 - RSZ(FEATURES_TSXABORT),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.ExtFeature.EDX.TSXLDTRK == 1,
		attr_Feat,
		2, "%s%.*sTSX-LDTRK   [%7s]", RSC(FEATURES_TSXLDTRK).CODE(),
		width - 24 - RSZ(FEATURES_TSXLDTRK),
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.ExtFeature.EBX.SGX_UMIP == 1,
		attr_Feat,
		2, "%s%.*sUMIP   [%7s]", RSC(FEATURES_UMIP).CODE(),
		width - 19 - RSZ(FEATURES_UMIP),
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature.ECX.UMIP == 1,
		attr_Feat,
		2, "%s%.*sUMIP   [%7s]", RSC(FEATURES_UMIP).CODE(),
		width - 19 - RSZ(FEATURES_UMIP),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.EDX.VME == 1,
		attr_Feat,
		2, "%s%.*sVME   [%7s]", RSC(FEATURES_VME).CODE(),
		width - 18 - RSZ(FEATURES_VME),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.VMX == 1,
		attr_Feat,
		2, "%s%.*sVMX   [%7s]", RSC(FEATURES_VMX).CODE(),
		width - 18 - RSZ(FEATURES_VMX),
		NULL
	},
	{
		NULL,
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.MP.x2APIC,
		attr_Feat,
		2, "%s%.*sx2APIC   [%7s]", RSC(FEATURES_X2APIC).CODE(),
		width - 21 - RSZ(FEATURES_X2APIC),
		x2APIC
	},
	{
		(unsigned int[]) { CRC_INTEL, 0},
		RO(Shm)->Proc.Features.ExtInfo.EDX.XD_Bit == 1,
		attr_Feat,
		2, "%s%.*sXD-Bit   [%7s]", RSC(FEATURES_XD_BIT).CODE(),
		width - 21 - RSZ(FEATURES_XD_BIT),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.XSAVE == 1,
		attr_Feat,
		2, "%s%.*sXSAVE   [%7s]", RSC(FEATURES_XSAVE).CODE(),
		width - 20 - RSZ(FEATURES_XSAVE),
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.xTPR == 1,
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
		RO(Shm)->Proc.Mechanisms.IBRS,
		attr_Feat,
		2, "%s%.*sIBRS   [%7s]", RSC(MECH_IBRS).CODE(),
		width - 19 - RSZ(MECH_IBRS),
		MECH
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.IBRS_AlwaysOn,
		attr_Feat,
		3, "%s%.*s[%7s]", RSC(MECH_IBRS_ALWAYS_ON).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(MECH_IBRS_ALWAYS_ON),
		MECH
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.IBRS_Preferred,
		attr_Feat,
		3, "%s%.*s[%7s]", RSC(MECH_IBRS_PREFERRED).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(MECH_IBRS_PREFERRED),
		MECH
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.IBRS_SameMode,
		attr_Feat,
		3, "%s%.*s[%7s]", RSC(MECH_IBRS_SAME_MODE).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(MECH_IBRS_SAME_MODE),
		MECH
	},
	{
		NULL,
		( RO(Shm)->Proc.Features.ExtFeature.EDX.IBRS_IBPB_Cap == 1 )
		|| ( RO(Shm)->Proc.Features.leaf80000008.EBX.IBPB == 1 ),
		attr_Feat,
		2, "%s%.*sIBPB   [%7s]", RSC(MECH_IBPB).CODE(),
		width - 19 - RSZ(MECH_IBPB),
		MECH
	},
	{
		NULL,
		RO(Shm)->Proc.Mechanisms.STIBP,
		attr_Feat,
		2, "%s%.*sSTIBP   [%7s]", RSC(MECH_STIBP).CODE(),
		width - 20 - RSZ(MECH_STIBP),
		MECH
	},
	{
		NULL,
		RO(Shm)->Proc.Mechanisms.SSBD,
		attr_Feat,
		2, "%s%.*sSSBD   [%7s]", RSC(MECH_SSBD).CODE(),
		width - 19 - RSZ(MECH_SSBD),
		MECH
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.SSBD_VirtSpecCtrl,
		attr_Feat,
		3, "%s%.*s[%7s]", RSC(MECH_SSBD_VIRTSPECCTRL).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(MECH_SSBD_VIRTSPECCTRL),
		MECH
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.leaf80000008.EBX.SSBD_NotRequired,
		attr_Feat,
		3, "%s%.*s[%7s]", RSC(MECH_SSBD_NOT_REQUIRED).CODE(),
		width - (OutFunc == NULL ? 15:13) - RSZ(MECH_SSBD_NOT_REQUIRED),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature.EDX.L1D_FLUSH_Cap == 1,
		attr_Feat,
		2, "%s%.*sL1D-FLUSH   [%7s]", RSC(MECH_L1D_FLUSH).CODE(),
		width - 24 - RSZ(MECH_L1D_FLUSH),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.L1DFL_VMENTRY_NO,
		attr_Feat,
		2, "%s%.*sL1DFL_VMENTRY_NO   [%7s]",
		RSC(MECH_L1DFL_VMENTRY_NO).CODE(),
		width - 31 - RSZ(MECH_L1DFL_VMENTRY_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.ExtFeature.EDX.MD_CLEAR_Cap == 1,
		attr_Feat,
		2, "%s%.*sMD-CLEAR   [%7s]", RSC(MECH_MD_CLEAR).CODE(),
		width - 23 - RSZ(MECH_MD_CLEAR),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.RDCL_NO,
		attr_Feat,
		2, "%s%.*sRDCL_NO   [%7s]", RSC(MECH_RDCL_NO).CODE(),
		width - 22 - RSZ(MECH_RDCL_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.IBRS_ALL,
		attr_Feat,
		2, "%s%.*sIBRS_ALL   [%7s]", RSC(MECH_IBRS_ALL).CODE(),
		width - 23 - RSZ(MECH_IBRS_ALL),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.RSBA,
		attr_Feat,
		2, "%s%.*sRSBA   [%7s]", RSC(MECH_RSBA).CODE(),
		width - 19 - RSZ(MECH_RSBA),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.SSB_NO,
		attr_Feat,
		2, "%s%.*sSSB_NO   [%7s]", RSC(MECH_SSB_NO).CODE(),
		width - 21 - RSZ(MECH_SSB_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.MDS_NO,
		attr_Feat,
		2, "%s%.*sMDS_NO   [%7s]", RSC(MECH_MDS_NO).CODE(),
		width - 21 - RSZ(MECH_MDS_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.TAA_NO,
		attr_Feat,
		2, "%s%.*sTAA_NO   [%7s]", RSC(MECH_TAA_NO).CODE(),
		width - 21 - RSZ(MECH_TAA_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.PSCHANGE_MC_NO,
		attr_Feat,
		2, "%s%.*sPSCHANGE_MC_NO   [%7s]",
		RSC(MECH_PSCHANGE_MC_NO).CODE(),
		width - 29 - RSZ(MECH_PSCHANGE_MC_NO),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.STLB,
		attr_Feat,
		2, "%s%.*sSTLB   [%7s]", RSC(MECH_STLB).CODE(),
		width - 19 - RSZ(MECH_STLB),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.FUSA,
		attr_Feat,
		2, "%s%.*sFuSa   [%7s]", RSC(MECH_FUSA).CODE(),
		width - 19 - RSZ(MECH_FUSA),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.RSM_CPL0,
		attr_Feat,
		2, "%s%.*sRSM   [%7s]", RSC(MECH_RSM_CPL0).CODE(),
		width - 18 - RSZ(MECH_RSM_CPL0),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.SPLA,
		attr_Feat,
		2, "%s%.*sSPLA   [%7s]", RSC(MECH_SPLA).CODE(),
		width - 19 - RSZ(MECH_SPLA),
		MECH
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Mechanisms.SNOOP_FILTER,
		attr_Feat,
		2, "%s%.*sSNOOP_FILTER   [%7s]", RSC(MECH_SNOOP_FILTER).CODE(),
		width - 27 - RSZ(MECH_SNOOP_FILTER),
		MECH
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Mechanisms.PSFD,
		attr_Feat,
		2, "%s%.*sPSFD   [%7s]", RSC(MECH_PSFD).CODE(),
		width - 19 - RSZ(MECH_PSFD),
		NULL
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
		if ( (*CRC) == RO(Shm)->Proc.Features.Info.Vendor.CRC ) {
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
	return reason;
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

void L1_HW_Prefetch_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.L1_HW_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void L1_HW_IP_Prefetch_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix=RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void L2_HW_Prefetch_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.L2_HW_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void L2_HW_CL_Prefetch_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix=RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void SpeedStepUpdate(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.EIST == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void IDA_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Features.Power.EAX.TurboIDA == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void TurboUpdate(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.Turbo == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void EEO_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Features.EEO_Enable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void R2H_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Features.R2H_Enable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void WDT_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.WDT == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	TechUpdate(grid, bix, pos, 3, ENABLED(bix));
}

REASON_CODE SysInfoTech(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	char IOMMU_Version_String[10+1+10+1+1];
	const ASCII *Hypervisor[HYPERVISORS] = {
		[HYPERV_NONE]	= RSC(TECH_HYPERV_NONE).CODE(),
		[BARE_METAL]	= RSC(TECH_BARE_METAL).CODE(),
		[HYPERV_XEN]	= RSC(TECH_HYPERV_XEN).CODE(),
		[HYPERV_KVM]	= RSC(TECH_HYPERV_KVM).CODE(),
		[HYPERV_VBOX]	= RSC(TECH_HYPERV_VBOX).CODE(),
		[HYPERV_KBOX]	= RSC(TECH_HYPERV_KBOX).CODE(),
		[HYPERV_VMWARE] = RSC(TECH_HYPERV_VMWARE).CODE(),
		[HYPERV_HYPERV] = RSC(TECH_HYPERV_HYPERV).CODE()
	};
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
		void			(*Update)(struct _Grid*, DATA_TYPE[]);
	} TECH[] = \
    {
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		0,
		2, "%s%.*s",
		RSC(TECHNOLOGIES_ICU).CODE(),
		width - 3 - RSZ(TECHNOLOGIES_ICU),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch,
		3, "%s%.*sL1 HW IP   <%3s>",
		RSC(TECH_L1_HW_IP_PREFETCH).CODE(),
		width - (OutFunc ? 20 : 22) - RSZ(TECH_L1_HW_IP_PREFETCH),
		NULL,
		BOXKEY_L1_HW_IP_PREFETCH,
		L1_HW_IP_Prefetch_Update
	},
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
		RO(Shm)->Proc.Technology.L1_HW_Prefetch,
		3, "%s%.*sL1 HW   <%3s>",
		RSC(TECH_L1_HW_PREFETCH).CODE(),
		width - (OutFunc ? 17 : 19) - RSZ(TECH_L1_HW_PREFETCH),
		NULL,
		BOXKEY_L1_HW_PREFETCH,
		L1_HW_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch,
		3, "%s%.*sL1 HW IP   <%3s>",
		RSC(TECH_L1_HW_IP_PREFETCH).CODE(),
		width - (OutFunc ? 20 : 22) - RSZ(TECH_L1_HW_IP_PREFETCH),
		NULL,
		BOXKEY_L1_HW_IP_PREFETCH,
		L1_HW_IP_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.L2_HW_Prefetch,
		3, "%s%.*sL2 HW   <%3s>",
		RSC(TECH_L2_HW_PREFETCH).CODE(),
		width - (OutFunc ? 17 : 19) - RSZ(TECH_L2_HW_PREFETCH),
		NULL,
		BOXKEY_L2_HW_PREFETCH,
		L2_HW_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch,
		3, "%s%.*sL2 HW CL   <%3s>",
		RSC(TECH_L2_HW_CL_PREFETCH).CODE(),
		width - (OutFunc ? 20 : 22) - RSZ(TECH_L2_HW_CL_PREFETCH),
		NULL,
		BOXKEY_L2_HW_CL_PREFETCH,
		L2_HW_CL_Prefetch_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.SMM == 1,
		2, "%s%.*sSMM-Dual   [%3s]",
		RSC(TECHNOLOGIES_SMM).CODE(),
		width - 19 - RSZ(TECHNOLOGIES_SMM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.HyperThreading == 1,
		2, "%s%.*sHTT   [%3s]",
		RSC(TECHNOLOGIES_HTT).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_HTT),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.EIST == 1,
		2, "%s%.*sEIST   <%3s>",
		RSC(TECHNOLOGIES_EIST).CODE(),
		width - 15 - RSZ(TECHNOLOGIES_EIST),
		NULL,
		BOXKEY_EIST,
		SpeedStepUpdate
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.Power.EAX.TurboIDA == 1,
		2, "%s%.*sIDA   [%3s]",
		RSC(TECHNOLOGIES_IDA).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_IDA),
		NULL,
		SCANKEY_NULL,
		IDA_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.Turbo == 1,
		2, "%s%.*sTURBO   <%3s>",
		RO(Shm)->Proc.Features.Power.EAX.Turbo_V3 == 1 ?
		RSC(TECHNOLOGIES_TBMT3).CODE() : RSC(TECHNOLOGIES_TURBO).CODE(),
		width - 16 - (RO(Shm)->Proc.Features.Power.EAX.Turbo_V3 == 1 ?
		RSZ(TECHNOLOGIES_TBMT3) : RSZ(TECHNOLOGIES_TURBO)),
		NULL,
		BOXKEY_TURBO,
		TurboUpdate
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.EEO_Enable == 1,
		2, "%s%.*sEEO   <%3s>",
		RSC(TECHNOLOGIES_EEO).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_EEO),
		NULL,
		BOXKEY_EEO,
		EEO_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Features.R2H_Enable == 1,
		2, "%s%.*sR2H   <%3s>",
		RSC(TECHNOLOGIES_R2H).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_R2H),
		NULL,
		BOXKEY_R2H,
		R2H_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.WDT == 1,
		2, "%s%.*sTCO   <%3s>",
		RSC(TECHNOLOGIES_WDT).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_WDT),
		NULL,
		BOXKEY_WDT,
		WDT_Update
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.VM == 1,
		2, "%s%.*sVMX   [%3s]",
		RSC(TECHNOLOGIES_VM).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_VM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_INTEL, 0 },
		RO(Shm)->Proc.Technology.IOMMU == 1,
		3, "%s%.*sVT-d   [%3s]",
		RSC(TECHNOLOGIES_IOMMU).CODE(),
		width - (OutFunc ? 16 : 18) - RSZ(TECHNOLOGIES_IOMMU),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.SMM == 1,
		2, "%s%.*sSMM-Lock   [%3s]",
		RSC(TECHNOLOGIES_SMM).CODE(),
		width - 19 - RSZ(TECHNOLOGIES_SMM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Features.HyperThreading == 1,
		2, "%s%.*sSMT   [%3s]",
		RSC(TECHNOLOGIES_SMT).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_SMT),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.PowerNow == 0b11, /*	VID + FID	*/
		2, "%s%.*sCnQ   [%3s]",
		RSC(TECHNOLOGIES_CNQ).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_CNQ),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
	     RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.CStateBaseAddr != 0,
		2, "%s%.*sCCx   [%3s]",
		RSC(PERF_MON_CORE_CSTATE).CODE(),
		width - 14 - RSZ(PERF_MON_CORE_CSTATE),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.Turbo == 1,
		2, "%s%.*sCPB   <%3s>",
		RSC(TECHNOLOGIES_CPB).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_CPB),
		NULL,
		BOXKEY_TURBO,
		TurboUpdate
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.WDT == 1,
		2, "%s%.*sWDT   <%3s>",
		RSC(TECHNOLOGIES_WDT).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_WDT),
		NULL,
		BOXKEY_WDT,
		WDT_Update
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.VM == 1,
		2, "%s%.*sSVM   [%3s]",
		RSC(TECHNOLOGIES_VM).CODE(),
		width - 14 - RSZ(TECHNOLOGIES_VM),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		(unsigned int[]) { CRC_AMD, CRC_HYGON, 0 },
		RO(Shm)->Proc.Technology.IOMMU == 1,
		3, "%s%.*sAMD-V   [%3s]",
		RSC(TECHNOLOGIES_IOMMU).CODE(),
		width - (OutFunc? 17 : 19) - RSZ(TECHNOLOGIES_IOMMU),
		NULL,
		SCANKEY_NULL,
		NULL
	},
	{
		NULL,
		(StrFormat(IOMMU_Version_String, 10+1+10+1+1, "%d.%d",
			RO(Shm)->Proc.Technology.IOMMU_Ver_Major,
			RO(Shm)->Proc.Technology.IOMMU_Ver_Minor) > 0) & 0x0,
		3, "%s%.*s""   [%12s]",
		RSC(VERSION).CODE(),
		width - (OutFunc? 21 : 23) - RSZ(VERSION),
		(RO(Shm)->Proc.Technology.IOMMU_Ver_Major > 0
		|| RO(Shm)->Proc.Technology.IOMMU_Ver_Minor > 0) ?
		IOMMU_Version_String : (char*) RSC(NOT_AVAILABLE).CODE(),
		SCANKEY_NULL,
		NULL
	},
	{
		NULL,
		RO(Shm)->Proc.Features.Std.ECX.Hyperv == 1,
		3, "%s%.*s""%10s   [%3s]",
		RSC(TECHNOLOGIES_HYPERV).CODE(),
		width - (OutFunc? 22 : 24) - RSZ(TECHNOLOGIES_HYPERV),
		(char*) Hypervisor[RO(Shm)->Proc.HypervisorID],
		SCANKEY_NULL,
		NULL
	},
	{
		NULL,
		0,
		3, "%s%.*s""   [%12s]",
		RSC(VENDOR_ID).CODE(),
		width - (OutFunc? 21 : 23) - RSZ(VENDOR_ID),
		strlen(RO(Shm)->Proc.Features.Info.Hypervisor.ID) > 0 ?
		RO(Shm)->Proc.Features.Info.Hypervisor.ID
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
		if ( (*CRC) == RO(Shm)->Proc.Features.Info.Vendor.CRC ) {
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
	return reason;
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

void C1E_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.C1E == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C1A_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.C1A == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C3A_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.C3A == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C1U_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.C1U == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void C3U_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.C3U == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void CC6_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.CC6 == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void PC6_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.PC6 == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void HWP_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Features.HWP_Enable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void Refresh_HWP_Cap_Freq(TGrid *grid, DATA_TYPE data[])
{
	ATTRIBUTE *HWP_Cap_Attr[2] = {
		RSC(SYSINFO_PERFMON_HWP_CAP_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_HWP_CAP_COND1).ATTR()
	};
	const unsigned int bix	= (RO(Shm)->Proc.Features.HWP_Enable == 1)
				| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

	memcpy(grid->cell.attr, HWP_Cap_Attr[bix], grid->cell.length);

	RefreshRatioFreq(grid, data);
}

void HDC_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Features.HDC_Enable == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	PerfMonUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void IOMWAIT_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = \
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.IORedir == 1;

	const signed int pos = grid->cell.length - 9;
	UNUSED(data);

	PerfMonUpdate( grid, bix ? 3 : 2, pos, 7,
		(char *)(bix ? RSC(ENABLE).CODE() : RSC(DISABLE).CODE()) );
}

void CStateLimit_Update(TGrid *grid, DATA_TYPE data[])
{
	const ASCII *CST_Encoding[] = {
		[ _C0]	= RSC(PERF_ENCODING_C0).CODE(),
		[ _C1]	= RSC(PERF_ENCODING_C1).CODE(),
		[ _C2]	= RSC(PERF_ENCODING_C2).CODE(),
		[ _C3]	= RSC(PERF_ENCODING_C3).CODE(),
		[ _C4]	= RSC(PERF_ENCODING_C4).CODE(),
		[ _C6]	= RSC(PERF_ENCODING_C6).CODE(),
		[_C6R]	= RSC(PERF_ENCODING_C6R).CODE(),
		[ _C7]	= RSC(PERF_ENCODING_C7).CODE(),
		[_C7S]	= RSC(PERF_ENCODING_C7S).CODE(),
		[ _C8]	= RSC(PERF_ENCODING_C8).CODE(),
		[ _C9]	= RSC(PERF_ENCODING_C9).CODE(),
		[_C10]	= RSC(PERF_ENCODING_C10).CODE(),
		[_UNSPEC]=RSC(PERF_ENCODING_UNS).CODE()
	};
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	memcpy(&grid->cell.item[pos],
		CST_Encoding[
			RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core
		].Query.CStateLimit], 3);
}

void CStateRange_Update(TGrid *grid, DATA_TYPE data[])
{
	const ASCII *CST_Encoding[] = {
		[ _C0]	= RSC(PERF_ENCODING_C0).CODE(),
		[ _C1]	= RSC(PERF_ENCODING_C1).CODE(),
		[ _C2]	= RSC(PERF_ENCODING_C2).CODE(),
		[ _C3]	= RSC(PERF_ENCODING_C3).CODE(),
		[ _C4]	= RSC(PERF_ENCODING_C4).CODE(),
		[ _C6]	= RSC(PERF_ENCODING_C6).CODE(),
		[_C6R]	= RSC(PERF_ENCODING_C6R).CODE(),
		[ _C7]	= RSC(PERF_ENCODING_C7).CODE(),
		[_C7S]	= RSC(PERF_ENCODING_C7S).CODE(),
		[ _C8]	= RSC(PERF_ENCODING_C8).CODE(),
		[ _C9]	= RSC(PERF_ENCODING_C9).CODE(),
		[_C10]	= RSC(PERF_ENCODING_C10).CODE(),
		[_UNSPEC]=RSC(PERF_ENCODING_UNS).CODE()
	};
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	memcpy(&grid->cell.item[pos],
		CST_Encoding[
			RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core
		].Query.CStateInclude], 3);
}

REASON_CODE SysInfoPerfMon(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	const ASCII *CST_Encoding[] = {
		[ _C0]	= RSC(PERF_ENCODING_C0).CODE(),
		[ _C1]	= RSC(PERF_ENCODING_C1).CODE(),
		[ _C2]	= RSC(PERF_ENCODING_C2).CODE(),
		[ _C3]	= RSC(PERF_ENCODING_C3).CODE(),
		[ _C4]	= RSC(PERF_ENCODING_C4).CODE(),
		[ _C6]	= RSC(PERF_ENCODING_C6).CODE(),
		[_C6R]	= RSC(PERF_ENCODING_C6R).CODE(),
		[ _C7]	= RSC(PERF_ENCODING_C7).CODE(),
		[_C7S]	= RSC(PERF_ENCODING_C7S).CODE(),
		[ _C8]	= RSC(PERF_ENCODING_C8).CODE(),
		[ _C9]	= RSC(PERF_ENCODING_C9).CODE(),
		[_C10]	= RSC(PERF_ENCODING_C10).CODE(),
		[_UNSPEC]=RSC(PERF_ENCODING_UNS).CODE()
	};
	ATTRIBUTE *attrib[5] = {
		RSC(SYSINFO_PERFMON_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_COND1).ATTR(),
		RSC(SYSINFO_PERFMON_COND2).ATTR(),
		RSC(SYSINFO_PERFMON_COND3).ATTR(),
		RSC(SYSINFO_PERFMON_COND4).ATTR()
	};
	unsigned int bix;
/* Section Mark */
    if (RO(Shm)->Proc.PM_version > 0)
    {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s%s       [%3d]", RSC(VERSION).CODE(),
		width - 17 - RSZ(VERSION), hSpace,
		RSC(PERF_LABEL_VER).CODE(), RO(Shm)->Proc.PM_version );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s%s       [%3s]", RSC(VERSION).CODE(),
		width - 17 - RSZ(VERSION), hSpace,
		RSC(PERF_LABEL_VER).CODE(), RSC(NOT_AVAILABLE).CODE() );
    }
	PUT(	SCANKEY_NULL, attrib[0], width, 2, "%s:%.*s%s%.*s%s",
		RSC(COUNTERS).CODE(),
		8 + (10 - RSZ(COUNTERS)), hSpace,
		RSC(GENERAL_CTRS).CODE(),
		width - (54 + RSZ(GENERAL_CTRS)), hSpace,
		RSC(FIXED_CTRS).CODE() );

    if (OutFunc == NULL) {
	PUT(	SCANKEY_NULL, attrib[0], width, 1,
		"%.*s{%3u,%3u,%3u } x%3u %s%.*s%3u x%3u %s",
		11, hSpace,	RO(Shm)->Proc.Features.PerfMon.EAX.MonCtrs,
				RO(Shm)->Proc.Features.Factory.PMC.LLC,
				RO(Shm)->Proc.Features.Factory.PMC.NB,
				RO(Shm)->Proc.Features.PerfMon.EAX.MonWidth,
				RSC(PERF_MON_UNIT_BIT).CODE(),
		10, hSpace,	RO(Shm)->Proc.Features.PerfMon.EDX.FixCtrs,
				RO(Shm)->Proc.Features.PerfMon.EDX.FixWidth,
				RSC(PERF_MON_UNIT_BIT).CODE() );
    } else {
	GridHover( PUT( SCANKEY_NULL, attrib[0], width, 0,
		"%.*s{%3u,%3u,%3u } x%3u %s%.*s%3u x%3u %s",
		11, hSpace,	RO(Shm)->Proc.Features.PerfMon.EAX.MonCtrs,
				RO(Shm)->Proc.Features.Factory.PMC.LLC,
				RO(Shm)->Proc.Features.Factory.PMC.NB,
				RO(Shm)->Proc.Features.PerfMon.EAX.MonWidth,
				RSC(PERF_MON_UNIT_BIT).CODE(),
		4, hSpace,	RO(Shm)->Proc.Features.PerfMon.EDX.FixCtrs,
				RO(Shm)->Proc.Features.PerfMon.EDX.FixWidth,
				RSC(PERF_MON_UNIT_BIT).CODE() ),
		(char *) RSC(PERF_MON_PMC_COMM).CODE() );
    }
/* Section Mark */
	bix = RO(Shm)->Proc.Technology.C1E == 1;

	GridCall( PUT(	BOXKEY_C1E, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C1E).CODE(),
			width - 18 - RSZ(PERF_MON_C1E), hSpace,
			RSC(PERF_LABEL_C1E).CODE(), ENABLED(bix) ),
		C1E_Update );
/* Row Mark */
    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = RO(Shm)->Proc.Technology.C1A == 1;

	GridCall( PUT(	BOXKEY_C1A, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C1A).CODE(),
			width - 18 - RSZ(PERF_MON_C1A), hSpace,
			RSC(PERF_LABEL_C1A).CODE(), ENABLED(bix) ),
		C1A_Update );
/* Row Mark */
	bix = RO(Shm)->Proc.Technology.C3A == 1;

	GridCall( PUT(	BOXKEY_C3A, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C3A).CODE(),
			width - 18 - RSZ(PERF_MON_C3A), hSpace,
			RSC(PERF_LABEL_C3A).CODE(), ENABLED(bix) ),
		C3A_Update );
    }
/* Row Mark */
	bix = RO(Shm)->Proc.Technology.C1U == 1;

    if ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
     ||  (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	GridCall( PUT(	BOXKEY_C1U, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C2U).CODE(),
			width - 18 - RSZ(PERF_MON_C2U), hSpace,
			RSC(PERF_LABEL_C2U).CODE(), ENABLED(bix) ),
		C1U_Update );
    } else {
	GridCall( PUT(	BOXKEY_C1U, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C1U).CODE(),
			width - 18 - RSZ(PERF_MON_C1U), hSpace,
			RSC(PERF_LABEL_C1U).CODE(), ENABLED(bix) ),
		C1U_Update );
    }
/* Row Mark */
	bix = RO(Shm)->Proc.Technology.C3U == 1;

	GridCall( PUT(	BOXKEY_C3U, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C3U).CODE(),
			width - 18 - RSZ(PERF_MON_C3U), hSpace,
			RSC(PERF_LABEL_C3U).CODE(), ENABLED(bix) ),
		C3U_Update );
/* Row Mark */
    if((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	bix = RO(Shm)->Proc.Technology.CC6 == 1;

	GridCall( PUT(	BOXKEY_CC6, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_CC6).CODE(),
			width - 18 - RSZ(PERF_MON_CC6), hSpace,
			RSC(PERF_LABEL_CC6).CODE(), ENABLED(bix) ),
		CC6_Update );
/* Row Mark */
	bix = RO(Shm)->Proc.Technology.PC6 == 1;

	GridCall( PUT(	BOXKEY_PC6, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_PC6).CODE(),
			width - 18 - RSZ(PERF_MON_PC6), hSpace,
			RSC(PERF_LABEL_PC6).CODE(), ENABLED(bix) ),
		PC6_Update );
    } else {
	bix = RO(Shm)->Proc.Technology.CC6 == 1;

	GridCall( PUT(	BOXKEY_CC6, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_C6D).CODE(),
			width - 18 - RSZ(PERF_MON_C6D), hSpace,
			RSC(PERF_LABEL_CC6).CODE(), ENABLED(bix) ),
		CC6_Update );

	bix = RO(Shm)->Proc.Technology.PC6 == 1;

	GridCall( PUT(	BOXKEY_PC6, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_MC6).CODE(),
			width - 18 - RSZ(PERF_MON_MC6), hSpace,
			RSC(PERF_LABEL_MC6).CODE(), ENABLED(bix) ),
		PC6_Update );
    }
/* Section Mark */
	bix = RO(Shm)->Proc.Features.AdvPower.EDX.FID == 1;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s       [%3s]", RSC(PERF_MON_FID).CODE(),
		width - 18 - RSZ(PERF_MON_FID), hSpace,
		RSC(PERF_LABEL_FID).CODE(), ENABLED(bix) );
/* Row Mark */
	bix = (RO(Shm)->Proc.Features.AdvPower.EDX.VID == 1);

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s       [%3s]", RSC(PERF_MON_VID).CODE(),
		width - 18 - RSZ(PERF_MON_VID), hSpace,
		RSC(PERF_LABEL_VID).CODE(), ENABLED(bix) );
/* Row Mark */
	bix = (RO(Shm)->Proc.Features.Power.ECX.HCF_Cap == 1)
	   || ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		&& (RO(Shm)->Proc.Features.AdvPower.EDX.EffFrqRO == 1))
	   || ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON)
		&& (RO(Shm)->Proc.Features.AdvPower.EDX.EffFrqRO == 1));

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s       [%3s]", RSC(PERF_MON_HWCF).CODE(),
		width - 26 - RSZ(PERF_MON_HWCF), hSpace,
		RSC(PERF_LABEL_HWCF).CODE(), ENABLED(bix) );
/* Section Mark */
	bix = (RO(Shm)->Proc.Features.Power.EAX.HWP_Reg == 1)	/* Intel:HWP */
	|| (RO(Shm)->Proc.Features.leaf80000008.EBX.CPPC == 1)	/* AMD:CPPC  */
	|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);		/* ACPI:CPPC */
    if (bix)
    {
	CPU_STRUCT *SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core];
	struct FLIP_FLOP *CFlop = &SProc->FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];
	ATTRIBUTE *HWP_Cap_Attr[2] = {
		RSC(SYSINFO_PERFMON_HWP_CAP_COND0).ATTR(),
		RSC(SYSINFO_PERFMON_HWP_CAP_COND1).ATTR()
	};
	bix = RO(Shm)->Proc.Features.HWP_Enable == 1;

     if  ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	|| (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
     {
      if ((bix |= RO(Shm)->Proc.Features.ACPI_CPPC) == 1)
      {
	GridHover( PUT( BOXKEY_FMW_CPPC, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_CPPC).CODE(),
			width - 19 - RSZ(PERF_MON_CPPC), hSpace,
			RSC(PERF_LABEL_CPPC).CODE(), RSC(FMW).CODE() ),
		(char *) RSC(PERF_MON_CPPC_COMM).CODE() );
      }
      else
      {
	GridCall( PUT(	BOXKEY_HWP, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_CPPC).CODE(),
			width - 19 - RSZ(PERF_MON_CPPC), hSpace,
			RSC(PERF_LABEL_CPPC).CODE(), ENABLED(bix) ),
		HWP_Update);
      }
     } else {
	GridCall( PUT(	BOXKEY_HWP, attrib[bix], width, 2,
			"%s%.*s%s       <%3s>", RSC(PERF_MON_HWP).CODE(),
			width - 18 - RSZ(PERF_MON_HWP), hSpace,
			RSC(PERF_LABEL_HWP).CODE(), ENABLED(bix) ),
		HWP_Update);
     }
	PUT(	SCANKEY_NULL, RSC(SYSINFO_PERFMON_HWP_CAP_COND1).ATTR(),
		width, 3, "%s""%.*s""%s""%.*s""%s", RSC(CAPABILITIES).CODE(),
		21 - 3 * (OutFunc == NULL) - RSZ(CAPABILITIES), hSpace,
		RSC(PERF_MON_UNIT_HWP).CODE(), 22, hSpace, RSC(RATIO).CODE() );

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
    }
    else if ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	   || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	PUT(	SCANKEY_NULL, attrib[4], width, 2,
		"%s%.*s%s       [%3s]", RSC(PERF_MON_CPPC).CODE(),
		width - 19 - RSZ(PERF_MON_CPPC), hSpace,
		RSC(PERF_LABEL_CPPC).CODE(), RSC(NOT_AVAILABLE).CODE() );
    } else {
	const unsigned int cix	= ((RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1));

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s       [%3s]", RSC(PERF_MON_HWP).CODE(),
		width - 18 - RSZ(PERF_MON_HWP), hSpace,
		RSC(PERF_LABEL_HWP).CODE(), ENABLED(cix) );
    }
/* Section Mark */
    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = RO(Shm)->Proc.Features.HDC_Enable == 1;

	if (RO(Shm)->Proc.Features.Power.EAX.HDC_Reg)
	{
		GridCall( PUT(	BOXKEY_HDC,
				attrib[bix], width, 2,
				"%s%.*s%s       <%3s>",RSC(PERF_MON_HDC).CODE(),
				width - 18 - RSZ(PERF_MON_HDC), hSpace,
				RSC(PERF_LABEL_HDC).CODE(), ENABLED(bix) ),
			HDC_Update );
	}
	else
	{
		PUT(	SCANKEY_NULL,
			attrib[0], width, 2,
			"%s%.*s%s       [%3s]", RSC(PERF_MON_HDC).CODE(),
			width - 18 - RSZ(PERF_MON_HDC), hSpace,
			RSC(PERF_LABEL_HDC).CODE(), ENABLED(bix) );
	}
/* Section Mark */
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s", RSC(PERF_MON_PKG_CSTATE).CODE() );

	bix = RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.CfgLock == 0 ? 3:0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 3,
		"%s%.*s%s   [%7s]", RSC(PERF_MON_CFG_CTRL).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(PERF_MON_CFG_CTRL), hSpace,
		RSC(PERF_LABEL_CFG_CTRL).CODE(),
		!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.CfgLock ?
			RSC(UNLOCK).CODE() : RSC(LOCK).CODE() );

	GridCall( PUT(	BOXKEY_PKGCST, attrib[0], width, 3,
			"%s%.*s%s   <%7s>", RSC(PERF_MON_LOW_CSTATE).CODE(),
			width - (OutFunc == NULL ? 23 : 21)
			- RSZ(PERF_MON_LOW_CSTATE), hSpace,
			RSC(PERF_LABEL_LOW_CST).CODE(),
			CST_Encoding[RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Core
						].Query.CStateLimit] ),
		CStateLimit_Update );

	bix = RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.IORedir == 1 ? 3:2;

	GridCall( PUT(	BOXKEY_IOMWAIT, attrib[bix], width, 3,
			"%s%.*s%s   <%7s>", RSC(PERF_MON_IOMWAIT).CODE(),
			width - (OutFunc == NULL ? 25 : 23)
			- RSZ(PERF_MON_IOMWAIT), hSpace,
			RSC(PERF_LABEL_IOMWAIT).CODE(),
			RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.IORedir ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE() ),
		IOMWAIT_Update );

	GridCall( PUT(	BOXKEY_IORCST, attrib[0], width, 3,
			"%s%.*s%s   <%7s>", RSC(PERF_MON_MAX_CSTATE).CODE(),
			width - (OutFunc == NULL ? 23 : 21)
			- RSZ(PERF_MON_MAX_CSTATE), hSpace,
			RSC(PERF_LABEL_MAX_CST).CODE(),
			CST_Encoding[RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Core
						].Query.CStateInclude] ),
		CStateRange_Update );
    }
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s", RSC(PERF_MON_CORE_CSTATE).CODE() );

	PUT(	SCANKEY_NULL,
		attrib[ !RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].Query.CStateBaseAddr ? 0 : 2 ],
		width, 3,
		"%s%.*s%s   [ 0x%-4X]", RSC(PERF_MON_CSTATE_BAR).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		- RSZ(PERF_MON_CSTATE_BAR), hSpace,
		RSC(PERF_LABEL_CST_BAR).CODE(),
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.CStateBaseAddr );
/* Section Mark */
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s", RSC(PERF_MON_MONITOR_MWAIT).CODE() );

	PUT(	SCANKEY_NULL, attrib[0], width, 3, "%s:%.*s%s",
		RSC(PERF_MON_MWAIT_IDX_CSTATE).CODE(), 04, hSpace,
		RSC(PERF_LABEL_MWAIT_IDX).CODE() );

	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s:%.*s%2d    %2d    %2d    %2d    %2d    %2d    %2d    %2d",
		RSC(PERF_MON_MWAIT_SUB_CSTATE).CODE(),
		15 - RSZ(PERF_MON_MWAIT_SUB_CSTATE), hSpace,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT0,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT1,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT2,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT3,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT4,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT5,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT6,
		RO(Shm)->Proc.Features.MWait.EDX.SubCstate_MWAIT7 );
/* Section Mark */
	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.CoreCycles == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz > 0) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_CORE_CYCLE).CODE(),
		width - 12 - RSZ(PERF_MON_CORE_CYCLE), hSpace, POWERED(bix) );

	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.InstrRetired == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 1) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_INST_RET).CODE(),
		width - 12 - RSZ(PERF_MON_INST_RET), hSpace, POWERED(bix) );

	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.RefCycles == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 2) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_REF_CYCLE).CODE(),
		width - 12 - RSZ(PERF_MON_REF_CYCLE), hSpace, POWERED(bix) );

	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.LLC_Ref == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 3) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_REF_LLC).CODE(),
		width - 12 - RSZ(PERF_MON_REF_LLC), hSpace, POWERED(bix) );

    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.LLC_Misses == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 4) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_MISS_LLC).CODE(),
		width - 12 - RSZ(PERF_MON_MISS_LLC), hSpace, POWERED(bix) );

	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.BranchRetired == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 5) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_BRANCH_RET).CODE(),
		width - 12 - RSZ(PERF_MON_BRANCH_RET), hSpace, POWERED(bix) );

	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.BranchMispred == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 6) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_BRANCH_MIS).CODE(),
		width - 12 - RSZ(PERF_MON_BRANCH_MIS), hSpace, POWERED(bix) );

	bix = (RO(Shm)->Proc.Features.PerfMon.EBX.TopdownSlots == 0)
	   || (RO(Shm)->Proc.Features.PerfMon.EAX.VectorSz >= 7) ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_TOPDOWN_SLOTS).CODE(),
		width - 12 - RSZ(PERF_MON_TOPDOWN_SLOTS), hSpace, POWERED(bix));
    }
    else if( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
      if (RO(Shm)->Proc.Features.ExtInfo.ECX.CU_PTSC == 1) {
	PUT(	SCANKEY_NULL, attrib[2], width, 2,
		"%s x%3u %s%.*s[%7s]", RSC(PERF_MON_TSC).CODE(),
		(unsigned int[]) { 40, 48, 56, 64 } [
			RO(Shm)->Proc.Features.leaf80000008.ECX.CU_PTSC_Size
		],
		RSC(PERF_MON_UNIT_BIT).CODE(),
		width - 18 - RSZ(PERF_MON_TSC) - RSZ(PERF_MON_UNIT_BIT),
		hSpace, POWERED(2) );
      } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_TSC).CODE(),
		width - 12 - RSZ(PERF_MON_TSC), hSpace, POWERED(0) );
      }

	bix = RO(Shm)->Proc.Features.ExtInfo.ECX.PerfNB == 1 ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_NB_DF).CODE(),
		width - 12 - RSZ(PERF_MON_NB_DF), hSpace, POWERED(bix) );

	bix = RO(Shm)->Proc.Features.ExtInfo.ECX.PerfCore == 1 ? 2 : 0;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s[%7s]", RSC(PERF_MON_CORE).CODE(),
		width - 12 - RSZ(PERF_MON_CORE), hSpace, POWERED(bix) );
    }
	return reason;
}

void PwrThermalUpdate(TGrid *grid, const unsigned int bix,const signed int pos,
				const size_t len, const char *item)
{
	ATTRIBUTE *attrib[7] = {
		RSC(SYSINFO_PWR_THERMAL_COND0).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND1).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND2).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND3).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND4).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND5).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND6).ATTR()
	};
	memcpy(&grid->cell.attr[pos], &attrib[bix][pos], len);
	memcpy(&grid->cell.item[pos], item, len);
}

void ODCM_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Proc.Technology.ODCM == 1;
	const signed int pos = grid->cell.length - 9;
	UNUSED(data);

	PwrThermalUpdate( grid, bix ? 3 : 1, pos, 7,
		(char *)(bix ? RSC(ENABLE).CODE() : RSC(DISABLE).CODE()) );
}

void DutyCycle_Update(TGrid *grid, DATA_TYPE data[])
{
	const signed int pos = grid->cell.length - 10;
	const unsigned int bix = (RO(Shm)->Proc.Features.Std.EDX.ACPI == 1)
				&& (RO(Shm)->Proc.Technology.ODCM == 1);
	char item[10+1];
	UNUSED(data);

	StrFormat(item, 10+1, "%c%6.2f%%%c",
		bix ? '<' : '[',
		(RO(Shm)->Cpu[
			RO(Shm)->Proc.Service.Core
		].PowerThermal.DutyCycle.Extended ? 6.25f : 12.5f)
		* RO(Shm)->Cpu[
			RO(Shm)->Proc.Service.Core
		].PowerThermal.DutyCycle.ClockMod,
		bix ? '>' : ']');

	memcpy(&grid->cell.item[pos], item, 9);
	grid->cell.quick.key = bix ? BOXKEY_DUTYCYCLE : SCANKEY_NULL;
}

void Hint_Update(TGrid *grid, DATA_TYPE data[])
{
	const signed int pos = grid->cell.length - 9;
	char item[10+1];

	StrFormat(item, 10+1, "%7u", (*data[0].puint));
	memcpy(&grid->cell.item[pos], item, 7);
}

void TjMax_Update(TGrid *grid, DATA_TYPE data[])
{
	struct FLIP_FLOP *SFlop = &RO(Shm)->Cpu[
		RO(Shm)->Proc.Service.Core
	].FlipFlop[
		!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];
	const signed int pos = grid->cell.length - 11;
	char item[11+1+11+2+1];
	UNUSED(data);

	if (Setting.fahrCels) {
		StrFormat(item, 11+1+11+2+1, "%3d:%3d F",
			Cels2Fahr(SFlop->Thermal.Param.Offset[1]),
			Cels2Fahr(SFlop->Thermal.Param.Offset[0]));
	} else {
		StrFormat(item, 10+1+10+2+1, "%3hu:%3hu C",
			SFlop->Thermal.Param.Offset[1],
			SFlop->Thermal.Param.Offset[0]);
	}
	memcpy(&grid->cell.item[pos], item, 9);
}

void TDP_State(TGrid *grid, DATA_TYPE data[])
{
	const enum PWR_DOMAIN pw = (enum PWR_DOMAIN) data[0].sint[0];
	const unsigned int bix = \
			  RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable
			| RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Enable;
	const signed int pos = grid->cell.length - 9;

	PwrThermalUpdate( grid, bix ? 3 : 1, pos, 7,
		(char *)(bix ? RSC(ENABLE).CODE() : RSC(DISABLE).CODE()) );
}

void PCT_Update(TGrid *grid, const char *item, const unsigned int cix)
{
	ATTRIBUTE *attrib[7] = {
		RSC(SYSINFO_PWR_THERMAL_COND0).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND1).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND2).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND3).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND4).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND5).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND6).ATTR()
	};
	const signed int pos = grid->cell.length - 9;

	memcpy(&grid->cell.attr[pos], &attrib[cix][pos], 7);
	memcpy(&grid->cell.item[pos], item, 7);
}

void TDP_Update(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	char item[7+1];
	StrFormat(item, 7+1, "%5u W", RO(Shm)->Proc.Power.TDP);

	PCT_Update(grid, item, RO(Shm)->Proc.Power.TDP > 0 ? 5 : 0);
}

void PWL_Update(TGrid *grid, DATA_TYPE data[])
{
	const enum PWR_DOMAIN pw = (enum PWR_DOMAIN) data[0].sint[0];
	const enum PWR_LIMIT pl = (enum PWR_LIMIT) data[1].uint[0];
	char item[7+1];
	StrFormat(item, 7+1, "%5u W", RO(Shm)->Proc.Power.Domain[pw].PWL[pl]);

	PCT_Update(	grid, item, RO(Shm)->Proc.Power.Domain[pw].PWL[pl] > 0 ?
			RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable ?
			3 : 5 : 0 );
}

char *FormatTime(const size_t fsz, char *str, const double fTime)
{
    if (fTime >= 1.0) {
	unsigned long long iTime, rTime;
	if (fTime >= 36000.0) {
		iTime = fTime / 86400LLU;
		rTime = fTime - (iTime * 86400LLU);
		rTime = (100LLU * rTime) / 86400LLU;
		StrFormat(str, fsz, "%2llu.%02llu d", iTime, rTime);
	} else if (fTime >= 60.0) {
		unsigned long long hTW = fTime / 3600LLU, mTime;
		iTime = fTime / 60LLU;
		rTime = fTime - (iTime * 60LLU);
		mTime = (fTime - (hTW * 3600LLU)) / 60LLU;
	    if (hTW) {
		StrFormat(str, fsz, "%1lluh%02llum%02llu", hTW, mTime, rTime);
	    } else if (rTime) {
		StrFormat(str, fsz, " %2llum%02llus", mTime, rTime);
	    } else {
		StrFormat(str, fsz, "   %2llu m", mTime);
	    }
	} else {
		iTime = fTime;
		rTime = (100LLU * fTime) - (100LLU * iTime);
	    if (rTime) {
		StrFormat(str, fsz, "%2llu.%02llu s", iTime, rTime);
	    } else {
		StrFormat(str, fsz, "   %2llu s", iTime);
	    }
	}
    } else {
	unsigned long long iTime;
	char unit;
	if (fTime < 0.000001) {
		iTime = 1000LLU * 1000LLU * 1000LLU * fTime;
		unit = 'n';
	} else if (fTime < 0.001) {
		iTime = 1000LLU * 1000LLU * fTime;
		unit = 'u';
	} else {
		iTime = 1000LLU * fTime;
		unit = 'm';
	}
	StrFormat(str, fsz, "%4llu %cs", iTime, unit);
    }
	return str;
}

void TAU_Update(TGrid *grid, DATA_TYPE data[])
{
	const enum PWR_DOMAIN pw = (enum PWR_DOMAIN) data[0].sint[0];
	const enum PWR_LIMIT pl = (enum PWR_LIMIT) data[1].uint[0];

	char *item = malloc(64);
    if (item != NULL) {
	PCT_Update(grid,
		FormatTime(64, item, RO(Shm)->Proc.Power.Domain[pw].TAU[pl]),
		RO(Shm)->Proc.Power.Domain[pw].TAU[pl] > 0 ?
		RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable ? 3 : 5 : 0);

	free(item);
    }
}

void TDC_Update(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	char item[7+1];
	StrFormat(item, 7+1, "%5u A", RO(Shm)->Proc.Power.TDC);

	PCT_Update(grid, item, RO(Shm)->Proc.Power.TDC > 0 ?
		RO(Shm)->Proc.Power.Feature.TDC ? 3 : 5 : 0);
}

REASON_CODE SysInfoPwrThermal(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	ATTRIBUTE *attrib[7] = {
		RSC(SYSINFO_PWR_THERMAL_COND0).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND1).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND2).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND3).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND4).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND5).ATTR(),
		RSC(SYSINFO_PWR_THERMAL_COND6).ATTR()
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
	struct FLIP_FLOP *SFlop = &RO(Shm)->Cpu[
		RO(Shm)->Proc.Service.Core
	].FlipFlop[
		!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];
	unsigned int bix;
/* Section Mark */
	GridCall( PUT(	SCANKEY_NULL, attrib[6], width, 2,
			"%s%.*s%s [%3u:%3u %c]",
			RSC(POWER_THERMAL_TJMAX).CODE(),
			width - 20 - RSZ(POWER_THERMAL_TJMAX), hSpace,
			RSC(POWER_LABEL_TJ).CODE(),
			Setting.fahrCels ? Cels2Fahr(
				SFlop->Thermal.Param.Offset[1]
			) : SFlop->Thermal.Param.Offset[1],
			Setting.fahrCels ? Cels2Fahr(
				SFlop->Thermal.Param.Offset[0]
			) : SFlop->Thermal.Param.Offset[0],
			Setting.fahrCels ? 'F' : 'C' ),
		TjMax_Update );
/* Section Mark */
  if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
	bix = RO(Shm)->Proc.Features.Std.EDX.ACPI == 1;

	GridCall( PUT(	bix ? BOXKEY_ODCM : SCANKEY_NULL,
			attrib[(RO(Shm)->Proc.Technology.ODCM == 1) ? 3 : 1],
			width, 2,
			"%s%.*s%s   %c%7s%c", RSC(POWER_THERMAL_ODCM).CODE(),
			width - 19 - RSZ(POWER_THERMAL_ODCM), hSpace,
			RSC(POWER_LABEL_ODCM).CODE(),
			bix ? '<' : '[',
			(RO(Shm)->Proc.Technology.ODCM == 1) ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE(),
			bix ? '>' : ']' ),
		ODCM_Update );

	bix = (RO(Shm)->Proc.Features.Std.EDX.ACPI == 1)
	   && (RO(Shm)->Proc.Technology.ODCM == 1);

	GridCall( PUT(	bix ? BOXKEY_DUTYCYCLE : SCANKEY_NULL,
			attrib[0], width, 3,
			"%s%.*s%c%6.2f%%%c", RSC(POWER_THERMAL_DUTY).CODE(),
			width - (OutFunc == NULL ? 15: 13)
			 - RSZ(POWER_THERMAL_DUTY), hSpace,
			bix ? '<' : '[',
			(RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.DutyCycle.Extended ? 6.25f : 12.5f)
			* RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.DutyCycle.ClockMod,
			bix ? '>' : ']' ),
		DutyCycle_Update );
/* Row Mark */
	bix = RO(Shm)->Proc.Technology.PowerMgmt == 1;
	PUT(	SCANKEY_NULL, attrib[ bix ? 3 : 0 ], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_MGMT).CODE(),
		width - 23 - RSZ(POWER_THERMAL_MGMT), hSpace,
		RSC(POWER_LABEL_PWM).CODE(),
		Unlock[RO(Shm)->Proc.Technology.PowerMgmt] );

	bix = RO(Shm)->Proc.Features.Power.ECX.SETBH == 1;
    if (bix) {
	GridCall( PUT(	BOXKEY_PWR_POLICY, attrib[0], width, 3,
			"%s%.*s%s   <%7u>", RSC(POWER_THERMAL_BIAS).CODE(),
			width - (OutFunc == NULL ? 27 : 25)
			 - RSZ(POWER_THERMAL_BIAS), hSpace,
			RSC(POWER_LABEL_BIAS).CODE(),
			RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.PowerPolicy ),
		Hint_Update,
		&RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.PowerPolicy );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%7u]", RSC(POWER_THERMAL_BIAS).CODE(),
		width - (OutFunc == NULL ? 27 : 25)
		 - RSZ(POWER_THERMAL_BIAS), hSpace,
		RSC(POWER_LABEL_BIAS).CODE(),
		RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.PowerPolicy );
    }
/* Row Mark */
	bix = RO(Shm)->Proc.Features.HWP_Enable == 1;
    if (bix) {
	GridCall( PUT(	BOXKEY_HWP_EPP, attrib[0], width, 3,
			"%s%.*s%s   <%7u>", RSC(POWER_THERMAL_BIAS).CODE(),
			width - (OutFunc == NULL ? 25 : 23)
			 - RSZ(POWER_THERMAL_BIAS), hSpace,
			RSC(POWER_LABEL_EPP).CODE(),
			RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.HWP.Request.Energy_Pref ),
		Hint_Update,
		&RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.HWP.Request.Energy_Pref );
    }
  } else if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	 || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
  {
/* Row Mark */
	bix = RO(Shm)->Proc.Features.HWP_Enable == 1;
    if (bix) {
	GridCall( PUT(	BOXKEY_HWP_EPP, attrib[0], width, 2,
			"%s%.*s%s   <%7u>", RSC(POWER_THERMAL_CPPC).CODE(),
			width - 19 - RSZ(POWER_THERMAL_CPPC), hSpace,
			RSC(POWER_LABEL_CPPC).CODE(),
			RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.HWP.Request.Energy_Pref ),
		Hint_Update,
		&RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.HWP.Request.Energy_Pref );
    } else {
	bix = RO(Shm)->Proc.Features.leaf80000008.EBX.CPPC == 1;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_CPPC).CODE(),
		width - 19 - RSZ(POWER_THERMAL_CPPC), hSpace,
		RSC(POWER_LABEL_CPPC).CODE(), POWERED(bix) );
    }
  }
/* Row Mark */
	bix = (RO(Shm)->Proc.Features.Power.EAX.DTS == 1)
	   || (RO(Shm)->Proc.Features.AdvPower.EDX.TS == 1);

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_DTS).CODE(),
		width - 18 - RSZ(POWER_THERMAL_DTS), hSpace,
		RSC(POWER_LABEL_DTS).CODE(), POWERED(bix) );
/* Row Mark */
	bix = RO(Shm)->Proc.Features.Power.EAX.PLN == 1;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_PLN).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PLN), hSpace,
		RSC(POWER_LABEL_PLN).CODE(), POWERED(bix) );
/* Row Mark */
	bix = RO(Shm)->Proc.Features.Power.EAX.PTM == 1;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_PTM).CODE(),
		width - 18 - RSZ(POWER_THERMAL_PTM), hSpace,
		RSC(POWER_LABEL_PTM).CODE(), POWERED(bix) );
/* Section Mark */
    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	bix = RO(Shm)->Proc.Technology.TM1;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_TM1).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM1), hSpace,
		RSC(POWER_LABEL_TM1).CODE(), TM[bix] );

	bix = RO(Shm)->Proc.Technology.TM2;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_TM2).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM2), hSpace,
		RSC(POWER_LABEL_TM2).CODE(), TM[bix] );
    }
    else if((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	 || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	bix = RO(Shm)->Proc.Technology.TM1;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_TM1).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM1), hSpace,
		RSC(POWER_LABEL_TTP).CODE(), TM[bix] );

	bix = RO(Shm)->Proc.Technology.TM2;

	PUT(	SCANKEY_NULL, attrib[bix], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_TM2).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TM2), hSpace,
		RSC(POWER_LABEL_HTC).CODE(), TM[bix] );
    }
/* Section Mark */
    if (RO(Shm)->Proc.Power.TDP > 0) {
	GridCall( PUT(	SCANKEY_NULL, attrib[5], width, 2,
			"%s%.*s%s   [%5u W]", RSC(POWER_THERMAL_TDP).CODE(),
			width - 18 - RSZ(POWER_THERMAL_TDP), hSpace,
			RSC(POWER_LABEL_TDP).CODE(), RO(Shm)->Proc.Power.TDP ),
		TDP_Update );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_TDP).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TDP), hSpace,
		RSC(POWER_LABEL_TDP).CODE(), POWERED(0) );
    }
/* Section Mark */
    if (RO(Shm)->Proc.Power.Min > 0) {
	PUT(	SCANKEY_NULL, attrib[5], width, 3,
		"%s%.*s%s   [%5u W]", RSC(POWER_THERMAL_MIN).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MIN), hSpace,
		RSC(POWER_LABEL_MIN).CODE(), RO(Shm)->Proc.Power.Min );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_MIN).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MIN), hSpace,
		RSC(POWER_LABEL_MIN).CODE(), POWERED(0) );
    }
/* Section Mark */
    if (RO(Shm)->Proc.Power.Max > 0) {
	PUT(	SCANKEY_NULL, attrib[5], width, 3,
		"%s%.*s%s   [%5u W]", RSC(POWER_THERMAL_MAX).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MAX), hSpace,
		RSC(POWER_LABEL_MAX).CODE(), RO(Shm)->Proc.Power.Max );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_MAX).CODE(),
		width - (OutFunc == NULL ? 21 : 19)
		 - RSZ(POWER_THERMAL_MAX), hSpace,
		RSC(POWER_LABEL_MAX).CODE(), POWERED(0) );
    }
/* Section Mark */
	struct {
		const ASCII *code;
		const int size;
	} label[] = {
		{RSC(POWER_LABEL_PKG).CODE()	, RSZ(POWER_LABEL_PKG)},
		{RSC(POWER_LABEL_CORE).CODE()	, RSZ(POWER_LABEL_CORE)},
		{RSC(POWER_LABEL_UNCORE).CODE() , RSZ(POWER_LABEL_UNCORE)},
		{RSC(POWER_LABEL_DRAM).CODE()	, RSZ(POWER_LABEL_DRAM)},
		{RSC(POWER_LABEL_PLATFORM).CODE(),RSZ(POWER_LABEL_PLATFORM)}
	};
	enum PWR_DOMAIN pw;
    for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
    {
	char item[7+1];
	unsigned int cix;

	GridCall(
	    PUT(RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ?
		(BOXKEY_TDP_OR | (pw << 5) | PL1) : SCANKEY_NULL,
		attrib[
			RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable ? 3:1
		],
		width, 2,
		"%s%.*s%s   %c%7s%c",
		RSC(POWER_THERMAL_TDP).CODE(),
		width - 15 - RSZ(POWER_THERMAL_TDP) - label[pw].size,
		hSpace,
		label[pw].code,
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ? '<' : '[',
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable ?
				RSC(ENABLE).CODE() : RSC(DISABLE).CODE(),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ? '>' : ']'),
	TDP_State, pw );

	cix = RO(Shm)->Proc.Power.Domain[pw].PWL[PL1] > 0 ?
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable ? 3 : 5 : 0;

	GridCall(
	    PUT(RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ?
		(BOXKEY_TDP_OR | (pw << 5) | PL1) : SCANKEY_NULL,
		attrib[cix],
		width, 3,
		"%s%.*s%s   %c%5u W%c",
		RSC(POWER_THERMAL_TPL).CODE(),
		width - (OutFunc == NULL ? 21 : 19) - RSZ(POWER_THERMAL_TPL),
		hSpace,
		RSC(POWER_LABEL_PL1).CODE(),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ? '<' : '[',
		RO(Shm)->Proc.Power.Domain[pw].PWL[PL1],
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ? '>' : ']'),
	PWL_Update, pw, PL1 );

	cix = RO(Shm)->Proc.Power.Domain[pw].TAU[PL1] > 0 ?
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable ? 3 : 5 : 0;

	GridCall(
	    PUT(RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ?
		(BOXKEY_TW_OR | (pw << 5) | PL1) : SCANKEY_NULL,
		attrib[cix],
		width, 3,
		"%s%.*s%s   %c%s%c",
		RSC(POWER_THERMAL_TW).CODE(),
		width - (OutFunc == NULL ? 21 : 19) - RSZ(POWER_THERMAL_TW),
		hSpace,
		RSC(POWER_LABEL_TW1).CODE(),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ? '<' : '[',
		FormatTime(7+1, item, RO(Shm)->Proc.Power.Domain[pw].TAU[PL1]),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock ? '>' : ']'),
	TAU_Update, pw, PL1 );

      if (pw == PWR_DOMAIN(PKG) || pw == PWR_DOMAIN(PLATFORM))
      {
	cix = RO(Shm)->Proc.Power.Domain[pw].PWL[PL2] > 0 ?
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Enable ? 3 : 5 : 0;

	GridCall(
	    PUT(RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock ?
		(BOXKEY_TDP_OR | (pw << 5) | PL2) : SCANKEY_NULL,
		attrib[cix],
		width, 3,
		"%s%.*s%s   %c%5u W%c",
		RSC(POWER_THERMAL_TPL).CODE(),
		width - (OutFunc == NULL ? 21 : 19) - RSZ(POWER_THERMAL_TPL),
		hSpace,
		RSC(POWER_LABEL_PL2).CODE(),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock ? '<' : '[',
		RO(Shm)->Proc.Power.Domain[pw].PWL[PL2],
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock ? '>' : ']'),
	PWL_Update, pw, PL2 );

	cix = RO(Shm)->Proc.Power.Domain[pw].TAU[PL2] > 0 ?
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Enable ? 3 : 5 : 0;

	GridCall(
	    PUT(RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock ?
		(BOXKEY_TW_OR | (pw << 5) | PL2) : SCANKEY_NULL,
		attrib[cix],
		width, 3,
		"%s%.*s%s   %c%s%c",
		RSC(POWER_THERMAL_TW).CODE(),
		width - (OutFunc == NULL ? 21 : 19) - RSZ(POWER_THERMAL_TW),
		hSpace,
		RSC(POWER_LABEL_TW2).CODE(),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock ? '<' : '[',
		FormatTime(7+1, item, RO(Shm)->Proc.Power.Domain[pw].TAU[PL2]),
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock ? '>' : ']'),
	TAU_Update, pw, PL2 );
      }
    }
/* Section Mark */
    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    ||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	if (RO(Shm)->Proc.Power.PPT > 0) {
		PUT(	SCANKEY_NULL, attrib[5], width, 2,
			"%s%.*s%s   [%5u W]", RSC(POWER_THERMAL_PPT).CODE(),
			width - 18 - RSZ(POWER_THERMAL_PPT), hSpace,
			RSC(POWER_LABEL_PPT).CODE(), RO(Shm)->Proc.Power.PPT );
	} else {
		PUT(	SCANKEY_NULL, attrib[0], width, 2,
			"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_PPT).CODE(),
			width - 18 - RSZ(POWER_THERMAL_PPT), hSpace,
			RSC(POWER_LABEL_PPT).CODE(), POWERED(0) );
	}
    }
/* Row Mark */
    if (RO(Shm)->Proc.Power.EDC > 0) {
	PUT(	SCANKEY_NULL, attrib[5], width, 2,
		"%s%.*s%s   [%5u A]", RSC(POWER_THERMAL_EDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_EDC), hSpace,
		RSC(POWER_LABEL_EDC).CODE(), RO(Shm)->Proc.Power.EDC );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_EDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_EDC), hSpace,
		RSC(POWER_LABEL_EDC).CODE(), POWERED(0) );
    }
/* Row Mark */
    if (RO(Shm)->Proc.Power.TDC > 0) {
	bix	= (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
		&& RO(Shm)->Proc.Features.TDP_Unlock;

	GridCall( PUT(	bix ? BOXKEY_TDC : SCANKEY_NULL,
			attrib[ RO(Shm)->Proc.Power.Feature.TDC ? 3 : 5 ],
			width, 2,
			"%s%.*s%s   %c%5u A%c", RSC(POWER_THERMAL_TDC).CODE(),
			width - 18 - RSZ(POWER_THERMAL_TDC), hSpace,
			RSC(POWER_LABEL_TDC).CODE(),
			bix ? '<' : '[',
			RO(Shm)->Proc.Power.TDC,
			bix ? '>' : ']' ),
		TDC_Update );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		"%s%.*s%s   [%7s]", RSC(POWER_THERMAL_TDC).CODE(),
		width - 18 - RSZ(POWER_THERMAL_TDC), hSpace,
		RSC(POWER_LABEL_TDC).CODE(), POWERED(0) );
    }
/* Section Mark */
	struct {
		const ASCII *code;
		const int size;
	} thmPt[THM_POINTS_DIM] = {
	[THM_THRESHOLD_1] = {
		RSC(THERMAL_POINT_THRESHOLD_1).CODE(),
		RSZ(THERMAL_POINT_THRESHOLD_1)
		},
	[THM_THRESHOLD_2] = {
		RSC(THERMAL_POINT_THRESHOLD_2).CODE(),
		RSZ(THERMAL_POINT_THRESHOLD_2)
		},
	[THM_TRIP_LIMIT] = {
		RSC(THERMAL_POINT_TRIP_LIMIT).CODE(),
		RSZ(THERMAL_POINT_TRIP_LIMIT)
		},
	[THM_HTC_LIMIT] = {
		RSC(THERMAL_POINT_HTC_LIMIT).CODE(),
		RSZ(THERMAL_POINT_HTC_LIMIT)
		},
	[THM_HTC_HYST] = {
		RSC(THERMAL_POINT_HTC_HYST).CODE(),
		RSZ(THERMAL_POINT_HTC_HYST)
		},
	};

	PUT(	SCANKEY_NULL, attrib[0], width, 2, "%s %s",
		RSC(POWER_LABEL_CORE).CODE(), RSC(POWER_THERMAL_POINT).CODE() );

	enum THM_POINTS tp;
  for (tp = THM_THRESHOLD_1; tp < THM_POINTS_DIM; tp++)
  {
   if (BITVAL(RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].ThermalPoint.Mask, tp))
   {
	ASCII *code;
	int size;
    if (BITVAL(RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].ThermalPoint.Kind, tp))
    {
		code = RSC(THERMAL_POINT_LIMIT).CODE();
		size = RSZ(THERMAL_POINT_LIMIT);
    } else {
		code = RSC(THERMAL_POINT_THRESHOLD).CODE();
		size = RSZ(THERMAL_POINT_THRESHOLD);
    }
    if (BITVAL(RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].ThermalPoint.State, tp))
    {
	PUT(	SCANKEY_NULL, attrib[5], width, 3,
		"%s%.*s%s   [%5u %c]",
		thmPt[tp].code,
		width - (OutFunc == NULL ? 18 : 16) - thmPt[tp].size - size,
		hSpace, code,
		Setting.fahrCels ? Cels2Fahr(RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Core
					].ThermalPoint.Value[tp])
				: RO(Shm)->Cpu[
					RO(Shm)->Proc.Service.Core
				].ThermalPoint.Value[tp],
		Setting.fahrCels ? 'F' : 'C');
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%7s]",
		thmPt[tp].code,
		width - (OutFunc == NULL ? 18 : 16) - thmPt[tp].size - size,
		hSpace, code, POWERED(0) );
    }
   }
  }
/* Section Mark */
	PUT(	SCANKEY_NULL, attrib[0], width, 2, "%s %s",
		RSC(POWER_LABEL_PKG).CODE(), RSC(POWER_THERMAL_POINT).CODE() );

    for (tp = THM_THRESHOLD_1; tp < THM_POINTS_DIM; tp++)
    {
     if (BITVAL(RO(Shm)->Proc.ThermalPoint.Mask, tp))
     {
	ASCII *code;
	int size;
	if (BITVAL(RO(Shm)->Proc.ThermalPoint.Kind, tp)) {
		code = RSC(THERMAL_POINT_LIMIT).CODE();
		size = RSZ(THERMAL_POINT_LIMIT);
	} else {
		code = RSC(THERMAL_POINT_THRESHOLD).CODE();
		size = RSZ(THERMAL_POINT_THRESHOLD);
	}
      if (BITVAL(RO(Shm)->Proc.ThermalPoint.State, tp))
      {
	PUT(	SCANKEY_NULL, attrib[5], width, 3,
		"%s%.*s%s   [%5u %c]",
		thmPt[tp].code,
		width - (OutFunc == NULL ? 18 : 16) - thmPt[tp].size - size,
		hSpace, code,
		Setting.fahrCels ?
			  Cels2Fahr(RO(Shm)->Proc.ThermalPoint.Value[tp])
			: RO(Shm)->Proc.ThermalPoint.Value[tp],
		Setting.fahrCels ? 'F' : 'C');
      } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%7s]",
		thmPt[tp].code,
		width - (OutFunc == NULL ? 18 : 16) - thmPt[tp].size - size,
		hSpace, code, POWERED(0) );
      }
     }
    }
/* Section Mark */
	PUT(	SCANKEY_NULL, attrib[0], width, 2,
		(char*) RSC(POWER_THERMAL_UNITS).CODE(), NULL );

    if (RO(Shm)->Proc.Power.Unit.Watts > 0.0) {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]",
		RSC(POWER_THERMAL_POWER).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		 - RSZ(POWER_THERMAL_POWER) - RSZ(POWER_THERMAL_WATT), hSpace,
		RSC(POWER_THERMAL_WATT).CODE(),
		RO(Shm)->Proc.Power.Unit.Watts );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13s]",
		RSC(POWER_THERMAL_POWER).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		 - RSZ(POWER_THERMAL_POWER) - RSZ(POWER_THERMAL_WATT), hSpace,
		RSC(POWER_THERMAL_WATT).CODE(), POWERED(0) );
    }
/* Row Mark */
    if (RO(Shm)->Proc.Power.Unit.Joules > 0.0) {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]", RSC(POWER_THERMAL_ENERGY).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_ENERGY) - RSZ(POWER_THERMAL_JOULE), hSpace,
		RSC(POWER_THERMAL_JOULE).CODE(),
		RO(Shm)->Proc.Power.Unit.Joules );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13s]", RSC(POWER_THERMAL_ENERGY).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_ENERGY) - RSZ(POWER_THERMAL_JOULE), hSpace,
		RSC(POWER_THERMAL_JOULE).CODE(), POWERED(0) );
    }
/* Row Mark */
    if (RO(Shm)->Proc.Power.Unit.Times > 0.0) {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13.9f]", RSC(POWER_THERMAL_WINDOW).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_WINDOW) - RSZ(POWER_THERMAL_SECOND), hSpace,
		RSC(POWER_THERMAL_SECOND).CODE(),
		RO(Shm)->Proc.Power.Unit.Times );
    } else {
	PUT(	SCANKEY_NULL, attrib[0], width, 3,
		"%s%.*s%s   [%13s]", RSC(POWER_THERMAL_WINDOW).CODE(),
		width - (OutFunc == NULL ? 24 : 22)
		- RSZ(POWER_THERMAL_WINDOW) - RSZ(POWER_THERMAL_SECOND), hSpace,
		RSC(POWER_THERMAL_SECOND).CODE(), POWERED(0) );
    }
	return reason;
}

void Kernel_RAM_Update(TGrid *grid, DATA_TYPE data[])
{
	char item[20+3+1];
	size_t len;

	StrLenFormat(len, item, 20+3+1, "%18lu KB", (*data[0].pulong));

	memcpy(&grid->cell.item[grid->cell.length - len - 1], item, len);
}

void Kernel_ClockSource_Update(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	char item[CPUFREQ_NAME_LEN+1];
	size_t fmtLen;
	const signed int len = KMIN(strlen(RO(Shm)->CS.array),CPUFREQ_NAME_LEN);

	if (len > 0) {
		StrLenFormat(	fmtLen, item, CPUFREQ_NAME_LEN+1, "%.*s""%-.*s",
				CPUFREQ_NAME_LEN - len, hSpace,
				len, RO(Shm)->CS.array );
	} else {
		StrLenFormat(	fmtLen, item, CPUFREQ_NAME_LEN+1, "%.*s""%-.*s",
				CPUFREQ_NAME_LEN - RSZ(MISSING), hSpace,
				CPUFREQ_NAME_LEN, RSC(MISSING).CODE() );
	}
	memcpy(&grid->cell.item[grid->cell.length - fmtLen - 2], item, fmtLen);
}

void Kernel_CPU_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	char item[CPUFREQ_NAME_LEN+1];
	size_t fmtLen;
	const signed int len = \
	KMIN(strlen(RO(Shm)->SysGate.OS.FreqDriver.Name), CPUFREQ_NAME_LEN);

	if (len > 0) {
		StrLenFormat(	fmtLen, item, CPUFREQ_NAME_LEN+1, "%.*s""%-.*s",
				CPUFREQ_NAME_LEN - len, hSpace,
				len, RO(Shm)->SysGate.OS.FreqDriver.Name );
	} else {
		StrLenFormat(	fmtLen, item, CPUFREQ_NAME_LEN+1, "%.*s""%-.*s",
				CPUFREQ_NAME_LEN - RSZ(MISSING), hSpace,
				CPUFREQ_NAME_LEN, RSC(MISSING).CODE() );
	}
	memcpy(&grid->cell.item[grid->cell.length - fmtLen - 2], item, fmtLen);
}

void Kernel_Governor_Update(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	char item[CPUFREQ_NAME_LEN+1];
	size_t fmtLen;
	const signed int len = \
	KMIN(strlen(RO(Shm)->SysGate.OS.FreqDriver.Governor), CPUFREQ_NAME_LEN);

	if (len > 0) {
		StrLenFormat(	fmtLen, item, CPUFREQ_NAME_LEN+1, "%.*s""%-.*s",
				CPUFREQ_NAME_LEN - len, hSpace,
				len, RO(Shm)->SysGate.OS.FreqDriver.Governor );
	} else {
		StrLenFormat(	fmtLen, item, CPUFREQ_NAME_LEN+1, "%.*s""%-.*s",
				CPUFREQ_NAME_LEN - RSZ(MISSING), hSpace,
				CPUFREQ_NAME_LEN, RSC(MISSING).CODE() );
	}
	memcpy(&grid->cell.item[grid->cell.length - fmtLen - 2], item, fmtLen);
}

void Kernel_CPU_Idle_Update(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	char item[CPUIDLE_NAME_LEN+1];
	size_t fmtLen;
	signed int len = \
	KMIN(strlen(RO(Shm)->SysGate.OS.IdleDriver.Name), CPUIDLE_NAME_LEN);

	if (len > 0) {
		StrLenFormat(	fmtLen, item, CPUIDLE_NAME_LEN+1, "%.*s""%-.*s",
				CPUIDLE_NAME_LEN - len, hSpace,
				len, RO(Shm)->SysGate.OS.IdleDriver.Name );
	} else {
		StrLenFormat(	fmtLen, item, CPUIDLE_NAME_LEN+1, "%.*s""%-.*s",
				CPUIDLE_NAME_LEN - RSZ(MISSING), hSpace,
				CPUIDLE_NAME_LEN, RSC(MISSING).CODE() );
	}
	memcpy(&grid->cell.item[grid->cell.length - fmtLen - 2], item, fmtLen);
}

void Kernel_IdleLimit_Update(TGrid *grid, DATA_TYPE data[])
{
	char item[CPUIDLE_NAME_LEN+1];
	size_t len;
	signed int idx = (*data[0].psint) - 1;

	if (RO(Shm)->SysGate.OS.IdleDriver.stateCount > 0)
	{
		StrLenFormat(len, item, CPUIDLE_NAME_LEN + 1,
				COREFREQ_FORMAT_STR(CPUIDLE_NAME_LEN),
				RO(Shm)->SysGate.OS.IdleDriver.State[idx].Name);
	}
	else
	{
		StrLenFormat(len, item, CPUIDLE_NAME_LEN + 1,
				COREFREQ_FORMAT_STR(CPUIDLE_NAME_LEN),
				RSC(NOT_AVAILABLE).CODE());
	}
	memcpy(&grid->cell.item[grid->cell.length - len - 2], item, len);

	grid->cell.item[grid->cell.length - 2] = \
		RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE ?
			'>' : ']';

	grid->cell.item[grid->cell.length - 3 - CPUIDLE_NAME_LEN] = \
		RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE ?
			'<' : '[';

	grid->cell.quick.key = \
		RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE ?
			BOXKEY_LIMIT_IDLE_STATE : SCANKEY_NULL;
}

REASON_CODE SysInfoKernel(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	REASON_INIT(reason);
	char	*item[5], str[CPUFREQ_NAME_LEN+4+1];
	signed int idx, len = (1 + width) * 5;
	for (idx = 0; idx < 5; idx++) {
		if ((item[idx] = malloc((size_t) len)) != NULL) {
			continue;
		} else {
			do {
				free(item[idx]);
			} while (idx-- != 0);

			REASON_SET(reason, RC_MEM_ERR);
			return reason;
		}
	}
/* Section Mark */
	PUT(	SCANKEY_NULL, RSC(SYSINFO_KERNEL).ATTR(), width, 0,
		"%s:", BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1) ?
		RO(Shm)->SysGate.sysname : (char*) RSC(SYSGATE).CODE() );

	PUT(	SCANKEY_NULL, RSC(KERNEL_RELEASE).ATTR(), width, 2,
		"%s%.*s[%s]", RSC(KERNEL_RELEASE).CODE(),
		width - 5 - RSZ(KERNEL_RELEASE)
		- (int) strlen(RO(Shm)->SysGate.release),
		hSpace, RO(Shm)->SysGate.release );

	PUT(	SCANKEY_NULL, RSC(KERNEL_VERSION).ATTR(), width, 2,
		"%s%.*s[%s]", RSC(KERNEL_VERSION).CODE(),
		width - 5 - RSZ(KERNEL_VERSION)
		- (int) strlen(RO(Shm)->SysGate.version),
		hSpace, RO(Shm)->SysGate.version );

	PUT(	SCANKEY_NULL, RSC(KERNEL_MACHINE).ATTR(), width, 2,
		"%s%.*s[%s]", RSC(KERNEL_MACHINE).CODE(),
		width - 5 - RSZ(KERNEL_MACHINE)
		- (int) strlen(RO(Shm)->SysGate.machine),
		hSpace, RO(Shm)->SysGate.machine );
/* Section Mark */
	PUT(	SCANKEY_NULL, RSC(KERNEL_MEMORY).ATTR(), width, 0,
		"%s:%.*s", RSC(KERNEL_MEMORY).CODE(),
		width - RSZ(KERNEL_MEMORY), hSpace );

	StrLenFormat(len, str, CPUFREQ_NAME_LEN+4+1, "%lu",
			RO(Shm)->SysGate.memInfo.totalram);

	PUT(	SCANKEY_NULL, RSC(KERNEL_TOTAL_RAM).ATTR(), width, 2,
		"%s%.*s" "%s KB", RSC(KERNEL_TOTAL_RAM).CODE(),
		width - 6 - RSZ(KERNEL_TOTAL_RAM) - len, hSpace, str );

	StrLenFormat(len, str, CPUFREQ_NAME_LEN+4+1, "%lu",
			RO(Shm)->SysGate.memInfo.sharedram);

	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_SHARED_RAM).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_SHARED_RAM).CODE(),
			width - 6 - RSZ(KERNEL_SHARED_RAM) - len, hSpace, str ),
		Kernel_RAM_Update, &RO(Shm)->SysGate.memInfo.sharedram );

	StrLenFormat(len, str, CPUFREQ_NAME_LEN+4+1,
			"%lu", RO(Shm)->SysGate.memInfo.freeram);

	GridCall( PUT( SCANKEY_NULL, RSC(KERNEL_FREE_RAM).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_FREE_RAM).CODE(),
			width - 6 - RSZ(KERNEL_FREE_RAM) - len, hSpace, str ),
		Kernel_RAM_Update, &RO(Shm)->SysGate.memInfo.freeram );

	StrLenFormat(len, str, CPUFREQ_NAME_LEN+4+1,
			"%lu", RO(Shm)->SysGate.memInfo.bufferram);

	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_BUFFER_RAM).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_BUFFER_RAM).CODE(),
			width - 6 - RSZ(KERNEL_BUFFER_RAM) - len, hSpace, str ),
		Kernel_RAM_Update, &RO(Shm)->SysGate.memInfo.bufferram );

	StrLenFormat(len, str, CPUFREQ_NAME_LEN+4+1,
			"%lu", RO(Shm)->SysGate.memInfo.totalhigh);

	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_TOTAL_HIGH).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_TOTAL_HIGH).CODE(),
			width - 6 - RSZ(KERNEL_TOTAL_HIGH) - len, hSpace, str ),
		Kernel_RAM_Update, &RO(Shm)->SysGate.memInfo.totalhigh );

	StrLenFormat(len, str, CPUFREQ_NAME_LEN+4+1,
			"%lu", RO(Shm)->SysGate.memInfo.freehigh);

	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_FREE_HIGH).ATTR(), width, 2,
			"%s%.*s" "%s KB", RSC(KERNEL_FREE_HIGH).CODE(),
			width - 6 - RSZ(KERNEL_FREE_HIGH) - len, hSpace, str ),
		Kernel_RAM_Update, &RO(Shm)->SysGate.memInfo.freehigh );
/* Section Mark */
	StrFormat(item[0], 2+4+1+6+1+1, "%%s%%.*s<%%%d.*s>", CPUFREQ_NAME_LEN);

	len = KMIN(strlen(RO(Shm)->CS.array), CPUFREQ_NAME_LEN);
    if (len > 0)
    {
	GridCall( PUT(	OPS_CLOCK_SOURCE_SEL, RSC(KERNEL_CLOCK_SOURCE).ATTR(),
			width, 0, item[0], RSC(KERNEL_CLOCK_SOURCE).CODE(),
			width - (OutFunc == NULL ? 2 : 3)
			- RSZ(KERNEL_CLOCK_SOURCE) - CPUFREQ_NAME_LEN, hSpace,
			len, RO(Shm)->CS.array ),
		Kernel_ClockSource_Update );
    } else {
	GridCall( PUT(	OPS_CLOCK_SOURCE_SEL, RSC(KERNEL_CLOCK_SOURCE).ATTR(),
			width, 0, item[0], RSC(KERNEL_CLOCK_SOURCE).CODE(),
			width - (OutFunc == NULL ? 2 : 3)
			- RSZ(KERNEL_CLOCK_SOURCE) - CPUFREQ_NAME_LEN, hSpace,
			CPUFREQ_NAME_LEN, RSC(MISSING).CODE() ),
		Kernel_ClockSource_Update );
    }
/* Section Mark */
	StrFormat(item[0], 2+4+1+6+1+1, "%%s%%.*s[%%%d.*s]", CPUFREQ_NAME_LEN);

    len = KMIN(strlen(RO(Shm)->SysGate.OS.FreqDriver.Name), CPUFREQ_NAME_LEN);
    if (len > 0)
    {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_FREQ_DRIVER).ATTR(), width, 0,
			item[0], RSC(KERNEL_FREQ_DRIVER).CODE(),
			width - (OutFunc == NULL ? 2 : 3)
			- RSZ(KERNEL_FREQ_DRIVER) - CPUFREQ_NAME_LEN, hSpace,
			len, RO(Shm)->SysGate.OS.FreqDriver.Name ),
		Kernel_CPU_Freq_Update );
    } else {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_FREQ_DRIVER).ATTR(), width, 0,
			item[0], RSC(KERNEL_FREQ_DRIVER).CODE(),
			width - (OutFunc == NULL ? 2 : 3)
			- RSZ(KERNEL_FREQ_DRIVER) - CPUFREQ_NAME_LEN, hSpace,
			CPUFREQ_NAME_LEN, RSC(MISSING).CODE() ),
		Kernel_CPU_Freq_Update );
    }
/* Row Mark */
    len=KMIN(strlen(RO(Shm)->SysGate.OS.FreqDriver.Governor), CPUFREQ_NAME_LEN);
    if (len > 0)
    {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_GOVERNOR).ATTR(), width, 0,
			item[0], RSC(KERNEL_GOVERNOR).CODE(),
			width - (OutFunc == NULL ? 2 : 3) - RSZ(KERNEL_GOVERNOR)
			- CPUFREQ_NAME_LEN, hSpace,
			len, RO(Shm)->SysGate.OS.FreqDriver.Governor ),
		Kernel_Governor_Update );
    } else {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_GOVERNOR).ATTR(), width, 0,
			item[0], RSC(KERNEL_GOVERNOR).CODE(),
			width - (OutFunc == NULL ? 2 : 3) - RSZ(KERNEL_GOVERNOR)
			- CPUFREQ_NAME_LEN, hSpace,
			CPUFREQ_NAME_LEN, RSC(MISSING).CODE() ),
		Kernel_Governor_Update );
    }
/* Row Mark */
	StrFormat(item[0], 2+4+1+6+1+1, "%%s%%.*s[%%%d.*s]", CPUIDLE_NAME_LEN);

    len = KMIN(strlen(RO(Shm)->SysGate.OS.IdleDriver.Name), CPUIDLE_NAME_LEN);
    if (len > 0)
    {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_IDLE_DRIVER).ATTR(), width, 0,
			item[0], RSC(KERNEL_IDLE_DRIVER).CODE(),
			width - (OutFunc == NULL ? 2 : 3)
			- RSZ(KERNEL_IDLE_DRIVER) - CPUIDLE_NAME_LEN, hSpace,
			len, RO(Shm)->SysGate.OS.IdleDriver.Name ),
		Kernel_CPU_Idle_Update );
    } else {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_IDLE_DRIVER).ATTR(), width, 0,
			item[0], RSC(KERNEL_IDLE_DRIVER).CODE(),
			width - (OutFunc == NULL ? 2 : 3)
			- RSZ(KERNEL_IDLE_DRIVER) - CPUIDLE_NAME_LEN, hSpace,
			CPUIDLE_NAME_LEN, RSC(MISSING).CODE() ),
		Kernel_CPU_Idle_Update );
    }
/* Section Mark */
	StrFormat(item[0], 2+4+1+4+1+1,
		"%%s%%.*s%c%%%ds%c",
	RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE ? '<' : '[',
		CPUIDLE_NAME_LEN,
	RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE ? '>' : ']');

  if (RO(Shm)->SysGate.OS.IdleDriver.stateCount > 0)
  {
	idx = RO(Shm)->SysGate.OS.IdleDriver.stateLimit - 1;
	GridCall(
		PUT(RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE ?
			BOXKEY_LIMIT_IDLE_STATE : SCANKEY_NULL,
			RSC(KERNEL_LIMIT).ATTR(), width, 2,
			item[0], RSC(KERNEL_LIMIT).CODE(),
			width - RSZ(KERNEL_LIMIT) - CPUIDLE_NAME_LEN-5, hSpace,
			RO(Shm)->SysGate.OS.IdleDriver.State[idx].Name ),
		Kernel_IdleLimit_Update,
		&RO(Shm)->SysGate.OS.IdleDriver.stateLimit );
  } else {
	GridCall( PUT(	SCANKEY_NULL, RSC(KERNEL_LIMIT).ATTR(), width, 2,
			item[0], RSC(KERNEL_LIMIT).CODE(),
			width - RSZ(KERNEL_LIMIT) - CPUIDLE_NAME_LEN-5, hSpace,
			RSC(NOT_AVAILABLE).CODE() ),
		Kernel_IdleLimit_Update,
		&RO(Shm)->SysGate.OS.IdleDriver.stateLimit );
  }
/* Row Mark */
  if (RO(Shm)->SysGate.OS.IdleDriver.stateCount > 0)
  {
	StrFormat(item[0], 10+1, "%s%.*s",
		RSC(KERNEL_STATE).CODE(), 10 - RSZ(KERNEL_STATE), hSpace);

	StrFormat(item[1], 10+1, "%.*s", 10, hSpace);

	StrFormat(item[2], 10+1, "%s%.*s",
		RSC(KERNEL_POWER).CODE(), 10 - RSZ(KERNEL_POWER), hSpace);

	StrFormat(item[3], 10+1, "%s%.*s",
		RSC(KERNEL_LATENCY).CODE(), 10 - RSZ(KERNEL_LATENCY), hSpace);

	StrFormat(item[4], 10+1, "%s%.*s",
		RSC(KERNEL_RESIDENCY).CODE(), 10-RSZ(KERNEL_RESIDENCY), hSpace);

    for (idx = 0; idx < CPUIDLE_STATE_MAX; idx++)
    {
      if (idx < RO(Shm)->SysGate.OS.IdleDriver.stateCount)
      {
	ssize_t cat;
	signed int n;

	len = KMIN(strlen(RO(Shm)->SysGate.OS.IdleDriver.State[idx].Name), 7);
	StrLenFormat(cat, str, 7+1, "%7.*s", (int) len,
			RO(Shm)->SysGate.OS.IdleDriver.State[idx].Name);

	len = strlen(item[0]);
	for (n = 0; n < cat; n++) {
		item[0][len + n] = str[n];
	}
	item[0][len + n] = '\0';

	len = KMIN(strlen(RO(Shm)->SysGate.OS.IdleDriver.State[idx].Desc), 7);
	StrLenFormat(cat, str, 7+1, "%7.*s", (int) len,
			RO(Shm)->SysGate.OS.IdleDriver.State[idx].Desc);

	len = strlen(item[1]);
	for (n = 0; n < cat; n++) {
		item[1][len + n] = str[n];
	}
	item[1][len + n] = '\0';

	StrLenFormat(cat, str, 10+1, "%7d",
			RO(Shm)->SysGate.OS.IdleDriver.State[idx].powerUsage);

	len = strlen(item[2]);
	for (n = 0; n < cat; n++) {
		item[2][len + n] = str[n];
	}
	item[2][len + n] = '\0';

	StrLenFormat(cat, str, 10+1, "%7u",
			RO(Shm)->SysGate.OS.IdleDriver.State[idx].exitLatency);

	len = strlen(item[3]);
	for (n = 0; n < cat; n++) {
		item[3][len + n] = str[n];
	}
	item[3][len + n] = '\0';

	StrLenFormat(cat, str, 10+1, "%7u",
		RO(Shm)->SysGate.OS.IdleDriver.State[idx].targetResidency);

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
	PUT(	SCANKEY_NULL, RSC(KERNEL_STATE).ATTR(), width, 3,
		 "%.*s", width - (OutFunc == NULL ? 6 : 3), item[0] );

	PUT(	SCANKEY_NULL, RSC(KERNEL_STATE).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[1] );

	PUT(	SCANKEY_NULL, RSC(KERNEL_POWER).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[2] );

	PUT(	SCANKEY_NULL, RSC(KERNEL_LATENCY).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[3] );

	PUT(	SCANKEY_NULL, RSC(KERNEL_RESIDENCY).ATTR(), width, 3,
		"%.*s", width - (OutFunc == NULL ? 6 : 3), item[4] );
  }
/* Section Mark */
    for (idx = 0; idx < 5; idx++) {
	free(item[idx]);
    }
	return reason;
}

char *ScrambleSMBIOS(enum SMB_STRING idx, const size_t mod, const char thing)
{
	struct {
		char		*pString;
		unsigned short	secret;
	} smb[SMB_STRING_COUNT] = {
		{.pString = RO(Shm)->SMB.BIOS.Vendor	,	.secret = 0},
		{.pString = RO(Shm)->SMB.BIOS.Version	,	.secret = 0},
		{.pString = RO(Shm)->SMB.BIOS.Release	,	.secret = 0},
		{.pString = RO(Shm)->SMB.System.Vendor	,	.secret = 0},
		{.pString = RO(Shm)->SMB.Product.Name	,	.secret = 0},
		{.pString = RO(Shm)->SMB.Product.Version,	.secret = 0},
		{.pString = RO(Shm)->SMB.Product.Serial,	.secret = 1},
		{.pString = RO(Shm)->SMB.Product.SKU	,	.secret = 0},
		{.pString = RO(Shm)->SMB.Product.Family ,	.secret = 0},
		{.pString = RO(Shm)->SMB.Board.Vendor	,	.secret = 0},
		{.pString = RO(Shm)->SMB.Board.Name	,	.secret = 0},
		{.pString = RO(Shm)->SMB.Board.Version	,	.secret = 0},
		{.pString = RO(Shm)->SMB.Board.Serial	,	.secret = 1},
		{.pString = RO(Shm)->SMB.Phys.Memory.Array ,	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Locator[0],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Locator[1],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Locator[2],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Locator[3],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Manufacturer[0],.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Manufacturer[1],.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Manufacturer[2],.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.Manufacturer[3],.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.PartNumber[0],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.PartNumber[1],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.PartNumber[2],	.secret = 0},
		{.pString = RO(Shm)->SMB.Memory.PartNumber[3],	.secret = 0}
	};
	if (smb[idx].secret && Setting.secret) {
		static char outStr[MAX_UTS_LEN];
		size_t len = strlen(smb[idx].pString), dst;
	    for (dst = 0; dst < len; dst++) {
		outStr[dst] = (dst % mod) ? thing : smb[idx].pString[dst];
	    }
		outStr[dst] = '\0';
		return outStr;
	} else {
		return smb[idx].pString;
	}
}

const char *SMB_Comment[SMB_STRING_COUNT] = {
	" "	COREFREQ_STRINGIFY(SMB_BIOS_VENDOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_BIOS_VERSION)		" ",
	" "	COREFREQ_STRINGIFY(SMB_BIOS_RELEASE)		" ",
	" "	COREFREQ_STRINGIFY(SMB_SYSTEM_VENDOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_NAME)		" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_VERSION) 	" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_SERIAL)		" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_SKU)		" ",
	" "	COREFREQ_STRINGIFY(SMB_PRODUCT_FAMILY)		" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_VENDOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_NAME)		" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_VERSION)		" ",
	" "	COREFREQ_STRINGIFY(SMB_BOARD_SERIAL)		" ",
	" "	COREFREQ_STRINGIFY(SMB_PHYS_MEM_ARRAY)		" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_0_LOCATOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_1_LOCATOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_2_LOCATOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_3_LOCATOR)		" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_0_MANUFACTURER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_1_MANUFACTURER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_2_MANUFACTURER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_3_MANUFACTURER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_0_PARTNUMBER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_1_PARTNUMBER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_2_PARTNUMBER)	" ",
	" "	COREFREQ_STRINGIFY(SMB_MEM_3_PARTNUMBER)	" "
};

REASON_CODE SysInfoSMBIOS(Window *win, CUINT width, CELL_FUNC OutFunc)
{
	enum SMB_STRING idx;

	REASON_INIT(reason);

    for (idx = 0; idx < SMB_STRING_COUNT; idx ++)
    {
	const unsigned long long key	= SMBIOS_STRING_INDEX
					| (unsigned long long) idx;

	GridHover( PUT( key, RSC(SMBIOS_ITEM).ATTR(), width, 0,
			"[%2d] %s", idx, ScrambleSMBIOS(idx, 4, '-')),
		SMB_Comment[idx] );
    }
	return reason;
}

void Package(unsigned int iter)
{
	char *out = malloc(8 + (RO(Shm)->Proc.CPU.Count + 10) * MIN_WIDTH);
  if (out != NULL)
  {
	int idx, rdx;
	const int sdx = sprintf(out, "\t\t" "Cycles" "\t\t" "State(%%)" "\n");

    while (!BITVAL(Shutdown, SYNC) && (iter-- > 0) && (sdx > 0))
    {
	while (!BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
	}
	struct PKG_FLIP_FLOP *PFlop = \
				&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	idx = sdx;
	if ((rdx = sprintf(&out[idx],
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
		PFlop->Delta.PC02, 100.f * RO(Shm)->Proc.State.PC02,
		PFlop->Delta.PC03, 100.f * RO(Shm)->Proc.State.PC03,
		PFlop->Delta.PC04, 100.f * RO(Shm)->Proc.State.PC04,
		PFlop->Delta.PC06, 100.f * RO(Shm)->Proc.State.PC06,
		PFlop->Delta.PC07, 100.f * RO(Shm)->Proc.State.PC07,
		PFlop->Delta.PC08, 100.f * RO(Shm)->Proc.State.PC08,
		PFlop->Delta.PC09, 100.f * RO(Shm)->Proc.State.PC09,
		PFlop->Delta.PC10, 100.f * RO(Shm)->Proc.State.PC10,
		PFlop->Delta.MC6,  100.f * RO(Shm)->Proc.State.MC6,
		PFlop->Delta.PCLK,
		PFlop->Uncore.FC0)) > 0)
	{
		idx += rdx;
	}
	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

signed int Core_Celsius(char *out, struct FLIP_FLOP *CFlop, unsigned int cpu)
{
	return sprintf(out,
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
		RO(Shm)->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST],
		CFlop->Thermal.Temp,
		CFlop->Thermal.Sensor,
		RO(Shm)->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST]);
}

signed int Core_Fahrenheit(char *out,struct FLIP_FLOP *CFlop,unsigned int cpu)
{
	return sprintf(out,
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
		Cels2Fahr(RO(Shm)->Cpu[cpu].PowerThermal.Limit[SENSOR_LOWEST]),
		Cels2Fahr(CFlop->Thermal.Temp),
		Cels2Fahr(CFlop->Thermal.Sensor),
		Cels2Fahr(RO(Shm)->Cpu[cpu].PowerThermal.Limit[SENSOR_HIGHEST])
		);
}

signed int Pkg_Celsius(char *out, struct PKG_FLIP_FLOP *PFlop)
{
	struct FLIP_FLOP *SFlop = &RO(Shm)->Cpu[
		RO(Shm)->Proc.Service.Core
	].FlipFlop[
		!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];

	return sprintf(out, "\n"					\
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
		100.f * RO(Shm)->Proc.Avg.Turbo,
		100.f * RO(Shm)->Proc.Avg.C0,
		100.f * RO(Shm)->Proc.Avg.C1,
		100.f * RO(Shm)->Proc.Avg.C3,
		100.f * RO(Shm)->Proc.Avg.C6,
		100.f * RO(Shm)->Proc.Avg.C7,
		5, hSpace,
		SFlop->Thermal.Param.Offset[0],
		3, hSpace,
		PFlop->Thermal.Temp);
}

signed int Pkg_Fahrenheit(char *out, struct PKG_FLIP_FLOP *PFlop)
{
	struct FLIP_FLOP *SFlop = &RO(Shm)->Cpu[
		RO(Shm)->Proc.Service.Core
	].FlipFlop[
		!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];

	return sprintf(out, "\n"					\
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
		100.f * RO(Shm)->Proc.Avg.Turbo,
		100.f * RO(Shm)->Proc.Avg.C0,
		100.f * RO(Shm)->Proc.Avg.C1,
		100.f * RO(Shm)->Proc.Avg.C3,
		100.f * RO(Shm)->Proc.Avg.C6,
		100.f * RO(Shm)->Proc.Avg.C7,
		5, hSpace,
		SFlop->Thermal.Param.Offset[0],
		3, hSpace,
		Cels2Fahr(PFlop->Thermal.Temp));
}

void Counters(unsigned int iter)
{
	signed int (*Core_Temp)(char *, struct FLIP_FLOP *, unsigned int) = \
		Setting.fahrCels ? Core_Fahrenheit : Core_Celsius;

	signed int (*Pkg_Temp)(char *, struct PKG_FLIP_FLOP *) = \
		Setting.fahrCels ? Pkg_Fahrenheit : Pkg_Celsius;

	char *out = malloc(8 + (RO(Shm)->Proc.CPU.Count + 3) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int cpu;
	signed int idx, rdx;
	const int sdx = \
	sprintf( out,	"CPU Freq(MHz) Ratio  Turbo"			\
			"  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
			"  Min TMP:TS  Max\n" );

    while (!BITVAL(Shutdown, SYNC) && (iter-- > 0) && (sdx > 0))
    {
	while (!BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu=0;(cpu < RO(Shm)->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);cpu++)
	{
	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, HW)) {
		struct FLIP_FLOP *CFlop = \
			&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

		if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)) {
			rdx = Core_Temp(&out[idx], CFlop, cpu);
		} else {
			rdx = sprintf(&out[idx], "%03u        OFF\n", cpu);
		}
		if (rdx > 0) {
			idx += rdx;
		}
	    }
	}
	struct PKG_FLIP_FLOP *PFlop = \
				&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	if ((rdx = Pkg_Temp(&out[idx], PFlop)) > 0) {
		idx += rdx;
	}
	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

void Sensors(unsigned int iter)
{
	char *out = malloc(8 + (RO(Shm)->Proc.CPU.Count + 4) * MIN_WIDTH);
	char *row = malloc(MIN_WIDTH + 16);
  if (out && row)
  {
	enum PWR_DOMAIN pw;
	unsigned int cpu;
	signed int idx, rdx;
	const int sdx = \
	sprintf( out,	"CPU Freq(MHz) VID  Vcore  TMP(%c)"		\
			"    Accumulator       Energy(J)     Power(W)\n",
			Setting.fahrCels ? 'F' : 'C'  );

	const int ldx = \
	sprintf( row,	"\n" "%.*sPackage[%c]%.*sCores%.*sUncore%.*sMemory" \
			"%.*sPlatform" "\n" "Energy(J):",
			13, hSpace,
		'0'+RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.PackageID,
			4, hSpace, 9, hSpace, 8, hSpace, 8, hSpace);

    while (!BITVAL(Shutdown, SYNC) && (iter-- > 0) && (sdx > 0) && (ldx > 0))
    {
	while (!BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu = 0;
		(cpu < RO(Shm)->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);
			cpu++)
	{
	  if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, HW))
	  {
		struct FLIP_FLOP *CFlop = \
			&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)) {
		rdx = sprintf(&out[idx],"%03u %7.2f %5d  %5.4f  %3u"	\
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
		rdx = sprintf(&out[idx], "%03u        OFF\n", cpu);
	    }
	    if (rdx > 0) {
		idx += rdx;
	    }
	  }
	}
	memcpy(&out[idx], row, (size_t) ldx);
	idx += ldx;

	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++) {
		rdx = sprintf(&out[idx], "%.*s" "%13.9f", 1, hSpace,
				RO(Shm)->Proc.State.Energy[pw].Current);
		if (rdx > 0) {
			idx += rdx;
		}
	}
	memcpy(&out[idx], "\n" "Power(W) :", 11);
	idx += 11;
	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++) {
		rdx = sprintf(&out[idx], "%.*s" "%13.9f", 1, hSpace,
				RO(Shm)->Proc.State.Power[pw].Current);
		if (rdx > 0) {
			idx += rdx;
		}
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

void Voltage(unsigned int iter)
{
	char *out = malloc(8 + (RO(Shm)->Proc.CPU.Count + 1) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int cpu;
	signed int idx, rdx;
	const int sdx=sprintf(out, "CPU Freq(MHz) VID  Min     Vcore   Max\n");

    while (!BITVAL(Shutdown, SYNC) && (iter-- > 0) && (sdx > 0))
    {
	while (!BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu = 0;
		(cpu < RO(Shm)->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);
			cpu++)
	{
	  if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, HW))
	  {
		struct FLIP_FLOP *CFlop = \
			&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)) {
		rdx = sprintf(&out[idx],
			"%03u %7.2f %5d  %5.4f  %5.4f  %5.4f\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Voltage.VID,
			RO(Shm)->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_LOWEST],
			CFlop->Voltage.Vcore,
			RO(Shm)->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_HIGHEST]
			);
	    } else {
		rdx = sprintf(&out[idx], "%03u        OFF\n", cpu);
	    }
	    if (rdx > 0) {
		idx += rdx;
	    }
	  }
	}
	out[idx++] = '\n';

	fwrite(out, (size_t) idx, 1, stdout);
    }
	free(out);
  }
}

void Power(unsigned int iter)
{
	char *out = malloc(8 + (RO(Shm)->Proc.CPU.Count + 5) * MIN_WIDTH);
	char *row = malloc(MIN_WIDTH + 8);
  if (out && row)
  {
	enum PWR_DOMAIN pw;
	unsigned int cpu;
	signed int idx, rdx;
	const int sdx = \
	sprintf( out,	"CPU Freq(MHz)" 				\
			"    Accumulator      Min  Energy(J) Max"	\
			"    Min  Power(W)  Max\n" );
	const int ldx = \
	sprintf( row, "\nEnergy(J)  Package[%c]%.*sCores%.*sUncore%.*sMemory\n",
		'0'+RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.PackageID,
			9, hSpace, 15, hSpace, 14, hSpace );

    while (!BITVAL(Shutdown, SYNC) && (iter-- > 0) && (sdx > 0) && (ldx > 0))
    {
	while (!BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu = 0;
		(cpu < RO(Shm)->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);
			cpu++)
	{
	  if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, HW))
	  {
		struct FLIP_FLOP *CFlop = \
			&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)) {
		rdx = sprintf(&out[idx],
			"%03u %7.2f"					\
			"  %018llu  %6.2f %6.2f %6.2f  %6.2f %6.2f %6.2f\n",
			cpu,
			CFlop->Relative.Freq,
			CFlop->Delta.Power.ACCU,
			RO(Shm)->Cpu[cpu].Sensors.Energy.Limit[SENSOR_LOWEST],
			CFlop->State.Energy,
			RO(Shm)->Cpu[cpu].Sensors.Energy.Limit[SENSOR_HIGHEST],
			RO(Shm)->Cpu[cpu].Sensors.Power.Limit[SENSOR_LOWEST],
			CFlop->State.Power,
			RO(Shm)->Cpu[cpu].Sensors.Power.Limit[SENSOR_HIGHEST]);
	    } else {
		rdx = sprintf(&out[idx], "%03u        OFF\n", cpu);
	    }
	    if (rdx > 0) {
		idx += rdx;
	    }
	  }
	}
	memcpy(&out[idx], row, (size_t) ldx);
	idx += ldx;

	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(PLATFORM); pw++) {
		rdx = sprintf(&out[idx], "%.*s" "%6.2f%6.2f%6.2f",
			pw == PWR_DOMAIN(PKG) ? 1 : 2, hSpace,
			RO(Shm)->Proc.State.Energy[pw].Limit[SENSOR_LOWEST],
			RO(Shm)->Proc.State.Energy[pw].Current,
			RO(Shm)->Proc.State.Energy[pw].Limit[SENSOR_HIGHEST]);
		if (rdx > 0) {
			idx += rdx;
		}
	}
	memcpy(&out[idx], "\n" "Power(W)\n", 10);
	idx += 10;

	for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(PLATFORM); pw++) {
		rdx = sprintf(&out[idx], "%.*s" "%6.2f%6.2f%6.2f",
			pw == PWR_DOMAIN(PKG) ? 1 : 2, hSpace,
			RO(Shm)->Proc.State.Power[pw].Limit[SENSOR_LOWEST],
			RO(Shm)->Proc.State.Power[pw].Current,
			RO(Shm)->Proc.State.Power[pw].Limit[SENSOR_HIGHEST]);
		if (rdx > 0) {
			idx += rdx;
		}
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

void Instructions(unsigned int iter)
{
	char *out = malloc(8 + (RO(Shm)->Proc.CPU.Count + 1) * MIN_WIDTH);
  if (out != NULL)
  {
	unsigned int cpu;
	signed int idx, rdx;
	const int sdx = \
		sprintf(out, "CPU     IPS            IPC            CPI\n");

    while (!BITVAL(Shutdown, SYNC) && (iter-- > 0) && (sdx > 0))
    {
	while (!BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)
	    && !BITVAL(Shutdown, SYNC)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
	}
	idx = sdx;
	for (cpu = 0;
		(cpu < RO(Shm)->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC);
			cpu++)
	{
	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, HW)) {
		struct FLIP_FLOP *CFlop = \
			&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

		if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS)) {
			rdx = sprintf(&out[idx],
					"%03u %12.6f/s %12.6f/c %12.6f/i\n",
					cpu,
					CFlop->State.IPS,
					CFlop->State.IPC,
					CFlop->State.CPI);
		} else {
			rdx = sprintf(&out[idx], "%03u\n", cpu);
		}
		if (rdx > 0) {
			idx += rdx;
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

ASCII* Topology_Std(char *pStr, unsigned int cpu)
{
    if (RO(Shm)->Cpu[cpu].Topology.MP.BSP) {
	StrFormat(&pStr[ 0], 4+(2*11)+1, "%03u:BSP%5d\x20",
			cpu,
			RO(Shm)->Cpu[cpu].Topology.ApicID);
	return RSC(TOPOLOGY_BSP_COMM).CODE();
    } else {
	StrFormat(&pStr[ 0], 1+(3*11)+1, "%03u:%3d%5d\x20",
			cpu,
			RO(Shm)->Cpu[cpu].Topology.PackageID,
			RO(Shm)->Cpu[cpu].Topology.ApicID);
	return NULL;
    }
}

ASCII* Topology_SMT(char *pStr, unsigned int cpu)
{
	ASCII *comment = Topology_Std(pStr, cpu);

	StrFormat(&pStr[TOPO_MATX+1], 1+(2*11)+1, "%5d\x20\x20%5d\x20",
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID);
	return comment;
}

ASCII* Topology_CMP(char *pStr, unsigned int cpu)
{
	ASCII *comment = Topology_Std(pStr, cpu);

	StrFormat(&pStr[TOPO_MATX+1], (3*11)+1, "%3u%4d%6d",
			RO(Shm)->Cpu[cpu].Topology.Cluster.CMP,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID);
	return comment;
}

ASCII* Topology_CCD(char *pStr, unsigned int cpu)
{
	ASCII *comment = Topology_Std(pStr, cpu);

	StrFormat(&pStr[TOPO_MATX+1], (4*11)+1, "%3u%3u%4d%3d",
			RO(Shm)->Cpu[cpu].Topology.Cluster.CCD,
			RO(Shm)->Cpu[cpu].Topology.Cluster.CCX,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID);
	return comment;
}

ASCII* Topology_Hybrid(char *pStr, unsigned int cpu)
{
	ASCII *comment = Topology_Std(pStr, cpu);

	StrFormat(&pStr[TOPO_MATX+1], 3+(3*11)+1, "\x20%c%4u%4d%3d",
			RO(Shm)->Cpu[cpu].Topology.MP.Ecore ?
			'E' : RO(Shm)->Cpu[cpu].Topology.MP.Pcore ?
			'P' : '?',
			RO(Shm)->Cpu[cpu].Topology.Cluster.Hybrid_ID,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID);
	return comment;
}

const char *TopologyStrOFF[] = {
	"\x20\x20\x20\x20-\x20\x20\x20\x20\x20\x20-\x20",
	"\x20\x20-\x20\x20\x20-\x20\x20\x20\x20\x20-",
	"\x20\x20-\x20\x20-\x20\x20\x20-\x20\x20-",
	"\x20-\x20\x20\x20-\x20\x20\x20-\x20\x20-"
};

const char *TopologyFmtOFF[] = {
	"%03u:\x20\x20-\x20\x20\x20\x20-\x20",
	"\x20\x20\x20\x20\x20\x20\x20-\x20\x20-\x20\x20"
};

void Topology(Window *win, CELL_FUNC OutFunc)
{
	ASCII *TopologyHeader[TOPO_MATY] = {
		RSC(TOPOLOGY_HDR_PKG).CODE(),
		RSC(TOPOLOGY_HDR_SMT).CODE(),
		RSC(TOPOLOGY_HDR_CACHE).CODE(),
		RSC(TOPOLOGY_HDR_WRBAK).CODE(),
		RSC(TOPOLOGY_HDR_INCL).CODE(),
		RSC(TOPOLOGY_HDR_EMPTY).CODE()
	};
	ASCII *TopologySubHeader[TOPO_MATY] = {
		RSC(TOPOLOGY_SUB_ITEM1).CODE(),
		NULL,
		RSC(TOPOLOGY_SUB_ITEM3).CODE(),
		RSC(TOPOLOGY_SUB_ITEM4).CODE(),
		RSC(TOPOLOGY_SUB_ITEM5).CODE(),
		RSC(TOPOLOGY_SUB_ITEM6).CODE()
	};
	ASCII *TopologyAltSubHeader[] = {
		RSC(TOPOLOGY_ALT_ITEM1).CODE(),
		RSC(TOPOLOGY_ALT_ITEM2).CODE(),
		RSC(TOPOLOGY_ALT_ITEM3).CODE(),
		RSC(TOPOLOGY_ALT_ITEM4).CODE()
	};
	ATTRIBUTE *attrib[5] = {
		RSC(TOPOLOGY_COND0).ATTR(),
		RSC(TOPOLOGY_COND1).ATTR(),
		RSC(TOPOLOGY_COND2).ATTR(),
		RSC(TOPOLOGY_COND3).ATTR(),
		RSC(TOPOLOGY_COND4).ATTR()
	};
	char *strID = malloc(2 * ((4*11) + 1));
	char const *pStrOFF = TopologyStrOFF[0];
	unsigned int cpu = 0, level = 0;
	CUINT cells_per_line = win->matrix.size.wth, *nl = &cells_per_line;

  if (strID != NULL)
  {
	ASCII* (*TopologyFunc)(char*, unsigned int) = Topology_SMT;
/* Row Mark */
	PRT(MAP, attrib[2], TopologyHeader[0]);
	PRT(MAP, attrib[2], TopologyHeader[1]);
	PRT(MAP, attrib[2], TopologyHeader[2]);
	PRT(MAP, attrib[2], TopologyHeader[3]);
	PRT(MAP, attrib[2], TopologyHeader[4]);
	PRT(MAP, attrib[2], TopologyHeader[5]);
/* Row Mark */
	PRT(MAP, attrib[2], TopologySubHeader[0]);
    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
     || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	switch (RO(Shm)->Proc.ArchID) {
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
	case Hygon_Family_18h:
	case AMD_Family_19h:
	case AMD_Zen:
	case AMD_Zen_APU:
	case AMD_ZenPlus:
	case AMD_ZenPlus_APU:
	case AMD_Zen_Dali:
	case AMD_EPYC_Rome_CPK:
	case AMD_Zen2_Renoir:
	case AMD_Zen2_LCN:
	case AMD_Zen2_MTS:
	case AMD_Zen2_Ariel:
	case AMD_Zen3_VMR:
	case AMD_Zen3_CZN:
	case AMD_EPYC_Milan:
	case AMD_Zen3_Chagall:
	case AMD_Zen3_Badami:
	case AMD_Zen3Plus_RMB:
	case AMD_Zen4_RPL:
		TopologyFunc = Topology_CCD;
		pStrOFF = TopologyStrOFF[2];
		TopologySubHeader[1] = TopologyAltSubHeader[2];
		break;
	default:
		if ( RO(Shm)->Proc.Features.Std.ECX.Hyperv )
		{		/*	Is capable of Virtualization	*/
			goto TOPOLOGY_CMP;
		}  else  {	/* AMD_Family_0Fh and AMD_Family_10h	*/
			TopologySubHeader[1] = TopologyAltSubHeader[0];
		}
		break;
	}
    } else {
	if (RO(Shm)->Proc.Features.ExtFeature.EDX.Hybrid) {
		TopologyFunc = Topology_Hybrid;
		pStrOFF = TopologyStrOFF[3];
		TopologySubHeader[1] = TopologyAltSubHeader[3];
	} else {
		TopologySubHeader[1] = TopologyAltSubHeader[0];
	}
    }
	PRT(MAP, attrib[2], TopologySubHeader[1]);
	PRT(MAP, attrib[2], TopologySubHeader[2]);
	PRT(MAP, attrib[2], TopologySubHeader[3]);
	PRT(MAP, attrib[2], TopologySubHeader[4]);
	PRT(MAP, attrib[2], TopologySubHeader[5]);
/* Row Mark */
    for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++)
    {
      if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
      {
	ASCII *comment = TopologyFunc(strID, cpu);

	GridHover(PRT(MAP, attrib[3], &strID[ 0]), (char*) comment);
	PRT(MAP, attrib[0], &strID[14]);

       for (level = 0; level < CACHE_MAX_LEVEL; level++)
       {
	PRT(MAP, attrib[0], "%8u%3u%c%c",
		RO(Shm)->Cpu[cpu].Topology.Cache[level].Size,
		RO(Shm)->Cpu[cpu].Topology.Cache[level].Way,
		RO(Shm)->Cpu[cpu].Topology.Cache[level].Feature.WriteBack ?
			'w' : 0x20,
		RO(Shm)->Cpu[cpu].Topology.Cache[level].Feature.Inclusive ?
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

#define TIMING(_mc, _cha)	RO(Shm)->Uncore.MC[_mc].Channel[_cha].Timing

const char *MEM_CTRL_FMT = "%*.s";

void iSplit(unsigned int sInt, char hInt[]) {
	char fInt[11+1];
	StrFormat(fInt, 11+1, "%10u", sInt);
	memcpy((hInt + 0), (fInt + 0), 5); *(hInt + 0 + 5) = '\0';
	memcpy((hInt + 8), (fInt + 5), 5); *(hInt + 8 + 5) = '\0';
}

typedef void (*TIMING_FUNC)(Window*, CELL_FUNC, CUINT*, unsigned short);

void Timing_DDR3(Window *win, CELL_FUNC OutFunc, CUINT *nl, unsigned short mc)
{
	const ASCII *Header_DDR3[2][MC_MATX] = {
		{
			RSC(MEM_CTRL_CHANNEL).CODE(),
			RSC(DDR3_CL).CODE(),
			RSC(DDR3_RCD).CODE(),
			RSC(DDR3_RP).CODE(),
			RSC(DDR3_RAS).CODE(),
			RSC(DDR3_RRD).CODE(),
			RSC(DDR3_RFC).CODE(),
			RSC(DDR3_WR).CODE(),
			RSC(DDR3_RTP).CODE(),
			RSC(DDR3_WTP).CODE(),
			RSC(DDR3_FAW).CODE(),
			RSC(DDR3_B2B).CODE(),
			RSC(DDR3_CWL).CODE(),
			RSC(DDR3_CMD).CODE(),
			RSC(DDR3_REFI).CODE()
		},
		{
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR3_DDWRTRD).CODE(),
			RSC(DDR3_DRWRTRD).CODE(),
			RSC(DDR3_SRWRTRD).CODE(),
			RSC(DDR3_DDRDTWR).CODE(),
			RSC(DDR3_DRRDTWR).CODE(),
			RSC(DDR3_SRRDTWR).CODE(),
			RSC(DDR3_DDRDTRD).CODE(),
			RSC(DDR3_DRRDTRD).CODE(),
			RSC(DDR3_SRRDTRD).CODE(),
			RSC(DDR3_DDWRTWR).CODE(),
			RSC(DDR3_DRWRTWR).CODE(),
			RSC(DDR3_SRWRTWR).CODE(),
			RSC(DDR3_CKE).CODE(),
			RSC(DDR3_ECC).CODE()
		}
	};
	const ASCII *Footer_DDR3[2][MC_MATX] = {
		{
			NULL,
			RSC(DDR3_CL_COMM).CODE(),
			RSC(DDR3_RCD_COMM).CODE(),
			RSC(DDR3_RP_COMM).CODE(),
			RSC(DDR3_RAS_COMM).CODE(),
			RSC(DDR3_RRD_COMM).CODE(),
			RSC(DDR3_RFC_COMM).CODE(),
			RSC(DDR3_WR_COMM).CODE(),
			RSC(DDR3_RTP_COMM).CODE(),
			RSC(DDR3_WTP_COMM).CODE(),
			RSC(DDR3_FAW_COMM).CODE(),
			RSC(DDR3_B2B_COMM).CODE(),
			RSC(DDR3_CWL_COMM).CODE(),
			RSC(DDR3_CMD_COMM).CODE(),
			RSC(DDR3_REFI_COMM).CODE()
		},
		{
			NULL,
			RSC(DDR3_DDWRTRD_COMM).CODE(),
			RSC(DDR3_DRWRTRD_COMM).CODE(),
			RSC(DDR3_SRWRTRD_COMM).CODE(),
			RSC(DDR3_DDRDTWR_COMM).CODE(),
			RSC(DDR3_DRRDTWR_COMM).CODE(),
			RSC(DDR3_SRRDTWR_COMM).CODE(),
			RSC(DDR3_DDRDTRD_COMM).CODE(),
			RSC(DDR3_DRRDTRD_COMM).CODE(),
			RSC(DDR3_SRRDTRD_COMM).CODE(),
			RSC(DDR3_DDWRTWR_COMM).CODE(),
			RSC(DDR3_DRWRTWR_COMM).CODE(),
			RSC(DDR3_SRWRTWR_COMM).CODE(),
			RSC(DDR3_CKE_COMM).CODE(),
			RSC(DDR3_ECC_COMM).CODE()
		}
	};
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	CUINT	nc;
	unsigned short cha;

	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC, attrib[0], Header_DDR3[0][nc]),
				(char*) Footer_DDR3[0][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tCL);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRCD);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRP);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRAS);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRRD);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRFC);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWR);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRTPr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWTPr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tFAW);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).B2B);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tCWL);
		PRT(IMC, attrib[1], "%3uT\x20", TIMING(mc, cha).CMD_Rate);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tREFI);
	}
	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC, attrib[0], Header_DDR3[1][nc]),
				(char*) Footer_DDR3[1][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tddWrTRd);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tdrWrTRd);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tsrWrTRd);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tddRdTWr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tdrRdTWr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tsrRdTWr);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tddRdTRd);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tdrRdTRd);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tsrRdTRd);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tddWrTWr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tdrWrTWr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tsrWrTWr);

		PRT(IMC, attrib[1], "%4u\x20", TIMING(mc, cha).tCKE);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).ECC);
	}
}

void Timing_DDR4(Window *win, CELL_FUNC OutFunc, CUINT *nl, unsigned short mc)
{
	const ASCII *Header_DDR4[3][MC_MATX] = {
		{
			RSC(MEM_CTRL_CHANNEL).CODE(),
			RSC(DDR4_CL).CODE(),
			RSC(DDR4_RCD).CODE(),
			RSC(DDR4_RP).CODE(),
			RSC(DDR4_RAS).CODE(),
			RSC(DDR4_RRD_S).CODE(),
			RSC(DDR4_RRD_L).CODE(),
			RSC(DDR4_FAW).CODE(),
			RSC(DDR4_WR).CODE(),
			RSC(DDR4_RTP).CODE(),
			RSC(DDR4_WTP).CODE(),
			RSC(DDR4_CWL).CODE(),
			RSC(DDR4_CKE).CODE(),
			RSC(DDR4_CMD).CODE(),
			RSC(DDR4_GEAR).CODE()
		},
		{
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_RDRD_SCL).CODE(),
			RSC(DDR4_RDRD_SC).CODE(),
			RSC(DDR4_RDRD_SD).CODE(),
			RSC(DDR4_RDRD_DD).CODE(),
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_RDWR_SCL).CODE(),
			RSC(DDR4_RDWR_SC).CODE(),
			RSC(DDR4_RDWR_SD).CODE(),
			RSC(DDR4_RDWR_DD).CODE(),
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_WRRD_SCL).CODE(),
			RSC(DDR4_WRRD_SC).CODE(),
			RSC(DDR4_WRRD_SD).CODE(),
			RSC(DDR4_WRRD_DD).CODE()
		},
		{
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_WRWR_SCL).CODE(),
			RSC(DDR4_WRWR_SC).CODE(),
			RSC(DDR4_WRWR_SD).CODE(),
			RSC(DDR4_WRWR_DD).CODE(),
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_REFI).CODE(),
			RSC(DDR4_RFC).CODE(),
			RSC(DDR3_XS).CODE(),
			RSC(DDR3_XP).CODE(),
			RSC(DDR4_CPDED).CODE(),
			RSC(DDR4_ECC).CODE()
		}
	};
	const ASCII *Footer_DDR4[3][MC_MATX] = {
		{
			NULL,
			RSC(DDR3_CL_COMM).CODE(),
			RSC(DDR3_RCD_COMM).CODE(),
			RSC(DDR3_RP_COMM).CODE(),
			RSC(DDR3_RAS_COMM).CODE(),
			RSC(DDR4_RRD_S_COMM).CODE(),
			RSC(DDR4_RRD_L_COMM).CODE(),
			RSC(DDR3_FAW_COMM).CODE(),
			RSC(DDR3_WR_COMM).CODE(),
			RSC(DDR3_RTP_COMM).CODE(),
			RSC(DDR3_WTP_COMM).CODE(),
			RSC(DDR3_CWL_COMM).CODE(),
			RSC(DDR3_CKE_COMM).CODE(),
			RSC(DDR3_CMD_COMM).CODE(),
			RSC(DDR4_GEAR_COMM).CODE()
		},
		{
			NULL,
			RSC(DDR4_RDRD_SCL_COMM).CODE(),
			RSC(DDR4_RDRD_SC_COMM).CODE(),
			RSC(DDR4_RDRD_SD_COMM).CODE(),
			RSC(DDR4_RDRD_DD_COMM).CODE(),
			NULL,
			RSC(DDR4_RDWR_SCL_COMM).CODE(),
			RSC(DDR4_RDWR_SC_COMM).CODE(),
			RSC(DDR4_RDWR_SD_COMM).CODE(),
			RSC(DDR4_RDWR_DD_COMM).CODE(),
			NULL,
			RSC(DDR4_WRRD_SCL_COMM).CODE(),
			RSC(DDR4_WRRD_SC_COMM).CODE(),
			RSC(DDR4_WRRD_SD_COMM).CODE(),
			RSC(DDR4_WRRD_DD_COMM).CODE()
		},
		{
			NULL,
			RSC(DDR4_WRWR_SCL_COMM).CODE(),
			RSC(DDR4_WRWR_SC_COMM).CODE(),
			RSC(DDR4_WRWR_SD_COMM).CODE(),
			RSC(DDR4_WRWR_DD_COMM).CODE(),
			NULL,
			NULL,
			NULL,
			NULL,
			RSC(DDR3_REFI_COMM).CODE(),
			RSC(DDR3_RFC_COMM).CODE(),
			RSC(DDR3_XS_COMM).CODE(),
			RSC(DDR3_XP_COMM).CODE(),
			RSC(DDR4_CPDED_COMM).CODE(),
			RSC(DDR3_ECC_COMM).CODE()
		}
	};
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	CUINT	nc;
	unsigned short cha;

	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC, attrib[0], Header_DDR4[0][nc]),
				(char*) Footer_DDR4[0][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tCL);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRCD);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRP);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRAS);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRRDS);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRRDL);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tFAW);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWR);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRTPr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWTPr);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tCWL);
		PRT(IMC, attrib[1], "%4u\x20", TIMING(mc, cha).tCKE);
		PRT(IMC, attrib[1], "%3uT\x20", TIMING(mc, cha).CMD_Rate);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).GEAR);
	}
	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC, attrib[0], Header_DDR4[1][nc]),
				(char*) Footer_DDR4[1][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDRD_SG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDRD_DG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDRD_DR);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDRD_DD);

		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDWR_SG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDWR_DG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDWR_DR);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRDWR_DD);

		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRRD_SG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRRD_DG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRRD_DR);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRRD_DD);
	}
	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC, attrib[0], Header_DDR4[2][nc]),
				(char*) Footer_DDR4[2][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRWR_SG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRWR_DG);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRWR_DR);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tWRWR_DD);

		for (nc = 0; nc < (MC_MATX - 11); nc++) {
			PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
		}
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tREFI);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tRFC);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tXS);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tXP);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).tCPDED);
		PRT(IMC, attrib[1], "%5u", TIMING(mc, cha).ECC);
	}
}

void Timing_DDR4_Zen(Window *win,CELL_FUNC OutFunc,CUINT *nl, unsigned short mc)
{
	const ASCII *Header_DDR4_Zen[4][MC_MATX] = {
		{
			RSC(MEM_CTRL_CHANNEL).CODE(),
			RSC(DDR4_ZEN_CL).CODE(),
			RSC(DDR4_ZEN_RCD_R).CODE(),
			RSC(DDR4_ZEN_RCD_W).CODE(),
			RSC(DDR4_ZEN_RP).CODE(),
			RSC(DDR4_ZEN_RAS).CODE(),
			RSC(DDR4_ZEN_RC).CODE(),
			RSC(DDR4_RRD_S).CODE(),
			RSC(DDR4_RRD_L).CODE(),
			RSC(DDR4_ZEN_FAW).CODE(),
			RSC(DDR4_ZEN_WTR_S).CODE(),
			RSC(DDR4_ZEN_WTR_L).CODE(),
			RSC(DDR4_ZEN_WR).CODE(),
			RSC(DDR4_ZEN_RDRD_SCL).CODE(),
			RSC(DDR4_ZEN_WRWR_SCL).CODE()
		},
		{
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_ZEN_CWL).CODE(),
			RSC(DDR4_ZEN_RTP).CODE(),
			RSC(DDR4_ZEN_RDWR).CODE(),
			RSC(DDR4_ZEN_WRRD).CODE(),
			RSC(DDR4_ZEN_WRWR_SC).CODE(),
			RSC(DDR4_ZEN_WRWR_SD).CODE(),
			RSC(DDR4_ZEN_WRWR_DD).CODE(),
			RSC(DDR4_ZEN_RDRD_SC).CODE(),
			RSC(DDR4_ZEN_RDRD_SD).CODE(),
			RSC(DDR4_ZEN_RDRD_DD).CODE(),
			RSC(DDR4_ZEN_RTR_DLR).CODE(),
			RSC(DDR4_ZEN_WTW_DLR).CODE(),
			RSC(DDR4_ZEN_WTR_DLR).CODE(),
			RSC(DDR4_ZEN_RRD_DLR).CODE()
		},
		{
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_ZEN_REFI).CODE(),
			RSC(DDR4_ZEN_RFC1).CODE(),
			RSC(DDR4_ZEN_RFC2).CODE(),
			RSC(DDR4_ZEN_RFC4).CODE(),
			RSC(DDR4_ZEN_RCPB).CODE(),
			RSC(DDR4_ZEN_RPPB).CODE(),
			RSC(DDR4_ZEN_BGS).CODE(),
			RSC(DDR4_ZEN_BGS_ALT).CODE(),
			RSC(DDR4_ZEN_BAN).CODE(),
			RSC(DDR4_ZEN_RCPAGE).CODE(),
			RSC(DDR4_ZEN_CKE).CODE(),
			RSC(DDR4_ZEN_CMD).CODE(),
			RSC(DDR4_ZEN_GDM).CODE(),
			RSC(DDR4_ZEN_ECC).CODE()
		},
		{
			RSC(MEM_CTRL_MTY_CELL).CODE(),
			RSC(DDR4_ZEN_MRD).CODE(),
			RSC(DDR4_ZEN_MRD_PDA).CODE(),
			RSC(DDR4_ZEN_MOD).CODE(),
			RSC(DDR4_ZEN_MOD_PDA).CODE(),
			RSC(DDR4_ZEN_WRMPR).CODE(),
			RSC(DDR4_ZEN_STAG).CODE(),
			RSC(DDR4_ZEN_PDM).CODE(),
			RSC(DDR4_ZEN_RDDATA).CODE(),
			RSC(DDR4_ZEN_PHYWRD).CODE(),
			RSC(DDR4_ZEN_PHYWRL).CODE(),
			RSC(DDR4_ZEN_PHYRDL).CODE(),
			RSC(DDR3_XS).CODE(),
			RSC(DDR3_XP).CODE(),
			RSC(DDR4_CPDED).CODE()
		}
	};
	const ASCII *Footer_DDR4_Zen[4][MC_MATX] = {
		{
			NULL,
			RSC(DDR3_CL_COMM).CODE(),
			RSC(DDR4_ZEN_RCD_R_COMM).CODE(),
			RSC(DDR4_ZEN_RCD_W_COMM).CODE(),
			RSC(DDR3_RP_COMM).CODE(),
			RSC(DDR3_RAS_COMM).CODE(),
			RSC(DDR4_ZEN_RC_COMM).CODE(),
			RSC(DDR4_RRD_S_COMM).CODE(),
			RSC(DDR4_RRD_L_COMM).CODE(),
			RSC(DDR3_FAW_COMM).CODE(),
			RSC(DDR4_ZEN_WTR_S_COMM).CODE(),
			RSC(DDR4_ZEN_WTR_L_COMM).CODE(),
			RSC(DDR3_WR_COMM).CODE(),
			RSC(DDR4_ZEN_RDRD_SCL_COMM).CODE(),
			RSC(DDR4_ZEN_WRWR_SCL_COMM).CODE()
		},
		{
			NULL,
			RSC(DDR3_CWL_COMM).CODE(),
			RSC(DDR4_ZEN_RTP_COMM).CODE(),
			RSC(DDR4_ZEN_RDWR_COMM).CODE(),
			RSC(DDR4_ZEN_WRRD_COMM).CODE(),
			RSC(DDR4_ZEN_WRWR_SC_COMM).CODE(),
			RSC(DDR4_ZEN_WRWR_SD_COMM).CODE(),
			RSC(DDR4_ZEN_WRWR_DD_COMM).CODE(),
			RSC(DDR4_ZEN_RDRD_SC_COMM).CODE(),
			RSC(DDR4_ZEN_RDRD_SD_COMM).CODE(),
			RSC(DDR4_ZEN_RDRD_DD_COMM).CODE(),
			RSC(DDR4_ZEN_RTR_DLR_COMM).CODE(),
			RSC(DDR4_ZEN_WTW_DLR_COMM).CODE(),
			RSC(DDR4_ZEN_WTR_DLR_COMM).CODE(),
			RSC(DDR4_ZEN_RRD_DLR_COMM).CODE()
		},
		{
			NULL,
			RSC(DDR3_REFI_COMM).CODE(),
			RSC(DDR4_ZEN_RFC1_COMM).CODE(),
			RSC(DDR4_ZEN_RFC2_COMM).CODE(),
			RSC(DDR4_ZEN_RFC4_COMM).CODE(),
			RSC(DDR4_ZEN_RCPB_COMM).CODE(),
			RSC(DDR4_ZEN_RPPB_COMM).CODE(),
			RSC(DDR4_ZEN_BGS_COMM).CODE(),
			RSC(DDR4_ZEN_BGS_ALT_COMM).CODE(),
			RSC(DDR4_ZEN_BAN_COMM).CODE(),
			RSC(DDR4_ZEN_RCPAGE_COMM).CODE(),
			RSC(DDR3_CKE_COMM).CODE(),
			RSC(DDR3_CMD_COMM).CODE(),
			RSC(DDR4_ZEN_GDM_COMM).CODE(),
			RSC(DDR3_ECC_COMM).CODE()
		},
		{
			NULL,
			RSC(DDR4_ZEN_MRD_COMM).CODE(),
			RSC(DDR4_ZEN_MRD_PDA_COMM).CODE(),
			RSC(DDR4_ZEN_MOD_COMM).CODE(),
			RSC(DDR4_ZEN_MOD_PDA_COMM).CODE(),
			RSC(DDR4_ZEN_WRMPR_COMM).CODE(),
			RSC(DDR4_ZEN_STAG_COMM).CODE(),
			RSC(DDR4_ZEN_PDM_COMM).CODE(),
			RSC(DDR4_ZEN_RDDATA_COMM).CODE(),
			RSC(DDR4_ZEN_PHYWRD_COMM).CODE(),
			RSC(DDR4_ZEN_PHYWRL_COMM).CODE(),
			RSC(DDR4_ZEN_PHYRDL_COMM).CODE(),
			RSC(DDR3_XS_COMM).CODE(),
			RSC(DDR3_XP_COMM).CODE(),
			RSC(DDR4_CPDED_COMM).CODE()
		}
	};
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	CUINT	nc;
	unsigned short cha;

	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC,attrib[0], Header_DDR4_Zen[0][nc]),
			(char*) Footer_DDR4_Zen[0][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tCL);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRCD_RD);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRCD_WR);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRP);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRAS);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRC);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRRDS);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRRDL);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tFAW);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tWTRS);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tWTRL);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tWR);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tRdRdScl);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tWrWrScl);
	}
	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC,attrib[0], Header_DDR4_Zen[1][nc]),
			(char*) Footer_DDR4_Zen[1][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tCWL);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).tRTP);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tddRdTWr);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tddWrTRd);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tscWrTWr);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tsdWrTWr);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tddWrTWr);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tscRdTRd);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tsdRdTRd);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tddRdTRd);

		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tRdRdScDLR);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tWrWrScDLR);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tWrRdScDLR);
		PRT(IMC, attrib[1], "%4u ", TIMING(mc, cha).Zen.tRRDDLR);
	}
	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC,attrib[0], Header_DDR4_Zen[2][nc]),
				(char*) Footer_DDR4_Zen[2][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%5u",	TIMING(mc, cha).tREFI);
		PRT(IMC, attrib[1], "%5u",	TIMING(mc, cha).tRFC1);
		PRT(IMC, attrib[1], "%5u",	TIMING(mc, cha).tRFC2);
		PRT(IMC, attrib[1], "%5u",	TIMING(mc, cha).tRFC4);

		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tRCPB);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tRPPB);

		PRT(IMC, attrib[1], "  %3s",	ENABLED(TIMING(mc, cha).BGS));
		PRT(IMC, attrib[1], " %3s ",  ENABLED(TIMING(mc, cha).BGS_ALT));

		PRT(IMC, attrib[1], " R%1uW%1u", TIMING(mc,cha).Zen.tRdRdBan,
						 TIMING(mc, cha).Zen.tWrWrBan);

		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tRCPage);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tCKE);
		PRT(IMC, attrib[1], "%3uT ",	TIMING(mc, cha).CMD_Rate);
		PRT(IMC, attrib[1], "  %3s",	ENABLED(TIMING(mc,cha).GDM));
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).ECC);
	}
	for (nc = 0; nc < MC_MATX; nc++) {
		GridHover(	PRT(IMC,attrib[0], Header_DDR4_Zen[3][nc]),
				(char*) Footer_DDR4_Zen[3][nc] );
	}
	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], "\x20\x20#%-2u", cha);

		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tMRD);
		PRT(IMC, attrib[1], " %-4u",	TIMING(mc, cha).tMRD_PDA);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tMOD);
		PRT(IMC, attrib[1], " %-4u",	TIMING(mc, cha).tMOD_PDA);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tWRMPR);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tSTAG);
		PRT(IMC, attrib[1],"%1u:%c:%1u",TIMING(mc, cha).PDM_AGGR,
						TIMING(mc, cha).PDM_MODE ?
								'P' : 'F',
						TIMING(mc, cha).PDM_EN);
		PRT(IMC, attrib[1], "%5u",	TIMING(mc, cha).tRDDATA);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tPHYWRD);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tPHYWRL);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tPHYRDL);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tXS);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tXP);
		PRT(IMC, attrib[1], "%4u ",	TIMING(mc, cha).tCPDED);
	}
}

#undef TIMING

void MemoryController(Window *win, CELL_FUNC OutFunc, TIMING_FUNC TimingFunc)
{
	ATTRIBUTE *attrib[2] = {
		RSC(MEMORY_CONTROLLER_COND0).ATTR(),
		RSC(MEMORY_CONTROLLER_COND1).ATTR()
	};
	const ASCII *MC_Unit[4] = {
		[MC_MHZ] = RSC(MEM_CTRL_UNIT_MHZ).CODE(),
		[MC_MTS] = RSC(MEM_CTRL_UNIT_MTS).CODE(),
		[MC_MBS] = RSC(MEM_CTRL_UNIT_MBS).CODE(),
		[MC_NIL] = (ASCII*) MEM_CTRL_FMT
	};
	char	item[MC_MATY + 1], *str = malloc(CODENAME_LEN + 4 + 10),
		*chipStr = malloc((MC_MATX * MC_MATY) + 1);
  if ((str != NULL) && (chipStr != NULL))
  {
	CUINT	cells_per_line = win->matrix.size.wth, *nl = &cells_per_line,
		ni, nc, li;

	unsigned short mc, cha, slot;

	StrLenFormat(li, str, CODENAME_LEN + 4 + 10,
			"%s  [%4hX]",
			RO(Shm)->Uncore.Chipset.CodeName,
			RO(Shm)->Uncore.ChipID);

	nc = (MC_MATX * MC_MATY) - li;
	ni = nc >> 1;
	nc = ni + (nc & 0x1);

	StrLenFormat(li, chipStr, (MC_MATX * MC_MATY) + 1,
			"%.*s" "%s" "%.*s",
			nc % ((MC_MATX * MC_MATY) / 2), HSPACE,
			str,
			ni % ((MC_MATX * MC_MATY) / 2), HSPACE);

    for (nc = 0; nc < MC_MATX; nc++) {
	memcpy(item, &chipStr[nc * MC_MATY], MC_MATY);
	item[MC_MATY] = '\0';
	PRT(IMC, attrib[1], item);
    }
    for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
    {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT1_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT1_1).CODE());
	PRT(IMC, attrib[1], RSC(MEM_CTRL_SUBSECT1_2).CODE(), mc);

	for (nc = 0; nc < (MC_MATX - 6); nc++) {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
	}

	switch (RO(Shm)->Uncore.MC[mc].ChannelCount) {
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
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DISABLED_0).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DISABLED_1).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DISABLED_2).CODE());
		break;
	}
      if (RO(Shm)->Uncore.MC[mc].ChannelCount > 0)
      {
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_RATE_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_RATE_1).CODE());
	PRT(IMC, attrib[1], "%5llu", RO(Shm)->Uncore.Bus.Rate);

	PRT(IMC, attrib[0], MC_Unit[
					RO(Shm)->Uncore.Unit.Bus_Rate
				], MC_MATY,HSPACE);

	PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);

	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_SPEED_0).CODE());
	PRT(IMC, attrib[0], RSC(MEM_CTRL_BUS_SPEED_1).CODE());
	PRT(IMC, attrib[1], "%5llu", RO(Shm)->Uncore.Bus.Speed);

	PRT(IMC, attrib[0], MC_Unit[
					RO(Shm)->Uncore.Unit.BusSpeed
				], MC_MATY,HSPACE);

	for (nc = 0; nc < (MC_MATX - 14); nc++) {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
	}

      switch (RO(Shm)->Uncore.Unit.DDR_Std) {
      case RAM_STD_UNSPEC:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_RAM_STD_0).CODE());
	break;
      case RAM_STD_SDRAM:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_RAM_STD_1).CODE());
	break;
      case RAM_STD_LPDDR:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_RAM_STD_2).CODE());
	break;
      case RAM_STD_RDIMM:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_RAM_STD_3).CODE());
	break;
      }
      switch (RO(Shm)->Uncore.Unit.DDR_Ver) {
      case 2:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_DDR2_0).CODE());
	break;
      case 3:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_DDR3_0).CODE());
	break;
      case 4:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_DDR4_0).CODE());
	break;
      case 5:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_DDR5_0).CODE());
	break;
      default:
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_SPEED_0).CODE());
	break;
      }
	PRT(IMC, attrib[0], RSC(MEM_CTRL_DRAM_SPEED_1).CODE());
	PRT(IMC, attrib[1], "%5llu", RO(Shm)->Uncore.CtrlSpeed);

	PRT(IMC, attrib[0], MC_Unit[
					RO(Shm)->Uncore.Unit.DDRSpeed
				], MC_MATY,HSPACE);

	for (nc = 0; nc < MC_MATX; nc++) {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
	}

	TimingFunc(win, OutFunc, nl, mc);

	for (nc = 0; nc < MC_MATX; nc++) {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
	}

	for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	{
		PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_0).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_1).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_2).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_3).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_4).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_SUBSECT2_5).CODE(), cha);
		for (nc = 0; nc < (MC_MATX - 6); nc++) {
			PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
		}

		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SLOT).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_BANK).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_RANK).CODE());
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_ROW).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_COLUMN0).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_COLUMN1).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_0).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_1).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_2).CODE());
		PRT(IMC, attrib[0], RSC(MEM_CTRL_DIMM_SIZE_3).CODE());
		for (nc = 0; nc < (MC_MATX - 12); nc++) {
			PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
		}

	  for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
	  {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
		PRT(IMC, attrib[0], "\x20\x20#%-2u", slot);

	    if (RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size > 0)
	    {
		PRT(IMC, attrib[1], "%5u",
			RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks);
		PRT(IMC, attrib[1], "%5u",
			RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);

		iSplit(RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows,str);
		PRT(IMC, attrib[1], "%5s", &str[0]);
		PRT(IMC, attrib[1], "%5s", &str[8]);

		iSplit(RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols,str);
		PRT(IMC, attrib[1], "%5s", &str[0]);
		PRT(IMC, attrib[1], "%5s", &str[8]);
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);

		iSplit(RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size,str);
		PRT(IMC, attrib[1], "%5s", &str[0]);
		PRT(IMC, attrib[1], "%5s", &str[8]);

	     if ((li = strlen(RO(Shm)->SMB.Memory.PartNumber[slot])) > 0)
	     {
		if (li >= (4 * MC_MATY)) {
			li = (4 * MC_MATY) - 1;
		}
		StrFormat(str, CODENAME_LEN + 4 + 10,
			"%%%d.*s", (MC_MATX - 11) * MC_MATY);

		StrFormat(chipStr, (MC_MATX * MC_MATY) + 1,
			str, li, RO(Shm)->SMB.Memory.PartNumber[slot]);

	      for (nc = 0; nc < (MC_MATX - 11); nc++) {
		memcpy(item, &chipStr[nc * MC_MATY], MC_MATY);
		item[MC_MATY] = '\0';
		PRT(IMC, attrib[1], "%5s", item);
	      }
	     } else  for (nc = 0; nc < (MC_MATX - 11); nc++) {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
	     }
	    }
	    else for (nc = 0; nc < (MC_MATX - 2); nc++) {
		PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
	    }
	  }
	}
      }
	if (mc < (RO(Shm)->Uncore.CtrlCount - 1)) {
		for (nc = 0; nc < MC_MATX; nc++) {
			PRT(IMC, attrib[0], MEM_CTRL_FMT, MC_MATY, HSPACE);
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

struct DRAW_ST Draw = {
	.Cache = {
	    #ifndef NO_HEADER
		.TopAvg = 0.0,
	    #endif
	    #ifndef NO_FOOTER
		.FreeRAM= 0,
		.procCount = 0
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
	.Area		= {
				.MinHeight = 0, .MaxRows = 0, .LoadWidth = 0,
	    #ifndef NO_FOOTER
		.Footer = {
			.VoltTemp = {
				.Hot = {
					[0] = MIN_WIDTH - 13,
					[1] = MIN_WIDTH - 20
				},
			},
			.TaskMem = {
				.Total	= MIN_WIDTH - 35,
				.Free	= MIN_WIDTH - 22,
				.Count	= MIN_WIDTH - 12
			},
		},
	    #endif
	},
	.iClock 	= 0,
	.cpuScroll	= 0,
	.Load		= 0,
	.Unit		= { .Memory = 0 },
	.SmbIndex	= SMB_BOARD_NAME,
	.Theme		= THM_DFLT,
    #ifndef NO_UPPER
	.garbage	= InitCC(0)
    #endif
};

struct RECORDER_ST Recorder = {
		.Reset = 0,
		.Select = 1,
		.Ratios = {0, 1, 2, 10, 20, 60, 90, 120, 240, 0}
};
/* <<< GLOBALS <<< */

int ByteReDim(unsigned long ival, int constraint, unsigned long *oval)
{
    if (ival > 0)
    {
	int base = 1 + (int) log10(ival);

	(*oval) = ival;
	if (base > constraint) {
		(*oval) = (*oval) >> 10;
		return 1 + ByteReDim((*oval), constraint, oval);
	} else {
		return 0;
	}
    } else {
	(*oval) = 0;
	return 0;
    }
}

#define Threshold(value, threshold1, threshold2, _low, _medium, _high)	\
({									\
	ATTRIBUTE _retAttr;						\
	if (value > threshold2) {					\
		_retAttr = _high;					\
	} else if (value > threshold1) {				\
		_retAttr = _medium;					\
	} else {							\
		_retAttr = _low;					\
	}								\
	_retAttr;							\
})

#define frtostr(r, d, pStr)						\
({									\
    if (r > 0) {							\
	int p = d - ((int) log10(r)) - 2 ;				\
	StrFormat(pStr, d+1, "%*.*f", d, p, r) ;			\
    } else {								\
	StrFormat(pStr, d+1, "%.*s", d, HSPACE);			\
    }									\
	pStr;								\
})

#define Clock2LCD(layer, col, row, value1, value2)			\
({									\
	StrFormat(Buffer, 4+1, "%04.0f", value1);			\
	PrintLCD(layer, col, row, 4, Buffer,				\
		Threshold(value2, Ruler.Minimum, Ruler.Median,		\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_CLOCK_LOW],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_CLOCK_MEDIUM],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_CLOCK_HIGH]));	\
})

#define Counter2LCD(layer, col, row, value)				\
({									\
	StrFormat(Buffer, 4+1, "%04.0f" , value);			\
	PrintLCD(layer, col, row, 4, Buffer,				\
		Threshold(value, 0.f, 1.f,				\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_COUNTER_LOW],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_COUNTER_MEDIUM],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_COUNTER_HIGH]));	\
})

#define Load2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, Buffer) ,	\
		Threshold(value, 100.f/3.f, 100.f/1.5f ,		\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_LOAD_LOW] ,	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_LOAD_MEDIUM],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_LOAD_HIGH]))

#define Idle2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, Buffer) ,	\
		Threshold(value, 100.f/3.f, 100.f/1.5f ,		\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_IDLE_LOW] ,	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_IDLE_MEDIUM],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_IDLE_HIGH]))

#define Sys2LCD(layer, col, row, value) 				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, Buffer) ,	\
		Threshold(value, 100.f/6.6f, 50.0,			\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_SYSTEM_LOW],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_SYSTEM_MEDIUM],	\
			RSC(UI).ATTR()[UI_LAYOUT_LCD_SYSTEM_HIGH]))

void ForEachCellPrint_Menu(Window *win, void *plist)
{
	WinList *list = (WinList *) plist;
	CUINT col, row;
	int len;

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
	len = Draw.Size.width
	- (int) win->lazyComp.rowLen
	- (int) win->matrix.origin.col;
    if (len > 0)
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
					3, 14, 3, 0,
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

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_i,	RSC(MENU_ITEM_INST_CYCLES).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_i,	RSC(MENU_ITEM_INST_CYCLES).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_e,	RSC(MENU_ITEM_FEATURES).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  4 */
	StoreTCell(wMenu, SCANKEY_SHIFT_b, RSC(MENU_ITEM_SMBIOS).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_c,	RSC(MENU_ITEM_CORE_CYCLES).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_c,	RSC(MENU_ITEM_CORE_CYCLES).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_i, RSC(MENU_ITEM_ISA_EXT).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  5 */
	StoreTCell(wMenu, SCANKEY_k,	RSC(MENU_ITEM_KERNEL).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_l,	RSC(MENU_ITEM_IDLE_STATES).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_l,	RSC(MENU_ITEM_IDLE_STATES).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_t,	RSC(MENU_ITEM_TECH).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  6 */
	StoreTCell(wMenu, SCANKEY_HASH, RSC(MENU_ITEM_HOTPLUG).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_g,	RSC(MENU_ITEM_PKG_CYCLES).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_g,	RSC(MENU_ITEM_PKG_CYCLES).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_o,	RSC(MENU_ITEM_PERF_MON).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  7 */
	StoreTCell(wMenu, SCANKEY_SHIFT_o, RSC(MENU_ITEM_TOOLS).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_x,	RSC(MENU_ITEM_TASKS_MON).CODE(),
			BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1) ?
					RSC(CREATE_MENU_SHORTKEY).ATTR()
					: RSC(CREATE_MENU_DISABLE).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_x,	RSC(MENU_ITEM_TASKS_MON).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_w,	RSC(MENU_ITEM_POW_THERM).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  8 */
	StoreTCell(wMenu, SCANKEY_SHIFT_e, RSC(MENU_ITEM_THEME).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_q,	RSC(MENU_ITEM_SYS_INTER).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_q,	RSC(MENU_ITEM_SYS_INTER).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_u,	RSC(MENU_ITEM_CPUID).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row  9 */
	StoreTCell(wMenu, SCANKEY_a,	RSC(MENU_ITEM_ABOUT).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_SHIFT_c, RSC(MENU_ITEM_SENSORS).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_SHIFT_c, RSC(MENU_ITEM_SENSORS).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_r, RSC(MENU_ITEM_SYS_REGS).CODE(),
					   RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row 10 */
	StoreTCell(wMenu, SCANKEY_h,	RSC(MENU_ITEM_HELP).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_SHIFT_v, RSC(MENU_ITEM_VOLTAGE).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_SHIFT_v, RSC(MENU_ITEM_VOLTAGE).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_m, RSC(MENU_ITEM_MEM_CTRL).CODE(),
				RO(Shm)->Uncore.CtrlCount > 0 ?
					RSC(CREATE_MENU_SHORTKEY).ATTR()
					: RSC(CREATE_MENU_DISABLE).ATTR());
/* Row 11 */
	StoreTCell(wMenu, SCANKEY_CTRL_x, RSC(MENU_ITEM_QUIT).CODE(),
					  RSC(CREATE_MENU_CTRL_KEY).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_SHIFT_w, RSC(MENU_ITEM_POWER).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_SHIFT_w, RSC(MENU_ITEM_POWER).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_SHIFT_h, RSC(MENU_ITEM_EVENTS).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
/* Row 12 */
	StoreTCell(wMenu, SCANKEY_VOID, "", RSC(VOID).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_SHIFT_t, RSC(MENU_ITEM_SLICE_CTRS).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_SHIFT_t, RSC(MENU_ITEM_SLICE_CTRS).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_VOID, "", RSC(VOID).ATTR());
/* Row 13 */
	StoreTCell(wMenu, SCANKEY_VOID, "", RSC(VOID).ATTR());

#ifndef NO_LOWER
	StoreTCell(wMenu, SCANKEY_y,	RSC(MENU_ITEM_CUSTOM).CODE(),
					RSC(CREATE_MENU_SHORTKEY).ATTR());
#else
	StoreTCell(wMenu, SCANKEY_y,	RSC(MENU_ITEM_CUSTOM).CODE(),
					RSC(CREATE_MENU_DISABLE).ATTR());
#endif

	StoreTCell(wMenu, SCANKEY_VOID, "", RSC(VOID).ATTR());
/* Bottom Menu */
	StoreWindow(wMenu, .color[0].select,
				RSC(UI).ATTR()[UI_WIN_MENU_UNSELECT]);

	StoreWindow(wMenu, .color[1].select,
				RSC(UI).ATTR()[UI_WIN_MENU_SELECT]);

	StoreWindow(wMenu, .color[0].title,
				RSC(UI).ATTR()[UI_WIN_MENU_TITLE_UNFOCUS]);

	StoreWindow(wMenu, .color[1].title,
				RSC(UI).ATTR()[UI_WIN_MENU_TITLE_FOCUS]);

	wMenu->matrix.select.col = matrixSelectCol;

	StoreWindow(wMenu,	.Print ,	ForEachCellPrint_Menu);
	StoreWindow(wMenu,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wMenu,	.key.Left,	MotionLeft_Menu);
	StoreWindow(wMenu,	.key.Right,	MotionRight_Menu);
	StoreWindow(wMenu,	.key.Down,	MotionDown_Menu);
	StoreWindow(wMenu,	.key.Up,	MotionUp_Menu);
	StoreWindow(wMenu,	.key.Home,	MotionHome_Menu);
	StoreWindow(wMenu,	.key.End,	MotionEnd_Menu);
    }
	return wMenu;
}

void IntervalUpdate(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	StrFormat(Buffer, 10+1, "%4u", RO(Shm)->Sleep.Interval);
	memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
}

void SysTickUpdate(TGrid *grid, DATA_TYPE data[])
{
	UNUSED(data);
	StrFormat(Buffer, 10+1, "%4u",
			RO(Shm)->Sleep.Interval*RO(Shm)->SysGate.tickReset);

	memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
}

void SvrWaitUpdate(TGrid *grid, DATA_TYPE data[])
{
	StrFormat(Buffer, 21+1, "%4ld", (*data[0].pslong) / 1000000L);
	memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
}

void RecorderUpdate(TGrid *grid, DATA_TYPE data[])
{
	unsigned int duration = RECORDER_SECONDS((*data[0].puint),
						RO(Shm)->Sleep.Interval);
	if (duration <= 9999) {
		StrFormat(Buffer, 11+1, "%4u", duration);
		memcpy(&grid->cell.item[grid->cell.length - 6], Buffer, 4);
	}
}

void SettingUpdate(TGrid *grid, const unsigned int bix, const int pos,
				const size_t len, const char *item)
{
	ATTRIBUTE *attrib[2] = {
		RSC(CREATE_SETTINGS_COND0).ATTR(),
		RSC(CREATE_SETTINGS_COND1).ATTR()
	};
	memcpy(&grid->cell.attr[pos], &attrib[bix][pos], len);
	memcpy(&grid->cell.item[pos], item, len);
}

void AutoClockUpdate(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = (RO(Shm)->Registration.AutoClock & 0b10) != 0;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void ExperimentalUpdate(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.Experimental != 0;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void HotPlug_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = !(RO(Shm)->Registration.HotPlug < 0);
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void PCI_Probe_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.PCI == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void HSMP_Registration_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.HSMP == 1;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void IdleRoute_Update(TGrid *grid, DATA_TYPE data[])
{
	const ASCII *instructions[ROUTE_SIZE] = {
		[ROUTE_DEFAULT] = RSC(SETTINGS_ROUTE_DFLT).CODE(),
		[ROUTE_IO]	= RSC(SETTINGS_ROUTE_IO).CODE(),
		[ROUTE_HALT]	= RSC(SETTINGS_ROUTE_HALT).CODE(),
		[ROUTE_MWAIT]	= RSC(SETTINGS_ROUTE_MWAIT).CODE()
	};
	UNUSED(data);

	const unsigned int bix	= RO(Shm)->Registration.Driver.CPUidle
				& REGISTRATION_ENABLE;
	const size_t size = (size_t) RSZ(SETTINGS_ROUTE_DFLT);

	memcpy(&grid->cell.item[grid->cell.length - size - 2],
		instructions[RO(Shm)->Registration.Driver.Route], size);

	grid->cell.item[grid->cell.length -  2] = bix ? '>' : ']';
	grid->cell.item[grid->cell.length - 10] = bix ? '<' : '[';
	grid->cell.quick.key = bix ? OPS_IDLE_ROUTE : SCANKEY_NULL;
}

void NMI_Registration_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = BITWISEAND(	LOCKLESS,
						RO(Shm)->Registration.NMI,
						BIT_NMI_MASK ) != 0;
	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void CPU_Idle_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.Driver.CPUidle
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void CPU_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.Driver.CPUfreq
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void Governor_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.Driver.Governor
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void ClockSource_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int bix = RO(Shm)->Registration.Driver.CS
				& REGISTRATION_ENABLE;

	const signed int pos = grid->cell.length - 5;
	UNUSED(data);

	SettingUpdate(grid, bix, pos, 3, ENABLED(bix));
}

void ScopeUpdate(TGrid *grid, DATA_TYPE data[])
{
	ASCII *code[] = {
		RSC(SCOPE_NONE).CODE(),
		RSC(SCOPE_THREAD).CODE(),
		RSC(SCOPE_CORE).CODE(),
		RSC(SCOPE_PACKAGE).CODE()
	};
	const enum FORMULA_SCOPE scope = SCOPE_OF_FORMULA(*data[0].psint);
	const size_t size = (size_t) RSZ(SCOPE_NONE);
	memcpy(&grid->cell.item[grid->cell.length - size-2], code[scope], size);
}

Window *CreateSettings(unsigned long long id)
{
	const CUINT height = \
			(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ?
			26 : 25;
	Window *wSet = CreateWindow(wLayer, id, 1, height, 8, TOP_HEADER_ROW+2);
    if (wSet != NULL)
    {
	ATTRIBUTE *attrib[2] = {
		RSC(CREATE_SETTINGS_COND0).ATTR(),
		RSC(CREATE_SETTINGS_COND1).ATTR()
	};
	TGrid *grid;
	unsigned int bix;

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	grid = StoreTCell(wSet, SCANKEY_NULL, RSC(SETTINGS_DAEMON).CODE(),
				MAKE_PRINT_UNFOCUS);
	if (grid != NULL) {
		const size_t length = strlen(RO(Shm)->ShmName);
		memcpy(&grid->cell.item[grid->cell.length - length - 1],
			RO(Shm)->ShmName, length);
	}
	GridCall( StoreTCell(	wSet, OPS_INTERVAL,
				RSC(SETTINGS_INTERVAL).CODE(),
				MAKE_PRINT_UNFOCUS ),
		IntervalUpdate );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_SYS_TICK).CODE(),
				MAKE_PRINT_UNFOCUS ),
		SysTickUpdate );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_POLL_WAIT).CODE(),
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &RO(Shm)->Sleep.pollingWait.tv_nsec );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_RING_WAIT).CODE(),
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &RO(Shm)->Sleep.ringWaiting.tv_nsec );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_CHILD_WAIT).CODE(),
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &RO(Shm)->Sleep.childWaiting.tv_nsec );

	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_SLICE_WAIT).CODE(),
				MAKE_PRINT_UNFOCUS ),
		SvrWaitUpdate, &RO(Shm)->Sleep.sliceWaiting.tv_nsec );

	GridCall( StoreTCell(	wSet, OPS_RECORDER,
				RSC(SETTINGS_RECORDER).CODE(),
				MAKE_PRINT_UNFOCUS ),
		RecorderUpdate, &Recorder.Reset );

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	bix = ((RO(Shm)->Registration.AutoClock & 0b10) != 0);
	GridCall( StoreTCell( wSet, OPS_AUTOCLOCK,
			RSC(SETTINGS_AUTO_CLOCK).CODE(),
			attrib[bix] ),
		AutoClockUpdate );

	bix = (RO(Shm)->Registration.Experimental != 0);
	GridCall( StoreTCell(	wSet, OPS_EXPERIMENTAL,
				RSC(SETTINGS_EXPERIMENTAL).CODE(),
				attrib[bix] ),
		ExperimentalUpdate );

	bix = !(RO(Shm)->Registration.HotPlug < 0);
	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_CPU_HOTPLUG).CODE(),
				attrib[bix] ),
		HotPlug_Update );

	bix = (RO(Shm)->Registration.PCI == 1);
	GridCall( StoreTCell(	wSet, SCANKEY_NULL,
				RSC(SETTINGS_PCI_ENABLED).CODE(),
				attrib[bix] ),
		PCI_Probe_Update );

    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
     || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	bix = (RO(Shm)->Registration.HSMP == 1);
	GridCall( StoreTCell(	wSet, BOXKEY_HSMP,
				RSC(SETTINGS_HSMP_ENABLED).CODE(),
				attrib[bix] ),
		HSMP_Registration_Update );
    }
	bix=BITWISEAND(LOCKLESS, RO(Shm)->Registration.NMI, BIT_NMI_MASK) != 0;
	GridCall( StoreTCell(	wSet, OPS_INTERRUPTS,
				RSC(SETTINGS_NMI_REGISTERED).CODE(),
				attrib[bix] ),
		NMI_Registration_Update );

	bix = RO(Shm)->Registration.Driver.CPUidle & REGISTRATION_ENABLE;
	grid = GridCall( StoreTCell(	wSet, bix ? OPS_IDLE_ROUTE:SCANKEY_NULL,
					RSC(SETTINGS_IDLE_ROUTE).CODE(),
					MAKE_PRINT_UNFOCUS ),
			IdleRoute_Update );
	if (grid != NULL) {
		grid->cell.item[grid->cell.length -  2] = bix ? '>' : ']';
		grid->cell.item[grid->cell.length - 10] = bix ? '<' : '[';
	}

	GridCall( StoreTCell(	wSet, OPS_CPU_IDLE,
				RSC(SETTINGS_CPUIDLE_REGISTERED).CODE(),
				attrib[bix] ),
		CPU_Idle_Update );

	bix = RO(Shm)->Registration.Driver.CPUfreq & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_CPU_FREQ,
				RSC(SETTINGS_CPUFREQ_REGISTERED).CODE(),
				attrib[bix] ),
		CPU_Freq_Update );

	bix = RO(Shm)->Registration.Driver.Governor & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_GOVERNOR,
				RSC(SETTINGS_GOVERNOR_REGISTERED).CODE(),
				attrib[bix] ),
		Governor_Update );

	bix = RO(Shm)->Registration.Driver.CS & REGISTRATION_ENABLE;
	GridCall( StoreTCell(	wSet, OPS_CLOCK_SOURCE,
				RSC(SETTINGS_CS_REGISTERED).CODE(),
				attrib[bix] ),
		ClockSource_Update );

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	GridCall( StoreTCell(	wSet, OPS_THERMAL_SCOPE,
				RSC(SETTINGS_THERMAL_SCOPE).CODE(),
				MAKE_PRINT_UNFOCUS ),
		ScopeUpdate, &RO(Shm)->Proc.thermalFormula );

	GridCall( StoreTCell(	wSet, OPS_VOLTAGE_SCOPE,
				RSC(SETTINGS_VOLTAGE_SCOPE).CODE(),
				MAKE_PRINT_UNFOCUS ),
		ScopeUpdate, &RO(Shm)->Proc.voltageFormula );

	GridCall( StoreTCell(	wSet, OPS_POWER_SCOPE,
				RSC(SETTINGS_POWER_SCOPE).CODE(),
				MAKE_PRINT_UNFOCUS ),
		ScopeUpdate, &RO(Shm)->Proc.powerFormula );

	StoreTCell(wSet, SCANKEY_NULL,  RSC(CREATE_SETTINGS_COND0).CODE(),
					MAKE_PRINT_UNFOCUS);

	StoreWindow(wSet,	.title, (char*) RSC(SETTINGS_TITLE).CODE());

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
	return wSet;
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
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_MENU).CODE(),
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
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_SHIFT_GR1).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_BLANK).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_SHIFT_GR2).CODE(),
					MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_MOVE_WINDOW).CODE(),
					MAKE_PRINT_UNFOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, RSC(HELP_KEY_ALT_GR3).CODE(),
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

	StoreWindow(wHelp,	.title, (char*) RSC(HELP_TITLE).CODE());
	StoreWindow(wHelp,	.color[0].select, MAKE_PRINT_UNFOCUS);
	StoreWindow(wHelp,	.color[1].select, MAKE_PRINT_FOCUS);

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
	return wHelp;
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
	{1, RSC(ADV_HELP_ITEM_RST).CODE(),	{SCANKEY_SHIFT_n}},
	{1, RSC(ADV_HELP_ITEM_SEL).CODE(),	{SCANKEY_n}	},
	{1, RSC(ADV_HELP_ITEM_REV).CODE(),	{SCANKEY_r}	},
	{1, RSC(ADV_HELP_ITEM_HIDE).CODE(),	{SCANKEY_v}	},
	{0, RSC(CREATE_ADV_HELP_COND0).CODE(),	{SCANKEY_NULL}	},
	{0, RSC(ADV_HELP_SECT_ANY).CODE(),	{SCANKEY_NULL}	},
	{1, RSC(ADV_HELP_ITEM_POWER).CODE(),	{SCANKEY_DOLLAR}},
	{1, RSC(ADV_HELP_ITEM_TOP).CODE(),	{SCANKEY_DOT}	},
	{1, RSC(ADV_HELP_ITEM_FAHR_CELS).CODE(),{SCANKEY_SHIFT_f}},
	{1, RSC(ADV_HELP_ITEM_SYSGATE).CODE(),	{SCANKEY_SHIFT_g}},
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
	StoreWindow(wHelp,	.title, (char*) RSC(ADV_HELP_TITLE).CODE());

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
	return wHelp;
}

Window *CreateAbout(unsigned long long id)
{
	ASCII *C[] = {
		RSC(CF0).CODE(),
		RSC(CF1).CODE(),
		RSC(CF2).CODE(),
		RSC(CF3).CODE(),
		RSC(CF4).CODE(),
		RSC(CF5).CODE()
	} , *F[] = {
		RSC(COPY0).CODE(),
		RSC(COPY1).CODE(),
		RSC(COPY2).CODE()
	};
	size_t	c = sizeof(C) / sizeof(C[0]),
		f = sizeof(F) / sizeof(F[0]),
		v = strlen(COREFREQ_VERSION);
	CUINT	cHeight = c + f,
		oCol = (Draw.Size.width - RSZ(CF0)) / 2,
		oRow = TOP_HEADER_ROW + 4;

	if (cHeight >= (Draw.Size.height - 1)) {
		cHeight = Draw.Size.height - 2;
	}
	if (oRow + cHeight >= Draw.Size.height) {
		oRow = abs(Draw.Size.height - (1 + cHeight));
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

		StoreWindow(wAbout, .title, (char *)RSC(COREFREQ_TITLE).CODE());

		StoreWindow(wAbout,	.color[0].select, MAKE_PRINT_UNFOCUS);
		StoreWindow(wAbout,	.color[1].select, MAKE_PRINT_FOCUS);

		StoreWindow(wAbout,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wAbout,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wAbout,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wAbout,	.key.WinUp,	MotionOriginUp_Win);
	}
	return wAbout;
}

/* >>> GLOBALS >>> */
int SysInfoCellPadding;
/* <<< GLOBALS <<< */

TGrid *AddSysInfoCell(CELL_ARGS)
{
	SysInfoCellPadding++;
	return StoreTCell(win, key, item, attrib);
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
		matrixSize.hth = 14;
		winOrigin.row = TOP_HEADER_ROW + 5;
		winWidth = 60;
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
		matrixSize.hth = 28;
		winOrigin.row = TOP_HEADER_ROW + 2;
		winWidth = 60;
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
		CUINT height = 16
			+ ( RO(Shm)->SysGate.OS.IdleDriver.stateCount > 0 ) * 5;
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
		    if (Draw.SmbIndex >= matrixSize.hth) {
			wSysInfo->matrix.scroll.vert = 1
					+ (Draw.SmbIndex - matrixSize.hth);

			wSysInfo->matrix.select.row = matrixSize.hth - 1;
		    } else {
			wSysInfo->matrix.scroll.vert = 0;
			wSysInfo->matrix.select.row = Draw.SmbIndex;
		    }
			fallthrough;
		default:
			StoreWindow(wSysInfo,	.key.Enter, MotionEnter_Cell);
			break;
		}
		if (title != NULL) {
			StoreWindow(wSysInfo,	.title, (char*) title);
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
	return wSysInfo;
}

TGrid *AddCell(CELL_ARGS)
{
	return StoreTCell(win, key, item, attrib);
}

Window *CreateTopology(unsigned long long id)
{
	Window *wTopology = CreateWindow(wLayer, id,
					TOPO_MATY, 2+RO(Shm)->Proc.CPU.Count,
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
	return wTopology;
}

Window *CreateISA(unsigned long long id)
{
	Window *wISA = CreateWindow(wLayer, id, 4, 14, 6, TOP_HEADER_ROW+2);

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
	return wISA;
}

Window *CreateSysRegs(unsigned long long id)
{
	Window *wSR = CreateWindow(	wLayer, id,
					17, (2*(1+RO(Shm)->Proc.CPU.Count)),
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
	return wSR;
}

Window *CreateMemCtrl(unsigned long long id)
{
	Window *wIMC;
	TIMING_FUNC pTimingFunc = Timing_DDR3;
	unsigned int	mc, rows = 1 + (RO(Shm)->Uncore.CtrlCount > 1),
			ctrlHeaders = 6, channelHeaders = 4;

	switch (RO(Shm)->Uncore.Unit.DDR_Ver) {
	case 5:
	case 4:
	    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	    {
		ctrlHeaders = 7;
		channelHeaders = 5;
		pTimingFunc = Timing_DDR4;
	    }
	    else if ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		  ||  (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
	    {
		ctrlHeaders = 8;
		channelHeaders = 6;
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
   for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++) {
	rows += ctrlHeaders * (RO(Shm)->Uncore.MC[mc].ChannelCount > 0);
	rows += channelHeaders * RO(Shm)->Uncore.MC[mc].ChannelCount;
	rows += RO(Shm)->Uncore.MC[mc].SlotCount
		* RO(Shm)->Uncore.MC[mc].ChannelCount;
    }
	wIMC = CreateWindow(wLayer, id, MC_MATX, rows, 4, TOP_HEADER_ROW + 2);
    if (wIMC != NULL)
    {
	MemoryController(wIMC, AddCell, pTimingFunc);

	wIMC->matrix.select.col = 2;
	wIMC->matrix.select.row = 1;

	StoreWindow(wIMC,	.title, (char*) RSC(MEM_CTRL_TITLE).CODE());

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
	return wIMC;
}

Window *CreateSortByField(unsigned long long id)
{
	const CUINT oRow = TOP_HEADER_ROW
	#ifndef NO_UPPER
			+ Draw.Area.MaxRows
			+ 2;
	#else
			+ 1;
	#endif
	Window *wSortBy = CreateWindow( wLayer, id,
					1, SORTBYCOUNT,
					33, oRow,
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

		wSortBy->matrix.select.row = RO(Shm)->SysGate.sortByField;

		StoreWindow(wSortBy,	.color[0].select, MAKE_PRINT_DROP);
		StoreWindow(wSortBy,	.color[1].select,
					RSC(UI).ATTR()[UI_WIN_MENU_SELECT]);

		StoreWindow(wSortBy,	.color[0].title, MAKE_PRINT_DROP);
		StoreWindow(wSortBy,	.color[1].title,
				RSC(UI).ATTR()[UI_WIN_SORT_BY_FIELD_TITLE]);

		StoreWindow(wSortBy,	.Print,		ForEachCellPrint_Drop);

		StoreWindow(wSortBy,	.key.Enter,	MotionEnter_Cell);
		StoreWindow(wSortBy,	.key.Down,	MotionDown_Win);
		StoreWindow(wSortBy,	.key.Up,	MotionUp_Win);
		StoreWindow(wSortBy,	.key.Home,	MotionReset_Win);
		StoreWindow(wSortBy,	.key.End,	MotionEnd_Cell);
	}
	return wSortBy;
}

int SortTaskListByForest(const void *p1, const void *p2)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;

	if (task1->ppid < task2->ppid) {
		return -1;
	} else if (task1->ppid > task2->ppid) {
		return 1;
	} else if (task1->tgid < task2->tgid) {
		return -1;
	} else if (task1->tgid > task2->tgid) {
		return 1;
	} else if (task1->pid < task2->pid) {
		return -1;
	} else if (task1->pid > task2->pid) {
		return 1;
	} else {
		return 1;
	}
}

void UpdateTracker(TGrid *grid, DATA_TYPE data[])
{
	signed int *tracked = &data[0].sint[1];
  if ((*tracked) == 1)
  {
	const pid_t pid = data[0].sint[0];
	signed int idx;
    for (idx = 0; idx < RO(Shm)->SysGate.taskCount; idx++)
    {
      if (((*tracked) = (RO(Shm)->SysGate.taskList[idx].pid == pid)) == 1)
      {
	double runtime, usertime, systime;
	/*
	 * Source: kernel/sched/cputime.c
	 * If either stime or utime are 0, assume all runtime is userspace.
	 */
	runtime = RO(Shm)->SysGate.taskList[idx].runtime;
	systime = RO(Shm)->SysGate.taskList[idx].systime;
	usertime= RO(Shm)->SysGate.taskList[idx].usertime;

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
	size_t len;
	StrLenFormat(len, Buffer, 20+20+5+4+6+1,
			"User:%3.0f%%   Sys:%3.0f%% ",
			usertime, systime);

	memcpy( &grid->cell.item[grid->cell.length - len], Buffer, len);
	break;
      }
    }
    if (!(idx < RO(Shm)->SysGate.taskCount)) {
	memcpy(grid->cell.item, hSpace, grid->cell.length);
    }
  }
}

Window *CreateTracking(unsigned long long id)
{
	Window *wTrack = NULL;
	const size_t tc = (size_t) RO(Shm)->SysGate.taskCount;

  if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1) && (tc > 0))
  {
	TASK_MCB *trackList;
	trackList = malloc(tc * sizeof(TASK_MCB));
    if (trackList != NULL)
    {
	memcpy(trackList, RO(Shm)->SysGate.taskList, tc * sizeof(TASK_MCB));
	qsort(trackList, tc, sizeof(TASK_MCB), SortTaskListByForest);

	const CUINT margin = 12;	/*	"--- Freq(MHz"		*/
	const CUINT height = TOP_SEPARATOR
		+ (Draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER));
	const CUINT width = (Draw.Size.width - margin) / 2;
	const int padding = width - TASK_COMM_LEN - (7 + 2);

	wTrack = CreateWindow( wLayer, id,
				2, height,
				margin, TOP_HEADER_ROW,
				WINFLAG_NO_STOCK
				| WINFLAG_NO_BORDER );
	if (wTrack != NULL)
	{
		size_t ti;
		ssize_t si = 0;
		signed int qi = 0;
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
		StrFormat(Buffer, MAX_WIDTH-1,
			"%.*s" "%-16s" "%.*s" "(%7d)",
			qi,
			hSpace,
			trackList[ti].comm,
			padding - qi,
			hSpace,
			trackList[ti].pid);

		StoreTCell(wTrack,
			(TRACK_TASK | (unsigned long long) trackList[ti].pid),
			Buffer,
			(trackList[ti].pid == trackList[ti].tgid) ?
			  RSC(UI).ATTR()[UI_WIN_TRACKING_PARENT_PROCESS]
			: RSC(UI).ATTR()[UI_WIN_TRACKING_CHILD_PROCESS]);

		StrFormat(Buffer, MAX_WIDTH-1, "%.*s", width, hSpace);

		DATA_TYPE data = {
			.sint = { [0] = (pid_t) trackList[ti].pid, [1] = 1 }
		};
		GridCall(StoreTCell(wTrack, SCANKEY_NULL, Buffer,
				RSC(UI).ATTR()[UI_WIN_TRACKING_COUNTERS]),
			UpdateTracker, data);
	    }
		StoreWindow(	wTrack, .color[0].select,
				RSC(UI).ATTR()[UI_WIN_TRACKING_COUNTERS] );

		StoreWindow(	wTrack, .color[1].select,
				RSC(UI).ATTR()[UI_MAKE_SELECT_FOCUS] );

		StoreWindow(	wTrack, .color[0].title,
				MAKE_PRINT_DROP );

		StoreWindow(	wTrack, .color[1].title,
				RSC(UI).ATTR()[UI_WIN_TRACKING_TITLE] );

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
	return wTrack;
}

void UpdateHotPlugCPU(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;

	if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
	{
	    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
	    {
		StrFormat(Buffer, 9+10+1,
			RSC(CREATE_HOTPLUG_CPU_OFFLINE).CODE(), cpu);

		memcpy(grid->cell.item, Buffer, grid->cell.length);
		memcpy(grid->cell.attr, RSC(CREATE_HOTPLUG_CPU_OFFLINE).ATTR(),
					grid->cell.length);

		grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
	    }
	}
	else
	{
	    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
	    {
		StrFormat(Buffer, 9+10+1,
			RSC(CREATE_HOTPLUG_CPU_ONLINE).CODE(), cpu);

		memcpy(grid->cell.item, Buffer, grid->cell.length);
		memcpy(grid->cell.attr, RSC(CREATE_HOTPLUG_CPU_ONLINE).ATTR(),
					grid->cell.length);

		grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
	    }
	}
}

void UpdateHotPlugTrigger(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;

	if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
	{
	    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
	    {
		memcpy( grid->cell.item,
			RSC(CREATE_HOTPLUG_CPU_ENABLE).CODE(),
		(size_t)RSZ(CREATE_HOTPLUG_CPU_ENABLE) );

		memcpy( grid->cell.attr,
			RSC(CREATE_HOTPLUG_CPU_ENABLE).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = (unsigned long long) (CPU_ONLINE | cpu);
		grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
	    }
	}
	else
	{
	    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
	    {
		memcpy( grid->cell.item,
			RSC(CREATE_HOTPLUG_CPU_DISABLE).CODE(),
		(size_t)RSZ(CREATE_HOTPLUG_CPU_DISABLE) );

		memcpy( grid->cell.attr,
			RSC(CREATE_HOTPLUG_CPU_DISABLE).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = (unsigned long long) (CPU_OFFLINE | cpu);
		grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
	    }
	}
}

Window *CreateHotPlugCPU(unsigned long long id)
{
	Window *wCPU = CreateWindow(	wLayer, id, 2, Draw.Area.MaxRows,
					LOAD_LEAD + 1, TOP_HEADER_ROW + 1,
					WINFLAG_NO_STOCK );
  if (wCPU != NULL)
  {
	unsigned int cpu;
    for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++)
    {
      if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
      {
	StrFormat(Buffer, 9+10+1, RSC(CREATE_HOTPLUG_CPU_OFFLINE).CODE(), cpu);

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
	StrFormat(Buffer, 9+10+1, RSC(CREATE_HOTPLUG_CPU_ONLINE).CODE(), cpu);

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

	StoreWindow(wCPU, .title, (char *)RSC(CREATE_HOTPLUG_CPU_TITLE).CODE());

	StoreWindow(wCPU,	.color[1].title,
				RSC(UI).ATTR()[UI_WIN_HOT_PLUG_CPU_TITLE]);

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
	return wCPU;
}

void UpdateRatioClock(TGrid *grid, DATA_TYPE data[])
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
		];
	if (data[0].uint[0] > MAXCLOCK_TO_RATIO(unsigned int, CFlop->Clock.Hz))
	{
		StrFormat(Buffer, 1+6+1, " %-6s", RSC(NOT_AVAILABLE).CODE());
	} else {
		StrFormat(Buffer, 8+1, "%7.2f",
			ABS_FREQ_MHz(double, data[0].uint[0], CFlop->Clock));
	}
	memcpy(&grid->cell.item[1], Buffer, 7);
}

void TitleForTurboClock(unsigned int NC, ASCII *title)
{
	StrFormat(title, 15+11+1, RSC(TURBO_CLOCK_TITLE).CODE(), NC);
}

void TitleForRatioClock(unsigned int NC, ASCII *title)
{
	StrFormat(title, 14+6+1, RSC(RATIO_CLOCK_TITLE).CODE(),
		(NC == CLOCK_MOD_TGT) || (NC == CLOCK_MOD_HWP_TGT) ?
			RSC(TARGET).CODE() :
		(NC == CLOCK_MOD_MAX) || (NC == CLOCK_MOD_HWP_MAX) ?
			RSC(MAX).CODE() :
		(NC == CLOCK_MOD_MIN) || (NC == CLOCK_MOD_HWP_MIN) ?
			RSC(MIN).CODE() :
		(NC == CLOCK_MOD_ACT) ?
			RSC(ACT).CODE() : (ASCII*) "");
}

void TitleForUncoreClock(unsigned int NC, ASCII *title)
{
	StrFormat(title, 15+3+1, RSC(UNCORE_CLOCK_TITLE).CODE(),
			(NC == CLOCK_MOD_MAX) ? RSC(MAX).CODE() :
			(NC == CLOCK_MOD_MIN) ? RSC(MIN).CODE() : (ASCII*) "");
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
	(*lowestShift)	= (int) COF - (int) minRatio;
	(*highestShift) = (int) maxRatio - (int) COF;
}

unsigned int MultiplierIsRatio(unsigned int cpu, unsigned int multiplier)
{
	enum RATIO_BOOST boost;
	for (boost = BOOST(MIN); boost < BOOST(SIZE); boost++)
	{
		switch (boost) {
		case BOOST(CPB):
		case BOOST(XFR):
		    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		     || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
		    {
			break;
		    }
			fallthrough;
		default:
		    if (RO(Shm)->Cpu[cpu].Boost[boost] == multiplier)
		    {
			return 1;
		    }
			break;
		}
	}
	if (RO(Shm)->Proc.Features.Factory.Ratio == multiplier) {
		return 1;
	}
	return 0;
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
	struct FLIP_FLOP *CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[
						!RO(Shm)->Cpu[cpu].Toggle
				];
	ATTRIBUTE *attrib[7] = {
		RSC(CREATE_RATIO_CLOCK_COND0).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND1).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND2).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND3).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND4).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND5).ATTR(),
		RSC(CREATE_RATIO_CLOCK_COND6).ATTR()
	};
	CLOCK_ARG clockMod = {.ullong = id};

	const CUINT hthMax = 1 + lowestShift + highestShift;
	CUINT	oRow = CUMAX(1 + TOP_HEADER_ROW + TOP_FOOTER_ROW, 2);
	CUINT hthWin = CUMIN(Draw.Size.height - oRow, hthMax);
	if (hthWin < oRow) {
		oRow = 1;
		hthWin = Draw.Size.height - 2;
	}
	Window *wCK = CreateWindow(	wLayer, id,
					1, hthWin,
					oCol,
					oRow,
					WINFLAG_NO_STOCK );
    if (wCK != NULL)
    {
	signed int multiplier, offset;
	for (offset = -lowestShift; offset <= highestShift; offset++)
	{
		ATTRIBUTE *attr = attrib[3];

		multiplier = (int) COF + offset;

		clockMod.NC = NC | boxKey;
		clockMod.cpu = any;

	    if (any == -1) {
		clockMod.Ratio	= multiplier;
	    } else {
		clockMod.Offset = offset;
	    }

	  if (multiplier == 0) {
		StrFormat(Buffer, RSZ(AUTOMATIC)+17+11+11+1,
			"    %s       <%4d >  %+4d ",
			RSC(AUTOMATIC).CODE(), multiplier, offset);

		StoreTCell(wCK, clockMod.ullong, Buffer, attr);
	  } else {
	    if (multiplier > MAXCLOCK_TO_RATIO(signed int, CFlop->Clock.Hz))
	    {
		StrFormat(Buffer, 15+6+11+11+1,
			"  %-6s MHz   <%4d >  %+4d ",
			RSC(NOT_AVAILABLE).CODE(), multiplier, offset);
	    } else {
		const unsigned int _multiplier = (unsigned int)multiplier;
		attr = attrib[
			multiplier < medianColdZone ?
				1 + 4 * MultiplierIsRatio(cpu, _multiplier)
			: multiplier >= startingHotZone ?
				2 + 4 * MultiplierIsRatio(cpu, _multiplier)
				: 0 + 4 * MultiplierIsRatio(cpu, _multiplier)
			];
		StrFormat(Buffer, 14+8+11+11+1,
			" %7.2f MHz   <%4d >  %+4d ",
			ABS_FREQ_MHz(double, _multiplier, CFlop->Clock),
			multiplier, offset);
	    }
		GridCall(StoreTCell(wCK, clockMod.ullong, Buffer, attr),
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
	return wCK;
}

void UpdateRoomSchedule(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	const signed int pos = grid->cell.length - 8;

	if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
	{
	    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
	    {
		memcpy( grid->cell.attr,
			RSC(CREATE_SELECT_CPU_COND0).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = SCANKEY_NULL;
		grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
	    }
	}
	else
	{
	    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
	    {
		memcpy( grid->cell.attr,
			RSC(CREATE_SELECT_CPU_COND1).ATTR(),
			grid->cell.length );

		grid->cell.quick.key = (unsigned long long) (CPU_SELECT | cpu);
		grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
	    }
	}

	const unsigned int run = BITVAL_CC(RO(Shm)->roomSched, cpu);
	unsigned int sched = BITVAL(data[0].ullong, 63);

	if (run != sched)
	{
		memcpy( &grid->cell.item[pos],
			ENABLED(run), __builtin_strlen(ENABLED(0)) );

		if (run) {
			BITSET(LOCKLESS, grid->data[0].ullong, 63);
		} else {
			BITCLR(LOCKLESS, grid->data[0].ullong, 63);
		}
	    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
	    {
		sched = BITVAL(grid->data[0].ullong, 63);
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
			+ (Draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER));
	const CUINT height = CUMIN(RO(Shm)->Proc.CPU.Count, _height);
	const CUINT column = Draw.Size.width <= 35 ? 1 : Draw.Size.width - 35;

	Window *wUSR = CreateWindow(	wLayer, id,
					1, height,
					column, TOP_HEADER_ROW + 1,
					WINFLAG_NO_STOCK );
    if (wUSR != NULL)
    {
	unsigned int cpu, bix;
	for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++)
	{
		bix = BITVAL_CC(RO(Shm)->roomSched, cpu);
		StrFormat(Buffer, 17+10+11+11+11+1,
				"  %03u  %4d%6d%6d    <%3s>    ",
				cpu,
				RO(Shm)->Cpu[cpu].Topology.PackageID,
				RO(Shm)->Cpu[cpu].Topology.CoreID,
				RO(Shm)->Cpu[cpu].Topology.ThreadID,
				ENABLED(bix));

	    if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
	    {
		unsigned long long
			data = BITVAL_CC(RO(Shm)->roomSched, cpu);
			data = data << 63;
			data = data | (CPU_OFFLINE | cpu);

		GridCall( StoreTCell(	wUSR, SCANKEY_NULL, Buffer,
					RSC(CREATE_SELECT_CPU_COND0).ATTR() ),
			UpdateRoomSchedule, data );
	    }
	  else
	    {
		unsigned long long
			data = BITVAL_CC(RO(Shm)->roomSched, cpu);
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
	return wUSR;
}

void Pkg_Fmt_Turbo(ASCII *item, CLOCK *clock, unsigned int ratio, char *NC)
{
    if (ratio == 0) {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
			(char *) RSC(CREATE_SELECT_AUTO_TURBO).CODE(),
			NC, RSC(AUTOMATIC).CODE(),
			RO(Shm)->Proc.Features.Turbo_Unlock ? '<' : '[',
			ratio,
			RO(Shm)->Proc.Features.Turbo_Unlock ? '>' : ']');
    } else {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1,
			(char *) RSC(CREATE_SELECT_FREQ_TURBO).CODE(),
			NC, ABS_FREQ_MHz(double, ratio, (*clock)),
			RO(Shm)->Proc.Features.Turbo_Unlock ? '<' : '[',
			ratio,
			RO(Shm)->Proc.Features.Turbo_Unlock ? '>' : ']');
    }
}

void Pkg_Fmt_Freq(	ASCII *item, ASCII *code, CLOCK *clock,
			unsigned int ratio, unsigned char unlock )
{
    if (ratio == 0) {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
			"%s" "   %s     %c%4u %c ",
			code, RSC(AUTOMATIC).CODE(),
			unlock ? '<' : '[', ratio, unlock ? '>' : ']');
    } else {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1,
			"%s" "%7.2f MHz %c%4u %c ",
			code, ABS_FREQ_MHz(double, ratio, (*clock)),
			unlock ? '<' : '[', ratio, unlock ? '>' : ']');
    }
}

void CPU_Item_Auto_Freq(unsigned int cpu, unsigned int ratio,
			unsigned char unlock, ASCII *item)
{
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+10+1,
			"  %03u  %4d%6d%6d   " "   %s     %c%4u %c ",
			cpu,
			RO(Shm)->Cpu[cpu].Topology.PackageID,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID,
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
			&RO(Shm)->Cpu[top].FlipFlop[			\
					!RO(Shm)->Cpu[top].Toggle	\
					].Clock,			\
			RO(Shm)->Cpu[top].Boost[BOOST(_NC)],		\
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
void Pkg_Update_Turbo_##_NC(TGrid *grid, DATA_TYPE data[])		\
{									\
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];		\
	unsigned int top = Ruler.Top[BOOST(_NC)];			\
	UNUSED(data);							\
									\
	Pkg_Fmt_Turbo(	item,						\
			&RO(Shm)->Cpu[top].FlipFlop[			\
					!RO(Shm)->Cpu[top].Toggle	\
					].Clock,			\
			RO(Shm)->Cpu[top].Boost[BOOST(_NC)],		\
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
    if (RO(Shm)->Cpu[cpu].Boost[BOOST(_NC)] == 0) { 			\
	CPU_Item_Auto_Freq(cpu, RO(Shm)->Cpu[cpu].Boost[BOOST(_NC)],	\
			   RO(Shm)->Proc.Features.Turbo_Unlock, item);	\
    } else {								\
	struct FLIP_FLOP *CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[		\
					!RO(Shm)->Cpu[cpu].Toggle	\
				];					\
	StrFormat(item ,						\
		RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,	\
		"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",		\
		cpu,							\
		RO(Shm)->Cpu[cpu].Topology.PackageID,			\
		RO(Shm)->Cpu[cpu].Topology.CoreID,			\
		RO(Shm)->Cpu[cpu].Topology.ThreadID,			\
		ABS_FREQ_MHz(double, RO(Shm)->Cpu[cpu].Boost[BOOST(_NC)],\
				     CFlop->Clock),			\
		RO(Shm)->Proc.Features.Turbo_Unlock ? '<' : '[',	\
		RO(Shm)->Cpu[cpu].Boost[BOOST(_NC)],			\
		RO(Shm)->Proc.Features.Turbo_Unlock ? '>' : ']');	\
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
void CPU_Update_Turbo_##_NC(TGrid *grid, DATA_TYPE data[])		\
{									\
	const unsigned int cpu = data[0].ullong & CPU_MASK;		\
	ASCII item[ RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1 ];	\
  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))				\
  {									\
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)		\
    {									\
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,	\
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);	\
									\
	memcpy(grid->cell.item, item, grid->cell.length);		\
									\
	memcpy( grid->cell.attr,					\
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),			\
		grid->cell.length );					\
									\
	grid->cell.quick.key = SCANKEY_NULL;				\
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);\
    }									\
  }									\
  else									\
  {									\
	CPU_Item_Turbo_##_NC(cpu, item);				\
	memcpy(grid->cell.item, item, grid->cell.length);		\
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)		\
    {									\
	memcpy( grid->cell.attr,					\
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),			\
		grid->cell.length );					\
									\
	grid->cell.quick.key = BOXKEY_TURBO_CLOCK_##_NC | (cpu ^ CORE_COUNT);\
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu); \
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
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(TGT)],
			RO(Shm)->Proc.Features.TgtRatio_Unlock );
}

void Pkg_Target_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(TGT)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_TGT).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(TGT)],
			RO(Shm)->Proc.Features.TgtRatio_Unlock );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_Target_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[ !RO(Shm)->Cpu[cpu].Toggle ];

    if (RO(Shm)->Cpu[cpu].Boost[BOOST(TGT)] == 0) {
	CPU_Item_Auto_Freq(	cpu, RO(Shm)->Cpu[cpu].Boost[BOOST(TGT)],
				RO(Shm)->Proc.Features.TgtRatio_Unlock, item );
    } else {
	StrFormat(item,
		RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+4+6+10+1,
		"  %03u  %4d%6d  %-3d" "[%3.0f ]%5.0f MHz %c%4u %c ",
		cpu,
		RO(Shm)->Cpu[cpu].Topology.PackageID,
		RO(Shm)->Cpu[cpu].Topology.CoreID,
		RO(Shm)->Cpu[cpu].Topology.ThreadID,
		CFlop->Absolute.Ratio.Perf,
		CFlop->Absolute.Freq,
		RO(Shm)->Proc.Features.TgtRatio_Unlock ? '<' : '[',
		RO(Shm)->Cpu[cpu].Boost[BOOST(TGT)],
		RO(Shm)->Proc.Features.TgtRatio_Unlock ? '>' : ']');
    }
}

void CPU_Target_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
  {
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_Target_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_TGT | (cpu ^ CORE_COUNT);
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_HWP_Target_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(HWP_TGT)];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_TGT).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(HWP_TGT)],
			isEnable );
}

void Pkg_HWP_Target_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(HWP_TGT)];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_TGT).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(HWP_TGT)],
			isEnable );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_HWP_Target_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[ !RO(Shm)->Cpu[cpu].Toggle ];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

    if (RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf == 0) {
	CPU_Item_Auto_Freq(	cpu,
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf,
				isEnable, item );
    } else {
	StrFormat(item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			RO(Shm)->Cpu[cpu].Topology.PackageID,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(double,
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf,
			CFlop->Clock
			),
			isEnable ? '<' : '[',
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf,
			isEnable ? '>' : ']');
    }
}

void CPU_HWP_Target_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
  {
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_HWP_Target_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_HWP_TGT | (cpu ^ CORE_COUNT);
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_HWP_Max_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(HWP_MAX)];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MAX).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(HWP_MAX)],
			isEnable );
}

void Pkg_HWP_Max_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(HWP_MAX)];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MAX).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(HWP_MAX)],
			isEnable );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_HWP_Max_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[ !RO(Shm)->Cpu[cpu].Toggle ];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

    if (RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf == 0) {
	CPU_Item_Auto_Freq(	cpu,
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf,
				isEnable, item );
    } else {
	StrFormat(item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			RO(Shm)->Cpu[cpu].Topology.PackageID,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(double,
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf,
			CFlop->Clock
			),
			isEnable ? '<' : '[',
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf,
			isEnable ? '>' : ']');
    }
}

void CPU_HWP_Max_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
  {
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_HWP_Max_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_HWP_MAX | (cpu ^ CORE_COUNT);
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_HWP_Min_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(HWP_MIN)];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MIN).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(HWP_MIN)],
			isEnable );
}

void Pkg_HWP_Min_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(HWP_MIN)];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_HWP_MIN).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(HWP_MIN)],
			isEnable );

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_HWP_Min_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[ !RO(Shm)->Cpu[cpu].Toggle ];
	const unsigned int isEnable = (RO(Shm)->Proc.Features.HWP_Enable == 1)
				|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1);

    if (RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf == 0) {
	CPU_Item_Auto_Freq(	cpu,
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf,
				isEnable, item );
    } else {
	StrFormat(item,
			RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
			"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
			cpu,
			RO(Shm)->Cpu[cpu].Topology.PackageID,
			RO(Shm)->Cpu[cpu].Topology.CoreID,
			RO(Shm)->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(double,
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf,
			CFlop->Clock
			),
			isEnable ? '<' : '[',
			RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf,
			isEnable ? '>' : ']');
    }
}

void CPU_HWP_Min_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
  {
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_HWP_Min_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_HWP_MIN | (cpu ^ CORE_COUNT);
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_Max_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(MAX)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MAX).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(MAX)],
		       (RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10);
}

void Pkg_Max_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(MAX)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MAX).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(MAX)],
		       (RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10);

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_Max_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[ !RO(Shm)->Cpu[cpu].Toggle ];

    if (RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)] == 0) {
	CPU_Item_Auto_Freq(	cpu, RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)],
			(RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10,
				item );
    } else {
	StrFormat(item,
		RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
		"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
		cpu,
		RO(Shm)->Cpu[cpu].Topology.PackageID,
		RO(Shm)->Cpu[cpu].Topology.CoreID,
		RO(Shm)->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(	double, RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)],
					CFlop->Clock ),
		(RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10 ?
			'<' : '[',
		RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)],
		(RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10 ?
			'>' : ']');
    }
}

void CPU_Max_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
  {
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_Max_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_MAX | (cpu ^ CORE_COUNT);
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

void Pkg_Item_Min_Freq(ASCII *item)
{
	unsigned int top = Ruler.Top[BOOST(MIN)];

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MIN).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(MIN)],
		       (RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01);
}

void Pkg_Min_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+9+10+1];
	unsigned int top = Ruler.Top[BOOST(MIN)];
	UNUSED(data);

	Pkg_Fmt_Freq(	item, RSC(CREATE_SELECT_FREQ_MIN).CODE(),
			&RO(Shm)->Cpu[top].FlipFlop[
				!RO(Shm)->Cpu[top].Toggle
			].Clock,
			RO(Shm)->Cpu[top].Boost[BOOST(MIN)],
		       (RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01);

	memcpy(grid->cell.item, item, grid->cell.length);
}

void CPU_Item_Min_Freq(unsigned int cpu, ASCII *item)
{
	struct FLIP_FLOP *CFlop;
	CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[ !RO(Shm)->Cpu[cpu].Toggle ];

    if (RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)] == 0) {
	CPU_Item_Auto_Freq(	cpu, RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)],
			(RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01,
				item );
    } else {
	StrFormat(item,
		RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+10+1,
		"  %03u  %4d%6d%6d   " "%7.2f MHz %c%4u %c ",
		cpu,
		RO(Shm)->Cpu[cpu].Topology.PackageID,
		RO(Shm)->Cpu[cpu].Topology.CoreID,
		RO(Shm)->Cpu[cpu].Topology.ThreadID,
		ABS_FREQ_MHz(	double, RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)],
					CFlop->Clock ),
		(RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01 ?
			'<' : '[',
		RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)],
		(RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01 ?
			'>' : ']');
    }
}

void CPU_Min_Freq_Update(TGrid *grid, DATA_TYPE data[])
{
	const unsigned int cpu = data[0].ullong & CPU_MASK;
	ASCII item[RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+11+11+11+8+1];

  if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
  {
    if ((data[0].ullong & CPU_STATE_MASK) == CPU_ONLINE)
    {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

	memcpy(grid->cell.item, item, grid->cell.length);

	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND1).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = SCANKEY_NULL;
	grid->data[0].ullong = (unsigned long long) (CPU_OFFLINE | cpu);
    }
  }
  else
  {
	CPU_Item_Min_Freq(cpu, item);

	memcpy(grid->cell.item, item, grid->cell.length);

    if ((data[0].ullong & CPU_STATE_MASK) == CPU_OFFLINE)
    {
	memcpy( grid->cell.attr,
		RSC(CREATE_SELECT_FREQ_COND0).ATTR(),
		grid->cell.length );

	grid->cell.quick.key = BOXKEY_RATIO_CLOCK_MIN | (cpu ^ CORE_COUNT);
	grid->data[0].ullong = (unsigned long long) (CPU_ONLINE | cpu);
    }
  }
}

Window *CreateSelectFreq(unsigned long long id,
			PKG_ITEM_CALLBACK Pkg_Item_Callback,
			CPU_ITEM_CALLBACK CPU_Item_Callback,
			UPDATE_CALLBACK Pkg_Freq_Update,
			UPDATE_CALLBACK CPU_Freq_Update)
{
	const CUINT _height = (Draw.Area.MaxRows << (ADD_UPPER & ADD_LOWER))
			#ifndef NO_FOOTER
				+ (ADD_UPPER | ADD_LOWER)
			#endif
		, height = CUMIN( (RO(Shm)->Proc.CPU.Count + 1), _height );

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

    for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++)
    {
      if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
      {
	StrFormat(item, RSZ(CREATE_SELECT_FREQ_OFFLINE)+10+1,
			RSC(CREATE_SELECT_FREQ_OFFLINE).CODE(), cpu);

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
	return wFreq;
}

Window *CreateSelectIdle(unsigned long long id)
{
	Window *wIdle = CreateWindow(wLayer, id,
			1, 1 + RO(Shm)->SysGate.OS.IdleDriver.stateCount,
			(Draw.Size.width - (2 + RSZ(BOX_IDLE_LIMIT_TITLE))) / 2,
			TOP_HEADER_ROW + 2,
			WINFLAG_NO_STOCK);

    if (wIdle != NULL)
    {
	signed int idx;

	StoreTCell(wIdle, BOXKEY_LIMIT_IDLE_ST00,
			RSC(BOX_IDLE_LIMIT_RESET).CODE(),
			RSC(UI).ATTR()[UI_WIN_SELECT_IDLE_RESET]);

	for (idx = 0; idx < RO(Shm)->SysGate.OS.IdleDriver.stateCount; idx++)
	{
		StrFormat(Buffer, 12+11+10+1,
			"           %2d%10.*s ", 1 + idx,
			10, RO(Shm)->SysGate.OS.IdleDriver.State[idx].Name);

		StoreTCell(wIdle, (BOXKEY_LIMIT_IDLE_ST00
				| ((1 + (unsigned long long)idx) << 4)),
				Buffer,
				RSC(UI).ATTR()[UI_WIN_SELECT_IDLE_POLL]);
	}
	StoreWindow(wIdle, .title, (char*) RSC(BOX_IDLE_LIMIT_TITLE).CODE());

	wIdle->matrix.select.row  = RO(Shm)->SysGate.OS.IdleDriver.stateLimit;
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[ 8] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[ 9] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[10] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[11] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[12] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[13] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[14] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[15] =		\
	TCellAt(wIdle, 0, wIdle->matrix.select.row).attr[16] =		\
				RSC(UI).ATTR()[UI_WIN_SELECT_IDLE_CURRENT];
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
	return wIdle;
}

#define TW_START_Z	0b000
#define TW_START_Y	0b00000
#define TW_STOP_Z	0b011
#define TW_STOP_Y	0b10010
#define TW_POST_Y	0b10011
#define TW_POST_Z	0b000

#define TW_CELL_COUNT(adj) (					\
	  (1 + (TW_STOP_Z - TW_START_Z))			\
	* (1 + (TW_STOP_Y - TW_START_Y))			\
	+ adj							\
)

#define TW_CELL_HEIGHT	(TW_CELL_COUNT(0) >> 2)
#define TW_CELL_WIDTH	18

struct TW_ST {
	double TAU;
	unsigned char TW;
};

void InsertionSortTW(	struct TW_ST base[],
			unsigned int cnt,
			enum RATIO_BOOST start )
{
	__typeof__(start) lt = start + 1, rt;
	while (lt < cnt)
	{
		rt = lt;
		while ((rt > start) && (base[rt - 1].TAU > base[rt].TAU))
		{
			__typeof__(base[0]) swap;
			swap.TAU = base[rt - 1].TAU;
			swap.TW = base[rt - 1].TW;

			base[rt - 1].TAU = base[rt].TAU;
			base[rt - 1].TW = base[rt].TW;

			base[rt].TAU = swap.TAU;
			base[rt].TW = swap.TW;

			rt = rt - 1;
		}
		lt = lt + 1;
	}
}

Window *CreatePowerTimeWindow(unsigned long long id)
{
	Window *wPTW = NULL;
	const enum PWR_DOMAIN	pw = (id >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = id & 0b11;

	struct TW_ST *array = calloc(TW_CELL_COUNT(+1), sizeof(struct TW_ST));
  if (array != NULL)
  {
	signed int idx = 0, fdx = -1;
	unsigned char Y, Z;
   for (Y = TW_START_Y; Y <= TW_STOP_Y; Y++)
     for (Z = TW_START_Z; Z <= TW_STOP_Z; Z++)
     {
	array[idx].TAU = COMPUTE_TAU(Y, Z, RO(Shm)->Proc.Power.Unit.Times);
	array[idx].TW = COMPUTE_TW(Y, Z);

	if ((fdx == -1)
	 && (array[idx].TW == RO(Shm)->Proc.Power.Domain[pw].Feature[pl].TW)) {
		fdx = idx;
	}
	idx++;
     }
    if (fdx == -1) {
	array[idx].TAU = RO(Shm)->Proc.Power.Domain[pw].TAU[pl];
	array[idx].TW = RO(Shm)->Proc.Power.Domain[pw].Feature[pl].TW;
    } else {
	array[idx].TAU = COMPUTE_TAU(	TW_POST_Y, TW_POST_Z,
					RO(Shm)->Proc.Power.Unit.Times );

	array[idx].TW = COMPUTE_TW(TW_POST_Y, TW_POST_Z);
    }

	InsertionSortTW(array, TW_CELL_COUNT(+1), 0);

	wPTW = CreateWindow(	wLayer, id,
				1, TW_CELL_HEIGHT,
				(MIN_WIDTH - TW_CELL_WIDTH) >> 1,
				TOP_HEADER_ROW + 1,
				WINFLAG_NO_VSB );
    if (wPTW != NULL)
    {
	const ASCII *labelTW[PWR_LIMIT_SIZE] = {
		RSC(POWER_LABEL_TW1).CODE(),
		RSC(POWER_LABEL_TW2).CODE()
	};
	const ASCII *labelDom[DOMAIN_SIZE] = {
		RSC(POWER_LABEL_PKG).CODE(),
		RSC(POWER_LABEL_CORE).CODE(),
		RSC(POWER_LABEL_UNCORE).CODE(),
		RSC(POWER_LABEL_DRAM).CODE(),
		RSC(POWER_LABEL_PLATFORM).CODE()
	};
	char item[3*20+5+1];

	unsigned long long us, ms, ss, bits, key;
	unsigned char circle;
     for (circle = 0, fdx = -1; circle < 2; circle++)
      for (idx = 0; idx < TW_CELL_COUNT(+1); idx++)
      {
	bits = ((unsigned long long) array[idx].TW) << 20,
	key = (BOXKEY_TW_OP | (0x8ULL << pl)) | (pw << 5) | bits;

	us = 1000LLU * 1000LLU * array[idx].TAU;
	ms = 1000LLU * array[idx].TAU;
	ss = array[idx].TAU;
	us = us % 1000LLU;
	ms = ms % 1000LLU;

       if (us) {
	StrFormat(item, 3*20+5+1, "  %7llu,%03llu%-3llu  ",ss, ms, us);
       } else if (ms) {
	StrFormat(item, 2*20+8+1, "  %7llu,%-3llu     ", ss, ms);
       } else {
	StrFormat(item, 20+11+1 , "  %7llu         ", ss);
       }
	if (array[idx].TW == RO(Shm)->Proc.Power.Domain[pw].Feature[pl].TW) {
		StoreTCell(wPTW, key, item, RSC(UI).ATTR()[UI_WHEEL_CURRENT]);
		fdx = idx;
	} else {
		StoreTCell(wPTW, key, item, RSC(UI).ATTR()[UI_WHEEL_LIST]);
	}
      }

	wPTW->matrix.select.row = wPTW->matrix.size.hth >> 1;

     if (fdx >= 0) {
	if (fdx >= wPTW->matrix.select.row) {
		wPTW->matrix.scroll.vert = fdx - wPTW->matrix.select.row;
	} else {
		wPTW->matrix.scroll.vert = (wPTW->dim >> 1)
					 - (wPTW->matrix.select.row - fdx);
	}
     } else {
	wPTW->matrix.scroll.vert = 0;
     }

	StrFormat(item, TW_CELL_WIDTH+1, " %s %s ", labelDom[pw], labelTW[pl]);
	StoreWindow(wPTW,	.title, 	item);

	StoreWindow(wPTW,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wPTW,	.key.Up,	MotionUp_Wheel);
	StoreWindow(wPTW,	.key.Down,	MotionDown_Wheel);
	StoreWindow(wPTW,	.key.PgUp,	MotionPgUp_Wheel);
	StoreWindow(wPTW,	.key.PgDw,	MotionPgDw_Wheel);
	StoreWindow(wPTW,	.key.Home,	MotionHome_Wheel);
	StoreWindow(wPTW,	.key.End,	MotionEnd_Wheel);

	StoreWindow(wPTW,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wPTW,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wPTW,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wPTW,	.key.WinUp,	MotionOriginUp_Win);

	StoreWindow(wPTW, .color[1].select, RSC(UI).ATTR()[UI_WHEEL_SELECT]);
    }
	free(array);
  }
	return wPTW;
}

#undef TW_CELL_WIDTH
#undef TW_CELL_HEIGHT
#undef TW_CELL_COUNT
#undef TW_POST_Y
#undef TW_POST_Z
#undef TW_STOP_Y
#undef TW_STOP_Z
#undef TW_START_Y
#undef TW_START_Z

#define EVENT_DOMAINS	4
#define EVENT_SECTIONS	12

void Update_Event(TGrid *grid, DATA_TYPE data[])
{
	const enum THERM_PWR_EVENTS events[eDIM] = {
		data[eLOG].ullong, data[eSTS].ullong
	};
	const ATTRIBUTE *attrib[eDIM][eDIM] = {
		{RSC(BOX_EVENT_COND00).ATTR(), RSC(BOX_EVENT_COND01).ATTR()},
		{RSC(BOX_EVENT_COND10).ATTR(), RSC(BOX_EVENT_COND11).ATTR()}
	};
	const unsigned int eTheme[eDIM] = {
		(events[eLOG] != EVENT_THERM_NONE)
	    &&	(RO(Shm)->ProcessorEvents[eLOG] & events[eLOG]),

		(events[eSTS] != EVENT_THERM_NONE)
	    &&	(RO(Shm)->ProcessorEvents[eSTS] & events[eSTS])
	};
	memcpy( grid->cell.attr,
		attrib[eTheme[eLOG]][eTheme[eSTS]],
		grid->cell.length - 1 );
}

Window *CreateEvents(unsigned long long id)
{
    struct EVENT_LDR_ST {
	SCANKEY 		quick;
	ASCII			*item;
	enum THERM_PWR_EVENTS	events[eDIM];
    } eLdr[EVENT_DOMAINS][EVENT_SECTIONS] = {
      {
	/*	Thermal Sensor						*/
	{	{BOXKEY_CLR_THM_SENSOR} , RSC(BOX_EVENT_THERMAL_SENSOR).CODE(),
		{EVENT_THERMAL_LOG	, EVENT_THERMAL_STS}		},
	/*	PROCHOT# Agent						*/
	{	{BOXKEY_CLR_PROCHOT_LOG}, RSC(BOX_EVENT_PROCHOT_STS).CODE(),
		{EVENT_PROCHOT_LOG	, EVENT_PROCHOT_STS}		},
	/*	Critical Temperature					*/
	{	{BOXKEY_CLR_THM_CRIT}	, RSC(BOX_EVENT_CRITICAL_TEMP).CODE(),
		{EVENT_CRITIC_LOG	, EVENT_CRITIC_TMP}		},
	/*	Thermal Threshold					*/
	{	{BOXKEY_CLR_THM_THOLD1} , RSC(BOX_EVENT_THOLD1_STS).CODE(),
		{EVENT_THOLD1_LOG	, EVENT_THOLD1_STS}		},
	/*	Thermal Threshold					*/
	{	{BOXKEY_CLR_THM_THOLD2} , RSC(BOX_EVENT_THOLD2_STS).CODE(),
		{EVENT_THOLD2_LOG	, EVENT_THOLD2_STS}		},
	/*	Power Limitation					*/
	{	{BOXKEY_CLR_PWR_LIMIT}	, RSC(BOX_EVENT_POWER_LIMIT).CODE(),
		{EVENT_POWER_LIMIT	, EVENT_THERM_NONE}		},
	/*	Current Limitation					*/
	{	{BOXKEY_CLR_CUR_LIMIT}	, RSC(BOX_EVENT_CURRENT_LIMIT).CODE(),
		{EVENT_CURRENT_LIMIT	, EVENT_THERM_NONE}		},
	/*	Cross Domain Limit.					*/
	{	{BOXKEY_CLR_X_DOMAIN}	, RSC(BOX_EVENT_CROSS_DOM_LIMIT).CODE(),
		{EVENT_CROSS_DOMAIN	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		}
      }, {
	/*	Thermal Sensor						*/
	{	{BOXKEY_CLR_CORE_THM}	, RSC(BOX_EVENT_THERMAL_SENSOR).CODE(),
		{EVENT_CORE_THM_LOG	, EVENT_CORE_THM_STS}		},
	/*	PROCHOT# Agent						*/
	{	{BOXKEY_CLR_CORE_HOT}	, RSC(BOX_EVENT_PROCHOT_STS).CODE(),
		{EVENT_CORE_HOT_LOG	, EVENT_CORE_HOT_STS}		},
	/*	Avg Thermal						*/
	{	{BOXKEY_CLR_CORE_AVG}	, RSC(BOX_EVENT_AVG_THERMAL).CODE(),
		{EVENT_CORE_AVG_LOG	, EVENT_CORE_AVG_STS}		},
	/*	VR Thermal						*/
	{	{BOXKEY_CLR_CORE_VRT}	, RSC(BOX_EVENT_VR_THERMAL).CODE(),
		{EVENT_CORE_VRT_LOG	, EVENT_CORE_VRT_STS}		},
	/*	VR TDC							*/
	{	{BOXKEY_CLR_CORE_TDC}	, RSC(BOX_EVENT_VR_TDC).CODE(),
		{EVENT_CORE_TDC_LOG	, EVENT_CORE_TDC_STS}		},
	/*	Package PL1						*/
	{	{BOXKEY_CLR_CORE_PL1}	, RSC(BOX_EVENT_POWER_PL1).CODE(),
		{EVENT_CORE_PL1_LOG	, EVENT_CORE_PL1_STS}		},
	/*	Package PL2						*/
	{	{BOXKEY_CLR_CORE_PL2}	, RSC(BOX_EVENT_POWER_PL2).CODE(),
		{EVENT_CORE_PL2_LOG	, EVENT_CORE_PL2_STS}		},
	/*	Electrical EDP						*/
	{	{BOXKEY_CLR_CORE_EDP}	, RSC(BOX_EVENT_ELECTRICAL).CODE(),
		{EVENT_CORE_EDP_LOG	, EVENT_CORE_EDP_STS}		},
	/*	Residency						*/
	{	{BOXKEY_CLR_CORE_RES}	, RSC(BOX_EVENT_RESIDENCY).CODE(),
		{EVENT_CORE_RES_LOG	, EVENT_CORE_RES_STS}		},
	/*	Max Turbo Limit.					*/
	{	{BOXKEY_CLR_CORE_BST}	, RSC(BOX_EVENT_MAX_TURBO).CODE(),
		{EVENT_CORE_BST_LOG	, EVENT_CORE_BST_STS}		},
	/*	Turbo Transition Attenuation				*/
	{	{BOXKEY_CLR_CORE_ATT}	, RSC(BOX_EVENT_TURBO_ATTEN).CODE(),
		{EVENT_CORE_ATT_LOG	, EVENT_CORE_ATT_STS}		},
	/*	Thermal Velocity Boost					*/
	{	{BOXKEY_CLR_CORE_TVB}	, RSC(BOX_EVENT_THERMAL_TVB).CODE(),
		{EVENT_CORE_TVB_LOG	, EVENT_CORE_TVB_STS}		}
      }, {
	/*	Thermal Sensor						*/
	{	{BOXKEY_CLR_GFX_THM}	, RSC(BOX_EVENT_THERMAL_SENSOR).CODE(),
		{EVENT_GFX_THM_LOG	, EVENT_GFX_THM_STS}		},
	/*	PROCHOT# Agent						*/
	{	{BOXKEY_CLR_GFX_HOT}	, RSC(BOX_EVENT_PROCHOT_STS).CODE(),
		{EVENT_GFX_HOT_LOG	, EVENT_GFX_HOT_STS}		},
	/*	Avg Thermal						*/
	{	{BOXKEY_CLR_GFX_AVG}	, RSC(BOX_EVENT_AVG_THERMAL).CODE(),
		{EVENT_GFX_AVG_LOG	, EVENT_GFX_AVG_STS}		},
	/*	VR Thermal						*/
	{	{BOXKEY_CLR_GFX_VRT}	, RSC(BOX_EVENT_VR_THERMAL).CODE(),
		{EVENT_GFX_VRT_LOG	, EVENT_GFX_VRT_STS}		},
	/*	VR TDC							*/
	{	{BOXKEY_CLR_GFX_TDC}	, RSC(BOX_EVENT_VR_TDC).CODE(),
		{EVENT_GFX_TDC_LOG	, EVENT_GFX_TDC_STS}		},
	/*	Package PL1						*/
	{	{BOXKEY_CLR_GFX_PL1}	, RSC(BOX_EVENT_POWER_PL1).CODE(),
		{EVENT_GFX_PL1_LOG	, EVENT_GFX_PL1_STS}		},
	/*	Package PL2						*/
	{	{BOXKEY_CLR_GFX_PL2}	, RSC(BOX_EVENT_POWER_PL2).CODE(),
		{EVENT_GFX_PL2_LOG	, EVENT_GFX_PL2_STS}		},
	/*	Electrical EDP						*/
	{	{BOXKEY_CLR_GFX_EDP}	, RSC(BOX_EVENT_ELECTRICAL).CODE(),
		{EVENT_GFX_EDP_LOG	, EVENT_GFX_EDP_STS}		},
	/*	Inefficiency Ops					*/
	{	{BOXKEY_CLR_GFX_EFF}	, RSC(BOX_EVENT_INEFFICIENCY).CODE(),
		{EVENT_GFX_EFF_LOG	, EVENT_GFX_EFF_STS}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		}
      }, {
	/*	Thermal Sensor						*/
	{	{BOXKEY_CLR_RING_THM}	, RSC(BOX_EVENT_THERMAL_SENSOR).CODE(),
		{EVENT_RING_THM_LOG	, EVENT_RING_THM_STS}		},
	/*	PROCHOT# Agent						*/
	{	{BOXKEY_CLR_RING_HOT}	, RSC(BOX_EVENT_PROCHOT_STS).CODE(),
		{EVENT_RING_HOT_LOG	, EVENT_RING_HOT_STS}		},
	/*	Avg Thermal						*/
	{	{BOXKEY_CLR_RING_AVG}	, RSC(BOX_EVENT_AVG_THERMAL).CODE(),
		{EVENT_RING_AVG_LOG	, EVENT_RING_AVG_STS}		},
	/*	VR Thermal						*/
	{	{BOXKEY_CLR_RING_VRT}	, RSC(BOX_EVENT_VR_THERMAL).CODE(),
		{EVENT_RING_VRT_LOG	, EVENT_RING_VRT_STS}		},
	/*	VR TDC							*/
	{	{BOXKEY_CLR_RING_TDC}	, RSC(BOX_EVENT_VR_TDC).CODE(),
		{EVENT_RING_TDC_LOG	, EVENT_RING_TDC_STS}		},
	/*	Package PL1						*/
	{	{BOXKEY_CLR_RING_PL1}	, RSC(BOX_EVENT_POWER_PL1).CODE(),
		{EVENT_RING_PL1_LOG	, EVENT_RING_PL1_STS}		},
	/*	Package PL2						*/
	{	{BOXKEY_CLR_RING_PL2}	, RSC(BOX_EVENT_POWER_PL2).CODE(),
		{EVENT_RING_PL2_LOG	, EVENT_RING_PL2_STS}		},
	/*	Electrical EDP						*/
	{	{BOXKEY_CLR_RING_EDP}	, RSC(BOX_EVENT_ELECTRICAL).CODE(),
		{EVENT_RING_EDP_LOG	, EVENT_RING_EDP_STS}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		},
	/*	Blank cell						*/
	{	{SCANKEY_NULL}		, RSC(BOX_EVENT_SPACE).CODE(),
		{EVENT_THERM_NONE	, EVENT_THERM_NONE}		}
      }
    };
	const size_t nmemb = sizeof(eLdr) / sizeof(struct EVENT_LDR_ST);

	Window *wEvent = CreateWindow(	wLayer, id,
					EVENT_DOMAINS,
					1 + (nmemb / EVENT_DOMAINS),
					6, TOP_HEADER_ROW + 2 );
  if (wEvent != NULL)
  {
	ATTRIBUTE *attrib[eDIM][eDIM] = {
		{RSC(BOX_EVENT_COND00).ATTR(), RSC(BOX_EVENT_COND01).ATTR()},
		{RSC(BOX_EVENT_COND10).ATTR(), RSC(BOX_EVENT_COND11).ATTR()}
	};
	CUINT col, row;
    for (row = 0; row < EVENT_SECTIONS; row++) {
      for (col = 0; col < EVENT_DOMAINS; col++) {
	const unsigned int eTheme[eDIM] = {
		(eLdr[col][row].events[eLOG] != EVENT_THERM_NONE)
	    &&	(RO(Shm)->ProcessorEvents[eLOG] & eLdr[col][row].events[eLOG]),

		(eLdr[col][row].events[eSTS] != EVENT_THERM_NONE)
	    &&	(RO(Shm)->ProcessorEvents[eSTS] & eLdr[col][row].events[eSTS])
	};

	TGrid *grid=StoreTCell( wEvent,
				eLdr[col][row].quick.key,
				eLdr[col][row].item,
				attrib[eTheme[eLOG]][eTheme[eSTS]] );

	GridCall(grid, Update_Event,
		eLdr[col][row].events[eLOG], eLdr[col][row].events[eSTS]);
      }
    }
    for (col = 0; col < EVENT_DOMAINS - 1; col++) {
	StoreTCell(wEvent,	SCANKEY_NULL,
				RSC(BOX_EVENT_SPACE).CODE(),
				RSC(BOX_EVENT_COND00).ATTR());
    }
	StoreTCell(wEvent,	BOXKEY_CLR_ALL_EVENTS,
				RSC(BOX_EVENT_ALL_OF_THEM).CODE(),
				RSC(BOX_EVENT_BUTTON).ATTR());

	wEvent->matrix.select.col = wEvent->matrix.size.wth - 1;
	wEvent->matrix.select.row = wEvent->matrix.size.hth - 1;

	StoreWindow(wEvent,	.title, (char*) RSC(BOX_EVENT_TITLE).CODE());
	StoreWindow(wEvent,	.color[1].title, wEvent->hook.color[1].border);

	StoreWindow(wEvent,	.key.Enter,	Enter_StickyCell);
	StoreWindow(wEvent,	.key.Left,	MotionLeft_Win);
	StoreWindow(wEvent,	.key.Right,	MotionRight_Win);
	StoreWindow(wEvent,	.key.Down,	MotionDown_Win);
	StoreWindow(wEvent,	.key.Up,	MotionUp_Win);
	StoreWindow(wEvent,	.key.PgUp,	MotionPgUp_Win);
	StoreWindow(wEvent,	.key.PgDw,	MotionPgDw_Win);
	StoreWindow(wEvent,	.key.Home,	MotionHome_Win);
	StoreWindow(wEvent,	.key.End,	MotionEnd_Win);

	StoreWindow(wEvent,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wEvent,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wEvent,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wEvent,	.key.WinUp,	MotionOriginUp_Win);

	StoreWindow(wEvent,	.key.Shrink,	MotionShrink_Win);
	StoreWindow(wEvent,	.key.Expand,	MotionExpand_Win);
  }
	return wEvent;
}

#undef EVENT_DOMAINS
#undef EVENT_SECTIONS

Window *CreateClockSource(unsigned long long id)
{
	const unsigned int count = (unsigned int) RO(Shm)->CS.index[0];

	Window *wCS = CreateWindow(	wLayer, id, 1, count + 1,
					22, TOP_HEADER_ROW + 7,
					WINFLAG_NO_STOCK );
    if (wCS != NULL)
    {
	const char *current = RO(Shm)->CS.array;
	CUINT row = 1;
	unsigned int idx;

	StoreTCell(wCS, SCANKEY_NULL, RSC(BOX_BLANK_DESC).CODE(),
			RSC(UI).ATTR()[UI_BOX_ENABLE_STATE]);

	for (idx = 1; idx < count; idx++)
	{
		const unsigned long long key = OPS_CLOCK_SOURCE | idx;
		const char *avail = &RO(Shm)->CS.array[RO(Shm)->CS.index[idx]];
		char *format;
		CUINT lt, rt, li = strlen(avail);
		ATTRIBUTE attrib;

		lt = RSZ(BOX_BLANK_DESC) - li;
		rt = lt >> 1;
		lt = rt + (lt & 0x1);

	    if (!strcmp(current, avail)) {
		format = "%.*s" "<  %s  >" "%.*s";
		attrib = RSC(UI).ATTR()[UI_BOX_DISABLE_STATE];
		row = (CUINT) (idx - 1);
		lt = lt - 3;
		rt = rt - 3;
	    } else {
		format = "%.*s" "%s" "%.*s";
		attrib = RSC(UI).ATTR()[UI_BOX_ENABLE_STATE];
	    }
		StrLenFormat(li, Buffer, MAX_UTS_LEN,
				format, lt, HSPACE, avail, rt, HSPACE);

		StoreTCell(wCS, key, Buffer, attrib);
	}
	StoreTCell(wCS, SCANKEY_NULL, RSC(BOX_BLANK_DESC).CODE(),
			RSC(UI).ATTR()[UI_BOX_ENABLE_STATE]);

	wCS->matrix.select.row = 1 + row;

	StoreWindow(wCS, .title, (char*) RSC(BOX_CLOCK_SOURCE_TITLE).CODE());

	StoreWindow(wCS,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wCS,	.key.Down,	MotionDown_Win);
	StoreWindow(wCS,	.key.Up,	MotionUp_Win);
	StoreWindow(wCS,	.key.Home,	MotionReset_Win);
	StoreWindow(wCS,	.key.End,	MotionEnd_Cell);
    }
	return wCS;
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
		unsigned int duration = Recorder.Ratios[idx] * RECORDER_DEFAULT,
				remainder = duration % (60 * 60),
				hours	= duration / (60 * 60),
				minutes = remainder / 60,
				seconds = remainder % 60;

		StrFormat(Buffer, 6+11+11+11+1,
				"\x20\x20%02u:%02u:%02u\x20\x20",
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
	return wRec;
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
				1, Draw.Size.height - (1 + 5),
				WINFLAG_NO_STOCK );
    if (wMsg != NULL)
    {
	struct tm *brokTime, localTime;
	time_t execTime;
	size_t hdrLen, sysLen;

	execTime = RO(Shm)->StartedAt + pCtrl->tds;
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
		CUINT		len;
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
		},
		{
		RC_CLOCKSOURCE,
		RSC(ERROR_CLOCKSOURCE).CODE(), RSZ(ERROR_CLOCKSOURCE)
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
	hdrLen = (size_t) sprintf(outStr, "[%x:%llx]", pCtrl->cmd, pCtrl->arg);
	if ((hdrLen > 0) && (hdrLen < POPUP_WIDTH)) {
		memcpy(item, outStr, hdrLen);
	}
	StoreTCell(wMsg, SCANKEY_NULL, item,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_ITEM]);

	memset(item, 0x20, POPUP_WIDTH);
	StoreTCell(wMsg, SCANKEY_NULL, item,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_ITEM]);

	if ((sysLen > 0) && (sysLen < POPUP_WIDTH)) {
		memcpy(&item[(POPUP_WIDTH / 2) - (sysLen / 2)], sysMsg,sysLen);
	} else {
		memcpy(item, sysMsg, POPUP_WIDTH);
	}
	StoreTCell(wMsg, SCANKEY_NULL, item,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_ITEM]);

	memset(item, 0x20, POPUP_WIDTH);
	StoreTCell(wMsg, SCANKEY_NULL, item,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_ITEM]);

	StoreTCell(wMsg, SCANKEY_NULL, item,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_ITEM]);

	StoreWindow(wMsg, .color[0].select,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_SELECT_UNFOCUS]);

	StoreWindow(wMsg, .color[1].select,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_SELECT_FOCUS]);

	StoreWindow(wMsg, .color[0].border,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_BORDER_UNFOCUS]);

	StoreWindow(wMsg, .color[1].border,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_BORDER_FOCUS]);

	StoreWindow(wMsg, .color[0].title,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_TITLE_UNFOCUS]);

	StoreWindow(wMsg, .color[1].title,
			RSC(UI).ATTR()[UI_WIN_POPUP_MSG_TITLE_FOCUS]);

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
	return wMsg;
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
		size_t size;
		cnt	= (pBox == NULL) ? 1 : pBox->cnt + 1;
		size	= sizeof(struct PBOX)
			+ sizeof(struct SBOX) * (unsigned int) cnt;
		if ((pBox = realloc(pBox, size)) != NULL)
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
	return wBox;
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
		RSC(UI).ATTR()[UI_WIN_EXIT_ISSUE_RECORDER],
		SCANKEY_ALT_p
	    },
		DumpStatus()
	  },
	  {
	    {
		RSC(STRESS).CODE(),
		RSZ(STRESS),
		RSC(UI).ATTR()[UI_WIN_EXIT_ISSUE_STRESS],
		BOXKEY_TOOLS_MACHINE
	    },
		BITVAL(RW(Shm)->Proc.Sync, BURN)
	  },
	  {
	    {
		RSC(KERNEL_IDLE_DRIVER).CODE(),
		RSZ(KERNEL_IDLE_DRIVER),
		RSC(UI).ATTR()[UI_WIN_EXIT_ISSUE_OS_CPU_IDLE],
		OPS_CPU_IDLE
	    },
		(RO(Shm)->Registration.Driver.CPUidle == REGISTRATION_ENABLE)
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
	return list;
}

Window *CreateExit(unsigned long long id, IssueList *issue, CUINT wth,CUINT hth)
{
	Window *wExit;

	wExit = CreateWindow(	wLayer, id,
				1, (hth + 6),
				(Draw.Size.width - wth) / 2,
				(hth + 6) > Draw.Size.height ?
					1 : (Draw.Size.height - (hth + 2)) / 2,
				WINFLAG_NO_STOCK );
    if (wExit != NULL)
    {
	CUINT idx;

	StoreTCell(	wExit, SCANKEY_NULL, RSC(EXIT_HEADER).CODE(),
			RSC(UI).ATTR()[UI_WIN_EXIT_HEADER] );

	memset(Buffer, 0x20, wth);
	Buffer[wth] = '\0';
	StoreTCell(	wExit, SCANKEY_NULL, Buffer,
			RSC(UI).ATTR()[UI_WIN_EXIT_BLANK] );

	for (idx = 0; idx < hth; idx++)
	{
		const CUINT pos = (wth - issue[idx].length) / 2;
		memset(Buffer, 0x20, wth);
		memcpy(&Buffer[pos], issue[idx].item, issue[idx].length);
		Buffer[wth] = '\0';

		StoreTCell(wExit, issue[idx].quick, Buffer, issue[idx].attrib);
	};
	memset(Buffer, 0x20, wth);
	Buffer[wth] = '\0';
	StoreTCell(	wExit, SCANKEY_NULL, Buffer,
			RSC(UI).ATTR()[UI_WIN_EXIT_BLANK] );

	StoreTCell(	wExit, SCANKEY_CTRL_ALT_x, RSC(EXIT_CONFIRM).CODE(),
			RSC(UI).ATTR()[UI_WIN_EXIT_CONFIRM] );

	memset(Buffer, 0x20, wth);
	Buffer[wth] = '\0';
	StoreTCell(	wExit, SCANKEY_NULL, Buffer,
			RSC(UI).ATTR()[UI_WIN_EXIT_BLANK] );

	StoreTCell(	wExit, SCANKEY_NULL, RSC(EXIT_FOOTER).CODE(),
			RSC(UI).ATTR()[UI_WIN_EXIT_FOOTER] );

	wExit->matrix.select.row = 2;

	StoreWindow(wExit,	.key.Enter,	MotionEnter_Cell);
	StoreWindow(wExit,	.key.Down,	MotionDown_Win);
	StoreWindow(wExit,	.key.Up,	MotionUp_Win);
	StoreWindow(wExit,	.key.Home,	MotionReset_Win);
	StoreWindow(wExit,	.key.End,	MotionEnd_Cell);
	StoreWindow(wExit,	.title, (char*) RSC(EXIT_TITLE).CODE());
    }
	return wExit;
}

void TrapScreenSize(int caught)
{
  if (caught == SIGWINCH)
  {
	SCREEN_SIZE currentSize = GetScreenSize();
	CUINT areaMaxRows = 1;

    if (currentSize.height != Draw.Size.height)
    {
	if (currentSize.height > MAX_HEIGHT) {
		Draw.Size.height = MAX_HEIGHT;
	} else {
		Draw.Size.height = currentSize.height;
	}
	switch (Draw.Disposal) {
	case D_MAINVIEW:
	Draw.Area.MinHeight = (ADD_LOWER * (Draw.View == V_PACKAGE ? 10 : 0))
				+ TOP_HEADER_ROW
				+ TOP_SEPARATOR
				+ TOP_FOOTER_ROW;
	break;
	default:
	Draw.Area.MinHeight = LEADING_TOP + 2 * (MARGIN_HEIGHT + INTER_HEIGHT);
	break;
	}
	Draw.Flag.clear  = 1;
	Draw.Flag.height = !(Draw.Size.height < Draw.Area.MinHeight);
    }
    if (currentSize.width != Draw.Size.width) {
	if (currentSize.width > MAX_WIDTH) {
		Draw.Size.width = MAX_WIDTH;
	} else {
		Draw.Size.width = currentSize.width;
	}
	Draw.Flag.clear = 1;
	Draw.Flag.width = !(Draw.Size.width < MIN_WIDTH);
    }
    if ((ADD_UPPER | ADD_LOWER) && (Draw.Size.height > Draw.Area.MinHeight)) {
	areaMaxRows = Draw.Size.height - Draw.Area.MinHeight;
      if (Draw.View != V_PACKAGE) {
	areaMaxRows = areaMaxRows >> (ADD_UPPER & ADD_LOWER);
      }
      if (!areaMaxRows) {
	areaMaxRows = 1;
      }
    }
	Draw.Area.MaxRows = CUMIN(RO(Shm)->Proc.CPU.Count, areaMaxRows);
	Draw.Area.LoadWidth = Draw.Size.width - LOAD_LEAD;
	Draw.cpuScroll = 0;

    if (Draw.Flag.clear == 1 ) {
	ReScaleAllWindows(&winList);
    }
  }
}

int Shortcut(SCANKEY *scan)
{
	const ATTRIBUTE stateAttr[2] = {
		RSC(UI).ATTR()[UI_BOX_ENABLE_STATE],
		RSC(UI).ATTR()[UI_BOX_DISABLE_STATE]
	},
	blankAttr = RSC(UI).ATTR()[UI_BOX_BLANK],
	descAttr =  RSC(UI).ATTR()[UI_BOX_DESC];

	const ASCII *stateStr[2][2] = {
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
		return -1;
	}
	fallthrough;
    case SCANKEY_PLUS:
	if ((Draw.Disposal == D_MAINVIEW)
	&&  (Draw.cpuScroll < (RO(Shm)->Proc.CPU.Count - Draw.Area.MaxRows)))
	{
		Draw.cpuScroll++;
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_UP:
	if (!IsDead(&winList)) {
		return -1;
	}
	fallthrough;
    case SCANKEY_MINUS:
	if ((Draw.Disposal == D_MAINVIEW) && (Draw.cpuScroll > 0)) {
		Draw.cpuScroll--;
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_HOME:
    case SCANCON_HOME:
    case SCANSYM_HOME:
	if (!IsDead(&winList)) {
		return -1;
	} else if (Draw.Disposal == D_MAINVIEW) {
		Draw.cpuScroll = 0;
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_END:
    case SCANCON_END:
    case SCANSYM_END:
	if (!IsDead(&winList)) {
		return -1;
	} else if (Draw.Disposal == D_MAINVIEW) {
		Draw.cpuScroll = RO(Shm)->Proc.CPU.Count - Draw.Area.MaxRows;
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_PGDW:
	if (!IsDead(&winList)) {
		return -1;
	} else if (Draw.Disposal == D_MAINVIEW) {
	    if ( (Draw.cpuScroll + Draw.Area.MaxRows) < (RO(Shm)->Proc.CPU.Count
							- Draw.Area.MaxRows ) )
	    {
		Draw.cpuScroll += Draw.Area.MaxRows;
	    } else {
		Draw.cpuScroll = RO(Shm)->Proc.CPU.Count - Draw.Area.MaxRows;
	    }
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_PGUP:
	if (!IsDead(&winList)) {
		return -1;
	} else if (Draw.Disposal == D_MAINVIEW) {
	    if (Draw.cpuScroll >= Draw.Area.MaxRows) {
		Draw.cpuScroll -= Draw.Area.MaxRows;
	    } else {
		Draw.cpuScroll = 0;
	    }
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_AST:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(RW(Shm)->Ring[0], COREFREQ_IOCTL_SYSUPDT);
	}
    break;

    case SCANKEY_OPEN_BRACE:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CONTROLLER );
	}
    break;

    case SCANKEY_CLOSE_BRACE:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CONTROLLER );
	}
    break;

    case SCANKEY_F2:
    case SCANCON_F2:
    case SCANSYM_F2:
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
    case SCANSYM_F3:
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
    case SCANSYM_F4:
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
	Draw.Flag.uBench = !Draw.Flag.uBench;
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
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_INTERVAL_TITLE).CODE(),
			RSC(BOX_INTERVAL_STEP1).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_100,
			RSC(BOX_INTERVAL_STEP2).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_150,
			RSC(BOX_INTERVAL_STEP3).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_250,
			RSC(BOX_INTERVAL_STEP4).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_500,
			RSC(BOX_INTERVAL_STEP5).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_750,
			RSC(BOX_INTERVAL_STEP6).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_1000,
			RSC(BOX_INTERVAL_STEP7).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_1500,
			RSC(BOX_INTERVAL_STEP8).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_2000,
			RSC(BOX_INTERVAL_STEP9).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL], OPS_INTERVAL_2500,
			RSC(BOX_INTERVAL_STEP10).CODE(),
			RSC(UI).ATTR()[UI_BOX_INTERVAL],OPS_INTERVAL_3000),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case OPS_INTERVAL_100:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				100,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_150:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				150,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_250:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				250,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_500:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				500,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_750:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				750,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_1000:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				1000,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_1500:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				1500,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_2000:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				2000,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_2500:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				2500,
				MACHINE_INTERVAL );
	}
    break;

    case OPS_INTERVAL_3000:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
	const int bON = ((RO(Shm)->Registration.AutoClock & 0b10) != 0);
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 4
	}, select = {
		.col = 0,
		.row = bON ? 2 : 1
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_AUTO_CLOCK_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][bON] , stateAttr[bON] , OPS_AUTOCLOCK_ON,
			stateStr[0][!bON], stateAttr[!bON], OPS_AUTOCLOCK_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case OPS_AUTOCLOCK_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_AUTOCLOCK );
	}
    break;

    case OPS_AUTOCLOCK_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		RSC(UI).ATTR()[UI_BOX_EXPERIMENTAL_WARNING],
		RSC(UI).ATTR()[UI_BOX_EXPERIMENTAL_NOMINAL]
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_MODE_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_MODE_DESC).CODE(), descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			ops_Str[0][RO(Shm)->Registration.Experimental == 0],
			stateAttr[RO(Shm)->Registration.Experimental == 0],
							OPS_EXPERIMENTAL_OFF,
			ops_Str[1][RO(Shm)->Registration.Experimental != 0] ,
			exp_Attr[RO(Shm)->Registration.Experimental != 0],
							OPS_EXPERIMENTAL_ON,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case OPS_EXPERIMENTAL_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_EXPERIMENTAL );
	}
    break;

    case OPS_EXPERIMENTAL_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_EXPERIMENTAL );
	}
    break;

    case OPS_IDLE_ROUTE:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = 43,
		.row = TOP_HEADER_ROW + 17
	}, select = {
		.col = 0,
		.row = RO(Shm)->Registration.Driver.Route
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char *) RSC(SETTINGS_ROUTE_TITLE).CODE(),
		RSC(SETTINGS_ROUTE_DFLT).CODE(), stateAttr[0], OPS_ROUTE_DFLT,
		RSC(SETTINGS_ROUTE_IO).CODE(),   stateAttr[0], OPS_ROUTE_IO,
		RSC(SETTINGS_ROUTE_HALT).CODE(), stateAttr[0], OPS_ROUTE_HALT,
		RSC(SETTINGS_ROUTE_MWAIT).CODE(),stateAttr[0], OPS_ROUTE_MWAIT),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case OPS_ROUTE_DFLT:
    case OPS_ROUTE_IO:
    case OPS_ROUTE_HALT:
    case OPS_ROUTE_MWAIT:
    {
	enum IDLE_ROUTE idleRoute = (scan->key & BOXKEY_ROUTE_MASK) >> 4;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				idleRoute,
				MACHINE_IDLE_ROUTE );
	}
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
		bix = BITWISEAND(LOCKLESS,
				RO(Shm)->Registration.NMI,BIT_NMI_MASK) != 0;
		break;
	case OPS_CPU_IDLE:
		ops_title = RSC(BOX_CPU_IDLE_TITLE).CODE();
		ops_key_on  = OPS_CPU_IDLE_ON;
		ops_key_off = OPS_CPU_IDLE_OFF;
		bix	= RO(Shm)->Registration.Driver.CPUidle
			& REGISTRATION_ENABLE;
		break;
	case OPS_CPU_FREQ:
		ops_title = RSC(BOX_CPU_FREQ_TITLE).CODE();
		ops_key_on  = OPS_CPU_FREQ_ON;
		ops_key_off = OPS_CPU_FREQ_OFF;
		bix	= RO(Shm)->Registration.Driver.CPUfreq
			& REGISTRATION_ENABLE;
		break;
	case OPS_GOVERNOR:
		ops_title = RSC(BOX_GOVERNOR_TITLE).CODE();
		ops_key_on  = OPS_GOVERNOR_ON;
		ops_key_off = OPS_GOVERNOR_OFF;
		bix	= RO(Shm)->Registration.Driver.Governor
			& REGISTRATION_ENABLE;
		break;
	case OPS_CLOCK_SOURCE:
		ops_title = RSC(BOX_CLOCK_SOURCE_TITLE).CODE();
		ops_key_on  = OPS_CLOCK_SOURCE_ON;
		ops_key_off = OPS_CLOCK_SOURCE_OFF;
		bix = RO(Shm)->Registration.Driver.CS & REGISTRATION_ENABLE;
		break;
	}
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 5
	}, select = {
		.col = 0,
		.row = bix == 0 ? 1 : 2
	};
	AppendWindow(
		CreateBox(scan->key, origin, select, (char*) ops_title,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL,
			ops_Str[0][bix != 0], stateAttr[bix != 0], ops_key_on,
			ops_Str[1][bix == 0], stateAttr[bix == 0], ops_key_off,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr, SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case OPS_CLOCK_SOURCE_1:
    case OPS_CLOCK_SOURCE_2:
    case OPS_CLOCK_SOURCE_3:
    case OPS_CLOCK_SOURCE_4:
    case OPS_CLOCK_SOURCE_5:
    case OPS_CLOCK_SOURCE_6:
    case OPS_CLOCK_SOURCE_7:
    {
	const unsigned short indexCS = scan->key & CLOCK_SOURCE_MASK;

	if (!RING_FULL(RW(Shm)->Ring[1])) {
		RING_WRITE(	RW(Shm)->Ring[1],
				COREFREQ_KERNEL_MISC,
				indexCS,
				MACHINE_CLOCK_SOURCE );
	}
    }
    break;

    case OPS_CLOCK_SOURCE_SEL:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	AppendWindow(CreateClockSource(scan->key), &winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case OPS_INTERRUPTS_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_INTERRUPTS );
	}
    break;

    case OPS_INTERRUPTS_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_INTERRUPTS );
	}
    break;

    case OPS_CPU_IDLE_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CPU_IDLE );
	}
    break;

    case OPS_CPU_IDLE_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CPU_IDLE );
	}
    break;

    case OPS_CPU_FREQ_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CPU_FREQ );
	}
    break;

    case OPS_CPU_FREQ_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_CPU_FREQ );
	}
    break;

    case OPS_GOVERNOR_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_GOVERNOR );
	}
    break;

    case OPS_GOVERNOR_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_ON,
				MACHINE_GOVERNOR );
	}
    break;

    case OPS_CLOCK_SOURCE_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				COREFREQ_TOGGLE_OFF,
				MACHINE_CLOCK_SOURCE );
	}
    break;

    case OPS_CLOCK_SOURCE_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		{ .pThermal	= &RO(Shm)->Proc.thermalFormula },
		{ .pVoltage	= &RO(Shm)->Proc.voltageFormula },
		{ .pPower	= &RO(Shm)->Proc.powerFormula	}
	};
	const int index = (scan->key & 0x000000000000f000) >> 12;
	const Coordinate origin = {
		.col = 43,
		.row = TOP_HEADER_ROW + 20
	}, select = {
		.col = 0,
		.row = SCOPE_OF_FORMULA((*formula[index].pInteger))
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
	if ((Draw.View == V_FREQ) && (Draw.Disposal == D_MAINVIEW)) {
		Draw.Flag.avgOrPC = !Draw.Flag.avgOrPC;
		Draw.Flag.clear = 1;
	}
    break;

    case SCANKEY_DOT:
	if (Draw.Disposal == D_MAINVIEW) {
		Draw.Flag.clkOrLd = !Draw.Flag.clkOrLd;
		Draw.Flag.clear = 1;
	}
    break;

    case 0x000000000000007e:
    {
	Draw.Disposal = D_ASCIITEST;
	Draw.Size.height = 0;
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
	if ((Draw.View == V_TASKS) && (Draw.Disposal == D_MAINVIEW)) {
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
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_CYCLES;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
#endif
    case SCANKEY_d:
    {
	Draw.Disposal = D_DASHBOARD;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_f:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_FREQ;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_SHIFT_n:
	if ((Draw.View == V_TASKS) && (Draw.Disposal == D_MAINVIEW)) {
		RING_WRITE_SUB_CMD(	TASK_TRACKING,
					RW(Shm)->Ring[1],
					COREFREQ_TASK_MONITORING,
					(pid_t) 0 );
	}
	break;

    case SCANKEY_n:
	if ((Draw.View == V_TASKS) && (Draw.Disposal == D_MAINVIEW)) {
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
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_PACKAGE;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;
#endif
    case SCANKEY_F1:
    case SCANCON_F1:
    case SCANSYM_F1:
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
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_INST;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_l:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_CSTATES;
	Draw.Size.height = 0;
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
	if (Draw.Disposal == D_MAINVIEW) {
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_SHIFT_f:
	Setting.fahrCels = !Setting.fahrCels;
	Draw.Flag.layout = 1;
    break;

    case SCANKEY_SHIFT_y:
	Setting.secret = !Setting.secret;
	Draw.Flag.layout = 1;
    break;

    case SCANKEY_SHIFT_g:
	if (!RING_FULL(RW(Shm)->Ring[1])) {
		RING_WRITE(RW(Shm)->Ring[1],
			COREFREQ_TOGGLE_SYSGATE,
			!BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1));
	}
    break;

    case SCANKEY_SHIFT_h:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreateEvents(scan->key), &winList);
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
			.row = 1 + ((GET_LOCALE() >= LOC_EN)
				 && (GET_LOCALE() < LOC_CNT) ?
					GET_LOCALE() : LOC_EN)
		};

	Window *wBox = CreateBox(scan->key, origin, select,
		(char*) RSC(BOX_LANG_TITLE).CODE(),
		RSC(BOX_LANG_BLANK).CODE(), blankAttr	, SCANKEY_NULL,
		RSC(BOX_LANG_ENGLISH).CODE(),stateAttr[0], BOXKEY_LANG_ENGLISH,
		RSC(BOX_LANG_FRENCH).CODE(), stateAttr[0], BOXKEY_LANG_FRENCH,
		RSC(BOX_LANG_BLANK).CODE(), blankAttr	, SCANKEY_NULL);

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
		Draw.Flag.layout = 1;
	}
    }
    break;

    case BOXKEY_LANG_FRENCH:
    {
	if (GET_LOCALE() != LOC_FR) {
		SET_LOCALE(LOC_FR);
		Draw.Flag.layout = 1;
	}
    }
    break;

    case SCANKEY_SHIFT_e:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	{
		const Coordinate origin = {
			.col = (Draw.Size.width - RSZ(BOX_THEME_BLANK)) / 2,
			.row = TOP_HEADER_ROW + 11
		}, select = {
			.col = 0,
			.row = 1 + GET_THEME()
		};

	Window *wBox = CreateBox(scan->key, origin, select,
		(char*) RSC(BOX_THEME_TITLE).CODE(),
		RSC(BOX_THEME_BLANK).CODE(), blankAttr, SCANKEY_NULL,
		RSC(THEME_DFLT).CODE()	, stateAttr[0], BOXKEY_THEME_DFLT,
		RSC(THEME_USR1).CODE()	, stateAttr[0], BOXKEY_THEME_USR1,
		RSC(THEME_USR2).CODE()	, stateAttr[0], BOXKEY_THEME_USR2,
		RSC(BOX_THEME_BLANK).CODE(), blankAttr, SCANKEY_NULL);

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

    case BOXKEY_THEME_DFLT:
	SET_THEME(THM_DFLT);
	Draw.Flag.clear = 1;
    break;

    case BOXKEY_THEME_USR1:
	SET_THEME(THM_USR1);
	Draw.Flag.clear = 1;
    break;

    case BOXKEY_THEME_USR2:
	SET_THEME(THM_USR2);
	Draw.Flag.clear = 1;
    break;

    case SCANKEY_SHIFT_m:
	if (RO(Shm)->Uncore.CtrlCount > 0) {
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
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_INTR;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_SHIFT_c:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_SENSORS;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_SHIFT_v:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_VOLTAGE;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_SHIFT_w:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_ENERGY;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_SHIFT_t:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_SLICE;
	Draw.Size.height = 0;
	TrapScreenSize(SIGWINCH);
    }
    break;

    case SCANKEY_y:
    {
	Draw.Disposal = D_MAINVIEW;
	Draw.View = V_CUSTOM;
	Draw.Size.height = 0;
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
	if ((Draw.View == V_TASKS) && (Draw.Disposal == D_MAINVIEW)) {
		RING_WRITE_SUB_CMD(	TASK_INVERSING,
					RW(Shm)->Ring[1],
					COREFREQ_TASK_MONITORING );
	}
    break;

    case SCANKEY_v:
	if ((Draw.View == V_TASKS) && (Draw.Disposal == D_MAINVIEW)) {
		Draw.Flag.taskVal = !Draw.Flag.taskVal;
		Draw.Flag.layout = 1;
	}
    break;
#ifndef NO_LOWER
    case SCANKEY_x:
	if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)) {
		Draw.Disposal = D_MAINVIEW;
		Draw.View = V_TASKS;
		Draw.Size.height = 0;
		TrapScreenSize(SIGWINCH);
	}
    break;
#endif
    case SCANKEY_EXCL:
	if (Draw.Disposal == D_MAINVIEW) {
		Draw.Load = !Draw.Load;
		Draw.Flag.layout = 1;
	}
    break;

    case SORTBY_STATE:
    {
	RING_WRITE_SUB_CMD(	TASK_SORTING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
				F_STATE );
    }
    break;

    case SORTBY_RTIME:
    {
	RING_WRITE_SUB_CMD(	TASK_SORTING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
				F_RTIME );
    }
    break;

    case SORTBY_UTIME:
    {
	RING_WRITE_SUB_CMD(	TASK_SORTING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
				F_UTIME );
    }
    break;

    case SORTBY_STIME:
    {
	RING_WRITE_SUB_CMD(	TASK_SORTING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
				F_STIME );
    }
    break;

    case SORTBY_PID:
    {
	RING_WRITE_SUB_CMD(	TASK_SORTING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
				F_PID );
    }
    break;

    case SORTBY_COMM:
    {
	RING_WRITE_SUB_CMD(	TASK_SORTING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
				F_COMM );
    }
    break;

    case BOXKEY_L1_HW_PREFETCH:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 2
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.L1_HW_Prefetch ? 2 : 1
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_CU_L1_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.L1_HW_Prefetch],
			stateAttr[RO(Shm)->Proc.Technology.L1_HW_Prefetch],
						BOXKEY_L1_HW_PREFETCH_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.L1_HW_Prefetch],
			stateAttr[!RO(Shm)->Proc.Technology.L1_HW_Prefetch],
						BOXKEY_L1_HW_PREFETCH_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_L1_HW_PREFETCH_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L1_HW_PREFETCH );
	}
    break;

    case BOXKEY_L1_HW_PREFETCH_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch ? 2 : 1
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_CU_L1_IP_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch],
			stateAttr[RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch],
						BOXKEY_L1_HW_IP_PREFETCH_ON,
		       stateStr[0][!RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch],
			stateAttr[!RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch],
						BOXKEY_L1_HW_IP_PREFETCH_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_L1_HW_IP_PREFETCH_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L1_HW_IP_PREFETCH );
	}
    break;

    case BOXKEY_L1_HW_IP_PREFETCH_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.L2_HW_Prefetch ? 2 : 1
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_CU_L2_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.L2_HW_Prefetch],
			stateAttr[RO(Shm)->Proc.Technology.L2_HW_Prefetch],
						BOXKEY_L2_HW_PREFETCH_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.L2_HW_Prefetch],
			stateAttr[!RO(Shm)->Proc.Technology.L2_HW_Prefetch],
						BOXKEY_L2_HW_PREFETCH_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_L2_HW_PREFETCH_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L2_HW_PREFETCH );
	}
    break;

    case BOXKEY_L2_HW_PREFETCH_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 4
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch ? 2 : 1
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_CU_L2_CL_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch],
			stateAttr[RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch],
						BOXKEY_L2_HW_CL_PREFETCH_ON,
		       stateStr[0][!RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch],
			stateAttr[!RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch],
						BOXKEY_L2_HW_CL_PREFETCH_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_L2_HW_CL_PREFETCH_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_L2_HW_CL_PREFETCH );
	}
    break;

    case BOXKEY_L2_HW_CL_PREFETCH_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 2
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.EIST ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_EIST_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_EIST_DESC).CODE(), descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.EIST],
				stateAttr[RO(Shm)->Proc.Technology.EIST],
								BOXKEY_EIST_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.EIST],
				stateAttr[!RO(Shm)->Proc.Technology.EIST],
								BOXKEY_EIST_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_EIST_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_EIST );
	}
    break;

    case BOXKEY_EIST_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.C1E ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_C1E_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_C1E_DESC).CODE(), descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.C1E],
				stateAttr[RO(Shm)->Proc.Technology.C1E],
								BOXKEY_C1E_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.C1E],
				stateAttr[!RO(Shm)->Proc.Technology.C1E],
								BOXKEY_C1E_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_C1E_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C1E );
	}
    break;

    case BOXKEY_C1E_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 2
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.Turbo ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_TURBO_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_TURBO_DESC).CODE(), descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.Turbo],
				stateAttr[RO(Shm)->Proc.Technology.Turbo],
								BOXKEY_TURBO_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.Turbo],
				stateAttr[!RO(Shm)->Proc.Technology.Turbo],
							       BOXKEY_TURBO_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_TURBO_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_TURBO );
	}
    break;

    case BOXKEY_TURBO_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 5
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.C1A ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_C1A_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_C1A_DESC).CODE(), descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.C1A],
				stateAttr[RO(Shm)->Proc.Technology.C1A],
								BOXKEY_C1A_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.C1A],
				stateAttr[!RO(Shm)->Proc.Technology.C1A],
								BOXKEY_C1A_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_C1A_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C1A );
	}
    break;

    case BOXKEY_C1A_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 6
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.C3A ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_C3A_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_C3A_DESC).CODE(), descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.C3A] ,
				stateAttr[RO(Shm)->Proc.Technology.C3A],
								BOXKEY_C3A_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.C3A],
				stateAttr[!RO(Shm)->Proc.Technology.C3A],
								BOXKEY_C3A_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_C3A_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C3A );
	}
    break;

    case BOXKEY_C3A_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 7
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.C1U ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
		    (  (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		    || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) ?
				  (char*) RSC(BOX_C2U_TITLE).CODE()
				: (char*) RSC(BOX_C1U_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
		    (  (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		    || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) ?
			RSC(BOX_C2U_DESC).CODE() : RSC(BOX_C1U_DESC).CODE(),
						descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.C1U],
				stateAttr[RO(Shm)->Proc.Technology.C1U],
								BOXKEY_C1U_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.C1U],
				stateAttr[!RO(Shm)->Proc.Technology.C1U],
								BOXKEY_C1U_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_C1U_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C1U );
	}
    break;

    case BOXKEY_C1U_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 8
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.C3U ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_C3U_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_C3U_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.C3U],
				stateAttr[RO(Shm)->Proc.Technology.C3U],
								BOXKEY_C3U_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.C3U],
				stateAttr[!RO(Shm)->Proc.Technology.C3U],
								BOXKEY_C3U_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_C3U_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_C3U );
	}
    break;

    case BOXKEY_C3U_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
	ASCII *title;
	ASCII *descCode;
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 9
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.CC6 ? 4 : 3
	};
	if ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) {
		title = RSC(BOX_CC6_TITLE).CODE();
		descCode = RSC(BOX_CC6_DESC).CODE();
	} else {
		title = RSC(BOX_C6D_TITLE).CODE();
		descCode = RSC(BOX_C6D_DESC).CODE();
	}
	AppendWindow(
		CreateBox(scan->key, origin, select, (char*) title,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			descCode, descAttr, SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.CC6],
				stateAttr[RO(Shm)->Proc.Technology.CC6],
								BOXKEY_CC6_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.CC6],
				stateAttr[!RO(Shm)->Proc.Technology.CC6],
								BOXKEY_CC6_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_CC6_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_CC6 );
	}
    break;

    case BOXKEY_CC6_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
	ASCII *title;
	ASCII *descCode;
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 10
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.PC6 ? 4 : 3
	};
	if ( (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	  || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) {
		title = RSC(BOX_PC6_TITLE).CODE();
		descCode = RSC(BOX_PC6_DESC).CODE();
	} else {
		title = RSC(BOX_MC6_TITLE).CODE();
		descCode = RSC(BOX_MC6_DESC).CODE();
	}
	AppendWindow(
		CreateBox(scan->key, origin, select, (char*) title,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			descCode, descAttr, SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.PC6],
				stateAttr[RO(Shm)->Proc.Technology.PC6],
								BOXKEY_PC6_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.PC6],
				stateAttr[!RO(Shm)->Proc.Technology.PC6],
								BOXKEY_PC6_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_PC6_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_PC6 );
	}
    break;

    case BOXKEY_PC6_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - (44 - 17)) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = (CUINT) \
		CST[RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.CStateLimit]
	};
	Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_PKG_STATE_LIMIT_TITLE).CODE(),
/* 0 */ RSC(BOX_STATE_UNSPECIFIED).CODE() , stateAttr[0], SCANKEY_NULL,
/* 1 */ RSC(BOX_PKG_STATE_LIMIT_C10).CODE(),stateAttr[0], BOXKEY_PKGCST_C10,
/* 2 */ RSC(BOX_PKG_STATE_LIMIT_C9).CODE(), stateAttr[0], BOXKEY_PKGCST_C9,
/* 3 */ RSC(BOX_PKG_STATE_LIMIT_C8).CODE(), stateAttr[0], BOXKEY_PKGCST_C8,
/* 4 */ RSC(BOX_PKG_STATE_LIMIT_C7S).CODE(),stateAttr[0], BOXKEY_PKGCST_C7S,
/* 5 */ RSC(BOX_PKG_STATE_LIMIT_C7).CODE(), stateAttr[0], BOXKEY_PKGCST_C7,
/* 6 */ RSC(BOX_PKG_STATE_LIMIT_C6R).CODE(),stateAttr[0], BOXKEY_PKGCST_C6R,
/* 7 */ RSC(BOX_PKG_STATE_LIMIT_C6).CODE(), stateAttr[0], BOXKEY_PKGCST_C6,
/* 8 */ RSC(BOX_PKG_STATE_LIMIT_C4).CODE(), stateAttr[0], BOXKEY_PKGCST_C4,
/* 9 */ RSC(BOX_PKG_STATE_LIMIT_C3).CODE(), stateAttr[0], BOXKEY_PKGCST_C3,
/*10 */ RSC(BOX_PKG_STATE_LIMIT_C2).CODE(), stateAttr[0], BOXKEY_PKGCST_C2,
/*11 */ RSC(BOX_PKG_STATE_LIMIT_C1).CODE(), stateAttr[0], BOXKEY_PKGCST_C1,
/*12 */ RSC(BOX_PKG_STATE_LIMIT_C0).CODE(), stateAttr[0], BOXKEY_PKGCST_C0);

      if (wBox != NULL)
      {
       if(RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.CStateLimit != _UNSPEC)
       {
		TCellAt(wBox, 0, select.row).attr[11] = \
		TCellAt(wBox, 0, select.row).attr[12] = \
		TCellAt(wBox, 0, select.row).attr[13] = \
		TCellAt(wBox, 0, select.row).attr[14] = \
		TCellAt(wBox, 0, select.row).attr[15] = \
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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Query.IORedir == 1
	);
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 9
	}, select = {
		.col = 0,
		.row = isIORedir ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_IO_MWAIT_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE() , blankAttr, SCANKEY_NULL,
			RSC(BOX_IO_MWAIT_DESC).CODE(),descAttr, SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE() , blankAttr, SCANKEY_NULL,
			stateStr[1][isIORedir] , stateAttr[isIORedir],
							BOXKEY_IOMWAIT_ON,
			stateStr[0][!isIORedir], stateAttr[!isIORedir],
							BOXKEY_IOMWAIT_OFF,
			RSC(BOX_BLANK_DESC).CODE() , blankAttr, SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_IOMWAIT_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_IO_MWAIT );
	}
    break;

    case BOXKEY_IOMWAIT_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		[   _C0 ] =  6,
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
		.col = (Draw.Size.width - (44 - 17)) / 2,
		.row = TOP_HEADER_ROW + 4
	}, select = {
	.col = 0,
	.row = (CUINT) (
		CST[RO(Shm)->Cpu[
			RO(Shm)->Proc.Service.Core
		].Query.CStateInclude] != -1 ? \
					CST[RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Core
					].Query.CStateInclude] : 0 )
	};
	Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_MWAIT_MAX_STATE_TITLE).CODE(),
	/* 0 */ RSC(BOX_STATE_UNSPECIFIED).CODE(), stateAttr[0], SCANKEY_NULL,
	/* 1 */ RSC(BOX_STATE_C8).CODE(), stateAttr[0], BOXKEY_IORCST_C8,
	/* 2 */ RSC(BOX_STATE_C7).CODE(), stateAttr[0], BOXKEY_IORCST_C7,
	/* 3 */ RSC(BOX_STATE_C6).CODE(), stateAttr[0], BOXKEY_IORCST_C6,
	/* 4 */ RSC(BOX_STATE_C4).CODE(), stateAttr[0], BOXKEY_IORCST_C4,
	/* 5 */ RSC(BOX_STATE_C3).CODE(), stateAttr[0], BOXKEY_IORCST_C3,
	/* 6 */ RSC(BOX_STATE_C0).CODE(), stateAttr[0], SCANKEY_NULL);

      if (wBox != NULL)
      {
	if ( RO(Shm)->Cpu[
			RO(Shm)->Proc.Service.Core
		].Query.CStateInclude != _UNSPEC )
	{
		TCellAt(wBox, 0, select.row).attr[11] = \
		TCellAt(wBox, 0, select.row).attr[12] = \
		TCellAt(wBox, 0, select.row).attr[13] = \
		TCellAt(wBox, 0, select.row).attr[14] = \
		TCellAt(wBox, 0, select.row).attr[15] = \
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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 6
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.ODCM ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_ODCM_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_ODCM_DESC).CODE() , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.ODCM],
				stateAttr[RO(Shm)->Proc.Technology.ODCM],
								BOXKEY_ODCM_ON,
			stateStr[0][!RO(Shm)->Proc.Technology.ODCM],
				stateAttr[!RO(Shm)->Proc.Technology.ODCM],
								BOXKEY_ODCM_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_ODCM_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_ODCM );
	}
    break;

    case BOXKEY_ODCM_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
	const CSINT maxCM = 7 << RO(Shm)->Cpu[
					RO(Shm)->Proc.Service.Core
				].PowerThermal.DutyCycle.Extended;
	const Coordinate origin = {
		.col = (Draw.Size.width - 27) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = (
			RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.DutyCycle.ClockMod <= maxCM) ?
			RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.DutyCycle.ClockMod : 1
	};
	Window *wBox = NULL;

    if(RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].PowerThermal.DutyCycle.Extended)
      {
	wBox = CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_EXT_DUTY_CYCLE_TITLE).CODE(),
	    RSC(BOX_DUTY_CYCLE_RESERVED).CODE(), blankAttr ,  BOXKEY_ODCM_DC00,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT1).CODE(), stateAttr[0],BOXKEY_ODCM_DC01,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT2).CODE(), stateAttr[0],BOXKEY_ODCM_DC02,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT3).CODE(), stateAttr[0],BOXKEY_ODCM_DC03,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT4).CODE(), stateAttr[0],BOXKEY_ODCM_DC04,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT5).CODE(), stateAttr[0],BOXKEY_ODCM_DC05,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT6).CODE(), stateAttr[0],BOXKEY_ODCM_DC06,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT7).CODE(), stateAttr[0],BOXKEY_ODCM_DC07,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT8).CODE(), stateAttr[0],BOXKEY_ODCM_DC08,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT9).CODE(), stateAttr[0],BOXKEY_ODCM_DC09,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT10).CODE(),stateAttr[0],BOXKEY_ODCM_DC10,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT11).CODE(),stateAttr[0],BOXKEY_ODCM_DC11,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT12).CODE(),stateAttr[0],BOXKEY_ODCM_DC12,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT13).CODE(),stateAttr[0],BOXKEY_ODCM_DC13,
	    RSC(BOX_EXT_DUTY_CYCLE_PCT14).CODE(),stateAttr[0],BOXKEY_ODCM_DC14);
      } else {
	wBox = CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_DUTY_CYCLE_TITLE).CODE(),
		RSC(BOX_DUTY_CYCLE_RESERVED).CODE(),blankAttr,BOXKEY_ODCM_DC00,
		RSC(BOX_DUTY_CYCLE_PCT1).CODE(), stateAttr[0],BOXKEY_ODCM_DC01,
		RSC(BOX_DUTY_CYCLE_PCT2).CODE(), stateAttr[0],BOXKEY_ODCM_DC02,
		RSC(BOX_DUTY_CYCLE_PCT3).CODE(), stateAttr[0],BOXKEY_ODCM_DC03,
		RSC(BOX_DUTY_CYCLE_PCT4).CODE(), stateAttr[0],BOXKEY_ODCM_DC04,
		RSC(BOX_DUTY_CYCLE_PCT5).CODE(), stateAttr[0],BOXKEY_ODCM_DC05,
		RSC(BOX_DUTY_CYCLE_PCT6).CODE(), stateAttr[0],BOXKEY_ODCM_DC06,
		RSC(BOX_DUTY_CYCLE_PCT7).CODE(), stateAttr[0],BOXKEY_ODCM_DC07);
      }
      if (wBox != NULL) {
	TCellAt(wBox, 0, select.row).attr[ 8] = \
	TCellAt(wBox, 0, select.row).attr[ 9] = \
	TCellAt(wBox, 0, select.row).attr[10] = \
	TCellAt(wBox, 0, select.row).attr[11] = \
	TCellAt(wBox, 0, select.row).attr[12] = \
	TCellAt(wBox, 0, select.row).attr[13] = \
	TCellAt(wBox, 0, select.row).attr[14] = \
	TCellAt(wBox, 0, select.row).attr[15] = \
	TCellAt(wBox, 0, select.row).attr[16] = \
	TCellAt(wBox, 0, select.row).attr[17] = \
	TCellAt(wBox, 0, select.row).attr[18] = \
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
		.col = (Draw.Size.width - (2 + RSZ(BOX_POWER_POLICY_LOW))) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = RO(Shm)->Cpu[
				RO(Shm)->Proc.Service.Core
			].PowerThermal.PowerPolicy
	};
	Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_POWER_POLICY_TITLE).CODE(),
		RSC(BOX_POWER_POLICY_LOW).CODE(),stateAttr[0],BOXKEY_PWR_POL00,
		RSC(BOX_POWER_POLICY_1).CODE(), stateAttr[0], BOXKEY_PWR_POL01,
		RSC(BOX_POWER_POLICY_2).CODE(), stateAttr[0], BOXKEY_PWR_POL02,
		RSC(BOX_POWER_POLICY_3).CODE(), stateAttr[0], BOXKEY_PWR_POL03,
		RSC(BOX_POWER_POLICY_4).CODE(), stateAttr[0], BOXKEY_PWR_POL04,
		RSC(BOX_POWER_POLICY_5).CODE(), stateAttr[0], BOXKEY_PWR_POL05,
		RSC(BOX_POWER_POLICY_6).CODE(), stateAttr[0], BOXKEY_PWR_POL06,
		RSC(BOX_POWER_POLICY_7).CODE(), stateAttr[0], BOXKEY_PWR_POL07,
		RSC(BOX_POWER_POLICY_8).CODE(), stateAttr[0], BOXKEY_PWR_POL08,
		RSC(BOX_POWER_POLICY_9).CODE(), stateAttr[0], BOXKEY_PWR_POL09,
		RSC(BOX_POWER_POLICY_10).CODE(),stateAttr[0], BOXKEY_PWR_POL10,
		RSC(BOX_POWER_POLICY_11).CODE(),stateAttr[0], BOXKEY_PWR_POL11,
		RSC(BOX_POWER_POLICY_12).CODE(),stateAttr[0], BOXKEY_PWR_POL12,
		RSC(BOX_POWER_POLICY_13).CODE(),stateAttr[0], BOXKEY_PWR_POL13,
		RSC(BOX_POWER_POLICY_14).CODE(),stateAttr[0], BOXKEY_PWR_POL14,
	      RSC(BOX_POWER_POLICY_HIGH).CODE(),stateAttr[0], BOXKEY_PWR_POL15);

	if (wBox != NULL) {
		TCellAt(wBox, 0, select.row).attr[ 8] = \
		TCellAt(wBox, 0, select.row).attr[ 9] = \
		TCellAt(wBox, 0, select.row).attr[10] = \
		TCellAt(wBox, 0, select.row).attr[11] = \
		TCellAt(wBox, 0, select.row).attr[12] = \
		TCellAt(wBox, 0, select.row).attr[13] = \
		TCellAt(wBox, 0, select.row).attr[14] = \
		TCellAt(wBox, 0, select.row).attr[15] = \
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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				newPolicy,
				TECHNOLOGY_POWER_POLICY );
	}
    }
    break;

    case BOXKEY_HWP_EPP:
    {
	CPU_STRUCT *SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core];
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - (2 + RSZ(BOX_POWER_POLICY_LOW))) / 2,
		.row = TOP_HEADER_ROW + 3
	}, select = {
		.col = 0,
		.row = SProc->PowerThermal.HWP.Request.Energy_Pref == 0xff ?
		   8 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0xe0 ?
		   7 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0xc0 ?
		   6 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0xa0 ?
		   5 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0x80 ?
		   4 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0x60 ?
		   3 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0x40 ?
		   2 : SProc->PowerThermal.HWP.Request.Energy_Pref >= 0x20 ?
		   1 : 0
	};
	Window *wBox = CreateBox(scan->key, origin, select,
				(char*) RSC(BOX_POWER_POLICY_TITLE).CODE(),
	/*00*/RSC(BOX_HWP_POLICY_MIN).CODE(), stateAttr[0], BOXKEY_HWP_EPP_MIN,
	/*20*/RSC(BOX_HWP_POLICY_020).CODE(), stateAttr[0], BOXKEY_HWP_EPP_020,
	/*40*/RSC(BOX_HWP_POLICY_040).CODE(), stateAttr[0], BOXKEY_HWP_EPP_040,
	/*60*/RSC(BOX_HWP_POLICY_060).CODE(), stateAttr[0], BOXKEY_HWP_EPP_060,
	/*80*/RSC(BOX_HWP_POLICY_MED).CODE(), stateAttr[0], BOXKEY_HWP_EPP_MED,
	/*A0*/RSC(BOX_HWP_POLICY_0A0).CODE(), stateAttr[0], BOXKEY_HWP_EPP_0A0,
	/*C0*/RSC(BOX_HWP_POLICY_PWR).CODE(), stateAttr[0], BOXKEY_HWP_EPP_PWR,
	/*E0*/RSC(BOX_HWP_POLICY_0E0).CODE(), stateAttr[0], BOXKEY_HWP_EPP_0E0,
	/*FF*/RSC(BOX_HWP_POLICY_MAX).CODE(), stateAttr[0], BOXKEY_HWP_EPP_MAX);

	if (wBox != NULL) {
		TCellAt(wBox, 0, select.row).attr[ 3] = \
		TCellAt(wBox, 0, select.row).attr[ 4] = \
		TCellAt(wBox, 0, select.row).attr[ 5] = \
		TCellAt(wBox, 0, select.row).attr[ 6] = \
		TCellAt(wBox, 0, select.row).attr[ 7] = \
		TCellAt(wBox, 0, select.row).attr[ 8] = \
		TCellAt(wBox, 0, select.row).attr[ 9] = \
		TCellAt(wBox, 0, select.row).attr[10] = \
		TCellAt(wBox, 0, select.row).attr[11] = \
		TCellAt(wBox, 0, select.row).attr[12] = \
		TCellAt(wBox, 0, select.row).attr[13] = \
		TCellAt(wBox, 0, select.row).attr[14] = \
		TCellAt(wBox, 0, select.row).attr[15] = \
		TCellAt(wBox, 0, select.row).attr[16] = \
		TCellAt(wBox, 0, select.row).attr[17] = \
		TCellAt(wBox, 0, select.row).attr[18] = \
		TCellAt(wBox, 0, select.row).attr[19] = \
		TCellAt(wBox, 0, select.row).attr[20] = \
		TCellAt(wBox, 0, select.row).attr[21] = stateAttr[1];
		TCellAt(wBox, 0, select.row).item[ 3] = '<';
		TCellAt(wBox, 0, select.row).item[21] = '>';

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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x0,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_020:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x20,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_040:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x40,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_060:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x60,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_MED:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0x80,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_0A0:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0xa0,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_PWR:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0xc0,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_0E0:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0xe0,
				TECHNOLOGY_HWP_EPP );
	}
    break;

    case BOXKEY_HWP_EPP_MAX:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
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
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 11
	}, select = {
		.col = 0,
		.row = 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
		    (  (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		    || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) ) ?
				(char*) RSC(BOX_CPPC_TITLE).CODE()
			:	(char*) RSC(BOX_HWP_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_HWP_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Features.HWP_Enable],
				stateAttr[RO(Shm)->Proc.Features.HWP_Enable],
								BOXKEY_HWP_ON,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_HWP_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_HWP );
	}
    break;

    case BOXKEY_FMW_CPPC:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 14
	}, select = {
		.col = 0,
		.row = 2
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_CPPC_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_FMW_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][0], stateAttr[0], BOXKEY_HWP_ON,
			stateStr[0][0], stateAttr[0], BOXKEY_FMW_CPPC_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_FMW_CPPC_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_HWP );
	}
    break;

    case BOXKEY_HDC:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 12
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Features.HDC_Enable ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_HDC_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_HDC_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Features.HDC_Enable],
				stateAttr[RO(Shm)->Proc.Features.HDC_Enable],
								BOXKEY_HDC_ON,
			stateStr[0][!RO(Shm)->Proc.Features.HDC_Enable],
				stateAttr[!RO(Shm)->Proc.Features.HDC_Enable],
								BOXKEY_HDC_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
		SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_HDC_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_HDC );
	}
    break;

    case BOXKEY_HDC_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_HDC );
	}
    break;

    case BOXKEY_EEO:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 11
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Features.EEO_Enable ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_EEO_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_EEO_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Features.EEO_Enable],
				stateAttr[RO(Shm)->Proc.Features.EEO_Enable],
								BOXKEY_EEO_ON,
			stateStr[0][!RO(Shm)->Proc.Features.EEO_Enable],
				stateAttr[!RO(Shm)->Proc.Features.EEO_Enable],
								BOXKEY_EEO_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_EEO_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_EEO );
	}
    break;

    case BOXKEY_EEO_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_EEO );
	}
    break;

    case BOXKEY_R2H:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 13
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Features.R2H_Enable ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_R2H_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_R2H_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Features.R2H_Enable],
				stateAttr[RO(Shm)->Proc.Features.R2H_Enable],
								BOXKEY_R2H_ON,
			stateStr[0][!RO(Shm)->Proc.Features.R2H_Enable],
				stateAttr[!RO(Shm)->Proc.Features.R2H_Enable],
								BOXKEY_R2H_OFF,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_R2H_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_R2H );
	}
    break;

    case BOXKEY_R2H_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_R2H );
	}
    break;

    case BOXKEY_CFG_TDP_LVL:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_CFG_TDP_BLANK)) / 2,
		.row = TOP_HEADER_ROW + 11
	}, select = {
		.col = 0,
		.row = 3 + RO(Shm)->Proc.Features.TDP_Cfg_Level
	};
	Window *wBox;
	wBox = CreateBox(scan->key, origin, select,
			(char *) RSC(BOX_CFG_TDP_TITLE).CODE(),
		RSC(BOX_CFG_TDP_BLANK).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_CFG_TDP_DESC).CODE() , descAttr , SCANKEY_NULL,
		RSC(BOX_CFG_TDP_BLANK).CODE(), blankAttr, SCANKEY_NULL,
		RSC(BOX_CFG_TDP_LVL0).CODE() , stateAttr[0],BOXKEY_CFG_TDP_LVL0,
		RSC(BOX_CFG_TDP_LVL1).CODE() , stateAttr[0],BOXKEY_CFG_TDP_LVL1,
		RSC(BOX_CFG_TDP_LVL2).CODE() , stateAttr[0],BOXKEY_CFG_TDP_LVL2,
		RSC(BOX_CFG_TDP_BLANK).CODE(), blankAttr, SCANKEY_NULL);
	if (wBox != NULL) {
		TCellAt(wBox, 0, select.row).attr[ 3] = \
		TCellAt(wBox, 0, select.row).attr[ 4] = \
		TCellAt(wBox, 0, select.row).attr[ 5] = \
		TCellAt(wBox, 0, select.row).attr[ 6] = \
		TCellAt(wBox, 0, select.row).attr[ 7] = \
		TCellAt(wBox, 0, select.row).attr[ 8] = \
		TCellAt(wBox, 0, select.row).attr[ 9] = \
		TCellAt(wBox, 0, select.row).attr[10] = \
		TCellAt(wBox, 0, select.row).attr[11] = \
		TCellAt(wBox, 0, select.row).attr[12] = \
		TCellAt(wBox, 0, select.row).attr[13] = \
		TCellAt(wBox, 0, select.row).attr[14] = stateAttr[1];
		TCellAt(wBox, 0, select.row).item[ 3] = '<';
		TCellAt(wBox, 0, select.row).item[14] = '>';

		AppendWindow(wBox, &winList);
	}
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_CFG_TDP_LVL0:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				0,
				TECHNOLOGY_CFG_TDP_LVL );
	}
    break;

    case BOXKEY_CFG_TDP_LVL1:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				1,
				TECHNOLOGY_CFG_TDP_LVL );
	}
    break;

    case BOXKEY_CFG_TDP_LVL2:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				2,
				TECHNOLOGY_CFG_TDP_LVL );
	}
    break;

    case BOXKEY_WDT:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 15
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Technology.WDT ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_WDT_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_WDT_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Proc.Technology.WDT],
				stateAttr[RO(Shm)->Proc.Technology.WDT],
								BOXKEY_WDT_OFF,
			stateStr[0][!RO(Shm)->Proc.Technology.WDT],
				stateAttr[!RO(Shm)->Proc.Technology.WDT],
								BOXKEY_WDT_ON,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_WDT_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_WDT );
	}
    break;

    case BOXKEY_WDT_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_WDT );
	}
    break;

    case BOXKEY_HSMP:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 9
	}, select = {
		.col = 0,
		.row = RO(Shm)->Registration.HSMP ? 4 : 3
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_HSMP_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			RSC(BOX_HSMP_DESC).CODE()  , descAttr,	SCANKEY_NULL,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL,
			stateStr[1][RO(Shm)->Registration.HSMP],
				stateAttr[RO(Shm)->Registration.HSMP],
								BOXKEY_HSMP_OFF,
			stateStr[0][!RO(Shm)->Registration.HSMP],
				stateAttr[!RO(Shm)->Registration.HSMP],
								BOXKEY_HSMP_ON,
			RSC(BOX_BLANK_DESC).CODE(), blankAttr,	SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_HSMP_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_HSMP );
	}
    break;

    case BOXKEY_HSMP_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_HSMP );
	}
    break;

    case (BOXKEY_TDP_PKG	| PL1):
    case (BOXKEY_TDP_CORES	| PL1):
    case (BOXKEY_TDP_UNCORE	| PL1):
    case (BOXKEY_TDP_RAM	| PL1):
    case (BOXKEY_TDP_PLATFORM	| PL1):
	fallthrough;
    case (BOXKEY_TDP_PKG	| PL2):
    case (BOXKEY_TDP_CORES	| PL2):
    case (BOXKEY_TDP_UNCORE	| PL2):
    case (BOXKEY_TDP_RAM	| PL2):
    case (BOXKEY_TDP_PLATFORM	| PL2):
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = scan->key & (PL1 | PL2);
	const unsigned long long key[14] = {
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+50U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+10U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+05U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+04U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+03U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+02U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (+01U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-01U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-02U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-03U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-04U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-05U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-10U << 20),
		(BOXKEY_PLX_OP | (0x8ULL << pl)) | (pw << 5) | (-50U << 20)
	};
	const ASCII *title[] = {
		RSC(BOX_TDP_PKG_TITLE).CODE(),
		RSC(BOX_TDP_CORES_TITLE).CODE(),
		RSC(BOX_TDP_UNCORE_TITLE).CODE(),
		RSC(BOX_TDP_RAM_TITLE).CODE(),
		RSC(BOX_TDP_PLATFORM_TITLE).CODE()
	};
	const ASCII *descItem[PWR_LIMIT_SIZE] = {
		RSC(BOX_PL1_DESC).CODE(),
		RSC(BOX_PL2_DESC).CODE()
	};
	const ASCII *clampItem[2][2] = {
		{
			RSC(BOX_CLAMPING_OFF_COND0).CODE(),
			RSC(BOX_CLAMPING_OFF_COND1).CODE()
		},
		{
			RSC(BOX_CLAMPING_ON_COND0).CODE(),
			RSC(BOX_CLAMPING_ON_COND1).CODE()
		}
	};
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 1
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable ? 12:11
	};
	AppendWindow(
		CreateBox(scan->key, origin, select, (char*) title[pw],
			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,
			descItem[pl],			descAttr , SCANKEY_NULL,

			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,
			RSC(BOX_PWR_OFFSET_00).CODE(),	stateAttr[0], key[ 0],
			RSC(BOX_PWR_OFFSET_01).CODE(),	stateAttr[0], key[ 1],
			RSC(BOX_PWR_OFFSET_02).CODE(),	stateAttr[0], key[ 2],
			RSC(BOX_PWR_OFFSET_03).CODE(),	stateAttr[0], key[ 3],
			RSC(BOX_PWR_OFFSET_04).CODE(),	stateAttr[0], key[ 4],
			RSC(BOX_PWR_OFFSET_05).CODE(),	stateAttr[0], key[ 5],
			RSC(BOX_PWR_OFFSET_06).CODE(),	stateAttr[0], key[ 6],
			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,

		stateStr[1][RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable],
		stateAttr[RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable],
			(BOXKEY_PLX_OP | (0x8U << pl)) | ( pw << 5) | 1,

		stateStr[0][!RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable],
		stateAttr[!RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Enable],
			(BOXKEY_PLX_OP | (0x8U << pl)) | ( pw << 5) | 2,

			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,
			RSC(BOX_PWR_OFFSET_07).CODE(),	stateAttr[0], key[ 7],
			RSC(BOX_PWR_OFFSET_08).CODE(),	stateAttr[0], key[ 8],
			RSC(BOX_PWR_OFFSET_09).CODE(),	stateAttr[0], key[ 9],
			RSC(BOX_PWR_OFFSET_10).CODE(),	stateAttr[0], key[10],
			RSC(BOX_PWR_OFFSET_11).CODE(),	stateAttr[0], key[11],
			RSC(BOX_PWR_OFFSET_12).CODE(),	stateAttr[0], key[12],
			RSC(BOX_PWR_OFFSET_13).CODE(),	stateAttr[0], key[13],
			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,

	      clampItem[1][RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Clamping],
		stateAttr[RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Clamping],
			(BOXKEY_PLX_OP | (0x8U << pl)) | (pw << 5) | 5,

	     clampItem[0][!RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Clamping],
		stateAttr[!RO(Shm)->Proc.Power.Domain[pw].Feature[pl].Clamping],
			(BOXKEY_PLX_OP | (0x8U << pl)) | (pw << 5) | 6,

			RSC(BOX_BLANK_DESC).CODE(),	blankAttr,SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_PL1_PKG_LIM_ON:
    case BOXKEY_PL1_CORE_LIM_ON:
    case BOXKEY_PL1_UNCORE_LIM_ON:
    case BOXKEY_PL1_RAM_LIM_ON:
    case BOXKEY_PL1_PLT_LIM_ON:
	fallthrough;
    case BOXKEY_PL2_PKG_LIM_ON:
    case BOXKEY_PL2_CORE_LIM_ON:
    case BOXKEY_PL2_UNCORE_LIM_ON:
    case BOXKEY_PL2_RAM_LIM_ON:
    case BOXKEY_PL2_PLT_LIM_ON:
    {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = (scan->key & BOXKEY_PLX_MASK) >> 4;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_TDP_LIMITING,
				pw,
				pl );
	}
    }
    break;

    case BOXKEY_PL1_PKG_LIM_OFF:
    case BOXKEY_PL1_CORE_LIM_OFF:
    case BOXKEY_PL1_UNCORE_LIM_OFF:
    case BOXKEY_PL1_RAM_LIM_OFF:
    case BOXKEY_PL1_PLT_LIM_OFF:
	fallthrough;
    case BOXKEY_PL2_PKG_LIM_OFF:
    case BOXKEY_PL2_CORE_LIM_OFF:
    case BOXKEY_PL2_UNCORE_LIM_OFF:
    case BOXKEY_PL2_RAM_LIM_OFF:
    case BOXKEY_PL2_PLT_LIM_OFF:
    {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = (scan->key & BOXKEY_PLX_MASK) >> 4;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_TDP_LIMITING,
				pw,
				pl );
	}
    }
    break;

    case BOXKEY_PL1_PKG_CLAMP_ON:
    case BOXKEY_PL1_CORE_CLAMP_ON:
    case BOXKEY_PL1_UNCORE_CLAMP_ON:
    case BOXKEY_PL1_RAM_CLAMP_ON:
    case BOXKEY_PL1_PLT_CLAMP_ON:
	fallthrough;
    case BOXKEY_PL2_PKG_CLAMP_ON:
    case BOXKEY_PL2_CORE_CLAMP_ON:
    case BOXKEY_PL2_UNCORE_CLAMP_ON:
    case BOXKEY_PL2_RAM_CLAMP_ON:
    case BOXKEY_PL2_PLT_CLAMP_ON:
    {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = (scan->key & BOXKEY_PLX_MASK) >> 4;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_TDP_CLAMPING,
				pw,
				pl );
	}
    }
    break;

    case BOXKEY_PL1_PKG_CLAMP_OFF:
    case BOXKEY_PL1_CORE_CLAMP_OFF:
    case BOXKEY_PL1_UNCORE_CLAMP_OFF:
    case BOXKEY_PL1_RAM_CLAMP_OFF:
    case BOXKEY_PL1_PLT_CLAMP_OFF:
	fallthrough;
    case BOXKEY_PL2_PKG_CLAMP_OFF:
    case BOXKEY_PL2_CORE_CLAMP_OFF:
    case BOXKEY_PL2_UNCORE_CLAMP_OFF:
    case BOXKEY_PL2_RAM_CLAMP_OFF:
    case BOXKEY_PL2_PLT_CLAMP_OFF:
    {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = (scan->key & BOXKEY_PLX_MASK) >> 4;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_TDP_CLAMPING,
				pw,
				pl );
	}
    }
    break;

    case BOXKEY_TW1_PKG:
    case BOXKEY_TW1_CORES:
    case BOXKEY_TW1_UNCORE:
    case BOXKEY_TW1_RAM:
    case BOXKEY_TW1_PLATFORM:
	fallthrough;
    case BOXKEY_TW2_PKG:
    case BOXKEY_TW2_CORES:
    case BOXKEY_TW2_UNCORE:
    case BOXKEY_TW2_RAM:
    case BOXKEY_TW2_PLATFORM:
    {
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL) {
		AppendWindow(CreatePowerTimeWindow(scan->key), &winList);
	} else {
		SetHead(&winList, win);
	}
    }
    break;

    case BOXKEY_TDC:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	const Coordinate origin = {
		.col = (Draw.Size.width - RSZ(BOX_BLANK_DESC)) / 2,
		.row = TOP_HEADER_ROW + 2
	}, select = {
		.col = 0,
		.row = RO(Shm)->Proc.Power.Feature.TDC ? 12 : 11
	};
	const unsigned long long key[14] = {
		BOXKEY_TDC_MASK | (+50U << 20),
		BOXKEY_TDC_MASK | (+10U << 20),
		BOXKEY_TDC_MASK | (+05U << 20),
		BOXKEY_TDC_MASK | (+04U << 20),
		BOXKEY_TDC_MASK | (+03U << 20),
		BOXKEY_TDC_MASK | (+02U << 20),
		BOXKEY_TDC_MASK | (+01U << 20),
		BOXKEY_TDC_MASK | (-01U << 20),
		BOXKEY_TDC_MASK | (-02U << 20),
		BOXKEY_TDC_MASK | (-03U << 20),
		BOXKEY_TDC_MASK | (-04U << 20),
		BOXKEY_TDC_MASK | (-05U << 20),
		BOXKEY_TDC_MASK | (-10U << 20),
		BOXKEY_TDC_MASK | (-50U << 20)
	};
	AppendWindow(
		CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_TDC_TITLE).CODE(),
			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,
			RSC(BOX_TDC_DESC).CODE(),	descAttr , SCANKEY_NULL,

			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,
			RSC(BOX_AMP_OFFSET_00).CODE(),	stateAttr[0], key[ 0],
			RSC(BOX_AMP_OFFSET_01).CODE(),	stateAttr[0], key[ 1],
			RSC(BOX_AMP_OFFSET_02).CODE(),	stateAttr[0], key[ 2],
			RSC(BOX_AMP_OFFSET_03).CODE(),	stateAttr[0], key[ 3],
			RSC(BOX_AMP_OFFSET_04).CODE(),	stateAttr[0], key[ 4],
			RSC(BOX_AMP_OFFSET_05).CODE(),	stateAttr[0], key[ 5],
			RSC(BOX_AMP_OFFSET_06).CODE(),	stateAttr[0], key[ 6],
			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,

			stateStr[1][RO(Shm)->Proc.Power.Feature.TDC],
			stateAttr[RO(Shm)->Proc.Power.Feature.TDC],
			BOXKEY_TDC_OR | 1,

			stateStr[0][!RO(Shm)->Proc.Power.Feature.TDC],
			stateAttr[!RO(Shm)->Proc.Power.Feature.TDC],
			BOXKEY_TDC_OR | 2,

			RSC(BOX_BLANK_DESC).CODE(),	blankAttr, SCANKEY_NULL,
			RSC(BOX_AMP_OFFSET_07).CODE(),	stateAttr[0], key[ 7],
			RSC(BOX_AMP_OFFSET_08).CODE(),	stateAttr[0], key[ 8],
			RSC(BOX_AMP_OFFSET_09).CODE(),	stateAttr[0], key[ 9],
			RSC(BOX_AMP_OFFSET_10).CODE(),	stateAttr[0], key[10],
			RSC(BOX_AMP_OFFSET_11).CODE(),	stateAttr[0], key[11],
			RSC(BOX_AMP_OFFSET_12).CODE(),	stateAttr[0], key[12],
			RSC(BOX_AMP_OFFSET_13).CODE(),	stateAttr[0], key[13],
			RSC(BOX_BLANK_DESC).CODE(),	blankAttr,SCANKEY_NULL),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_TDC_ON:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_ON,
				TECHNOLOGY_TDC_LIMITING );
	}
    break;

    case BOXKEY_TDC_OFF:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				COREFREQ_TOGGLE_OFF,
				TECHNOLOGY_TDC_LIMITING );
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
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_MACHINE,
				newLim,
				MACHINE_LIMIT_IDLE );
	}
    }
    break;

    case BOXKEY_CLR_ALL_EVENTS:
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_CLEAR_EVENTS,
				EVENT_ALL_OF_THEM );
	}
    break;

    case BOXKEY_CLR_THM_SENSOR:
    case BOXKEY_CLR_PROCHOT_LOG:
    case BOXKEY_CLR_THM_CRIT:
    case BOXKEY_CLR_THM_THOLD1:
    case BOXKEY_CLR_THM_THOLD2:
    case BOXKEY_CLR_PWR_LIMIT:
    case BOXKEY_CLR_CUR_LIMIT:
    case BOXKEY_CLR_X_DOMAIN:
    case BOXKEY_CLR_CORE_HOT:
    case BOXKEY_CLR_CORE_THM:
    case BOXKEY_CLR_CORE_RES:
    case BOXKEY_CLR_CORE_AVG:
    case BOXKEY_CLR_CORE_VRT:
    case BOXKEY_CLR_CORE_TDC:
    case BOXKEY_CLR_CORE_PL1:
    case BOXKEY_CLR_CORE_PL2:
    case BOXKEY_CLR_CORE_EDP:
    case BOXKEY_CLR_CORE_BST:
    case BOXKEY_CLR_CORE_ATT:
    case BOXKEY_CLR_CORE_TVB:
    case BOXKEY_CLR_GFX_HOT:
    case BOXKEY_CLR_GFX_THM:
    case BOXKEY_CLR_GFX_AVG:
    case BOXKEY_CLR_GFX_VRT:
    case BOXKEY_CLR_GFX_TDC:
    case BOXKEY_CLR_GFX_PL1:
    case BOXKEY_CLR_GFX_PL2:
    case BOXKEY_CLR_GFX_EDP:
    case BOXKEY_CLR_GFX_EFF:
    case BOXKEY_CLR_RING_HOT:
    case BOXKEY_CLR_RING_THM:
    case BOXKEY_CLR_RING_AVG:
    case BOXKEY_CLR_RING_VRT:
    case BOXKEY_CLR_RING_TDC:
    case BOXKEY_CLR_RING_PL1:
    case BOXKEY_CLR_RING_PL2:
    case BOXKEY_CLR_RING_EDP:
    {
	const enum EVENT_LOG lshift = (scan->key & CLEAR_EVENT_MASK) >> 1;
	const enum THERM_PWR_EVENTS event = 0x1LLU << lshift;
      if (!RING_FULL(RW(Shm)->Ring[0])) {
	RING_WRITE(RW(Shm)->Ring[0], COREFREQ_IOCTL_CLEAR_EVENTS, event);
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
	if (win == NULL)
	{
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

    case BOXKEY_RATIO_ACTIVATION:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	CPU_STRUCT *SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core];
	struct FLIP_FLOP *CFlop = &SProc->FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];
	CLOCK_ARG clockMod  = {.ullong = scan->key};
	const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;

	signed int lowestShift, highestShift;
	ComputeRatioShifts(	SProc->Boost[BOOST(ACT)],
				0,
				MAXCLOCK_TO_RATIO(unsigned int,CFlop->Clock.Hz),
				&lowestShift,
				&highestShift );
	AppendWindow(
		CreateRatioClock(scan->key,
			SProc->Boost[BOOST(ACT)],
			-1,
			NC,
			lowestShift,
			highestShift,

		(int)	((SProc->Boost[BOOST(MIN)]
			+ RO(Shm)->Proc.Features.Factory.Ratio) >> 1),

		(int)	(RO(Shm)->Proc.Features.Factory.Ratio
			+ ((MAXCLOCK_TO_RATIO(unsigned int, CFlop->Clock.Hz)
			- RO(Shm)->Proc.Features.Factory.Ratio) >> 1)),

			BOXKEY_CFGTDP_CLOCK,
			TitleForRatioClock,
			35),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_UNCORE_CLOCK_MAX:
    {
	Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL)
      {
	CPU_STRUCT *SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core];
	struct FLIP_FLOP *CFlop = &SProc->FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];
	CLOCK_ARG clockMod  = {.ullong = scan->key};
	const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;

	signed int lowestShift, highestShift;
	ComputeRatioShifts(	RO(Shm)->Uncore.Boost[BOOST(MAX)],
				RO(Shm)->Uncore.Boost[BOOST(MIN)],
				MAXCLOCK_TO_RATIO(unsigned int,CFlop->Clock.Hz),
				&lowestShift,
				&highestShift );
	AppendWindow(
		CreateRatioClock(scan->key,
			RO(Shm)->Uncore.Boost[BOOST(MAX)],
			-1,
			NC,
			lowestShift,
			highestShift,

		(int)	((RO(Shm)->Uncore.Boost[BOOST(MIN)]
			+ RO(Shm)->Proc.Features.Factory.Ratio ) >> 1),

		(int)	(RO(Shm)->Proc.Features.Factory.Ratio
			+ ((MAXCLOCK_TO_RATIO(unsigned int, CFlop->Clock.Hz)
			- RO(Shm)->Proc.Features.Factory.Ratio) >> 1)),

			BOXKEY_UNCORE_CLOCK,
			TitleForUncoreClock,
			36),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case BOXKEY_UNCORE_CLOCK_MIN:
    {
      Window *win = SearchWinListById(scan->key, &winList);
      if (win == NULL) {
	CLOCK_ARG clockMod  = {.ullong = scan->key};
	const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;

	signed int lowestShift, highestShift;
	ComputeRatioShifts(	RO(Shm)->Uncore.Boost[BOOST(MIN)],
				1,
				RO(Shm)->Proc.Features.Factory.Ratio,
				&lowestShift,
				&highestShift );
	AppendWindow(
		CreateRatioClock(scan->key,
				RO(Shm)->Uncore.Boost[BOOST(MIN)],
				-1,
				NC,
				lowestShift,
				highestShift,

			(int)	((RO(Shm)->Uncore.Boost[BOOST(MIN)]
				+ RO(Shm)->Proc.Features.Factory.Ratio ) >> 1),

			(int)	(RO(Shm)->Proc.Features.Factory.Ratio - 1),

				BOXKEY_UNCORE_CLOCK,
				TitleForUncoreClock,
				37),
		&winList);
      } else {
	SetHead(&winList, win);
      }
    }
    break;

    case SCANKEY_F10:
    case BOXKEY_TOOLS_MACHINE:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE(	RW(Shm)->Ring[1],
			COREFREQ_ORDER_MACHINE,
			COREFREQ_TOGGLE_OFF);
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
	Window *wBox;
	wBox = CreateBox(scan->key, origin, select,
			(char*) RSC(BOX_TOOLS_TITLE).CODE(),
			RSC(BOX_TOOLS_STOP_BURN).CODE(),
				BITVAL(RW(Shm)->Proc.Sync, BURN) ?
				RSC(UI).ATTR()[UI_BOX_TOOLS_STOP] : blankAttr,
			BITVAL(RW(Shm)->Proc.Sync, BURN) ?
				BOXKEY_TOOLS_MACHINE : SCANKEY_NULL,
			RSC(BOX_TOOLS_ATOMIC_BURN).CODE(),stateAttr[0],
				BOXKEY_TOOLS_ATOMIC,
			RSC(BOX_TOOLS_CRC32_BURN).CODE() ,stateAttr[0],
				BOXKEY_TOOLS_CRC32,
			RSC(BOX_TOOLS_CONIC_BURN).CODE() ,stateAttr[0],
				BOXKEY_TOOLS_CONIC,
			RSC(BOX_TOOLS_RANDOM_CPU).CODE() ,stateAttr[0],
				BOXKEY_TOOLS_TURBO_RND,
			RSC(BOX_TOOLS_ROUND_ROBIN_CPU).CODE(), stateAttr[0],
				BOXKEY_TOOLS_TURBO_RR,
			RSC(BOX_TOOLS_USER_CPU).CODE(), stateAttr[0],
				BOXKEY_TOOLS_TURBO_CPU);

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
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE(	RW(Shm)->Ring[1],
			COREFREQ_ORDER_ATOMIC,
			RO(Shm)->Proc.Service.Core,
			COREFREQ_TOGGLE_ON);
      }
    break;

    case BOXKEY_TOOLS_CRC32:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE(	RW(Shm)->Ring[1],
			COREFREQ_ORDER_CRC32,
			RO(Shm)->Proc.Service.Core,
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
	Window *wBox;
	wBox = CreateBox(scan->key, origin, select,
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
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_ELLIPSOID,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_CONIC,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_CONIC1:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_HYPERBOLOID_ONE_SHEET,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_CONIC,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_CONIC2:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_HYPERBOLOID_TWO_SHEETS,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_CONIC,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_CONIC3:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_ELLIPTICAL_CYLINDER,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_CONIC,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_CONIC4:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_HYPERBOLIC_CYLINDER,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_CONIC,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_CONIC5:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	CONIC_TWO_PARALLEL_PLANES,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_CONIC,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_TURBO_RND:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	RAND_SMT,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_TURBO,
				RO(Shm)->Proc.Service.Core );
      }
    break;

    case BOXKEY_TOOLS_TURBO_RR:
      if (!RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	RR_SMT,
				RW(Shm)->Ring[1],
				COREFREQ_ORDER_TURBO,
				RO(Shm)->Proc.Service.Core );
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
		Draw.Flag.layout = 1;
	}
    break;

    case SCANKEY_ALT_p:
    case SCANCON_ALT_p:
	if (DumpStatus())
	{
		AbortDump();
	}
	else if (StartDump(	"corefreq_%llx.cast",
				(int) (Recorder.Reset - 1),
				DUMP_TO_JSON) == 0 )
	{
		Draw.Flag.layout = 1;
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
	RECORDER_COMPUTE(Recorder, RO(Shm)->Sleep.Interval);
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
      if ((scan->key & TRACK_TASK) && !RING_FULL(RW(Shm)->Ring[1])) {
	RING_WRITE_SUB_CMD(	TASK_TRACKING,
				RW(Shm)->Ring[1],
				COREFREQ_TASK_MONITORING,
			(pid_t) scan->key & TRACK_MASK );
      }
      else if (scan->key & SMBIOS_STRING_INDEX)
      {
	if (Draw.Disposal == D_MAINVIEW) {
		enum SMB_STRING usrIdx = scan->key & SMBIOS_STRING_MASK;
		if (usrIdx < SMB_STRING_COUNT) {
			Draw.SmbIndex = usrIdx;
			Draw.Flag.layout = 1;
		}
	}
      }
      else if (scan->key & CPU_ONLINE)
      {
	const unsigned long cpu = scan->key & CPU_MASK;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(RW(Shm)->Ring[0], COREFREQ_IOCTL_CPU_ON, cpu);
	}
      }
      else if (scan->key & CPU_OFFLINE)
      {
	const unsigned long cpu = scan->key & CPU_MASK;
	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(RW(Shm)->Ring[0], COREFREQ_IOCTL_CPU_OFF, cpu);
	}
      }
      else if (scan->key & CPU_SELECT)
      {
	const unsigned short cpu = scan->key & CPU_MASK;
	if (!RING_FULL(RW(Shm)->Ring[1])) {
		RING_WRITE_SUB_CMD(	USR_CPU,
					RW(Shm)->Ring[1],
					COREFREQ_ORDER_TURBO,
					cpu );
	}
      }
      else if ((scan->key & BOXKEY_PLX_OP) == BOXKEY_PLX_OP)
      {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = (scan->key & BOXKEY_PLX_MASK) >> 4;
	const unsigned short offset= (scan->key & BOXKEY_TDP_OFFSET) >> 20;

	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				offset,
				TECHNOLOGY_TDP_OFFSET,
				pw,
				pl );
	}
      }
      else if ((scan->key & BOXKEY_TDC_MASK) == BOXKEY_TDC_MASK)
      {
	const unsigned short offset = (scan->key & BOXKEY_TDC_OFFSET) >> 20;

	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				offset,
				TECHNOLOGY_TDC_OFFSET );
	}
      }
      else if ((scan->key & BOXKEY_TW_OP) == BOXKEY_TW_OP)
      {
	const enum PWR_DOMAIN	pw = (scan->key >> 5) & BOXKEY_TDP_MASK;
	const enum PWR_LIMIT	pl = (scan->key & BOXKEY_PLX_MASK) >> 4;
	const unsigned short	tw = (scan->key & BOXKEY_TDP_OFFSET) >> 20;

	if (!RING_FULL(RW(Shm)->Ring[0])) {
		RING_WRITE(	RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TECHNOLOGY,
				tw,
				TECHNOLOGY_TW_POWER,
				pw,
				pl );
	}
      }
      else
      {
	switch (scan->key & (~BOXKEY_RATIO_SELECT_OR ^ RATIO_MASK)) {
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
	  if (RO(Shm)->Proc.Features.Turbo_Unlock)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		struct FLIP_FLOP *CFlop;
		CLOCK_ARG clockMod = {.ullong = scan->key};
		unsigned int COF;
		const unsigned int lowestOperating = 1;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		const enum RATIO_BOOST boost = BOOST(SIZE) - NC;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	    if (cpu == -1) {
		COF = RO(Shm)->Cpu[ Ruler.Top[boost] ].Boost[boost];
		CFlop = &RO(Shm)->Cpu[ Ruler.Top[boost] ].FlipFlop[
				!RO(Shm)->Cpu[ Ruler.Top[boost] ].Toggle
			];
	    } else {
		COF = RO(Shm)->Cpu[cpu].Boost[boost];
		CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
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

			(int)	((lowestOperating
				+ RO(Shm)->Proc.Features.Factory.Ratio) >> 1),

			(int)	(RO(Shm)->Proc.Features.Factory.Ratio
			    + ((MAXCLOCK_TO_RATIO(unsigned int, CFlop->Clock.Hz)
				- RO(Shm)->Proc.Features.Factory.Ratio) >> 1)),

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
	  if (RO(Shm)->Proc.Features.TgtRatio_Unlock)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	   if (win == NULL)
	   {
		CLOCK_ARG clockMod = {.ullong = scan->key};
		unsigned int COF, lowestOperating, highestOperating;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	    if (cpu == -1) {
		COF = RO(Shm)->Cpu[ Ruler.Top[BOOST(TGT)] ].Boost[BOOST(TGT)];
		lowestOperating = RO(Shm)->Cpu[
						Ruler.Top[ BOOST(TGT) ]
					].Boost[ BOOST(MIN) ];
		highestOperating = RO(Shm)->Cpu[
						Ruler.Top[ BOOST(TGT) ]
					].Boost[ BOOST(MAX) ];
	    } else {
		COF = RO(Shm)->Cpu[cpu].Boost[BOOST(TGT)];
		lowestOperating = RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)];
		highestOperating = RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)];
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

				(int)	((lowestOperating
					+ highestOperating) >> 1),

			(int)	(CUMIN( RO(Shm)->Proc.Features.Factory.Ratio,
					GetTopOfRuler() )),

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
	  if ((RO(Shm)->Proc.Features.HWP_Enable == 1)
	   || (RO(Shm)->Proc.Features.ACPI_CPPC == 1))
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod = {.ullong = scan->key};
		struct HWP_STRUCT *pHWP;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	      if (cpu == -1) {
		pHWP = &RO(Shm)->Cpu[
				Ruler.Top[BOOST(HWP_TGT)]
			].PowerThermal.HWP;
		COF = pHWP->Request.Desired_Perf;
	      } else {
		pHWP = &RO(Shm)->Cpu[cpu].PowerThermal.HWP;
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

				(int)	((pHWP->Capabilities.Most_Efficient
					+ pHWP->Capabilities.Guaranteed) >> 1),

				(int)	pHWP->Capabilities.Guaranteed,
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
	  if ((RO(Shm)->Proc.Features.HWP_Enable == 1)
	   || (RO(Shm)->Proc.Features.ACPI_CPPC == 1))
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod = {.ullong = scan->key};
		struct HWP_STRUCT *pHWP;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	      if (cpu == -1) {
		pHWP = &RO(Shm)->Cpu[
				Ruler.Top[BOOST(HWP_MAX)]
			].PowerThermal.HWP;
		COF = pHWP->Request.Maximum_Perf;
	      } else {
		pHWP = &RO(Shm)->Cpu[cpu].PowerThermal.HWP;
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

				(int)	((pHWP->Capabilities.Most_Efficient
					+ pHWP->Capabilities.Guaranteed) >> 1),

				(int)	pHWP->Capabilities.Guaranteed,
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
	  if ((RO(Shm)->Proc.Features.HWP_Enable == 1)
	   || (RO(Shm)->Proc.Features.ACPI_CPPC == 1))
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod = {.ullong = scan->key};
		struct HWP_STRUCT *pHWP;
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	      if (cpu == -1) {
		pHWP = &RO(Shm)->Cpu[
				Ruler.Top[BOOST(HWP_MIN)]
			].PowerThermal.HWP;
		COF = pHWP->Request.Minimum_Perf;
	      } else {
		pHWP = &RO(Shm)->Cpu[cpu].PowerThermal.HWP;
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

				(int)	((pHWP->Capabilities.Most_Efficient
					+ pHWP->Capabilities.Guaranteed) >> 1),

				(int)	pHWP->Capabilities.Guaranteed,
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
	  if ((RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b10) == 0b10)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		struct FLIP_FLOP *CFlop;
		CLOCK_ARG clockMod  = {.ullong = scan->key};
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF, lowestOperating;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	      if (cpu == -1) {
		COF = RO(Shm)->Cpu[ Ruler.Top[BOOST(MAX)] ].Boost[BOOST(MAX)];

		lowestOperating = KMIN( RO(Shm)->Cpu[
						Ruler.Top[BOOST(MAX)]
					].Boost[BOOST(MIN)],
					RO(Shm)->Proc.Features.Factory.Ratio );

		CFlop = &RO(Shm)->Cpu[Ruler.Top[ BOOST(MAX)] ].FlipFlop[
					!RO(Shm)->Cpu[Ruler.Top[BOOST(MAX)]
					].Toggle];
	      } else {
		COF = RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)];
		lowestOperating = KMIN( RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)],
					RO(Shm)->Proc.Features.Factory.Ratio );
		CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
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

			(int)	((lowestOperating
				+ RO(Shm)->Proc.Features.Factory.Ratio) >> 1),

			(int)	(RO(Shm)->Proc.Features.Factory.Ratio
			    + ((MAXCLOCK_TO_RATIO(unsigned int, CFlop->Clock.Hz)
				- RO(Shm)->Proc.Features.Factory.Ratio) >> 1)),

				BOXKEY_RATIO_CLOCK,
				TitleForRatioClock,
				36), &winList);
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;

	case BOXKEY_RATIO_CLOCK_MIN:
	  if ((RO(Shm)->Proc.Features.ClkRatio_Unlock & 0b01) == 0b01)
	  {
		Window *win = SearchWinListById(scan->key, &winList);
	    if (win == NULL)
	    {
		CLOCK_ARG clockMod  = {.ullong = scan->key};
		const unsigned int NC = clockMod.NC & CLOCKMOD_RATIO_MASK;
		unsigned int COF, lowestOperating;
		signed int lowestShift, highestShift;

		const signed short cpu = (signed short) (
			(scan->key & RATIO_MASK) ^ CORE_COUNT
		);
	      if (cpu == -1) {
		COF = RO(Shm)->Cpu[ Ruler.Top[BOOST(MIN)] ].Boost[BOOST(MIN)];
		lowestOperating = KMIN( RO(Shm)->Cpu[
						Ruler.Top[BOOST(MIN)]
					].Boost[BOOST(MIN)],
					RO(Shm)->Proc.Features.Factory.Ratio );
	      } else {
		COF = RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)];
		lowestOperating = KMIN( RO(Shm)->Cpu[cpu].Boost[BOOST(MIN)],
					RO(Shm)->Proc.Features.Factory.Ratio );
	      }
		ComputeRatioShifts(COF,
				0,
				RO(Shm)->Proc.Features.Factory.Ratio,
				&lowestShift,
				&highestShift);

		AppendWindow(
			CreateRatioClock(scan->key,
				COF,
				cpu,
				NC,
				lowestShift,
				highestShift,

			(int)	((lowestOperating
				+ RO(Shm)->Proc.Features.Factory.Ratio) >> 1),

			(int)	(RO(Shm)->Proc.Features.Factory.Ratio - 1),

				BOXKEY_RATIO_CLOCK,
				TitleForRatioClock,
				37), &winList);
	    } else {
		SetHead(&winList, win);
	    }
	  }
	  break;

	default: {
		CLOCK_ARG clockMod = {.ullong = scan->key};
	    if (clockMod.NC & BOXKEY_TURBO_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(RW(Shm)->Ring[0])) {
		    RING_WRITE( RW(Shm)->Ring[0],
				COREFREQ_IOCTL_TURBO_CLOCK, clockMod.ullong );
		}
	    }
	    else if (clockMod.NC & BOXKEY_RATIO_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(RW(Shm)->Ring[0])) {
		    RING_WRITE( RW(Shm)->Ring[0],
				COREFREQ_IOCTL_RATIO_CLOCK, clockMod.ullong );
		}
	    }
	    else if (clockMod.NC & BOXKEY_CFGTDP_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(RW(Shm)->Ring[0])) {
		    RING_WRITE( RW(Shm)->Ring[0],
				COREFREQ_IOCTL_CONFIG_TDP, clockMod.ullong );
		}
	    }
	    else if (clockMod.NC & BOXKEY_UNCORE_CLOCK)
	    {
			clockMod.NC &= CLOCKMOD_RATIO_MASK;

		if (!RING_FULL(RW(Shm)->Ring[0])) {
		    RING_WRITE( RW(Shm)->Ring[0],
				COREFREQ_IOCTL_UNCORE_CLOCK, clockMod.ullong );
		}
	    } else {
		return -1;
	    }
	  }
	  break;
	}
      }
    break;
    }
	return 0;
}

#ifndef NO_FOOTER
void PrintTaskMemory(Layer *layer, CUINT row,
			unsigned short procCount,
			unsigned long freeRAM,
			unsigned long totalRAM)
{
	StrFormat(Buffer, 6+20+20+1,
			"%6u" "%9lu" "%-9lu",
			procCount,
			freeRAM >> Draw.Unit.Memory,
			totalRAM >> Draw.Unit.Memory);

	memcpy(&LayerAt(layer, code, Draw.Area.Footer.TaskMem.Total, row),
			&Buffer[0], 6);

	memcpy(&LayerAt(layer, code, Draw.Area.Footer.TaskMem.Free, row),
			&Buffer[6], 9);

	memcpy(&LayerAt(layer, code, Draw.Area.Footer.TaskMem.Count, row),
			&Buffer[15], 9);
}
#endif

#ifndef NO_HEADER
void Layout_Header(Layer *layer, CUINT row)
{
	int len;
	const CUINT	lProc0 = RSZ(LAYOUT_HEADER_PROC),
			xProc0 = 12,
			lProc1 = RSZ(LAYOUT_HEADER_CPU),
			xProc1 = Draw.Size.width - lProc1,
			lArch0 = RSZ(LAYOUT_HEADER_ARCH),
			xArch0 = 12,
			lArch1 = RSZ(LAYOUT_HEADER_CACHE_L1),
			xArch1 = Draw.Size.width-lArch1,
			lBClk0 = RSZ(LAYOUT_HEADER_BCLK),
			xBClk0 = 12,
			lArch2 = RSZ(LAYOUT_HEADER_CACHES),
			xArch2 = Draw.Size.width-lArch2;

	PrintLCD(layer, 0, row, RSZ(LAYOUT_LCD_RESET),
		(char *) RSC(LAYOUT_LCD_RESET).CODE(),
		RSC(UI).ATTR()[UI_LAYOUT_LCD_RESET]);

	LayerDeclare(LAYOUT_HEADER_PROC, lProc0, xProc0, row, hProc0);
	LayerDeclare(LAYOUT_HEADER_CPU , lProc1, xProc1, row, hProc1);

	row++;

	LayerDeclare(LAYOUT_HEADER_ARCH, lArch0, xArch0, row, hArch0);
	LayerDeclare(LAYOUT_HEADER_CACHE_L1,lArch1,xArch1,row,hArch1);

	row++;

	LayerDeclare(LAYOUT_HEADER_BCLK, lBClk0, xBClk0, row, hBClk0);
	LayerDeclare(LAYOUT_HEADER_CACHES,lArch2,xArch2, row, hArch2);

	row++;

	if (StrFormat(Buffer, 10+10+1,
			"%3u" "%-3u",
			RO(Shm)->Proc.CPU.OnLine,
			RO(Shm)->Proc.CPU.Count) > 0)
	{
		hProc1.code[2] = (ASCII) Buffer[0];
		hProc1.code[3] = (ASCII) Buffer[1];
		hProc1.code[4] = (ASCII) Buffer[2];
		hProc1.code[6] = (ASCII) Buffer[3];
		hProc1.code[7] = (ASCII) Buffer[4];
		hProc1.code[8] = (ASCII) Buffer[5];
	}
	unsigned int L1I_Size = 0, L1D_Size = 0, L2U_Size = 0, L3U_Size = 0;
    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	L1I_Size = \
	RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[0].Size / 1024;

	L1D_Size = \
	RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[1].Size / 1024;

	L2U_Size = \
	RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[2].Size / 1024;

	L3U_Size = \
	RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[3].Size / 1024;
    }
    else if (	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	 ||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	L1I_Size = \
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[0].Size;

	L1D_Size = \
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[1].Size;

	L2U_Size = \
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[2].Size;

	L3U_Size = \
		RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.Cache[3].Size;
    }
	if (StrFormat(Buffer, 10+10+1, "%-3u" "%-3u", L1I_Size, L1D_Size) > 0)
	{
		hArch1.code[17] = (ASCII) Buffer[0];
		hArch1.code[18] = (ASCII) Buffer[1];
		hArch1.code[19] = (ASCII) Buffer[2];
		hArch1.code[25] = (ASCII) Buffer[3];
		hArch1.code[26] = (ASCII) Buffer[4];
		hArch1.code[27] = (ASCII) Buffer[5];
	}
	if (StrFormat(Buffer, 10+10+1, "%-4u" "%-5u", L2U_Size, L3U_Size) > 0)
	{
		hArch2.code[ 3] = (ASCII) Buffer[0];
		hArch2.code[ 4] = (ASCII) Buffer[1];
		hArch2.code[ 5] = (ASCII) Buffer[2];
		hArch2.code[ 6] = (ASCII) Buffer[3];
		hArch2.code[13] = (ASCII) Buffer[4];
		hArch2.code[14] = (ASCII) Buffer[5];
		hArch2.code[15] = (ASCII) Buffer[6];
		hArch2.code[16] = (ASCII) Buffer[7];
		hArch2.code[17] = (ASCII) Buffer[8];
	}
	len = CUMIN(xProc1 - (hProc0.origin.col + hProc0.length),
			(CUINT) strlen(RO(Shm)->Proc.Brand));
	/* RED DOT */
	hProc0.code[0] = BITVAL(RW(Shm)->Proc.Sync, BURN) ? '.' : 0x20;

	LayerCopyAt(	layer, hProc0.origin.col, hProc0.origin.row,
			hProc0.length, hProc0.attr, hProc0.code );

	LayerFillAt(	layer,
			(hProc0.origin.col + hProc0.length), hProc0.origin.row,
			len, RO(Shm)->Proc.Brand,
			RSC(UI).ATTR()[UI_LAYOUT_HEADER_PROC_BRAND] );

    if ((hProc1.origin.col - len) > 0) {
	LayerFillAt(	layer,
			(hProc0.origin.col + hProc0.length + len),
			hProc0.origin.row,
			(hProc1.origin.col - len), hSpace,
			RSC(UI).ATTR()[UI_LAYOUT_HEADER_PROC_BRAND_FILL] );
    }
	LayerCopyAt(	layer, hProc1.origin.col, hProc1.origin.row,
			hProc1.length, hProc1.attr, hProc1.code );

	len = CUMIN(xArch1 - (hArch0.origin.col + hArch0.length),
			(CUINT) strlen(RO(Shm)->Proc.Architecture));
	/* DUMP DOT */
	hArch0.code[0] = DumpStatus() ? '.' : 0x20;

	LayerCopyAt(	layer, hArch0.origin.col, hArch0.origin.row,
			hArch0.length, hArch0.attr, hArch0.code );

	LayerFillAt(	layer,
			(hArch0.origin.col + hArch0.length), hArch0.origin.row,
			len, RO(Shm)->Proc.Architecture,
			RSC(UI).ATTR()[UI_LAYOUT_HEADER_ARCHITECTURE] );

    if ((hArch1.origin.col - len) > 0) {
	LayerFillAt(	layer,
			(hArch0.origin.col + hArch0.length + len),
			hArch0.origin.row,
			(hArch1.origin.col - len), hSpace,
			RSC(UI).ATTR()[UI_LAYOUT_HEADER_ARCH_FILL] );
    }
	LayerCopyAt(	layer, hArch1.origin.col, hArch1.origin.row,
			hArch1.length, hArch1.attr, hArch1.code );

	LayerCopyAt(	layer, hBClk0.origin.col, hBClk0.origin.row,
			hBClk0.length, hBClk0.attr, hBClk0.code );

	LayerFillAt(	layer,
			(hBClk0.origin.col + hBClk0.length), hBClk0.origin.row,
			(hArch2.origin.col - hBClk0.origin.col + hBClk0.length),
			hSpace,
			RSC(UI).ATTR()[UI_LAYOUT_HEADER_BCLK_FILL] );

	LayerCopyAt(	layer, hArch2.origin.col, hArch2.origin.row,
			hArch2.length, hArch2.attr, hArch2.code );
}
#endif /* NO_HEADER */

#ifndef NO_UPPER
void Layout_Ruler_Load(Layer *layer, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_LOAD, Draw.Size.width, 0, row, hLoad0);

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
	const ATTRIBUTE attr[2] = {
		RSC(UI).ATTR()[UI_LAYOUT_RULER_LOAD_TAB_DIM],
		RSC(UI).ATTR()[UI_LAYOUT_RULER_LOAD_TAB_BRIGHT]
	};
	unsigned int idx = Ruler.Count, bright = 1;
	const CUINT margin = 4;
	CUINT lPos = Draw.Area.LoadWidth + margin;

	LayerCopyAt(layer, hLoad0.origin.col, hLoad0.origin.row,
			hLoad0.length, hLoad0.attr, hLoad0.code);

	/* Alternate the color of the frequency ratios			*/
  while (idx--)
  {
	double fPos = (Ruler.Uniq[idx] * Draw.Area.LoadWidth) / Ruler.Maximum;
	CUINT hPos = (CUINT) fPos;

	ASCII tabStop[10+1] = "00";
    if ((StrFormat(tabStop, 10+1, "%2u", Ruler.Uniq[idx]) > 0) && (hPos < lPos))
    {
	hPos = hLoad0.origin.col + hPos + 2;
      if (tabStop[0] != 0x20) {
	LayerAt(layer,code, hPos, hLoad0.origin.row) = tabStop[0];
	LayerAt(layer,attr, hPos, hLoad0.origin.row) = attr[bright];
      }
	LayerAt(layer, code, (hPos + 1), hLoad0.origin.row) = tabStop[1];
	LayerAt(layer, attr, (hPos + 1), hLoad0.origin.row) = attr[bright];

	bright = !bright;
    }
	lPos = hPos >= margin ? hPos - margin : margin;
  }
	LayerCopyAt(layer, hLoad1.origin.col, hLoad1.origin.row, hLoad1.length,
			hLoad1.attr[Draw.Load], hLoad1.code[Draw.Load]);
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

	return 0;
}

CUINT Layout_Monitor_Instructions(Layer *layer,const unsigned int cpu,CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_INST, RSZ(LAYOUT_MONITOR_INST),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return 0;
}

CUINT Layout_Monitor_Common(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_COMMON, RSZ(LAYOUT_MONITOR_COMMON),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return 0;
}

CUINT Layout_Monitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(layer);
	UNUSED(cpu);
	UNUSED(row);

	return 0;
}

CUINT Layout_Monitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_TASKS, (MAX_WIDTH - LOAD_LEAD + 1),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );

	cTask[cpu].col = LOAD_LEAD + 8;
	cTask[cpu].row = hMon0.origin.row;

	return 0;
}

CUINT Layout_Monitor_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_MONITOR_SLICE, RSZ(LAYOUT_MONITOR_SLICE),
			(LOAD_LEAD - 1), row, hMon0 );

	LayerCopyAt(	layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code );
	UNUSED(cpu);

	return 0;
}

CUINT Layout_Monitor_Custom(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_CUSTOM_FIELD, RSZ(LAYOUT_CUSTOM_FIELD),
			(LOAD_LEAD - 1), row, hCustom0 );

	LayerCopyAt(	layer, hCustom0.origin.col, hCustom0.origin.row,
			hCustom0.length, hCustom0.attr, hCustom0.code );
	UNUSED(cpu);

	return 0;
}

CUINT Layout_Ruler_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_FREQUENCY, Draw.Size.width,
			0, row, hFreq0 );

	LayerCopyAt(	layer, hFreq0.origin.col, hFreq0.origin.row,
			hFreq0.length, hFreq0.attr, hFreq0.code );
	UNUSED(cpu);

	if (!Draw.Flag.avgOrPC) {
		LayerDeclare(	LAYOUT_RULER_FREQUENCY_AVG, Draw.Size.width,
				0, (row + Draw.Area.MaxRows + 1), hAvg0 );

		LayerCopyAt(	layer, hAvg0.origin.col, hAvg0.origin.row,
				hAvg0.length, hAvg0.attr, hAvg0.code );
	} else {
		LayerDeclare(	LAYOUT_RULER_FREQUENCY_PKG, Draw.Size.width,
				0, (row + Draw.Area.MaxRows + 1), hPkg0) ;

		LayerCopyAt(	layer, hPkg0.origin.col, hPkg0.origin.row,
				hPkg0.length, hPkg0.attr, hPkg0.code );
	}
	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Instructions(Layer *layer, const unsigned int cpu,CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_INST, Draw.Size.width,
			0, row, hInstr );

	LayerCopyAt(	layer, hInstr.origin.col, hInstr.origin.row,
			hInstr.length, hInstr.attr, hInstr.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_INSTRUCTIONS] );
	UNUSED(cpu);

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Cycles(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_CYCLES, Draw.Size.width,
			0, row, hCycles );

	LayerCopyAt(	layer, hCycles.origin.col, hCycles.origin.row,
			hCycles.length, hCycles.attr, hCycles.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_CYCLES] );
	UNUSED(cpu);

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_CStates(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_CSTATES, Draw.Size.width,
			0, row, hCStates );

	LayerCopyAt(	layer, hCStates.origin.col, hCStates.origin.row,
			hCStates.length, hCStates.attr, hCStates.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_CSTATES] );
	UNUSED(cpu);

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Interrupts(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_INTERRUPTS, Draw.Size.width,
			0, row, hIntr0 );

	LayerCopyAt(	layer, hIntr0.origin.col, hIntr0.origin.row,
			hIntr0.length, hIntr0.attr, hIntr0.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_INTERRUPTS] );
	UNUSED(cpu);

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	const ASCII *Intel_CState[9] = {
		RSC(LAYOUT_PACKAGE_PC02).CODE(),
		RSC(LAYOUT_PACKAGE_PC03).CODE(),
		RSC(LAYOUT_PACKAGE_PC04).CODE(),
		RSC(LAYOUT_PACKAGE_PC06).CODE(),
		RSC(LAYOUT_PACKAGE_PC07).CODE(),
		RSC(LAYOUT_PACKAGE_PC08).CODE(),
		RSC(LAYOUT_PACKAGE_PC09).CODE(),
		RSC(LAYOUT_PACKAGE_PC10).CODE(),
		RSC(LAYOUT_PACKAGE_MC06).CODE()
	}, *AMD_CState[9] = {
		RSC(LAYOUT_PACKAGE_CTR0).CODE(),
		RSC(LAYOUT_PACKAGE_CTR1).CODE(),
		RSC(LAYOUT_PACKAGE_CTR2).CODE(),
		RSC(LAYOUT_PACKAGE_CTR3).CODE(),
		RSC(LAYOUT_PACKAGE_CTR4).CODE(),
		RSC(LAYOUT_PACKAGE_CTR5).CODE(),
		RSC(LAYOUT_PACKAGE_CTR6).CODE(),
		RSC(LAYOUT_PACKAGE_CTR7).CODE(),
		RSC(LAYOUT_PACKAGE_FCLK).CODE()
	}, **hCState = RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL ?
		Intel_CState : AMD_CState;

	LayerFillAt(	layer, 0, row, Draw.Size.width,
			RSC(LAYOUT_RULER_PACKAGE).CODE(),
			RSC(LAYOUT_RULER_PACKAGE).ATTR()[0]);

	unsigned int idx;
	UNUSED(cpu);

	for (idx = 0; idx < 9; idx++)
	{
		LayerCopyAt(	layer, 0, (row + idx + 1),
				Draw.Size.width,
				RSC(LAYOUT_PACKAGE_PC).ATTR(),
				RSC(LAYOUT_PACKAGE_PC).CODE());

		LayerAt(layer, code, 0, (row + idx + 1)) = hCState[idx][0];
		LayerAt(layer, code, 1, (row + idx + 1)) = hCState[idx][1];
		LayerAt(layer, code, 2, (row + idx + 1)) = hCState[idx][2];
		LayerAt(layer, code, 3, (row + idx + 1)) = hCState[idx][3];
	}

	LayerDeclare(	LAYOUT_PACKAGE_UNCORE, Draw.Size.width,
			0, (row + 10), Intel_Uncore);

	LayerDeclare(	LAYOUT_PACKAGE_FABRIC, Draw.Size.width,
			0, (row + 10), AMD_Fabric);

	const LAYER_DECL_ST *hUncore = \
		RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL ?
			&Intel_Uncore : &AMD_Fabric;

	LayerCopyAt(	layer, hUncore->origin.col, hUncore->origin.row,
			hUncore->length, hUncore->attr, hUncore->code);

	LayerFillAt(	layer, 0, (row + 11),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_PACKAGE] );

	row += 2 + 10;
	return row;
}

CUINT Layout_Ruler_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_TASKS, Draw.Size.width, 0, row, hTask0);

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
		.attr = hSort[RO(Shm)->SysGate.sortByField].attr,
		.code = hSort[RO(Shm)->SysGate.sortByField].code
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
			.col = (Draw.Size.width - 18),
			.row = (row + Draw.Area.MaxRows + 1)
		},
		.length = 15,
		.attr = hReverse[RO(Shm)->SysGate.reverseOrder].attr,
		.code = hReverse[RO(Shm)->SysGate.reverseOrder].code
	};

	LayerDeclare(LAYOUT_TASKS_VALUE_SWITCH, RSZ(LAYOUT_TASKS_VALUE_SWITCH),
			(Draw.Size.width - 34),
			(row + Draw.Area.MaxRows + 1),
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

	memcpy(&hTask3.attr[8], hTaskVal[Draw.Flag.taskVal].attr, 3);
	memcpy(&hTask3.code[8], hTaskVal[Draw.Flag.taskVal].code, 3);

	LayerDeclare(	LAYOUT_TASKS_TRACKING, RSZ(LAYOUT_TASKS_TRACKING),
			53, row, hTrack0 );

	LayerCopyAt(	layer, hTask0.origin.col, hTask0.origin.row,
			hTask0.length, hTask0.attr, hTask0.code );

	LayerCopyAt(	layer, hTask1.origin.col, hTask1.origin.row,
			hTask1.length, hTask1.attr, hTask1.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_TASKS_FILL] );

	LayerCopyAt(	layer, hTask2.origin.col, hTask2.origin.row,
			hTask2.length, hTask2.attr, hTask2.code );

	LayerCopyAt(	layer, hTask3.origin.col, hTask3.origin.row,
			hTask3.length, hTask3.attr, hTask3.code );

	LayerCopyAt(	layer, hTrack0.origin.col, hTrack0.origin.row,
			hTrack0.length, hTrack0.attr, hTrack0.code );

	if (RO(Shm)->SysGate.trackTask)
	{
		StrFormat(Buffer, 11+1, "%7d", RO(Shm)->SysGate.trackTask);
		LayerFillAt(	layer,
				(hTrack0.origin.col + 15), hTrack0.origin.row,
				7, Buffer,
				RSC(UI).ATTR()[UI_LAYOUT_RULER_TASKS_TRACKING]);
	}
	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Sensors(Layer *layer, const unsigned int cpu, CUINT row)
{
	CUINT oRow;

	LayerDeclare(LAYOUT_RULER_SENSORS, Draw.Size.width, 0, row, hSensors);

	UNUSED(cpu);

	LayerCopyAt(	layer, hSensors.origin.col, hSensors.origin.row,
			hSensors.length, hSensors.attr, hSensors.code );

	LayerAt(layer,code,32,hSensors.origin.row)=Setting.fahrCels ? 'F':'C';

  if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
    switch (RO(Shm)->Proc.ArchID) {
    case Kabylake_UY:
    case Kabylake:
    case Cannonlake_U:
    case Cannonlake_H:
    case Icelake_UY:
    case Icelake:
    case Cometlake_UY:
    case Cometlake:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_PLATFORM, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hPwrPfm);

	LayerCopyAt(	layer, hPwrPfm.origin.col, hPwrPfm.origin.row,
			hPwrPfm.length, hPwrPfm.attr, hPwrPfm.code );

	oRow = hPwrPfm.origin.row;
	}
	break;
    default:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_UNCORE, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hPwrUncore );

	LayerCopyAt(	layer, hPwrUncore.origin.col, hPwrUncore.origin.row,
			hPwrUncore.length, hPwrUncore.attr, hPwrUncore.code );

	oRow = hPwrUncore.origin.row;
	}
	break;
    }
  } else {
	LayerDeclare(	LAYOUT_RULER_PWR_SOC, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hPwrSoC );

	LayerCopyAt(	layer, hPwrSoC.origin.col, hPwrSoC.origin.row,
			hPwrSoC.length, hPwrSoC.attr, hPwrSoC.code );

	oRow = hPwrSoC.origin.row;
  }
	LayerAt(layer,code, 45, oRow) = '0'
	+ RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.PackageID;

	LayerAt(layer,code, 14, oRow) = \
	LayerAt(layer,code, 35, oRow) = \
	LayerAt(layer,code, 57, oRow) = \
	LayerAt(layer,code, 77, oRow) = Setting.jouleWatt ? 'W':'J';

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(	LAYOUT_RULER_VOLTAGE, Draw.Size.width, 0, row, hVolt );

	UNUSED(cpu);

	LayerCopyAt(	layer, hVolt.origin.col, hVolt.origin.row,
			hVolt.length, hVolt.attr, hVolt.code );

    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	LayerDeclare(	LAYOUT_RULER_VPKG_SAV, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hVPkg );

	LayerCopyAt(	layer, hVPkg.origin.col, hVPkg.origin.row,
			hVPkg.length, hVPkg.attr, hVPkg.code );
    }
    else
    {
	LayerDeclare(	LAYOUT_RULER_VPKG_SOC, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hVPkg );

	LayerCopyAt(	layer, hVPkg.origin.col, hVPkg.origin.row,
			hVPkg.length, hVPkg.attr, hVPkg.code );
    }
	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Energy(Layer *layer, const unsigned int cpu, CUINT row)
{
	CUINT oRow;
	UNUSED(cpu);

    if (Setting.jouleWatt)
    {
	LayerDeclare(LAYOUT_RULER_POWER, Draw.Size.width, 0, row,  hPwr0);

	LayerCopyAt(	layer,  hPwr0.origin.col,  hPwr0.origin.row,
			 hPwr0.length,  hPwr0.attr,  hPwr0.code );
    }
   else
    {
	LayerDeclare(LAYOUT_RULER_ENERGY, Draw.Size.width, 0, row,  hPwr0);

	LayerCopyAt(	layer,  hPwr0.origin.col,  hPwr0.origin.row,
			 hPwr0.length,  hPwr0.attr,  hPwr0.code );
    }
	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_ENERGY] );

  if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
    switch (RO(Shm)->Proc.ArchID) {
    case Kabylake_UY:
    case Kabylake:
    case Cannonlake_U:
    case Cannonlake_H:
    case Icelake_UY:
    case Icelake:
    case Cometlake_UY:
    case Cometlake:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_PLATFORM, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hPwrPfm);

	LayerCopyAt(	layer, hPwrPfm.origin.col, hPwrPfm.origin.row,
			hPwrPfm.length, hPwrPfm.attr, hPwrPfm.code );

	oRow = hPwrPfm.origin.row;
	}
	break;
    default:
	{
	LayerDeclare(	LAYOUT_RULER_PWR_UNCORE, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hPwrUncore);

	LayerCopyAt(	layer, hPwrUncore.origin.col, hPwrUncore.origin.row,
			hPwrUncore.length, hPwrUncore.attr, hPwrUncore.code );

	oRow = hPwrUncore.origin.row;
	}
	break;
    }
  } else {
	LayerDeclare(	LAYOUT_RULER_PWR_SOC, Draw.Size.width,
			0, (row + Draw.Area.MaxRows + 1), hPwrSoC);

	LayerCopyAt(	layer, hPwrSoC.origin.col, hPwrSoC.origin.row,
			hPwrSoC.length, hPwrSoC.attr, hPwrSoC.code );

	oRow = hPwrSoC.origin.row;
  }
	LayerAt(layer,code, 45, oRow) = '0'
	+ RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.PackageID;

	LayerAt(layer,code, 14, oRow) = \
	LayerAt(layer,code, 35, oRow) = \
	LayerAt(layer,code, 57, oRow) = \
	LayerAt(layer,code, 77, oRow) = Setting.jouleWatt ? 'W':'J';

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Slice(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_SLICE, Draw.Size.width, 0, row, hSlice0);

	UNUSED(cpu);

	LayerCopyAt(	layer, hSlice0.origin.col, hSlice0.origin.row,
			hSlice0.length, hSlice0.attr, hSlice0.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_SLICE] );

	row += Draw.Area.MaxRows + 2;
	return row;
}

CUINT Layout_Ruler_Custom(Layer *layer, const unsigned int cpu, CUINT row)
{
	LayerDeclare(LAYOUT_RULER_CUSTOM, Draw.Size.width, 0, row, hCust0);

	UNUSED(cpu);

	LayerCopyAt(	layer, hCust0.origin.col, hCust0.origin.row,
			hCust0.length, hCust0.attr, hCust0.code );

	LayerFillAt(	layer, 0, (row + Draw.Area.MaxRows + 1),
			Draw.Size.width, hLine,
			RSC(UI).ATTR()[UI_LAYOUT_RULER_CUSTOM] );

	LayerAt(layer, code,  99, hCust0.origin.row) = \
	LayerAt(layer, code, 117, hCust0.origin.row) = \
						Setting.jouleWatt ? 'W':'J';

	row += Draw.Area.MaxRows + 2;
	return row;
}
#endif /* NO_LOWER */

#ifndef NO_FOOTER
void Layout_Footer(Layer *layer, CUINT row)
{
	size_t len;
	CUINT col = 0;

	LayerDeclare(	LAYOUT_FOOTER_TECH_X86, RSZ(LAYOUT_FOOTER_TECH_X86),
			0, row, hTech0 );

	LayerDeclare(	LAYOUT_FOOTER_VOLT_TEMP, RSZ(LAYOUT_FOOTER_VOLT_TEMP),
			Draw.Size.width - 15, row, hVoltTemp0 );
	/* Pre-compute right-aligned position of Voltage & Temperature items */
	Draw.Area.Footer.VoltTemp.Hot[0] = hVoltTemp0.origin.col + 2;

	const ATTRIBUTE EN[] = {
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_ENABLE_0],
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_ENABLE_1],
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_ENABLE_2]
	};
	const ATTRIBUTE TM[] = {
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TM_0],
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TM_1],
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TM_2],
		RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TM_3]
	};
	const struct { ASCII *code; ATTRIBUTE attr; } TSC[] = {
		{RSC(LAYOUT_FOOTER_TSC_NONE).CODE(),
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TSC_NONE]},
		{RSC(LAYOUT_FOOTER_TSC_VAR).CODE(),
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TSC_VAR]},
		{RSC(LAYOUT_FOOTER_TSC_INV).CODE(),
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_TSC_INV]}
	};

	hTech0.code[ 6] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[0];
	hTech0.code[ 7] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[1];
	hTech0.code[ 8] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[2];
	hTech0.code[ 9] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[3];
	hTech0.code[10] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[4];
	hTech0.code[11] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[5];
	hTech0.code[12] = TSC[RO(Shm)->Proc.Features.InvariantTSC].code[6];

	hTech0.attr[ 6] = hTech0.attr[ 7] = hTech0.attr[ 8] = \
	hTech0.attr[ 9] = hTech0.attr[10] = hTech0.attr[11] = \
	hTech0.attr[12] = TSC[RO(Shm)->Proc.Features.InvariantTSC].attr;

	LayerCopyAt(	layer, hTech0.origin.col, hTech0.origin.row,
			hTech0.length, hTech0.attr, hTech0.code );

    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	LayerDeclare(	LAYOUT_FOOTER_TECH_INTEL, RSZ(LAYOUT_FOOTER_TECH_INTEL),
			hTech0.length, hTech0.origin.row,
			hTech1 );

	hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] = \
				EN[RO(Shm)->Proc.Features.HyperThreading];

	hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] = \
	hTech1.attr[7] = EN[RO(Shm)->Proc.Technology.EIST];

      if (RO(Shm)->Proc.Features.Power.EAX.HWP_Reg == 1)
      {
	memcpy(&hTech1.code[9], RSC(PERF_LABEL_HWP).CODE(),RSZ(PERF_LABEL_HWP));
	hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] = \
			EN[RO(Shm)->Proc.Features.HWP_Enable == 1 ? 1 : 2];
      } else {
	hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] = \
				EN[RO(Shm)->Proc.Features.Power.EAX.TurboIDA];
      }
	hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] = \
	hTech1.attr[16] = hTech1.attr[17] = EN[RO(Shm)->Proc.Technology.Turbo];

	hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] = \
				EN[RO(Shm)->Proc.Technology.C1E];

	StrFormat(Buffer, 2+10+1, "PM%1u", RO(Shm)->Proc.PM_version);

	hTech1.code[23] = (ASCII) Buffer[0];
	hTech1.code[24] = (ASCII) Buffer[1];
	hTech1.code[25] = (ASCII) Buffer[2];

	hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] = \
				EN[(RO(Shm)->Proc.PM_version > 0)];

	hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] = \
				EN[RO(Shm)->Proc.Technology.C3A];

	hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] = \
				EN[RO(Shm)->Proc.Technology.C1A];

	hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] = \
				EN[RO(Shm)->Proc.Technology.C3U];

	hTech1.attr[39] = hTech1.attr[40] = hTech1.attr[41] = \
				EN[RO(Shm)->Proc.Technology.C1U];

	hTech1.attr[43] = hTech1.attr[44] = \
		TM[RO(Shm)->Proc.Technology.TM1|RO(Shm)->Proc.Technology.TM2];

	LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
			hTech1.origin.row,
			(Draw.Size.width - hTech0.length - hTech1.length),
			hSpace,
			RSC(UI).ATTR()[UI_LAYOUT_FOOTER_FILL]);

	Draw.Area.Footer.VoltTemp.Hot[1] = hTech1.origin.col + 46;
    }
    else
    {
      if (	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
      {
	LayerDeclare(	LAYOUT_FOOTER_TECH_AMD, RSZ(LAYOUT_FOOTER_TECH_AMD),
			hTech0.length, hTech0.origin.row,
			hTech1 );

	hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] = \
				EN[RO(Shm)->Proc.Features.HyperThreading];

	hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] = \
				EN[(RO(Shm)->Proc.PowerNow == 0b11)];

	hTech1.attr[8] = hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] = \
		EN[
			RO(Shm)->Proc.Features.HWP_Enable == 1 ? 1
			:  (RO(Shm)->Proc.Features.leaf80000008.EBX.CPPC == 1)
			|| (RO(Shm)->Proc.Features.ACPI_CPPC == 1) ?
				2 : 0
		];

	hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] = \
	hTech1.attr[16] = hTech1.attr[17] = EN[RO(Shm)->Proc.Technology.Turbo];

	hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] = \
					EN[RO(Shm)->Proc.Technology.C1E];

	hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] = \
					EN[RO(Shm)->Proc.Technology.CC6];

	hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] = \
					EN[RO(Shm)->Proc.Technology.PC6];

	hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] = \
					EN[(RO(Shm)->Cpu[
						RO(Shm)->Proc.Service.Core
					].Query.CStateBaseAddr != 0)];

	hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] = \
			EN[(RO(Shm)->Proc.Features.AdvPower.EDX.TS != 0)];

	hTech1.attr[39] = hTech1.attr[40] = \
		TM[RO(Shm)->Proc.Technology.TM1|RO(Shm)->Proc.Technology.TM2];

	LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
			hTech1.origin.row,
			(Draw.Size.width - hTech0.length - hTech1.length),
			hSpace,
			RSC(UI).ATTR()[UI_LAYOUT_FOOTER_FILL]);

	Draw.Area.Footer.VoltTemp.Hot[1] = hTech1.origin.col + 42;
      }
    }
	LayerCopyAt(layer, hVoltTemp0.origin.col, hVoltTemp0.origin.row,
			hVoltTemp0.length, hVoltTemp0.attr, hVoltTemp0.code);

	LayerAt(layer, code, Draw.Area.Footer.VoltTemp.Hot[0] + 11, row) = \
						Setting.fahrCels ? 'F' : 'C';

	row++;

	StrLenFormat(len, Buffer, MAX_UTS_LEN, "%s",
			BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1) ?
				  RO(Shm)->SysGate.sysname
				: (char *) RSC(SYSGATE).CODE());

	LayerFillAt(	layer, col, row,
			len, Buffer,
			RSC(UI).ATTR()[UI_LAYOUT_FOOTER_GATE]);
	col += len;

	LayerAt(layer, attr, col, row) = RSC(UI).ATTR()[UI_LAYOUT_FOOTER_SPACE];
	LayerAt(layer, code, col, row) = 0x20;

	col++;

	LayerAt(layer, attr, col, row) = \
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_LEFT_BRACE];

	LayerAt(layer, code, col, row) = '[';

	col++;

	if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)) {
		StrLenFormat(len, Buffer, 2+5+5+5+1, "%hu.%hu.%hu",
				RO(Shm)->SysGate.kernel.version,
				RO(Shm)->SysGate.kernel.major,
				RO(Shm)->SysGate.kernel.minor);

		LayerFillAt(	layer, col, row,
				len, Buffer,
			RSC(UI).ATTR()[UI_LAYOUT_FOOTER_KERNEL_VERSION] );

		col += len;
	} else {
		LayerFillAt(	layer, col, row,
				3, "OFF",
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_UNMAP_GATE]);
		col += 3;
	}
	LayerAt(layer, attr, col, row) = \
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_RIGHT_BRACE];

	LayerAt(layer, code, col, row) = ']';

	col++;

	LayerDeclare(	LAYOUT_FOOTER_SYSTEM, RSZ(LAYOUT_FOOTER_SYSTEM),
			(Draw.Size.width-RSZ(LAYOUT_FOOTER_SYSTEM)),row, hSys1);

	LayerCopyAt(	layer, hSys1.origin.col, hSys1.origin.row,
			hSys1.length, hSys1.attr, hSys1.code );
	/* Center the DMI string					*/
	if ((len = strlen(RO(Shm)->SMB.String[Draw.SmbIndex])) > 0) {
		const CSINT	mlt = (CSINT) (hSys1.origin.col - col - 1),
				mrt = (CSINT) len,
				can = CUMIN(mlt, mrt),
				ctr = ((hSys1.origin.col + col) - can) / 2;

		LayerFillAt(	layer, ctr, hSys1.origin.row,
				can, ScrambleSMBIOS(Draw.SmbIndex, 4, '-'),
				RSC(UI).ATTR()[UI_LAYOUT_FOOTER_DMI_STRING] );
	}
	/* Pre-compute position of Tasks count & Memory usage		*/
	Draw.Area.Footer.TaskMem.Total	= Draw.Size.width - 35;
	Draw.Area.Footer.TaskMem.Free	= Draw.Size.width - 22;
	Draw.Area.Footer.TaskMem.Count	= Draw.Size.width - 12;
	/* Reset Tasks count & Memory usage				*/
	if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)) {
		PrintTaskMemory(layer, row,
				RO(Shm)->SysGate.procCount,
				RO(Shm)->SysGate.memInfo.freeram,
				RO(Shm)->SysGate.memInfo.totalram);
	}
	/* Print the memory unit character				*/
	LayerAt(layer, code,
		hSys1.origin.col + (RSZ(LAYOUT_FOOTER_SYSTEM) - 3),
		hSys1.origin.row) =	Draw.Unit.Memory ==  0 ? 'K'
				:	Draw.Unit.Memory == 10 ? 'M'
				:	Draw.Unit.Memory == 20 ? 'G' : 0x20;
}
#endif /* NO_FOOTER */

void Layout_CPU_To_String(const unsigned int cpu)
{
	StrFormat(Buffer, 10+1, "%03u", cpu);
}

void Layout_CPU_To_View(Layer *layer, const CUINT col, const CUINT row)
{
	LayerAt(layer, code, col + 0, row) = (ASCII) Buffer[0];
	LayerAt(layer, code, col + 1, row) = (ASCII) Buffer[1];
	LayerAt(layer, code, col + 2, row) = (ASCII) Buffer[2];
}

void Layout_BCLK_To_View(Layer *layer, const CUINT col, const CUINT row)
{
	LayerAt(layer, attr, col + 3, row) = \
					RSC(UI).ATTR()[UI_LAYOUT_BCLK_TO_VIEW];

	LayerAt(layer, code, col + 3, row) = 0x20;
}

#ifndef NO_UPPER
CUINT Draw_Frequency_Load(	Layer *layer, CUINT row,
				const unsigned int cpu, double ratio )
{	/*	Upper view area: draw bar chart iff load exists:	*/
	const double fPos = ((ratio > Ruler.Maximum ? Ruler.Maximum : ratio)
				* Draw.Area.LoadWidth) / Ruler.Maximum;
	const CUINT col = (CUINT) fPos;
    if (col > 0) {
	const ATTRIBUTE attr = ratio > Ruler.Median ?
				RSC(UI).ATTR()[UI_DRAW_FREQUENCY_LOAD_HIGH]
				: ratio > Ruler.Minimum ?
				RSC(UI).ATTR()[UI_DRAW_FREQUENCY_LOAD_MEDIUM]
				: RSC(UI).ATTR()[UI_DRAW_FREQUENCY_LOAD_LOW];

	LayerFillAt(layer, LOAD_LEAD, row, col, hBar, attr);
    }
    if (BITVAL_CC(Draw.garbage, row)) {
	struct {
		const CUINT col, wth;
	} garbage = {
		.col = col + LOAD_LEAD,
		.wth = Draw.Area.LoadWidth - col
	};
	ClearGarbage(	layer, attr, garbage.col, row, garbage.wth,
			RSC(UI).ATTR()[UI_DRAW_FREQUENCY_LOAD_CLEAR].value );

	ClearGarbage(	layer, code, garbage.col, row, garbage.wth, 0x0 );
    }
    if (!col) {
	BITCLR_CC(LOCKLESS, Draw.garbage, row);
    } else {
	BITSET_CC(LOCKLESS, Draw.garbage, row);
    }
	return 0;
}

CUINT Draw_Relative_Load(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
			&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	/*		Draw the relative Core frequency ratio		*/
	return Draw_Frequency_Load(layer, row, cpu, CFlop->Relative.Ratio);
}

CUINT Draw_Absolute_Load(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	/*		Draw the absolute Core frequency ratio		*/
	return Draw_Frequency_Load(layer, row, cpu, CFlop->Absolute.Ratio.Perf);
}
#endif /* NO_UPPER */

#ifndef NO_LOWER
size_t Draw_Relative_Freq_Spaces(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);
	UNUSED(cpu);

	size_t len;
	StrLenFormat(len, Buffer, 17+8+6+7+7+7+7+7+7+11+1,
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

	return len;
}

size_t Draw_Absolute_Freq_Spaces(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);
	UNUSED(cpu);

	size_t len;
	StrLenFormat(len, Buffer, 17+8+6+7+7+7+7+7+7+11+1,
		"%7.2f" " (" "%5.2f" ") "			\
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
		11, hSpace);

	return len;
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
	return Draw_Freq_Spaces_Matrix[Draw.Load](CFlop, Cpu, cpu);
}

size_t Draw_Relative_Freq_Celsius(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	size_t len;
	StrLenFormat(len, Buffer, 19+8+6+7+7+7+7+7+7+10+10+10+1,
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
		Cpu->PowerThermal.Limit[SENSOR_HIGHEST]);

	return len;
}

size_t Draw_Absolute_Freq_Celsius(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	size_t len;
	StrLenFormat(len, Buffer, 19+8+6+7+7+7+7+7+7+10+10+10+1,
		"%7.2f" " (" "%5.2f" ") "			\
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
		Cpu->PowerThermal.Limit[SENSOR_HIGHEST]);

	return len;
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
	return Draw_Freq_Celsius_Matrix[Draw.Load](CFlop, Cpu, cpu);
};

size_t Draw_Relative_Freq_Fahrenheit(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	size_t len;
	StrLenFormat(len, Buffer, 19+8+6+7+7+7+7+7+7+10+10+10+1,
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
		Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_HIGHEST]));

	return len;
}

size_t Draw_Absolute_Freq_Fahrenheit(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(cpu);

	size_t len;
	StrLenFormat(len, Buffer, 19+8+6+7+7+7+7+7+7+10+10+10+1,
		"%7.2f" " (" "%5.2f" ") "			\
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
		Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_HIGHEST]));

	return len;
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
	return Draw_Freq_Fahrenheit_Matrix[Draw.Load](CFlop, Cpu, cpu);
};

size_t Draw_Freq_Celsius_PerCore(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Freq_Celsius(CFlop, &RO(Shm)->Cpu[cpu], cpu);
	} else {
		return Draw_Freq_Spaces(CFlop, NULL, cpu);
	}
}

size_t Draw_Freq_Fahrenheit_PerCore(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Freq_Fahrenheit(CFlop, &RO(Shm)->Cpu[cpu], cpu);
	} else {
		return Draw_Freq_Spaces(CFlop, NULL, cpu);
	}
}

size_t Draw_Freq_Celsius_PerPkg(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Freq_Celsius(CFlop, &RO(Shm)->Cpu[cpu], cpu);
	} else {
		return Draw_Freq_Spaces(CFlop, NULL, cpu);
	}
}

size_t Draw_Freq_Fahrenheit_PerPkg(	struct FLIP_FLOP *CFlop,
					CPU_STRUCT *Cpu,
					const unsigned int cpu )
{
	UNUSED(Cpu);

	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Freq_Fahrenheit(CFlop, &RO(Shm)->Cpu[cpu], cpu);
	} else {
		return Draw_Freq_Spaces(CFlop, NULL, cpu);
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
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	const enum FORMULA_SCOPE
		thermalScope = SCOPE_OF_FORMULA(RO(Shm)->Proc.thermalFormula);

	size_t len = Draw_Freq_Temp_Matrix[thermalScope][Setting.fahrCels](
				CFlop, &RO(Shm)->Cpu[cpu], cpu
	);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	ATTRIBUTE warning = RSC(UI).ATTR()[UI_DRAW_MONITOR_FREQUENCY_NOMINAL];

	if (CFlop->Thermal.Temp <= RO(Shm)->Cpu[
						cpu
					].PowerThermal.Limit[SENSOR_LOWEST])
	{
		warning = RSC(UI).ATTR()[UI_DRAW_MONITOR_FREQUENCY_LOW];
	} else {
	    if (CFlop->Thermal.Temp >= RO(Shm)->Cpu[
							cpu
					].PowerThermal.Limit[SENSOR_HIGHEST])
		warning = RSC(UI).ATTR()[UI_DRAW_MONITOR_FREQUENCY_HIGH];
	}
	if (CFlop->Thermal.Events[eSTS] & STATUS_EVENT_FILTER)
	{
		warning = RSC(UI).ATTR()[UI_DRAW_MONITOR_FREQUENCY_HOT];
	}
	LayerAt(layer, attr, (LOAD_LEAD + 69), row) = \
		LayerAt(layer, attr, (LOAD_LEAD + 70), row) = \
			LayerAt(layer, attr, (LOAD_LEAD + 71), row) = warning;

	return 0;
}

CUINT Draw_Monitor_Instructions(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	size_t len;
	StrLenFormat(len, Buffer, 6+18+18+18+20+1,
			"%17.6f" "/s"					\
			"%17.6f" "/c"					\
			"%17.6f" "/i"					\
			"%18llu",
			CFlop->State.IPS,
			CFlop->State.IPC,
			CFlop->State.CPI,
			CFlop->Delta.INST);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

CUINT Draw_Monitor_Cycles(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	size_t len;
	StrLenFormat(len, Buffer, 20+20+20+20+1,
			"%18llu%18llu%18llu%18llu",
			CFlop->Delta.C0.UCC,
			CFlop->Delta.C0.URC,
			CFlop->Delta.C1,
			CFlop->Delta.TSC);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

CUINT Draw_Monitor_CStates(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	size_t len;
	StrLenFormat(len, Buffer, 20+20+20+20+1,
			"%18llu%18llu%18llu%18llu",
			CFlop->Delta.C1,
			CFlop->Delta.C3,
			CFlop->Delta.C6,
			CFlop->Delta.C7);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

CUINT Draw_Monitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(layer);
	UNUSED(cpu);
	UNUSED(row);

	return 0;
}

size_t Draw_Monitor_Tasks_Relative_Freq(struct FLIP_FLOP *CFlop)
{
	size_t len;
	StrLenFormat(len, Buffer, 8+1, "%7.2f", CFlop->Relative.Freq);
	return len;
}

size_t Draw_Monitor_Tasks_Absolute_Freq(struct FLIP_FLOP *CFlop)
{
	size_t len;
	StrLenFormat(len, Buffer, 8+1, "%7.2f", CFlop->Absolute.Freq);
	return len;
}

size_t (*Draw_Monitor_Tasks_Matrix[2])(struct FLIP_FLOP *CFlop) = \
{
	Draw_Monitor_Tasks_Relative_Freq,
	Draw_Monitor_Tasks_Absolute_Freq
};

CUINT Draw_Monitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	size_t len = Draw_Monitor_Tasks_Matrix[Draw.Load](CFlop);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);
	cTask[cpu].col = LOAD_LEAD + 8;

	return 0;
}

CUINT Draw_Monitor_Interrupts(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	size_t len;

	StrLenFormat(len, Buffer, 10+1, "%10u", CFlop->Counter.SMI);
	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

    if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_LOCAL) == 1) {
	StrLenFormat(len, Buffer, 10+1, "%10u", CFlop->Counter.NMI.LOCAL);
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 24),row), Buffer, len);
    } else {
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 24),row), hSpace, 10);
    }
    if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_UNKNOWN) == 1) {
	StrLenFormat(len, Buffer, 10+1,"%10u",CFlop->Counter.NMI.UNKNOWN);
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 34),row), Buffer, len);
    } else {
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 34),row), hSpace, 10);
    }
    if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_SERR) == 1) {
	StrLenFormat(len, Buffer, 10+1,"%10u",CFlop->Counter.NMI.PCISERR);
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 44),row), Buffer, len);
    } else {
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 44),row), hSpace, 10);
    }
    if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_IO_CHECK) == 1) {
	StrLenFormat(len, Buffer, 10+1,"%10u",CFlop->Counter.NMI.IOCHECK);
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 54),row), Buffer, len);
    } else {
	memcpy(&LayerAt(layer,code,(LOAD_LEAD + 54),row), hSpace, 10);
    }
	return 0;
}

size_t Draw_Sensors_V0_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			69, hSpace);
	return len;
}

size_t Draw_Sensors_V0_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f%.*s"					\
			"%8.4f\x20\x20\x20\x20\x20\x20" 		\
			"%8.4f%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			26, hSpace,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace);
	return len;
}

size_t Draw_Sensors_V0_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V0_T0_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V0_T0_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f%.*s"					\
			"%3u%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			17, hSpace,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			49, hSpace);
	return len;
}

size_t Draw_Sensors_V0_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f%.*s"					\
			"%3u\x20\x20\x20\x20\x20\x20"			\
			"%8.4f\x20\x20\x20\x20\x20\x20" 		\
			"%8.4f%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			17, hSpace,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace);
	return len;
}

size_t Draw_Sensors_V0_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V0_T1_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V0_T1_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V0_T1_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V0_T1_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V0_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V0_T1_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			56, hSpace);
	return len;
}

size_t Draw_Sensors_V1_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f\x20\x20\x20\x20" 			\
			"\x20\x20\x20\x20\x20\x20\x20\x20\x20"		\
			"%8.4f\x20\x20\x20\x20\x20\x20" 		\
			"%8.4f%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace);
	return len;
}

size_t Draw_Sensors_V1_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T0_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T0_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f\x20\x20\x20\x20" 			\
			"%3u%.*s",					\
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			49, hSpace);
	return len;
}

size_t Draw_Sensors_V1_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20\x20\x20\x20\x20\x20\x20"		\
			"%5.4f\x20\x20\x20\x20" 			\
			"%3u\x20\x20\x20\x20\x20\x20"			\
			"%8.4f\x20\x20\x20\x20\x20\x20" 		\
			"%8.4f%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.Vcore,
			Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
			CFlop->State.Energy,
			CFlop->State.Power,
			21, hSpace);
	return len;
}

size_t Draw_Sensors_V1_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T1_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T1_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V1_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V1_T0_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T0_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T0_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T0_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T0_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T1_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T2_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T2_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T2_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T2_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T3_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T3_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T3_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V2_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Sensors_V1_T3_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T0_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T0_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T0_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T0_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T0_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T0_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T0_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T0_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T0_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T1_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T1_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T1_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T1_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T1_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T1_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T2_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T2_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T2_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T2_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T2_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T2_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T2_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T2_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T2_P3(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T3_P0(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T3_P0(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P0(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T3_P1(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T3_P1(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P1(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T3_P2(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T3_P2(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P2(layer, cpu, row);
	}
}

size_t Draw_Sensors_V3_T3_P3(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Sensors_V1_T3_P3(layer, cpu, row);
	} else {
		return Draw_Sensors_V0_T3_P3(layer, cpu, row);
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
		voltageScope = SCOPE_OF_FORMULA(RO(Shm)->Proc.voltageFormula),
		thermalScope = SCOPE_OF_FORMULA(RO(Shm)->Proc.thermalFormula),
		powerScope   = SCOPE_OF_FORMULA(RO(Shm)->Proc.powerFormula);

	len = Draw_Sensors_VTP_Matrix	[voltageScope]
					[thermalScope]
					[powerScope]	(layer, cpu, row);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

#define Draw_Voltage_None	Draw_Sensors_V0_T0_P0

size_t Draw_Voltage_SMT(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20%7d"					\
			"\x20\x20\x20\x20\x20%5.4f"			\
			"\x20\x20\x20%5.4f"				\
			"\x20\x20\x20%5.4f"				\
			"%.*s",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Voltage.VID,
			RO(Shm)->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_LOWEST],
			CFlop->Voltage.Vcore,
			RO(Shm)->Cpu[cpu].Sensors.Voltage.Limit[SENSOR_HIGHEST],
			32, hSpace);
	return len;
}

size_t Draw_Voltage_Core(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Voltage_SMT(layer, cpu, row);
	} else {
		return Draw_Voltage_None(layer, cpu, row);
	}
}

size_t Draw_Voltage_Pkg(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Voltage_SMT(layer, cpu, row);
	} else {
		return Draw_Voltage_None(layer, cpu, row);
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
		voltageScope = SCOPE_OF_FORMULA(RO(Shm)->Proc.voltageFormula);

	len = Draw_Voltage_Matrix[voltageScope](layer, cpu, row);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

#define Draw_Energy_None	Draw_Sensors_V0_T0_P0

size_t Draw_Energy_Joule(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20\x20\x20%018llu\x20\x20\x20\x20"	\
			"%10.6f\x20\x20%10.6f\x20\x20%10.6f",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Delta.Power.ACCU,
			RO(Shm)->Cpu[cpu].Sensors.Energy.Limit[SENSOR_LOWEST],
			CFlop->State.Energy,
			RO(Shm)->Cpu[cpu].Sensors.Energy.Limit[SENSOR_HIGHEST]);
	return len;
}

size_t Draw_Power_Watt(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];
	UNUSED(layer);
	UNUSED(row);

	size_t len;
	StrLenFormat(len, Buffer, 80+1,
			"%7.2f\x20\x20\x20%018llu\x20\x20\x20\x20"	\
			"%10.6f\x20\x20%10.6f\x20\x20%10.6f",
			Draw.Load ? CFlop->Absolute.Freq:CFlop->Relative.Freq,
			CFlop->Delta.Power.ACCU,
			RO(Shm)->Cpu[cpu].Sensors.Power.Limit[SENSOR_LOWEST],
			CFlop->State.Power,
			RO(Shm)->Cpu[cpu].Sensors.Power.Limit[SENSOR_HIGHEST]);
	return len;
}

size_t (*Draw_Energy_Power_Matrix[])(Layer*, const unsigned int, CUINT) = {
	Draw_Energy_Joule,
	Draw_Power_Watt
};

size_t Draw_Energy_SMT(Layer *layer, const unsigned int cpu, CUINT row)
{
	return Draw_Energy_Power_Matrix[Setting.jouleWatt](layer, cpu, row);
}

size_t Draw_Energy_Core(Layer *layer, const unsigned int cpu, CUINT row)
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1)) {
		return Draw_Energy_SMT(layer, cpu, row);
	} else {
		return Draw_Energy_None(layer, cpu, row);
	}
}

size_t Draw_Energy_Pkg(Layer *layer, const unsigned int cpu, CUINT row)
{
	if (cpu == RO(Shm)->Proc.Service.Core) {
		return Draw_Energy_SMT(layer, cpu, row);
	} else {
		return Draw_Energy_None(layer, cpu, row);
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
		powerScope = SCOPE_OF_FORMULA(RO(Shm)->Proc.powerFormula);

	len = Draw_Energy_Matrix[powerScope](layer, cpu, row);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

size_t Draw_Monitor_Slice_Error_Relative_Freq(	struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice )
{
	size_t len;
	StrLenFormat(len, Buffer, 8+20+20+20+20+20+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%18llu",
			CFlop->Relative.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			pSlice->Error);
	return len;
}

size_t Draw_Monitor_Slice_Error_Absolute_Freq(	struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice )
{
	size_t len;
	StrLenFormat(len, Buffer, 8+20+20+20+20+20+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%18llu",
			CFlop->Absolute.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			pSlice->Error);
	return len;
}

size_t Draw_Monitor_Slice_NoError_Relative_Freq(struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice)
{
	size_t len;
	StrLenFormat(len, Buffer, 8+20+20+20+20+18+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%.*s",
			CFlop->Relative.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			18, hSpace);
	return len;
}

size_t Draw_Monitor_Slice_NoError_Absolute_Freq(struct FLIP_FLOP *CFlop,
						struct SLICE_STRUCT *pSlice)
{
	size_t len;
	StrLenFormat(len, Buffer, 8+20+20+20+20+18+1,
			"%7.2f "					\
			"%16llu%16llu%18llu%18llu%.*s",
			CFlop->Absolute.Freq,
			pSlice->Delta.TSC,
			pSlice->Delta.INST,
			pSlice->Counter[1].TSC,
			pSlice->Counter[1].INST,
			18, hSpace);
	return len;
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
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[cpu].FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	struct SLICE_STRUCT *pSlice = &RO(Shm)->Cpu[cpu].Slice;

	const unsigned int flagError = (RO(Shm)->Cpu[cpu].Slice.Error > 0);
	size_t len;
	len = Draw_Monitor_Slice_Matrix[flagError][Draw.Load](CFlop, pSlice);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

CUINT Draw_Monitor_Custom(Layer *layer, const unsigned int cpu, CUINT row)
{
	CPU_STRUCT *Cpu = &RO(Shm)->Cpu[cpu];
	struct FLIP_FLOP *CFlop = &Cpu->FlipFlop[!RO(Shm)->Cpu[cpu].Toggle];

	size_t len;
	StrLenFormat(len, Buffer, MAX_WIDTH,
		"%7.2f %7.2f %7.2f"					\
		"\x20\x20\x20" "%7.2f %7.2f %7.2f"			\
		"\x20\x20" "%3u %3u %3u"				\
		"\x20\x20" "%7.4f %7.4f %7.4f"				\
		"\x20\x20" "%8.4f %8.4f %8.4f"				\
		"\x20\x20" "%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"	\
		"\x20\x20" "%10.6f" "%10.6f" "%10.6f",
/* [ 1] MIN_REL_FREQ	*/
		Cpu->Relative.Freq[SENSOR_LOWEST],
/* [ 2] RELATIVE_FREQ	*/
		CFlop->Relative.Freq,
/* [ 3] MAX_REL_FREQ	*/
		Cpu->Relative.Freq[SENSOR_HIGHEST],
/* [ 4] MIN_ABS_FREQ	*/
		Cpu->Absolute.Freq[SENSOR_LOWEST],
/* [ 5] ABSOLUTE_FREQ	*/
		CFlop->Absolute.Freq,
/* [ 6] MAX_ABS_FREQ	*/
		Cpu->Absolute.Freq[SENSOR_HIGHEST],
/* [ 7] MIN_TEMP	*/
	Setting.fahrCels ? Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_LOWEST])
			 : Cpu->PowerThermal.Limit[SENSOR_LOWEST],
/* [ 8] TEMPERATURE	*/
	Setting.fahrCels ? Cels2Fahr(CFlop->Thermal.Temp)
					 : CFlop->Thermal.Temp,
/* [ 9] MAX_TEMP	*/
	Setting.fahrCels ? Cels2Fahr(Cpu->PowerThermal.Limit[SENSOR_HIGHEST])
			 : Cpu->PowerThermal.Limit[SENSOR_HIGHEST],
/* [10] MIN_VOLT	*/
		Cpu->Sensors.Voltage.Limit[SENSOR_LOWEST],
/* [11] CORE_VOLTAGE	*/
		CFlop->Voltage.Vcore,
/* [12] MAX_VOLT	*/
		Cpu->Sensors.Voltage.Limit[SENSOR_HIGHEST],
/* [13] MIN_POWER	*/
	Setting.jouleWatt ? Cpu->Sensors.Energy.Limit[SENSOR_LOWEST]
			  : Cpu->Sensors.Power.Limit[SENSOR_LOWEST],
/* [14] ENERGY_POWER	*/
	Setting.jouleWatt ? CFlop->State.Energy : CFlop->State.Power,
/* [15] MAX_POWER	*/
	Setting.jouleWatt ? Cpu->Sensors.Energy.Limit[SENSOR_HIGHEST]
			  : Cpu->Sensors.Power.Limit[SENSOR_HIGHEST],
/* [16] PERCENT_TURBO	*/
		100.f * CFlop->State.Turbo,
/* [17 ... 21] C-STATES */
		100.f * CFlop->State.C0,
		100.f * CFlop->State.C1,
		100.f * CFlop->State.C3,
		100.f * CFlop->State.C6,
		100.f * CFlop->State.C7,
/* [22 ... 24] INST	*/
		CFlop->State.IPS,
		CFlop->State.IPC,
		CFlop->State.CPI);

	memcpy(&LayerAt(layer, code, LOAD_LEAD, row), Buffer, len);

	return 0;
}

CUINT Draw_AltMonitor_Frequency(Layer *layer, const unsigned int cpu, CUINT row)
{
	size_t len;
	UNUSED(cpu);

	row += 1 + Draw.Area.MaxRows;
	if (!Draw.Flag.avgOrPC) {
		StrLenFormat(len, Buffer, ((5*2)+1)+(6*7)+1,
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% " \
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%",
				100.f * RO(Shm)->Proc.Avg.Turbo,
				100.f * RO(Shm)->Proc.Avg.C0,
				100.f * RO(Shm)->Proc.Avg.C1,
				100.f * RO(Shm)->Proc.Avg.C3,
				100.f * RO(Shm)->Proc.Avg.C6,
				100.f * RO(Shm)->Proc.Avg.C7);

		memcpy(&LayerAt(layer, code, 20, row), Buffer, len);
	} else {
	#if defined(ARCH_PMC) && (ARCH_PMC == UMC)
		struct PKG_FLIP_FLOP *PFlop = \
			&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

		StrLenFormat(len, Buffer, ((20*8)+(4*6)+1),
				"   %5llu"	"   :%5llu"	"   :%5llu" \
				"   :%5llu"	"   :%5llu"	"   :%5llu" \
				"   :%5llu"	"    :%5llu",
				PFlop->Delta.PC02 / (1000LLU * 1000LLU),
				PFlop->Delta.PC03 / (1000LLU * 1000LLU),
				PFlop->Delta.PC04 / (1000LLU * 1000LLU),
				PFlop->Delta.PC06 / (1000LLU * 1000LLU),
				PFlop->Delta.PC07 / (1000LLU * 1000LLU),
				PFlop->Delta.PC08 / (1000LLU * 1000LLU),
				PFlop->Delta.PC09 / (1000LLU * 1000LLU),
				PFlop->Delta.PC10 / (1000LLU * 1000LLU));
	#else
		StrLenFormat(len, Buffer, ((6*4)+3+5)+(8*6)+1,
				"c2:%-5.1f"	" c3:%-5.1f"	" c4:%-5.1f" \
				" c6:%-5.1f"	" c7:%-5.1f"	" c8:%-5.1f" \
				" c9:%-5.1f"	" c10:%-5.1f",
				100.f * RO(Shm)->Proc.State.PC02,
				100.f * RO(Shm)->Proc.State.PC03,
				100.f * RO(Shm)->Proc.State.PC04,
				100.f * RO(Shm)->Proc.State.PC06,
				100.f * RO(Shm)->Proc.State.PC07,
				100.f * RO(Shm)->Proc.State.PC08,
				100.f * RO(Shm)->Proc.State.PC09,
				100.f * RO(Shm)->Proc.State.PC10);
	#endif
		memcpy(&LayerAt(layer, code, 7, row), Buffer, len);
	}
	row += 1;
	return row;
}

CUINT Draw_AltMonitor_Common(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(layer);
	UNUSED(cpu);

	row += 2 + Draw.Area.MaxRows;
	return row;
}

CUINT Draw_AltMonitor_Package(Layer *layer, const unsigned int cpu, CUINT row)
{
	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	CUINT bar0, bar1, margin = Draw.Area.LoadWidth - 28;
	size_t len;
	UNUSED(cpu);

	row += 1;
/* PC02 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC02 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC02, 100.f * RO(Shm)->Proc.State.PC02,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, row), Buffer, len);
/* PC03 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC03 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC03, 100.f * RO(Shm)->Proc.State.PC03,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 1)), Buffer, len);
/* PC04 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC04 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC04, 100.f * RO(Shm)->Proc.State.PC04,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 2)), Buffer, len);
/* PC06 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC06 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC06, 100.f * RO(Shm)->Proc.State.PC06,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 3)), Buffer, len);
/* PC07 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC07 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC07, 100.f * RO(Shm)->Proc.State.PC07,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 4)), Buffer, len);
/* PC08 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC08 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC08, 100.f * RO(Shm)->Proc.State.PC08,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 5)), Buffer, len);
/* PC09 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC09 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC09, 100.f * RO(Shm)->Proc.State.PC09,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 6)), Buffer, len);
/* PC10 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.PC10 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.PC10, 100.f * RO(Shm)->Proc.State.PC10,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 7)), Buffer, len);
/* MC6 */
	bar0 = (CUINT) (RO(Shm)->Proc.State.MC6 * margin);
	bar1 = margin - bar0;

	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu" "%7.2f" "%% " "%.*s" "%.*s",
			PFlop->Delta.MC6, 100.f * RO(Shm)->Proc.State.MC6,
			bar0, hBar, bar1, hSpace);

	memcpy(&LayerAt(layer, code, 5, (row + 8)), Buffer, len);
/* TSC */
	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu", PFlop->Delta.PCLK);

	memcpy(&LayerAt(layer, code, 5, (row + 9)), Buffer, len);
/* UNCORE */
	StrLenFormat(len, Buffer, Draw.Area.LoadWidth,
			"%18llu", PFlop->Uncore.FC0);

	memcpy(&LayerAt(layer, code, 5+18+7+2+18+7, (row + 9)), Buffer, len);

	row += 1 + 10;
	return row;
}

CUINT Draw_AltMonitor_Tasks(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(cpu);

  if (RO(Shm)->SysGate.tickStep == RO(Shm)->SysGate.tickReset)
  {
	ssize_t len = 0;
	signed int idx;
	char stateStr[TASK_COMM_LEN];
	ATTRIBUTE *stateAttr;

	/* Clear the trailing garbage chars left by the previous drawing. */
	FillLayerArea(	layer, (LOAD_LEAD + 8), (row + 1),
			(Draw.Size.width - (LOAD_LEAD + 8)), Draw.Area.MaxRows,
			hSpace, RSC(UI).ATTR()[UI_DRAW_ALTMONITOR_TASKS_CLEAR]);

   for (idx = 0; idx < RO(Shm)->SysGate.taskCount; idx++)
   {
    if(!BITVAL(RO(Shm)->Cpu[RO(Shm)->SysGate.taskList[idx].wake_cpu].OffLine,OS)
      && (RO(Shm)->SysGate.taskList[idx].wake_cpu >= (short int)Draw.cpuScroll)
      && (RO(Shm)->SysGate.taskList[idx].wake_cpu < (short int)(Draw.cpuScroll
							+ Draw.Area.MaxRows)))
    {
	unsigned int ldx = 2;
	CSINT dif = Draw.Size.width
		  - cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col;

     if (dif > 0)
     {
	stateAttr=StateToSymbol(RO(Shm)->SysGate.taskList[idx].state, stateStr);

      if (RO(Shm)->SysGate.taskList[idx].pid == RO(Shm)->SysGate.trackTask)
      {
	stateAttr = RSC(TRACKER_STATE_COLOR).ATTR();
      }
      if (!Draw.Flag.taskVal) {
	StrLenFormat(len, Buffer, TASK_COMM_LEN, "%s",
			RO(Shm)->SysGate.taskList[idx].comm);
      } else {
	switch (RO(Shm)->SysGate.sortByField) {
	case F_STATE:
		StrLenFormat(len, Buffer, 2 * TASK_COMM_LEN + 2,
				"%s(%s)",
				RO(Shm)->SysGate.taskList[idx].comm,
				stateStr);
		break;
	case F_RTIME:
		StrLenFormat(len, Buffer, TASK_COMM_LEN + 20 + 2,
				"%s(%llu)",
				RO(Shm)->SysGate.taskList[idx].comm,
				RO(Shm)->SysGate.taskList[idx].runtime);
		break;
	case F_UTIME:
		StrLenFormat(len, Buffer, TASK_COMM_LEN + 20 + 2,
				"%s(%llu)",
				RO(Shm)->SysGate.taskList[idx].comm,
				RO(Shm)->SysGate.taskList[idx].usertime);
		break;
	case F_STIME:
		StrLenFormat(len, Buffer, TASK_COMM_LEN + 20 + 2,
				"%s(%llu)",
				RO(Shm)->SysGate.taskList[idx].comm,
				RO(Shm)->SysGate.taskList[idx].systime);
		break;
	case F_PID:
		fallthrough;
	case F_COMM:
		StrLenFormat(len, Buffer, TASK_COMM_LEN + 11 + 2,
				"%s(%d)",
				RO(Shm)->SysGate.taskList[idx].comm,
				RO(Shm)->SysGate.taskList[idx].pid);
		break;
	}
      }
      if (dif >= len) {
	LayerCopyAt(layer,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].row,
		len, stateAttr, Buffer);

	cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col += len;
	/* Add a blank spacing between items: up to (ldx) chars	*/
       while ((dif = Draw.Size.width
		- cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col) > 0
		&& ldx--)
       {
	LayerAt(layer, attr,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].row) =
			RSC(UI).ATTR()[UI_DRAW_ALTMONITOR_TASKS_SPACE];
	LayerAt(layer, code,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].row) = 0x20;

	cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col++;
       }
      } else {
	LayerCopyAt(layer,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col,
		cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].row,
		dif, stateAttr, Buffer);

	cTask[RO(Shm)->SysGate.taskList[idx].wake_cpu].col += dif;
      }
     }
    }
   }
  }
  row += 2 + Draw.Area.MaxRows;
  return row;
}

void Draw_AltMonitor_Energy_Joule(void)
{
	StrFormat(Buffer, 9+9+9+9+1,
		"%8.4f" "%8.4f" "%8.4f" "%8.4f",
		RO(Shm)->Proc.State.Energy[PWR_DOMAIN(PKG)].Current,
		RO(Shm)->Proc.State.Energy[PWR_DOMAIN(CORES)].Current,
		RO(Shm)->Proc.State.Energy[PWR_DOMAIN(UNCORE)].Current,
		RO(Shm)->Proc.State.Energy[PWR_DOMAIN(RAM)].Current);
}

void Draw_AltMonitor_Power_Watt(void)
{
	StrFormat(Buffer, 9+9+9+9+1,
		"%8.4f" "%8.4f" "%8.4f" "%8.4f",
		RO(Shm)->Proc.State.Power[PWR_DOMAIN(PKG)].Current,
		RO(Shm)->Proc.State.Power[PWR_DOMAIN(CORES)].Current,
		RO(Shm)->Proc.State.Power[PWR_DOMAIN(UNCORE)].Current,
		RO(Shm)->Proc.State.Power[PWR_DOMAIN(RAM)].Current);
}

void (*Draw_AltMonitor_Power_Matrix[])(void) = {
	Draw_AltMonitor_Energy_Joule,
	Draw_AltMonitor_Power_Watt
};

CUINT Draw_AltMonitor_Power(Layer *layer, const unsigned int cpu, CUINT row)
{
	const CUINT col = LOAD_LEAD;
	UNUSED(cpu);

	row += 1 + Draw.Area.MaxRows;

	Draw_AltMonitor_Power_Matrix[Setting.jouleWatt]();

	memcpy(&LayerAt(layer, code, col +  1,	row), &Buffer[24], 8);
	memcpy(&LayerAt(layer, code, col + 22,	row), &Buffer[16], 8);
	memcpy(&LayerAt(layer, code, col + 44,	row), &Buffer[ 0], 8);
	memcpy(&LayerAt(layer, code, col + 64,	row), &Buffer[ 8], 8);

	row += 1;
	return row;
}

CUINT Draw_AltMonitor_Voltage(Layer *layer, const unsigned int cpu, CUINT row)
{
	const CUINT col = LOAD_LEAD + 8;
	UNUSED(cpu);

	row += 1 + Draw.Area.MaxRows;

	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	StrFormat(Buffer, 2*11+4*6+1,
			"%7d" "%05.4f" "%05.4f" "%05.4f" "%7d" "%05.4f",
			PFlop->Voltage.VID.CPU,
			RO(Shm)->Proc.State.Voltage.Limit[SENSOR_LOWEST],
			PFlop->Voltage.CPU,
			RO(Shm)->Proc.State.Voltage.Limit[SENSOR_HIGHEST],
			PFlop->Voltage.VID.SOC,
			PFlop->Voltage.SOC);

	memcpy(&LayerAt(layer, code, col,	row), &Buffer[ 0], 7);
	memcpy(&LayerAt(layer, code, col + 12,	row), &Buffer[ 7], 6);
	memcpy(&LayerAt(layer, code, col + 21,	row), &Buffer[13], 6);
	memcpy(&LayerAt(layer, code, col + 30,	row), &Buffer[19], 6);
	memcpy(&LayerAt(layer, code, col + 49,	row), &Buffer[25], 7);
	memcpy(&LayerAt(layer, code, col + 59,	row), &Buffer[32], 6);

	row += 1;
	return row;
}

#if defined(ARCH_PMC) && (ARCH_PMC == UMC)

#define CUST_CTR_FMT	" -- UMC [ "					\
			"%4llu "	"%4llu "	"%4llu "	\
			"%4llu "	"%4llu "	"%4llu "	\
			"%4llu "	"%4llu "	"MHz]"

#define CUST_CTR_VAR0	PFlop->Delta.PC02 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR1	PFlop->Delta.PC03 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR2	PFlop->Delta.PC04 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR3	PFlop->Delta.PC06 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR4	PFlop->Delta.PC07 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR5	PFlop->Delta.PC08 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR6	PFlop->Delta.PC09 / (1000LLU * 1000LLU)
#define CUST_CTR_VAR7	PFlop->Delta.PC10 / (1000LLU * 1000LLU)

#else

#define CUST_CTR_FMT	" c2:%-5.1f"	" c3:%-5.1f"	" c4:%-5.1f"	\
			" c6:%-5.1f"	" c7:%-5.1f"	" c8:%-5.1f"	\
			" c9:%-5.1f"	" c10:%-5.1f"

#define CUST_CTR_VAR0	100.f * RO(Shm)->Proc.State.PC02
#define CUST_CTR_VAR1	100.f * RO(Shm)->Proc.State.PC03
#define CUST_CTR_VAR2	100.f * RO(Shm)->Proc.State.PC04
#define CUST_CTR_VAR3	100.f * RO(Shm)->Proc.State.PC06
#define CUST_CTR_VAR4	100.f * RO(Shm)->Proc.State.PC07
#define CUST_CTR_VAR5	100.f * RO(Shm)->Proc.State.PC08
#define CUST_CTR_VAR6	100.f * RO(Shm)->Proc.State.PC09
#define CUST_CTR_VAR7	100.f * RO(Shm)->Proc.State.PC10

#endif /* ARCH_PMC */

#define CUST_INTEL(unit) " RAM:%8.4f(" COREFREQ_STRINGIFY(unit) ") -"	\
			"- SA:%8.4f(" COREFREQ_STRINGIFY(unit) ")/%5.4f(V)" \
			" - Pkg[%c]:%8.4f(" COREFREQ_STRINGIFY(unit) ") - " \
			"%5.4f  %5.4f  %5.4f  %8.4f %8.4f %8.4f -"

#define CUST_AMD(unit)	" RAM:%8.4f(" COREFREQ_STRINGIFY(unit) ") -"	\
			" SoC:%8.4f(" COREFREQ_STRINGIFY(unit) ")/%5.4f(V)" \
			" - Pkg[%c]:%8.4f(" COREFREQ_STRINGIFY(unit) ") - " \
			"%5.4f  %5.4f  %5.4f  %8.4f %8.4f %8.4f"

size_t Draw_AltMonitor_Custom_Energy_Joule(void)
{
	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	size_t len;
	StrLenFormat(len, Buffer, MAX_WIDTH,
			(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL ?
			CUST_INTEL(J) CUST_CTR_FMT : CUST_AMD(J) CUST_CTR_FMT),

			RO(Shm)->Proc.State.Energy[PWR_DOMAIN(RAM)].Current,
			RO(Shm)->Proc.State.Energy[PWR_DOMAIN(UNCORE)].Current,
			PFlop->Voltage.SOC,

		'0'+RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.PackageID,
			RO(Shm)->Proc.State.Energy[PWR_DOMAIN(PKG)].Current,

			RO(Shm)->Proc.State.Voltage.Limit[SENSOR_LOWEST],
			PFlop->Voltage.CPU,
			RO(Shm)->Proc.State.Voltage.Limit[SENSOR_HIGHEST],

	   RO(Shm)->Proc.State.Energy[PWR_DOMAIN(CORES)].Limit[SENSOR_LOWEST],
			RO(Shm)->Proc.State.Energy[PWR_DOMAIN(CORES)].Current,
	  RO(Shm)->Proc.State.Energy[PWR_DOMAIN(CORES)].Limit[SENSOR_HIGHEST],

			CUST_CTR_VAR0,
			CUST_CTR_VAR1,
			CUST_CTR_VAR2,
			CUST_CTR_VAR3,
			CUST_CTR_VAR4,
			CUST_CTR_VAR5,
			CUST_CTR_VAR6,
			CUST_CTR_VAR7);
	return len;
}

size_t Draw_AltMonitor_Custom_Power_Watt(void)
{
	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	size_t len;
	StrLenFormat(len, Buffer, MAX_WIDTH,
			(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL ?
			CUST_INTEL(W) CUST_CTR_FMT : CUST_AMD(W) CUST_CTR_FMT),

			RO(Shm)->Proc.State.Power[PWR_DOMAIN(RAM)].Current,
			RO(Shm)->Proc.State.Power[PWR_DOMAIN(UNCORE)].Current,
			PFlop->Voltage.SOC,

		'0'+RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Topology.PackageID,
			RO(Shm)->Proc.State.Power[PWR_DOMAIN(PKG)].Current,

			RO(Shm)->Proc.State.Voltage.Limit[SENSOR_LOWEST],
			PFlop->Voltage.CPU,
			RO(Shm)->Proc.State.Voltage.Limit[SENSOR_HIGHEST],

	   RO(Shm)->Proc.State.Power[PWR_DOMAIN(CORES)].Limit[SENSOR_LOWEST],
			RO(Shm)->Proc.State.Power[PWR_DOMAIN(CORES)].Current,
	  RO(Shm)->Proc.State.Power[PWR_DOMAIN(CORES)].Limit[SENSOR_HIGHEST],

			CUST_CTR_VAR0,
			CUST_CTR_VAR1,
			CUST_CTR_VAR2,
			CUST_CTR_VAR3,
			CUST_CTR_VAR4,
			CUST_CTR_VAR5,
			CUST_CTR_VAR6,
			CUST_CTR_VAR7);
	return len;
}

#undef CUST_CTR_FMT
#undef CUST_CTR_VAR0
#undef CUST_CTR_VAR1
#undef CUST_CTR_VAR2
#undef CUST_CTR_VAR3
#undef CUST_CTR_VAR4
#undef CUST_CTR_VAR5
#undef CUST_CTR_VAR6
#undef CUST_CTR_VAR7
#undef CUST_INTEL
#undef CUST_AMD

size_t (*Draw_AltMonitor_Custom_Matrix[])(void) = {
	Draw_AltMonitor_Custom_Energy_Joule,
	Draw_AltMonitor_Custom_Power_Watt
};

CUINT Draw_AltMonitor_Custom(Layer *layer, const unsigned int cpu, CUINT row)
{
	UNUSED(cpu);

	row += 1 + Draw.Area.MaxRows;

	size_t len = Draw_AltMonitor_Custom_Matrix[Setting.jouleWatt]();

	memcpy(&LayerAt(layer, code, 1, row), &Buffer[ 0], len);

	row += 1;
	return row;
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
	(SCOPE_OF_FORMULA(RO(Shm)->Proc.thermalFormula) != FORMULA_SCOPE_NONE),
	(SCOPE_OF_FORMULA(RO(Shm)->Proc.voltageFormula) != FORMULA_SCOPE_NONE)
	};

	StrFormat(Buffer, 10+5+1, Format_Temp_Voltage[ fmt[0] ] [ fmt[1] ],
			Cels2Fahr(PFlop->Thermal.Temp),
			PFlop->Voltage.CPU);
}

void Draw_Footer_Voltage_Celsius(struct PKG_FLIP_FLOP *PFlop)
{
	const enum FORMULA_SCOPE fmt[2] = {
	(SCOPE_OF_FORMULA(RO(Shm)->Proc.thermalFormula) != FORMULA_SCOPE_NONE),
	(SCOPE_OF_FORMULA(RO(Shm)->Proc.voltageFormula) != FORMULA_SCOPE_NONE)
	};

	StrFormat(Buffer, 10+5+1, Format_Temp_Voltage[ fmt[0] ] [ fmt[1] ],
			PFlop->Thermal.Temp,
			PFlop->Voltage.CPU);
}

void (*Draw_Footer_Voltage_Temp[])(struct PKG_FLIP_FLOP*) = {
	Draw_Footer_Voltage_Celsius,
	Draw_Footer_Voltage_Fahrenheit
};

void Draw_Footer(Layer *layer, CUINT row)
{	/* Update Footer view area					*/
	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	ATTRIBUTE *eventAttr[] = {
		RSC(HOT_EVENT_COND0).ATTR(),
		RSC(HOT_EVENT_COND1).ATTR(),
		RSC(HOT_EVENT_COND2).ATTR(),
		RSC(HOT_EVENT_COND3).ATTR(),
		RSC(HOT_EVENT_COND4).ATTR()
	};
	register unsigned int _hot = 0, _tmp = 0;

	if (!RO(Shm)->ProcessorEvents[eLOG] && !RO(Shm)->ProcessorEvents[eSTS])
	{
		_hot = 0;
		_tmp = 3;
	}
	else
	{
	    if ((RO(Shm)->ProcessorEvents[eLOG] & HOT_LOG_EVENT_FILTER)
	     || (RO(Shm)->ProcessorEvents[eSTS] & HOT_STS_EVENT_FILTER))
	    {
		_hot = 4;
		_tmp = 1;
	    } else {
		_hot = 2;
		_tmp = 3;
	    }
	}
	CUINT col[2] = {
		Draw.Area.Footer.VoltTemp.Hot[0],
		Draw.Area.Footer.VoltTemp.Hot[1]
	};

	LayerAt(layer, attr, col[1]	, row) = eventAttr[_hot][0];
	LayerAt(layer, attr, col[1] + 1 , row) = eventAttr[_hot][1];
	LayerAt(layer, attr, col[1] + 2 , row) = eventAttr[_hot][2];
	LayerAt(layer, attr, col[0] + 8 , row) = eventAttr[_tmp][0];
	LayerAt(layer, attr, col[0] + 9 , row) = eventAttr[_tmp][1];
	LayerAt(layer, attr, col[0] + 10, row) = eventAttr[_tmp][2];

	Draw_Footer_Voltage_Temp[Setting.fahrCels](PFlop);

	LayerAt(layer, code, col[0] + 8 , row) = (ASCII) Buffer[0];
	LayerAt(layer, code, col[0] + 9 , row) = (ASCII) Buffer[1];
	LayerAt(layer, code, col[0] + 10, row) = (ASCII) Buffer[2];
	LayerAt(layer, code, col[0]	, row) = (ASCII) Buffer[3];
	LayerAt(layer, code, col[0] + 1 , row) = (ASCII) Buffer[4];
	LayerAt(layer, code, col[0] + 2 , row) = (ASCII) Buffer[5];
	LayerAt(layer, code, col[0] + 3 , row) = (ASCII) Buffer[6];

	if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)
	&& (RO(Shm)->SysGate.tickStep == RO(Shm)->SysGate.tickReset)) {
		if ((Draw.Cache.procCount != RO(Shm)->SysGate.procCount)
		 || (Draw.Cache.FreeRAM != RO(Shm)->SysGate.memInfo.freeram))
		{
			Draw.Cache.procCount = RO(Shm)->SysGate.procCount;
			Draw.Cache.FreeRAM = RO(Shm)->SysGate.memInfo.freeram;

			PrintTaskMemory(layer, (row + 1),
					RO(Shm)->SysGate.procCount,
					RO(Shm)->SysGate.memInfo.freeram,
					RO(Shm)->SysGate.memInfo.totalram);
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
    if (!Draw.Flag.clkOrLd)
    {
	if (Draw.Load) {
		const unsigned int top = RO(Shm)->Proc.Top.Abs;
		CFlop = &RO(Shm)->Cpu[top].FlipFlop[!RO(Shm)->Cpu[top].Toggle];

		Clock2LCD(layer, 0,row, CFlop->Absolute.Freq,
					CFlop->Absolute.Ratio.Perf);
	} else {
		const unsigned int top = RO(Shm)->Proc.Top.Rel;
		CFlop = &RO(Shm)->Cpu[top].FlipFlop[!RO(Shm)->Cpu[top].Toggle];

		Clock2LCD(layer, 0,row, CFlop->Relative.Freq,
					CFlop->Relative.Ratio);
	}
    } else {
	if (Draw.Cache.TopAvg != RO(Shm)->Proc.Avg.C0) {
		double percent = 100.f * RO(Shm)->Proc.Avg.C0;
		Draw.Cache.TopAvg = RO(Shm)->Proc.Avg.C0;

		Load2LCD(layer, 0, row, percent);
	}
    }
	/* Print the focused BCLK					*/
	row += 2;

	CFlop = &RO(Shm)->Cpu[ Draw.iClock + Draw.cpuScroll ]		\
		.FlipFlop[ !RO(Shm)->Cpu[Draw.iClock + Draw.cpuScroll].Toggle ];

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
	Layout_Monitor_Slice,
	Layout_Monitor_Custom
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
	Layout_Ruler_Slice,
	Layout_Ruler_Custom
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
	Draw_Monitor_Slice,
	Draw_Monitor_Custom
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
	Draw_AltMonitor_Common,
	Draw_AltMonitor_Custom
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
		LayerAt(_layer, attr, _col, (1 + _row + Draw.Area.MaxRows)) =
#else
	#define Illuminates_Lower_CPU_At(_layer, _col, _row)		\
		LayerAt(_layer, attr, _col, (_row)) =
#endif
#else
	#define Illuminates_Lower_CPU_At(_layer, _col, _row)
#endif

#define Illuminates_CPU(_layer, _row, _attr)				\
({									\
	Illuminates_Upper_CPU_At(_layer, 0, _row)			\
	Illuminates_Lower_CPU_At(_layer, 0, _row)			\
	Illuminates_Upper_CPU_At(_layer, 1, _row)			\
	Illuminates_Lower_CPU_At(_layer, 1, _row)			\
	Illuminates_Upper_CPU_At(_layer, 2, _row)			\
	Illuminates_Lower_CPU_At(_layer, 2, _row)			\
									\
						_attr;			\
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
  for (cpu = Draw.cpuScroll; cpu < (Draw.cpuScroll + Draw.Area.MaxRows); cpu++)
  {
	Layout_CPU_To_String(cpu);
#ifndef NO_UPPER
	Layout_CPU_To_View(layer, 0, row + 1);

	Layout_BCLK_To_View(layer, 0, row + 1);
#endif
	row = row + 1;
#ifndef NO_LOWER
#ifndef NO_UPPER
    if (Draw.View != V_PACKAGE) {
	Layout_CPU_To_View(layer, 0, row + Draw.Area.MaxRows + 1);
    }
#else
    if (Draw.View != V_PACKAGE) {
	Layout_CPU_To_View(layer, 0, row);
    }
#endif
#endif
    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
    {
      if (cpu == RO(Shm)->Proc.Service.Core) {
	Illuminates_CPU(layer, row, RSC(UI).ATTR()[UI_ILLUMINATES_CPU_SP]);
      } else if ((signed short) cpu == RO(Shm)->Proc.Service.Thread) {
	Illuminates_CPU(layer, row, RSC(UI).ATTR()[UI_ILLUMINATES_CPU_SP]);
      } else if ((signed short) cpu == RO(Shm)->Proc.Service.Hybrid) {
	Illuminates_CPU(layer,row,RSC(UI).ATTR()[UI_ILLUMINATES_CPU_SP_HYBRID]);
      } else {
	Illuminates_CPU(layer, row, RSC(UI).ATTR()[UI_ILLUMINATES_CPU_ON]);
      }
#ifndef NO_LOWER
#ifndef NO_UPPER
	Matrix_Layout_Monitor[Draw.View](layer,cpu,row + Draw.Area.MaxRows + 1);
#else
	Matrix_Layout_Monitor[Draw.View](layer,cpu,row);
#endif
#endif
    }
    else
    {
	Illuminates_CPU(layer, row, RSC(UI).ATTR()[UI_ILLUMINATES_CPU_OFF]);

#ifndef NO_UPPER
	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), row,
			(Draw.Size.width - LOAD_LEAD + 1), 0x0);
#endif
#ifndef NO_LOWER
#ifndef NO_UPPER
	ClearGarbage(	dLayer, attr,
			(LOAD_LEAD - 1), (row + Draw.Area.MaxRows + 1),
			(Draw.Size.width - LOAD_LEAD + 1),
			RSC(UI).ATTR()[UI_LAYOUT_ROW_CPU_OFFLINE].value );

	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), (row + Draw.Area.MaxRows + 1),
			(Draw.Size.width - LOAD_LEAD + 1), 0x0 );
#else
	ClearGarbage(	dLayer, code,
			(LOAD_LEAD - 1), row,
			(Draw.Size.width - LOAD_LEAD + 1), 0x0 );
#endif
#endif
    }
  }
	row++;
#ifndef NO_LOWER
#ifndef NO_UPPER
	row = Matrix_Layout_Ruler[Draw.View](layer, 0, row);
#else
	row = Matrix_Layout_Ruler[Draw.View](layer, 0, TOP_HEADER_ROW);
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
#ifndef NO_HEADER
	Draw_Header(layer, row);
#endif
	row += TOP_HEADER_ROW;

  for (cpu = Draw.cpuScroll; cpu < (Draw.cpuScroll + Draw.Area.MaxRows); cpu++)
  {
	row++;

    if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS))
    {
      if (!BITVAL(RO(Shm)->Cpu[cpu].OffLine, HW))
      {
#ifndef NO_UPPER
	Matrix_Draw_Load[Draw.Load](layer, cpu, row);
#endif
      }
	/*	Print the Per Core BCLK indicator (yellow)		*/
#ifdef NO_UPPER
      if (Draw.View != V_PACKAGE) {
#endif
#if !defined(NO_LOWER) || !defined(NO_UPPER)
	LayerAt(layer, code, (LOAD_LEAD - 1), row) =
			(Draw.iClock == (cpu - Draw.cpuScroll)) ? '~' : 0x20;
#endif
#ifdef NO_UPPER
      }
#endif

#ifndef NO_LOWER
#ifndef NO_UPPER
	Matrix_Draw_Monitor[Draw.View](layer, cpu, row + Draw.Area.MaxRows + 1);
#else
	Matrix_Draw_Monitor[Draw.View](layer, cpu, row);
#endif
#endif
    }
  }
	row++;
#ifndef NO_LOWER
#ifndef NO_UPPER
	row = Matrix_Draw_AltMon[Draw.View](layer, 0, row);
#else
	row = Matrix_Draw_AltMon[Draw.View](layer, 0, TOP_HEADER_ROW);
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

	if (!BITVAL(RO(Shm)->Cpu[_cpu].OffLine, OS))
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
		(4 * INTER_WIDTH), " _  _  _  _ ",
		RSC(UI).ATTR()[UI_LAYOUT_CARD_CORE_OFFLINE]);

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

	StrFormat(Buffer, 10+1, "x%2u",
			RO(Shm)->Uncore.Boost[UNCORE_BOOST(MAX)]);

	hUncore.code[ 8] = (ASCII) Buffer[0];
	hUncore.code[ 9] = (ASCII) Buffer[1];
	hUncore.code[10] = (ASCII) Buffer[2];

	LayerCopyAt(	layer, hUncore.origin.col, hUncore.origin.row,
			hUncore.length, hUncore.attr, hUncore.code);
}

void Layout_Card_Bus(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_BUS, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hBus);

	StrFormat(Buffer, 10+1, "%4u", RO(Shm)->Uncore.Bus.Rate);
	hBus.code[5] = (ASCII) Buffer[0];
	hBus.code[6] = (ASCII) Buffer[1];
	hBus.code[7] = (ASCII) Buffer[2];
	hBus.code[8] = (ASCII) Buffer[3];

	switch (RO(Shm)->Uncore.Unit.Bus_Rate) {
	case MC_MHZ:
		hBus.code[ 9] = 'H';
		hBus.code[10] = 'z';
		break;
	case MC_MTS:
		hBus.code[ 9] = 'M';
		hBus.code[10] = 'T';
		break;
	case MC_MBS:
		hBus.code[ 9] = 'B';
		hBus.code[10] = 's';
		break;
	}

	LayerCopyAt(	layer, hBus.origin.col, hBus.origin.row,
			hBus.length, hBus.attr, hBus.code);

	Counter2LCD(	layer, card->origin.col, card->origin.row,
			(double) RO(Shm)->Uncore.Bus.Speed);
}

static char Card_MC_Timing[(5*10)+5+1];

void Layout_Card_MC(Layer *layer, Card *card)
{
	LayerDeclare(	LAYOUT_CARD_MC, (4 * INTER_WIDTH),
			card->origin.col, (card->origin.row + 3),
			hRAM);

	card->data.word.lo[0] = 0;
	StrLenFormat(card->data.word.lo[1], Card_MC_Timing,
			(5*10)+5+1, "%s", " -  -  -  -  - " );

	unsigned short mc, cha, gotTimings = 0;
  if (RO(Shm)->Uncore.CtrlCount > 0) {
    for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount && !gotTimings; mc++) {
      if (RO(Shm)->Uncore.MC[mc].ChannelCount > 0) {
	for (cha = 0;
		cha < RO(Shm)->Uncore.MC[mc].ChannelCount && !gotTimings;
			cha++)
	{
	  if (RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tCL
	   || RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tRCD
	   || RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tRP
	   || RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tRAS)
	  {
		StrLenFormat(card->data.word.lo[1],
			Card_MC_Timing, (5*10)+5+1, "%u-%u-%u-%u-%uT",
			RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tCL,
			RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tRCD,
			RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tRP,
			RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.tRAS,
			RO(Shm)->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);

		gotTimings = 1;
	  }
	}
      }
    }
	Counter2LCD(layer, card->origin.col, card->origin.row,
			(double) RO(Shm)->Uncore.CtrlSpeed);
  }
	LayerCopyAt(	layer, hRAM.origin.col, hRAM.origin.row,
			hRAM.length, hRAM.attr, hRAM.code);
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

    if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)) {
	unsigned long totalRAM, totalDimm = 0;
	int unit;
	char symbol = 0x20;

      if (RO(Shm)->Uncore.CtrlCount > 0) {
	unsigned short mc, cha, slot;
	for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
	  for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
	    for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++) {
		totalDimm +=RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size;
	    }
	unit = ByteReDim(totalDimm, 3, &totalRAM);
	if ((unit >= 0) && (unit < 4)) {
		char symbols[4] = {'M', 'G', 'T', 'P'};
		symbol = symbols[unit];
	}
      } else {
	unit = ByteReDim(RO(Shm)->SysGate.memInfo.totalram, 3, &totalRAM);
	if ((unit >= 0) && (unit < 4)) {
		char symbols[4] = {'K', 'M', 'G', 'T'};
		symbol = symbols[unit];
	}
      }
      if (totalRAM > 99) {
	hMem.attr[hMem.length-2] = RSC(UI).ATTR()[UI_LAYOUT_CARD_RAM_3DIGITS];
	StrFormat(Buffer, 20+1+1, "%3lu", totalRAM);
      } else {
	hMem.attr[hMem.length-2] = RSC(UI).ATTR()[UI_LAYOUT_CARD_RAM_2DIGITS];
	StrFormat(Buffer, 20+1+1, "%2lu%c", totalRAM, symbol);
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

	if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)) {
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
	const CUINT	rightEdge = Draw.Size.width - marginWidth,
			bottomEdge = Draw.Size.height + marginHeight;

	coord->col += marginWidth;
	if (coord->col > rightEdge) {
		coord->col = LEADING_LEFT;
		coord->row += marginHeight;
	}
	if (coord->row > bottomEdge) {
		return RENDER_KO;
	}
	return RENDER_OK;
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
		thermalScope = SCOPE_OF_FORMULA(RO(Shm)->Proc.thermalFormula);

	unsigned int _cpu = card->data.dword.lo;

	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[_cpu].FlipFlop[!RO(Shm)->Cpu[_cpu].Toggle];

	ATTRIBUTE warning = RSC(UI).ATTR()[UI_DRAW_CARD_CORE_NOMINAL];

	Clock2LCD(layer, card->origin.col, card->origin.row,
			CFlop->Relative.Freq, CFlop->Relative.Ratio);

	if (CFlop->Thermal.Temp <= RO(Shm)->Cpu[
						_cpu
					].PowerThermal.Limit[SENSOR_LOWEST])
	{
		warning = RSC(UI).ATTR()[UI_DRAW_CARD_CORE_LOW];
	} else {
	    if(CFlop->Thermal.Temp >= RO(Shm)->Cpu[
						_cpu
					].PowerThermal.Limit[SENSOR_HIGHEST])
	    {
		warning = RSC(UI).ATTR()[UI_DRAW_CARD_CORE_MEDIUM];
	    }
	}
	if (CFlop->Thermal.Events[eSTS]) {
		warning = RSC(UI).ATTR()[UI_DRAW_CARD_CORE_HIGH];
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
	if ( !(_cpu == RO(Shm)->Proc.Service.Core) ) {
		break;
	}
	fallthrough;
    case FORMULA_SCOPE_CORE:
	if (	!( (RO(Shm)->Cpu[_cpu].Topology.ThreadID == 0)
		|| (RO(Shm)->Cpu[_cpu].Topology.ThreadID == -1)) ) {
		break;
	}
	fallthrough;
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
	struct FLIP_FLOP *CFlop = \
		&RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
			!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
	];
	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	double bclk = (double)(PFlop->Delta.PCLK / RO(Shm)->Sleep.Interval);

	Counter2LCD(layer, card->origin.col, card->origin.row, bclk);

	StrFormat(Buffer, 6+1, "%5.1f", CLOCK_MHz(double, CFlop->Clock.Hz));

	memcpy(&LayerAt(layer, code, (card->origin.col+2),(card->origin.row+3)),
		Buffer, 5);
}

void Draw_Card_Uncore(Layer *layer, Card *card)
{
	struct PKG_FLIP_FLOP *PFlop = \
		&RO(Shm)->Proc.FlipFlop[!RO(Shm)->Proc.Toggle];

	double Uncore = CLOCK_MHz(double, PFlop->Uncore.FC0);

	Idle2LCD(layer, card->origin.col, card->origin.row, Uncore);
}

void Draw_Card_MC(Layer *layer, Card *card)
{
	memcpy( &LayerAt(layer,code, (card->origin.col+1),(card->origin.row+3)),
		&Card_MC_Timing[card->data.word.lo[0]], 10);

	card->data.word.lo[0]++;
	if ((card->data.word.lo[0] + 10) > card->data.word.lo[1]) {
		card->data.word.lo[0] = 0;
	}
}

void Draw_Card_Load(Layer *layer, Card* card)
{
	double percent = 100.f * RO(Shm)->Proc.Avg.C0;

	Load2LCD(layer, card->origin.col, card->origin.row, percent);
}

void Draw_Card_Idle(Layer *layer, Card* card)
{
	double percent =( RO(Shm)->Proc.Avg.C1
			+ RO(Shm)->Proc.Avg.C3
			+ RO(Shm)->Proc.Avg.C6
			+ RO(Shm)->Proc.Avg.C7 ) * 100.f;

	Idle2LCD(layer, card->origin.col, card->origin.row, percent);
}

void Draw_Card_RAM(Layer *layer, Card *card)
{
    if (card->data.dword.hi == RENDER_OK)
    {
      if (RO(Shm)->SysGate.tickStep == RO(Shm)->SysGate.tickReset) {
	unsigned long freeRAM;
	int unit;
	char symbol[4] = {'K', 'M', 'G', 'T'};
	double percent = (100.f * RO(Shm)->SysGate.memInfo.freeram)
				/ RO(Shm)->SysGate.memInfo.totalram;

	Sys2LCD(layer, card->origin.col, card->origin.row, percent);

	unit = ByteReDim(RO(Shm)->SysGate.memInfo.freeram, 6, &freeRAM);
	StrFormat(Buffer, 20+1+1, "%5lu%c", freeRAM, symbol[unit]);
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
      if (RO(Shm)->SysGate.tickStep == RO(Shm)->SysGate.tickReset) {
	size_t	pb, pe;
	const int cl = (int) strnlen(RO(Shm)->SysGate.taskList[0].comm, 12),
		hl = ((12 - cl) / 2) % 12, hr = (hl + (cl & 1)) % 12;
	char	stateStr[TASK_COMM_LEN];
	ATTRIBUTE *stateAttr;
	stateAttr = StateToSymbol(RO(Shm)->SysGate.taskList[0].state, stateStr);

	StrFormat(Buffer, 3*12+11+3+5+1+1, "%.*s%.*s%.*s%zn%7d(%c)%zn%5u",
				hl, hSpace,
				cl, RO(Shm)->SysGate.taskList[0].comm,
				hr, hSpace,
			(ssize_t*)&pb,
				RO(Shm)->SysGate.taskList[0].pid,
				stateStr[0],
			(ssize_t*)&pe,
				RO(Shm)->SysGate.procCount);

	LayerCopyAt(layer,	(card->origin.col + 0),
				(card->origin.row + 1),
				pb,
				stateAttr,
				Buffer);

	LayerFillAt(layer,	(card->origin.col + 1),
				(card->origin.row + 2),
				pe - pb,
				&Buffer[pb],
				RSC(UI).ATTR()[UI_DRAW_CARD_TASK_FILL]);

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
  for (cpu=0; (cpu < RO(Shm)->Proc.CPU.Count) && !BITVAL(Shutdown, SYNC); cpu++)
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
  }
  else if ((cTask=calloc(RO(Shm)->Proc.CPU.Count, sizeof(Coordinate))) == NULL)
  {
	REASON_SET(reason, RC_MEM_ERR);
  }
  else if (AllocAll(&Buffer) == ENOMEM) {
	REASON_SET(reason, RC_MEM_ERR, ENOMEM);
  }
  else
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

	Draw.Disposal = (option == 'd') ? D_DASHBOARD : D_MAINVIEW;

	AggregateRatio();

	RECORDER_COMPUTE(Recorder, RO(Shm)->Sleep.Interval);

	LoadGeometries(BuildConfigFQN("CoreFreq"));

	SET_THEME(Draw.Theme);

	/* MAIN LOOP */
    while (!BITVAL(Shutdown, SYNC))
    {
      do
      {
	if ((Draw.Flag.daemon=BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, SYNC0)) == 0)
	{
		SCANKEY scan = {.key = 0};

	  if (GetKey(&scan, &RO(Shm)->Sleep.pollingWait) > 0) {
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
	if (!RING_NULL(RW(Shm)->Error))
	{
		RING_CTRL ctrl __attribute__ ((aligned(16)));
		RING_READ(RW(Shm)->Error, ctrl);

		AppendWindow(
			PopUpMessage(RSC(POPUP_DRIVER_TITLE).CODE(), &ctrl),
			&winList );
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, COMP0)) {
		AggregateRatio();
		Draw.Flag.clear = 1;	/* Compute required,clear the layout */
	}
	if (BITCLR(LOCKLESS, RW(Shm)->Proc.Sync, NTFY0)) {
		ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);
		RECORDER_COMPUTE(Recorder, RO(Shm)->Sleep.Interval);
		Draw.Flag.layout = 1;	 /* Platform changed, redraw layout */
	}
      } while ( !BITVAL(Shutdown, SYNC)
		&& !Draw.Flag.daemon
		&& !Draw.Flag.layout
		&& !Draw.Flag.clear ) ;

      if (Draw.Flag.height & Draw.Flag.width)
      {
	if (Draw.Flag.clear) {
		Draw.Flag.clear  = 0;
		Draw.Flag.layout = 1;
		ResetLayer(dLayer, RSC(UI).ATTR()[UI_FUSE_RESET_LAYER]);
	}
	if (Draw.Flag.layout) {
		Draw.Flag.layout = 0;
		ResetLayer(sLayer, RSC(UI).ATTR()[UI_FUSE_PAINT_LAYER]);
		LayoutView[Draw.Disposal](sLayer);
	}
	if (Draw.Flag.daemon)
	{
		DynamicView[Draw.Disposal](dLayer);

		/* Increment the BCLK indicator (skip offline CPU)	*/
		do {
			Draw.iClock++;
			if (Draw.iClock >= Draw.Area.MaxRows) {
				Draw.iClock = 0;
			}
		} while (BITVAL(RO(Shm)->Cpu[Draw.iClock].OffLine, OS)
			&& (Draw.iClock != RO(Shm)->Proc.Service.Core)) ;
	}
	Draw_uBenchmark(dLayer);
	UBENCH_RDCOUNTER(1);

	/* Write to the standard output.				*/
	Draw.Flag.layout = WriteConsole(Draw.Size);

	UBENCH_RDCOUNTER(2);
	UBENCH_COMPUTE();
      } else {
	fprintf(stderr, CUH RoK "Term(%d x %d) < View(%d x %u)\n",
		Draw.Size.width,Draw.Size.height,MIN_WIDTH,Draw.Area.MinHeight);
      }
    }
    SaveGeometries(BuildConfigFQN("CoreFreq"));
  }
	FreeAll(Buffer);

  if (cTask != NULL) {
	free(cTask);
  }
	DestroyAllCards(&cardList);

	return reason;
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
	return reason;
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
		fallthrough;
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
		RING_WRITE_SUB_CMD(	SESSION_CLI, RW(Shm)->Ring[1],
					COREFREQ_SESSION_APP, (pid_t) 0 );
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

		RING_WRITE_SUB_CMD(	SESSION_CLI, RW(Shm)->Ring[1],
					COREFREQ_SESSION_APP, getpid() );

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

CHECK_DUPLICATE_KEY(0LLU);

int main(int argc, char *argv[])
{
	struct {
		struct stat ro, rw;
	} stat_st = {
		.ro = {0},
		.rw = {0}
	};
	struct
	{
		int ro, rw;
	} fd = {
		.ro = -1,
		.rw = -1
	};
	int	idx = 0;
	char	*program = strdup(argv[0]),
		*appName = program != NULL ? basename(program) : argv[idx],
		option = 't', trailing = '\0';

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
  } else if (	((fd.ro = shm_open(RO(SHM_FILENAME), O_RDONLY,
				S_IRUSR|S_IWUSR
				|S_IRGRP|S_IROTH) ) !=-1)
	&&	((fd.rw = shm_open(RW(SHM_FILENAME), O_RDWR,
				 S_IRUSR|S_IWUSR
				|S_IRGRP|S_IWGRP
				|S_IROTH|S_IWOTH) ) !=-1) )
  {
    if ((fstat(fd.ro, &stat_st.ro) != -1)
     && (fstat(fd.rw, &stat_st.rw) != -1))
    {
	const size_t	roSize = (size_t) stat_st.ro.st_size,
			rwSize = (size_t) stat_st.rw.st_size;

      if (((RO(Shm) =	mmap(	NULL, roSize,
				PROT_READ, MAP_SHARED,
				fd.ro, 0)) != MAP_FAILED )
       && ((RW(Shm) =	mmap(	NULL, rwSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.rw, 0)) != MAP_FAILED ))
      {
       if (CHK_FOOTPRINT(RO(Shm)->FootPrint,MAX_FREQ_HZ,
					CORE_COUNT,
					TASK_ORDER,
					COREFREQ_MAJOR,
					COREFREQ_MINOR,
					COREFREQ_REV))
       {
	ClientFollowService(&localService, &RO(Shm)->Proc.Service, 0);

  #define CONDITION_RDTSCP()						\
	(  (RO(Shm)->Proc.Features.AdvPower.EDX.Inv_TSC == 1)		\
	|| (RO(Shm)->Proc.Features.ExtInfo.EDX.RDTSCP == 1) )

  #define CONDITION_RDPMC()						\
	(  (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)	\
	&& (RO(Shm)->Proc.PM_version >= 1)				\
	&& (BITVAL(RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].SystemRegister.CR4,\
							CR4_PCE) == 1) )

	UBENCH_SETUP(CONDITION_RDTSCP(), CONDITION_RDPMC());

	do {
	    switch (option) {
	    case 'n':
		printf("\n");
		break;
	    case 'O':
		switch (argv[idx][2]) {
		case 'a':
			Draw.Load = 1;
			break;
		case 'k':
			Draw.Unit.Memory = 10 * 0;
			break;
		case 'm':
			Draw.Unit.Memory = 10 * 1;
			break;
		case 'g':
			Draw.Unit.Memory = 10 * 2;
			break;
		case 'F':
			Setting.fahrCels = 1;
			break;
		case 'J':
		    if (++idx < argc) {
			enum SMB_STRING usrIdx = SMB_BOARD_NAME;
			if ((sscanf(argv[idx], "%u%c", &usrIdx, &trailing) != 1)
			 || (usrIdx >= SMB_STRING_COUNT)) {
				goto SYNTAX_ERROR;
			} else {
				Draw.SmbIndex = usrIdx;
			}
		    }
			break;
		case 'Y':
			Setting.secret = 0;
			break;
		case 'E':
		    if (++idx < argc) {
			enum THEMES theme;
			if ((sscanf(argv[idx], "%u%c", &theme, &trailing) != 1)
			|| (theme >= THM_CNT)) {
				goto SYNTAX_ERROR;
			} else {
				Draw.Theme = theme;
			}
		    }
			break;
		default: /* `/0' */
			goto SYNTAX_ERROR;
			break;
		}
		break;
	    case 'B':
		reason = SysInfoSMBIOS(NULL, 80, NULL);
		break;
	    case 'k':
		if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1)) {
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
		JsonSysInfo(RO(Shm), RW(Shm), NULL);
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

		switch (RO(Shm)->Uncore.Unit.DDR_Ver) {
		case 5:
		case 4:
		    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL) {
			MemoryController(&tty, NULL, Timing_DDR4);
		    }
		 else if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
		       || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
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
		{
			int iter = -1;
		    if (++idx < argc) {
			if ((sscanf(argv[idx], "%d%c", &iter, &trailing) != 1)
				|| (iter <= 0))
			{
				goto SYNTAX_ERROR;
			}
		    }
			TrapSignal(1);
			Instructions((unsigned int) iter);
			TrapSignal(0);
		}
		break;
	    case 'c':
		{
			int iter = -1;
		    if (++idx < argc) {
			if ((sscanf(argv[idx], "%d%c", &iter, &trailing) != 1)
				|| (iter <= 0))
			{
				goto SYNTAX_ERROR;
			}
		    }
			TrapSignal(1);
			Counters((unsigned int) iter);
			TrapSignal(0);
		}
		break;
	    case 'C':
		{
			int iter = -1;
		    if (++idx < argc) {
			if ((sscanf(argv[idx], "%d%c", &iter, &trailing) != 1)
				|| (iter <= 0))
			{
				goto SYNTAX_ERROR;
			}
		    }
			TrapSignal(1);
			Sensors((unsigned int) iter);
			TrapSignal(0);
		}
		break;
	    case 'V':
		{
			int iter = -1;
		    if (++idx < argc) {
			if ((sscanf(argv[idx], "%d%c", &iter, &trailing) != 1)
				|| (iter <= 0))
			{
				goto SYNTAX_ERROR;
			}
		    }
			TrapSignal(1);
			Voltage((unsigned int) iter);
			TrapSignal(0);
		}
		break;
	    case 'W':
		{
			int iter = -1;
		    if (++idx < argc) {
			if ((sscanf(argv[idx], "%d%c", &iter, &trailing) != 1)
				|| (iter <= 0))
			{
				goto SYNTAX_ERROR;
			}
		    }
			TrapSignal(1);
			Power((unsigned int) iter);
			TrapSignal(0);
		}
		break;
	    case 'g':
		{
			int iter = -1;
		    if (++idx < argc) {
			if ((sscanf(argv[idx], "%d%c", &iter, &trailing) != 1)
				|| (iter <= 0))
			{
				goto SYNTAX_ERROR;
			}
		    }
			TrapSignal(1);
			Package((unsigned int) iter);
			TrapSignal(0);
		}
		break;
	    case 'd':
		fallthrough;
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
				{	V_SLICE 	, "slices"	},
				{	V_CUSTOM	, "custom"	}
			}, *pOpt, *lOpt = &opt[sizeof(opt)/sizeof(opt[0])];
		    for (pOpt = opt; pOpt < lOpt; pOpt++)
		    {
			const size_t llen = strlen(pOpt->first);
			const size_t rlen = strlen(argv[idx]);
			const ssize_t nlen = (ssize_t) llen - (ssize_t) rlen;
			if ((nlen >= 0)
			&& (strncmp(pOpt->first, argv[idx], rlen) == 0))
			{
				Draw.View = pOpt->view;
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

	if (munmap(RO(Shm), roSize) == -1) {
		REASON_SET(reason, RC_SHM_MMAP);
		reason = Help(reason, RO(SHM_FILENAME));
	}
	if (munmap(RW(Shm), rwSize) == -1) {
		REASON_SET(reason, RC_SHM_MMAP);
		reason = Help(reason, RW(SHM_FILENAME));
	}
       } else {
		char *wrongVersion = malloc(10+5+5+5+1);
		REASON_SET(reason, RC_SHM_MMAP, EPERM);
		if (wrongVersion != NULL) {
			StrFormat(wrongVersion, 10+5+5+5+1,
				"Version %hu.%hu.%hu",
				RO(Shm)->FootPrint.major,
				RO(Shm)->FootPrint.minor,
				RO(Shm)->FootPrint.rev);
			reason = Help(reason, wrongVersion);
			free(wrongVersion);
		}
		munmap(RO(Shm), roSize);
		munmap(RW(Shm), rwSize);
       }
      } else {
		REASON_SET(reason, RC_SHM_MMAP);
		reason = Help(reason, RO(SHM_FILENAME));
      }
    } else {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, RO(SHM_FILENAME));
    }
    if (close(fd.ro) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, RO(SHM_FILENAME));
    }
    if (close(fd.rw) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, RO(SHM_FILENAME));
    }
  } else {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, RO(SHM_FILENAME));
  }
    LOCALE(OUT);

    if (program != NULL) {
	free(program);
    }
    return reason.rc;
}
