/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <locale.h>
#include <poll.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq-ui.h"

const char LCD[0x6][0x10][3][3] = {
	{
		{/* 0x20 */
			"   ",
			"   ",
			"   "
		},
		{/* 0x21 */
			" | ",
			" | ",
			" o "
		},
		{/* 0x22 */
			"|| ",
			"   ",
			"   "
		},
		{/* 0x23 */
			" //",
			"=/=",
			"// "
		},
		{/* 0x24 */
			" _ ",
			"(|`",
			"_) "
		},
		{/* 0x25 */
			"   ",
			"O/ ",
			"/O "
		},
		{/* 0x26 */
			"_  ",
			"\\' ",
			"(\\ "
		},
		{/* 0x27 */
			" | ",
			"   ",
			"   "
		},
		{/* 0x28 */
			" / ",
			"|  ",
			" \\ "
		},
		{/* 0x29 */
			" \\ ",
			"  |",
			" / "
		},
		{/* 0x2a */
			"   ",
			"\\|/",
			"/|\\"
		},
		{/* 0x2b */
			"   ",
			"_|_",
			" | "
		},
		{/* 0x2c */
			"   ",
			"   ",
			" / "
		},
		{/* 0x2d */
			"   ",
			"___",
			"   "
		},
		{/* 0x2e */
			"   ",
			"   ",
			" o "
		},
		{/* 0x2f */
			"  /",
			" / ",
			"/  "
		}
	},{
		{/* 0x30 */
			" _ ",
			"| |",
			"|_|"
		},
		{/* 0x31 */
			"   ",
			" | ",
			" | "
		},
		{/* 0x32 */
			" _ ",
			" _|",
			"|_ "
		},
		{/* 0x33 */
			" _ ",
			" _|",
			" _|"
		},
		{/* 0x34 */
			"   ",
			"|_|",
			"  |"
		},
		{/* 0x35 */
			" _ ",
			"|_ ",
			" _|"
		},
		{/* 0x36 */
			" _ ",
			"|_ ",
			"|_|"
		},
		{/* 0x37 */
			" _ ",
			"  |",
			"  |"
		},
		{/* 0x38 */
			" _ ",
			"|_|",
			"|_|"
		},
		{/* 0x39 */
			" _ ",
			"|_|",
			" _|"
		},
		{/* 0x3a */
			"   ",
			" o ",
			" o "
		},
		{/* 0x3b */
			"   ",
			" o ",
			"/  "
		},
		{/* 0x3c */
			" / ",
			"<  ",
			" \\ "
		},
		{/* 0x3d */
			"___",
			"___",
			"   "
		},
		{/* 0x3e */
			" \\ ",
			"  >",
			" / "
		},
		{/* 0x3f */
			" _ ",
			"'_)",
			" ! "
		}
	},{
		{/* 0x40 */
			" _ ",
			"(()",
			" ``"
		},
		{/* 0x41 */
			" _ ",
			"|_|",
			"| |"
		},
		{/* 0x42 */
			"__ ",
			"[_)",
			"[_)"
		},
		{/* 0x43 */
			" _ ",
			"|  ",
			"|_ "
		},
		{/* 0x44 */
			"__ ",
			"| |",
			"|_|"
		},
		{/* 0x45 */
			" _ ",
			"|_ ",
			"|_ "
		},
		{/* 0x46 */
			" __",
			"|- ",
			"|  "
		},
		{/* 0x47 */
			" _ ",
			"|  ",
			"|_]"
		},
		{/* 0x48 */
			"   ",
			"|_|",
			"| |"
		},
		{/* 0x49 */
			" . ",
			" | ",
			" | "
		},
		{/* 0x4a */
			" . ",
			" | ",
			"_| "
		},
		{/* 0x4b */
			"   ",
			"|/ ",
			"|\\ "
		},
		{/* 0x4c */
			"   ",
			"|  ",
			"|_ "
		},
		{/* 0x4d */
			"   ",
			"|||",
			"| |"
		},
		{/* 0x4e */
			"   ",
			"|\\|",
			"| |"
		},
		{/* 0x4f */
			" _.",
			"| |",
			"|_|"
		}
	},{
		{/* 0x50 */
			" _ ",
			"|_|",
			"|  "
		},
		{/* 0x51 */
			" _ ",
			"|\\|",
			"|_!"
		},
		{/* 0x52 */
			" _ ",
			"|_|",
			"| \\"
		},
		{/* 0x53 */
			" _ ",
			"(  ",
			"_) "
		},
		{/* 0x54 */
			"___",
			" | ",
			" | "
		},
		{/* 0x55 */
			"   ",
			"| |",
			"|_|"
		},
		{/* 0x56 */
			"   ",
			"\\ /",
			" v "
		},
		{/* 0x57 */
			"   ",
			"| |",
			"!^!"
		},
		{/* 0x58 */
			"   ",
			"\\/ ",
			"/\\ "
		},
		{/* 0x59 */
			"   ",
			"\\ /",
			" | "
		},
		{/* 0x5a */
			"__ ",
			" / ",
			"/_ "
		},
		{/* 0x5b */
			"  _",
			" | ",
			" |_"
		},
		{/* 0x5c */
			"\\  ",
			" \\ ",
			"  \\"
		},
		{/* 0x5d */
			"_  ",
			" | ",
			"_| "
		},
		{/* 0x5e */
			"/\\ ",
			"   ",
			"   "
		},
		{/* 0x5f */
			"   ",
			"   ",
			"___"
		}
	},{
		{/* 0x60 */
			" \\ ",
			"   ",
			"   "
		},
		{/* 0x61 */
			"   ",
			" _ ",
			"(_("
		},
		{/* 0x62 */
			"   ",
			"|_ ",
			"|_)"
		},
		{/* 0x63 */
			"   ",
			" _ ",
			"(_ "
		},
		{/* 0x64 */
			"   ",
			" _|",
			"(_|"
		},
		{/* 0x65 */
			"   ",
			" _ ",
			"(-'"
		},
		{/* 0x66 */
			"   ",
			",- ",
			"|' "
		},
		{/* 0x67 */
			"   ",
			",- ",
			"|] "
		},
		{/* 0x68 */
			"   ",
			"|_ ",
			"| |"
		},
		{/* 0x69 */
			"   ",
			" . ",
			" | "
		},
		{/* 0x6a */
			"   ",
			" . ",
			" ] "
		},
		{/* 0x6b */
			"   ",
			"., ",
			"|\\ "
		},
		{/* 0x6c */
			"   ",
			"|  ",
			"|_ "
		},
		{/* 0x6d */
			"   ",
			"   ",
			"|'|"
		},
		{/* 0x6e */
			"   ",
			"   ",
			"|\\|"
		},
		{/* 0x6f */
			"   ",
			" _ ",
			"|_|"
		}
	},{
		{/* 0x70 */
			"   ",
			" _ ",
			"|-'"
		},
		{/* 0x71 */
			"   ",
			" _ ",
			"|_!"
		},
		{/* 0x72 */
			"   ",
			" _ ",
			"|\\'"
		},
		{/* 0x73 */
			"   ",
			" _ ",
			"_) "
		},
		{/* 0x74 */
			"   ",
			":_ ",
			"|  "
		},
		{/* 0x75 */
			"   ",
			"   ",
			"|_|"
		},
		{/* 0x76 */
			"   ",
			"   ",
			"\\/ "
		},
		{/* 0x77 */
			"   ",
			"   ",
			"V^V"
		},
		{/* 0x78 */
			"   ",
			"   ",
			">< "
		},
		{/* 0x79 */
			"   ",
			"   ",
			"`/ "
		},
		{/* 0x7a */
			"   ",
			"_  ",
			"/_ "
		},
		{/* 0x7b */
			"  _",
			"_| ",
			" |_"
		},
		{/* 0x7c */
			" | ",
			" | ",
			" | "
		},
		{/* 0x7d */
			"_  ",
			" |_",
			"_| "
		},
		{/* 0x7e */
			"   ",
			"   ",
			" ~ "
		},
		{/* 0x7f */
			"   ",
			".^.",
			"DEL"
		}
	}
};

