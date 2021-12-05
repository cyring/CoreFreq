/*
 * CoreFreq
 * Copyright (C) 2015-2021 CYRIL INGENIERIE
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
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

#include "bitasm.h"
#include "amdmsr.h"
#include "intelmsr.h"
#include "coretypes.h"
#include "corefreq.h"
#include "corefreqm.h"
#include "corefreq-api.h"

#define AT( _loc_ )		[ _loc_ ]
#define OF( _ptr_ , ...)	_ptr_ __VA_ARGS__
#define RO( _ptr_ , ...)	OF( _ptr_##_RO , __VA_ARGS__ )
#define RW( _ptr_ , ...)	OF( _ptr_##_RW , __VA_ARGS__ )

#define PAGE_SIZE							\
(									\
	sysconf(_SC_PAGESIZE) > 0 ? sysconf(_SC_PAGESIZE) : 4096	\
)

/* §8.10.6.7 Place Locks and Semaphores in Aligned, 128-Byte Blocks of Memory */
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
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	UNUSED(Shm);
	UNUSED(cpu);
	CFlip->Thermal.Temp = 0;
}

#define ComputeThermal_None_PerSMT	ComputeThermal_None
#define ComputeThermal_None_PerCore	ComputeThermal_None
#define ComputeThermal_None_PerPkg	ComputeThermal_None

static void (*ComputeThermal_None_Matrix[4])( struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_None_PerPkg
};

static void ComputeThermal_Intel( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_THERMAL(INTEL,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeThermal_Intel_PerSMT	ComputeThermal_Intel

static void ComputeThermal_Intel_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_Intel(CFlip, Shm, cpu);
	}
}

static void ComputeThermal_Intel_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_Intel(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_Intel_Matrix[4])( struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_Intel_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_Intel_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_Intel_PerPkg
};

static void ComputeThermal_AMD( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeThermal_AMD_PerSMT	ComputeThermal_AMD

static void ComputeThermal_AMD_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD(CFlip, Shm, cpu);
	}
}

static void ComputeThermal_AMD_PerPkg(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_AMD(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_AMD_Matrix[4])( struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_PerPkg
};

static void ComputeThermal_AMD_0Fh( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD_0Fh,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeThermal_AMD_0Fh_PerSMT	ComputeThermal_AMD_0Fh

static void ComputeThermal_AMD_0Fh_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static void ComputeThermal_AMD_0Fh_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_AMD_0Fh_Matrix[4])( struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_0Fh_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_0Fh_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_0Fh_PerPkg
};

static void ComputeThermal_AMD_15h( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
    if (Shm->Cpu[cpu].Topology.CoreID == 0)
    {
	COMPUTE_THERMAL(AMD_15h,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip);
    }
}

#define ComputeThermal_AMD_15h_PerSMT	ComputeThermal_AMD_15h

static void ComputeThermal_AMD_15h_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (Shm->Cpu[cpu].Topology.CoreID == 0) /* Opteron test case	*/
	{
		ComputeThermal_AMD_15h(CFlip, Shm, cpu);
	}
}

static void ComputeThermal_AMD_15h_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_AMD_15h(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_AMD_15h_Matrix[4])(struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_15h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_15h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_15h_PerPkg
};

static void ComputeThermal_AMD_17h( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD_17h,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeThermal_AMD_17h_PerSMT	ComputeThermal_AMD_17h

static void ComputeThermal_AMD_17h_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD_17h(CFlip, Shm, cpu);
	}
}

static void ComputeThermal_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_AMD_17h(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_AMD_17h_Matrix[4])(struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_17h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_17h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_17h_PerPkg
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
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	UNUSED(CFlip);
	UNUSED(Shm);
	UNUSED(cpu);
}

#define ComputeVoltage_None_PerSMT	ComputeVoltage_None
#define ComputeVoltage_None_PerCore	ComputeVoltage_None
#define ComputeVoltage_None_PerPkg	ComputeVoltage_None

static void (*ComputeVoltage_None_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_None_PerPkg
};

#define ComputeVoltage_Intel_Matrix	ComputeVoltage_None_Matrix

