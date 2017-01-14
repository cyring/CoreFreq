/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <math.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"

unsigned int Shutdown = 0x0;

void Emergency(int caught)
{
	switch (caught) {
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
	case SIGTSTP:
		Shutdown = 0x1;
		break;
	}
}

void TrapSignal(void)
{
	signal(SIGINT, Emergency);
	signal(SIGQUIT, Emergency);
	signal(SIGTERM, Emergency);
	signal(SIGTSTP, Emergency);	// [CTRL] + [Z]
}

// VT100 requirements.
#define SCP	"\033[s"
#define RCP	"\033[u"
#define HIDE	"\033[?25l"
#define SHOW	"\033[?25h"
#define RESET	"\033c"
#define SCR1	"\033[?47h"
#define SCR0	"\033[?47l"
#define CLS	"\033[J"
#define CUH	"\033[H"
#define CUP(col, row) "\033["#row";"#col"H"

#define BLACK	0
#define RED	1
#define GREEN	2
#define YELLOW	3
#define BLUE	4
#define MAGENTA	5
#define CYAN	6
#define WHITE	7

#define _COLOR(_mod, _fg, _bg) "\033["#_mod";3"#_fg";4"#_bg"m"
#define COLOR(mod, fg, bg) _COLOR(mod, fg, bg)

#define AoK	COLOR(0, WHITE, BLACK)
#define DoK	COLOR(1, BLACK, BLACK)
#define RoK	COLOR(1, RED, BLACK)
#define GoK	COLOR(1, GREEN, BLACK)
#define YoK	COLOR(1, YELLOW, BLACK)
#define BoK	COLOR(1, BLUE, BLACK)
#define MoK	COLOR(1, MAGENTA, BLACK)
#define CoK	COLOR(1, CYAN, BLACK)
#define WoK	COLOR(1, WHITE, BLACK)

char lcd[10][3][3] = {
	{// 0
		" _ ",
		"| |",
		"|_|"
	},
	{// 1
		"   ",
		" | ",
		" | "
	},
	{// 2
		" _ ",
		" _|",
		"|_ "
	},
	{// 3
		" _ ",
		" _|",
		" _|"
	},
	{// 4
		"   ",
		"|_|",
		"  |"
	},
	{// 5
		" _ ",
		"|_ ",
		" _|"
	},
	{// 6
		" _ ",
		"|_ ",
		"|_|"
	},
	{// 7
		" _ ",
		"  |",
		"  |"
	},
	{// 8
		" _ ",
		"|_|",
		"|_|"
	},
	{// 9
		" _ ",
		"|_|",
		" _|"
	}
};

#define TOP_HEADER_ROW	3
#define TOP_FOOTER_ROW	2
#define TOP_SEPARATOR	3
#define MAX_CPU_ROW	32

#define MAX_HEIGHT	((2 * MAX_CPU_ROW)				\
			+ TOP_HEADER_ROW				\
			+ TOP_SEPARATOR					\
			+ TOP_FOOTER_ROW)
#define MAX_WIDTH	132
#define MIN_WIDTH	80

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

typedef struct {
	int	width,
		height;
} SCREEN_SIZE;

