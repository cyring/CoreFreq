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
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "intelasm.h"
#include "coretypes.h"
#include "corefreq.h"

unsigned int Shutdown=0x0;

void Emergency(int caught)
{
	switch(caught)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
		case SIGTSTP:
			Shutdown=0x1;
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

char lcd[10][3][3]=
{
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

char hSpace[]=	"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""    ";
char hBar[]=	"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||";
char hLine[]=	"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""----";

typedef struct
{
	int	width,
		height;
} SCREEN_SIZE;

SCREEN_SIZE GetScreenSize(void)
{
	SCREEN_SIZE _screenSize={.width=0, .height=0};
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	_screenSize.width=(int) ts.ws_col;
	_screenSize.height=(int) ts.ws_row;
	return(_screenSize);
}

unsigned int Dec2Digit(unsigned int decimal, unsigned int thisDigit[])
{
	memset(thisDigit, 0, 9 * sizeof(unsigned int));
	unsigned int j=9;
	while(decimal > 0)
	{
		thisDigit[--j]=decimal % 10;
		decimal/=10;
	}
	return(9 - j);
}


void SysInfo(SHM_STRUCT *Shm,unsigned short width,void(*OutFunc)(char *output))
{
	const unsigned int
	    	isTurboBoost=(Shm->Proc.TurboBoost==Shm->Proc.TurboBoost_Mask),
		isSpeedStep=(Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask),
		isEnhancedHaltState=(Shm->Proc.C1E == Shm->Proc.C1E_Mask),
		isC3autoDemotion=(Shm->Proc.C3A == Shm->Proc.C3A_Mask),
		isC1autoDemotion=(Shm->Proc.C1A == Shm->Proc.C1A_Mask),
		isC3undemotion=(Shm->Proc.C3U == Shm->Proc.C3U_Mask),
		isC1undemotion=(Shm->Proc.C1U == Shm->Proc.C1U_Mask);

	char	*line=malloc(width + 1 + 1),
		*row=malloc(width + 1),
		*str=malloc(width + 1),
		*pad=NULL;
	size_t	len=0;
	int	i=0;

	void printv(char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf(line, fmt, ap);
		if(OutFunc == NULL)
			printf("%s\n", line);
		else
			OutFunc(line);
		va_end(ap);
	}
/* Section Mark */
	printv(	"CPUID:" "%.*s", width-6, hSpace);

	printv(	"|- Largest Standard Function%.*s[%08X]",
		width-38, hSpace,
		Shm->Proc.Features.Info.LargestStdFunc);

	printv(	"|- Largest Extended Function%.*s[%08X]",
		width-38, hSpace,
		Shm->Proc.Features.Info.LargestExtFunc);

	printv(	"|- Vendor ID%.*s[%s]",
		width-14 - strlen(Shm->Proc.Features.Info.VendorID), hSpace,
		Shm->Proc.Features.Info.VendorID);

	printv(	"%.*s", width, hSpace);
	printv(	"Processor%.*s[%s]",
		width-11 - strlen(Shm->Proc.Brand), hSpace, Shm->Proc.Brand);

	printv(	"|- Signature%.*s[%1X%1X_%1X%1X]",
		width-19, hSpace,
		Shm->Proc.Features.Std.AX.ExtFamily,
		Shm->Proc.Features.Std.AX.Family,
		Shm->Proc.Features.Std.AX.ExtModel,
		Shm->Proc.Features.Std.AX.Model);

	printv(	"|- Stepping%.*s[%3u]",
		width-16, hSpace, Shm->Proc.Features.Std.AX.Stepping);

	printv(	"|- Architecture%.*s[%s]",
		width-17 - strlen(Shm->Proc.Architecture), hSpace,
		Shm->Proc.Architecture);

	printv(	"|- Online CPU%.*s[%u/%u]",
		width-18, hSpace, Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	printv(	"|- Base Clock%.*s[%3llu]",
		width-18, hSpace, Shm->Cpu[0].Clock.Hz / 1000000L);

	printv(	"|- Ratio Boost:%.*s",
		width-15, hSpace);

	len=sprintf(row, "%.*sMin Max  8C  7C  6C  5C  4C  3C  2C  1C",
		22, hSpace);

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);

	len=sprintf(row, "%.*s", 22, hSpace);
	for(i=0; i < 10; i++)
	{
		if(Shm->Proc.Boost[i] != 0)
			len+=sprintf(str, "%3d ", Shm->Proc.Boost[i]);
		else
			len+=sprintf(str, "  - ");
		strcat(row, str);
	}
	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);
/* Section Mark */
	const char *TSC[]=
	{
		"Missing",
		"Variant",
		"Invariant"
	};
	const char *x2APIC[]=
	{
		"Missing",
	    GoK	"  xAPIC" DoK,
	    GoK	" x2APIC" DoK
	};
	const char *TM[]=
	{
		"Missing",
	    	"Present",
	    	"Disable",
	    	" Enable",
	};
	printv("Instruction set:" "%.*s", width-16, hSpace);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "3DNow!/Ext [%c,%c]",
		Shm->Proc.Features.ExtInfo.DX._3DNow ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.DX._3DNowEx ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 11, hSpace);

	strcat(row, str);

	len+=sprintf(str, "AES [%c]",
		Shm->Proc.Features.Std.CX.AES ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 6, hSpace);

	strcat(row, str);

	len+=sprintf(str, "AVX/AVX2 [%c/%c]",
		Shm->Proc.Features.Std.CX.AVX ? 'Y' : 'N',
			Shm->Proc.Features.ExtFeature.BX.AVX2 ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "BMI1/BMI2 [%c/%c]",
		Shm->Proc.Features.ExtFeature.BX.BMI1 ? 'Y' : 'N',
			Shm->Proc.Features.ExtFeature.BX.BMI2 ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(row);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "CLFSH        [%c]",
		Shm->Proc.Features.Std.DX.CLFSH ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len+=sprintf(str, "CMOV [%c]",
		Shm->Proc.Features.Std.DX.CMOV ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 7, hSpace);

	strcat(row, str);

	len+=sprintf(str, "CMPXCH8   [%c]",
		Shm->Proc.Features.Std.DX.CMPXCH8 ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "CMPXCH16   [%c]",
		Shm->Proc.Features.Std.CX.CMPXCH16 ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(row);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "F16C         [%c]",
		Shm->Proc.Features.Std.CX.F16C ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 11, hSpace);

	strcat(row, str);

	len+=sprintf(str, "FPU [%c]",
		Shm->Proc.Features.Std.DX.FPU ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len+=sprintf(str, "FXSR   [%c]",
	Shm->Proc.Features.Std.DX.FXSR ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "LAHF/SAHF   [%c]",
		Shm->Proc.Features.ExtInfo.CX.LAHFSAHF ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(row);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "MMX/Ext    [%c/%c]",
		Shm->Proc.Features.Std.DX.MMX ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.DX.MMX_Ext ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 7, hSpace);

	strcat(row, str);

	len+=sprintf(str, "MONITOR [%c]",
		Shm->Proc.Features.Std.CX.MONITOR ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 9, hSpace);

	strcat(row, str);

	len+=sprintf(str, "MOVBE   [%c]",
		Shm->Proc.Features.Std.CX.MOVBE ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "PCLMULDQ   [%c]",
		Shm->Proc.Features.Std.CX.PCLMULDQ ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(row);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "POPCNT       [%c]",
		Shm->Proc.Features.Std.CX.POPCNT ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 8, hSpace);

	strcat(row, str);

	len+=sprintf(str, "RDRAND [%c]",
		Shm->Proc.Features.Std.CX.RDRAND ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 8, hSpace);

	strcat(row, str);

	len+=sprintf(str, "RDTSCP   [%c]",
		Shm->Proc.Features.ExtInfo.DX.RDTSCP ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "SEP   [%c]",
		Shm->Proc.Features.Std.DX.SEP ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(row);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "SSE          [%c]",
		Shm->Proc.Features.Std.DX.SSE ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len+=sprintf(str, "SSE2 [%c]",
		Shm->Proc.Features.Std.DX.SSE2 ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 10, hSpace);

	strcat(row, str);

	len+=sprintf(str, "SSE3   [%c]",
		Shm->Proc.Features.Std.CX.SSE3 ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "SSSE3   [%c]",
		Shm->Proc.Features.Std.CX.SSSE3 ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, pad);
	strcat(row, str);
	printv(row);
/* Row Mark */
	len=sprintf(row, "|-" "%.*s", 1, hSpace);

	len+=sprintf(str, "SSE4.1/4A  [%c/%c]",
		Shm->Proc.Features.Std.CX.SSE41 ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.CX.SSE4A ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 8, hSpace);

	strcat(row, str);

	len+=sprintf(str, "SSE4.2 [%c]",
		Shm->Proc.Features.Std.CX.SSE42 ? 'Y' : 'N');

	strcat(row, str);

	len+=sprintf(str, "%.*s", 7, hSpace);

	strcat(row, str);

	len+=sprintf(str, "SYSCALL   [%c]",
		Shm->Proc.Features.ExtInfo.DX.SYSCALL ? 'Y' : 'N');

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);

	strcat(row, str);
	strcat(row, pad);
	printv(row);
/* Section Mark */
	printv(	"%.*s", width, hSpace);

	printv(	"Features:" "%.*s", width-9, hSpace);

	printv(								\
	"|- 1 GB Pages Support%.*s1GB-PAGES   [%7s]",
	width-42, hSpace, powered(Shm->Proc.Features.ExtInfo.DX.PG_1GB));

	printv(								\
	"|- 100 MHz multiplier Control%.*s100MHzSteps   [%7s]",
	width-52, hSpace, powered(Shm->Proc.Features.AdvPower.DX._100MHz));

	printv(								\
	"|- Advanced Configuration & Power Interface"			\
					"%.*sACPI   [%7s]",
	width-59, hSpace, powered(Shm->Proc.Features.Std.DX.ACPI	// Intel
			| Shm->Proc.Features.AdvPower.DX.HwPstate) );	// AMD

	printv(								\
	"|- Advanced Programmable Interrupt Controller"			\
						"%.*sAPIC   [%7s]",
	width-61, hSpace, powered(Shm->Proc.Features.Std.DX.APIC));

	printv(								\
	"|- Core Multi-Processing%.*sCMP Legacy   [%7s]",
	width-46, hSpace, powered(Shm->Proc.Features.ExtInfo.CX.MP_Mode));

	printv(								\
	"|- L1 Data Cache Context ID%.*sCNXT-ID   [%7s]",
	width-46, hSpace, powered(Shm->Proc.Features.Std.CX.CNXT_ID));

	printv(								\
	"|- Direct Cache Access%.*sDCA   [%7s]",
	width-37, hSpace, powered(Shm->Proc.Features.Std.CX.DCA));

	printv(								\
	"|- Debugging Extension%.*sDE   [%7s]",
	width-36, hSpace, powered(Shm->Proc.Features.Std.DX.DE));

	printv(								\
	"|- Debug Store & Precise Event Based Sampling"			\
					"%.*sDS, PEBS   [%7s]",
	width-65, hSpace, powered(Shm->Proc.Features.Std.DX.DS_PEBS));

	printv(								\
	"|- CPL Qualified Debug Store%.*sDS-CPL   [%7s]",
	width-46, hSpace, powered(Shm->Proc.Features.Std.CX.DS_CPL));

	printv(								\
	"|- 64-Bit Debug Store%.*sDTES64   [%7s]",
	width-39, hSpace, powered(Shm->Proc.Features.Std.CX.DTES64));

	printv(								\
	"|- Fast-String Operation%.*sFast-Strings   [%7s]",
	width-48,hSpace, powered(Shm->Proc.Features.ExtFeature.BX.FastStrings));

	printv(								\
	"|- Fused Multiply Add%.*sFMA|FMA4   [%7s]",
	width-41, hSpace, powered(	  Shm->Proc.Features.Std.CX.FMA
				| Shm->Proc.Features.ExtInfo.CX.FMA4 ));

	printv(								\
	"|- Hardware Lock Elision%.*sHLE   [%7s]",
	width-39, hSpace, powered(Shm->Proc.Features.ExtFeature.BX.HLE));

	printv(								\
	"|- Long Mode 64 bits%.*sIA64|LM   [%7s]",
	width-39, hSpace, powered(Shm->Proc.Features.ExtInfo.DX.IA64));

	printv(								\
	"|- LightWeight Profiling%.*sLWP   [%7s]",
	width-39, hSpace, powered(Shm->Proc.Features.ExtInfo.CX.LWP));

	printv(								\
	"|- Machine-Check Architecture%.*sMCA   [%7s]",
	width-44, hSpace, powered(Shm->Proc.Features.Std.DX.MCA));

	printv(								\
	"|- Model Specific Registers%.*sMSR   [%7s]",
	width-42, hSpace, powered(Shm->Proc.Features.Std.DX.MSR));

	printv(								\
	"|- Memory Type Range Registers%.*sMTRR   [%7s]",
	width-46, hSpace, powered(Shm->Proc.Features.Std.DX.MTRR));

	printv(								\
	"|- OS-Enabled Ext. State Management%.*sOSXSAVE   [%7s]",
	width-54, hSpace, powered(Shm->Proc.Features.Std.CX.OSXSAVE));

	printv(								\
	"|- Physical Address Extension%.*sPAE   [%7s]",
	width-44, hSpace, powered(Shm->Proc.Features.Std.DX.PAE));

	printv(								\
	"|- Page Attribute Table%.*sPAT   [%7s]",
	width-38, hSpace, powered(Shm->Proc.Features.Std.DX.PAT));

	printv(								\
	"|- Pending Break Enable%.*sPBE   [%7s]",
	width-38, hSpace, powered(Shm->Proc.Features.Std.DX.PBE));

	printv(								\
	"|- Process Context Identifiers%.*sPCID   [%7s]",
	width-46, hSpace, powered(Shm->Proc.Features.Std.CX.PCID));

	printv(								\
	"|- Perfmon and Debug Capability%.*sPDCM   [%7s]",
	width-47, hSpace, powered(Shm->Proc.Features.Std.CX.PDCM));

	printv(								\
	"|- Page Global Enable%.*sPGE   [%7s]",
	width-36, hSpace, powered(Shm->Proc.Features.Std.DX.PGE));

	printv(								\
	"|- Page Size Extension%.*sPSE   [%7s]",
	width-37, hSpace, powered(Shm->Proc.Features.Std.DX.PSE));

	printv(								\
	"|- 36-bit Page Size Extension%.*sPSE36   [%7s]",
	width-46, hSpace, powered(Shm->Proc.Features.Std.DX.PSE36));

	printv(								\
	"|- Processor Serial Number%.*sPSN   [%7s]",
	width-41, hSpace, powered(Shm->Proc.Features.Std.DX.PSN));

	printv(								\
	"|- Restricted Transactional Memory%.*sRTM   [%7s]",
	width-49, hSpace, powered(Shm->Proc.Features.ExtFeature.BX.RTM));

	printv(								\
	"|- Safer Mode Extensions%.*sSMX   [%7s]",
	width-39, hSpace, powered(Shm->Proc.Features.Std.CX.SMX));

	printv(								\
	"|- Self-Snoop%.*sSS   [%7s]",
	width-27, hSpace, powered(Shm->Proc.Features.Std.DX.SS));

	printv(								\
	"|- Time Stamp Counter%.*sTSC [%9s]",
	width-36, hSpace, TSC[Shm->Proc.InvariantTSC]);

	printv(								\
	"|- Time Stamp Counter Deadline%.*sTSC-DEADLINE   [%7s]",
	width-54, hSpace, powered(Shm->Proc.Features.Std.CX.TSCDEAD));

	printv(								\
	"|- Virtual Mode Extension%.*sVME   [%7s]",
	width-40, hSpace, powered(Shm->Proc.Features.Std.DX.VME));

	printv(								\
	"|- Virtual Machine Extensions%.*sVMX   [%7s]",
	width-44, hSpace, powered(Shm->Proc.Features.Std.CX.VMX));

	printv(								\
	"|- Extended xAPIC Support%.*sx2APIC   [%7s]",
	width-43, hSpace, x2APIC[Shm->Cpu[0].Topology.MP.x2APIC]);

	printv(								\
	"|- Execution Disable Bit Support%.*sXD-Bit   [%7s]",
	width-50, hSpace, powered(Shm->Proc.Features.ExtInfo.DX.XD_Bit));

	printv(								\
	"|- XSAVE/XSTOR States%.*sXSAVE   [%7s]",
	width-38, hSpace, powered(Shm->Proc.Features.Std.CX.XSAVE));

	printv(								\
	"|- xTPR Update Control%.*sxTPR   [%7s]",
	width-38, hSpace, powered(Shm->Proc.Features.Std.CX.xTPR));
/* Section Mark */
	printv(	"%.*s", width, hSpace);

	printv(	"Technologies:" "%.*s", width-13, hSpace);

	printv(								\
	"|- Hyper-Threading%.*sHTT       [%3s]",
	width-33, hSpace, enabled(Shm->Proc.HyperThreading));

	printv(								\
	"|- SpeedStep%.*sEIST       [%3s]",
	width-28, hSpace, enabled(isSpeedStep));

	printv(								\
	"|- PowerNow!%.*sPowerNow       [%3s]",
	width-32, hSpace, enabled(Shm->Proc.PowerNow == 0b11));	// VID + FID

	printv(								\
	"|- Dynamic Acceleration%.*sIDA       [%3s]",
	width-38, hSpace, enabled(Shm->Proc.Features.Power.AX.TurboIDA));

	printv(								\
	"|- Turbo Boost%.*sTURBO|CPB       [%3s]",
	width-35, hSpace, enabled(	  isTurboBoost
				| Shm->Proc.Features.AdvPower.DX.CPB ));
/* Section Mark */
	printv(	"%.*s", width, hSpace);

	printv(	"Performance Monitoring:" "%.*s", width-23, hSpace);

	printv(								\
	"|- Version%.*sPM       [%3d]",
	width-24, hSpace, Shm->Proc.PM_version);

	len=sprintf(row,						\
	"|- Counters:%.*sGeneral%.*sFixed",
	10, hSpace, width-61, hSpace);

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);

	len=sprintf(row,						\
	"|%.*s%3u x%3u bits%.*s%3u x%3u bits",
	19, hSpace,	Shm->Proc.Features.PerfMon.AX.MonCtrs,
			Shm->Proc.Features.PerfMon.AX.MonWidth,
	11, hSpace,	Shm->Proc.Features.PerfMon.DX.FixCtrs,
			Shm->Proc.Features.PerfMon.DX.FixWidth);

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);

	printv(								\
	"|- Enhanced Halt State%.*sC1E       [%3s]",
	width-37, hSpace, enabled(isEnhancedHaltState));

	printv(								\
	"|- C1 Auto Demotion%.*sC1A       [%3s]",
	width-34, hSpace, enabled(isC1autoDemotion));

	printv(								\
	"|- C3 Auto Demotion%.*sC3A       [%3s]",
	width-34, hSpace, enabled(isC3autoDemotion));

	printv(								\
	"|- C1 UnDemotion%.*sC1U       [%3s]",
	width-31, hSpace, enabled(isC1undemotion));

	printv(								\
	"|- C3 UnDemotion%.*sC3U       [%3s]",
	width-31, hSpace, enabled(isC3undemotion));

	printv(								\
	"|- Frequency ID control%.*sFID       [%3s]",
	width-38, hSpace, enabled(Shm->Proc.Features.AdvPower.DX.FID));

	printv(								\
	"|- Voltage ID control%.*sVID       [%3s]",
	width-36, hSpace, enabled(Shm->Proc.Features.AdvPower.DX.VID));

	printv(								\
	"|- P-State Hardware Coordination Feedback"			\
			"%.*sMPERF/APERF       [%3s]",
	width-64, hSpace, enabled(Shm->Proc.Features.Power.CX.HCF_Cap));

	printv(								\
	"|- Hardware-Controlled Performance States%.*sHWP       [%3s]",
	width-56, hSpace, enabled(	  Shm->Proc.Features.Power.AX.HWP_Reg
				| Shm->Proc.Features.AdvPower.DX.HwPstate ));

	printv(								\
	"|- Hardware Duty Cycling%.*sHDC       [%3s]",
	width-39, hSpace, enabled(	  Shm->Proc.Features.Power.AX.HDC_Reg));

	len=sprintf(row,						\
	"|- MWAIT States:%.*sC0      C1      C2      C3      C4",
	06, hSpace);

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);

	len=sprintf(row,						\
	"|%.*s%2d      %2d      %2d      %2d      %2d",
	21, hSpace,
		Shm->Proc.Features.MWait.DX.Num_C0_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C1_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C2_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C3_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C4_MWAIT);

	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);

	printv(								\
	"|- Core Cycles%.*s[%7s]",
	width-23, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.CoreCycles));

	printv(								\
	"|- Instructions Retired%.*s[%7s]",
	width-32, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.InstrRetired));

	printv(								\
	"|- Reference Cycles%.*s[%7s]",
	width-28, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.RefCycles));

	printv(								\
	"|- Last Level Cache References%.*s[%7s]",
	width-39, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.LLC_Ref));

	printv(								\
	"|- Last Level Cache Misses%.*s[%7s]",
	width-35, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.LLC_Misses));

	printv(								\
	"|- Branch Instructions Retired%.*s[%7s]",
	width-39, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.BranchRetired));

	printv(								\
	"|- Branch Mispredicts Retired%.*s[%7s]",
	width-38, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.BranchMispred));
