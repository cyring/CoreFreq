/*
 * CoreFreq
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	TASK_COMM_LEN	16

#define	SHM_FILENAME	"corefreq-shm"

typedef struct
{
	unsigned int		OffLine;

	unsigned int		Toggle;

	struct {
		int		BSP,
				ApicID,
				CoreID,
				ThreadID,
				x2APIC,
				Enable;
		struct {
		unsigned int	Size;
		} Cache[CACHE_MAX_LEVEL];
	} Topology;

	struct FLIP_FLOP {
	    unsigned long long	Temperature;

		struct {
			double	IPS,
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
			double	Ratio,
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

	CLOCK				Clock;

	unsigned char			Architecture[32];
	unsigned int			Boost[1+1+8],
					PerCore;

	char				Brand[64];

	struct {
		double	Turbo,
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
	PROC_STRUCT	Proc;
	CPU_STRUCT	Cpu[];
} SHM_STRUCT;
