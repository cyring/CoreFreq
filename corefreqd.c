/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
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

unsigned int Shutdown=0x0, Quiet=0x001, Math=0x0;
static unsigned long long roomSeed=0x0;

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
	unsigned int cpu=Arg->Bind,
	thermalFormula=!strncmp(Proc->Features.Info.VendorID, VENDOR_INTEL, 12)?
	0x01 : !strncmp(Proc->Features.Info.VendorID, VENDOR_AMD, 12) ?
		0x10 : 0x0;
	pthread_t tid=pthread_self();

	cpu_set_t affinity;
	cpu_set_t cpuset;
	CPU_ZERO(&affinity);
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);

	if(!pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset))
	{
		CPU_OR(&affinity, &affinity, &cpuset);
	}
	char comm[TASK_COMM_LEN];
	sprintf(comm, "corefreqd/%-3d", cpu);
	pthread_setname_np(tid, comm);

	if(Quiet & 0x100){
		printf("    Thread [%p] Init CPU %03u\n", tid, cpu);
		fflush(stdout);
	}
	BITSET(BUS_LOCK, roomSeed, cpu);
	BITSET(BUS_LOCK, Proc->Room, cpu);
	do
	{
	    while( !BITVAL(Core->Sync.V, 63)
		&& !Shutdown
		&& !Core->OffLine.OS )
		{	// Exit function if the thread lost its cpu affinity.
		    CPU_ZERO(&cpuset);
		    if(!pthread_getaffinity_np(tid, sizeof(cpu_set_t), &cpuset)
		    && CPU_EQUAL(&cpuset, &affinity))
			usleep(Proc->SleepInterval * BASE_SLEEP);
		    else
			break;
		}
	    BITCLR(LOCKLESS, Core->Sync.V, 63);

	    if(!Shutdown && !Core->OffLine.OS && CPU_EQUAL(&affinity,&cpuset))
	    {
		if(BITVAL(Proc->Room, cpu))
		{
			Cpu->Toggle=!Cpu->Toggle;
			BITCLR(BUS_LOCK, Proc->Room, cpu);
		}
		struct FLIP_FLOP *Flip=&Cpu->FlipFlop[Cpu->Toggle];

		Flip->Delta.INST	= Core->Delta.INST;
		Flip->Delta.C0.UCC	= Core->Delta.C0.UCC;
		Flip->Delta.C0.URC	= Core->Delta.C0.URC;
		Flip->Delta.C3		= Core->Delta.C3;
		Flip->Delta.C6		= Core->Delta.C6;
		Flip->Delta.C7		= Core->Delta.C7;
		Flip->Delta.TSC		= Core->Delta.TSC;
		Flip->Delta.C1		= Core->Delta.C1;

		// Compute IPS=Instructions per TSC
		Flip->State.IPS	= (double) (Flip->Delta.INST)		\
				/ (double) (Flip->Delta.TSC);

		// Compute IPC=Instructions per non-halted reference cycle.
		// (Protect against a division by zero)
		Flip->State.IPC	= (double) (Flip->Delta.C0.URC != 0) ?	\
				  (double) (Flip->Delta.INST)		\
				/ (double) Flip->Delta.C0.URC		\
				: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		Flip->State.CPI	= (double) (Flip->Delta.INST != 0) ?	\
				  (double) Flip->Delta.C0.URC		\
				/ (double) (Flip->Delta.INST)		\
				: 0.0f;

		// Compute Turbo State.
		Flip->State.Turbo=(double) (Flip->Delta.C0.UCC)		\
				/ (double) (Flip->Delta.TSC);
		// Compute C-States.
		Flip->State.C0	= (double) (Flip->Delta.C0.URC)		\
				/ (double) (Flip->Delta.TSC);
		Flip->State.C3	= (double) (Flip->Delta.C3)		\
				/ (double) (Flip->Delta.TSC);
		Flip->State.C6	= (double) (Flip->Delta.C6)		\
				/ (double) (Flip->Delta.TSC);
		Flip->State.C7	= (double) (Flip->Delta.C7)		\
				/ (double) (Flip->Delta.TSC);
		Flip->State.C1	= (double) (Flip->Delta.C1)		\
				/ (double) (Flip->Delta.TSC);
		// Relative Ratio formula.
		Flip->Relative.Ratio	= (double) (Flip->Delta.C0.UCC	\
						* Proc->Boost[1])	\
					/ (double) (Flip->Delta.TSC);

		if(!Math && Core->Query.Turbo)
		{// Relative Frequency equals UCC per second.
			Flip->Relative.Freq=(double) (Flip->Delta.C0.UCC)
						/ (Proc->SleepInterval * 1000);
		}
		else
		{// Relative Frequency = Relative Ratio x Bus Clock Frequency
		  Flip->Relative.Freq=
		  (double) REL_FREQ(Proc->Boost[1],			\
				    Flip->Relative.Ratio,		\
				    Core->Clock, Proc->SleepInterval)
					/ (Proc->SleepInterval * 1000);
		}
		Flip->Thermal.Trip=Core->PowerThermal.Trip;
		Flip->Thermal.Sensor=Core->PowerThermal.Sensor;

		if(thermalFormula == 0x01)
		    Flip->Thermal.Temp=Cpu->PowerThermal.Target		\
					- Flip->Thermal.Sensor;
	    else
		if(thermalFormula == 0x10)
		    Flip->Thermal.Temp=Flip->Thermal.Sensor		\
					- (Cpu->PowerThermal.Target * 2) - 49;

	    if(Flip->Thermal.Temp < Cpu->PowerThermal.Limit[0])
		Cpu->PowerThermal.Limit[0]=Flip->Thermal.Temp;
	    if(Flip->Thermal.Temp > Cpu->PowerThermal.Limit[1])
		Cpu->PowerThermal.Limit[1]=Flip->Thermal.Temp;
	    }
	} while(!Shutdown && !Core->OffLine.OS && CPU_EQUAL(&affinity,&cpuset));
	BITCLR(BUS_LOCK, Proc->Room, cpu);
	BITCLR(BUS_LOCK, roomSeed, cpu);

	if(Quiet & 0x100) {
		printf("    Thread [%p] %s CPU %03u\n", tid,
			CPU_EQUAL(&affinity, &cpuset) ? "Exit" : "Lost", cpu);
		fflush(stdout);
	}
	return(NULL);
}