ATTRIBUTE vColor[] = VOID_COLOR;

ASCII	hSpace[] = HSPACE,
	hBar[]   = HBAR,
	hLine[]  = HLINE;

Layer	*sLayer = NULL,
	*dLayer = NULL,
	*wLayer = NULL,
	*fuse   = NULL;

StockList stockList = {.head = NULL, .tail = NULL};

WinList winList = {.head = NULL};

char *console = NULL;

struct {
    union {
		Bit64	Reset __attribute__ ((aligned (8)));
	struct {
		Bit32	Status;
		int	Tick;
	};
    };
		time_t	StartedAt;
		FILE	*Handle;
		void	(*Header)(void);
		void	(*Write)(char*, int);
		void	(*Break)(void);
} Dump __attribute__ ((aligned (8))) = {
	.StartedAt = 0,
	.Reset	= 0x0,
	.Handle = NULL,
	.Header = JSON_Header,
	.Write	= JSON_Page,
	.Break	= JSON_Break
};

int GetKey(SCANKEY *scan, struct timespec *tsec)
{
	struct pollfd fds = {.fd = STDIN_FILENO, .events = POLLIN};
	int rp = 0, rz = 0;

	if ((rp = ppoll(&fds, 1, tsec, NULL)) > 0) {
		if (fds.revents & POLLIN) {
			size_t lc = read(STDIN_FILENO, &scan->key, 8);
			for (rz = lc; rz < 8; rz++)
				scan->code[rz] = 0;
		}
	}
	return (rp);
}

SCREEN_SIZE GetScreenSize(void)
{
	SCREEN_SIZE _screenSize = {.width = 0, .height = 0};
	struct winsize ts;

	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	_screenSize.width  = (int) ts.ws_col;
	_screenSize.height = (int) ts.ws_row;

	return (_screenSize);
}

TGrid *GridHover(TGrid *pGrid, const char *comment)
{
    if ((pGrid != NULL) && (comment != NULL))
    {
	size_t length = strlen(comment);
	if (length > 0)
	{
		ASCII *comm = malloc(1 + length);
	    if (comm != NULL)
	    {
		StrCopy(comm, comment, (1 + length));
		pGrid->hover.comm = comm;
		pGrid->hover.length = length;
	    }
	}
    }
	return (pGrid);
}

__inline__ void Set_pVOID(TGrid *pGrid, void *pVOID)
{
	pGrid->data.pvoid = pVOID;
}

__inline__ void Set_pULLONG(TGrid *pGrid, unsigned long long *pULLONG)
{
	pGrid->data.pullong = pULLONG;
}

__inline__ void Set_pSLLONG(TGrid *pGrid, signed long long *pSLLONG)
{
	pGrid->data.psllong = pSLLONG;
}

__inline__ void Set_pULONG(TGrid *pGrid, unsigned long *pULONG)
{
	pGrid->data.pulong = pULONG;
}

__inline__ void Set_pSLONG(TGrid *pGrid, signed long *pSLONG)
{
	pGrid->data.pslong = pSLONG;
}

__inline__ void Set_pUINT(TGrid *pGrid, unsigned int *pUINT)
{
	pGrid->data.puint = pUINT;
}

__inline__ void Set_pSINT(TGrid *pGrid, signed int *pSINT)
{
	pGrid->data.psint = pSINT;
}

__inline__ void Set_ULLONG(TGrid *pGrid, unsigned long long _ULLONG)
{
	pGrid->data.ullong = _ULLONG;
}

__inline__ void Set_SLLONG(TGrid *pGrid, signed long long _SLLONG)
{
	pGrid->data.sllong = _SLLONG;
}

__inline__ void Set_ULONG(TGrid *pGrid, unsigned long _ULONG)
{
	pGrid->data.ulong = _ULONG;
}

__inline__ void Set_SLONG(TGrid *pGrid, signed long _SLONG)
{
	pGrid->data.slong = _SLONG;
}

__inline__ void Set_UINT(TGrid *pGrid, unsigned int _UINT)
{
	pGrid->data.uint[0] = _UINT;
	pGrid->data.uint[1] = 0;
}

__inline__ void Set_SINT(TGrid *pGrid, signed int _SINT)
{
	pGrid->data.sint[0] = _SINT;
	pGrid->data.sint[1] = 0;
}

void HookCellFunc(TCELLFUNC *with, TCELLFUNC what) { *with=what; }

void HookKeyFunc(KEYFUNC *with, KEYFUNC what) { *with=what; }

void HookWinFunc(WINFUNC *with, WINFUNC what) { *with=what; }

void HookAttrib(ATTRIBUTE *with, ATTRIBUTE what) { with->value=what.value; }

void HookString(REGSTR *with, REGSTR what) { strcpy(*with, what); }

void HookPointer(REGPTR *with, REGPTR what)
{
	if ((*with = realloc(*with, 1 + strlen(what))) != NULL) {
		strcpy(*with, what);
	}
}

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

