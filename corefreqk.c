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
#include <linux/cpufreq.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif
#include <asm/msr.h>
#include <asm/nmi.h>

#include "bitasm.h"
#include "amdmsr.h"
#include "intelmsr.h"
#include "coretypes.h"
#include "corefreq-api.h"
#include "corefreqk.h"

MODULE_AUTHOR ("CYRIL INGENIERIE <labs[at]cyring[dot]fr>");
MODULE_DESCRIPTION ("CoreFreq Processor Driver");
MODULE_SUPPORTED_DEVICE ("Intel Core Core2 Atom Xeon i3 i5 i7, AMD Family 0Fh");
MODULE_LICENSE ("GPL");
MODULE_VERSION (COREFREQ_VERSION);

typedef struct {
	FEATURES	Features;
	unsigned int	SMT_Count;
	signed int	rc;
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
module_param(SleepInterval, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SleepInterval, "Timer interval (ms)");

static unsigned int TickInterval = 0;
module_param(TickInterval, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TickInterval, "System requested interval (ms)");

static signed int Experimental = 0;
module_param(Experimental, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Experimental, "Enable features under development");

static unsigned short NMI_Disable = 1;
module_param(NMI_Disable, ushort, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(NMI_Disable, "Disable the NMI handler");

static signed short PkgCStateLimit = -1;
module_param(PkgCStateLimit, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PkgCStateLimit, "Package C-State Limit");

static signed short IOMWAIT_Enable = -1;
module_param(IOMWAIT_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(IOMWAIT_Enable, "I/O MWAIT Redirection Enable");

static signed short CStateIORedir = -1;
module_param(CStateIORedir, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(CStateIORedir, "Power Mgmt IO Redirection C-State");

static signed short SpeedStep_Enable = -1;
module_param(SpeedStep_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SpeedStep_Enable, "Enable SpeedStep");

static signed short C1E_Enable = -1;
module_param(C1E_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1E_Enable, "Enable SpeedStep C1E");

static signed short TurboBoost_Enable = -1;
module_param(TurboBoost_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TurboBoost_Enable, "Enable Turbo Boost");

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

static signed short ODCM_Enable = -1;
module_param(ODCM_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ODCM_Enable, "Enable On-Demand Clock Modulation");

static signed short ODCM_DutyCycle = -1;
module_param(ODCM_DutyCycle, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ODCM_DutyCycle, "ODCM DutyCycle [0-7] | [0-14]");

static signed short PowerMGMT_Unlock = -1;
module_param(PowerMGMT_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerMGMT_Unlock, "Unlock Power Management");

static signed short PowerPolicy = -1;
module_param(PowerPolicy, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerPolicy, "Power Policy Preference [0-15]");

static signed int PState_FID = -1;
module_param(PState_FID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PState_FID, "P-State Frequency Id");

static signed int PState_VID = -1;
module_param(PState_VID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PState_VID, "P-State Voltage Id");

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
		for (jx = 0; jx < 4; jx++, px++) {
			idString[px     ] = Brand.AX.Chr[jx];
			idString[px +  4] = Brand.BX.Chr[jx];
			idString[px +  8] = Brand.CX.Chr[jx];
			idString[px + 12] = Brand.DX.Chr[jx];
		}
		px += 12;
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
		for (jx = 0; jx < 4; jx++, px++) {
			idString[px     ] = Brand.AX.Chr[jx];
			idString[px +  4] = Brand.BX.Chr[jx];
			idString[px +  8] = Brand.CX.Chr[jx];
			idString[px + 12] = Brand.DX.Chr[jx];
		}
		px += 12;
	}
	for (ix = jx = 0; jx < 48; jx++)
		if (!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++] = idString[jx];
}

// Retreive the Processor(BSP) features through calls to the CPUID instruction.
static void Query_Features(void *pArg)
{
	ARG *Arg = (ARG *) pArg;

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
		: "=r" (Arg->Features.Info.LargestStdFunc),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	Arg->Features.Info.Vendor.ID[ 0] = ebx;
	Arg->Features.Info.Vendor.ID[ 1] = (ebx >> 8);
	Arg->Features.Info.Vendor.ID[ 2] = (ebx >> 16);
	Arg->Features.Info.Vendor.ID[ 3] = (ebx >> 24);
	Arg->Features.Info.Vendor.ID[ 4] = edx;
	Arg->Features.Info.Vendor.ID[ 5] = (edx >> 8);
	Arg->Features.Info.Vendor.ID[ 6] = (edx >> 16);
	Arg->Features.Info.Vendor.ID[ 7] = (edx >> 24);
	Arg->Features.Info.Vendor.ID[ 8] = ecx;
	Arg->Features.Info.Vendor.ID[ 9] = (ecx >> 8);
	Arg->Features.Info.Vendor.ID[10] = (ecx >> 16);
	Arg->Features.Info.Vendor.ID[11] = (ecx >> 24);
	Arg->Features.Info.Vendor.ID[12] = '\0';

	if (!strncmp(Arg->Features.Info.Vendor.ID, VENDOR_INTEL, 12))
		Arg->Features.Info.Vendor.CRC = CRC_INTEL;
	else if (!strncmp(Arg->Features.Info.Vendor.ID, VENDOR_AMD, 12))
		Arg->Features.Info.Vendor.CRC = CRC_AMD;
	else {
		Arg->rc = -ENXIO;
		return;
	}

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
		: "=r" (Arg->Features.Std.EAX),
		  "=r" (Arg->Features.Std.EBX),
		  "=r" (Arg->Features.Std.ECX),
		  "=r" (Arg->Features.Std.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	if (Arg->Features.Info.LargestStdFunc >= 0x5) {
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
			: "=r" (Arg->Features.MWait.EAX),
			  "=r" (Arg->Features.MWait.EBX),
			  "=r" (Arg->Features.MWait.ECX),
			  "=r" (Arg->Features.MWait.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	if (Arg->Features.Info.LargestStdFunc >= 0x6) {
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
			: "=r" (Arg->Features.Power.EAX),
			  "=r" (Arg->Features.Power.EBX),
			  "=r" (Arg->Features.Power.ECX),
			  "=r" (Arg->Features.Power.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	if (Arg->Features.Info.LargestStdFunc >= 0x7) {
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
			: "=r" (Arg->Features.ExtFeature.EAX),
			  "=r" (Arg->Features.ExtFeature.EBX),
			  "=r" (Arg->Features.ExtFeature.ECX),
			  "=r" (Arg->Features.ExtFeature.EDX)
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
		: "=r" (Arg->Features.Info.LargestExtFunc),
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
		  "=r" (Arg->Features.ExtInfo.ECX),
		  "=r" (Arg->Features.ExtInfo.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	if (Arg->Features.Info.LargestExtFunc >= 0x80000007) {
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
			: "=r" (Arg->Features.AdvPower.EAX),
			  "=r" (Arg->Features.AdvPower.EBX),
			  "=r" (Arg->Features.AdvPower.ECX),
			  "=r" (Arg->Features.AdvPower.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}

	// Reset the performance features bits (present is zero)
	Arg->Features.PerfMon.EBX.CoreCycles    = 1;
	Arg->Features.PerfMon.EBX.InstrRetired  = 1;
	Arg->Features.PerfMon.EBX.RefCycles     = 1;
	Arg->Features.PerfMon.EBX.LLC_Ref       = 1;
	Arg->Features.PerfMon.EBX.LLC_Misses    = 1;
	Arg->Features.PerfMon.EBX.BranchRetired = 1;
	Arg->Features.PerfMon.EBX.BranchMispred = 1;

	// Per Vendor features
	if (Arg->Features.Info.Vendor.CRC == CRC_INTEL) {
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
		Arg->SMT_Count = (eax >> 26) & 0x3f;
		Arg->SMT_Count++;

	    if (Arg->Features.Info.LargestStdFunc >= 0xa) {
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
			: "=r" (Arg->Features.PerfMon.EAX),
			  "=r" (Arg->Features.PerfMon.EBX),
			  "=r" (Arg->Features.PerfMon.ECX),
			  "=r" (Arg->Features.PerfMon.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	    }
	    Arg->Features.FactoryFreq = Intel_Brand(Arg->Features.Info.Brand);

	} else if (Arg->Features.Info.Vendor.CRC == CRC_AMD) {

		if (Arg->Features.Std.EDX.HTT)
			Arg->SMT_Count = Arg->Features.Std.EBX.MaxThread;
		else {
			if (Arg->Features.Info.LargestExtFunc >= 0x80000008) {
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
				Arg->SMT_Count = (ecx & 0xf) + 1;
			}
		}
		AMD_Brand(Arg->Features.Info.Brand);
	}
}


typedef struct {			// V[0] stores previous TSC
	unsigned long long V[2];	// V[1] stores current TSC
} TSC_STRUCT;

#define OCCURRENCES 4
// OCCURRENCES x 2 (TSC values) needs a 64-byte cache line size.
#define STRUCT_SIZE (OCCURRENCES * sizeof(TSC_STRUCT))

static void Compute_Clock(void *arg)
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
		if (	(Proc->Features.AdvPower.EDX.Inv_TSC == 1)
			|| (Proc->Features.ExtInfo.EDX.RDTSCP == 1))
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
{	// For AMD Families 0Fh, 10h up to 16h
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
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

CLOCK Clock_AMD_Family_17h(unsigned int ratio)
{	// AMD PPR Family 17h § 1.4/ Table 11: REFCLK = 100 MHz
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
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
	if (Proc->Features.Info.Vendor.CRC == CRC_INTEL) {
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
	else if (Proc->Features.Info.Vendor.CRC == CRC_AMD) {
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
static void Map_Topology(void *arg)
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
			  "=r" (features.Std.EBX),
			  "=r" (ecx),
			  "=r" (edx)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);

		Core->T.CoreID = Core->T.ApicID = features.Std.EBX.Apic_ID;

		Cache_Topology(Core);
	}
}

static void Map_Extended_Topology(void *arg)
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
		unsigned long long OS_State=!cpu_online(cpu)||!cpu_active(cpu);

		KPublic->Core[cpu]->T.Base.value = 0;
		KPublic->Core[cpu]->T.ApicID     = -1;
		KPublic->Core[cpu]->T.CoreID     = -1;
		KPublic->Core[cpu]->T.ThreadID   = -1;

		// CPU state based on the OS
		if (!OS_State) {
			BITCLR(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);
			if (!Core_Topology(cpu)) {
			    // CPU state based on the hardware
			    if (KPublic->Core[cpu]->T.ApicID >= 0) {
				BITCLR(LOCKLESS,KPublic->Core[cpu]->OffLine,HW);
				CountEnabledCPU++;
			    }
			    else
				BITSET(LOCKLESS,KPublic->Core[cpu]->OffLine,HW);
			}
		} else
			BITSET(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);
	}
	return(CountEnabledCPU);
}

void HyperThreading_Technology(void)
{
	unsigned int CountEnabledCPU = Proc_Topology();

	if (Proc->Features.Std.EDX.HTT)
		Proc->CPU.OnLine = CountEnabledCPU;
	else
		Proc->CPU.OnLine = Proc->CPU.Count;
}

int Intel_MaxBusRatio(PLATFORM_ID *PfID)
{
	struct SIGNATURE whiteList[] = {
		_Core_Conroe,		/* 06_0F */
		_Core_Yorkfield,	/* 06_17 */
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft,		/* 06_27 */
		_Atom_Clovertrail,	/* 06_35 */
		_Atom_Saltwell,		/* 06_36 */
		_Silvermont_637,	/* 06_37 */
	};
	int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
	for (id = 0; id < ids; id++) {
		if((whiteList[id].ExtFamily == Proc->Features.Std.EAX.ExtFamily)
		 && (whiteList[id].Family == Proc->Features.Std.EAX.Family)
		 && (whiteList[id].ExtModel == Proc->Features.Std.EAX.ExtModel)
		 && (whiteList[id].Model == Proc->Features.Std.EAX.Model)) {

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
	unsigned int ratio0 = 10, ratio1 = 10, ratio2 = 10; // Arbitrary

	RDMSR(PfInfo, MSR_PLATFORM_INFO);
	if (PfInfo.value != 0) {
		ratio0 = KMIN(PfInfo.MinimumRatio, PfInfo.MaxNonTurboRatio);
		ratio1 = KMAX(PfInfo.MinimumRatio, PfInfo.MaxNonTurboRatio);
		ratio2 = ratio1;
	}

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	if (PerfStatus.value != 0) {				// §18.18.3.4
		if (PerfStatus.CORE.XE_Enable) {
			ratio2 = PerfStatus.CORE.MaxBusRatio;
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
	Proc->Boost[BOOST(MIN)] = ratio0;
	Proc->Boost[BOOST(MAX)] = KMIN(ratio1, ratio2);
	Proc->Boost[BOOST(1C)] = KMAX(ratio1, ratio2);

	Proc->Features.SpecTurboRatio = 1;
}

void Intel_Platform_Turbo(void)
{
	PLATFORM_INFO Platform = {.value = 0};
	RDMSR(Platform, MSR_PLATFORM_INFO);

	Proc->Features.Ratio_Unlock = Platform.Ratio_Limited;
	Proc->Features.TDP_Unlock = Platform.TDP_Limited;

	Proc->Boost[BOOST(MIN)] = Platform.MinimumRatio;
	Proc->Boost[BOOST(MAX)] = Platform.MaxNonTurboRatio;

	Proc->Features.SpecTurboRatio = 0;
}

void Intel_Turbo_Config8C(void)
{
	TURBO_RATIO_CONFIG0 TurboCfg0 = {.value = 0};
	RDMSR(TurboCfg0, MSR_TURBO_RATIO_LIMIT);

	Proc->Boost[BOOST(8C)] = TurboCfg0.MaxRatio_8C;
	Proc->Boost[BOOST(7C)] = TurboCfg0.MaxRatio_7C;
	Proc->Boost[BOOST(6C)] = TurboCfg0.MaxRatio_6C;
	Proc->Boost[BOOST(5C)] = TurboCfg0.MaxRatio_5C;
	Proc->Boost[BOOST(4C)] = TurboCfg0.MaxRatio_4C;
	Proc->Boost[BOOST(3C)] = TurboCfg0.MaxRatio_3C;
	Proc->Boost[BOOST(2C)] = TurboCfg0.MaxRatio_2C;
	Proc->Boost[BOOST(1C)] = TurboCfg0.MaxRatio_1C;

	Proc->Features.SpecTurboRatio += 8;
}

void Intel_Turbo_Config15C(void)
{
	TURBO_RATIO_CONFIG1 TurboCfg1 = {.value = 0};
	RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);

	Proc->Boost[BOOST(15C)] = TurboCfg1.IVB_EP.MaxRatio_15C;
	Proc->Boost[BOOST(14C)] = TurboCfg1.IVB_EP.MaxRatio_14C;
	Proc->Boost[BOOST(13C)] = TurboCfg1.IVB_EP.MaxRatio_13C;
	Proc->Boost[BOOST(12C)] = TurboCfg1.IVB_EP.MaxRatio_12C;
	Proc->Boost[BOOST(11C)] = TurboCfg1.IVB_EP.MaxRatio_11C;
	Proc->Boost[BOOST(10C)] = TurboCfg1.IVB_EP.MaxRatio_10C;
	Proc->Boost[BOOST(9C) ] = TurboCfg1.IVB_EP.MaxRatio_9C;

	Proc->Features.SpecTurboRatio += 7;
}

void Intel_Turbo_Config16C(void)
{
	TURBO_RATIO_CONFIG1 TurboCfg1 = {.value = 0};
	RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);

	Proc->Boost[BOOST(16C)] = TurboCfg1.HSW_EP.MaxRatio_16C;
	Proc->Boost[BOOST(15C)] = TurboCfg1.HSW_EP.MaxRatio_15C;
	Proc->Boost[BOOST(14C)] = TurboCfg1.HSW_EP.MaxRatio_14C;
	Proc->Boost[BOOST(13C)] = TurboCfg1.HSW_EP.MaxRatio_13C;
	Proc->Boost[BOOST(12C)] = TurboCfg1.HSW_EP.MaxRatio_12C;
	Proc->Boost[BOOST(11C)] = TurboCfg1.HSW_EP.MaxRatio_11C;
	Proc->Boost[BOOST(10C)] = TurboCfg1.HSW_EP.MaxRatio_10C;
	Proc->Boost[BOOST(9C) ] = TurboCfg1.HSW_EP.MaxRatio_9C;

	Proc->Features.SpecTurboRatio += 8;
}

void Intel_Turbo_Config18C(void)
{
	TURBO_RATIO_CONFIG2 TurboCfg2 = {.value = 0};
	RDMSR(TurboCfg2, MSR_TURBO_RATIO_LIMIT2);

	Proc->Boost[BOOST(18C)] = TurboCfg2.MaxRatio_18C;
	Proc->Boost[BOOST(17C)] = TurboCfg2.MaxRatio_17C;

	Proc->Features.SpecTurboRatio += 2;
}

void Nehalem_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C();
}

void IvyBridge_EP_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C();
	Intel_Turbo_Config15C();
}

void Haswell_EP_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C();
	Intel_Turbo_Config16C();
	Intel_Turbo_Config18C();
}

void Skylake_X_Platform_Info(void)
{
	TURBO_RATIO_CONFIG1 TurboCfg1 = {.value = 0};
	RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);

	Intel_Platform_Turbo();
	Intel_Turbo_Config8C();

	Proc->Boost[BOOST(16C)] = TurboCfg1.SKL_X.NUMCORE_7;
	Proc->Boost[BOOST(15C)] = TurboCfg1.SKL_X.NUMCORE_6;
	Proc->Boost[BOOST(14C)] = TurboCfg1.SKL_X.NUMCORE_5;
	Proc->Boost[BOOST(13C)] = TurboCfg1.SKL_X.NUMCORE_4;
	Proc->Boost[BOOST(12C)] = TurboCfg1.SKL_X.NUMCORE_3;
	Proc->Boost[BOOST(11C)] = TurboCfg1.SKL_X.NUMCORE_2;
	Proc->Boost[BOOST(10C)] = TurboCfg1.SKL_X.NUMCORE_1;
	Proc->Boost[BOOST(9C) ] = TurboCfg1.SKL_X.NUMCORE_0;

	Proc->Features.SpecTurboRatio += 8;
}


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

void Query_IVB_IMC(void __iomem *mchmap)
{	// Sources:	Desktop 3rd Generation Intel® Core™ Processor Family
	//		Intel® Xeon Processor E3-1200 Family
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;
/* ToDo
	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0x0);
*/
	Proc->Uncore.MC[0].IVB.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].IVB.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].IVB.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].IVB.MAD0.Dimm_B_Size != 0))  /*0 or 1*/
		+ ((Proc->Uncore.MC[0].IVB.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].IVB.MAD1.Dimm_B_Size != 0)); /*0 or 1*/

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].IVB.DBP.value =
					readl(mchmap + 0x4000 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].IVB.RAP.value =
					readl(mchmap + 0x4004 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].IVB.RFTP.value =
					readl(mchmap + 0x4298 + 0x400 * cha);
	}
	// DIMM A & DIMM B
	Proc->Uncore.MC[0].SlotCount = 2;
}

