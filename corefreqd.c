/*
 * CoreFreq
 * Copyright (C) 2015 CYRIL INGENIERIE
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
#include "intelapi.h"

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
		Flip->Relative.Freq = Flip->Relative.Ratio * Proc->Clock.Q;
		Flip->Relative.Freq +=(Flip->Relative.Ratio * Proc->Clock.R) \
					/ ((double) Proc->Boost[1] * PRECISION);

		Flip->Temperature=Core->TjMax.Target - Core->ThermStat.DTS;

		if(Debug)
			printf("#%03u %7.2fMHz (%5.2f)\n", cpu,
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
		if((Core[cpu]=mmap(	NULL, PAGE_SIZE,
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
	{
	    if(((fd->Svr=shm_open(SHM_FILENAME,
				O_CREAT|O_TRUNC|O_RDWR,
				S_IRUSR|S_IWUSR				\
				|S_IRGRP|S_IWGRP			\
				|S_IROTH|S_IWOTH)) != -1)
	    && (ftruncate(fd->Svr, ShmSize) != -1)
	    && ((Shm=mmap(	0, ShmSize,
			PROT_READ|PROT_WRITE, MAP_SHARED,
			fd->Svr, 0)) != MAP_FAILED))
	    {
		memset(Shm, 0, ShmSize);

		Shm->Proc.msleep=Proc->msleep;

		Shm->Proc.CPU.Count=Proc->CPU.Count;
		Shm->Proc.CPU.OnLine=Proc->CPU.OnLine;

		Shm->Proc.Clock.Q=Proc->Clock.Q;
		Shm->Proc.Clock.R=Proc->Clock.R;

		strncpy(Shm->Proc.Architecture, Proc->Architecture, 32);
		memcpy(Shm->Proc.Boost, Proc->Boost,
			(1+1+8) * sizeof(unsigned int));

		Shm->Proc.PerCore=Proc->PerCore;
		strncpy(Shm->Proc.Brand, Proc->Features.Brand, 48);

		double Clock=Shm->Proc.Clock.Q				\
				+ ((double) Shm->Proc.Clock.R		\
				/ (Shm->Proc.Boost[1] * PRECISION));

		printf("CoreFreqd [%s] , Clock @ %f MHz\n",
			Shm->Proc.Brand, Clock);

		strncpy(Shm->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

		BITCLR(Shm->Proc.Sync, 0);

		unsigned long long roomSeed=0x0;
		for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
			if(!(Shm->Cpu[cpu].OffLine=Core[cpu]->OffLine))
			{
				BITSET(Shm->Proc.Room, cpu);
				BITSET(roomSeed, cpu);
			}

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
		while(!Shutdown)
		{
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
				{
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

//					BITSET(Shm->Proc.Room, cpu);
				}
			Shm->Proc.Room=~(1 << Shm->Proc.CPU.Count);

			Shm->Proc.Avg.Turbo/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C0/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C3/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C6/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C7/=Shm->Proc.CPU.OnLine;
			Shm->Proc.Avg.C1/=Shm->Proc.CPU.OnLine;

			BITSET(Shm->Proc.Sync, 0);
		}
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

int Proc_Topology(FD *fd, PROC *Proc)
{
	unsigned int	cpu=0, level=0x0;
	CORE		**Core;
	int		rc=0;

	Core=calloc(Proc->CPU.Count, sizeof(Core));
	for(cpu=0; !rc && (cpu < Proc->CPU.Count); cpu++)
	{
		off_t offset=(1 + cpu) * PAGE_SIZE;
		if((Core[cpu]=mmap(	NULL, PAGE_SIZE,
				PROT_READ|PROT_WRITE,
				MAP_SHARED,
				fd->Drv, offset)) == NULL)
			rc=4;
	}
	if(!rc)
	{
		double Clock=Proc->Clock.Q				\
				+ ((double)Proc->Clock.R		\
				/ (Proc->Boost[1] * PRECISION));

		printf(	"CoreFreqd [%s]\n"				\
			"Signature [%1X%1X_%1X%1X]"			\
			" Architecture [%s]\n"				\
			"%u/%u CPU Online"				\
			" , Clock @ %f MHz\n\n"				\
			"CPU       ApicID CoreID ThreadID"		\
			" x2APIC Enable Caches L1 L2 L3\n",
			Proc->Features.Brand,
			Proc->Features.Std.AX.ExtFamily,
			Proc->Features.Std.AX.Family,
			Proc->Features.Std.AX.ExtModel,
			Proc->Features.Std.AX.Model,
			Proc->Architecture,
			Proc->CPU.OnLine,
			Proc->CPU.Count,
			Clock);

		for(cpu=0; cpu < Proc->CPU.Count; cpu++)
		{
			printf(	"#%02u%-5s  %6d %6d   %6d"		\
				"    %3s    %c   ",
				cpu,
				(Core[cpu]->T.Base.BSP) ? "(BSP)" : "(AP)",
				Core[cpu]->T.ApicID,
				Core[cpu]->T.CoreID,
				Core[cpu]->T.ThreadID,
				(Core[cpu]->T.Base.EXTD) ? "ON" : "OFF",
				(Core[cpu]->T.Base.EN) ? 'Y' : 'N');
			for(level=0; level < CACHE_MAX_LEVEL; level++)
				printf(" %-u", Core[cpu]->T.Cache[level].Size);
			printf("\n");
		}
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
	char option=(argc == 2) && (argv[1][0] == '-') ? argv[1][1] : 'h';

	uid_t UID=geteuid();

	if(UID == 0)
	{
	    if((fd.Drv=open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
	    {
		if((Proc=mmap(	NULL, PAGE_SIZE,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.Drv, 0)) != NULL)
		{
		  switch(option)
		  {
		    case 't':
			rc=Proc_Topology(&fd, Proc);
		    break;
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
