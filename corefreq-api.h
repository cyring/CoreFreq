/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef struct
{
	struct
	{
		unsigned char Chr[4];
	} AX, BX, CX, DX;
} BRAND;

#define LEVEL_INVALID	0
#define LEVEL_THREAD	1
#define LEVEL_CORE	2

typedef struct {
	union {
		struct
		{
			unsigned int
			SHRbits	:  5-0,
			Unused1	: 32-5;
		};
		unsigned int Register;
	} AX;
	union {
		struct
		{
			unsigned int
			Threads	: 16-0,
			Unused1	: 32-16;
		};
		unsigned int Register;
	} BX;
	union {
		struct
		{
			unsigned int
			Level	:  8-0,
			Type	: 16-8,
			Unused1 : 32-16;
		};
		unsigned int Register;
	} CX;
	union {
		struct
		{
			unsigned int
			x2ApicID: 32-0;
		};
		unsigned int Register;
	} DX;
} CPUID_TOPOLOGY_LEAF;

typedef struct
{
	LOCAL_APIC	Base;
	int		ApicID,
			CoreID,
			ThreadID;

	struct CACHE_INFO
	{
		union
		{
			struct
			{	// Intel
				unsigned int
				Type	:  5-0,  // Cache Type *
				Level	:  8-5,  // Cache Level (starts at 1)
				Init	:  9-8,  // Self Init. cache level
				Assoc	: 10-9,  // Fully Associative cache
				Unused	: 14-10,
				MxThrdID: 26-14, // Max threads for this cache
				MxCoreID: 32-26; // Max cores for this cache
			};
			struct
			{	// AMD L1
				unsigned int
				ISize	:  8-0,  // Inst. TLB number of entries
				IAssoc	: 16-8,  // Inst. TLB associativity
				DSize	: 24-16, // Data TLB number of entries
				DAssoc	: 32-24; // Data TLB associativity
			} CPUID_0x80000005_L1Tlb2and4M; // for 2 MB & 4 MB pages
			struct
			{	// AMD L2
				unsigned int
				ISize	: 12-0,
				IAssoc	: 16-12,
				DSize	: 28-16,
				DAssoc	: 32-28;
			} CPUID_0x80000006_L2ITlb2and4M;
			unsigned int AX;
		};
		union
		{
			struct
			{	// Intel
				unsigned int
				LineSz	: 12-0,  // L=System Coherency Line Size
				Part	: 22-12, // P=Physical Line partitions
				Way	: 32-22; // W=Ways of associativity
			};
			struct
			{	// AMD L1
				unsigned int
				ISize	:  8-0,  // Inst. TLB number of entries
				IAssoc	: 16-8,  // Inst. TLB associativity *
				DSize	: 24-16, // Data TLB number of entries
				DAssoc	: 32-24; // Data TLB associativity *
			} CPUID_0x80000005_L1Tlb4K; // for 4 KB pages
			struct
			{	// AMD L2
				unsigned int
				ISize	: 12-0,
				IAssoc	: 16-12,
				DSize	: 28-16,
				DAssoc	: 32-28;
			} CPUID_0x80000006_L2Tlb4K;
			unsigned int BX;
		};
		union
		{		// Intel
			unsigned int Set;	// S=Number of Sets
			struct
			{	// AMD L1
				unsigned int
				LineSz	:  8-0,  // L1 data cache line size (B)
				ClPerTag: 16-8,  // L1 data cache lines per tag
				Assoc	: 24-16, // L1 data cache associativity*
				Size	: 32-24; // L1 data cache size (KB)
			} CPUID_0x80000005_L1D;
			struct
			{	// AMD L2
				unsigned int
				LineSz	:  8-0,  // L2 cache line size (B)
				ClPerTag: 12-8,  // L2 cache lines per tag
				Assoc	: 16-12, // L2 cache associativity **
				Size	: 32-16; // L2 cache size (KB) ***
			} CPUID_0x80000006_L2;
			unsigned int CX;
		};
		union
		{
			struct
			{	// Intel
				unsigned int
				WrBack	: 1-0,  // Write-Back **
				Inclus	: 2-1,  // Cache Inclusiveness ***
				Direct	: 3-2,  // Complex Cache Indexing ****
				Resrvd	: 32-3;
			};
			struct
			{	// AMD L1
				unsigned int
				LineSz	:  8-0,  // L1 inst. cache line size (B)
				ClPerTag: 16-8,  // L1 inst. cache lines per tag
				Assoc	: 24-16, // L1 inst. cache associativity
				Size	: 32-24; // L1 inst. cache size (KB) 
			} CPUID_0x80000005_L1I;
			struct
			{	// AMD L3
				unsigned int
				LineSz	:  8-0,  // L3 cache line (B)
				ClPerTag: 12-8,  // L3 cache lines per tag
				Assoc	: 16-12, // L3 cache associativity
				Reserved: 18-16,
				Size	: 32-18; // L3 cache size
			} CPUID_0x80000006_L3;
			unsigned int DX;
		};
		unsigned int	Size;
	} Cache[CACHE_MAX_LEVEL];
} CACHE_TOPOLOGY;

