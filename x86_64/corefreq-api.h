/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define DRV_DEVNAME	"corefreqk"
#define DRV_FILENAME	"/dev/"DRV_DEVNAME

#define ID_RO_VMA_PROC	(CORE_COUNT + 0)
#define ID_RW_VMA_PROC	(CORE_COUNT + ID_RO_VMA_PROC)
#define ID_RO_VMA_GATE	(CORE_COUNT + ID_RW_VMA_PROC)
#define ID_RO_VMA_CORE	(CORE_COUNT + ID_RO_VMA_GATE)
#define ID_RW_VMA_CORE	(CORE_COUNT + ID_RO_VMA_CORE)
#define ID_ANY_VMA_JAIL (CORE_COUNT + ID_RW_VMA_CORE)

#define WAKEUP_RATIO	4
#define LOOP_MIN_MS	100
#define LOOP_MAX_MS	((1000 - 1) * WAKEUP_RATIO)
#define LOOP_DEF_MS	1000
#define TICK_DEF_MS	2000

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
			SHRbits :  5-0,
			Unused1 : 32-5;
		};
		unsigned int Register;
	} AX;
	union {
		struct
		{
			unsigned int
			Threads : 16-0,
			Unused1 : 32-16;
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

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		BSP		:  9-8,
		ReservedBits2	: 10-9,
		x2APIC_EN	: 11-10, /* Intel Nehalem [06_1A]; AMD[F17h] */
		APIC_EN 	: 12-11, /* Intel P4, Xeon, P6; AMD	*/
		Addr		: 64-12;
	};
} LOCAL_APIC;