void FillLayerArea(Layer *layer,CUINT col, CUINT row,
				CUINT width, CUINT height,
				ASCII *source, ATTRIBUTE attrib)
{
	CUINT _row;
	for (_row = row; _row < row + height; _row++) {
		LayerFillAt(layer, col, _row, width, source, attrib);
	}
}

void AllocCopyAttr(TCell *cell, ATTRIBUTE attrib[])
{
	if ((attrib != NULL) && (cell->attr = malloc(cell->length)) != NULL) {
		memcpy(&cell->attr->value, &attrib->value, cell->length);
	}
}

void AllocFillAttr(TCell *cell, ATTRIBUTE attrib)
{
	if ((cell->attr = malloc(cell->length)) != NULL) {
		memset(&cell->attr->value, attrib.value, cell->length);
	}
}

void AllocCopyItem(TCell *cell, ASCII *item)
{
	if ((cell->item = malloc(cell->length)) != NULL) {
		strncpy((char *)cell->item, (char *)item, cell->length);
	}
}

void FreeAllTCells(Window *win)
{
	if (win->grid != NULL) {
		CUINT i;
		for (i = 0; i < win->dim; i++)
		{
			free(win->grid[i].cell.attr);
			free(win->grid[i].cell.item);

			if (win->grid[i].hover.comm != NULL) {
				free(win->grid[i].hover.comm);
			}
		}
		free(win->grid);
		win->grid = NULL;
	}
}

Stock *CreateStock(unsigned long long id, Coordinate origin)
{
	Stock *stock = malloc(sizeof(Stock));
	if (stock != NULL) {
		GetNext(stock) = NULL;
		stock->id = id;
		stock->geometry.origin = origin;
	}
	return (stock);
}

Stock *AppendStock(Stock *stock)
{
	if (stock != NULL) {
		if (stockList.head == NULL) {
			stockList.head = stockList.tail = stock;
		} else {
			GetNext(stockList.tail) = stock;
			stockList.tail = stock;
		}
	}
	return (stock);
}

void DestroyFullStock(void)
{
	Stock *stock = stockList.head;
	while (stock != NULL) {
		Stock *_next = GetNext(stock);
		free(stock);
		stock = _next;
	}
	stockList.head = stockList.tail = NULL;
}

Stock *SearchStockById(unsigned long long id)
{
	Stock *walker = stockList.head;
	while (walker != NULL) {
		if (walker->id == id) {
			break;
		}
		walker = GetNext(walker);
	}
	return (walker);
}

#define ComputeLazyBottomRow(_win, withBorder)				\
({									\
    if (_win->lazyComp.rowLen == 0)					\
    {									\
	CUINT	_col;							\
	for (	_col = 0, _win->lazyComp.rowLen = 2 * withBorder;	\
		_col < _win->matrix.size.wth;				\
		_col++ )						\
	{								\
		_win->lazyComp.rowLen += TCellAt(_win, _col, 0).length; \
	}								\
    }									\
})

CUINT LazyCompBottomRow(Window *win)
{
    if ((win->dim > 0) && win->matrix.size.wth) {
	return ((win->dim / win->matrix.size.wth) - win->matrix.size.hth);
    } else {
	return (1);
    }
}

