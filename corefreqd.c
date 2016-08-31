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

unsigned int Shutdown=0x0, Debug=0x0;

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
		usleep(Proc->msleep * 100);
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
					/ (double) Core->Delta.C0.URC	\
				: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		Flip->State.CPI	= (double) (Core->Delta.INST != 0) ?	\
				  (double) Core->Delta.C0.URC		\
					/ (double) (Core->Delta.INST)	\
				: 0.0f;

		// Compute Turbo State per Cycles Delta.
		// (Protect against a division by zero)
		Flip->State.Turbo=(double) (Core->Delta.C0.URC!=0) ?	\
				  (double) (Core->Delta.C0.UCC)		\
					/ (double) Core->Delta.C0.URC	\
				: 0.0f;

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

		Flip->Relative.Ratio	= Flip->State.Turbo		\
					* Flip->State.C0		\
					* (double) Proc->Boost[1];

		// Relative Frequency = Relative Ratio x Bus Clock Frequency
		Flip->Relative.Freq=(double) REL_FREQ(	Proc->Boost[1], \
							Flip->Relative.Ratio, \
							Proc->Clock) / 1000000L;

		Flip->Temperature=Core->TjMax.Target - Core->ThermStat.DTS;

		if(Debug)
			printf("#%03u %.2f Hz (%5.2f)\n", cpu,
					Flip->Relative.Freq,
					Flip->Relative.Ratio);
	    }
	} while(!Shutdown) ;

	return(NULL);
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
		// Copy global attributes.
		Shm->Proc.msleep=Proc->msleep;

		Shm->Proc.CPU.Count=Proc->CPU.Count;
		Shm->Proc.CPU.OnLine=Proc->CPU.OnLine;
		// Copy the Base Clock.
		Shm->Proc.Clock.Q=Proc->Clock.Q;
		Shm->Proc.Clock.R=Proc->Clock.R;
		Shm->Proc.Clock.Hz=Proc->Clock.Hz;
		// Copy the Architecture name.
		strncpy(Shm->Proc.Architecture, Proc->Architecture, 32);
		memcpy(Shm->Proc.Boost, Proc->Boost,
			(1+1+8) * sizeof(unsigned int));
		// Copy the operational mode.
		Shm->Proc.PerCore=Proc->PerCore;
		strncpy(Shm->Proc.Brand, Proc->Features.Brand, 48);

		// Welcomes with brand and bclk.
		printf("CoreFreq Daemon [%s] Frequency @ %llu Hz\n",	\
					Shm->Proc.Brand,		\
			REL_FREQ(	Shm->Proc.Boost[1],		\
					Shm->Proc.Boost[1],		\
					Shm->Proc.Clock));

		// Store the application name.
		strncpy(Shm->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

		// Initialize notification.
		BITCLR(Shm->Proc.Sync, 0);

		// Copy per CPU data from Kernel to Userspace.
		unsigned long long roomSeed=0x0;
		for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
		{	// Copy Core topology.
			Shm->Cpu[cpu].Topology.BSP=(Core[cpu]->T.Base.BSP) ? 1 : 0;
			Shm->Cpu[cpu].Topology.ApicID=Core[cpu]->T.ApicID;
			Shm->Cpu[cpu].Topology.CoreID=Core[cpu]->T.CoreID;
			Shm->Cpu[cpu].Topology.ThreadID=Core[cpu]->T.ThreadID;
			Shm->Cpu[cpu].Topology.x2APIC=(Core[cpu]->T.Base.EXTD) ? 1 : 0;
			Shm->Cpu[cpu].Topology.Enable=(Core[cpu]->T.Base.EN) ? 1 : 0;
			// Compute Caches size.
			unsigned int level=0x0;
			for(level=0; level < CACHE_MAX_LEVEL; level++)
			{
				Shm->Cpu[cpu].Topology.Cache[level].Size=
				  (Core[cpu]->T.Cache[level].Sets + 1)
				* (Core[cpu]->T.Cache[level].Linez + 1)
				* (Core[cpu]->T.Cache[level].Parts + 1)
				* (Core[cpu]->T.Cache[level].Ways + 1);
			}
			// Define the Clients room mask.
			if(!(Shm->Cpu[cpu].OffLine=Core[cpu]->OffLine))
			{
				BITSET(Shm->Proc.Room, cpu);
				BITSET(roomSeed, cpu);
			}
		}
		// Launch one Server thread per online CPU.
		ARG *Arg=calloc(Shm->Proc.CPU.Count,
				sizeof(ARG));
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
			}
		// Main loop : aggregate the ratios.
		while(!Shutdown)
		{	// Wait until all the rooms & mask are cleared.
			while(!Shutdown && BITWISEAND(Shm->Proc.Room,roomSeed))
				usleep(Shm->Proc.msleep * 100);

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
	FD		fd={0, 0};
	PROC		*Proc=NULL;
	int		rc=0;
	char option=(argc == 2) ? ((argv[1][0] == '-') ? argv[1][1] : 'h'):'\0';
	if(option == 'h')
		printf(	"usage:\t%s [-option]\n"		\
			"\t-d\tDebug\n"				\
			"\t-h\tPrint out this message\n"	\
			"\nExit status:\n"			\
				"0\tif OK,\n"			\
				"1\tif problems,\n"		\
				">1\tif serious trouble.\n"	\
			"\nReport bugs to labs[at]cyring.fr\n", argv[0]);

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
		    case 'd':
			Debug=0x1;
		    default:
		    {
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
		    }
		    break;
		  }
		  munmap(Proc, PAGE_SIZE);
		}
		else rc=3;

		close(fd.Drv);
	    }
	    else rc=2;
	} else rc=1;
	return(rc);
}
