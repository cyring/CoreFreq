/*
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <asm/msr.h>
#include <stdatomic.h>

#include "intelfreq.h"

MODULE_AUTHOR ("CyrIng");
MODULE_DESCRIPTION ("IntelFreq");
MODULE_SUPPORTED_DEVICE ("all");
MODULE_LICENSE ("GPL");

static struct
{
	signed int Major;
	struct cdev *kcdev;
	dev_t nmdev, mkdev;
	struct class *clsdev;
} IntelFreq;

static PROC *Proc=NULL;

static ARCH Arch[ARCHITECTURES]=
{
	{ _GenuineIntel,       Arch_Genuine,     "Genuine" },
	{ _Core_Yonah,         Arch_Genuine,     "Core/Yonah" },
	{ _Core_Conroe,        Arch_Core2,       "Core2/Conroe" },
	{ _Core_Kentsfield,    Arch_Core2,       "Core2/Kentsfield" },
	{ _Core_Yorkfield,     Arch_Core2,       "Core2/Yorkfield" },
	{ _Core_Dunnington,    Arch_Core2,       "Xeon/Dunnington" },
	{ _Atom_Bonnell,       Arch_Core2,       "Atom/Bonnell" },
	{ _Atom_Silvermont,    Arch_Core2,       "Atom/Silvermont" },
	{ _Atom_Lincroft,      Arch_Core2,       "Atom/Lincroft" },
	{ _Atom_Clovertrail,   Arch_Core2,       "Atom/Clovertrail" },
	{ _Atom_Saltwell,      Arch_Core2,       "Atom/Saltwell" },
	{ _Silvermont_637,     Arch_Nehalem,     "Silvermont" },
	{ _Silvermont_64D,     Arch_Nehalem,     "Silvermont" },
	{ _Nehalem_Bloomfield, Arch_Nehalem,     "Nehalem/Bloomfield" },
	{ _Nehalem_Lynnfield,  Arch_Nehalem,     "Nehalem/Lynnfield" },
	{ _Nehalem_MB,         Arch_Nehalem,     "Nehalem/Mobile" },
	{ _Nehalem_EX,         Arch_Nehalem,     "Nehalem/eXtreme.EP" },
	{ _Westmere,           Arch_Nehalem,     "Westmere" },
	{ _Westmere_EP,        Arch_Nehalem,     "Westmere/EP" },
	{ _Westmere_EX,        Arch_Nehalem,     "Westmere/eXtreme" },
	{ _SandyBridge,        Arch_SandyBridge, "SandyBridge" },
	{ _SandyBridge_EP,     Arch_SandyBridge, "SandyBridge/eXtreme.EP" },
	{ _IvyBridge,          Arch_SandyBridge, "IvyBridge" },
	{ _IvyBridge_EP,       Arch_SandyBridge, "IvyBridge/EP" },
	{ _Haswell_DT,         Arch_SandyBridge, "Haswell/Desktop" },
	{ _Haswell_MB,         Arch_SandyBridge, "Haswell/Mobile" },
	{ _Haswell_ULT,        Arch_SandyBridge, "Haswell/Ultra Low TDP" },
	{ _Haswell_ULX,        Arch_SandyBridge, "Haswell/Ultra Low eXtreme" },
};


unsigned int Core_Count(void)
{
	unsigned int count=0;

	__asm__ volatile
	(
		"movq	$0x4, %%rax;"
		"xorq	%%rcx, %%rcx;"
		"cpuid;"
		"shr	$26, %%rax;"
		"and	$0x3f, %%rax;"
		"add	$1, %%rax;"
		: "=a"	(count)
	);
	return((!count) ? 1 : count);
}

void Proc_Brand(char *pBrand)
{
	char tmpString[48+1]={0x20};
	unsigned int ix=0, jx=0, px=0;
	BRAND Brand;

	for(ix=0; ix<3; ix++)
	{
		__asm__ volatile
		(
			"cpuid ;"
			: "=a"  (Brand.AX),
			  "=b"  (Brand.BX),
			  "=c"  (Brand.CX),
			  "=d"  (Brand.DX)
			: "a"   (0x80000002 + ix)
		);
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.AX.Chr[jx];
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.BX.Chr[jx];
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.CX.Chr[jx];
		for(jx=0; jx<4; jx++, px++)
			tmpString[px]=Brand.DX.Chr[jx];
	}
	for(ix=jx=0; jx < px; jx++)
		if(!(tmpString[jx] == 0x20 && tmpString[jx+1] == 0x20))
			pBrand[ix++]=tmpString[jx];
}

// Retreive the Processor features through calls to the CPUID instruction.
void	Proc_Features(FEATURES *features)
{
	int BX=0, DX=0, CX=0;
	__asm__ volatile
	(
		"cpuid"
		: "=b"	(BX),
		  "=d"	(DX),
		  "=c"	(CX)
                : "a" (0x0)
	);
	features->VendorID[ 0]=BX;
	features->VendorID[ 1]=(BX >> 8);
	features->VendorID[ 2]=(BX >> 16);
	features->VendorID[ 3]=(BX >> 24);
	features->VendorID[ 4]=DX;
	features->VendorID[ 5]=(DX >> 8);
	features->VendorID[ 6]=(DX >> 16);
	features->VendorID[ 7]=(DX >> 24);
	features->VendorID[ 8]=CX;
	features->VendorID[ 9]=(CX >> 8);
	features->VendorID[10]=(CX >> 16);
	features->VendorID[11]=(CX >> 24);

	__asm__ volatile
	(
		"cpuid"
		: "=a"	(features->Std.AX),
		  "=b"	(features->Std.BX),
		  "=c"	(features->Std.CX),
		  "=d"	(features->Std.DX)
                : "a" (0x1)
	);
	__asm__ volatile
	(
		"cpuid"
		: "=a"	(features->MONITOR_MWAIT_Leaf.AX),
		  "=b"	(features->MONITOR_MWAIT_Leaf.BX),
		  "=c"	(features->MONITOR_MWAIT_Leaf.CX),
		  "=d"	(features->MONITOR_MWAIT_Leaf.DX)
                : "a" (0x5)
	);
	__asm__ volatile
	(
		"cpuid"
		: "=a"	(features->Thermal_Power_Leaf.AX),
		  "=b"	(features->Thermal_Power_Leaf.BX),
		  "=c"	(features->Thermal_Power_Leaf.CX),
		  "=d"	(features->Thermal_Power_Leaf.DX)
                : "a" (0x6)
	);
	__asm__ volatile
	(
		"xorq	%%rbx, %%rbx    \n\t"
		"xorq	%%rcx, %%rcx    \n\t"
		"xorq	%%rdx, %%rdx    \n\t"
		"cpuid"
		: "=a"	(features->ExtFeature.AX),
		  "=b"	(features->ExtFeature.BX),
		  "=c"	(features->ExtFeature.CX),
		  "=d"	(features->ExtFeature.DX)
                : "a" (0x7)
	);
	__asm__ volatile
	(
		"cpuid"
		: "=a"	(features->Perf_Monitoring_Leaf.AX),
		  "=b"	(features->Perf_Monitoring_Leaf.BX),
		  "=c"	(features->Perf_Monitoring_Leaf.CX),
		  "=d"	(features->Perf_Monitoring_Leaf.DX)
                : "a" (0xa)
	);
	__asm__ volatile
	(
		"cpuid"
		: "=a"	(features->LargestExtFunc)
                : "a" (0x80000000)
	);
	if(features->LargestExtFunc >= 0x80000004 \
	&& features->LargestExtFunc <= 0x80000008)
	{
		__asm__ volatile
		(
			"cpuid                   \n\t"
			"and	$0x100, %%rdx    \n\t"
			"shr	$8, %%rdx"
			: "=d"	(features->InvariantTSC)
			: "a" (0x80000007)
		);
		__asm__ volatile
		(
			"cpuid"
			: "=c"	(features->ExtFunc.CX),
			  "=d"	(features->ExtFunc.DX)
			: "a" (0x80000001)
		);
	}
	Proc_Brand(features->Brand);
}

CLOCK Proc_Clock(unsigned int ratio)
{
	unsigned long long TSC[2];
	unsigned int Lo, Hi;
	CLOCK clock;

	__asm__ volatile
	(
		"rdtsc"
		:"=a" (Lo),
		 "=d" (Hi)
	);
	TSC[0]=((unsigned long long) Lo)		\
		| (((unsigned long long) Hi) << 32);

	ssleep(1);

	__asm__ volatile
	(
		"rdtsc"
		:"=a" (Lo),
		 "=d" (Hi)
	);
	TSC[1]=((unsigned long long) Lo) | (((unsigned long long) Hi) << 32);
	TSC[1]-=TSC[0];

	clock.Q=TSC[1] / (ratio * 1000000L);
	clock.R=TSC[1] - (clock.Q * ratio);

	return(clock);
}

// Enumerate the Processor's Cores and Threads topology.
signed int Read_APIC(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;

		signed int	InputLevel=0, NoMoreLevels=0,
			SMT_Mask_Width=0, SMT_Select_Mask=0,
			CorePlus_Mask_Width=0, CoreOnly_Select_Mask=0;

		CPUID_TOPOLOGY_LEAF ExtTopology=
		{
			.AX.Register=0,
			.BX.Register=0,
			.CX.Register=0,
			.DX.Register=0
		};
		do
		{
			__asm__ volatile
			(
				"cpuid"
				: "=a"	(ExtTopology.AX),
				  "=b"	(ExtTopology.BX),
				  "=c"	(ExtTopology.CX),
				  "=d"	(ExtTopology.DX)
				:  "a"	(0xb),
				   "c"	(InputLevel)
			);
			// Exit from the loop if the BX register equals 0;
			// or if the requested level exceeds the level of a Core.
			if(!ExtTopology.BX.Register || (InputLevel > LEVEL_CORE))
				NoMoreLevels=1;
			else
			{
			    switch(ExtTopology.CX.Type)
			    {
			    case LEVEL_THREAD:
				{
				SMT_Mask_Width = ExtTopology.AX.SHRbits;
				SMT_Select_Mask= ~((-1) << SMT_Mask_Width );
				Core->T.ThreadID=ExtTopology.DX.x2ApicID\
						& SMT_Select_Mask;

				if((Core->T.ThreadID > 0)
				&& !Proc->Features.HTT_enabled)
					Proc->Features.HTT_enabled=1;
				}
				break;
			    case LEVEL_CORE:
				{
				CorePlus_Mask_Width=ExtTopology.AX.SHRbits;
				CoreOnly_Select_Mask=				\
					(~((-1) << CorePlus_Mask_Width ) )	\
					^ SMT_Select_Mask;
				Core->T.CoreID=					\
				(ExtTopology.DX.x2ApicID & CoreOnly_Select_Mask)\
					>> SMT_Mask_Width;
				}
				break;
			    }
			}
			InputLevel++;
		}
		while(!NoMoreLevels);

		Core->T.ApicID=ExtTopology.DX.x2ApicID;
	}
	while(!kthread_should_stop())
		msleep(50);
	do_exit(0);
}

unsigned int Proc_Topology(void)
{
	unsigned int cpu=0, CountEnabledCPU=0;

	for(cpu=0; cpu < Proc->CPU.Count; cpu++)
	{
		if(!Proc->Core[cpu].OffLine)
		{
			Proc->Core[cpu].TID[APIC_TID]= \
				kthread_create(	Read_APIC,
						&Proc->Core[cpu],
						"kintelapic-%03d",
						Proc->Core[cpu].Bind);

			if(!IS_ERR(Proc->Core[cpu].TID[APIC_TID]))
			{
				kthread_bind(Proc->Core[cpu].TID[APIC_TID], cpu);
				wake_up_process(Proc->Core[cpu].TID[APIC_TID]);
			}
			CountEnabledCPU++;
		}
		else
		{
			Proc->Core[cpu].T.ApicID=-1;
			Proc->Core[cpu].T.CoreID=-1;
			Proc->Core[cpu].T.ThreadID=-1;
		}
	}
	for(cpu=0; cpu < Proc->CPU.Count; cpu++)
		if(!Proc->Core[cpu].OffLine
		&& !IS_ERR(Proc->Core[cpu].TID[APIC_TID]))
		{
			kthread_stop(Proc->Core[cpu].TID[APIC_TID]);
		}
	return(CountEnabledCPU);
}


void Counters_Set(unsigned int cpu)
{
	GLOBAL_PERF_COUNTER GlobalPerfCounter={0};
	FIXED_PERF_COUNTER FixedPerfCounter={0};
	GLOBAL_PERF_STATUS Overflow={0};
	GLOBAL_PERF_OVF_CTRL OvfControl={0};

	RDMSR(GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);

	Proc->Core[cpu].SaveArea.GlobalPerfCounter=	\
					GlobalPerfCounter;
	GlobalPerfCounter.EN_FIXED_CTR0=1;
	GlobalPerfCounter.EN_FIXED_CTR1=1;
	GlobalPerfCounter.EN_FIXED_CTR2=1;

	WRMSR(GlobalPerfCounter, MSR_CORE_PERF_GLOBAL_CTRL);

	RDMSR(FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);

	Proc->Core[cpu].SaveArea.FixedPerfCounter=	\
					FixedPerfCounter;
	FixedPerfCounter.EN0_OS=1;
	FixedPerfCounter.EN1_OS=1;
	FixedPerfCounter.EN2_OS=1;
	FixedPerfCounter.EN0_Usr=1;
	FixedPerfCounter.EN1_Usr=1;
	FixedPerfCounter.EN2_Usr=1;
	if(Proc->PerCore)
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
	WRMSR(FixedPerfCounter, MSR_CORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Overflow, MSR_CORE_PERF_GLOBAL_STATUS);
	if(Overflow.Overflow_CTR0)
		OvfControl.Clear_Ovf_CTR0=1;
	if(Overflow.Overflow_CTR1)
		OvfControl.Clear_Ovf_CTR1=1;
	if(Overflow.Overflow_CTR2)
		OvfControl.Clear_Ovf_CTR2=1;
	if(Overflow.Overflow_CTR0		\
		| Overflow.Overflow_CTR1	\
		| Overflow.Overflow_CTR2)
			WRMSR(	OvfControl,
				MSR_CORE_PERF_GLOBAL_OVF_CTRL);
}

void Counters_Clear(unsigned int cpu)
{
	WRMSR(	Proc->Core[cpu].SaveArea.FixedPerfCounter,
		MSR_CORE_PERF_FIXED_CTR_CTRL);

	WRMSR(	Proc->Core[cpu].SaveArea.GlobalPerfCounter,
		MSR_CORE_PERF_GLOBAL_CTRL);
}

void Counters_Genuine(unsigned int cpu, unsigned int T)
{
	// Actual & Maximum Performance Frequency Clock counters.
	RDCNT(Proc->Core[cpu].Counter[T].C0.UCC, MSR_IA32_APERF);
	RDCNT(Proc->Core[cpu].Counter[T].C0.URC, MSR_IA32_MPERF);

	// TSC in relation to the Core.
	RDCNT(Proc->Core[cpu].Counter[T].TSC, MSR_IA32_TSC);

	// Derive C1
	Proc->Core[cpu].Counter[T].C1=				\
	  (Proc->Core[cpu].Counter[T].TSC > Proc->Core[cpu].Counter[T].C0.URC)? \
	    Proc->Core[cpu].Counter[T].TSC - Proc->Core[cpu].Counter[T].C0.URC	\
	  : 0;
}

void Counters_Core2(unsigned int cpu, unsigned int T)
{
	// Instructions Retired
	RDCNT(Proc->Core[cpu].Counter[T].INST, MSR_CORE_PERF_FIXED_CTR0);

	// Unhalted Core & Reference Cycles.
	RDCNT(Proc->Core[cpu].Counter[T].C0.UCC, MSR_CORE_PERF_FIXED_CTR1);
	RDCNT(Proc->Core[cpu].Counter[T].C0.URC, MSR_CORE_PERF_FIXED_CTR2);

	// TSC in relation to the Logical Core.
	RDCNT(Proc->Core[cpu].Counter[T].TSC, MSR_IA32_TSC);

	// Derive C1
	Proc->Core[cpu].Counter[T].C1=				\
	  (Proc->Core[cpu].Counter[T].TSC > Proc->Core[cpu].Counter[T].C0.URC)? \
	    Proc->Core[cpu].Counter[T].TSC - Proc->Core[cpu].Counter[T].C0.URC	\
	  : 0;
}

void Counters_Nehalem(unsigned int cpu, unsigned int T)
{
	register unsigned long long Cx=0;

	// Instructions Retired
	RDCNT(Proc->Core[cpu].Counter[T].INST, MSR_CORE_PERF_FIXED_CTR0);

	// Unhalted Core & Reference Cycles.
	RDCNT(Proc->Core[cpu].Counter[T].C0.UCC, MSR_CORE_PERF_FIXED_CTR1);
	RDCNT(Proc->Core[cpu].Counter[T].C0.URC, MSR_CORE_PERF_FIXED_CTR2);

	// TSC in relation to the Logical Core.
	RDCNT(Proc->Core[cpu].Counter[T].TSC, MSR_IA32_TSC);

	// C-States.
	RDCNT(Proc->Core[cpu].Counter[T].C3, MSR_CORE_C3_RESIDENCY);
	RDCNT(Proc->Core[cpu].Counter[T].C6, MSR_CORE_C6_RESIDENCY);

	// Derive C1
	Cx=	Proc->Core[cpu].Counter[T].C6		\
		+ Proc->Core[cpu].Counter[T].C3		\
		+ Proc->Core[cpu].Counter[T].C0.URC;

	Proc->Core[cpu].Counter[T].C1=				\
		(Proc->Core[cpu].Counter[T].TSC > Cx) ?		\
			Proc->Core[cpu].Counter[T].TSC - Cx	\
			: 0;
}

void Counters_SandyBridge(unsigned int cpu, unsigned int T)
{
	register unsigned long long Cx=0;

	// Instructions Retired
	RDCNT(Proc->Core[cpu].Counter[T].INST, MSR_CORE_PERF_FIXED_CTR0);

	// Unhalted Core & Reference Cycles.
	RDCNT(Proc->Core[cpu].Counter[T].C0.UCC, MSR_CORE_PERF_FIXED_CTR1);
	RDCNT(Proc->Core[cpu].Counter[T].C0.URC, MSR_CORE_PERF_FIXED_CTR2);

	// TSC in relation to the Logical Core.
	RDCNT(Proc->Core[cpu].Counter[T].TSC, MSR_IA32_TSC);

	// C-States.
	RDCNT(Proc->Core[cpu].Counter[T].C3, MSR_CORE_C3_RESIDENCY);
	RDCNT(Proc->Core[cpu].Counter[T].C6, MSR_CORE_C6_RESIDENCY);
	RDCNT(Proc->Core[cpu].Counter[T].C7, MSR_CORE_C7_RESIDENCY);

	// Derive C1
	Cx=	Proc->Core[cpu].Counter[T].C7		\
		+ Proc->Core[cpu].Counter[T].C6		\
		+ Proc->Core[cpu].Counter[T].C3		\
		+ Proc->Core[cpu].Counter[T].C0.URC;

	Proc->Core[cpu].Counter[T].C1=				\
		(Proc->Core[cpu].Counter[T].TSC > Cx) ?		\
			Proc->Core[cpu].Counter[T].TSC - Cx	\
			: 0;
}

void Core_Temp(unsigned int cpu)
{
	RDMSR(Proc->Core[cpu].TjMax, MSR_IA32_TEMPERATURE_TARGET);
	RDMSR(Proc->Core[cpu].ThermStat, MSR_IA32_THERM_STATUS);
}

signed int Cycle_Genuine(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;
		unsigned int leave=0, down=0, steps=Proc->msleep / 100;

		Counters_Genuine(cpu, 0);

		while(!leave)
		{
			down=steps;
			do {
				if(!kthread_should_stop())
					msleep(100);
				else
					leave=1;
			} while(--down && !leave);

			Counters_Genuine(cpu, 1);
			Core_Temp(cpu);

			// Absolute Delta of Unhalted (Core & Ref) C0 Counter.
			Proc->Core[cpu].Delta.C0.UCC=				\
				(Proc->Core[cpu].Counter[0].C0.UCC >		\
				Proc->Core[cpu].Counter[1].C0.UCC) ?		\
					Proc->Core[cpu].Counter[0].C0.UCC	\
					- Proc->Core[cpu].Counter[1].C0.UCC	\
					: Proc->Core[cpu].Counter[1].C0.UCC	\
					- Proc->Core[cpu].Counter[0].C0.UCC;

			Proc->Core[cpu].Delta.C0.URC=			\
				Proc->Core[cpu].Counter[1].C0.URC	\
				- Proc->Core[cpu].Counter[0].C0.URC;

			Proc->Core[cpu].Delta.TSC=		\
				Proc->Core[cpu].Counter[1].TSC	\
				- Proc->Core[cpu].Counter[0].TSC;

			Proc->Core[cpu].Delta.C1=			\
				(Proc->Core[cpu].Counter[0].C1 >	\
				 Proc->Core[cpu].Counter[1].C1) ?	\
					Proc->Core[cpu].Counter[0].C1	\
					- Proc->Core[cpu].Counter[1].C1	\
					: Proc->Core[cpu].Counter[1].C1	\
					- Proc->Core[cpu].Counter[0].C1;

			// Save TSC.
			Proc->Core[cpu].Counter[0].TSC=		\
				Proc->Core[cpu].Counter[1].TSC;

			// Save the Unhalted Core & Reference Counter
			// for next iteration.
			Proc->Core[cpu].Counter[0].C0.UCC=	\
				Proc->Core[cpu].Counter[1].C0.UCC;
			Proc->Core[cpu].Counter[1].C0.URC=	\
				Proc->Core[cpu].Counter[1].C0.URC;

			Proc->Core[cpu].Counter[0].C1=		\
				Proc->Core[cpu].Counter[1].C1;

			atomic_store(&Proc->Core[cpu].Sync, 0x1);
		}
	}
	do_exit(0);
}

void Arch_Genuine(unsigned int stage)
{
	unsigned int cpu=0;

	switch(stage)
	{
		case END:
		{
		}
		break;
		case INIT:
		{
			PLATFORM_INFO Platform={0};
			TURBO_RATIO Turbo={0};

			RDMSR(Platform, MSR_NHM_PLATFORM_INFO);
			RDMSR(Turbo, MSR_NHM_TURBO_RATIO_LIMIT);

			if(Platform.qword != 0)
			{
			  Proc->Boost[0]=MINCNT(Platform.MinimumRatio,	\
						Platform.MaxNonTurboRatio);
			  Proc->Boost[1]=MAXCNT(Platform.MinimumRatio,	\
						Platform.MaxNonTurboRatio);
			}
			if(Turbo.qword != 0)
			{
				Proc->Boost[2]=Turbo.MaxRatio_8C;
				Proc->Boost[3]=Turbo.MaxRatio_7C;
				Proc->Boost[4]=Turbo.MaxRatio_6C;
				Proc->Boost[5]=Turbo.MaxRatio_5C;
				Proc->Boost[6]=Turbo.MaxRatio_4C;
				Proc->Boost[7]=Turbo.MaxRatio_3C;
				Proc->Boost[8]=Turbo.MaxRatio_2C;
				Proc->Boost[9]=Turbo.MaxRatio_1C;
			}
			else
				Proc->Boost[9]=Proc->Boost[1];
		}
		break;
		case STOP:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				kthread_stop(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
		case START:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine)
			    {
				Proc->Core[cpu].TID[CYCLE_TID]= \
				kthread_create(	Cycle_Genuine,
						&Proc->Core[cpu],
						"kintelfreq-%03d",
						Proc->Core[cpu].Bind);
				if(!IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				    kthread_bind(Proc->Core[cpu].TID[CYCLE_TID], cpu);
			    }
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				wake_up_process(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
	}
}

signed int Cycle_Core2(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;
		unsigned int leave=0, down=0, steps=Proc->msleep / 100;

		Counters_Set(cpu);
		Counters_Core2(cpu, 0);

		while(!leave)
		{
			down=steps;
			do {
				if(!kthread_should_stop())
					msleep(100);
				else
					leave=1;
			} while(--down && !leave);

			Counters_Core2(cpu, 1);
			Core_Temp(cpu);

			// Delta of Instructions Retired
			Proc->Core[cpu].Delta.INST=			\
				Proc->Core[cpu].Counter[1].INST		\
				- Proc->Core[cpu].Counter[0].INST;

			// Absolute Delta of Unhalted (Core & Ref) C0 Counter.
			Proc->Core[cpu].Delta.C0.UCC=				\
				(Proc->Core[cpu].Counter[0].C0.UCC >		\
				Proc->Core[cpu].Counter[1].C0.UCC) ?		\
					Proc->Core[cpu].Counter[0].C0.UCC	\
					- Proc->Core[cpu].Counter[1].C0.UCC	\
					: Proc->Core[cpu].Counter[1].C0.UCC	\
					- Proc->Core[cpu].Counter[0].C0.UCC;

			Proc->Core[cpu].Delta.C0.URC=			\
				Proc->Core[cpu].Counter[1].C0.URC	\
				- Proc->Core[cpu].Counter[0].C0.URC;

			Proc->Core[cpu].Delta.TSC=		\
				Proc->Core[cpu].Counter[1].TSC	\
				- Proc->Core[cpu].Counter[0].TSC;

			Proc->Core[cpu].Delta.C1=			\
				(Proc->Core[cpu].Counter[0].C1 >	\
				 Proc->Core[cpu].Counter[1].C1) ?	\
					Proc->Core[cpu].Counter[0].C1	\
					- Proc->Core[cpu].Counter[1].C1	\
					: Proc->Core[cpu].Counter[1].C1	\
					- Proc->Core[cpu].Counter[0].C1;

			// Save the Instructions counter.
			Proc->Core[cpu].Counter[0].INST=	\
				Proc->Core[cpu].Counter[1].INST;

			// Save TSC.
			Proc->Core[cpu].Counter[0].TSC=		\
				Proc->Core[cpu].Counter[1].TSC;

			// Save the Unhalted Core & Reference Counter
			// for next iteration.
			Proc->Core[cpu].Counter[0].C0.UCC=	\
				Proc->Core[cpu].Counter[1].C0.UCC;
			Proc->Core[cpu].Counter[1].C0.URC=	\
				Proc->Core[cpu].Counter[1].C0.URC;

			Proc->Core[cpu].Counter[0].C1=		\
				Proc->Core[cpu].Counter[1].C1;

			atomic_store(&Proc->Core[cpu].Sync, 0x1);
		}
		Counters_Clear(cpu);
	}
	do_exit(0);
}

void Arch_Core2(unsigned int stage)
{
	unsigned int cpu=0;

	switch(stage)
	{
		case END:
		{
		}
		break;
		case INIT:
		{
			PLATFORM_INFO Platform={0};
			TURBO_RATIO Turbo={0};

			RDMSR(Platform, MSR_NHM_PLATFORM_INFO);
			RDMSR(Turbo, MSR_NHM_TURBO_RATIO_LIMIT);

			if(Platform.qword != 0)
			{
			  Proc->Boost[0]=MINCNT(Platform.MinimumRatio,	\
						Platform.MaxNonTurboRatio);
			  Proc->Boost[1]=MAXCNT(Platform.MinimumRatio,	\
						Platform.MaxNonTurboRatio);
			}
			if(Turbo.qword != 0)
			{
				Proc->Boost[2]=Turbo.MaxRatio_8C;
				Proc->Boost[3]=Turbo.MaxRatio_7C;
				Proc->Boost[4]=Turbo.MaxRatio_6C;
				Proc->Boost[5]=Turbo.MaxRatio_5C;
				Proc->Boost[6]=Turbo.MaxRatio_4C;
				Proc->Boost[7]=Turbo.MaxRatio_3C;
				Proc->Boost[8]=Turbo.MaxRatio_2C;
				Proc->Boost[9]=Turbo.MaxRatio_1C;
			}
			else
				Proc->Boost[9]=Proc->Boost[1];
		}
		break;
		case STOP:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				kthread_stop(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
		case START:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine)
			    {
				Proc->Core[cpu].TID[CYCLE_TID]= \
				kthread_create(	Cycle_Core2,
						&Proc->Core[cpu],
						"kintelfreq-%03d",
						Proc->Core[cpu].Bind);
				if(!IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				    kthread_bind(Proc->Core[cpu].TID[CYCLE_TID], cpu);
			    }
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				wake_up_process(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
	}
}

signed int Cycle_Nehalem(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;
		unsigned int leave=0, down=0, steps=Proc->msleep / 100;

		Counters_Set(cpu);
		Counters_Nehalem(cpu, 0);

		while(!leave)
		{
			down=steps;
			do {
				if(!kthread_should_stop())
					msleep(100);
				else
					leave=1;
			} while(--down && !leave);

			Counters_Nehalem(cpu, 1);
			Core_Temp(cpu);

			// Delta of Instructions Retired
			Proc->Core[cpu].Delta.INST=			\
				Proc->Core[cpu].Counter[1].INST		\
				- Proc->Core[cpu].Counter[0].INST;

			// Absolute Delta of Unhalted (Core & Ref) C0 Counter.
			Proc->Core[cpu].Delta.C0.UCC=				\
				(Proc->Core[cpu].Counter[0].C0.UCC >		\
				Proc->Core[cpu].Counter[1].C0.UCC) ?		\
					Proc->Core[cpu].Counter[0].C0.UCC	\
					- Proc->Core[cpu].Counter[1].C0.UCC	\
					: Proc->Core[cpu].Counter[1].C0.UCC	\
					- Proc->Core[cpu].Counter[0].C0.UCC;

			Proc->Core[cpu].Delta.C0.URC=			\
				Proc->Core[cpu].Counter[1].C0.URC	\
				- Proc->Core[cpu].Counter[0].C0.URC;

			Proc->Core[cpu].Delta.C3=		\
				Proc->Core[cpu].Counter[1].C3	\
				- Proc->Core[cpu].Counter[0].C3;
			Proc->Core[cpu].Delta.C6=		\
				Proc->Core[cpu].Counter[1].C6	\
				- Proc->Core[cpu].Counter[0].C6;

			Proc->Core[cpu].Delta.TSC=		\
				Proc->Core[cpu].Counter[1].TSC	\
				- Proc->Core[cpu].Counter[0].TSC;

			Proc->Core[cpu].Delta.C1=			\
				(Proc->Core[cpu].Counter[0].C1 >	\
				 Proc->Core[cpu].Counter[1].C1) ?	\
					Proc->Core[cpu].Counter[0].C1	\
					- Proc->Core[cpu].Counter[1].C1	\
					: Proc->Core[cpu].Counter[1].C1	\
					- Proc->Core[cpu].Counter[0].C1;

			// Save the Instructions counter.
			Proc->Core[cpu].Counter[0].INST=	\
				Proc->Core[cpu].Counter[1].INST;

			// Save TSC.
			Proc->Core[cpu].Counter[0].TSC=		\
				Proc->Core[cpu].Counter[1].TSC;

			// Save the Unhalted Core & Reference Counter
			// for next iteration.
			Proc->Core[cpu].Counter[0].C0.UCC=	\
				Proc->Core[cpu].Counter[1].C0.UCC;
			Proc->Core[cpu].Counter[1].C0.URC=	\
				Proc->Core[cpu].Counter[1].C0.URC;

			// Save also the C-State Reference Counter.
			Proc->Core[cpu].Counter[0].C3=		\
				Proc->Core[cpu].Counter[1].C3;
			Proc->Core[cpu].Counter[0].C6=		\
				Proc->Core[cpu].Counter[1].C6;
			Proc->Core[cpu].Counter[0].C1=		\
				Proc->Core[cpu].Counter[1].C1;

			atomic_store(&Proc->Core[cpu].Sync, 0x1);
		}
		Counters_Clear(cpu);
	}
	do_exit(0);
}

void Arch_Nehalem(unsigned int stage)
{
	unsigned int cpu=0;

	switch(stage)
	{
		case END:
		{
		}
		break;
		case INIT:
		{
			PLATFORM_INFO Platform={0};
			TURBO_RATIO Turbo={0};

			RDMSR(Platform, MSR_NHM_PLATFORM_INFO);
			RDMSR(Turbo, MSR_NHM_TURBO_RATIO_LIMIT);

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
		break;
		case STOP:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				kthread_stop(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
		case START:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine)
			    {
				Proc->Core[cpu].TID[CYCLE_TID]= \
				kthread_create(	Cycle_Nehalem,
						&Proc->Core[cpu],
						"kintelfreq-%03d",
						Proc->Core[cpu].Bind);
				if(!IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				    kthread_bind(Proc->Core[cpu].TID[CYCLE_TID], cpu);
			    }
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				wake_up_process(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
	}
}

signed int Cycle_SandyBridge(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;
		unsigned int leave=0, down=0, steps=Proc->msleep / 100;

		Counters_Set(cpu);
		Counters_SandyBridge(cpu, 0);

		while(!leave)
		{
			down=steps;
			do {
				if(!kthread_should_stop())
					msleep(100);
				else
					leave=1;
			} while(--down && !leave);

			Counters_SandyBridge(cpu, 1);
			Core_Temp(cpu);

			// Delta of Instructions Retired
			Proc->Core[cpu].Delta.INST=			\
				Proc->Core[cpu].Counter[1].INST		\
				- Proc->Core[cpu].Counter[0].INST;

			// Absolute Delta of Unhalted (Core & Ref) C0 Counter.
			Proc->Core[cpu].Delta.C0.UCC=				\
				(Proc->Core[cpu].Counter[0].C0.UCC >		\
				Proc->Core[cpu].Counter[1].C0.UCC) ?		\
					Proc->Core[cpu].Counter[0].C0.UCC	\
					- Proc->Core[cpu].Counter[1].C0.UCC	\
					: Proc->Core[cpu].Counter[1].C0.UCC	\
					- Proc->Core[cpu].Counter[0].C0.UCC;

			Proc->Core[cpu].Delta.C0.URC=			\
				Proc->Core[cpu].Counter[1].C0.URC	\
				- Proc->Core[cpu].Counter[0].C0.URC;

			Proc->Core[cpu].Delta.C3=		\
				Proc->Core[cpu].Counter[1].C3	\
				- Proc->Core[cpu].Counter[0].C3;
			Proc->Core[cpu].Delta.C6=		\
				Proc->Core[cpu].Counter[1].C6	\
				- Proc->Core[cpu].Counter[0].C6;
			Proc->Core[cpu].Delta.C7=		\
				Proc->Core[cpu].Counter[1].C7	\
				- Proc->Core[cpu].Counter[0].C7;

			Proc->Core[cpu].Delta.TSC=		\
				Proc->Core[cpu].Counter[1].TSC	\
				- Proc->Core[cpu].Counter[0].TSC;

			Proc->Core[cpu].Delta.C1=			\
				(Proc->Core[cpu].Counter[0].C1 >	\
				 Proc->Core[cpu].Counter[1].C1) ?	\
					Proc->Core[cpu].Counter[0].C1	\
					- Proc->Core[cpu].Counter[1].C1	\
					: Proc->Core[cpu].Counter[1].C1	\
					- Proc->Core[cpu].Counter[0].C1;

			// Save the Instructions counter.
			Proc->Core[cpu].Counter[0].INST=	\
				Proc->Core[cpu].Counter[1].INST;

			// Save TSC.
			Proc->Core[cpu].Counter[0].TSC=		\
				Proc->Core[cpu].Counter[1].TSC;

			// Save the Unhalted Core & Reference Counter
			// for next iteration.
			Proc->Core[cpu].Counter[0].C0.UCC=	\
				Proc->Core[cpu].Counter[1].C0.UCC;
			Proc->Core[cpu].Counter[1].C0.URC=	\
				Proc->Core[cpu].Counter[1].C0.URC;

			// Save also the C-State Reference Counter.
			Proc->Core[cpu].Counter[0].C3=		\
				Proc->Core[cpu].Counter[1].C3;
			Proc->Core[cpu].Counter[0].C6=		\
				Proc->Core[cpu].Counter[1].C6;
			Proc->Core[cpu].Counter[0].C7=		\
				Proc->Core[cpu].Counter[1].C7;
			Proc->Core[cpu].Counter[0].C1=		\
				Proc->Core[cpu].Counter[1].C1;

			atomic_store(&Proc->Core[cpu].Sync, 0x1);
		}
		Counters_Clear(cpu);
	}
	do_exit(0);
}

void Arch_SandyBridge(unsigned int stage)
{
	unsigned int cpu=0;

	switch(stage)
	{
		case END:
		{
		}
		break;
		case INIT:
		{
			PLATFORM_INFO Platform={0};
			TURBO_RATIO Turbo={0};

			RDMSR(Platform, MSR_NHM_PLATFORM_INFO);
			RDMSR(Turbo, MSR_NHM_TURBO_RATIO_LIMIT);

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
		break;
		case STOP:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				kthread_stop(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
		case START:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine)
			    {
				Proc->Core[cpu].TID[CYCLE_TID]= \
				kthread_create(	Cycle_SandyBridge,
						&Proc->Core[cpu],
						"kintelfreq-%03d",
						Proc->Core[cpu].Bind);
				if(!IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				    kthread_bind(Proc->Core[cpu].TID[CYCLE_TID], cpu);
			    }
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			    if(!Proc->Core[cpu].OffLine
			    && !IS_ERR(Proc->Core[cpu].TID[CYCLE_TID]))
				wake_up_process(Proc->Core[cpu].TID[CYCLE_TID]);
		}
		break;
	}
}

static signed int IntelFreq_mmap(struct file *filp, struct vm_area_struct *vma)
{
	if(Proc)
	{
		remap_vmalloc_range(vma, Proc, 0);
	}
	return(0);
}

static signed int IntelFreq_release(struct inode *inode, struct file *file)
{
	if(Proc)
		Proc->msleep=LOOP_DEF_MS;
	return(0);
}

static struct file_operations IntelFreq_fops=
{
	.mmap	= IntelFreq_mmap,
	.open	= nonseekable_open,
	.release= IntelFreq_release
};

static signed int __init IntelFreq_init(void)
{
	Proc=vmalloc_user(sizeof(PROC));
	Proc->msleep=LOOP_DEF_MS;
	Proc->PerCore=0;

	IntelFreq.kcdev=cdev_alloc();
	IntelFreq.kcdev->ops=&IntelFreq_fops;
	IntelFreq.kcdev->owner=THIS_MODULE;

        if(alloc_chrdev_region(&IntelFreq.nmdev, 0, 1, SHM_FILENAME) >= 0)
	{
		IntelFreq.Major=MAJOR(IntelFreq.nmdev);
		IntelFreq.mkdev=MKDEV(IntelFreq.Major,0);

		if(cdev_add(IntelFreq.kcdev, IntelFreq.mkdev, 1) >= 0)
		{
			struct device *tmpDev;

			IntelFreq.clsdev=class_create(THIS_MODULE, SHM_DEVNAME);

			if((tmpDev=device_create(IntelFreq.clsdev, NULL,
						 IntelFreq.mkdev, NULL,
						 SHM_DEVNAME)) != NULL)
			{
				unsigned int cpu=0;

				Proc->CPU.Count=Core_Count();

				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				{
					atomic_init(&Proc->Core[cpu].Sync, 0x0);
					Proc->Core[cpu].Bind=cpu;
					if(!cpu_online(cpu) || !cpu_active(cpu))
						Proc->Core[cpu].OffLine=1;
					else
						Proc->Core[cpu].OffLine=0;
				}
				Proc_Features(&Proc->Features);

				for(Proc->ArchID=ARCHITECTURES - 1;
					Proc->ArchID >0;
						Proc->ArchID--)
					if(!(Arch[Proc->ArchID].Signature.ExtFamily
					^ Proc->Features.Std.AX.ExtFamily)
					&& !(Arch[Proc->ArchID].Signature.Family
					^ Proc->Features.Std.AX.Family)
					&& !(Arch[Proc->ArchID].Signature.ExtModel
					^ Proc->Features.Std.AX.ExtModel)
					&& !(Arch[Proc->ArchID].Signature.Model
					^ Proc->Features.Std.AX.Model))
						break;

				Arch[Proc->ArchID].Arch_Controller(INIT);

				Proc->CPU.OnLine=Proc_Topology();
				Proc->PerCore=(Proc->Features.HTT_enabled) ? 1 : 0;
				Proc->Clock=Proc_Clock(Proc->Boost[1]);

				printk(	"IntelFreq [%s]\n" \
					"Signature [%1X%1X_%1X%1X]" \
					" Architecture [%s]\n" \
					"%u/%u CPU Online" \
					" , Clock @ {%u/%llu} MHz\n" \
					"Ratio={%d,%d,%d,%d,%d,%d,%d,%d,%d,%d}\n",
					Proc->Features.Brand,
					Arch[Proc->ArchID].Signature.ExtFamily,
					Arch[Proc->ArchID].Signature.Family,
					Arch[Proc->ArchID].Signature.ExtModel,
					Arch[Proc->ArchID].Signature.Model,
					Arch[Proc->ArchID].Architecture,
					Proc->CPU.OnLine,
					Proc->CPU.Count,
					Proc->Clock.Q,
					Proc->Clock.R,
					Proc->Boost[0],
					Proc->Boost[1],
					Proc->Boost[2],
					Proc->Boost[3],
					Proc->Boost[4],
					Proc->Boost[5],
					Proc->Boost[6],
					Proc->Boost[7],
					Proc->Boost[8],
					Proc->Boost[9]);

				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
					printk(	"Topology(%u)"	\
						" Apic[%3d]"	\
						" Core[%3d]"	\
						" Thread[%3d]\n",
						cpu,
						Proc->Core[cpu].T.ApicID,
						Proc->Core[cpu].T.CoreID,
						Proc->Core[cpu].T.ThreadID);

				Arch[Proc->ArchID].Arch_Controller(START);
			}
			else
			{
				printk("IntelFreq_init():device_create():KO\n");
				return(-EBUSY);
			}
		}
		else
		{
			printk("IntelFreq_init():cdev_add():KO\n");
			return(-EBUSY);
		}
	}
	else
	{
		printk("IntelFreq_init():alloc_chrdev_region():KO\n");
		return(-EBUSY);
	}
	return(0);
}

static void __exit IntelFreq_cleanup(void)
{
	device_destroy(IntelFreq.clsdev, IntelFreq.mkdev);
	class_destroy(IntelFreq.clsdev);
	cdev_del(IntelFreq.kcdev);
	unregister_chrdev_region(IntelFreq.mkdev, 1);

	if(Proc)
	{
		Arch[Proc->ArchID].Arch_Controller(STOP);
//		Arch[Proc->ArchID].Arch_Controller(END);
		vfree(Proc);
	}
}

module_init(IntelFreq_init);
module_exit(IntelFreq_cleanup);
