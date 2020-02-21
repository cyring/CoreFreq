/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define TOP_HEADER_ROW	3
#define TOP_FOOTER_ROW	2
#define TOP_SEPARATOR	3

#define MAX_HEIGHT	((2 * CORE_COUNT)				\
			+ TOP_HEADER_ROW				\
			+ TOP_SEPARATOR					\
			+ TOP_FOOTER_ROW)
#define MAX_WIDTH	320
#define MIN_WIDTH	80

#define CUMAX(M, m)	((M) > (m) ? (M) : (m))
#define CUMIN(m, M)	((m) < (M) ? (m) : (M))

typedef unsigned short int	CUINT;
typedef signed short int	CSINT;

/* VT100 requirements.							*/
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
#define _LF	"\x0a"
#define _FF	"\x0c"

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

#define VOID_COLOR 							\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define HSPACE	"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "	\
		"        ""        ""        ""        ""        "

#define HBAR	"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"	\
		"||||||||""||||||||""||||||||""||||||||""||||||||"

#define HLINE	"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"	\
		"--------""--------""--------""--------""--------"

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
#define SCANKEY_EXCL		0x0000000000000021
#define SCANKEY_HASH		0x0000000000000023
#define SCANKEY_DOLLAR		0x0000000000000024
#define SCANKEY_PERCENT 	0x0000000000000025
#define SCANKEY_AST		0x000000000000002a
#define SCANKEY_PLUS		0x000000000000002b
#define SCANKEY_MINUS		0x000000000000002d
#define SCANKEY_DOT		0x000000000000002e
#define SCANKEY_SHIFT_UP	0x000041323b315b1b
#define SCANKEY_SHIFT_DOWN	0x000042323b315b1b
#define SCANKEY_SHIFT_RIGHT	0x000043323b315b1b
#define SCANKEY_SHIFT_LEFT	0x000044323b315b1b
#define SCANKEY_ALT_UP		0x000041333b315b1b
#define SCANKEY_ALT_DOWN	0x000042333b315b1b
#define SCANKEY_SHIFT_a 	0x0000000000000041
#define SCANKEY_SHIFT_b 	0x0000000000000042
#define SCANKEY_SHIFT_c 	0x0000000000000043
#define SCANKEY_SHIFT_d 	0x0000000000000044
#define SCANKEY_SHIFT_f 	0x0000000000000046
#define SCANKEY_SHIFT_h 	0x0000000000000048
#define SCANKEY_SHIFT_i 	0x0000000000000049
#define SCANKEY_SHIFT_l 	0x000000000000004c
#define SCANKEY_SHIFT_m 	0x000000000000004d
#define SCANKEY_SHIFT_o 	0x000000000000004f
#define SCANKEY_SHIFT_q 	0x0000000000000051
#define SCANKEY_SHIFT_r 	0x0000000000000052
#define SCANKEY_SHIFT_s 	0x0000000000000053
#define SCANKEY_SHIFT_t 	0x0000000000000054
#define SCANKEY_SHIFT_v 	0x0000000000000056
#define SCANKEY_SHIFT_w 	0x0000000000000057
#define SCANKEY_SHIFT_y 	0x0000000000000059
#define SCANKEY_SHIFT_z 	0x000000000000005a
#define SCANKEY_CTRL_p		0x0000000000000010
#define SCANKEY_CTRL_u		0x0000000000000015
#define SCANKEY_CTRL_x		0x0000000000000018
#define SCANKEY_ALT_SHIFT_a	0x00000000000081c3
#define SCANKEY_ALT_SHIFT_s	0x00000000000093c3
#define SCANKEY_ALT_SHIFT_z	0x0000000000009ac3
#define SCANKEY_ALT_a		0x000000000000a1c3
#define SCANKEY_ALT_p		0x000000000000b0c3
#define SCANKEY_ALT_s		0x000000000000b3c3
#define SCANKEY_ALT_z		0x000000000000bac3
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
#define SCANCON_ALT_a		0x000000000000611b
#define SCANCON_ALT_p		0x000000000000701b
#define SCANCON_ALT_s		0x000000000000731b
#define SCANCON_ALT_z		0x0000000000007a1b

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
#define HMK	{.fg = MAGENTA, .bg = BLACK,	.bf = 1}
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
#define LMK	{.fg = MAGENTA, .bg = BLACK}
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

