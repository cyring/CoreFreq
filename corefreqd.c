/*
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
#include <stdatomic.h>

#include "corefreq.h"
#include "intelfreq.h"
#define	PAGE_SIZE (sysconf(_SC_PAGESIZE))

unsigned int Shutdown=0x0;

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
	sprintf(comm, "corefreqd-%03d", cpu);
	pthread_setname_np(Arg->TID, comm);

	while(!Shutdown)
	{
		while(!atomic_load(&Core->Sync))
			usleep(Proc->msleep * 100);
		atomic_store(&Core->Sync, 0x0);

		// Compute IPS=Instructions per TSC
		Cpu->IPS=	(double) (Core->Delta.INST)	\
				/ (double) (Core->Delta.TSC);

		// Compute IPC=Instructions per non-halted reference cycle.
		// (Protect against a division by zero)
		Cpu->IPC=	(double) (Core->Delta.C0.URC != 0) ?	\
					(double) (Core->Delta.INST)	\
					/ (double) Core->Delta.C0.URC	\
						: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		Cpu->CPI=	(double) (Core->Delta.INST != 0) ?	\
					(double) Core->Delta.C0.URC	\
					/ (double) (Core->Delta.INST)	\
						: 0.0f;

		// Compute Turbo State per Cycles Delta.
		// (Protect against a division by zero)
		Cpu->Turbo=	(double) (Core->Delta.C0.URC != 0) ?	\
					(double) (Core->Delta.C0.UCC)	\
					/ (double) Core->Delta.C0.URC	\
						: 0.0f;

		// Compute C-States.
		Cpu->C0=(double) (Core->Delta.C0.URC)	\
				/ (double) (Core->Delta.TSC);
		Cpu->C3=(double) (Core->Delta.C3)	\
				/ (double) (Core->Delta.TSC);
		Cpu->C6=(double) (Core->Delta.C6)	\
				/ (double) (Core->Delta.TSC);
		Cpu->C7=(double) (Core->Delta.C7)	\
				/ (double) (Core->Delta.TSC);
		Cpu->C1=(double) (Core->Delta.C1)	\
				/ (double) (Core->Delta.TSC);

		Cpu->Relative.Ratio=Cpu->Turbo		\
					* Cpu->C0		\
					* (double) Proc->Boost[1];

		// Relative Frequency = Relative Ratio x Bus Clock Frequency
		Cpu->Relative.Freq=Cpu->Relative.Ratio * Proc->Clock.Q;
		Cpu->Relative.Freq+=(Cpu->Relative.Ratio / Proc->Clock.R) / 1000000L;

		Cpu->Temperature=Core->TjMax.Target	\
					- Core->ThermStat.DTS;

		atomic_store(&Cpu->Sync, 0x1);
	}
	return(NULL);
}

typedef struct {
	sigset_t Signal;
	pthread_t TID;
	int Started;
} SIG;

static void *Emergency(void *arg)
{
	int caught=0;
	SIG *Sig=(SIG *) arg;
	pthread_setname_np(Sig->TID, "corefreqd-kill");

	while(!Shutdown && !sigwait(&Sig->Signal, &caught))
		switch(caught)
		{
			case SIGINT:
			case SIGQUIT:
			case SIGUSR1:
			case SIGTERM:
				Shutdown=0x1;
			break;
		}
	return(NULL);
}

int main(void)
{
	struct
	{
		int	Drv,
			Svr;
	} FD;

	PROC		*Proc=NULL;
	CORE		**Core;
	SHM_STRUCT	*Shm;

	SIG Sig={.Signal={0}, .TID=0, .Started=0};
	uid_t UID=geteuid();

	if(UID == 0)
	{
		if((FD.Drv=open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
		{
			unsigned int cpu=0;
			if((Proc=mmap(	NULL, PAGE_SIZE,
					PROT_READ|PROT_WRITE, MAP_SHARED,
					FD.Drv, 0)) != NULL)
			{
				Core=calloc(Proc->CPU.Count, sizeof(Core));
				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				{
					off_t offset=(1 + cpu) * PAGE_SIZE;
					Core[cpu]=mmap(NULL, PAGE_SIZE,
							PROT_READ|PROT_WRITE, MAP_SHARED,
							FD.Drv, offset);
				}
				
				size_t ShmSize, ShmCpuSize;
				ShmCpuSize=sizeof(CPU_STRUCT) * Proc->CPU.Count;
				ShmSize=sizeof(SHM_STRUCT) + ShmCpuSize;
				ShmSize=PAGE_SIZE * ((ShmSize / PAGE_SIZE)
					+ ((ShmSize % PAGE_SIZE) ? 1 : 0));

				umask(!S_IRUSR|!S_IWUSR|!S_IRGRP|!S_IWGRP|!S_IROTH|!S_IWOTH);

				if(((FD.Svr=shm_open(SHM_FILENAME,
					O_CREAT|O_TRUNC|O_RDWR,
					S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1)
				&& (ftruncate(FD.Svr, ShmSize) != -1)
				&& ((Shm=mmap(0, ShmSize,
					      PROT_READ|PROT_WRITE, MAP_SHARED, FD.Svr, 0)) != MAP_FAILED))
				{
					Shm->Proc.msleep=Proc->msleep;
					Shm->Proc.CPU.Count=Proc->CPU.Count;
					Shm->Proc.CPU.OnLine=Proc->CPU.OnLine;
					memcpy(Shm->Proc.Boost, Proc->Boost,
					       (1+1+8) * sizeof(unsigned int));
					Shm->Proc.PerCore=Proc->PerCore;
					Shm->Proc.Clock.Q=Proc->Clock.Q;
					Shm->Proc.Clock.R=Proc->Clock.R;
					strncpy(Shm->Proc.Brand, Proc->Features.Brand, 48);
					
					printf("CoreFreqd [%s]\n", Shm->Proc.Brand);

					sigemptyset(&Sig.Signal);
					sigaddset(&Sig.Signal, SIGINT);
					sigaddset(&Sig.Signal, SIGQUIT);
					sigaddset(&Sig.Signal, SIGUSR1);
					sigaddset(&Sig.Signal, SIGTERM);
					if(!pthread_sigmask(SIG_BLOCK, &Sig.Signal, NULL)
					&& !pthread_create(&Sig.TID, NULL, Emergency, &Sig))
						Sig.Started=1;

					strncpy(Shm->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

					ARG *Arg=calloc(Shm->Proc.CPU.Count, sizeof(ARG));
					for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
					{
						atomic_init(&Shm->Cpu[cpu].Sync, 0x0);

						if(!(Shm->Cpu[cpu].OffLine=Core[cpu]->OffLine))
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
					}
					free(Arg);

					while(!Shutdown)
						usleep(Shm->Proc.msleep * 1000);

					// shutting down
					for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
						if(!Shm->Cpu[cpu].OffLine)
							pthread_join(Arg[cpu].TID, NULL);

					if(Sig.Started)
					{
						pthread_kill(Sig.TID, SIGUSR1);
						pthread_join(Sig.TID, NULL);
					}
					munmap(Shm, ShmSize);
					shm_unlink(SHM_FILENAME);
				}
				else
					printf("Error: creating the shared memory");

				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
					if(Core[cpu])
						munmap(Core[cpu], PAGE_SIZE);
				free(Core);
				if(Proc)
					munmap(Proc, PAGE_SIZE);
			}
			else
				printf("Error: mmap(fd:%d):KO\n", FD.Drv);

			close(FD.Drv);
		}
		else
			printf("Error: open('%s', O_RDWR):%d\n",
				DRV_FILENAME, errno);
	}
	return(0);
}
