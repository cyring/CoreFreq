/*
 * CoreFreq
 * Copyright (C) 2015-2025 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#ifndef __USE_GNU
#include <libgen.h>
#endif

#include "bitasm.h"
#include "arm_reg.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreqm.h"
#include "corefreq-api.h"

#define AT( _loc_ )		[ _loc_ ]

#define PAGE_SIZE							\
(									\
	sysconf(_SC_PAGESIZE) > 0 ? sysconf(_SC_PAGESIZE) : 4096	\
)

/* AArch64 LDAXP/STLXP alignment, 128-Byte Blocks of Memory */
static BitCC roomSeed	__attribute__ ((aligned (16))) = InitCC(0x0);
static BitCC roomCore	__attribute__ ((aligned (16))) = InitCC(0x0);
static BitCC roomClear	__attribute__ ((aligned (16))) = InitCC(0x0);
static BitCC roomReady	__attribute__ ((aligned (16))) = InitCC(0x0);
static Bit64 Shutdown	__attribute__ ((aligned (8))) = 0x0;
static Bit64 PendingSync __attribute__ ((aligned (8))) = 0x0;
unsigned int Quiet = 0x001, SysGateStartUp = 1;

UBENCH_DECLARE()

typedef struct
{
	int	drv,
		ro,
		rw;
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
	RO(SHM_STRUCT)		*RO(Shm);
	RW(SHM_STRUCT)		*RW(Shm);
	RO(PROC)		*RO(Proc);
	RW(PROC)		*RW(Proc);
	RO(CORE)		**RO(Core);
	RW(CORE)		**RW(Core);
	RO(SYSGATE)		*RO(SysGate);
} REF;

void Core_ResetSensorLimits(CPU_STRUCT *Cpu)
{
	RESET_SENSOR_LIMIT(THERMAL,	LOWEST, Cpu->PowerThermal.Limit);
	RESET_SENSOR_LIMIT(VOLTAGE,	LOWEST, Cpu->Sensors.Voltage.Limit);
	RESET_SENSOR_LIMIT(ENERGY,	LOWEST, Cpu->Sensors.Energy.Limit);
	RESET_SENSOR_LIMIT(POWER,	LOWEST, Cpu->Sensors.Power.Limit);
	RESET_SENSOR_LIMIT(REL_FREQ,	LOWEST, Cpu->Relative.Freq);
	RESET_SENSOR_LIMIT(ABS_FREQ,	LOWEST, Cpu->Absolute.Freq);

	RESET_SENSOR_LIMIT(THERMAL,	HIGHEST, Cpu->PowerThermal.Limit);
	RESET_SENSOR_LIMIT(VOLTAGE,	HIGHEST, Cpu->Sensors.Voltage.Limit);
	RESET_SENSOR_LIMIT(ENERGY,	HIGHEST, Cpu->Sensors.Energy.Limit);
	RESET_SENSOR_LIMIT(POWER,	HIGHEST, Cpu->Sensors.Power.Limit);
	RESET_SENSOR_LIMIT(REL_FREQ,	HIGHEST, Cpu->Relative.Freq);
	RESET_SENSOR_LIMIT(ABS_FREQ,	HIGHEST, Cpu->Absolute.Freq);
}

void Core_ComputeThermalLimits(CPU_STRUCT *Cpu, struct FLIP_FLOP *CFlip)
{	/* Per Core, computes the Min temperature.			*/
	TEST_AND_SET_SENSOR( THERMAL, LOWEST,	CFlip->Thermal.Temp,
						Cpu->PowerThermal.Limit );
	/* Per Core, computes the Max temperature.			*/
	TEST_AND_SET_SENSOR( THERMAL, HIGHEST,	CFlip->Thermal.Temp,
						Cpu->PowerThermal.Limit );
}

static void ComputeThermal_None( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	UNUSED(RO(Shm));
	UNUSED(cpu);
	CFlip->Thermal.Temp = 0;
}

#define ComputeThermal_None_PerSMT	ComputeThermal_None
#define ComputeThermal_None_PerCore	ComputeThermal_None
#define ComputeThermal_None_PerPkg	ComputeThermal_None

static void (*ComputeThermal_None_Matrix[4])( struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_None_PerPkg
};

void Core_ComputeVoltageLimits(CPU_STRUCT *Cpu, struct FLIP_FLOP *CFlip)
{	/* Per Core, computes the Min CPU voltage.			*/
	TEST_AND_SET_SENSOR( VOLTAGE, LOWEST,	CFlip->Voltage.Vcore,
						Cpu->Sensors.Voltage.Limit );
	/* Per Core, computes the Max CPU voltage.			*/
	TEST_AND_SET_SENSOR( VOLTAGE, HIGHEST,	CFlip->Voltage.Vcore,
						Cpu->Sensors.Voltage.Limit );
}

static void ComputeVoltage_None( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	UNUSED(CFlip);
	UNUSED(RO(Shm));
	UNUSED(cpu);
}

#define ComputeVoltage_None_PerSMT	ComputeVoltage_None
#define ComputeVoltage_None_PerCore	ComputeVoltage_None
#define ComputeVoltage_None_PerPkg	ComputeVoltage_None

static void (*ComputeVoltage_None_Matrix[4])(	struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_None_PerPkg
};

static void ComputeVoltage_OPP( struct FLIP_FLOP *CFlip,
				RO(SHM_STRUCT) *RO(Shm),
				unsigned int cpu )
{
	COMPUTE_VOLTAGE(OPP,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_OPP_PerSMT	ComputeVoltage_OPP

static void ComputeVoltage_OPP_PerCore( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_OPP(CFlip, RO(Shm), cpu);
	}
}

static  void ComputeVoltage_OPP_PerPkg( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_OPP(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_OPP_Matrix[4])(	struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_OPP_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_OPP_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_OPP_PerPkg
};

void Core_ComputePowerLimits(CPU_STRUCT *Cpu, struct FLIP_FLOP *CFlip)
{	/* Per Core, computes the Min CPU Energy consumed.		*/
	TEST_AND_SET_SENSOR( ENERGY, LOWEST,	CFlip->State.Energy,
						Cpu->Sensors.Energy.Limit );
	/* Per Core, computes the Max CPU Energy consumed.		*/
	TEST_AND_SET_SENSOR( ENERGY, HIGHEST,	CFlip->State.Energy,
						Cpu->Sensors.Energy.Limit );
	/* Per Core, computes the Min CPU Power consumed.		*/
	TEST_AND_SET_SENSOR( POWER, LOWEST,	CFlip->State.Power,
						Cpu->Sensors.Power.Limit);
	/* Per Core, computes the Max CPU Power consumed.		*/
	TEST_AND_SET_SENSOR( POWER, HIGHEST,	CFlip->State.Power,
						Cpu->Sensors.Power.Limit );
}

static void ComputePower_None( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	UNUSED(CFlip);
	UNUSED(RO(Shm));
	UNUSED(cpu);
}

#define ComputePower_None_PerSMT	ComputePower_None
#define ComputePower_None_PerCore	ComputePower_None
#define ComputePower_None_PerPkg	ComputePower_None

static void (*ComputePower_None_Matrix[4])(	struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputePower_None,
	[FORMULA_SCOPE_SMT ] = ComputePower_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputePower_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputePower_None_PerPkg
};

typedef struct {
	REF		*Ref;
	unsigned int	Bind;
	pthread_t	TID;
} ARG;

static void *Core_Cycle(void *arg)
{
	ARG		*Arg		= (ARG *) arg;
	unsigned int	cpu		= Arg->Bind;
	RO(SHM_STRUCT)	*RO(Shm)	= Arg->Ref->RO(Shm);
	RO(CORE)	*RO(Core)	= Arg->Ref->RO(Core, AT(cpu));
	RW(CORE)	*RW(Core)	= Arg->Ref->RW(Core, AT(cpu));
	CPU_STRUCT	*Cpu		= &RO(Shm)->Cpu[cpu];

	pthread_t tid = pthread_self();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) != 0) {
		goto EXIT;
	}
	char *comm = malloc(TASK_COMM_LEN+10+1);
	if (comm != NULL) {
		snprintf(comm, TASK_COMM_LEN+10+1, "corefreqd/%u", cpu);
		pthread_setname_np(tid, comm);
		free(comm);
	}

	void (**ComputeThermalFormula)( struct FLIP_FLOP*,
					RO(SHM_STRUCT)*,
					unsigned int );

	void (**ComputeVoltageFormula)( struct FLIP_FLOP*,
					RO(SHM_STRUCT)*,
					unsigned int );

	void (**ComputePowerFormula)(	struct FLIP_FLOP*,
					RO(SHM_STRUCT)*,
					unsigned int );

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.thermalFormula)) {
	case THERMAL_KIND_NONE:
	default:
		ComputeThermalFormula = ComputeThermal_None_Matrix;
		break;
	}

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.voltageFormula)) {
	case VOLTAGE_KIND_OPP:
		ComputeVoltageFormula = ComputeVoltage_OPP_Matrix;
		break;
	case VOLTAGE_KIND_NONE:
	default:
		ComputeVoltageFormula = ComputeVoltage_None_Matrix;
		break;
	}

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.powerFormula)) {
	case POWER_KIND_NONE:
	default:
		ComputePowerFormula = ComputePower_None_Matrix;
		break;
	}

	if (Quiet & 0x100) {
		printf("    Thread [%lx] Init CYCLE %03u\n", (long) tid, cpu);
		fflush(stdout);
	}
	BITSET_CC(BUS_LOCK, roomSeed, cpu);
	BITSET_CC(BUS_LOCK, roomCore, cpu);

  do {
    while (!BITCLR(LOCKLESS, RW(Core)->Sync.V, NTFY)
	&& !BITVAL(Shutdown, SYNC)
	&& !BITVAL(RO(Core)->OffLine, OS)) {
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
    }

    if (!BITVAL(Shutdown, SYNC) && !BITVAL(RO(Core)->OffLine, OS))
    {
	if (BITCLR_CC(BUS_LOCK, roomCore, cpu)) {
		Cpu->Toggle = !Cpu->Toggle;
	}
	struct FLIP_FLOP *CFlip = &Cpu->FlipFlop[Cpu->Toggle];

	/* Refresh this Base Clock.					*/
	CFlip->Clock.Q  = RO(Core)->Clock.Q;
	CFlip->Clock.R  = RO(Core)->Clock.R;
	CFlip->Clock.Hz = RO(Core)->Clock.Hz;

	/* Copy the Performance & C-States Counters.			*/
	CFlip->Delta.INST	= RO(Core)->Delta.INST;
	CFlip->Delta.C0.UCC	= RO(Core)->Delta.C0.UCC;
	CFlip->Delta.C0.URC	= RO(Core)->Delta.C0.URC;
	CFlip->Delta.C3 	= RO(Core)->Delta.C3;
	CFlip->Delta.C6 	= RO(Core)->Delta.C6;
	CFlip->Delta.C7 	= RO(Core)->Delta.C7;
	CFlip->Delta.TSC	= RO(Core)->Delta.TSC;
	CFlip->Delta.C1 	= RO(Core)->Delta.C1;

	/* Update all clock ratios.					*/
	memcpy(Cpu->Boost, RO(Core)->Boost, (BOOST(SIZE)) * sizeof(COF_ST));

	const double FSF = UNIT_KHz(1.0)
			 / ( (double)(RO(Shm)->Sleep.Interval * CFlip->Clock.Hz)
			 * COF2FLOAT(Cpu->Boost[BOOST(MAX)]) );

	CFlip->Absolute.Ratio.Perf = COF2FLOAT(RO(Core)->Ratio);

	/* Compute IPS=Instructions per Hz				*/
	CFlip->State.IPS = (double)CFlip->Delta.INST * FSF;

	/* Compute IPC=Instructions per non-halted reference cycle.
	   ( Protect against a division by zero )			*/
	if (CFlip->Delta.C0.URC != 0) {
		CFlip->State.IPC = (double)CFlip->Delta.INST
				 / (double)CFlip->Delta.C0.URC;
	} else {
		CFlip->State.IPC = 0.0f;
	}
	/* Compute CPI=Non-halted reference cycles per instruction.
	   ( Protect against a division by zero )			*/
	if (CFlip->Delta.INST != 0) {
		CFlip->State.CPI = (double)CFlip->Delta.C0.URC
				 / (double)CFlip->Delta.INST;
	} else {
		CFlip->State.CPI = 0.0f;
	}
	/* Compute the Turbo State.					*/
	CFlip->State.Turbo = (double)CFlip->Delta.C0.UCC * FSF;

	/* Compute the C-States.					*/
	CFlip->State.C0 = (double)CFlip->Delta.C0.URC * FSF;

	CFlip->State.C3 = (double)CFlip->Delta.C3 * FSF;

	CFlip->State.C6 = (double)CFlip->Delta.C6 * FSF;

	CFlip->State.C7 = (double)CFlip->Delta.C7 * FSF;

	CFlip->State.C1 = (double)CFlip->Delta.C1 * FSF;

	/* Relative Frequency = Relative Ratio x Bus Clock Frequency	*/
	CFlip->Relative.Ratio	= (double)CFlip->Delta.C0.URC
			/ (double)UNIT_KHz(RO(Shm)->Sleep.Interval * PRECISION);

	CFlip->Relative.Freq	= REL_FREQ_MHz( double,
						CFlip->Relative.Ratio,
						CFlip->Clock,
						RO(Shm)->Sleep.Interval );

	/* Per Core, compute the Relative Frequency limits.		*/
	TEST_AND_SET_SENSOR( REL_FREQ, LOWEST,	CFlip->Relative.Freq,
						Cpu->Relative.Freq );

	TEST_AND_SET_SENSOR( REL_FREQ, HIGHEST, CFlip->Relative.Freq,
						Cpu->Relative.Freq );
	/* Per Core, compute the Absolute Frequency limits.		*/
	CFlip->Absolute.Freq = ABS_FREQ_MHz(double, CFlip->Absolute.Ratio.Perf,
						(double)CFlip->Clock);

	TEST_AND_SET_SENSOR( ABS_FREQ, LOWEST,	CFlip->Absolute.Freq,
						Cpu->Absolute.Freq );

	TEST_AND_SET_SENSOR( ABS_FREQ, HIGHEST, CFlip->Absolute.Freq,
						Cpu->Absolute.Freq );
	/* Core Processor events					*/
	memcpy( CFlip->Thermal.Events, RO(Core)->PowerThermal.Events,
		sizeof(CFlip->Thermal.Events) );
	/* Per Core, evaluate thermal properties.			*/
	CFlip->Thermal.Sensor	= RO(Core)->PowerThermal.Sensor;
	CFlip->Thermal.Param	= RO(Core)->PowerThermal.Param;

	ComputeThermalFormula[SCOPE_OF_FORMULA(RO(Shm)->Proc.thermalFormula)](
		CFlip, RO(Shm), cpu
	);

	/* Per Core, evaluate the voltage properties.			*/
	CFlip->Voltage.VID = RO(Core)->PowerThermal.VID;

	ComputeVoltageFormula[SCOPE_OF_FORMULA(RO(Shm)->Proc.voltageFormula)](
		CFlip, RO(Shm), cpu
	);

	/* Per Core, evaluate the Power properties.			*/
	CFlip->Delta.Power.ACCU = RO(Core)->Delta.Power.ACCU;

	ComputePowerFormula[SCOPE_OF_FORMULA(RO(Shm)->Proc.powerFormula)](
		CFlip, RO(Shm), cpu
	);

	/* Copy the Interrupts counters.				*/
	CFlip->Counter.SMI = RO(Core)->Interrupt.SMI;

	/* If driver registered, copy any NMI counter.			*/
	if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_LOCAL) == 1) {
		CFlip->Counter.NMI.LOCAL   = RO(Core)->Interrupt.NMI.LOCAL;
	}
	if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_UNKNOWN) == 1) {
		CFlip->Counter.NMI.UNKNOWN = RO(Core)->Interrupt.NMI.UNKNOWN;
	}
	if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_SERR) == 1) {
		CFlip->Counter.NMI.PCISERR = RO(Core)->Interrupt.NMI.PCISERR;
	}
	if (BITVAL(RO(Shm)->Registration.NMI, BIT_NMI_IO_CHECK) == 1) {
		CFlip->Counter.NMI.IOCHECK = RO(Core)->Interrupt.NMI.IOCHECK;
	}
    }
  } while (!BITVAL(Shutdown, SYNC) && !BITVAL(RO(Core)->OffLine, OS)) ;

	BITCLR_CC(BUS_LOCK, roomCore, cpu);
	BITCLR_CC(BUS_LOCK, roomSeed, cpu);
