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
#include <time.h>
#include <poll.h>
#include <termios.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sched.h>

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

const char lcd[10][3][3] = {
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
#define MAX_CPU_ROW	48

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

typedef union {
	unsigned long long key;
	unsigned char code[8];
} SCANKEY;

#define SCANKEY_VOID		0x8000000000000000
#define SCANKEY_NULL		0x0000000000000000
#define SCANKEY_TAB		0x0000000000000009
#define SCANKEY_ENTER		0x000000000000000a
#define SCANKEY_ESC		0x000000000000001b
#define SCANKEY_PLUS		0x000000000000002b
#define SCANKEY_MINUS		0x000000000000002d
#define SCANKEY_UP		0x0000000000415b1b
#define SCANKEY_DOWN		0x0000000000425b1b
#define SCANKEY_RIGHT		0x0000000000435b1b
#define SCANKEY_LEFT		0x0000000000445b1b
#define SCANKEY_HOME		0x0000000000485b1b
#define SCANKEY_END		0x0000000000465b1b
#define SCANKEY_F1		0x0000000000504f1b
#define SCANKEY_F2		0x0000000000514f1b
#define SCANKEY_F3		0x0000000000524f1b
#define SCANKEY_F4		0x0000000000534f1b
#define SCANKEY_F10		0x0000000031325b1b
#define SCANKEY_SHIFT_TAB	0x00000000005a5b1b
#define SCANKEY_PGUP		0x000000007e355b1b
#define SCANKEY_PGDW		0x000000007e365b1b
#define SCANKEY_PERCENT		0x0000000000000025
#define SCANKEY_SHIFT_a		0x0000000000000041
#define SCANKEY_SHIFT_d		0x0000000000000044
#define SCANKEY_SHIFT_m		0x000000000000004d
#define SCANKEY_SHIFT_q		0x0000000000000051
#define SCANKEY_SHIFT_s		0x0000000000000053
#define SCANKEY_SHIFT_v		0x0000000000000056
#define SCANKEY_SHIFT_w		0x0000000000000057
#define SCANKEY_SHIFT_z		0x000000000000005a
#define SCANKEY_a		0x0000000000000061
#define SCANKEY_b		0x0000000000000062
#define SCANKEY_c		0x0000000000000063
#define SCANKEY_d		0x0000000000000064
#define SCANKEY_e		0x0000000000000065
#define SCANKEY_f		0x0000000000000066
#define SCANKEY_g		0x0000000000000067
#define SCANKEY_h		0x0000000000000068
#define SCANKEY_i		0x0000000000000069
#define SCANKEY_k		0x000000000000006b
#define SCANKEY_l		0x000000000000006c
#define SCANKEY_m		0x000000000000006d
#define SCANKEY_n		0x000000000000006e
#define SCANKEY_o		0x000000000000006f
#define SCANKEY_p		0x0000000000000070
#define SCANKEY_q		0x0000000000000071
#define SCANKEY_r		0x0000000000000072
#define SCANKEY_s		0x0000000000000073
#define SCANKEY_t		0x0000000000000074
#define SCANKEY_u		0x0000000000000075
#define SCANKEY_v		0x0000000000000076
#define SCANKEY_w		0x0000000000000077
#define SCANKEY_x		0x0000000000000078
#define SCANKEY_z		0x000000000000007a

#define SCANCON_HOME		0x000000007e315b1b
#define SCANCON_END		0x000000007e345b1b
#define SCANCON_F1		0x00000000415b5b1b
#define SCANCON_F2		0x00000000425b5b1b
#define SCANCON_F3		0x00000000435b5b1b
#define SCANCON_F4		0x00000000445b5b1b
#define SCANCON_SHIFT_TAB	0x000000000000091b

#define SORTBY_STATE		0x400000000000000e
#define SORTBY_RTIME		0x400000000000000d
#define SORTBY_UTIME		0x400000000000000c
#define SORTBY_STIME		0x400000000000000b
#define SORTBY_PID		0x400000000000000a
#define SORTBY_COMM		0x4000000000000009

#define BOXKEY_EIST		0x3000000000000004
#define BOXKEY_EIST_OFF		0x3000000000000005
#define BOXKEY_EIST_ON		0x3000000000000006
#define BOXKEY_C1E		0x3000000000000008
#define BOXKEY_C1E_OFF		0x3000000000000009
#define BOXKEY_C1E_ON		0x300000000000000a
#define BOXKEY_TURBO		0x3000000000000010
#define BOXKEY_TURBO_OFF	0x3000000000000011
#define BOXKEY_TURBO_ON		0x3000000000000012
#define BOXKEY_C1A		0x3000000000000020
#define BOXKEY_C1A_OFF		0x3000000000000021
#define BOXKEY_C1A_ON		0x3000000000000022
#define BOXKEY_C3A		0x3000000000000040
#define BOXKEY_C3A_OFF		0x3000000000000041
#define BOXKEY_C3A_ON		0x3000000000000042
#define BOXKEY_C1U		0x3000000000000080
#define BOXKEY_C1U_OFF		0x3000000000000081
#define BOXKEY_C1U_ON		0x3000000000000082
#define BOXKEY_C3U		0x3000000000000100
#define BOXKEY_C3U_OFF		0x3000000000000101
#define BOXKEY_C3U_ON		0x3000000000000102
#define BOXKEY_PKGCST		0x3000000000000200
#define BOXKEY_PKGCST_C0	0x3000000000000201
#define BOXKEY_PKGCST_C1	0x3000000000000211
#define BOXKEY_PKGCST_C2	0x3000000000000221
#define BOXKEY_PKGCST_C3	0x3000000000000231
#define BOXKEY_PKGCST_C6	0x3000000000000261
#define BOXKEY_PKGCST_C7	0x3000000000000271
#define BOXKEY_IOMWAIT		0x3000000000000400
#define BOXKEY_IOMWAIT_OFF	0x3000000000000401
#define BOXKEY_IOMWAIT_ON	0x3000000000000402
#define BOXKEY_IORCST		0x3000000000000800
#define BOXKEY_IORCST_C0	0x3000000000000801
#define BOXKEY_IORCST_C3	0x3000000000000831
#define BOXKEY_IORCST_C4	0x3000000000000841
#define BOXKEY_IORCST_C6	0x3000000000000861
#define BOXKEY_IORCST_C7	0x3000000000000871
#define BOXKEY_ODCM		0x3000000000001000
#define BOXKEY_ODCM_OFF		0x3000000000001001
#define BOXKEY_ODCM_ON		0x3000000000001002
#define BOXKEY_DUTYCYCLE	0x3000000000002000
#define BOXKEY_ODCM_DC00	0x3000000000002001
#define BOXKEY_ODCM_DC01	0x3000000000002011
#define BOXKEY_ODCM_DC02	0x3000000000002021
#define BOXKEY_ODCM_DC03	0x3000000000002031
#define BOXKEY_ODCM_DC04	0x3000000000002041
#define BOXKEY_ODCM_DC05	0x3000000000002051
#define BOXKEY_ODCM_DC06	0x3000000000002061
#define BOXKEY_ODCM_DC07	0x3000000000002071
#define BOXKEY_ODCM_DC08	0x3000000000002081
#define BOXKEY_ODCM_DC09	0x3000000000002091
#define BOXKEY_ODCM_DC10	0x30000000000020a1
#define BOXKEY_ODCM_DC11	0x30000000000020b1
#define BOXKEY_ODCM_DC12	0x30000000000020c1
#define BOXKEY_ODCM_DC13	0x30000000000020d1
#define BOXKEY_ODCM_DC14	0x30000000000020e1

#define TRACK_TASK		0x2000000000000000
#define TRACK_MASK		0x0000000000007fff


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

void printv(	void(*OutFunc)(unsigned long long key, char *output),
		unsigned long long key, CUINT width, int tab, char *fmt, ...)
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
	    OutFunc(key, output);
	    free(output);
	}
	va_end(ap);
	free(line);
}

void SysInfoCPUID(SHM_STRUCT *Shm, CUINT width,
		void(*OutFunc)(unsigned long long key, char *output) )
{
	char format[] = "%08x:%08x%.*s%08x     %08x     %08x     %08x";
	unsigned int cpu;
	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	    if (OutFunc == NULL) {
		printv(OutFunc, SCANKEY_NULL, width, 0,
			"CPU #%-2u function"				\
			"          EAX          EBX          ECX          EDX",
			cpu);
	    } else {
		printv(OutFunc, SCANKEY_NULL, width, 0, "CPU #%-2u", cpu);
	    }
		printv(OutFunc, SCANKEY_NULL, width, 2, format,
			0x00000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc,
			Shm->Cpu[cpu].Query.StdFunc.BX,
			Shm->Cpu[cpu].Query.StdFunc.CX,
			Shm->Cpu[cpu].Query.StdFunc.DX);

		printv(OutFunc, SCANKEY_NULL, width, 3,
			"Largest Standard Function=%08x",
			Shm->Cpu[cpu].Query.StdFunc.LargestStdFunc);

		printv(OutFunc, SCANKEY_NULL, width, 2, format,
			0x80000000, 0x00000000,
			4, hSpace,
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc,
			Shm->Cpu[cpu].Query.ExtFunc.BX,
			Shm->Cpu[cpu].Query.ExtFunc.CX,
			Shm->Cpu[cpu].Query.ExtFunc.DX);

		printv(OutFunc, SCANKEY_NULL, width, 3,
			"Largest Extended Function=%08x",
			Shm->Cpu[cpu].Query.ExtFunc.LargestExtFunc);

		int i;
		for (i = 0; i < CPUID_MAX_FUNC; i++)
		    if (Shm->Cpu[cpu].CpuID[i].func) {
			printv(OutFunc, SCANKEY_NULL, width, 2, format,
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
		CUINT width,
		void(*OutFunc)(unsigned long long key, char *output))
{
	size_t	len = 0;
	char	*row[2] = {malloc(width + 1), malloc(width + 1)},
		*str = malloc(width + 1),
		*pad = NULL;
	int	i = 0;

/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 0, "Processor%.*s[%s]",
		width - 11 - strlen(Shm->Proc.Brand), hSpace, Shm->Proc.Brand);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Architecture%.*s[%s]",
		width - 17 - strlen(Shm->Proc.Architecture), hSpace,
		Shm->Proc.Architecture);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Vendor ID%.*s[%s]",
		width - 14 - strlen(Shm->Proc.Features.Info.VendorID), hSpace,
		Shm->Proc.Features.Info.VendorID);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Signature%.*s[%1X%1X_%1X%1X]",
		width - 19, hSpace,
		Shm->Proc.Features.Std.AX.ExtFamily,
		Shm->Proc.Features.Std.AX.Family,
		Shm->Proc.Features.Std.AX.ExtModel,
		Shm->Proc.Features.Std.AX.Model);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Stepping%.*s[%5u]",
		width - 18, hSpace, Shm->Proc.Features.Std.AX.Stepping);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Microcode%.*s[%5u]",
		width - 19, hSpace, Shm->Cpu[0].Query.Microcode);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Online CPU%.*s[%2u/%-2u]",
		width - 20, hSpace, Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Base Clock%.*s[%5.1f]",
		width - 20, hSpace, Shm->Cpu[0].Clock.Hz / 1000000.0);

	len = sprintf(row[0],
			"Core Boost%.*sMin   Max"			\
			"    8C    7C    6C    5C    4C    3C    2C    1C",
			6, hSpace);

	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);

	if (OutFunc == NULL) {
		strcpy(row[0], "   |- ratio :    ");
		strcpy(row[1], "   |-  freq :    ");
	} else {
		strcpy(row[0], "  |- ratio :    ");
		strcpy(row[1], "  |-  freq :    ");
	}
	for (i = 0; i < 10; i++) {
		if (Shm->Proc.Boost[i] != 0) {
			len += sprintf(str, " %4d ", Shm->Proc.Boost[i]);
			strcat(row[0], str);

			len += sprintf(str, " %4.0f ",
					(double) ( Shm->Proc.Boost[i]
					* Shm->Cpu[0].Clock.Hz) / 1000000.0);
			strcat(row[1], str);
		} else {
			strcat(row[0], "   -  ");
			strcat(row[1], "      ");
		}
	}
	printv(OutFunc, SCANKEY_NULL, width, 0, row[0]);
	printv(OutFunc, SCANKEY_NULL, width, 0, row[1]);
