/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/utsname.h>
#include <linux/cpuidle.h>
#include <asm/msr.h>

#include "coretypes.h"
#include "bitasm.h"
#include "intelmsr.h"
#include "amdmsr.h"
#include "corefreq-api.h"
#include "corefreqk.h"

MODULE_AUTHOR ("CYRIL INGENIERIE <labs[at]cyring[dot]fr>");
MODULE_DESCRIPTION ("CoreFreq Processor Driver");
MODULE_SUPPORTED_DEVICE ("Intel Core Core2 Atom Xeon i3 i5 i7, AMD Family 0Fh");
MODULE_LICENSE ("GPL");

typedef struct {
	FEATURES	features;
	unsigned int	count;
} ARG;

static struct {
	signed int	Major;
	struct cdev	*kcdev;
	dev_t		nmdev, mkdev;
	struct class	*clsdev;
} CoreFreqK;

static signed int ArchID = -1;
module_param(ArchID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ArchID, "Force an architecture (ID)");

static signed int AutoClock = 1;
module_param(AutoClock, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(AutoClock, "Auto estimate the clock frequency");

static unsigned int SleepInterval = 0;
module_param(SleepInterval, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SleepInterval, "Sleep interval (ms) of the loops");

static unsigned int Experimental = 0;
module_param(Experimental, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Experimental, "Enable features under development");

static PROC *Proc = NULL;
static KPUBLIC *KPublic = NULL;
static KPRIVATE *KPrivate = NULL;
static ktime_t RearmTheTimer;


unsigned int Intel_Brand(char *pBrand)
{
	char idString[64] = {0x20};
	unsigned int ix = 0, jx = 0, px = 0;
	unsigned int frequency = 0, multiplier = 0;
	BRAND Brand;

	for (ix = 0; ix < 3; ix++) {
		asm volatile
		(
			"cpuid"
			: "=a"  (Brand.AX),
			  "=b"  (Brand.BX),
			  "=c"  (Brand.CX),
			  "=d"  (Brand.DX)
			:  "a"   (0x80000002 + ix)
		);
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.AX.Chr[jx];
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.BX.Chr[jx];
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.CX.Chr[jx];
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.DX.Chr[jx];
	}
	for (ix = 0; ix < 46; ix++)
		if ((idString[ix+1] == 'H') && (idString[ix+2] == 'z')) {
			switch (idString[ix]) {
			case 'M':
					multiplier = 1;
				break;
			case 'G':
					multiplier = 1000;
				break;
			case 'T':
					multiplier = 1000000;
				break;
			}
			break;
		}
	if (multiplier > 0) {
	    if (idString[ix-3] == '.') {
		frequency  = (int) (idString[ix-4] - '0') * multiplier;
		frequency += (int) (idString[ix-2] - '0') * (multiplier / 10);
		frequency += (int) (idString[ix-1] - '0') * (multiplier / 100);
	    } else {
		frequency  = (int) (idString[ix-4] - '0') * 1000;
		frequency += (int) (idString[ix-3] - '0') * 100;
		frequency += (int) (idString[ix-2] - '0') * 10;
		frequency += (int) (idString[ix-1] - '0');
		frequency *= frequency;
	    }
	}
	for (ix = jx = 0; jx < 48; jx++)
		if (!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++] = idString[jx];

	return(frequency);
}

void AMD_Brand(char *pBrand)
{
	char idString[64] = {0x20};
	unsigned int ix = 0, jx = 0, px = 0;
	BRAND Brand;

	for (ix = 0; ix < 3; ix++) {
		asm volatile
		(
			"cpuid"
			: "=a"  (Brand.AX),
			  "=b"  (Brand.BX),
			  "=c"  (Brand.CX),
			  "=d"  (Brand.DX)
			:  "a"   (0x80000002 + ix)
		);
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.AX.Chr[jx];
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.BX.Chr[jx];
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.CX.Chr[jx];
		for (jx = 0; jx < 4; jx++, px++)
			idString[px] = Brand.DX.Chr[jx];
	}
	for (ix = jx = 0; jx < 48; jx++)
		if (!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++] = idString[jx];
}

// Retreive the Processor(BSP) features through calls to the CPUID instruction.
void Query_Features(void *pArg)
{
	ARG *arg = (ARG *) pArg;

	unsigned int eax = 0x0, ebx = 0x0, ecx = 0x0, edx = 0x0; // DWORD Only!

	// Must have x86 CPUID 0x0, 0x1, and Intel CPUID 0x4
	asm volatile
	(
		"cpuid"
		: "=a" (arg->features.Info.LargestStdFunc),
		  "=b" (ebx),
		  "=c" (ecx),
		  "=d" (edx)
		:  "a" (0x0)
	);
	arg->features.Info.VendorID[ 0] = ebx;
	arg->features.Info.VendorID[ 1] = (ebx >> 8);
	arg->features.Info.VendorID[ 2] = (ebx >> 16);
	arg->features.Info.VendorID[ 3] = (ebx >> 24);
	arg->features.Info.VendorID[ 4] = edx;
	arg->features.Info.VendorID[ 5] = (edx >> 8);
	arg->features.Info.VendorID[ 6] = (edx >> 16);
	arg->features.Info.VendorID[ 7] = (edx >> 24);
	arg->features.Info.VendorID[ 8] = ecx;
	arg->features.Info.VendorID[ 9] = (ecx >> 8);
	arg->features.Info.VendorID[10] = (ecx >> 16);
	arg->features.Info.VendorID[11] = (ecx >> 24);
	arg->features.Info.VendorID[12] = '\0';

	asm volatile
	(
		"cpuid"
		: "=a" (arg->features.Std.AX),
		  "=b" (arg->features.Std.BX),
		  "=c" (arg->features.Std.CX),
		  "=d" (arg->features.Std.DX)
		:  "a" (0x1)
	);
	if (arg->features.Info.LargestStdFunc >= 0x5) {
		asm volatile
		(
			"cpuid"
			: "=a" (arg->features.MWait.AX),
			  "=b" (arg->features.MWait.BX),
			  "=c" (arg->features.MWait.CX),
			  "=d" (arg->features.MWait.DX)
			:  "a" (0x5)
		);
	}
	if (arg->features.Info.LargestStdFunc >= 0x6) {
		asm volatile
		(
			"cpuid"
			: "=a" (arg->features.Power.AX),
			  "=b" (arg->features.Power.BX),
			  "=c" (arg->features.Power.CX),
			  "=d" (arg->features.Power.DX)
			:  "a" (0x6)
		);
	}
	if (arg->features.Info.LargestStdFunc >= 0x7) {
		asm volatile
		(
			"movq	$0x7, %%rax	\n\t"
			"xorq	%%rbx, %%rbx    \n\t"
			"xorq	%%rcx, %%rcx    \n\t"
			"xorq	%%rdx, %%rdx    \n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (arg->features.ExtFeature.AX),
			  "=r" (arg->features.ExtFeature.BX),
			  "=r" (arg->features.ExtFeature.CX),
			  "=r" (arg->features.ExtFeature.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	// Must have 0x80000000, 0x80000001, 0x80000002, 0x80000003, 0x80000004
	asm volatile
	(
		"cpuid"
		: "=a" (arg->features.Info.LargestExtFunc)
		:  "a" (0x80000000)
	);
	asm volatile
	(
		"cpuid"
		: "=c" (arg->features.ExtInfo.CX),
		  "=d" (arg->features.ExtInfo.DX)
		:  "a" (0x80000001)
	);
	if (arg->features.Info.LargestExtFunc >= 0x80000007) {
		asm volatile
		(
			"cpuid"
			: "=a" (arg->features.AdvPower.AX),
			  "=b" (arg->features.AdvPower.BX),
			  "=c" (arg->features.AdvPower.CX),
			  "=d" (arg->features.AdvPower.DX)
			:  "a" (0x80000007)
		);
	}

	// Reset the performance features bits (present is zero)
	arg->features.PerfMon.BX.CoreCycles    = 1;
	arg->features.PerfMon.BX.InstrRetired  = 1;
	arg->features.PerfMon.BX.RefCycles     = 1;
	arg->features.PerfMon.BX.LLC_Ref       = 1;
	arg->features.PerfMon.BX.LLC_Misses    = 1;
	arg->features.PerfMon.BX.BranchRetired = 1;
	arg->features.PerfMon.BX.BranchMispred = 1;

	// Per Vendor features
	if (!strncmp(arg->features.Info.VendorID, VENDOR_INTEL, 12)) {
		asm volatile
		(
			"cpuid"
			: "=a" (eax),
			  "=b" (ebx),
			  "=c" (ecx),
			  "=d" (edx)
			:  "a" (0x4),
			   "c" (0x0)
		);
		arg->count = (eax >> 26) & 0x3f;
		arg->count++;

	    if (arg->features.Info.LargestStdFunc >= 0xa) {
		asm volatile
		(
			"cpuid"
			: "=a" (arg->features.PerfMon.AX),
			  "=b" (arg->features.PerfMon.BX),
			  "=c" (arg->features.PerfMon.CX),
			  "=d" (arg->features.PerfMon.DX)
			:  "a" (0xa)
		);
	    }
	    arg->features.FactoryFreq = Intel_Brand(arg->features.Info.Brand);

	} else if (!strncmp(arg->features.Info.VendorID, VENDOR_AMD, 12)) {

		if (arg->features.Std.DX.HTT)
			arg->count = arg->features.Std.BX.MaxThread;
		else {
			if (arg->features.Info.LargestExtFunc >= 0x80000008) {
				asm volatile
				(
					"cpuid"
					: "=a" (eax),
					  "=b" (ebx),
					  "=c" (ecx),
					  "=d" (edx)
					:  "a" (0x80000008)
				);
				arg->count = (ecx & 0xf) + 1;
			}
		}
		AMD_Brand(arg->features.Info.Brand);
	}
}


typedef struct {			// V[0] stores previous TSC
	unsigned long long V[2];	// V[1] stores current TSC
} TSC_STRUCT;

#define OCCURRENCES 4
// OCCURRENCES x 2 (TSC values) needs a 64-byte cache line size.
#define STRUCT_SIZE (OCCURRENCES * sizeof(TSC_STRUCT))

void Compute_Clock(void *arg)
{
	CLOCK *clock = (CLOCK *) arg;
	unsigned int ratio = clock->Q;
	// Allocate Cache aligned resources.
	struct kmem_cache *hardwareCache = kmem_cache_create(
				"CoreFreqCache",
				STRUCT_SIZE, 0,
				SLAB_HWCACHE_ALIGN, NULL);
	/*
		TSC[0] stores the overhead
		TSC[1] stores the estimation
	*/
	TSC_STRUCT *TSC[2] = {
		kmem_cache_alloc(hardwareCache, GFP_KERNEL),
		kmem_cache_alloc(hardwareCache, GFP_KERNEL)
	};
	unsigned long long D[2][OCCURRENCES];
	unsigned int loop = 0, what = 0, best[2] = {0, 0}, top[2] = {0, 0};

	void ComputeWithSerializedTSC(void)
	{
		// No preemption, no interrupt.
		unsigned long flags;
		preempt_disable();
		raw_local_irq_save(flags);

		// Warm-up & Overhead
		for (loop = 0; loop < OCCURRENCES; loop++) {
			RDTSCP64(TSC[0][loop].V[0]);

			udelay(0);

			RDTSCP64(TSC[0][loop].V[1]);
		}

		// Estimation
		for (loop=0; loop < OCCURRENCES; loop++) {
			RDTSCP64(TSC[1][loop].V[0]);

			udelay(1000);

			RDTSCP64(TSC[1][loop].V[1]);
		}
		// Restore preemption and interrupt.
		raw_local_irq_restore(flags);
		preempt_enable();
	}

	void ComputeWithUnSerializedTSC(void)
	{
		// No preemption, no interrupt.
		unsigned long flags;
		preempt_disable();
		raw_local_irq_save(flags);

		// Warm-up & Overhead
		for (loop=0; loop < OCCURRENCES; loop++) {
			RDTSC64(TSC[0][loop].V[0]);

			udelay(0);

			RDTSC64(TSC[0][loop].V[1]);
		}
		// Estimation
		for (loop = 0; loop < OCCURRENCES; loop++) {
			RDTSC64(TSC[1][loop].V[0]);

			udelay(1000);

			RDTSC64(TSC[1][loop].V[1]);
		}
		// Restore preemption and interrupt.
		raw_local_irq_restore(flags);
		preempt_enable();
	}
	// Is the TSC invariant or a serialized read instruction is available ?
	if (	(Proc->Features.AdvPower.DX.Inv_TSC == 1)
		|| (Proc->Features.ExtInfo.DX.RDTSCP == 1))
			ComputeWithSerializedTSC();
	else
			ComputeWithUnSerializedTSC();

	// Select the best clock.
	memset(D, 0, 2 * OCCURRENCES);
	for (loop = 0; loop < OCCURRENCES; loop++)
		for (what = 0; what < 2; what++) {
			D[what][loop] 	= TSC[what][loop].V[1]
					- TSC[what][loop].V[0];
		}
	for (loop = 0; loop < OCCURRENCES; loop++) {
		unsigned int inner = 0, count[2] = {0, 0};
		for (inner = loop; inner < OCCURRENCES; inner++) {
			for (what = 0; what < 2; what++) {
				if (D[what][loop] == D[what][inner])
					count[what]++;
			}
		}
		for (what = 0; what < 2; what++) {
			if (	(count[what] > top[what])
				|| ((count[what] == top[what])
				&& (D[what][loop] < D[what][best[what]]))) {

					top[what]  = count[what];
					best[what] = loop;

			}
		}
	}
	// Substract the overhead.
	D[1][best[1]] -= D[0][best[0]];
	D[1][best[1]] *= 1000;
	// Compute Divisor and Remainder.
	clock->Q = D[1][best[1]] / (1000000L * ratio);
	clock->R = D[1][best[1]] % (1000000L * ratio);
	// Compute full Hertz.
	clock->Hz  = D[1][best[1]] / ratio;
	clock->Hz += D[1][best[1]] % ratio;
	// Release resources.
	kmem_cache_free(hardwareCache, TSC[1]);
	kmem_cache_free(hardwareCache, TSC[0]);
	kmem_cache_destroy(hardwareCache);
}

CLOCK Base_Clock(unsigned int cpu, unsigned int ratio)
{
	CLOCK clock = {.Q = ratio, .R = 0, .Hz = 0};

	smp_call_function_single(cpu,
				Compute_Clock,
				&clock,
				1); // Synchronous call.
	return(clock);
}

void ClockToHz(CLOCK *clock)
{
	clock->Hz  = clock->Q * 1000000L;
	clock->Hz += clock->R * PRECISION;
}

// [Genuine Intel]
CLOCK Clock_GenuineIntel(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};

	if (Proc->Features.FactoryFreq > 0) {
		clock.Hz = (Proc->Features.FactoryFreq * 1000000L) / ratio;
		clock.Q  = Proc->Features.FactoryFreq / ratio;
		clock.R  = (Proc->Features.FactoryFreq % ratio) * PRECISION;
	}
	return(clock);
};

// [Authentic AMD]
CLOCK Clock_AuthenticAMD(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L}; // AMD assumption
	return(clock);
};

// [Core]
CLOCK Clock_Core(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	FSB_FREQ FSB={.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed) {
	case 0b101: {
		clock.Q = 100;
		clock.R = 0;
		};
		break;
	case 0b001: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b011: {
		clock.Q = 166;
		clock.R = 6666;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Core2]
CLOCK Clock_Core2(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	FSB_FREQ FSB={.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed) {
	case 0b101: {
		clock.Q = 100;
		clock.R = 0;
		}
		break;
	case 0b001: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b011: {
		clock.Q = 166;
		clock.R = 6666;
		}
		break;
	case 0b010: {
		clock.Q = 200;
		clock.R = 0;
		}
		break;
	case 0b000: {
		clock.Q = 266;
		clock.R = 6666;
		}
		break;
	case 0b100: {
		clock.Q = 333;
		clock.R = 3333;
		}
		break;
	case 0b110: {
		clock.Q = 400;
		clock.R = 0;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Atom]
CLOCK Clock_Atom(unsigned int ratio)
{
	CLOCK clock = {.Q = 83, .R = 0};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed) {
	case 0b111: {
		clock.Q = 83;
		clock.R = 2000;
		}
		break;
	case 0b101: {
		clock.Q = 99;
		clock.R = 8400;
		}
		break;
	case 0b001: {
		clock.Q = 133;
		clock.R = 2000;
		}
		break;
	case 0b011: {
		clock.Q = 166;
		clock.R = 4000;
		}
		break;
	default: {
		clock.Q = 83;
		clock.R = 2000;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Airmont]
CLOCK Clock_Airmont(unsigned int ratio)
{
	CLOCK clock = {.Q = 87, .R = 5};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed) {
	case 0b000: {
		clock.Q = 83;
		clock.R = 3333;
		}
		break;
	case 0b001: {
		clock.Q = 100;
		clock.R = 0000;
		}
		break;
	case 0b010: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b011: {
		clock.Q = 116;
		clock.R = 6666;
		}
		break;
	case 0b100: {
		clock.Q = 80;
		clock.R = 0000;
		}
		break;
	case 0b101: {
		clock.Q = 93;
		clock.R = 3333;
		}
		break;
	case 0b110: {
		clock.Q = 90;
		clock.R = 0000;
		}
		break;
	case 0b111: {
		clock.Q = 88;
		clock.R = 9000;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Silvermont]
CLOCK Clock_Silvermont(unsigned int ratio)
{
	CLOCK clock = {.Q = 83, .R = 3};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed)
	{
	case 0b100: {
			clock.Q = 80;
			clock.R = 0;
		}
		break;
	case 0b000: {
			clock.Q = 83;
			clock.R = 3000;
		}
		break;
	case 0b001: {
			clock.Q = 100;
			clock.R = 0;
		}
		break;
	case 0b010: {
			clock.Q = 133;
			clock.R = 3333;
		}
		break;
	case 0b011: {
			clock.Q = 116;
			clock.R = 7000;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Nehalem]
CLOCK Clock_Nehalem(unsigned int ratio)
{
	CLOCK clock = {.Q = 133, .R = 3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Westmere]
CLOCK Clock_Westmere(unsigned int ratio)
{
	CLOCK clock = {.Q = 133, .R = 3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [SandyBridge]
CLOCK Clock_SandyBridge(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [IvyBridge]
CLOCK Clock_IvyBridge(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Haswell]
CLOCK Clock_Haswell(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Skylake]
CLOCK Clock_Skylake(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};

	if (Proc->Features.Info.LargestStdFunc >= 0x16) {
		unsigned int eax = 0x0, ebx = 0x0, edx = 0x0, fsb = 0;
		asm volatile
		(
			"cpuid"
			: "=a" (eax),
			  "=b" (ebx),
			  "=c" (fsb),
			  "=d" (edx)
			:  "a" (0x16)
		);
		if (fsb > 0)
			clock.Q = fsb;
		else
			clock.Q = 100;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

void Define_CPUID(CORE *Core, const CPUID_STRUCT CpuIDforVendor[])
{	// Per vendor, define a CPUID dump table to query.
	int i;
	for (i = 0; i < CPUID_MAX_FUNC; i++) {
		Core->CpuID[i].func = CpuIDforVendor[i].func;
		Core->CpuID[i].sub  = CpuIDforVendor[i].sub;
	}
}

void Cache_Topology(CORE *Core)
{
	unsigned int level = 0x0;
	if (!strncmp(Proc->Features.Info.VendorID, VENDOR_INTEL, 12)) {
	    for (level = 0; level < CACHE_MAX_LEVEL; level++) {
		asm volatile
		(
			"cpuid"
			: "=a" (Core->T.Cache[level].AX),
			  "=b" (Core->T.Cache[level].BX),
			  "=c" (Core->T.Cache[level].Set),
			  "=d" (Core->T.Cache[level].DX)
			:  "a" (0x4),
			   "c" (level)
		);
		if (!Core->T.Cache[level].Type)
			break;
	    }
	}
	else if (!strncmp(Proc->Features.Info.VendorID, VENDOR_AMD, 12)) {
	    struct CACHE_INFO CacheInfo; // Employ the Intel algorithm.

	    if (Proc->Features.Info.LargestExtFunc >= 0x80000005) {
		Core->T.Cache[0].Level = 1;
		Core->T.Cache[0].Type  = 2;		// Inst.
		Core->T.Cache[1].Level = 1;
		Core->T.Cache[1].Type  = 1;		// Data

		// Fn8000_0005 L1 Data and Inst. caches
		asm volatile
		(
			"cpuid"
			: "=a" (CacheInfo.AX),
			  "=b" (CacheInfo.BX),
			  "=c" (CacheInfo.CX),
			  "=d" (CacheInfo.DX)
			:  "a" (0x80000005)
		);
		// L1 Inst.
		Core->T.Cache[0].Way  = CacheInfo.CPUID_0x80000005_L1I.Assoc;
		Core->T.Cache[0].Size = CacheInfo.CPUID_0x80000005_L1I.Size;
		// L1 Data
		Core->T.Cache[1].Way  = CacheInfo.CPUID_0x80000005_L1D.Assoc;
		Core->T.Cache[1].Size = CacheInfo.CPUID_0x80000005_L1D.Size;
	    }
	    if (Proc->Features.Info.LargestExtFunc >= 0x80000006) {
		Core->T.Cache[2].Level = 2;
		Core->T.Cache[2].Type  = 3;		// Unified!
		Core->T.Cache[3].Level = 3;
		Core->T.Cache[3].Type  = 3;

		// Fn8000_0006 L2 and L3 caches
		asm volatile
		(
			"cpuid"
			: "=a" (CacheInfo.AX),
			  "=b" (CacheInfo.BX),
			  "=c" (CacheInfo.CX),
			  "=d" (CacheInfo.DX)
			:  "a" (0x80000006)
		);
		// L2
		Core->T.Cache[2].Way  = CacheInfo.CPUID_0x80000006_L2.Assoc;
		Core->T.Cache[2].Size = CacheInfo.CPUID_0x80000006_L2.Size;
		// L3
		Core->T.Cache[3].Way  = CacheInfo.CPUID_0x80000006_L3.Assoc;
		Core->T.Cache[3].Size = CacheInfo.CPUID_0x80000006_L3.Size;
	    }
	}
}

// Enumerate the Processor's Cores and Threads topology.
void Map_Topology(void *arg)
{
	if (arg != NULL) {
		CORE *Core = (CORE *) arg;
		FEATURES features;

		RDMSR(Core->T.Base, MSR_IA32_APICBASE);

		asm volatile
		(
			"cpuid"
			: "=b" (features.Std.BX)
			:  "a" (0x1)
		);
		Core->T.CoreID = Core->T.ApicID=features.Std.BX.Apic_ID;

		Cache_Topology(Core);
	}
}

void Map_Extended_Topology(void *arg)
{
	if (arg != NULL) {
		CORE *Core = (CORE *) arg;

		int	InputLevel = 0, NoMoreLevels = 0,
			SMT_Mask_Width = 0, SMT_Select_Mask = 0,
			CorePlus_Mask_Width = 0, CoreOnly_Select_Mask = 0;

		CPUID_TOPOLOGY_LEAF ExtTopology = {
			.AX.Register = 0,
			.BX.Register = 0,
			.CX.Register = 0,
			.DX.Register = 0
		};

		RDMSR(Core->T.Base, MSR_IA32_APICBASE);

		do {
			asm volatile
			(
				"cpuid"
				: "=a" (ExtTopology.AX),
				  "=b" (ExtTopology.BX),
				  "=c" (ExtTopology.CX),
				  "=d" (ExtTopology.DX)
				:  "a" (0xb),
				   "c" (InputLevel)
			);
			// Exit from the loop if the BX register equals 0 or
			// if the requested level exceeds the level of a Core.
			if (	!ExtTopology.BX.Register
				|| (InputLevel > LEVEL_CORE))
					NoMoreLevels = 1;
			else {
			    switch (ExtTopology.CX.Type) {
			    case LEVEL_THREAD: {
				SMT_Mask_Width   = ExtTopology.AX.SHRbits;

				SMT_Select_Mask  = ~((-1) << SMT_Mask_Width);

				Core->T.ThreadID = ExtTopology.DX.x2ApicID
							& SMT_Select_Mask;
				}
				break;
			    case LEVEL_CORE: {
				CorePlus_Mask_Width  = ExtTopology.AX.SHRbits;

				CoreOnly_Select_Mask =
						(~((-1) << CorePlus_Mask_Width))
							^ SMT_Select_Mask;

				Core->T.CoreID = (ExtTopology.DX.x2ApicID
				    & CoreOnly_Select_Mask) >> SMT_Mask_Width;
				}
				break;
			    }
			}
			InputLevel++;
		} while (!NoMoreLevels);

		Core->T.ApicID = ExtTopology.DX.x2ApicID;

		Cache_Topology(Core);
	}
}

int Core_Topology(unsigned int cpu)
{
	int rc = smp_call_function_single(cpu,
				(Proc->Features.Info.LargestStdFunc >= 0xb) ?
					Map_Extended_Topology : Map_Topology,
				KPublic->Core[cpu],
				1); // Synchronous call.

	if (	!rc
		&& !Proc->Features.HTT_Enable
		&& (KPublic->Core[cpu]->T.ThreadID > 0))
			Proc->Features.HTT_Enable = 1;

	return(rc);
}

unsigned int Proc_Topology(void)
{
	unsigned int cpu, CountEnabledCPU = 0;

	for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
		KPublic->Core[cpu]->T.Base.value = 0;
		KPublic->Core[cpu]->T.ApicID     = -1;
		KPublic->Core[cpu]->T.CoreID     = -1;
		KPublic->Core[cpu]->T.ThreadID   = -1;

		// CPU state based on the OS
		if (	!(KPublic->Core[cpu]->OffLine.OS = !cpu_online(cpu)
			|| !cpu_active(cpu)) ) {

			if (!Core_Topology(cpu)) {
				// CPU state based on the hardware
				if (KPublic->Core[cpu]->T.ApicID >= 0) {
					KPublic->Core[cpu]->OffLine.HW = 0;
					CountEnabledCPU++;
				}
				else
					KPublic->Core[cpu]->OffLine.HW = 1;
			}
		}
	}
	return(CountEnabledCPU);
}

void HyperThreading_Technology(void)
{
	unsigned int CountEnabledCPU = Proc_Topology();

	if (Proc->Features.Std.DX.HTT)
		Proc->CPU.OnLine = CountEnabledCPU;
	else
		Proc->CPU.OnLine = Proc->CPU.Count;
}

int Intel_MaxBusRatio(PLATFORM_ID *PfID)
{
	struct SIGNATURE signature[] = {
		_Core_Conroe,		/* 06_0F */
		_Core_Yorkfield,	/* 06_17 */
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft,		/* 06_27 */
		_Atom_Clovertrail,	/* 06_35 */
		_Atom_Saltwell,		/* 06_36 */
		_Silvermont_637,	/* 06_37 */
	};
	int id, ids = sizeof(signature) / sizeof(signature[0]);
	for (id = 0; id < ids; id++) {
		if ((signature[id].ExtFamily == Proc->Features.Std.AX.ExtFamily)
		 && (signature[id].Family == Proc->Features.Std.AX.Family)
		 && (signature[id].ExtModel == Proc->Features.Std.AX.ExtModel)
		 && (signature[id].Model == Proc->Features.Std.AX.Model)) {

			RDMSR((*PfID), MSR_IA32_PLATFORM_ID);
			return(0);
		}
	}
	return(-1);
}

void Intel_Platform_Info(void)
{
	PLATFORM_ID PfID = {.value = 0};
	PLATFORM_INFO PfInfo = {.value = 0};
	PERF_STATUS PerfStatus = {.value = 0};
	unsigned int ratio0 = 1, ratio1 = 10, ratio2 = 50; // Arbitrary

	RDMSR(PfInfo, MSR_PLATFORM_INFO);
	if (PfInfo.value != 0) {
		ratio0 = KMIN(PfInfo.MinimumRatio, PfInfo.MaxNonTurboRatio);
		ratio1 = KMAX(PfInfo.MinimumRatio, PfInfo.MaxNonTurboRatio);
		ratio2 = ratio1;
	}

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	if (PerfStatus.value != 0) {				// §18.18.3.4
		if (PerfStatus.XE_Enable) {
			ratio2 = PerfStatus.MaxBusRatio;
		} else {
			if (Intel_MaxBusRatio(&PfID) == 0) {
				if (PfID.value != 0)
					ratio2 = PfID.MaxBusRatio;
			}
		}
	} else {
			if (Intel_MaxBusRatio(&PfID) == 0) {
				if (PfID.value != 0)
					ratio2 = PfID.MaxBusRatio;
			}
	}
	Proc->Boost[0] = ratio0;
	Proc->Boost[1] = KMIN(ratio1, ratio2);
	Proc->Boost[9] = KMAX(ratio1, ratio2);
}

void DynamicAcceleration(void)
{
	if (Proc->Features.Info.LargestStdFunc >= 0x6) {
		struct THERMAL_POWER_LEAF Power = {{0}};
		asm volatile
		(
			"cpuid"
			: "=a" (Power.AX),
			  "=b" (Power.BX),
			  "=c" (Power.CX),
			  "=d" (Power.DX)
			:  "a" (0x6)
		);
		if (Power.AX.TurboIDA == 1) {
			Proc->Boost[8] = Proc->Boost[9];
			Proc->Boost[9]++;
		}
	}
}

#ifndef MSR_TURBO_RATIO_LIMIT
#define MSR_TURBO_RATIO_LIMIT MSR_NHM_TURBO_RATIO_LIMIT
#endif

void Nehalem_Platform_Info(void)
{
	PLATFORM_INFO Platform = {.value = 0};
	TURBO_RATIO Turbo = {.value = 0};

	RDMSR(Platform, MSR_PLATFORM_INFO);
	RDMSR(Turbo, MSR_TURBO_RATIO_LIMIT);

	Proc->Boost[0] = Platform.MinimumRatio;
	Proc->Boost[1] = Platform.MaxNonTurboRatio;
	Proc->Boost[2] = Turbo.MaxRatio_8C;
	Proc->Boost[3] = Turbo.MaxRatio_7C;
	Proc->Boost[4] = Turbo.MaxRatio_6C;
	Proc->Boost[5] = Turbo.MaxRatio_5C;
	Proc->Boost[6] = Turbo.MaxRatio_4C;
	Proc->Boost[7] = Turbo.MaxRatio_3C;
	Proc->Boost[8] = Turbo.MaxRatio_2C;
	Proc->Boost[9] = Turbo.MaxRatio_1C;
}

typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

typedef void (*ROUTER)(void __iomem *mchmap);

PCI_CALLBACK Router(struct pci_dev *dev, unsigned int offset, ROUTER route)
{
	void __iomem *mchmap;
	union {
		unsigned long long addr;
		struct {
			unsigned int low;
			unsigned int high;
		};
	} mchbar;

	Proc->Uncore.ChipID = dev->device;

	pci_read_config_dword(dev, offset    , &mchbar.low);
	pci_read_config_dword(dev, offset + 4, &mchbar.high);

	mchbar.addr &= 0xffffc000ULL;

	mchmap = ioremap(mchbar.addr, 0x4000);
	if (mchmap != NULL) {
		route(mchmap);

		iounmap(mchmap);

		return(0);
	} else
		return((PCI_CALLBACK) -ENOMEM);
}

void Query_P965(void __iomem *mchmap)
{
	unsigned short cha;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P965.CKE0.value = readl(mchmap + 0x260);
	Proc->Uncore.MC[0].P965.CKE1.value = readl(mchmap + 0x660);
	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].P965.CKE0.RankPop0 != 0)
				+ (Proc->Uncore.MC[0].P965.CKE1.RankPop0 != 0);

	Proc->Uncore.CtrlCount = 1;
	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {

		Proc->Uncore.MC[0].Channel[cha].P965.DRT0.value =
					readl(mchmap + 0x29c + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT1.value =
					readw(mchmap + 0x250 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT2.value =
					readl(mchmap + 0x252 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT3.value =
					readw(mchmap + 0x256 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT4.value =
					readl(mchmap + 0x258 + 0x400 * cha);
	}
}

void Query_G965(void __iomem *mchmap)
{	// Source: Mobile Intel 965 Express Chipset Family
	unsigned short cha;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].G965.DRB0.value = readl(mchmap + 0x1200);
	Proc->Uncore.MC[0].G965.DRB1.value = readl(mchmap + 0x1300);
	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].G965.DRB0.Rank1Addr != 0)
				+ (Proc->Uncore.MC[0].G965.DRB1.Rank1Addr != 0);

	Proc->Uncore.CtrlCount = 1;
	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {

		Proc->Uncore.MC[0].Channel[cha].G965.DRT0.value =
					readl(mchmap + 0x1210 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT1.value =
					readl(mchmap + 0x1214 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT2.value =
					readl(mchmap + 0x1218 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT3.value =
					readl(mchmap + 0x121c + 0x100 * cha);
	}
}

void Query_P35(void __iomem *mchmap)
{	// Source: Intel® 3 Series Express Chipset Family
	unsigned short cha;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P35.CKE0.value = readl(mchmap + 0x260);
	Proc->Uncore.MC[0].P35.CKE1.value = readl(mchmap + 0x660);
	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].P35.CKE0.RankPop0 != 0)
				+ (Proc->Uncore.MC[0].P35.CKE1.RankPop0 != 0);

	Proc->Uncore.CtrlCount = 1;
	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {

		Proc->Uncore.MC[0].Channel[cha].P35.DRT0.value =
					readw(mchmap + 0x265 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT1.value =
					readw(mchmap + 0x250 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT2.value =
					readl(mchmap + 0x252 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT3.value =
					readl(mchmap + 0x256 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT4.value =
					readl(mchmap + 0x258 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT5.value =
					readw(mchmap + 0x25d + 0x400 * cha);
	}
}

kernel_ulong_t Query_X58_Timing(unsigned short mc, unsigned short cha)
{	// Source: Micron Technical Note DDR3 Power-Up, Initialization, & Reset
    unsigned int did[3] = {
			PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL,
			PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL,
			PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL
		};
    struct pci_dev *dev = pci_get_device(PCI_VENDOR_ID_INTEL, did[cha], NULL);
    if(dev != NULL) {
	pci_read_config_dword(dev, 0x70,
			      &Proc->Uncore.MC[mc].Channel[cha].X58.MRS.value);

	pci_read_config_dword(dev ,0x80,
			    &Proc->Uncore.MC[mc].Channel[cha].X58.Rank_A.value);

	pci_read_config_dword(dev ,0x84,
			    &Proc->Uncore.MC[mc].Channel[cha].X58.Rank_B.value);

	pci_read_config_dword(dev ,0x88,
			      &Proc->Uncore.MC[mc].Channel[cha].X58.Bank.value);

	pci_read_config_dword(dev ,0x8c,
			   &Proc->Uncore.MC[mc].Channel[cha].X58.Refresh.value);

	pci_dev_put(dev);
	return(0);
    } else
	return(-ENODEV);
}

kernel_ulong_t Query_X58(struct pci_dev *dev, unsigned short mc)
{
	kernel_ulong_t rc = 0;
	unsigned short cha;

	pci_read_config_dword(dev,0x48, &Proc->Uncore.MC[mc].X58.CONTROL.value);
	pci_read_config_dword(dev,0x4c, &Proc->Uncore.MC[mc].X58.STATUS.value);

	Proc->Uncore.MC[mc].ChannelCount =
		  (Proc->Uncore.MC[mc].X58.CONTROL.CHANNEL0_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].X58.CONTROL.CHANNEL1_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].X58.CONTROL.CHANNEL2_ACTIVE != 0);

	for (cha = 0; (cha < Proc->Uncore.MC[mc].ChannelCount) && !rc; cha++) {
		rc = Query_X58_Timing(mc, cha);
	}
	return(rc);
}

void Query_C200(void __iomem *mchmap)
{	// Source: Intel® Xeon Processor E3-1200 Family
	unsigned short cha;

/*?*/	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].C200.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].C200.MAD1.value = readl(mchmap + 0x5008);
	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].C200.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD0.Dimm_B_Size != 0))
		+ ((Proc->Uncore.MC[0].C200.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD1.Dimm_B_Size != 0));

	Proc->Uncore.CtrlCount = 1;
	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {

		Proc->Uncore.MC[0].Channel[cha].C200.DBP.value =
					readl(mchmap + 0x4000 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].C200.RAP.value =
					readl(mchmap + 0x4004 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].C200.RFTP.value =
					readl(mchmap + 0x4298 + 0x400 * cha);
	}
}

void Query_C220(void __iomem *mchmap)
{	// Source: Desktop 4th Generation Intel® Core™ Processor Family
	unsigned short cha;

/*?*/	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].C200.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].C200.MAD1.value = readl(mchmap + 0x5008);
	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].C200.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD0.Dimm_B_Size != 0))
		+ ((Proc->Uncore.MC[0].C200.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD1.Dimm_B_Size != 0));

	Proc->Uncore.CtrlCount = 1;
	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {

		Proc->Uncore.MC[0].Channel[cha].C220.Rank.value =
					readl(mchmap + 0x4c14 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].C220.Refresh.value =
					readl(mchmap + 0x4e98 + 0x400 * cha);
	}
}

PCI_CALLBACK P965(struct pci_dev *dev)
{
	return(Router(dev, 0x48, Query_P965));
}

PCI_CALLBACK G965(struct pci_dev *dev)
{
	return(Router(dev, 0x48, Query_G965));
}

PCI_CALLBACK P35(struct pci_dev *dev)
{
	return(Router(dev, 0x48, Query_P35));
}

PCI_CALLBACK X58_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_X58(dev, mc);

	return((PCI_CALLBACK) rc);
}

PCI_CALLBACK X58_IMC_TR(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x50, &Proc->Uncore.Bus.DimmClock.value);

	return(0);
}

PCI_CALLBACK X58_QPI(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xd0, &Proc->Uncore.Bus.QuickPath.value);

	return(0);
}

PCI_CALLBACK C200(struct pci_dev *dev)
{
	return(Router(dev, 0x48, Query_C200));
}

PCI_CALLBACK C220(struct pci_dev *dev)
{
	return(Router(dev, 0x48, Query_C220));
}

static int CoreFreqK_ProbePCI(	struct pci_dev *dev,
				const struct pci_device_id *id)
{
	int rc = -ENODEV;

	if (!pci_enable_device(dev)) {
		PCI_CALLBACK Callback = (PCI_CALLBACK) id->driver_data;
			rc =(int) Callback(dev);
	}

	return(rc);
}

static void CoreFreqK_RemovePCI(struct pci_dev *dev)
{
	pci_disable_device(dev);
}

static struct pci_device_id CoreFreqK_pci_ids[] = {
	{	// i946 - Lakeport
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2970),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// Q963/Q965 - Broadwater
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2990),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// P965/G965 - Broadwater
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29a0),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// GM965 - Crestline
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2a00),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// GME965
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2a10),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// GM45 - Cantiga
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2a40),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// Q35 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29b0),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// P35/G33 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29c0),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Q33 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29d0),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// X38/X48 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29e0),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// 3200/3210 - Unknown
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x29f0),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Q45/Q43 - Unknown
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2e10),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// P45/G45 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2e20),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// G41 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2e30),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Nehalem IMC
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MCR),
		.driver_data = (kernel_ulong_t) X58_IMC
	},
	{	// Nehalem IMC
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MC_TEST),
		.driver_data = (kernel_ulong_t) X58_IMC_TR
	},
	{	// Nehalem Control Status and RAS Registers
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x3423),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{	// Sandy Bridge
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0),
		.driver_data = (kernel_ulong_t) C200
	},
	{	// Ivy Bridge
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x0ea0),
		.driver_data = (kernel_ulong_t) C200
	},
	{	// Haswell
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x2fa0),
		.driver_data = (kernel_ulong_t) C220
	},
	{0, }
};

