/*
 * CoreFreq
 * Copyright (C) 2015-2021 CYRIL INGENIERIE
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
#include <asm/msr.h>
#include <asm/nmi.h>
#ifdef CONFIG_XEN
#include <xen/xen.h>
#endif /* CONFIG_XEN */
#include <asm/mwait.h>

#include "bitasm.h"
#include "amdmsr.h"
#include "intelmsr.h"
#include "coretypes.h"
#include "corefreq-api.h"
#include "corefreqk.h"

#ifdef CONFIG_AMD_NB
#include <asm/amd_nb.h>
#endif

MODULE_AUTHOR ("CYRIL INGENIERIE <labs[at]cyring[dot]fr>");
MODULE_DESCRIPTION ("CoreFreq Processor Driver");
MODULE_SUPPORTED_DEVICE ("Intel,AMD,HYGON");
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

static SERVICE_PROC DefaultSMT = {.Proc = -1};

static unsigned short RDPMC_Enable = 0;
module_param(RDPMC_Enable, ushort, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(RDPMC_Enable, "Enable RDPMC bit in CR4 register");

static unsigned short NMI_Disable = 1;
module_param(NMI_Disable, ushort, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(NMI_Disable, "Disable the NMI Handler");

static int Override_SubCstate_Depth = 0;
static unsigned short Override_SubCstate[CPUIDLE_STATE_MAX] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
module_param_array(	Override_SubCstate,ushort, &Override_SubCstate_Depth, \
			S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Override_SubCstate, "Override Sub C-States");

static signed short PkgCStateLimit = -1;
module_param(PkgCStateLimit, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PkgCStateLimit, "Package C-State Limit");

static signed short IOMWAIT_Enable = -1;
module_param(IOMWAIT_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(IOMWAIT_Enable, "I/O MWAIT Redirection Enable");

static signed short CStateIORedir = -1;
module_param(CStateIORedir, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(CStateIORedir, "Power Mgmt IO Redirection C-State");

static signed short SpeedStep_Enable = -1;
module_param(SpeedStep_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SpeedStep_Enable, "Enable SpeedStep");

static signed short C1E_Enable = -1;
module_param(C1E_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1E_Enable, "Enable SpeedStep C1E");

static signed short TurboBoost_Enable = -1;
module_param(TurboBoost_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TurboBoost_Enable, "Enable Turbo Boost");

static signed short C3A_Enable = -1;
module_param(C3A_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C3A_Enable, "Enable C3 Auto Demotion");

static signed short C1A_Enable = -1;
module_param(C1A_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1A_Enable, "Enable C3 Auto Demotion");

static signed short C3U_Enable = -1;
module_param(C3U_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C3U_Enable, "Enable C3 UnDemotion");

static signed short C1U_Enable = -1;
module_param(C1U_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1U_Enable, "Enable C1 UnDemotion");

static signed short CC6_Enable = -1;
module_param(CC6_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(CC6_Enable, "Enable Core C6 State");

static signed short PC6_Enable = -1;
module_param(PC6_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PC6_Enable, "Enable Package C6 State");

static signed short ODCM_Enable = -1;
module_param(ODCM_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ODCM_Enable, "Enable On-Demand Clock Modulation");

static signed short ODCM_DutyCycle = -1;
module_param(ODCM_DutyCycle, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ODCM_DutyCycle, "ODCM DutyCycle [0-7] | [0-14]");

static signed short PowerMGMT_Unlock = -1;
module_param(PowerMGMT_Unlock, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerMGMT_Unlock, "Unlock Power Management");

static signed short PowerPolicy = -1;
module_param(PowerPolicy, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerPolicy, "Power Policy Preference [0-15]");

static signed int PState_FID = -1;
module_param(PState_FID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PState_FID, "P-State Frequency Id");

static signed int PState_VID = -1;
module_param(PState_VID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PState_VID, "P-State Voltage Id");

static signed short HWP_Enable = -1;
module_param(HWP_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HWP_Enable, "Hardware-Controlled Performance States");

static signed short HWP_EPP = -1;
module_param(HWP_EPP, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HWP_EPP, "Energy Performance Preference");

static signed short HDC_Enable = -1;
module_param(HDC_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HDC_Enable, "Hardware Duty Cycling");

static signed short R2H_Disable = -1;
module_param(R2H_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(R2H_Disable, "Disable Race to Halt");

static unsigned int Clear_Events = 0;
module_param(Clear_Events, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
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

static signed short Mech_IBRS = -1;
module_param(Mech_IBRS, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_IBRS, "Mitigation Mechanism IBRS");

static signed short Mech_STIBP = -1;
module_param(Mech_STIBP, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_STIBP, "Mitigation Mechanism STIBP");

static signed short Mech_SSBD = -1;
module_param(Mech_SSBD, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_SSBD, "Mitigation Mechanism SSBD");

static signed short Mech_IBPB = -1;
module_param(Mech_IBPB, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_IBPB, "Mitigation Mechanism IBPB");

static signed short Mech_L1D_FLUSH = -1;
module_param(Mech_L1D_FLUSH, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_L1D_FLUSH, "Mitigation Mechanism Cache L1D Flush");

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
	}
#endif /* CONFIG_CPU_FREQ */
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
	return ((unsigned int) r64);
}

unsigned long long CoreFreqK_Read_CS_From_Invariant_TSC(struct clocksource *cs)
{
	unsigned long long TSC __attribute__ ((aligned (8)));
	RDTSCP64(TSC);
	return (TSC);
}

unsigned long long CoreFreqK_Read_CS_From_Variant_TSC(struct clocksource *cs)
{
	unsigned long long TSC __attribute__ ((aligned (8)));
	RDTSC64(TSC);
	return (TSC);
}

static struct clocksource CoreFreqK_CS = {
	.name	= "corefreq",
	.rating = 250,
	.mask	= CLOCKSOURCE_MASK(64),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
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
	return (rc);
}

static long CoreFreqK_Register_ClockSource(unsigned int cpu)
{
	long rc = -EINVAL;
    if (Register_ClockSource == 1)
    {
	unsigned long long Freq_Hz;
	unsigned int Freq_KHz;
	int rx;

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

	rx = clocksource_register_khz(&CoreFreqK_CS, Freq_KHz);
	switch ( rx ) {
	default:
		/* Fallthrough */
	case -EBUSY:
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_DISABLE;
		rc = (long) rx;
		break;
	case 0:
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_ENABLE;
		rc = RC_SUCCESS;

	pr_warn("%s: Freq_KHz[%u] Kernel CPU_KHZ[%u] TSC_KHZ[%u]\n" \
		"LPJ[%lu] mask[%llx] mult[%u] shift[%u]\n",
		CoreFreqK_CS.name, Freq_KHz, cpu_khz, tsc_khz, loops_per_jiffy,
		CoreFreqK_CS.mask, CoreFreqK_CS.mult, CoreFreqK_CS.shift);

		break;
	}
    } else {
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_DISABLE;
    }
	return (rc);
}

void VendorFromCPUID(	char *pVendorID, unsigned int *pLargestFunc,
			unsigned int *pCRC, enum HYPERVISOR *pHypervisor,
			unsigned long leaf, unsigned long subLeaf )
{
    struct {
		char		*vendorID;
		size_t		vendorLen;
		unsigned int	mfrCRC;
		enum HYPERVISOR hypervisor;
    } mfrTbl[] = {
	{VENDOR_INTEL ,__builtin_strlen(VENDOR_INTEL) ,CRC_INTEL ,  BARE_METAL},
	{VENDOR_AMD   ,__builtin_strlen(VENDOR_AMD)   ,CRC_AMD   ,  BARE_METAL},
	{VENDOR_HYGON ,__builtin_strlen(VENDOR_HYGON) ,CRC_HYGON ,  BARE_METAL},
	{VENDOR_KVM   ,__builtin_strlen(VENDOR_KVM)   ,CRC_KVM   ,  HYPERV_KVM},
	{VENDOR_VBOX  ,__builtin_strlen(VENDOR_VBOX)  ,CRC_VBOX  , HYPERV_VBOX},
	{VENDOR_KBOX  ,__builtin_strlen(VENDOR_KBOX)  ,CRC_KBOX  , HYPERV_KBOX},
      {VENDOR_VMWARE ,__builtin_strlen(VENDOR_VMWARE),CRC_VMWARE,HYPERV_VMWARE},
      {VENDOR_HYPERV ,__builtin_strlen(VENDOR_HYPERV),CRC_HYPERV,HYPERV_HYPERV}
    };
	unsigned int eax = 0x0, ebx = 0x0, ecx = 0x0, edx = 0x0; /*DWORD Only!*/

	__asm__ volatile
	(
		"movq	%4, %%rax	\n\t"
		"movq	%5, %%rcx	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (eax),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		: "ir" (leaf),
		  "ir" (subLeaf)
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	pVendorID[ 0] = ebx;
	pVendorID[ 1] = (ebx >> 8);
	pVendorID[ 2] = (ebx >> 16);
	pVendorID[ 3] = (ebx >> 24);
	pVendorID[ 4] = edx;
	pVendorID[ 5] = (edx >> 8);
	pVendorID[ 6] = (edx >> 16);
	pVendorID[ 7] = (edx >> 24);
	pVendorID[ 8] = ecx;
	pVendorID[ 9] = (ecx >> 8);
	pVendorID[10] = (ecx >> 16);
	pVendorID[11] = (ecx >> 24);
	pVendorID[12] = '\0';

	(*pLargestFunc) = eax;

    for (eax = 0; eax < sizeof(mfrTbl) / sizeof(mfrTbl[0]); eax++) {
	if (!strncmp(pVendorID, mfrTbl[eax].vendorID, mfrTbl[eax].vendorLen))
	{
		(*pCRC) = mfrTbl[eax].mfrCRC;
		(*pHypervisor) = mfrTbl[eax].hypervisor;

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
	return (id);
}

void BrandCleanup(char *pBrand, char inOrder[])
{
	unsigned long ix, jx;
	for (jx = 0; jx < BRAND_LENGTH; jx++) {
		if (inOrder[jx] != 0x20) {
			break;
		}
	}
	for (ix = 0; jx < BRAND_LENGTH; jx++) {
		if (!(inOrder[jx] == 0x20 && inOrder[jx + 1] == 0x20)) {
			pBrand[ix++] = inOrder[jx];
		}
	}
}

void BrandFromCPUID(char *buffer)
{
	BRAND Brand;
	unsigned long ix;
	unsigned int jx , px = 0;

	for (ix = 0; ix < 3; ix++)
	{	__asm__ volatile
		(
			"movq	%4,    %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r"  (Brand.AX),
			  "=r"  (Brand.BX),
			  "=r"  (Brand.CX),
			  "=r"  (Brand.DX)
			: "r"   (0x80000002LU + ix)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		for (jx = 0; jx < 4; jx++, px++) {
			buffer[px     ] = Brand.AX.Chr[jx];
			buffer[px +  4] = Brand.BX.Chr[jx];
			buffer[px +  8] = Brand.CX.Chr[jx];
			buffer[px + 12] = Brand.DX.Chr[jx];
		}
		px += BRAND_PART;
	}
}

unsigned int Intel_Brand(char *pBrand, char buffer[])
{
	unsigned int ix, frequency = 0, multiplier = 0;

	BrandFromCPUID(buffer);

	for (ix = 0; ix < (BRAND_LENGTH - 2); ix++)
	{
		if ((buffer[ix + 1] == 'H') && (buffer[ix + 2] == 'z')) {
			switch (buffer[ix]) {
			case 'M':
					multiplier = 1;
				break;
			case 'G':
					multiplier = 1000;
				break;
			case 'T':
					multiplier = 1000000;
				break;
			}
			break;
		}
	}
	if (multiplier > 0)
	{
	    if (buffer[ix - 3] == '.') {
		frequency  = (int) (buffer[ix - 4] - '0') * multiplier;
		frequency += (int) (buffer[ix - 2] - '0') * (multiplier / 10);
		frequency += (int) (buffer[ix - 1] - '0') * (multiplier / 100);
	    } else {
		frequency  = (int) (buffer[ix - 4] - '0') * 1000;
		frequency += (int) (buffer[ix - 3] - '0') * 100;
		frequency += (int) (buffer[ix - 2] - '0') * 10;
		frequency += (int) (buffer[ix - 1] - '0');
		frequency *= frequency;
	    }
	}

	BrandCleanup(pBrand, buffer);

	return (frequency);
}

/* Retreive the Processor(BSP) features. */
static void Query_Features(void *pArg)
{	/* Must have x86 CPUID 0x0, 0x1, and Intel CPUID 0x4 */
	INIT_ARG *iArg = (INIT_ARG *) pArg;
	unsigned int eax = 0x0, ebx = 0x0, ecx = 0x0, edx = 0x0; /*DWORD Only!*/
	enum HYPERVISOR hypervisor = HYPERV_NONE;

	VendorFromCPUID(iArg->Features->Info.Vendor.ID,
			&iArg->Features->Info.LargestStdFunc,
			&iArg->Features->Info.Vendor.CRC,
			&hypervisor,
			0x0LU, 0x0LU);

	if (hypervisor != BARE_METAL) {
		iArg->rc = -ENXIO;
		return;
	}

	__asm__ volatile
	(
		"movq	$0x1,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->Std.EAX),
		  "=r" (iArg->Features->Std.EBX),
		  "=r" (iArg->Features->Std.ECX),
		  "=r" (iArg->Features->Std.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

    if (iArg->Features->Info.LargestStdFunc >= 0x5)
    {
	__asm__ volatile
	(
		"movq	$0x5,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->MWait.EAX),
		  "=r" (iArg->Features->MWait.EBX),
		  "=r" (iArg->Features->MWait.ECX),
		  "=r" (iArg->Features->MWait.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
      switch (Override_SubCstate_Depth) {
      case 8:
	iArg->Features->MWait.EDX.SubCstate_MWAIT7 = Override_SubCstate[7];
	/* Fallthrough */
      case 7:
	iArg->Features->MWait.EDX.SubCstate_MWAIT6 = Override_SubCstate[6];
	/* Fallthrough */
      case 6:
	iArg->Features->MWait.EDX.SubCstate_MWAIT5 = Override_SubCstate[5];
	/* Fallthrough */
      case 5:
	iArg->Features->MWait.EDX.SubCstate_MWAIT4 = Override_SubCstate[4];
	/* Fallthrough */
      case 4:
	iArg->Features->MWait.EDX.SubCstate_MWAIT3 = Override_SubCstate[3];
	/* Fallthrough */
      case 3:
	iArg->Features->MWait.EDX.SubCstate_MWAIT2 = Override_SubCstate[2];
	/* Fallthrough */
      case 2:
	iArg->Features->MWait.EDX.SubCstate_MWAIT1 = Override_SubCstate[1];
	/* Fallthrough */
      case 1:
	iArg->Features->MWait.EDX.SubCstate_MWAIT0 = Override_SubCstate[0];
	break;
      };
    }
    if (iArg->Features->Info.LargestStdFunc >= 0x6)
    {
	__asm__ volatile
	(
		"movq	$0x6,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->Power.EAX),
		  "=r" (iArg->Features->Power.EBX),
		  "=r" (iArg->Features->Power.ECX),
		  "=r" (iArg->Features->Power.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
    }
    if (iArg->Features->Info.LargestStdFunc >= 0x7)
    {
	__asm__ volatile
	(
		"movq	$0x7,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx    \n\t"
		"xorq	%%rcx, %%rcx    \n\t"
		"xorq	%%rdx, %%rdx    \n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->ExtFeature.EAX),
		  "=r" (iArg->Features->ExtFeature.EBX),
		  "=r" (iArg->Features->ExtFeature.ECX),
		  "=r" (iArg->Features->ExtFeature.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	if (iArg->Features->ExtFeature.EAX.MaxSubLeaf >= 1)
	{
		__asm__ volatile
		(
			"movq	$0x7,  %%rax	\n\t"
			"movq	$0x1,  %%rcx    \n\t"
			"xorq	%%rbx, %%rbx    \n\t"
			"xorq	%%rdx, %%rdx    \n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (iArg->Features->ExtFeature_Leaf1.EAX),
			  "=r" (iArg->Features->ExtFeature_Leaf1.EBX),
			  "=r" (iArg->Features->ExtFeature_Leaf1.ECX),
			  "=r" (iArg->Features->ExtFeature_Leaf1.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
    }
	/* Must have 0x80000000,0x80000001,0x80000002,0x80000003,0x80000004 */
	__asm__ volatile
	(
		"movq	$0x80000000, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->Info.LargestExtFunc),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile
	(
		"movq	$0x80000001, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (eax),
		  "=r" (ebx),
		  "=r" (iArg->Features->ExtInfo.ECX),
		  "=r" (iArg->Features->ExtInfo.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
    if (iArg->Features->Info.LargestExtFunc >= 0x80000007)
    {
	__asm__ volatile
	(
		"movq	$0x80000007, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->AdvPower.EAX),
		  "=r" (iArg->Features->AdvPower.EBX),
		  "=r" (iArg->Features->AdvPower.ECX),
		  "=r" (iArg->Features->AdvPower.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
    }
    if (iArg->Features->Info.LargestExtFunc >= 0x80000008)
    {
	__asm__ volatile
	(
		"movq	$0x80000008, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->leaf80000008.EAX),
		  "=r" (iArg->Features->leaf80000008.EBX),
		  "=r" (iArg->Features->leaf80000008.ECX),
		  "=r" (iArg->Features->leaf80000008.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
    }
	/* Reset the performance features bits (present is zero) */
	iArg->Features->PerfMon.EBX.CoreCycles    = 1;
	iArg->Features->PerfMon.EBX.InstrRetired  = 1;
	iArg->Features->PerfMon.EBX.RefCycles     = 1;
	iArg->Features->PerfMon.EBX.LLC_Ref       = 1;
	iArg->Features->PerfMon.EBX.LLC_Misses    = 1;
	iArg->Features->PerfMon.EBX.BranchRetired = 1;
	iArg->Features->PerfMon.EBX.BranchMispred = 1;

	/* Per Vendor features */
    if (iArg->Features->Info.Vendor.CRC == CRC_INTEL)
    {
	__asm__ volatile
	(
		"movq	$0x4,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (eax),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	iArg->SMT_Count = (eax >> 26) & 0x3f;
	iArg->SMT_Count++;

	if (iArg->Features->Info.LargestStdFunc >= 0xa)
	{
		__asm__ volatile
		(
			"movq	$0xa,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (iArg->Features->PerfMon.EAX),
			  "=r" (iArg->Features->PerfMon.EBX),
			  "=r" (iArg->Features->PerfMon.ECX),
			  "=r" (iArg->Features->PerfMon.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	/*	Extract the factory frequency from the brand string.	*/
	iArg->Features->Factory.Freq = Intel_Brand( iArg->Features->Info.Brand,
							iArg->Brand );

    } else if ( (iArg->Features->Info.Vendor.CRC == CRC_AMD)
	||	(iArg->Features->Info.Vendor.CRC == CRC_HYGON) )
    {	/*	Specified as Core Performance 64 bits General Counters. */
	iArg->Features->PerfMon.EAX.MonWidth = 64;

	if (iArg->Features->ExtInfo.ECX.PerfCore)
	{
		iArg->Features->PerfMon.EAX.MonCtrs = 6;
	} else {
		iArg->Features->PerfMon.EAX.MonCtrs = 4;
	}
	/* Fix the Performance Counters. Use Intel bits as AMD placeholder */
	iArg->Features->PerfMon.EDX.FixWidth = 64;

	if	( iArg->Features->Power.ECX.HCF_Cap
		| iArg->Features->AdvPower.EDX.EffFrqRO )
	{
		iArg->Features->PerfMon.EBX.CoreCycles = 0;
		iArg->Features->PerfMon.EBX.RefCycles  = 0;
		iArg->Features->PerfMon.EDX.FixCtrs += 2;
	}
	if (iArg->Features->ExtInfo.ECX.PerfLLC)
	{ /* PerfCtrExtLLC: Last Level Cache performance counter extensions */
		iArg->Features->PerfMon.EBX.LLC_Ref = 0;
	}
	if (iArg->Features->Info.LargestExtFunc >= 0x80000008)
	{
		iArg->SMT_Count = iArg->Features->leaf80000008.ECX.NC + 1;
		/* Add the Retired Instructions Perf Counter to the Fixed set */
		if (iArg->Features->leaf80000008.EBX.IRPerf)
		{
			iArg->Features->PerfMon.EBX.InstrRetired = 0;
			iArg->Features->PerfMon.EDX.FixCtrs++;
		}
	}
	else if (iArg->Features->Std.EDX.HTT)
	{
		iArg->SMT_Count = iArg->Features->Std.EBX.Max_SMT_ID;
	} else {
		iArg->SMT_Count = 1;
	}
	BrandFromCPUID(iArg->Brand);
	BrandCleanup(iArg->Features->Info.Brand, iArg->Brand);
    }
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
	#define THIS_LPJ	this_cpu_read(cpu_info.loops_per_jiffy)
#else
	#define THIS_LPJ	loops_per_jiffy
#endif

#define COMPUTE_LPJ(BCLK_Hz, COF)	( (BCLK_Hz * COF) / HZ )

#if defined(DELAY_TSC) && (DELAY_TSC == 1)
FEAT_MSG("udelay() built with TSC implementation")
#define CLOCK_TSC( CYCLES, _TIMER, CTR )				\
({									\
	__asm__ volatile						\
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
	);								\
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
	memset(D, 0, 2 * OCCURRENCES);
	for (loop = 0; loop < OCCURRENCES; loop++)
		for (what = 0; what < 2; what++) {
			D[what][loop]	= pCompute->TSC[what][loop].V[1]
					- pCompute->TSC[what][loop].V[0];
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

	return (pCompute->Clock);
}

void ClockToHz(CLOCK *clock)
{
	clock->Hz  = clock->Q * 1000000L;
	clock->Hz += clock->R * PRECISION;
}

/* [Genuine Intel] */
CLOCK BaseClock_GenuineIntel(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};

	if ((PUBLIC(RO(Proc))->Features.Factory.Freq > 0) && (ratio > 0))
	{
		clock.Hz=(PUBLIC(RO(Proc))->Features.Factory.Freq * 1000000L)
			/ ratio;

		clock.Q = PUBLIC(RO(Proc))->Features.Factory.Freq / ratio;

		clock.R = (PUBLIC(RO(Proc))->Features.Factory.Freq % ratio)
			* PRECISION;
	}
	return (clock);
};

/* [Authentic AMD] */
CLOCK BaseClock_AuthenticAMD(unsigned int ratio)
{	/* For AMD Families 0Fh, 10h up to 16h */
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
	return (clock);
};

/* [Core] */
CLOCK BaseClock_Core(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch (FSB.Bus_Speed) {
	case 0b101: {
		clock.Q = 100;
		clock.R = 0;
		};
		break;
	case 0b001: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b011: {
		clock.Q = 166;
		clock.R = 6666;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Core2] */
CLOCK BaseClock_Core2(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch (FSB.Bus_Speed) {
	case 0b101: {
		clock.Q = 100;
		clock.R = 0;
		}
		break;
	case 0b001: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b011: {
		clock.Q = 166;
		clock.R = 6666;
		}
		break;
	case 0b010: {
		clock.Q = 200;
		clock.R = 0;
		}
		break;
	case 0b000: {
		clock.Q = 266;
		clock.R = 6666;
		}
		break;
	case 0b100: {
		clock.Q = 333;
		clock.R = 3333;
		}
		break;
	case 0b110: {
		clock.Q = 400;
		clock.R = 0;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Atom] */
CLOCK BaseClock_Atom(unsigned int ratio)
{
	CLOCK clock = {.Q = 83, .R = 0};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch (FSB.Bus_Speed) {
	case 0b111: {
		clock.Q = 83;
		clock.R = 2000;
		}
		break;
	case 0b101: {
		clock.Q = 99;
		clock.R = 8400;
		}
		break;
	case 0b001: {
		clock.Q = 133;
		clock.R = 2000;
		}
		break;
	case 0b011: {
		clock.Q = 166;
		clock.R = 4000;
		}
		break;
	default: {
		clock.Q = 83;
		clock.R = 2000;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Airmont] */
CLOCK BaseClock_Airmont(unsigned int ratio)
{
	CLOCK clock = {.Q = 87, .R = 5};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch (FSB.Bus_Speed) {
	case 0b000: {
		clock.Q = 83;
		clock.R = 3333;
		}
		break;
	case 0b001: {
		clock.Q = 100;
		clock.R = 0000;
		}
		break;
	case 0b010: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b011: {
		clock.Q = 116;
		clock.R = 6666;
		}
		break;
	case 0b100: {
		clock.Q = 80;
		clock.R = 0000;
		}
		break;
	case 0b101: {
		clock.Q = 93;
		clock.R = 3333;
		}
		break;
	case 0b110: {
		clock.Q = 90;
		clock.R = 0000;
		}
		break;
	case 0b111: {
		clock.Q = 88;
		clock.R = 9000;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Silvermont] */
CLOCK BaseClock_Silvermont(unsigned int ratio)
{
	CLOCK clock = {.Q = 83, .R = 3};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch (FSB.Bus_Speed)
	{
	case 0b100: {
			clock.Q = 80;
			clock.R = 0;
		}
		break;
	case 0b000: {
			clock.Q = 83;
			clock.R = 3000;
		}
		break;
	case 0b001: {
			clock.Q = 100;
			clock.R = 0;
		}
		break;
	case 0b010: {
			clock.Q = 133;
			clock.R = 3333;
		}
		break;
	case 0b011: {
			clock.Q = 116;
			clock.R = 7000;
		}
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Nehalem] */
CLOCK BaseClock_Nehalem(unsigned int ratio)
{
	CLOCK clock = {.Q = 133, .R = 3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Westmere] */
CLOCK BaseClock_Westmere(unsigned int ratio)
{
	CLOCK clock = {.Q = 133, .R = 3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [SandyBridge] */
CLOCK BaseClock_SandyBridge(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [IvyBridge] */
CLOCK BaseClock_IvyBridge(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Haswell] */
CLOCK BaseClock_Haswell(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

/* [Skylake] */
CLOCK BaseClock_Skylake(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};

	if (PUBLIC(RO(Proc))->Features.Info.LargestStdFunc >= 0x16) {
		unsigned int eax = 0x0, ebx = 0x0, edx = 0x0, fsb = 0;
		__asm__ volatile
		(
			"movq	$0x16, %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (eax),
			  "=r" (ebx),
			  "=r" (fsb),
			  "=r" (edx)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		if (fsb > 0)
			clock.Q = fsb;
		else
			clock.Q = 100;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return (clock);
};

CLOCK BaseClock_AMD_Family_17h(unsigned int ratio)
{	/* Source: AMD PPR Family 17h § 1.4/ Table 11: REFCLK = 100 MHz */
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
	return (clock);
};

void Define_CPUID(CORE_RO *Core, const CPUID_STRUCT CpuIDforVendor[])
{	/*	Per vendor, define a CPUID dump table to query .	*/
	enum CPUID_ENUM i;
	for (i = 0; i < CPUID_MAX_FUNC; i++) {
		Core->CpuID[i].func = CpuIDforVendor[i].func;
		Core->CpuID[i].sub  = CpuIDforVendor[i].sub;
	}
}

void Cache_Topology(CORE_RO *Core)
{
	unsigned long level = 0x0;
	if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL) {
	    for (level = 0; level < CACHE_MAX_LEVEL; level++) {
		__asm__ volatile
		(
			"movq	$0x4,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"movq	%4,    %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (Core->T.Cache[level].AX),
			  "=r" (Core->T.Cache[level].BX),
			  "=r" (Core->T.Cache[level].Set),
			  "=r" (Core->T.Cache[level].DX)
			: "r" (level)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		if (!Core->T.Cache[level].Type)
			break;
	    }
	}
	else if ( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		||(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) )
	{
	    struct CACHE_INFO CacheInfo; /* Employ same Intel algorithm. */

	    if (PUBLIC(RO(Proc))->Features.Info.LargestExtFunc >= 0x80000005)
	    {
		Core->T.Cache[0].Level = 1;
		Core->T.Cache[0].Type  = 2;		/* Inst. */
		Core->T.Cache[1].Level = 1;
		Core->T.Cache[1].Type  = 1;		/* Data */

		/*	Fn8000_0005 L1 Data and Inst. caches.		*/
		__asm__ volatile
		(
			"movq	$0x80000005, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (CacheInfo.AX),
			  "=r" (CacheInfo.BX),
			  "=r" (CacheInfo.CX),
			  "=r" (CacheInfo.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		/* L1 Inst. */
		Core->T.Cache[0].Way  = CacheInfo.CPUID_0x80000005_L1I.Assoc;
		Core->T.Cache[0].Size = CacheInfo.CPUID_0x80000005_L1I.Size;
		/* L1 Data */
		Core->T.Cache[1].Way  = CacheInfo.CPUID_0x80000005_L1D.Assoc;
		Core->T.Cache[1].Size = CacheInfo.CPUID_0x80000005_L1D.Size;
	    }
	    if (PUBLIC(RO(Proc))->Features.Info.LargestExtFunc >= 0x80000006)
	    {
		Core->T.Cache[2].Level = 2;
		Core->T.Cache[2].Type  = 3;		/* Unified! */
		Core->T.Cache[3].Level = 3;
		Core->T.Cache[3].Type  = 3;

		/*	Fn8000_0006 L2 and L3 caches.			*/
		__asm__ volatile
		(
			"movq	$0x80000006, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (CacheInfo.AX),
			  "=r" (CacheInfo.BX),
			  "=r" (CacheInfo.CX),
			  "=r" (CacheInfo.DX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		/* L2 */
		Core->T.Cache[2].Way  = CacheInfo.CPUID_0x80000006_L2.Assoc;
		Core->T.Cache[2].Size = CacheInfo.CPUID_0x80000006_L2.Size;
		/* L3 */
		Core->T.Cache[3].Way  = CacheInfo.CPUID_0x80000006_L3.Assoc;
		Core->T.Cache[3].Size = CacheInfo.CPUID_0x80000006_L3.Size;
	    }
	}
}

unsigned int L3_SubCache_AMD_Piledriver(unsigned int bits)
{	/* Return the AMD Piledriver L3 Sub-Cache size in unit of 512 KB  */
	switch (bits) {
	case 0xc:
		return (4);
	case 0xd:
	case 0xe:
		return (2);
	default:
		return (0);
	}
}

static void Map_AMD_Topology(void *arg)
{
    if (arg != NULL)
    {
	CORE_RO *Core = (CORE_RO *) arg;

	struct CPUID_0x00000001_EBX leaf1_ebx = {0};

	CPUID_0x80000008 leaf80000008 = {
		.EAX = {0}, .EBX = {0}, .ECX = {0}, .EDX = {0}
	};

	Cache_Topology(Core);

	RDMSR(Core->T.Base, MSR_IA32_APICBASE);

	__asm__ volatile
	(
		"movq	$0x1,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%ebx, %0"
		: "=r" (leaf1_ebx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

	__asm__ volatile
	(
		"movq	$0x80000008, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%ecx, %0"
		: "=r" (leaf80000008.ECX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

	switch (PUBLIC(RO(Proc))->ArchID) {
	default:
	case AMD_Family_0Fh:	/* Legacy processor. */
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = 0;
		break;
	case AMD_Family_10h:
	    {
		CPUID_0x80000001 leaf80000001 = {
			.EAX = {0}, .EBX = {0}, .ECX = {{0}}, .EDX = {{0}}
		};

		__asm__ volatile
		(
			"movq	$0x80000001, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (leaf80000001.EAX),
			  "=r" (leaf80000001.EBX),
			  "=r" (leaf80000001.ECX),
			  "=r" (leaf80000001.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		Core->T.Cluster.Node = leaf80000001.ECX.NodeId;
	    }
		/* Fallthrough */
	case AMD_Family_11h:
	case AMD_Family_12h:
	case AMD_Family_14h:
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = leaf1_ebx.Init_APIC_ID
				  >> leaf80000008.ECX.ApicIdCoreIdSize;
		break;
	case AMD_Family_15h:
	    if ((PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel == 0x0)
	     && (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	     && (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf))
	    {
		L3_CACHE_PARAMETER L3;
		RDPCI(L3, PCI_AMD_L3_CACHE_PARAMETER);

	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize0);
	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize1);
	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize2);
	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize3);
	    }
		/* Fallthrough */
	case AMD_Family_16h:
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = leaf1_ebx.Init_APIC_ID
				  >> leaf80000008.ECX.ApicIdCoreIdSize;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID
				  - (Core->T.PackageID
					<< leaf80000008.ECX.ApicIdCoreIdSize);

	    if (PUBLIC(RO(Proc))->Features.ExtInfo.ECX.ExtApicId == 1)
	    {
		CPUID_0x8000001e leaf8000001e;

		__asm__ volatile
		(
			"movq	$0x8000001e, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (leaf8000001e.EAX),
			  "=r" (leaf8000001e.EBX),
			  "=r" (leaf8000001e.ECX),
			  "=r" (leaf8000001e.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		/*	TODO(Case of leaf8000001e.EAX.ExtApicId)	*/
		Core->T.Cluster.Node= leaf8000001e.ECX.NodeId;
		Core->T.Cluster.CMP = leaf8000001e.EBX.CompUnitId;
	    }
	    break;
	case AMD_Zen:
	case AMD_Zen_APU:
	case AMD_ZenPlus:
	case AMD_ZenPlus_APU:
	case AMD_Zen_APU_Rv2:
	case AMD_EPYC_Rome:
	case AMD_Zen2_CPK:
	case AMD_Zen2_APU:
	case AMD_Zen2_MTS:
	case AMD_Zen3_VMR:
	case AMD_Family_17h:
	case AMD_Family_18h:
	case AMD_Family_19h:
	    if (PUBLIC(RO(Proc))->Features.ExtInfo.ECX.ExtApicId == 1)
	    {
		struct CACHE_INFO CacheInfo = {
			.AX = 0, .BX = 0, .CX = 0, .DX = 0, .Size = 0
		};
		CPUID_0x8000001e leaf8000001e = {
			.EAX = {0}, .EBX = {{0}}, .ECX = {0}, .EDX = {0}
		};
		/* Fn8000_001D Cache Properties. */
		unsigned long idx, level[CACHE_MAX_LEVEL] = {1, 0, 2, 3};
		/* Skip one CDD on two with x48 and x64 SMT Threadripper */
		unsigned int factor	=	(leaf80000008.ECX.NC == 0x3f)
					||	(leaf80000008.ECX.NC == 0x2f);

		for (idx = 0; idx < CACHE_MAX_LEVEL; idx++ ) {
		    __asm__ volatile
		    (
			"movq	$0x8000001d, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"movq	%4,    %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (CacheInfo.AX),
			  "=r" (CacheInfo.BX),
			  "=r" (CacheInfo.CX),
			  "=r" (CacheInfo.DX)
			: "r" (idx)
			: "%rax", "%rbx", "%rcx", "%rdx"
		    );
			Core->T.Cache[level[idx]].WrBack = CacheInfo.WrBack;
			Core->T.Cache[level[idx]].Inclus = CacheInfo.Inclus;
		}
		/*	Fn8000_001E {ExtApic, Core, Node} Identifiers.	*/
		__asm__ volatile
		(
			"movq	$0x8000001e, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (leaf8000001e.EAX),
			  "=r" (leaf8000001e.EBX),
			  "=r" (leaf8000001e.ECX),
			  "=r" (leaf8000001e.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		Core->T.ApicID    = leaf8000001e.EAX.ExtApicId;
		Core->T.CoreID    = leaf8000001e.EBX.CoreId;
		Core->T.PackageID = leaf8000001e.ECX.NodeId;

		if (leaf8000001e.EBX.ThreadsPerCore > 0) {
			Core->T.ThreadID  = leaf8000001e.EAX.ExtApicId & 1;
		} else {
			Core->T.ThreadID  = 0;
		}

		Core->T.Cluster.Node=leaf8000001e.ECX.NodeId;
		Core->T.Cluster.CCD = (Core->T.CoreID >> 3) << factor;
		Core->T.Cluster.CCX = Core->T.CoreID >> 2;
	    } else {	/*	Fallback algorithm.			*/
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = leaf1_ebx.Init_APIC_ID
				  >> leaf80000008.ECX.ApicIdCoreIdSize;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID
				  - (Core->T.PackageID
					<< leaf80000008.ECX.ApicIdCoreIdSize);
		Core->T.ThreadID  = 0;
	    }
	    break;
	}
    }
}

/*
 Enumerate the topology of Processors, Cores and Threads
 Remark: Early single-core processors are not processed.
 Sources: Intel Software Developer's Manual vol 3A §8.9 /
	  Intel whitepaper: Detecting Hyper-Threading Technology /
*/
unsigned short FindMaskWidth(unsigned short maxCount)
{
	unsigned short maskWidth = 0, count = (maxCount - 1);

	if (BITBSR(count, maskWidth) == 0)
		maskWidth++;

	return (maskWidth);
}

static void Map_Intel_Topology(void *arg)
{
    if (arg != NULL)
    {
	CORE_RO *Core = (CORE_RO *) arg;
	unsigned short	SMT_Mask_Width, CORE_Mask_Width,
			SMT_Select_Mask, CORE_Select_Mask, PKG_Select_Mask;

	struct CPUID_0x00000001_EBX leaf1_ebx;

	struct
	{	/* CPUID 4 */
		unsigned int
		Type		:  5-0,
		Level		:  8-5,
		Init		:  9-8,
		Assoc		: 10-9,
		Unused		: 14-10,
		Cache_SMT_ID	: 26-14,
		Max_Core_ID	: 32-26;
	} leaf4_eax;

	RDMSR(Core->T.Base, MSR_IA32_APICBASE);

	__asm__ volatile
	(
		"movq	$0x1,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%ebx, %0"
		: "=r" (leaf1_ebx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

	if (PUBLIC(RO(Proc))->Features.Std.EDX.HTT) {
		SMT_Mask_Width = leaf1_ebx.Max_SMT_ID;

		__asm__ volatile
		(
			"movq	$0x4,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0"
			: "=r" (leaf4_eax)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);

		CORE_Mask_Width = leaf4_eax.Max_Core_ID + 1;
	} else {
		SMT_Mask_Width = 0;
		CORE_Mask_Width = 1;
	}

	if (CORE_Mask_Width != 0)
	   SMT_Mask_Width = FindMaskWidth(SMT_Mask_Width) / CORE_Mask_Width;

	SMT_Select_Mask   = ~((-1) << SMT_Mask_Width);

	CORE_Select_Mask  = (~((-1) << (CORE_Mask_Width + SMT_Mask_Width)))
			  ^ SMT_Select_Mask;

	PKG_Select_Mask   = (-1) << (CORE_Mask_Width + SMT_Mask_Width);

	Core->T.ThreadID  = leaf1_ebx.Init_APIC_ID & SMT_Select_Mask;

	Core->T.CoreID    = (leaf1_ebx.Init_APIC_ID & CORE_Select_Mask)
			  >> SMT_Mask_Width;

	Core->T.PackageID = (leaf1_ebx.Init_APIC_ID & PKG_Select_Mask)
			  >> (CORE_Mask_Width + SMT_Mask_Width);

	Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;

	Cache_Topology(Core);
    }
}

static void Map_Intel_Extended_Topology(void *arg)
{
    if (arg != NULL) {
	CORE_RO *Core = (CORE_RO *) arg;

	long	InputLevel = 0;
	int	NoMoreLevels = 0,
		SMT_Mask_Width = 0, SMT_Select_Mask = 0,
		CorePlus_Mask_Width = 0, CoreOnly_Select_Mask = 0,
		Package_Select_Mask = 0;

	CPUID_TOPOLOGY_LEAF ExtTopology = {
		.AX.Register = 0,
		.BX.Register = 0,
		.CX.Register = 0,
		.DX.Register = 0
	};

	RDMSR(Core->T.Base, MSR_IA32_APICBASE);

	do {
		__asm__ volatile
		(
			"movq	$0xb,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"movq	%4,    %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (ExtTopology.AX),
			  "=r" (ExtTopology.BX),
			  "=r" (ExtTopology.CX),
			  "=r" (ExtTopology.DX)
			: "r" (InputLevel)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		/*	Exit from the loop if the BX register equals 0 or
			if the requested level exceeds the level of a Core. */
		if (!ExtTopology.BX.Register || (InputLevel > LEVEL_CORE))
			NoMoreLevels = 1;
		else {
		    switch (ExtTopology.CX.Type) {
		    case LEVEL_THREAD: {
			SMT_Mask_Width   = ExtTopology.AX.SHRbits;

			SMT_Select_Mask  = ~((-1) << SMT_Mask_Width);

			Core->T.ThreadID = ExtTopology.DX.x2ApicID
					 & SMT_Select_Mask;
			}
			break;
		    case LEVEL_CORE: {
			CorePlus_Mask_Width  = ExtTopology.AX.SHRbits;

			CoreOnly_Select_Mask = (~((-1) << CorePlus_Mask_Width))
					     ^ SMT_Select_Mask;

			Core->T.CoreID	= (ExtTopology.DX.x2ApicID
						& CoreOnly_Select_Mask)
					>> SMT_Mask_Width;

			Package_Select_Mask = (-1) << CorePlus_Mask_Width;

			Core->T.PackageID = (ExtTopology.DX.x2ApicID
						& Package_Select_Mask)
					  >> CorePlus_Mask_Width;
			}
			break;
		    }
		}
		InputLevel++;
	} while (!NoMoreLevels);

	Core->T.ApicID = ExtTopology.DX.x2ApicID;

	Cache_Topology(Core);
    }
}

int Core_Topology(unsigned int cpu)
{
	int rc = smp_call_function_single(cpu,
		( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		||(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) ) ?
			Map_AMD_Topology
		: (PUBLIC(RO(Proc))->Features.Info.LargestStdFunc >= 0xb) ?
			Map_Intel_Extended_Topology : Map_Intel_Topology,
		PUBLIC(RO(Core, AT(cpu))), 1); /* Synchronous call. */

	if (	!rc
		&& (PUBLIC(RO(Proc))->Features.HTT_Enable == 0)
		&& (PUBLIC(RO(Core, AT(cpu)))->T.ThreadID > 0) )
	{
			PUBLIC(RO(Proc))->Features.HTT_Enable = 1;
	}
	return (rc);
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
	return (CountEnabledCPU);
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
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[
							pSpecific->CodeNameIdx
						].CodeName,
		CODENAME_LEN);
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
			return (pSpecific);
		}
	}
    }
	return (NULL);
}

int Intel_MaxBusRatio(PLATFORM_ID *PfID)
{
	struct SIGNATURE whiteList[] = {
		_Core_Conroe,		/* 06_0F */
		_Core_Penryn,		/* 06_17 */
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft, 	/* 06_27 */
		_Atom_Clover_Trail,	/* 06_35 */
		_Atom_Saltwell, 	/* 06_36 */
		_Silvermont_Bay_Trail,	/* 06_37 */
	};
	int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
	for (id = 0; id < ids; id++) {
		if ((whiteList[id].ExtFamily \
			== PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
		 && (whiteList[id].Family \
			== PUBLIC(RO(Proc))->Features.Std.EAX.Family)
		 && (whiteList[id].ExtModel \
			== PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
		 && (whiteList[id].Model \
			== PUBLIC(RO(Proc))->Features.Std.EAX.Model))
		{
			RDMSR((*PfID), MSR_IA32_PLATFORM_ID);
			return (0);
		}
	}
	return (-1);
}

void Intel_Core_Platform_Info(unsigned int cpu)
{
	PLATFORM_ID PfID = {.value = 0};
	PLATFORM_INFO PfInfo = {.value = 0};
	PERF_STATUS PerfStatus = {.value = 0};
	unsigned int ratio0 = 10, ratio1 = 10; /*Arbitrary values*/

	RDMSR(PfInfo, MSR_PLATFORM_INFO);
	if (PfInfo.value != 0) {
		ratio0 = PfInfo.MaxNonTurboRatio;
	}

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	if (PerfStatus.value != 0) {				/* §18.18.3.4 */
		if (PerfStatus.CORE.XE_Enable) {
			ratio1 = PerfStatus.CORE.MaxBusRatio;
		} else {
			if (Intel_MaxBusRatio(&PfID) == 0) {
				if (PfID.value != 0)
				{
					ratio1 = PfID.MaxBusRatio;
				}
			}
		}
	} else {
			if (Intel_MaxBusRatio(&PfID) == 0) {
				if (PfID.value != 0)
				{
					ratio1 = PfID.MaxBusRatio;
				}
			}
	}

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] =	KMIN(ratio0, ratio1);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] =	KMAX(ratio0, ratio1);
}

void Compute_Intel_Core_Burst(unsigned int cpu)
{
	PERF_STATUS PerfStatus = {.value = 0};
	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);

	if (PRIVATE(OF(Specific)) != NULL)
	{
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)]
				+ PRIVATE(OF(Specific))->Boost[0]
				+ PRIVATE(OF(Specific))->Boost[1];

	    if (PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] > 0) {
		PUBLIC(RO(Proc))->Features.SpecTurboRatio = 1;
	    }
	} else {	/* Is Processor half ratio or Burst capable ?	*/
	    if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	    {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] = \
			PerfStatus.value != 0 ? PerfStatus.CORE.MaxBusRatio
			: 2 + PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];

		PUBLIC(RO(Proc))->Features.SpecTurboRatio = 1;
	    } else {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] = 0;
		PUBLIC(RO(Proc))->Features.SpecTurboRatio = 0;
	    }
	}
}

PLATFORM_INFO Intel_Platform_Info(unsigned int cpu)
{
	PLATFORM_INFO PfInfo = {.value = 0};
	RDMSR(PfInfo, MSR_PLATFORM_INFO);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = PfInfo.MinimumRatio;
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = PfInfo.MaxNonTurboRatio;

	return (PfInfo);
}

typedef union {
	unsigned long long	value;
} TURBO_RATIO_CFG_MSR;

typedef union {
	TURBO_RATIO_CFG_MSR	MSR;
	TURBO_RATIO_CONFIG0	Cfg0;
	TURBO_RATIO_CONFIG1	Cfg1;
	TURBO_RATIO_CONFIG2	Cfg2;
} TURBO_CONFIG;

typedef struct {
	CLOCK_ARG	*pClockMod;
	TURBO_CONFIG	Config;
	long		rc;
} CLOCK_TURBO_ARG;

void Assign_8C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(8C)] = pConfig->Cfg0.MaxRatio_8C;
	pBoost[BOOST(7C)] = pConfig->Cfg0.MaxRatio_7C;
	pBoost[BOOST(6C)] = pConfig->Cfg0.MaxRatio_6C;
	pBoost[BOOST(5C)] = pConfig->Cfg0.MaxRatio_5C;
	pBoost[BOOST(4C)] = pConfig->Cfg0.MaxRatio_4C;
	pBoost[BOOST(3C)] = pConfig->Cfg0.MaxRatio_3C;
	pBoost[BOOST(2C)] = pConfig->Cfg0.MaxRatio_2C;
	pBoost[BOOST(1C)] = pConfig->Cfg0.MaxRatio_1C;
}

void Assign_15C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(15C)] = pConfig->Cfg1.IVB_EP.MaxRatio_15C;
	pBoost[BOOST(14C)] = pConfig->Cfg1.IVB_EP.MaxRatio_14C;
	pBoost[BOOST(13C)] = pConfig->Cfg1.IVB_EP.MaxRatio_13C;
	pBoost[BOOST(12C)] = pConfig->Cfg1.IVB_EP.MaxRatio_12C;
	pBoost[BOOST(11C)] = pConfig->Cfg1.IVB_EP.MaxRatio_11C;
	pBoost[BOOST(10C)] = pConfig->Cfg1.IVB_EP.MaxRatio_10C;
	pBoost[BOOST(9C) ] = pConfig->Cfg1.IVB_EP.MaxRatio_9C;
}

void Assign_16C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(16C)] = pConfig->Cfg1.HSW_EP.MaxRatio_16C;
	pBoost[BOOST(15C)] = pConfig->Cfg1.HSW_EP.MaxRatio_15C;
	pBoost[BOOST(14C)] = pConfig->Cfg1.HSW_EP.MaxRatio_14C;
	pBoost[BOOST(13C)] = pConfig->Cfg1.HSW_EP.MaxRatio_13C;
	pBoost[BOOST(12C)] = pConfig->Cfg1.HSW_EP.MaxRatio_12C;
	pBoost[BOOST(11C)] = pConfig->Cfg1.HSW_EP.MaxRatio_11C;
	pBoost[BOOST(10C)] = pConfig->Cfg1.HSW_EP.MaxRatio_10C;
	pBoost[BOOST(9C) ] = pConfig->Cfg1.HSW_EP.MaxRatio_9C;
}

void Assign_18C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(18C)] = pConfig->Cfg2.MaxRatio_18C;
	pBoost[BOOST(17C)] = pConfig->Cfg2.MaxRatio_17C;
}

void Assign_SKL_X_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(16C)] = pConfig->Cfg1.SKL_X.NUMCORE_7;
	pBoost[BOOST(15C)] = pConfig->Cfg1.SKL_X.NUMCORE_6;
	pBoost[BOOST(14C)] = pConfig->Cfg1.SKL_X.NUMCORE_5;
	pBoost[BOOST(13C)] = pConfig->Cfg1.SKL_X.NUMCORE_4;
	pBoost[BOOST(12C)] = pConfig->Cfg1.SKL_X.NUMCORE_3;
	pBoost[BOOST(11C)] = pConfig->Cfg1.SKL_X.NUMCORE_2;
	pBoost[BOOST(10C)] = pConfig->Cfg1.SKL_X.NUMCORE_1;
	pBoost[BOOST(9C) ] = pConfig->Cfg1.SKL_X.NUMCORE_0;
}

long For_All_Turbo_Clock(CLOCK_ARG *pClockMod, void (*ConfigFunc)(void *))
{
	long rc = RC_SUCCESS;
	unsigned int cpu = PUBLIC(RO(Proc))->CPU.Count;
    do {
	cpu--;	/* From last AP to BSP */

	if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)
	&& ((pClockMod->cpu == -1) || (pClockMod->cpu == cpu)))
	{
		CLOCK_TURBO_ARG ClockTurbo = {
			.pClockMod = pClockMod,
			.Config = {.MSR = {.value = 0}},
			.rc = RC_SUCCESS
		};

		smp_call_function_single(cpu, ConfigFunc, &ClockTurbo, 1);

		rc = ClockTurbo.rc;
	}
    } while ((cpu != 0) && (rc >= RC_SUCCESS)) ;

	return (rc);
}

static void Intel_Turbo_Cfg8C_PerCore(void *arg)
{
	CLOCK_TURBO_ARG *pClockCfg8C = (CLOCK_TURBO_ARG *) arg;

	RDMSR(pClockCfg8C->Config.Cfg0, MSR_TURBO_RATIO_LIMIT);

  if (pClockCfg8C->pClockMod != NULL)	/* Read-Only function called ?	*/
  {
	pClockCfg8C->rc = RC_SUCCESS;
    if (PUBLIC(RO(Proc))->Features.Turbo_Unlock)
    {
	unsigned short WrRd8C = 0;
    switch (pClockCfg8C->pClockMod->NC) {
    case 1:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_1C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_1C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 2:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_2C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_2C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 3:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_3C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_3C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 4:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_4C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_4C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 5:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_5C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_5C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 6:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_6C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_6C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 7:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_7C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_7C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    case 8:
      if (pClockCfg8C->pClockMod->cpu == -1) {
	pClockCfg8C->Config.Cfg0.MaxRatio_8C = pClockCfg8C->pClockMod->Ratio;
      } else {
	pClockCfg8C->Config.Cfg0.MaxRatio_8C += pClockCfg8C->pClockMod->Offset;
      }
	WrRd8C = 1;
	break;
    default:
	WrRd8C = 0;
	break;
    }
      if (WrRd8C) {
	WRMSR(pClockCfg8C->Config.Cfg0, MSR_TURBO_RATIO_LIMIT);
	RDMSR(pClockCfg8C->Config.Cfg0, MSR_TURBO_RATIO_LIMIT);
	pClockCfg8C->rc = RC_OK_COMPUTE;
      }
    } else {
	pClockCfg8C->rc = -RC_TURBO_PREREQ;
    }
  }
}

long Intel_Turbo_Config8C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg8C_PerCore);

	return (rc);
}

static void Intel_Turbo_Cfg15C_PerCore(void *arg)
{
	CLOCK_TURBO_ARG *pClockCfg15C = (CLOCK_TURBO_ARG *) arg;

	RDMSR(pClockCfg15C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);

  if (pClockCfg15C->pClockMod != NULL)
  {
	pClockCfg15C->rc = RC_SUCCESS;
    if (PUBLIC(RO(Proc))->Features.Turbo_Unlock)
    {
	unsigned short WrRd15C = 0;
    switch (pClockCfg15C->pClockMod->NC) {
    case 9:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_9C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_9C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    case 10:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_10C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_10C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    case 11:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_11C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_11C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    case 12:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_12C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_12C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    case 13:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_13C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_13C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    case 14:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_14C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_14C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    case 15:
      if (pClockCfg15C->pClockMod->cpu == -1) {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_15C = \
						pClockCfg15C->pClockMod->Ratio;
      } else {
	pClockCfg15C->Config.Cfg1.IVB_EP.MaxRatio_15C += \
						pClockCfg15C->pClockMod->Offset;
      }
	WrRd15C = 1;
	break;
    default:
	WrRd15C = 0;
	break;
    }
      if (WrRd15C) {
	WRMSR(pClockCfg15C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);
	RDMSR(pClockCfg15C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);
	pClockCfg15C->rc = RC_OK_COMPUTE;
      }
    } else {
	pClockCfg15C->rc = -RC_TURBO_PREREQ;
    }
  }
}

long Intel_Turbo_Config15C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg15C_PerCore);

	return (rc);
}

static void Intel_Turbo_Cfg16C_PerCore(void *arg)
{
	CLOCK_TURBO_ARG *pClockCfg16C = (CLOCK_TURBO_ARG *) arg;

	RDMSR(pClockCfg16C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);

  if (pClockCfg16C->pClockMod != NULL)
  {
	pClockCfg16C->rc = RC_SUCCESS;
    if (PUBLIC(RO(Proc))->Features.Turbo_Unlock)
    {
	unsigned short WrRd16C = 0;
    switch (pClockCfg16C->pClockMod->NC) {
    case 9:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_9C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_9C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 10:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_10C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_10C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 11:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_11C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_11C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 12:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_12C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_12C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 13:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_13C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_13C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 14:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_14C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_14C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 15:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_15C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_15C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 16:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_16C = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.HSW_EP.MaxRatio_16C += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    default:
	WrRd16C = 0;
	break;
    }
      if (WrRd16C) {
	WRMSR(pClockCfg16C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);
	RDMSR(pClockCfg16C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);
	pClockCfg16C->rc = RC_OK_COMPUTE;
      }
    } else {
	pClockCfg16C->rc = -RC_TURBO_PREREQ;
    }
  }
}

long Intel_Turbo_Config16C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg16C_PerCore);

	return (rc);
}

static void Intel_Turbo_Cfg18C_PerCore(void *arg)
{
	CLOCK_TURBO_ARG *pClockCfg18C = (CLOCK_TURBO_ARG *) arg;

	RDMSR(pClockCfg18C->Config.Cfg2, MSR_TURBO_RATIO_LIMIT2);

  if (pClockCfg18C->pClockMod != NULL)
  {
	pClockCfg18C->rc = RC_SUCCESS;
    if (PUBLIC(RO(Proc))->Features.Turbo_Unlock)
    {
	unsigned short WrRd18C = 0;
    switch (pClockCfg18C->pClockMod->NC) {
    case 17:
      if (pClockCfg18C->pClockMod->cpu == -1) {
	pClockCfg18C->Config.Cfg2.MaxRatio_17C = \
						pClockCfg18C->pClockMod->Ratio;
      } else {
	pClockCfg18C->Config.Cfg2.MaxRatio_17C += \
						pClockCfg18C->pClockMod->Offset;
      }
	WrRd18C = 1;
	break;
    case 18:
      if (pClockCfg18C->pClockMod->cpu == -1) {
	pClockCfg18C->Config.Cfg2.MaxRatio_18C = \
						pClockCfg18C->pClockMod->Ratio;
      } else {
	pClockCfg18C->Config.Cfg2.MaxRatio_18C += \
						pClockCfg18C->pClockMod->Offset;
      }
	WrRd18C = 1;
	break;
    default:
	WrRd18C = 0;
	break;
    }
      if (WrRd18C) {
	WRMSR(pClockCfg18C->Config.Cfg2, MSR_TURBO_RATIO_LIMIT2);
	RDMSR(pClockCfg18C->Config.Cfg2, MSR_TURBO_RATIO_LIMIT2);
	pClockCfg18C->rc = RC_OK_COMPUTE;
      }
    } else {
	pClockCfg18C->rc = -RC_TURBO_PREREQ;
    }
  }
}

long Intel_Turbo_Config18C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg18C_PerCore);

	return (rc);
}

static void Intel_Turbo_Cfg_SKL_X_PerCore(void *arg)
{
	CLOCK_TURBO_ARG *pClockCfg16C = (CLOCK_TURBO_ARG *) arg;

	RDMSR(pClockCfg16C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);

  if (pClockCfg16C->pClockMod != NULL)
  {
	pClockCfg16C->rc = RC_SUCCESS;
    if (PUBLIC(RO(Proc))->Features.Turbo_Unlock)
    {
	unsigned short WrRd16C = 0;
    switch (pClockCfg16C->pClockMod->NC) {
    case 9:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_0 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_0 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 10:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_1 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_1 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 11:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_2 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_2 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 12:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_3 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_3 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 13:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_4 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_4 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 14:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_5 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_5 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 15:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_6 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_6 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    case 16:
      if (pClockCfg16C->pClockMod->cpu == -1) {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_7 = \
						pClockCfg16C->pClockMod->Ratio;
      } else {
	pClockCfg16C->Config.Cfg1.SKL_X.NUMCORE_7 += \
						pClockCfg16C->pClockMod->Offset;
      }
	WrRd16C = 1;
	break;
    default:
	WrRd16C = 0;
	break;
    }
      if (WrRd16C) {
	WRMSR(pClockCfg16C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);
	RDMSR(pClockCfg16C->Config.Cfg1, MSR_TURBO_RATIO_LIMIT1);
	pClockCfg16C->rc = RC_OK_COMPUTE;
      }
    } else {
	pClockCfg16C->rc = -RC_TURBO_PREREQ;
    }
  }
}

long Skylake_X_Turbo_Config16C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg_SKL_X_PerCore);

	return (rc);
}

long TurboClock_IvyBridge_EP(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod);
	if (rc >= RC_SUCCESS)
	{
		rc = Intel_Turbo_Config15C(pClockMod);
		if (rc >= RC_SUCCESS)
		{
			TURBO_RATIO_CONFIG1 Cfg1;
			RDMSR(Cfg1, MSR_TURBO_RATIO_LIMIT1);
			Cfg1.IVB_EP.Semaphore = 1;
			WRMSR(Cfg1, MSR_TURBO_RATIO_LIMIT1);
		}
	}
	return (rc);
}

long TurboClock_Haswell_EP(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod);
	if (rc >= RC_SUCCESS) {
		rc = Intel_Turbo_Config16C(pClockMod);
	}
	if (rc >= RC_SUCCESS) {
		rc = Intel_Turbo_Config18C(pClockMod);

		if (rc >= RC_SUCCESS)
		{
			TURBO_RATIO_CONFIG2 Cfg2;
			RDMSR(Cfg2, MSR_TURBO_RATIO_LIMIT2);
			Cfg2.Semaphore = 1;
			WRMSR(Cfg2, MSR_TURBO_RATIO_LIMIT2);
		}
	}
	return (rc);
}

long TurboClock_Broadwell_EP(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod);
	if (rc >= RC_SUCCESS) {
		rc = Intel_Turbo_Config16C(pClockMod);
	}
	if (rc >= RC_SUCCESS) {
		rc = Intel_Turbo_Config18C(pClockMod);

		if (rc >= RC_SUCCESS)
		{
			TURBO_RATIO_CONFIG3 Cfg3;
			RDMSR(Cfg3, MSR_TURBO_RATIO_LIMIT3);
			Cfg3.Semaphore = 1;
			WRMSR(Cfg3, MSR_TURBO_RATIO_LIMIT3);
		}
	}
	return (rc);
}

long TurboClock_Skylake_X(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod);
	if (rc >= RC_SUCCESS) {
		rc = Skylake_X_Turbo_Config16C(pClockMod);
	}
	return (rc);
}

static void PerCore_Intel_HWP_Notification(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

    if ((arg != NULL) && PUBLIC(RO(Proc))->Features.Power.EAX.HWP_Int) {
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->HWP_Mask, Core->Bind);
	/* HWP Notifications are fully disabled.			*/
	Core->PowerThermal.HWP_Interrupt.value = 0;
	WRMSR(Core->PowerThermal.HWP_Interrupt, MSR_HWP_INTERRUPT);

	RDMSR(Core->PowerThermal.HWP_Interrupt, MSR_HWP_INTERRUPT);
	if (Core->PowerThermal.HWP_Interrupt.value == 0) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->HWP, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->HWP, Core->Bind);
	}
    }
}

static void PerCore_Intel_HWP_Ignition(void *arg)
{
    if ((arg != NULL) && PUBLIC(RO(Proc))->Features.Power.EAX.HWP_EPP) {
	CORE_RO *Core = (CORE_RO *) arg;

	RDMSR(Core->PowerThermal.HWP_Capabilities, MSR_IA32_HWP_CAPABILITIES);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

	Core->PowerThermal.HWP_Request.Minimum_Perf =
				Core->PowerThermal.HWP_Capabilities.Lowest;

	Core->PowerThermal.HWP_Request.Maximum_Perf =
				Core->PowerThermal.HWP_Capabilities.Highest;

	Core->PowerThermal.HWP_Request.Desired_Perf =
				Core->PowerThermal.HWP_Capabilities.Guaranteed;

	if ((HWP_EPP >= 0) && (HWP_EPP <= 0xff)) {
		Core->PowerThermal.HWP_Request.Energy_Pref = HWP_EPP;
	}
	WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
    }
}

void Intel_Hardware_Performance(void)
{
    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL) {
	PM_ENABLE PM_Enable = {.value = 0};
	HDC_CONTROL HDC_Control = {.value = 0};

	if (PUBLIC(RO(Proc))->Features.Power.EAX.HWP_Reg)
	{
		/*	MSR_IA32_PM_ENABLE is a Package register.	*/
		RDMSR(PM_Enable, MSR_IA32_PM_ENABLE);
		/*	Is the HWP requested and its current state is off ? */
	  if ((HWP_Enable == 1) && (PM_Enable.HWP_Enable == 0))
	  {
		unsigned int cpu;
		/*	From last AP to BSP				*/
			cpu = PUBLIC(RO(Proc))->CPU.Count;
		do {
			cpu--;

		    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)) {
			/*	Synchronous call.			*/
			smp_call_function_single(cpu,
						PerCore_Intel_HWP_Notification,
						&PUBLIC(RO(Core, AT(cpu))), 1);
		    }
		  } while (cpu != 0) ;

	    if (BITCMP_CC(PUBLIC(RO(Proc))->CPU.Count, \
		LOCKLESS, PUBLIC(RW(Proc))->HWP, PUBLIC(RO(Proc))->HWP_Mask) )
	    {
		/*	Enable the Hardware-controlled Performance States. */
		PM_Enable.HWP_Enable = 1;
		WRMSR(PM_Enable, MSR_IA32_PM_ENABLE);
		RDMSR(PM_Enable, MSR_IA32_PM_ENABLE);

		if (PM_Enable.HWP_Enable)
		{
			cpu = PUBLIC(RO(Proc))->CPU.Count;
		do {
			cpu--;

		    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)) {
			/*	Asynchronous call.			*/
			smp_call_function_single(cpu,
						PerCore_Intel_HWP_Ignition,
						&PUBLIC(RO(Core, AT(cpu))), 0);
		    }
		  } while (cpu != 0) ;
		}
	    }
	  }
	}
	PUBLIC(RO(Proc))->Features.HWP_Enable = PM_Enable.HWP_Enable;
	/*		Hardware Duty Cycling				*/
	if (PUBLIC(RO(Proc))->Features.Power.EAX.HDC_Reg)
	{
		RDMSR(HDC_Control, MSR_IA32_PKG_HDC_CTL);
		switch (HDC_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			HDC_Control.HDC_Enable = HDC_Enable;
			WRMSR(HDC_Control, MSR_IA32_PKG_HDC_CTL);
			RDMSR(HDC_Control, MSR_IA32_PKG_HDC_CTL);
			break;
		}
	}
	PUBLIC(RO(Proc))->Features.HDC_Enable = HDC_Control.HDC_Enable;
    }
}

void Intel_RaceToHalt(void)
{
	POWER_CONTROL PowerCtrl = {.value = 0};
	RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);

	switch (R2H_Disable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PowerCtrl.R2H_Disable = R2H_Disable;
			WRMSR(PowerCtrl, MSR_IA32_POWER_CTL);
			RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);
		break;
	}
	PUBLIC(RO(Proc))->Features.R2H_Disable = PowerCtrl.R2H_Disable;
}

void SandyBridge_Uncore_Ratio(unsigned int cpu)
{
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)];
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];
}

long Haswell_Uncore_Ratio(CLOCK_ARG *pClockMod)
{
	long rc = RC_SUCCESS;
	UNCORE_RATIO_LIMIT UncoreRatio = {.value = 0};
	RDMSR(UncoreRatio, MSR_HSW_UNCORE_RATIO_LIMIT);

	if (pClockMod != NULL) {	/* Called as Read-Only ?	*/
	    if (PUBLIC(RO(Proc))->Features.Uncore_Unlock)
	    {
		unsigned short WrRdMSR;
		switch (pClockMod->NC) {
		case CLOCK_MOD_MAX:
		    if (pClockMod->cpu == -1) {
			UncoreRatio.MaxRatio = pClockMod->Ratio;
		    } else {
			UncoreRatio.MaxRatio += pClockMod->Offset;
		    }
			WrRdMSR = 1;
			break;
		case CLOCK_MOD_MIN:
		    if (pClockMod->cpu == -1) {
			UncoreRatio.MinRatio = pClockMod->Ratio;
		    } else {
			UncoreRatio.MinRatio += pClockMod->Offset;
		    }
			WrRdMSR = 1;
			break;
		default:
			WrRdMSR = 0;
			rc = -RC_UNIMPLEMENTED;
			break;
		}
		if (WrRdMSR) {
			WRMSR(UncoreRatio, MSR_HSW_UNCORE_RATIO_LIMIT);
			RDMSR(UncoreRatio, MSR_HSW_UNCORE_RATIO_LIMIT);
			rc = RC_OK_COMPUTE;
		}
	    } else {
		rc = -RC_UNCORE_PREREQ;
	    }
	}

	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)]=UncoreRatio.MinRatio;
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)]=UncoreRatio.MaxRatio;

	return (rc);
}

void SandyBridge_PowerInterface(void)
{
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.Unit, MSR_RAPL_POWER_UNIT);
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.PowerInfo, MSR_PKG_POWER_INFO);
}

void Intel_PackagePowerLimit(void)
{
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.PowerLimit, MSR_PKG_POWER_LIMIT);
}

void Intel_Processor_PIN(bool capable)
{
	if (capable) {
		INTEL_PPIN_CTL PPinCtl = {.value = 0};
		RDMSR(PPinCtl, MSR_PPIN_CTL);
		if (PPinCtl.Enable) {
			INTEL_PPIN_NUM PPinNum = {.value = 0};
			RDMSR(PPinNum, MSR_PPIN);
			PUBLIC(RO(Proc))->Features.Factory.PPIN = PPinNum.value;
		}
	}
}

void AMD_Processor_PIN(bool capable)
{
	if (capable) {
		AMD_PPIN_CTL PPinCtl = {.value = 0};
		RDMSR(PPinCtl, MSR_AMD_PPIN_CTL);
		if (PPinCtl.Enable) {
			AMD_PPIN_NUM PPinNum = {.value = 0};
			RDMSR(PPinNum, MSR_AMD_PPIN);
			PUBLIC(RO(Proc))->Features.Factory.PPIN = PPinNum.value;
		}
	}
}

void Query_Same_Platform_Features(unsigned int cpu)
{
	PLATFORM_INFO PfInfo;

	PfInfo = Intel_Platform_Info(cpu);
	PUBLIC(RO(Proc))->Features.TDP_Unlock = PfInfo.ProgrammableTDP;
	PUBLIC(RO(Proc))->Features.TDP_Levels = PfInfo.ConfigTDPlevels;
	PUBLIC(RO(Proc))->Features.Turbo_Unlock = PfInfo.ProgrammableTurbo;

	if ((PRIVATE(OF(Specific)) = LookupProcessor()) != NULL) {
		OverrideCodeNameString(PRIVATE(OF(Specific)));
		OverrideUnlockCapability(PRIVATE(OF(Specific)));
	}
	Default_Unlock_Reset();

	Intel_Processor_PIN(PfInfo.PPIN_CAP);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 0;
}

void Nehalem_Platform_Info(unsigned int cpu)
{
	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
}

void IvyBridge_EP_Platform_Info(unsigned int cpu)
{
	const unsigned int NC = \
	PUBLIC(RO(Proc))->CPU.Count >> PUBLIC(RO(Proc))->Features.HTT_Enable;

	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
    if (NC > 8) {
	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 7; /*	15C	*/
    }
}

void Haswell_EP_Platform_Info(unsigned int cpu)
{
	const unsigned int NC = \
	PUBLIC(RO(Proc))->CPU.Count >> PUBLIC(RO(Proc))->Features.HTT_Enable;

	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
    if (NC > 8) {
	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	16C	*/
    }
    if (NC > 16) {
	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 2; /*	18C	*/
    }
}

void Skylake_X_Platform_Info(unsigned int cpu)
{
	const unsigned int NC = \
	PUBLIC(RO(Proc))->CPU.Count >> PUBLIC(RO(Proc))->Features.HTT_Enable;

	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
    if (NC > 8) {
	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	16C	*/
    }
}


typedef void (*ROUTER)(void __iomem *mchmap);

PCI_CALLBACK Router(struct pci_dev *dev, unsigned int offset,
		unsigned int bsize, unsigned long long wsize, ROUTER route)
{
	void __iomem *mchmap;
	union {
		unsigned long long addr;
		struct {
			unsigned int low;
			unsigned int high;
		};
	} mchbar;
	unsigned long long wmask = BITCPL(wsize);
	unsigned char mchbarEnable = 0;

	switch (bsize) {
	case 32:
		pci_read_config_dword(dev, offset    , &mchbar.low);
		mchbar.high = 0;
		break;
	case 64:
		pci_read_config_dword(dev, offset    , &mchbar.low);
		pci_read_config_dword(dev, offset + 4, &mchbar.high);
		break;
	}
	mchbarEnable = BITVAL(mchbar, 0);
	if (mchbarEnable) {
		mchbar.addr &= wmask;
		mchmap = ioremap(mchbar.addr, wsize);
		if (mchmap != NULL) {
			route(mchmap);

			iounmap(mchmap);

			return (0);
		} else
			return ((PCI_CALLBACK) -ENOMEM);
	} else
		return ((PCI_CALLBACK) -ENOMEM);
}

void Query_P945(void __iomem *mchmap)
{	/* Source: Mobile Intel 945 Express Chipset Family.		*/
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[0].P945.DCC.value = readl(mchmap + 0x200);

	switch (PUBLIC(RO(Proc))->Uncore.MC[0].P945.DCC.DAMC) {
	case 0b00:
	case 0b11:
		PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 1;
		break;
	case 0b01:
	case 0b10:
		PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 2;
		break;
	}

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = 1;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
    {
	unsigned short rank, rankCount;

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.DRT0.value = \
					readl(mchmap + 0x110 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.DRT1.value = \
					readw(mchmap + 0x114 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.DRT2.value = \
					readl(mchmap + 0x118 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.BANK.value = \
					readw(mchmap + 0x10e + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.WIDTH.value = \
					readw(mchmap + 0x40c + 0x80 * cha);
      if (cha == 0) {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.WIDTH.value &= \
								0b11111111;
	rankCount = 4;
      } else {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.WIDTH.value &= 0b1111;
	rankCount = 2;
      }
      for (rank = 0; rank < rankCount; rank++)
      {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P945.DRB[rank].value = \
				readb(mchmap + 0x100 + rank + 0x80 * cha);
      }
    }
}

void Query_P955(void __iomem *mchmap)
{	/* Source: Intel 82955X Memory Controller Hub (MCH)		*/
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[0].P955.DCC.value = readl(mchmap + 0x200);

	switch (PUBLIC(RO(Proc))->Uncore.MC[0].P955.DCC.DAMC) {
	case 0b00:
	case 0b11:
		PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 1;
		break;
	case 0b01:
	case 0b10:
		PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 2;
		break;
	}

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = 1;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
    {
	unsigned short rank;

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P955.DRT1.value = \
					readw(mchmap + 0x114 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P955.BANK.value = \
					readw(mchmap + 0x10e + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P955.WIDTH.value = \
					readw(mchmap + 0x40c + 0x80 * cha);

      for (rank = 0; rank < 4; rank++)
      {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P955.DRB[rank].value = \
				readb(mchmap + 0x100 + rank + 0x80 * cha);
      }
    }
}

void Query_P965(void __iomem *mchmap)
{
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[0].P965.CKE0.value = readl(mchmap + 0x260);
	PUBLIC(RO(Proc))->Uncore.MC[0].P965.CKE1.value = readl(mchmap + 0x660);

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[0].P965.CKE0.RankPop0 != 0)
		+ (PUBLIC(RO(Proc))->Uncore.MC[0].P965.CKE1.RankPop0 != 0);

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[0].P965.CKE0.SingleDimmPop ?1:2)
		+ (PUBLIC(RO(Proc))->Uncore.MC[0].P965.CKE1.SingleDimmPop ?1:2);

	for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
	{
		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P965.DRT0.value = \
					readl(mchmap + 0x29c + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P965.DRT1.value = \
					readw(mchmap + 0x250 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P965.DRT2.value = \
					readl(mchmap + 0x252 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P965.DRT3.value = \
					readw(mchmap + 0x256 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P965.DRT4.value = \
					readl(mchmap + 0x258 + 0x400 * cha);
	}
}

void Query_G965(void __iomem *mchmap)
{	/* Source: Mobile Intel 965 Express Chipset Family.		*/
	unsigned short cha, slot;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[0].G965.DRB0.value = readl(mchmap + 0x1200);
	PUBLIC(RO(Proc))->Uncore.MC[0].G965.DRB1.value = readl(mchmap + 0x1300);

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[0].G965.DRB0.Rank1Addr != 0)
		+ (PUBLIC(RO(Proc))->Uncore.MC[0].G965.DRB1.Rank1Addr != 0);

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = \
			PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount > 1 ? 1:2;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].G965.DRT0.value = \
					readl(mchmap + 0x1210 + 0x100 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].G965.DRT1.value = \
					readl(mchmap + 0x1214 + 0x100 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].G965.DRT2.value = \
					readl(mchmap + 0x1218 + 0x100 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].G965.DRT3.value = \
					readl(mchmap + 0x121c + 0x100 * cha);

      for (slot = 0; slot < PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount; slot++)
      {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].DIMM[slot].DRA.value = \
					readl(mchmap + 0x1208 + 0x100 * cha);
      }
    }
}

void Query_P35(void __iomem *mchmap)
{	/* Source: Intel® 3 Series Express Chipset Family. */
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[0].P35.CKE0.value = readl(mchmap + 0x260);
	PUBLIC(RO(Proc))->Uncore.MC[0].P35.CKE1.value = readl(mchmap + 0x660);

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[0].P35.CKE0.RankPop0 != 0)
		+ (PUBLIC(RO(Proc))->Uncore.MC[0].P35.CKE1.RankPop0 != 0);

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = \
	  (PUBLIC(RO(Proc))->Uncore.MC[0].P35.CKE0.SingleDimmPop ? 1 : 2)
	+ (PUBLIC(RO(Proc))->Uncore.MC[0].P35.CKE1.SingleDimmPop ? 1 : 2);

	for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
	{
		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P35.DRT0.value = \
					readw(mchmap + 0x265 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P35.DRT1.value = \
					readw(mchmap + 0x250 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P35.DRT2.value = \
					readl(mchmap + 0x252 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P35.DRT3.value = \
					readl(mchmap + 0x256 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P35.DRT4.value = \
					readl(mchmap + 0x258 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].P35.DRT5.value = \
					readw(mchmap + 0x25d + 0x400 * cha);
	}
}

kernel_ulong_t Query_NHM_Timing(unsigned int did,
				unsigned short mc,
				unsigned short cha)
{	/*Source: Micron Technical Note DDR3 Power-Up, Initialization, & Reset*/
    struct pci_dev *dev = pci_get_device(PCI_VENDOR_ID_INTEL, did, NULL);
    if (dev != NULL)
    {
	pci_read_config_dword(dev, 0x70,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.MR0_1.value);

	pci_read_config_dword(dev, 0x74,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.MR2_3.value);

	pci_read_config_dword(dev ,0x80,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.Rank_A.value);

	pci_read_config_dword(dev ,0x84,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.Rank_B.value);

	pci_read_config_dword(dev ,0x88,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.Bank.value);

	pci_read_config_dword(dev ,0x8c,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.Refresh.value);

	pci_read_config_dword(dev ,0x90,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.CKE_Timing.value);

	pci_read_config_dword(dev, 0xb8,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.Params.value);

	pci_dev_put(dev);
	return (0);
    } else
	return (-ENODEV);
}

kernel_ulong_t Query_NHM_DIMM(	unsigned int did,
				unsigned short mc,
				unsigned short cha)
{
	struct pci_dev *dev = pci_get_device(PCI_VENDOR_ID_INTEL, did, NULL);
    if (dev != NULL)
    {
	unsigned short slot;
	for (slot = 0; slot < PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount; slot++)
	{
	pci_read_config_dword(dev, 0x48 + 4 * slot,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.value);
	}
	pci_dev_put(dev);
	return (0);
    } else {
	return (-ENODEV);
    }
}

void Query_NHM_MaxDIMMs(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(dev, 0x64,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].MaxDIMMs.NHM.DOD.value);

	switch (PUBLIC(RO(Proc))->Uncore.MC[mc].MaxDIMMs.NHM.DOD.MAXNUMDIMMS) {
	case 0b00:
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 1;
		break;
	case 0b01:
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 2;
		break;
	case 0b10:
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 3;
		break;
	default:
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 0;
		break;
	}
}

kernel_ulong_t Query_NHM_IMC(	struct pci_dev *dev,
				unsigned int did[2][3],
				unsigned short mc)
{
	kernel_ulong_t rc = 0;
	unsigned short cha;

	Query_NHM_MaxDIMMs(dev, mc);

	pci_read_config_dword(dev, 0x48,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.value);

	pci_read_config_dword(dev, 0x4c,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.STATUS.value);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  (PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.CHANNEL0_ACTIVE != 0)
	+ (PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0)
	+ (PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.CHANNEL2_ACTIVE != 0);

	for (cha = 0;
		(cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount) && !rc;
			cha++)
	{
		rc = Query_NHM_Timing(did[0][cha], mc, cha)
		   & Query_NHM_DIMM(did[1][cha], mc, cha);
	}
	return (rc);
}

kernel_ulong_t Query_Lynnfield_IMC(struct pci_dev *dev, unsigned short mc)
{
	kernel_ulong_t rc = 0;
	unsigned int did[2][2] = {
		{
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_CTRL,
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_CTRL
		},
		{
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_ADDR,
			PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_ADDR
		}
	};
	unsigned short cha;

	Query_NHM_MaxDIMMs(dev, mc);

	pci_read_config_dword(dev, 0x48,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.value);

	pci_read_config_dword(dev, 0x4c,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.STATUS.value);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  (PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.CHANNEL0_ACTIVE != 0)
	+ (PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0);

    for (cha = 0;
		(cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount) && !rc;
			cha++)
    {
	rc = Query_NHM_Timing(did[0][cha], mc, cha)
	   & Query_NHM_DIMM(did[1][cha], mc, cha);
    }
	return (rc);
}

void Query_SNB_IMC(void __iomem *mchmap)
{	/* Sources:	2nd & 3rd Generation Intel® Core™ Processor Family
			Intel® Xeon Processor E3-1200 Family		*/
	unsigned short cha, dimmCount[2];

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD0.value = readl(mchmap + 0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD1.value = readl(mchmap + 0x5008);

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 0;

	dimmCount[0] =(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD0.Dimm_A_Size > 0)
		     +(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD0.Dimm_B_Size > 0);
	dimmCount[1] =(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD1.Dimm_A_Size > 0)
		     +(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD1.Dimm_B_Size > 0);

    for (cha = 0; cha < 2; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount += (dimmCount[cha] > 0);
    }
    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
    {
		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SNB.DBP.value = \
					readl(mchmap + 0x4000 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SNB.RAP.value = \
					readl(mchmap + 0x4004 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SNB.RFTP.value = \
					readl(mchmap + 0x4298 + 0x400 * cha);
    }
	/*		Is Dual DIMM Per Channel Disable ?		*/
	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = \
			(PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.DDPCD == 1) ?
			1 : PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount;
}

void Query_Turbo_TDP_Config(void __iomem *mchmap)
{
	TURBO_ACTIVATION TurboActivation = {.value = 0};
	CONFIG_TDP_NOMINAL NominalTDP = {.value = 0};
	CONFIG_TDP_CONTROL ControlTDP = {.value = 0};
	CONFIG_TDP_LEVEL ConfigTDP;
	unsigned int cpu, local = get_cpu();	/* TODO(preempt_disable) */

	NominalTDP.value = readl(mchmap + 0x5f3c);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP)] = NominalTDP.Ratio;

	ConfigTDP.value = readq(mchmap + 0x5f40);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP1)] = ConfigTDP.Ratio;

	ConfigTDP.value = readq(mchmap + 0x5f48);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP2)] = ConfigTDP.Ratio;

	ControlTDP.value = readl(mchmap + 0x5f50);
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Lock  = ControlTDP.Lock;
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Level = ControlTDP.Level;

	TurboActivation.value = readl(mchmap + 0x5f54);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(ACT)]=TurboActivation.MaxRatio;
	PUBLIC(RO(Proc))->Features.TurboActiv_Lock = TurboActivation.Ratio_Lock;

	put_cpu();	/* TODO(preempt_enable) */

	PUBLIC(RO(Proc))->Features.TDP_Levels = 3;

	for (cpu = 0; cpu < PUBLIC(RO(Proc))->CPU.Count; cpu++) {
	    if (cpu != local)
	    {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TDP)] = \
				PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP)];

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TDP1)] = \
				PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP1)];

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TDP2)] = \
				PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP2)];

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(ACT)] = \
				PUBLIC(RO(Core, AT(local)))->Boost[BOOST(ACT)];
	    }
	}
}

