/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xresource.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
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

void CloseDisplay(uARG *A)
{
	int MC = MC_COUNT;
	do {
		MC-- ;
		if (A->MouseCursor[MC])
		{
			XFreeCursor(A->display, A->MouseCursor[MC]);
			A->MouseCursor[MC] = 0;
		}
	} while (MC);

	if (A->font.List != NULL)
	{
		XFreeFontNames(A->font.List);
		A->font.List = NULL;
		A->font.Count=0;
		A->font.Index=0;
	}
	if (A->font.Info)
	{
		XFreeFont(A->display, A->font.Info);
		A->font.Info = NULL;
	}
	if (A->display) {
		XCloseDisplay(A->display);
	}
}

BOOL OpenDisplay(uARG *A)
{
	BOOL noerr = TRUE;

    if ( (A->display = XOpenDisplay(NULL))
      && (A->screen = DefaultScreenOfDisplay(A->display)) )
    {
      switch (A->xACL) {
      case 'Y':
      case 'y':
		XEnableAccessControl(A->display);
		break;
      case 'N':
      case 'n':
		XDisableAccessControl(A->display);
		break;
      }
	// Try to load the requested font.
      if (strlen(A->font.Name) == 0) {
		strcpy(A->font.Name, "fixed");
      }
      if ((A->font.Info = XLoadQueryFont(A->display, A->font.Name)) == NULL) {
	noerr = FALSE;
      }
      if (noerr)
      {
	A->MouseCursor[MC_DEFAULT]=XCreateFontCursor(A->display, XC_left_ptr);
	A->MouseCursor[MC_MOVE]=XCreateFontCursor(A->display, XC_fleur);
	A->MouseCursor[MC_WAIT]=XCreateFontCursor(A->display, XC_watch);
      }
    } else {
	noerr = FALSE;
    }
	return (noerr);
}

void	CloseWidgets(uARG *A)
{
	XFreePixmap(A->display, A->W[MAIN].pixmap.B);
	XFreePixmap(A->display, A->W[MAIN].pixmap.F);
	XFreeGC(A->display, A->W[MAIN].gc);
	XDestroyWindow(A->display, A->W[MAIN].window);
}

BOOL OpenWidgets(uARG *A)
{
	BOOL noerr = TRUE;

	XSetWindowAttributes swa = {
	.background_pixmap = None,
	.background_pixel = A->W[MAIN].color.background,
	.border_pixmap	= CopyFromParent,
	.border_pixel	= A->W[MAIN].color.foreground,
	.bit_gravity	= 0,
	.win_gravity	= 0,
	.backing_store	= DoesBackingStore(DefaultScreenOfDisplay(A->display)),
	.backing_planes = AllPlanes,
	.backing_pixel	= 0,
	.save_under	= DoesSaveUnders(DefaultScreenOfDisplay(A->display)),
	.event_mask	= EventMaskOfScreen(DefaultScreenOfDisplay(A->display)),
	.do_not_propagate_mask = 0,
	.override_redirect = False,
	.colormap	= DefaultColormap(A->display, DefaultScreen(A->display)),
	.cursor 	= A->MouseCursor[MC_DEFAULT]
	};

    if ((A->W[MAIN].window = XCreateWindow(A->display,
				DefaultRootWindow(A->display),
				A->W[MAIN].x, A->W[MAIN].y,
				A->W[MAIN].width, A->W[MAIN].height,
				A->W[MAIN].border_width,
				CopyFromParent,
				InputOutput,
				CopyFromParent,
				CWBorderPixel | CWOverrideRedirect | CWCursor,
				&swa)) )
    {
      if ((A->W[MAIN].gc = XCreateGC(A->display, A->W[MAIN].window, 0, NULL)))
      {
	XSetFont(A->display, A->W[MAIN].gc, A->font.Info->fid);

	XTextExtents(	A->font.Info, DEFAULT_HEADER_STR, MAIN_TEXT_WIDTH,
			&A->W[MAIN].extents.dir, &A->W[MAIN].extents.ascent,
			&A->W[MAIN].extents.descent,&A->W[MAIN].extents.overall);

	A->W[MAIN].extents.charWidth = A->font.Info->max_bounds.rbearing
					- A->font.Info->min_bounds.lbearing;

	A->W[MAIN].extents.charHeight = A->W[MAIN].extents.ascent
					+ A->W[MAIN].extents.descent;

	if ((A->W[MAIN].pixmap.B = XCreatePixmap(
					A->display, A->W[MAIN].window,
					A->W[MAIN].width, A->W[MAIN].height,
					DefaultDepthOfScreen(A->screen)
				))
	 && (A->W[MAIN].pixmap.F = XCreatePixmap(
					A->display, A->W[MAIN].window,
					A->W[MAIN].width, A->W[MAIN].height,
					DefaultDepthOfScreen(A->screen)
				)))
	{
		const long EventProfile = BASE_EVENTS|CLICK_EVENTS|MOVE_EVENTS;

		XSelectInput(A->display, A->W[MAIN].window, EventProfile);
	}
      } else {
	noerr = FALSE;
      }
    } else {
	noerr = FALSE;
    }
	return (noerr);
}