static void ComputeVoltage_Intel_Core2( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* Intel Core 2 Extreme Datasheet §3.3-Table 2			*/
	COMPUTE_VOLTAGE(INTEL_CORE2,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_Core2_PerSMT	ComputeVoltage_Intel_Core2

static void ComputeVoltage_Intel_Core2_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_Core2(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_Intel_Core2_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_Core2(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_Core2_Matrix[4])( struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_Core2_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_Core2_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_Core2_PerPkg
};

static void ComputeVoltage_Intel_SoC( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* Intel Valleyview-D/M SoC					*/
	COMPUTE_VOLTAGE(INTEL_SOC,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_SoC_PerSMT	ComputeVoltage_Intel_SoC

static void ComputeVoltage_Intel_SoC_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SoC(CFlip, Shm, cpu);
	}
}

static  void ComputeVoltage_Intel_SoC_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SoC(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_SoC_Matrix[4])( struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SoC_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SoC_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SoC_PerPkg
};

static void ComputeVoltage_Intel_SNB( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(INTEL_SNB,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_SNB_PerSMT 	ComputeVoltage_Intel_SNB

static void ComputeVoltage_Intel_SNB_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SNB(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_Intel_SNB_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SNB(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_SNB_Matrix[4])( struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SNB_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SNB_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SNB_PerPkg
};

static void ComputeVoltage_Intel_SKL_X( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(INTEL_SKL_X,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_SKL_X_PerSMT	ComputeVoltage_Intel_SKL_X

static void ComputeVoltage_Intel_SKL_X_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SKL_X(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_Intel_SKL_X_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SKL_X(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_SKL_X_Matrix[4])( struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SKL_X_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SKL_X_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SKL_X_PerPkg
};

static void ComputeVoltage_AMD( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_PerSMT	ComputeVoltage_AMD

static void ComputeVoltage_AMD_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_AMD_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_AMD(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_AMD_Matrix[4])( struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_PerPkg
};

static void ComputeVoltage_AMD_0Fh( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* AMD BKDG Family 0Fh §10.6 Table 70				*/
	COMPUTE_VOLTAGE(AMD_0Fh,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_0Fh_PerSMT	ComputeVoltage_AMD_0Fh

static void ComputeVoltage_AMD_0Fh_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_AMD_0Fh_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_AMD_0Fh_Matrix[4])(struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_0Fh_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_0Fh_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_0Fh_PerPkg
};

static void ComputeVoltage_AMD_15h( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD_15h,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_15h_PerSMT	ComputeVoltage_AMD_15h

static void ComputeVoltage_AMD_15h_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_15h(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_AMD_15h_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_AMD_15h(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_AMD_15h_Matrix[4])(struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_15h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_15h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_15h_PerPkg
};

static void ComputeVoltage_AMD_17h( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD_17h,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_17h_PerSMT	ComputeVoltage_AMD_17h

static void ComputeVoltage_AMD_17h_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_17h(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_AMD_17h(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_AMD_17h_Matrix[4])(struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_17h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_17h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_17h_PerPkg
};

static void ComputeVoltage_Winbond_IO( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* Winbond W83627EHF/EF, W83627EHG,EG				*/
	COMPUTE_VOLTAGE(WINBOND_IO,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Winbond_IO_PerSMT	ComputeVoltage_Winbond_IO

static void ComputeVoltage_Winbond_IO_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Winbond_IO(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_Winbond_IO_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Winbond_IO(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Winbond_IO_Matrix[4])( struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Winbond_IO_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Winbond_IO_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Winbond_IO_PerPkg
};

static void ComputeVoltage_ITE_Tech_IO( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* ITE Tech IT8720F						*/
	COMPUTE_VOLTAGE(ITETECH_IO,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputeVoltage_ITE_Tech_IO_PerSMT	ComputeVoltage_ITE_Tech_IO

static void ComputeVoltage_ITE_Tech_IO_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_ITE_Tech_IO(CFlip, Shm, cpu);
	}
}

static void ComputeVoltage_ITE_Tech_IO_PerPkg( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_ITE_Tech_IO(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_ITE_Tech_IO_Matrix[4])(	struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_ITE_Tech_IO_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_ITE_Tech_IO_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_ITE_Tech_IO_PerPkg
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
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	UNUSED(CFlip);
	UNUSED(Shm);
	UNUSED(cpu);
}

#define ComputePower_None_PerSMT	ComputePower_None
#define ComputePower_None_PerCore	ComputePower_None
#define ComputePower_None_PerPkg	ComputePower_None

static void (*ComputePower_None_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputePower_None,
	[FORMULA_SCOPE_SMT ] = ComputePower_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputePower_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputePower_None_PerPkg
};

#define ComputePower_Intel_Matrix	ComputePower_None_Matrix

#define ComputePower_Intel_Atom_Matrix	ComputePower_None_Matrix

#define ComputePower_AMD_Matrix 	ComputePower_None_Matrix

static void ComputePower_AMD_17h( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu)
{
	CFlip->State.Energy	= (double)CFlip->Delta.Power.ACCU
				* Shm->Proc.Power.Unit.Joules;

	CFlip->State.Power	= 1000.0 * CFlip->State.Energy;
	CFlip->State.Power	/= Shm->Sleep.Interval;

	Core_ComputePowerLimits(&Shm->Cpu[cpu], CFlip);
}

#define ComputePower_AMD_17h_PerSMT	ComputePower_AMD_17h

static void ComputePower_AMD_17h_PerCore( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputePower_AMD_17h(CFlip, Shm, cpu);
	}
}

static void ComputePower_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputePower_AMD_17h(CFlip, Shm, cpu);
	}
}

static void (*ComputePower_AMD_17h_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputePower_None,
	[FORMULA_SCOPE_SMT ] = ComputePower_AMD_17h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputePower_AMD_17h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputePower_AMD_17h_PerPkg
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
	SHM_STRUCT	*Shm		= Arg->Ref->Shm;
	RO(CORE)	*RO(Core)	= Arg->Ref->RO(Core, AT(cpu));
	RW(CORE)	*RW(Core)	= Arg->Ref->RW(Core, AT(cpu));
	CPU_STRUCT	*Cpu		= &Shm->Cpu[cpu];

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
					SHM_STRUCT*,
					unsigned int );

	void (**ComputeVoltageFormula)( struct FLIP_FLOP*,
					SHM_STRUCT*,
					unsigned int );

	void (**ComputePowerFormula)(	struct FLIP_FLOP*,
					SHM_STRUCT*,
					unsigned int );

	switch (KIND_OF_FORMULA(Shm->Proc.thermalFormula)) {
	case THERMAL_KIND_INTEL:
		ComputeThermalFormula = ComputeThermal_Intel_Matrix;
		break;
	case THERMAL_KIND_AMD:
		ComputeThermalFormula = ComputeThermal_AMD_Matrix;
		break;
	case THERMAL_KIND_AMD_0Fh:
		ComputeThermalFormula = ComputeThermal_AMD_0Fh_Matrix;
		break;
	case THERMAL_KIND_AMD_15h:
		ComputeThermalFormula = ComputeThermal_AMD_15h_Matrix;
		break;
	case THERMAL_KIND_AMD_17h:
		ComputeThermalFormula = ComputeThermal_AMD_17h_Matrix;
		break;
	case THERMAL_KIND_NONE:
	default:
		ComputeThermalFormula = ComputeThermal_None_Matrix;
		break;
	}

	switch (KIND_OF_FORMULA(Shm->Proc.voltageFormula)) {
	case VOLTAGE_KIND_INTEL:
		ComputeVoltageFormula = ComputeVoltage_Intel_Matrix;
		break;
	case VOLTAGE_KIND_INTEL_CORE2:
		ComputeVoltageFormula = ComputeVoltage_Intel_Core2_Matrix;
		break;
	case VOLTAGE_KIND_INTEL_SOC:
		ComputeVoltageFormula = ComputeVoltage_Intel_SoC_Matrix;
		break;
	case VOLTAGE_KIND_INTEL_SNB:
		ComputeVoltageFormula = ComputeVoltage_Intel_SNB_Matrix;
		break;
	case VOLTAGE_KIND_INTEL_SKL_X:
		ComputeVoltageFormula = ComputeVoltage_Intel_SKL_X_Matrix;
		break;
	case VOLTAGE_KIND_AMD:
		ComputeVoltageFormula = ComputeVoltage_AMD_Matrix;
		break;
	case VOLTAGE_KIND_AMD_0Fh:
		ComputeVoltageFormula = ComputeVoltage_AMD_0Fh_Matrix;
		break;
	case VOLTAGE_KIND_AMD_15h:
		ComputeVoltageFormula = ComputeVoltage_AMD_15h_Matrix;
		break;
	case VOLTAGE_KIND_AMD_17h:
		ComputeVoltageFormula = ComputeVoltage_AMD_17h_Matrix;
		break;
	case VOLTAGE_KIND_WINBOND_IO:
		ComputeVoltageFormula = ComputeVoltage_Winbond_IO_Matrix;
		break;
	case VOLTAGE_KIND_ITETECH_IO:
		ComputeVoltageFormula = ComputeVoltage_ITE_Tech_IO_Matrix;
		break;
	case VOLTAGE_KIND_NONE:
	default:
		ComputeVoltageFormula = ComputeVoltage_None_Matrix;
		break;
	}

	switch (KIND_OF_FORMULA(Shm->Proc.powerFormula)) {
	case POWER_KIND_INTEL:
		ComputePowerFormula = ComputePower_Intel_Matrix;
		break;
	case POWER_KIND_INTEL_ATOM:
		ComputePowerFormula = ComputePower_Intel_Atom_Matrix;
		break;
	case POWER_KIND_AMD:
		ComputePowerFormula = ComputePower_AMD_Matrix;
		break;
	case POWER_KIND_AMD_17h:
		ComputePowerFormula = ComputePower_AMD_17h_Matrix;
		break;
	case POWER_KIND_NONE:
	default:
		ComputePowerFormula = ComputePower_None_Matrix;
		break;
	}

	if (Quiet & 0x100) {
		printf("    Thread [%lx] Init CYCLE %03u\n", tid, cpu);
		fflush(stdout);
	}
	BITSET_CC(BUS_LOCK, roomSeed, cpu);
	BITSET_CC(BUS_LOCK, roomCore, cpu);

  do {
    while (!BITCLR(LOCKLESS, RW(Core)->Sync.V, NTFY)
	&& !BITVAL(Shutdown, SYNC)
	&& !BITVAL(RO(Core)->OffLine, OS)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
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
	memcpy(Cpu->Boost, RO(Core)->Boost, (BOOST(SIZE))*sizeof(unsigned int));

	CFlip->Absolute.Ratio.Perf = (double)RO(Core)->Ratio.COF.Q;
	CFlip->Absolute.Ratio.Perf +=(double)RO(Core)->Ratio.COF.R /UNIT_KHz(1);

	/* Compute IPS=Instructions per TSC				*/
	CFlip->State.IPS = (double)CFlip->Delta.INST
			 / (double)CFlip->Delta.TSC;

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
	CFlip->State.Turbo	= (double)CFlip->Delta.C0.UCC
				/ (double)CFlip->Delta.TSC;

	/* Compute the C-States.					*/
	CFlip->State.C0 = (double)CFlip->Delta.C0.URC
			/ (double)CFlip->Delta.TSC;

	CFlip->State.C3 = (double)CFlip->Delta.C3
			/ (double)CFlip->Delta.TSC;

	CFlip->State.C6 = (double)CFlip->Delta.C6
			/ (double)CFlip->Delta.TSC;

	CFlip->State.C7 = (double)CFlip->Delta.C7
			/ (double)CFlip->Delta.TSC;

	CFlip->State.C1 = (double)CFlip->Delta.C1
			/ (double)CFlip->Delta.TSC;

	/* Relative Frequency = Relative Ratio x Bus Clock Frequency	*/
	CFlip->Relative.Ratio	= (double)(CFlip->Delta.C0.UCC
					* Cpu->Boost[BOOST(MAX)])
				/ (double)CFlip->Delta.TSC;

	CFlip->Relative.Freq	= REL_FREQ_MHz( double,
						CFlip->Relative.Ratio,
						CFlip->Clock,
						Shm->Sleep.Interval );

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
	/* Per Core, evaluate thermal properties.			*/
	CFlip->Thermal.Sensor	= RO(Core)->PowerThermal.Sensor;
	CFlip->Thermal.Events	= RO(Core)->PowerThermal.Events;
	CFlip->Thermal.Param	= RO(Core)->PowerThermal.Param;

	ComputeThermalFormula[SCOPE_OF_FORMULA(Shm->Proc.thermalFormula)](
		CFlip, Shm, cpu
	);

	/* Per Core, evaluate the voltage properties.			*/
	CFlip->Voltage.VID = RO(Core)->PowerThermal.VID;

	ComputeVoltageFormula[SCOPE_OF_FORMULA(Shm->Proc.voltageFormula)](
		CFlip, Shm, cpu
	);

	/* Per Core, evaluate the Power properties.			*/
	CFlip->Delta.Power.ACCU = RO(Core)->Delta.Power.ACCU;

	ComputePowerFormula[SCOPE_OF_FORMULA(Shm->Proc.powerFormula)](
		CFlip, Shm, cpu
	);

	/* Copy the Interrupts counters.				*/
	CFlip->Counter.SMI = RO(Core)->Interrupt.SMI;

	/* If driver registered, copy any NMI counter.			*/
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_LOCAL) == 1) {
		CFlip->Counter.NMI.LOCAL   = RO(Core)->Interrupt.NMI.LOCAL;
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_UNKNOWN) == 1) {
		CFlip->Counter.NMI.UNKNOWN = RO(Core)->Interrupt.NMI.UNKNOWN;
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_SERR) == 1) {
		CFlip->Counter.NMI.PCISERR = RO(Core)->Interrupt.NMI.PCISERR;
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_IO_CHECK) == 1) {
		CFlip->Counter.NMI.IOCHECK = RO(Core)->Interrupt.NMI.IOCHECK;
	}
    }
  } while (!BITVAL(Shutdown, SYNC) && !BITVAL(RO(Core)->OffLine, OS)) ;

	BITCLR_CC(BUS_LOCK, roomCore, cpu);
	BITCLR_CC(BUS_LOCK, roomSeed, cpu);
EXIT:
	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CYCLE %03u\n", tid,
			BITVAL(RO(Core)->OffLine, OS) ? "Offline" : "Shutdown",
			cpu);
		fflush(stdout);
	}
	return NULL;
}

void SliceScheduling(SHM_STRUCT *Shm, unsigned int cpu, enum PATTERN pattern)
{
	unsigned int seek;
	switch (pattern) {
	case RESET_CSP:
		for (seek = 0; seek < Shm->Proc.CPU.Count; seek++) {
			if (seek == Shm->Proc.Service.Core) {
				BITSET_CC(LOCKLESS, Shm->roomSched, seek);
			} else {
				BITCLR_CC(LOCKLESS, Shm->roomSched, seek);
			}
		}
		break;
	case ALL_SMT:
		if (cpu == Shm->Proc.Service.Core) {
			BITSTOR_CC(LOCKLESS, Shm->roomSched, roomSeed);
		}
		break;
	case RAND_SMT:
		do {
			seek = (unsigned int) rand();
			seek = seek % Shm->Proc.CPU.Count;
		} while (BITVAL(Shm->Cpu[seek].OffLine, OS));
		BITCLR_CC(LOCKLESS, Shm->roomSched, cpu);
		BITSET_CC(LOCKLESS, Shm->roomSched, seek);
		break;
	case RR_SMT:
		seek = cpu;
		do {
			seek++;
			if (seek >= Shm->Proc.CPU.Count) {
				seek = 0;
			}
		} while (BITVAL(Shm->Cpu[seek].OffLine, OS));
		BITCLR_CC(LOCKLESS, Shm->roomSched, cpu);
		BITSET_CC(LOCKLESS, Shm->roomSched, seek);
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
	SHM_STRUCT *Shm = Arg->Ref->Shm;
	CPU_STRUCT *Cpu = &Shm->Cpu[cpu];

	CALL_FUNC MatrixCallFunc[2][2] = {
		{ CallWith_RDTSC_No_RDPMC,  CallWith_RDTSC_RDPMC  },
		{ CallWith_RDTSCP_No_RDPMC, CallWith_RDTSCP_RDPMC }
	};
	const int withTSCP = ((Shm->Proc.Features.AdvPower.EDX.Inv_TSC == 1)
			   || (Shm->Proc.Features.ExtInfo.EDX.RDTSCP == 1)),
		withRDPMC = ((Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
			  && (Shm->Proc.PM_version >= 1)
			  && (BITVAL(Cpu->SystemRegister.CR4, CR4_PCE) == 1));

	CALL_FUNC CallSliceFunc = MatrixCallFunc[withTSCP][withRDPMC];

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
		printf("    Thread [%lx] Init CHILD %03u\n", tid, cpu);
		fflush(stdout);
	}

	BITSET_CC(BUS_LOCK, roomSeed, cpu);

	do {
		while (!BITVAL(Shm->Proc.Sync, BURN)
		    && !BITVAL(Shutdown, SYNC)
		    && !BITVAL(Cpu->OffLine, OS)) {
			nanosleep(&Shm->Sleep.sliceWaiting, NULL);
		}

		BITSET_CC(BUS_LOCK, roomCore, cpu);

		RESET_Slice(Cpu->Slice);

		while ( BITVAL(Shm->Proc.Sync, BURN)
		    && !BITVAL(Shutdown, SYNC) )
		{
		    if (BITVAL_CC(Shm->roomSched, cpu)) {
			CallSliceFunc(	Shm, cpu,
					Arg->Ref->Slice.Func,
					Arg->Ref->Slice.arg);

			SliceScheduling(Shm, cpu, Arg->Ref->Slice.pattern);

		      if (BITVAL(Cpu->OffLine, OS)) {/* ReSchedule to Service */
			SliceScheduling(Shm, Shm->Proc.Service.Core, RESET_CSP);
			break;
		      }
		    } else {
			nanosleep(&Shm->Sleep.sliceWaiting, NULL);

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
		printf("    Thread [%lx] %s CHILD %03u\n", tid,
			BITVAL(Cpu->OffLine, OS) ? "Offline" : "Shutdown",cpu);
		fflush(stdout);
	}
	return NULL;
}

void Architecture(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	signed long ix;
	Bit32	fTSC = RO(Proc)->Features.Std.EDX.TSC,
		aTSC = RO(Proc)->Features.AdvPower.EDX.Inv_TSC;

	/* Copy all initial CPUID features.				*/
	memcpy(&Shm->Proc.Features, &RO(Proc)->Features, sizeof(FEATURES));
	/* Copy the fomula identifiers					*/
	Shm->Proc.thermalFormula = RO(Proc)->thermalFormula;
	Shm->Proc.voltageFormula = RO(Proc)->voltageFormula;
	Shm->Proc.powerFormula   = RO(Proc)->powerFormula;
	/* Copy the numbers of total & online CPU.			*/
	Shm->Proc.CPU.Count	= RO(Proc)->CPU.Count;
	Shm->Proc.CPU.OnLine	= RO(Proc)->CPU.OnLine;
	Shm->Proc.Service.Proc	= RO(Proc)->Service.Proc;
	/* Architecture and Hypervisor identifiers.			*/
	Shm->Proc.ArchID = RO(Proc)->ArchID;
	Shm->Proc.HypervisorID = RO(Proc)->HypervisorID;
	/* Copy the Architecture name.					*/
	StrCopy(Shm->Proc.Architecture, RO(Proc)->Architecture, CODENAME_LEN);
	/* Make the processor's brand string with no trailing spaces	*/
	ix = BRAND_LENGTH;
	do {
		if ((RO(Proc)->Features.Info.Brand[ix] == 0x20)
		 || (RO(Proc)->Features.Info.Brand[ix] == 0x0)) {
			ix--;
		} else {
			break;
		}
	} while (ix != 0);
	Shm->Proc.Brand[1 + ix] = '\0';
	for (; ix >= 0; ix--) {
		Shm->Proc.Brand[ix] = RO(Proc)->Features.Info.Brand[ix];
	}
	/* Compute the TSC mode: None, Variant, Invariant		*/
	Shm->Proc.Features.InvariantTSC = fTSC << aTSC;
}

void PerformanceMonitoring(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	Shm->Proc.PM_version = RO(Proc)->Features.PerfMon.EAX.Version;
}

void HyperThreading(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{	/* Update the HyperThreading state				*/
	Shm->Proc.Features.HyperThreading = RO(Proc)->Features.HTT_Enable;
}

void PowerInterface(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short pwrUnits = 0, pwrVal;
	enum PWR_DOMAIN pw;

    switch (KIND_OF_FORMULA(RO(Proc)->powerFormula)) {
    case POWER_KIND_INTEL:
    case POWER_KIND_AMD:
    case POWER_KIND_AMD_17h:
	Shm->Proc.Power.Unit.Watts = RO(Proc)->PowerThermal.Unit.PU > 0 ?
		1.0 / (double) (1 << RO(Proc)->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= RO(Proc)->PowerThermal.Unit.ESU > 0 ?
		1.0 / (double) (1 << RO(Proc)->PowerThermal.Unit.ESU) :0;
      TIME_UNIT:
	Shm->Proc.Power.Unit.Times = RO(Proc)->PowerThermal.Unit.TU > 0 ?
			1.0 / (double) (1 << RO(Proc)->PowerThermal.Unit.TU)
			: 1.0 / (double) (1 << 10);

	pwrUnits = 2 << (RO(Proc)->PowerThermal.Unit.PU - 1);
	break;
    case POWER_KIND_INTEL_ATOM:
	Shm->Proc.Power.Unit.Watts = RO(Proc)->PowerThermal.Unit.PU > 0 ?
		0.001 / (double) (1 << RO(Proc)->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= RO(Proc)->PowerThermal.Unit.ESU > 0 ?
		0.001 / (double) (1 << RO(Proc)->PowerThermal.Unit.ESU) : 0;
	goto TIME_UNIT;
	break;
    case POWER_KIND_NONE:
	break;
    }
  if (	(Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
  ||	(Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
  { /* AMD PowerNow */
	if (RO(Proc)->Features.AdvPower.EDX.FID) {
		BITSET(LOCKLESS, Shm->Proc.PowerNow, 0);
	} else {
		BITCLR(LOCKLESS, Shm->Proc.PowerNow, 0);
	}
	if (RO(Proc)->Features.AdvPower.EDX.VID) {
		BITSET(LOCKLESS, Shm->Proc.PowerNow, 1);
	} else {
		BITCLR(LOCKLESS, Shm->Proc.PowerNow, 1);
	}
	Shm->Proc.Power.PPT = RO(Proc)->PowerThermal.Zen.PWR.PPT;
	Shm->Proc.Power.TDP = RO(Proc)->PowerThermal.Zen.TDP.TDP;
	Shm->Proc.Power.Min = RO(Proc)->PowerThermal.Zen.TDP.TDP2;
	Shm->Proc.Power.Max = RO(Proc)->PowerThermal.Zen.TDP.TDP3;
	Shm->Proc.Power.EDC = RO(Proc)->PowerThermal.Zen.EDC.EDC << 2;
	Shm->Proc.Power.TDC = RO(Proc)->PowerThermal.Zen.EDC.TDC;

      for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
      {
	Shm->Proc.Power.Domain[pw].Feature[PL1].Enable = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Enable_Limit1;

	Shm->Proc.Power.Domain[pw].Feature[PL2].Enable = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Enable_Limit2;

	Shm->Proc.Power.Domain[pw].Feature[PL1].Clamping = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Clamping1;

	Shm->Proc.Power.Domain[pw].Feature[PL2].Clamping = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Clamping2;

	pwrVal = RO(Proc)->PowerThermal.PowerLimit[pw].Domain_Limit1;
	Shm->Proc.Power.Domain[pw].PL1 = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.PowerLimit[pw].Domain_Limit2;
	Shm->Proc.Power.Domain[pw].PL2 = pwrVal;
      }
  }
  else if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
    if (pwrUnits != 0)
    {
      for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
      {
	Shm->Proc.Power.Domain[pw].Feature[PL1].Enable = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Enable_Limit1;

	Shm->Proc.Power.Domain[pw].Feature[PL2].Enable = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Enable_Limit2;

	Shm->Proc.Power.Domain[pw].Feature[PL1].Clamping = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Clamping1;

	Shm->Proc.Power.Domain[pw].Feature[PL2].Clamping = \
			RO(Proc)->PowerThermal.PowerLimit[pw].Clamping2;

	pwrVal = RO(Proc)->PowerThermal.PowerLimit[pw].Domain_Limit1 / pwrUnits;
	Shm->Proc.Power.Domain[pw].PL1 = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.PowerLimit[pw].Domain_Limit2 / pwrUnits;
	Shm->Proc.Power.Domain[pw].PL2 = pwrVal;
      }
	pwrVal = RO(Proc)->PowerThermal.PowerInfo.ThermalSpecPower / pwrUnits;
	Shm->Proc.Power.TDP = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.PowerInfo.MinimumPower / pwrUnits;
	Shm->Proc.Power.Min = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.PowerInfo.MaximumPower / pwrUnits;
	Shm->Proc.Power.Max = pwrVal;
    }
    for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
    {
      if (RO(Proc)->PowerThermal.PowerLimit[pw].TimeWindow1 > 0)
      {
	unsigned long long duration;
	duration = (1 << RO(Proc)->PowerThermal.PowerLimit[pw].TimeWindow1_Y);
	duration *= (4 + RO(Proc)->PowerThermal.PowerLimit[pw].TimeWindow1_Z);
	duration = duration >> 2;

	Shm->Proc.Power.Domain[pw].TW1	= Shm->Proc.Power.Unit.Times
					* (double)duration;
	if (Shm->Proc.Power.Domain[pw].TW1 < 1.0)
		Shm->Proc.Power.Domain[pw].TW1 = 1.0;
      } else {
	Shm->Proc.Power.Domain[pw].TW1 = 1.0;
      }
      if (RO(Proc)->PowerThermal.PowerLimit[pw].TimeWindow2 > 0)
      {
	unsigned long long duration;
	duration = (1 << RO(Proc)->PowerThermal.PowerLimit[pw].TimeWindow2_Y);
	duration *= (4 + RO(Proc)->PowerThermal.PowerLimit[pw].TimeWindow2_Z);
	duration = duration >> 2;

	Shm->Proc.Power.Domain[pw].TW2	= Shm->Proc.Power.Unit.Times
					* (double)duration;
	if (Shm->Proc.Power.Domain[pw].TW2 < 1.0)
		Shm->Proc.Power.Domain[pw].TW2 = 1.0;
      } else {
	Shm->Proc.Power.Domain[pw].TW2 = 1.0;
      }
    }
	Shm->Proc.Power.TDC = RO(Proc)->PowerThermal.TDC;
	Shm->Proc.Power.Feature.TDC = RO(Proc)->PowerThermal.Enable_Limit.TDC;
  } else {
	Shm->Proc.PowerNow = 0;
  }
}

void Technology_Update(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RW(PROC) *RW(Proc))
{	/* Technologies aggregation.					*/
	Shm->Proc.Technology.PowerNow = (Shm->Proc.PowerNow == 0b11);

	Shm->Proc.Technology.ODCM = BITCMP_CC(	LOCKLESS,
						RW(Proc)->ODCM,
						RO(Proc)->ODCM_Mask );

	Shm->Proc.Technology.L1_HW_Prefetch=BITCMP_CC(LOCKLESS,
						RW(Proc)->L1_HW_Prefetch,
						RO(Proc)->DCU_Mask );

	Shm->Proc.Technology.L1_HW_IP_Prefetch=BITCMP_CC(LOCKLESS,
						RW(Proc)->L1_HW_IP_Prefetch,
						RO(Proc)->DCU_Mask );

	Shm->Proc.Technology.L2_HW_Prefetch = BITCMP_CC(LOCKLESS,
						RW(Proc)->L2_HW_Prefetch,
						RO(Proc)->DCU_Mask );

	Shm->Proc.Technology.L2_HW_CL_Prefetch=BITCMP_CC(LOCKLESS,
						RW(Proc)->L2_HW_CL_Prefetch,
						RO(Proc)->DCU_Mask );

	Shm->Proc.Technology.PowerMgmt=BITCMP_CC(LOCKLESS,
						RW(Proc)->PowerMgmt,
						RO(Proc)->PowerMgmt_Mask);

	Shm->Proc.Technology.EIST = BITCMP_CC(	LOCKLESS,
						RW(Proc)->SpeedStep,
						RO(Proc)->SpeedStep_Mask );

	Shm->Proc.Technology.Turbo = BITWISEAND_CC(LOCKLESS,
						RW(Proc)->TurboBoost,
						RO(Proc)->TurboBoost_Mask) != 0;

	Shm->Proc.Technology.C1E = BITCMP_CC(	LOCKLESS,
						RW(Proc)->C1E,
						RO(Proc)->C1E_Mask );

	Shm->Proc.Technology.C3A = BITCMP_CC(	LOCKLESS,
						RW(Proc)->C3A,
						RO(Proc)->C3A_Mask );

	Shm->Proc.Technology.C1A = BITCMP_CC(	LOCKLESS,
						RW(Proc)->C1A,
						RO(Proc)->C1A_Mask );

	Shm->Proc.Technology.C3U = BITCMP_CC(	LOCKLESS,
						RW(Proc)->C3U,
						RO(Proc)->C3U_Mask );

	Shm->Proc.Technology.C1U = BITCMP_CC(	LOCKLESS,
						RW(Proc)->C1U,
						RO(Proc)->C1U_Mask );

	Shm->Proc.Technology.CC6 = BITCMP_CC(	LOCKLESS,
						RW(Proc)->CC6,
						RO(Proc)->CC6_Mask );

	Shm->Proc.Technology.PC6 = BITCMP_CC(	LOCKLESS,
						RW(Proc)->PC6,
						RO(Proc)->PC6_Mask );

	Shm->Proc.Technology.SMM = BITCMP_CC(	LOCKLESS,
						RW(Proc)->SMM,
						RO(Proc)->CR_Mask );

	Shm->Proc.Technology.VM = BITCMP_CC(	LOCKLESS,
						RW(Proc)->VM,
						RO(Proc)->CR_Mask );

	Shm->Proc.Technology.WDT = BITCMP_CC(	LOCKLESS,
						RW(Proc)->WDT,
						RO(Proc)->WDT_Mask );

								/* 000v */
	Shm->Proc.Technology.TM1 = RO(Proc)->Features.Std.EDX.TM1
				 | RO(Proc)->Features.AdvPower.EDX.TTP;
								/* 00v0 */
	Shm->Proc.Technology.TM1 |= BITCMP_CC(	LOCKLESS,
						RW(Proc)->TM1,
						RO(Proc)->TM_Mask ) << 1;

								/* 000v */
	Shm->Proc.Technology.TM2 = RO(Proc)->Features.Std.ECX.TM2
				 | RO(Proc)->Features.AdvPower.EDX.TM;
								/* 00v0 */
	Shm->Proc.Technology.TM2 |= BITCMP_CC(	LOCKLESS,
						RW(Proc)->TM2,
						RO(Proc)->TM_Mask ) << 1;
}

void Mitigation_2nd_Stage(SHM_STRUCT *Shm,RO(PROC) *RO(Proc),RW(PROC) *RW(Proc))
{
	unsigned short	IBRS = BITCMP_CC(	LOCKLESS,
						RW(Proc)->IBRS,
						RO(Proc)->SPEC_CTRL_Mask ),

			STIBP = BITCMP_CC(	LOCKLESS,
						RW(Proc)->STIBP,
						RO(Proc)->SPEC_CTRL_Mask ),

			SSBD = BITCMP_CC(	LOCKLESS,
						RW(Proc)->SSBD,
						RO(Proc)->SPEC_CTRL_Mask );

	Shm->Proc.Mechanisms.IBRS +=	(2 * IBRS);
	Shm->Proc.Mechanisms.STIBP +=	(2 * STIBP);
	Shm->Proc.Mechanisms.SSBD +=	(2 * SSBD);
}

void Mitigation_1st_Stage(SHM_STRUCT *Shm,RO(PROC) *RO(Proc),RW(PROC) *RW(Proc))
{
    if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
    {
	unsigned short	RDCL_NO = BITCMP_CC(	LOCKLESS,
						RW(Proc)->RDCL_NO,
						RO(Proc)->ARCH_CAP_Mask ),

			IBRS_ALL = BITCMP_CC(	LOCKLESS,
						RW(Proc)->RDCL_NO,
						RO(Proc)->ARCH_CAP_Mask ),

			RSBA = BITCMP_CC(	LOCKLESS,
						RW(Proc)->RSBA,
						RO(Proc)->ARCH_CAP_Mask ),

			L1DFL_NO = BITCMP_CC(	LOCKLESS,
						RW(Proc)->L1DFL_VMENTRY_NO,
						RO(Proc)->ARCH_CAP_Mask ),

			SSB_NO = BITCMP_CC(	LOCKLESS,
						RW(Proc)->SSB_NO,
						RO(Proc)->ARCH_CAP_Mask ),

			MDS_NO = BITCMP_CC(	LOCKLESS,
						RW(Proc)->MDS_NO,
						RO(Proc)->ARCH_CAP_Mask ),

			PSCHANGE_MC_NO=BITCMP_CC(LOCKLESS,
						RW(Proc)->PSCHANGE_MC_NO,
						RO(Proc)->ARCH_CAP_Mask),

			TAA_NO = BITCMP_CC(	LOCKLESS,
						RW(Proc)->TAA_NO,
						RO(Proc)->ARCH_CAP_Mask );

	Shm->Proc.Mechanisms.IBRS = (
		Shm->Proc.Features.ExtFeature.EDX.IBRS_IBPB_Cap == 1
	);
	Shm->Proc.Mechanisms.STIBP = (
		Shm->Proc.Features.ExtFeature.EDX.STIBP_Cap == 1
	);
	Shm->Proc.Mechanisms.SSBD = (
		Shm->Proc.Features.ExtFeature.EDX.SSBD_Cap == 1
	);

	Mitigation_2nd_Stage(Shm, RO(Proc), RW(Proc));

	Shm->Proc.Mechanisms.RDCL_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * RDCL_NO)
	);
	Shm->Proc.Mechanisms.IBRS_ALL = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * IBRS_ALL)
	);
	Shm->Proc.Mechanisms.RSBA = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * RSBA)
	);
	Shm->Proc.Mechanisms.L1DFL_VMENTRY_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * L1DFL_NO)
	);
	Shm->Proc.Mechanisms.SSB_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * SSB_NO)
	);
	Shm->Proc.Mechanisms.MDS_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * MDS_NO)
	);
	Shm->Proc.Mechanisms.PSCHANGE_MC_NO = (
	    Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2*PSCHANGE_MC_NO)
	);
	Shm->Proc.Mechanisms.TAA_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP + (2 * TAA_NO)
	);
	Shm->Proc.Mechanisms.STLB = BITCMP_CC(	LOCKLESS,
						RW(Proc)->STLB,
						RO(Proc)->ARCH_CAP_Mask );
	Shm->Proc.Mechanisms.FUSA = BITCMP_CC(	LOCKLESS,
						RW(Proc)->FUSA,
						RO(Proc)->ARCH_CAP_Mask );
	Shm->Proc.Mechanisms.RSM_CPL0 = BITCMP_CC(LOCKLESS,
						RW(Proc)->RSM_CPL0,
						RO(Proc)->ARCH_CAP_Mask );
	Shm->Proc.Mechanisms.SPLA = BITCMP_CC(	LOCKLESS,
						RW(Proc)->SPLA,
						RO(Proc)->ARCH_CAP_Mask );
	Shm->Proc.Mechanisms.SNOOP_FILTER = BITCMP_CC(LOCKLESS,
						RW(Proc)->SNOOP_FILTER,
						RO(Proc)->ARCH_CAP_Mask );
    }
    else if (	(Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	||	(Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	unsigned short	PSFD = BITCMP_CC(	LOCKLESS,
						RW(Proc)->PSFD,
						RO(Proc)->SPEC_CTRL_Mask );

	Shm->Proc.Mechanisms.IBRS = (
		Shm->Proc.Features.leaf80000008.EBX.IBRS == 1
	);
	Shm->Proc.Mechanisms.STIBP = (
		Shm->Proc.Features.leaf80000008.EBX.STIBP == 1
	);
	Shm->Proc.Mechanisms.SSBD = (
		Shm->Proc.Features.leaf80000008.EBX.SSBD == 1
	);

	Mitigation_2nd_Stage(Shm, RO(Proc), RW(Proc));

	Shm->Proc.Mechanisms.PSFD = (
		Shm->Proc.Features.leaf80000008.EBX.PSFD + (2 * PSFD)
	);
    }
}

void Package_Update(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RW(PROC) *RW(Proc))
{	/* Copy the operational settings.				*/
	Shm->Registration.AutoClock = RO(Proc)->Registration.AutoClock;
	Shm->Registration.Experimental = RO(Proc)->Registration.Experimental;
	Shm->Registration.HotPlug = RO(Proc)->Registration.HotPlug;
	Shm->Registration.PCI = RO(Proc)->Registration.PCI;
	Shm->Registration.PFM = RO(Proc)->Registration.PFM;
	Shm->Registration.I2C = RO(Proc)->Registration.I2C;
	Shm->Registration.HSMP= RO(Proc)->Features.HSMP_Enable;
	BITSTOR(LOCKLESS, Shm->Registration.NMI, RO(Proc)->Registration.NMI);
	Shm->Registration.Driver = RO(Proc)->Registration.Driver;
	/* Copy the timer interval delay.				*/
	Shm->Sleep.Interval = RO(Proc)->SleepInterval;
	/* Compute the polling wait time based on the timer interval.	*/
	Shm->Sleep.pollingWait = TIMESPEC((Shm->Sleep.Interval * 1000000L)
				/ WAKEUP_RATIO);
	/* Copy the SysGate tick steps.					*/
	Shm->SysGate.tickReset = RO(Proc)->tickReset;
	Shm->SysGate.tickStep  = RO(Proc)->tickStep;

	Architecture(Shm, RO(Proc));

	PerformanceMonitoring(Shm, RO(Proc));

	HyperThreading(Shm, RO(Proc));

	PowerInterface(Shm, RO(Proc));

	Mitigation_1st_Stage(Shm, RO(Proc), RW(Proc));
}

typedef struct {
	unsigned int	Q,
			R;
} RAM_Ratio;

void P945_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT0.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRAS;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRRD) {
	case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 1;
		break;
	case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = 2;
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRFC;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tCL) {
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

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRCD + 2;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRP + 2;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
		unsigned short rank, rankCount = (cha == 0) ? 4 : 2;

	    for (rank = 0; rank < rankCount; rank++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += \
		  RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRB[rank].Boundary;
	    }
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.BANK.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.WIDTH.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 16384;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 8192;
		break;
	    }
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

		DIMM_Size=Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT2.tCKE;
      }
    }
}

