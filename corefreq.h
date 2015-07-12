/*
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	SHM_FILENAME	"corefreq-shm"
#define	PAGE_SIZE	(sysconf(_SC_PAGESIZE))

typedef struct
{
	atomic_ullong	Sync;

	double		IPS,
			IPC,
			CPI,
			Turbo,
			C0,
			C3,
			C6,
			C7,
			C1;

	struct {
		double	Ratio,
			Freq;
	} Relative;

	unsigned long long Temperature;
} CPU_STRUCT;


typedef	struct
{
	char		AppName[TASK_COMM_LEN];
	CPU_STRUCT	CPU[];
} SHM_STRUCT;