MODULE_DEVICE_TABLE(pci, CoreFreqK_pci_ids);

static struct pci_driver CoreFreqK_pci_driver = {
	.name = "corefreqk",
	.id_table = CoreFreqK_pci_ids,
	.probe = CoreFreqK_ProbePCI,
	.remove = CoreFreqK_RemovePCI
};


void Query_GenuineIntel(void)
{
	Intel_Platform_Info();
	HyperThreading_Technology();
}

void Query_AuthenticAMD(void)
{
	if (Proc->Features.AdvPower.DX.FID == 1) {
		// Processor supports FID changes.
		FIDVID_STATUS FidVidStatus = {.value = 0};

	/* FID */ const unsigned int VCO[0b1000][5] = {
	/* 000000b */	{ 0,  0, 16, 17, 18},
	/* 000001b */	{16, 17, 18, 19, 20},
	/* 000010b */	{18, 19, 20, 21, 22},
	/* 000011b */	{20, 21, 22, 23, 24},
	/* 000100b */	{22, 23, 24, 25, 26},
	/* 000101b */	{24, 25, 26, 27, 28},
	/* 000110b */	{26, 27, 28, 29, 30},
	/* 000111b */	{28, 29, 30, 31, 32}
		};

		RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		Proc->Boost[0] = 8 + FidVidStatus.StartFID;
		Proc->Boost[1] = 8 + FidVidStatus.MaxFID;

		if (FidVidStatus.StartFID < 0b1000) {
		    unsigned int t;
		    for (t = 0; t < 5; t++)
			Proc->Boost[5 + t] = VCO[FidVidStatus.StartFID][t];
		}
		else
			Proc->Boost[9] = 8 + FidVidStatus.MaxFID;
	} else {
		HWCR HwCfgRegister = {.value = 0};

		RDMSR(HwCfgRegister, MSR_K7_HWCR);

		Proc->Boost[0] = 8 + HwCfgRegister.Family_0Fh.StartFID;
		Proc->Boost[1] = Proc->Boost[0];
		Proc->Boost[9] = Proc->Boost[0];
	}
/*
Note: hardware Families > 0Fh

	else if ( Proc->Features.AdvPower.DX.HwPstate == 1)
		CoreCOF = 100 * (MSRC001_00[6B:64][CpuFid] + 10h)
			/ (2^MSRC001_00[6B:64][CpuDid])
*/

	Proc->Features.FactoryFreq = Proc->Boost[1] * 1000; // MHz

	HyperThreading_Technology();
}