void P955_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tRAS;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tCL) {
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

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tRCD + 2;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tRP + 2;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
		unsigned short rank;

	    for (rank = 0; rank < 4; rank++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += \
		  RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRB[rank].Boundary;
	    }
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P955.BANK.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P955.WIDTH.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 16384;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 8192;
		break;
	    }
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

		DIMM_Size=Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
							(unsigned int)DIMM_Size;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P945_CLK(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b000:
		Shm->Uncore.Bus.Rate = 400;

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			fallthrough;
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

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			fallthrough;
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
		fallthrough;
	case 0b011:
		Shm->Uncore.Bus.Rate = 667;

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			fallthrough;
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

	Shm->Uncore.CtrlSpeed = (RO(Core)->Clock.Hz * Ratio.Q * 2) /* DDR2 */
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 2;
}

void P965_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT0.tCL;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT1.tRAS;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT1.tWR;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.tRFC;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.tRP;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.tRRD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT4.tRCD_RD;
/* TODO(Timings)
	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT_.tFAW;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT_.tRTPr;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = ?
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL  += 3;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
/* TODO(Geometry):Hardware missing! */
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P965.CKE0.tCKE_Low
				: RO(Proc)->Uncore.MC[mc].P965.CKE1.tCKE_Low;
      }
    }
}

void P965_CLK(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b111:	/* Unknown */
		fallthrough;
	case 0b000:
		Shm->Uncore.Bus.Rate = 1066;

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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
		fallthrough;
	case 0b010:
		Shm->Uncore.Bus.Rate = 800;

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

	Shm->Uncore.CtrlSpeed = (RO(Core)->Clock.Hz * Ratio.Q * 2) /* DDR2 */
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 2;
}

void G965_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT0.tWR;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRCD) {
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

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRP) {
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

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRAS;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRRD) {
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

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT2.tFAW;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tCL) {
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

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tRFC;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tCWL) {
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
		Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = \
			Shm->Uncore.MC[mc].Channel[cha].Timing.tCL - 1;
		break;
	}
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;

	  switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank1Bank)
	  {
	  case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	  case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	  }
	  switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank0Bank)
	  {
	  case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 4;
		break;
	  case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 8;
		break;
	  }
	  switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.OddRank1)
	  {
	  case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1;
		break;
	  case 0b11:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 2;
		break;
	  }
	  switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.EvenRank0)
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

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT2.tCKE;
      }
    }
}

void G965_CLK(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b001:
		Shm->Uncore.Bus.Rate = 533;

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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
		fallthrough;
	case 0b010:
		Shm->Uncore.Bus.Rate = 800;

		switch (RO(Proc)->Uncore.Bus.ClkCfg.RAM_Select) {
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

	Shm->Uncore.CtrlSpeed = (RO(Core)->Clock.Hz * Ratio.Q * 2) /* DDR2 */
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 2;
}

void P3S_MCH(	SHM_STRUCT *Shm, RO(PROC) *RO(Proc),
		unsigned short mc,unsigned short cha )
{
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT0.tCL;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT1.tWR;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.tRFC;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.tRP;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.tRRD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT4.tRCD_RD;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT5.tRAS;
/* TODO(Timings)
	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRTn.tFAW;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRTn.tRTPr;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWL   = ?
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P35.CKE0.tCKE_Low
				: RO(Proc)->Uncore.MC[mc].P35.CKE1.tCKE_Low;
}

void P35_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	P3S_MCH(Shm, RO(Proc), mc, cha);

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL -= 9;
/* TODO(Timings) */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
/* TODO(Geometry):Hardware missing! */
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P35_CLK(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	P965_CLK(Shm, RO(Proc), RO(Core));
}

void P4S_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	P3S_MCH(Shm, RO(Proc), mc, cha);

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL -= 6;
/* TODO(Timings) */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
/* TODO(Geometry):Hardware missing! */
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void SLM_PTR(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned short mc, cha, slot;
/* BUS & DRAM frequency */
	Shm->Uncore.CtrlSpeed = 800LLU + (
			((2134LLU * RO(Proc)->Uncore.MC[0].SLM.DTR0.DFREQ) >> 3)
	);
	Shm->Uncore.Bus.Rate = 5000;

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 3;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
/* Standard Timings */
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL  = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tCL + 5;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tRCD + 5;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP  = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tRP + 5;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tRRD + 4;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tXS == 0 ? 256 : 384;

	switch (RO(Proc)->Uncore.MC[mc].SLM.DRFC.tREFI) {
	case 0 ... 1:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = 0;
		break;
	case 2:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = 39;
		break;
	case 3:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = 78;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI *= Shm->Uncore.CtrlSpeed;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI /= 20;

/*TODO( Advanced Timings )
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SLM
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tRTP + 4;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tWTP + 14;

	Shm->Uncore.MC[mc].Channel[cha].Timing.B2B   = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tCCD;

	switch (RO(Proc)->Uncore.MC[mc].SLM.DTR1.tFAW) {
	case 0 ... 1:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW = 0;
		break;
	default:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW = \
		10 + ((unsigned int)RO(Proc)->Uncore.MC[mc].SLM.DTR1.tFAW << 1);
		break;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tWCL + 3;
/* Same Rank */
/*TODO( Read to Read )
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR?.;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr = 6
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tRWSR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd = 11
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tWRSR;
/*TODO( Write to Write )
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SLM.DTR?.;
*/
/* Different Rank */
	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRRDR;
	if (Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd > 0) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd += 5;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRWDR;
	if (Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr > 0) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr += 5;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tWRDR;
	if (Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd > 0) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd += 3;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr = 4
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tWWDR;
/* Different DIMM */
	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRRDD;
	if (Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd > 0) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd += 5;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRWDD;
	if (Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr > 0) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr += 5;
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd = 4
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tWRDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr = 4
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tWWDD;
/* Command Rate */
	Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR1.tCMD;
/* Topology */
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
		const struct {
			unsigned int	Banks,
					Rows,
					Cols;
		} DDR3L[4] = {
			{ .Banks = 8,	.Rows = 1<<14,	.Cols = 1<<10	},
			{ .Banks = 8,	.Rows = 1<<15,	.Cols = 1<<10	},
			{ .Banks = 8,	.Rows = 1<<16,	.Cols = 1<<10	},
			{ .Banks = 8,	.Rows = 1<<14,	.Cols = 1<<10	}
		};
	    if  (cha == 0) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SLM.DRP.RKEN0
				+	RO(Proc)->Uncore.MC[mc].SLM.DRP.RKEN1;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN0].Banks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN0].Rows;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN0].Cols;
	    } else {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SLM.DRP.RKEN2
				+	RO(Proc)->Uncore.MC[mc].SLM.DRP.RKNE3;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN1].Banks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN1].Rows;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN1].Cols;
	    }
		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
/* Error Correcting Code */
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = \
			  RO(Proc)->Uncore.MC[mc].SLM.BIOS_CFG.EFF_ECC_EN
			| RO(Proc)->Uncore.MC[mc].SLM.BIOS_CFG.ECC_EN;
      }
    }
}

void NHM_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;

      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;
      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tCL ?
		4 + RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tCL : 0;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tWR) {
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
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Refresh.tREFI_8;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.CKE_Timing.tCKE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tWTPr;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrRdTRd) {
	case 0b0:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = 4;
		break;
	case 0b1:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = 6;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd = 10
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddWrTRd;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tsrWrTWr) {
	case 0b0:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = 4;
		break;
	case 0b1:
		Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = 6;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tdrWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tddWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.B2B  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.B2B;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR2_3.tCWL) {
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

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Params.ENABLE_2N_3N)
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
	  if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.DIMMPRESENT)
	  {
		unsigned long long DIMM_Size;

	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMBANK)
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
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMRANK)
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
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMROW)
	    {
	    case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 12;
		break;
	    case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 13;
		break;
	    case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 14;
		break;
	    case 0b011:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 15;
		break;
	    case 0b100:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 16;
		break;
	    }
	    switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMCOL)
	    {
	    case 0b000:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 10;
		break;
	    case 0b001:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 11;
		break;
	    case 0b010:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 12;
		break;
	    }

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	  }
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (unsigned int)\
			( RO(Proc)->Uncore.MC[mc].NHM.STATUS.ECC_ENABLED
			& RO(Proc)->Uncore.MC[mc].NHM.CONTROL.ECC_ENABLED );
      }
    }
}

void QPI_CLK(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.DimmClock.QCLK_RATIO) {
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
		fallthrough;
	default:
		Shm->Uncore.CtrlSpeed = 800;
		break;
	}

	Shm->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = \
			RO(Proc)->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 0b00 ?
		4800 :	RO(Proc)->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 0b10 ?
		6400 :	RO(Proc)->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 0b01 ?
		5866 : 6400;

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 3;
}

void X58_VTD(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	UNUSED(RO(Core));
    Shm->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.QuickPath.X58.VT_d;
    Shm->Proc.Technology.IOMMU_Ver_Major = RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;
    Shm->Proc.Technology.IOMMU_Ver_Minor = RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void DMI_CLK(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.DimmClock.QCLK_RATIO) {
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
		fallthrough;
	default:
		Shm->Uncore.CtrlSpeed = 266;
		break;
	}

	Shm->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = 2500;	/* TODO: hardwired to Lynnfield */

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 3;
}

