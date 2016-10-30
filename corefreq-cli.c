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
			Shutdown=0x1;
		break;
	}
}

// VT100 requirements.
#define SCP	"\033[s"
#define RCP	"\033[u"
#define HIDE	"\033[?25l"
#define SHOW	"\033[?25h"
#define RESET	"\033c"
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
	 *hTech=NULL,
	 *hSys=NULL,
	 *hMem=NULL,
	 *headerView=NULL,
	 *footerView=NULL,
	 *loadView=NULL,
	 *monitorView=NULL,
	 *lcdView=NULL,
	 *viewMask=NULL;

    double minRatio=Shm->Proc.Boost[0], maxRatio=Shm->Proc.Boost[9];
    double medianRatio=(minRatio + maxRatio) / 2;
    double availRatio[10]={minRatio};
    unsigned int cpu=0, iclk=0, i, ratioCount=0;
    const unsigned int CPU_BitMask=(1 << Shm->Proc.CPU.OnLine) - 1,
    	isTurboBoost=(Shm->Proc.TurboBoost & CPU_BitMask) == CPU_BitMask,
	isSpeedStep=(Shm->Proc.SpeedStep & CPU_BitMask) == CPU_BitMask;

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
    int MIN_HEIGHT=(2 * Shm->Proc.CPU.Count) + 8; // incl. header, footer lines
    int loadWidth=0;
    int drawFlag=0b0000;

    void layout(void)
    {
	char *hString=malloc(32);
	size_t	len=0;

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
	    DoK	"%.*sProcessor["CoK"%-48s"DoK"]"			\
	    WoK	"%2u"DoK"/"WoK"%-2u"DoK"CPU",
		13, hSpace,
		Shm->Proc.Brand,
		Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count);

	sprintf(hArch,
	    DoK	"%.*sArchitecture["CoK"%s"DoK"]",
		13, hSpace,
		Shm->Proc.Architecture);

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
	sprintf(headerView,
		"%s\n"							\
		"%s%.*s"						\
		"Caches "						\
		"L1 Inst="WoK"%-3u"DoK"Data="WoK"%-3u"DoK"KB\n"		\
		"%.*sBase Clock ~ 000 000 000 Hz%.*s"			\
		"L2="WoK"%-4u"DoK" L3="WoK"%-5u"DoK"KB\n",
		hProc,
		hArch,
		drawSize.width - 55 - strlen(Shm->Proc.Architecture), hSpace,
		L1I_Size,
		L1D_Size,
		13, hSpace,
		drawSize.width - 58, hSpace,
		L2U_Size,
		L3U_Size);

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
	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12))
	{
		const char *TM1[]=
		{
			"TM1",
		    BoK	"TM1" DoK,
		    WoK	"TM1" DoK,
		    GoK	"TM1" DoK,
		};
		const char *TM2[]=
		{
			"TM2",
		    BoK	"TM2" DoK,
		    WoK	"TM2" DoK,
		    GoK	"TM2" DoK,
		};
	  sprintf(hTech,
	    DoK	"Tech [%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s]%.*s",
		TSC[Shm->Proc.InvariantTSC],
		Shm->Proc.HyperThreading ? GoK"HTT"DoK : "HTT",
		isSpeedStep ? GoK"EIST"DoK : "EIST",
		isTurboBoost ? GoK"TURBO"DoK : "TURBO",
		Shm->Cpu[0].C1E ? GoK"C1E"DoK : "C1E",
		hString,
		Shm->Cpu[0].C3A ? GoK"C3A"DoK : "C3A",
		Shm->Cpu[0].C1A ? GoK"C1A"DoK : "C1A",
		Shm->Cpu[0].C3U ? GoK"C3U"DoK : "C3U",
		Shm->Cpu[0].C1U ? GoK"C1U"DoK : "C1U",
		TM1[Shm->Cpu[0].Thermal.TM1],
		TM2[Shm->Cpu[0].Thermal.TM2],
		drawSize.width - 61,
		hSpace);
	}
  else	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12))
	{
	  sprintf(hTech,
	    DoK	"Tech [%s,%s,%s,%s,%s,%s,%s,%s]%.*s",
		TSC[Shm->Proc.InvariantTSC],
		Shm->Proc.HyperThreading ? GoK"HTT"DoK : "HTT",
		Shm->Proc.PowerNow == 0b11 ? GoK"PowerNow"DoK : "PowerNow",
		isTurboBoost ? GoK"TURBO"DoK : "TURBO",
		Shm->Cpu[0].C1E ? GoK"C1E"DoK : "C1E",
		hString,
		Shm->Proc.Features.AdvPower.DX.TS ? BoK"DTS"DoK : "DTS",
		Shm->Proc.Features.AdvPower.DX.TTP ? BoK"TTP"DoK : "TTP",
		drawSize.width - 49,
		hSpace);
	}
	struct utsname OSinfo={{0}};
	uname(&OSinfo);

	len=strlen(Shm->IdleDriver.Name);
	if(len > 0)
		sprintf(hString, "/"WoK"%s"DoK"]", Shm->IdleDriver.Name);
	else
		strcpy(hString, "]");
	
	sprintf(hSys,
	    CoK	"%s"DoK" ["WoK"%s"DoK"%s%.*sTasks [",
		OSinfo.sysname,
		OSinfo.release,
		hString,
		drawSize.width
		- ( (len > 0) ? 45 : 44 )
		- strlen(OSinfo.sysname)
		- strlen(OSinfo.release)
		- len,
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
	free(hTech);
	free(hSys);
	free(hMem);
	free(headerView);
	free(footerView);
	free(loadView);
	free(monitorView);
	free(lcdView);
	free(viewMask);
    }

    void allocAll()
    {
	const int allocSize=4 * MAX_WIDTH,
		headerSize =3 * allocSize,
		footerSize =3 * allocSize,
		loadSize   =(1 + Shm->Proc.CPU.Count) * allocSize,
		monitorSize=(1 + Shm->Proc.CPU.Count) * allocSize,
		lcdSize    =(9 * 3 * 3) + (9 * 3 * sizeof("\033[000;000H")) + 1,
		maskSize   = headerSize
			   + footerSize
			   + loadSize
			   + monitorSize
			   + lcdSize;
	hRatio=malloc(allocSize);
	hProc=malloc(allocSize);
	hArch=malloc(allocSize);
	hBClk=malloc(64);
	hCore=malloc(allocSize);
	hTech=malloc(allocSize);
	hSys=malloc(allocSize);
	hMem=malloc(allocSize);
	headerView=malloc(headerSize);
	footerView=malloc(footerSize);
	loadView=malloc(loadSize);
	monitorView=malloc(monitorSize);
	lcdView=malloc(lcdSize);
	viewMask=malloc(maskSize);
    }

    allocAll();

    while(!Shutdown)
    {
	double maxRelFreq;
    	unsigned int topRatio, digit[9];

	char cursor[]="\033[000;000H", lcdColor[]="\033[1;000;000m";

	while(!BITWISEAND(Shm->Proc.Sync, 0x1UL) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * 50);
	BITCLR(Shm->Proc.Sync, 0);
	if(BITWISEAND(Shm->Proc.Sync, 0x8000000000000000UL))
	{	// Platform changed, redraw the layout.
		drawFlag |= 0b1000;
		BITCLR(Shm->Proc.Sync, 63);
	}
	SCREEN_SIZE currentSize=getScreenSize();
	if(currentSize.height != drawSize.height)
	{
		drawSize.height=currentSize.height;
		if(drawSize.height < MIN_HEIGHT)
			drawFlag &= 0b0001;
		else
			drawFlag |= 0b1110;
	}
	if(currentSize.width != drawSize.width)
	{
		if(currentSize.width > MAX_WIDTH)
			drawSize.width=MAX_WIDTH;
		else
			drawSize.width=currentSize.width;

		if(drawSize.width < MIN_WIDTH)
			drawFlag &= 0b0010;
		else
			drawFlag |= 0b1001;
	}
/*
			.Bit flags.
  0b0000 L C H W
         | | | |
 Layout--' | | |	Redraw layout
 Clear-----' | |	Clear screen
 Height----' | |	Valid height
 Width---------'	Valid width
*/
      if((drawFlag & 0b0011) == 0b0011)
      {
	if((drawFlag & 0b0100) == 0b0100)
	{
		drawFlag &= 0b1011;
		printf(CLS);
	}
	if((drawFlag & 0b1000) == 0b1000)
	{
		drawFlag &= 0b0111;
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
	    if(!Shm->Cpu[cpu].OffLine.HW)
	    {
		struct FLIP_FLOP *Flop=
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
		if(!Shm->Cpu[cpu].OffLine.OS)
		{
			int hPos=Flop->Relative.Ratio * loadWidth / maxRatio;
			sprintf(hCore,
			    DoK	"#%s%-2u %.*s%.*s\n",
				Flop->Relative.Ratio > medianRatio ?
				RoK : Flop->Relative.Ratio > minRatio ?
				YoK : GoK,
				cpu, hPos, hBar,
				loadWidth - hPos,
				hSpace);
		}
		else
			sprintf(hCore, DoK "#%-2u %.*s\n",
				cpu, drawSize.width - 4, hSpace);

		strcat(loadView, hCore);

		if(!Shm->Cpu[cpu].OffLine.OS)
		    sprintf(hCore,
			DoK"#"WoK"%-2u"YoK"%c"				\
			WoK"%7.2f"DoK" MHz ("WoK"%5.2f"DoK") "		\
			WoK"%6.2f"DoK"%% "WoK"%6.2f"DoK"%% "WoK"%6.2f"DoK"%% "\
			WoK"%6.2f"DoK"%% "WoK"%6.2f"DoK"%% "WoK"%6.2f"DoK"%%  "\
			WoK"%3llu"DoK" C%s%.*s\n",
			cpu,
			(cpu == iclk) ? '~' : ' ',
			Flop->Relative.Freq,
			Flop->Relative.Ratio,
			100.f * Flop->State.Turbo,
			100.f * Flop->State.C0,
			100.f * Flop->State.C1,
			100.f * Flop->State.C3,
			100.f * Flop->State.C6,
			100.f * Flop->State.C7,
			Flop->Thermal.Temp,
			Flop->Thermal.Trip ? RoK"*"DoK : " ",
			drawSize.width - 78,
			hSpace);
		else
		    sprintf(hCore,
			DoK"#""%-2u"YoK"%c"DoK"%.*s\n",
			cpu,
			(cpu == iclk) ? '~' : ' ',
			drawSize.width - 4, hSpace);

		strcat(monitorView, hCore);
	    }

	sprintf(hCore,
	    	"%6.2f""%%%% ""%6.2f""%%%% ""%6.2f""%%%% "		\
	    	"%6.2f""%%%% ""%6.2f""%%%% ""%6.2f""%%%%",
		100.f * Shm->Proc.Avg.Turbo,
		100.f * Shm->Proc.Avg.C0,
		100.f * Shm->Proc.Avg.C1,
		100.f * Shm->Proc.Avg.C3,
		100.f * Shm->Proc.Avg.C6,
		100.f * Shm->Proc.Avg.C7);

	struct sysinfo sysLinux=
	{
		.totalram=0,
		.freeram=0,
		.procs=0
	};
	sysinfo(&sysLinux);
	sprintf(hMem,
		"%s"WoK"%6u"DoK"]"					\
		" Mem ["WoK"%8lu"DoK"/"WoK"%8lu"DoK" KB]",
		hSys,
		sysLinux.procs,
		sysLinux.freeram  / 1024,
		sysLinux.totalram / 1024);

	sprintf(footerView,
		"%s""%s"" ] %.*s\n"					\
		"%s\n"							\
		"%s",
		hAvg, hCore,
		drawSize.width - (1 + sizeof(hAvg) + (6 * 8)), hLine,
		hTech,
		hMem);

	lcdDraw(1, 1, lcdView, cursor, (unsigned int) maxRelFreq, digit);

	cursorXY(27, 3, cursor);

	sprintf(viewMask,
		WoK "\033[1;1H"						\
/*header*/	"%s"							\
/*load*/	"%s"							\
/*monitor*/	"%s"							\
/*footer*/ SCP	"%s"							\
/*LCD*/		"%s""%s"						\
/*clock*/	"%s%s" RCP,
		headerView,
		loadView,
		monitorView,
		footerView,
		lcdColor, lcdView,
		cursor, hBClk);

	printf(viewMask);

	fflush(stdout);

	do
	{
		iclk++;
		if(iclk == Shm->Proc.CPU.Count)
			iclk=0;
	} while(Shm->Cpu[iclk].OffLine.OS && iclk) ;
      }
      else
	printf(	CLS "Term(%u x %u) < View(%u x %u)\n",
		drawSize.width, drawSize.height, MIN_WIDTH, MIN_HEIGHT);
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

	while(!BITWISEAND(Shm->Proc.Sync, 0x1UL) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * 50);
	BITCLR(Shm->Proc.Sync, 0);
	if(BITWISEAND(Shm->Proc.Sync, 0x8000000000000000UL))
		BITCLR(Shm->Proc.Sync, 63);

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
			lcdDraw(X, Y, lcdView, cursor,
				(unsigned int) Flop->Relative.Freq, digit);
			cursorXY(X, Y + 3, cursor);
			sprintf(cpuView, "%s"DoK"[ µ%-2u"WoK"%4llu"DoK"C ]",
					cursor, cpu, Flop->Thermal.Temp);

			if(Flop->Relative.Ratio > medianRatio)
				strcat(boardView, RoK);
			else if(Flop->Relative.Ratio > minRatio)
				strcat(boardView, YoK);
			else
				strcat(boardView, GoK);
		}
		else
		{
			cursorXY(X + 1, Y + 1, cursor);
			sprintf(lcdView, "%s_  _  _  _", cursor, 3);
			cursorXY(X, Y + 3, cursor);
			sprintf(cpuView, "%s""[ µ%-2u""  OFF ]",
					cursor, cpu);
		}
		X+=marginWidth;
		if(X - 3 >= getScreenSize().width - marginWidth)
		{
			X=leadingLeft;
			Y+=marginHeight;
		}
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
	while(!BITWISEAND(Shm->Proc.Sync, 0x1UL) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * 50);
	BITCLR(Shm->Proc.Sync, 0);
	if(BITWISEAND(Shm->Proc.Sync, 0x8000000000000000UL))
		BITCLR(Shm->Proc.Sync, 63);

		printf("CPU  Frequency  Ratio   Turbo"			\
			"    C0      C1      C3      C6      C7"	\
			"     T:dts\n");
	for(cpu=0; (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
	  if(!Shm->Cpu[cpu].OffLine.HW)
	  {
	    struct FLIP_FLOP *Flop=
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

	    if(!Shm->Cpu[cpu].OffLine.OS)
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
			Flop->Thermal.Temp,
			Flop->Thermal.Sensor);
	    else
		printf("#%02u        OFF\n", cpu);

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
	  while(!BITWISEAND(Shm->Proc.Sync, 0x1UL) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * 50);
	  BITCLR(Shm->Proc.Sync, 0);
	  if(BITWISEAND(Shm->Proc.Sync, 0x8000000000000000UL))
		BITCLR(Shm->Proc.Sync, 63);

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


void Topology(SHM_STRUCT *Shm)
{
	unsigned int cpu=0, level=0;

	while(!BITWISEAND(Shm->Proc.Sync, 0x1UL) && !Shutdown)
		usleep(Shm->Proc.SleepInterval * 50);
	BITCLR(Shm->Proc.Sync, 0);
	if(BITWISEAND(Shm->Proc.Sync, 0x8000000000000000UL))
		BITCLR(Shm->Proc.Sync, 63);

	printf(	"CPU   Apic Core Thread"				\
		"| Caches    (w) Write-Back  (i) Inclusive\n"		\
		" #     ID   ID    ID  "				\
		"|L1-Inst Way    L1-Data Way     L2  Way         L3  Way\n");
	for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
	{
		printf(	"%02u%-4s%4d %4d %4d",
			cpu,
			(Shm->Cpu[cpu].Topology.MP.BSP) ? ":BSP" : ":AP",
			Shm->Cpu[cpu].Topology.ApicID,
			Shm->Cpu[cpu].Topology.CoreID,
			Shm->Cpu[cpu].Topology.ThreadID);
	    for(level=0; level < CACHE_MAX_LEVEL; level++)
		printf(	"%8u%4u %c%c",
			Shm->Cpu[cpu].Topology.Cache[level].Size,
			Shm->Cpu[cpu].Topology.Cache[level].Way,
			Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack?
				'w' : '.',
			Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive?
				'i' : '.');
	    printf("\n");
	}
}


void SysInfo(SHM_STRUCT *Shm)
{
	const unsigned int CPU_BitMask=(1 << Shm->Proc.CPU.OnLine) - 1,
    	isTurboBoost=(Shm->Proc.TurboBoost & CPU_BitMask) == CPU_BitMask,
	isSpeedStep=(Shm->Proc.SpeedStep & CPU_BitMask) == CPU_BitMask;
	size_t len=0;
	int i=0;
	printf(	"  Processor%.*s[%s]\n"					\
		"  |- Signature%.*s[%1X%1X_%1X%1X]\n"			\
		"  |- Stepping%.*s[%3u]\n"				\
		"  |- Architecture%.*s[%s]\n"				\
		"  |- Online CPU%.*s[%u/%u]\n"				\
		"  |- Base Clock%.*s[%3llu]\n"				\
		"  |- Ratio Boost:%.*s"					\
			"Min Max  8C  7C  6C  5C  4C  3C  2C  1C\n"	\
		"%.*s",
		67-strlen(Shm->Proc.Brand), hSpace, Shm->Proc.Brand,
		59, hSpace,
		Shm->Proc.Features.Std.AX.ExtFamily,
		Shm->Proc.Features.Std.AX.Family,
		Shm->Proc.Features.Std.AX.ExtModel,
		Shm->Proc.Features.Std.AX.Model,
		62, hSpace, Shm->Proc.Features.Std.AX.Stepping,
		61-strlen(Shm->Proc.Architecture),hSpace,Shm->Proc.Architecture,
		60, hSpace, Shm->Proc.CPU.OnLine, Shm->Proc.CPU.Count,
		60, hSpace, Shm->Cpu[0].Clock.Hz / 1000000L,
		7, hSpace,
		24, hSpace);
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
	printf(	"\n"							\
	"  Instruction set:\n"						\
	"  |- FPU       [%c]          CMPXCH8 [%c]"			\
		"             SEP [%c]             CMOV [%c]\n"		\
	"  |- CLFSH     [%c]        MMX/Ext [%c/%c]"			\
		"            FXSR [%c]              SSE [%c]\n"		\
	"  |- SSE2      [%c]             SSE3 [%c]"			\
		"           SSSE3 [%c]      SSE4.1/4A [%c/%c]\n"	\
	"  |- SSE4.2    [%c]         PCLMULDQ [%c]"			\
		"         MONITOR [%c]         CMPXCH16 [%c]\n"		\
	"  |- MOVBE     [%c]           POPCNT [%c]"			\
		"             AES [%c]       AVX/AVX2 [%c/%c]\n"	\
	"  |- F16C      [%c]           RDRAND [%c]"			\
		"          RDTSCP [%c]        LAHF/SAHF [%c]\n"		\
	"  |- SYSCALL   [%c]      BMI1/BMI2 [%c/%c]"			\
		"          3DNow! [%c]         3DNowExt [%c]\n"		\
	"\n"								\
	"  Features:\n"							\
	"  |- Virtual Mode Extension%.*sVME   [%7s]\n"			\
	"  |- Debugging Extension%.*sDE   [%7s]\n"			\
	"  |- Page Size Extension%.*sPSE   [%7s]\n"			\
	"  |- Time Stamp Counter%.*sTSC [%9s]\n"			\
	"  |- Time Stamp Counter Deadline%.*sTSC-DEADLINE   [%7s]\n"	\
	"  |- Model Specific Registers%.*sMSR   [%7s]\n"		\
	"  |- Physical Address Extension%.*sPAE   [%7s]\n"		\
	"  |- Advanced Programmable Interrupt Controller"		\
						"%.*sAPIC   [%7s]\n"	\
	"  |- Extended xAPIC Support%.*sx2APIC   [%7s]\n"		\
	"  |- Memory Type Range Registers%.*sMTRR   [%7s]\n"		\
	"  |- Page Global Enable%.*sPGE   [%7s]\n"			\
	"  |- Machine-Check Architecture%.*sMCA   [%7s]\n"		\
	"  |- Page Attribute Table%.*sPAT   [%7s]\n"			\
	"  |- 36-bit Page Size Extension%.*sPSE36   [%7s]\n"		\
	"  |- Processor Serial Number%.*sPSN   [%7s]\n"			\
	"  |- Debug Store & Precise Event Based Sampling"		\
					"%.*sDS, PEBS   [%7s]\n"	\
	"  |- Advanced Configuration & Power Interface"			\
					"%.*sACPI   [%7s]\n"		\
	"  |- Self-Snoop%.*sSS   [%7s]\n"				\
	"  |- Pending Break Enable%.*sPBE   [%7s]\n"			\
	"  |- 64-Bit Debug Store%.*sDTES64   [%7s]\n"			\
	"  |- CPL Qualified Debug Store%.*sDS-CPL   [%7s]\n"		\
	"  |- Virtual Machine Extensions%.*sVMX   [%7s]\n"		\
	"  |- Safer Mode Extensions%.*sSMX   [%7s]\n"			\
	"  |- L1 Data Cache Context ID%.*sCNXT-ID   [%7s]\n"		\
	"  |- Fused Multiply Add%.*sFMA|FMA4   [%7s]\n"			\
	"  |- xTPR Update Control%.*sxTPR   [%7s]\n"			\
	"  |- Perfmon and Debug Capability%.*sPDCM   [%7s]\n"		\
	"  |- Process Context Identifiers%.*sPCID   [%7s]\n"		\
	"  |- Direct Cache Access%.*sDCA   [%7s]\n"			\
	"  |- XSAVE/XSTOR States%.*sXSAVE   [%7s]\n"			\
	"  |- OS-Enabled Ext. State Management%.*sOSXSAVE   [%7s]\n"	\
	"  |- Execution Disable Bit Support%.*sXD-Bit   [%7s]\n"	\
	"  |- 1 GB Pages Support%.*s1GB-PAGES   [%7s]\n"		\
	"  |- Hardware Lock Elision%.*sHLE   [%7s]\n"			\
	"  |- Restricted Transactional Memory%.*sRTM   [%7s]\n"		\
	"  |- Fast-String Operation%.*sFast-Strings   [%7s]\n"		\
	"  |- Long Mode 64 bits%.*sIA64|LM   [%7s]\n"			\
	"  |- Core Multi-Processing%.*sCMP Legacy   [%7s]\n"		\
	"  |- LightWeight Profiling%.*sLWP   [%7s]\n"			\
	"  |- 100 MHz multiplier Control%.*s100MHzSteps   [%7s]\n"	\
	"\n"								\
	"  Technologies:\n"						\
	"  |- Hyper-Threading%.*sHTT       [%3s]\n"			\
	"  |- SpeedStep%.*sEIST       [%3s]\n"				\
	"  |- PowerNow!%.*sPowerNow       [%3s]\n"			\
	"  |- Dynamic Acceleration%.*sIDA       [%3s]\n"		\
	"  |- Turbo Boost%.*sTURBO|CPB       [%3s]\n"			\
	"\n"								\
	"  Performance Monitoring:\n"					\
	"  |- Version%.*sPM       [%3d]\n"				\
	"  |- Counters:%.*sGeneral%.*sFixed\n"				\
	"  |%.*s%3u x%3u bits%.*s%3u x%3u bits\n"			\
	"  |- Enhanced Halt State%.*sC1E       [%3s]\n"			\
	"  |- C1 Auto Demotion%.*sC1A       [%3s]\n"			\
	"  |- C3 Auto Demotion%.*sC3A       [%3s]\n"			\
	"  |- C1 UnDemotion%.*sC1U       [%3s]\n"			\
	"  |- C3 UnDemotion%.*sC3U       [%3s]\n"			\
	"  |- Frequency ID control%.*sFID       [%3s]\n"		\
	"  |- Voltage ID control%.*sVID       [%3s]\n"			\
	"  |- P-State Hardware Coordination Feedback"			\
			"%.*sMPERF/APERF       [%3s]\n"			\
	"  |- Hardware Performance States%.*sHWP       [%3s]\n"		\
	"  |- Hardware Duty Cycling%.*sHDC       [%3s]\n"		\
	"  |- MWAIT States:%.*sC0      C1      C2      C3      C4\n"	\
	"  |%.*s%2d      %2d      %2d      %2d      %2d\n"		\
	"  |- Core Cycles%.*s[%7s]\n"					\
	"  |- Instructions Retired%.*s[%7s]\n"				\
	"  |- Reference Cycles%.*s[%7s]\n"				\
	"  |- Last Level Cache References%.*s[%7s]\n"			\
	"  |- Last Level Cache Misses%.*s[%7s]\n"			\
	"  |- Branch Instructions Retired%.*s[%7s]\n"			\
	"  |- Branch Mispredicts Retired%.*s[%7s]\n"			\
	"\n"								\
	"  Thermal Monitoring:\n"					\
	"  |- Digital Thermal Sensor%.*sDTS   [%7s]\n"			\
	"  |- Thermal Monitor 1%.*sTM1|TTP   [%7s]\n"			\
	"  |- Thermal Monitor 2%.*sTM2|HTC   [%7s]\n",
	Shm->Proc.Features.Std.DX.FPU ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.CMPXCH8 ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.SEP ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.CMOV ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.CLFSH ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.MMX ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.DX.MMX_Ext ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.FXSR ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.SSE ? 'Y' : 'N',
	Shm->Proc.Features.Std.DX.SSE2 ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.SSE3 ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.SSSE3 ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.SSE41 ? 'Y' : 'N',
		Shm->Proc.Features.ExtInfo.CX.SSE4A ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.SSE42 ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.PCLMULDQ ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.MONITOR ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.CMPXCH16 ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.MOVBE ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.POPCNT ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.AES ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.AVX ? 'Y' : 'N',
		Shm->Proc.Features.ExtFeature.BX.AVX2 ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.F16C ? 'Y' : 'N',
	Shm->Proc.Features.Std.CX.RDRAND ? 'Y' : 'N',
	Shm->Proc.Features.ExtInfo.DX.RDTSCP ? 'Y' : 'N',
	Shm->Proc.Features.ExtInfo.CX.LAHFSAHF ? 'Y' : 'N',
	Shm->Proc.Features.ExtInfo.DX.SYSCALL ? 'Y' : 'N',
	Shm->Proc.Features.ExtFeature.BX.BMI1 ? 'Y' : 'N',
		Shm->Proc.Features.ExtFeature.BX.BMI2 ? 'Y' : 'N',
	Shm->Proc.Features.ExtInfo.DX._3DNow ? 'Y' : 'N',
	Shm->Proc.Features.ExtInfo.DX._3DNowEx ? 'Y' : 'N',
	38, hSpace, powered(Shm->Proc.Features.Std.DX.VME),
	42, hSpace, powered(Shm->Proc.Features.Std.DX.DE),
	41, hSpace, powered(Shm->Proc.Features.Std.DX.PSE),
	42, hSpace, TSC[Shm->Proc.InvariantTSC],
	24, hSpace, powered(Shm->Proc.Features.Std.CX.TSCDEAD),
	36, hSpace, powered(Shm->Proc.Features.Std.DX.MSR),
	34, hSpace, powered(Shm->Proc.Features.Std.DX.PAE),
	17, hSpace, powered(Shm->Proc.Features.Std.DX.APIC),
	35, hSpace, x2APIC[Shm->Cpu[0].Topology.MP.x2APIC],
	32, hSpace, powered(Shm->Proc.Features.Std.DX.MTRR),
	42, hSpace, powered(Shm->Proc.Features.Std.DX.PGE),
	34, hSpace, powered(Shm->Proc.Features.Std.DX.MCA),
	40, hSpace, powered(Shm->Proc.Features.Std.DX.PAT),
	32, hSpace, powered(Shm->Proc.Features.Std.DX.PSE36),
	37, hSpace, powered(Shm->Proc.Features.Std.DX.PSN),
	13, hSpace, powered(Shm->Proc.Features.Std.DX.DS_PEBS),
	19, hSpace, powered(Shm->Proc.Features.Std.DX.ACPI),
	51, hSpace, powered(Shm->Proc.Features.Std.DX.SS),
	40, hSpace, powered(Shm->Proc.Features.Std.DX.PBE),
	39, hSpace, powered(Shm->Proc.Features.Std.CX.DTES64),
	32, hSpace, powered(Shm->Proc.Features.Std.CX.DS_CPL),
	34, hSpace, powered(Shm->Proc.Features.Std.CX.VMX),
	39, hSpace, powered(Shm->Proc.Features.Std.CX.SMX),
	32, hSpace, powered(Shm->Proc.Features.Std.CX.CNXT_ID),
	37, hSpace, powered(	  Shm->Proc.Features.Std.CX.FMA
				| Shm->Proc.Features.ExtInfo.CX.FMA4 ),
	40, hSpace, powered(Shm->Proc.Features.Std.CX.xTPR),
	31, hSpace, powered(Shm->Proc.Features.Std.CX.PDCM),
	32, hSpace, powered(Shm->Proc.Features.Std.CX.PCID),
	41, hSpace, powered(Shm->Proc.Features.Std.CX.DCA),
	40, hSpace, powered(Shm->Proc.Features.Std.CX.XSAVE),
	24, hSpace, powered(Shm->Proc.Features.Std.CX.OSXSAVE),
	28, hSpace, powered(Shm->Proc.Features.ExtInfo.DX.XD_Bit),
	36, hSpace, powered(Shm->Proc.Features.ExtInfo.DX.PG_1GB),
	39, hSpace, powered(Shm->Proc.Features.ExtFeature.BX.HLE),
	29, hSpace, powered(Shm->Proc.Features.ExtFeature.BX.RTM),
	30, hSpace, powered(Shm->Proc.Features.ExtFeature.BX.FastStrings),
	39, hSpace, powered(Shm->Proc.Features.ExtInfo.DX.IA64),
	32, hSpace, powered(Shm->Proc.Features.ExtInfo.CX.MP_Mode),
	39, hSpace, powered(Shm->Proc.Features.ExtInfo.CX.LWP),
	26, hSpace, powered(Shm->Proc.Features.AdvPower.DX._100MHz),
	45, hSpace, enabled(Shm->Proc.HyperThreading),
	50, hSpace, enabled(isSpeedStep),
	46, hSpace, enabled(Shm->Proc.PowerNow == 0b11),
	40, hSpace, enabled(Shm->Proc.Features.Power.AX.TurboIDA),
	43, hSpace, enabled(	  isTurboBoost
				| Shm->Proc.Features.AdvPower.DX.CPB ),
	54, hSpace, Shm->Proc.PM_version,
	10, hSpace, 17, hSpace,
	19, hSpace,	Shm->Proc.Features.PerfMon.AX.MonCtrs,
			Shm->Proc.Features.PerfMon.AX.MonWidth,
	11, hSpace,	Shm->Proc.Features.PerfMon.DX.FixCtrs,
			Shm->Proc.Features.PerfMon.DX.FixWidth,
	41, hSpace, enabled(Shm->Cpu[0].C1E),
	44, hSpace, enabled(Shm->Cpu[0].C3A),
	44, hSpace, enabled(Shm->Cpu[0].C1A),
	47, hSpace, enabled(Shm->Cpu[0].C3U),
	47, hSpace, enabled(Shm->Cpu[0].C1U),
	40, hSpace, enabled(Shm->Proc.Features.AdvPower.DX.FID),
	42, hSpace, enabled(Shm->Proc.Features.AdvPower.DX.VID),
	14, hSpace, enabled(Shm->Proc.Features.Power.CX.HCF_Cap),
	33, hSpace, enabled(	  Shm->Proc.Features.Power.AX.HWP_Reg
				| Shm->Proc.Features.AdvPower.DX.HwPstate ),
	39, hSpace, enabled(	  Shm->Proc.Features.Power.AX.HDC_Reg),
	06, hSpace,
	21, hSpace,
		Shm->Proc.Features.MWait.DX.Num_C0_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C1_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C2_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C3_MWAIT,
		Shm->Proc.Features.MWait.DX.Num_C4_MWAIT,
	55, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.CoreCycles),
	46, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.InstrRetired),
	50, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.RefCycles),
	39, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.LLC_Ref),
	43, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.LLC_Misses),
	39, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.BranchRetired),
	40, hSpace,
	    powered(!Shm->Proc.Features.PerfMon.BX.BranchMispred),
	38, hSpace, powered(	  Shm->Proc.Features.Power.AX.DTS
				| Shm->Proc.Features.AdvPower.DX.TS ),
	39, hSpace, TM[   Shm->Cpu[0].Thermal.TM1
			| Shm->Proc.Features.AdvPower.DX.TTP ],
	39, hSpace, TM[	  Shm->Cpu[0].Thermal.TM2
			| Shm->Proc.Features.AdvPower.DX.TM ]);

	struct utsname OSinfo={{0}};
	uname(&OSinfo);

	printf( "\n"							\
		"  %s:\n"						\
		"  |- Release%.*s[%s]\n",
		OSinfo.sysname,
		66-strlen(OSinfo.release), hSpace, OSinfo.release);

	if((len=strlen(Shm->IdleDriver.Name)) > 0)
	{
		printf(	"  |- Idle driver%.*s[%s]\n"			\
			"     |- States:%.*s",
			62-len, hSpace, Shm->IdleDriver.Name,
			9, hSpace);
		for(i=0; i < Shm->IdleDriver.stateCount; i++)
			printf("%-8s", Shm->IdleDriver.State[i].Name);
		printf(	"\n"						\
			"     |- Power:%.*s",
			10, hSpace);
		for(i=0; i < Shm->IdleDriver.stateCount; i++)
			printf("%-8d", Shm->IdleDriver.State[i].powerUsage);
		printf(	"\n"						\
			"     |- Latency:%.*s",
			8, hSpace);
		for(i=0; i < Shm->IdleDriver.stateCount; i++)
			printf("%-8u", Shm->IdleDriver.State[i].exitLatency);
		printf(	"\n"						\
			"     |- Residency:%.*s",
			6, hSpace);
		for(i=0; i < Shm->IdleDriver.stateCount; i++)
			printf("%-8u",Shm->IdleDriver.State[i].targetResidency);
		printf(	"\n");
	}
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
					printf(SHOW RESET);
				}
				else if(argc == 2)
				{
					printf(HIDE);
					Dashboard(Shm,	LEADING_LEFT,
							LEADING_TOP,
							MARGIN_WIDTH,
							MARGIN_HEIGHT);
					printf(SHOW RESET);
				}
				else
					rc=help(appName);
			break;
			case 't':
			{
				printf(CLS HIDE);
				Top(Shm);
				printf(SHOW RESET);
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