void Query_Core2(void)
{
	Intel_Platform_Info();
	DynamicAcceleration();
	HyperThreading_Technology();
}

void Query_Nehalem(void)
{
	Nehalem_Platform_Info();
	HyperThreading_Technology();
}

void Query_SandyBridge(void)
{
	Nehalem_Platform_Info();
	HyperThreading_Technology();
}

void Dump_CPUID(CORE *Core)
{
	unsigned int i;

	asm volatile
	(
		"cpuid"
		: "=a" (Core->Query.StdFunc.LargestStdFunc),
		  "=b" (Core->Query.StdFunc.BX),
		  "=c" (Core->Query.StdFunc.CX),
		  "=d" (Core->Query.StdFunc.DX)
		:  "a" (0x00000000),
		   "c" (0x00000000)
	);
	asm volatile
	(
		"cpuid"
		: "=a" (Core->Query.ExtFunc.LargestExtFunc),
		  "=b" (Core->Query.ExtFunc.BX),
		  "=c" (Core->Query.ExtFunc.CX),
		  "=d" (Core->Query.ExtFunc.DX)
		:  "a" (0x80000000),
		   "c" (0x00000000)
	);
	for (i = 0; i < CPUID_MAX_FUNC; i++) {
	    if ( ( (Core->CpuID[i].func & 0x80000000)
		&& (Core->CpuID[i].func <= Core->Query.ExtFunc.LargestExtFunc))
		|| (Core->CpuID[i].func && !(Core->CpuID[i].func & 0x80000000)
		&& (Core->CpuID[i].func <= Core->Query.StdFunc.LargestStdFunc)))
			asm volatile
			(
				"cpuid"
				: "=a" (Core->CpuID[i].reg[0]),
				  "=b" (Core->CpuID[i].reg[1]),
				  "=c" (Core->CpuID[i].reg[2]),
				  "=d" (Core->CpuID[i].reg[3])
				:  "a" (Core->CpuID[i].func),
				   "c" (Core->CpuID[i].sub)
			);
	}
}

