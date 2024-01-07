/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define OF( _ptr_ , ...)	_ptr_ __VA_ARGS__
#define RO( _ptr_ , ...)	OF( _ptr_##_RO , __VA_ARGS__ )
#define RW( _ptr_ , ...)	OF( _ptr_##_RW , __VA_ARGS__ )

#define SHM_FILENAME_RO "corefreq-ro-shm"
#define SHM_FILENAME_RW "corefreq-rw-shm"

#define SIG_RING_MS	(500 * 1000000LU)
#define CHILD_PS_MS	(500 * 1000000LU)
#define CHILD_TH_MS	(500 * 1000000LU)

enum CHIPSET {
	IC_CHIPSET,
/*	IC_LAKEPORT,
	IC_LAKEPORT_P,
	IC_LAKEPORT_X,
	IC_CALISTOGA,
	IC_BROADWATER,
	IC_CRESTLINE,
	IC_CANTIGA,
	IC_BEARLAKE_Q,
	IC_BEARLAKE_P,
	IC_BEARLAKE_QF,
	IC_BEARLAKE_X,
	IC_INTEL_3200,
	IC_EAGLELAKE_Q,
	IC_EAGLELAKE_P,
	IC_EAGLELAKE_G,
	IC_PINEVIEW,
	IC_CEDARVIEW,
	IC_BAYTRAIL,
	IC_AIRMONT,
	IC_GOLDMONT_PLUS,
	IC_TYLERSBURG,
	IC_IBEXPEAK,
	IC_IBEXPEAK_M,
	IC_COUGARPOINT,
	IC_PATSBURG,
	IC_CAVECREEK,
	IC_WELLSBURG,
	IC_PANTHERPOINT,
	IC_PANTHERPOINT_M,
	IC_LYNXPOINT,
	IC_LYNXPOINT_M,
	IC_WILDCATPOINT,
	IC_WILDCATPOINT_M,
	IC_SUNRISEPOINT,
	IC_UNIONPOINT,
	IC_CANNONPOINT,
	IC_400_SERIES_P,
	IC_400_SERIES_M,
	IC_495,
	IC_H470,
	IC_Z490,
	IC_Q470,
	IC_HM470,
	IC_QM480,
	IC_WM490,
	IC_W480,
	IC_H510,
	IC_B560,
	IC_H570,
	IC_Q570,
	IC_Z590,
	IC_W580,
	IC_H610,
	IC_B660,
	IC_H670,
	IC_Z690,
	IC_Q670,
	IC_W680,
	IC_WM690,
	IC_HM670,
	IC_ADL_PCH_P,
	IC_Z790,
	IC_H770,
	IC_B760,
	IC_MTL_PCH,
	IC_K8,
	IC_ZEN,
*/	CHIPSETS
};

/* Circular buffer							*/
#define RING_SIZE	16