SCREEN_SIZE GetScreenSize(void)
{
	SCREEN_SIZE _screenSize = {.width = 0, .height = 0};
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	_screenSize.width  = (int) ts.ws_col;
	_screenSize.height = (int) ts.ws_row;
	return(_screenSize);
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

void printv(	void(*OutFunc)(char *output),
		unsigned short width, int tab, char *fmt, ...)
{
	const char *indent[2][4] = {
		{"",	"|",	"|- ",	"   |- "},
		{"",	" ",	"  ",	"   "}
	};
	char	*line = malloc(width + 1);

	va_list ap;
	va_start(ap, fmt);
	vsprintf(line, fmt, ap);

	if (OutFunc == NULL)
	    printf("%s%s%.*s\n", indent[0][tab], line,
		(int)(width - strlen(line) - strlen(indent[0][tab])), hSpace);
	else {
	    char *output = malloc(width + 1);
	    sprintf(output, "%s%s%.*s", indent[1][tab], line,
		(int)(width - strlen(line) - strlen(indent[1][tab])), hSpace);
	    OutFunc(output);
	    free(output);
	}
	va_end(ap);
	free(line);
}

void SysInfoCPUID(SHM_STRUCT *Shm,
		unsigned short width,
		void(*OutFunc)(char *output) )
{
	char format[] = "%08x:%08x%.*s%08x     %08x     %08x     %08x";
	unsigned int cpu;
	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	    if (OutFunc == NULL) {
		printv(OutFunc, width, 0,
			"CPU #%-2u function"				\
			"          EAX          EBX          ECX          EDX",
			cpu);
	    } else {
		printv(OutFunc, width, 0, "CPU #%-2u", cpu);
	    }
		printv(OutFunc,width, 2, format,
			0x00000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc,
			Shm->Cpu[cpu].Query.StdFunc.BX,
			Shm->Cpu[cpu].Query.StdFunc.CX,
			Shm->Cpu[cpu].Query.StdFunc.DX);

		printv(OutFunc, width, 3, "Largest Standard Function=%08x",
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc);

		printv(OutFunc,width, 2, format,
			0x80000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc,
			Shm->Cpu[cpu].Query.ExtFunc.BX,
			Shm->Cpu[cpu].Query.ExtFunc.CX,
			Shm->Cpu[cpu].Query.ExtFunc.DX);

		printv(OutFunc, width, 3, "Largest Extended Function=%08x",
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc);

		int i;
		for (i = 0; i < CPUID_MAX_FUNC; i++)
		    if (Shm->Cpu[cpu].CpuID[i].func) {
			printv(OutFunc,width, 2, format,
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

void SysInfoProc(SHM_STRUCT *Shm,
		unsigned short width,
		void(*OutFunc)(char *output))
{
	size_t	len = 0;
	char	*row = malloc(width + 1),
		*str = malloc(width + 1),
		*pad = NULL;
	int	i = 0;

/* Section Mark */
	printv(OutFunc, width, 0, "Processor%.*s[%s]",
		width - 11 - strlen(Shm->Proc.Brand), hSpace, Shm->Proc.Brand);

	printv(OutFunc, width, 2, "Vendor ID%.*s[%s]",
		width - 14 - strlen(Shm->Proc.Features.Info.VendorID), hSpace,
		Shm->Proc.Features.Info.VendorID);

	printv(OutFunc, width, 2, "Signature%.*s[%1X%1X_%1X%1X]",
		width - 19, hSpace,
		Shm->Proc.Features.Std.AX.ExtFamily,
		Shm->Proc.Features.Std.AX.Family,
		Shm->Proc.Features.Std.AX.ExtModel,
		Shm->Proc.Features.Std.AX.Model);

	printv(OutFunc, width, 2, "Stepping%.*s[%3u]",
		width - 16, hSpace, Shm->Proc.Features.Std.AX.Stepping);

	printv(OutFunc, width, 2, "Architecture%.*s[%s]",
		width - 17 - strlen(Shm->Proc.Architecture), hSpace,
		Shm->Proc.Architecture);

	printv(OutFunc, width, 2, "Online CPU%.*s[%u/%u]",
		width - 18, hSpace, Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	printv(OutFunc, width, 2, "Base Clock%.*s[%3llu]",
		width - 18, hSpace, Shm->Cpu[0].Clock.Hz / 1000000L);

	printv(OutFunc, width, 2, "Ratio Boost:");

	len = sprintf(row, "%.*sMin Max  8C  7C  6C  5C  4C  3C  2C  1C",
			22, hSpace);

	printv(OutFunc, width, 1, row);

	len = sprintf(row, "%.*s", 22, hSpace);
	for (i = 0; i < 10; i++) {
		if (Shm->Proc.Boost[i] != 0)
			len += sprintf(str, "%3d ", Shm->Proc.Boost[i]);
		else
			len += sprintf(str, "  - ");
		strcat(row, str);
	}
	printv(OutFunc, width, 1, row);
/* Section Mark */
	printv(OutFunc, width, 0, "Instruction set:");
/* Row Mark */
	len = 3;
	len += sprintf(str, "3DNow!/Ext [%c,%c]",
			Shm->Proc.Features.ExtInfo.DX._3DNow ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.DX._3DNowEx ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 11, hSpace);

	strcat(row, str);

	len += sprintf(str, "AES [%c]",
			Shm->Proc.Features.Std.CX.AES ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 6, hSpace);

	strcat(row, str);

	len += sprintf(str, "AVX/AVX2 [%c/%c]",
			Shm->Proc.Features.Std.CX.AVX ? 'Y' : 'N',
			Shm->Proc.Features.ExtFeature.BX.AVX2 ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "BMI1/BMI2 [%c/%c]",
			Shm->Proc.Features.ExtFeature.BX.BMI1 ? 'Y' : 'N',
			Shm->Proc.Features.ExtFeature.BX.BMI2 ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(OutFunc, width, 2, row);
/* Row Mark */
	len = 3;
	len += sprintf(str, "CLFSH        [%c]",
			Shm->Proc.Features.Std.DX.CLFSH ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len += sprintf(str, "CMOV [%c]",
			Shm->Proc.Features.Std.DX.CMOV ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 7, hSpace);

	strcat(row, str);

	len += sprintf(str, "CMPXCH8   [%c]",
			Shm->Proc.Features.Std.DX.CMPXCH8 ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "CMPXCH16   [%c]",
			Shm->Proc.Features.Std.CX.CMPXCH16 ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(OutFunc, width, 2, row);
/* Row Mark */
	len = 3;
	len += sprintf(str, "F16C         [%c]",
			Shm->Proc.Features.Std.CX.F16C ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 11, hSpace);

	strcat(row, str);

	len += sprintf(str, "FPU [%c]",
			Shm->Proc.Features.Std.DX.FPU ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len += sprintf(str, "FXSR   [%c]",
			Shm->Proc.Features.Std.DX.FXSR ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "LAHF/SAHF   [%c]",
			Shm->Proc.Features.ExtInfo.CX.LAHFSAHF ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(OutFunc, width, 2, row);
/* Row Mark */
	len = 3;
	len += sprintf(str, "MMX/Ext    [%c/%c]",
			Shm->Proc.Features.Std.DX.MMX ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.DX.MMX_Ext ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 7, hSpace);

	strcat(row, str);

	len += sprintf(str, "MONITOR [%c]",
			Shm->Proc.Features.Std.CX.MONITOR ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 9, hSpace);

	strcat(row, str);

	len += sprintf(str, "MOVBE   [%c]",
			Shm->Proc.Features.Std.CX.MOVBE ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "PCLMULDQ   [%c]",
			Shm->Proc.Features.Std.CX.PCLMULDQ ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(OutFunc, width, 2, row);
/* Row Mark */
	len = 3;
	len += sprintf(str, "POPCNT       [%c]",
			Shm->Proc.Features.Std.CX.POPCNT ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 8, hSpace);

	strcat(row, str);

	len += sprintf(str, "RDRAND [%c]",
			Shm->Proc.Features.Std.CX.RDRAND ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 8, hSpace);

	strcat(row, str);

	len += sprintf(str, "RDTSCP   [%c]",
			Shm->Proc.Features.ExtInfo.DX.RDTSCP ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "SEP   [%c]",
			Shm->Proc.Features.Std.DX.SEP ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(OutFunc, width, 2, row);
/* Row Mark */
	len = 3;
	len += sprintf(str, "SSE          [%c]",
			Shm->Proc.Features.Std.DX.SSE ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len += sprintf(str, "SSE2 [%c]",
			Shm->Proc.Features.Std.DX.SSE2 ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len += sprintf(str, "SSE3   [%c]",
			Shm->Proc.Features.Std.CX.SSE3 ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "SSSE3   [%c]",
			Shm->Proc.Features.Std.CX.SSSE3 ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(OutFunc, width, 2, row);
/* Row Mark */
	len = 3;
	len += sprintf(str, "SSE4.1/4A  [%c/%c]",
			Shm->Proc.Features.Std.CX.SSE41 ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.CX.SSE4A ? 'Y' : 'N');

	strcpy(row, str);

	len += sprintf(str, "%.*s", 8, hSpace);

	strcat(row, str);

	len += sprintf(str, "SSE4.2 [%c]",
			Shm->Proc.Features.Std.CX.SSE42 ? 'Y' : 'N');

	strcat(row, str);

	len += sprintf(str, "%.*s", 7, hSpace);

	strcat(row, str);

	len += sprintf(str, "SYSCALL   [%c]",
			Shm->Proc.Features.ExtInfo.DX.SYSCALL ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row, str);
	strcat(row, pad);
	printv(OutFunc, width, 2, row);

	free(row);
	free(str);
	free(pad);
}

void SysInfoFeatures(	SHM_STRUCT *Shm,
			unsigned short width,
			void(*OutFunc)(char *output) )
{
/* Section Mark */
	const char *TSC[] = {
		"Missing",
		"Variant",
		"Invariant"
	};
	const char *x2APIC[] = {
		"Missing",
	    GoK	"  xAPIC" DoK,
	    GoK	" x2APIC" DoK
	};
/* Section Mark */
	printv(OutFunc, width, 2,
		"1 GB Pages Support%.*s1GB-PAGES   [%7s]",
		width - 42, hSpace,
		powered(Shm->Proc.Features.ExtInfo.DX.PG_1GB));

	printv(OutFunc, width, 2,
		"100 MHz multiplier Control%.*s100MHzSteps   [%7s]",
		width - 52, hSpace,
		powered(Shm->Proc.Features.AdvPower.DX._100MHz));

	printv(OutFunc, width, 2,
		"Advanced Configuration & Power Interface"		\
					"%.*sACPI   [%7s]",
		width - 59, hSpace,
		powered(Shm->Proc.Features.Std.DX.ACPI			// Intel
			| Shm->Proc.Features.AdvPower.DX.HwPstate) );	// AMD

	printv(OutFunc, width, 2,
		"Advanced Programmable Interrupt Controller"		\
						"%.*sAPIC   [%7s]",
		width - 61, hSpace, powered(Shm->Proc.Features.Std.DX.APIC));

	printv(OutFunc, width, 2,
		"Core Multi-Processing%.*sCMP Legacy   [%7s]",
		width - 46, hSpace,
		powered(Shm->Proc.Features.ExtInfo.CX.MP_Mode));

	printv(OutFunc, width, 2,
		"L1 Data Cache Context ID%.*sCNXT-ID   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.CX.CNXT_ID));

	printv(OutFunc, width, 2,
		"Direct Cache Access%.*sDCA   [%7s]",
		width - 37, hSpace, powered(Shm->Proc.Features.Std.CX.DCA));

	printv(OutFunc, width, 2,
		"Debugging Extension%.*sDE   [%7s]",
		width - 36, hSpace, powered(Shm->Proc.Features.Std.DX.DE));

	printv(OutFunc, width, 2,
		"Debug Store & Precise Event Based Sampling"		\
					"%.*sDS, PEBS   [%7s]",
		width - 65, hSpace, powered(Shm->Proc.Features.Std.DX.DS_PEBS));

	printv(OutFunc, width, 2,
		"CPL Qualified Debug Store%.*sDS-CPL   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.CX.DS_CPL));

	printv(OutFunc, width, 2,
		"64-Bit Debug Store%.*sDTES64   [%7s]",
		width - 39, hSpace, powered(Shm->Proc.Features.Std.CX.DTES64));

	printv(OutFunc, width, 2,
		"Fast-String Operation%.*sFast-Strings   [%7s]",
		width - 48,hSpace,
		powered(Shm->Proc.Features.ExtFeature.BX.FastStrings));

	printv(OutFunc, width, 2,
		"Fused Multiply Add%.*sFMA|FMA4   [%7s]",
		width - 41, hSpace,
		powered(  Shm->Proc.Features.Std.CX.FMA
			| Shm->Proc.Features.ExtInfo.CX.FMA4 ));

	printv(OutFunc, width, 2,
		"Hardware Lock Elision%.*sHLE   [%7s]",
		width - 39, hSpace,
		powered(Shm->Proc.Features.ExtFeature.BX.HLE));

	printv(OutFunc, width, 2,
		"Long Mode 64 bits%.*sIA64|LM   [%7s]",
		width - 39, hSpace,
		powered(Shm->Proc.Features.ExtInfo.DX.IA64));

	printv(OutFunc, width, 2,
		"LightWeight Profiling%.*sLWP   [%7s]",
		width - 39, hSpace,
		powered(Shm->Proc.Features.ExtInfo.CX.LWP));

	printv(OutFunc, width, 2,
		"Machine-Check Architecture%.*sMCA   [%7s]",
		width - 44, hSpace,
		powered(Shm->Proc.Features.Std.DX.MCA));

	printv(OutFunc, width, 2,
		"Model Specific Registers%.*sMSR   [%7s]",
		width - 42, hSpace, powered(Shm->Proc.Features.Std.DX.MSR));

	printv(OutFunc, width, 2,
		"Memory Type Range Registers%.*sMTRR   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.DX.MTRR));

	printv(OutFunc, width, 2,
		"OS-Enabled Ext. State Management%.*sOSXSAVE   [%7s]",
		width - 54, hSpace, powered(Shm->Proc.Features.Std.CX.OSXSAVE));

	printv(OutFunc, width, 2,
		"Physical Address Extension%.*sPAE   [%7s]",
		width - 44, hSpace, powered(Shm->Proc.Features.Std.DX.PAE));

	printv(OutFunc, width, 2,
		"Page Attribute Table%.*sPAT   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.DX.PAT));

	printv(OutFunc, width, 2,
		"Pending Break Enable%.*sPBE   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.DX.PBE));

	printv(OutFunc, width, 2,
		"Process Context Identifiers%.*sPCID   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.CX.PCID));

	printv(OutFunc, width, 2,
		"Perfmon and Debug Capability%.*sPDCM   [%7s]",
		width - 47, hSpace, powered(Shm->Proc.Features.Std.CX.PDCM));

	printv(OutFunc, width, 2,
		"Page Global Enable%.*sPGE   [%7s]",
		width - 36, hSpace, powered(Shm->Proc.Features.Std.DX.PGE));

	printv(OutFunc, width, 2,
		"Page Size Extension%.*sPSE   [%7s]",
		width - 37, hSpace, powered(Shm->Proc.Features.Std.DX.PSE));

	printv(OutFunc, width, 2,
		"36-bit Page Size Extension%.*sPSE36   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.DX.PSE36));

	printv(OutFunc, width, 2,
		"Processor Serial Number%.*sPSN   [%7s]",
		width - 41, hSpace, powered(Shm->Proc.Features.Std.DX.PSN));

	printv(OutFunc, width, 2,
		"Restricted Transactional Memory%.*sRTM   [%7s]",
		width - 49, hSpace,
		powered(Shm->Proc.Features.ExtFeature.BX.RTM));

	printv(OutFunc, width, 2,
		"Safer Mode Extensions%.*sSMX   [%7s]",
		width - 39, hSpace, powered(Shm->Proc.Features.Std.CX.SMX));

	printv(OutFunc, width, 2,
		"Self-Snoop%.*sSS   [%7s]",
		width - 27, hSpace, powered(Shm->Proc.Features.Std.DX.SS));

	printv(OutFunc, width, 2,
		"Time Stamp Counter%.*sTSC [%9s]",
		width - 36, hSpace, TSC[Shm->Proc.InvariantTSC]);

	printv(OutFunc, width, 2,
		"Time Stamp Counter Deadline%.*sTSC-DEADLINE   [%7s]",
		width - 54, hSpace, powered(Shm->Proc.Features.Std.CX.TSCDEAD));

	printv(OutFunc, width, 2,
		"Virtual Mode Extension%.*sVME   [%7s]",
		width - 40, hSpace, powered(Shm->Proc.Features.Std.DX.VME));

	printv(OutFunc, width, 2,
		"Virtual Machine Extensions%.*sVMX   [%7s]",
		width - 44, hSpace, powered(Shm->Proc.Features.Std.CX.VMX));

	printv(OutFunc, width, 2,
		"Extended xAPIC Support%.*sx2APIC   [%7s]",
		width - 43, hSpace, x2APIC[Shm->Cpu[0].Topology.MP.x2APIC]);

	printv(OutFunc, width, 2,
		"Execution Disable Bit Support%.*sXD-Bit   [%7s]",
		width - 50, hSpace,
		powered(Shm->Proc.Features.ExtInfo.DX.XD_Bit));

	printv(OutFunc, width, 2,
		"XSAVE/XSTOR States%.*sXSAVE   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.CX.XSAVE));

	printv(OutFunc, width, 2,
		"xTPR Update Control%.*sxTPR   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.CX.xTPR));
}

void SysInfoTech(SHM_STRUCT *Shm,
		unsigned short width,
		void(*OutFunc)(char *output) )
{
	const unsigned int
	    isTurboBoost = (Shm->Proc.TurboBoost==Shm->Proc.TurboBoost_Mask),
	    isSpeedStep  = (Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask);

/* Section Mark */
	printv(OutFunc, width, 2,
		"Hyper-Threading%.*sHTT       [%3s]",
		width - 33, hSpace, enabled(Shm->Proc.HyperThreading));

	printv(OutFunc, width, 2,
		"SpeedStep%.*sEIST       [%3s]",
		width - 28, hSpace, enabled(isSpeedStep));

	printv(OutFunc, width, 2,
		"PowerNow!%.*sPowerNow       [%3s]",
		width - 32, hSpace,
		enabled(Shm->Proc.PowerNow == 0b11));	// VID + FID

	printv(OutFunc, width, 2,
		"Dynamic Acceleration%.*sIDA       [%3s]",
		width - 38, hSpace,
		enabled(Shm->Proc.Features.Power.AX.TurboIDA));

	printv(OutFunc, width, 2,
		"Turbo Boost%.*sTURBO|CPB       [%3s]",
		width - 35, hSpace,
		enabled(isTurboBoost|Shm->Proc.Features.AdvPower.DX.CPB ));
}

void SysInfoPerfMon(	SHM_STRUCT *Shm,
			unsigned short width,
			void(*OutFunc)(char *output) )
{
	const unsigned int
		isEnhancedHaltState = (Shm->Proc.C1E == Shm->Proc.C1E_Mask),
		isC3autoDemotion = (Shm->Proc.C3A == Shm->Proc.C3A_Mask),
		isC1autoDemotion = (Shm->Proc.C1A == Shm->Proc.C1A_Mask),
		isC3undemotion = (Shm->Proc.C3U == Shm->Proc.C3U_Mask),
		isC1undemotion = (Shm->Proc.C1U == Shm->Proc.C1U_Mask);

/* Section Mark */
	printv(OutFunc, width, 2,
		"Version%.*sPM       [%3d]",
		width - 24, hSpace, Shm->Proc.PM_version);

	printv(OutFunc, width, 2,
		"Counters:%.*sGeneral%.*sFixed",
		10, hSpace, width - 61, hSpace);

    if (OutFunc == NULL) {
	printv(OutFunc, width, 1,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.AX.MonCtrs,
				Shm->Proc.Features.PerfMon.AX.MonWidth,
		11, hSpace,	Shm->Proc.Features.PerfMon.DX.FixCtrs,
				Shm->Proc.Features.PerfMon.DX.FixWidth);
    } else {
	printv(OutFunc, width, 0,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.AX.MonCtrs,
				Shm->Proc.Features.PerfMon.AX.MonWidth,
		5, hSpace,	Shm->Proc.Features.PerfMon.DX.FixCtrs,
				Shm->Proc.Features.PerfMon.DX.FixWidth);
    }
	printv(OutFunc, width, 2,
		"Enhanced Halt State%.*sC1E       [%3s]",
		width - 37, hSpace, enabled(isEnhancedHaltState));

	printv(OutFunc, width, 2,
		"C1 Auto Demotion%.*sC1A       [%3s]",
		width - 34, hSpace, enabled(isC1autoDemotion));

	printv(OutFunc, width, 2,
		"C3 Auto Demotion%.*sC3A       [%3s]",
		width - 34, hSpace, enabled(isC3autoDemotion));

	printv(OutFunc, width, 2,
		"C1 UnDemotion%.*sC1U       [%3s]",
		width - 31, hSpace, enabled(isC1undemotion));

	printv(OutFunc, width, 2,
		"C3 UnDemotion%.*sC3U       [%3s]",
		width - 31, hSpace, enabled(isC3undemotion));

	printv(OutFunc, width, 2,
		"Frequency ID control%.*sFID       [%3s]",
		width - 38, hSpace,
		enabled(Shm->Proc.Features.AdvPower.DX.FID));

	printv(OutFunc, width, 2,
		"Voltage ID control%.*sVID       [%3s]",
		width - 36, hSpace,
		enabled(Shm->Proc.Features.AdvPower.DX.VID));

	printv(OutFunc, width, 2,
		"P-State Hardware Coordination Feedback"		\
			"%.*sMPERF/APERF       [%3s]",
		width - 64, hSpace,
		enabled(Shm->Proc.Features.Power.CX.HCF_Cap));

	printv(OutFunc, width, 2,
		"Hardware-Controlled Performance States%.*sHWP       [%3s]",
		width - 56, hSpace,
		enabled(  Shm->Proc.Features.Power.AX.HWP_Reg
			| Shm->Proc.Features.AdvPower.DX.HwPstate));

	printv(OutFunc, width, 2,
		"Hardware Duty Cycling%.*sHDC       [%3s]",
		width - 39, hSpace,
		enabled(Shm->Proc.Features.Power.AX.HDC_Reg));

	printv(OutFunc, width, 2,
		"MWAIT States:%.*sC0      C1      C2      C3      C4",
		06, hSpace);

	printv(OutFunc, width, (OutFunc == NULL) ? 1 : 0,
		"%.*s%2d      %2d      %2d      %2d      %2d",
		21, hSpace,
		Shm->Proc.Features.MWait.DX.Num_C0_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C1_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C2_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C3_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C4_MWAIT);

	printv(OutFunc, width, 2,
		"Core Cycles%.*s[%7s]",
		width - 23, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.CoreCycles));

	printv(OutFunc, width, 2,
		"Instructions Retired%.*s[%7s]",
		width - 32, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.InstrRetired));

	printv(OutFunc, width, 2,
		"Reference Cycles%.*s[%7s]",
		width - 28, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.RefCycles));

	printv(OutFunc, width, 2,
		"Last Level Cache References%.*s[%7s]",
		width - 39, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.LLC_Ref));

	printv(OutFunc, width, 2,
		"Last Level Cache Misses%.*s[%7s]",
		width - 35, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.LLC_Misses));

	printv(OutFunc, width, 2,
		"Branch Instructions Retired%.*s[%7s]",
		width - 39, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.BranchRetired));

	printv(OutFunc, width, 2,
		"Branch Mispredicts Retired%.*s[%7s]",
		width - 38, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.BranchMispred));
}

void SysInfoPwrThermal( SHM_STRUCT *Shm,
			unsigned short width,
			void(*OutFunc)(char *output) )
{
	const char *TM[] = {
		"Missing",
		"Present",
		"Disable",
		" Enable",
	};
/* Section Mark */
	printv(OutFunc, width, 2,
		"Digital Thermal Sensor%.*sDTS   [%7s]",
		width - 40, hSpace,
		powered( Shm->Proc.Features.Power.AX.DTS
			|Shm->Proc.Features.AdvPower.DX.TS));

	printv(OutFunc, width, 2,
		"Power Limit Notification%.*sPLN   [%7s]",
		width - 42, hSpace, powered(Shm->Proc.Features.Power.AX.PLN));

	printv(OutFunc, width, 2,
		"Package Thermal Management%.*sPTM   [%7s]",
		width - 44, hSpace, powered(Shm->Proc.Features.Power.AX.PTM));

	printv(OutFunc, width, 2,
		"Thermal Monitor 1%.*sTM1|TTP   [%7s]",
		width - 39, hSpace, TM[  Shm->Cpu[0].PowerThermal.TM1
					|Shm->Proc.Features.AdvPower.DX.TTP ]);
	printv(OutFunc, width, 2,
		"Thermal Monitor 2%.*sTM2|HTC   [%7s]",
		width - 39, hSpace, TM[  Shm->Cpu[0].PowerThermal.TM2
					|Shm->Proc.Features.AdvPower.DX.TM ]);

	printv(OutFunc, width, 2,
		"Clock Modulation%.*sODCM   [%6.2f%%]",
		width - 35, hSpace, Shm->Cpu[0].PowerThermal.ODCM);

	printv(OutFunc, width, 2,
		"Energy Policy%.*sBias Hint   [%7u]",
		width - 37, hSpace, Shm->Cpu[0].PowerThermal.PowerPolicy);
}

void SysInfoKernel(	SHM_STRUCT *Shm,
			unsigned short width,
			void(*OutFunc)(char *output) )
{
	size_t	len = 0;
	char	*row = malloc(width + 1),
		*str = malloc(width + 1);
	int	i = 0;

/* Section Mark */
	printv(OutFunc, width, 0, "%s:", Shm->SysGate.sysname);

	printv(OutFunc, width, 2, "Release%.*s[%s]",
		width - 12 - strlen(Shm->SysGate.release), hSpace,
		Shm->SysGate.release);

    if ((len = strlen(Shm->SysGate.IdleDriver.Name)) > 0) {
	printv(OutFunc, width, 2, "Idle driver%.*s[%s]",
		width - 16 - len, hSpace, Shm->SysGate.IdleDriver.Name);
/* Row Mark */
	len = sprintf(row, "States:%.*s", 9, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8s",
			Shm->SysGate.IdleDriver.State[i].Name);
		strcat(row, str);
	}
	printv(OutFunc, width, 3, row);
/* Row Mark */
	len = sprintf(row, "Power:%.*s", 10, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8d",
			Shm->SysGate.IdleDriver.State[i].powerUsage);
		strcat(row, str);
	}
	printv(OutFunc, width, 3, row);
/* Row Mark */
	len = sprintf(row, "Latency:%.*s", 8, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8u",
			Shm->SysGate.IdleDriver.State[i].exitLatency);
		strcat(row, str);
	}
	printv(OutFunc, width, 3, row);
/* Row Mark */
	len = sprintf(row, "Residency:%.*s", 6, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8u",
			Shm->SysGate.IdleDriver.State[i].targetResidency);
		strcat(row, str);
	}
	printv(OutFunc, width, 3, row);
    }
	free(row);
	free(str);
}

#define LEADING_LEFT	2
#define LEADING_TOP	1
#define MARGIN_WIDTH	2
#define MARGIN_HEIGHT	1

void LCD_Draw(	unsigned short col,
		unsigned short row,
		char *thisView,
		unsigned int thisNumber,
		unsigned int thisDigit[] )
{
    char lcdBuf[32];
    unsigned short j = (unsigned short) Dec2Digit(thisNumber, thisDigit);

    thisView[0] = '\0';
    j = 4;
    do {
	short offset = col + (4 - j) * 3;

	sprintf(lcdBuf,
		"\033[%hu;%hdH" "%.*s"					\
		"\033[%hu;%hdH" "%.*s"					\
		"\033[%hu;%hdH" "%.*s",
		row	, offset, 3, lcd[thisDigit[9 - j]][0],
		row + 1	, offset, 3, lcd[thisDigit[9 - j]][1],
		row + 2	, offset, 3, lcd[thisDigit[9 - j]][2]);

	strcat(thisView, lcdBuf);
	j--;
    } while (j > 0) ;
}

void Dashboard( SHM_STRUCT *Shm,
		unsigned short leadingLeft,
		unsigned short leadingTop,
		unsigned short marginWidth,
		unsigned short marginHeight)
{
    char *boardView = NULL,
	 *lcdView = NULL,
	 *cpuView = NULL,
	 *absMove = CUH CLS;
    double minRatio = Shm->Proc.Boost[0], maxRatio = Shm->Proc.Boost[9];
    double medianRatio =(minRatio + maxRatio) / 2;

    void FreeAll(void)
    {
	free(cpuView);
	free(lcdView);
	free(boardView);
    }

    void AllocAll()
    {
	// Up to 9 digits x 3 cols x 3 lines per digit
	const size_t lcdSize	= (9 * 3 * 3)
				+ (9 * 3 * sizeof("\033[000;000H")) + 1;
	lcdView = malloc(lcdSize);
	const size_t cpuSize	= (9 * 3) + sizeof("\033[000;000H")
				+ (3 * sizeof(DoK)) + 1;
	cpuView = malloc(cpuSize);
	// All LCD views x total number of CPU
	boardView = malloc(Shm->Proc.CPU.Count * (lcdSize + cpuSize));
    }

    AllocAll();

    marginWidth  += 12;		// shifted by lcd width
    marginHeight += 3 + 1;	// shifted by lcd height + cpu frame
    unsigned int cpu = 0;

    while (!Shutdown) {
    	unsigned int digit[9];
	unsigned short X, Y;

	while (!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * BASE_SLEEP);

	BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);

	X = leadingLeft;
	Y = leadingTop;
	boardView[0] = '\0';

	for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if (!Shm->Cpu[cpu].OffLine.HW) {
		struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!Shm->Cpu[cpu].OffLine.OS) {
			LCD_Draw(X, Y, lcdView,
				(unsigned int) Flop->Relative.Freq, digit);
			sprintf(cpuView,
				"\033[%hu;%huH"DoK"[ µ%-2u"WoK"%4u"DoK"C ]",
					Y + 3, X, cpu, Flop->Thermal.Temp);

			if (Flop->Relative.Ratio > medianRatio)
				strcat(boardView, RoK);
			else if (Flop->Relative.Ratio > minRatio)
				strcat(boardView, YoK);
			else
				strcat(boardView, GoK);
		} else {
			sprintf(lcdView, "\033[%hu;%huH" "_  _  _  _",
				Y + 1, X + 1);
			sprintf(cpuView, "\033[%hu;%huH" "[ µ%-2u""  OFF ]",
				Y + 3, X, cpu);
		}
		X += marginWidth;

		if ((X - 3) >= (GetScreenSize().width - marginWidth)) {
			X = leadingLeft;
			Y += marginHeight;
			absMove = CUH;
		}
		else
			absMove = CUH CLS;

		strcat(boardView, lcdView);
		strcat(boardView, cpuView);
	    }
	if (printf("%s" "%s", absMove, boardView) > 0)
		fflush(stdout);
    }
    FreeAll();
}


void Counters(SHM_STRUCT *Shm)
{
    unsigned int cpu = 0;
    while (!Shutdown) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * BASE_SLEEP);

	BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);

		printf("CPU Freq(MHz) Ratio  Turbo"			\
			"  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
			"  Min TMP:TS  Max\n");
	for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	  if (!Shm->Cpu[cpu].OffLine.HW) {
	    struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!Shm->Cpu[cpu].OffLine.OS)
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
			Shm->Cpu[0].PowerThermal.Target);
    }
}


void Instructions(SHM_STRUCT *Shm)
{
	unsigned int cpu = 0;
	while (!Shutdown) {
	  while (!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
			usleep(Shm->Proc.SleepInterval * BASE_SLEEP);

	  BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);

	  if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);

		    printf("CPU     IPS            IPC            CPI\n");

	  for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if (!Shm->Cpu[cpu].OffLine.HW) {
		struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!Shm->Cpu[cpu].OffLine.OS)
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


void Topology(SHM_STRUCT *Shm, void(*OutFunc)(char *output))
{
	unsigned int cpu = 0, level = 0, nl = 6;
	char *line = malloc(13 + 1);

	void printv(char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf(line, fmt, ap);
		if (OutFunc == NULL)
			if (!--nl) {
				nl = 6;
				printf("%s\n", line);
			}
			else
				printf("%s", line);
		else
			OutFunc(line);
		va_end(ap);
	}

	printv("CPU    Apic  ");printv(" Core  Thread");printv("  Caches     ");
	printv(" (w)rite-Back");printv(" (i)nclusive ");printv("             ");
	printv(" #      ID   ");printv("  ID     ID  ");printv(" L1-Inst Way ");
	printv(" L1-Data Way ");printv("     L2  Way ");printv("     L3  Way ");

	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
		printv("%02u%-4s%6d ",
			cpu,
			(Shm->Cpu[cpu].Topology.MP.BSP) ? ":BSP" : ":AP",
			Shm->Cpu[cpu].Topology.ApicID);

		printv("%6d %6d",
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);

	    for (level = 0; level < CACHE_MAX_LEVEL; level++) {
		printv("%8u%3u%c%c",
			Shm->Cpu[cpu].Topology.Cache[level].Size,
			Shm->Cpu[cpu].Topology.Cache[level].Way,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack?
				'w' : 0x20,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive?
				'i' : 0x20);
	    }
	}
	free(line);
}

typedef union {
	unsigned long long key;
	unsigned char code[8];
} SCANKEY;

#define SCANKEY_VOID		0xffffffffffffffff
#define SCANKEY_NULL		0x0
#define SCANKEY_TAB		0x9
#define SCANKEY_ENTER		0xa
#define SCANKEY_ESC		0x1b
#define SCANKEY_UP		0x415b1b
#define SCANKEY_DOWN		0x425b1b
#define SCANKEY_RIGHT		0x435b1b
#define SCANKEY_LEFT		0x445b1b
#define SCANKEY_HOME		0x485b1b
#define SCANKEY_END		0x465b1b
#define SCANKEY_F1		0x504f1b
#define SCANKEY_F2		0x514f1b
#define SCANKEY_F3		0x524f1b
#define SCANKEY_F4		0x534f1b
#define SCANKEY_F10		0x31325b1b
#define SCANKEY_SHIFT_TAB	0x5a5b1b
#define SCANKEY_PGUP		0x7e355b1b
#define SCANKEY_PGDW		0x7e365b1b
#define SCANKEY_SHIFT_a		0x41
#define SCANKEY_SHIFT_d		0x44
#define SCANKEY_SHIFT_q		0x51
#define SCANKEY_SHIFT_s		0x53
#define SCANKEY_SHIFT_w		0x57
#define SCANKEY_SHIFT_z		0x5a
#define SCANKEY_a		0x61
#define SCANKEY_c		0x63
#define SCANKEY_d		0x64
#define SCANKEY_e		0x65
#define SCANKEY_f		0x66
#define SCANKEY_h		0x68
#define SCANKEY_i		0x69
#define SCANKEY_k		0x6b
#define SCANKEY_l		0x6c
#define SCANKEY_m		0x6d
#define SCANKEY_o		0x6f
#define SCANKEY_p		0x70
#define SCANKEY_q		0x71
#define SCANKEY_r		0x72
#define SCANKEY_s		0x73
#define SCANKEY_t		0x74
#define SCANKEY_u		0x75
#define SCANKEY_v		0x76
#define SCANKEY_w		0x77
#define SCANKEY_x		0x78
#define SCANKEY_z		0x7a

#define SCANCON_HOME		0x7e315b1b
#define SCANCON_END		0x7e345b1b
#define SCANCON_F1		0x415b5b1b
#define SCANCON_F2		0x425b5b1b
#define SCANCON_F3		0x435b5b1b
#define SCANCON_F4		0x445b5b1b
#define SCANCON_SHIFT_TAB	0x91b

#define SORTBY_TIME		0xfffffffffffffffe
#define SORTBY_PID		0xfffffffffffffffd
#define SORTBY_COMM		0xfffffffffffffffc

int GetKey(SCANKEY *scan, struct timespec *tsec)
{
	struct pollfd fds = {.fd = STDIN_FILENO, .events = POLLIN};
	int rp = 0, rz = 0;

	if ((rp=ppoll(&fds, 1, tsec, NULL)) > 0)
		if (fds.revents == POLLIN) {
			size_t lc = fread(&scan->key, 1, 8, stdin);
			for (rz = lc; rz < 8; rz++)
				scan->code[rz] = 0;
		}
	return(rp);
}

typedef struct {
	unsigned short	col,
			row;
} Coordinate;

typedef struct {
	signed short	horz,
			vert;
} CoordShift;

typedef struct {
	unsigned short	wth,
			hth;
} CoordSize;

typedef union {
	unsigned char	value;
	struct {
	unsigned char	fg:  3-0,
			un:  4-3,
			bg:  7-4,
			bf:  8-7;
	};
} Attribute;

#define MakeAttr(_fg, _un, _bg, _bf)					\
	({Attribute _attr={.fg = _fg,.un = _un,.bg = _bg,.bf = _bf}; _attr;})

#define HDK	{.fg = BLACK,	.bg = BLACK,	.bf = 1}
#define HBK	{.fg = BLUE,	.bg = BLACK,	.bf = 1}
#define HRK	{.fg = RED,	.bg = BLACK,	.bf = 1}
#define HGK	{.fg = GREEN,	.bg = BLACK,	.bf = 1}
#define HYK	{.fg = YELLOW,	.bg = BLACK,	.bf = 1}
#define HWK	{.fg = WHITE,	.bg = BLACK,	.bf = 1}
#define HKB	{.fg = BLACK,	.bg = BLUE,	.bf = 1}
#define HWB	{.fg = WHITE,	.bg = BLUE,	.bf = 1}
#define HKW	{.fg = BLACK,	.bg = WHITE,	.bf = 1}
#define _HWK	{.fg = WHITE,	.bg = BLACK,	.un = 1,	.bf = 1}
#define _HWB	{.fg = WHITE,	.bg = BLUE,	.un = 1,	.bf = 1}
#define LDK	{.fg = BLACK,	.bg = BLACK}
#define LKW	{.fg = BLACK,	.bg = WHITE}
#define LRK	{.fg = RED,	.bg = BLACK}
#define LYK	{.fg = YELLOW,	.bg = BLACK}
#define LBK	{.fg = BLUE,	.bg = BLACK}
#define LBW	{.fg = BLUE,	.bg = WHITE}
#define LCK	{.fg = CYAN,	.bg = BLACK}
#define LWK	{.fg = WHITE,	.bg = BLACK}
#define LWB	{.fg = WHITE,	.bg = BLUE}
#define _LKW	{.fg = BLACK,	.bg = WHITE,	.un = 1}
#define _LBW	{.fg = BLUE,	.bg = WHITE,	.un = 1}
#define _LWK	{.fg = WHITE,	.bg = BLACK,	.un = 1}

#define MAKE_TITLE_UNFOCUS	MakeAttr(BLACK, 0, BLUE, 1)
#define MAKE_TITLE_FOCUS	MakeAttr(WHITE, 0, CYAN, 1)
#define MAKE_BORDER_UNFOCUS	MakeAttr(BLACK, 0, BLUE, 1)
#define MAKE_BORDER_FOCUS	MakeAttr(WHITE, 0, BLUE, 1)
#define MAKE_SELECT_UNFOCUS	MakeAttr(WHITE, 0, BLACK, 0)
#define MAKE_SELECT_FOCUS	MakeAttr(BLACK, 0, CYAN, 0)
#define MAKE_PRINT_UNFOCUS	MakeAttr(WHITE, 0, BLACK, 0)
#define MAKE_PRINT_FOCUS	MakeAttr(WHITE, 0, BLACK, 1)

typedef unsigned char	ASCII;

#define LayerDeclare(_len)						\
	struct {							\
		Coordinate	origin;					\
		size_t		length;					\
		Attribute	attr[_len];				\
		ASCII		code[_len];				\
	}

typedef struct {
	ASCII		*code;
	Attribute	*attr;
	CoordSize	size;
} Layer;

typedef struct {
	CoordSize	size;
	Coordinate	origin,
			select;
	CoordShift	scroll;
} Matrix;

typedef struct {
	SCANKEY 	quick;
	Attribute	*attr;
	ASCII		*item;
	size_t		length;
} TCell;

typedef struct _Win {
	Layer		*layer;

	unsigned long long id;

	struct _Win	*prev,
			*next;
	struct {
		void	(*Print)(struct _Win *win, void *list);
	    struct {
		int	(*Enter)(SCANKEY *scan, struct _Win *win);
		void	(*Escape)(struct _Win *win);
		void	(*Left)(struct _Win *win);
		void	(*Right)(struct _Win *win);
		void	(*Down)(struct _Win *win);
		void	(*Up)(struct _Win *win);
		void	(*Home)(struct _Win *win);
		void	(*End)(struct _Win *win);
		void	(*PgUp)(struct _Win *win);
		void	(*PgDw)(struct _Win *win);
		void	(*WinLeft)(struct _Win *win);
		void	(*WinRight)(struct _Win *win);
		void	(*WinDown)(struct _Win *win);
		void	(*WinUp)(struct _Win *win);
	    } key;

	    struct {
	      Attribute	select,
			border,
			title;
	    } color[2];

		char	*title;
	} hook;

	Matrix		matrix;
	TCell		*cell;
	size_t		dim;

	struct {
		size_t	rowLen,
			titleLen;
	unsigned short	bottomRow;
	} lazyComp;
} Window;

typedef struct {
	Window	*head;
} WinList;

typedef void (*TCELLFUNC)(Window*, void*);
typedef int  (*KEYFUNC)(SCANKEY*, Window*);
typedef void (*WINFUNC)(Window*);
typedef char REGSTR[];
typedef char *REGPTR;

void HookCellFunc(TCELLFUNC *with, TCELLFUNC what) { *with=what; }

void HookKeyFunc(KEYFUNC *with, KEYFUNC what) { *with=what; }

void HookWinFunc(WINFUNC *with, WINFUNC what) { *with=what; }

void HookAttrib(Attribute *with, Attribute what) { with->value=what.value; }

void HookString(REGSTR *with, REGSTR what) { strcpy(*with, what); }

void HookPointer(REGPTR *with, REGPTR what)
{
	if ((*with = realloc(*with, 1 + strlen(what))) != NULL)
		strcpy(*with, what);
}

#define StoreWindow(win, with, what)					\
(									\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(TCELLFUNC)), HookCellFunc,	\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(KEYFUNC)), HookKeyFunc,		\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(WINFUNC)), HookWinFunc,		\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(Attribute)), HookAttrib,		\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(REGSTR)), HookString,		\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(REGPTR)), HookPointer,		\
    (void)0))))))							\
	(&(win->hook with), what)					\
)

