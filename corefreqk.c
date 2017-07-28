/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <linux/version.h>
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif
#include <asm/msr.h>
#include <asm/nmi.h>

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

static signed int Experimental = 0;
module_param(Experimental, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Experimental, "Enable features under development");

static signed short PkgCStateLimit = -1;
module_param(PkgCStateLimit, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PkgCStateLimit, "Package C-State Limit");

static signed short SpeedStepEnable = -1;
module_param(SpeedStepEnable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SpeedStepEnable, "Enable SpeedStep");

static signed short C1E_Enable = -1;
module_param(C1E_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1E_Enable, "Enable SpeedStep C1E");

static signed short TurboBoostEnable = -1;
module_param(TurboBoostEnable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TurboBoostEnable, "Enable Turbo Boost");

static signed short C3A_Enable = -1;
module_param(C3A_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C3A_Enable, "Enable C3 Auto Demotion");

static signed short C1A_Enable = -1;
module_param(C1A_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1A_Enable, "Enable C3 Auto Demotion");

static signed short C3U_Enable = -1;
module_param(C3U_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C3U_Enable, "Enable C3 UnDemotion");

static signed short C1U_Enable = -1;
module_param(C1U_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1U_Enable, "Enable C1 UnDemotion");

static signed short ODCM_DutyCycle = -1;
module_param(ODCM_DutyCycle, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ODCM_DutyCycle, "On-Demand Clock Modulation DutyCycle [0-7]");

static signed short PowerMGMT_Unlock = -1;
module_param(PowerMGMT_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerMGMT_Unlock, "Unlock Power Management");

static signed short PowerPolicy = -1;
module_param(PowerPolicy, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerPolicy, "Power Policy Preference [0-15]");

static PROC *Proc = NULL;
static KPUBLIC *KPublic = NULL;
static KPRIVATE *KPrivate = NULL;
static ktime_t RearmTheTimer;


