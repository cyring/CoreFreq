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
	int Major;
	struct cdev *kcdev;
	dev_t nmdev, mkdev;
	struct class *clsdev;
} IntelFreq;

static PROC *Proc=NULL;

unsigned int Core_Count(void)
{
	unsigned int Count=0;

	__asm__ volatile
	(
		"movq	$0x4, %%rax;"
		"xorq	%%rcx, %%rcx;"
		"cpuid;"
		"shr	$26, %%rax;"
		"and	$0x3f, %%rax;"
		"add	$1, %%rax;"
		: "=a"	(Count)
	);
	return(Count);
}

void Proc_Brand(void)
{
	char tmpString[48+1]={0x20};
	int ix=0, jx=0, px=0;
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
			Proc->Brand[ix++]=tmpString[jx];
}

unsigned int Proc_Clock(unsigned int ratio)
{
	unsigned long long int TSC[2];
	unsigned int Lo, Hi, Clock;

	__asm__ volatile
	(
		"rdtsc"
		:"=a" (Lo),
		 "=d" (Hi)
	);
	TSC[0]=((unsigned long long int) Lo)		\
		| (((unsigned long long int) Hi) << 32);

	ssleep(1);

	__asm__ volatile
	(
		"rdtsc"
		:"=a" (Lo),
		 "=d" (Hi)
	);
	TSC[1]=((unsigned long long int) Lo)		\
		| (((unsigned long long int) Hi) << 32);
	TSC[1]-=TSC[0];

	Clock=TSC[1] / (ratio * 1000000);
	return(Clock);
}

void Core_Nehalem(unsigned int cpu, int T)
{
	register unsigned long long int Cx=0;

	// Instructions Retired
	RDMSR(	Proc->Core[cpu].Cycles.INST[T].Lo,
		Proc->Core[cpu].Cycles.INST[T].Hi,
		MSR_CORE_PERF_FIXED_CTR0);

	// Unhalted Core & Reference Cycles.
	RDMSR(	Proc->Core[cpu].Cycles.C0[T].UCC.Lo,
		Proc->Core[cpu].Cycles.C0[T].UCC.Hi,
		MSR_CORE_PERF_FIXED_CTR1);
	RDMSR(	Proc->Core[cpu].Cycles.C0[T].URC.Lo,
		Proc->Core[cpu].Cycles.C0[T].URC.Hi,
		MSR_CORE_PERF_FIXED_CTR2);

	// TSC in relation to the Logical Core.
	RDMSR(	Proc->Core[cpu].Cycles.TSC[T].Lo,
		Proc->Core[cpu].Cycles.TSC[T].Hi,
		MSR_IA32_TSC);

	// C-States.
	RDMSR(	Proc->Core[cpu].Cycles.C3[T].Lo,
		Proc->Core[cpu].Cycles.C3[T].Hi,
		MSR_CORE_C3_RESIDENCY);
	RDMSR(	Proc->Core[cpu].Cycles.C6[T].Lo,
		Proc->Core[cpu].Cycles.C6[T].Hi,
		MSR_CORE_C6_RESIDENCY);

	// Derive C1
	Cx=	Proc->Core[cpu].Cycles.C6[T].r64	\
		+ Proc->Core[cpu].Cycles.C3[T].r64	\
		+ Proc->Core[cpu].Cycles.C0[T].URC.r64;

	Proc->Core[cpu].Cycles.C1[T]=				\
		(Proc->Core[cpu].Cycles.TSC[T].r64 > Cx) ?	\
			Proc->Core[cpu].Cycles.TSC[T].r64 - Cx	\
			: 0;
}
/*
void Core_Temp(unsigned int cpu)
{
	RDMSR(	Proc->Core[cpu].TjMax.Lo,
		Proc->Core[cpu].TjMax.Hi,
		MSR_IA32_TEMPERATURE_TARGET)
	RDMSR(	Proc->Core[cpu].ThermStat.Lo,
		Proc->Core[cpu].ThermStat.Hi,
		MSR_IA32_THERM_STATUS)
}
*/
int Core_Cycle(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;

		Core_Nehalem(cpu, 0);

		while(!kthread_should_stop())
		{
			msleep(Proc->msleep);

			Core_Nehalem(cpu, 1);

			// Delta of Instructions Retired
			Proc->Core[cpu].Delta.INST=			\
				Proc->Core[cpu].Cycles.INST[1].r64	\
				- Proc->Core[cpu].Cycles.INST[0].r64;

			// Absolute Delta of Unhalted (Core & Ref) C0 Cycles.
			Proc->Core[cpu].Delta.C0.UCC=                         \
				(Proc->Core[cpu].Cycles.C0[0].UCC.r64 >       \
				Proc->Core[cpu].Cycles.C0[1].UCC.r64) ?       \
					Proc->Core[cpu].Cycles.C0[0].UCC.r64  \
					- Proc->Core[cpu].Cycles.C0[1].UCC.r64\
					: Proc->Core[cpu].Cycles.C0[1].UCC.r64\
					- Proc->Core[cpu].Cycles.C0[0].UCC.r64;

			Proc->Core[cpu].Delta.C0.URC=			\
				Proc->Core[cpu].Cycles.C0[1].URC.r64	\
				- Proc->Core[cpu].Cycles.C0[0].URC.r64;

			Proc->Core[cpu].Delta.C3=			\
				Proc->Core[cpu].Cycles.C3[1].r64	\
				- Proc->Core[cpu].Cycles.C3[0].r64;
			Proc->Core[cpu].Delta.C6=			\
				Proc->Core[cpu].Cycles.C6[1].r64	\
				- Proc->Core[cpu].Cycles.C6[0].r64;
			Proc->Core[cpu].Delta.C7=			\
				Proc->Core[cpu].Cycles.C7[1].r64	\
				- Proc->Core[cpu].Cycles.C7[0].r64;

			Proc->Core[cpu].Delta.TSC=			\
				Proc->Core[cpu].Cycles.TSC[1].r64	\
				- Proc->Core[cpu].Cycles.TSC[0].r64;

			Proc->Core[cpu].Delta.C1=			\
				(Proc->Core[cpu].Cycles.C1[0] >		\
				 Proc->Core[cpu].Cycles.C1[1]) ?	\
					Proc->Core[cpu].Cycles.C1[0]	\
					- Proc->Core[cpu].Cycles.C1[1]	\
					: Proc->Core[cpu].Cycles.C1[1]	\
					- Proc->Core[cpu].Cycles.C1[0];

			// Save the Instructions counter.
			Proc->Core[cpu].Cycles.INST[0]=		\
				Proc->Core[cpu].Cycles.INST[1];

			// Save TSC.
			Proc->Core[cpu].Cycles.TSC[0]=		\
				Proc->Core[cpu].Cycles.TSC[1];

			// Save the Unhalted Core & Reference Cycles
			// for next iteration.
			Proc->Core[cpu].Cycles.C0[0].UCC=	\
				Proc->Core[cpu].Cycles.C0[1].UCC;
			Proc->Core[cpu].Cycles.C0[0].URC=	\
				Proc->Core[cpu].Cycles.C0[1].URC;

			// Save also the C-State Reference Cycles.
			Proc->Core[cpu].Cycles.C3[0]=		\
				Proc->Core[cpu].Cycles.C3[1];
			Proc->Core[cpu].Cycles.C6[0]=		\
				Proc->Core[cpu].Cycles.C6[1];
			Proc->Core[cpu].Cycles.C7[0]=		\
				Proc->Core[cpu].Cycles.C7[1];
			Proc->Core[cpu].Cycles.C1[0]=		\
				Proc->Core[cpu].Cycles.C1[1];

			atomic_store(&Proc->Core[cpu].Sync, 0x1);
		}
	}
	return(0);
}