EXIT:
	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CYCLE %03u\n", (long) tid,
			BITVAL(RO(Core)->OffLine, OS) ? "Offline" : "Shutdown",
			cpu);
		fflush(stdout);
	}
	return NULL;
}

void SliceScheduling(	RO(SHM_STRUCT) *RO(Shm),
			unsigned int cpu, enum PATTERN pattern )
{
	unsigned int seek = 0;
	switch (pattern) {
	case RESET_CSP:
		for (seek = 0; seek < RO(Shm)->Proc.CPU.Count; seek++) {
			if (seek == RO(Shm)->Proc.Service.Core) {
				BITSET_CC(LOCKLESS, RO(Shm)->roomSched, seek);
			} else {
				BITCLR_CC(LOCKLESS, RO(Shm)->roomSched, seek);
			}
		}
		break;
	case ALL_SMT:
		if (cpu == RO(Shm)->Proc.Service.Core) {
			BITSTOR_CC(LOCKLESS, RO(Shm)->roomSched, roomSeed);
		}
		break;
	case RAND_SMT:
		do {
		  #ifdef __GLIBC__
		    if (random_r(&RO(Shm)->Cpu[cpu].Slice.Random.data,
				&RO(Shm)->Cpu[cpu].Slice.Random.value[0]) == 0)
		  #else
		    RO(Shm)->Cpu[cpu].Slice.Random.value[0] = (int) random();
		  #endif /* __GLIBC__ */
		    {
			seek = RO(Shm)->Cpu[cpu].Slice.Random.value[0]
				% RO(Shm)->Proc.CPU.Count;
		    }
		} while (BITVAL(RO(Shm)->Cpu[seek].OffLine, OS));
		BITCLR_CC(LOCKLESS, RO(Shm)->roomSched, cpu);
		BITSET_CC(LOCKLESS, RO(Shm)->roomSched, seek);
		break;
	case RR_SMT:
		seek = cpu;
		do {
			seek++;
			if (seek >= RO(Shm)->Proc.CPU.Count) {
				seek = 0;
			}
		} while (BITVAL(RO(Shm)->Cpu[seek].OffLine, OS));
		BITCLR_CC(LOCKLESS, RO(Shm)->roomSched, cpu);
		BITSET_CC(LOCKLESS, RO(Shm)->roomSched, seek);
		break;
	case USR_CPU:
		/*	NOP	*/
		break;
	}
}

static void *Child_Thread(void *arg)
{
	ARG *Arg	= (ARG *) arg;
	unsigned int cpu = Arg->Bind;

	RO(SHM_STRUCT) *RO(Shm) = Arg->Ref->RO(Shm);
	RW(SHM_STRUCT) *RW(Shm) = Arg->Ref->RW(Shm);
	CPU_STRUCT *Cpu = &RO(Shm)->Cpu[cpu];

	CALL_FUNC CallSliceFunc = (CALL_FUNC[2]){
		CallWith_RDTSC_No_RDPMC,  CallWith_RDTSC_RDPMC
	}[ RO(Shm)->Proc.Features.PerfMon.CoreCycles > 0 ];

	pthread_t tid = pthread_self();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpu, &cpuset);
	if (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset) != 0) {
		goto EXIT;
	}
	char *comm = malloc(TASK_COMM_LEN+10+1);
	if (comm != NULL) {
		snprintf(comm, TASK_COMM_LEN+10+1, "corefreqd#%u", cpu);
		pthread_setname_np(tid, comm);
		free(comm);
	}
	if (Quiet & 0x100) {
		printf("    Thread [%lx] Init CHILD %03u\n", (long) tid, cpu);
		fflush(stdout);
	}

	BITSET_CC(BUS_LOCK, roomSeed, cpu);

    do {
	while (!BITVAL(RW(Shm)->Proc.Sync, BURN)
	    && !BITVAL(Shutdown, SYNC)
	    && !BITVAL(Cpu->OffLine, OS)) {
		nanosleep(&RO(Shm)->Sleep.sliceWaiting, NULL);
	}

	BITSET_CC(BUS_LOCK, roomCore, cpu);

	RESET_Slice(Cpu->Slice);

	while ( BITVAL(RW(Shm)->Proc.Sync, BURN)
	    && !BITVAL(Shutdown, SYNC) )
	{
	    if (BITVAL_CC(RO(Shm)->roomSched, cpu)) {
		CallSliceFunc(	RO(Shm), RW(Shm), cpu,
				Arg->Ref->Slice.Func,
				Arg->Ref->Slice.arg);

		SliceScheduling(RO(Shm), cpu, Arg->Ref->Slice.pattern);

	      if (BITVAL(Cpu->OffLine, OS)) {	/* ReSchedule to Service */
		SliceScheduling(RO(Shm), RO(Shm)->Proc.Service.Core, RESET_CSP);
		break;
	      }
	    } else {
		nanosleep(&RO(Shm)->Sleep.sliceWaiting, NULL);

		if (BITVAL(Cpu->OffLine, OS)) {
			break;
		}
	    }
	}

	BITCLR_CC(BUS_LOCK, roomCore, cpu);

    } while (!BITVAL(Shutdown, SYNC) && !BITVAL(Cpu->OffLine, OS)) ;

	BITCLR_CC(BUS_LOCK, roomSeed, cpu);

	RESET_Slice(Cpu->Slice);
EXIT:
	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CHILD %03u\n", (long) tid,
			BITVAL(Cpu->OffLine, OS) ? "Offline" : "Shutdown",cpu);
		fflush(stdout);
	}
	return NULL;
}

void Architecture(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	Bit32	fTSC = RO(Proc)->Features.TSC,
		aTSC = RO(Proc)->Features.Inv_TSC;

	/*	Copy all initial CPUID features.			*/
	memcpy(&RO(Shm)->Proc.Features, &RO(Proc)->Features, sizeof(FEATURES));
	/*	Copy the fomula identifiers				*/
	RO(Shm)->Proc.thermalFormula = RO(Proc)->thermalFormula;
	RO(Shm)->Proc.voltageFormula = RO(Proc)->voltageFormula;
	RO(Shm)->Proc.powerFormula   = RO(Proc)->powerFormula;
	/*	Copy the numbers of total & online CPU.			*/
	RO(Shm)->Proc.CPU.Count	= RO(Proc)->CPU.Count;
	RO(Shm)->Proc.CPU.OnLine	= RO(Proc)->CPU.OnLine;
	RO(Shm)->Proc.Service.Proc	= RO(Proc)->Service.Proc;
	/*	Architecture and Hypervisor identifiers.		*/
	RO(Shm)->Proc.ArchID = RO(Proc)->ArchID;
	RO(Shm)->Proc.HypervisorID = RO(Proc)->HypervisorID;
	/*	Copy the Architecture name and Brand string.		*/
	StrCopy(RO(Shm)->Proc.Architecture,RO(Proc)->Architecture,CODENAME_LEN);
	StrCopy(RO(Shm)->Proc.Brand, RO(Proc)->Features.Info.Brand, BRAND_SIZE);
	/*	Compute the TSC mode: None, Variant, Invariant		*/
	RO(Shm)->Proc.Features.InvariantTSC = fTSC << aTSC;
}

void PerformanceMonitoring(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	switch (RO(Proc)->Features.PerfMon.Version) {
	case 0b0001: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x0};
		break;
	case 0b0100: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x1};
		break;
	case 0b0101: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x4};
		break;
	case 0b0110: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x5};
		break;
	case 0b0111: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x7};
		break;
	case 0b1000: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x8};
		break;
	case 0b1001: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0x9};
		break;
	case 0b1010: RO(Shm)->Proc.PM_ext = (struct PMU_ST){.v = 3, .p = 0xa};
		break;
	default: RO(Shm)->Proc.PM_version = 0;
		break;
	}
}

