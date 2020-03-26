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

#include "corefreq-gui-lib.h"
#include "corefreq_gui_main.xbm"

void CloseDisplay(xARG *A)
{
	enum MOUSE_CURSOR mc = MC_COUNT;
	do {
		mc--;
		if (A->mouseCursor[mc] != 0)
		{
			XFreeCursor(A->display, A->mouseCursor[mc]);
			A->mouseCursor[mc] = 0;
		}
	} while (mc);

	enum THEME thm = THEMES;
	do {
		thm--;
#ifdef HAVE_XFT
		if (A->font[thm].xft != NULL)
		{
			XftFontClose(A->display, A->font[thm].xft);
			A->font[thm].xft = NULL;
		}
#endif /* HAVE_XFT */
		if (A->font[thm].info != NULL)
		{
			XFreeFont(A->display, A->font[thm].info);
			A->font[thm].info = NULL;
		}
	} while (thm);

	if (A->display) {
		XCloseDisplay(A->display);
	}
}

GUI_REASON OpenXftFont(xARG *A, enum THEME thm,char *fontName,double *pointSize)
{
#ifdef HAVE_XFT
	A->font[thm].xft = XftFontOpen( A->display, A->_screen,
					XFT_FAMILY, XftTypeString,
					fontName,
					XFT_SIZE, XftTypeDouble,
					(*pointSize), NULL );

	return (A->font[thm].xft == NULL ? GUI_DISPLAY : GUI_SUCCESS);
#else
	return (GUI_DISPLAY);
#endif /* HAVE_XFT */
}

GUI_REASON OpenX11Font(xARG *A, enum THEME thm,char *fontName,double *pointSize)
{
	A->font[thm].info = XLoadQueryFont(A->display, fontName);

	return (A->font[thm].info == NULL ? GUI_DISPLAY : GUI_SUCCESS);
}

void FixFontPattern(xARG *A,enum THEME thm,char *pKind,char *pName,double *pPt)
{
	char *defaultKind[FT_COUNT] = {
		[FT_X11] = "x11"	, [FT_XFT] = "xft"
	};
	char *defaultName[FT_COUNT] = {
		[FT_X11] = "fixed"	, [FT_XFT] = "dejavu"
	};
	double defaultSize[FT_COUNT][THEMES] = {
		[FT_X11] = {
			[SMALL]  = DEFAULT_FONT_CHAR_WIDTH,
			[MEDIUM] = DEFAULT_FONT_CHAR_WIDTH,
			[LARGE]  = DEFAULT_FONT_CHAR_WIDTH
		},
		[FT_XFT] = {
			[SMALL]  = 8.0,
			[MEDIUM] = 12.0,
			[LARGE]  = 16.0
		}
	};
	char *fontKind = pKind;
	char *fontName = pName;
	double *pointSize = pPt;

	if (fontKind == NULL) {
#ifdef HAVE_XFT
		fontKind = defaultKind[A->font[thm].kind];
#else
		fontKind = defaultKind[FT_X11];
#endif /* HAVE_XFT */
	}
	if (fontName == NULL) {
#ifdef HAVE_XFT
		fontName = defaultName[A->font[thm].kind];
#else
		fontName = defaultName[FT_X11];
#endif /* HAVE_XFT */
	}
	if (pointSize == NULL) {
#ifdef HAVE_XFT
		pointSize = &(defaultSize[A->font[thm].kind][thm]);
#else
		pointSize = &(defaultSize[FT_X11][thm]);
#endif /* HAVE_XFT */
	}
	sprintf(A->font[thm].name,"%s:%s:%.1lf", fontKind,fontName,(*pointSize));
}

