/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <stdlib.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xresource.h>
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreq-gui-lib.h"
#include "corefreq_gui_main.xbm"

void CloseDisplay(xARG *A)
{
	int idx = MC_COUNT;
	do {
		idx--;
		if (A->mouseCursor[idx] != 0)
		{
			XFreeCursor(A->display, A->mouseCursor[idx]);
			A->mouseCursor[idx] = 0;
		}
	} while (idx);

	idx = THEMES;
	do {
		idx--;
		if (A->font[idx].info != NULL)
		{
			XFreeFont(A->display, A->font[idx].info);
			A->font[idx].info = NULL;
		}
	} while (idx);

	if (A->display) {
		XCloseDisplay(A->display);
	}
}

int OpenDisplay(xARG *A)
{
	int rc = 0, idx;

    if ( (A->display = XOpenDisplay(NULL))
      && (A->screen = DefaultScreenOfDisplay(A->display)) )
    {
	A->_screen  =	DefaultScreen(A->display);
	A->_depth   =	DefaultDepthOfScreen(A->screen);
	A->visual   =	DefaultVisual(A->display, A->_screen);
	A->colormap =	DefaultColormap(A->display, A->_screen);

	A->mouseCursor[MC_DEFAULT]=XCreateFontCursor(A->display, XC_left_ptr);
	A->mouseCursor[MC_MOVE] = XCreateFontCursor(A->display, XC_fleur);
	A->mouseCursor[MC_WAIT] = XCreateFontCursor(A->display, XC_watch);

	switch (A->Xacl) {
	case 'Y':
	case 'y':
	case '1':
		XEnableAccessControl(A->display);
		break;
	case 'N':
	case 'n':
	case '0':
		XDisableAccessControl(A->display);
		break;
	default:
		rc = 1;
		/* Fallthrough */
	case '\0':
		break;
	}
	/* Try to load the requested fonts such as "fixed"	*/
      for (idx = 0; (rc == 0) && (idx < THEMES); idx++)
      {
#ifdef HAVE_XFT
	if (strlen(A->font[idx].name) == 0) {
		strcpy(A->font[idx].name, "dejavu");
	}
	A->font[idx].xft = XftFontOpen( A->display, A->_screen,
					XFT_FAMILY, XftTypeString,
					A->font[idx].name,
					XFT_SIZE, XftTypeDouble,
					idx == SMALL ?	 8.0 : \
					idx == MEDIUM ? 12.0 : \
					idx == LARGE ?	16.0 : 8.0, NULL );
	if (A->font[idx].xft == NULL) {
		rc = 3;
	}
#else
	if (strlen(A->font[idx].name) == 0) {
		strcpy(A->font[idx].name, "fixed");
	}
	A->font[idx].info = XLoadQueryFont(A->display, A->font[idx].name);
	if (A->font[idx].info == NULL) {
		rc = 3;
	}
#endif
      }
    } else {
	rc = 3;
    }
	return (rc);
}

void CloseWidgets(xARG *A)
{
	int idx;

#ifdef HAVE_XFT
	if (A->W.drawable.P != NULL) {
		XftDrawDestroy(A->W.drawable.P);
	}
	if (A->W.drawable.B != NULL) {
		XftDrawDestroy(A->W.drawable.B);
	}
	if (A->W.drawable.F != NULL) {
		XftDrawDestroy(A->W.drawable.F);
	}
#endif /* HAVE_XFT */
	if (A->W.pixmap.P) {
		XFreePixmap(A->display, A->W.pixmap.P);
	}
	if (A->W.pixmap.B) {
		XFreePixmap(A->display, A->W.pixmap.B);
	}
	if (A->W.pixmap.F) {
		XFreePixmap(A->display, A->W.pixmap.F);
	}
	for (idx = 0; idx < THEMES; idx++) {
#ifdef HAVE_XFT
		if (A->W.xft[idx].allocated == True) {
			XftColorFree(	A->display, A->visual, A->colormap,
					&A->W.xft[idx].color );
		}
#endif /* HAVE_XFT */
		if (A->W.gc[idx]) {
			XFreeGC(A->display, A->W.gc[idx]);
		}
	}
	if (A->W.window) {
		XDestroyWindow(A->display, A->W.window);
	}
}

