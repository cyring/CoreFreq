/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/pci.h>
#ifdef CONFIG_DMI
#include <linux/dmi.h>
#endif /* CONFIG_DMI */
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/percpu.h>
#include <linux/utsname.h>
#ifdef CONFIG_CPU_IDLE
#include <linux/cpuidle.h>
#endif /* CONFIG_CPU_IDLE */
#ifdef CONFIG_CPU_FREQ
#include <linux/cpufreq.h>
#endif /* CONFIG_CPU_FREQ */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif /* KERNEL_VERSION(4, 11, 0) */
#include <linux/clocksource.h>
#ifdef CONFIG_XEN
#include <xen/xen.h>
#endif /* CONFIG_XEN */
#include <asm/sysreg.h>
#ifdef CONFIG_ACPI
#include <linux/acpi.h>
#include <acpi/processor.h>
#endif
#ifdef CONFIG_ACPI_CPPC_LIB
#include <acpi/cppc_acpi.h>
#endif

#include "bitasm.h"
#include "arm_reg.h"
#include "coretypes.h"
#include "corefreq-api.h"
#include "corefreqk.h"

MODULE_AUTHOR ("CYRIL COURTIAT <labs[at]cyring[dot]fr>");
MODULE_DESCRIPTION ("CoreFreq Processor Driver");
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
MODULE_SUPPORTED_DEVICE ("Intel,AMD,HYGON");
#endif
MODULE_LICENSE ("GPL");
MODULE_VERSION (COREFREQ_VERSION);

static signed int ArchID = -1;
module_param(ArchID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ArchID, "Force an architecture (ID)");

static signed int AutoClock = 0b11;
module_param(AutoClock, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(AutoClock, "Estimate Clock Frequency 0:Spec; 1:Once; 2:Auto");

static unsigned int SleepInterval = 0;
module_param(SleepInterval, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SleepInterval, "Timer interval (ms)");

static unsigned int TickInterval = 0;
module_param(TickInterval, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TickInterval, "System requested interval (ms)");

static signed int Experimental = 0;
module_param(Experimental, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Experimental, "Enable features under development");

static signed int CPU_Count = -1;
module_param(CPU_Count, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(CPU_Count, "-1:Kernel(default); 0:Hardware; >0: User value");

static signed short Target_Ratio_Unlock = -1;
module_param(Target_Ratio_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Target_Ratio_Unlock, "1:Target Ratio Unlock; 0:Lock");

static signed short Clock_Ratio_Unlock = -1;
module_param(Clock_Ratio_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Clock_Ratio_Unlock, "1:MinRatio; 2:MaxRatio; 3:Both Unlock");

static signed short Turbo_Ratio_Unlock = -1;
module_param(Turbo_Ratio_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Turbo_Ratio_Unlock, "1:Turbo Ratio Unlock; 0:Lock");

static signed short Uncore_Ratio_Unlock = -1;
module_param(Uncore_Ratio_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Uncore_Ratio_Unlock, "1:Uncore Ratio Unlock; 0:Lock");

static signed int ServiceProcessor = -1; /* -1=ANY ; 0=BSP */
module_param(ServiceProcessor, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ServiceProcessor, "Select a CPU to run services with");

static SERVICE_PROC DefaultSMT = RESET_SERVICE;

static unsigned short NMI_Disable = 1;
module_param(NMI_Disable, ushort, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(NMI_Disable, "Disable the NMI Handler");

static unsigned int TurboBoost_Enable_Count = 1;
static signed short TurboBoost_Enable[2] = {-1, -1};
module_param_array(TurboBoost_Enable, short, &TurboBoost_Enable_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TurboBoost_Enable, "Enable Turbo Boost");

static enum RATIO_BOOST Ratio_Boost_Count = 0;
static signed int Ratio_Boost[BOOST(SIZE) - BOOST(18C)] = {
	/*	18C		*/	-1,
	/*	17C		*/	-1,
	/*	16C		*/	-1,
	/*	15C		*/	-1,
	/*	14C		*/	-1,
	/*	13C		*/	-1,
	/*	12C		*/	-1,
	/*	11C		*/	-1,
	/*	10C		*/	-1,
	/*	 9C		*/	-1,
	/*	 8C		*/	-1,
	/*	 7C		*/	-1,
	/*	 6C		*/	-1,
	/*	 5C		*/	-1,
	/*	 4C		*/	-1,
	/*	 3C		*/	-1,
	/*	 2C		*/	-1,
	/*	 1C		*/	-1
};
module_param_array(Ratio_Boost, int, &Ratio_Boost_Count,	\
				S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Ratio_Boost, "Turbo Boost Frequency ratios");

static signed int Ratio_PPC = -1;
module_param(Ratio_PPC, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Ratio_PPC, "Target Performance ratio");

static signed short HWP_Enable = -1;
module_param(HWP_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HWP_Enable, "Hardware-Controlled Performance States");

static signed short HWP_EPP = -1;
module_param(HWP_EPP, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HWP_EPP, "Energy Performance Preference");

static enum RATIO_BOOST Ratio_HWP_Count = 0;
static signed int Ratio_HWP[1 + (BOOST(HWP_TGT) - BOOST(HWP_MIN))] = {
	/*	HWP_MIN 	*/	-1,
	/*	HWP_MAX 	*/	-1,
	/*	HWP_TGT 	*/	-1
};
module_param_array(Ratio_HWP, int, &Ratio_HWP_Count,	\
				S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Ratio_HWP, "Hardware-Controlled Performance ratios");

static unsigned long long Clear_Events = 0;
module_param(Clear_Events, ullong, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Clear_Events, "Clear Thermal and Power Events");

static int ThermalScope = -1;
module_param(ThermalScope, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ThermalScope, "[0:None; 1:SMT; 2:Core; 3:Package]");

static int VoltageScope = -1;
module_param(VoltageScope, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(VoltageScope, "[0:None; 1:SMT; 2:Core; 3:Package]");

static int PowerScope = -1;
module_param(PowerScope, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerScope, "[0:None; 1:SMT; 2:Core; 3:Package]");

static signed short Register_CPU_Idle = -1;
module_param(Register_CPU_Idle, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Register_CPU_Idle, "Register the Kernel cpuidle driver");

static signed short Register_CPU_Freq = -1;
module_param(Register_CPU_Freq, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Register_CPU_Freq, "Register the Kernel cpufreq driver");

static signed short Register_Governor = -1;
module_param(Register_Governor, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Register_Governor, "Register the Kernel governor");

static signed short Register_ClockSource = -1;
module_param(Register_ClockSource, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Register_ClockSource, "Register Clock Source driver");

static signed short Idle_Route = -1;
module_param(Idle_Route, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Idle_Route, "[0:Default; 1:I/O; 2:HALT; 3:MWAIT]");

static struct {
	signed int		Major;
	struct cdev		*kcdev;
	dev_t			nmdev, mkdev;
	struct class		*clsdev;
#ifdef CONFIG_CPU_IDLE
	struct cpuidle_device __percpu *IdleDevice;
	struct cpuidle_driver	IdleDriver;
#endif /* CONFIG_CPU_IDLE */
#ifdef CONFIG_CPU_FREQ
	struct cpufreq_driver	FreqDriver;
	struct cpufreq_governor FreqGovernor;
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_PM_SLEEP
	bool			ResumeFromSuspend;
#endif /* CONFIG_PM_SLEEP */
	unsigned int		SubCstate[SUB_CSTATE_COUNT];
} CoreFreqK = {
#ifdef CONFIG_CPU_IDLE
	.IdleDriver = {
			.name	= "corefreqk-idle",
			.owner	= THIS_MODULE
	},
#endif /* CONFIG_CPU_IDLE */
#ifdef CONFIG_CPU_FREQ
	.FreqDriver = {
			.name	= "corefreqk-perf",
			.flags	= CPUFREQ_CONST_LOOPS,
			.exit	= CoreFreqK_Policy_Exit,
	/*MANDATORY*/	.init	= CoreFreqK_Policy_Init,
	/*MANDATORY*/	.verify = CoreFreqK_Policy_Verify,
	/*MANDATORY*/	.setpolicy = CoreFreqK_SetPolicy,
			.bios_limit= CoreFreqK_Bios_Limit,
			.set_boost = CoreFreqK_SetBoost
	},
	.FreqGovernor = {
			.name	= "corefreq-policy",
			.owner	= THIS_MODULE,
			.show_setspeed	= CoreFreqK_Show_SetSpeed,
			.store_setspeed = CoreFreqK_Store_SetSpeed
	},
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_PM_SLEEP
	.ResumeFromSuspend = false,
#endif /* CONFIG_PM_SLEEP */
};

static KPUBLIC *KPublic = NULL;
static KPRIVATE *KPrivate = NULL;
static ktime_t RearmTheTimer;

#define AT( _loc_ )		[ _loc_ ]
#define OF( _ptr_ , ...)	-> _ptr_ __VA_ARGS__
#define RO( _ptr_ , ...)	OF( _ptr_##_RO , __VA_ARGS__ )
#define RW( _ptr_ , ...)	OF( _ptr_##_RW , __VA_ARGS__ )
#define ADDR( _head_ , _mbr_ )	( _head_ _mbr_ )
#define PUBLIC(...)		ADDR( KPublic , __VA_ARGS__ )
#define PRIVATE(...)		ADDR( KPrivate, __VA_ARGS__ )

#define RESET_ARRAY(_array, _cnt, _val, ... )				\
({									\
	unsigned int rst;						\
	for (rst = 0; rst < _cnt; rst++) {				\
		_array[rst] __VA_ARGS__ = _val;				\
	}								\
})

static long CoreFreqK_Thermal_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	PUBLIC(RO(Proc))->thermalFormula = \
		(KIND_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula) << 8)|scope;

	return RC_SUCCESS;
    } else {
	return -EINVAL;
    }
}

static long CoreFreqK_Voltage_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	PUBLIC(RO(Proc))->voltageFormula = \
		(KIND_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula) << 8)|scope;

	return RC_SUCCESS;
    } else {
	return -EINVAL;
    }
}

static long CoreFreqK_Power_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	PUBLIC(RO(Proc))->powerFormula = \
		(KIND_OF_FORMULA(PUBLIC(RO(Proc))->powerFormula) << 8)|scope;

	return RC_SUCCESS;
    } else {
	return -EINVAL;
    }
}

unsigned int FixMissingRatioAndFrequency(unsigned int r32, CLOCK *pClock)
{
	unsigned long long r64 = r32;
  if (PUBLIC(RO(Proc))->Features.Factory.Freq != 0)
  {
   if ((r32 == 0) && (pClock->Q > 0))
   {	/*	Fix missing ratio.					*/
      r64=DIV_ROUND_CLOSEST(PUBLIC(RO(Proc))->Features.Factory.Freq, pClock->Q);
      PUBLIC(RO(Core,AT(PUBLIC(RO(Proc))->Service.Core)))->Boost[BOOST(MAX)]=\
		(unsigned int) r64;
   }
  }
  else if (r32 > 0)
  {	/*	Fix the Factory frequency (unit: MHz)			*/
	r64 = pClock->Hz * r32;
	r64 = r64 / 1000000LLU;
	PUBLIC(RO(Proc))->Features.Factory.Freq = (unsigned int) r64;
  }
	PUBLIC(RO(Proc))->Features.Factory.Clock.Q  = pClock->Q;
	PUBLIC(RO(Proc))->Features.Factory.Clock.R  = pClock->R;
	PUBLIC(RO(Proc))->Features.Factory.Clock.Hz = pClock->Hz;

  if (PUBLIC(RO(Proc))->Features.Factory.Clock.Hz > 0)
  {
    r64 = PUBLIC(RO(Proc))->Features.Factory.Freq * 1000000LLU;
    r64 = DIV_ROUND_CLOSEST(r64, PUBLIC(RO(Proc))->Features.Factory.Clock.Hz);
    PUBLIC(RO(Proc))->Features.Factory.Ratio = (unsigned int) r64;
  }
	return (unsigned int) r64;
}

static unsigned long long
CoreFreqK_Read_CS_From_Invariant_TSC(struct clocksource *cs)
{
	unsigned long long TSC __attribute__ ((aligned (8)));
	UNUSED(cs);
	RDTSCP64(TSC);
	return TSC;
}

static unsigned long long
CoreFreqK_Read_CS_From_Variant_TSC(struct clocksource *cs)
{
	unsigned long long TSC __attribute__ ((aligned (8)));
	UNUSED(cs);
	RDTSC64(TSC);
	return TSC;
}

static struct clocksource CoreFreqK_CS = {
	.name	= "corefreq_tsc",
	.rating = 300,
	.mask	= CLOCKSOURCE_MASK(64),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS
		| CLOCK_SOURCE_VALID_FOR_HRES,
};

static long CoreFreqK_UnRegister_ClockSource(void)
{
	long rc = -EINVAL;
    if (PUBLIC(RO(Proc))->Registration.Driver.CS & REGISTRATION_ENABLE)
    {
	int rx = clocksource_unregister(&CoreFreqK_CS);
	switch ( rx ) {
	case 0:
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_DISABLE;
		rc = RC_SUCCESS;
		break;
	default:
		rc = (long) rx;
		break;
	}
    }
	return rc;
}

static long CoreFreqK_Register_ClockSource(unsigned int cpu)
{
	long rc = -EINVAL;
    if (Register_ClockSource == 1)
    {
	unsigned long long Freq_Hz;
	unsigned int Freq_KHz;

	if ((PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (PUBLIC(RO(Proc))->Features.ExtInfo.EDX.RDTSCP == 1))
	{
		CoreFreqK_CS.read = CoreFreqK_Read_CS_From_Invariant_TSC;
	}
	else
	{
		CoreFreqK_CS.read = CoreFreqK_Read_CS_From_Variant_TSC;
	}

	Freq_Hz = PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)]
		* PUBLIC(RO(Core, AT(cpu)))->Clock.Hz;
	Freq_KHz = Freq_Hz / 1000U;
	if (Freq_KHz != 0)
	{
		int rx = clocksource_register_khz(&CoreFreqK_CS, Freq_KHz);
	    switch (rx) {
	    default:
		fallthrough;
	    case -EBUSY:
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_DISABLE;
		rc = (long) rx;
		break;
	    case 0:
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_ENABLE;
		rc = RC_SUCCESS;
		break;
	    }
	}
    } else {
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_DISABLE;
    }
	return rc;
}

void VendorFromMainID(	MIDR midr, char *pVendorID,
			unsigned int *pCRC, enum HYPERVISOR *pHypervisor )
{
    const struct {
		unsigned short		implementer;
		char			*vendorID;
		size_t			vendorLen;
		enum CRC_MANUFACTURER	mfrCRC;
		enum HYPERVISOR 	hypervisor;
    } mfrTbl[] = {
	{	0x00,	VENDOR_RESERVED, __builtin_strlen(VENDOR_RESERVED),
		CRC_RESERVED,	BARE_METAL				},
	{	0x41,	VENDOR_ARM,	__builtin_strlen(VENDOR_ARM),
		CRC_ARM,	BARE_METAL				},
	{	0x42,	VENDOR_BROADCOM, __builtin_strlen(VENDOR_BROADCOM),
		CRC_BROADCOM,	BARE_METAL				},
	{	0x43,	VENDOR_CAVIUM,	__builtin_strlen(VENDOR_CAVIUM),
		CRC_CAVIUM,	BARE_METAL				},
	{	0x44,	VENDOR_DEC,	__builtin_strlen(VENDOR_DEC),
		CRC_DEC,	BARE_METAL				},
	{	0x46,	VENDOR_FUJITSU, __builtin_strlen(VENDOR_FUJITSU),
		CRC_FUJITSU,	BARE_METAL				},
	{	0x49,	VENDOR_INFINEON, __builtin_strlen(VENDOR_INFINEON),
		CRC_INFINEON,	BARE_METAL				},
	{	0x4d,	VENDOR_MOTOROLA, __builtin_strlen(VENDOR_MOTOROLA),
		CRC_MOTOROLA,	BARE_METAL				},
	{	0x4e,	VENDOR_NVIDIA,	__builtin_strlen(VENDOR_NVIDIA),
		CRC_NVIDIA,	BARE_METAL				},
	{	0x50,	VENDOR_APM,	__builtin_strlen(VENDOR_APM),
		CRC_APM,	BARE_METAL				},
	{	0x51,	VENDOR_QUALCOMM, __builtin_strlen(VENDOR_QUALCOMM),
		CRC_QUALCOMM,	BARE_METAL				},
	{	0x56,	VENDOR_MARVELL, __builtin_strlen(VENDOR_MARVELL),
		CRC_MARVELL,	BARE_METAL				},
	{	0x69,	VENDOR_INTEL,	__builtin_strlen(VENDOR_INTEL),
		CRC_INTEL,	BARE_METAL				},
	{	0xc0,	VENDOR_AMPERE,	__builtin_strlen(VENDOR_AMPERE),
		CRC_AMPERE,	BARE_METAL				},

	{	0xff,	VENDOR_KVM,	__builtin_strlen(VENDOR_KVM),
		CRC_KVM,	HYPERV_KVM				},
	{	0xff,	VENDOR_VBOX,	__builtin_strlen(VENDOR_VBOX),
		CRC_VBOX,	HYPERV_VBOX				},
	{	0xff,	VENDOR_KBOX,	__builtin_strlen(VENDOR_KBOX),
		CRC_KBOX,	HYPERV_KBOX				},
	{	0xff,	VENDOR_VMWARE,	__builtin_strlen(VENDOR_VMWARE),
		CRC_VMWARE,	HYPERV_VMWARE				},
	{	0xff,	VENDOR_HYPERV,	__builtin_strlen(VENDOR_HYPERV),
		CRC_HYPERV,	HYPERV_HYPERV				}
    };
	unsigned int idx;
    for (idx = 0; idx < sizeof(mfrTbl) / sizeof(mfrTbl[0]); idx++) {
	if (midr.Implementer == mfrTbl[idx].implementer)
	{
		memcpy(pVendorID, mfrTbl[idx].vendorID, mfrTbl[idx].vendorLen);
		(*pCRC) = mfrTbl[idx].mfrCRC;
		(*pHypervisor) = mfrTbl[idx].hypervisor;

		return;
	}
    }
}

signed int SearchArchitectureID(void)
{
	signed int id;
    for (id = ARCHITECTURES - 1; id > 0; id--)
    {	/* Search for an architecture signature. */
	if ( (PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily \
		== Arch[id].Signature.ExtFamily)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Family \
		== Arch[id].Signature.Family)
	&& ( ( (PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel \
			==  Arch[id].Signature.ExtModel)
		&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model \
			==  Arch[id].Signature.Model) )
		|| (!Arch[id].Signature.ExtModel \
		&& !Arch[id].Signature.Model) ) )
	{
		break;
	}
    }
	return id;
}

