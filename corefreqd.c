/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
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

#define PAGE_SIZE (sysconf(_SC_PAGESIZE))

/* §8.10.6.7 Place Locks and Semaphores in Aligned, 128-Byte Blocks of Memory */
static Bit256 roomSeed	__attribute__ ((aligned (16))) = {0x0, 0x0, 0x0, 0x0};
static Bit256 roomCore	__attribute__ ((aligned (16))) = {0x0, 0x0, 0x0, 0x0};
static Bit256 roomSched __attribute__ ((aligned (16))) = {0x0, 0x0, 0x0, 0x0};
static Bit256 roomClear __attribute__ ((aligned (16))) = {0x0, 0x0, 0x0, 0x0};
static Bit64 Shutdown	__attribute__ ((aligned (8))) = 0x0;
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
	PROC			*Proc;
	CORE			**Core;
	SYSGATE			*SysGate;
} REF;

void Core_ComputeThermalLimits(CPU_STRUCT *Cpu, unsigned int Temp)
{	/* Per Core, computes the Min and Max temperatures.		*/
    if (((Cpu->PowerThermal.Limit[SENSOR_LOWEST] == 0) && (Temp != 0))
    || ((Temp != 0) && (Temp < Cpu->PowerThermal.Limit[SENSOR_LOWEST])))
    {
	Cpu->PowerThermal.Limit[SENSOR_LOWEST] = Temp;
    }
    if (Temp > Cpu->PowerThermal.Limit[SENSOR_HIGHEST])
    {
	Cpu->PowerThermal.Limit[SENSOR_HIGHEST] = Temp;
    }
}

static inline void ComputeThermal_None( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
}

#define ComputeThermal_None_PerSMT	ComputeThermal_None
#define ComputeThermal_None_PerCore	ComputeThermal_None
#define ComputeThermal_None_PerPkg	ComputeThermal_None

static void (*ComputeThermal_None_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_None_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_None_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_None_PerPkg
};

static inline void ComputeThermal_Intel(struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu)
{
	COMPUTE_THERMAL(INTEL,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip->Thermal.Temp);
}

#define ComputeThermal_Intel_PerSMT	ComputeThermal_Intel

static inline void ComputeThermal_Intel_PerCore(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_Intel(CFlip, Shm, cpu);
	}
}

static inline void ComputeThermal_Intel_PerPkg( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_Intel(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_Intel_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_Intel_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_Intel_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_Intel_PerPkg
};

static inline void ComputeThermal_AMD(	struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_THERMAL(AMD,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip->Thermal.Temp);
}

#define ComputeThermal_AMD_PerSMT	ComputeThermal_AMD

static inline void ComputeThermal_AMD_PerCore(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD(CFlip, Shm, cpu);
	}
}