void DestroyWindow(Window *win)
{
    if (win != NULL) {
	if (!(win->flag & WINFLAG_NO_STOCK)) {
	    if (win->stock == NULL)
	    {
		win->stock=AppendStock(CreateStock(win->id,win->matrix.origin));
	    } else {
		win->stock->geometry.origin = win->matrix.origin;
	    }
	}
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

Window *_CreateWindow(	Layer *layer, unsigned long long id,
			CUINT width, CUINT height,
			CUINT oCol, CUINT oRow, WINDOW_FLAG flag )
{
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
	unsigned int idx;

	Window *win = calloc(1, sizeof(Window));
	if (win != NULL) {
		win->layer = layer;
		win->id = id;

		win->matrix.size.wth = width;
		win->matrix.size.hth = height;

		win->flag = flag;
	  if (win->flag & WINFLAG_NO_STOCK)
	  {
		win->matrix.origin.col = oCol;
		win->matrix.origin.row = oRow;
	  } else {
	    if ((win->stock = SearchStockById(win->id)) != NULL) {
		win->matrix.origin = win->stock->geometry.origin;
	    } else {
		win->matrix.origin.col = oCol;
		win->matrix.origin.row = oRow;
	    }
	  }
	    for (idx = 0; idx < 2; idx++) {
		win->hook.color[idx].select = select[idx];
		win->hook.color[idx].border = border[idx];
		win->hook.color[idx].title  = title[idx];
	    }
	}
	return (win);
}

Window *CreateWindow_7xArg(	Layer *layer, unsigned long long id,
				CUINT width, CUINT height,
				CUINT oCol, CUINT oRow, WINDOW_FLAG flag )
{
	return ( _CreateWindow(layer, id,
				width, height, oCol, oRow, flag) );
}

Window *CreateWindow_6xArg(	Layer *layer, unsigned long long id,
				CUINT width, CUINT height,
				CUINT oCol, CUINT oRow )
{
	return ( _CreateWindow(layer, id,
				width, height, oCol, oRow,
				WINFLAG_NO_FLAGS) );
}

void RemoveWindow(Window *win, WinList *list)
{
	RemoveWinList(win, list);

	if (IsCycling(GetHead(list))) {
		SetDead(list);
	} else if (IsHead(list, win)) {
	/*Auto shift*/	SetHead(list, GetPrev(win));
	}
	DestroyWindow(win);
}

void AppendWindow(Window *win, WinList *list)
{
	if (win != NULL) {
		if (!IsDead(list))
			AppendWinList(win, list);
		else {
			/* Dead head, now cycling			*/
			GetPrev(win) = win;
			GetNext(win) = win;
		}
		SetHead(list, win);
	}
}

void DestroyAllWindows(WinList *list)
{
	while (!IsDead(list)) {
		RemoveWindow(GetHead(list), list);
	}
}

void AnimateWindow(int rotate, WinList *list)
{
	if (!IsDead(list)) {
		SetHead( list,	rotate == 1	? GetNext(GetHead(list))
						: GetPrev(GetHead(list)) );
	}
}

Window *SearchWinListById(unsigned long long id, WinList *list)
{
	Window *win = NULL;
	if (!IsDead(list)) {
		Window *walker = GetHead(list);
		do {
			if (walker->id == id)
				win = walker;

			walker = GetPrev(walker);
		} while (!IsHead(list, walker) && (win == NULL));
	}
	return (win);
}

void PrintContent(Window *win, WinList *list, CUINT col, CUINT row)
{
	CUINT	horzCol = win->matrix.scroll.horz + col,
		vertRow = win->matrix.scroll.vert + row;

    if ((win->matrix.select.col == col)
     && (win->matrix.select.row == row)) {
	LayerFillAt(win->layer,
		(win->matrix.origin.col
		+ (col * TCellAt(win, horzCol, vertRow).length)),
		(win->matrix.origin.row + row),
		TCellAt(win, horzCol, vertRow).length,
		TCellAt(win, horzCol, vertRow).item,
		win->hook.color[(GetFocus(list) == win)].select);
    } else if (GetFocus(list) == win) {
	LayerCopyAt(win->layer,
		(win->matrix.origin.col
		+ (col * TCellAt(win, horzCol, vertRow).length)),
		(win->matrix.origin.row + row),
		TCellAt(win, horzCol, vertRow).length,
		TCellAt(win, horzCol, vertRow).attr,
		TCellAt(win, horzCol, vertRow).item);
    } else {
	LayerFillAt(win->layer,
		(win->matrix.origin.col
		+ (col * TCellAt(win, horzCol, vertRow).length)),
		(win->matrix.origin.row + row),
		TCellAt(win, horzCol, vertRow).length,
		TCellAt(win, horzCol, vertRow).item,
		win->hook.color[0].select);
    }
}

void PrintComment(Window *win)
{	/* The cell comment is positioned at the bottom border center	*/
	CUINT	horzCol = win->matrix.scroll.horz + win->matrix.select.col,
		vertRow = win->matrix.scroll.vert + win->matrix.select.row;

    if (TGridAt(win, horzCol, vertRow).hover.comm != NULL)
    {
	size_t pos;
	pos = win->lazyComp.rowLen - TGridAt(win,horzCol,vertRow).hover.length;
	pos = pos >> 1;
	pos = pos + (1 & TGridAt(win, horzCol, vertRow).hover.length);

	memcpy(&LayerAt(win->layer, code,
			(win->matrix.origin.col + pos),
			(win->matrix.origin.row + win->matrix.size.hth)),
			TGridAt(win, horzCol, vertRow).hover.comm,
			TGridAt(win, horzCol, vertRow).hover.length);
    }
}

void ForEachCellPrint(Window *win, WinList *list)
{
	CUINT col, row;
	ATTRIBUTE border = win->hook.color[(GetFocus(list) == win)].border;

	/* Top, Left Border Corner					*/
	LayerAt(win->layer, attr,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row - 1)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row - 1)) = 0x20;
	/* Top Border Line						*/
    if (win->hook.title == NULL) {
	LayerFillAt(	win->layer,
			win->matrix.origin.col,
			(win->matrix.origin.row - 1),
			(win->lazyComp.rowLen - 2), hLine, border );
    } else {
	if (win->lazyComp.titleLen == 0) {
		win->lazyComp.titleLen = strlen(win->hook.title);
	}
	size_t halfLeft=(win->lazyComp.rowLen - win->lazyComp.titleLen) / 2;
	size_t halfRight = halfLeft
			+ (win->lazyComp.rowLen - win->lazyComp.titleLen) % 2;
	/* Top, Half-Left Border Line					*/
	LayerFillAt(	win->layer,
			win->matrix.origin.col,
			(win->matrix.origin.row - 1),
			halfLeft, hLine, border );
	/* Top, Centered Border Title					*/
	LayerFillAt(	win->layer,
			(halfLeft + (win->matrix.origin.col - 1)),
			(win->matrix.origin.row - 1),
			win->lazyComp.titleLen, win->hook.title,
			((GetFocus(list) == win) ?
				win->hook.color[1].title
			:	win->hook.color[0].title) );
	/* Top, Half-Right Border Line					*/
	LayerFillAt(	win->layer,
			(halfLeft + win->lazyComp.titleLen
			+ (win->matrix.origin.col - 1)),
			(win->matrix.origin.row - 1),
			(halfRight - 1), hLine, border );
    }
	/* Top, Right Border Corner					*/
	LayerAt(win->layer, attr,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row - 1)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row - 1)) = 0x20;

    for (row = 0; row < win->matrix.size.hth; row++) {
	/* Left Side Border Column					*/
	LayerAt(win->layer, attr,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row + row)) = border;
	LayerAt(win->layer, code,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row + row)) = 0x20;

	for (col = 0; col < win->matrix.size.wth; col++) {
		PrintContent(win, list, col, row);
	}
	/* Right Side Border Column					*/
	LayerAt(win->layer, attr,
		(win->matrix.origin.col
		+ col * TCellAt(win, 0, 0).length),
		(win->matrix.origin.row + row)) = border;
	LayerAt(win->layer, code,
		(win->matrix.origin.col
		+ col * TCellAt(win, 0, 0).length),
		(win->matrix.origin.row + row)) = 0x20;
    }
	/* Bottom, Left Border Corner					*/
	LayerAt(win->layer, attr,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row + win->matrix.size.hth)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col - 1),
		(win->matrix.origin.row + win->matrix.size.hth)) = 0x20;
	/* Bottom Border Line						*/
	LayerFillAt(win->layer,
		win->matrix.origin.col,
		(win->matrix.origin.row + win->matrix.size.hth),
		(win->lazyComp.rowLen - 2), hLine, border);
	/* Bottom, Right Border Corner					*/
	LayerAt(win->layer, attr,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row + win->matrix.size.hth)) = border;

	LayerAt(win->layer, code,
		(win->matrix.origin.col + win->lazyComp.rowLen - 2),
		(win->matrix.origin.row + win->matrix.size.hth)) = 0x20;
	/* Vertical Scrolling Bar					*/
	if (win->dim / win->matrix.size.wth > win->matrix.size.hth)
	{
		CUINT vScrollbar = win->matrix.origin.row
				 + (win->matrix.size.hth - 1)
				 * win->matrix.scroll.vert
				 / LazyCompBottomRow(win);

		ATTRIBUTE attrBar=GetFocus(list) == win ? MAKE_TITLE_FOCUS
							: border;

		LayerAt(win->layer, attr,
			(win->matrix.origin.col + win->lazyComp.rowLen - 2),
			vScrollbar) =  attrBar;

		LayerAt(win->layer, code,
			(win->matrix.origin.col + win->lazyComp.rowLen - 2),
			vScrollbar) =  '=';
	}
}

void EraseWindowWithBorder(Window *win)
{	/* Care about the four window side borders.			*/
	CUINT row;
	for (row = 0; row < win->matrix.size.hth + 2; row++) {
		Coordinate origin = {
			.col = win->matrix.origin.col - 1,
			.row = (win->matrix.origin.row
				- !(win->flag & WINFLAG_NO_BORDER)) + row
		};
		size_t len = win->lazyComp.rowLen + 1;

		memset(&LayerAt(win->layer, attr, origin.col,origin.row),0,len);
		memset(&LayerAt(win->layer, code, origin.col,origin.row),0,len);
	}
}