GUI_REASON OpenDisplay(xARG *A)
{
	GUI_REASON rc = GUI_SUCCESS;

    if ( (A->display = XOpenDisplay(NULL))
      && (A->screen = DefaultScreenOfDisplay(A->display)) )
    {
	GUI_REASON (*OpenFont[FT_COUNT])(xARG*, enum THEME, char*, double*) = {
			OpenX11Font,	OpenXftFont
	};

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
		rc = GUI_SYNTAX;
		/* Fallthrough */
	case '\0':
		break;
	}
	/* Try to load the requested fonts such as "fixed"	*/
	enum THEME thm;
	for (thm = SMALL; (rc == GUI_SUCCESS) && (thm < THEMES); thm++)
	{
		char *fontKind = NULL, *fontName = NULL;
		double pointSize = 8.0;
		int cnt = 0;
RETRY_PATTERN:
		cnt = sscanf(	A->font[thm].name, "%m[^:]:%m[^:]:%lf",
				&fontKind, &fontName, &pointSize );
		switch(cnt)
		{
		case 3:
			if ((fontKind != NULL) && (fontName != NULL)) {
				if (strncmp(fontKind, "xft", 3) == 0)
				{
					A->font[thm].kind = FT_XFT;
				}
				else if (strncmp(fontKind, "x11", 3) == 0)
				{
					A->font[thm].kind = FT_X11;
				}
				else {
					rc = GUI_SYNTAX;
				}
			} else {
					rc = GUI_SYSTEM;
				}
			break;
		case 2:
			FixFontPattern(A, thm, fontKind, fontName, &pointSize);
			goto RETRY_PATTERN;
		default:
			rc = GUI_SYNTAX;
			break;
		}
	    if (rc == GUI_SUCCESS) {
		rc = OpenFont[A->font[thm].kind](A, thm, fontName, &pointSize);
	    }
		if (fontKind != NULL) {
			free(fontKind);
		}
		if (fontName != NULL) {
			free(fontName);
		}
	}
    } else {
	rc = GUI_DISPLAY;
    }
	return (rc);
}

void CloseWidgets(xARG *A)
{
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

	enum THEME thm;
	for (thm = SMALL; thm < THEMES; thm++) {
#ifdef HAVE_XFT
		if (A->W.xft[thm].allocated == True) {
			XftColorFree(	A->display, A->visual, A->colormap,
					&A->W.xft[thm].color );
		}
#endif /* HAVE_XFT */
		if (A->W.gc[thm]) {
			XFreeGC(A->display, A->W.gc[thm]);
		}
	}
	if (A->W.window) {
		XDestroyWindow(A->display, A->W.window);
	}
}

GUI_REASON ComputeXftMetrics(xARG *A, enum THEME thm, const int trainingLength)
{
#ifdef HAVE_XFT
    if (A->font[thm].xft != NULL)
    {
	XftTextExtents8(A->display, A->font[thm].xft,
			(FcChar8 *) FONT_TRAINING_EXTENT, trainingLength,
			&A->font[thm].glyph);

	A->font[thm].metrics.ascent = A->font[thm].xft->ascent;
	A->font[thm].metrics.descent = A->font[thm].xft->descent;

	A->font[thm].metrics.overall.lbearing	= A->font[thm].glyph.x;
	A->font[thm].metrics.overall.rbearing	= A->font[thm].glyph.xOff;
	A->font[thm].metrics.overall.width	= A->font[thm].glyph.width;
	A->font[thm].metrics.overall.ascent	= A->font[thm].xft->ascent;
	A->font[thm].metrics.overall.descent	= A->font[thm].xft->descent;
	A->font[thm].metrics.overall.attributes = 0;

	return (GUI_SUCCESS);
    } else {
	return (GUI_DISPLAY);
    }
#else
	return (GUI_DISPLAY);
#endif /* HAVE_XFT */
}

GUI_REASON ComputeX11Metrics(xARG *A, enum THEME thm, const int trainingLength)
{
    if (A->font[thm].info != NULL)
    {
	if (XTextExtents(A->font[thm].info,
			FONT_TRAINING_EXTENT, trainingLength,
			&A->font[thm].metrics.dir,
			&A->font[thm].metrics.ascent,
			&A->font[thm].metrics.descent,
			&A->font[thm].metrics.overall) == Success)
	{
		XSetFont(A->display,A->W.gc[thm], A->font[thm].info->fid);
		return (GUI_SUCCESS);
	}
    }
	return (GUI_DISPLAY);
}