#define MAKE_TITLE_UNFOCUS	MakeAttr(BLACK, 0, BLUE , 1)
#define MAKE_TITLE_FOCUS	MakeAttr(WHITE, 0, CYAN , 1)
#define MAKE_BORDER_UNFOCUS	MakeAttr(BLACK, 0, BLUE , 1)
#define MAKE_BORDER_FOCUS	MakeAttr(WHITE, 0, BLUE , 1)
#define MAKE_SELECT_UNFOCUS	MakeAttr(BLACK, 0, BLACK, 1)
#define MAKE_SELECT_FOCUS	MakeAttr(BLACK, 0, CYAN , 0)
#define MAKE_PRINT_UNFOCUS	MakeAttr(WHITE, 0, BLACK, 0)
#define MAKE_PRINT_FOCUS	MakeAttr(WHITE, 0, BLACK, 1)
#define MAKE_PRINT_DROP 	MakeAttr(BLACK, 0, WHITE, 0)

typedef unsigned char	ASCII;

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

typedef union {
	void			*pvoid;
	unsigned long long	*pullong;
	signed long long	*psllong;
	unsigned long		*pulong;
	signed long		*pslong;
	unsigned int		*puint;
	signed int		*psint;

	unsigned long long	ullong;
	signed long long	sllong;
	unsigned long		ulong;
	signed long		slong;

	struct {
		unsigned int	uint[2];
	};
	struct {
		signed int	sint[2];
	};
} DATA_TYPE;

typedef struct _Grid {
	TCell		cell;
	DATA_TYPE	data;
	void		(*Update)(struct _Grid *grid, DATA_TYPE data);
} TGrid;

extern void Set_pVOID(TGrid *pGrid	, void *pVOID) ;
extern void Set_pULLONG(TGrid *pGrid	, unsigned long long *pULLONG) ;
extern void Set_pSLLONG(TGrid *pGrid	, signed long long *pSLLONG) ;
extern void Set_pULONG(TGrid *pGrid	, unsigned long *pULONG) ;
extern void Set_pSLONG(TGrid *pGrid	, signed long *pSLONG) ;
extern void Set_pUINT(TGrid *pGrid	, unsigned int *pUINT) ;
extern void Set_pSINT(TGrid *pGrid	, signed int *pSINT) ;
extern void Set_ULLONG(TGrid *pGrid	, unsigned long long _ULLONG) ;
extern void Set_SLLONG(TGrid *pGrid	, signed long long _SLLONG) ;
extern void Set_ULONG(TGrid *pGrid	, unsigned long _ULONG) ;
extern void Set_SLONG(TGrid *pGrid	, signed long _SLONG) ;
extern void Set_UINT(TGrid *pGrid	, unsigned int _UINT) ;
extern void Set_SINT(TGrid *pGrid	, signed int _SINT) ;

#define SET_DATA(_pGrid , _data)					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(void *)) ,		\
			Set_pVOID,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(unsigned long long *)),	\
			Set_pULLONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(signed long long *)),	\
			Set_pSLLONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(unsigned long *)) ,	\
			Set_pULONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(signed long *)),		\
			Set_pSLONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(unsigned int *)) ,	\
			Set_pUINT,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(signed int *)),		\
			Set_pSINT,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(unsigned long long)) ,	\
			Set_ULLONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(signed long long)) ,	\
			Set_SLLONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(unsigned long)) , 	\
			Set_ULONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(signed long)) ,		\
			Set_SLONG,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(unsigned int)) ,		\
			Set_UINT,					\
	__builtin_choose_expr(__builtin_types_compatible_p (		\
		__typeof__(_data), __typeof__(signed int)) ,		\
			Set_SINT,					\
	(void)0)))))))))))))(_pGrid, _data)

typedef struct _Stock {
	struct _Stock	*next;

	unsigned long long id;

	struct Geometry {
		Coordinate origin;
	} geometry;
} Stock;

typedef struct {
	Stock	*head,
		*tail;
} StockList;