void PrintLCD(	Layer *layer, CUINT col, CUINT row,
		int len, char *pStr, enum PALETTE lcdColor)
{
	register int j = len;
	do {
		register int offset = col + (len - j) * 3,
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

void LazyCompWindow(Window *win)
{
	if (win->matrix.scroll.vert > LazyCompBottomRow(win)) {
		win->matrix.scroll.vert = LazyCompBottomRow(win);
	}
	if (win->matrix.select.row > win->matrix.size.hth - 1) {
		win->matrix.select.row = win->matrix.size.hth - 1;
	}
}

void MotionReset_Win(Window *win)
{
	win->matrix.scroll.horz = win->matrix.select.col = 0;
	win->matrix.scroll.vert = win->matrix.select.row = 0;
}

void MotionLeft_Win(Window *win)
{
	if (win->matrix.select.col > 0) {
		win->matrix.select.col--;
	} else {
		win->matrix.select.col = win->matrix.size.wth - 1;
	}
}

void MotionRight_Win(Window *win)
{
	if (win->matrix.select.col < win->matrix.size.wth - 1) {
		win->matrix.select.col++;
	} else {
		win->matrix.select.col = 0;
	}
}

void MotionUp_Win(Window *win)
{
	if (win->matrix.select.row > 0) {
		win->matrix.select.row--;
	} else if (win->matrix.scroll.vert > 0) {
		win->matrix.scroll.vert--;
	}
}

void MotionDown_Win(Window *win)
{
	if (win->matrix.select.row < win->matrix.size.hth - 1) {
		win->matrix.select.row++;
	} else if (win->matrix.scroll.vert < LazyCompBottomRow(win)) {
		win->matrix.scroll.vert++;
	}
}

void MotionHome_Win(Window *win)
{
	win->matrix.select.col = 0;
}

void MotionEnd_Win(Window *win)
{
	win->matrix.select.col = win->matrix.size.wth - 1;
}

void MotionTop_Win(Window *win)
{
	win->matrix.scroll.vert = win->matrix.select.row = 0;
}

void MotionBottom_Win(Window *win)
{
	win->matrix.scroll.vert = LazyCompBottomRow(win);
	win->matrix.select.row = win->matrix.size.hth - 1;
}

void MotionPgUp_Win(Window *win)
{
	if (win->matrix.scroll.vert >= win->matrix.size.hth) {
		win->matrix.scroll.vert -= win->matrix.size.hth;
	} else {
		win->matrix.scroll.vert = 0;
	}
}

void MotionPgDw_Win(Window *win)
{
    if(win->matrix.scroll.vert + win->matrix.size.hth < LazyCompBottomRow(win))
    {
	win->matrix.scroll.vert += win->matrix.size.hth;
    } else {
	win->matrix.scroll.vert = LazyCompBottomRow(win);
    }
}

void MotionOriginLeft_Win(Window *win)
{
	if (win->matrix.origin.col > 1) {
		EraseWindowWithBorder(win);
		win->matrix.origin.col--;
	}
}

void MotionOriginRight_Win(Window *win)
{	/* Care about the right-side window border.			*/
	const CUINT maxVisibleCol = CUMIN(MAX_WIDTH - 1, GetScreenSize().width)
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
{	/* Care about the bottom window border				*/
	const CUINT maxVisibleRow = CUMIN(MAX_HEIGHT-1, GetScreenSize().height)
				  - win->matrix.size.hth - 1;

	if (win->matrix.origin.row < maxVisibleRow) {
		EraseWindowWithBorder(win);
		win->matrix.origin.row++;
	}
}

void MotionShrink_Win(Window *win)
{
	if (win->matrix.size.hth > 1) {
		EraseWindowWithBorder(win);
		win->matrix.size.hth--;
		LazyCompWindow(win);
	}
}

void MotionExpand_Win(Window *win)
{
	const CUINT maxVisibleRow = GetScreenSize().height - 1,
		winBottomRow = win->matrix.origin.row + win->matrix.size.hth,
		winMaxHeight = win->dim / win->matrix.size.wth;

    if ((winBottomRow < maxVisibleRow)&&(win->matrix.size.hth < winMaxHeight))
    {
		EraseWindowWithBorder(win);
		win->matrix.size.hth++;
		LazyCompWindow(win);
    }
}

void MotionReScale(Window *win)
{
	CSINT	col = -1, row = -1, height = -1;
	CSINT	rightSide = CUMAX(MIN_WIDTH, GetScreenSize().width)
				- win->lazyComp.rowLen,
		bottomSide = GetScreenSize().height - win->matrix.size.hth;

	if (rightSide > 0 && win->matrix.origin.col > rightSide)
	{
		col = rightSide + !(win->flag & WINFLAG_NO_BORDER);
	}
	if (bottomSide < 0 && !(win->flag & WINFLAG_NO_SCALE))
	{
		height	= win->matrix.size.hth + bottomSide
			- !(win->flag & WINFLAG_NO_BORDER) * 2;

		bottomSide = GetScreenSize().height - height;
	}
	if (bottomSide > 0 && win->matrix.origin.row >= bottomSide)
	{
		row = bottomSide - !(win->flag & WINFLAG_NO_BORDER);
	}
	if (col > 0 && win->matrix.origin.col != col) {
		win->matrix.origin.col = col;
	}
	if (row > 0 && win->matrix.origin.row != row) {
		win->matrix.origin.row = row;
	}
	if (height > 0 && win->matrix.size.hth != height) {
		win->matrix.size.hth = height;
		LazyCompWindow(win);
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

		ResetLayer(thisLayer, MakeAttr(BLACK,0,BLACK,0).value, 0x0);
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
	case SCANKEY_ALT_UP:
	case SCANKEY_ALT_SHIFT_a:
	case SCANKEY_ALT_SHIFT_z:
	case SCANKEY_ALT_a:
	case SCANKEY_ALT_z:
	case SCANCON_ALT_a:
	case SCANCON_ALT_z:
		if (win->hook.key.Shrink !=  NULL)
			win->hook.key.Shrink(win);
		break;
	case SCANKEY_ALT_DOWN:
	case SCANKEY_ALT_SHIFT_s:
	case SCANKEY_ALT_s:
	case SCANCON_ALT_s:
		if (win->hook.key.Expand != NULL)
			win->hook.key.Expand(win);
		break;
	case SCANKEY_ENTER:
		if (win->hook.key.Enter != NULL)
			return (win->hook.key.Enter(scan, win));
		/* fallthrough */
	default:
		return (-1);
	}
	return (0);
}

void ForEachCellPrint_Drop(Window *win, void *plist)
{
	WinList *list = (WinList *) plist;
	CUINT row;
	for (row = 0; row < win->matrix.size.hth; row++) {
	    if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
	    {
		PrintContent(win, list, win->matrix.select.col, row);
	    }
	}
}

int Enter_StickyCell(SCANKEY *scan, Window *win)
{
	if ((scan->key = TCellAt(win,
				( win->matrix.select.col
				+ win->matrix.scroll.horz),
				( win->matrix.select.row
				+ win->matrix.scroll.vert)
				).quick.key) != SCANKEY_NULL)
				{
					return (1);
				} else {
					return (0);
				}
}

int MotionEnter_Cell(SCANKEY *scan, Window *win)
{
	if ((scan->key = TCellAt(win,
				( win->matrix.select.col
				+ win->matrix.scroll.horz),
				( win->matrix.select.row
				+ win->matrix.scroll.vert)
				).quick.key) != SCANKEY_NULL)
				{
					SCANKEY closeKey = {.key = SCANKEY_ESC};
					Motion_Trigger(&closeKey, win,&winList);
					return (1);
				} else {
					return (0);
				}
}

void MotionEnd_Cell(Window *win)
{
	win->matrix.scroll.vert = LazyCompBottomRow(win);
	win->matrix.select.row  = win->matrix.size.hth - 1;
}

void MotionLeft_Menu(Window *win)
{
	CUINT row;
	for (row = 1; row < win->matrix.size.hth; row++) {
		EraseTCell_Menu(win);
	}
	if (win->matrix.select.col > 0) {
		win->matrix.select.col--;
	} else {
		win->matrix.select.col = win->matrix.size.wth - 1;
	}
	win->matrix.select.row = 0;
}

void MotionRight_Menu(Window *win)
{
	CUINT row;
	for (row = 1; row < win->matrix.size.hth; row++) {
		EraseTCell_Menu(win);
	}
	if (win->matrix.select.col < win->matrix.size.wth - 1) {
		win->matrix.select.col++;
	} else {
		win->matrix.select.col = 0;
	}
	win->matrix.select.row = 0;
}

void MotionUp_Menu(Window *win)
{
	CUINT row = win->matrix.select.row;

	if (win->matrix.select.row > 0) {
		row--;
	}
	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
	{
		win->matrix.select.row = row;
	}
}

void MotionDown_Menu(Window *win)
{
	CUINT row = win->matrix.select.row;

	if (row < win->matrix.size.hth - 1) {
		row++;
	}
	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
	{
		win->matrix.select.row = row;
	}
}

void MotionHome_Menu(Window *win)
{
	if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + 1)).quick.key != SCANKEY_VOID)
	{
			win->matrix.select.row = 1;
	} else {
			win->matrix.select.row = 0;
	}
}