/* Section Mark */
	printv(	"%.*s", width, hSpace);

	printv(	"Power & Thermal Monitoring:" "%.*s", width-27, hSpace);

	printv(								\
	"|- Digital Thermal Sensor%.*sDTS   [%7s]",
	width-40, hSpace, powered(	  Shm->Proc.Features.Power.AX.DTS
				| Shm->Proc.Features.AdvPower.DX.TS ));

	printv(								\
	"|- Power Limit Notification%.*sPLN   [%7s]",
	width-42, hSpace, powered(Shm->Proc.Features.Power.AX.PLN));

	printv(								\
	"|- Package Thermal Management%.*sPTM   [%7s]",
	width-44, hSpace, powered(Shm->Proc.Features.Power.AX.PTM));

	printv(								\
	"|- Thermal Monitor 1%.*sTM1|TTP   [%7s]",
	width-39, hSpace, TM[   Shm->Cpu[0].PowerThermal.TM1
			| Shm->Proc.Features.AdvPower.DX.TTP ]);
	printv(								\
	"|- Thermal Monitor 2%.*sTM2|HTC   [%7s]",
	width-39, hSpace, TM[	  Shm->Cpu[0].PowerThermal.TM2
			| Shm->Proc.Features.AdvPower.DX.TM ]);

	printv(								\
	"|- Clock Modulation%.*sODCM   [%6.2f%%]",
	width-35, hSpace, Shm->Cpu[0].PowerThermal.ODCM);

	printv(								\
	"|- Energy Policy%.*sBias Hint   [%7u]",
	width-37, hSpace, Shm->Cpu[0].PowerThermal.PowerPolicy);

	struct utsname OSinfo={{0}};
	uname(&OSinfo);
/* Section Mark */
	printv(	"%.*s", width, hSpace);

	printv(	"%s:" "%.*s",
		OSinfo.sysname, width-strlen(OSinfo.sysname)-1, hSpace);
	printv(	"|- Release%.*s[%s]",
		width-12 - strlen(OSinfo.release), hSpace, OSinfo.release);

    if((len=strlen(Shm->IdleDriver.Name)) > 0)
    {
	printv(	"|- Idle driver%.*s[%s]",
		width-16 - len, hSpace, Shm->IdleDriver.Name);
/* Row Mark */
	len=sprintf(row, "   |- States:%.*s",
		9, hSpace);
	for(i=0; i < Shm->IdleDriver.stateCount; i++) {
		len+=sprintf(str, "%-8s", Shm->IdleDriver.State[i].Name);
		strcat(row, str);
	}
	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);
