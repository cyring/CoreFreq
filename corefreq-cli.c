/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

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
			Shutdown=0x1;
		break;
	}
}

// VT100 requirements.
#define SCP	"\033[s"
#define RCP	"\033[u"
#define HIDE	"\033[?25l"
#define SHOW	"\033[?25h"
#define CLS	"\033[H\033[J"
#define DoK	"\033[1;30;40m"
#define RoK	"\033[1;31;40m"
#define GoK	"\033[1;32;40m"
#define YoK	"\033[1;33;40m"
#define BoK	"\033[1;34;40m"
#define MoK	"\033[1;35;40m"
#define CoK	"\033[1;36;40m"
#define WoK	"\033[1;37;40m"

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

char hSpace[]=	"        ""        ""        ""        ""        "\
		"        ""        ""        ""        ""        "\
		"        ""        ""        ""        ""        "\
		"        ""    ";
char hBar[]=	"||||||||""||||||||""||||||||""||||||||""||||||||"\
		"||||||||""||||||||""||||||||""||||||||""||||||||"\
		"||||||||""||||||||""||||||||""||||||||""||||||||"\
		"||||||||""||||";
char hLine[]=	"--------""--------""--------""--------""--------"\
		"--------""--------""--------""--------""--------"\
		"--------""--------""--------""--------""--------"\
		"--------""----";

typedef struct
{
	int	width,
		height;
} SCREEN_SIZE;

SCREEN_SIZE getScreenSize(void)
{
	SCREEN_SIZE _screenSize={.width=0, .height=0};
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	_screenSize.width=(int) ts.ws_col;
	_screenSize.height=(int) ts.ws_row;
	return(_screenSize);
}

unsigned int dec2Digit(unsigned int decimal, unsigned int thisDigit[])
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

void cursorXY(unsigned int X, unsigned int Y, char *thisCursor)
{
	sprintf(thisCursor, "\033[%d;%dH", Y, X);
}

void lcdDraw(	unsigned int X,
		unsigned int Y,
		char *thisView,
		char *thisCursor,
		unsigned int thisNumber,
		unsigned int thisDigit[] )
{
    char *lcdBuf=malloc(32);
    thisView[0]='\0';

    unsigned int j=dec2Digit(thisNumber, thisDigit);
    j=4;
    do
    {
	int offset=X + (4 - j) * 3;
	cursorXY(offset, Y, thisCursor);
	sprintf(lcdBuf, "%s%.*s", thisCursor, 3, lcd[thisDigit[9 - j]][0]);
	strcat(thisView, lcdBuf);
	cursorXY(offset, Y + 1, thisCursor);
	sprintf(lcdBuf, "%s%.*s", thisCursor, 3, lcd[thisDigit[9 - j]][1]);
	strcat(thisView, lcdBuf);
	cursorXY(offset, Y + 2, thisCursor);
	sprintf(lcdBuf, "%s%.*s", thisCursor, 3, lcd[thisDigit[9 - j]][2]);
	strcat(thisView, lcdBuf);

	j--;
    } while(j > 0) ;
    free(lcdBuf);
}


