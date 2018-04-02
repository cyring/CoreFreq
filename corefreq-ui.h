/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define TOP_HEADER_ROW	3
#define TOP_FOOTER_ROW	2
#define TOP_SEPARATOR	3
#define MAX_CPU_ROW	64

#define MAX_HEIGHT	((2 * MAX_CPU_ROW)				\
			+ TOP_HEADER_ROW				\
			+ TOP_SEPARATOR					\
			+ TOP_FOOTER_ROW)
#define MAX_WIDTH	132
#define MIN_WIDTH	80

#define CUMAX(M, m)	((M) > (m) ? (M) : (m))
#define CUMIN(m, M)	((m) < (M) ? (m) : (M))

typedef unsigned short int	CUINT;
typedef signed short int	CSINT;

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

enum PALETTE {
	_BLACK,
	_RED,
	_GREEN,
	_YELLOW,
	_BLUE,
	_MAGENTA,
	_CYAN,
	_WHITE
};

#define BLACK	0
#define RED	1
#define GREEN	2
#define YELLOW	3
#define BLUE	4
#define MAGENTA 5
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

typedef union {
	unsigned long long key;
	unsigned char code[8];
} SCANKEY;

#define SCANKEY_VOID		0x8000000000000000
#define SCANKEY_NULL		0x0000000000000000
#define SCANKEY_TAB		0x0000000000000009
#define SCANKEY_ENTER		0x000000000000000a
#define SCANKEY_ESC		0x000000000000001b
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
#define SCANKEY_F10		0x0000007e31325b1b
#define SCANKEY_SHIFT_TAB	0x00000000005a5b1b
#define SCANKEY_PGUP		0x000000007e355b1b
#define SCANKEY_PGDW		0x000000007e365b1b
#define SCANKEY_HASH		0x0000000000000023
#define SCANKEY_PERCENT		0x0000000000000025
#define SCANKEY_PLUS		0x000000000000002b
#define SCANKEY_MINUS		0x000000000000002d
#define SCANKEY_DOT		0x000000000000002e
#define SCANKEY_SHIFT_UP	0x000041323b315b1b
#define SCANKEY_SHIFT_DOWN	0x000042323b315b1b
#define SCANKEY_SHIFT_RIGHT	0x000043323b315b1b
#define SCANKEY_SHIFT_LEFT	0x000044323b315b1b
#define SCANKEY_SHIFT_a		0x0000000000000041
#define SCANKEY_SHIFT_d		0x0000000000000044
#define SCANKEY_SHIFT_i		0x0000000000000049
#define SCANKEY_SHIFT_m		0x000000000000004d
#define SCANKEY_SHIFT_q		0x0000000000000051
#define SCANKEY_SHIFT_r		0x0000000000000052
#define SCANKEY_SHIFT_s		0x0000000000000053
#define SCANKEY_SHIFT_t		0x0000000000000054
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
#define SCANKEY_OPEN_BRACE	0x000000000000007b
#define SCANKEY_CLOSE_BRACE	0x000000000000007d

#define SCANCON_HOME		0x000000007e315b1b
#define SCANCON_END		0x000000007e345b1b
#define SCANCON_F1		0x00000000415b5b1b
#define SCANCON_F2		0x00000000425b5b1b
#define SCANCON_F3		0x00000000435b5b1b
#define SCANCON_F4		0x00000000445b5b1b
#define SCANCON_SHIFT_TAB	0x000000000000091b

typedef struct {
	int	width,
		height;
} SCREEN_SIZE;

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
} ATTRIBUTE;

#define MakeAttr(_fg, _un, _bg, _bf)					\
	({ATTRIBUTE _attr={.fg = _fg,.un = _un,.bg = _bg,.bf = _bf}; _attr;})

#define HDK	{.fg = BLACK,	.bg = BLACK,	.bf = 1}
#define HRK	{.fg = RED,	.bg = BLACK,	.bf = 1}
#define HGK	{.fg = GREEN,	.bg = BLACK,	.bf = 1}
#define HYK	{.fg = YELLOW,	.bg = BLACK,	.bf = 1}
#define HBK	{.fg = BLUE,	.bg = BLACK,	.bf = 1}
#define HCK	{.fg = CYAN,	.bg = BLACK,	.bf = 1}
#define HWK	{.fg = WHITE,	.bg = BLACK,	.bf = 1}
#define HKB	{.fg = BLACK,	.bg = BLUE,	.bf = 1}
#define HWB	{.fg = WHITE,	.bg = BLUE,	.bf = 1}
#define HWC	{.fg = WHITE,	.bg = CYAN,	.bf = 1}
#define HKW	{.fg = BLACK,	.bg = WHITE,	.bf = 1}
#define _HCK	{.fg = CYAN,	.bg = BLACK,	.un = 1,	.bf = 1}
#define _HWK	{.fg = WHITE,	.bg = BLACK,	.un = 1,	.bf = 1}
#define _HWB	{.fg = WHITE,	.bg = BLUE,	.un = 1,	.bf = 1}
#define _HKW	{.fg = BLACK,	.bg = WHITE,	.un = 1,	.bf = 1}
#define LDK	{.fg = BLACK,	.bg = BLACK}
#define LRK	{.fg = RED,	.bg = BLACK}
#define LGK	{.fg = GREEN,	.bg = BLACK}
#define LYK	{.fg = YELLOW,	.bg = BLACK}
#define LBK	{.fg = BLUE,	.bg = BLACK}
#define LCK	{.fg = CYAN,	.bg = BLACK}
#define LWK	{.fg = WHITE,	.bg = BLACK}
#define LWB	{.fg = WHITE,	.bg = BLUE}
#define LKC	{.fg = BLACK,	.bg = CYAN}
#define LKW	{.fg = BLACK,	.bg = WHITE}
#define LRW	{.fg = RED,	.bg = WHITE}
#define LBW	{.fg = BLUE,	.bg = WHITE}
#define _LCK	{.fg = CYAN,	.bg = BLACK,	.un = 1}
#define _LWK	{.fg = WHITE,	.bg = BLACK,	.un = 1}
#define _LKW	{.fg = BLACK,	.bg = WHITE,	.un = 1}
#define _LBW	{.fg = BLUE,	.bg = WHITE,	.un = 1}

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
		ATTRIBUTE	attr[_len];				\
		ASCII		code[_len];				\
	}