typedef enum {
	WINFLAG_NO_FLAGS = 0,
	WINMASK_NO_STOCK = 0,
	WINFLAG_NO_STOCK = 1,
	WINMASK_NO_SCALE = 1,
	WINFLAG_NO_SCALE = 2
} WINDOW_FLAG;

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
		void	(*Shrink)(struct _Win *win);
		void	(*Expand)(struct _Win *win);
	    } key;

	    struct {
	      ATTRIBUTE select,
			border,
			title;
	    } color[2];

		char	*title;
	} hook;

	Stock		*stock;
	Matrix		matrix;
	TGrid		*grid;
	size_t		dim;

	struct {
		size_t	rowLen,
			titleLen;
	} lazyComp;

	WINDOW_FLAG	flag;
} Window;

typedef struct {
	Window	*head;
} WinList;

typedef void (*TCELLFUNC)(Window*, void*);
typedef int  (*KEYFUNC)(SCANKEY*, Window*);
typedef void (*WINFUNC)(Window*);
typedef char REGSTR[];
typedef char *REGPTR;

extern ATTRIBUTE vColor[];

extern ASCII hSpace[];
extern ASCII hBar[];
extern ASCII hLine[];

extern Layer	*sLayer,
		*dLayer,
		*wLayer;

extern WinList winList;

extern int GetKey(SCANKEY *scan, struct timespec *tsec) ;

extern SCREEN_SIZE GetScreenSize(void) ;

extern void HookCellFunc(TCELLFUNC *with, TCELLFUNC what) ;

extern void HookKeyFunc(KEYFUNC *with, KEYFUNC what) ;

extern void HookWinFunc(WINFUNC *with, WINFUNC what) ;

extern void HookAttrib(ATTRIBUTE *with, ATTRIBUTE what) ;

extern void HookString(REGSTR *with, REGSTR what) ;

extern void HookPointer(REGPTR *with, REGPTR what) ;

#define StoreWindow(win, with, what)					\
(									\
    __builtin_choose_expr(__builtin_types_compatible_p (		\
	__typeof__(win->hook with), __typeof__(TCELLFUNC)),HookCellFunc,\
    __builtin_choose_expr(__builtin_types_compatible_p (		\
	__typeof__(win->hook with), __typeof__(KEYFUNC)), HookKeyFunc,	\
    __builtin_choose_expr(__builtin_types_compatible_p (		\
	__typeof__(win->hook with), __typeof__(WINFUNC)), HookWinFunc,	\
    __builtin_choose_expr(__builtin_types_compatible_p (		\
	__typeof__(win->hook with), __typeof__(ATTRIBUTE)), HookAttrib ,\
    __builtin_choose_expr(__builtin_types_compatible_p (		\
	__typeof__(win->hook with), __typeof__(REGSTR) ), HookString,	\
    __builtin_choose_expr(__builtin_types_compatible_p (		\
	__typeof__(win->hook with), __typeof__(REGPTR) ), HookPointer,	\
    (void)0))))))							\
	(&(win->hook with), what)					\
)

#define GridCall_2xArg(gridCall, updateFunc)				\
({									\
	TGrid *pGrid = gridCall;					\
	if (pGrid != NULL)						\
	{								\
		pGrid->Update = updateFunc;				\
		pGrid->data.pvoid = NULL;				\
	}								\
})

#define GridCall_3xArg(gridCall, updateFunc,	arg0)			\
({									\
	TGrid *pGrid = gridCall;					\
	if (pGrid != NULL)						\
	{								\
		pGrid->Update = updateFunc;				\
		SET_DATA(pGrid, arg0);					\
	}								\
})

#define DISPATCH_GridCall(_1,_2,_3,_CURSOR, ... ) _CURSOR

#define GridCall(...)							\
	DISPATCH_GridCall( __VA_ARGS__ ,GridCall_3xArg ,		\
					GridCall_2xArg ,		\
					NULL)( __VA_ARGS__ )

#define LayerAt( layer, plane, col, row )				\
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

#define TGridAt(win, col, row)						\
	win->grid[col + (row * win->matrix.size.wth)]

#define TCellAt(win, col, row)						\
	TGridAt(win, col, row).cell

#define GetHead(list)		(list)->head
#define SetHead(list, win)	GetHead(list) = win
#define SetDead(list)		SetHead(list, NULL)
#define IsHead(list, win)	(GetHead(list) == win)
#define IsDead(list)		(GetHead(list) == NULL)
#define IsCycling(win)		((win->next == win) && (win->prev == win))
#define GetFocus(list)		GetHead(list)

extern void DestroyLayer(Layer *layer) ;