static inline void ComputeThermal_AMD_PerPkg(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_AMD(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_AMD_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_PerPkg
};

static inline void ComputeThermal_AMD_0Fh(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_THERMAL(AMD_0Fh,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip->Thermal.Temp);
}

#define ComputeThermal_AMD_0Fh_PerSMT	ComputeThermal_AMD_0Fh

static inline void ComputeThermal_AMD_0Fh_PerCore(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static inline void ComputeThermal_AMD_0Fh_PerPkg(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeThermal_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static void (*ComputeThermal_AMD_0Fh_Matrix[4])(struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int) = \
{
	[FORMULA_SCOPE_NONE] = ComputeThermal_None,
	[FORMULA_SCOPE_SMT ] = ComputeThermal_AMD_0Fh_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeThermal_AMD_0Fh_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeThermal_AMD_0Fh_PerPkg
};

static inline void ComputeThermal_AMD_15h(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
    if (Shm->Cpu[cpu].Topology.CoreID == 0)
    {
	COMPUTE_THERMAL(AMD_15h,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip->Thermal.Temp);
    }
}

#define ComputeThermal_AMD_15h_PerSMT	ComputeThermal_AMD_15h

static inline void ComputeThermal_AMD_15h_PerCore(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (Shm->Cpu[cpu].Topology.CoreID == 0) /* Opteron test case	*/
	{
		ComputeThermal_AMD_15h(CFlip, Shm, cpu);
	}
}

static inline void ComputeThermal_AMD_15h_PerPkg(struct FLIP_FLOP *CFlip,
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

static inline void ComputeThermal_AMD_17h(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_THERMAL(AMD_17h,
			CFlip->Thermal.Temp,
			CFlip->Thermal.Param,
			CFlip->Thermal.Sensor);

	Core_ComputeThermalLimits(&Shm->Cpu[cpu], CFlip->Thermal.Temp);
}

#define ComputeThermal_AMD_17h_PerSMT	ComputeThermal_AMD_17h

static inline void ComputeThermal_AMD_17h_PerCore(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeThermal_AMD_17h(CFlip, Shm, cpu);
	}
}

static inline void ComputeThermal_AMD_17h_PerPkg(struct FLIP_FLOP *CFlip,
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

void Core_ComputeVoltageLimits(CPU_STRUCT *Cpu, double Vcore)
{	/* Per Core, computes the Min and Max CPU voltage.		*/
    if (((Cpu->Sensors.Voltage.Limit[SENSOR_LOWEST] == 0) && (Vcore != 0))
    || ((Vcore != 0) && (Vcore < Cpu->Sensors.Voltage.Limit[SENSOR_LOWEST])))
    {
	Cpu->Sensors.Voltage.Limit[SENSOR_LOWEST] = Vcore;
    }
    if (Vcore > Cpu->Sensors.Voltage.Limit[SENSOR_HIGHEST])
    {
	Cpu->Sensors.Voltage.Limit[SENSOR_HIGHEST] = Vcore;
    }
}

static inline void ComputeVoltage_None( struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
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

static inline void ComputeVoltage_Intel_Core2( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* Intel Core 2 Extreme Datasheet §3.3-Table 2			*/
	COMPUTE_VOLTAGE(INTEL_CORE2,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_Intel_Core2_PerSMT	ComputeVoltage_Intel_Core2

static inline void ComputeVoltage_Intel_Core2_PerCore( struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_Core2(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_Intel_Core2_PerPkg(	struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_Core2(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_Core2_Matrix[4])(	struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_Core2_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_Core2_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_Core2_PerPkg
};

static inline void ComputeVoltage_Intel_SNB(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(INTEL_SNB,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_Intel_SNB_PerSMT 	ComputeVoltage_Intel_SNB

static inline void ComputeVoltage_Intel_SNB_PerCore(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SNB(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_Intel_SNB_PerPkg(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SNB(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_SNB_Matrix[4])(	struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SNB_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SNB_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SNB_PerPkg
};

static inline void ComputeVoltage_Intel_SKL_X( struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(INTEL_SKL_X,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_Intel_SKL_X_PerSMT	ComputeVoltage_Intel_SKL_X

static inline void ComputeVoltage_Intel_SKL_X_PerCore(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Intel_SKL_X(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_Intel_SKL_X_PerPkg(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Intel_SKL_X(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Intel_SKL_X_Matrix[4])(	struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Intel_SKL_X_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Intel_SKL_X_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Intel_SKL_X_PerPkg
};

static inline void ComputeVoltage_AMD(	struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_AMD_PerSMT	ComputeVoltage_AMD

static inline void ComputeVoltage_AMD_PerCore(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_AMD_PerPkg(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_AMD(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_AMD_Matrix[4])(	struct FLIP_FLOP*,
						SHM_STRUCT*,
						unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_AMD_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_AMD_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_AMD_PerPkg
};

static inline void ComputeVoltage_AMD_0Fh(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* AMD BKDG Family 0Fh §10.6 Table 70				*/
	COMPUTE_VOLTAGE(AMD_0Fh,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_AMD_0Fh_PerSMT	ComputeVoltage_AMD_0Fh

static inline void ComputeVoltage_AMD_0Fh_PerCore(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_0Fh(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_AMD_0Fh_PerPkg(struct FLIP_FLOP *CFlip,
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

static inline void ComputeVoltage_AMD_15h(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD_15h,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_AMD_15h_PerSMT	ComputeVoltage_AMD_15h

static inline void ComputeVoltage_AMD_15h_PerCore(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_15h(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_AMD_15h_PerPkg(struct FLIP_FLOP *CFlip,
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

static inline void ComputeVoltage_AMD_17h(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{
	COMPUTE_VOLTAGE(AMD_17h,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_AMD_17h_PerSMT	ComputeVoltage_AMD_17h

static inline void ComputeVoltage_AMD_17h_PerCore(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_AMD_17h(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_AMD_17h_PerPkg(struct FLIP_FLOP *CFlip,
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

static inline void ComputeVoltage_Winbond_IO(	struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu )
{	/* Winbond W83627EHF/EF, W83627EHG,EG				*/
	COMPUTE_VOLTAGE(WINBOND_IO,
			CFlip->Voltage.Vcore,
			CFlip->Voltage.VID);

	Core_ComputeVoltageLimits(&Shm->Cpu[cpu], CFlip->Voltage.Vcore);
}

#define ComputeVoltage_Winbond_IO_PerSMT	ComputeVoltage_Winbond_IO

static inline void ComputeVoltage_Winbond_IO_PerCore(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputeVoltage_Winbond_IO(CFlip, Shm, cpu);
	}
}

static inline void ComputeVoltage_Winbond_IO_PerPkg(struct FLIP_FLOP *CFlip,
							SHM_STRUCT *Shm,
							unsigned int cpu )
{
	if (cpu == Shm->Proc.Service.Core)
	{
		ComputeVoltage_Winbond_IO(CFlip, Shm, cpu);
	}
}

static void (*ComputeVoltage_Winbond_IO_Matrix[4])(	struct FLIP_FLOP*,
							SHM_STRUCT*,
							unsigned int ) = \
{
	[FORMULA_SCOPE_NONE] = ComputeVoltage_None,
	[FORMULA_SCOPE_SMT ] = ComputeVoltage_Winbond_IO_PerSMT,
	[FORMULA_SCOPE_CORE] = ComputeVoltage_Winbond_IO_PerCore,
	[FORMULA_SCOPE_PKG ] = ComputeVoltage_Winbond_IO_PerPkg
};

void Core_ComputePowerLimits(CPU_STRUCT *Cpu, double Energy, double Power)
{	/* Per Core, computes the Min and Max CPU energy & power consumed. */
	if (Energy && Energy < Cpu->Sensors.Energy.Limit[SENSOR_LOWEST]) {
		Cpu->Sensors.Energy.Limit[SENSOR_LOWEST] = Energy;
	}
	if (Energy > Cpu->Sensors.Energy.Limit[SENSOR_HIGHEST]) {
		Cpu->Sensors.Energy.Limit[SENSOR_HIGHEST] = Energy;
	}
	if (Power && Power < Cpu->Sensors.Power.Limit[SENSOR_LOWEST]) {
		Cpu->Sensors.Power.Limit[SENSOR_LOWEST] = Power;
	}
	if (Power > Cpu->Sensors.Power.Limit[SENSOR_HIGHEST]) {
		Cpu->Sensors.Power.Limit[SENSOR_HIGHEST] = Power;
	}
}

static inline void ComputePower_None(	struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu )
{
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

static inline void ComputePower_AMD_17h(struct FLIP_FLOP *CFlip,
					SHM_STRUCT *Shm,
					unsigned int cpu)
{
	CFlip->State.Energy	= (double) CFlip->Delta.Power.ACCU
				* Shm->Proc.Power.Unit.Joules;

	CFlip->State.Power	= (1000.0 * CFlip->State.Energy)
				/ (double) Shm->Sleep.Interval;

	Core_ComputePowerLimits(&Shm->Cpu[cpu],
				CFlip->State.Energy,
				CFlip->State.Power);
}

#define ComputePower_AMD_17h_PerSMT	ComputePower_AMD_17h

static inline void ComputePower_AMD_17h_PerCore(struct FLIP_FLOP *CFlip,
						SHM_STRUCT *Shm,
						unsigned int cpu)
{
	if ((Shm->Cpu[cpu].Topology.ThreadID == 0)
	 || (Shm->Cpu[cpu].Topology.ThreadID == -1))
	{
		ComputePower_AMD_17h(CFlip, Shm, cpu);
	}
}

static inline void ComputePower_AMD_17h_PerPkg( struct FLIP_FLOP *CFlip,
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
	ARG		*Arg = (ARG *) arg;
	unsigned int	cpu  = Arg->Bind;
	SHM_STRUCT	*Shm = Arg->Ref->Shm;
	CORE		*Core= Arg->Ref->Core[cpu];
	CPU_STRUCT	*Cpu = &Shm->Cpu[cpu];

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
	double dTSC, dUCC, dURC, dINST, dC3, dC6, dC7, dC1;

    while (!BITCLR(LOCKLESS, Core->Sync.V, NTFY)
	&& !BITVAL(Shutdown, SYNC)
	&& !BITVAL(Core->OffLine, OS)) {
		nanosleep(&Shm->Sleep.pollingWait, NULL);
    }

    if (!BITVAL(Shutdown, SYNC) && !BITVAL(Core->OffLine, OS))
    {
	if (BITCLR_CC(BUS_LOCK, roomCore, cpu)) {
		Cpu->Toggle = !Cpu->Toggle;
	}
	struct FLIP_FLOP *CFlip = &Cpu->FlipFlop[Cpu->Toggle];

	/* Refresh this Base Clock.					*/
	CFlip->Clock.Q  = Core->Clock.Q;
	CFlip->Clock.R  = Core->Clock.R;
	CFlip->Clock.Hz = Core->Clock.Hz;

	/* Copy the Performance & C-States Counters.			*/
	CFlip->Delta.INST	= Core->Delta.INST;
	CFlip->Delta.C0.UCC	= Core->Delta.C0.UCC;
	CFlip->Delta.C0.URC	= Core->Delta.C0.URC;
	CFlip->Delta.C3 	= Core->Delta.C3;
	CFlip->Delta.C6 	= Core->Delta.C6;
	CFlip->Delta.C7 	= Core->Delta.C7;
	CFlip->Delta.TSC	= Core->Delta.TSC;
	CFlip->Delta.C1 	= Core->Delta.C1;

	dTSC	= (double) CFlip->Delta.TSC;
	dUCC	= (double) CFlip->Delta.C0.UCC;
	dURC	= (double) CFlip->Delta.C0.URC;
	dINST	= (double) CFlip->Delta.INST;
	dC3	= (double) CFlip->Delta.C3;
	dC6	= (double) CFlip->Delta.C6;
	dC7	= (double) CFlip->Delta.C7;
	dC1	= (double) CFlip->Delta.C1;

	/* Compute IPS=Instructions per TSC				*/
	CFlip->State.IPS = dINST / dTSC;

	/* Compute IPC=Instructions per non-halted reference cycle.
	   ( Protect against a division by zero )			*/
	CFlip->State.IPC = (CFlip->Delta.C0.URC != 0) ? dINST / dURC : 0.0f;

	/* Compute CPI=Non-halted reference cycles per instruction.
	   ( Protect against a division by zero )			*/
	CFlip->State.CPI = (CFlip->Delta.INST != 0) ? dURC / dINST : 0.0f;

	/* Compute the Turbo State.					*/
	CFlip->State.Turbo = dUCC / dTSC;

	/* Compute the C-States.					*/
	CFlip->State.C0 = dURC / dTSC;
	CFlip->State.C3 = dC3  / dTSC;
	CFlip->State.C6 = dC6  / dTSC;
	CFlip->State.C7 = dC7  / dTSC;
	CFlip->State.C1 = dC1  / dTSC;

	/* Apply the relative Ratio formula.				*/
	CFlip->Relative.Ratio = (dUCC * Shm->Proc.Boost[BOOST(MAX)]) / dTSC;

	if ((Shm->Proc.PM_version >= 2) && !Shm->Proc.Features.Std.ECX.Hyperv)
	{
	/* Case: Relative Frequency = UCC per second.			*/
		CFlip->Relative.Freq = dUCC / (Shm->Sleep.Interval * 1000);
	} else {
	/* Case: Relative Frequency = Relative Ratio x Bus Clock Frequency */
	  CFlip->Relative.Freq=(double)REL_FREQ(Shm->Proc.Boost[BOOST(MAX)], \
						CFlip->Relative.Ratio,	\
						Core->Clock,		\
						Shm->Sleep.Interval)
				/ (Shm->Sleep.Interval * 1000);
	}

	/* Per Core, evaluate thermal properties.			*/
	CFlip->Thermal.Sensor	= Core->PowerThermal.Sensor;
	CFlip->Thermal.Events	= Core->PowerThermal.Events;
	CFlip->Thermal.Param	= Core->PowerThermal.Param;

	ComputeThermalFormula[SCOPE_OF_FORMULA(Shm->Proc.thermalFormula)](
		CFlip, Shm, cpu
	);

	/* Per Core, evaluate the voltage properties.			*/
	CFlip->Voltage.VID = Core->PowerThermal.VID;

	ComputeVoltageFormula[SCOPE_OF_FORMULA(Shm->Proc.voltageFormula)](
		CFlip, Shm, cpu
	);

	/* Per Core, evaluate the Power properties.			*/
	CFlip->Delta.Power.ACCU = Core->Delta.Power.ACCU;

	ComputePowerFormula[SCOPE_OF_FORMULA(Shm->Proc.powerFormula)](
		CFlip, Shm, cpu
	);

	/* Copy the Interrupts counters.				*/
	CFlip->Counter.SMI = Core->Interrupt.SMI;

	/* If driver registered, copy any NMI counter.			*/
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_LOCAL) == 1) {
		CFlip->Counter.NMI.LOCAL   = Core->Interrupt.NMI.LOCAL;
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_UNKNOWN) == 1) {
		CFlip->Counter.NMI.UNKNOWN = Core->Interrupt.NMI.UNKNOWN;
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_SERR) == 1) {
		CFlip->Counter.NMI.PCISERR = Core->Interrupt.NMI.PCISERR;
	}
	if (BITVAL(Shm->Registration.NMI, BIT_NMI_IO_CHECK) == 1) {
		CFlip->Counter.NMI.IOCHECK = Core->Interrupt.NMI.IOCHECK;
	}

	CFlip->Ratio.Perf	= Core->Ratio.Perf;
	CFlip->Ratio.Target	= Core->Ratio.Target;

	CFlip->Frequency.Perf	= ABS_FREQ_MHz(
					__typeof__(CFlip->Frequency.Perf),
					Core->Ratio.Perf, CFlip->Clock
				);
	CFlip->Frequency.Target = ABS_FREQ_MHz(
					__typeof__(CFlip->Frequency.Target),
					Core->Ratio.Target, CFlip->Clock
				);
    }
  } while (!BITVAL(Shutdown, SYNC) && !BITVAL(Core->OffLine, OS)) ;

	BITCLR_CC(BUS_LOCK, roomCore, cpu);
	BITCLR_CC(BUS_LOCK, roomSeed, cpu);
EXIT:
	if (Quiet & 0x100) {
		printf("    Thread [%lx] %s CYCLE %03u\n", tid,
			BITVAL(Core->OffLine, OS) ? "Offline" : "Shutdown",cpu);
		fflush(stdout);
	}
	return (NULL);
}

void SliceScheduling(SHM_STRUCT *Shm, unsigned int cpu, enum PATTERN pattern)
{
	unsigned int seek;
	switch (pattern) {
	case RESET_CSP:
		for (seek = 0; seek < Shm->Proc.CPU.Count; seek++) {
			if (seek == Shm->Proc.Service.Core) {
				BITSET_CC(LOCKLESS, roomSched, seek);
			} else {
				BITCLR_CC(LOCKLESS, roomSched, seek);
			}
		}
		break;
	case ALL_SMT:
		if (cpu == Shm->Proc.Service.Core) {
			BITSTOR_CC(LOCKLESS, roomSched, roomSeed);
		}
		break;
	case RAND_SMT:
		do {
			seek = (unsigned int) rand();
			seek = seek % Shm->Proc.CPU.Count;
		} while (BITVAL(Shm->Cpu[seek].OffLine, OS));
		BITCLR_CC(LOCKLESS, roomSched, cpu);
		BITSET_CC(LOCKLESS, roomSched, seek);
		break;
	case RR_SMT:
		seek = cpu;
		do {
			seek++;
			if (seek >= Shm->Proc.CPU.Count) {
				seek = 0;
			}
		} while (BITVAL(Shm->Cpu[seek].OffLine, OS));
		BITCLR_CC(LOCKLESS, roomSched, cpu);
		BITSET_CC(LOCKLESS, roomSched, seek);
		break;
	case USR_CPU:
		BITSET_CC(LOCKLESS, roomSched, cpu);
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
		    if (BITVAL_CC(roomSched, cpu)) {
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
	return (NULL);
}

void Architecture(SHM_STRUCT *Shm, PROC *Proc)
{
	Bit32	fTSC = Proc->Features.Std.EDX.TSC,
		aTSC = Proc->Features.AdvPower.EDX.Inv_TSC;

	/* Copy all initial CPUID features.				*/
	memcpy(&Shm->Proc.Features, &Proc->Features, sizeof(FEATURES));
	/* Copy the fomula identifiers					*/
	Shm->Proc.thermalFormula = Proc->thermalFormula;
	Shm->Proc.voltageFormula = Proc->voltageFormula;
	Shm->Proc.powerFormula   = Proc->powerFormula;
	/* Copy the numbers of total & online CPU.			*/
	Shm->Proc.CPU.Count	= Proc->CPU.Count;
	Shm->Proc.CPU.OnLine	= Proc->CPU.OnLine;
	Shm->Proc.Service.Proc	= Proc->Service.Proc;
	/* Hypervisor identifier.					*/
	Shm->Proc.HypervisorID = Proc->HypervisorID;
	/* Copy the Architecture name.					*/
	StrCopy(Shm->Proc.Architecture, Proc->Architecture, CODENAME_LEN);
	/* Copy the base clock ratios.					*/
	memcpy(Shm->Proc.Boost, Proc->Boost,(BOOST(SIZE))*sizeof(unsigned int));
	/* Copy the processor's brand string.				*/
	StrCopy(Shm->Proc.Brand, Proc->Features.Info.Brand, 48 + 4);
	/* Compute the TSC mode: None, Variant, Invariant		*/
	Shm->Proc.Features.InvariantTSC = fTSC << aTSC;
}

void PerformanceMonitoring(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.PM_version = Proc->Features.PerfMon.EAX.Version;
}

void HyperThreading(SHM_STRUCT *Shm, PROC *Proc)
{	/* Update the HyperThreading state				*/
	Shm->Proc.Features.HyperThreading = Proc->Features.HTT_Enable;
}

void PowerInterface(SHM_STRUCT *Shm, PROC *Proc)
{
	unsigned int PowerUnits;

    if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    { /* AMD PowerNow */
	if (Proc->Features.AdvPower.EDX.FID) {
		BITSET(LOCKLESS, Shm->Proc.PowerNow, 0);
	} else {
		BITCLR(LOCKLESS, Shm->Proc.PowerNow, 0);
	}
	if (Proc->Features.AdvPower.EDX.VID) {
		BITSET(LOCKLESS, Shm->Proc.PowerNow, 1);
	} else {
		BITCLR(LOCKLESS, Shm->Proc.PowerNow, 1);
	}
    } else {
	Shm->Proc.PowerNow = 0;
    }
    switch (KIND_OF_FORMULA(Proc->powerFormula)) {
      case POWER_KIND_INTEL:
      case POWER_KIND_AMD:
      case POWER_KIND_AMD_17h:
	Shm->Proc.Power.Unit.Watts = Proc->PowerThermal.Unit.PU > 0 ?
			1.0 / (double) (1 << Proc->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= Proc->PowerThermal.Unit.ESU > 0 ?
			1.0 / (double)(1 << Proc->PowerThermal.Unit.ESU) : 0;
	break;
      case POWER_KIND_INTEL_ATOM:
	Shm->Proc.Power.Unit.Watts = Proc->PowerThermal.Unit.PU > 0 ?
			0.001 / (double)(1 << Proc->PowerThermal.Unit.PU) : 0;
	Shm->Proc.Power.Unit.Joules= Proc->PowerThermal.Unit.ESU > 0 ?
			0.001 / (double)(1 << Proc->PowerThermal.Unit.ESU) : 0;
	break;
      case POWER_KIND_NONE:
	break;
    }
	Shm->Proc.Power.Unit.Times = Proc->PowerThermal.Unit.TU > 0 ?
			1.0 / (double) (1 << Proc->PowerThermal.Unit.TU) : 0;

	PowerUnits = 2 << (Proc->PowerThermal.Unit.PU - 1);
    if (PowerUnits != 0)
    {
	Shm->Proc.Power.TDP	= Proc->PowerThermal.PowerInfo.ThermalSpecPower
				/ PowerUnits;
	Shm->Proc.Power.Min	= Proc->PowerThermal.PowerInfo.MinimumPower
				/ PowerUnits;
	Shm->Proc.Power.Max	= Proc->PowerThermal.PowerInfo.MaximumPower
				/ PowerUnits;
    }
}

void Technology_Update(SHM_STRUCT *Shm, PROC *Proc)
{	/* Technologies aggregation.					*/
	Shm->Proc.Technology.PowerNow = (Shm->Proc.PowerNow == 0b11);

	Shm->Proc.Technology.ODCM = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->ODCM,
						Proc->ODCM_Mask );

	Shm->Proc.Technology.PowerMgmt=BITCMP_CC(Shm->Proc.CPU.Count,LOCKLESS,
						Proc->PowerMgmt,
						Proc->PowerMgmt_Mask);

	Shm->Proc.Technology.EIST = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->SpeedStep,
						Proc->SpeedStep_Mask );

	Shm->Proc.Technology.Turbo = BITWISEAND_CC(LOCKLESS,
						Proc->TurboBoost,
						Proc->TurboBoost_Mask) != 0;

	Shm->Proc.Technology.C1E = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->C1E,
						Proc->C1E_Mask );

	Shm->Proc.Technology.C3A = BITCMP_CC( Shm->Proc.CPU.Count, LOCKLESS,
						Proc->C3A,
						Proc->C3A_Mask );

	Shm->Proc.Technology.C1A = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->C1A,
						Proc->C1A_Mask );

	Shm->Proc.Technology.C3U = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->C3U,
						Proc->C3U_Mask );

	Shm->Proc.Technology.C1U = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->C1U,
						Proc->C1U_Mask );

	Shm->Proc.Technology.CC6 = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->CC6,
						Proc->CC6_Mask );

	Shm->Proc.Technology.PC6 = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->PC6,
						Proc->PC6_Mask );

	Shm->Proc.Technology.SMM = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->SMM,
						Proc->CR_Mask );

	Shm->Proc.Technology.VM = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->VM,
						Proc->CR_Mask );
}

void Mitigation_Mechanisms(SHM_STRUCT *Shm, PROC *Proc)
{
	unsigned short	IBRS = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->IBRS,
						Proc->SPEC_CTRL_Mask ),

			STIBP = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->STIBP,
						Proc->SPEC_CTRL_Mask ),

			SSBD = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->SSBD,
						Proc->SPEC_CTRL_Mask ),

			RDCL_NO = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->RDCL_NO,
						Proc->ARCH_CAP_Mask ),

			IBRS_ALL = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->RDCL_NO,
						Proc->ARCH_CAP_Mask ),

			RSBA = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->RSBA,
						Proc->ARCH_CAP_Mask ),

			L1DFL_NO = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->L1DFL_VMENTRY_NO,
						Proc->ARCH_CAP_Mask ),

			SSB_NO = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->SSB_NO,
						Proc->ARCH_CAP_Mask ),

			MDS_NO = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->MDS_NO,
						Proc->ARCH_CAP_Mask ),

			PSCHANGE_MC_NO=BITCMP_CC(Shm->Proc.CPU.Count, LOCKLESS,
						Proc->PSCHANGE_MC_NO,
						Proc->ARCH_CAP_Mask),

			TAA_NO = BITCMP_CC(	Shm->Proc.CPU.Count, LOCKLESS,
						Proc->TAA_NO,
						Proc->ARCH_CAP_Mask );

	Shm->Proc.Mechanisms.IBRS = (
		Shm->Proc.Features.ExtFeature.EDX.IBRS_IBPB_Cap+(2 * IBRS)
	);
	Shm->Proc.Mechanisms.STIBP = (
		Shm->Proc.Features.ExtFeature.EDX.STIBP_Cap + (2 * STIBP)
	);
	Shm->Proc.Mechanisms.SSBD = (
		Shm->Proc.Features.ExtFeature.EDX.SSBD_Cap + (2 * SSBD)
	);
	Shm->Proc.Mechanisms.RDCL_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * RDCL_NO)
	);
	Shm->Proc.Mechanisms.IBRS_ALL = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * IBRS_ALL)
	);
	Shm->Proc.Mechanisms.RSBA = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * RSBA)
	);
	Shm->Proc.Mechanisms.L1DFL_VMENTRY_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * L1DFL_NO)
	);
	Shm->Proc.Mechanisms.SSB_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * SSB_NO)
	);
	Shm->Proc.Mechanisms.MDS_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * MDS_NO)
	);
	Shm->Proc.Mechanisms.PSCHANGE_MC_NO = (
	    Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2*PSCHANGE_MC_NO)
	);
	Shm->Proc.Mechanisms.TAA_NO = (
		Shm->Proc.Features.ExtFeature.EDX.IA32_ARCH_CAP+(2 * TAA_NO)
	);
}

void Package_Update(SHM_STRUCT *Shm, PROC *Proc)
{	/* Copy the operational settings.				*/
	Shm->Registration.AutoClock = Proc->Registration.AutoClock;
	Shm->Registration.Experimental = Proc->Registration.Experimental;
	Shm->Registration.HotPlug = Proc->Registration.HotPlug;
	Shm->Registration.PCI = Proc->Registration.PCI;
	BITSTOR(LOCKLESS, Shm->Registration.NMI, Proc->Registration.NMI);
	Shm->Registration.Driver = Proc->Registration.Driver;
	/* Copy the timer interval delay.				*/
	Shm->Sleep.Interval = Proc->SleepInterval;
	/* Compute the polling wait time based on the timer interval.	*/
	Shm->Sleep.pollingWait = TIMESPEC((Shm->Sleep.Interval * 1000000L)
				/ WAKEUP_RATIO);
	/* Copy the SysGate tick steps.					*/
	Shm->SysGate.tickReset = Proc->tickReset;
	Shm->SysGate.tickStep  = Proc->tickStep;

	Architecture(Shm, Proc);

	PerformanceMonitoring(Shm, Proc);

	HyperThreading(Shm, Proc);

	PowerInterface(Shm, Proc);

	Mitigation_Mechanisms(Shm, Proc);
}

typedef struct {
	unsigned int	Q,
			R;
} RAM_Ratio;

void P945_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

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
		unsigned long long DIMM_Size;
		unsigned short rank, rankCount = (cha == 0) ? 4 : 2;

	    for (rank = 0; rank < rankCount; rank++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks +=
		    Proc->Uncore.MC[mc].Channel[cha].P945.DRB[rank].Boundary;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].P945.BANK.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].P945.WIDTH.Rank0)
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

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P955_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

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
		unsigned long long DIMM_Size;
		unsigned short rank;

	    for (rank = 0; rank < 4; rank++) {
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks +=
		    Proc->Uncore.MC[mc].Channel[cha].P955.DRB[rank].Boundary;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].P955.BANK.Rank0)
	    {
	    case 0b00:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4;
		break;
	    case 0b01:
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 8;
		break;
	    }
	    switch (Proc->Uncore.MC[mc].Channel[cha].P955.WIDTH.Rank0)
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

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P945_CLK(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (Proc->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b000:
		Shm->Uncore.Bus.Rate = 400;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			/* Fallthrough */
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
			/* Fallthrough */
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
		/* Fallthrough */
	case 0b011:
		Shm->Uncore.Bus.Rate = 667;

		switch (Proc->Uncore.Bus.ClkCfg.RAM_Select) {
		default:
			/* Fallthrough */
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

	Shm->Uncore.CtrlSpeed = (Core->Clock.Hz * Ratio.Q * 2)	/* DDR2 */
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void P965_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
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
/* TODO(Timings)
	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT_.tFAW;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].P965.DRT_.tRTPr;
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL = ?
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
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P965_CLK(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
{
	RAM_Ratio Ratio = {.Q = 1, .R = 1};

	switch (Proc->Uncore.Bus.ClkCfg.FSB_Select) {
	case 0b111:	/* Unknown */
		/* Fallthrough */
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
		/* Fallthrough */
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

	Shm->Uncore.CtrlSpeed = (Core->Clock.Hz * Ratio.Q * 2)	/* DDR2 */
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b00;
	Shm->Uncore.Unit.BusSpeed = 0b00;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void G965_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
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
	for (slot = 0; slot < Shm->Uncore.MC[mc].SlotCount; slot++)
	{
		unsigned long long DIMM_Size;

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

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;
		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void G965_CLK(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
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
		/* Fallthrough */
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

	Shm->Uncore.CtrlSpeed = (Core->Clock.Hz * Ratio.Q * 2)	/* DDR2 */
				/ (Ratio.R * 1000000L);

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
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
/* TODO(Timings)
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

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	P3S_MCH(Shm, Proc, mc, cha);

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

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void P35_CLK(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
{
	P965_CLK(Shm, Proc, Core);
}

void P4S_MCH(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	P3S_MCH(Shm, Proc, mc, cha);

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

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = 0;
      }
    }
}

void NHM_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

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

	switch (Proc->Uncore.MC[mc].Channel[cha].NHM.Params.ENABLE_2N_3N)
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
	    if (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.DIMMPRESENT)
	    {
		unsigned long long DIMM_Size;

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

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	    }
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC =
				Proc->Uncore.MC[mc].NHM.STATUS.ECC_ENABLED
				& Proc->Uncore.MC[mc].NHM.CONTROL.ECC_ENABLED;
      }
    }
}

void QPI_CLK(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
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
		/* Fallthrough */
	default:
		Shm->Uncore.CtrlSpeed = 800;
		break;
	}

	Shm->Uncore.CtrlSpeed *= Core->Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = Proc->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 00 ?
		4800 : Proc->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 10 ?
			6400 : Proc->Uncore.Bus.QuickPath.X58.QPIFREQSEL == 01 ?
				5866 : 6400;

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

	Shm->Proc.Technology.IOMMU = !Proc->Uncore.Bus.QuickPath.X58.VT_d;
}

void DMI_CLK(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
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
		/* Fallthrough */
	default:
		Shm->Uncore.CtrlSpeed = 266;
		break;
	}

	Shm->Uncore.CtrlSpeed *= Core->Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate = 2500;	/* TODO: hardwired to Lynnfield */

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
}

void SNB_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
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

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
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

	switch (Proc->Uncore.MC[mc].Channel[cha].SNB.RAP.CMD_Stretch) {
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

		DIMM_Banks = 8 * dimmSize[cha][slot] * 1024 * 1024;

		DIMM_Banks = DIMM_Banks
			/ (Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = DIMM_Banks;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (cha == 0) ?
					Proc->Uncore.MC[mc].SNB.MAD0.ECC
				:	Proc->Uncore.MC[mc].SNB.MAD1.ECC;
      }
    }
}

void SNB_CAP(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
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
	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;

	Shm->Proc.Technology.IOMMU = !Proc->Uncore.Bus.SNB_Cap.VT_d;
}

void SNB_EP_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha, slot;

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
      Shm->Uncore.MC[mc].SlotCount = Proc->Uncore.MC[mc].SlotCount;
      Shm->Uncore.MC[mc].ChannelCount = Proc->Uncore.MC[mc].ChannelCount;

      for (cha = 0; cha < Shm->Uncore.MC[mc].ChannelCount; cha++)
      {
	Shm->Uncore.MC[mc].Channel[cha].Timing.tCL   =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRCD  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRCD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRP   =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRP;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCWL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tRAS;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRRD  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tRRD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRFC  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.EP.tRFC;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWR  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tRTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tWTPr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tFAW  =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tFAW;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTRd =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWRDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTRd =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWRDR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTRd =
		4 + Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.EP.tCWL
			+ Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.tWTPr;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTWr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTWr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWDR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTWr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRWSR;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tddRdTRd =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrRdTRd =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRDR;
/* TODO
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrRdTRd =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tRRSR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.tddWrTWr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWDD;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tdrWrTWr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWDR;
/* TODO
	Shm->Uncore.MC[mc].Channel[cha].Timing.tsrWrTWr =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tWWSR;
*/
	Shm->Uncore.MC[mc].Channel[cha].Timing.B2B   =
			Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.EP.tCCD;

	switch (Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.EP.CMD_Stretch) {
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
	    if (Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.DIMM_POP)
	    {
		unsigned long long DIMM_Size;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = 4 <<
		Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.DDR3_WIDTH;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks = 1 <<
		Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.RANK_CNT;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows = 1 << ( 13
		+ Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.RA_WIDTH );

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols = 1 << ( 10
		+ Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].MTR.CA_WIDTH );

		DIMM_Size = 8LLU
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks
			* Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks;

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Size=DIMM_Size >>20;
	    }
	}

	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC =			\
				Proc->Uncore.MC[mc].SNB_EP.TECH.ECC_EN
				& !Proc->Uncore.Bus.SNB_EP_Cap3.ECC_DIS;
      }
    }
}