#define LayerAt(layer, plane, col, row)					\
	layer->plane[col + (row * layer->size.wth)]

#define LayerFillAt(layer, col, row, len, source, attrib)		\
({									\
	memset(&LayerAt(layer, attr, col, row), attrib.value, len);	\
	memcpy(&LayerAt(layer, code, col, row), source, len);		\
})

#define LayerCopyAt(layer, col, row, len, attrib, source)		\
({									\
	memcpy(&LayerAt(layer, attr, col, row), attrib, len);		\
	memcpy(&LayerAt(layer, code, col, row), source, len);		\
})

#define TCellAt(win, col, row)						\
	win->cell[col + (row * win->matrix.size.wth)]

#define GetHead(list)		(list)->head
#define SetHead(list, win)	GetHead(list) = win
#define SetDead(list)		SetHead(list, NULL)
#define IsHead(list, win)	(GetHead(list) == win)
#define IsDead(list)		(GetHead(list) == NULL)
#define IsCycling(win)		((win->next == win) && (win->prev == win))
#define GetFocus(list)		GetHead(list)

void DestroyLayer(Layer *layer)
{
	if (layer != NULL) {
		if (layer->attr != NULL) {
			free(layer->attr);
			layer->attr = NULL;
		}
		if (layer->code != NULL) {
			free(layer->code);
			layer->code = NULL;
		}
	}
}

