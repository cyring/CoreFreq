/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define SHM_FILENAME	"corefreq-shm"

typedef struct
{
	Bit64				OffLine __attribute__ ((aligned (8)));

	unsigned int			Toggle;

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
		unsigned short int	CStateLimit,
					CStateInclude;
	} Query;

	struct {
		signed int		ApicID,
					CoreID,
					ThreadID,
					PackageID;
		struct {
			unsigned short	x2APIC	:  8-0,
					CCX	: 12-8,
					_pad8	: 15-12,
					BSP	: 16-15;
		} MP;
		struct {
		unsigned int		Set,
					Size;
		unsigned short		LineSz,
					Part,
					Way;
		    struct {
		    unsigned short	WriteBack: 1-0,
					Inclusive: 2-1,
					_pad16	: 16-2;
		    } Feature;
		} Cache[CACHE_MAX_LEVEL];
	} Topology;

	struct {
		unsigned int		TM1,
					TM2,
					Limit[SENSOR_LIMITS_DIM];
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
		unsigned int		Sensor,
					Temp;
		enum THERM_PWR_EVENTS	Events;
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

		struct {
			unsigned int	Perf,
					Target;
		} Ratio;

		struct {
			double		Perf,
					Target;
		} Frequency;
	} FlipFlop[2];

	struct {
		Bit64			RFLAGS	__attribute__ ((aligned (8))),
					CR0	__attribute__ ((aligned (8))),
					CR3	__attribute__ ((aligned (8))),
					CR4	__attribute__ ((aligned (8))),
					EFCR	__attribute__ ((aligned (8))),
					EFER	__attribute__ ((aligned (8)));
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
	} Slice __attribute__ ((aligned (8)));
} CPU_STRUCT;

typedef struct
{
	volatile unsigned long long	Sync __attribute__ ((aligned (8)));

	FEATURES			Features;

	Bit64			PowerNow	__attribute__ ((aligned (8)));

	struct {
		unsigned long long
				PowerNow	:  1-0,
				ODCM		:  2-1,
				PowerMgmt	:  3-2,
				EIST		:  4-3,
				Turbo		:  5-4,
				C1E		:  6-5,
				C3A		:  7-6,
				C1A		:  8-7,
				C3U		:  9-8,
				C1U		: 10-9,
				CC6		: 11-10,
				PC6		: 12-11,
				SMM		: 13-12,
				VM		: 14-13,
				IOMMU		: 15-14,
				_pad64		: 64-15;
	} Technology;

	struct {
		unsigned long long
				IBRS		:  2-0,
				STIBP		:  4-2,
				SSBD		:  6-4,
				RDCL_NO 	:  8-6,
				IBRS_ALL	: 10-8,
				RSBA		: 12-10,
				L1DFL_VMENTRY_NO: 14-12,
				SSB_NO		: 16-14,
				MDS_NO		: 18-16,
				PSCHANGE_MC_NO	: 20-18,
				TAA_NO		: 22-20,
				_UnusedMechBits : 64-22;
	} Mechanisms;

	enum THERMAL_FORMULAS	thermalFormula;
	enum VOLTAGE_FORMULAS	voltageFormula;
	enum POWER_FORMULAS	powerFormula;

	struct {
		unsigned int		Count,
					OnLine;
	} CPU;

	SERVICE_PROC			Service;

	unsigned int			Boost[BOOST(SIZE)],
					PM_version;

	unsigned int			Top;

	unsigned int			Toggle;

	struct PKG_FLIP_FLOP {
		struct {
		unsigned long long	PTSC,
					PC02,
					PC03,
					PC06,
					PC07,
					PC08,
					PC09,
					PC10,
					ACCU[PWR_DOMAIN(SIZE)];
		} Delta;

		struct {
		unsigned long long	FC0;
		} Uncore;

		struct {
		unsigned int		Sensor,
					Temp;
		enum THERM_PWR_EVENTS	Events;
		} Thermal;
	} FlipFlop[2] __attribute__ ((aligned (8)));

	struct {
		double			PC02,
					PC03,
					PC06,
					PC07,
					PC08,
					PC09,
					PC10;
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
					C1;
	} Avg;

	struct {
		struct {
			double		Watts,
					Joules,
					Times;
		} Unit;
		unsigned int		TDP, Min, Max;
	} Power;

	enum HYPERVISOR 		HypervisorID;
	char				Brand[64],
					Architecture[CODENAME_LEN];
} PROC_STRUCT;

typedef struct
{
	FOOTPRINT		FootPrint;

	struct {	/*	NMI bits: 0 is Unregistered; 1 is Registered */
		Bit64		NMI	__attribute__ ((aligned (8)));
		signed int	AutoClock, /* 10: Auto, 01: Init, 00: Specs */
				Experimental,/* 0: Disable, 1: Enable	*/
				HotPlug, /* < 0: Disable, Other: Enable */
				PCI;	/*  < 0: Disable, other: Enable */
		KERNEL_DRIVER	Driver;
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

		MEM_MCB		memInfo;

		struct {
		unsigned short	version,
				major,
				minor;
		} kernel;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
	} SysGate;

	struct {
		unsigned int	Interval;
		struct timespec pollingWait,
				ringWaiting,
				childWaiting,
				sliceWaiting;
	} Sleep;

	struct {
		RING_CTRL	buffer[RING_SIZE] __attribute__((aligned(16)));
		unsigned int	head, tail;
	} Ring[2]; /* [0] Parent ; [1] Child				*/

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
		unsigned char	/* 00:MHz , 01:MT/s , 10:MB/s , 11:VOID */
					Bus_Rate: 2-0,
					BusSpeed: 4-2,
					DDR_Rate: 6-4,
					DDRSpeed: 8-6;
	    } Unit;

	    struct {
		enum CHIPSET		ArchID;
		char			CodeName[CODENAME_LEN];
	    } Chipset;
	} Uncore;

	SMBIOS_ST		SMB;

	PROC_STRUCT		Proc;
	CPU_STRUCT		Cpu[];
} SHM_STRUCT;


enum REASON_CLASS {
	RC_SUCCESS	= 0,
	RC_CMD_SYNTAX	= 1,
	RC_SHM_FILE	= 2,
	RC_SHM_MMAP	= 3,
	RC_PERM_ERR	= 4,
	RC_MEM_ERR	= 5,
	RC_EXEC_ERR	= 6,
	RC_SYS_CALL	= 15
};

typedef struct {
	__typeof__ (errno)	no: 32;
	__typeof__ (__LINE__)	ln: 28;
	enum REASON_CLASS	rc: 4;
} REASON_CODE;

#define REASON_SET_2xARG(_reason, _rc, _no)				\
({									\
	_reason.no = _no;						\
	_reason.ln = __LINE__;						\
	_reason.rc = _rc;						\
})

#define REASON_SET_1xARG(_reason, _rc)					\
({									\
	_reason.no = errno;						\
	_reason.ln = __LINE__;						\
	_reason.rc = _rc;						\
})

#define REASON_SET_0xARG(_reason)					\
({									\
	_reason.no = errno;						\
	_reason.ln = __LINE__;						\
	_reason.rc = RC_SYS_CALL;					\
})

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

