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

#include "intelfreq.h"

unsigned int Shutdown=0x0;

static void *Core_Temp(void *arg)
{
	PROC *P=(PROC *) arg;
	unsigned int cpu;

	while(!Shutdown)
	{
		for(cpu=0; cpu < P->CPU.Count; cpu++)
		{
			P->Core[cpu].Temp=P->Core[cpu].TjMax.Target	\
						- P->Core[cpu].ThermStat.DTS;

			printf("\tCore(%02d) @ %d°C\n", cpu, P->Core[cpu].Temp);
		}	printf("\n");
		usleep(P->msleep * 10000);
	}
	return(NULL);
}

static void *Core_Freq(void *arg)
{
	PROC *P=(PROC *) arg;
	unsigned int cpu;

	while(!Shutdown)
	{
	  for(cpu=0; cpu < P->CPU.Count; cpu++)
	  {
	// Compute IPS=Instructions per TSC
	double IPS=	(double) (P->Core[cpu].Delta.INST)	\
			/ (double) (P->Core[cpu].Delta.TSC);

	// Compute IPC=Instructions per non-halted reference cycle.
	// (Protect against a division by zero)
	double IPC=	(double) (P->Core[cpu].Delta.C0.URC != 0) ?	\
				(double) (P->Core[cpu].Delta.INST)	\
				/ (double) P->Core[cpu].Delta.C0.URC	\
					: 0.0f;

	// Compute CPI=Non-halted reference cycles per instruction.
	// (Protect against a division by zero)
	double CPI=	(double) (P->Core[cpu].Delta.INST != 0) ?	\
				(double) P->Core[cpu].Delta.C0.URC	\
				/ (double) (P->Core[cpu].Delta.INST)	\
					: 0.0f;

	// Compute Turbo State per Cycles Delta.
	// (Protect against a division by zero)
	double Turbo=	(double) (P->Core[cpu].Delta.C0.URC != 0) ?	\
				(double) (P->Core[cpu].Delta.C0.UCC)	\
				/ (double) P->Core[cpu].Delta.C0.URC	\
					: 0.0f;

	// Compute C-States.
	double C0=(double) (P->Core[cpu].Delta.C0.URC)	\
			/ (double) (P->Core[cpu].Delta.TSC);
	double C3=(double) (P->Core[cpu].Delta.C3)	\
			/ (double) (P->Core[cpu].Delta.TSC);
	double C6=(double) (P->Core[cpu].Delta.C6)	\
			/ (double) (P->Core[cpu].Delta.TSC);
	double C7=(double) (P->Core[cpu].Delta.C7)	\
			/ (double) (P->Core[cpu].Delta.TSC);
	double C1=(double) (P->Core[cpu].Delta.C1)	\
			/ (double) (P->Core[cpu].Delta.TSC);

	// Relative Frequency = Relative Ratio x Bus Clock Frequency
	double RelativeRatio=Turbo * C0 * (double) P->Boost[1];
	double RelativeFreq=RelativeRatio * P->Clock;

		printf("\tCore(%02d) @ %7.2f MHz\n", cpu, RelativeFreq);
	  }	printf("\n");
	  usleep(P->msleep * 10000);
	}
	return(NULL);
}

typedef struct {
	sigset_t Signal;
	pthread_t TID;
	int Started;
} ARG;

static void *Emergency(void *arg)
{
	int caught=0;
	ARG *A=(ARG *) arg;
	pthread_setname_np(A->TID, "corefreqd-kill");

	while(!Shutdown && !sigwait(&A->Signal, &caught))
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

void	abstimespec(useconds_t usec, struct timespec *tsec)
{
	tsec->tv_sec=usec / 1000000L;
	tsec->tv_nsec=(usec % 1000000L) * 1000;
}

int	addtimespec(struct timespec *asec, const struct timespec *tsec)
{
	int rc=0;
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

int main(void)
{
	PROC *P=NULL;
	pthread_t TID;
	ARG A={.Signal={0}, .TID=0, .Started=0};
	uid_t UID=geteuid();

	if(UID == 0)
	{
		int  fd=open(SHM_FILENAME, O_RDWR);
		if(fd != -1)
		{
			P=mmap(	NULL, sizeof(PROC),
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd, 0x0);

			if(P != NULL)
			{
				sigemptyset(&A.Signal);
				sigaddset(&A.Signal, SIGINT);
				sigaddset(&A.Signal, SIGQUIT);
				sigaddset(&A.Signal, SIGUSR1);
				sigaddset(&A.Signal, SIGTERM);
				if(!pthread_sigmask(SIG_BLOCK, &A.Signal, NULL)
				&& !pthread_create(&A.TID, NULL, Emergency, &A))
					A.Started=1;

				if(!pthread_create(&TID, NULL, Core_Freq, P))
				{
					while(!Shutdown)
					{
						usleep(10000);
					}
					// shutting down
					struct timespec absoluteTime={.tv_sec=0, .tv_nsec=0},
							gracePeriod={.tv_sec=0, .tv_nsec=0};
					abstimespec(4000000, &absoluteTime);
					if(!addtimespec(&gracePeriod, &absoluteTime)
					&& pthread_timedjoin_np(TID, NULL, &gracePeriod))
					{
						pthread_kill(TID, SIGKILL);
						pthread_join(TID, NULL);
					}
				}
				munmap(P, sizeof(PROC));

				if(A.Started)
				{
					pthread_kill(A.TID, SIGUSR1);
					pthread_join(A.TID, NULL);
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