/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 0, "Instruction set:");
/* Row Mark */
	len = 3;
	len += sprintf(str, "3DNow!/Ext [%c,%c]",
			Shm->Proc.Features.ExtInfo.DX._3DNow ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.DX._3DNowEx ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 11, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "AES [%c]",
			Shm->Proc.Features.Std.CX.AES ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 6, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "AVX/AVX2 [%c/%c]",
			Shm->Proc.Features.Std.CX.AVX ? 'Y' : 'N',
			Shm->Proc.Features.ExtFeature.BX.AVX2 ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "BMI1/BMI2 [%c/%c]",
			Shm->Proc.Features.ExtFeature.BX.BMI1 ? 'Y' : 'N',
			Shm->Proc.Features.ExtFeature.BX.BMI2 ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], pad);
	strcat(row[0], str);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);
/* Row Mark */
	len = 3;
	len += sprintf(str, "CLFSH        [%c]",
			Shm->Proc.Features.Std.DX.CLFSH ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "CMOV [%c]",
			Shm->Proc.Features.Std.DX.CMOV ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 7, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "CMPXCH8   [%c]",
			Shm->Proc.Features.Std.DX.CMPXCH8 ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "CMPXCH16   [%c]",
			Shm->Proc.Features.Std.CX.CMPXCH16 ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], pad);
	strcat(row[0], str);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);
/* Row Mark */
	len = 3;
	len += sprintf(str, "F16C         [%c]",
			Shm->Proc.Features.Std.CX.F16C ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 11, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "FPU [%c]",
			Shm->Proc.Features.Std.DX.FPU ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "FXSR   [%c]",
			Shm->Proc.Features.Std.DX.FXSR ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "LAHF/SAHF   [%c]",
			Shm->Proc.Features.ExtInfo.CX.LAHFSAHF ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], pad);
	strcat(row[0], str);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);
/* Row Mark */
	len = 3;
	len += sprintf(str, "MMX/Ext    [%c/%c]",
			Shm->Proc.Features.Std.DX.MMX ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.DX.MMX_Ext ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 7, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "MONITOR [%c]",
			Shm->Proc.Features.Std.CX.MONITOR ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 9, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "MOVBE   [%c]",
			Shm->Proc.Features.Std.CX.MOVBE ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "PCLMULDQ   [%c]",
			Shm->Proc.Features.Std.CX.PCLMULDQ ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], pad);
	strcat(row[0], str);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);
/* Row Mark */
	len = 3;
	len += sprintf(str, "POPCNT       [%c]",
			Shm->Proc.Features.Std.CX.POPCNT ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 8, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "RDRAND [%c]",
			Shm->Proc.Features.Std.CX.RDRAND ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 8, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "RDTSCP   [%c]",
			Shm->Proc.Features.ExtInfo.DX.RDTSCP ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "SEP   [%c]",
			Shm->Proc.Features.Std.DX.SEP ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], pad);
	strcat(row[0], str);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);
/* Row Mark */
	len = 3;
	len += sprintf(str, "SSE          [%c]",
			Shm->Proc.Features.Std.DX.SSE ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "SSE2 [%c]",
			Shm->Proc.Features.Std.DX.SSE2 ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 10, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "SSE3   [%c]",
			Shm->Proc.Features.Std.CX.SSE3 ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "SSSE3   [%c]",
			Shm->Proc.Features.Std.CX.SSSE3 ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], pad);
	strcat(row[0], str);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);
/* Row Mark */
	len = 3;
	len += sprintf(str, "SSE4.1/4A  [%c/%c]",
			Shm->Proc.Features.Std.CX.SSE41 ? 'Y' : 'N',
			Shm->Proc.Features.ExtInfo.CX.SSE4A ? 'Y' : 'N');

	strcpy(row[0], str);

	len += sprintf(str, "%.*s", 8, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "SSE4.2 [%c]",
			Shm->Proc.Features.Std.CX.SSE42 ? 'Y' : 'N');

	strcat(row[0], str);

	len += sprintf(str, "%.*s", 7, hSpace);

	strcat(row[0], str);

	len += sprintf(str, "SYSCALL   [%c]",
			Shm->Proc.Features.ExtInfo.DX.SYSCALL ? 'Y' : 'N');

	pad = realloc(pad, (width - len) + 1);
	sprintf(pad, "%.*s", (int)(width - len), hSpace);

	strcat(row[0], str);
	strcat(row[0], pad);
	printv(OutFunc, SCANKEY_NULL, width, 2, row[0]);

	free(row[0]);
	free(row[1]);
	free(str);
	free(pad);
}

void SysInfoFeatures(	SHM_STRUCT *Shm, CUINT width,
			void(*OutFunc)(unsigned long long key, char *output) )
{
/* Section Mark */
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
/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 2,
		"1 GB Pages Support%.*s1GB-PAGES   [%7s]",
		width - 42, hSpace,
		powered(Shm->Proc.Features.ExtInfo.DX.PG_1GB));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"100 MHz multiplier Control%.*s100MHzSteps   [%7s]",
		width - 52, hSpace,
		powered(Shm->Proc.Features.AdvPower.DX._100MHz));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Advanced Configuration & Power Interface"		\
					"%.*sACPI   [%7s]",
		width - 59, hSpace,
		powered(Shm->Proc.Features.Std.DX.ACPI			// Intel
			| Shm->Proc.Features.AdvPower.DX.HwPstate) );	// AMD

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Advanced Programmable Interrupt Controller"		\
						"%.*sAPIC   [%7s]",
		width - 61, hSpace, powered(Shm->Proc.Features.Std.DX.APIC));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Core Multi-Processing%.*sCMP Legacy   [%7s]",
		width - 46, hSpace,
		powered(Shm->Proc.Features.ExtInfo.CX.MP_Mode));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"L1 Data Cache Context ID%.*sCNXT-ID   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.CX.CNXT_ID));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Direct Cache Access%.*sDCA   [%7s]",
		width - 37, hSpace, powered(Shm->Proc.Features.Std.CX.DCA));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Debugging Extension%.*sDE   [%7s]",
		width - 36, hSpace, powered(Shm->Proc.Features.Std.DX.DE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Debug Store & Precise Event Based Sampling"		\
					"%.*sDS, PEBS   [%7s]",
		width - 65, hSpace, powered(Shm->Proc.Features.Std.DX.DS_PEBS));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"CPL Qualified Debug Store%.*sDS-CPL   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.CX.DS_CPL));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"64-Bit Debug Store%.*sDTES64   [%7s]",
		width - 39, hSpace, powered(Shm->Proc.Features.Std.CX.DTES64));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Fast-String Operation%.*sFast-Strings   [%7s]",
		width - 48,hSpace,
		powered(Shm->Proc.Features.ExtFeature.BX.FastStrings));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Fused Multiply Add%.*sFMA|FMA4   [%7s]",
		width - 41, hSpace,
		powered(  Shm->Proc.Features.Std.CX.FMA
			| Shm->Proc.Features.ExtInfo.CX.FMA4 ));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Hardware Lock Elision%.*sHLE   [%7s]",
		width - 39, hSpace,
		powered(Shm->Proc.Features.ExtFeature.BX.HLE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Long Mode 64 bits%.*sIA64|LM   [%7s]",
		width - 39, hSpace,
		powered(Shm->Proc.Features.ExtInfo.DX.IA64));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"LightWeight Profiling%.*sLWP   [%7s]",
		width - 39, hSpace,
		powered(Shm->Proc.Features.ExtInfo.CX.LWP));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Machine-Check Architecture%.*sMCA   [%7s]",
		width - 44, hSpace,
		powered(Shm->Proc.Features.Std.DX.MCA));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Model Specific Registers%.*sMSR   [%7s]",
		width - 42, hSpace, powered(Shm->Proc.Features.Std.DX.MSR));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Memory Type Range Registers%.*sMTRR   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.DX.MTRR));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"OS-Enabled Ext. State Management%.*sOSXSAVE   [%7s]",
		width - 54, hSpace, powered(Shm->Proc.Features.Std.CX.OSXSAVE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Physical Address Extension%.*sPAE   [%7s]",
		width - 44, hSpace, powered(Shm->Proc.Features.Std.DX.PAE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Page Attribute Table%.*sPAT   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.DX.PAT));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Pending Break Enable%.*sPBE   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.DX.PBE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Process Context Identifiers%.*sPCID   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.CX.PCID));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Perfmon and Debug Capability%.*sPDCM   [%7s]",
		width - 47, hSpace, powered(Shm->Proc.Features.Std.CX.PDCM));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Page Global Enable%.*sPGE   [%7s]",
		width - 36, hSpace, powered(Shm->Proc.Features.Std.DX.PGE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Page Size Extension%.*sPSE   [%7s]",
		width - 37, hSpace, powered(Shm->Proc.Features.Std.DX.PSE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"36-bit Page Size Extension%.*sPSE36   [%7s]",
		width - 46, hSpace, powered(Shm->Proc.Features.Std.DX.PSE36));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Processor Serial Number%.*sPSN   [%7s]",
		width - 41, hSpace, powered(Shm->Proc.Features.Std.DX.PSN));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Restricted Transactional Memory%.*sRTM   [%7s]",
		width - 49, hSpace,
		powered(Shm->Proc.Features.ExtFeature.BX.RTM));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Safer Mode Extensions%.*sSMX   [%7s]",
		width - 39, hSpace, powered(Shm->Proc.Features.Std.CX.SMX));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Self-Snoop%.*sSS   [%7s]",
		width - 27, hSpace, powered(Shm->Proc.Features.Std.DX.SS));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Time Stamp Counter%.*sTSC [%9s]",
		width - 36, hSpace, TSC[Shm->Proc.InvariantTSC]);

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Time Stamp Counter Deadline%.*sTSC-DEADLINE   [%7s]",
		width - 54, hSpace, powered(Shm->Proc.Features.Std.CX.TSCDEAD));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Virtual Mode Extension%.*sVME   [%7s]",
		width - 40, hSpace, powered(Shm->Proc.Features.Std.DX.VME));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Virtual Machine Extensions%.*sVMX   [%7s]",
		width - 44, hSpace, powered(Shm->Proc.Features.Std.CX.VMX));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Extended xAPIC Support%.*sx2APIC   [%7s]",
		width - 43, hSpace, x2APIC[Shm->Cpu[0].Topology.MP.x2APIC]);

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Execution Disable Bit Support%.*sXD-Bit   [%7s]",
		width - 50, hSpace,
		powered(Shm->Proc.Features.ExtInfo.DX.XD_Bit));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"XSAVE/XSTOR States%.*sXSAVE   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.CX.XSAVE));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"xTPR Update Control%.*sxTPR   [%7s]",
		width - 38, hSpace, powered(Shm->Proc.Features.Std.CX.xTPR));
}

void SysInfoTech(SHM_STRUCT *Shm, CUINT width,
		void(*OutFunc)(unsigned long long key, char *output) )
{
	const unsigned int
	    isTurboBoost = (Shm->Proc.TurboBoost==Shm->Proc.TurboBoost_Mask),
	    isSpeedStep  = (Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask);

/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Hyper-Threading%.*sHTT       [%3s]",
		width - 33, hSpace, enabled(Shm->Proc.HyperThreading));

	printv(OutFunc, BOXKEY_EIST, width, 2,
		"SpeedStep%.*sEIST       <%3s>",
		width - 28, hSpace, enabled(isSpeedStep));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"PowerNow!%.*sPowerNow       [%3s]",
		width - 32, hSpace,
		enabled(Shm->Proc.PowerNow == 0b11));	// VID + FID

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Dynamic Acceleration%.*sIDA       [%3s]",
		width - 38, hSpace,
		enabled(Shm->Proc.Features.Power.AX.TurboIDA));

	printv(OutFunc, BOXKEY_TURBO, width, 2,
		"Turbo Boost/CPB%.*sTURBO       <%3s>",
		width - 35, hSpace,
		enabled(isTurboBoost|Shm->Proc.Features.AdvPower.DX.CPB));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Virtualization%.*sHYPERVISOR       [%3s]",
		width - 39, hSpace,
		enabled(Shm->Proc.Features.Std.CX.Hyperv));
}