void Architecture(SHM_STRUCT *Shm, PROC *Proc)
{
	// Copy all BSP features.
	memcpy(&Shm->Proc.Features, &Proc->Features, sizeof(FEATURES));
	// Copy the numbers of total & online CPU.
	Shm->Proc.CPU.Count=Proc->CPU.Count;
	Shm->Proc.CPU.OnLine=Proc->CPU.OnLine;
	// Copy the Architecture name.
	strncpy(Shm->Proc.Architecture, Proc->Architecture, 32);
	// Copy the base clock ratios.
	memcpy(Shm->Proc.Boost, Proc->Boost, (1+1+8) * sizeof(unsigned int));
	// Copy the processor's brand string.
	strncpy(Shm->Proc.Brand, Proc->Features.Info.Brand, 48);
}

void InvariantTSC(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.InvariantTSC=(Proc->Features.Std.DX.TSC
				<< Proc->Features.AdvPower.DX.Inv_TSC);
}

void PerformanceMonitoring(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.PM_version=Proc->Features.PerfMon.AX.Version;
}

void HyperThreading(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.HyperThreading=Proc->Features.HTT_Enable;
}

void PowerNow(SHM_STRUCT *Shm, PROC *Proc)
{
	if(!strncmp(Proc->Features.Info.VendorID, VENDOR_AMD, 12))
	{
		if(Proc->Features.AdvPower.DX.FID)
			BITSET(LOCKLESS, Shm->Proc.PowerNow, 0);
		else
			BITCLR(LOCKLESS, Shm->Proc.PowerNow, 0);

		if(Proc->Features.AdvPower.DX.VID)
			BITSET(LOCKLESS, Shm->Proc.PowerNow, 1);
		else
			BITCLR(LOCKLESS, Shm->Proc.PowerNow, 1);
	}
	else
		Shm->Proc.PowerNow=0;
}