/* Retreive the Processor(BSP) features. */
static void Query_Features(void *pArg)
{
	INIT_ARG *iArg = (INIT_ARG *) pArg;

	volatile MIDR midr;
	volatile CNTFRQ cntfrq;
	volatile CNTPCT cntpct;
	volatile PMCR pmcr;
	volatile AA64DFR0 dbgfr0;
	volatile AA64ISAR0 isar0;
	volatile AA64MMFR1 mmfr1;
	volatile AA64PFR0 pfr0;
	volatile AA64PFR1 pfr1;
	volatile CLUSTERCFR clustercfg;

	iArg->Features->Info.LargestStdFunc = 0x1;
	iArg->Features->Info.LargestExtFunc = 0x80000001;
	iArg->Features->Info.Vendor.CRC = CRC_RESERVED;
	iArg->SMT_Count = 1;
	iArg->HypervisorID = HYPERV_NONE;

	__asm__ __volatile__(
		"mrs	%[midr] ,	midr_el1"	"\n\t"
		"mrs	%[cntfrq],	cntfrq_el0"	"\n\t"
		"mrs	%[cntpct],	cntpct_el0"	"\n\t"
		"mrs	%[pmcr] ,	pmcr_el0"	"\n\t"
		"mrs	%[dbgfr0],	id_aa64dfr0_el1""\n\t"
		"mrs	%[isar0],	id_aa64isar0_el1""\n\t"
		"mrs	%[mmfr1],	id_aa64mmfr1_el1""\n\t"
		"mrs	%[pfr0] ,	id_aa64pfr0_el1""\n\t"
		"mrs	%[pfr1] ,	id_aa64pfr1_el1""\n\t"
		"isb"
		: [midr]	"=r" (midr),
		  [cntfrq]	"=r" (cntfrq),
		  [cntpct]	"=r" (cntpct),
		  [pmcr]	"=r" (pmcr),
		  [dbgfr0]	"=r" (dbgfr0),
		  [isar0]	"=r" (isar0),
		  [mmfr1]	"=r" (mmfr1),
		  [pfr0]	"=r" (pfr0),
		  [pfr1]	"=r" (pfr1)
		:
		: "memory"
	);
	iArg->Features->Std.EAX.Stepping = midr.Revision | (midr.Variant << 4);
	iArg->Features->Std.EAX.Family = midr.PartNum & 0x00f;
	iArg->Features->Std.EAX.ExtFamily = (midr.PartNum & 0xff0) >> 4;
	iArg->Features->Std.EAX.Model = pmcr.IDcode & 0x0f;
	iArg->Features->Std.EAX.ExtModel = (pmcr.IDcode & 0xf0) >> 4;

	VendorFromMainID(midr, iArg->Features->Info.Vendor.ID,
			&iArg->Features->Info.Vendor.CRC, &iArg->HypervisorID);

	iArg->Features->Factory.Freq = cntfrq.ClockFrequency;
	iArg->Features->Factory.Freq /= 10000;

#if defined(CONFIG_ACPI)
	iArg->Features->Std.EDX.ACPI = acpi_disabled == 0;
#else
	iArg->Features->Std.EDX.ACPI = 0;
#endif
	iArg->Features->Std.EDX.TSC = \
	iArg->Features->AdvPower.EDX.Inv_TSC = \
	iArg->Features->ExtInfo.EDX.RDTSCP = cntpct.PhysicalCount != 0;

	iArg->Features->PerfMon.EDX.FixCtrs = 1; /* Fixed Cycle Counter */
	iArg->Features->PerfMon.EAX.MonCtrs = pmcr.NumEvtCtrs;
	iArg->Features->PerfMon.EAX.Version = dbgfr0.PMUVer;

    if (iArg->Features->PerfMon.EAX.Version) {
	volatile unsigned long long pmcfgr;
	/*TODO(Memory-mapped PMU register at offset 0xe00)		*/
	__asm__ __volatile__(
		"" 	"\n\t"
		"isb"
		: [pmcfgr]	"=r" (pmcfgr)
		:
		: "memory"
	);
	iArg->Features->PerfMon.EAX.MonWidth = \
	iArg->Features->PerfMon.EDX.FixWidth = 0b111111 == 0b111111 ? 64 : 0;
    }
	iArg->Features->Std.ECX.AES = isar0.AES == 0x2;
	iArg->Features->ExtFeature.EBX.SHA = (isar0.SHA1 == 0x1)
					  || (isar0.SHA2 == 0x1);
	iArg->Features->Std.EDX.CMPXCHG8 = isar0.CAS == 0x2;
	iArg->Features->Std.ECX.RDRAND = isar0.RNDR == 0x1;
	iArg->Features->Std.ECX.VMX = mmfr1.VH == 0x1;
	iArg->Features->Std.EDX.FPU = pfr0.FP == 0x1;
	iArg->Features->Std.EDX.SSE = pfr0.AdvSIMD == 0x1;
	iArg->Features->Std.EDX.APIC = pfr0.GIC == 0x1;
	iArg->Features->SSBS = pfr1.SSBS;
	iArg->Features->CSV2 = pfr1.CSV2_frac;

    if (Experimental && (iArg->HypervisorID == HYPERV_NONE)) {
	/* Query the Cluster Configuration				*/
	clustercfg.value = read_sysreg_s(CLUSTERCFR_EL1);
	if (clustercfg.NUMCORE) {
		iArg->SMT_Count = iArg->SMT_Count + clustercfg.NUMCORE;
	}
    }
	/* Reset the performance features bits (present is zero)	*/
	iArg->Features->PerfMon.EBX.CoreCycles    = 1;
	iArg->Features->PerfMon.EBX.InstrRetired  = 1;
	iArg->Features->PerfMon.EBX.RefCycles     = 1;
	iArg->Features->PerfMon.EBX.LLC_Ref       = 1;
	iArg->Features->PerfMon.EBX.LLC_Misses    = 1;
	iArg->Features->PerfMon.EBX.BranchRetired = 1;
	iArg->Features->PerfMon.EBX.BranchMispred = 1;
	iArg->Features->PerfMon.EBX.TopdownSlots  = 1;
}

void Compute_Interval(void)
{
	if ( (SleepInterval >= LOOP_MIN_MS)
	  && (SleepInterval <= LOOP_MAX_MS))
	{
		PUBLIC(RO(Proc))->SleepInterval = SleepInterval;
	} else {
		PUBLIC(RO(Proc))->SleepInterval = LOOP_DEF_MS;
	}
	/*		Compute the tick steps .			*/
	PUBLIC(RO(Proc))->tickReset = \
		(  (TickInterval >= PUBLIC(RO(Proc))->SleepInterval)
		&& (TickInterval <= LOOP_MAX_MS) ) ?
			TickInterval
		:	KMAX(TICK_DEF_MS, PUBLIC(RO(Proc))->SleepInterval);

	PUBLIC(RO(Proc))->tickReset /= PUBLIC(RO(Proc))->SleepInterval;
	PUBLIC(RO(Proc))->tickStep = PUBLIC(RO(Proc))->tickReset;

	RearmTheTimer = ktime_set( 0,	PUBLIC(RO(Proc))->SleepInterval
					* 1000000LU );
}

#ifdef CONFIG_SMP
/*TODO	#define THIS_LPJ	this_cpu_read(cpu_info.loops_per_jiffy)*/
	#define THIS_LPJ	loops_per_jiffy
#else
	#define THIS_LPJ	loops_per_jiffy
#endif

#define COMPUTE_LPJ(BCLK_Hz, COF)	( (BCLK_Hz * COF) / HZ )

#if defined(DELAY_TSC) && (DELAY_TSC == 1)
/*			udelay() built with TSC implementation		*/
#define CLOCK_TSC( CYCLES, _TIMER, CTR )				\
({									\
/*TODO	__asm__ volatile						\
	(								\
		ASM_RD##_TIMER(r14)					\
		"addq	%[cycles], %%rax"		"\n\t"		\
		"movq	%%rax	, %%r15"		"\n\t"		\
	"1:"						"\n\t"		\
		ASM_RD##_TIMER(r13)					\
		"cmpq	%%r15	, %%r13"		"\n\t"		\
		"jc	1b"				"\n\t"		\
		"movq	%%r14	, %[ctr0]"		"\n\t"		\
		"movq	%%r13	, %[ctr1]"				\
		: [ctr0] "=m"	(CTR[0]),				\
		  [ctr1] "=m"	(CTR[1])				\
		: [cycles] "ir" (CYCLES)				\
		: "%rax", "%rbx", "%rcx", "%rdx",			\
		  "%r13", "%r14", "%r15",				\
		  "cc", "memory"					\
	);							*/	\
})

#define CLOCK2CYCLE(INTERVAL_NS) ((INTERVAL_NS * THIS_LPJ * HZ) / 1000000LLU)

#define CLOCK_DELAY(INTERVAL_NS, _TIMER, CTR)				\
({									\
	CLOCK_TSC( CLOCK2CYCLE(INTERVAL_NS), _TIMER, CTR );		\
})

#define CLOCK_OVERHEAD(_TIMER, CTR)	CLOCK_TSC( 1LLU, _TIMER, CTR )

#else

#define CLOCK_DELAY(INTERVAL_NS, _TIMER, CTR)				\
({									\
	RD##_TIMER##64(CTR[0]);						\
	udelay(INTERVAL_NS);						\
	RD##_TIMER##64(CTR[1]);						\
})

#define CLOCK_OVERHEAD(_TIMER, CTR)	CLOCK_DELAY( 0LLU, _TIMER, CTR )

#endif

static void ComputeWithSerializedTSC(COMPUTE_ARG *pCompute)
{
	unsigned int loop;
	/*		Writeback and Invalidate Caches.		*/
	WBINVD();
	/*		Warm-up & Overhead				*/
	for (loop = 0; loop < OCCURRENCES; loop++)
	{
		CLOCK_OVERHEAD(TSCP, pCompute->TSC[0][loop].V);
	}
	/*		Estimation					*/
	for (loop = 0; loop < OCCURRENCES; loop++)
	{
		CLOCK_DELAY(1000LLU, TSCP, pCompute->TSC[1][loop].V);
	}
}

static void ComputeWithUnSerializedTSC(COMPUTE_ARG *pCompute)
{
	unsigned int loop;
	/*		Writeback and Invalidate Caches.		*/
	WBINVD();
	/*		Warm-up & Overhead				*/
	for (loop = 0; loop < OCCURRENCES; loop++)
	{
		CLOCK_OVERHEAD(TSC, pCompute->TSC[0][loop].V);
	}
	/*		Estimation					*/
	for (loop = 0; loop < OCCURRENCES; loop++)
	{
		CLOCK_DELAY(1000LLU, TSC, pCompute->TSC[1][loop].V);
	}
}

static void Compute_TSC(void *arg)
{
	COMPUTE_ARG *pCompute = (COMPUTE_ARG *) arg;
	unsigned long long D[2][OCCURRENCES];
	unsigned int ratio = (unsigned int) pCompute->Clock.Q;
	unsigned int loop = 0, what = 0, best[2] = {0, 0}, top[2] = {0, 0};
/*
	TSC[0] stores the overhead
	TSC[1] stores the estimation
*/
	/*	Is the TSC invariant or can serialize  ?		*/
	if ((PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (PUBLIC(RO(Proc))->Features.ExtInfo.EDX.RDTSCP == 1))
	{
		ComputeWithSerializedTSC(pCompute);
	} else {
		ComputeWithUnSerializedTSC(pCompute);
	}
	/*		Select the best clock.				*/
	for (loop = 0; loop < OCCURRENCES; loop++) {
		for (what = 0; what < 2; what++) {
			D[what][loop]	= pCompute->TSC[what][loop].V[1]
					- pCompute->TSC[what][loop].V[0];
		}
	}
	for (loop = 0; loop < OCCURRENCES; loop++) {
		unsigned int inner = 0, count[2] = {0, 0};
		for (inner = loop; inner < OCCURRENCES; inner++) {
			for (what = 0; what < 2; what++) {
				if (D[what][loop] == D[what][inner])
					count[what]++;
			}
		}
		for (what = 0; what < 2; what++) {
		    if ((count[what] > top[what])
			|| ((count[what] == top[what])
			&& (D[what][loop] < D[what][best[what]]))) {

				top[what]  = count[what];
				best[what] = loop;

		    }
		}
	}
	/*		Substract the overhead .			*/
	D[1][best[1]] -= D[0][best[0]];
	/*		Compute the Base Clock .			*/
	REL_BCLK(pCompute->Clock, ratio, D[1][best[1]], 1LLU);
}

CLOCK Compute_Clock(unsigned int cpu, COMPUTE_ARG *pCompute)
{
/*	Synchronously call the Base Clock estimation on a pinned CPU.
 * 1/ Preemption is disabled by smp_call_function_single() > get_cpu()
 * 2/ IRQ are suspended by generic_exec_single(func) > local_irq_save()
 * 3/ Function 'func' is executed
 * 4/ IRQ are resumed > local_irq_restore()
 * 5/ Preemption is enabled > put_cpu()
 */
	smp_call_function_single(cpu, Compute_TSC, pCompute, 1);

	return pCompute->Clock;
}

void ClockToHz(CLOCK *clock)
{
	clock->Hz  = clock->Q * 1000000L;
	clock->Hz += clock->R * PRECISION;
}

static CLOCK BaseClock_GenericMachine(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
	UNUSED(ratio);
	return clock;
};

void Cache_Level(CORE_RO *Core, unsigned int level, unsigned int select)
{
	const CSSELR cssel[CACHE_MAX_LEVEL] = {
		[0] = { .InD = 1, .Level = 0 }, /*	L1I	*/
		[1] = { .InD = 0, .Level = 0 }, /*	L1D	*/
		[2] = { .InD = 0, .Level = 1 }, /*	L2	*/
		[3] = { .InD = 0, .Level = 2 }	/*	L3	*/
	};
	__asm__ volatile
	(
		"msr	csselr_el1,	%[cssel]"	"\n\t"
		"mrs	%[ccsid],	ccsidr_el1"	"\n\t"
		"isb"
		: [ccsid]	"=r" (Core->T.Cache[level].ccsid)
		: [cssel]	"r"  (cssel[select])
		: "memory"
	);
}

void Cache_Topology(CORE_RO *Core)
{
	volatile CLIDR clidr;
	__asm__ volatile
	(
		"mrs	%[clidr],	clidr_el1"	"\n\t"
		"isb"
		: [clidr]	"=r" (clidr)
		:
		: "memory"
	);

	if (clidr.Ctype1 == 0b011) {
		Cache_Level(Core, 0, 0);
		Cache_Level(Core, 1, 1);
	} else {
		Cache_Level(Core, 0, 1);
		/*	Skip L1I	*/
	}
	if (clidr.Ctype2 == 0b100) {
		Cache_Level(Core, 2, 2);
	}
	if (clidr.Ctype3 == 0b100) {
		Cache_Level(Core, 3, 3);
	}
}

static void Map_Generic_Topology(void *arg)
{
    if (arg != NULL) {
	CORE_RO *Core = (CORE_RO *) arg;

	volatile MPIDR mpid;
	__asm__ volatile
	(
		"mrs	%[mpid] ,	mpidr_el1"	"\n\t"
		"isb"
		: [mpid]	"=r" (mpid)
		:
		: "memory"
	);
	Core->T.ApicID = mpid.value & 0xfffff;
	Core->T.Cluster.CMP = mpid.Aff3;
	Core->T.PackageID = mpid.Aff2;
	Core->T.CoreID = mpid.Aff1;
	if (mpid.MT) {
		Core->T.ThreadID = mpid.Aff0;
	}
	Cache_Topology(Core);
    }
}

int Core_Topology(unsigned int cpu)
{
	int rc = smp_call_function_single(cpu , Map_Generic_Topology,
						PUBLIC(RO(Core, AT(cpu))), 1);
	if (	!rc
		&& (PUBLIC(RO(Proc))->Features.HTT_Enable == 0)
		&& (PUBLIC(RO(Core, AT(cpu)))->T.ThreadID > 0) )
	{
			PUBLIC(RO(Proc))->Features.Std.EDX.HTT = 1;
			PUBLIC(RO(Proc))->Features.HTT_Enable = 1;
	}
	return rc;
}

unsigned int Proc_Topology(void)
{
	unsigned int cpu, CountEnabledCPU = 0;

    for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++) {
	PUBLIC(RO(Core, AT(cpu)))->T.Base.value = 0;
	PUBLIC(RO(Core, AT(cpu)))->T.ApicID     = -1;
	PUBLIC(RO(Core, AT(cpu)))->T.CoreID     = -1;
	PUBLIC(RO(Core, AT(cpu)))->T.ThreadID   = -1;
	PUBLIC(RO(Core, AT(cpu)))->T.PackageID  = -1;
	PUBLIC(RO(Core, AT(cpu)))->T.Cluster.ID = 0;

	BITSET(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, HW);
	BITSET(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, OS);

	if (cpu_present(cpu)) { /*	CPU state probed by the OS.	*/
	    if (Core_Topology(cpu) == 0) {
		/* CPU state based on the hardware. */
		if (PUBLIC(RO(Core, AT(cpu)))->T.ApicID >= 0)
		{
			BITCLR(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine,HW);

			CountEnabledCPU++;
		}
		BITCLR(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, OS);
	    }
	}
    }
	return CountEnabledCPU;
}

#define HyperThreading_Technology()					\
(									\
	PUBLIC(RO(Proc))->CPU.OnLine = Proc_Topology()			\
)

void Package_Init_Reset(void)
{
	PUBLIC(RO(Proc))->Features.TgtRatio_Unlock = 1;
	PUBLIC(RO(Proc))->Features.ClkRatio_Unlock = 0;
	PUBLIC(RO(Proc))->Features.TDP_Unlock = 0;
	PUBLIC(RO(Proc))->Features.Turbo_Unlock = 0;
	PUBLIC(RO(Proc))->Features.TurboActiv_Lock = 1;
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Lock = 1;
	PUBLIC(RO(Proc))->Features.Uncore_Unlock = 0;
}

void Default_Unlock_Reset(void)
{
    switch (Target_Ratio_Unlock) {
    case COREFREQ_TOGGLE_OFF:
    case COREFREQ_TOGGLE_ON:
	PUBLIC(RO(Proc))->Features.TgtRatio_Unlock = Target_Ratio_Unlock;
	break;
    }
    switch (Clock_Ratio_Unlock) {
    case COREFREQ_TOGGLE_OFF:
    case 0b01:
    case 0b10:
    case 0b11:
	PUBLIC(RO(Proc))->Features.ClkRatio_Unlock = Clock_Ratio_Unlock;
	break;
    }
    switch (Turbo_Ratio_Unlock) {
    case COREFREQ_TOGGLE_OFF:
    case COREFREQ_TOGGLE_ON:
	PUBLIC(RO(Proc))->Features.Turbo_Unlock = Turbo_Ratio_Unlock;
	break;
    }
    switch (Uncore_Ratio_Unlock) {
    case COREFREQ_TOGGLE_OFF:
    case COREFREQ_TOGGLE_ON:
	PUBLIC(RO(Proc))->Features.Uncore_Unlock = Uncore_Ratio_Unlock;
	break;
    }
}

void OverrideCodeNameString(PROCESSOR_SPECIFIC *pSpecific)
{
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[
			PUBLIC(RO(Proc))->ArchID
		].Architecture.Brand[pSpecific->CodeNameIdx], CODENAME_LEN);
}

void OverrideUnlockCapability(PROCESSOR_SPECIFIC *pSpecific)
{
    if (pSpecific->Latch & LATCH_TGT_RATIO_UNLOCK) {
	PUBLIC(RO(Proc))->Features.TgtRatio_Unlock=pSpecific->TgtRatioUnlocked;
    }
    if (pSpecific->Latch & LATCH_CLK_RATIO_UNLOCK) {
	PUBLIC(RO(Proc))->Features.ClkRatio_Unlock=pSpecific->ClkRatioUnlocked;
    }
    if (pSpecific->Latch & LATCH_TURBO_UNLOCK) {
	PUBLIC(RO(Proc))->Features.Turbo_Unlock = pSpecific->TurboUnlocked;
    }
    if (pSpecific->Latch & LATCH_UNCORE_UNLOCK) {
	PUBLIC(RO(Proc))->Features.Uncore_Unlock = pSpecific->UncoreUnlocked;
    }
    if (pSpecific->Latch & LATCH_HSMP_CAPABLE) {
	PUBLIC(RO(Proc))->Features.HSMP_Capable = pSpecific->HSMP_Capable;
    }
}

PROCESSOR_SPECIFIC *LookupProcessor(void)
{
	PROCESSOR_SPECIFIC *pSpecific;
    for (pSpecific = Arch[PUBLIC(RO(Proc))->ArchID].Specific;
		(pSpecific != NULL) && (pSpecific->Brand != NULL);
			pSpecific++)
    {
	char **brands, *brand;
	for (brands = pSpecific->Brand, brand = *brands;
		brand != NULL;
			brands++, brand = *brands)
	{
		if (strstr(PUBLIC(RO(Proc))->Features.Info.Brand, brand)) {
			return pSpecific;
		}
	}
    }
	return NULL;
}

void Compute_ACPI_CPPC_Bounds(unsigned int cpu)
{
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Core->PowerThermal.ACPI_CPPC.Highest > \
		PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum)
	{
		PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum = \
			Core->PowerThermal.ACPI_CPPC.Highest;
	}
	if (Core->PowerThermal.ACPI_CPPC.Highest < \
		PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Minimum)
	{
		PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Minimum = \
			Core->PowerThermal.ACPI_CPPC.Highest;
	}
}

inline signed int Disable_ACPI_CPPC(unsigned int cpu, void *arg)
{
#if defined(CONFIG_ACPI_CPPC_LIB) \
 && LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
	signed int rc = cppc_set_enable((signed int) cpu, false);
#else
	signed int rc = -ENODEV;
#endif /* CONFIG_ACPI_CPPC_LIB */
	UNUSED(arg);

	if (rc != 0) {
		pr_debug("CoreFreq: cppc_set_enable(cpu=%u, false) error %d\n",
			cpu, rc);
	}
	Compute_ACPI_CPPC_Bounds(cpu);

	return rc;
}

inline signed int Enable_ACPI_CPPC(unsigned int cpu, void *arg)
{
#if defined(CONFIG_ACPI_CPPC_LIB) \
 && LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
	signed int rc = cppc_set_enable((signed int) cpu, true);
#else
	signed int rc = -ENODEV;
#endif /* CONFIG_ACPI_CPPC_LIB */
	UNUSED(arg);

	if (rc != 0) {
		pr_debug("CoreFreq: cppc_set_enable(cpu=%u, true) error %d\n",
			cpu, rc);
	}
	Compute_ACPI_CPPC_Bounds(cpu);

	return rc;
}

