/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>

#include <X11/Xlib.h>
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreq-gui-lib.h"
#include "corefreq-gui.h"

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

		return (sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset));
	}
	return (0);
}

void Build_CPU_Frequency(const int x, const int y, const int w, const int h,
			const XWINDOW *W)
{	/* Columns header						*/
	DrawStr(W, B, MEDIUM, x, y, "CPU", 3);

	DrawStr(W, B, MEDIUM, x + w - ((8+1) * One_Char_Width(W, SMALL)), y,
		"Freq[Mhz]", 9 );
}

void BuildLayout(uARG *U)
{
	XWINDOW *W = &U->A->W;
	const size_t len = strlen(U->M.Shm->Proc.Brand);
	/* Reset the color context.					*/
	SetFG(W, SMALL, W->color.foreground);
	/* Processor specification.					*/
	DrawStr(W, B, LARGE,
		abs(W->width - (One_Char_Width(W, LARGE) * len)) / 2,
		One_Char_Height(W, LARGE),
		U->M.Shm->Proc.Brand, len);

	Build_CPU_Frequency(	One_Char_Width(W, SMALL),
				Twice_Char_Height(W, LARGE),
				42 * One_Char_Width(W, SMALL), 0,
				W );
}

void Draw_CPU_Frequency(const int x, const int y, const int w, const int h,
			const uARG *U,
			const unsigned int cpu, const struct FLIP_FLOP *CFlop)
{
	XWINDOW *W = &U->A->W;
	char str[16];
	snprintf(str, 16, "%03u%7.2f", cpu, CFlop->Relative.Freq);

	int r	= ((w - Twice_Char_Width(W, SMALL)) * CFlop->Relative.Ratio)
		/ U->M.Shm->Proc.Boost[BOOST(1C)];

	int p = y + (Twice_Char_Height(W, SMALL) * (cpu + 1));

    if (CFlop->Relative.Ratio >= U->M.Shm->Proc.Boost[BOOST(MAX)]) {
	SetFG(W, SMALL, _COLOR_BAR);
    } else {
	SetFG(W);
    }
	FillRect(W, F, SMALL, x, p, r, h);

	SetFG(W, SMALL, _COLOR_TEXT);

	DrawStr(W, F, SMALL, x, p + One_Char_Height(W, SMALL), str, 3);

	DrawStr(W, F, SMALL,
		x + w - ((7+1) * One_Char_Width(W, SMALL)),
		p + One_Char_Height(W, SMALL),
		&str[3], 7);
}

void DrawLayout(uARG *U)
{
	XWINDOW *W = &U->A->W;
	unsigned int cpu;
    for (cpu = 0; cpu < U->M.Shm->Proc.CPU.Count; cpu++)
    {
	struct FLIP_FLOP *CFlop = \
		&U->M.Shm->Cpu[cpu].FlipFlop[
			!U->M.Shm->Cpu[cpu].Toggle
		];

	Draw_CPU_Frequency(	One_Char_Width(W, SMALL),
				One_Half_Char_Height(W, LARGE),
				42 * One_Char_Width(W, SMALL),
				One_Half_Char_Height(W, SMALL),
				U, cpu, CFlop );
    }
}

void MapLayout(xARG *A)
{
	XCopyArea(A->display, A->W.pixmap.B, A->W.pixmap.F, A->W.gc[SMALL],
			0, 0, A->W.width, A->W.height, 0, 0);
}

void FlushLayout(xARG *A)
{
	XCopyArea(A->display, A->W.pixmap.F, A->W.window, A->W.gc[SMALL],
			0, 0, A->W.width, A->W.height, 0, 0);

	XFlush(A->display);
}

static void *DrawLoop(void *uArg)
{
	uARG *U = (uARG *) uArg;

	pthread_setname_np(U->TID.Drawing, "corefreq-gui-dw");

	ClientFollowService(&localService, &U->M.Shm->Proc.Service, 0);

    while (!BITVAL(U->Shutdown, SYNC))
    {
	if (BITCLR(LOCKLESS, U->M.Shm->Proc.Sync, SYNC1) == 0) {
		nanosleep(&U->M.Shm->Sleep.pollingWait, NULL);
	} else {
		Paint(U, False, True, True, True);
	}
	if (BITCLR(LOCKLESS, U->M.Shm->Proc.Sync, NTFY1)) {
		ClientFollowService(&localService, &U->M.Shm->Proc.Service, 0);
	}
    }
	return(NULL);
}