void Query_HSW_IMC(void __iomem *mchmap)
{	// Source: Desktop 4th Generation Intel® Core™ Processor Family
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;
/* ToDo
	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0x0);
*/
	Proc->Uncore.MC[0].IVB.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].IVB.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].IVB.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].IVB.MAD0.Dimm_B_Size != 0))
		+ ((Proc->Uncore.MC[0].IVB.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].IVB.MAD1.Dimm_B_Size != 0));

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].HSW.Timing.value =
					readl(mchmap + 0x4c04 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank.value =
					readl(mchmap + 0x4c14 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].HSW.Refresh.value =
					readl(mchmap + 0x4e98 + 0x400 * cha);
	}
/* ToDo */
	Proc->Uncore.MC[0].SlotCount = 1;
}

void Query_BDW_IMC(void __iomem *mchmap)
{	// Source: Mobile 5th Generation Intel® Core™ Processor Family
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;
/* ToDo
	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0x0);
*/
	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].IVB.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].IVB.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].IVB.MAD0.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].IVB.MAD0.Dimm_B_Size != 0))
		+ ((Proc->Uncore.MC[0].IVB.MAD1.Dimm_A_Size != 0)
		|| (Proc->Uncore.MC[0].IVB.MAD1.Dimm_B_Size != 0));

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].HSW.Timing.value =
					readl(mchmap + 0x4c04);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank_A.value =
					readl(mchmap + 0x4c08);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank_B.value =
					readl(mchmap + 0x4c0c);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank.value =
					readl(mchmap + 0x4c14);

		Proc->Uncore.MC[0].Channel[cha].HSW.Refresh.value =
					readl(mchmap + 0x4e98);
	}