void Top(SHM_STRUCT *Shm)
{
/*
           SCREEN
.--------------------------.
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
'--------------------------'
*/

    char hLoad[]=	"--------""----- CP""U Ratio ""--------""--------"\
			"--------""--------""--------""--------""--------"\
			"--------""--------""--------""--------""--------"\
			"--------""----";
    char hMon[]=	"---- Fre""quency -"" Ratio -"" Turbo -""- C0 ---"\
			"- C1 ---""- C3 ---""- C6 ---""- C7 ---"" Temps -"\
			"--------""--------""--------""--------""--------"\
			"--------""----";
    char hAvg[]=	"--------""---- Ave""rages [ ";

    char *hRatio=NULL,
	 *hProc=NULL,
	 *hArch=NULL,
	 *hBClk=NULL,
	 *hCore=NULL,
	 *hFeat=NULL,
	 *headerView=NULL,
	 *footerView=NULL,
	 *loadView=NULL,
	 *monitorView=NULL,
	 *lcdView=NULL;

    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[9];
    double medianRatio=(minRatio + maxRatio) / 2;
    double availRatio[10]={minRatio};
    unsigned int cpu=0, iclk=0, i, ratioCount=0;

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

    #define MAX_WIDTH	132
    #define MIN_WIDTH	80
    #define LOAD_LEAD	4

    SCREEN_SIZE drawSize={.width=0, .height=0};
    int MIN_HEIGHT=(2 * Shm->Proc.CPU.Count) + 7;
    int loadWidth=0;
    int drawFlag=0x0;

    void layout(void)
    {
	char *hString=malloc(32);

	loadWidth=drawSize.width - LOAD_LEAD;

	sprintf(hRatio, "%.*s",
		drawSize.width,
		hLoad);
	for(i=0; i < ratioCount; i++)
	{
		char tabStop[]="00";
		int hPos=availRatio[i] * loadWidth / maxRatio;
		sprintf(tabStop, "%2.0f", availRatio[i]);
		hRatio[hPos+2]=tabStop[0];
		hRatio[hPos+3]=tabStop[1];
	}

	sprintf(hProc,
	    DoK	"%.*sProcessor ["CoK"%s"DoK"]"				\
		"%.*s"							\
	    WoK	"%2u"DoK"/"WoK"%-2u"DoK"CPU Online ",
		14, hSpace,
		Shm->Proc.Brand,
		drawSize.width - 42 - strlen(Shm->Proc.Brand), hSpace,
		Shm->Proc.CPU.OnLine,
		Shm->Proc.CPU.Count);

	sprintf(hArch,
	    DoK	"%.*sArchitecture ["CoK"%s"DoK"]",
		14, hSpace,
		Shm->Proc.Architecture);

	sprintf(headerView,
		"%s\n"							\
		"%s%.*s"						\
		"Caches "						\
		"L1 Inst="WoK"%-3u"DoK"Data="WoK"%-3u"DoK"KB\n"		\
		"%.*sBase Clock ~ 000 000 000 Hz%.*s"			\
		"L2="WoK"%-4u"DoK" L3="WoK"%-5u"DoK"KB\n",
		hProc,
		hArch,
		drawSize.width - 57 - strlen(Shm->Proc.Architecture), hSpace,
		Shm->Cpu[0].Topology.Cache[0].Size / 1024,
		Shm->Cpu[0].Topology.Cache[1].Size / 1024,
		14, hSpace,
		drawSize.width - 59, hSpace,
		Shm->Cpu[0].Topology.Cache[2].Size / 1024,
		Shm->Cpu[0].Topology.Cache[3].Size / 1024);

	if(Shm->Proc.PM_version == 0)
		strcpy(hString, " PM");
	else
		sprintf(hString, GoK"PM%1d"DoK, Shm->Proc.PM_version);

	const char *TSC[]=
	{
		"  TSC  ",
	    GoK	"TSC-VAR" DoK,
	    GoK	"TSC-INV" DoK
	};
	sprintf(hFeat,
	    DoK	"Tech [%s,%s,%s,%s,%s,%s,%s,%s,%s,%s]%.*s",
		TSC[Shm->Proc.InvariantTSC],
		Shm->Proc.HyperThreading ? GoK"HTT"DoK : "HTT",
		Shm->Proc.TurboBoost ? GoK"TURBO"DoK : "TURBO",
		Shm->Proc.SpeedStep ? GoK"EIST"DoK : "EIST",
		Shm->Proc.C1E ? GoK"C1E"DoK : "C1E",
		hString,
		Shm->Cpu[0].C3A ? GoK"C3A"DoK : "C3A",
		Shm->Cpu[0].C1A ? GoK"C1A"DoK : "C1A",
		Shm->Cpu[0].C3U ? GoK"C3U"DoK : "C3U",
		Shm->Cpu[0].C1U ? GoK"C1U"DoK : "C1U",
		drawSize.width - 53,
		hSpace);
	free(hString);
    }

    void freeAll(void)
    {
	free(hRatio);
	free(hProc);
	free(hArch);
	free(hBClk);
	free(hCore);
	free(hFeat);
	free(headerView);
	free(footerView);
	free(loadView);
	free(monitorView);
	free(lcdView);
    }

    void allocAll()
    {
	const int allocSize=4 * MAX_WIDTH;
	hRatio=malloc(allocSize);
	hProc=malloc(allocSize);
	hArch=malloc(allocSize);
	hBClk=malloc(64);
	hCore=malloc(allocSize);
	hFeat=malloc(allocSize);
	headerView=malloc(3 * allocSize);
	footerView=malloc(2 * allocSize);
	loadView=malloc((1 + Shm->Proc.CPU.Count) * allocSize);
	monitorView=malloc((1 + Shm->Proc.CPU.Count) * allocSize);
	lcdView=malloc((9 * 3 * 3) + (9 * 3 * sizeof("\033[000;000H")) + 1);
    }

    allocAll();

    while(!Shutdown)
    {
	double maxRelFreq;
    	unsigned int topRatio, digit[9];

	char cursor[]="\033[000;000H", lcdColor[]="\033[1;000;000m";

	while(!BITWISEAND(Shm->Proc.Sync, 0x1) && !Shutdown)
		usleep(Shm->Proc.msleep * 50);
	BITCLR(Shm->Proc.Sync, 0);

	SCREEN_SIZE currentSize=getScreenSize();
	if(currentSize.height != drawSize.height)
	{
		drawSize.height=currentSize.height;
		if(drawSize.height < MIN_HEIGHT)
			drawFlag &= 0x0001;
		else
			drawFlag |= 0x1110;
	}
	if(currentSize.width != drawSize.width)
	{
		if(currentSize.width > MAX_WIDTH)
			drawSize.width=MAX_WIDTH;
		else
			drawSize.width=currentSize.width;

		if(drawSize.width < MIN_WIDTH)
			drawFlag &= 0x0010;
		else
			drawFlag |= 0x1001;
	}					
/*
			.Bit flags.
   0x....L C H W
         | | | |
 Layout--' | | |	Redraw layout
 Clear-----' | |	Clear screen
 Height----' | |	Valid height
 Width---------'	Valid width
*/
      if((drawFlag & 0x0011) == 0x0011)
      {
	if((drawFlag & 0x0100) == 0x0100)
	{
		drawFlag &= 0x1011;
		printf(CLS);
	}
	if((drawFlag & 0x1000) == 0x1000)
	{
		drawFlag &= 0x0111;
    		layout();
	}

	i=dec2Digit(Shm->Cpu[iclk].Clock.Hz, digit);
	sprintf(hBClk,
	    YoK	"%u%u%u %u%u%u %u%u%u"DoK,
		digit[0], digit[1], digit[2],
		digit[3], digit[4], digit[5],
		digit[6], digit[7], digit[8]);

	sprintf(loadView, "%.*s\n", drawSize.width, hRatio);
	sprintf(monitorView, DoK"%.*s\n", drawSize.width, hMon);

	maxRelFreq=0.0;
    	topRatio=0;
	for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if(!Shm->Cpu[cpu].OffLine)
	    {
		struct FLIP_FLOP *Flop=					\
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if(Flop->Relative.Freq > maxRelFreq)
		{
			maxRelFreq=Flop->Relative.Freq;
			topRatio=Flop->Relative.Ratio;
			if(topRatio > medianRatio)
				strcpy(lcdColor, RoK);
			else if(topRatio > minRatio)
				strcpy(lcdColor, YoK);
			else
				strcpy(lcdColor, GoK);
		}
		int hPos=Flop->Relative.Ratio * loadWidth / maxRatio;
		sprintf(hCore,
		    DoK	"#%s%-2u %.*s%.*s\n",
			Flop->Relative.Ratio > medianRatio ?
			RoK : Flop->Relative.Ratio > minRatio ?
			YoK : GoK,
			cpu, hPos, hBar,
			loadWidth - hPos,
			hSpace);
		strcat(loadView, hCore);

		sprintf(hCore,
		    DoK	"#"WoK"%-2u"YoK"%c"				\
		    WoK"%7.2f"DoK" MHz ("WoK"%5.2f"DoK") "		\
		    WoK	"%6.2f"DoK"%% "WoK"%6.2f"DoK"%% "WoK"%6.2f"DoK"%% "\
		    WoK	"%6.2f"DoK"%% "WoK"%6.2f"DoK"%% "WoK"%6.2f"DoK"%%  "\
		    WoK	"%3llu"DoK" C%.*s\n",
			cpu,
			cpu == iclk ? '~' : ' ',
			Flop->Relative.Freq,
			Flop->Relative.Ratio,
			100.f * Flop->State.Turbo,
			100.f * Flop->State.C0,
			100.f * Flop->State.C1,
			100.f * Flop->State.C3,
			100.f * Flop->State.C6,
			100.f * Flop->State.C7,
			Flop->Thermal.Temperature,
			drawSize.width - 78,
			hSpace);
		strcat(monitorView, hCore);
	    }
	sprintf(hCore,
	    	"%6.2f""%% ""%6.2f""%% ""%6.2f""%% "			\
	    	"%6.2f""%% ""%6.2f""%% ""%6.2f""%%",
		100.f * Shm->Proc.Avg.Turbo,
		100.f * Shm->Proc.Avg.C0,
		100.f * Shm->Proc.Avg.C1,
		100.f * Shm->Proc.Avg.C3,
		100.f * Shm->Proc.Avg.C6,
		100.f * Shm->Proc.Avg.C7);

	sprintf(footerView,
		"%s""%s"" ] %.*s\n"					\
		"%s",
		hAvg, hCore,
		drawSize.width - (1 + sizeof(hAvg) + (6 * 8)), hLine,
		hFeat);

	lcdDraw(2, 1, lcdView, cursor, (unsigned int) maxRelFreq, digit);

	cursorXY(28, 3, cursor);

	printf(	WoK "\033[1;1H"						\
		"%s"							\
		"%s"							\
		"%s"							\
	    SCP	"%s"							\
	    "%s""%s"							\
		"%s%s" RCP,
		headerView,
		loadView,
		monitorView,
		footerView,
		lcdColor, lcdView,
		cursor, hBClk);

	fflush(stdout);

	iclk++;
	if(iclk == Shm->Proc.CPU.Count)
		iclk=0;
      }
    }
    fflush(stdout);

    freeAll();
}