/* Row Mark */
	len=sprintf(row, "   |- Power:%.*s",
		10, hSpace);
	for(i=0; i < Shm->IdleDriver.stateCount; i++) {
		len+=sprintf(str, "%-8d", Shm->IdleDriver.State[i].powerUsage);
		strcat(row, str);
	}
	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);
/* Row Mark */
	len=sprintf(row, "   |- Latency:%.*s",
		8, hSpace);
	for(i=0; i < Shm->IdleDriver.stateCount; i++) {
		len+=sprintf(str, "%-8u", Shm->IdleDriver.State[i].exitLatency);
		strcat(row, str);
	}
	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);
/* Row Mark */
	len=sprintf(row, "   |- Residency:%.*s",
		6, hSpace);
	for(i=0; i < Shm->IdleDriver.stateCount; i++) {
	    len+=sprintf(str, "%-8u", Shm->IdleDriver.State[i].targetResidency);
	    strcat(row, str);
	}
	pad=realloc(pad, (width-len) + 1);
	sprintf(pad, "%.*s", width-len, hSpace);
	strcat(row, pad);
	printv(row);
    }
	free(line);
	free(row);
	free(str);
	free(pad);
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
    unsigned short j=(unsigned short) Dec2Digit(thisNumber, thisDigit);

    thisView[0]='\0';
    j=4;
    do
    {
	short offset=col + (4 - j) * 3;

	sprintf(lcdBuf,
		"\033[%hu;%hdH" "%.*s"					\
		"\033[%hu;%hdH" "%.*s"					\
		"\033[%hu;%hdH" "%.*s",
		row	, offset, 3, lcd[thisDigit[9 - j]][0],
		row + 1	, offset, 3, lcd[thisDigit[9 - j]][1],
		row + 2	, offset, 3, lcd[thisDigit[9 - j]][2]);

	strcat(thisView, lcdBuf);
	j--;
    } while(j > 0) ;
}

void Dashboard(	SHM_STRUCT *Shm,
		unsigned short leadingLeft,
		unsigned short leadingTop,
		unsigned short marginWidth,
		unsigned short marginHeight)
{
    char *boardView=NULL,
	 *lcdView=NULL,
	 *cpuView=NULL,
	 *absMove=CUH CLS;
    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[9];
    double medianRatio=(minRatio + maxRatio) / 2;

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
	lcdView=malloc(lcdSize);
	const size_t cpuSize	= (9 * 3) + sizeof("\033[000;000H")
				+ (3 * sizeof(DoK)) + 1;
	cpuView=malloc(cpuSize);
	// All LCD views x total number of CPU
	boardView=malloc(Shm->Proc.CPU.Count * (lcdSize + cpuSize));
    }

    AllocAll();

    marginWidth+=12;	// shifted by lcd width
    marginHeight+=3+1;	// shifted by lcd height + cpu frame
    unsigned int cpu=0;

    while(!Shutdown)
    {
    	unsigned int digit[9];
	unsigned short X, Y;

	while(!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * BASE_SLEEP);
	BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);
	if(BITVAL(Shm->Proc.Sync, 63))
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);

	X=leadingLeft;
	Y=leadingTop;
	boardView[0]='\0';

	for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if(!Shm->Cpu[cpu].OffLine.HW)
	    {
		struct FLIP_FLOP *Flop=
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if(!Shm->Cpu[cpu].OffLine.OS)
		{
			LCD_Draw(X, Y, lcdView,
				(unsigned int) Flop->Relative.Freq, digit);
			sprintf(cpuView,
				"\033[%hu;%huH"DoK"[ µ%-2u"WoK"%4llu"DoK"C ]",
					Y + 3, X, cpu, Flop->Thermal.Temp);

			if(Flop->Relative.Ratio > medianRatio)
				strcat(boardView, RoK);
			else if(Flop->Relative.Ratio > minRatio)
				strcat(boardView, YoK);
			else
				strcat(boardView, GoK);
		}
		else
		{
			sprintf(lcdView, "\033[%hu;%huH" "_  _  _  _",
				Y + 1, X + 1, 3);
			sprintf(cpuView, "\033[%hu;%huH" "[ µ%-2u""  OFF ]",
				Y + 3, X, cpu);
		}
		X+=marginWidth;
		if((X - 3) >= (GetScreenSize().width - marginWidth))
		{
			X=leadingLeft;
			Y+=marginHeight;
			absMove=CUH;
		}
		else
			absMove=CUH CLS;

		strcat(boardView, lcdView);
		strcat(boardView, cpuView);
	    }
	if(printf("%s" "%s", absMove, boardView) > 0)
		fflush(stdout);
    }
    FreeAll();
}