extern void CreateLayer(Layer *layer, CoordSize size) ;

#define ResetLayer(layer)						\
	memset(layer->attr, 0, layer->size.wth * layer->size.hth);	\
	memset(layer->code, 0, layer->size.wth * layer->size.hth);

#define ClearGarbage(_layer, _plane, _col, _row, _len)			\
	memset(&LayerAt(_layer, _plane, _col, _row), 0, _len)

extern void FillLayerArea(Layer *layer,CUINT col, CUINT row,
				CUINT width, CUINT height,
				ASCII *source, ATTRIBUTE attrib) ;

extern void AllocCopyAttr(TCell *cell, ATTRIBUTE attrib[]) ;

extern void AllocFillAttr(TCell *cell, ATTRIBUTE attrib) ;

extern void AllocCopyItem(TCell *cell, ASCII *item) ;

extern void FreeAllTCells(Window *win) ;

#define StoreTCell(win, shortkey, item, attrib) 			\
({									\
	TGrid *pGrid = NULL;						\
  if (item != NULL)							\
  {									\
	win->dim++;							\
									\
	win->grid = realloc(win->grid, sizeof(TGrid) * win->dim);	\
    if (win->grid != NULL)						\
    {									\
	pGrid = &win->grid[win->dim - 1];				\
	pGrid->cell.quick.key = shortkey;				\
	pGrid->cell.length = strlen((char *) item);			\
	pGrid->data.pvoid = NULL;					\
	pGrid->Update = NULL;						\
									\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		__typeof__(attrib),__typeof__(ATTRIBUTE[])),AllocCopyAttr,\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		__typeof__(attrib),__typeof__(ATTRIBUTE*)),AllocCopyAttr,\
	__builtin_choose_expr(__builtin_types_compatible_p(		\
		__typeof__(attrib),__typeof__(ATTRIBUTE)),AllocFillAttr,\
	(void)0)))							\
		(&pGrid->cell, attrib);					\
									\
	AllocCopyItem(&pGrid->cell, (ASCII *) item);			\
    }									\
  }									\
	pGrid;								\
})

extern void DestroyWindow(Window *win) ;


extern Window *CreateWindow_6xArg(Layer *layer, unsigned long long id,
				CUINT width, CUINT height,
				CUINT oCol, CUINT oRow) ;

extern Window *CreateWindow_7xArg(Layer *layer, unsigned long long id,
				CUINT width, CUINT height,
				CUINT oCol, CUINT oRow, WINDOW_FLAG flag) ;

#define DISPATCH_CreateWindow(_1,_2,_3,_4,_5,_6,_7, _CURSOR, ... ) _CURSOR

#define CreateWindow(...)						\
	DISPATCH_CreateWindow( __VA_ARGS__ ,				\
				CreateWindow_7xArg ,			\
				CreateWindow_6xArg ,			\
				NULL,					\
				NULL,					\
				NULL,					\
				NULL,					\
				NULL)( __VA_ARGS__ )

extern void RemoveWindow(Window *win, WinList *list) ;

extern void AppendWindow(Window *win, WinList *list) ;

extern void DestroyAllWindows(WinList *list) ;

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

extern void AnimateWindow(int rotate, WinList *list) ;

extern Window *SearchWinListById(unsigned long long id, WinList *list) ;

extern void PrintContent(Window *win, WinList *list, CUINT col, CUINT row) ;

extern void ForEachCellPrint(Window *win, WinList *list) ;

extern void EraseWindowWithBorder(Window *win) ;

extern void PrintLCD(	Layer *layer, CUINT col, CUINT row,
			int len, char *pStr, enum PALETTE lcdColor) ;

extern void MotionReset_Win(Window *win) ;

extern void MotionLeft_Win(Window *win) ;

extern void MotionRight_Win(Window *win) ;

extern void MotionUp_Win(Window *win) ;

extern void MotionDown_Win(Window *win) ;

extern void MotionHome_Win(Window *win) ;

extern void MotionEnd_Win(Window *win) ;

extern void MotionTop_Win(Window *win) ;

extern void MotionBottom_Win(Window *win) ;

extern void MotionPgUp_Win(Window *win) ;

extern void MotionPgDw_Win(Window *win) ;

extern void MotionOriginLeft_Win(Window *win) ;

