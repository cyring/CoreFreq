/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef enum {
	GUI_SUCCESS	= 0,
	GUI_SYNTAX	= 1,
	GUI_SYSTEM	= 2,
	GUI_DISPLAY	= 3,
	GUI_VERSION	= 4,
} GUI_REASON;

#define _BACKGROUND_GLOBAL	0x2a0308
#define _FOREGROUND_GLOBAL	0x8fcefa

#define _COLOR_TEXT		0xf0f0f0
#define _COLOR_BAR		0xff0efa

enum MOUSE_CURSOR { MC_DEFAULT, MC_MOVE, MC_WAIT, MC_COUNT };

enum THEME { SMALL, MEDIUM, LARGE, THEMES };

enum FT_KIND { FT_X11, FT_XFT, FT_COUNT };

typedef struct _XWINDOW
{
	struct _XWINDOW *prev,
			*next;

	Window		window;
	struct {
		Pixmap	P,	/* Picture	*/
			B,	/* Background	*/
			F;	/* Foreground	*/
	} pixmap;
#ifdef HAVE_XFT
	struct {
		XftDraw *P,
			*B,
			*F;
	} drawable;
	struct {
		XRenderColor rgba;
		XftColor color;
		Bool allocated;
	} xft[THEMES];
#endif /* HAVE_XFT */
	struct {
    unsigned long int	background,
			foreground;
	} color;

	GC		gc[THEMES];

	int		x,
			y,
			width,
			height,
			border_width;
	struct
	{
		int	bitmask,
			xoffset,
			yoffset;
	} Position;
} XWINDOW;

/*		    L-CTRL	  L-ALT      R-CTRL	L-WIN	    R-ALTGR  */
#define AllModMask (ControlMask | Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask)

#define BASE_EVENTS	( KeyPressMask			\
			| ExposureMask			\
			| VisibilityChangeMask		\
			| StructureNotifyMask		\
			| FocusChangeMask )

#define MOVE_EVENTS	(ButtonReleaseMask | Button3MotionMask)

#define CLICK_EVENTS	ButtonPressMask

#define KEYINPUT_DEPTH	256

typedef struct
{
	Display 	*display;
	Screen		*screen;
	int		_screen,
			_depth;
	Visual		*visual;
	Colormap	colormap;
	XWINDOW 	W;
	Cursor		mouseCursor[MC_COUNT];
	Atom		atom[5];

    struct
    {	/*			FT_X11: Core fonts			*/
	char		*name;
	XFontStruct	*info;
#ifdef HAVE_XFT
	/*			FT_XFT: FreeType			*/
	XftFont 	*xft;
	XGlyphInfo	glyph;
#endif
	enum FT_KIND	kind;
	struct
	{
	    XCharStruct overall;
		int	dir,
			ascent,
			descent;
		double	charWidth,
			charHeight;
	} metrics;
    } font[THEMES];

	char		Xacl;
} xARG;