unsigned int Intel_Brand(char *pBrand)
{
	char idString[64] = {0x20};
	unsigned long ix = 0, jx = 0, px = 0;
	unsigned int frequency = 0, multiplier = 0;
	BRAND Brand;

	for (ix = 0; ix < 3; ix++) {
		asm volatile
		(
			"movq	%4,    %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r"  (Brand.AX),
			  "=r"  (Brand.BX),
			  "=r"  (Brand.CX),
			  "=r"  (Brand.DX)
			: "r"   (0x80000002 + ix)
			: "%rax", "%rbx", "%rcx", "%rdx"
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
	unsigned long ix = 0, jx = 0, px = 0;
	BRAND Brand;

	for (ix = 0; ix < 3; ix++) {
		asm volatile
		(
			"movq	%4,    %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r"  (Brand.AX),
			  "=r"  (Brand.BX),
			  "=r"  (Brand.CX),
			  "=r"  (Brand.DX)
			: "r"   (0x80000002 + ix)
			: "%rax", "%rbx", "%rcx", "%rdx"
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
		"xorq	%%rax, %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (arg->features.Info.LargestStdFunc),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
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
		"movq	$0x1,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (arg->features.Std.AX),
		  "=r" (arg->features.Std.BX),
		  "=r" (arg->features.Std.CX),
		  "=r" (arg->features.Std.DX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	if (arg->features.Info.LargestStdFunc >= 0x5) {
		asm volatile
		(
			"movq	$0x5,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (arg->features.MWait.AX),
			  "=r" (arg->features.MWait.BX),
			  "=r" (arg->features.MWait.CX),
			  "=r" (arg->features.MWait.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	if (arg->features.Info.LargestStdFunc >= 0x6) {
		asm volatile
		(
			"movq	$0x6,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (arg->features.Power.AX),
			  "=r" (arg->features.Power.BX),
			  "=r" (arg->features.Power.CX),
			  "=r" (arg->features.Power.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	if (arg->features.Info.LargestStdFunc >= 0x7) {
		asm volatile
		(
			"movq	$0x7,  %%rax	\n\t"
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
		"movq	$0x80000000, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (arg->features.Info.LargestExtFunc),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	asm volatile
	(
		"movq	$0x80000001, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (eax),
		  "=r" (ebx),
		  "=r" (arg->features.ExtInfo.CX),
		  "=r" (arg->features.ExtInfo.DX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	if (arg->features.Info.LargestExtFunc >= 0x80000007) {
		asm volatile
		(
			"movq	$0x80000007, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (arg->features.AdvPower.AX),
			  "=r" (arg->features.AdvPower.BX),
			  "=r" (arg->features.AdvPower.CX),
			  "=r" (arg->features.AdvPower.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
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
			"movq	$0x4,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (eax),
			  "=r" (ebx),
			  "=r" (ecx),
			  "=r" (edx)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		arg->count = (eax >> 26) & 0x3f;
		arg->count++;

	    if (arg->features.Info.LargestStdFunc >= 0xa) {
		asm volatile
		(
			"movq	$0xa,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (arg->features.PerfMon.AX),
			  "=r" (arg->features.PerfMon.BX),
			  "=r" (arg->features.PerfMon.CX),
			  "=r" (arg->features.PerfMon.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
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
					"movq	$0x80000008, %%rax	\n\t"
					"xorq	%%rbx, %%rbx		\n\t"
					"xorq	%%rcx, %%rcx		\n\t"
					"xorq	%%rdx, %%rdx		\n\t"
					"cpuid				\n\t"
					"mov	%%eax, %0		\n\t"
					"mov	%%ebx, %1		\n\t"
					"mov	%%ecx, %2		\n\t"
					"mov	%%edx, %3"
					: "=r" (eax),
					  "=r" (ebx),
					  "=r" (ecx),
					  "=r" (edx)
					:
					: "%rax", "%rbx", "%rcx", "%rdx"
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
	struct kmem_cache *hardwareCache = NULL;
	/*
		TSC[0] stores the overhead
		TSC[1] stores the estimation
	*/
	TSC_STRUCT *TSC[2] = {NULL, NULL};
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

	// Allocate Cache aligned resources.
	hardwareCache = kmem_cache_create("CoreFreqCache",
					STRUCT_SIZE, 0,
					SLAB_HWCACHE_ALIGN, NULL);
	if (hardwareCache != NULL) {
	  TSC[0] = kmem_cache_alloc(hardwareCache, GFP_KERNEL);
	  if (TSC[0] != NULL) {
	    TSC[1] = kmem_cache_alloc(hardwareCache, GFP_KERNEL);
	    if (TSC[1] != NULL) {

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
			    if ((count[what] > top[what])
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
	    }
	    kmem_cache_free(hardwareCache, TSC[0]);
	  }
	  kmem_cache_destroy(hardwareCache);
	}
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
			"movq	$0x16, %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (eax),
			  "=r" (ebx),
			  "=r" (fsb),
			  "=r" (edx)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
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
	unsigned long level = 0x0;
	if (!strncmp(Proc->Features.Info.VendorID, VENDOR_INTEL, 12)) {
	    for (level = 0; level < CACHE_MAX_LEVEL; level++) {
		asm volatile
		(
			"movq	$0x4,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"movq	%4,    %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (Core->T.Cache[level].AX),
			  "=r" (Core->T.Cache[level].BX),
			  "=r" (Core->T.Cache[level].Set),
			  "=r" (Core->T.Cache[level].DX)
			: "r" (level)
			: "%rax", "%rbx", "%rcx", "%rdx"
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
			"movq	$0x80000005, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (CacheInfo.AX),
			  "=r" (CacheInfo.BX),
			  "=r" (CacheInfo.CX),
			  "=r" (CacheInfo.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
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
			"movq	$0x80000006, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (CacheInfo.AX),
			  "=r" (CacheInfo.BX),
			  "=r" (CacheInfo.CX),
			  "=r" (CacheInfo.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
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
		unsigned int eax = 0x0, ecx = 0x0, edx = 0x0;
		CORE *Core = (CORE *) arg;
		FEATURES features;

		RDMSR(Core->T.Base, MSR_IA32_APICBASE);

		asm volatile
		(
			"movq	$0x1,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (eax),
			  "=r" (features.Std.BX),
			  "=r" (ecx),
			  "=r" (edx)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);

		Core->T.CoreID = Core->T.ApicID=features.Std.BX.Apic_ID;

		Cache_Topology(Core);
	}
}

void Map_Extended_Topology(void *arg)
{
	if (arg != NULL) {
		CORE *Core = (CORE *) arg;

		long	InputLevel = 0;
		int	NoMoreLevels = 0,
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
				"movq	$0xb,  %%rax	\n\t"
				"xorq	%%rbx, %%rbx	\n\t"
				"movq	%4,    %%rcx	\n\t"
				"xorq	%%rdx, %%rdx	\n\t"
				"cpuid			\n\t"
				"mov	%%eax, %0	\n\t"
				"mov	%%ebx, %1	\n\t"
				"mov	%%ecx, %2	\n\t"
				"mov	%%edx, %3"
				: "=r" (ExtTopology.AX),
				  "=r" (ExtTopology.BX),
				  "=r" (ExtTopology.CX),
				  "=r" (ExtTopology.DX)
				: "r" (InputLevel)
				: "%rax", "%rbx", "%rcx", "%rdx"
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
			"movq	$0x6,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (Power.AX),
			  "=r" (Power.BX),
			  "=r" (Power.CX),
			  "=r" (Power.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
}

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

PCI_CALLBACK Router(	struct pci_dev *dev, unsigned int offset,
			unsigned long long wsize, ROUTER route)
{
	void __iomem *mchmap;
	union {
		unsigned long long addr;
		struct {
			unsigned int low;
			unsigned int high;
		};
	} mchbar;
	unsigned long long wmask = BITCPL(wsize);

	Proc->Uncore.ChipID = dev->device;

	pci_read_config_dword(dev, offset    , &mchbar.low);
	pci_read_config_dword(dev, offset + 4, &mchbar.high);

	mchbar.addr &= wmask;

	mchmap = ioremap(mchbar.addr, wsize);
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

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P965.CKE0.value = readl(mchmap + 0x260);
	Proc->Uncore.MC[0].P965.CKE1.value = readl(mchmap + 0x660);

	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].P965.CKE0.RankPop0 != 0)
				+ (Proc->Uncore.MC[0].P965.CKE1.RankPop0 != 0);

	Proc->Uncore.MC[0].SlotCount =
			  (Proc->Uncore.MC[0].P965.CKE0.SingleDimmPop ? 1 : 2)
			+ (Proc->Uncore.MC[0].P965.CKE1.SingleDimmPop ? 1 : 2);

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
	unsigned short cha, slot;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].G965.DRB0.value = readl(mchmap + 0x1200);
	Proc->Uncore.MC[0].G965.DRB1.value = readl(mchmap + 0x1300);

	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].G965.DRB0.Rank1Addr != 0)
				+ (Proc->Uncore.MC[0].G965.DRB1.Rank1Addr != 0);

	Proc->Uncore.MC[0].SlotCount=Proc->Uncore.MC[0].ChannelCount > 1 ? 1:2;

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].G965.DRT0.value =
					readl(mchmap + 0x1210 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT1.value =
					readl(mchmap + 0x1214 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT2.value =
					readl(mchmap + 0x1218 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT3.value =
					readl(mchmap + 0x121c + 0x100 * cha);

		for (slot = 0; slot < Proc->Uncore.MC[0].SlotCount; slot++) {
			Proc->Uncore.MC[0].Channel[cha].DIMM[slot].DRA.value =
				readl(mchmap + 0x1208 + 0x100 * cha);
		}
	}
}

void Query_P35(void __iomem *mchmap)
{	// Source: Intel® 3 Series Express Chipset Family
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P35.CKE0.value = readl(mchmap + 0x260);
	Proc->Uncore.MC[0].P35.CKE1.value = readl(mchmap + 0x660);

	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].P35.CKE0.RankPop0 != 0)
				+ (Proc->Uncore.MC[0].P35.CKE1.RankPop0 != 0);

	Proc->Uncore.MC[0].SlotCount =
			  (Proc->Uncore.MC[0].P35.CKE0.SingleDimmPop ? 1 : 2)
			+ (Proc->Uncore.MC[0].P35.CKE1.SingleDimmPop ? 1 : 2);

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

kernel_ulong_t Query_NHM_Timing(unsigned int did,
				unsigned short mc,
				unsigned short cha)
{	// Source: Micron Technical Note DDR3 Power-Up, Initialization, & Reset
    struct pci_dev *dev = pci_get_device(PCI_VENDOR_ID_INTEL, did, NULL);
    if (dev != NULL) {
	pci_read_config_dword(dev, 0x70,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.MR0_1.value);

	pci_read_config_dword(dev, 0x74,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.MR2_3.value);

	pci_read_config_dword(dev ,0x80,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.value);

	pci_read_config_dword(dev ,0x84,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.value);

	pci_read_config_dword(dev ,0x88,
			      &Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.value);

	pci_read_config_dword(dev ,0x8c,
			   &Proc->Uncore.MC[mc].Channel[cha].NHM.Refresh.value);

	pci_read_config_dword(dev, 0xb8,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.Params.value);

	pci_dev_put(dev);
	return(0);
    } else
	return(-ENODEV);
}

kernel_ulong_t Query_NHM_DIMM(	unsigned int did,
				unsigned short mc,
				unsigned short cha)
{
	struct pci_dev *dev = pci_get_device(PCI_VENDOR_ID_INTEL, did, NULL);
	if (dev != NULL) {
		unsigned short slot;

		for (slot = 0; slot < Proc->Uncore.MC[mc].SlotCount; slot++) {
		    pci_read_config_dword(dev, 0x48 + 4 * slot,
			&Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.value);
		}
		pci_dev_put(dev);
		return(0);
	} else
		return(-ENODEV);
}

void Query_NHM_MaxDIMMs(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(	dev, 0x64,
				&Proc->Uncore.MC[mc].MaxDIMMs.NHM.DOD.value);

	switch (Proc->Uncore.MC[mc].MaxDIMMs.NHM.DOD.MAXNUMDIMMS) {
	case 0b00:
		Proc->Uncore.MC[mc].SlotCount = 1;
		break;
	case 0b01:
		Proc->Uncore.MC[mc].SlotCount = 2;
		break;
	case 0b10:
		Proc->Uncore.MC[mc].SlotCount = 3;
		break;
	default:
		Proc->Uncore.MC[mc].SlotCount = 0;
		break;
	}
}

kernel_ulong_t Query_Bloomfield_IMC(struct pci_dev *dev, unsigned short mc)
{
	kernel_ulong_t rc = 0;
	unsigned int did[2][3] = {
		{
			PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL,
			PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL,
			PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL
		},
		{
			PCI_DEVICE_ID_INTEL_I7_MC_CH0_ADDR,
			PCI_DEVICE_ID_INTEL_I7_MC_CH1_ADDR,
			PCI_DEVICE_ID_INTEL_I7_MC_CH2_ADDR
		}
	};
	unsigned short cha;

	Query_NHM_MaxDIMMs(dev, mc);

	pci_read_config_dword(dev,0x48, &Proc->Uncore.MC[mc].NHM.CONTROL.value);
	pci_read_config_dword(dev,0x4c, &Proc->Uncore.MC[mc].NHM.STATUS.value);

	Proc->Uncore.MC[mc].ChannelCount =
		  (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL0_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL2_ACTIVE != 0);

	for (cha = 0; (cha < Proc->Uncore.MC[mc].ChannelCount) && !rc; cha++) {
		rc = Query_NHM_Timing(did[0][cha], mc, cha)
		   & Query_NHM_DIMM(did[1][cha], mc, cha);
	}
	return(rc);
}

kernel_ulong_t Query_Lynnfield_IMC(struct pci_dev *dev, unsigned short mc)
{
	kernel_ulong_t rc = 0;
	unsigned int did[2][2] = {
		{
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_CTRL,
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_CTRL
		},
		{
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_ADDR,
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_ADDR
		}
	};
	unsigned short cha;

	Query_NHM_MaxDIMMs(dev, mc);

	pci_read_config_dword(dev,0x48, &Proc->Uncore.MC[mc].NHM.CONTROL.value);
	pci_read_config_dword(dev,0x4c, &Proc->Uncore.MC[mc].NHM.STATUS.value);

	Proc->Uncore.MC[mc].ChannelCount =
		  (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL0_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0);

	for (cha = 0; (cha < Proc->Uncore.MC[mc].ChannelCount) && !rc; cha++) {
		rc = Query_NHM_Timing(did[0][cha], mc, cha)
		   & Query_NHM_DIMM(did[1][cha], mc, cha);
	}
	return(rc);
}

void Query_C200(void __iomem *mchmap)
{	// Source: Intel® Xeon Processor E3-1200 Family
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;

/* ToDo */
	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].C200.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].C200.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].C200.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD0.Dimm_B_Size != 0))  /*0 or 1*/
		+ ((Proc->Uncore.MC[0].C200.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD1.Dimm_B_Size != 0)); /*0 or 1*/

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

	Proc->Uncore.CtrlCount = 1;

/* ToDo */
	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].C200.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].C200.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].C200.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD0.Dimm_B_Size != 0))
		+ ((Proc->Uncore.MC[0].C200.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].C200.MAD1.Dimm_B_Size != 0));

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].C220.Timing.value =
					readl(mchmap + 0x4c04 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].C220.Rank.value =
					readl(mchmap + 0x4c14 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].C220.Refresh.value =
					readl(mchmap + 0x4e98 + 0x400 * cha);
	}
}

PCI_CALLBACK P965(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x4000, Query_P965));
}

PCI_CALLBACK G965(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x4000, Query_G965));
}

PCI_CALLBACK P35(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x4000, Query_P35));
}

PCI_CALLBACK Bloomfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_Bloomfield_IMC(dev, mc);

	return((PCI_CALLBACK) rc);
}

PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_Lynnfield_IMC(dev, mc);

	return((PCI_CALLBACK) rc);
}

PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev)
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
	return(Router(dev, 0x48, 0x8000, Query_C200));
}

