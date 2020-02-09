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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

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
			"_  ",
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
			" _.",
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
			", ,",
			"|^|"
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
		FILE	*Handle;
} dump __attribute__ ((aligned (8))) = {
	.Reset = 0x0,
	.Handle = NULL
};

int GetKey(SCANKEY *scan, struct timespec *tsec)
{
	struct pollfd fds = {.fd = STDIN_FILENO, .events = POLLIN};
	int rp = 0, rz = 0;

	if ((rp = ppoll(&fds, 1, tsec, NULL)) > 0)
		if (fds.revents & POLLIN) {
			size_t lc = read(STDIN_FILENO, &scan->key, 8);
			for (rz = lc; rz < 8; rz++)
				scan->code[rz] = 0;
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
	if ((*with = realloc(*with, 1 + strlen(what))) != NULL)
		strcpy(*with, what);
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
	for (_row = row; _row < row + height; _row++)
		LayerFillAt(layer, col, _row, width, source, attrib);
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

void FreeAllTCells(Window *win)
{
	if (win->grid != NULL) {
		CUINT i;
		for (i = 0; i < win->dim; i++)
		{
			free(win->grid[i].cell.attr);
			free(win->grid[i].cell.item);
		}
		free(win->grid);
		win->grid = NULL;
	}
}

Stock *CreateStock(unsigned long long id, Coordinate origin)
{
	Stock *stock = malloc(sizeof(Stock));
	if (stock != NULL) {
		stock->next = NULL;
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
			stockList.tail->next = stock;
			stockList.tail = stock;
		}
	}
	return (stock);
}

void DestroyFullStock(void)
{
	Stock *stock = stockList.head;
	while (stock != NULL) {
		Stock *next = stock->next;
		free(stock);
		stock = next;
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
		walker = walker->next;
	}
	return (walker);
}

CUINT LazyCompBottomRow(Window *win)
{
    if ((win->dim > 0) && win->matrix.size.wth)
	return ((win->dim / win->matrix.size.wth) - win->matrix.size.hth);
    else
	return (1);
}

void DestroyWindow(Window *win)
{
    if (win != NULL) {
	if (BITVAL(win->flag, WINMASK_NO_STOCK) == 0) {
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
	    if ((BITVAL(win->flag, WINMASK_NO_STOCK) == 0)
	    && ((win->stock = SearchStockById(win->id)) != NULL))
	    {
		win->matrix.origin = win->stock->geometry.origin;
	    } else {
		win->matrix.origin.col = oCol;
		win->matrix.origin.row = oRow;
	    }
		MotionReScale(win, NULL);

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
	/*Auto shift*/	SetHead(list, win->prev);
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

void ForEachCellPrint(Window *win, WinList *list)
{
	CUINT col, row;
	ATTRIBUTE border = win->hook.color[(GetFocus(list) == win)].border;

    if (win->lazyComp.rowLen == 0) {
	for (col=0, win->lazyComp.rowLen=2; col < win->matrix.size.wth; col++)
		win->lazyComp.rowLen += TCellAt(win, col, 0).length;
    }
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
		win->lazyComp.titleLen=strlen(win->hook.title);
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
			.row = (win->matrix.origin.row - 1) + row
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

void MotionReScale(Window *win, WinList *list)
{
    if (BITVAL(win->flag, WINMASK_NO_SCALE) == 0)
    {
	CSINT col = -1, row = -1, height = -1;
	const CSINT rightSide = CUMAX(MIN_WIDTH, GetScreenSize().width)
				- win->lazyComp.rowLen,
		scaledHeight = GetScreenSize().height - win->matrix.size.hth;

	if ((rightSide > 0) && (win->matrix.origin.col > rightSide))
	{
		col = rightSide + 1;
	}
	if (scaledHeight > 0) {
		if (win->matrix.origin.row >= scaledHeight)
		{
			row = scaledHeight;
		    if (row <= 2) {
			row = 1;
			height = GetScreenSize().height - 2;
		    } else {
			row--;
		    }
		}
	} else {
		row = 1;
		height = GetScreenSize().height - 2;
	}
	if ((col > 0) || (row > 0) || (height > 0))
	{
		if (list != NULL) {
			EraseWindowWithBorder(win);
		}
		if (col > 0) {
			win->matrix.origin.col = col;
		}
		if (row > 0) {
			win->matrix.origin.row = row;
		}
		if (height > 0) {
			win->matrix.size.hth = height;
			LazyCompWindow(win);
		}
		if (list != NULL) {
			if (win->hook.Print != NULL)
				win->hook.Print(win, list);
			else
				ForEachCellPrint(win, list);
		}
	}
    }
}

void ReScaleAllWindows(WinList *list)
{
	if (!IsDead(list)) {
		Window *walker = GetHead(list);
		do
		{
			MotionReScale(walker, list);

			walker = walker->next;
		} while (!IsHead(list, walker)) ;
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
	CUINT col, row;

	if (win->lazyComp.rowLen == 0) {
		for (col = 0; col < win->matrix.size.wth; col++)
			win->lazyComp.rowLen += TCellAt(win, col, 0).length;
	}
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

void PrintWindowStack(WinList *winList)
{
	Window *walker;
	if ((walker = GetHead(winList)) != NULL) {
		do {
			walker = walker->next;

			if (walker->hook.Print != NULL) {
				walker->hook.Print(walker, winList);
			} else {
				ForEachCellPrint(walker, winList);
			}
		} while (!IsHead(winList, walker)) ;
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
	walker = walker->next;
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
			marker = walker->prev;
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
	walker = walker->next;

	if (walker->hook.Print != NULL) {
		walker->hook.Print(walker, winList);
	} else {
		ForEachCellPrint(walker, winList);
	}
    } while (!IsHead(winList, walker)) ;
  }
}

void HookCardFunc(CARDFUNC *with, CARDFUNC what) { *with=what; }

Card *CreateCard(void)
{
	Card *card = calloc(1, sizeof(Card));
	if (card != NULL) {
		card->next = NULL;
	}
	return (card);
}

void AppendCard(Card *card, CardList *list)
{
	if (card != NULL) {
		if (list->head == NULL) {
			list->head = list->tail = card;
		} else {
			list->tail->next = card;
			list->tail = card;
		}
	}
}

void DestroyAllCards(CardList *list)
{
	Card *card = list->head;
	while (card != NULL) {
		Card *next = card->next;
		free(card);
		card = next;
	}
	list->head = list->tail = NULL;
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
	if (dump.Handle != NULL) {
		fclose(dump.Handle);
	}
}

__typeof__ (errno) AllocAll(char **buffer)
{	/* Alloc 10 times to include the ANSI cursor strings.		*/
	if ((*buffer = malloc(10 * MAX_WIDTH)) == NULL)
		return (ENOMEM);
	if ((console = malloc((10 * MAX_WIDTH) * MAX_HEIGHT)) == NULL)
		return (ENOMEM);

	const CoordSize layerSize = {
		.wth = MAX_WIDTH,
		.hth = MAX_HEIGHT
	};

	if ((sLayer = calloc(1, sizeof(Layer))) == NULL)
		return (ENOMEM);
	if ((dLayer = calloc(1, sizeof(Layer))) == NULL)
		return (ENOMEM);
	if ((wLayer = calloc(1, sizeof(Layer))) == NULL)
		return (ENOMEM);
	if ((fuse   = calloc(1, sizeof(Layer))) == NULL)
		return (ENOMEM);

	CreateLayer(sLayer, layerSize);
	CreateLayer(dLayer, layerSize);
	CreateLayer(wLayer, layerSize);
	CreateLayer(fuse, layerSize);

	return (0);
}

unsigned int FuseAll(char stream[], SCREEN_SIZE drawSize, char *buffer)
{
	register ATTRIBUTE	*fa, *sa, *da, *wa;
	register ASCII		*fc, *sc, *dc, *wc;
	register unsigned int	sdx = 0, _bix, _bdx, _idx;
	register unsigned int	cursor_flag;
	register unsigned int	cursor_col, cursor_row;
	register signed int	_col, _row, _wth;
	register ATTRIBUTE	attr = {.value = 0};

    for (_row = 0; _row < drawSize.height; _row++)
    {
	cursor_flag = 0;
	_wth = _row * fuse->size.wth;

	for (_col = 0, _bix = 0; _col < drawSize.width; _col++)
	{
		_idx = _col + _wth;
		fa =   &fuse->attr[_idx];
		sa = &sLayer->attr[_idx];
		da = &dLayer->attr[_idx];
		wa = &wLayer->attr[_idx];
		fc =   &fuse->code[_idx];
		sc = &sLayer->code[_idx];
		dc = &dLayer->code[_idx];
		wc = &wLayer->code[_idx];
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
		buffer[_bix++] = 0x1b;
		buffer[_bix++] = '[';
		buffer[_bix++] = '0' + fa->bf;
		buffer[_bix++] = ';';
		buffer[_bix++] = '3';
		buffer[_bix++] = '0' + fa->fg;
		buffer[_bix++] = ';';
		buffer[_bix++] = '4';
		buffer[_bix++] = '0' + fa->bg;
		buffer[_bix++] = 'm';
	    }
	    if (fa->un ^ attr.un)
	    {
		buffer[_bix++] = 0x1b;
		buffer[_bix++] = '[';
		if (fa->un) {
			buffer[_bix++] = '4';
			buffer[_bix++] = 'm';
		} else {
			buffer[_bix++] = '2';
			buffer[_bix++] = '4';
			buffer[_bix++] = 'm';
		}
	    }
	    attr.value = fa->value;

	    if (*fc != 0) {
		if (cursor_flag == 0)
		{
			cursor_flag = 1;
			cursor_row = _row + 1;
			cursor_col = _col + 1;

			buffer[_bix++] = 0x1b;
			buffer[_bix++] = '[';

			_bix += cursor_row >= 100 ? 3 : cursor_row >= 10 ? 2:1;
			for (_bdx = _bix; cursor_row > 0; cursor_row /= 10)
				buffer[--_bdx] = '0' + (cursor_row % 10);

			buffer[_bix++] = ';';

			_bix += cursor_col >= 100 ? 3 : cursor_col >= 10 ? 2:1;
			for (_bdx = _bix; cursor_col > 0; cursor_col /= 10)
				buffer[--_bdx] = '0' + (cursor_col % 10);

			buffer[_bix++] = 'H';
		}
		buffer[_bix++] = *fc;
	    } else {
		if (cursor_flag != 0)
			cursor_flag = 0;
	    }
	}
	memcpy(&stream[sdx], buffer, _bix);
	sdx += _bix;
    }
	return (sdx);
}

__typeof__ (errno) StartDump(char *dumpFormat, int tickReset)
{
	__typeof__ (errno) rc = EBUSY;

	if (!BITVAL(dump.Status, 0))
	{
		char *dumpFileName = malloc(64);
		if (dumpFileName != NULL)
		{
			Bit64 tsc __attribute__ ((aligned (8)));
			RDTSC64(tsc);

			snprintf(dumpFileName, 64, dumpFormat, tsc);
			if ((dump.Handle = fopen(dumpFileName, "w")) != NULL)
			{
				dump.Tick = tickReset;
				BITSET(LOCKLESS, dump.Status, 0);
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
	dump.Tick = 0;
}

unsigned char DumpStatus(void)
{
	return (BITVAL(dump.Status, 0));
}

unsigned int WriteConsole(SCREEN_SIZE drawSize, char *buffer)
{
	unsigned int writeSize = FuseAll(console, drawSize, buffer), layout = 0;

	if (writeSize > 0) {
		fwrite(console, (size_t) writeSize, 1, stdout);
		fflush(stdout);

		if (BITVAL(dump.Status, 0))
		{
			fwrite(console, (size_t) writeSize, 1, dump.Handle);

			if (dump.Tick > 0) {
				dump.Tick--;

				fwrite( _LF _FF, 1, 1, dump.Handle);
			} else {
				BITCLR(LOCKLESS, dump.Status, 0);

				if (fclose(dump.Handle) == 0) {
					dump.Handle = NULL;
				}
				layout = 1;
			}
		}
	}
	return (layout);
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
	"br_FR",
	"ca_FR",
	"fr_BE",
	"fr_CA",
	"fr_CH",
	"fr_FR",
	"fr_LU",
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
	return;
}

void _LOCALE_OUT(void)
{
	if (SysLoc != (locale_t) 0) {
		freelocale(SysLoc);
	}
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
		walker = walker->next;
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

