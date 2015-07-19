/*
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdatomic.h>

#include "corefreq.h"

unsigned int Shutdown=0x0;

void Emergency(int caught)
{
	switch(caught)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			Shutdown=0x1;
		break;
	}
}

int main(int argc, char *argv[])
{
	struct stat shmStat={0};
	SHM_STRUCT *Shm;
	unsigned int cpu=0;
	int fd=-1, rc=0;

	if(((fd=shm_open(SHM_FILENAME,
			O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1)
	&& ((fstat(fd, &shmStat) != -1)
	&& ((Shm=mmap(0, shmStat.st_size,
		      PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) != MAP_FAILED)))
	{
		double Clock=Shm->Proc.Clock.Q				\
				+ ((double) Shm->Proc.Clock.R		\
				/ (Shm->Proc.Boost[1] * 1000000L));

		printf("CoreFreq-Cli [%s] , Clock @ %.2f MHz\n\n",
		       Shm->Proc.Brand, Clock);

		signal(SIGINT, Emergency);
		signal(SIGQUIT, Emergency);
		signal(SIGTERM, Emergency);

		while(!Shutdown)
		{
			while(!atomic_load(&Shm->Proc.Sync))
				usleep(Shm->Proc.msleep * 100);
			atomic_store(&Shm->Proc.Sync, 0x0);

			for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
			    if(!Shm->Cpu[cpu].OffLine)
			    {
				unsigned int flop=!Shm->Cpu[cpu].FlipFlop;
				printf(	"Cpu#%02d %7.2fMHz (%5.2f)" \
					" %6.2f%% %6.2f%% %6.2f%% %6.2f%% %6.2f%% %6.2f%%" \
					" @ %llu°C\n",
					cpu,
					Shm->Cpu[cpu].Relative.Freq,
					Shm->Cpu[cpu].Relative.Ratio,
					100.f * Shm->Cpu[cpu].State[flop].Turbo,
					100.f * Shm->Cpu[cpu].State[flop].C0,
					100.f * Shm->Cpu[cpu].State[flop].C1,
					100.f * Shm->Cpu[cpu].State[flop].C3,
					100.f * Shm->Cpu[cpu].State[flop].C6,
					100.f * Shm->Cpu[cpu].State[flop].C7,
					Shm->Cpu[cpu].Temperature);
			    }
			printf(	"\nAverage C-states\n" \
				"Turbo\t  C0\t  C1\t  C3\t  C6\t  C7\n" \
				"%6.2f%%\t%6.2f%%\t%6.2f%%\t%6.2f\t%6.2f%%\t%6.2f%%\n\n",
				100.f * Shm->Proc.Avg.Turbo,
				100.f * Shm->Proc.Avg.C0,
				100.f * Shm->Proc.Avg.C1,
				100.f * Shm->Proc.Avg.C3,
				100.f * Shm->Proc.Avg.C6,
				100.f * Shm->Proc.Avg.C7);
		}
		if(munmap(Shm, shmStat.st_size) == -1)
			printf("Error: unmapping the shared memory");
		if(close(fd) == -1)
			printf("Error: closing the shared memory");
	}
	else
		rc=-1;
	return(rc);
}
