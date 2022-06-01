/*
 * CoreFreq
 * Copyright (C) 2015-2022 CYRIL INGENIERIE
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

static void ComputeThermal_Intel( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	COMPUTE_THERMAL(INTEL,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeThermal_Intel_PerSMT	ComputeThermal_Intel

static void ComputeThermal_Intel_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_Intel(CFlip, RO(Shm), cpu);
	}
}

static void ComputeThermal_Intel_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeThermal_Intel(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeThermal_Intel_Matrix[4])(	struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_Intel_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_Intel_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_Intel_PerPkg
};

static void ComputeThermal_AMD( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeThermal_AMD_PerSMT	ComputeThermal_AMD

static void ComputeThermal_AMD_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD(CFlip, RO(Shm), cpu);
	}
}

static void ComputeThermal_AMD_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeThermal_AMD(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeThermal_AMD_Matrix[4])( struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_PerPkg
};

static void ComputeThermal_AMD_0Fh( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD_0Fh,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeThermal_AMD_0Fh_PerSMT	ComputeThermal_AMD_0Fh

static void ComputeThermal_AMD_0Fh_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD_0Fh(CFlip, RO(Shm), cpu);
	}
}

static void ComputeThermal_AMD_0Fh_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeThermal_AMD_0Fh(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeThermal_AMD_0Fh_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_0Fh_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_0Fh_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_0Fh_PerPkg
};

static void ComputeThermal_AMD_15h( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
    if (RO(Shm)->Cpu[cpu].Topology.CoreID == 0)
    {
	COMPUTE_THERMAL(AMD_15h,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&RO(Shm)->Cpu[cpu], CFlip);
    }
}

#define ComputeThermal_AMD_15h_PerSMT	ComputeThermal_AMD_15h

static void ComputeThermal_AMD_15h_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (RO(Shm)->Cpu[cpu].Topology.CoreID == 0)	/* Opteron use case */
	{
		ComputeThermal_AMD_15h(CFlip, RO(Shm), cpu);
	}
}

static void ComputeThermal_AMD_15h_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeThermal_AMD_15h(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeThermal_AMD_15h_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_15h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_15h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_15h_PerPkg
};

static void ComputeThermal_AMD_17h( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD_17h,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeThermal_AMD_17h_PerSMT	ComputeThermal_AMD_17h

static void ComputeThermal_AMD_17h_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD_17h(CFlip, RO(Shm), cpu);
	}
}

static void ComputeThermal_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeThermal_AMD_17h(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeThermal_AMD_17h_Matrix[4])(struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
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

#define ComputeVoltage_Intel_Matrix	ComputeVoltage_None_Matrix

static void ComputeVoltage_Intel_Core2( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{	/* Intel Core 2 Extreme Datasheet §3.3-Table 2			*/
	COMPUTE_VOLTAGE(INTEL_CORE2,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_Core2_PerSMT	ComputeVoltage_Intel_Core2

static void ComputeVoltage_Intel_Core2_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_Core2(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_Intel_Core2_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_Intel_Core2(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_Intel_Core2_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_Core2_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_Core2_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_Core2_PerPkg
};

static void ComputeVoltage_Intel_SoC( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{	/* Intel Valleyview-D/M SoC					*/
	COMPUTE_VOLTAGE(INTEL_SOC,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_SoC_PerSMT	ComputeVoltage_Intel_SoC

static void ComputeVoltage_Intel_SoC_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SoC(CFlip, RO(Shm), cpu);
	}
}

static  void ComputeVoltage_Intel_SoC_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SoC(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_Intel_SoC_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SoC_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SoC_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SoC_PerPkg
};

static void ComputeVoltage_Intel_SNB( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(INTEL_SNB,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_SNB_PerSMT 	ComputeVoltage_Intel_SNB

static void ComputeVoltage_Intel_SNB_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SNB(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_Intel_SNB_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SNB(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_Intel_SNB_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SNB_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SNB_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SNB_PerPkg
};

static void ComputeVoltage_Intel_SKL_X( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(INTEL_SKL_X,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Intel_SKL_X_PerSMT	ComputeVoltage_Intel_SKL_X

static void ComputeVoltage_Intel_SKL_X_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SKL_X(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_Intel_SKL_X_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SKL_X(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_Intel_SKL_X_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SKL_X_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SKL_X_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SKL_X_PerPkg
};

static void ComputeVoltage_AMD( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_PerSMT	ComputeVoltage_AMD

static void ComputeVoltage_AMD_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_AMD_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_AMD(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_AMD_Matrix[4])( struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_PerPkg
};

static void ComputeVoltage_AMD_0Fh( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{	/* AMD BKDG Family 0Fh §10.6 Table 70				*/
	COMPUTE_VOLTAGE(AMD_0Fh,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_0Fh_PerSMT	ComputeVoltage_AMD_0Fh

static void ComputeVoltage_AMD_0Fh_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_0Fh(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_AMD_0Fh_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_AMD_0Fh(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_AMD_0Fh_Matrix[4])(struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_0Fh_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_0Fh_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_0Fh_PerPkg
};

static void ComputeVoltage_AMD_15h( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD_15h,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_15h_PerSMT	ComputeVoltage_AMD_15h

static void ComputeVoltage_AMD_15h_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_15h(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_AMD_15h_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_AMD_15h(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_AMD_15h_Matrix[4])(struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_15h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_15h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_15h_PerPkg
};

static void ComputeVoltage_AMD_17h( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD_17h,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_AMD_17h_PerSMT	ComputeVoltage_AMD_17h

static void ComputeVoltage_AMD_17h_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_17h(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_AMD_17h(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_AMD_17h_Matrix[4])(struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_17h_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_17h_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_17h_PerPkg
};

static void ComputeVoltage_Winbond_IO( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{	/* Winbond W83627EHF/EF, W83627EHG,EG				*/
	COMPUTE_VOLTAGE(WINBOND_IO,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_Winbond_IO_PerSMT	ComputeVoltage_Winbond_IO

static void ComputeVoltage_Winbond_IO_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Winbond_IO(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_Winbond_IO_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_Winbond_IO(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_Winbond_IO_Matrix[4])( struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Winbond_IO_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Winbond_IO_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Winbond_IO_PerPkg
};

static void ComputeVoltage_ITE_Tech_IO( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{	/* ITE Tech IT8720F						*/
	COMPUTE_VOLTAGE(ITETECH_IO,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputeVoltage_ITE_Tech_IO_PerSMT	ComputeVoltage_ITE_Tech_IO

static void ComputeVoltage_ITE_Tech_IO_PerCore( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_ITE_Tech_IO(CFlip, RO(Shm), cpu);
	}
}

static void ComputeVoltage_ITE_Tech_IO_PerPkg( struct FLIP_FLOP *CFlip,
							RO(SHM_STRUCT) *RO(Shm),
							unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputeVoltage_ITE_Tech_IO(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputeVoltage_ITE_Tech_IO_Matrix[4])(	struct FLIP_FLOP*,
							RO(SHM_STRUCT)*,
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

#define ComputePower_Intel_Matrix	ComputePower_None_Matrix

#define ComputePower_Intel_Atom_Matrix	ComputePower_None_Matrix

#define ComputePower_AMD_Matrix 	ComputePower_None_Matrix

static void ComputePower_AMD_17h( struct FLIP_FLOP *CFlip,
					RO(SHM_STRUCT) *RO(Shm),
					unsigned int cpu)
{
	CFlip->State.Energy	= (double)CFlip->Delta.Power.ACCU
				* RO(Shm)->Proc.Power.Unit.Joules;

	CFlip->State.Power	= 1000.0 * CFlip->State.Energy;
	CFlip->State.Power	/= RO(Shm)->Sleep.Interval;

	Core_ComputePowerLimits(&RO(Shm)->Cpu[cpu], CFlip);
}

#define ComputePower_AMD_17h_PerSMT	ComputePower_AMD_17h

static void ComputePower_AMD_17h_PerCore( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if ((RO(Shm)->Cpu[cpu].Topology.ThreadID == 0)
	 || (RO(Shm)->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputePower_AMD_17h(CFlip, RO(Shm), cpu);
	}
}

static void ComputePower_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
						RO(SHM_STRUCT) *RO(Shm),
						unsigned int cpu )
{
	if (cpu == RO(Shm)->Proc.Service.Core)
	{
		ComputePower_AMD_17h(CFlip, RO(Shm), cpu);
	}
}

static void (*ComputePower_AMD_17h_Matrix[4])(	struct FLIP_FLOP*,
						RO(SHM_STRUCT)*,
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

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.voltageFormula)) {
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
	case VOLTAGE_KIND_INTEL_SAV:
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

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.powerFormula)) {
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
		printf("    Thread [%lx] %s CYCLE %03u\n", tid,
			BITVAL(RO(Core)->OffLine, OS) ? "Offline" : "Shutdown",
			cpu);
		fflush(stdout);
	}
	return NULL;
}

void SliceScheduling(	RO(SHM_STRUCT) *RO(Shm),
			unsigned int cpu, enum PATTERN pattern )
{
	unsigned int seek;
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
			seek = (unsigned int) rand();
			seek = seek % RO(Shm)->Proc.CPU.Count;
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

	CALL_FUNC MatrixCallFunc[2][2] = {
		{ CallWith_RDTSC_No_RDPMC,  CallWith_RDTSC_RDPMC  },
		{ CallWith_RDTSCP_No_RDPMC, CallWith_RDTSCP_RDPMC }
	};
	const int withTSCP = ((RO(Shm)->Proc.Features.AdvPower.EDX.Inv_TSC == 1)
			   || (RO(Shm)->Proc.Features.ExtInfo.EDX.RDTSCP == 1)),
		withRDPMC=((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
			  && (RO(Shm)->Proc.PM_version >= 1)
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
		printf("    Thread [%lx] %s CHILD %03u\n", tid,
			BITVAL(Cpu->OffLine, OS) ? "Offline" : "Shutdown",cpu);
		fflush(stdout);
	}
	return NULL;
}

void Architecture(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	signed long ix;
	Bit32	fTSC = RO(Proc)->Features.Std.EDX.TSC,
		aTSC = RO(Proc)->Features.AdvPower.EDX.Inv_TSC;

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
	/*	Copy the Architecture name.				*/
	StrCopy(RO(Shm)->Proc.Architecture,RO(Proc)->Architecture,CODENAME_LEN);
	/*	Make the processor's brand string with no trailing spaces */
	ix = BRAND_LENGTH;
	do {
		if ((RO(Proc)->Features.Info.Brand[ix] == 0x20)
		 || (RO(Proc)->Features.Info.Brand[ix] == 0x0)) {
			ix--;
		} else {
			break;
		}
	} while (ix != 0);
	RO(Shm)->Proc.Brand[1 + ix] = '\0';
	for (; ix >= 0; ix--) {
		RO(Shm)->Proc.Brand[ix] = RO(Proc)->Features.Info.Brand[ix];
	}
	/*	Compute the TSC mode: None, Variant, Invariant		*/
	RO(Shm)->Proc.Features.InvariantTSC = fTSC << aTSC;
}

void PerformanceMonitoring(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	RO(Shm)->Proc.PM_version = RO(Proc)->Features.PerfMon.EAX.Version;
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

    switch (KIND_OF_FORMULA(RO(Proc)->powerFormula)) {
    case POWER_KIND_INTEL:
    case POWER_KIND_AMD:
    case POWER_KIND_AMD_17h:
	RO(Shm)->Proc.Power.Unit.Watts = RO(Proc)->PowerThermal.Unit.PU > 0 ?
		1.0 / (double) (1 << RO(Proc)->PowerThermal.Unit.PU) : 0;

	RO(Shm)->Proc.Power.Unit.Joules= RO(Proc)->PowerThermal.Unit.ESU > 0 ?
		1.0 / (double) (1 << RO(Proc)->PowerThermal.Unit.ESU) :0;
      TIME_UNIT:
	RO(Shm)->Proc.Power.Unit.Times = RO(Proc)->PowerThermal.Unit.TU > 0 ?
			1.0 / (double) (1 << RO(Proc)->PowerThermal.Unit.TU)
			: 1.0 / (double) (1 << 10);

	pwrUnits = 2 << (RO(Proc)->PowerThermal.Unit.PU - 1);
	break;
    case POWER_KIND_INTEL_ATOM:
	RO(Shm)->Proc.Power.Unit.Watts = RO(Proc)->PowerThermal.Unit.PU > 0 ?
		0.001 / (double) (1 << RO(Proc)->PowerThermal.Unit.PU) : 0;

	RO(Shm)->Proc.Power.Unit.Joules= RO(Proc)->PowerThermal.Unit.ESU > 0 ?
		0.001 / (double) (1 << RO(Proc)->PowerThermal.Unit.ESU) : 0;
	goto TIME_UNIT;
	break;
    case POWER_KIND_NONE:
	break;
    }
  if (	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
  ||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
  { /* AMD PowerNow */
	if (RO(Proc)->Features.AdvPower.EDX.FID) {
		BITSET(LOCKLESS, RO(Shm)->Proc.PowerNow, 0);
	} else {
		BITCLR(LOCKLESS, RO(Shm)->Proc.PowerNow, 0);
	}
	if (RO(Proc)->Features.AdvPower.EDX.VID) {
		BITSET(LOCKLESS, RO(Shm)->Proc.PowerNow, 1);
	} else {
		BITCLR(LOCKLESS, RO(Shm)->Proc.PowerNow, 1);
	}
	RO(Shm)->Proc.Power.PPT = RO(Proc)->PowerThermal.Zen.PWR.PPT;
	RO(Shm)->Proc.Power.TDP = RO(Proc)->PowerThermal.Zen.TDP.TDP;
	RO(Shm)->Proc.Power.Min = RO(Proc)->PowerThermal.Zen.TDP.TDP2;
	RO(Shm)->Proc.Power.Max = RO(Proc)->PowerThermal.Zen.TDP.TDP3;
	RO(Shm)->Proc.Power.EDC = RO(Proc)->PowerThermal.Zen.EDC.EDC << 2;
	RO(Shm)->Proc.Power.TDC = RO(Proc)->PowerThermal.Zen.EDC.TDC;

      for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
      {
	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Enable_Limit1;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Enable = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Enable_Limit2;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Clamping = \
			RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Clamping1;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Clamping = \
			RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Clamping2;

	pwrVal = RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Domain_Limit1;
	RO(Shm)->Proc.Power.Domain[pw].PWL[PL1] = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Domain_Limit2;
	RO(Shm)->Proc.Power.Domain[pw].PWL[PL2] = pwrVal;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock = \
	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock = \
				RO(Proc)->PowerThermal.Domain[pw].Unlock;
      }
  }
  else if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
  {
    if (pwrUnits != 0)
    {
      for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
      {
	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Enable = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Enable_Limit1;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Enable = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Enable_Limit2;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Clamping = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Clamping1;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Clamping = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Clamping2;

	pwrVal	= RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Domain_Limit1
		/ pwrUnits;
	RO(Shm)->Proc.Power.Domain[pw].PWL[PL1] = pwrVal;

	pwrVal	= RO(Proc)->PowerThermal.Domain[pw].PowerLimit.Domain_Limit2
		/ pwrUnits;
	RO(Shm)->Proc.Power.Domain[pw].PWL[PL2] = pwrVal;
      }
	pwrVal = RO(Proc)->PowerThermal.PowerInfo.ThermalSpecPower / pwrUnits;
	RO(Shm)->Proc.Power.TDP = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.PowerInfo.MinimumPower / pwrUnits;
	RO(Shm)->Proc.Power.Min = pwrVal;

	pwrVal = RO(Proc)->PowerThermal.PowerInfo.MaximumPower / pwrUnits;
	RO(Shm)->Proc.Power.Max = pwrVal;
    }
    for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
    {
	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].TW = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.TimeWindow1;

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].TW = \
		RO(Proc)->PowerThermal.Domain[pw].PowerLimit.TimeWindow2;

	RO(Shm)->Proc.Power.Domain[pw].TAU[PL1] = ComputeTAU(
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].TW_Y,
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].TW_Z,
		RO(Shm)->Proc.Power.Unit.Times
	);

	RO(Shm)->Proc.Power.Domain[pw].TAU[PL2] = ComputeTAU(
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].TW_Y,
		RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].TW_Z,
		RO(Shm)->Proc.Power.Unit.Times
	);

	RO(Shm)->Proc.Power.Domain[pw].Feature[PL1].Unlock = \
	RO(Shm)->Proc.Power.Domain[pw].Feature[PL2].Unlock = \
				RO(Proc)->PowerThermal.Domain[pw].Unlock;
    }
	RO(Shm)->Proc.Power.TDC = RO(Proc)->PowerThermal.TDC;
	RO(Shm)->Proc.Power.Feature.TDC=RO(Proc)->PowerThermal.Enable_Limit.TDC;
  } else {
	RO(Shm)->Proc.PowerNow = 0;
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
	RO(Shm)->Proc.Technology.PowerNow = (RO(Shm)->Proc.PowerNow == 0b11);

	RO(Shm)->Proc.Technology.ODCM = BITCMP_CC(LOCKLESS,
						RW(Proc)->ODCM,
						RO(Proc)->ODCM_Mask);

	RO(Shm)->Proc.Technology.L1_HW_Prefetch = BITCMP_CC(LOCKLESS,
						RW(Proc)->L1_HW_Prefetch,
						RO(Proc)->DCU_Mask);

	RO(Shm)->Proc.Technology.L1_HW_IP_Prefetch = BITCMP_CC(LOCKLESS,
						RW(Proc)->L1_HW_IP_Prefetch,
						RO(Proc)->DCU_Mask);

	RO(Shm)->Proc.Technology.L2_HW_Prefetch = BITCMP_CC(LOCKLESS,
						RW(Proc)->L2_HW_Prefetch,
						RO(Proc)->DCU_Mask);

	RO(Shm)->Proc.Technology.L2_HW_CL_Prefetch = BITCMP_CC(LOCKLESS,
						RW(Proc)->L2_HW_CL_Prefetch,
						RO(Proc)->DCU_Mask);

	RO(Shm)->Proc.Technology.PowerMgmt = BITCMP_CC(LOCKLESS,
						RW(Proc)->PowerMgmt,
						RO(Proc)->PowerMgmt_Mask);

	RO(Shm)->Proc.Technology.EIST = BITCMP_CC(LOCKLESS,
						RW(Proc)->SpeedStep,
						RO(Proc)->SpeedStep_Mask);

	RO(Shm)->Proc.Technology.Turbo = BITWISEAND_CC(LOCKLESS,
						RW(Proc)->TurboBoost,
						RO(Proc)->TurboBoost_Mask) != 0;

	RO(Shm)->Proc.Technology.C1E = BITCMP_CC(LOCKLESS,
						RW(Proc)->C1E,
						RO(Proc)->C1E_Mask);

	RO(Shm)->Proc.Technology.C3A = BITCMP_CC(LOCKLESS,
						RW(Proc)->C3A,
						RO(Proc)->C3A_Mask);

	RO(Shm)->Proc.Technology.C1A = BITCMP_CC(LOCKLESS,
						RW(Proc)->C1A,
						RO(Proc)->C1A_Mask);

	RO(Shm)->Proc.Technology.C3U = BITCMP_CC(LOCKLESS,
						RW(Proc)->C3U,
						RO(Proc)->C3U_Mask);

	RO(Shm)->Proc.Technology.C1U = BITCMP_CC(LOCKLESS,
						RW(Proc)->C1U,
						RO(Proc)->C1U_Mask);

	RO(Shm)->Proc.Technology.CC6 = BITCMP_CC(LOCKLESS,
						RW(Proc)->CC6,
						RO(Proc)->CC6_Mask);

	RO(Shm)->Proc.Technology.PC6 = BITCMP_CC(LOCKLESS,
						RW(Proc)->PC6,
						RO(Proc)->PC6_Mask);

	RO(Shm)->Proc.Technology.SMM = BITCMP_CC(LOCKLESS,
						RW(Proc)->SMM,
						RO(Proc)->CR_Mask);

	RO(Shm)->Proc.Technology.VM = BITCMP_CC(LOCKLESS,
						RW(Proc)->VM,
						RO(Proc)->CR_Mask);

	RO(Shm)->Proc.Technology.WDT = BITCMP_CC(LOCKLESS,
						RW(Proc)->WDT,
						RO(Proc)->WDT_Mask);

								/* 000v */
	RO(Shm)->Proc.Technology.TM1 = RO(Proc)->Features.Std.EDX.TM1
					| RO(Proc)->Features.AdvPower.EDX.TTP;
								/* 00v0 */
	RO(Shm)->Proc.Technology.TM1 |= BITCMP_CC(LOCKLESS,
						RW(Proc)->TM1,
						RO(Proc)->TM_Mask) << 1;

								/* 000v */
	RO(Shm)->Proc.Technology.TM2 = RO(Proc)->Features.Std.ECX.TM2
					| RO(Proc)->Features.AdvPower.EDX.TM;
								/* 00v0 */
	RO(Shm)->Proc.Technology.TM2 |= BITCMP_CC(LOCKLESS,
						RW(Proc)->TM2,
						RO(Proc)->TM_Mask) << 1;
}

void Mitigation_2nd_Stage(	RO(SHM_STRUCT) *RO(Shm),
				RO(PROC) *RO(Proc), RW(PROC) *RW(Proc) )
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

	RO(Shm)->Proc.Mechanisms.IBRS  += (2 * IBRS);
	RO(Shm)->Proc.Mechanisms.STIBP += (2 * STIBP);
	RO(Shm)->Proc.Mechanisms.SSBD  += (2 * SSBD);
}

void Mitigation_1st_Stage(	RO(SHM_STRUCT) *RO(Shm),
				RO(PROC) *RO(Proc), RW(PROC) *RW(Proc) )
{
    if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
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

	RO(Shm)->Proc.Mechanisms.IBRS = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IBRS_IBPB_Cap == 1
	);
	RO(Shm)->Proc.Mechanisms.STIBP = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.STIBP_Cap == 1
	);
	RO(Shm)->Proc.Mechanisms.SSBD = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.SSBD_Cap == 1
	);

	Mitigation_2nd_Stage(RO(Shm), RO(Proc), RW(Proc));

	RO(Shm)->Proc.Mechanisms.RDCL_NO = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * RDCL_NO)
	);
	RO(Shm)->Proc.Mechanisms.IBRS_ALL = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * IBRS_ALL)
	);
	RO(Shm)->Proc.Mechanisms.RSBA = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * RSBA)
	);
	RO(Shm)->Proc.Mechanisms.L1DFL_VMENTRY_NO = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * L1DFL_NO)
	);
	RO(Shm)->Proc.Mechanisms.SSB_NO = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * SSB_NO)
	);
	RO(Shm)->Proc.Mechanisms.MDS_NO = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * MDS_NO)
	);
	RO(Shm)->Proc.Mechanisms.PSCHANGE_MC_NO = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2*PSCHANGE_MC_NO)
	);
	RO(Shm)->Proc.Mechanisms.TAA_NO = (
		RO(Shm)->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP
		+ (2 * TAA_NO)
	);
	RO(Shm)->Proc.Mechanisms.STLB = BITCMP_CC(LOCKLESS,
						RW(Proc)->STLB,
						RO(Proc)->ARCH_CAP_Mask);
	RO(Shm)->Proc.Mechanisms.FUSA = BITCMP_CC(LOCKLESS,
						RW(Proc)->FUSA,
						RO(Proc)->ARCH_CAP_Mask);
	RO(Shm)->Proc.Mechanisms.RSM_CPL0 = BITCMP_CC(LOCKLESS,
						RW(Proc)->RSM_CPL0,
						RO(Proc)->ARCH_CAP_Mask);
	RO(Shm)->Proc.Mechanisms.SPLA = BITCMP_CC(LOCKLESS,
						RW(Proc)->SPLA,
						RO(Proc)->ARCH_CAP_Mask);
	RO(Shm)->Proc.Mechanisms.SNOOP_FILTER = BITCMP_CC(LOCKLESS,
						RW(Proc)->SNOOP_FILTER,
						RO(Proc)->ARCH_CAP_Mask);
    }
    else if (	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON) )
    {
	unsigned short	PSFD = BITCMP_CC(	LOCKLESS,
						RW(Proc)->PSFD,
						RO(Proc)->SPEC_CTRL_Mask );

	RO(Shm)->Proc.Mechanisms.IBRS = (
		RO(Shm)->Proc.Features.leaf80000008.EBX.IBRS == 1
	);
	RO(Shm)->Proc.Mechanisms.STIBP = (
		RO(Shm)->Proc.Features.leaf80000008.EBX.STIBP == 1
	);
	RO(Shm)->Proc.Mechanisms.SSBD = (
		RO(Shm)->Proc.Features.leaf80000008.EBX.SSBD == 1
	);

	Mitigation_2nd_Stage(RO(Shm), RO(Proc), RW(Proc));

	RO(Shm)->Proc.Mechanisms.PSFD = (
		RO(Shm)->Proc.Features.leaf80000008.EBX.PSFD + (2 * PSFD)
	);
    }
}

#define TIMING(_mc, _cha)	RO(Shm)->Uncore.MC[_mc].Channel[_cha].Timing

typedef struct {
	unsigned int	Q,
			R;
} RAM_Ratio;

void P945_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT0.tWR;

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRTPr;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRAS;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRRD) {
	case 0b000:
		TIMING(mc, cha).tRRD = 1;
		break;
	case 0b001:
		TIMING(mc, cha).tRRD = 2;
		break;
	}

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRFC;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tCL) {
	case 0b00:
		TIMING(mc, cha).tCL = 5;
		break;
	case 0b01:
		TIMING(mc, cha).tCL = 4;
		break;
	case 0b10:
		TIMING(mc, cha).tCL = 3;
		break;
	case 0b11:
		TIMING(mc, cha).tCL = 6;
		break;
	}

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRCD + 2;

	TIMING(mc, cha).tRP  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT1.tRP + 2;

      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned long long DIMM_Size;
	unsigned short rank, rankCount = (cha == 0) ? 4 : 2;

	for (rank = 0; rank < rankCount; rank++) {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += \
		  RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRB[rank].Boundary;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.BANK.Rank0)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P945.WIDTH.Rank0)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 16384;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 8192;
		break;
	}
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

	DIMM_Size	= RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
      }
	TIMING(mc, cha).ECC = 0;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P945.DRT2.tCKE;
    }
  }
}