void SpeedStep_Technology(CORE *Core)				// Per Package!
{
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};

	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	Core->Query.EIST = Proc->Features.Std.CX.EIST & MiscFeatures.EIST;
}

void TurboBoost_Technology(CORE *Core)
{
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	PERF_CONTROL PerfControl = {.value = 0};

	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
	RDMSR(PerfControl, MSR_IA32_PERF_CTL);

	Core->Query.Turbo = (MiscFeatures.Turbo_IDA == 1) ? 0
				: (PerfControl.Turbo_IDA == 1) ? 0 : 1;
}

void Query_Intel_C1E(CORE *Core)
{
	POWER_CONTROL PowerCtrl = {.value = 0};

	RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);			// Per Core

	Core->Query.C1E = PowerCtrl.C1E;			// Per Package!
}

void Query_AMD_C1E(CORE *Core)
{
	INT_PENDING_MSG IntPendingMsg = {.value = 0};

	RDMSR(IntPendingMsg, MSR_K8_INT_PENDING_MSG);

	Core->Query.C1E = IntPendingMsg.C1eOnCmpHalt;
}

void ThermalMonitor_Set(CORE *Core)
{
	TJMAX TjMax = {.value = 0};
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	THERM2_CONTROL Therm2Control = {.value = 0};

	//Silvermont + Xeon[06_57] + Nehalem + Sandy Bridge & superior arch.
	RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);

	Core->PowerThermal.Target = TjMax.Target;
	if (Core->PowerThermal.Target == 0)
		Core->PowerThermal.Target = 100; // ToDo: TjMax database.

	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	Core->PowerThermal.TCC_Enable = MiscFeatures.TCC;
	Core->PowerThermal.TM2_Enable = MiscFeatures.TM2_Enable;

	RDMSR(Therm2Control, MSR_THERM2_CTL);		// All Intel families.

	Core->PowerThermal.TM2_Enable = Therm2Control.TM_SELECT;
}