int OpenWidgets(xARG *A, const char *winTitle)
{
	int rc = 0, idx;

	XSetWindowAttributes swa = {
	.background_pixmap = None,
	.background_pixel = A->W.color.background,
	.border_pixmap	= CopyFromParent,
	.border_pixel	= A->W.color.foreground,
	.bit_gravity	= 0,
	.win_gravity	= 0,
	.backing_store	= DoesBackingStore(A->screen),
	.backing_planes = AllPlanes,
	.backing_pixel	= 0,
	.save_under	= DoesSaveUnders(A->screen),
	.event_mask	= EventMaskOfScreen(A->screen),
	.do_not_propagate_mask = 0,
	.override_redirect = False,
	.colormap	= A->colormap,
	.cursor 	= A->mouseCursor[MC_DEFAULT]
	};

  if ( (A->W.window = XCreateWindow(A->display,
				DefaultRootWindow(A->display),
				A->W.x, A->W.y,
				A->W.width, A->W.height,
				A->W.border_width,
				CopyFromParent,
				InputOutput,
				CopyFromParent,
				CWBorderPixel | CWOverrideRedirect | CWCursor,
				&swa)) )
  {
	const int trainingSize = sizeof(FONT_TRAINING_EXTENT);

   for (idx = 0; (rc == 0) && (idx < THEMES); idx++)
   {
	A->W.gc[idx] = XCreateGC(A->display, A->W.window, 0, NULL);
    if (A->W.gc[idx])
    {
#ifdef HAVE_XFT
      if (A->font[idx].xft != NULL)
      {
	XftTextExtents8(A->display, A->font[idx].xft,
			(FcChar8 *) FONT_TRAINING_EXTENT, trainingSize,
			&A->font[idx].glyphInfo);

	A->font[idx].extents.ascent = A->font[idx].xft->ascent;
	A->font[idx].extents.descent = A->font[idx].xft->descent;
	A->font[idx].extents.charWidth = A->font[idx].xft->max_advance_width;
      } else {
	rc = 3;
      }
#else
      if (A->font[idx].info != NULL)
      {
	XTextExtents(	A->font[idx].info,
			FONT_TRAINING_EXTENT, trainingSize,
			&A->font[idx].extents.dir,
			&A->font[idx].extents.ascent,
			&A->font[idx].extents.descent,
			&A->font[idx].extents.overall );

	A->font[idx].extents.charWidth = A->font[idx].info->max_bounds.rbearing
					- A->font[idx].info->min_bounds.lbearing;

	XSetFont(A->display,A->W.gc[idx], A->font[idx].info->fid);
      } else {
	rc = 3;
      }
#endif /* HAVE_XFT */

	A->font[idx].extents.charHeight = A->font[idx].extents.ascent
					+ A->font[idx].extents.descent;
    } else {
	rc = 3;
    }
   }
  } else {
	rc = 3;
  }
  if (rc == 0)
  {
    if ((A->W.pixmap.B = XCreatePixmap( A->display, A->W.window,
					A->W.width, A->W.height,
					A->_depth) )
     && (A->W.pixmap.F = XCreatePixmap( A->display, A->W.window,
					A->W.width, A->W.height,
					A->_depth) )
     && (A->W.pixmap.P = XCreateBitmapFromData( A->display,
						A->W.window,
				(const char*)	corefreq_gui_main_bits,
						A->W.width, A->W.height )))
    {
	const long EventProfile = BASE_EVENTS|CLICK_EVENTS|MOVE_EVENTS;

	#define _COUNT (sizeof(A->atom) / sizeof(Atom))
	const struct {
		char	*property;
		Bool	only_if_exists;
	} atoms[_COUNT] = {
		{	"WM_DELETE_WINDOW",		False	},
		{	"_MOTIF_WM_HINTS",		True	},
		{	"_NET_WM_STATE",		False	},
		{	"_NET_WM_STATE_ABOVE",		False	},
		{	"_NET_WM_STATE_SKIP_TASKBAR",	False	}
	};

	for (idx = 0; idx < _COUNT; idx++)
	{
		A->atom[idx] = XInternAtom(	A->display,
						atoms[idx].property,
						atoms[idx].only_if_exists );
	}
	XSetWMProtocols(A->display, A->W.window, A->atom, _COUNT);
	#undef _COUNT

#ifdef HAVE_XFT
	if(((A->W.drawable.P = XftDrawCreate(A->display, A->W.pixmap.P,
					A->visual, A->colormap)) == NULL)

	|| ((A->W.drawable.B = XftDrawCreate(A->display, A->W.pixmap.B,
					A->visual, A->colormap)) == NULL)

	|| ((A->W.drawable.F = XftDrawCreate(A->display, A->W.pixmap.F,
					A->visual, A->colormap)) == NULL))
	{
		rc = 3;
	}
#endif /* HAVE_XFT */
	if (rc == 0)
	{
		XSelectInput(A->display, A->W.window, EventProfile);

		XStoreName(A->display, A->W.window, winTitle);
		XSetIconName(A->display, A->W.window, winTitle);

		XMapWindow(A->display, A->W.window);
	}
    } else {
	rc = 3;
    }
  }
	return (rc);
}