void *EventLoop(uARG *U)
{
	while (!BITVAL(U->Shutdown, SYNC))
	{
		switch (EventGUI(U->A)) {
		case GUI_NONE:
			break;
		case GUI_EXIT:
			BITSET(LOCKLESS, U->Shutdown, SYNC);
			break;
		case GUI_BUILD:
			break;
		case GUI_MAP:
			break;
		case GUI_DRAW:
			break;
		case GUI_FLUSH:
			Paint(U, False, False, False, True);
			break;
		case GUI_PAINT:
			Paint(U, True, True, False, True);
			break;
		}
	}
	return (NULL);
}

static void *Emergency(void *uArg)
{
	uARG *U=(uARG *) uArg;

	pthread_setname_np(U->TID.SigHandler, "corefreq-gui-sg");

	ClientFollowService(&localService, &U->M.Shm->Proc.Service, 0);

	int caught = 0;
    while (!BITVAL(U->Shutdown, SYNC) && !sigwait(&U->TID.Signal, &caught))
    {
	if (BITVAL(LOCKLESS, U->M.Shm->Proc.Sync, NTFY1)) {
		ClientFollowService(&localService, &U->M.Shm->Proc.Service, 0);
	}
	switch (caught) {
	case SIGINT:
	case SIGQUIT:
	case SIGUSR1:
	case SIGTERM: {
		XClientMessageEvent E = {
			.type	= ClientMessage,
			.serial = 0,
			.send_event = False,
			.display = U->A->display,
			.window = U->A->W.window,
			.message_type = U->A->atom[0],
			.format = 32,
			.data	= { .l = {U->A->atom[0]} }
		};
		XSendEvent(U->A->display, U->A->W.window, 0, 0, (XEvent *) &E);
		XFlush(U->A->display);
	}
		break;
	}
    }
	return (NULL);
}

OPTION Options[] = {
	{ .opt="--help"  , .fmt=NULL , .dsc="\t\tPrint out this message"},
	{ .opt="--small" , .fmt="%s" , .dsc="\t\tX Small Font"		},
	{ .opt="--medium", .fmt="%s" , .dsc="\tX Medium Font"		},
	{ .opt="--large" , .fmt="%s" , .dsc="\t\tX Large Font"		},
	{ .opt="--Xacl"  , .fmt="%c" , .dsc="\t\tXEnableAccessControl"	},
	{ .opt="--root"  , .fmt="%c" , .dsc="\t\tUse the root window"	},
	{ .opt="--fg"    , .fmt="%lx", .dsc="\t\tForeground color"	},
	{ .opt="--bg"    , .fmt="%lx", .dsc="\t\tbackground color"	}
};

OPTION *Option(int jdx, uARG *U)
{
	OPTION variable[] = {
		{ .var = NULL					},
		{ .var = U->A->font[SMALL].name 		},
		{ .var = U->A->font[MEDIUM].name		},
		{ .var = U->A->font[LARGE].name 		},
		{ .var = &U->A->Xacl,				},
		{ .var = &U->A->Xroot,				},
		{ .var = &U->A->W.color.foreground		},
		{ .var = &U->A->W.color.background		}
	};
	const size_t zdx = sizeof(Options) / sizeof(OPTION);

	if (jdx < zdx) {
		Options[jdx].var = variable[jdx].var;
		return (&Options[jdx]);
	} else {
		return (NULL);
	}
}

void Help(uARG *U)
{
	const size_t zdx = sizeof(Options) / sizeof(OPTION);
	int jdx = strlen(__FILE__) - 2;

	printf( "CoreFreq."					\
		"  Copyright (C) 2015-2020 CYRIL INGENIERIE\n\n"\
		"Usage:\t%.*s [--option <arguments>]\n", jdx, __FILE__ );

	for (jdx = 0; jdx < zdx; jdx++) {
		OPTION *pOpt = Option(jdx, U);
		if (pOpt != NULL) {
			printf("\t%s%s\n", pOpt->opt, pOpt->dsc);
		}
	}
	printf( "\nExit status:\n"				\
		"\t0\tSUCCESS\t\tSuccessful execution\n"	\
		"\t1\tSYNTAX\t\tCommand syntax error\n"	\
		"\t2\tSYSTEM\t\tAny system issue\n"	\
		"\t3\tDISPLAY\t\tDisplay setup error\n" 	\
		"\t4\tVERSION\t\tMismatch API version\n"	\
		"\nReport bugs to labs[at]cyring.fr\n" );
}