void Counters(SHM_STRUCT *Shm)
{
    unsigned int cpu=0;
    while(!Shutdown)
    {
	while(!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * BASE_SLEEP);
	BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);
	if(BITVAL(Shm->Proc.Sync, 63))
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);

		printf("CPU Freq(MHz) Ratio  Turbo"			\
			"  C0(%%)  C1(%%)  C3(%%)  C6(%%)  C7(%%)"	\
			"  Min TMP:TS  Max\n");
	for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	  if(!Shm->Cpu[cpu].OffLine.HW)
	  {
	    struct FLIP_FLOP *Flop=
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if(!Shm->Cpu[cpu].OffLine.OS)
		printf("#%02u %7.2f (%5.2f)"				\
			" %6.2f %6.2f %6.2f %6.2f %6.2f %6.2f"		\
			"  %-3llu/%3llu:%-3llu/%3llu\n",
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
		"%.*s" "%3llu C\n\n",
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
	unsigned int cpu=0;
	while(!Shutdown)
	{
	  while(!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * BASE_SLEEP);
	  BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);
	  if(BITVAL(Shm->Proc.Sync, 63))
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);

		    printf("CPU     IPS            IPC            CPI\n");

	  for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if(!Shm->Cpu[cpu].OffLine.HW)
	    {
		struct FLIP_FLOP *Flop=
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if(!Shm->Cpu[cpu].OffLine.OS)
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
	unsigned int cpu=0, level=0, nl=6;
	char *line=malloc(13 + 1);

	void printv(char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf(line, fmt, ap);
		if(OutFunc == NULL)
			if(!--nl)
			{
				nl=6;
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
	for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
	{
		printv("%02u%-4s%6d ",
			cpu,
			(Shm->Cpu[cpu].Topology.MP.BSP) ? ":BSP" : ":AP",
			Shm->Cpu[cpu].Topology.ApicID);

		printv("%6d %6d",
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);

	    for(level=0; level < CACHE_MAX_LEVEL; level++)
	    {
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

typedef union
{
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
#define SCANKEY_SHIFT_TAB	0x5a5b1b
#define SCANKEY_PGUP		0x7e355b1b
#define SCANKEY_PGDW		0x7e365b1b
#define SCANKEY_SHIFT_q		0x51
#define SCANKEY_a		0x61
#define SCANKEY_c		0x63
#define SCANKEY_f		0x66
#define SCANKEY_h		0x68
#define SCANKEY_i		0x69
#define SCANKEY_l		0x6c
#define SCANKEY_m		0x6d
#define SCANKEY_s		0x73

int GetKey(SCANKEY *scan, struct timespec *tsec)
{
	struct pollfd fds={.fd=STDIN_FILENO, .events=POLLIN};
	int rp=0, rz=0;

	if((rp=ppoll(&fds, 1, tsec, NULL)) > 0)
		if(fds.revents == POLLIN)
		{
			size_t lc=fread(&scan->key, 1, 8, stdin);
			for(rz=lc; rz < 8; rz++)
				scan->code[rz]=0;
		}
	return(rp);
}

typedef struct
{
	unsigned short	col,
			row;
} Coordinate;

typedef struct
{
	signed short	horz,
			vert;
} CoordShift;

typedef struct
{
	unsigned short	wth,
			hth;
} CoordSize;

typedef union
{
	unsigned char	value;
	struct {
	unsigned char	fg:  3-0,
			un:  4-3,
			bg:  7-4,
			bf:  8-7;
	};
} Attribute;

#define MakeAttr(_fg, _un, _bg, _bf)					\
	({Attribute _attr={.fg=_fg,.un=_un,.bg=_bg,.bf=_bf}; _attr;})

#define HDK	{.fg=BLACK,	.bg=BLACK,	.bf=1}
#define HBK	{.fg=BLUE,	.bg=BLACK,	.bf=1}
#define HRK	{.fg=RED,	.bg=BLACK,	.bf=1}
#define HGK	{.fg=GREEN,	.bg=BLACK,	.bf=1}
#define HYK	{.fg=YELLOW,	.bg=BLACK,	.bf=1}
#define HWK	{.fg=WHITE,	.bg=BLACK,	.bf=1}
#define HKB	{.fg=BLACK,	.bg=BLUE,	.bf=1}
#define HWB	{.fg=WHITE,	.bg=BLUE,	.bf=1}
#define HKW	{.fg=BLACK,	.bg=WHITE,	.bf=1}
#define _HWK	{.fg=WHITE,	.bg=BLACK,	.un=1,	.bf=1}
#define _HWB	{.fg=WHITE,	.bg=BLUE,	.un=1,	.bf=1}
#define LKW	{.fg=BLACK,	.bg=WHITE}
#define LYK	{.fg=YELLOW,	.bg=BLACK}
#define LBK	{.fg=BLUE,	.bg=BLACK}
#define LBW	{.fg=BLUE,	.bg=WHITE}
#define LWK	{.fg=WHITE,	.bg=BLACK}
#define LWB	{.fg=WHITE,	.bg=BLUE}
#define _LKW	{.fg=BLACK,	.bg=WHITE,	.un=1}
#define _LBW	{.fg=BLUE,	.bg=WHITE,	.un=1}

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
	struct								\
	{								\
		Coordinate	origin;					\
		size_t		length;					\
		Attribute	attr[_len];				\
		ASCII		code[_len];				\
	}

typedef struct
{
	ASCII		*code;
	Attribute	*attr;
	CoordSize	size;
} Layer;

typedef struct
{
	CoordSize	size;
	Coordinate	origin,
			select;
	CoordShift	scroll;
} Matrix;

typedef struct
{
	SCANKEY 	quick;
	Attribute	*attr;
	ASCII		*item;
	size_t		length;
} TCell;

typedef struct _Win
{
	Layer		*layer;

	unsigned long long id;

	struct _Win	*prev,
			*next;
	struct
	{
		void	(*Print)(struct _Win *win, void *list);
	    struct
	    {
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
	    } key;

	    struct
	    {
	      Attribute	select,
			border,
			title;
	    } color[2];

		char	*title;
	} hook;

	Matrix		matrix;
	TCell		*cell;
	size_t		dim;

	struct
	{
		size_t	rowLen,
			titleLen;
	unsigned short	bottomRow;
	} lazyComp;
} Window;

typedef struct
{
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
	if((*with=realloc(*with, 1 + strlen(what))) != NULL)
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
#define SetHead(list, win)	GetHead(list)=win
#define SetDead(list)		SetHead(list, NULL)
#define IsHead(list, win)	(GetHead(list) == win)
#define IsDead(list)		(GetHead(list) == NULL)
#define IsCycling(win)		((win->next == win) && (win->prev == win))
#define GetFocus(list)		GetHead(list)

void DestroyLayer(Layer *layer)
{
	if(layer != NULL)
	{
		if(layer->attr != NULL)
		{
			free(layer->attr);
			layer->attr=NULL;
		}
		if(layer->code != NULL)
		{
			free(layer->code);
			layer->code=NULL;
		}
	}
}

void CreateLayer(Layer *layer, CoordSize size)
{
    if(layer != NULL)
    {
	layer->size.wth=size.wth;
	layer->size.hth=size.hth;
	size_t len=layer->size.wth * layer->size.hth;

	layer->attr=calloc(len, sizeof(Attribute));
	layer->code=calloc(len, sizeof(ASCII));
    }
}

#define ResetLayer(layer)						\
	memset(layer->attr, 0, layer->size.wth * layer->size.hth);	\
	memset(layer->code, 0, layer->size.wth * layer->size.hth);

void FreeAllTCells(Window *win)
{
	if(win->cell != NULL)
	{
		unsigned short i;
		for(i=0; i < win->dim; i++)
		{
			free(win->cell[i].attr);
			free(win->cell[i].item);
		}
		free(win->cell);
		win->cell=NULL;
	}
}

void AllocCopyAttr(TCell *cell, Attribute attrib[])
{
	if((attrib != NULL) && (cell->attr=malloc(cell->length)) != NULL)
		memcpy(&cell->attr->value, &attrib->value, cell->length);
}

void AllocFillAttr(TCell *cell, Attribute attrib)
{
	if((cell->attr=malloc(cell->length)) != NULL)
		memset(&cell->attr->value, attrib.value, cell->length);
}

void AllocCopyItem(TCell *cell, ASCII *item)
{
	if((cell->item=malloc(cell->length)) != NULL)
		strncpy(cell->item, item, cell->length);
}

#define StoreTCell(win, shortkey, item, attrib)				\
({									\
    if(item != NULL)							\
    {									\
	win->dim++ ;							\
	win->lazyComp.bottomRow = (win->dim / win->matrix.size.wth)	\
				- win->matrix.size.hth;			\
									\
      if((win->cell=realloc(win->cell,sizeof(TCell) * win->dim)) != NULL) \
      {									\
	win->cell[win->dim - 1].quick.key=shortkey;			\
	win->cell[win->dim - 1].length=strlen(item);			\
									\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		typeof(attrib), typeof(Attribute[])), AllocCopyAttr,	\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		typeof(attrib), typeof(Attribute)), AllocFillAttr,	\
	(void)0))							\
		(&(win->cell[win->dim - 1]), attrib);			\
									\
	AllocCopyItem(&win->cell[win->dim - 1], item);			\
      }									\
    }									\
})

void DestroyWindow(Window *win)
{
	if(win != NULL)
	{
		if(win->hook.title != NULL)
		{
			free(win->hook.title);
			win->hook.title=NULL;
			win->lazyComp.titleLen=0;
		}
		FreeAllTCells(win);
		free(win);
		win=NULL;
	}
}

Window *CreateWindow(	Layer *layer,
			unsigned long long id,
			unsigned short width,
			unsigned short height,
			unsigned short oCol,
			unsigned short oRow)
{
	Window *win=calloc(1, sizeof(Window));
	if(win != NULL)
	{
		win->layer=layer;
		win->id=id;
		win->matrix.size.wth=width;
		win->matrix.size.hth=height;
		win->matrix.origin.col=oCol;
		win->matrix.origin.row=oRow;

	    Attribute	select[2]=
			{
				MAKE_SELECT_UNFOCUS,
				MAKE_SELECT_FOCUS
			},
			border[2]=
			{
				MAKE_BORDER_UNFOCUS,
				MAKE_BORDER_FOCUS
			},
			title[2]=
			{
				MAKE_TITLE_UNFOCUS,
				MAKE_TITLE_FOCUS
			};
	    for(int i=0; i < 2; i++)
	    {
		win->hook.color[i].select=select[i];
		win->hook.color[i].border=border[i];
		win->hook.color[i].title=title[i];
	    }
	}
	return(win);
}

#define RemoveWinList(win, list)					\
({									\
	win->prev->next=win->next;					\
	win->next->prev=win->prev;					\
})

#define AppendWinList(win, list)					\
({									\
	win->prev=GetHead(list);					\
	win->next=GetHead(list)->next;					\
	GetHead(list)->next->prev=win;					\
	GetHead(list)->next=win;					\
})

void RemoveWindow(Window *win, WinList *list)
{
	RemoveWinList(win, list);

	if(IsCycling(GetHead(list)))
		SetDead(list);
	else if(IsHead(list, win))
	/*Auto shift*/	SetHead(list, win->next);

	DestroyWindow(win);
}

void AppendWindow(Window *win, WinList *list)
{
	if(!IsDead(list))
		AppendWinList(win, list);
	else
	{	// Dead head, now cycling
		win->prev=win;
		win->next=win;
	}
	SetHead(list, win);
}

void DestroyAllWindows(WinList *list)
{
	while(!IsDead(list))
		RemoveWindow(GetHead(list), list);
}

void AnimateWindow(int rotate, WinList *list)
{
    if(!IsDead(list))
	SetHead(list, rotate == 1 ? GetHead(list)->next : GetHead(list)->prev);
}

Window *SearchWinListById(unsigned long long id, WinList *list)
{
	Window *win=NULL;
	if(!IsDead(list))
	{
		Window *walker=GetHead(list);
		do
		{
			if(walker->id == id)
				win=walker;

			walker=walker->prev;
		} while(!IsHead(list, walker) && (win == NULL));
	}
	return(win);
}

void PrintContent(Window*win,WinList*list,unsigned short col,unsigned short row)
{
    if((win->matrix.select.col == col)
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
    else if(GetFocus(list) == win)
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
    else
    {
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
	Attribute border=win->hook.color[(GetFocus(list) == win)].border;

	if(win->lazyComp.rowLen == 0)
	  for(col=0, win->lazyComp.rowLen=2; col < win->matrix.size.wth; col++)
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;

	if(win->hook.title == NULL)
	    LayerFillAt(win->layer,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row - 1),
			win->lazyComp.rowLen, hLine, border);
	else
	{
	    if(win->lazyComp.titleLen == 0)
	    	win->lazyComp.titleLen=strlen(win->hook.title);

	    size_t halfLeft=(win->lazyComp.rowLen - win->lazyComp.titleLen) / 2;
	    size_t halfRight=halfLeft
			+ (win->lazyComp.rowLen - win->lazyComp.titleLen) % 2;

	    LayerFillAt(win->layer,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row - 1),
			halfLeft, hLine, border);

	    LayerFillAt(win->layer,
			(halfLeft + (win->matrix.origin.col - 1)),
			(win->matrix.origin.row - 1),
			win->lazyComp.titleLen, win->hook.title,
			((GetFocus(list) == win) ?
				win->hook.color[1].title
			:	win->hook.color[0].title));

	    LayerFillAt(win->layer,
			(halfLeft + win->lazyComp.titleLen
			+ (win->matrix.origin.col - 1)),
			(win->matrix.origin.row - 1),
			halfRight, hLine, border);
	}

	for(row=0; row < win->matrix.size.hth; row++)
	{
	    LayerAt(	win->layer, attr,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row + row))=border;
	    LayerAt(	win->layer, code,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row + row))=0x20;

	    for(col=0; col < win->matrix.size.wth; col++)
		PrintContent(win, list, col, row);

	    LayerAt(	win->layer, attr,
			(win->matrix.origin.col
			+ col * TCellAt(win, 0, 0).length),
			(win->matrix.origin.row + row))=border;
	    LayerAt(	win->layer, code,
			(win->matrix.origin.col
			+ col * TCellAt(win, 0, 0).length),
			(win->matrix.origin.row + row))=0x20;
	}
	LayerFillAt(	win->layer,
			(win->matrix.origin.col - 1),
			(win->matrix.origin.row + win->matrix.size.hth),
			win->lazyComp.rowLen, hLine, border);
}

void MotionReset_Win(Window *win)
{
	win->matrix.scroll.horz=win->matrix.select.col=0;
	win->matrix.scroll.vert=win->matrix.select.row=0;
}

void MotionLeft_Win(Window *win)
{
	if(win->matrix.select.col > 0)
		win->matrix.select.col-- ;
	else
		win->matrix.select.col=win->matrix.size.wth - 1;
}

void MotionRight_Win(Window *win)
{
	if(win->matrix.select.col < win->matrix.size.wth - 1)
		win->matrix.select.col++ ;
	else
		win->matrix.select.col=0;
}

void MotionUp_Win(Window *win)
{
	if(win->matrix.select.row > 0)
		win->matrix.select.row-- ;
	else
		win->matrix.select.row=win->matrix.size.hth - 1;
}

void MotionDown_Win(Window *win)
{
	if(win->matrix.select.row < win->matrix.size.hth - 1)
		win->matrix.select.row++ ;
	else
		win->matrix.select.row=0;
}

void MotionHome_Win(Window *win)
{
	win->matrix.select.col=0;
}

void MotionEnd_Win(Window *win)
{
	win->matrix.select.col=win->matrix.size.wth - 1;
}

void MotionPgUp_Win(Window *win)
{
	if(win->matrix.scroll.vert >= win->matrix.size.hth)
		win->matrix.scroll.vert-=win->matrix.size.hth;
	else
		win->matrix.scroll.vert=0;
}

void MotionPgDw_Win(Window *win)
{
    if(win->matrix.scroll.vert < win->lazyComp.bottomRow - win->matrix.size.hth)
	win->matrix.scroll.vert+=win->matrix.size.hth;
    else
	win->matrix.scroll.vert=win->lazyComp.bottomRow;
}

int Motion_Trigger(SCANKEY *scan, Window *win, WinList *list)
{
	switch(scan->key)
	{
	case SCANKEY_ESC:
	{
		Layer *thisLayer=win->layer;

		if(win->hook.key.Escape != NULL)
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
		AnimateWindow(0, list);
	break;
	case SCANKEY_LEFT:
		if(win->hook.key.Left != NULL)
			win->hook.key.Left(win);
	break;
	case SCANKEY_RIGHT:
		if(win->hook.key.Right != NULL)
			win->hook.key.Right(win);
	break;
	case SCANKEY_DOWN:
		if(win->hook.key.Down != NULL)
			win->hook.key.Down(win);
	break;
	case SCANKEY_UP:
		if(win->hook.key.Up != NULL)
			win->hook.key.Up(win);
	break;
	case SCANKEY_HOME:
		if(win->hook.key.Home != NULL)
			win->hook.key.Home(win);
	break;
	case SCANKEY_END:
		if(win->hook.key.End != NULL)
			win->hook.key.End(win);
	break;
	case SCANKEY_PGUP:
		if(win->hook.key.PgUp != NULL)
			win->hook.key.PgUp(win);
	break;
	case SCANKEY_PGDW:
		if(win->hook.key.PgDw != NULL)
		win->hook.key.PgDw(win);
	break;
	case SCANKEY_ENTER:
		if(win->hook.key.Enter != NULL)
			return(win->hook.key.Enter(scan, win));
	// fallthrough
	default:
	return(-1);
	}
	return(0);
}

enum {L_STATIC, L_DYNAMIC, L_WINDOW, LAYERS};

#define LOAD_LEAD	4

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

    Layer	*sLayer=NULL,
		*dLayer=NULL,
		*wLayer=NULL,
		*fuze=NULL;
    WinList	winList={.head=NULL};

    struct
    {
	unsigned long long
		layout	:  1-0,	 // Draw layout
		clear	:  2-1,	 // Clear screen
		height	:  3-2,	 // Valid height
		width	:  4-3,	 // Valid width
		view	:  6-4,  // 0=Freq, 1=IPS, 2=COUNTERS
		_pad1	: 32-6,
		daemon	: 33-32, // Draw dynamic
		_pad2	: 64-33;
    } drawFlag={.layout=0,.clear=0,.height=0,.width=0,.view=0,.daemon=0};

    SCREEN_SIZE drawSize={.width=0, .height=0};

    unsigned int timeout=Shm->Proc.SleepInterval * BASE_SLEEP;
    struct timespec tsec={
	.tv_sec=timeout / 1000000L,
	.tv_nsec=(timeout % 1000000L) * 1000
    };

    unsigned int topRatio, digit[9], lcdColor, cpu=0, iClock=0, ratioCount=0, i,
		mSteps=(timeout < 100000L) ? (100000L / timeout) : 1,
		tSteps=0;

    int MIN_HEIGHT=(2 * Shm->Proc.CPU.Count)
		 + TOP_HEADER_ROW + TOP_SEPARATOR + TOP_FOOTER_ROW,
	loadWidth=0;

    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[9],
	medianRatio=(minRatio + maxRatio) / 2,
	availRatio[10]={minRatio}, maxRelFreq;

    const unsigned int
	isTurboBoost=(Shm->Proc.TurboBoost == Shm->Proc.TurboBoost_Mask),
	isSpeedStep=(Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask),
	isEnhancedHaltState=(Shm->Proc.C1E == Shm->Proc.C1E_Mask),
	isC3autoDemotion=(Shm->Proc.C3A == Shm->Proc.C3A_Mask),
	isC1autoDemotion=(Shm->Proc.C1A == Shm->Proc.C1A_Mask),
	isC3undemotion=(Shm->Proc.C3U == Shm->Proc.C3U_Mask),
	isC1undemotion=(Shm->Proc.C1U == Shm->Proc.C1U_Mask);

    char *buffer=NULL, *viewMask=NULL;

    typedef char HBCLK[11 + 1];
    HBCLK *hBClk;

    void ForEachCellPrint_Menu(Window *win, void *plist)
    {
	WinList *list=(WinList *) plist;
	unsigned short col, row;
	size_t len;

	if(win->lazyComp.rowLen == 0)
	  for(col=0; col < win->matrix.size.wth; col++)
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;

	if(win->matrix.origin.col > 0)
		LayerFillAt(win->layer,
				0,
				win->matrix.origin.row,
				win->matrix.origin.col, hSpace,
				win->hook.color[0].title);

	for(col=0; col < win->matrix.size.wth; col++)
		PrintContent(win, list, col, 0);
	for(row=1; row < win->matrix.size.hth; row++)
	    if(TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			PrintContent(win, list, win->matrix.select.col, row);

	if((len=drawSize.width - win->lazyComp.rowLen) > 0)
		LayerFillAt(win->layer,
				win->matrix.origin.col + win->lazyComp.rowLen,
				win->matrix.origin.row,
				len, hSpace,
				win->hook.color[0].title);
    }

    int MotionEnter_Menu(SCANKEY *scan, Window *win)
    {
	if((scan->key=TCellAt(win,
			win->matrix.select.col,
			win->matrix.select.row).quick.key) != SCANKEY_NULL)
	{
		SCANKEY closeKey={.key=SCANKEY_ESC};
		Motion_Trigger(&closeKey, win, &winList);
		return(1);
	}
	else
		return(0);
    }

    #define ResetTCell_Menu(win)					\
    (									\
	{								\
	    CoordShift shift={						\
		.horz=win->matrix.scroll.horz + win->matrix.select.col,	\
		.vert=win->matrix.scroll.vert + row			\
	    };								\
	    Coordinate cell={						\
		.col=	(win->matrix.origin.col				\
			+ (win->matrix.select.col			\
			* TCellAt(win, shift.horz, shift.vert).length)), \
			(win->matrix.origin.row + row),			\
		.row=	win->matrix.origin.row + row			\
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
	for(unsigned short row=1; row < win->matrix.size.hth; row++)
		ResetTCell_Menu(win);

	if(win->matrix.select.col > 0)
		win->matrix.select.col-- ;
	else
		win->matrix.select.col=win->matrix.size.wth - 1;

	win->matrix.select.row=0;
    }

    void MotionRight_Menu(Window *win)
    {
	for(unsigned short row=1; row < win->matrix.size.hth; row++)
		ResetTCell_Menu(win);

	if(win->matrix.select.col < win->matrix.size.wth - 1)
		win->matrix.select.col++ ;
	else
		win->matrix.select.col=0;

	win->matrix.select.row=0;
    }

    void MotionUp_Menu(Window *win)
    {
	unsigned short row=win->matrix.select.row;

	if(win->matrix.select.row > 0)
		row-- ;

	if(TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			win->matrix.select.row=row;
    }

    void MotionDown_Menu(Window *win)
    {
	unsigned short row=win->matrix.select.row;

	if(row < win->matrix.size.hth - 1)
		row++ ;

	if(TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			win->matrix.select.row=row;
    }

    void MotionHome_Menu(Window *win)
    {
	if(TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + 1)).quick.key != SCANKEY_VOID)
			win->matrix.select.row=1;
	else
			win->matrix.select.row=0;
    }

    void MotionEnd_Menu(Window *win)
    {
	unsigned short row=0;

	for(row=win->matrix.size.hth - 1; row > 1; row--)
	    if(TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
			break;

	win->matrix.select.row=row;
    }

    Window *CreateMenu(unsigned long long id)
    {
	Window *wMenu=CreateWindow(wLayer, id, 3, 5, 3, 0);
	if(wMenu != NULL)
	{
		Attribute sameAttr={.fg=BLACK, .bg=WHITE, .bf=0},
			voidAttr={.value=0},
			helpAttr[18]={
				LKW,LKW,LKW,LKW,LKW,LKW,_LKW,LKW,LKW,
				LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW
			},
			skeyAttr[18]={
				LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,
				LKW,LKW,LKW,LKW,LKW,HKW,_LKW,HKW,LKW
			};

		StoreTCell(wMenu, SCANKEY_h,	"      Help        ", helpAttr);
		StoreTCell(wMenu, SCANKEY_NULL,	"      Layer       ", sameAttr);
		StoreTCell(wMenu, SCANKEY_NULL,	"      Window      ", sameAttr);

		StoreTCell(wMenu, SCANKEY_a,	" About        [a] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_f,	" Frequency    [f] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_s,	" System info  [s] ", skeyAttr);

		StoreTCell(wMenu,SCANKEY_SHIFT_q," Quit         [Q] ",skeyAttr);
		StoreTCell(wMenu, SCANKEY_i,	" Inst cycles  [i] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_m,	" Topology     [m] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SCANKEY_c,	" Core cycles  [c] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SCANKEY_l,	" Idle states  [l] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);

		StoreWindow(wMenu,	.Print,		ForEachCellPrint_Menu);
		StoreWindow(wMenu,	.key.Enter,	MotionEnter_Menu);
		StoreWindow(wMenu,	.key.Left,	MotionLeft_Menu);
		StoreWindow(wMenu,	.key.Right,	MotionRight_Menu);
		StoreWindow(wMenu,	.key.Down,	MotionDown_Menu);
		StoreWindow(wMenu,	.key.Up,	MotionUp_Menu);
		StoreWindow(wMenu,	.key.Home,	MotionHome_Menu);
		StoreWindow(wMenu,	.key.End,	MotionEnd_Menu);
		StoreWindow(wMenu, .color[0].select, MakeAttr(BLACK,0,WHITE,0));
		StoreWindow(wMenu, .color[0].title, MakeAttr(BLACK,0,WHITE,0));
		StoreWindow(wMenu, .color[1].title, MakeAttr(BLACK,0,WHITE,1));
	}
	return(wMenu);
    }

    Window *CreateHelp(unsigned long long id)
    {
      Window *wHelp=CreateWindow(wLayer, id, 2, 14, 2, TOP_HEADER_ROW + 2);
      if(wHelp != NULL)
      {
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [F1]             ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "             Menu ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Escape]         ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "     Close window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Shift]+[Tab]    ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "  Previous window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Tab]            ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "      Next window ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Enter]          ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "Trigger selection ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       [Up]       ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Left]    [Right]", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "   Move selection ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "      [Down]      ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Home]           ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "       First cell ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [End]            ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "        Last cell ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Page-Up]        ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "    Previous page ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, " [Page-Dw]        ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "        Next page ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);
	StoreTCell(wHelp, SCANKEY_NULL, "                  ", MAKE_PRINT_FOCUS);

	StoreWindow(wHelp, .title, " Help ");
	StoreWindow(wHelp, .color[0].select, MAKE_PRINT_UNFOCUS);
	StoreWindow(wHelp, .color[1].select, MAKE_PRINT_FOCUS);
      }
      return(wHelp);
    }

    Window *CreateAbout(unsigned long long id)
    {
      char *C[]={
	"   ""   ______                ______               ""   ",
	"   ""  / ____/___  ________  / ____/_______  ____ _""   ",
	"   "" / /   / __ \\/ ___/ _ \\/ /_  / ___/ _ \\/ __ `/""   ",
	"   ""/ /___/ /_/ / /  /  __/ __/ / /  /  __/ /_/ / ""   ",
	"   ""\\____/\\____/_/   \\___/_/   /_/   \\___/\\__, /  ""   ",
	"   ""                                        /_/   ""   "
      };
      char *F[]={
	"   ""   by CyrIng                                  ""   ",
	"   ""                                              ""   ",
	"   ""         (C)2015-2016 CYRIL INGENIERIE        ""   ",
      };
	size_t c=sizeof(C)/sizeof(C[0]),f=sizeof(F)/sizeof(F[0]),l=strlen(C[0]);

	Window *wAbout=CreateWindow(	wLayer,
					id,
					1, c + f,
					(drawSize.width - l) / 2,
					TOP_HEADER_ROW + 4);
	if(wAbout != NULL)
	{
		unsigned int i;

		for(i=0; i < c; i++)
			StoreTCell(wAbout,SCANKEY_NULL, C[i], MAKE_PRINT_FOCUS);
		for(i=0; i < f; i++)
			StoreTCell(wAbout,SCANKEY_NULL, F[i], MAKE_PRINT_FOCUS);

		wAbout->matrix.select.row=wAbout->matrix.size.hth - 1;

		StoreWindow(wAbout, .title, " CoreFreq ");
		StoreWindow(wAbout, .color[0].select, MAKE_PRINT_UNFOCUS);
		StoreWindow(wAbout, .color[1].select, MAKE_PRINT_FOCUS);
	}
	return(wAbout);
    }

    void MotionEnd_SysInfo(Window *win)
    {
	win->matrix.scroll.vert=win->lazyComp.bottomRow;
	win->matrix.select.row=win->matrix.size.hth - 1;
    }

    Window *CreateSysInfo(unsigned long long id)
    {
	Window *wSysInfo=CreateWindow(	wLayer,
					id,
					1,
					18,
					3,
					TOP_HEADER_ROW);

	void AddSysInfoCell(char *input)
	{
		StoreTCell(wSysInfo, SCANKEY_NULL, input, MAKE_PRINT_FOCUS);
	}

	if(wSysInfo != NULL)
	{
		StoreWindow(wSysInfo,	.key.Left,	MotionLeft_Win);
		StoreWindow(wSysInfo,	.key.Right,	MotionRight_Win);
		StoreWindow(wSysInfo,	.key.Down,	MotionDown_Win);
		StoreWindow(wSysInfo,	.key.Up,	MotionUp_Win);
		StoreWindow(wSysInfo,	.key.PgUp,	MotionPgUp_Win);
		StoreWindow(wSysInfo,	.key.PgDw,	MotionPgDw_Win);
		StoreWindow(wSysInfo,	.key.Home,	MotionReset_Win);
		StoreWindow(wSysInfo,	.key.End,	MotionEnd_SysInfo);
		StoreWindow(wSysInfo,	.title,	" System Information ");
		SysInfo(Shm, drawSize.width - 6, AddSysInfoCell);
	}
	return(wSysInfo);
    }

    Window *CreateTopology(unsigned long long id)
    {
	Window *wTopology=CreateWindow(	wLayer,
					id,
					6,
					2 + Shm->Proc.CPU.Count,
					1,
					TOP_HEADER_ROW + 3);

	void AddTopologyCell(char *input)
	{
		StoreTCell(wTopology, SCANKEY_NULL, input, MAKE_PRINT_FOCUS);
	}

	if(wTopology != NULL)
	{
		StoreWindow(wTopology,	.key.Left,	MotionLeft_Win);
		StoreWindow(wTopology,	.key.Right,	MotionRight_Win);
		StoreWindow(wTopology,	.key.Down,	MotionDown_Win);
		StoreWindow(wTopology,	.key.Up,	MotionUp_Win);
		StoreWindow(wTopology,	.key.Home,	MotionHome_Win);
		StoreWindow(wTopology,	.key.End,	MotionEnd_Win);
		StoreWindow(wTopology,	.title, " Topology ");
		Topology(Shm, AddTopologyCell);
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
	hBClk=calloc(Shm->Proc.CPU.Count, sizeof(HBCLK));
	buffer=malloc(4 * MAX_WIDTH);
	viewMask=malloc((4 * MAX_WIDTH) * MAX_HEIGHT);

	const CoordSize layerSize={
		.wth=MAX_WIDTH,
		.hth=MAX_HEIGHT
	};

	sLayer=calloc(1, sizeof(Layer));
	dLayer=calloc(1, sizeof(Layer));
	wLayer=calloc(1, sizeof(Layer));
	fuze=calloc(1, sizeof(Layer));
	CreateLayer(sLayer, layerSize);
	CreateLayer(dLayer, layerSize);
	CreateLayer(wLayer, layerSize);
	CreateLayer(fuze, layerSize);
    }

    for(i=1; i < 10; i++)
	if(Shm->Proc.Boost[i] != 0)
	{
		int sort=Shm->Proc.Boost[i] - availRatio[ratioCount];
		if(sort < 0)
		{
			availRatio[ratioCount + 1]=availRatio[ratioCount];
			availRatio[ratioCount++]=Shm->Proc.Boost[i];
		}
		else if(sort > 0)
			availRatio[++ratioCount]=Shm->Proc.Boost[i];
	}
    ratioCount++;

    void TrapScreenSize(int caught)
    {
	if(caught == SIGWINCH)
	{
		SCREEN_SIZE currentSize=GetScreenSize();
		if(currentSize.height != drawSize.height)
		{
			if(currentSize.height > MAX_HEIGHT)
				drawSize.height=MAX_HEIGHT;
			else
				drawSize.height=currentSize.height;

			drawFlag.clear=1;
			drawFlag.height=!(drawSize.height < MIN_HEIGHT);
		}
		if(currentSize.width != drawSize.width)
		{
			if(currentSize.width > MAX_WIDTH)
				drawSize.width=MAX_WIDTH;
			else
				drawSize.width=currentSize.width;

			drawFlag.clear=1;
			drawFlag.width=!(drawSize.width < MIN_WIDTH);
		}
	}
    }

    int Shortcut(SCANKEY *scan)
    {
	switch(scan->key)
	{
	case SCANKEY_F1:
	{
		Window *win=SearchWinListById(scan->key, &winList);
		if(win == NULL)
			AppendWindow(CreateMenu(SCANKEY_F1), &winList);
		else
			SetHead(&winList, win);
	}
	break;
	case SCANKEY_a:
	{
		Window *win=SearchWinListById(scan->key, &winList);
		if(win == NULL)
			AppendWindow(CreateAbout(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
	break;
	case SCANKEY_c:
	{
		drawFlag.view=2;
		drawFlag.clear=1;
	}
	break;
	case SCANKEY_f:
	{
		drawFlag.view=0;
		drawFlag.clear=1;
	}
	break;
	case SCANKEY_h:
	{
		Window *win=SearchWinListById(scan->key, &winList);
		if(win == NULL)
			AppendWindow(CreateHelp(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
	break;
	case SCANKEY_i:
	{
		drawFlag.view=1;
		drawFlag.clear=1;
	}
	break;
	case SCANKEY_l:
	{
		drawFlag.view=3;
		drawFlag.clear=1;
	}
	break;
	case SCANKEY_m:
	{
		Window *win=SearchWinListById(scan->key, &winList);
		if(win == NULL)
			AppendWindow(CreateTopology(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
	break;
	case SCANKEY_SHIFT_q:
		Shutdown=0x1;
	break;
	case SCANKEY_s:
	{
		Window *win=SearchWinListById(scan->key, &winList);
		if(win == NULL)
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
	unsigned short col=0, row=0;
	loadWidth=drawSize.width - LOAD_LEAD;

	LayerDeclare(12) hProc0=
	{
		.origin={.col=12, .row=row}, .length=12,
		.attr={HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK},
		.code={' ','P','r','o','c','e','s','s','o','r',' ','['},
	};

	LayerDeclare(9) hProc1=
	{
		.origin={.col=drawSize.width - 9, .row=row}, .length=9,
		.attr={HDK,HWK,HWK,HDK,HWK,HWK,LWK,LWK,LWK},
		.code={']',' ',' ','/',' ',' ','C','P','U'},
	};

	row++ ;

	LayerDeclare(15) hArch0=
	{
	    .origin={.col=12, .row=row}, .length=15,
	    .attr={HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK},
	    .code={' ','A','r','c','h','i','t','e','c','t','u','r','e',' ','['},
	};

	LayerDeclare(30) hArch1=
	{
		.origin={.col=drawSize.width - 30, .row=row}, .length=30,
		.attr={	HDK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,		\
			LWK,LWK,HDK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,	\
			LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HDK,HDK
		},
		.code={	']',' ','C','a','c','h','e','s',' ',		\
			'L','1',' ','I','n','s','t','=',' ',' ',' ',	\
			'D','a','t','a','=',' ',' ',' ','K','B'
		},
	};

	row++ ;

	LayerDeclare(28) hBClk0=
	{
		.origin={.col=12, .row=row}, .length=28,
		.attr={	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
			HDK,HDK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,\
			HDK,HDK,HDK
			},
		.code={	' ','B','a','s','e',' ','C','l','o','c','k',' ',\
			'~',' ','0','0','0',' ','0','0','0',' ','0','0','0',\
			' ','H','z'
		},
	};

	LayerDeclare(18) hBClk1=
	{
		.origin={.col=drawSize.width - 18, .row=row}, .length=18,
		.attr={	LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,		\
			LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HDK,HDK
		},
		.code={	'L','2','=',' ',' ',' ',' ',' ',		\
			'L','3','=',' ',' ',' ',' ',' ','K','B'
		},
	};

	row++ ;

	sprintf(buffer, "%2u" "%-2u",
		Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	hProc1.code[1]=buffer[0];
	hProc1.code[2]=buffer[1];
	hProc1.code[4]=buffer[2];
	hProc1.code[5]=buffer[3];

	unsigned int L1I_Size=0, L1D_Size=0, L2U_Size=0, L3U_Size=0;
	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12))
	{
		L1I_Size=Shm->Cpu[0].Topology.Cache[0].Size / 1024;
		L1D_Size=Shm->Cpu[0].Topology.Cache[1].Size / 1024;
		L2U_Size=Shm->Cpu[0].Topology.Cache[2].Size / 1024;
		L3U_Size=Shm->Cpu[0].Topology.Cache[3].Size / 1024;
	}
  else	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12))
	{
		L1I_Size=Shm->Cpu[0].Topology.Cache[0].Size;
		L1D_Size=Shm->Cpu[0].Topology.Cache[1].Size;
		L2U_Size=Shm->Cpu[0].Topology.Cache[2].Size;
		L3U_Size=Shm->Cpu[0].Topology.Cache[3].Size;
	}
	sprintf(buffer, "%-3u" "%-3u", L1I_Size, L1D_Size);

	hArch1.code[17]=buffer[0];
	hArch1.code[18]=buffer[1];
	hArch1.code[19]=buffer[2];
	hArch1.code[25]=buffer[3];
	hArch1.code[26]=buffer[4];
	hArch1.code[27]=buffer[5];

	sprintf(buffer, "%-4u" "%-5u", L2U_Size, L3U_Size);

	hBClk1.code[ 3]=buffer[0];
	hBClk1.code[ 4]=buffer[1];
	hBClk1.code[ 5]=buffer[2];
	hBClk1.code[ 6]=buffer[3];
	hBClk1.code[11]=buffer[4];
	hBClk1.code[12]=buffer[5];
	hBClk1.code[13]=buffer[6];
	hBClk1.code[14]=buffer[7];
	hBClk1.code[15]=buffer[8];

	LayerDeclare(MAX_WIDTH) hLoad0=
	{
		.origin={.col=0, .row=row}, .length=drawSize.width,
		.attr={	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK
		},
		.code={	"------------- CPU Ratio ----------------"	\
			"----------------------------------------"	\
			"----------------------------------------"	\
			"------------"
		},
	};

	for(i=0; i < ratioCount; i++)
	{
		char tabStop[]="00";
		int hPos=availRatio[i] * loadWidth / maxRatio;
		sprintf(tabStop, "%2.0f", availRatio[i]);

		hLoad0.code[hPos + 2]=tabStop[0];
		hLoad0.code[hPos + 3]=tabStop[1];
		hLoad0.attr[hPos + 2]=					\
		hLoad0.attr[hPos + 3]=MakeAttr(CYAN, 0, BLACK, 0);
	}
	len=strlen(Shm->Proc.Brand);

	LayerCopyAt(layer, hProc0.origin.col, hProc0.origin.row,
			hProc0.length, hProc0.attr, hProc0.code);

	LayerFillAt(layer, hProc0.origin.col + hProc0.length, hProc0.origin.row,
			len, Shm->Proc.Brand,
			MakeAttr(CYAN, 0, BLACK, 1));

	if((hProc1.origin.col - len) > 0)
	    LayerFillAt(layer, hProc0.origin.col + hProc0.length + len,
			hProc0.origin.row,
			hProc1.origin.col - len, hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hProc1.origin.col, hProc1.origin.row,
			hProc1.length, hProc1.attr, hProc1.code);

	len=strlen(Shm->Proc.Architecture);

	LayerCopyAt(layer, hArch0.origin.col, hArch0.origin.row,
			hArch0.length, hArch0.attr, hArch0.code);

	LayerFillAt(layer, hArch0.origin.col + hArch0.length, hArch0.origin.row,
			len, Shm->Proc.Architecture,
			MakeAttr(CYAN, 0, BLACK, 1));

	if((hArch1.origin.col - len) > 0)
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

    for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
    {
	row++ ;
	sprintf(buffer, "%-2u", cpu);

	LayerAt(layer, attr, 0, row)=					\
		LayerAt(layer, attr, 0, (1 + row + Shm->Proc.CPU.Count))= \
			MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, 0, row)=					\
		LayerAt(layer, code, 0, (1 + row + Shm->Proc.CPU.Count))='#';
	LayerAt(layer, code, 1, row)=					\
		LayerAt(layer, code, 1, (1 + row + Shm->Proc.CPU.Count))= \
			buffer[0];
	LayerAt(layer, code, 2, row)=					\
		LayerAt(layer, code, 2, (1 + row + Shm->Proc.CPU.Count))= \
			buffer[1];
	LayerAt(layer, attr, 3, row)=MakeAttr(YELLOW, 0, BLACK, 1);
	LayerAt(layer, code, 3, row)=0x20;

      if(!Shm->Cpu[cpu].OffLine.OS)
      {
	LayerAt(layer, attr, 1, row)=					\
		LayerAt(layer, attr, 1, (1 + row + Shm->Proc.CPU.Count))= \
			MakeAttr(WHITE, 0, BLACK, 0);
	LayerAt(layer, attr, 2, row)=					\
		LayerAt(layer, attr, 2, (1 + row + Shm->Proc.CPU.Count))= \
			MakeAttr(WHITE, 0, BLACK, 0);
      }
      else
      {
	LayerAt(layer, attr, 1, row)=					\
		LayerAt(layer, attr, 1, (1 + row + Shm->Proc.CPU.Count))= \
			MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, attr, 2, row)=					\
		LayerAt(layer, attr, 2, (1 + row + Shm->Proc.CPU.Count))= \
			MakeAttr(BLACK, 0, BLACK, 1);
      }

	switch(drawFlag.view)
	{
	  case 0:
	  {
	    LayerDeclare(77) hMon0=
	    {
		.origin={.col=LOAD_LEAD - 1,
			.row=(row + Shm->Proc.CPU.Count + 1)},
			.length=77,
		.attr={	HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,		\
			HDK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,HDK,		\
			HBK,HBK,HBK,HDK,				\
			LWK,LWK,LWK,HDK,				\
			LYK,LYK,LYK					\
		},
		.code={	0x0,						\
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
		},
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
	  case 1:
	  {
	    LayerDeclare(76) hMon0=
	    {
		.origin={.col=LOAD_LEAD - 1,
			.row=(row + Shm->Proc.CPU.Count + 1)},
			.length=76,
		.attr={	HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
			HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code={	0x0,						\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,	\
			' ',' ',' ',' ',' ',' ',0x0,0x0,		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,	\
			' ',' ',' ',' ',' ',' ',0x0,0x0,		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,	\
			' ',' ',' ',' ',' ',' ',0x0,0x0,		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' '
		},
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
	  case 2:
	  case 3:
	  {
	    LayerDeclare(73) hMon0=
	    {
		.origin={.col=LOAD_LEAD - 1,
			.row=(row + Shm->Proc.CPU.Count + 1)},
			.length=73,
		.attr={	HYK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code={	0x0,						\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' ',		\
			' ',' ',' ',' ',' ',' ',' ',' ',' '
		},
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
	}

	i=Dec2Digit(Shm->Cpu[cpu].Clock.Hz, digit);
	sprintf(hBClk[cpu],
		"%u%u%u %u%u%u %u%u%u",
		digit[0], digit[1], digit[2],
		digit[3], digit[4], digit[5],
		digit[6], digit[7], digit[8]);
    }

	row++ ;

	switch(drawFlag.view)
	{
	  case 0:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"--- Freq(MHz) Ratio - Turbo --- "			\
		"C0 ---- C1 ---- C3 ---- C6 ---- C7 --"			\
		"Min TMP Max "						\
		"---------------------------------------------------",
		MakeAttr(BLACK, 0, BLACK, 1));

	    LayerDeclare(70) hAvg0=
	    {
		.origin={.col=0,.row=(row + Shm->Proc.CPU.Count +1)},.length=70,
		.attr={	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,		\
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,		\
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,		\
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,		\
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,		\
			HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK
		},
		.code={	'-','-','-','-','-','-','-','-',		\
			' ','A','v','e','r','a','g','e','s',' ','[',	\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,		\
			' ',' ',' ',' ',0x0,' ',' ',0x0,' ',']',' '
		},
	    };

	    LayerCopyAt(layer, hAvg0.origin.col, hAvg0.origin.row,
			hAvg0.length, hAvg0.attr, hAvg0.code);

	    LayerFillAt(layer, hAvg0.length, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width - hAvg0.length, hLine,
			MakeAttr(BLACK, 0, BLACK, 1));
	  }
	  break;
	  case 1:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"------------ IPS -------------- IPC ----"		\
		"---------- CPI ------------------ INST -"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(BLACK, 0, BLACK, 1));

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine,
			MakeAttr(BLACK, 0, BLACK, 1));
	  }
	  break;
	  case 2:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"-------------- C0:UCC ---------- C0:URC "		\
		"------------ C1 ------------- TSC ------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(BLACK, 0, BLACK, 1));

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(BLACK, 0, BLACK, 1));
	  }
	  break;
	  case 3:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"---------------- C1 -------------- C3 --"		\
		"------------ C6 -------------- C7 ------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(BLACK, 0, BLACK, 1));

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(BLACK, 0, BLACK, 1));
	  }
	  break;
	}

	row += Shm->Proc.CPU.Count + 2;

	LayerDeclare(61) hTech0=
	{
		.origin={.col=0, .row=row}, .length=14,
		.attr={LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK},
		.code={'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',','},
	};

	const Attribute Pwr[]=
	{
		MakeAttr(BLACK, 0, BLACK, 1),
		MakeAttr(GREEN, 0, BLACK, 1)
	};
	const struct { ASCII *code; Attribute attr; } TSC[]=
	{
		"  TSC  ",  MakeAttr(BLACK, 0, BLACK, 1),
	    	"TSC-VAR" , MakeAttr(BLUE,  0, BLACK, 0),
	    	"TSC-INV" , MakeAttr(GREEN, 0, BLACK, 1)
	};

	hTech0.code[ 6]=TSC[Shm->Proc.InvariantTSC].code[0];
	hTech0.code[ 7]=TSC[Shm->Proc.InvariantTSC].code[1];
	hTech0.code[ 8]=TSC[Shm->Proc.InvariantTSC].code[2];
	hTech0.code[ 9]=TSC[Shm->Proc.InvariantTSC].code[3];
	hTech0.code[10]=TSC[Shm->Proc.InvariantTSC].code[4];
	hTech0.code[11]=TSC[Shm->Proc.InvariantTSC].code[5];
	hTech0.code[12]=TSC[Shm->Proc.InvariantTSC].code[6];

	hTech0.attr[ 6]=hTech0.attr[ 7]=hTech0.attr[ 8]=		\
	hTech0.attr[ 9]=hTech0.attr[10]=hTech0.attr[11]=		\
	hTech0.attr[12]=TSC[Shm->Proc.InvariantTSC].attr;

	LayerCopyAt(layer, hTech0.origin.col, hTech0.origin.row,
			hTech0.length, hTech0.attr, hTech0.code);

	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12))
	{
	    LayerDeclare(61) hTech1=
	    {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=47,
		.attr={	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK
		},
		.code={	'H','T','T',',','E','I','S','T',',',		\
			'T','U','R','B','O',',','C','1','E',',',	\
			' ','P','M',',','C','3','A',',','C','1','A',',',\
			'C','3','U',',','C','1','U',',',		\
			'T','M','1',',','T','M','2',']'
		},
	    };

	    hTech1.attr[0]=hTech1.attr[1]=hTech1.attr[2]=		\
		Pwr[Shm->Proc.HyperThreading];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[19]=buffer[0];
	    hTech1.code[20]=buffer[1];
	    hTech1.code[21]=buffer[2];

	    hTech1.attr[19]=hTech1.attr[20]=hTech1.attr[21]=		\
		Pwr[(Shm->Proc.PM_version > 0)];

		const Attribute TM1[]=
		{
			MakeAttr(BLACK, 0, BLACK, 1),
			MakeAttr(BLUE,  0, BLACK, 0),
			MakeAttr(WHITE, 0, BLACK, 0),
			MakeAttr(GREEN, 0, BLACK, 1)
		};
		const Attribute TM2[]=
		{
			MakeAttr(BLACK, 0, BLACK, 1),
			MakeAttr(BLUE,  0, BLACK, 0),
			MakeAttr(WHITE, 0, BLACK, 0),
			MakeAttr(GREEN, 0, BLACK, 1)
		};

	    hTech1.attr[4]=hTech1.attr[5]=hTech1.attr[6]=hTech1.attr[7]=\
		Pwr[isSpeedStep];

	    hTech1.attr[ 9]=hTech1.attr[10]=hTech1.attr[11]=		\
	    hTech1.attr[12]=hTech1.attr[13]=Pwr[isTurboBoost];

	    hTech1.attr[15]=hTech1.attr[16]=hTech1.attr[17]=		\
		Pwr[isEnhancedHaltState];

	    hTech1.attr[23]=hTech1.attr[24]=hTech1.attr[25]=		\
		Pwr[isC3autoDemotion];

	    hTech1.attr[27]=hTech1.attr[28]=hTech1.attr[29]=		\
		Pwr[isC1autoDemotion];

	    hTech1.attr[31]=hTech1.attr[32]=hTech1.attr[33]=		\
		Pwr[isC3undemotion];

	    hTech1.attr[35]=hTech1.attr[36]=hTech1.attr[37]=		\
		Pwr[isC1undemotion];

	    hTech1.attr[39]=hTech1.attr[40]=hTech1.attr[41]=		\
		TM1[Shm->Cpu[0].PowerThermal.TM1];

	    hTech1.attr[43]=hTech1.attr[44]=hTech1.attr[45]=		\
		TM2[Shm->Cpu[0].PowerThermal.TM2];

	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
				hTech1.origin.row,
				(drawSize.width-hTech0.length-hTech1.length),
				hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}
  else	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12))
	{
	    LayerDeclare(61) hTech1=
	    {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=35,
		.attr={	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
			HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK
		},
		.code={	'H','T','T',',','P','o','w','e','r','N','o','w',',',\
			'T','U','R','B','O',',','C','1','E',',',	\
			' ','P','M',',','D','T','S',',','T','T','P',']'
		},
	    };

	    hTech1.attr[0]=hTech1.attr[1]=hTech1.attr[2]=		\
		Pwr[Shm->Proc.HyperThreading];

	    hTech1.attr[4]=hTech1.attr[5]=hTech1.attr[ 6]=hTech1.attr[ 7]= \
	    hTech1.attr[8]=hTech1.attr[9]=hTech1.attr[10]=hTech1.attr[11]=
		Pwr[(Shm->Proc.PowerNow == 0b11)];

	    hTech1.attr[13]=hTech1.attr[14]=hTech1.attr[15]=		\
	    hTech1.attr[16]=hTech1.attr[17]=Pwr[isTurboBoost];

	    hTech1.attr[19]=hTech1.attr[20]=hTech1.attr[21]=		\
		Pwr[isEnhancedHaltState];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[23]=buffer[0];
	    hTech1.code[24]=buffer[1];
	    hTech1.code[25]=buffer[2];

	    hTech1.attr[23]=hTech1.attr[24]=hTech1.attr[25]=		\
		Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[27]=hTech1.attr[28]=hTech1.attr[29]=		\
		Pwr[(Shm->Proc.Features.AdvPower.DX.TS != 0)];

	    hTech1.attr[31]=hTech1.attr[32]=hTech1.attr[33]=		\
		Pwr[(Shm->Proc.Features.AdvPower.DX.TTP != 0)];

	    LayerCopyAt(layer, hTech1.origin.col, hTech1.origin.row,
			hTech1.length, hTech1.attr, hTech1.code);

	    LayerFillAt(layer, (hTech1.origin.col + hTech1.length),
				hTech1.origin.row,
				(drawSize.width-hTech0.length-hTech1.length),
				hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
	}

	struct utsname OSinfo={{0}};
	uname(&OSinfo);

	row++ ;
	len=strlen(OSinfo.sysname);

	LayerFillAt(	layer, col, row,
			len, OSinfo.sysname,
			MakeAttr(CYAN, 0, BLACK, 1));

	col += len;

	LayerAt(layer, attr, col, row)=MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, col, row)=0x20;

	col++ ;

	LayerAt(layer, attr, col, row)=MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, col, row)='[';

	col++ ;
	len=strlen(OSinfo.release);

	LayerFillAt(	layer, col, row,
			len, OSinfo.release,
			MakeAttr(WHITE, 0, BLACK, 0));

	col += len;
	len=strlen(Shm->IdleDriver.Name);
	if(len > 0)
	{
		LayerAt(layer, attr, col, row)=MakeAttr(BLACK, 0, BLACK, 1);
		LayerAt(layer, code, col, row)='/';

		col++ ;

		LayerFillAt(	layer, col, row,
				len, Shm->IdleDriver.Name,
				MakeAttr(WHITE, 0, BLACK, 0));

		col += len;
	}
	LayerAt(layer, attr, col, row)=MakeAttr(BLACK, 0, BLACK, 1);
	LayerAt(layer, code, col, row)=']';

	col++ ;

	LayerDeclare(41) hSys1=
	{
	    .origin={.col=(drawSize.width - 41), .row=row}, .length=41,
	    .attr={
		LWK,LWK,LWK,LWK,LWK,HDK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HDK, \
		HDK,LWK,LWK,LWK,HDK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK, \
		HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,HDK
		},
	    .code={
		'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',0x0, \
		' ',0x0,0x0,0x0,' ',0x0,' ',' ',' ',' ',' ',' ',' ',' ', \
		0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ','K','B',']'
		},
	};

	if((len=hSys1.origin.col - col) > 0)
	    LayerFillAt(layer, col, hSys1.origin.row,
			len, hSpace,
			MakeAttr(BLACK, 0, BLACK, 1));

	LayerCopyAt(layer, hSys1.origin.col, hSys1.origin.row,
			hSys1.length, hSys1.attr, hSys1.code);

	while(++row < drawSize.height)
		LayerFillAt(	layer, 0, row,
				drawSize.width, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));
    }

    TrapScreenSize(SIGWINCH);
    signal(SIGWINCH, TrapScreenSize);

    AllocAll();

    while(!Shutdown)
    {
	do
	{
	  SCANKEY scan={.key=0};

	  if((drawFlag.daemon=BITVAL(Shm->Proc.Sync, 0)) == 0)
	  {
	    if(GetKey(&scan, &tsec) > 0)
	    {
		if(Shortcut(&scan) == -1)
		{
		    if(IsDead(&winList))
			AppendWindow(CreateMenu(SCANKEY_F1), &winList);
		    else
			if(Motion_Trigger(&scan,GetFocus(&winList),&winList)>0)
				Shortcut(&scan);
		}
		break;
	    }
	  }
	} while(!Shutdown && !drawFlag.daemon && !drawFlag.layout) ;

	if(drawFlag.daemon)
	{
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);
		if(BITVAL(Shm->Proc.Sync, 63))
		{	// Platform changed, redraw the layout.
			drawFlag.layout=1;
			BITCLR(BUS_LOCK, Shm->Proc.Sync, 63);
		}
	}

      if(drawFlag.height & drawFlag.width)
      {
	if(drawFlag.clear)
	{
		drawFlag.clear=0;
		drawFlag.layout=1;

		ResetLayer(dLayer);
	}

	if(drawFlag.layout)
	{
		drawFlag.layout=0;

		ResetLayer(sLayer);
		Layout(sLayer);
	}
	if(drawFlag.daemon)
	{
	  maxRelFreq=0.0;
	  topRatio=0;
	  for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if(!Shm->Cpu[cpu].OffLine.HW)
	    {
		struct FLIP_FLOP *Flop=
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if(Flop->Relative.Freq > maxRelFreq)
		{
			maxRelFreq=Flop->Relative.Freq;
			topRatio=Flop->Relative.Ratio;
			if(topRatio > medianRatio)
				lcdColor=RED;
			else if(topRatio > minRatio)
				lcdColor=YELLOW;
			else
				lcdColor=GREEN;
		}

		if(!Shm->Cpu[cpu].OffLine.OS)
		{
		unsigned short bar0=(Flop->Relative.Ratio * loadWidth)/maxRatio,
				bar1=loadWidth - bar0,
				row=1 + TOP_HEADER_ROW + cpu;

		    LayerFillAt(dLayer, LOAD_LEAD, row,
				bar0, hBar,
				MakeAttr((Flop->Relative.Ratio > medianRatio ?
					RED : Flop->Relative.Ratio > minRatio ?
					YELLOW : GREEN),
					0, BLACK, 1));

		    LayerFillAt(dLayer, (bar0 + LOAD_LEAD), row,
				bar1, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));

		    row=2 + TOP_HEADER_ROW + cpu + Shm->Proc.CPU.Count;
		    switch(drawFlag.view)
		    {
		      case 0:
		      {
			sprintf(&LayerAt(dLayer, code, LOAD_LEAD - 1, row),
				"%c"					\
				"%7.2f" " (" "%5.2f" ") "		\
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% " \
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  " \
				"%-3llu" "/" "%3llu" "/" "%3llu",
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

			Attribute warning={.fg=WHITE, .un=0, .bg=BLACK, .bf=1};

		  if(Flop->Thermal.Temp <= Shm->Cpu[cpu].PowerThermal.Limit[0])
			warning=MakeAttr(BLUE, 0, BLACK, 1);
		else
		  if(Flop->Thermal.Temp >= Shm->Cpu[cpu].PowerThermal.Limit[1])
			warning=MakeAttr(YELLOW, 0, BLACK, 0);

		  if(Flop->Thermal.Trip)
			warning=MakeAttr(RED, 0, BLACK, 1);

			LayerAt(dLayer, attr, LOAD_LEAD + 69, row)=	\
			LayerAt(dLayer, attr, LOAD_LEAD + 70, row)=	\
			LayerAt(dLayer, attr, LOAD_LEAD + 71, row)=warning;
		      }
		      break;
		      case 1:
		      {
			sprintf(&LayerAt(dLayer, code, LOAD_LEAD - 1, row),
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
		      case 2:
		      {
			sprintf(&LayerAt(dLayer, code, LOAD_LEAD - 1, row),
				"%c"					\
				"%18llu%18llu%18llu%18llu",
				(cpu == iClock) ? '~' : 0x20,
				Flop->Delta.C0.UCC,
				Flop->Delta.C0.URC,
				Flop->Delta.C1,
				Flop->Delta.TSC);
		      }
		      break;
		      case 3:
		      {
			sprintf(&LayerAt(dLayer, code, LOAD_LEAD - 1, row),
				"%c"					\
				"%18llu%18llu%18llu%18llu",
				(cpu == iClock) ? '~' : 0x20,
				Flop->Delta.C1,
				Flop->Delta.C3,
				Flop->Delta.C6,
				Flop->Delta.C7);
		      }
		      break;
		    }
		}
		else
		{
			unsigned short row=2 + TOP_HEADER_ROW
					 + cpu + Shm->Proc.CPU.Count;

			sprintf(&LayerAt(dLayer, code, LOAD_LEAD - 1, row),
				"%c", (cpu == iClock) ? '~' : 0x20);
		}
	    }

	    switch(drawFlag.view)
	    {
	    case 0:
		{
		unsigned short row=2 + TOP_HEADER_ROW + 2 * Shm->Proc.CPU.Count;

		sprintf(&LayerAt(dLayer, code, 20, row),
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
	    }

	    if(tSteps == 0)
	    {
		struct sysinfo sysLinux=
		{
			.totalram=0,
			.freeram=0,
			.procs=0
		};
		sysinfo(&sysLinux);

		unsigned short row=2 + TOP_HEADER_ROW + TOP_FOOTER_ROW
					+ 2 * Shm->Proc.CPU.Count;

		sprintf(&LayerAt(dLayer, code, (drawSize.width - 34), row),
			"%6u""]"					\
			" Mem [""%8lu""/""%8lu",
			sysLinux.procs,
			sysLinux.freeram  / 1024,
			sysLinux.totalram / 1024);
	    }
	    tSteps++ ;
	    if(tSteps >= mSteps)
		tSteps=0;

	    {
		unsigned short j=4;
		Dec2Digit((unsigned int) maxRelFreq, digit);
		do
		{
			short offset=(4 - j) * 3;

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
		} while(j > 0) ;
	    }

		memcpy(&LayerAt(dLayer, code, 26, 2), hBClk[iClock], 11);
	    do
	    {
		iClock++;
		if(iClock == Shm->Proc.CPU.Count)
			iClock=0;
	    } while(Shm->Cpu[iClock].OffLine.OS && iClock) ;
	}

	Window *walker;
	if((walker=GetHead(&winList)) != NULL)
	{
		do
		{
			walker=walker->next;

			if(walker->hook.Print != NULL)
				walker->hook.Print(walker, &winList);
			else
				ForEachCellPrint(walker, &winList);
		} while(!IsHead(&winList, walker)) ;
	}
	Attribute attr={.value=0};
	i=0;
	for(unsigned short row=0; row < drawSize.height; row++)
	{
	  struct {
		int cursor;
	  } flag={0};
	  int j=0, k;
	  for(unsigned short col=0; col < drawSize.width; col++)
	  {
	  Attribute	*fa=&fuze->attr[col + (row * fuze->size.wth)],
			*sa=&sLayer->attr[col + (row * fuze->size.wth)],
			*da=&dLayer->attr[col + (row * fuze->size.wth)],
			*wa=&wLayer->attr[col + (row * fuze->size.wth)];
	  ASCII		*fc=&fuze->code[col + (row * fuze->size.wth)],
			*sc=&sLayer->code[col + (row * fuze->size.wth)],
			*dc=&dLayer->code[col + (row * fuze->size.wth)],
			*wc=&wLayer->code[col + (row * fuze->size.wth)];
	/* STATIC LAYER */
	    if(sa->value != 0)
		fa->value=sa->value;
	    if(*sc != 0)
		*fc=*sc;
	/* DYNAMIC LAYER */
	    if(da->value != 0)
		fa->value=da->value;
	    if(*dc != 0)
		*fc=*dc;
	/* WINDOWS LAYER */
	    if(wa->value != 0)
		fa->value=wa->value;
	    if(*wc != 0)
		*fc=*wc;
	/* FUZED LAYER */
	    if((fa->fg ^ attr.fg) || (fa->bg ^ attr.bg) || (fa->bf ^ attr.bf))
	    {
		buffer[j++]=0x1b;
		buffer[j++]='[';
		buffer[j++]='0' + fa->bf;
		buffer[j++]=';';
		buffer[j++]='3';
		buffer[j++]='0' + fa->fg;
		buffer[j++]=';';
		buffer[j++]='4';
		buffer[j++]='0' + fa->bg;
		buffer[j++]='m';
	    }
	    if(fa->un ^ attr.un)
	    {
		buffer[j++]=0x1b;
		buffer[j++]='[';
		if(fa->un)
		{
			buffer[j++]='4';
			buffer[j++]='m';
		}
		else
		{
			buffer[j++]='2';
			buffer[j++]='4';
			buffer[j++]='m';
		}
	    }
	    attr.value=fa->value;

	    if(*fc != 0)
	    {
		if(flag.cursor == 0)
		{
			flag.cursor=1;

			struct {
				unsigned short col, row;
			} scr={.col=col + 1, .row=row + 1};

			buffer[j++]=0x1b;
			buffer[j++]='[';

			for(j=log10(scr.row)+j+1, k=j; scr.row > 0; scr.row/=10)
				buffer[--k]='0' + (scr.row % 10);

			buffer[j++]=';';

			for(j=log10(scr.col)+j+1, k=j; scr.col > 0; scr.col/=10)
				buffer[--k]='0' + (scr.col % 10);

			buffer[j++]='H';
		}
		buffer[j++]=*fc;
	    }
	    else
		flag.cursor=0;
	  }
	  memcpy(&viewMask[i], buffer, j);
	  i+=j ;
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
		"  Copyright (C) 2015-2016 CYRIL INGENIERIE\n\n");
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
	struct stat shmStat={0};
	SHM_STRUCT *Shm;
	int fd=-1, rc=EXIT_SUCCESS;

	char *program=strdup(argv[0]), *appName=basename(program);
	char option='t';
	if((argc >= 2) && (argv[1][0] == '-'))
		option=argv[1][1];
	if(option == 'h')
		Help(appName);
	else if(((fd=shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) !=-1)
		&& ((fstat(fd, &shmStat) != -1)
		&& ((Shm=mmap(0, shmStat.st_size,
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))!=MAP_FAILED)))
	    {
		switch(option)
		{
			case 's':
			{
				SysInfo(Shm, 80, NULL);
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
				if(argc == 6)
				{
					printf(SCP SCR1 HIDE);
					TrapSignal();
					Dashboard(Shm,	atoi(argv[2]),
							atoi(argv[3]),
							atoi(argv[4]),
							atoi(argv[5]) );
					printf(SHOW SCR0 RCP COLOR(0,9,9));
				}
				else if(argc == 2)
				{
					printf(SCP SCR1 HIDE);
					TrapSignal();
					Dashboard(Shm,	LEADING_LEFT,
							LEADING_TOP,
							MARGIN_WIDTH,
							MARGIN_HEIGHT);
					printf(SHOW SCR0 RCP COLOR(0,9,9));
				}
				else
					rc=Help(appName);
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
				rc=Help(appName);
			break;
		}
		munmap(Shm, shmStat.st_size);
		close(fd);
	    }
		else rc=2;
	free(program);
	return(rc);
}
