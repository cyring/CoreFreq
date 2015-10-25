/*
 * CoreFreq
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	DRV_DEVNAME "intelfreq"
#define	DRV_FILENAME "/dev/"DRV_DEVNAME

#define	PRECISION	100

#define	CACHE_MAX_LEVEL	3

typedef struct
{
	unsigned int		Q;
	unsigned long long	R;
	unsigned long long	Hz;
} CLOCK;

#define	REL_FREQ(max_ratio, this_ratio, clock)		\
		( ((this_ratio * clock.Q) * 1000000L)	\
		+ ((this_ratio * clock.R) / max_ratio))