#define LEADING_LEFT	2
#define LEADING_TOP	1
#define MARGIN_WIDTH	2
#define MARGIN_HEIGHT	1

void Dashboard(	SHM_STRUCT *Shm,
		unsigned int leadingLeft,
		unsigned int leadingTop,
		unsigned int marginWidth,
		unsigned int marginHeight)
{
    char *boardView=NULL,
	 *lcdView=NULL,
	 *cpuView=NULL;

    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[9];
    double medianRatio=(minRatio + maxRatio) / 2;

    void freeAll(void)
    {
	free(cpuView);
	free(lcdView);
	free(boardView);
    }

    void allocAll()
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

    allocAll();

    marginWidth+=12;	// shifted by lcd width
    marginHeight+=3+1;	// shifted by lcd height + cpu frame
    unsigned int cpu=0;
    while(!Shutdown)
    {
    	unsigned int digit[9];
	unsigned int X, Y;

	char cursor[]="\033[000;000H";

	while(!BITWISEAND(Shm->Proc.Sync, 0x1) && !Shutdown)
		usleep(Shm->Proc.msleep * 50);
	BITCLR(Shm->Proc.Sync, 0);

	X=leadingLeft;
	Y=leadingTop;
	boardView[0]='\0';

	for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	    if(!Shm->Cpu[cpu].OffLine)
	    {
		struct FLIP_FLOP *Flop=					\
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		lcdDraw(X, Y, lcdView, cursor,
			(unsigned int) Flop->Relative.Freq, digit);

		cursorXY(X, Y + 3, cursor);
		sprintf(cpuView, "%s"DoK"[ µ%-2u"WoK"%4llu"DoK"C ]",
				cursor, cpu, Flop->Thermal.Temperature);

		X+=marginWidth;
		if(X - 3 >= getScreenSize().width - marginWidth)
		{
			X=leadingLeft;
			Y+=marginHeight;
		}
		if(Flop->Relative.Ratio > medianRatio)
			strcat(boardView, RoK);
		else if(Flop->Relative.Ratio > minRatio)
			strcat(boardView, YoK);
		else
			strcat(boardView, GoK);

		strcat(boardView, lcdView);
		strcat(boardView, cpuView);
	    }
	printf( CLS "%s", boardView);

	fflush(stdout);
    }
    fflush(stdout);

    freeAll();
}


