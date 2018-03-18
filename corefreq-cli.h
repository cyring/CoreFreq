/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define SORTBY_STATE		0x400000000000000e
#define SORTBY_RTIME		0x400000000000000d
#define SORTBY_UTIME		0x400000000000000c
#define SORTBY_STIME		0x400000000000000b
#define SORTBY_PID		0x400000000000000a
#define SORTBY_COMM		0x4000000000000009

#define BOXKEY_EIST		0x3000000000000004
#define BOXKEY_EIST_OFF		0x3000000000000005
#define BOXKEY_EIST_ON		0x3000000000000006
#define BOXKEY_C1E		0x3000000000000008
#define BOXKEY_C1E_OFF		0x3000000000000009
#define BOXKEY_C1E_ON		0x300000000000000a
#define BOXKEY_TURBO		0x3000000000000010
#define BOXKEY_TURBO_OFF	0x3000000000000011
#define BOXKEY_TURBO_ON		0x3000000000000012
#define BOXKEY_C1A		0x3000000000000020
#define BOXKEY_C1A_OFF		0x3000000000000021
#define BOXKEY_C1A_ON		0x3000000000000022
#define BOXKEY_C3A		0x3000000000000040
#define BOXKEY_C3A_OFF		0x3000000000000041
#define BOXKEY_C3A_ON		0x3000000000000042
#define BOXKEY_C1U		0x3000000000000080
#define BOXKEY_C1U_OFF		0x3000000000000081
#define BOXKEY_C1U_ON		0x3000000000000082
#define BOXKEY_C3U		0x3000000000000100
#define BOXKEY_C3U_OFF		0x3000000000000101
#define BOXKEY_C3U_ON		0x3000000000000102
#define BOXKEY_PKGCST		0x3000000000000200
#define BOXKEY_PKGCST_C0	0x3000000000000201
#define BOXKEY_PKGCST_C1	0x3000000000000211
#define BOXKEY_PKGCST_C2	0x3000000000000221
#define BOXKEY_PKGCST_C3	0x3000000000000231
#define BOXKEY_PKGCST_C6	0x3000000000000261
#define BOXKEY_PKGCST_C7	0x3000000000000271
#define BOXKEY_IOMWAIT		0x3000000000000400
#define BOXKEY_IOMWAIT_OFF	0x3000000000000401
#define BOXKEY_IOMWAIT_ON	0x3000000000000402
#define BOXKEY_IORCST		0x3000000000000800
#define BOXKEY_IORCST_C0	0x3000000000000801
#define BOXKEY_IORCST_C3	0x3000000000000831
#define BOXKEY_IORCST_C4	0x3000000000000841
#define BOXKEY_IORCST_C6	0x3000000000000861
#define BOXKEY_IORCST_C7	0x3000000000000871
#define BOXKEY_ODCM		0x3000000000001000
#define BOXKEY_ODCM_OFF		0x3000000000001001
#define BOXKEY_ODCM_ON		0x3000000000001002
#define BOXKEY_DUTYCYCLE	0x3000000000002000
#define BOXKEY_ODCM_DC00	0x3000000000002001
#define BOXKEY_ODCM_DC01	0x3000000000002011
#define BOXKEY_ODCM_DC02	0x3000000000002021
#define BOXKEY_ODCM_DC03	0x3000000000002031
#define BOXKEY_ODCM_DC04	0x3000000000002041
#define BOXKEY_ODCM_DC05	0x3000000000002051
#define BOXKEY_ODCM_DC06	0x3000000000002061
#define BOXKEY_ODCM_DC07	0x3000000000002071
#define BOXKEY_ODCM_DC08	0x3000000000002081
#define BOXKEY_ODCM_DC09	0x3000000000002091
#define BOXKEY_ODCM_DC10	0x30000000000020a1
#define BOXKEY_ODCM_DC11	0x30000000000020b1
#define BOXKEY_ODCM_DC12	0x30000000000020c1
#define BOXKEY_ODCM_DC13	0x30000000000020d1
#define BOXKEY_ODCM_DC14	0x30000000000020e1