void Query_HSW_IMC(void __iomem *mchmap)
{	/*Source: Desktop 4th & 5th Generation Intel® Core™ Processor Family.*/
	unsigned short cha, dimmCount[2];

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD0.value = readl(mchmap + 0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD1.value = readl(mchmap + 0x5008);

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 0;

	dimmCount[0] =(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD0.Dimm_A_Size > 0)
		     +(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD0.Dimm_B_Size > 0);
	dimmCount[1] =(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD1.Dimm_A_Size > 0)
		     +(PUBLIC(RO(Proc))->Uncore.MC[0].SNB.MAD1.Dimm_B_Size > 0);

    for (cha = 0; cha < 2; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount += (dimmCount[cha] > 0);
    }
    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
    {
/*TODO( Unsolved: What is the channel #1 'X' factor of Haswell registers ? )*/
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].HSW.REG4C00.value = \
							readl(mchmap + 0x4c00);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].HSW.Timing.value = \
							readl(mchmap + 0x4c04);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].HSW.Rank_A.value = \
							readl(mchmap + 0x4c08);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].HSW.Rank_B.value = \
							readl(mchmap + 0x4c0c);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].HSW.Rank.value = \
							readl(mchmap + 0x4c14);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].HSW.Refresh.value = \
							readl(mchmap + 0x4e98);
    }
	/*		Is Dual DIMM Per Channel Disable ?		*/
	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = \
			(PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.DDPCD == 1) ?
			1 : PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount;

	Query_Turbo_TDP_Config(mchmap);
}

