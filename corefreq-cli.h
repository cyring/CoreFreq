/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define SORTBY_STATE		0x010000000000000e
#define SORTBY_RTIME		0x010000000000000d
#define SORTBY_UTIME		0x010000000000000c
#define SORTBY_STIME		0x010000000000000b
#define SORTBY_PID		0x010000000000000a
#define SORTBY_COMM		0x0100000000000009

#define OPS_INTERVAL		0x1000000000001000
#define OPS_INTERVAL_100	0x1000000000001001
#define OPS_INTERVAL_150	0x1000000000001011
#define OPS_INTERVAL_250	0x1000000000001021
#define OPS_INTERVAL_500	0x1000000000001031
#define OPS_INTERVAL_750	0x1000000000001041
#define OPS_INTERVAL_1000	0x1000000000001051
#define OPS_INTERVAL_1500	0x1000000000001061
#define OPS_INTERVAL_2000	0x1000000000001071
#define OPS_INTERVAL_2500	0x1000000000001081
#define OPS_INTERVAL_3000	0x1000000000001091
#define OPS_AUTOCLOCK		0x1000000000002000
#define OPS_AUTOCLOCK_OFF	0x1000000000002001
#define OPS_AUTOCLOCK_ON	0x1000000000002002
#define OPS_EXPERIMENTAL	0x1000000000004000
#define OPS_EXPERIMENTAL_OFF	0x1000000000004001
#define OPS_EXPERIMENTAL_ON	0x1000000000004002
#define OPS_INTERRUPTS		0x1000000000008000
#define OPS_INTERRUPTS_OFF	0x1000000000008001
#define OPS_INTERRUPTS_ON	0x1000000000008002

#define BOXKEY_EIST		0x3000000000000004
#define BOXKEY_EIST_OFF 	0x3000000000000005
#define BOXKEY_EIST_ON		0x3000000000000006
#define BOXKEY_C1E		0x3000000000000008
#define BOXKEY_C1E_OFF		0x3000000000000009
#define BOXKEY_C1E_ON		0x300000000000000a
#define BOXKEY_TURBO		0x3000000000000010
#define BOXKEY_TURBO_OFF	0x3000000000000011
#define BOXKEY_TURBO_ON 	0x3000000000000012
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
#define BOXKEY_ODCM_OFF 	0x3000000000001001
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
#define BOXKEY_CC6		0x3000000000004000
#define BOXKEY_CC6_OFF		0x3000000000004001
#define BOXKEY_CC6_ON		0x3000000000004002
#define BOXKEY_PC6		0x3000000000008000
#define BOXKEY_PC6_OFF		0x3000000000008001
#define BOXKEY_PC6_ON		0x3000000000008002

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

#define BOXKEY_CLR_THM_SENSOR	0x3000000000020011
#define BOXKEY_CLR_THM_PROCHOT	0x3000000000020021
#define BOXKEY_CLR_THM_CRIT	0x3000000000020041
#define BOXKEY_CLR_THM_THOLD	0x3000000000020081
#define BOXKEY_CLR_PWR_LIMIT	0x3000000000020101
#define BOXKEY_CLR_CUR_LIMIT	0x3000000000020201
#define BOXKEY_CLR_X_DOMAIN	0x3000000000020401

#define CLEAR_EVENT_MASK	0x0000000000000ff0

#define TRACK_TASK		0x0200000000000000
#define TRACK_MASK		0x0000000000007fff

#define CPU_ONLINE		0x0400000000000000
#define CPU_OFFLINE		0x0800000000000000
#define CPUID_MASK		0x000000000000003f

#define BOXKEY_TURBO_CLOCK	0x00010000
#define BOXKEY_TURBO_CLOCK_NC	0x00020000
#define BOXKEY_TURBO_CLOCK_1C	0x0002000100000000
#define BOXKEY_TURBO_CLOCK_2C	0x0002000200000000
#define BOXKEY_TURBO_CLOCK_3C	0x0002000300000000
#define BOXKEY_TURBO_CLOCK_4C	0x0002000400000000
#define BOXKEY_TURBO_CLOCK_5C	0x0002000500000000
#define BOXKEY_TURBO_CLOCK_6C	0x0002000600000000
#define BOXKEY_TURBO_CLOCK_7C	0x0002000700000000
#define BOXKEY_TURBO_CLOCK_8C	0x0002000800000000