void BaseClock(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{	// Copy per CPU base clock.
	Shm->Cpu[cpu].Clock.Q=Core[cpu]->Clock.Q;
	Shm->Cpu[cpu].Clock.R=Core[cpu]->Clock.R;
	Shm->Cpu[cpu].Clock.Hz=Core[cpu]->Clock.Hz;
}

void Topology(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{	// Copy Core topology.
	Shm->Cpu[cpu].Topology.MP.BSP=(Core[cpu]->T.Base.BSP) ? 1 : 0;
	Shm->Cpu[cpu].Topology.ApicID=Core[cpu]->T.ApicID;
	Shm->Cpu[cpu].Topology.CoreID=Core[cpu]->T.CoreID;
	Shm->Cpu[cpu].Topology.ThreadID=Core[cpu]->T.ThreadID;
	Shm->Cpu[cpu].Topology.MP.x2APIC=((Proc->Features.Std.CX.x2APIC
					& Core[cpu]->T.Base.EN)
					<< Core[cpu]->T.Base.EXTD);
	unsigned int loop=0;
	for(loop=0; loop < CACHE_MAX_LEVEL; loop++)
	{
	    if(Core[cpu]->T.Cache[loop].Type > 0)
	    {
		unsigned int level=Core[cpu]->T.Cache[loop].Level;
		if(Core[cpu]->T.Cache[loop].Type == 2) // Instruction
			level=0;

		if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12))
		{
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
	  else	if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12))
		{
			unsigned int Compute_Way(unsigned int value)
			{
				switch(value)
				{
				case 0x6:
					return(8);
				case 0x8:
					return(16);
				case 0xa:
					return(32);
				case 0xb:
					return(48);
				case 0xc:
					return(64);
				case 0xd:
					return(96);
				case 0xe:
					return(128);
				default:
					return(value);
				}
			}
			Shm->Cpu[cpu].Topology.Cache[level].Way=
				(loop != 2) ?
					Core[cpu]->T.Cache[loop].Way
				: Compute_Way(Core[cpu]->T.Cache[loop].Way);

			Shm->Cpu[cpu].Topology.Cache[level].Size=
				Core[cpu]->T.Cache[loop].Size;
		}
		Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack=
			Core[cpu]->T.Cache[loop].WrBack;
		Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive=
			Core[cpu]->T.Cache[loop].Inclus;
	    }
	}
}

void SpeedStep(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	if((Core[cpu]->T.ApicID >= 0)
	&& (Core[cpu]->T.CoreID >= 0))
		BITSET(LOCKLESS, Shm->Proc.SpeedStep_Mask, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.SpeedStep_Mask, cpu);

	if(Core[cpu]->Query.EIST)
		BITSET(LOCKLESS, Shm->Proc.SpeedStep, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.SpeedStep, cpu);
}

void TurboBoost(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	if((Core[cpu]->T.ApicID >= 0)
	&& (Core[cpu]->T.CoreID >= 0))
		BITSET(LOCKLESS, Shm->Proc.TurboBoost_Mask, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.TurboBoost_Mask, cpu);

	if(Core[cpu]->Query.Turbo)
		BITSET(LOCKLESS, Shm->Proc.TurboBoost, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.TurboBoost, cpu);
}

