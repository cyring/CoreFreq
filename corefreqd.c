/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include "coretypes.h"
#include "corefreq.h"
#include "bitasm.h"
#include "intelmsr.h"
#include "amdmsr.h"
#include "corefreq-api.h"

#define PAGE_SIZE (sysconf(_SC_PAGESIZE))

unsigned int Shutdown = 0x0, Quiet = 0x001, Math = 0x0, SysGateStartUp = 1;
static unsigned long long roomSeed = 0x0;

typedef struct {
	SHM_STRUCT	*Shm;
	PROC		*Pkg;
	CORE		*Core;
	unsigned int	Bind;
	pthread_t	TID;
} ARG;


static void *Core_Cycle(void *arg)
{
	ARG *Arg = (ARG *) arg;
	SHM_STRUCT *Shm = Arg->Shm;
	PROC *Pkg = Arg->Pkg;
	CORE *Core = Arg->Core;
	unsigned int cpu = Arg->Bind;
	CPU_STRUCT *Cpu = &Shm->Cpu[cpu];

	pthread_t tid = pthread_self();
	cpu_set_t affinity;
	cpu_set_t cpuset;
	CPU_ZERO(&affinity);
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (!pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset)) {
		CPU_OR(&affinity, &affinity, &cpuset);
	}

	char comm[TASK_COMM_LEN];
	sprintf(comm, "corefreqd/%d", cpu);
	pthread_setname_np(tid, comm);

	if (Quiet & 0x100) {
		printf("    Thread [%lx] Init CPU %03u\n", tid, cpu);
		fflush(stdout);
	}
	BITSET(BUS_LOCK, roomSeed, cpu);
	BITSET(BUS_LOCK, Shm->Proc.Room, cpu);

	do {
	    while (!BITVAL(Core->Sync.V, 63)
		&& !Shutdown
		&& !Core->OffLine.OS) {
			nanosleep(&Shm->Proc.BaseSleep, NULL);
	    }
	    BITCLR(LOCKLESS, Core->Sync.V, 63);

	    if (!Shutdown && !Core->OffLine.OS)
	    {
		if (BITVAL(Shm->Proc.Room, cpu)) {
			Cpu->Toggle = !Cpu->Toggle;
			BITCLR(BUS_LOCK, Shm->Proc.Room, cpu);
		}
		struct FLIP_FLOP *Flip = &Cpu->FlipFlop[Cpu->Toggle];

		Flip->Counter.SMI	  = Core->Counter[1].SMI;
		Flip->Counter.NMI.LOCAL	  = Core->Counter[1].NMI.LOCAL;
		Flip->Counter.NMI.UNKNOWN = Core->Counter[1].NMI.UNKNOWN;
		Flip->Counter.NMI.PCISERR = Core->Counter[1].NMI.PCISERR;
		Flip->Counter.NMI.IOCHECK = Core->Counter[1].NMI.IOCHECK;

		Flip->Delta.INST	= Core->Delta.INST;
		Flip->Delta.C0.UCC	= Core->Delta.C0.UCC;
		Flip->Delta.C0.URC	= Core->Delta.C0.URC;
		Flip->Delta.C3		= Core->Delta.C3;
		Flip->Delta.C6		= Core->Delta.C6;
		Flip->Delta.C7		= Core->Delta.C7;
		Flip->Delta.TSC		= Core->Delta.TSC;
		Flip->Delta.C1		= Core->Delta.C1;
		Flip->Delta.SMI		= Core->Delta.SMI;

		// Compute IPS=Instructions per TSC
		Flip->State.IPS = (double) (Flip->Delta.INST)
				/ (double) (Flip->Delta.TSC);

		// Compute IPC=Instructions per non-halted reference cycle.
		// (Protect against a division by zero)
		Flip->State.IPC = (Flip->Delta.C0.URC != 0) ?
				  (double) (Flip->Delta.INST)
				/ (double) Flip->Delta.C0.URC
				: 0.0f;

		// Compute CPI=Non-halted reference cycles per instruction.
		// (Protect against a division by zero)
		Flip->State.CPI = (Flip->Delta.INST != 0) ?
				  (double) Flip->Delta.C0.URC
				/ (double) (Flip->Delta.INST)
				: 0.0f;

		// Compute Turbo State.
		Flip->State.Turbo=(double) (Flip->Delta.C0.UCC)
				/ (double) (Flip->Delta.TSC);
		// Compute C-States.
		Flip->State.C0	= (double) (Flip->Delta.C0.URC)
				/ (double) (Flip->Delta.TSC);
		Flip->State.C3	= (double) (Flip->Delta.C3)
				/ (double) (Flip->Delta.TSC);
		Flip->State.C6	= (double) (Flip->Delta.C6)
				/ (double) (Flip->Delta.TSC);
		Flip->State.C7	= (double) (Flip->Delta.C7)
				/ (double) (Flip->Delta.TSC);
		Flip->State.C1	= (double) (Flip->Delta.C1)
				/ (double) (Flip->Delta.TSC);

		// Compute SMI percent increase when delta > 0
		// (Protect against a division by zero)
		if (Flip->Delta.SMI > 0) {
			Flip->State.SMI = (Core->Counter[0].SMI != 0) ?
					  (double) (Flip->Delta.SMI)
					/ (double) (Core->Counter[0].SMI)
					: 0.0f;
		}
		// Relative Ratio formula.
		Flip->Relative.Ratio	= (double) (Flip->Delta.C0.UCC
						  * Shm->Proc.Boost[1])
					/ (double) (Flip->Delta.TSC);

		if (!Math && Core->Query.Turbo) {
			// Relative Frequency equals UCC per second.
			Flip->Relative.Freq = (double) (Flip->Delta.C0.UCC)
					/ (Shm->Proc.SleepInterval * 1000);
		} else {
		// Relative Frequency = Relative Ratio x Bus Clock Frequency
		  Flip->Relative.Freq =
		  (double) REL_FREQ(Shm->Proc.Boost[1],			\
				    Flip->Relative.Ratio,		\
				    Core->Clock, Shm->Proc.SleepInterval)
					/ (Shm->Proc.SleepInterval * 1000);
		}

		// Thermal formulas
		Flip->Thermal.Trip   = Core->PowerThermal.Trip;
		Flip->Thermal.Sensor = Core->PowerThermal.Sensor;

		switch (Pkg->thermalFormula) {
		case THERMAL_FORMULA_INTEL:
			Flip->Thermal.Temp = Cpu->PowerThermal.Target
					   - Flip->Thermal.Sensor;
			break;
		case THERMAL_FORMULA_AMD:
			break;
		case THERMAL_FORMULA_AMD_0F:
			Flip->Thermal.Temp = Flip->Thermal.Sensor
					  - (Cpu->PowerThermal.Target * 2) - 49;
			break;
		}
		if (Flip->Thermal.Temp < Cpu->PowerThermal.Limit[0])
			Cpu->PowerThermal.Limit[0] = Flip->Thermal.Temp;
		if (Flip->Thermal.Temp > Cpu->PowerThermal.Limit[1])
			Cpu->PowerThermal.Limit[1] = Flip->Thermal.Temp;

		// Voltage formulas
		Flip->Voltage.VID    = Core->Counter[1].VID;
		switch (Pkg->voltageFormula) {
		// Intel Core 2 Extreme Datasheet §3.3-Table 2
		case VOLTAGE_FORMULA_INTEL_MEROM:
			Flip->Voltage.Vcore = 0.8875
					+ (double) (Flip->Voltage.VID) * 0.0125;
			break;
		// Intel 2nd Gen Datasheet Vol-1 §7.4 Table 7-1
		case VOLTAGE_FORMULA_INTEL_SNB:
			if (Core->T.Base.BSP) {
			    Flip->Voltage.Vcore = (double) (Flip->Voltage.VID)
						/ 8192.0;
			}
			break;
		case VOLTAGE_FORMULA_AMD:
			break;
		// AMD BKDG Family 0Fh §10.6 Table 70
		case VOLTAGE_FORMULA_AMD_0F: {
			short	Vselect = (Flip->Voltage.VID & 0b110000) >> 4,
				Vnibble = Flip->Voltage.VID & 0b1111;

			switch (Vselect) {
			case 0b00:
			    Flip->Voltage.Vcore = 1.550
						- (double) (Vnibble) * 0.025;
			    break;
			case 0b01:
			    Flip->Voltage.Vcore = 1.150
						- (double) (Vnibble) * 0.025;
			    break;
			case 0b10:
			    Flip->Voltage.Vcore = 0.7625
						- (double) (Vnibble) * 0.0125;
			    break;
			case 0b11:
			    Flip->Voltage.Vcore = 0.5625
						- (double) (Vnibble) * 0.0125;
			    break;
			}
		    }
		    break;
		}

		// Package C-state Residency Counters
		if (Core->T.Base.BSP) {
			Shm->Proc.Toggle = !Shm->Proc.Toggle;

			struct PKG_FLIP_FLOP *Flip =
					&Shm->Proc.FlipFlop[Shm->Proc.Toggle];

			Flip->Delta.PTSC = Pkg->Delta.PTSC;
			Flip->Delta.PC02 = Pkg->Delta.PC02;
			Flip->Delta.PC03 = Pkg->Delta.PC03;
			Flip->Delta.PC06 = Pkg->Delta.PC06;
			Flip->Delta.PC07 = Pkg->Delta.PC07;
			Flip->Delta.PC08 = Pkg->Delta.PC08;
			Flip->Delta.PC09 = Pkg->Delta.PC09;
			Flip->Delta.PC10 = Pkg->Delta.PC10;

			Shm->Proc.State.PC02 = (Flip->Delta.PC02 != 0) ?
						  (double) Flip->Delta.PC02
						/ (double) Flip->Delta.PTSC
						: 0.0f;
			Shm->Proc.State.PC03 = (Flip->Delta.PC03 != 0) ?
						  (double) Flip->Delta.PC03
						/ (double) Flip->Delta.PTSC
						: 0.0f;
			Shm->Proc.State.PC06 = (Flip->Delta.PC06 != 0) ?
						  (double) Flip->Delta.PC06
						/ (double) Flip->Delta.PTSC
						: 0.0f;
			Shm->Proc.State.PC07 = (Flip->Delta.PC07 != 0) ?
						  (double) Flip->Delta.PC07
						/ (double) Flip->Delta.PTSC
						: 0.0f;
			Shm->Proc.State.PC08 = (Flip->Delta.PC08 != 0) ?
						  (double) Flip->Delta.PC08
						/ (double) Flip->Delta.PTSC
						: 0.0f;
			Shm->Proc.State.PC09 = (Flip->Delta.PC09 != 0) ?
						  (double) Flip->Delta.PC09
						/ (double) Flip->Delta.PTSC
						: 0.0f;
			Shm->Proc.State.PC10 = (Flip->Delta.PC10 != 0) ?
						  (double) Flip->Delta.PC10
						/ (double) Flip->Delta.PTSC
						: 0.0f;
		}
	    }
	} while (!Shutdown && !Core->OffLine.OS) ;

	BITCLR(BUS_LOCK, Shm->Proc.Room, cpu);
	BITCLR(BUS_LOCK, roomSeed, cpu);

	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CPU %03u\n", tid,
			Core->OffLine.OS ? "Offline" : "Shutdown", cpu);
		fflush(stdout);
	}
	return(NULL);
}