void CreateLayer(Layer *layer, CoordSize size)
{
    if (layer != NULL) {
	layer->size.wth = size.wth;
	layer->size.hth = size.hth;
	size_t len = layer->size.wth * layer->size.hth;

	layer->attr = calloc(len, sizeof(Attribute));
	layer->code = calloc(len, sizeof(ASCII));
    }
}

#define ResetLayer(layer)						\
	memset(layer->attr, 0, layer->size.wth * layer->size.hth);	\
	memset(layer->code, 0, layer->size.wth * layer->size.hth);

void FreeAllTCells(Window *win)
{
	if (win->cell != NULL) {
		unsigned short i;
		for (i = 0; i < win->dim; i++)
		{
			free(win->cell[i].attr);
			free(win->cell[i].item);
		}
		free(win->cell);
		win->cell = NULL;
	}
}

void AllocCopyAttr(TCell *cell, Attribute attrib[])
{
	if ((attrib != NULL) && (cell->attr = malloc(cell->length)) != NULL)
		memcpy(&cell->attr->value, &attrib->value, cell->length);
}

void AllocFillAttr(TCell *cell, Attribute attrib)
{
	if ((cell->attr = malloc(cell->length)) != NULL)
		memset(&cell->attr->value, attrib.value, cell->length);
}

void AllocCopyItem(TCell *cell, ASCII *item)
{
	if ((cell->item = malloc(cell->length)) != NULL)
		strncpy((char *)cell->item, (char *)item, cell->length);
}

#define StoreTCell(win, shortkey, item, attrib)				\
({									\
    if (item != NULL) {							\
	win->dim++;							\
	win->lazyComp.bottomRow = (win->dim / win->matrix.size.wth)	\
				- win->matrix.size.hth;			\
									\
      if ((win->cell = realloc(win->cell,sizeof(TCell) * win->dim)) != NULL) {\
	win->cell[win->dim - 1].quick.key = shortkey;			\
	win->cell[win->dim - 1].length = strlen((char *)item);		\
									\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		typeof(attrib), typeof(Attribute[])), AllocCopyAttr,	\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		typeof(attrib), typeof(Attribute)), AllocFillAttr,	\
	(void)0))							\
		(&(win->cell[win->dim - 1]), attrib);			\
									\
	AllocCopyItem(&win->cell[win->dim - 1], (ASCII *)item);		\
      }									\
    }									\
})

void DestroyWindow(Window *win)
{
	if (win != NULL) {
		if (win->hook.title != NULL) {
			free(win->hook.title);
			win->hook.title = NULL;
			win->lazyComp.titleLen = 0;
		}
		FreeAllTCells(win);
		free(win);
		win = NULL;
	}
}

Window *CreateWindow(	Layer *layer,
			unsigned long long id,
			unsigned short width,
			unsigned short height,
			unsigned short oCol,
			unsigned short oRow)
{
	Window *win = calloc(1, sizeof(Window));
	if (win != NULL) {
		win->layer = layer;
		win->id = id;
		win->matrix.size.wth = width;
		win->matrix.size.hth = height;
		win->matrix.origin.col = oCol;
		win->matrix.origin.row = oRow;

	    Attribute	select[2] = {
				MAKE_SELECT_UNFOCUS,
				MAKE_SELECT_FOCUS
			},
			border[2] = {
				MAKE_BORDER_UNFOCUS,
				MAKE_BORDER_FOCUS
			},
			title[2] = {
				MAKE_TITLE_UNFOCUS,
				MAKE_TITLE_FOCUS
			};
	    int i;
	    for (i = 0; i < 2; i++) {
		win->hook.color[i].select = select[i];
		win->hook.color[i].border = border[i];
		win->hook.color[i].title  = title[i];
	    }
	}
	return(win);
}

#define RemoveWinList(win, list)					\
({									\
	win->prev->next = win->next;					\
	win->next->prev = win->prev;					\
})

#define AppendWinList(win, list)					\
({									\
	win->prev = GetHead(list);					\
	win->next = GetHead(list)->next;				\
	GetHead(list)->next->prev = win;				\
	GetHead(list)->next = win;					\
})

void RemoveWindow(Window *win, WinList *list)
{
	RemoveWinList(win, list);

	if (IsCycling(GetHead(list)))
		SetDead(list);
	else if (IsHead(list, win))
	/*Auto shift*/	SetHead(list, win->next);

	DestroyWindow(win);
}

void AppendWindow(Window *win, WinList *list)
{
	if (!IsDead(list))
		AppendWinList(win, list);
	else {
		// Dead head, now cycling
		win->prev = win;
		win->next = win;
	}
	SetHead(list, win);
}

void DestroyAllWindows(WinList *list)
{
	while (!IsDead(list))
		RemoveWindow(GetHead(list), list);
}

void AnimateWindow(int rotate, WinList *list)
{
    if (!IsDead(list))
	SetHead(list, rotate == 1 ? GetHead(list)->next : GetHead(list)->prev);
}

Window *SearchWinListById(unsigned long long id, WinList *list)
{
	Window *win = NULL;
	if (!IsDead(list)) {
		Window *walker = GetHead(list);
		do {
			if (walker->id == id)
				win = walker;

			walker = walker->prev;
		} while (!IsHead(list, walker) && (win == NULL));
	}
	return(win);
}

void PrintContent(Window*win,WinList*list,unsigned short col,unsigned short row)
{
    if ((win->matrix.select.col == col)
     && (win->matrix.select.row == row))
	LayerFillAt(win->layer,
		(win->matrix.origin.col
		+ (col * TCellAt(win,
				(win->matrix.scroll.horz + col),
				(win->matrix.scroll.vert + row)).length)),
		(win->matrix.origin.row + row),
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).length,
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).item,
		win->hook.color[(GetFocus(list) == win)].select);
    else if (GetFocus(list) == win)
	LayerCopyAt(win->layer,
		(win->matrix.origin.col
		+ (col * TCellAt(win,
				(win->matrix.scroll.horz + col),
				(win->matrix.scroll.vert + row)).length)),
		(win->matrix.origin.row + row),
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).length,
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).attr,
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).item);
    else {
	LayerFillAt(win->layer,
		(win->matrix.origin.col
		+ (col * TCellAt(win,
				(win->matrix.scroll.horz + col),
				(win->matrix.scroll.vert + row)).length)),
		(win->matrix.origin.row + row),
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).length,
		TCellAt(win,
			(win->matrix.scroll.horz + col),
			(win->matrix.scroll.vert + row)).item,
		win->hook.color[0].select);
    }
}

void ForEachCellPrint(Window *win, WinList *list)
{
	unsigned short col, row;
	Attribute border = win->hook.color[(GetFocus(list) == win)].border;

	if (win->lazyComp.rowLen == 0)
	  for (col=0, win->lazyComp.rowLen=2; col < win->matrix.size.wth; col++)
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;
	// Top, Left Border Corner
	LayerAt(win->layer, attr,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row - 1)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row - 1)) = 0x20;
	// Top Border Line
	if (win->hook.title == NULL)
	    LayerFillAt(win->layer,
			win->matrix.origin.col,
			(win->matrix.origin.row - 1),
			(win->lazyComp.rowLen - 2), hLine, border);
	else {
	    if (win->lazyComp.titleLen == 0)
			win->lazyComp.titleLen=strlen(win->hook.title);

	    size_t halfLeft=(win->lazyComp.rowLen - win->lazyComp.titleLen) / 2;
	    size_t halfRight = halfLeft
			+ (win->lazyComp.rowLen - win->lazyComp.titleLen) % 2;
	    // Top, Half-Left Border Line
	    LayerFillAt(win->layer,
			win->matrix.origin.col,
			(win->matrix.origin.row - 1),
			halfLeft, hLine, border);
	    // Top, Centered Border Title
	    LayerFillAt(win->layer,
			(halfLeft + (win->matrix.origin.col - 1)),
			(win->matrix.origin.row - 1),
			win->lazyComp.titleLen, win->hook.title,
			((GetFocus(list) == win) ?
				win->hook.color[1].title
			:	win->hook.color[0].title));
	    // Top, Half-Right Border Line
	    LayerFillAt(win->layer,
			(halfLeft + win->lazyComp.titleLen
			+ (win->matrix.origin.col - 1)),
			(win->matrix.origin.row - 1),
			(halfRight - 1), hLine, border);
	}
	// Top, Right Border Corner
	LayerAt(win->layer, attr,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row - 1)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row - 1)) = 0x20;

	for (row = 0; row < win->matrix.size.hth; row++) {
	    // Left Side Border Column
	    LayerAt(	win->layer, attr,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row + row)) = border;
	    LayerAt(	win->layer, code,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row + row)) = 0x20;

	    for (col = 0; col < win->matrix.size.wth; col++)
			PrintContent(win, list, col, row);

	    // Right Side Border Column
	    LayerAt(	win->layer, attr,
			(win->matrix.origin.col
			+ col * TCellAt(win, 0, 0).length),
			(win->matrix.origin.row + row)) = border;
	    LayerAt(	win->layer, code,
			(win->matrix.origin.col
			+ col * TCellAt(win, 0, 0).length),
			(win->matrix.origin.row + row)) = 0x20;
	}
	// Bottom, Left Border Corner
	LayerAt(win->layer, attr,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row + win->matrix.size.hth)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row + win->matrix.size.hth)) = 0x20;
	// Bottom Border Line
	LayerFillAt(win->layer,
		win->matrix.origin.col,
		(win->matrix.origin.row + win->matrix.size.hth),
		(win->lazyComp.rowLen - 2), hLine, border);
	// Bottom, Right Border Corner
	LayerAt(win->layer, attr,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row + win->matrix.size.hth)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row + win->matrix.size.hth)) = 0x20;
}

void EraseWindowWithBorder(Window *win)
{	// Care about the four window side borders.
	unsigned short row;
	for (row = 0; row < win->matrix.size.hth + 2; row++) {
		Coordinate origin = {
			.col = win->matrix.origin.col - 1,
			.row = (win->matrix.origin.row - 1) + row
		};
		size_t len = win->lazyComp.rowLen + 1;

		memset(&LayerAt(win->layer, attr, origin.col,origin.row),0,len);
		memset(&LayerAt(win->layer, code, origin.col,origin.row),0,len);
	}
}

void MotionReset_Win(Window *win)
{
	win->matrix.scroll.horz = win->matrix.select.col = 0;
	win->matrix.scroll.vert = win->matrix.select.row = 0;
}

void MotionLeft_Win(Window *win)
{
	if (win->matrix.select.col > 0)
		win->matrix.select.col--;
	else
		win->matrix.select.col = win->matrix.size.wth - 1;
}

void MotionRight_Win(Window *win)
{
	if (win->matrix.select.col < win->matrix.size.wth - 1)
		win->matrix.select.col++;
	else
		win->matrix.select.col = 0;
}

void MotionUp_Win(Window *win)
{
	if (win->matrix.select.row > 0)
		win->matrix.select.row--;
	else if (win->matrix.scroll.vert > 0)
		win->matrix.scroll.vert--;
}

void MotionDown_Win(Window *win)
{
	if (win->matrix.select.row < win->matrix.size.hth - 1)
		win->matrix.select.row++;
	else if (win->matrix.scroll.vert < win->lazyComp.bottomRow)
		win->matrix.scroll.vert++;
}

void MotionHome_Win(Window *win)
{
	win->matrix.select.col = 0;
}

void MotionEnd_Win(Window *win)
{
	win->matrix.select.col = win->matrix.size.wth - 1;
}

void MotionPgUp_Win(Window *win)
{
	if (win->matrix.scroll.vert >= win->matrix.size.hth)
		win->matrix.scroll.vert -= win->matrix.size.hth;
	else
		win->matrix.scroll.vert = 0;
}

void MotionPgDw_Win(Window *win)
{
    if(win->matrix.scroll.vert < win->lazyComp.bottomRow - win->matrix.size.hth)
	win->matrix.scroll.vert += win->matrix.size.hth;
    else
	win->matrix.scroll.vert = win->lazyComp.bottomRow;
}


void MotionOriginLeft_Win(Window *win)
{
	if (win->matrix.origin.col > 1) {
		EraseWindowWithBorder(win);
		win->matrix.origin.col--;
	}
}

void MotionOriginRight_Win(Window *win)
{	// Care about the right-side window border.
	unsigned short maxVisibleCol = MIN(MAX_WIDTH, GetScreenSize().width)
					- win->lazyComp.rowLen;

	if (win->matrix.origin.col <= maxVisibleCol) {
		EraseWindowWithBorder(win);
		win->matrix.origin.col++;
	}
}