/* ToDo */
	Proc->Uncore.MC[0].SlotCount = 1;
}

static PCI_CALLBACK P965(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x4000, Query_P965));
}

static PCI_CALLBACK G965(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x4000, Query_G965));
}

static PCI_CALLBACK P35(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x4000, Query_P35));
}

static PCI_CALLBACK Bloomfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_Bloomfield_IMC(dev, mc);

	return((PCI_CALLBACK) rc);
}

static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_Lynnfield_IMC(dev, mc);

	return((PCI_CALLBACK) rc);
}

static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x50, &Proc->Uncore.Bus.DimmClock.value);

	return(0);
}

static PCI_CALLBACK X58_QPI(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xd0, &Proc->Uncore.Bus.QuickPath.value);

	return(0);
}

static PCI_CALLBACK IVB_IMC(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x8000, Query_IVB_IMC));
}

static PCI_CALLBACK HSW_IMC(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x8000, Query_HSW_IMC));
}

static PCI_CALLBACK BDW_IMC(struct pci_dev *dev)
{
	return(Router(dev, 0x48, 0x8000, Query_BDW_IMC));
}

static PCI_CALLBACK AMD_0F_MCH(struct pci_dev *dev)
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

static PCI_CALLBACK AMD_0F_HTT(struct pci_dev *dev)
{
	unsigned int link;

	pci_read_config_dword(dev, 0x64, &Proc->Uncore.Bus.UnitID.value);

	for (link = 0; link < 3; link++) {
		pci_read_config_dword(dev, 0x88 + 0x20 * link,
				&Proc->Uncore.Bus.LDTi_Freq[link].value);
	};

	return(0);
}

static int CoreFreqK_ProbePCI(void)
{
	struct pci_device_id *id = Arch[Proc->ArchID].PCI_ids;
	struct pci_dev *dev = NULL;
	int rc = -ENODEV;

	while (id->vendor || id->subvendor || id->class_mask) {
		dev = pci_get_device(id->vendor, id->device, NULL);
		if (dev != NULL) {
		    if (!pci_enable_device(dev)) {
			PCI_CALLBACK Callback = (PCI_CALLBACK) id->driver_data;
				rc =(int) Callback(dev);
			pci_disable_device(dev);
		    }
		    pci_dev_put(dev);
		}
		id++;
	}
	return(rc);
}

void Query_GenuineIntel(void)
{
	Intel_Platform_Info();
	HyperThreading_Technology();
}

void Query_Core2(void)
{
	Intel_Platform_Info();
	HyperThreading_Technology();
}

void Query_Nehalem(void)
{
	Nehalem_Platform_Info();
	HyperThreading_Technology();
}

void Query_IvyBridge_EP(void)
{
	IvyBridge_EP_Platform_Info();
	HyperThreading_Technology();
}

void Query_Haswell_EP(void)
{
	Haswell_EP_Platform_Info();
	HyperThreading_Technology();
}

void Query_Skylake_X(void)
{
	Skylake_X_Platform_Info();
	HyperThreading_Technology();
}

void Query_AuthenticAMD(void)
{	// Fallback algorithm for unspecified AMD architectures.
	Proc->Boost[BOOST(MIN)] = 8;

	if (Proc->Features.AdvPower.EDX.HwPstate == 1) {
		COFVID CofVid = {.value = 0};

		switch(Arch[Proc->ArchID].Signature.ExtFamily) {
		case 0x1:
		case 0x6:
		case 0x7:
			RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
			Proc->Boost[BOOST(MAX)] = CofVid.Arch_COF.MaxCpuCof;
			break;
		case 0x2:
			RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
			Proc->Boost[BOOST(MAX)]=CofVid.Arch_Pll.MainPllOpFidMax;
			if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
				Proc->Boost[BOOST(MAX)] += 0x8;
			break;
		case 0x3:
		case 0x5:
			RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
			Proc->Boost[BOOST(MAX)]=CofVid.Arch_Pll.MainPllOpFidMax;
			if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
				Proc->Boost[BOOST(MAX)] += 0x10;
			break;
		}
	} else { // No Solution !
		Proc->Boost[BOOST(MAX)] = Proc->Boost[BOOST(MIN)];
	}
	Proc->Boost[BOOST(1C)] = Proc->Boost[BOOST(MAX)];

	Proc->Features.SpecTurboRatio = 0;

	HyperThreading_Technology();
}

void Query_AMD_Family_0Fh(void)
{   // BKDG for AMD NPT Family 0Fh: §13.8
    if (Proc->Features.AdvPower.EDX.FID == 1) {
	// Processor supports FID changes.
	FIDVID_STATUS FidVidStatus = {.value = 0};

	RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

	Proc->Boost[BOOST(MIN)] = VCO[FidVidStatus.StartFID].MCF;
	Proc->Boost[BOOST(MAX)] = 8 + FidVidStatus.MaxFID;

	if (FidVidStatus.StartFID < 0b1000) {
	    unsigned int t;
	    for (t = 0; t < 5; t++) {
		Proc->Boost[BOOST(SIZE)-5+t]=VCO[FidVidStatus.StartFID].PCF[t];
	    }

	    Proc->Features.SpecTurboRatio = 5;
	} else {
		Proc->Boost[BOOST(1C)] = 8 + FidVidStatus.MaxFID;

		Proc->Features.SpecTurboRatio = 1;
	}
    } else {
	HWCR HwCfgRegister = {.value = 0};

	RDMSR(HwCfgRegister, MSR_K7_HWCR);

	Proc->Boost[BOOST(MIN)] = 8 + HwCfgRegister.Family_0Fh.StartFID;
	Proc->Boost[BOOST(MAX)] = Proc->Boost[BOOST(MIN)];
	Proc->Boost[BOOST(1C) ] = Proc->Boost[BOOST(MIN)];

	Proc->Features.SpecTurboRatio = 1;
    }

	HyperThreading_Technology();
}

void Query_AMD_Family_10h(void)
{
	unsigned int pstate, sort[5] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 4; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_10h.CpuFid + 0x10)
					  / (1 << PstateDef.Family_10h.CpuDid);
	}
	Proc->Features.SpecTurboRatio = 3;

	HyperThreading_Technology();
}

void Query_AMD_Family_11h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_10h.CpuFid + 0x8)
					  / (1 << PstateDef.Family_10h.CpuDid);
	}
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_12h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_12h.CpuFid + 0x10)
					  /  PstateDef.Family_12h.CpuDid;
	}
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_14h(void)
{
	COFVID CofVid = {.value = 0};
	unsigned int MaxFreq = 100, ClockDiv;
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};

	RDMSR(CofVid, MSR_AMD_COFVID_STATUS);

	if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
		MaxFreq *= (CofVid.Arch_Pll.MainPllOpFidMax + 0x10);

	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		ClockDiv = (PstateDef.Family_14h.CpuDidMSD + 1) * 4;
		ClockDiv += PstateDef.Family_14h.CpuDidLSD;

		Proc->Boost[sort[pstate]] = (MaxFreq * 4) / ClockDiv;
	}	// @ MainPllOpFidMax MHz
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_15h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_15h.CpuFid + 0x10)
					  / (1 << PstateDef.Family_15h.CpuDid);
	}
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_17h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = PstateDef.Family_17h.CpuFid * 25;
	}
	Proc->Features.SpecTurboRatio = 6;

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
		  "=r" (Core->Query.ExtFunc.EBX),
		  "=r" (Core->Query.ExtFunc.ECX),
		  "=r" (Core->Query.ExtFunc.EDX)
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

void SpeedStep_Technology(CORE *Core, unsigned int cpu)
{
	if (Core->T.Base.BSP) {
		if (Proc->Features.Std.ECX.EIST == 1) {
			MISC_PROC_FEATURES MiscFeatures = {.value = 0};
			RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

			switch (SpeedStep_Enable) {
			    case COREFREQ_TOGGLE_OFF:
			    case COREFREQ_TOGGLE_ON:
				MiscFeatures.EIST = SpeedStep_Enable;
				WRMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
				RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
			    break;
			}
			Core->Query.EIST = MiscFeatures.EIST;
		} else {
			Core->Query.EIST = 0;
		}
		BITSET(LOCKLESS, Proc->SpeedStep_Mask, cpu);	// Per Package
	} else
		BITCLR(LOCKLESS, Proc->SpeedStep_Mask, cpu);
}

