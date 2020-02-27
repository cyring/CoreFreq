/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
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

typedef struct
{
	LOCAL_APIC	Base;
	signed int	ApicID,
			CoreID,
			ThreadID,
			PackageID;

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
		enum THERM_PWR_EVENTS	Events;
		unsigned int
					TCC_Enable:  1-0,
					TM2_Enable:  2-1,
					Unused    : 32-2;
	};
	PERF_CONTROL			PerfControl;
	CLOCK_MODULATION		ClockModulation;
	ENERGY_PERF_BIAS		PerfEnergyBias;
	MISC_PWR_MGMT			PwrManagement;
	HWP_CAPABILITIES		HWP_Capabilities;
	HWP_INTERRUPT			HWP_Interrupt;
	HWP_REQUEST			HWP_Request;
} POWER_THERMAL;

typedef struct
{
	struct	/* 64-byte cache line size.				*/
	{
		unsigned long long	V,
					_pad[7];
	} Sync __attribute__ ((aligned (8)));

	Bit64				OffLine __attribute__ ((aligned (8)));

	struct
	{
		unsigned long long	TSC;
	} Overhead __attribute__ ((aligned (8)));

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

		struct
		{
		unsigned long long	ACCU;
		} Power;
	} Counter[2] __attribute__ ((aligned (8)));

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

	union {
	    struct	/* Intel					*/
	    {
		CORE_GLOBAL_PERF_CONTROL Core_GlobalPerfControl;
		CORE_FIXED_PERF_CONTROL  Core_FixedPerfControl;
	    };
	    struct	/* AMD						*/
	    {
		unsigned long long	Core_PerfEventsCtrsControl;
		HWCR			Core_HardwareConfiguration;
	    };
	} SaveArea;

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
		unsigned short int	CStateLimit,
					CStateInclude;
	} Query;

	CACHE_TOPOLOGY			T;

	struct {
		Bit64			RFLAGS	__attribute__ ((aligned (8))),
					CR0	__attribute__ ((aligned (8))),
					CR3	__attribute__ ((aligned (8))),
					CR4	__attribute__ ((aligned (8))),
					EFER	__attribute__ ((aligned (8)));
		union {
			Bit64		EFCR	__attribute__ ((aligned (8)));
			VM_CR		VMCR;

		};
	} SystemRegister;

	unsigned int			Bind;

	CLOCK				Clock;

	CPUID_STRUCT			CpuID[CPUID_MAX_FUNC];

	struct {
		unsigned int		Perf,
					Target;
	} Ratio;
} CORE;

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
	/* 29Ch */		P965_MC_ODTCTRL		DRT0;	/* 32 bits    */
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
				NHM_IMC_SCHEDULER_PARAMS Params;
			} NHM;
			struct {
	/* 4000h */		SNB_IMC_TC_DBP		DBP;	/* 32 bits    */
	/* 4004h */		SNB_IMC_TC_RAP		RAP;	/* 32 bits    */
	/* 4298h */		SNB_IMC_TC_RFTP 	RFTP;	/* 32 bits    */
			} SNB;
			struct {
	/*  200h */		SNB_IMC_TC_DBP		DBP;	/* 32 bits    */
	/*  204h */		SNB_IMC_TC_RAP		RAP;	/* 32 bits    */
	/*  208h */		SNB_IMC_TC_RWP		RWP;	/* 32 bits    */
	/*  214h */		SNB_IMC_TC_RFTP 	RFTP;	/* 32 bits    */
			} SNB_EP;
			struct {
	/* 4C04h */		HSW_DDR_TIMING		Timing;	/* 32 bits    */
	/* 4C08h */		HSW_DDR_RANK_TIMING_A	Rank_A;	/* 32 bits    */
	/* 4C0Ch */		HSW_DDR_RANK_TIMING_B	Rank_B;	/* 32 bits    */
	/* 4C14h */		HSW_DDR_RANK_TIMING	Rank;	/* 32 bits    */
	/* 4E98h */		HSW_TC_REFRESH_TIMING	Refresh; /*32 bits    */
			} HSW;
			struct {
	/* 4000h */		SKL_IMC_CR_TC_PRE	Timing;	/* 32 bits    */
	/* 401Ch */		SKL_IMC_CR_SC_CFG	Sched;	/* 32 bits    */
	/* 4070h */		SKL_IMC_CR_TC_ODT	ODT;	/* 32 bits    */
	/* 423Ch */		SKL_IMC_REFRESH_TC	Refresh; /*32 bits    */
			} SKL;
			struct {
	/* 88h */		AMD_0F_DRAM_TIMING_LOW	DTRL;	/* 32 bits    */
			} AMD0F;
		};
		union {
	/* 1208h */	G965_MC_DRAM_RANK_ATTRIB	DRA;	/* 32 bits    */
	/* 48h */	NHM_IMC_DOD_CHANNEL		DOD;	/* 32 bits    */
	/* 80h */	SNB_EP_DIMM_MTR 		MTR;	/* 32 bits    */
	/* 40h */	AMD_0F_DRAM_CS_BASE_ADDR	MBA;	/* 32 bits    */
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
	/* 3:0-48h */	NHM_IMC_CONTROL 	CONTROL;	/* 32 bits    */
	/* 3:0 4Ch */	NHM_IMC_STATUS		STATUS;		/* 32 bits    */
		} NHM;
		struct {
	/* 5004h */	SNB_IMC_MAD_CHANNEL	MAD0,		/* 32 bits    */
	/* 5008h */				MAD1;		/* 32 bits    */
		} SNB;
		struct {
			SNB_EP_MC_TECH		TECH;		/* 32 bits    */
	/* 80h */	SNB_EP_TADWAYNESS	TAD;		/* 12x32 bits */
		} SNB_EP;
		struct {
	/* 5000h */	SKL_IMC_MAD_MAPPING	MADCH;		/* 32 bits    */
	/* 5004h */	SKL_IMC_MAD_CHANNEL	MADC0,		/* 32 bits    */
	/* 5008h */				MADC1;		/* 32 bits    */
	/* 500Ch */	SKL_IMC_MAD_DIMM	MADD0,		/* 32 bits    */
	/* 5010h */				MADD1;		/* 32 bits    */
		} SKL;
		struct {
	/* 90h */	AMD_0F_DRAM_CONFIG_LOW	DCRL;		/* 32 bits    */
	/* 94h */	AMD_0F_DRAM_CONFIG_HIGH DCRH;		/* 32 bits    */
		} AMD0F;
	};

	union {
		struct {
	/* 64h */	NHM_IMC_MAX_DOD 	DOD;		/* 32 bits    */
		} NHM;
		struct {
	/* 80h */	AMD_0F_DRAM_CS_MAPPING	CS;		/* 32 bits    */
		} AMD0F;
	} MaxDIMMs;

	unsigned short		SlotCount, ChannelCount;
} MC_REGISTERS;