void HyperThreading(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{	/*	Update the HyperThreading state.			*/
	RO(Shm)->Proc.Features.HyperThreading = RO(Proc)->Features.HTT_Enable;
}

double ComputeTAU(unsigned char Y, unsigned char Z, double TU)
{
	return COMPUTE_TAU(Y, Z, TU);
}

void PowerInterface(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short pwrUnits = 0, pwrVal;
	enum PWR_DOMAIN pw;
	UNUSED(pwrUnits);
	UNUSED(pwrVal);
	UNUSED(pw);
    switch (KIND_OF_FORMULA(RO(Proc)->powerFormula)) {
    case POWER_KIND_NONE:
	break;
    }
}

void ThermalPoint(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	memcpy(&RO(Shm)->Proc.ThermalPoint, &RO(Proc)->ThermalPoint,
		sizeof(THERMAL_POINT));
}

void Technology_Update( RO(SHM_STRUCT) *RO(Shm),
			RO(PROC) *RO(Proc), RW(PROC) *RW(Proc) )
{	/*	Technologies aggregation.				*/
	RO(Shm)->Proc.Technology.VM = BITWISEAND_CC(LOCKLESS,
						RW(Proc)->VM,
						RO(Proc)->CR_Mask) != 0;
}

void Mitigation_Stage(	RO(SHM_STRUCT) *RO(Shm),
			RO(PROC) *RO(Proc), RW(PROC) *RW(Proc) )
{
	const unsigned short
		CLRBHB = BITWISEAND_CC( LOCKLESS,
					RW(Proc)->CLRBHB,
					RO(Proc)->SPEC_CTRL_Mask) != 0,

		CSV2_1 = BITWISEAND_CC( LOCKLESS,
					RW(Proc)->CSV2_1,
					RO(Proc)->SPEC_CTRL_Mask) != 0,

		CSV2_2 = BITWISEAND_CC( LOCKLESS,
					RW(Proc)->CSV2_2,
					RO(Proc)->SPEC_CTRL_Mask) != 0,

		CSV2_3 = BITWISEAND_CC( LOCKLESS,
					RW(Proc)->CSV2_3,
					RO(Proc)->SPEC_CTRL_Mask) != 0,

		CSV3 = BITWISEAND_CC(	LOCKLESS,
					RW(Proc)->CSV3,
					RO(Proc)->SPEC_CTRL_Mask) != 0,

		SSBS = BITCMP_CC(	LOCKLESS,
					RW(Proc)->SSBS,
					RO(Proc)->SPEC_CTRL_Mask );

	RO(Shm)->Proc.Mechanisms.CLRBHB = CLRBHB ? 0b11 : 0b00;

	RO(Shm)->Proc.Mechanisms.CSV2 = CSV_NONE;
    if (CSV2_1) {
	if (RO(Shm)->Proc.Features.CSV2 == 0b0001) {
		RO(Shm)->Proc.Mechanisms.CSV2 = CSV2_1p1;
	} else if (RO(Shm)->Proc.Features.CSV2 == 0b0010) {
		RO(Shm)->Proc.Mechanisms.CSV2 = CSV2_1p2;
	} else if (RO(Shm)->Proc.Features.CSV2 == 0b0000) {
		RO(Shm)->Proc.Mechanisms.CSV2 = CSV2_1p0;
	}
    } else if (CSV2_2) {
	RO(Shm)->Proc.Mechanisms.CSV2 = CSV2_2p0;
    } else if (CSV2_3) {
	RO(Shm)->Proc.Mechanisms.CSV2 = CSV2_3p0;
    }
	RO(Shm)->Proc.Mechanisms.CSV3 = CSV3 ? 0b11 : 0b00;

	switch (RO(Shm)->Proc.Features.SSBS) {
	case 0b0001:
	case 0b0010:
		RO(Shm)->Proc.Mechanisms.SSBS = 1;
		break;
	case 0b0000:
	default:
		RO(Shm)->Proc.Mechanisms.SSBS = 0;
		break;
	}
	RO(Shm)->Proc.Mechanisms.SSBS += (2 * SSBS);
}

static char *Chipset[CHIPSETS] = {
	[IC_CHIPSET]		= NULL,
};


void Uncore_Update(	RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc),
			RO(CORE) *RO(Core) )
{
	/*	Copy the # of controllers.				*/
	RO(Shm)->Uncore.CtrlCount = RO(Proc)->Uncore.CtrlCount;
	/*	Decode the Memory Controller for each found vendor:device */
	Chipset[IC_CHIPSET] = RO(Proc)->Features.Info.Vendor.ID;
	RO(Shm)->Uncore.ChipID	=  RO(Proc)->Uncore.ClusterRev.Revision
				| (RO(Proc)->Uncore.ClusterRev.Variant << 4);
	RO(Shm)->Uncore.Chipset.ArchID = IC_CHIPSET;
	/*	Copy the chipset codename.				*/
	StrCopy(RO(Shm)->Uncore.Chipset.CodeName,
		Chipset[RO(Shm)->Uncore.Chipset.ArchID],
		CODENAME_LEN);
	/*	Copy the Uncore clock ratios.				*/
	memcpy( RO(Shm)->Uncore.Boost,
		RO(Proc)->Uncore.Boost,
		(UNCORE_BOOST(SIZE)) * sizeof(COF_ST) );
}

void Topology(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) **RO(Core),
		unsigned int cpu)
{
	unsigned int level;
	/*	Copy each Core topology.				*/
	RO(Shm)->Cpu[cpu].Topology.PN = RO(Core, AT(cpu))->T.PN;
	RO(Shm)->Cpu[cpu].Topology.BSP = RO(Core, AT(cpu))->T.BSP ? 1:0;
	RO(Shm)->Cpu[cpu].Topology.MPID = RO(Core, AT(cpu))->T.MPID;
	RO(Shm)->Cpu[cpu].Topology.CoreID = RO(Core, AT(cpu))->T.CoreID;
	RO(Shm)->Cpu[cpu].Topology.ThreadID = RO(Core, AT(cpu))->T.ThreadID;
	RO(Shm)->Cpu[cpu].Topology.PackageID = RO(Core, AT(cpu))->T.PackageID;
	RO(Shm)->Cpu[cpu].Topology.Cluster.ID = RO(Core, AT(cpu))->T.Cluster.ID;
	RO(Shm)->Cpu[cpu].Topology.Cluster.Hybrid_ID = \
					RO(Core, AT(cpu))->T.Cluster.Hybrid_ID;
	/*	Aggregate the Caches topology.				*/
  for (level = 0; level < CACHE_MAX_LEVEL; level++)
    if (RO(Core, AT(cpu))->T.Cache[level].ccsid.value != 0) {
	RO(Shm)->Cpu[cpu].Topology.Cache[level].LineSz = \
			RO(Core, AT(cpu))->T.Cache[level].ccsid.LineSz + 4;

	RO(Shm)->Cpu[cpu].Topology.Cache[level].Set = \
		RO(Core, AT(cpu))->T.Cache[level].ccsid.FEAT_CCIDX ?
			RO(Core, AT(cpu))->T.Cache[level].ccsid.NumSets + 1
		:	RO(Core, AT(cpu))->T.Cache[level].ccsid.Set + 1;

	RO(Shm)->Cpu[cpu].Topology.Cache[level].Way = \
			RO(Core, AT(cpu))->T.Cache[level].ccsid.Assoc + 1;

	RO(Shm)->Cpu[cpu].Topology.Cache[level].Size = \
			RO(Shm)->Cpu[cpu].Topology.Cache[level].Way
			<< RO(Shm)->Cpu[cpu].Topology.Cache[level].LineSz;

	RO(Shm)->Cpu[cpu].Topology.Cache[level].Size = \
			RO(Shm)->Cpu[cpu].Topology.Cache[level].Set
			* RO(Shm)->Cpu[cpu].Topology.Cache[level].Size;

	RO(Shm)->Cpu[cpu].Topology.Cache[level].Feature.WriteBack = \
			RO(Core, AT(cpu))->T.Cache[level].ccsid.WrBack;
    }
}

void CStates(RO(SHM_STRUCT) *RO(Shm), RO(CORE) **RO(Core), unsigned int cpu)
{	/*	Copy the C-State Configuration Control			*/
	RO(Shm)->Cpu[cpu].Query.CfgLock = RO(Core, AT(cpu))->Query.CfgLock;

	RO(Shm)->Cpu[cpu].Query.CStateLimit = \
					RO(Core, AT(cpu))->Query.CStateLimit;
	/*	Copy the Max C-State Inclusion				*/
	RO(Shm)->Cpu[cpu].Query.IORedir = RO(Core, AT(cpu))->Query.IORedir;
	/*	Copy any architectural C-States I/O Base Address	*/
	RO(Shm)->Cpu[cpu].Query.CStateBaseAddr = \
					RO(Core, AT(cpu))->Query.CStateBaseAddr;
}

void PowerThermal(	RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc),
			RO(CORE) **RO(Core), unsigned int cpu )
{
	UNUSED(RO(Proc));

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Highest = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Highest;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Guaranteed = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Guaranteed;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Most_Efficient = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Most_Efficient;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Capabilities.Lowest = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Lowest;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Minimum_Perf;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Maximum_Perf;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Desired_Perf;

	RO(Shm)->Cpu[cpu].PowerThermal.HWP.Request.Energy_Pref = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Energy_Pref;

	memcpy( &RO(Shm)->Cpu[cpu].ThermalPoint,
		&RO(Core, AT(cpu))->ThermalPoint,
		sizeof(THERMAL_POINT) );
}

void SystemRegisters(	RO(SHM_STRUCT) *RO(Shm), RO(CORE) **RO(Core),
			unsigned int cpu )
{
	RO(Shm)->Cpu[cpu].SystemRegister.FLAGS = \
				RO(Core, AT(cpu))->SystemRegister.FLAGS;

	RO(Shm)->Cpu[cpu].SystemRegister.HCR = \
				RO(Core, AT(cpu))->SystemRegister.HCR;

	RO(Shm)->Cpu[cpu].SystemRegister.SCTLR = \
				RO(Core, AT(cpu))->SystemRegister.SCTLR;

	RO(Shm)->Cpu[cpu].SystemRegister.SCTLR2 = \
				RO(Core, AT(cpu))->SystemRegister.SCTLR2;

	RO(Shm)->Cpu[cpu].SystemRegister.EL = \
				RO(Core, AT(cpu))->SystemRegister.EL;

	RO(Shm)->Cpu[cpu].SystemRegister.FPSR = \
				RO(Core, AT(cpu))->SystemRegister.FPSR;

	RO(Shm)->Cpu[cpu].SystemRegister.FPCR = \
				RO(Core, AT(cpu))->SystemRegister.FPCR;

	RO(Shm)->Cpu[cpu].SystemRegister.SVCR = \
				RO(Core, AT(cpu))->SystemRegister.SVCR;

	RO(Shm)->Cpu[cpu].SystemRegister.CPACR = \
				RO(Core, AT(cpu))->SystemRegister.CPACR;

	RO(Shm)->Cpu[cpu].Query.SCTLRX = RO(Core, AT(cpu))->Query.SCTLRX;
}

void SysGate_OS_Driver(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	memset(&RO(Shm)->SysGate.OS, 0, sizeof(OS_DRIVER));
    if (strlen(RO(Proc)->OS.IdleDriver.Name) > 0) {
	int idx;

	StrCopy(RO(Shm)->SysGate.OS.IdleDriver.Name,
		RO(Proc)->OS.IdleDriver.Name,
		CPUIDLE_NAME_LEN);

	RO(Shm)->SysGate.OS.IdleDriver.stateCount = \
					RO(Proc)->OS.IdleDriver.stateCount;

	RO(Shm)->SysGate.OS.IdleDriver.stateLimit = \
					RO(Proc)->OS.IdleDriver.stateLimit;

	for (idx = 0; idx < RO(Shm)->SysGate.OS.IdleDriver.stateCount; idx++)
	{
		StrCopy(RO(Shm)->SysGate.OS.IdleDriver.State[idx].Name,
			RO(Proc)->OS.IdleDriver.State[idx].Name,
			CPUIDLE_NAME_LEN);

		StrCopy(RO(Shm)->SysGate.OS.IdleDriver.State[idx].Desc,
			RO(Proc)->OS.IdleDriver.State[idx].Desc,
			CPUIDLE_NAME_LEN);

		RO(Shm)->SysGate.OS.IdleDriver.State[idx].exitLatency = \
			RO(Proc)->OS.IdleDriver.State[idx].exitLatency;

		RO(Shm)->SysGate.OS.IdleDriver.State[idx].powerUsage = \
			RO(Proc)->OS.IdleDriver.State[idx].powerUsage;

		RO(Shm)->SysGate.OS.IdleDriver.State[idx].targetResidency = \
			RO(Proc)->OS.IdleDriver.State[idx].targetResidency;
	}
    }
    if (strlen(RO(Proc)->OS.FreqDriver.Name) > 0) {
	StrCopy(RO(Shm)->SysGate.OS.FreqDriver.Name,
		RO(Proc)->OS.FreqDriver.Name,
		CPUFREQ_NAME_LEN);
    }
    if (strlen(RO(Proc)->OS.FreqDriver.Governor) > 0) {
	StrCopy(RO(Shm)->SysGate.OS.FreqDriver.Governor,
		RO(Proc)->OS.FreqDriver.Governor,
		CPUFREQ_NAME_LEN);
    }
}

void SysGate_Kernel(REF *Ref)
{
	RO(SHM_STRUCT) *RO(Shm) = Ref->RO(Shm);
	RO(SYSGATE) *SysGate = Ref->RO(SysGate);

	RO(Shm)->SysGate.kernel.version = SysGate->kernelVersionNumber >> 16;
	RO(Shm)->SysGate.kernel.major = SysGate->kernelVersionNumber >> 8;
	RO(Shm)->SysGate.kernel.major = RO(Shm)->SysGate.kernel.major & 0xff;
	RO(Shm)->SysGate.kernel.minor = SysGate->kernelVersionNumber & 0xff;

	memcpy(RO(Shm)->SysGate.sysname, SysGate->sysname, MAX_UTS_LEN);
	memcpy(RO(Shm)->SysGate.release, SysGate->release, MAX_UTS_LEN);
	memcpy(RO(Shm)->SysGate.version, SysGate->version, MAX_UTS_LEN);
	memcpy(RO(Shm)->SysGate.machine, SysGate->machine, MAX_UTS_LEN);
}