void P955_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tRAS;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tCL) {
	case 0b00:
		TIMING(mc, cha).tCL = 5;
		break;
	case 0b01:
		TIMING(mc, cha).tCL = 4;
		break;
	case 0b10:
		TIMING(mc, cha).tCL = 3;
		break;
	case 0b11:
		TIMING(mc, cha).tCL = 6;
		break;
	}

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tRCD + 2;

	TIMING(mc, cha).tRP  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRT1.tRP + 2;

      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned long long DIMM_Size;
	unsigned short rank;

	for (rank = 0; rank < 4; rank++) {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += \
		  RO(Proc)->Uncore.MC[mc].Channel[cha].P955.DRB[rank].Boundary;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P955.BANK.Rank0)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].P955.WIDTH.Rank0)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 16384;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 8192;
		break;
	}
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

	DIMM_Size	= RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
							(unsigned int)DIMM_Size;
      }
	TIMING(mc, cha).ECC = 0;
    }
  }
}

void P945_CLK(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b000:
		RO(Shm)->Uncore.Bus.Rate = 400;

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
		RO(Shm)->Uncore.Bus.Rate = 533;

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
		RO(Shm)->Uncore.Bus.Rate = 667;

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

	RO(Shm)->Uncore.CtrlSpeed = (RO(Core)->Clock.Hz * Ratio.Q * 2)/* DDR2 */
				/ (Ratio.R * 1000000L);

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 2;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_UNSPEC;
}

void P965_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT0.tCL;

	TIMING(mc, cha).tCL += 3;

	TIMING(mc, cha).tRAS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT1.tRAS;

	TIMING(mc, cha).tWR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT1.tWR;

	TIMING(mc, cha).tRFC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.tRFC;

	TIMING(mc, cha).tRP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.tRP;

	TIMING(mc, cha).tRRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.tRRD;

	TIMING(mc, cha).tRCD_WR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT3.tRCD_WR;

	TIMING(mc, cha).tRCD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT4.tRCD_RD;

	TIMING(mc, cha).tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.ACT_Count;

	TIMING(mc, cha).tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT2.ReservedBits;
/* TODO(Timings)
	TIMING(mc, cha).tCWL  = ?
*/
	TIMING(mc, cha).tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT3.tRD_WR;

	TIMING(mc, cha).tsrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT4.tRD_RD_SR;

	TIMING(mc, cha).tdrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT4.tRD_RD_DR;

	TIMING(mc, cha).tsrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT3.tWR_WR_SR;

	TIMING(mc, cha).tdrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT3.tWR_WR_DR;

	TIMING(mc, cha).tsrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT4.tWR_RD_SR;

	TIMING(mc, cha).tdrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P965.DRT4.tWR_RD_DR;

	for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;
/* TODO(Geometry):Hardware missing! */
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

		DIMM_Size = 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
	TIMING(mc, cha).ECC = 0;

	TIMING(mc, cha).tXS = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P965.CKE0.tXSNR
				: RO(Proc)->Uncore.MC[mc].P965.CKE1.tXSNR;

	TIMING(mc, cha).tXP = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P965.CKE0.tXP
				: RO(Proc)->Uncore.MC[mc].P965.CKE1.tXP;

	TIMING(mc, cha).tCKE = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P965.CKE0.tCKE_Low
				: RO(Proc)->Uncore.MC[mc].P965.CKE1.tCKE_Low;
    }
  }
}

void P965_CLK(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b111:	/* Unknown */
		fallthrough;
	case 0b000:
		RO(Shm)->Uncore.Bus.Rate = 1066;

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
		RO(Shm)->Uncore.Bus.Rate = 533;

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
		RO(Shm)->Uncore.Bus.Rate = 667;
		break;
	case 0b100:
		RO(Shm)->Uncore.Bus.Rate = 1333;

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
		RO(Shm)->Uncore.Bus.Rate = 1600;

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
		RO(Shm)->Uncore.Bus.Rate = 800;

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

	RO(Shm)->Uncore.CtrlSpeed = (RO(Core)->Clock.Hz * Ratio.Q * 2) /* DDR2 */
				/ (Ratio.R * 1000000L);

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 2;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_UNSPEC;
}

