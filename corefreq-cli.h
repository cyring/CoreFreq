/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef unsigned short int	CUINT;
typedef signed short int	CSINT;

#define powered(bit)	((bit) ? "Present" : "Missing")
#define enabled(bit)	((bit) ? "ON" : "OFF")

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

const char LCD[0x6][0x10][3][3] = {
	{
		{// 0x20
			"   ",
			"   ",
			"   "
		},
		{// 0x21
			" | ",
			" | ",
			" o "
		},
		{// 0x22
			"|| ",
			"   ",
			"   "
		},
		{// 0x23
			" //",
			"=/=",
			"// "
		},
		{// 0x24
			" _ ",
			"(|`",
			"_) "
		},
		{// 0x25
			"   ",
			"O/ ",
			"/O "
		},
		{// 0x26
			"_  ",
			"\\' ",
			"(\\ "
		},
		{// 0x27
			" | ",
			"   ",
			"   "
		},
		{// 0x28
			" / ",
			"|  ",
			" \\ "
		},
		{// 0x29
			" \\ ",
			"  |",
			" / "
		},
		{// 0x2a
			"   ",
			"\\|/",
			"/|\\"
		},
		{// 0x2b
			"   ",
			"_|_",
			" | "
		},
		{// 0x2c
			"   ",
			"   ",
			" / "
		},
		{// 0x2d
			"   ",
			"___",
			"   "
		},
		{// 0x2e
			"   ",
			"   ",
			" o "
		},
		{// 0x2f
			"  /",
			" / ",
			"/  "
		}
	},{
		{// 0x30
			" _ ",
			"|.|",
			"|_|"
		},
		{// 0x31
			"   ",
			" | ",
			" | "
		},
		{// 0x32
			" _ ",
			" _|",
			"|_ "
		},
		{// 0x33
			" _ ",
			" _|",
			" _|"
		},
		{// 0x34
			"   ",
			"|_|",
			"  |"
		},
		{// 0x35
			" _ ",
			"|_ ",
			" _|"
		},
		{// 0x36
			" _ ",
			"|_ ",
			"|_|"
		},
		{// 0x37
			" _ ",
			"  |",
			"  |"
		},
		{// 0x38
			" _ ",
			"|_|",
			"|_|"
		},
		{// 0x39
			" _ ",
			"|_|",
			" _|"
		},
		{// 0x3a
			"   ",
			" o ",
			" o "
		},
		{// 0x3b
			"   ",
			" o ",
			"/  "
		},
		{// 0x3c
			" / ",
			"<  ",
			" \\ "
		},
		{// 0x3d
			"___",
			"___",
			"   "
		},
		{// 0x3e
			" \\ ",
			"  >",
			" / "
		},
		{// 0x3f
			" _ ",
			"'_)",
			" ! "
		}
	},{
		{// 0x40
			" _ ",
			"(()",
			" ``"
		},
		{// 0x41
			" _ ",
			"|_|",
			"| |"
		},
		{// 0x42
			"__ ",
			"[_)",
			"[_)"
		},
		{// 0x43
			" _ ",
			"|  ",
			"|_ "
		},
		{// 0x44
			"__ ",
			"| |",
			"|_|"
		},
		{// 0x45
			" _ ",
			"|_ ",
			"|_ "
		},
		{// 0x46
			" __",
			"|- ",
			"|  "
		},
		{// 0x47
			" _ ",
			"|  ",
			"|_]"
		},
		{// 0x48
			"   ",
			"|_|",
			"| |"
		},
		{// 0x49
			" . ",
			" | ",
			" | "
		},
		{// 0x4a
			" . ",
			" | ",
			"_| "
		},
		{// 0x4b
			"   ",
			"|/ ",
			"|\\ "
		},
		{// 0x4c
			"   ",
			"|  ",
			"|_ "
		},
		{// 0x4d
			"   ",
			"|||",
			"| |"
		},
		{// 0x4e
			"   ",
			"|\\|",
			"| |"
		},
		{// 0x4f
			" _ ",
			"| |",
			"|_|"
		}
	},{
		{// 0x50
			" _ ",
			"|_|",
			"|  "
		},
		{// 0x51
			" _ ",
			"|\\|",
			"|_!"
		},
		{// 0x52
			" _ ",
			"|_|",
			"| \\"
		},
		{// 0x53
			" _ ",
			"(  ",
			"_) "
		},
		{// 0x54
			"___",
			" | ",
			" | "
		},
		{// 0x55
			"   ",
			"| |",
			"|_|"
		},
		{// 0x56
			"   ",
			"\\ /",
			" v "
		},
		{// 0x57
			"   ",
			"| |",
			"!^!"
		},
		{// 0x58
			"   ",
			"\\/ ",
			"/\\ "
		},
		{// 0x59
			"   ",
			"\\ /",
			" | "
		},
		{// 0x5a
			"__ ",
			" / ",
			"/_ "
		},
		{// 0x5b
			"  _",
			" | ",
			" |_"
		},
		{// 0x5c
			"\\  ",
			" \\ ",
			"  \\"
		},
		{// 0x5d
			"_  ",
			" | ",
			"_| "
		},
		{// 0x5e
			"/\\ ",
			"   ",
			"   "
		},
		{// 0x5f
			"   ",
			"   ",
			"___"
		}
	},{
		{// 0x60
			" \\ ",
			"   ",
			"   "
		},
		{// 0x61
			"   ",
			" _ ",
			"(_("
		},
		{// 0x62
			"   ",
			"|_ ",
			"|_)"
		},
		{// 0x63
			"   ",
			" _ ",
			"(_ "
		},
		{// 0x64
			"   ",
			" _|",
			"(_|"
		},
		{// 0x65
			"   ",
			" _ ",
			"(-'"
		},
		{// 0x66
			"   ",
			",- ",
			"|' "
		},
		{// 0x67
			"   ",
			",- ",
			"|] "
		},
		{// 0x68
			"   ",
			"|_ ",
			"| |"
		},
		{// 0x69
			"   ",
			" . ",
			" | "
		},
		{// 0x6a
			"   ",
			" . ",
			" ] "
		},
		{// 0x6b
			"   ",
			"., ",
			"|\\ "
		},
		{// 0x6c
			"   ",
			"_  ",
			"|_ "
		},
		{// 0x6d
			"   ",
			",,,",
			"|'|"
		},
		{// 0x6e
			"   ",
			", ,",
			"|\\|"
		},
		{// 0x6f
			"   ",
			" _ ",
			"|_|"
		}
	},{
		{// 0x70
			"   ",
			" _ ",
			"|-'"
		},
		{// 0x71
			"   ",
			" _ ",
			"|_!"
		},
		{// 0x72
			"   ",
			" _ ",
			"|\\'"
		},
		{// 0x73
			"   ",
			" _ ",
			"_) "
		},
		{// 0x74
			"   ",
			":_ ",
			"|  "
		},
		{// 0x75
			"   ",
			"   ",
			"|_|"
		},
		{// 0x76
			"   ",
			"   ",
			"\\/ "
		},
		{// 0x77
			"   ",
			", ,",
			"|^|"
		},
		{// 0x78
			"   ",
			"   ",
			">< "
		},
		{// 0x79
			"   ",
			"   ",
			"`/ "
		},
		{// 0x7a
			"   ",
			"_  ",
			"/_ "
		},
		{// 0x7b
			"  _",
			"_| ",
			" |_"
		},
		{// 0x7c
			" | ",
			" | ",
			" | "
		},
		{// 0x7d
			"_  ",
			" |_",
			"_| "
		},
		{// 0x7e
			"   ",
			"   ",
			" ~ "
		},
		{// 0x7f
			"   ",
			".^.",
			"DEL"
		}
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

#define MARGIN_WIDTH	2
#define MARGIN_HEIGHT	1
#define INTER_WIDTH	3
#define INTER_HEIGHT	(3 + 1)
#define LEADING_LEFT	(MIN_WIDTH / (MARGIN_WIDTH + (4 * INTER_WIDTH)))
#define LEADING_TOP	1

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

int GetKey(SCANKEY *scan, struct timespec *tsec)
{
	struct pollfd fds = {.fd = STDIN_FILENO, .events = POLLIN};
	int rp = 0, rz = 0;

	if ((rp = ppoll(&fds, 1, tsec, NULL)) > 0)
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

void HookCellFunc(TCELLFUNC *with, TCELLFUNC what) { *with=what; }

void HookKeyFunc(KEYFUNC *with, KEYFUNC what) { *with=what; }

void HookWinFunc(WINFUNC *with, WINFUNC what) { *with=what; }

void HookAttrib(ATTRIBUTE *with, ATTRIBUTE what) { with->value=what.value; }

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

	layer->attr = calloc(len, sizeof(ATTRIBUTE));
	layer->code = calloc(len, sizeof(ASCII));
    }
}

#define ResetLayer(layer)						\
	memset(layer->attr, 0, layer->size.wth * layer->size.hth);	\
	memset(layer->code, 0, layer->size.wth * layer->size.hth);

void FillLayerArea(Layer *layer,CUINT col, CUINT row,
				CUINT width, CUINT height,
				ASCII *source, ATTRIBUTE attrib)
{
	CUINT _row;
	for (_row = row; _row < row + height; _row++)
		LayerFillAt(layer, col, _row, width, source, attrib);
}

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

void AllocCopyAttr(TCell *cell, ATTRIBUTE attrib[])
{
	if ((attrib != NULL) && (cell->attr = malloc(cell->length)) != NULL)
		memcpy(&cell->attr->value, &attrib->value, cell->length);
}

void AllocFillAttr(TCell *cell, ATTRIBUTE attrib)
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

	    ATTRIBUTE	select[2] = {
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
	ATTRIBUTE border = win->hook.color[(GetFocus(list) == win)].border;

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
	case SCANKEY_SHIFT_RIGHT:
	case SCANKEY_SHIFT_d:
		if (win->hook.key.WinRight != NULL)
			win->hook.key.WinRight(win);
		break;
	case SCANKEY_SHIFT_LEFT:
	case SCANKEY_SHIFT_a:	/* AZERTY */
	case SCANKEY_SHIFT_q:	/* QWERTY */
		if (win->hook.key.WinLeft != NULL)
			win->hook.key.WinLeft(win);
		break;
	case SCANKEY_SHIFT_UP:
	case SCANKEY_SHIFT_w:	/* QWERTY */
	case SCANKEY_SHIFT_z:	/* AZERTY */
		if (win->hook.key.WinUp != NULL)
			win->hook.key.WinUp(win);
		break;
	case SCANKEY_SHIFT_DOWN:
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

void PrintWindowStack(WinList *winList)
{
	Window *walker;
	if ((walker = GetHead(winList)) != NULL) {
		do {
			walker = walker->next;

			if (walker->hook.Print != NULL)
				walker->hook.Print(walker, winList);
			else
				ForEachCellPrint(walker, winList);
		} while (!IsHead(winList, walker)) ;
	}
}

#define Threshold(value, threshold1, threshold2, _low, _medium, _high)	\
({									\
	enum PALETTE _ret;						\
	if (value > threshold2)						\
		_ret = _high;						\
	else if (value > threshold1)					\
		_ret = _medium;						\
	else								\
		_ret = _low;						\
	_ret;								\
})

void PrintLCD(Layer *layer, CUINT col, CUINT row,
			int len, char *pStr, enum PALETTE lcdColor)
{
	int j = len;
	do {
		int offset = col + (len - j) * 3,
			lo = pStr[len - j] & 0b00001111,
			hi = pStr[len - j] & 0b11110000;
			hi = (hi >> 4) - 0x2;

		LayerFillAt(layer, offset, row,				\
				3, LCD[hi][lo][0],			\
				MakeAttr(lcdColor, 0, BLACK, 1));

		LayerFillAt(layer, offset, (row + 1),			\
				3, LCD[hi][lo][1],			\
				MakeAttr(lcdColor, 0, BLACK, 1));

		LayerFillAt(layer, offset, (row + 2),			\
				3, LCD[hi][lo][2],			\
				MakeAttr(lcdColor, 0, BLACK, 1));
		j--;
	} while (j > 0) ;
}

int ByteReDim(unsigned long ival, int constraint, unsigned long *oval)
{
	int base = 1 + (int) log10(ival);

	(*oval) = ival;
	if (base > constraint) {
		(*oval) = (*oval) >> 10;
		return(1 + ByteReDim((*oval), constraint, oval));
	} else
		return(0);
}

#define Prolog								\
	printf(SCP SCR1 CUH CLS HIDE);					\
									\
	struct termios oldt, newt;					\
	tcgetattr(STDIN_FILENO, &oldt);					\
	newt = oldt;							\
	newt.c_lflag &= ~( ICANON | ECHO );				\
	newt.c_cc[VTIME] = 0;						\
	newt.c_cc[VMIN] = 0;						\
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);


#define Epilog								\
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);			\
									\
	printf(SHOW SCR0 RCP COLOR(0,9,9));