void SNB_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      unsigned short dimmSize[2][2] = {
	{
		RO(Proc)->Uncore.MC[mc].SNB.MAD0.Dimm_A_Size,
		RO(Proc)->Uncore.MC[mc].SNB.MAD0.Dimm_B_Size
	}, {
		RO(Proc)->Uncore.MC[mc].SNB.MAD1.Dimm_A_Size,
		RO(Proc)->Uncore.MC[mc].SNB.MAD1.Dimm_B_Size
	}
      };

      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RFTP.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RFTP.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tWTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tCKE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tCWL;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.CMD_Stretch) {
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

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned int width, DIMM_Banks;

	    if (slot == 0) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DANOR;

		width = RO(Proc)->Uncore.MC[mc].SNB.MAD0.DAW;
	    } else {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBNOR;

		width = RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBW;
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

		DIMM_Banks = 8 * dimmSize[cha][slot] * 1024 * 1024;

		DIMM_Banks = DIMM_Banks
			/ (Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = DIMM_Banks;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (cha == 0) ?
					  RO(Proc)->Uncore.MC[mc].SNB.MAD0.ECC
					: RO(Proc)->Uncore.MC[mc].SNB.MAD1.ECC;
      }
    }
}

void SNB_CAP(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.SNB_Cap.DMFC) {
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
	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 3;

	Shm->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.SNB_Cap.VT_d;
}

void SNB_EP_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCWL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.EP.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.EP.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tCKE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWRDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWRDR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd = \
		4U + RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCWL
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWDR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWSR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRDR;
/* TODO
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRSR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWDR;
/* TODO
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWSR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.B2B = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tCCD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.CMD_Stretch)
	{
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

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
	    if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.DIMM_POP)
	    {
		unsigned long long DIMM_Size;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4 << \
		RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.DDR3_WIDTH;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1 << \
		RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.RANK_CNT;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << ( 13
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.RA_WIDTH);

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << ( 10
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.CA_WIDTH);

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	    }
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (unsigned int)\
				( RO(Proc)->Uncore.MC[mc].SNB_EP.TECH.ECC_EN
				& !RO(Proc)->Uncore.Bus.SNB_EP_Cap3.ECC_DIS );
      }
    }
}

void SNB_EP_CAP(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.SNB_EP_Cap1.DMFC) {
	case 0b111:
		Shm->Uncore.CtrlSpeed = 1066;
		break;
	case 0b110:
		Shm->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		Shm->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		Shm->Uncore.CtrlSpeed = 1866;
		break;
	case 0b011:
		Shm->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		Shm->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		Shm->Uncore.CtrlSpeed = 2666;
		break;
	case 0b000:
		Shm->Uncore.CtrlSpeed = 2933;
		break;
	}

	Shm->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = \
		RO(Proc)->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 0b010 ?
	5600 :	RO(Proc)->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 0b011 ?
	6400 :	RO(Proc)->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 0b100 ?
	7200 :	RO(Proc)->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 0b101 ?
	8000 : 5000;

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 3;
/* TODO */
	Shm->Proc.Technology.IOMMU = 0;
}

void IVB_CAP(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.IVB_Cap.DMFC) {
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
		switch (RO(Proc)->ArchID) {
		case IvyBridge:
		case Haswell_ULT:
			Shm->Uncore.CtrlSpeed = 2933;
			break;
		case Haswell_DT:
		case Haswell_EP:
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
	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 3;

	Shm->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.SNB_Cap.VT_d;
}

void HSW_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      unsigned short dimmSize[2][2] = {
	{
		RO(Proc)->Uncore.MC[mc].SNB.MAD0.Dimm_A_Size,
		RO(Proc)->Uncore.MC[mc].SNB.MAD0.Dimm_B_Size
	}, {
		RO(Proc)->Uncore.MC[mc].SNB.MAD1.Dimm_A_Size,
		RO(Proc)->Uncore.MC[mc].SNB.MAD1.Dimm_B_Size
	}
      };

      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank.tCWL;
/*TODO(Not Found)
	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW._.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW._.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.B2B   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW._.B2B;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Refresh.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.tsrRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.tdrRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.tddRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tsrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tdrWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tddWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tsrWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tdrWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tddWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tsrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tdrRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tddRdTWr;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.CMD_Stretch) {
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

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned int width, DIMM_Banks;

	    if (slot == 0) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DANOR;

		width = RO(Proc)->Uncore.MC[mc].SNB.MAD0.DAW;
	    } else {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBNOR;

		width = RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBW;
	    }
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks++;

	    if (width == 0) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 14;
	    } else {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 15;
	    }

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 10;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						dimmSize[cha][slot] * 256;

		DIMM_Banks = 8 * dimmSize[cha][slot] * 1024 * 1024;

		DIMM_Banks = DIMM_Banks
			/ (Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = DIMM_Banks;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (cha == 0) ?
					  RO(Proc)->Uncore.MC[mc].SNB.MAD0.ECC
					: RO(Proc)->Uncore.MC[mc].SNB.MAD1.ECC;
      }
    }
}

unsigned int SKL_DimmWidthToRows(unsigned int width)
{
	unsigned int rows = 0;
	switch (width) {
	case 0b00:
		rows = 8;
		break;
	case 0b01:
		rows = 16;
		break;
	case 0b10:
		rows = 32;
		break;
	}
	return (8 * 1024 * rows);
}

void SKL_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
	Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
	Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

     for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
     {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ACT.tRRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Refresh.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCWL -4U;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRDPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tWRPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ACT.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCWL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.CMD_Stretch) {
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

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.DRAM_Tech == 0b00 ? 16:8;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.x8_device_Dimm0 ?
			1024 : 512;
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.x8_device_Dimm1 ?
			1024 : 512;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.tCKE;
     }
	Shm->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].SKL.MADC0.ECC;

	Shm->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].SKL.MADC1.ECC;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD0.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD0.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD1.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD1.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD0.DLNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD0.DSNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD1.DLNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD1.DSNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD0.DLW);

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD0.DSW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD1.DLW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD1.DSW);
    }
}

void SKL_CAP(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned int DMFC;

	switch (RO(Proc)->ArchID) {
	case Skylake_UY:
	case Kabylake_UY:
		DMFC = RO(Proc)->Uncore.Bus.SKL_Cap_B.DMFC_DDR3;
		Shm->Uncore.Bus.Rate = 4000;	/* 4 GT/s QPI */
		break;
	default:
		DMFC = RO(Proc)->Uncore.Bus.SKL_Cap_C.DMFC_DDR4;
		Shm->Uncore.Bus.Rate = 8000;	/* 8 GT/s DMI3 */
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

	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 4;

    Shm->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.SKL_Cap_A.VT_d;
    Shm->Proc.Technology.IOMMU_Ver_Major = RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;
    Shm->Proc.Technology.IOMMU_Ver_Minor = RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void RKL_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
	Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
	Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

     for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
     {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ACT.tRRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Refresh.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCWL -4U;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRDPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tWRPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ACT.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCWL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.CMD_Stretch) {
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

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	!RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.ReservedBits1 ? 16 : 8;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.x8_device_Dimm0 ?
			1024 : 512;
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.x8_device_Dimm1 ?
			1024 : 512;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.PWDEN.tCKE;
     }
	Shm->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].RKL.MADC0.ECC;

	Shm->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].RKL.MADC1.ECC;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD0.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD0.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD1.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD1.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD0.DLNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD0.DSNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD1.DLNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD1.DSNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD0.DLW);

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD0.DSW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD1.DLW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD1.DSW);
    }
}

void RKL_CAP(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned int units = 12;
	unsigned short mc;

	Shm->Uncore.Bus.Rate = 8000;
	Shm->Uncore.Bus.Speed = (RO(Core)->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

  for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++) {
    if (RO(Proc)->Uncore.MC[mc].RKL.MADCH.value) {
	switch (RO(Proc)->Uncore.MC[mc].RKL.MADCH.DDR_TYPE) {
	default:
	case 0b00:	/*	DDR4	*/
		Shm->Uncore.Unit.DDR_Ver = 4;

		if ((RO(Proc)->Uncore.Bus.RKL_Cap_C.DDR4_EN)
		 && (RO(Proc)->Uncore.Bus.RKL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.RKL_Cap_C.DATA_RATE_DDR4;
		}
		break;
	case 0b11:	/*	LPDDR4	*/
		Shm->Uncore.Unit.DDR_Ver = 4;

		if ((RO(Proc)->Uncore.Bus.RKL_Cap_C.LPDDR4_EN)
		 && (RO(Proc)->Uncore.Bus.RKL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.RKL_Cap_C.DATA_RATE_LPDDR4;
		}
		break;
	case 0b01:	/*	DDR5	*/
		Shm->Uncore.Unit.DDR_Ver = 5;
		break;
	case 0b10:	/*	LPDDR5	*/
		Shm->Uncore.Unit.DDR_Ver = 5;
		break;
	}
    }
  }
	Shm->Uncore.CtrlSpeed = (266 * units) + ((333 * units) / 500);

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

    Shm->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.RKL_Cap_A.VT_d;
    Shm->Proc.Technology.IOMMU_Ver_Major = RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;
    Shm->Proc.Technology.IOMMU_Ver_Minor = RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void TGL_IMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
	Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
	Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

     for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
     {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ACT.tRRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Refresh.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Refresh.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCWL -4U;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRDPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tWRPRE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ACT.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCWL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_DD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_SG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_DG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_DR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.DDR4.tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.CMD_Stretch) {
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

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	!RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.ReservedBits1 ? 16 : 8;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.x8_device_Dimm0 ?
			1024 : 512;
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.x8_device_Dimm1 ?
			1024 : 512;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.PWDEN.tCKE;
     }
	Shm->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].TGL.MADC0.ECC;

	Shm->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].TGL.MADC1.ECC;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD0.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD0.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD1.Dimm_L_Size;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD1.Dimm_S_Size;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD0.DLNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD0.DSNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD1.DLNOR;

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD1.DSNOR;

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD0.DLW);

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD0.DSW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD1.DLW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD1.DSW);
    }
}

#define TGL_CAP RKL_CAP

void AMD_0Fh_MCH(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
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

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++) {
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tCL) {
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

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRCD) {
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

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRP) {
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

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRTPr) {
	case 0b0:
		if (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRL.BurstLength32 == 0b1) {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 2;
		} else {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 4;
		}
		break;
	case 0b1:
		if (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRL.BurstLength32 == 0b1) {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 3;
		} else {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 5;
		}
		break;
	}

	if (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRAS >= 0b0010) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRAS+3;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRC+11;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tWR) {
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

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRRD) {
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

	if ((RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.tFAW > 0b0000)
	 && (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.tFAW <= 0b1101)) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW = \
				RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.tFAW + 7;
	}

	if (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.SlowAccessMode == 0b1)
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
	else
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;

	shift = 4 * cha;
	mask  = 0b1111U << shift;

	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++) {
	  if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MBA.CSEnable)
	  {
		index =(RO(Proc)->Uncore.MC[mc].MaxDIMMs.AMD0Fh.CS.value & mask)
			>> shift;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
			module[index].size;
	  }
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = \
			RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRL.ECC_DIMM_Enable;
      }
    }
}

