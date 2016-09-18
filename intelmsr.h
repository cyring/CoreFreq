/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0		:  1-0,
		EN_PMC1		:  2-1,
		EN_PMC2		:  3-2,
		EN_PMC3		:  4-3,
		EN_PMCn		: 32-4,
		EN_FIXED_CTR0	: 33-32,
		EN_FIXED_CTR1	: 34-33,
		EN_FIXED_CTR2	: 35-34,
		ReservedBits2	: 64-35;
	};
} GLOBAL_PERF_COUNTER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN0_OS		:  1-0,
		EN0_Usr		:  2-1,
		AnyThread_EN0	:  3-2,
		EN0_PMI		:  4-3,
		EN1_OS		:  5-4,
		EN1_Usr		:  6-5,
		AnyThread_EN1	:  7-6,
		EN1_PMI		:  8-7,
		EN2_OS		:  9-8,
		EN2_Usr		: 10-9,
		AnyThread_EN2	: 11-10,
		EN2_PMI		: 12-11,
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