void MotionOriginUp_Win(Window *win)
{
	if (win->matrix.origin.row > 1) {
		EraseWindowWithBorder(win);
		win->matrix.origin.row--;
	}
}

void MotionOriginDown_Win(Window *win)
{	// Care about the bottom window border.
	unsigned short maxVisibleRow = MIN(MAX_HEIGHT, GetScreenSize().height)
					- win->matrix.size.hth - 1;

	if (win->matrix.origin.row < maxVisibleRow) {
		EraseWindowWithBorder(win);
		win->matrix.origin.row++;
	}
}

int Motion_Trigger(SCANKEY *scan, Window *win, WinList *list)
{
	switch (scan->key) {
	case SCANKEY_ESC:
		{
		Layer *thisLayer = win->layer;

		if (win->hook.key.Escape != NULL)
			win->hook.key.Escape(win);
		else
			RemoveWindow(win, list);

		ResetLayer(thisLayer);
		}
		break;
	case SCANKEY_TAB:
		AnimateWindow(1, list);
		break;
	case SCANKEY_SHIFT_TAB:
	case SCANCON_SHIFT_TAB:
		AnimateWindow(0, list);
		break;
	case SCANKEY_LEFT:
		if (win->hook.key.Left != NULL)
			win->hook.key.Left(win);
		break;
	case SCANKEY_RIGHT:
		if (win->hook.key.Right != NULL)
			win->hook.key.Right(win);
		break;
	case SCANKEY_DOWN:
		if (win->hook.key.Down != NULL)
			win->hook.key.Down(win);
		break;
	case SCANKEY_UP:
		if (win->hook.key.Up != NULL)
			win->hook.key.Up(win);
		break;
	case SCANKEY_HOME:
	case SCANCON_HOME:
		if (win->hook.key.Home != NULL)
			win->hook.key.Home(win);
		break;
	case SCANKEY_END:
	case SCANCON_END:
		if (win->hook.key.End != NULL)
			win->hook.key.End(win);
		break;
	case SCANKEY_PGUP:
		if (win->hook.key.PgUp != NULL)
			win->hook.key.PgUp(win);
		break;
	case SCANKEY_PGDW:
		if (win->hook.key.PgDw != NULL)
			win->hook.key.PgDw(win);
		break;
	case SCANKEY_SHIFT_d:
		if (win->hook.key.WinRight != NULL)
			win->hook.key.WinRight(win);
		break;
	case SCANKEY_SHIFT_a:
	case SCANKEY_SHIFT_q:
		if (win->hook.key.WinLeft != NULL)
			win->hook.key.WinLeft(win);
		break;
	case SCANKEY_SHIFT_w:
	case SCANKEY_SHIFT_z:
		if (win->hook.key.WinUp != NULL)
			win->hook.key.WinUp(win);
		break;
	case SCANKEY_SHIFT_s:
		if (win->hook.key.WinDown != NULL)
			win->hook.key.WinDown(win);
		break;
	case SCANKEY_ENTER:
		if (win->hook.key.Enter != NULL)
			return(win->hook.key.Enter(scan, win));
		// fallthrough
	default:
		return(-1);
	}
	return(0);
}

enum VIEW {V_FREQ, V_INST, V_CYCLES, V_CSTATES, V_TASKS};

#define LOAD_LEAD 4

