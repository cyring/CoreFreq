/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef	struct
{
	LOCAL_APIC	Base;
	int		ApicID,
			CoreID,
			ThreadID;
	struct
	{
		union
		{
			struct
			{
				unsigned int
				Type:	 5-0,
				Level:	 8-5,
				Init:	 9-8,
				Assoc:	10-9,
				Unused:	14-10,
				MaxID:	26-14,
				PerPkg:	32-26;
			};
			unsigned int AX;
		};
		union
		{
			struct
			{
				unsigned int
				LineSz:	12-0,
				Part:	22-12,
				Way:	32-22;
			};
			unsigned int BX;
		};
		unsigned int	Set;
		union
		{
			struct
			{
				unsigned int
				WrBack:	 1-0,
				Inclus:	 2-1,
				Direct:	 3-2,
				Resrvd: 32-3;
			};
			unsigned int DX;
		};
		unsigned int	Size;
	} Cache[CACHE_MAX_LEVEL];
} TOPOLOGY;

typedef struct
{
	unsigned int		Sensor,
				Target;
	struct {
		unsigned int
				TCC_Enable:  1-0,
				TM2_Enable:  2-1,
				TM_Select :  3-2,
				Trip      :  4-3,
				Unused    : 32-4;
	};
} THERMAL;

typedef struct
{
	volatile struct
	{	// Cache line size aligned structure.
		unsigned long long	V,
					_pad64[7];
	} Sync;

	struct
	{
		unsigned long long 	INST;
		struct
		{
			unsigned long long
				UCC,
				URC;
		}			C0;
			unsigned long long
					C3,
					C6,
					C7,
					TSC;

		unsigned long long	C1;
	} Counter[2];

	struct
	{
		unsigned long long
					INST;
		struct
		{
		unsigned long long
				UCC,
				URC;
		}			C0;
		unsigned long long
					C3,
					C6,
					C7,
					TSC,
					C1;
	} Delta;

	THERMAL				Thermal;

	struct SAVEAREA
	{
		GLOBAL_PERF_COUNTER	GlobalPerfCounter;
		FIXED_PERF_COUNTER	FixedPerfCounter;
	} SaveArea;

	TOPOLOGY			T;

	unsigned int			Bind,
					OffLine;

	CLOCK				Clock;

	struct
	{
		unsigned long long
					EIST	:  1-0,	// Package
					C1E	:  2-1,	// Package
					Turbo	:  3-2,	// Thread
					C3A	:  4-3,	// Core
					C1A	:  5-4,	// Core
					C3U	:  6-5,	// Sandy Bridge
					C1U	:  7-6,	// Sandy Bridge
					Unused	: 64-7;
	} Query;
} CORE;

typedef struct
{
	FEATURES		Features;

	unsigned int		msleep;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	unsigned char		Architecture[32];
	signed int		ArchID;
	unsigned int		Boost[1+1+8];

	IDLEDRIVER		IdleDriver;
} PROC;