signed int Get_ACPI_CPPC_Registers(unsigned int cpu, void *arg)
{
#ifdef CONFIG_ACPI_CPPC_LIB
	struct cppc_perf_fb_ctrs CPPC_Perf;
	struct cppc_perf_caps CPPC_Caps;

	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	signed int rc = 0;
	UNUSED(arg);

	if ((rc = cppc_get_perf_ctrs(Core->Bind, &CPPC_Perf)) == 0) {
	    if ((rc = cppc_get_perf_caps(Core->Bind, &CPPC_Caps)) != 0)
		pr_debug("CoreFreq: cppc_get_perf_caps(cpu=%u) error %d\n",
			Core->Bind, rc);
	} else {
		pr_debug("CoreFreq: cppc_get_perf_ctrs(cpu=%u) error %d\n",
			Core->Bind, rc);
	}
	if (rc == 0) {
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
		unsigned long long desired_perf = 0;
	    #endif
		Core->PowerThermal.ACPI_CPPC = (struct ACPI_CPPC_STRUCT) {
			.Highest	= CPPC_Caps.highest_perf,
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
			.Guaranteed	= CPPC_Caps.guaranteed_perf == 0 ?
						CPPC_Caps.nominal_perf
					:	CPPC_Caps.guaranteed_perf,
			#else
			.Guaranteed	= CPPC_Caps.nominal_perf,
			#endif
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
			.Efficient	= CPPC_Caps.nominal_freq,
			.Lowest 	= CPPC_Caps.lowest_freq,
			.Minimum	= CPPC_Caps.lowest_freq,
			#else
			.Efficient	= CPPC_Caps.nominal_perf,
			.Lowest 	= CPPC_Caps.lowest_perf,
			.Minimum	= CPPC_Caps.lowest_perf,
			#endif
			.Maximum	= CPPC_Caps.highest_perf,
			.Desired	= CPPC_Perf.reference_perf,
			.Energy 	= 0
		};
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
		#if (defined(CONFIG_SCHED_BORE) || defined(CONFIG_CACHY)) \
		 && (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		rc = cppc_get_desired_perf(Core->Bind, &desired_perf);
		rc = rc == -EINVAL ? 0 : rc;
		#else
		rc = cppc_get_desired_perf(Core->Bind, &desired_perf);
		#endif
	    if (rc == 0) {
		Core->PowerThermal.ACPI_CPPC.Desired = desired_perf;
	    } else {
		 pr_debug("CoreFreq: cppc_get_desired_perf(cpu=%u) error %d\n",
			Core->Bind, rc);
	    }
	    #endif
	}
	return rc;
#else
	return -ENODEV;
#endif /* CONFIG_ACPI_CPPC_LIB */
}

signed int Get_EPP_ACPI_CPPC(unsigned int cpu)
{
	signed int rc = -ENODEV;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	u64 epp_perf;

	if ((rc = cppc_get_epp_perf((signed int) cpu, &epp_perf)) == 0)
	{
		CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

		Core->PowerThermal.ACPI_CPPC.Energy = epp_perf;
	} else {
	    pr_debug("CoreFreq: cppc_get_epp_perf(cpu=%u) error %d\n", cpu, rc);
	}
#endif
	return rc;
}

signed int Put_EPP_ACPI_CPPC(unsigned int cpu, signed short epp)
{
	signed int rc = -ENODEV;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	struct cppc_perf_ctrls perf_ctrls = {
		.max_perf = 0,
		.min_perf = 0,
		.desired_perf = 0,
		.energy_perf = epp
	};
	if ((rc = cppc_set_epp_perf((signed int) cpu, &perf_ctrls, true)) < 0) {
	    pr_debug("CoreFreq: cppc_set_epp_perf(cpu=%u) error %d\n", cpu, rc);
	}
#endif
	return rc;
}

signed int Set_EPP_ACPI_CPPC(unsigned int cpu, void *arg)
{
	signed int rc = 0;
	UNUSED(arg);

	if ((HWP_EPP >= 0) && (HWP_EPP <= 0xff)) {
		if ((rc = Put_EPP_ACPI_CPPC(cpu, HWP_EPP)) == 0) {
			rc = Get_EPP_ACPI_CPPC(cpu);
		}
	}
	return rc;
}

signed int Read_ACPI_CPPC_Registers(unsigned int cpu, void *arg)
{
	signed int rc = Get_ACPI_CPPC_Registers(cpu, arg);

	Compute_ACPI_CPPC_Bounds(cpu);

	if (Get_EPP_ACPI_CPPC(cpu) == 0) {
		PUBLIC(RO(Proc))->Features.OSPM_EPP = 1;
	}
	Set_EPP_ACPI_CPPC(cpu, arg);

	return rc;
}

void For_All_ACPI_CPPC(signed int(*CPPC_Func)(unsigned int, void*), void *arg)
{
	#if defined(CONFIG_ACPI_CPPC_LIB) \
	 && LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
	signed int rc = acpi_cpc_valid() == false;
	#elif defined(CONFIG_ACPI)
	signed int rc = acpi_disabled;
	#else
	signed int rc = false;
	#endif
	unsigned int cpu;

	PUBLIC(RO(Proc))->Features.OSPM_CPC = !rc;
	PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Minimum = 255U;
	PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum = 1U;

	for (cpu = 0; (cpu < PUBLIC(RO(Proc))->CPU.Count) && (rc == 0); cpu++)
	{
		if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)) {
			rc = CPPC_Func(cpu, arg);
		}
	}
	PUBLIC(RO(Proc))->Features.ACPI_CPPC = (rc == 0);
}

static void CoreFreqK_ResetChip(struct pci_dev *dev)
{
	UNUSED(dev);

	memset( PUBLIC(RO(Proc))->Uncore.Chip, 0,
		CHIP_MAX_PCI*sizeof(struct CHIP_ST) );
}

static void CoreFreqK_AppendChip(struct pci_dev *dev)
{
	unsigned int idx;
	for (idx = 0; idx < CHIP_MAX_PCI; idx++)
	{
		if (PUBLIC(RO(Proc))->Uncore.Chip[idx].VID == 0)
		{
			PUBLIC(RO(Proc))->Uncore.Chip[idx].VID = dev->vendor;
			PUBLIC(RO(Proc))->Uncore.Chip[idx].DID = dev->device;

			break;
		}
	}
}

static int CoreFreqK_ProbePCI(	struct pci_device_id PCI_ids[],
				void (*PreProbe)(struct pci_dev*),
				void (*PostProbe)(struct pci_dev*) )
{
	struct pci_device_id *id = PCI_ids;
	struct pci_dev *dev = NULL;
	int rc = -ENODEV;

	if (PreProbe != NULL) {
		PreProbe(dev);
	}
	while (id->vendor || id->subvendor || id->class_mask)
	{
		dev = pci_get_device(id->vendor, id->device, NULL);
	  if (dev != NULL) {
	    if (!pci_enable_device(dev))
	    {
		PCI_CALLBACK Callback = (PCI_CALLBACK) id->driver_data;

		if ((rc = (int) Callback(dev)) == 0)
		{
			if (PostProbe != NULL) {
				PostProbe(dev);
			}
		}
		pci_disable_device(dev);
	    }
	    pci_dev_put(dev);
	  }
		id++;
	}
	return rc;
}

void Query_Same_Genuine_Features(void)
{
	if ((PRIVATE(OF(Specific)) = LookupProcessor()) != NULL)
	{
		OverrideCodeNameString(PRIVATE(OF(Specific)));
		OverrideUnlockCapability(PRIVATE(OF(Specific)));
	}
	Default_Unlock_Reset();

	if (PUBLIC(RO(Proc))->Features.Turbo_Unlock)
	{
		PUBLIC(RO(Proc))->Features.SpecTurboRatio = 1;
	} else {
		PUBLIC(RO(Proc))->Features.SpecTurboRatio = 0;
	}
}

static void Query_GenericMachine(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	/* Reset Max ratio to call the clock estimation in Controller_Init() */
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = 0;
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = 8;

    if (PRIVATE(OF(Specific)) != NULL) {
	/*	Save the thermal parameters if specified		*/
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	HyperThreading_Technology();
}

void SystemRegisters(CORE_RO *Core)
{
	volatile AA64ISAR2 isar2;
	volatile AA64MMFR1 mmfr1;
	volatile AA64PFR0 pfr0;

	isar2.value = read_sysreg_s(ID_AA64ISAR2_EL1);

	__asm__ __volatile__(
		"mrs	%[mmfr1],	id_aa64mmfr1_el1""\n\t"
		"mrs	%[pfr0] ,	id_aa64pfr0_el1""\n\t"
		"cmp	xzr	,	xzr, lsl #0"	"\n\t"
		"mrs	x14	,	nzcv"		"\n\t"
		"mrs	x13	,	daif"		"\n\t"
		"mrs	x12	,	currentel"	"\n\t"
		"isb"					"\n\t"
		"mov	%[flags],	xzr"		"\n\t"
		"orr	%[flags],	x14, x13"	"\n\t"
		"orr	%[flags],	%[flags], x12"
		: [mmfr1]	"=r" (mmfr1),
		  [pfr0]	"=r" (pfr0),
		  [flags]	"=r" (Core->SystemRegister.FLAGS)
		:
		: "cc", "memory", "%x12", "%x13", "%x14"
	);
	if (mmfr1.VH) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->VM, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->VM, Core->Bind);
	}
	if (isar2.CLRBHB == 0b0001) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CLRBHB, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CLRBHB, Core->Bind);
	}
	switch (pfr0.CSV2) {
	case 0b0001:
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_1, Core->Bind);
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_2, Core->Bind);
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_3, Core->Bind);
		break;
	case 0b0010:
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_1, Core->Bind);
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_2, Core->Bind);
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_3, Core->Bind);
		break;
	case 0b0011:
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_1, Core->Bind);
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_2, Core->Bind);
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_3, Core->Bind);
		break;
	default:
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_1, Core->Bind);
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_2, Core->Bind);
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_3, Core->Bind);
	}
	if (pfr0.CSV3) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV3, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV3, Core->Bind);
	}
	if (PUBLIC(RO(Proc))->Features.SSBS == 0b0010)
	{
		SSBS2 mrs_ssbs = {.value = read_sysreg_s(MRS_SSBS2)};

	    if (mrs_ssbs.SSBS) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBS, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBS, Core->Bind);
	    }
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CR_Mask, Core->Bind);
}

#define Pkg_Reset_ThermalPoint(Pkg)					\
({									\
	BITWISECLR(LOCKLESS, Pkg->ThermalPoint.Mask);			\
	BITWISECLR(LOCKLESS, Pkg->ThermalPoint.Kind);			\
	BITWISECLR(LOCKLESS, Pkg->ThermalPoint.State);			\
})

void PerCore_Reset(CORE_RO *Core)
{
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->HWP_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->CR_Mask	, Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->HWP	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_1	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_2	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV2_3	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CSV3	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBS	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->VM	, Core->Bind);

	BITWISECLR(LOCKLESS, Core->ThermalPoint.Mask);
	BITWISECLR(LOCKLESS, Core->ThermalPoint.Kind);
	BITWISECLR(LOCKLESS, Core->ThermalPoint.State);
}

static void PerCore_GenericMachine(void *arg)
{
	volatile CPUPWRCTLR cpupwrctl;
	CORE_RO *Core = (CORE_RO *) arg;

	PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MIN)] = 8;
	PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MAX)] = \
				PUBLIC(RO(Proc))->Features.Factory.Ratio;

    if (Experimental && (PUBLIC(RO(Proc))->HypervisorID == HYPERV_NONE)) {
	cpupwrctl.value = read_sysreg_s(CPUPWRCTLR_EL1);
	Core->Query.CStateBaseAddr = cpupwrctl.WFI_RET_CTRL;
    }

	SystemRegisters(Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 56)
void Sys_DumpTask(SYSGATE_RO *SysGate)
{
	SysGate->taskCount = 0;
}
#else /* KERNEL_VERSION(3, 10, 56) */
void Sys_DumpTask(SYSGATE_RO *SysGate)
{	/* Source: /include/linux/sched.h */
	struct task_struct *process, *thread;
	int cnt = 0;

	rcu_read_lock();
	for_each_process_thread(process, thread) {
	    if (cnt < TASK_LIMIT) {
#if defined(CONFIG_SCHED_MUQSS) \
 || defined(CONFIG_SCHED_BMQ) \
 || defined(CONFIG_SCHED_PDS)
		SysGate->taskList[cnt].runtime  = tsk_seruntime(thread);
#else
		SysGate->taskList[cnt].runtime  = thread->se.sum_exec_runtime;
#endif /* CONFIG_SCHED_*	*/
		SysGate->taskList[cnt].usertime = thread->utime;
		SysGate->taskList[cnt].systime  = thread->stime;
		SysGate->taskList[cnt].pid      = thread->pid;
		SysGate->taskList[cnt].tgid     = thread->tgid;
		SysGate->taskList[cnt].ppid     = thread->parent->pid;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
		SysGate->taskList[cnt].state    = (short int) thread->state;
#else
		SysGate->taskList[cnt].state    = (short int) thread->__state;
#endif
#if defined(CONFIG_SCHED_ALT)
		SysGate->taskList[cnt].wake_cpu = (short int) task_cpu(thread);
#elif defined(CONFIG_SCHED_BMQ) || defined(CONFIG_SCHED_PDS)
		SysGate->taskList[cnt].wake_cpu = (short int) thread->cpu;
#else
		SysGate->taskList[cnt].wake_cpu = (short int) thread->wake_cpu;
#endif /* CONFIG_SCHED_BMQ	*/
		memcpy(SysGate->taskList[cnt].comm, thread->comm,TASK_COMM_LEN);

		cnt++;
	    }
	}
	rcu_read_unlock();
	SysGate->taskCount = cnt;
}
#endif /* KERNEL_VERSION(3, 10, 56) */

#define Sys_Tick(Pkg, ...)					\
({								\
	if (PUBLIC(OF(Gate)) != NULL)				\
	{							\
		Pkg->tickStep--;				\
		if (!Pkg->tickStep) {				\
			Pkg->tickStep = Pkg->tickReset ;	\
			Sys_DumpTask( PUBLIC(OF(Gate)) );	\
			__VA_ARGS__				\
		}						\
	}							\
})

static void InitTimer(void *Cycle_Function)
{
	unsigned int cpu = smp_processor_id();

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, CREATED) == 0)
    {
	hrtimer_init(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			CLOCK_MONOTONIC,
			HRTIMER_MODE_REL_PINNED);

	PRIVATE(OF(Core, AT(cpu)))->Join.Timer.function = Cycle_Function;
	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, CREATED);
    }
}

void Controller_Init(void)
{
	CLOCK sClock = {.Q = 0, .R = 0, .Hz = 0};
	unsigned int cpu = PUBLIC(RO(Proc))->CPU.Count, ratio = 0;

	Package_Init_Reset();

    if (Arch[PUBLIC(RO(Proc))->ArchID].Query != NULL)
    {
	Arch[PUBLIC(RO(Proc))->ArchID].Query(PUBLIC(RO(Proc))->Service.Core);
    }

   ratio=PUBLIC(RO(Core,AT(PUBLIC(RO(Proc))->Service.Core)))->Boost[BOOST(MAX)];

    if (Arch[PUBLIC(RO(Proc))->ArchID].BaseClock != NULL)
    {
	sClock = Arch[PUBLIC(RO(Proc))->ArchID].BaseClock(ratio);
    }
    if (sClock.Hz == 0) {	/*	Fallback to 100 MHz		*/
	sClock.Q = 100;
	sClock.R = 0;
	sClock.Hz = 100000000LLU;
    }
	ratio = FixMissingRatioAndFrequency(ratio, &sClock);

  if ((AutoClock & 0b01) || PUBLIC(RO(Proc))->Features.Std.ECX.Hyperv)
  {
	CLOCK vClock = {.Q = 0, .R =0, .Hz = 0};
	COMPUTE_ARG Compute;
	struct kmem_cache *hwCache = NULL;
	/* Allocate Cache aligned resources. */
	hwCache = kmem_cache_create(	"CoreFreqCache",
					STRUCT_SIZE, 0,
					SLAB_HWCACHE_ALIGN, NULL);
    if (hwCache != NULL)
    {
      do {	/*		from last AP to BSP			*/
		cpu--;

	if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS))
	{
		Compute.TSC[0] = kmem_cache_alloc(hwCache, GFP_ATOMIC);
	  if (Compute.TSC[0] != NULL)
	  {
		Compute.TSC[1] = kmem_cache_alloc(hwCache, GFP_ATOMIC);
	    if (Compute.TSC[1] != NULL)
	    {
		if (ratio != 0)
		{
		Compute.Clock.Q = ratio;
		Compute.Clock.R = 0;
		Compute.Clock.Hz = 0;

		PUBLIC(RO(Core, AT(cpu)))->Clock = Compute_Clock(cpu,&Compute);

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = ratio;
		}
		else
		{
		vClock.Q = 0; vClock.R = 0; vClock.Hz = 0;
		Compute.Clock.Q = sClock.Q;
		Compute.Clock.R = 0;
		Compute.Clock.Hz = 0;

		vClock = Compute_Clock(cpu, &Compute);

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = vClock.Q;
		}
		/*		Release memory resources.		*/
		kmem_cache_free(hwCache, Compute.TSC[1]);
	    }
		kmem_cache_free(hwCache, Compute.TSC[0]);
	  }
	}
      } while (cpu != 0) ;

	kmem_cache_destroy(hwCache);
    }
	if ((ratio == 0) && (vClock.Hz != 0)) {
		ratio = FixMissingRatioAndFrequency(vClock.Q, &sClock);
	}
  }
	/*	Launch a high resolution timer per online CPU.		*/
	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
	{
		if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)) {
			if (!PUBLIC(RO(Core, AT(cpu)))->Clock.Hz)
			{
				PUBLIC(RO(Core, AT(cpu)))->Clock = sClock;
			}
			if (Arch[PUBLIC(RO(Proc))->ArchID].Timer != NULL)
			{
				Arch[PUBLIC(RO(Proc))->ArchID].Timer(cpu);
			}
		}
	}
}

void Controller_Start(int wait)
{
    if (Arch[PUBLIC(RO(Proc))->ArchID].Start != NULL)
    {
	unsigned int cpu;
      for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
      {
	if ((BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, CREATED) == 1)
	 && (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED) == 0))
	{
		smp_call_function_single(cpu,
					Arch[PUBLIC(RO(Proc))->ArchID].Start,
					NULL, wait);
	}
      }
    }
}

void Controller_Stop(int wait)
{
    if (Arch[PUBLIC(RO(Proc))->ArchID].Stop != NULL)
    {
	unsigned int cpu;
	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
	    if ((BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, CREATED) == 1)
	     && (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED) == 1))
	    {
		smp_call_function_single(cpu,
					Arch[PUBLIC(RO(Proc))->ArchID].Stop,
					NULL, wait);
	    }
    }
}

void Controller_Exit(void)
{
	unsigned int cpu;

	if (Arch[PUBLIC(RO(Proc))->ArchID].Exit != NULL)
	{
		Arch[PUBLIC(RO(Proc))->ArchID].Exit();
	}
	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
	{
		BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, CREATED);
	}
}

void Generic_Core_Counters_Set(union SAVE_AREA_CORE *Save, CORE_RO *Core)
{
	__asm__ __volatile__
	(
		"# Assign an event number per counter"	"\n\t"
		"mrs	x12	,	pmselr_el0"	"\n\t"
		"str	x12	,	%[PMSELR]"	"\n\t"
		"orr	x12	,	x12, #3"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"

		"# Choosen [EVENT#] to collect from"	"\n\t"
		"mrs	x12	,	pmxevtyper_el0" "\n\t"
		"str	x12	,	%[PMTYPE3]"	"\n\t"
		"orr	x12	,	x12, %[EVENT3]" "\n\t"
		"msr	pmxevtyper_el0, x12"		"\n\t"

		"ldr	x12	,	%[PMSELR]"	"\n\t"
		"orr	x12	,	x12, #2"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"
		"mrs	x12	,	pmxevtyper_el0" "\n\t"
		"str	x12	,	%[PMTYPE2]"	"\n\t"
		"orr	x12	,	x12, %[EVENT2]" "\n\t"
		"msr	pmxevtyper_el0, x12"		"\n\t"

		"ldr	x12	,	%[PMSELR]"	"\n\t"
		"orr	x12	,	x12, #0b11111"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"
		"mrs	x12	,	pmxevtyper_el0" "\n\t"
		"str	x12	,	%[PMTYPE1]"	"\n\t"
		"orr	x12	,	x12, %[FILTR1]" "\n\t"
		"msr	pmxevtyper_el0, x12"		"\n\t"

		"# No filtered EL within Cycle counter" "\n\t"
		"mrs	x12	,	pmccfiltr_el0"	"\n\t"
		"str	x12	,	%[PMCCFILTR]"	"\n\t"
		"msr	pmccfiltr_el0,	xzr"		"\n\t"

		"# Enable counters at position [ENSET]" "\n\t"
		"mrs	x12	,	pmcntenset_el0" "\n\t"
		"str	x12	,	%[PMCNTEN]"	"\n\t"
		"orr	x12	,	x12, %[ENSET]"	"\n\t"
		"msr	pmcntenset_el0, x12"		"\n\t"

		"# Enable all PMU counters"		"\n\t"
		"mrs	x12	,	pmcr_el0"	"\n\t"
		"str	x12	,	%[PMCR]"	"\n\t"
		"mov	x12	,	%[CTRL]"	"\n\t"
		"msr	pmcr_el0,	x12"		"\n\t"
		"isb"
		: [PMCR]	"+m" (Save->PMCR),
		  [PMSELR]	"+m" (Save->PMSELR),
		  [PMTYPE3]	"+m" (Save->PMTYPE[2]),
		  [PMTYPE2]	"+m" (Save->PMTYPE[1]),
		  [PMTYPE1]	"+m" (Save->PMTYPE[0]),
		  [PMCCFILTR]	"+m" (Save->PMCCFILTR),
		  [PMCNTEN]	"+m" (Save->PMCNTEN)
		: [EVENT3]	"r" (0x0008),
		  [EVENT2]	"r" (0x0011),
		  [FILTR1]	"r" (0x0),
		  [ENSET]	"r" (0b10000000000000000000000000001100),
		  [CTRL]	"i" (0b0000000010000111)
		: "memory", "%x12"
	);
}