void Top(SHM_STRUCT *Shm)
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

    Layer	*sLayer = NULL,
		*dLayer = NULL,
		*wLayer = NULL,
		*fuze = NULL;
    WinList	winList = {.head = NULL};

    struct {
	struct {
	unsigned int
		layout	:  1-0,		// Draw layout
		clear	:  2-1,		// Clear screen
		height	:  3-2,		// Valid height
		width	:  4-3,		// Valid width
		daemon	:  5-4,		// Draw dynamic
		taskVal	:  6-5,		// Display task's value
		_pad	: 32-6;
	};
	enum VIEW view;
    } drawFlag = {
	.layout=0,
	.clear=0,
	.height=0,
	.width=0,
	.daemon=0,
	.taskVal=0,
	.view=V_FREQ
    };

    SCREEN_SIZE drawSize = {.width = 0, .height = 0};

    unsigned int timeout = Shm->Proc.SleepInterval * BASE_SLEEP;
    struct timespec tsec = {
	.tv_sec  = timeout / 1000000L,
	.tv_nsec = (timeout % 1000000L) * 1000
    };

    unsigned int digit[9], cpu=0, iClock=0, ratioCount=0, i;

    int MIN_HEIGHT = (2 * Shm->Proc.CPU.Count)
		 + TOP_HEADER_ROW + TOP_SEPARATOR + TOP_FOOTER_ROW,
	loadWidth = 0;

    double minRatio = Shm->Proc.Boost[0], maxRatio = Shm->Proc.Boost[9],
	medianRatio = (minRatio + maxRatio) / 2, availRatio[10] = {minRatio};

    const unsigned int
	isTurboBoost = (Shm->Proc.TurboBoost == Shm->Proc.TurboBoost_Mask),
	isSpeedStep = (Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask),
	isEnhancedHaltState = (Shm->Proc.C1E == Shm->Proc.C1E_Mask),
	isC3autoDemotion = (Shm->Proc.C3A == Shm->Proc.C3A_Mask),
	isC1autoDemotion = (Shm->Proc.C1A == Shm->Proc.C1A_Mask),
	isC3undemotion = (Shm->Proc.C3U == Shm->Proc.C3U_Mask),
	isC1undemotion = (Shm->Proc.C1U == Shm->Proc.C1U_Mask);

    char *buffer = NULL, *viewMask = NULL;

    typedef char HBCLK[11 + 1];
    HBCLK *hBClk;

    void ForEachCellPrint_Menu(Window *win, void *plist)
    {
	WinList *list = (WinList *) plist;
	unsigned short col, row;
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
				win->matrix.origin.col + win->lazyComp.rowLen,
				win->matrix.origin.row,
				len, hSpace,
				win->hook.color[0].title);
    }

    int MotionEnter_Menu(SCANKEY *scan, Window *win)
    {
	if ((scan->key = TCellAt(win,
				win->matrix.select.col,
				win->matrix.select.row).quick.key)
		!= SCANKEY_NULL) {
			SCANKEY closeKey = {.key = SCANKEY_ESC};
			Motion_Trigger(&closeKey, win, &winList);
			return(1);
	} else
		return(0);
    }

    #define EraseTCell_Menu(win)					\
    (									\
	{								\
	    CoordShift shift = {					\
		.horz = win->matrix.scroll.horz + win->matrix.select.col,\
		.vert = win->matrix.scroll.vert + row			\
	    };								\
	    Coordinate cell = {						\
		.col =	(win->matrix.origin.col				\
			+ (win->matrix.select.col			\
			* TCellAt(win, shift.horz, shift.vert).length)), \
			(win->matrix.origin.row + row),			\
		.row =	win->matrix.origin.row + row			\
	    };								\
		memset(&LayerAt(win->layer, attr, cell.col, cell.row),	\
			0,						\
			TCellAt(win, shift.horz, shift.vert).length);	\
		memset(&LayerAt(win->layer, code, cell.col, cell.row),	\
			0,						\
			TCellAt(win, shift.horz, shift.vert).length);	\
	}								\
    )

    void MotionLeft_Menu(Window *win)
    {
	unsigned short row;
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
	unsigned short row;
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
	unsigned short row = win->matrix.select.row;

	if (win->matrix.select.row > 0)
		row--;

	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			win->matrix.select.row = row;
    }

    void MotionDown_Menu(Window *win)
    {
	unsigned short row = win->matrix.select.row;

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
	unsigned short row = 0;
	for (row = win->matrix.size.hth - 1; row > 1; row--)
	    if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			break;

	win->matrix.select.row = row;
    }

    Window *CreateMenu(unsigned long long id)
    {
	Window *wMenu = CreateWindow(wLayer, id, 3, 9, 3, 0);
	if (wMenu != NULL) {
		Attribute sameAttr = {.fg = BLACK, .bg = WHITE, .bf = 0},
			voidAttr = {.value = 0},
			helpAttr[18] = {
				LKW,LKW,LKW,LKW,LKW,LKW,_LKW,LKW,LKW,
				LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW
			},
			quitAttr[18] = {
				LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,
				LKW,LKW,LKW,LKW,HKW,_LKW,_LKW,HKW,LKW
			},
			skeyAttr[18] = {
				LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,
				LKW,LKW,LKW,LKW,LKW,HKW,_LKW,HKW,LKW
			};

		StoreTCell(wMenu, SCANKEY_h,	"      Help        ", helpAttr);
		StoreTCell(wMenu, SCANKEY_NULL,	"      View        ", sameAttr);
		StoreTCell(wMenu, SCANKEY_NULL,	"      Window      ", sameAttr);

		StoreTCell(wMenu, SCANKEY_s,	" Settings     [s] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_f,	" Frequency    [f] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_p,	" Processor    [p] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_a,	" About        [a] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_i,	" Inst cycles  [i] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_m,	" Topology     [m] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_F4,	" Quit        [F4] ", quitAttr);
		StoreTCell(wMenu, SCANKEY_c,	" Core cycles  [c] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_e,	" Features     [e] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SCANKEY_l,	" Idle states  [l] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_t,	" Technologies [t] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SCANKEY_x,	" Task / State [x] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_o,	" Perf. Monit. [o] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SORTBY_TIME,	"   by Time        ", sameAttr);
		StoreTCell(wMenu, SCANKEY_w,	" PowerThermal [w] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SORTBY_PID,	"   by PID         ", sameAttr);
		StoreTCell(wMenu, SCANKEY_u,	" CPUID        [u] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SORTBY_COMM,	"   by Command     ", sameAttr);
		StoreTCell(wMenu, SCANKEY_k,	" Kernel       [k] ", skeyAttr);

		StoreWindow(wMenu, .color[0].select, MakeAttr(BLACK,0,WHITE,0));
		StoreWindow(wMenu, .color[0].title, MakeAttr(BLACK,0,WHITE,0));
		StoreWindow(wMenu, .color[1].title, MakeAttr(BLACK,0,WHITE,1));

		StoreWindow(wMenu,	.Print,		ForEachCellPrint_Menu);
		StoreWindow(wMenu,	.key.Enter,	MotionEnter_Menu);
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
      Window *wSet = CreateWindow(wLayer, id, 2, 4, 8, TOP_HEADER_ROW + 3);
      if (wSet != NULL) {
	char intvStr[16];
	int intvLen = sprintf(intvStr, "%15u", Shm->Proc.SleepInterval);
	size_t appLen = strlen(Shm->AppName);

	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Daemon gate    ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Interval(msec) ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	memcpy(&TCellAt(wSet, 1, 1).item[15 - appLen], Shm->AppName, appLen);
	memcpy(&TCellAt(wSet, 1, 2).item[15 - intvLen], intvStr, intvLen);

	StoreWindow(wSet, .title, " Settings ");
	StoreWindow(wSet, .color[0].select, MAKE_PRINT_UNFOCUS);
	StoreWindow(wSet, .color[1].select, MAKE_PRINT_FOCUS);

	StoreWindow(wSet,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wSet,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wSet,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wSet,	.key.WinUp,	MotionOriginUp_Win);
      }
      return(wSet);
    }

    Window *CreateHelp(unsigned long long id)
    {
      Window *wHelp = CreateWindow(wLayer, id, 2, 17, 2, TOP_HEADER_ROW + 2);
      if (wHelp != NULL) {
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [F2]             ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "             Menu ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Escape]         ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "     Close window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Shift]+[Tab]    ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "  Previous window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Tab]            ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "      Next window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       [A|Z]      ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [W|Q]  [S]  [D]  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "      Move window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       [Up]       ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Left]    [Right]", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "   Move selection ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "      [Down]      ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [End]            ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "        Last cell ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Home]           ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       First cell ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Enter]          ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "Trigger selection ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Page-Up]        ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "    Previous page ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Page-Dw]        ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "        Next page ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);

	StoreWindow(wHelp, .title, " Help ");
	StoreWindow(wHelp, .color[0].select, MAKE_PRINT_UNFOCUS);
	StoreWindow(wHelp, .color[1].select, MAKE_PRINT_FOCUS);

	StoreWindow(wHelp,	.key.WinLeft,	MotionOriginLeft_Win);
	StoreWindow(wHelp,	.key.WinRight,	MotionOriginRight_Win);
	StoreWindow(wHelp,	.key.WinDown,	MotionOriginDown_Win);
	StoreWindow(wHelp,	.key.WinUp,	MotionOriginUp_Win);
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
	"   ""         (C)2015-2017 CYRIL INGENIERIE        ""   "
      };
	size_t	c = sizeof(C) / sizeof(C[0]),
		f = sizeof(F) / sizeof(F[0]),
		l = strlen(C[0]);

	Window *wAbout = CreateWindow(	wLayer,
					id,
					1, c + f,
					(drawSize.width - l) / 2,
					TOP_HEADER_ROW + 4);
	if (wAbout != NULL) {
		unsigned int i;

		for (i = 0; i < c; i++)
			StoreTCell(wAbout,SCANKEY_NULL, C[i], MAKE_PRINT_FOCUS);
		for (i = 0; i < f; i++)
			StoreTCell(wAbout,SCANKEY_NULL, F[i], MAKE_PRINT_FOCUS);

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

    void MotionEnd_SysInfo(Window *win)
    {
	win->matrix.scroll.vert = win->lazyComp.bottomRow;
	win->matrix.select.row  = win->matrix.size.hth - 1;
    }

    Window *CreateSysInfo(unsigned long long id)
    {
	CoordSize matrixSize = {.wth = 1, .hth = 18};
	Coordinate winOrigin = {.col = 3, .row = TOP_HEADER_ROW + 1};
	unsigned short winWidth = 74;
	void (*SysInfoFunc)(SHM_STRUCT*, unsigned short, void(*OutFunc)(char*));
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
		matrixSize.hth = 5;
		winOrigin.col = 23;
		winOrigin.row = TOP_HEADER_ROW + 12;
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
		matrixSize.hth = 7;
		winOrigin.col = 23;
		winOrigin.row = TOP_HEADER_ROW + 2;
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
		matrixSize.hth = 7;
		winOrigin.col = 4;
		winOrigin.row = TOP_HEADER_ROW + 8;
		SysInfoFunc = SysInfoKernel;
		title = " Kernel ";
		}
		break;
	}

	int i = 0;

	Window *wSysInfo = CreateWindow(wLayer,
					id,
					matrixSize.wth,
					matrixSize.hth,
					winOrigin.col,
					winOrigin.row);

	void AddSysInfoCell(char *input)
	{
		i++;
		StoreTCell(wSysInfo, SCANKEY_NULL, input, MAKE_PRINT_FOCUS);
	}

	if (wSysInfo != NULL) {
		SysInfoFunc(Shm, winWidth, AddSysInfoCell);

		while (i < matrixSize.hth) {	// Pad with blank rows.
			i++;
			StoreTCell(wSysInfo,
				SCANKEY_NULL,
				&hSpace[MAX_WIDTH - winWidth],
				MAKE_PRINT_FOCUS);
		}

		switch (id) {
		case SCANKEY_u:
			StoreWindow(wSysInfo,	.color[1].title,
						wSysInfo->hook.color[1].border);
			break;
		default:
			break;
		}
		StoreWindow(wSysInfo,	.title,		title);

		StoreWindow(wSysInfo,	.key.Left,	MotionLeft_Win);
		StoreWindow(wSysInfo,	.key.Right,	MotionRight_Win);
		StoreWindow(wSysInfo,	.key.Down,	MotionDown_Win);
		StoreWindow(wSysInfo,	.key.Up,	MotionUp_Win);
		StoreWindow(wSysInfo,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wSysInfo,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wSysInfo,	.key.Home,	MotionReset_Win);
		StoreWindow(wSysInfo,	.key.End,	MotionEnd_SysInfo);

		StoreWindow(wSysInfo,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wSysInfo,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wSysInfo,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wSysInfo,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wSysInfo);
    }

    Window *CreateTopology(unsigned long long id)
    {
	Window *wTopology = CreateWindow(wLayer,
					id,
					6,
					2 + Shm->Proc.CPU.Count,
					1,
					TOP_HEADER_ROW + 3);

	void AddTopologyCell(char *input)
	{
		StoreTCell(wTopology, SCANKEY_NULL, input, MAKE_PRINT_FOCUS);
	}

	if (wTopology != NULL) {
		Topology(Shm, AddTopologyCell);

		StoreWindow(wTopology,	.title, " Topology ");
		StoreWindow(wTopology,	.key.Left,	MotionLeft_Win);
		StoreWindow(wTopology,	.key.Right,	MotionRight_Win);
		StoreWindow(wTopology,	.key.Down,	MotionDown_Win);
		StoreWindow(wTopology,	.key.Up,	MotionUp_Win);
		StoreWindow(wTopology,	.key.Home,	MotionHome_Win);
		StoreWindow(wTopology,	.key.End,	MotionEnd_Win);

		StoreWindow(wTopology,	.key.WinLeft,	MotionOriginLeft_Win);
		StoreWindow(wTopology,	.key.WinRight,	MotionOriginRight_Win);
		StoreWindow(wTopology,	.key.WinDown,	MotionOriginDown_Win);
		StoreWindow(wTopology,	.key.WinUp,	MotionOriginUp_Win);
	}
	return(wTopology);
    }

    void FreeAll(void)
    {
	DestroyAllWindows(&winList);

	free(hBClk);
	free(buffer);
	free(viewMask);

	DestroyLayer(sLayer);
	DestroyLayer(dLayer);
	DestroyLayer(wLayer);
	DestroyLayer(fuze);
	free(sLayer);
	free(dLayer);
	free(wLayer);
	free(fuze);
    }

    void AllocAll()
    {
	hBClk = calloc(Shm->Proc.CPU.Count, sizeof(HBCLK));
	buffer = malloc(4 * MAX_WIDTH);
	viewMask = malloc((4 * MAX_WIDTH) * MAX_HEIGHT);

	const CoordSize layerSize = {
		.wth = MAX_WIDTH,
		.hth = MAX_HEIGHT
	};

	sLayer = calloc(1, sizeof(Layer));
	dLayer = calloc(1, sizeof(Layer));
	wLayer = calloc(1, sizeof(Layer));
	fuze   = calloc(1, sizeof(Layer));
	CreateLayer(sLayer, layerSize);
	CreateLayer(dLayer, layerSize);
	CreateLayer(wLayer, layerSize);
	CreateLayer(fuze, layerSize);
    }

    for (i = 1; i < 10; i++)
	if (Shm->Proc.Boost[i] != 0) {
		int sort = Shm->Proc.Boost[i] - availRatio[ratioCount];
		if (sort < 0) {
			availRatio[ratioCount + 1] = availRatio[ratioCount];
			availRatio[ratioCount++]   = Shm->Proc.Boost[i];
		}
		else if (sort > 0)
			availRatio[++ratioCount]   = Shm->Proc.Boost[i];
	}
    ratioCount++;

    void TrapScreenSize(int caught)
    {
	if (caught == SIGWINCH) {
		SCREEN_SIZE currentSize = GetScreenSize();
		if (currentSize.height != drawSize.height) {
			if (currentSize.height > MAX_HEIGHT)
				drawSize.height = MAX_HEIGHT;
			else
				drawSize.height = currentSize.height;

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
	}
    }

    int Shortcut(SCANKEY *scan)
    {
	switch (scan->key) {
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
		Shutdown = 0x1;
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
	case SCANKEY_c:
		{
		drawFlag.view = V_CYCLES;
		drawFlag.clear = 1;
		}
		break;
	case SCANKEY_f:
		{
		drawFlag.view = V_FREQ;
		drawFlag.clear = 1;
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
		drawFlag.view = V_INST;
		drawFlag.clear = 1;
		}
		break;
	case SCANKEY_l:
		{
		drawFlag.view = V_CSTATES;
		drawFlag.clear = 1;
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
		if (drawFlag.view == V_TASKS) {
			Shm->SysGate.reverseOrder = !Shm->SysGate.reverseOrder;
			drawFlag.layout = 1;
		}
		break;
	case SCANKEY_v:
		if (drawFlag.view == V_TASKS) {
			drawFlag.taskVal = !drawFlag.taskVal;
			drawFlag.layout = 1;
		}
		break;
	case SCANKEY_x:
		{
		Shm->SysGate.sortByField = 0;
		drawFlag.view = V_TASKS;
		drawFlag.clear = 1;
		}
		break;
	case SORTBY_TIME:
		{
		Shm->SysGate.sortByField = 1;
		drawFlag.view = V_TASKS;
		drawFlag.clear = 1;
		}
		break;
	case SORTBY_PID:
		{
		Shm->SysGate.sortByField = 2;
		drawFlag.view = V_TASKS;
		drawFlag.clear = 1;
		}
		break;
	case SORTBY_COMM:
		{
		Shm->SysGate.sortByField = 3;
		drawFlag.view = V_TASKS;
		drawFlag.clear = 1;
		}
		break;
	case SCANKEY_e:
	case SCANKEY_k:
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
		return(-1);
	}
	return(0);
    }

    void Layout(Layer *layer)
    {
	size_t len;
	unsigned short col = 0, row = 0;
	loadWidth = drawSize.width - LOAD_LEAD;

	LayerDeclare(12) hProc0 = {
		.origin = {.col = 12, .row = row}, .length = 12,
		.attr = {LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK},
		.code = {' ','P','r','o','c','e','s','s','o','r',' ','['}
	};

	LayerDeclare(9) hProc1 = {
		.origin = {.col = drawSize.width - 9, .row = row}, .length = 9,
		.attr = {LWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,LWK},
		.code = {']',' ',' ','/',' ',' ','C','P','U'}
	};

	row++;

	LayerDeclare(15) hArch0 = {
	    .origin = {.col = 12, .row = row}, .length = 15,
	    .attr={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK},
	    .code={' ','A','r','c','h','i','t','e','c','t','u','r','e',' ','['}
	};

	LayerDeclare(30) hArch1 = {
		.origin = {.col = drawSize.width - 30, .row = row},.length = 30,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,	\
			LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,LWK,LWK
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

	LayerDeclare(18) hBClk1 = {
		.origin = {.col = drawSize.width - 18, .row = row},.length = 18,
		.attr ={LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,		\
			LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK
		},
		.code ={'L','2','=',' ',' ',' ',' ',' ',		\
			'L','3','=',' ',' ',' ',' ',' ','K','B'
		}
	};

	row++;

	sprintf(buffer, "%2u" "%-2u",
		Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	hProc1.code[1] = buffer[0];
	hProc1.code[2] = buffer[1];
	hProc1.code[4] = buffer[2];
	hProc1.code[5] = buffer[3];

	unsigned int L1I_Size = 0, L1D_Size = 0, L2U_Size = 0, L3U_Size = 0;
	if (!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12)) {
		L1I_Size = Shm->Cpu[0].Topology.Cache[0].Size / 1024;
		L1D_Size = Shm->Cpu[0].Topology.Cache[1].Size / 1024;
		L2U_Size = Shm->Cpu[0].Topology.Cache[2].Size / 1024;
		L3U_Size = Shm->Cpu[0].Topology.Cache[3].Size / 1024;
	} else {
	    if (!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12)) {
		L1I_Size = Shm->Cpu[0].Topology.Cache[0].Size;
		L1D_Size = Shm->Cpu[0].Topology.Cache[1].Size;
		L2U_Size = Shm->Cpu[0].Topology.Cache[2].Size;
		L3U_Size = Shm->Cpu[0].Topology.Cache[3].Size;
	    }
	}
	sprintf(buffer, "%-3u" "%-3u", L1I_Size, L1D_Size);

	hArch1.code[17] = buffer[0];
	hArch1.code[18] = buffer[1];
	hArch1.code[19] = buffer[2];
	hArch1.code[25] = buffer[3];
	hArch1.code[26] = buffer[4];
	hArch1.code[27] = buffer[5];

	sprintf(buffer, "%-4u" "%-5u", L2U_Size, L3U_Size);

	hBClk1.code[ 3] = buffer[0];
	hBClk1.code[ 4] = buffer[1];
	hBClk1.code[ 5] = buffer[2];
	hBClk1.code[ 6] = buffer[3];
	hBClk1.code[11] = buffer[4];
	hBClk1.code[12] = buffer[5];
	hBClk1.code[13] = buffer[6];
	hBClk1.code[14] = buffer[7];
	hBClk1.code[15] = buffer[8];

	LayerDeclare(MAX_WIDTH) hLoad0 = {
		.origin = {.col = 0, .row = row}, .length = drawSize.width,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LCK,LCK,		\
			LCK,LCK,LCK,LCK,LCK,LCK,LCK,LWK,		\
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
		.code ={"------------- CPU Ratio ----------------"	\
			"----------------------------------------"	\
			"----------------------------------------"	\
			"------------"
		}
	};

	for (i = 0; i < ratioCount; i++) {
		char tabStop[] = "00";
		int hPos = availRatio[i] * loadWidth / maxRatio;
		sprintf(tabStop, "%2.0f", availRatio[i]);

		if (tabStop[0] != 0x20) {
			hLoad0.code[hPos + 2] = tabStop[0];
			hLoad0.attr[hPos + 2] = MakeAttr(CYAN, 0, BLACK, 0);
		}
		hLoad0.code[hPos + 3] = tabStop[1];
		hLoad0.attr[hPos + 3] = MakeAttr(CYAN, 0, BLACK, 0);
	}
	len = strlen(Shm->Proc.Brand);

	LayerCopyAt(layer, hProc0.origin.col, hProc0.origin.row,
			hProc0.length, hProc0.attr, hProc0.code);

	LayerFillAt(layer, hProc0.origin.col + hProc0.length, hProc0.origin.row,
			len, Shm->Proc.Brand,
			MakeAttr(CYAN, 0, BLACK, 1));

	if ((hProc1.origin.col - len) > 0)
	    LayerFillAt(layer, hProc0.origin.col + hProc0.length + len,
			hProc0.origin.row,
			hProc1.origin.col - len, hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hProc1.origin.col, hProc1.origin.row,
			hProc1.length, hProc1.attr, hProc1.code);

	len = strlen(Shm->Proc.Architecture);

	LayerCopyAt(layer, hArch0.origin.col, hArch0.origin.row,
			hArch0.length, hArch0.attr, hArch0.code);

	LayerFillAt(layer, hArch0.origin.col + hArch0.length, hArch0.origin.row,
			len, Shm->Proc.Architecture,
			MakeAttr(CYAN, 0, BLACK, 1));

	if ((hArch1.origin.col - len) > 0)
	    LayerFillAt(layer, hArch0.origin.col + hArch0.length + len,
			hArch0.origin.row,
			hArch1.origin.col - len, hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hArch1.origin.col, hArch1.origin.row,
			hArch1.length, hArch1.attr, hArch1.code);

	LayerCopyAt(layer, hBClk0.origin.col, hBClk0.origin.row,
			hBClk0.length, hBClk0.attr, hBClk0.code);

	LayerFillAt(layer, hBClk0.origin.col + hBClk0.length, hBClk0.origin.row,
			hBClk1.origin.col - hBClk0.origin.col + hBClk0.length,
			hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hBClk1.origin.col, hBClk1.origin.row,
			hBClk1.length, hBClk1.attr, hBClk1.code);

	LayerCopyAt(layer, hLoad0.origin.col, hLoad0.origin.row,
			hLoad0.length, hLoad0.attr, hLoad0.code);

    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
    {
	row++;
	sprintf(buffer, "%-2u", cpu);

	LayerAt(layer, attr, 0, row) =					\
		LayerAt(layer, attr, 0, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, 0, row) =					\
		LayerAt(layer, code, 0, (1 + row + Shm->Proc.CPU.Count)) = '#';
	LayerAt(layer, code, 1, row) =					\
		LayerAt(layer, code, 1, (1 + row + Shm->Proc.CPU.Count)) = \
			buffer[0];
	LayerAt(layer, code, 2, row) =					\
		LayerAt(layer, code, 2, (1 + row + Shm->Proc.CPU.Count)) = \
			buffer[1];
	LayerAt(layer, attr, 3, row) = MakeAttr(YELLOW, 0, BLACK, 1);
	LayerAt(layer, code, 3, row) = 0x20;

      if (!Shm->Cpu[cpu].OffLine.OS) {
	LayerAt(layer, attr, 1, row) =					\
		LayerAt(layer, attr, 1, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(CYAN, 0, BLACK, 0);
	LayerAt(layer, attr, 2, row) =					\
		LayerAt(layer, attr, 2, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(CYAN, 0, BLACK, 0);
      } else {
	LayerAt(layer, attr, 1, row) =					\
		LayerAt(layer, attr, 1, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(BLUE, 0, BLACK, 0);
	LayerAt(layer, attr, 2, row) =					\
		LayerAt(layer, attr, 2, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(BLUE, 0, BLACK, 0);
      }

	switch (drawFlag.view) {
	default:
	case V_FREQ:
	  {
	    LayerDeclare(77) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = 77,
		.attr ={HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,		\
			LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,		\
			HBK,HBK,HBK,LWK,				\
			LWK,LWK,LWK,LWK,				\
			LYK,LYK,LYK					\
		},
		.code ={0x0,						\
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
	  }
	  break;
	case V_INST:
	  {
	    LayerDeclare(76) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = 76,
		.attr ={HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code ={0x0,						\
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
	  }
	  break;
	case V_CYCLES:
	case V_CSTATES:
	  {
	    LayerDeclare(73) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = 73,
		.attr ={HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code ={0x0,						\
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
	  }
	  break;
	case V_TASKS:
	  {
	    LayerDeclare(MAX_WIDTH - LOAD_LEAD + 1) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = MAX_WIDTH - LOAD_LEAD + 1,
		.attr ={HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			LRK,LRK,LRK,LRK,LRK,LRK,LRK,LRK,LRK,LRK,	\
			LRK,LRK,LRK,LRK,LRK,LRK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK		\
		},
		.code ={0x0,						\
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
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
		}
	    };
	    LayerCopyAt(layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);
	  }
	  break;
	}

	i = Dec2Digit(Shm->Cpu[cpu].Clock.Hz, digit);
	sprintf(hBClk[cpu],
		"%u%u%u %u%u%u %u%u%u",
		digit[0], digit[1], digit[2],
		digit[3], digit[4], digit[5],
		digit[6], digit[7], digit[8]);
    }

	row++;

	switch (drawFlag.view) {
	default:
	case V_FREQ:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"--- Freq(MHz) Ratio - Turbo --- "			\
		"C0 ---- C1 ---- C3 ---- C6 ---- C7 --"			\
		"Min TMP Max "						\
		"---------------------------------------------------",
		MakeAttr(WHITE, 0, BLACK, 0));

	    LayerDeclare(70) hAvg0 = {
		.origin={.col=0,.row=(row + Shm->Proc.CPU.Count +1)},.length=70,
		.attr ={LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code ={'-','-','-','-','-','-','-','-',		\
			' ','A','v','e','r','a','g','e','s',' ','[',	\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,' ',']',' '
		}
	    };

	    LayerCopyAt(layer, hAvg0.origin.col, hAvg0.origin.row,
			hAvg0.length, hAvg0.attr, hAvg0.code);

	    LayerFillAt(layer, hAvg0.length, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width - hAvg0.length, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));
	  }
	  break;
	case V_INST:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"------------ IPS -------------- IPC ----"		\
		"---------- CPI ------------------ INST -"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));
	  }
	  break;
	case V_CYCLES:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"-------------- C0:UCC ---------- C0:URC "		\
		"------------ C1 ------------- TSC ------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));
	  }
	  break;
	case V_CSTATES:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"---------------- C1 -------------- C3 --"		\
		"------------ C6 -------------- C7 ------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));
	  }
	  break;
	case V_TASKS:
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
		.code = "--- Freq(MHz) --- Tasks                 "	\
			"   -------------------------------------"	\
			"----------------------------------------"	\
			"------------",
	    };

	    LayerDeclare(20) hTask1 = {
		.origin = {.col = 23, .row = row},
		.length = 20,
	    };

	    struct {
		Attribute attr[20];
		ASCII code[20];
	    } hSort[4] = {
		{
		  .attr = {
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LCK,LCK,LCK,LCK,LCK,HDK,LWK,LWK,LWK
		  },
		  .code = {
			'(','s','o','r','t','e','d',' ','b','y',	\
			' ','S','t','a','t','e',')',' ','-','-'
		  }
		},
		{
		  .attr = {
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,HDK,LWK
		  },
		  .code = {
			'(','s','o','r','t','e','d',' ','b','y',	\
			' ','R','u','n','T','i','m','e',')',' '
		  }
		},
		{
		  .attr = {
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LCK,LCK,LCK,HDK,LWK,LWK,LWK,LWK,LWK
		  },
		  .code = {
			'(','s','o','r','t','e','d',' ','b','y',	\
			' ','P','I','D',')',' ','-','-','-','-'
		  }
		},
		{
		  .attr = {
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
			LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,HDK,LWK
		  },
		  .code = {
			'(','s','o','r','t','e','d',' ','b','y',	\
			' ','C','o','m','m','a','n','d',')',' '
		  }
		}
	    };

	    memcpy(hTask1.attr, hSort[Shm->SysGate.sortByField].attr,
			hTask1.length);
	    memcpy(hTask1.code, hSort[Shm->SysGate.sortByField].code,
			hTask1.length);

	    LayerDeclare(15) hTask2 = {
		.origin = {
			.col = drawSize.width - 18,
			.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = 15,
	    };

	    struct {
		Attribute attr[15];
		ASCII code[15];
	    } hReverse[2] = {
		{
		  .attr = {
		    LWK,_LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK
		  },
		  .code = {
		    ' ', 'R','e','v','e','r','s','e',' ','[','O','F','F',']',' '
		  }
		},
		{
		  .attr = {
		    LWK,_LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LCK,LCK,LCK,HDK,LWK
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
			.col = drawSize.width - 34,
			.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = 13,
		.attr = {
		    LWK,_LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK
		},
		.code = {
		    ' ', 'V','a','l','u','e',' ','[',' ',' ',' ',']',' '
		}
	    };

	    struct {
		Attribute attr[3];
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

	    LayerCopyAt(layer, hTask0.origin.col, hTask0.origin.row,
			hTask0.length, hTask0.attr, hTask0.code);

	    LayerCopyAt(layer, hTask1.origin.col, hTask1.origin.row,
			hTask1.length, hTask1.attr, hTask1.code);

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	    LayerCopyAt(layer, hTask2.origin.col, hTask2.origin.row,
			hTask2.length, hTask2.attr, hTask2.code);

	    LayerCopyAt(layer, hTask3.origin.col, hTask3.origin.row,
			hTask3.length, hTask3.attr, hTask3.code);
	  }
	  break;
	}

	row += Shm->Proc.CPU.Count + 2;

	LayerDeclare(61) hTech0 = {
		.origin = {.col = 0, .row = row}, .length = 14,
		.attr={LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK},
		.code={'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',','},
	};

	const Attribute Pwr[] = {
		MakeAttr(BLACK, 0, BLACK, 1),
		MakeAttr(GREEN, 0, BLACK, 1)
	};
	const struct { ASCII *code; Attribute attr; } TSC[] = {
		(ASCII *)"  TSC  ",  MakeAttr(BLACK, 0, BLACK, 1),
		(ASCII *)"TSC-VAR" , MakeAttr(BLUE,  0, BLACK, 1),
		(ASCII *)"TSC-INV" , MakeAttr(GREEN, 0, BLACK, 1)
	};

	hTech0.code[ 6] = TSC[Shm->Proc.InvariantTSC].code[0];
	hTech0.code[ 7] = TSC[Shm->Proc.InvariantTSC].code[1];
	hTech0.code[ 8] = TSC[Shm->Proc.InvariantTSC].code[2];
	hTech0.code[ 9] = TSC[Shm->Proc.InvariantTSC].code[3];
	hTech0.code[10] = TSC[Shm->Proc.InvariantTSC].code[4];
	hTech0.code[11] = TSC[Shm->Proc.InvariantTSC].code[5];
	hTech0.code[12] = TSC[Shm->Proc.InvariantTSC].code[6];

	hTech0.attr[ 6] = hTech0.attr[ 7]=hTech0.attr[ 8] =
	hTech0.attr[ 9] = hTech0.attr[10]=hTech0.attr[11] =
	hTech0.attr[12] = TSC[Shm->Proc.InvariantTSC].attr;

	LayerCopyAt(layer, hTech0.origin.col, hTech0.origin.row,
			hTech0.length, hTech0.attr, hTech0.code);

	if (!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12))
	{
	    LayerDeclare(65) hTech1 = {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=51,
		.attr ={HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK, \
			HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,	\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK
		},
		.code ={'H','T','T',',','E','I','S','T',',','I','D','A',',', \
			'T','U','R','B','O',',','C','1','E',',',	\
			' ','P','M',',','C','3','A',',','C','1','A',',',\
			'C','3','U',',','C','1','U',',',		\
			'T','M','1',',','T','M','2',']'
		},
	    };

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =
		Pwr[Shm->Proc.HyperThreading];

		const Attribute TM1[] = {
			MakeAttr(BLACK, 0, BLACK, 1),
			MakeAttr(BLUE,  0, BLACK, 1),
			MakeAttr(WHITE, 0, BLACK, 1),
			MakeAttr(GREEN, 0, BLACK, 1)
		};
		const Attribute TM2[] = {
			MakeAttr(BLACK, 0, BLACK, 1),
			MakeAttr(BLUE,  0, BLACK, 1),
			MakeAttr(WHITE, 0, BLACK, 1),
			MakeAttr(GREEN, 0, BLACK, 1)
		};

	    hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] = hTech1.attr[7]=
		Pwr[isSpeedStep];

	    hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] =
		Pwr[Shm->Proc.Features.Power.AX.TurboIDA];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =
	    hTech1.attr[16] = hTech1.attr[17] = Pwr[isTurboBoost];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] =
		Pwr[isEnhancedHaltState];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[23] = buffer[0];
	    hTech1.code[24] = buffer[1];
	    hTech1.code[25] = buffer[2];

	    hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =
		Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] =
		Pwr[isC3autoDemotion];

	    hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] =
		Pwr[isC1autoDemotion];

	    hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] =
		Pwr[isC3undemotion];

	    hTech1.attr[39] = hTech1.attr[40] = hTech1.attr[41] =
		Pwr[isC1undemotion];

	    hTech1.attr[43] = hTech1.attr[44] = hTech1.attr[45] =
		TM1[Shm->Cpu[0].PowerThermal.TM1];

	    hTech1.attr[47] = hTech1.attr[48] = hTech1.attr[49] =
		TM2[Shm->Cpu[0].PowerThermal.TM2];

	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
				hTech1.origin.row,
				(drawSize.width - hTech0.length-hTech1.length),
				hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	} else {
	  if (!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12)) {
	    LayerDeclare(61) hTech1 = {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=35,
		.attr ={HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK,\
			HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,	\
			HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK
		},
		.code ={'H','T','T',',','P','o','w','e','r','N','o','w',',',\
			'T','U','R','B','O',',','C','1','E',',',	\
			' ','P','M',',','D','T','S',',','T','T','P',']'
		},
	    };

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =
		Pwr[Shm->Proc.HyperThreading];

	    hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[ 6] = hTech1.attr[ 7]=
	    hTech1.attr[8] = hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11]=
		Pwr[(Shm->Proc.PowerNow == 0b11)];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =
	    hTech1.attr[16]=hTech1.attr[17]=Pwr[isTurboBoost];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] =
		Pwr[isEnhancedHaltState];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[23] = buffer[0];
	    hTech1.code[24] = buffer[1];
	    hTech1.code[25] = buffer[2];

	    hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =
		Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] =
		Pwr[(Shm->Proc.Features.AdvPower.DX.TS != 0)];

	    hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] =
		Pwr[(Shm->Proc.Features.AdvPower.DX.TTP != 0)];

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
	len = strlen(Shm->SysGate.sysname);

	LayerFillAt(	layer, col, row,
			len, Shm->SysGate.sysname,
			MakeAttr(CYAN, 0, BLACK, 0));

	col += len;

	LayerAt(layer, attr, col, row) = MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, col, row) = 0x20;

	col++;

	LayerAt(layer, attr, col, row) = MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, col, row) = '[';

	col++;
	len = strlen(Shm->SysGate.release);

	LayerFillAt(	layer, col, row,
			len, Shm->SysGate.release,
			MakeAttr(WHITE, 0, BLACK, 1));

	col += len;
	len = strlen(Shm->SysGate.IdleDriver.Name);
	if (len > 0) {
		LayerAt(layer, attr, col, row) = MakeAttr(WHITE, 0, BLACK, 0);
		LayerAt(layer, code, col, row) = '/';

		col++;

		LayerFillAt(	layer, col, row,
				len, Shm->SysGate.IdleDriver.Name,
				MakeAttr(WHITE, 0, BLACK, 1));

		col += len;
	}
	LayerAt(layer, attr, col, row) = MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, code, col, row) = ']';

	col++;

	LayerDeclare(41) hSys1 = {
	    .origin = {.col = (drawSize.width - 41), .row = row}, .length = 41,
	    .attr = {
		LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK, \
		LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK, \
		LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK
		},
	    .code = {
		'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',0x0, \
		' ',0x0,0x0,0x0,' ',0x0,' ',' ',' ',' ',' ',' ',' ',' ', \
		0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ','K','B',']'
		},
	};

	if ((len=hSys1.origin.col - col) > 0)
	    LayerFillAt(layer, col, hSys1.origin.row,
			len, hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hSys1.origin.col, hSys1.origin.row,
			hSys1.length, hSys1.attr, hSys1.code);

	while (++row < drawSize.height)
		LayerFillAt(	layer, 0, row,
				drawSize.width, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
    }

    TrapScreenSize(SIGWINCH);
    signal(SIGWINCH, TrapScreenSize);

    AllocAll();

    while (!Shutdown)
    {
	do {
	  SCANKEY scan = {.key = 0};

	  if ((drawFlag.daemon=BITVAL(Shm->Proc.Sync, 0)) == 0) {
	    if (GetKey(&scan, &tsec) > 0) {
		if (Shortcut(&scan) == -1) {
		    if (IsDead(&winList))
				AppendWindow(CreateMenu(SCANKEY_F2), &winList);
		    else
			if (Motion_Trigger(&scan,GetFocus(&winList),&winList)>0)
				Shortcut(&scan);
		}
		break;
	    }
	  }
	} while (!Shutdown && !drawFlag.daemon && !drawFlag.layout) ;

	if (drawFlag.daemon) {
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);
		if (BITVAL(Shm->Proc.Sync, 63)) {
			// Platform changed, redraw the layout.
			drawFlag.layout = 1;
			BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);
		}
	}

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
		Layout(sLayer);
	}
	if (drawFlag.daemon)
	{
	  for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	  {
	    if (!Shm->Cpu[cpu].OffLine.HW)
	    {
		struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!Shm->Cpu[cpu].OffLine.OS)
		{
		unsigned short bar0=(Flop->Relative.Ratio * loadWidth)/maxRatio,
				bar1 = loadWidth - bar0,
				row = 1 + TOP_HEADER_ROW + cpu;

		    LayerFillAt(dLayer, LOAD_LEAD, row,
				bar0, hBar,
				MakeAttr((Flop->Relative.Ratio > medianRatio ?
					RED : Flop->Relative.Ratio > minRatio ?
					YELLOW : GREEN),
					0, BLACK, 1));

		    LayerFillAt(dLayer, (bar0 + LOAD_LEAD), row,
				bar1, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));

		    row = 2 + TOP_HEADER_ROW + cpu + Shm->Proc.CPU.Count;

		    switch (drawFlag.view) {
		    default:
		    case V_FREQ:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD - 1,row),
				"%c"					\
				"%7.2f" " (" "%5.2f" ") "		\
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% " \
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  " \
				"%-3u" "/" "%3u" "/" "%3u",
				(cpu == iClock) ? '~' : 0x20,
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

			Attribute warning ={.fg=WHITE, .un=0, .bg=BLACK, .bf=1};

			if (Flop->Thermal.Temp <=
				Shm->Cpu[cpu].PowerThermal.Limit[0])
					warning = MakeAttr(BLUE, 0, BLACK, 1);
			else {
				if (Flop->Thermal.Temp >=
				    Shm->Cpu[cpu].PowerThermal.Limit[1])
					warning = MakeAttr(YELLOW, 0, BLACK, 0);
			}
			if (Flop->Thermal.Trip)
				warning = MakeAttr(RED, 0, BLACK, 1);

			LayerAt(dLayer, attr, LOAD_LEAD + 69, row) =
			LayerAt(dLayer, attr, LOAD_LEAD + 70, row) =
			LayerAt(dLayer, attr, LOAD_LEAD + 71, row) = warning;
		      }
		      break;
		    case V_INST:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD - 1,row),
				"%c"					\
				"%17.6f" "/s"				\
				"%17.6f" "/c"				\
				"%17.6f" "/i"				\
				"%18llu",
				(cpu == iClock) ? '~' : 0x20,
				Flop->State.IPS,
				Flop->State.IPC,
				Flop->State.CPI,
				Flop->Delta.INST);
		      }
		      break;
		    case V_CYCLES:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD - 1,row),
				"%c"					\
				"%18llu%18llu%18llu%18llu",
				(cpu == iClock) ? '~' : 0x20,
				Flop->Delta.C0.UCC,
				Flop->Delta.C0.URC,
				Flop->Delta.C1,
				Flop->Delta.TSC);
		      }
		      break;
		    case V_CSTATES:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD - 1,row),
				"%c"					\
				"%18llu%18llu%18llu%18llu",
				(cpu == iClock) ? '~' : 0x20,
				Flop->Delta.C1,
				Flop->Delta.C3,
				Flop->Delta.C6,
				Flop->Delta.C7);
		      }
		      break;
		    case V_TASKS:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD - 1,row),
				"%c"					\
				"%7.2f",
				(cpu == iClock) ? '~' : 0x20,
				Flop->Relative.Freq);
		      }
		      break;
		    }
		} else {
			unsigned short row = 2 + TOP_HEADER_ROW
					   + cpu + Shm->Proc.CPU.Count;

			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD - 1,row),
				"%c", (cpu == iClock) ? '~' : 0x20);
		}
	    }
	  }
	  switch (drawFlag.view) {
	  case V_FREQ:
	    {
		unsigned short row=2 + TOP_HEADER_ROW + 2 * Shm->Proc.CPU.Count;

		sprintf((char *)&LayerAt(dLayer, code, 20, row),
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "	\
			"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%",
			100.f * Shm->Proc.Avg.Turbo,
			100.f * Shm->Proc.Avg.C0,
			100.f * Shm->Proc.Avg.C1,
			100.f * Shm->Proc.Avg.C3,
			100.f * Shm->Proc.Avg.C6,
			100.f * Shm->Proc.Avg.C7);
	    }
	    break;
	  case V_TASKS:
	    {
		unsigned long long first = 0xffffffffffffffff;
		int len, vec;
		char taskStruct[64], taskList[MAX_CPU_ROW][MAX_WIDTH];

		memset(taskList, 0, MAX_CPU_ROW * MAX_WIDTH);
		for (i = 0; i < Shm->SysGate.taskCount; i++) {

		    if (first & (1 << Shm->SysGate.taskList[i].wake_cpu)) {
			first ^= (1 << Shm->SysGate.taskList[i].wake_cpu);
			vec = 1;
		    } else
			vec = 0;

		    if (drawFlag.taskVal) {
			switch (Shm->SysGate.sortByField) {
			case 0:
			  {
			  char *fmt[2] = {"%s(%c)  ", "%16s(%c)  "};
			  len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					Shm->SysGate.taskList[i].state == 0 ?
					'R':Shm->SysGate.taskList[i].state > 0 ?
					'S' : 'U');
			  }
			  break;
			case 1:
			  if (Shm->SysGate.taskList[i].runtime
				> 1000000000000000000LLU) {
			    char *fmt[2] = {"%s(%.2f as)  ", "%16s(%.2f as)  "};
			    len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					(double)Shm->SysGate.taskList[i].runtime
					/ 1000000000000000000LLU);
			  } else if (Shm->SysGate.taskList[i].runtime
				> 1000000000000000LLU) {
			    char *fmt[2] = {"%s(%.2f fs)  ", "%16s(%.2f fs)  "};
			    len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					(double)Shm->SysGate.taskList[i].runtime
					/ 1000000000000000LLU);
			  } else if (Shm->SysGate.taskList[i].runtime
				> 1000000000000LLU) {
			    char *fmt[2] = {"%s(%.2f ps)  ", "%16s(%.2f ps)  "};
			    len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					(double)Shm->SysGate.taskList[i].runtime
					/ 1000000000000LLU);
			  } else if (Shm->SysGate.taskList[i].runtime
				> 1000000000LLU) {
			    char *fmt[16] = {"%s(%.2f ns)  ","%16s(%.2f ns)  "};
			    len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					(double)Shm->SysGate.taskList[i].runtime
					/ 1000000000LLU);
			  } else {
			    char *fmt[16] = {"%s(%.2f us)  ","%16s(%.2f us)  "};
			    len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					(double)Shm->SysGate.taskList[i].runtime
					/ 1000000LLU);
			  }
			  break;
			case 2:
			  /* fallthrough */
			case 3:
			  /* fallthrough */
			default:
			    {
			    char *fmt[2] = {"%s(%d)  ", "%16s(%d)  "};
			    len = sprintf(taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm,
					Shm->SysGate.taskList[i].pid);
			    }
			  break;
			}
		    } else {
			char *fmt[2] = {"%s  ", "%16s  "};
			len = sprintf(  taskStruct,
					fmt[vec],
					Shm->SysGate.taskList[i].comm);
		    }
		    len = strlen(taskList[Shm->SysGate.taskList[i].wake_cpu])
			+ len;

		    if (len < (MAX_WIDTH - LOAD_LEAD - 8))
			    strcat(taskList[Shm->SysGate.taskList[i].wake_cpu],
					taskStruct);
		}
		for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
			unsigned short row = 2 + TOP_HEADER_ROW
					   + cpu + Shm->Proc.CPU.Count;

			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD + 8,row),
				"%s", taskList[cpu]);
		}
	    }
	    break;
	  default:
	    break;
	  }

	    unsigned short row = 2 + TOP_HEADER_ROW + TOP_FOOTER_ROW
				+ 2 * Shm->Proc.CPU.Count;

	    sprintf((char *) &LayerAt(dLayer,code,(drawSize.width - 34), row),
			"%6u""]"					\
			" Mem [""%8lu""/""%8lu",
			Shm->SysGate.taskCount,
			Shm->SysGate.memInfo.freeram,
			Shm->SysGate.memInfo.totalram);

	    {
	    struct FLIP_FLOP *Flop =
	    &Shm->Cpu[Shm->Proc.Top].FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];

		Dec2Digit((unsigned int) Flop->Relative.Freq, digit);

		unsigned int lcdColor;
		if (Flop->Relative.Ratio > medianRatio)
			lcdColor = RED;
		else if (Flop->Relative.Ratio > minRatio)
			lcdColor = YELLOW;
		else
			lcdColor = GREEN;

		unsigned short j = 4;
		do {
			short offset = (4 - j) * 3;

			LayerFillAt(dLayer, offset, 0,
					3, lcd[digit[9 - j]][0],
					MakeAttr(lcdColor, 0, BLACK, 1));
			LayerFillAt(dLayer, offset, 1,
					3, lcd[digit[9 - j]][1],
					MakeAttr(lcdColor, 0, BLACK, 1));
			LayerFillAt(dLayer, offset, 2,
					3, lcd[digit[9 - j]][2],
					MakeAttr(lcdColor, 0, BLACK, 1));

			j--;
		} while (j > 0) ;
	    }
	    memcpy(&LayerAt(dLayer, code, 26, 2), hBClk[iClock], 11);

	    do {
		iClock++;
		if (iClock == Shm->Proc.CPU.Count)
			iClock = 0;
	    } while (Shm->Cpu[iClock].OffLine.OS && iClock) ;
	}

	Window *walker;
	if ((walker=GetHead(&winList)) != NULL) {
		do {
			walker = walker->next;

			if (walker->hook.Print != NULL)
				walker->hook.Print(walker, &winList);
			else
				ForEachCellPrint(walker, &winList);
		} while (!IsHead(&winList, walker)) ;
	}
	Attribute attr = {.value = 0};
	unsigned short col, row;
	i = 0;
	for (row = 0; row < drawSize.height; row++)
	{
	  struct {
		int cursor;
	  } flag = {0};
	  int j = 0, k;
	  for (col = 0; col < drawSize.width; col++) {
	    Attribute	*fa=&fuze->attr[col + (row * fuze->size.wth)],
			*sa=&sLayer->attr[col + (row * fuze->size.wth)],
			*da=&dLayer->attr[col + (row * fuze->size.wth)],
			*wa=&wLayer->attr[col + (row * fuze->size.wth)];
	    ASCII	*fc=&fuze->code[col + (row * fuze->size.wth)],
			*sc=&sLayer->code[col + (row * fuze->size.wth)],
			*dc=&dLayer->code[col + (row * fuze->size.wth)],
			*wc=&wLayer->code[col + (row * fuze->size.wth)];
	/* STATIC LAYER */
	    if (sa->value != 0)
		fa->value=sa->value;
	    if (*sc != 0)
		*fc=*sc;
	/* DYNAMIC LAYER */
	    if (da->value != 0)
		fa->value=da->value;
	    if (*dc != 0)
		*fc=*dc;
	/* WINDOWS LAYER */
	    if (wa->value != 0)
		fa->value=wa->value;
	    if (*wc != 0)
		*fc=*wc;
	/* FUZED LAYER */
	    if ((fa->fg ^ attr.fg) || (fa->bg ^ attr.bg) || (fa->bf ^ attr.bf)){
		buffer[j++] = 0x1b;
		buffer[j++] = '[';
		buffer[j++] = '0' + fa->bf;
		buffer[j++] = ';';
		buffer[j++] = '3';
		buffer[j++] = '0' + fa->fg;
		buffer[j++] = ';';
		buffer[j++] = '4';
		buffer[j++] = '0' + fa->bg;
		buffer[j++] = 'm';
	    }
	    if (fa->un ^ attr.un) {
		buffer[j++] = 0x1b;
		buffer[j++] = '[';
		if (fa->un) {
			buffer[j++] = '4';
			buffer[j++] = 'm';
		} else {
			buffer[j++] = '2';
			buffer[j++] = '4';
			buffer[j++] = 'm';
		}
	    }
	    attr.value = fa->value;

	    if (*fc != 0) {
		if (flag.cursor == 0) {
			flag.cursor = 1;

			struct {
				unsigned short col, row;
			} scr = {.col = col + 1, .row = row + 1};

			buffer[j++] = 0x1b;
			buffer[j++] = '[';

			for(j=log10(scr.row)+j+1, k=j; scr.row > 0; scr.row/=10)
				buffer[--k] = '0' + (scr.row % 10);

			buffer[j++] = ';';

			for(j=log10(scr.col)+j+1, k=j; scr.col > 0; scr.col/=10)
				buffer[--k] = '0' + (scr.col % 10);

			buffer[j++] = 'H';
		}
		buffer[j++] = *fc;
	    }
	    else
		flag.cursor = 0;
	  }
	  memcpy(&viewMask[i], buffer, j);
	  i += j;
	}
	fwrite(viewMask, i, 1, stdout);
	fflush(stdout);
      }
      else
	printf(	CUH RoK "Term(%u x %u) < View(%u x %u)\n",
		drawSize.width, drawSize.height, MIN_WIDTH, MIN_HEIGHT);
    }
    FreeAll();
}