GUI_REASON OpenWidgets(xARG *A, const char *winTitle)
{
	GUI_REASON rc = GUI_SUCCESS;

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
	const int trainingLength = sizeof(FONT_TRAINING_EXTENT);

	enum THEME thm;
   for (thm = SMALL; (rc == GUI_SUCCESS) && (thm < THEMES); thm++)
   {
	A->W.gc[thm] = XCreateGC(A->display, A->W.window, 0, NULL);
    if (A->W.gc[thm])
    {
#ifdef HAVE_XFT
	if (A->font[thm].kind == FT_XFT) {
		rc = ComputeXftMetrics(A, thm, trainingLength);
	} else {
		rc = ComputeX11Metrics(A, thm, trainingLength);
	}
#else
	rc = ComputeX11Metrics(A, thm, trainingLength);
#endif /* HAVE_XFT */
	A->font[thm].metrics.charWidth = A->font[thm].metrics.overall.width;
	A->font[thm].metrics.charWidth /= trainingLength;

	A->font[thm].metrics.charHeight = A->font[thm].metrics.ascent
					+ A->font[thm].metrics.descent;
    } else {
	rc = GUI_DISPLAY;
    }
   }
  } else {
	rc = GUI_DISPLAY;
  }
  if (rc == GUI_SUCCESS)
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
	int _atom;
	for (_atom = 0; _atom < _COUNT; _atom++)
	{
		A->atom[_atom] = XInternAtom(	A->display,
						atoms[_atom].property,
						atoms[_atom].only_if_exists );
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
		rc = GUI_DISPLAY;
	}
#endif /* HAVE_XFT */
	if (rc == GUI_SUCCESS)
	{
		XSelectInput(A->display, A->W.window, EventProfile);

		XStoreName(A->display, A->W.window, winTitle);
		XSetIconName(A->display, A->W.window, winTitle);

		XMapWindow(A->display, A->W.window);
	}
    } else {
	rc = GUI_DISPLAY;
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
	xARG *A = calloc(1, sizeof(xARG));

	enum THEME thm;
    for (thm = SMALL; (thm < THEMES) && (A != NULL); thm++)
    {
	if ((A->font[thm].name = calloc(256, sizeof(char))) != NULL)
	{
		A->font[thm].metrics.overall.lbearing = DEFAULT_FONT_LBEARING;

		A->font[thm].metrics.overall.rbearing = corefreq_gui_main_width
							- DEFAULT_FONT_LBEARING;

		A->font[thm].metrics.overall.width = corefreq_gui_main_width;

		A->font[thm].metrics.overall.ascent	= DEFAULT_FONT_ASCENT
							- DEFAULT_FONT_DESCENT;

		A->font[thm].metrics.overall.descent	= DEFAULT_FONT_DESCENT;

		A->font[thm].metrics.dir	= DEFAULT_FONT_EXT_DIR;
		A->font[thm].metrics.ascent	= DEFAULT_FONT_ASCENT;
		A->font[thm].metrics.descent	= DEFAULT_FONT_DESCENT;
		A->font[thm].metrics.charWidth	= DEFAULT_FONT_CHAR_WIDTH;
		A->font[thm].metrics.charHeight = DEFAULT_FONT_CHAR_HEIGHT;
#ifdef HAVE_XFT
		A->font[thm].kind = FT_XFT;
#else
		A->font[thm].kind = FT_X11;
#endif
		A->W.color.background = _BACKGROUND_GLOBAL;
		A->W.color.foreground = _FOREGROUND_GLOBAL;

		A->W.width =	corefreq_gui_main_width;
		A->W.height =	corefreq_gui_main_height;
		A->W.border_width = 1;

		FixFontPattern(A, thm, NULL, NULL, NULL);

		continue;
	} else {
		do {
			free(A->font[thm].name);
		} while (thm-- != SMALL);

		free(A);
		A = NULL;

		break;
	}
    }
	return (A);
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