void Counters(SHM_STRUCT *Shm)
{
	unsigned int cpu=0;
	while(!Shutdown)
	{
		while(!BITWISEAND(Shm->Proc.Sync, 0x1) && !Shutdown)
			usleep(Shm->Proc.msleep * 50);
		BITCLR(Shm->Proc.Sync, 0);

		printf("CPU  Frequency  Ratio   Turbo"			\
			"    C0      C1      C3      C6      C7"	\
			"     Temps\n");
		for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
		if(!Shm->Cpu[cpu].OffLine)
		{
		    struct FLIP_FLOP *Flop=				\
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		printf("#%02u %7.2fMHz (%5.2f)"				\
			" %6.2f%% %6.2f%% %6.2f%% %6.2f%% %6.2f%% %6.2f%%"\
			" %3llu:%-3lluC\n",
			cpu,
			Flop->Relative.Freq,
			Flop->Relative.Ratio,
			100.f * Flop->State.Turbo,
			100.f * Flop->State.C0,
			100.f * Flop->State.C1,
			100.f * Flop->State.C3,
			100.f * Flop->State.C6,
			100.f * Flop->State.C7,
			Flop->Thermal.Temperature,
			Flop->Thermal.Sensor);
		}
		printf("\nAverage C-states\n"				\
		"Turbo\t  C0\t  C1\t  C3\t  C6\t  C7\n"			\
		"%6.2f%%\t%6.2f%%\t%6.2f%%\t%6.2f%%\t%6.2f%%\t%6.2f%%\n\n",
			100.f * Shm->Proc.Avg.Turbo,
			100.f * Shm->Proc.Avg.C0,
			100.f * Shm->Proc.Avg.C1,
			100.f * Shm->Proc.Avg.C3,
			100.f * Shm->Proc.Avg.C6,
			100.f * Shm->Proc.Avg.C7);
	}
}