void Generic_Core_Counters_Clear(union SAVE_AREA_CORE *Save, CORE_RO *Core)
{
	__asm__ __volatile__(
		"# Restore PMU configuration registers" "\n\t"
		"ldr	x12	,	%[PMCR]"	"\n\t"
		"msr	pmcr_el0,	x12"		"\n\t"

		"ldr	x12	,	%[PMCNTEN]"	"\n\t"
		"msr	pmcntenset_el0, x12"		"\n\t"

		"ldr	x12	,	%[PMCCFILTR]"	"\n\t"
		"msr	pmccfiltr_el0,	x12"		"\n\t"

		"ldr	x12	,	%[PMSELR]"	"\n\t"
		"orr	x12	,	x12, #0b11111"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"
		"ldr	x12	,	%[PMTYPE1]"	"\n\t"
		"msr	pmxevtyper_el0, x12"		"\n\t"

		"ldr	x12	,	%[PMSELR]"	"\n\t"
		"orr	x12	,	x12, #2"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"
		"ldr	x12	,	%[PMTYPE2]"	"\n\t"
		"msr	pmxevtyper_el0, x12"		"\n\t"

		"ldr	x12	,	%[PMSELR]"	"\n\t"
		"orr	x12	,	x12, #3"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"
		"ldr	x12	,	%[PMTYPE3]"	"\n\t"
		"msr	pmxevtyper_el0, x12"		"\n\t"

		"ldr	x12	,	%[PMSELR]"	"\n\t"
		"msr	pmselr_el0,	x12"		"\n\t"

		"isb"
		:
		: [PMCR]	"m" (Save->PMCR),
		  [PMSELR]	"m" (Save->PMSELR),
		  [PMTYPE3]	"m" (Save->PMTYPE[2]),
		  [PMTYPE2]	"m" (Save->PMTYPE[1]),
		  [PMTYPE1]	"m" (Save->PMTYPE[0]),
		  [PMCCFILTR]	"m" (Save->PMCCFILTR),
		  [PMCNTEN]	"m" (Save->PMCNTEN)
		: "memory", "%x12"
	);
}

#define Counters_Generic(Core, T)					\
({									\
	volatile unsigned long long UCC, URC;				\
	RDTSC_COUNTERx3(Core->Counter[T].TSC,				\
			pmevcntr2_el0,	UCC,				\
			pmccntr_el0,	URC,				\
			pmevcntr3_el0,	Core->Counter[T].INST );	\
	/* Normalize Frequencies */					\
	Core->Counter[T].C0.UCC = DIV_ROUND_CLOSEST(UCC, PRECISION);	\
	Core->Counter[T].C0.URC = DIV_ROUND_CLOSEST(URC, PRECISION);	\
	/* Derive C1: */						\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Mark_OVH(Core)							\
({									\
	RDTSCP64(Core->Overhead.TSC);					\
})

#define Core_OVH(Core)							\
({									\
	Core->Delta.TSC -= (Core->Counter[1].TSC - Core->Overhead.TSC); \
})

#define Delta_TSC(Core) 						\
({									\
	Core->Delta.TSC = Core->Counter[1].TSC				\
			- Core->Counter[0].TSC; 			\
})

#define Delta_TSC_OVH(Core)						\
({									\
	Delta_TSC(Core);						\
									\
	if (AutoClock & 0b10) { 					\
		Core_OVH(Core); 					\
									\
		REL_BCLK(Core->Clock,					\
			Core->Boost[BOOST(MAX)],			\
			Core->Delta.TSC,				\
			PUBLIC(RO(Proc))->SleepInterval);		\
	}								\
})

#define Delta_C0(Core)							\
({	/* Absolute Delta of Unhalted (Core & Ref) C0 Counter. */	\
	Core->Delta.C0.UCC = (						\
		Core->Counter[0].C0.UCC > Core->Counter[1].C0.UCC	\
	)	? Core->Counter[0].C0.UCC - Core->Counter[1].C0.UCC	\
		: Core->Counter[1].C0.UCC - Core->Counter[0].C0.UCC;	\
									\
	Core->Delta.C0.URC = (						\
		Core->Counter[0].C0.URC > Core->Counter[1].C0.URC	\
	)	? Core->Counter[0].C0.URC - Core->Counter[1].C0.URC	\
		: Core->Counter[1].C0.URC - Core->Counter[0].C0.URC;	\
})

#define Delta_C1(Core)							\
({									\
	Core->Delta.C1 = (Core->Counter[0].C1 > Core->Counter[1].C1) ? 	\
			  Core->Counter[0].C1 - Core->Counter[1].C1	\
			: Core->Counter[1].C1 - Core->Counter[0].C1;	\
})

#define Delta_C3(Core)							\
({									\
	Core->Delta.C3  = Core->Counter[1].C3				\
			- Core->Counter[0].C3;				\
})

#define Delta_C6(Core)							\
({									\
	Core->Delta.C6  = Core->Counter[1].C6				\
			- Core->Counter[0].C6;				\
})

#define Delta_C7(Core)							\
({									\
	Core->Delta.C7  = Core->Counter[1].C7				\
			- Core->Counter[0].C7;				\
})

#define Delta_INST(Core)						\
({	/* Delta of Retired Instructions */				\
	Core->Delta.INST = (						\
		Core->Counter[0].INST > Core->Counter[1].INST		\
	)	? Core->Counter[0].INST - Core->Counter[1].INST 	\
		: Core->Counter[1].INST - Core->Counter[0].INST;	\
})

#define PKG_Counters_Generic(Core, T)					\
({									\
	volatile CNTPCT cntpct; 					\
	__asm__ volatile						\
	(								\
		"mrs	%[cntpct],	cntpct_el0"	"\n\t"		\
		"isb"							\
		: [cntpct]	"=r" (cntpct) 				\
		:							\
		: "cc", "memory"					\
	);								\
	PUBLIC(RO(Proc))->Counter[T].PCLK = cntpct.PhysicalCount;	\
})

#define Pkg_OVH(Pkg, Core)						\
({									\
	Pkg->Delta.PCLK -= (Pkg->Counter[1].PCLK - Core->Overhead.TSC); \
})

#define Delta_PTSC(Pkg) 						\
({									\
	Pkg->Delta.PCLK = Pkg->Counter[1].PCLK				\
			- Pkg->Counter[0].PCLK; 			\
})

#define Delta_PTSC_OVH(Pkg, Core)					\
({									\
	Delta_PTSC(Pkg);						\
									\
	if (AutoClock & 0b10)						\
		Pkg_OVH(Pkg, Core);					\
})

#define Delta_PC02(Pkg) 						\
({									\
	Pkg->Delta.PC02 = Pkg->Counter[1].PC02				\
			- Pkg->Counter[0].PC02; 			\
})

#define Delta_PC03(Pkg) 						\
({									\
	Pkg->Delta.PC03 = Pkg->Counter[1].PC03				\
			- Pkg->Counter[0].PC03; 			\
})

#define Delta_PC04(Pkg) 						\
({									\
	Pkg->Delta.PC04 = Pkg->Counter[1].PC04				\
			- Pkg->Counter[0].PC04; 			\
})

#define Delta_PC06(Pkg) 						\
({									\
	Pkg->Delta.PC06 = Pkg->Counter[1].PC06				\
			- Pkg->Counter[0].PC06; 			\
})

#define Delta_PC07(Pkg) 						\
({									\
	Pkg->Delta.PC07 = Pkg->Counter[1].PC07				\
			- Pkg->Counter[0].PC07; 			\
})

#define Delta_PC08(Pkg) 						\
({									\
	Pkg->Delta.PC08 = Pkg->Counter[1].PC08				\
			- Pkg->Counter[0].PC08; 			\
})

#define Delta_PC09(Pkg) 						\
({									\
	Pkg->Delta.PC09 = Pkg->Counter[1].PC09				\
			- Pkg->Counter[0].PC09; 			\
})

#define Delta_PC10(Pkg) 						\
({									\
	Pkg->Delta.PC10 = Pkg->Counter[1].PC10				\
			- Pkg->Counter[0].PC10; 			\
})

#define Delta_MC6(Pkg)							\
({									\
	Pkg->Delta.MC6	= Pkg->Counter[1].MC6				\
			- Pkg->Counter[0].MC6;				\
})

#define Delta_UNCORE_FC0(Pkg)						\
({									\
	Pkg->Delta.Uncore.FC0 =						\
		(Pkg->Counter[0].Uncore.FC0 >				\
		Pkg->Counter[1].Uncore.FC0) ?				\
			Pkg->Counter[0].Uncore.FC0			\
			- Pkg->Counter[1].Uncore.FC0			\
			: Pkg->Counter[1].Uncore.FC0			\
			- Pkg->Counter[0].Uncore.FC0;			\
})

#define Save_TSC(Core)							\
({	/* Save Time Stamp Counter. */					\
	Core->Counter[0].TSC = Core->Counter[1].TSC;			\
})

#define Save_C0(Core)							\
({	/* Save the Unhalted Core & Reference Counter */		\
	Core->Counter[0].C0.UCC = Core->Counter[1].C0.UCC;		\
	Core->Counter[0].C0.URC = Core->Counter[1].C0.URC;		\
})

#define Save_C1(Core)							\
({									\
	Core->Counter[0].C1 = Core->Counter[1].C1;			\
})

#define Save_C3(Core)							\
({									\
	Core->Counter[0].C3 = Core->Counter[1].C3;			\
})

#define Save_C6(Core)							\
({									\
	Core->Counter[0].C6 = Core->Counter[1].C6;			\
})

#define Save_C7(Core)							\
({									\
	Core->Counter[0].C7 = Core->Counter[1].C7;			\
})

#define Save_INST(Core) 						\
({	/* Save the Instructions counter. */				\
	Core->Counter[0].INST = Core->Counter[1].INST;			\
})

#define Save_PTSC(Pkg)							\
({									\
	Pkg->Counter[0].PCLK = Pkg->Counter[1].PCLK;			\
})

#define Save_PC02(Pkg)							\
({									\
	Pkg->Counter[0].PC02 = Pkg->Counter[1].PC02;			\
})

#define Save_PC03(Pkg)							\
({									\
	Pkg->Counter[0].PC03 = Pkg->Counter[1].PC03;			\
})

#define Save_PC04(Pkg)							\
({									\
	Pkg->Counter[0].PC04 = Pkg->Counter[1].PC04;			\
})

#define Save_PC06(Pkg)							\
({									\
	Pkg->Counter[0].PC06 = Pkg->Counter[1].PC06;			\
})

#define Save_PC07(Pkg)							\
({									\
	Pkg->Counter[0].PC07 = Pkg->Counter[1].PC07;			\
})

#define Save_PC08(Pkg)							\
({									\
	Pkg->Counter[0].PC08 = Pkg->Counter[1].PC08;			\
})

#define Save_PC09(Pkg)							\
({									\
	Pkg->Counter[0].PC09 = Pkg->Counter[1].PC09;			\
})

#define Save_PC10(Pkg)							\
({									\
	Pkg->Counter[0].PC10 = Pkg->Counter[1].PC10;			\
})

#define Save_MC6(Pkg)							\
({									\
	Pkg->Counter[0].MC6 = Pkg->Counter[1].MC6;			\
})

#define Save_UNCORE_FC0(Pkg)						\
({									\
	Pkg->Counter[0].Uncore.FC0 = Pkg->Counter[1].Uncore.FC0;	\
})

static enum hrtimer_restart Cycle_GenericMachine(struct hrtimer *pTimer)
{
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	RDTSC64(Core->Overhead.TSC);

	if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

			Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

			Save_PTSC(PUBLIC(RO(Proc)));

			Sys_Tick(PUBLIC(RO(Proc)));
		}

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		return HRTIMER_RESTART;
	} else
		return HRTIMER_NORESTART;
}

static void InitTimer_GenericMachine(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_GenericMachine, 1);
}

static void Start_GenericMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Generic_Core_Counters_Set(Save, Core);

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_GenericMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Generic_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

long Sys_OS_Driver_Query(void)
{
	int rc = RC_SUCCESS;
#ifdef CONFIG_CPU_FREQ
	const char *pFreqDriver;
	struct cpufreq_policy freqPolicy;
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_CPU_IDLE
	struct cpuidle_driver *idleDriver;
#endif /* CONFIG_CPU_IDLE */
	memset(&PUBLIC(RO(Proc))->OS, 0, sizeof(OS_DRIVER));
#ifdef CONFIG_CPU_IDLE
    if ((idleDriver = cpuidle_get_driver()) != NULL)
    {
	int idx;

	StrCopy(PUBLIC(RO(Proc))->OS.IdleDriver.Name,
		idleDriver->name,
		CPUIDLE_NAME_LEN);

      if (idleDriver->state_count < CPUIDLE_STATE_MAX) {
	PUBLIC(RO(Proc))->OS.IdleDriver.stateCount = idleDriver->state_count;
      } else {
	PUBLIC(RO(Proc))->OS.IdleDriver.stateCount = CPUIDLE_STATE_MAX;
      }
	PUBLIC(RO(Proc))->OS.IdleDriver.stateLimit = idleDriver->state_count;

      for (idx = 0; idx < PUBLIC(RO(Proc))->OS.IdleDriver.stateCount; idx++)
      {
	StrCopy(PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].Name,
		idleDriver->states[idx].name, CPUIDLE_NAME_LEN);

	StrCopy(PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].Desc,
		idleDriver->states[idx].desc, CPUIDLE_NAME_LEN);

	PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].exitLatency = \
					idleDriver->states[idx].exit_latency;

	PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].powerUsage = \
					idleDriver->states[idx].power_usage;

	PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].targetResidency = \
				idleDriver->states[idx].target_residency;
      }
      if(PUBLIC(RO(Proc))->Registration.Driver.CPUidle == REGISTRATION_ENABLE)
      {
	for (idx = SUB_CSTATE_COUNT - 1; idx >= 0 ; idx--)
	{
	    if (CoreFreqK.SubCstate[idx] > 0)
	    {
		PUBLIC(RO(Proc))->OS.IdleDriver.stateLimit = 1 + idx;
		break;
	    }
	}
      }
    }
#endif /* CONFIG_CPU_IDLE */
#ifdef CONFIG_CPU_FREQ
	if ((pFreqDriver = cpufreq_get_current_driver()) != NULL) {
		StrCopy(PUBLIC(RO(Proc))->OS.FreqDriver.Name,
			pFreqDriver, CPUFREQ_NAME_LEN);
	}
	memset(&freqPolicy, 0, sizeof(freqPolicy));
    if((rc=cpufreq_get_policy(&freqPolicy, PUBLIC(RO(Proc))->Service.Core))==0)
    {
	struct cpufreq_governor *pGovernor = freqPolicy.governor;
	if (pGovernor != NULL) {
		StrCopy(PUBLIC(RO(Proc))->OS.FreqDriver.Governor,
			pGovernor->name, CPUFREQ_NAME_LEN);
	} else {
		PUBLIC(RO(Proc))->OS.FreqDriver.Governor[0] = '\0';
	}
    } else {
	PUBLIC(RO(Proc))->OS.FreqDriver.Governor[0] = '\0';
    }
#endif /* CONFIG_CPU_FREQ */
	return rc;
}

long Sys_Kernel(SYSGATE_RO *SysGate)
{	/* Sources:	/include/generated/uapi/linux/version.h
			/include/uapi/linux/utsname.h			*/
	if (SysGate != NULL) {
		SysGate->kernelVersionNumber = LINUX_VERSION_CODE;
		memcpy(SysGate->sysname, utsname()->sysname, MAX_UTS_LEN);
		memcpy(SysGate->release, utsname()->release, MAX_UTS_LEN);
		memcpy(SysGate->version, utsname()->version, MAX_UTS_LEN);
		memcpy(SysGate->machine, utsname()->machine, MAX_UTS_LEN);

		return RC_OK_SYSGATE;
	} else {
		return -ENXIO;
	}
}

long SysGate_OnDemand(void)
{
	long rc = -1;
    if (PUBLIC(OF(Gate)) == NULL)
    {	/*			On-demand allocation.			*/
	PUBLIC(OF(Gate)) = alloc_pages_exact(PUBLIC(RO(Proc))->Gate.ReqMem.Size,
						GFP_KERNEL);
	if (PUBLIC(OF(Gate)) != NULL)
	{
		const size_t
		allocPages = PAGE_SIZE << PUBLIC(RO(Proc))->Gate.ReqMem.Order;
		memset(PUBLIC(OF(Gate)), 0, allocPages);
		rc = 0;
	}
    } else {					/* Already allocated	*/
		rc = 1;
    }
	return rc;
}

#define Atomic_Write_VPMC( _Core, cycles, _lvl)				\
{									\
	switch (_lvl) {							\
	case 0:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C1, cycles);	\
		break;							\
	case 1:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C2, cycles);	\
		break;							\
	case 2:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C3, cycles);	\
		break;							\
	case 3:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C4, cycles);	\
		break;							\
	case 4:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C5, cycles);	\
		break;							\
	case 5:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C6, cycles);	\
		break;							\
	case 6:								\
		Atomic_Add_VPMC(LOCKLESS, _Core->VPMC.C7, cycles);	\
		break;							\
	};								\
}

#undef Atomic_Write_VPMC

static void CoreFreqK_IdleDriver_UnInit(void)
{
#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	struct cpuidle_device *device;
	unsigned int cpu;

	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++) {
	    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, HW)) {
		if ((device=per_cpu_ptr(CoreFreqK.IdleDevice, cpu)) != NULL)
		{
			cpuidle_unregister_device(device);
		}
	    }
	}
	cpuidle_unregister_driver(&CoreFreqK.IdleDriver);
	free_percpu(CoreFreqK.IdleDevice);
#endif /* CONFIG_CPU_IDLE and 4.14.0 */
	PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_DEFAULT;
}

static int CoreFreqK_IdleDriver_Init(void)
{
	int rc = -RC_UNIMPLEMENTED;
#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
  if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.IdleState != NULL)
  {
	IDLE_STATE *pIdleState;
	pIdleState = Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.IdleState;
   if ((pIdleState != NULL) && PUBLIC(RO(Proc))->Features.Std.ECX.MONITOR)
   {
    if ((CoreFreqK.IdleDevice=alloc_percpu(struct cpuidle_device)) == NULL)
    {
	rc = -ENOMEM;
    }
    else
    {
	struct cpuidle_device *device;
	unsigned int cpu, enroll = 0;
	/*		Kernel polling loop			*/
	cpuidle_poll_state_init(&CoreFreqK.IdleDriver);

	CoreFreqK.IdleDriver.state_count = 1;
	/*		Idle States				*/
     while (pIdleState->Name != NULL)
     {
	if (CoreFreqK.IdleDriver.state_count < SUB_CSTATE_COUNT)
	{
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].flags = pIdleState->flags;

	    if (CoreFreqK.SubCstate[CoreFreqK.IdleDriver.state_count] == 0)
	    {
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].flags |= CPUIDLE_FLAG_UNUSABLE;
	    }
		StrCopy(CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].name, pIdleState->Name, CPUIDLE_NAME_LEN);

		StrCopy(CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].desc, pIdleState->Desc, CPUIDLE_NAME_LEN);

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].exit_latency = pIdleState->Latency;

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].target_residency = pIdleState->Residency;

	  switch (Idle_Route) {
	  case ROUTE_MWAIT:
	    {
		goto IDLE_DEFAULT;
	    }
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[0] = 'M';

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[1] = 'W';

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[2] = 'T';

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_MWAIT;
		break;

	  case ROUTE_HALT:
	    {
		goto IDLE_DEFAULT;
	    }
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[0] = 'H';

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[1] = 'L';

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[2] = 'T';

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_HALT;
		break;

	  case ROUTE_IO:
	  {
	    {
		goto IDLE_DEFAULT;
	    }
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[0] = 'I';

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[1] = '/';

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].desc[2] = 'O';
	  }

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_IO;
		break;

	  case ROUTE_DEFAULT:
	  IDLE_DEFAULT:
	  default:
		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_DEFAULT;
		break;
	  }
		CoreFreqK.IdleDriver.state_count++;
	}
	pIdleState++;
     }
     if ((rc = cpuidle_register_driver(&CoreFreqK.IdleDriver)) == 0)
     {
	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
	{
	    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, HW))
	    {
		device = per_cpu_ptr(CoreFreqK.IdleDevice, cpu);
	      if (device != NULL)
	      {
		device->cpu = cpu;
		if ((rc = cpuidle_register_device(device)) == 0) {
			continue;
		}
	      }
		break;
	    }
	}
	enroll = cpu;
     }
     if (rc != 0)
     { /* Cancel the registration if the driver and/or a device failed */
	for (cpu = 0; cpu < enroll; cpu++)
	{
		device = per_cpu_ptr(CoreFreqK.IdleDevice, cpu);
	    if (device != NULL)
	    {
		cpuidle_unregister_device(device);
	    }
	}
	cpuidle_unregister_driver(&CoreFreqK.IdleDriver);
	free_percpu(CoreFreqK.IdleDevice);
     }
    }
   }
  }
#endif /* CONFIG_CPU_IDLE and 4.14.0 */
	return rc;
}