#define BOXKEY_RATIO_CLOCK	0x00040000
#define BOXKEY_RATIO_CLOCK_OR	0x00080000
#define BOXKEY_RATIO_CLOCK_MAX	0x0008000100000000
#define BOXKEY_RATIO_CLOCK_MIN	0x0008000200000000

#define BOXKEY_UNCORE_CLOCK	0x00400000
#define BOXKEY_UNCORE_CLOCK_OR	0x00800000
#define BOXKEY_UNCORE_CLOCK_MAX 0x0080000100000000
#define BOXKEY_UNCORE_CLOCK_MIN 0x0080000200000000

#define CLOCKMOD_RATIO_MASK	0x0000ffff

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

typedef CUINT (*VIEW_FUNC)(Layer*, const unsigned int, CUINT);

#define CELL_ARGS	Window *win,					\
			unsigned long long key, 			\
			ATTRIBUTE *attrib,				\
			ASCII *item

typedef void (*CELL_FUNC)(CELL_ARGS);

#define RENDER_OK	0x000
#define RENDER_KO	0x010
#define RENDER_OFF	0x100

#define RUN_STATE_COLOR 			\
{						\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK 	\
}

#define UNINT_STATE_COLOR			\
{						\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK 	\
}

#define ZOMBIE_STATE_COLOR			\
{						\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW 	\
}

#define SLEEP_STATE_COLOR			\
{						\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}

#define WAIT_STATE_COLOR			\
{						\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 	\
}

#define OTHER_STATE_COLOR			\
{						\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK 	\
}

#define TRACKER_STATE_COLOR			\
{						\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC 	\
}

#define LAYOUT_HEADER_PROC_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HRK,HDK 		\
}

#define LAYOUT_HEADER_PROC_CODE 					\
{									\
	' ','P','r','o','c','e','s','s','o','r',' ','[' 		\
}

#define LAYOUT_HEADER_CPU_ATTR						\
{									\
	HDK,HWK,HWK,HWK,HDK,HWK,HWK,HWK,LWK,LWK,LWK			\
}

#define LAYOUT_HEADER_CPU_CODE						\
{									\
	']',' ',' ',' ','/',' ',' ',' ','C','P','U'			\
}

#define LAYOUT_HEADER_ARCH_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK	\
}

#define LAYOUT_HEADER_ARCH_CODE 					\
{									\
	' ','A','r','c','h','i','t','e','c','t','u','r','e',' ','['	\
}

#define LAYOUT_HEADER_CACHE_L1_ATTR					\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,				\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,			\
	LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,LWK,LWK 			\
}

#define LAYOUT_HEADER_CACHE_L1_CODE					\
{									\
	']',' ','C','a','c','h','e','s',' ',				\
	'L','1',' ','I','n','s','t','=',' ',' ',' ',			\
	'D','a','t','a','=',' ',' ',' ','K','B' 			\
}

#define LAYOUT_HEADER_BCLK_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
	HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,LWK,LWK,LWK \
}

#define LAYOUT_HEADER_BCLK_CODE 					\
{									\
	' ','B','a','s','e',' ','C','l','o','c','k',' ',		\
	'~',' ','0','0','0',' ','0','0','0',' ','0','0','0',' ','H','z' \
}

#define LAYOUT_HEADER_CACHES_ATTR					\
{									\
	LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,			\
	LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK			\
}

#define LAYOUT_HEADER_CACHES_CODE					\
{									\
	'L','2','=',' ',' ',' ',' ',' ',' ',' ',			\
	'L','3','=',' ',' ',' ',' ',' ',' ','K','B'			\
}