void FreeGUI(xARG *A)
{
	if (A != NULL) {
		if (A->font[SMALL].name != NULL) {
			free(A->font[SMALL].name);
		}
		if (A->font[MEDIUM].name != NULL) {
			free(A->font[MEDIUM].name);
		}
		if (A->font[LARGE].name != NULL) {
			free(A->font[LARGE].name);
		}
		free(A);
	}
}

xARG *AllocGUI(void)
{
	xARG *pARG = malloc(sizeof(xARG));
  if (pARG != NULL)
  {
	xARG A = \
    {
	.display  = NULL,
	.screen   = NULL,
	._screen  = 0,
	._depth   = 0,
	.visual   = NULL,
	.colormap = 0,

	.font = {
	[SMALL] = {
		.name = calloc(256, sizeof(char)),
		.info = NULL,
		.extents = {
		.overall = {
			.lbearing = DEFAULT_FONT_LBEARING,
			.rbearing=corefreq_gui_main_width-DEFAULT_FONT_LBEARING,
			.width = corefreq_gui_main_width,
			.ascent = DEFAULT_FONT_ASCENT - DEFAULT_FONT_DESCENT,
			.descent = DEFAULT_FONT_DESCENT,
			.attributes = 0
		    },
			.dir = DEFAULT_FONT_EXT_DIR,
			.ascent = DEFAULT_FONT_ASCENT,
			.descent= DEFAULT_FONT_DESCENT,
			.charWidth = DEFAULT_FONT_CHAR_WIDTH,
			.charHeight= DEFAULT_FONT_CHAR_HEIGHT
		},
#ifdef HAVE_XFT
		.xft	= NULL,
#endif
	    },
	[MEDIUM] = {
		.name = calloc(256, sizeof(char)),
		.info = NULL,
		.extents = {
		.overall = {
			.lbearing = DEFAULT_FONT_LBEARING,
			.rbearing=corefreq_gui_main_width-DEFAULT_FONT_LBEARING,
			.width = corefreq_gui_main_width,
			.ascent = DEFAULT_FONT_ASCENT - DEFAULT_FONT_DESCENT,
			.descent = DEFAULT_FONT_DESCENT,
			.attributes = 0
		    },
			.dir = DEFAULT_FONT_EXT_DIR,
			.ascent = DEFAULT_FONT_ASCENT,
			.descent= DEFAULT_FONT_DESCENT,
			.charWidth = DEFAULT_FONT_CHAR_WIDTH,
			.charHeight= DEFAULT_FONT_CHAR_HEIGHT
		},
#ifdef HAVE_XFT
		.xft	= NULL,
#endif
	    },
	[LARGE] = {
		.name = calloc(256, sizeof(char)),
		.info = NULL,
		.extents = {
		.overall = {
			.lbearing = DEFAULT_FONT_LBEARING,
			.rbearing=corefreq_gui_main_width-DEFAULT_FONT_LBEARING,
			.width = corefreq_gui_main_width,
			.ascent = DEFAULT_FONT_ASCENT - DEFAULT_FONT_DESCENT,
			.descent = DEFAULT_FONT_DESCENT,
			.attributes = 0
		    },
			.dir = DEFAULT_FONT_EXT_DIR,
			.ascent = DEFAULT_FONT_ASCENT,
			.descent= DEFAULT_FONT_DESCENT,
			.charWidth = DEFAULT_FONT_CHAR_WIDTH,
			.charHeight= DEFAULT_FONT_CHAR_HEIGHT
		},
#ifdef HAVE_XFT
		.xft	= NULL,
#endif
	    },
	 },
	.mouseCursor = { [MC_DEFAULT] = 0, [MC_MOVE] = 0, [MC_WAIT] = 0 },
	.W = {
		.window = 0,
		.pixmap = { .P = 0, .B = 0, .F = 0 },
#ifdef HAVE_XFT
		.drawable = { .P = NULL, .B = NULL, .F = NULL },
		.xft = {
			[SMALL]  = { .rgba={0}, .color={0}, .allocated=False },
			[MEDIUM] = { .rgba={0}, .color={0}, .allocated=False },
			[LARGE]  = { .rgba={0}, .color={0}, .allocated=False }
		},
#endif
		.color = {
			.background = _BACKGROUND_GLOBAL,
			.foreground = _FOREGROUND_GLOBAL
		},
		.gc = { [SMALL] = 0, [MEDIUM] = 0, [LARGE] = 0 },
		.x = +0,
		.y = +0,
		.width = corefreq_gui_main_width,
		.height= corefreq_gui_main_height,
		.border_width = 1,
		.Position = {.bitmask = 0x0, .xoffset = 0, .yoffset = 0}
	},
	.Xacl = '\0'
    };
	memcpy(pARG, &A, sizeof(xARG));
  }
	return (pARG);
}