void TurboBoost_Technology(CORE *Core, unsigned int cpu)
{
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	if (MiscFeatures.Turbo_IDA == 0) {
		PERF_CONTROL PerfControl = {.value = 0};
		RDMSR(PerfControl, MSR_IA32_PERF_CTL);

		switch (TurboBoost_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				PerfControl.Turbo_IDA = !TurboBoost_Enable;
				WRMSR(PerfControl, MSR_IA32_PERF_CTL);
				RDMSR(PerfControl, MSR_IA32_PERF_CTL);
			break;
		}
		Core->Query.Turbo = !PerfControl.Turbo_IDA;
	} else {
		Core->Query.Turbo = 0;
	}
	BITSET(LOCKLESS, Proc->TurboBoost_Mask, cpu);		// Per Thread
}

void DynamicAcceleration(CORE *Core, unsigned int cpu)
{
	if (Proc->Features.Power.EAX.TurboIDA) {
		TurboBoost_Technology(Core, cpu);
	} else {
		Core->Query.Turbo = 0;

		BITSET(LOCKLESS, Proc->TurboBoost_Mask, cpu);	// Unique
	}
}

void Query_Intel_C1E(CORE *Core, unsigned int cpu)
{
	if (Core->T.Base.BSP) {
		POWER_CONTROL PowerCtrl = {.value = 0};
		RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);

		switch (C1E_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				PowerCtrl.C1E = C1E_Enable;
				WRMSR(PowerCtrl, MSR_IA32_POWER_CTL);
				RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);
			break;
		}
		Core->Query.C1E = PowerCtrl.C1E;

		BITSET(LOCKLESS, Proc->C1E_Mask, cpu);		// Per Package
	} else {
		Core->Query.C1E = 0;

		BITCLR(LOCKLESS, Proc->C1E_Mask, cpu);
	}
}

void Query_AMD_Family_0Fh_C1E(CORE *Core, unsigned int cpu)
{
	INT_PENDING_MSG IntPendingMsg = {.value = 0};

	RDMSR(IntPendingMsg, MSR_K8_INT_PENDING_MSG);

	Core->Query.C1E = IntPendingMsg.C1eOnCmpHalt
			& !IntPendingMsg.SmiOnCmpHalt;

	BITSET(LOCKLESS, Proc->C1E_Mask, cpu);			// Per Core
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

void PowerThermal(CORE *Core, unsigned int cpu)
{
  CLOCK_MODULATION ClockModulation = {.value = 0};

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
	: "=r" (Power.EAX),
	  "=r" (Power.EBX),
	  "=r" (Power.ECX),
	  "=r" (Power.EDX)
	:
	: "%rax", "%rbx", "%rcx", "%rdx"
    );

    if (Power.ECX.SETBH == 1) {
	struct SIGNATURE blackList[] = {
		_Silvermont_637,	/* 06_37 */
		_Atom_Airmont,		/* 06_4C */
		_Atom_Avoton,		/* 06_4D */
	};
	int id, ids = sizeof(blackList) / sizeof(blackList[0]);
	for (id = 0; id < ids; id++) {
		if((blackList[id].ExtFamily == Proc->Features.Std.EAX.ExtFamily)
		 && (blackList[id].Family == Proc->Features.Std.EAX.Family)
		 && (blackList[id].ExtModel == Proc->Features.Std.EAX.ExtModel)
		 && (blackList[id].Model == Proc->Features.Std.EAX.Model)) {
			break;
		}
	}
	if (id == ids) {
	  RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	  RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);

	  if (Experimental == 1) {
	    switch (PowerMGMT_Unlock) {
	    case COREFREQ_TOGGLE_OFF:
	    case COREFREQ_TOGGLE_ON:
	     Core->PowerThermal.PwrManagement.Perf_BIAS_Enable=PowerMGMT_Unlock;
	     WRMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
	     RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
	     break;
	   }
	  }

	  if (Core->PowerThermal.PwrManagement.Perf_BIAS_Enable
	  && (PowerPolicy >= 0) && (PowerPolicy <= 15))
	  {
	    Core->PowerThermal.PerfEnergyBias.PowerPolicy = PowerPolicy;
	    WRMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	    RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	  }
	}
    }

    if (Proc->Features.Std.EDX.ACPI == 1) {
	int ToggleFeature = 0;

	RDMSR(ClockModulation, MSR_IA32_THERM_CONTROL);
	ClockModulation.ECMD = Power.EAX.ECMD;

	switch (ODCM_Enable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		ClockModulation.ODCM_Enable = ODCM_Enable;
		ToggleFeature = 1;
		break;
	}
	if ((ODCM_DutyCycle >= 0)
	 && (ODCM_DutyCycle <= (7 << ClockModulation.ECMD)))
	{
	    ClockModulation.DutyCycle = ODCM_DutyCycle << !ClockModulation.ECMD;
	    ToggleFeature = 1;
	}
	if (ToggleFeature == 1) {
	    WRMSR(ClockModulation, MSR_IA32_THERM_CONTROL);
	    RDMSR(ClockModulation, MSR_IA32_THERM_CONTROL);
	}
	Core->PowerThermal.ClockModulation = ClockModulation;
    }
  }
  BITSET(LOCKLESS, Proc->ODCM_Mask, cpu);
  BITSET(LOCKLESS, Proc->PowerMgmt_Mask, cpu);
}

void CStatesConfiguration(int encoding, CORE *Core, unsigned int cpu)
{
	CSTATE_CONFIG CStateConfig = {.value = 0};
	CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
	int ToggleFeature = 0;

	RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

	switch (C3A_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			CStateConfig.C3autoDemotion = C3A_Enable;
			ToggleFeature = 1;
		break;
	}
	switch (C1A_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			CStateConfig.C1autoDemotion = C1A_Enable;
			ToggleFeature = 1;
		break;
	}
	if (encoding == 0x062A) {
		switch (C3U_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				CStateConfig.C3undemotion = C3U_Enable;
				ToggleFeature = 1;
			break;
		}
		switch (C1U_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				CStateConfig.C1undemotion = C1U_Enable;
				ToggleFeature = 1;
			break;
		}
	}
	if (ToggleFeature == 1) {
		WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
	}
	if (CStateConfig.CFG_Lock == 0) {
		ToggleFeature = 0;
		if (encoding == 0x061A) { // NHM encoding compatibility
			switch (IOMWAIT_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				CStateConfig.IO_MWAIT_Redir = IOMWAIT_Enable;
				ToggleFeature = 1;
				break;
			}
			switch (PkgCStateLimit) {
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b100;
				ToggleFeature = 1;
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b011;
				ToggleFeature = 1;
				break;
			case 3: // Cannot be used to limit package C-State to C3
				CStateConfig.Pkg_CStateLimit = 0b010;
				ToggleFeature = 1;
				break;
			case 1:
				CStateConfig.Pkg_CStateLimit = 0b001;
				ToggleFeature = 1;
				break;
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b000;
				ToggleFeature = 1;
				break;
			}
		} else if (encoding == 0x062A) { // SNB encoding compatibility
			switch (PkgCStateLimit) {
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b100;
				ToggleFeature = 1;
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b011;
				ToggleFeature = 1;
				break;
			case 3:
				CStateConfig.Pkg_CStateLimit = 0b010;
				ToggleFeature = 1;
				break;
			case 2:
				CStateConfig.Pkg_CStateLimit = 0b001;
				ToggleFeature = 1;
				break;
			case 1:
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b000;
				ToggleFeature = 1;
				break;
			}
		} else if (encoding == 0x0645) {//HSW_ULT encoding compatibility
			switch (PkgCStateLimit) {
			case 10:
				CStateConfig.Pkg_CStateLimit = 0b1000;
				ToggleFeature = 1;
				break;
			case 9:
				CStateConfig.Pkg_CStateLimit = 0b0111;
				ToggleFeature = 1;
				break;
			case 8:
				CStateConfig.Pkg_CStateLimit = 0b0110;
				ToggleFeature = 1;
				break;
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b0100;
				ToggleFeature = 1;
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b0011;
				ToggleFeature = 1;
				break;
			case 3:
				CStateConfig.Pkg_CStateLimit = 0b0010;
				ToggleFeature = 1;
				break;
			case 2:
				CStateConfig.Pkg_CStateLimit = 0b0001;
				ToggleFeature = 1;
				break;
			case 1:
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b0000;
				ToggleFeature = 1;
				break;
			}
		}
		if (ToggleFeature == 1) {
			WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
			RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		}
	}
	Core->Query.C3A = CStateConfig.C3autoDemotion;
	Core->Query.C1A = CStateConfig.C1autoDemotion;

	if (encoding == 0x062A) {
		Core->Query.C3U = CStateConfig.C3undemotion;
		Core->Query.C1U = CStateConfig.C1undemotion;
	}
	Core->Query.CfgLock = CStateConfig.CFG_Lock;
	Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;

	if (encoding == 0x061A) {
		switch (CStateConfig.Pkg_CStateLimit & 0x7) {
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
	} else if (encoding == 0x062A) {
		switch (CStateConfig.Pkg_CStateLimit & 0x7) {
		case 0b101:
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
			Core->Query.CStateLimit = 2;
			break;
		case 0b000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}
	} else if (encoding == 0x0645) {
		switch (CStateConfig.Pkg_CStateLimit) {
		case 0b1000:
			Core->Query.CStateLimit = 10;
			break;
		case 0b0111:
			Core->Query.CStateLimit = 9;
			break;
		case 0b0110:
			Core->Query.CStateLimit = 8;
			break;
		case 0b0101:
		case 0b0100:
			Core->Query.CStateLimit = 7;
			break;
		case 0b0011:
			Core->Query.CStateLimit = 6;
			break;
		case 0b0010:
			Core->Query.CStateLimit = 3;
			break;
		case 0b0001:
			Core->Query.CStateLimit = 2;
			break;
		case 0b0000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}
	}
	BITSET(LOCKLESS, Proc->C3A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3U_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1U_Mask, cpu);

	RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);

	if (CStateConfig.IO_MWAIT_Redir) {
		switch (CStateIORedir) {
		case 7:
			CState_IO_MWAIT.CStateRange = 0b010;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		case 6:
			CState_IO_MWAIT.CStateRange = 0b001;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		case 3:
			CState_IO_MWAIT.CStateRange = 0b000;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		}
		if (CStateIORedir != -1) {
			RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
		}
	}

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

void PerCore_AMD_Family_0Fh_PStates(CORE *Core)
{
    if (Experimental == 1) {
	FIDVID_STATUS FidVidStatus = {.value = 0};
	FIDVID_CONTROL FidVidControl = {.value = 0};
	int NewFID = -1, NewVID = -1, loop = 100;

	RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

	NewFID	= ((PState_FID >= FidVidStatus.StartFID)
		&& (PState_FID <= FidVidStatus.MaxFID)) ?
		PState_FID : FidVidStatus.CurrFID,

	NewVID	= ((PState_VID <= FidVidStatus.StartVID)
		&& (PState_VID >= FidVidStatus.MaxVID)) ?
		PState_VID : FidVidStatus.CurrVID;
	do {
		if (FidVidStatus.FidVidPending == 0) {
			RDMSR(FidVidControl, MSR_K7_FID_VID_CTL);

			FidVidControl.InitFidVid = 1;
			FidVidControl.StpGntTOCnt = 400;
			FidVidControl.NewFID = NewFID;
			FidVidControl.NewVID = NewVID;

			WRMSR(FidVidControl, MSR_K7_FID_VID_CTL);
			loop = 0;
		} else
			RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		if (loop == 0) {
			break;
		} else
			loop-- ;
	} while (FidVidStatus.FidVidPending == 1) ;
    }
}

void Microcode(CORE *Core)
{
	MICROCODE_ID Microcode = {.value = 0};
	RDMSR(Microcode, MSR_IA32_UCODE_REV);
	Core->Query.Microcode = Microcode.Signature;
}

void PerCore_Intel_Query(CORE *Core, unsigned int cpu)
{
	Microcode(Core);

	Dump_CPUID(Core);

	BITSET(LOCKLESS, Proc->SpeedStep_Mask, cpu);
	BITSET(LOCKLESS, Proc->TurboBoost_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1E_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3U_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1U_Mask, cpu);

	PowerThermal(Core, cpu);

	ThermalMonitor_Set(Core);
}

void PerCore_AuthenticAMD_Query(CORE *Core, unsigned int cpu)
{
	Dump_CPUID(Core);

	BITSET(LOCKLESS, Proc->ODCM_Mask, cpu);
	BITSET(LOCKLESS, Proc->PowerMgmt_Mask, cpu);
	BITSET(LOCKLESS, Proc->SpeedStep_Mask, cpu);
	BITSET(LOCKLESS, Proc->TurboBoost_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3U_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1U_Mask, cpu);
}

void PerCore_Core2_Query(CORE *Core, unsigned int cpu)
{
	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core, cpu);
	DynamicAcceleration(Core, cpu);				// Unique

	BITSET(LOCKLESS, Proc->C1E_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3U_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1U_Mask, cpu);

	PowerThermal(Core, cpu);				// Shared|Unique

	ThermalMonitor_Set(Core);
}