void Query_SKL_IMC(void __iomem *mchmap)
{	/*Source: 6th & 7th Generation Intel® Processor for S-Platforms Vol 2*/
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	/*		Intra channel configuration			*/
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADCH.value = readl(mchmap + 0x5000);
    if (PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADCH.CH_L_MAP)
    {
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADC0.value = readl(mchmap + 0x5008);
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADC1.value = readl(mchmap + 0x5004);
    } else {
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADC0.value = readl(mchmap + 0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADC1.value = readl(mchmap + 0x5008);
    }
	/*		DIMM parameters					*/
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD0.value = readl(mchmap + 0x500c);
	PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD1.value = readl(mchmap + 0x5010);
	/*		Sum up any present DIMM per channel.		*/
	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = \
		  ((PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD0.Dimm_L_Size != 0)
		|| (PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD0.Dimm_S_Size != 0))
		+ ((PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD1.Dimm_L_Size != 0)
		|| (PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD1.Dimm_S_Size != 0));
	/*		Count of populated DIMMs L and DIMMs S		*/
	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = \
		  ((PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD0.Dimm_L_Size != 0)
		|| (PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD1.Dimm_L_Size != 0))
		+ ((PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD0.Dimm_S_Size != 0)
		|| (PUBLIC(RO(Proc))->Uncore.MC[0].SKL.MADD1.Dimm_S_Size != 0));

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SKL.Timing.value = \
					readl(mchmap + 0x4000 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SKL.Sched.value = \
					readl(mchmap + 0x401c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SKL.ODT.value = \
					readl(mchmap + 0x4070 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].SKL.Refresh.value = \
					readl(mchmap + 0x423c + 0x400 * cha);
    }

	Query_Turbo_TDP_Config(mchmap);
}

static PCI_CALLBACK P945(struct pci_dev *dev)
{
	return (Router(dev, 0x44, 32, 0x4000, Query_P945));
}

static PCI_CALLBACK P955(struct pci_dev *dev)
{
	return (Router(dev, 0x44, 32, 0x4000, Query_P955));
}

static PCI_CALLBACK P965(struct pci_dev *dev)
{
	return (Router(dev, 0x48, 64, 0x4000, Query_P965));
}

static PCI_CALLBACK G965(struct pci_dev *dev)
{
	return (Router(dev, 0x48, 64, 0x4000, Query_G965));
}

static PCI_CALLBACK P35(struct pci_dev *dev)
{
	return (Router(dev, 0x48, 64, 0x4000, Query_P35));
}

static PCI_CALLBACK SoC_SLM(struct pci_dev *dev)
{/* DRP */
	PCI_MCR MsgCtrlReg = {
	.MBZ = 0, .Bytes = 0, .Offset = 0x0, .Port = 0x1, .OpCode = 0x10
	};

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.value);
/* DTR0 */
	MsgCtrlReg.Offset = 0x1;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DTR0.value);
/* DTR1 */
	MsgCtrlReg.Offset = 0x2;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DTR1.value);
/* DTR2 */
	MsgCtrlReg.Offset = 0x3;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DTR2.value);
/* DTR3 */
	MsgCtrlReg.Offset = 0x4;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DTR3.value);
/* DRFC */
	MsgCtrlReg.Offset = 0x8;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRFC.value);
/* BIOS_CFG */
	MsgCtrlReg.Port = 0x4;
	MsgCtrlReg.Offset = 0x6;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.value);

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 1
		+ ( PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.EFF_DUAL_CH_EN
		| !PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.DUAL_CH_DIS );

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = (
			PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN0
		|	PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN1
	) + (
			PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN2
		|	PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKNE3
	);
	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK Bloomfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned int did[2][3] = {
		{
			PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL,
			PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL,
			PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL
		},
		{
			PCI_DEVICE_ID_INTEL_I7_MC_CH0_ADDR,
			PCI_DEVICE_ID_INTEL_I7_MC_CH1_ADDR,
			PCI_DEVICE_ID_INTEL_I7_MC_CH2_ADDR
		}
	};
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < PUBLIC(RO(Proc))->Uncore.CtrlCount) && !rc; mc++) {
		rc = Query_NHM_IMC(dev, did, mc);
	}
	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < PUBLIC(RO(Proc))->Uncore.CtrlCount) && !rc; mc++) {
		rc = Query_Lynnfield_IMC(dev, mc);
	}
	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK Westmere_EP_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned int did[2][3] = {
		{
			PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_CTRL,
			PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_CTRL,
			PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_CTRL
		},
		{
			PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_ADDR,
			PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_ADDR,
			PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_ADDR
		}
	};
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < PUBLIC(RO(Proc))->Uncore.CtrlCount) && !rc; mc++) {
		rc = Query_NHM_IMC(dev, did, mc);
	}
	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x50,
				&PUBLIC(RO(Proc))->Uncore.Bus.DimmClock.value);

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK NHM_NON_CORE(struct pci_dev *dev)
{
	NHM_CURRENT_UCLK_RATIO UncoreClock = {.value = 0};

	pci_read_config_dword(dev, 0xc0, &UncoreClock.value);

	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)]=UncoreClock.UCLK;
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)]=UncoreClock.MinRatio;

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK X58_QPI(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xd0,
				&PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.value);

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK X58_VTD(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned int base = 0;

	pci_read_config_dword(dev, 0x180, &base);
	if (base) {
		PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.X58.VT_d = 0;
/* IOMMU Bug:	{
			void __iomem *mmio;
			unsigned int version = 0;
			base = (base >> 13) + 1;
			mmio = ioremap(base, 0x1000);
			if (mmio != NULL) {
				version = readl(mmio + 0x0);
				iounmap(mmio);
			} else
				rc = -ENOMEM;
		}	*/
	} else {
		PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.X58.VT_d = 1;
	}
	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK SNB_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SNB_IMC));
}

static PCI_CALLBACK IVB_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.IVB_Cap.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SNB_IMC));
}

static PCI_CALLBACK SNB_EP_HB(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK SNB_EP_CAP(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x84,
			&PUBLIC(RO(Proc))->Uncore.Bus.SNB_EP_Cap0.value);

	pci_read_config_dword(dev, 0x88,
			&PUBLIC(RO(Proc))->Uncore.Bus.SNB_EP_Cap1.value);

	pci_read_config_dword(dev, 0x8c,
			&PUBLIC(RO(Proc))->Uncore.Bus.SNB_EP_Cap2.value);

	pci_read_config_dword(dev, 0x90,
			&PUBLIC(RO(Proc))->Uncore.Bus.SNB_EP_Cap3.value);

	pci_read_config_dword(dev, 0x94,
			&PUBLIC(RO(Proc))->Uncore.Bus.SNB_EP_Cap4.value);

	return ((PCI_CALLBACK) 0);
}

kernel_ulong_t SNB_EP_CTRL(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(dev, 0x7c,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].SNB_EP.TECH.value);

	pci_read_config_dword(dev, 0x80,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].SNB_EP.TAD.value);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
			PUBLIC(RO(Proc))->Uncore.MC[mc].SNB_EP.TAD.CH_WAY;

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount++;
/*TODO(Specs missing)*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 2;

	return (0);
}

static PCI_CALLBACK SNB_EP_CTRL0(struct pci_dev *dev)
{
	if (PUBLIC(RO(Proc))->Uncore.CtrlCount < 1) {
		PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	}
	SNB_EP_CTRL(dev, 0);

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK SNB_EP_CTRL1(struct pci_dev *dev)
{
	if (PUBLIC(RO(Proc))->Uncore.CtrlCount < 2) {
		PUBLIC(RO(Proc))->Uncore.CtrlCount = 2;
	}
	SNB_EP_CTRL(dev, 1);

	return ((PCI_CALLBACK) 0);
}

kernel_ulong_t SNB_EP_IMC(struct pci_dev *dev ,unsigned short mc,
						unsigned short cha)
{
	pci_read_config_dword(dev, 0x200,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.value);

	pci_read_config_dword(dev, 0x204,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.value);

	pci_read_config_dword(dev, 0x208,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.value);

	pci_read_config_dword(dev, 0x214,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.value);

	return (0);
}

static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA0(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 0, 0));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA1(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 0, 1));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA2(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 0, 2));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA3(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 0, 3));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA0(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 1, 0));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA1(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 1, 1));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA2(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 1, 2));
}

static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA3(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_IMC(dev, 1, 3));
}

kernel_ulong_t SNB_EP_TAD(struct pci_dev *dev,	unsigned short mc,
						unsigned short cha)
{
	pci_read_config_dword(dev, 0x80,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[0].MTR.value);

	pci_read_config_dword(dev, 0x84,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[1].MTR.value);

	pci_read_config_dword(dev, 0x88,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[2].MTR.value);
	return (0);
}

static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA0(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 0, 0));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA1(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 0, 1));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA2(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 0, 2));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA3(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 0, 3));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA0(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 1, 0));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA1(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 1, 1));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA2(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 1, 2));
}

static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA3(struct pci_dev *dev)
{
	return ((PCI_CALLBACK) SNB_EP_TAD(dev, 1, 3));
}

static PCI_CALLBACK SNB_EP_QPI(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xd4,
				&PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.value);

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK HSW_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.IVB_Cap.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_HSW_IMC));
}

static PCI_CALLBACK SKL_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_A.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_B.value);

	pci_read_config_dword(dev, 0xec,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_C.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SKL_IMC));
}
/* TODO(Hardware missing)
static PCI_CALLBACK SKL_SA(struct pci_dev *dev)
{
	SKL_SA_PLL_RATIOS PllRatios = {.value = 0};

	pci_read_config_dword(dev, 0x5918, &PllRatios.value);

	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)] = PllRatios.UCLK;
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)] = 0;

	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_A.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_B.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SKL_IMC));

	return (0);
}
*/
static PCI_CALLBACK AMD_0Fh_MCH(struct pci_dev *dev)
{	/* Source: BKDG for AMD NPT Family 0Fh Processors.		*/
	unsigned short cha, slot, chip;
	/*		As defined by specifications.			*/
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	/*		DRAM Configuration low register.		*/
	pci_read_config_dword(dev, 0x90,
			&PUBLIC(RO(Proc))->Uncore.MC[0].AMD0Fh.DCRL.value);
	/*		DRAM Configuration high register.		*/
	pci_read_config_dword(dev, 0x94,
			&PUBLIC(RO(Proc))->Uncore.MC[0].AMD0Fh.DCRH.value);
	/*	One channel if 64 bits / two channels if 128 bits width. */
	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = \
			PUBLIC(RO(Proc))->Uncore.MC[0].AMD0Fh.DCRL.Width128 + 1;
	/*		DIMM Geometry.					*/
    for (chip = 0; chip < 8; chip++)
    {
	cha = chip >> 2;
	slot = chip % 4;
	pci_read_config_dword(dev, 0x40 + 4 * chip,
	    &PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].DIMM[slot].MBA.value);

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount += \
	  PUBLIC(RO(Proc))->Uncore.MC[0].Channel[cha].DIMM[slot].MBA.CSEnable;
    }
	/* DIMM Size. */
	pci_read_config_dword(dev, 0x80,
		&PUBLIC(RO(Proc))->Uncore.MC[0].MaxDIMMs.AMD0Fh.CS.value);
	/* DRAM Timings. */
	pci_read_config_dword(dev, 0x88,
		&PUBLIC(RO(Proc))->Uncore.MC[0].Channel[0].AMD0Fh.DTRL.value);
	/* Assume same timings for both channels. */
	PUBLIC(RO(Proc))->Uncore.MC[0].Channel[1].AMD0Fh.DTRL.value = \
		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[0].AMD0Fh.DTRL.value;

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK AMD_0Fh_HTT(struct pci_dev *dev)
{
	unsigned int link;

	pci_read_config_dword(dev, 0x60,
				&PUBLIC(RO(Proc))->Uncore.Bus.NodeID.value);

    switch ( PCI_SLOT(dev->devfn) ) {
    case 24:
	PUBLIC(RO(Core, AT(0)))->T.Cluster.Node = \
				PUBLIC(RO(Proc))->Uncore.Bus.NodeID.Node;
	break;
    case 25:
      if ( ( (1 + PUBLIC(RO(Proc))->Uncore.Bus.NodeID.CPUCnt)
		>=PUBLIC(RO(Proc))->CPU.Count )
	&& (PUBLIC(RO(Proc))->CPU.Count >= 2) )
      {
	PUBLIC(RO(Core, AT(1)))->T.Cluster.Node = \
				PUBLIC(RO(Proc))->Uncore.Bus.NodeID.Node;
      }
	break;
    }

	pci_read_config_dword(dev, 0x64,
				&PUBLIC(RO(Proc))->Uncore.Bus.UnitID.value);

      for (link = 0; link < 3; link++)
      {
	pci_read_config_dword(dev, 0x88 + 0x20 * link,
			&PUBLIC(RO(Proc))->Uncore.Bus.LDTi_Freq[link].value);
      };

	return ((PCI_CALLBACK) 0);
}

#ifdef CONFIG_AMD_NB
static PCI_CALLBACK AMD_17h_ZenIF(struct pci_dev *dev)
{
	if (PRIVATE(OF(ZenIF_dev)) == NULL) {
		PRIVATE(OF(ZenIF_dev)) = dev;
	}
	return ((PCI_CALLBACK) 0);
}
#endif /* CONFIG_AMD_NB */

static PCI_CALLBACK AMD_Zen_IOMMU(struct pci_dev *dev)
{
/*TODO(Not Used Yet)
	void __iomem *iommu_mmio = NULL;
*/
	unsigned long long iommu_cap_base;
	unsigned int iommu_cap_base_lo = 0, iommu_cap_base_hi = 0;

	pci_read_config_dword(dev, 0x44, &iommu_cap_base_lo);
	pci_read_config_dword(dev, 0x48, &iommu_cap_base_hi);

	iommu_cap_base = ((iommu_cap_base_lo & 0xfff80000) >> 19)
			+ ((unsigned long long) iommu_cap_base_hi << 32);

    if (BITVAL(iommu_cap_base, 0))
    {
/*TODO(Not Used Yet)
	iommu_mmio = ioremap(iommu_cap_base, 0x200);
	if (iommu_mmio != NULL) {
		PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_CR=readq(iommu_mmio + 0x18);

		iounmap(iommu_mmio);
	} else {
		return ((PCI_CALLBACK) -ENOMEM);
	}
*/
	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_CR = 1;

	return ((PCI_CALLBACK) 0);
    }
	return ((PCI_CALLBACK) -ENOMEM);
}

static PCI_CALLBACK AMD_17h_UMC(struct pci_dev *dev)
{
	AMD_17_UMC_SDP_CTRL SDP_CTRL;
	unsigned short mc, cha, chip, sec;
/*TODO( Query the number of UMC )					*/
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

  for (mc = 0; mc < PUBLIC(RO(Proc))->Uncore.CtrlCount; mc++)
  {
	unsigned int UMC_BAR[MC_MAX_CHA] = { 0,0,0,0,0,0,0,0 };

	unsigned short count = 0;
    for (cha = 0; cha < MC_MAX_CHA; cha++)
    {
	SDP_CTRL.value = 0;

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.ECC,
			(SMU_AMD_UMC_BASE_CHA_F17H(cha) + 0xdf4),
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

	Core_AMD_SMN_Read(	SDP_CTRL,
				(SMU_AMD_UMC_BASE_CHA_F17H(cha) + 0x104),
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	if ((SDP_CTRL.value != 0xffffffff) && (SDP_CTRL.INIT))
	{
		UMC_BAR[count++] = SMU_AMD_UMC_BASE_CHA_F17H(cha);
	}
    }
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = count;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	unsigned int CHIP_BAR[2][2];

	CHIP_BAR[0][0] = UMC_BAR[cha] + 0x0;

	CHIP_BAR[0][1] = UMC_BAR[cha] + 0x20;

	CHIP_BAR[1][0] = UMC_BAR[cha] + 0x10;

	CHIP_BAR[1][1] = UMC_BAR[cha] + 0x28;

	for (chip = 0; chip < 4; chip++)
	{
	  for (sec = 0; sec < 2; sec++)
	  {
		unsigned int addr[2], ranks = 0;

		addr[1] = CHIP_BAR[sec][1] + 4 * (chip >> 1);

		Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
				.AMD17h.CHIP[chip][sec].Mask,
				addr[1],
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	    if ( (ranks == 0)
		&& (PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
			.AMD17h.CHIP[chip][sec].Mask.value != 0) )
	    {
		ranks = BITVAL(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
				.AMD17h.CHIP[chip][sec].Mask.value, 9) ? 1 : 2;
	    }
	    if (ranks == 2) {
		addr[0] = CHIP_BAR[sec][0] + 4 * chip;
	    } else {
		addr[0] = CHIP_BAR[sec][0] + 4 * (chip - (chip > 2));
	    }
		Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
				.AMD17h.CHIP[chip][sec].Chip,
				addr[0],
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	    if (PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.Ranks == 0)
	    {
		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.Ranks=ranks;
	    }
	  }
	}
    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.MISC,
			UMC_BAR[cha] + 0x200,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR1,
			UMC_BAR[cha] + 0x204,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR2,
			UMC_BAR[cha] + 0x208,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR3,
			UMC_BAR[cha] + 0x20c,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR4,
			UMC_BAR[cha] + 0x210,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR5,
			UMC_BAR[cha] + 0x214,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR6,
			UMC_BAR[cha] + 0x218,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR7,
			UMC_BAR[cha] + 0x21c,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR8,
			UMC_BAR[cha] + 0x220,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR9,
			UMC_BAR[cha] + 0x224,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR10,
			UMC_BAR[cha] + 0x228,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR12,
			UMC_BAR[cha] + 0x230,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR54,
			UMC_BAR[cha] + 0x254,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR60,
			UMC_BAR[cha] + 0x260,
			SMU_AMD_INDEX_REGISTER_F17H,
			SMU_AMD_DATA_REGISTER_F17H );
    }
  }
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)] = \
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)] = \
		PUBLIC(RO(Proc))->Uncore.MC[0].Channel[0].AMD17h.MISC.MEMCLK/3;

	return ((PCI_CALLBACK) 0);
}

static int CoreFreqK_ProbePCI(void)
{
	struct pci_device_id *id = Arch[PUBLIC(RO(Proc))->ArchID].PCI_ids;
	struct pci_dev *dev = NULL;
	int rc = -ENODEV;

	memset( PUBLIC(RO(Proc))->Uncore.Chip, 0,
		CHIP_MAX_PCI*sizeof(struct CHIP_ST) );

	while (id->vendor || id->subvendor || id->class_mask)
	{
		dev = pci_get_device(id->vendor, id->device, NULL);
	  if (dev != NULL) {
	    if (!pci_enable_device(dev))
	    {
		PCI_CALLBACK Callback = (PCI_CALLBACK) id->driver_data;

		if ((rc = (int) Callback(dev)) == 0)
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
		pci_disable_device(dev);
	    }
	    pci_dev_put(dev);
	  }
		id++;
	}
	return (rc);
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

void Query_GenuineIntel(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	Intel_Core_Platform_Info(cpu);
	HyperThreading_Technology();
}

void Query_Core2(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	Intel_Core_Platform_Info(cpu);
	HyperThreading_Technology();
}

void Query_Silvermont(unsigned int cpu)
{	/*	Query the Min and Max frequency ratios			*/
	PLATFORM_INFO PfInfo = {.value = 0};
	RDMSR(PfInfo, MSR_PLATFORM_INFO);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = PfInfo.MinimumRatio;
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = PfInfo.MaxNonTurboRatio;
	/*	Assume a count of Boost ratios equals to the CPU count	*/
	PUBLIC(RO(Proc))->Features.SpecTurboRatio=PUBLIC(RO(Proc))->CPU.Count;
	/*	But can be overridden following the specifications	*/
	if ((PRIVATE(OF(Specific)) = LookupProcessor()) != NULL)
	{
		OverrideCodeNameString(PRIVATE(OF(Specific)));
		OverrideUnlockCapability(PRIVATE(OF(Specific)));
	}
	Default_Unlock_Reset();

	HyperThreading_Technology();
	/*	The architecture is gifted of Power and Energy registers */
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.Unit, MSR_RAPL_POWER_UNIT);
	Intel_PackagePowerLimit();
}

void Query_Nehalem(unsigned int cpu)
{
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
}

void Query_SandyBridge(unsigned int cpu)
{
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	SandyBridge_PowerInterface();
	Intel_PackagePowerLimit();
}

void Query_IvyBridge(unsigned int cpu)
{
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	SandyBridge_PowerInterface();
	Intel_PackagePowerLimit();
}

void Query_IvyBridge_EP(unsigned int cpu)
{
	IvyBridge_EP_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	SandyBridge_PowerInterface();
	Intel_PackagePowerLimit();
}

void Query_Haswell(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	SandyBridge_PowerInterface();
	Intel_PackagePowerLimit();
}

void Query_Haswell_EP(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Haswell_EP_Platform_Info(cpu);
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	SandyBridge_PowerInterface();
	Intel_PackagePowerLimit();
}

void Query_Haswell_ULT(unsigned int cpu)
{
	Query_IvyBridge(cpu);
}

void Query_Haswell_ULX(unsigned int cpu)
{
	Query_IvyBridge(cpu);
}

void Query_Broadwell(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	SandyBridge_PowerInterface();
	Intel_Hardware_Performance();
	Intel_PackagePowerLimit();
}

void Query_Skylake(unsigned int cpu)
{
	Query_Broadwell(cpu);

	PUBLIC(RO(Proc))->Features.R2H_Capable = 1;
	Intel_RaceToHalt();
}

void Query_Broadwell_EP(unsigned int cpu)
{
	Query_Haswell_EP(cpu);
	Intel_Hardware_Performance();
}

void Query_Skylake_X(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Skylake_X_Platform_Info(cpu);
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	SandyBridge_PowerInterface();
	Intel_Hardware_Performance();
}

unsigned short Compute_AuthenticAMD_Boost(unsigned int cpu)
{
	unsigned short SpecTurboRatio = 0;
	/* Lowest frequency according to BKDG				*/
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = 8;

  if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.HwPstate == 1)
  {
	COFVID CofVid = {.value = 0};

    switch (Arch[PUBLIC(RO(Proc))->ArchID].Signature.ExtFamily) {
    case 0x1:
    case 0x6:
    case 0x7:
	RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)]=CofVid.Arch_COF.MaxCpuCof;
	break;
    case 0x2:
	RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = \
						CofVid.Arch_Pll.MainPllOpFidMax;
      if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
      {
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] += 0x8;
      }
	break;
    case 0x3:
    case 0x5:
	RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = \
						CofVid.Arch_Pll.MainPllOpFidMax;
      if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
      {
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] += 0x10;
      }
	break;
    }
  } else {
	/* TODO(Get PLL for non HwPstate processor)			*/
  }
  if (PRIVATE(OF(Specific)) != NULL)
  {
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)]
				+ PRIVATE(OF(Specific))->Boost[0];

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(2C)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)]
				+ PRIVATE(OF(Specific))->Boost[1];
	SpecTurboRatio = 2;
  } else {
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];
	SpecTurboRatio = 0;
  }
	return (SpecTurboRatio);
}

void Query_VirtualMachine(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	/* Reset Max ratio to call the clock estimation in Controller_Init() */
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = 0;
    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
    {
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = 10;
    }
    else if ( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
	||(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) )
    {	/* Lowest frequency according to BKDG				*/
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = 8;

      if (PRIVATE(OF(Specific)) != NULL) {
	/*	Save the thermal parameters if specified		*/
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
      } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
      }
   }
	HyperThreading_Technology();
}

void Query_AuthenticAMD(unsigned int cpu)
{	/*	Fallback algorithm for unspecified AMD architectures.	*/
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL) {
	/*	Save thermal parameters for later use in the Daemon	*/
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	/*	Override Processor CodeName & Locking capabilities	*/
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	PUBLIC(RO(Proc))->Features.SpecTurboRatio = \
					Compute_AuthenticAMD_Boost(cpu);
	HyperThreading_Technology();
}

unsigned short Compute_AMD_Family_0Fh_Boost(unsigned int cpu)
{	/* Source: BKDG for AMD NPT Family 0Fh: §13.8			*/
	unsigned short SpecTurboRatio = 0;

    if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.FID == 1)
    {	/*		Processor supports FID changes .		*/
	FIDVID_STATUS FidVidStatus = {.value = 0};
	RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = \
						VCO[FidVidStatus.StartFID].MCF;

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = 8 + FidVidStatus.MaxFID;

	if (FidVidStatus.StartFID < 0b1000)
	{
		unsigned int t;
	    for (t = 0; t < 5; t++)
	    {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(SIZE)-5+t] = \
					VCO[FidVidStatus.StartFID].PCF[t];
	    }
		SpecTurboRatio = 5;
	} else {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C)] = \
							8 + FidVidStatus.MaxFID;
		SpecTurboRatio = 1;
	}
    } else {
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] =	8
					+ HwCfgRegister.Family_0Fh.StartFID;

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)];

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C) ] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)];
	SpecTurboRatio = 1;
    }
	return (SpecTurboRatio);
}

void Query_AMD_Family_0Fh(unsigned int cpu)
{
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL)
    {
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	PUBLIC(RO(Proc))->Features.SpecTurboRatio = \
					Compute_AMD_Family_0Fh_Boost(cpu);
	HyperThreading_Technology();
}

void Compute_AMD_Family_10h_Boost(unsigned int cpu)
{
	unsigned int pstate, sort[5] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 4; pstate++)
	{
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = \
					(PstateDef.Family_10h.CpuFid + 0x10)
					/ (1 << PstateDef.Family_10h.CpuDid);
	}
}

void Query_AMD_Family_10h(unsigned int cpu)
{
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL)
    {
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	Compute_AMD_Family_10h_Boost(cpu);
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 3;

	HyperThreading_Technology();
}

void Compute_AMD_Family_11h_Boost(unsigned int cpu)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++)
	{
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = \
					(PstateDef.Family_10h.CpuFid + 0x8)
					/ (1 << PstateDef.Family_10h.CpuDid);
	}
}

void Query_AMD_Family_11h(unsigned int cpu)
{
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL)
    {
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	Compute_AMD_Family_11h_Boost(cpu);
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Compute_AMD_Family_12h_Boost(unsigned int cpu)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++)
	{
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = \
					(PstateDef.Family_12h.CpuFid + 0x10)
					/  PstateDef.Family_12h.CpuDid;
	}
}

void Query_AMD_Family_12h(unsigned int cpu)
{
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL)
    {
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	Compute_AMD_Family_12h_Boost(cpu);
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Compute_AMD_Family_14h_Boost(unsigned int cpu)
{
	COFVID CofVid = {.value = 0};
	unsigned int MaxFreq = 100, ClockDiv;
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	RDMSR(CofVid, MSR_AMD_COFVID_STATUS);

	if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
		MaxFreq *= (CofVid.Arch_Pll.MainPllOpFidMax + 0x10);

	for (pstate = 0; pstate <= 7; pstate++)
	{
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		ClockDiv = (PstateDef.Family_14h.CpuDidMSD + 1) * 4;
		ClockDiv += PstateDef.Family_14h.CpuDidLSD;

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = (MaxFreq * 4)
							/ ClockDiv;
	}	/*	Frequency @ MainPllOpFidMax (MHz)		*/
}

void Query_AMD_Family_14h(unsigned int cpu)
{
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL)
    {
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	Compute_AMD_Family_14h_Boost(cpu);
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

inline unsigned int AMD_F15h_CoreCOF(unsigned int FID, unsigned int DID)
{/*	CoreCOF (MHz) = 100 * (CpuFid[5:0] + 10h) / (2 ^ CpuDid)	*/
	unsigned int COF = (FID + 0x10) / (1 << DID);

	return (COF);
}

inline unsigned int AMD_F15h_CoreFID(unsigned int COF, unsigned int DID)
{
	unsigned int FID = (COF * (1 << DID)) - 0x10;

	return (FID);
}

void Compute_AMD_Family_15h_Boost(unsigned int cpu)
{
    if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.HwPstate)
    {
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++)
	{
		PSTATEDEF PstateDef = {.value = 0};
		unsigned int COF;

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		COF = AMD_F15h_CoreCOF( PstateDef.Family_15h.CpuFid,
					PstateDef.Family_15h.CpuDid );

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = COF;
	}
    }
}

void Query_AMD_Family_15h(unsigned int cpu)
{
	Compute_AMD_Family_15h_Boost(cpu);
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();

  /* Find micro-architecture based on the CPUID model. Bulldozer initialized */
	PRIVATE(OF(Specific)) = LookupProcessor();
    switch (PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel) {
    case 0x0:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
	  Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_PILEDRIVER].CodeName,
		CODENAME_LEN);
      }
	break;
    case 0x1:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
	  Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_PILEDRIVER].CodeName,
		CODENAME_LEN);
      }
	break;
    case 0x3:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
	  Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_STEAMROLLER].CodeName,
			CODENAME_LEN);
      }
	break;
    case 0x6:
    case 0x7:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
	    Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_EXCAVATOR].CodeName,
		CODENAME_LEN);
      }
	break;
    };
    if (PRIVATE(OF(Specific)) != NULL)
    {
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();
}

