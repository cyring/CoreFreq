/*
 * CoreFreq
 * Copyright (C) 2015-2025 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include "bitasm.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreqm.h"

#define DeltaTSC(pSlice)						\
({									\
	unsigned long long overhead = pSlice->Counter[1].TSC		\
				    - pSlice->Counter[0].TSC;		\
	pSlice->Delta.TSC = pSlice->Counter[2].TSC			\
			  - pSlice->Counter[1].TSC;			\
	if (overhead <= pSlice->Delta.TSC)				\
		pSlice->Delta.TSC -= overhead;				\
})

#define DeltaINST(pSlice)						\
({									\
	unsigned long long overhead = pSlice->Counter[1].INST		\
				    - pSlice->Counter[0].INST;		\
	/*	Test and compute if counter has overflowed	*/	\
	if (pSlice->Counter[2].INST >= pSlice->Counter[1].INST) 	\
		pSlice->Delta.INST  =  pSlice->Counter[2].INST		\
				    -  pSlice->Counter[1].INST; 	\
	else {								\
		pSlice->Delta.INST  = (INST_COUNTER_OVERFLOW + 0x1)	\
				    - pSlice->Counter[1].INST;		\
		pSlice->Delta.INST += pSlice->Counter[2].INST;		\
	}								\
	if (overhead <= pSlice->Delta.INST)				\
		pSlice->Delta.INST -= overhead; 			\
})

void CallWith_RDTSC_RDPMC(	RO(SHM_STRUCT) *RO(Shm),
				RW(SHM_STRUCT) *RW(Shm),
				unsigned int cpu,
				SLICE_FUNC SliceFunc,
				unsigned long arg )
{
	struct SLICE_STRUCT *pSlice = &RO(Shm)->Cpu[cpu].Slice;

	RDTSC_PMCx1(	pSlice->Counter[0].TSC,
			/*TODO:pmevcntr3_el0*/mcycle,
			pSlice->Counter[0].INST );

	pSlice->Counter[0].INST &= INST_COUNTER_OVERFLOW;

	RDTSC_PMCx1(	pSlice->Counter[1].TSC,
			/*TODO:pmevcntr3_el0*/mcycle,
			pSlice->Counter[1].INST );

	pSlice->Counter[1].INST &= INST_COUNTER_OVERFLOW;

	SliceFunc(RO(Shm), RW(Shm), cpu, arg);

	RDTSC_PMCx1(	pSlice->Counter[2].TSC,
			/*TODO:pmevcntr3_el0*/mcycle,
			pSlice->Counter[2].INST );

	pSlice->Counter[2].INST &= INST_COUNTER_OVERFLOW;

	if (BITVAL(RW(Shm)->Proc.Sync, BURN)) {
		DeltaTSC(pSlice);
		DeltaINST(pSlice);
	}
}

void CallWith_RDTSC_No_RDPMC(	RO(SHM_STRUCT) *RO(Shm),
				RW(SHM_STRUCT) *RW(Shm),
				unsigned int cpu,
				SLICE_FUNC SliceFunc,
				unsigned long arg )
{
	struct SLICE_STRUCT *pSlice = &RO(Shm)->Cpu[cpu].Slice;

	RDTSC64(pSlice->Counter[0].TSC);

	RDTSC64(pSlice->Counter[1].TSC);

	SliceFunc(RO(Shm), RW(Shm), cpu, arg);

	RDTSC64(pSlice->Counter[2].TSC);

	if (BITVAL(RW(Shm)->Proc.Sync, BURN)) {
		DeltaTSC(pSlice);
		pSlice->Delta.INST = 0;
	}
}

void Slice_NOP( RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
		unsigned int cpu, unsigned long arg )
{
	UNUSED(RO(Shm));
	UNUSED(RW(Shm));
	UNUSED(cpu);
	UNUSED(arg);

	__asm__ volatile
	(
		"nop"
		:
		:
		:
	);
}

void Slice_Atomic(RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
		unsigned int cpu, unsigned long arg)
{
	UNUSED(RW(Shm));
	register unsigned long loop = arg;
    do {
/*	__asm__ volatile
	(
		"ldr	x10,	%[_err]"		"\n\t"
		"1:"					"\n\t"
		"ldxr	x11,	[%[addr]]"		"\n\t"
		"mrs	x11,	cntvct_el0"		"\n\t"
		"stxr	w9,	x11, [%[addr]]" 	"\n\t"
		"cbz	w9,	2f"			"\n\t"
		"add	x10,	x10, #1"		"\n\t"
		"b	1b"				"\n\t"
		"2:"					"\n\t"
		"str	x10,	%[_err]"
		: [_err] "=m"	(RO(Shm)->Cpu[cpu].Slice.Error)
		: [addr] "r"	(&RO(Shm)->Cpu[cpu].Slice.Exclusive)
		: "cc", "memory", "%w9", "%x10", "%x11"
	);*/
    } while (loop-- != 0) ;
}

unsigned int CRC32vC(unsigned char *data, unsigned int len)
{
	unsigned int rem = 0, oloop = len, iloop;

	while (oloop--) {
		rem ^= *data++;
		for (iloop = 0; iloop < 8; iloop++)
			rem = (rem >> 1) ^ ((rem & 1) ? 0x436f7265 : 0);
	}
	return(rem);
}

void Slice_CRC32(RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
		unsigned int cpu, unsigned long arg)
{
	unsigned char *data = (unsigned char *) CRC32_SRC;
	unsigned int len = 16;
	UNUSED(RW(Shm));
	UNUSED(arg);

	if (CRC32vC(data, len) != CRC32_EXP)
		RO(Shm)->Cpu[cpu].Slice.Error++ ;
}