void G965_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
      RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tWR  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT0.tWR;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRCD) {
	case 0b000:
		TIMING(mc, cha).tRCD = 2;
		break;
	case 0b001:
		TIMING(mc, cha).tRCD = 3;
		break;
	case 0b010:
		TIMING(mc, cha).tRCD = 4;
		break;
	case 0b011:
		TIMING(mc, cha).tRCD = 5;
		break;
	case 0b100:
		TIMING(mc, cha).tRCD = 6;
		break;
	case 0b101:
		TIMING(mc, cha).tRCD = 7;
		break;
	case 0b110:
		TIMING(mc, cha).tRCD = 8;
		break;
	}

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRP) {
	case 0b000:
		TIMING(mc, cha).tRP = 2;
		break;
	case 0b001:
		TIMING(mc, cha).tRP = 3;
		break;
	case 0b010:
		TIMING(mc, cha).tRP = 4;
		break;
	case 0b011:
		TIMING(mc, cha).tRP = 5;
		break;
	case 0b100:
		TIMING(mc, cha).tRP = 6;
		break;
	case 0b101:
		TIMING(mc, cha).tRP = 7;
		break;
	case 0b110:
		TIMING(mc, cha).tRP = 8;
		break;
	}

	TIMING(mc, cha).tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRAS;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRRD) {
	case 0b000:
		TIMING(mc, cha).tRRD = 2;
		break;
	case 0b001:
		TIMING(mc, cha).tRRD = 3;
		break;
	case 0b010:
		TIMING(mc, cha).tRRD = 4;
		break;
	case 0b011:
		TIMING(mc, cha).tRRD = 5;
		break;
	case 0b100:
		TIMING(mc, cha).tRRD = 6;
		break;
	}

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT1.tRTPr;

	TIMING(mc, cha).tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT2.tFAW;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tCL) {
	case 0b000:
		TIMING(mc, cha).tCL = 3;
		break;
	case 0b001:
		TIMING(mc, cha).tCL = 4;
		break;
	case 0b010:
		TIMING(mc, cha).tCL = 5;
		break;
	case 0b011:
		TIMING(mc, cha).tCL = 6;
		break;
	case 0b100:
		TIMING(mc, cha).tCL = 7;
		break;
	}

	TIMING(mc, cha).tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tRFC;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tCWL) {
	case 0b000:
		TIMING(mc, cha).tCWL = 2;
		break;
	case 0b001:
		TIMING(mc, cha).tCWL = 3;
		break;
	case 0b010:
		TIMING(mc, cha).tCWL = 4;
		break;
	case 0b011:
		TIMING(mc, cha).tCWL = 5;
		break;
	case 0b100:
		TIMING(mc, cha).tCWL = 6;
		break;
	default:
		TIMING(mc, cha).tCWL = \
			TIMING(mc, cha).tCL - 1;
		break;
	}
      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned long long DIMM_Size;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank1Bank)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.Rank0Bank)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 4;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks += 8;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.OddRank1)
	{
	case 0b10:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1;
		break;
	case 0b11:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 2;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.EvenRank0)
	{
	case 0b10:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += 1;
		break;
	case 0b11:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks += 2;
		break;
	}

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 4096;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1024;

	DIMM_Size	= 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
      }
	TIMING(mc, cha).ECC = 0;

	TIMING(mc, cha).tXS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT3.tXS;

	TIMING(mc, cha).tXP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT2.tXP;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].G965.DRT2.tCKE;
    }
  }
}

void G965_CLK(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b001:
		RO(Shm)->Uncore.Bus.Rate = 533;

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
		RO(Shm)->Uncore.Bus.Rate = 667;

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
		RO(Shm)->Uncore.Bus.Rate = 1066;

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
		RO(Shm)->Uncore.Bus.Rate = 800;

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

	RO(Shm)->Uncore.CtrlSpeed = (RO(Core)->Clock.Hz * Ratio.Q * 2) /* DDR2 */
				/ (Ratio.R * 1000000L);

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 2;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_UNSPEC;
}

void P3S_MCH(	RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc),
		unsigned short mc,unsigned short cha )
{
	TIMING(mc, cha).tCL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT0.tCL;

	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT1.tWR;

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.tRFC;

	TIMING(mc, cha).tRP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.tRP;

	TIMING(mc, cha).tRRD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.tRRD;

	TIMING(mc, cha).tRCD_WR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT3.tRCD_WR;

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT4.tRCD_RD;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT5.tRAS;

	TIMING(mc, cha).tFAW = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.ACT_Count;

	TIMING(mc, cha).tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT2.ReservedBits;
/* TODO(Timings)
	TIMING(mc, cha).tCWL = ?
*/
	TIMING(mc, cha).tddRdTWr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT3.tRD_WR;

	TIMING(mc, cha).tsrRdTRd = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT4.tRD_RD_SR;

	TIMING(mc, cha).tdrRdTRd = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT4.tRD_RD_DR;

	TIMING(mc, cha).tsrWrTWr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT3.tWR_WR_SR;

	TIMING(mc, cha).tdrWrTWr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT3.tWR_WR_DR;

	TIMING(mc, cha).tsrWrTRd = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT4.tWR_RD_SR;

	TIMING(mc, cha).tdrWrTRd = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].P35.DRT4.tWR_RD_DR;

	TIMING(mc, cha).tXS = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P35.CKE0.tXSNR
				: RO(Proc)->Uncore.MC[mc].P35.CKE1.tXSNR;

	TIMING(mc, cha).tXP = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P35.CKE0.tXP
				: RO(Proc)->Uncore.MC[mc].P35.CKE1.tXP;

	TIMING(mc, cha).tCKE = (cha == 0) ?
				  RO(Proc)->Uncore.MC[mc].P35.CKE0.tCKE_Low
				: RO(Proc)->Uncore.MC[mc].P35.CKE1.tCKE_Low;
}

void P35_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	P3S_MCH(RO(Shm), RO(Proc), mc, cha);

	TIMING(mc, cha).tCL -= 9;
/* TODO(Timings) */
      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned long long DIMM_Size;
/* TODO(Geometry):Hardware missing! */
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

	DIMM_Size	= 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
      }
	TIMING(mc, cha).ECC = 0;
    }
  }
}

void P35_CLK(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	P965_CLK(RO(Shm), RO(Proc), RO(Core));
}

void P4S_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	P3S_MCH(RO(Shm), RO(Proc), mc, cha);

	TIMING(mc, cha).tCL -= 6;
/* TODO(Timings) */
      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned long long DIMM_Size;
/* TODO(Geometry):Hardware missing! */
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 0;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 0;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 0;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 0;

	DIMM_Size	= 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
      }
	TIMING(mc, cha).ECC = 0;
    }
  }
}

void SLM_PTR(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned short mc, cha, slot;
/* BUS & DRAM frequency */
	RO(Shm)->Uncore.CtrlSpeed = 800LLU + (
			((2134LLU * RO(Proc)->Uncore.MC[0].SLM.DTR0.DFREQ) >> 3)
	);
	RO(Shm)->Uncore.Bus.Rate = 5000;

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
/* Standard Timings */
	TIMING(mc, cha).tCL  = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tCL + 5;

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tRCD + 5;

	TIMING(mc, cha).tRP  = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tRP + 5;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tRAS;

	TIMING(mc, cha).tRRD = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tRRD + 4;

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR0.tXS == 0 ? 256 : 384;

	switch (RO(Proc)->Uncore.MC[mc].SLM.DRFC.tREFI) {
	case 0 ... 1:
		TIMING(mc, cha).tREFI = 0;
		break;
	case 2:
		TIMING(mc, cha).tREFI = 39;
		break;
	case 3:
		TIMING(mc, cha).tREFI = 78;
		break;
	}
	TIMING(mc, cha).tREFI *= RO(Shm)->Uncore.CtrlSpeed;
	TIMING(mc, cha).tREFI /= 20;

/*TODO( Advanced Timings )
	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SLM
*/
	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tRTP + 4;

	TIMING(mc, cha).tWTPr = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tWTP + 14;

	TIMING(mc, cha).B2B   = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tCCD;

	switch (RO(Proc)->Uncore.MC[mc].SLM.DTR1.tFAW) {
	case 0 ... 1:
		TIMING(mc, cha).tFAW = 0;
		break;
	default:
		TIMING(mc, cha).tFAW = \
		10 + ((unsigned int)RO(Proc)->Uncore.MC[mc].SLM.DTR1.tFAW << 1);
		break;
	}

	TIMING(mc, cha).tCWL = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR1.tWCL + 3;
/* Same Rank */
/*TODO( Read to Read )
	TIMING(mc, cha).tsrRdTRd = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR?.;
*/
	TIMING(mc, cha).tsrRdTWr = 6
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tRWSR;

	TIMING(mc, cha).tsrWrTRd = 11
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tWRSR;
/*TODO( Write to Write )
	TIMING(mc, cha).tsrWrTWr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SLM.DTR?.;
*/
/* Different Rank */
	TIMING(mc, cha).tdrRdTRd = \
			RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRRDR;
	if (TIMING(mc, cha).tdrRdTRd > 0) {
		TIMING(mc, cha).tdrRdTRd += 5;
	}

	TIMING(mc, cha).tdrRdTWr = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRWDR;
	if (TIMING(mc, cha).tdrRdTWr > 0) {
		TIMING(mc, cha).tdrRdTWr += 5;
	}

	TIMING(mc, cha).tdrWrTRd = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tWRDR;
	if (TIMING(mc, cha).tdrWrTRd > 0) {
		TIMING(mc, cha).tdrWrTRd += 3;
	}

	TIMING(mc, cha).tdrWrTWr = 4
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tWWDR;
/* Different DIMM */
	TIMING(mc, cha).tddRdTRd = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRRDD;
	if (TIMING(mc, cha).tddRdTRd > 0) {
		TIMING(mc, cha).tddRdTRd += 5;
	}

	TIMING(mc, cha).tddRdTWr = \
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tRWDD;
	if (TIMING(mc, cha).tddRdTWr > 0) {
		TIMING(mc, cha).tddRdTWr += 5;
	}

	TIMING(mc, cha).tddWrTRd = 4
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR3.tWRDD;

	TIMING(mc, cha).tddWrTWr = 4
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR2.tWWDD;
/* Command Rate */
	TIMING(mc, cha).CMD_Rate = 1
			+ RO(Proc)->Uncore.MC[mc].SLM.DTR1.tCMD;

	TIMING(mc, cha).tXS = RO(Proc)->Uncore.MC[mc].SLM.DTR0.tXS;

	TIMING(mc, cha).tXP = RO(Proc)->Uncore.MC[mc].SLM.DTR3.tXP;
/* Topology */
	for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
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
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SLM.DRP.RKEN0
				+	RO(Proc)->Uncore.MC[mc].SLM.DRP.RKEN1;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN0].Banks;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN0].Rows;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN0].Cols;
	    } else {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SLM.DRP.RKEN2
				+	RO(Proc)->Uncore.MC[mc].SLM.DRP.RKNE3;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN1].Banks;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN1].Rows;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = \
			DDR3L[RO(Proc)->Uncore.MC[mc].SLM.DRP.DIMMDDEN1].Cols;
	    }
		DIMM_Size = 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	}
/* Error Correcting Code */
	TIMING(mc, cha).ECC = \
			  RO(Proc)->Uncore.MC[mc].SLM.BIOS_CFG.EFF_ECC_EN
			| RO(Proc)->Uncore.MC[mc].SLM.BIOS_CFG.ECC_EN;
    }
    if (RO(Proc)->Uncore.MC[mc].SLM.DRP.DRAMTYPE) {
	RO(Shm)->Uncore.Unit.DDR_Ver = 2;
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_LPDDR;
    } else {
	RO(Shm)->Uncore.Unit.DDR_Ver = 3;

	if (RO(Proc)->Uncore.MC[mc].SLM.DRP.ENLPDDR3) {
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_LPDDR;
	} else {
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
	}
    }
  }
}

void NHM_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tCL ?
		4 + RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tCL : 0;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR0_1.tWR) {
	case 0b001:
		TIMING(mc, cha).tWR = 5;
		break;
	case 0b010:
		TIMING(mc, cha).tWR = 6;
		break;
	case 0b011:
		TIMING(mc, cha).tWR = 7;
		break;
	case 0b100:
		TIMING(mc, cha).tWR = 8;
		break;
	case 0b101:
		TIMING(mc, cha).tWR = 10;
		break;
	case 0b110:
		TIMING(mc, cha).tWR = 12;
		break;
	}
	TIMING(mc, cha).tRCD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRCD;

	TIMING(mc, cha).tRP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRP;

	TIMING(mc, cha).tRAS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRAS;

	TIMING(mc, cha).tRRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tRRD;

	TIMING(mc, cha).tRFC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Refresh.tRFC;

	TIMING(mc, cha).tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Refresh.tREFI_8;

	TIMING(mc, cha).tCKE = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.CKE_Timing.tCKE;

	TIMING(mc, cha).tXS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.CKE_Timing.tXS;

	TIMING(mc, cha).tXP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.CKE_Timing.tXP;

	TIMING(mc, cha).tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tRTPr;

	TIMING(mc, cha).tWTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Bank.tWTPr;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrRdTRd) {
	case 0b0:
		TIMING(mc, cha).tsrRdTRd = 4;
		break;
	case 0b1:
		TIMING(mc, cha).tsrRdTRd = 6;
		break;
	}
	TIMING(mc, cha).tdrRdTRd = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrRdTRd;

	TIMING(mc, cha).tddRdTRd = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddRdTRd;

	TIMING(mc, cha).tsrRdTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrRdTWr;

	TIMING(mc, cha).tdrRdTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrRdTWr;

	TIMING(mc, cha).tddRdTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddRdTWr;

	TIMING(mc, cha).tsrWrTRd = 10
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tsrWrTRd;

	TIMING(mc, cha).tdrWrTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tdrWrTRd;

	TIMING(mc, cha).tddWrTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_A.tddWrTRd;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tsrWrTWr) {
	case 0b0:
		TIMING(mc, cha).tsrWrTWr = 4;
		break;
	case 0b1:
		TIMING(mc, cha).tsrWrTWr = 6;
		break;
	}
	TIMING(mc, cha).tdrWrTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tdrWrTWr;

	TIMING(mc, cha).tddWrTWr = 2
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tddWrTWr;

	TIMING(mc, cha).tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.tFAW;

	TIMING(mc, cha).B2B  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Rank_B.B2B;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.MR2_3.tCWL) {
	case 0b000:
		TIMING(mc, cha).tCWL = 5;
		break;
	case 0b001:
		TIMING(mc, cha).tCWL = 6;
		break;
	case 0b010:
		TIMING(mc, cha).tCWL = 7;
		break;
	case 0b011:
		TIMING(mc, cha).tCWL = 8;
		break;
	}

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.Sched.ENABLE_2N_3N)
	{
	case 0b00:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b01:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	}

	if (RO(Proc)->Uncore.MC[mc].Channel[cha].NHM.DIMM_Init.REGISTERED_DIMM)
	{
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_RDIMM;
	} else {
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
	}

     for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++) {
      if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.DIMMPRESENT)
      {
	unsigned long long DIMM_Size;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMBANK)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	case 0b10:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 16;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMRANK)
	{
	case 0b00:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1;
		break;
	case 0b01:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 2;
		break;
	case 0b10:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 4;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMROW)
	{
	case 0b000:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 12;
		break;
	case 0b001:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 13;
		break;
	case 0b010:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 14;
		break;
	case 0b011:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 15;
		break;
	case 0b100:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 16;
		break;
	}
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.NUMCOL)
	{
	case 0b000:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 10;
		break;
	case 0b001:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 11;
		break;
	case 0b010:
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 12;
		break;
	}

	DIMM_Size	= 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
      }
     }
	TIMING(mc, cha).ECC = (unsigned int)\
			( RO(Proc)->Uncore.MC[mc].NHM.STATUS.ECC_ENABLED
			& RO(Proc)->Uncore.MC[mc].NHM.CONTROL.ECC_ENABLED );
    }
  }
}