void BuildLayout(uARG *A, int G)
{
	size_t len = strlen(A->M.Shm->Proc.Brand);
	const int x = ( A->W[MAIN].extents.overall.width
			- (One_Char_Width(MAIN) * len) ) / 2;

	XSetBackground(A->display, A->W[G].gc, A->W[MAIN].color.background);
	XSetForeground(A->display, A->W[G].gc, A->W[MAIN].color.background);
	/* Clear entirely the background.				*/
	XFillRectangle(A->display, A->W[G].pixmap.B, A->W[G].gc,
			0, 0, A->W[G].width, A->W[G].height);
	/* Processor specification.					*/
	XSetForeground(A->display, A->W[G].gc, A->W[MAIN].color.foreground);

	XDrawString(	A->display, A->W[G].pixmap.B, A->W[G].gc,
			x, One_Char_Height(G),
			A->M.Shm->Proc.Brand, len );
	/* Columns header						*/
	XDrawString(	A->display, A->W[G].pixmap.B, A->W[G].gc,
			One_Char_Width(MAIN), Twice_Half_Char_Height(G),
			"CPU", 3 );

	XDrawString(	A->display, A->W[G].pixmap.B, A->W[G].gc,
			A->W[MAIN].extents.overall.width-((8+1)
			* One_Char_Width(MAIN)),
			Twice_Half_Char_Height(G),
			"Freq[Mhz]", 9 );
}

void MapLayout(uARG *A, int G)
{
	XCopyArea(A->display, A->W[G].pixmap.B, A->W[G].pixmap.F, A->W[G].gc,
			0, 0, A->W[G].width, A->W[G].height, 0, 0);
}

void FlushLayout(uARG *A, int G)
{
	XCopyArea(A->display, A->W[G].pixmap.F, A->W[G].window, A->W[G].gc,
			0, 0, A->W[G].width, A->W[G].height, 0, 0);

	XFlush(A->display);
}