#define LAYOUT_RULLER_LOAD_ATTR 					\
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
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define LAYOUT_RULLER_LOAD_CODE 					\
{									\
	'-','-','-',' ','R','a','t','i','o',' ','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define LAYOUT_MONITOR_FREQUENCY_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HWK,HWK,LWK,HWK,HWK,HDK,\
	LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,\
	LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,\
	LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,\
	LWK,LWK,HBK,HBK,HBK,HDK,LWK,LWK,LWK,HDK,LYK,LYK,LYK		\
}

#define LAYOUT_MONITOR_FREQUENCY_CODE					\
{									\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',0x0,' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',0x0,' ',' ',' '		\
}

#define LAYOUT_MONITOR_INST_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HDK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HDK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK			\
}

#define LAYOUT_MONITOR_INST_CODE					\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,' ',' ',' ',' ',\
	' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,' ',\
	' ',' ',' ',' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',0x0,' ',' ',' ',' ',' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '			\
}

#define LAYOUT_MONITOR_COMMON_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK		\
}

#define LAYOUT_MONITOR_COMMON_CODE					\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
}

#define LAYOUT_MONITOR_TASKS_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_MONITOR_TASKS_CODE					\
{									\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define LAYOUT_RULLER_FREQUENCY_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_FREQUENCY_CODE					\
	"--- Freq(MHz) Ratio - Turbo --- C0 ---- C1 ---- C3 ---- C6 -"	\
	"--- C7 --Min TMP Max ---------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_RULLER_FREQUENCY_AVG_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_FREQUENCY_AVG_CODE				\
{									\
	'-','-','-','-','-','-',' ','%',' ','A','v','e','r','a','g','e',\
	's',' ','[',' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',']','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define LAYOUT_RULLER_FREQUENCY_PKG_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_FREQUENCY_PKG_CODE				\
{									\
	'-','-','-','-','-',' ','%',' ','P','k','g',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define LAYOUT_RULLER_INST_CODE 					\
	"------------ IPS -------------- IPC -------------- CPI -----"	\
	"------------- INST -----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_RULLER_CYCLES_CODE					\
	"-------------- C0:UCC ---------- C0:URC ------------ C1 ----"	\
	"--------- TSC ----------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_RULLER_CSTATES_CODE					\
	"---------------- C1 -------------- C3 -------------- C6 ----"	\
	"---------- C7 ----------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_RULLER_INTERRUPTS_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_INTERRUPTS_CODE					\
{									\
	"---------- SMI ------------ NMI[ LOCAL   UNKNOWN  PCI_SERR# "	\
	" IO_CHECK] -------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
}

#define LAYOUT_RULLER_PACKAGE_CODE					\
	"------------ Cycles ---- State -------------------- TSC Rati"	\
	"o ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_PACKAGE_PC_ATTR						\
{									\
	LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK \
}

#define LAYOUT_PACKAGE_PC_CODE						\
{									\
	'P','C','0','0',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define LAYOUT_PACKAGE_UNCORE_ATTR					\
{									\
	LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_PACKAGE_UNCORE_CODE					\
{									\
	' ','T','S','C',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ','U','N','C','O','R','E',':',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define LAYOUT_RULLER_TASKS_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,\
	LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LWK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_TASKS_CODE					\
	"--- Freq(MHz) --- Tasks                    -----------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_TASKS_STATE_SORTED_ATTR					\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,HDK,LWK, LWK,LWK,LWK			\
}

#define LAYOUT_TASKS_STATE_SORTED_CODE					\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','S','t','a','t','e',')',' ', '-','-','-'			\
}

#define LAYOUT_TASKS_RUNTIME_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK			\
}

#define LAYOUT_TASKS_RUNTIME_SORTED_CODE				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','R','u','n','T','i','m','e', ')',' ','-'			\
}

#define LAYOUT_TASKS_USRTIME_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, LCK,HDK,LWK			\
}

#define LAYOUT_TASKS_USRTIME_SORTED_CODE				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','U','s','e','r','T','i','m', 'e',')',' '			\
}