void QPI_CLK(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.DimmClock.QCLK_RATIO) {
	case 0b00110:
		RO(Shm)->Uncore.CtrlSpeed = 800;
		break;
	case 0b01000:
		RO(Shm)->Uncore.CtrlSpeed = 1066;
		break;
	case 0b01010:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b01100:
		RO(Shm)->Uncore.CtrlSpeed = 1600;
		break;
	case 0b01110:
		RO(Shm)->Uncore.CtrlSpeed = 1866;
		break;
	case 0b10000:
		RO(Shm)->Uncore.CtrlSpeed = 2133;
		break;
	case 0b000000:
		fallthrough;
	default:
		RO(Shm)->Uncore.CtrlSpeed = 800;
		break;
	}

	RO(Shm)->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	RO(Shm)->Uncore.CtrlSpeed /= RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Bus.Rate = \
			RO(Proc)->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 0b00 ?
		4800 :	RO(Proc)->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 0b10 ?
		6400 :	RO(Proc)->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 0b01 ?
		5866 : 6400;

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 3;
}

void X58_VTD(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	UNUSED(RO(Core));

	RO(Shm)->Proc.Technology.IOMMU = \
				!RO(Proc)->Uncore.Bus.QuickPath.X58.VT_d;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Major = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Minor = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void DMI_CLK(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.DimmClock.QCLK_RATIO) {
	case 0b00010:
		RO(Shm)->Uncore.CtrlSpeed = 266;
		break;
	case 0b00100:
		RO(Shm)->Uncore.CtrlSpeed = 533;
		break;
	case 0b00110:
		RO(Shm)->Uncore.CtrlSpeed = 800;
		break;
	case 0b01000:
		RO(Shm)->Uncore.CtrlSpeed = 1066;
		break;
	case 0b01010:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b000000:
		fallthrough;
	default:
		RO(Shm)->Uncore.CtrlSpeed = 266;
		break;
	}

	RO(Shm)->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	RO(Shm)->Uncore.CtrlSpeed /= RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Bus.Rate = 2500; /* TODO: hardwired to Lynnfield */

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 3;
}

void SNB_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
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

     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tCL;

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tRCD;

	TIMING(mc, cha).tRP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tRP;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tRAS;

	TIMING(mc, cha).tRRD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tRRD;

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RFTP.tRFC;

	TIMING(mc, cha).tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RFTP.tREFI;

	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tWR;

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tRTPr;

	TIMING(mc, cha).tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tWTPr;

	TIMING(mc, cha).tFAW = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tFAW;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.tCKE;

	TIMING(mc, cha).tCWL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.DBP.tCWL;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RAP.CMD_Stretch) {
	case 0b00:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	default:
		TIMING(mc, cha).CMD_Rate = 0;
		break;
	}

	TIMING(mc, cha).tddWrTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tWRDD;

	TIMING(mc, cha).tdrWrTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tWRDR;
/*
	TIMING(mc, cha).tsrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tWRSR;

	TIMING(mc, cha).tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tRWDD;
*/
	TIMING(mc, cha).tdrRdTWr = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tRWDR;

	TIMING(mc, cha).tsrRdTWr = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tRWSR;
/*
	TIMING(mc, cha).tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tRRDD;
*/
	TIMING(mc, cha).tdrRdTRd = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tRRDR;

	TIMING(mc, cha).tsrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tRRSR;

	TIMING(mc, cha).tddWrTWr = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tWWDD;

	TIMING(mc, cha).tdrWrTWr = 1
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tWWDR;

	TIMING(mc, cha).tsrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tWWSR;

	TIMING(mc, cha).B2B = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.RWP.tCCD;

	TIMING(mc, cha).tXS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.SRFTP.tXS;

	TIMING(mc, cha).PDM_EN = \
		0 != RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.PDWN.PDWN_Mode;

	TIMING(mc, cha).PDM_MODE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.PDWN.GLPDN;

	TIMING(mc, cha).PDM_AGGR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SNB.PDWN.PDWN_Mode;

      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned int width = 1;

	if (slot == 0) {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DANOR;

		width += RO(Proc)->Uncore.MC[mc].SNB.MAD0.DAW;
	} else {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBNOR;

		width += RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBW;
	}
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks++;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4 << width;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << (14 + width);
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 10;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						dimmSize[cha][slot] * 256;
      }
	TIMING(mc, cha).ECC = (cha == 0) ?
					  RO(Proc)->Uncore.MC[mc].SNB.MAD0.ECC
					: RO(Proc)->Uncore.MC[mc].SNB.MAD1.ECC;
    }
  }
}

const unsigned long long SNB_MCLK(RO(PROC) *RO(Proc),
				const unsigned long long fallback)
{
	switch (RO(Proc)->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b111:
		return 1067;
	case 0b110:
		return 1333;
	case 0b101:
		return 1600;
	case 0b100:
		return 1867;
	case 0b011:
		return 2133;
	case 0b010:
		return 2400;
	case 0b001:
		return 2667;
	case 0b000:
		return fallback;
	}
}

void SNB_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.SNB_Cap.DMFC) {
	case 0b111:
		RO(Shm)->Uncore.CtrlSpeed = 1067;
		break;
	case 0b110:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		RO(Shm)->Uncore.CtrlSpeed = SNB_MCLK(RO(Proc), 1600);
		break;
	case 0b100:
		RO(Shm)->Uncore.CtrlSpeed = 1867;
		break;
	case 0b011:
		RO(Shm)->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		RO(Shm)->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		RO(Shm)->Uncore.CtrlSpeed = 2667;
		break;
	case 0b000:
		RO(Shm)->Uncore.CtrlSpeed = 2933;
		break;
	}

	RO(Shm)->Uncore.Bus.Rate = RO(Shm)->Uncore.CtrlSpeed;
	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 3;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_UNSPEC;

	RO(Shm)->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.SNB_Cap.VT_d;
}

void SNB_EP_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

    for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
    {
     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
      {
	TIMING(mc, cha).tCL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCL;

	TIMING(mc, cha).tRCD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRCD;

	TIMING(mc, cha).tRP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRP;

	TIMING(mc, cha).tCWL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCWL;

	TIMING(mc, cha).tRAS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRAS;

	TIMING(mc, cha).tRRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tRRD;

	TIMING(mc, cha).tRFC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.EP.tRFC;

	TIMING(mc, cha).tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.EP.tREFI;

	TIMING(mc, cha).tWR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWR;

	TIMING(mc, cha).tRTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tRTPr;

	TIMING(mc, cha).tWTPr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWTPr;

	TIMING(mc, cha).tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tFAW;

	TIMING(mc, cha).tCKE = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tCKE;

	TIMING(mc, cha).tddWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWRDD;

	TIMING(mc, cha).tdrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWRDR;

	TIMING(mc, cha).tsrWrTRd = \
		4U + RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCWL
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWTPr;

	TIMING(mc, cha).tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWDD;

	TIMING(mc, cha).tdrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWDR;

	TIMING(mc, cha).tsrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWSR;

	TIMING(mc, cha).tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRDD;

	TIMING(mc, cha).tdrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRDR;
/* TODO
	TIMING(mc, cha).tsrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRSR;
*/
	TIMING(mc, cha).tddWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWDD;

	TIMING(mc, cha).tdrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWDR;
/* TODO
	TIMING(mc, cha).tsrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWSR;
*/
	TIMING(mc, cha).B2B = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tCCD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.CMD_Stretch)
	{
	case 0b00:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	default:
		TIMING(mc, cha).CMD_Rate = 0;
		break;
	}

	TIMING(mc, cha).tXS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SNB_EP.SRFTP.EP.tXS_DLL;

	for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
	{
	    if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.DIMM_POP)
	    {
		unsigned long long DIMM_Size;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4 << \
		RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.DDR3_WIDTH;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1 << \
		RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.RANK_CNT;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << ( 13
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.RA_WIDTH);

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << ( 10
		+ RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.CA_WIDTH);

		DIMM_Size = 8LLU
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						(unsigned int)(DIMM_Size >> 20);
	    }
	}

	TIMING(mc, cha).ECC = (unsigned int)\
				( RO(Proc)->Uncore.MC[mc].SNB_EP.TECH.ECC_EN
				& !RO(Proc)->Uncore.Bus.SNB_EP_Cap3.ECC_DIS );
      }
    }
}

void SNB_EP_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.SNB_EP_Cap1.DMFC) {
	case 0b111:
		RO(Shm)->Uncore.CtrlSpeed = 1066;
		break;
	case 0b110:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		RO(Shm)->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		RO(Shm)->Uncore.CtrlSpeed = 1866;
		break;
	case 0b011:
		RO(Shm)->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		RO(Shm)->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		RO(Shm)->Uncore.CtrlSpeed = 2666;
		break;
	case 0b000:
		RO(Shm)->Uncore.CtrlSpeed = 2933;
		break;
	}

	RO(Shm)->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	RO(Shm)->Uncore.CtrlSpeed /= RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Bus.Rate = \
		RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b010 ?
	5600 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b011 ?
	6400 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b100 ?
	7200 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b101 ?
	8000 : 5000;

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 3;

	if (RO(Proc)->Uncore.Bus.SNB_EP_Cap3.RDIMM_DIS)
	{
		if (RO(Proc)->Uncore.Bus.SNB_EP_Cap3.UDIMM_DIS) {
			RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_UNSPEC;
		} else {
			RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
		}
	} else {
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_RDIMM;
	}
/* TODO(I/O MMU capabiility registers for SandyBridge-EP) */
	RO(Shm)->Proc.Technology.IOMMU = 0;
}

void IVB_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.IVB_Cap.DMFC) {
	case 0b111:
		RO(Shm)->Uncore.CtrlSpeed = 1067;
		break;
	case 0b110:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		RO(Shm)->Uncore.CtrlSpeed = SNB_MCLK(RO(Proc), 1600);
		break;
	case 0b100:
		RO(Shm)->Uncore.CtrlSpeed = 1867;
		break;
	case 0b011:
		RO(Shm)->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		RO(Shm)->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		RO(Shm)->Uncore.CtrlSpeed = 2667;
		break;
	case 0b000:
		switch (RO(Proc)->ArchID) {
		case IvyBridge:
		case Haswell_ULT:
			RO(Shm)->Uncore.CtrlSpeed = 2933;
			break;
		case Haswell_DT:
		case Haswell_EP:
		case Haswell_ULX:
			RO(Shm)->Uncore.CtrlSpeed = 2667;
			break;
		case Broadwell:
		case Broadwell_D:
		case Broadwell_H:
		case Broadwell_EP:
			RO(Shm)->Uncore.CtrlSpeed = 3200;
			break;
		}
		break;
	}

	RO(Shm)->Uncore.Bus.Rate = RO(Shm)->Uncore.CtrlSpeed;
	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 3;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_UNSPEC;

	RO(Shm)->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.SNB_Cap.VT_d;
}

void HSW_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha, slot;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
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

      RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
      RO(Shm)->Uncore.MC[mc].ChannelCount=RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL   = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank.tCL;

	TIMING(mc, cha).tCWL  = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank.tCWL;
/*TODO(Not Found)
	TIMING(mc, cha).tWR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW._.tWR;

	TIMING(mc, cha).tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW._.tFAW;

	TIMING(mc, cha).B2B = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW._.B2B;
*/
	TIMING(mc, cha).tRP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRP;

	TIMING(mc, cha).tRRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRRD;

	TIMING(mc, cha).tRCD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRCD;

	TIMING(mc, cha).tRAS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.REG4C00.tRAS;

	TIMING(mc, cha).tRFC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Refresh.tRFC;

	TIMING(mc, cha).tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Refresh.tREFI;

	TIMING(mc, cha).tsrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.tsrRdTRd;

	TIMING(mc, cha).tdrRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.tdrRdTRd;

	TIMING(mc, cha).tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.tddRdTRd;

	TIMING(mc, cha).tsrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tsrWrTRd;

	TIMING(mc, cha).tdrWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tdrWrTRd;

	TIMING(mc, cha).tddWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tddWrTRd;

	TIMING(mc, cha).tsrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tsrWrTWr;

	TIMING(mc, cha).tdrWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tdrWrTWr;

	TIMING(mc, cha).tddWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_A.tddWrTWr;

	TIMING(mc, cha).tsrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tsrRdTWr;

	TIMING(mc, cha).tdrRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tdrRdTWr;

	TIMING(mc, cha).tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Rank_B.tddRdTWr;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].HSW.Timing.CMD_Stretch) {
	case 0b00:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	default:
		TIMING(mc, cha).CMD_Rate = 0;
		break;
	}

      for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
      {
	unsigned int width, DIMM_Banks;

	if (slot == 0) {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DANOR;

		width = RO(Proc)->Uncore.MC[mc].SNB.MAD0.DAW;
	} else {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
					RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBNOR;

		width = RO(Proc)->Uncore.MC[mc].SNB.MAD0.DBW;
	}
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks++;

	if (width == 0) {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 14;
	} else {
		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << 15;
	}

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << 10;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
						dimmSize[cha][slot] * 256;

	DIMM_Banks = 8 * dimmSize[cha][slot] * 1024 * 1024;

	DIMM_Banks = DIMM_Banks
		/ (RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
		*  RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
		*  RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = DIMM_Banks;
      }
	TIMING(mc, cha).ECC = (cha == 0) ?
					  RO(Proc)->Uncore.MC[mc].SNB.MAD0.ECC
					: RO(Proc)->Uncore.MC[mc].SNB.MAD1.ECC;
    }
    if (RO(Proc)->Uncore.MC[mc].SNB.MADCH.LPDDR) {
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_LPDDR;
    } else {
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
    }
  }
}

void HSW_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	IVB_CAP(RO(Shm), RO(Proc), RO(Core));

	switch (RO(Proc)->Uncore.Bus.HSW_BIOS.MEMCLK) {
	case 0b0101:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b0110:
		RO(Shm)->Uncore.CtrlSpeed = 1600;
		break;
	}
}

#define HSW_EP_IMC	SNB_EP_IMC

void HSW_EP_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	switch (RO(Proc)->Uncore.Bus.SNB_EP_Cap1.DMFC) {
	case 0b111:
		RO(Shm)->Uncore.CtrlSpeed = 1066;
		break;
	case 0b110:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		RO(Shm)->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		RO(Shm)->Uncore.CtrlSpeed = 1866;
		break;
	case 0b011:
		RO(Shm)->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		RO(Shm)->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
		RO(Shm)->Uncore.CtrlSpeed = 2666;
		break;
	case 0b000:
		RO(Shm)->Uncore.CtrlSpeed = 2933;
		break;
	}

	RO(Shm)->Uncore.CtrlSpeed *= RO(Core)->Clock.Hz;
	RO(Shm)->Uncore.CtrlSpeed /= RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Bus.Rate = \
		RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b010 ?
	5600 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b011 ?
	6400 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b100 ?
	7200 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b101 ?
	8000 :	RO(Proc)->Uncore.Bus.QuickPath.EP.QPIFREQSEL == 0b111 ?
	9600 : 6400;

	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;

	if (RO(Proc)->Uncore.MC[0].HSW_EP.TECH.DDR4_Mode) {
		RO(Shm)->Uncore.Unit.DDR_Ver = 4;
	} else {
		RO(Shm)->Uncore.Unit.DDR_Ver = 3;
	}
	if (RO(Proc)->Uncore.Bus.SNB_EP_Cap3.RDIMM_DIS)
	{
		if (RO(Proc)->Uncore.Bus.SNB_EP_Cap3.UDIMM_DIS) {
			RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_UNSPEC;
		} else {
			RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
		}
	} else {
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_RDIMM;
	}