typedef struct
{
	Bit64				Toggle __attribute__ ((aligned (8)));

	Bit64				OffLine __attribute__ ((aligned (8)));

	unsigned int			Boost[BOOST(SIZE)];

	struct
	{
		CPUID_0x00000000	StdFunc;
		CPUID_0x80000000	ExtFunc;

		unsigned int		Microcode;

		struct {
		unsigned short int	CfgLock :  1-0,
					IORedir :  2-1,
					Unused	: 16-2;
		};
		unsigned short int	CStateLimit;
		struct {
		unsigned short int	CStateInclude;	/* Intel	*/
		unsigned short int	CStateBaseAddr; /* Any I/O BAR	*/
		};
	} Query;

	struct {
		signed int		ApicID,
					CoreID,
					ThreadID,
					PackageID;
		union {
			unsigned int	ID;
		    struct {
			unsigned int	Node	:  8-0,
					CCX	: 16-8,
					CCD	: 24-16,
					CMP	: 32-24;
		    };
			unsigned int	Hybrid_ID;
		} Cluster;

		struct {
			unsigned short	x2APIC	:  8-0,
					_pad	: 12-8,
					Ecore	: 13-12,
					Pcore	: 14-13,
					BSC	: 15-14,
					BSP	: 16-15;
		} MP;

		struct {
		unsigned int		Set,
					Size;
		unsigned short		LineSz,
/*TODO(CleanUp)				Part,	*/
					Way;
		    struct {
		    unsigned short	WriteBack: 1-0,
					Inclusive: 2-1,
					_pad16	: 16-2;
		    } Feature;
		} Cache[CACHE_MAX_LEVEL];
	} Topology;

	struct {
		unsigned int		Limit[SENSOR_LIMITS_DIM];
		struct {
			unsigned int	ClockMod : 16-0,
					Extended : 32-16;
			} DutyCycle;
		unsigned int		PowerPolicy;
		struct HWP_STRUCT {
			struct {
			unsigned int	Highest,
					Guaranteed,
					Most_Efficient,
					Lowest;
			} Capabilities;
			struct {
			unsigned int	Minimum_Perf,
					Maximum_Perf,
					Desired_Perf,
					Energy_Pref;
			} Request;
		} HWP;
	} PowerThermal;

	THERMAL_POINT			ThermalPoint;

	struct {
		struct {
			double		Limit[SENSOR_LIMITS_DIM];
		} Voltage;
		struct {
			double		Limit[SENSOR_LIMITS_DIM];
		} Energy;
		struct {
			double		Limit[SENSOR_LIMITS_DIM];
		} Power;
	} Sensors;

	struct FLIP_FLOP {

		struct
		{
		unsigned long long
					INST;
			struct {
		unsigned long long
				UCC,
				URC;
			}		C0;
		unsigned long long
					C3,
					C6,
					C7,
					TSC,
					C1;
			struct {
		unsigned long long	ACCU;
			} Power;
		} Delta __attribute__ ((aligned (8)));

		CLOCK			Clock;

		struct {
			double		IPS,
					IPC,
					CPI,
					Turbo,
					C0,
					C3,
					C6,
					C7,
					C1,
					Energy,
					Power;
		} State;

		struct {
			double		Ratio,
					Freq;
		} Relative;

		struct {
			double		Freq;
		    struct {
			double		Perf;	/* STATUS or BOOST P-State */
		    } Ratio;
		} Absolute;

		struct {
		unsigned int		Sensor,
					Temp;
		enum THERM_PWR_EVENTS	Events[eDIM];
		THERMAL_PARAM		Param;
		} Thermal;

		struct {
			int		VID;
			double		Vcore;
		} Voltage;

		struct {
		unsigned int		SMI;
			struct {
			unsigned int	LOCAL,
					UNKNOWN,
					PCISERR,
					IOCHECK;
			} NMI;
		} Counter;
	} FlipFlop[2];

	struct {
		double		Freq[SENSOR_LIMITS_DIM];
	} Relative;
	struct {
		double		Freq[SENSOR_LIMITS_DIM];
	} Absolute;

	struct {
		Bit64			FLAGS	__attribute__ ((aligned (8)));
/*TODO(CleanUp)
					CR0	__attribute__ ((aligned (8))),
					CR3	__attribute__ ((aligned (8))),
					CR4	__attribute__ ((aligned (8))),
					CR8	__attribute__ ((aligned (8))),
					EFCR	__attribute__ ((aligned (8))),
					EFER	__attribute__ ((aligned (8))),
					XCR0	__attribute__ ((aligned (8))),
					SYSCFG	__attribute__ ((aligned (8)));
*/
	} SystemRegister;

	CPUID_STRUCT			CpuID[CPUID_MAX_FUNC];

	struct SLICE_STRUCT {
		struct
		{
		unsigned long long	TSC,
					INST;
		} Delta;

		struct {
		unsigned long long	TSC,
					INST;
		} Counter[3];

		unsigned long long	Error;

		struct {
		     struct random_data data;
			char		state[128];
			int		value[2];
		} Random;

		struct {
		unsigned long long	inside,
					trials;
		} Monte_Carlo;
	} Slice __attribute__ ((aligned (8)));
} CPU_STRUCT;

