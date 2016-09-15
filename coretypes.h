/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	TRUE	1
#define	FALSE	0

typedef unsigned long long int	Bit64;
typedef unsigned int		Bit32;

#define MAX(M, m)	((M) > (m) ? (M) : (m))
#define MIN(m, M)	((m) < (M) ? (m) : (M))

#define	powered(bit)	((bit) ? 'Y' : 'N')
#define	enabled(bit)	((bit) ? "ON" : "OFF")

#define	DRV_DEVNAME	"corefreqk"
#define	DRV_FILENAME	"/dev/"DRV_DEVNAME

#define	PRECISION	100

#define	CACHE_MAX_LEVEL	(3 + 1)

typedef struct
{
	unsigned int		Q;
	unsigned long long	R;
	unsigned long long	Hz;
} CLOCK;

#define	REL_FREQ(max_ratio, this_ratio, clock)		\
		( ((this_ratio * clock.Q) * 1000000L)	\
		+ ((this_ratio * clock.R) / max_ratio))
