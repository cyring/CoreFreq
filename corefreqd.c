/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "coretypes.h"
#include "corefreq.h"
#include "intelasm.h"
#include "intelmsr.h"
#include "corefreq-api.h"

#define	PAGE_SIZE (sysconf(_SC_PAGESIZE))

unsigned int Shutdown=0x0, Quiet=0x001;

typedef struct {
	PROC_STRUCT	*Proc;
	CPU_STRUCT	*Cpu;
	CORE		*Core;
	unsigned int	Bind;
	pthread_t	TID;
} ARG;


static void *Core_Cycle(void *arg)
{
	ARG *Arg=(ARG *) arg;
	PROC_STRUCT *Proc=Arg->Proc;
	CPU_STRUCT *Cpu=Arg->Cpu;
	CORE *Core=Arg->Core;
	unsigned int cpu=Arg->Bind;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);

	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

	char comm[TASK_COMM_LEN];
	sprintf(comm, "corefreqd/%-3d", cpu);
	pthread_setname_np(Arg->TID, comm);

	do
	{
	    while(!BITWISEAND(Core->Sync.V, 0x1) && !Shutdown)
		usleep(Proc->msleep * 50);
	    BITCLR(Core->Sync.V, 0);

	    if(!Shutdown)
	    {
		if(BITWISEAND(Proc->Room, 1 << cpu))
		{
			Cpu->Toggle =! Cpu->Toggle;
			BITCLR(Proc->Room, cpu);
		}
		struct FLIP_FLOP *Flip=&Cpu->FlipFlop[Cpu->Toggle];

		// Compute IPS=Instructions per TSC
		Flip->State.IPS	= (double) (Core->Delta.INST)		\
				/ (double) (Core->Delta.TSC);

		// Compute IPC=Instructions per non-halted reference cycle.
		// (Protect against a division by zero)
		Flip->State.IPC	= (double) (Core->Delta.C0.URC != 0) ?	\
				  (double) (Core->Delta.INST)		\
				/ (double) Core->Delta.C0.URC		\
				: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		Flip->State.CPI	= (double) (Core->Delta.INST != 0) ?	\
				  (double) Core->Delta.C0.URC		\
				/ (double) (Core->Delta.INST)		\
				: 0.0f;

		// Compute Turbo State.
		Flip->State.Turbo=(double) (Core->Delta.C0.UCC)		\
				/ (double) (Core->Delta.TSC);
		// Compute C-States.
		Flip->State.C0	= (double) (Core->Delta.C0.URC)		\
				/ (double) (Core->Delta.TSC);
		Flip->State.C3	= (double) (Core->Delta.C3)		\
				/ (double) (Core->Delta.TSC);
		Flip->State.C6	= (double) (Core->Delta.C6)		\
				/ (double) (Core->Delta.TSC);
		Flip->State.C7	= (double) (Core->Delta.C7)		\
				/ (double) (Core->Delta.TSC);
		Flip->State.C1	= (double) (Core->Delta.C1)		\
				/ (double) (Core->Delta.TSC);
		// Relative Ratio formula.
		Flip->Relative.Ratio	= (double) (Core->Delta.C0.UCC	\
						* Proc->Boost[1])	\
					/ (double) (Core->Delta.TSC);

		if(Core->Query.Turbo)
		{// Relative Frequency equals UCC per second.
			Flip->Relative.Freq=(double) (Core->Delta.C0.UCC)
						/ (Proc->msleep * 1000);
		}
		else
		{// Relative Frequency = Relative Ratio x Bus Clock Frequency
		Flip->Relative.Freq=
			(double) REL_FREQ(Proc->Boost[1], \
					Flip->Relative.Ratio, \
					Core->Clock) / (Proc->msleep * 1000);
		}
		Flip->Thermal.Target=Core->Thermal.Target;
		Flip->Thermal.Sensor=Core->Thermal.Sensor;
		Flip->Thermal.Temp=Flip->Thermal.Target - Flip->Thermal.Sensor;
		Flip->Thermal.Trip=Core->Thermal.Trip;
	    }
	} while(!Shutdown) ;

	return(NULL);
}