/*
--- Intel Cache Parameters Leaf ---

* Cache Type Field
	0 = Null - No more caches
	1 = Data Cache
	2 = Instruction Cache
	3 = Unified Cache
	4-31 = Reserved

** Write-Back Invalidate/Invalidate
	0 = WBINVD/INVD from threads sharing this cache
		acts upon lower level caches for threads sharing this cache.
	1 = WBINVD/INVD is not guaranteed to act upon lower level caches
		of non-originating threads sharing this cache.

*** Cache Inclusiveness
	0 = Cache is not inclusive of lower cache levels.
	1 = Cache is inclusive of lower cache levels.

**** Complex Cache Indexing
	0 = Direct mapped cache.
	1 = A complex function is used to index the cache,
		potentially using all address bits.


--- AMD Cache Identifiers ---

* L1 data cache associativity
	Bits	Description
	00h	Reserved
	01h	1 way (direct mapped)
	02h	2 way
	03h	3 way
	FEh-04h	[L1IcAssoc] way
	FFh	Fully associative

** L2 cache associativity
	Bits	Description		Bits	Description
	0h	Disabled.		8h	16 ways
	1h	1 way (direct mapped)	9h	Reserved
	2h	2 ways			Ah	32 ways
	3h	Reserved		Bh	48 ways
	4h	4 ways			Ch	64 ways
	5h	Reserved		Dh	96 ways
	6h	8 ways			Eh	128 ways
	7h	Reserved		Fh	Fully associative

*** L2 cache size
	Bits		Description
	03FFh-0000h	Reserved
	0400h		1 MB
	07FFh-0401h	Reserved
	0800h		2 MB
	FFFFh-0801h	Reserved
*/

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
	CLOCK_MODULATION	ClockModulation;
	ENERGY_PERF_BIAS	PerfEnergyBias;
} POWER_THERMAL;

typedef struct
{
	struct	// 64-byte cache line size.
	{
		unsigned long long	V,
					_pad[7];
	} Sync;

	OFFLINE				OffLine;

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

	POWER_THERMAL			PowerThermal;

	struct SAVEAREA
	{
		GLOBAL_PERF_COUNTER	GlobalPerfCounter;
		FIXED_PERF_COUNTER	FixedPerfCounter;
	} SaveArea;

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

		CPUID_0x00000000	StdFunc;
		CPUID_0x80000000	ExtFunc;
	} Query;

	CPUID_STRUCT			CpuID[CPUID_MAX_FUNC];

	CACHE_TOPOLOGY			T;

	unsigned int			Bind;

	CLOCK				Clock;
} CORE;

typedef struct {
		IDLEDRIVER	IdleDriver;

		int		taskCount;
		TASK_MCB	taskList[PID_MAX_DEFAULT];

		MEM_MCB		memInfo;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
} SYSGATE;

typedef struct
{
	FEATURES		Features;

	unsigned int		SleepInterval;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	signed int		ArchID;
	unsigned int		Boost[1+1+8];

	union {
		QPI_FREQUENCY	QPI;
	} NB;

	MC_STRUCT		MC;

	SYSGATE			*SysGate;

	char			Architecture[32];
} PROC;

#define COREFREQ_IOCTL_MAGIC 0xc3
#define COREFREQ_IOCTL_SYSUPDT _IO(COREFREQ_IOCTL_MAGIC, 0)
#define COREFREQ_IOCTL_SYSONCE _IO(COREFREQ_IOCTL_MAGIC, 1)