void Architecture(SHM_STRUCT *Shm, PROC *Proc)
{
	// Copy all BSP features.
	memcpy(&Shm->Proc.Features, &Proc->Features, sizeof(FEATURES));
	// Copy the numbers of total & online CPU.
	Shm->Proc.CPU.Count  = Proc->CPU.Count;
	Shm->Proc.CPU.OnLine = Proc->CPU.OnLine;
	// Copy the Architecture name.
	strncpy(Shm->Proc.Architecture, Proc->Architecture, 32);
	// Copy the base clock ratios.
	memcpy(Shm->Proc.Boost, Proc->Boost, (1+1+8) * sizeof(unsigned int));
	// Copy the processor's brand string.
	strncpy(Shm->Proc.Brand, Proc->Features.Info.Brand, 48);
}

void InvariantTSC(SHM_STRUCT *Shm, PROC *Proc)
{
	Bit32	fTSC = Proc->Features.Std.DX.TSC,
		aTSC = Proc->Features.AdvPower.DX.Inv_TSC;

	Shm->Proc.InvariantTSC = fTSC << aTSC;
}

void PerformanceMonitoring(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.PM_version = Proc->Features.PerfMon.AX.Version;
}

void HyperThreading(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.HyperThreading = Proc->Features.HTT_Enable;
}

