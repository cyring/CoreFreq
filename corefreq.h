/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	TASK_COMM_LEN		16
#define CPUIDLE_STATE_MAX       10
#define CPUIDLE_NAME_LEN        16

#define	SHM_FILENAME	"corefreq-shm"

typedef struct
{
	unsigned int			OffLine;

	CLOCK				Clock;

	unsigned int			Toggle;

	struct {
		Bit32			BSP,
					ApicID,
					CoreID,
					ThreadID,
					x2APIC;
		struct {
		unsigned short int	LineSz,
					Part,
					Way;
		unsigned int		Set,
					Size;
		} Cache[CACHE_MAX_LEVEL];
	} Topology;

	unsigned int			C1E,
					C3A,		// Nehalem
					C1A,		// Nehalem
					C3U,		// Sandy Bridge
					C1U;		// Sandy Bridge

	struct {
	    	unsigned int		TM1,
					TM2;
	} Thermal;

	struct FLIP_FLOP {
		struct {
	    	unsigned int		Target,
					Sensor,
					Temp,
					Trip;
		} Thermal;

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
	} FlipFlop[2];
} CPU_STRUCT;

typedef struct
{
	volatile unsigned long long	Sync,
					Room;

	unsigned int			msleep;

	struct {
		unsigned int		Count,
					OnLine;
	} CPU;

	union
	{
		struct {
			unsigned int
					Stepping	:  4-0,
					Model		:  8-4,
					Family		: 12-8,
					ProcType	: 14-12,
					Unused1		: 16-14,
					ExtModel	: 20-16,
					ExtFamily	: 28-20,
					Unused2		: 32-28;
		};
		unsigned int Signature;
	};
	unsigned char			Architecture[32];
	unsigned int			Boost[1+1+8],
					PM_version;

	char				Brand[64];

	Bit32				InvariantTSC,
					HyperThreading;
	Bit64				SpeedStep,
					TurboBoost;
	struct {
		double			Turbo,
					C0,
					C3,
					C6,
					C7,
					C1;
	} Avg;
} PROC_STRUCT;


typedef	struct
{
	char		AppName[TASK_COMM_LEN];
	struct {
		char	Name[CPUIDLE_NAME_LEN];
		int	stateCount;
	  	struct {
			char	Name[CPUIDLE_NAME_LEN];
		unsigned int	exitLatency;		/* in US */
			int	powerUsage;		/* in mW */
		unsigned int	targetResidency;	/* in US */
		} State[CPUIDLE_STATE_MAX];
	} IdleDriver;
	PROC_STRUCT	Proc;
	CPU_STRUCT	Cpu[];
} SHM_STRUCT;