void DrawLayout(uARG *A, int G)
{
	char str[16];
	unsigned int cpu;

  for (cpu = 0; cpu < A->M.Shm->Proc.CPU.Count; cpu++)
  {
	struct FLIP_FLOP *CFlop = \
		&A->M.Shm->Cpu[cpu].FlipFlop[
			!A->M.Shm->Cpu[cpu].Toggle
	];
	const int x = One_Char_Width(MAIN),
	y = One_Char_Height(MAIN) + (Twice_Char_Height(MAIN) * (cpu + 1)),
	width = ( (A->W[MAIN].extents.overall.width - Twice_Char_Width(MAIN))
		* CFlop->Relative.Ratio ) / A->M.Shm->Proc.Boost[BOOST(1C)],
	height = One_Half_Char_Height(MAIN);

	snprintf(str, 16, "%03u%7.2f", cpu, CFlop->Relative.Freq);

    if (CFlop->Relative.Ratio >= A->M.Shm->Proc.Boost[BOOST(MAX)]) {
	XSetForeground(A->display, A->W[MAIN].gc, _COLOR_BAR);
    } else {
	XSetForeground(A->display, A->W[MAIN].gc, A->W[MAIN].color.foreground);
    }

	XFillRectangle( A->display,
			A->W[MAIN].pixmap.F,
			A->W[MAIN].gc,
			x, y, width, height );

	XSetForeground(A->display, A->W[MAIN].gc, _COLOR_FOCUS);

	XDrawString(	A->display,
			A->W[MAIN].pixmap.F,
			A->W[MAIN].gc,
			One_Char_Width(MAIN),
			y + One_Char_Height(MAIN),
			str, 3 );

	XDrawString(	A->display,
			A->W[MAIN].pixmap.F,
			A->W[MAIN].gc,
			A->W[MAIN].extents.overall.width-((7+1)
			* One_Char_Width(MAIN)),
			y + One_Char_Height(MAIN),
			&str[3], 7);
  }
}

static void *DrawLoop(void *uArg)
{
	uARG *A = (uARG *) uArg;

	ClientFollowService(&localService, &A->M.Shm->Proc.Service, 0);

    while (!BITVAL(A->Shutdown, SYNC))
    {
	if (BITCLR(LOCKLESS, A->M.Shm->Proc.Sync, SYNC1) == 0) {
		nanosleep(&A->M.Shm->Sleep.pollingWait, NULL);
	} else {
		Paint(MAIN, FALSE, TRUE);
	}
	if (BITCLR(LOCKLESS, A->M.Shm->Proc.Sync, NTFY1)) {
		ClientFollowService(&localService, &A->M.Shm->Proc.Service, 0);
	}
    }
	return(NULL);
}

static void *EventLoop(uARG *A)
{
	XEvent E = {0};

	while (!BITVAL(A->Shutdown, SYNC))
	{
		XNextEvent(A->display, &E);

	    if (E.xany.window == A->W[MAIN].window)
	    {
		switch (E.type) {
		case Expose: {
			if (!E.xexpose.count) {
				FlushLayout(A, MAIN);
			}
		    }
		    break;
		case KeyPress: {
			switch (XLookupKeysym(&E.xkey, 0)) {
			case XK_x:
			    if (E.xkey.state & ControlMask) {
				BITSET(LOCKLESS, A->Shutdown, SYNC);
			    }
				break;
			}
		    }
		    break;
		case ResizeRequest:
			A->W[MAIN].width = E.xresizerequest.width;
			A->W[MAIN].height= E.xresizerequest.height;

			XMoveResizeWindow(A->display,
					A->W[MAIN].window,
					A->W[MAIN].x,
					A->W[MAIN].y,
					A->W[MAIN].width,
					A->W[MAIN].height);
		    break;
		case MotionNotify:
			A->W[MAIN].x = E.xmotion.x;
			A->W[MAIN].y = E.xmotion.y;

			XMoveResizeWindow(A->display,
					A->W[MAIN].window,
					A->W[MAIN].x,
					A->W[MAIN].y,
					A->W[MAIN].width,
					A->W[MAIN].height);
		    break;
		case ConfigureNotify:
			A->W[MAIN].x = E.xconfigure.x;
			A->W[MAIN].y = E.xconfigure.y;
			A->W[MAIN].width = E.xconfigure.width;
			A->W[MAIN].height= E.xconfigure.height;
			A->W[MAIN].border_width = E.xconfigure.border_width;
		    break;
		case FocusIn:
			XSetWindowBorder(A->display, A->W[MAIN].window,
					A->W[MAIN].color.foreground);
		    break;
		case FocusOut:
			XSetWindowBorder(A->display, A->W[MAIN].window,
					A->W[MAIN].color.foreground);
		    break;
		case DestroyNotify:
			BITSET(LOCKLESS, A->Shutdown, SYNC);
		    break;
		case UnmapNotify: {
			Paint(MAIN, TRUE, FALSE);
		}
		    break;
		case MapNotify: {
			Paint(MAIN, TRUE, FALSE);
		    }
		}
	    }
	}
	return (NULL);
}