typedef struct
{
	Bit64			Toggle __attribute__ ((aligned (8)));

	FEATURES		Features;
/*
	Bit64			PowerNow	__attribute__ ((aligned (8)));
*/
	struct {
		unsigned long long
/*				PowerNow	:  1-0,
				ODCM		:  2-1,
				PowerMgmt	:  3-2,
				EIST		:  4-3,*/
				Turbo		:  5-4,
/*				C1E		:  6-5,
				C3A		:  7-6,
				C1A		:  8-7,
				C3U		:  9-8,
				C1U		: 10-9,
				CC6		: 11-10,
				PC6		: 12-11,
				SMM		: 13-12,*/
				VM		: 14-13,
				IOMMU		: 15-14,
/*				RaceToHalt	: 16-15,
				L1_HW_Prefetch	: 17-16,
				L1_HW_IP_Prefetch:18-17,
				L2_HW_Prefetch	: 19-18,
				L2_HW_CL_Prefetch:20-19,*/
				IOMMU_Ver_Major : 24-20,
				IOMMU_Ver_Minor : 28-24,
/*				WDT		: 29-28,
				TM1		: 31-29,
				TM2		: 33-31,
				L1_Scrubbing	: 34-33,*/
				_pad64		: 64-34;
	} Technology;

	struct {
		unsigned long long
				IBRS		:  2-0,
				STIBP		:  4-2,
				SSBD		:  6-4,
			/*	RDCL_NO 	:  8-6,
				IBRS_ALL	: 10-8,
				RSBA		: 12-10,
				L1DFL_VMENTRY_NO: 14-12,
				SSB_NO		: 16-14,
				MDS_NO		: 18-16,
				PSCHANGE_MC_NO	: 20-18,
				TAA_NO		: 22-20,
				STLB		: 24-22,
				FUSA		: 26-24,
				RSM_CPL0	: 28-26,
				SPLA		: 30-28,
				SNOOP_FILTER	: 32-30,*/
				PSFD		: 34-32,
			/*	DOITM_EN	: 36-34,
				SBDR_SSDP_NO	: 38-36,
				FBSDP_NO	: 40-38,
				PSDP_NO 	: 42-40,
				FB_CLEAR	: 44-42,
				SRBDS		: 45-44,
				RNGDS		: 47-45,
				RTM		: 49-47,
				VERW		: 51-49,
				RRSBA		: 53-51,
				BHI_NO		: 55-53,
				XAPIC_DIS	: 57-55,
				PBRSB_NO	: 59-57,
				IPRED_DIS_U	: 61-59,
				IPRED_DIS_S	: 63-61,
				MCDT_NO 	: 64-63,
				RRSBA_DIS_U	:  2-0,
				RRSBA_DIS_S	:  4-2,
				BHI_DIS_S	:  6-4,
				BTC_NOBR	:  8-6,
				DRAM_Scrambler	: 10-8,
				TSME		: 12-10,
				DDPD_U_DIS	: 14-12,*/
				_UnusedMechBits : 64-14;
	} Mechanisms;

	enum THERMAL_FORMULAS	thermalFormula;
	enum VOLTAGE_FORMULAS	voltageFormula;
	enum POWER_FORMULAS	powerFormula;

	struct {
		unsigned int		Count,
					OnLine;
	} CPU;

	SERVICE_PROC			Service;

	union {
		unsigned int		PM_version;
		struct PMU_ST {
			unsigned short	v, p;
		} PM_ext;
	};
	struct {
		unsigned int		Rel,
					Abs;
	} Top;

	struct PKG_FLIP_FLOP {
		struct {
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
		unsigned long long	MC6,
					ACCU[PWR_DOMAIN(SIZE)];
		} Delta;

		struct {
		unsigned long long	FC0;
		} Uncore;

		struct {
		unsigned int		Sensor,
					Temp;
		enum THERM_PWR_EVENTS	Events[eDIM];
		} Thermal;

		struct {
		    struct {
			int		CPU, SOC;
		    } VID;
			double		CPU, SOC;
		} Voltage;
	} FlipFlop[2] __attribute__ ((aligned (8)));

	struct {
		double			PC02,
					PC03,
					PC04,
					PC06,
					PC07,
					PC08,
					PC09,
					PC10,
					MC6;
		struct {
			double		Limit[SENSOR_LIMITS_DIM];
		} Voltage;

		struct {
			double		Current,
					Limit[SENSOR_LIMITS_DIM];
		} Energy[PWR_DOMAIN(SIZE)];

		struct {
			double		Current,
					Limit[SENSOR_LIMITS_DIM];
		} Power[PWR_DOMAIN(SIZE)];
	} State;

	struct {
		double			Turbo,
					C0,
					C3,
					C6,
					C7,
					C1,
					MC6;
	} Avg;

	struct {
		struct {
			double		Watts,
					Joules,
					Times;
		} Unit;
		struct {
			double		TAU[PWR_LIMIT_SIZE];
			unsigned short	PWL[PWR_LIMIT_SIZE];
		  struct {
			unsigned char	Enable	:  1-0,
					Clamping:  2-1,
					Unlock	:  3-2,
					_Unused :  8-3;
		    union {
			unsigned char	TW;
		      struct {
			unsigned char	TW_Y	:  5-0,
					TW_Z	:  7-5,
					MaskBit :  8-7;
		      };
		    };
		  } Feature[PWR_LIMIT_SIZE];
		} Domain[PWR_DOMAIN(SIZE)];
		unsigned short		TDP, Min, Max;
		unsigned short		PPT, EDC, TDC;
		struct {
			unsigned short	TDC	:  1-0,
					_Unused : 16-1;
		} Feature;
	} Power;

	THERMAL_POINT			ThermalPoint;

	signed int			ArchID;
	enum HYPERVISOR 		HypervisorID;
	char				Architecture[CODENAME_LEN],
					Brand[BRAND_SIZE];
} PROC_STRUCT;