void Architecture(SHM_STRUCT *Shm, PROC *Proc)
{
	// Copy all BSP features.
	memcpy(&Shm->Proc.Features, &Proc->Features, sizeof(FEATURES));
	// Copy the numbers of total & online CPU.
	Shm->Proc.CPU.Count=Proc->CPU.Count;
	Shm->Proc.CPU.OnLine=Proc->CPU.OnLine;
	// Copy signature.
	Shm->Proc.Signature=Proc->Features.Std.Signature;
	// Copy the Architecture name.
	strncpy(Shm->Proc.Architecture, Proc->Architecture, 32);
	// Copy the base clock ratios.
	memcpy(Shm->Proc.Boost, Proc->Boost, (1+1+8) * sizeof(unsigned int));
	// Copy the processor's brand string.
	strncpy(Shm->Proc.Brand, Proc->Features.Brand, 48);
}

void InvariantTSC(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.InvariantTSC=(Proc->Features.Std.DX.TSC
				<< Proc->Features.InvariantTSC);
}

void PerformanceMonitoring(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.PM_version=Proc->Features.Perf_Monitoring_Leaf.AX.Version;
}

void HyperThreading(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.HyperThreading=Proc->Features.HTT_Enable;
}

void BaseClock(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{	// Copy per CPU base clock.
	Shm->Cpu[cpu].Clock.Q=Core[cpu]->Clock.Q;
	Shm->Cpu[cpu].Clock.R=Core[cpu]->Clock.R;
	Shm->Cpu[cpu].Clock.Hz=Core[cpu]->Clock.Hz;
}

void Topology(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{	// Copy Core topology.
	Shm->Cpu[cpu].Topology.BSP=(Core[cpu]->T.Base.BSP) ? 1 : 0;
	Shm->Cpu[cpu].Topology.ApicID=Core[cpu]->T.ApicID;
	Shm->Cpu[cpu].Topology.CoreID=Core[cpu]->T.CoreID;
	Shm->Cpu[cpu].Topology.ThreadID=Core[cpu]->T.ThreadID;
	Shm->Cpu[cpu].Topology.x2APIC=((Proc->Features.Std.CX.x2APIC
					& Core[cpu]->T.Base.EN)
					<< Core[cpu]->T.Base.EXTD);
	/* Compute Caches size.
		Bits 04-00: Cache Type Field
			0 = Null - No more caches
			1 = Data Cache
			2 = Instruction Cache
			3 = Unified Cache
			4-31 = Reserved
		Bits 07-05: Cache Level (starts at 1)
	*/
	unsigned int loop=0;
	for(loop=0; loop < CACHE_MAX_LEVEL; loop++)
	{
		if(Core[cpu]->T.Cache[loop].Type > 0)
		{
			unsigned int level=Core[cpu]->T.Cache[loop].Level;
			if(Core[cpu]->T.Cache[loop].Type == 2) // Instruction
				level=0;

			Shm->Cpu[cpu].Topology.Cache[level].Set=
				Core[cpu]->T.Cache[loop].Set + 1;
			Shm->Cpu[cpu].Topology.Cache[level].LineSz=
				Core[cpu]->T.Cache[loop].LineSz + 1;
			Shm->Cpu[cpu].Topology.Cache[level].Part=
				Core[cpu]->T.Cache[loop].Part + 1;
			Shm->Cpu[cpu].Topology.Cache[level].Way=
				Core[cpu]->T.Cache[loop].Way + 1;

			Shm->Cpu[cpu].Topology.Cache[level].Size=
			  Shm->Cpu[cpu].Topology.Cache[level].Set
			* Shm->Cpu[cpu].Topology.Cache[level].LineSz
			* Shm->Cpu[cpu].Topology.Cache[level].Part
			* Shm->Cpu[cpu].Topology.Cache[level].Way;
		}
	}
}

void SpeedStep(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	if(Core[cpu]->Query.EIST)
		BITSET(Shm->Proc.SpeedStep, cpu);
	else
		BITCLR(Shm->Proc.SpeedStep, cpu);
}

void TurboBoost(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	if(Core[cpu]->Query.Turbo)
		BITSET(Shm->Proc.TurboBoost, cpu);
	else
		BITCLR(Shm->Proc.TurboBoost, cpu);
}

void CStates(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	Shm->Cpu[cpu].C1E=Core[cpu]->Query.C1E;
	Shm->Cpu[cpu].C3A=Core[cpu]->Query.C3A;
	Shm->Cpu[cpu].C1A=Core[cpu]->Query.C1A;
	Shm->Cpu[cpu].C3U=Core[cpu]->Query.C3U;
	Shm->Cpu[cpu].C1U=Core[cpu]->Query.C1U;
}

void ThermalMonitoring(SHM_STRUCT *Shm,PROC *Proc,CORE **Core,unsigned int cpu)
{
	Shm->Cpu[cpu].Thermal.TM1 =Proc->Features.Std.DX.TM1;		//0001
	Shm->Cpu[cpu].Thermal.TM1|=(Core[cpu]->Thermal.TCC_Enable << 1);//0010
	Shm->Cpu[cpu].Thermal.TM1^=(Core[cpu]->Thermal.TM_Select << 1);	//0010
	Shm->Cpu[cpu].Thermal.TM2 =Proc->Features.Std.CX.TM2;		//0001
	Shm->Cpu[cpu].Thermal.TM2|=(Core[cpu]->Thermal.TM2_Enable << 1);//0010
}

void IdleDriver(SHM_STRUCT *Shm,PROC *Proc)
{
	if(strlen(Proc->IdleDriver.Name) > 0)
	{
		int i=0;

		strncpy(Shm->IdleDriver.Name,
			Proc->IdleDriver.Name, CPUIDLE_NAME_LEN - 1);

		Shm->IdleDriver.stateCount=Proc->IdleDriver.stateCount;
		for(i=0; i < Shm->IdleDriver.stateCount; i++)
		{
			strncpy(Shm->IdleDriver.State[i].Name,
				Proc->IdleDriver.State[i].Name,
				CPUIDLE_NAME_LEN - 1);

			Shm->IdleDriver.State[i].exitLatency=
				Proc->IdleDriver.State[i].exitLatency;
			Shm->IdleDriver.State[i].powerUsage=
				Proc->IdleDriver.State[i].powerUsage;
			Shm->IdleDriver.State[i].targetResidency=
				Proc->IdleDriver.State[i].targetResidency;
		}
	}
}

typedef	struct
{
	int	Drv,
		Svr;
} FD;

int Proc_Cycle(FD *fd, PROC *Proc)
{
	unsigned int	cpu=0;
	CORE		**Core;
	SHM_STRUCT	*Shm;
	int		rc=0;

	Core=calloc(Proc->CPU.Count, sizeof(Core));
	for(cpu=0; !rc && (cpu < Proc->CPU.Count); cpu++)
	{
		off_t offset=(1 + cpu) * PAGE_SIZE;
		if((Core[cpu]=mmap(NULL, PAGE_SIZE,
				PROT_READ|PROT_WRITE,
				MAP_SHARED,
				fd->Drv, offset)) == NULL)
			rc=4;
	}
	size_t ShmSize, ShmCpuSize;
	ShmCpuSize=sizeof(CPU_STRUCT) * Proc->CPU.Count;
	ShmSize=sizeof(SHM_STRUCT) + ShmCpuSize;
	ShmSize=PAGE_SIZE * ((ShmSize / PAGE_SIZE)
		+ ((ShmSize % PAGE_SIZE) ? 1 : 0));

	umask(!S_IRUSR|!S_IWUSR|!S_IRGRP|!S_IWGRP|!S_IROTH|!S_IWOTH);

	if(!rc)
	{	// Initialize shared memory.
	    if(((fd->Svr=shm_open(SHM_FILENAME,
				O_CREAT|O_TRUNC|O_RDWR,
				S_IRUSR|S_IWUSR				\
				|S_IRGRP|S_IWGRP			\
				|S_IROTH|S_IWOTH)) != -1)
	    && (ftruncate(fd->Svr, ShmSize) != -1)
	    && ((Shm=mmap(0, ShmSize,
			PROT_READ|PROT_WRITE, MAP_SHARED,
			fd->Svr, 0)) != MAP_FAILED))
	    {
		// Copy Processor data from Kernel to Userspace.
		memset(Shm, 0, ShmSize);

		Architecture(Shm, Proc);

		// Copy the high-resolution timer delay.
		Shm->Proc.msleep=Proc->msleep;

		// Technologies aggregation.

		PerformanceMonitoring(Shm, Proc);

		InvariantTSC(Shm, Proc);

		HyperThreading(Shm, Proc);

		// Store the application name.
		strncpy(Shm->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

		// Aggregate the OS idle driver data.
		IdleDriver(Shm, Proc);

		// Initialize notification.
		BITCLR(Shm->Proc.Sync, 0);

		// Copy per CPU data from Kernel to Userspace.
		unsigned long long roomSeed=0x0;
		for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
		{
			BaseClock(Shm, Core, cpu);

			Topology(Shm, Proc, Core, cpu);

			SpeedStep(Shm, Proc, Core, cpu);

			TurboBoost(Shm, Core, cpu);

			CStates(Shm, Core, cpu);

			ThermalMonitoring(Shm, Proc, Core, cpu);

			// Define the Clients room mask.
			if(!(Shm->Cpu[cpu].OffLine=Core[cpu]->OffLine))
			{
				BITSET(Shm->Proc.Room, cpu);
				BITSET(roomSeed, cpu);
			}
		}
		// Welcomes with brand and per CPU base clock.
		if(Quiet & 0x001)
		 printf("CoreFreq Daemon."				\
			"  Copyright (C) 2015-2016 CYRIL INGENIERIE\n");
		if(Quiet & 0x100)
		 printf("\n  Loop(%u)\n", Shm->Proc.msleep);
		if(Quiet & 0x010)
		 printf("\n"						\
			"  Processor [%s]\n"				\
			"  Architecture [%s] %u/%u CPU Online.\n",
			Shm->Proc.Brand,
			Shm->Proc.Architecture,
			Shm->Proc.CPU.OnLine,
			Shm->Proc.CPU.Count );
		if(Quiet & 0x100)
		 printf("  BSP: x2APIC[%d:%d:%d] [TSC:%c-%c]"		\
			" [HTT:%u-%u] [EIST:%u-%x] [IDA:%u-%x]"		\
			" [TM:%u-%u-%u-%u-%u]\n\n",
			Proc->Features.Std.CX.x2APIC,
			Core[0]->T.Base.EN,
			Core[0]->T.Base.EXTD,
			Proc->Features.Std.DX.TSC ?
				Proc->Features.ExtFunc.DX.RdTSCP ? 'P':'1':'0',
			Proc->Features.InvariantTSC ? 'I':'V',
			Proc->Features.Std.DX.HTT,
				Proc->Features.HTT_Enable,
			Proc->Features.Std.CX.EIST,
				Shm->Proc.SpeedStep,
			Proc->Features.Thermal_Power_Leaf.AX.TurboIDA,
				Shm->Proc.TurboBoost,
			Proc->Features.Std.DX.TM1,
			Proc->Features.Std.CX.TM2,
			Core[0]->Thermal.TCC_Enable,
			Core[0]->Thermal.TM2_Enable,
			Core[0]->Thermal.TM_Select );

		// Launch one Server thread per online CPU.
		ARG *Arg=calloc(Shm->Proc.CPU.Count, sizeof(ARG));
		for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
			if(!Shm->Cpu[cpu].OffLine)
			{
				Arg[cpu].Proc=&Shm->Proc;
				Arg[cpu].Core=Core[cpu];
				Arg[cpu].Cpu=&Shm->Cpu[cpu];
				Arg[cpu].Bind=cpu;
				pthread_create(	&Arg[cpu].TID,
						NULL,
						Core_Cycle,
						&Arg[cpu]);
				if(Quiet & 0x100)
				    printf("    CPU #%03u @ %.2f MHz\n", cpu,
					(double)( Shm->Cpu[cpu].Clock.Hz
						* Shm->Proc.Boost[1])
						/ 1000000L );
			}
		fflush(stdout);
		// Main loop : aggregate the ratios.
		while(!Shutdown)
		{	// Wait until all the rooms & mask are cleared.
			while(!Shutdown && BITWISEAND(Shm->Proc.Room,roomSeed))
				usleep(Shm->Proc.msleep * 50);

			Shm->Proc.Avg.Turbo=0;
			Shm->Proc.Avg.C0=0;
			Shm->Proc.Avg.C3=0;
			Shm->Proc.Avg.C6=0;
			Shm->Proc.Avg.C7=0;
			Shm->Proc.Avg.C1=0;

			for(cpu=0;
			   (cpu < Shm->Proc.CPU.Count) && !Shutdown; cpu++)
				if(!Shm->Cpu[cpu].OffLine)
				{ // For each cpu,
				  // store into the alternated memory structure.
					struct FLIP_FLOP *Flop=		      \
						&Shm->Cpu[cpu].FlipFlop[      \
							!Shm->Cpu[cpu].Toggle \
						];

					Shm->Proc.Avg.Turbo+=Flop->State.Turbo;
					Shm->Proc.Avg.C0+=Flop->State.C0;
					Shm->Proc.Avg.C3+=Flop->State.C3;
					Shm->Proc.Avg.C6+=Flop->State.C6;
					Shm->Proc.Avg.C7+=Flop->State.C7;
					Shm->Proc.Avg.C1+=Flop->State.C1;
				}
			Shm->Proc.Room=~(1 << Shm->Proc.CPU.Count);

			Shm->Proc.Avg.Turbo/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C0/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C3/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C6/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C7/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C1/=Shm->Proc.CPU.OnLine;
			// Notify.
			BITSET(Shm->Proc.Sync, 0);
		}
		// Synchronize threads for shutdown.
		for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
			if(Arg[cpu].TID)
				pthread_join(Arg[cpu].TID, NULL);
		free(Arg);

		munmap(Shm, ShmSize);
		shm_unlink(SHM_FILENAME);
	    }
	    else rc=5;
	}
	for(cpu=0; cpu < Proc->CPU.Count; cpu++)
		if(Core[cpu] != NULL)
			munmap(Core[cpu], PAGE_SIZE);
	free(Core);
	return(rc);
}

typedef struct {
	sigset_t Signal;
	pthread_t TID;
	int Started;
} SIG;

static void *Emergency(void *arg)
{
	int caught=0, leave=0;
	SIG *Sig=(SIG *) arg;
	pthread_setname_np(Sig->TID, "corefreqd-kill");

	while(!leave && !sigwait(&Sig->Signal, &caught))
		switch(caught)
		{
			case SIGUSR1:
				leave=0x1;
			break;
			case SIGINT:
			case SIGQUIT:
			case SIGTERM:
				Shutdown=0x1;
			break;
		}
	return(NULL);
}

int main(int argc, char *argv[])
{
	FD	fd={0, 0};
	PROC	*Proc=NULL;	// Kernel module anchor point.
	int	rc=0;
	char option=(argc == 2) ? ((argv[1][0] == '-') ? argv[1][1] : 'h'):'\0';
	if(option == 'h')
	{
		printf(	"usage:\t%s [-option]\n"		\
			"\t-q\tQuiet\n"				\
			"\t-i\tInfo\n"				\
			"\t-d\tDebug\n"				\
			"\t-h\tPrint out this message\n"	\
			"\nExit status:\n"			\
				"0\tif OK,\n"			\
				"1\tif problems,\n"		\
				">1\tif serious trouble.\n"	\
			"\nReport bugs to labs[at]cyring.fr\n", argv[0]);
		return 0;
	}
	else if(geteuid() == 0)
	{
	    if((fd.Drv=open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
	    {
		if((Proc=mmap(	NULL, PAGE_SIZE,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.Drv, 0)) != NULL)
		{
			switch(option)
			{
			case 'q':
				Quiet=0x000;
			break;
			case 'i':
				Quiet=0x011;
			break;
			case 'd':
				Quiet=0x111;
			break;
			default:
				Quiet=0x001;
			break;
			}
			SIG Sig={.Signal={0}, .TID=0, .Started=0};
			sigemptyset(&Sig.Signal);
			sigaddset(&Sig.Signal, SIGUSR1);
			sigaddset(&Sig.Signal, SIGINT);	// [CTRL] + [C]
			sigaddset(&Sig.Signal, SIGQUIT);
			sigaddset(&Sig.Signal, SIGTERM);

			if(!pthread_sigmask(SIG_BLOCK,&Sig.Signal, NULL)
			&& !pthread_create(&Sig.TID, NULL, Emergency, &Sig))
				Sig.Started=1;

			rc=Proc_Cycle(&fd, Proc);

			if(Sig.Started)
			{
				pthread_kill(Sig.TID, SIGUSR1);
				pthread_join(Sig.TID, NULL);
			}
			munmap(Proc, PAGE_SIZE);
		}
		else rc=3;

		close(fd.Drv);
	    }
	    else rc=2;
	} else rc=1;
	printf("You must be root or use sudo to launch this daemon\n");
	return(rc);
}