#ifdef CONFIG_CPU_IDLE
static void CoreFreqK_Idle_State_Withdraw(int idx, bool disable)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0) || (RHEL_MAJOR == 8)
	struct cpuidle_device *device;
	unsigned int cpu;
  for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
  {
    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, HW)
    && ((device = per_cpu_ptr(CoreFreqK.IdleDevice, cpu)) != NULL))
    {
      if (disable) {
	device->states_usage[idx].disable |= CPUIDLE_STATE_DISABLED_BY_DRIVER;
      } else {
	device->states_usage[idx].disable &= ~CPUIDLE_STATE_DISABLED_BY_DRIVER;
      }
    }
  }
#else
	CoreFreqK.IdleDriver.states[idx].disabled = disable;
#endif /* 5.5.0 */
}
#endif /* CONFIG_CPU_IDLE */

static long CoreFreqK_Limit_Idle(int target)
{
	long rc = -EINVAL;
#ifdef CONFIG_CPU_IDLE
	int idx, floor = -1;

    if ((target > 0) && (target <= CoreFreqK.IdleDriver.state_count))
    {
	for (idx = 0; idx < CoreFreqK.IdleDriver.state_count; idx++)
	{
	    if (idx < target)
	    {
		CoreFreqK_Idle_State_Withdraw(idx, false);

		floor = idx;
	    } else {
		CoreFreqK_Idle_State_Withdraw(idx, true);
	    }
	}
	rc = RC_SUCCESS;
    }
    else if (target == 0)
    {
	for (idx = 0; idx < CoreFreqK.IdleDriver.state_count; idx++)
	{
		CoreFreqK_Idle_State_Withdraw(idx, false);

		floor = idx;
	}
	rc = RC_SUCCESS;
    }
    if (floor != -1) {
	PUBLIC(RO(Proc))->OS.IdleDriver.stateLimit = 1 + floor;
    }
#endif /* CONFIG_CPU_IDLE */
	return rc;
}

#ifdef CONFIG_CPU_FREQ
static int CoreFreqK_Policy_Exit(struct cpufreq_policy *policy)
{
	UNUSED(policy);
	return 0;
}

static int CoreFreqK_Policy_Init(struct cpufreq_policy *policy)
{
    if (policy != NULL) {
	if (policy->cpu < PUBLIC(RO(Proc))->CPU.Count)
	{
		CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(policy->cpu)));

		policy->cpuinfo.min_freq =(Core->Boost[BOOST(MIN)]
					 * Core->Clock.Hz) / 1000LLU;

		policy->cpuinfo.max_freq =(Core->Boost[BOOST(MAX)]
					 * Core->Clock.Hz) / 1000LLU;

		/*		MANDATORY Per-CPU Initialization	*/
		policy->cpuinfo.transition_latency = CPUFREQ_ETERNAL;
		policy->cur = policy->cpuinfo.max_freq;
		policy->min = policy->cpuinfo.min_freq;
		policy->max = policy->cpuinfo.max_freq;
		policy->policy = CPUFREQ_POLICY_PERFORMANCE;
	    if (Register_Governor == 1) {
		policy->governor = &CoreFreqK.FreqGovernor;
	    } else {
		policy->governor = NULL;
	    }
	}
    }
	return 0;
}

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 19))	\
  && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 5, 0)))	\
  || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 3))	\
  || (RHEL_MAJOR == 8)
static int CoreFreqK_Policy_Verify(struct cpufreq_policy_data *policy)
#else
static int CoreFreqK_Policy_Verify(struct cpufreq_policy *policy)
#endif
{
	if (policy != NULL) {
		cpufreq_verify_within_cpu_limits(policy);
	}
	return 0;
}

static int CoreFreqK_SetPolicy(struct cpufreq_policy *policy)
{
	UNUSED(policy);
	return 0;
}

static int CoreFreqK_Bios_Limit(int cpu, unsigned int *limit)
{
    if ((cpu >= 0) && (cpu < PUBLIC(RO(Proc))->CPU.Count) && (limit != NULL))
    {
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	(*limit) = (Core->Boost[BOOST(MAX)] * Core->Clock.Hz) / 1000LLU;
    }
	return 0;
}

void Policy_Aggregate_Turbo(void)
{
    if (PUBLIC(RO(Proc))->Registration.Driver.CPUfreq & REGISTRATION_ENABLE) {
	CoreFreqK.FreqDriver.boost_enabled = (
			BITWISEAND_CC(	LOCKLESS,
					PUBLIC(RW(Proc))->TurboBoost,
					PUBLIC(RO(Proc))->TurboBoost_Mask ) != 0
	);
    }
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)	\
  || ((RHEL_MAJOR == 8) && (RHEL_MINOR > 3))
static int CoreFreqK_SetBoost(struct cpufreq_policy *policy, int state)
{
	UNUSED(policy);
#else
static int CoreFreqK_SetBoost(int state)
{
#endif /* 5.8.0 */
	Controller_Stop(1);
	TurboBoost_Enable[0] = (state != 0);
	Controller_Start(1);
	TurboBoost_Enable[0] = -1;
	Policy_Aggregate_Turbo();
	BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY); /* Notify Daemon*/
	return 0;
}

static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy *policy,char *buf)
{
  if (policy != NULL)
  {
	CORE_RO *Core;
	enum RATIO_BOOST boost;

    if (policy->cpu < PUBLIC(RO(Proc))->CPU.Count)
    {
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(policy->cpu)));
    } else {
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(PUBLIC(RO(Proc))->Service.Core)));
    }
    if (PUBLIC(RO(Proc))->Features.HWP_Enable) {
	boost = BOOST(HWP_TGT);
    } else {
	boost = BOOST(TGT);
    }
	return sprintf( buf, "%7llu\n",
			(Core->Boost[boost] * Core->Clock.Hz) / 1000LLU );
  }
	return 0;
}

static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy *policy,
					unsigned int freq)
{
  if (policy != NULL)
  {
	void (*SetTarget)(void *arg) = NULL;

	SetTarget = Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget;

    if ((policy->cpu < PUBLIC(RO(Proc))->CPU.Count) && (SetTarget != NULL))
    {
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(policy->cpu)));
	unsigned int ratio = (freq * 1000LLU) / Core->Clock.Hz;

	if (ratio > 0) {
		if (smp_call_function_single(	policy->cpu,
						SetTarget,
						&ratio, 1) == 0 )
		{
			BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY);
		}
		return 0;
	}
    }
  }
	return -EINVAL;
}
#endif /* CONFIG_CPU_FREQ */

static unsigned int Policy_GetFreq(unsigned int cpu)
{
	unsigned int CPU_Freq = PUBLIC(RO(Proc))->Features.Factory.Freq * 1000U;

    if (cpu < PUBLIC(RO(Proc))->CPU.Count)
    {
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	unsigned int Freq_MHz	= Core->Delta.C0.UCC
				/ PUBLIC(RO(Proc))->SleepInterval;

	if (Freq_MHz > 0) {	/* at least 1 interval must have been elapsed */
		CPU_Freq = Freq_MHz;
	}
    }
	return CPU_Freq;
}

static int CoreFreqK_FreqDriver_UnInit(void)
{
	int rc = -EINVAL;
#ifdef CONFIG_CPU_FREQ
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)) && (!defined(CONFIG_CACHY))
	rc =
#else
	rc = 0;
#endif
	cpufreq_unregister_driver(&CoreFreqK.FreqDriver);
#endif /* CONFIG_CPU_FREQ */
	return rc;
}

static int CoreFreqK_FreqDriver_Init(void)
{
	int rc = -RC_UNIMPLEMENTED;
#ifdef CONFIG_CPU_FREQ
 if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.IdleState != NULL)
 {
  if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.GetFreq != NULL)
  {
  CoreFreqK.FreqDriver.get=Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.GetFreq;

	rc = cpufreq_register_driver(&CoreFreqK.FreqDriver);
  }
 }
#endif /* CONFIG_CPU_FREQ */
	return rc;
}

static void CoreFreqK_Governor_UnInit(void)
{
#ifdef CONFIG_CPU_FREQ
	cpufreq_unregister_governor(&CoreFreqK.FreqGovernor);
#endif /* CONFIG_CPU_FREQ */
}

static int CoreFreqK_Governor_Init(void)
{
	int rc = -RC_UNIMPLEMENTED;
#ifdef CONFIG_CPU_FREQ
    if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.IdleState != NULL)
    {
	if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget != NULL)
	{
		rc = cpufreq_register_governor(&CoreFreqK.FreqGovernor);
	}
    }
#endif /* CONFIG_CPU_FREQ */
	return rc;
}

signed int Seek_Topology_Core_Peer(unsigned int cpu, signed int exclude)
{
	unsigned int seek;

    for (seek = 0; seek < PUBLIC(RO(Proc))->CPU.Count; seek++) {
	if ( ((exclude ^ cpu) > 0)
	  && (PUBLIC(RO(Core, AT(seek)))->T.ApicID \
		!= PUBLIC(RO(Core, AT(cpu)))->T.ApicID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.CoreID \
		== PUBLIC(RO(Core, AT(cpu)))->T.CoreID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.ThreadID \
		!= PUBLIC(RO(Core, AT(cpu)))->T.ThreadID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.PackageID \
		== PUBLIC(RO(Core, AT(cpu)))->T.PackageID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.ThreadID == 0)
	  && !BITVAL(PUBLIC(RO(Core, AT(seek)))->OffLine, OS) )
	{
		return (signed int) seek;
	}
    }
	return -1;
}

signed int Seek_Topology_Thread_Peer(unsigned int cpu, signed int exclude)
{
	unsigned int seek;

    for (seek = 0; seek < PUBLIC(RO(Proc))->CPU.Count; seek++) {
	if ( ((exclude ^ cpu) > 0)
	  && (PUBLIC(RO(Core, AT(seek)))->T.ApicID \
		!= PUBLIC(RO(Core, AT(cpu)))->T.ApicID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.CoreID \
		== PUBLIC(RO(Core, AT(cpu)))->T.CoreID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.ThreadID \
		!= PUBLIC(RO(Core, AT(cpu)))->T.ThreadID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.PackageID \
		== PUBLIC(RO(Core, AT(cpu)))->T.PackageID)
	  && (PUBLIC(RO(Core, AT(seek)))->T.ThreadID > 0)
	  && !BITVAL(PUBLIC(RO(Core, AT(seek)))->OffLine, OS) )
	{
		return (signed int) seek;
	}
    }
	return -1;
}

signed int Seek_Topology_Hybrid_Core(unsigned int cpu)
{
	signed int any = (signed int) PUBLIC(RO(Proc))->CPU.Count, seek;

    do {
	any--;
    } while (BITVAL(PUBLIC(RO(Core, AT(any)))->OffLine, OS) && (any > 0)) ;

    for (seek = any; seek != 0; seek--)
    {
      if ((PUBLIC(RO(Core, AT(seek)))->T.Cluster.Hybrid.CoreType == Hybrid_Atom)
       && (PUBLIC(RO(Core, AT(seek)))->T.PackageID \
	== PUBLIC(RO(Core, AT(cpu)))->T.PackageID)
       && !BITVAL(PUBLIC(RO(Core, AT(seek)))->OffLine, OS))
	{
		any = seek;
		break;
	}
    }
	return any;
}

void MatchCoreForService(SERVICE_PROC *pService,unsigned int cpi,signed int cpx)
{
	unsigned int cpu;

    for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++) {
	if ( ((cpx ^ cpu) > 0)
	  && (PUBLIC(RO(Core, AT(cpu)))->T.PackageID \
		== PUBLIC(RO(Core, AT(cpi)))->T.PackageID)
	  && !BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS) )
	{
		pService->Core = cpu;
		pService->Thread = -1;
		break;
	}
    }
}

int MatchPeerForService(SERVICE_PROC *pService, unsigned int cpi,signed int cpx)
{
	unsigned int cpu = cpi, cpn = 0;
	signed int seek;
MATCH:
	if (PUBLIC(RO(Core, AT(cpu)))->T.ThreadID == 0)
	{
		if ((seek = Seek_Topology_Thread_Peer(cpu, cpx)) != -1) {
			pService->Core = cpu;
			pService->Thread = seek;
			return 0;
		}
	}
	else if (PUBLIC(RO(Core, AT(cpu)))->T.ThreadID > 0)
	{
		if ((seek = Seek_Topology_Core_Peer(cpu, cpx)) != -1) {
			pService->Core = seek;
			pService->Thread = cpu;
			return 0;
		}
	}
	while (cpn < PUBLIC(RO(Proc))->CPU.Count) {
		cpu = cpn++;
		if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)) {
			goto MATCH;
		}
	}
	return -1;
}

void MatchPeerForDefaultService(SERVICE_PROC *pService, unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.HTT_Enable) {
		if (MatchPeerForService(pService, cpu, -1) == -1)
		{
			MatchCoreForService(pService, cpu, -1);
		}
	} else {
		pService->Core = cpu;
		pService->Thread = -1;
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.Hybrid) {
		pService->Hybrid = Seek_Topology_Hybrid_Core(cpu);
	} else {
		pService->Hybrid = -1;
	}
	if (ServiceProcessor != -1) {
		DefaultSMT.Core = pService->Core;
		DefaultSMT.Thread = pService->Thread;
	}
}

void MatchPeerForUpService(SERVICE_PROC *pService, unsigned int cpu)
{	/* Try to restore the initial Service affinity or move to SMT peer. */
	SERVICE_PROC hService = {
		.Core = cpu,
		.Thread = -1,
		.Hybrid = -1
	};
	if (PUBLIC(RO(Proc))->Features.HTT_Enable)
	{
		signed int seek;

		if ((PUBLIC(RO(Core, AT(cpu)))->T.ThreadID == 0)
		&& ((seek = Seek_Topology_Thread_Peer(cpu, -1)) != -1))
		{
			hService.Core = cpu;
			hService.Thread = seek;
		} else {
			if ((PUBLIC(RO(Core, AT(cpu)))->T.ThreadID > 0)
			&& ((seek = Seek_Topology_Core_Peer(cpu, -1)) != -1))
			{
				hService.Core = seek;
				hService.Thread = cpu;
			}
		}
	}
	if ((pService->Core != DefaultSMT.Core)
	 || (pService->Thread != DefaultSMT.Thread))
	{
		if ((hService.Core == DefaultSMT.Core)
		 && (hService.Thread == DefaultSMT.Thread))
		{
			pService->Core = hService.Core;
			pService->Thread = hService.Thread;
		} else {
			if ((pService->Thread == -1) && (hService.Thread > 0))
			{
				pService->Core = hService.Core;
				pService->Thread = hService.Thread;
			}
		}
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.Hybrid) {
		pService->Hybrid = Seek_Topology_Hybrid_Core(cpu);
	}
}

void MatchPeerForDownService(SERVICE_PROC *pService, unsigned int cpu)
{
	int rc = -1;

	if (PUBLIC(RO(Proc))->Features.HTT_Enable) {
		rc = MatchPeerForService(pService, cpu, cpu);
	}
	if (rc == -1) {
		MatchCoreForService(pService, cpu, cpu);
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
#if defined(CONFIG_HAVE_NMI) && defined(CONFIG_X86_64)
static int CoreFreqK_NMI_Handler(unsigned int type, struct pt_regs *pRegs)
{
	unsigned int cpu = smp_processor_id();
	UNUSED(pRegs);

	switch (type) {
	case NMI_LOCAL:
		PUBLIC(RO(Core, AT(cpu)))->Interrupt.NMI.LOCAL++;
		break;
	case NMI_UNKNOWN:
		PUBLIC(RO(Core, AT(cpu)))->Interrupt.NMI.UNKNOWN++;
		break;
	case NMI_SERR:
		PUBLIC(RO(Core, AT(cpu)))->Interrupt.NMI.PCISERR++;
		break;
	case NMI_IO_CHECK:
		PUBLIC(RO(Core, AT(cpu)))->Interrupt.NMI.IOCHECK++;
		break;
	}
	return NMI_DONE;
}
#endif /* CONFIG_HAVE_NMI */

static long CoreFreqK_UnRegister_CPU_Idle(void)
{
	long rc = -EINVAL;
    if (PUBLIC(RO(Proc))->Registration.Driver.CPUidle & REGISTRATION_ENABLE)
    {
	CoreFreqK_IdleDriver_UnInit();
	PUBLIC(RO(Proc))->Registration.Driver.CPUidle = REGISTRATION_DISABLE;
	rc = RC_SUCCESS;
    }
	return rc;
}

static long CoreFreqK_Register_CPU_Idle(void)
{
	long rc = -EINVAL;
  if (Register_CPU_Idle == 1)
  {
	int rx = CoreFreqK_IdleDriver_Init();
    switch ( rx ) {
    default:
	fallthrough;
    case -ENODEV:
    case -ENOMEM:
	PUBLIC(RO(Proc))->Registration.Driver.CPUidle = REGISTRATION_DISABLE;
	rc = (long) rx;
	break;
    case 0:	/*	Registration succeeded.				*/
	PUBLIC(RO(Proc))->Registration.Driver.CPUidle = REGISTRATION_ENABLE;
	rc = RC_SUCCESS;
	break;
    }
  } else {	/*	Nothing requested by User.			*/
	PUBLIC(RO(Proc))->Registration.Driver.CPUidle = REGISTRATION_DISABLE;
  }
	return rc;
}

static long CoreFreqK_UnRegister_CPU_Freq(void)
{
	long rc = -EINVAL;
    if (PUBLIC(RO(Proc))->Registration.Driver.CPUfreq & REGISTRATION_ENABLE)
    {
	int rx = CoreFreqK_FreqDriver_UnInit();
	switch ( rx ) {
	case 0:
	    PUBLIC(RO(Proc))->Registration.Driver.CPUfreq=REGISTRATION_DISABLE;
		rc = RC_SUCCESS;
		break;
	default:
		rc = (long) rx;
		break;
	}
    }
	return rc;
}

static long CoreFreqK_Register_CPU_Freq(void)
{ /* Source: cpufreq_register_driver @ /drivers/cpufreq/cpufreq.c	*/
	long rc = -EINVAL;
  if (Register_CPU_Freq == 1)
  {
	int rx = CoreFreqK_FreqDriver_Init();
    switch ( rx ) {
    default:
	fallthrough;
    case -EEXIST:		/*	Another driver is in control.	*/
	PUBLIC(RO(Proc))->Registration.Driver.CPUfreq = REGISTRATION_DISABLE;
	rc = (long) rx;
	break;
    case -ENODEV:		/*	Missing CPU-Freq or Interfaces.	*/
    case -EPROBE_DEFER:	/*	CPU probing failed			*/
    case -EINVAL:		/*	Missing CPU-Freq prerequisites. */
	PUBLIC(RO(Proc))->Registration.Driver.CPUfreq = REGISTRATION_FULLCTRL;
	rc = (long) rx;
	break;
    case 0:		/*	Registration succeeded .		*/
	PUBLIC(RO(Proc))->Registration.Driver.CPUfreq = REGISTRATION_ENABLE;
	rc = RC_SUCCESS;
	break;
    }
  } else {		/*	Invalid or no User request.		*/
#ifdef CONFIG_CPU_FREQ
	PUBLIC(RO(Proc))->Registration.Driver.CPUfreq = REGISTRATION_DISABLE;
#else	/* No CPU-FREQ built in Kernel, presume we have the full control. */
	PUBLIC(RO(Proc))->Registration.Driver.CPUfreq = REGISTRATION_FULLCTRL;
#endif /* CONFIG_CPU_FREQ */
  }
	return rc;
}

static long CoreFreqK_UnRegister_Governor(void)
{
	long rc = EINVAL;
    if (PUBLIC(RO(Proc))->Registration.Driver.Governor & REGISTRATION_ENABLE)
    {
	CoreFreqK_Governor_UnInit();
	PUBLIC(RO(Proc))->Registration.Driver.Governor = REGISTRATION_DISABLE;
	rc = RC_SUCCESS;
    }
	return rc;
}

static long CoreFreqK_Register_Governor(void)
{
	long rc = -EINVAL;
  if (Register_Governor == 1)
  {
	int rx = CoreFreqK_Governor_Init();
    switch ( rx ) {
    default:
    case -ENODEV:
	PUBLIC(RO(Proc))->Registration.Driver.Governor = REGISTRATION_DISABLE;
	rc = (long) rx;
	break;
    case 0:		/*	Registration succeeded .		*/
	PUBLIC(RO(Proc))->Registration.Driver.Governor = REGISTRATION_ENABLE;
	rc = RC_SUCCESS;
	break;
    }
  } else {	/* Nothing requested by User.				*/
	PUBLIC(RO(Proc))->Registration.Driver.Governor = REGISTRATION_DISABLE;
  }
	return rc;
}

static void CoreFreqK_Register_NMI(void)
{
#if defined(CONFIG_HAVE_NMI) && defined(CONFIG_X86_64)
  if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_LOCAL) == 0)
  {
    if(register_nmi_handler(NMI_LOCAL,
			CoreFreqK_NMI_Handler,
			0,
			"corefreqk") == 0)
    {
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_LOCAL);
    } else {
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_LOCAL);
    }
  }
  if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_UNKNOWN) == 0)
  {
    if(register_nmi_handler(NMI_UNKNOWN,
			CoreFreqK_NMI_Handler,
			0,
			"corefreqk") == 0)
    {
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_UNKNOWN);
    } else {
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_UNKNOWN);
    }
  }
  if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_SERR) == 0)
  {
    if(register_nmi_handler(NMI_SERR,
			CoreFreqK_NMI_Handler,
			0,
			"corefreqk") == 0)
    {
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_SERR);
    } else {
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_SERR);
    }
  }
  if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_IO_CHECK) == 0)
  {
    if(register_nmi_handler(NMI_IO_CHECK,
			CoreFreqK_NMI_Handler,
			0,
			"corefreqk") == 0)
    {
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_IO_CHECK);
    } else {
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_IO_CHECK);
    }
  }