void PowerNow(SHM_STRUCT *Shm, PROC *Proc)
{
	if (!strncmp(Proc->Features.Info.VendorID, VENDOR_AMD, 12)) {
		if (Proc->Features.AdvPower.DX.FID)
			BITSET(LOCKLESS, Shm->Proc.PowerNow, 0);
		else
			BITCLR(LOCKLESS, Shm->Proc.PowerNow, 0);

		if (Proc->Features.AdvPower.DX.VID)
			BITSET(LOCKLESS, Shm->Proc.PowerNow, 1);
		else
			BITCLR(LOCKLESS, Shm->Proc.PowerNow, 1);
	}
	else
		Shm->Proc.PowerNow = 0;
}

typedef struct {
	unsigned int	Q,
			R;
} RAM_Ratio;

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

	Shm->Uncore.Bus.Speed = (Shm->Proc.Boost[1]
				* Shm->Cpu[cpu].Clock.Hz
				* Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.FactoryFreq;
	Shm->Uncore.Bus.Speed /= 1000000L;

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
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank1Bank) {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank0Bank) {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 8;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.OddRank1) {
	    case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1;
		break;
	    case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 2;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.EvenRank0) {
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

	Shm->Uncore.Bus.Speed = (Shm->Proc.Boost[1]
				* Shm->Cpu[cpu].Clock.Hz
				* Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.FactoryFreq;
	Shm->Uncore.Bus.Speed /= 1000000L;

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
	Shm->Uncore.CtrlSpeed	= (Shm->Cpu[cpu].Clock.Hz
				* Proc->Uncore.Bus.DimmClock.QCLK_RATIO)
				/ 1000000L;

	Shm->Uncore.Bus.Rate = Proc->Uncore.Bus.QuickPath.QPIFREQSEL == 00 ?
		4800 : Proc->Uncore.Bus.QuickPath.QPIFREQSEL == 10 ?
			6400 : Proc->Uncore.Bus.QuickPath.QPIFREQSEL == 01 ?
				5866 : 8000;

	Shm->Uncore.Bus.Speed = (Proc->Boost[1]
				* Shm->Cpu[cpu].Clock.Hz
				* Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.FactoryFreq;
	Shm->Uncore.Bus.Speed /= 1000000L;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void DMI_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	Shm->Uncore.CtrlSpeed	= (Shm->Cpu[cpu].Clock.Hz
				* Proc->Uncore.Bus.DimmClock.QCLK_RATIO)
				/ 1000000L;

	Shm->Uncore.Bus.Rate = 2500;	// ToDo: hardwired to Lynnfield

	Shm->Uncore.Bus.Speed = (Proc->Boost[1]
				* Shm->Cpu[cpu].Clock.Hz
				* Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.FactoryFreq;
	Shm->Uncore.Bus.Speed /= 1000000L;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void C200_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].C200.DBP.tCL;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].C200._.tWR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].C200.RFTP.tRFC;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].C200.DBP.tRP;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].C200.RAP.tRRD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].C200.DBP.tRCD;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].C200._.tRAS;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = ?