extern void MotionOriginRight_Win(Window *win) ;

extern void MotionOriginUp_Win(Window *win) ;

extern void MotionOriginDown_Win(Window *win) ;

extern void MotionShrink_Win(Window *win) ;

extern void MotionExpand_Win(Window *win) ;

void MotionReScale(Window *win, WinList *list) ;

extern void ReScaleAllWindows(WinList *list) ;

extern int Motion_Trigger(SCANKEY *scan, Window *win, WinList *list) ;

extern void PrintWindowStack(WinList *winList) ;

extern void WindowsUpdate(WinList *winList) ;

#define EraseTCell_Menu(win)						\
({									\
	CoordShift shift = {						\
		.horz = win->matrix.scroll.horz + win->matrix.select.col,\
		.vert = win->matrix.scroll.vert + row			\
	};								\
	Coordinate cell = {						\
		.col =	(win->matrix.origin.col				\
			+ (win->matrix.select.col			\
			* TCellAt(win, shift.horz, shift.vert).length)),\
		.row =	(win->matrix.origin.row + row)			\
	};								\
	memset(&LayerAt(win->layer, attr, cell.col, cell.row), 0,	\
		TCellAt(win, shift.horz, shift.vert).length);		\
	memset(&LayerAt(win->layer, code, cell.col, cell.row), 0,	\
		TCellAt(win, shift.horz, shift.vert).length);		\
})

extern void ForEachCellPrint_Drop(Window *win, void *plist) ;

extern int Enter_StickyCell(SCANKEY *scan, Window *win) ;

extern int MotionEnter_Cell(SCANKEY *scan, Window *win) ;

extern void MotionEnd_Cell(Window *win) ;

extern void MotionLeft_Menu(Window *win) ;

extern void MotionRight_Menu(Window *win) ;

extern void MotionUp_Menu(Window *win) ;

extern void MotionDown_Menu(Window *win) ;

extern void MotionHome_Menu(Window *win) ;

extern void MotionEnd_Menu(Window *win) ;

typedef union {
	unsigned long long	qword;
	struct {
	unsigned int	hi, lo;
	}			dword;
	unsigned short		word[4];
} DATA;

typedef struct _Card {
	struct _Card	*next;

	Coordinate	origin;
	struct {
		void	(*Layout)(Layer *layer, struct _Card *card);
		void	(*Draw)(Layer *layer, struct _Card *card);
	} hook;
	DATA		data;
} Card;

typedef struct {
	Card	*head,
		*tail;
} CardList;

typedef void (*CARDFUNC)(Layer*, Card*);

extern Card *CreateCard(void) ;

extern void AppendCard(Card *card, CardList *list) ;

extern void DestroyAllCards(CardList *list) ;

extern void HookCardFunc(CARDFUNC *with, CARDFUNC what) ;

#define StoreCard(card, with, what)					\
(									\
    __builtin_choose_expr(__builtin_types_compatible_p( 		\
	__typeof__(card->hook with), __typeof__(CARDFUNC)),HookCardFunc,\
    (void)0)								\
	(&(card->hook with), what)					\
)

extern void FreeAll(char *buffer) ;

__typeof__ (errno) AllocAll(char **buffer) ;

extern unsigned int WriteConsole(SCREEN_SIZE drawSize, char *buffer) ;

extern void _TERMINAL_IN(void) ;
extern void _TERMINAL_OUT(void) ;
#define TERMINAL(IO)	_TERMINAL_##IO()

extern void _LOCALE_IN(void) ;
extern void _LOCALE_OUT(void) ;
#define LOCALE(IO)	_LOCALE_##IO()

enum LOCALES {
	LOC_EN,
	LOC_FR,
	LOC_CNT
};

extern enum LOCALES	AppLoc;
extern  locale_t	SysLoc;

#define SYS_LOCALE()		(SysLoc)
#define GET_LOCALE()		(AppLoc)
#define SET_LOCALE(_apploc)						\
({									\
	AppLoc = _apploc;						\
})

extern void StopDump(void) ;
extern __typeof__ (errno) StartDump(char *dumpFormat, int tickReset) ;
extern void AbortDump(void) ;
extern unsigned char DumpStatus(void) ;

extern __typeof__ (errno) SaveGeometries(char*) ;
extern __typeof__ (errno) LoadGeometries(char*) ;

