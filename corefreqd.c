/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "bitasm.h"
#include "amdmsr.h"
#include "intelmsr.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreqm.h"
#include "corefreq-api.h"

#define PAGE_SIZE (sysconf(_SC_PAGESIZE))

// §8.10.6.7 Place Locks and Semaphores in Aligned, 128-Byte Blocks of Memory
static Bit64 roomSeed __attribute__ ((aligned (64))) = 0x0;
static Bit64 roomCore __attribute__ ((aligned (64))) = 0x0;
static Bit64 roomSched __attribute__ ((aligned (64))) = 0x0;
static Bit64 Shutdown __attribute__ ((aligned (64))) = 0x0;
unsigned int Quiet = 0x001, SysGateStartUp = 1;

typedef struct
{
	int	Drv,
		Svr;
} FD;

typedef struct {
	sigset_t		Signal;
	pid_t			CPID;
	pthread_t		KID;
	int			Started;
	struct {
		SLICE_FUNC	Func;
		unsigned long	arg;
		enum PATTERN	pattern;
	} Slice;
	FD			*fd;
	SHM_STRUCT		*Shm;
	PROC			*Proc;
	CORE			**Core;
	SYSGATE			*SysGate;
} REF;

typedef struct {
	REF		*Ref;
	unsigned int	Bind;
	pthread_t	TID;
} ARG;

static void *Core_Cycle(void *arg)
{
	ARG *Arg	= (ARG *) arg;
	unsigned int cpu = Arg->Bind;
	SHM_STRUCT *Shm = Arg->Ref->Shm;
	PROC *Pkg	= Arg->Ref->Proc;
	CORE *Core	= Arg->Ref->Core[cpu];
	CPU_STRUCT *Cpu = &Shm->Cpu[cpu];

	pthread_t tid = pthread_self();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) != 0)
		goto EXIT;

	char comm[TASK_COMM_LEN];
	sprintf(comm, "corefreqd/%d", cpu);
	pthread_setname_np(tid, comm);

	if (Quiet & 0x100) {
		printf("    Thread [%lx] Init CYCLE %03u\n", tid, cpu);
		fflush(stdout);
	}
	BITSET(BUS_LOCK, roomSeed, cpu);
	BITSET(BUS_LOCK, roomCore, cpu);

  do {
    while (!BITVAL(Core->Sync.V, 63)
	&& !BITVAL(Shutdown, 0)
	&& !BITVAL(Core->OffLine, OS)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
    }
	BITCLR(LOCKLESS, Core->Sync.V, 63);

    if (!BITVAL(Shutdown, 0) && !BITVAL(Core->OffLine, OS))
    {
	if (BITVAL(roomCore, cpu)) {
		Cpu->Toggle = !Cpu->Toggle;
		BITCLR(BUS_LOCK, roomCore, cpu);
	}
	struct FLIP_FLOP *CFlip = &Cpu->FlipFlop[Cpu->Toggle];

	CFlip->Delta.INST	= Core->Delta.INST;
	CFlip->Delta.C0.UCC	= Core->Delta.C0.UCC;
	CFlip->Delta.C0.URC	= Core->Delta.C0.URC;
	CFlip->Delta.C3		= Core->Delta.C3;
	CFlip->Delta.C6		= Core->Delta.C6;
	CFlip->Delta.C7		= Core->Delta.C7;
	CFlip->Delta.TSC	= Core->Delta.TSC;
	CFlip->Delta.C1		= Core->Delta.C1;

	// Compute IPS=Instructions per TSC
	CFlip->State.IPS = (double) (CFlip->Delta.INST)
			 / (double) (CFlip->Delta.TSC);

	// Compute IPC=Instructions per non-halted reference cycle.
	// (Protect against a division by zero)
	CFlip->State.IPC = (CFlip->Delta.C0.URC != 0) ?
			  (double) (CFlip->Delta.INST)
			 / (double) CFlip->Delta.C0.URC
			 : 0.0f;

	// Compute CPI=Non-halted reference cycles per instruction.
	// (Protect against a division by zero)
	CFlip->State.CPI = (CFlip->Delta.INST != 0) ?
			  (double) CFlip->Delta.C0.URC
			 / (double) (CFlip->Delta.INST)
			 : 0.0f;

	// Compute Turbo State.
	CFlip->State.Turbo = (double) (CFlip->Delta.C0.UCC)
			   / (double) (CFlip->Delta.TSC);
	// Compute C-States.
	CFlip->State.C0 = (double) (CFlip->Delta.C0.URC)
			/ (double) (CFlip->Delta.TSC);
	CFlip->State.C3 = (double) (CFlip->Delta.C3)
			/ (double) (CFlip->Delta.TSC);
	CFlip->State.C6 = (double) (CFlip->Delta.C6)
			/ (double) (CFlip->Delta.TSC);
	CFlip->State.C7 = (double) (CFlip->Delta.C7)
			/ (double) (CFlip->Delta.TSC);
	CFlip->State.C1 = (double) (CFlip->Delta.C1)
			/ (double) (CFlip->Delta.TSC);

	// Relative Ratio formula.
	CFlip->Relative.Ratio	= (double) (CFlip->Delta.C0.UCC
					  * Shm->Proc.Boost[BOOST(MAX)])
				/ (double) (CFlip->Delta.TSC);

	if (Shm->Proc.PM_version >= 2) {
		// Relative Frequency equals UCC per second.
		CFlip->Relative.Freq = (double) (CFlip->Delta.C0.UCC)
				     / (Shm->Sleep.Interval * 1000);
	} else {
	// Relative Frequency = Relative Ratio x Bus Clock Frequency
	  CFlip->Relative.Freq = (double)REL_FREQ( Shm->Proc.Boost[BOOST(MAX)],\
					CFlip->Relative.Ratio,		\
					Core->Clock, Shm->Sleep.Interval)
				/ (Shm->Sleep.Interval * 1000);
	}

	// Per Core thermal formulas
	CFlip->Thermal.Sensor = Core->PowerThermal.Sensor;
	CFlip->Thermal.Events = Core->PowerThermal.Events;

	switch (Pkg->thermalFormula) {
	case THERMAL_FORMULA_INTEL:
		COMPUTE_THERMAL(INTEL,
				CFlip->Thermal.Temp,
				Cpu->PowerThermal.Target,
				CFlip->Thermal.Sensor);
		break;
	case THERMAL_FORMULA_AMD:
		COMPUTE_THERMAL(AMD,
				CFlip->Thermal.Temp,
				Cpu->PowerThermal.Target,
				CFlip->Thermal.Sensor);
		break;
	case THERMAL_FORMULA_AMD_0Fh:
		COMPUTE_THERMAL(AMD_0Fh,
				CFlip->Thermal.Temp,
				Cpu->PowerThermal.Target,
				CFlip->Thermal.Sensor);
		break;
	}

	// Per Core voltage formulas
	CFlip->Voltage.VID = Core->Counter[1].VID;
	switch (Pkg->voltageFormula) {
	// Intel Core 2 Extreme Datasheet §3.3-Table 2
	case VOLTAGE_FORMULA_INTEL_MEROM:
		COMPUTE_VOLTAGE(INTEL_MEROM,
				CFlip->Voltage.Vcore,
				CFlip->Voltage.VID);
		break;
	case VOLTAGE_FORMULA_INTEL_SKL_X:
		COMPUTE_VOLTAGE(INTEL_SKL_X,
				CFlip->Voltage.Vcore,
				CFlip->Voltage.VID);
		break;
	case VOLTAGE_FORMULA_AMD:
		COMPUTE_VOLTAGE(AMD,
				CFlip->Voltage.Vcore,
				CFlip->Voltage.VID);
		break;
	// AMD BKDG Family 0Fh §10.6 Table 70
	case VOLTAGE_FORMULA_AMD_0Fh:
		COMPUTE_VOLTAGE(AMD_0Fh,
				CFlip->Voltage.Vcore,
				CFlip->Voltage.VID);
		break;
	case VOLTAGE_FORMULA_AMD_17h:
		COMPUTE_VOLTAGE(AMD_17h,
				CFlip->Voltage.Vcore,
				CFlip->Voltage.VID);
		break;
	}
	// Interrupts counters
	CFlip->Counter.SMI = Core->Interrupt.SMI;

	if (Shm->Registration.nmi) {
		CFlip->Counter.NMI.LOCAL   = Core->Interrupt.NMI.LOCAL;
		CFlip->Counter.NMI.UNKNOWN = Core->Interrupt.NMI.UNKNOWN;
		CFlip->Counter.NMI.PCISERR = Core->Interrupt.NMI.PCISERR;
		CFlip->Counter.NMI.IOCHECK = Core->Interrupt.NMI.IOCHECK;
	}

	// Package scope counters
	if (cpu == Pkg->Service.Core)
	{
		enum PWR_DOMAIN pw;

		Shm->Proc.Toggle = !Shm->Proc.Toggle;

	    struct PKG_FLIP_FLOP *PFlip = &Shm->Proc.FlipFlop[Shm->Proc.Toggle];

		PFlip->Delta.PTSC = Pkg->Delta.PTSC;
		PFlip->Delta.PC02 = Pkg->Delta.PC02;
		PFlip->Delta.PC03 = Pkg->Delta.PC03;
		PFlip->Delta.PC06 = Pkg->Delta.PC06;
		PFlip->Delta.PC07 = Pkg->Delta.PC07;
		PFlip->Delta.PC08 = Pkg->Delta.PC08;
		PFlip->Delta.PC09 = Pkg->Delta.PC09;
		PFlip->Delta.PC10 = Pkg->Delta.PC10;
		// Package C-state Residency counters
		Shm->Proc.State.PC02	= (double) PFlip->Delta.PC02
					/ (double) PFlip->Delta.PTSC;
		Shm->Proc.State.PC03	= (double) PFlip->Delta.PC03
					/ (double) PFlip->Delta.PTSC;
		Shm->Proc.State.PC06	= (double) PFlip->Delta.PC06
					/ (double) PFlip->Delta.PTSC;
		Shm->Proc.State.PC07	= (double) PFlip->Delta.PC07
					/ (double) PFlip->Delta.PTSC;
		Shm->Proc.State.PC08	= (double) PFlip->Delta.PC08
					/ (double) PFlip->Delta.PTSC;
		Shm->Proc.State.PC09	= (double) PFlip->Delta.PC09
					/ (double) PFlip->Delta.PTSC;
		Shm->Proc.State.PC10	= (double) PFlip->Delta.PC10
					/ (double) PFlip->Delta.PTSC;
		// Uncore scope counters
		PFlip->Uncore.FC0 = Pkg->Delta.Uncore.FC0;

		// Power & Energy counters
	    for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
	    {
		PFlip->Delta.ACCU[pw] = Pkg->Delta.Power.ACCU[pw];

		Shm->Proc.State.Energy[pw] = (double) PFlip->Delta.ACCU[pw]
					   * Shm->Proc.Power.Unit.Joules;

		Shm->Proc.State.Power[pw] =(double) PFlip->Delta.ACCU[pw]
					  * Shm->Proc.Power.Unit.Watts
					  * Shm->Proc.Power.Unit.Times;
	    }
		// Package thermal formulas
		PFlip->Thermal.Sensor = Pkg->PowerThermal.Sensor;
		PFlip->Thermal.Events = Pkg->PowerThermal.Events;

		switch (Pkg->thermalFormula) {
		case THERMAL_FORMULA_INTEL:
		    if (Shm->Proc.Features.Power.EAX.PTM)
			COMPUTE_THERMAL(INTEL,
					PFlip->Thermal.Temp,
					Cpu->PowerThermal.Target,
					PFlip->Thermal.Sensor);
			break;
		case THERMAL_FORMULA_AMD_17h:
			COMPUTE_THERMAL(AMD_17h,
					CFlip->Thermal.Temp,
					Cpu->PowerThermal.Target,
					CFlip->Thermal.Sensor);
			break;
		}
		// Package voltage formulas
		switch (Pkg->voltageFormula) {
		// Intel 2nd Gen Datasheet Vol-1 §7.4 Table 7-1
		case VOLTAGE_FORMULA_INTEL_SNB:
			COMPUTE_VOLTAGE(INTEL_SNB,
					CFlip->Voltage.Vcore,
					CFlip->Voltage.VID);
			break;
	    }
	}
	// Min and Max temperatures per Core
	if (CFlip->Thermal.Temp < Cpu->PowerThermal.Limit[0])
		Cpu->PowerThermal.Limit[0] = CFlip->Thermal.Temp;
	if (CFlip->Thermal.Temp > Cpu->PowerThermal.Limit[1])
		Cpu->PowerThermal.Limit[1] = CFlip->Thermal.Temp;
    }
  } while (!BITVAL(Shutdown, 0) && !BITVAL(Core->OffLine, OS)) ;

	BITCLR(BUS_LOCK, roomCore, cpu);
	BITCLR(BUS_LOCK, roomSeed, cpu);