inline unsigned int AMD_Zen_CoreCOF(unsigned int FID, unsigned int DID)
{/* Source: PPR for AMD Family 17h Model 01h, Revision B1 Processors
    CoreCOF = (PStateDef[CpuFid[7:0]] / PStateDef[CpuDfsId]) * 200	*/
	unsigned int COF;
	if (DID != 0) {
		COF = (FID << 1) / DID;
	} else {
		COF = FID >> 2;
	}
	return (COF);
}

inline unsigned int AMD_Zen_CoreFID(unsigned int COF, unsigned int DID)
{
	unsigned int FID;
	if (DID != 0) {
		FID = (COF * DID) >> 1;
	} else {
		FID = COF << 2;
	}
	return (FID);
}

bool Compute_AMD_Zen_Boost(unsigned int cpu)
{
	unsigned int COF = 0, pstate, sort[8] = { /* P[0..7]-States */
		BOOST(MAX), BOOST(1C), BOOST(2C), BOOST(3C),
		BOOST(4C) , BOOST(5C), BOOST(6C), BOOST(7C)
	};
	HWCR HwCfgRegister = {.value = 0};
	PSTATEDEF PstateDef = {.value = 0};
	PSTATECTRL PstateCtrl = {.value = 0};

    for (pstate = BOOST(MIN); pstate < BOOST(SIZE); pstate++) {
		PUBLIC(RO(Core, AT(cpu)))->Boost[pstate] = 0;
    }
    if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.HwPstate)
    {
	RDMSR(PstateDef, MSR_AMD_PC6_F17H_STATUS);
	if (PstateDef.Family_17h.CpuFid == 0)
	{
		PSTATELIMIT PstateLimit = {.value = 0};

		RDMSR(PstateLimit, MSR_AMD_PSTATE_CURRENT_LIMIT);

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE
				+ PstateLimit.Family_17h.PstateMaxVal));
	}
	COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
				PstateDef.Family_17h.CpuDfsId );

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = COF;
    } else {
	/*Core & L3 frequencies < 400MHz are not supported by the architecture*/
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = 4;
    }
	/* Loop over all frequency ids. */
    for (pstate = 0; pstate <= 7; pstate++)
    {
	RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));
	/* Handle only valid P-States. */
	if (PstateDef.Family_17h.PstateEn)
	{
		COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId );

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = COF;
	}
    }
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = pstate;

	/*		Read the Target P-State				*/
	RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
	RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + PstateCtrl.PstateCmd);

	COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
				PstateDef.Family_17h.CpuDfsId );

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TGT)] = COF;

	/*	If CPB is enabled then add Boost + XFR to the P0 ratio. */
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (HwCfgRegister.Family_17h.CpbDis == 0)
    {
	AMD_17_MTS_CPK_COF XtraCOF = {.value = 0};

	switch (PUBLIC(RO(Proc))->ArchID) {
	case AMD_Zen3_VMR:
	case AMD_Zen2_MTS:
		Core_AMD_SMN_Read(XtraCOF,
				SMU_AMD_F17H_MATISSE_COF,
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H);
		break;
	case AMD_Zen2_CPK:
		Core_AMD_SMN_Read(XtraCOF,
				SMU_AMD_F17H_CASTLEPEAK_COF,
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H);
		break;
	}
	if (XtraCOF.value != 0)
	{
		unsigned int	CPB = XtraCOF.BoostRatio >> 2,
				XFR = !!(XtraCOF.BoostRatio & 0b11);

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(CPB)] = CPB;
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(XFR)] = CPB + XFR;
	}
	if (PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(CPB)] == 0)
	{
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(CPB)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];

	    if (PRIVATE(OF(Specific)) != NULL)
	    {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(CPB)] += \
						PRIVATE(OF(Specific))->Boost[0];
	    }
	}
	if (PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(XFR)] == 0)
	{
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(XFR)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];

	    if (PRIVATE(OF(Specific)) != NULL)
	    {
		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(XFR)] += \
						PRIVATE(OF(Specific))->Boost[0];

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(XFR)] += \
						PRIVATE(OF(Specific))->Boost[1];
	    }
	}
	return (true);	/* Have CPB: inform to add the Xtra Boost ratios */
    } else {
	return (false); /* CPB is disabled				*/
    }
}

typedef struct {
	CLOCK_ARG *pClockMod;
	long	rc;
	unsigned long long PstateAddr;
	unsigned int BoostIndex;
} CLOCK_ZEN_ARG;

static void TargetClock_AMD_Zen_PerCore(void *arg)
{
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;
	unsigned int cpu = smp_processor_id();
	unsigned int COF, pstate,
		target	= (pClockZen->pClockMod->cpu == -1) ?
			pClockZen->pClockMod->Ratio
		:
			PUBLIC(RO(Core, AT(cpu)))->Boost[pClockZen->BoostIndex]
			+ pClockZen->pClockMod->Offset;

	unsigned short RdWrMSR = 0;

    if (target == 0) {
	pstate = 0;	/*	AUTO Frequency is requested by User	*/
	RdWrMSR = 1;
    } else {
    /* Look-up for the first enabled P-State with the same target frequency */
	for (pstate = 0; pstate <= 7; pstate++)
	{
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, pClockZen->PstateAddr + pstate);

	    if (PstateDef.Family_17h.PstateEn)
	    {
		COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId );
		if (COF == target) {
			RdWrMSR = 1;
			break;
		}
	    }
	}
    }
    if (RdWrMSR == 1)
    {
	PSTATECTRL PstateCtrl = {.value = 0};
	/*		Command a new target P-state			*/
	RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
	PstateCtrl.PstateCmd = pstate;
	WRMSR(PstateCtrl, MSR_AMD_PERF_CTL);
	pClockZen->rc = RC_OK_COMPUTE;
    } else {
	pClockZen->rc = -RC_PSTATE_NOT_FOUND;
    }
}

static void TurboClock_AMD_Zen_PerCore(void *arg)
{
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;
	PSTATEDEF PstateDef = {.value = 0};
	HWCR HwCfgRegister = {.value = 0};
	unsigned int COF, FID;
	/*	Make sure the Core Performance Boost is disabled.	*/
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
  if (HwCfgRegister.Family_17h.CpbDis)
  {
	/*	Apply if and only if the P-State is enabled ?		*/
	RDMSR(PstateDef, pClockZen->PstateAddr);
    if (PstateDef.Family_17h.PstateEn)
    {
	if (pClockZen->pClockMod->cpu == -1)
	{
		COF = pClockZen->pClockMod->Ratio;
	} else {
		/* Compute the new Frequency ID with the COF offset	*/
		COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId );

		COF = COF + pClockZen->pClockMod->Offset;
	}
	FID = AMD_Zen_CoreFID(COF, PstateDef.Family_17h.CpuDfsId);
	/*		Write the P-State MSR with the new FID		*/
	PstateDef.Family_17h.CpuFid = FID;
	WRMSR(PstateDef, pClockZen->PstateAddr);

	pClockZen->rc = RC_OK_COMPUTE;
    } else {
	pClockZen->rc = -ENODEV;
    }
  } else {
	pClockZen->rc = -RC_TURBO_PREREQ;
  }
}

static void BaseClock_AMD_Zen_PerCore(void *arg)
{
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;
	PSTATEDEF PstateDef = {.value = 0};
	COMPUTE_ARG Compute = {	.TSC = { NULL, NULL } };

	Compute.TSC[0] = kmalloc(STRUCT_SIZE, GFP_KERNEL);
    if (Compute.TSC[0] == NULL) {
	goto OutOfMemory;
    }
	Compute.TSC[1] = kmalloc(STRUCT_SIZE, GFP_KERNEL);
    if (Compute.TSC[1] == NULL) {
	goto OutOfMemory;
    }
	TurboClock_AMD_Zen_PerCore(arg);

    if (pClockZen->rc == RC_OK_COMPUTE)
    {
	const unsigned int cpu = smp_processor_id();
	/*			Calibration Phase One			*/
	RDMSR(PstateDef, pClockZen->PstateAddr);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = \
			AMD_Zen_CoreCOF(PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId);

	cpu_data(cpu).loops_per_jiffy = \
		COMPUTE_LPJ(	PUBLIC(RO(Core, AT(cpu)))->Clock.Hz,
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] );
	/*			Calibration Phase Two			*/
	Compute.Clock.Q = PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];
	Compute.Clock.R = 0;
	Compute.Clock.Hz = 0;

	Compute_TSC(&Compute);

	PUBLIC(RO(Core, AT(cpu)))->Clock = Compute.Clock;
	/*			Calibration Phase Three 		*/
	RDMSR(PstateDef, pClockZen->PstateAddr);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = \
			AMD_Zen_CoreCOF(PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId);

	cpu_data(cpu).loops_per_jiffy = \
		COMPUTE_LPJ(	PUBLIC(RO(Core, AT(cpu)))->Clock.Hz,
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] );
    }
OutOfMemory:
    if (Compute.TSC[1] != NULL) {
	kfree(Compute.TSC[1]);
    } else {
	pClockZen->rc = -ENOMEM;
    }
    if (Compute.TSC[0] != NULL) {
	kfree(Compute.TSC[0]);
    } else {
	pClockZen->rc = -ENOMEM;
    }
}

long For_All_AMD_Zen_Clock(CLOCK_ZEN_ARG *pClockZen, void (*PerCore)(void *))
{
	long rc = RC_SUCCESS;
	unsigned int cpu = PUBLIC(RO(Proc))->CPU.Count;

  do {
	cpu--;	/* From last AP to BSP */

    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)
    && ((pClockZen->pClockMod->cpu == -1)||(pClockZen->pClockMod->cpu == cpu)))
    {
	smp_call_function_single(cpu, PerCore, pClockZen, 1);
	rc = pClockZen->rc;
    }
  } while ((cpu != 0) && (rc >= RC_SUCCESS)) ;

	return (rc);
}

long For_All_AMD_Zen_BaseClock(CLOCK_ZEN_ARG *pClockZen, void (*PerCore)(void*))
{
	long rc;
	unsigned int cpu = PUBLIC(RO(Proc))->Service.Core;

	CoreFreqK_UnRegister_ClockSource();

	rc = For_All_AMD_Zen_Clock(pClockZen, PerCore);

    if (rc == RC_OK_COMPUTE)
    {
	PUBLIC(RO(Proc))->Features.Factory.Clock = (CLOCK) {.Q=0, .R=0, .Hz=0};
	PUBLIC(RO(Proc))->Features.Factory.Ratio = 0;
	PUBLIC(RO(Proc))->Features.Factory.Freq = 0;

	FixMissingRatioAndFrequency(PUBLIC(RO(Core,AT(cpu)))->Boost[BOOST(MAX)],
					 &PUBLIC(RO(Core, AT(cpu)))->Clock);

	loops_per_jiffy = cpu_data(cpu).loops_per_jiffy;

	cpu_khz = tsc_khz = (unsigned int) ((loops_per_jiffy * HZ) / 1000LU);
    }
	CoreFreqK_Register_ClockSource(cpu);

	return (rc);
}

long TurboClock_AMD_Zen(CLOCK_ARG *pClockMod)
{
  if (pClockMod != NULL) {
    if ((pClockMod->NC >= 1) && (pClockMod->NC <= 7))
    {
      if (PUBLIC(RO(Proc))->Registration.Experimental)
      {
	CLOCK_ZEN_ARG ClockZen = {	/* P[1..7]-States allowed	*/
		.pClockMod  = pClockMod,
		.PstateAddr = MSR_AMD_PSTATE_DEF_BASE + pClockMod->NC,
		.BoostIndex = BOOST(SIZE) - pClockMod->NC,
		.rc = RC_SUCCESS
	};
	return (For_All_AMD_Zen_Clock(&ClockZen, TurboClock_AMD_Zen_PerCore));
      } else {
	return (-RC_EXPERIMENTAL);
      }
    } else {
	return (-RC_UNIMPLEMENTED);
    }
  } else {
	return (-EINVAL);
  }
}

long ClockMod_AMD_Zen(CLOCK_ARG *pClockMod)
{
  if (pClockMod != NULL) {
    switch (pClockMod->NC) {
    case CLOCK_MOD_MAX:
      if (PUBLIC(RO(Proc))->Registration.Experimental)
      {
	CLOCK_ZEN_ARG ClockZen = {	/* P[0]:Max non-boosted P-State */
		.pClockMod  = pClockMod,
		.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
		.BoostIndex = BOOST(MAX),
		.rc = RC_SUCCESS
	};
	return(For_All_AMD_Zen_BaseClock(&ClockZen, BaseClock_AMD_Zen_PerCore));
      } else {
	return (-RC_EXPERIMENTAL);
      }
    case CLOCK_MOD_TGT:
      {
		CLOCK_ZEN_ARG ClockZen = {	/* Target non-boosted P-State*/
			.pClockMod  = pClockMod,
			.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
			.BoostIndex = BOOST(TGT),
			.rc = RC_SUCCESS
		};
	return (For_All_AMD_Zen_Clock(&ClockZen, TargetClock_AMD_Zen_PerCore));
      }
    default:
	return (-RC_UNIMPLEMENTED);
    }
  } else {
	return (-EINVAL);
  }
}

void Query_AMD_Family_17h(unsigned int cpu)
{
	PRIVATE(OF(Specific)) = LookupProcessor();
    if (PRIVATE(OF(Specific)) != NULL)
    {
	/*	Save thermal parameters for later use in the Daemon	*/
	PUBLIC(RO(Proc))->PowerThermal.Param = PRIVATE(OF(Specific))->Param;
	/*	Override Processor CodeName & Locking capabilities	*/
	OverrideCodeNameString(PRIVATE(OF(Specific)));
	OverrideUnlockCapability(PRIVATE(OF(Specific)));
    } else {
	PUBLIC(RO(Proc))->PowerThermal.Param.Target = 0;
    }
	Default_Unlock_Reset();

	switch (PUBLIC(RO(Proc))->ArchID) {
	case AMD_EPYC_Rome:
	case AMD_Zen2_CPK:
	case AMD_Zen2_APU:
	case AMD_Zen2_MTS:
	case AMD_Zen3_VMR:
	    {
		AMD_17_MTS_CPK_TJMAX TjMax = {.value = 0};

		Core_AMD_SMN_Read(	TjMax,
					SMU_AMD_F17H_MTS_CPK_TJMAX,
					SMU_AMD_INDEX_REGISTER_F17H,
					SMU_AMD_DATA_REGISTER_F17H );

		PUBLIC(RO(Proc))->PowerThermal.Param.Offset[0] = TjMax.Target;

		Core_AMD_Family_17h_Temp = CCD_AMD_Family_17h_Zen2_Temp;
	    }
		break;
	default:
		Core_AMD_Family_17h_Temp = CTL_AMD_Family_17h_Temp;
		break;
	}

	if (Compute_AMD_Zen_Boost(cpu) == true)
	{	/*	Count the Xtra Boost ratios			*/
		PUBLIC(RO(Proc))->Features.TDP_Levels = 2;
	}
	else {	/*	Disabled CPB: Hide ratios			*/
		PUBLIC(RO(Proc))->Features.TDP_Levels = 0;
	}
	/*	Apply same register bit fields as Intel RAPL_POWER_UNIT */
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.Unit, MSR_AMD_RAPL_POWER_UNIT);

	HyperThreading_Technology();

	AMD_Processor_PIN(PUBLIC(RO(Proc))->Features.leaf80000008.EBX.PPIN);
}

void Dump_CPUID(CORE_RO *Core)
{
	unsigned int i;

	__asm__ volatile
	(
		"xorq	%%rax, %%rax	\n\t"
		"xorq	%%rbx, %%rbx	\n\t"
		"xorq	%%rcx, %%rcx	\n\t"
		"xorq	%%rdx, %%rdx	\n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (Core->Query.StdFunc.LargestStdFunc),
		  "=r" (Core->Query.StdFunc.BX),
		  "=r" (Core->Query.StdFunc.CX),
		  "=r" (Core->Query.StdFunc.DX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile
	(
		"movq	$0x80000000, %%rax	\n\t"
		"xorq	%%rbx, %%rbx		\n\t"
		"xorq	%%rcx, %%rcx		\n\t"
		"xorq	%%rdx, %%rdx		\n\t"
		"cpuid				\n\t"
		"mov	%%eax, %0		\n\t"
		"mov	%%ebx, %1		\n\t"
		"mov	%%ecx, %2		\n\t"
		"mov	%%edx, %3"
		: "=r" (Core->Query.ExtFunc.LargestExtFunc),
		  "=r" (Core->Query.ExtFunc.EBX),
		  "=r" (Core->Query.ExtFunc.ECX),
		  "=r" (Core->Query.ExtFunc.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
/* Intel: CPUID INSTRUCTION
   If a value is entered for CPUID.EAX is invalid for a particular processor,
   the data for the highest basic information leaf is returned.
*/
	for (i = 0; i < CPUID_MAX_FUNC; i++)
	{	__asm__ volatile
		(
			"xorq	%%rax, %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"mov	%4,    %%eax	\n\t"
			"mov	%5,    %%ecx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (Core->CpuID[i].reg[0]),
			  "=r" (Core->CpuID[i].reg[1]),
			  "=r" (Core->CpuID[i].reg[2]),
			  "=r" (Core->CpuID[i].reg[3])
			:  "r" (Core->CpuID[i].func),
			   "r" (Core->CpuID[i].sub)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
}

void SpeedStep_Technology(CORE_RO *Core)			/*Per Package*/
{
  if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
    if (PUBLIC(RO(Proc))->Features.Std.ECX.EIST == 1)
    {
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	switch (SpeedStep_Enable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatures.EIST = SpeedStep_Enable;
		WRMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
		RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
		break;
	}
	if (MiscFeatures.EIST)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SpeedStep, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SpeedStep, Core->Bind);
	}
    } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SpeedStep, Core->Bind);
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SpeedStep_Mask, Core->Bind);
  }
}

void Intel_Turbo_Config(CORE_RO *Core, void (*ConfigFunc)(void*),/*Per Package*/
			void (*AssignFunc)(unsigned int*, TURBO_CONFIG*))
{
	CLOCK_TURBO_ARG ClockTurbo = {
		.pClockMod = NULL,	/*	Read-Only Operation	*/
		.Config = {.MSR = {.value = 0}},
		.rc = RC_SUCCESS
	};

	ConfigFunc(&ClockTurbo);
	AssignFunc(Core->Boost, &ClockTurbo.Config);
}

typedef void (*SET_TARGET)(CORE_RO*, unsigned int);

static void Set_Core2_Target(CORE_RO *Core, unsigned int ratio)
{
	Core->PowerThermal.PerfControl.CORE.TargetRatio = ratio;
}

static void Set_Nehalem_Target(CORE_RO *Core, unsigned int ratio)
{
	Core->PowerThermal.PerfControl.NHM.TargetRatio = ratio & 0xff;
}

static void Set_SandyBridge_Target(CORE_RO *Core, unsigned int ratio)
{
	Core->PowerThermal.PerfControl.SNB.TargetRatio = ratio;
}

#define GET_CORE2_TARGET(Core)						\
(									\
	Core->PowerThermal.PerfControl.CORE.TargetRatio 		\
)

typedef unsigned int (*GET_TARGET)(CORE_RO*);

static unsigned int Get_Core2_Target(CORE_RO *Core)
{
	return (GET_CORE2_TARGET(Core));
}

#define GET_NEHALEM_TARGET(Core)					\
(									\
	Core->PowerThermal.PerfControl.NHM.TargetRatio & 0xff		\
)

static unsigned int Get_Nehalem_Target(CORE_RO *Core)
{
	return (GET_NEHALEM_TARGET(Core));
}

#define GET_SANDYBRIDGE_TARGET(Core)					\
(									\
	Core->PowerThermal.PerfControl.SNB.TargetRatio			\
)

static unsigned int Get_SandyBridge_Target(CORE_RO *Core)
{
	return (GET_SANDYBRIDGE_TARGET(Core));
}

typedef int (*CMP_TARGET)(CORE_RO*, unsigned int);

static int Cmp_Core2_Target(CORE_RO *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.CORE.TargetRatio == ratio);
}

static int Cmp_Nehalem_Target(CORE_RO *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.NHM.TargetRatio == ratio);
}

static int Cmp_SandyBridge_Target(CORE_RO *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.SNB.TargetRatio > ratio);
}

bool IsPerformanceControlCapable(void)
{
	struct SIGNATURE blackList[] = {
		_Silvermont_Bay_Trail,	/* 06_37 */
		_Atom_Merrifield,	/* 06_4A */
		_Atom_Avoton,		/* 06_4D */
		_Atom_Moorefield,	/* 06_5A */
		_Atom_Sofia		/* 06_5D */
	};
	const int ids = sizeof(blackList) / sizeof(blackList[0]);
	int id;
     for (id = 0; id < ids; id++) {
      if((blackList[id].ExtFamily==PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
      && (blackList[id].Family == PUBLIC(RO(Proc))->Features.Std.EAX.Family)
      && (blackList[id].ExtModel == PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
      && (blackList[id].Model == PUBLIC(RO(Proc))->Features.Std.EAX.Model))
      {
	return (PUBLIC(RO(Proc))->Registration.Driver.CPUfreq == 1);
      }
     }
	return (true);
}

bool WritePerformanceControl(PERF_CONTROL *pPerfControl)
{
	bool isCapable = IsPerformanceControlCapable();
	if (isCapable == true)
	{
		WRMSR((*pPerfControl), MSR_IA32_PERF_CTL);
	}
	return (isCapable);
}

void TurboBoost_Technology(CORE_RO *Core,	SET_TARGET SetTarget,
						GET_TARGET GetTarget,
						CMP_TARGET CmpTarget,
						unsigned int TurboRatio,
						unsigned int ValidRatio)
{								/* Per SMT */
	int ToggleFeature;
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

  if ( (MiscFeatures.Turbo_IDA == 0)
	&& (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA) )
  {
    switch (TurboBoost_Enable) {
    case COREFREQ_TOGGLE_OFF:	/*	Restore the nominal P-state	*/
	Core->PowerThermal.PerfControl.Turbo_IDA = 1;
	SetTarget(Core, Core->Boost[BOOST(MAX)]);
	ToggleFeature = 1;
	break;
    case COREFREQ_TOGGLE_ON:	/*	Request the Turbo P-state	*/
	Core->PowerThermal.PerfControl.Turbo_IDA = 0;
	SetTarget(Core, TurboRatio);
	ToggleFeature = 1;
	break;
    default:
	ToggleFeature = 0;
	break;
    }
    if (ToggleFeature == 1)
    {
	WritePerformanceControl(&Core->PowerThermal.PerfControl);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
    }
    if (Core->PowerThermal.PerfControl.Turbo_IDA == 0)
    {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
    } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
    }
  } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
  }

  if (PUBLIC(RO(Proc))->Features.HWP_Enable)
  {
	RDMSR(Core->PowerThermal.HWP_Capabilities, MSR_IA32_HWP_CAPABILITIES);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

    if (PUBLIC(RO(Proc))->Features.Power.EAX.HWP_EPP)
    {		/*		EPP mode				*/
	if ((HWP_EPP >= 0) && (HWP_EPP <= 0xff))
	{
		Core->PowerThermal.HWP_Request.Energy_Pref = HWP_EPP;
		WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
		RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	}
    } else {	/*		EPB fallback mode			*/
	if (PUBLIC(RO(Proc))->Registration.Driver.CPUfreq
			& (REGISTRATION_ENABLE | REGISTRATION_FULLCTRL) )
	{
		/*	Turbo is a function of the Target P-state	*/
	    if (!CmpTarget(Core, ValidRatio))
	    {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    }
	}
    }
	Core->Boost[BOOST(HWP_MIN)]=Core->PowerThermal.HWP_Request.Minimum_Perf;
	Core->Boost[BOOST(HWP_MAX)]=Core->PowerThermal.HWP_Request.Maximum_Perf;
	Core->Boost[BOOST(HWP_TGT)]=Core->PowerThermal.HWP_Request.Desired_Perf;
  } else {	/*			EPB mode			*/
	if (PUBLIC(RO(Proc))->Registration.Driver.CPUfreq
			& (REGISTRATION_ENABLE | REGISTRATION_FULLCTRL) )
	{
	    if (!CmpTarget(Core, ValidRatio))
	    {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    }
	}
  }
}

void DynamicAcceleration(CORE_RO *Core) 			/* Unique */
{
  if (IsPerformanceControlCapable() == true)
  {
    if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
    {
	TurboBoost_Technology(	Core,
				Set_Core2_Target,
				Get_Core2_Target,
				Cmp_Core2_Target,
				1 + Core->Boost[BOOST(MAX)],
				1 + Core->Boost[BOOST(MAX)] );
    } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);
    }
  } else {
	int ToggleFeature;

	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	switch (TurboBoost_Enable) {
	case COREFREQ_TOGGLE_OFF:
		MiscFeatures.Turbo_IDA = 1;
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		MiscFeatures.Turbo_IDA = 0;
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1)
	{
		WRMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
		RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
		/*	Refresh the Turbo capability in the leaf.	*/
	    if (PUBLIC(RO(Proc))->Features.Info.LargestStdFunc >= 0x6) {
		__asm__ volatile
		(
			"movq	$0x6,  %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (PUBLIC(RO(Proc))->Features.Power.EAX),
			  "=r" (PUBLIC(RO(Proc))->Features.Power.EBX),
			  "=r" (PUBLIC(RO(Proc))->Features.Power.ECX),
			  "=r" (PUBLIC(RO(Proc))->Features.Power.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	    }
	}
	if (MiscFeatures.Turbo_IDA == 0)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);
  }
}

void SoC_Turbo_Override(CORE_RO *Core)
{
	int ToggleFeature;

	PKG_TURBO_CONFIG TurboCfg = {.value = 0};
	RDMSR(TurboCfg, MSR_PKG_TURBO_CFG);

	switch (TurboBoost_Enable) {
	case COREFREQ_TOGGLE_OFF:
		TurboCfg.TjMax_Turbo = 0x0;
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		TurboCfg.TjMax_Turbo = 0x2;
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if ((ToggleFeature == 1)
	 && (Core->Bind == PUBLIC(RO(Proc))->Service.Core))	/* Package */
	{
		WRMSR(TurboCfg, MSR_PKG_TURBO_CFG);
		RDMSR(TurboCfg, MSR_PKG_TURBO_CFG);
	}
	if (TurboCfg.TjMax_Turbo == 0x2)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);
}

typedef struct {
	CLOCK_ARG *pClockMod;
	SET_TARGET SetTarget;
	GET_TARGET GetTarget;
	long	rc;
} CLOCK_PPC_ARG;

static void ClockMod_PPC_PerCore(void *arg)
{
	CLOCK_PPC_ARG *pClockPPC;
	CORE_RO *Core;
	unsigned int cpu;

	pClockPPC = (CLOCK_PPC_ARG *) arg;
	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	pClockPPC->SetTarget(Core, (pClockPPC->pClockMod->cpu == -1) ?
		pClockPPC->pClockMod->Ratio
	:	pClockPPC->GetTarget(Core) + pClockPPC->pClockMod->Offset);

	if (WritePerformanceControl(&Core->PowerThermal.PerfControl) == true) {
		pClockPPC->rc = RC_OK_COMPUTE;
	} else {
		pClockPPC->rc = -EINTR;
	}
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	Core->Boost[BOOST(TGT)] = pClockPPC->GetTarget(Core);
}

long For_All_PPC_Clock(CLOCK_PPC_ARG *pClockPPC)
{
	long rc = RC_SUCCESS;
	unsigned int cpu = PUBLIC(RO(Proc))->CPU.Count;
  do {
	cpu--;	/*		From last AP to BSP			*/

    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)
    && ((pClockPPC->pClockMod->cpu == -1)||(pClockPPC->pClockMod->cpu == cpu)))
    {
	smp_call_function_single(cpu, ClockMod_PPC_PerCore, pClockPPC, 1);
	rc = pClockPPC->rc;
    }
  } while ((cpu != 0) && (rc >= RC_SUCCESS)) ;
	return (rc);
}

long ClockMod_Core2_PPC(CLOCK_ARG *pClockMod)
{
	if (pClockMod != NULL) {
		if (pClockMod->NC == CLOCK_MOD_TGT)
		{
			CLOCK_PPC_ARG ClockPPC = {
				.pClockMod = pClockMod,
				.SetTarget = Set_Core2_Target,
				.GetTarget = Get_Core2_Target,
				.rc = RC_SUCCESS
			};
			return ( For_All_PPC_Clock(&ClockPPC) );
		} else {
			return (-RC_UNIMPLEMENTED);
		}
	} else {
		return (-EINVAL);
	}
}

long ClockMod_Nehalem_PPC(CLOCK_ARG *pClockMod)
{
	if (pClockMod != NULL) {
		if (pClockMod->NC == CLOCK_MOD_TGT)
		{
			CLOCK_PPC_ARG ClockPPC = {
				.pClockMod = pClockMod,
				.SetTarget = Set_Nehalem_Target,
				.GetTarget = Get_Nehalem_Target,
				.rc = RC_SUCCESS
			};
			return ( For_All_PPC_Clock(&ClockPPC) );
		} else {
			return (-RC_UNIMPLEMENTED);
		}
	} else {
		return (-EINVAL);
	}
}

long ClockMod_SandyBridge_PPC(CLOCK_ARG *pClockMod)
{
	if (pClockMod != NULL) {
		if (pClockMod->NC == CLOCK_MOD_TGT)
		{
			CLOCK_PPC_ARG ClockPPC = {
				.pClockMod = pClockMod,
				.SetTarget = Set_SandyBridge_Target,
				.GetTarget = Get_SandyBridge_Target,
				.rc = RC_SUCCESS
			};
			return ( For_All_PPC_Clock(&ClockPPC) );
		} else {
			return (-RC_UNIMPLEMENTED);
		}
	} else {
		return (-EINVAL);
	}
}

typedef struct {
	CLOCK_ARG *pClockMod;
	long	rc;
} CLOCK_HWP_ARG;

static void ClockMod_HWP_PerCore(void *arg)
{
	CLOCK_HWP_ARG *pClockHWP;
	CORE_RO *Core;
	unsigned int cpu;
	unsigned short WrRdHWP;

	pClockHWP = (CLOCK_HWP_ARG *) arg;
	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

  switch (pClockHWP->pClockMod->NC) {
  case CLOCK_MOD_HWP_MIN:
    if (pClockHWP->pClockMod->cpu == -1) {
    Core->PowerThermal.HWP_Request.Minimum_Perf = pClockHWP->pClockMod->Ratio;
    } else {
    Core->PowerThermal.HWP_Request.Minimum_Perf += pClockHWP->pClockMod->Offset;
    }
	WrRdHWP = 1;
	break;
  case CLOCK_MOD_HWP_MAX:
    if (pClockHWP->pClockMod->cpu == -1) {
    Core->PowerThermal.HWP_Request.Maximum_Perf = pClockHWP->pClockMod->Ratio;
    } else {
    Core->PowerThermal.HWP_Request.Maximum_Perf += pClockHWP->pClockMod->Offset;
    }
	WrRdHWP = 1;
	break;
  case CLOCK_MOD_HWP_TGT:
    if (pClockHWP->pClockMod->cpu == -1) {
    Core->PowerThermal.HWP_Request.Desired_Perf = pClockHWP->pClockMod->Ratio;
    } else {
    Core->PowerThermal.HWP_Request.Desired_Perf += pClockHWP->pClockMod->Offset;
    }
	WrRdHWP = 1;
	break;
  default:
	WrRdHWP = 0;
	pClockHWP->rc = -RC_UNIMPLEMENTED;
	break;
  }
  if (WrRdHWP == 1)
  {
	WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	pClockHWP->rc = RC_OK_COMPUTE;
  }
}

long For_All_HWP_Clock(CLOCK_HWP_ARG *pClockHWP)
{
	long rc = RC_SUCCESS;
	unsigned int cpu = PUBLIC(RO(Proc))->CPU.Count;
  do {
	cpu--;	/*	From last AP to BSP				*/

    if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)
    && ((pClockHWP->pClockMod->cpu == -1)||(pClockHWP->pClockMod->cpu == cpu)))
	{
	smp_call_function_single(cpu, ClockMod_HWP_PerCore, pClockHWP, 1);
	rc = pClockHWP->rc;
	}
  } while ((cpu != 0) && (rc >= RC_SUCCESS)) ;
	return (rc);
}

long ClockMod_Intel_HWP(CLOCK_ARG *pClockMod)
{
	if (PUBLIC(RO(Proc))->Features.HWP_Enable) {
		if (pClockMod != NULL) {
			switch (pClockMod->NC) {
			case CLOCK_MOD_HWP_MIN:
			case CLOCK_MOD_HWP_MAX:
			case CLOCK_MOD_HWP_TGT:
			    {
				CLOCK_HWP_ARG ClockHWP = {
					.pClockMod = pClockMod,
					.rc = RC_SUCCESS
				};
				return ( For_All_HWP_Clock(&ClockHWP) );
			    }
			case CLOCK_MOD_TGT:
				return ( ClockMod_SandyBridge_PPC(pClockMod) );
			default:
				return (-RC_UNIMPLEMENTED);
			}
		} else {
			return (-EINVAL);
		}
	} else {
		return ( ClockMod_SandyBridge_PPC(pClockMod) );
	}
}

void PerCore_Query_AMD_Zen_Features(CORE_RO *Core)		/* Per SMT */
{
	CSTATE_BASE_ADDR CStateBaseAddr = {.value = 0};
	unsigned long long CC6 = 0, PC6 = 0;
	int ToggleFeature;

	/*		Read The Hardware Configuration Register.	*/
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);

	/* Query the SMM. */
	if (HwCfgRegister.Family_17h.SmmLock) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM, Core->Bind);
	}
	/*		Enable or Disable the Core Performance Boost.	*/
	switch (TurboBoost_Enable) {
	case COREFREQ_TOGGLE_OFF:
		HwCfgRegister.Family_17h.CpbDis = 1;
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		HwCfgRegister.Family_17h.CpbDis = 0;
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1)
	{
		WRMSR(HwCfgRegister, MSR_K7_HWCR);
		RDMSR(HwCfgRegister, MSR_K7_HWCR);
	}
	if (HwCfgRegister.Family_17h.CpbDis == 0)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);

	/*	Enable or Disable the Core C6 State. Bit[22,14,16]	*/
	RDMSR64(CC6, MSR_AMD_CC6_F17H_STATUS);
	switch (CC6_Enable) {
	case COREFREQ_TOGGLE_OFF:
		BITCLR(LOCKLESS, CC6, 22);
		BITCLR(LOCKLESS, CC6, 14);
		BITCLR(LOCKLESS, CC6,  6);
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		BITSET(LOCKLESS, CC6, 22);
		BITSET(LOCKLESS, CC6, 14);
		BITSET(LOCKLESS, CC6,  6);
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1)
	{
		WRMSR64(CC6, MSR_AMD_CC6_F17H_STATUS);
		RDMSR64(CC6, MSR_AMD_CC6_F17H_STATUS);
	}
	if ((BITWISEAND(LOCKLESS, CC6, 0x404040LLU) == 0x404040LLU)
	  && HwCfgRegister.Family_17h.INVDWBINVD)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	/*	Enable or Disable the Package C6 State . Bit[32]	*/
	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		RDMSR64(PC6, MSR_AMD_PC6_F17H_STATUS);
		switch (PC6_Enable) {
		case COREFREQ_TOGGLE_OFF:
			BITCLR(LOCKLESS, PC6, 32);
			ToggleFeature = 1;
			break;
		case COREFREQ_TOGGLE_ON:
			BITSET(LOCKLESS, PC6, 32);
			ToggleFeature = 1;
			break;
		default:
			ToggleFeature = 0;
			break;
		}
		if (ToggleFeature == 1) {
			WRMSR64(PC6, MSR_AMD_PC6_F17H_STATUS);
			RDMSR64(PC6, MSR_AMD_PC6_F17H_STATUS);
		}
		if(BITWISEAND(LOCKLESS, PC6, 0x100000000LLU) == 0x100000000LLU)
		{
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6, Core->Bind);
		}
		BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PC6_Mask, Core->Bind);
	}
	/*		Core C-State Base Address.			*/
	RDMSR(CStateBaseAddr, MSR_AMD_CSTATE_BAR);
	Core->Query.CStateBaseAddr = CStateBaseAddr.IOaddr;
	/*		Package C-State: Configuration Control .	*/
	Core->Query.CfgLock = 1;
	/*		Package C-State: I/O MWAIT Redirection .	*/
	Core->Query.IORedir = 0;
}

void Intel_Turbo_TDP_Config(CORE_RO *Core)
{
	TURBO_ACTIVATION TurboActivation = {.value = 0};
	CONFIG_TDP_NOMINAL NominalTDP = {.value = 0};
	CONFIG_TDP_CONTROL ControlTDP = {.value = 0};
	CONFIG_TDP_LEVEL ConfigTDP;

	RDMSR(TurboActivation, MSR_TURBO_ACTIVATION_RATIO);
	Core->Boost[BOOST(ACT)] = TurboActivation.MaxRatio;

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	PUBLIC(RO(Proc))->Features.TurboActiv_Lock = TurboActivation.Ratio_Lock;
    }

	RDMSR(NominalTDP, MSR_CONFIG_TDP_NOMINAL);
	Core->Boost[BOOST(TDP)] = NominalTDP.Ratio;

	ConfigTDP.value = 0;
	RDMSR(ConfigTDP, MSR_CONFIG_TDP_LEVEL_2);
	Core->Boost[BOOST(TDP2)] = ConfigTDP.Ratio;

	ConfigTDP.value = 0;
	RDMSR(ConfigTDP, MSR_CONFIG_TDP_LEVEL_1);
	Core->Boost[BOOST(TDP1)] = ConfigTDP.Ratio;

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	RDMSR(ControlTDP, MSR_CONFIG_TDP_CONTROL);
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Lock  = ControlTDP.Lock;
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Level = ControlTDP.Level;
    }
}