void Instructions(SHM_STRUCT *Shm)
{
	unsigned int cpu=0;
	while(!Shutdown)
	{
		while(!BITWISEAND(Shm->Proc.Sync, 0x1) && !Shutdown)
			usleep(Shm->Proc.msleep * 50);
		BITCLR(Shm->Proc.Sync, 0);

		printf("CPU     IPS            IPC            CPI\n");
		for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
		    if(!Shm->Cpu[cpu].OffLine)
		    {
		    struct FLIP_FLOP *Flop=				\
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		    printf("#%02u %12.6f/s %12.6f/c %12.6f/i\n",
			cpu,
			Flop->State.IPS,
			Flop->State.IPC,
			Flop->State.CPI);
		    }
		printf("\n");
	}
}


void Topology(SHM_STRUCT *Shm)
{
	const char *x2APIC[]=
	{
		"  OFF ",
	    GoK	" xAPIC" DoK,
	    GoK	"x2APIC" DoK
	};
	unsigned int cpu=0, level=0x0;

	while(!BITWISEAND(Shm->Proc.Sync, 0x1) && !Shutdown)
		usleep(Shm->Proc.msleep * 50);
	BITCLR(Shm->Proc.Sync, 0);

	printf(		"CPU       ApicID CoreID ThreadID x2APIC "	\
			"Caches L1-Inst Way L1-Data Way      L2 Way      L3 Way\n");
	for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
	{
		printf(	"#%02u%-5s  %6d %6d   %6d %s    |  ",
			cpu,
			(Shm->Cpu[cpu].Topology.BSP) ? "(BSP)" : "(AP)",
			Shm->Cpu[cpu].Topology.ApicID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID,
			x2APIC[Shm->Cpu[cpu].Topology.x2APIC]);
	    for(level=0; level < CACHE_MAX_LEVEL; level++)
		printf(	"%8u%4u",
			Shm->Cpu[cpu].Topology.Cache[level].Size,
			Shm->Cpu[cpu].Topology.Cache[level].Way);
	    printf("\n");
	}
}