*/
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
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (cha == 0) ?
					Proc->Uncore.MC[mc].C200.MAD0.ECC
				:	Proc->Uncore.MC[mc].C200.MAD1.ECC;
      }
    }
}

void C200_CLK(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
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
	case 0b000:
		// Fallthrough
	case 0b001:
		Shm->Uncore.CtrlSpeed = 2667;
		break;
	}
	Shm->Uncore.Bus.Rate = 5000;
	Shm->Uncore.Bus.Speed = (Shm->Proc.Boost[1]
				* Shm->Cpu[cpu].Clock.Hz
				* Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.FactoryFreq;
	Shm->Uncore.Bus.Speed /= 1000000L;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void C220_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {

      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].C220.Rank.tCL;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  =
			Proc->Uncore.MC[mc].Channel[cha].C220.Rank.tCWL;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].C220._.tWR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].C220.Refresh.tRFC;
/* ToDo
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].C220._.tRP;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].C220._.tRRD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].C220._.tRCD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].C220._.tRAS;
*/
	switch(Proc->Uncore.MC[mc].Channel[cha].C220.Timing.CMD_Stretch) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
		break;
	case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 3;
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

void Uncore(SHM_STRUCT *Shm, PROC *Proc, unsigned int cpu)
{
	switch (Proc->Uncore.ChipID) {
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
		QPI_CLK(Shm, Proc, cpu);
		NHM_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR:		// Lynnfield
		DMI_CLK(Shm, Proc, cpu);
		NHM_IMC(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0:	// Sandy Bridge
	case PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0:	// Ivy Bridge
		C200_CLK(Shm, Proc, cpu);
		C200_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0:	// Haswell
		C200_CLK(Shm, Proc, cpu);
		C220_MCH(Shm, Proc);
		break;
	case PCI_DEVICE_ID_AMD_K8_NB_MEMCTL:
		AMD_0F_HTT(Shm, Proc);
		AMD_0F_MCH(Shm, Proc);
		break;
	}
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
	Shm->Cpu[cpu].Topology.MP.x2APIC = ((Proc->Features.Std.CX.x2APIC
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

		if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_INTEL, 12))
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
		  if(!strncmp(Shm->Proc.Features.Info.VendorID, VENDOR_AMD, 12))
		  {

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
}

void SpeedStep(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	if ((Core[cpu]->T.ApicID >= 0) && (Core[cpu]->T.CoreID >= 0))
		BITSET(LOCKLESS, Shm->Proc.SpeedStep_Mask, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.SpeedStep_Mask, cpu);

	if (Core[cpu]->Query.EIST)
		BITSET(LOCKLESS, Shm->Proc.SpeedStep, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.SpeedStep, cpu);
}

void TurboBoost(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	if ((Core[cpu]->T.ApicID >= 0) && (Core[cpu]->T.CoreID >= 0))
		BITSET(LOCKLESS, Shm->Proc.TurboBoost_Mask, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.TurboBoost_Mask, cpu);

	if (Core[cpu]->Query.Turbo)
		BITSET(LOCKLESS, Shm->Proc.TurboBoost, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.TurboBoost, cpu);
}

void CStates(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{
	if ((Core[cpu]->T.ApicID >= 0) && (Core[cpu]->T.CoreID >= 0)) {
		BITSET(LOCKLESS, Shm->Proc.C1E_Mask, cpu);

	    if ((Core[cpu]->T.ThreadID == 0) || (Core[cpu]->T.ThreadID == -1)) {
		BITSET(LOCKLESS, Shm->Proc.C3A_Mask, cpu);
		BITSET(LOCKLESS, Shm->Proc.C1A_Mask, cpu);
		BITSET(LOCKLESS, Shm->Proc.C3U_Mask, cpu);
		BITSET(LOCKLESS, Shm->Proc.C1U_Mask, cpu);
	    } else {
		BITCLR(LOCKLESS, Shm->Proc.C3A_Mask, cpu);
		BITCLR(LOCKLESS, Shm->Proc.C1A_Mask, cpu);
		BITCLR(LOCKLESS, Shm->Proc.C3U_Mask, cpu);
		BITCLR(LOCKLESS, Shm->Proc.C1U_Mask, cpu);
	    }
	}
	else
		BITCLR(LOCKLESS, Shm->Proc.C1E_Mask, cpu);

	if (Core[cpu]->Query.C1E)
		BITSET(LOCKLESS, Shm->Proc.C1E, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C1E, cpu);
	if (Core[cpu]->Query.C3A)
		BITSET(LOCKLESS, Shm->Proc.C3A, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C3A, cpu);
	if (Core[cpu]->Query.C1A)
		BITSET(LOCKLESS, Shm->Proc.C1A, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C1A, cpu);
	if (Core[cpu]->Query.C3U)
		BITSET(LOCKLESS, Shm->Proc.C3U, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C3U, cpu);
	if (Core[cpu]->Query.C1U)
		BITSET(LOCKLESS, Shm->Proc.C1U, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.C1U, cpu);

	Shm->Cpu[cpu].Query.CfgLock = Core[cpu]->Query.CfgLock;
	Shm->Cpu[cpu].Query.CStateLimit = Core[cpu]->Query.CStateLimit;

	Shm->Cpu[cpu].Query.IORedir = Core[cpu]->Query.IORedir;
	Shm->Cpu[cpu].Query.CStateInclude = Core[cpu]->Query.CStateInclude;
}

void PowerThermal(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	BITSET(LOCKLESS, Shm->Proc.ODCM_Mask, cpu);
	if (Core[cpu]->PowerThermal.ClockModulation.ODCM_Enable)
		BITSET(LOCKLESS, Shm->Proc.ODCM, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.ODCM, cpu);

	Shm->Cpu[cpu].PowerThermal.DutyCycle =
		Core[cpu]->PowerThermal.ClockModulation.ExtensionBit == 1 ?
					6.25f : 12.5f;
	Shm->Cpu[cpu].PowerThermal.DutyCycle
		*= Core[cpu]->PowerThermal.ClockModulation.ODCM_DutyCycle;

	BITSET(LOCKLESS, Shm->Proc.PowerMgmt_Mask, cpu);
	if (Core[cpu]->PowerThermal.PwrManagement.Perf_BIAS_Enable)
		BITSET(LOCKLESS, Shm->Proc.PowerMgmt, cpu);
	else
		BITCLR(LOCKLESS, Shm->Proc.PowerMgmt, cpu);

	Shm->Cpu[cpu].PowerThermal.PowerPolicy =
		Core[cpu]->PowerThermal.PerfEnergyBias.PowerPolicy;

	Shm->Cpu[cpu].PowerThermal.TM1 =
		Proc->Features.Std.DX.TM1;			//0001

	Shm->Cpu[cpu].PowerThermal.TM1 |=
		(Core[cpu]->PowerThermal.TCC_Enable << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.TM1 ^=
		(Core[cpu]->PowerThermal.TM_Select << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.TM2 =
		Proc->Features.Std.CX.TM2;			//0001

	Shm->Cpu[cpu].PowerThermal.TM2 |=
		(Core[cpu]->PowerThermal.TM2_Enable << 1);	//0010

	Shm->Cpu[cpu].PowerThermal.Target = Core[cpu]->PowerThermal.Target;

	switch (Proc->thermalFormula) {
	case THERMAL_FORMULA_INTEL:
	case THERMAL_FORMULA_AMD:
	  Shm->Cpu[cpu].PowerThermal.Limit[0] = Core[cpu]->PowerThermal.Target;
	  Shm->Cpu[cpu].PowerThermal.Limit[1] = 0;
	  break;
	case THERMAL_FORMULA_AMD_0F:
	  Shm->Cpu[cpu].PowerThermal.Limit[0] = Core[cpu]->PowerThermal.Sensor
				    - (Core[cpu]->PowerThermal.Target * 2) - 49;
	  break;
	}
}

void SysGate_IdleDriver(SHM_STRUCT *Shm, SYSGATE *SysGate)
{
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
}

void SysGate_Kernel(SHM_STRUCT *Shm, SYSGATE *SysGate)
{
	memcpy(Shm->SysGate.sysname, SysGate->sysname, MAX_UTS_LEN);
	memcpy(Shm->SysGate.release, SysGate->release, MAX_UTS_LEN);
	memcpy(Shm->SysGate.version, SysGate->version, MAX_UTS_LEN);
	memcpy(Shm->SysGate.machine, SysGate->machine, MAX_UTS_LEN);
}

void SysGate_Update(SHM_STRUCT *Shm, SYSGATE *SysGate)
{
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
	Shm->Cpu[cpu].OffLine.HW = Core[cpu]->OffLine.HW;

	Shm->Cpu[cpu].Query.Microcode = Core[cpu]->Query.Microcode;

	CPUID_Dump(Shm, Core, cpu);

	BaseClock(Shm, Core, cpu);

	Topology(Shm, Proc, Core, cpu);

	SpeedStep(Shm, Proc, Core, cpu);

	TurboBoost(Shm, Core, cpu);

	CStates(Shm, Core, cpu);

	PowerThermal(Shm, Proc, Core, cpu);

	if (cpu == 0)
		Uncore(Shm, Proc, cpu);
}

typedef	struct
{
	int	Drv,
		Svr;
} FD;

typedef struct {
	sigset_t	Signal;
	pthread_t	TID;
	int		Started;
	FD		*fd;
	SHM_STRUCT	*Shm;
	SYSGATE		**SysGate;
} SIG;

int SysGate_OnDemand(FD *fd, SYSGATE **SysGate, int operation)
{
	int rc = -1;
	const size_t pageSize = ROUND_TO_PAGES(sizeof(SYSGATE));
	if (operation == 0) {
		if (*SysGate != NULL) {
			if ((rc = munmap(*SysGate, pageSize)) == 0)
				*SysGate = NULL;
		}
		else
			rc = -1;
	} else {
		if (*SysGate == NULL) {
			const off_t offset = 1 * PAGE_SIZE;
			SYSGATE *MapGate = mmap(NULL, pageSize,
						PROT_READ|PROT_WRITE,
						MAP_SHARED,
						fd->Drv, offset);
			if (MapGate != MAP_FAILED) {
				*SysGate = MapGate;
				rc = 0;
			}
		}
		else
			rc = 0;
	}
	return(rc);
}

void SysGate_Toggle(SIG *Sig, unsigned int state)
{
    if (state == 0) {
	if (BITWISEAND(LOCKLESS, Sig->Shm->SysGate.Operation, 0x1)) {
		// Stop SysGate
		BITCLR(LOCKLESS, Sig->Shm->SysGate.Operation,0);
		// Notify
		if (!BITVAL(Sig->Shm->Proc.Sync, 63))
			BITSET(BUS_LOCK, Sig->Shm->Proc.Sync, 63);

		if (Quiet & 0x100) {
			printf("  SysGate(%llu)\n",Sig->Shm->SysGate.Operation);
			fflush(stdout);
		}
	}
    } else {
	if (!BITWISEAND(LOCKLESS, Sig->Shm->SysGate.Operation, 0x1)) {
	    if (SysGate_OnDemand(Sig->fd, Sig->SysGate, 1) == 0) {
		if (ioctl(Sig->fd->Drv, COREFREQ_IOCTL_SYSONCE) != -1) {
			// Aggregate the OS idle driver data.
			SysGate_IdleDriver(Sig->Shm, *(Sig->SysGate));
			// Copy system information.
			SysGate_Kernel(Sig->Shm, *(Sig->SysGate));
			// Start SysGate
			BITSET(LOCKLESS, Sig->Shm->SysGate.Operation,0);
			// Notify
			if (!BITVAL(Sig->Shm->Proc.Sync, 63))
				BITSET(BUS_LOCK, Sig->Shm->Proc.Sync, 63);

		    if (Quiet & 0x100) {
			printf("  SysGate(%llu)\n",Sig->Shm->SysGate.Operation);
			fflush(stdout);
		    }
		}
	    }
	}
    }
}

static void *Emergency_Handler(void *arg)
{
	int caught = 0, leave = 0;
	SIG *Sig = (SIG *) arg;
	pthread_t tid = pthread_self();

	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);

	pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	pthread_setname_np(tid, "corefreqd-kill");

	while (!leave && !sigwait(&Sig->Signal, &caught))
		switch (caught) {
		case SIGUSR2:
			SysGate_Toggle(Sig, 0);
			break;
		case SIGUSR1:
			SysGate_Toggle(Sig, 1);
			break;
		case SIGTERM:
			leave = 0x1;
			break;
		case SIGINT:
			// Fallthrough
		case SIGQUIT:
			Shutdown = 0x1;
			break;
		}
	return(NULL);
}

void Emergency_Command(SIG *Sig, unsigned int cmd)
{
	switch (cmd) {
	case 0:
		if (Sig->Started) {
			pthread_kill(Sig->TID, SIGTERM);
			pthread_join(Sig->TID, NULL);
		}
		break;
	case 1: {
		sigemptyset(&Sig->Signal);
		sigaddset(&Sig->Signal, SIGUSR1);	// Start SysGate
		sigaddset(&Sig->Signal, SIGUSR2);	// Stop  SysGate
		sigaddset(&Sig->Signal, SIGINT);	// [CTRL] + [C]
		sigaddset(&Sig->Signal, SIGQUIT);	// Shutdown
		sigaddset(&Sig->Signal, SIGTERM);	// Thread kill

		if (!pthread_sigmask(SIG_BLOCK, &Sig->Signal, NULL)
		 && !pthread_create(&Sig->TID, NULL, Emergency_Handler, Sig))
			Sig->Started = 1;
		}
		break;
	}
}

void Core_Manager(FD *fd,
		SHM_STRUCT *Shm,
		PROC *Proc,
		CORE **Core,
		SYSGATE **SysGate)
{
    unsigned int cpu = 0;

    pthread_t tid = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    pthread_setname_np(tid, "corefreqd-mgr");

    ARG *Arg = calloc(Shm->Proc.CPU.Count, sizeof(ARG));

    while (!Shutdown) {
	for (cpu = 0; !Shutdown && (cpu < Shm->Proc.CPU.Count); cpu++) {
	    if (Core[cpu]->OffLine.OS == 1) {
		if (Arg[cpu].TID) {
			// Remove this cpu.
			pthread_join(Arg[cpu].TID, NULL);
			Arg[cpu].TID = 0;
			// Raise this bit up to notify a platform change.
			if (!BITVAL(Shm->Proc.Sync, 63))
				BITSET(LOCKLESS, Shm->Proc.Sync, 63);
		}
		Shm->Cpu[cpu].OffLine.OS = 1;
	    } else {
		if (!Arg[cpu].TID) {
			// Add this cpu.
			PerCore_Update(Shm, Proc, Core, cpu);
			HyperThreading(Shm, Proc);

			if (Quiet & 0x100)
			    printf("    CPU #%03u @ %.2f MHz\n", cpu,
				(double)( Core[cpu]->Clock.Hz
					* Proc->Boost[1])
					/ 1000000L );

			Arg[cpu].Shm  = Shm;
			Arg[cpu].Pkg  = Proc;
			Arg[cpu].Core = Core[cpu];
			Arg[cpu].Bind = cpu;
			pthread_create( &Arg[cpu].TID,
					NULL,
					Core_Cycle,
					&Arg[cpu]);
			// Notify
			if (!BITVAL(Shm->Proc.Sync, 63))
				BITSET(LOCKLESS, Shm->Proc.Sync, 63);
		}
		Shm->Cpu[cpu].OffLine.OS = 0;
	    }
	}
	// Loop while all the cpu room bits are not cleared.
	while (!Shutdown && BITWISEAND(BUS_LOCK, Shm->Proc.Room, roomSeed)) {
		nanosleep(&Shm->Proc.BaseSleep, NULL);
	}

	if (!Shutdown) {
		double maxRelFreq = 0.0;

		// Update the count of online CPU
		Shm->Proc.CPU.OnLine = Proc->CPU.OnLine;

		//  Average the counters.
		Shm->Proc.Avg.Turbo = 0;
		Shm->Proc.Avg.C0    = 0;
		Shm->Proc.Avg.C3    = 0;
		Shm->Proc.Avg.C6    = 0;
		Shm->Proc.Avg.C7    = 0;
		Shm->Proc.Avg.C1    = 0;

		for (cpu = 0; !Shutdown &&(cpu < Shm->Proc.CPU.Count); cpu++) {
		    if (!Shm->Cpu[cpu].OffLine.OS) {
			struct FLIP_FLOP *Flop =
				&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

			// Index cpu with the highest frequency.
			if (Flop->Relative.Freq > maxRelFreq) {
				maxRelFreq = Flop->Relative.Freq;
				Shm->Proc.Top = cpu;
			}
			// For each cpu, sum counters values from
			// the alternated memory structure.
			Shm->Proc.Avg.Turbo += Flop->State.Turbo;
			Shm->Proc.Avg.C0    += Flop->State.C0;
			Shm->Proc.Avg.C3    += Flop->State.C3;
			Shm->Proc.Avg.C6    += Flop->State.C6;
			Shm->Proc.Avg.C7    += Flop->State.C7;
			Shm->Proc.Avg.C1    += Flop->State.C1;
		    }
		}
		// Compute the counters averages.
		Shm->Proc.Avg.Turbo /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C0    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C3    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C6    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C7    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C1    /= Shm->Proc.CPU.OnLine;

		if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1)) {
			// Update OS tasks and memory usage.
			if (SysGate_OnDemand(fd, SysGate, 1) == 0) {
				if (ioctl(fd->Drv,COREFREQ_IOCTL_SYSUPDT) != -1)
					SysGate_Update(Shm, *SysGate);
			}
		}

		// Notify Client.
		BITSET(LOCKLESS, Shm->Proc.Sync, 0);
	}
	// Reset the Room mask
	BITMSK(BUS_LOCK, Shm->Proc.Room, Shm->Proc.CPU.Count);
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++)
	if (Arg[cpu].TID)
		pthread_join(Arg[cpu].TID, NULL);
    free(Arg);
}

int Shm_Manager(FD *fd, PROC *Proc)
{
	unsigned int	cpu = 0;
	CORE		**Core;
	SYSGATE		*SysGate = NULL;
	SHM_STRUCT	*Shm = NULL;
	int		rc = 0;
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
	    if (((fd->Svr = shm_open(SHM_FILENAME, O_CREAT|O_TRUNC|O_RDWR,
					 S_IRUSR|S_IWUSR
					|S_IRGRP|S_IWGRP
					|S_IROTH|S_IWOTH)) != -1)
	    && (ftruncate(fd->Svr, ShmSize) != -1)
	    && ((Shm = mmap(0, ShmSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd->Svr, 0)) != MAP_FAILED))
	    {
		// Clear SHM
		memset(Shm, 0, ShmSize);

		Shm->Registration.Experimental=Proc->Registration.Experimental;
		Shm->Registration.hotplug = Proc->Registration.hotplug;
		Shm->Registration.pci = Proc->Registration.pci;
		Shm->Registration.nmi = Proc->Registration.nmi;

		SIG Sig = {
			.Signal	= {{0}},
			.TID	= 0,
			.Started= 0,
			.fd	= fd,
			.Shm	= Shm,
			.SysGate= &SysGate
		};
		Emergency_Command(&Sig, 1);

		// Copy the timer interval delay.
		Shm->Proc.SleepInterval = Proc->SleepInterval;
		// Compute the polling rate based on the timer interval.
		Shm->Proc.BaseSleep =
			TIMESPEC((Shm->Proc.SleepInterval * 1000000L) / 5);

		Architecture(Shm, Proc);

		// Technologies aggregation.
		PerformanceMonitoring(Shm, Proc);

		InvariantTSC(Shm, Proc);

		HyperThreading(Shm, Proc);

		PowerNow(Shm, Proc);

		// Store the application name.
		strncpy(Shm->AppName, SHM_FILENAME, TASK_COMM_LEN - 1);

		// Initialize notification.
		BITCLR(LOCKLESS, Shm->Proc.Sync, 0);

		SysGate_Toggle(&Sig, SysGateStartUp);

		// Welcomes with brand and per CPU base clock.
		if (Quiet & 0x001)
		 printf("CoreFreq Daemon."				\
			"  Copyright (C) 2015-2017 CYRIL INGENIERIE\n");
		if (Quiet & 0x010)
		 printf("\n"						\
			"  Processor [%s]\n"				\
			"  Architecture [%s] %u/%u CPU Online.\n",
			Shm->Proc.Brand,
			Shm->Proc.Architecture,
			Shm->Proc.CPU.OnLine,
			Shm->Proc.CPU.Count );
		if (Quiet & 0x100)
		  printf("  SleepInterval(%u), SysGate(%llu)\n\n",
			Shm->Proc.SleepInterval, Shm->SysGate.Operation);
		if (Quiet)
			fflush(stdout);

		Core_Manager(fd, Shm, Proc, Core, &SysGate);

		Emergency_Command(&Sig, 0);

		munmap(Shm, ShmSize);
		shm_unlink(SHM_FILENAME);
	    }
	    else
		rc = 5;
	}
	SysGate_OnDemand(fd, &SysGate, 0);

	for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		if (Core[cpu] != NULL)
			munmap(Core[cpu], CoreSize);
	free(Core);
	return(rc);
}

int help(char *appName)
{
	printf( "usage:\t%s [-option]\n"				\
		"\t-q\tQuiet\n"						\
		"\t-i\tInfo\n"						\
		"\t-d\tDebug\n"						\
		"\t-m\tMath\n"						\
		"\t-gon\tEnable SysGate\n"				\
		"\t-goff\tDisable SysGate\n"				\
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
			case 'm':
				Math = 0x1;
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