EXIT:
	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CYCLE %03u\n", tid,
			BITVAL(Core->OffLine, OS) ? "Offline" : "Shutdown",cpu);
		fflush(stdout);
	}
	return(NULL);
}

void SliceScheduling(SHM_STRUCT *Shm, unsigned int cpu, enum PATTERN pattern)
{
	unsigned int seek;
	switch (pattern) {
	case RESET_CSP:
		for (seek = 0; seek < Shm->Proc.CPU.Count; seek++) {
			if (seek == Shm->Proc.Service.Core)
				BITSET(LOCKLESS, roomSched, seek);
			else
				BITCLR(LOCKLESS, roomSched, seek);
		}
		break;
	case ALL_SMT:
		if (cpu == Shm->Proc.Service.Core)
			roomSched = roomSeed;
		break;
	case RAND_SMT:
		do {
			seek = (unsigned int) rand();
			seek = seek % Shm->Proc.CPU.Count;
		} while (BITVAL(Shm->Cpu[seek].OffLine, OS));
		BITCLR(LOCKLESS, roomSched, cpu);
		BITSET(LOCKLESS, roomSched, seek);
		break;
	case RR_SMT:
		seek = cpu;
		do {
			seek++;
			if (seek >= Shm->Proc.CPU.Count)
				seek = 0;
		} while (BITVAL(Shm->Cpu[seek].OffLine, OS));
		BITCLR(LOCKLESS, roomSched, cpu);
		BITSET(LOCKLESS, roomSched, seek);
		break;
	}
}

static void *Child_Thread(void *arg)
{
	ARG *Arg	= (ARG *) arg;
	unsigned int cpu = Arg->Bind;
	SHM_STRUCT *Shm = Arg->Ref->Shm;
	PROC *Pkg	= Arg->Ref->Proc;
	CPU_STRUCT *Cpu = &Shm->Cpu[cpu];

	CALL_FUNC MatrixCallFunc[2][2] = {
		{ CallWith_RDTSC_No_RDPMC,  CallWith_RDTSC_RDPMC  },
		{ CallWith_RDTSCP_No_RDPMC, CallWith_RDTSCP_RDPMC }
	};
	const int withTSCP = ((Pkg->Features.AdvPower.EDX.Inv_TSC == 1)
			   || (Pkg->Features.ExtInfo.EDX.RDTSCP == 1)),
		withRDPMC = ((Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
			  && (Shm->Proc.PM_version >= 1)
			  && (BITVAL(Cpu->SystemRegister.CR4, CR4_PCE) == 1));

	CALL_FUNC CallSliceFunc = MatrixCallFunc[withTSCP][withRDPMC];

	pthread_t tid = pthread_self();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) != 0)
		goto EXIT;

	char comm[TASK_COMM_LEN];
	sprintf(comm, "corefreqd#%d", cpu);
	pthread_setname_np(tid, comm);

	if (Quiet & 0x100) {
		printf("    Thread [%lx] Init CHILD %03u\n", tid, cpu);
		fflush(stdout);
	}

	BITSET(BUS_LOCK, roomSeed, cpu);

	do {
		while (!BITVAL(Shm->Proc.Sync, 31)
		    && !BITVAL(Shutdown, 0)
		    && !BITVAL(Cpu->OffLine, OS)) {
			nanosleep(&Shm->Sleep.sliceWaiting, NULL);
		}

		BITSET(BUS_LOCK, roomCore, cpu);

		RESET_Slice(Cpu->Slice);

		while ( BITVAL(Shm->Proc.Sync, 31)
		    && !BITVAL(Shutdown, 0) )
		{
		    if (BITVAL(roomSched, cpu)) {
			CallSliceFunc(	Shm, cpu,
					Arg->Ref->Slice.Func,
					Arg->Ref->Slice.arg);

			SliceScheduling(Shm, cpu, Arg->Ref->Slice.pattern);

		      if (BITVAL(Cpu->OffLine, OS)) { // ReSchedule to Service
			SliceScheduling(Shm, Shm->Proc.Service.Core, RESET_CSP);
			break;
		      }
		    } else {
			nanosleep(&Shm->Sleep.sliceWaiting, NULL);

			if (BITVAL(Cpu->OffLine, OS))
				break;
		    }
		}

		BITCLR(BUS_LOCK, roomCore, cpu);

	} while (!BITVAL(Shutdown, 0) && !BITVAL(Cpu->OffLine, OS)) ;

	BITCLR(BUS_LOCK, roomSeed, cpu);

	RESET_Slice(Cpu->Slice);
EXIT:
	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CHILD %03u\n", tid,
			BITVAL(Cpu->OffLine, OS) ? "Offline" : "Shutdown",cpu);
		fflush(stdout);
	}
	return(NULL);
}

void Architecture(SHM_STRUCT *Shm, PROC *Proc)
{
	Bit32	fTSC = Proc->Features.Std.EDX.TSC,
		aTSC = Proc->Features.AdvPower.EDX.Inv_TSC;

	// Copy all initial CPUID features.
	memcpy(&Shm->Proc.Features, &Proc->Features, sizeof(FEATURES));
	// Copy the numbers of total & online CPU.
	Shm->Proc.CPU.Count	= Proc->CPU.Count;
	Shm->Proc.CPU.OnLine	= Proc->CPU.OnLine;
	Shm->Proc.Service.Proc	= Proc->Service.Proc;
	// Copy the Architecture name.
	strncpy(Shm->Proc.Architecture, Proc->Architecture, 32);
	// Copy the base clock ratios.
	memcpy(Shm->Proc.Boost, Proc->Boost,(BOOST(SIZE))*sizeof(unsigned int));
	// Copy the processor's brand string.
	strncpy(Shm->Proc.Brand, Proc->Features.Info.Brand, 48);
	// Compute the TSC mode: None, Variant, Invariant
	Shm->Proc.Features.InvariantTSC = fTSC << aTSC;
}

void PerformanceMonitoring(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.PM_version = Proc->Features.PerfMon.EAX.Version;
}

void HyperThreading(SHM_STRUCT *Shm, PROC *Proc)
{	// Update the HyperThreading state
	Shm->Proc.Features.HyperThreading = Proc->Features.HTT_Enable;
}