void PerCore_Nehalem_Query(CORE *Core, unsigned int cpu)
{
	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core, cpu);
	TurboBoost_Technology(Core, cpu);
	Query_Intel_C1E(Core, cpu);

	if (Core->T.ThreadID == 0) {				// Per Core
		CStatesConfiguration(0x061A, Core, cpu);
	}
	PowerThermal(Core, cpu);

	ThermalMonitor_Set(Core);
}

void PerCore_SandyBridge_Query(CORE *Core, unsigned int cpu)
{
	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core, cpu);
	TurboBoost_Technology(Core, cpu);
	Query_Intel_C1E(Core, cpu);

	if (Core->T.ThreadID == 0) {				// Per Core
		CStatesConfiguration(0x062A, Core, cpu);
	}
	PowerThermal(Core, cpu);

	ThermalMonitor_Set(Core);
}

void PerCore_Haswell_ULT_Query(CORE *Core, unsigned int cpu)
{
	Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core, cpu);
	TurboBoost_Technology(Core, cpu);
	Query_Intel_C1E(Core, cpu);

	if (Core->T.ThreadID == 0) {				// Per Core
		CStatesConfiguration(0x0645, Core, cpu);
	}
	PowerThermal(Core, cpu);

	ThermalMonitor_Set(Core);
}

void PerCore_AMD_Family_0Fh_Query(CORE *Core, unsigned int cpu)
{
	Dump_CPUID(Core);

	Query_AMD_Family_0Fh_C1E(Core, cpu);

	BITSET(LOCKLESS, Proc->ODCM_Mask, cpu);
	BITSET(LOCKLESS, Proc->PowerMgmt_Mask, cpu);
	BITSET(LOCKLESS, Proc->SpeedStep_Mask, cpu);
	BITSET(LOCKLESS, Proc->TurboBoost_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3U_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1U_Mask, cpu);

	PerCore_AMD_Family_0Fh_PStates(Core);
}

void PerCore_AMD_Family_10h_Query(CORE *Core, unsigned int cpu)
{
	Dump_CPUID(Core);

	Query_AMD_Family_0Fh_C1E(Core, cpu);

	BITSET(LOCKLESS, Proc->ODCM_Mask, cpu);
	BITSET(LOCKLESS, Proc->PowerMgmt_Mask, cpu);
	BITSET(LOCKLESS, Proc->SpeedStep_Mask, cpu);
	BITSET(LOCKLESS, Proc->TurboBoost_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1A_Mask, cpu);
	BITSET(LOCKLESS, Proc->C3U_Mask, cpu);
	BITSET(LOCKLESS, Proc->C1U_Mask, cpu);
}

void Sys_DumpTask(SYSGATE *SysGate)
{	// Source: /include/linux/sched.h
	struct task_struct *process, *thread;
	int cnt = 0;

	rcu_read_lock();
	for_each_process_thread(process, thread) {
		task_lock(thread);

		SysGate->taskList[cnt].runtime  = thread->se.sum_exec_runtime;
		SysGate->taskList[cnt].usertime = thread->utime;
		SysGate->taskList[cnt].systime  = thread->stime;
		SysGate->taskList[cnt].state    = thread->state;
		SysGate->taskList[cnt].wake_cpu = thread->wake_cpu;
		SysGate->taskList[cnt].pid      = thread->pid;
		SysGate->taskList[cnt].tgid     = thread->tgid;
		SysGate->taskList[cnt].ppid     = thread->parent->pid;
		memcpy(SysGate->taskList[cnt].comm, thread->comm,TASK_COMM_LEN);

		task_unlock(thread);
		cnt++;
	}
	rcu_read_unlock();
	SysGate->taskCount = cnt;
}

void Sys_MemInfo(SYSGATE *SysGate)
{	// Source: /include/uapi/linux/sysinfo.h
	struct sysinfo info;
	si_meminfo(&info);

	SysGate->memInfo.totalram  = info.totalram  << (PAGE_SHIFT - 10);
	SysGate->memInfo.sharedram = info.sharedram << (PAGE_SHIFT - 10);
	SysGate->memInfo.freeram   = info.freeram   << (PAGE_SHIFT - 10);
	SysGate->memInfo.bufferram = info.bufferram << (PAGE_SHIFT - 10);
	SysGate->memInfo.totalhigh = info.totalhigh << (PAGE_SHIFT - 10);
	SysGate->memInfo.freehigh  = info.freehigh  << (PAGE_SHIFT - 10);
}

#define Sys_Tick(Pkg)						\
({								\
	if (Pkg->SysGate != NULL) {				\
		Pkg->tickStep--;				\
		if (!Pkg->tickStep) {				\
			Pkg->tickStep = Pkg->tickReset;		\
								\
			Sys_DumpTask(Pkg->SysGate);		\
			Sys_MemInfo(Pkg->SysGate);		\
		}						\
	}							\
})

static void InitTimer(void *Cycle_Function)
{
	unsigned int cpu = smp_processor_id();

	if (BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 0) {
		hrtimer_init(	&KPrivate->Join[cpu]->Timer,
				CLOCK_MONOTONIC,
				HRTIMER_MODE_REL_PINNED);

		KPrivate->Join[cpu]->Timer.function = Cycle_Function;
		BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, CREATED);
	}
}

void Controller_Init(void)
{
	CLOCK clock = {.Q = 0, .R = 0, .Hz = 0};
	unsigned int cpu = Proc->CPU.Count, ratio = 0;

	if (Arch[Proc->ArchID].Query != NULL) {
		Arch[Proc->ArchID].Query();
	}
	if (Proc->Boost[BOOST(MAX)] > 0) {
		ratio = Proc->Boost[BOOST(MAX)];
	}
	if (ratio > 0) {
		if (Arch[Proc->ArchID].Clock != NULL) {
			clock = Arch[Proc->ArchID].Clock(ratio);
		}
	}
	if (clock.Hz == 0) {	// Fallback @ 100 MHz
		clock.Q = 100;
		clock.R = 0;
		clock.Hz = 100000000L;
	}
	if (Proc->Features.FactoryFreq != 0) {
	    if (ratio == 0) {
		ratio = Proc->Boost[BOOST(MAX)] = Proc->Features.FactoryFreq
						/ clock.Q;
	    }
	} else { // Fix factory frequency (MHz)
		Proc->Features.FactoryFreq = (ratio * clock.Hz) / 1000000L;
	}

	do {	// from last AP to BSP
	    cpu--;

	    if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)) {
		if ((AutoClock != 0) && (ratio != 0)) {
			clock = Base_Clock(cpu, ratio);
		} // else ENOMEM
		KPublic->Core[cpu]->Clock = clock;

		if (Arch[Proc->ArchID].Timer != NULL)
			Arch[Proc->ArchID].Timer(cpu);
		}
	} while (cpu != 0) ;
}