#define LAYOUT_TASKS_SYSTIME_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK			\
}

#define LAYOUT_TASKS_SYSTIME_SORTED_CODE				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','S','y','s','T','i','m','e', ')',' ','-'			\
}

#define LAYOUT_TASKS_PROCESS_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,HDK,LWK,LWK,LWK, LWK,LWK,LWK			\
}

#define LAYOUT_TASKS_PROCESS_SORTED_CODE				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','P','I','D',')',' ','-','-', '-','-','-'			\
}

#define LAYOUT_TASKS_COMMAND_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK			\
}

#define LAYOUT_TASKS_COMMAND_SORTED_CODE				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','C','o','m','m','a','n','d', ')',' ','-'			\
}

#define LAYOUT_TASKS_REVERSE_SORT_OFF_ATTR				\
{									\
	LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK	\
}

#define LAYOUT_TASKS_REVERSE_SORT_OFF_CODE				\
{									\
	' ', 'R','e','v','e','r','s','e',' ','[','O','F','F',']',' '	\
}

#define LAYOUT_TASKS_REVERSE_SORT_ON_ATTR				\
{									\
	LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LCK,LCK,LCK,HDK,LWK	\
}

#define LAYOUT_TASKS_REVERSE_SORT_ON_CODE				\
{									\
	' ', 'R','e','v','e','r','s','e',' ','[',' ','O','N',']',' '	\
}

#define LAYOUT_TASKS_VALUE_SWITCH_ATTR					\
{									\
	LWK,_HCK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK		\
}

#define LAYOUT_TASKS_VALUE_SWITCH_CODE					\
{									\
	' ', 'V','a','l','u','e',' ','[',' ',' ',' ',']',' '		\
}

#define LAYOUT_TASKS_VALUE_OFF_ATTR					\
{									\
	LWK,LWK,LWK							\
}

#define LAYOUT_TASKS_VALUE_OFF_CODE					\
{									\
	'O','F','F'							\
}

#define LAYOUT_TASKS_VALUE_ON_ATTR					\
{									\
	LCK,LCK,LCK							\
}

#define LAYOUT_TASKS_VALUE_ON_CODE					\
{									\
	' ','O','N'							\
}

#define LAYOUT_TASKS_TRACKING_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,\
	LWK,LWK,LWK,LWK,HDK,LWK						\
}

#define LAYOUT_TASKS_TRACKING_CODE					\
{									\
	' ','T','r','a','c','k','i', 'n','g',' ','P','I','D',' ','[',' ',\
	'O','F','F',' ',']',' '						\
}

#define LAYOUT_RULLER_VOLTAGE_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_VOLTAGE_CODE					\
	"--- Freq(MHz) - VID - Vcore ------------------ Energy(J) ---"	\
	"-- Power(W) ------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_POWER_MONITOR_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK \
}

#define LAYOUT_POWER_MONITOR_CODE					\
	"                                       "

#define LAYOUT_RULLER_SLICE_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
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

#define LAYOUT_RULLER_SLICE_CODE					\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 -----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"

#define LAYOUT_FOOTER_TECH_X86_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK 	\
}

#define LAYOUT_FOOTER_TECH_X86_CODE					\
{									\
	'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',',' 	\
}

#define LAYOUT_FOOTER_TECH_INTEL_ATTR					\
{									\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,			\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,LWK,HDK,HDK,HDK,HDK,LWK,				\
	HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,				\
	LWK,HDK,HWK,LWK,HWK,HWK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK 	\
}

#define LAYOUT_FOOTER_TECH_INTEL_CODE					\
{									\
	'H','T','T',',','E','I','S','T',',','I','D','A',',',		\
	'T','U','R','B','O',',','C','1','E',',',			\
	' ','P','M',',','C','3','A',',','C','1','A',',',		\
	'C','3','U',',','C','1','U',',',				\
	'T','M',',','H','O','T',']',' ',' ',				\
	'V','[',' ','.',' ',' ',']',' ','T','[',' ',' ',' ',']' 	\
}

