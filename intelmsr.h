/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef	union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		MaxBusRatio	: 13-8,
		ReservedBits2	: 50-13,
		PlatformId	: 53-50,
		ReservedBits3	: 64-53;
	};
} PLATFORM_ID;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Bus_Speed	:  3-0,
		ReservedBits	: 64-3;
	};
} FSB_FREQ;

typedef	union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		CurrentRatio	: 16-0,
		ReservedBits1	: 31-16,
		XE		: 32-31,
		ReservedBits2	: 40-32,
		MaxBusRatio	: 45-40,
		ReservedBits3	: 46-45,
		NonInt_BusRatio	: 47-46,
		ReservedBits4	: 64-47;
	};
} PERF_STATUS;

typedef	union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		EIST_Target	: 16-0,
		ReservedBits1	: 32-16,
		Turbo_IDA	: 33-32, // IDA_Turbo DISENGAGE bit
		ReservedBits2	: 64-33;
	};
} PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		MaxNonTurboRatio: 16-8,
		ReservedBits2	: 28-16,
		Ratio_Limited	: 29-28,
		TDC_TDP_Limited	: 30-29,
		ReservedBits3	: 32-30,
		LowPowerMode	: 33-32,
		ConfigTDPlevels	: 35-33,
		ReservedBits4	: 40-35,
		MinimumRatio	: 48-40,
		MinOperatRatio	: 56-48,	// Ivy Bridge, Haswell(-E)
		ReservedBits5	: 64-56;
	};
} PLATFORM_INFO;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Pkg_CST_Limit	:  3-0,
		ReservedBits1	: 10-3,
		IO_MWAIT_Redir	: 11-10,
		ReservedBits2	: 15-11,
		CFG_Lock	: 16-15,
		ReservedBits3	: 24-16,
		Int_Filtering	: 25-24,	// Nehalem
		C3autoDemotion	: 26-25,
		C1autoDemotion	: 27-26,
		C3undemotion	: 28-27,	// Sandy Bridge
		C1undemotion	: 29-28,	// Sandy Bridge
		ReservedBits4	: 64-29;
	};
} CSTATE_CONFIG;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		MaxRatio_1C	:  8-0,
		MaxRatio_2C	: 16-8,
		MaxRatio_3C	: 24-16,
		MaxRatio_4C	: 32-24,
		MaxRatio_5C	: 40-32,
		MaxRatio_6C	: 48-40,
		MaxRatio_7C	: 56-48,
		MaxRatio_8C	: 64-56;
	};
} TURBO_RATIO;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		FastStrings	:  1-0,  // Fast-Strings Enable
		ReservedBits1	:  2-1,
		x87Compat_Enable:  3-2,  // Pentium4, Xeon
		TCC		:  4-3,  // Automatic Thermal Control Circuit
		SplitLockDisable:  5-4,  // Pentium4, Xeon
		ReservedBits2	:  6-5,
		L3Cache_Disable	:  7-6,  // Pentium4, Xeon
		PerfMonitoring	:  8-7,  // Performance Monitoring Available
		SupprLock_Enable:  9-8,  // Pentium4, Xeon
		PrefetchQueueDis: 10-9,  // Pentium4, Xeon
		Int_FERR_Enable	: 11-10, // Pentium4, Xeon
		BTS		: 12-11, // Branch Trace Storage Unavailable
		PEBS		: 13-12, // Precise Event Based Sampling Unavail
		TM2_Enable	: 14-13,
		ReservedBits3	: 16-14,
		EIST		: 17-16, // Enhanced Intel SpeedStep Technology
		ReservedBits4	: 18-17,
		FSM		: 19-18,
		PrefetchCacheDis: 20-19, // Pentium4, Xeon
		ReservedBits5	: 22-20,
		CPUID_MaxVal	: 23-22,
		xTPR		: 24-23,
		L1DataCacheMode	: 25-24, // Pentium4, Xeon
		ReservedBits6	: 34-25,
		XD_Bit_Disable	: 35-34,
		ReservedBits7	: 37-35,
		DCU_Prefetcher	: 38-37,
		Turbo_IDA	: 39-38, // Disable=1 -> CPUID(0x6).IDA=0
		IP_Prefetcher	: 40-39,
		ReservedBits8	: 64-40;
	};
} MISC_PROC_FEATURES;
/*
typedef struct
{
	unsigned long long
		Type		:  8-0,
		ReservedBits1	: 10-8,
		FixeRange	: 11-10,
		Enable		: 12-11,
		ReservedBits2	: 64-12;
} MTRR_DEF_TYPE;
*/
typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0,		// PM2
		Overflow_PMC1	:  2-1,		// PM2
		Overflow_PMC2	:  3-2,		// PM3
		Overflow_PMC3	:  4-3,		// PM3
		Overflow_PMCn	: 32-4,		// PM3
		Overflow_CTR0	: 33-32,	// PM2
		Overflow_CTR1	: 34-33,	// PM2
		Overflow_CTR2	: 35-34,	// PM2
		ReservedBits2	: 55-35,
		TraceToPAPMI	: 56-55,	// PM4, PM3(Broadwell)
		ReservedBits3	: 58-56,
		LBR_Frz		: 59-58,	// PM4
		CTR_Frz		: 60-59,	// PM4
		ASCI		: 61-60,	// PM4
		Overflow_UNC	: 62-61,	// PM3
		Overflow_Buf	: 63-62,	// PM2
		Ovf_CondChg	: 64-63;	// PM2
	};
} GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0,		// PM2
		Clear_Ovf_PMC1	:  2-1,		// PM2
		Clear_Ovf_PMC2	:  3-2,		// PM3
		Clear_Ovf_PMC3	:  4-3,		// PM3
		Clear_Ovf_PMCn	: 32-2,		// PM3
		Clear_Ovf_CTR0 	: 33-32,	// PM2
		Clear_Ovf_CTR1	: 34-33,	// PM2
		Clear_Ovf_CTR2	: 35-34,	// PM2
		ReservedBits2	: 55-35,
		ClrTraceToPA_PMI: 56-55,	// PM4, PM3(Broadwell)
		ReservedBits3	: 61-56,
		Clear_Ovf_UNC	: 62-61,	// PM3
		Clear_Ovf_Buf	: 63-62,	// PM2
		Clear_CondChg	: 64-63;	// PM2
	};
} GLOBAL_PERF_OVF_CTRL;