void Controller_Start(int wait)
{
	if (Arch[Proc->ArchID].Start != NULL) {
		unsigned int cpu;
		for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    if ((BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 1)
		     && (BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 0))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, wait);
	}
}

void Controller_Stop(int wait)
{
	if (Arch[Proc->ArchID].Stop != NULL) {
		unsigned int cpu;
		for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    if ((BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 1)
		     && (BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 1))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, wait);
	}
}

void Controller_Exit(void)
{
	unsigned int cpu;

	if (Arch[Proc->ArchID].Exit != NULL)
		Arch[Proc->ArchID].Exit();

	for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, CREATED);
}

void Core_Counters_Set(CORE *Core)
{
    if (Proc->Features.PerfMon.EAX.Version >= 2) {
	CORE_GLOBAL_PERF_CONTROL	Core_GlobalPerfControl = {.value = 0};
	CORE_FIXED_PERF_CONTROL 	Core_FixedPerfControl = {.value = 0};
	CORE_GLOBAL_PERF_STATUS 	Core_PerfOverflow = {.value = 0};
	CORE_GLOBAL_PERF_OVF_CTRL	Core_PerfOvfControl = {.value = 0};

	RDMSR(Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);
	Core->SaveArea.Core_GlobalPerfControl = Core_GlobalPerfControl;
	Core_GlobalPerfControl.EN_FIXED_CTR0  = 1;
	Core_GlobalPerfControl.EN_FIXED_CTR1  = 1;
	Core_GlobalPerfControl.EN_FIXED_CTR2  = 1;
	WRMSR(Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);

	RDMSR(Core_FixedPerfControl, MSR_CORE_PERF_FIXED_CTR_CTRL);
	Core->SaveArea.Core_FixedPerfControl = Core_FixedPerfControl;
	Core_FixedPerfControl.EN0_OS = 1;
	Core_FixedPerfControl.EN1_OS = 1;
	Core_FixedPerfControl.EN2_OS = 1;
	Core_FixedPerfControl.EN0_Usr = 1;
	Core_FixedPerfControl.EN1_Usr = 1;
	Core_FixedPerfControl.EN2_Usr = 1;

	if (Proc->Features.PerfMon.EAX.Version >= 3) {
		if (!Proc->Features.HTT_Enable) {
			Core_FixedPerfControl.AnyThread_EN0 = 1;
			Core_FixedPerfControl.AnyThread_EN1 = 1;
			Core_FixedPerfControl.AnyThread_EN2 = 1;
		} else {
			// Per Thread
			Core_FixedPerfControl.AnyThread_EN0 = 0;
			Core_FixedPerfControl.AnyThread_EN1 = 0;
			Core_FixedPerfControl.AnyThread_EN2 = 0;
		}
	}
	WRMSR(Core_FixedPerfControl, MSR_CORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Core_PerfOverflow, MSR_CORE_PERF_GLOBAL_STATUS);
	RDMSR(Core_PerfOvfControl, MSR_CORE_PERF_GLOBAL_OVF_CTRL);
	if (Core_PerfOverflow.Overflow_CTR0)
		Core_PerfOvfControl.Clear_Ovf_CTR0 = 1;
	if (Core_PerfOverflow.Overflow_CTR1)
		Core_PerfOvfControl.Clear_Ovf_CTR1 = 1;
	if (Core_PerfOverflow.Overflow_CTR2)
		Core_PerfOvfControl.Clear_Ovf_CTR2 = 1;
	if (Core_PerfOverflow.Overflow_CTR0
	  | Core_PerfOverflow.Overflow_CTR1
	  | Core_PerfOverflow.Overflow_CTR2)
		WRMSR(Core_PerfOvfControl, MSR_CORE_PERF_GLOBAL_OVF_CTRL);
    }
}