int Counter_Start(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;

		GLOBAL_PERF_COUNTER GlobalPerfCounter={0};
		FIXED_PERF_COUNTER FixedPerfCounter={0};
		GLOBAL_PERF_STATUS Overflow={0};
		GLOBAL_PERF_OVF_CTRL OvfControl={0};

		RDMSR(	GlobalPerfCounter.Lo, GlobalPerfCounter.Hi,
			MSR_CORE_PERF_GLOBAL_CTRL);

		Proc->Core[cpu].SaveArea.GlobalPerfCounter=	\
						GlobalPerfCounter;
		GlobalPerfCounter.EN_FIXED_CTR0=1;
		GlobalPerfCounter.EN_FIXED_CTR1=1;
		GlobalPerfCounter.EN_FIXED_CTR2=1;

		WRMSR(	GlobalPerfCounter.Lo, GlobalPerfCounter.Hi,
			MSR_CORE_PERF_GLOBAL_CTRL);

		RDMSR(	FixedPerfCounter.Lo, FixedPerfCounter.Hi,
			MSR_CORE_PERF_FIXED_CTR_CTRL);

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
		WRMSR(	FixedPerfCounter.Lo, FixedPerfCounter.Hi,
			MSR_CORE_PERF_FIXED_CTR_CTRL);

		RDMSR(	Overflow.Lo, Overflow.Hi,
			MSR_CORE_PERF_GLOBAL_STATUS);
		if(Overflow.Overflow_CTR0)
			OvfControl.Clear_Ovf_CTR0=1;
		if(Overflow.Overflow_CTR1)
			OvfControl.Clear_Ovf_CTR1=1;
		if(Overflow.Overflow_CTR2)
			OvfControl.Clear_Ovf_CTR2=1;
		if(Overflow.Overflow_CTR0	\
			| Overflow.Overflow_CTR1	\
			| Overflow.Overflow_CTR2)
				WRMSR(	OvfControl.Lo, OvfControl.Hi,
					MSR_CORE_PERF_GLOBAL_OVF_CTRL);
	}
	return(0);
}

