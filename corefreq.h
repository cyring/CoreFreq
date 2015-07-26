/*
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	TASK_COMM_LEN	16

#define	SHM_FILENAME	"corefreq-shm"

typedef struct
{
	unsigned long long Temperature;

	unsigned int	OffLine;

	unsigned int	FlipFlop;
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
	} State[2];

	struct {
		double	Ratio,
			Freq;
	} Relative;
} CPU_STRUCT;

typedef struct
{
	unsigned int		Q;
	unsigned long long	R;
} CLOCK;

typedef struct
{
/*	atomic_ullong		Sync,
				Room;
*/
	unsigned long long	Sync,
				Room;

	unsigned int		msleep;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	unsigned int		Boost[1+1+8],
				PerCore;

	CLOCK			Clock;

	char			Brand[64];

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