typedef struct
{
	FOOTPRINT		FootPrint;

	BitCC			roomSched __attribute__ ((aligned (16)));

	enum THERM_PWR_EVENTS	ProcessorEvents[eDIM];

	struct {	/*	NMI bits: 0 is Unregistered; 1 is Registered */
		Bit64		NMI	__attribute__ ((aligned (8)));
		signed int	AutoClock, /* 10: Auto, 01: Init, 00: Specs */
				Experimental,/* 0: Disable, 1: Enable	*/
				HotPlug, /* < 0: Disable, Other: Enable */
				PCI;	/*  < 0: Disable, other: Enable */
	    struct {			/*  = 0: Disable, 1: Enable	*/
		unsigned int	HSMP	:  1-0,
				_pad32	: 32-1;
	    };
		KERNEL_DRIVER	Driver; /*0:Disable, 1:Enable, 2:Full-control*/
	} Registration;

	struct {
		Bit64		Operation	__attribute__ ((aligned (8)));

		OS_DRIVER	OS;

		unsigned int	tickReset,
				tickStep;

		pid_t		trackTask;
		enum SORTBYFIELD sortByField;
		int		reverseOrder,
				taskCount;
		TASK_MCB	taskList[TASK_LIMIT];

		MEM_MCB 	memInfo;

		struct {
		unsigned short	version,
				major,
				minor;
		} kernel;

		unsigned short	procCount;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
	} SysGate;

	struct {
		unsigned char	index[8];
		char		array[MAX_UTS_LEN];
	} CS;

	struct {
		unsigned int	Interval;
		struct timespec pollingWait,
				ringWaiting,
				childWaiting,
				sliceWaiting;
	} Sleep;

	time_t				StartedAt;

	char				ShmName[TASK_COMM_LEN];
	struct {
		pid_t			Svr,
					Cli,
					GUI;
	} App;

	struct {
		unsigned int		Boost[UNCORE_BOOST(SIZE)];
	    struct
	    {
		unsigned long long	Speed;
		unsigned int		Rate;
	    } Bus;

	    struct {
		struct {
			RAM_TIMING	Timing;
			RAM_GEOMETRY	DIMM[MC_MAX_DIMM];
		} Channel[MC_MAX_CHA];
		unsigned short		SlotCount, ChannelCount;
	    } MC[MC_MAX_CTRL];

	unsigned long long		CtrlSpeed;
	unsigned short			CtrlCount,
					ChipID;

	    struct {
		unsigned short	/* 00:MHz , 01:MT/s , 10:MB/s , 11:VOID */
					Bus_Rate:  2-0,
					BusSpeed:  4-2,
					DDR_Rate:  6-4,
					DDRSpeed:  8-6,
					DDR_Std : 12-8,
					DDR_Ver : 16-12;
	    } Unit;

	    struct {
		enum CHIPSET		ArchID;
		char			CodeName[CODENAME_LEN];
	    } Chipset;
	} Uncore;

	SMBIOS_ST		SMB;

	PROC_STRUCT		Proc;
	CPU_STRUCT		Cpu[];
} SHM_STRUCT_RO;