PCI_CALLBACK C220(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x8000, Query_C220));
}

PCI_CALLBACK AMD_0F_MCH(struct pci_dev *dev)
{	// Source: BKDG for AMD NPT Family 0Fh Processors
	unsigned short cha, slot, chip;

	Proc->Uncore.ChipID = dev->device;
	// Specs defined
	Proc->Uncore.CtrlCount = 1;
	// DRAM Configuration low register
	pci_read_config_dword(dev, 0x90,
			&Proc->Uncore.MC[0].AMD0F.DCRL.value);
	// DRAM Configuration high register
	pci_read_config_dword(dev, 0x94,
			&Proc->Uncore.MC[0].AMD0F.DCRH.value);
	// 1 channel if 64 bits / 2 channels if 128 bits width
	Proc->Uncore.MC[0].ChannelCount = Proc->Uncore.MC[0].AMD0F.DCRL.Width128
					+ 1;
	// DIMM Geometry
	for (chip = 0; chip < 8; chip++) {
		cha = chip >> 2;
		slot = chip % 4;
		pci_read_config_dword(dev, 0x40 + 4 * chip,
			&Proc->Uncore.MC[0].Channel[cha].DIMM[slot].MBA.value);

		Proc->Uncore.MC[0].SlotCount +=
			Proc->Uncore.MC[0].Channel[cha].DIMM[slot].MBA.CSEnable;
	}
	// DIMM Size
	pci_read_config_dword(	dev, 0x80,
				&Proc->Uncore.MC[0].MaxDIMMs.AMD0F.CS.value);
	// DRAM Timings
	pci_read_config_dword(dev, 0x88,
			&Proc->Uncore.MC[0].Channel[0].AMD0F.DTRL.value);
	// Assume same timings for both channels
	Proc->Uncore.MC[0].Channel[1].AMD0F.DTRL.value =
			Proc->Uncore.MC[0].Channel[0].AMD0F.DTRL.value;

	return(0);
}