int Counter_Stop(void *arg)
{
	if(arg != NULL)
	{
		CORE *Core=(CORE *) arg;
		unsigned int cpu=Core->Bind;

		WRMSR(	Proc->Core[cpu].SaveArea.FixedPerfCounter.Lo,
			Proc->Core[cpu].SaveArea.FixedPerfCounter.Hi,
			MSR_CORE_PERF_FIXED_CTR_CTRL);

		WRMSR(	Proc->Core[cpu].SaveArea.GlobalPerfCounter.Lo,
			Proc->Core[cpu].SaveArea.GlobalPerfCounter.Hi,
			MSR_CORE_PERF_GLOBAL_CTRL);
	}
	return(0);
}

void Arch_Nehalem(unsigned int stage)
{
	unsigned int cpu=0;

	switch(stage)
	{
		case END:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			{
				Proc->Core[cpu].TID[COUNTER]= \
					kthread_create(	Counter_Stop,
							&Proc->Core[cpu],
							"kintelstop%02d",
							Proc->Core[cpu].Bind);
				kthread_bind(Proc->Core[cpu].TID[COUNTER], cpu);
				wake_up_process(Proc->Core[cpu].TID[COUNTER]);
			}
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				if(Proc->Core[cpu].TID[COUNTER])
					kthread_stop(Proc->Core[cpu].TID[COUNTER]);
		}
		break;
		case INIT:
		{
			PLATFORM_INFO Platform={0};
			TURBO_RATIO Turbo={0};
//			TURBO_RATIO_64 Turbo={0};

			RDMSR(Platform.Lo, Platform.Hi, MSR_NHM_PLATFORM_INFO);
			RDMSR(Turbo.Lo, Turbo.Hi, MSR_NHM_TURBO_RATIO_LIMIT);
//			RDMSR64(Turbo, MSR_NHM_TURBO_RATIO_LIMIT);

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

			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			{
				Proc->Core[cpu].Bind=cpu;
				Proc->Core[cpu].TID[COUNTER]= \
					kthread_create(	Counter_Start,
							&Proc->Core[cpu],
							"kintelstart%02d",
							Proc->Core[cpu].Bind);

				kthread_bind(Proc->Core[cpu].TID[COUNTER], cpu);
				wake_up_process(Proc->Core[cpu].TID[COUNTER]);
			}
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				if(Proc->Core[cpu].TID[COUNTER])
					kthread_stop(Proc->Core[cpu].TID[COUNTER]);
		}
		break;
		case STOP:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				if(Proc->Core[cpu].TID[CYCLE])
					kthread_stop(Proc->Core[cpu].TID[CYCLE]);
		}
		break;
		case START:
		{
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
			{
				Proc->Core[cpu].TID[CYCLE]=	\
					kthread_create(
						Core_Cycle,
						&Proc->Core[cpu],
						"kintelfreq%02d",
						Proc->Core[cpu].Bind);
				kthread_bind(Proc->Core[cpu].TID[CYCLE], cpu);
			}
			for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				if(Proc->Core[cpu].TID[CYCLE])
					wake_up_process(Proc->Core[cpu].TID[CYCLE]);
		}
		break;
	}
}

static int IntelFreq_mmap(struct file *filp, struct vm_area_struct *vma)
{
	if(Proc)
	{
		remap_vmalloc_range(vma, Proc, 0);
	}
	return(0);
}

static int IntelFreq_release(struct inode *inode, struct file *file)
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

static int __init IntelFreq_init(void)
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
				unsigned int count=0;

				count=Core_Count();
				Proc->CPU.Count=(!count) ? 1 : count;
				Proc->CPU.OnLine=Proc->CPU.Count;

				for(count=0; count < Proc->CPU.Count; count++)
					atomic_init(&Proc->Core[count].Sync, 0x0);

				Proc_Brand();

				Arch_Nehalem(INIT);

				Proc->Clock=Proc_Clock(Proc->Boost[1]);

				printk(	"IntelFreq [%s] [%d x CPU]\n"	\
					"[Clock @ %d MHz]  Ratio="	\
					"{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d}\n",
					Proc->Brand, Proc->CPU.Count,
					Proc->Clock,
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

				Arch_Nehalem(START);
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
		Arch_Nehalem(STOP);
		Arch_Nehalem(END);

		vfree(Proc);
	}
}

module_init(IntelFreq_init);
module_exit(IntelFreq_cleanup);
