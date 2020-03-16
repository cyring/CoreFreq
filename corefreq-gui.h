/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef enum { FALSE, TRUE } BOOL;

enum {
	BACKGROUND_MAIN,
	FOREGROUND_MAIN,
	COLOR_FOCUS,
	COLOR_COUNT
};

#define _BACKGROUND_GLOBAL	0x2a0308
#define _FOREGROUND_GLOBAL	0x8fcefa

#define _BACKGROUND_MAIN	0x2a0308
#define _FOREGROUND_MAIN	0x8fcefa
#define _COLOR_FOCUS		0xffffff
#define _COLOR_BAR		0xff0efa

typedef struct
{
	unsigned long int	RGB;
	char			*xrmClass,
				*xrmKey;
} COLORS;

enum { MC_DEFAULT, MC_MOVE, MC_WAIT, MC_COUNT };

typedef struct
{
	Window		window;
	struct {
		Pixmap	B,
			F;
	} pixmap;

	struct {
	unsigned long int	background,
				foreground;
	} color;

	GC		gc;

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
	struct
	{
	    XCharStruct overall;
		int	dir,
			ascent,
			descent,
			charWidth,
			charHeight;
	} extents;
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

#define DEFAULT_FONT_ASCENT	11
#define DEFAULT_FONT_DESCENT	2
#define DEFAULT_FONT_CHAR_WIDTH 6
#define DEFAULT_FONT_CHAR_HEIGHT (DEFAULT_FONT_ASCENT + DEFAULT_FONT_DESCENT)
#define DEFAULT_HEADER_STR	".1.2.3.4.5.6.7.8.9.0.1.2.3.4.5.6.7.8.9.0" \
				".1.2.3.4.5.6.7.8.9.0.1.2.3.4.5.6.7.8.9.0" \
				".1.2.3.4.5.6.7.8.9.0.1.2.3.4.5.6.7.8.9.0"

#define Quarter_Char_Width(N)	(A->W[N].extents.charWidth >> 2)
#define Half_Char_Width(N)	(A->W[N].extents.charWidth >> 1)
#define One_Char_Width(N)	(A->W[N].extents.charWidth)
#define One_Half_Char_Width(N)	(One_Char_Width(N) + Half_Char_Width(N))
#define Twice_Char_Width(N)	(A->W[N].extents.charWidth << 1)
#define Twice_Half_Char_Width(N) (Twice_Char_Width(N) + Half_Char_Width(N))

#define Quarter_Char_Height(N)	(A->W[N].extents.charHeight >> 2)
#define Half_Char_Height(N)	(A->W[N].extents.charHeight >> 1)
#define One_Char_Height(N)	(A->W[N].extents.charHeight)
#define One_Half_Char_Height(N) (One_Char_Height(N) + Half_Char_Height(N))
#define Twice_Char_Height(N)	(A->W[N].extents.charHeight << 1)
#define Twice_Half_Char_Height(N) (Twice_Char_Height(N) + Half_Char_Height(N))

#define Header_Height(N)	(One_Char_Height(N) + Quarter_Char_Height(N))
#define Footer_Height(N)	(One_Char_Height(N) + Half_Char_Height(N))

#define GEOMETRY_MAIN_COLS	80
#define GEOMETRY_MAIN_ROWS	32

#define MAIN_TEXT_WIDTH 	GEOMETRY_MAIN_COLS
#define MAIN_TEXT_HEIGHT	GEOMETRY_MAIN_ROWS

#define KEYINPUT_DEPTH		256

typedef enum {MAIN, WIDGETS} LAYOUTS;

typedef struct
{
	Bit64 Shutdown	__attribute__ ((aligned (8)));
    struct
    {
	SHM_STRUCT	*Shm;
	int		fd;
    } M;

	Display		*display;
	Screen		*screen;
    struct
    {
	char		**List,
			*Name;
	XFontStruct	*Info;
	int		Count,
			Index;
    } font;
	Cursor		MouseCursor[MC_COUNT];
	XWINDOW		W[WIDGETS];
	pthread_t	TID_SigHandler,
			TID_Draw;
	char		xACL;
} uARG;

/*
	The bouble buffering is as the following sequence:
*/
/* step 1 : draw the static graphics into the background pixmap.	*/
void	BuildLayout(uARG *A, int G) ;
/* step 2 : copy the background into the foreground pixmap.		*/
void	MapLayout(uARG *A, int G) ;
/* step 3 : add the animated graphics into the foreground pixmap.	*/
void	DrawLayout(uARG *A, int G) ;
/* step 4 : copy the foreground into the display window
		(taking care of the scrolling pixmap).			*/
void	FlushLayout(uARG *A, int G) ;
/* loop to step 2 to avoid the execution of the building process.	*/

/* All-in-One macro.							*/
#define Paint(N, DoBuild, DoDraw) {		\
	if (DoBuild) {	BuildLayout(A, N); }	\
	MapLayout(A, N);			\
	if (DoDraw) {	DrawLayout(A, N); }	\
	FlushLayout(A, N);			\
}