void Query_Intel_C1E(CORE_RO *Core)				/*Per Package*/
{
	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		POWER_CONTROL PowerCtrl = {.value = 0};
		RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);

		switch (C1E_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				PowerCtrl.C1E = C1E_Enable;
				WRMSR(PowerCtrl, MSR_IA32_POWER_CTL);
				RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);
			break;
		}
		if (PowerCtrl.C1E)
		{
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E, Core->Bind);
		}
		BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask, Core->Bind);
	}
}

void Query_AMD_Family_0Fh_C1E(CORE_RO *Core)			/* Per Core */
{
	INT_PENDING_MSG IntPendingMsg = {.value = 0};

	RDMSR(IntPendingMsg, MSR_K8_INT_PENDING_MSG);

	if (IntPendingMsg.C1eOnCmpHalt & !IntPendingMsg.SmiOnCmpHalt)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask, Core->Bind);
}

void ThermalMonitor2_Set( CORE_RO *Core )	/* Intel Core Solo Duo. */
{
	struct SIGNATURE whiteList[] = {
		_Core_Yonah,		/* 06_0E */
		_Core_Conroe,		/* 06_0F */
		_Core_Penryn,		/* 06_17 */
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft,		/* 06_27 */
		_Atom_Clover_Trail,	/* 06_35 */
		_Atom_Saltwell,		/* 06_36 */
	};
	int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
  for (id = 0; id < ids; id++)
  {
    if((whiteList[id].ExtFamily == PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
    && (whiteList[id].Family == PUBLIC(RO(Proc))->Features.Std.EAX.Family)
    && (whiteList[id].ExtModel == PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
    && (whiteList[id].Model == PUBLIC(RO(Proc))->Features.Std.EAX.Model))
    {
	if (Core->PowerThermal.TCC_Enable)
	{
		THERM2_CONTROL Therm2Control = {.value = 0};

		RDMSR(Therm2Control, MSR_THERM2_CTL);

		if (Therm2Control.TM_SELECT)
		{
			Core->PowerThermal.TCC_Enable = 0;
			Core->PowerThermal.TM2_Enable = 1;
		} else {
			Core->PowerThermal.TM2_Enable = 0;
		}
	}
	break;
    }
  }
}

void ThermalMonitor_Set(CORE_RO *Core)
{
	TJMAX TjMax = {.value = 0};
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	THERM_STATUS ThermStatus = {.value = 0};
	PLATFORM_INFO PfInfo = {.value = 0};
	int ClearBit;

	/* Silvermont + Xeon[06_57] + Nehalem + Sandy Bridge & superior arch. */
	RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);

	Core->PowerThermal.Param.Offset[0] = TjMax.Target;
	if (Core->PowerThermal.Param.Offset[0] == 0)
	{
		Core->PowerThermal.Param.Offset[0] = 100;
	}

	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	Core->PowerThermal.TCC_Enable = MiscFeatures.TCC;	/* alias TM1 */
	Core->PowerThermal.TM2_Enable = MiscFeatures.TM2_Enable;

	ThermalMonitor2_Set(Core);

	/* Clear Thermal Events if requested. */
	ClearBit = 0;
	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);

	if (Clear_Events & EVENT_THERM_SENSOR) {
		ThermStatus.StatusLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THERM_PROCHOT) {
		ThermStatus.PROCHOTLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THERM_CRIT) {
		ThermStatus.CriticalTempLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THERM_THOLD) {
		ThermStatus.Threshold1Log = 0;
		ThermStatus.Threshold2Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_POWER_LIMIT) {
		ThermStatus.PwrLimitLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CURRENT_LIMIT) {
		ThermStatus.CurLimitLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CROSS_DOMAIN) {
		ThermStatus.XDomLimitLog = 0;
		ClearBit = 1;
	}
	if (ClearBit) {
		WRMSR(ThermStatus, MSR_IA32_THERM_STATUS);
		RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);
	}
	Core->PowerThermal.Events = (	(ThermStatus.StatusBit
					|ThermStatus.StatusLog ) << 0)
				  | (ThermStatus.PROCHOTLog << 1)
				  | (ThermStatus.CriticalTempLog << 2)
				  | (	(ThermStatus.Threshold1Log
					|ThermStatus.Threshold2Log ) << 3)
				  | (ThermStatus.PwrLimitLog << 4)
				  | (ThermStatus.CurLimitLog << 5)
				  | (ThermStatus.XDomLimitLog << 6);

	if (PUBLIC(RO(Proc))->Features.Power.EAX.PTM
	&& (Core->Bind == PUBLIC(RO(Proc))->Service.Core))
	{
		ClearBit = 0;
		ThermStatus.value = 0;
		RDMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);

		if (Clear_Events & EVENT_THERM_SENSOR) {
			ThermStatus.StatusLog = 0;
			ClearBit = 1;
		}
		if (Clear_Events & EVENT_THERM_PROCHOT) {
			ThermStatus.PROCHOTLog = 0;
			ClearBit = 1;
		}
		if (Clear_Events & EVENT_THERM_CRIT) {
			ThermStatus.CriticalTempLog = 0;
			ClearBit = 1;
		}
		if (Clear_Events & EVENT_THERM_THOLD) {
			ThermStatus.Threshold1Log = 0;
			ThermStatus.Threshold2Log = 0;
			ClearBit = 1;
		}
		if (Clear_Events & EVENT_POWER_LIMIT) {
			ThermStatus.PwrLimitLog = 0;
			ClearBit = 1;
		}
		if (ClearBit) {
			WRMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);
			RDMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);
		}
		PUBLIC(RO(Proc))->PowerThermal.Events = ((ThermStatus.StatusBit
						|ThermStatus.StatusLog) << 0)
					  | (ThermStatus.PROCHOTLog << 1)
					  | (ThermStatus.CriticalTempLog << 2)
					  | (	(ThermStatus.Threshold1Log
						|ThermStatus.Threshold2Log)<< 3)
					  | (ThermStatus.PwrLimitLog << 4);
	}

	RDMSR(PfInfo, MSR_PLATFORM_INFO);

	if (PfInfo.ProgrammableTj)
	{
		switch (PUBLIC(RO(Proc))->ArchID) {
		case Atom_Goldmont:
		case Xeon_Phi:
	/*TODO( case 06_85h: )	*/
			Core->PowerThermal.Param.Offset[1] = TjMax.Atom.Offset;
			break;
		case IvyBridge_EP:
		case Broadwell_EP:
		case Broadwell_D:
		case Skylake_X:
			Core->PowerThermal.Param.Offset[1] = TjMax.EP.Offset;
			break;
		default:
			Core->PowerThermal.Param.Offset[1] = TjMax.Offset;
			break;
		case Core_Yonah ... Core_Dunnington:
			break;
		}
	}
}

void PowerThermal(CORE_RO *Core)
{
	struct {
		struct SIGNATURE Arch;
		unsigned short	grantPWR_MGMT	:  1-0,
				grantODCM	:  2-1,
				experimental	:  3-2,
				freeToUse	: 16-3;
	} whiteList[] = {
		{_Core_Yonah,		0, 1, 1, 0},
		{_Core_Conroe,		0, 1, 0, 0},
		{_Core_Kentsfield,	0, 1, 1, 0},
		{_Core_Conroe_616,	0, 1, 1, 0},
		{_Core_Penryn,		0, 1, 0, 0},
		{_Core_Dunnington,	0, 1, 1, 0},

		{_Atom_Bonnell ,	0, 1, 1, 0},	/* 06_1C */
		{_Atom_Silvermont,	0, 1, 1, 0},	/* 06_26 */
		{_Atom_Lincroft,	0, 1, 1, 0},	/* 06_27 */
		{_Atom_Clover_Trail,	0, 1, 1, 0},	/* 06_35 */
		{_Atom_Saltwell,	0, 1, 1, 0},	/* 06_36 */

		{_Silvermont_Bay_Trail, 0, 1, 0, 0},	/* 06_37 */

		{_Atom_Avoton,		0, 1, 1, 0},	/* 06_4D */
		{_Atom_Airmont ,	0, 0, 1, 0},	/* 06_4C */
		{_Atom_Goldmont,	1, 0, 1, 0},	/* 06_5C */
		{_Atom_Sofia,		0, 1, 1, 0},	/* 06_5D */
		{_Atom_Merrifield,	0, 1, 1, 0},	/* 06_4A */
		{_Atom_Moorefield,	0, 1, 1, 0},	/* 06_5A */

		{_Nehalem_Bloomfield,	1, 1, 0, 0},	/* 06_1A */
		{_Nehalem_Lynnfield,	1, 1, 0, 0},	/* 06_1E */
		{_Nehalem_MB,		1, 1, 0, 0},	/* 06_1F */
		{_Nehalem_EX,		1, 1, 0, 0},	/* 06_2E */

		{_Westmere,		1, 1, 0, 0},	/* 06_25 */
		{_Westmere_EP,		1, 1, 0, 0},	/* 06_2C */
		{_Westmere_EX,		1, 1, 0, 0},	/* 06_2F */

		{_SandyBridge,		1, 1, 0, 0},	/* 06_2A */
		{_SandyBridge_EP,	1, 1, 0, 0},	/* 06_2D */

		{_IvyBridge,		1, 0, 1, 0},	/* 06_3A */
		{_IvyBridge_EP ,	1, 1, 0, 0},	/* 06_3E */

		{_Haswell_DT,		1, 1, 0, 0},	/* 06_3C */
		{_Haswell_EP,		1, 1, 1, 0},	/* 06_3F */
		{_Haswell_ULT,		1, 1, 0, 0},	/* 06_45 */
		{_Haswell_ULX,		1, 1, 1, 0},	/* 06_46 */

		{_Broadwell,		1, 1, 1, 0},	/* 06_3D */
		{_Broadwell_D,		1, 1, 1, 0},	/* 06_56 */
		{_Broadwell_H,		1, 1, 1, 0},	/* 06_47 */
		{_Broadwell_EP ,	1, 1, 1, 0},	/* 06_4F */

		{_Skylake_UY,		1, 1, 1, 0},	/* 06_4E */
		{_Skylake_S,		1, 0, 0, 0},	/* 06_5E */
		{_Skylake_X,		1, 1, 1, 0},	/* 06_55 */

		{_Xeon_Phi,		0, 1, 1, 0},	/* 06_57 */

		{_Kabylake,		1, 1, 0, 0},	/* 06_9E */
		{_Kabylake_UY,		1, 1, 1, 0},	/* 06_8E */

		{_Cannonlake,		1, 1, 1, 0},	/* 06_66 */
		{_Geminilake,		1, 0, 1, 0},	/* 06_7A */
		{_Icelake_UY,		1, 1, 1, 0},	/* 06_7E */
	};
	unsigned int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
 for (id = 0; id < ids; id++)
 {
 if((whiteList[id].Arch.ExtFamily==PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
  && (whiteList[id].Arch.Family == PUBLIC(RO(Proc))->Features.Std.EAX.Family)
  && (whiteList[id].Arch.ExtModel==PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
  && (whiteList[id].Arch.Model == PUBLIC(RO(Proc))->Features.Std.EAX.Model))
  {
	break;
  }
 }
  if (PUBLIC(RO(Proc))->Features.Info.LargestStdFunc >= 0x6)
  {
    struct THERMAL_POWER_LEAF Power = {
	.EAX = {0}, .EBX = {0}, .ECX = {{0}}, .EDX = {0}
    };

    __asm__ volatile
    (
	"movq	$0x6,  %%rax	\n\t"
	"xorq	%%rbx, %%rbx	\n\t"
	"xorq	%%rcx, %%rcx	\n\t"
	"xorq	%%rdx, %%rdx	\n\t"
	"cpuid			\n\t"
	"mov	%%eax, %0	\n\t"
	"mov	%%ebx, %1	\n\t"
	"mov	%%ecx, %2	\n\t"
	"mov	%%edx, %3"
	: "=r" (Power.EAX),
	  "=r" (Power.EBX),
	  "=r" (Power.ECX),
	  "=r" (Power.EDX)
	:
	: "%rax", "%rbx", "%rcx", "%rdx"
    );
    if (Power.ECX.SETBH == 1)
    {
      if ((id < ids) && (whiteList[id].grantPWR_MGMT == 1))
      {
	RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);

	switch (PowerMGMT_Unlock) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
	  if (!whiteList[id].experimental
	   || (whiteList[id].experimental
	   && PUBLIC(RO(Proc))->Registration.Experimental))
	  {
	    Core->PowerThermal.PwrManagement.Perf_BIAS_Enable=PowerMGMT_Unlock;
	    WRMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
	    RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
	  }
	     break;
	}
	if (Core->PowerThermal.PwrManagement.Perf_BIAS_Enable)
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PowerMgmt, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PowerMgmt, Core->Bind);
      } else
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PowerMgmt, Core->Bind);

      if ((PowerPolicy >= 0) && (PowerPolicy <= 15))
      {
	if (!whiteList[id].experimental
	 || (whiteList[id].experimental
	 && PUBLIC(RO(Proc))->Registration.Experimental))
	{
	Core->PowerThermal.PerfEnergyBias.PowerPolicy = PowerPolicy;
	WRMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	}
      }
    }
    else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PowerMgmt, Core->Bind);
    }
    if ((PUBLIC(RO(Proc))->Features.Std.EDX.ACPI == 1)
     && (id < ids) && (whiteList[id].grantODCM == 1))
    {
	CLOCK_MODULATION ClockModulation = {.value = 0};
	int ToggleFeature = 0;

	RDMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);
	ClockModulation = Core->PowerThermal.ClockModulation;

	switch (ODCM_Enable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		ClockModulation.ODCM_Enable = ODCM_Enable;
		ToggleFeature = 1;
		break;
	}
	if ((ODCM_DutyCycle >= 0)
	 && (ODCM_DutyCycle <= (7 << Power.EAX.ECMD)))
	{
	    ClockModulation.DutyCycle = ODCM_DutyCycle << !Power.EAX.ECMD;
	    ToggleFeature = 1;
	}
	if (ToggleFeature == 1)
	{
	  if (!whiteList[id].experimental
	   || (whiteList[id].experimental
	   && PUBLIC(RO(Proc))->Registration.Experimental))
	  {
	    WRMSR(ClockModulation, MSR_IA32_THERM_CONTROL);
	    RDMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);
	  }
	}
	Core->PowerThermal.ClockModulation.ECMD = Power.EAX.ECMD;
	if (Core->PowerThermal.ClockModulation.ODCM_Enable)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->ODCM, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->ODCM, Core->Bind);
	}
    }
    else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->ODCM, Core->Bind);
    }
  }
  else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PowerMgmt, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->ODCM, Core->Bind);
  }
  BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask, Core->Bind);
  BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PowerMgmt_Mask, Core->Bind);
}

#define UNSPEC 0b11111111

struct CSTATES_ENCODING_ST {
	enum CSTATES_ENCODING	enc;
	unsigned short int	dec;
} Limit_CSTATES_NHM[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },
	{  _C1	, 0b0001 },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0010 }, /*Cannot be used to limit package C-State*/
	{  _C4	, UNSPEC },
	{  _C6	, 0b0011 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0100 },
	{ _C7S	, UNSPEC },
	{  _C8	, UNSPEC },
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, IORedir_CSTATES_NHM[CSTATES_ENCODING_COUNT] = {
	{  _C0	, UNSPEC },
	{  _C1	, UNSPEC },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0000 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0001 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0010 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0011 },	/* TODO(Undefined!)	*/
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, Limit_CSTATES_SNB[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },
	{  _C1	, 0b0000 },
	{  _C2	, 0b0001 },
	{  _C3	, 0b0010 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0011 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0100 },
	{ _C7S	, 0b0101 },
	{  _C8	, 0b0110 },
	{  _C9	, 0b0111 },
	{ _C10	, 0b1000 }
}, IORedir_CSTATES_SNB[CSTATES_ENCODING_COUNT] = {
	{  _C0	, UNSPEC },
	{  _C1	, UNSPEC },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0000 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0001 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0010 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0011 },	/* TODO(Untested?)	*/
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, Limit_CSTATES_ULT[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },
	{  _C1	, 0b0000 },
	{  _C2	, 0b0001 },
	{  _C3	, 0b0010 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0011 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0100 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0110 },
	{  _C9	, 0b0111 },
	{ _C10	, 0b1000 }
}, IORedir_CSTATES_ULT[CSTATES_ENCODING_COUNT] = {
	{  _C0	, UNSPEC },
	{  _C1	, UNSPEC },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0000 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0001 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0010 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0011 },	/* TODO(Untested?)	*/
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, Limit_CSTATES_SKL[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },
	{  _C1	, 0b0000 },
	{  _C2	, 0b0001 },
	{  _C3	, 0b0010 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0011 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0100 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0110 },
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, IORedir_CSTATES_SKL[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },
	{  _C1	, 0b0001 },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0010 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0011 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0100 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0101 },
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, Limit_CSTATES_SOC_SLM[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },	/* Silvermont, Airmont	*/
	{  _C1	, 0b0001 },	/* Silvermont, Airmont	*/
	{  _C2	, 0b0010 },	/* Airmont		*/
	{  _C3	, UNSPEC },
	{  _C4	, 0b0100 },	/* Silvermont		*/
	{  _C6	, 0b0110 },	/* Silvermont, Airmont	*/
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0111 },	/* Silvermont, Airmont	*/
	{ _C7S	, UNSPEC },
	{  _C8	, UNSPEC },
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, IORedir_CSTATES_SOC_SLM[CSTATES_ENCODING_COUNT] = {
	{  _C0	, UNSPEC },
	{  _C1	, UNSPEC },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0000 },	/* Airmont		*/
	{  _C4	, 0b0100 },	/* Silvermont: 0b0100	*/
	{  _C6	, 0b0001 },	/* Silvermont: 0b0110, Atom Deep Power Down */
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0010 },	/* Silvermont: 0b0111, Airmont: 0b0010	*/
	{ _C7S	, UNSPEC },
	{  _C8	, UNSPEC },
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
}, Limit_CSTATES_SOC_GDM[CSTATES_ENCODING_COUNT] = {
	{  _C0	, 0b0000 },
	{  _C1	, 0b0001 },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0010 },	/* Goldmont, Tremont	*/
	{  _C4	, UNSPEC },
	{  _C6	, 0b0011 },	/* Goldmont, Tremont	*/
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0100 },	/* Goldmont, Tremont	*/
	{ _C7S	, 0b0101 },	/* Goldmont, Tremont	*/
	{  _C8	, 0b0110 },	/* Goldmont, Tremont	*/
	{  _C9	, 0b0111 },	/* Goldmont, Tremont	*/
	{ _C10	, 0b1000 }
}, IORedir_CSTATES_SOC_GDM[CSTATES_ENCODING_COUNT] = {
	{  _C0	, UNSPEC },
	{  _C1	, UNSPEC },
	{  _C2	, UNSPEC },
	{  _C3	, 0b0000 },
	{  _C4	, UNSPEC },
	{  _C6	, 0b0001 },
	{ _C6R	, UNSPEC },
	{  _C7	, 0b0010 },
	{ _C7S	, UNSPEC },
	{  _C8	, 0b0011 },	/* TODO(Untested?)	*/
	{  _C9	, UNSPEC },
	{ _C10	, UNSPEC }
};

#define MAKE_TOGGLE_CSTATE_FUNC( _type, _feature, _parameter )		\
inline unsigned int Toggle_CState_##_feature( _type *pConfigRegister,	\
					typeof(_parameter) _parameter)  \
{									\
	switch ( _parameter )						\
	{								\
	case COREFREQ_TOGGLE_OFF:					\
	case COREFREQ_TOGGLE_ON:					\
		pConfigRegister->_feature = _parameter;			\
		return (1);						\
	}								\
	return (0);							\
}

MAKE_TOGGLE_CSTATE_FUNC(CSTATE_CONFIG, C3autoDemotion, C3A_Enable)
MAKE_TOGGLE_CSTATE_FUNC(CSTATE_CONFIG, C1autoDemotion, C1A_Enable)
MAKE_TOGGLE_CSTATE_FUNC(CSTATE_CONFIG, C3undemotion, C3U_Enable)
MAKE_TOGGLE_CSTATE_FUNC(CSTATE_CONFIG, C1undemotion, C1U_Enable)
MAKE_TOGGLE_CSTATE_FUNC(CSTATE_CONFIG, IO_MWAIT_Redir, IOMWAIT_Enable)
MAKE_TOGGLE_CSTATE_FUNC(CC6_CONFIG, CC6demotion, CC6_Enable)
MAKE_TOGGLE_CSTATE_FUNC(MC6_CONFIG, MC6demotion, PC6_Enable)

#undef MAKE_TOGGLE_CSTATE

#define Toggle_CState_Feature( _config, _feature, _parameter )		\
(									\
	Toggle_CState_##_feature( _config, _parameter ) 		\
)

#define For_All_Encodings(	loopCondition, breakStatement,		\
				bodyStatement, closure )		\
({									\
	unsigned int idx, ret = 0;					\
    for (idx = 0; idx < CSTATES_ENCODING_COUNT && (loopCondition); idx++)\
    {									\
	if (breakStatement)						\
	{								\
		ret = 1;						\
		bodyStatement						\
		break;							\
	}								\
    }									\
	closure								\
	ret;								\
})

void Control_IO_MWAIT(	struct CSTATES_ENCODING_ST IORedir[],
			CORE_RO *Core )
{
	CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
	RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*		Core C-State Base Address.			*/
	Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;

    if (Core->Query.IORedir)
    {
	For_All_Encodings(
			/* loopCondition: */
			(CStateIORedir >= 0),
			/* breakStatement: */
			( (IORedir[idx].dec != UNSPEC)
			&& (CStateIORedir == IORedir[idx].enc) ),
			/* bodyStatement: */
			{
			CState_IO_MWAIT.CStateRange = IORedir[idx].dec;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			},
			/* closure: */
			{}
	);
    }
	For_All_Encodings(
		/* loopCondition: */ (1),
		/* breakStatement: */
		((CState_IO_MWAIT.CStateRange & 0b111) == IORedir[idx].dec),
		/* bodyStatement: */
		{
		Core->Query.CStateInclude = IORedir[idx].enc;
		},
		/* closure: */
		if (!ret) {
			Core->Query.CStateInclude = _UNSPEC;
		}
	);
}

void Control_CSTATES_NHM(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{	/* Family: 06_1A, 06_1E, 06_1F, 06_25, 06_2C, 06_2E		*/
	CSTATE_CONFIG CStateConfig = {.value = 0};
	unsigned int toggleFeature = 0;

	RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						C3autoDemotion,
						C3A_Enable );

	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						C1autoDemotion,
						C1A_Enable );
    if (CStateConfig.CFG_Lock == 0)
    {
	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						IO_MWAIT_Redir,
						IOMWAIT_Enable);

	toggleFeature |= For_All_Encodings(
				/* loopCondition: */
				(PkgCStateLimit >= 0),
				/* breakStatement: */
				( (Limit[idx].dec != UNSPEC)
				&& (PkgCStateLimit == Limit[idx].enc) ),
				/* bodyStatement: */
				{
				CStateConfig.Pkg_CStateLimit = Limit[idx].dec
				| (0b1000 & CStateConfig.Pkg_CStateLimit);
				},
				/* closure: */
				{}
	);
    }
	if (toggleFeature == 1) {
		WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
	}

	if (CStateConfig.C3autoDemotion)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C3A, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C3A, Core->Bind);
	}
	if (CStateConfig.C1autoDemotion)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1A, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1A, Core->Bind);
	}

	Core->Query.CfgLock = CStateConfig.CFG_Lock;
	Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;

	For_All_Encodings(
		/* loopCondition: */ (1),
		/* breakStatement: */
		((CStateConfig.Pkg_CStateLimit & 0b0111) == Limit[idx].dec),
		/* bodyStatement: */
		{
			Core->Query.CStateLimit = Limit[idx].enc;
		},
		/* closure: */
		if (!ret) {
			Core->Query.CStateLimit = _UNSPEC;
		}
	);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask, Core->Bind);

	Control_IO_MWAIT(IORedir, Core);
}

void Control_CSTATES_COMMON(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core,
				const unsigned short bitMask,
				const unsigned short unMask )
{
	CSTATE_CONFIG CStateConfig = {.value = 0};
	unsigned int toggleFeature = 0;

	RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						C3autoDemotion,
						C3A_Enable );

	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						C1autoDemotion,
						C1A_Enable );

	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						C3undemotion,
						C3U_Enable );

	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						C1undemotion,
						C1U_Enable );
    if (CStateConfig.CFG_Lock == 0)
    {
	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						IO_MWAIT_Redir,
						IOMWAIT_Enable);

	toggleFeature |= For_All_Encodings(
				/* loopCondition: */
				(PkgCStateLimit >= 0),
				/* breakStatement: */
				( (Limit[idx].dec != UNSPEC)
				&& (PkgCStateLimit == Limit[idx].enc) ),
				/* bodyStatement: */
				{
				CStateConfig.Pkg_CStateLimit = Limit[idx].dec
				| (unMask & CStateConfig.Pkg_CStateLimit);
				},
				/* closure: */
				{}
	);
    }
	if (toggleFeature == 1) {
		WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
	}

	if (CStateConfig.C3autoDemotion)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C3A, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C3A, Core->Bind);
	}
	if (CStateConfig.C1autoDemotion)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1A, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1A, Core->Bind);
	}
	if (CStateConfig.C3undemotion)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C3U, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C3U, Core->Bind);
	}
	if (CStateConfig.C1undemotion)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1U, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1U, Core->Bind);
	}

	Core->Query.CfgLock = CStateConfig.CFG_Lock;
	Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;

	For_All_Encodings(
		/* loopCondition: */ (1),
		/* breakStatement: */
		((CStateConfig.Pkg_CStateLimit & bitMask) == Limit[idx].dec),
		/* bodyStatement: */
		{
			Core->Query.CStateLimit = Limit[idx].enc;
		},
		/* closure: */
		if (!ret) {
			Core->Query.CStateLimit = _UNSPEC;
		}
	);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask, Core->Bind);

	Control_IO_MWAIT(IORedir, Core);
}

void Control_CSTATES_SNB(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{	/* Family: 06_2A, 06_3A, 06_3E, 06_3F, 06_4F, 06_56, 06_57, 06_85 */
	Control_CSTATES_COMMON(Limit, IORedir, Core, 0b0111, 0b1000);
}

void Control_CSTATES_ULT(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{	/* Family: 06_3C, 06_3D, 06_45, 06_46, 06_47, 06_86		*/
	Control_CSTATES_COMMON(Limit, IORedir, Core, 0b1111, 0b0000);
}

void Control_CSTATES_SKL(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{	/* Family: 06_4E, 06_5E, 06_55, 06_66, 06_7D, 06_7E, 06_8E, 06_9E */
	Control_CSTATES_COMMON(Limit, IORedir, Core, 0b0111, 0b1000);
}

void Control_CSTATES_SOC_ATOM(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core,
				const unsigned short bitMask,
				const unsigned short unMask )
{
	CSTATE_CONFIG CStateConfig = {.value = 0};
	unsigned int toggleFeature = 0;

	RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

    if (CStateConfig.CFG_Lock == 0)
    {
	toggleFeature |= Toggle_CState_Feature(	&CStateConfig,
						IO_MWAIT_Redir,
						IOMWAIT_Enable);

	toggleFeature |= For_All_Encodings(
				/* loopCondition: */
				(PkgCStateLimit >= 0),
				/* breakStatement: */
				( (Limit[idx].dec != UNSPEC)
				&& (PkgCStateLimit == Limit[idx].enc) ),
				/* bodyStatement: */
				{
				CStateConfig.Pkg_CStateLimit = Limit[idx].dec
				| (unMask & CStateConfig.Pkg_CStateLimit);
				},
				/* closure: */
				{}
	);
    }
	if (toggleFeature == 1) {
		WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
	}

	Core->Query.CfgLock = CStateConfig.CFG_Lock;
	Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;

	For_All_Encodings(
		/* loopCondition: */ (1),
		/* breakStatement: */
		((CStateConfig.Pkg_CStateLimit & bitMask) == Limit[idx].dec),
		/* bodyStatement: */
		{
			Core->Query.CStateLimit = Limit[idx].enc;
		},
		/* closure: */
		if (!ret) {
			Core->Query.CStateLimit = _UNSPEC;
		}
	);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask, Core->Bind);

	Control_IO_MWAIT(IORedir, Core);
}

void Control_CSTATES_SOC_SLM(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{	/* Family: 06_37, 06_4A, 06_4C, 06_4D, 06_5A			*/
	CC6_CONFIG CC6_Config = {.value = 0};
	MC6_CONFIG MC6_Config = {.value = 0};

	Control_CSTATES_SOC_ATOM(Limit, IORedir, Core, 0b0111, 0b1000);

	RDMSR(CC6_Config, MSR_CC6_DEMOTION_POLICY_CONFIG);
    if (Toggle_CState_Feature(&CC6_Config, CC6demotion, CC6_Enable))
    {
	WRMSR(CC6_Config, MSR_CC6_DEMOTION_POLICY_CONFIG);
	RDMSR(CC6_Config, MSR_CC6_DEMOTION_POLICY_CONFIG);
    }

	RDMSR(MC6_Config, MSR_MC6_DEMOTION_POLICY_CONFIG);
    if (Toggle_CState_Feature(&MC6_Config, MC6demotion, PC6_Enable))
    {
	WRMSR(MC6_Config, MSR_MC6_DEMOTION_POLICY_CONFIG);
	RDMSR(MC6_Config, MSR_MC6_DEMOTION_POLICY_CONFIG);
    }

    if (CC6_Config.CC6demotion)
    {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6, Core->Bind);
    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6, Core->Bind);
    }
    if (MC6_Config.MC6demotion)
    {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6, Core->Bind);
    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6, Core->Bind);
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PC6_Mask, Core->Bind);
}

void Control_CSTATES_SOC_GDM(	struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{	/* Family: 06_5CH						*/
	Control_CSTATES_SOC_ATOM(Limit, IORedir, Core, 0b1111, 0b0000);
}

#undef UNSPEC
#undef For_All_Encodings
#undef Toggle_CState_Feature

#define Intel_CStatesConfiguration( _CLASS, Core )			\
({									\
	Control_##_CLASS(Limit_##_CLASS, IORedir_##_CLASS, Core);	\
})

void PerCore_AMD_Family_0Fh_PStates(CORE_RO *Core)
{
	FIDVID_STATUS FidVidStatus = {.value = 0};
	FIDVID_CONTROL FidVidControl = {.value = 0};
	int NewFID = -1, NewVID = -1, loop = 100;

	RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

	NewFID	= ((PState_FID >= FidVidStatus.StartFID)
		&& (PState_FID <= FidVidStatus.MaxFID)) ?
		PState_FID : FidVidStatus.CurrFID,

	NewVID	= ((PState_VID <= FidVidStatus.StartVID)
		&& (PState_VID >= FidVidStatus.MaxVID)) ?
		PState_VID : FidVidStatus.CurrVID;
	do {
		if (FidVidStatus.FidVidPending == 0) {
			RDMSR(FidVidControl, MSR_K7_FID_VID_CTL);

			FidVidControl.InitFidVid = 1;
			FidVidControl.StpGntTOCnt = 400;
			FidVidControl.NewFID = NewFID;
			FidVidControl.NewVID = NewVID;

			WRMSR(FidVidControl, MSR_K7_FID_VID_CTL);
			loop = 0;
		} else {
			RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);
		}
		if (loop == 0) {
			break;
		} else {
			loop-- ;
		}
	} while (FidVidStatus.FidVidPending == 1) ;
}

void SystemRegisters(CORE_RO *Core)
{
	if (RDPMC_Enable) {
		__asm__ volatile
		(
			"movq	%%cr4, %%rax"	"\n\t"
			"btsq	%0,    %%rax"	"\n\t"
			"movq	%%rax, %%cr4"	"\n\t"
			"wbinvd"
			:
			: "i" (CR4_PCE)
			: "%rax"
		);
	}
	__asm__ volatile
	(
		"# RFLAGS"		"\n\t"
		"pushfq"		"\n\t"
		"popq	%0"		"\n\t"
		"movq	%%cr0, %1"	"\n\t"
		"movq	%%cr3, %2"	"\n\t"
		"movq	%%cr4, %3"	"\n\t"
		"# EFER"		"\n\t"
		"xorq	%%rax, %%rax"	"\n\t"
		"xorq	%%rdx, %%rdx"	"\n\t"
		"movq	%5,%%rcx"	"\n\t"
		"rdmsr"			"\n\t"
		"shlq	$32, %%rdx"	"\n\t"
		"orq	%%rdx, %%rax"	"\n\t"
		"movq	%%rax, %4"
		: "=r" (Core->SystemRegister.RFLAGS),
		  "=r" (Core->SystemRegister.CR0),
		  "=r" (Core->SystemRegister.CR3),
		  "=r" (Core->SystemRegister.CR4),
		  "=r" (Core->SystemRegister.EFER)
		: "i" (MSR_EFER)
		: "%rax", "%rcx", "%rdx"
	);
	if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL) {
		__asm__ volatile
		(
			"# EFCR"		"\n\t"
			"xorq	%%rax, %%rax"	"\n\t"
			"xorq	%%rdx, %%rdx"	"\n\t"
			"movq	%1,%%rcx"	"\n\t"
			"rdmsr"			"\n\t"
			"shlq	$32, %%rdx"	"\n\t"
			"orq	%%rdx, %%rax"	"\n\t"
			"movq	%%rax, %0"
			: "=r" (Core->SystemRegister.EFCR)
			: "i" (MSR_IA32_FEAT_CTL)
			: "%rax", "%rcx", "%rdx"
		);
		/*		Virtualization Technology.		*/
		if (BITVAL(Core->SystemRegister.EFCR, EXFCR_VMX_IN_SMX)
		  | BITVAL(Core->SystemRegister.EFCR, EXFCR_VMXOUT_SMX))
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->VM, Core->Bind);
	}
	else if ( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		||(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) )
	{
		RDMSR(Core->SystemRegister.VMCR, MSR_VM_CR);
		/*		Secure Virtual Machine .		*/
		if (!Core->SystemRegister.VMCR.SVME_Disable
		  && Core->SystemRegister.VMCR.SVM_Lock)
		{
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->VM, Core->Bind);
		}
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CR_Mask, Core->Bind);
}

void Intel_Mitigation_Mechanisms(CORE_RO *Core)
{
	SPEC_CTRL Spec_Ctrl = {.value = 0};
	PRED_CMD  Pred_Cmd  = {.value = 0};
	FLUSH_CMD Flush_Cmd = {.value = 0};
	unsigned short WrRdMSR = 0;

	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.IBRS_IBPB_Cap
	 || PUBLIC(RO(Proc))->Features.ExtFeature.EDX.STIBP_Cap
	 || PUBLIC(RO(Proc))->Features.ExtFeature.EDX.SSBD_Cap)
	{
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.IBRS_IBPB_Cap
	&& ((Mech_IBRS == COREFREQ_TOGGLE_OFF)
	 || (Mech_IBRS == COREFREQ_TOGGLE_ON)))
	{
		Spec_Ctrl.IBRS = Mech_IBRS;
		WrRdMSR = 1;
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.STIBP_Cap
	&& ((Mech_STIBP == COREFREQ_TOGGLE_OFF)
	 || (Mech_STIBP == COREFREQ_TOGGLE_ON)))
	{
		Spec_Ctrl.STIBP = Mech_STIBP;
		WrRdMSR = 1;
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.SSBD_Cap
	&& ((Mech_SSBD == COREFREQ_TOGGLE_OFF)
	 || (Mech_SSBD == COREFREQ_TOGGLE_ON)))
	{
		Spec_Ctrl.SSBD = Mech_SSBD;
		WrRdMSR = 1;
	}
	if (WrRdMSR == 1)
	{
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 11)
		x86_spec_ctrl_base = Spec_Ctrl.value;
	    #endif
		WRMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);
	}
	if (Spec_Ctrl.IBRS) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS, Core->Bind);
	}
	if (Spec_Ctrl.STIBP) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->STIBP, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->STIBP, Core->Bind);
	}
	if (Spec_Ctrl.SSBD) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBD, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBD, Core->Bind);
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.IBRS_IBPB_Cap
	&& ((Mech_IBPB == COREFREQ_TOGGLE_OFF)
	 || (Mech_IBPB == COREFREQ_TOGGLE_ON)))
	{
		Pred_Cmd.IBPB = Mech_IBPB;
		WRMSR(Pred_Cmd, MSR_IA32_PRED_CMD);
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.L1D_FLUSH_Cap
	&& ((Mech_L1D_FLUSH == COREFREQ_TOGGLE_OFF)
	 || (Mech_L1D_FLUSH == COREFREQ_TOGGLE_ON)))
	{
		Flush_Cmd.L1D_FLUSH_CMD = Mech_L1D_FLUSH;
		WRMSR(Flush_Cmd, MSR_IA32_FLUSH_CMD);
	}
    if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.IA32_ARCH_CAP)
    {
		ARCH_CAPABILITIES Arch_Cap = {.value = 0};

		RDMSR(Arch_Cap, MSR_IA32_ARCH_CAPABILITIES);

	if (Arch_Cap.RDCL_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RDCL_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RDCL_NO, Core->Bind);
	}
	if (Arch_Cap.IBRS_ALL) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS_ALL, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS_ALL, Core->Bind);
	}
	if (Arch_Cap.RSBA) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RSBA, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RSBA, Core->Bind);
	}
	if (Arch_Cap.L1DFL_VMENTRY_NO) {
	    BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1DFL_VMENTRY_NO,Core->Bind);
	} else {
	    BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1DFL_VMENTRY_NO,Core->Bind);
	}
	if (Arch_Cap.SSB_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SSB_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSB_NO, Core->Bind);
	}
	if (Arch_Cap.MDS_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->MDS_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->MDS_NO, Core->Bind);
	}
	if (Arch_Cap.PSCHANGE_MC_NO) {
	    BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PSCHANGE_MC_NO, Core->Bind);
	} else {
	    BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSCHANGE_MC_NO, Core->Bind);
	}
	if (Arch_Cap.TAA_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TAA_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TAA_NO, Core->Bind);
	}
    }
    if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.IA32_CORE_CAP)
    {
	CORE_CAPABILITIES Core_Cap = {.value = 0};

	RDMSR(Core_Cap, MSR_IA32_CORE_CAPABILITIES);

	if (Core_Cap.SPLA_EXCEPTION) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA, Core->Bind);
	}
    } else {
	/* Source: arch/x86/kernel/cpu/intel.c				*/
	struct SIGNATURE whiteList[] = {
		_Icelake_UY,		/* 06_7E */
		_Icelake_X,		/* 06_6A */
		_Tremont_Jacobsville,	/* 06_86 */
		_Tremont_Elkhartlake,	/* 06_96 */
		_Tremont_Jasperlake	/* 06_9C */
	};
	const int ids = sizeof(whiteList) / sizeof(whiteList[0]);
	int id;
     for (id = 0; id < ids; id++) {
      if((whiteList[id].ExtFamily==PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
      && (whiteList[id].Family == PUBLIC(RO(Proc))->Features.Std.EAX.Family)
      && (whiteList[id].ExtModel == PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
      && (whiteList[id].Model == PUBLIC(RO(Proc))->Features.Std.EAX.Model))
      {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA, Core->Bind);
	break;
      }
     }
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask, Core->Bind);
}