typedef struct {
	struct {
		Bit64		Sync __attribute__ ((aligned (8)));
	} Proc;

	struct {
		RING_CTRL	buffer[RING_SIZE] __attribute__((aligned(16)));
		unsigned int	head, tail;
	} Ring[2]; /* [0] Parent ; [1] Child				*/

	struct {
		RING_CTRL	buffer[RING_SIZE] __attribute__((aligned(16)));
		unsigned int	head, tail;
	} Error;
} SHM_STRUCT_RW;

/* Error Reasons management.						*/
typedef struct {
	__typeof__ (errno)	no: 32;
	__typeof__ (__LINE__)	ln: 26;
	enum REASON_CLASS	rc: 6;
} REASON_CODE;

#define REASON_SET_2xARG(_reason, _rc, _no)				\
{									\
	_reason.no = _no;						\
	_reason.ln = __LINE__;						\
	_reason.rc = _rc;						\
}

#define REASON_SET_1xARG(_reason, _rc)					\
{									\
	_reason.no = errno;						\
	_reason.ln = __LINE__;						\
	_reason.rc = _rc;						\
}

#define REASON_SET_0xARG(_reason)					\
{									\
	_reason.no = errno;						\
	_reason.ln = __LINE__;						\
	_reason.rc = RC_SYS_CALL;					\
}

#define REASON_DISPATCH(_1,_2,_3,REASON_CURSOR, ... ) REASON_CURSOR

#define REASON_SET( ... )						\
	REASON_DISPATCH( __VA_ARGS__ ,	REASON_SET_2xARG,		\
					REASON_SET_1xARG,		\
					REASON_SET_0xARG)( __VA_ARGS__ )

#define REASON_INIT(_reason)		\
	REASON_CODE _reason = {.no = 0, .ln = 0, .rc = RC_SUCCESS}

#define IS_REASON_SUCCESSFUL(_reason) (_reason.rc == RC_SUCCESS)

#if defined(UBENCH) && UBENCH == 1
    #define Print_uBenchmark(quiet)					\
    ({									\
	if (quiet)							\
		printf("%llu\t%llu\n",UBENCH_METRIC(0),UBENCH_METRIC(1));\
    })
#else
	#define Print_uBenchmark(quiet) {}
#endif /* UBENCH */

#define ELAPSED(ref)							\
({									\
	time_t now;							\
	time(&now);							\
	now - ref;							\
})