int Help(char *appName)
{
	printf(	"CoreFreq."						\
		"  Copyright (C) 2015-2017 CYRIL INGENIERIE\n\n");
	printf(	"usage:\t%s [-option <arguments>]\n"			\
		"\t-t\tShow Top (default)\n"				\
		"\t-d\tShow Dashboard\n"				\
		"\t\t  arguments:"					\
			" <left>"					\
			" <top>"					\
			" <marginWidth>"				\
			" <marginHeight>"				\
		"\n"							\
		"\t-c\tMonitor Counters\n"				\
		"\t-i\tMonitor Instructions\n"				\
		"\t-s\tPrint System Information\n"			\
		"\t-m\tPrint Topology\n"				\
		"\t-u\tPrint CPUID\n"					\
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
	struct termios oldt, newt;
	struct stat shmStat = {0};
	SHM_STRUCT *Shm;
	int fd = -1, rc = EXIT_SUCCESS;

	char *program = strdup(argv[0]), *appName=basename(program);
	char option = 't';
	if ((argc >= 2) && (argv[1][0] == '-'))
		option = argv[1][1];
	if (option == 'h')
		Help(appName);
	else if (((fd = shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) !=-1)
		&& ((fstat(fd, &shmStat) != -1)
		&& ((Shm = mmap(0, shmStat.st_size,
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))!=MAP_FAILED)))
	    {
		switch (option) {
		case 'u':
			SysInfoCPUID(Shm, 80, NULL);
			break;
		case 's':
			{
			SysInfoProc(Shm, 80, NULL);
			printv(NULL, 80, 0, "");
			printv(NULL, 80, 0, "Features:");
			SysInfoFeatures(Shm, 80, NULL);
			printv(NULL, 80, 0, "");
			printv(NULL, 80, 0, "Technologies:");
			SysInfoTech(Shm, 80, NULL);
			printv(NULL, 80, 0, "");
			printv(NULL, 80, 0, "Performance Monitoring:");
			SysInfoPerfMon(Shm, 80, NULL);
			printv(NULL, 80, 0, "");
			printv(NULL,80,0,"Power & Thermal Monitoring:");
			SysInfoPwrThermal(Shm, 80, NULL);
			printv(NULL, 80, 0, "");
			SysInfoKernel(Shm, 80, NULL);

			fflush(stdout);
			}
			break;
		case 'm':
			Topology(Shm, NULL);
			break;
		case 'i':
			TrapSignal();
			Instructions(Shm);
			break;
		case 'c':
			TrapSignal();
			Counters(Shm);
			break;
		case 'd':
			if (argc == 6) {
				printf(SCP SCR1 HIDE);
				TrapSignal();
				Dashboard(Shm,	atoi(argv[2]),
						atoi(argv[3]),
						atoi(argv[4]),
						atoi(argv[5]) );
				printf(SHOW SCR0 RCP COLOR(0,9,9));
			} else if (argc == 2) {
				printf(SCP SCR1 HIDE);
				TrapSignal();
				Dashboard(Shm,	LEADING_LEFT,
						LEADING_TOP,
						MARGIN_WIDTH,
						MARGIN_HEIGHT);
				printf(SHOW SCR0 RCP COLOR(0,9,9));
			}
			else
				rc = Help(appName);
			break;
		case 't':
			{
			printf(SCP SCR1 HIDE);

			tcgetattr(STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~( ICANON | ECHO );
			newt.c_cc[VTIME] = 0;
			newt.c_cc[VMIN] = 0;
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);

			TrapSignal();
			Top(Shm);

			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

			printf(SHOW SCR0 RCP COLOR(0,9,9));
			}
			break;
		default:
			rc = Help(appName);
			break;
		}
		munmap(Shm, shmStat.st_size);
		close(fd);
	    }
		else
			rc = 2;
	free(program);
	return(rc);
}