void AMD_Mitigation_Mechanisms(CORE_RO *Core)
{
	AMD_SPEC_CTRL Spec_Ctrl = {.value = 0};
	AMD_PRED_CMD  Pred_Cmd  = {.value = 0};
	unsigned short WrRdMSR = 0;

    if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.IBRS
     || PUBLIC(RO(Proc))->Features.leaf80000008.EBX.STIBP
     || PUBLIC(RO(Proc))->Features.leaf80000008.EBX.SSBD)
    {
	RDMSR(Spec_Ctrl, MSR_AMD_SPEC_CTRL);

	if ((Mech_IBRS == COREFREQ_TOGGLE_OFF)
	 || (Mech_IBRS == COREFREQ_TOGGLE_ON))
	{
		Spec_Ctrl.IBRS = Mech_IBRS;
		WrRdMSR = 1;
	}
	if ((Mech_STIBP == COREFREQ_TOGGLE_OFF)
	 || (Mech_STIBP == COREFREQ_TOGGLE_ON))
	{
		Spec_Ctrl.STIBP = Mech_STIBP;
		WrRdMSR = 1;
	}
	if ((Mech_SSBD == COREFREQ_TOGGLE_OFF)
	 || (Mech_SSBD == COREFREQ_TOGGLE_ON))
	{
		Spec_Ctrl.SSBD = Mech_SSBD;
		WrRdMSR = 1;
	}
	if (WrRdMSR == 1)
	{
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 11)
		x86_spec_ctrl_base = Spec_Ctrl.value;
	    #endif
		WRMSR(Spec_Ctrl, MSR_AMD_SPEC_CTRL);
		RDMSR(Spec_Ctrl, MSR_AMD_SPEC_CTRL);
	}
	if (Spec_Ctrl.IBRS) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS, Core->Bind);
	}
	if (Spec_Ctrl.STIBP) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->STIBP, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->STIBP, Core->Bind);
	}
	if (Spec_Ctrl.SSBD) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBD, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBD, Core->Bind);
	}
    }
	if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.IBPB
	&& ((Mech_IBPB == COREFREQ_TOGGLE_OFF)
	 || (Mech_IBPB == COREFREQ_TOGGLE_ON)))
	{
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1))
	    {
		Pred_Cmd.IBPB = Mech_IBPB;
		WRMSR(Pred_Cmd, MSR_AMD_PRED_CMD);
	    }
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask, Core->Bind);
}

void Intel_VirtualMachine(CORE_RO *Core)
{
	if (PUBLIC(RO(Proc))->Features.Std.ECX.VMX) {
		VMX_BASIC VMX_Basic = {.value = 0};
		/*		Basic VMX Information.			*/
		RDMSR(VMX_Basic, MSR_IA32_VMX_BASIC);

		if (VMX_Basic.SMM_DualMon)
		{
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM, Core->Bind);
		}
	}
}

void Intel_Microcode(CORE_RO *Core)
{
	MICROCODE_ID Microcode = {.value = 0};

	RDMSR(Microcode, MSR_IA32_UCODE_REV);
	Core->Query.Microcode = Microcode.Signature;
}

void AMD_Microcode(CORE_RO *Core)
{
	unsigned long long value = 0;
	RDMSR64(value, MSR_AMD64_PATCH_LEVEL);
	Core->Query.Microcode = (unsigned int) value;
}

void PerCore_Reset(CORE_RO *Core)
{
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->PowerMgmt_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->SpeedStep_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->HWP_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->PC6_Mask	, Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->ODCM	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PowerMgmt , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SpeedStep , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->HWP	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C3A	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1A	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C3U	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1U	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6	, Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->STIBP 	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSBD	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RDCL_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS_ALL	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RSBA	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1DFL_VMENTRY_NO,Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSB_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->MDS_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSCHANGE_MC_NO, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TAA_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA	, Core->Bind);
}

static void PerCore_VirtualMachine(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
	{
		PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MIN)] = 10;

		Intel_VirtualMachine(Core);

		Intel_Microcode(Core);

		SpeedStep_Technology(Core);
	}
	else if ( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		||(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) )
	{
		PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MIN)] = 8;

		if (PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily >= 1) {
			AMD_Microcode(Core);
		}

		BITSET_CC(LOCKLESS,PUBLIC(RO(Proc))->SpeedStep_Mask,Core->Bind);
		BITSET_CC(LOCKLESS,PUBLIC(RO(Proc))->ODCM_Mask, Core->Bind);
		BITSET_CC(LOCKLESS,PUBLIC(RO(Proc))->PowerMgmt_Mask,Core->Bind);
	}

	SystemRegisters(Core);

	Dump_CPUID(Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);
}

static void PerCore_Intel_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Core_Platform_Info(Core->Bind);

	SystemRegisters(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_AuthenticAMD_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AuthenticAMD_Boost(Core->Bind);

	SystemRegisters(Core);

	if (PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily >= 1) {
		AMD_Microcode(Core);
	}
	Dump_CPUID(Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);
}

static void PerCore_Core2_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Core_Platform_Info(Core->Bind);

	SystemRegisters(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	DynamicAcceleration(Core);				/* Unique */
	Compute_Intel_Core_Burst(Core->Bind);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);

	PowerThermal(Core);				/* Shared | Unique */

	ThermalMonitor_Set(Core);
}

void Compute_Intel_Silvermont_Burst(CORE_RO *Core)
{
	enum RATIO_BOOST boost;
	unsigned int burstRatio;
	PLATFORM_ID PfID = {.value = 0};
	PERF_STATUS PerfStatus = {.value = 0};
	bool initialize = false;

	burstRatio = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MAX)];
    if (Intel_MaxBusRatio(&PfID) == 0) {
	burstRatio = KMAX(burstRatio, PfID.MaxBusRatio);
    }
	PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(ACT)] = burstRatio;

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	burstRatio = KMAX(burstRatio, PerfStatus.CORE.MaxBusRatio);

    if (PRIVATE(OF(Specific)) != NULL)
    {
	unsigned int XtraBoost	= PRIVATE(OF(Specific))->Boost[0]
				+ PRIVATE(OF(Specific))->Boost[1];
      if (XtraBoost > 0) {
	burstRatio = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MAX)];
	burstRatio = burstRatio + XtraBoost;
      }
    }
    if (PUBLIC(RO(Proc))->Features.Turbo_Unlock == 1)
    {	/*	Read the Turbo Boost register from any programmed ratio */
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	boost = BOOST(SIZE) - PUBLIC(RO(Proc))->Features.SpecTurboRatio;
      do
      {
	if (PUBLIC(RO(Core, AT(Core->Bind)))->Boost[boost] == 0)
	{
		PUBLIC(RO(Core, AT(Core->Bind)))->Boost[boost] = burstRatio;

		initialize = true;
	}
      } while ( ++boost < BOOST(SIZE) );

      if (initialize == true)
      { /*	Re-program the register if at least one value was zero	*/
	TURBO_RATIO_CONFIG0 Cfg0 = {
	.MaxRatio_1C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(1C)],
	.MaxRatio_2C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(2C)],
	.MaxRatio_3C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(3C)],
	.MaxRatio_4C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(4C)],
	.MaxRatio_5C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(5C)],
	.MaxRatio_6C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(6C)],
	.MaxRatio_7C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(7C)],
	.MaxRatio_8C = PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(8C)]
	};
	WRMSR(Cfg0, MSR_TURBO_RATIO_LIMIT);
      }
    }
}

static void PerCore_Silvermont_Query(void *arg)
{	/*	Query the Min and Max frequency ratios per CPU		*/
	CORE_RO *Core = (CORE_RO *) arg;

	PLATFORM_INFO PfInfo = {.value = 0};
	RDMSR(PfInfo, MSR_PLATFORM_INFO);

	PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MIN)]=PfInfo.MinimumRatio;
    PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(MAX)]=PfInfo.MaxNonTurboRatio;

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	DynamicAcceleration(Core);				/* Unique */
	SoC_Turbo_Override(Core);
	Compute_Intel_Silvermont_Burst(Core);

	Query_Intel_C1E(Core);
	/*TODO(Needs a per Module topology)*/
	Intel_CStatesConfiguration(CSTATES_SOC_SLM, Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);

	PowerThermal(Core);				/* Shared | Unique */

	ThermalMonitor_Set(Core);
}

static void PerCore_Nehalem_Same_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_Nehalem_Target,
				Get_Nehalem_Target,
				Cmp_Nehalem_Target,
				1 + Core->Boost[BOOST(MAX)],
				1 + Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_NHM, Core);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Nehalem_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);
	PerCore_Nehalem_Same_Query(Core);
}

static void PerCore_Nehalem_EX_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Core_Platform_Info(Core->Bind);
	PerCore_Nehalem_Same_Query(Core);
}

static void PerCore_SandyBridge_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_SandyBridge_Target,
				Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SNB, Core);
	}

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_SandyBridge_EP_Query(void *arg)
{
	PerCore_SandyBridge_Query(arg);
}

static void PerCore_IvyBridge_Query(void *arg)
{
	PerCore_SandyBridge_Query(arg);
	Intel_Turbo_TDP_Config( (CORE_RO*) arg );
}

static void PerCore_IvyBridge_EP_Query(void *arg)
{
	Intel_Turbo_Config(	(CORE_RO*) arg,
				Intel_Turbo_Cfg15C_PerCore,
				Assign_15C_Boost );

	PerCore_SandyBridge_EP_Query(arg);
}

static void PerCore_Haswell_Query(void *arg)
{
	PerCore_SandyBridge_Query(arg);
	Intel_Turbo_TDP_Config( (CORE_RO*) arg );
}

static void PerCore_Haswell_EP_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);

	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	Intel_Turbo_Config(	(CORE_RO*) arg,
				Intel_Turbo_Cfg16C_PerCore,
				Assign_16C_Boost );

	Intel_Turbo_Config(	(CORE_RO*) arg,
				Intel_Turbo_Cfg18C_PerCore,
				Assign_18C_Boost );

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	Intel_Microcode(Core);
    }
	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_SandyBridge_Target,
				Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SNB, Core);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);

	Intel_Turbo_TDP_Config(Core);
}

static void PerCore_Haswell_ULT_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_SandyBridge_Target,
				Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_ULT, Core);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Goldmont_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_SandyBridge_Target,
				Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SOC_GDM, Core);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Haswell_ULX(void *arg)
{
	PerCore_IvyBridge_Query(arg);
}

static void PerCore_Broadwell_Query(void *arg)
{
	PerCore_SandyBridge_Query(arg);
	Intel_Turbo_TDP_Config( (CORE_RO*) arg );
}

static void PerCore_Skylake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_SandyBridge_Target,
				Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SKL, Core);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Skylake_X_Query(void *arg)
{
	Intel_Turbo_Config(	(CORE_RO*) arg,
				Intel_Turbo_Cfg_SKL_X_PerCore,
				Assign_SKL_X_Boost );

	PerCore_Skylake_Query(arg);
	Intel_Turbo_TDP_Config( (CORE_RO*) arg );
}

static void PerCore_AMD_Family_0Fh_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_AMD_Family_0Fh_PStates(Core);		/* Alter P-States */
	Compute_AMD_Family_0Fh_Boost(Core->Bind);	/* Query P-States */

	SystemRegisters(Core);

	Dump_CPUID(Core);

	Query_AMD_Family_0Fh_C1E(Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask , Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);
}

static void PerCore_AMD_Family_Same_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	SystemRegisters(Core);

	AMD_Microcode(Core);

	Dump_CPUID(Core);

    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	Query_AMD_Family_0Fh_C1E(Core);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask	, Core->Bind);
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask , Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);
}

static void PerCore_AMD_Family_10h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_10h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
}

static void PerCore_AMD_Family_11h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_11h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
}

static void PerCore_AMD_Family_12h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_12h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
}

static void PerCore_AMD_Family_14h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_14h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
}

static void PerCore_AMD_Family_15h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_15h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
}

static void PerCore_AMD_Family_17h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;
	PM16 PM = {.value = 0};
	int ToggleFeature = 0;
	bool CPB_State;

	/*	Query the Min, Max, Target & Turbo P-States		*/
	PerCore_Query_AMD_Zen_Features(Core);
	CPB_State = Compute_AMD_Zen_Boost(Core->Bind);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		if (CPB_State == true)
		{	/*	Count CPB and XFR ratios		*/
			PUBLIC(RO(Proc))->Features.TDP_Levels = 2;
		}
		else {
			PUBLIC(RO(Proc))->Features.TDP_Levels = 0;
		}
	}
	SystemRegisters(Core);

	AMD_Microcode(Core);

	Dump_CPUID(Core);

	AMD_Mitigation_Mechanisms(Core);

	/*	Query the FCH for various registers			*/
	AMD_FCH_PM_Read16(AMD_FCH_PM_CSTATE_EN, PM);
	switch (C3U_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PM.CStateEn.C1eToC3En = C3U_Enable;
			ToggleFeature = 1;
		break;
	}
	switch (C1U_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PM.CStateEn.C1eToC2En = C1U_Enable;
			ToggleFeature = 1;
		break;
	}
	if (ToggleFeature == 1) {
		AMD_FCH_PM_Write16(AMD_FCH_PM_CSTATE_EN, PM);
		AMD_FCH_PM_Read16(AMD_FCH_PM_CSTATE_EN, PM);
	}
	if (PM.CStateEn.C1eToC2En)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1U, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1U, Core->Bind);
	}
	if (PM.CStateEn.C1eToC3En)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C3U, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C3U, Core->Bind);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask , Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SpeedStep_Mask, Core->Bind);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1E_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->C1U_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask , Core->Bind);

	Core->PowerThermal.Param = PUBLIC(RO(Proc))->PowerThermal.Param;
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
		SysGate->taskList[cnt].state    = (short int) thread->state;
#if defined(CONFIG_SCHED_BMQ)
		SysGate->taskList[cnt].wake_cpu = (short int) thread->cpu;
#else
		SysGate->taskList[cnt].wake_cpu = (short int) thread->wake_cpu;
#endif /* CONFIG_SCHED_BMQ	*/
		memcpy(SysGate->taskList[cnt].comm, thread->comm,TASK_COMM_LEN);

		if (cnt < TASK_LIMIT) {
			cnt++;
		}
	}
	rcu_read_unlock();
	SysGate->taskCount = cnt;
}
#endif /* KERNEL_VERSION(3, 10, 56) */

void Sys_MemInfo(SYSGATE_RO *SysGate)
{	/* Source: /include/uapi/linux/sysinfo.h */
	struct sysinfo info;
	si_meminfo(&info);

	SysGate->memInfo.totalram  = info.totalram  << (PAGE_SHIFT - 10);
	SysGate->memInfo.sharedram = info.sharedram << (PAGE_SHIFT - 10);
	SysGate->memInfo.freeram   = info.freeram   << (PAGE_SHIFT - 10);
	SysGate->memInfo.bufferram = info.bufferram << (PAGE_SHIFT - 10);
	SysGate->memInfo.totalhigh = info.totalhigh << (PAGE_SHIFT - 10);
	SysGate->memInfo.freehigh  = info.freehigh  << (PAGE_SHIFT - 10);
}

#define Sys_Tick(Pkg)						\
({								\
	if (PUBLIC(OF(Gate)) != NULL)				\
	{							\
		Pkg->tickStep--;				\
		if (!Pkg->tickStep) {				\
			Pkg->tickStep = Pkg->tickReset ;	\
			Sys_DumpTask( PUBLIC(OF(Gate)) );	\
			Sys_MemInfo( PUBLIC(OF(Gate)) );	\
		}						\
	}							\
})

static void InitTimer(void *Cycle_Function)
{
	unsigned int cpu = smp_processor_id();

	if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, CREATED) == 0)
	{
		hrtimer_init(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
				CLOCK_MONOTONIC,
				HRTIMER_MODE_REL_PINNED);

		PRIVATE(OF(Join, AT(cpu)))->Timer.function = Cycle_Function;
		BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, CREATED);
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

		    if (PUBLIC(RO(Proc))->Features.Std.ECX.Hyperv)
		    {
			PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = ratio;
		    }
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
	if ((BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, CREATED) == 1)
	 && (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED) == 0))
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
	    if ((BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, CREATED) == 1)
	     && (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED) == 1))
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
		BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, CREATED);
	}
}

void Intel_Core_Counters_Set(CORE_RO *Core)
{
    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 2) {
	CORE_GLOBAL_PERF_CONTROL	Core_GlobalPerfControl = {.value = 0};
	CORE_FIXED_PERF_CONTROL 	Core_FixedPerfControl = {.value = 0};
	CORE_GLOBAL_PERF_STATUS 	Core_PerfOverflow = {.value = 0};
	CORE_GLOBAL_PERF_OVF_CTRL	Core_PerfOvfControl = {.value = 0};

	RDMSR(Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);
	Core->SaveArea.Core_GlobalPerfControl = Core_GlobalPerfControl;
	Core_GlobalPerfControl.EN_FIXED_CTR0  = 1;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
	Core_GlobalPerfControl.EN_FIXED_CTR1  = 1;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
	Core_GlobalPerfControl.EN_FIXED_CTR2  = 1;
#endif
	WRMSR(Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);

	RDMSR(Core_FixedPerfControl, MSR_CORE_PERF_FIXED_CTR_CTRL);
	Core->SaveArea.Core_FixedPerfControl = Core_FixedPerfControl;
	Core_FixedPerfControl.EN0_OS = 1;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
	Core_FixedPerfControl.EN1_OS = 1;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
	Core_FixedPerfControl.EN2_OS = 1;
#endif
	Core_FixedPerfControl.EN0_Usr = 1;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
	Core_FixedPerfControl.EN1_Usr = 1;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
	Core_FixedPerfControl.EN2_Usr = 1;
#endif

	if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 3) {
		if (!PUBLIC(RO(Proc))->Features.HTT_Enable) {
			Core_FixedPerfControl.AnyThread_EN0 = 1;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
			Core_FixedPerfControl.AnyThread_EN1 = 1;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
			Core_FixedPerfControl.AnyThread_EN2 = 1;
#endif
		} else {
			/* Per Thread */
			Core_FixedPerfControl.AnyThread_EN0 = 0;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
			Core_FixedPerfControl.AnyThread_EN1 = 0;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
			Core_FixedPerfControl.AnyThread_EN2 = 0;
#endif
		}
	}
	WRMSR(Core_FixedPerfControl, MSR_CORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Core_PerfOverflow, MSR_CORE_PERF_GLOBAL_STATUS);
	RDMSR(Core_PerfOvfControl, MSR_CORE_PERF_GLOBAL_OVF_CTRL);
	if (Core_PerfOverflow.Overflow_CTR0)
		Core_PerfOvfControl.Clear_Ovf_CTR0 = 1;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
	if (Core_PerfOverflow.Overflow_CTR1)
		Core_PerfOvfControl.Clear_Ovf_CTR1 = 1;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
	if (Core_PerfOverflow.Overflow_CTR2)
		Core_PerfOvfControl.Clear_Ovf_CTR2 = 1;
#endif
	if (Core_PerfOverflow.Overflow_CTR0
	  | Core_PerfOverflow.Overflow_CTR1
	  | Core_PerfOverflow.Overflow_CTR2)
		WRMSR(Core_PerfOvfControl, MSR_CORE_PERF_GLOBAL_OVF_CTRL);
    }
}

#define AMD_Core_Counters_Set(Core, PMU)				\
({									\
    if (PUBLIC(RO(Proc))->Features.PerfMon.EBX.InstrRetired == 0)	\
    {									\
	HWCR HwCfgRegister = {.value = 0};				\
									\
	RDMSR(HwCfgRegister, MSR_K7_HWCR);				\
	Core->SaveArea.Core_HardwareConfiguration = HwCfgRegister;	\
	HwCfgRegister.PMU.IRPerfEn = 1;					\
	WRMSR(HwCfgRegister, MSR_K7_HWCR);				\
    }									\
})

#define Uncore_Counters_Set(PMU)					\
({									\
  if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 3)		\
  {									\
	UNCORE_GLOBAL_PERF_CONTROL  Uncore_GlobalPerfControl;		\
	UNCORE_FIXED_PERF_CONTROL   Uncore_FixedPerfControl;		\
	UNCORE_GLOBAL_PERF_STATUS   Uncore_PerfOverflow = {.value = 0}; \
	UNCORE_GLOBAL_PERF_OVF_CTRL Uncore_PerfOvfControl = {.value = 0};\
									\
	RDMSR(	Uncore_GlobalPerfControl,				\
		MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL );			\
									\
	PUBLIC(RO(Proc))->SaveArea.Uncore_GlobalPerfControl =		\
						Uncore_GlobalPerfControl;\
									\
	Uncore_GlobalPerfControl.PMU.EN_FIXED_CTR0  = 1;		\
	WRMSR(	Uncore_GlobalPerfControl,				\
		MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL );			\
									\
	RDMSR(	Uncore_FixedPerfControl,				\
		MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL );		\
									\
	PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl =		\
						Uncore_FixedPerfControl;\
									\
	Uncore_FixedPerfControl.PMU.EN_CTR0 = 1;			\
	WRMSR(	Uncore_FixedPerfControl,				\
		MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL );		\
									\
	RDMSR(Uncore_PerfOverflow, MSR_##PMU##_UNCORE_PERF_GLOBAL_STATUS);\
									\
    if (Uncore_PerfOverflow.PMU.Overflow_CTR0) {			\
	RDMSR(Uncore_PerfOvfControl, MSR_UNCORE_PERF_GLOBAL_OVF_CTRL);	\
	Uncore_PerfOvfControl.Clear_Ovf_CTR0 = 1;			\
	WRMSR(Uncore_PerfOvfControl, MSR_UNCORE_PERF_GLOBAL_OVF_CTRL);	\
    }									\
  }									\
})

void Intel_Core_Counters_Clear(CORE_RO *Core)
{
    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 2)
    {
	WRMSR(Core->SaveArea.Core_FixedPerfControl,
					MSR_CORE_PERF_FIXED_CTR_CTRL);

	WRMSR(Core->SaveArea.Core_GlobalPerfControl,
					MSR_CORE_PERF_GLOBAL_CTRL);
    }
}

void AMD_Core_Counters_Clear(CORE_RO *Core)
{
	if (PUBLIC(RO(Proc))->Features.PerfMon.EBX.InstrRetired == 0)
	{
		WRMSR(Core->SaveArea.Core_HardwareConfiguration, MSR_K7_HWCR);
	}
}

#define Uncore_Counters_Clear(PMU)					\
({									\
    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 3)		\
    {									\
	WRMSR(	PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl,	\
		MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL );		\
									\
	WRMSR(	PUBLIC(RO(Proc))->SaveArea.Uncore_GlobalPerfControl,	\
		MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL );			\
    }									\
})

#define Counters_VirtualMachine(Core, T)				\
({									\
	if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC) { 	\
		RDTSC64(Core->Counter[T].TSC);				\
	} else {							\
		RDTSCP64(Core->Counter[T].TSC);				\
	}								\
	/* HV_X64_MSR_VP_RUNTIME: vcpu runtime in 100ns units	*/	\
	RDCOUNTER(Core->Counter[T].C0.URC, 0x40000010);			\
									\
	Core->Counter[T].C0.URC = Core->Counter[T].C0.URC		\
	        * PUBLIC(RO(Proc))->Features.Factory.Ratio * 10;	\
									\
	Core->Counter[T].C0.UCC = Core->Counter[T].C0.URC;		\
	/* Derive C1: */						\
	Core->Counter[T].C1 =						\
	    (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
		Core->Counter[T].TSC - Core->Counter[T].C0.URC : 0;	\
})

#define Counters_Generic(Core, T)					\
({									\
	RDTSC_COUNTERx2(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC);	\
	/* Derive C1: */						\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_Core2(Core, T)						\
({									\
    if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC)		\
    {									\
	RDTSC_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
    } else {								\
	RDTSCP_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
    }									\
	/* Derive C1: */						\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_SLM(Core, T)						\
({									\
	RDTSC_COUNTERx6(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST, \
			MSR_CORE_C1_RESIDENCY, Core->Counter[T].C1,	\
			MSR_CORE_C3_RESIDENCY, Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY, Core->Counter[T].C6);	\
})

#define SMT_Counters_Nehalem(Core, T)					\
({									\
	register unsigned long long Cx;					\
									\
	RDTSCP_COUNTERx5(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1: */						\
	Cx =	Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1 =						\
		(Core->Counter[T].TSC > Cx) ?				\
			Core->Counter[T].TSC - Cx			\
			: 0;						\
})

#define SMT_Counters_SandyBridge(Core, T)				\
({									\
	register unsigned long long Cx;					\
									\
	RDTSCP_COUNTERx6(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_C7_RESIDENCY,Core->Counter[T].C7,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1: */						\
	Cx =	Core->Counter[T].C7					\
		+ Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1 =						\
		(Core->Counter[T].TSC > Cx) ?				\
			Core->Counter[T].TSC - Cx			\
			: 0;						\
})

#define SMT_Counters_AMD_Family_17h(Core, T)				\
({									\
	register unsigned long long Cx;					\
									\
	RDTSCP_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_AMD_F17H_IRPERF, Core->Counter[T].INST);	\
	/* Read Virtual PMC and cumulative store: */			\
	Atomic_Read_VPMC(LOCKLESS, Core->Counter[T].C1, Core->VPMC.C1); \
	Atomic_Read_VPMC(LOCKLESS, Core->Counter[T].C3, Core->VPMC.C2); \
	Atomic_Add_VPMC (LOCKLESS, Core->Counter[T].C3, Core->VPMC.C3); \
	Atomic_Read_VPMC(LOCKLESS, Core->Counter[T].C6, Core->VPMC.C4); \
	Atomic_Add_VPMC (LOCKLESS, Core->Counter[T].C6, Core->VPMC.C5); \
	Atomic_Add_VPMC (LOCKLESS, Core->Counter[T].C6, Core->VPMC.C6); \
									\
	Cx =	Core->Counter[T].C6					\
		+ Core->Counter[T].C3					\
		+ Core->Counter[T].C0.URC;				\
									\
	Core->Counter[T].C1 =						\
		(Core->Counter[T].TSC > Cx) ?				\
			Core->Counter[T].TSC - Cx			\
			: 0;						\
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
	Core->Delta.C0.UCC =						\
		(Core->Counter[0].C0.UCC >				\
		Core->Counter[1].C0.UCC) ?				\
			Core->Counter[0].C0.UCC				\
			- Core->Counter[1].C0.UCC			\
			: Core->Counter[1].C0.UCC			\
			- Core->Counter[0].C0.UCC;			\
									\
	Core->Delta.C0.URC = Core->Counter[1].C0.URC			\
			   - Core->Counter[0].C0.URC;			\
})

#define Delta_C1(Core)							\
({									\
	Core->Delta.C1 =						\
		(Core->Counter[0].C1 >					\
		 Core->Counter[1].C1) ?					\
			Core->Counter[0].C1				\
			- Core->Counter[1].C1				\
			: Core->Counter[1].C1				\
			- Core->Counter[0].C1;				\
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
	Core->Delta.INST = Core->Counter[1].INST			\
			 - Core->Counter[0].INST;			\
})

#define PKG_Counters_VirtualMachine(Core, T)				\
({									\
	PUBLIC(RO(Proc))->Counter[T].PTSC = Core->Counter[T].TSC;	\
})

#define PKG_Counters_Generic(Core, T)					\
({									\
	PUBLIC(RO(Proc))->Counter[T].PTSC = Core->Counter[T].TSC;	\
})

#define PKG_Counters_SLM(Core, T)					\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PTSC ,		\
	MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,	\
	MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,	\
	MSR_ATOM_PKG_C4_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC04,	\
	MSR_ATOM_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,	\
	MSR_ATOM_MC6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].MC6);	\
})

#define PKG_Counters_Nehalem(Core, T)					\
({									\
    RDTSCP_COUNTERx4(PUBLIC(RO(Proc))->Counter[T].PTSC ,		\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
      MSR_NHM_UNCORE_PERF_FIXED_CTR0, PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_SandyBridge(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PTSC ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
      MSR_SNB_UNCORE_PERF_FIXED_CTR0, PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_SandyBridge_EP(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PTSC ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
    MSR_SNB_EP_UNCORE_PERF_FIXED_CTR0,PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_Haswell_EP(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PTSC ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
    MSR_HSW_EP_UNCORE_PERF_FIXED_CTR0,PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_Haswell_ULT(Core, T)				\
({									\
    RDTSCP_COUNTERx7(PUBLIC(RO(Proc))->Counter[T].PTSC,			\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_PKG_C8_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC08,\
		MSR_PKG_C9_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC09,\
		MSR_PKG_C10_RESIDENCY,PUBLIC(RO(Proc))->Counter[T].PC10);\
									\
	RDCOUNTER	(PUBLIC(RO(Proc))->Counter[T].Uncore.FC0,	\
			MSR_SNB_UNCORE_PERF_FIXED_CTR0 );		\
})

#define PKG_Counters_Skylake(Core, T)					\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PTSC,			\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
      MSR_SKL_UNCORE_PERF_FIXED_CTR0, PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_Skylake_X(Core, T) 				\
({									\
    RDTSCP_COUNTERx4(PUBLIC(RO(Proc))->Counter[T].PTSC,			\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07);\
})

#define Pkg_OVH(Pkg, Core)						\
({									\
	Pkg->Delta.PTSC -= (Pkg->Counter[1].PTSC - Core->Overhead.TSC); \
})

#define Delta_PTSC(Pkg) 						\
({									\
	Pkg->Delta.PTSC = Pkg->Counter[1].PTSC				\
			- Pkg->Counter[0].PTSC; 			\
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
			- Pkg->Counter[0].PC02;				\
})

#define Delta_PC03(Pkg) 						\
({									\
	Pkg->Delta.PC03 = Pkg->Counter[1].PC03				\
			- Pkg->Counter[0].PC03;				\
})

#define Delta_PC04(Pkg) 						\
({									\
	Pkg->Delta.PC04 = Pkg->Counter[1].PC04				\
			- Pkg->Counter[0].PC04;				\
})

#define Delta_PC06(Pkg) 						\
({									\
	Pkg->Delta.PC06 = Pkg->Counter[1].PC06				\
			- Pkg->Counter[0].PC06;				\
})

#define Delta_PC07(Pkg) 						\
({									\
	Pkg->Delta.PC07 = Pkg->Counter[1].PC07				\
			- Pkg->Counter[0].PC07;				\
})

#define Delta_PC08(Pkg) 						\
({									\
	Pkg->Delta.PC08 = Pkg->Counter[1].PC08				\
			- Pkg->Counter[0].PC08;				\
})

#define Delta_PC09(Pkg) 						\
({									\
	Pkg->Delta.PC09 = Pkg->Counter[1].PC09				\
			- Pkg->Counter[0].PC09;				\
})

#define Delta_PC10(Pkg) 						\
({									\
	Pkg->Delta.PC10 = Pkg->Counter[1].PC10				\
			- Pkg->Counter[0].PC10;				\
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
	Pkg->Counter[0].PTSC = Pkg->Counter[1].PTSC;			\
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

#define PWR_ACCU_SandyBridge(Pkg, T)					\
({									\
	RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(PKG)],		\
						MSR_PKG_ENERGY_STATUS); \
									\
	RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(CORES)],	\
						MSR_PP0_ENERGY_STATUS); \
									\
	RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(UNCORE)],	\
						MSR_PP1_ENERGY_STATUS); \
})

#define PWR_ACCU_SandyBridge_EP(Pkg, T) 				\
({									\
	RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(PKG)],		\
						MSR_PKG_ENERGY_STATUS); \
									\
	RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(CORES)],	\
						MSR_PP0_ENERGY_STATUS); \
									\
	RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(RAM)],		\
						MSR_DRAM_ENERGY_STATUS);\
})

#define PWR_ACCU_Skylake(Pkg, T)					\
({									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(PKG)],		\
						MSR_PKG_ENERGY_STATUS); \
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(CORES)],	\
						MSR_PP0_ENERGY_STATUS); \
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(UNCORE)],	\
						MSR_PP1_ENERGY_STATUS); \
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(RAM)],		\
						MSR_DRAM_ENERGY_STATUS);\
})

#define PWR_ACCU_SKL_PLATFORM(Pkg, T)					\
({									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(PKG)],		\
						MSR_PKG_ENERGY_STATUS); \
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(CORES)],	\
						MSR_PP0_ENERGY_STATUS); \
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(PLATFORM)],	\
					MSR_PLATFORM_ENERGY_STATUS);	\
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(RAM)],		\
						MSR_DRAM_ENERGY_STATUS);\
})

#define Delta_PWR_ACCU(Pkg, PwrDomain)					\
({									\
	PUBLIC(RW(Pkg))->Delta.Power.ACCU[PWR_DOMAIN(PwrDomain)] =	\
	(PUBLIC(RO(Pkg))->Counter[1].Power.ACCU[PWR_DOMAIN(PwrDomain)]	\
	- PUBLIC(RO(Pkg))->Counter[0].Power.ACCU[PWR_DOMAIN(PwrDomain)])\
	& 0x7fffffffU;							\
})

#define Save_PWR_ACCU(Pkg, PwrDomain)					\
({									\
	Pkg->Counter[0].Power.ACCU[PWR_DOMAIN(PwrDomain)] =		\
		Pkg->Counter[1].Power.ACCU[PWR_DOMAIN(PwrDomain)];	\
})

void Core_Intel_Temp(CORE_RO *Core)
{
	THERM_STATUS ThermStatus = {.value = 0};
	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);	/*All Intel families.*/

	Core->PowerThermal.Sensor = ThermStatus.DTS;
	Core->PowerThermal.Events = (	(ThermStatus.StatusBit
					|ThermStatus.StatusLog ) << 0)
				  | (ThermStatus.PROCHOTLog << 1)
				  | (ThermStatus.CriticalTempLog << 2)
				  | (	(ThermStatus.Threshold1Log
					|ThermStatus.Threshold2Log ) << 3)
				  | (ThermStatus.PwrLimitLog << 4)
				  | (ThermStatus.CurLimitLog << 5)
				  | (ThermStatus.XDomLimitLog << 6);
}

#define Pkg_Intel_Temp(Pkg)						\
({									\
    if (Pkg->Features.Power.EAX.PTM)					\
    {									\
	THERM_STATUS ThermStatus = {.value = 0};			\
	RDMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);		\
									\
	Pkg->PowerThermal.Sensor = ThermStatus.DTS;			\
	Pkg->PowerThermal.Events = (	(ThermStatus.StatusBit		\
					|ThermStatus.StatusLog ) << 0)	\
				 | (ThermStatus.PROCHOTLog << 1)	\
				 | (ThermStatus.CriticalTempLog << 2)	\
				 | (	(ThermStatus.Threshold1Log	\
					|ThermStatus.Threshold2Log )<<3)\
				 | (ThermStatus.PwrLimitLog << 4);	\
    }									\
})

void Core_AMD_Family_0Fh_Temp(CORE_RO *Core)
{
	if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		RDPCI(ThermTrip, PCI_AMD_THERMTRIP_STATUS);

		/* Select Core to read sensor from: */
		ThermTrip.SensorCoreSelect = Core->Bind;

		WRPCI(ThermTrip, PCI_AMD_THERMTRIP_STATUS);
		RDPCI(ThermTrip, PCI_AMD_THERMTRIP_STATUS);

		/* Formula is " CurTmp - (TjOffset * 2) - 49 " */
		Core->PowerThermal.Param.Target = ThermTrip.TjOffset;
		Core->PowerThermal.Sensor = ThermTrip.CurrentTemp;

		Core->PowerThermal.Events = ThermTrip.SensorTrip << 0;
	}
}

void Core_AMD_Family_15h_Temp(CORE_RO *Core)
{
	TCTL_REGISTER TctlSensor = {.value = 0};

	RDPCI(TctlSensor, PCI_AMD_TEMPERATURE_TCTL);
	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		RDPCI(ThermTrip, PCI_AMD_THERMTRIP_STATUS);

		Core->PowerThermal.Events = ThermTrip.SensorTrip << 0;
	}
}

/*TODO: Bulldozer/Excavator [need hardware to test with]
void Core_AMD_Family_15_60h_Temp(CORE_RO *Core)
{
    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	TCTL_REGISTER TctlSensor = {.value = 0};

	Core_AMD_SMN_Read(	TctlSensor,
				SMU_AMD_THM_TCTL_REGISTER_F15H,
				SMU_AMD_INDEX_REGISTER_F15H,
				SMU_AMD_DATA_REGISTER_F15H );

	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		WRPCI(	SMU_AMD_THM_TRIP_REGISTER_F15H,
			SMU_AMD_INDEX_REGISTER_F15H);
		RDPCI(ThermTrip, SMU_AMD_DATA_REGISTER_F15H);

		Core->PowerThermal.Events = ThermTrip.SensorTrip << 0;
	}
    }
}
*/