#endif /* CONFIG_HAVE_NMI */
}

static void CoreFreqK_UnRegister_NMI(void)
{
#if defined(CONFIG_HAVE_NMI) && defined(CONFIG_X86_64)
    if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_LOCAL) == 1)
    {
	unregister_nmi_handler(NMI_LOCAL, "corefreqk");
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_LOCAL);
    }
    if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_UNKNOWN) == 1)
    {
	unregister_nmi_handler(NMI_UNKNOWN, "corefreqk");
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_UNKNOWN);
    }
    if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_SERR) == 1)
    {
	unregister_nmi_handler(NMI_SERR, "corefreqk");
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_SERR);
    }
    if (BITVAL(PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_IO_CHECK) == 1)
    {
	unregister_nmi_handler(NMI_IO_CHECK, "corefreqk");
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Registration.NMI, BIT_NMI_IO_CHECK);
    }
#endif /* CONFIG_HAVE_NMI */
}
#else
static void CoreFreqK_Register_NMI(void) {}
static void CoreFreqK_UnRegister_NMI(void) {}
#endif


static void For_All_CPU_Compute_Clock(void)
{
	unsigned int cpu = PUBLIC(RO(Proc))->CPU.Count;
  do {
	CLOCK Clock = {
		.Q  = PUBLIC(RO(Proc))->Features.Factory.Clock.Q,
		.R  = PUBLIC(RO(Proc))->Features.Factory.Clock.R,
		.Hz = PUBLIC(RO(Proc))->Features.Factory.Clock.Hz
	};
	/* from last AP to BSP */
	cpu--;

    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS))
    {
	COMPUTE_ARG Compute = {
		.TSC = {NULL, NULL},
		.Clock = {
			.Q = PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)],
			.R = 0, .Hz = 0
		}
	};
      if ((Compute.TSC[0] = kmalloc(STRUCT_SIZE, GFP_KERNEL)) != NULL)
      {
	if ((Compute.TSC[1] = kmalloc(STRUCT_SIZE, GFP_KERNEL)) != NULL)
	{
		Clock = Compute_Clock(cpu, &Compute);

		kfree(Compute.TSC[1]);
	}
		kfree(Compute.TSC[0]);
      }
    }
	PUBLIC(RO(Core, AT(cpu)))->Clock.Q  = Clock.Q;
	PUBLIC(RO(Core, AT(cpu)))->Clock.R  = Clock.R;
	PUBLIC(RO(Core, AT(cpu)))->Clock.Hz = Clock.Hz;
  } while (cpu != 0) ;
}

#define SYSGATE_UPDATE(_rc)						\
({									\
	_rc = Sys_OS_Driver_Query();					\
	_rc = (_rc != -ENXIO) ? RC_OK_SYSGATE : _rc;			\
})

static long CoreFreqK_ioctl(	struct file *filp,
				unsigned int cmd,
				unsigned long arg )
{
	long rc = -EPERM;
	UNUSED(filp);

    switch ((enum COREFREQ_MAGIC_COMMAND) cmd)
    {
    case COREFREQ_IOCTL_SYSUPDT:
	Controller_Stop(1);
	SYSGATE_UPDATE(rc);
	Controller_Start(1);
	BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY);
    break;

    case COREFREQ_IOCTL_SYSONCE:
	rc = Sys_OS_Driver_Query();
	rc = (rc != -ENXIO) ? Sys_Kernel(PUBLIC(OF(Gate))) : rc;
    break;

    case COREFREQ_IOCTL_MACHINE:
    {
	RING_ARG_QWORD prm = {.arg = arg};

      switch (prm.dl.hi)
      {
      case MACHINE_CONTROLLER:
	switch (prm.dl.lo)
	{
	case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		rc = RC_SUCCESS;
		break;
	case COREFREQ_TOGGLE_ON:
		Controller_Start(1);
	#ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	#endif /* CONFIG_CPU_FREQ */
		rc = RC_OK_COMPUTE;
		break;
	}
	break;

      case MACHINE_INTERVAL:
	Controller_Stop(1);
	SleepInterval = prm.dl.lo;
	Compute_Interval();
	Controller_Start(1);
	rc = RC_SUCCESS;
	break;

      case MACHINE_AUTOCLOCK:
	switch (prm.dl.lo)
	{
	case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		For_All_CPU_Compute_Clock();
		BITCLR(LOCKLESS, AutoClock, 1);
		PUBLIC(RO(Proc))->Registration.AutoClock = AutoClock;
		Controller_Start(1);
		rc = RC_SUCCESS;
		break;
	case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		BITSET(LOCKLESS, AutoClock, 1);
		PUBLIC(RO(Proc))->Registration.AutoClock = AutoClock;
		Controller_Start(1);
		rc = RC_SUCCESS;
		break;
	}
	break;

      case MACHINE_EXPERIMENTAL:
	switch (prm.dl.lo) {
	    case COREFREQ_TOGGLE_OFF:
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		PUBLIC(RO(Proc))->Registration.Experimental = prm.dl.lo;
		Controller_Start(1);
	    if (PUBLIC(RO(Proc))->Registration.Experimental)
	    {
	      if ( !PUBLIC(RO(Proc))->Registration.PCI ) {
		PUBLIC(RO(Proc))->Registration.PCI = \
			CoreFreqK_ProbePCI(
				Arch[PUBLIC(RO(Proc))->ArchID].PCI_ids,
				CoreFreqK_ResetChip, CoreFreqK_AppendChip
			) == 0;
		rc = RC_OK_COMPUTE;
	     } else {
		rc = RC_SUCCESS;
	     }
	    } else {
		rc = RC_SUCCESS;
	    }
		break;
	}
	break;

      case MACHINE_INTERRUPTS:
	switch (prm.dl.lo)
	{
	    case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		CoreFreqK_UnRegister_NMI();
		Controller_Start(1);
		rc = RC_SUCCESS;
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		CoreFreqK_Register_NMI();
		Controller_Start(1);
		rc = RC_SUCCESS;
		break;
	}
	break;

      case MACHINE_LIMIT_IDLE:
	if (PUBLIC(RO(Proc))->Registration.Driver.CPUidle & REGISTRATION_ENABLE)
	{
		rc = CoreFreqK_Limit_Idle(prm.dl.lo);
	}
	break;

      case MACHINE_CPU_IDLE:
	switch (prm.dl.lo)
	{
	    case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		rc = CoreFreqK_UnRegister_CPU_Idle();
		Register_CPU_Idle = -1;
		if (rc == RC_SUCCESS) {
			SYSGATE_UPDATE(rc);
		}
		Controller_Start(1);
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_CPU_Idle = 1;
		rc = CoreFreqK_Register_CPU_Idle();
		if (rc == RC_SUCCESS) {
			SYSGATE_UPDATE(rc);
		}
		Controller_Start(1);
		break;
	}
	break;

      case MACHINE_IDLE_ROUTE:
	if (PUBLIC(RO(Proc))->Registration.Driver.CPUidle & REGISTRATION_ENABLE)
	{
		Controller_Stop(1);
		rc = CoreFreqK_UnRegister_CPU_Idle();
		Register_CPU_Idle = -1;
		if (rc == RC_SUCCESS)
		{
			Register_CPU_Idle = 1;
			Idle_Route = prm.dl.lo;
			rc = CoreFreqK_Register_CPU_Idle();
			if (rc == RC_SUCCESS) {
				SYSGATE_UPDATE(rc);
			}
		}
		Controller_Start(1);
	}
	break;

      case MACHINE_CPU_FREQ:
	switch (prm.dl.lo)
	{
	    case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		rc = CoreFreqK_UnRegister_CPU_Freq();
		Register_CPU_Freq = -1;
		if (rc == RC_SUCCESS) {
			SYSGATE_UPDATE(rc);
		}
		Controller_Start(1);
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_CPU_Freq = 1;
		rc = CoreFreqK_Register_CPU_Freq();
		if (rc == RC_SUCCESS) {
			SYSGATE_UPDATE(rc);
		}
		Controller_Start(1);
		break;
	}
	break;

      case MACHINE_GOVERNOR:
	switch (prm.dl.lo)
	{
	    case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		rc = CoreFreqK_UnRegister_Governor();
		Register_Governor = -1;
		if (rc == RC_SUCCESS) {
			SYSGATE_UPDATE(rc);
		}
		Controller_Start(1);
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_Governor = 1;
		rc = CoreFreqK_Register_Governor();
		if (rc == RC_SUCCESS) {
			SYSGATE_UPDATE(rc);
		}
		Controller_Start(1);
		break;
	}
	break;

      case MACHINE_CLOCK_SOURCE:
	switch (prm.dl.lo)
	{
	    case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		rc = CoreFreqK_UnRegister_ClockSource();
		Register_ClockSource = -1;
		Controller_Start(1);
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_ClockSource = 1;
	      rc=CoreFreqK_Register_ClockSource(PUBLIC(RO(Proc))->Service.Core);
		Controller_Start(1);
		break;
	}
	break;

      case MACHINE_FORMULA_SCOPE:
	switch (prm.dl.lo)
	{
	    case 0:
		rc = CoreFreqK_Thermal_Scope(prm.dh.lo);
		break;
	    case 1:
		rc = CoreFreqK_Voltage_Scope(prm.dh.lo);
		break;
	    case 2:
		rc = CoreFreqK_Power_Scope(prm.dh.lo);
		break;
	}
	break;
      }
    }
    break;

    case COREFREQ_IOCTL_TECHNOLOGY:
    {
	RING_ARG_QWORD prm = {.arg = arg};

	switch (prm.dl.hi)
	{
	case TECHNOLOGY_TURBO:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				TurboBoost_Enable[0] = prm.dl.lo;
				Controller_Start(1);
				TurboBoost_Enable[0] = -1;
			#ifdef CONFIG_CPU_FREQ
				Policy_Aggregate_Turbo();
			#endif /* CONFIG_CPU_FREQ */
				rc = RC_OK_COMPUTE;
				break;
		}
		break;

	case TECHNOLOGY_HWP:
	    switch (prm.dl.lo) {
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		HWP_Enable = prm.dl.lo;
	      {
		if (PUBLIC(RO(Proc))->Features.ACPI_CPPC) {
			For_All_ACPI_CPPC(Enable_ACPI_CPPC, NULL);
		}
	      }
		Controller_Start(1);
		HWP_Enable = -1;
		rc = RC_SUCCESS;
		break;
	    case COREFREQ_TOGGLE_OFF:
	      {
		if (PUBLIC(RO(Proc))->Features.ACPI_CPPC) {
			Controller_Stop(1);
			For_All_ACPI_CPPC(Disable_ACPI_CPPC, NULL);
			Controller_Start(1);
			rc = RC_SUCCESS;
		}
	      }
		break;
	    }
	    break;

	case TECHNOLOGY_HWP_EPP:
		Controller_Stop(1);
		HWP_EPP = prm.dl.lo;
		if (PUBLIC(RO(Proc))->Features.OSPM_EPP == 1)
		{
			For_All_ACPI_CPPC(Set_EPP_ACPI_CPPC, NULL);
		}
		Controller_Start(1);
		HWP_EPP = -1;
		rc = RC_SUCCESS;
		break;
	}
	break;
    }
    break;

    case COREFREQ_IOCTL_CPU_OFF:
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
	{
	unsigned int cpu = (unsigned int) arg;

	if (cpu < PUBLIC(RO(Proc))->CPU.Count) {
		if (!cpu_is_hotpluggable(cpu)) {
			rc = -EBUSY;
		} else {
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
			if ((rc = remove_cpu(cpu)) == 0) {
				rc = RC_OK_COMPUTE;
			}
	#else
			if ((rc = cpu_down(cpu)) == 0) {
				rc = RC_OK_COMPUTE;
			}
	#endif
		}
	    }
	}
    #else
	rc = -EINVAL;
    #endif
	break;

    case COREFREQ_IOCTL_CPU_ON:
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
	{
	unsigned int cpu = (unsigned int) arg;

	if (cpu < PUBLIC(RO(Proc))->CPU.Count) {
		if (!cpu_is_hotpluggable(cpu)) {
			rc = -EBUSY;
		} else {
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
			if ((rc = add_cpu(cpu)) == 0) {
				rc = RC_OK_COMPUTE;
			}
	#else
			if ((rc = cpu_up(cpu)) == 0) {
				rc = RC_OK_COMPUTE;
			}
	#endif
		}
	    }
	}
    #else
	rc = -EINVAL;
    #endif
	break;

    case COREFREQ_IOCTL_TURBO_CLOCK:
	if (Arch[PUBLIC(RO(Proc))->ArchID].TurboClock) {
		CLOCK_ARG clockMod = {.sllong = arg};
		Controller_Stop(1);
		rc = Arch[PUBLIC(RO(Proc))->ArchID].TurboClock(&clockMod);
		Controller_Start(1);
	} else {
		rc = -RC_UNIMPLEMENTED;
	}
	break;

    case COREFREQ_IOCTL_RATIO_CLOCK:
	if (Arch[PUBLIC(RO(Proc))->ArchID].ClockMod) {
		CLOCK_ARG clockMod = {.sllong = arg};
		Controller_Stop(1);
		rc = Arch[PUBLIC(RO(Proc))->ArchID].ClockMod(&clockMod);
		Controller_Start(1);
	#ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	#endif /* CONFIG_CPU_FREQ */
	} else {
		rc = -RC_UNIMPLEMENTED;
	}
	break;

    case COREFREQ_IOCTL_UNCORE_CLOCK:
	if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.ClockMod) {
		CLOCK_ARG clockMod = {.sllong = arg};
		Controller_Stop(1);
		rc = Arch[PUBLIC(RO(Proc))->ArchID].Uncore.ClockMod(&clockMod);
		Controller_Start(1);
	} else {
		rc = -RC_UNIMPLEMENTED;
	}
	break;

    case COREFREQ_IOCTL_CLEAR_EVENTS:
	switch (arg) {
		case EVENT_THERMAL_LOG:
		case EVENT_PROCHOT_LOG:
		case EVENT_CRITIC_LOG:
		case EVENT_THOLD1_LOG:
		case EVENT_THOLD2_LOG:
		case EVENT_POWER_LIMIT:
		case EVENT_CURRENT_LIMIT:
		case EVENT_CROSS_DOMAIN:
		case EVENT_CORE_HOT_LOG:
		case EVENT_CORE_THM_LOG:
		case EVENT_CORE_RES_LOG:
		case EVENT_CORE_AVG_LOG:
		case EVENT_CORE_VRT_LOG:
		case EVENT_CORE_TDC_LOG:
		case EVENT_CORE_PL1_LOG:
		case EVENT_CORE_PL2_LOG:
		case EVENT_CORE_EDP_LOG:
		case EVENT_CORE_BST_LOG:
		case EVENT_CORE_ATT_LOG:
		case EVENT_CORE_TVB_LOG:
		case EVENT_GFX_HOT_LOG:
		case EVENT_GFX_THM_LOG:
		case EVENT_GFX_AVG_LOG:
		case EVENT_GFX_VRT_LOG:
		case EVENT_GFX_TDC_LOG:
		case EVENT_GFX_PL1_LOG:
		case EVENT_GFX_PL2_LOG:
		case EVENT_GFX_EDP_LOG:
		case EVENT_GFX_EFF_LOG:
		case EVENT_RING_HOT_LOG:
		case EVENT_RING_THM_LOG:
		case EVENT_RING_AVG_LOG:
		case EVENT_RING_VRT_LOG:
		case EVENT_RING_TDC_LOG:
		case EVENT_RING_PL1_LOG:
		case EVENT_RING_PL2_LOG:
		case EVENT_RING_EDP_LOG:
			Controller_Stop(1);
			Clear_Events = arg;
			Controller_Start(1);
			Clear_Events = 0;
			rc = RC_OK_COMPUTE;
			break;
		case EVENT_ALL_OF_THEM:
			Controller_Stop(1);
			Clear_Events = (unsigned long long)(-1);
			Controller_Start(1);
			Clear_Events = 0;
			rc = RC_OK_COMPUTE;
			break;
	}
	break;

    default:
	rc = -EINVAL;
	break;
    }
	return rc;
}

#undef SYSGATE_UPDATE

static int CoreFreqK_mmap(struct file *pfile, struct vm_area_struct *vma)
{
	unsigned long reqSize = vma->vm_end - vma->vm_start;
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	vm_flags_t vm_ro = VM_READ;
    #endif
	int rc = -EIO;
	UNUSED(pfile);

  if (vma->vm_pgoff == ID_RO_VMA_PROC) {
    if (PUBLIC(RO(Proc)) != NULL)
    {
	const unsigned long secSize = ROUND_TO_PAGES(sizeof(PROC_RO));
	if (reqSize != secSize) {
		rc = -EAGAIN;
		goto EXIT_PAGE;
	}

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	vm_flags_reset_once(vma, vm_ro);
    #else
	vma->vm_flags = VM_READ;
    #endif
	vma->vm_page_prot = PAGE_READONLY;

	rc = remap_pfn_range(	vma,
				vma->vm_start,
			virt_to_phys((void *) PUBLIC(RO(Proc))) >> PAGE_SHIFT,
				reqSize,
				vma->vm_page_prot);
    }
  } else if (vma->vm_pgoff == ID_RW_VMA_PROC) {
    if (PUBLIC(RW(Proc)) != NULL)
    {
	const unsigned long secSize = ROUND_TO_PAGES(sizeof(PROC_RW));
	if (reqSize != secSize) {
		rc = -EAGAIN;
		goto EXIT_PAGE;
	}

	rc = remap_pfn_range(	vma,
				vma->vm_start,
			virt_to_phys((void *) PUBLIC(RW(Proc))) >> PAGE_SHIFT,
				reqSize,
				vma->vm_page_prot);
    }
  } else if (vma->vm_pgoff == ID_RO_VMA_GATE) {
    if (PUBLIC(RO(Proc)) != NULL)
    {
	switch (SysGate_OnDemand()) {
	default:
	case -1:
		break;
	case 1:
		fallthrough;
	case 0: {
		const unsigned long
		secSize = PAGE_SIZE << PUBLIC(RO(Proc))->Gate.ReqMem.Order;
		if (reqSize != secSize) {
			return -EAGAIN;
		}

	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
		vm_flags_reset_once(vma, vm_ro);
	    #else
		vma->vm_flags = VM_READ;
	    #endif
		vma->vm_page_prot = PAGE_READONLY;

		rc = remap_pfn_range(	vma,
					vma->vm_start,
			virt_to_phys((void *) PUBLIC(OF(Gate))) >> PAGE_SHIFT,
					reqSize,
					vma->vm_page_prot);
	    }
		break;
	}
    }
  } else if ((vma->vm_pgoff >= ID_RO_VMA_CORE)
	  && (vma->vm_pgoff < ID_RW_VMA_CORE))
  {
	signed int cpu = vma->vm_pgoff - ID_RO_VMA_CORE;

    if (PUBLIC(RO(Proc)) != NULL) {
      if ((cpu >= 0) && (cpu < PUBLIC(RO(Proc))->CPU.Count)) {
	if (PUBLIC(RO(Core, AT(cpu))) != NULL)
	{
		const unsigned long secSize = ROUND_TO_PAGES(sizeof(CORE_RO));
		if (reqSize != secSize) {
			rc = -EAGAIN;
			goto EXIT_PAGE;
		}

	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
		vm_flags_reset_once(vma, vm_ro);
	    #else
		vma->vm_flags = VM_READ;
	    #endif
		vma->vm_page_prot = PAGE_READONLY;

		rc = remap_pfn_range(	vma,
					vma->vm_start,
		virt_to_phys((void *) PUBLIC(RO(Core, AT(cpu)))) >> PAGE_SHIFT,
					reqSize,
					vma->vm_page_prot);
	}
      }
    }
  } else if ((vma->vm_pgoff >= ID_RW_VMA_CORE)
	  && (vma->vm_pgoff < ID_ANY_VMA_JAIL))
  {
	signed int cpu = vma->vm_pgoff - ID_RW_VMA_CORE;

    if (PUBLIC(RO(Proc)) != NULL) {
      if ((cpu >= 0) && (cpu < PUBLIC(RO(Proc))->CPU.Count)) {
	if (PUBLIC(RW(Core, AT(cpu))) != NULL)
	{
		const unsigned long secSize = ROUND_TO_PAGES(sizeof(CORE_RW));
		if (reqSize != secSize) {
			rc = -EAGAIN;
			goto EXIT_PAGE;
		}

		rc = remap_pfn_range(	vma,
					vma->vm_start,
		virt_to_phys((void *) PUBLIC(RW(Core, AT(cpu)))) >> PAGE_SHIFT,
					reqSize,
					vma->vm_page_prot);
	}
      }
    }
  }
EXIT_PAGE:
	return rc;
}

static DEFINE_MUTEX(CoreFreqK_mutex);		/* Only one driver instance. */

static int CoreFreqK_open(struct inode *inode, struct file *pfile)
{
	UNUSED(inode);
	UNUSED(pfile);

	if (!mutex_trylock(&CoreFreqK_mutex))
		return -EBUSY;
	else
		return 0;
}