void AMD_0Fh_HTT(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned int link = 0;
	RAM_Ratio Ratio = {.Q = 0, .R = 0};
	unsigned long long HTT_Clock = 0;

	switch (RO(Proc)->Uncore.MC[0].AMD0Fh.DCRH.MemClkFreq) {
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
	Shm->Uncore.CtrlSpeed = (Ratio.Q * 2) + Ratio.R;	/* DDR2 */

	if ((link = RO(Proc)->Uncore.Bus.UnitID.McUnit) < 0b11) {
		switch (RO(Proc)->Uncore.Bus.LDTi_Freq[link].LinkFreqMax)
		{						/* "MHz" */
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
		Shm->Uncore.Bus.Rate = HTT_Clock * 2;	/* "MT/s" */
		Shm->Uncore.Bus.Speed = HTT_Clock * 4;	/* "MB/s" */
	}
	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b10;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 2;
}

void AMD_17h_UMC(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	unsigned short mc;
 for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
 {
	Shm->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;
	Shm->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;

	unsigned short cha;
  for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
  {
	unsigned long long DIMM_Size = 0;
	unsigned short chip;
   for (chip = 0; chip < 4; chip++)
   {
	const unsigned short slot = chip >> 1;
	unsigned short sec;
    for (sec = 0; sec < 2; sec++)
    {
	unsigned int chipSize = 0;
     if (BITVAL(RO(Proc)->Uncore.MC[mc].Channel[cha]\
		.AMD17h.CHIP[chip][sec].Chip.value, 0))
     {	/*			CSEnable				*/
	__asm__ volatile
	(
		"xorl	%%edx, %%edx"		"\n\t"
		"bsrl	%[base], %%ecx" 	"\n\t"
		"jz	1f"			"\n\t"
		"incl	%%edx"			"\n\t"
		"shll	%%cl, %%edx"	 	"\n\t"
		"negl	%%edx"			"\n\t"
		"notl	%%edx"			"\n\t"
		"andl	$0xfffffffe, %%edx"	"\n\t"
		"shrl	$2, %%edx"		"\n\t"
		"incl	%%edx"			"\n\t"
	"1:"					"\n\t"
		"movl	%%edx, %[dest]"
		: [dest] "=m" (chipSize)
		: [base] "m" (
	  RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.CHIP[chip][sec].Mask.value
		)
		: "cc", "memory", "%ecx", "%edx"
	);
	DIMM_Size += chipSize;
     }
    }
    if (DIMM_Size > 0)
    {
	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.Ranks;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = \
	8 << RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DAC.NumBanks;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = \
	1 << (10+RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DAC.NumRowLo);

	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = \
	1 << (5 + RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DAC.NumCol);

	Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 10);
    }
   }
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD_RD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tRCD_RD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD_WR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tRCD_WR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRDS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRRDS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRDL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRRDL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRRDDLR =
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRRDDLR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCPB = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRCPB;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRPPB = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRPPB;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR4.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAWSLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR4.tFAWSLR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAWDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR4.tFAWDLR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTRS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR5.tWTRS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTRL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR5.tWTRL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR6.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCPage = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR7.tRCPage;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRdRdScl = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tRdRdScl;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrWrScl = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tWrWrScl;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRdRdBan = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tRdRdBan;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrWrBan = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tWrWrBan;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR5.tCWL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRTP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR10.tddRdTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR10.tddWrTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tscWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tscWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tsdWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tsdWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tddWrTWr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tscRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tscRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tsdRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tsdRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tddRdTRd;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tRdRdScDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tRdRdScDLR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrWrScDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tWrWrScDLR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.Zen.tWrRdScDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR10.tWrRdScDLR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCKE  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR21.tCKE;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR12.tREFI;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC1 = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC.tRFC1;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC2 = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC.tRFC2;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC4 = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC.tRFC4;

	switch(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.CMD_Rate) {
	case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 1;
		break;
	case 0b10:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 2;
		break;
	default:
		Shm->Uncore.MC[mc].Channel[cha].Timing.CMD_Rate = 0;
		break;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.ECC.Enable;

	Shm->Uncore.MC[mc].Channel[cha].Timing.GDM = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.GearDownMode;

	Shm->Uncore.MC[mc].Channel[cha].Timing.BGS = \
		!((RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.BGS.value
		& AMD_17_UMC_BGS_MASK_OFF) == AMD_17_UMC_BGS_MASK_OFF);

	Shm->Uncore.MC[mc].Channel[cha].Timing.BGS_ALT = \
		(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.BGS_ALT.value
		& AMD_17_UMC_BGS_ALT_MASK_ON) == AMD_17_UMC_BGS_ALT_MASK_ON;

	Shm->Uncore.MC[mc].Channel[cha].Timing.PDM_EN = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ.PwrDownEn;

	Shm->Uncore.MC[mc].Channel[cha].Timing.PDM_MODE = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ.PwrDownMode;

	Shm->Uncore.MC[mc].Channel[cha].Timing.PDM_AGGR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ.AggrPwrDownEn;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tMRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tMOD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMOD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tMRD_PDA = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMRD_PDA;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tMOD_PDA = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMOD_PDA;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tSTAG = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR20.tSTAG;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tPHYWRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tPHY_WRDATA;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tPHYWRL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tPHY_WRLAT;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tPHYRDL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tPHY_RDLAT;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRDDATA = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tRDDATA_EN;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWRMPR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR35.tWR_MPR;
  }
 }
}

void AMD_17h_CAP(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned short mc, clock_done = 0;
  for (mc = 0; mc < Shm->Uncore.CtrlCount && !clock_done; mc++) {
	unsigned short cha;
    for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount && !clock_done; cha++) {
      if (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK)
      {
	Shm->Uncore.Bus.Rate = \
			(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK
			*  Shm->Proc.Features.Factory.Clock.Q) / 3;

	Shm->Uncore.Bus.Speed = \
			(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK
			* RO(Core)->Clock.Hz * 333333333LLU)
			/ (Shm->Proc.Features.Factory.Clock.Hz
			* (1000LLU * PRECISION * PRECISION));

	Shm->Uncore.CtrlSpeed = \
			(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK
			* RO(Core)->Clock.Hz * 666666666LLU)
			/ (Shm->Proc.Features.Factory.Clock.Hz
			* (1000LLU * PRECISION * PRECISION));

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Ver  = 4;

	clock_done = 1;
      }
    }
  }
}

void AMD_17h_IOMMU(SHM_STRUCT *Shm, RO(PROC) *RO(Proc))
{
	Shm->Proc.Technology.IOMMU = RO(Proc)->Uncore.Bus.IOMMU_CR.IOMMU_En;

	Shm->Proc.Technology.IOMMU_Ver_Major = \
			(RO(Proc)->Uncore.Bus.IOMMU_HDR.CapRev & 0b10000) >> 5;

	Shm->Proc.Technology.IOMMU_Ver_Minor = \
			RO(Proc)->Uncore.Bus.IOMMU_HDR.CapRev & 0b01111;
}

static char *Chipset[CHIPSETS] = {
	[IC_CHIPSET]		= NULL,
	[IC_LAKEPORT]		= "82945/Lakeport",
	[IC_LAKEPORT_P] 	= "82946/Lakeport-P",
	[IC_LAKEPORT_X] 	= "82955/Lakeport-X",
	[IC_CALISTOGA]		= "82945/Calistoga",
	[IC_BROADWATER] 	= "82965/Broadwater",
	[IC_CRESTLINE]		= "82965/Crestline",
	[IC_CANTIGA]		= "G45/Cantiga",
	[IC_BEARLAKE_Q] 	= "Q35/Bearlake-Q",
	[IC_BEARLAKE_P] 	= "G33/Bearlake-PG+",
	[IC_BEARLAKE_QF]	= "Q33/Bearlake-QF",
	[IC_BEARLAKE_X] 	= "X38/Bearlake-X",
	[IC_INTEL_3200] 	= "Intel 3200",
	[IC_EAGLELAKE_Q]	= "Q45/Eaglelake-Q",
	[IC_EAGLELAKE_P]	= "G45/Eaglelake-P",
	[IC_EAGLELAKE_G]	= "G41/Eaglelake-G",
	[IC_BAYTRAIL]		= "Bay Trail",
	[IC_TYLERSBURG] 	= "X58/Tylersburg",
	[IC_IBEXPEAK]		= "P55/Ibex Peak",
	[IC_IBEXPEAK_M] 	= "QM57/Ibex Peak-M",
	[IC_COUGARPOINT]	= "P67/Cougar Point",
	[IC_PATSBURG]		= "X79/Patsburg",
	[IC_CAVECREEK]		= "Cave Creek",
	[IC_WELLSBURG]		= "X99/Wellsburg",
	[IC_PANTHERPOINT]	= "Panther Point",
	[IC_PANTHERPOINT_M]	= "Panther Point-M",
	[IC_LYNXPOINT]		= "Lynx Point",
	[IC_LYNXPOINT_M]	= "Lynx Point-M",
	[IC_WILDCATPOINT]	= "Wildcat Point",
	[IC_WILDCATPOINT_M]	= "Wildcat Point-M",
	[IC_SUNRISEPOINT]	= "Sunrise Point",
	[IC_UNIONPOINT] 	= "Union Point",
	[IC_CANNONPOINT]	= "Cannon Point",
	[IC_400_SERIES_P]	= "400 Series-Prem-U",
	[IC_400_SERIES_M]	= "400 Series-Base-U",
	[IC_495]		= "Intel 495",
	[IC_H470]		= "Intel H470",
	[IC_Z490]		= "Intel Z490",
	[IC_Q470]		= "Intel Q470",
	[IC_HM470]		= "Intel HM470",
	[IC_QM480]		= "Intel QM480",
	[IC_WM490]		= "Intel WM490",
	[IC_W480]		= "Intel W480",
	[IC_H510]		= "Intel H510",
	[IC_B560]		= "Intel B560",
	[IC_H570]		= "Intel H570",
	[IC_Q570]		= "Intel Q570",
	[IC_Z590]		= "Intel Z590",
	[IC_W580]		= "Intel W580",
	[IC_K8] 		= "K8/HyperTransport",
	[IC_ZEN]		= "Zen UMC"
};

#define SET_CHIPSET(ic) 						\
({									\
	Shm->Uncore.ChipID = DID;					\
	Shm->Uncore.Chipset.ArchID = ic;				\
})

void PCI_Intel(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core),
		unsigned short DID)
{
	switch (DID) {
	case DID_INTEL_82945P_HB:
		P945_CLK(Shm, RO(Proc), RO(Core));
		P945_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_LAKEPORT);
		break;
	case DID_INTEL_82945GM_HB:
	case DID_INTEL_82945GME_HB:
		P945_CLK(Shm, RO(Proc), RO(Core));
		P945_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_CALISTOGA);
		break;
	case DID_INTEL_82955_HB:
		P945_CLK(Shm, RO(Proc), RO(Core));
		P955_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_LAKEPORT_X);
		break;
	case DID_INTEL_82946GZ_HB:
		P965_CLK(Shm, RO(Proc), RO(Core));
		P965_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_LAKEPORT_P);
		break;
	case DID_INTEL_82965Q_HB:
	case DID_INTEL_82965G_HB:
		P965_CLK(Shm, RO(Proc), RO(Core));
		P965_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_BROADWATER);
		break;
	case DID_INTEL_82965GM_HB:
	case DID_INTEL_82965GME_HB:
		G965_CLK(Shm, RO(Proc), RO(Core));
		G965_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_CRESTLINE);
		break;
	case DID_INTEL_GM45_HB:
		G965_CLK(Shm, RO(Proc), RO(Core));
		G965_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_CANTIGA);
		break;
	case DID_INTEL_Q35_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P35_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_Q);
		break;
	case DID_INTEL_G33_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P35_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_P);
		break;
	case DID_INTEL_Q33_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P35_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_QF);
		break;
	case DID_INTEL_X38_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P35_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_X);
		break;
	case DID_INTEL_3200_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P35_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_INTEL_3200);
		break;
	case DID_INTEL_Q45_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P4S_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_EAGLELAKE_Q);
		break;
	case DID_INTEL_G45_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P4S_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_EAGLELAKE_P);
		break;
	case DID_INTEL_G41_HB:
		P35_CLK(Shm, RO(Proc), RO(Core));
		P4S_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_EAGLELAKE_G);
		break;
	case DID_INTEL_SLM_PTR:
		SLM_PTR(Shm, RO(Proc), RO(Core));
		SET_CHIPSET(IC_BAYTRAIL);
		break;
	case DID_INTEL_X58_HUB_CTRL:
		QPI_CLK(Shm, RO(Proc), RO(Core));
		break;
	case DID_INTEL_X58_HUB_CORE:
	case DID_INTEL_IIO_CORE_REG:
		X58_VTD(Shm, RO(Proc), RO(Core));
		break;
	case DID_INTEL_I7_MCR:			/*	Bloomfield	*/
	case DID_INTEL_NHM_EP_MCR:		/*	Westmere EP	*/
		NHM_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_TYLERSBURG);
		break;
	case DID_INTEL_I7_MC_TEST:
	case DID_INTEL_LYNNFIELD_MC_TEST:
	case DID_INTEL_NHM_EP_MC_TEST:
	case DID_INTEL_NHM_EC_MC_TEST:
		DMI_CLK(Shm, RO(Proc), RO(Core));
		break;
	case DID_INTEL_LYNNFIELD_MCR:		/*	Lynnfield	*/
	case DID_INTEL_NHM_EC_MCR:		/*	C5500-C3500	*/
		NHM_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_IBEXPEAK);
		break;
	case DID_INTEL_SNB_IMC_HA0:		/*	Sandy Bridge-E	*/
		SNB_EP_CAP(Shm, RO(Proc), RO(Core));
		SNB_EP_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_PATSBURG);
		break;
	case DID_INTEL_SNB_IMC_SA:		/*	SNB Desktop	*/
		SNB_CAP(Shm, RO(Proc), RO(Core));
		SNB_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_COUGARPOINT);
		break;
	case DID_INTEL_SNB_IMC_0104:
		SNB_CAP(Shm, RO(Proc), RO(Core));
		SNB_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_IBEXPEAK_M);
		break;
	case DID_INTEL_IVB_EP_HOST_BRIDGE:	/* Xeon E5 & E7 v2	*/
		SNB_EP_CAP(Shm, RO(Proc), RO(Core));
		SNB_EP_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CAVECREEK);
		break;
	case DID_INTEL_IVB_IMC_SA:		/*	IVB Desktop	*/
		IVB_CAP(Shm, RO(Proc), RO(Core));
		SNB_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_PANTHERPOINT);
		break;
	case DID_INTEL_IVB_IMC_0154:		/* IVB Mobile i5-3337U	*/
		IVB_CAP(Shm, RO(Proc), RO(Core));
		SNB_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_PANTHERPOINT_M);
		break;
	case DID_INTEL_HASWELL_IMC_SA:		/* HSW & BDW Desktop	*/
	case DID_INTEL_HASWELL_MH_IMC_HA0:	/* HSW Mobile M/H	*/
	case DID_INTEL_HASWELL_UY_IMC_HA0:	/* HSW Mobile U/Y	*/
		IVB_CAP(Shm, RO(Proc), RO(Core));
		HSW_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_LYNXPOINT_M);
		break;
	case DID_INTEL_HASWELL_IMC_HA0: 		/* Haswell	*/
		IVB_CAP(Shm, RO(Proc), RO(Core));
		HSW_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_LYNXPOINT);
		break;
	case DID_INTEL_BROADWELL_IMC_HA0:	/* Broadwell/Y/U Core m */
		IVB_CAP(Shm, RO(Proc), RO(Core));
		HSW_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_WILDCATPOINT_M);
		break;
	case DID_INTEL_BROADWELL_D_IMC_HA0:	/*	BDW/Desktop	*/
	case DID_INTEL_BROADWELL_H_IMC_HA0:	/*	Broadwell/H	*/
		IVB_CAP(Shm, RO(Proc), RO(Core));
		HSW_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_WELLSBURG);
		break;
	case DID_INTEL_SKYLAKE_U_IMC_HA:	/* Skylake/U Processor */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_Y_IMC_HA:	/* Skylake/Y Processor */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_S_IMC_HAD:	/* Skylake/S Dual Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_S_IMC_HAQ:	/* Skylake/S Quad Core	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_H_IMC_HAD:	/* Skylake/H Dual Core	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_H_IMC_HAQ:	/* Skylake/H Quad Core	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_DT_IMC_HA:	/* Skylake/DT Server	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_KABYLAKE_U_IMC_HA:	/*	BGA 1356	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_Y_IMC_HA:	/*	BGA 1515	*/
	case DID_INTEL_KABYLAKE_Y_IMC_HQ:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_H_IMC_HAD:	/* Kaby Lake/H Dual Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_S_IMC_HAD:	/* Kaby Lake/S Dual Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_H_IMC_HAQ:	/* Kaby Lake/H Quad Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_DT_IMC_HA:	/* Kaby Lake/DT Server	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_U_IMC_HAQ:	/* U-Quad Core BGA 1356 */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_S_IMC_HAQ:	/* Kaby Lake/S Quad Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_X_IMC_HAQ:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_S_IMC_HAQ:	/* Coffee Lake Quad Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_S_IMC_HAS:	/* Coffee Lake Hexa Core */
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAD:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_U_IMC_HAD:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_U_IMC_HAQ:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_H_IMC_HAQ:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_H_IMC_HAS:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_H_IMC_HAO:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_W_IMC_HAQ:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_W_IMC_HAS:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_W_IMC_HAO:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAQ:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAS:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAO:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_WHISKEYLAKE_U_IMC_HAD:	/*	WHL Dual Core	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_WHISKEYLAKE_U_IMC_HAQ:	/*	WHL Quad Core	*/
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COMETLAKE_S_IMC_6C:
	case DID_INTEL_COMETLAKE_S_IMC_10C:
	case DID_INTEL_COMETLAKE_H_IMC_10C:
	case DID_INTEL_COMETLAKE_W_IMC_10C:
	case DID_INTEL_COMETLAKE_M_IMC_6C:
	case DID_INTEL_COMETLAKE_S1_IMC:
	case DID_INTEL_COMETLAKE_S2_IMC:
	case DID_INTEL_COMETLAKE_S5_IMC:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		break;
	case DID_INTEL_COMETLAKE_U_IMC_HB:
	case DID_INTEL_COMETLAKE_U1_IMC:
	case DID_INTEL_COMETLAKE_U3_IMC:
		RKL_CAP(Shm, RO(Proc), RO(Core));
		RKL_IMC(Shm, RO(Proc));
		break;
	case DID_INTEL_COMETLAKE_PREM_U_PCH:
		SET_CHIPSET(IC_400_SERIES_P);
		break;
	case DID_INTEL_COMETLAKE_BASE_U_PCH:
		SET_CHIPSET(IC_400_SERIES_M);
		break;
	case DID_INTEL_COMETLAKE_U_ES_PCH:
	case DID_INTEL_COMETLAKE_Y_ES_PCH:
	case DID_INTEL_COMETLAKE_Y_PCH:
	case DID_INTEL_ICELAKE_U_PCH:
		SET_CHIPSET(IC_495);
		break;
	case DID_INTEL_COMETLAKE_H470_PCH:
		SET_CHIPSET(IC_H470);
		break;
	case DID_INTEL_COMETLAKE_Z490_PCH:
		SET_CHIPSET(IC_Z490);
		break;
	case DID_INTEL_COMETLAKE_Q470_PCH:
		SET_CHIPSET(IC_Q470);
		break;
	case DID_INTEL_COMETLAKE_HM470_PCH:
		SET_CHIPSET(IC_HM470);
		break;
	case DID_INTEL_COMETLAKE_QM480_PCH:
		SET_CHIPSET(IC_QM480);
		break;
	case DID_INTEL_COMETLAKE_WM490_PCH:
		SET_CHIPSET(IC_WM490);
		break;
	case DID_INTEL_COMETLAKE_W480_PCH:
		SET_CHIPSET(IC_W480);
		break;
	case DID_INTEL_ICELAKE_U_IMC:
	case DID_INTEL_ICELAKE_U_4C:
		SKL_CAP(Shm, RO(Proc), RO(Core));
		SKL_IMC(Shm, RO(Proc));
		break;
	case DID_INTEL_TIGERLAKE_U1_IMC:
	case DID_INTEL_TIGERLAKE_U2_IMC:
	case DID_INTEL_TIGERLAKE_U3_IMC:
	case DID_INTEL_TIGERLAKE_U4_IMC:
	case DID_INTEL_TIGERLAKE_H_IMC:
		TGL_CAP(Shm, RO(Proc), RO(Core));
		TGL_IMC(Shm, RO(Proc));
		break;
	case DID_INTEL_ROCKETLAKE_S_8C_IMC_HB:
	case DID_INTEL_ROCKETLAKE_S_6C_IMC_HB:
		RKL_CAP(Shm, RO(Proc), RO(Core));
		RKL_IMC(Shm, RO(Proc));
		break;
	case DID_INTEL_TIGERLAKE_UP3_IMC:
	case DID_INTEL_TIGERLAKE_UP4_IMC:
		SET_CHIPSET(IC_H510);
		break;
	case DID_INTEL_ROCKETLAKE_H510_PCH:
		SET_CHIPSET(IC_H510);
		break;
	case DID_INTEL_ROCKETLAKE_B560_PCH:
		SET_CHIPSET(IC_B560);
		break;
	case DID_INTEL_ROCKETLAKE_H570_PCH:
		SET_CHIPSET(IC_H570);
		break;
	case DID_INTEL_ROCKETLAKE_Q570_PCH:
		SET_CHIPSET(IC_Q570);
		break;
	case DID_INTEL_ROCKETLAKE_Z590_PCH:
		SET_CHIPSET(IC_Z590);
		break;
	case DID_INTEL_ROCKETLAKE_W580_PCH:
		SET_CHIPSET(IC_W580);
		break;
	case DID_INTEL_ALDERLAKE_1_IMC:
	case DID_INTEL_ALDERLAKE_2_IMC:
		RKL_CAP(Shm, RO(Proc), RO(Core));
		RKL_IMC(Shm, RO(Proc));
		break;
	}
}

void PCI_AMD(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core),
		unsigned short DID)
{
	switch (DID) {
	case DID_AMD_K8_NB_MEMCTL:
		AMD_0Fh_HTT(Shm, RO(Proc));
		AMD_0Fh_MCH(Shm, RO(Proc));
		SET_CHIPSET(IC_K8);
		break;
	case DID_AMD_17H_ZEN_PLUS_NB_IOMMU:
	case DID_AMD_17H_ZEPPELIN_NB_IOMMU:
	case DID_AMD_17H_RAVEN_NB_IOMMU:
	case DID_AMD_17H_ZEN2_MTS_NB_IOMMU:
	case DID_AMD_17H_STARSHIP_NB_IOMMU:
	case DID_AMD_17H_RENOIR_NB_IOMMU:
	case DID_AMD_17H_ZEN_APU_NB_IOMMU:
	case DID_AMD_17H_ZEN2_APU_NB_IOMMU:
		AMD_17h_IOMMU(Shm, RO(Proc));
		break;
	case DID_AMD_17H_ZEPPELIN_DF_UMC:
	case DID_AMD_17H_RAVEN_DF_UMC:
	case DID_AMD_17H_MATISSE_DF_UMC:
	case DID_AMD_17H_STARSHIP_DF_UMC:
	case DID_AMD_17H_RENOIR_DF_UMC:
	case DID_AMD_17H_ARIEL_DF_UMC:
	case DID_AMD_17H_FIREFLIGHT_DF_UMC:
	case DID_AMD_17H_ARDEN_DF_UMC:
		AMD_17h_UMC(Shm, RO(Proc));
		AMD_17h_CAP(Shm, RO(Proc), RO(Core));
		SET_CHIPSET(IC_ZEN);
		break;
	}
}

#undef SET_CHIPSET