#define LAYOUT_FOOTER_TECH_AMD_ATTR					\
{									\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,		\
	LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
	LWK,HDK,HWK,LWK,HWK,HWK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK 	\
}

#define LAYOUT_FOOTER_TECH_AMD_CODE					\
{									\
	'S','M','T',',','P','o','w','e','r','N','o','w',',',		\
	'B','O','O','S','T',',','C','1','E',',','C','C','6',		\
	',','P','C','6',',',' ','P','M',',','D','T','S',',',		\
	'T','T','P',',','H','O','T',']',' ',' ',' ',' ',' ',		\
	'V','[',' ','.',' ',' ',']',' ','T','[',' ',' ',' ',']' 	\
}

#define LAYOUT_FOOTER_SYSTEM_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,	\
	LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
	HWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,HDK 	\
}

#define LAYOUT_FOOTER_SYSTEM_CODE					\
{									\
	'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ','K','B',']' 	\
}

#define LAYOUT_CARD_CORE_ONLINE_ATTR					\
{									\
	HDK,HDK,HDK,LCK,LCK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 		\
}

#define LAYOUT_CARD_CORE_ONLINE_CODE					\
{									\
	'[',' ','#',' ',' ',' ',' ',' ',' ','C',' ',']' 		\
}

#define LAYOUT_CARD_CORE_OFFLINE_ATTR					\
{									\
	HDK,HDK,HDK,LBK,LBK,HDK,HDK,LWK,LWK,LWK,HDK,HDK 		\
}

#define LAYOUT_CARD_CORE_OFFLINE_CODE					\
{									\
	'[',' ','#',' ',' ',' ',' ','O','F','F',' ',']' 		\
}

#define LAYOUT_CARD_CLK_ATTR						\
{									\
	HDK,HDK,HWK,HWK,HWK,LWK,HWK,HDK,HDK,HDK,HDK,HDK 		\
}

#define LAYOUT_CARD_CLK_CODE						\
{									\
	'[',' ','0','0','0','.','0',' ','M','H','z',']' 		\
}

#define LAYOUT_CARD_UNCORE_ATTR 					\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,LCK,LCK,HDK 		\
}

#define LAYOUT_CARD_UNCORE_CODE 					\
{									\
	'[','U','N','C','O','R','E',' ',' ',' ',' ',']' 		\
}

#define LAYOUT_CARD_BUS_ATTR						\
{									\
	HDK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,HDK 		\
}

#define LAYOUT_CARD_BUS_CODE						\
{									\
	'[','B','u','s',' ',' ',' ',' ',' ',' ',' ',']' 		\
}

#define LAYOUT_CARD_MC_ATTR						\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK 		\
}

#define LAYOUT_CARD_MC_CODE						\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',']' 		\
}

#define LAYOUT_CARD_LOAD_ATTR						\
{									\
	HDK,HDK,HDK,HDK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK 		\
}

#define LAYOUT_CARD_LOAD_CODE						\
{									\
	'[',' ',' ','%','L','O','A','D',' ',' ',' ',']' 		\
}

#define LAYOUT_CARD_IDLE_ATTR						\
{									\
	HDK,HDK,HDK,HDK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK 		\
}

#define LAYOUT_CARD_IDLE_CODE						\
{									\
	'[',' ',' ','%','I','D','L','E',' ',' ',' ',']' 		\
}

#define LAYOUT_CARD_RAM_ATTR						\
{									\
	HDK,HWK,HWK,HWK,HWK,HWK,LWK,HDK,HWK,HWK,LWK,HDK 		\
}

#define LAYOUT_CARD_RAM_CODE						\
{									\
	'[',' ',' ',' ',' ',' ',' ','/',' ',' ',' ',']' 		\
}

#define LAYOUT_CARD_TASK_ATTR						\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HDK 		\
}

#define LAYOUT_CARD_TASK_CODE						\
{									\
	'[','T','a','s','k','s',' ',' ',' ',' ',' ',']' 		\
}