void PowerInterface(SHM_STRUCT *Shm, PROC *Proc)
{
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD) {	// AMD PowerNow
	if (Proc->Features.AdvPower.EDX.FID)
		BITSET(LOCKLESS, Shm->Proc.PowerNow, 0);
	else
		BITCLR(LOCKLESS, Shm->Proc.PowerNow, 0);

	if (Proc->Features.AdvPower.EDX.VID)
		BITSET(LOCKLESS, Shm->Proc.PowerNow, 1);
	else
		BITCLR(LOCKLESS, Shm->Proc.PowerNow, 1);
    }
    else
	Shm->Proc.PowerNow = 0;

    switch (Proc->powerFormula) {
      case POWER_FORMULA_INTEL:
      case POWER_FORMULA_AMD:
	Shm->Proc.Power.Unit.Watts = Proc->PowerThermal.Unit.PU > 0 ?
			1.0 / (double) (1 << Proc->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= Proc->PowerThermal.Unit.ESU > 0 ?
			1.0 / (double)(1 << Proc->PowerThermal.Unit.ESU) : 0;
	break;
      case POWER_FORMULA_INTEL_ATOM:
	Shm->Proc.Power.Unit.Watts = Proc->PowerThermal.Unit.PU > 0 ?
			0.001 / (double)(1 << Proc->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= Proc->PowerThermal.Unit.ESU > 0 ?
			0.001 / (double)(1 << Proc->PowerThermal.Unit.ESU) : 0;
	break;
      case POWER_FORMULA_AMD_17h: {
	unsigned int maxCoreCount = (Shm->Proc.Features.leaf80000008.ECX.NC + 1)
					>> Shm->Proc.Features.HTT_Enable;

	Shm->Proc.Power.Unit.Watts = Proc->PowerThermal.Unit.PU > 0 ?
			1.0 / (double) (1 << Proc->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= Proc->PowerThermal.Unit.ESU > 0 ?
			1.0 / (double)(1 << Proc->PowerThermal.Unit.ESU) : 0;
	if (maxCoreCount != 0) {
		Shm->Proc.Power.Unit.Watts  /= maxCoreCount;
		Shm->Proc.Power.Unit.Joules /= maxCoreCount;
	}
      }
	break;
    }
	Shm->Proc.Power.Unit.Times = Proc->PowerThermal.Unit.TU > 0 ?
			1.0 / (double) (1 << Proc->PowerThermal.Unit.TU) : 0;
	// Scale window unit time to the driver monitoring interval.
	Shm->Proc.Power.Unit.Times *= 1000.0 / (double) Shm->Sleep.Interval;
}

void Technology_Update(SHM_STRUCT *Shm, PROC *Proc)
{	// Technologies aggregation.
	Shm->Proc.Technology.PowerNow = (Shm->Proc.PowerNow == 0b11);

	Shm->Proc.Technology.ODCM = BITWISEAND(LOCKLESS,
						Proc->ODCM,
						Proc->ODCM_Mask) != 0;

	Shm->Proc.Technology.PowerMgmt = BITWISEAND(LOCKLESS,
						Proc->PowerMgmt,
						Proc->PowerMgmt_Mask) != 0;

	Shm->Proc.Technology.EIST = BITWISEAND(LOCKLESS,
						Proc->SpeedStep,
						Proc->SpeedStep_Mask) != 0;

	Shm->Proc.Technology.Turbo = BITWISEAND(LOCKLESS,
						Proc->TurboBoost,
						Proc->TurboBoost_Mask) != 0;

	Shm->Proc.Technology.C1E = BITWISEAND(LOCKLESS,
						Proc->C1E,
						Proc->C1E_Mask) != 0;

	Shm->Proc.Technology.C3A = BITWISEAND(LOCKLESS,
						Proc->C3A,
						Proc->C3A_Mask) != 0;

	Shm->Proc.Technology.C1A = BITWISEAND(LOCKLESS,
						Proc->C1A,
						Proc->C1A_Mask) != 0;

	Shm->Proc.Technology.C3U = BITWISEAND(LOCKLESS,
						Proc->C3U,
						Proc->C3U_Mask) != 0;

	Shm->Proc.Technology.C1U = BITWISEAND(LOCKLESS,
						Proc->C1U,
						Proc->C1U_Mask) != 0;

	Shm->Proc.Technology.CC6 = BITWISEAND(LOCKLESS,
						Proc->CC6,
						Proc->CC6_Mask) != 0;

	Shm->Proc.Technology.PC6 = BITWISEAND(LOCKLESS,
						Proc->PC6,
						Proc->PC6_Mask) != 0;

	Shm->Proc.Technology.SMM = BITWISEAND(LOCKLESS,
						Proc->SMM,
						Proc->CR_Mask) != 0;

	Shm->Proc.Technology.VM = BITWISEAND(LOCKLESS,
						Proc->VM,
						Proc->CR_Mask) != 0;
}

void Package_Update(SHM_STRUCT *Shm, PROC *Proc)
{
	// Copy the timer interval delay.
	Shm->Sleep.Interval = Proc->SleepInterval;
	// Compute the polling wait time based on the timer interval.
	Shm->Sleep.pollingWait = TIMESPEC((Shm->Sleep.Interval*1000000L)
				/ WAKEUP_RATIO);
	// Copy the SysGate tick steps.
	Shm->SysGate.tickReset = Proc->tickReset;
	Shm->SysGate.tickStep  = Proc->tickStep;

	Shm->Registration.Experimental = Proc->Registration.Experimental;
	Shm->Registration.hotplug = Proc->Registration.hotplug;
	Shm->Registration.pci = Proc->Registration.pci;
	Shm->Registration.nmi = Proc->Registration.nmi;

	Architecture(Shm, Proc);

	PerformanceMonitoring(Shm, Proc);

	HyperThreading(Shm, Proc);

	PowerInterface(Shm, Proc);
}

typedef struct {
	unsigned int	Q,
			R;
} RAM_Ratio;

void P945_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR =
			Proc->Uncore.MC[mc].Channel[cha].P945.DRT0.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS =
			Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tRAS;

	switch (Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tRRD) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 1;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 2;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC =
			Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tRFC;

	switch (Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tCL) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 5;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 4;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 3;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 6;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD =
			Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tRCD + 2;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP =
			Proc->Uncore.MC[mc].Channel[cha].P945.DRT1.tRP + 2;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
	    unsigned short rank, rankCount = (cha == 0) ? 4 : 2;
	    for (rank = 0; rank < rankCount; rank++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks +=
		    Proc->Uncore.MC[mc].Channel[cha].P945.DRB[rank].Boundary;
	    }
	    switch(Proc->Uncore.MC[mc].Channel[cha].P945.BANK.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch(Proc->Uncore.MC[mc].Channel[cha].P945.WIDTH.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 16384;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 8192;
		break;
	    }
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size =
			  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /=(1024 * 1024);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P955_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS =
			Proc->Uncore.MC[mc].Channel[cha].P955.DRT1.tRAS;

	switch (Proc->Uncore.MC[mc].Channel[cha].P955.DRT1.tCL) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 5;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 4;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 3;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 6;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD =
			Proc->Uncore.MC[mc].Channel[cha].P955.DRT1.tRCD + 2;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP =
			Proc->Uncore.MC[mc].Channel[cha].P955.DRT1.tRP + 2;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
	    unsigned short rank;
	    for (rank = 0; rank < 4; rank++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks +=
		    Proc->Uncore.MC[mc].Channel[cha].P955.DRB[rank].Boundary;
	    }
	    switch(Proc->Uncore.MC[mc].Channel[cha].P955.BANK.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch(Proc->Uncore.MC[mc].Channel[cha].P955.WIDTH.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 16384;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 8192;
		break;
	    }
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size =
			  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P945_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};
	switch (Proc->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b000:
		Shm->Uncore.Bus.Rate = 400;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			// Fallthrough
		case 0b010:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b011:
			Ratio.Q = 4;
			Ratio.R = 3;
			break;
		case 0b100:
			Ratio.Q = 5;
			Ratio.R = 3;
			break;
		}
		break;
	case 0b001:
		Shm->Uncore.Bus.Rate = 533;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			// Fallthrough
		case 0b010:
			Ratio.Q = 3;
			Ratio.R = 4;
			break;
		case 0b011:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b001:
			Ratio.Q = 5;
			Ratio.R = 4;
			break;
		}
		break;
	default:
		// Fallthrough
	case 0b011:
		Shm->Uncore.Bus.Rate = 667;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			// Fallthrough
		case 0b010:
			Ratio.Q = 3;
			Ratio.R = 5;
			break;
		case 0b011:
			Ratio.Q = 4;
			Ratio.R = 5;
			break;
		case 0b100:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		}
		break;
	}
	Shm->Uncore.CtrlSpeed = (Shm->Cpu[cpu].Clock.Hz * Ratio.Q * 2)	// DDR2
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void P965_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT0.tCL;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT1.tRAS;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT1.tWR;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT2.tRFC;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT2.tRP;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT2.tRRD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT4.tRCD_RD;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT_.tFAW;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT_.tRTPr;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = ?
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL  += 3;
/* ToDo */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = 8
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /= (1024 *1024);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P965_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};
	switch (Proc->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b111:	// Unknown
		// Fallthrough
	case 0b000:
		Shm->Uncore.Bus.Rate = 1066;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b001:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b010:
			Ratio.Q = 5;
			Ratio.R = 4;
			break;
		case 0b011:
			Ratio.Q = 3;
			Ratio.R = 2;
			break;
		case 0b100:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		case 0b101:
			Ratio.Q = 5;
			Ratio.R = 2;
			break;
		}
		break;
	case 0b001:
		Shm->Uncore.Bus.Rate = 533;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b001:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		case 0b010:
			Ratio.Q = 5;
			Ratio.R = 2;
			break;
		case 0b011:
			Ratio.Q = 3;
			Ratio.R = 1;
			break;
		}
		break;
	case 0b011:
		Shm->Uncore.Bus.Rate = 667;
		break;
	case 0b100:
		Shm->Uncore.Bus.Rate = 1333;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b010:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b011:
			Ratio.Q = 6;
			Ratio.R = 5;
			break;
		case 0b100:
			Ratio.Q = 8;
			Ratio.R = 5;
			break;
		case 0b101:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		}
		break;
	case 0b110:
		Shm->Uncore.Bus.Rate = 1600;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b011:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b100:
			Ratio.Q = 4;
			Ratio.R = 3;
			break;
		case 0b101:
			Ratio.Q = 3;
			Ratio.R = 2;
			break;
		case 0b110:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		}
		break;
	default:
		// Fallthrough
	case 0b010:
		Shm->Uncore.Bus.Rate = 800;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b000:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b001:
			Ratio.Q = 5;
			Ratio.R = 4;
			break;
		case 0b010:
			Ratio.Q = 5;
			Ratio.R = 3;
			break;
		case 0b011:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		case 0b100:
			Ratio.Q = 8;
			Ratio.R = 3;
			break;
		case 0b101:
			Ratio.Q = 10;
			Ratio.R = 3;
			break;
		}
		break;
	}
	Shm->Uncore.CtrlSpeed = (Shm->Cpu[cpu].Clock.Hz * Ratio.Q * 2)	// DDR2
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void G965_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].G965.DRT0.tWR;

	switch (Proc->Uncore.MC[mc].Channel[cha].G965.DRT1.tRCD) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 2;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 3;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 4;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 5;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 6;
		break;
	case 0b101:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 7;
		break;
	case 0b110:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 8;
		break;
	}

	switch (Proc->Uncore.MC[mc].Channel[cha].G965.DRT1.tRP) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 2;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 3;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 4;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 5;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 6;
		break;
	case 0b101:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 7;
		break;
	case 0b110:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 8;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].G965.DRT1.tRAS;

	switch (Proc->Uncore.MC[mc].Channel[cha].G965.DRT1.tRRD) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 2;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 3;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 4;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 5;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 6;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].G965.DRT1.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
			Proc->Uncore.MC[mc].Channel[cha].G965.DRT2.tFAW;

	switch (Proc->Uncore.MC[mc].Channel[cha].G965.DRT3.tCL) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 3;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 4;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 5;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 6;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 7;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].G965.DRT3.tRFC;

	switch (Proc->Uncore.MC[mc].Channel[cha].G965.DRT3.tCWL) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 2;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 3;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 4;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 5;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 6;
		break;
	default:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL =
			Shm->Uncore.MC[mc].Channel[cha].Timing.tCL - 1;
		break;
	}
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank1Bank)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank0Bank)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 8;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.OddRank1)
	    {
	    case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1;
		break;
	    case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 2;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.EvenRank0)
	    {
	    case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += 1;
		break;
	    case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += 2;
		break;
	    }

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 4096;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = 8
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /= (1024 *1024);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void G965_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};
	switch (Proc->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b001:
		Shm->Uncore.Bus.Rate = 533;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b001:
			Ratio.Q = 5;
			Ratio.R = 4;
			break;
		case 0b010:
			Ratio.Q = 3;
			Ratio.R = 2;
			break;
		case 0b011:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		}
		break;
	case 0b011:
		Shm->Uncore.Bus.Rate = 667;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b001:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b010:
			Ratio.Q = 6;
			Ratio.R = 5;
			break;
		case 0b011:
			Ratio.Q = 8;
			Ratio.R = 5;
			break;
		case 0b100:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		case 0b101:
			Ratio.Q = 12;
			Ratio.R = 5;
			break;
		}
		break;
	case 0b110:
		Shm->Uncore.Bus.Rate = 1066;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b101:
			Ratio.Q = 3;
			Ratio.R = 2;
			break;
		case 0b110:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		}
		break;
	default:
		// Fallthrough
	case 0b010:
		Shm->Uncore.Bus.Rate = 800;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		case 0b001:
			Ratio.Q = 5;
			Ratio.R = 6;
			break;
		case 0b010:
			Ratio.Q = 1;
			Ratio.R = 1;
			break;
		case 0b011:
			Ratio.Q = 4;
			Ratio.R = 3;
			break;
		case 0b100:
			Ratio.Q = 5;
			Ratio.R = 3;
			break;
		case 0b101:
			Ratio.Q = 2;
			Ratio.R = 1;
			break;
		}
		break;
	}
	Shm->Uncore.CtrlSpeed = (Shm->Cpu[cpu].Clock.Hz * Ratio.Q * 2)	// DDR2
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void P3S_MCH(SHM_STRUCT *Shm, PROC *Proc, unsigned short mc, unsigned short cha)
{
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT0.tCL;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT1.tWR;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT2.tRFC;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT2.tRP;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT2.tRRD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT4.tRCD_RD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRT5.tRAS;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRTn.tFAW;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
		Proc->Uncore.MC[mc].Channel[cha].P35.DRTn.tRTPr;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWL  = ?
*/
}