/*TODO(VT-d capability from device 30 in CAPID# registers among offsets 0x80)*/
	RO(Shm)->Proc.Technology.IOMMU = 0;
	RO(Shm)->Proc.Technology.IOMMU_Ver_Major = 0;
	RO(Shm)->Proc.Technology.IOMMU_Ver_Minor = 0;
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

void SKL_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCL;

	TIMING(mc, cha).tRCD  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRP;

	TIMING(mc, cha).tRP   = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRP;

	TIMING(mc, cha).tRAS  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRAS;

	TIMING(mc, cha).tRRDS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ACT.tRRD_DG;

	TIMING(mc, cha).tRRDL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ACT.tRRD_SG;

	TIMING(mc, cha).tRFC  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Refresh.tRFC;

	TIMING(mc, cha).tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Refresh.tREFI;

      if (RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tWRPRE >=
		(RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCWL + 4U))
      {
	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCWL -4U;
      }

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tRDPRE;

	TIMING(mc, cha).tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Timing.tWRPRE;

	TIMING(mc, cha).tFAW  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ACT.tFAW;

	TIMING(mc, cha).tCWL  = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.ODT.tCWL;

	TIMING(mc, cha).tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_SG;

	TIMING(mc, cha).tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_DG;

	TIMING(mc, cha).tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_DR;

	TIMING(mc, cha).tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDRD.tRDRD_DD;

	TIMING(mc, cha).tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_SG;

	TIMING(mc, cha).tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_DG;

	TIMING(mc, cha).tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_DR;

	TIMING(mc, cha).tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.RDWR.tRDWR_DD;

	TIMING(mc, cha).tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_SG;

	TIMING(mc, cha).tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_DG;

	TIMING(mc, cha).tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_DR;

	TIMING(mc, cha).tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRRD.tWRRD_DD;

	TIMING(mc, cha).tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_SG;

	TIMING(mc, cha).tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_DG;

	TIMING(mc, cha).tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_DR;

	TIMING(mc, cha).tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.CMD_Stretch) {
	case 0b00:
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b01:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	}

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.DRAM_Tech == 0b00 ? 16:8;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.x8_device_Dimm0 ?
			1024 : 512;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.x8_device_Dimm1 ?
			1024 : 512;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.tCKE;

	TIMING(mc, cha).tCPDED = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].SKL.Sched.tCPDED;
    }
	RO(Shm)->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].SKL.MADC0.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].SKL.MADC1.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD0.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD0.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD1.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Size = 1024 * RO(Proc)->Uncore.MC[mc].SKL.MADD1.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD0.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD0.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD1.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].SKL.MADD1.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD0.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD0.DSW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD1.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].SKL.MADD1.DSW);

    switch (RO(Proc)->Uncore.MC[mc].SKL.MADCH.DDR_TYPE) {
    case 0b00:
	RO(Shm)->Uncore.Unit.DDR_Ver = 4;
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
	break;
    case 0b01:
	RO(Shm)->Uncore.Unit.DDR_Ver = 3;
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;
	break;
    case 0b10:
	RO(Shm)->Uncore.Unit.DDR_Ver = 3;
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_LPDDR;
	break;
    default:
	RO(Shm)->Uncore.Unit.DDR_Ver = 4;
	RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_UNSPEC;
	break;
    }
  }
}

void SKL_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned int DMFC;

	switch (RO(Proc)->ArchID) {
	case Skylake_UY:
	case Kabylake_UY:
		DMFC = RO(Proc)->Uncore.Bus.SKL_Cap_B.DMFC_DDR3;
		RO(Shm)->Uncore.Bus.Rate = 4000;	/* 4 GT/s QPI */
		break;
	default:
		DMFC = RO(Proc)->Uncore.Bus.SKL_Cap_C.DMFC_DDR4;
		RO(Shm)->Uncore.Bus.Rate = 8000;	/* 8 GT/s DMI3 */
		break;
	}
    if (RO(Proc)->Uncore.Bus.SKL_SA_Pll.QCLK == 0)
    {
	switch (DMFC) {
	case 0b111:
		RO(Shm)->Uncore.CtrlSpeed = 1067;
		break;
	case 0b110:
		RO(Shm)->Uncore.CtrlSpeed = 1333;
		break;
	case 0b101:
		RO(Shm)->Uncore.CtrlSpeed = 1600;
		break;
	case 0b100:
		RO(Shm)->Uncore.CtrlSpeed = 1867;
		break;
	case 0b011:
		RO(Shm)->Uncore.CtrlSpeed = 2133;
		break;
	case 0b010:
		RO(Shm)->Uncore.CtrlSpeed = 2400;
		break;
	case 0b001:
	case 0b000:
		RO(Shm)->Uncore.CtrlSpeed = 2667;
		break;
	}
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
    }
    else
    {
	unsigned long long Freq_Hz;

	if (RO(Proc)->Uncore.Bus.SKL_SA_Pll.QCLK_REF == 0) {
		Freq_Hz = 133333333LLU * RO(Proc)->Uncore.Bus.SKL_SA_Pll.QCLK;
		Freq_Hz = Freq_Hz / 1000000LLU;
	} else {
		Freq_Hz = 100LLU * RO(Proc)->Uncore.Bus.SKL_SA_Pll.QCLK;
	}
	RO(Shm)->Uncore.CtrlSpeed = (unsigned short) Freq_Hz;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
    }
	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;

	RO(Shm)->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.SKL_Cap_A.VT_d;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Major = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Minor = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void RKL_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCL;

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRP;

	TIMING(mc, cha).tRP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRP;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRAS;

	TIMING(mc, cha).tRRDS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ACT.tRRD_DG;

	TIMING(mc, cha).tRRDL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ACT.tRRD_SG;

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Refresh.tRFC;

	TIMING(mc, cha).tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Refresh.tREFI;

      if (RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tWRPRE >=
		(RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCWL + 4U))
      {
	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCWL -4U;
      }

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tRDPRE;

	TIMING(mc, cha).tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Timing.tWRPRE;

	TIMING(mc, cha).tFAW = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ACT.tFAW;

	TIMING(mc, cha).tCWL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.ODT.tCWL;

	TIMING(mc, cha).tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_SG;

	TIMING(mc, cha).tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_DG;

	TIMING(mc, cha).tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_DR;

	TIMING(mc, cha).tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDRD.tRDRD_DD;

	TIMING(mc, cha).tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_SG;

	TIMING(mc, cha).tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_DG;

	TIMING(mc, cha).tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_DR;

	TIMING(mc, cha).tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.RDWR.tRDWR_DD;

	TIMING(mc, cha).tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_SG;

	TIMING(mc, cha).tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_DG;

	TIMING(mc, cha).tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_DR;

	TIMING(mc, cha).tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRRD.tWRRD_DD;

	TIMING(mc, cha).tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_SG;

	TIMING(mc, cha).tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_DG;

	TIMING(mc, cha).tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_DR;

	TIMING(mc, cha).tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.CMD_Stretch) {
	case 0b00:
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b01:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	}

	TIMING(mc, cha).tXS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.SRExit.tXSR;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	!RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.ReservedBits1 ? 16 : 8;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.x8_device_Dimm0 ?
			1024 : 512;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.x8_device_Dimm1 ?
			1024 : 512;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.PWDEN.tCKE;

	TIMING(mc, cha).tXP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.PWDEN.tXP;

	TIMING(mc, cha).tCPDED = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.tCPDED;

	TIMING(mc, cha).GEAR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].RKL.Sched.GEAR2 ? 2 : 1;
    }
	RO(Shm)->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].RKL.MADC0.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].RKL.MADC1.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD0.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD0.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD1.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].RKL.MADD1.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD0.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD0.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD1.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].RKL.MADD1.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD0.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD0.DSW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD1.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].RKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].RKL.MADD1.DSW);
  }
}

void RKL_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned int units = 12;
	unsigned short mc, clock_done;

    if (RO(Proc)->Uncore.Bus.RKL_SA_Pll.UCLK_RATIO > 0) {/* Ring Interconnect */
	RO(Shm)->Uncore.Bus.Rate = RO(Proc)->Uncore.Bus.RKL_SA_Pll.UCLK_RATIO;
	RO(Shm)->Uncore.Bus.Rate *= 100;
	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
    } else {					/* Advertised Bus Speed */
	RO(Shm)->Uncore.Bus.Rate = 8000;
	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
    }
	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

  for (mc = 0, clock_done = 0;
	mc < RO(Shm)->Uncore.CtrlCount && !clock_done;
		mc++)
  {
    if (RO(Proc)->Uncore.MC[mc].RKL.MADCH.value) {
	switch (RO(Proc)->Uncore.MC[mc].RKL.MADCH.DDR_TYPE) {
	default:
	case 0b00:	/*	DDR4	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 4;
		RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_SDRAM;

		if ((RO(Proc)->Uncore.Bus.RKL_Cap_C.DDR4_EN)
		 && (RO(Proc)->Uncore.Bus.RKL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.RKL_Cap_C.DATA_RATE_DDR4;
			clock_done = 1;
		}
		break;
	case 0b11:	/*	LPDDR4	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 4;
		RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_LPDDR;

		if ((RO(Proc)->Uncore.Bus.RKL_Cap_C.LPDDR4_EN)
		 && (RO(Proc)->Uncore.Bus.RKL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.RKL_Cap_C.DATA_RATE_LPDDR4;
			clock_done = 1;
		}
		break;
	case 0b01:	/*	DDR5	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 5;
		RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_SDRAM;

		if ((RO(Proc)->Uncore.Bus.TGL_Cap_E.DDR5_EN)
		 && (RO(Proc)->Uncore.Bus.TGL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.TGL_Cap_E.DATA_RATE_DDR5;
			clock_done = 1;
		}
		break;
	case 0b10:	/*	LPDDR5	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 5;
		RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_LPDDR;

		if ((RO(Proc)->Uncore.Bus.TGL_Cap_E.LPDDR5_EN)
		 && (RO(Proc)->Uncore.Bus.TGL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.TGL_Cap_E.DATA_RATE_LPDDR5;
			clock_done = 1;
		}
		break;
	}
    }
  }
    if (RO(Proc)->Uncore.Bus.RKL_SA_Pll.QCLK_RATIO == 0)
    {
	RO(Shm)->Uncore.CtrlSpeed = (266 * units) + ((334 * units) / 501);
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MTS;
    }
    else	/*	Is Memory frequency overclocked ?		*/
    {
	unsigned long long Freq_Hz;

	if (RO(Proc)->Uncore.Bus.RKL_SA_Pll.QCLK_REF == 0) {
		Freq_Hz = RO(Proc)->Uncore.Bus.RKL_SA_Pll.QCLK_RATIO;
		Freq_Hz = Freq_Hz * RO(Core)->Clock.Hz * 400LLU;
		Freq_Hz = Freq_Hz / RO(Shm)->Proc.Features.Factory.Clock.Hz;
		Freq_Hz = Freq_Hz / 3LLU;
	} else {
		Freq_Hz = RO(Proc)->Uncore.Bus.RKL_SA_Pll.QCLK_RATIO;
		Freq_Hz = Freq_Hz * RO(Core)->Clock.Hz * 100LLU;
		Freq_Hz = Freq_Hz / RO(Shm)->Proc.Features.Factory.Clock.Hz;
	}
	RO(Shm)->Uncore.CtrlSpeed = (unsigned short) Freq_Hz;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
    }
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;

	RO(Shm)->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.RKL_Cap_A.VT_d;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Major = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Minor = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void TGL_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCL;

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRP;

	TIMING(mc, cha).tRP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRP;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRAS;

	TIMING(mc, cha).tRRDS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ACT.tRRD_DG;

	TIMING(mc, cha).tRRDL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ACT.tRRD_SG;

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Refresh.tRFC;

	TIMING(mc, cha).tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Refresh.tREFI;

      if (RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tWRPRE >=
		(RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCWL + 4U))
      {
	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCWL -4U;
      }

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tRDPRE;

	TIMING(mc, cha).tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Timing.tWRPRE;

	TIMING(mc, cha).tFAW = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ACT.tFAW;

	TIMING(mc, cha).tCWL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.ODT.tCWL;

	TIMING(mc, cha).tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_SG;

	TIMING(mc, cha).tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_DG;

	TIMING(mc, cha).tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_DR;

	TIMING(mc, cha).tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDRD.tRDRD_DD;

	TIMING(mc, cha).tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_SG;

	TIMING(mc, cha).tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_DG;

	TIMING(mc, cha).tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_DR;

	TIMING(mc, cha).tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.RDWR.tRDWR_DD;

	TIMING(mc, cha).tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_SG;

	TIMING(mc, cha).tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_DG;

	TIMING(mc, cha).tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_DR;

	TIMING(mc, cha).tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRRD.tWRRD_DD;

	TIMING(mc, cha).tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_SG;

	TIMING(mc, cha).tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_DG;

	TIMING(mc, cha).tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_DR;

	TIMING(mc, cha).tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.CMD_Stretch) {
	case 0b00:
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b01:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	}

	TIMING(mc, cha).tXS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.SRExit.tXSR;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	!RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.ReservedBits1 ? 16 : 8;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.x8_device_Dimm0 ?
			1024 : 512;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.x8_device_Dimm1 ?
			1024 : 512;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.PWDEN.tCKE;

	TIMING(mc, cha).tXP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.PWDEN.tXP;

	TIMING(mc, cha).tCPDED = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.tCPDED;

	TIMING(mc, cha).GEAR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].TGL.Sched.GEAR2 ? 2 : 1;
    }
	RO(Shm)->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].TGL.MADC0.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].TGL.MADC1.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD0.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD0.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD1.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].TGL.MADD1.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD0.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD0.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD1.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].TGL.MADD1.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD0.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD0.DSW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD1.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].TGL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].TGL.MADD1.DSW);
  }
}

#define TGL_CAP RKL_CAP

