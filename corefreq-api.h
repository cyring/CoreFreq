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

/* Cache Parameters Leaf.
EAX Bits
04-00: Cache Type Field
	0 = Null - No more caches
	1 = Data Cache
	2 = Instruction Cache
	3 = Unified Cache
	4-31 = Reserved
07-05: Cache Level (starts at 1)
08   : Self Initializing cache level (does not need SW initialization)
09   : Fully Associative cache
13-10: Reserved
25-14: Maximum # of addressable IDs for logical processors sharing this cache
31-26: Maximum # of addressable IDs for processor cores in the physical package

EBX Bits
11-00: L = System Coherency Line Size
21-12: P = Physical Line partitions
31-22: W = Ways of associativity

ECX Bits
31-00: S = Number of Sets

EDX Bits
0: Write-Back Invalidate/Invalidate
	0 = WBINVD/INVD from threads sharing this cache
		acts upon lower level caches for threads sharing this cache.
	1 = WBINVD/INVD is not guaranteed to act upon lower level caches
		of non-originating threads sharing this cache.
1: Cache Inclusiveness
	0 = Cache is not inclusive of lower cache levels.
	1 = Cache is inclusive of lower cache levels.
2: Complex Cache Indexing
	0 = Direct mapped cache.
	1 = A complex function is used to index the cache,
		potentially using all address bits.
31-03: Reserved = 0
*/
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
{	// Cache line size aligned structures.
	volatile struct
	{
		unsigned long long	V,
					_pad[3];
	} Sync;
	volatile OFFLINE		OffLine;

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

	unsigned int			Bind;

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

	unsigned int		SleepInterval;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	unsigned char		Architecture[32];
	signed int		ArchID;
	unsigned int		Boost[1+1+8];

	IDLEDRIVER		IdleDriver;
} PROC;