#define WIN_COND0_SYSINFO_CPUID_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LCK,LCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define WIN_COND1_SYSINFO_CPUID_ATTR					\
{									\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define WIN_COND2_SYSINFO_CPUID_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define WIN_COND3_SYSINFO_CPUID_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,\
	LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,\
	LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK 			\
}

#define WIN_COND0_SYSTEM_REGISTERS_ATTR 				\
{									\
	LWK,LWK,LWK,LWK 						\
}

#define WIN_COND1_SYSTEM_REGISTERS_ATTR 				\
{									\
	HBK,HBK,HBK,HBK 						\
}

#define WIN_COND2_SYSTEM_REGISTERS_ATTR 				\
{									\
	HWK,HWK,HWK,HWK 						\
}

#define WIN_COND0_SYSINFO_PROC_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 		\
}

#define WIN_COND1_SYSINFO_PROC_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,HGK,LWK,LWK 		\
}

#define WIN_COND2_SYSINFO_PROC_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK 		\
}

#define WIN_COND3_SYSINFO_PROC_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK 		\
}

#define WIN_COND0_SYSINFO_ISA_ATTR					\
{									\
    {	/*	[N] & [N/N]	2*(0|0)+(0<<0)			*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ /*	[Y]						*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,LWK					\
    },{ /*	[N/Y]	2*(0|1)+(0<<1)				*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,LWK					\
    },{ /*	[Y/N]	2*(1|0)+(1<<0)				*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ /*	[Y/Y]	2*(1|1)+(1<<1)				*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,HGK,LWK					\
    }									\
}

#define WIN_COND1_SYSINFO_ISA_ATTR					\
{									\
    {									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,HGK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,HGK,LWK,LWK					\
    }									\
}

#define WIN_COND2_SYSINFO_ISA_ATTR					\
{									\
    {									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,LWK,HGK,LWK,LWK,LWK					\
    }									\
}

#define WIN_COND3_SYSINFO_ISA_ATTR					\
{									\
    {									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,HGK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,HGK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,HGK,LWK,LWK					\
    }									\
}

#define WIN_COND0_SYSINFO_FEATURES_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK 							\
}

#define WIN_COND1_SYSINFO_FEATURES_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,			\
	LWK,LWK 							\
}

#define WIN_COND2_SYSINFO_FEATURES_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,			\
	LWK,LWK 							\
}

#define WIN_COND0_SYSINFO_TECH_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define WIN_COND1_SYSINFO_TECH_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,HGK,HGK,LWK,LWK 			\
}

#define WIN_COND0_SYSINFO_PERFMON_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK 						\
}

#define WIN_COND1_SYSINFO_PERFMON_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HGK,			\
	HGK,HGK,LWK,LWK 						\
}

#define WIN_COND2_SYSINFO_PERFMON_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HBK,HBK,HBK,HBK,HBK,			\
	HBK,HBK,LWK,LWK 						\
}

#define WIN_COND3_SYSINFO_PERFMON_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,			\
	HGK,HGK,LWK,LWK 						\
}

#define WIN_COND0_SYSINFO_PWR_THERMAL_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define WIN_COND1_SYSINFO_PWR_THERMAL_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK 			\
}

#define WIN_COND2_SYSINFO_PWR_THERMAL_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LWK,LWK 			\
}

#define WIN_COND3_SYSINFO_PWR_THERMAL_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,LWK,LWK 			\
}

#define WIN_SYSINFO_KERNEL_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK 					\
}

#define WIN_COND0_TOPOLOGY_ATTR 					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK		\
}

#define WIN_COND1_TOPOLOGY_ATTR 					\
{									\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK		\
}

#define WIN_COND2_TOPOLOGY_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK		\
}

#define WIN_COND0_MEMORYCONTROLLER_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK						\
}

#define WIN_COND1_MEMORYCONTROLLER_ATTR 				\
{									\
	HWK,HWK,HWK,HWK,HWK						\
}
