/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/cpuidle.h>
#include <asm/msr.h>

#include "coretypes.h"
#include "intelasm.h"
#include "intelmsr.h"
#include "corefreq-api.h"
#include "corefreqk.h"

MODULE_AUTHOR ("CYRIL INGENIERIE <labs[at]cyring[dot]fr>");
MODULE_DESCRIPTION ("CoreFreq Processor Driver");
MODULE_SUPPORTED_DEVICE ("Intel Core Core2 Atom Xeon i3 i5 i7");
MODULE_LICENSE ("GPL");

typedef struct {
	FEATURES	features;
	unsigned int	count;
} ARG;

static struct
{
	signed int	Major;
	struct cdev	*kcdev;
	dev_t		nmdev, mkdev;
	struct class	*clsdev;
} CoreFreqK;

static signed int ArchID=-1;
module_param(ArchID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ArchID, "Force an architecture (ID)");

static signed int AutoClock=1;
module_param(AutoClock, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(AutoClock, "Auto estimate the clock frequency");

static signed int SleepInterval=0;
module_param(SleepInterval, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SleepInterval, "Sleep interval (ms) of the loops");

static signed int IdleDriverQuery=0;
module_param(IdleDriverQuery, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(IdleDriverQuery, "Query information from the idle driver");

static PROC *Proc=NULL;
static KPUBLIC *KPublic=NULL;
static KPRIVATE *KPrivate=NULL;
static ktime_t RearmTheTimer;


unsigned int Intel_Brand(char *pBrand)
{
	char idString[64]={0x20};
	unsigned int ix=0, jx=0, px=0;
	unsigned int frequency=0, multiplier=0;
	BRAND Brand;

	for(ix=0; ix < 3; ix++)
	{
		asm volatile
		(
			"cpuid"
			: "=a"  (Brand.AX),
			  "=b"  (Brand.BX),
			  "=c"  (Brand.CX),
			  "=d"  (Brand.DX)
			: "a"   (0x80000002 + ix)
		);
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.AX.Chr[jx];
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.BX.Chr[jx];
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.CX.Chr[jx];
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.DX.Chr[jx];
	}
	for(ix=0; ix < 46; ix++)
		if((idString[ix+1] == 'H') && (idString[ix+2] == 'z'))
		{
			switch(idString[ix])
			{
				case 'M':
					multiplier=1;
				break;
				case 'G':
					multiplier=1000;
				break;
				case 'T':
					multiplier=1000000;
				break;
			}
			break;
		}
	if(multiplier > 0)
	{
	    if(idString[ix-3] == '.')
	    {
		frequency  = (int) (idString[ix-4] - '0') * multiplier;
		frequency += (int) (idString[ix-2] - '0') * (multiplier / 10);
		frequency += (int) (idString[ix-1] - '0') * (multiplier / 100);
	    }
	    else
	    {
		frequency  = (int) (idString[ix-4] - '0') * 1000;
		frequency += (int) (idString[ix-3] - '0') * 100;
		frequency += (int) (idString[ix-2] - '0') * 10;
		frequency += (int) (idString[ix-1] - '0');
		frequency *= frequency;
	    }
	}
	for(ix=jx=0; jx < 48; jx++)
		if(!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++]=idString[jx];

	return(frequency);
}

void AMD_Brand(char *pBrand)
{
	char idString[64]={0x20};
	unsigned int ix=0, jx=0, px=0;
	BRAND Brand;

	for(ix=0; ix < 3; ix++)
	{
		asm volatile
		(
			"cpuid"
			: "=a"  (Brand.AX),
			  "=b"  (Brand.BX),
			  "=c"  (Brand.CX),
			  "=d"  (Brand.DX)
			: "a"   (0x80000002 + ix)
		);
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.AX.Chr[jx];
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.BX.Chr[jx];
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.CX.Chr[jx];
		for(jx=0; jx < 4; jx++, px++)
			idString[px]=Brand.DX.Chr[jx];
	}
	for(ix=jx=0; jx < 48; jx++)
		if(!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++]=idString[jx];
}

// Retreive the Processor features through calls to the CPUID instruction.
void Query_Features(void *pArg)
{
	ARG *arg=(ARG *) pArg;

	// Extended Function CPUID Information. CPUID 0x80000000
	unsigned int LargestExtFunc;
	unsigned int AX=0x0, BX=0x0, CX=0x0, DX=0x0;	// DWORD Only!

	asm volatile
	(
		"cpuid"
		: "=a"	(arg->features.Info.LargestStdFunc),
		  "=b"	(BX),
		  "=d"	(DX),
		  "=c"	(CX)
                : "a" (0x0)
	);
	arg->features.Info.VendorID[ 0]=BX;
	arg->features.Info.VendorID[ 1]=(BX >> 8);
	arg->features.Info.VendorID[ 2]=(BX >> 16);
	arg->features.Info.VendorID[ 3]=(BX >> 24);
	arg->features.Info.VendorID[ 4]=DX;
	arg->features.Info.VendorID[ 5]=(DX >> 8);
	arg->features.Info.VendorID[ 6]=(DX >> 16);
	arg->features.Info.VendorID[ 7]=(DX >> 24);
	arg->features.Info.VendorID[ 8]=CX;
	arg->features.Info.VendorID[ 9]=(CX >> 8);
	arg->features.Info.VendorID[10]=(CX >> 16);
	arg->features.Info.VendorID[11]=(CX >> 24);
	arg->features.Info.VendorID[12]='\0';

	asm volatile
	(
		"cpuid"
		: "=a"	(arg->features.Std.AX),
		  "=b"	(arg->features.Std.BX),
		  "=c"	(arg->features.Std.CX),
		  "=d"	(arg->features.Std.DX)
                : "a" (0x1)
	);
	asm volatile
	(
		"cpuid"
		: "=a"	(arg->features.MWait.AX),
		  "=b"	(arg->features.MWait.BX),
		  "=c"	(arg->features.MWait.CX),
		  "=d"	(arg->features.MWait.DX)
                : "a" (0x5)
	);
	asm volatile
	(
		"cpuid"
		: "=a"	(arg->features.Power.AX),
		  "=b"	(arg->features.Power.BX),
		  "=c"	(arg->features.Power.CX),
		  "=d"	(arg->features.Power.DX)
                : "a" (0x6)
	);
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
		: "=r"	(arg->features.ExtFeature.AX),
		  "=r"	(arg->features.ExtFeature.BX),
		  "=r"	(arg->features.ExtFeature.CX),
		  "=r"	(arg->features.ExtFeature.DX)
                :
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	asm volatile
	(
		"cpuid"
		: "=a"	(AX),
		  "=b"	(BX),
		  "=c"	(CX),
		  "=d"	(DX)
		: "a"	(0x4),
		  "c"	(0x0)
	);
	if(!strncmp(arg->features.Info.VendorID, VENDOR_INTEL, 12))
	{
		arg->count=(AX >> 26) & 0x3f;
		arg->count++;

		asm volatile
		(
			"cpuid"
			: "=a"	(arg->features.PerfMon.AX),
			  "=b"	(arg->features.PerfMon.BX),
			  "=c"	(arg->features.PerfMon.CX),
			  "=d"	(arg->features.PerfMon.DX)
	                : "a" (0xa)
		);

		arg->features.FactoryFreq=Intel_Brand(arg->features.Info.Brand);
	}
	else if(!strncmp(arg->features.Info.VendorID, VENDOR_AMD, 12))
	{
		if(arg->features.Std.DX.HTT)
			arg->count=arg->features.Std.BX.MaxThread;
		else
			arg->count=(BX >> 16) & 0x0ff;

		AMD_Brand(arg->features.Info.Brand);

		arg->features.FactoryFreq=0;
	}
	// Common x86
	asm volatile
	(
		"cpuid"
		: "=a"	(LargestExtFunc)
                : "a" (0x80000000)
	);
	if(LargestExtFunc >= 0x80000004 && LargestExtFunc <= 0x80000008)
	{
		asm volatile
		(
			"cpuid"
			: "=d"	(arg->features.InvariantTSC)
			: "a" (0x80000007)
		);
		arg->features.InvariantTSC &= 0x100;
		arg->features.InvariantTSC >>= 8;

		asm volatile
		(
			"cpuid"
			: "=c"	(arg->features.ExtInfo.CX),
			  "=d"	(arg->features.ExtInfo.DX)
			: "a" (0x80000001)
		);
	}
}


DECLARE_COMPLETION(bclk_job_complete);

typedef	struct {
	unsigned long long V[2];
} TSC_STRUCT;

#define	OCCURRENCES 8

signed int Compute_Clock(void *arg)
{
	CLOCK *clock=(CLOCK *) arg;
	unsigned int ratio=clock->Q;
	struct kmem_cache *hardwareCache=kmem_cache_create(
				"CoreFreqCache",
				OCCURRENCES * sizeof(TSC_STRUCT), 0,
				SLAB_HWCACHE_ALIGN, NULL);
	TSC_STRUCT *TSC[2]={
		kmem_cache_alloc(hardwareCache, GFP_KERNEL),
		kmem_cache_alloc(hardwareCache, GFP_KERNEL)
	};
	unsigned long long D[2][OCCURRENCES];
	unsigned int loop=0, what=0, best[2]={0, 0}, top[2]={0, 0};

	void ComputeWithInvariantTSC(void)
	{
		// No preemption, no interrupt.
		unsigned long flags;
		preempt_disable();
		raw_local_irq_save(flags);

		// Warm-up
		for(loop=0; loop < OCCURRENCES; loop++)
		{
			RDTSCP64(TSC[0][loop].V[0]);
			RDTSCP64(TSC[0][loop].V[1]);
		}
		// Pick-up
		for(loop=0; loop < OCCURRENCES; loop++)
		{
			RDTSCP64(TSC[1][loop].V[0]);

			udelay(1000);

			RDTSCP64(TSC[1][loop].V[1]);
		}
		// Restore preemption and interrupt.
		raw_local_irq_restore(flags);
		preempt_enable();
	}

	void ComputeWithVariantTSC(void)
	{
		// No preemption, no interrupt.
		unsigned long flags;
		preempt_disable();
		raw_local_irq_save(flags);

		// Warm-up
		for(loop=0; loop < OCCURRENCES; loop++)
		{
			RDTSC64(TSC[0][loop].V[0]);
			RDTSC64(TSC[0][loop].V[1]);
		}
		// Pick-up
		for(loop=0; loop < OCCURRENCES; loop++)
		{
			RDTSC64(TSC[1][loop].V[0]);

			udelay(1000);

			RDTSC64(TSC[1][loop].V[1]);
		}
		// Restore preemption and interrupt.
		raw_local_irq_restore(flags);
		preempt_enable();
	}

	if(Proc->Features.InvariantTSC)
		ComputeWithInvariantTSC();
	else
		ComputeWithVariantTSC();


	memset(D, 0, 2 * OCCURRENCES);
	for(loop=0; loop < OCCURRENCES; loop++)
		for(what=0; what < 2; what++) {
			D[what][loop] 	= TSC[what][loop].V[1]
					- TSC[what][loop].V[0];
		}
	for(loop=0; loop < OCCURRENCES; loop++) {
		unsigned int inner=0, count[2]={0, 0};
		for(inner=loop; inner < OCCURRENCES; inner++) {
			for(what=0; what < 2; what++) {
				if(D[what][loop] == D[what][inner])
					count[what]++;
			}
		}
		for(what=0; what < 2; what++) {
			if((count[what] > top[what])
			||((count[what] == top[what])
			&& (D[what][loop] < D[what][best[what]]))) {
				top[what]=count[what];
				best[what]=loop;

			}
		}
	}	// Select the best clock.
	D[1][best[1]] -= D[0][best[0]];
	D[1][best[1]] *= 1000;

	clock->Q=D[1][best[1]] / (1000000L * ratio);
	clock->R=D[1][best[1]] % (1000000L * ratio);

	clock->Hz=D[1][best[1]] / ratio;
	clock->Hz+=D[1][best[1]] % ratio;

	kmem_cache_free(hardwareCache, TSC[1]);
	kmem_cache_free(hardwareCache, TSC[0]);
	kmem_cache_destroy(hardwareCache);

	complete_and_exit(&bclk_job_complete, 0);
}

CLOCK Base_Clock(unsigned int cpu, unsigned int ratio)
{
	CLOCK clock={.Q=ratio, .R=0, .Hz=0};

	struct task_struct *tid=kthread_create(	Compute_Clock,
						&clock,
						"baseclock/%-3d",
						cpu);
	if(!IS_ERR(tid))
	{
		kthread_bind(tid, cpu);
		wake_up_process(tid);
		wait_for_completion(&bclk_job_complete);
		printk(KERN_DEBUG "CoreFreq:"				\
				" CPU #%-3d Base Clock @ %llu Hz\n",
				cpu, clock.Hz);
	}
	return(clock);
}

void ClockToHz(CLOCK *clock)
{
	clock->Hz =clock->Q * 1000000L;
	clock->Hz+=clock->R * PRECISION;
}

// [Genuine Intel]
CLOCK Clock_GenuineIntel(unsigned int ratio)
{
	CLOCK clock={.Q=100, .R=0, .Hz=100000000L};

	if(Proc->Features.FactoryFreq > 0)
	{
		clock.Hz=(Proc->Features.FactoryFreq * 1000000L) / ratio;
		clock.Q=Proc->Features.FactoryFreq / ratio;
		clock.R=(Proc->Features.FactoryFreq % ratio) * PRECISION;
	}
	return(clock);
};

// [Authentic AMD]
CLOCK Clock_AuthenticAMD(unsigned int ratio)
{
	CLOCK clock={.Q=100, .R=0, .Hz=100000000L};
	return(clock);
};

// [Core]
CLOCK Clock_Core(unsigned int ratio)
{
	CLOCK clock={.Q=100, .R=0};
	FSB_FREQ FSB={.value=0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed)
	{
		case 0b101: {
			clock.Q=100;
			clock.R=0;
		};
		break;
		case 0b001: {
			clock.Q=133;
			clock.R=3333;
		}
		break;
		case 0b011: {
			clock.Q=166;
			clock.R=6666;
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
	CLOCK clock={.Q=100, .R=0};
	FSB_FREQ FSB={.value=0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed)
	{
		case 0b101: {
			clock.Q=100;
			clock.R=0;
		}
		break;
		case 0b001: {
			clock.Q=133;
			clock.R=3333;
		}
		break;
		case 0b011: {
			clock.Q=166;
			clock.R=6666;
		}
		break;
		case 0b010: {
			clock.Q=200;
			clock.R=0;
		}
		break;
		case 0b000: {
			clock.Q=266;
			clock.R=6666;
		}
		break;
		case 0b100: {
			clock.Q=333;
			clock.R=3333;
		}
		break;
		case 0b110: {
			clock.Q=400;
			clock.R=0;
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
	CLOCK clock={.Q=83, .R=0};
	FSB_FREQ FSB={.value=0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed)
	{
		case 0b111: {
			clock.Q=83;
			clock.R=2000;
		}
		break;
		case 0b101: {
			clock.Q=99;
			clock.R=8400;
		}
		break;
		case 0b001: {
			clock.Q=133;
			clock.R=2000;
		}
		break;
		case 0b011: {
			clock.Q=166;
			clock.R=4000;
		}
		break;
		default: {
			clock.Q=83;
			clock.R=2000;
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
	CLOCK clock={.Q=83, .R=3};
	FSB_FREQ FSB={.value=0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch(FSB.Bus_Speed)
	{
		case 0b100: {
			clock.Q=80;
			clock.R=0;
		}
		break;
		case 0b000: {
			clock.Q=83;
			clock.R=3000;
		}
		break;
		case 0b001: {
			clock.Q=100;
			clock.R=0;
		}
		break;
		case 0b010: {
			clock.Q=133;
			clock.R=3333;
		}
		break;
		case 0b011: {
			clock.Q=116;
			clock.R=7000;
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
	CLOCK clock={.Q=133, .R=3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Westmere]
CLOCK Clock_Westmere(unsigned int ratio)
{
	CLOCK clock={.Q=133, .R=3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [SandyBridge]
CLOCK Clock_SandyBridge(unsigned int ratio)
{
	CLOCK clock={.Q=100, .R=0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [IvyBridge]
CLOCK Clock_IvyBridge(unsigned int ratio)
{
	CLOCK clock={.Q=100, .R=0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

// [Haswell]
CLOCK Clock_Haswell(unsigned int ratio)
{
	CLOCK clock={.Q=100, .R=0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return(clock);
};

void Cache_Topology(CORE *Core)
{
	unsigned int level=0x0;
	for(level=0; level < CACHE_MAX_LEVEL; level++)
	{
		asm volatile
		(
			"cpuid"
			: "=a"	(Core->T.Cache[level].AX),
			  "=b"	(Core->T.Cache[level].BX),
			  "=c"	(Core->T.Cache[level].Set),
			  "=d"	(Core->T.Cache[level].DX)
			: "a"	(0x4),
			  "c"	(level)
		);
		if(!Core->T.Cache[level].Type)
			break;
	}
}

// Enumerate the Processor's Cores and Threads topology.
void Map_Topology(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		FEATURES features;

		RDMSR(Core->T.Base, MSR_IA32_APICBASE);

		asm volatile
		(
			"cpuid"
			: "=b"	(features.Std.BX)
			: "a"	(0x1)
		);
		Core->T.ApicID=features.Std.BX.Apic_ID;

		Cache_Topology(Core);
	}
}

void Map_Extended_Topology(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;

		int	InputLevel=0, NoMoreLevels=0,
			SMT_Mask_Width=0, SMT_Select_Mask=0,
			CorePlus_Mask_Width=0, CoreOnly_Select_Mask=0;

		CPUID_TOPOLOGY_LEAF ExtTopology=
		{
			.AX.Register=0,
			.BX.Register=0,
			.CX.Register=0,
			.DX.Register=0
		};

		RDMSR(Core->T.Base, MSR_IA32_APICBASE);

		do
		{
			asm volatile
			(
				"cpuid"
				: "=a"	(ExtTopology.AX),
				  "=b"	(ExtTopology.BX),
				  "=c"	(ExtTopology.CX),
				  "=d"	(ExtTopology.DX)
				:  "a"	(0xb),
				   "c"	(InputLevel)
			);
			// Exit from the loop if the BX register equals 0 or
			// if the requested level exceeds the level of a Core.
			if(!ExtTopology.BX.Register
			|| (InputLevel > LEVEL_CORE))
				NoMoreLevels=1;
			else
			{
			    switch(ExtTopology.CX.Type)
			    {
			    case LEVEL_THREAD:
				{
				SMT_Mask_Width  = ExtTopology.AX.SHRbits;
				SMT_Select_Mask = ~((-1) << SMT_Mask_Width);
				Core->T.ThreadID= ExtTopology.DX.x2ApicID \
						& SMT_Select_Mask;
				}
			    break;
			    case LEVEL_CORE:
				{
				CorePlus_Mask_Width=ExtTopology.AX.SHRbits;
				CoreOnly_Select_Mask=			\
					(~((-1) << CorePlus_Mask_Width))\
					^ SMT_Select_Mask;
				Core->T.CoreID=				\
				    (ExtTopology.DX.x2ApicID		\
				    & CoreOnly_Select_Mask) >> SMT_Mask_Width;
				}
			    break;
			    }
			}
			InputLevel++;
		}
		while(!NoMoreLevels);

		Core->T.ApicID=ExtTopology.DX.x2ApicID;

		Cache_Topology(Core);
	}
}

int Core_Topology(unsigned int cpu)
{
	int rc=smp_call_function_single(cpu,
				(Proc->Features.Info.LargestStdFunc >= 0xb) ?
				Map_Extended_Topology : Map_Topology,
				KPublic->Core[cpu],
				1); // Synchronous call.

	if(!rc
	&& !Proc->Features.HTT_Enable
	&& (KPublic->Core[cpu]->T.ThreadID > 0))
		Proc->Features.HTT_Enable=1;

	return(rc);
}

unsigned int Proc_Topology(void)
{
	unsigned int cpu=0, CountEnabledCPU=0;

	for(cpu=0; cpu < Proc->CPU.Count; cpu++)
	{
		KPublic->Core[cpu]->T.Base.value=0;
		KPublic->Core[cpu]->T.ApicID=-1;
		KPublic->Core[cpu]->T.CoreID=-1;
		KPublic->Core[cpu]->T.ThreadID=-1;

		// CPU state based on the OS
		if( !(KPublic->Core[cpu]->OffLine.OS = !cpu_online(cpu)
						   || !cpu_active(cpu)) )
		{
			if(!Core_Topology(cpu))
			{
				// CPU state based on the harware
				if(KPublic->Core[cpu]->T.ApicID >= 0)
				{
					KPublic->Core[cpu]->OffLine.HW=0;
					CountEnabledCPU++;
				}
				else
					KPublic->Core[cpu]->OffLine.HW=1;
			}
		}
	}
	return(CountEnabledCPU);
}

void HyperThreading_Technology(void)
{
	unsigned int CountEnabledCPU=Proc_Topology();

	if(Proc->Features.Std.DX.HTT)
		Proc->CPU.OnLine=CountEnabledCPU;
	else
		Proc->CPU.OnLine=Proc->CPU.Count;
}

void Intel_Platform_Info(void)
{
	PLATFORM_INFO Platform={.value=0};

	RDMSR(Platform, MSR_PLATFORM_INFO);

	if(Platform.value != 0)
	{
	  Proc->Boost[0]=MINCOUNTER(Platform.MinimumRatio,\
				Platform.MaxNonTurboRatio);
	  Proc->Boost[1]=MAXCOUNTER(Platform.MinimumRatio,\
				Platform.MaxNonTurboRatio);
	}
	Proc->Boost[9]=Proc->Boost[1];
}

void DynamicAcceleration(void)
{
	struct THERMAL_POWER_LEAF thermal_Power_Leaf={
		.AX={0}, .BX={0}, .CX={0}, .DX={0}
	};
	asm volatile
	(
		"cpuid"
		: "=a"	(thermal_Power_Leaf.AX),
		  "=b"	(thermal_Power_Leaf.BX),
		  "=c"	(thermal_Power_Leaf.CX),
		  "=d"	(thermal_Power_Leaf.DX)
                : "a" (0x6)
	);

	if(thermal_Power_Leaf.AX.TurboIDA == 1)
		Proc->Boost[9]=Proc->Boost[1] + 1;
}

#ifndef MSR_TURBO_RATIO_LIMIT
#define MSR_TURBO_RATIO_LIMIT MSR_NHM_TURBO_RATIO_LIMIT
#endif
void Nehalem_Platform_Info(void)
{
	PLATFORM_INFO Platform={.value=0};
	TURBO_RATIO Turbo={.value=0};

	RDMSR(Platform, MSR_PLATFORM_INFO);
	RDMSR(Turbo, MSR_TURBO_RATIO_LIMIT);

	Proc->Boost[0]=Platform.MinimumRatio;
	Proc->Boost[1]=Platform.MaxNonTurboRatio;
	Proc->Boost[2]=Turbo.MaxRatio_8C;
	Proc->Boost[3]=Turbo.MaxRatio_7C;
	Proc->Boost[4]=Turbo.MaxRatio_6C;
	Proc->Boost[5]=Turbo.MaxRatio_5C;
	Proc->Boost[6]=Turbo.MaxRatio_4C;
	Proc->Boost[7]=Turbo.MaxRatio_3C;
	Proc->Boost[8]=Turbo.MaxRatio_2C;
	Proc->Boost[9]=Turbo.MaxRatio_1C;
}

void Query_GenuineIntel(void)
{
	Intel_Platform_Info();
	HyperThreading_Technology();
}

void Query_AuthenticAMD(void)
{
	Proc->CPU.OnLine=Proc->CPU.Count;

	Proc->Boost[0]=1;
	Proc->Boost[1]=10;
	Proc->Boost[9]=10;
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

void SpeedStep_Technology(CORE *Core)				// Per Package!
{
	MISC_PROC_FEATURES MiscFeatures={.value=0};

	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	Core->Query.EIST=Proc->Features.Std.CX.EIST & MiscFeatures.EIST;
}

void TurboBoost_Technology(CORE *Core)
{
	struct THERMAL_POWER_LEAF thermal_Power_Leaf={
		.AX={0}, .BX={0}, .CX={0}, .DX={0}
	};
	asm volatile
	(
		"cpuid"
		: "=a"	(thermal_Power_Leaf.AX),
		  "=b"	(thermal_Power_Leaf.BX),
		  "=c"	(thermal_Power_Leaf.CX),
		  "=d"	(thermal_Power_Leaf.DX)
                : "a" (0x6)
	);

	if(thermal_Power_Leaf.AX.TurboIDA)			// Per Thread
	{
		PERF_CONTROL PerfControl={.value=0};

		RDMSR(PerfControl, MSR_IA32_PERF_CTL);

		Core->Query.Turbo= !PerfControl.Turbo_IDA;
	}
}

void Enhanced_Halt_State(CORE *Core)
{
	POWER_CONTROL PowerCtrl={.value=0};

	RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);			// Per Core

	Core->Query.C1E=PowerCtrl.C1E;				// Per Package!
}

void ThermalMonitor_Set(CORE *Core)
{
	TJMAX TjMax={.value=0};
	MISC_PROC_FEATURES MiscFeatures={.value=0};
	THERM2_CONTROL Therm2Control={.value=0};

	RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);

	Core->Thermal.Target=TjMax.Target;
	if(!Core->Thermal.Target)
		Core->Thermal.Target=100;

	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	Core->Thermal.TCC_Enable = MiscFeatures.TCC;
	Core->Thermal.TM2_Enable = MiscFeatures.TM2_Enable;

	RDMSR(Therm2Control, MSR_THERM2_CTL);

	Core->Thermal.TM2_Enable = Therm2Control.TM_SELECT;
}

#define PerCore_Intel_Query(Core)					\
({									\
	ThermalMonitor_Set(Core);					\
})

void PerCore_Core2_Query(CORE *Core)
{
	SpeedStep_Technology(Core);
	ThermalMonitor_Set(Core);
}

void PerCore_Nehalem_Query(CORE *Core)
{
	SpeedStep_Technology(Core);
	TurboBoost_Technology(Core);
	Enhanced_Halt_State(Core);

	if(Core->T.ThreadID == 0)				// Per Core
	{
		CSTATE_CONFIG CStateConfig={.value=0};

		RDMSR(CStateConfig, MSR_NHM_SNB_PKG_CST_CFG_CTL);

		Core->Query.C3A=CStateConfig.C3autoDemotion;
		Core->Query.C1A=CStateConfig.C1autoDemotion;
	}
	ThermalMonitor_Set(Core);
}

void PerCore_SandyBridge_Query(CORE *Core)
{
	SpeedStep_Technology(Core);
	TurboBoost_Technology(Core);
	Enhanced_Halt_State(Core);

	if(Core->T.ThreadID == 0)				// Per Core
	{
		CSTATE_CONFIG CStateConfig={.value=0};

		RDMSR(CStateConfig, MSR_NHM_SNB_PKG_CST_CFG_CTL);

		Core->Query.C3A=CStateConfig.C3autoDemotion;
		Core->Query.C1A=CStateConfig.C1autoDemotion;
		Core->Query.C3U=CStateConfig.C3undemotion;
		Core->Query.C1U=CStateConfig.C1undemotion;
	}
	ThermalMonitor_Set(Core);
}

void InitTimer(void *Cycle_Function)
{
	unsigned int cpu=smp_processor_id();

	if(KPrivate->Join[cpu]->tsm.created == 0)
	{
		hrtimer_init(	&KPrivate->Join[cpu]->Timer,
				CLOCK_MONOTONIC,
				HRTIMER_MODE_REL_PINNED);
		KPrivate->Join[cpu]->Timer.function=Cycle_Function;

		KPrivate->Join[cpu]->tsm.created=1;

		printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Init Timer\n", cpu);
	}
}

void Controller_Init(void)
{
	CLOCK clock;
	unsigned int cpu=Proc->CPU.Count;

	if(Arch[Proc->ArchID].Query != NULL)
		Arch[Proc->ArchID].Query();

	do {	// from last AP to BSP
		cpu--;
		if(!KPublic->Core[cpu]->OffLine.OS)
		{
			if(AutoClock)
				clock=Base_Clock(cpu, Proc->Boost[1]);
			else if(Arch[Proc->ArchID].Clock != NULL)
				clock=Arch[Proc->ArchID].Clock(Proc->Boost[1]);

			KPublic->Core[cpu]->Clock=clock;

			if(Arch[Proc->ArchID].Timer != NULL)
				Arch[Proc->ArchID].Timer(cpu);
		}
	} while(cpu != 0) ;
}

void Controller_Start(void)
{
	if(Arch[Proc->ArchID].Start != NULL)
	{
		unsigned int cpu=0;
		for(cpu=0; cpu < Proc->CPU.Count; cpu++)
		    if(	(KPrivate->Join[cpu]->tsm.created == 1)
		    &&	(KPrivate->Join[cpu]->tsm.started == 0))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, 0);
	}
}

void Controller_Stop(void)
{
	if(Arch[Proc->ArchID].Stop != NULL)
	{
		unsigned int cpu=0;
		for(cpu=0; cpu < Proc->CPU.Count; cpu++)
		    if(	(KPrivate->Join[cpu]->tsm.created == 1)
		    &&	(KPrivate->Join[cpu]->tsm.started == 1))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, 1);
	}
}

void Controller_Exit(void)
{
	unsigned int cpu=0;

	if(Arch[Proc->ArchID].Exit != NULL)
		Arch[Proc->ArchID].Exit();

	for(cpu=0; cpu < Proc->CPU.Count; cpu++)
		KPrivate->Join[cpu]->tsm.created=0;
}

void Counters_Set(CORE *Core)
{
    if(Proc->Features.PerfMon.AX.Version >= 2)
    {
	GLOBAL_PERF_COUNTER GlobalPerfCounter={.value=0};
	FIXED_PERF_COUNTER FixedPerfCounter={.value=0};
	GLOBAL_PERF_STATUS Overflow={.value=0};
	GLOBAL_PERF_OVF_CTRL OvfControl={.value=0};

	RDMSR(GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);
	Core->SaveArea.GlobalPerfCounter=GlobalPerfCounter;
	GlobalPerfCounter.EN_FIXED_CTR0=1;
	GlobalPerfCounter.EN_FIXED_CTR1=1;
	GlobalPerfCounter.EN_FIXED_CTR2=1;
	WRMSR(GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);


	RDMSR(FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);
	Core->SaveArea.FixedPerfCounter=FixedPerfCounter;
	FixedPerfCounter.EN0_OS=1;
	FixedPerfCounter.EN1_OS=1;
	FixedPerfCounter.EN2_OS=1;
	FixedPerfCounter.EN0_Usr=1;
	FixedPerfCounter.EN1_Usr=1;
	FixedPerfCounter.EN2_Usr=1;

	if(Proc->Features.PerfMon.AX.Version >= 3)
	{
		if(!Proc->Features.HTT_Enable)
		{
			FixedPerfCounter.AnyThread_EN0=1;
			FixedPerfCounter.AnyThread_EN1=1;
			FixedPerfCounter.AnyThread_EN2=1;
		}
		else	// Per Thread
		{
			FixedPerfCounter.AnyThread_EN0=0;
			FixedPerfCounter.AnyThread_EN1=0;
			FixedPerfCounter.AnyThread_EN2=0;
		}
	}
	WRMSR(FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);


	RDMSR(Overflow, MSR_CORE_PERF_GLOBAL_STATUS);
	if(Overflow.Overflow_CTR0)
		OvfControl.Clear_Ovf_CTR0=1;
	if(Overflow.Overflow_CTR1)
		OvfControl.Clear_Ovf_CTR1=1;
	if(Overflow.Overflow_CTR2)
		OvfControl.Clear_Ovf_CTR2=1;
	if(Overflow.Overflow_CTR0					\
	| Overflow.Overflow_CTR1					\
	| Overflow.Overflow_CTR2)
		WRMSR(OvfControl, MSR_CORE_PERF_GLOBAL_OVF_CTRL);
    }
}

void Counters_Clear(CORE *Core)
{
	WRMSR(	Core->SaveArea.FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);
	WRMSR(	Core->SaveArea.GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);
}

#define Counters_Genuine(Core, T)					\
({									\
	RDTSC_COUNTERx2(Core->Counter[T].TSC,				\
			MSR_IA32_APERF, Core->Counter[T].C0.UCC,	\
			MSR_IA32_MPERF, Core->Counter[T].C0.URC);	\
	/* Derive C1 */							\
	Core->Counter[T].C1=						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_Core2(Core, T)						\
({									\
    if(!Proc->Features.InvariantTSC)					\
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
	Core->Counter[T].C1=						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_Nehalem(Core, T)					\
({									\
	register unsigned long long Cx=0;				\
									\
	RDTSCP_COUNTERx5(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_FIXED_CTR1,Core->Counter[T].C0.UCC,\
			MSR_CORE_PERF_FIXED_CTR2,Core->Counter[T].C0.URC,\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1 */							\
	Cx=	Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1=						\
		(Core->Counter[T].TSC > Cx) ?				\
			Core->Counter[T].TSC - Cx			\
			: 0;						\
})

#define Counters_SandyBridge(Core, T)					\
({									\
	register unsigned long long Cx=0;				\
									\
	RDTSCP_COUNTERx6(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_FIXED_CTR1,Core->Counter[T].C0.UCC,\
			MSR_CORE_PERF_FIXED_CTR2,Core->Counter[T].C0.URC,\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_C7_RESIDENCY,Core->Counter[T].C7,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1 */							\
	Cx=	Core->Counter[T].C7					\
		+ Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1=						\
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
	Core->Delta.C0.UCC=						\
		(Core->Counter[0].C0.UCC >				\
		Core->Counter[1].C0.UCC) ?				\
			Core->Counter[0].C0.UCC				\
			- Core->Counter[1].C0.UCC			\
			: Core->Counter[1].C0.UCC			\
			- Core->Counter[0].C0.UCC;			\
									\
	Core->Delta.C0.URC= Core->Counter[1].C0.URC			\
			  - Core->Counter[0].C0.URC;			\
})

#define Delta_C1(Core)							\
({									\
	Core->Delta.C1=							\
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
	Core->Delta.INST= Core->Counter[1].INST				\
			- Core->Counter[0].INST;			\
})

#define Save_TSC(Core)							\
({	/* Save Time Stamp Counter. */					\
	Core->Counter[0].TSC=Core->Counter[1].TSC;			\
})

#define Save_C0(Core)							\
({	/* Save the Unhalted Core & Reference Counter */		\
	Core->Counter[0].C0.UCC=Core->Counter[1].C0.UCC;		\
	Core->Counter[0].C0.URC=Core->Counter[1].C0.URC;		\
})

#define Save_C1(Core)							\
({									\
	Core->Counter[0].C1=Core->Counter[1].C1;			\
})

#define Save_C3(Core)							\
({									\
	Core->Counter[0].C3=Core->Counter[1].C3;			\
})

#define Save_C6(Core)							\
({									\
	Core->Counter[0].C6=Core->Counter[1].C6;			\
})

#define Save_C7(Core)							\
({									\
	Core->Counter[0].C7=Core->Counter[1].C7;			\
})

#define Save_INST(Core)							\
({	/* Save the Instructions counter. */				\
	Core->Counter[0].INST=Core->Counter[1].INST;			\
})

void Core_Temp(CORE *Core)
{
	THERM_STATUS ThermStatus={.value=0};
	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);

	Core->Thermal.Sensor=ThermStatus.DTS;
	Core->Thermal.Trip=ThermStatus.StatusBit | ThermStatus.StatusLog;
}


static enum hrtimer_restart Cycle_GenuineIntel(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if(KPrivate->Join[cpu]->tsm.mustFwd == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Genuine(Core, 1);
		Core_Temp(Core);

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(Core->Sync.V, 0);

		return(HRTIMER_RESTART);
	}
	else
		return(HRTIMER_NORESTART);
}

void InitTimer_GenuineIntel(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_GenuineIntel, 1);
}

void Start_GenuineIntel(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_Intel_Query(Core);

	Counters_Genuine(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd=1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started=1;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Start Timer\n", cpu);
}

void Stop_GenuineIntel(void *arg)
{
	unsigned int cpu=smp_processor_id();

	KPrivate->Join[cpu]->tsm.mustFwd=0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	KPrivate->Join[cpu]->tsm.started=0;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Stop Timer\n", cpu);
}

static enum hrtimer_restart Cycle_AuthenticAMD(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if(KPrivate->Join[cpu]->tsm.mustFwd == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		if(Proc->Features.Power.CX.HCF_Cap == 1) // MPERF & APERF ?
			Counters_Genuine(Core, 1);

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(Core->Sync.V, 0);

		return(HRTIMER_RESTART);
	}
	else
		return(HRTIMER_NORESTART);
}

void InitTimer_AuthenticAMD(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AuthenticAMD, 1);
}

void Start_AuthenticAMD(void *arg)
{
	unsigned int cpu=smp_processor_id();
//!	CORE *Core=(CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd=1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started=1;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Start Timer\n", cpu);
}

void Stop_AuthenticAMD(void *arg)
{
	unsigned int cpu=smp_processor_id();

	KPrivate->Join[cpu]->tsm.mustFwd=0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	KPrivate->Join[cpu]->tsm.started=0;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Stop Timer\n", cpu);
}

static enum hrtimer_restart Cycle_Core2(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if(KPrivate->Join[cpu]->tsm.mustFwd == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Core2(Core, 1);
		Core_Temp(Core);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_TSC(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(Core->Sync.V, 0);

		return(HRTIMER_RESTART);
	}
	else
		return(HRTIMER_NORESTART);
}

void InitTimer_Core2(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Core2, 1);
}

void Start_Core2(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_Core2_Query(Core);

	Counters_Set(Core);
	Counters_Core2(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd=1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started=1;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Start Timer\n", cpu);
}

void Stop_Core2(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd=0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Counters_Clear(Core);

	KPrivate->Join[cpu]->tsm.started=0;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Stop Timer\n", cpu);
}

static enum hrtimer_restart Cycle_Nehalem(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if(KPrivate->Join[cpu]->tsm.mustFwd == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Nehalem(Core, 1);
		Core_Temp(Core);

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

		BITSET(Core->Sync.V, 0);

		return(HRTIMER_RESTART);
	}
	else
		return(HRTIMER_NORESTART);
}

void InitTimer_Nehalem(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Nehalem, 1);
}

void Start_Nehalem(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_Nehalem_Query(Core);

	Counters_Set(Core);
	Counters_Nehalem(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd=1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started=1;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Start Timer\n", cpu);
}

void Stop_Nehalem(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd=0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Counters_Clear(Core);

	KPrivate->Join[cpu]->tsm.started=0;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Stop Timer\n", cpu);
}


static enum hrtimer_restart Cycle_SandyBridge(struct hrtimer *pTimer)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	if(KPrivate->Join[cpu]->tsm.mustFwd == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_SandyBridge(Core, 1);
		Core_Temp(Core);

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

		BITSET(Core->Sync.V, 0);

		return(HRTIMER_RESTART);
	}
	else
		return(HRTIMER_NORESTART);
}

void InitTimer_SandyBridge(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_SandyBridge, 1);
}

void Start_SandyBridge(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core);

	Counters_Set(Core);
	Counters_SandyBridge(Core, 0);

	KPrivate->Join[cpu]->tsm.mustFwd=1;

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	KPrivate->Join[cpu]->tsm.started=1;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Start Timer\n", cpu);
}

void Stop_SandyBridge(void *arg)
{
	unsigned int cpu=smp_processor_id();
	CORE *Core=(CORE *) KPublic->Core[cpu];

	KPrivate->Join[cpu]->tsm.mustFwd=0;

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Counters_Clear(Core);

	KPrivate->Join[cpu]->tsm.started=0;

	printk(	KERN_DEBUG "CoreFreq: CPU #%-3d Stop Timer\n", cpu);
}

void IdleDriver_Query(void *arg)
{
	struct cpuidle_driver *idleDriver;

	if((idleDriver=cpuidle_get_driver()) != NULL)
	{
		int i=0;

		strncpy(Proc->IdleDriver.Name,
			idleDriver->name,
			CPUIDLE_NAME_LEN - 1);

		if(idleDriver->state_count < CPUIDLE_STATE_MAX)
			Proc->IdleDriver.stateCount=idleDriver->state_count;
		else	// No overflow check.
			Proc->IdleDriver.stateCount=CPUIDLE_STATE_MAX;

		for(i=0; i < Proc->IdleDriver.stateCount; i++)
		{
			strncpy(Proc->IdleDriver.State[i].Name,
				idleDriver->states[i].name,
				CPUIDLE_NAME_LEN - 1);

			Proc->IdleDriver.State[i].exitLatency=
				idleDriver->states[i].exit_latency;
			Proc->IdleDriver.State[i].powerUsage=
				idleDriver->states[i].power_usage;
			Proc->IdleDriver.State[i].targetResidency=
				idleDriver->states[i].target_residency;
		}
	}
	else
		memset(&Proc->IdleDriver, 0, sizeof(IDLEDRIVER));
}

static int CoreFreqK_mmap(struct file *pfile, struct vm_area_struct *vma)
{
	if(vma->vm_pgoff == 0)
	{
	    if((Proc != NULL)
	    && remap_pfn_range(	vma,
				vma->vm_start,
				virt_to_phys((void *) Proc) >> PAGE_SHIFT,
				vma->vm_end - vma->vm_start,
				vma->vm_page_prot) < 0)
		return(-EIO);
	}
	else
	{
	    unsigned int cpu=vma->vm_pgoff - 1;
	    if((cpu < Proc->CPU.Count) && (cpu >= 0)
	    && (KPublic->Core[cpu] != NULL)
	    && remap_pfn_range(vma,
			vma->vm_start,
			virt_to_phys((void *) KPublic->Core[cpu]) >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot) < 0)
		return(-EIO);
	}
	return(0);
}

static DEFINE_MUTEX(CoreFreqK_mutex);

static int CoreFreqK_open(struct inode *inode, struct file *pfile)
{
	if(!mutex_trylock(&CoreFreqK_mutex))
		return(-EBUSY);
	else
		return(0);
}

static int CoreFreqK_release(struct inode *inode, struct file *pfile)
{
	mutex_unlock(&CoreFreqK_mutex);
	return(0);
}

static struct file_operations CoreFreqK_fops=
{
	.open	= CoreFreqK_open,
	.release= CoreFreqK_release,
	.mmap	= CoreFreqK_mmap,
	.owner  = THIS_MODULE,
};

#ifdef CONFIG_PM_SLEEP
static int CoreFreqK_suspend(struct device *dev)
{
	Controller_Stop();

	printk(KERN_DEBUG "CoreFreq: Suspend\n");
	return(0);
}

static int CoreFreqK_resume(struct device *dev)
{
	Controller_Start();

	printk(KERN_DEBUG "CoreFreq: Resume\n");
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
	    switch(action)
	    {
		case CPU_ONLINE:
		case CPU_DOWN_FAILED:
//-		case CPU_ONLINE_FROZEN:
		if((cpu >=0) && (cpu < Proc->CPU.Count))
		{
		    if((KPublic->Core[cpu]->T.ApicID == -1)
		    && !KPublic->Core[cpu]->OffLine.HW)
		    {
			if(!Core_Topology(cpu))
			{
				if(KPublic->Core[cpu]->T.ApicID >= 0)
					KPublic->Core[cpu]->OffLine.HW=0;
				else
					KPublic->Core[cpu]->OffLine.HW=1;
			}
			memcpy(	&KPublic->Core[cpu]->Clock,
				&KPublic->Core[0]->Clock,
				sizeof(CLOCK) );
		    }
		    if(Arch[Proc->ArchID].Timer != NULL)
			Arch[Proc->ArchID].Timer(cpu);

		    if(	(KPrivate->Join[cpu]->tsm.started == 0)
		    &&	(Arch[Proc->ArchID].Start != NULL) )
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, 0);

		    Proc->CPU.OnLine++;
		    KPublic->Core[cpu]->OffLine.OS=0;
		}
		break;
		case CPU_DOWN_PREPARE:
//-		case CPU_DOWN_PREPARE_FROZEN:
		if((cpu >=0) && (cpu < Proc->CPU.Count))
		{
		    if(	(KPrivate->Join[cpu]->tsm.created == 1)
		    &&	(KPrivate->Join[cpu]->tsm.started == 1)
		    &&	(Arch[Proc->ArchID].Stop != NULL) )
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, 1);

		    Proc->CPU.OnLine--;
		    KPublic->Core[cpu]->OffLine.OS=1;
		}
		break;
		default:
		break;
	    }
	return(NOTIFY_OK);
}

static struct notifier_block CoreFreqK_notifier_block=
{
	.notifier_call=CoreFreqK_hotplug,
};

static int __init CoreFreqK_init(void)
{
	int rc=0;
	ARG Arg={.count=0};
	// Query features on the presumed BSP processor.
	memset(&Arg.features, 0, sizeof(FEATURES));
	rc=smp_call_function_single(0, Query_Features, &Arg, 1);
	rc = (rc == 0) ? ((Arg.count > 0) ? 0 : -ENXIO) : rc;
	if(rc == 0)
	{
	  CoreFreqK.kcdev=cdev_alloc();
	  CoreFreqK.kcdev->ops=&CoreFreqK_fops;
	  CoreFreqK.kcdev->owner=THIS_MODULE;

	  if(alloc_chrdev_region(&CoreFreqK.nmdev, 0, 1, DRV_FILENAME) >= 0)
	  {
	    CoreFreqK.Major=MAJOR(CoreFreqK.nmdev);
	    CoreFreqK.mkdev=MKDEV(CoreFreqK.Major, 0);

	    if(cdev_add(CoreFreqK.kcdev, CoreFreqK.mkdev, 1) >= 0)
	    {
		struct device *tmpDev;

		CoreFreqK.clsdev=class_create(THIS_MODULE, DRV_DEVNAME);
		CoreFreqK.clsdev->pm=COREFREQ_PM_OPS;

		if((tmpDev=device_create(CoreFreqK.clsdev, NULL,
					 CoreFreqK.mkdev, NULL,
					 DRV_DEVNAME)) != NULL)
		{
		    unsigned int cpu=0;
		    unsigned long publicSize=0, privateSize=0, packageSize=0;

		    publicSize=sizeof(KPUBLIC) + sizeof(CORE *) * Arg.count;

		    privateSize=sizeof(KPRIVATE) + sizeof(JOIN *) * Arg.count;

		    if(((KPublic=kmalloc(publicSize, GFP_KERNEL)) != NULL)
		    && ((KPrivate=kmalloc(privateSize, GFP_KERNEL)) != NULL))
		    {
			memset(KPublic, 0, publicSize);
			memset(KPrivate, 0, privateSize);

			packageSize=ROUND_TO_PAGES(sizeof(PROC));
			if((Proc=kmalloc(packageSize, GFP_KERNEL)) != NULL)
			{
			    memset(Proc, 0, packageSize);
			    Proc->CPU.Count=Arg.count;

			    if((SleepInterval >= LOOP_MIN_MS)
			    && (SleepInterval <= LOOP_MAX_MS))
				Proc->SleepInterval=SleepInterval;
			    else
				Proc->SleepInterval=LOOP_DEF_MS;

			    memcpy(	&Proc->Features,
					&Arg.features,
					sizeof(FEATURES) );

			    Arch[0].Architecture=Proc->Features.Info.VendorID;

			    RearmTheTimer=
				ktime_set(0, Proc->SleepInterval * 1000000L);

			    publicSize=ROUND_TO_PAGES(sizeof(CORE));
			    privateSize=ROUND_TO_PAGES(sizeof(JOIN));
			    if(((KPublic->Cache=kmem_cache_create(
					"corefreqk-pub",
					publicSize, 0,
					SLAB_HWCACHE_ALIGN, NULL)) != NULL)
			    && ((KPrivate->Cache=kmem_cache_create(
					"corefreqk-priv",
					privateSize, 0,
					SLAB_HWCACHE_ALIGN, NULL)) != NULL))
			    {
				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				{
				    void *kcache=kmem_cache_alloc(
						KPublic->Cache, GFP_KERNEL);
				    memset(kcache, 0, publicSize);
				    KPublic->Core[cpu]=kcache;

				    kcache=kmem_cache_alloc(
						KPrivate->Cache, GFP_KERNEL);
				    memset(kcache, 0, privateSize);
				    KPrivate->Join[cpu]=kcache;

				    BITCLR(KPublic->Core[cpu]->Sync.V, 0);

				    KPublic->Core[cpu]->Bind=cpu;
				}
				if(!strncmp(Arch[0].Architecture,
						VENDOR_INTEL, 12))
				{
					Arch[0].Query=Query_GenuineIntel;
					Arch[0].Start=Start_GenuineIntel;
					Arch[0].Stop=Stop_GenuineIntel;
					Arch[0].Timer=InitTimer_GenuineIntel;
					Arch[0].Clock=Clock_GenuineIntel;
				}
				else if(!strncmp(Arch[0].Architecture,
						VENDOR_AMD, 12))
				{
					Arch[0].Query=Query_AuthenticAMD;
					Arch[0].Start=Start_AuthenticAMD;
					Arch[0].Stop=Stop_AuthenticAMD;
					Arch[0].Timer=InitTimer_AuthenticAMD;
					Arch[0].Clock=Clock_AuthenticAMD;
				}
				if((ArchID != -1)
				&& (ArchID >= 0)
				&& (ArchID < ARCHITECTURES))
					Proc->ArchID=ArchID;
				else
				{
				  for(	Proc->ArchID=ARCHITECTURES - 1;
					Proc->ArchID > 0;
					Proc->ArchID--)
				  // Search for an architecture signature.
				    if(!(Arch[Proc->ArchID].Signature.ExtFamily
				    ^ Proc->Features.Std.AX.ExtFamily)
				    && !(Arch[Proc->ArchID].Signature.Family
				    ^ Proc->Features.Std.AX.Family)
				    && !(Arch[Proc->ArchID].Signature.ExtModel
				    ^ Proc->Features.Std.AX.ExtModel)
				    && !(Arch[Proc->ArchID].Signature.Model
				    ^ Proc->Features.Std.AX.Model))
					break;
				}

				strncpy(Proc->Architecture,
					Arch[Proc->ArchID].Architecture, 32);

				if(IdleDriverQuery)
				    smp_call_function_single(0, /* BSP */
							IdleDriver_Query,
							NULL,
							0);
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

			    register_hotcpu_notifier(&CoreFreqK_notifier_block);
			    }
			    else
			    {
				if(KPublic->Cache != NULL)
					kmem_cache_destroy(KPublic->Cache);
				if(KPrivate->Cache != NULL)
					kmem_cache_destroy(KPrivate->Cache);

				kfree(Proc);
				kfree(KPublic);
				kfree(KPrivate);

				device_destroy(CoreFreqK.clsdev,
					       CoreFreqK.mkdev);
				class_destroy(CoreFreqK.clsdev);
				cdev_del(CoreFreqK.kcdev);
				unregister_chrdev_region(CoreFreqK.mkdev, 1);

				rc=-ENOMEM;
			    }
			}
			else
			{
			    kfree(KPublic);
			    kfree(KPrivate);

			    device_destroy(CoreFreqK.clsdev,
					   CoreFreqK.mkdev);
			    class_destroy(CoreFreqK.clsdev);
			    cdev_del(CoreFreqK.kcdev);
			    unregister_chrdev_region(CoreFreqK.mkdev, 1);

			    rc=-ENOMEM;
			}
		    }
		    else
		    {
			if(KPublic != NULL)
				kfree(KPublic);
			if(KPrivate != NULL)
				kfree(KPrivate);

			device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
			class_destroy(CoreFreqK.clsdev);
			cdev_del(CoreFreqK.kcdev);
			unregister_chrdev_region(CoreFreqK.mkdev, 1);

			rc=-ENOMEM;
		    }
		}
		else
		{
		    class_destroy(CoreFreqK.clsdev);
		    cdev_del(CoreFreqK.kcdev);
		    unregister_chrdev_region(CoreFreqK.mkdev, 1);

		    rc=-EBUSY;
		}
	    }
	    else
	    {
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		rc=-EBUSY;
	    }
	  }
	  else
	  {
	    cdev_del(CoreFreqK.kcdev);

	    rc=-EBUSY;
	  }
	}
	return(rc);
}

static void __exit CoreFreqK_cleanup(void)
{
	unsigned int cpu=0;

	unregister_hotcpu_notifier(&CoreFreqK_notifier_block);

	Controller_Stop();
	Controller_Exit();

	for(cpu=0; (KPublic->Cache != NULL) && (cpu < Proc->CPU.Count); cpu++)
	{
		if(KPublic->Core[cpu] != NULL)
			kmem_cache_free(KPublic->Cache,	KPublic->Core[cpu]);
		if(KPrivate->Join[cpu] != NULL)
			kmem_cache_free(KPrivate->Cache, KPrivate->Join[cpu]);
	}
	if(KPublic->Cache != NULL)
		kmem_cache_destroy(KPublic->Cache);
	if(KPrivate->Cache != NULL)
		kmem_cache_destroy(KPrivate->Cache);
	if(Proc != NULL)
		kfree(Proc);
	if(KPublic != NULL)
		kfree(KPublic);
	if(KPrivate != NULL)
		kfree(KPrivate);

	device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
	class_destroy(CoreFreqK.clsdev);
	cdev_del(CoreFreqK.kcdev);
	unregister_chrdev_region(CoreFreqK.mkdev, 1);

	printk(KERN_NOTICE "CoreFreq: Unload\n");
}

module_init(CoreFreqK_init);
module_exit(CoreFreqK_cleanup);