#define CS_Reset_Array(_CS)						\
({									\
	_CS.index[0] = 0;						\
})

#define CS_Seek_Store(_fd, _ptr, _CS)					\
({									\
	size_t	length; 						\
	if (fscanf(_fd, "%s", _ptr) == 1) {				\
		unsigned char offset;					\
		length = strlen(_ptr);					\
		_ptr += length; 					\
		(*_ptr) = 0x0;						\
		_ptr++; 						\
		length = _ptr - _CS.array;				\
		offset = (unsigned char) length;			\
		_CS.index[0]++; 					\
		_CS.index[_CS.index[0]] = offset;			\
	}								\
})

void ClockSource_Update(RO(SHM_STRUCT) *RO(Shm))
{
	FILE	*fd;

	CS_Reset_Array(RO(Shm)->CS);

    if ((fd = fopen(CLOCKSOURCE_PATH"/current_clocksource", "r")) != NULL)
    {
	char *ptr = RO(Shm)->CS.array;

	CS_Seek_Store(fd, ptr, RO(Shm)->CS);

	fclose(fd);
	if ((fd = fopen(CLOCKSOURCE_PATH"/available_clocksource", "r")) != NULL)
	{
		do {
			CS_Seek_Store(fd, ptr, RO(Shm)->CS);
		} while (!feof(fd) && (RO(Shm)->CS.index[0] < 7));
		fclose(fd);
	}
    }
}

long ClockSource_Submit(RO(SHM_STRUCT) *RO(Shm), unsigned char index)
{
    if (index <= RO(Shm)->CS.index[0])
    {
	FILE	*fd;
	if ((fd = fopen(CLOCKSOURCE_PATH"/current_clocksource", "w")) != NULL)
	{
		char *ptr = &RO(Shm)->CS.array[RO(Shm)->CS.index[index]];
		fprintf(fd, "%s", ptr);
		fclose(fd);
		return 0;
	}
    }
	return -EINVAL;
}

static const int reverseSign[2] = {+1, -1};

static int SortByRuntime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = task1->runtime < task2->runtime ? +1 : -1;
	sort *= reverseSign[Ref->RO(Shm)->SysGate.reverseOrder];
	return sort;
}

static int SortByUsertime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = task1->usertime < task2->usertime ? +1 : -1;
	sort *= reverseSign[Ref->RO(Shm)->SysGate.reverseOrder];
	return sort;
}

static int SortBySystime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = task1->systime < task2->systime ? +1 : -1;
	sort *= reverseSign[Ref->RO(Shm)->SysGate.reverseOrder];
	return sort;
}

static int SortByState(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = task1->state < task2->state ? -1 : +1;
	sort *= reverseSign[Ref->RO(Shm)->SysGate.reverseOrder];
	return sort;
}

static int SortByPID(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = task1->pid < task2->pid ? -1 : +1;
	sort *= reverseSign[Ref->RO(Shm)->SysGate.reverseOrder];
	return sort;
}

static int SortByCommand(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = strncmp(task1->comm, task2->comm, TASK_COMM_LEN);
	sort *= reverseSign[Ref->RO(Shm)->SysGate.reverseOrder];
	return sort;
}

typedef int (*SORTBYFUNC)(const void *, const void *, void *);

static SORTBYFUNC SortByFunc[SORTBYCOUNT] = {
	SortByState,
	SortByRuntime,
	SortByUsertime,
	SortBySystime,
	SortByPID,
	SortByCommand
};

static int SortByTracker(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	REF *Ref = (REF*) arg;

	int sort = (task1->pid == Ref->RO(Shm)->SysGate.trackTask) ?
		-1 : (task2->pid == Ref->RO(Shm)->SysGate.trackTask) ?
		+1 : SortByFunc[Ref->RO(Shm)->SysGate.sortByField](p1, p2, Ref);
	return sort;
}

void SysGate_Update(REF *Ref)
{
	struct sysinfo info;
	RO(SHM_STRUCT) *RO(Shm) = Ref->RO(Shm);
	RO(SYSGATE) *SysGate = Ref->RO(SysGate);
	RO(PROC) *RO(Proc) = Ref->RO(Proc);

	RO(Shm)->SysGate.taskCount = SysGate->taskCount;

	memcpy( RO(Shm)->SysGate.taskList, SysGate->taskList,
		(size_t) RO(Shm)->SysGate.taskCount * sizeof(TASK_MCB));

	qsort_r(RO(Shm)->SysGate.taskList,
		(size_t) RO(Shm)->SysGate.taskCount, sizeof(TASK_MCB),
		RO(Shm)->SysGate.trackTask ?
			  SortByTracker
			: SortByFunc[RO(Shm)->SysGate.sortByField], Ref);

    if (sysinfo(&info) == 0) {
	RO(Shm)->SysGate.memInfo.totalram  = info.totalram	>> 10;
	RO(Shm)->SysGate.memInfo.sharedram = info.sharedram	>> 10;
	RO(Shm)->SysGate.memInfo.freeram   = info.freeram	>> 10;
	RO(Shm)->SysGate.memInfo.bufferram = info.bufferram	>> 10;
	RO(Shm)->SysGate.memInfo.totalhigh = info.totalhigh	>> 10;
	RO(Shm)->SysGate.memInfo.freehigh  = info.freehigh	>> 10;
	RO(Shm)->SysGate.procCount = info.procs;
    }
	RO(Shm)->SysGate.OS.IdleDriver.stateLimit =
					RO(Proc)->OS.IdleDriver.stateLimit;
}

void Package_Update(	RO(SHM_STRUCT) *RO(Shm),
			RO(PROC) *RO(Proc), RW(PROC) *RW(Proc) )
{	/*	Copy the operational settings.				*/
	RO(Shm)->Registration.AutoClock = RO(Proc)->Registration.AutoClock;
	RO(Shm)->Registration.Experimental= RO(Proc)->Registration.Experimental;
	RO(Shm)->Registration.HotPlug = RO(Proc)->Registration.HotPlug;
	RO(Shm)->Registration.PCI = RO(Proc)->Registration.PCI;
	BITSTOR(LOCKLESS,RO(Shm)->Registration.NMI, RO(Proc)->Registration.NMI);
	RO(Shm)->Registration.Driver = RO(Proc)->Registration.Driver;
	/*	Copy the timer interval delay.				*/
	RO(Shm)->Sleep.Interval = RO(Proc)->SleepInterval;
	/*	Compute the polling wait time based on the timer interval. */
	RO(Shm)->Sleep.pollingWait=TIMESPEC((RO(Shm)->Sleep.Interval * 1000000L)
					/ WAKEUP_RATIO);
	/*	Copy the SysGate tick steps.				*/
	RO(Shm)->SysGate.tickReset = RO(Proc)->tickReset;
	RO(Shm)->SysGate.tickStep  = RO(Proc)->tickStep;

	Architecture(RO(Shm), RO(Proc));

	PerformanceMonitoring(RO(Shm), RO(Proc));

	HyperThreading(RO(Shm), RO(Proc));

	PowerInterface(RO(Shm), RO(Proc));

	ThermalPoint(RO(Shm), RO(Proc));

	Mitigation_Stage(RO(Shm), RO(Proc), RW(Proc));
	/*	Aggregate OS idle driver data and Clock Source		*/
	SysGate_OS_Driver(RO(Shm), RO(Proc));
	ClockSource_Update(RO(Shm));
}

void PerCore_Update(	RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc),
			RO(CORE) **RO(Core), unsigned int cpu )
{
	if (BITVAL(RO(Core, AT(cpu))->OffLine, HW)) {
		BITSET(LOCKLESS, RO(Shm)->Cpu[cpu].OffLine, HW);
	} else {
		BITCLR(LOCKLESS, RO(Shm)->Cpu[cpu].OffLine, HW);
	}
	/*	Initialize all clock ratios.				*/
	memcpy( RO(Shm)->Cpu[cpu].Boost, RO(Core, AT(cpu))->Boost,
		(BOOST(SIZE)) * sizeof(COF_ST) );

	RO(Shm)->Cpu[cpu].Query.Revision = RO(Core, AT(cpu))->Query.Revision;

	Topology(RO(Shm), RO(Proc), RO(Core), cpu);

	CStates(RO(Shm), RO(Core), cpu);

	PowerThermal(RO(Shm), RO(Proc), RO(Core), cpu);

	SystemRegisters(RO(Shm), RO(Core), cpu);
}

#define SysOnce(drv)	ioctl(drv, COREFREQ_IOCTL_SYSONCE)

int SysGate_OnDemand(REF *Ref, int operation)
{
	int rc = -1;
	const size_t allocPages = Ref->RO(Proc)->Gate.ReqMem.Size;

	if (operation == 0) {
	    if (Ref->RO(SysGate) != NULL) {
		if ((rc = munmap(Ref->RO(SysGate), allocPages)) == 0) {
			Ref->RO(SysGate) = NULL;
		}
	    } else {
		rc = -1;
	    }
	} else {
	    if (Ref->RO(SysGate) == NULL) {
		const off_t vm_pgoff = ID_RO_VMA_GATE * PAGE_SIZE;
		RO(SYSGATE) *MapGate = mmap(NULL, allocPages,
						PROT_READ,
						MAP_SHARED,
						Ref->fd->drv, vm_pgoff);
		if (MapGate != MAP_FAILED) {
			Ref->RO(SysGate) = MapGate;
			rc = 0;
		}
	    } else {
		rc = 0;
	    }
	}
	return rc;
}

void SysGate_Toggle(REF *Ref, unsigned int state)
{
    if (state == 0) {
	if (BITWISEAND(LOCKLESS, Ref->RO(Shm)->SysGate.Operation, 0x1)) {
		/*		Stop SysGate 				*/
		BITCLR(LOCKLESS, Ref->RO(Shm)->SysGate.Operation, 0);
		/*		Notify					*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
	}
    } else {
	if (!BITWISEAND(LOCKLESS, Ref->RO(Shm)->SysGate.Operation, 0x1)) {
	    if (SysGate_OnDemand(Ref, 1) == 0) {
		/*		Start SysGate				*/
		BITSET(LOCKLESS, Ref->RO(Shm)->SysGate.Operation, 0);
		/*		Notify					*/
		BITWISESET(LOCKLESS, PendingSync,BIT_MASK_NTFY);
	    }
	}
    }
    if (BITWISEAND(LOCKLESS, Ref->RO(Shm)->SysGate.Operation, 0x1))
	if (SysOnce(Ref->fd->drv) == RC_OK_SYSGATE) {
		/*		Copy system information.		*/
		SysGate_Kernel(Ref);
	}
}

void Master_Ring_Handler(REF *Ref, unsigned int rid)
{
    if (!RING_NULL(Ref->RW(Shm)->Ring[rid]))
    {
	RING_CTRL ctrl __attribute__ ((aligned(16)));
	RING_READ(Ref->RW(Shm)->Ring[rid], ctrl);
	int rc = -EPERM, drc = -EPERM;

	if (	(ctrl.cmd >= COREFREQ_IOCTL_SYSUPDT)
	&&	(ctrl.cmd <= COREFREQ_IOCTL_CLEAR_EVENTS) )
	{
		rc = ioctl(Ref->fd->drv, ctrl.cmd, ctrl.arg);
		drc = errno;
	}
	if (Quiet & 0x100) {
		printf("\tRING[%u](%x,%x)(%llx)>(%d,%d)\n",
			rid, ctrl.cmd, ctrl.sub, ctrl.arg, rc, drc);
	}
	switch (rc) {
	case -EPERM:
	    {
		RING_CTRL error __attribute__ ((aligned(16))) = {
			.arg = ctrl.arg,
			.cmd = ctrl.cmd,
			.drc = (unsigned int) drc,
			.tds = ELAPSED(Ref->RO(Shm)->StartedAt)
		};
		RING_WRITE_1xPARAM(	error.sub,
					Ref->RW(Shm)->Error,
					error.cmd,
					error.arg );
	    }
		break;
	case RC_OK_SYSGATE:
		SysGate_OS_Driver(Ref->RO(Shm), Ref->RO(Proc));
		fallthrough;
	case RC_SUCCESS: /* Platform changed -> pending notification.	*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
		break;
	case RC_OK_COMPUTE: /* Compute claimed -> pending notification. */
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_COMP);
		break;
	}
    }
}