void CStates(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	if((Core[cpu]->T.ApicID >= 0)
	&& (Core[cpu]->T.CoreID >= 0))
	{
		BITSET(LOCKLESS, Shm->Proc.C1E_Mask, cpu);

	    if((Core[cpu]->T.ThreadID == 0) || (Core[cpu]->T.ThreadID == -1))
	    {
		BITSET(LOCKLESS, Shm->Proc.C3A_Mask, cpu);
		BITSET(LOCKLESS, Shm->Proc.C1A_Mask, cpu);
		BITSET(LOCKLESS, Shm->Proc.C3U_Mask, cpu);
		BITSET(LOCKLESS, Shm->Proc.C1U_Mask, cpu);
	    }
	    else
	    {
		BITCLR(LOCKLESS, Shm->Proc.C3A_Mask, cpu);
		BITCLR(LOCKLESS, Shm->Proc.C1A_Mask, cpu);
		BITCLR(LOCKLESS, Shm->Proc.C3U_Mask, cpu);
		BITCLR(LOCKLESS, Shm->Proc.C1U_Mask, cpu);
	    }
	}
	else
		BITCLR(LOCKLESS, Shm->Proc.C1E_Mask, cpu);

	if(Core[cpu]->Query.C1E)
		BITSET(LOCKLESS, Shm->Proc.C1E, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C1E, cpu);
	if(Core[cpu]->Query.C3A)
		BITSET(LOCKLESS, Shm->Proc.C3A, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C3A, cpu);
	if(Core[cpu]->Query.C1A)
		BITSET(LOCKLESS, Shm->Proc.C1A, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C1A, cpu);
	if(Core[cpu]->Query.C3U)
		BITSET(LOCKLESS, Shm->Proc.C3U, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C3U, cpu);
	if(Core[cpu]->Query.C1U)
		BITSET(LOCKLESS, Shm->Proc.C1U, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C1U, cpu);
}

