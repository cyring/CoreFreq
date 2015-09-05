/*
 * CoreFreq
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	DRV_DEVNAME "intelfreq"
#define	DRV_FILENAME "/dev/"DRV_DEVNAME

#define	PRECISION	100

typedef struct
{
	unsigned int		Q;
	unsigned long long	R;
} CLOCK;