static int CoreFreqK_release(struct inode *inode, struct file *pfile)
{
	UNUSED(inode);
	UNUSED(pfile);

	mutex_unlock(&CoreFreqK_mutex);
	return 0;
}

static struct file_operations CoreFreqK_fops = {
	.open		= CoreFreqK_open,
	.release	= CoreFreqK_release,
	.mmap		= CoreFreqK_mmap,
	.unlocked_ioctl = CoreFreqK_ioctl,
	.owner		= THIS_MODULE,
};

#ifdef CONFIG_PM_SLEEP
void Print_SuspendResume(void)
{
	pr_notice("CoreFreq: %s(%u:%d:%d)\n",
		CoreFreqK.ResumeFromSuspend ? "Suspend" : "Resume",
		PUBLIC(RO(Proc))->Service.Core,
		PUBLIC(RO(Proc))->Service.Thread,
		PUBLIC(RO(Proc))->Service.Hybrid);
}

static int CoreFreqK_Suspend(struct device *dev)
{
	UNUSED(dev);

	CoreFreqK.ResumeFromSuspend = true;

	Controller_Stop(1);

	Print_SuspendResume();
	return 0;
}

static int CoreFreqK_Resume(struct device *dev)
{	/*		Probe Processor again				*/
	UNUSED(dev);

    if (Arch[PUBLIC(RO(Proc))->ArchID].Query != NULL) {
	Arch[PUBLIC(RO(Proc))->ArchID].Query(PUBLIC(RO(Proc))->Service.Core);
    }
	/*		Probe PCI again 				*/
    if (PUBLIC(RO(Proc))->Registration.PCI) {
	PUBLIC(RO(Proc))->Registration.PCI = \
		CoreFreqK_ProbePCI(Arch[PUBLIC(RO(Proc))->ArchID].PCI_ids,
				CoreFreqK_ResetChip, CoreFreqK_AppendChip) == 0;
    }
	Controller_Start(1);

#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
#endif /* CONFIG_CPU_FREQ */

	BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY); /* Notify Daemon*/

	CoreFreqK.ResumeFromSuspend = false;

	Print_SuspendResume();
	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
static DEFINE_SIMPLE_DEV_PM_OPS(CoreFreqK_pm_ops,	\
				CoreFreqK_Suspend,	\
				CoreFreqK_Resume);
#else
static SIMPLE_DEV_PM_OPS(CoreFreqK_pm_ops, CoreFreqK_Suspend, CoreFreqK_Resume);
#endif /* KERNEL_VERSION(5, 17, 0) */
#define COREFREQ_PM_OPS (&CoreFreqK_pm_ops)
#else /* CONFIG_PM_SLEEP */
#define COREFREQ_PM_OPS NULL
#endif /* CONFIG_PM_SLEEP */


#ifdef CONFIG_HOTPLUG_CPU
static int CoreFreqK_HotPlug_CPU_Online(unsigned int cpu)
{
  if (cpu < PUBLIC(RO(Proc))->CPU.Count)
  {
	/*	Is this the very first time the processor is online ?	*/
   if (PUBLIC(RO(Core, AT(cpu)))->T.ApicID == -1)
   {
    if (Core_Topology(cpu) == 0)
    {
     if (PUBLIC(RO(Core, AT(cpu)))->T.ApicID >= 0)
     {
	BITCLR(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, HW);
	/*		Is the BCLK frequency missing  ?		*/
      if (PUBLIC(RO(Core, AT(cpu)))->Clock.Hz == 0)
      {
       if (AutoClock & 0b01)
       {
	COMPUTE_ARG Compute = {
		.TSC = {NULL, NULL},
		.Clock = {
			.Q = PUBLIC(RO(Core, \
			AT(PUBLIC(RO(Proc))->Service.Core)))->Boost[BOOST(MAX)],
			.R = 0, .Hz = 0
		}
	};
	if ((Compute.TSC[0] = kmalloc(STRUCT_SIZE, GFP_KERNEL)) != NULL)
	{
	    if ((Compute.TSC[1] = kmalloc(STRUCT_SIZE, GFP_KERNEL)) != NULL)
	    {
		PUBLIC(RO(Core, AT(cpu)))->Clock = Compute_Clock(cpu, &Compute);

		kfree(Compute.TSC[1]);
	    }
		kfree(Compute.TSC[0]);
	}
       } else {
	PUBLIC(RO(Core, AT(cpu)))->Clock = \
		PUBLIC(RO(Core, AT(PUBLIC(RO(Proc))->Service.Core)))->Clock;
       }
      }
     }
    } else {
	BITSET(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, HW);
    }
   }
	PUBLIC(RO(Proc))->CPU.OnLine++;
	BITCLR(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, OS);

	MatchPeerForUpService(&PUBLIC(RO(Proc))->Service, cpu);

   if (PUBLIC(RO(Proc))->Features.ACPI_CPPC == 1) {
	Read_ACPI_CPPC_Registers(cpu, NULL);
   }

	/* Start the collect timer dedicated to this CPU iff not STR resuming */
#ifdef CONFIG_PM_SLEEP
   if (CoreFreqK.ResumeFromSuspend == false)
#endif /* CONFIG_PM_SLEEP */
   {
    if (Arch[PUBLIC(RO(Proc))->ArchID].Timer != NULL) {
	Arch[PUBLIC(RO(Proc))->ArchID].Timer(cpu);
    }
    if ((BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED) == 0)
     && (Arch[PUBLIC(RO(Proc))->ArchID].Start != NULL)) {
		smp_call_function_single(cpu,
					Arch[PUBLIC(RO(Proc))->ArchID].Start,
					NULL, 0);
    }
   }
#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
   if (PUBLIC(RO(Proc))->Registration.Driver.CPUidle & REGISTRATION_ENABLE) {
	struct cpuidle_device *device = per_cpu_ptr(CoreFreqK.IdleDevice, cpu);
	if (device != NULL) {
		if (device->registered == 0) {
			device->cpu = cpu;
			cpuidle_register_device(device);
		}
	}
   }
#endif /* CONFIG_CPU_IDLE */

#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
#endif /* CONFIG_CPU_FREQ */

	return 0;
  } else
	return -EINVAL;
}

static int CoreFreqK_HotPlug_CPU_Offline(unsigned int cpu)
{
  if (cpu < PUBLIC(RO(Proc))->CPU.Count)
  {	/*		Stop the associated collect timer.		*/
    if ((BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, CREATED) == 1)
     && (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED) == 1)
     && (Arch[PUBLIC(RO(Proc))->ArchID].Stop != NULL)) {
	smp_call_function_single(cpu,
				Arch[PUBLIC(RO(Proc))->ArchID].Stop,
				NULL, 1);
    }
	PUBLIC(RO(Proc))->CPU.OnLine--;
	BITSET(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, OS);

	/*		Seek for an alternate Service Processor.	*/
#ifdef CONFIG_PM_SLEEP
   if (CoreFreqK.ResumeFromSuspend == false)
#endif /* CONFIG_PM_SLEEP */
   {
    if ((cpu == PUBLIC(RO(Proc))->Service.Core)
     || (cpu == PUBLIC(RO(Proc))->Service.Thread))
    {
	MatchPeerForDownService(&PUBLIC(RO(Proc))->Service, cpu);

     if (PUBLIC(RO(Proc))->Service.Core != cpu)
     {
	const unsigned int alt = PUBLIC(RO(Proc))->Service.Core;

      if (BITVAL(PRIVATE(OF(Core, AT(alt)))->Join.TSM, CREATED) == 1)
      {
	if ((BITVAL(PRIVATE(OF(Core, AT(alt)))->Join.TSM, STARTED) == 1)
	 && (Arch[PUBLIC(RO(Proc))->ArchID].Stop != NULL)) {
		smp_call_function_single(alt,
					Arch[PUBLIC(RO(Proc))->ArchID].Stop,
					NULL, 1);
	}
	if ((BITVAL(PRIVATE(OF(Core, AT(alt)))->Join.TSM, STARTED) == 0)
	 && (Arch[PUBLIC(RO(Proc))->ArchID].Start != NULL)) {
		smp_call_function_single(alt,
					Arch[PUBLIC(RO(Proc))->ArchID].Start,
					NULL, 0);
	}
      }
     }
    } else if ((cpu == PUBLIC(RO(Proc))->Service.Hybrid)
	&& (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.Hybrid))
    {
	PUBLIC(RO(Proc))->Service.Hybrid = Seek_Topology_Hybrid_Core(cpu);
    }
   }
#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
#endif /* CONFIG_CPU_FREQ */
	return 0;
  } else
	return -EINVAL;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
static int CoreFreqK_HotPlug(	struct notifier_block *nfb,
				unsigned long action,
				void *hcpu)
{
	unsigned int cpu = (unsigned long) hcpu, rc = 0;

	switch (action) {
	case CPU_ONLINE:
	case CPU_DOWN_FAILED:
/*TODO	case CPU_ONLINE_FROZEN: 					*/
		rc = CoreFreqK_HotPlug_CPU_Online(cpu);
		break;
	case CPU_DOWN_PREPARE:
/*TODO	case CPU_DOWN_PREPARE_FROZEN:					*/
		rc = CoreFreqK_HotPlug_CPU_Offline(cpu);
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}

static struct notifier_block CoreFreqK_notifier_block = {
	.notifier_call = CoreFreqK_HotPlug,
};
#endif /* KERNEL_VERSION(4, 10, 0) */
#endif /* CONFIG_HOTPLUG_CPU */

void SMBIOS_Collect(void)
{
#ifdef CONFIG_DMI
	struct {
		enum dmi_field field;
		char *recipient;
	} dmi_collect[] = {
		{ DMI_BIOS_VENDOR,	PUBLIC(RO(Proc))->SMB.BIOS.Vendor    },
		{ DMI_BIOS_VERSION,	PUBLIC(RO(Proc))->SMB.BIOS.Version   },
		{ DMI_BIOS_DATE,	PUBLIC(RO(Proc))->SMB.BIOS.Release   },
		{ DMI_SYS_VENDOR,	PUBLIC(RO(Proc))->SMB.System.Vendor  },
		{ DMI_PRODUCT_NAME,	PUBLIC(RO(Proc))->SMB.Product.Name   },
		{ DMI_PRODUCT_VERSION,	PUBLIC(RO(Proc))->SMB.Product.Version},
		{ DMI_PRODUCT_SERIAL,	PUBLIC(RO(Proc))->SMB.Product.Serial },
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
		{ DMI_PRODUCT_SKU,	PUBLIC(RO(Proc))->SMB.Product.SKU    },
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
		{ DMI_PRODUCT_FAMILY,	PUBLIC(RO(Proc))->SMB.Product.Family },
#endif
		{ DMI_BOARD_VENDOR,	PUBLIC(RO(Proc))->SMB.Board.Vendor   },
		{ DMI_BOARD_NAME,	PUBLIC(RO(Proc))->SMB.Board.Name     },
		{ DMI_BOARD_VERSION,	PUBLIC(RO(Proc))->SMB.Board.Version  },
		{ DMI_BOARD_SERIAL,	PUBLIC(RO(Proc))->SMB.Board.Serial   }
	};
	size_t count = sizeof(dmi_collect) / sizeof(dmi_collect[0]), idx;

	for (idx = 0; idx < count; idx++) {
		const char *pInfo = dmi_get_system_info(dmi_collect[idx].field);
		if ((pInfo != NULL) && (strlen(pInfo) > 0)) {
			StrCopy(dmi_collect[idx].recipient, pInfo, MAX_UTS_LEN);
		}
	}
#endif /* CONFIG_DMI */
}

#ifdef CONFIG_DMI
char *SMBIOS_String(const struct dmi_header *dh, u8 id)
{
	char *pStr = (char *) dh;
	pStr += dh->length;
	while (id > 1 && *pStr) {
		pStr += strlen(pStr);
		pStr++;
		id--;
	}
	if (!*pStr) {
		return NULL;
	}
	return pStr;
}

#define safe_strim(pStr)	(strim(pStr == NULL ? "" : pStr))

void SMBIOS_Entries(const struct dmi_header *dh, void *priv)
{
	size_t *count = (size_t*) priv;
    switch (dh->type) {
    case DMI_ENTRY_PHYS_MEM_ARRAY:
	{
		const struct SMBIOS16 *entry = (struct SMBIOS16*) dh;

		StrFormat(PUBLIC(RO(Proc))->SMB.Phys.Memory.Array, MAX_UTS_LEN,
			"Number Of Devices:%d\\Maximum Capacity:%lld kilobytes",
			entry->number_devices,
			entry->maximum_capacity >= 0x80000000 ?
			entry->extended_capacity : entry->maximum_capacity);
	}
	break;
    case DMI_ENTRY_MEM_DEVICE:
	{
		const struct SMBIOS17 *entry = (struct SMBIOS17*) dh;
	  if ((entry->length > 0x1a) && (entry->size > 0))
	  {
	    if ((*count) < MC_MAX_DIMM)
	    {
		const char *locator[2] = {
			safe_strim(SMBIOS_String(dh, entry->device_locator_id)),
			safe_strim(SMBIOS_String(dh, entry->bank_locator_id))
		};
		const size_t len[2] = {
			strlen(locator[0]) > 0 ? strlen(locator[0]) : 0,
			strlen(locator[1]) > 0 ? strlen(locator[1]) : 0
		}, prop = (len[0] + len[1]) > 0 ? (len[0] + len[1]) : 1;

		const int ratio[2] = {
			DIV_ROUND_CLOSEST(len[0] * (MAX_UTS_LEN - (1+1)), prop),
			DIV_ROUND_CLOSEST(len[1] * (MAX_UTS_LEN - (1+1)), prop)
		};
		StrFormat(PUBLIC(RO(Proc))->SMB.Memory.Locator[(*count)],
			MAX_UTS_LEN, "%.*s\\%.*s",
			ratio[0], locator[0],
			ratio[1], locator[1]);

		StrCopy(PUBLIC(RO(Proc))->SMB.Memory.Manufacturer[(*count)],
			safe_strim(SMBIOS_String(dh, entry->manufacturer_id)),
			MAX_UTS_LEN);

		StrCopy(PUBLIC(RO(Proc))->SMB.Memory.PartNumber[(*count)],
			safe_strim(SMBIOS_String(dh, entry->part_number_id)),
			MAX_UTS_LEN);
	    }
	  }
		(*count) = (*count) + 1;
	}
	break;
    }
}
#undef safe_strim
#endif /* CONFIG_DMI */

void SMBIOS_Decoder(void)
{
#ifdef CONFIG_DMI
	size_t count = 0;
	dmi_walk(SMBIOS_Entries, &count);
#endif /* CONFIG_DMI */
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2, 0)
static char *CoreFreqK_DevNode(const struct device *dev, umode_t *mode)
#else
static char *CoreFreqK_DevNode(struct device *dev, umode_t *mode)
#endif
{
	UNUSED(dev);

	if (mode != NULL) {
		(*mode) = 0600 ; /*	Device access is crw------	*/
	}
	return NULL;
}

static void CoreFreqK_Empty_Func_Level_Down(void)
{
}

static void CoreFreqK_Alloc_Features_Level_Down(void)
{
	pr_notice("CoreFreq: Unload\n");
}

static int CoreFreqK_Alloc_Features_Level_Up(INIT_ARG *pArg)
{
	pArg->Features = kmalloc(sizeof(FEATURES), GFP_KERNEL);
	if (pArg->Features == NULL)
	{
		return -ENOMEM;
	} else {
		memset(pArg->Features, 0, sizeof(FEATURES));
	}
	return 0;
}

#define CoreFreqK_Query_Features_Level_Down CoreFreqK_Empty_Func_Level_Down

static int CoreFreqK_Query_Features_Level_Up(INIT_ARG *pArg)
{
	int rc = 0;

	if ((CPU_Count > 0) && (CPU_Count <= CORE_COUNT)
	 && (CPU_Count <= NR_CPUS)
	 && ((ServiceProcessor == -1) || (ServiceProcessor >= CPU_Count)))
	{	/*	Force Service to a Core with allocated memory	*/
		ServiceProcessor = CPU_Count - 1;
	}
	if (ServiceProcessor == -1)
	{	/*	Query features on any processor.		*/
		pArg->localProcessor = get_cpu(); /* TODO(preempt_disable) */
		Query_Features(pArg);
		put_cpu();	/* TODO(preempt_enable) */
		rc = pArg->rc;
	} else { /*	Query features on User selected processor.	*/
		if (ServiceProcessor >= 0)
		{
			pArg->localProcessor = ServiceProcessor;
			if ((rc = smp_call_function_single(pArg->localProcessor,
							Query_Features,
							pArg, 1)) == 0)
			{
				rc = pArg->rc;
			}
		} else {
			rc = -ENXIO;
		}
	}
	if (rc == 0)
	{
		switch (CPU_Count) {
		case -1:
		    {	/* Rely on the operating system's cpu counting. */
			unsigned int OS_Count = num_present_cpus();
			if (pArg->SMT_Count != OS_Count) {
				pArg->SMT_Count = OS_Count;
			}
		    }
			break;
		default:
			if ((CPU_Count > 0) && (CPU_Count <= CORE_COUNT)
			 && (CPU_Count <= NR_CPUS))
			{ /* Hardware probing unless User override value */
				pArg->SMT_Count = (unsigned int) CPU_Count;
			}
			break;
		}
	} else {
		rc = -ENXIO;
	}
	return rc;
}

static void CoreFreqK_Alloc_Device_Level_Down(void)
{
	unregister_chrdev_region(CoreFreqK.mkdev, 1);
}

static int CoreFreqK_Alloc_Device_Level_Up(INIT_ARG *pArg)
{
	UNUSED(pArg);

	CoreFreqK.kcdev = cdev_alloc();
	CoreFreqK.kcdev->ops = &CoreFreqK_fops;
	CoreFreqK.kcdev->owner = THIS_MODULE;

	if (alloc_chrdev_region(&CoreFreqK.nmdev, 0, 1, DRV_FILENAME) >= 0)
	{
		return 0;
	} else {
		return -EBUSY;
	}
}

static void CoreFreqK_Make_Device_Level_Down(void)
{
	cdev_del(CoreFreqK.kcdev);
}

static int CoreFreqK_Make_Device_Level_Up(INIT_ARG *pArg)
{
	UNUSED(pArg);

	CoreFreqK.Major = MAJOR(CoreFreqK.nmdev);
	CoreFreqK.mkdev = MKDEV(CoreFreqK.Major, 0);

	if (cdev_add(CoreFreqK.kcdev, CoreFreqK.mkdev, 1) >= 0)
	{
		return 0;
	} else {
		return -EBUSY;
	}
}

static void CoreFreqK_Create_Device_Level_Down(void)
{
	device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
	class_destroy(CoreFreqK.clsdev);
}

static int CoreFreqK_Create_Device_Level_Up(INIT_ARG *pArg)
{
	struct device *tmpDev;
	UNUSED(pArg);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	CoreFreqK.clsdev = class_create(DRV_DEVNAME);
#else
	CoreFreqK.clsdev = class_create(THIS_MODULE, DRV_DEVNAME);
#endif
	CoreFreqK.clsdev->pm = COREFREQ_PM_OPS;
	CoreFreqK.clsdev->devnode = CoreFreqK_DevNode;

	if ((tmpDev = device_create(	CoreFreqK.clsdev, NULL,
					CoreFreqK.mkdev, NULL,
					DRV_DEVNAME)) != NULL)
	{
		return 0;
	} else {
		return -EBUSY;
	}
}

static void CoreFreqK_Alloc_Public_Level_Down(void)
{
	if (PUBLIC() != NULL)
	{
		kfree(PUBLIC());
	}
}

static int CoreFreqK_Alloc_Public_Level_Up(INIT_ARG *pArg)
{
	const size_t	coreSizeRO = sizeof(CORE_RO*) * pArg->SMT_Count,
			coreSizeRW = sizeof(CORE_RW*) * pArg->SMT_Count,
			publicSize = sizeof(KPUBLIC),
			alloc_size = publicSize + coreSizeRO + coreSizeRW;

	if (((PUBLIC() = kmalloc(alloc_size, GFP_KERNEL)) != NULL))
	{
		void *addr;

		memset(PUBLIC(), 0, alloc_size);

		addr = (void*) PUBLIC();
		addr = addr + publicSize;
		PUBLIC(RO(Core)) = (CORE_RO**) addr;

		addr = (void*) PUBLIC(RO(Core));
		addr = addr + coreSizeRO;
		PUBLIC(RW(Core)) = (CORE_RW**) addr;

		return 0;
	} else{
		return -ENOMEM;
	}
}

static void CoreFreqK_Alloc_Private_Level_Down(void)
{
	if (PRIVATE() != NULL)
	{
		kfree(PRIVATE());
	}
}

static int CoreFreqK_Alloc_Private_Level_Up(INIT_ARG *pArg)
{
	const unsigned long
		privCoreSize = sizeof(struct PRIV_CORE_ST *) * pArg->SMT_Count,
		privateSize = sizeof(KPRIVATE) + privCoreSize;

	if (((PRIVATE() = kmalloc(privateSize, GFP_KERNEL)) != NULL))
	{
		memset(PRIVATE(), 0, privateSize);

		return 0;
	} else {
		return -ENOMEM;
	}
}

static void CoreFreqK_Alloc_Processor_RO_Level_Down(void)
{
	if (PUBLIC(RO(Proc)) != NULL)
	{
		kfree(PUBLIC(RO(Proc)));
	}
}

static int CoreFreqK_Alloc_Processor_RO_Level_Up(INIT_ARG *pArg)
{
	const unsigned long procSize = ROUND_TO_PAGES(sizeof(PROC_RO));
	UNUSED(pArg);

	if ( (PUBLIC(RO(Proc)) = kmalloc(procSize, GFP_KERNEL)) != NULL)
	{
		memset(PUBLIC(RO(Proc)), 0, procSize);

		return 0;
	} else {
		return -ENOMEM;
	}
}

static void CoreFreqK_Alloc_Processor_RW_Level_Down(void)
{
	if (PUBLIC(RW(Proc)) != NULL)
	{
		kfree(PUBLIC(RW(Proc)));
	}
}

static int CoreFreqK_Alloc_Processor_RW_Level_Up(INIT_ARG *pArg)
{
	const unsigned long procSize = ROUND_TO_PAGES(sizeof(PROC_RW));
	UNUSED(pArg);

	if ( (PUBLIC(RW(Proc)) = kmalloc(procSize, GFP_KERNEL)) != NULL)
	{
		memset(PUBLIC(RW(Proc)), 0, procSize);

		return 0;
	} else {
		return -ENOMEM;
	}
}

#define CoreFreqK_Scale_And_Compute_Level_Down CoreFreqK_Empty_Func_Level_Down

static int CoreFreqK_Scale_And_Compute_Level_Up(INIT_ARG *pArg)
{
	SET_FOOTPRINT(PUBLIC(RO(Proc))->FootPrint,	\
					MAX_FREQ_HZ,	\
					CORE_COUNT,	\
					TASK_ORDER,	\
					COREFREQ_MAJOR, \
					COREFREQ_MINOR, \
					COREFREQ_REV	);

	PUBLIC(RO(Proc))->CPU.Count = pArg->SMT_Count;
	/* PreCompute SysGate memory allocation. */
	PUBLIC(RO(Proc))->Gate.ReqMem.Size = sizeof(SYSGATE_RO);

	PUBLIC(RO(Proc))->Gate.ReqMem.Order = \
				get_order(PUBLIC(RO(Proc))->Gate.ReqMem.Size);

	PUBLIC(RO(Proc))->Registration.AutoClock = AutoClock;
	PUBLIC(RO(Proc))->Registration.Experimental = Experimental;

	Compute_Interval();

	memcpy(&PUBLIC(RO(Proc))->Features, pArg->Features, sizeof(FEATURES));

	/* Initialize default uArch's codename with the CPUID brand.	*/
	Arch[GenuineArch].Architecture.Brand[0] = \
				PUBLIC(RO(Proc))->Features.Info.Vendor.ID;
	/* Initialize with any hypervisor found so far.			*/
	PUBLIC(RO(Proc))->HypervisorID = pArg->HypervisorID;
	return 0;
}

static void CoreFreqK_Alloc_Public_Cache_Level_Down(void)
{
	if (PUBLIC(OF(Cache)) != NULL)
	{
		kmem_cache_destroy(PUBLIC(OF(Cache)));
	}
}

static int CoreFreqK_Alloc_Public_Cache_Level_Up(INIT_ARG *pArg)
{
	const unsigned long cacheSize = KMAX( ROUND_TO_PAGES(sizeof(CORE_RO)),
					      ROUND_TO_PAGES(sizeof(CORE_RW)) );
	UNUSED(pArg);

	if ( (PUBLIC(OF(Cache)) = kmem_cache_create(	"corefreqk-pub",
							cacheSize, 0,
							SLAB_HWCACHE_ALIGN,
							NULL ) ) != NULL)
	{
		return 0;
	} else {
		return -ENOMEM;
	}
}

static void CoreFreqK_Alloc_Private_Cache_Level_Down(void)
{
	if (PRIVATE(OF(Cache)) != NULL)
	{
		kmem_cache_destroy(PRIVATE(OF(Cache)));
	}
}

static int CoreFreqK_Alloc_Private_Cache_Level_Up(INIT_ARG *pArg)
{
	const unsigned long joinSize = \
				ROUND_TO_PAGES(sizeof(struct PRIV_CORE_ST));
	UNUSED(pArg);

	if ( (PRIVATE(OF(Cache)) = kmem_cache_create(	"corefreqk-priv",
							joinSize, 0,
							SLAB_HWCACHE_ALIGN,
							NULL ) ) != NULL)
	{
		return 0;
	} else {
		return -ENOMEM;
	}
}

static void CoreFreqK_Alloc_Per_CPU_Level_Down(void)
{
	unsigned int cpu;
    for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
    {
	if (PUBLIC(OF(Cache)) != NULL)
	{
	    if (PUBLIC(RO(Core, AT(cpu))) != NULL) {
		kmem_cache_free(PUBLIC(OF(Cache)), PUBLIC(RO(Core, AT(cpu))));
	    }
	    if (PUBLIC(RW(Core, AT(cpu))) != NULL) {
		kmem_cache_free(PUBLIC(OF(Cache)), PUBLIC(RW(Core, AT(cpu))));
	    }
	}
	if (PRIVATE(OF(Cache)) != NULL)
	{
	    if (PRIVATE(OF(Core, AT(cpu))) != NULL) {
		kmem_cache_free(PRIVATE(OF(Cache)), PRIVATE(OF(Core, AT(cpu))));
	    }
	}
    }
}

static int CoreFreqK_Alloc_Per_CPU_Level_Up(INIT_ARG *pArg)
{
	const unsigned long cacheSize = KMAX( ROUND_TO_PAGES(sizeof(CORE_RO)),
					      ROUND_TO_PAGES(sizeof(CORE_RW)) );
	const unsigned long joinSize = \
				ROUND_TO_PAGES(sizeof(struct PRIV_CORE_ST));
	unsigned int cpu;
	int rc = 0;
	UNUSED(pArg);

	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++)
	{
		void *kcache = NULL;

		kcache = kmem_cache_alloc(PUBLIC(OF(Cache)), GFP_KERNEL);
		if (kcache != NULL) {
			memset(kcache, 0, cacheSize);
			PUBLIC(RO(Core, AT(cpu))) = kcache;
		} else {
			rc = -ENOMEM;
			break;
		}
		kcache = kmem_cache_alloc(PUBLIC(OF(Cache)), GFP_KERNEL);
		if (kcache != NULL) {
			memset(kcache, 0, cacheSize);
			PUBLIC(RW(Core, AT(cpu))) = kcache;
		} else {
			rc = -ENOMEM;
			break;
		}
		kcache = kmem_cache_alloc(PRIVATE(OF(Cache)), GFP_KERNEL);
		if (kcache != NULL) {
			memset(kcache, 0, joinSize);
			PRIVATE(OF(Core, AT(cpu))) = kcache;
		} else {
			rc = -ENOMEM;
			break;
		}
	    if (rc == 0) {
		BITCLR(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		PUBLIC(RO(Core, AT(cpu)))->Bind = cpu;
	    }
	}
	return rc;
}

static void CoreFreqK_Ignition_Level_Down(void)
{
	CoreFreqK_UnRegister_ClockSource();
	CoreFreqK_UnRegister_Governor();
	CoreFreqK_UnRegister_CPU_Freq();
	CoreFreqK_UnRegister_CPU_Idle();
	CoreFreqK_UnRegister_NMI();

#ifdef CONFIG_HOTPLUG_CPU
    #if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
	unregister_hotcpu_notifier(&CoreFreqK_notifier_block);
    #else /* KERNEL_VERSION(4, 10, 0) */
    if ( !(PUBLIC(RO(Proc))->Registration.HotPlug < 0) )
    {
	cpuhp_remove_state_nocalls(PUBLIC(RO(Proc))->Registration.HotPlug);
    }
    #endif /* KERNEL_VERSION(4, 10, 0) */
#endif /* CONFIG_HOTPLUG_CPU */

	Controller_Stop(1);
	Controller_Exit();

    if (PUBLIC(OF(Gate)) != NULL)
    {
	free_pages_exact(PUBLIC(OF(Gate)), PUBLIC(RO(Proc))->Gate.ReqMem.Size);
    }
}

static int CoreFreqK_Ignition_Level_Up(INIT_ARG *pArg)
{
	/*	Is an architecture identifier requested by user ?	*/
	if ( (ArchID != -1) && (ArchID >= 0) && (ArchID < ARCHITECTURES) )
	{
		PUBLIC(RO(Proc))->ArchID = ArchID;
	} else {
		PUBLIC(RO(Proc))->ArchID = SearchArchitectureID();
	}
	/*	Set the uArch's name with the first found codename	*/
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		CodeName[Arch[PUBLIC(RO(Proc))->ArchID].Architecture.CN],
		CODENAME_LEN);

	StrCopy(PUBLIC(RO(Proc))->Features.Info.Brand,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture.Brand[0],
		BRAND_SIZE);
	/*	Check if the Processor is actually virtualized ?	*/
	#ifdef CONFIG_XEN
	if (xen_pv_domain() || xen_hvm_domain())
	{
		if (PUBLIC(RO(Proc))->Features.Std.ECX.Hyperv == 0) {
			PUBLIC(RO(Proc))->Features.Std.ECX.Hyperv = 1;
		}
		PUBLIC(RO(Proc))->HypervisorID = HYPERV_XEN;
	}
	#endif /* CONFIG_XEN */

	PUBLIC(RO(Proc))->thermalFormula = \
				Arch[PUBLIC(RO(Proc))->ArchID].thermalFormula;

	PUBLIC(RO(Proc))->voltageFormula = \
				Arch[PUBLIC(RO(Proc))->ArchID].voltageFormula;

	PUBLIC(RO(Proc))->powerFormula = \
				Arch[PUBLIC(RO(Proc))->ArchID].powerFormula;

	CoreFreqK_Thermal_Scope(ThermalScope);
	CoreFreqK_Voltage_Scope(VoltageScope);
	CoreFreqK_Power_Scope(PowerScope);

	/*	Copy various SMBIOS data [version 3.2]			*/
	SMBIOS_Collect();
	SMBIOS_Decoder();

	/*	Initialize the CoreFreq controller			*/
	Controller_Init();

	MatchPeerForDefaultService(	&PUBLIC(RO(Proc))->Service,
					pArg->localProcessor );

	/*	Register the Idle & Frequency sub-drivers		*/
	CoreFreqK_Register_CPU_Idle();
	CoreFreqK_Register_CPU_Freq();
	CoreFreqK_Register_Governor();

	if (NMI_Disable == 0) {
		CoreFreqK_Register_NMI();
	}

	pr_info("CoreFreq(%u:%d:%d):"		\
		" Processor [%2X%1X_%1X%1X]"	\
		" Architecture [%s] %3s [%u/%u]\n",
		PUBLIC(RO(Proc))->Service.Core,
		PUBLIC(RO(Proc))->Service.Thread,
		PUBLIC(RO(Proc))->Service.Hybrid,
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily,
		PUBLIC(RO(Proc))->Features.Std.EAX.Family,
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel,
		PUBLIC(RO(Proc))->Features.Std.EAX.Model,
		PUBLIC(RO(Proc))->Architecture,
		PUBLIC(RO(Proc))->Features.HTT_Enable ? "SMT" : "CPU",
		PUBLIC(RO(Proc))->CPU.OnLine,
		PUBLIC(RO(Proc))->CPU.Count);

	PUBLIC(RO(Proc))->Registration.PCI = \
		CoreFreqK_ProbePCI(Arch[PUBLIC(RO(Proc))->ArchID].PCI_ids,
				CoreFreqK_ResetChip, CoreFreqK_AppendChip) == 0;

	Controller_Start(0);

#ifdef CONFIG_HOTPLUG_CPU
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)
	/*	Always returns zero (kernel/notifier.c)			*/
	PUBLIC(RO(Proc))->Registration.HotPlug = \
			register_hotcpu_notifier(&CoreFreqK_notifier_block);
	#else	/*	Continue with or without cpu hot-plugging.	*/
	PUBLIC(RO(Proc))->Registration.HotPlug = \
			cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
						"corefreqk/cpu:online",
						CoreFreqK_HotPlug_CPU_Online,
						CoreFreqK_HotPlug_CPU_Offline);
	#endif /* KERNEL_VERSION(4, 6, 0) */