void SNB_EP_CAP(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
{
	switch (Proc->Uncore.Bus.SNB_EP_Cap1.DMFC) {
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

	Shm->Uncore.CtrlSpeed *= Core->Clock.Hz;
	Shm->Uncore.CtrlSpeed /= Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Bus.Rate =						\
	  Proc->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 010 ? 5600
	: Proc->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 011 ? 6400
	: Proc->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 100 ? 7200
	: Proc->Uncore.Bus.QuickPath.IVB_EP.QPIFREQSEL == 101 ? 8000 : 5000;

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
				/ Shm->Proc.Features.Factory.Clock.Hz;

	Shm->Uncore.Unit.Bus_Rate = 0b01;
	Shm->Uncore.Unit.BusSpeed = 0b01;
	Shm->Uncore.Unit.DDR_Rate = 0b11;
	Shm->Uncore.Unit.DDRSpeed = 0b00;
/* TODO */
	Shm->Proc.Technology.IOMMU = 0;
}

void IVB_CAP(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
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
	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
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

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
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
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank.tCL;

	Shm->Uncore.MC[mc].Channel[cha].Timing.tCWL  =
			Proc->Uncore.MC[mc].Channel[cha].HSW.Rank.tCWL;
/*TODO(Not Found)
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

	switch (Proc->Uncore.MC[mc].Channel[cha].HSW.Timing.CMD_Stretch) {
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

		DIMM_Banks = 8 * dimmSize[cha][slot] * 1024 * 1024;

		DIMM_Banks = DIMM_Banks
			/ (Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Rows
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Cols
			*  Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Ranks);

		Shm->Uncore.MC[mc].Channel[cha].DIMM[slot].Banks = DIMM_Banks;
	}
	Shm->Uncore.MC[mc].Channel[cha].Timing.ECC = (cha == 0) ?
					Proc->Uncore.MC[mc].SNB.MAD0.ECC
				:	Proc->Uncore.MC[mc].SNB.MAD1.ECC;
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

void SKL_IMC(SHM_STRUCT *Shm, PROC *Proc)
{
    unsigned short mc, cha;

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

	switch (Proc->Uncore.MC[mc].Channel[cha].SKL.Sched.CMD_Stretch) {
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

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Banks =			\
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Banks =			\
	Proc->Uncore.MC[mc].Channel[cha].SKL.Sched.DRAM_Tech == 0b00 ? 16 : 8;

	Shm->Uncore.MC[mc].Channel[cha].DIMM[0].Cols =			\
		Proc->Uncore.MC[mc].Channel[cha].SKL.Sched.x8_device_Dimm0 ?
			1024 : 0;
	Shm->Uncore.MC[mc].Channel[cha].DIMM[1].Cols =			\
		Proc->Uncore.MC[mc].Channel[cha].SKL.Sched.x8_device_Dimm1 ?
			1024 : 0;
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
	].Rows = SKL_DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD0.DLW);

	Shm->Uncore.MC[mc].Channel[0].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC0.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD0.DSW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD1.DLW);

	Shm->Uncore.MC[mc].Channel[1].DIMM[
		!Proc->Uncore.MC[mc].SKL.MADC1.Dimm_L_Map
	].Rows = SKL_DimmWidthToRows(Proc->Uncore.MC[mc].SKL.MADD1.DSW);
    }
}

void SKL_CAP(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
{
	unsigned int DMFC;

	switch (Proc->ArchID) {
	case Skylake_UY:
	case Kabylake_UY:
		DMFC = Proc->Uncore.Bus.SKL_Cap_B.DMFC_DDR3;
		Shm->Uncore.Bus.Rate = 4000;	/* 4 GT/s QPI */
		break;
	default:
		DMFC = Proc->Uncore.Bus.SKL_Cap_C.DMFC_DDR4;
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

	Shm->Uncore.Bus.Speed = (Core->Clock.Hz * Shm->Uncore.Bus.Rate)
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

    for (mc = 0; mc < Shm->Uncore.CtrlCount; mc++)
    {
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
		if (Proc->Uncore.MC[mc].AMD0F.DCRL.BurstLength32 == 0b1) {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 2;
		} else {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 4;
		}
		break;
	case 0b1:
		if (Proc->Uncore.MC[mc].AMD0F.DCRL.BurstLength32 == 0b1) {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 3;
		} else {
			Shm->Uncore.MC[mc].Channel[cha].Timing.tRTPr = 5;
		}
		break;
	}

	if (Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRAS >= 0b0010) {
		Shm->Uncore.MC[mc].Channel[cha].Timing.tRAS =
			Proc->Uncore.MC[mc].Channel[cha].AMD0F.DTRL.tRAS + 3;
	}
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
	Shm->Uncore.CtrlSpeed = (Ratio.Q * 2) + Ratio.R;	/* DDR2 */

	if ((link = Proc->Uncore.Bus.UnitID.McUnit) < 0b11) {
		switch (Proc->Uncore.Bus.LDTi_Freq[link].LinkFreqMax)/*"MHz"*/
		{
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
}

void AMD_17h_IOMMU(SHM_STRUCT *Shm, PROC *Proc)
{
	Shm->Proc.Technology.IOMMU = BITVAL(Proc->Uncore.Bus.IOMMU_CR, 0);
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
	[IC_K8] 		= "K8/HyperTransport",
	[IC_ZEN]		= "Zen"
};

#define SET_CHIPSET(ic) (Shm->Uncore.Chipset.ArchID = ic)

void Uncore(SHM_STRUCT *Shm, PROC *Proc, CORE *Core)
{
	/* Copy the # of controllers and the chipset ID.		*/
	Shm->Uncore.CtrlCount = Proc->Uncore.CtrlCount;
	Shm->Uncore.ChipID = Proc->Uncore.ChipID;
	/* Decode the Memory Controller per architecture.		*/
	switch (Shm->Uncore.ChipID) {
	case PCI_DEVICE_ID_INTEL_82945P_HB:
		P945_CLK(Shm, Proc, Core);
		P945_MCH(Shm, Proc);
		SET_CHIPSET(IC_LAKEPORT);
		break;
	case PCI_DEVICE_ID_INTEL_82945GM_HB:
	case PCI_DEVICE_ID_INTEL_82945GME_HB:
		P945_CLK(Shm, Proc, Core);
		P945_MCH(Shm, Proc);
		SET_CHIPSET(IC_CALISTOGA);
		break;
	case PCI_DEVICE_ID_INTEL_82955_HB:
		P945_CLK(Shm, Proc, Core);
		P955_MCH(Shm, Proc);
		SET_CHIPSET(IC_LAKEPORT_X);
		break;
	case PCI_DEVICE_ID_INTEL_82946GZ_HB:
		P965_CLK(Shm, Proc, Core);
		P965_MCH(Shm, Proc);
		SET_CHIPSET(IC_LAKEPORT_P);
		break;
	case PCI_DEVICE_ID_INTEL_82965Q_HB:
	case PCI_DEVICE_ID_INTEL_82965G_HB:
		P965_CLK(Shm, Proc, Core);
		P965_MCH(Shm, Proc);
		SET_CHIPSET(IC_BROADWATER);
		break;
	case PCI_DEVICE_ID_INTEL_82965GM_HB:
	case PCI_DEVICE_ID_INTEL_82965GME_HB:
		G965_CLK(Shm, Proc, Core);
		G965_MCH(Shm, Proc);
		SET_CHIPSET(IC_CRESTLINE);
		break;
	case PCI_DEVICE_ID_INTEL_GM45_HB:
		G965_CLK(Shm, Proc, Core);
		G965_MCH(Shm, Proc);
		SET_CHIPSET(IC_CANTIGA);
		break;
	case PCI_DEVICE_ID_INTEL_Q35_HB:
		P35_CLK(Shm, Proc, Core);
		P35_MCH(Shm, Proc);
		SET_CHIPSET(IC_BEARLAKE_Q);
		break;
	case PCI_DEVICE_ID_INTEL_G33_HB:
		P35_CLK(Shm, Proc, Core);
		P35_MCH(Shm, Proc);
		SET_CHIPSET(IC_BEARLAKE_P);
		break;
	case PCI_DEVICE_ID_INTEL_Q33_HB:
		P35_CLK(Shm, Proc, Core);
		P35_MCH(Shm, Proc);
		SET_CHIPSET(IC_BEARLAKE_QF);
		break;
	case PCI_DEVICE_ID_INTEL_X38_HB:
		P35_CLK(Shm, Proc, Core);
		P35_MCH(Shm, Proc);
		SET_CHIPSET(IC_BEARLAKE_X);
		break;
	case PCI_DEVICE_ID_INTEL_3200_HB:
		P35_CLK(Shm, Proc, Core);
		P35_MCH(Shm, Proc);
		SET_CHIPSET(IC_INTEL_3200);
		break;
	case PCI_DEVICE_ID_INTEL_Q45_HB:
		P35_CLK(Shm, Proc, Core);
		P4S_MCH(Shm, Proc);
		SET_CHIPSET(IC_EAGLELAKE_Q);
		break;
	case PCI_DEVICE_ID_INTEL_G45_HB:
		P35_CLK(Shm, Proc, Core);
		P4S_MCH(Shm, Proc);
		SET_CHIPSET(IC_EAGLELAKE_P);
		break;
	case PCI_DEVICE_ID_INTEL_G41_HB:
		P35_CLK(Shm, Proc, Core);
		P4S_MCH(Shm, Proc);
		SET_CHIPSET(IC_EAGLELAKE_G);
		break;
	case PCI_DEVICE_ID_INTEL_I7_MCR:	/* Bloomfield		*/
	case PCI_DEVICE_ID_INTEL_NHM_EP_MCR:	/* Westmere EP		*/
		QPI_CLK(Shm, Proc, Core);
		NHM_IMC(Shm, Proc);
		SET_CHIPSET(IC_TYLERSBURG);
		break;
	case PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR:	/* Lynnfield		*/
		DMI_CLK(Shm, Proc, Core);
		NHM_IMC(Shm, Proc);
		SET_CHIPSET(IC_IBEXPEAK);
		break;
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0:   /* Sandy Bridge-E	*/
		SNB_EP_CAP(Shm, Proc, Core);
		SNB_EP_IMC(Shm, Proc);
		SET_CHIPSET(IC_PATSBURG);
		break;
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA:    /* SNB Desktop	*/
		SNB_CAP(Shm, Proc, Core);
		SNB_IMC(Shm, Proc);
		SET_CHIPSET(IC_COUGARPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104:
		SNB_CAP(Shm, Proc, Core);
		SNB_IMC(Shm, Proc);
		SET_CHIPSET(IC_IBEXPEAK_M);
		break;
	case PCI_DEVICE_ID_INTEL_IVB_EP_HOST_BRIDGE:  /* Xeon E5 & E7 v2 */
		SNB_EP_CAP(Shm, Proc, Core);
		SNB_EP_IMC(Shm, Proc);
		SET_CHIPSET(IC_CAVECREEK);
		break;
	case PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA:    /* IVB Desktop	*/
		IVB_CAP(Shm, Proc, Core);
		SNB_IMC(Shm, Proc);
		SET_CHIPSET(IC_PANTHERPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154:  /* IVB Mobile i5-3337U */
		IVB_CAP(Shm, Proc, Core);
		SNB_IMC(Shm, Proc);
		SET_CHIPSET(IC_PANTHERPOINT_M);
		break;
	case PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA:    /* HSW & BDW Desktop */
	case PCI_DEVICE_ID_INTEL_HASWELL_MH_IMC_HA0:/* HSW Mobile M/H	*/
	case PCI_DEVICE_ID_INTEL_HASWELL_UY_IMC_HA0:/* HSW Mobile U/Y	*/
		IVB_CAP(Shm, Proc, Core);
		HSW_IMC(Shm, Proc);
		SET_CHIPSET(IC_LYNXPOINT_M);
		break;
	case PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0:   /* Haswell		*/
		IVB_CAP(Shm, Proc, Core);
		HSW_IMC(Shm, Proc);
		SET_CHIPSET(IC_LYNXPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0: /* Broadwell/Y/U Core m */
		IVB_CAP(Shm, Proc, Core);
		HSW_IMC(Shm, Proc);
		SET_CHIPSET(IC_WILDCATPOINT_M);
		break;
	case PCI_DEVICE_ID_INTEL_BROADWELL_D_IMC_HA0:/* BDW/Desktop	*/
	case PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0:/* Broadwell/H	*/
		IVB_CAP(Shm, Proc, Core);
		HSW_IMC(Shm, Proc);
		SET_CHIPSET(IC_WELLSBURG);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA:  /* Skylake/U Processor */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA:  /* Skylake/Y Processor */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD: /* Skylake/S Dual Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ: /* Skylake/S Quad Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD: /* Skylake/H Dual Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ: /* Skylake/H Quad Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_SKYLAKE_DT_IMC_HA:	/* Skylake/DT Server */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_SUNRISEPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA:	/* BGA 1356	*/
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA:	/* BGA 1515	*/
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD: /* Kaby Lake/H Dual Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD: /* Kaby Lake/S Dual Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ: /* Kaby Lake/H Quad Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_DT_IMC_HA: /* Kaby Lake/DT Server */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ: /* U-Quad Core BGA 1356 */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ: /* Kaby Lake/S Quad Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_UNIONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ:/* Coffee Lake Quad Core*/
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAS:/* Coffee Lake Hexa Core*/
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAD:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAD:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAQ:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAQ:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAS:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAO:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAQ:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAS:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAO:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAQ:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAS:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAO:
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAD: /* WHL Dual Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAQ: /* WHL Quad Core */
		SKL_CAP(Shm, Proc, Core);
		SKL_IMC(Shm, Proc);
		SET_CHIPSET(IC_CANNONPOINT);
		break;
	case PCI_DEVICE_ID_AMD_K8_NB_MEMCTL:
		AMD_0F_HTT(Shm, Proc);
		AMD_0F_MCH(Shm, Proc);
		SET_CHIPSET(IC_K8);
		break;
	case PCI_DEVICE_ID_AMD_17H_IOMMU:
		AMD_17h_IOMMU(Shm, Proc);
		SET_CHIPSET(IC_ZEN);
		break;
	default:
		Chipset[IC_CHIPSET] = Proc->Features.Info.Vendor.ID;
		SET_CHIPSET(IC_CHIPSET);
		break;
	}
	/* Copy the chipset codename.					*/
	StrCopy(Shm->Uncore.Chipset.CodeName,
		Chipset[Shm->Uncore.Chipset.ArchID],
		CODENAME_LEN);
	/* Copy the Uncore clock ratios.				*/
	memcpy( Shm->Uncore.Boost,
		Proc->Uncore.Boost,
		(UNCORE_BOOST(SIZE)) * sizeof(unsigned int) );
}

void CPUID_Dump(SHM_STRUCT *Shm, CORE **Core, unsigned int cpu)
{	/* Copy the Vendor CPUID dump per Core.				*/
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

unsigned int AMD_L2_L3_Way_Associativity(unsigned int value)
{
	switch (value) {
	case 0x6:
		return (8);
	case 0x8:
		return (16);
	case 0xa:
		return (32);
	case 0xb:
		return (48);
	case 0xc:
		return (64);
	case 0xd:
		return (96);
	case 0xe:
		return (128);
	default:
		return (value);
	}
}

void Topology(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{	/* Copy each Core topology.					*/
	Shm->Cpu[cpu].Topology.MP.BSP    = (Core[cpu]->T.Base.BSP) ? 1 : 0;
	Shm->Cpu[cpu].Topology.ApicID    = Core[cpu]->T.ApicID;
	Shm->Cpu[cpu].Topology.CoreID    = Core[cpu]->T.CoreID;
	Shm->Cpu[cpu].Topology.ThreadID  = Core[cpu]->T.ThreadID;
	Shm->Cpu[cpu].Topology.PackageID = Core[cpu]->T.PackageID;
	Shm->Cpu[cpu].Topology.MP.x2APIC = ((Proc->Features.Std.ECX.x2APIC
					    & Core[cpu]->T.Base.EN)
					   << Core[cpu]->T.Base.EXTD);
	/* AMD Core Complex ID						*/
    if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
    || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
    {
	Shm->Cpu[cpu].Topology.MP.CCX = (Core[cpu]->T.ApicID & 0b1000) >> 3;
    }
	unsigned int loop;
    for (loop = 0; loop < CACHE_MAX_LEVEL; loop++)
    {
      if (Core[cpu]->T.Cache[loop].Type > 0)
      {
	unsigned int level = Core[cpu]->T.Cache[loop].Level;
	if (Core[cpu]->T.Cache[loop].Type == 2) {/* Instruction	*/
		level = 0;
	}
	if (Shm->Proc.Features.Info.Vendor.CRC == CRC_INTEL)
	{
		Shm->Cpu[cpu].Topology.Cache[level].Set =		\
					Core[cpu]->T.Cache[loop].Set + 1;

		Shm->Cpu[cpu].Topology.Cache[level].LineSz =		\
					Core[cpu]->T.Cache[loop].LineSz + 1;

		Shm->Cpu[cpu].Topology.Cache[level].Part =		\
					Core[cpu]->T.Cache[loop].Part + 1;

		Shm->Cpu[cpu].Topology.Cache[level].Way =		\
					Core[cpu]->T.Cache[loop].Way + 1;

		Shm->Cpu[cpu].Topology.Cache[level].Size =		\
				  Shm->Cpu[cpu].Topology.Cache[level].Set
				* Shm->Cpu[cpu].Topology.Cache[level].LineSz
				* Shm->Cpu[cpu].Topology.Cache[level].Part
				* Shm->Cpu[cpu].Topology.Cache[level].Way;
	} else {
	    if((Shm->Proc.Features.Info.Vendor.CRC == CRC_AMD)
	    || (Shm->Proc.Features.Info.Vendor.CRC == CRC_HYGON))
	    {
		Shm->Cpu[cpu].Topology.Cache[level].Way=(loop == 2)||(loop == 3)?
			AMD_L2_L3_Way_Associativity(Core[cpu]->T.Cache[loop].Way)
			: Core[cpu]->T.Cache[loop].Way;

		Shm->Cpu[cpu].Topology.Cache[level].Size =		\
						Core[cpu]->T.Cache[loop].Size;
	    }
	}
	Shm->Cpu[cpu].Topology.Cache[level].Feature.WriteBack = 	\
						Core[cpu]->T.Cache[loop].WrBack;

	Shm->Cpu[cpu].Topology.Cache[level].Feature.Inclusive = 	\
						Core[cpu]->T.Cache[loop].Inclus;
      }
    }
	/* Apply various architecture size unit.			*/
    switch (Proc->ArchID) {
    case AMD_Family_15h:
	/*TODO: do models 60h & 70h need a 512 KB size unit adjustment ? */
	if ((Shm->Proc.Features.Std.EAX.ExtModel == 0x6)
	 || (Shm->Proc.Features.Std.EAX.ExtModel == 0x7)) {
		break;
	}
	/* Fallthrough */
    case AMD_Family_17h:
    case AMD_Family_18h:
	/* CPUID_Fn80000006_EDX: Value in [3FFFh - 0001h] = (<Value> *0.5) MB */
	Shm->Cpu[cpu].Topology.Cache[3].Size *= (512 / 2);
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

	Shm->Cpu[cpu].PowerThermal.TM1 = Proc->Features.Std.EDX.TM1; /* 000v */

	Shm->Cpu[cpu].PowerThermal.TM1 |=
			(Core[cpu]->PowerThermal.TCC_Enable << 1);   /* 00v0 */

	Shm->Cpu[cpu].PowerThermal.TM2 = Proc->Features.Std.ECX.TM2; /* 000v */

	Shm->Cpu[cpu].PowerThermal.TM2 |=
			(Core[cpu]->PowerThermal.TM2_Enable << 1);   /* 00v0 */

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Highest =
			Core[cpu]->PowerThermal.HWP_Capabilities.Highest;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Guaranteed =
			Core[cpu]->PowerThermal.HWP_Capabilities.Guaranteed;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Most_Efficient =
			Core[cpu]->PowerThermal.HWP_Capabilities.Most_Efficient;

	Shm->Cpu[cpu].PowerThermal.HWP.Capabilities.Lowest =
			Core[cpu]->PowerThermal.HWP_Capabilities.Lowest;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Minimum_Perf =
			Core[cpu]->PowerThermal.HWP_Request.Minimum_Perf;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Maximum_Perf =
			Core[cpu]->PowerThermal.HWP_Request.Maximum_Perf;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Desired_Perf =
			Core[cpu]->PowerThermal.HWP_Request.Desired_Perf;

	Shm->Cpu[cpu].PowerThermal.HWP.Request.Energy_Pref =
			Core[cpu]->PowerThermal.HWP_Request.Energy_Pref;
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

void SysGate_OS_Driver(REF *Ref)
{
    SHM_STRUCT *Shm = Ref->Shm;
    SYSGATE *SysGate = Ref->SysGate;

    if (strlen(SysGate->OS.IdleDriver.Name) > 0) {
	int idx;

	StrCopy(Shm->SysGate.OS.IdleDriver.Name,
		SysGate->OS.IdleDriver.Name,
		CPUIDLE_NAME_LEN);

	Shm->SysGate.OS.IdleDriver.stateCount=SysGate->OS.IdleDriver.stateCount;
	Shm->SysGate.OS.IdleDriver.stateLimit=SysGate->OS.IdleDriver.stateLimit;

	for (idx = 0; idx < Shm->SysGate.OS.IdleDriver.stateCount; idx++)
	{
		StrCopy(Shm->SysGate.OS.IdleDriver.State[idx].Name,
			SysGate->OS.IdleDriver.State[idx].Name,
			CPUIDLE_NAME_LEN);

		StrCopy(Shm->SysGate.OS.IdleDriver.State[idx].Desc,
			SysGate->OS.IdleDriver.State[idx].Desc,
			CPUIDLE_NAME_LEN);

		Shm->SysGate.OS.IdleDriver.State[idx].exitLatency =
			SysGate->OS.IdleDriver.State[idx].exitLatency;

		Shm->SysGate.OS.IdleDriver.State[idx].powerUsage =
			SysGate->OS.IdleDriver.State[idx].powerUsage;

		Shm->SysGate.OS.IdleDriver.State[idx].targetResidency =
			SysGate->OS.IdleDriver.State[idx].targetResidency;
	}
    }
    if (strlen(SysGate->OS.FreqDriver.Name) > 0) {
	StrCopy(Shm->SysGate.OS.FreqDriver.Name,
		SysGate->OS.FreqDriver.Name,
		CPUFREQ_NAME_LEN);
    }
    if (strlen(SysGate->OS.FreqDriver.Governor) > 0) {
	StrCopy(Shm->SysGate.OS.FreqDriver.Governor,
		SysGate->OS.FreqDriver.Governor,
		CPUFREQ_NAME_LEN);
    }
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

static const int reverseSign[2] = {+1, -1};

static int SortByRuntime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->runtime < task2->runtime ? +1 : -1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return (sort);
}

static int SortByUsertime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->usertime < task2->usertime ? +1 : -1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return (sort);
}

static int SortBySystime(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->systime < task2->systime ? +1 : -1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return (sort);
}

static int SortByState(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->state < task2->state ? -1 : +1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return (sort);
}

static int SortByPID(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = task1->pid < task2->pid ? -1 : +1;
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return (sort);
}

static int SortByCommand(const void *p1, const void *p2, void *arg)
{
	TASK_MCB *task1 = (TASK_MCB*) p1, *task2 = (TASK_MCB*) p2;
	SHM_STRUCT *Shm = (SHM_STRUCT *) arg;
	int sort = strncmp(task1->comm, task2->comm, TASK_COMM_LEN);
	sort *= reverseSign[Shm->SysGate.reverseOrder];
	return (sort);
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
	return (sort);
}

void SysGate_Update(REF *Ref)
{
	SHM_STRUCT *Shm = Ref->Shm;
	SYSGATE *SysGate = Ref->SysGate;

	Shm->SysGate.taskCount = SysGate->taskCount;

	memcpy( Shm->SysGate.taskList, SysGate->taskList,
		Shm->SysGate.taskCount * sizeof(TASK_MCB));

	qsort_r(Shm->SysGate.taskList, Shm->SysGate.taskCount, sizeof(TASK_MCB),
		Shm->SysGate.trackTask ?
			  SortByTracker
			: SortByFunc[Shm->SysGate.sortByField], Shm);

	Shm->SysGate.memInfo.totalram  = SysGate->memInfo.totalram;
	Shm->SysGate.memInfo.sharedram = SysGate->memInfo.sharedram;
	Shm->SysGate.memInfo.freeram   = SysGate->memInfo.freeram;
	Shm->SysGate.memInfo.bufferram = SysGate->memInfo.bufferram;
	Shm->SysGate.memInfo.totalhigh = SysGate->memInfo.totalhigh;
	Shm->SysGate.memInfo.freehigh  = SysGate->memInfo.freehigh;

	Shm->SysGate.OS.IdleDriver.stateLimit=SysGate->OS.IdleDriver.stateLimit;
}

void PerCore_Update(SHM_STRUCT *Shm, PROC *Proc, CORE **Core, unsigned int cpu)
{
	if (BITVAL(Core[cpu]->OffLine, HW)) {
		BITSET(LOCKLESS, Shm->Cpu[cpu].OffLine, HW);
	} else {
		BITCLR(LOCKLESS, Shm->Cpu[cpu].OffLine, HW);
	}
	Shm->Cpu[cpu].Query.Microcode = Core[cpu]->Query.Microcode;

	Topology(Shm, Proc, Core, cpu);

	CStates(Shm, Core, cpu);

	PowerThermal(Shm, Proc, Core, cpu);

	SystemRegisters(Shm, Core, cpu);

	CPUID_Dump(Shm, Core, cpu);
}

int SysGate_OnDemand(REF *Ref, int operation)
{
	int rc = -1;
	const size_t allocPages = PAGE_SIZE << Ref->Proc->OS.ReqMem.Order;
	if (operation == 0) {
	    if (Ref->SysGate != NULL) {
		if ((rc = munmap(Ref->SysGate, allocPages)) == 0) {
			Ref->SysGate = NULL;
		}
	    } else {
		rc = -1;
	    }
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
	    } else {
		rc = 0;
	    }
	}
	return (rc);
}

void SysGate_Toggle(REF *Ref, unsigned int state)
{
    if (state == 0) {
	if (BITWISEAND(LOCKLESS, Ref->Shm->SysGate.Operation, 0x1)) {
		/* Stop SysGate 					*/
		BITCLR(LOCKLESS, Ref->Shm->SysGate.Operation, 0);
		/* Notify						*/
		BITWISESET(LOCKLESS, Ref->Shm->Proc.Sync, BIT_MASK_NTFY);
	}
    } else {
	if (!BITWISEAND(LOCKLESS, Ref->Shm->SysGate.Operation, 0x1)) {
	    if (SysGate_OnDemand(Ref, 1) == 0) {
		if (ioctl(Ref->fd->Drv, COREFREQ_IOCTL_SYSONCE) != -1) {
			/* Aggregate the OS idle driver data.		*/
			SysGate_OS_Driver(Ref);
			/* Copy system information.			*/
			SysGate_Kernel(Ref);
			/* Start SysGate				*/
			BITSET(LOCKLESS, Ref->Shm->SysGate.Operation, 0);
			/* Notify					*/
			BITWISESET(LOCKLESS,Ref->Shm->Proc.Sync,BIT_MASK_NTFY);
		}
	    }
	}
    }
}

void UpdateFeatures(REF *Ref)
{
	unsigned int cpu;

	Package_Update(Ref->Shm, Ref->Proc);
	for (cpu = 0; cpu < Ref->Shm->Proc.CPU.Count; cpu++) {
	    if (BITVAL(Ref->Core[cpu]->OffLine, OS) == 0)
	    {
		PerCore_Update(Ref->Shm, Ref->Proc, Ref->Core, cpu);
	    }
	}
	Uncore(Ref->Shm, Ref->Proc, Ref->Core[Ref->Proc->Service.Core]);
	Technology_Update(Ref->Shm, Ref->Proc);
}

void Master_Ring_Handler(REF *Ref, unsigned int rid)
{
    if (!RING_NULL(Ref->Shm->Ring[rid]))
    {
	RING_CTRL ctrl __attribute__ ((aligned(16)));
	RING_READ(Ref->Shm->Ring[rid], ctrl);
	int rc = ioctl(Ref->fd->Drv, ctrl.cmd, ctrl.arg);

	if (Quiet & 0x100) {
		printf("\tRING[%u](%x,%x)(%lx)>%d\n",
			rid, ctrl.cmd, ctrl.sub, ctrl.arg, rc);
	}
	switch (rc) {
	case -EPERM:
		break;
	case 1:
		SysGate_OS_Driver(Ref);
	/* Fallthrough */
	case 0: /* Update SHM and notify a platform changed.		*/
		UpdateFeatures(Ref);

		BITWISESET(LOCKLESS, Ref->Shm->Proc.Sync, BIT_MASK_NTFY);
		break;
	case 2: /* Update SHM and notify to re-compute.			*/
		UpdateFeatures(Ref);

		BITWISESET(LOCKLESS, Ref->Shm->Proc.Sync, BIT_MASK_COMP);
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

   switch (ctrl.cmd)
   {
   case COREFREQ_ORDER_MACHINE:
	switch (ctrl.arg) {
	case COREFREQ_TOGGLE_OFF:
	    if (BITVAL(Ref->Shm->Proc.Sync, BURN))
	    {
		BITCLR(BUS_LOCK, Ref->Shm->Proc.Sync, BURN);

		while (BITWISEAND_CC(BUS_LOCK, roomCore, roomSeed))
		{
			if (BITVAL(Shutdown, SYNC)) {	/* SpinLock */
				break;
			}
		}
		BITSTOR_CC(BUS_LOCK, roomSched, roomClear);

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
       if (!BITVAL(Ref->Shm->Proc.Sync, BURN))
       {
	while (BITWISEAND_CC(BUS_LOCK, roomCore, roomSeed))
	{
		if (BITVAL(Shutdown, SYNC)) {	/* SpinLock */
			break;
		}
	}
	SliceScheduling(Ref->Shm, ctrl.dl.lo, porder->pattern);

	Ref->Slice.Func = porder->func;
	Ref->Slice.arg  = porder->ctrl.dl.lo;
	Ref->Slice.pattern = porder->pattern;

	BITSET(BUS_LOCK, Ref->Shm->Proc.Sync, BURN);
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
			CPU_SET(pSlave->Thread, &cpuset);
		}
		return (pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset));
	}
	return (-1);
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
			/* Fallthrough */
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
	return (NULL);
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

static inline void Pkg_ComputeThermal_None(	struct PKG_FLIP_FLOP *PFlip,
						struct FLIP_FLOP *SProc )
{
}

static inline void Pkg_ComputeThermal_Intel(	struct PKG_FLIP_FLOP *PFlip,
						struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(INTEL,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static inline void Pkg_ComputeThermal_AMD(	struct PKG_FLIP_FLOP *PFlip,
						struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static inline void Pkg_ComputeThermal_AMD_0Fh(	struct PKG_FLIP_FLOP *PFlip,
						struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD_0Fh,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static inline void Pkg_ComputeThermal_AMD_15h(	struct PKG_FLIP_FLOP *PFlip,
						struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD_15h,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static inline void Pkg_ComputeThermal_AMD_17h(	struct PKG_FLIP_FLOP *PFlip,
						struct FLIP_FLOP *SProc )
{
	COMPUTE_THERMAL(AMD_17h,
		PFlip->Thermal.Temp,
		SProc->Thermal.Param,
		PFlip->Thermal.Sensor);
}

static inline void Pkg_ComputeVoltage_None(	CPU_STRUCT *Cpu,
						struct FLIP_FLOP *SProc )
{
}

#define Pkg_ComputeVoltage_Intel	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_Intel_Core2	Pkg_ComputeVoltage_None

static inline void Pkg_ComputeVoltage_Intel_SNB(CPU_STRUCT *Cpu,
						struct FLIP_FLOP *SProc)
{	/* Intel 2nd Generation Datasheet Vol-1 §7.4 Table 7-1		*/
	COMPUTE_VOLTAGE(INTEL_SNB,
			SProc->Voltage.Vcore,
			SProc->Voltage.VID);

	Core_ComputeVoltageLimits(Cpu, SProc->Voltage.Vcore);
}

#define Pkg_ComputeVoltage_Intel_SKL_X	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD		Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD_0Fh	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD_15h	Pkg_ComputeVoltage_None

#define Pkg_ComputeVoltage_AMD_17h	Pkg_ComputeVoltage_None

static inline void Pkg_ComputeVoltage_Winbond_IO(CPU_STRUCT *Cpu,
						struct FLIP_FLOP *SProc)
{	/* Winbond W83627EHF/EF, W83627EHG,EG				*/
	COMPUTE_VOLTAGE(WINBOND_IO,
			SProc->Voltage.Vcore,
			SProc->Voltage.VID);

	Core_ComputeVoltageLimits(Cpu, SProc->Voltage.Vcore);
}

#define Pkg_ComputePowerLimits(pw)					\
{ /* Package scope, computes Min and Max CPU energy & power consumed. */\
    if ( ( (Shm->Proc.State.Energy[pw].Limit[SENSOR_LOWEST] == 0)	\
	&& (Shm->Proc.State.Energy[pw].Current != 0) )			\
    || ( (Shm->Proc.State.Energy[pw].Current != 0)			\
	&& (Shm->Proc.State.Energy[pw].Current				\
		< Shm->Proc.State.Energy[pw].Limit[SENSOR_LOWEST]) ) )	\
    {									\
	Shm->Proc.State.Energy[pw].Limit[SENSOR_LOWEST] =		\
				Shm->Proc.State.Energy[pw].Current;	\
    }									\
    if (Shm->Proc.State.Energy[pw].Current				\
	> Shm->Proc.State.Energy[pw].Limit[SENSOR_HIGHEST])		\
    {									\
	Shm->Proc.State.Energy[pw].Limit[SENSOR_HIGHEST] =		\
				Shm->Proc.State.Energy[pw].Current;	\
    }									\
    if ( ( (Shm->Proc.State.Power[pw].Limit[SENSOR_LOWEST] == 0)	\
	&& (Shm->Proc.State.Power[pw].Current != 0) )			\
    || ( (Shm->Proc.State.Power[pw].Current != 0)			\
	&& (Shm->Proc.State.Power[pw].Current				\
		< Shm->Proc.State.Power[pw].Limit[SENSOR_LOWEST]) ) )	\
    {									\
	Shm->Proc.State.Power[pw].Limit[SENSOR_LOWEST] =		\
				Shm->Proc.State.Power[pw].Current;	\
    }									\
    if (Shm->Proc.State.Power[pw].Current				\
	> Shm->Proc.State.Power[pw].Limit[SENSOR_HIGHEST])		\
    {									\
	Shm->Proc.State.Power[pw].Limit[SENSOR_HIGHEST] =		\
				Shm->Proc.State.Power[pw].Current;	\
    }									\
}

static inline void Pkg_ComputePower_None(PROC *Proc, struct FLIP_FLOP *CFlop)
{
}

#define Pkg_ComputePower_Intel		Pkg_ComputePower_None

#define Pkg_ComputePower_Intel_Atom	Pkg_ComputePower_None

#define Pkg_ComputePower_AMD		Pkg_ComputePower_None

static inline void Pkg_ComputePower_AMD_17h(PROC *Proc, struct FLIP_FLOP *CFlop)
{
	Proc->Delta.Power.ACCU[PWR_DOMAIN(CORES)] += CFlop->Delta.Power.ACCU;
}

static inline void Pkg_ResetPower_None(PROC *Proc)
{
}

#define Pkg_ResetPower_Intel		Pkg_ResetPower_None

#define Pkg_ResetPower_Intel_Atom	Pkg_ResetPower_None

#define Pkg_ResetPower_AMD		Pkg_ResetPower_None

static inline void Pkg_ResetPower_AMD_17h(PROC *Proc)
{
	Proc->Delta.Power.ACCU[PWR_DOMAIN(CORES)] = 0;
}

REASON_CODE Core_Manager(REF *Ref)
{
	SHM_STRUCT		*Shm = Ref->Shm;
	PROC			*Proc = Ref->Proc;
	CORE			**Core = Ref->Core;
	struct PKG_FLIP_FLOP	*PFlip;
	struct FLIP_FLOP	*SProc;
	SERVICE_PROC		localService = {.Proc = -1};
	double			maxRelFreq;
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

	void (*Pkg_ComputeVoltageFormula)(	CPU_STRUCT*,
						struct FLIP_FLOP* );

	void (*Pkg_ComputePowerFormula)(PROC*, struct FLIP_FLOP*);

	void (*Pkg_ResetPowerFormula)(PROC*);

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

    #define CONDITION_RDTSCP()						\
	(  (Proc->Features.AdvPower.EDX.Inv_TSC == 1)			\
	|| (Proc->Features.ExtInfo.EDX.RDTSCP == 1) )

    #define CONDITION_RDPMC()						\
	(  (Proc->Features.Info.Vendor.CRC == CRC_INTEL)		\
	&& (Proc->Features.PerfMon.EAX.Version >= 1)			\
	&& (BITVAL(Core[Proc->Service.Core]->SystemRegister.CR4,	\
							CR4_PCE) == 1) )

	UBENCH_SETUP(CONDITION_RDTSCP(), CONDITION_RDPMC());
	Print_uBenchmark((Quiet & 0x100));

    while (!BITVAL(Shutdown, SYNC))
    {	/* Loop while all the cpu room bits are not cleared.		*/
	while ( !BITVAL(Shutdown, SYNC)
		&& !(Shm->Proc.Features.Std.ECX.CMPXCHG16 ?
		    BITCMP_CC(Shm->Proc.CPU.Count, BUS_LOCK, roomCore,roomClear)
		  : BITZERO(BUS_LOCK, roomCore[CORE_WORD_TOP(CORE_COUNT)])) )
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
	maxRelFreq	    = 0.0;

	Pkg_ResetPowerFormula(Proc);

	for (cpu=0; !BITVAL(Shutdown, SYNC)&&(cpu < Shm->Proc.CPU.Count);cpu++)
	{
	    if (BITVAL(Core[cpu]->OffLine, OS) == 1) {
		if (Arg[cpu].TID)
		{	/* Remove this cpu.				*/
			pthread_join(Arg[cpu].TID, NULL);
			Arg[cpu].TID = 0;

			PerCore_Update(Shm, Proc, Core, cpu);
			Technology_Update(Shm, Proc);

		    if (ServerFollowService(	&localService,
						&Shm->Proc.Service,
						tid ) == 0)
		    {
			SProc = &Shm->Cpu[Shm->Proc.Service.Core].FlipFlop[ \
				!Shm->Cpu[Shm->Proc.Service.Core].Toggle ];
		    }
			/* Raise these bits up to notify a platform change. */
			BITWISESET(LOCKLESS, Shm->Proc.Sync, BIT_MASK_NTFY);
		}
		BITSET(LOCKLESS, Shm->Cpu[cpu].OffLine, OS);
	    } else {
		struct FLIP_FLOP *CFlop = \
				&Shm->Cpu[cpu].FlipFlop[!Shm->Cpu[cpu].Toggle];

		if (!Arg[cpu].TID)
		{	/* Add this cpu.				*/
			Arg[cpu].Ref  = Ref;
			Arg[cpu].Bind = cpu;
			pthread_create( &Arg[cpu].TID,
					NULL,
					Core_Cycle,
					&Arg[cpu]);

			PerCore_Update(Shm, Proc, Core, cpu);
			Technology_Update(Shm, Proc);

		    if (ServerFollowService(&localService,
						&Shm->Proc.Service,
						tid) == 0)
		    {
			SProc = &Shm->Cpu[Shm->Proc.Service.Core].FlipFlop[ \
				!Shm->Cpu[Shm->Proc.Service.Core].Toggle ];
		    }
		    if (Quiet & 0x100) {
			printf( "    CPU #%03u @ %.2f MHz\n", cpu,
				ABS_FREQ_MHz(double,Shm->Proc.Boost[BOOST(MAX)],
				Core[cpu]->Clock) );
		    }
			/* Notify a CPU has been brought up		*/
			BITWISESET(LOCKLESS, Shm->Proc.Sync, BIT_MASK_NTFY);
		}
		BITCLR(LOCKLESS, Shm->Cpu[cpu].OffLine, OS);

		/* Index cpu with the highest frequency.		*/
		if (CFlop->Relative.Freq > maxRelFreq) {
			maxRelFreq = CFlop->Relative.Freq;
			Shm->Proc.Top = cpu;
		}

		/* Workaround to Package Thermal Management: the hottest Core */
		if (!Shm->Proc.Features.Power.EAX.PTM) {
			if (CFlop->Thermal.Temp > PFlip->Thermal.Temp)
				PFlip->Thermal.Temp = CFlop->Thermal.Temp;
		}

		/* Workaround to RAPL Package counter: sum of all Cores */
		Pkg_ComputePowerFormula(Proc, CFlop);

		/* Sum counters.					*/
		Shm->Proc.Avg.Turbo += CFlop->State.Turbo;
		Shm->Proc.Avg.C0    += CFlop->State.C0;
		Shm->Proc.Avg.C3    += CFlop->State.C3;
		Shm->Proc.Avg.C6    += CFlop->State.C6;
		Shm->Proc.Avg.C7    += CFlop->State.C7;
		Shm->Proc.Avg.C1    += CFlop->State.C1;
	    }
	}

	if (!BITVAL(Shutdown, SYNC))
	{
		double dPTSC;
		/* Compute the counters averages.			*/
		Shm->Proc.Avg.Turbo /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C0    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C3    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C6    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C7    /= Shm->Proc.CPU.OnLine;
		Shm->Proc.Avg.C1    /= Shm->Proc.CPU.OnLine;
		/* Package scope counters				*/
		PFlip->Delta.PTSC = Proc->Delta.PTSC;
		PFlip->Delta.PC02 = Proc->Delta.PC02;
		PFlip->Delta.PC03 = Proc->Delta.PC03;
		PFlip->Delta.PC06 = Proc->Delta.PC06;
		PFlip->Delta.PC07 = Proc->Delta.PC07;
		PFlip->Delta.PC08 = Proc->Delta.PC08;
		PFlip->Delta.PC09 = Proc->Delta.PC09;
		PFlip->Delta.PC10 = Proc->Delta.PC10;
		/* Package C-state Residency counters			*/
		dPTSC = (double) PFlip->Delta.PTSC;

		Shm->Proc.State.PC02	= (double) PFlip->Delta.PC02 / dPTSC;
		Shm->Proc.State.PC03	= (double) PFlip->Delta.PC03 / dPTSC;
		Shm->Proc.State.PC06	= (double) PFlip->Delta.PC06 / dPTSC;
		Shm->Proc.State.PC07	= (double) PFlip->Delta.PC07 / dPTSC;
		Shm->Proc.State.PC08	= (double) PFlip->Delta.PC08 / dPTSC;
		Shm->Proc.State.PC09	= (double) PFlip->Delta.PC09 / dPTSC;
		Shm->Proc.State.PC10	= (double) PFlip->Delta.PC10 / dPTSC;
		/* Uncore scope counters				*/
		PFlip->Uncore.FC0 = Proc->Delta.Uncore.FC0;
		/* Power & Energy counters				*/
		enum PWR_DOMAIN pw;
	    for (pw = PWR_DOMAIN(PKG); pw < PWR_DOMAIN(SIZE); pw++)
	    {
		PFlip->Delta.ACCU[pw] = Proc->Delta.Power.ACCU[pw];

		Shm->Proc.State.Energy[pw].Current = \
						(double) PFlip->Delta.ACCU[pw]
						* Shm->Proc.Power.Unit.Joules;

		Shm->Proc.State.Power[pw].Current = \
				(1000.0 * Shm->Proc.State.Energy[pw].Current)
						/ (double) Shm->Sleep.Interval;

		Pkg_ComputePowerLimits(pw);
	    }
		/* Package thermal formulas				*/
	    if (Shm->Proc.Features.Power.EAX.PTM) {
		PFlip->Thermal.Sensor = Proc->PowerThermal.Sensor;
		PFlip->Thermal.Events = Proc->PowerThermal.Events;

		Pkg_ComputeThermalFormula(PFlip, SProc);
	    }

	    Pkg_ComputeVoltageFormula(&Shm->Cpu[Shm->Proc.Service.Core], SProc);

		/*
		The Driver tick is bound to the Service Core:
		1- Tasks collection; Tasks count; and Memory usage.
		2- Processor has resumed from Suspend To RAM.
		*/
		Shm->SysGate.tickStep = Proc->tickStep;
	    if (Shm->SysGate.tickStep == Shm->SysGate.tickReset) {
		if (BITWISEAND(LOCKLESS, Shm->SysGate.Operation, 0x1))
		{
		    if (SysGate_OnDemand(Ref, 1) == 0) {
			SysGate_Update(Ref);
		    }
		}
		if (BITVAL(Proc->OS.Signal, NTFY)) {
			BITCLR(BUS_LOCK, Proc->OS.Signal, NTFY);

			UpdateFeatures(Ref);

			BITWISESET(LOCKLESS,Ref->Shm->Proc.Sync,BIT_MASK_NTFY);
		}
	    }
		/* All aggregations done: Notify Clients.		*/
		BITWISESET(LOCKLESS, Shm->Proc.Sync, BIT_MASK_SYNC);
	}
	/* Reset the Room mask						*/
	BITSTOR_CC(BUS_LOCK, roomCore, roomSeed);

	UBENCH_RDCOUNTER(2);

	UBENCH_COMPUTE();
	Print_uBenchmark((Quiet & 0x100));
    }
    for (cpu = 0; cpu < Shm->Proc.CPU.Count; cpu++) {
	if (Arg[cpu].TID) {
		pthread_join(Arg[cpu].TID, NULL);
	}
    }
	free(Arg);
  } else {
	REASON_SET(reason, RC_MEM_ERR, (errno == 0 ? ENOMEM : errno));
  }
	return (reason);
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
	return (reason);
}

REASON_CODE Shm_Manager(FD *fd, PROC *Proc, uid_t uid, uid_t gid, mode_t cmask)
{
	unsigned int	cpu = 0;
	CORE		**Core;
	SHM_STRUCT	*Shm = NULL;
	const size_t	CoreSize  = ROUND_TO_PAGES(sizeof(CORE));
	REASON_INIT(reason);

    if ((Core = calloc(Proc->CPU.Count, sizeof(Core))) == NULL) {
	REASON_SET(reason, RC_MEM_ERR, (errno == 0 ? ENOMEM : errno));
    }
    for (cpu = 0; (reason.rc == RC_SUCCESS) && (cpu < Proc->CPU.Count); cpu++)
    {
	const off_t offset = (10 + cpu) * PAGE_SIZE;

	if ((Core[cpu] = mmap(	NULL, CoreSize,
				PROT_READ|PROT_WRITE,
				MAP_SHARED,
				fd->Drv, offset)) == MAP_FAILED)
	{
		REASON_SET(reason, RC_SHM_MMAP);
	}
    }
    if (reason.rc == RC_SUCCESS) {
	if (gid != 0) {
		if (setregid(-1, gid) != 0) {
			REASON_SET(reason, RC_SYS_CALL);
		}
	}
    }
    if (reason.rc == RC_SUCCESS) {
	if (uid != 0) {
		if (setreuid(-1, uid) != 0) {
			REASON_SET(reason, RC_SYS_CALL);
		}
	}
    }
    if (reason.rc == RC_SUCCESS) {
	umask(cmask);
    }
    if (reason.rc == RC_SUCCESS)
    {	/* Initialize shared memory.					*/
	const size_t ShmCpuSize = sizeof(CPU_STRUCT) * Proc->CPU.Count,
			ShmSize = ROUND_TO_PAGES((sizeof(SHM_STRUCT)
				+ ShmCpuSize));

      if ((fd->Svr = shm_open(SHM_FILENAME, O_CREAT|O_TRUNC|O_RDWR,
					 S_IRUSR|S_IWUSR
					|S_IRGRP|S_IWGRP
					|S_IROTH|S_IWOTH)) != -1)
      {
	pid_t CPID = -1;

	if (ftruncate(fd->Svr, ShmSize) != -1)
	{
	    if ((Shm = mmap(NULL, ShmSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd->Svr, 0)) != MAP_FAILED)
	    {
		__typeof__ (errno) fork_err = 0;
		/* Clear SHM						*/
		memset(Shm, 0, ShmSize);
		/* Store version footprint into SHM			*/
		SET_FOOTPRINT(Shm->FootPrint,	COREFREQ_MAJOR,
						COREFREQ_MINOR,
						COREFREQ_REV	);
		/* Store the daemon gate name.				*/
		StrCopy(Shm->ShmName, SHM_FILENAME, TASK_COMM_LEN);
		/* Initialize the busy wait times.			*/
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
		Uncore(Shm, Proc, Core[Proc->Service.Core]);
		memcpy(&Shm->SMB, &Proc->SMB, sizeof(SMBIOS_ST));

		/* Initialize notifications.				*/
		BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC0);
		BITCLR(LOCKLESS, Shm->Proc.Sync, SYNC1);

		SysGate_Toggle(&Ref, SysGateStartUp);

		/* Welcomes with brand and per CPU base clock.		*/
	      if (Quiet & 0x001) {
		printf( "CoreFreq Daemon %s"		\
				"  Copyright (C) 2015-2020 CYRIL INGENIERIE\n",
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
				if (setregid(-1, 0) != 0) {
					REASON_SET(reason, RC_SYS_CALL);
				}
			}
			if (uid != 0) {
				if (setreuid(-1, 0) != 0) {
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
    for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
    {
	if (Core[cpu] != NULL) {
	    if (munmap(Core[cpu], CoreSize) == -1) {
		REASON_SET(reason, RC_SHM_MMAP);
	    }
	}
    }
    if (Core != NULL) {
	free(Core);
    }
	return (reason);
}

REASON_CODE Help(REASON_CODE reason, ...)
{
	va_list ap;
	va_start(ap, reason);
	switch (reason.rc) {
	case RC_SUCCESS:
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
			"\t0\tSUCCESS\t\tSuccessful execution\n"	\
			"\t1\tCMD_SYNTAX\tCommand syntax error\n"	\
			"\t2\tSHM_FILE\tShared memory file error\n"	\
			"\t3\tSHM_MMAP\tShared memory mapping error\n"	\
			"\t4\tPERM_ERR\tExecution not permitted\n"	\
			"\t5\tMEM_ERR\t\tMemory operation error\n"	\
			"\t6\tEXEC_ERR\tGeneral execution error\n"	\
			"\t15\tSYS_CALL\tSystem call error\n"		\
			"\nReport bugs to labs[at]cyring.fr\n", appName);
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
	return (reason);
}

int main(int argc, char *argv[])
{
	FD   fd = {0, 0};
	PROC *Proc = NULL;	/* Kernel module anchor point.		*/
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
		const size_t packageSize = ROUND_TO_PAGES(sizeof(PROC));

		if ((Proc = mmap(NULL, packageSize,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd.Drv, 0)) != MAP_FAILED)
		{
		    if (CHK_FOOTPRINT(Proc->FootPrint,	COREFREQ_MAJOR,
							COREFREQ_MINOR,
							COREFREQ_REV)	)
		    {
			reason = Shm_Manager(&fd, Proc, uid, gid, cmask);

			switch (reason.rc) {
			case RC_SUCCESS:
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
					Proc->FootPrint.major,
					Proc->FootPrint.minor,
					Proc->FootPrint.rev);
				reason = Help(reason, wrongVersion);
				free(wrongVersion);
			}
		    }
		    if (munmap(Proc, packageSize) == -1) {
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
	return (reason.rc);
}