/*
	Source: 1996 Software graduate thesis
	Conic: a.X² + b.Y² + c.Z² = k
	Variations:
	    1 - l'ellipsoïde
	    2 - l'hyperboloïde à une nappe
	    3 - l'hyperboloïde à deux nappes
	    4 - le cylindre elliptique
	    5 - le cylindre hyperbolique
	    6 - deux plans parallèles
*/
void Slice_Conic(RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
		unsigned int cpu, unsigned long v)
{
	const double interval = (4.0 * 1024.0) - (double) cpu;
	const struct {
		double a, b, c, k;
	} p[CONIC_VARIATIONS] = {
		{.a=+1.0 , .b=+1.0, .c=-1.0 , .k=    +0.0},
		{.a=-3.0 , .b=+3.0, .c=-3.0 , .k=-17000.0},
		{.a=+0.01, .b=+0.7, .c=+0.2 , .k= +3000.0},
		{.a=+2.0 , .b=+0.0, .c=+1.0 , .k= +5000.0},
		{.a=-0.5 , .b=+0.0, .c=+0.75, .k=  +500.0},
		{.a=+0.0 , .b=+0.0, .c=+1.0 , .k= +3000.0}
	};
	double X, Y, Z, Q, k;

	const double step = (double) RO(Shm)->Proc.CPU.Count;

	UNUSED(RW(Shm));

	for (Y = -interval; Y <= interval; Y += step)
	    for (X = -interval; X <= interval; X += step) {
		Q=(p[v].k - p[v].a * pow(X,2.0) - p[v].b * pow(Y,2.0)) / p[v].c;
		Z = sqrt(Q);

		k = p[v].a*pow(X,2.0) + p[v].b*pow(Y,2.0) + p[v].c*pow(Z,2.0);

		if (fabs(k - p[v].k) > CONIC_ERROR)
			RO(Shm)->Cpu[cpu].Slice.Error++ ;
	    }
}

void Slice_Turbo(RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
		unsigned int cpu, unsigned long arg)
{
	UNUSED(arg);

	Slice_Atomic(RO(Shm), RW(Shm), cpu, TURBO_LOOP);
}

void Slice_Monte_Carlo(RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
			unsigned int cpu, unsigned long arg)
{
    if (RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.trials < PI_TRIALS)
    {
	double X, Y, Z;
	UNUSED(arg);
      #ifdef __GLIBC__
	if (!random_r(	&RO(Shm)->Cpu[cpu].Slice.Random.data,
			&RO(Shm)->Cpu[cpu].Slice.Random.value[0] )
	 && !random_r(	&RO(Shm)->Cpu[cpu].Slice.Random.data,
			&RO(Shm)->Cpu[cpu].Slice.Random.value[1] ))
      #else
	RO(Shm)->Cpu[cpu].Slice.Random.value[0] = (int) random();
	RO(Shm)->Cpu[cpu].Slice.Random.value[1] = (int) random();
      #endif /* __GLIBC__ */
	{
		X = (double) RO(Shm)->Cpu[cpu].Slice.Random.value[0] / RAND_MAX;
		Y = (double) RO(Shm)->Cpu[cpu].Slice.Random.value[1] / RAND_MAX;

		Z = pow(X, 2.0) + pow(Y, 2.0);

		if (Z <= 1.0) {
			RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.inside++;
		}
	}
	RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.trials++ ;
    } else {
	const double fi = (double) RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.inside,
		ft = RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.trials ?
		(double) RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.trials : 1.0;

	const double PI = (fi / ft) * 4.0;

	RO(Shm)->Cpu[cpu].Slice.Error += (fabs(PI - PI_CONST) > PI_ERROR) ?
		+1LLU : RO(Shm)->Cpu[cpu].Slice.Error > 0LLU ? -1LLU : 0LLU;

	RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.inside = 0LLU;
	RO(Shm)->Cpu[cpu].Slice.Monte_Carlo.trials = 0LLU;
    }
}

RING_SLICE order_list[] = {
    {
	.ctrl = {
	.cmd = COREFREQ_ORDER_ATOMIC, .sub = 0x0U,
	.dl = {.lo = 1	, .hi = COREFREQ_TOGGLE_ON},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Atomic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd=COREFREQ_ORDER_CRC32, .sub = 0x0U,
	.dl = {.lo = 1	, .hi = COREFREQ_TOGGLE_ON},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_CRC32, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_CONIC, .sub = CONIC_ELLIPSOID,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Conic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_CONIC, .sub = CONIC_HYPERBOLOID_ONE_SHEET,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Conic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_CONIC, .sub = CONIC_HYPERBOLOID_TWO_SHEETS,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Conic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_CONIC, .sub = CONIC_ELLIPTICAL_CYLINDER,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Conic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_CONIC, .sub = CONIC_HYPERBOLIC_CYLINDER,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Conic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_CONIC, .sub = CONIC_TWO_PARALLEL_PLANES,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Conic, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_TURBO, .sub = RAND_SMT,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Turbo, .pattern = RAND_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_TURBO, .sub = RR_SMT,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Turbo, .pattern = RR_SMT
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_TURBO, .sub = USR_CPU,
	.dl = {.lo = 1	, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Turbo, .pattern = USR_CPU
    },{
	.ctrl = {
	.cmd = COREFREQ_ORDER_MONTE_CARLO, .sub = 0x0U,
	.dl = {.lo = 0x0, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = Slice_Monte_Carlo, .pattern = ALL_SMT
    },{
	.ctrl = {
	.cmd = 0x0U, .sub = 0x0U,
	.dl = {.lo = 0x0, .hi = 0x0},
	.dh = {.lo = 0x0, .hi = 0x0}
	},
	.func = NULL
    }
};