void Uncore_Update(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned int idx;
	/* Copy the # of controllers.					*/
	Shm->Uncore.CtrlCount = RO(Proc)->Uncore.CtrlCount;
	/* Decode the Memory Controller for each found vendor:device	*/
	Chipset[IC_CHIPSET] = RO(Proc)->Features.Info.Vendor.ID;
	Shm->Uncore.ChipID = 0x0;
	Shm->Uncore.Chipset.ArchID = IC_CHIPSET;

  for (idx = 0; idx < CHIP_MAX_PCI; idx++) {
    switch (RO(Proc)->Uncore.Chip[idx].VID) {
    case PCI_VENDOR_ID_INTEL:
	PCI_Intel(Shm, RO(Proc), RO(Core), RO(Proc)->Uncore.Chip[idx].DID);
	break;
    case PCI_VENDOR_ID_AMD:
	PCI_AMD(Shm, RO(Proc), RO(Core), RO(Proc)->Uncore.Chip[idx].DID);
	break;
    }
  }
	/* Copy the chipset codename.					*/
	StrCopy(Shm->Uncore.Chipset.CodeName,
		Chipset[Shm->Uncore.Chipset.ArchID],
		CODENAME_LEN);
	/* Copy the Uncore clock ratios.				*/
	memcpy( Shm->Uncore.Boost,
		RO(Proc)->Uncore.Boost,
		(UNCORE_BOOST(SIZE)) * sizeof(unsigned int) );
}

void CPUID_Dump(SHM_STRUCT *Shm, RO(CORE) **RO(Core), unsigned int cpu)
{	/* Copy the Vendor CPUID dump per Core.				*/
	Shm->Cpu[cpu].Query.StdFunc = RO(Core, AT(cpu))->Query.StdFunc;
	Shm->Cpu[cpu].Query.ExtFunc = RO(Core, AT(cpu))->Query.ExtFunc;

	enum CPUID_ENUM i;
    for (i = 0; i < CPUID_MAX_FUNC; i++) {
	Shm->Cpu[cpu].CpuID[i].func   = RO(Core, AT(cpu))->CpuID[i].func;
	Shm->Cpu[cpu].CpuID[i].sub    = RO(Core, AT(cpu))->CpuID[i].sub;
	Shm->Cpu[cpu].CpuID[i].reg[0] = RO(Core, AT(cpu))->CpuID[i].reg[0];
	Shm->Cpu[cpu].CpuID[i].reg[1] = RO(Core, AT(cpu))->CpuID[i].reg[1];
	Shm->Cpu[cpu].CpuID[i].reg[2] = RO(Core, AT(cpu))->CpuID[i].reg[2];
	Shm->Cpu[cpu].CpuID[i].reg[3] = RO(Core, AT(cpu))->CpuID[i].reg[3];
    }
}

unsigned int AMD_L2_L3_Way_Associativity(RO(CORE) **RO(Core),
					unsigned int cpu,
					unsigned int level)
{
	switch (RO(Core, AT(cpu))->T.Cache[level].Way) {
	case 0x9:
		return ((RO(Core, AT(cpu))->CpuID[
			CPUID_8000001D_00000000_CACHE_L1D_PROPERTIES + level
			].reg[1] >> 22) + 1);
	case 0x6:
		return 8;
	case 0x8:
		return 16;
	case 0xa:
		return 32;
	case 0xb:
		return 48;
	case 0xc:
		return 64;
	case 0xd:
		return 96;
	case 0xe:
		return 128;
	default:
		return RO(Core, AT(cpu))->T.Cache[level].Way;
	}
}

void Topology(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) **RO(Core),
		unsigned int cpu)
{
	unsigned int loop;
	/* Copy each Core topology.					*/
	Shm->Cpu[cpu].Topology.MP.BSP     =(RO(Core, AT(cpu))->T.Base.BSP)? 1:0;
	Shm->Cpu[cpu].Topology.ApicID     = RO(Core, AT(cpu))->T.ApicID;
	Shm->Cpu[cpu].Topology.CoreID     = RO(Core, AT(cpu))->T.CoreID;
	Shm->Cpu[cpu].Topology.ThreadID   = RO(Core, AT(cpu))->T.ThreadID;
	Shm->Cpu[cpu].Topology.PackageID  = RO(Core, AT(cpu))->T.PackageID;
	Shm->Cpu[cpu].Topology.Cluster.ID = RO(Core, AT(cpu))->T.Cluster.ID;
	/* x2APIC capability.						*/
	Shm->Cpu[cpu].Topology.MP.x2APIC  = RO(Proc)->Features.Std.ECX.x2APIC;
    if ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    ||	(Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	Shm->Cpu[cpu].Topology.MP.x2APIC |= \
				RO(Proc)->Features.ExtInfo.ECX.ExtApicId;
    }
	/* Is local APIC enabled in xAPIC mode ?			*/
	Shm->Cpu[cpu].Topology.MP.x2APIC &= RO(Core, AT(cpu))->T.Base.APIC_EN;
	/* Is xAPIC enabled in x2APIC mode ?				*/
	Shm->Cpu[cpu].Topology.MP.x2APIC = Shm->Cpu[cpu].Topology.MP.x2APIC
					<< RO(Core, AT(cpu))->T.Base.x2APIC_EN;
	/* Aggregate the Caches topology.				*/
    for (loop = 0; loop < CACHE_MAX_LEVEL; loop++)
    {
      if (RO(Core, AT(cpu))->T.Cache[loop].Type > 0)
      {
	unsigned int level = RO(Core, AT(cpu))->T.Cache[loop].Level;
	if (RO(Core, AT(cpu))->T.Cache[loop].Type == 2) {/* Instruction	*/
		level = 0;
	}
	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
		Shm->Cpu[cpu].Topology.Cache[level].Set = \
				RO(Core, AT(cpu))->T.Cache[loop].Set + 1;

		Shm->Cpu[cpu].Topology.Cache[level].LineSz = \
				RO(Core, AT(cpu))->T.Cache[loop].LineSz + 1;

		Shm->Cpu[cpu].Topology.Cache[level].Part = \
				RO(Core, AT(cpu))->T.Cache[loop].Part + 1;

		Shm->Cpu[cpu].Topology.Cache[level].Way = \
				RO(Core, AT(cpu))->T.Cache[loop].Way + 1;

		Shm->Cpu[cpu].Topology.Cache[level].Size = \
				  Shm->Cpu[cpu].Topology.Cache[level].Set
				* Shm->Cpu[cpu].Topology.Cache[level].LineSz
				* Shm->Cpu[cpu].Topology.Cache[level].Part
				* Shm->Cpu[cpu].Topology.Cache[level].Way;
	} else {
	    if ((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	     || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
	    {
		Shm->Cpu[cpu].Topology.Cache[level].Way = \
			(loop == 2) || (loop == 3) ?
				AMD_L2_L3_Way_Associativity(RO(Core), cpu, loop)
				: RO(Core, AT(cpu))->T.Cache[loop].Way;

		Shm->Cpu[cpu].Topology.Cache[level].Size = \
					RO(Core, AT(cpu))->T.Cache[loop].Size;
	    }
	}
	Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack = \
					RO(Core, AT(cpu))->T.Cache[loop].WrBack;

	Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive = \
					RO(Core, AT(cpu))->T.Cache[loop].Inclus;
      }
    }
	/* Apply various architecture size unit.			*/
    switch (RO(Proc)->ArchID) {
    case AMD_Family_15h:
	/*TODO: do models 60h & 70h need a 512 KB size unit adjustment ? */
	if ((Shm->Proc.Features.Std.EAX.ExtModel == 0x6)
	 || (Shm->Proc.Features.Std.EAX.ExtModel == 0x7)) {
		break;
	}
	fallthrough;
    case AMD_Zen:
    case AMD_Zen_APU:
    case AMD_ZenPlus:
    case AMD_ZenPlus_APU:
    case AMD_Zen_Dali:
    case AMD_EPYC_Rome_CPK:
    case AMD_Zen2_Renoir:
    case AMD_Zen2_LCN:
    case AMD_Zen2_MTS:
    case AMD_Zen2_Ariel:
    case AMD_Zen3_VMR:
    case AMD_Zen3_CZN:
    case AMD_EPYC_Milan:
    case AMD_Zen3_Chagall:
    case AMD_Family_17h:
    case AMD_Family_18h:
    case AMD_Family_19h:
    VIRTUALIZED_L3:
	/* CPUID_Fn80000006_EDX: Value in [3FFFh - 0001h] = (<Value> *0.5) MB */
	Shm->Cpu[cpu].Topology.Cache[3].Size *= 512;
	break;
    default:
	if ( Shm->Proc.Features.Std.ECX.Hyperv ) { /*	Virtualized ?	*/
		goto VIRTUALIZED_L3;
	}
	break;
    }
}

void CStates(SHM_STRUCT *Shm, RO(CORE) **RO(Core), unsigned int cpu)
{	/* Copy the C-State Configuration Control			*/
	Shm->Cpu[cpu].Query.CfgLock = RO(Core, AT(cpu))->Query.CfgLock;
	Shm->Cpu[cpu].Query.CStateLimit = RO(Core, AT(cpu))->Query.CStateLimit;
	/* Copy Intel Max C-State Inclusion				*/
	Shm->Cpu[cpu].Query.IORedir = RO(Core, AT(cpu))->Query.IORedir;

	Shm->Cpu[cpu].Query.CStateInclude = \
					RO(Core, AT(cpu))->Query.CStateInclude;
	/* Copy any architectural C-States I/O Base Address		*/
	Shm->Cpu[cpu].Query.CStateBaseAddr = \
					RO(Core, AT(cpu))->Query.CStateBaseAddr;
}

void PowerThermal(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) **RO(Core),
			unsigned int cpu)
{
	Shm->Cpu[cpu].PowerThermal.DutyCycle.Extended = \
		RO(Core, AT(cpu))->PowerThermal.ClockModulation.ECMD;

	Shm->Cpu[cpu].PowerThermal.DutyCycle.ClockMod = \
		RO(Core, AT(cpu))->PowerThermal.ClockModulation.DutyCycle
			>> !Shm->Cpu[cpu].PowerThermal.DutyCycle.Extended;

	Shm->Cpu[cpu].PowerThermal.PowerPolicy = \
		RO(Core, AT(cpu))->PowerThermal.PerfEnergyBias.PowerPolicy;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Highest = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Highest;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Guaranteed = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Guaranteed;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Most_Efficient = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Most_Efficient;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Lowest = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Capabilities.Lowest;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Minimum_Perf;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Maximum_Perf;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Desired_Perf;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Energy_Pref = \
		RO(Core, AT(cpu))->PowerThermal.HWP_Request.Energy_Pref;
}

void SystemRegisters(SHM_STRUCT *Shm, RO(CORE) **RO(Core), unsigned int cpu)
{
  Shm->Cpu[cpu].SystemRegister.RFLAGS= RO(Core, AT(cpu))->SystemRegister.RFLAGS;
  Shm->Cpu[cpu].SystemRegister.CR0   = RO(Core, AT(cpu))->SystemRegister.CR0;
  Shm->Cpu[cpu].SystemRegister.CR3   = RO(Core, AT(cpu))->SystemRegister.CR3;
  Shm->Cpu[cpu].SystemRegister.CR4   = RO(Core, AT(cpu))->SystemRegister.CR4;
  Shm->Cpu[cpu].SystemRegister.CR8   = RO(Core, AT(cpu))->SystemRegister.CR8;
  Shm->Cpu[cpu].SystemRegister.EFER  = RO(Core, AT(cpu))->SystemRegister.EFER;
  Shm->Cpu[cpu].SystemRegister.EFCR  = RO(Core, AT(cpu))->SystemRegister.EFCR;
}

void SysGate_OS_Driver(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	RO(PROC) *RO(Proc) = Ref->RO(Proc);

	memset(&Shm->SysGate.OS, 0, sizeof(OS_DRIVER));
    if (strlen(RO(Proc)->OS.IdleDriver.Name) > 0) {
	int idx;

	StrCopy(Shm->SysGate.OS.IdleDriver.Name,
		RO(Proc)->OS.IdleDriver.Name,
		CPUIDLE_NAME_LEN);

    Shm->SysGate.OS.IdleDriver.stateCount = RO(Proc)->OS.IdleDriver.stateCount;
    Shm->SysGate.OS.IdleDriver.stateLimit = RO(Proc)->OS.IdleDriver.stateLimit;

	for (idx = 0; idx < Shm->SysGate.OS.IdleDriver.stateCount; idx++)
	{
		StrCopy(Shm->SysGate.OS.IdleDriver.State[idx].Name,
			RO(Proc)->OS.IdleDriver.State[idx].Name,
			CPUIDLE_NAME_LEN);

		StrCopy(Shm->SysGate.OS.IdleDriver.State[idx].Desc,
			RO(Proc)->OS.IdleDriver.State[idx].Desc,
			CPUIDLE_NAME_LEN);

		Shm->SysGate.OS.IdleDriver.State[idx].exitLatency = \
			RO(Proc)->OS.IdleDriver.State[idx].exitLatency;

		Shm->SysGate.OS.IdleDriver.State[idx].powerUsage = \
			RO(Proc)->OS.IdleDriver.State[idx].powerUsage;

		Shm->SysGate.OS.IdleDriver.State[idx].targetResidency = \
			RO(Proc)->OS.IdleDriver.State[idx].targetResidency;
	}
    }
    if (strlen(RO(Proc)->OS.FreqDriver.Name) > 0) {
	StrCopy(Shm->SysGate.OS.FreqDriver.Name,
		RO(Proc)->OS.FreqDriver.Name,
		CPUFREQ_NAME_LEN);
    }
    if (strlen(RO(Proc)->OS.FreqDriver.Governor) > 0) {
	StrCopy(Shm->SysGate.OS.FreqDriver.Governor,
		RO(Proc)->OS.FreqDriver.Governor,
		CPUFREQ_NAME_LEN);
    }
}

void SysGate_Kernel(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	RO(SYSGATE) *SysGate = Ref->RO(SysGate);

	Shm->SysGate.kernel.version = SysGate->kernelVersionNumber >> 16;
	Shm->SysGate.kernel.major = (SysGate->kernelVersionNumber >> 8) & 0xff;
	Shm->SysGate.kernel.minor = SysGate->kernelVersionNumber & 0xff;

	memcpy(Shm->SysGate.sysname, SysGate->sysname, MAX_UTS_LEN);
	memcpy(Shm->SysGate.release, SysGate->release, MAX_UTS_LEN);
	memcpy(Shm->SysGate.version, SysGate->version, MAX_UTS_LEN);
	memcpy(Shm->SysGate.machine, SysGate->machine, MAX_UTS_LEN);
}

static const int reverseSign[2] = {+1, -1};

static int SortByRuntime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->runtime < task2->runtime ? +1 : -1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return sort;
}

static int SortByUsertime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->usertime < task2->usertime ? +1 : -1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return sort;
}

static int SortBySystime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->systime < task2->systime ? +1 : -1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return sort;
}

static int SortByState(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->state < task2->state ? -1 : +1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return sort;
}

static int SortByPID(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->pid < task2->pid ? -1 : +1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return sort;
}

static int SortByCommand(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = strncmp(task1->comm, task2->comm, TASK_COMM_LEN);
	sort *= reverseSign[Shm->SysGate.reverseOrder];
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
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;

	int sort = (task1->pid == Shm->SysGate.trackTask) ?
		-1 : (task2->pid == Shm->SysGate.trackTask) ?
		+1 :  SortByFunc[Shm->SysGate.sortByField](p1, p2, Shm);
	return sort;
}

void SysGate_Update(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	RO(SYSGATE) *SysGate = Ref->RO(SysGate);
	RO(PROC) *RO(Proc) = Ref->RO(Proc);

	Shm->SysGate.taskCount = SysGate->taskCount;

	memcpy( Shm->SysGate.taskList, SysGate->taskList,
		(size_t) Shm->SysGate.taskCount * sizeof(TASK_MCB));

	qsort_r(Shm->SysGate.taskList,
		(size_t) Shm->SysGate.taskCount, sizeof(TASK_MCB),
		Shm->SysGate.trackTask ?
			  SortByTracker
			: SortByFunc[Shm->SysGate.sortByField], Shm);

	Shm->SysGate.memInfo.totalram  = SysGate->memInfo.totalram;
	Shm->SysGate.memInfo.sharedram = SysGate->memInfo.sharedram;
	Shm->SysGate.memInfo.freeram   = SysGate->memInfo.freeram;
	Shm->SysGate.memInfo.bufferram = SysGate->memInfo.bufferram;
	Shm->SysGate.memInfo.totalhigh = SysGate->memInfo.totalhigh;
	Shm->SysGate.memInfo.freehigh  = SysGate->memInfo.freehigh;
	Shm->SysGate.OS.IdleDriver.stateLimit =
				RO(Proc)->OS.IdleDriver.stateLimit;
}

void PerCore_Update(SHM_STRUCT *Shm, RO(PROC) *RO(Proc), RO(CORE) **RO(Core),
			unsigned int cpu )
{
	if (BITVAL(RO(Core, AT(cpu))->OffLine, HW)) {
		BITSET(LOCKLESS, Shm->Cpu[cpu].OffLine, HW);
	} else {
		BITCLR(LOCKLESS, Shm->Cpu[cpu].OffLine, HW);
	}
	/* Initialize all clock ratios.					*/
	memcpy( Shm->Cpu[cpu].Boost, RO(Core, AT(cpu))->Boost,
		(BOOST(SIZE)) * sizeof(unsigned int) );

	Shm->Cpu[cpu].Query.Microcode = RO(Core, AT(cpu))->Query.Microcode;

	Topology(Shm, RO(Proc), RO(Core), cpu);

	CStates(Shm, RO(Core), cpu);

	PowerThermal(Shm, RO(Proc), RO(Core), cpu);

	SystemRegisters(Shm, RO(Core), cpu);

	CPUID_Dump(Shm, RO(Core), cpu);
}