void Child_Ring_Handler(REF *Ref, unsigned int rid)
{
  if (!RING_NULL(Ref->RW(Shm)->Ring[rid]))
  {
	RING_CTRL ctrl __attribute__ ((aligned(16)));
	RING_READ(Ref->RW(Shm)->Ring[rid], ctrl);

   switch (ctrl.cmd) {
   case COREFREQ_KERNEL_MISC:
     switch (ctrl.dl.hi) {
     case MACHINE_CLOCK_SOURCE:
	if (ClockSource_Submit(Ref->RO(Shm), (unsigned char) ctrl.dl.lo) == 0) {
		ClockSource_Update(Ref->RO(Shm));
	}
	break;
     }
     break;
   case COREFREQ_SESSION_APP:
	switch (ctrl.sub) {
	case SESSION_CLI:
		Ref->RO(Shm)->App.Cli = (pid_t) ctrl.arg;
		break;
	case SESSION_GUI:
		Ref->RO(Shm)->App.GUI = (pid_t) ctrl.arg;
		break;
	}
	break;
   case COREFREQ_TASK_MONITORING:
	switch (ctrl.sub) {
	case TASK_TRACKING:
		Ref->RO(Shm)->SysGate.trackTask = (pid_t) ctrl.arg;
		break;
	case TASK_SORTING:
	    {
		enum SORTBYFIELD sortByField = (enum SORTBYFIELD) ctrl.dl.lo;
		Ref->RO(Shm)->SysGate.sortByField = sortByField % SORTBYCOUNT;
	    }
		break;
	case TASK_INVERSING:
	    {
		const int reverseOrder = !!!Ref->RO(Shm)->SysGate.reverseOrder;
		Ref->RO(Shm)->SysGate.reverseOrder = reverseOrder;
	    }
		break;
	}
	BITWISESET(LOCKLESS, Ref->RW(Shm)->Proc.Sync, BIT_MASK_NTFY);
	break;
   case COREFREQ_TOGGLE_SYSGATE:
	switch (ctrl.arg) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		SysGate_Toggle(Ref, ctrl.arg);
		BITWISESET(LOCKLESS, Ref->RW(Shm)->Proc.Sync, BIT_MASK_NTFY);
		break;
	}
	break;
   case COREFREQ_ORDER_MACHINE:
	switch (ctrl.arg) {
	case COREFREQ_TOGGLE_OFF:
	SCHEDULER_STOP:
	    if (BITVAL(Ref->RW(Shm)->Proc.Sync, BURN))
	    {
		BITCLR(BUS_LOCK, Ref->RW(Shm)->Proc.Sync, BURN);

		while (BITWISEAND_CC(BUS_LOCK, roomCore, roomSeed))
		{
			if (BITVAL(Shutdown, SYNC)) {	/* SpinLock */
				break;
			}
		}
		BITSTOR_CC(BUS_LOCK, Ref->RO(Shm)->roomSched, roomClear);

		Ref->Slice.Func = Slice_NOP;
		Ref->Slice.arg = 0;
		Ref->Slice.pattern = RESET_CSP;
		/*	Notify the Slice module has stopped		*/
		BITWISESET(LOCKLESS, Ref->RW(Shm)->Proc.Sync, BIT_MASK_NTFY);
	    }
	    break;
	}
	break;
   default:
    {
	RING_SLICE *porder = order_list;

     while (porder->func != NULL)
     {
      if ((porder->ctrl.cmd == ctrl.cmd) &&  (porder->ctrl.sub == ctrl.sub))
      {
       if ( !BITVAL(Ref->RW(Shm)->Proc.Sync, BURN)
	|| ((Ref->Slice.Func == Slice_Turbo) && (Ref->Slice.pattern == USR_CPU)
		&& (ctrl.cmd == COREFREQ_ORDER_TURBO) && (ctrl.sub == USR_CPU)))
       {
	if (ctrl.sub == USR_CPU) {
	    if (BITVAL_CC(Ref->RO(Shm)->roomSched, ctrl.dl.lo))
	    {
		BITCLR_CC(LOCKLESS, Ref->RO(Shm)->roomSched, ctrl.dl.lo);

		if (!BITWISEAND_CC(BUS_LOCK, Ref->RO(Shm)->roomSched, roomSeed))
		{
			goto SCHEDULER_STOP;
		}
	    } else {
		BITSET_CC(LOCKLESS, Ref->RO(Shm)->roomSched, ctrl.dl.lo);

		goto SCHEDULER_START;
	    }
	} else {
		SliceScheduling(Ref->RO(Shm), ctrl.dl.lo, porder->pattern);

	SCHEDULER_START:
		Ref->Slice.Func = porder->func;
		Ref->Slice.arg  = porder->ctrl.dl.lo;
		Ref->Slice.pattern = porder->pattern;

		BITSET(BUS_LOCK, Ref->RW(Shm)->Proc.Sync, BURN);
	}
	/*	Notify the Slice module is starting up			*/
	BITWISESET(LOCKLESS, Ref->RW(Shm)->Proc.Sync, BIT_MASK_NTFY);
       }
       break;
      }
	porder++;
     }
    }
    break;
   }
    if (Quiet & 0x100) {
	printf("\tRING[%u](%x,%x)(%hx:%hx,%hx:%hx)\n",
		rid, ctrl.cmd, ctrl.sub,
		ctrl.dh.hi, ctrl.dh.lo, ctrl.dl.hi, ctrl.dl.lo);
    }
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
		if (pSlave->Thread != -1) {
			const signed int cpu = pSlave->Thread;
			CPU_SET(cpu , &cpuset);
		}
		return pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	}
	return -1;
}

static void *Emergency_Handler(void *pRef)
{
	REF *Ref = (REF *) pRef;
	unsigned int rid = (Ref->CPID == 0);
	SERVICE_PROC localService = RESET_SERVICE;
	int caught = 0, leave = 0;
	char handlerName[TASK_COMM_LEN] = {
		'c','o','r','e','f','r','e','q','d','-','r','i','n','g','0',0
	};
	handlerName[14] += (char) rid;

	pthread_t tid = pthread_self();

    if(ServerFollowService(&localService, &Ref->RO(Shm)->Proc.Service,tid) == 0)
    {
	pthread_setname_np(tid, handlerName);
    }
    while (!leave) {
	caught=sigtimedwait(&Ref->Signal,NULL,&Ref->RO(Shm)->Sleep.ringWaiting);
	if (caught != -1) {
		switch (caught) {
		case SIGUSR2:
			if (Ref->CPID) { /* Stop  SysGate */
				SysGate_Toggle(Ref, 0);
			}
			break;
		case SIGUSR1:
			if (Ref->CPID) { /* Start SysGate */
				SysGate_Toggle(Ref, 1);
			}
			break;
		case SIGCHLD: /* Exit Ring Thread  */
			leave = 0x1;
			fallthrough;
		case SIGSTKFLT:
		case SIGXFSZ:
		case SIGXCPU:
		case SIGSEGV:
		case SIGTERM:
		case SIGQUIT:
		case SIGALRM:
		case SIGABRT:
		case SIGPIPE:
		case SIGTRAP:
		case SIGSYS:
		case SIGFPE:
		case SIGBUS:
		case SIGILL:
		case SIGINT:	/* [CTRL] + [C] */
			BITSET(LOCKLESS, Shutdown, SYNC);
			break;
		case SIGVTALRM:
		case SIGWINCH:
		case SIGTTOU:
		case SIGTTIN:
		case SIGTSTP:
		case SIGPROF:
		case SIGHUP:
		case SIGPWR:
		case SIGIO:
		default:	/* RTMIN ... RTMAX */
			break;
		}
	} else if (errno == EAGAIN) {
		if (Ref->CPID) {
			Master_Ring_Handler(Ref, rid);
		} else {
			Child_Ring_Handler(Ref, rid);
		}
	}
	ServerFollowService(&localService, &Ref->RO(Shm)->Proc.Service, tid);
    }
	return NULL;
}

void Emergency_Command(REF *Ref, unsigned int cmd)
{
	switch (cmd) {
	case 0:
		if (Ref->Started) {
			if (!pthread_kill(Ref->KID, SIGCHLD)) {
				if (!pthread_join(Ref->KID, NULL)) {
					Ref->Started = 0;
				}
			}
		}
		break;
	case 1: {
		const int ignored[] = {
			SIGIO, SIGPROF, SIGPWR, SIGHUP, SIGTSTP,
			SIGTTIN, SIGTTOU, SIGVTALRM, SIGWINCH
		}, handled[] = {
			SIGUSR1, SIGUSR2, SIGCHLD, SIGINT, SIGILL, SIGBUS,
			SIGFPE, SIGSYS, SIGTRAP, SIGPIPE, SIGABRT, SIGALRM,
			SIGQUIT, SIGTERM, SIGSEGV, SIGXCPU, SIGXFSZ, SIGSTKFLT
		};
		/* SIGKILL,SIGCONT,SIGSTOP,SIGURG:	Reserved	*/
		const ssize_t	ignoredCount = sizeof(ignored) / sizeof(int),
				handledCount = sizeof(handled) / sizeof(int);
		int signo;

		sigemptyset(&Ref->Signal);
		for (signo = SIGRTMIN; signo <= SIGRTMAX; signo++) {
			sigaddset(&Ref->Signal, signo);
		}
		for (signo = 0; signo < ignoredCount; signo++) {
			sigaddset(&Ref->Signal, ignored[signo]);
		}
		for (signo = 0; signo < handledCount; signo++) {
			sigaddset(&Ref->Signal, handled[signo]);
		}
		if (!pthread_sigmask(SIG_BLOCK, &Ref->Signal, NULL))
		{
		    if(!pthread_create(&Ref->KID, NULL, Emergency_Handler, Ref))
		    {
			Ref->Started = 1;
		    }
		}
	    }
		break;
	}
}

static void Pkg_ComputeThermal_None(	struct PKG_FLIP_FLOP *PFlip,
					struct FLIP_FLOP *SProc )
{
	UNUSED(PFlip);
	UNUSED(SProc);
}

static void Pkg_ComputeVoltage_None(struct PKG_FLIP_FLOP *PFlip)
{
	UNUSED(PFlip);
}

static void Pkg_ComputeVoltage_OPP(struct PKG_FLIP_FLOP *PFlip)
{
	COMPUTE_VOLTAGE(OPP,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);
}

static void Pkg_ComputePower_None(RW(PROC) *RW(Proc), struct FLIP_FLOP *CFlop)
{
	UNUSED(RW(Proc));
	UNUSED(CFlop);
}

static void Pkg_ResetPower_None(PROC_RW *Proc)
{
	UNUSED(Proc);
}

void Pkg_ResetSensorLimits(PROC_STRUCT *Pkg)
{
	enum PWR_DOMAIN pw;
      for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
      {
	RESET_SENSOR_LIMIT(ENERGY, LOWEST , Pkg->State.Energy[pw].Limit);
	RESET_SENSOR_LIMIT(ENERGY, HIGHEST, Pkg->State.Energy[pw].Limit );
	RESET_SENSOR_LIMIT(POWER , LOWEST , Pkg->State.Power[pw].Limit );
	RESET_SENSOR_LIMIT(POWER , HIGHEST, Pkg->State.Power[pw].Limit );
      }
	RESET_SENSOR_LIMIT(VOLTAGE, LOWEST, Pkg->State.Voltage.Limit );
	RESET_SENSOR_LIMIT(VOLTAGE, HIGHEST,Pkg->State.Voltage.Limit );
}