#define DEFAULT_FONT_LBEARING	1
#define DEFAULT_FONT_EXT_DIR	0
#define DEFAULT_FONT_ASCENT	11
#define DEFAULT_FONT_DESCENT	2
#define DEFAULT_FONT_CHAR_WIDTH 6
#define DEFAULT_FONT_CHAR_HEIGHT (DEFAULT_FONT_ASCENT + DEFAULT_FONT_DESCENT)
#define FONT_TRAINING_EXTENT	" !\"#$%&'()*+,-./0123456789:;<=>?@"	\
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`"	\
				"abcdefghijklmnopqrstuvwxyz{|}~"

#define Quarter_Char_Width(A,N) 	(A->font[N].metrics.charWidth / 4.0)
#define Half_Char_Width(A,N)		(A->font[N].metrics.charWidth / 2.0)
#define One_Char_Width(A,N)		(A->font[N].metrics.charWidth)

#define One_Half_Char_Width(A,N)	 (One_Char_Width(A,N)		\
					+ Half_Char_Width(A,N))

#define Twice_Char_Width(A,N)		(A->font[N].metrics.charWidth * 2.0)

#define Twice_Half_Char_Width(A,N)	 (Twice_Char_Width(A,N) 	\
					+ Half_Char_Width(A,N))

#define Quarter_Char_Height(A,N)	(A->font[N].metrics.charHeight / 4.0)
#define Half_Char_Height(A,N)		(A->font[N].metrics.charHeight / 2.0)
#define One_Char_Height(A,N)		(A->font[N].metrics.charHeight)

#define One_Half_Char_Height(A,N)	 (One_Char_Height(A,N)		\
					+ Half_Char_Height(A,N))

#define Twice_Char_Height(A,N)		(A->font[N].metrics.charHeight * 2.0)

#define Twice_Half_Char_Height(A,N)	 (Twice_Char_Height(A,N)	\
					+ Half_Char_Height(A,N))

#define Header_Height(A,N)		 (One_Char_Height(A,N)		\
					+ Quarter_Char_Height(A,N))

#define Footer_Height(A,N)		 (One_Char_Height(A,N)		\
					+ Half_Char_Height(A,N))

#define GEOMETRY_DEFAULT_COLS	80
#define GEOMETRY_DEFAULT_ROWS	32

typedef enum {
	GUI_NONE,
	GUI_EXIT,
	GUI_BUILD,
	GUI_MAP,
	GUI_DRAW,
	GUI_FLUSH,
	GUI_PAINT
} GUI_STEP;

extern void CloseDisplay(xARG *) ;
extern GUI_REASON OpenDisplay(xARG *) ;
extern void CloseWidgets(xARG *) ;
extern GUI_REASON OpenWidgets(xARG *, const char *) ;
extern void FreeGUI(xARG *) ;
extern xARG *AllocGUI(void) ;
extern GUI_STEP EventGUI(xARG *) ;

#ifdef HAVE_XFT
#define ConvertToRGBA( _rgba, _RGB )					\
({									\
	_rgba.red	= ( _RGB & 0xff0000 ) >> 8;			\
	_rgba.green	= ( _RGB & 0x00ff00 );				\
	_rgba.blue	= ( _RGB & 0x0000ff ) << 8;			\
	_rgba.alpha	= 0xffff;					\
})

#define ConvertFromRGBA( _RGB, _rgba )					\
(									\
	_RGB = ( _rgba.red << 8) | ( _rgba.green) | ( _rgba.blue >> 8)	\
)

#define ConditionFree( _A , _T )					\
({									\
	if (_A->W.xft[_T].allocated == True) {				\
		XftColorFree(	_A->display, _A->visual, _A->colormap,	\
				&_A->W.xft[_T].color ) ;		\
									\
		_A->W.xft[_T].allocated = False;			\
	}								\
})
#endif /* HAVE_XFT */


#define _SetFG_2xP( _A, _T, _RGB )					\
	XSetForeground( _A->display, _A->W.gc[_T], _RGB )

#ifdef HAVE_XFT
#define SetFG_2xP( _A, _T, _RGB )					\
({									\
    if ( _A->font[_T].kind == FT_XFT )					\
    {									\
	ConditionFree( _A , _T );					\
	ConvertToRGBA( _A->W.xft[_T].rgba, _RGB );			\
									\
	_A->W.xft[_T].allocated = XftColorAllocValue(	_A->display,	\
							_A->visual,	\
							_A->colormap,	\
						&_A->W.xft[_T].rgba,	\
						&_A->W.xft[_T].color ); \
									\
	_SetFG_2xP( _A, _T, _RGB );					\
    } else {								\
	_SetFG_2xP( _A, _T, _RGB );					\
    }									\
})
#else
#define SetFG_2xP( _A, _T, _RGB )					\
	_SetFG_2xP( _A, _T, _RGB )
#endif /* HAVE_XFT */


#define _SetFG_1xP( _A, _RGB )						\
	XSetForeground( _A->display, _A->W.gc[SMALL], _RGB )

#ifdef HAVE_XFT
#define SetFG_1xP( _A, _RGB )						\
({									\
    if ( _A->font[SMALL].kind == FT_XFT )				\
    {									\
	ConditionFree( _A , SMALL );					\
	ConvertToRGBA( _A->W.xft[SMALL].rgba, _RGB );			\
									\
	_A->W.xft[SMALL].allocated = XftColorAllocValue(_A->display,	\
							_A->visual,	\
							_A->colormap,	\
						&_A->W.xft[SMALL].rgba, \
						&_A->W.xft[SMALL].color);\
									\
	_SetFG_1xP( _A, _RGB ) ;					\
    } else {								\
	_SetFG_1xP( _A, _RGB ) ;					\
    }									\
})
#else
#define SetFG_1xP( _A, _RGB )						\
	_SetFG_1xP( _A, _RGB )
#endif /* HAVE_XFT */


#define _SetFG_0xP( _A ) 						\
	XSetForeground( _A->display, _A->W.gc[SMALL], _A->W.color.foreground)

#ifdef HAVE_XFT
#define SetFG_0xP( _A ) 						\
({									\
    if ( _A->font[SMALL].kind == FT_XFT )				\
    {									\
	ConditionFree( _A , SMALL );					\
	ConvertToRGBA( _A->W.xft[SMALL].rgba, _A->W.color.foreground ); \
									\
	_A->W.xft[SMALL].allocated = XftColorAllocValue(_A->display,	\
							_A->visual,	\
							_A->colormap,	\
						&_A->W.xft[SMALL].rgba, \
						&_A->W.xft[SMALL].color);\
									\
	_SetFG_0xP( _A );						\
    } else {								\
	_SetFG_0xP( _A );						\
    }									\
})
#else
#define SetFG_0xP( _A ) 						\
	_SetFG_0xP( _A)
#endif /* HAVE_XFT */

#define DISPATCH_SetFG(_1,_2,_3, _CURSOR, ... ) _CURSOR

#define SetFG( ... )							\
	DISPATCH_SetFG( __VA_ARGS__ ,		SetFG_2xP,	/*3*/	\
						SetFG_1xP,	/*2*/	\
						SetFG_0xP )	/*1*/	\
							( __VA_ARGS__ )


#define SetBG_2xP( _A, _T, _RGB )					\
	XSetBackground( _A->display, _A->W.gc[_T], _RGB )

#define SetBG_1xP( _A, _RGB )						\
	XSetBackground( _A->display, _A->W.gc[SMALL], _RGB )

#define SetBG_0xP( _A )							\
	XSetBackground( _A->display, _A->W.gc[SMALL], _A->W.color.background )

#define DISPATCH_SetBG(_1,_2,_3, _CURSOR, ... ) _CURSOR

#define SetBG( ... )							\
	DISPATCH_SetBG( __VA_ARGS__ ,		SetBG_2xP,	/*3*/	\
						SetBG_1xP,	/*2*/	\
						SetBG_0xP )	/*1*/	\
							( __VA_ARGS__ )


#define _DrawStr_6xP( _A, _P, _T, _x, _y, _txt, _len )			\
  XDrawString(_A->display, _A->W.pixmap._P, _A->W.gc[_T], _x, _y, _txt, _len)

#ifdef HAVE_XFT
#define DrawStr_6xP( _A, _P, _T, _x, _y, _txt, _len )			\
({									\
    if ( _A->font[_T].kind == FT_XFT )					\
    {									\
	XftDrawString8( _A->W.drawable._P, &_A->W.xft[_T].color,	\
			_A->font[_T].xft, _x, _y, (FcChar8 *) _txt, _len);\
    } else {								\
	_DrawStr_6xP(_A, _P, _T, _x, _y, _txt, _len);			\
    }									\
})
#else
#define DrawStr_6xP( _A, _P, _T, _x, _y, _txt, _len )			\
	_DrawStr_6xP(_A, _P, _T, _x, _y, _txt, _len)
#endif /* HAVE_XFT */


#define _DrawStr_5xP( _A, _P, _x, _y, _txt, _len )			\
  XDrawString(_A->display, _A->W.pixmap._P, _A->W.gc[SMALL], _x, _y, _txt, _len)

#ifdef HAVE_XFT
#define DrawStr_5xP( _A, _P, _x, _y, _txt, _len )			\
({									\
    if ( _A->font[SMALL].kind == FT_XFT )				\
    {									\
	XftDrawString8( _A->W.drawable._P, &_A->W.xft[SMALL].color,	\
			_A->font[SMALL].xft, _x, _y, (FcChar8 *) _txt, _len);\
    } else {								\
	_DrawStr_5xP(_A, _P, _x, _y, _txt, _len);			\
    }									\
})
#else
#define DrawStr_5xP( _A, _P, _x, _y, _txt, _len )			\
	_DrawStr_5xP(_A, _P, _x, _y, _txt, _len)
#endif /* HAVE_XFT */


#define _DrawStr_4xP( _A, _x, _y, _txt, _len )				\
  XDrawString(_A->display, _A->W.pixmap.B, _A->W.gc[SMALL], _x, _y, _txt, _len)

#ifdef HAVE_XFT
#define DrawStr_4xP( _A, _x, _y, _txt, _len )				\
({									\
    if ( _A->font[SMALL].kind == FT_XFT )				\
    {									\
	XftDrawString8( _A->W.drawable.B, &_A->W.xft[SMALL].color,	\
			_A->font[SMALL].xft, _x, _y, (FcChar8 *) _txt, _len);\
    } else {								\
	_DrawStr_4xP(_A, _x, _y, _txt, _len);				\
    }									\
})
#else
#define DrawStr_4xP( _A, _x, _y, _txt, _len )				\
	_DrawStr_4xP(_A, _x, _y, _txt, _len)
#endif /* HAVE_XFT */

#define DISPATCH_DrawStr(_1,_2,_3,_4,_5,_6,_7, _CURSOR, ... ) _CURSOR

#define DrawStr( ... )							\
	DISPATCH_DrawStr( __VA_ARGS__ , 	DrawStr_6xP,	/*7*/	\
						DrawStr_5xP,	/*6*/	\
						DrawStr_4xP,	/*5*/	\
						NULL,		/*4*/	\
						NULL,		/*3*/	\
						NULL,		/*2*/	\
						NULL )		/*1*/	\
							( __VA_ARGS__ )


#define FillRect_6xP( _A, _P, _T, _x, _y, _w, _h )			\
  XFillRectangle(_A->display, _A->W.pixmap._P, _A->W.gc[_T], _x, _y, _w, _h)

#define FillRect_5xP( _A, _P, _x, _y, _w, _h )				\
  XFillRectangle(_A->display, _A->W.pixmap._P, _A->W.gc[SMALL], _x, _y, _w, _h)

#define FillRect_4xP( _A, _P, _x, _y, _w, _h )				\
  XFillRectangle(_A->display, _A->W.pixmap.B, _A->W.gc[SMALL], _x, _y, _w, _h)

#define DISPATCH_FillRect(_1,_2,_3,_4,_5,_6,_7, _CURSOR, ... ) _CURSOR

#define FillRect( ... ) 						\
	DISPATCH_FillRect( __VA_ARGS__ , 	FillRect_6xP,	/*7*/	\
						FillRect_5xP,	/*6*/	\
						FillRect_4xP,	/*5*/	\
						NULL,		/*4*/	\
						NULL,		/*3*/	\
						NULL,		/*2*/	\
						NULL )		/*1*/	\
							( __VA_ARGS__ )

