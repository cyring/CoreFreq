/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	SHM_FILENAME	"corefreq-shm"

typedef struct
{
	OFFLINE				OffLine;

	CLOCK				Clock;

	unsigned int			Toggle;

	struct
	{
		CPUID_0x00000000	StdFunc;
		CPUID_0x80000000	ExtFunc;
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
		float			ODCM;
		unsigned int		PowerPolicy;
	} PowerThermal;

	struct FLIP_FLOP {
		struct
		{
		unsigned long long
					INST;
			struct
			{
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
	} FlipFlop[2];
} CPU_STRUCT;

typedef struct
{
	volatile unsigned long long	Sync,
					Room;

	FEATURES			Features;

	unsigned int			SleepInterval;
	struct timespec			BaseSleep;

	struct {
		unsigned int		Count,
					OnLine;
	} CPU;

	unsigned int			Boost[1+1+8],
					PM_version;

	Bit32				InvariantTSC,
					HyperThreading;

	Bit64				PowerNow,
					SpeedStep,
					SpeedStep_Mask,
					TurboBoost,
					TurboBoost_Mask,
					C1E,
					C1E_Mask,
					C3A,		// Nehalem
					C3A_Mask,
					C1A,		// Nehalem
					C1A_Mask,
					C3U,		// Sandy Bridge
					C3U_Mask,
					C1U,		// Sandy Bridge;
					C1U_Mask;

	unsigned int			Top;

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
		Bit64		Operation;

		IDLEDRIVER	IdleDriver;

		pid_t		trackTask;
		int		sortByField,
				reverseOrder,
				taskCount;
		TASK_MCB	taskList[PID_MAX_DEFAULT];

		MEM_MCB		memInfo;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
	} SysGate;

	char			AppName[TASK_COMM_LEN];

	struct {
		struct
		{
			unsigned int		Rate,
						Unit;	// 0: MHz , 1: MT/s
			unsigned long long	Speed;
		} Bus;

		struct {
			struct {
				RAM_TIMING	Timing;
			} Channel[MC_MAX_CHA];
			unsigned short		ChannelCount;
		} MC[MC_MAX_CTRL];
		unsigned short			CtrlCount;
		unsigned long long		CtrlSpeed;
	} Uncore;

	PROC_STRUCT		Proc;
	CPU_STRUCT		Cpu[];
} SHM_STRUCT;