REASON_CODE Core_Manager(REF *Ref)
{
	RO(SHM_STRUCT)		*RO(Shm) = Ref->RO(Shm);
	RW(SHM_STRUCT)		*RW(Shm) = Ref->RW(Shm);
	RO(PROC)		*RO(Proc) = Ref->RO(Proc);
	RW(PROC)		*RW(Proc) = Ref->RW(Proc);
	RO(CORE)		**RO(Core) = Ref->RO(Core);
	struct PKG_FLIP_FLOP	*PFlip;
	struct FLIP_FLOP	*SProc;
	SERVICE_PROC		localService = RESET_SERVICE;
	struct {
		double		RelFreq,
				AbsFreq;
	} prevTop;
	REASON_INIT		(reason);
	unsigned int		cpu = 0;

	pthread_t tid = pthread_self();
    #ifdef __GLIBC__
	RO(Shm)->App.Svr = tid;
    #else
	RO(Shm)->App.Svr = getpid();
    #endif

  if (ServerFollowService(&localService, &RO(Shm)->Proc.Service, tid) == 0) {
	pthread_setname_np(tid, "corefreqd-pmgr");
  }
	ARG *Arg = calloc(RO(Shm)->Proc.CPU.Count, sizeof(ARG));
  if (Arg != NULL)
  {
	void (*Pkg_ComputeThermalFormula)(	struct PKG_FLIP_FLOP*,
						struct FLIP_FLOP* );

	void (*Pkg_ComputeVoltageFormula)( struct PKG_FLIP_FLOP* );

	void (*Pkg_ComputePowerFormula)( PROC_RW*, struct FLIP_FLOP* );

	void (*Pkg_ResetPowerFormula)(PROC_RW*);

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.thermalFormula)) {
	case THERMAL_KIND_NONE:
	default:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_None;
	}

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.voltageFormula)) {
	case VOLTAGE_KIND_OPP:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_OPP;
		break;
	case VOLTAGE_KIND_NONE:
	default:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_None;
		break;
	}

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.powerFormula)) {
	case POWER_KIND_NONE:
	default:
		Pkg_ComputePowerFormula = Pkg_ComputePower_None;
		Pkg_ResetPowerFormula	= Pkg_ResetPower_None;
		break;
	}

    #if defined(LEGACY) && LEGACY > 0
	#define ROOM_CLEAR(_room_)					\
		BITZERO(BUS_LOCK, _room_##Core[CORE_WORD_TOP(CORE_COUNT)])
    #else
	#define ROOM_CLEAR(_room_)					\
		BITCMP_CC(BUS_LOCK, _room_##Core, _room_##Clear)
    #endif

    #define ROOM_RESET(_room_)						\
	BITSTOR_CC(BUS_LOCK, _room_##Core, _room_##Seed)

    #define ROOM_READY(_room_)						\
	BITCMP_CC(BUS_LOCK, _room_##Ready, _room_##Seed)

    #define CONDITION_RDTSCP()						\
	(  (RO(Proc)->Features.Inv_TSC == 1)				\
	|| (RO(Proc)->Features.RDTSCP == 1) )

    #define CONDITION_RDPMC()						\
	(RO(Proc)->Features.PerfMon.Version >= 1)

	UBENCH_SETUP(CONDITION_RDTSCP(), CONDITION_RDPMC());
	Print_uBenchmark((Quiet & 0x100));

   while (!BITVAL(Shutdown, SYNC))
   {	/*	Loop while all the cpu room bits are not cleared.	*/
	while (!BITVAL(Shutdown, SYNC) && !ROOM_CLEAR(room))
	{
		nanosleep(&RO(Shm)->Sleep.pollingWait, NULL);
	}

	UBENCH_RDCOUNTER(1);

	RO(Shm)->Proc.Toggle = !RO(Shm)->Proc.Toggle;
	PFlip = &RO(Shm)->Proc.FlipFlop[RO(Shm)->Proc.Toggle];

	SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
		];

	PFlip->Thermal.Temp = 0;
	PFlip->Voltage.VID.CPU = 0;
	/*	Reset the averages & the max frequency			*/
	RO(Shm)->Proc.Avg.Turbo = 0;
	RO(Shm)->Proc.Avg.C0    = 0;
	RO(Shm)->Proc.Avg.C3    = 0;
	RO(Shm)->Proc.Avg.C6    = 0;
	RO(Shm)->Proc.Avg.C7    = 0;
	RO(Shm)->Proc.Avg.C1    = 0;

	prevTop.RelFreq = 0.0;
	prevTop.AbsFreq = 0.0;

	Pkg_ResetPowerFormula(RW(Proc));
	/*	Reset with Package Events				*/
	memcpy( RO(Shm)->ProcessorEvents, PFlip->Thermal.Events,
		sizeof(RO(Shm)->ProcessorEvents) );

    for (cpu=0; !BITVAL(Shutdown, SYNC)&&(cpu < RO(Shm)->Proc.CPU.Count);cpu++)
    {
      if (BITVAL(RO(Core, AT(cpu))->OffLine, OS) == 1)
      {
	if (Arg[cpu].TID)
	{	/*	Remove this cpu.				*/
	  if (pthread_join(Arg[cpu].TID, NULL) == 0)
	  {
		BITCLR_CC(BUS_LOCK, roomReady, cpu);
		Arg[cpu].TID = 0;

		PerCore_Update(RO(Shm), RO(Proc), RO(Core), cpu);

	    if (ServerFollowService(	&localService,
					&RO(Shm)->Proc.Service,
					tid ) == 0)
	    {
		SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
			];
	    }
		/*	Raise these bits up to notify a platform change. */
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
	  }
	}
	BITSET(LOCKLESS, RO(Shm)->Cpu[cpu].OffLine, OS);
      } else {
	struct FLIP_FLOP *CFlop = &RO(Shm)->Cpu[cpu].FlipFlop[
					!RO(Shm)->Cpu[cpu].Toggle
				];
	if (!Arg[cpu].TID)
	{	/*	Add this cpu.					*/
		Arg[cpu].Ref  = Ref;
		Arg[cpu].Bind = cpu;
	  if (pthread_create( &Arg[cpu].TID,
				NULL,
				Core_Cycle,
				&Arg[cpu]) == 0)
	  {
		BITSET_CC(BUS_LOCK, roomReady, cpu);

		PerCore_Update(RO(Shm), RO(Proc), RO(Core), cpu);

	    if (ServerFollowService(&localService,
					&RO(Shm)->Proc.Service,
					tid) == 0)
	    {
		SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
			];
	    }
	    if (Quiet & 0x100) {
		printf( "    CPU #%03u @ %.2f MHz\n", cpu,
		  ABS_FREQ_MHz(double , RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)].Q,
					RO(Core, AT(cpu))->Clock) );
	    }
		/*	Notify a CPU has been brought up		*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
	  }
	}
	BITCLR(LOCKLESS, RO(Shm)->Cpu[cpu].OffLine, OS);

	/*	Index CPU with the highest Rel. and Abs frequencies.	*/
	if (CFlop->Relative.Freq > prevTop.RelFreq) {
		prevTop.RelFreq = CFlop->Relative.Freq;
		RO(Shm)->Proc.Top.Rel = cpu;
	}
	if (CFlop->Absolute.Freq > prevTop.AbsFreq) {
		prevTop.AbsFreq = CFlop->Absolute.Freq;
		RO(Shm)->Proc.Top.Abs = cpu;
	}

	/*	Workaround to Package Thermal Management: the hottest Core */
	if (!RO(Shm)->Proc.Features.Power.PTM) {
		if (CFlop->Thermal.Temp > PFlip->Thermal.Temp)
			PFlip->Thermal.Temp = CFlop->Thermal.Temp;
	}
	/*	Workaround to a Package discrete voltage: the highest Vcore */
	if (!RO(Proc)->PowerThermal.VID.CPU) {
		if (CFlop->Voltage.VID > PFlip->Voltage.VID.CPU)
			PFlip->Voltage.VID.CPU = CFlop->Voltage.VID;
	}
	/*	Workaround to RAPL Package counter: sum of all Cores	*/
	Pkg_ComputePowerFormula(RW(Proc), CFlop);

	/*	Sum counters.						*/
	RO(Shm)->Proc.Avg.Turbo += CFlop->State.Turbo;
	RO(Shm)->Proc.Avg.C0    += CFlop->State.C0;
	RO(Shm)->Proc.Avg.C3    += CFlop->State.C3;
	RO(Shm)->Proc.Avg.C6    += CFlop->State.C6;
	RO(Shm)->Proc.Avg.C7    += CFlop->State.C7;
	RO(Shm)->Proc.Avg.C1    += CFlop->State.C1;
	/*	Aggregate all Cores Events.				*/
	RO(Shm)->ProcessorEvents[eLOG] |= CFlop->Thermal.Events[eLOG];
	RO(Shm)->ProcessorEvents[eSTS] |= CFlop->Thermal.Events[eSTS];
      }
    }
    if (!BITVAL(Shutdown, SYNC) && ROOM_READY(room))
    {
	unsigned char fRESET = 0;
	/*	Compute the counters averages.				*/
	RO(Shm)->Proc.Avg.Turbo /= RO(Shm)->Proc.CPU.OnLine;
	RO(Shm)->Proc.Avg.C0    /= RO(Shm)->Proc.CPU.OnLine;
	RO(Shm)->Proc.Avg.C3    /= RO(Shm)->Proc.CPU.OnLine;
	RO(Shm)->Proc.Avg.C6    /= RO(Shm)->Proc.CPU.OnLine;
	RO(Shm)->Proc.Avg.C7    /= RO(Shm)->Proc.CPU.OnLine;
	RO(Shm)->Proc.Avg.C1    /= RO(Shm)->Proc.CPU.OnLine;
	/*	Package scope counters					*/
	PFlip->Delta.PCLK = RO(Proc)->Delta.PCLK;
	PFlip->Delta.PC02 = RO(Proc)->Delta.PC02;
	PFlip->Delta.PC03 = RO(Proc)->Delta.PC03;
	PFlip->Delta.PC04 = RO(Proc)->Delta.PC04;
	PFlip->Delta.PC06 = RO(Proc)->Delta.PC06;
	PFlip->Delta.PC07 = RO(Proc)->Delta.PC07;
	PFlip->Delta.PC08 = RO(Proc)->Delta.PC08;
	PFlip->Delta.PC09 = RO(Proc)->Delta.PC09;
	PFlip->Delta.PC10 = RO(Proc)->Delta.PC10;
	PFlip->Delta.MC6  = RO(Proc)->Delta.MC6;
	/*	Package C-state Residency counters			*/
	RO(Shm)->Proc.State.PC02= (double)PFlip->Delta.PC02
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC03= (double)PFlip->Delta.PC03
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC04= (double)PFlip->Delta.PC04
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC06= (double)PFlip->Delta.PC06
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC07= (double)PFlip->Delta.PC07
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC08= (double)PFlip->Delta.PC08
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC09= (double)PFlip->Delta.PC09
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.PC10= (double)PFlip->Delta.PC10
				/ (double)PFlip->Delta.PCLK;

	RO(Shm)->Proc.State.MC6 = (double)PFlip->Delta.MC6
				/ (double)PFlip->Delta.PCLK;
	/*	Uncore scope counters				*/
	PFlip->Uncore.FC0 = RO(Proc)->Delta.Uncore.FC0;
	/*	Power & Energy counters 				*/
	enum PWR_DOMAIN pw;
      for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
      {
	PFlip->Delta.ACCU[pw] = RW(Proc)->Delta.Power.ACCU[pw];

	RO(Shm)->Proc.State.Energy[pw].Current = PFlip->Delta.ACCU[pw];
	RO(Shm)->Proc.State.Energy[pw].Current *= \
					RO(Shm)->Proc.Power.Unit.Joules;

	RO(Shm)->Proc.State.Power[pw].Current = \
				RO(Shm)->Proc.State.Energy[pw].Current;

	RO(Shm)->Proc.State.Power[pw].Current *= 1000.0;
	RO(Shm)->Proc.State.Power[pw].Current /= RO(Shm)->Sleep.Interval;

	/*	Processor scope: computes Min and Max energy consumed.	*/
	TEST_AND_SET_SENSOR(	ENERGY, LOWEST,
				RO(Shm)->Proc.State.Energy[pw].Current,
				RO(Shm)->Proc.State.Energy[pw].Limit );

	TEST_AND_SET_SENSOR(	ENERGY, HIGHEST,
				RO(Shm)->Proc.State.Energy[pw].Current,
				RO(Shm)->Proc.State.Energy[pw].Limit );
	/*	Processor scope: computes Min and Max power consumed. */
	TEST_AND_SET_SENSOR(	POWER, LOWEST,
				RO(Shm)->Proc.State.Power[pw].Current,
				RO(Shm)->Proc.State.Power[pw].Limit );

	TEST_AND_SET_SENSOR(	POWER, HIGHEST,
				RO(Shm)->Proc.State.Power[pw].Current,
				RO(Shm)->Proc.State.Power[pw].Limit );
      }
	/*	Package Processor & Plaftorm events			*/
	memcpy( PFlip->Thermal.Events, RO(Proc)->PowerThermal.Events,
		sizeof(PFlip->Thermal.Events) );
	/*	Package thermal formulas				*/
      if (RO(Shm)->Proc.Features.Power.PTM)
      {
	PFlip->Thermal.Sensor = RO(Proc)->PowerThermal.Sensor;
	Pkg_ComputeThermalFormula(PFlip, SProc);
      }
	/*	Package Voltage formulas				*/
      if (RO(Proc)->PowerThermal.VID.CPU)
      {
	PFlip->Voltage.VID.CPU = RO(Proc)->PowerThermal.VID.CPU;
      }
	PFlip->Voltage.VID.SOC = RO(Proc)->PowerThermal.VID.SOC;

	Pkg_ComputeVoltageFormula(PFlip);
	/*	Computes the Min Processor voltage.			*/
	TEST_AND_SET_SENSOR(	VOLTAGE, LOWEST, PFlip->Voltage.CPU,
				RO(Shm)->Proc.State.Voltage.Limit );
	/* Computes the Max Processor voltage.			*/
	TEST_AND_SET_SENSOR(	VOLTAGE, HIGHEST, PFlip->Voltage.CPU,
				RO(Shm)->Proc.State.Voltage.Limit );
	/*
	The Driver tick is bound to the Service Core:
	1- Tasks collection; Tasks count; and Memory usage.
	2- Processor has resumed from Suspend To RAM.
	*/
	RO(Shm)->SysGate.tickStep = RO(Proc)->tickStep;
     if (RO(Shm)->SysGate.tickStep == RO(Shm)->SysGate.tickReset)
     {
	if (BITWISEAND(LOCKLESS, RO(Shm)->SysGate.Operation, 0x1))
	{
		if (SysGate_OnDemand(Ref, 1) == 0) {
			SysGate_Update(Ref);
		}
	}
	/*	OS notifications: such as Resumed from Suspend		*/
	if ( (fRESET = BITCLR(BUS_LOCK, RW(Proc)->OS.Signal, NTFY )) == 1)
	{
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_COMP|BIT_MASK_NTFY);
	}
     }
     if (BITWISEAND(LOCKLESS, PendingSync, BIT_MASK_COMP|BIT_MASK_NTFY))
     {
	Package_Update(RO(Shm), RO(Proc), RW(Proc));

	Uncore_Update(RO(Shm), RO(Proc), RO(Core, AT(RO(Proc)->Service.Core)));

      for (cpu = 0; cpu < Ref->RO(Shm)->Proc.CPU.Count; cpu++)
      {
	if (fRESET == 1)
	{
		Core_ResetSensorLimits(&RO(Shm)->Cpu[cpu]);
	}
	if (BITVAL(RO(Ref->Core, AT(cpu))->OffLine, OS) == 0)
	{
		PerCore_Update(Ref->RO(Shm), RO(Ref->Proc), RO(Ref->Core), cpu);
	}
      }
	if (fRESET == 1)
	{
		Pkg_ResetSensorLimits(&RO(Shm)->Proc);
	}
	Technology_Update(RO(Shm), RO(Proc), RW(Proc));

	if (ServerFollowService(&localService,
				&RO(Shm)->Proc.Service,
				tid) == 0)
	{
		SProc = &RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].FlipFlop[
				!RO(Shm)->Cpu[RO(Shm)->Proc.Service.Core].Toggle
			];
	}
	if (Quiet & 0x100) {
		printf("\t%s || %s\n",
			BITVAL(PendingSync, NTFY0) ? "NTFY":"....",
			BITVAL(PendingSync, COMP0) ? "COMP":"....");
	}
      }
	/*	All aggregations done: Notify Clients.		*/
	BITWISESET(LOCKLESS, PendingSync, BIT_MASK_SYNC);
	BITWISESET(LOCKLESS, RW(Shm)->Proc.Sync, PendingSync);
	BITWISECLR(LOCKLESS, PendingSync);
    }
	/*	Reset the Room mask					*/
	ROOM_RESET(room);

	UBENCH_RDCOUNTER(2);

	UBENCH_COMPUTE();
	Print_uBenchmark((Quiet & 0x100));
   }
	#undef CONDITION_RDPMC
	#undef CONDITION_RDTSCP
	#undef ROOM_RESET
	#undef ROOM_READY
	#undef ROOM_CLEAR
	for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++) {
		if (Arg[cpu].TID) {
			pthread_join(Arg[cpu].TID, NULL);
		}
	}
	free(Arg);
  } else {
	REASON_SET(reason, RC_MEM_ERR, (errno == 0 ? ENOMEM : errno));
  }
	return reason;
}