void P35_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	P3S_MCH(Shm, Proc, mc, cha);

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL -= 9;
/* ToDo */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = 8
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /= (1024 *1024);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P35_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	P965_CLK(Shm, Proc, cpu);
}

void P4S_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	P3S_MCH(Shm, Proc, mc, cha);

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL -= 6;
/* ToDo */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = 8
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /= (1024 *1024);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void NHM_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;

      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;
      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
		Proc->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tCL ?
		4 + Proc->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tCL : 0;

	switch (Proc->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tWR) {
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 5;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 6;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 7;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 8;
		break;
	case 0b101:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 10;
		break;
	case 0b110:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 12;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.tWTPr;

	switch (Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrRdTRd) {
	case 0b0:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = 4;
		break;
	case 0b1:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = 6;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd = 10
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd = 1
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd = 1
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddWrTRd;

	switch (Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tsrWrTWr) {
	case 0b0:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = 4;
		break;
	case 0b1:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = 6;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tdrWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr = 2
		+ Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tddWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.B2B   =
		Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.B2B;

	switch (Proc->Uncore.MC[mc].Channel[cha].NHM.MR2_3.tCWL) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 5;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 6;
		break;
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 7;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = 8;
		break;
	}

	switch(Proc->Uncore.MC[mc].Channel[cha].NHM.Params.ENABLE_2N_3N)
	{
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 3;
		break;
	}

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
	    if (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.DIMMPRESENT) {
		switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMBANK)
		{
		case 0b00:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
			break;
		case 0b01:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
			break;
		case 0b10:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 16;
			break;
		}
		switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMRANK)
		{
		case 0b00:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1;
			break;
		case 0b01:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 2;
			break;
		case 0b10:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 4;
			break;
		}
		switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMROW)
		{
		case 0b000:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1<<12;
			break;
		case 0b001:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1<<13;
			break;
		case 0b010:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1<<14;
			break;
		case 0b011:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1<<15;
			break;
		case 0b100:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1<<16;
			break;
		}
		switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMCOL)
		{
		case 0b000:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1<<10;
			break;
		case 0b001:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1<<11;
			break;
		case 0b010:
			Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1<<12;
			break;
		}

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = 8
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /= (1024 *1024);
	    }
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC =
				Proc->Uncore.MC[mc].NHM.STATUS.ECC_ENABLED;
      }
    }
}

void QPI_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.Bus.DimmClock.QCLK_RATIO) {
	case 0b00110:
		Shm->Uncore.CtrlSpeed = 800;
		break;
	case 0b01000:
		Shm->Uncore.CtrlSpeed = 1066;
		break;
	case 0b01010:
		Shm->Uncore.CtrlSpeed = 1333;
		break;
	case 0b01100:
		Shm->Uncore.CtrlSpeed = 1600;
		break;
	case 0b01110:
		Shm->Uncore.CtrlSpeed = 1866;
		break;
	case 0b10000:
		Shm->Uncore.CtrlSpeed = 2133;
		break;
	case 0b000000:
		// Fallthrough
	default:
		Shm->Uncore.CtrlSpeed = 800;
		break;
	}
	Shm->Uncore.CtrlSpeed *= Shm->Cpu[cpu].Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = Proc->Uncore.Bus.QuickPath.QPIFREQSEL == 00 ?
		4800 : Proc->Uncore.Bus.QuickPath.QPIFREQSEL == 10 ?
			6400 : Proc->Uncore.Bus.QuickPath.QPIFREQSEL == 01 ?
				5866 : 6400;

	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

	Shm->Proc.Technology.IOMMU = !Proc->Uncore.Bus.QuickPath.VT_d;
}

void DMI_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.Bus.DimmClock.QCLK_RATIO) {
	case 0b00010:
		Shm->Uncore.CtrlSpeed = 266;
		break;
	case 0b00100:
		Shm->Uncore.CtrlSpeed = 533;
		break;
	case 0b00110:
		Shm->Uncore.CtrlSpeed = 800;
		break;
	case 0b01000:
		Shm->Uncore.CtrlSpeed = 1066;
		break;
	case 0b01010:
		Shm->Uncore.CtrlSpeed = 1333;
		break;
	case 0b000000:
		// Fallthrough
	default:
		Shm->Uncore.CtrlSpeed = 266;
		break;
	}
	Shm->Uncore.CtrlSpeed *= Shm->Cpu[cpu].Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = 2500;	// ToDo: hardwired to Lynnfield

	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void SNB_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {
      unsigned short dimmSize[2][2] = {
	{
		Proc->Uncore.MC[mc].SNB.MAD0.Dimm_A_Size,
		Proc->Uncore.MC[mc].SNB.MAD0.Dimm_B_Size
	}, {
		Proc->Uncore.MC[mc].SNB.MAD1.Dimm_A_Size,
		Proc->Uncore.MC[mc].SNB.MAD1.Dimm_B_Size
	}
      };

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].SNB.DBP.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.DBP.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].SNB.DBP.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.DBP.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.RFTP.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr =
			Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.tWTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  =
			Proc->Uncore.MC[mc].Channel[cha].SNB.DBP.tCWL;

	switch(Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.CMD_Stretch) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 3;
		break;
	default:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 0;
		break;
	}

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		unsigned int width;

	    if (slot == 0) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks =
					Proc->Uncore.MC[mc].SNB.MAD0.DANOR;

		width = Proc->Uncore.MC[mc].SNB.MAD0.DAW;
	    } else {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks =
					Proc->Uncore.MC[mc].SNB.MAD0.DBNOR;

		width = Proc->Uncore.MC[mc].SNB.MAD0.DBW;
	    }
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks++;

	    if (width == 0) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 14;
	    } else {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 15;
	    }

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 10;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size =
						dimmSize[cha][slot] * 256;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks =
			(8 * dimmSize[cha][slot] * 1024 * 1024)
			/ (Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (cha == 0) ?
					Proc->Uncore.MC[mc].SNB.MAD0.ECC
				:	Proc->Uncore.MC[mc].SNB.MAD1.ECC;
      }
    }
}

void SNB_CAP(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.Bus.SNB_Cap.DMFC) {
	case 0b111:
		Shm->Uncore.CtrlSpeed = 1067;
		break;
	case 0b110:
		Shm->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		Shm->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		Shm->Uncore.CtrlSpeed = 1867;
		break;
	case 0b011:
		Shm->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		Shm->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		Shm->Uncore.CtrlSpeed = 2667;
		break;
	case 0b000:
		Shm->Uncore.CtrlSpeed = 2933;
		break;
	}

	Shm->Uncore.Bus.Rate = 5000;
	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

	Shm->Proc.Technology.IOMMU = !Proc->Uncore.Bus.SNB_Cap.VT_d;
}

void IVB_CAP(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.Bus.IVB_Cap.DMFC) {
	case 0b111:
		Shm->Uncore.CtrlSpeed = 1067;
		break;
	case 0b110:
		Shm->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		Shm->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		Shm->Uncore.CtrlSpeed = 1867;
		break;
	case 0b011:
		Shm->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		Shm->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		Shm->Uncore.CtrlSpeed = 2667;
		break;
	case 0b000:
		switch (Proc->ArchID) {
		case IvyBridge:
		case IvyBridge_EP:
			Shm->Uncore.CtrlSpeed = 2933;
			break;
		case Haswell_DT:
		case Haswell_EP:
		case Haswell_ULT:
		case Haswell_ULX:
			Shm->Uncore.CtrlSpeed = 2667;
			break;
		case Broadwell:
		case Broadwell_D:
		case Broadwell_H:
		case Broadwell_EP:
			Shm->Uncore.CtrlSpeed = 3200;
			break;
		}
		break;
	}

	Shm->Uncore.Bus.Rate = 5000;
	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

	Shm->Proc.Technology.IOMMU = !Proc->Uncore.Bus.SNB_Cap.VT_d;
}

void HSW_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank.tCWL;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   =
			Proc->Uncore.MC[mc].Channel[cha].HSW._.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].HSW._.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].HSW._.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].HSW._.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].HSW._.tRAS;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Timing.tsrRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Timing.tdrRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Timing.tddRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tsrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tdrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tddWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tsrWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tdrWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tddWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tsrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tdrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tddRdTWr;

	switch(Proc->Uncore.MC[mc].Channel[cha].HSW.Timing.CMD_Stretch) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 3;
		break;
	default:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 0;
		break;
	}
/* ToDo */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = 8
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size /= (1024 *1024);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void SKL_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
	unsigned int DimmWidthToRows(unsigned int width)
	{
		switch (width) {
		case 0b00:
			return(1 << 14);
		case 0b01:
			return(1 << 15);
		case 0b10:
			return(1 << 16);
		case 0b11:
			return(1 << 0);
		}
		return(1 << 0);
	}

    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
	Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
	Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

     for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
     {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].SKL.ODT.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Timing.tRAS;