void MotionEnd_Menu(Window *win)
{
	CUINT row = 0;
	for (row = win->matrix.size.hth - 1; row > 1; row--) {
	    if (TCellAt(win,
		(win->matrix.scroll.horz + win->matrix.select.col),
		(win->matrix.scroll.vert + row)).quick.key != SCANKEY_VOID)
	    {
			break;
	    }
	}
	win->matrix.select.row = row;
}

#define Call_PrintHook(_win, _list)					\
({									\
	ComputeLazyBottomRow(_win, !(_win->flag & WINFLAG_NO_BORDER));	\
									\
	if (_win->hook.Print != NULL) {					\
		MotionReScale(_win);					\
		_win->hook.Print(_win, _list);				\
	} else {							\
		MotionReScale(_win);					\
		ForEachCellPrint(_win, _list);				\
		PrintComment(_win);					\
	}								\
})

void PrintWindowStack(WinList *winList)
{
	Window *walker;
	if ((walker = GetHead(winList)) != NULL) {
		do {
			walker = GetNext(walker);
			Call_PrintHook(walker, winList);
		}
		while (!IsHead(winList, walker)) ;
	}
}

void WindowsUpdate(WinList *winList)
{
	Window *walker, *marker = NULL;
  if ((walker = GetFocus(winList)) != NULL)
  {
	CUINT col, row;
    do
    {
	walker = GetNext(walker);
	for (row = 0; row < walker->matrix.size.hth; row++) {
	  for (col = 0; col < walker->matrix.size.wth; col++)
	  {
		CUINT	horzCol = walker->matrix.scroll.horz + col,
			vertRow = walker->matrix.scroll.vert + row;

	    if (TGridAt(walker, horzCol, vertRow).Update != NULL)
	    {
		TGridAt(walker, horzCol, vertRow).Update(
					&TGridAt(walker, horzCol, vertRow),
					TGridAt(walker, horzCol, vertRow).data);
		if (marker == NULL) {
			marker = GetPrev(walker);
		}
	    }
	  }
	}
    } while (!IsHead(winList, walker)) ;
  }
  if ((walker = marker) != NULL)
  {
    do
    {
	walker = GetNext(walker);
	Call_PrintHook(walker, winList);
    }
    while (!IsHead(winList, walker)) ;
  }
}

void ReScaleAllWindows(WinList *list)
{
	if (!IsDead(list)) {
		Window *walker = GetHead(list);
		do
		{
			EraseWindowWithBorder(walker);
			MotionReScale(walker);
			Call_PrintHook(walker, list);

			walker = GetNext(walker);
		} while (!IsHead(list, walker)) ;
	}
}

void HookCardFunc(CARDFUNC *with, CARDFUNC what) { *with=what; }

Card *CreateCard(void)
{
	Card *card = calloc(1, sizeof(Card));
	if (card != NULL) {
		GetNext(card) = NULL;
	}
	return (card);
}

void AppendCard(Card *card, CardList *list)
{
	if (card != NULL) {
		if (GetHead(list) == NULL) {
			GetHead(list) = GetTail(list) = card;
		} else {
			GetNext(GetTail(list)) = card;
			GetTail(list) = card;
		}
	}
}

void DestroyAllCards(CardList *list)
{
	Card *card = GetHead(list);
	while (card != NULL) {
		Card *_next = GetNext(card);
		free(card);
		card = _next;
	}
	GetHead(list) = GetTail(list) = NULL;
}