int SysGate_OnDemand(REF *Ref, int operation)
{
	int rc = -1;
	const size_t allocPages = \
			(size_t)PAGE_SIZE << Ref->RO(Proc)->Gate.ReqMem.Order;

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
						Ref->fd->Drv, vm_pgoff);
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
	if (BITWISEAND(LOCKLESS, Ref->Shm->SysGate.Operation, 0x1)) {
		/* Stop SysGate 					*/
		BITCLR(LOCKLESS, Ref->Shm->SysGate.Operation, 0);
		/* Notify						*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
	}
    } else {
	if (!BITWISEAND(LOCKLESS, Ref->Shm->SysGate.Operation, 0x1)) {
	    if (SysGate_OnDemand(Ref, 1) == 0) {
		if (ioctl(Ref->fd->Drv, COREFREQ_IOCTL_SYSONCE) != -EPERM) {
			/* Aggregate the OS idle driver data.		*/
			SysGate_OS_Driver(Ref);
			/* Copy system information.			*/
			SysGate_Kernel(Ref);
			/* Start SysGate				*/
			BITSET(LOCKLESS, Ref->Shm->SysGate.Operation, 0);
			/* Notify					*/
			BITWISESET(LOCKLESS, PendingSync,BIT_MASK_NTFY);
		}
	    }
	}
    }
}