void SysInfoPerfMon(	SHM_STRUCT *Shm, CUINT width,
			void(*OutFunc)(unsigned long long key, char *output) )
{
	const unsigned int
		isEnhancedHaltState = (Shm->Proc.C1E == Shm->Proc.C1E_Mask),
		isC3autoDemotion = (Shm->Proc.C3A == Shm->Proc.C3A_Mask),
		isC1autoDemotion = (Shm->Proc.C1A == Shm->Proc.C1A_Mask),
		isC3undemotion = (Shm->Proc.C3U == Shm->Proc.C3U_Mask),
		isC1undemotion = (Shm->Proc.C1U == Shm->Proc.C1U_Mask);

/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Version%.*sPM       [%3d]",
		width - 24, hSpace, Shm->Proc.PM_version);

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Counters:%.*sGeneral%.*sFixed",
		10, hSpace, width - 61, hSpace);

    if (OutFunc == NULL) {
	printv(OutFunc, SCANKEY_NULL, width, 1,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.AX.MonCtrs,
				Shm->Proc.Features.PerfMon.AX.MonWidth,
		11, hSpace,	Shm->Proc.Features.PerfMon.DX.FixCtrs,
				Shm->Proc.Features.PerfMon.DX.FixWidth);
    } else {
	printv(OutFunc, SCANKEY_NULL, width, 0,
		"%.*s%3u x%3u bits%.*s%3u x%3u bits",
		19, hSpace,	Shm->Proc.Features.PerfMon.AX.MonCtrs,
				Shm->Proc.Features.PerfMon.AX.MonWidth,
		5, hSpace,	Shm->Proc.Features.PerfMon.DX.FixCtrs,
				Shm->Proc.Features.PerfMon.DX.FixWidth);
    }
	printv(OutFunc, BOXKEY_C1E, width, 2,
		"Enhanced Halt State%.*sC1E       <%3s>",
		width - 37, hSpace, enabled(isEnhancedHaltState));

	printv(OutFunc, BOXKEY_C1A, width, 2,
		"C1 Auto Demotion%.*sC1A       <%3s>",
		width - 34, hSpace, enabled(isC1autoDemotion));

	printv(OutFunc, BOXKEY_C3A, width, 2,
		"C3 Auto Demotion%.*sC3A       <%3s>",
		width - 34, hSpace, enabled(isC3autoDemotion));

	printv(OutFunc, BOXKEY_C1U, width, 2,
		"C1 UnDemotion%.*sC1U       <%3s>",
		width - 31, hSpace, enabled(isC1undemotion));

	printv(OutFunc, BOXKEY_C3U, width, 2,
		"C3 UnDemotion%.*sC3U       <%3s>",
		width - 31, hSpace, enabled(isC3undemotion));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Frequency ID control%.*sFID       [%3s]",
		width - 38, hSpace,
		enabled(Shm->Proc.Features.AdvPower.DX.FID));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Voltage ID control%.*sVID       [%3s]",
		width - 36, hSpace,
		enabled(Shm->Proc.Features.AdvPower.DX.VID));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"P-State Hardware Coordination Feedback"		\
			"%.*sMPERF/APERF       [%3s]",
		width - 64, hSpace,
		enabled(Shm->Proc.Features.Power.CX.HCF_Cap));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Hardware-Controlled Performance States%.*sHWP       [%3s]",
		width - 56, hSpace,
		enabled(  Shm->Proc.Features.Power.AX.HWP_Reg
			| Shm->Proc.Features.AdvPower.DX.HwPstate));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Hardware Duty Cycling%.*sHDC       [%3s]",
		width - 39, hSpace,
		enabled(Shm->Proc.Features.Power.AX.HDC_Reg));

	printv(OutFunc, SCANKEY_NULL, width, 2, "Package C-State");

	printv(OutFunc, SCANKEY_NULL, width, 3,
		"Configuration Control%.*sCONFIG   [%7s]",
		width - (OutFunc == NULL ? 45 : 43), hSpace,
		!Shm->Cpu[0].Query.CfgLock? "UNLOCK":"LOCK");

	if (!Shm->Cpu[0].Query.CfgLock) {
		printv(OutFunc, BOXKEY_PKGCST, width, 3,
			"Lowest C-State%.*sLIMIT   <%7d>",
			width - (OutFunc == NULL ? 37 : 35), hSpace,
			Shm->Cpu[0].Query.CStateLimit);

		printv(OutFunc, BOXKEY_IOMWAIT, width, 3,
			"I/O MWAIT Redirection%.*sIOMWAIT   <%7s>",
			width - (OutFunc == NULL ? 46 : 44), hSpace,
			Shm->Cpu[0].Query.IORedir? " ENABLE":"DISABLE");

		printv(OutFunc, BOXKEY_IORCST, width, 3,
			"Max C-State Inclusion%.*sRANGE   <%7d>",
			width - (OutFunc == NULL ? 44 : 42), hSpace,
			Shm->Cpu[0].Query.CStateInclude);
	} else {
		printv(OutFunc, SCANKEY_NULL, width, 3,
			"Lowest C-State%.*sLIMIT   [%7d]",
			width - (OutFunc == NULL ? 37 : 35), hSpace,
			Shm->Cpu[0].Query.CStateLimit);

		printv(OutFunc, SCANKEY_NULL, width, 3,
			"I/O MWAIT Redirection%.*sIOMWAIT   [%7s]",
			width - (OutFunc == NULL ? 46 : 44), hSpace,
			Shm->Cpu[0].Query.IORedir? " ENABLE":"DISABLE");

		printv(OutFunc, SCANKEY_NULL, width, 3,
			"Max C-State Inclusion%.*sRANGE   [%7d]",
			width - (OutFunc == NULL ? 44 : 42), hSpace,
			Shm->Cpu[0].Query.CStateInclude);
	}
	printv(OutFunc, SCANKEY_NULL, width, 2,
		"MWAIT States:%.*sC0      C1      C2      C3      C4",
		06, hSpace);

	printv(OutFunc, SCANKEY_NULL, width, (OutFunc == NULL) ? 1 : 0,
		"%.*s%2d      %2d      %2d      %2d      %2d",
		21, hSpace,
		Shm->Proc.Features.MWait.DX.Num_C0_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C1_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C2_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C3_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C4_MWAIT);

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Core Cycles%.*s[%7s]",
		width - 23, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.CoreCycles));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Instructions Retired%.*s[%7s]",
		width - 32, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.InstrRetired));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Reference Cycles%.*s[%7s]",
		width - 28, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.RefCycles));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Last Level Cache References%.*s[%7s]",
		width - 39, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.LLC_Ref));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Last Level Cache Misses%.*s[%7s]",
		width - 35, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.LLC_Misses));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Branch Instructions Retired%.*s[%7s]",
		width - 39, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.BranchRetired));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Branch Mispredicts Retired%.*s[%7s]",
		width - 38, hSpace,
		powered(!Shm->Proc.Features.PerfMon.BX.BranchMispred));
}

void SysInfoPwrThermal( SHM_STRUCT *Shm, CUINT width,
			void(*OutFunc)(unsigned long long key, char *output) )
{
	const char *TM[] = {
		"Missing",
		"Present",
		"Disable",
		" Enable",
	}, *Unlock[] = {
		"  LOCK",
		"UNLOCK",
	};
	const unsigned int
		isODCM = (Shm->Proc.ODCM == Shm->Proc.ODCM_Mask),
		isPowerMgmt = (Shm->Proc.PowerMgmt == Shm->Proc.PowerMgmt_Mask);
/* Section Mark */
	printv(OutFunc, BOXKEY_ODCM, width, 2,
		"Clock Modulation%.*sODCM   <%7s>",
		width - 35, hSpace, isODCM ? " Enable" : "Disable");

	printv(OutFunc, BOXKEY_DUTYCYCLE, width, 3,
		"DutyCycle%.*s<%6.2f%%>",
		width - (OutFunc == NULL ? 24: 22), hSpace,
		(Shm->Cpu[0].PowerThermal.DutyCycle.Extended ?
			6.25f : 12.5f
			* Shm->Cpu[0].PowerThermal.DutyCycle.ClockMod));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Power Management%.*sPWR MGMT   [%7s]",
		width - 39, hSpace, Unlock[isPowerMgmt]);

	printv(OutFunc, SCANKEY_NULL, width, 3,
		"Energy Policy%.*sBias Hint   [%7u]",
		width - (OutFunc == NULL ? 40 : 38),
		hSpace, Shm->Cpu[0].PowerThermal.PowerPolicy);

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Junction Temperature%.*sTjMax   [%7u]",
		width - 40, hSpace, Shm->Cpu[0].PowerThermal.Target);

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Digital Thermal Sensor%.*sDTS   [%7s]",
		width - 40, hSpace,
		powered( Shm->Proc.Features.Power.AX.DTS
			|Shm->Proc.Features.AdvPower.DX.TS));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Power Limit Notification%.*sPLN   [%7s]",
		width - 42, hSpace, powered(Shm->Proc.Features.Power.AX.PLN));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Package Thermal Management%.*sPTM   [%7s]",
		width - 44, hSpace, powered(Shm->Proc.Features.Power.AX.PTM));

	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Thermal Monitor 1%.*sTM1|TTP   [%7s]",
		width - 39, hSpace, TM[  Shm->Cpu[0].PowerThermal.TM1
					|Shm->Proc.Features.AdvPower.DX.TTP ]);
	printv(OutFunc, SCANKEY_NULL, width, 2,
		"Thermal Monitor 2%.*sTM2|HTC   [%7s]",
		width - 39, hSpace, TM[  Shm->Cpu[0].PowerThermal.TM2
					|Shm->Proc.Features.AdvPower.DX.TM ]);
}

void SysInfoKernel(	SHM_STRUCT *Shm, CUINT width,
			void(*OutFunc)(unsigned long long key, char *output) )
{
	size_t	len = 0;
	char	*row = malloc(width + 1),
		*str = malloc(width + 1);
	int	i = 0;

/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 0, "%s:", Shm->SysGate.sysname);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Release%.*s[%s]",
		width - 12 - strlen(Shm->SysGate.release), hSpace,
		Shm->SysGate.release);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Version%.*s[%s]",
		width - 12 - strlen(Shm->SysGate.version), hSpace,
		Shm->SysGate.version);

	printv(OutFunc, SCANKEY_NULL, width, 2, "Machine%.*s[%s]",
		width - 12 - strlen(Shm->SysGate.machine), hSpace,
		Shm->SysGate.machine);
/* Section Mark */
	printv(OutFunc, SCANKEY_NULL, width, 0, "Memory:%.*s",
		width - 7, hSpace);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.totalram);
	printv(OutFunc, SCANKEY_NULL, width, 2, "Total RAM" "%.*s" "%s KB",
		width - 15 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.sharedram);
	printv(OutFunc, SCANKEY_NULL, width, 2, "Shared RAM" "%.*s" "%s KB",
		width - 16 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.freeram);
	printv(OutFunc, SCANKEY_NULL, width, 2, "Free RAM" "%.*s" "%s KB",
		width - 14 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.bufferram);
	printv(OutFunc, SCANKEY_NULL, width, 2, "Buffer RAM" "%.*s" "%s KB",
		width - 16 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.totalhigh);
	printv(OutFunc, SCANKEY_NULL, width, 2, "Total High" "%.*s" "%s KB",
		width - 16 - len, hSpace, str);

	len = sprintf(str, "%lu", Shm->SysGate.memInfo.freehigh);
	printv(OutFunc, SCANKEY_NULL, width, 2, "Free High" "%.*s" "%s KB",
		width - 15 - len, hSpace, str);
/* Section Mark */
    if ((len = strlen(Shm->SysGate.IdleDriver.Name)
		+ strlen(Shm->SysGate.IdleDriver.Governor)) > 0) {
	printv(OutFunc, SCANKEY_NULL, width, 0, "Idle driver%.*s[%s@%s]",
		width - 14 - len, hSpace,
		Shm->SysGate.IdleDriver.Governor, Shm->SysGate.IdleDriver.Name);
/* Row Mark */
	len = sprintf(row, "States:%.*s", 9, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8s",
			Shm->SysGate.IdleDriver.State[i].Name);
		strcat(row, str);
	}
	printv(OutFunc, SCANKEY_NULL, width, 3, row);
/* Row Mark */
	len = sprintf(row, "Power:%.*s", 10, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8d",
			Shm->SysGate.IdleDriver.State[i].powerUsage);
		strcat(row, str);
	}
	printv(OutFunc, SCANKEY_NULL, width, 3, row);
/* Row Mark */
	len = sprintf(row, "Latency:%.*s", 8, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8u",
			Shm->SysGate.IdleDriver.State[i].exitLatency);
		strcat(row, str);
	}
	printv(OutFunc, SCANKEY_NULL, width, 3, row);
/* Row Mark */
	len = sprintf(row, "Residency:%.*s", 6, hSpace);
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++) {
		len += sprintf(str, "%-8u",
			Shm->SysGate.IdleDriver.State[i].targetResidency);
		strcat(row, str);
	}
	printv(OutFunc, SCANKEY_NULL, width, 3, row);
    }
	free(row);
	free(str);
}

#define LEADING_LEFT	2
#define LEADING_TOP	1
#define MARGIN_WIDTH	2
#define MARGIN_HEIGHT	1