/*
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].SKL._.tRRD;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Refresh.tRFC;
/*
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].SKL._.tWR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Timing.tRDPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Timing.tWRPRE;
/*
	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
			Proc->Uncore.MC[mc].Channel[cha].SKL._.tFAW;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  =
			Proc->Uncore.MC[mc].Channel[cha].SKL.ODT.tCWL;

	switch(Proc->Uncore.MC[mc].Channel[cha].SKL.Sched.CMD_Stretch) {
	case 0b00:
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 3;
		break;
	}

      for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
      {
	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols =
			Proc->Uncore.MC[mc].Channel[cha].SKL.Sched.Dimm_x8 ?
				  (1 << 13)
				: (1 << 10);
      }
     }
	Shm->Uncore.MC[mc].Channel[0].Timing.ECC =
				Proc->Uncore.MC[mc].SKL.MADC0.ECC;

	Shm->Uncore.MC[mc].Channel[1].Timing.ECC =
				Proc->Uncore.MC[mc].SKL.MADC1.ECC;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Size = 1024 * Proc->Uncore.MC[mc].SKL.MADD0.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Size = 1024 * Proc->Uncore.MC[mc].SKL.MADD0.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Size = 1024 * Proc->Uncore.MC[mc].SKL.MADD1.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Size = 1024 * Proc->Uncore.MC[mc].SKL.MADD1.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Ranks = 1 + Proc->Uncore.MC[mc].SKL.MADD0.DLNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Ranks = 1 + Proc->Uncore.MC[mc].SKL.MADD0.DSNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Ranks = 1 + Proc->Uncore.MC[mc].SKL.MADD1.DLNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Ranks = 1 + Proc->Uncore.MC[mc].SKL.MADD1.DSNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD0.DLW);

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD0.DSW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD1.DLW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD1.DSW);
    }
}

void SKL_CAP(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	unsigned int DMFC;

	switch (Proc->ArchID) {
	case Skylake_UY:
	case Kabylake_UY:
		DMFC = Proc->Uncore.Bus.SKL_Cap_B.DMFC_DDR3;
		Shm->Uncore.Bus.Rate = 4000;	// 4 GT/s QPI
		break;
	default:
		DMFC = Proc->Uncore.Bus.SKL_Cap_C.DMFC_DDR4;
		Shm->Uncore.Bus.Rate = 8000;	// 8 GT/s DMI3
		break;
	}

	switch (DMFC) {
	case 0b111:
		Shm->Uncore.CtrlSpeed = 1067;
		break;
	case 0b110:
		Shm->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		Shm->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		Shm->Uncore.CtrlSpeed = 1867;
		break;
	case 0b011:
		Shm->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		Shm->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
	case 0b000:
		Shm->Uncore.CtrlSpeed = 2667;
		break;
	}

	Shm->Uncore.Bus.Speed = ( Shm->Cpu[cpu].Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

	Shm->Proc.Technology.IOMMU = !Proc->Uncore.Bus.SKL_Cap_A.VT_d;
}

void AMD_0F_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    struct {
	unsigned int size;
    } module[] = {
	{256}, {512}, {1024}, {1024},
	{1024}, {2048}, {2048}, {4096},
	{4096}, {8192}, {8192}, {16384},
	{0}, {0}, {0}, {0}
    };
    unsigned int mask;
    unsigned short mc, cha, slot, shift, index;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	switch (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tCL) {
	case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 3;
		break;
	case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 4;
		break;
	case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 5;
		break;
	case 0b101:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = 6;
		break;
	}

	switch (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRCD) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 3;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 4;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 5;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = 6;
		break;
	}

	switch (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRP) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 3;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 4;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 5;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRP = 6;
		break;
	}

	switch (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRTPr) {
	case 0b0:
		if (Proc->Uncore.MC[mc].AMD0F.DCRL.BurstLength32 == 0b1)
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 2;
		else
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 4;
		break;
	case 0b1:
		if (Proc->Uncore.MC[mc].AMD0F.DCRL.BurstLength32 == 0b1)
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 3;
		else
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 5;
		break;
	}

	if (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRAS >= 0b0010)
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS =
			Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRAS + 3;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC =
			Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRC + 11;

	switch (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tWR) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 3;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 4;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 5;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = 6;
		break;
	}

	switch (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRRD) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 2;
		break;
	case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 3;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 4;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 5;
		break;
	}

	if ((Proc->Uncore.MC[mc].AMD0F.DCRH.tFAW > 0b0000)
	 && (Proc->Uncore.MC[mc].AMD0F.DCRH.tFAW <= 0b1101)) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
				Proc->Uncore.MC[mc].AMD0F.DCRH.tFAW + 7;
	}

	if (Proc->Uncore.MC[mc].AMD0F.DCRH.SlowAccessMode == 0b1)
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
	else
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;

	shift = 4 * cha;
	mask  = 0b1111 << shift;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
	  if (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].MBA.CSEnable) {
	    index=(Proc->Uncore.MC[mc].MaxDIMMs.AMD0F.CS.value & mask) >> shift;

	    Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=module[index].size;
	  }
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC =
				Proc->Uncore.MC[mc].AMD0F.DCRL.ECC_DIMM_Enable;
      }
    }
}

void AMD_0F_HTT(SHM_STRUCT *Shm, PROC *Proc)
{
	unsigned int link = 0;
	RAM_Ratio Ratio = {.Q = 0, .R = 0};
	unsigned long long HTT_Clock = 0;

	switch (Proc->Uncore.MC[0].AMD0F.DCRH.MemClkFreq) {
		case 0b000:
			Ratio.Q = 200;
			Ratio.R = 0;
			break;
		case 0b001:
			Ratio.Q = 266;
			Ratio.R = 1;
			break;
		case 0b010:
			Ratio.Q = 333;
			Ratio.R = 1;
			break;
		case 0b011:
			Ratio.Q = 400;
			Ratio.R = 0;
			break;
	}
	Shm->Uncore.CtrlSpeed = (Ratio.Q * 2) + Ratio.R;	// DDR2

	if ((link = Proc->Uncore.Bus.UnitID.McUnit) < 0b11) {
		switch (Proc->Uncore.Bus.LDTi_Freq[link].LinkFreqMax) { // "MHz"
		case 0b0000:
			HTT_Clock = 200;
			break;
		case 0b0010:
			HTT_Clock = 400;
			break;
		case 0b0100:
			HTT_Clock = 600;
			break;
		case 0b0101:
			HTT_Clock = 800;
			break;
		case 0b0110:
			HTT_Clock = 1000;
			break;
		case 0b1111:
			HTT_Clock = 100;
			break;
		}
		Shm->Uncore.Bus.Rate = HTT_Clock * 2;	// "MT/s"
		Shm->Uncore.Bus.Speed = HTT_Clock * 4;	// "MB/s"
	}
	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b10;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void AMD_17h_IOMMU(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.Technology.IOMMU = BITVAL(Proc->Uncore.Bus.IOMMU_CR, 0);
}

void Uncore(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.ChipID) {
	case PCI_DEVICE_ID_INTEL_82945P_HB:
	case PCI_DEVICE_ID_INTEL_82945GM_HB:
	case PCI_DEVICE_ID_INTEL_82945GME_HB:
		P945_CLK(Shm, Proc, cpu);
		P945_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_82955_HB:
		P945_CLK(Shm, Proc, cpu);
		P955_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_82946GZ_HB:
	case PCI_DEVICE_ID_INTEL_82965Q_HB:
	case PCI_DEVICE_ID_INTEL_82965G_HB:
		P965_CLK(Shm, Proc, cpu);
		P965_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_82965GM_HB:
	case PCI_DEVICE_ID_INTEL_82965GME_HB:
	case PCI_DEVICE_ID_INTEL_GM45_HB:
		G965_CLK(Shm, Proc, cpu);
		G965_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_Q35_HB:
	case PCI_DEVICE_ID_INTEL_G33_HB:
	case PCI_DEVICE_ID_INTEL_Q33_HB:
	case PCI_DEVICE_ID_INTEL_X38_HB:
	case PCI_DEVICE_ID_INTEL_3200_HB:
		P35_CLK(Shm, Proc, cpu);
		P35_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_Q45_HB:
	case PCI_DEVICE_ID_INTEL_G45_HB:
	case PCI_DEVICE_ID_INTEL_G41_HB:
		P35_CLK(Shm, Proc, cpu);
		P4S_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_I7_MCR:		// Bloomfield
	case PCI_DEVICE_ID_INTEL_NHM_EP_MCR:		// Westmere EP
		QPI_CLK(Shm, Proc, cpu);
		NHM_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR:		// Lynnfield
		DMI_CLK(Shm, Proc, cpu);
		NHM_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0:	// Sandy Bridge
		break;
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA:	// SNB Desktop
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104:
		SNB_CAP(Shm, Proc, cpu);
		SNB_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0:	// Ivy Bridge
		break;
	case PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA:	// IVB Desktop
	case PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154:	// IVB Mobile i5-3337U
		IVB_CAP(Shm, Proc, cpu);
		SNB_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA:	// HSW & BDW Desktop
		IVB_CAP(Shm, Proc, cpu);
		// Fallthrough
	case PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0:	// Haswell
	case PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0:	// Broadwell/U , Core m
	case PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0:	// Broadwell/H
		HSW_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA:	// Skylake/U Processor
	case PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA:	// Skylake/Y Processor
	case PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD:	// Skylake/S Dual Core
	case PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ:	// Skylake/S Quad Core
	case PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD:	// Skylake/H Dual Core
	case PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ:	// Skylake/H Quad Core
	case PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA:	// BGA 1356
	case PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA:	// BGA 1515
	case PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD:	// Kabylake/H Dual Core
	case PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD:	// Kabylake/S Dual Core
	case PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ:	// Kabylake/H Quad Core
	case PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ:	// U-Quad Core BGA 1356
	case PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ:	// Kabylake/S Quad Core
	case PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ:
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ:	// CoffeeLake Quad Core
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAH:	// CoffeeLake Hexa Core
		SKL_CAP(Shm, Proc, cpu);
		SKL_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_AMD_K8_NB_MEMCTL:
		AMD_0F_HTT(Shm, Proc);
		AMD_0F_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_AMD_17H_IOMMU:
		AMD_17h_IOMMU(Shm, Proc);
		break;
	}

	// Copy the Uncore clock ratios.
	memcpy( Shm->Uncore.Boost,
		Proc->Uncore.Boost,
		(UNCORE_BOOST(SIZE)) * sizeof(unsigned int) );
}

void BaseClock(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{	// Copy the estimated base clock per CPU.
	Shm->Cpu[cpu].Clock.Q  = Core[cpu]->Clock.Q;
	Shm->Cpu[cpu].Clock.R  = Core[cpu]->Clock.R;
	Shm->Cpu[cpu].Clock.Hz = Core[cpu]->Clock.Hz;
}

void CPUID_Dump(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{	// Copy the Vendor CPUID dump per Core.
	Shm->Cpu[cpu].Query.StdFunc = Core[cpu]->Query.StdFunc;
	Shm->Cpu[cpu].Query.ExtFunc = Core[cpu]->Query.ExtFunc;

	int i;
	for (i = 0; i < CPUID_MAX_FUNC; i++) {
		Shm->Cpu[cpu].CpuID[i].func   = Core[cpu]->CpuID[i].func;
		Shm->Cpu[cpu].CpuID[i].sub    = Core[cpu]->CpuID[i].sub;
		Shm->Cpu[cpu].CpuID[i].reg[0] = Core[cpu]->CpuID[i].reg[0];
		Shm->Cpu[cpu].CpuID[i].reg[1] = Core[cpu]->CpuID[i].reg[1];
		Shm->Cpu[cpu].CpuID[i].reg[2] = Core[cpu]->CpuID[i].reg[2];
		Shm->Cpu[cpu].CpuID[i].reg[3] = Core[cpu]->CpuID[i].reg[3];
	}
}

void Topology(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{	// Copy each Core topology.
	Shm->Cpu[cpu].Topology.MP.BSP    = (Core[cpu]->T.Base.BSP) ? 1 : 0;
	Shm->Cpu[cpu].Topology.ApicID    = Core[cpu]->T.ApicID;
	Shm->Cpu[cpu].Topology.CoreID    = Core[cpu]->T.CoreID;
	Shm->Cpu[cpu].Topology.ThreadID  = Core[cpu]->T.ThreadID;
	Shm->Cpu[cpu].Topology.PackageID = Core[cpu]->T.PackageID;
	Shm->Cpu[cpu].Topology.MP.x2APIC = ((Proc->Features.Std.ECX.x2APIC
					    & Core[cpu]->T.Base.EN)
					   << Core[cpu]->T.Base.EXTD);
	unsigned int loop;
	for (loop = 0; loop < CACHE_MAX_LEVEL; loop++)
	{
	    if (Core[cpu]->T.Cache[loop].Type > 0)
	    {
		unsigned int level=Core[cpu]->T.Cache[loop].Level;
		if (Core[cpu]->T.Cache[loop].Type == 2) // Instruction
			level = 0;

		if(Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
		{
			Shm->Cpu[cpu].Topology.Cache[level].Set =
				Core[cpu]->T.Cache[loop].Set + 1;

			Shm->Cpu[cpu].Topology.Cache[level].LineSz =
				Core[cpu]->T.Cache[loop].LineSz + 1;

			Shm->Cpu[cpu].Topology.Cache[level].Part =
				Core[cpu]->T.Cache[loop].Part + 1;

			Shm->Cpu[cpu].Topology.Cache[level].Way =
				Core[cpu]->T.Cache[loop].Way + 1;

			Shm->Cpu[cpu].Topology.Cache[level].Size =
			  Shm->Cpu[cpu].Topology.Cache[level].Set
			* Shm->Cpu[cpu].Topology.Cache[level].LineSz
			* Shm->Cpu[cpu].Topology.Cache[level].Part
			* Shm->Cpu[cpu].Topology.Cache[level].Way;
		}
		else {
		    if(Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD) {

			unsigned int Compute_Way(unsigned int value)
			{
				switch (value) {
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

			Shm->Cpu[cpu].Topology.Cache[level].Way =
			    (loop != 2) ?
				  Core[cpu]->T.Cache[loop].Way
				: Compute_Way(Core[cpu]->T.Cache[loop].Way);

			Shm->Cpu[cpu].Topology.Cache[level].Size =
				Core[cpu]->T.Cache[loop].Size;
		    }
		}
		Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack =
			Core[cpu]->T.Cache[loop].WrBack;
		Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive =
			Core[cpu]->T.Cache[loop].Inclus;
	    }
	}
	// Apply various architecture size unit.
	switch (Proc->ArchID) {
	case AMD_Family_17h:
		Shm->Cpu[cpu].Topology.Cache[3].Size *= 512;
		break;
	}
}

void CStates(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	Shm->Cpu[cpu].Query.CfgLock = Core[cpu]->Query.CfgLock;
	Shm->Cpu[cpu].Query.CStateLimit = Core[cpu]->Query.CStateLimit;

	Shm->Cpu[cpu].Query.IORedir = Core[cpu]->Query.IORedir;
	Shm->Cpu[cpu].Query.CStateInclude = Core[cpu]->Query.CStateInclude;
}

void PowerThermal(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	Shm->Cpu[cpu].PowerThermal.DutyCycle.Extended =
			Core[cpu]->PowerThermal.ClockModulation.ECMD;

	Shm->Cpu[cpu].PowerThermal.DutyCycle.ClockMod =
		Core[cpu]->PowerThermal.ClockModulation.DutyCycle
			>> !Shm->Cpu[cpu].PowerThermal.DutyCycle.Extended;

	Shm->Cpu[cpu].PowerThermal.PowerPolicy =
			Core[cpu]->PowerThermal.PerfEnergyBias.PowerPolicy;

	Shm->Cpu[cpu].PowerThermal.TM1 =
		Proc->Features.Std.EDX.TM1;			//0001

	Shm->Cpu[cpu].PowerThermal.TM1 |=
		(Core[cpu]->PowerThermal.TCC_Enable << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.TM1 ^=
		(Core[cpu]->PowerThermal.TM_Select << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.TM2 =
		Proc->Features.Std.ECX.TM2;			//0001

	Shm->Cpu[cpu].PowerThermal.TM2 |=
		(Core[cpu]->PowerThermal.TM2_Enable << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.Target = Core[cpu]->PowerThermal.Target;
}

void InitThermal(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
    switch (Proc->thermalFormula) {
    case THERMAL_FORMULA_INTEL:
    case THERMAL_FORMULA_AMD:
	Shm->Cpu[cpu].PowerThermal.Limit[0] = Core[cpu]->PowerThermal.Target;
	Shm->Cpu[cpu].PowerThermal.Limit[1] = 0;
    break;
    case THERMAL_FORMULA_AMD_0Fh:
	COMPUTE_THERMAL(AMD_0Fh,
			Shm->Cpu[cpu].PowerThermal.Limit[0],
			Core[cpu]->PowerThermal.Target,
			Core[cpu]->PowerThermal.Sensor);
    break;
    case THERMAL_FORMULA_AMD_17h:
      if (cpu == Proc->Service.Core) {
	COMPUTE_THERMAL(AMD_17h,
			Shm->Cpu[cpu].PowerThermal.Limit[0],
			Core[cpu]->PowerThermal.Target,
			Core[cpu]->PowerThermal.Sensor);
      }
    break;
    }
}

void SystemRegisters(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	Shm->Cpu[cpu].SystemRegister.RFLAGS = Core[cpu]->SystemRegister.RFLAGS;
	Shm->Cpu[cpu].SystemRegister.CR0    = Core[cpu]->SystemRegister.CR0;
	Shm->Cpu[cpu].SystemRegister.CR3    = Core[cpu]->SystemRegister.CR3;
	Shm->Cpu[cpu].SystemRegister.CR4    = Core[cpu]->SystemRegister.CR4;
	Shm->Cpu[cpu].SystemRegister.EFER   = Core[cpu]->SystemRegister.EFER;
	Shm->Cpu[cpu].SystemRegister.EFCR   = Core[cpu]->SystemRegister.EFCR;
}

void SysGate_IdleDriver(REF *Ref)
{
    SHM_STRUCT *Shm = Ref->Shm;
    SYSGATE *SysGate = Ref->SysGate;

    if (strlen(SysGate->IdleDriver.Name) > 0) {
	int i;

	strncpy(Shm->SysGate.IdleDriver.Name,
		SysGate->IdleDriver.Name, CPUIDLE_NAME_LEN - 1);

	Shm->SysGate.IdleDriver.stateCount = SysGate->IdleDriver.stateCount;
	for (i = 0; i < Shm->SysGate.IdleDriver.stateCount; i++)
	{
		strncpy(Shm->SysGate.IdleDriver.State[i].Name,
			SysGate->IdleDriver.State[i].Name,
			CPUIDLE_NAME_LEN - 1);

		Shm->SysGate.IdleDriver.State[i].exitLatency =
			SysGate->IdleDriver.State[i].exitLatency;

		Shm->SysGate.IdleDriver.State[i].powerUsage =
			SysGate->IdleDriver.State[i].powerUsage;

		Shm->SysGate.IdleDriver.State[i].targetResidency =
			SysGate->IdleDriver.State[i].targetResidency;
	}
    }
    if (strlen(SysGate->IdleDriver.Governor) > 0)
	strncpy(Shm->SysGate.IdleDriver.Governor,
		SysGate->IdleDriver.Governor, CPUIDLE_NAME_LEN - 1);
}

void SysGate_Kernel(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	SYSGATE *SysGate = Ref->SysGate;

	Shm->SysGate.kernel.version = SysGate->kernelVersionNumber >> 16;
	Shm->SysGate.kernel.major = (SysGate->kernelVersionNumber >> 8) & 0xff;
	Shm->SysGate.kernel.minor = SysGate->kernelVersionNumber & 0xff;

	memcpy(Shm->SysGate.sysname, SysGate->sysname, MAX_UTS_LEN);
	memcpy(Shm->SysGate.release, SysGate->release, MAX_UTS_LEN);
	memcpy(Shm->SysGate.version, SysGate->version, MAX_UTS_LEN);
	memcpy(Shm->SysGate.machine, SysGate->machine, MAX_UTS_LEN);
}

void SysGate_Update(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	SYSGATE *SysGate = Ref->SysGate;

	Shm->SysGate.taskCount = SysGate->taskCount;

	memcpy( Shm->SysGate.taskList, SysGate->taskList,
		Shm->SysGate.taskCount * sizeof(TASK_MCB));

	int reverseSign[2] = {+1, -1};

	int SortByRuntime(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
		int sort = task1->runtime < task2->runtime ? +1 : -1;
		sort *= reverseSign[Shm->SysGate.reverseOrder];
		return(sort);
	}

	int SortByUsertime(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
		int sort = task1->usertime < task2->usertime ? +1 : -1;
		sort *= reverseSign[Shm->SysGate.reverseOrder];
		return(sort);
	}

	int SortBySystime(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
		int sort = task1->systime < task2->systime ? +1 : -1;
		sort *= reverseSign[Shm->SysGate.reverseOrder];
		return(sort);
	}

	int SortByState(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
		int sort = task1->state < task2->state ? -1 : +1;
		sort *= reverseSign[Shm->SysGate.reverseOrder];
		return(sort);
	}

	int SortByPID(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
		int sort = task1->pid < task2->pid ? -1 : +1;
		sort *= reverseSign[Shm->SysGate.reverseOrder];
		return(sort);
	}

	int SortByCommand(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
		int sort = strcmp(task1->comm, task2->comm);
		sort *= reverseSign[Shm->SysGate.reverseOrder];
		return(sort);
	}

	typedef int (*SORTBYFUNC)(const void *, const void *);

	SORTBYFUNC SortByFunc[SORTBYCOUNT] = {
		SortByState,
		SortByRuntime,
		SortByUsertime,
		SortBySystime,
		SortByPID,
		SortByCommand
	};

	int SortByTracker(const void *p1, const void *p2)
	{
		TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;

		int sort = (task1->pid == Shm->SysGate.trackTask) ?
			-1 : (task2->pid == Shm->SysGate.trackTask) ?
			+1 :  SortByFunc[Shm->SysGate.sortByField](p1, p2);
		return(sort);
	}

	qsort(Shm->SysGate.taskList, Shm->SysGate.taskCount, sizeof(TASK_MCB),
		Shm->SysGate.trackTask ?
			  SortByTracker
			: SortByFunc[Shm->SysGate.sortByField]);

	Shm->SysGate.memInfo.totalram  = SysGate->memInfo.totalram;
	Shm->SysGate.memInfo.sharedram = SysGate->memInfo.sharedram;
	Shm->SysGate.memInfo.freeram   = SysGate->memInfo.freeram;
	Shm->SysGate.memInfo.bufferram = SysGate->memInfo.bufferram;
	Shm->SysGate.memInfo.totalhigh = SysGate->memInfo.totalhigh;
	Shm->SysGate.memInfo.freehigh  = SysGate->memInfo.freehigh;
}

void PerCore_Update(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	if (BITVAL(Core[cpu]->OffLine, HW))
		BITSET(LOCKLESS, Shm->Cpu[cpu].OffLine, HW);
	else
		BITCLR(LOCKLESS, Shm->Cpu[cpu].OffLine, HW);

	Shm->Cpu[cpu].Query.Microcode = Core[cpu]->Query.Microcode;

	BaseClock(Shm, Core, cpu);

	Topology(Shm, Proc, Core, cpu);

	CStates(Shm, Core, cpu);

	PowerThermal(Shm, Proc, Core, cpu);

	SystemRegisters(Shm, Core, cpu);

	CPUID_Dump(Shm, Core, cpu);

	if (cpu == Proc->Service.Core)
		Uncore(Shm, Proc, cpu);
}

int SysGate_OnDemand(REF *Ref, int operation)
{
	int rc = -1;
	const size_t allocPages = PAGE_SIZE << Ref->Proc->OS.ReqMem.Order;
	if (operation == 0) {
	    if (Ref->SysGate != NULL) {
		if ((rc = munmap(Ref->SysGate, allocPages)) == 0)
			Ref->SysGate = NULL;
	    }
	    else
		rc = -1;
	} else {
	    if (Ref->SysGate == NULL) {
		const off_t vm_pgoff = 1 * PAGE_SIZE;
		SYSGATE *MapGate = mmap(NULL, allocPages,
					PROT_READ|PROT_WRITE,
					MAP_SHARED,
					Ref->fd->Drv, vm_pgoff);
		if (MapGate != MAP_FAILED) {
			Ref->SysGate = MapGate;
			rc = 0;
		}
	    }
	    else
		rc = 0;
	}
	return(rc);
}

void SysGate_Toggle(REF *Ref, unsigned int state)
{
    if (state == 0) {
	if (BITWISEAND(LOCKLESS, Ref->Shm->SysGate.Operation, 0x1)) {
		// Stop SysGate
		BITCLR(LOCKLESS, Ref->Shm->SysGate.Operation, 0);
		// Notify
		if (!BITVAL(Ref->Shm->Proc.Sync, 63))
			BITSET(LOCKLESS, Ref->Shm->Proc.Sync, 63);
	}
    } else {
	if (!BITWISEAND(LOCKLESS, Ref->Shm->SysGate.Operation, 0x1)) {
	    if (SysGate_OnDemand(Ref, 1) == 0) {
		if (ioctl(Ref->fd->Drv, COREFREQ_IOCTL_SYSONCE) != -1) {
			// Aggregate the OS idle driver data.
			SysGate_IdleDriver(Ref);
			// Copy system information.
			SysGate_Kernel(Ref);
			// Start SysGate
			BITSET(LOCKLESS, Ref->Shm->SysGate.Operation, 0);
			// Notify
			if (!BITVAL(Ref->Shm->Proc.Sync, 63))
				BITSET(LOCKLESS, Ref->Shm->Proc.Sync, 63);
		}
	    }
	}
    }
}

void Master_Ring_Handler(REF *Ref, unsigned int rid)
{
    void UpdateFeatures(void)
    {
	unsigned int cpu;

	Package_Update(Ref->Shm, Ref->Proc);
	for (cpu = 0; cpu < Ref->Shm->Proc.CPU.Count; cpu++)
	    if (BITVAL(Ref->Core[cpu]->OffLine, OS) == 0)
	    {
		PerCore_Update(Ref->Shm, Ref->Proc, Ref->Core, cpu);
	    }
	Technology_Update(Ref->Shm, Ref->Proc);
    }

    if (!RING_NULL(Ref->Shm->Ring[rid])) {
	struct RING_CTRL ctrl = RING_READ(Ref->Shm->Ring[rid]);
	int rc = ioctl(Ref->fd->Drv, ctrl.cmd, ctrl.arg);
	if (Quiet & 0x100)
		printf("\tRING[%u](%x,%lx)>%d\n",rid,ctrl.cmd,ctrl.arg, rc);
	switch (rc) {
	case -EPERM:
		break;
	case 0: // Update SHM and notify a platform changed.
		UpdateFeatures();
		if (!BITVAL(Ref->Shm->Proc.Sync, 63))
			BITSET(LOCKLESS, Ref->Shm->Proc.Sync, 63);
		break;
	case 2: // Update SHM and notify to re-compute.
		UpdateFeatures();
		if (!BITVAL(Ref->Shm->Proc.Sync, 62))
			BITSET(LOCKLESS, Ref->Shm->Proc.Sync, 62);
		break;
	}
    }
}

void Child_Ring_Handler(REF *Ref, unsigned int rid)
{
  if (!RING_NULL(Ref->Shm->Ring[rid]))
  {
	struct RING_CTRL ctrl = RING_READ(Ref->Shm->Ring[rid]);

   switch (ctrl.cmd)
   {
   case COREFREQ_ORDER_MACHINE:
	switch (ctrl.arg) {
	case COREFREQ_TOGGLE_OFF:
	    if (BITVAL(Ref->Shm->Proc.Sync, 31)) {
		BITCLR(BUS_LOCK, Ref->Shm->Proc.Sync, 31);

		while (BITWISEAND(BUS_LOCK, roomCore, roomSeed))
		{
			if (BITVAL(Shutdown, 0))	/*SpinLock*/
				break;
		}
		roomSched = 0;
		Ref->Slice.Func = Slice_NOP;
		Ref->Slice.arg = 0;
		Ref->Slice.pattern = RESET_CSP;
		// Notify
		if (!BITVAL(Ref->Shm->Proc.Sync, 63))
			BITSET(LOCKLESS, Ref->Shm->Proc.Sync, 63);
	    }
	    break;
	}
	break;
   default:
    {
	RING_SLICE *porder = order_list;

     while (porder->func != NULL)
     {
      if ((porder->ctrl.cmd == ctrl.cmd) &&  (porder->ctrl.arg == ctrl.arg))
      {
       if (!BITVAL(Ref->Shm->Proc.Sync, 31))
       {
	while (BITWISEAND(BUS_LOCK, roomCore, roomSeed))
	{
		if (BITVAL(Shutdown, 0))	/*SpinLock*/
			break;
	}
	SliceScheduling(Ref->Shm, Ref->Shm->Proc.Service.Core, porder->pattern);

	Ref->Slice.Func = porder->func;
	Ref->Slice.arg  = porder->ctrl.arg;
	Ref->Slice.pattern = porder->pattern;

	BITSET(BUS_LOCK, Ref->Shm->Proc.Sync, 31);
	// Notify
	if (!BITVAL(Ref->Shm->Proc.Sync, 63))
		BITSET(LOCKLESS, Ref->Shm->Proc.Sync, 63);
       }
       break;
      }
	porder++;
     }
    }
    break;
   }
    if (Quiet & 0x100)
	printf("\tRING[%u](%x,%lx)\n", rid, ctrl.cmd, ctrl.arg);
  }
}