#endif /* CONFIG_HOTPLUG_CPU */

	#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
	#endif /* CONFIG_CPU_FREQ */

	return 0;
}

#define CoreFreqK_User_Ops_Level_Down CoreFreqK_Empty_Func_Level_Down

static int CoreFreqK_User_Ops_Level_Up(INIT_ARG *pArg)
{
	UNUSED(pArg);

  if (Register_ClockSource == COREFREQ_TOGGLE_ON)
  {
	Controller_Stop(1);
	Controller_Start(1);
	CoreFreqK_Register_ClockSource(PUBLIC(RO(Proc))->Service.Core);
  }
  if (Ratio_HWP_Count > 0)
  {
	CLOCK_ARG clockMod;
	enum RATIO_BOOST boost;

	Controller_Stop(1);
    for (boost = 0; boost < Ratio_HWP_Count; boost++)
    {
      if (Ratio_HWP[boost] >= 0)
      {
	long rc = RC_SUCCESS;

	switch (boost) {
	case BOOST(HWP_MIN) - BOOST(HWP_MIN):
	    if (Arch[PUBLIC(RO(Proc))->ArchID].ClockMod) {
		clockMod.Ratio = Ratio_HWP[boost];
		clockMod.cpu = -1;
		clockMod.NC = CLOCK_MOD_HWP_MIN;
		rc = Arch[PUBLIC(RO(Proc))->ArchID].ClockMod(&clockMod);
	    #ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	    #endif
	    }
		break;
	case BOOST(HWP_MAX) - BOOST(HWP_MIN):
	    if (Arch[PUBLIC(RO(Proc))->ArchID].ClockMod) {
		clockMod.Ratio = Ratio_HWP[boost];
		clockMod.cpu = -1;
		clockMod.NC = CLOCK_MOD_HWP_MAX;
		rc = Arch[PUBLIC(RO(Proc))->ArchID].ClockMod(&clockMod);
	    #ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	    #endif
	    }
		break;
	case BOOST(HWP_TGT) - BOOST(HWP_MIN):
	    if (Arch[PUBLIC(RO(Proc))->ArchID].ClockMod) {
		clockMod.Ratio = Ratio_HWP[boost];
		clockMod.cpu = -1;
		clockMod.NC = CLOCK_MOD_HWP_TGT;
		rc = Arch[PUBLIC(RO(Proc))->ArchID].ClockMod(&clockMod);
	    #ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	    #endif
	    }
		break;
	default:
		rc = -RC_UNIMPLEMENTED;
		break;
	};
	if (rc < RC_SUCCESS) {
		pr_warn("CoreFreq: "					\
			"'Ratio_HWP' at #%d Execution failure code %ld\n",
			boost, rc);
	}
      }
    }
	Controller_Start(1);

	RESET_ARRAY(Ratio_HWP, Ratio_HWP_Count, -1);
	Ratio_HWP_Count = 0;
  }
  if (Ratio_Boost_Count > 0)
  {
	CLOCK_ARG clockMod;
	enum RATIO_BOOST boost;

	Controller_Stop(1);
    for (boost = 0; boost < Ratio_Boost_Count; boost++)
    {
      if (Ratio_Boost[boost] >= 0)
      {
	long rc = RC_SUCCESS;

	switch (boost) {
	case BOOST(1C) - BOOST(1C) ... BOOST(1C) - BOOST(18C):
	    if (Arch[PUBLIC(RO(Proc))->ArchID].TurboClock) {
		clockMod.Ratio = Ratio_Boost[boost];
		clockMod.cpu = -1;
		clockMod.NC = BOOST(SIZE) - BOOST(18C) - boost;
		rc = Arch[PUBLIC(RO(Proc))->ArchID].TurboClock(&clockMod);
	    }
		break;
	default:
		rc = -RC_UNIMPLEMENTED;
		break;
	};
	if (rc < RC_SUCCESS) {
		pr_warn("CoreFreq: "					\
			"'Ratio_Boost' at #%d Execution failure code %ld\n",
			boost, rc);
	}
      }
    }
    if (TurboBoost_Enable_Count == 2) {
	TurboBoost_Enable[0] = TurboBoost_Enable[1];
    }
	Controller_Start(1);

	RESET_ARRAY(Ratio_Boost, Ratio_Boost_Count, -1);
	Ratio_Boost_Count = 0;
  }
  if (Ratio_PPC >= 0)
  {
	long rc = RC_SUCCESS;

    if (Arch[PUBLIC(RO(Proc))->ArchID].ClockMod) {
	CLOCK_ARG clockMod={.Ratio = Ratio_PPC, .cpu = -1, .NC = CLOCK_MOD_TGT};

	Controller_Stop(1);
	rc = Arch[PUBLIC(RO(Proc))->ArchID].ClockMod(&clockMod);
	Controller_Start(0);
    } else {
	rc = -RC_UNIMPLEMENTED;
    }
    if (rc < RC_SUCCESS) {
	pr_warn("CoreFreq: 'Ratio_PPC' Execution failure code %ld\n", rc);
    }
	Ratio_PPC = -1;
  }
	return 0;
}

enum RUN_LEVEL {
	Alloc_Features_Level,
	Query_Features_Level,
	Alloc_Device_Level,
	Make_Device_Level,
	Create_Device_Level,
	Alloc_Public_Level,
	Alloc_Private_Level,
	Alloc_Processor_RO_Level,
	Alloc_Processor_RW_Level,
	Scale_And_Compute_Level,
	Alloc_Public_Cache_Level,
	Alloc_Private_Cache_Level,
	Alloc_Per_CPU_Level,
	Ignition_Level,
	User_Ops_Level,
	Running_Level
};

static enum RUN_LEVEL RunLevel = Alloc_Features_Level;

#define COREFREQ_RUN( _level, _action )	CoreFreqK_##_level##_##_action

static void CoreFreqK_ShutDown(void)
{
	void (*LevelFunc[Running_Level])(void) = {
		COREFREQ_RUN(Alloc_Features_Level, Down),
		COREFREQ_RUN(Query_Features_Level, Down),
		COREFREQ_RUN(Alloc_Device_Level, Down),
		COREFREQ_RUN(Make_Device_Level, Down),
		COREFREQ_RUN(Create_Device_Level, Down),
		COREFREQ_RUN(Alloc_Public_Level, Down),
		COREFREQ_RUN(Alloc_Private_Level, Down),
		COREFREQ_RUN(Alloc_Processor_RO_Level, Down),
		COREFREQ_RUN(Alloc_Processor_RW_Level, Down),
		COREFREQ_RUN(Scale_And_Compute_Level, Down),
		COREFREQ_RUN(Alloc_Public_Cache_Level, Down),
		COREFREQ_RUN(Alloc_Private_Cache_Level, Down),
		COREFREQ_RUN(Alloc_Per_CPU_Level, Down),
		COREFREQ_RUN(Ignition_Level, Down),
		COREFREQ_RUN(User_Ops_Level, Down)
	};

	do
	{
		RunLevel--;
		LevelFunc[RunLevel]();
	} while (RunLevel != Alloc_Features_Level) ;
}

static int CoreFreqK_StartUp(void)
{
	int (*LevelFunc[Running_Level])(INIT_ARG *pArg) = {
		COREFREQ_RUN(Alloc_Features_Level, Up),
		COREFREQ_RUN(Query_Features_Level, Up),
		COREFREQ_RUN(Alloc_Device_Level, Up),
		COREFREQ_RUN(Make_Device_Level, Up),
		COREFREQ_RUN(Create_Device_Level, Up),
		COREFREQ_RUN(Alloc_Public_Level, Up),
		COREFREQ_RUN(Alloc_Private_Level, Up),
		COREFREQ_RUN(Alloc_Processor_RO_Level, Up),
		COREFREQ_RUN(Alloc_Processor_RW_Level, Up),
		COREFREQ_RUN(Scale_And_Compute_Level, Up),
		COREFREQ_RUN(Alloc_Public_Cache_Level, Up),
		COREFREQ_RUN(Alloc_Private_Cache_Level, Up),
		COREFREQ_RUN(Alloc_Per_CPU_Level, Up),
		COREFREQ_RUN(Ignition_Level, Up),
		COREFREQ_RUN(User_Ops_Level, Up)
	};
	INIT_ARG iArg = {
		.Features = NULL,
		.SMT_Count = 0, .localProcessor = 0, .rc = 0
	};
	int rc = 0;

	do
	{
		rc = LevelFunc[RunLevel](&iArg);
		RunLevel++;
	} while (RunLevel != Running_Level && rc == 0) ;

	/*	Free any initialization memory allocation.		*/
	if (iArg.Features != NULL)
	{
		kfree(iArg.Features);
	}
	return rc;
}

#undef COREFREQ_RUN
#undef CoreFreqK_Scale_And_Compute_Level_Down
#undef CoreFreqK_Query_Features_Level_Down
#undef COREFREQ_PM_OPS

static int __init CoreFreqK_Init(void)
{
	int rc = CoreFreqK_StartUp();

	if (rc != 0) {
		CoreFreqK_ShutDown();
	}
	return rc;
}

static void __exit CoreFreqK_Exit(void)
{
	CoreFreqK_ShutDown();
}

module_init(CoreFreqK_Init)
module_exit(CoreFreqK_Exit)