void ADL_IMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc, cha;

  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
  {
     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

    for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
    {
	TIMING(mc, cha).tCL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ODT.tCL;

	TIMING(mc, cha).tRCD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tRCD;

	TIMING(mc, cha).tRP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tRP;

	TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tRAS;

	TIMING(mc, cha).tRRDS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ACT.tRRD_DG;

	TIMING(mc, cha).tRRDL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ACT.tRRD_SG;

	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Refresh.tRFC;

	TIMING(mc, cha).tREFI = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Refresh.tREFI;

      if (RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tWRPRE >=
		(RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ODT.tCWL + 4U))
      {
	TIMING(mc, cha).tWR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tWRPRE
			- RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ODT.tCWL -4U;
      }

	TIMING(mc, cha).tRTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tRDPRE;

	TIMING(mc, cha).tWTPr = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Timing.tWRPRE;

	TIMING(mc, cha).tFAW = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ACT.tFAW;

	TIMING(mc, cha).tCWL = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.ODT.tCWL;

	TIMING(mc, cha).tRDRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDRD.tRDRD_SG;

	TIMING(mc, cha).tRDRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDRD.tRDRD_DG;

	TIMING(mc, cha).tRDRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDRD.tRDRD_DR;

	TIMING(mc, cha).tRDRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDRD.tRDRD_DD;

	TIMING(mc, cha).tRDWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDWR.tRDWR_SG;

	TIMING(mc, cha).tRDWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDWR.tRDWR_DG;

	TIMING(mc, cha).tRDWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDWR.tRDWR_DR;

	TIMING(mc, cha).tRDWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.RDWR.tRDWR_DD;

	TIMING(mc, cha).tWRRD_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRRD.tWRRD_SG;

	TIMING(mc, cha).tWRRD_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRRD.tWRRD_DG;

	TIMING(mc, cha).tWRRD_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRRD.tWRRD_DR;

	TIMING(mc, cha).tWRRD_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRRD.tWRRD_DD;

	TIMING(mc, cha).tWRWR_SG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRWR.tWRWR_SG;

	TIMING(mc, cha).tWRWR_DG = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRWR.tWRWR_DG;

	TIMING(mc, cha).tWRWR_DR = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRWR.tWRWR_DR;

	TIMING(mc, cha).tWRWR_DD = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.WRWR.tWRWR_DD;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Sched.CMD_Stretch) {
	case 0b00:
	case 0b11:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b01:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 3;
		break;
	}

	TIMING(mc, cha).tXS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.SRExit.tXSR;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Banks = \
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Banks = \
	!RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Sched.ReservedBits1 ? 16 : 8;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[0].Cols = 1 << 10;
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[1].Cols = 1 << 10;

	TIMING(mc, cha).tCKE = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.PWDEN.tCKE;

	TIMING(mc, cha).tXP = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.PWDEN.tXP;

	TIMING(mc, cha).tCPDED = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Sched.tCPDED;

	TIMING(mc, cha).GEAR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Sched.GEAR4 ? 4 : \
		RO(Proc)->Uncore.MC[mc].Channel[cha].ADL.Sched.GEAR2 ? 2 : 1;
    }
	RO(Shm)->Uncore.MC[mc].Channel[0].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].ADL.MADC0.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[1].Timing.ECC = \
				RO(Proc)->Uncore.MC[mc].ADL.MADC1.ECC;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].ADL.MADD0.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].ADL.MADD0.Dimm_S_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].ADL.MADD1.Dimm_L_Size;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Size = 512 * RO(Proc)->Uncore.MC[mc].ADL.MADD1.Dimm_S_Size;

    switch (RO(Shm)->Uncore.Unit.DDR_Ver) {
    case 1 ... 4:
	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].ADL.MADD0.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].ADL.MADD0.DSW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].ADL.MADD1.DLW);

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(RO(Proc)->Uncore.MC[mc].ADL.MADD1.DSW);
	break;
    case 5:
    default:
	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Rows = 1 << 17;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Rows = 1 << 17;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Rows = 1 << 17;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Rows = 1 << 17;
	break;
    }
	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].ADL.MADD0.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[0].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC0.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].ADL.MADD0.DSNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].ADL.MADD1.DLNOR;

	RO(Shm)->Uncore.MC[mc].Channel[1].DIMM[
		!RO(Proc)->Uncore.MC[mc].ADL.MADC1.Dimm_L_Map
	].Ranks = 1 + RO(Proc)->Uncore.MC[mc].ADL.MADD1.DSNOR;
  }
}

void ADL_CAP(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned int units = 12;
	unsigned short mc, clock_done;

    if (RO(Proc)->Uncore.Bus.ADL_SA_Pll.UCLK_RATIO > 0) {/* Ring Interconnect */
	RO(Shm)->Uncore.Bus.Rate = RO(Proc)->Uncore.Bus.ADL_SA_Pll.UCLK_RATIO;
	RO(Shm)->Uncore.Bus.Rate *= 100;
	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
    } else {					/* Advertised Bus Speed */
	RO(Shm)->Uncore.Bus.Rate = 8000;
	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MTS;
    }
	RO(Shm)->Uncore.Bus.Speed = (RO(Core)->Clock.Hz
				* RO(Shm)->Uncore.Bus.Rate)
				/ RO(Shm)->Proc.Features.Factory.Clock.Hz;

  for (mc = 0, clock_done = 0;
	mc < RO(Shm)->Uncore.CtrlCount && !clock_done;
		mc++)
  {
    if (RO(Proc)->Uncore.MC[mc].ADL.MADCH.value) {
	switch (RO(Proc)->Uncore.MC[mc].ADL.MADCH.DDR_TYPE) {
	default:
	case 0b00:	/*	DDR4	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 4;
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;

		if ((RO(Proc)->Uncore.Bus.ADL_Cap_C.DDR4_EN)
		 && (RO(Proc)->Uncore.Bus.ADL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.ADL_Cap_C.DATA_RATE_DDR4;
			clock_done = 1;
		}
		break;
	case 0b11:	/*	LPDDR4	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 4;
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_LPDDR;

		if ((RO(Proc)->Uncore.Bus.ADL_Cap_C.LPDDR4_EN)
		 && (RO(Proc)->Uncore.Bus.ADL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.ADL_Cap_C.DATA_RATE_LPDDR4;
			clock_done = 1;
		}
		break;
	case 0b01:	/*	DDR5	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 5;
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_SDRAM;

		if ((RO(Proc)->Uncore.Bus.ADL_Cap_E.DDR5_EN)
		 && (RO(Proc)->Uncore.Bus.ADL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.ADL_Cap_E.DATA_RATE_DDR5;
			clock_done = 1;
		}
		break;
	case 0b10:	/*	LPDDR5	*/
		RO(Shm)->Uncore.Unit.DDR_Ver = 5;
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_LPDDR;

		if ((RO(Proc)->Uncore.Bus.ADL_Cap_E.LPDDR5_EN)
		 && (RO(Proc)->Uncore.Bus.ADL_Cap_A.DDR_OVERCLOCK == 0))
		{
			units = RO(Proc)->Uncore.Bus.ADL_Cap_E.DATA_RATE_LPDDR5;
			clock_done = 1;
		}
		break;
	}
    }
  }
    if (RO(Proc)->Uncore.Bus.ADL_SA_Pll.QCLK_RATIO == 0)
    {
	RO(Shm)->Uncore.CtrlSpeed = (266 * units) + ((334 * units) / 501);
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MTS;
    }
    else	/*	Is Memory frequency overclocked ?		*/
    {
	unsigned long long Freq_Hz;

	if (RO(Proc)->Uncore.Bus.ADL_SA_Pll.QCLK_REF == 0) {
		Freq_Hz = RO(Proc)->Uncore.Bus.ADL_SA_Pll.QCLK_RATIO;
		Freq_Hz = Freq_Hz * RO(Core)->Clock.Hz * 400LLU;
		Freq_Hz = Freq_Hz / RO(Shm)->Proc.Features.Factory.Clock.Hz;
		Freq_Hz = Freq_Hz / 3LLU;
	} else {
		Freq_Hz = RO(Proc)->Uncore.Bus.ADL_SA_Pll.QCLK_RATIO;
		Freq_Hz = Freq_Hz * RO(Core)->Clock.Hz * 100LLU;
		Freq_Hz = Freq_Hz / RO(Shm)->Proc.Features.Factory.Clock.Hz;
	}
	RO(Shm)->Uncore.CtrlSpeed = (unsigned short) Freq_Hz;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
    }
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;

	RO(Shm)->Proc.Technology.IOMMU = !RO(Proc)->Uncore.Bus.ADL_Cap_A.VT_d;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Major = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Major;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Minor = \
					RO(Proc)->Uncore.Bus.IOMMU_Ver.Minor;
}

void AMD_0Fh_MCH(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
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

    for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
    {
     RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;
     RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++) {
	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tCL) {
	case 0b010:
		TIMING(mc, cha).tCL = 3;
		break;
	case 0b011:
		TIMING(mc, cha).tCL = 4;
		break;
	case 0b100:
		TIMING(mc, cha).tCL = 5;
		break;
	case 0b101:
		TIMING(mc, cha).tCL = 6;
		break;
	}

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRCD) {
	case 0b00:
		TIMING(mc, cha).tRCD = 3;
		break;
	case 0b01:
		TIMING(mc, cha).tRCD = 4;
		break;
	case 0b10:
		TIMING(mc, cha).tRCD = 5;
		break;
	case 0b11:
		TIMING(mc, cha).tRCD = 6;
		break;
	}

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRP) {
	case 0b00:
		TIMING(mc, cha).tRP = 3;
		break;
	case 0b01:
		TIMING(mc, cha).tRP = 4;
		break;
	case 0b10:
		TIMING(mc, cha).tRP = 5;
		break;
	case 0b11:
		TIMING(mc, cha).tRP = 6;
		break;
	}

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRTPr) {
	case 0b0:
		if (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRL.BurstLength32 == 0b1) {
			TIMING(mc, cha).tRTPr = 2;
		} else {
			TIMING(mc, cha).tRTPr = 4;
		}
		break;
	case 0b1:
		if (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRL.BurstLength32 == 0b1) {
			TIMING(mc, cha).tRTPr = 3;
		} else {
			TIMING(mc, cha).tRTPr = 5;
		}
		break;
	}

	if (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRAS >= 0b0010) {
		TIMING(mc, cha).tRAS = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRAS+3;
	}
	TIMING(mc, cha).tRFC = \
			RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRC+11;

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tWR) {
	case 0b00:
		TIMING(mc, cha).tWR = 3;
		break;
	case 0b01:
		TIMING(mc, cha).tWR = 4;
		break;
	case 0b10:
		TIMING(mc, cha).tWR = 5;
		break;
	case 0b11:
		TIMING(mc, cha).tWR = 6;
		break;
	}

	switch (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD0Fh.DTRL.tRRD) {
	case 0b00:
		TIMING(mc, cha).tRRD = 2;
		break;
	case 0b01:
		TIMING(mc, cha).tRRD = 3;
		break;
	case 0b10:
		TIMING(mc, cha).tRRD = 4;
		break;
	case 0b11:
		TIMING(mc, cha).tRRD = 5;
		break;
	}

	if ((RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.tFAW > 0b0000)
	 && (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.tFAW <= 0b1101)) {
		TIMING(mc, cha).tFAW = \
				RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.tFAW + 7;
	}

	if (RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRH.SlowAccessMode == 0b1)
		TIMING(mc, cha).CMD_Rate = 2;
	else
		TIMING(mc, cha).CMD_Rate = 1;

	shift = 4 * cha;
	mask  = 0b1111U << shift;

	for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++) {
	  if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].MBA.CSEnable)
	  {
		index =(RO(Proc)->Uncore.MC[mc].MaxDIMMs.AMD0Fh.CS.value & mask)
			>> shift;

		RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = \
			module[index].size;
	  }
	}
	TIMING(mc, cha).ECC = \
			RO(Proc)->Uncore.MC[mc].AMD0Fh.DCRL.ECC_DIMM_Enable;
      }
    }
}

void AMD_0Fh_HTT(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
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
	RO(Shm)->Uncore.CtrlSpeed = (Ratio.Q * 2) + Ratio.R;	/* DDR2 */

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
		RO(Shm)->Uncore.Bus.Rate = HTT_Clock * 2;	/* "MT/s" */
		RO(Shm)->Uncore.Bus.Speed = HTT_Clock * 4;	/* "MB/s" */
	}
	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MTS;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MBS;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 2;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_UNSPEC;
}

void AMD_17h_UMC(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	unsigned short mc;
 for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount; mc++)
 {
    RO(Shm)->Uncore.MC[mc].ChannelCount = RO(Proc)->Uncore.MC[mc].ChannelCount;
    RO(Shm)->Uncore.MC[mc].SlotCount = RO(Proc)->Uncore.MC[mc].SlotCount;

	unsigned short cha;
  for (cha = 0; cha < RO(Shm)->Uncore.MC[mc].ChannelCount; cha++)
  {
	unsigned long long DIMM_Size;
	unsigned short slot;
    for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
    {
	const unsigned short chipselect_pair = slot << 1;
      if (BITVAL(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.CHIP[
			chipselect_pair
		][0].Chip.value, 0)
      )
      { /*			CSEnable				*/
	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8 << \
	RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.DAC.NumBanks;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << (10
	+ RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.DAC.NumRowLo);

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << (5
	+ RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.DAC.NumCol);

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[
			slot
		].AMD17h.CFG.OnDimmMirror ? 2 : 1;

	DIMM_Size = 8LLU;
	DIMM_Size *= RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks;
	DIMM_Size *= RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
	DIMM_Size *= RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows;
	DIMM_Size *= RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols;

	RO(Shm)->Uncore.MC[mc].Channel[cha].DIMM[slot].Size = DIMM_Size >> 20;
      }
    }

	TIMING(mc, cha).tCL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tCL;

	TIMING(mc, cha).tRCD_RD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tRCD_RD;

	TIMING(mc, cha).tRCD_WR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tRCD_WR;

	TIMING(mc, cha).tRP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRP;

	TIMING(mc, cha).tRAS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR1.tRAS;

	TIMING(mc, cha).tRRDS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRRDS;

	TIMING(mc, cha).tRRDL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRRDL;

	TIMING(mc, cha).Zen.tRRDDLR =
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRRDDLR;

	TIMING(mc, cha).tRC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRC;

	TIMING(mc, cha).tRCPB = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRCPB;

	TIMING(mc, cha).tRPPB = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR2.tRPPB;

	TIMING(mc, cha).tFAW = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR4.tFAW;

	TIMING(mc, cha).tFAWSLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR4.tFAWSLR;

	TIMING(mc, cha).tFAWDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR4.tFAWDLR;

	TIMING(mc, cha).tWTRS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR5.tWTRS;

	TIMING(mc, cha).tWTRL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR5.tWTRL;

	TIMING(mc, cha).tWR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR6.tWR;

	TIMING(mc, cha).tRCPage = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR7.tRCPage;

	TIMING(mc, cha).Zen.tRdRdScl = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tRdRdScl;

	TIMING(mc, cha).Zen.tWrWrScl = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tWrWrScl;

	TIMING(mc, cha).Zen.tRdRdBan = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tRdRdBan;

	TIMING(mc, cha).Zen.tWrWrBan = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tWrWrBan;

	TIMING(mc, cha).tCWL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR5.tCWL;

	TIMING(mc, cha).tRTP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR3.tRTP;

	TIMING(mc, cha).Zen.tddRdTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR10.tddRdTWr;

	TIMING(mc, cha).Zen.tddWrTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR10.tddWrTRd;

	TIMING(mc, cha).Zen.tscWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tscWrTWr;

	TIMING(mc, cha).Zen.tsdWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tsdWrTWr;

	TIMING(mc, cha).Zen.tddWrTWr = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tddWrTWr;

	TIMING(mc, cha).Zen.tscRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tscRdTRd;

	TIMING(mc, cha).Zen.tsdRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tsdRdTRd;

	TIMING(mc, cha).Zen.tddRdTRd = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tddRdTRd;

	TIMING(mc, cha).Zen.tRdRdScDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR8.tRdRdScDLR;

	TIMING(mc, cha).Zen.tWrWrScDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR9.tWrWrScDLR;

	TIMING(mc, cha).Zen.tWrRdScDLR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR10.tWrRdScDLR;

	TIMING(mc, cha).tXP = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR21.tXP;

	TIMING(mc, cha).tCKE = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR21.tCKE;

	TIMING(mc, cha).tCPDED = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR21.tCPDED;

	TIMING(mc, cha).tREFI = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR12.tREFI;

	TIMING(mc, cha).tRFC1 = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC.tRFC1;

	TIMING(mc, cha).tRFC2 = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC.tRFC2;

	TIMING(mc, cha).tRFC4 = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC.tRFC4;

	switch(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.CMD_Rate) {
	case 0b00:
		TIMING(mc, cha).CMD_Rate = 1;
		break;
	case 0b10:
		TIMING(mc, cha).CMD_Rate = 2;
		break;
	default:
		TIMING(mc, cha).CMD_Rate = 0;
		break;
	}
	TIMING(mc, cha).ECC = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.ECC.__.Enable;

	TIMING(mc, cha).GDM = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.GearDownMode;

	TIMING(mc, cha).BGS = \
		!((RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.BGS.value
		& AMD_17_UMC_BGS_MASK_OFF) == AMD_17_UMC_BGS_MASK_OFF);

	TIMING(mc, cha).BGS_ALT = \
		(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.BGS_ALT.value
		& AMD_17_UMC_BGS_ALT_MASK_ON) == AMD_17_UMC_BGS_ALT_MASK_ON;

	TIMING(mc, cha).PDM_EN = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ.PwrDownEn;

	TIMING(mc, cha).PDM_MODE = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ.PwrDownMode;

	TIMING(mc, cha).PDM_AGGR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ.AggrPwrDownEn;

	TIMING(mc, cha).tMRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMRD;

	TIMING(mc, cha).tMOD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMOD;

	TIMING(mc, cha).tMRD_PDA = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMRD_PDA;

	TIMING(mc, cha).tMOD_PDA = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR13.tMOD_PDA;

	TIMING(mc, cha).tXS = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR14.tXS;

	TIMING(mc, cha).tSTAG = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR20.tSTAG;

	TIMING(mc, cha).tPHYWRD = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tPHY_WRDATA;

	TIMING(mc, cha).tPHYWRL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tPHY_WRLAT;

	TIMING(mc, cha).tPHYRDL = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tPHY_RDLAT;

	TIMING(mc, cha).tRDDATA = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR22.tRDDATA_EN;

	TIMING(mc, cha).tWRMPR = \
		RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.DTR35.tWR_MPR;
  }
 }
}

