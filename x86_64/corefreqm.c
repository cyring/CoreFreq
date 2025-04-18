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
	pSlice->Delta.INST = pSlice->Counter[2].INST			\
			   - pSlice->Counter[1].INST;			\
	if (overhead <= pSlice->Delta.INST)				\
		pSlice->Delta.INST -= overhead;				\
})

void CallWith_RDTSCP_RDPMC(	RO(SHM_STRUCT) *RO(Shm),
				RW(SHM_STRUCT) *RW(Shm),
				unsigned int cpu,
				SLICE_FUNC SliceFunc,
				unsigned long arg )
{
	struct SLICE_STRUCT *pSlice = &RO(Shm)->Cpu[cpu].Slice;

	RDTSCP_PMCx1(pSlice->Counter[0].TSC,0x40000000,pSlice->Counter[0].INST);

	RDTSCP_PMCx1(pSlice->Counter[1].TSC,0x40000000,pSlice->Counter[1].INST);

	SliceFunc(RO(Shm), RW(Shm), cpu, arg);

	RDTSCP_PMCx1(pSlice->Counter[2].TSC,0x40000000,pSlice->Counter[2].INST);

	if (BITVAL(RW(Shm)->Proc.Sync, BURN)) {
		DeltaTSC(pSlice);
		DeltaINST(pSlice);
	}
}

void CallWith_RDTSC_RDPMC(	RO(SHM_STRUCT) *RO(Shm),
				RW(SHM_STRUCT) *RW(Shm),
				unsigned int cpu,
				SLICE_FUNC SliceFunc,
				unsigned long arg )
{
	struct SLICE_STRUCT *pSlice = &RO(Shm)->Cpu[cpu].Slice;

	RDTSC_PMCx1(pSlice->Counter[0].TSC,0x40000000,pSlice->Counter[0].INST);

	RDTSC_PMCx1(pSlice->Counter[1].TSC,0x40000000,pSlice->Counter[1].INST);

	SliceFunc(RO(Shm), RW(Shm), cpu, arg);

	RDTSC_PMCx1(pSlice->Counter[2].TSC,0x40000000,pSlice->Counter[2].INST);

	if (BITVAL(RW(Shm)->Proc.Sync, BURN)) {
		DeltaTSC(pSlice);
		DeltaINST(pSlice);
	}
}

void CallWith_RDTSCP_No_RDPMC(	RO(SHM_STRUCT) *RO(Shm),
				RW(SHM_STRUCT) *RW(Shm),
				unsigned int cpu,
				SLICE_FUNC SliceFunc,
				unsigned long arg )
{
	struct SLICE_STRUCT *pSlice = &RO(Shm)->Cpu[cpu].Slice;

	RDTSCP64(pSlice->Counter[0].TSC);

	RDTSCP64(pSlice->Counter[1].TSC);

	SliceFunc(RO(Shm), RW(Shm), cpu, arg);

	RDTSCP64(pSlice->Counter[2].TSC);

	if (BITVAL(RW(Shm)->Proc.Sync, BURN)) {
		DeltaTSC(pSlice);
		pSlice->Delta.INST = 0;
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
	__asm__ volatile
	(
		"movq %[_atom], %%r14"	"\n\t"
		"movq %[_xchg], %%r15"	"\n\t"
		"movq %[_loop], %%rcx"	"\n\t"
		"movq %[i_err], %%r11"	"\n\t"
		"1:"			"\n\t"
		"movq	%%r14, %%r12"	"\n\t"
		"movq	%%r15, %%r13"	"\n\t"
		"xchg	%%r12, %%r13"	"\n\t"
		"cmpq	%%r13, %%r14"	"\n\t"
		"jz 2f"			"\n\t"
		"incq	%%r11"		"\n\t"
		"2:"			"\n\t"
		"loop	1b"		"\n\t"
		"movq	%%r11, %[o_err]"
		: [o_err] "=m" (RO(Shm)->Cpu[cpu].Slice.Error)
		: [_loop] "m" (arg),
		  [_atom] "i" (SLICE_ATOM),
		  [_xchg] "i" (SLICE_XCHG),
		  [i_err] "m" (RO(Shm)->Cpu[cpu].Slice.Error)
		: "%rcx", "%r11", "%r12", "%r13", "%r14", "%r15",
		  "cc", "memory"
	);
}

#define CRC32vASM(data, len)						\
({									\
	unsigned int rem = 0;						\
	__asm__ (							\
		"	movl	%[_len], %%r8d"		"\n\t"		\
		"	movq	%[_data], %%r10"	"\n\t"		\
		"	addq	%%r10, %%r8"		"\n\t"		\
		"	movl	$0, %%r12d"		"\n\t"		\
		"	movl	$0x436f7265, %%r9d"	"\n\t"		\
		"1:"					"\n\t"		\
		"	cmpq	%%r8, %%r10"		"\n\t"		\
		"	je	3f"			"\n\t"		\
		"	addq	$1, %%r10"		"\n\t"		\
		"	movzbl	-1(%%r10), %%edx"	"\n\t"		\
		"	xorl	%%edx, %%r12d"		"\n\t"		\
		"	movl	$8, %%edx"		"\n\t"		\
		"2:"					"\n\t"		\
		"	movl	%%r12d, %%r11d"		"\n\t"		\
		"	shrl	%%r11d"			"\n\t"		\
		"	andl	$1, %%r12d"		"\n\t"		\
		"	cmovne	%%r9d, %%r12d"		"\n\t"		\
		"	xorl	%%r11d, %%r12d"		"\n\t"		\
		"	subl	$1, %%edx"		"\n\t"		\
		"	jne	2b"			"\n\t"		\
		"	jmp	1b"			"\n\t"		\
		"3:"					"\n\t"		\
		"	movl	%%r12d, %[_rem]"			\
		: [_rem] "+m" (rem)					\
		: [_data] "m" (data),					\
		  [_len] "im" (len)					\
		: "%rdx", "%r8", "%r9", "%r10", "%r11", "%r12" ,	\
		  "cc", "memory"					\
	);								\
	rem;								\
})

void Slice_CRC32(RO(SHM_STRUCT) *RO(Shm), RW(SHM_STRUCT) *RW(Shm),
		unsigned int cpu, unsigned long arg)
{
	unsigned char *data = (unsigned char *) CRC32_SRC;
	unsigned int len = 16;
	UNUSED(RW(Shm));
	UNUSED(arg);

	if (CRC32vASM(data, len) != CRC32_EXP)
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
