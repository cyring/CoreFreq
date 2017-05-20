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

		unsigned long long	NMI;
		unsigned int		SMI;
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

typedef struct
{
	struct {
		union {
			struct {
	/* 29Ch */		P965_MC_ODTCTRL		DRT0;	/* 32 bits */
	/* 250h */		P965_MC_CYCTRK_PCHG	DRT1;	/* 16 bits */
	/* 252h */		P965_MC_CYCTRK_ACT	DRT2;	/* 32 bits */
	/* 256h */		P965_MC_CYCTRK_WR	DRT3;	/* 16 bits */
	/* 258h */		P965_MC_CYCTRK_RD	DRT4;	/* 24 bits */
			} P965;
			struct {
	/* 1210h */		G965_MC_DRAM_TIMING_R0	DRT0;	/* 32 bits */
	/* 1214h */		G965_MC_DRAM_TIMING_R1	DRT1;	/* 32 bits */
	/* 1218h */		G965_MC_DRAM_TIMING_R2	DRT2;	/* 32 bits */
	/* 121Ch */		G965_MC_DRAM_TIMING_R3	DRT3;	/* 32 bits */
			} G965;
			struct {
	/* 265h */		P35_MC_UNKNOWN_R0	DRT0;	/* 16 bits */
	/* 250h */		P35_MC_CYCTRK_PCHG	DRT1;	/* 16 bits */
	/* 252h */		P35_MC_CYCTRK_ACT	DRT2;	/* 32 bits */
	/* 256h */		P35_MC_CYCTRK_WR	DRT3;	/* 16 bits */
	/* 258h */		P35_MC_CYCTRK_RD	DRT4;	/* 24 bits */
	/* 25Dh */		P35_MC_UNKNOWN_R1	DRT5;	/* 16 bits */
			} P35;
			struct {
				NHM_IMC_MRS_VALUE_0_1	MR0_1;
				NHM_IMC_MRS_VALUE_2_3	MR2_3;
				NHM_IMC_RANK_TIMING_A	Rank_A;
				NHM_IMC_RANK_TIMING_B	Rank_B;
				NHM_IMC_BANK_TIMING	Bank;
				NHM_IMC_REFRESH_TIMING	Refresh;
				NHM_IMC_SCHEDULER_PARAMS Params;
			} NHM;
			struct {
	/* 4000h */		C200_TC_DBP		DBP;	/* 32 bits */
	/* 4004h */		C200_TC_RAP		RAP;	/* 32 bits */
	/* 4298h */		C200_TC_RFTP		RFTP;	/* 32 bits */
			} C200;
			struct {
	/* 4C04h*/		C220_DDR_TIMING		Timing;	/* 32 bits */
	/* 4c14h */		C220_DDR_RANK_TIMING	Rank;	/* 32 bits */
	/* 4e98h */		C220_TC_REFRESH_TIMING	Refresh; /*32 bits */
			} C220;
			struct {
	/* 88h */		AMD_0F_DRAM_TIMING_LOW	DTRL;	/* 32 bits */
			} AMD0F;
		};
		union {
	/* 48h */	NHM_IMC_DOD_CHANNEL		DOD;	/* 32 bits */
	/* 40h*/	AMD_0F_DRAM_CS_BASE_ADDR	MBA;	/* 32 bits */
		} DIMM[MC_MAX_DIMM];
	} Channel[MC_MAX_CHA];

	union {
		struct {
	/* 260h */	P965_MC_CKECTRL		CKE0,		/* 32 bits */
						CKE1;		/* 32 bits */
		} P965;
		struct {
	/* 1200h */	G965_MC_DRB_0_1		DRB0,	/* 32 bits @ channel0 */
	/* 1300h*/				DRB1;	/* 32 bits @ channel1 */
		} G965;
		struct {
	/* 260h */	P35_MC_CKECTRL		CKE0,		/* 32 bits */
						CKE1;		/* 32 bits */
		} P35;
		struct {
	/* 3:0-48h */	NHM_IMC_CONTROL		CONTROL;	/* 32 bits */
	/* 3:0 4Ch*/	NHM_IMC_STATUS		STATUS;		/* 32 bits */
		} NHM;
		struct {
	/* 5004h */	C200_MAD_CHANNEL	MAD0,		/* 32 bits */
	/* 5008h */				MAD1;		/* 32 bits */
		} C200;
		struct {
	/* 90h */	AMD_0F_DRAM_CONFIG_LOW	DCRL;		/* 32 bits */
	/* 94h */	AMD_0F_DRAM_CONFIG_HIGH DCRH;		/* 32 bits */
		} AMD0F;
	};

	union {
		struct {
	/* 64h */	NHM_IMC_MAX_DOD		DOD;		/* 32 bits */
		} NHM;
		struct {
	/* 80h */	AMD_0F_DRAM_CS_MAPPING	CS;		/* 32 bits */
		} AMD0F;
	} MaxDIMMs;

	unsigned short		SlotCount, ChannelCount;
} MC_REGISTERS;

typedef union
{
	union {
		struct {
			MCH_CLKCFG		ClkCfg;
		};
		struct {
			NHM_IMC_CLK_RATIO_STATUS DimmClock;
			X58_QPI_FREQUENCY	QuickPath;
		};
		struct {
			AMD_0F_HTT_UNIT_ID	UnitID;
			AMD_0F_HTT_FREQUENCY	LDTi_Freq[3];
		};
	};
} BUS_REGISTERS;


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
/* ToDo: Package C-state Residency Counters
	struct
	{
	    unsigned long long	PC02, // Goldmont, Sandy-Bridge, Phi
				PC03, // Goldmont, Nehalem, Sandy-Bridge, Phi
				PC06, // Goldmont, Nehalem, Sandy-Bridge, Phi
				PC07, // Nehalem, Sandy-Bridge, Phi
				PC08, // Haswell
				PC09, // Haswell
				PC10; // Goldmont, Haswell
	} Counter[2];

	struct
	{
	    unsigned long long	PC02,
				PC03,
				PC06,
				PC07,
				PC08,
				PC09,
				PC10;
	} Delta;
*/
	FEATURES		Features;

	unsigned int		SleepInterval;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	signed int		ArchID;
	unsigned int		Boost[1+1+8];

	struct {
		BUS_REGISTERS	Bus;
		MC_REGISTERS	MC[MC_MAX_CTRL];
		unsigned short	CtrlCount;
		unsigned short	ChipID;
	} Uncore;

	SYSGATE			*SysGate;

	struct {
		signed int	Experimental,
				hotplug,
				pci,
				nmi;
	} Registration;

	char			Architecture[32];
} PROC;

#define COREFREQ_IOCTL_MAGIC 0xc3
#define COREFREQ_IOCTL_SYSUPDT _IO(COREFREQ_IOCTL_MAGIC, 0)
#define COREFREQ_IOCTL_SYSONCE _IO(COREFREQ_IOCTL_MAGIC, 1)