PCI_CALLBACK AMD_0F_HTT(struct pci_dev *dev)
{
	unsigned int link;

	pci_read_config_dword(dev, 0x64, &Proc->Uncore.Bus.UnitID.value);

	for (link = 0; link < 3; link++) {
		pci_read_config_dword(dev, 0x88 + 0x20 * link,
				&Proc->Uncore.Bus.LDTi_Freq[link].value);
	};

	return(0);
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
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82946GZ_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// Q963/Q965 - Broadwater
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965Q_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// P965/G965 - Broadwater
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965G_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// GM965 - Crestline
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965GM_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// GME965
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965GME_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// GM45 - Cantiga
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_GM45_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// Q35 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// P35/G33 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Q33 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// X38/X48 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X38_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// 3200/3210 - Unknown
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_3200_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Q45/Q43 - Unknown
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// P45/G45 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// G41 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G41_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	// 1st Generation
	{	// Bloomfield IMC
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MCR),
		.driver_data = (kernel_ulong_t) Bloomfield_IMC
	},
	{	// Bloomfield IMC Test Registers
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	// Nehalem Control Status and RAS Registers
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X58_HUB_CTRL),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{	// Lynnfield IMC
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR),
		.driver_data = (kernel_ulong_t) Lynnfield_IMC
	},
	{	// Lynnfield IMC Test Registers
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	// 2nd Generation
	// Sandy Bridge ix-2xxx, Xeon E3-E5: IMC_HA=0x3ca0 / IMC_TA=0x3ca8 /
	// TA0=0x3caa, TA1=0x3cab / TA2=0x3cac / TA3=0x3cad / TA4=0x3cae
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0),
		.driver_data = (kernel_ulong_t) C200
	},
	// 3rd Generation
	// Ivy Bridge ix-3xxx, Xeon E7/E5 v2: IMC_HA=0x0ea0 / IMC_TA=0x0ea8
	// TA0=0x0eaa / TA1=0x0eab / TA2=0x0eac / TA3=0x0ead
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0),
		.driver_data = (kernel_ulong_t) C200
	},
	// 4th Generation
	// Haswell ix-4xxx, Xeon E7/E5 v3: IMC_HA0=0x2fa0 / IMC_HA0_TA=0x2fa8
	// TAD0=0x2faa / TAD1=0x2fab / TAD2=0x2fac / TAD3=0x2fad
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) C220
	},
	// AMD Family 0Fh
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_K8_NB_MEMCTL),
		.driver_data = (kernel_ulong_t) AMD_0F_MCH
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_K8_NB),
		.driver_data = (kernel_ulong_t) AMD_0F_HTT
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
		"xorq	%%rax, %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (Core->Query.StdFunc.LargestStdFunc),
		  "=r" (Core->Query.StdFunc.BX),
		  "=r" (Core->Query.StdFunc.CX),
		  "=r" (Core->Query.StdFunc.DX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	asm volatile
	(
		"movq	$0x80000000, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (Core->Query.ExtFunc.LargestExtFunc),
		  "=r" (Core->Query.ExtFunc.BX),
		  "=r" (Core->Query.ExtFunc.CX),
		  "=r" (Core->Query.ExtFunc.DX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	for (i = 0; (i < CPUID_MAX_FUNC) && (Core->CpuID[i].func != 0x0); i++) {
		asm volatile
		(
			"xorq	%%rax, %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"mov	%4,    %%eax	\n\t"
			"mov	%5,    %%ecx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (Core->CpuID[i].reg[0]),
			  "=r" (Core->CpuID[i].reg[1]),
			  "=r" (Core->CpuID[i].reg[2]),
			  "=r" (Core->CpuID[i].reg[3])
			:  "r" (Core->CpuID[i].func),
			   "r" (Core->CpuID[i].sub)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
}

void SpeedStep_Technology(CORE *Core)				// Per Package!
{
	if (Proc->Features.Std.CX.EIST == 1) {
		MISC_PROC_FEATURES MiscFeatures = {.value = 0};
		RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

		switch (SpeedStepEnable) {
		    case 0:
		    case 1:
			if ((Core->T.CoreID == 0) && (Core->T.ThreadID == 0)) {
				MiscFeatures.EIST = SpeedStepEnable;
				WRMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
				RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
			}
		    break;
		}
		Core->Query.EIST = MiscFeatures.EIST;
	} else {
		Core->Query.EIST = 0;
	}
}

void TurboBoost_Technology(CORE *Core)
{
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	if (MiscFeatures.Turbo_IDA == 0) {
		PERF_CONTROL PerfControl = {.value = 0};
		RDMSR(PerfControl, MSR_IA32_PERF_CTL);

		switch (TurboBoostEnable) {			// Per Thread
		    case 0:
		    case 1:
			PerfControl.Turbo_IDA = !TurboBoostEnable;
			WRMSR(PerfControl, MSR_IA32_PERF_CTL);
			RDMSR(PerfControl, MSR_IA32_PERF_CTL);
		    break;
		}
		Core->Query.Turbo = !PerfControl.Turbo_IDA;
	} else {
		Core->Query.Turbo = 0;
	}
}

void Query_Intel_C1E(CORE *Core)
{
	POWER_CONTROL PowerCtrl = {.value = 0};
	RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);			// Per Core

	switch (C1E_Enable) {					// Per Package
		case 0:
		case 1:
			if ((Core->T.CoreID == 0) && (Core->T.ThreadID == 0)) {
				PowerCtrl.C1E = C1E_Enable;
				WRMSR(PowerCtrl, MSR_IA32_POWER_CTL);
				RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);
			}
		break;
	}
	Core->Query.C1E = PowerCtrl.C1E;
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
		"movq	$0x6,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (Power.AX),
		  "=r" (Power.BX),
		  "=r" (Power.CX),
		  "=r" (Power.DX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	if (Power.CX.SETBH == 1) {
	  RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);

	  if ((PowerPolicy >= 0) && (PowerPolicy <= 15)) {
	    Core->PowerThermal.PerfEnergyBias.PowerPolicy = PowerPolicy;
	    WRMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	    RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	  }
	} else {
	  RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);

	  if (Experimental == 1) {
	    switch (PowerMGMT_Unlock) {
	    case 0:
		Core->PowerThermal.PwrManagement.Perf_BIAS_Enable = 0;
		WRMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
		break;
	    case 1:
		Core->PowerThermal.PwrManagement.Perf_BIAS_Enable = 1;
		WRMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
		break;
	    }
	  }
	}
	RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);

	if (Proc->Features.Std.DX.ACPI == 1) {
	  RDMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);
	  Core->PowerThermal.ClockModulation.ExtensionBit = Power.AX.ECMD;

	 if (Experimental == 1) {
	  if ((ODCM_DutyCycle >= 0) && (ODCM_DutyCycle <= 7)) {
	    if (ODCM_DutyCycle > 0)
		Core->PowerThermal.ClockModulation.ODCM_Enable = 1;
	    else
		Core->PowerThermal.ClockModulation.ODCM_Enable = 0;
	    Core->PowerThermal.ClockModulation.ODCM_DutyCycle = ODCM_DutyCycle;
	    WRMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);
	    RDMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);
	  }
	 }
	}
    }
}