/* ToDo
	§18.2.4 PM4

	IA32_PERF_GLOBAL_STATUS_RESET
	IA32_PERF_GLOBAL_STATUS_SET
	IA32_PERF_GLOBAL_INUSE
*/

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long int
		ReservedBits1	:  1-0,
		C1E		:  2-1,
		ReservedBits2	: 64-2;
	};
} POWER_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0		:  1-0,		// PM2
		EN_PMC1		:  2-1,		// PM2
		EN_PMC2		:  3-2,		// PM3
		EN_PMC3		:  4-3,		// PM3
		EN_PMCn		: 32-4,		// PM3
		EN_FIXED_CTR0	: 33-32,	// PM2
		EN_FIXED_CTR1	: 34-33,	// PM2
		EN_FIXED_CTR2	: 35-34,	// PM2
		ReservedBits2	: 64-35;
	};
} GLOBAL_PERF_COUNTER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN0_OS		:  1-0,		// PM2
		EN0_Usr		:  2-1,		// PM2
		AnyThread_EN0	:  3-2,		// PM3
		EN0_PMI		:  4-3,		// PM2
		EN1_OS		:  5-4,		// PM2
		EN1_Usr		:  6-5,		// PM2
		AnyThread_EN1	:  7-6,		// PM3
		EN1_PMI		:  8-7,		// PM2
		EN2_OS		:  9-8,		// PM2
		EN2_Usr		: 10-9,		// PM2
		AnyThread_EN2	: 11-10,	// PM3
		EN2_PMI		: 12-11,	// PM2
		ReservedBits	: 64-12;
	};
} FIXED_PERF_COUNTER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		StatusBit       :  1-0,
		StatusLog       :  2-1,
		PROCHOT         :  3-2,
		PROCHOTLog      :  4-3,
		CriticalTemp    :  5-4,
		CriticalTempLog :  6-5,
		Threshold1      :  7-6,
		Threshold1Log   :  8-7,
		Threshold2      :  9-8,
		Threshold2Log   : 10-9,
		PwrLimitStatus	: 11-10,
		PwrLimitLog	: 12-11,
		CurLimitStatus	: 13-12,
		CurLimitLog	: 14-13,
		CrDomLimitStatus: 15-14,
		CrDomLimitLog	: 16-15,
		DTS             : 23-16,
		ReservedBits1   : 27-23,
		Resolution      : 31-27,
		ReadingValid    : 32-31,
		ReservedBits2   : 64-32;
	};
} THERM_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		TM_SELECT	: 17-16, // Unique(Core2), Shared(Xeon, Atom)
		ReservedBits2	: 64-17;
	};
} THERM2_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1   : 16-0,
		Target          : 24-16,
		ReservedBits2   : 64-24;
	};
} TJMAX;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		BSP		:  9-8,
		ReservedBits2	: 10-9,
		EXTD		: 11-10,
		EN		: 12-11,
		Addr		: 64-12;
	};
} LOCAL_APIC;