void Master_Ring_Handler(REF *Ref, unsigned int rid)
{
    if (!RING_NULL(Ref->Shm->Ring[rid]))
    {
	RING_CTRL ctrl __attribute__ ((aligned(16)));
	RING_READ(Ref->Shm->Ring[rid], ctrl);
	int rc = -EPERM, drc = -EPERM;

	if (	(ctrl.cmd >= COREFREQ_IOCTL_SYSUPDT)
	&&	(ctrl.cmd <= COREFREQ_IOCTL_CLEAR_EVENTS) )
	{
		rc = ioctl(Ref->fd->Drv, ctrl.cmd, ctrl.arg);
		drc = errno;
	}
	if (Quiet & 0x100) {
		printf("\tRING[%u](%x,%x)(%lx)>(%d,%d)\n",
			rid, ctrl.cmd, ctrl.sub, ctrl.arg, rc, drc);
	}
	switch (rc) {
	case -EPERM:
	    {
		RING_CTRL error __attribute__ ((aligned(16))) = {
			.arg = ctrl.arg,
			.cmd = ctrl.cmd,
			.drc = (unsigned int) drc,
			.tds = ELAPSED(Ref->Shm->StartedAt)
		};
		RING_WRITE_1xPARAM(	error.sub,
					Ref->Shm->Error,
					error.cmd,
					error.arg );
	    }
		break;
	case RC_OK_SYSGATE:
		SysGate_OS_Driver(Ref);
		fallthrough;
	case RC_SUCCESS: /* Platform changed -> pending notification.	*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
		break;
	case RC_OK_COMPUTE: /* Compute claimed -> pending notification.	*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_COMP);
		break;
	}
    }
}

void Child_Ring_Handler(REF *Ref, unsigned int rid)
{
  if (!RING_NULL(Ref->Shm->Ring[rid]))
  {
	RING_CTRL ctrl __attribute__ ((aligned(16)));
	RING_READ(Ref->Shm->Ring[rid], ctrl);

   switch (ctrl.cmd) {
   case COREFREQ_TOGGLE_SYSGATE:
	switch (ctrl.arg) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		SysGate_Toggle(Ref, ctrl.arg);
		BITWISESET(LOCKLESS, Ref->Shm->Proc.Sync, BIT_MASK_NTFY);
		break;
	}
	break;
   case COREFREQ_ORDER_MACHINE:
	switch (ctrl.arg) {
	case COREFREQ_TOGGLE_OFF:
	SCHEDULER_STOP:
	    if (BITVAL(Ref->Shm->Proc.Sync, BURN))
	    {
		BITCLR(BUS_LOCK, Ref->Shm->Proc.Sync, BURN);

		while (BITWISEAND_CC(BUS_LOCK, roomCore, roomSeed))
		{
			if (BITVAL(Shutdown, SYNC)) {	/* SpinLock */
				break;
			}
		}
		BITSTOR_CC(BUS_LOCK, Ref->Shm->roomSched, roomClear);

		Ref->Slice.Func = Slice_NOP;
		Ref->Slice.arg = 0;
		Ref->Slice.pattern = RESET_CSP;
		/* Notify the Slice module has stopped			*/
		BITWISESET(LOCKLESS, Ref->Shm->Proc.Sync, BIT_MASK_NTFY);
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
       if ( !BITVAL(Ref->Shm->Proc.Sync, BURN)
	|| ((Ref->Slice.Func == Slice_Turbo) && (Ref->Slice.pattern == USR_CPU)
		&& (ctrl.cmd == COREFREQ_ORDER_TURBO) && (ctrl.sub == USR_CPU)))
       {
	if (ctrl.sub == USR_CPU) {
		if (BITVAL_CC(Ref->Shm->roomSched, ctrl.dl.lo))
		{
			BITCLR_CC(LOCKLESS, Ref->Shm->roomSched, ctrl.dl.lo);

		    if (!BITWISEAND_CC(BUS_LOCK, Ref->Shm->roomSched, roomSeed))
		    {
			goto SCHEDULER_STOP;
		    }
		} else {
			BITSET_CC(LOCKLESS, Ref->Shm->roomSched, ctrl.dl.lo);

			goto SCHEDULER_START;
		}
	} else {
		SliceScheduling(Ref->Shm, ctrl.dl.lo, porder->pattern);

	SCHEDULER_START:
		Ref->Slice.Func = porder->func;
		Ref->Slice.arg  = porder->ctrl.dl.lo;
		Ref->Slice.pattern = porder->pattern;

		BITSET(BUS_LOCK, Ref->Shm->Proc.Sync, BURN);
	}
	/* Notify the Slice module is starting up			*/
	BITWISESET(LOCKLESS, Ref->Shm->Proc.Sync, BIT_MASK_NTFY);
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
			CPU_SET((unsigned int) pSlave->Thread, &cpuset);
		}
		return pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
	}
	return -1;
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
	handlerName[14] += (char) rid;

	pthread_t tid = pthread_self();

    if(ServerFollowService(&localService, &Ref->Shm->Proc.Service, tid) == 0) {
	pthread_setname_np(tid, handlerName);
    }
    while (!leave) {
	caught = sigtimedwait(&Ref->Signal, NULL, &Ref->Shm->Sleep.ringWaiting);
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
	ServerFollowService(&localService, &Ref->Shm->Proc.Service, tid);
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

static void Pkg_ComputeThermal_Intel(	struct PKG_FLIP_FLOP *PFlip,
					struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(INTEL,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static void Pkg_ComputeThermal_AMD(	struct PKG_FLIP_FLOP *PFlip,
					struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static void Pkg_ComputeThermal_AMD_0Fh( struct PKG_FLIP_FLOP *PFlip,
					struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD_0Fh,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static void Pkg_ComputeThermal_AMD_15h( struct PKG_FLIP_FLOP *PFlip,
					struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD_15h,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static void Pkg_ComputeThermal_AMD_17h( struct PKG_FLIP_FLOP *PFlip,
					struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD_17h,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static void Pkg_ComputeVoltage_None(struct PKG_FLIP_FLOP *PFlip)
{
	UNUSED(PFlip);
}

#define Pkg_ComputeVoltage_Intel	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_Intel_Core2	Pkg_ComputeVoltage_None

static void Pkg_ComputeVoltage_Intel_SoC(struct PKG_FLIP_FLOP *PFlip)
{
	COMPUTE_VOLTAGE(INTEL_SOC,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);
}

static void Pkg_ComputeVoltage_Intel_SNB(struct PKG_FLIP_FLOP *PFlip)
{	/* Intel 2nd Generation Datasheet Vol-1 §7.4 Table 7-1		*/
	COMPUTE_VOLTAGE(INTEL_SNB,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);
}

#define Pkg_ComputeVoltage_Intel_SKL_X	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD		Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD_0Fh	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD_15h	Pkg_ComputeVoltage_None

static void Pkg_ComputeVoltage_AMD_17h(struct PKG_FLIP_FLOP *PFlip)
{
	COMPUTE_VOLTAGE(AMD_17h,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);

	COMPUTE_VOLTAGE(AMD_17h,
			PFlip->Voltage.SOC,
			PFlip->Voltage.VID.SOC);
}

static void Pkg_ComputeVoltage_Winbond_IO(struct PKG_FLIP_FLOP *PFlip)
{	/* Winbond W83627EHF/EF, W83627EHG,EG				*/
	COMPUTE_VOLTAGE(WINBOND_IO,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);
}

static void Pkg_ComputeVoltage_ITE_Tech_IO(struct PKG_FLIP_FLOP *PFlip)
{
	COMPUTE_VOLTAGE(ITETECH_IO,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);
}

static void Pkg_ComputePower_None(RW(PROC) *RW(Proc), struct FLIP_FLOP *CFlop)
{
	UNUSED(RW(Proc));
	UNUSED(CFlop);
}

#define Pkg_ComputePower_Intel		Pkg_ComputePower_None

#define Pkg_ComputePower_Intel_Atom	Pkg_ComputePower_None

#define Pkg_ComputePower_AMD		Pkg_ComputePower_None

static void Pkg_ComputePower_AMD_17h(	RW(PROC) *RW(Proc),
					struct FLIP_FLOP *CFlop )
{
    RW(Proc)->Delta.Power.ACCU[PWR_DOMAIN(CORES)] += CFlop->Delta.Power.ACCU;
}

static void Pkg_ResetPower_None(PROC_RW *Proc)
{
	UNUSED(Proc);
}

#define Pkg_ResetPower_Intel		Pkg_ResetPower_None

#define Pkg_ResetPower_Intel_Atom	Pkg_ResetPower_None

#define Pkg_ResetPower_AMD		Pkg_ResetPower_None

static void Pkg_ResetPower_AMD_17h(RW(PROC) *RW(Proc))
{
	RW(Proc)->Delta.Power.ACCU[PWR_DOMAIN(CORES)] = 0;
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
	SHM_STRUCT		*Shm = Ref->Shm;
	RO(PROC)		*RO(Proc) = Ref->RO(Proc);
	RW(PROC)		*RW(Proc) = Ref->RW(Proc);
	RO(CORE)		**RO(Core)= Ref->RO(Core);
	struct PKG_FLIP_FLOP	*PFlip;
	struct FLIP_FLOP	*SProc;
	SERVICE_PROC		localService = {.Proc = -1};
	struct {
		double		RelFreq,
				AbsFreq;
	} prevTop;
	REASON_INIT		(reason);
	unsigned int		cpu = 0;

	pthread_t tid = pthread_self();
	Shm->App.Svr = tid;

	if (ServerFollowService(&localService, &Shm->Proc.Service, tid) == 0) {
		pthread_setname_np(tid, "corefreqd-pmgr");
	}
	ARG *Arg = calloc(Shm->Proc.CPU.Count, sizeof(ARG));
  if (Arg != NULL)
  {
	void (*Pkg_ComputeThermalFormula)(	struct PKG_FLIP_FLOP*,
						struct FLIP_FLOP* );

	void (*Pkg_ComputeVoltageFormula)( struct PKG_FLIP_FLOP* );

	void (*Pkg_ComputePowerFormula)( PROC_RW*, struct FLIP_FLOP* );

	void (*Pkg_ResetPowerFormula)(PROC_RW*);

	switch (KIND_OF_FORMULA(Shm->Proc.thermalFormula)) {
	case THERMAL_KIND_INTEL:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_Intel;
		break;
	case THERMAL_KIND_AMD:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_AMD;
		break;
	case THERMAL_KIND_AMD_0Fh:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_AMD_0Fh;
		break;
	case THERMAL_KIND_AMD_15h:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_AMD_15h;
		break;
	case THERMAL_KIND_AMD_17h:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_AMD_17h;
		break;
	case THERMAL_KIND_NONE:
	default:
		Pkg_ComputeThermalFormula = Pkg_ComputeThermal_None;
	}

	switch (KIND_OF_FORMULA(Shm->Proc.voltageFormula)) {
	case VOLTAGE_KIND_INTEL:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Intel;
		break;
	case VOLTAGE_KIND_INTEL_CORE2:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Intel_Core2;
		break;
	case VOLTAGE_KIND_INTEL_SOC:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Intel_SoC;
		break;
	case VOLTAGE_KIND_INTEL_SNB:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Intel_SNB;
		break;
	case VOLTAGE_KIND_INTEL_SKL_X:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Intel_SKL_X;
		break;
	case VOLTAGE_KIND_AMD:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_AMD;
		break;
	case VOLTAGE_KIND_AMD_0Fh:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_AMD_0Fh;
		break;
	case VOLTAGE_KIND_AMD_15h:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_AMD_15h;
		break;
	case VOLTAGE_KIND_AMD_17h:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_AMD_17h;
		break;
	case VOLTAGE_KIND_WINBOND_IO:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Winbond_IO;
		break;
	case VOLTAGE_KIND_ITETECH_IO:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_ITE_Tech_IO;
		break;
	case VOLTAGE_KIND_NONE:
	default:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_None;
		break;
	}

	switch (KIND_OF_FORMULA(Shm->Proc.powerFormula)) {
	case POWER_KIND_INTEL:
		Pkg_ComputePowerFormula = Pkg_ComputePower_Intel;
		Pkg_ResetPowerFormula	= Pkg_ResetPower_Intel;
		break;
	case POWER_KIND_INTEL_ATOM:
		Pkg_ComputePowerFormula = Pkg_ComputePower_Intel_Atom;
		Pkg_ResetPowerFormula	= Pkg_ResetPower_Intel_Atom;
		break;
	case POWER_KIND_AMD:
		Pkg_ComputePowerFormula = Pkg_ComputePower_AMD;
		Pkg_ResetPowerFormula	= Pkg_ResetPower_AMD;
		break;
	case POWER_KIND_AMD_17h:
		Pkg_ComputePowerFormula = Pkg_ComputePower_AMD_17h;
		Pkg_ResetPowerFormula	= Pkg_ResetPower_AMD_17h;
		break;
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
	(  (RO(Proc)->Features.AdvPower.EDX.Inv_TSC == 1)		\
	|| (RO(Proc)->Features.ExtInfo.EDX.RDTSCP == 1) )

    #define CONDITION_RDPMC()						\
	(  (RO(Proc)->Features.Info.Vendor.CRC == CRC_INTEL)		\
	&& (RO(Proc)->Features.PerfMon.EAX.Version >= 1)		\
	&& (BITVAL(RO(Core, AT(RO(Proc)->Service.Core))->SystemRegister.CR4, \
							CR4_PCE) == 1) )

	UBENCH_SETUP(CONDITION_RDTSCP(), CONDITION_RDPMC());
	Print_uBenchmark((Quiet & 0x100));

    while (!BITVAL(Shutdown, SYNC))
    {	/* Loop while all the cpu room bits are not cleared.		*/
	while (!BITVAL(Shutdown, SYNC) && !ROOM_CLEAR(room))
	{
		nanosleep(&Shm->Sleep.pollingWait, NULL);
	}

	UBENCH_RDCOUNTER(1);

	Shm->Proc.Toggle = !Shm->Proc.Toggle;
	PFlip = &Shm->Proc.FlipFlop[Shm->Proc.Toggle];

	SProc = &Shm->Cpu[Shm->Proc.Service.Core].FlipFlop[ \
				!Shm->Cpu[Shm->Proc.Service.Core].Toggle
	];

	PFlip->Thermal.Temp = 0;
	/* Reset the averages & the max frequency			*/
	Shm->Proc.Avg.Turbo = 0;
	Shm->Proc.Avg.C0    = 0;
	Shm->Proc.Avg.C3    = 0;
	Shm->Proc.Avg.C6    = 0;
	Shm->Proc.Avg.C7    = 0;
	Shm->Proc.Avg.C1    = 0;

	prevTop.RelFreq = 0.0;
	prevTop.AbsFreq = 0.0;

	Pkg_ResetPowerFormula(RW(Proc));

	for (cpu=0; !BITVAL(Shutdown, SYNC)&&(cpu < Shm->Proc.CPU.Count);cpu++)
	{
	    if (BITVAL(RO(Core, AT(cpu))->OffLine, OS) == 1)
	    {
		if (Arg[cpu].TID)
		{	/* Remove this cpu.				*/
		  if (pthread_join(Arg[cpu].TID, NULL) == 0)
		  {
			BITCLR_CC(BUS_LOCK, roomReady, cpu);
			Arg[cpu].TID = 0;

			PerCore_Update(Shm, RO(Proc), RO(Core), cpu);

		    if (ServerFollowService(	&localService,
						&Shm->Proc.Service,
						tid ) == 0)
		    {
			SProc = &Shm->Cpu[Shm->Proc.Service.Core].FlipFlop[ \
				!Shm->Cpu[Shm->Proc.Service.Core].Toggle ];
		    }
			/* Raise these bits up to notify a platform change. */
			BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
		  }
		}
		BITSET(LOCKLESS, Shm->Cpu[cpu].OffLine, OS);
	    } else {
		struct FLIP_FLOP *CFlop = \
				&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!Arg[cpu].TID)
		{	/* Add this cpu.				*/
			Arg[cpu].Ref  = Ref;
			Arg[cpu].Bind = cpu;
		  if (pthread_create( &Arg[cpu].TID,
					NULL,
					Core_Cycle,
					&Arg[cpu]) == 0)
		  {
			BITSET_CC(BUS_LOCK, roomReady, cpu);

			PerCore_Update(Shm, RO(Proc), RO(Core), cpu);

		    if (ServerFollowService(&localService,
						&Shm->Proc.Service,
						tid) == 0)
		    {
			SProc = &Shm->Cpu[Shm->Proc.Service.Core].FlipFlop[ \
				!Shm->Cpu[Shm->Proc.Service.Core].Toggle ];
		    }
		    if (Quiet & 0x100) {
			printf( "    CPU #%03u @ %.2f MHz\n", cpu,
			  ABS_FREQ_MHz(double , Shm->Cpu[cpu].Boost[BOOST(MAX)],
						RO(Core, AT(cpu))->Clock) );
		    }
			/* Notify a CPU has been brought up		*/
			BITWISESET(LOCKLESS, PendingSync, BIT_MASK_NTFY);
		  }
		}
		BITCLR(LOCKLESS, Shm->Cpu[cpu].OffLine, OS);

		/* Index CPU with the highest Rel. and Abs frequencies.	*/
		if (CFlop->Relative.Freq > prevTop.RelFreq) {
			prevTop.RelFreq = CFlop->Relative.Freq;
			Shm->Proc.Top.Rel = cpu;
		}
		if (CFlop->Absolute.Freq > prevTop.AbsFreq) {
			prevTop.AbsFreq = CFlop->Absolute.Freq;
			Shm->Proc.Top.Abs = cpu;
		}

		/* Workaround to Package Thermal Management: the hottest Core */
		if (!Shm->Proc.Features.Power.EAX.PTM) {
			if (CFlop->Thermal.Temp > PFlip->Thermal.Temp)
				PFlip->Thermal.Temp = CFlop->Thermal.Temp;
		}

		/* Workaround to RAPL Package counter: sum of all Cores */
		Pkg_ComputePowerFormula(RW(Proc), CFlop);

		/* Sum counters.					*/
		Shm->Proc.Avg.Turbo += CFlop->State.Turbo;
		Shm->Proc.Avg.C0    += CFlop->State.C0;
		Shm->Proc.Avg.C3    += CFlop->State.C3;
		Shm->Proc.Avg.C6    += CFlop->State.C6;
		Shm->Proc.Avg.C7    += CFlop->State.C7;
		Shm->Proc.Avg.C1    += CFlop->State.C1;
	    }
	}
	if (!BITVAL(Shutdown, SYNC) && ROOM_READY(room))
	{
		unsigned char fRESET = 0;
		/* Compute the counters averages.			*/
		Shm->Proc.Avg.Turbo /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C0    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C3    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C6    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C7    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C1    /= Shm->Proc.CPU.OnLine;
		/* Package scope counters				*/
		PFlip->Delta.PTSC = RO(Proc)->Delta.PTSC;
		PFlip->Delta.PC02 = RO(Proc)->Delta.PC02;
		PFlip->Delta.PC03 = RO(Proc)->Delta.PC03;
		PFlip->Delta.PC04 = RO(Proc)->Delta.PC04;
		PFlip->Delta.PC06 = RO(Proc)->Delta.PC06;
		PFlip->Delta.PC07 = RO(Proc)->Delta.PC07;
		PFlip->Delta.PC08 = RO(Proc)->Delta.PC08;
		PFlip->Delta.PC09 = RO(Proc)->Delta.PC09;
		PFlip->Delta.PC10 = RO(Proc)->Delta.PC10;
		PFlip->Delta.MC6  = RO(Proc)->Delta.MC6;
		/* Package C-state Residency counters			*/
		Shm->Proc.State.PC02	= (double)PFlip->Delta.PC02
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC03	= (double)PFlip->Delta.PC03
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC04	= (double)PFlip->Delta.PC04
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC06	= (double)PFlip->Delta.PC06
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC07	= (double)PFlip->Delta.PC07
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC08	= (double)PFlip->Delta.PC08
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC09	= (double)PFlip->Delta.PC09
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.PC10	= (double)PFlip->Delta.PC10
					/ (double)PFlip->Delta.PTSC;

		Shm->Proc.State.MC6	= (double)PFlip->Delta.MC6
					/ (double)PFlip->Delta.PTSC;
		/* Uncore scope counters				*/
		PFlip->Uncore.FC0 = RO(Proc)->Delta.Uncore.FC0;
		/* Power & Energy counters				*/
		enum PWR_DOMAIN pw;
	  for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
	  {
		PFlip->Delta.ACCU[pw] = RW(Proc)->Delta.Power.ACCU[pw];

		Shm->Proc.State.Energy[pw].Current = PFlip->Delta.ACCU[pw];
		Shm->Proc.State.Energy[pw].Current *= \
						Shm->Proc.Power.Unit.Joules;

		Shm->Proc.State.Power[pw].Current = \
					Shm->Proc.State.Energy[pw].Current;

		Shm->Proc.State.Power[pw].Current *= 1000.0;
		Shm->Proc.State.Power[pw].Current /= Shm->Sleep.Interval;

		/* Processor scope: computes Min and Max energy consumed. */
		TEST_AND_SET_SENSOR(	ENERGY, LOWEST,
					Shm->Proc.State.Energy[pw].Current,
					Shm->Proc.State.Energy[pw].Limit );

		TEST_AND_SET_SENSOR(	ENERGY, HIGHEST,
					Shm->Proc.State.Energy[pw].Current,
					Shm->Proc.State.Energy[pw].Limit );
		/* Processor scope: computes Min and Max power consumed. */
		TEST_AND_SET_SENSOR(	POWER, LOWEST,
					Shm->Proc.State.Power[pw].Current,
					Shm->Proc.State.Power[pw].Limit );

		TEST_AND_SET_SENSOR(	POWER, HIGHEST,
					Shm->Proc.State.Power[pw].Current,
					Shm->Proc.State.Power[pw].Limit );
	  }
		/* Package thermal formulas				*/
		PFlip->Thermal.Events = RO(Proc)->PowerThermal.Events;

	  if (Shm->Proc.Features.Power.EAX.PTM)
	  {
		PFlip->Thermal.Sensor = RO(Proc)->PowerThermal.Sensor;
		Pkg_ComputeThermalFormula(PFlip, SProc);
	  }
		/* Package Voltage formulas				*/
		PFlip->Voltage.VID.CPU = RO(Proc)->PowerThermal.VID.CPU;
		PFlip->Voltage.VID.SOC = RO(Proc)->PowerThermal.VID.SOC;

		Pkg_ComputeVoltageFormula(PFlip);
		/* Computes the Min Processor voltage.			*/
		TEST_AND_SET_SENSOR( VOLTAGE, LOWEST,	PFlip->Voltage.CPU,
						Shm->Proc.State.Voltage.Limit );
		/* Computes the Max Processor voltage.			*/
		TEST_AND_SET_SENSOR( VOLTAGE, HIGHEST,	PFlip->Voltage.CPU,
						Shm->Proc.State.Voltage.Limit );
		/*
		The Driver tick is bound to the Service Core:
		1- Tasks collection; Tasks count; and Memory usage.
		2- Processor has resumed from Suspend To RAM.
		*/
		Shm->SysGate.tickStep = RO(Proc)->tickStep;
	  if (Shm->SysGate.tickStep == Shm->SysGate.tickReset)
	  {
	    if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
	    {
		if (SysGate_OnDemand(Ref, 1) == 0) {
			SysGate_Update(Ref);
		}
	    }
		/* OS notifications: such as Resumed from Suspend.	*/
	    if ( (fRESET = BITCLR(BUS_LOCK, RW(Proc)->OS.Signal, NTFY )) == 1)
	    {
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_COMP|BIT_MASK_NTFY);
	    }
	  }
	  if (BITWISEAND(LOCKLESS, PendingSync, BIT_MASK_COMP|BIT_MASK_NTFY))
	  {
		Package_Update(Shm, RO(Proc), RW(Proc));

		Uncore_Update(Shm,RO(Proc),RO(Core,AT(RO(Proc)->Service.Core)));

	    for (cpu = 0; cpu < Ref->Shm->Proc.CPU.Count; cpu++)
	    {
	      if (fRESET == 1)
	      {
		Core_ResetSensorLimits(&Shm->Cpu[cpu]);
	      }
	      if (BITVAL(RO(Ref->Core, AT(cpu))->OffLine, OS) == 0)
	      {
		PerCore_Update(Ref->Shm, RO(Ref->Proc), RO(Ref->Core), cpu);
	      }
	    }
	    if (fRESET == 1)
	    {
		Pkg_ResetSensorLimits(&Shm->Proc);
	    }
		Technology_Update(Shm, RO(Proc), RW(Proc));

	    if (ServerFollowService(&localService,
				&Shm->Proc.Service,
				tid) == 0)
	    {
		SProc = &Shm->Cpu[Shm->Proc.Service.Core].FlipFlop[ \
			!Shm->Cpu[Shm->Proc.Service.Core].Toggle ];
	    }
	    if (Quiet & 0x100) {
		printf("\t%s || %s\n",
			BITVAL(PendingSync, NTFY0) ? "NTFY":"....",
			BITVAL(PendingSync, COMP0) ? "COMP":"....");
	    }
	  }
		/* All aggregations done: Notify Clients.		*/
		BITWISESET(LOCKLESS, PendingSync, BIT_MASK_SYNC);
		BITWISESET(LOCKLESS, Shm->Proc.Sync, PendingSync);
		BITWISECLR(LOCKLESS, PendingSync);
	}
	/* Reset the Room mask						*/
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
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
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
	SHM_STRUCT *Shm = Ref->Shm;
	SERVICE_PROC localService = {.Proc = -1};
	REASON_INIT(reason);
	unsigned int cpu = 0;

	pthread_t tid = pthread_self();

	if (ServerFollowService(&localService, &Shm->Proc.Service, tid) == 0) {
		pthread_setname_np(tid, "corefreqd-cmgr");
	}
	ARG *Arg = calloc(Shm->Proc.CPU.Count, sizeof(ARG));
  if (Arg != NULL)
  {
    do {
	for (cpu=0; !BITVAL(Shutdown, SYNC)&&(cpu < Shm->Proc.CPU.Count);cpu++)
	{
	    if (BITVAL(Shm->Cpu[cpu].OffLine, OS) == 1) {
		if (Arg[cpu].TID) {
			/* Remove this child thread.			*/
			pthread_join(Arg[cpu].TID, NULL);
			Arg[cpu].TID = 0;
		}
	    } else {
		if (!Arg[cpu].TID) {
			/* Add this child thread.			*/
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
    }
    while (!BITVAL(Shutdown, SYNC)) ;

	for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
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
			uid_t uid, uid_t gid, mode_t cmask)
{
	unsigned int	cpu = 0;
	RO(CORE)	**RO(Core);
	RW(CORE)	**RW(Core);
	SHM_STRUCT	*Shm = NULL;
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
				fd->Drv, vm_ro_pgoff)) == MAP_FAILED)
	{
		REASON_SET(reason, RC_SHM_MMAP);
	}
	if ((RW(Core, AT(cpu)) = mmap(NULL, Size.RW(Core),
				PROT_READ|PROT_WRITE,
				MAP_SHARED,
				fd->Drv, vm_rw_pgoff)) == MAP_FAILED)
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
    if (reason.rc == RC_SUCCESS) {
	umask(cmask);
    }
    if (reason.rc == RC_SUCCESS)
    {	/* Initialize shared memory.					*/
	const size_t ShmCpuSize = sizeof(CPU_STRUCT) * RO(Proc)->CPU.Count,
			ShmSize = ROUND_TO_PAGES((sizeof(SHM_STRUCT)
				+ ShmCpuSize));

      if ((fd->Svr = shm_open(SHM_FILENAME, O_CREAT|O_TRUNC|O_RDWR,
					 S_IRUSR|S_IWUSR
					|S_IRGRP|S_IWGRP
					|S_IROTH|S_IWOTH)) != -1)
      {
	pid_t CPID = -1;

	if (ftruncate(fd->Svr, (off_t) ShmSize) != -1)
	{
	    if ((Shm = mmap(NULL, ShmSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd->Svr, 0)) != MAP_FAILED)
	    {
		__typeof__ (errno) fork_err = 0;
		/* Clear SHM						*/
		memset(Shm, 0, ShmSize);
		/* Store version footprint into SHM			*/
		SET_FOOTPRINT(Shm->FootPrint,	MAX_FREQ_HZ,
						CORE_COUNT,
						TASK_ORDER,
						COREFREQ_MAJOR,
						COREFREQ_MINOR,
						COREFREQ_REV	);
		/* Reference time the Server is starting at.		*/
		time(&Shm->StartedAt);
		/* Store the daemon gate name.				*/
		StrCopy(Shm->ShmName, SHM_FILENAME, TASK_COMM_LEN);
		/* Initialize the busy wait times.			*/
		Shm->Sleep.ringWaiting  = TIMESPEC(SIG_RING_MS);
		Shm->Sleep.childWaiting = TIMESPEC(CHILD_PS_MS);
		Shm->Sleep.sliceWaiting = TIMESPEC(CHILD_TH_MS);

		REF Ref = {
			.CPID		= -1,
			.KID		= 0,
			.Started	= 0,
			.Slice.Func	= NULL,
			.Slice.arg	= 0,
			.fd		= fd,
			.Shm		= Shm,
			.RO(Proc)	= RO(Proc),
			.RW(Proc)	= RW(Proc),
			.RO(Core)	= RO(Core),
			.RW(Core)	= RW(Core),
			.RO(SysGate)	= NULL
		};
		sigemptyset(&Ref.Signal);

		Package_Update(Shm, RO(Proc), RW(Proc));
		Uncore_Update(Shm,RO(Proc),RO(Core,AT(RO(Proc)->Service.Core)));
		memcpy(&Shm->SMB, &RO(Proc)->SMB, sizeof(SMBIOS_ST));

		/*		Clear notification.			*/
		BITWISECLR(LOCKLESS, Shm->Proc.Sync);

		SysGate_Toggle(&Ref, SysGateStartUp);

		/*	Welcomes with brand and per CPU base clock.	*/
	      if (Quiet & 0x001) {
		printf( "CoreFreq Daemon %s"		\
				"  Copyright (C) 2015-2021 CYRIL INGENIERIE\n",
			COREFREQ_VERSION );
	      }
	      if (Quiet & 0x010) {
		printf( "\n"						\
			"  Processor [%s]\n"				\
			"  Architecture [%s] %u/%u CPU Online.\n",
			Shm->Proc.Brand,
			Shm->Proc.Architecture,
			Shm->Proc.CPU.OnLine,
			Shm->Proc.CPU.Count );
	      }
	      if (Quiet & 0x100) {
		printf( "  SleepInterval(%u), SysGate(%u), %ld tasks\n\n",
			Shm->Sleep.Interval,
			!BITVAL(Shm->SysGate.Operation, 0) ?
				0:Shm->Sleep.Interval * Shm->SysGate.tickReset,
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
			if (Shm->App.Cli) {
				if (kill(Shm->App.Cli, SIGTERM) == -1) {
					REASON_SET(reason, RC_EXEC_ERR);
				}
			}
			if (Shm->App.GUI) {
				if (kill(Shm->App.GUI, SIGTERM) == -1) {
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

		if (munmap(Shm, ShmSize) == -1) {
			REASON_SET(reason, RC_SHM_MMAP);
		}
	    } else {
		REASON_SET(reason, RC_SHM_MMAP);
	    }
	} else {
		REASON_SET(reason, RC_SHM_FILE);
	}
	if (close(fd->Svr) == -1) {
		REASON_SET(reason, RC_SHM_FILE);
	}
	if (CPID != 0) {
	    if (shm_unlink(SHM_FILENAME) == -1) {
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
			"\t-M <octal>\tSet the mode creation mask\n"	\
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
	FD   fd = {0, 0};
	RO(PROC) *RO(Proc) = NULL;	/* Kernel module anchor points. */
	RW(PROC) *RW(Proc) = NULL;
	uid_t uid = 0, gid = 0;
	mode_t cmask = !S_IRUSR|!S_IWUSR|!S_IRGRP|!S_IWGRP|!S_IROTH|!S_IWOTH;

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
			    } else if (sscanf(argv[i] , "%o%c",
							&cmask,
							&trailing) != 1) {
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
	    if ((fd.Drv = open(DRV_FILENAME, O_RDWR|O_SYNC)) != -1)
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
				fd.Drv, vm_pgoff[0])) != MAP_FAILED)
		{
		  if ((RW(Proc) = mmap(NULL, packageSize[1],
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.Drv, vm_pgoff[1])) != MAP_FAILED)
		  {
		    if (CHK_FOOTPRINT(RO(Proc)->FootPrint,MAX_FREQ_HZ,
							CORE_COUNT,
							TASK_ORDER,
							COREFREQ_MAJOR,
							COREFREQ_MINOR,
							COREFREQ_REV))
		    {
			reason = Shm_Manager(&fd, RO(Proc), RW(Proc),
						uid, gid, cmask);

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
				reason = Help(reason, SHM_FILENAME);
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
		if (close(fd.Drv) == -1) {
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
