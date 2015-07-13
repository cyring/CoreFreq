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
	struct stat shmStat={0}, smbStat={0};
	SHM_STRUCT *Shm;
	unsigned int cpu=0;
	int fd=-1, rc=0;

	if(((fd=shm_open(SHM_FILENAME,
			O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)) != -1)
	&& ((fstat(fd, &shmStat) != -1)
	&& ((Shm=mmap(0, shmStat.st_size,
		      PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) != MAP_FAILED)))
	{
		printf("CoreFreq-Cli [%s]\n", Shm->Proc.Brand);

		signal(SIGINT, Emergency);
		signal(SIGQUIT, Emergency);
		signal(SIGTERM, Emergency);

		while(!Shutdown)
		{
			usleep(Shm->Proc.msleep * 250);

			for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
			if(!Shm->Cpu[cpu].OffLine
			&& atomic_load(&Shm->Cpu[cpu].Sync))
			{
				atomic_store(&Shm->Cpu[cpu].Sync, 0x0);

				printf("Core[%02d]%12.6f x %u = %8.2f MHz  @  %llu°C\n",
					cpu,
					Shm->Cpu[cpu].Relative.Ratio,
					Shm->Proc.Clock.Q,
					Shm->Cpu[cpu].Relative.Freq,
					Shm->Cpu[cpu].Temperature);
			}
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