void FreeAll(char *buffer)
{
	DestroyAllWindows(&winList);
	DestroyFullStock();

	if (console != NULL) {
		free(console);
	}
	if (buffer != NULL) {
		free(buffer);
	}
	DestroyLayer(sLayer);
	DestroyLayer(dLayer);
	DestroyLayer(wLayer);
	DestroyLayer(fuse);

	if (sLayer != NULL) {
		free(sLayer);
	}
	if (dLayer != NULL) {
		free(dLayer);
	}
	if (wLayer != NULL) {
		free(wLayer);
	}
	if (fuse != NULL) {
		free(fuse);
	}
	if (Dump.Handle != NULL) {
		fclose(Dump.Handle);
	}
}

UBENCH_DECLARE()

__typeof__ (errno) AllocAll(char **buffer)
{	/* Alloc 10 times to include the ANSI cursor strings.		*/
	if ((*buffer = malloc(10 * MAX_WIDTH)) == NULL) {
		return (ENOMEM);
	}
	if ((console = malloc((10 * MAX_WIDTH) * MAX_HEIGHT)) == NULL) {
		return (ENOMEM);
	}
	const CoordSize layerSize = {
		.wth = MAX_WIDTH,
		.hth = MAX_HEIGHT
	};

	if ((sLayer = calloc(1, sizeof(Layer))) == NULL) {
		return (ENOMEM);
	}
	if ((dLayer = calloc(1, sizeof(Layer))) == NULL) {
		return (ENOMEM);
	}
	if ((wLayer = calloc(1, sizeof(Layer))) == NULL) {
		return (ENOMEM);
	}
	if ((fuse   = calloc(1, sizeof(Layer))) == NULL) {
		return (ENOMEM);
	}
	CreateLayer(sLayer, layerSize);
	CreateLayer(dLayer, layerSize);
	CreateLayer(wLayer, layerSize);
	CreateLayer(fuse, layerSize);

	UBENCH_SETUP(1, 0);
	return (0);
}

unsigned int FuseAll(char stream[], SCREEN_SIZE drawSize)
{
	register ATTRIBUTE	*fa, *sa, *da, *wa;
	register ASCII		*fc, *sc, *dc, *wc;
	register unsigned int	sdx = 0, idx;
	register unsigned int	cursor;
	register signed int	_col, _row;
	register ATTRIBUTE	attr = {.value = 0};

    for (_row = 0; _row < drawSize.height; _row++)
    {
	register const signed int _wth = _row * fuse->size.wth;

	stream[sdx++] = 0x1b;
	stream[sdx++] = '[';

	cursor = _row + 1;
	sdx += cursor >= 100 ? 3 : cursor >= 10 ? 2 : 1;
	for (idx = sdx; cursor > 0; cursor /= 10) {
		stream[--idx] = '0' + (cursor % 10);
	}
	stream[sdx++] = ';';
	stream[sdx++] = '1';
	stream[sdx++] = 'H';

	for (_col = 0; _col < drawSize.width; _col++)
	{
		idx = _col + _wth;
		fa =   &fuse->attr[idx];
		sa = &sLayer->attr[idx];
		da = &dLayer->attr[idx];
		wa = &wLayer->attr[idx];
		fc =   &fuse->code[idx];
		sc = &sLayer->code[idx];
		dc = &dLayer->code[idx];
		wc = &wLayer->code[idx];
	/* STATIC LAYER */
		fa->value = sa->value;
		*fc = *sc;
	/* DYNAMIC LAYER */
		fa->value = da->value ? da->value : fa->value;
		*fc = *dc ? *dc : *fc;
	/* WINDOWS LAYER */
		fa->value = wa->value ? wa->value : fa->value;
		*fc = *wc ? *wc : *fc;
	/* FUSED LAYER */
	    if ((fa->fg ^ attr.fg) || (fa->bg ^ attr.bg) || (fa->bf ^ attr.bf))
	    {
		stream[sdx++] = 0x1b;
		stream[sdx++] = '[';
		stream[sdx++] = '0' + fa->bf;
		stream[sdx++] = ';';
		stream[sdx++] = '3';
		stream[sdx++] = '0' + fa->fg;
		stream[sdx++] = ';';
		stream[sdx++] = '4';
		stream[sdx++] = '0' + fa->bg;
		stream[sdx++] = 'm';
	    }
	    if (fa->un ^ attr.un)
	    {
		stream[sdx++] = 0x1b;
		stream[sdx++] = '[';
		if (fa->un) {
			stream[sdx++] = '4';
			stream[sdx++] = 'm';
		} else {
			stream[sdx++] = '2';
			stream[sdx++] = '4';
			stream[sdx++] = 'm';
		}
	    }
		attr.value = fa->value;

	    switch (*fc) {
	    case 0x0:
		stream[sdx++] = 0x20;
		break;
	    case 0xc0 ... 0xdf:
		stream[sdx++] = 0xc2;	/* Control C2 Unicode UTF-8	*/
		stream[sdx++] = *fc ^ 0x60;
		break;
	    case 0x80 ... 0xbf:
		stream[sdx++] = 0xc3;	/* Control C3 Unicode UTF-8	*/
		/* Fallthrough */
	    default:
		stream[sdx++] = *fc;
		break;
	    }
	}
    }
	return (sdx);
}

__typeof__ (errno) StartDump(	char *dumpFormat, int tickReset,
				enum DUMP_METHOD method )
{
	__typeof__ (errno) rc = EBUSY;

	if (!BITVAL(Dump.Status, 0))
	{
		char *dumpFileName = malloc(64);
		if (dumpFileName != NULL)
		{
			Bit64 tsc __attribute__ ((aligned (8)));
			RDTSC64(tsc);

			snprintf(dumpFileName, 64, dumpFormat, tsc);
			if ((Dump.Handle = fopen(dumpFileName, "w")) != NULL)
			{
				switch (method) {
				case DUMP_TO_JSON:
					Dump.Header= JSON_Header;
					Dump.Write = JSON_Page;
					Dump.Break = JSON_Break;
					break;
				case DUMP_TO_ANSI:
					Dump.Header= ANSI_Header;
					Dump.Write = ANSI_Page;
					Dump.Break = ANSI_Break;
					break;
				};
				Dump.Tick = tickReset;
				Dump.Header();
				BITSET(LOCKLESS, Dump.Status, 0);
				rc = 0;
			} else {
				rc = errno;
			}
			free(dumpFileName);
		} else {
			rc = ENOMEM;
		}
	}
	return (rc);
}

void AbortDump(void)
{
	Dump.Tick = 0;
}

unsigned char DumpStatus(void)
{
	return (BITVAL(Dump.Status, 0));
}