int Command(int argc, char *argv[], uARG *U)
{
	GUI_REASON rc = GUI_SUCCESS;

    if (argc > 1)
    {
	OPTION *pOpt = NULL;
	int idx = 1, jdx;

	while ((rc == GUI_SUCCESS) && (idx < argc))
	{
	    for (jdx = 0;
		(rc == GUI_SUCCESS) && ((pOpt = Option(jdx, U)) != NULL);
			jdx++)
	    {
		ssize_t len = strlen(pOpt->opt) - strlen(argv[idx]);
		if (len == 0)
		{
			len = strlen(pOpt->opt);
		    if (strncmp(pOpt->opt, argv[idx], len) == 0)
		    {
			if ((++idx < argc) && (pOpt->var != NULL)) {
			    if (sscanf(argv[idx++], pOpt->fmt, pOpt->var) != 1)
			    {
				rc = GUI_SYNTAX;
			    }
			} else {
				rc = GUI_SYNTAX;
			}
			break;
		    }
		}
	    }
	    if((jdx == sizeof(Options) / sizeof(OPTION)) && (rc == GUI_SUCCESS))
	    {
		rc = GUI_SYNTAX;
	    }
	}
    }
	return (rc);
}

int main(int argc, char *argv[])
{
	GUI_REASON rc = GUI_SUCCESS;

	uARG U = {
		.Shutdown = 0x0,
		.M = { .Shm = NULL, .fd = -1 },
		.TID = { .SigHandler = 0, .Drawing = 0 },

		.A = AllocGUI()
	};

  if (U.A != NULL) {
	rc = Command(argc, argv, &U);
  } else {
	rc = GUI_SYSTEM;
  }
  if ((rc == GUI_SUCCESS)
	&& (XInitThreads() != 0)
		&& ((rc = OpenDisplay(U.A)) == GUI_SUCCESS))
  {
    if ((U.M.fd = shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1)
    {
		struct stat shmStat = {0};
	if (fstat(U.M.fd, &shmStat) != -1)
	{
	  if ((U.M.Shm = mmap(NULL, shmStat.st_size,
					PROT_READ|PROT_WRITE,MAP_SHARED,
					U.M.fd, 0)) != MAP_FAILED)
	  {
	    if (CHK_FOOTPRINT(U.M.Shm->FootPrint,	COREFREQ_MAJOR,
							COREFREQ_MINOR,
							COREFREQ_REV))
	    {
		ClientFollowService(	&localService,
					&U.M.Shm->Proc.Service, 0 );

		U.M.Shm->App.GUI = getpid();

		if ( (rc = StartGUI( U.A, "CoreFreq" ) ) == GUI_SUCCESS )
		{
			sigemptyset(&U.TID.Signal);
			sigaddset(&U.TID.Signal, SIGINT);  /* [CTRL] + [C] */
			sigaddset(&U.TID.Signal, SIGQUIT);
			sigaddset(&U.TID.Signal, SIGUSR1);
			sigaddset(&U.TID.Signal, SIGTERM);

		    if (pthread_sigmask(SIG_BLOCK, &U.TID.Signal, NULL) == 0) {
			pthread_create(&U.TID.SigHandler, NULL, Emergency, &U);
		    } else {
			rc = GUI_SYSTEM; /* Run without it however... */
		    }
		    if (pthread_create(&U.TID.Drawing, NULL, DrawLoop, &U) == 0)
		    {
			EventLoop(&U);

			pthread_join(U.TID.Drawing, NULL);
		    } else {
			rc = GUI_SYSTEM;
		    }
		    if (U.TID.SigHandler != 0)
		    {
			pthread_kill(U.TID.SigHandler, SIGUSR1);
			pthread_join(U.TID.SigHandler, NULL);
		    }
			StopGUI(U.A);
		}
		U.M.Shm->App.GUI = 0;
	    } else {
		rc = GUI_VERSION;
	    }
		munmap(U.M.Shm, shmStat.st_size);
	  } else {
		rc = GUI_SYSTEM;
	  }
	} else {
		rc = GUI_SYSTEM;
	}
    } else {
	rc = GUI_SYSTEM;
    }
	CloseDisplay(U.A);
  }
  if ((rc != GUI_SUCCESS) && (U.A != NULL)) {
	Help(&U);
  }
	FreeGUI(U.A);
	return (rc);
}