void LCD_Draw(	CUINT col,
		CUINT row,
		char *thisView,
		unsigned int thisNumber,
		unsigned int thisDigit[] )
{
    char lcdBuf[32];
    CUINT j = (CUINT) Dec2Digit(thisNumber, thisDigit);

    thisView[0] = '\0';
    j = 4;
    do {
	CSINT offset = col + (4 - j) * 3;

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
		CUINT leadingLeft,
		CUINT leadingTop,
		CUINT marginWidth,
		CUINT marginHeight)
{
    char *boardView = NULL,
	 *lcdView = NULL,
	 *cpuView = NULL,
	 *absMove = CUH CLS;
    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[LAST_BOOST];
    double medianRatio = (minRatio + maxRatio) / 2;

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
	CUINT X, Y;

	while (!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		nanosleep(&Shm->Proc.BaseSleep, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

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
		nanosleep(&Shm->Proc.BaseSleep, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

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

void Voltage(SHM_STRUCT *Shm)
{
    unsigned int cpu = 0;
    while (!Shutdown) {
	while (!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
		nanosleep(&Shm->Proc.BaseSleep, NULL);

	BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

		printf("CPU Freq(MHz) VID  Vcore\n");
	for (cpu = 0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	  if (!Shm->Cpu[cpu].OffLine.HW) {
	    struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if (!Shm->Cpu[cpu].OffLine.OS)
		printf("#%02u %7.2f %5d  %5.4f\n",
			cpu,
			Flop->Relative.Freq,
			Flop->Voltage.VID,
			Flop->Voltage.Vcore);
	    else
		printf("#%02u        OFF\n", cpu);
	  }
    }
}

void Instructions(SHM_STRUCT *Shm)
{
	unsigned int cpu = 0;
	while (!Shutdown) {
	  while (!BITVAL(Shm->Proc.Sync, 0) && !Shutdown)
			nanosleep(&Shm->Proc.BaseSleep, NULL);

	  BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

	  if (BITVAL(Shm->Proc.Sync, 63))
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);

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
	char line[16];

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
}

void MemoryController(SHM_STRUCT *Shm, void(*OutFunc)(char *output))
{
	unsigned int nl = 14;
	unsigned short mc, cha, slot;
	char line[8], fInt[16], hInt[2][8];

	void printv(char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsprintf(line, fmt, ap);
		if (OutFunc == NULL)
			if (!--nl) {
				nl = 14;
				printf("%s\n", line);
			}
			else
				printf("%s", line);
		else
			OutFunc(line);
		va_end(ap);
	}

	void iSplit(unsigned int sInt) {
		sprintf(fInt, "%10u", sInt);
		strncpy(hInt[0], &fInt[0], 5); hInt[0][5] = '\0';
		strncpy(hInt[1], &fInt[5], 5); hInt[1][5] = '\0';
	}

	for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {
	    printv("Contr"); printv("oller");
	    printv(" #%-3u", mc); printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");
	    printv("     "); printv("     ");

	    switch (Shm->Uncore.MC[mc].ChannelCount) {
	    case 1:
		printv("Singl"); printv("e Cha"); printv("nnel ");
		break;
	    case 2:
		printv(" Dual"); printv(" Chan"); printv("nel  ");
		break;
	    case 3:
		printv("Tripl"); printv("e Cha"); printv("nnel ");
		break;
	    case 4:
		printv(" Quad"); printv(" Chan"); printv("nel  ");
		break;
	    case 6:
		printv(" Hexa"); printv(" Chan"); printv("nel  ");
		break;
	    case 8:
		printv(" Octa"); printv(" Chan"); printv("nel  ");
		break;
	    default:
		printv("     "); printv("     "); printv("     ");
		break;
	    }

	    printv(" Bus "); printv("Rate ");
	    printv("%5llu", Shm->Uncore.Bus.Rate);
	    switch (Shm->Uncore.Unit.Bus_Rate) {
	    case 0b00:
		printv(" MHz ");
		break;
	    case 0b01:
		printv(" MT/s");
		break;
	    case 0b10:
		printv(" MB/s");
		break;
	    case 0b11:
		printv("     ");
		break;
	    }
	    printv("     ");

	    printv(" Bus "); printv("Speed");
	    printv("%5llu", Shm->Uncore.Bus.Speed);
	    switch (Shm->Uncore.Unit.BusSpeed) {
	    case 0b00:
		printv(" MHz ");
		break;
	    case 0b01:
		printv(" MT/s");
		break;
	    case 0b10:
		printv(" MB/s");
		break;
	    case 0b11:
		printv("     ");
		break;
	    }
	    printv("     ");

	    printv("DRAM "); printv("Speed");
	    printv("%5llu", Shm->Uncore.CtrlSpeed);
	    switch (Shm->Uncore.Unit.DDRSpeed) {
	    case 0b00:
		printv(" MHz ");
		break;
	    case 0b01:
		printv(" MT/s");
		break;
	    case 0b10:
		printv(" MB/s");
		break;
	    case 0b11:
		printv("     ");
		break;
	    }

	    printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");

	    printv(" Cha ");
	    printv("   CL");printv("  RCD");printv("   RP");printv("  RAS");
	    printv("  RRD");printv("  RFC");printv("   WR");printv(" RTPr");
	    printv(" WTPr");printv("  FAW");printv("  B2B");printv("  CWL");
	    printv(" Rate");

	    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
		printv("\x20\x20#%-2u", cha);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tCL);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRP);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tWR);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.B2B);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL);
		printv("%4uN",Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate);
	    }
	    printv("     ");
	    printv(" ddWR"); printv(" drWR"); printv(" srWR");
	    printv(" ddRW"); printv(" drRW"); printv(" srRW");
	    printv(" ddRR"); printv(" drRR"); printv(" srRR");
	    printv(" ddWW"); printv(" drWW"); printv(" srWW");
	    printv("  ECC");

	    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
		printv("\x20\x20#%-2u", cha);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd);

		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr);

		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd);

		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr);

		printv("%4u ", Shm->Uncore.MC[mc].Channel[cha].Timing.ECC);
	    }
	    printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");
	    printv("     "); printv("     "); printv("     "); printv("     ");

	    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	      printv(" DIMM"); printv(" Geom"); printv("etry ");printv("for c");
	      printv("hanne"); printv("l #%-2u", cha);
	      printv("     "); printv("     "); printv("     ");printv("     ");
	      printv("     "); printv("     "); printv("     ");printv("     ");

	      printv("     ");
	      printv(" Slot"); printv(" Bank"); printv(" Rank");
	      printv("     "); printv("Rows "); printv("  Col");printv("umns ");
	      printv("   Me"); printv("mory "); printv("Size ");printv("(MB) ");
	      printv("     "); printv("     ");

	      for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		printv("     ");
		printv("\x20\x20#%-2u", slot);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks);
		printv("%5u", Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows);
		printv("%5s", hInt[0]);
		printv("%5s", hInt[1]);
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols);
		printv("%5s", hInt[0]);
		printv("%5s", hInt[1]);
		printv("     ");
		iSplit(Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size);
		printv("%5s", hInt[0]);
		printv("%5s", hInt[1]);
		printv("     "); printv("     "); printv("     ");
	      }
	    }
	}
}

typedef struct {
	CUINT	col,
		row;
} Coordinate;

typedef struct {
	CSINT	horz,
		vert;
} CoordShift;