void CTL_AMD_Family_17h_Temp(CORE_RO *Core)
{
	TCTL_REGISTER TctlSensor = {.value = 0};

	Core_AMD_SMN_Read(	TctlSensor,
				SMU_AMD_THM_TCTL_REGISTER_F17H,
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (TctlSensor.CurTempRangeSel == 1)
	{
	/* Register: SMU::THM::THM_TCON_CUR_TMP - Bit 19: CUR_TEMP_RANGE_SEL
		0 = Report on 0C to 225C scale range.
		1 = Report on -49C to 206C scale range.
	*/
		Core->PowerThermal.Param.Offset[1] = 49;
	} else {
		Core->PowerThermal.Param.Offset[1] = 0;
	}
}

void CCD_AMD_Family_17h_Zen2_Temp(CORE_RO *Core)
{
	TCCD_REGISTER TccdSensor = {.value = 0};

	Core_AMD_SMN_Read(	TccdSensor,
				SMU_AMD_THM_TCTL_CCD_REGISTER_F17H
				+ (Core->T.Cluster.CCD << 2),
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	Core->PowerThermal.Sensor = TccdSensor.CurTmp;

	if (TccdSensor.CurTempRangeSel == 1)
	{
		Core->PowerThermal.Param.Offset[1] = 49;
	} else {
		Core->PowerThermal.Param.Offset[1] = 0;
	}
}

#define Pkg_AMD_Family_17h_Temp(Pkg, Core)				\
({									\
	Core_AMD_Family_17h_Temp(Core);					\
									\
	Pkg->PowerThermal.Sensor = Core->PowerThermal.Sensor;		\
})

static enum hrtimer_restart Cycle_VirtualMachine(struct hrtimer *pTimer)
{
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC) {
		RDTSC64(Core->Overhead.TSC);
	} else {
		RDTSCP64(Core->Overhead.TSC);
	}
	if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_VirtualMachine(Core, 1);

		if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
		{
			PKG_Counters_VirtualMachine(Core, 1);

			Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

			Save_PTSC(PUBLIC(RO(Proc)));

			Sys_Tick(PUBLIC(RO(Proc)));
		}

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		return (HRTIMER_RESTART);
	} else
		return (HRTIMER_NORESTART);
}

void InitTimer_VirtualMachine(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_VirtualMachine, 1);
}

static void Start_VirtualMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_VirtualMachine(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_VirtualMachine(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_VirtualMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static enum hrtimer_restart Cycle_GenuineIntel(struct hrtimer *pTimer)
{
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC) {
		RDTSC64(Core->Overhead.TSC);
	} else {
		RDTSCP64(Core->Overhead.TSC);
	}
	if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

			Pkg_Intel_Temp(PUBLIC(RO(Proc)));

		    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
		    {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

			Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

			Save_PTSC(PUBLIC(RO(Proc)));

			Sys_Tick(PUBLIC(RO(Proc)));
		}

		switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		return (HRTIMER_RESTART);
	} else
		return (HRTIMER_NORESTART);
}

void InitTimer_GenuineIntel(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_GenuineIntel, 1);
}

static void Start_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static enum hrtimer_restart Cycle_AuthenticAMD(struct hrtimer *pTimer)
{
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC) {
		RDTSC64(Core->Overhead.TSC);
	} else {
		RDTSCP64(Core->Overhead.TSC);
	}
	if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
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

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		return (HRTIMER_RESTART);
	} else
		return (HRTIMER_NORESTART);
}

void InitTimer_AuthenticAMD(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AuthenticAMD, 1);
}

static void Start_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Core2(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC) {
	RDTSC64(Core->Overhead.TSC);
    } else {
	RDTSCP64(Core->Overhead.TSC);
    }
    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	Counters_Core2(Core, 1);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.CORE.CurrFID;

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_CORE2_TARGET(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Generic(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

		switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
		{
		case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		}

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU=PerfStatus.CORE.CurrVID;

		switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
		{
		case FORMULA_SCOPE_PKG:
			Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
			break;
		}

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Save_PTSC(PUBLIC(RO(Proc)));

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
		break;
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

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Core2(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Core2, 1);
}

static void Start_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	Counters_Core2(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Silvermont(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

  if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
  {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	Counters_SLM(Core, 1);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.CORE.CurrFID;

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_CORE2_TARGET(Core);

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	PKG_Counters_SLM(Core, 1);

	Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	{
	case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	}

	PUBLIC(RO(Proc))->PowerThermal.VID.CPU=PerfStatus.CORE.CurrVID;

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	{
	case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
		break;
	}
	RDCOUNTER( PUBLIC(RO(Proc))->Counter[1].Power.ACCU[PWR_DOMAIN(PKG)],
			MSR_PKG_ENERGY_STATUS );

	RDCOUNTER( PUBLIC(RO(Proc))->Counter[1].Power.ACCU[PWR_DOMAIN(CORES)],
			MSR_PP0_ENERGY_STATUS );

	Delta_PC02(PUBLIC(RO(Proc)));

	Delta_PC03(PUBLIC(RO(Proc)));

	Delta_PC04(PUBLIC(RO(Proc)));

	Delta_PC06(PUBLIC(RO(Proc)));

	Delta_MC6(PUBLIC(RO(Proc)));

	Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

	Delta_PWR_ACCU(Proc, PKG);

	Delta_PWR_ACCU(Proc, CORES);

	Save_PC02(PUBLIC(RO(Proc)));

	Save_PC03(PUBLIC(RO(Proc)));

	Save_PC04(PUBLIC(RO(Proc)));

	Save_PC06(PUBLIC(RO(Proc)));

	Save_MC6(PUBLIC(RO(Proc)));

	Save_PTSC(PUBLIC(RO(Proc)));

	Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

	Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

	Sys_Tick(PUBLIC(RO(Proc)));
    } else {
	Core->PowerThermal.VID = 0;
    }

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C1(Core);

	Save_C3(Core);

	Save_C6(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
  } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Silvermont(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Silvermont, 1);
}

static void Start_Silvermont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	Counters_SLM(Core, 0);

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
	}
	PKG_Counters_SLM(Core, 0);

	RDCOUNTER( PUBLIC(RO(Proc))->Counter[0].Power.ACCU[PWR_DOMAIN(PKG)],
			MSR_PKG_ENERGY_STATUS );

	RDCOUNTER( PUBLIC(RO(Proc))->Counter[0].Power.ACCU[PWR_DOMAIN(CORES)],
			MSR_PP0_ENERGY_STATUS );
    }
	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Silvermont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Nehalem(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_Nehalem(Core, 1);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Nehalem(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    #if defined(HWM_CHIPSET) \
	    && ((HWM_CHIPSET == W83627) || (HWM_CHIPSET == IT8720))
		RDSIO(	PUBLIC(RO(Proc))->PowerThermal.VID.CPU,
			HWM_W83627_CPUVCORE,
			HWM_W83627_INDEX_PORT, HWM_W83627_DATA_PORT );
	    #endif

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
	    #if defined(HWM_CHIPSET) \
	    && ((HWM_CHIPSET == W83627) || (HWM_CHIPSET == IT8720))
		Core->PowerThermal.VID = PUBLIC(RO(Proc))->PowerThermal.VID.CPU;
	    #endif
		break;
	    }

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
	    #if defined(HWM_CHIPSET) \
	    && ((HWM_CHIPSET == W83627) || (HWM_CHIPSET == IT8720))
		Core->PowerThermal.VID = 0;
	    #endif
	}

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_NEHALEM_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.NHM.CurrentRatio;

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
	    #if defined(HWM_CHIPSET) \
	    && ((HWM_CHIPSET == W83627) || (HWM_CHIPSET == IT8720))
		RDSIO(	Core->PowerThermal.VID, HWM_W83627_CPUVCORE,
			HWM_W83627_INDEX_PORT, HWM_W83627_DATA_PORT );
	    #endif
	    }
		break;
	case FORMULA_SCOPE_SMT:
	    #if defined(HWM_CHIPSET) \
	    && ((HWM_CHIPSET == W83627) || (HWM_CHIPSET == IT8720))
		RDSIO(	Core->PowerThermal.VID, HWM_W83627_CPUVCORE,
			HWM_W83627_INDEX_PORT, HWM_W83627_DATA_PORT );
	    #endif
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Nehalem(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Nehalem, 1);
}

static void Start_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_Nehalem(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Nehalem(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_Nehalem(void *arg)
{
	Uncore_Counters_Set(NHM);
}

static void Stop_Uncore_Nehalem(void *arg)
{
	Uncore_Counters_Clear(NHM);
}


static enum hrtimer_restart Cycle_SandyBridge(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_SandyBridge(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_SandyBridge(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = PerfStatus.SNB.CurrVID;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	    }

		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, UNCORE);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), UNCORE);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_C7(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C7(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_SandyBridge(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_SandyBridge, 1);
}

static void Start_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_SandyBridge(Core, 0);
		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_SandyBridge(void *arg)
{
	Uncore_Counters_Set(SNB);
}

static void Stop_Uncore_SandyBridge(void *arg)
{
	Uncore_Counters_Clear(SNB);
}


static enum hrtimer_restart Cycle_SandyBridge_EP(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_SandyBridge(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_SandyBridge_EP(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = PerfStatus.SNB.CurrVID;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	    }

		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, RAM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), RAM);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_C7(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C7(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_SandyBridge_EP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_SandyBridge_EP, 1);
}

static void Start_SandyBridge_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_SandyBridge_EP(Core, 0);
		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_SandyBridge_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_SandyBridge_EP(void *arg)
{
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;

	RDMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl = \
						Uncore_FixedPerfControl;

	Uncore_FixedPerfControl.SNB.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_SNB_EP_PMON_GLOBAL_CTRL);

	PUBLIC(RO(Proc))->SaveArea.Uncore_PMonGlobalControl = \
						Uncore_PMonGlobalControl;

	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_SNB_EP_PMON_GLOBAL_CTRL);
}

static void Stop_Uncore_SandyBridge_EP(void *arg)
{	/* If fixed counter was disable at entry, force freezing	*/
    if (PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl.SNB.EN_CTR0 == 0)
    {
	PUBLIC(RO(Proc))->SaveArea.Uncore_PMonGlobalControl.Freeze_All = 1;
    }
	WRMSR(	PUBLIC(RO(Proc))->SaveArea.Uncore_PMonGlobalControl,
		MSR_SNB_EP_PMON_GLOBAL_CTRL);

	WRMSR(	PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl,
		MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);
}


static enum hrtimer_restart Cycle_Haswell_ULT(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_SandyBridge(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Haswell_ULT(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = PerfStatus.SNB.CurrVID;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	    }

		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PC08(PUBLIC(RO(Proc)));

		Delta_PC09(PUBLIC(RO(Proc)));

		Delta_PC10(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, UNCORE);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PC08(PUBLIC(RO(Proc)));

		Save_PC09(PUBLIC(RO(Proc)));

		Save_PC10(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), UNCORE);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_C7(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C7(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Haswell_ULT(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Haswell_ULT, 1);
}

static void Start_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Haswell_ULT(Core, 0);
		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_Haswell_ULT(void *arg)
{
    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	Uncore_Counters_Set(SNB);
    }
}

static void Stop_Uncore_Haswell_ULT(void *arg)
{
    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	Uncore_Counters_Clear(SNB);
    }
}

static void Start_Goldmont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Haswell_ULT(Core, 0);
		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}


static enum hrtimer_restart Cycle_Haswell_EP(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_SandyBridge(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Haswell_EP(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = PerfStatus.SNB.CurrVID;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	    }

		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, RAM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), RAM);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_C7(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C7(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Haswell_EP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Haswell_EP, 1);
}

static void Start_Haswell_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Haswell_EP(Core, 0);
		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Haswell_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_Haswell_EP(void *arg)
{
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;

	RDMSR(Uncore_FixedPerfControl, MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl = \
						Uncore_FixedPerfControl;

	Uncore_FixedPerfControl.HSW_EP.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_HSW_EP_PMON_GLOBAL_CTRL);

	PUBLIC(RO(Proc))->SaveArea.Uncore_PMonGlobalControl = \
						Uncore_PMonGlobalControl;

	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_HSW_EP_PMON_GLOBAL_CTRL);
}

static void Stop_Uncore_Haswell_EP(void *arg)
{
    if (PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl.HSW_EP.EN_CTR0 == 0)
    {
	PUBLIC(RO(Proc))->SaveArea.Uncore_PMonGlobalControl.Freeze_All = 1;
    }
	WRMSR(	PUBLIC(RO(Proc))->SaveArea.Uncore_PMonGlobalControl,
		MSR_HSW_EP_PMON_GLOBAL_CTRL );

	WRMSR(	PUBLIC(RO(Proc))->SaveArea.Uncore_FixedPerfControl,
		MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL );
}


static enum hrtimer_restart Cycle_Skylake(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_SandyBridge(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Skylake(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = PerfStatus.SNB.CurrVID;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	    }

	    switch (PUBLIC(RO(Proc))->ArchID) {
	    case Kabylake_UY:
	    case Kabylake:
	    case Cannonlake:
	    case Icelake_UY:
	    case Icelake:
	    case Cometlake_UY:
	    case Cometlake:
		PWR_ACCU_SKL_PLATFORM(PUBLIC(RO(Proc)), 1);
		break;
	    case Skylake_UY:
	    case Skylake_S:
	    case Skylake_X:
	    case Icelake_X:
	    case Icelake_D:
	    case Sunny_Cove:
	    case Tigerlake_U:
	    case Tigerlake:
	    case Atom_C3000:
	    case Tremont_Jacobsville:
	    case Tremont_Lakefield:
	    case Tremont_Elkhartlake:
	    case Tremont_Jasperlake:
	    default:
		PWR_ACCU_Skylake(PUBLIC(RO(Proc)), 1);
		break;
	    }

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, UNCORE);

		Delta_PWR_ACCU(Proc, RAM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), UNCORE);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), RAM);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_C7(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C7(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Skylake(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Skylake, 1);
}

static void Start_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Skylake(Core, 0);

	    switch (PUBLIC(RO(Proc))->ArchID) {
	    case Kabylake_UY:
	    case Kabylake:
	    case Cannonlake:
	    case Icelake_UY:
	    case Icelake:
	    case Cometlake_UY:
	    case Cometlake:
		PWR_ACCU_SKL_PLATFORM(PUBLIC(RO(Proc)), 0);
		break;
	    case Skylake_UY:
	    case Skylake_S:
	    case Skylake_X:
	    case Icelake_X:
	    case Icelake_D:
	    case Sunny_Cove:
	    case Tigerlake_U:
	    case Tigerlake:
	    case Atom_C3000:
	    case Tremont_Jacobsville:
	    case Tremont_Lakefield:
	    case Tremont_Elkhartlake:
	    case Tremont_Jasperlake:
	    default:
		PWR_ACCU_Skylake(PUBLIC(RO(Proc)), 0);
		break;
	    }
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_Skylake(void *arg)
{
	Uncore_Counters_Set(SKL);
}

static void Stop_Uncore_Skylake(void *arg)
{
	Uncore_Counters_Clear(SKL);
}

static enum hrtimer_restart Cycle_Skylake_X(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_SandyBridge(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Skylake_X(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_Intel_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = PerfStatus.SNB.CurrVID;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	    }

		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, RAM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), RAM);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_Intel_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_Intel_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
		break;
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_C7(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C7(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_Skylake_X(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Skylake_X, 1);
}

static void Start_Skylake_X(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Skylake_X(Core, 0);
		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_Skylake_X(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_Uncore_Skylake_X(void *arg)
{
/*TODO:	Uncore_Counters_Set(SKL_X);*/
}

static void Stop_Uncore_Skylake_X(void *arg)
{
/*TODO:	Uncore_Counters_Clear(SKL_X); */
}

static enum hrtimer_restart Cycle_AMD_Family_0Fh(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	FIDVID_CONTROL FidVidControl = {.value = 0};
	FIDVID_STATUS FidVidStatus = {.value = 0};

	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	RDMSR(FidVidControl, MSR_K7_FID_VID_CTL);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TGT)] = FidVidControl.NewFID;

	RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

	Core->PowerThermal.VID	= FidVidStatus.CurrVID;
	Core->Ratio.Perf	= 8 + FidVidStatus.CurrFID;

	/* P-States */
	Core->Counter[1].C0.UCC = Core->Counter[0].C0.UCC
				+ Core->Ratio.Perf
				* Core->Clock.Hz;

	Core->Counter[1].C0.URC = Core->Counter[1].C0.UCC;

	Core->Counter[1].TSC	= Core->Counter[0].TSC
				+ (Core->Boost[BOOST(MAX)]
					* Core->Clock.Hz);

	/* Derive C1 */
	Core->Counter[1].C1 =
	  (Core->Counter[1].TSC > Core->Counter[1].C0.URC) ?
	    Core->Counter[1].TSC - Core->Counter[1].C0.URC
	    : 0;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Generic(Core, 1);

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_AMD_Family_0Fh_Temp(Core);
		break;
	    }

		PUBLIC(RO(Proc))->PowerThermal.VID.CPU = FidVidStatus.CurrVID;

		Delta_PTSC(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Sys_Tick(PUBLIC(RO(Proc)));
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core_AMD_Family_0Fh_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_AMD_Family_0Fh_Temp(Core);
		break;
	}

	Delta_C0(Core);

	Delta_TSC(Core);

	Delta_C1(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C1(Core);

	if (AutoClock & 0b10)
	{
		REL_BCLK(Core->Clock,
			Core->Boost[BOOST(MAX)],
			Core->Delta.TSC,
			PUBLIC(RO(Proc))->SleepInterval);
	}
	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_AMD_Family_0Fh(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_0Fh, 1);
}

static void Start_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_AMD_Family_11h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_AMD_Family_12h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Start_AMD_Family_14h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static enum hrtimer_restart Cycle_AMD_Family_15h(struct hrtimer *pTimer)
{
	PSTATECTRL PstateCtrl = {.value = 0};
	PSTATESTAT PstateStat = {.value = 0};
	PSTATEDEF PstateDef = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	Counters_Generic(Core, 1);

	if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.HwPstate)
	{
		unsigned int pstate, COF;
		/*	Read the Target & Status P-State.		*/
		RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
		RDMSR(PstateDef,MSR_AMD_PSTATE_DEF_BASE + PstateCtrl.PstateCmd);

		COF = AMD_F15h_CoreCOF( PstateDef.Family_15h.CpuFid,
					PstateDef.Family_15h.CpuDid );

		PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TGT)] = COF;

		/*	Read the current P-State number.		*/
		RDMSR(PstateStat, MSR_AMD_PERF_STATUS);
		/*	Offset the P-State base register.		*/
		pstate = MSR_AMD_PSTATE_DEF_BASE + PstateStat.Current;
		/*	Read the voltage ID at the offset		*/
		RDMSR(PstateDef, pstate);

		COF = AMD_F15h_CoreCOF( PstateDef.Family_15h.CpuFid,
					PstateDef.Family_15h.CpuDid );
		Core->Ratio.Perf = COF;
	}
	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Generic(Core, 1);

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core_AMD_Family_15h_Temp(Core);
		break;
	    }

	    PUBLIC(RO(Proc))->PowerThermal.VID.CPU=PstateDef.Family_15h.CpuVid;

	    switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula))
	    {
	    case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PstateDef.Family_15h.CpuVid;
		break;
	    }

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Save_PTSC(PUBLIC(RO(Proc)));

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}
	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if (Core->T.CoreID == 0) {
		Core_AMD_Family_15h_Temp(Core);
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core_AMD_Family_15h_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		Core->PowerThermal.VID = PstateDef.Family_15h.CpuVid;
	    }
		break;
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PstateDef.Family_15h.CpuVid;
		break;
	}

	Delta_C0(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

	return (HRTIMER_RESTART);
    } else
	return (HRTIMER_NORESTART);
}

void InitTimer_AMD_Family_15h(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_15h, 1);
}

static void Start_AMD_Family_15h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Counters_Generic(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

void Cycle_AMD_Family_17h(CORE_RO *Core,
			void (*Call_SMU)(const unsigned int, const unsigned int,
					const unsigned long long),
			const unsigned int plane0, const unsigned int plane1,
			const unsigned long long factor)
{
	PSTATEDEF PstateDef;
	PSTATECTRL PstateCtrl;
	unsigned int COF;

	SMT_Counters_AMD_Family_17h(Core, 1);

	/*		Read the Target P-State.			*/
	RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
	RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + PstateCtrl.PstateCmd);

	COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
				PstateDef.Family_17h.CpuDfsId );

	PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(TGT)] = COF;

	/*	Read the Boosted Frequency and voltage VID.		*/
	RDMSR(PstateDef, MSR_AMD_PSTATE_F17H_BOOST);
	COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
				PstateDef.Family_17h.CpuDfsId );
	Core->Ratio.Perf = COF;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Generic(Core, 1);

		Pkg_AMD_Family_17h_Temp(PUBLIC(RO(Proc)), Core);

	  switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	  case FORMULA_SCOPE_PKG:
		Core_AMD_Family_17h_Temp(Core);
		break;
	  }

	  switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	  case FORMULA_SCOPE_PKG:
		Core->PowerThermal.VID = PstateDef.Family_17h.CpuVid;
		break;
	  }

		Call_SMU(plane0, plane1, factor);

	    RDCOUNTER(PUBLIC(RO(Proc))->Counter[1].Power.ACCU[PWR_DOMAIN(PKG)],
			MSR_AMD_PKG_ENERGY_STATUS);

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_PWR_ACCU(Proc, PKG);

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Sys_Tick(PUBLIC(RO(Proc)));
	} else {
		Core->PowerThermal.VID = 0;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula)) {
	case FORMULA_SCOPE_CORE:
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1))
	    {
		Core_AMD_Family_17h_Temp(Core);
		break;
	    }
		/* Fallthrough */
	case FORMULA_SCOPE_SMT:
		Core_AMD_Family_17h_Temp(Core);
		break;
	}

	switch (SCOPE_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula)) {
	case FORMULA_SCOPE_CORE:
	    if (!((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)))
	    {
		break;
	    }
		/* Fallthrough */
	case FORMULA_SCOPE_SMT:
		Core->PowerThermal.VID = PstateDef.Family_17h.CpuVid;
		break;
	}

	/*		Read the Physical Core RAPL counter.		*/
    if (Core->T.ThreadID == 0)
    {
	RDCOUNTER(Core->Counter[1].Power.ACCU,MSR_AMD_PP0_ENERGY_STATUS);
	Core->Counter[1].Power.ACCU &= 0xffffffff;

	Core->Delta.Power.ACCU  = Core->Counter[1].Power.ACCU
				- Core->Counter[0].Power.ACCU;

	Core->Delta.Power.ACCU &= 0xffffffff;

	Core->Counter[0].Power.ACCU = Core->Counter[1].Power.ACCU;
    }

	Delta_INST(Core);

	Delta_C0(Core);

	Delta_C3(Core);

	Delta_C6(Core);

	Delta_TSC_OVH(Core);

	Delta_C1(Core);

	Save_INST(Core);

	Save_TSC(Core);

	Save_C0(Core);

	Save_C3(Core);

	Save_C6(Core);

	Save_C1(Core);

	BITSET(LOCKLESS, PUBLIC(RW(Core, AT(Core->Bind)))->Sync.V, NTFY);
}

void Call_SVI(	const unsigned int plane0, const unsigned int plane1,
		const unsigned long long factor )
{
	AMD_17_SVI SVI = {.value = 0};
	unsigned long long VCC, ICC, PWR;

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_F17H_SVI(plane0),
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	PUBLIC(RO(Proc))->PowerThermal.VID.CPU = SVI.VID;

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_F17H_SVI(plane1),
				SMU_AMD_INDEX_REGISTER_F17H,
				SMU_AMD_DATA_REGISTER_F17H );

	PUBLIC(RO(Proc))->PowerThermal.VID.SOC = SVI.VID;

	/*	PLATFORM RAPL workaround to provide the SoC power	*/
	VCC = 155000LLU - (625LLU * SVI.VID);
	ICC = SVI.IDD * factor;
	PWR = VCC * ICC;
	PWR = PWR << PUBLIC(RO(Proc))->PowerThermal.Unit.ESU;
	PWR = PWR / (100000LLU * 1000000LLU);
	PUBLIC(RW(Proc))->Delta.Power.ACCU[PWR_DOMAIN(PLATFORM)] = PWR;
}

void Call_DFLT( const unsigned int plane0, const unsigned int plane1,
		const unsigned long long factor )
{
	PUBLIC(RO(Proc))->PowerThermal.VID.CPU = \
	PUBLIC(RO(Core,AT( PUBLIC(RO(Proc))->Service.Core )))->PowerThermal.VID;
}

static enum hrtimer_restart Entry_AMD_F17h(struct hrtimer *pTimer,
		void (*Call_SMU)(const unsigned int, const unsigned int,
				const unsigned long long),
		const unsigned int plane0, const unsigned int plane1,
		const unsigned long long factor)
{
	CORE_RO *Core;
	unsigned int cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

	if (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD) == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Cycle_AMD_Family_17h(Core, Call_SMU, plane0, plane1, factor);

		return (HRTIMER_RESTART);
	} else
		return (HRTIMER_NORESTART);
}

static enum hrtimer_restart Cycle_AMD_F17h_Zen(struct hrtimer *pTimer)
{
	return ( Entry_AMD_F17h(pTimer, Call_SVI, 0, 1, 360772LLU) );
}
static enum hrtimer_restart Cycle_AMD_F17h_Zen2_SP(struct hrtimer *pTimer)
{
	return ( Entry_AMD_F17h(pTimer, Call_SVI, 1, 0, 294300LLU) );
}
static enum hrtimer_restart Cycle_AMD_F17h_Zen2_MP(struct hrtimer *pTimer)
{
	return ( Entry_AMD_F17h(pTimer, Call_SVI, 2, 1, 294300LLU) );
}
static enum hrtimer_restart Cycle_AMD_F17h(struct hrtimer *pTimer)
{
	return ( Entry_AMD_F17h(pTimer, Call_DFLT, 0, 0, 0LLU) );
}

void InitTimer_AMD_Family_17h(unsigned int cpu)
{
    switch (PUBLIC(RO(Proc))->ArchID) {
    case AMD_Zen:
    case AMD_ZenPlus:
    case AMD_ZenPlus_APU:
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen, 1);
	break;
    case AMD_Zen3_VMR:
    case AMD_Zen2_MTS:
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen2_SP, 1);
	break;
    case AMD_EPYC_Rome:
    case AMD_Zen2_CPK:
    case AMD_Zen2_APU:
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen2_MP, 1);
	break;
    default:
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h, 1);
	break;
    }
}

static void Start_AMD_Family_17h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	AMD_Core_Counters_Set(Core, Family_17h);
	SMT_Counters_AMD_Family_17h(Core, 0);

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
	}
	PKG_Counters_Generic(Core, 0);

	RDCOUNTER(PUBLIC(RO(Proc))->Counter[0].Power.ACCU[PWR_DOMAIN(PKG)],
			MSR_AMD_PKG_ENERGY_STATUS );
    }
    if (Core->T.ThreadID == 0)
    {
	RDCOUNTER(Core->Counter[0].Power.ACCU,MSR_AMD_PP0_ENERGY_STATUS);
	Core->Counter[0].Power.ACCU &= 0xffffffff;
    }

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Join, AT(cpu)))->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

static void Stop_AMD_Family_17h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Join, AT(cpu)))->Timer);

	AMD_Core_Counters_Clear(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED);
}

long Sys_OS_Driver_Query(SYSGATE_RO *SysGate)
{
	int rc = RC_SUCCESS;
    if (SysGate != NULL)
    {
#ifdef CONFIG_CPU_FREQ
	const char *pFreqDriver;
	struct cpufreq_policy freqPolicy;
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_CPU_IDLE
	struct cpuidle_driver *idleDriver;
#endif /* CONFIG_CPU_IDLE */
	memset(&SysGate->OS, 0, sizeof(OS_DRIVER));
#ifdef CONFIG_CPU_IDLE
	if ((idleDriver = cpuidle_get_driver()) != NULL) {
		int i;
		StrCopy(SysGate->OS.IdleDriver.Name,
			idleDriver->name, CPUIDLE_NAME_LEN);

	    if (idleDriver->state_count < CPUIDLE_STATE_MAX) {
		SysGate->OS.IdleDriver.stateCount = idleDriver->state_count;
	    } else {
		SysGate->OS.IdleDriver.stateCount = CPUIDLE_STATE_MAX;
	    }
		SysGate->OS.IdleDriver.stateLimit = idleDriver->state_count;

	    for (i = 0; i < SysGate->OS.IdleDriver.stateCount; i++)
	    {
		StrCopy(SysGate->OS.IdleDriver.State[i].Name,
			idleDriver->states[i].name, CPUIDLE_NAME_LEN);

		StrCopy(SysGate->OS.IdleDriver.State[i].Desc,
			idleDriver->states[i].desc, CPUIDLE_NAME_LEN);

		SysGate->OS.IdleDriver.State[i].exitLatency =
				idleDriver->states[i].exit_latency;

		SysGate->OS.IdleDriver.State[i].powerUsage =
				idleDriver->states[i].power_usage;

		SysGate->OS.IdleDriver.State[i].targetResidency =
				idleDriver->states[i].target_residency;
	    }
	}
#endif /* CONFIG_CPU_IDLE */
#ifdef CONFIG_CPU_FREQ
	if ((pFreqDriver = cpufreq_get_current_driver()) != NULL) {
		StrCopy(SysGate->OS.FreqDriver.Name,
			pFreqDriver, CPUFREQ_NAME_LEN);
	}
	memset(&freqPolicy, 0, sizeof(freqPolicy));
     if((rc=cpufreq_get_policy(&freqPolicy, PUBLIC(RO(Proc))->Service.Core))==0)
     {
	struct cpufreq_governor *pGovernor = freqPolicy.governor;
	if (pGovernor != NULL) {
		StrCopy(SysGate->OS.FreqDriver.Governor,
			pGovernor->name, CPUFREQ_NAME_LEN);
	} else {
		SysGate->OS.FreqDriver.Governor[0] = '\0';
	}
     } else {
	SysGate->OS.FreqDriver.Governor[0] = '\0';
     }
#endif /* CONFIG_CPU_FREQ */
    } else {
	rc = -ENXIO;
    }
	return (rc);
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

		return (RC_OK_SYSGATE);
	} else {
		return (-ENXIO);
	}
}

long SysGate_OnDemand(void)
{
	long rc = -1;
    if (PUBLIC(OF(Gate)) == NULL)
    {	/*			On-demand allocation.			*/
	PUBLIC(OF(Gate)) = alloc_pages_exact( PUBLIC(RO(Proc))->OS.ReqMem.Size,
						GFP_KERNEL );
      if (PUBLIC(OF(Gate)) != NULL)
      {
	const size_t allocPages=PAGE_SIZE << PUBLIC(RO(Proc))->OS.ReqMem.Order;
		memset(PUBLIC(OF(Gate)), 0, allocPages);
		rc = 0;
      }
    } else {					/* Already allocated	*/
		rc = 1;
    }
	return (rc);
}

#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	/*			MWAIT Idle methods			*/
static int CoreFreqK_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{/*	Source: /drivers/cpuidle/cpuidle.c				*/
	unsigned long MWAIT=(CoreFreqK.IdleDriver.states[index].flags>>24)&0xff;
	mwait_idle_with_hints(MWAIT, 1UL);
	return index;
}

static int CoreFreqK_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{/* Avoid kernel idle loop to be stuck if the MWAIT opcode has been disabled */
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	return (CoreFreqK_MWAIT_Handler(pIdleDevice, pIdleDriver, index));
    } else {
	return index;
    }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_S2_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#else
static int CoreFreqK_S2_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#endif /* 5.9.0 */
{
	unsigned long MWAIT=(CoreFreqK.IdleDriver.states[index].flags>>24)&0xff;
	mwait_idle_with_hints(MWAIT, 1UL);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
	return index;
#endif /* 5.9.0 */
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_S2_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	CoreFreqK_S2_MWAIT_Handler(pIdleDevice, pIdleDriver, index);
    }
}
#else
static int CoreFreqK_S2_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	return (CoreFreqK_S2_MWAIT_Handler(pIdleDevice, pIdleDriver, index));
    } else {
	return index;
    }
}
#endif /* 5.9.0 */
	/*			HALT Idle methods			*/
static int CoreFreqK_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{/*	Source: /drivers/acpi/processor_idle.c				*/
	safe_halt();
	return index;
}

static int CoreFreqK_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	return (CoreFreqK_HALT_Handler(pIdleDevice, pIdleDriver, index));
    } else {
	return index;
    }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#else
static int CoreFreqK_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#endif /* 5.9.0 */
{
	safe_halt();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
	return index;
#endif /* 5.9.0 */
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	CoreFreqK_S2_HALT_Handler(pIdleDevice, pIdleDriver, index);
    }
}
#else
static int CoreFreqK_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	return (CoreFreqK_S2_HALT_Handler(pIdleDevice, pIdleDriver, index));
    } else {
	return index;
    }
}
#endif /* 5.9.0 */
	/*			I/O Idle methods			*/
static int CoreFreqK_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	const unsigned short lvl = \
			(CoreFreqK.IdleDriver.states[index].flags >> 28) & 0xf;
	const unsigned short cstate_addr = Core->Query.CStateBaseAddr + lvl;

	inw(cstate_addr);
	return index;
}

static int CoreFreqK_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	return (CoreFreqK_IO_Handler(pIdleDevice, pIdleDriver, index));
    } else {
	return index;
    }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#else
static int CoreFreqK_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#endif /* 5.9.0 */
{
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	const unsigned short lvl = \
			(CoreFreqK.IdleDriver.states[index].flags >> 28) & 0xf;
	const unsigned short cstate_addr = Core->Query.CStateBaseAddr + lvl;

	inw(cstate_addr);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
	return index;
#endif /* 5.9.0 */
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	CoreFreqK_S2_IO_Handler(pIdleDevice, pIdleDriver, index);
    }
}
#else
static int CoreFreqK_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
    if (BITVAL(HwCfgRegister.value, 9) == 0)
    {
	return (CoreFreqK_S2_IO_Handler(pIdleDevice, pIdleDriver, index));
    } else {
	return index;
    }
}
#endif /* 5.9.0 */
	/*		Idle Cycles callback functions			*/
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