void AMD_17h_CAP(RO(SHM_STRUCT) *RO(Shm),
		RO(PROC) *RO(Proc), RO(CORE) *RO(Core))
{
	unsigned short mc, clock_done = 0;
  for (mc = 0; mc < RO(Shm)->Uncore.CtrlCount && !clock_done; mc++)
  {
	unsigned short cha;
    for (cha = 0;
	cha < RO(Shm)->Uncore.MC[mc].ChannelCount && !clock_done;
		cha++)
    {
      if (RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK)
      {
	unsigned short slot;

	RO(Shm)->Uncore.Bus.Rate = \
			(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK
			*  RO(Shm)->Proc.Features.Factory.Clock.Q) / 3;

	RO(Shm)->Uncore.Bus.Speed = \
			(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK
			* RO(Core)->Clock.Hz * 333333333LLU)
			/ (RO(Shm)->Proc.Features.Factory.Clock.Hz
			* (1000LLU * PRECISION * PRECISION));

	RO(Shm)->Uncore.CtrlSpeed = \
			(RO(Proc)->Uncore.MC[mc].Channel[cha].AMD17h.MISC.MEMCLK
			* RO(Core)->Clock.Hz * 666666666LLU)
			/ (RO(Shm)->Proc.Features.Factory.Clock.Hz
			* (1000LLU * PRECISION * PRECISION));

	RO(Shm)->Uncore.Unit.Bus_Rate = MC_MHZ;
	RO(Shm)->Uncore.Unit.BusSpeed = MC_MHZ;
	RO(Shm)->Uncore.Unit.DDR_Rate = MC_NIL;
	RO(Shm)->Uncore.Unit.DDRSpeed = MC_MTS;
	RO(Shm)->Uncore.Unit.DDR_Ver  = 4;
	RO(Shm)->Uncore.Unit.DDR_Std  = RAM_STD_SDRAM;

       for (slot = 0; slot < RO(Shm)->Uncore.MC[mc].SlotCount; slot++)
       {
	if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.CFG.value \
		!= 0xffffffff)
	{
	  if (RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.CFG.RDIMM
	   || RO(Proc)->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.CFG.LRDIMM)
	  {
		RO(Shm)->Uncore.Unit.DDR_Std = RAM_STD_RDIMM;
		break;
	  }
	}
       }
	clock_done = 1;
      }
    }
  }
}

void AMD_17h_IOMMU(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc))
{
	RO(Shm)->Proc.Technology.IOMMU = RO(Proc)->Uncore.Bus.IOMMU_CR.IOMMU_En;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Major = \
			(RO(Proc)->Uncore.Bus.IOMMU_HDR.CapRev & 0b10000) >> 5;

	RO(Shm)->Proc.Technology.IOMMU_Ver_Minor = \
			RO(Proc)->Uncore.Bus.IOMMU_HDR.CapRev & 0b01111;
}

#undef TIMING

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
	[IC_PINEVIEW]		= "Pineview",
	[IC_CEDARVIEW]		= "Cedarview",
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
	[IC_Z690]		= "Intel Z690",
	[IC_K8] 		= "K8/HyperTransport",
	[IC_ZEN]		= "Zen UMC"
};

#define SET_CHIPSET(ic) 						\
({									\
	RO(Shm)->Uncore.ChipID = DID;					\
	RO(Shm)->Uncore.Chipset.ArchID = ic;				\
})

void PCI_Intel(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core),
		unsigned short DID)
{
	switch (DID) {
	case DID_INTEL_82945P_HB:
		P945_CLK(RO(Shm), RO(Proc), RO(Core));
		P945_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_LAKEPORT);
		break;
	case DID_INTEL_82945GM_HB:
	case DID_INTEL_82945GME_HB:
		P945_CLK(RO(Shm), RO(Proc), RO(Core));
		P945_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CALISTOGA);
		break;
	case DID_INTEL_82955_HB:
		P945_CLK(RO(Shm), RO(Proc), RO(Core));
		P955_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_LAKEPORT_X);
		break;
	case DID_INTEL_82946GZ_HB:
		P965_CLK(RO(Shm), RO(Proc), RO(Core));
		P965_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_LAKEPORT_P);
		break;
	case DID_INTEL_82965Q_HB:
	case DID_INTEL_82965G_HB:
		P965_CLK(RO(Shm), RO(Proc), RO(Core));
		P965_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_BROADWATER);
		break;
	case DID_INTEL_82965GM_HB:
	case DID_INTEL_82965GME_HB:
		G965_CLK(RO(Shm), RO(Proc), RO(Core));
		G965_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CRESTLINE);
		break;
	case DID_INTEL_GM45_HB:
		G965_CLK(RO(Shm), RO(Proc), RO(Core));
		G965_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANTIGA);
		break;
	case DID_INTEL_Q35_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P35_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_Q);
		break;
	case DID_INTEL_G33_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P35_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_P);
		break;
	case DID_INTEL_Q33_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P35_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_QF);
		break;
	case DID_INTEL_X38_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P35_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_BEARLAKE_X);
		break;
	case DID_INTEL_3200_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P35_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_INTEL_3200);
		break;
	case DID_INTEL_Q45_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P4S_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_EAGLELAKE_Q);
		break;
	case DID_INTEL_G45_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P4S_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_EAGLELAKE_P);
		break;
	case DID_INTEL_G41_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P4S_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_EAGLELAKE_G);
		break;
	case DID_INTEL_BONNELL_HB:
		P35_CLK(RO(Shm), RO(Proc), RO(Core));
		P35_MCH(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_PINEVIEW);
		break;
	case DID_INTEL_SALTWELL_HB:
		SLM_PTR(RO(Shm), RO(Proc), RO(Core));
		SET_CHIPSET(IC_CEDARVIEW);
		break;
	case DID_INTEL_SLM_PTR:
		SLM_PTR(RO(Shm), RO(Proc), RO(Core));
		SET_CHIPSET(IC_BAYTRAIL);
		break;
	case DID_INTEL_X58_HUB_CTRL:
		QPI_CLK(RO(Shm), RO(Proc), RO(Core));
		break;
	case DID_INTEL_X58_HUB_CORE:
	case DID_INTEL_IIO_CORE_REG:
		X58_VTD(RO(Shm), RO(Proc), RO(Core));
		break;
	case DID_INTEL_I7_MCR:			/*	Bloomfield	*/
	case DID_INTEL_NHM_EP_MCR:		/*	Westmere EP	*/
		NHM_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_TYLERSBURG);
		break;
	case DID_INTEL_I7_MC_TEST:
	case DID_INTEL_LYNNFIELD_MC_TEST:
	case DID_INTEL_NHM_EP_MC_TEST:
	case DID_INTEL_NHM_EC_MC_TEST:
		DMI_CLK(RO(Shm), RO(Proc), RO(Core));
		break;
	case DID_INTEL_LYNNFIELD_MCR:		/*	Lynnfield	*/
	case DID_INTEL_NHM_EC_MCR:		/*	C5500-C3500	*/
		NHM_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_IBEXPEAK);
		break;
	case DID_INTEL_SNB_IMC_HA0:		/*	Sandy Bridge-E	*/
		SNB_EP_CAP(RO(Shm), RO(Proc), RO(Core));
		SNB_EP_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_PATSBURG);
		break;
	case DID_INTEL_SNB_IMC_SA:		/*	SNB Desktop	*/
		SNB_CAP(RO(Shm), RO(Proc), RO(Core));
		SNB_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_COUGARPOINT);
		break;
	case DID_INTEL_SNB_IMC_0104:
		SNB_CAP(RO(Shm), RO(Proc), RO(Core));
		SNB_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_IBEXPEAK_M);
		break;
	case DID_INTEL_SNB_EP_HOST_BRIDGE:	/* Xeon E5-2640		*/
	case DID_INTEL_IVB_EP_HOST_BRIDGE:	/* Xeon E5 & E7 v2	*/
		SNB_EP_CAP(RO(Shm), RO(Proc), RO(Core));
		SNB_EP_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CAVECREEK);
		break;
	case DID_INTEL_IVB_IMC_SA:		/*	IVB Desktop	*/
		IVB_CAP(RO(Shm), RO(Proc), RO(Core));
		SNB_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_PANTHERPOINT);
		break;
	case DID_INTEL_IVB_IMC_0154:		/* IVB Mobile i5-3337U	*/
		IVB_CAP(RO(Shm), RO(Proc), RO(Core));
		SNB_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_PANTHERPOINT_M);
		break;
	case DID_INTEL_HASWELL_IMC_SA:		/* HSW & BDW Desktop	*/
	case DID_INTEL_HASWELL_MH_IMC_HA0:	/* HSW Mobile M/H	*/
		HSW_CAP(RO(Shm), RO(Proc), RO(Core));
		HSW_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_LYNXPOINT_M);
		break;
	case DID_INTEL_HASWELL_UY_IMC_HA0:	/* HSW Mobile U/Y	*/
		IVB_CAP(RO(Shm), RO(Proc), RO(Core));
		HSW_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_LYNXPOINT_M);
		break;
	case DID_INTEL_HASWELL_IMC_HA0: 		/* Haswell	*/
		IVB_CAP(RO(Shm), RO(Proc), RO(Core));
		HSW_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_LYNXPOINT);
		break;
	case DID_INTEL_HSW_EP_HOST_BRIDGE:
		HSW_EP_CAP(RO(Shm), RO(Proc), RO(Core));
		HSW_EP_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_WELLSBURG);
		break;
	case DID_INTEL_BROADWELL_IMC_HA0:	/* Broadwell/Y/U Core m */
		IVB_CAP(RO(Shm), RO(Proc), RO(Core));
		HSW_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_WILDCATPOINT_M);
		break;
	case DID_INTEL_BROADWELL_D_IMC_HA0:	/*	BDW/Desktop	*/
	case DID_INTEL_BROADWELL_H_IMC_HA0:	/*	Broadwell/H	*/
	case DID_INTEL_BROADWELL_U_IMC_HA0:	/*	Broadwell/U	*/
		IVB_CAP(RO(Shm), RO(Proc), RO(Core));
		HSW_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_WELLSBURG);
		break;
	case DID_INTEL_SKYLAKE_U_IMC_HA:	/* Skylake/U Processor */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_Y_IMC_HA:	/* Skylake/Y Processor */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_S_IMC_HAD:	/* Skylake/S Dual Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_S_IMC_HAQ:	/* Skylake/S Quad Core	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_H_IMC_HAD:	/* Skylake/H Dual Core	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_H_IMC_HAQ:	/* Skylake/H Quad Core	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_SKYLAKE_DT_IMC_HA:	/* Skylake/DT Server	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case DID_INTEL_KABYLAKE_U_IMC_HA:	/*	BGA 1356	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_Y_IMC_HA:	/*	BGA 1515	*/
	case DID_INTEL_KABYLAKE_Y_IMC_HQ:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_H_IMC_HAD:	/* Kaby Lake/H Dual Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_S_IMC_HAD:	/* Kaby Lake/S Dual Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_H_IMC_HAQ:	/* Kaby Lake/H Quad Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_DT_IMC_HA:	/* Kaby Lake/DT Server	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_U_IMC_HAQ:	/* U-Quad Core BGA 1356 */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_S_IMC_HAQ:	/* Kaby Lake/S Quad Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_KABYLAKE_X_IMC_HAQ:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_S_IMC_HAQ:	/* Coffee Lake Quad Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_S_IMC_HAS:	/* Coffee Lake Hexa Core */
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAD:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_U_IMC_HAD:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_U_IMC_HAQ:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_H_IMC_HAQ:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_H_IMC_HAS:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_H_IMC_HAO:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_W_IMC_HAQ:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_W_IMC_HAS:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_W_IMC_HAO:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAQ:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAS:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_COFFEELAKE_R_S_IMC_HAO:
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_WHISKEYLAKE_U_IMC_HAD:	/*	WHL Dual Core	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_WHISKEYLAKE_U_IMC_HAQ:	/*	WHL Quad Core	*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case DID_INTEL_CANNONLAKE_U_IMC_HB:	/*	CNL-U		*/
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
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
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		break;
	case DID_INTEL_COMETLAKE_U_IMC_HB:
	case DID_INTEL_COMETLAKE_U1_IMC:
	case DID_INTEL_COMETLAKE_U3_IMC:
		RKL_CAP(RO(Shm), RO(Proc), RO(Core));
		RKL_IMC(RO(Shm), RO(Proc));
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
		SKL_CAP(RO(Shm), RO(Proc), RO(Core));
		SKL_IMC(RO(Shm), RO(Proc));
		break;
	case DID_INTEL_TIGERLAKE_U1_IMC:
	case DID_INTEL_TIGERLAKE_U2_IMC:
	case DID_INTEL_TIGERLAKE_U3_IMC:
	case DID_INTEL_TIGERLAKE_U4_IMC:
	case DID_INTEL_TIGERLAKE_H_IMC:
		TGL_CAP(RO(Shm), RO(Proc), RO(Core));
		TGL_IMC(RO(Shm), RO(Proc));
		break;
	case DID_INTEL_ROCKETLAKE_S_8C_IMC_HB:
	case DID_INTEL_ROCKETLAKE_S_6C_IMC_HB:
		RKL_CAP(RO(Shm), RO(Proc), RO(Core));
		RKL_IMC(RO(Shm), RO(Proc));
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
	case DID_INTEL_ALDERLAKE_S_8P_8E_IMC:
	case DID_INTEL_ALDERLAKE_S_8P_4E_IMC:
	case DID_INTEL_ALDERLAKE_S_6P_4E_IMC:
		ADL_CAP(RO(Shm), RO(Proc), RO(Core));
		ADL_IMC(RO(Shm), RO(Proc));
		break;
	case DID_INTEL_ALDERLAKE_Z690_PCH:
		SET_CHIPSET(IC_Z690);
		break;
	}
}

