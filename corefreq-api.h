/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
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

typedef struct
{
	LOCAL_APIC	Base;
	signed int	ApicID,
			CoreID,
			ThreadID,
			PackageID;
	union {
	unsigned int	ID;		/* AMD-17h MSR(0x0000002a)	*/
	unsigned int	Node	:  8-0, /* CPUID(0x8000001e):ECX[8-0]	*/
			CCX	: 16-8, /* CPUID(0x8000001e):EAX[32-0]:[3] */
			CCD	: 24-16,
			CMP	: 32-24;
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

	unsigned int			Boost[BOOST(SIZE)];
	struct {
		unsigned int		Perf;
	} Ratio;
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
	/* 4C00h */		HSW_DDR_TIMING_4C00	REG4C00; /*32 bits    */
	/* 4C04h */		HSW_DDR_TIMING		Timing; /* 32 bits    */
	/* 4C08h */		HSW_DDR_RANK_TIMING_A	Rank_A; /* 32 bits    */
	/* 4C0Ch */		HSW_DDR_RANK_TIMING_B	Rank_B; /* 32 bits    */
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
			AMD_0F_HTT_NODE_ID	NodeID;
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
} SYSGATE_RO; /*		RO Pages				*/

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
		unsigned short	ChipID;
	} Uncore;

	struct {
		THERMAL_PARAM	Param;
		unsigned int	Sensor;
	enum THERM_PWR_EVENTS	Events;
		RAPL_POWER_UNIT Unit;
		PKG_POWER_INFO	PowerInfo;
	} PowerThermal;

	struct {
		struct {
			size_t	Size;
			int	Order;
		} ReqMem;
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
} PROC_RO; /*			RO Pages				*/