void PowerThermal(CORE *Core)
{
    if (Proc->Features.Info.LargestStdFunc >= 0x6) {
	struct THERMAL_POWER_LEAF Power = {{0}};

	asm volatile
	(
		"cpuid"
		: "=a" (Power.AX),
		  "=b" (Power.BX),
		  "=c" (Power.CX),
		  "=d" (Power.DX)
		:  "a" (0x6)
	);

	if (Proc->Features.Std.DX.ACPI == 1) {
	    RDMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);

	    Core->PowerThermal.ClockModulation.ExtensionBit = Power.AX.ECMD;
	}
	if (Power.CX.SETBH == 1)
	    RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
    }
}

void PerCore_Intel_Query(CORE *Core)
{
	Dump_CPUID(Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

void PerCore_AMD_Query(CORE *Core)
{
	Dump_CPUID(Core);

	Query_AMD_C1E(Core);
}

void PerCore_Core2_Query(CORE *Core)
{
	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	PowerThermal(Core);					// Shared/Unique

	ThermalMonitor_Set(Core);
}

void PerCore_Nehalem_Query(CORE *Core)
{
	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	TurboBoost_Technology(Core);
	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				// Per Core
		CSTATE_CONFIG CStateConfig = {.value = 0};

		RDMSR(CStateConfig, MSR_NHM_SNB_PKG_CST_CFG_CTL);

		Core->Query.C3A = CStateConfig.C3autoDemotion;
		Core->Query.C1A = CStateConfig.C1autoDemotion;
	}
	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

void PerCore_SandyBridge_Query(CORE *Core)
{
	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	TurboBoost_Technology(Core);
	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				// Per Core
		CSTATE_CONFIG CStateConfig = {.value = 0};

		RDMSR(CStateConfig, MSR_NHM_SNB_PKG_CST_CFG_CTL);

		Core->Query.C3A = CStateConfig.C3autoDemotion;
		Core->Query.C1A = CStateConfig.C1autoDemotion;
		Core->Query.C3U = CStateConfig.C3undemotion;
		Core->Query.C1U = CStateConfig.C1undemotion;
	}
	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

void InitTimer(void *Cycle_Function)
{
	unsigned int cpu=smp_processor_id();

	if (KPrivate->Join[cpu]->tsm.created == 0) {
		hrtimer_init(	&KPrivate->Join[cpu]->Timer,
				CLOCK_MONOTONIC,
				HRTIMER_MODE_REL_PINNED);

		KPrivate->Join[cpu]->Timer.function = Cycle_Function;
		KPrivate->Join[cpu]->tsm.created = 1;
	}
}

void Controller_Init(void)
{
	CLOCK clock;
	unsigned int cpu = Proc->CPU.Count;

	if (Arch[Proc->ArchID].Query != NULL)
		Arch[Proc->ArchID].Query();

	do {	// from last AP to BSP
	    cpu--;

	    if (!KPublic->Core[cpu]->OffLine.OS) {
		if(AutoClock)
			clock = Base_Clock(cpu, Proc->Boost[1]);
		else if(Arch[Proc->ArchID].Clock != NULL)
			clock = Arch[Proc->ArchID].Clock(Proc->Boost[1]);

		KPublic->Core[cpu]->Clock = clock;

		if (Arch[Proc->ArchID].Timer != NULL)
			Arch[Proc->ArchID].Timer(cpu);
		}
	} while (cpu != 0) ;
}

void Controller_Start(void)
{
	if (Arch[Proc->ArchID].Start != NULL) {
		unsigned int cpu;
		for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    if (   (KPrivate->Join[cpu]->tsm.created == 1)
			&& (KPrivate->Join[cpu]->tsm.started == 0))
				smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, 0);
	}
}

void Controller_Stop(void)
{
	if (Arch[Proc->ArchID].Stop != NULL) {
		unsigned int cpu;
		for (cpu=0; cpu < Proc->CPU.Count; cpu++)
		    if (   (KPrivate->Join[cpu]->tsm.created == 1)
			&& (KPrivate->Join[cpu]->tsm.started == 1))
				smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, 1);
	}
}

void Controller_Exit(void)
{
	unsigned int cpu;

	if (Arch[Proc->ArchID].Exit != NULL)
		Arch[Proc->ArchID].Exit();

	for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		KPrivate->Join[cpu]->tsm.created = 0;
}

void Counters_Set(CORE *Core)
{
    if (Proc->Features.PerfMon.AX.Version >= 2) {
	GLOBAL_PERF_COUNTER GlobalPerfCounter = {.value = 0};
	FIXED_PERF_COUNTER FixedPerfCounter = {.value = 0};
	GLOBAL_PERF_STATUS Overflow = {.value = 0};
	GLOBAL_PERF_OVF_CTRL OvfControl = {.value = 0};

	RDMSR(GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);
	Core->SaveArea.GlobalPerfCounter = GlobalPerfCounter;
	GlobalPerfCounter.EN_FIXED_CTR0  = 1;
	GlobalPerfCounter.EN_FIXED_CTR1  = 1;
	GlobalPerfCounter.EN_FIXED_CTR2  = 1;
	WRMSR(GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);

	RDMSR(FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);
	Core->SaveArea.FixedPerfCounter = FixedPerfCounter;
	FixedPerfCounter.EN0_OS = 1;
	FixedPerfCounter.EN1_OS = 1;
	FixedPerfCounter.EN2_OS = 1;
	FixedPerfCounter.EN0_Usr = 1;
	FixedPerfCounter.EN1_Usr = 1;
	FixedPerfCounter.EN2_Usr = 1;

	if (Proc->Features.PerfMon.AX.Version >= 3) {
		if (!Proc->Features.HTT_Enable) {
			FixedPerfCounter.AnyThread_EN0 = 1;
			FixedPerfCounter.AnyThread_EN1 = 1;
			FixedPerfCounter.AnyThread_EN2 = 1;
		} else {
			// Per Thread
			FixedPerfCounter.AnyThread_EN0 = 0;
			FixedPerfCounter.AnyThread_EN1 = 0;
			FixedPerfCounter.AnyThread_EN2 = 0;
		}
	}
	WRMSR(FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Overflow, MSR_CORE_PERF_GLOBAL_STATUS);
	if (Overflow.Overflow_CTR0)
		OvfControl.Clear_Ovf_CTR0 = 1;
	if (Overflow.Overflow_CTR1)
		OvfControl.Clear_Ovf_CTR1 = 1;
	if (Overflow.Overflow_CTR2)
		OvfControl.Clear_Ovf_CTR2 = 1;
	if (Overflow.Overflow_CTR0
	  | Overflow.Overflow_CTR1
	  | Overflow.Overflow_CTR2)
		WRMSR(OvfControl, MSR_CORE_PERF_GLOBAL_OVF_CTRL);
    }
}