void PCI_AMD(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) *RO(Core),
		unsigned short DID)
{
	switch (DID) {
	case DID_AMD_K8_NB_MEMCTL:
		AMD_0Fh_HTT(RO(Shm), RO(Proc));
		AMD_0Fh_MCH(RO(Shm), RO(Proc));
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
		AMD_17h_IOMMU(RO(Shm), RO(Proc));
		break;
	case DID_AMD_17H_ZEPPELIN_DF_UMC:
	case DID_AMD_17H_RAVEN_DF_UMC:
	case DID_AMD_17H_MATISSE_DF_UMC:
	case DID_AMD_17H_STARSHIP_DF_UMC:
	case DID_AMD_17H_RENOIR_DF_UMC:
	case DID_AMD_17H_ARIEL_DF_UMC:
	case DID_AMD_17H_FIREFLIGHT_DF_UMC:
	case DID_AMD_17H_ARDEN_DF_UMC:
		AMD_17h_UMC(RO(Shm), RO(Proc));
		AMD_17h_CAP(RO(Shm), RO(Proc), RO(Core));
		SET_CHIPSET(IC_ZEN);
		break;
	}
}

#undef SET_CHIPSET

void Uncore_Update(	RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc),
			RO(CORE) *RO(Core) )
{
	unsigned int idx;
	/*	Copy the # of controllers.				*/
	RO(Shm)->Uncore.CtrlCount = RO(Proc)->Uncore.CtrlCount;
	/*	Decode the Memory Controller for each found vendor:device */
	Chipset[IC_CHIPSET] = RO(Proc)->Features.Info.Vendor.ID;
	RO(Shm)->Uncore.ChipID = 0x0;
	RO(Shm)->Uncore.Chipset.ArchID = IC_CHIPSET;

  for (idx = 0; idx < CHIP_MAX_PCI; idx++) {
    switch (RO(Proc)->Uncore.Chip[idx].VID) {
    case PCI_VENDOR_ID_INTEL:
	PCI_Intel(RO(Shm), RO(Proc), RO(Core), RO(Proc)->Uncore.Chip[idx].DID);
	break;
    case PCI_VENDOR_ID_AMD:
	PCI_AMD(RO(Shm), RO(Proc), RO(Core), RO(Proc)->Uncore.Chip[idx].DID);
	break;
    }
  }
	/*	Copy the chipset codename.				*/
	StrCopy(RO(Shm)->Uncore.Chipset.CodeName,
		Chipset[RO(Shm)->Uncore.Chipset.ArchID],
		CODENAME_LEN);
	/*	Copy the Uncore clock ratios.				*/
	memcpy( RO(Shm)->Uncore.Boost,
		RO(Proc)->Uncore.Boost,
		(UNCORE_BOOST(SIZE)) * sizeof(unsigned int) );
}

void CPUID_Dump(RO(SHM_STRUCT) *RO(Shm), RO(CORE) **RO(Core), unsigned int cpu)
{	/* Copy the Vendor CPUID dump per Core.				*/
	RO(Shm)->Cpu[cpu].Query.StdFunc = RO(Core, AT(cpu))->Query.StdFunc;
	RO(Shm)->Cpu[cpu].Query.ExtFunc = RO(Core, AT(cpu))->Query.ExtFunc;

	enum CPUID_ENUM i;
    for (i = 0; i < CPUID_MAX_FUNC; i++) {
	RO(Shm)->Cpu[cpu].CpuID[i].func   = RO(Core, AT(cpu))->CpuID[i].func;
	RO(Shm)->Cpu[cpu].CpuID[i].sub    = RO(Core, AT(cpu))->CpuID[i].sub;
	RO(Shm)->Cpu[cpu].CpuID[i].reg[0] = RO(Core, AT(cpu))->CpuID[i].reg[0];
	RO(Shm)->Cpu[cpu].CpuID[i].reg[1] = RO(Core, AT(cpu))->CpuID[i].reg[1];
	RO(Shm)->Cpu[cpu].CpuID[i].reg[2] = RO(Core, AT(cpu))->CpuID[i].reg[2];
	RO(Shm)->Cpu[cpu].CpuID[i].reg[3] = RO(Core, AT(cpu))->CpuID[i].reg[3];
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

void Topology(RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc), RO(CORE) **RO(Core),
		unsigned int cpu)
{
	unsigned int loop;
	/*	Copy each Core topology.				*/
	RO(Shm)->Cpu[cpu].Topology.MP.BSP= (RO(Core, AT(cpu))->T.Base.BSP)? 1:0;
	RO(Shm)->Cpu[cpu].Topology.ApicID     = RO(Core,AT(cpu))->T.ApicID;
	RO(Shm)->Cpu[cpu].Topology.CoreID     = RO(Core,AT(cpu))->T.CoreID;
	RO(Shm)->Cpu[cpu].Topology.ThreadID   = RO(Core,AT(cpu))->T.ThreadID;
	RO(Shm)->Cpu[cpu].Topology.PackageID  = RO(Core,AT(cpu))->T.PackageID;
	RO(Shm)->Cpu[cpu].Topology.Cluster.ID = RO(Core,AT(cpu))->T.Cluster.ID;
	/*	x2APIC capability.					*/
	RO(Shm)->Cpu[cpu].Topology.MP.x2APIC= RO(Proc)->Features.Std.ECX.x2APIC;

    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    ||	(RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	RO(Shm)->Cpu[cpu].Topology.MP.x2APIC |= \
				RO(Proc)->Features.ExtInfo.ECX.ExtApicId;
    }
  else if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	&& (RO(Shm)->Proc.Features.ExtFeature.EDX.Hybrid == 1))
    {
	switch (RO(Core,AT(cpu))->T.Cluster.Hybrid.CoreType) {
	case Hybrid_Atom:
		RO(Shm)->Cpu[cpu].Topology.MP.Ecore = 1;
		break;
	case Hybrid_Core:
		RO(Shm)->Cpu[cpu].Topology.MP.Pcore = 1;
		break;
	}
	RO(Shm)->Cpu[cpu].Topology.Cluster.Hybrid_ID = \
				RO(Core,AT(cpu))->T.Cluster.Hybrid.Model_ID;
    }
	/*	Is local APIC enabled in xAPIC mode ?			*/
	RO(Shm)->Cpu[cpu].Topology.MP.x2APIC &= \
					RO(Core, AT(cpu))->T.Base.APIC_EN;
	/*	Is xAPIC enabled in x2APIC mode ?			*/
	RO(Shm)->Cpu[cpu].Topology.MP.x2APIC = \
					RO(Shm)->Cpu[cpu].Topology.MP.x2APIC
					<< RO(Core, AT(cpu))->T.Base.x2APIC_EN;
	/*	Aggregate the Caches topology.				*/
    for (loop = 0; loop < CACHE_MAX_LEVEL; loop++)
    {
      if (RO(Core, AT(cpu))->T.Cache[loop].Type > 0)
      {
	unsigned int level = RO(Core, AT(cpu))->T.Cache[loop].Level;
	if (RO(Core, AT(cpu))->T.Cache[loop].Type == 2) {/* Instruction	*/
		level = 0;
	}
	if (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
		RO(Shm)->Cpu[cpu].Topology.Cache[level].Set = \
				RO(Core, AT(cpu))->T.Cache[loop].Set + 1;

		RO(Shm)->Cpu[cpu].Topology.Cache[level].LineSz = \
				RO(Core, AT(cpu))->T.Cache[loop].LineSz + 1;

		RO(Shm)->Cpu[cpu].Topology.Cache[level].Part = \
				RO(Core, AT(cpu))->T.Cache[loop].Part + 1;

		RO(Shm)->Cpu[cpu].Topology.Cache[level].Way = \
				RO(Core, AT(cpu))->T.Cache[loop].Way + 1;

		RO(Shm)->Cpu[cpu].Topology.Cache[level].Size = \
				  RO(Shm)->Cpu[cpu].Topology.Cache[level].Set
				* RO(Shm)->Cpu[cpu].Topology.Cache[level].LineSz
				* RO(Shm)->Cpu[cpu].Topology.Cache[level].Part
				* RO(Shm)->Cpu[cpu].Topology.Cache[level].Way;
	} else {
	    if ((RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	     || (RO(Shm)->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
	    {
		RO(Shm)->Cpu[cpu].Topology.Cache[level].Way = \
			(loop == 2) || (loop == 3) ?
				AMD_L2_L3_Way_Associativity(RO(Core), cpu, loop)
				: RO(Core, AT(cpu))->T.Cache[loop].Way;

		RO(Shm)->Cpu[cpu].Topology.Cache[level].Size = \
					RO(Core, AT(cpu))->T.Cache[loop].Size;
	    }
	}
	RO(Shm)->Cpu[cpu].Topology.Cache[level].Feature.WriteBack = \
					RO(Core, AT(cpu))->T.Cache[loop].WrBack;

	RO(Shm)->Cpu[cpu].Topology.Cache[level].Feature.Inclusive = \
					RO(Core, AT(cpu))->T.Cache[loop].Inclus;
      }
    }
	/*	Apply various architecture size unit.			*/
    switch (RO(Proc)->ArchID) {
    case AMD_Family_15h:
	/*TODO: do models 60h & 70h need a 512 KB size unit adjustment ? */
	if ((RO(Shm)->Proc.Features.Std.EAX.ExtModel == 0x6)
	 || (RO(Shm)->Proc.Features.Std.EAX.ExtModel == 0x7)) {
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
    case AMD_Zen3_Badami:
    case AMD_Zen3Plus_RMB:
    case AMD_Family_17h:
    case Hygon_Family_18h:
    case AMD_Family_19h:
    VIRTUALIZED_L3:
	/* CPUID_Fn80000006_EDX: Value in [3FFFh - 0001h] = (<Value> *0.5) MB */
	RO(Shm)->Cpu[cpu].Topology.Cache[3].Size *= 512;
	break;
    default:
	if (RO(Shm)->Proc.Features.Std.ECX.Hyperv) {	/* Virtualized ? */
		goto VIRTUALIZED_L3;
	}
	break;
    }
}

void CStates(RO(SHM_STRUCT) *RO(Shm), RO(CORE) **RO(Core), unsigned int cpu)
{	/*	Copy the C-State Configuration Control			*/
	RO(Shm)->Cpu[cpu].Query.CfgLock = RO(Core, AT(cpu))->Query.CfgLock;
	RO(Shm)->Cpu[cpu].Query.CStateLimit = RO(Core, AT(cpu))->Query.CStateLimit;
	/*	Copy Intel Max C-State Inclusion			*/
	RO(Shm)->Cpu[cpu].Query.IORedir = RO(Core, AT(cpu))->Query.IORedir;

	RO(Shm)->Cpu[cpu].Query.CStateInclude = \
					RO(Core, AT(cpu))->Query.CStateInclude;
	/*	Copy any architectural C-States I/O Base Address	*/
	RO(Shm)->Cpu[cpu].Query.CStateBaseAddr = \
					RO(Core, AT(cpu))->Query.CStateBaseAddr;
}

void PowerThermal(	RO(SHM_STRUCT) *RO(Shm), RO(PROC) *RO(Proc),
			RO(CORE) **RO(Core), unsigned int cpu )
{
	RO(Shm)->Cpu[cpu].PowerThermal.DutyCycle.Extended = \
		RO(Core, AT(cpu))->PowerThermal.ClockModulation.ECMD;

	RO(Shm)->Cpu[cpu].PowerThermal.DutyCycle.ClockMod = \
		RO(Core, AT(cpu))->PowerThermal.ClockModulation.DutyCycle
			>> !RO(Shm)->Cpu[cpu].PowerThermal.DutyCycle.Extended;

	RO(Shm)->Cpu[cpu].PowerThermal.PowerPolicy = \
		RO(Core, AT(cpu))->PowerThermal.PerfEnergyBias.PowerPolicy;

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
	RO(Shm)->Cpu[cpu].SystemRegister.RFLAGS = \
				RO(Core, AT(cpu))->SystemRegister.RFLAGS;

	RO(Shm)->Cpu[cpu].SystemRegister.CR0 = \
				RO(Core, AT(cpu))->SystemRegister.CR0;

	RO(Shm)->Cpu[cpu].SystemRegister.CR3 = \
				RO(Core, AT(cpu))->SystemRegister.CR3;

	RO(Shm)->Cpu[cpu].SystemRegister.CR4 = \
				RO(Core, AT(cpu))->SystemRegister.CR4;

	RO(Shm)->Cpu[cpu].SystemRegister.CR8 = \
				RO(Core, AT(cpu))->SystemRegister.CR8;

	RO(Shm)->Cpu[cpu].SystemRegister.EFER = \
				RO(Core, AT(cpu))->SystemRegister.EFER;

	RO(Shm)->Cpu[cpu].SystemRegister.EFCR = \
				RO(Core, AT(cpu))->SystemRegister.EFCR;
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

#define CLOCKSOURCE_PATH "/sys/devices/system/clocksource/clocksource0"
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
#undef CLOCKSOURCE_PATH

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

	RO(Shm)->SysGate.memInfo.totalram  = SysGate->memInfo.totalram;
	RO(Shm)->SysGate.memInfo.sharedram = SysGate->memInfo.sharedram;
	RO(Shm)->SysGate.memInfo.freeram   = SysGate->memInfo.freeram;
	RO(Shm)->SysGate.memInfo.bufferram = SysGate->memInfo.bufferram;
	RO(Shm)->SysGate.memInfo.totalhigh = SysGate->memInfo.totalhigh;
	RO(Shm)->SysGate.memInfo.freehigh  = SysGate->memInfo.freehigh;
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
	RO(Shm)->Registration.HSMP= RO(Proc)->Features.HSMP_Enable;
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

	Mitigation_1st_Stage(RO(Shm), RO(Proc), RW(Proc));
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
		(BOOST(SIZE)) * sizeof(unsigned int) );

	RO(Shm)->Cpu[cpu].Query.Microcode = RO(Core, AT(cpu))->Query.Microcode;

	Topology(RO(Shm), RO(Proc), RO(Core), cpu);

	CStates(RO(Shm), RO(Core), cpu);

	PowerThermal(RO(Shm), RO(Proc), RO(Core), cpu);

	SystemRegisters(RO(Shm), RO(Core), cpu);

	CPUID_Dump(RO(Shm), RO(Core), cpu);
}

#define SysOnce(drv)	ioctl(drv, COREFREQ_IOCTL_SYSONCE)

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

static void Pkg_ComputeVoltage_Intel_SAV(struct PKG_FLIP_FLOP *PFlip)
{
	COMPUTE_VOLTAGE(INTEL_SNB,
			PFlip->Voltage.CPU,
			PFlip->Voltage.VID.CPU);

	COMPUTE_VOLTAGE(INTEL_SAV,
			PFlip->Voltage.SOC,
			PFlip->Voltage.VID.SOC);
}

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
	RO(Shm)->App.Svr = tid;

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

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.voltageFormula)) {
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
	case VOLTAGE_KIND_INTEL_SAV:
		Pkg_ComputeVoltageFormula = Pkg_ComputeVoltage_Intel_SAV;
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

	switch (KIND_OF_FORMULA(RO(Shm)->Proc.powerFormula)) {
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
		  ABS_FREQ_MHz(double , RO(Shm)->Cpu[cpu].Boost[BOOST(MAX)],
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
	if (!RO(Shm)->Proc.Features.Power.EAX.PTM) {
		if (CFlop->Thermal.Temp > PFlip->Thermal.Temp)
			PFlip->Thermal.Temp = CFlop->Thermal.Temp;
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
      if (RO(Shm)->Proc.Features.Power.EAX.PTM)
      {
	PFlip->Thermal.Sensor = RO(Proc)->PowerThermal.Sensor;
	Pkg_ComputeThermalFormula(PFlip, SProc);
      }
	/*	Package Voltage formulas				*/
	PFlip->Voltage.VID.CPU = RO(Proc)->PowerThermal.VID.CPU;
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
		printf( "CoreFreq Daemon %s"		\
				"  Copyright (C) 2015-2022 CYRIL INGENIERIE\n",
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