int ServerFollowService(SERVICE_PROC *pSlave,
			SERVICE_PROC *pMaster,
			pthread_t tid)
{
	if (pSlave->Proc != pMaster->Proc) {
		pSlave->Proc = pMaster->Proc;

		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(pSlave->Core, &cpuset);
		if (pSlave->Thread != -1)
			CPU_SET(pSlave->Thread, &cpuset);

		return(pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset));
	}
	return(0);
}

static void *Emergency_Handler(void *pRef)
{
	REF *Ref = (REF *) pRef;
	unsigned int rid = (Ref->CPID == 0);
	SERVICE_PROC localService = {.Proc = -1};
	int caught = 0, leave = 0;
	char handlerName[TASK_COMM_LEN] = {
		'c','o','r','e','f','r','e','q','d','-','r','i','n','g','0',0
	};
	handlerName[14] += rid;

	pthread_t tid = pthread_self();

    if (ServerFollowService(&localService, &Ref->Shm->Proc.Service, tid) == 0)
	pthread_setname_np(tid, handlerName);

    while (!leave) {
	caught = sigtimedwait(&Ref->Signal, NULL, &Ref->Shm->Sleep.ringWaiting);
	if (caught != -1) {
		switch (caught) {
		case SIGUSR2:
			if (Ref->CPID)
				SysGate_Toggle(Ref, 0);
			break;
		case SIGUSR1:
			if (Ref->CPID)
				SysGate_Toggle(Ref, 1);
			break;
		case SIGCHLD:
			leave = 0x1;
			break;
		case SIGSEGV:
		case SIGTERM:
		case SIGINT:	// [CTRL] + [C]
			// Fallthrough
		case SIGQUIT:
			BITSET(LOCKLESS, Shutdown, 0);
			break;
		}
	} else if (errno == EAGAIN) {
		if (Ref->CPID) {
			Master_Ring_Handler(Ref, rid);
		} else {
			Child_Ring_Handler(Ref, rid);
		}
	}
	ServerFollowService(&localService, &Ref->Shm->Proc.Service, tid);
    }
	return(NULL);
}