void ThermalMonitoring(SHM_STRUCT *Shm,PROC *Proc,CORE **Core,unsigned int cpu)
{
	Shm->Cpu[cpu].PowerThermal.TM1 =				\
		Proc->Features.Std.DX.TM1;			//0001
	Shm->Cpu[cpu].PowerThermal.TM1 |=				\
		(Core[cpu]->PowerThermal.TCC_Enable << 1);	//0010
	Shm->Cpu[cpu].PowerThermal.TM1 ^=				\
		(Core[cpu]->PowerThermal.TM_Select << 1);	//0010
	Shm->Cpu[cpu].PowerThermal.TM2 =				\
		Proc->Features.Std.CX.TM2;			//0001
	Shm->Cpu[cpu].PowerThermal.TM2 |=				\
		(Core[cpu]->PowerThermal.TM2_Enable << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.ODCM =				\
		Core[cpu]->PowerThermal.ClockModulation.ODCM_DutyCycle
		* (Core[cpu]->PowerThermal.ClockModulation.ExtensionBit == 1) ?
			6.25f : 12.5f;

	Shm->Cpu[cpu].PowerThermal.Target=Core[cpu]->PowerThermal.Target;

	Shm->Cpu[cpu].PowerThermal.PowerPolicy =			\
		Core[cpu]->PowerThermal.PerfEnergyBias.PowerPolicy;

	Shm->Cpu[cpu].PowerThermal.Limit[0]=Core[cpu]->PowerThermal.Target;
	Shm->Cpu[cpu].PowerThermal.Limit[1]=0;
}

void IdleDriver(SHM_STRUCT *Shm, PROC *Proc)
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

void PerCore_Update(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	Shm->Cpu[cpu].OffLine.HW=Core[cpu]->OffLine.HW;

	BaseClock(Shm, Core, cpu);

	Topology(Shm, Proc, Core, cpu);

	SpeedStep(Shm, Proc, Core, cpu);

	TurboBoost(Shm, Core, cpu);

	CStates(Shm, Core, cpu);

	ThermalMonitoring(Shm, Proc, Core, cpu);
}

void Core_Manager(SHM_STRUCT *Shm, PROC *Proc, CORE **Core)
{
    unsigned int cpu=0;

    pthread_t tid=pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    pthread_setname_np(tid, "corefreqd-mgr");

    ARG *Arg=calloc(Shm->Proc.CPU.Count, sizeof(ARG));
    while(!Shutdown)
    {
	for(cpu=0; !Shutdown && (cpu < Shm->Proc.CPU.Count); cpu++)
	  if(Core[cpu]->OffLine.OS == 1)
	  {
	    if(Arg[cpu].TID)
	    {	// Remove this cpu.
		pthread_join(Arg[cpu].TID, NULL);
		Arg[cpu].TID=0;
		// Raise this bit up to notify a platform change.
		if(!BITVAL(Shm->Proc.Sync, 63))
			BITSET(BUS_LOCK, Shm->Proc.Sync, 63);
	    }
	  Shm->Cpu[cpu].OffLine.OS=1;
	  }
	  else
	  {
	    if(!Arg[cpu].TID)
	    {	// Add this cpu.
		PerCore_Update(Shm, Proc, Core, cpu);
		HyperThreading(Shm, Proc);

		if(Quiet & 0x100)
		    printf("    CPU #%03u @ %.2f MHz\n", cpu,
			(double)( Core[cpu]->Clock.Hz
				* Proc->Boost[1])
				/ 1000000L );

		Arg[cpu].Proc=&Shm->Proc;
		Arg[cpu].Core=Core[cpu];
		Arg[cpu].Cpu=&Shm->Cpu[cpu];
		Arg[cpu].Bind=cpu;
		pthread_create(	&Arg[cpu].TID,
				NULL,
				Core_Cycle,
				&Arg[cpu]);

		if(!BITVAL(Shm->Proc.Sync, 63))
			BITSET(BUS_LOCK, Shm->Proc.Sync, 63);
	    }
	    Shm->Cpu[cpu].OffLine.OS=0;
	  }
	  // Average counters if all the room bits are cleared.
	  if(!Shutdown && BITWISEAND(BUS_LOCK, Shm->Proc.Room, roomSeed))
		usleep(Shm->Proc.SleepInterval * BASE_SLEEP);
	  else if(!Shutdown)
	  {
		// Update the count of online CPU
		Shm->Proc.CPU.OnLine=Proc->CPU.OnLine;

		//  Average the counters.
		Shm->Proc.Avg.Turbo=0;
		Shm->Proc.Avg.C0=0;
		Shm->Proc.Avg.C3=0;
		Shm->Proc.Avg.C6=0;
		Shm->Proc.Avg.C7=0;
		Shm->Proc.Avg.C1=0;

		for(cpu=0; !Shutdown &&(cpu < Shm->Proc.CPU.Count); cpu++)
		{
		    if(!Shm->Cpu[cpu].OffLine.OS)
		    {	// For each cpu, sum counters values from
			// the alternated memory structure.
			struct FLIP_FLOP *Flop=	\
				&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

			Shm->Proc.Avg.Turbo+=Flop->State.Turbo;
			Shm->Proc.Avg.C0+=Flop->State.C0;
			Shm->Proc.Avg.C3+=Flop->State.C3;
			Shm->Proc.Avg.C6+=Flop->State.C6;
			Shm->Proc.Avg.C7+=Flop->State.C7;
			Shm->Proc.Avg.C1+=Flop->State.C1;
		    }
		}
		Shm->Proc.Room=~(1 << Shm->Proc.CPU.Count);
		// Compute the counters averages.
		Shm->Proc.Avg.Turbo/=Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C0/=Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C3/=Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C6/=Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C7/=Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C1/=Shm->Proc.CPU.OnLine;
		// Notify Client.
		BITSET(BUS_LOCK, Shm->Proc.Sync, 0);
	  }
    }
    for(cpu=0; cpu < Shm->Proc.CPU.Count; cpu++)
	if(Arg[cpu].TID)
		pthread_join(Arg[cpu].TID, NULL);
    free(Arg);
}

typedef	struct
{
	int	Drv,
		Svr;
} FD;

int Shm_Manager(FD *fd, PROC *Proc)
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

		// Copy the timer interval delay.
		Shm->Proc.SleepInterval=Proc->SleepInterval;

		// Technologies aggregation.

		PerformanceMonitoring(Shm, Proc);

		InvariantTSC(Shm, Proc);

		HyperThreading(Shm, Proc);

		PowerNow(Shm, Proc);

		// Store the application name.
		strncpy(Shm->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

		// Aggregate the OS idle driver data.
		IdleDriver(Shm, Proc);

		// Initialize notification.
		BITCLR(BUS_LOCK, Shm->Proc.Sync, 0);

		// Welcomes with brand and per CPU base clock.
		if(Quiet & 0x001)
		 printf("CoreFreq Daemon."				\
			"  Copyright (C) 2015-2016 CYRIL INGENIERIE\n");
		if(Quiet & 0x010)
		 printf("\n"						\
			"  Processor [%s]\n"				\
			"  Architecture [%s] %u/%u CPU Online.\n",
			Shm->Proc.Brand,
			Shm->Proc.Architecture,
			Shm->Proc.CPU.OnLine,
			Shm->Proc.CPU.Count );
		if(Quiet & 0x100)
		  printf("  SleepInterval(%u)\n\n", Shm->Proc.SleepInterval);
		if(Quiet)
			fflush(stdout);

		Core_Manager(Shm, Proc, Core);

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
	pthread_t tid=pthread_self();

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);

	pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	pthread_setname_np(tid, "corefreqd-kill");

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

int help(char *appName)
{
	printf(	"usage:\t%s [-option]\n"				\
		"\t-q\tQuiet\n"						\
		"\t-i\tInfo\n"						\
		"\t-d\tDebug\n"						\
		"\t-m\tMath\n"						\
		"\t-h\tPrint out this message\n"			\
		"\nExit status:\n"					\
			"0\tif OK,\n"					\
			"1\tif problems,\n"				\
			">1\tif serious trouble.\n"			\
		"\nReport bugs to labs[at]cyring.fr\n", appName);
	return(1);
}

int main(int argc, char *argv[])
{
	FD   fd={0, 0};
	PROC *Proc=NULL;	// Kernel module anchor point.
	int  rc=0, i=0;
	char *program=strdup(argv[0]), *appName=basename(program);

	for(i=1; i < argc; i++)
	{
	    if(strlen(argv[i]) > 1)
	    {
		if(argv[i][0] == '-')
		{
			char option=argv[i][1];
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
			case 'm':
				Math=0x1;
			break;
			case 'h':
			default:
				rc=help(appName);
			break;
			}
		}
		else rc=help(appName);
	    }
	    else rc=help(appName);
	}
	if(!rc)
	  if(geteuid() == 0)
	  {
	    if((fd.Drv=open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
	    {
		if((Proc=mmap(	NULL, PAGE_SIZE,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.Drv, 0)) != NULL)
		{
			SIG Sig={.Signal={0}, .TID=0, .Started=0};
			sigemptyset(&Sig.Signal);
			sigaddset(&Sig.Signal, SIGUSR1);
			sigaddset(&Sig.Signal, SIGINT);	// [CTRL] + [C]
			sigaddset(&Sig.Signal, SIGQUIT);
			sigaddset(&Sig.Signal, SIGTERM);

			if(!pthread_sigmask(SIG_BLOCK, &Sig.Signal, NULL)
			&& !pthread_create(&Sig.TID, NULL, Emergency, &Sig))
				Sig.Started=1;

			rc=Shm_Manager(&fd, Proc);

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
	    else rc=3;
	  }
	  else
	  {
	    printf("Insufficient permissions. Need root to start daemon.\n");
	    rc=2;
	  }
	free(program);
	return(rc);
}