unsigned int WriteConsole(SCREEN_SIZE drawSize)
{
	unsigned int writeSize, layout = 0;

	UI_Draw_uBenchmark(dLayer);

	UBENCH_RDCOUNTER(1);

	writeSize = FuseAll(console, drawSize);

	UBENCH_RDCOUNTER(2);
	UBENCH_COMPUTE();

	if (writeSize > 0)
	{
		fwrite(console, (size_t) writeSize, 1, stdout);
		fflush(stdout);

		if (BITVAL(Dump.Status, 0))
		{
			Dump.Write(console, writeSize);

			if (Dump.Tick > 0) {
				Dump.Tick--;

				Dump.Break();
			} else {
				BITCLR(LOCKLESS, Dump.Status, 0);

				if (fclose(Dump.Handle) == 0) {
					Dump.Handle = NULL;
				}
				layout = 1;
			}
		}
	}
	return (layout);
}

void ANSI_Header(void)
{
}

void ANSI_Page(char *inStr, int outSize)
{
	fwrite(inStr, (size_t) outSize, 1, Dump.Handle);
}

void ANSI_Break(void)
{
	fwrite( _LF _FF, 1, 1, Dump.Handle);
}

void JSON_Header(void)
{
	SCREEN_SIZE terminalSize = GetScreenSize();

	time(&Dump.StartedAt);
	fprintf(Dump.Handle,
		"{\"version\": %d, \"width\": %d, \"height\": %d,"	\
		" \"timestamp\": %ld, \"title\": \"%s\", "		\
		"\"env\": {\"TERM\": \"%s\"}}\n",
		2, terminalSize.width, terminalSize.height,
		Dump.StartedAt, "CoreFreq", "xterm");
}

void JSON_Page(char *inStr, int outSize)
{
	double integral, fractional;
	unsigned char	*pLast = (unsigned char *) &inStr[outSize],
			*pFirst= (unsigned char *) &inStr[0], *pChr;
	time_t now;
	time(&now);

	fractional = modf(difftime(now, Dump.StartedAt), &integral);
	fprintf(Dump.Handle, "[%lld.%lld, \"o\", \"",
		(unsigned long long) integral,
		(unsigned long long) fractional);

	for (pChr = pFirst; pChr < pLast; pChr++) {
		switch (*pChr) {
		case 0x1b:
			fprintf(Dump.Handle, "\\u001b");
			break;
		case 0xc2:
			fprintf(Dump.Handle, "\\u00%x", *(++pChr));
			break;
		case 0xc3:
			fprintf(Dump.Handle, "\\u00%x", *(++pChr) ^ 0x40);
			break;
		case '\\':
			fprintf(Dump.Handle, "\\u005c");
			break;
		default:
			fputc((*pChr), Dump.Handle);
			break;
		}
	}
	fprintf(Dump.Handle, "\"]\n");
}

void JSON_Break(void)
{
}

struct termios oldt, newt;

void _TERMINAL_IN(void)
{
	printf(SCP SCR1 CUH CLS HIDE);

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	newt.c_cc[VTIME] = 0;
	newt.c_cc[VMIN] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void _TERMINAL_OUT(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	printf(SHOW SCR0 RCP COLOR(0,9,9));
}

enum LOCALES	AppLoc = LOC_EN;
locale_t	SysLoc = (locale_t) 0;

typedef char I18N[5];

I18N	i18n_FR[] = {
	"fr_FR",
	"br_FR",
	"fr_BE",
	"fr_CA",
	"fr_CH",
	"fr_LU",
	"ca_FR",
	"ia_FR",
	"oc_FR",
	{0,0,0,0,0}
};

struct LOCALE_LOOKUP {
	enum LOCALES	apploc;
	I18N		*i18n;
} LocaleLookUp[] = {
		{
			.apploc = LOC_FR,
			.i18n	= i18n_FR
		},
		{
			.apploc = LOC_EN,
			.i18n	= NULL
		}
};

void _LOCALE_IN(void)
{
	SysLoc = newlocale(LC_MESSAGES_MASK, "", (locale_t) 0);

	if (SysLoc != NULL) {
		const char *s18n = SysLoc->__names[5];
		struct LOCALE_LOOKUP *lookUp = LocaleLookUp;
		while (lookUp->i18n != NULL) {
			I18N *i18n = lookUp->i18n;
			while (i18n[0][0] != 0) {
				if (!strncmp((char *) i18n, s18n, sizeof(I18N)))
				{
					SET_LOCALE(lookUp->apploc);
					return;
				}
				i18n++;
			}
			lookUp++;
		}
	}
}

void _LOCALE_OUT(void)
{
	if (SysLoc != (locale_t) 0) {
		freelocale(SysLoc);
	}
}

void LocaleTo(int category)
{
	struct LOCALE_LOOKUP *lookUp = LocaleLookUp;
	while (lookUp->i18n != NULL) {
		if (lookUp->apploc == GET_LOCALE()) {
			char localeStr[sizeof(I18N) + 1];
			memcpy(localeStr, lookUp->i18n[0], sizeof(I18N));
			localeStr[sizeof(I18N)] = '\0';
			setlocale(category, localeStr);
			return;
		}
		lookUp++;
	}
	setlocale(category, "C");
}

__typeof__ (errno) SaveGeometries(char *cfgFQN)
{
	__typeof__ (errno) rc = 0;
	FILE *cfgHandle;
    if ((cfgHandle = fopen(cfgFQN, "w")) != NULL)
    {
	Stock *walker = stockList.head;
	while (walker != NULL) {
	    if (fprintf(cfgHandle, "%llx,%hu,%hu\n",
			walker->id,
			walker->geometry.origin.col,
			walker->geometry.origin.row) < 0)
	    {
		rc = errno;
		break;
	    } else {
		walker = GetNext(walker);
	    }
	}
	fclose(cfgHandle);
    } else {
	rc = errno;
    }
	return (rc);
}

__typeof__ (errno) LoadGeometries(char *cfgFQN)
{
	__typeof__ (errno) rc = 0;
	FILE *cfgHandle;
    if ((cfgHandle = fopen(cfgFQN, "r")) != NULL)
    {
      while (!feof(cfgHandle))
      {
	unsigned long long id;
	struct Geometry geometry;
	int match = fscanf(cfgHandle, "%llx,%hu,%hu\n",
				&id,
				&geometry.origin.col,
				&geometry.origin.row);

	if ((match != EOF) && (match == 3))
	{
	    if (AppendStock(CreateStock(id, geometry.origin)) == NULL)
	    {
		rc = ENOMEM;
		break;
	    }
	} else {
		rc = errno;
		break;
	}
      }
	fclose(cfgHandle);
    } else {
	rc = errno;
    }
	return (rc);
}