int main(int argc, char *argv[])
{
	uARG A =	\
    {
	.Shutdown = 0x0,
	.M = { .Shm = NULL, .fd = -1 },
	.display = NULL,
	.screen = NULL,
	.TID_SigHandler = 0,
	.TID_Draw = 0,
	.font = {
		.List = NULL,
		.Name = calloc(256, sizeof(char)),
		.Info = NULL,
		.Count = 0xfff,
		.Index = 0,
		},
	.MouseCursor = {0},
	.W = {
	[MAIN] = {
		.window = 0,
		.pixmap ={ .B = 0, .F = 0 },
		.color = {
			.background = _BACKGROUND_GLOBAL,
			.foreground = _FOREGROUND_GLOBAL
		},
		.gc = 0,
		.x = +0,
		.y = +0,
		.width = (GEOMETRY_MAIN_COLS * DEFAULT_FONT_CHAR_WIDTH),
		.height= (3 * DEFAULT_FONT_CHAR_HEIGHT),
		.border_width = 1,
		.Position = {.bitmask = 0x0, .xoffset = 0, .yoffset = 0},
		.extents = {
			.overall = {0},
			.dir = 0,
			.ascent = DEFAULT_FONT_ASCENT,
			.descent= DEFAULT_FONT_DESCENT,
			.charWidth = DEFAULT_FONT_CHAR_WIDTH,
			.charHeight= DEFAULT_FONT_CHAR_HEIGHT
		    },
	    },
	},
	.xACL = 'N'
    };

  if ((XInitThreads() != 0) && (OpenDisplay(&A) == TRUE))
  {
    if ((A.M.fd = shm_open(SHM_FILENAME, O_RDWR,
			S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) !=-1)
    {
		struct stat shmStat = {0};
	if (fstat(A.M.fd, &shmStat) != -1)
	{
	  if ((A.M.Shm = mmap(NULL, shmStat.st_size,
					PROT_READ|PROT_WRITE,MAP_SHARED,
					A.M.fd, 0)) != MAP_FAILED)
	  {
	    if (CHK_FOOTPRINT(A.M.Shm->FootPrint,	COREFREQ_MAJOR,
							COREFREQ_MINOR,
							COREFREQ_REV))
	    {
		ClientFollowService(	&localService,
					&A.M.Shm->Proc.Service, 0 );

		A.M.Shm->App.GUI = getpid();

		A.W[MAIN].height = A.W[MAIN].height
				+ (A.M.Shm->Proc.CPU.Count
				* (2 * DEFAULT_FONT_CHAR_HEIGHT));

		if (OpenWidgets(&A) == TRUE)
		{
			const char *winTitle = "CoreFreq";

			XStoreName(A.display, A.W[MAIN].window, winTitle);
			XSetIconName(A.display, A.W[MAIN].window, winTitle);
			XMapWindow(A.display, A.W[MAIN].window);

		    if (pthread_create(&A.TID_Draw, NULL, DrawLoop, &A) == 0)
		    {
			EventLoop(&A);

			pthread_join(A.TID_Draw, NULL);
		    }
			CloseWidgets(&A);
		}
		A.M.Shm->App.GUI = 0;
	    }
		munmap(A.M.Shm, shmStat.st_size);
	  }
	}
    }
	CloseDisplay(&A);
  }
  if (A.font.Name != NULL) {
	free(A.font.Name);
  }
	return (0);
}