typedef struct {
	ASCII		*code;
	ATTRIBUTE	*attr;
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
	ATTRIBUTE	*attr;
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
	      ATTRIBUTE select,
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

extern Layer	*sLayer,
		*dLayer,
		*wLayer;

extern WinList winList;

int GetKey(SCANKEY *scan, struct timespec *tsec) ;

SCREEN_SIZE GetScreenSize(void) ;

void HookCellFunc(TCELLFUNC *with, TCELLFUNC what) ;

void HookKeyFunc(KEYFUNC *with, KEYFUNC what) ;

void HookWinFunc(WINFUNC *with, WINFUNC what) ;

void HookAttrib(ATTRIBUTE *with, ATTRIBUTE what) ;

void HookString(REGSTR *with, REGSTR what) ;

void HookPointer(REGPTR *with, REGPTR what) ;

#define StoreWindow(win, with, what)					\
(									\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(TCELLFUNC)), HookCellFunc,	\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(KEYFUNC)), HookKeyFunc,		\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(WINFUNC)), HookWinFunc,		\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(win->hook with), typeof(ATTRIBUTE)), HookAttrib,		\
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

void DestroyLayer(Layer *layer) ;

void CreateLayer(Layer *layer, CoordSize size) ;

#define ResetLayer(layer)						\
	memset(layer->attr, 0, layer->size.wth * layer->size.hth);	\
	memset(layer->code, 0, layer->size.wth * layer->size.hth);

#define ClearGarbage(_layer, _plane, _col, _row, _len)			\
	memset(&LayerAt(_layer, _plane, _col, _row), 0, _len)

void FillLayerArea(Layer *layer,CUINT col, CUINT row,
				CUINT width, CUINT height,
				ASCII *source, ATTRIBUTE attrib) ;

void AllocCopyAttr(TCell *cell, ATTRIBUTE attrib[]) ;

void AllocFillAttr(TCell *cell, ATTRIBUTE attrib) ;

void AllocCopyItem(TCell *cell, ASCII *item) ;

void FreeAllTCells(Window *win) ;

#define StoreTCell(win, shortkey, item, attrib)				\
({									\
    if (item != NULL) {							\
	win->dim++;							\
	win->lazyComp.bottomRow = (win->dim / win->matrix.size.wth)	\
				- win->matrix.size.hth;			\
									\
      if ((win->cell = realloc(win->cell,sizeof(TCell) * win->dim)) != NULL)\
      {									\
	win->cell[win->dim - 1].quick.key = shortkey;			\
	win->cell[win->dim - 1].length = strlen((char *)item);		\
									\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		typeof(attrib), typeof(ATTRIBUTE[])), AllocCopyAttr,	\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		typeof(attrib), typeof(ATTRIBUTE)), AllocFillAttr,	\
	(void)0))							\
		(&(win->cell[win->dim - 1]), attrib);			\
									\
	AllocCopyItem(&win->cell[win->dim - 1], (ASCII *)item);		\
      }									\
    }									\
})

void DestroyWindow(Window *win) ;

Window *CreateWindow(	Layer *layer, unsigned long long id,
			CUINT width, CUINT height,
			CUINT oCol, CUINT oRow) ;

void RemoveWindow(Window *win, WinList *list) ;

void AppendWindow(Window *win, WinList *list) ;

void DestroyAllWindows(WinList *list) ;

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

void AnimateWindow(int rotate, WinList *list) ;

Window *SearchWinListById(unsigned long long id, WinList *list) ;

void PrintContent(Window *win, WinList *list, CUINT col, CUINT row) ;

void ForEachCellPrint(Window *win, WinList *list) ;

void EraseWindowWithBorder(Window *win) ;

void PrintLCD(	Layer *layer, CUINT col, CUINT row,
		int len, char *pStr, enum PALETTE lcdColor) ;

void MotionReset_Win(Window *win) ;

void MotionLeft_Win(Window *win) ;

void MotionRight_Win(Window *win) ;

void MotionUp_Win(Window *win) ;

void MotionDown_Win(Window *win) ;

void MotionHome_Win(Window *win) ;

void MotionEnd_Win(Window *win) ;

void MotionPgUp_Win(Window *win) ;

void MotionPgDw_Win(Window *win) ;

void MotionOriginLeft_Win(Window *win) ;

void MotionOriginRight_Win(Window *win) ;

void MotionOriginUp_Win(Window *win) ;

void MotionOriginDown_Win(Window *win) ;

int Motion_Trigger(SCANKEY *scan, Window *win, WinList *list) ;

void PrintWindowStack(WinList *winList) ;

void FreeAll(char *buffer) ;

void AllocAll(char **buffer) ;

void WriteConsole(SCREEN_SIZE drawSize, char *buffer) ;

void _TERMINAL_IN(void) ;

void _TERMINAL_OUT(void) ;

#define TERMINAL(IO)	_TERMINAL_##IO()