#define BOXKEY_TOOLS_MACHINE	0x3000000000010000
#define BOXKEY_TOOLS_ATOMIC	0x3000000000010010
#define BOXKEY_TOOLS_CRC32	0x3000000000010020
#define BOXKEY_TOOLS_CONIC	0x3000000000011000
#define BOXKEY_TOOLS_CONIC0	0x3000000000011401
#define BOXKEY_TOOLS_CONIC1	0x3000000000011411
#define BOXKEY_TOOLS_CONIC2	0x3000000000011421
#define BOXKEY_TOOLS_CONIC3	0x3000000000011431
#define BOXKEY_TOOLS_CONIC4	0x3000000000011441
#define BOXKEY_TOOLS_CONIC5	0x3000000000011451
#define BOXKEY_TOOLS_TURBO_RND	0x3000000000012001
#define BOXKEY_TOOLS_TURBO_RR	0x3000000000012011

#define TRACK_TASK		0x2000000000000000
#define TRACK_MASK		0x0000000000007fff

#define powered(bit)	((bit) ? "Present" : "Missing")
#define enabled(bit)	((bit) ? "ON" : "OFF")

#define MARGIN_WIDTH	2
#define MARGIN_HEIGHT	1
#define INTER_WIDTH	3
#define INTER_HEIGHT	(3 + 1)
#define LEADING_LEFT	(MIN_WIDTH / (MARGIN_WIDTH + (4 * INTER_WIDTH)))
#define LEADING_TOP	1

#define LOAD_LEAD	4

typedef char HBCLK[11 + 1];

enum DISPOSAL {
	D_MAINVIEW,
	D_DASHBOARD,
	D_ASCIITEST
};
#define DISPOSAL_SIZE	(1 + D_ASCIITEST)

enum VIEW {
	V_FREQ,
	V_INST,
	V_CYCLES,
	V_CSTATES,
	V_PACKAGE,
	V_TASKS,
	V_INTR,
	V_VOLTAGE,
	V_SLICE
};
#define VIEW_SIZE	(1 + V_SLICE)

typedef void (*DISPOSAL_FUNC)(Layer*);

typedef CUINT (*VIEW_FUNC)(Layer*, CUINT);

typedef union {
	unsigned long long	qword;
	struct {
	unsigned int	hi, lo;
	}			dword;
	unsigned short		word[4];
} DATA;

#define RENDER_OK	0x000
#define RENDER_KO	0x010
#define RENDER_OFF	0x100

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

void HookCardFunc(CARDFUNC *with, CARDFUNC what) { *with=what; }

#define StoreCard(card, with, what)					\
(									\
    __builtin_choose_expr(__builtin_types_compatible_p(			\
	typeof(card->hook with), typeof(CARDFUNC)), HookCardFunc,	\
    (void)0)							\
	(&(card->hook with), what)					\
)

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

#define frtostr(r, d, pStr)						\
({									\
	int p = d - ((int) log10(fabs(r))) - 2;				\
	sprintf(pStr, "%*.*f", d, p, r);				\
	pStr;								\
})

#define Clock2LCD(layer, col, row, value1, value2)			\
({									\
	sprintf(buffer, "%04.0f", value1);				\
	PrintLCD(layer, col, row, 4, buffer,				\
	    Threshold(value2,minRatio,medianRatio,_GREEN,_YELLOW,_RED));\
})

#define Counter2LCD(layer, col, row, value)				\
({									\
	sprintf(buffer, "%04.0f", value);				\
	PrintLCD(layer, col, row, 4, buffer,				\
		Threshold(value, 0.f, 1.f, _RED,_YELLOW,_WHITE));	\
})

#define Load2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/3.f, 100.f/1.5f, _WHITE,_YELLOW,_RED))

#define Idle2LCD(layer, col, row, value)				\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/3.f, 100.f/1.5f, _YELLOW,_WHITE,_GREEN))

#define Sys2LCD(layer, col, row, value)					\
	PrintLCD(layer, col, row, 4, frtostr(value, 4, buffer),		\
		Threshold(value, 100.f/6.6f, 50.0, _RED,_YELLOW,_WHITE))