typedef struct
{
	struct
	{
	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Delta __attribute__ ((aligned (8)));

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
	Bit256			SPLA		__attribute__ ((aligned (16)));

	struct {
		Bit64		Signal	__attribute__ ((aligned (8)));
	} OS;
} PROC_RW; /*			RW Pages				*/


#ifndef PCI_DEVICE_ID_INTEL_82945P_HB
	#define PCI_DEVICE_ID_INTEL_82945P_HB		0x2770
#endif
#ifndef PCI_DEVICE_ID_INTEL_82945GM_HB
	#define PCI_DEVICE_ID_INTEL_82945GM_HB		0x27a0
#endif
#ifndef PCI_DEVICE_ID_INTEL_82955_HB
	#define PCI_DEVICE_ID_INTEL_82955_HB		0x2774
#endif
/* Source: /drivers/char/agp/intel-agp.h				*/
#ifndef PCI_DEVICE_ID_INTEL_82945GME_HB
	#define PCI_DEVICE_ID_INTEL_82945GME_HB		0x27ac
#endif
#ifndef PCI_DEVICE_ID_INTEL_82946GZ_HB
	#define PCI_DEVICE_ID_INTEL_82946GZ_HB		0x2970
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965Q_HB
	#define PCI_DEVICE_ID_INTEL_82965Q_HB		0x2990
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965G_HB
	#define PCI_DEVICE_ID_INTEL_82965G_HB		0x29a0
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965GM_HB
	#define PCI_DEVICE_ID_INTEL_82965GM_HB		0x2a00
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965GME_HB
	#define PCI_DEVICE_ID_INTEL_82965GME_HB 	0x2a10
#endif
#ifndef PCI_DEVICE_ID_INTEL_GM45_HB
	#define PCI_DEVICE_ID_INTEL_GM45_HB		0x2a40
#endif
#ifndef PCI_DEVICE_ID_INTEL_Q35_HB
	#define PCI_DEVICE_ID_INTEL_Q35_HB		0x29b0
#endif
#ifndef PCI_DEVICE_ID_INTEL_G33_HB
	#define PCI_DEVICE_ID_INTEL_G33_HB		0x29c0
#endif
#ifndef PCI_DEVICE_ID_INTEL_Q33_HB
	#define PCI_DEVICE_ID_INTEL_Q33_HB		0x29d0
#endif
/* Source: /drivers/edac/x38_edac.c					*/
#ifndef PCI_DEVICE_ID_INTEL_X38_HB
	#define PCI_DEVICE_ID_INTEL_X38_HB		0x29e0
#endif
/* Source: /drivers/edac/i3200_edac.c					*/
#ifndef PCI_DEVICE_ID_INTEL_3200_HB
	#define PCI_DEVICE_ID_INTEL_3200_HB		0x29f0
#endif
/* Source: /drivers/char/agp/intel-agp.h				*/
#ifndef PCI_DEVICE_ID_INTEL_Q45_HB
	#define PCI_DEVICE_ID_INTEL_Q45_HB		0x2e10
#endif
#ifndef PCI_DEVICE_ID_INTEL_G45_HB
	#define PCI_DEVICE_ID_INTEL_G45_HB		0x2e20
#endif
#ifndef PCI_DEVICE_ID_INTEL_G41_HB
	#define PCI_DEVICE_ID_INTEL_G41_HB		0x2e30
#endif
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_INTEL_I7_MCR
	#define PCI_DEVICE_ID_INTEL_I7_MCR		0x2c18
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL	0x2c20
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL	0x2c28
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL	0x2c30
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_TEST
	#define PCI_DEVICE_ID_INTEL_I7_MC_TEST		0x2c1c
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH0_ADDR
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH0_ADDR	0x2c21
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH1_ADDR
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH1_ADDR	0x2c29
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH2_ADDR
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH2_ADDR	0x2c31
#endif
#ifndef PCI_DEVICE_ID_INTEL_BLOOMFIELD_NON_CORE
	#define PCI_DEVICE_ID_INTEL_BLOOMFIELD_NON_CORE 0x2c41
#endif
#ifndef PCI_DEVICE_ID_INTEL_C5500_NON_CORE
	#define PCI_DEVICE_ID_INTEL_C5500_NON_CORE	0x2c58
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE	0x2c51
#endif
#ifndef PCI_DEVICE_ID_INTEL_CLARKSFIELD_NON_CORE
    #define PCI_DEVICE_ID_INTEL_CLARKSFIELD_NON_CORE	0x2c52
#endif
#ifndef PCI_DEVICE_ID_INTEL_CLARKDALE_NON_CORE
	#define PCI_DEVICE_ID_INTEL_CLARKDALE_NON_CORE	0x2c61
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR	0x2c98
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_CTRL
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_CTRL	0x2ca0
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_CTRL
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_CTRL	0x2ca8
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST	0x2c9c
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_ADDR
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_ADDR	0x2ca1
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_ADDR
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_ADDR	0x2ca9
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MCR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MCR		0x2d98
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_CTRL
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_CTRL	0x2da0
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_CTRL
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_CTRL	0x2da8
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_CTRL
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_CTRL	0x2db0
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_TEST
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_TEST	0x2d9c
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_ADDR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_ADDR	0x2da1
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_ADDR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_ADDR	0x2da9
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_ADDR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_ADDR	0x2db1
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_NON_CORE
	#define PCI_DEVICE_ID_INTEL_NHM_EP_NON_CORE	0x2c70
#endif
/* Source: Intel X58 Express Chipset Datasheet				*/
#define PCI_DEVICE_ID_INTEL_X58_HUB_CORE		0x342e
#define PCI_DEVICE_ID_INTEL_X58_HUB_CTRL		0x3423
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0	0x3ca0
#endif
/* Source: 2nd Generation Intel® Core™ Processor Family Vol2		*/
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA	0x0100
#endif
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104	0x0104
#endif
/* Source: /drivers/edac/sb_edac.c					*/
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0	0x0ea0
#endif
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA1
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA1	0x0e60
#endif
/* Source: 3rd Generation Intel® Core™ Processor Family Vol2		*/
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA	0x0150
#endif
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154	0x0154
#endif
/* Source: Intel Xeon Processor E5 & E7 v1 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_HOST_BRIDGE
	#define PCI_DEVICE_ID_INTEL_SNB_EP_HOST_BRIDGE	0x3c00
#endif
/*	QPIMISCSTAT: Device=8 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_QPI_LINK0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_QPI_LINK0	0x3c80
#endif
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=15 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CPGC
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CPGC 0x3ca8
#endif
/*TODO( Nehalem/Xeon E7 - CPGC: Device=?? - Function=? )
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CPGC
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CPGC 0x0
#endif									*/
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=16 - Function=0,1,2,3			*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH0 0x3cb0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH1 0x3cb1
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH2 0x3cb2
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH3 0x3cb3
#endif
/*	Controller #1: Device=16 - Function=4,5,6,7			*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH0 0x3cb4
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH1 0x3cb5
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH2 0x3cb6
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH3 0x3cb7
#endif
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=15 - Function=2,3,4,5,6	*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH0 0x3caa
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH1 0x3cab
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH2 0x3cac
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH3 0x3cad
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH4
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH4 0x3cae
#endif
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*TODO( Nehalem/Xeon E7 - TAD Controller #1: Device=?? - Function=? )
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH0 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH1 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH2 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH3 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH4
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH4 0x0
#endif									*/
/*	Power Control Unit						*/
/*TODO( PCU: Device=10 - Function=3 )					*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_CAPABILITY
	#define PCI_DEVICE_ID_INTEL_SNB_EP_CAPABILITY	0x3cd0
#endif
/* Source: Intel Xeon Processor E5 & E7 v2 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_HOST_BRIDGE
	#define PCI_DEVICE_ID_INTEL_IVB_EP_HOST_BRIDGE	0x0e00
#endif
/*	QPIMISCSTAT: Device=8 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_QPI_LINK0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_QPI_LINK0	0x0e80
#endif
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=15 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CPGC
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CPGC 0x0ea8
#endif
/*	Xeon E7 - CPGC: Device=29 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CPGC
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CPGC 0x0e68
#endif
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=16 - Function=0,1,2,3			*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH0 0x0eb0
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH1 0x0eb1
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH2 0x0eb2
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH3 0x0eb3
#endif
/*	Controller #1: Device=16 - Function=4,5,6,7			*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH0 0x0eb4
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH1 0x0eb5
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH2 0x0eb6
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH3 0x0eb7
#endif
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=15 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH0 0x0eaa
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH1 0x0eab
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH2 0x0eac
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH3 0x0ead
#endif
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*	Xeon E7 - TAD Controller #1: Device=29 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH0 0x0e6a
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH1 0x0e6b
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH2 0x0e6c
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH3 0x0e6d
#endif
/*	Power Control Unit						*/
/*	PCU: Device=10 - Function=3					*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_CAPABILITY
	#define PCI_DEVICE_ID_INTEL_IVB_EP_CAPABILITY	0x0ec3
#endif
/* Source: Intel Xeon Processor E5 & E7 v3 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_HOST_BRIDGE
	#define PCI_DEVICE_ID_INTEL_HSW_EP_HOST_BRIDGE	0x2f00
#endif
/*	QPIMISCSTAT: Device=8 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_QPI_LINK0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_QPI_LINK0	0x2f80
#endif
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=19 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CPGC
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CPGC 0x2fa8
#endif
/*	Xeon E7 - CPGC: Device=22 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CPGC
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CPGC 0x2f68
#endif
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*TODO( Controller #0: Device=?? - Function=0,1,2,3 )
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH0 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH1 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH2 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH3 0x0
#endif									*/
/*TODO( Controller #1: Device=?? - Function=4,5,6,7 )
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH0 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH1 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH2 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH3 0x0
#endif									*/
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=19 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH0 0x2faa
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH1 0x2fab
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH2 0x2fac
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH3 0x2fad
#endif
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*	Xeon E7 - TAD Controller #1: Device=22 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH0 0x2f6a
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH1 0x2f6b
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH2 0x2f6c
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH3 0x2f6d
#endif
/*	Power Control Unit						*/
/*	PCU: Device=30 - Function=3					*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_CAPABILITY
	#define PCI_DEVICE_ID_INTEL_HSW_EP_CAPABILITY	0x2fc0
#endif
/* Source: 4th, 5th Generation Intel® Core™ Processor Family Vol2 §3.0	*/
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0	0x2fa0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA
	#define PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA	0x0c00
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_MH_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_MH_IMC_HA0	0x0c04
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_UY_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_UY_IMC_HA0	0x0a04
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0	0x1604
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_D_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_D_IMC_HA0 0x1610
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0 0x1614
#endif
/* Source: 6th Generation Intel® Processor Datasheet for U/Y-Platforms Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA	0x1904
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA	0x190c
#endif
/* Source: 6th Generation Intel® Processor Datasheet for S-Platforms Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD	0x190f
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ	0x191f
#endif
/* Source: 6th Generation Intel® Processor Datasheet for H-Platforms Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD	0x1900
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ	0x1910
#endif
/* Source: Intel Xeon Processor E3-1200 v5 Product Family		*/
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_DT_IMC_HA
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_DT_IMC_HA	0x1918
#endif
/* Source:7th Generation Intel® Processor for S-Platforms & Core X-Series Vol2*/
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD	0x5900
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA	0x5904
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA	0x590c
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD	0x590f
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ	0x5910
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_DT_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_DT_IMC_HA	0x5918
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ	0x5914
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ	0x591f
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ	0x5906
#endif
/* Source: 8th Generation Intel® Processor for S-Platforms Datasheet Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ 0x3e1f
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAS 0x3ec2
#endif
/* Source: 8th and 9th Generation Intel® Core™ and Xeon™ E Processor Families */
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAD 0x3e0f
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAD 0x3ecc
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAQ 0x3ed0
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAQ 0x3e10
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAS 0x3ec4
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAO
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAO 0x3e30
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAQ 0x3e18
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAS 0x3ec6
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAO
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAO 0x3e31
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAQ 0x3e33
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAS 0x3eca
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAO
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAO 0x3e32
#endif
/* Source: 8th Generation Intel® Core™ Processor Families Datasheet Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAD 0x3e35
#endif
#ifndef PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAQ 0x3e34
#endif
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_AMD_K8_NB_MEMCTL
	#define PCI_DEVICE_ID_AMD_K8_NB_MEMCTL		0x1102
#endif
#ifndef PCI_DEVICE_ID_AMD_K8_NB
	#define PCI_DEVICE_ID_AMD_K8_NB			0x1100
#endif
/* Source: AMD PPR for AMD Family 17h Model 20h, Revision A1 Processors */
#ifndef PCI_DEVICE_ID_AMD_17H_MATISSE_NB_IOMMU
	#define PCI_DEVICE_ID_AMD_17H_MATISSE_NB_IOMMU	0x1481
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_20H_2FH_NB_IOMMU
	#define PCI_DEVICE_ID_AMD_17H_20H_2FH_NB_IOMMU	0x15d1
#endif
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3	0x1463	/* Zeppelin */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_RAVEN_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_RAVEN_DF_F3	0x15eb	/* Raven */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_MATISSE_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_MATISSE_DF_F3	0x1443	/* Zen2 */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_STARSHIP_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_STARSHIP_DF_F3	0x1493	/* Zen2 */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_RENOIR_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_RENOIR_DF_F3	0x144b	/* Renoir */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_ARIEL_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_ARIEL_DF_F3	0x13f3	/* Ariel */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_DF_F3	0x15f3	/* FireFlight*/
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_ARDEN_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_ARDEN_DF_F3	0x160b	/* Arden */
#endif

