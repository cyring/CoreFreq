/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	SHM_FILENAME	"corefreq-shm"

typedef struct
{
	Bit64				OffLine __attribute__ ((aligned (64)));

	CLOCK				Clock;

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

	CPUID_STRUCT			CpuID[CPUID_MAX_FUNC];

	struct {
		unsigned int		ApicID,
					CoreID,
					ThreadID;
		struct {
			Bit32		BSP,
					x2APIC;
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
					Target,
					Limit[2];
		struct {
			unsigned int	ClockMod : 16-0,
					Extended : 32-16;
			} DutyCycle;
		unsigned int		PowerPolicy;
	} PowerThermal;

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
		} Delta;

		struct {
			double		IPS,
					IPC,
					CPI,
					Turbo,
					C0,
					C3,
					C6,
					C7,
					C1;
		} State;

		struct {
			double		Ratio,
					Freq;
		} Relative;

		struct {
		unsigned int		Sensor,
					Temp,
					Trip;
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
} CPU_STRUCT;

typedef struct
{
	volatile unsigned long long	Sync,
					Room __attribute__ ((aligned (128)));

	FEATURES			Features;

	Bit64			PowerNow	__attribute__ ((aligned (64)));
	Bit64			ODCM		__attribute__ ((aligned (64)));
	Bit64			ODCM_Mask	__attribute__ ((aligned (64)));
	Bit64			PowerMgmt	__attribute__ ((aligned (64)));
	Bit64			PowerMgmt_Mask	__attribute__ ((aligned (64)));
	Bit64			SpeedStep	__attribute__ ((aligned (64)));
	Bit64			SpeedStep_Mask	__attribute__ ((aligned (64)));
	Bit64			TurboBoost	__attribute__ ((aligned (64)));
	Bit64			TurboBoost_Mask __attribute__ ((aligned (64)));
	Bit64			C1E		__attribute__ ((aligned (64)));
	Bit64			C1E_Mask	__attribute__ ((aligned (64)));
	Bit64			C3A		__attribute__ ((aligned (64)));
	Bit64			C3A_Mask	__attribute__ ((aligned (64)));
	Bit64			C1A		__attribute__ ((aligned (64)));
	Bit64			C1A_Mask	__attribute__ ((aligned (64)));
	Bit64			C3U		__attribute__ ((aligned (64)));
	Bit64			C3U_Mask	__attribute__ ((aligned (64)));
	Bit64			C1U		__attribute__ ((aligned (64)));
	Bit64			C1U_Mask	__attribute__ ((aligned (64)));

	unsigned int			SleepInterval;
	struct timespec			BaseSleep;

	struct {
		unsigned int		Count,
					OnLine;
	} CPU;

	unsigned int			Boost[BOOST(SIZE)],
					PM_version;

	unsigned int			Top;

	unsigned int			Toggle;

	struct PKG_FLIP_FLOP {
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
		} Delta;
		struct
		{
		unsigned long long	FC0;
		} Uncore;
	} FlipFlop[2];

	struct {
		double			PC02,
					PC03,
					PC06,
					PC07,
					PC08,
					PC09,
					PC10;
	} State;

	struct {
		double			Turbo,
					C0,
					C3,
					C6,
					C7,
					C1;
	} Avg;

	char				Brand[64],
					Architecture[32];
} PROC_STRUCT;

typedef struct
{
	struct {
		signed int	Experimental,	// 0: Disable, 1: Enable
				hotplug,	// < 0: Disable, Other: Enable
				pci,		// < 0: Disable, other: Enable
				nmi;		// <> 0: Failed, == 0: Enable
	} Registration;

	struct {
		Bit64		Operation	__attribute__ ((aligned (64)));

		IDLEDRIVER	IdleDriver;

		unsigned int	tickReset,
				tickStep;

		pid_t		trackTask;
		int		sortByField,
				reverseOrder,
				taskCount;
		TASK_MCB	taskList[PID_MAX_DEFAULT];

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
		struct RING_CTRL {
			unsigned long	arg;
			unsigned int	cmd;
		} buffer[RING_SIZE];
		unsigned int		head, tail;
	} Ring;

	char				AppName[TASK_COMM_LEN];

	struct {
		unsigned int		Boost[2];
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
	unsigned short			CtrlCount;

	    struct {
		unsigned char		// 00:MHz , 01:MT/s , 10:MB/s , 11:VOID
					Bus_Rate: 2-0,
					BusSpeed: 4-2,
					DDR_Rate: 6-4,
					DDRSpeed: 8-6;
	    } Unit;
	} Uncore;

	PROC_STRUCT		Proc;
	CPU_STRUCT		Cpu[];
} SHM_STRUCT;