typedef struct {
	CUINT	wth,
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
#define HCK	{.fg = CYAN,	.bg = BLACK,	.bf = 1}
#define HWC	{.fg = WHITE,	.bg = CYAN,	.bf = 1}
#define _HWK	{.fg = WHITE,	.bg = BLACK,	.un = 1,	.bf = 1}
#define _HWB	{.fg = WHITE,	.bg = BLUE,	.un = 1,	.bf = 1}
#define _HKW	{.fg = BLACK,	.bg = WHITE,	.un = 1,	.bf = 1}
#define _HCK	{.fg = CYAN,	.bg = BLACK,	.un = 1,	.bf = 1}
#define LDK	{.fg = BLACK,	.bg = BLACK}
#define LKW	{.fg = BLACK,	.bg = WHITE}
#define LRK	{.fg = RED,	.bg = BLACK}
#define LYK	{.fg = YELLOW,	.bg = BLACK}
#define LBK	{.fg = BLUE,	.bg = BLACK}
#define LBW	{.fg = BLUE,	.bg = WHITE}
#define LCK	{.fg = CYAN,	.bg = BLACK}
#define LWK	{.fg = WHITE,	.bg = BLACK}
#define LWB	{.fg = WHITE,	.bg = BLUE}
#define LKC	{.fg = BLACK,	.bg = CYAN}
#define _LKW	{.fg = BLACK,	.bg = WHITE,	.un = 1}
#define _LBW	{.fg = BLUE,	.bg = WHITE,	.un = 1}
#define _LWK	{.fg = WHITE,	.bg = BLACK,	.un = 1}
#define _LCK	{.fg = CYAN,	.bg = BLACK,	.un = 1}

#define MAKE_TITLE_UNFOCUS	MakeAttr(BLACK, 0, BLUE, 1)
#define MAKE_TITLE_FOCUS	MakeAttr(WHITE, 0, CYAN, 1)
#define MAKE_BORDER_UNFOCUS	MakeAttr(BLACK, 0, BLUE, 1)
#define MAKE_BORDER_FOCUS	MakeAttr(WHITE, 0, BLUE, 1)
#define MAKE_SELECT_UNFOCUS	MakeAttr(WHITE, 0, BLACK, 0)
#define MAKE_SELECT_FOCUS	MakeAttr(BLACK, 0, CYAN, 0)
#define MAKE_PRINT_UNFOCUS	MakeAttr(WHITE, 0, BLACK, 0)
#define MAKE_PRINT_FOCUS	MakeAttr(WHITE, 0, BLACK, 1)
#define MAKE_PRINT_DROP		MakeAttr(BLACK, 0, WHITE, 0)

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
		CUINT	bottomRow;
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
		CUINT i;
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

Window *CreateWindow(	Layer *layer, unsigned long long id,
			CUINT width, CUINT height,
			CUINT oCol, CUINT oRow)
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
	if (win != NULL) {
		if (!IsDead(list))
			AppendWinList(win, list);
		else {
			// Dead head, now cycling
			win->prev = win;
			win->next = win;
		}
		SetHead(list, win);
	}
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

void PrintContent(Window *win, WinList *list, CUINT col, CUINT row)
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
	CUINT col, row;
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
	CUINT row;
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
  if(win->matrix.scroll.vert < (win->lazyComp.bottomRow - win->matrix.size.hth))
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
	CUINT maxVisibleCol=KMIN(MAX_WIDTH - 1,GetScreenSize().width)
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
	CUINT maxVisibleRow=KMIN(MAX_HEIGHT - 1,GetScreenSize().height)
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

enum VIEW {V_FREQ,V_INST,V_CYCLES,V_CSTATES,V_PACKAGE,V_TASKS,V_INTR,V_VOLTAGE};

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
		avgOrPC :  7-6,		// C-states average | % pkg states
		_pad	: 32-7;
	};
	enum VIEW view;
    } drawFlag = {
	.layout=0,
	.clear=0,
	.height=0,
	.width=0,
	.daemon=0,
	.taskVal=0,
	.avgOrPC=0,
	.view=V_FREQ
    };

    SCREEN_SIZE drawSize = {.width = 0, .height = 0};

    unsigned int cpu = 0,prevTopFreq = 0,digit[9],iClock = 0,ratioCount = 0,idx;
    unsigned long prevFreeRAM = 0;
    int prevTaskCount = 0;

    CUINT	_col, _row, loadWidth = 0;
    CUINT	MIN_HEIGHT = 0,
		TOP_UPPER_FIRST = 1 + TOP_HEADER_ROW,
		TOP_LOWER_FIRST = 2+ TOP_HEADER_ROW + Shm->Proc.CPU.Count,
		TOP_LOWER_LAST  = 2 + TOP_HEADER_ROW + 2 * Shm->Proc.CPU.Count,
		TOP_FOOTER_LAST = 2 + TOP_HEADER_ROW + TOP_FOOTER_ROW
				+ 2 * Shm->Proc.CPU.Count;

    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[LAST_BOOST],
	medianRatio=(minRatio + maxRatio) / 2, availRatio[MAX_BOOST]={minRatio};

    typedef char HBCLK[11 + 1];
    HBCLK *hBClk;

    char *buffer = NULL, *viewMask = NULL;

    Coordinate *cTask;

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
				win->matrix.origin.col + win->lazyComp.rowLen,
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
	Window *wMenu = CreateWindow(wLayer, id, 3, 10, 3, 0);
	if (wMenu != NULL) {
		Attribute sameAttr = {.fg = BLACK, .bg = WHITE, .bf = 0},
			voidAttr = {.value = 0},
			stopAttr[18] = {
				HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,
				HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW
			},
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
		StoreTCell(wMenu, SCANKEY_g,	" Pkg. cycles  [g] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_o,	" Perf. Monit. [o] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);

	    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		StoreTCell(wMenu, SCANKEY_x,	" Task Monitor [x] ", skeyAttr);
	    else
		StoreTCell(wMenu, SCANKEY_x,	" Task Monitor [x] ", stopAttr);

		StoreTCell(wMenu, SCANKEY_w,	" PowerThermal [w] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SCANKEY_q,	" Interrupts   [q] ", skeyAttr);
		StoreTCell(wMenu, SCANKEY_u,	" CPUID Dump   [u] ", skeyAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu,SCANKEY_SHIFT_v," Voltage      [V] ",skeyAttr);

	    if (Shm->Uncore.CtrlCount > 0)
		StoreTCell(wMenu,SCANKEY_SHIFT_m," Memory Ctrl  [M] ",skeyAttr);
	    else
		StoreTCell(wMenu,SCANKEY_SHIFT_m," Memory Ctrl  [M] ",stopAttr);

		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);
		StoreTCell(wMenu, SCANKEY_VOID,	"", voidAttr);

	    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		StoreTCell(wMenu, SCANKEY_k,	" Kernel       [k] ", skeyAttr);
	    else
		StoreTCell(wMenu, SCANKEY_k,	" Kernel       [k] ", stopAttr);

		StoreWindow(wMenu, .color[0].select, MakeAttr(BLACK,0,WHITE,0));
		StoreWindow(wMenu, .color[0].title, MakeAttr(BLACK,0,WHITE,0));
		StoreWindow(wMenu, .color[1].title, MakeAttr(BLACK,0,WHITE,1));

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
      Window *wSet = CreateWindow(wLayer, id, 2, 10, 8, TOP_HEADER_ROW + 3);
      if (wSet != NULL) {
	char	intervStr[16], tickStr[16], pollStr[16], experStr[16],
		cpuhpStr[16], pciRegStr[16], nmiRegStr[16];
	int intervLen = sprintf(intervStr, "%13uE6",
				Shm->Proc.SleepInterval),
	    tickLen = sprintf(tickStr, "%13uE6",
				Shm->Proc.SleepInterval*Shm->SysGate.tickReset),
	    pollLen = sprintf(pollStr, "%13ldE6",
				Shm->Proc.BaseSleep.tv_nsec / 1000000L),
	    experLen = sprintf(experStr, "[%3s]",
				enabled(Shm->Registration.Experimental)),
	    cpuhpLen = sprintf(cpuhpStr, "[%3s]",
				enabled(!(Shm->Registration.hotplug < 0))),
	    pciRegLen = sprintf(pciRegStr, "[%3s]",
				enabled(!(Shm->Registration.pci < 0))),
	    nmiRegLen = sprintf(nmiRegStr, "[%3s]",
				enabled(Shm->Registration.nmi));
	size_t appLen = strlen(Shm->AppName);

	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Daemon gate    ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Interval(ns)   ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Sys.Tick(ns)   ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Polling (ns)   ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " Experimental   ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " CPU Hot-Plug   ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, " PCI enablement ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

        StoreTCell(wSet, SCANKEY_NULL, " NMI registered ", MAKE_PRINT_FOCUS);
        StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);
	StoreTCell(wSet, SCANKEY_NULL, "                ", MAKE_PRINT_FOCUS);

	memcpy(&TCellAt(wSet, 1, 1).item[15 - appLen], Shm->AppName, appLen);
	memcpy(&TCellAt(wSet, 1, 2).item[15 - intervLen], intervStr, intervLen);
	memcpy(&TCellAt(wSet, 1, 3).item[15 - tickLen], tickStr, tickLen);
	memcpy(&TCellAt(wSet, 1, 4).item[15 - pollLen], pollStr, pollLen);
	memcpy(&TCellAt(wSet, 1, 5).item[15 - experLen], experStr, experLen);
	memcpy(&TCellAt(wSet, 1, 6).item[15 - cpuhpLen], cpuhpStr, cpuhpLen);
	memcpy(&TCellAt(wSet, 1, 7).item[15 - pciRegLen], pciRegStr, pciRegLen);
	memcpy(&TCellAt(wSet, 1, 8).item[15 - nmiRegLen], nmiRegStr, nmiRegLen);

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
		l = strlen(C[0]), v = strlen(COREFREQ_VERSION);

	Window *wAbout = CreateWindow(	wLayer, id,
					1, c + f,
					(drawSize.width-l)/2, TOP_HEADER_ROW+4);
	if (wAbout != NULL) {
		unsigned int i;

		for (i = 0; i < c; i++)
			StoreTCell(wAbout,SCANKEY_NULL, C[i], MAKE_PRINT_FOCUS);
		for (i = 0; i < f; i++)
			StoreTCell(wAbout,SCANKEY_NULL, F[i], MAKE_PRINT_FOCUS);

		size_t pos = strlen((char*) TCellAt(wAbout, 1, 5).item) - 2 - v;
		memcpy(&TCellAt(wAbout, 1, 5).item[pos], COREFREQ_VERSION, v);

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
			Attribute attr;
		} btn[];
	} *pBox = NULL;
	int cnt = 0;

	va_list ap;
	va_start(ap, button);
	ASCII *item = button;
	Attribute attr = va_arg(ap, Attribute);
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
		attr = va_arg(ap, Attribute);
		aKey = va_arg(ap, unsigned long long);
	    }
	} while (item != NULL) ;
	va_end(ap);

	Window *wBox = NULL;
	if (pBox != NULL) {
	    wBox = CreateWindow(wLayer, id,
				1, pBox->cnt,
				origin.col, origin.row);
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
	CoordSize matrixSize = {.wth = 1, .hth = 18};
	Coordinate winOrigin = {.col = 3, .row = TOP_HEADER_ROW + 1};
	CUINT winWidth = 74;
	void (*SysInfoFunc)	(SHM_STRUCT*, CUINT,
				void(*OutFunc)(unsigned long long, char*));
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
		matrixSize.hth = 6;
		winOrigin.col = 23;
		winOrigin.row = TOP_HEADER_ROW + 11;
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
		matrixSize.hth = 10;
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
		matrixSize.hth = 11;
		winOrigin.col = 4;
		winOrigin.row = TOP_HEADER_ROW + 8;
		SysInfoFunc = SysInfoKernel;
		title = " Kernel ";
		}
		break;
	}

	int pad = 0;

	Window *wSysInfo = CreateWindow(wLayer, id,
					matrixSize.wth, matrixSize.hth,
					winOrigin.col, winOrigin.row);

	void AddSysInfoCell(unsigned long long key, char *input)
	{
		pad++;
		StoreTCell(wSysInfo, key, input, MAKE_PRINT_FOCUS);
	}

	if (wSysInfo != NULL) {
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
					6, 2 + Shm->Proc.CPU.Count,
					1, TOP_HEADER_ROW + 3);
		wTopology->matrix.select.row = 2;

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

    Window *CreateMemCtrl(unsigned long long id)
    {
	unsigned short mc, cha, rows = 0;
	for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
		for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
			rows++;
	rows *= 2;
	if (rows > 0) {
	    Window *wIMC = CreateWindow(wLayer, id,
					14, rows + 11,
					1, TOP_HEADER_ROW + 2);
		wIMC->matrix.select.row = 4;

	    void AddMemoryControllerCell(char *input)
	    {
		StoreTCell(wIMC, SCANKEY_NULL, input, MAKE_PRINT_FOCUS);
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
				      33, TOP_HEADER_ROW+Shm->Proc.CPU.Count+2);
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
					1, TOP_HEADER_ROW+Shm->Proc.CPU.Count*2,
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

    void PrintWindowStack(void)
    {
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
    }

    void FreeAll(void)
    {
	DestroyAllWindows(&winList);

	free(hBClk);
	free(buffer);
	free(viewMask);
	free(cTask);

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
	buffer = malloc(10 * MAX_WIDTH); // 10 times for ANSI cursor string.
	viewMask = malloc((10 * MAX_WIDTH) * MAX_HEIGHT);

	const CoordSize layerSize = {
		.wth = MAX_WIDTH,
		.hth = MAX_HEIGHT
	};
	cTask = calloc(Shm->Proc.CPU.Count, sizeof(Coordinate));

	sLayer = calloc(1, sizeof(Layer));
	dLayer = calloc(1, sizeof(Layer));
	wLayer = calloc(1, sizeof(Layer));
	fuze   = calloc(1, sizeof(Layer));
	CreateLayer(sLayer, layerSize);
	CreateLayer(dLayer, layerSize);
	CreateLayer(wLayer, layerSize);
	CreateLayer(fuze, layerSize);
    }

    for (idx = 1; idx < MAX_BOOST; idx++)
	if (Shm->Proc.Boost[idx] != 0) {
		int sort = Shm->Proc.Boost[idx] - availRatio[ratioCount];
		if (sort < 0) {
			availRatio[ratioCount + 1] = availRatio[ratioCount];
			availRatio[ratioCount++]   = Shm->Proc.Boost[idx];
		}
		else if (sort > 0)
			availRatio[++ratioCount]   = Shm->Proc.Boost[idx];
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

		    switch (drawFlag.view) {
		    case V_FREQ:
		    case V_INST:
		    case V_CYCLES:
		    case V_CSTATES:
		    case V_TASKS:
		    case V_INTR:
		    case V_VOLTAGE:
			MIN_HEIGHT = (2 * Shm->Proc.CPU.Count) + TOP_HEADER_ROW
					+ TOP_SEPARATOR + TOP_FOOTER_ROW;
			break;
		    case V_PACKAGE:
			MIN_HEIGHT = Shm->Proc.CPU.Count + 8 + TOP_HEADER_ROW
					+ TOP_SEPARATOR + TOP_FOOTER_ROW;
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
	}
    }

  int Shortcut(SCANKEY *scan)
  {
	Attribute stateAttr[2] = {
		MakeAttr(WHITE, 0, BLACK, 0),
		MakeAttr(CYAN, 0, BLACK, 1)
	},
	blankAttr = MakeAttr(BLACK, 0, BLACK, 1),
	descAttr =  MakeAttr(CYAN, 0, BLACK, 0);
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
			(ASCII*)"          Clock Modulation          "
	};

    switch (scan->key) {
/*
    case SCANKEY_PLUS:
	{
	if (!RING_FULL(Shm->Ring))
	    RING_WRITE(Shm->Ring, COREFREQ_IOCTL_MACHINE, COREFREQ_TOGGLE_ON);
	}
	break;
    case SCANKEY_MINUS:
	{
	if (!RING_FULL(Shm->Ring))
	    RING_WRITE(Shm->Ring, COREFREQ_IOCTL_MACHINE, COREFREQ_TOGGLE_OFF);
	}
	break;
*/
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
    case SCANKEY_PERCENT:
	{
	drawFlag.avgOrPC = !drawFlag.avgOrPC;
	drawFlag.clear = 1;
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
	if (drawFlag.view == V_TASKS) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateSortByField(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
	break;
    case SCANKEY_c:
	{
	drawFlag.view = V_CYCLES;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
	}
	break;
    case SCANKEY_f:
	{
	drawFlag.view = V_FREQ;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
	}
	break;
    case SCANKEY_n:
	if (drawFlag.view == V_TASKS) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateTracking(scan->key), &winList);
		else
			SetHead(&winList, win);
	}
	break;
    case SCANKEY_g:
	{
	drawFlag.view = V_PACKAGE;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
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
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
	}
	break;
    case SCANKEY_l:
	{
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
    case SCANKEY_SHIFT_m:
	if (Shm->Uncore.CtrlCount > 0) {
		Window *win = SearchWinListById(scan->key, &winList);
		if (win == NULL)
			AppendWindow(CreateMemCtrl(scan->key),&winList);
		else
			SetHead(&winList, win);
	}
	break;
    case SCANKEY_q:
	{
	drawFlag.view = V_INTR;
	drawSize.height = 0;
	TrapScreenSize(SIGWINCH);
	}
	break;
    case SCANKEY_SHIFT_v:
	{
	drawFlag.view = V_VOLTAGE;
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
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		Shm->SysGate.trackTask = 0;
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
	    const unsigned int isEIST =
			(Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 3
	    }, select = {
		.col = 0,
		.row = isEIST ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " EIST ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[0], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isEIST], stateAttr[isEIST], BOXKEY_EIST_ON,
		stateStr[0][!isEIST], stateAttr[!isEIST], BOXKEY_EIST_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_EIST_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_EIST, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_EIST_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_EIST, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_C1E:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isC1E = (Shm->Proc.C1E == Shm->Proc.C1E_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 2
	    }, select = {
		.col = 0,
		.row = isC1E ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " C1E ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[1], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isC1E], stateAttr[isC1E], BOXKEY_C1E_ON,
		stateStr[0][!isC1E], stateAttr[!isC1E], BOXKEY_C1E_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_C1E_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C1E, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_C1E_ON:
	{
	    if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C1E, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_TURBO:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isTurbo =
			(Shm->Proc.TurboBoost == Shm->Proc.TurboBoost_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 4
	    }, select = {
		.col = 0,
		.row = isTurbo ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " Turbo ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[2], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isTurbo], stateAttr[isTurbo], BOXKEY_TURBO_ON,
		stateStr[0][!isTurbo], stateAttr[!isTurbo], BOXKEY_TURBO_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_TURBO_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_TURBO,COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_TURBO_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_TURBO, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_C1A:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isC1A = (Shm->Proc.C1A == Shm->Proc.C1A_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 5
	    }, select = {
		.col = 0,
		.row = isC1A ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " C1A ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[3], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isC1A], stateAttr[isC1A], BOXKEY_C1A_ON,
		stateStr[0][!isC1A], stateAttr[!isC1A], BOXKEY_C1A_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_C1A_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C1A, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_C1A_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C1A, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_C3A:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isC3A = (Shm->Proc.C3A == Shm->Proc.C3A_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 6
	    }, select = {
		.col = 0,
		.row = isC3A ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " C3A ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[4], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isC3A], stateAttr[isC3A], BOXKEY_C3A_ON,
		stateStr[0][!isC3A], stateAttr[!isC3A], BOXKEY_C3A_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
	case BOXKEY_C3A_OFF:
	    {
	    if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C3A, COREFREQ_TOGGLE_OFF);
	    }
	    break;
    case BOXKEY_C3A_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C3A, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_C1U:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isC1U = (Shm->Proc.C1U == Shm->Proc.C1U_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 7
	    }, select = {
		.col = 0,
		.row = isC1U ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " C1U ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[5], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isC1U], stateAttr[isC1U], BOXKEY_C1U_ON,
		stateStr[0][!isC1U], stateAttr[!isC1U], BOXKEY_C1U_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_C1U_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C1U, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_C1U_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C1U, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_C3U:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isC3U = (Shm->Proc.C3U == Shm->Proc.C3U_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 8
	    }, select = {
		.col = 0,
		.row = isC3U ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " C3U ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[6], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isC3U], stateAttr[isC3U], BOXKEY_C3U_ON,
		stateStr[0][!isC3U], stateAttr[!isC3U], BOXKEY_C3U_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_C3U_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C3U, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_C3U_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_C3U, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_PKGCST:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
		const CSINT thisCST[] = {5, 4, 3, 2, -1, -1, 1, 0}; // Row index
		const Coordinate origin = {
			.col = (drawSize.width - (44 - 17)) / 2,
			.row = TOP_HEADER_ROW + 2
		}, select = {
			.col = 0,
			.row = thisCST[Shm->Cpu[0].Query.CStateLimit] != -1 ?
				    thisCST[Shm->Cpu[0].Query.CStateLimit] : 0
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
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_PKGCST, newCST);
	}
	break;
    case BOXKEY_IOMWAIT:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isIORedir = (Shm->Cpu[0].Query.IORedir == 1);
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
	      stateStr[1][isIORedir], stateAttr[isIORedir], BOXKEY_IOMWAIT_ON,
	      stateStr[0][!isIORedir], stateAttr[!isIORedir],BOXKEY_IOMWAIT_OFF,
	      blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_IOMWAIT_OFF:
	{
	if (!RING_FULL(Shm->Ring))
	    RING_WRITE(Shm->Ring, COREFREQ_IOCTL_IOMWAIT, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_IOMWAIT_ON:
	{
	if (!RING_FULL(Shm->Ring))
	    RING_WRITE(Shm->Ring, COREFREQ_IOCTL_IOMWAIT, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_IORCST:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
		const CSINT thisCST[]={-1, -1, -1, 3, 2, -1, 1, 0}; // Row index
		const Coordinate origin = {
			.col = (drawSize.width - (44 - 17)) / 2,
			.row = TOP_HEADER_ROW + 3
		}, select = {
			.col = 0,
			.row = thisCST[Shm->Cpu[0].Query.CStateInclude] != -1 ?
				    thisCST[Shm->Cpu[0].Query.CStateInclude] : 0
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
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_IORCST, newCST);
	}
	break;
    case BOXKEY_ODCM:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	    {
	    const unsigned int isODCM = (Shm->Proc.ODCM == Shm->Proc.ODCM_Mask);
	    const Coordinate origin = {
		.col = (drawSize.width - strlen((char *) blankStr)) / 2,
		.row = TOP_HEADER_ROW + 6
	    }, select = {
		.col = 0,
		.row = isODCM ? 4 : 3
	    };
	    AppendWindow(CreateBox(scan->key, origin, select, " ODCM ",
		blankStr, blankAttr, SCANKEY_NULL,
		descStr[8], descAttr, SCANKEY_NULL,
		blankStr, blankAttr, SCANKEY_NULL,
		stateStr[1][isODCM], stateAttr[isODCM], BOXKEY_ODCM_ON,
		stateStr[0][!isODCM], stateAttr[!isODCM],BOXKEY_ODCM_OFF,
		blankStr, blankAttr, SCANKEY_NULL),
		&winList);
	    } else
		SetHead(&winList, win);
	}
	break;
    case BOXKEY_ODCM_OFF:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_ODCM, COREFREQ_TOGGLE_OFF);
	}
	break;
    case BOXKEY_ODCM_ON:
	{
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_ODCM, COREFREQ_TOGGLE_ON);
	}
	break;
    case BOXKEY_DUTYCYCLE:
	{
	Window *win = SearchWinListById(scan->key, &winList);
	if (win == NULL)
	  {
	  const CSINT maxCM = 7 << Shm->Cpu[0].PowerThermal.DutyCycle.Extended;
	  const Coordinate origin = {
		.col = (drawSize.width - (44 - 17)) / 2,
		.row = TOP_HEADER_ROW + 3
	  }, select = {
	  .col = 0,
	  .row = (Shm->Cpu[0].PowerThermal.DutyCycle.ClockMod >= 0)
	      && (Shm->Cpu[0].PowerThermal.DutyCycle.ClockMod <= maxCM) ?
			Shm->Cpu[0].PowerThermal.DutyCycle.ClockMod : 1
	  };
	  Window *wBox = NULL;
	  if (Shm->Cpu[0].PowerThermal.DutyCycle.Extended)
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
	(ASCII*)"           63.50%          ", stateAttr[0], BOXKEY_ODCM_DC05,
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
	if (!RING_FULL(Shm->Ring))
		RING_WRITE(Shm->Ring, COREFREQ_IOCTL_ODCM_DC, newDC);
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
	else
		return(-1);
    }
    return(0);
  }

    void PrintLCD(Layer *layer, unsigned int relativeFreq, double relativeRatio)
    {
	unsigned int lcdColor, j = 4;

	Dec2Digit(relativeFreq, digit);

	if (relativeRatio > medianRatio)
		lcdColor = RED;
	else if (relativeRatio > minRatio)
		lcdColor = YELLOW;
	else
		lcdColor = GREEN;

	do {
		int offset = (4 - j) * 3;

		LayerFillAt(layer, offset, 0,
				3, lcd[digit[9 - j]][0],
				MakeAttr(lcdColor, 0, BLACK, 1));
		LayerFillAt(layer, offset, 1,
				3, lcd[digit[9 - j]][1],
				MakeAttr(lcdColor, 0, BLACK, 1));
		LayerFillAt(layer, offset, 2,
				3, lcd[digit[9 - j]][2],
				MakeAttr(lcdColor, 0, BLACK, 1));
		j--;
	} while (j > 0) ;
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

    void Layout(Layer *layer)
    {
	struct FLIP_FLOP *Flop = NULL;
	const unsigned int
		isTurbo = (Shm->Proc.TurboBoost == Shm->Proc.TurboBoost_Mask),
		isEIST = (Shm->Proc.SpeedStep == Shm->Proc.SpeedStep_Mask),
		isC1E = (Shm->Proc.C1E == Shm->Proc.C1E_Mask),
		isC3A = (Shm->Proc.C3A == Shm->Proc.C3A_Mask),
		isC1A = (Shm->Proc.C1A == Shm->Proc.C1A_Mask),
		isC3U = (Shm->Proc.C3U == Shm->Proc.C3U_Mask),
		isC1U = (Shm->Proc.C1U == Shm->Proc.C1U_Mask);
	unsigned int processorHot = 0;
	CUINT col = 0, row = 0;
	size_t len;

	loadWidth = drawSize.width - LOAD_LEAD;

	LayerDeclare(12) hProc0 = {
		.origin = {.col = 12, .row = row}, .length = 12,
		.attr = {LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK},
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

	hProc1.code[2] = buffer[0];
	hProc1.code[3] = buffer[1];
	hProc1.code[5] = buffer[2];
	hProc1.code[6] = buffer[3];

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
		.attr ={LWK,LWK,LWK,LWK,LCK,LCK,LCK,LCK,		\
			LCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
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
	int bright = 0;
	for (idx = 0; idx < ratioCount; idx++) {
		char tabStop[] = "00";
		int hPos = availRatio[idx] * loadWidth / maxRatio;
		sprintf(tabStop, "%2.0f", availRatio[idx]);

		if (tabStop[0] != 0x20) {
			hLoad0.code[hPos + 2] =tabStop[0];
			hLoad0.attr[hPos + 2] =MakeAttr(CYAN, 0, BLACK, bright);
		}
		hLoad0.code[hPos + 3] = tabStop[1];
		hLoad0.attr[hPos + 3] = MakeAttr(CYAN, 0, BLACK, bright);

		bright = !bright;
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

	switch (drawFlag.view) {
	default:
	case V_FREQ:
	  {
	    LayerDeclare(77) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
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
	  }
	  break;
	case V_INST:
	  {
	    LayerDeclare(76) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
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
	  }
	  break;
	case V_INTR:
	case V_CYCLES:
	case V_CSTATES:
	case V_VOLTAGE:
	  {
	    LayerDeclare(73) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = 73,
		.attr ={HWK,						\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
			HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK
		},
		.code ={' ',						\
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
	case V_PACKAGE:
	  {
	  }
	  break;
	case V_TASKS:
	  {
	    LayerDeclare(MAX_WIDTH - LOAD_LEAD + 1) hMon0 = {
		.origin = {	.col = LOAD_LEAD - 1,
				.row = (row + Shm->Proc.CPU.Count + 1)
		},
		.length = MAX_WIDTH - LOAD_LEAD + 1,
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
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK		\
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
			' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
		}
	    };
	    LayerCopyAt(layer, hMon0.origin.col, hMon0.origin.row,
			hMon0.length, hMon0.attr, hMon0.code);

		cTask[cpu].col = LOAD_LEAD + 8;
		cTask[cpu].row = 2 + TOP_HEADER_ROW + cpu + Shm->Proc.CPU.Count;
	  }
	  break;
	}
	Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	if (Flop->Thermal.Trip && !processorHot) {
		processorHot = cpu;
	}
    } else {
	LayerAt(layer, attr, 1, row) =					\
		LayerAt(layer, attr, 1, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(BLUE, 0, BLACK, 0);
	LayerAt(layer, attr, 2, row) =					\
		LayerAt(layer, attr, 2, (1 + row + Shm->Proc.CPU.Count)) = \
			MakeAttr(BLUE, 0, BLACK, 0);

	LayerFillAt(layer, LOAD_LEAD, row,
			(drawSize.width - LOAD_LEAD), hSpace,
			MakeAttr(WHITE, 0, BLACK, 0));
	LayerFillAt(layer, (LOAD_LEAD - 1), (row + Shm->Proc.CPU.Count + 1),
			(drawSize.width - LOAD_LEAD + 1), hSpace,
			MakeAttr(WHITE, 0, BLACK, 0));
    }

	idx = Dec2Digit(Shm->Cpu[cpu].Clock.Hz, digit);

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

	switch (drawFlag.view) {
	default:
	case V_FREQ:
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
			.row = (row + Shm->Proc.CPU.Count +1)
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
			.row = (row + Shm->Proc.CPU.Count +1)
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
	    row += Shm->Proc.CPU.Count + 2;
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

	    row += Shm->Proc.CPU.Count + 2;
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

	    row += Shm->Proc.CPU.Count + 2;
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

	    row += Shm->Proc.CPU.Count + 2;
	  }
	  break;
	case V_INTR:
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

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	    row += Shm->Proc.CPU.Count + 2;
	  }
	  break;
	case V_PACKAGE:
	  {
	    LayerFillAt(layer, 0, row, drawSize.width,
		"------------ Cycles ---- State ---------"		\
		"------------------------ Cycles --------"		\
		"----------------------------------------"		\
		"------------",
			MakeAttr(WHITE, 0, BLACK, 0));
	    row++;
	    for (idx=0; idx < 8; idx++, row++) {
		LayerFillAt(layer, 0, row,
			drawSize.width, hSpace, MakeAttr(WHITE, 0, BLACK, 0));
	    }
	    LayerFillAt(layer, 0, row,
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	    row ++;
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

	    LayerDeclare(21) hTask1 = {
		.origin = {.col = 23, .row = row},
		.length = 21,
	    };

	    struct {
		Attribute attr[21];
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
			.col = drawSize.width - 34,
			.row = (row + Shm->Proc.CPU.Count + 1)
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

	    LayerDeclare(22) hTrack0 = {
		.origin = {
			.col = 55,
			.row = row
		},
		.length = 22,
		.attr = {
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,		\
			LWK,LWK,LWK,LWK,LWK,HDK,LWK, LWK,LWK,		\
			LWK,LWK,HDK,LWK
		},
		.code = {
			' ','T','r','a','c','k','i', 'n','g',		\
			' ','P','I','D',' ','[',' ', 'O','F',		\
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

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine, MakeAttr(WHITE, 0, BLACK, 0));

	    LayerCopyAt(layer, hTask2.origin.col, hTask2.origin.row,
			hTask2.length, hTask2.attr, hTask2.code);

	    LayerCopyAt(layer, hTask3.origin.col, hTask3.origin.row,
			hTask3.length, hTask3.attr, hTask3.code);

	    LayerCopyAt(layer, hTrack0.origin.col, hTrack0.origin.row,
			hTrack0.length, hTrack0.attr, hTrack0.code);

	    row += Shm->Proc.CPU.Count + 2;
	  }
	  break;
	case V_VOLTAGE:
	  {
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
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK, \
			LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
		},
		.code = "--- Freq(MHz) - VID - Vcore ------------"	\
			"----------------------------------------"	\
			"----------------------------------------"	\
			"------------",
	    };
	    LayerCopyAt(layer, hVolt0.origin.col, hVolt0.origin.row,
			hVolt0.length, hVolt0.attr, hVolt0.code);

	    LayerFillAt(layer, 0, (row + Shm->Proc.CPU.Count + 1),
			drawSize.width, hLine,
			MakeAttr(WHITE, 0, BLACK, 0));

	    row += Shm->Proc.CPU.Count + 2;
	  }
	  break;
	}

	LayerDeclare(61) hTech0 = {
		.origin = {.col = 0, .row = row}, .length = 14,
		.attr={LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK},
		.code={'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',','},
	};

	const Attribute Pwr[] = {
		MakeAttr(BLACK, 0, BLACK, 1),
		MakeAttr(GREEN, 0, BLACK, 1),
		MakeAttr(BLUE,  0, BLACK, 1)
	};
	const struct { ASCII *code; Attribute attr; } TSC[] = {
		{(ASCII *) "  TSC  ",  MakeAttr(BLACK, 0, BLACK, 1)},
		{(ASCII *) "TSC-VAR" , MakeAttr(BLUE,  0, BLACK, 1)},
		{(ASCII *) "TSC-INV" , MakeAttr(GREEN, 0, BLACK, 1)}
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

	    hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[6] = hTech1.attr[7] =
								Pwr[isEIST];

	    hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11] =
		Pwr[Shm->Proc.Features.Power.AX.TurboIDA];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =
	    hTech1.attr[16] = hTech1.attr[17] = Pwr[isTurbo];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] = Pwr[isC1E];

	    sprintf(buffer, "PM%1d", Shm->Proc.PM_version);

	    hTech1.code[23] = buffer[0];
	    hTech1.code[24] = buffer[1];
	    hTech1.code[25] = buffer[2];

	    hTech1.attr[23] = hTech1.attr[24] = hTech1.attr[25] =
		Pwr[(Shm->Proc.PM_version > 0)];

	    hTech1.attr[27] = hTech1.attr[28] = hTech1.attr[29] = Pwr[isC3A];

	    hTech1.attr[31] = hTech1.attr[32] = hTech1.attr[33] = Pwr[isC1A];

	    hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] = Pwr[isC3U];

	    hTech1.attr[39] = hTech1.attr[40] = hTech1.attr[41] = Pwr[isC1U];

	    hTech1.attr[43] = hTech1.attr[44] = hTech1.attr[45] =
		TM1[Shm->Cpu[0].PowerThermal.TM1];

	    hTech1.attr[47] = hTech1.attr[48] = hTech1.attr[49] =
		TM2[Shm->Cpu[0].PowerThermal.TM2];

	    if (processorHot) {
		hTech1.attr[51] = hTech1.attr[52] = hTech1.attr[53] =
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
	  if (!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12)) {
	    LayerDeclare(65) hTech1 = {
		.origin={.col=hTech0.length, .row=hTech0.origin.row},.length=39,
		.attr={HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK,\
		       HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,\
		       HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK
		},
		.code={'H','T','T',',','P','o','w','e','r','N','o','w',',',\
		       'T','U','R','B','O',',','C','1','E',',',' ','P','M',',',\
		       'D','T','S',',','T','T','P',',','H','O','T',']'
		},
	    };

	    hTech1.attr[0] = hTech1.attr[1] = hTech1.attr[2] =
		Pwr[Shm->Proc.HyperThreading];

	    hTech1.attr[4] = hTech1.attr[5] = hTech1.attr[ 6] = hTech1.attr[ 7]=
	    hTech1.attr[8] = hTech1.attr[9] = hTech1.attr[10] = hTech1.attr[11]=
		Pwr[(Shm->Proc.PowerNow == 0b11)];

	    hTech1.attr[13] = hTech1.attr[14] = hTech1.attr[15] =
	    hTech1.attr[16] = hTech1.attr[17] = Pwr[isTurbo];

	    hTech1.attr[19] = hTech1.attr[20] = hTech1.attr[21] = Pwr[isEIST];

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

	    if (processorHot) {
		hTech1.attr[35] = hTech1.attr[36] = hTech1.attr[37] =
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

		if ((len = strlen(Shm->SysGate.IdleDriver.Name)) > 0) {
		  LayerAt(layer, attr, col, row) = MakeAttr(BLACK, 0, BLACK, 1);
		  LayerAt(layer, code, col, row) = '/';

		  col++;

		  LayerFillAt(	layer, col, row,
				len, Shm->SysGate.IdleDriver.Name,
				MakeAttr(WHITE, 0, BLACK, 1));
		  col += len;
		}
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

	Flop=&Shm->Cpu[Shm->Proc.Top].FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];
	// Reset the Top Frequency
	PrintLCD(layer,(unsigned int)Flop->Relative.Freq,Flop->Relative.Ratio);

	// Reset Tasks count & Memory usage
	if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		PrintTaskMemory(layer, row,
				Shm->SysGate.taskCount,
				Shm->SysGate.memInfo.freeram,
				Shm->SysGate.memInfo.totalram);

	// Clear garbage in bottom screen
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
	struct FLIP_FLOP *Flop = NULL;

	do {
	  SCANKEY scan = {.key = 0};

	  if ((drawFlag.daemon = BITVAL(Shm->Proc.Sync, 0)) == 0) {
	    if (GetKey(&scan, &Shm->Proc.BaseSleep) > 0) {
		if (Shortcut(&scan) == -1) {
		  if (IsDead(&winList))
			AppendWindow(CreateMenu(SCANKEY_F2), &winList);
		  else
		    if (Motion_Trigger(&scan,GetFocus(&winList),&winList) > 0)
			Shortcut(&scan);
		}
		PrintWindowStack();

		break;
	    }
	  } else {
		BITCLR(LOCKLESS, Shm->Proc.Sync, 0);
	  }
	  if (BITVAL(Shm->Proc.Sync, 63)) {
		// Platform changed, redraw the layout.
		drawFlag.layout = 1;
		BITCLR(LOCKLESS, Shm->Proc.Sync, 63);
	  }
	} while (!Shutdown && !drawFlag.daemon && !drawFlag.layout) ;

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
		Flop = &Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!Shm->Cpu[cpu].OffLine.OS)
		{ // Upper view area
			CUINT	bar0=(Flop->Relative.Ratio *loadWidth)/maxRatio,
				bar1 = loadWidth - bar0,
				row = TOP_UPPER_FIRST + cpu;
		// Print the Per Core BCLK indicator (yellow)
		    LayerAt(dLayer, code, LOAD_LEAD - 1, row) = (cpu == iClock)?
								'~' : 0x20;
		// Draw the relative Core frequency ratio
		    LayerFillAt(dLayer, LOAD_LEAD, row,
				bar0, hBar,
				MakeAttr((Flop->Relative.Ratio > medianRatio ?
					RED : Flop->Relative.Ratio > minRatio ?
					YELLOW : GREEN),
					0, BLACK, 1));
		// Pad with blank characters
		    LayerFillAt(dLayer, (bar0 + LOAD_LEAD), row,
				bar1, hSpace,
				MakeAttr(BLACK, 0, BLACK, 1));

		// Lower view area
		    row = TOP_LOWER_FIRST + cpu;

		    switch (drawFlag.view) {
		    default:
		    case V_FREQ:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD, row),
				"%7.2f" " (" "%5.2f" ") "		\
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% " \
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%  " \
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

			Attribute warning ={.fg=WHITE, .un=0, .bg=BLACK, .bf=1};

			if (Flop->Thermal.Temp <=
				Shm->Cpu[cpu].PowerThermal.Limit[0])
					warning = MakeAttr(BLUE, 0, BLACK, 1);
			else {
				if (Flop->Thermal.Temp >=
				    Shm->Cpu[cpu].PowerThermal.Limit[1])
					warning = MakeAttr(YELLOW, 0, BLACK, 0);
			}
			if (Flop->Thermal.Trip) {
				warning = MakeAttr(RED, 0, BLACK, 1);
			}
			LayerAt(dLayer, attr, LOAD_LEAD + 69, row) =
			LayerAt(dLayer, attr, LOAD_LEAD + 70, row) =
			LayerAt(dLayer, attr, LOAD_LEAD + 71, row) = warning;
		      }
		      break;
		    case V_INST:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD, row),
				"%17.6f" "/s"				\
				"%17.6f" "/c"				\
				"%17.6f" "/i"				\
				"%18llu",
				Flop->State.IPS,
				Flop->State.IPC,
				Flop->State.CPI,
				Flop->Delta.INST);
		      }
		      break;
		    case V_CYCLES:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD, row),
				"%18llu%18llu%18llu%18llu",
				Flop->Delta.C0.UCC,
				Flop->Delta.C0.URC,
				Flop->Delta.C1,
				Flop->Delta.TSC);
		      }
		      break;
		    case V_CSTATES:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD, row),
				"%18llu%18llu%18llu%18llu",
				Flop->Delta.C1,
				Flop->Delta.C3,
				Flop->Delta.C6,
				Flop->Delta.C7);
		      }
		      break;
		    case V_PACKAGE:
		      {
		      }
		      break;
		    case V_TASKS:
		      {
			size_t len;

			sprintf((char *) &LayerAt(dLayer, code, LOAD_LEAD, row),
				"%7.2f",
				Flop->Relative.Freq);

			if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
				CSINT pos;
				char symbol;
				Attribute runColor[] = {
					HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
					HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
					HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
					HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,\
					HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK
				}, unintColor[] = {
					LYK,LYK,LYK,LYK,LYK,LYK,LYK,LYK,\
					LYK,LYK,LYK,LYK,LYK,LYK,LYK,LYK,\
					LYK,LYK,LYK,LYK,LYK,LYK,LYK,LYK,\
					LYK,LYK,LYK,LYK,LYK,LYK,LYK,LYK,\
					LYK,LYK,LYK,LYK,LYK,LYK,LYK,LYK
				}, zombieColor[] = {
					LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,\
					LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,\
					LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,\
					LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,\
					LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW
				}, sleepColor[] = {
					LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
					LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
					LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
					LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
					LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK
				}, otherColor[] = {
					HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
					HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
					HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
					HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
					HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK
				}, trackerColor[] = {
					LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
					LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
					LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
					LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,\
					LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC
				}, *attr;

				cTask[cpu].col = LOAD_LEAD + 8;

				LayerFillAt(dLayer,
					cTask[cpu].col,
					cTask[cpu].row,
					(drawSize.width - LOAD_LEAD - 8),
					hSpace,
					MakeAttr(WHITE, 0, BLACK, 0));

			  for (idx = 0; idx < Shm->SysGate.taskCount; idx++) {
				switch (Shm->SysGate.taskList[idx].state) {
				case 0: {	// TASK_RUNNING
					attr = runColor;
					symbol = 'R';
					}
					break;
				case 1: {	// TASK_INTERRUPTIBLE
					attr = sleepColor;
					symbol = 'S';
					}
					break;
				case 2: {	// TASK_UNINTERRUPTIBLE
					attr = unintColor;
					symbol = 'U';
					}
					break;
				case 4: {	// TASK_ZOMBIE
					attr = zombieColor;
					symbol = 'Z';
					}
					break;
				case 8: {	// TASK_STOPPED
					attr = sleepColor;
					symbol = 'H';
					}
					break;
				default: {
					attr = otherColor;
					symbol = 'O';
					}
					break;
				}
			    if (Shm->SysGate.taskList[idx].pid ==
							Shm->SysGate.trackTask)
			    {
				attr = trackerColor;
			    }
			    if (!drawFlag.taskVal) {
				len = sprintf(buffer, "%s",
					Shm->SysGate.taskList[idx].comm);
			    } else {
			      switch (Shm->SysGate.sortByField) {
			      case F_STATE:
				len = sprintf(buffer, "%s(%c)",
					Shm->SysGate.taskList[idx].comm,
					symbol);
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
			      default:
				len = sprintf(buffer, "%s(%d)",
					Shm->SysGate.taskList[idx].comm,
					Shm->SysGate.taskList[idx].pid);
				break;
			      }
			    }
			    pos =drawSize.width
				-cTask[Shm->SysGate.taskList[idx].wake_cpu].col;
			    if (pos >= 0) {
			      LayerCopyAt(dLayer,
				cTask[Shm->SysGate.taskList[idx].wake_cpu].col,
				cTask[Shm->SysGate.taskList[idx].wake_cpu].row,
				(pos > len ? len : pos),
				attr,
				buffer);

			      cTask[Shm->SysGate.taskList[idx].wake_cpu].col +=
									len + 2;
			    }
			  }
			}
		      }
		      break;
		    case V_INTR:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD, row),
				"%10u", Flop->Counter.SMI);
		      if (Shm->Registration.nmi)
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD +24,row),
				"%10u%10u%10u%10u",
				Flop->Counter.NMI.LOCAL,
				Flop->Counter.NMI.UNKNOWN,
				Flop->Counter.NMI.PCISERR,
				Flop->Counter.NMI.IOCHECK);
		      }
		      break;
		    case V_VOLTAGE:
		      {
			sprintf((char *)&LayerAt(dLayer,code,LOAD_LEAD, row),
				"%7.2f "				\
				"%7d   %5.4f",
				Flop->Relative.Freq,
				Flop->Voltage.VID,
				Flop->Voltage.Vcore);
		      }
		      break;
		    }
		}
	    }
	  }

	  switch (drawFlag.view) {
	  case V_FREQ:
	    {
		_row = TOP_LOWER_LAST;
		if (!drawFlag.avgOrPC)
			sprintf((char *)&LayerAt(dLayer, code, 20, _row),
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%% "\
				"%6.2f" "%% " "%6.2f" "%% " "%6.2f" "%%",
				100.f * Shm->Proc.Avg.Turbo,
				100.f * Shm->Proc.Avg.C0,
				100.f * Shm->Proc.Avg.C1,
				100.f * Shm->Proc.Avg.C3,
				100.f * Shm->Proc.Avg.C6,
				100.f * Shm->Proc.Avg.C7);
		else
			sprintf((char *)&LayerAt(dLayer, code, 11, _row),
				"  c2:%-5.1f" "  c3:%-5.1f" "  c6:%-5.1f"\
				"  c7:%-5.1f" "  c8:%-5.1f" "  c9:%-5.1f"\
				" c10:%-5.1f",
				100.f * Shm->Proc.State.PC02,
				100.f * Shm->Proc.State.PC03,
				100.f * Shm->Proc.State.PC06,
				100.f * Shm->Proc.State.PC07,
				100.f * Shm->Proc.State.PC08,
				100.f * Shm->Proc.State.PC09,
				100.f * Shm->Proc.State.PC10);
	    }
	    // fallthrough
	  default:  // V_INST, V_CYCLES, V_CSTATES, V_TASKS, V_TASKS, V_VOLTAGE
		_row = TOP_FOOTER_LAST;
	    break;
	  case V_PACKAGE:
	    {
		struct PKG_FLIP_FLOP *Pkg =
				&Shm->Proc.FlipFlop[!Shm->Proc.Toggle];

		_row = TOP_LOWER_FIRST;
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC02:%18llu" "%7.2f%%",
			Pkg->Delta.PC02, 100.f * Shm->Proc.State.PC02);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC03:%18llu" "%7.2f%%",
			Pkg->Delta.PC03, 100.f * Shm->Proc.State.PC03);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC06:%18llu" "%7.2f%%",
			Pkg->Delta.PC06, 100.f * Shm->Proc.State.PC06);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC07:%18llu" "%7.2f%%",
			Pkg->Delta.PC07, 100.f * Shm->Proc.State.PC07);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC08:%18llu" "%7.2f%%",
			Pkg->Delta.PC08, 100.f * Shm->Proc.State.PC08);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC09:%18llu" "%7.2f%%",
			Pkg->Delta.PC09, 100.f * Shm->Proc.State.PC09);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row++),
			"PC10:%18llu" "%7.2f%%",
			Pkg->Delta.PC10, 100.f * Shm->Proc.State.PC10);
		sprintf((char *)&LayerAt(dLayer, code, 0, _row),
			" TSC:%18llu", Pkg->Delta.PTSC);
		sprintf((char *)&LayerAt(dLayer, code, 50, _row++),
			"UNCORE:%18llu", Pkg->Uncore.FC0);

		_row += 2;
	    }
	    break;
	  }
	// Footer view area
	  if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)
	  && (Shm->SysGate.tickStep == Shm->SysGate.tickReset)) {
		if ((prevTaskCount != Shm->SysGate.taskCount)
		 || (prevFreeRAM != Shm->SysGate.memInfo.freeram)) {
			prevTaskCount = Shm->SysGate.taskCount;
			prevFreeRAM = Shm->SysGate.memInfo.freeram;

			PrintTaskMemory(dLayer, _row,
					Shm->SysGate.taskCount,
					Shm->SysGate.memInfo.freeram,
					Shm->SysGate.memInfo.totalram);
		}
	  }
	// Header view area
	Flop=&Shm->Cpu[Shm->Proc.Top].FlipFlop[!Shm->Cpu[Shm->Proc.Top].Toggle];
	// Print the Top Frequency
	  unsigned int relativeTopFreq = (unsigned int) Flop->Relative.Freq;
	  if (prevTopFreq != relativeTopFreq) {
		prevTopFreq = relativeTopFreq;
		PrintLCD(dLayer, relativeTopFreq, Flop->Relative.Ratio);
	  }
	// Print the focus BCLK
	  memcpy(&LayerAt(dLayer, code, 26, 2), hBClk[iClock], 11);
	// Increment the BCLK indicator (skip offline CPU)
	  do {
		iClock++;
		if (iClock == Shm->Proc.CPU.Count)
			iClock = 0;
	  } while (Shm->Cpu[iClock].OffLine.OS && iClock) ;

	} // endif (drawFlag.daemon)

	// Fuse all layers
	Attribute attr = {.value = 0};
	idx = 0;

	for (_row = 0; _row < drawSize.height; _row++)
	{
	  struct {
		int cursor;
	  } flag = {0};

	  int _bix = 0, _bdx;
	  CUINT _wth = _row * fuze->size.wth;

	  for (_col = 0; _col < drawSize.width; _col++) {
	    int _idx = _col + _wth;
	    Attribute	*fa =   &fuze->attr[_idx],
			*sa = &sLayer->attr[_idx],
			*da = &dLayer->attr[_idx],
			*wa = &wLayer->attr[_idx];
	    ASCII	*fc =   &fuze->code[_idx],
			*sc = &sLayer->code[_idx],
			*dc = &dLayer->code[_idx],
			*wc = &wLayer->code[_idx];
	/* STATIC LAYER */
	    if (sa->value != 0)
		fa->value = sa->value;
	    if (*sc != 0)
		*fc = *sc;
	/* DYNAMIC LAYER */
	    if (da->value != 0)
		fa->value = da->value;
	    if (*dc != 0)
		*fc = *dc;
	/* WINDOWS LAYER */
	    if (wa->value != 0)
		fa->value = wa->value;
	    if (*wc != 0)
		*fc = *wc;
	/* FUZED LAYER */
	    if((fa->fg ^ attr.fg) || (fa->bg ^ attr.bg) || (fa->bf ^ attr.bf)) {
		buffer[_bix++] = 0x1b;
		buffer[_bix++] = '[';
		buffer[_bix++] = '0' + fa->bf;
		buffer[_bix++] = ';';
		buffer[_bix++] = '3';
		buffer[_bix++] = '0' + fa->fg;
		buffer[_bix++] = ';';
		buffer[_bix++] = '4';
		buffer[_bix++] = '0' + fa->bg;
		buffer[_bix++] = 'm';
	    }
	    if (fa->un ^ attr.un) {
		buffer[_bix++] = 0x1b;
		buffer[_bix++] = '[';
		if (fa->un) {
			buffer[_bix++] = '4';
			buffer[_bix++] = 'm';
		} else {
			buffer[_bix++] = '2';
			buffer[_bix++] = '4';
			buffer[_bix++] = 'm';
		}
	    }
	    attr.value = fa->value;

	    if (*fc != 0) {
		if (flag.cursor == 0) {
			flag.cursor = 1;

			struct {
				CUINT col, row;
			} scr = {.col = _col + 1, .row = _row + 1};

			buffer[_bix++] = 0x1b;
			buffer[_bix++] = '[';

			_bix = log10(scr.row) + _bix + 1;
			for(_bdx = _bix; scr.row > 0; scr.row /= 10)
				buffer[--_bdx] = '0' + (scr.row % 10);

			buffer[_bix++] = ';';

			_bix = log10(scr.col) + _bix + 1;
			for(_bdx = _bix; scr.col > 0; scr.col /= 10)
				buffer[--_bdx] = '0' + (scr.col % 10);

			buffer[_bix++] = 'H';
		}
		buffer[_bix++] = *fc;
	    }
	    else
		flag.cursor = 0;
	  }
	  memcpy(&viewMask[idx], buffer, _bix);
	  idx += _bix;
	}
	// Write buffering to the standard output
	fwrite(viewMask, idx, 1, stdout);
	fflush(stdout);
      } // endif (drawFlag.height & drawFlag.width)
      else
	printf( CUH RoK "Term(%u x %u) < View(%u x %u)\n",
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
		"\t-V\tMonitor Voltage\n"				\
		"\t-c\tMonitor Counters\n"				\
		"\t-i\tMonitor Instructions\n"				\
		"\t-s\tPrint System Information\n"			\
		"\t-M\tPrint Memory Controller\n"			\
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
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

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
		printv(NULL, SCANKEY_VOID, 80, 0,"");
		printv(NULL, SCANKEY_VOID, 80, 0,"Features:");
		SysInfoFeatures(Shm, 80, NULL);
		printv(NULL, SCANKEY_VOID, 80, 0,"");
		printv(NULL, SCANKEY_VOID, 80, 0,"Technologies:");
		SysInfoTech(Shm, 80, NULL);
		printv(NULL, SCANKEY_VOID, 80, 0,"");
		printv(NULL, SCANKEY_VOID, 80, 0,"Performance Monitoring:");
		SysInfoPerfMon(Shm, 80, NULL);
		printv(NULL, SCANKEY_VOID, 80, 0,"");
		printv(NULL, SCANKEY_VOID, 80, 0,"Power & Thermal Monitoring:");
		SysInfoPwrThermal(Shm, 80, NULL);
		}
		break;
	case 'm':
		Topology(Shm, NULL);
		break;
	case 'M':
		MemoryController(Shm, NULL);
		break;
	case 'i':
		TrapSignal();
		Instructions(Shm);
		break;
	case 'c':
		TrapSignal();
		Counters(Shm);
		break;
	case 'V':
		TrapSignal();
		Voltage(Shm);
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