void Microcode(CORE *Core)
{
	MICROCODE_ID Microcode = {.value = 0};
	RDMSR(Microcode, MSR_IA32_UCODE_REV);
	Core->Query.Microcode = Microcode.Signature;
}

void PerCore_Intel_Query(CORE *Core)
{
	Microcode(Core);

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
	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	PowerThermal(Core);					// Shared/Unique

	ThermalMonitor_Set(Core);
}

void PerCore_Nehalem_Query(CORE *Core)
{
	CSTATE_CONFIG CStateConfig = {.value = 0};

	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	TurboBoost_Technology(Core);
	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				// Per Core
		int ToggleDemotion = 0;

		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

		switch (C3A_Enable) {
			case 0:
			case 1:
				CStateConfig.C3autoDemotion = C3A_Enable;
				ToggleDemotion = 1;
			break;
		}
		switch (C1A_Enable) {
			case 0:
			case 1:
				CStateConfig.C1autoDemotion = C1A_Enable;
				ToggleDemotion = 1;
			break;
		}
		if (ToggleDemotion == 1) {
			WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
			RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		}
		Core->Query.C3A = CStateConfig.C3autoDemotion;
		Core->Query.C1A = CStateConfig.C1autoDemotion;

		if (CStateConfig.CFG_Lock == 0) {
			switch (PkgCStateLimit) {
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b100;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b011;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 3: // Cannot be used to limit package C-state to C3
				CStateConfig.Pkg_CStateLimit = 0b010;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 1:
				CStateConfig.Pkg_CStateLimit = 0b001;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b000;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			}
			if (PkgCStateLimit != -1) {
				RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
			}
		}
		Core->Query.CfgLock = CStateConfig.CFG_Lock;

		switch (CStateConfig.Pkg_CStateLimit) {
		case 0b100:
			Core->Query.CStateLimit = 7;
			break;
		case 0b011:
			Core->Query.CStateLimit = 6;
			break;
		case 0b010:
			Core->Query.CStateLimit = 3;
			break;
		case 0b001:
			Core->Query.CStateLimit = 1;
			break;
		case 0b000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}

		if (CStateConfig.IO_MWAIT_Redir) {
			CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};

			RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);

			switch (CState_IO_MWAIT.CStateRange) {
			case 0b010:
				Core->Query.CStateInclude = 7;
				break;
			case 0b001:
				Core->Query.CStateInclude = 6;
				break;
			case 0b000:
				Core->Query.CStateInclude = 3;
				break;
			default:
				Core->Query.CStateInclude = 0;
				break;
			}
		}
		Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;
	}
	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