void Counters_Clear(CORE *Core)
{
    if (Proc->Features.PerfMon.AX.Version >= 2) {
	WRMSR(Core->SaveArea.FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);
	WRMSR(Core->SaveArea.GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);
    }
}

#define Counters_Genuine(Core, T)					\
({									\
	RDTSC_COUNTERx2(Core->Counter[T].TSC,				\
			MSR_IA32_APERF, Core->Counter[T].C0.UCC,	\
			MSR_IA32_MPERF, Core->Counter[T].C0.URC);	\
	/* Derive C1 */							\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_Core2(Core, T)						\
({									\
    if (!Proc->Features.AdvPower.DX.Inv_TSC)				\
 	{								\
	RDTSC_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_FIXED_CTR1,Core->Counter[T].C0.UCC,\
			MSR_CORE_PERF_FIXED_CTR2,Core->Counter[T].C0.URC,\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	}								\
    else								\
	{								\
	RDTSCP_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_FIXED_CTR1,Core->Counter[T].C0.UCC,\
			MSR_CORE_PERF_FIXED_CTR2,Core->Counter[T].C0.URC,\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	}								\
	/* Derive C1 */							\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_Nehalem(Core, T)					\
({									\
	register unsigned long long Cx = 0;				\
									\
	RDTSCP_COUNTERx5(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_FIXED_CTR1,Core->Counter[T].C0.UCC,\
			MSR_CORE_PERF_FIXED_CTR2,Core->Counter[T].C0.URC,\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1 */							\
	Cx =	Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1 =						\
		(Core->Counter[T].TSC > Cx) ?				\
			Core->Counter[T].TSC - Cx			\
			: 0;						\
})

#define Counters_SandyBridge(Core, T)					\
({									\
	register unsigned long long Cx = 0;				\
									\
	RDTSCP_COUNTERx6(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_FIXED_CTR1,Core->Counter[T].C0.UCC,\
			MSR_CORE_PERF_FIXED_CTR2,Core->Counter[T].C0.URC,\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_C7_RESIDENCY,Core->Counter[T].C7,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1 */							\
	Cx =	Core->Counter[T].C7					\
		+ Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1 =						\
		(Core->Counter[T].TSC > Cx) ?				\
			Core->Counter[T].TSC - Cx			\
			: 0;						\
})

#define Delta_TSC(Core)							\
({									\
	Core->Delta.TSC = Core->Counter[1].TSC				\
			- Core->Counter[0].TSC;				\
})

#define Delta_C0(Core)							\
({	/* Absolute Delta of Unhalted (Core & Ref) C0 Counter. */	\
	Core->Delta.C0.UCC =						\
		(Core->Counter[0].C0.UCC >				\
		Core->Counter[1].C0.UCC) ?				\
			Core->Counter[0].C0.UCC				\
			- Core->Counter[1].C0.UCC			\
			: Core->Counter[1].C0.UCC			\
			- Core->Counter[0].C0.UCC;			\
									\
	Core->Delta.C0.URC = Core->Counter[1].C0.URC			\
			   - Core->Counter[0].C0.URC;			\
})

#define Delta_C1(Core)							\
({									\
	Core->Delta.C1 =						\
		(Core->Counter[0].C1 >					\
		 Core->Counter[1].C1) ?					\
			Core->Counter[0].C1				\
			- Core->Counter[1].C1				\
			: Core->Counter[1].C1				\
			- Core->Counter[0].C1;				\
})

#define Delta_C3(Core)							\
({									\
	Core->Delta.C3  = Core->Counter[1].C3				\
			- Core->Counter[0].C3;				\
})

#define Delta_C6(Core)							\
({									\
	Core->Delta.C6  = Core->Counter[1].C6				\
			- Core->Counter[0].C6;				\
})

#define Delta_C7(Core)							\
({									\
	Core->Delta.C7  = Core->Counter[1].C7				\
			- Core->Counter[0].C7;				\
})

#define Delta_INST(Core)						\
({	/* Delta of Instructions Retired */				\
	Core->Delta.INST = Core->Counter[1].INST			\
			 - Core->Counter[0].INST;			\
})

#define Save_TSC(Core)							\
({	/* Save Time Stamp Counter. */					\
	Core->Counter[0].TSC = Core->Counter[1].TSC;			\
})

#define Save_C0(Core)							\
({	/* Save the Unhalted Core & Reference Counter */		\
	Core->Counter[0].C0.UCC = Core->Counter[1].C0.UCC;		\
	Core->Counter[0].C0.URC = Core->Counter[1].C0.URC;		\
})

#define Save_C1(Core)							\
({									\
	Core->Counter[0].C1 = Core->Counter[1].C1;			\
})

#define Save_C3(Core)							\
({									\
	Core->Counter[0].C3 = Core->Counter[1].C3;			\
})

#define Save_C6(Core)							\
({									\
	Core->Counter[0].C6 = Core->Counter[1].C6;			\
})

#define Save_C7(Core)							\
({									\
	Core->Counter[0].C7 = Core->Counter[1].C7;			\
})

#define Save_INST(Core)							\
({	/* Save the Instructions counter. */				\
	Core->Counter[0].INST = Core->Counter[1].INST;			\
})

void Core_Intel_Temp(CORE *Core)
{
	THERM_STATUS ThermStatus = {.value = 0};
	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);	// All Intel families.

	Core->PowerThermal.Sensor = ThermStatus.DTS;
	Core->PowerThermal.Trip = ThermStatus.StatusBit | ThermStatus.StatusLog;
}

void Core_AMD_Temp(CORE *Core)
{
	if (Proc->Features.AdvPower.DX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip;

		RDPCI(ThermTrip, PCI_CONFIG_ADDRESS(0, 24, 3, 0xe4));

		// Select Core to read sensor from:
		ThermTrip.SensorCoreSelect = Core->Bind;

		WRPCI(ThermTrip, PCI_CONFIG_ADDRESS(0, 24, 3, 0xe4));
		RDPCI(ThermTrip, PCI_CONFIG_ADDRESS(0, 24, 3, 0xe4));

		// Formula is " CurTmp - (TjOffset * 2) - 49 "
		Core->PowerThermal.Target = ThermTrip.TjOffset;
		Core->PowerThermal.Sensor = ThermTrip.CurrentTemp;

		Core->PowerThermal.Trip = ThermTrip.SensorTrip;
	}
}

static enum hrtimer_restart Cycle_GenuineIntel(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Genuine(Core, 1);
		Core_Intel_Temp(Core);

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, 63);

		return(HRTIMER_RESTART);
	} else
		return(HRTIMER_NORESTART);
}

#define Cycle_AuthenticAMD Cycle_GenuineIntel

void InitTimer_GenuineIntel(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_GenuineIntel, 1);
}

void Start_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Intel_Query(Core);

	Counters_Genuine(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd = 1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started = 1;
}

void Stop_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();

	KPrivate->Join[cpu]->tsm.mustFwd = 0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	KPrivate->Join[cpu]->tsm.started = 0;
}

/*
Note: hardware Family_12h

static enum hrtimer_restart Cycle_AMD_Family_12h(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		// Core Performance Boost instructions
		// [ Here ]

		// Derive C1
		Core->Counter[1].C1 =
		  (Core->Counter[1].TSC > Core->Counter[1].C0.URC) ?
		    Core->Counter[1].TSC - Core->Counter[1].C0.URC
		    : 0;

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		Core_AMD_Temp(Core);

		BITSET(LOCKLESS, Core->Sync.V, 63);

		return(HRTIMER_RESTART);
	} else
		return(HRTIMER_NORESTART);
}
*/

static enum hrtimer_restart Cycle_AMD_Family_10h(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		FIDVID_STATUS FidVidStatus = {.value = 0};

		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		// C-state workaround
		Core->Counter[1].C0.UCC = Core->Counter[0].C0.UCC
					+ (8 + FidVidStatus.CurrFID)
					* Core->Clock.Hz;

		Core->Counter[1].C0.URC = Core->Counter[1].C0.UCC;

		Core->Counter[1].TSC	= Core->Counter[0].TSC
					+ (Proc->Boost[1] * Core->Clock.Hz);

		/* Derive C1 */
		Core->Counter[1].C1 =
		  (Core->Counter[1].TSC > Core->Counter[1].C0.URC) ?
		    Core->Counter[1].TSC - Core->Counter[1].C0.URC
		    : 0;

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		Core_AMD_Temp(Core);

		BITSET(LOCKLESS, Core->Sync.V, 63);

		return(HRTIMER_RESTART);
	} else
		return(HRTIMER_NORESTART);
}

void InitTimer_AuthenticAMD(unsigned int cpu)
{
/*
Note: hardware Family_12h

	if(Proc->Features.AdvPower.DX.CPB == 1)	// Core Performance Boost.
	    smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_12h, 1);
	else
*/
	if (Proc->Features.Power.CX.EffFreq == 1) // MPERF & APERF ?
	    smp_call_function_single(cpu, InitTimer, Cycle_AuthenticAMD, 1);
	else
	    smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_10h, 1);
}

void Start_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_AMD_Query(Core);

	KPrivate->Join[cpu]->tsm.mustFwd = 1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started = 1;
}

void Stop_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();

	KPrivate->Join[cpu]->tsm.mustFwd = 0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	KPrivate->Join[cpu]->tsm.started = 0;
}

static enum hrtimer_restart Cycle_Core2(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Core2(Core, 1);
		Core_Intel_Temp(Core);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, 63);

		return(HRTIMER_RESTART);
	} else
		return(HRTIMER_NORESTART);
}

void InitTimer_Core2(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Core2, 1);
}

void Start_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Core2_Query(Core);

	Counters_Set(Core);
	Counters_Core2(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd = 1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started = 1;
}

void Stop_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd = 0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Counters_Clear(Core);

	KPrivate->Join[cpu]->tsm.started = 0;
}

static enum hrtimer_restart Cycle_Nehalem(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Nehalem(Core, 1);
		Core_Intel_Temp(Core);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_C3(Core);

		Delta_C6(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C3(Core);
		Save_C6(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, 63);

		return(HRTIMER_RESTART);
	} else
		return(HRTIMER_NORESTART);
}