GUI_STEP EventGUI(xARG *A)
{
	XEvent E = {0};

	XNextEvent(A->display, &E);

    if (E.xany.window == A->W.window)
    {
	switch (E.type) {
	case Expose:
		if (!E.xexpose.count) {
			return (GUI_FLUSH);
		}
	    break;
	case KeyPress:
		switch (XLookupKeysym(&E.xkey, 0)) {
		case XK_x:
			if (E.xkey.state & ControlMask) {
				return (GUI_EXIT);
			}
			break;
		}
	    break;
	case ResizeRequest:
		A->W.width = E.xresizerequest.width;
		A->W.height= E.xresizerequest.height;

		XMoveResizeWindow(A->display,
				A->W.window,
				A->W.x,
				A->W.y,
				A->W.width,
				A->W.height);
	    break;
	case MotionNotify:
		A->W.x = E.xmotion.x;
		A->W.y = E.xmotion.y;

		XMoveResizeWindow(A->display,
				A->W.window,
				A->W.x,
				A->W.y,
				A->W.width,
				A->W.height);
	    break;
	case ConfigureNotify:
		A->W.x = E.xconfigure.x;
		A->W.y = E.xconfigure.y;
		A->W.width = E.xconfigure.width;
		A->W.height= E.xconfigure.height;
		A->W.border_width = E.xconfigure.border_width;
	    break;
	case FocusIn:
		XSetWindowBorder(A->display, A->W.window,
				A->W.color.foreground);
	    break;
	case FocusOut:
		XSetWindowBorder(A->display, A->W.window,
				A->W.color.foreground);
	    break;
	case ClientMessage:
		if(E.xclient.data.l[0] != A->atom[0]) {
			break;
		}
		/* Fallthrough */
	case DestroyNotify:
		return (GUI_EXIT);
	case UnmapNotify:
		return (GUI_PAINT);
	case MapNotify:
		return (GUI_PAINT);
	}
    }
	return (GUI_NONE);
}

