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

#include "intelfreq.h"
#include "corefreq.h"

unsigned int Shutdown=0x0;

typedef struct {
	PROC		*Proc;
	CORE		*Core;
	SHM_STRUCT	*SHM;
	unsigned int	Bind;
	pthread_t	TID;
} ARG;


static void *Core_Cycle(void *arg)
{
	ARG *Arg=(ARG *) arg;
	PROC *Proc=Arg->Proc;
	CORE *Core=Arg->Core;
	SHM_STRUCT *SHM=Arg->SHM;
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
		SHM->CPU[cpu].IPS=	(double) (Core->Delta.INST)	\
				/ (double) (Core->Delta.TSC);

		// Compute IPC=Instructions per non-halted reference cycle.
		// (Protect against a division by zero)
		SHM->CPU[cpu].IPC=	(double) (Core->Delta.C0.URC != 0) ?	\
					(double) (Core->Delta.INST)	\
					/ (double) Core->Delta.C0.URC	\
						: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		SHM->CPU[cpu].CPI=	(double) (Core->Delta.INST != 0) ?	\
					(double) Core->Delta.C0.URC	\
					/ (double) (Core->Delta.INST)	\
						: 0.0f;

		// Compute Turbo State per Cycles Delta.
		// (Protect against a division by zero)
		SHM->CPU[cpu].Turbo=	(double) (Core->Delta.C0.URC != 0) ?	\
					(double) (Core->Delta.C0.UCC)	\
					/ (double) Core->Delta.C0.URC	\
						: 0.0f;

		// Compute C-States.
		SHM->CPU[cpu].C0=(double) (Core->Delta.C0.URC)	\
				/ (double) (Core->Delta.TSC);
		SHM->CPU[cpu].C3=(double) (Core->Delta.C3)	\
				/ (double) (Core->Delta.TSC);
		SHM->CPU[cpu].C6=(double) (Core->Delta.C6)	\
				/ (double) (Core->Delta.TSC);
		SHM->CPU[cpu].C7=(double) (Core->Delta.C7)	\
				/ (double) (Core->Delta.TSC);
		SHM->CPU[cpu].C1=(double) (Core->Delta.C1)	\
				/ (double) (Core->Delta.TSC);

		SHM->CPU[cpu].Relative.Ratio=SHM->CPU[cpu].Turbo		\
					* SHM->CPU[cpu].C0		\
					* (double) Proc->Boost[1];

		// Relative Frequency = Relative Ratio x Bus Clock Frequency
		SHM->CPU[cpu].Relative.Freq=SHM->CPU[cpu].Relative.Ratio * Proc->Clock.Q;
		SHM->CPU[cpu].Relative.Freq+=(SHM->CPU[cpu].Relative.Ratio / Proc->Clock.R) / 1000000L;

		SHM->CPU[cpu].Temperature=Core->TjMax.Target	\
					- Core->ThermStat.DTS;

		atomic_store(&SHM->CPU[cpu].Sync, 0x1);
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
	SHM_STRUCT	*SHM;

	SIG Sig={.Signal={0}, .TID=0, .Started=0};
	uid_t UID=geteuid();

	if(UID == 0)
	{
		if((FD.Drv=open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
		{
			unsigned int cpu=0;
			if((Proc=mmap(	NULL, sysconf(_SC_PAGESIZE),
					PROT_READ|PROT_WRITE, MAP_SHARED,
					FD.Drv, 0)) != NULL)
			{
				printf("mmap: Proc at %p\n", Proc);
				printf("CoreFreqd [%s]\n", Proc->Features.Brand);

				Core=calloc(Proc->CPU.Count, sizeof(Core));
				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
				{
					off_t offset=(1 + cpu) * sysconf(_SC_PAGESIZE);
					if((Core[cpu]=mmap(NULL, sysconf(_SC_PAGESIZE),
							PROT_READ|PROT_WRITE, MAP_SHARED,
							FD.Drv, offset)) != NULL)
						printf("mmap: CPU(%u) map at %p\n", cpu, Core[cpu]);
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
				&& ((SHM=mmap(0, ShmSize, PROT_READ|PROT_WRITE, MAP_SHARED, FD.Svr, 0))
					!= MAP_FAILED))
				{
					sigemptyset(&Sig.Signal);
					sigaddset(&Sig.Signal, SIGINT);
					sigaddset(&Sig.Signal, SIGQUIT);
					sigaddset(&Sig.Signal, SIGUSR1);
					sigaddset(&Sig.Signal, SIGTERM);
					if(!pthread_sigmask(SIG_BLOCK, &Sig.Signal, NULL)
					&& !pthread_create(&Sig.TID, NULL, Emergency, &Sig))
						Sig.Started=1;

					strncpy(SHM->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

					ARG *Arg=calloc(Proc->CPU.Count, sizeof(ARG));
					for(cpu=0; cpu < Proc->CPU.Count; cpu++)
					{
						atomic_init(&SHM->CPU[cpu].Sync, 0x0);

						if(!Core[cpu]->OffLine)
						{
							Arg[cpu].Proc=Proc;
							Arg[cpu].Core=Core[cpu];
							Arg[cpu].SHM=SHM;
							Arg[cpu].Bind=cpu;
							pthread_create(	&Arg[cpu].TID,
									NULL,
									Core_Cycle,
									&Arg[cpu]);
						}
					}
					free(Arg);

					while(!Shutdown)
					{
						usleep(Proc->msleep * 100);

						for(cpu=0; cpu < Proc->CPU.Count; cpu++)
						if(!Core[cpu]->OffLine
						&& atomic_load(&SHM->CPU[cpu].Sync))
						{
							atomic_store(&SHM->CPU[cpu].Sync, 0x0);

							printf("Core[%02d]%8.2f (%12.6f) MHz @ %llu°C\n",
								cpu,
								SHM->CPU[cpu].Relative.Freq,
								SHM->CPU[cpu].Relative.Ratio,
								SHM->CPU[cpu].Temperature);
						}
//					    else
//							printf("Core[%02d] OffLine\n",
//								cpu);
					}
					// shutting down
					for(cpu=0; cpu < Proc->CPU.Count; cpu++)
						if(!Core[cpu]->OffLine)
							pthread_join(Arg[cpu].TID, NULL);

					if(Sig.Started)
					{
						pthread_kill(Sig.TID, SIGUSR1);
						pthread_join(Sig.TID, NULL);
					}
					munmap(SHM, ShmSize);
					shm_unlink(SHM_FILENAME);
				}
				else
					printf("Error: creating the shared memory");

				for(cpu=0; cpu < Proc->CPU.Count; cpu++)
					if(Core[cpu])
					{
						printf("unmap: CPU(%u) at %p\n", cpu, Core[cpu]);
						munmap(Core[cpu], sysconf(_SC_PAGESIZE));
					}
				free(Core);
				if(Proc)
				{
					printf("unmap: Proc at %p\n", Proc);
					munmap(Proc, sysconf(_SC_PAGESIZE));
				}
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