void Emergency_Command(REF *Ref, unsigned int cmd)
{
	switch (cmd) {
	case 0:
		if (Ref->Started) {
			if (!pthread_kill(Ref->KID, SIGCHLD))
				if (!pthread_join(Ref->KID, NULL))
					Ref->Started = 0;
		}
		break;
	case 1: {
		sigemptyset(&Ref->Signal);
		sigaddset(&Ref->Signal, SIGUSR1);	// Start SysGate
		sigaddset(&Ref->Signal, SIGUSR2);	// Stop  SysGate
		sigaddset(&Ref->Signal, SIGINT);	// Shutdown
		sigaddset(&Ref->Signal, SIGQUIT);	// Shutdown
		sigaddset(&Ref->Signal, SIGTERM);	// Shutdown
		sigaddset(&Ref->Signal, SIGSEGV);	// Shutdown
		sigaddset(&Ref->Signal, SIGCHLD);	// Exit Ring Thread

		if (!pthread_sigmask(SIG_BLOCK, &Ref->Signal, NULL)
		 && !pthread_create(&Ref->KID, NULL, Emergency_Handler, Ref))
			Ref->Started = 1;
		}
		break;
	}
}

void Core_Manager(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	PROC *Proc	= Ref->Proc;
	CORE **Core	= Ref->Core;
	SERVICE_PROC localService = {.Proc = -1};
	double maxRelFreq;
	unsigned int cpu = 0;

	pthread_t tid = pthread_self();
	Shm->AppSvr = tid;

	if (ServerFollowService(&localService, &Shm->Proc.Service, tid) == 0)
		pthread_setname_np(tid, "corefreqd-pmgr");

	ARG *Arg = calloc(Shm->Proc.CPU.Count, sizeof(ARG));

    while (!BITVAL(Shutdown, 0))
    {	// Loop while all the cpu room bits are not cleared.
	while (!BITVAL(Shutdown, 0)
	    && BITWISEAND(BUS_LOCK, roomCore, roomSeed))
	{
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}
	// Reset the averages & the max frequency
	Shm->Proc.Avg.Turbo = 0;
	Shm->Proc.Avg.C0    = 0;
	Shm->Proc.Avg.C3    = 0;
	Shm->Proc.Avg.C6    = 0;
	Shm->Proc.Avg.C7    = 0;
	Shm->Proc.Avg.C1    = 0;
	maxRelFreq = 0.0;

	for (cpu=0; !BITVAL(Shutdown,0) && (cpu < Shm->Proc.CPU.Count); cpu++)
	{
	    if (BITVAL(Core[cpu]->OffLine, OS) == 1) {
		if (Arg[cpu].TID) {
			// Remove this cpu.
			pthread_join(Arg[cpu].TID, NULL);
			Arg[cpu].TID = 0;

			Package_Update(Shm, Proc);
			PerCore_Update(Shm, Proc, Core, cpu);
			Technology_Update(Shm, Proc);

			ServerFollowService(&localService,
					    &Shm->Proc.Service,
					    tid);

			// Raise this bit up to notify a platform change.
			if (!BITVAL(Shm->Proc.Sync, 63))
				BITSET(LOCKLESS, Shm->Proc.Sync, 63);
		}
		BITSET(LOCKLESS, Shm->Cpu[cpu].OffLine, OS);
	    } else {
		if (!Arg[cpu].TID) {
			// Add this cpu.
			Package_Update(Shm, Proc);
			InitThermal(Shm, Proc, Core, cpu);
			PerCore_Update(Shm, Proc, Core, cpu);
			Technology_Update(Shm, Proc);

			ServerFollowService(&localService,
					    &Shm->Proc.Service,
					    tid);

			if (Quiet & 0x100)
			    printf("    CPU #%03u @ %.2f MHz\n", cpu,
				(double)( Core[cpu]->Clock.Hz
					* Proc->Boost[BOOST(MAX)])
					/ 1000000L );

			Arg[cpu].Ref  = Ref;
			Arg[cpu].Bind = cpu;
			pthread_create( &Arg[cpu].TID,
					NULL,
					Core_Cycle,
					&Arg[cpu]);
			// Notify
			if (!BITVAL(Shm->Proc.Sync, 63))
				BITSET(LOCKLESS, Shm->Proc.Sync, 63);
		}
		BITCLR(LOCKLESS, Shm->Cpu[cpu].OffLine, OS);

		struct FLIP_FLOP *Flop =
			&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		// Index cpu with the highest frequency.
		if (Flop->Relative.Freq > maxRelFreq) {
			maxRelFreq = Flop->Relative.Freq;
			Shm->Proc.Top = cpu;
		}
		// Sum counters values from the alternated memory structure.
		Shm->Proc.Avg.Turbo += Flop->State.Turbo;
		Shm->Proc.Avg.C0    += Flop->State.C0;
		Shm->Proc.Avg.C3    += Flop->State.C3;
		Shm->Proc.Avg.C6    += Flop->State.C6;
		Shm->Proc.Avg.C7    += Flop->State.C7;
		Shm->Proc.Avg.C1    += Flop->State.C1;
	    }
	}
	if (!BITVAL(Shutdown, 0)) {
		// Compute the counters averages.
		Shm->Proc.Avg.Turbo /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C0    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C3    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C6    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C7    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C1    /= Shm->Proc.CPU.OnLine;

	    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
		Shm->SysGate.tickStep = Proc->tickStep;
		if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
			// Update OS tasks and memory usage.
			if (SysGate_OnDemand(Ref, 1) == 0) {
				SysGate_Update(Ref);
			}
		}
	    }
		// Notify Client.
		BITSET(LOCKLESS, Shm->Proc.Sync, 0);
	}
	// Reset the Room mask
	BITMSK(BUS_LOCK, roomCore, Shm->Proc.CPU.Count);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	if (Arg[cpu].TID)
		pthread_join(Arg[cpu].TID, NULL);
    }
	free(Arg);
}

