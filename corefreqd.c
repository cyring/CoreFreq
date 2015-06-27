/*
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdlib.h>
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

unsigned int Shutdown=0x0;

struct {
	double	IPS,
		IPC,
		CPI,
		Turbo,
		C0,
		C3,
		C6,
		C7,
		C1;

	struct {
	double	Ratio,
		Freq;
	} Relative;

	unsigned long long Temperature;
} C[_MAX_CPU_];

typedef struct {
	PROC		*Proc;
	unsigned int	Bind;
	pthread_t	TID;
} ARG;


static void *Core_Cycle(void *arg)
{
	ARG *A=(ARG *) arg;
	PROC *P=A->Proc;
	unsigned int cpu=A->Bind;

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);

	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

	char comm[TASK_COMM_LEN];
	sprintf(comm, "corefreqd-%03d", cpu);
	pthread_setname_np(A->TID, comm);

	while(!Shutdown)
	{
		while(!atomic_load(&P->Core[cpu].Sync))
			usleep(P->msleep * 100);
		atomic_store(&P->Core[cpu].Sync, 0x0);

		// Compute IPS=Instructions per TSC
		C[cpu].IPS=	(double) (P->Core[cpu].Delta.INST)	\
				/ (double) (P->Core[cpu].Delta.TSC);

		// Compute IPC=Instructions per non-halted reference cycle.
		// (Protect against a division by zero)
		C[cpu].IPC=	(double) (P->Core[cpu].Delta.C0.URC != 0) ?	\
					(double) (P->Core[cpu].Delta.INST)	\
					/ (double) P->Core[cpu].Delta.C0.URC	\
						: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		C[cpu].CPI=	(double) (P->Core[cpu].Delta.INST != 0) ?	\
					(double) P->Core[cpu].Delta.C0.URC	\
					/ (double) (P->Core[cpu].Delta.INST)	\
						: 0.0f;

		// Compute Turbo State per Cycles Delta.
		// (Protect against a division by zero)
		C[cpu].Turbo=	(double) (P->Core[cpu].Delta.C0.URC != 0) ?	\
					(double) (P->Core[cpu].Delta.C0.UCC)	\
					/ (double) P->Core[cpu].Delta.C0.URC	\
						: 0.0f;

		// Compute C-States.
		C[cpu].C0=(double) (P->Core[cpu].Delta.C0.URC)	\
				/ (double) (P->Core[cpu].Delta.TSC);
		C[cpu].C3=(double) (P->Core[cpu].Delta.C3)	\
				/ (double) (P->Core[cpu].Delta.TSC);
		C[cpu].C6=(double) (P->Core[cpu].Delta.C6)	\
				/ (double) (P->Core[cpu].Delta.TSC);
		C[cpu].C7=(double) (P->Core[cpu].Delta.C7)	\
				/ (double) (P->Core[cpu].Delta.TSC);
		C[cpu].C1=(double) (P->Core[cpu].Delta.C1)	\
				/ (double) (P->Core[cpu].Delta.TSC);

		C[cpu].Relative.Ratio=	C[cpu].Turbo		\
					* C[cpu].C0		\
					* (double) P->Boost[1];

		// Relative Frequency = Relative Ratio x Bus Clock Frequency
		C[cpu].Relative.Freq=C[cpu].Relative.Ratio * P->Clock.Q;
		C[cpu].Relative.Freq+=(C[cpu].Relative.Ratio / P->Clock.R) / 1000000L;

		C[cpu].Temperature=P->Core[cpu].TjMax.Target		\
					- P->Core[cpu].ThermStat.DTS;
	}
	return(NULL);
}

typedef struct {
	sigset_t Signal;
	pthread_t TID;
	signed int Started;
} SIG;

static void *Emergency(void *arg)
{
	signed int caught=0;
	SIG *S=(SIG *) arg;
	pthread_setname_np(S->TID, "corefreqd-kill");

	while(!Shutdown && !sigwait(&S->Signal, &caught))
		switch(caught)
		{
			case SIGINT:
			case SIGQUIT:
			case SIGUSR1:
			case SIGTERM:
				Shutdown=1;
			break;
		}
	return(NULL);
}
/*
void	abstimespec(useconds_t usec, struct timespec *tsec)
{
	tsec->tv_sec=usec / 1000000L;
	tsec->tv_nsec=(usec % 1000000L) * 1000;
}

signed int	addtimespec(struct timespec *asec, const struct timespec *tsec)
{
	signed int rc=0;
	if((rc=clock_gettime(CLOCK_REALTIME, asec)) != -1)
	{
		if((asec->tv_nsec += tsec->tv_nsec) >= 1000000000L)
		{
			asec->tv_nsec -= 1000000000L;
			asec->tv_sec += 1;
		}
		asec->tv_sec += tsec->tv_sec;

		return(0);
	}
	else
		return(errno);
}
*/
signed int main(void)
{
	PROC *P=NULL;
	SIG S={.Signal={0}, .TID=0, .Started=0};
	uid_t UID=geteuid();

	if(UID == 0)
	{
		signed int  fd=open(SHM_FILENAME, O_RDWR);
		if(fd != -1)
		{
			P=mmap(	NULL, sizeof(PROC),
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd, 0x0);

			if(P != NULL)
			{
				unsigned int cpu=0;
				ARG *A=calloc(P->CPU.Count, sizeof(ARG));
				const char CLS[6+1]={27,'[','H',27,'[','J',0};

				sigemptyset(&S.Signal);
				sigaddset(&S.Signal, SIGINT);
				sigaddset(&S.Signal, SIGQUIT);
				sigaddset(&S.Signal, SIGUSR1);
				sigaddset(&S.Signal, SIGTERM);
				if(!pthread_sigmask(SIG_BLOCK, &S.Signal, NULL)
				&& !pthread_create(&S.TID, NULL, Emergency, &S))
					S.Started=1;

				for(cpu=0; cpu < P->CPU.Count; cpu++)
				    if(!P->Core[cpu].OffLine)
				    {
					A[cpu].Proc=P;
					A[cpu].Bind=cpu;
					pthread_create(	&A[cpu].TID,
							NULL,
							Core_Cycle,
							&A[cpu]);
				    }
				while(!Shutdown)
				{
					usleep(P->msleep * 100);

					printf("%s", CLS);
					for(cpu=0; cpu < P->CPU.Count; cpu++)
					    if(!P->Core[cpu].OffLine)
						printf("Core[%02d]%8.2f (%12.6f) MHz @ %llu°C\n",
							cpu,
							C[cpu].Relative.Freq,
							C[cpu].Relative.Ratio,
							C[cpu].Temperature);
					    else
						printf("Core[%02d] OffLine\n",
							cpu);
				}
				// shutting down
/*
				struct timespec absoluteTime={.tv_sec=0, .tv_nsec=0},
						gracePeriod={.tv_sec=0, .tv_nsec=0};
				abstimespec(4000000, &absoluteTime);
				if(!addtimespec(&gracePeriod, &absoluteTime)
				&& pthread_timedjoin_np(TID, NULL, &gracePeriod))
				{
					pthread_kill(TID, SIGKILL);
					pthread_join(TID, NULL);
				}
*/
				for(cpu=0; cpu < P->CPU.Count; cpu++)
					if(!P->Core[cpu].OffLine)
						pthread_join(A[cpu].TID, NULL);

				free(A);
				munmap(P, sizeof(PROC));

				if(S.Started)
				{
					pthread_kill(S.TID, SIGUSR1);
					pthread_join(S.TID, NULL);
				}
			}
			else
				printf("Error: mmap(fd:%d):KO\n", fd);

			close(fd);
		}
		else
			printf("Error: open('%s', O_RDWR):%d\n",
				SHM_FILENAME, errno);
	}
	return(0);
}
