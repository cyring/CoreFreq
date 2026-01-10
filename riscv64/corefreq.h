/*
 * CoreFreq
 * Copyright (C) 2015-2026 CYRIL COURTIAT
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
	CHIPSETS
};

/* Circular buffer							*/
#define RING_SIZE	16

typedef struct
{
	Bit64				Toggle __attribute__ ((aligned (8)));

	Bit64				OffLine __attribute__ ((aligned (8)));

	COF_ST				Boost[BOOST(SIZE)];

	struct
	{
		unsigned long		Revision;

		struct {
		unsigned short int	CfgLock :  1-0,
					IORedir :  2-1,
					SCTLRX	:  3-2,
					Unused	: 16-3;
		};
		unsigned short int	CStateLimit;
		unsigned short int	CStateBaseAddr; /* Any I/O BAR	*/
	} Query;

	struct {
		unsigned long		PN;
		signed int		BSP,
					CoreID,
					ThreadID,
					PackageID;
		struct CLUSTER_ST	Cluster;
		struct {
		unsigned int		Set,
					Size;
		unsigned short		LineSz,
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
		Bit64			sstatus	__attribute__ ((aligned (8)));
		Bit64			HCR	__attribute__ ((aligned (8)));
		Bit64			SCTLR	__attribute__ ((aligned (8)));
		Bit64			SCTLR2	__attribute__ ((aligned (8)));
		Bit64			EL	__attribute__ ((aligned (8)));
		Bit64			FPSR	__attribute__ ((aligned (8)));
		Bit64			FPCR	__attribute__ ((aligned (8)));
		Bit64			SVCR	__attribute__ ((aligned (8)));
		Bit64			CPACR	__attribute__ ((aligned (8)));
	} SystemRegister;

	struct SLICE_STRUCT {
		unsigned long long	Exclusive __attribute__ ((aligned (8)));

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

	    #ifdef __GLIBC__
		struct {
		     struct random_data data;
			char		state[128];
			int		value[2];
		} Random;
	    #else
		struct {
			char		state[128];
			int		value[2];
		} Random;
	    #endif /* __GLIBC__ */

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

	struct {
		unsigned long long
				_Unused1_	: 13-0,
				VM		: 14-13,
				IOMMU		: 15-14,
				_Unused2_	: 20-15,
				IOMMU_Ver_Major : 24-20,
				IOMMU_Ver_Minor : 28-24,
				_Unused3_	: 64-28;
	} Technology;

	struct {
		unsigned long long
				CLRBHB		:  2-0,
				CSV2		:  6-2,
				CSV3		:  8-6,
				SSBS		: 10-8,
				_UnusedMechBits : 64-10;
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
		COF_ST			Boost[UNCORE_BOOST(SIZE)];
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

#define COF2FLOAT(_COF) 						\
(									\
	(double)_COF.Q + (double)_COF.R / UNIT_KHz(PRECISION)		\
)