void Child_Manager(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	SERVICE_PROC localService = {.Proc = -1};
	unsigned int cpu = 0;

	pthread_t tid = pthread_self();

	if (ServerFollowService(&localService, &Shm->Proc.Service, tid) == 0)
		pthread_setname_np(tid, "corefreqd-cmgr");

	ARG *Arg = calloc(Shm->Proc.CPU.Count, sizeof(ARG));

    do {
	for (cpu=0; !BITVAL(Shutdown,0) && (cpu < Shm->Proc.CPU.Count); cpu++) {
	    if (BITVAL(Shm->Cpu[cpu].OffLine, OS) == 1) {
		if (Arg[cpu].TID) {
			// Remove this child thread.
			pthread_join(Arg[cpu].TID, NULL);
			Arg[cpu].TID = 0;
		}
	    } else {
		if (!Arg[cpu].TID) {
			// Add this child thread.
			Arg[cpu].Ref  = Ref;
			Arg[cpu].Bind = cpu;
			pthread_create( &Arg[cpu].TID,
					NULL,
					Child_Thread,
					&Arg[cpu]);
		}
	    }
	}
	ServerFollowService(&localService, &Shm->Proc.Service, tid);

	nanosleep(&Shm->Sleep.childWaiting, NULL);
    } while (!BITVAL(Shutdown, 0)) ;

	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
		if (Arg[cpu].TID)
			pthread_join(Arg[cpu].TID, NULL);
	free(Arg);
}

int Shm_Manager(FD *fd, PROC *Proc)
{
	int		rc = 0;
	unsigned int	cpu = 0;
	CORE		**Core;
	SHM_STRUCT	*Shm = NULL;
	const size_t	CoreSize  = ROUND_TO_PAGES(sizeof(CORE));

	Core = calloc(Proc->CPU.Count, sizeof(Core));
	for (cpu = 0; !rc && (cpu < Proc->CPU.Count); cpu++) {
		const off_t offset = (10 + cpu) * PAGE_SIZE;

		if ((Core[cpu] = mmap(NULL, CoreSize,
					PROT_READ|PROT_WRITE,
					MAP_SHARED,
					fd->Drv, offset)) == MAP_FAILED)
			rc = 4;
	}
	const size_t ShmCpuSize = sizeof(CPU_STRUCT) * Proc->CPU.Count,
		ShmSize = ROUND_TO_PAGES((sizeof(SHM_STRUCT) + ShmCpuSize));

	umask(!S_IRUSR|!S_IWUSR|!S_IRGRP|!S_IWGRP|!S_IROTH|!S_IWOTH);

	if (!rc)
	{	// Initialize shared memory.
	  if ((fd->Svr = shm_open(SHM_FILENAME, O_CREAT|O_TRUNC|O_RDWR,
					 S_IRUSR|S_IWUSR
					|S_IRGRP|S_IWGRP
					|S_IROTH|S_IWOTH)) != -1)
	  {
	    if (ftruncate(fd->Svr, ShmSize) != -1)
	    {
	      if ((Shm = mmap(NULL, ShmSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd->Svr, 0)) != MAP_FAILED)
	      {
		// Clear SHM
		memset(Shm, 0, ShmSize);

		// Store the daemon gate name.
		strncpy(Shm->ShmName, SHM_FILENAME, TASK_COMM_LEN - 1);

		// Initialize the busy wait times.
		Shm->Sleep.ringWaiting  = TIMESPEC(SIG_RING_MS);
		Shm->Sleep.childWaiting = TIMESPEC(CHILD_PS_MS);
		Shm->Sleep.sliceWaiting = TIMESPEC(CHILD_TH_MS);

		REF Ref = {
			.Signal		= {{0}},
			.CPID		= -1,
			.KID		= 0,
			.Started	= 0,
			.Slice.Func	= NULL,
			.Slice.arg	= 0,
			.fd		= fd,
			.Shm		= Shm,
			.Proc		= Proc,
			.Core		= Core,
			.SysGate	= NULL
		};

		Package_Update(Shm, Proc);

		// Initialize notification.
		BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

		SysGate_Toggle(&Ref, SysGateStartUp);

		// Welcomes with brand and per CPU base clock.
		if (Quiet & 0x001)
		 printf("CoreFreq Daemon."				\
			"  Copyright (C) 2015-2018 CYRIL INGENIERIE\n");
		if (Quiet & 0x010)
		 printf("\n"						\
			"  Processor [%s]\n"				\
			"  Architecture [%s] %u/%u CPU Online.\n",
			Shm->Proc.Brand,
			Shm->Proc.Architecture,
			Shm->Proc.CPU.OnLine,
			Shm->Proc.CPU.Count );
		if (Quiet & 0x100)
		 printf("  SleepInterval(%u), SysGate(%u), %ld tasks\n\n",
			Shm->Sleep.Interval,
			!BITVAL(Shm->SysGate.Operation, 0) ?
				0:Shm->Sleep.Interval * Shm->SysGate.tickReset,
			TASK_LIMIT);
		if (Quiet)
			fflush(stdout);

		Ref.CPID = fork();

		Emergency_Command(&Ref, 1);

		switch (Ref.CPID) {
		case 0:
			Child_Manager(&Ref);
			break;
		case -1:
			rc = 6;
			break;
		default:
			{
			Core_Manager(&Ref);

			if (Shm->AppCli)
				kill(Shm->AppCli, SIGCHLD);

			SysGate_OnDemand(&Ref, 0);

			if (!kill(Ref.CPID, SIGQUIT))
				waitpid(Ref.CPID, NULL, 0);
			}
			break;
		}
		Emergency_Command(&Ref, 0);

		munmap(Shm, ShmSize);
		shm_unlink(SHM_FILENAME);
	      }
	      else
		rc = 7;
	    }
	    else
		rc = 6;
	  }
	  else
		rc = 5;
	}
	for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		if (Core[cpu] != NULL)
			munmap(Core[cpu], CoreSize);
	free(Core);
	return(rc);
}

int help(char *appName)
{
	printf( "usage:\t%s [-option <arguments>]\n"			\
		"\t-q\t\tQuiet\n"					\
		"\t-i\t\tInfo\n"					\
		"\t-d\t\tDebug\n"					\
		"\t-gon\t\tEnable SysGate\n"				\
		"\t-goff\t\tDisable SysGate\n"				\
		"\t-h\t\tPrint out this message\n"			\
		"\nExit status:\n"					\
			"0\tif OK,\n"					\
			"1\tif problems,\n"				\
			">1\tif serious trouble.\n"			\
		"\nReport bugs to labs[at]cyring.fr\n", appName);
	return(1);
}

int main(int argc, char *argv[])
{
	FD   fd = {0, 0};
	PROC *Proc = NULL;	// Kernel module anchor point.
	int   rc = 0, i = 0;
	char *program = strdup(argv[0]), *appName = basename(program);

	for (i = 1; i < argc; i++) {
	    if (strlen(argv[i]) > 1) {
		if (argv[i][0] == '-') {
			char option = argv[i][1];
			switch (option) {
			case 'q':
				Quiet = 0x000;
				break;
			case 'i':
				Quiet = 0x011;
				break;
			case 'd':
				Quiet = 0x111;
				break;
			case 'g':
				if (argv[i][2]=='o'
				 && argv[i][3]=='f'
				 && argv[i][4]=='f')
					SysGateStartUp = 0;
			  else if (argv[i][2]=='o'
				&& argv[i][3]=='n')
					SysGateStartUp = 1;
			  else
					rc = help(appName);
				break;
			case 'h':
			default:
				rc = help(appName);
				break;
			}
		}
		else
			rc = help(appName);
	    }
	    else
		rc = help(appName);
	}
	if (!rc) {
	  if (geteuid() == 0) {
	    if ((fd.Drv = open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1) {
		const size_t packageSize = ROUND_TO_PAGES(sizeof(PROC));

		if ((Proc = mmap(NULL, packageSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.Drv, 0)) != MAP_FAILED) {

			rc = Shm_Manager(&fd, Proc);

			munmap(Proc, packageSize);
		}
		else
			rc = 3;
		close(fd.Drv);
	    }
	    else
		rc = 3;
	  } else {
	    printf("Insufficient permissions. Need root to start daemon.\n");
	    rc = 2;
	  }
	}
	free(program);
	return(rc);
}