void InitTimer_Nehalem(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Nehalem, 1);
}

void Start_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Nehalem_Query(Core);

	Counters_Set(Core);
	Counters_Nehalem(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd = 1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started = 1;
}

void Stop_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd = 0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Counters_Clear(Core);

	KPrivate->Join[cpu]->tsm.started = 0;
}


static enum hrtimer_restart Cycle_SandyBridge(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_SandyBridge(Core, 1);
		Core_Intel_Temp(Core);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_C3(Core);

		Delta_C6(Core);

		Delta_C7(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C3(Core);
		Save_C6(Core);
		Save_C7(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, 63);

		return(HRTIMER_RESTART);
	} else
		return(HRTIMER_NORESTART);
}

void InitTimer_SandyBridge(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_SandyBridge, 1);
}

void Start_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core);

	Counters_Set(Core);
	Counters_SandyBridge(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd = 1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started = 1;
}

void Stop_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd = 0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Counters_Clear(Core);

	KPrivate->Join[cpu]->tsm.started = 0;
}

long Sys_IdleDriver_Query(void)
{
	if (Proc->SysGate != NULL) {
	    struct cpuidle_driver *idleDriver;

	    if ((idleDriver = cpuidle_get_driver()) != NULL) {
		int i;

		strncpy(Proc->SysGate->IdleDriver.Name,
			idleDriver->name,
			CPUIDLE_NAME_LEN - 1);

		if (idleDriver->state_count < CPUIDLE_STATE_MAX)
			Proc->SysGate->IdleDriver.stateCount =
				idleDriver->state_count;
		else	// No overflow check.
			Proc->SysGate->IdleDriver.stateCount=CPUIDLE_STATE_MAX;

		for (i = 0; i < Proc->SysGate->IdleDriver.stateCount; i++) {
			strncpy(Proc->SysGate->IdleDriver.State[i].Name,
				idleDriver->states[i].name,
				CPUIDLE_NAME_LEN - 1);

			Proc->SysGate->IdleDriver.State[i].exitLatency =
				idleDriver->states[i].exit_latency;
			Proc->SysGate->IdleDriver.State[i].powerUsage =
				idleDriver->states[i].power_usage;
			Proc->SysGate->IdleDriver.State[i].targetResidency =
				idleDriver->states[i].target_residency;
		}
	    }
	    else
		memset(&Proc->SysGate->IdleDriver, 0, sizeof(IDLEDRIVER));

	    return(0);
	}
	else
		return(-1);
}

long Sys_Kernel(void)
{	// Source: /include/uapi/linux/utsname.h
    if (Proc->SysGate != NULL) {
	memcpy(Proc->SysGate->sysname, utsname()->sysname, MAX_UTS_LEN);
	memcpy(Proc->SysGate->release, utsname()->release, MAX_UTS_LEN);
	memcpy(Proc->SysGate->version, utsname()->version, MAX_UTS_LEN);
	memcpy(Proc->SysGate->machine, utsname()->machine, MAX_UTS_LEN);

	return(0);
    }
    else
	return(-1);
}

long Sys_DumpTask(void)
{	/// Source: /include/linux/sched.h
    if (Proc->SysGate != NULL) {
	struct task_struct *process, *thread;
	int cnt = 0;

	rcu_read_lock();
	for_each_process_thread(process, thread) {
	    task_lock(thread);

	    Proc->SysGate->taskList[cnt].runtime  = thread->se.sum_exec_runtime;
	    Proc->SysGate->taskList[cnt].usertime = thread->utime;
	    Proc->SysGate->taskList[cnt].systime  = thread->stime;
	    Proc->SysGate->taskList[cnt].state    = thread->state;
	    Proc->SysGate->taskList[cnt].wake_cpu = thread->wake_cpu;
	    Proc->SysGate->taskList[cnt].pid      = thread->pid;
	    Proc->SysGate->taskList[cnt].tgid     = thread->tgid;
	    Proc->SysGate->taskList[cnt].ppid     = thread->parent->pid;
	    memcpy(Proc->SysGate->taskList[cnt].comm,
		  thread->comm, TASK_COMM_LEN);

	    task_unlock(thread);
	    cnt++;
	}
	rcu_read_unlock();
	Proc->SysGate->taskCount = cnt;

	return(0);
    }
    else
	return(-1);
}

long Sys_MemInfo(void)
{	// Source: /include/uapi/linux/sysinfo.h
    if (Proc->SysGate != NULL) {
	struct sysinfo info;
	si_meminfo(&info);

	Proc->SysGate->memInfo.totalram  = info.totalram << (PAGE_SHIFT - 10);
	Proc->SysGate->memInfo.sharedram = info.sharedram << (PAGE_SHIFT - 10);
	Proc->SysGate->memInfo.freeram   = info.freeram << (PAGE_SHIFT - 10);
	Proc->SysGate->memInfo.bufferram = info.bufferram << (PAGE_SHIFT - 10);
	Proc->SysGate->memInfo.totalhigh = info.totalhigh << (PAGE_SHIFT - 10);
	Proc->SysGate->memInfo.freehigh  = info.freehigh << (PAGE_SHIFT - 10);

	return(0);
    }
    else
	return(-1);
}

long SysGate_OnDemand(void)
{
	long rc = -1;
	if (Proc->SysGate == NULL) {
		unsigned long pageSize = ROUND_TO_PAGES(sizeof(SYSGATE));
		// Alloc on demand
		if((Proc->SysGate = kmalloc(pageSize, GFP_KERNEL)) != NULL) {
			memset(Proc->SysGate, 0, pageSize);
			rc = 0;
		}
	}
	else	// Already allocated
		rc = 1;

	return(rc);
}

static long CoreFreqK_ioctl(struct file *filp,
			unsigned int cmd,
			unsigned long arg)
{
	long rc;
	switch (cmd) {
	case COREFREQ_IOCTL_SYSUPDT:
		rc = Sys_DumpTask() & Sys_MemInfo();
		break;
	case COREFREQ_IOCTL_SYSONCE:
		rc = Sys_IdleDriver_Query() & Sys_Kernel();
		break;
	default:
		rc = -1;
	}
	return(rc);
}

static int CoreFreqK_mmap(struct file *pfile, struct vm_area_struct *vma)
{
	if (vma->vm_pgoff == 0) {
	    if (Proc != NULL) {
	      if (remap_pfn_range(vma,
			vma->vm_start,
			virt_to_phys((void *) Proc) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot) < 0)
				return(-EIO);
	    }
	    else
		return(-EIO);
	} else if (vma->vm_pgoff == 1) {
	    if (Proc != NULL) {
		switch (SysGate_OnDemand()) {
		case -1:
			return(-EIO);
		case 1:
			// Fallthrough
		case 0:
			if (remap_pfn_range(vma,
				vma->vm_start,
				virt_to_phys((void *)Proc->SysGate)>>PAGE_SHIFT,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot) < 0)
					return(-EIO);
			break;
		}
	    }
	    else
		return(-EIO);
	} else if (vma->vm_pgoff >= 10) {
	  unsigned int cpu = vma->vm_pgoff - 10;

	  if (Proc != NULL) {
	    if ((cpu >= 0) && (cpu < Proc->CPU.Count)) {
	      if (KPublic->Core[cpu] != NULL) {
		if (remap_pfn_range(vma,
			vma->vm_start,
			virt_to_phys((void *) KPublic->Core[cpu]) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot) < 0)
				return(-EIO);
	      }
	      else
		return(-EIO);
	    }
	    else
		return(-EIO);
	  }
	  else
		return(-EIO);
	}
	else
		return(-EIO);
	return(0);
}

static DEFINE_MUTEX(CoreFreqK_mutex);		// Only one driver instance.

static int CoreFreqK_open(struct inode *inode, struct file *pfile)
{
	if (!mutex_trylock(&CoreFreqK_mutex))
		return(-EBUSY);
	else
		return(0);
}

static int CoreFreqK_release(struct inode *inode, struct file *pfile)
{
	mutex_unlock(&CoreFreqK_mutex);
	return(0);
}

static struct file_operations CoreFreqK_fops = {
	.open		= CoreFreqK_open,
	.release	= CoreFreqK_release,
	.mmap		= CoreFreqK_mmap,
	.unlocked_ioctl = CoreFreqK_ioctl,
	.owner		= THIS_MODULE,
};

#ifdef CONFIG_PM_SLEEP
static int CoreFreqK_suspend(struct device *dev)
{
	Controller_Stop();

	return(0);
}

static int CoreFreqK_resume(struct device *dev)
{
	Controller_Start();

	return(0);
}

static SIMPLE_DEV_PM_OPS(CoreFreqK_pm_ops, CoreFreqK_suspend, CoreFreqK_resume);
#define COREFREQ_PM_OPS (&CoreFreqK_pm_ops)
#else
#define COREFREQ_PM_OPS NULL
#endif


static int CoreFreqK_hotplug(	struct notifier_block *nfb,
				unsigned long action,
				void *hcpu)
{
	unsigned int cpu = (unsigned long) hcpu;

	switch (action) {
	case CPU_ONLINE:
	case CPU_DOWN_FAILED:
//-	case CPU_ONLINE_FROZEN:
		if ((cpu >= 0) && (cpu < Proc->CPU.Count)) {
		    if ((KPublic->Core[cpu]->T.ApicID == -1)
			&& !KPublic->Core[cpu]->OffLine.HW) {
			    if (!Core_Topology(cpu)) {
				if (KPublic->Core[cpu]->T.ApicID >= 0)
					KPublic->Core[cpu]->OffLine.HW = 0;
				else
					KPublic->Core[cpu]->OffLine.HW = 1;
			    }
			memcpy(&KPublic->Core[cpu]->Clock,
				&KPublic->Core[0]->Clock,
				sizeof(CLOCK) );
		    }
		    if (Arch[Proc->ArchID].Timer != NULL)
			Arch[Proc->ArchID].Timer(cpu);

		    if ((KPrivate->Join[cpu]->tsm.started == 0)
		      && (Arch[Proc->ArchID].Start != NULL))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, 0);

		    Proc->CPU.OnLine++;
		    KPublic->Core[cpu]->OffLine.OS = 0;
		}
		break;
	case CPU_DOWN_PREPARE:
//-	case CPU_DOWN_PREPARE_FROZEN:
		if ((cpu >= 0) && (cpu < Proc->CPU.Count)) {
		    if ((KPrivate->Join[cpu]->tsm.created == 1)
		      && (KPrivate->Join[cpu]->tsm.started == 1)
		      && (Arch[Proc->ArchID].Stop != NULL))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, 1);

		    Proc->CPU.OnLine--;
		    KPublic->Core[cpu]->OffLine.OS = 1;
		}
		break;
	default:
		break;
	}
	return(NOTIFY_OK);
}