#define Uncore_Counters_Set(PMU, Core)					\
({									\
    if ((Proc->Features.PerfMon.EAX.Version >= 3) && (Core->T.Base.BSP))\
    {									\
	UNCORE_GLOBAL_PERF_CONTROL  Uncore_GlobalPerfControl;		\
	UNCORE_FIXED_PERF_CONTROL   Uncore_FixedPerfControl;		\
	UNCORE_GLOBAL_PERF_STATUS   Uncore_PerfOverflow = {.value = 0}; \
	UNCORE_GLOBAL_PERF_OVF_CTRL Uncore_PerfOvfControl = {.value = 0};\
									\
	RDMSR(Uncore_GlobalPerfControl, MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL);\
	Proc->SaveArea.Uncore_GlobalPerfControl = Uncore_GlobalPerfControl;\
	Uncore_GlobalPerfControl.PMU.EN_FIXED_CTR0  = 1;		\
	WRMSR(Uncore_GlobalPerfControl, MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL);\
									\
	RDMSR(Uncore_FixedPerfControl, MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL);\
	Proc->SaveArea.Uncore_FixedPerfControl = Uncore_FixedPerfControl;\
	Uncore_FixedPerfControl.PMU.EN_CTR0 = 1;			\
	WRMSR(Uncore_FixedPerfControl, MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL);\
									\
	RDMSR(Uncore_PerfOverflow, MSR_##PMU##_UNCORE_PERF_GLOBAL_STATUS);\
	if (Uncore_PerfOverflow.PMU.Overflow_CTR0) {			\
		RDMSR(Uncore_PerfOvfControl, MSR_UNCORE_PERF_GLOBAL_OVF_CTRL);\
		Uncore_PerfOvfControl.Clear_Ovf_CTR0 = 1;		\
		WRMSR(Uncore_PerfOvfControl, MSR_UNCORE_PERF_GLOBAL_OVF_CTRL);\
	}								\
    }									\
})

void Core_Counters_Clear(CORE *Core)
{
    if (Proc->Features.PerfMon.EAX.Version >= 2) {
	WRMSR(Core->SaveArea.Core_FixedPerfControl,
					MSR_CORE_PERF_FIXED_CTR_CTRL);
	WRMSR(Core->SaveArea.Core_GlobalPerfControl,
					MSR_CORE_PERF_GLOBAL_CTRL);
    }
}

#define Uncore_Counters_Clear(PMU, Core)				\
({									\
    if ((Proc->Features.PerfMon.EAX.Version >= 3) && (Core->T.Base.BSP))\
    {									\
	WRMSR(Proc->SaveArea.Uncore_FixedPerfControl,			\
				MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL);\
	WRMSR(Proc->SaveArea.Uncore_GlobalPerfControl,			\
				MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL);	\
    }									\
})

#define Counters_Generic(Core, T)					\
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
    if (!Proc->Features.AdvPower.EDX.Inv_TSC)				\
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

#define PKG_Counters_Generic(Core, T)					\
({									\
	Proc->Counter[T].PTSC = Core->Counter[T].TSC;			\
})

#define PKG_Counters_Nehalem(Core, T)					\
({									\
	RDTSCP_COUNTERx4(Proc->Counter[T].PTSC,				\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_NHM_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_SandyBridge(Core, T)				\
({									\
	RDTSCP_COUNTERx5(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_SNB_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_Haswell_ULT(Core, T)				\
({									\
	RDTSCP_COUNTERx7(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
			MSR_PKG_C8_RESIDENCY, Proc->Counter[T].PC08,	\
			MSR_PKG_C9_RESIDENCY, Proc->Counter[T].PC09,	\
			MSR_PKG_C10_RESIDENCY,Proc->Counter[T].PC10);	\
})

#define PKG_Counters_Skylake(Core, T)					\
({									\
	RDTSCP_COUNTERx5(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_SKL_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);	\
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

#define Delta_UNCORE_FC0(Pkg)						\
({									\
	Pkg->Delta.Uncore.FC0 =						\
		(Pkg->Counter[0].Uncore.FC0 >				\
		Pkg->Counter[1].Uncore.FC0) ?				\
			Pkg->Counter[0].Uncore.FC0			\
			- Pkg->Counter[1].Uncore.FC0			\
			: Pkg->Counter[1].Uncore.FC0			\
			- Pkg->Counter[0].Uncore.FC0;			\
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

#define Save_UNCORE_FC0(Pkg)						\
({									\
	Pkg->Counter[0].Uncore.FC0 = Pkg->Counter[1].Uncore.FC0;	\
})

void Core_Intel_Temp(CORE *Core)
{
	THERM_STATUS ThermStatus = {.value = 0};
	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);	// All Intel families.

	Core->PowerThermal.Sensor = ThermStatus.DTS;
	Core->PowerThermal.Trip = ThermStatus.StatusBit | ThermStatus.StatusLog;
}

void Core_AMD_Family_0Fh_Temp(CORE *Core)
{
	if (Proc->Features.AdvPower.EDX.TTP == 1) {
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

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_Generic(Core, 1);

			Delta_PTSC(Proc);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

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

void InitTimer_GenuineIntel(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_GenuineIntel, 1);
}

static void Start_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Intel_Query(Core, cpu);

	Counters_Generic(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_AuthenticAMD(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_Generic(Core, 1);

			Delta_PTSC(Proc);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

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

void InitTimer_AuthenticAMD(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AuthenticAMD, 1);
}

static void Start_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_AuthenticAMD_Query(Core, cpu);

	if (Core->T.Base.BSP) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Core2(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	unsigned int cpu = smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Core2(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_Generic(Core, 1);

			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->Counter[1].VID = PerfStatus.CORE.CurrVID;

			Delta_PTSC(Proc);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

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

static void Start_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Core2_Query(Core, cpu);

	Core_Counters_Set(Core);
	Counters_Core2(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Core_Counters_Clear(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Nehalem(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
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

			Delta_UNCORE_FC0(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Sys_Tick(Proc);
		}

		Core_Intel_Temp(Core);

		RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

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

static void Start_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Nehalem_Query(Core, cpu);

	Core_Counters_Set(Core);
	Uncore_Counters_Set(NHM, Core);
	SMT_Counters_Nehalem(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_Nehalem(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Core_Counters_Clear(Core);
	Uncore_Counters_Clear(NHM, Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}


static enum hrtimer_restart Cycle_SandyBridge(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_SandyBridge(Core, 1);

			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->Counter[1].VID = PerfStatus.SNB.CurrVID;

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC(Proc);

			Delta_UNCORE_FC0(Proc);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Sys_Tick(Proc);
		}

		Core_Intel_Temp(Core);

		RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

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

static void Start_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core, cpu);

	Core_Counters_Set(Core);
	Uncore_Counters_Set(SNB, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_SandyBridge(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Core_Counters_Clear(Core);
	Uncore_Counters_Clear(SNB, Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}


static enum hrtimer_restart Cycle_Haswell_ULT(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_Haswell_ULT(Core, 1);

			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->Counter[1].VID = PerfStatus.SNB.CurrVID;

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PC08(Proc);

			Delta_PC09(Proc);

			Delta_PC10(Proc);

			Delta_PTSC(Proc);

			Delta_UNCORE_FC0(Proc);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PC08(Proc);

			Save_PC09(Proc);

			Save_PC10(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Sys_Tick(Proc);
		}

		Core_Intel_Temp(Core);

		RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

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

void InitTimer_Haswell_ULT(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Haswell_ULT, 1);
}

static void Start_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Haswell_ULT_Query(Core, cpu);

	Core_Counters_Set(Core);
	if (Experimental)
		Uncore_Counters_Set(SNB, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_Haswell_ULT(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Core_Counters_Clear(Core);
	if (Experimental)
		Uncore_Counters_Clear(SNB, Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}


static enum hrtimer_restart Cycle_Skylake(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->T.Base.BSP) {
			PKG_Counters_Skylake(Core, 1);

			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->Counter[1].VID = PerfStatus.SNB.CurrVID;

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC(Proc);

			Delta_UNCORE_FC0(Proc);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Sys_Tick(Proc);
		}

		Core_Intel_Temp(Core);

		RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

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

void InitTimer_Skylake(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Skylake, 1);
}

static void Start_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core, cpu);

	Core_Counters_Set(Core);
	Uncore_Counters_Set(SKL, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->T.Base.BSP) {
		PKG_Counters_Skylake(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Core_Counters_Clear(Core);
	Uncore_Counters_Clear(SKL, Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}


static enum hrtimer_restart Cycle_AMD_Family_0Fh(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		FIDVID_STATUS FidVidStatus = {.value = 0};

		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		Core->Counter[1].VID = FidVidStatus.CurrVID;

		// P-States
		Core->Counter[1].C0.UCC = Core->Counter[0].C0.UCC
					+ (8 + FidVidStatus.CurrFID)
					* Core->Clock.Hz;

		Core->Counter[1].C0.URC = Core->Counter[1].C0.UCC;

		Core->Counter[1].TSC	= Core->Counter[0].TSC
				+ (Proc->Boost[BOOST(MAX)] * Core->Clock.Hz);

		/* Derive C1 */
		Core->Counter[1].C1 =
		  (Core->Counter[1].TSC > Core->Counter[1].C0.URC) ?
		    Core->Counter[1].TSC - Core->Counter[1].C0.URC
		    : 0;

		if (Core->T.Base.BSP) {
			PKG_Counters_Generic(Core, 1);

			Delta_PTSC(Proc);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

		Core_AMD_Family_0Fh_Temp(Core);

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

void InitTimer_AMD_Family_0Fh(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_0Fh, 1);
}

static void Start_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_AMD_Family_0Fh_Query(Core, cpu);

	if (Core->T.Base.BSP) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Start_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_AMD_Family_10h_Query(Core, cpu);

	if (Core->T.Base.BSP) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

/*
Note: hardware Family_12h

	if (Proc->Features.AdvPower.DX.CPB == 1)
	// Core Performance Boost [Here].
	    smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_12h, 1);
	else
	if (Proc->Features.Power.ECX.EffFreq == 1) // MPERF & APERF ?
*/

/*
Note: hardware Family_12h

static enum hrtimer_restart Cycle_AMD_Family_12h(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
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

long Sys_IdleDriver_Query(SYSGATE *SysGate)
{
    if (SysGate != NULL) {
	struct cpuidle_driver *idleDriver;
	struct cpufreq_policy freqPolicy;

	if ((idleDriver = cpuidle_get_driver()) != NULL) {
		int i;

		strncpy(SysGate->IdleDriver.Name,
			idleDriver->name,
			CPUIDLE_NAME_LEN - 1);

		if (idleDriver->state_count < CPUIDLE_STATE_MAX)
			SysGate->IdleDriver.stateCount=idleDriver->state_count;
		else	// No overflow check.
			SysGate->IdleDriver.stateCount=CPUIDLE_STATE_MAX;

		for (i = 0; i < SysGate->IdleDriver.stateCount; i++) {
			strncpy(SysGate->IdleDriver.State[i].Name,
				idleDriver->states[i].name,
				CPUIDLE_NAME_LEN - 1);

			SysGate->IdleDriver.State[i].exitLatency =
					idleDriver->states[i].exit_latency;
			SysGate->IdleDriver.State[i].powerUsage =
					idleDriver->states[i].power_usage;
			SysGate->IdleDriver.State[i].targetResidency =
					idleDriver->states[i].target_residency;
		}
	}
	else
		memset(&SysGate->IdleDriver, 0, sizeof(IDLEDRIVER));

	memset(&freqPolicy, 0, sizeof(freqPolicy));
	if (cpufreq_get_policy(&freqPolicy, smp_processor_id()) == 0) {
		struct cpufreq_governor *pGovernor = freqPolicy.governor;
		if (pGovernor != NULL)
			strncpy(SysGate->IdleDriver.Governor,
				pGovernor->name,
				CPUIDLE_NAME_LEN - 1);
	}
	return(0);
    }
    else
	return(-1);
}

long Sys_Kernel(SYSGATE *SysGate)
{	/* Sources:	/include/generated/uapi/linux/version.h
			/include/uapi/linux/utsname.h		*/
	if (SysGate != NULL) {
		SysGate->kernelVersionNumber = LINUX_VERSION_CODE;
		memcpy(SysGate->sysname, utsname()->sysname, MAX_UTS_LEN);
		memcpy(SysGate->release, utsname()->release, MAX_UTS_LEN);
		memcpy(SysGate->version, utsname()->version, MAX_UTS_LEN);
		memcpy(SysGate->machine, utsname()->machine, MAX_UTS_LEN);

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

static long CoreFreqK_ioctl(	struct file *filp,
				unsigned int cmd,
				unsigned long arg)
{
	long rc = -EPERM;

	switch (cmd) {
	case COREFREQ_IOCTL_SYSUPDT:
		if (Proc->SysGate != NULL) {
			Sys_DumpTask(Proc->SysGate);
			Sys_MemInfo(Proc->SysGate);
			rc = 0;
		}
		break;
	case COREFREQ_IOCTL_SYSONCE:
		rc = Sys_IdleDriver_Query(Proc->SysGate)
		   & Sys_Kernel(Proc->SysGate);
		break;
	case COREFREQ_IOCTL_MACHINE:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
					Controller_Stop(1);
					rc = 0;
				break;
			case COREFREQ_TOGGLE_ON:
					Controller_Start(1);
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_EIST:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					SpeedStep_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					SpeedStep_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_C1E:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					C1E_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					C1E_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_TURBO:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					TurboBoost_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					TurboBoost_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_C1A:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					C1A_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					C1A_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_C3A:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					C3A_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					C3A_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_C1U:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					C1U_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					C1U_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_C3U:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					C3U_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					C3U_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_PKGCST:
		PkgCStateLimit = arg;
		Controller_Stop(1);
		Controller_Start(1);
		PkgCStateLimit = -1;
		rc = 0;
		break;
	case COREFREQ_IOCTL_IOMWAIT:
		switch (arg) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
					IOMWAIT_Enable = arg;
					Controller_Stop(1);
					Controller_Start(1);
					IOMWAIT_Enable = -1;
					rc = 0;
				break;
		}
		break;
	case COREFREQ_IOCTL_IORCST:
		CStateIORedir = arg;
		Controller_Stop(1);
		Controller_Start(1);
		CStateIORedir = -1;
		rc = 0;
		break;
	case COREFREQ_IOCTL_ODCM:
		ODCM_Enable = arg;
		Controller_Stop(1);
		Controller_Start(1);
		ODCM_Enable = -1;
		rc = 0;
		break;
	case COREFREQ_IOCTL_ODCM_DC:
		ODCM_DutyCycle = arg;
		Controller_Stop(1);
		Controller_Start(1);
		ODCM_DutyCycle = -1;
		rc = 0;
		break;
	default:
		rc = -EINVAL;
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
	Controller_Stop(1);

	printk(KERN_NOTICE "CoreFreq: Suspend\n");

	return(0);
}

static int CoreFreqK_resume(struct device *dev)
{
	Controller_Start(0);

	printk(KERN_NOTICE "CoreFreq: Resume\n");

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
		 && !BITVAL(KPublic->Core[cpu]->OffLine, HW)) {
			if (!Core_Topology(cpu)) {
			    if (KPublic->Core[cpu]->T.ApicID >= 0)
				BITCLR(LOCKLESS,KPublic->Core[cpu]->OffLine,HW);
			    else
				BITSET(LOCKLESS,KPublic->Core[cpu]->OffLine,HW);
		    	}
		memcpy(&KPublic->Core[cpu]->Clock,
			&KPublic->Core[0]->Clock,
			sizeof(CLOCK) );
		}
		if (Arch[Proc->ArchID].Timer != NULL) {
			Arch[Proc->ArchID].Timer(cpu);
		}
		if ((BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 0)
		 && (Arch[Proc->ArchID].Start != NULL)) {
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, 0);
		}
		Proc->CPU.OnLine++;
		BITCLR(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);

		return(0);
	} else
		return(-EINVAL);
}

static int CoreFreqK_hotplug_cpu_offline(unsigned int cpu)
{
	if ((cpu >= 0) && (cpu < Proc->CPU.Count)) {
		if ((BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 1)
		 && (BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 1)
		 && (Arch[Proc->ArchID].Stop != NULL)) {
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, 1);
		}
		Proc->CPU.OnLine--;
		BITSET(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);

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
//	case CPU_ONLINE_FROZEN:
			rc = CoreFreqK_hotplug_cpu_online(cpu);
		break;
	case CPU_DOWN_PREPARE:
//	case CPU_DOWN_PREPARE_FROZEN:
			rc = CoreFreqK_hotplug_cpu_offline(cpu);
		break;
	default:
		break;
	}
	return(NOTIFY_OK);
}

static struct notifier_block CoreFreqK_notifier_block = {
	.notifier_call = CoreFreqK_hotplug,
};
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
static int CoreFreqK_NMI_handler(unsigned int type, struct pt_regs *pRegs)
{
	unsigned int cpu = smp_processor_id();

	switch (type) {
	case NMI_LOCAL:
		KPublic->Core[cpu]->Interrupt.NMI.LOCAL++;
		break;
	case NMI_UNKNOWN:
		KPublic->Core[cpu]->Interrupt.NMI.UNKNOWN++;
		break;
	case NMI_SERR:
		KPublic->Core[cpu]->Interrupt.NMI.PCISERR++;
		break;
	case NMI_IO_CHECK:
		KPublic->Core[cpu]->Interrupt.NMI.IOCHECK++;
		break;
	}
	return(NMI_DONE);
}
#endif

static int __init CoreFreqK_init(void)
{
	int rc = 0;
	ARG Arg = {.SMT_Count = 0, .rc = 0};

	// Query features on the presumed BSP processor.
	memset(&Arg.Features, 0, sizeof(FEATURES));
	if ((rc = smp_call_function_single(0, Query_Features, &Arg, 1)) == 0)
		rc = Arg.rc;
	if (rc == 0) {
		unsigned int OS_Count = num_present_cpus();
		// Rely on operating system's cpu counting.
		if (Arg.SMT_Count != OS_Count)
			Arg.SMT_Count = OS_Count;
	} else
		rc = -ENXIO;
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

		  publicSize=sizeof(KPUBLIC) + sizeof(CORE *) * Arg.SMT_Count;

		  privateSize=sizeof(KPRIVATE) + sizeof(JOIN *) * Arg.SMT_Count;

		    if (((KPublic = kmalloc(publicSize, GFP_KERNEL)) != NULL)
		     && ((KPrivate = kmalloc(privateSize, GFP_KERNEL)) != NULL))
		    {
			memset(KPublic, 0, publicSize);
			memset(KPrivate, 0, privateSize);

			packageSize = ROUND_TO_PAGES(sizeof(PROC));
			if ((Proc = kmalloc(packageSize, GFP_KERNEL)) != NULL)
			{
			    memset(Proc, 0, packageSize);
			    Proc->CPU.Count = Arg.SMT_Count;

			    if ( (SleepInterval >= LOOP_MIN_MS)
			      && (SleepInterval <= LOOP_MAX_MS))
				Proc->SleepInterval = SleepInterval;
			    else
				Proc->SleepInterval = LOOP_DEF_MS;

			// Compute the tick steps.
			    Proc->tickReset =
				 ( (TickInterval >= Proc->SleepInterval)
				&& (TickInterval <= LOOP_MAX_MS) ) ?
					TickInterval:KMAX(TICK_DEF_MS,
							  Proc->SleepInterval);
			    Proc->tickReset /= Proc->SleepInterval;
			    Proc->tickStep = Proc->tickReset;

			    Proc->Registration.Experimental = Experimental;

			    memcpy(&Proc->Features, &Arg.Features,
							sizeof(FEATURES));

			    Arch[0].Architecture=Proc->Features.Info.Vendor.ID;

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
				if (allocPerCPU)
				{
				  for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
					BITCLR( LOCKLESS,
						KPublic->Core[cpu]->Sync.V, 63);

					KPublic->Core[cpu]->Bind = cpu;

					Define_CPUID(KPublic->Core[cpu],
							CpuIDforVendor);
				  }

				  switch (Proc->Features.Info.Vendor.CRC) {
				  case CRC_INTEL: {
					Arch[0].Query = Query_GenuineIntel;
					Arch[0].Start = Start_GenuineIntel;
					Arch[0].Stop  = Stop_GenuineIntel;
					Arch[0].Timer = InitTimer_GenuineIntel;
					Arch[0].Clock = Clock_GenuineIntel;

					Arch[0].thermalFormula =
							THERMAL_FORMULA_INTEL;

					Arch[0].voltageFormula =
							VOLTAGE_FORMULA_INTEL;
					}
					break;
				  case CRC_AMD: {
					Arch[0].Query = Query_AuthenticAMD;
					Arch[0].Start = Start_AuthenticAMD;
					Arch[0].Stop  = Stop_AuthenticAMD;
					Arch[0].Timer = InitTimer_AuthenticAMD;
					Arch[0].Clock = Clock_AuthenticAMD;

					Arch[0].thermalFormula =
							THERMAL_FORMULA_AMD;

					Arch[0].voltageFormula =
							VOLTAGE_FORMULA_AMD;
					}
					break;
				  }
				  if ( (Proc->Features.Std.ECX.Hyperv == 1)
				    && (ArchID == -1) ) {
					ArchID = 0;
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
				    if((Proc->Features.Std.EAX.ExtFamily
				    == Arch[Proc->ArchID].Signature.ExtFamily)
				    && (Proc->Features.Std.EAX.Family
				    == Arch[Proc->ArchID].Signature.Family)
				    && (((Proc->Features.Std.EAX.ExtModel
				      ==  Arch[Proc->ArchID].Signature.ExtModel)
				      && (Proc->Features.Std.EAX.Model
				      ==  Arch[Proc->ArchID].Signature.Model))
				      || (!Arch[Proc->ArchID].Signature.ExtModel
				      &&  !Arch[Proc->ArchID].Signature.Model)))
					{
						break;
					}
				    }
				  }

				  Proc->thermalFormula =
					Arch[Proc->ArchID].thermalFormula;

				  Proc->voltageFormula =
					Arch[Proc->ArchID].voltageFormula;

				  strncpy(Proc->Architecture,
					Arch[Proc->ArchID].Architecture, 32);

				  Controller_Init();

				  printk(KERN_INFO "CoreFreq:"		\
				      " Processor [%2X%1X_%1X%1X]"	\
				      " Architecture [%s] CPU [%u/%u]\n",
					Proc->Features.Std.EAX.ExtFamily,
					Proc->Features.Std.EAX.Family,
					Proc->Features.Std.EAX.ExtModel,
					Proc->Features.Std.EAX.Model,
					Arch[Proc->ArchID].Architecture,
					Proc->CPU.OnLine,
					Proc->CPU.Count);

				  Controller_Start(0);

				if (Proc->Registration.Experimental) {
				  Proc->Registration.pci |=CoreFreqK_ProbePCI();
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
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)
				  if (!NMI_Disable) {
				    Proc->Registration.nmi =
					!( register_nmi_handler(NMI_LOCAL,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk")
					| register_nmi_handler(NMI_UNKNOWN,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk")
					| register_nmi_handler(NMI_SERR,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk")
					| register_nmi_handler(NMI_IO_CHECK,
							CoreFreqK_NMI_handler,
							0,
							"corefreqk"));
				  }
		#endif
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
	if (Proc != NULL) {
		unsigned int cpu = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0)
		if (Proc->Registration.nmi) {
			unregister_nmi_handler(NMI_LOCAL,    "corefreqk");
			unregister_nmi_handler(NMI_UNKNOWN,  "corefreqk");
			unregister_nmi_handler(NMI_SERR,     "corefreqk");
			unregister_nmi_handler(NMI_IO_CHECK, "corefreqk");
		}
#endif
#ifdef CONFIG_HOTPLUG_CPU
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
		unregister_hotcpu_notifier(&CoreFreqK_notifier_block);
	#else
		if (!(Proc->Registration.hotplug < 0))
			cpuhp_remove_state_nocalls(Proc->Registration.hotplug);
	#endif
#endif
		Controller_Stop(1);
		Controller_Exit();

		if (Proc->SysGate != NULL)
			kfree(Proc->SysGate);

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

		if (KPublic != NULL)
			kfree(KPublic);
		if (KPrivate != NULL)
			kfree(KPrivate);

		device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
		class_destroy(CoreFreqK.clsdev);
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		printk(KERN_NOTICE "CoreFreq: Unload\n");

		kfree(Proc);
	}
}

module_init(CoreFreqK_init);
module_exit(CoreFreqK_cleanup);