void PerCore_SandyBridge_Query(CORE *Core)
{
	CSTATE_CONFIG CStateConfig = {.value = 0};

	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	TurboBoost_Technology(Core);
	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				// Per Core
		int ToggleDemotion = 0;

		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

		switch (C3A_Enable) {
			case 0:
			case 1:
				CStateConfig.C3autoDemotion = C3A_Enable;
				ToggleDemotion = 1;
			break;
		}
		switch (C1A_Enable) {
			case 0:
			case 1:
				CStateConfig.C1autoDemotion = C1A_Enable;
				ToggleDemotion = 1;
			break;
		}
		switch (C3U_Enable) {
			case 0:
			case 1:
				CStateConfig.C3undemotion = C3U_Enable;
				ToggleDemotion = 1;
			break;
		}
		switch (C1U_Enable) {
			case 0:
			case 1:
				CStateConfig.C1undemotion = C1U_Enable;
				ToggleDemotion = 1;
			break;
		}
		if (ToggleDemotion == 1) {
			WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
			RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		}
		Core->Query.C3A = CStateConfig.C3autoDemotion;
		Core->Query.C1A = CStateConfig.C1autoDemotion;
		Core->Query.C3U = CStateConfig.C3undemotion;
		Core->Query.C1U = CStateConfig.C1undemotion;

		if (CStateConfig.CFG_Lock == 0) {
			switch (PkgCStateLimit) {
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b100;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b010;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 2:
				CStateConfig.Pkg_CStateLimit = 0b001;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			case 1:
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b000;
				WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
				break;
			}
			if (PkgCStateLimit != -1) {
				RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
			}
		}
		Core->Query.CfgLock = CStateConfig.CFG_Lock;

		switch (CStateConfig.Pkg_CStateLimit) {
		case 0b101:
		case 0b100:
			Core->Query.CStateLimit = 7;
			break;
		case 0b011:
		case 0b010:
			Core->Query.CStateLimit = 6;
			break;
		case 0b001:
			Core->Query.CStateLimit = 2;
			break;
		case 0b000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}

		if (CStateConfig.IO_MWAIT_Redir) {
			CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};

			RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);

			switch (CState_IO_MWAIT.CStateRange) {
			case 0b010:
				Core->Query.CStateInclude = 7;
				break;
			case 0b001:
				Core->Query.CStateInclude = 6;
				break;
			case 0b000:
				Core->Query.CStateInclude = 3;
				break;
			default:
				Core->Query.CStateInclude = 0;
				break;
			}
		}
		Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;
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
	CLOCK clock = {.Q = 0, .R = 0, .Hz = 0};
	unsigned int cpu = Proc->CPU.Count;

	if (Arch[Proc->ArchID].Query != NULL)
		Arch[Proc->ArchID].Query();

	do {	// from last AP to BSP
	    cpu--;

	    if (!KPublic->Core[cpu]->OffLine.OS) {
		if (AutoClock) {
			clock = Base_Clock(cpu, Proc->Boost[1]);
		} // else ENOMEM
		if ((clock.Hz == 0) && (Arch[Proc->ArchID].Clock != NULL))
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

#define SMT_Counters_Nehalem(Core, T)					\
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

#define SMT_Counters_SandyBridge(Core, T)				\
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

#define Delta_SMI(Core)							\
({	/* Delta of SMI interrupt. */					\
	Core->Delta.SMI = Core->Counter[1].SMI				\
			- Core->Counter[0].SMI;				\
})

#define PKG_Counters_Nehalem(Core, T)					\
({									\
	RDTSCP_COUNTERx3(Proc->Counter[T].PTSC,				\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07);	\
})

#define PKG_Counters_SandyBridge(Core, T)				\
({									\
	RDTSCP_COUNTERx4(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07);	\
})

#define Delta_PTSC(Pkg)							\
({									\
	Pkg->Delta.PTSC = Pkg->Counter[1].PTSC				\
			- Pkg->Counter[0].PTSC;				\
})

#define Delta_PC02(Pkg)							\
({									\
	Pkg->Delta.PC02 = Pkg->Counter[1].PC02				\
			- Pkg->Counter[0].PC02;				\
})

#define Delta_PC03(Pkg)							\
({									\
	Pkg->Delta.PC03 = Pkg->Counter[1].PC03				\
			- Pkg->Counter[0].PC03;				\
})

#define Delta_PC06(Pkg)							\
({									\
	Pkg->Delta.PC06 = Pkg->Counter[1].PC06				\
			- Pkg->Counter[0].PC06;				\
})

#define Delta_PC07(Pkg)							\
({									\
	Pkg->Delta.PC07 = Pkg->Counter[1].PC07				\
			- Pkg->Counter[0].PC07;				\
})

#define Delta_PC08(Pkg)							\
({									\
	Pkg->Delta.PC08 = Pkg->Counter[1].PC08				\
			- Pkg->Counter[0].PC08;				\
})

#define Delta_PC09(Pkg)							\
({									\
	Pkg->Delta.PC09 = Pkg->Counter[1].PC09				\
			- Pkg->Counter[0].PC09;				\
})

#define Delta_PC10(Pkg)							\
({									\
	Pkg->Delta.PC10 = Pkg->Counter[1].PC10				\
			- Pkg->Counter[0].PC10;				\
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

#define Save_SMI(Core)							\
({	/* Save the SMI interrupt counter. */				\
	Core->Counter[0].SMI = Core->Counter[1].SMI;			\
})

#define Save_PTSC(Pkg)							\
({									\
	Pkg->Counter[0].PTSC = Pkg->Counter[1].PTSC;			\
})

#define Save_PC02(Pkg)							\
({									\
	Pkg->Counter[0].PC02 = Pkg->Counter[1].PC02;			\
})

#define Save_PC03(Pkg)							\
({									\
	Pkg->Counter[0].PC03 = Pkg->Counter[1].PC03;			\
})

#define Save_PC06(Pkg)							\
({									\
	Pkg->Counter[0].PC06 = Pkg->Counter[1].PC06;			\
})

#define Save_PC07(Pkg)							\
({									\
	Pkg->Counter[0].PC07 = Pkg->Counter[1].PC07;			\
})

#define Save_PC08(Pkg)							\
({									\
	Pkg->Counter[0].PC08 = Pkg->Counter[1].PC08;			\
})

#define Save_PC09(Pkg)							\
({									\
	Pkg->Counter[0].PC09 = Pkg->Counter[1].PC09;			\
})

#define Save_PC10(Pkg)							\
({									\
	Pkg->Counter[0].PC10 = Pkg->Counter[1].PC10;			\
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

static enum hrtimer_restart Cycle_AuthenticAMD(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Genuine(Core, 1);

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

static enum hrtimer_restart Cycle_AMD_Family_0Fh(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (KPrivate->Join[cpu]->tsm.mustFwd == 1) {
		FIDVID_STATUS FidVidStatus = {.value = 0};

		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		// C-state like placeholder
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

	if (Proc->Features.AdvPower.DX.CPB == 1)	// Core Performance Boost.
	    smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_12h, 1);
	else
*/
	if (Proc->Features.Power.CX.EffFreq == 1) // MPERF & APERF ?
	    smp_call_function_single(cpu, InitTimer, Cycle_AuthenticAMD, 1);
	else
	    smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_0Fh, 1);
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

		SMT_Counters_Nehalem(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_Nehalem(Core, 1);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);
		}

		Core_Intel_Temp(Core);

		RDCOUNTER(Core->Counter[1].SMI, MSR_SMI_COUNT);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_C3(Core);

		Delta_C6(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Delta_SMI(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C3(Core);

		Save_C6(Core);

		Save_C1(Core);

		Save_SMI(Core);

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
	SMT_Counters_Nehalem(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_Nehalem(Core, 0);
	}

	RDCOUNTER(Core->Counter[0].SMI, MSR_SMI_COUNT);

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

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_SandyBridge(Core, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC(Proc);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);
		}

		Core_Intel_Temp(Core);

		RDCOUNTER(Core->Counter[1].SMI, MSR_SMI_COUNT);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_C3(Core);

		Delta_C6(Core);

		Delta_C7(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Delta_SMI(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C3(Core);

		Save_C6(Core);

		Save_C7(Core);

		Save_C1(Core);

		Save_SMI(Core);

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
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_SandyBridge(Core, 0);
	}

	RDCOUNTER(Core->Counter[0].SMI, MSR_SMI_COUNT);

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
		if ((Proc->SysGate = kmalloc(pageSize, GFP_KERNEL)) != NULL) {
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


#ifdef CONFIG_HOTPLUG_CPU
static int CoreFreqK_hotplug_cpu_online(unsigned int cpu)
{
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
		if (Arch[Proc->ArchID].Timer != NULL) {
			Arch[Proc->ArchID].Timer(cpu);
		}
		if ((KPrivate->Join[cpu]->tsm.started == 0)
		 && (Arch[Proc->ArchID].Start != NULL)) {
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, 0);
		}
		Proc->CPU.OnLine++;
		KPublic->Core[cpu]->OffLine.OS = 0;

		return(0);
	} else
		return(-EINVAL);
}

static int CoreFreqK_hotplug_cpu_offline(unsigned int cpu)
{
	if ((cpu >= 0) && (cpu < Proc->CPU.Count)) {
		if ((KPrivate->Join[cpu]->tsm.created == 1)
		 && (KPrivate->Join[cpu]->tsm.started == 1)
		 && (Arch[Proc->ArchID].Stop != NULL)) {
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, 1);
		}
		Proc->CPU.OnLine--;
		KPublic->Core[cpu]->OffLine.OS = 1;

		return(0);
	} else
		return(-EINVAL);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
static int CoreFreqK_hotplug(	struct notifier_block *nfb,
				unsigned long action,
				void *hcpu)
{
	unsigned int cpu = (unsigned long) hcpu, rc = 0;

	switch (action) {
	case CPU_ONLINE:
	case CPU_DOWN_FAILED:
//-	case CPU_ONLINE_FROZEN:
			rc = CoreFreqK_hotplug_cpu_online(cpu);
		break;
	case CPU_DOWN_PREPARE:
//-	case CPU_DOWN_PREPARE_FROZEN:
			rc = CoreFreqK_hotplug_cpu_offline(cpu);
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
#endif
#endif

static int CoreFreqK_NMI_handler(unsigned int type, struct pt_regs *pRegs)
{
	unsigned int cpu = smp_processor_id();

	switch (type) {
	case NMI_LOCAL:
		KPublic->Core[cpu]->Counter[1].NMI.LOCAL++;
		break;
	case NMI_UNKNOWN:
		KPublic->Core[cpu]->Counter[1].NMI.UNKNOWN++;
		break;
	case NMI_SERR:
		KPublic->Core[cpu]->Counter[1].NMI.PCISERR++;
		break;
	case NMI_IO_CHECK:
		KPublic->Core[cpu]->Counter[1].NMI.IOCHECK++;
		break;
	}
	return(NMI_DONE);
}

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

			    Proc->Registration.Experimental = Experimental;

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
				int allocPerCPU = 1;
				// Allocation per CPU
				for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
					void *kcache = NULL;
					kcache=kmem_cache_alloc(KPublic->Cache,
								GFP_KERNEL);
					if (kcache != NULL) {
						memset(kcache, 0, publicSize);
						KPublic->Core[cpu] = kcache;
					} else {
						allocPerCPU = 0;
						break;
					}
					kcache=kmem_cache_alloc(KPrivate->Cache,
								GFP_KERNEL);
					if (kcache != NULL) {
						memset(kcache, 0, privateSize);
						KPrivate->Join[cpu] = kcache;
					} else {
						allocPerCPU = 0;
						break;
					}
				}
				if (allocPerCPU) {
				  for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
					BITCLR( BUS_LOCK,
						KPublic->Core[cpu]->Sync.V, 63);

					KPublic->Core[cpu]->Bind = cpu;

					Define_CPUID(KPublic->Core[cpu],
							CpuIDforVendor);
				  }

				  if (!strncmp(Arch[0].Architecture,
						VENDOR_INTEL, 12)) {
					Arch[0].Query = Query_GenuineIntel;
					Arch[0].Start = Start_GenuineIntel;
					Arch[0].Stop  = Stop_GenuineIntel;
					Arch[0].Timer = InitTimer_GenuineIntel;
					Arch[0].Clock = Clock_GenuineIntel;
				  } else if (!strncmp(Arch[0].Architecture,
						VENDOR_AMD, 12) ) {
					Arch[0].Query = Query_AuthenticAMD;
					Arch[0].Start = Start_AuthenticAMD;
					Arch[0].Stop  = Stop_AuthenticAMD;
					Arch[0].Timer = InitTimer_AuthenticAMD;
					Arch[0].Clock = Clock_AuthenticAMD;
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

				  if (Proc->Registration.Experimental) {
				   Proc->Registration.pci =
				     pci_register_driver(&CoreFreqK_pci_driver);
				  }

		#ifdef CONFIG_HOTPLUG_CPU
			#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
			// Always returns zero (kernel/notifier.c)
			  Proc->Registration.hotplug =
			    register_hotcpu_notifier(&CoreFreqK_notifier_block);
			#else	// Continue with or without cpu hot-plugging.
			  Proc->Registration.hotplug =
				cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
						"corefreqk/cpu:online",
						CoreFreqK_hotplug_cpu_online,
						CoreFreqK_hotplug_cpu_offline);
			#endif
		#endif
				  Proc->Registration.nmi =
					register_nmi_handler(NMI_LOCAL,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk")
				|	register_nmi_handler(NMI_UNKNOWN,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk")
				|	register_nmi_handler(NMI_SERR,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk")
				|	register_nmi_handler(NMI_IO_CHECK,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk");
				} else {
				    if (KPublic->Cache != NULL) {
					for(cpu = 0;cpu < Proc->CPU.Count;cpu++)
					{
					  if (KPublic->Core[cpu] != NULL)
					    kmem_cache_free(KPublic->Cache,
							    KPublic->Core[cpu]);
					}
					kmem_cache_destroy(KPublic->Cache);
				    }
				    if (KPrivate->Cache != NULL) {
					for(cpu = 0;cpu < Proc->CPU.Count;cpu++)
					{
					  if (KPrivate->Join[cpu] != NULL)
					    kmem_cache_free(KPrivate->Cache,
							  KPrivate->Join[cpu]);
					}
					kmem_cache_destroy(KPrivate->Cache);
				    }
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
			if (KPublic != NULL)
				kfree(KPublic);
			if (KPrivate != NULL)
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

	if (!Proc->Registration.nmi) {
		unregister_nmi_handler(NMI_LOCAL,    "corefreqk");
		unregister_nmi_handler(NMI_UNKNOWN,  "corefreqk");
		unregister_nmi_handler(NMI_SERR,     "corefreqk");
		unregister_nmi_handler(NMI_IO_CHECK, "corefreqk");
	}
#ifdef CONFIG_HOTPLUG_CPU
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
		unregister_hotcpu_notifier(&CoreFreqK_notifier_block);
	#else
		if (!(Proc->Registration.hotplug < 0))
			cpuhp_remove_state_nocalls(Proc->Registration.hotplug);
	#endif
#endif

	if (Proc->Registration.Experimental) {
		if (!Proc->Registration.pci)
			pci_unregister_driver(&CoreFreqK_pci_driver);
	}
	Controller_Stop();
	Controller_Exit();

	for (cpu = 0;(KPublic->Cache != NULL) && (cpu < Proc->CPU.Count); cpu++)
	{
		if (KPublic->Core[cpu] != NULL)
			kmem_cache_free(KPublic->Cache, KPublic->Core[cpu]);
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