void SysInfo(SHM_STRUCT *Shm)
{
	int i=0;
	printf(	"  Processor [%s]\n"					\
		"  Signature [%1X%1X_%1X%1X]\n"				\
		"  Stepping  [%u]\n"					\
		"  Architecture [%s]\n"					\
		"  %u/%u CPU Online.\n"					\
		"  Ratio Boost:     Min Max  8C  7C  6C  5C  4C  3C  2C  1C\n"\
		"                   ",
		Shm->Proc.Brand,
		Shm->Proc.ExtFamily,
		Shm->Proc.Family,
		Shm->Proc.ExtModel,
		Shm->Proc.Model,
		Shm->Proc.Stepping,
		Shm->Proc.Architecture,
		Shm->Proc.CPU.OnLine,
		Shm->Proc.CPU.Count	);
	for(i=0; i < 10; i++)
		if(Shm->Proc.Boost[i] != 0)
			printf("%3d ", Shm->Proc.Boost[i]);
		else
			printf("  - ");

	const char *TSC[]=
	{
		"Missing",
		"Variant",
		"Invariant"
	};
	printf(	"\n"							\
		"  Technologies:\n"					\
		"  |- Time Stamp Counter                    TSC [%9s]\n"\
		"  |- Hyper-Threading                       HTT       [%3s]\n"\
		"  |- Turbo Boost                           IDA       [%3s]\n"\
		"  |- SpeedStep                            EIST       [%3s]\n"\
		"  |- Performance Monitoring                 PM       [%3d]\n"\
		"  |- Enhanced Halt State                   C1E       [%3s]\n"\
		"  |- C1 Auto Demotion                      C1A       [%3s]\n"\
		"  |- C3 Auto Demotion                      C3A       [%3s]\n"\
		"  |- C1 UnDemotion                         C1U       [%3s]\n"\
		"  |- C3 UnDemotion                         C3U       [%3s]\n",
		TSC[Shm->Proc.InvariantTSC],
		enabled(Shm->Proc.HyperThreading),
		enabled(Shm->Proc.TurboBoost),
		enabled(Shm->Proc.SpeedStep),
		Shm->Proc.PM_version,
		enabled(Shm->Proc.C1E),
		enabled(Shm->Cpu[0].C3A),
		enabled(Shm->Cpu[0].C1A),
		enabled(Shm->Cpu[0].C3U),
		enabled(Shm->Cpu[0].C1U));
}


int help(char *appName)
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
	struct stat shmStat={0};
	SHM_STRUCT *Shm;
	int fd=-1, rc=0;

	char *program=strdup(argv[0]), *appName=basename(program);
	char option='t';
	if((argc >= 2) && (argv[1][0] == '-'))
		option=argv[1][1];
	if(option == 'h')
		help(appName);
	else if(((fd=shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) !=-1)
		&& ((fstat(fd, &shmStat) != -1)
		&& ((Shm=mmap(0, shmStat.st_size,
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))!=MAP_FAILED)))
	   {
		signal(SIGINT, Emergency);
		signal(SIGQUIT, Emergency);
		signal(SIGTERM, Emergency);

		switch(option)
		{
			case 's':
				SysInfo(Shm);
			break;
			case 'm':
				Topology(Shm);
			break;
			case 'i':
				Instructions(Shm);
			break;
			case 'c':
				Counters(Shm);
			break;
			case 'd':
				if(argc == 6)
				{
					printf(HIDE);
					Dashboard(Shm,	atoi(argv[2]),
							atoi(argv[3]),
							atoi(argv[4]),
							atoi(argv[5])	);
					printf(SHOW);
				}
				else if(argc == 2)
				{
					printf(HIDE);
					Dashboard(Shm,	LEADING_LEFT,
							LEADING_TOP,
							MARGIN_WIDTH,
							MARGIN_HEIGHT);
					printf(SHOW);
				}
				else
					rc=help(appName);
			break;
			case 't':
			{
				printf(CLS HIDE);
				Top(Shm);
				printf(CLS SHOW);
			}
			break;
			default:
				rc=help(appName);
			break;
		}
		munmap(Shm, shmStat.st_size);
		close(fd);
	    }
		else rc=2;
	free(program);
	return(rc);
}