REASON_CODE Child_Manager(REF *Ref)
{
	RO(SHM_STRUCT) *RO(Shm) = Ref->RO(Shm);
	SERVICE_PROC localService = RESET_SERVICE;
	REASON_INIT(reason);
	unsigned int cpu = 0;

	pthread_t tid = pthread_self();

  if (ServerFollowService(&localService, &RO(Shm)->Proc.Service, tid) == 0) {
	pthread_setname_np(tid, "corefreqd-cmgr");
  }
	ARG *Arg = calloc(RO(Shm)->Proc.CPU.Count, sizeof(ARG));
  if (Arg != NULL)
  {
   do {
    for(cpu=0; !BITVAL(Shutdown, SYNC) && (cpu < RO(Shm)->Proc.CPU.Count);cpu++)
    {
	if (BITVAL(RO(Shm)->Cpu[cpu].OffLine, OS) == 1) {
		if (Arg[cpu].TID) {
			/*	Remove this child thread.		*/
			pthread_join(Arg[cpu].TID, NULL);
			Arg[cpu].TID = 0;
		}
	} else {
		unsigned int seed32;
		__asm__ volatile
		(
			"isb"			"\n\t"
			"mrs %0, cntvct_el0"
			: "=r" (seed32)
			:
			:
		);
	    #ifdef __GLIBC__
		initstate_r(	seed32,
				RO(Shm)->Cpu[cpu].Slice.Random.state,
				sizeof(RO(Shm)->Cpu[cpu].Slice.Random.state),
				&RO(Shm)->Cpu[cpu].Slice.Random.data );
	    #else
		initstate(seed32, RO(Shm)->Cpu[cpu].Slice.Random.state, 128);
	    #endif
		if (!Arg[cpu].TID) {
			/*	Add this child thread.			*/
			Arg[cpu].Ref  = Ref;
			Arg[cpu].Bind = cpu;
			pthread_create( &Arg[cpu].TID,
					NULL,
					Child_Thread,
					&Arg[cpu]);
		}
	}
    }
	ServerFollowService(&localService, &RO(Shm)->Proc.Service, tid);

	nanosleep(&RO(Shm)->Sleep.childWaiting, NULL);
   }
   while (!BITVAL(Shutdown, SYNC)) ;

	for (cpu = 0; cpu < RO(Shm)->Proc.CPU.Count; cpu++) {
		if (Arg[cpu].TID) {
			pthread_join(Arg[cpu].TID, NULL);
		}
	}
	free(Arg);
  } else {
	REASON_SET(reason, RC_MEM_ERR, (errno == 0 ? ENOMEM : errno));
  }
	return reason;
}

REASON_CODE Shm_Manager(FD *fd, RO(PROC) *RO(Proc), RW(PROC) *RW(Proc),
			uid_t uid, uid_t gid, mode_t cmask[2])
{
	unsigned int	cpu = 0;
	RO(CORE)	**RO(Core);
	RW(CORE)	**RW(Core);
	RO(SHM_STRUCT)	*RO(Shm) = NULL;
	RW(SHM_STRUCT)	*RW(Shm) = NULL;
	struct {
		const size_t	RO(Core),
				RW(Core);
	} Size = {
		ROUND_TO_PAGES(sizeof(RO(CORE))),
		ROUND_TO_PAGES(sizeof(RW(CORE)))
	};
	REASON_INIT(reason);

    if ((RO(Core) = calloc(RO(Proc)->CPU.Count, sizeof(RO(Core)))) == NULL) {
	REASON_SET(reason, RC_MEM_ERR, (errno == 0 ? ENOMEM : errno));
    }
    if ((RW(Core) = calloc(RO(Proc)->CPU.Count, sizeof(RW(Core)))) == NULL) {
	REASON_SET(reason, RC_MEM_ERR, (errno == 0 ? ENOMEM : errno));
    }
    for(cpu = 0;(reason.rc == RC_SUCCESS) && (cpu < RO(Proc)->CPU.Count); cpu++)
    {
	const off_t	vm_ro_pgoff = (ID_RO_VMA_CORE + cpu) * PAGE_SIZE,
			vm_rw_pgoff = (ID_RW_VMA_CORE + cpu) * PAGE_SIZE;

	if ((RO(Core, AT(cpu)) = mmap(NULL, Size.RO(Core),
				PROT_READ,
				MAP_SHARED,
				fd->drv, vm_ro_pgoff)) == MAP_FAILED)
	{
		REASON_SET(reason, RC_SHM_MMAP);
	}
	if ((RW(Core, AT(cpu)) = mmap(NULL, Size.RW(Core),
				PROT_READ|PROT_WRITE,
				MAP_SHARED,
				fd->drv, vm_rw_pgoff)) == MAP_FAILED)
	{
		REASON_SET(reason, RC_SHM_MMAP);
	}
    }
    if (reason.rc == RC_SUCCESS) {
	if (gid != 0) {
		if (setregid((gid_t) -1, gid) != 0) {
			REASON_SET(reason, RC_SYS_CALL);
		}
	}
    }
    if (reason.rc == RC_SUCCESS) {
	if (uid != 0) {
		if (setreuid((uid_t) -1, uid) != 0) {
			REASON_SET(reason, RC_SYS_CALL);
		}
	}
    }
    if (reason.rc == RC_SUCCESS)
    {	/* Initialize shared memory.					*/
	const size_t shmCpuSize = sizeof(CPU_STRUCT) * RO(Proc)->CPU.Count,
			roSize = ROUND_TO_PAGES((sizeof(RO(SHM_STRUCT))
				+ shmCpuSize)),
			rwSize = ROUND_TO_PAGES(sizeof(RW(SHM_STRUCT)));

      if ( ((fd->ro = shm_open(RO(SHM_FILENAME), O_CREAT|O_TRUNC|O_RDWR,
					 S_IRUSR|S_IWUSR
					|S_IRGRP|S_IWGRP
					|S_IROTH|S_IWOTH)) != -1)
	&& ((fd->rw = shm_open(RW(SHM_FILENAME), O_CREAT|O_TRUNC|O_RDWR,
					 S_IRUSR|S_IWUSR
					|S_IRGRP|S_IWGRP
					|S_IROTH|S_IWOTH)) != -1) )
      {
	pid_t CPID = -1;

	if ( (ftruncate(fd->ro, (off_t) roSize) != -1)
	  && (ftruncate(fd->rw, (off_t) rwSize) != -1) )
	{
		fchmod(fd->ro, cmask[0]);
		fchmod(fd->rw, cmask[1]);

	    if ( ( ( RO(Shm) = mmap(NULL, roSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd->ro, 0) ) != MAP_FAILED )
	      && ( ( RW(Shm) = mmap(NULL, rwSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd->rw, 0) ) != MAP_FAILED ) )
	    {
		__typeof__ (errno) fork_err = 0;
		/*	Clear SHM					*/
		memset(RO(Shm), 0, roSize);
		memset(RW(Shm), 0, rwSize);
		/*	Store version footprint into SHM		*/
		SET_FOOTPRINT(RO(Shm)->FootPrint,MAX_FREQ_HZ,
						CORE_COUNT,
						TASK_ORDER,
						COREFREQ_MAJOR,
						COREFREQ_MINOR,
						COREFREQ_REV	);
		/*	Reference time the Server is starting at.	*/
		time(&RO(Shm)->StartedAt);
		/*	Store the daemon gate name.			*/
		StrCopy(RO(Shm)->ShmName, RO(SHM_FILENAME), TASK_COMM_LEN);
		/*	Initialize the busy wait times.			*/
		RO(Shm)->Sleep.ringWaiting  = TIMESPEC(SIG_RING_MS);
		RO(Shm)->Sleep.childWaiting = TIMESPEC(CHILD_PS_MS);
		RO(Shm)->Sleep.sliceWaiting = TIMESPEC(CHILD_TH_MS);

		REF Ref = {
			.CPID		= -1,
			.KID		= 0,
			.Started	= 0,
			.Slice.Func	= NULL,
			.Slice.arg	= 0,
			.fd		= fd,
			.RO(Shm)	= RO(Shm),
			.RW(Shm)	= RW(Shm),
			.RO(Proc)	= RO(Proc),
			.RW(Proc)	= RW(Proc),
			.RO(Core)	= RO(Core),
			.RW(Core)	= RW(Core),
			.RO(SysGate)	= NULL
		};
		sigemptyset(&Ref.Signal);

		Package_Update(RO(Shm), RO(Proc), RW(Proc));

		memcpy(&RO(Shm)->SMB, &RO(Proc)->SMB, sizeof(SMBIOS_ST));

		/*	Clear notification.				*/
		BITWISECLR(LOCKLESS, RW(Shm)->Proc.Sync);

		SysGate_Toggle(&Ref, SysGateStartUp);

		/*	Welcomes with brand and per CPU base clock.	*/
	      if (Quiet & 0x001) {
		printf( "CoreFreq Daemon %s"				\
			"  Copyright (C) 2015-2025 CYRIL COURTIAT\n",
			COREFREQ_VERSION );
	      }
	      if (Quiet & 0x010) {
		printf( "\n"						\
			"  Processor [%s]\n"				\
			"  Architecture [%s] %u/%u CPU Online.\n",
			RO(Shm)->Proc.Brand,
			RO(Shm)->Proc.Architecture,
			RO(Shm)->Proc.CPU.OnLine,
			RO(Shm)->Proc.CPU.Count );
	      }
	      if (Quiet & 0x100) {
		printf( "  SleepInterval(%u), SysGate(%u), %ld tasks\n\n",
			RO(Shm)->Sleep.Interval,
			!BITVAL(RO(Shm)->SysGate.Operation, 0) ?
			0:RO(Shm)->Sleep.Interval * RO(Shm)->SysGate.tickReset,
			TASK_LIMIT );
		}
	      if (Quiet) {
		fflush(stdout);
	      }
		CPID = Ref.CPID = fork();
	/*-----[ Resources inherited ]----------------------------------*/
		fork_err = errno;

		Emergency_Command(&Ref, 1);

		switch (Ref.CPID) {
		case 0:
			reason = Child_Manager(&Ref);
			break;
		case -1:
			REASON_SET(reason, RC_EXEC_ERR, fork_err);
			break;
		default:
			reason = Core_Manager(&Ref);

			if (gid != 0) {
				if (setregid((gid_t) -1, 0) != 0) {
					REASON_SET(reason, RC_SYS_CALL);
				}
			}
			if (uid != 0) {
				if (setreuid((uid_t) -1, 0) != 0) {
					REASON_SET(reason, RC_SYS_CALL);
				}
			}
			if (RO(Shm)->App.Cli) {
				if (kill(RO(Shm)->App.Cli, SIGTERM) == -1) {
				    if (errno != ESRCH)
					REASON_SET(reason, RC_EXEC_ERR);
				}
			}
			if (RO(Shm)->App.GUI) {
				if (kill(RO(Shm)->App.GUI, SIGTERM) == -1) {
				    if (errno != ESRCH)
					REASON_SET(reason, RC_EXEC_ERR);
				}
			}
			SysGate_OnDemand(&Ref, 0);

			if (kill(Ref.CPID, SIGQUIT) == 0) {
				if (waitpid(Ref.CPID, NULL, 0) == -1) {
					REASON_SET(reason, RC_EXEC_ERR);
				}
			} else {
				REASON_SET(reason, RC_EXEC_ERR);
			}
			break;
		}
		Emergency_Command(&Ref, 0);

		if (munmap(RO(Shm), roSize) == -1) {
			REASON_SET(reason, RC_SHM_MMAP);
		}
		if (munmap(RW(Shm), rwSize) == -1) {
			REASON_SET(reason, RC_SHM_MMAP);
		}
	    } else {
		REASON_SET(reason, RC_SHM_MMAP);
	    }
	} else {
		REASON_SET(reason, RC_SHM_FILE);
	}
	if (close(fd->ro) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
	}
	if (close(fd->rw) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
	}
	if (CPID != 0) {
	    if (shm_unlink(RO(SHM_FILENAME)) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
	    }
	    if (shm_unlink(RW(SHM_FILENAME)) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
	    }
	}
      } else {
		REASON_SET(reason, RC_SHM_FILE);
      }
    }
    for (cpu = 0; cpu < RO(Proc)->CPU.Count; cpu++)
    {
	if (RO(Core, AT(cpu)) != NULL) {
	    if (munmap(RO(Core, AT(cpu)), Size.RO(Core)) == -1) {
		REASON_SET(reason, RC_SHM_MMAP);
	    }
	}
	if (RW(Core, AT(cpu)) != NULL) {
	    if (munmap(RW(Core, AT(cpu)), Size.RW(Core)) == -1) {
		REASON_SET(reason, RC_SHM_MMAP);
	    }
	}
    }
    if (RO(Core) != NULL) {
	free(RO(Core));
    }
    if (RW(Core) != NULL) {
	free(RW(Core));
    }
	return reason;
}