static struct notifier_block CoreFreqK_notifier_block=
{
	.notifier_call = CoreFreqK_hotplug,
};

static int __init CoreFreqK_init(void)
{
	int rc = 0;
	ARG Arg = {.count = 0};
	// Query features on the presumed BSP processor.
	memset(&Arg.features, 0, sizeof(FEATURES));
	rc = smp_call_function_single(0, Query_Features, &Arg, 1);
	rc = (rc == 0) ? ((Arg.count > 0) ? 0 : -ENXIO) : rc;
	if (rc == 0)
	{
	  CoreFreqK.kcdev = cdev_alloc();
	  CoreFreqK.kcdev->ops = &CoreFreqK_fops;
	  CoreFreqK.kcdev->owner = THIS_MODULE;

	  if (alloc_chrdev_region(&CoreFreqK.nmdev, 0, 1, DRV_FILENAME) >= 0)
	  {
	    CoreFreqK.Major = MAJOR(CoreFreqK.nmdev);
	    CoreFreqK.mkdev = MKDEV(CoreFreqK.Major, 0);

	    if (cdev_add(CoreFreqK.kcdev, CoreFreqK.mkdev, 1) >= 0)
	    {
		struct device *tmpDev;

		CoreFreqK.clsdev = class_create(THIS_MODULE, DRV_DEVNAME);
		CoreFreqK.clsdev->pm = COREFREQ_PM_OPS;

		if ((tmpDev=device_create(CoreFreqK.clsdev, NULL,
					 CoreFreqK.mkdev, NULL,
					 DRV_DEVNAME)) != NULL)
		{
		  unsigned int cpu = 0;
		  unsigned long publicSize = 0,privateSize = 0,packageSize = 0;

		    publicSize = sizeof(KPUBLIC) + sizeof(CORE *) * Arg.count;

		    privateSize = sizeof(KPRIVATE) + sizeof(JOIN *) * Arg.count;

		    if (((KPublic = kmalloc(publicSize, GFP_KERNEL)) != NULL)
		     && ((KPrivate = kmalloc(privateSize, GFP_KERNEL)) != NULL))
		    {
			memset(KPublic, 0, publicSize);
			memset(KPrivate, 0, privateSize);

			packageSize = ROUND_TO_PAGES(sizeof(PROC));
			if ((Proc = kmalloc(packageSize, GFP_KERNEL)) != NULL)
			{
			    memset(Proc, 0, packageSize);
			    Proc->CPU.Count = Arg.count;

			    if ( (SleepInterval >= LOOP_MIN_MS)
			      && (SleepInterval < LOOP_MAX_MS))
				Proc->SleepInterval = SleepInterval;
			    else
				Proc->SleepInterval = LOOP_DEF_MS;

			    memcpy(&Proc->Features, &Arg.features,
					sizeof(FEATURES) );

			    Arch[0].Architecture = Proc->Features.Info.VendorID;

			    RearmTheTimer =
				ktime_set(0, Proc->SleepInterval * 1000000LU);

			    publicSize  = ROUND_TO_PAGES(sizeof(CORE));
			    privateSize = ROUND_TO_PAGES(sizeof(JOIN));

			    if (((KPublic->Cache=kmem_cache_create(
					"corefreqk-pub",
					publicSize, 0,
					SLAB_HWCACHE_ALIGN, NULL)) != NULL)
			     && ((KPrivate->Cache=kmem_cache_create(
					"corefreqk-priv",
					privateSize, 0,
					SLAB_HWCACHE_ALIGN, NULL)) != NULL))
			    {
				const CPUID_STRUCT *CpuIDforVendor =
				!strncmp(Arch[0].Architecture,VENDOR_INTEL,12)?
						CpuIDforIntel
				: !strncmp( Arch[0].Architecture,VENDOR_AMD,12)?
						CpuIDforAMD : CpuIDforIntel;

			      // Begin Loop to initialize allocation per CPU
			      for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
			      {
				void *kcache = kmem_cache_alloc(KPublic->Cache,
								GFP_KERNEL);
				memset(kcache, 0, publicSize);
				KPublic->Core[cpu] = kcache;

				kcache = kmem_cache_alloc(KPrivate->Cache,
							GFP_KERNEL);
				memset(kcache, 0, privateSize);
				KPrivate->Join[cpu] = kcache;

				BITCLR(BUS_LOCK,KPublic->Core[cpu]->Sync.V,63);

				KPublic->Core[cpu]->Bind = cpu;

				Define_CPUID(KPublic->Core[cpu],CpuIDforVendor);
			      }
			      // End Loop

			      if (!strncmp(Arch[0].Architecture,
						VENDOR_INTEL, 12)) {
					Arch[0].Query = Query_GenuineIntel;
					Arch[0].Start = Start_GenuineIntel;
					Arch[0].Stop  = Stop_GenuineIntel;
					Arch[0].Timer = InitTimer_GenuineIntel;
					Arch[0].Clock = Clock_GenuineIntel;
			      }
			      else if (!strncmp(Arch[0].Architecture,
						VENDOR_AMD, 12) ) {
					Arch[0].Query = Query_AuthenticAMD;
					Arch[0].Start = Start_AuthenticAMD;
					Arch[0].Stop  = Stop_AuthenticAMD;
					Arch[0].Timer = InitTimer_AuthenticAMD;
					Arch[0].Clock = Clock_AuthenticAMD;

					if(!Proc->Features.AdvPower.DX.Inv_TSC)
						AutoClock = 0;
			      }
			      if ( (ArchID != -1)
				&& (ArchID >= 0)
				&& (ArchID < ARCHITECTURES) ) {
					Proc->ArchID = ArchID;
			      } else {
				  for ( Proc->ArchID = ARCHITECTURES - 1;
					Proc->ArchID > 0;
					Proc->ArchID--) {
				  // Search for an architecture signature.
				    if ((Arch[Proc->ArchID].Signature.ExtFamily
					== Proc->Features.Std.AX.ExtFamily)
				    && (Arch[Proc->ArchID].Signature.Family
					== Proc->Features.Std.AX.Family)
				    && (Arch[Proc->ArchID].Signature.ExtModel
					== Proc->Features.Std.AX.ExtModel)
				    && (Arch[Proc->ArchID].Signature.Model
					== Proc->Features.Std.AX.Model)) {
						break;
					}
				  }
			      }

				strncpy(Proc->Architecture,
					Arch[Proc->ArchID].Architecture, 32);

				Controller_Init();

				printk(KERN_INFO "CoreFreq:"		\
				      " Processor [%1X%1X_%1X%1X]"	\
				      " Architecture [%s] CPU [%u/%u]\n",
					Proc->Features.Std.AX.ExtFamily,
					Proc->Features.Std.AX.Family,
					Proc->Features.Std.AX.ExtModel,
					Proc->Features.Std.AX.Model,
					Arch[Proc->ArchID].Architecture,
					Proc->CPU.OnLine,
					Proc->CPU.Count);

				Controller_Start();

			    if (Experimental)
				pci_register_driver(&CoreFreqK_pci_driver);

			    register_hotcpu_notifier(&CoreFreqK_notifier_block);

			    } else {
				if (KPublic->Cache != NULL)
					kmem_cache_destroy(KPublic->Cache);
				if (KPrivate->Cache != NULL)
					kmem_cache_destroy(KPrivate->Cache);

				kfree(Proc);
				kfree(KPublic);
				kfree(KPrivate);

				device_destroy(CoreFreqK.clsdev,
					       CoreFreqK.mkdev);
				class_destroy(CoreFreqK.clsdev);
				cdev_del(CoreFreqK.kcdev);
				unregister_chrdev_region(CoreFreqK.mkdev, 1);

				rc = -ENOMEM;
			    }
			} else {
			    kfree(KPublic);
			    kfree(KPrivate);

			    device_destroy(CoreFreqK.clsdev,
					   CoreFreqK.mkdev);
			    class_destroy(CoreFreqK.clsdev);
			    cdev_del(CoreFreqK.kcdev);
			    unregister_chrdev_region(CoreFreqK.mkdev, 1);

			    rc = -ENOMEM;
			}
		    } else {
			if(KPublic != NULL)
				kfree(KPublic);
			if(KPrivate != NULL)
				kfree(KPrivate);

			device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
			class_destroy(CoreFreqK.clsdev);
			cdev_del(CoreFreqK.kcdev);
			unregister_chrdev_region(CoreFreqK.mkdev, 1);

			rc = -ENOMEM;
		    }
		} else {
		    class_destroy(CoreFreqK.clsdev);
		    cdev_del(CoreFreqK.kcdev);
		    unregister_chrdev_region(CoreFreqK.mkdev, 1);

		    rc = -EBUSY;
		}
	    } else {
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		rc = -EBUSY;
	    }
	  } else {
	    cdev_del(CoreFreqK.kcdev);

	    rc = -EBUSY;
	  }
	}
	return(rc);
}

static void __exit CoreFreqK_cleanup(void)
{
	unsigned int cpu = 0;

	unregister_hotcpu_notifier(&CoreFreqK_notifier_block);

	if (Experimental)
		pci_unregister_driver(&CoreFreqK_pci_driver);

	Controller_Stop();
	Controller_Exit();

	for (cpu = 0;(KPublic->Cache != NULL) && (cpu < Proc->CPU.Count); cpu++)
	{
		if (KPublic->Core[cpu] != NULL)
			kmem_cache_free(KPublic->Cache,	KPublic->Core[cpu]);
		if (KPrivate->Join[cpu] != NULL)
			kmem_cache_free(KPrivate->Cache, KPrivate->Join[cpu]);
	}
	if (KPublic->Cache != NULL)
		kmem_cache_destroy(KPublic->Cache);
	if (KPrivate->Cache != NULL)
		kmem_cache_destroy(KPrivate->Cache);
	if (Proc != NULL) {
		if (Proc->SysGate != NULL)
			kfree(Proc->SysGate);
		kfree(Proc);
	}
	if (KPublic != NULL)
		kfree(KPublic);
	if (KPrivate != NULL)
		kfree(KPrivate);

	device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
	class_destroy(CoreFreqK.clsdev);
	cdev_del(CoreFreqK.kcdev);
	unregister_chrdev_region(CoreFreqK.mkdev, 1);

	printk(KERN_NOTICE "CoreFreq: Unload\n");
}

module_init(CoreFreqK_init);
module_exit(CoreFreqK_cleanup);