static int Alternative_Computation_Of_Cycles(
	int (*Handler)(struct cpuidle_device*, struct cpuidle_driver*, int),
			struct cpuidle_device *pIdleDevice,
			struct cpuidle_driver *pIdleDriver, int index
)
{
	unsigned long long TSC[2] __attribute__ ((aligned (8)));
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	const unsigned short lvl = \
			(CoreFreqK.IdleDriver.states[index].flags >> 28) & 0xf;

	if ((PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (PUBLIC(RO(Proc))->Features.ExtInfo.EDX.RDTSCP == 1))
	{
		RDTSCP64(TSC[0]);

		Handler(pIdleDevice, pIdleDriver, index);

		RDTSCP64(TSC[1]);
	}
	else
	{
		RDTSC64(TSC[0]);

		Handler(pIdleDevice, pIdleDriver, index);

		RDTSC64(TSC[1]);
	}
	TSC[1] = TSC[1] - TSC[0];

	Atomic_Write_VPMC(Core, TSC[1], lvl);

	return index;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void Alternative_Computation_Of_Cycles_S2(
	void (*S2_Handler)(struct cpuidle_device*, struct cpuidle_driver*, int),
				struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index
)
#else
static int Alternative_Computation_Of_Cycles_S2(
	int (*S2_Handler)(struct cpuidle_device*, struct cpuidle_driver*, int),
			struct cpuidle_device *pIdleDevice,
			struct cpuidle_driver *pIdleDriver, int index
)
#endif /* 5.9.0 */
{
	unsigned long long TSC[2] __attribute__ ((aligned (8)));
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	const unsigned short lvl = \
			(CoreFreqK.IdleDriver.states[index].flags >> 28) & 0xf;

	if ((PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (PUBLIC(RO(Proc))->Features.ExtInfo.EDX.RDTSCP == 1))
	{
		RDTSCP64(TSC[0]);

		S2_Handler(pIdleDevice, pIdleDriver, index);

		RDTSCP64(TSC[1]);
	}
	else
	{
		RDTSC64(TSC[0]);

		S2_Handler(pIdleDevice, pIdleDriver, index);

		RDTSC64(TSC[1]);
	}
	TSC[1] = TSC[1] - TSC[0];

	Atomic_Write_VPMC(Core, TSC[1], lvl);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
	return index;
#endif /* 5.9.0 */
}
	/*		Alternative Idle methods			*/
static int CoreFreqK_Alt_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles( CoreFreqK_MWAIT_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles( CoreFreqK_HALT_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles( CoreFreqK_IO_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles( CoreFreqK_MWAIT_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles( CoreFreqK_HALT_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles( CoreFreqK_IO_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
static void CoreFreqK_Alt_S2_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static void CoreFreqK_Alt_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_HALT_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static void CoreFreqK_Alt_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_IO_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}
static void CoreFreqK_Alt_S2_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static void CoreFreqK_Alt_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_HALT_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static void CoreFreqK_Alt_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_IO_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}
#else
static int CoreFreqK_Alt_S2_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_HALT_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_IO_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static int CoreFreqK_Alt_S2_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
return Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_AMD_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
}

static int CoreFreqK_Alt_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
return Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_HALT_AMD_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
}

static int CoreFreqK_Alt_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
return Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_IO_AMD_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
}
#endif /* 5.9.0 */
#endif /* CONFIG_CPU_IDLE and 4.14.0 */

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
	} else {
		struct cpuidle_device *device;
		unsigned int cpu, enroll = 0;
		unsigned int subState[] = {
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT0,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT1,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT1, /* C1E */
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT2,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT3,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT4,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT5,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT6,
		PUBLIC(RO(Proc))->Features.MWait.EDX.SubCstate_MWAIT7
		};
		const unsigned int subStateCount = sizeof(subState)
						 / sizeof(subState[0]);
		/*		Kernel polling loop			*/
		cpuidle_poll_state_init(&CoreFreqK.IdleDriver);

		CoreFreqK.IdleDriver.state_count = 1;
		/*		Idle States				*/
	    while (pIdleState->Name != NULL)
	    {
		if (CoreFreqK.IdleDriver.state_count < subStateCount)
		{
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].flags = pIdleState->flags;

		    if (subState[CoreFreqK.IdleDriver.state_count] == 0)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].flags = pIdleState->flags |= CPUIDLE_FLAG_UNUSABLE;
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
		    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_Alt_MWAIT_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_Alt_S2_MWAIT_Handler;
		    }
		  else if(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_Alt_MWAIT_AMD_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_Alt_S2_MWAIT_AMD_Handler;
		    }
		  else {
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
			break;
		  case ROUTE_HALT:
		    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_Alt_HALT_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_Alt_S2_HALT_Handler;
		    }
		  else if(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_Alt_HALT_AMD_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_Alt_S2_HALT_AMD_Handler;
		    }
		  else {
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
			break;
		  case ROUTE_IO:
		  {
		    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
		    {
			CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
			RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			if (CState_IO_MWAIT.LVL2_BaseAddr != 0x0)
			{
				CoreFreqK.IdleDriver.states[
					CoreFreqK.IdleDriver.state_count
				].enter = CoreFreqK_Alt_IO_Handler;

				CoreFreqK.IdleDriver.states[
					CoreFreqK.IdleDriver.state_count
				].enter_s2idle = CoreFreqK_Alt_S2_IO_Handler;
			} else {
				goto IDLE_DEFAULT;
			}
		    }
		  else if(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		    {
			CSTATE_BASE_ADDR CStateBaseAddr = {.value = 0};
			RDMSR(CStateBaseAddr, MSR_AMD_CSTATE_BAR);
			if (CStateBaseAddr.IOaddr != 0x0)
			{
				CoreFreqK.IdleDriver.states[
					CoreFreqK.IdleDriver.state_count
				].enter = CoreFreqK_Alt_IO_AMD_Handler;

				CoreFreqK.IdleDriver.states[
					CoreFreqK.IdleDriver.state_count
				].enter_s2idle=CoreFreqK_Alt_S2_IO_AMD_Handler;
			} else {
				goto IDLE_DEFAULT;
			}
		    }
		  else {
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
			break;
		  case ROUTE_DEFAULT:
		  IDLE_DEFAULT:
		  default:
		    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_MWAIT_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_S2_MWAIT_Handler;
		    }
		  else if(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		    {
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_HALT_AMD_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_S2_HALT_AMD_Handler;
		    }
		  else {
			pr_warn("CoreFreq: "	\
				"No Idle implementation for this architecture");
		  }
			break;
		  }
			CoreFreqK.IdleDriver.state_count++;
		}
		pIdleState++;
	    }
	    if ((rc = cpuidle_register_driver(&CoreFreqK.IdleDriver)) == 0) {
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
	    {/* Cancel the registration if the driver and/or a device failed */
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
	return (rc);
}

#ifdef CONFIG_CPU_IDLE
static void CoreFreqK_Idle_State_Withdraw(int idx, bool disable)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
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
    if ((PUBLIC(OF(Gate)) != NULL) && (floor != -1)) {
	PUBLIC(OF(Gate))->OS.IdleDriver.stateLimit = 1 + floor;
    }
#endif /* CONFIG_CPU_IDLE */
	return (rc);
}

#ifdef CONFIG_CPU_FREQ
static int CoreFreqK_Policy_Exit(struct cpufreq_policy *policy)
{
	return (0);
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
	return (0);
}

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 19))	\
  && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 5, 0)))	\
  || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 3))
static int CoreFreqK_Policy_Verify(struct cpufreq_policy_data *policy)
#else
static int CoreFreqK_Policy_Verify(struct cpufreq_policy *policy)
#endif
{
	if (policy != NULL) {
		cpufreq_verify_within_cpu_limits(policy);
	}
	return (0);
}

static int CoreFreqK_SetPolicy(struct cpufreq_policy *policy)
{
	return (0);
}

static int CoreFreqK_Bios_Limit(int cpu, unsigned int *limit)
{
    if ((cpu >= 0) && (cpu < PUBLIC(RO(Proc))->CPU.Count) && (limit != NULL))
    {
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	(*limit) = (Core->Boost[BOOST(MAX)] * Core->Clock.Hz) / 1000LLU;
    }
	return (0);
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
static int CoreFreqK_SetBoost(struct cpufreq_policy *policy, int state)
#else
static int CoreFreqK_SetBoost(int state)
#endif /* 5.8.0 */
{
	Controller_Stop(1);
	TurboBoost_Enable = (state != 0);
	Controller_Start(1);
	TurboBoost_Enable = -1;
	Policy_Aggregate_Turbo();
	BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY); /* Notify Daemon*/
	return (0);
}

static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy *policy,char *buf)
{
  if (policy != NULL) {
	CORE_RO *Core;
    if (policy->cpu < PUBLIC(RO(Proc))->CPU.Count)
    {
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(policy->cpu)));
    } else {
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(PUBLIC(RO(Proc))->Service.Core)));
    }
	return ( sprintf(buf, "%7llu\n",
			(Core->Boost[BOOST(TGT)] * Core->Clock.Hz) / 1000LLU) );
  }
	return (0);
}

static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy *policy,
					unsigned int freq)
{
    if (policy != NULL) {
	if ((policy->cpu < PUBLIC(RO(Proc))->CPU.Count)
	 && (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget != NULL))
	{
		CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(policy->cpu)));
		unsigned int ratio = (freq * 1000LLU) / Core->Clock.Hz;

	    if (ratio > 0) {
		if (smp_call_function_single(policy->cpu,
			Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget,
						&ratio, 1) == 0)
		{
			BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY);
		}
	    }
		return (0);
	}
    }
	return (-EINVAL);
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
	return (CPU_Freq);
}

static void Policy_Core2_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if ((*ratio) <= Core->Boost[BOOST(1C)])
    {
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Set_Core2_Target(Core, (*ratio));
	WritePerformanceControl(&Core->PowerThermal.PerfControl);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA) {
	    if (Cmp_Core2_Target(Core, 1 + Core->Boost[BOOST(MAX)]))
	    {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    }
	}
	Core->Boost[BOOST(TGT)] = Get_Core2_Target(Core);
    }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_Nehalem_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if ((*ratio) <= Core->Boost[BOOST(1C)])
    {
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Set_Nehalem_Target(Core, (*ratio));
	WritePerformanceControl(&Core->PowerThermal.PerfControl);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA) {
	    if (Cmp_Nehalem_Target(Core, 1 + Core->Boost[BOOST(MAX)]))
	    {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    }
	}
	Core->Boost[BOOST(TGT)] = Get_Nehalem_Target(Core);
    }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_SandyBridge_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if ((*ratio) <= Core->Boost[BOOST(1C)])
    {
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Set_SandyBridge_Target(Core, (*ratio));
	WritePerformanceControl(&Core->PowerThermal.PerfControl);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA) {
	    if (Cmp_SandyBridge_Target(Core, Core->Boost[BOOST(MAX)]))
	    {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	    }
	}
	Core->Boost[BOOST(TGT)] = Get_SandyBridge_Target(Core);
    }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_HWP_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
  if (PUBLIC(RO(Proc))->Features.HWP_Enable)
  {
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
    if (((*ratio) >= Core->PowerThermal.HWP_Capabilities.Lowest)
     && ((*ratio) <= Core->PowerThermal.HWP_Capabilities.Highest))
    {
	Core->PowerThermal.HWP_Request.Maximum_Perf =	\
	Core->PowerThermal.HWP_Request.Desired_Perf = (*ratio);
	WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

	Core->Boost[BOOST(HWP_MAX)]=Core->PowerThermal.HWP_Request.Maximum_Perf;
	Core->Boost[BOOST(HWP_TGT)]=Core->PowerThermal.HWP_Request.Desired_Perf;
    }
  } else {
	Policy_SandyBridge_SetTarget(arg);
  }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_Zen_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	PSTATEDEF PstateDef;
	unsigned int COF, pstate;
	unsigned short WrModRd = 0;
	/* Look-up for the first enabled P-State with the same target ratio */
	for (pstate = 0; pstate <= 7; pstate++)
	{
		PstateDef.value = 0;
		RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + pstate);

	    if (PstateDef.Family_17h.PstateEn)
	    {
		COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId );
		if (COF == (*ratio)) {
			WrModRd = 1;
			break;
		}
	    }
	}
	if (WrModRd == 1)
	{
		PSTATECTRL PstateCtrl;
		/*	Write-Modify-Read the new target P-state	*/
		PstateCtrl.value = 0;
		RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
		PstateCtrl.PstateCmd = pstate;
		WRMSR(PstateCtrl, MSR_AMD_PERF_CTL);

		PstateCtrl.value = 0;
		RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
		PstateDef.value = 0;
		RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + pstate);

		COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
					PstateDef.Family_17h.CpuDfsId );

		Core->Boost[BOOST(TGT)] = COF;
	}
#endif /* CONFIG_CPU_FREQ */
}

static int CoreFreqK_FreqDriver_UnInit(void)
{
	int rc = -EINVAL;
#ifdef CONFIG_CPU_FREQ
	rc = cpufreq_unregister_driver(&CoreFreqK.FreqDriver);
#endif /* CONFIG_CPU_FREQ */
	return (rc);
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
	return (rc);
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
	return (rc);
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
		return ((signed int) seek);
	}
    }
	return (-1);
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
		return ((signed int) seek);
	}
    }
	return (-1);
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
			return (0);
		}
	}
	else if (PUBLIC(RO(Core, AT(cpu)))->T.ThreadID > 0)
	{
		if ((seek = Seek_Topology_Core_Peer(cpu, cpx)) != -1) {
			pService->Core = seek;
			pService->Thread = cpu;
			return (0);
		}
	}
	while (cpn < PUBLIC(RO(Proc))->CPU.Count) {
		cpu = cpn++;
		if (!BITVAL(PUBLIC(RO(Core, AT(cpu)))->OffLine, OS)) {
			goto MATCH;
		}
	}
	return (-1);
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
	if (ServiceProcessor != -1) {
		DefaultSMT.Proc = pService->Proc;
	}
}

void MatchPeerForUpService(SERVICE_PROC *pService, unsigned int cpu)
{	/* Try to restore the initial Service affinity or move to SMT peer. */
	SERVICE_PROC hService = {
		.Core = cpu,
		.Thread = -1,
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
	if (pService->Proc != DefaultSMT.Proc)
	{
		if (hService.Proc == DefaultSMT.Proc) {
			pService->Proc = hService.Proc;
		} else {
			if ((pService->Thread == -1) && (hService.Thread > 0))
			{
				pService->Proc = hService.Proc;
			}
		}
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
static int CoreFreqK_NMI_Handler(unsigned int type, struct pt_regs *pRegs)
{
	unsigned int cpu = smp_processor_id();

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
	return (NMI_DONE);
}

static long CoreFreqK_UnRegister_CPU_Idle(void)
{
	long rc = -EINVAL;
    if (PUBLIC(RO(Proc))->Registration.Driver.CPUidle & REGISTRATION_ENABLE)
    {
	CoreFreqK_IdleDriver_UnInit();
	PUBLIC(RO(Proc))->Registration.Driver.CPUidle = REGISTRATION_DISABLE;
	rc = RC_SUCCESS;
    }
	return (rc);
}

static long CoreFreqK_Register_CPU_Idle(void)
{
	long rc = -EINVAL;
  if (Register_CPU_Idle == 1)
  {
	int rx = CoreFreqK_IdleDriver_Init();
    switch ( rx ) {
    default:
	/* Fallthrough */
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
	return (rc);
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
	return (rc);
}

static long CoreFreqK_Register_CPU_Freq(void)
{ /* Source: cpufreq_register_driver @ /drivers/cpufreq/cpufreq.c	*/
	long rc = -EINVAL;
  if (Register_CPU_Freq == 1)
  {
	int rx = CoreFreqK_FreqDriver_Init();
    switch ( rx ) {
    default:
	/* Fallthrough */
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
	return (rc);
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
	return (rc);
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
	return (rc);
}

static void CoreFreqK_Register_NMI(void)
{
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
}

static void CoreFreqK_UnRegister_NMI(void)
{
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
}
#else
static void CoreFreqK_Register_NMI(void) {}
static void CoreFreqK_UnRegister_NMI(void) {}
#endif


static long CoreFreqK_Thermal_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	PUBLIC(RO(Proc))->thermalFormula = \
		(KIND_OF_FORMULA(PUBLIC(RO(Proc))->thermalFormula) << 8)|scope;

	return (RC_SUCCESS);
    } else {
	return (-EINVAL);
    }
}

static long CoreFreqK_Voltage_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	PUBLIC(RO(Proc))->voltageFormula = \
		(KIND_OF_FORMULA(PUBLIC(RO(Proc))->voltageFormula) << 8)|scope;

	return (RC_SUCCESS);
    } else {
	return (-EINVAL);
    }
}

static long CoreFreqK_Power_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	PUBLIC(RO(Proc))->powerFormula = \
		(KIND_OF_FORMULA(PUBLIC(RO(Proc))->powerFormula) << 8)|scope;

	return (RC_SUCCESS);
    } else {
	return (-EINVAL);
    }
}

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

static long CoreFreqK_ioctl(	struct file *filp,
				unsigned int cmd,
				unsigned long arg )
{
	long rc = -EPERM;

    switch (cmd)
    {
    case COREFREQ_IOCTL_SYSUPDT:
    SYSGATE_UPDATE:
	rc = Sys_OS_Driver_Query(PUBLIC(OF(Gate)));
	rc = (rc != -ENXIO) ? RC_OK_SYSGATE : rc;
    break;

    case COREFREQ_IOCTL_SYSONCE:
	rc = Sys_OS_Driver_Query(PUBLIC(OF(Gate)));
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
		PUBLIC(RO(Proc))->Registration.PCI = CoreFreqK_ProbePCI() == 0;
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
		Controller_Start(1);
		if (rc == RC_SUCCESS) {
			goto SYSGATE_UPDATE;
		}
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_CPU_Idle = 1;
		rc = CoreFreqK_Register_CPU_Idle();
		Controller_Start(1);
		if (rc == RC_SUCCESS) {
			goto SYSGATE_UPDATE;
		}
		break;
	}
	break;

      case MACHINE_CPU_FREQ:
	switch (prm.dl.lo)
	{
	    case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		rc = CoreFreqK_UnRegister_CPU_Freq();
		Register_CPU_Freq = -1;
		Controller_Start(1);
		if (rc == RC_SUCCESS) {
			goto SYSGATE_UPDATE;
		}
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_CPU_Freq = 1;
		rc = CoreFreqK_Register_CPU_Freq();
		Controller_Start(1);
		if (rc == RC_SUCCESS) {
			goto SYSGATE_UPDATE;
		}
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
		Controller_Start(1);
		if (rc == RC_SUCCESS) {
			goto SYSGATE_UPDATE;
		}
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Register_Governor = 1;
		rc = CoreFreqK_Register_Governor();
		Controller_Start(1);
		if (rc == RC_SUCCESS) {
			goto SYSGATE_UPDATE;
		}
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
	case TECHNOLOGY_EIST:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			SpeedStep_Enable = prm.dl.lo;
			Controller_Start(1);
			SpeedStep_Enable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_C1E:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			C1E_Enable = prm.dl.lo;
			Controller_Start(1);
			C1E_Enable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_TURBO:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				TurboBoost_Enable = prm.dl.lo;
				Controller_Start(1);
				TurboBoost_Enable = -1;
			#ifdef CONFIG_CPU_FREQ
				Policy_Aggregate_Turbo();
			#endif /* CONFIG_CPU_FREQ */
				rc = RC_OK_COMPUTE;
				break;
		}
		break;

	case TECHNOLOGY_C1A:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				C1A_Enable = prm.dl.lo;
				Controller_Start(1);
				C1A_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_C3A:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				C3A_Enable = prm.dl.lo;
				Controller_Start(1);
				C3A_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_C1U:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				C1U_Enable = prm.dl.lo;
				Controller_Start(1);
				C1U_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_C3U:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				C3U_Enable = prm.dl.lo;
				Controller_Start(1);
				C3U_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_CC6:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				CC6_Enable = prm.dl.lo;
				Controller_Start(1);
				CC6_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_PC6:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				PC6_Enable = prm.dl.lo;
				Controller_Start(1);
				PC6_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_PKG_CSTATE:
		Controller_Stop(1);
		PkgCStateLimit = prm.dl.lo;
		Controller_Start(1);
		PkgCStateLimit = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_IO_MWAIT:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				IOMWAIT_Enable = prm.dl.lo;
				Controller_Start(1);
				IOMWAIT_Enable = -1;
				rc = RC_SUCCESS;
				break;
		}
		break;

	case TECHNOLOGY_IO_MWAIT_REDIR:
		Controller_Stop(1);
		CStateIORedir = prm.dl.lo;
		Controller_Start(1);
		CStateIORedir = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_ODCM:
		Controller_Stop(1);
		ODCM_Enable = prm.dl.lo;
		Controller_Start(1);
		ODCM_Enable = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_ODCM_DUTYCYCLE:
		Controller_Stop(1);
		ODCM_DutyCycle = prm.dl.lo;
		Controller_Start(1);
		ODCM_DutyCycle = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_POWER_POLICY:
		Controller_Stop(1);
		PowerPolicy = prm.dl.lo;
		Controller_Start(1);
		PowerPolicy = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_HWP:
		Controller_Stop(1);
		HWP_Enable = prm.dl.lo;
		Intel_Hardware_Performance();
		Controller_Start(1);
		HWP_Enable = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_HWP_EPP:
		Controller_Stop(1);
		HWP_EPP = prm.dl.lo;
		Controller_Start(1);
		HWP_EPP = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_HDC:
		Controller_Stop(1);
		HDC_Enable = prm.dl.lo;
		Intel_Hardware_Performance();
		Controller_Start(1);
		HDC_Enable = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_R2H:
	    if (PUBLIC(RO(Proc))->Features.R2H_Capable)
	    {
		R2H_Disable = prm.dl.lo;
		Intel_RaceToHalt();
		R2H_Disable = -1;
		rc = RC_SUCCESS;
	    } else {
		rc = -ENXIO;
	    }
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
		case EVENT_THERM_SENSOR:
		case EVENT_THERM_PROCHOT:
		case EVENT_THERM_CRIT:
		case EVENT_THERM_THOLD:
		case EVENT_POWER_LIMIT:
		case EVENT_CURRENT_LIMIT:
		case EVENT_CROSS_DOMAIN:
			Controller_Stop(1);
			Clear_Events = arg;
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
	return (rc);
}

static int CoreFreqK_mmap(struct file *pfile, struct vm_area_struct *vma)
{
	unsigned long reqSize = vma->vm_end - vma->vm_start;
	int rc = -EIO;

  if (vma->vm_pgoff == ID_RO_VMA_PROC) {
    if (PUBLIC(RO(Proc)) != NULL)
    {
	const unsigned long secSize = ROUND_TO_PAGES(sizeof(PROC_RO));
	if (reqSize != secSize) {
		rc = -EAGAIN;
		goto EXIT_PAGE;
	}

	vma->vm_flags = VM_READ;
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
		/* Fallthrough */
	case 0: {
		const unsigned long secSize = \
				PAGE_SIZE << PUBLIC(RO(Proc))->OS.ReqMem.Order;
		if (reqSize != secSize) {
			return (-EAGAIN);
		}

		vma->vm_flags = VM_READ;
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

		vma->vm_flags = VM_READ;
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
	return (rc);
}

static DEFINE_MUTEX(CoreFreqK_mutex);		/* Only one driver instance. */

static int CoreFreqK_open(struct inode *inode, struct file *pfile)
{
	if (!mutex_trylock(&CoreFreqK_mutex))
		return (-EBUSY);
	else
		return (0);
}

static int CoreFreqK_release(struct inode *inode, struct file *pfile)
{
	mutex_unlock(&CoreFreqK_mutex);
	return (0);
}

static struct file_operations CoreFreqK_fops = {
	.open		= CoreFreqK_open,
	.release	= CoreFreqK_release,
	.mmap		= CoreFreqK_mmap,
	.unlocked_ioctl = CoreFreqK_ioctl,
	.owner		= THIS_MODULE,
};

#ifdef CONFIG_PM_SLEEP
static int CoreFreqK_Suspend(struct device *dev)
{
	Controller_Stop(1);

	printk(KERN_NOTICE "CoreFreq: Suspend\n");

	return (0);
}

static int CoreFreqK_Resume(struct device *dev)
{	/*		Probe Processor again				*/
    if (Arch[PUBLIC(RO(Proc))->ArchID].Query != NULL) {
	Arch[PUBLIC(RO(Proc))->ArchID].Query(PUBLIC(RO(Proc))->Service.Core);
    }
	/*		Probe PCI again 				*/
    if (PUBLIC(RO(Proc))->Registration.PCI) {
	PUBLIC(RO(Proc))->Registration.PCI = CoreFreqK_ProbePCI() == 0;
    }
	Controller_Start(1);

#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
#endif /* CONFIG_CPU_FREQ */

	BITSET(BUS_LOCK, PUBLIC(RW(Proc))->OS.Signal, NTFY); /* Notify Daemon*/

	printk(KERN_NOTICE "CoreFreq: Resume\n");

	return (0);
}

static SIMPLE_DEV_PM_OPS(CoreFreqK_pm_ops, CoreFreqK_Suspend, CoreFreqK_Resume);
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
	/*		Start the collect timer dedicated to this CPU.	*/
   if (Arch[PUBLIC(RO(Proc))->ArchID].Timer != NULL) {
	Arch[PUBLIC(RO(Proc))->ArchID].Timer(cpu);
   }
   if ((BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED) == 0)
    && (Arch[PUBLIC(RO(Proc))->ArchID].Start != NULL)) {
		smp_call_function_single(cpu,
					Arch[PUBLIC(RO(Proc))->ArchID].Start,
					NULL, 0);
   }
	PUBLIC(RO(Proc))->CPU.OnLine++;
	BITCLR(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, OS);

	MatchPeerForUpService(&PUBLIC(RO(Proc))->Service, cpu);

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

	return (0);
  } else
	return (-EINVAL);
}

static int CoreFreqK_HotPlug_CPU_Offline(unsigned int cpu)
{
    if (cpu < PUBLIC(RO(Proc))->CPU.Count) {
	/*		Stop the associated collect timer.		*/
	if((BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, CREATED) == 1)
	&& (BITVAL(PRIVATE(OF(Join, AT(cpu)))->TSM, STARTED) == 1)
	&& (Arch[PUBLIC(RO(Proc))->ArchID].Stop != NULL)) {
		smp_call_function_single(cpu,
					Arch[PUBLIC(RO(Proc))->ArchID].Stop,
					NULL, 1);
	}
	PUBLIC(RO(Proc))->CPU.OnLine--;
	BITSET(LOCKLESS, PUBLIC(RO(Core, AT(cpu)))->OffLine, OS);

	/*		Seek for an alternate Service Processor.	*/
	if ((cpu == PUBLIC(RO(Proc))->Service.Core)
	 || (cpu == PUBLIC(RO(Proc))->Service.Thread))
	{
		MatchPeerForDownService(&PUBLIC(RO(Proc))->Service, cpu);

	  if (PUBLIC(RO(Proc))->Service.Core != cpu)
	  {
	    if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL)
	    {
		smp_call_function_single(PUBLIC(RO(Proc))->Service.Core,
					Arch[PUBLIC(RO(Proc))->ArchID].Update,
			PUBLIC(RO(Core, AT(PUBLIC(RO(Proc))->Service.Core))),1);
	    }
#if CONFIG_HAVE_PERF_EVENTS==1
		/*	Reinitialize the PMU Uncore counters.		*/
	    if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL)
	    {
		smp_call_function_single(PUBLIC(RO(Proc))->Service.Core,
				Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop,
					NULL, 1); /* Must wait! */
	    }
	    if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL)
	    {
		smp_call_function_single(PUBLIC(RO(Proc))->Service.Core,
				Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start,
					NULL, 0); /* Don't wait */
	    }
#endif /* CONFIG_HAVE_PERF_EVENTS */
	  }
	}
#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
#endif /* CONFIG_CPU_FREQ */
	return (0);
    } else
	return (-EINVAL);
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
	return (NOTIFY_OK);
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

static char *CoreFreqK_DevNode(struct device *dev, umode_t *mode)
{
	if (mode != NULL) {
		(*mode) = 0600 ; /*	Device access is crw------	*/
	}
	return (NULL);
}

static void CoreFreqK_Empty_Func_Level_Down(void)
{
}

static void CoreFreqK_Alloc_Features_Level_Down(void)
{
	printk(KERN_NOTICE "CoreFreq: Unload\n");
}

static int CoreFreqK_Alloc_Features_Level_Up(INIT_ARG *pArg)
{
	pArg->Features = kmalloc(sizeof(FEATURES), GFP_KERNEL);
	if (pArg->Features == NULL)
	{
		return (-ENOMEM);
	} else {
		memset(pArg->Features, 0, sizeof(FEATURES));
	}
	pArg->Brand = kmalloc(BRAND_SIZE, GFP_KERNEL);
	if (pArg->Brand == NULL)
	{
		return (-ENOMEM);
	} else {
		memset(pArg->Brand, 0x20, BRAND_SIZE);
	}
	return (0);
}

#define CoreFreqK_Query_Features_Level_Down CoreFreqK_Empty_Func_Level_Down

static int CoreFreqK_Query_Features_Level_Up(INIT_ARG *pArg)
{
	int rc = 0;
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
		unsigned int OS_Count = num_present_cpus();
		/*	Rely on the operating system's cpu counting.	*/
		if (pArg->SMT_Count != OS_Count) {
			pArg->SMT_Count = OS_Count;
		}
	} else {
		rc = -ENXIO;
	}
	return (rc);
}

static void CoreFreqK_Alloc_Device_Level_Down(void)
{
	unregister_chrdev_region(CoreFreqK.mkdev, 1);
}

static int CoreFreqK_Alloc_Device_Level_Up(INIT_ARG *pArg)
{
	CoreFreqK.kcdev = cdev_alloc();
	CoreFreqK.kcdev->ops = &CoreFreqK_fops;
	CoreFreqK.kcdev->owner = THIS_MODULE;

	if (alloc_chrdev_region(&CoreFreqK.nmdev, 0, 1, DRV_FILENAME) >= 0)
	{
		return (0);
	} else {
		return (-EBUSY);
	}
}

static void CoreFreqK_Make_Device_Level_Down(void)
{
	cdev_del(CoreFreqK.kcdev);
}

static int CoreFreqK_Make_Device_Level_Up(INIT_ARG *pArg)
{
	CoreFreqK.Major = MAJOR(CoreFreqK.nmdev);
	CoreFreqK.mkdev = MKDEV(CoreFreqK.Major, 0);

	if (cdev_add(CoreFreqK.kcdev, CoreFreqK.mkdev, 1) >= 0)
	{
		return (0);
	} else {
		return (-EBUSY);
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

	CoreFreqK.clsdev = class_create(THIS_MODULE, DRV_DEVNAME);
	CoreFreqK.clsdev->pm = COREFREQ_PM_OPS;
	CoreFreqK.clsdev->devnode = CoreFreqK_DevNode;

	if ((tmpDev = device_create(	CoreFreqK.clsdev, NULL,
					CoreFreqK.mkdev, NULL,
					DRV_DEVNAME)) != NULL)
	{
		return (0);
	} else {
		return (-EBUSY);
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
	const unsigned long publicSize	= sizeof(KPUBLIC)
					+ sizeof(CORE_RO*) * pArg->SMT_Count
					+ sizeof(CORE_RW*) * pArg->SMT_Count;

	if (((PUBLIC() = kmalloc(publicSize, GFP_KERNEL)) != NULL))
	{
		memset(PUBLIC(), 0, publicSize);

		PUBLIC(RO(Core))	= (CORE_RO**) &PUBLIC()
					+ sizeof(KPUBLIC);

		PUBLIC(RW(Core))	= (CORE_RW**) PUBLIC(RO(Core))
					+ sizeof(CORE_RO*) * pArg->SMT_Count;

		return (0);
	} else{
		return (-ENOMEM);
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
	const unsigned long privateSize = sizeof(KPRIVATE)
					+ sizeof(JOIN*) * pArg->SMT_Count;

	if (((PRIVATE() = kmalloc(privateSize, GFP_KERNEL)) != NULL))
	{
		memset(PRIVATE(), 0, privateSize);

		return (0);
	} else {
		return (-ENOMEM);
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

	if ( (PUBLIC(RO(Proc)) = kmalloc(procSize, GFP_KERNEL)) != NULL)
	{
		memset(PUBLIC(RO(Proc)), 0, procSize);

		return (0);
	} else {
		return (-ENOMEM);
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

	if ( (PUBLIC(RW(Proc)) = kmalloc(procSize, GFP_KERNEL)) != NULL)
	{
		memset(PUBLIC(RW(Proc)), 0, procSize);

		return (0);
	} else {
		return (-ENOMEM);
	}
}

#define CoreFreqK_Scale_And_Compute_Level_Down CoreFreqK_Empty_Func_Level_Down

static int CoreFreqK_Scale_And_Compute_Level_Up(INIT_ARG *pArg)
{
	SET_FOOTPRINT(PUBLIC(RO(Proc))->FootPrint,	\
					COREFREQ_MAJOR, \
					COREFREQ_MINOR, \
					COREFREQ_REV	);

	PUBLIC(RO(Proc))->CPU.Count = pArg->SMT_Count;
	/* PreCompute SysGate memory allocation. */
	PUBLIC(RO(Proc))->OS.ReqMem.Size = sizeof(SYSGATE_RO);
	PUBLIC(RO(Proc))->OS.ReqMem.Order = \
				get_order(PUBLIC(RO(Proc))->OS.ReqMem.Size);

	PUBLIC(RO(Proc))->Registration.AutoClock = AutoClock;
	PUBLIC(RO(Proc))->Registration.Experimental = Experimental;

	Compute_Interval();

	memcpy(&PUBLIC(RO(Proc))->Features, pArg->Features, sizeof(FEATURES));

	/* Initialize default uArch's codename with the CPUID brand. */
	Arch[GenuineArch].Architecture->CodeName = \
				PUBLIC(RO(Proc))->Features.Info.Vendor.ID;
	return (0);
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

	if ( (PUBLIC(OF(Cache)) = kmem_cache_create(	"corefreqk-pub",
							cacheSize, 0,
							SLAB_HWCACHE_ALIGN,
							NULL ) ) != NULL)
	{
		return (0);
	} else {
		return (-ENOMEM);
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
	const unsigned long joinSize = ROUND_TO_PAGES(sizeof(JOIN));

	if ( (PRIVATE(OF(Cache)) = kmem_cache_create(	"corefreqk-priv",
							joinSize, 0,
							SLAB_HWCACHE_ALIGN,
							NULL ) ) != NULL)
	{
		return (0);
	} else {
		return (-ENOMEM);
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
	    if (PRIVATE(OF(Join, AT(cpu))) != NULL) {
		kmem_cache_free(PRIVATE(OF(Cache)), PRIVATE(OF(Join, AT(cpu))));
	    }
	}
    }
}

static int CoreFreqK_Alloc_Per_CPU_Level_Up(INIT_ARG *pArg)
{
	const unsigned long cacheSize = KMAX( ROUND_TO_PAGES(sizeof(CORE_RO)),
					      ROUND_TO_PAGES(sizeof(CORE_RW)) );
	const unsigned long joinSize = ROUND_TO_PAGES(sizeof(JOIN));
	int rc = 0;

	unsigned int cpu;
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
			PRIVATE(OF(Join, AT(cpu))) = kcache;
		} else {
			rc = -ENOMEM;
			break;
		}

		BITCLR(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		PUBLIC(RO(Core, AT(cpu)))->Bind = cpu;

		Define_CPUID(PUBLIC(RO(Core, AT(cpu))), CpuIDforVendor);
	}
	return (rc);
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
	free_pages_exact(PUBLIC(OF(Gate)), PUBLIC(RO(Proc))->OS.ReqMem.Size);
    }
}

static int CoreFreqK_Ignition_Level_Up(INIT_ARG *pArg)
{
	BIT_ATOM_INIT(PRIVATE(OF(AMD_SMN_LOCK)), ATOMIC_SEED);
	BIT_ATOM_INIT(PRIVATE(OF(AMD_FCH_LOCK)), ATOMIC_SEED);

	switch (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC) {
	case CRC_INTEL: {
		Arch[GenuineArch].Query = Query_GenuineIntel;
		Arch[GenuineArch].Update= PerCore_Intel_Query;
		Arch[GenuineArch].Start = Start_GenuineIntel;
		Arch[GenuineArch].Stop	= Stop_GenuineIntel;
		Arch[GenuineArch].Timer = InitTimer_GenuineIntel;
		Arch[GenuineArch].BaseClock = BaseClock_GenuineIntel;

		Arch[GenuineArch].thermalFormula=THERMAL_FORMULA_INTEL;

		Arch[GenuineArch].voltageFormula=VOLTAGE_FORMULA_INTEL;

		Arch[GenuineArch].powerFormula = POWER_FORMULA_INTEL;
		}
		break;
	case CRC_HYGON:
		/* Fallthrough */
	case CRC_AMD: {
		Arch[GenuineArch].Query = Query_AuthenticAMD;
		Arch[GenuineArch].Update= PerCore_AuthenticAMD_Query;
		Arch[GenuineArch].Start = Start_AuthenticAMD;
		Arch[GenuineArch].Stop	= Stop_AuthenticAMD;
		Arch[GenuineArch].Timer = InitTimer_AuthenticAMD;
		Arch[GenuineArch].BaseClock = BaseClock_AuthenticAMD;

		Arch[GenuineArch].thermalFormula = THERMAL_FORMULA_AMD;

		Arch[GenuineArch].voltageFormula = VOLTAGE_FORMULA_AMD;

		Arch[GenuineArch].powerFormula = POWER_FORMULA_AMD;
		}
		break;
	}
	/*	Is an architecture identifier requested by user ?	*/
	if ( (ArchID != -1) && (ArchID >= 0) && (ArchID < ARCHITECTURES) )
	{
		PUBLIC(RO(Proc))->ArchID = ArchID;
	} else {
		PUBLIC(RO(Proc))->ArchID = SearchArchitectureID();
	}
	/*	Set the uArch's name with the first found codename	*/
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[0].CodeName,
		CODENAME_LEN);

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

	if ((PUBLIC(RO(Proc))->Features.Std.ECX.Hyperv == 1) && (ArchID == -1))
	{
		VendorFromCPUID(PUBLIC(RO(Proc))->Features.Info.Hypervisor.ID,
				&PUBLIC(RO(Proc))->Features.Info.LargestHypFunc,
				&PUBLIC(RO(Proc))->Features.Info.Hypervisor.CRC,
				&PUBLIC(RO(Proc))->HypervisorID,
				0x40000000LU, 0x0);

		switch (PUBLIC(RO(Proc))->HypervisorID) {
		case HYPERV_NONE:
		case HYPERV_KVM:
		case HYPERV_VBOX:
		case HYPERV_KBOX:
		case HYPERV_VMWARE:
		case HYPERV_HYPERV:
			PUBLIC(RO(Proc))->ArchID = GenuineArch;
			Arch[GenuineArch].Query = Query_VirtualMachine;
			Arch[GenuineArch].Update= PerCore_VirtualMachine;
			Arch[GenuineArch].Start = Start_VirtualMachine;
			Arch[GenuineArch].Stop	= Stop_VirtualMachine;
			Arch[GenuineArch].Timer = InitTimer_VirtualMachine;

			Arch[GenuineArch].thermalFormula = THERMAL_FORMULA_NONE;
			Arch[GenuineArch].voltageFormula = VOLTAGE_FORMULA_NONE;
			Arch[GenuineArch].powerFormula = POWER_FORMULA_NONE;
			break;
		case BARE_METAL:
		case HYPERV_XEN:
		/*	Xen virtualizes better the MSR & PCI registers	*/
			break;
		}
	}
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

	printk(KERN_INFO "CoreFreq(%u:%d):"	\
		" Processor [%2X%1X_%1X%1X]"	\
		" Architecture [%s] %3s [%u/%u]\n",
		PUBLIC(RO(Proc))->Service.Core,PUBLIC(RO(Proc))->Service.Thread,
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily,
		PUBLIC(RO(Proc))->Features.Std.EAX.Family,
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel,
		PUBLIC(RO(Proc))->Features.Std.EAX.Model,
		PUBLIC(RO(Proc))->Architecture,
		PUBLIC(RO(Proc))->Features.HTT_Enable ? "SMT" : "CPU",
		PUBLIC(RO(Proc))->CPU.OnLine,
		PUBLIC(RO(Proc))->CPU.Count);

	Controller_Start(0);

	PUBLIC(RO(Proc))->Registration.PCI = CoreFreqK_ProbePCI() == 0;

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

	CoreFreqK_Register_ClockSource(pArg->localProcessor);

	return (0);
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
		COREFREQ_RUN(Ignition_Level, Down)
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
		COREFREQ_RUN(Ignition_Level, Up)
	};
	INIT_ARG iArg = {
		.Features = NULL, .Brand = NULL,
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
	if (iArg.Brand != NULL)
	{
		kfree(iArg.Brand);
	}
	return (rc);
}

#undef COREFREQ_RUN

static int __init CoreFreqK_Init(void)
{
	int rc = CoreFreqK_StartUp();

	if (rc != 0) {
		CoreFreqK_ShutDown();
	}
	return (rc);
}

static void __exit CoreFreqK_Exit(void)
{
	CoreFreqK_ShutDown();
}

module_init(CoreFreqK_Init);
module_exit(CoreFreqK_Exit);