REASON_CODE Help(REASON_CODE reason, ...)
{
	va_list ap;
	va_start(ap, reason);
	switch (reason.rc) {
	case RC_SUCCESS:
	case RC_OK_SYSGATE:
	case RC_OK_COMPUTE:
	case RC_DRIVER_BASE ... RC_DRIVER_LAST:
		break;
	case RC_CMD_SYNTAX: {
		char *appName = va_arg(ap, char *);
		printf( "Usage:\t%s [-option <arguments>]\n"		\
			"\t-q\t\tQuiet\n"				\
			"\t-i\t\tInfo\n"				\
			"\t-d\t\tDebug\n"				\
			"\t-gon\t\tEnable SysGate\n"			\
			"\t-goff\t\tDisable SysGate\n"			\
			"\t-U <decimal>\tSet the effective user ID\n"	\
			"\t-G <decimal>\tSet the effective group ID\n"	\
			"\t-M <oct>,<oct>\tShared Memories permission\n"\
			"\t-h\t\tPrint out this message\n"		\
			"\t-v\t\tPrint the version number\n"		\
			"\nExit status:\n"				\
			"\t%u\tSUCCESS\t\tSuccessful execution\n"	\
			"\t%u\tCMD_SYNTAX\tCommand syntax error\n"	\
			"\t%u\tSHM_FILE\tShared memory file error\n"	\
			"\t%u\tSHM_MMAP\tShared memory mapping error\n"	\
			"\t%u\tPERM_ERR\tExecution not permitted\n"	\
			"\t%u\tMEM_ERR\t\tMemory operation error\n"	\
			"\t%u\tEXEC_ERR\tGeneral execution error\n"	\
			"\t%u\tSYS_CALL\tSystem call error\n"		\
			"\nReport bugs to labs[at]cyring.fr\n", appName,
			RC_SUCCESS,
			RC_CMD_SYNTAX,
			RC_SHM_FILE,
			RC_SHM_MMAP,
			RC_PERM_ERR,
			RC_MEM_ERR,
			RC_EXEC_ERR,
			RC_SYS_CALL);
		}
		break;
	case RC_PERM_ERR:
	case RC_MEM_ERR:
	case RC_EXEC_ERR: {
		char *appName = va_arg(ap, char *);
		char *sysMsg = strerror(reason.no);
		fprintf(stderr, "%s execution error code %d\n%s @ line %d\n",
				appName, reason.no, sysMsg, reason.ln);
		}
		break;
	case RC_SHM_FILE:
	case RC_SHM_MMAP: {
		char *shmFileName = va_arg(ap, char *);
		char *sysMsg = strerror(reason.no);
		fprintf(stderr , "Driver connection error code %d\n"	\
				"%s: '%s' @ line %d\n",
				reason.no, shmFileName, sysMsg, reason.ln);
		}
		break;
	case RC_SYS_CALL: {
		char *sysMsg = strerror(reason.no);
		fprintf(stderr, "System error code %d\n%s @ line %d\n",
				reason.no, sysMsg, reason.ln);
		}
		break;
	}
	va_end(ap);
	return reason;
}

int main(int argc, char *argv[])
{
	FD fd = {0, 0, 0};
	RO(PROC) *RO(Proc) = NULL;	/* Kernel module anchor points. */
	RW(PROC) *RW(Proc) = NULL;
	uid_t uid = 0, gid = 0;
	mode_t cmask[2] = {
		S_IRUSR|S_IRGRP|S_IROTH,
		S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
	};
	char *program = strdup(argv[0]),
		*appName = program != NULL ? basename(program) : argv[0];

	REASON_INIT(reason);
	int  i;
    for (i = 1; i < argc; i++)
    {
	if (strlen(argv[i]) > 1) {
		if (argv[i][0] == '-')
		{
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
					 && argv[i][4]=='f') {
						SysGateStartUp = 0;
				} else if ( argv[i][2]=='o'
					 && argv[i][3]=='n') {
						SysGateStartUp = 1;
				} else {
					REASON_SET(reason, RC_CMD_SYNTAX, 0);
					reason = Help(reason, appName);
				}
				break;
			case 'v':
				printf("%s\n", COREFREQ_VERSION);
				reason.rc = RC_CMD_SYNTAX;
				break;
			case 'U': {
				char trailing =  '\0';
			    if (argv[++i] == NULL) {
					REASON_SET(reason, RC_CMD_SYNTAX, 0);
					reason = Help(reason, appName);
			    } else if (sscanf(argv[i], "%d%c",
							&uid,
							&trailing) != 1) {
					REASON_SET(reason, RC_CMD_SYNTAX);
					reason = Help(reason, appName);
			    }
			}
				break;
			case 'G': {
				char trailing =  '\0';
			    if (argv[++i] == NULL) {
					REASON_SET(reason, RC_CMD_SYNTAX, 0);
					reason = Help(reason, appName);
			    } else if (sscanf(argv[i], "%d%c",
							&gid,
							&trailing) != 1) {
					REASON_SET(reason, RC_CMD_SYNTAX);
					reason = Help(reason, appName);
			    }
			}
				break;
			case 'M': {
				char trailing =  '\0';
			    if (argv[++i] == NULL) {
				REASON_SET(reason, RC_CMD_SYNTAX, 0);
				reason = Help(reason, appName);
			    } else if (sscanf(argv[i] , "%o,%o%c",
							&cmask[0],
							&cmask[1],
							&trailing) > 2) {
					REASON_SET(reason, RC_CMD_SYNTAX);
					reason = Help(reason, appName);
			    }
			}
				break;
			case 'h':
			default: {
				REASON_SET(reason, RC_CMD_SYNTAX, 0);
				reason = Help(reason, appName);
				}
				break;
			}
		} else {
			REASON_SET(reason, RC_CMD_SYNTAX, 0);
			reason = Help(reason, appName);
		}
	} else {
		REASON_SET(reason, RC_CMD_SYNTAX, 0);
		reason = Help(reason, appName);
	}
    }
    if (reason.rc == RC_SUCCESS)
    {
	if (geteuid() == 0)
	{
	    if ((fd.drv = open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
	    {
		const size_t packageSize[] = {
			ROUND_TO_PAGES(sizeof(RO(PROC))),
			ROUND_TO_PAGES(sizeof(RW(PROC)))
		};
		const off_t vm_pgoff[] = {
			ID_RO_VMA_PROC * PAGE_SIZE,
			ID_RW_VMA_PROC * PAGE_SIZE
		};
		if ((RO(Proc) = mmap(NULL, packageSize[0],
				PROT_READ, MAP_SHARED,
				fd.drv, vm_pgoff[0])) != MAP_FAILED)
		{
		  if ((RW(Proc) = mmap(NULL, packageSize[1],
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.drv, vm_pgoff[1])) != MAP_FAILED)
		  {
		    if (CHK_FOOTPRINT(RO(Proc)->FootPrint,MAX_FREQ_HZ,
							CORE_COUNT,
							TASK_ORDER,
							COREFREQ_MAJOR,
							COREFREQ_MINOR,
							COREFREQ_REV))
		    {
			reason = Shm_Manager(	&fd, RO(Proc), RW(Proc),
						uid, gid, cmask );

			switch (reason.rc) {
			case RC_SUCCESS:
			case RC_OK_SYSGATE:
			case RC_OK_COMPUTE:
			case RC_DRIVER_BASE ... RC_DRIVER_LAST:
				break;
			case RC_CMD_SYNTAX:
			case RC_PERM_ERR:
				break;
			case RC_SHM_FILE:
			case RC_SHM_MMAP:
				reason = Help(reason, RO(SHM_FILENAME));
				break;
			case RC_MEM_ERR:
			case RC_EXEC_ERR:
				reason = Help(reason, appName);
				break;
			case RC_SYS_CALL:
				reason = Help(reason);
				break;
			}
		    } else {
			char *wrongVersion = malloc(10+5+5+5+1);
			REASON_SET(reason, RC_SHM_MMAP, EACCES);
			if (wrongVersion != NULL) {
				snprintf(wrongVersion, 10+5+5+5+1,
					"Version %hu.%hu.%hu",
					RO(Proc)->FootPrint.major,
					RO(Proc)->FootPrint.minor,
					RO(Proc)->FootPrint.rev);
				reason = Help(reason, wrongVersion);
				free(wrongVersion);
			}
		    }
		    if (munmap(RW(Proc), packageSize[1]) == -1) {
			REASON_SET(reason, RC_SHM_MMAP);
			reason = Help(reason, DRV_FILENAME);
		    }
		  }
		  if (munmap(RO(Proc), packageSize[0]) == -1) {
			REASON_SET(reason, RC_SHM_MMAP);
			reason = Help(reason, DRV_FILENAME);
		  }
		} else {
			REASON_SET(reason, RC_SHM_MMAP);
			reason = Help(reason, DRV_FILENAME);
		}
		if (close(fd.drv) == -1) {
			REASON_SET(reason, RC_SHM_FILE);
			reason = Help(reason, DRV_FILENAME);
		}
	    } else {
		REASON_SET(reason, RC_SHM_FILE);
		reason = Help(reason, DRV_FILENAME);
	    }
	} else {
		REASON_SET(reason, RC_PERM_ERR, EACCES);
		reason = Help(reason, appName);
	}
    }
    if (program != NULL) {
	free(program);
    }
	return reason.rc;
}