typedef struct
{
	union {
		struct {
			MCH_CLKCFG		ClkCfg;
		};
		struct {
			NHM_IMC_CLK_RATIO_STATUS DimmClock;
			QPI_FREQUENCY		QuickPath;
		};
		struct {
			SNB_CAPID		SNB_Cap;
			IVB_CAPID		IVB_Cap;
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
		};
		struct {
			AMD_0F_HTT_UNIT_ID	UnitID;
			AMD_0F_HTT_FREQUENCY	LDTi_Freq[3];
		};
	};
	struct {
		unsigned long long	IOMMU_CR;
	};
} BUS_REGISTERS;


typedef struct {
		OS_DRIVER	OS;

		int		taskCount;
		TASK_MCB	taskList[TASK_LIMIT];

		MEM_MCB		memInfo;

		unsigned int	kernelVersionNumber;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
} SYSGATE;

typedef struct
{
	struct
	{
	    unsigned long long	PTSC, /* Package Time Stamp Counter	*/
				PC02, /* Goldmont, Sandy-Bridge, Phi	*/
				PC03, /* Goldmont, Nehalem, Sandy-Bridge, Phi */
				PC06, /* Goldmont, Nehalem, Sandy-Bridge, Phi */
				PC07, /* Nehalem, Sandy-Bridge, Phi	*/
				PC08, /* Haswell			*/
				PC09, /* Haswell			*/
				PC10; /* Goldmont, Haswell		*/
	  struct {
	    unsigned long long	FC0; /* Uncore fixed counter #0		*/
	  } Uncore;

	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Counter[2] __attribute__ ((aligned (8)));

	struct
	{
	    unsigned long long	PTSC,
				PC02,
				PC03,
				PC06,
				PC07,
				PC08,
				PC09,
				PC10;
	  struct {
	    unsigned long long	FC0;
	  } Uncore;

	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Delta __attribute__ ((aligned (8)));

	struct
	{
	    union {
		UNCORE_GLOBAL_PERF_CONTROL Uncore_GlobalPerfControl;
		UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;
	    };
		UNCORE_FIXED_PERF_CONTROL  Uncore_FixedPerfControl;
	} SaveArea;

	FEATURES		Features;

	Bit256			CR_Mask 	__attribute__ ((aligned (16)));
	Bit256			ODCM_Mask	__attribute__ ((aligned (16)));
	Bit256			PowerMgmt_Mask	__attribute__ ((aligned (16)));
	Bit256			SpeedStep_Mask	__attribute__ ((aligned (16)));
	Bit256			TurboBoost_Mask __attribute__ ((aligned (16)));
	Bit256			HWP_Mask __attribute__ ((aligned (16)));
	Bit256			C1E_Mask __attribute__ ((aligned (16)));
	Bit256	/* NHM */	C3A_Mask __attribute__ ((aligned (16)));
	Bit256	/* NHM */	C1A_Mask __attribute__ ((aligned (16)));
	Bit256	/* SNB */	C3U_Mask __attribute__ ((aligned (16)));
	Bit256	/* SNB */	C1U_Mask __attribute__ ((aligned (16)));
	Bit256	/* AMD */	CC6_Mask __attribute__ ((aligned (16)));
	Bit256	/* AMD */	PC6_Mask __attribute__ ((aligned (16)));
	Bit256			SPEC_CTRL_Mask __attribute__ ((aligned (16)));
	Bit256			ARCH_CAP_Mask  __attribute__ ((aligned (16)));

	Bit256			ODCM		__attribute__ ((aligned (16)));
	Bit256			PowerMgmt	__attribute__ ((aligned (16)));
	Bit256			SpeedStep	__attribute__ ((aligned (16)));
	Bit256			TurboBoost	__attribute__ ((aligned (16)));
	Bit256			HWP		__attribute__ ((aligned (16)));
	Bit256			C1E		__attribute__ ((aligned (16)));
	Bit256			C3A		__attribute__ ((aligned (16)));
	Bit256			C1A		__attribute__ ((aligned (16)));
	Bit256			C3U		__attribute__ ((aligned (16)));
	Bit256			C1U		__attribute__ ((aligned (16)));
	Bit256			CC6		__attribute__ ((aligned (16)));
	Bit256			PC6		__attribute__ ((aligned (16)));
	Bit256			SMM		__attribute__ ((aligned (16)));
	Bit256			VM		__attribute__ ((aligned (16)));
	Bit256			IBRS		__attribute__ ((aligned (16)));
	Bit256			STIBP		__attribute__ ((aligned (16)));
	Bit256			SSBD		__attribute__ ((aligned (16)));
	Bit256			RDCL_NO 	__attribute__ ((aligned (16)));
	Bit256			IBRS_ALL	__attribute__ ((aligned (16)));
	Bit256			RSBA		__attribute__ ((aligned (16)));
	Bit256			L1DFL_VMENTRY_NO __attribute__ ((aligned (16)));
	Bit256			SSB_NO		__attribute__ ((aligned (16)));
	Bit256			MDS_NO		__attribute__ ((aligned (16)));
	Bit256			PSCHANGE_MC_NO	__attribute__ ((aligned (16)));
	Bit256			TAA_NO		__attribute__ ((aligned (16)));

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
	unsigned int		Boost[BOOST(SIZE)];

	struct {
		unsigned int	Boost[UNCORE_BOOST(SIZE)];
		BUS_REGISTERS	Bus;
		MC_REGISTERS	MC[MC_MAX_CTRL];
		unsigned short	CtrlCount;
		unsigned short	ChipID;
	} Uncore;

	struct {
		unsigned int	Sensor;
	enum THERM_PWR_EVENTS	Events;
		RAPL_POWER_UNIT Unit;
		PKG_POWER_INFO	PowerInfo;
	} PowerThermal;

	struct {
		Bit64		Signal	__attribute__ ((aligned (8)));
		struct {
			size_t	Size;
			int	Order;
		} ReqMem;
		SYSGATE 	*Gate;
	} OS;

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
} PROC;