typedef struct
{
	LOCAL_APIC		Base;
	signed int		ApicID,
				CoreID,
				ThreadID,
				PackageID;
	union {
		unsigned int	ID; /* AMD-17h MSR(0x0000002a) | Intel Hybrid */
	    struct {
		unsigned int	Node:  8-0, /* CPUID(0x8000001e):ECX[8-0] */
				CCX : 16-8, /* CPUID(0x8000001e):EAX[32-0]:[3]*/
				CCD : 24-16,
				CMP : 32-24;
	    };
	    CPUID_0x0000001a	Hybrid;
	} Cluster;

	struct CACHE_INFO
	{
		union
		{
			struct
			{	/* Intel				*/
				unsigned int
				Type	:  5-0,  /* Cache Type* 	*/
				Level	:  8-5,  /* Cache Level (starts at 1) */
				Init	:  9-8,  /* Self Init. cache level    */
				Assoc	: 10-9,  /* Fully Associative cache   */
				Unused	: 14-10,
				MxThrdID: 26-14, /* Max threads w/ this cache */
				MxCoreID: 32-26; /* Max cores for this cache  */
			};
			struct
			{	/* AMD L1				*/
				unsigned int
				ISize	:  8-0,  /* Inst. TLB number/entries  */
				IAssoc	: 16-8,  /* Inst. TLB associativity   */
				DSize	: 24-16, /* Data TLB number/entries   */
				DAssoc	: 32-24; /* Data TLB associativity    */
			} CPUID_0x80000005_L1Tlb2and4M; /* 2 MB & 4 MB pages  */
			struct
			{	/* AMD L2				*/
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
			{	/* Intel				*/
				unsigned int
				LineSz	: 12-0,  /* L=Sys Coherency Line Size */
				Part	: 22-12, /* P=Phys Line partitions    */
				Way	: 32-22; /* W=Ways of associativity   */
			};
			struct
			{	/* AMD L1				*/
				unsigned int
				ISize	:  8-0,  /* Inst. TLB number/entries  */
				IAssoc	: 16-8,  /* Inst. TLB associativity*  */
				DSize	: 24-16, /* Data TLB number/entries   */
				DAssoc	: 32-24; /* Data TLB associativity*   */
			} CPUID_0x80000005_L1Tlb4K; /* for 4 KB pages	*/
			struct
			{	/* AMD L2				*/
				unsigned int
				ISize	: 12-0,
				IAssoc	: 16-12,
				DSize	: 28-16,
				DAssoc	: 32-28;
			} CPUID_0x80000006_L2Tlb4K;
			unsigned int BX;
		};
		union
		{		/* Intel				*/
			unsigned int Set;	/* S=Number of Sets	*/
			struct
			{	/* AMD L1-Data				*/
				unsigned int
				LineSz	:  8-0,  /* L1-D cache line size (B)  */
				ClPerTag: 16-8,  /* L1-D cache lines per tag  */
				Assoc	: 24-16, /* L1-D cache associativity* */
				Size	: 32-24; /* L1-D cache size (KB)      */
			} CPUID_0x80000005_L1D;
			struct
			{	/* AMD L2				*/
				unsigned int
				LineSz	:  8-0,  /* L2 cache line size (B)    */
				ClPerTag: 12-8,  /* L2 cache lines per tag    */
				Assoc	: 16-12, /* L2 cache associativity**  */
				Size	: 32-16; /* L2 cache size (KB)***     */
			} CPUID_0x80000006_L2;
			unsigned int CX;
		};
		union
		{
			struct
			{	/* Intel				*/
				unsigned int
				WrBack	: 1-0,  /* Write-Back**		*/
				Inclus	: 2-1,  /* Cache Inclusiveness*** */
				Direct	: 3-2,  /* Cache Indexing****	*/
				Resrvd	: 32-3;
			};
			struct
			{	/* AMD L1-Instruction			*/
				unsigned int
				LineSz	:  8-0,  /* L1-I cache line size (B)  */
				ClPerTag: 16-8,  /* L1-I cache lines per tag  */
				Assoc	: 24-16, /* L1-I cache associativity  */
				Size	: 32-24; /* L1-I cache size (KB)      */
			} CPUID_0x80000005_L1I;
			struct
			{	/* AMD L3				*/
				unsigned int
				LineSz	:  8-0,  /* L3 cache line (B)	*/
				ClPerTag: 12-8,  /* L3 cache lines per tag */
				Assoc	: 16-12, /* L3 cache associativity */
				Reserved: 18-16,
				Size	: 32-18; /* L3 cache size	*/
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
	THERMAL_PARAM			Param;
	unsigned int			Sensor;
	signed int			VID;
	struct {
		enum THERM_PWR_EVENTS	Events[eDIM];
	};
	PERF_CONTROL			PerfControl;
	CLOCK_MODULATION		ClockModulation;
	ENERGY_PERF_BIAS		PerfEnergyBias;
	MISC_PWR_MGMT			PwrManagement;
	HWP_CAPABILITIES		HWP_Capabilities;
	HWP_INTERRUPT			HWP_Interrupt;
	HWP_REQUEST			HWP_Request;
	struct ACPI_CPPC_STRUCT {
		unsigned short		Highest,
					Guaranteed,
					Efficient,
					Lowest,
					Minimum,
					Maximum,
					Desired,
					Energy;
	} ACPI_CPPC;
} POWER_THERMAL;

/*				ACPI/OSPM

	Highest Performance ... .------,
				|######|
				|######|
	Nominal Performance ... |------|  ..
				|//////|    \
				|//////|    :
				|//////|    :
				|//////|    :
				|//////|    --- Guaranteed Performance
Lowest Nonlinear Performance .. --------    :	Allowed Range
				|OOOOOO|    :
				|OOOOOO|    :
				|OOOOOO|    :
	Lowest Performance .... --------  ../
				|++++++|
				|++++++|
				|++++++|
			0 ..... `------'
*/

typedef struct
{
	Bit64				OffLine __attribute__ ((aligned (8)));

	struct
	{
		unsigned long long	TSC;
	} Overhead __attribute__ ((aligned (8)));

	struct
	{
		unsigned long long	INST;
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

		struct
		{
		unsigned long long	ACCU;
		} Power;
	} Counter[2] __attribute__ ((aligned (8)));

	struct
	{
	unsigned long long
				C1,
				C2,
				C3,
				C4,
				C5,
				C6,
				C7;
	} VPMC __attribute__ ((aligned (8)));

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

		struct
		{
		unsigned long long	ACCU;
		} Power;

		unsigned int		SMI;
	} Delta __attribute__ ((aligned (8)));

	POWER_THERMAL			PowerThermal;
	THERMAL_POINT			ThermalPoint;

	struct
	{
		unsigned int		SMI;
		struct {
			unsigned int
					LOCAL,
					UNKNOWN,
					PCISERR,
					IOCHECK;
		}			NMI;
	} Interrupt;

	struct
	{
		CPUID_0x00000000	StdFunc;
		CPUID_0x80000000	ExtFunc;

		struct {
		unsigned long long
					CfgLock :  1-0,  /* Core	*/
					IORedir :  2-1,  /* Core	*/
					Unused	: 32-3,
					Microcode:64-32; /* Thread	*/
		};
		unsigned short int	CStateLimit;
		struct {
		unsigned short int	CStateInclude;	/* Intel	*/
		unsigned short int	CStateBaseAddr; /* Any I/O BAR	*/
		};
	} Query;

	CACHE_TOPOLOGY			T;

	struct {
		Bit64			RFLAGS	__attribute__ ((aligned (8))),
					CR0	__attribute__ ((aligned (8))),
					CR3	__attribute__ ((aligned (8))),
					CR4	__attribute__ ((aligned (8))),
					CR8	__attribute__ ((aligned (8))),
					EFER	__attribute__ ((aligned (8)));
		union {
			Bit64		EFCR	__attribute__ ((aligned (8)));
			VM_CR		VMCR;

		};
		Bit64			XCR0	__attribute__ ((aligned (8)));
		Bit64			XSS	__attribute__ ((aligned (8)));
		Bit64			SYSCFG	__attribute__ ((aligned (8)));
		HWCR			HWCR	__attribute__ ((aligned (8)));
	} SystemRegister;

	unsigned int			Bind;

	CLOCK				Clock;

	CPUID_STRUCT			CpuID[CPUID_MAX_FUNC];

	unsigned int			Boost[BOOST(SIZE)];

	COF_UNION			Ratio;
} CORE_RO;

typedef struct
{
	struct	/* 64-byte cache line size.				*/
	{
		Bit64			V,
					_pad[7];
	} Sync __attribute__ ((aligned (8)));

} CORE_RW;

typedef struct
{
	struct {
		union {
			struct {
	/* 100h */		P945_MC_DRAM_RANK_BOUND DRB[4]; /* 4x8 bits   */
	/* 110h */		P945_MC_DRAM_TIMING_R0	DRT0;	/* 32 bits    */
	/* 114h */		P945_MC_DRAM_TIMING_R1	DRT1;	/* 32 bits    */
	/* 118h */		P945_MC_DRAM_TIMING_R2	DRT2;	/* 32 bits    */
	/* 10Eh */		P945_MC_DRAM_BANK_ARCH	BANK;	/* 16 bits    */
	/* 40Ch */		P945_MC_DRAM_RANK_WIDTH WIDTH;	/* 16 bits    */
			} P945;
			struct {
	/* 100h */		P945_MC_DRAM_RANK_BOUND DRB[4]; /* 4x8 bits   */
	/* 114h */		P955_MC_DRAM_TIMING_R1	DRT1;	/* 32 bits    */
	/* 10Eh */		P945_MC_DRAM_BANK_ARCH	BANK;	/* 16 bits    */
	/* 40Ch */		P945_MC_DRAM_RANK_WIDTH WIDTH;	/* 16 bits    */
			} P955;
			struct {
	/* 29Ch */		P965_MC_ODTCTRL 	DRT0;	/* 32 bits    */
	/* 250h */		P965_MC_CYCTRK_PCHG	DRT1;	/* 16 bits    */
	/* 252h */		P965_MC_CYCTRK_ACT	DRT2;	/* 32 bits    */
	/* 256h */		P965_MC_CYCTRK_WR	DRT3;	/* 16 bits    */
	/* 258h */		P965_MC_CYCTRK_RD	DRT4;	/* 24 bits    */
			} P965;
			struct {
	/* 1210h */		G965_MC_DRAM_TIMING_R0	DRT0;	/* 32 bits    */
	/* 1214h */		G965_MC_DRAM_TIMING_R1	DRT1;	/* 32 bits    */
	/* 1218h */		G965_MC_DRAM_TIMING_R2	DRT2;	/* 32 bits    */
	/* 121Ch */		G965_MC_DRAM_TIMING_R3	DRT3;	/* 32 bits    */
			} G965;
			struct {
	/* 200h */		P35_MC_DRAM_RANK_BOUND	DRB[4]; /* 4x16 bits  */
	/* 208h */		P35_MC_DRAM_BANK_RANK	DRA[2]; /* 2x16 bits  */
	/* 265h */		P35_MC_UNKNOWN_R0	DRT0;	/* 16 bits    */
	/* 250h */		P35_MC_CYCTRK_PCHG	DRT1;	/* 16 bits    */
	/* 252h */		P35_MC_CYCTRK_ACT	DRT2;	/* 32 bits    */
	/* 256h */		P35_MC_CYCTRK_WR	DRT3;	/* 16 bits    */
	/* 258h */		P35_MC_CYCTRK_RD	DRT4;	/* 24 bits    */
	/* 25Dh */		P35_MC_UNKNOWN_R1	DRT5;	/* 16 bits    */
			} P35;
			struct {
				NHM_IMC_MRS_VALUE_0_1	MR0_1;
				NHM_IMC_MRS_VALUE_2_3	MR2_3;
				NHM_IMC_RANK_TIMING_A	Rank_A;
				NHM_IMC_RANK_TIMING_B	Rank_B;
				NHM_IMC_BANK_TIMING	Bank;
				NHM_IMC_REFRESH_TIMING	Refresh;
				NHM_IMC_CKE_TIMING	CKE_Timing;
				NHM_IMC_SCHEDULER_PARAMS Sched;
				NHM_IMC_DIMM_INIT_PARAMS DIMM_Init;
			} NHM;
			struct {
	/* 4000h */		SNB_IMC_TC_DBP		DBP;	/* 32 bits    */
	/* 4004h */		SNB_IMC_TC_RAP		RAP;	/* 32 bits    */
	/* 4008h */		SNB_IMC_TC_RWP		RWP;	/* 32 bits    */
	/* 400Ch */		SNB_IMC_TC_OTP		OTP;	/* 32 bits    */
	/* 40B0h */		SNB_IMC_PDWN		PDWN;	/* 32 bits    */
	/* 4294h */		SNB_IMC_TC_RFP		RFP;	/* 32 bits    */
	/* 4298h */		SNB_IMC_TC_RFTP 	RFTP;	/* 32 bits    */
	/* 42A4h */		SNB_IMC_TC_SRFTP	SRFTP;	/* 32 bits    */
			} SNB;
			struct {
	/*  200h */		SNB_IMC_TC_DBP		DBP;	/* 32 bits    */
	/*  204h */		SNB_IMC_TC_RAP		RAP;	/* 32 bits    */
	/*  208h */		SNB_IMC_TC_RWP		RWP;	/* 32 bits    */
	/*  20Ch */		SNB_IMC_TC_OTP		OTP;	/* 32 bits    */
	/*  210h */		SNB_IMC_TC_RFP		RFP;	/* 32 bits    */
	/*  214h */		SNB_IMC_TC_RFTP 	RFTP;	/* 32 bits    */
	/*  218h */		SNB_IMC_TC_SRFTP	SRFTP;	/* 32 bits    */
			} SNB_EP;
			struct {
	/* 4C00h */		HSW_DDR_TIMING_4C00	REG4C00; /*32 bits    */
	/* 4C04h */		HSW_DDR_TIMING		Timing; /* 32 bits    */
	/* 4C08h */		HSW_DDR_RANK_TIMING_A	Rank_A; /* 32 bits    */
	/* 4C0Ch */		HSW_DDR_RANK_TIMING_B	Rank_B; /* 32 bits    */
	/* 4C14h */		HSW_DDR_RANK_TIMING	Rank;	/* 32 bits    */
	/* 4E98h */		HSW_TC_REFRESH_TIMING	Refresh; /*32 bits    */
	/* 4CB0h */		HSW_PM_POWER_DOWN	PDWN;	/* 32 bits    */
			} HSW;
			struct {
	/* 4000h */		SKL_IMC_CR_TC_PRE	Timing; /* 32 bits    */
	/* 4004h */		SKL_IMC_CR_TC_ACT	ACT;	/* 32 bits    */
	/* 400Ch */		SKL_IMC_CR_TC_RDRD	RDRD;	/* 32 bits    */
	/* 4010h */		SKL_IMC_CR_TC_RDWR	RDWR;	/* 32 bits    */
	/* 4014h */		SKL_IMC_CR_TC_WRRD	WRRD;	/* 32 bits    */
	/* 4018h */		SKL_IMC_CR_TC_WRWR	WRWR;	/* 32 bits    */
	/* 401Ch */		SKL_IMC_CR_SC_CFG	Sched;	/* 32 bits    */
	/* 4070h */		SKL_IMC_CR_TC_ODT	ODT;	/* 32 bits    */
	/* 423Ch */		SKL_IMC_REFRESH_TC	Refresh; /*32 bits    */
			} SKL;
			struct {
	/* 4000h */		RKL_IMC_CR_TC_PRE	Timing; /* 32 bits    */
	/* 4004h */		RKL_IMC_CR_TC_ACT	ACT;	/* 32 bits    */
	/* 400Ch */		RKL_IMC_CR_TC_RDRD	RDRD;	/* 32 bits    */
	/* 4010h */		RKL_IMC_CR_TC_RDWR	RDWR;	/* 32 bits    */
	/* 4014h */		RKL_IMC_CR_TC_WRRD	WRRD;	/* 32 bits    */
	/* 4018h */		RKL_IMC_CR_TC_WRWR	WRWR;	/* 32 bits    */
	/* 4050h */		RKL_IMC_TC_PWDEN	PWDEN;	/* 64-bits    */
	/* 4070h */		RKL_IMC_CR_TC_ODT	ODT;	/* 64 bits    */
	/* 4088h */		RKL_IMC_SC_GS_CFG	Sched;	/* 64 bits    */
	/* 423Ch */		RKL_IMC_REFRESH_TC	Refresh; /*32 bits    */
	/* 42C4h */		RKL_IMC_SREXITTP	SRExit;	/* 32 bits    */
			} RKL;
			struct {
	/* 4000h */		TGL_IMC_CR_TC_PRE	Timing; /* 64 bits    */
	/* 4008h */		TGL_IMC_CR_TC_ACT	ACT;	/* 32 bits    */
	/* 400Ch */		TGL_IMC_CR_TC_RDRD	RDRD;	/* 32 bits    */
	/* 4010h */		TGL_IMC_CR_TC_RDWR	RDWR;	/* 32 bits    */
	/* 4014h */		TGL_IMC_CR_TC_WRRD	WRRD;	/* 32 bits    */
	/* 4018h */		TGL_IMC_CR_TC_WRWR	WRWR;	/* 32 bits    */
	/* 4050h */		TGL_IMC_TC_PWDEN	PWDEN;	/* 64-bits    */
	/* 4070h */		TGL_IMC_CR_TC_ODT	ODT;	/* 64 bits    */
	/* 4088h */		TGL_IMC_SC_GS_CFG	Sched;	/* 64 bits    */
	/* 423Ch */		TGL_IMC_REFRESH_TC	Refresh; /*32 bits    */
	/* 42C0h */		TGL_IMC_SREXITTP	SRExit;	/* 64 bits    */
			} TGL;
			struct {
	/* E000h */		ADL_IMC_CR_TC_PRE	Timing; /* 64 bits    */
	/* E008h */		ADL_IMC_CR_TC_ACT	ACT;	/* 32 bits    */
	/* E00Ch */		ADL_IMC_CR_TC_RDRD	RDRD;	/* 32 bits    */
	/* E010h */		ADL_IMC_CR_TC_RDWR	RDWR;	/* 32 bits    */
	/* E014h */		ADL_IMC_CR_TC_WRRD	WRRD;	/* 32 bits    */
	/* E018h */		ADL_IMC_CR_TC_WRWR	WRWR;	/* 32 bits    */
	/* E050h */		ADL_IMC_TC_PWDEN	PWDEN;	/* 64-bits    */
	/* E070h */		ADL_IMC_CR_TC_ODT	ODT;	/* 64 bits    */
	/* E088h */		ADL_IMC_SC_GS_CFG	Sched;	/* 64 bits    */
	/* E43Ch */		ADL_IMC_REFRESH_TC	Refresh; /*32 bits    */
	/* E4C0h */		ADL_IMC_SREXITTP	SRExit;	/* 64 bits    */
			} ADL;
			struct {
	/* E000h */		MTL_IMC_CR_TC_PRE	Timing; /* 64 bits    */
	/* E138h */		MTL_IMC_CR_TC_ACT	ACT;	/* 64 bits    */
	/* E00Ch */		MTL_IMC_CR_TC_RDRD	RDRD;	/* 32 bits    */
	/* E010h */		MTL_IMC_CR_TC_RDWR	RDWR;	/* 32 bits    */
	/* E014h */		MTL_IMC_CR_TC_WRRD	WRRD;	/* 32 bits    */
	/* E018h */		MTL_IMC_CR_TC_WRWR	WRWR;	/* 32 bits    */
	/* E050h */		MTL_IMC_TC_PWDEN	PWDEN;	/* 64-bits    */
	/* E070h */		MTL_IMC_CR_TC_ODT	ODT;	/* 64 bits    */
	/* E088h */		MTL_IMC_SC_GS_CFG	Sched;	/* 64 bits    */
	/* E4A0h */		MTL_IMC_REFRESH_TC	Refresh; /*64 bits    */
	/* E4C0h */		MTL_IMC_SREXITTP	SRExit;	/* 64 bits    */
			} MTL;
			struct {
	/* 88h */		AMD_0F_DRAM_TIMING_LOW	DTRL;	/* 32 bits    */
			} AMD0Fh;
			struct
			{
				AMD_17_UMC_DEBUG_MISC	DbgMisc;/* 32 bits   */
			  struct {
				AMD_17_UMC_ECC_CAP_LO	Lo;	/* 32 bits   */
				AMD_17_UMC_ECC_CAP_HI	Hi;	/* 32 bits   */
			  } ECC;
			  struct {
			    struct {
				unsigned int value;
			    }				Chip,	/* 32 bits   */
							Mask;	/* 32 bits   */
			  } CHIP[4][2];
				AMD_ZEN_UMC_CONFIG	CONFIG; /* 32 bits   */
				AMD_17_UMC_SPAZ_CTRL	SPAZ;	/* 32 bits   */
				AMD_17_UMC_DATA_CTRL	ENCR;	/* 32 bits   */
				AMD_ZEN_UMC_CFG_MISC	MISC;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR1	DTR1;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR2	DTR2;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR3	DTR3;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR4	DTR4;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR5	DTR5;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR6	DTR6;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR7	DTR7;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR8	DTR8;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR9	DTR9;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR10 DTR10;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR12 DTR12;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR13 DTR13;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR14 DTR14;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR20 DTR20;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR21 DTR21;	/* 32 bits   */
				AMD_17_UMC_TIMING_DTR22 DTR22;	/* 32 bits   */
				AMD_ZEN_UMC_TIMING_DTRFC DTRFC; /* 32 bits   */
				AMD_ZEN_UMC_TIMING_RFCSB RFCSB; /* 32 bits   */
				AMD_17_UMC_TIMING_DTR35 DTR35;	/* 32 bits   */
			    struct {
				unsigned int value;		/* 32 bits   */
			    }				BGS,
							BGS_ALT;
			} AMD17h;
		};
		union {
	/* 1208h */	G965_MC_DRAM_RANK_ATTRIB	DRA;	/* 32 bits    */
	/* 48h */	NHM_IMC_DOD_CHANNEL		DOD;	/* 32 bits    */
	/* 80h */	SNB_EP_DIMM_MTR 		MTR;	/* 32 bits    */
	/* 40h */	AMD_0F_DRAM_CS_BASE_ADDR	MBA;	/* 32 bits    */
		    struct {
	/* 5003{0,4}h*/ AMD_ZEN_UMC_DRAM_ADDR_CFG	DAC;	/* 32 bits    */
	/* 5008{0,4}h*/ AMD_17_UMC_DIMM_CFG		CFG;	/* 32 bits    */
		    } AMD17h;
		} DIMM[MC_MAX_DIMM];
	} Channel[MC_MAX_CHA];

	union {
		struct {
	/* 200h */	P945_MC_DCC		DCC;		/* 32 bits    */
		} P945;
		struct {
	/* 200h */	P945_MC_DCC		DCC;		/* 32 bits    */
		} P955;
		struct {
	/* 260h */	P965_MC_CKECTRL 	CKE0,		/* 32 bits    */
						CKE1;		/* 32 bits    */
		} P965;
		struct {
	/* 1200h */	G965_MC_DRB_0_1 	DRB0,	/* 32 bits @ channel0 */
	/* 1300h */				DRB1;	/* 32 bits @ channel1 */
		} G965;
		struct {
	/* 260h */	P35_MC_CKECTRL		CKE0,		/* 32 bits    */
						CKE1;		/* 32 bits    */
		} P35;
		struct {
	/* 0F00h */	SOC_MC_DRP		DRP;		/* 32 bits    */
	/* 0F00h */	SOC_MC_DTR0		DTR0;		/* 32 bits    */
	/* 0F00h */	SOC_MC_DTR1		DTR1;		/* 32 bits    */
	/* 0F00h */	SOC_MC_DTR2		DTR2;		/* 32 bits    */
	/* 0F00h */	SOC_MC_DTR3		DTR3;		/* 32 bits    */
	/* 0F00h */	SOC_MC_DRFC		DRFC;		/* 32 bits    */
			SOC_MC_DRMC		DRMC;		/* 32 bits    */
	/* 0F00h */	SOC_MC_BIOS_CFG 	BIOS_CFG;	/* 32 bits    */
		} SLM;
		struct {
	/* 3:0-48h */	NHM_IMC_CONTROL 	CONTROL;	/* 32 bits    */
	/* 3:0 4Ch */	NHM_IMC_STATUS		STATUS; 	/* 32 bits    */
		} NHM;
		struct {
	/* 5000h */	SNB_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* 5004h */	SNB_IMC_MAD_CHANNEL	MAD0,		/* 32 bits    */
	/* 5008h */				MAD1;		/* 32 bits    */
		} SNB;
		struct {
	/* 7Ch*/	SNB_EP_MC_TECH		TECH;		/* 32 bits    */
	/* 80h */	SNB_EP_TADWAYNESS	TAD;		/* 12x32 bits */
		} SNB_EP;
		struct {
	/* 7Ch*/	HSW_EP_MC_TECH		TECH;		/* 32 bits    */
	/* 80h */	HSW_EP_TADWAYNESS	TAD;		/* 12x32 bits */
		} HSW_EP;
		struct {
	/* 5000h */	SKL_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* 5004h */	SKL_IMC_MAD_CHANNEL	MADC0,		/* 32 bits    */
	/* 5008h */				MADC1;		/* 32 bits    */
	/* 500Ch */	SKL_IMC_MAD_DIMM	MADD0,		/* 32 bits    */
	/* 5010h */				MADD1;		/* 32 bits    */
		} SKL;
		struct {
	/* 5000h */	RKL_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* 5004h */	RKL_IMC_MAD_CHANNEL	MADC0,		/* 32 bits    */
	/* 5008h */				MADC1;		/* 32 bits    */
	/* 500Ch */	RKL_IMC_MAD_DIMM	MADD0,		/* 32 bits    */
	/* 5010h */				MADD1;		/* 32 bits    */
		} RKL;
		struct {
	/* 5000h */	TGL_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* 5004h */	TGL_IMC_MAD_CHANNEL	MADC0,		/* 32 bits    */
	/* 5008h */				MADC1;		/* 32 bits    */
	/* 500Ch */	TGL_IMC_MAD_DIMM	MADD0,		/* 32 bits    */
	/* 5010h */				MADD1;		/* 32 bits    */
		} TGL;
		struct {
	/* D800h */	ADL_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* D804h */	ADL_IMC_MAD_CHANNEL	MADC0,		/* 32 bits    */
	/* D808h */				MADC1;		/* 32 bits    */
	/* D80Ch */	ADL_IMC_MAD_DIMM	MADD0,		/* 32 bits    */
	/* D810h */				MADD1;		/* 32 bits    */
		} ADL;
		struct {
	/* D800h */	MTL_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* D804h */	MTL_IMC_MAD_CHANNEL	MADC0,		/* 32 bits    */
	/* D808h */				MADC1;		/* 32 bits    */
	/* D80Ch */	MTL_IMC_MAD_DIMM	MADD0,		/* 32 bits    */
	/* D810h */				MADD1;		/* 32 bits    */
		} MTL;
		struct {
	/* 90h */	AMD_0F_DRAM_CONFIG_LOW	DCRL;		/* 32 bits    */
	/* 94h */	AMD_0F_DRAM_CONFIG_HIGH DCRH;		/* 32 bits    */
		} AMD0Fh;
	};

	union {
		struct {
	/* 64h */	NHM_IMC_MAX_DOD 	DOD;		/* 32 bits    */
		} NHM;
		struct {
	/* 80h */	AMD_0F_DRAM_CS_MAPPING	CS;		/* 32 bits    */
		} AMD0Fh;
	} MaxDIMMs;

	unsigned short		SlotCount, ChannelCount;
} MC_REGISTERS;

typedef struct
{
		struct {
			NHM_IMC_CLK_RATIO_STATUS DimmClock;
			QPI_FREQUENCY		QuickPath;
			BIOS_MEMCLOCK		BIOS_DDR;
		};
	union {
		struct {
			MCH_CLKCFG		ClkCfg;
			SNB_CAPID_A		SNB_Cap;
			IVB_CAPID_B		IVB_Cap;
		};
		struct {
			SNB_EP_CAPID0		SNB_EP_Cap0;
			SNB_EP_CAPID1		SNB_EP_Cap1;
			SNB_EP_CAPID2		SNB_EP_Cap2;
			SNB_EP_CAPID3		SNB_EP_Cap3;
			SNB_EP_CAPID4		SNB_EP_Cap4;
		};
		struct {
			SKL_CAPID_A		SKL_Cap_A;
			SKL_CAPID_B		SKL_Cap_B;
			SKL_CAPID_C		SKL_Cap_C;
			SKL_SA_PLL_RATIOS	SKL_SA_Pll;
		};
		struct {
			RKL_CAPID_A		RKL_Cap_A;
			RKL_CAPID_B		RKL_Cap_B;
			RKL_CAPID_C		RKL_Cap_C;
			RKL_SA_PERF_STATUS	RKL_SA_Pll;
		};
		struct {
			TGL_CAPID_A		TGL_Cap_A;
			TGL_CAPID_B		TGL_Cap_B;
			TGL_CAPID_C		TGL_Cap_C;
			TGL_CAPID_E		TGL_Cap_E;
			TGL_SA_PERF_STATUS	TGL_SA_Pll;
		};
		struct {
			ADL_CAPID_A		ADL_Cap_A;
			ADL_CAPID_B		ADL_Cap_B;
			ADL_CAPID_C		ADL_Cap_C;
			ADL_CAPID_E		ADL_Cap_E;
			ADL_SA_PERF_STATUS	ADL_SA_Pll;
		};
		struct {
			GLK_CAPID_A		GLK_Cap_A;
			GLK_CAPID_B		GLK_Cap_B;
		};
		struct {
			MTL_CAPID_A		MTL_Cap_A;
			MTL_CAPID_B		MTL_Cap_B;
			MTL_CAPID_C		MTL_Cap_C;
			MTL_CAPID_E		MTL_Cap_E;
			MTL_SA_PERF_STATUS	MTL_SA_Pll;
		};
		struct {
			AMD_0F_HTT_NODE_ID	NodeID;
			AMD_0F_HTT_UNIT_ID	UnitID;
			AMD_0F_HTT_FREQUENCY	LDTi_Freq[3];
		};
	};
	union {
		struct {
			AMD_IOMMU_CTRL_REG	IOMMU_CR;	/* 64 bits    */
			AMD_IOMMU_CAP_HEADER	IOMMU_HDR;	/* 32 bits    */
		};
		struct {
			Intel_IOMMU_CAP_REG	IOMMU_Cap;	/* 64 bits    */
			Intel_IOMMU_VER_REG	IOMMU_Ver;	/* 32 bits    */
		};
	};
} BUS_REGISTERS;


typedef struct {
		int		taskCount;
		TASK_MCB	taskList[TASK_LIMIT];

		unsigned int	kernelVersionNumber;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
} SYSGATE_RO; /*		RO Pages				*/

#define CHIP_MAX_PCI	24

typedef struct
{
	struct
	{
	    unsigned long long	PCLK; /* Contextual Clock [TSC|UCLK|MCLK] */
	  union {
	    struct {
	    unsigned long long	PC02, /* Goldmont, Sandy-Bridge, Phi	*/
				PC03, /* Goldmont, Nehalem, Sandy-Bridge, Phi */
				PC04, /* Atom [06_27h]			*/
				PC06, /* Goldmont, Nehalem, Sandy-Bridge, Phi */
				PC07, /* Nehalem, Sandy-Bridge, Phi	*/
				PC08, /* Haswell			*/
				PC09, /* Haswell			*/
				PC10; /* Goldmont, Haswell		*/
	    };
	    #if defined(ARCH_PMC)
		unsigned long long
			CTR[MC_VECTOR_TO_SCALAR(MC_MAX_CTRL, MC_MAX_CHA)];
	    #endif
	  };
	  union {
	    unsigned long long	MC6;  /* Atom, Silervmont: per Module	*/
	    unsigned long long	FCLK; /* AMD Family 17h, 18h, 19h	*/
	  };
	  struct {
	    unsigned long long	FC0; /* Uncore fixed counter #0 	*/
	  } Uncore;

	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Counter[2] __attribute__ ((aligned (8)));

	struct
	{
	    unsigned long long	PCLK;
	  union {
	    struct {
	    unsigned long long	PC02,
				PC03,
				PC04,
				PC06,
				PC07,
				PC08,
				PC09,
				PC10;
	    };
	    #if defined(ARCH_PMC)
		unsigned long long
			CTR[MC_VECTOR_TO_SCALAR(MC_MAX_CTRL, MC_MAX_CHA)];
	    #endif
	  };
	  union {
	    unsigned long long	MC6;
	    unsigned long long	FCLK;
	  };
	  struct {
	    unsigned long long	FC0;
	  } Uncore;
	} Delta __attribute__ ((aligned (8)));

	FEATURES		Features;

	BitCC			CR_Mask 	__attribute__ ((aligned (16)));
	BitCC			TM_Mask 	__attribute__ ((aligned (16)));
	BitCC			ODCM_Mask	__attribute__ ((aligned (16)));
	BitCC			DCU_Mask	__attribute__ ((aligned (16)));
	BitCC			L1_Scrub_Mask	__attribute__ ((aligned (16)));
	BitCC			L2_AMP_Mask	__attribute__ ((aligned (16)));
	BitCC			ECORE_Mask	__attribute__ ((aligned (16)));
	BitCC			PowerMgmt_Mask	__attribute__ ((aligned (16)));
	BitCC			SpeedStep_Mask	__attribute__ ((aligned (16)));
	BitCC			TurboBoost_Mask __attribute__ ((aligned (16)));
	BitCC			HWP_Mask __attribute__ ((aligned (16)));
	BitCC			C1E_Mask __attribute__ ((aligned (16)));
	BitCC	/* NHM */	C3A_Mask __attribute__ ((aligned (16)));
	BitCC	/* NHM */	C1A_Mask __attribute__ ((aligned (16)));
	BitCC	/* SNB */	C3U_Mask __attribute__ ((aligned (16)));
	BitCC	/* SNB */	C1U_Mask __attribute__ ((aligned (16)));
	BitCC	/* AMD */	CC6_Mask __attribute__ ((aligned (16)));
	BitCC	/* AMD */	PC6_Mask __attribute__ ((aligned (16)));
	BitCC			SPEC_CTRL_Mask	__attribute__ ((aligned (16)));
	BitCC			ARCH_CAP_Mask	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	BTC_NOBR_Mask	__attribute__ ((aligned (16)));
	BitCC			WDT_Mask	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	XPROC_LEAK_Mask __attribute__ ((aligned (16)));

	enum THERMAL_FORMULAS	thermalFormula;
	enum VOLTAGE_FORMULAS	voltageFormula;
	enum POWER_FORMULAS	powerFormula;

	unsigned int		SleepInterval,
				tickReset,
				tickStep;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	SERVICE_PROC		Service;

	signed int		ArchID;

	struct {
		unsigned int	Boost[UNCORE_BOOST(SIZE)];
		BUS_REGISTERS	Bus;
		MC_REGISTERS	MC[MC_MAX_CTRL];
		unsigned short	CtrlCount;
	    struct CHIP_ST {
		unsigned short	VID, DID;
	    } Chip[CHIP_MAX_PCI];
	} Uncore;

	struct {
		THERMAL_PARAM	Param;
		unsigned int	Sensor;
	    struct {
		signed int	CPU, SOC;
	    } VID;

	enum THERM_PWR_EVENTS	Events[eDIM];

		RAPL_POWER_UNIT Unit;

	    struct {
		PKGCST_IRTL	PC03,
				PC06,
				PC07;
	    } IRTL;

	  union {
	    struct {
	      struct {
/*64-bits*/	DOMAIN_POWER_LIMIT	PowerLimit;
/*16-bits*/	unsigned short		Unlock; /* Not Agent Locked */
	      } Domain[PWR_DOMAIN(SIZE)];

/*64-bits*/	DOMAIN_POWER_INFO	PowerInfo;
/*32-bits*/	struct {
			unsigned int	TDC	:  1-0,
					_Unused : 32-1;
		} Enable_Limit;
/*16-bits*/	unsigned short		EDC;
/*16-bits*/	unsigned short		TDC;
	    };
	    struct {
	      struct {
/*64-bits*/	unsigned long long	_pad64;
/*16-bits*/	unsigned short		_pad16;
	      } Domain[PWR_DOMAIN(SIZE)];

/*32-bits*/	AMD_17_MTS_MCM_PWR	PWR;
/*32-bits*/	AMD_17_MTS_MCM_TDP	TDP;
/*32-bits*/	AMD_17_MTS_MCM_EDC	EDC;
/*32-bits*/	unsigned int		_pad32;
	    } Zen;
	  };

	    struct {
		unsigned short		Minimum,
					Maximum;
	    } ACPI_CPPC;
	} PowerThermal;

	THERMAL_POINT		ThermalPoint;

	struct {
		struct {
			size_t	Size;
			int	Order;
		} ReqMem;
	} Gate;

	OS_DRIVER		OS;

	struct {
		Bit64		NMI;
		signed int	AutoClock,
				Experimental,
				HotPlug,
				PCI;
		KERNEL_DRIVER	Driver;
	} Registration;

	enum HYPERVISOR 	HypervisorID;
	char			Architecture[CODENAME_LEN];

	SMBIOS_ST		SMB;

	FOOTPRINT		FootPrint;
} PROC_RO; /*			RO Pages				*/

typedef struct
{
	struct
	{
	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Delta __attribute__ ((aligned (8)));

	BitCC			TM1		__attribute__ ((aligned (16)));
	BitCC			TM2		__attribute__ ((aligned (16)));
	BitCC			ODCM		__attribute__ ((aligned (16)));
	BitCC			L1_HW_Prefetch	__attribute__ ((aligned (16)));
	BitCC			L1_HW_IP_Prefetch __attribute__((aligned (16)));
	BitCC	/* Intel */	L1_NPP_Prefetch __attribute__ ((aligned (16)));
	BitCC	/* Intel */	L1_Scrubbing	__attribute__ ((aligned (16)));
	BitCC			L2_HW_Prefetch	__attribute__ ((aligned (16)));
	BitCC			L2_HW_CL_Prefetch __attribute__((aligned (16)));
	BitCC	/* Intel */	L2_AMP_Prefetch __attribute__ ((aligned (16)));
	BitCC	/* Intel */	L2_NLP_Prefetch __attribute__ ((aligned (16)));
	BitCC	/* AMD */	L1_Stride_Pf	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	L1_Region_Pf	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	L1_Burst_Pf	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	L2_Stream_HW_Pf __attribute__ ((aligned (16)));
	BitCC	/* AMD */	L2_UpDown_Pf	__attribute__ ((aligned (16)));
	BitCC	/* Intel */	LLC_Streamer	__attribute__ ((aligned (16)));
	BitCC			PowerMgmt	__attribute__ ((aligned (16)));
	BitCC			SpeedStep	__attribute__ ((aligned (16)));
	BitCC			TurboBoost	__attribute__ ((aligned (16)));
	BitCC			HWP		__attribute__ ((aligned (16)));
	BitCC			C1E		__attribute__ ((aligned (16)));
	BitCC			C3A		__attribute__ ((aligned (16)));
	BitCC			C1A		__attribute__ ((aligned (16)));
	BitCC			C3U		__attribute__ ((aligned (16)));
	BitCC			C1U		__attribute__ ((aligned (16)));
	BitCC			CC6		__attribute__ ((aligned (16)));
	BitCC			PC6		__attribute__ ((aligned (16)));
	BitCC			SMM		__attribute__ ((aligned (16)));
	BitCC			VM		__attribute__ ((aligned (16)));
	BitCC			IBRS		__attribute__ ((aligned (16)));
	BitCC			STIBP		__attribute__ ((aligned (16)));
	BitCC			SSBD		__attribute__ ((aligned (16)));
	BitCC			PSFD		__attribute__ ((aligned (16)));
	BitCC			RDCL_NO 	__attribute__ ((aligned (16)));
	BitCC			IBRS_ALL	__attribute__ ((aligned (16)));
	BitCC			RSBA		__attribute__ ((aligned (16)));
	BitCC			L1DFL_VMENTRY_NO __attribute__ ((aligned (16)));
	BitCC			SSB_NO		__attribute__ ((aligned (16)));
	BitCC			MDS_NO		__attribute__ ((aligned (16)));
	BitCC			PSCHANGE_MC_NO	__attribute__ ((aligned (16)));
	BitCC			TAA_NO		__attribute__ ((aligned (16)));
	BitCC			STLB		__attribute__ ((aligned (16)));
	BitCC			FUSA		__attribute__ ((aligned (16)));
	BitCC			RSM_CPL0	__attribute__ ((aligned (16)));
	BitCC			SPLA		__attribute__ ((aligned (16)));
	BitCC			SNOOP_FILTER	__attribute__ ((aligned (16)));
	BitCC			WDT		__attribute__ ((aligned (16)));
	BitCC			AMD_LS_CFG_SSBD __attribute__ ((aligned (16)));
	BitCC			DOITM_EN	__attribute__ ((aligned (16)));
	BitCC			DOITM_MSR	__attribute__ ((aligned (16)));
	BitCC			SBDR_SSDP_NO	__attribute__ ((aligned (16)));
	BitCC			FBSDP_NO	__attribute__ ((aligned (16)));
	BitCC			PSDP_NO 	__attribute__ ((aligned (16)));
	BitCC			FB_CLEAR	__attribute__ ((aligned (16)));
	BitCC			SRBDS_MSR	__attribute__ ((aligned (16)));
	BitCC			RNGDS		__attribute__ ((aligned (16)));
	BitCC			RTM		__attribute__ ((aligned (16)));
	BitCC			VERW		__attribute__ ((aligned (16)));
	BitCC			RRSBA		__attribute__ ((aligned (16)));
	BitCC			BHI_NO		__attribute__ ((aligned (16)));
	BitCC			XAPIC_MSR	__attribute__ ((aligned (16)));
	BitCC			XAPIC_DIS	__attribute__ ((aligned (16)));
	BitCC			PBRSB_NO	__attribute__ ((aligned (16)));
	BitCC			IPRED_DIS_U	__attribute__ ((aligned (16)));
	BitCC			IPRED_DIS_S	__attribute__ ((aligned (16)));
	BitCC			RRSBA_DIS_U	__attribute__ ((aligned (16)));
	BitCC			RRSBA_DIS_S	__attribute__ ((aligned (16)));
	BitCC			DDPD_U_DIS	__attribute__ ((aligned (16)));
	BitCC			BHI_DIS_S	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	BTC_NOBR	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	XPROC_LEAK	__attribute__ ((aligned (16)));
	BitCC	/* AMD */	AGENPICK	__attribute__ ((aligned (16)));
	BitCC	/* Intel */	OC_UTILIZED	__attribute__ ((aligned (16)));
	BitCC	/* Intel */	OC_UNDERVOLT	__attribute__ ((aligned (16)));
	BitCC	/* Intel */	OC_UNLOCKED	__attribute__ ((aligned (16)));
	BitCC	/* Intel */	GDS_NO		__attribute__ ((aligned (16)));
	BitCC	/* Intel */	RFDS_NO 	__attribute__ ((aligned (16)));

	struct {
		Bit64		Signal	__attribute__ ((aligned (8)));
	} OS;
} PROC_RW; /*			RW Pages				*/


#ifndef PCI_VENDOR_ID_INTEL
	#define PCI_VENDOR_ID_INTEL	0x8086
#endif

#ifndef PCI_VENDOR_ID_AMD
	#define PCI_VENDOR_ID_AMD	0x1022
#endif

#define DID_INTEL_82945P_HB		0x2770
#define DID_INTEL_82945GM_HB		0x27a0
#define DID_INTEL_82955_HB		0x2774
/* Source: /drivers/char/agp/intel-agp.h				*/
#define DID_INTEL_82945GME_HB		0x27ac
#define DID_INTEL_82946GZ_HB		0x2970
#define DID_INTEL_82965Q_HB		0x2990
#define DID_INTEL_82965G_HB		0x29a0
#define DID_INTEL_82965GM_HB		0x2a00
#define DID_INTEL_82965GME_HB		0x2a10
#define DID_INTEL_GM45_HB		0x2a40
#define DID_INTEL_Q35_HB		0x29b0
#define DID_INTEL_G33_HB		0x29c0
#define DID_INTEL_Q33_HB		0x29d0
/* Source: /drivers/edac/x38_edac.c					*/
#define DID_INTEL_X38_HB		0x29e0
/* Source: /drivers/edac/i3200_edac.c					*/
#define DID_INTEL_3200_HB		0x29f0
/* Source: /drivers/char/agp/intel-agp.h				*/
#define DID_INTEL_Q45_HB		0x2e10
#define DID_INTEL_G45_HB		0x2e20
#define DID_INTEL_G41_HB		0x2e30
/* Source: Intel Atom Processor N400 & N500 Series Datasheet– Volume 2	*/
#define DID_INTEL_BONNELL_HB		0xa010
/* Source: Intel Atom Processor D2000 & N2000 Series Datasheet– Volume 2 */
#define DID_INTEL_SALTWELL_HB		0x0bf1
/* Source: SoC / Silvermont / Processor Transaction Router		*/
#define DID_INTEL_SLM_PTR		0x0f00
/* Source: [Atom/Airmont] Pentium(R) CPU N3700				*/
#define DID_INTEL_AIRMONT_HB		0x2280
/* Source: Intel(R) Celeron(R) CPU J3455 @ 1.50GHz			*/
#define DID_INTEL_GOLDMONT_HB		0x5af0
/* Source: /include/linux/pci_ids.h					*/
#define DID_INTEL_I7_MCR		0x2c18
#define DID_INTEL_I7_MC_CH0_CTRL	0x2c20
#define DID_INTEL_I7_MC_CH1_CTRL	0x2c28
#define DID_INTEL_I7_MC_CH2_CTRL	0x2c30
#define DID_INTEL_I7_MC_TEST		0x2c1c
#define DID_INTEL_I7_MC_CH0_ADDR	0x2c21
#define DID_INTEL_I7_MC_CH1_ADDR	0x2c29
#define DID_INTEL_I7_MC_CH2_ADDR	0x2c31
#define DID_INTEL_BLOOMFIELD_NON_CORE	0x2c41
#define DID_INTEL_C5500_NON_CORE	0x2c58
#define DID_INTEL_LYNNFIELD_NON_CORE	0x2c51
#define DID_INTEL_CLARKSFIELD_NON_CORE	0x2c52
#define DID_INTEL_CLARKDALE_NON_CORE	0x2c61
#define DID_INTEL_LYNNFIELD_MCR 	0x2c98
#define DID_INTEL_LYNNFIELD_MC_CH0_CTRL 0x2ca0
#define DID_INTEL_LYNNFIELD_MC_CH1_CTRL 0x2ca8
#define DID_INTEL_LYNNFIELD_MC_TEST	0x2c9c
#define DID_INTEL_LYNNFIELD_MC_CH0_ADDR 0x2ca1
#define DID_INTEL_LYNNFIELD_MC_CH1_ADDR 0x2ca9
#define DID_INTEL_NHM_EC_MCR		0x2cd8
#define DID_INTEL_NHM_EC_MC_CH0_CTRL	0x2ce0
#define DID_INTEL_NHM_EC_MC_CH1_CTRL	0x2ce8
#define DID_INTEL_NHM_EC_MC_CH2_CTRL	0x2cf0
#define DID_INTEL_NHM_EC_MC_TEST	0x2cdc
#define DID_INTEL_NHM_EC_MC_CH0_ADDR	0x2ce1
#define DID_INTEL_NHM_EC_MC_CH1_ADDR	0x2ce9
#define DID_INTEL_NHM_EC_MC_CH2_ADDR	0x2cf1
#define DID_INTEL_NHM_EP_MCR		0x2d98
#define DID_INTEL_NHM_EP_MC_CH0_CTRL	0x2da0
#define DID_INTEL_NHM_EP_MC_CH1_CTRL	0x2da8
#define DID_INTEL_NHM_EP_MC_CH2_CTRL	0x2db0
#define DID_INTEL_NHM_EP_MC_TEST	0x2d9c
#define DID_INTEL_NHM_EP_MC_CH0_ADDR	0x2da1
#define DID_INTEL_NHM_EP_MC_CH1_ADDR	0x2da9
#define DID_INTEL_NHM_EP_MC_CH2_ADDR	0x2db1
#define DID_INTEL_NHM_EP_NON_CORE	0x2c70
/* Source: Intel Using the Intel ICH Family Watchdog Timer (WDT)	*/
#define DID_INTEL_ICH10_LPC		0x3a16
/* Source: Intel X58 Express Chipset Datasheet				*/
#define DID_INTEL_X58_HUB_CORE		0x342e
#define DID_INTEL_X58_HUB_CTRL		0x3423
#define DID_INTEL_IIO_CORE_REG		0x3728
/* Source: Intel Corporation Elkhart Lake SMBus and Jasper Lake SMBus	*/
#define DID_INTEL_EHL_SMBUS		0x4b23
#define DID_INTEL_JSL_SMBUS		0x4da3
/* Source: /include/linux/pci_ids.h					*/
#define DID_INTEL_SNB_IMC_HA0		0x3ca0
/* Source: 2nd Generation Intel Core Processor Family Vol2		*/
#define DID_INTEL_SNB_IMC_SA		0x0100
#define DID_INTEL_SNB_IMC_0104		0x0104
/* Source: /drivers/edac/sb_edac.c					*/
/*TODO( Harware missing )*/
#define DID_INTEL_IVB_IMC_HA0		0x0ea0
#define DID_INTEL_IVB_IMC_HA1		0x0e60
/* Source: 3rd Generation Intel Core Processor Family Vol2		*/
#define DID_INTEL_IVB_IMC_SA		0x0150
#define DID_INTEL_IVB_IMC_0154		0x0154
/* Source: Intel Xeon Processor E5 & E7 v1 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#define DID_INTEL_SNB_EP_HOST_BRIDGE	0x3c00
/*	QPIMISCSTAT: Device=8,9 - Function=0				*/
#define DID_INTEL_SNB_EP_QPI_LINK0	0x3c80
#define DID_INTEL_SNB_EP_QPI_LINK1	0x3c90
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=15 - Function=0				*/
#define DID_INTEL_SNB_EP_IMC_CTRL0_CPGC 0x3ca8
/*TODO( Nehalem/Xeon E7 - CPGC: Device=?? - Function=? )
#define DID_INTEL_SNB_EP_IMC_CTRL1_CPGC 0x0				*/
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=16 - Function=0,1,2,3			*/
#define DID_INTEL_SNB_EP_IMC_CTRL0_CH0	0x3cb0
#define DID_INTEL_SNB_EP_IMC_CTRL0_CH1	0x3cb1
#define DID_INTEL_SNB_EP_IMC_CTRL0_CH2	0x3cb2
#define DID_INTEL_SNB_EP_IMC_CTRL0_CH3	0x3cb3
/*	Controller #1: Device=16 - Function=4,5,6,7			*/
#define DID_INTEL_SNB_EP_IMC_CTRL1_CH0	0x3cb4
#define DID_INTEL_SNB_EP_IMC_CTRL1_CH1	0x3cb5
#define DID_INTEL_SNB_EP_IMC_CTRL1_CH2	0x3cb6
#define DID_INTEL_SNB_EP_IMC_CTRL1_CH3	0x3cb7
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=15 - Function=2,3,4,5,6	*/
#define DID_INTEL_SNB_EP_TAD_CTRL0_CH0	0x3caa
#define DID_INTEL_SNB_EP_TAD_CTRL0_CH1	0x3cab
#define DID_INTEL_SNB_EP_TAD_CTRL0_CH2	0x3cac
#define DID_INTEL_SNB_EP_TAD_CTRL0_CH3	0x3cad
#define DID_INTEL_SNB_EP_TAD_CTRL0_CH4	0x3cae
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*TODO( Nehalem/Xeon E7 - TAD Controller #1: Device=?? - Function=? )
#define DID_INTEL_SNB_EP_TAD_CTRL1_CH0	0x0
#define DID_INTEL_SNB_EP_TAD_CTRL1_CH1	0x0
#define DID_INTEL_SNB_EP_TAD_CTRL1_CH2	0x0
#define DID_INTEL_SNB_EP_TAD_CTRL1_CH3	0x0
#define DID_INTEL_SNB_EP_TAD_CTRL1_CH4	0x0				*/
/*	Power Control Unit						*/
/*	PCU: Device=10 - Function=3					*/
#define DID_INTEL_SNB_EP_CAPABILITY	0x3cd0
/* Source: Intel Xeon Processor E5 & E7 v2 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#define DID_INTEL_IVB_EP_HOST_BRIDGE	0x0e00
/*	IIO - Bus0: 0x0, Bus1:0x40 - Device=5 - Function=0		*/
#define DID_INTEL_IVB_EP_IIO_VTD	0x0e28
/*	QPIMISCSTAT: Device={8,9},10 - Function=0			*/
#define DID_INTEL_IVB_EP_QPI_LINK0	0x0e80
#define DID_INTEL_IVB_EP_QPI_LINK1	0x0e90
#define DID_INTEL_IVB_EP_QPI_LINK2	0x0e40
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=15 - Function=0				*/
#define DID_INTEL_IVB_EP_IMC_CTRL0_CPGC 0x0ea8
/*	Xeon E7 - CPGC: Device=29 - Function=0				*/
#define DID_INTEL_IVB_EP_IMC_CTRL1_CPGC 0x0e68
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=16 - Function=0,1,2,3			*/
#define DID_INTEL_IVB_EP_IMC_CTRL0_CH0	0x0eb0
#define DID_INTEL_IVB_EP_IMC_CTRL0_CH1	0x0eb1
#define DID_INTEL_IVB_EP_IMC_CTRL0_CH2	0x0eb2
#define DID_INTEL_IVB_EP_IMC_CTRL0_CH3	0x0eb3
/*	Controller #1: Device=16 - Function=4,5,6,7			*/
#define DID_INTEL_IVB_EP_IMC_CTRL1_CH0	0x0eb4
#define DID_INTEL_IVB_EP_IMC_CTRL1_CH1	0x0eb5
#define DID_INTEL_IVB_EP_IMC_CTRL1_CH2	0x0eb6
#define DID_INTEL_IVB_EP_IMC_CTRL1_CH3	0x0eb7
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=15 - Function=2,3,4,5	*/
#define DID_INTEL_IVB_EP_TAD_CTRL0_CH0	0x0eaa
#define DID_INTEL_IVB_EP_TAD_CTRL0_CH1	0x0eab
#define DID_INTEL_IVB_EP_TAD_CTRL0_CH2	0x0eac
#define DID_INTEL_IVB_EP_TAD_CTRL0_CH3	0x0ead
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*	Xeon E7 - TAD Controller #1: Device=29 - Function=2,3,4,5	*/
#define DID_INTEL_IVB_EP_TAD_CTRL1_CH0	0x0e6a
#define DID_INTEL_IVB_EP_TAD_CTRL1_CH1	0x0e6b
#define DID_INTEL_IVB_EP_TAD_CTRL1_CH2	0x0e6c
#define DID_INTEL_IVB_EP_TAD_CTRL1_CH3	0x0e6d
/*	Power Control Unit						*/
/*	PCU: Device=10 - Function=3					*/
#define DID_INTEL_IVB_EP_CAPABILITY	0x0ec3
/* Source: Intel 7 Series / C216 Chipset Family Platform Controller Hub */
#define DID_INTEL_PCH_C216_LPC		0x1e44
/* Source: Intel Xeon Processor E5 & E7 v3 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#define DID_INTEL_HSW_EP_HOST_BRIDGE	0x2f00
/*	QPIMISCSTAT: Device={8,9},10 - Function=0			*/
#define DID_INTEL_HSW_EP_QPI_LINK0	0x2f80
#define DID_INTEL_HSW_EP_QPI_LINK1	0x2f90
#define DID_INTEL_HSW_EP_QPI_LINK2	0x2f40
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E7 - CPGC: Device=19 - Function=0,1			*/
#define DID_INTEL_HSW_E7_IMC_CTRL0_F0_CPGC 0x2fa8
#define DID_INTEL_HSW_E7_IMC_CTRL0_F1_CPGC 0x2f71
/*	Xeon E7 - CPGC: Device=22 - Function=0,1			*/
#define DID_INTEL_HSW_E7_IMC_CTRL1_F0_CPGC 0x2f68
#define DID_INTEL_HSW_E7_IMC_CTRL1_F1_CPGC 0x2f79
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=20,21 - Function=0,1			*/
#define DID_INTEL_HSW_EP_IMC_CTRL0_CH0	0x2fb4
#define DID_INTEL_HSW_EP_IMC_CTRL0_CH1	0x2fb5
#define DID_INTEL_HSW_EP_IMC_CTRL0_CH2	0x2fb0
#define DID_INTEL_HSW_EP_IMC_CTRL0_CH3	0x2fb1
/*	Controller #1: Device=23,24 - Function=2,3			*/
#define DID_INTEL_HSW_EP_IMC_CTRL1_CH0	0x2fd6
#define DID_INTEL_HSW_EP_IMC_CTRL1_CH1	0x2fd7
#define DID_INTEL_HSW_EP_IMC_CTRL1_CH2	0x2fd2
#define DID_INTEL_HSW_EP_IMC_CTRL1_CH3	0x2fd3
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=19 - Function=2,3,4,5	*/
#define DID_INTEL_HSW_EP_TAD_CTRL0_CH0	0x2faa
#define DID_INTEL_HSW_EP_TAD_CTRL0_CH1	0x2fab
#define DID_INTEL_HSW_EP_TAD_CTRL0_CH2	0x2fac
#define DID_INTEL_HSW_EP_TAD_CTRL0_CH3	0x2fad
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*	Xeon E7 - TAD Controller #1: Device=22 - Function=2,3,4,5	*/
#define DID_INTEL_HSW_EP_TAD_CTRL1_CH0	0x2f6a
#define DID_INTEL_HSW_EP_TAD_CTRL1_CH1	0x2f6b
#define DID_INTEL_HSW_EP_TAD_CTRL1_CH2	0x2f6c
#define DID_INTEL_HSW_EP_TAD_CTRL1_CH3	0x2f6d
/*	Power Control Unit						*/
/*	PCU: Device=30 - Function=3					*/
#define DID_INTEL_HSW_EP_CAPABILITY	0x2fc0
/* Source: 4th, 5th Generation Intel Core Processor Family Vol2 Chap. 3.0 */
#define DID_INTEL_HASWELL_IMC_HA0	0x2fa0
#define DID_INTEL_HASWELL_IMC_SA	0x0c00
#define DID_INTEL_HASWELL_MH_IMC_HA0	0x0c04
#define DID_INTEL_HASWELL_UY_IMC_HA0	0x0a04
#define DID_INTEL_BROADWELL_IMC_HA0	0x1604
#define DID_INTEL_BROADWELL_D_IMC_HA0	0x1610
#define DID_INTEL_BROADWELL_H_IMC_HA0	0x1614
#define DID_INTEL_BROADWELL_U_IMC_HA0	0x1618
/* Source: 6th Generation Intel Processor Datasheet for U/Y-Platforms Vol2 */
#define DID_INTEL_SKYLAKE_U_IMC_HA	0x1904
#define DID_INTEL_SKYLAKE_Y_IMC_HA	0x190c
/* Source: 6th Generation Intel Processor Datasheet for S-Platforms Vol2 */
#define DID_INTEL_SKYLAKE_S_IMC_HAD	0x190f
#define DID_INTEL_SKYLAKE_S_IMC_HAQ	0x191f
/* Source: 6th Generation Intel Processor Datasheet for H-Platforms Vol2 */
#define DID_INTEL_SKYLAKE_H_IMC_HAD	0x1900
#define DID_INTEL_SKYLAKE_H_IMC_HAQ	0x1910
/* Source: Intel Xeon Processor E3-1200 v5 Product Family		*/
#define DID_INTEL_SKYLAKE_DT_IMC_HA	0x1918
/* Source: Intel C620 Series Chipset Platform Controller Hub Datasheet	*/
#define DID_INTEL_C620_PCH_SMBUS	0xa1a3
#define DID_INTEL_C620_SUPER_SMBUS	0xa223
/* Source:7th Generation Intel Processor for S-Platforms & Core X-Series Vol2*/
#define DID_INTEL_KABYLAKE_H_IMC_HAD	0x5900
#define DID_INTEL_KABYLAKE_U_IMC_HA	0x5904
#define DID_INTEL_KABYLAKE_Y_IMC_HA	0x590c
#define DID_INTEL_KABYLAKE_Y_IMC_HQ	0x590d
#define DID_INTEL_KABYLAKE_S_IMC_HAD	0x590f
#define DID_INTEL_KABYLAKE_H_IMC_HAQ	0x5910
#define DID_INTEL_KABYLAKE_DT_IMC_HA	0x5918
#define DID_INTEL_KABYLAKE_U_IMC_HAQ	0x5914
#define DID_INTEL_KABYLAKE_S_IMC_HAQ	0x591f
#define DID_INTEL_KABYLAKE_X_IMC_HAQ	0x5906
#define DID_INTEL_KBL_PCH_H_SMBUS	0xa2a3
/* Source: Hewlett-Packard Company - Sunrise Point-LP SMBus		*/
#define DID_INTEL_SPT_LP_SMBUS		0x9d23
#define DID_INTEL_SPT_H_PCH_SMBUS	0xa123
/* Source: 8th Generation Intel Processor for S-Platforms Datasheet Vol2 */
#define DID_INTEL_COFFEELAKE_S_IMC_HAQ	0x3e1f
#define DID_INTEL_COFFEELAKE_S_IMC_HAS	0x3ec2
/* Source: 8th and 9th Generation Intel Core and Xeon E Processor Families */
#define DID_INTEL_COFFEELAKE_R_S_IMC_HAD 0x3e0f
#define DID_INTEL_COFFEELAKE_R_U_IMC_HAD 0x3ecc
#define DID_INTEL_COFFEELAKE_R_U_IMC_HAQ 0x3ed0
#define DID_INTEL_COFFEELAKE_R_H_IMC_HAQ 0x3e10
#define DID_INTEL_COFFEELAKE_R_H_IMC_HAS 0x3ec4
#define DID_INTEL_COFFEELAKE_R_H_IMC_HAO 0x3e30
#define DID_INTEL_COFFEELAKE_R_W_IMC_HAQ 0x3e18
#define DID_INTEL_COFFEELAKE_R_W_IMC_HAS 0x3ec6
#define DID_INTEL_COFFEELAKE_R_W_IMC_HAO 0x3e31
#define DID_INTEL_COFFEELAKE_R_S_IMC_HAQ 0x3e33
#define DID_INTEL_COFFEELAKE_R_S_IMC_HAS 0x3eca
#define DID_INTEL_COFFEELAKE_R_S_IMC_HAO 0x3e32
/* Source: 8th Generation Intel Core Processor Families Datasheet Vol2	*/
#define DID_INTEL_WHISKEYLAKE_U_IMC_HAD 0x3e35
#define DID_INTEL_WHISKEYLAKE_U_IMC_HAQ 0x3e34
#define DID_INTEL_CANNONLAKE_U_IMC_HB	0x5a04
#define DID_INTEL_CNL_PCH_LP_SMBUS	0x9da3
#define DID_INTEL_CNL_PCH_SMBUS 	0xa323
/* Source: Comet Lake PCH-LP SMBus and PCH-V SMBus Host Controllers	*/
#define DID_INTEL_CML_PCH_LP_SMBUS	0x02a3
#define DID_INTEL_CML_PCH_V_SMBUS	0xa3a3
#define DID_INTEL_CML_H_PCH_SMBUS	0x06a3
/* Source: Intel 400 Series Chipset Family On-Package Platform Controller Hub */
#define DID_INTEL_COMETLAKE_S_IMC_6C	0x9b53
#define DID_INTEL_COMETLAKE_S_IMC_10C	0x9b54
#define DID_INTEL_COMETLAKE_H_IMC_10C	0x9b63
#define DID_INTEL_COMETLAKE_W_IMC_10C	0x9b64
#define DID_INTEL_COMETLAKE_M_IMC_6C	0x9b44
#define DID_INTEL_COMETLAKE_U_IMC_HB	0x9b61
#define DID_INTEL_COMETLAKE_PREM_U_PCH	0x0284
#define DID_INTEL_COMETLAKE_BASE_U_PCH	0x0285
#define DID_INTEL_COMETLAKE_U_ES_PCH	0x3481
#define DID_INTEL_COMETLAKE_Y_ES_PCH	0x3486
#define DID_INTEL_COMETLAKE_Y_PCH	0x3487
#define DID_INTEL_COMETLAKE_H470_PCH	0x0684
#define DID_INTEL_COMETLAKE_Z490_PCH	0x0685
#define DID_INTEL_COMETLAKE_Q470_PCH	0x0687
#define DID_INTEL_COMETLAKE_HM470_PCH	0x068d
#define DID_INTEL_COMETLAKE_QM480_PCH	0x068c
#define DID_INTEL_COMETLAKE_WM490_PCH	0x068e
#define DID_INTEL_COMETLAKE_W480_PCH	0x0697
#define DID_INTEL_ICELAKE_U_4C		0x8a12
#define DID_INTEL_ICELAKE_U_PCH 	0x3482
#define DID_INTEL_ICL_LP_SMBUS		0x34a3
/* Source: Apple MacBook Air/Pro - Intel Core i5-1038NG7		*/
#define DID_INTEL_ICL_PCH_NG_SMBUS	0x38a3
/* Source: Linux: arch/x86/events/intel/uncore_snb.c			*/
#define DID_INTEL_COMETLAKE_U1_IMC	0x9b51
#define DID_INTEL_COMETLAKE_U3_IMC	0x9b71
#define DID_INTEL_COMETLAKE_S1_IMC	0x9b33
#define DID_INTEL_COMETLAKE_S2_IMC	0x9b43
#define DID_INTEL_COMETLAKE_S5_IMC	0x9b73
#define DID_INTEL_ICELAKE_U_IMC 	0x8a02
#define DID_INTEL_TIGERLAKE_U1_IMC	0x9a02
#define DID_INTEL_TIGERLAKE_U2_IMC	0x9a04
#define DID_INTEL_TIGERLAKE_U3_IMC	0x9a12
#define DID_INTEL_TIGERLAKE_U4_IMC	0x9a14
#define DID_INTEL_TIGERLAKE_H_IMC	0x9a36
/* Source: 11th Generation Intel Core Processor Desktop Datasheet Vol1	*/
#define DID_INTEL_TIGERLAKE_UP3_IMC	0xa082
#define DID_INTEL_TIGERLAKE_UP4_IMC	0xa087
#define DID_INTEL_TGL_PCH_LP_SMBUS 	0xa0a3 /* TGL-LP Watchdog	*/
#define DID_INTEL_TGL_H_PCH_SMBUS	0x43a3
#define DID_INTEL_ROCKETLAKE_S_8C_IMC_HB 0x4c43
#define DID_INTEL_ROCKETLAKE_S_6C_IMC_HB 0x4c53
/* Source: Intel 500 Series Chipset Family Platform Controller Hub	*/
#define DID_INTEL_ROCKETLAKE_H510_PCH	0x4388
#define DID_INTEL_ROCKETLAKE_B560_PCH	0x4387
#define DID_INTEL_ROCKETLAKE_H570_PCH	0x4386
#define DID_INTEL_ROCKETLAKE_Q570_PCH	0x4384
#define DID_INTEL_ROCKETLAKE_Z590_PCH	0x4385
#define DID_INTEL_ROCKETLAKE_W580_PCH	0x438f
/* Source: 12th Generation Intel Core Processors datasheet, vol 1	*/
#define DID_INTEL_ALDERLAKE_S_8P_8E_HB	0x4660
#define DID_INTEL_ALDERLAKE_S_8P_4E_HB	0x4668
#define DID_INTEL_ALDERLAKE_S_6P_4E_HB	0x4648	/* RAPTORLAKE_S_6P_4E	*/
#define DID_INTEL_ALDERLAKE_S_6P_0E_HB	0x4650
#define DID_INTEL_ALDERLAKE_S_4P_0E_HB	0x4630
#define DID_INTEL_ALDERLAKE_S_2P_0E_HB	0x4610
#define DID_INTEL_ALDERLAKE_H_6P_8E_HB	0x4641
#define DID_INTEL_ALDERLAKE_H_6P_4E_HB	0x4649
#define DID_INTEL_ALDERLAKE_H_4P_8E_HB	0x4621
#define DID_INTEL_ALDERLAKE_H_4P_4E_HB	0x4629
#define DID_INTEL_ALDERLAKE_HL_6P_8E_HB 0x4643
#define DID_INTEL_ALDERLAKE_U_2P_8E_HB	0x4601
#define DID_INTEL_ALDERLAKE_U_2P_4E_HB	0x4609
#define DID_INTEL_ALDERLAKE_U_1P_4E_HB	0x4619
/* Source: Intel 600 Series Chipset Family Platform Controller Hub	*/
#define DID_INTEL_ALDERLAKE_H610_PCH	0x7a87
#define DID_INTEL_ALDERLAKE_B660_PCH	0x7a86
#define DID_INTEL_ALDERLAKE_H670_PCH	0x7a85
#define DID_INTEL_ALDERLAKE_Z690_PCH	0x7a84
#define DID_INTEL_ALDERLAKE_Q670_PCH	0x7a83
#define DID_INTEL_ALDERLAKE_W680_PCH	0x7a88
#define DID_INTEL_ALDERLAKE_WM690_PCH	0x7a8d
#define DID_INTEL_ALDERLAKE_HM670_PCH	0x7a8c
/* Source: 12th Gen Intel Core i5-12500H;i7-12700H;i7-12800HL;i9-12900H */
#define DID_INTEL_ALDERLAKE_PCH_P	0x5182 /* PCH eSPI Controller	*/
#define DID_INTEL_ALDERLAKE_PCH_U	0x5188 /* PCH-U eSPI Controller */
#define DID_INTEL_ADL_PCH_P_SMBUS	0x51a3 /* ADL PCH-P Watchdog	*/
#define DID_INTEL_ADL_PCH_M_SMBUS	0x54a3 /* ADL PCH-M Watchdog	*/
#define DID_INTEL_ADL_S_PCH_SMBUS	0x7aa3
/* Source: Intel Atom x7000 / Intel Core i3 N-Series Datasheet, vol 1	*/
#define DID_INTEL_ALDERLAKE_N300_8E_HB	0x4617
#define DID_INTEL_ALDERLAKE_N200_4E_HB	0x461b	/*	SKU_3		*/
#define DID_INTEL_ALDERLAKE_N100_4E_HB	0x461c
#define DID_INTEL_ALDERLAKE_X7835RE_8C_HB 0x4675 /*	SKU_11_14	*/
#define DID_INTEL_ALDERLAKE_X7433RE_4C_HB 0x4674 /*	SKU_10_13	*/
#define DID_INTEL_ALDERLAKE_N97_4E_HB	0x4678	/*	SKU_2		*/
#define DID_INTEL_ALDERLAKE_X7425E_4C_HB 0x4679 /*	SKU_6		*/
#define DID_INTEL_ALDERLAKE_N50_2E_HB	0x4614	/*	SKU_1		*/
#define DID_INTEL_ALDERLAKE_X7213RE_2C_HB 0x4632 /*	SKU_9		*/
#define DID_INTEL_ALDERLAKE_X7211RE_2C_HB 0x4673 /*	SKU_8_12	*/
#define DID_INTEL_ALDERLAKE_X7213E_2C_HB 0x4677 /*	SKU_7		*/
#define DID_INTEL_ALDERLAKE_X7211E_2C_HB 0x467c /*	SKU_5		*/
#define DID_INTEL_ALDERLAKE_N305_PCH	0x5481	/*	SKU_4		*/
#define DID_INTEL_ALDERLAKE_N95_PCH	0x5482
#define DID_INTEL_ALDERLAKE_X7000E_PCH	0x5489	/*	SKU_5_7 	*/
#define DID_INTEL_ALDERLAKE_X7000RE_PCH 0x548a	/*	SKU_8_14	*/
/* Source: 13th Generation Intel Core Processors Datasheet, vol 1	*/
#define DID_INTEL_RAPTORLAKE_S_8P_16E_HB 0xa700
#define DID_INTEL_RAPTORLAKE_S_8P_8E_HB 0xa703
#define DID_INTEL_RAPTORLAKE_S_6P_8E_HB1 0xa704
#define DID_INTEL_RAPTORLAKE_S_6P_4E_HB 0xa705
#define DID_INTEL_RAPTORLAKE_S_6P_8E_HB2 0x4640
#define DID_INTEL_RAPTORLAKE_S_4P_0E_HB 0x4692	/* ALDERLAKE_S_6P_0E	*/
#define DID_INTEL_RAPTORLAKE_HX_8P_16E_HB 0xa702
#define DID_INTEL_RAPTORLAKE_HX_8P_12E_HB 0xa729
#define DID_INTEL_RAPTORLAKE_HX_8P_8E_HB1 0xa728
#define DID_INTEL_RAPTORLAKE_HX_6P_8E_HB1 0xa72a
#define DID_INTEL_RAPTORLAKE_HX_6P_4E_HB1 0xa719
#define DID_INTEL_RAPTORLAKE_HX_8P_8E_HB2 0x4637
#define DID_INTEL_RAPTORLAKE_HX_6P_8E_HB2 0x463b
#define DID_INTEL_RAPTORLAKE_HX_6P_4E_HB2 0x4647
#define DID_INTEL_RAPTORLAKE_H_6P_8E_HB 0xa706
#define DID_INTEL_RAPTORLAKE_H_4P_8E_HB 0xa707
#define DID_INTEL_RAPTORLAKE_H_4P_4E_HB 0xa716
#define DID_INTEL_RAPTORLAKE_U_2P_8E_HB 0xa708
#define DID_INTEL_RAPTORLAKE_U_2P_4E_HB 0xa71b
#define DID_INTEL_RAPTORLAKE_U_1P_4E_HB 0xa71c
#define DID_INTEL_RAPTORLAKE_E_8P_0E_HB 0xa711
#define DID_INTEL_RAPTORLAKE_E_6P_0E_HB 0xa712
#define DID_INTEL_RAPTORLAKE_E_4P_0E_HB 0xa713
/* Source: Intel(R) Core(TM) i7-14700K					*/
#define DID_INTEL_RAPTORLAKE_S_8P_12E_HB 0xa740
/* Source: PCH Controller Device IDs—Intel 700 Series Chipset Family PCH */
#define DID_INTEL_RAPTORLAKE_Z790_PCH	0x7a04
#define DID_INTEL_RAPTORLAKE_H770_PCH	0x7a05
#define DID_INTEL_RAPTORLAKE_B760_PCH	0x7a06
#define DID_INTEL_RAPTORLAKE_WM790_PCH	0x7a0d
#define DID_INTEL_RAPTORLAKE_HM770_PCH	0x7a0c
#define DID_INTEL_RAPTORLAKE_C262_PCH	0x7a14
#define DID_INTEL_RAPTORLAKE_C266_PCH	0x7a13
#define DID_INTEL_RPL_D_PCH_SMBUS	0x7a23 /* RPL-Desktop Watchdog	*/
/* Source: Intel(R) Pentium(R) Silver N5000 CPU @ 1.10GHz		*/
#define DID_INTEL_GEMINILAKE_HB 	0x31f0
/* Source: Meteor Lake: Intel Core Ultra Processor Datasheet, Vol 1	*/
#define DID_INTEL_METEORLAKE_UT4_2_8_2_HB 0x7d00
#define DID_INTEL_METEORLAKE_H_6_8_2_HB 0x7d01
#define DID_INTEL_METEORLAKE_U_2_8_2_HB 0x7d02
#define DID_INTEL_METEORLAKE_H_4_8_2_HB 0x7d14
#define DID_INTEL_METEORLAKE_U_2_4_2_HB 0x7d16
#define DID_INTEL_METEORLAKE_H_PCH	0x7e02
#define DID_INTEL_METEORLAKE_U_PCH	0x7e03
#define DID_INTEL_METEORLAKE_UT4_PCH	0x7e07
/* Source: Arrow Lake Client Platform/MTL-S (0x6-0xc6, stepping: 0x0)	*/
#define DID_INTEL_ARL_MTL_PCH_S_SMBUS	0x7f23
/* Source: Intel Core Ultra 200S | ASRock Z890 Taichi OCF		*/
#define DID_INTEL_ARROWLAKE_S_8_16_HB	0x7d1a
#define DID_INTEL_ARROWLAKE_S_8_12_HB	0x7d1b
#define DID_INTEL_ARROWLAKE_S_6_8_HB	0x7d2a
#define DID_INTEL_ARROWLAKE_S_PCH	0xae0d
#define DID_INTEL_ARROWLAKE_S_SMBUS	0xae22
/* Source: Intel Core Ultra 200V Series Processors Datasheet, Vol 1	*/
#define DID_INTEL_LUNARLAKE_V_4P_4E	0x6400
#define DID_INTEL_LUNARLAKE_V_PCH	0xa807
#define DID_INTEL_LUNARLAKE_V_SMBUS	0xa822
/* Source: /include/linux/pci_ids.h					*/
#define DID_AMD_K8_NB_MEMCTL		0x1102
#define DID_AMD_K8_NB			0x1100
/* Source: AMD Ryzen 7 2700X [ Zen+ Pinnacle Ridge ]			*/
#define DID_AMD_17H_ZEN_PLUS_NB_IOMMU	0x1451	/*	PinnacleRidge	*/
#define DID_AMD_17H_ZEPPELIN_NB_IOMMU	0x14a1	/*	Zeppelin	*/
#define DID_AMD_17H_RAVEN_NB_IOMMU	0x1629	/*	Raven		*/
/* Source: AMD Ryzen 9 3950X [ Zen2/Matisse ]				*/
#define DID_AMD_17H_ZEN2_MTS_NB_IOMMU	0x1481	/*	Matisse 	*/
#define DID_AMD_17H_STARSHIP_NB_IOMMU	0x14d1	/*	Starship	*/
#define DID_AMD_17H_RENOIR_NB_IOMMU	0x1489	/*	Renoir		*/
/* Source: AMD PPR for AMD Family 17h Models 18h & 20h Processors	*/
#define DID_AMD_17H_ZEN_APU_NB_IOMMU	0x15d1	/*	Raven2		*/
/* Source: AMD PPR for AMD Family 17h Model 60h Processors		*/
/* Source: AMD PPR for AMD Family 19h Model 51h, Rev A1 Processors	*/
#define DID_AMD_17H_ZEN2_APU_NB_IOMMU	0x1631	/*	Renoir/Cezanne	*/
#define DID_AMD_17H_FIREFLIGHT_NB_IOMMU 0x15f9	/*	FireFlight	*/
#define DID_AMD_17H_ARDEN_NB_IOMMU	0x1627	/*	Arden		*/
/* Source: Steam Deck LCD						*/
#define DID_AMD_17H_JUPITER_NB_IOMMU	0x1646	/*	Van Gogh/Jupiter*/
/* Source: AMD Ryzen 5 6600H [ Zen3+ Rembrandt ]			*/
#define DID_AMD_19H_ZEN3_RMB_NB_IOMMU	0x14b6	/*	Rembrandt	*/
/* Source: AMD Ryzen 9 7950X [ Zen4/Raphael ]				*/
#define DID_AMD_19H_ZEN4_RPL_NB_IOMMU	0x14d9	/*	Raphael 	*/
#define DID_AMD_19H_ZEN4_GNA_NB_IOMMU	0x149e	/*	Genoa		*/
#define DID_AMD_19H_ZEN4_PHX_NB_IOMMU	0x14e9	/*	Phoenix 	*/
/* Source: /include/linux/pci_ids.h					*/
#define DID_AMD_17H_ZEPPELIN_DF_UMC	0x1460	/*	Zeppelin	*/
#define DID_AMD_17H_RAVEN_DF_UMC	0x15e8	/*	Raven		*/
#define DID_AMD_17H_MATISSE_DF_UMC	0x1440	/*	Matisse 	*/
#define DID_AMD_17H_STARSHIP_DF_UMC	0x1490	/*	Starship	*/
#define DID_AMD_17H_RENOIR_DF_UMC	0x1448	/*	Renoir		*/
#define DID_AMD_17H_ARIEL_DF_UMC	0x13f3	/*	Ariel		*/
#define DID_AMD_17H_RAVEN2_DF_UMC	0x15eb	/*	Raven2		*/
#define DID_AMD_17H_FIREFLIGHT_DF_UMC	0x15f3	/*	FireFlight	*/
#define DID_AMD_17H_ARDEN_DF_UMC	0x160b	/*	Arden		*/
#define DID_AMD_17H_JUPITER_DF_UMC	0x1660	/*	Van Gogh/Jupiter*/
#define DID_AMD_19H_VERMEER_DF_UMC	0x1650	/*	Vermeer 	*/
#define DID_AMD_19H_CEZANNE_DF_UMC	0x166a	/*	Cezanne 	*/
#define DID_AMD_19H_REMBRANDT_DF_UMC	0x1679	/*	Rembrandt 	*/
#define DID_AMD_19H_RAPHAEL_DF_UMC	0x14e0	/*	Raphael 	*/
#define DID_AMD_19H_GENOA_DF_UMC	0x14ad	/*	Genoa		*/
#define DID_AMD_19H_PHOENIX_DF_UMC	0x14f0	/*	Phoenix 	*/
