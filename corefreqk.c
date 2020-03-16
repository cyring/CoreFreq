/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
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
#include <linux/utsname.h>
#include <linux/cpuidle.h>
#include <linux/cpufreq.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#include <linux/sched/signal.h>
#endif /* KERNEL_VERSION(4, 11, 0) */
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
MODULE_SUPPORTED_DEVICE ("Intel Core Core2 Atom Xeon i3 i5 i7, AMD [0Fh, 17h]");
MODULE_LICENSE ("GPL");
MODULE_VERSION (COREFREQ_VERSION);

static signed int ArchID = -1;
module_param(ArchID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ArchID, "Force an architecture (ID)");

static signed int AutoClock = 0b11;
module_param(AutoClock, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(AutoClock, "Auto estimate the clock frequency");

static unsigned int SleepInterval = 0;
module_param(SleepInterval, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SleepInterval, "Timer interval (ms)");

static unsigned int TickInterval = 0;
module_param(TickInterval, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(TickInterval, "System requested interval (ms)");

static signed int Experimental = 0;
module_param(Experimental, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Experimental, "Enable features under development");

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

static PROC *Proc = NULL;
static KPUBLIC *KPublic = NULL;
static KPRIVATE *KPrivate = NULL;
static ktime_t RearmTheTimer;


signed int SearchArchitectureID(void)
{
	signed int id;
    for (id = ARCHITECTURES - 1; id > 0; id--) {
	/* Search for an architecture signature. */
	if((Proc->Features.Std.EAX.ExtFamily == Arch[id].Signature.ExtFamily)
	&& (Proc->Features.Std.EAX.Family == Arch[id].Signature.Family)
	&& (((Proc->Features.Std.EAX.ExtModel ==  Arch[id].Signature.ExtModel)
		&& (Proc->Features.Std.EAX.Model ==  Arch[id].Signature.Model))
		|| (!Arch[id].Signature.ExtModel && !Arch[id].Signature.Model)))
	{
		break;
	}
    }
	return (id);
}

unsigned int Intel_Brand(char *pBrand)
{
	char idString[64] = {0x20};
	unsigned long ix = 0, jx = 0, px = 0;
	unsigned int frequency = 0, multiplier = 0;
	BRAND Brand;

	for (ix = 0; ix < 3; ix++) {
		__asm__ volatile
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
			: "r"   (0x80000002 + ix)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		for (jx = 0; jx < 4; jx++, px++) {
			idString[px     ] = Brand.AX.Chr[jx];
			idString[px +  4] = Brand.BX.Chr[jx];
			idString[px +  8] = Brand.CX.Chr[jx];
			idString[px + 12] = Brand.DX.Chr[jx];
		}
		px += 12;
	}
	for (ix = 0; ix < 46; ix++)
		if ((idString[ix+1] == 'H') && (idString[ix+2] == 'z')) {
			switch (idString[ix]) {
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
	if (multiplier > 0) {
	    if (idString[ix-3] == '.') {
		frequency  = (int) (idString[ix-4] - '0') * multiplier;
		frequency += (int) (idString[ix-2] - '0') * (multiplier / 10);
		frequency += (int) (idString[ix-1] - '0') * (multiplier / 100);
	    } else {
		frequency  = (int) (idString[ix-4] - '0') * 1000;
		frequency += (int) (idString[ix-3] - '0') * 100;
		frequency += (int) (idString[ix-2] - '0') * 10;
		frequency += (int) (idString[ix-1] - '0');
		frequency *= frequency;
	    }
	}
	for (jx = 0; jx < 48; jx++)
		if (idString[jx] != 0x20)
			break;
	for (ix = 0; jx < 48; jx++)
		if (!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++] = idString[jx];

	return (frequency);
}

void AMD_Brand(char *pBrand)
{
	char idString[64] = {0x20};
	unsigned long ix = 0, jx = 0, px = 0;
	BRAND Brand;

	for (ix = 0; ix < 3; ix++) {
		__asm__ volatile
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
			: "r"   (0x80000002 + ix)
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
		for (jx = 0; jx < 4; jx++, px++) {
			idString[px     ] = Brand.AX.Chr[jx];
			idString[px +  4] = Brand.BX.Chr[jx];
			idString[px +  8] = Brand.CX.Chr[jx];
			idString[px + 12] = Brand.DX.Chr[jx];
		}
		px += 12;
	}
	for (jx = 0; jx < 48; jx++)
		if (idString[jx] != 0x20)
			break;
	for (ix = 0; jx < 48; jx++)
		if (!(idString[jx] == 0x20 && idString[jx+1] == 0x20))
			pBrand[ix++] = idString[jx];
}

/* Retreive the Processor(BSP) features. */
static void Query_Features(void *pArg)
{
	INIT_ARG *iArg = (INIT_ARG *) pArg;

	unsigned int eax = 0x0, ebx = 0x0, ecx = 0x0, edx = 0x0; /*DWORD Only!*/

	/* Must have x86 CPUID 0x0, 0x1, and Intel CPUID 0x4 */
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
		: "=r" (iArg->Features->Info.LargestStdFunc),
		  "=r" (ebx),
		  "=r" (ecx),
		  "=r" (edx)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	iArg->Features->Info.Vendor.ID[ 0] = ebx;
	iArg->Features->Info.Vendor.ID[ 1] = (ebx >> 8);
	iArg->Features->Info.Vendor.ID[ 2] = (ebx >> 16);
	iArg->Features->Info.Vendor.ID[ 3] = (ebx >> 24);
	iArg->Features->Info.Vendor.ID[ 4] = edx;
	iArg->Features->Info.Vendor.ID[ 5] = (edx >> 8);
	iArg->Features->Info.Vendor.ID[ 6] = (edx >> 16);
	iArg->Features->Info.Vendor.ID[ 7] = (edx >> 24);
	iArg->Features->Info.Vendor.ID[ 8] = ecx;
	iArg->Features->Info.Vendor.ID[ 9] = (ecx >> 8);
	iArg->Features->Info.Vendor.ID[10] = (ecx >> 16);
	iArg->Features->Info.Vendor.ID[11] = (ecx >> 24);
	iArg->Features->Info.Vendor.ID[12] = '\0';

	if (!strncmp(iArg->Features->Info.Vendor.ID, VENDOR_INTEL, 12))
		iArg->Features->Info.Vendor.CRC = CRC_INTEL;
	else if (!strncmp(iArg->Features->Info.Vendor.ID, VENDOR_AMD, 12))
		iArg->Features->Info.Vendor.CRC = CRC_AMD;
	else if (!strncmp(iArg->Features->Info.Vendor.ID, VENDOR_HYGON, 12))
		iArg->Features->Info.Vendor.CRC = CRC_HYGON;
	else {
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
	if (iArg->Features->Info.LargestStdFunc >= 0x5) {
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
	}
	if (iArg->Features->Info.LargestStdFunc >= 0x6) {
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
	if (iArg->Features->Info.LargestStdFunc >= 0x7) {
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
	if (iArg->Features->Info.LargestExtFunc >= 0x80000007) {
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
	if (iArg->Features->Info.LargestExtFunc >= 0x80000008) {
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
	if (iArg->Features->Info.Vendor.CRC == CRC_INTEL) {
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

	    if (iArg->Features->Info.LargestStdFunc >= 0xa) {
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
	  iArg->Features->Factory.Freq=Intel_Brand(iArg->Features->Info.Brand);

	} else if ((iArg->Features->Info.Vendor.CRC == CRC_AMD)
		|| (iArg->Features->Info.Vendor.CRC == CRC_HYGON)) {
		/* Core Performance 64 bits General Counters. */
		iArg->Features->PerfMon.EAX.MonWidth = 64;
	    if (iArg->Features->ExtInfo.ECX.PerfCore) {
		iArg->Features->PerfMon.EAX.MonCtrs = 6;
	    } else {
		iArg->Features->PerfMon.EAX.MonCtrs = 4;
	    }
		/* Fixed Performance Counters. */
		iArg->Features->PerfMon.EDX.FixWidth = 64;
	    if ( iArg->Features->Power.ECX.HCF_Cap
	       | iArg->Features->AdvPower.EDX.EffFrqRO ) {
		iArg->Features->PerfMon.EBX.CoreCycles = 0;
		iArg->Features->PerfMon.EBX.RefCycles  = 0;
		iArg->Features->PerfMon.EDX.FixCtrs += 2;
	    }
	    if (iArg->Features->Info.LargestExtFunc >= 0x80000008) {
		iArg->SMT_Count = iArg->Features->leaf80000008.ECX.NC + 1;
		/* Add the Retired Instructions Perf Counter to the Fixed set */
		if (iArg->Features->leaf80000008.EBX.IRPerf) {
			iArg->Features->PerfMon.EBX.InstrRetired = 0;
			iArg->Features->PerfMon.EDX.FixCtrs++;
		}
	    } else if (iArg->Features->Std.EDX.HTT) {
		iArg->SMT_Count = iArg->Features->Std.EBX.Max_SMT_ID;
	    } else {
		iArg->SMT_Count = 1;
	    }
		AMD_Brand(iArg->Features->Info.Brand);
	}
}

void Compute_Interval(void)
{
	if ( (SleepInterval >= LOOP_MIN_MS)
	  && (SleepInterval <= LOOP_MAX_MS))
		Proc->SleepInterval = SleepInterval;
	else
		Proc->SleepInterval = LOOP_DEF_MS;
	/* Compute the tick steps. */
	Proc->tickReset = ((TickInterval >= Proc->SleepInterval)
			&& (TickInterval <= LOOP_MAX_MS)) ?
				TickInterval
			    :	KMAX(TICK_DEF_MS, Proc->SleepInterval);
	Proc->tickReset /= Proc->SleepInterval;
	Proc->tickStep = Proc->tickReset;

	RearmTheTimer = ktime_set(0, Proc->SleepInterval * 1000000LU);
}

#define BUSYWAIT 1000

static void ComputeWithSerializedTSC(COMPUTE_ARG *pCompute)
{
	unsigned int loop;
	/* Writeback and Invalidate Caches */
	WBINVD();
	/* Warm-up & Overhead */
	for (loop = 0; loop < OCCURRENCES; loop++) {
		RDTSCP64(pCompute->TSC[0][loop].V[0]);

		udelay(0);

		RDTSCP64(pCompute->TSC[0][loop].V[1]);
	}

	/* Estimation */
	for (loop=0; loop < OCCURRENCES; loop++) {
		RDTSCP64(pCompute->TSC[1][loop].V[0]);

		udelay(BUSYWAIT);

		RDTSCP64(pCompute->TSC[1][loop].V[1]);
	}
}

static void ComputeWithUnSerializedTSC(COMPUTE_ARG *pCompute)
{
	unsigned int loop;
	/* Writeback and Invalidate Caches */
	WBINVD();
	/* Warm-up & Overhead */
	for (loop = 0; loop < OCCURRENCES; loop++) {
		RDTSC64(pCompute->TSC[0][loop].V[0]);

		udelay(0);

		RDTSC64(pCompute->TSC[0][loop].V[1]);
	}
	/* Estimation */
	for (loop = 0; loop < OCCURRENCES; loop++) {
		RDTSC64(pCompute->TSC[1][loop].V[0]);

		udelay(BUSYWAIT);

		RDTSC64(pCompute->TSC[1][loop].V[1]);
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
	/* Is the TSC invariant or can serialize ? */
	if ((Proc->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (Proc->Features.ExtInfo.EDX.RDTSCP == 1))
		ComputeWithSerializedTSC(pCompute);
	else
		ComputeWithUnSerializedTSC(pCompute);

	/* Select the best clock. */
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
	/* Substract the overhead. */
	D[1][best[1]] -= D[0][best[0]];
	D[1][best[1]] *= BUSYWAIT;
	/* Compute the Base Clock */
	REL_BCLK(pCompute->Clock, ratio, D[1][best[1]], BUSYWAIT);
}

CLOCK Compute_Clock(unsigned int cpu, COMPUTE_ARG *pCompute)
{	/* Synchronous call the Base Clock estimation on a pinned CPU. */
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

	if (Proc->Features.Factory.Freq > 0) {
		clock.Hz = (Proc->Features.Factory.Freq * 1000000L) / ratio;
		clock.Q  = Proc->Features.Factory.Freq / ratio;
		clock.R  = (Proc->Features.Factory.Freq % ratio) * PRECISION;
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

	if (Proc->Features.Info.LargestStdFunc >= 0x16) {
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
{	/* AMD PPR Family 17h § 1.4/ Table 11: REFCLK = 100 MHz */
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
	return (clock);
};

void Define_CPUID(CORE *Core, const CPUID_STRUCT CpuIDforVendor[])
{	/* Per vendor, define a CPUID dump table to query. */
	int i;
	for (i = 0; i < CPUID_MAX_FUNC; i++) {
		Core->CpuID[i].func = CpuIDforVendor[i].func;
		Core->CpuID[i].sub  = CpuIDforVendor[i].sub;
	}
}

void Cache_Topology(CORE *Core)
{
	unsigned long level = 0x0;
	if (Proc->Features.Info.Vendor.CRC == CRC_INTEL) {
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
	else if ( (Proc->Features.Info.Vendor.CRC == CRC_AMD)
		||(Proc->Features.Info.Vendor.CRC == CRC_HYGON) )
	{
	    struct CACHE_INFO CacheInfo; /* Employ the Intel algorithm. */

	    if (Proc->Features.Info.LargestExtFunc >= 0x80000005)
	    {
		Core->T.Cache[0].Level = 1;
		Core->T.Cache[0].Type  = 2;		/* Inst. */
		Core->T.Cache[1].Level = 1;
		Core->T.Cache[1].Type  = 1;		/* Data */

		/* Fn8000_0005 L1 Data and Inst. caches. */
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
	    if (Proc->Features.Info.LargestExtFunc >= 0x80000006)
	    {
		Core->T.Cache[2].Level = 2;
		Core->T.Cache[2].Type  = 3;		/* Unified! */
		Core->T.Cache[3].Level = 3;
		Core->T.Cache[3].Type  = 3;

		/* Fn8000_0006 L2 and L3 caches. */
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
	CORE *Core = (CORE *) arg;

	struct CPUID_0x00000001_EBX leaf1_ebx = {0};

	CPUID_0x80000008 leaf80000008 = {{0}};

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

	switch (Proc->ArchID) {
	default:
	case AMD_Family_0Fh:	/* Legacy processor. */
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = 0;
		break;
	case AMD_Family_10h:
	case AMD_Family_11h:
	case AMD_Family_12h:
	case AMD_Family_14h:
	case AMD_Family_16h:
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = leaf1_ebx.Init_APIC_ID
				  >> leaf80000008.ECX.ApicIdCoreIdSize;
		break;
	case AMD_Family_15h:
	    if ((Proc->Features.Std.EAX.ExtModel == 0x0)
	     && (Proc->Features.Std.EAX.Model >= 0x0)
	     && (Proc->Features.Std.EAX.Model <= 0xf))
	    {
		L3_CACHE_PARAMETER L3;
		RDPCI(L3, PCI_AMD_L3_CACHE_PARAMETER);

	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize0);
	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize1);
	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize2);
	    Core->T.Cache[3].Size+=L3_SubCache_AMD_Piledriver(L3.SubCacheSize3);
	    }
		Core->T.ApicID    = leaf1_ebx.Init_APIC_ID;
		Core->T.PackageID = leaf1_ebx.Init_APIC_ID
				  >> leaf80000008.ECX.ApicIdCoreIdSize;
		Core->T.CoreID    = leaf1_ebx.Init_APIC_ID
				  - (Core->T.PackageID
					<< leaf80000008.ECX.ApicIdCoreIdSize);

/*TODO: Map the Bulldozer extended topology
	    if (Proc->Features.ExtInfo.ECX.TopoExt == 1)
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
		leaf8000001e.EAX.ExtApicId;
		leaf8000001e.EBX.CompUnitId;
		leaf8000001e.ECX.NodeId;
	    }
*/
	    break;
	case AMD_Family_17h:
	case AMD_Family_18h:
	    if (Proc->Features.ExtInfo.ECX.TopoExt == 1)
	    {
		struct CACHE_INFO CacheInfo = {{{0}}};
		CPUID_0x8000001e leaf8000001e = {{0}};

		/* Fn8000_001D Cache Properties. */
		unsigned long idx, level[CACHE_MAX_LEVEL] = {1, 0, 2, 3};
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
		/* Fn8000_001E {ExtApic, Core, Node} Identifiers. */
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

		if (leaf8000001e.EBX.ThreadsPerCore > 0)
			Core->T.ThreadID  = leaf8000001e.EAX.ExtApicId & 1;
		else
			Core->T.ThreadID  = 0;
	    } else {	/* Fallback algorithm. */
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
    if (arg != NULL) {
	CORE *Core = (CORE *) arg;
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

	if (Proc->Features.Std.EDX.HTT) {
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
	CORE *Core = (CORE *) arg;

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
		/* Exit from the loop if the BX register equals 0 or
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
		( (Proc->Features.Info.Vendor.CRC == CRC_AMD)
		||(Proc->Features.Info.Vendor.CRC == CRC_HYGON) ) ?
			Map_AMD_Topology
		: (Proc->Features.Info.LargestStdFunc >= 0xb) ?
			Map_Intel_Extended_Topology : Map_Intel_Topology,
		KPublic->Core[cpu], 1); /* Synchronous call. */

	if (	!rc
		&& (Proc->Features.HTT_Enable == 0)
		&& (KPublic->Core[cpu]->T.ThreadID > 0) ) {
			Proc->Features.HTT_Enable = 1;
	}
	return (rc);
}

unsigned int Proc_Topology(void)
{
	unsigned int cpu, CountEnabledCPU = 0;

    for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
	KPublic->Core[cpu]->T.Base.value = 0;
	KPublic->Core[cpu]->T.ApicID     = -1;
	KPublic->Core[cpu]->T.CoreID     = -1;
	KPublic->Core[cpu]->T.ThreadID   = -1;
	KPublic->Core[cpu]->T.PackageID  = -1;

	BITSET(LOCKLESS, KPublic->Core[cpu]->OffLine, HW);
	BITSET(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);

	if (cpu_present(cpu)) { /* CPU state based on the OS. */
	    if (Core_Topology(cpu) == 0) {
		/* CPU state based on the hardware. */
		if (KPublic->Core[cpu]->T.ApicID >= 0) {
			BITCLR(LOCKLESS, KPublic->Core[cpu]->OffLine, HW);

			CountEnabledCPU++;
		}
		BITCLR(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);
	    }
	}
    }
	return (CountEnabledCPU);
}

#define HyperThreading_Technology()					\
(									\
	Proc->CPU.OnLine = Proc_Topology()				\
)

void Package_Reset(void)
{
	Proc->Features.TDP_Unlock = 0;
	Proc->Features.Turbo_Unlock = 0;
	Proc->Features.TurboActiv_Lock = 1;
	Proc->Features.TDP_Cfg_Lock = 1;
}

void OverrideCodeNameString(PROCESSOR_SPECIFIC *pSpecific)
{
    StrCopy(Proc->Architecture,
	    Arch[Proc->ArchID].Architecture[pSpecific->CodeNameIdx].CodeName,
	    CODENAME_LEN);
}

void OverrideUnlockCapability(PROCESSOR_SPECIFIC *pSpecific)
{
	if (pSpecific->Latch & LATCH_TGT_RATIO_UNLOCK)
		Proc->Features.TgtRatio_Unlock = pSpecific->TgtRatioUnlocked;

	if (pSpecific->Latch & LATCH_CLK_RATIO_UNLOCK)
		Proc->Features.ClkRatio_Unlock = pSpecific->ClkRatioUnlocked;

	if (pSpecific->Latch & LATCH_TURBO_UNLOCK)
		Proc->Features.Turbo_Unlock = pSpecific->TurboUnlocked;

	if (pSpecific->Latch & LATCH_UNCORE_UNLOCK)
		Proc->Features.Uncore_Unlock= pSpecific->UncoreUnlocked;
}

PROCESSOR_SPECIFIC *LookupProcessor(void)
{
	PROCESSOR_SPECIFIC *pSpecific = Arch[Proc->ArchID].Specific;

	while (pSpecific->BrandSubStr != NULL) {
		if (strstr(Proc->Features.Info.Brand, pSpecific->BrandSubStr))
		{
			break;
		}
		pSpecific++;
	}
	return (pSpecific);
}

int Intel_MaxBusRatio(PLATFORM_ID *PfID)
{
	struct SIGNATURE whiteList[] = {
		_Core_Conroe,		/* 06_0F */
		_Core_Penryn,		/* 06_17 */
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft,		/* 06_27 */
		_Atom_Clovertrail,	/* 06_35 */
		_Atom_Saltwell,		/* 06_36 */
		_Silvermont_637,	/* 06_37 */
	};
	int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
	for (id = 0; id < ids; id++) {
		if((whiteList[id].ExtFamily == Proc->Features.Std.EAX.ExtFamily)
		 && (whiteList[id].Family == Proc->Features.Std.EAX.Family)
		 && (whiteList[id].ExtModel == Proc->Features.Std.EAX.ExtModel)
		 && (whiteList[id].Model == Proc->Features.Std.EAX.Model))
		{
			RDMSR((*PfID), MSR_IA32_PLATFORM_ID);
			return (0);
		}
	}
	return (-1);
}

void Intel_Core_Platform_Info(void)
{
	PROCESSOR_SPECIFIC *pSpecific = NULL;
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
					ratio1 = PfID.MaxBusRatio;
			}
		}
	} else {
			if (Intel_MaxBusRatio(&PfID) == 0) {
				if (PfID.value != 0)
					ratio1 = PfID.MaxBusRatio;
			}
	}
	Proc->Boost[BOOST(MIN)] = KMIN(ratio0, ratio1);
	Proc->Boost[BOOST(MAX)] = KMAX(ratio0, ratio1);

	Proc->Features.TgtRatio_Unlock = 1;
	if ((pSpecific = LookupProcessor()) != NULL) {
		OverrideCodeNameString(pSpecific);
		OverrideUnlockCapability(pSpecific);

		Proc->Boost[BOOST(1C)]  = Proc->Boost[BOOST(MAX)]
					+ pSpecific->Boost[0]
					+ pSpecific->Boost[1];
	}
	else if (Proc->Features.Power.EAX.TurboIDA) { /* Half Ratio capable ? */
		Proc->Boost[BOOST(1C)] = Proc->Boost[BOOST(MAX)] + 2;
	}
	if (Proc->Features.Turbo_Unlock) {
		Proc->Features.SpecTurboRatio = 1;
	} else
		Proc->Features.SpecTurboRatio = 0;
}

void Intel_Platform_Turbo(void)
{
	PROCESSOR_SPECIFIC *pSpecific = NULL;
	PLATFORM_INFO Platform = {.value = 0};
	RDMSR(Platform, MSR_PLATFORM_INFO);

	Proc->Features.TDP_Unlock = Platform.ProgrammableTDP;
	Proc->Features.TDP_Levels = Platform.ConfigTDPlevels;
	Proc->Features.Turbo_Unlock = Platform.ProgrammableTurbo;

	Proc->Boost[BOOST(MIN)] = Platform.MinimumRatio;
	Proc->Boost[BOOST(MAX)] = Platform.MaxNonTurboRatio;

	Proc->Features.TgtRatio_Unlock = 1;
	if ((pSpecific = LookupProcessor()) != NULL) {
		OverrideCodeNameString(pSpecific);
		OverrideUnlockCapability(pSpecific);
	}

	Proc->Features.SpecTurboRatio = 0;
}

long Intel_Turbo_Config8C(CLOCK_ARG *pClockMod)
{
	long rc = 0;
	TURBO_RATIO_CONFIG0 TurboCfg0 = {.value = 0};
	RDMSR(TurboCfg0, MSR_TURBO_RATIO_LIMIT);

	if (pClockMod != NULL) {
		if (Proc->Features.Turbo_Unlock) {
			unsigned short WrRd8C = 0;
			switch (pClockMod->NC) {
			case 1:
				TurboCfg0.MaxRatio_1C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 2:
				TurboCfg0.MaxRatio_2C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 3:
				TurboCfg0.MaxRatio_3C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 4:
				TurboCfg0.MaxRatio_4C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 5:
				TurboCfg0.MaxRatio_5C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 6:
				TurboCfg0.MaxRatio_6C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 7:
				TurboCfg0.MaxRatio_7C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			case 8:
				TurboCfg0.MaxRatio_8C += pClockMod->Offset;
				WrRd8C = 1;
				break;
			}
			if (WrRd8C) {
				WRMSR(TurboCfg0, MSR_TURBO_RATIO_LIMIT);
				RDMSR(TurboCfg0, MSR_TURBO_RATIO_LIMIT);
				rc = 2;
			}
		}
	} else
		Proc->Features.SpecTurboRatio += 8;

	Proc->Boost[BOOST(8C)] = TurboCfg0.MaxRatio_8C;
	Proc->Boost[BOOST(7C)] = TurboCfg0.MaxRatio_7C;
	Proc->Boost[BOOST(6C)] = TurboCfg0.MaxRatio_6C;
	Proc->Boost[BOOST(5C)] = TurboCfg0.MaxRatio_5C;
	Proc->Boost[BOOST(4C)] = TurboCfg0.MaxRatio_4C;
	Proc->Boost[BOOST(3C)] = TurboCfg0.MaxRatio_3C;
	Proc->Boost[BOOST(2C)] = TurboCfg0.MaxRatio_2C;
	Proc->Boost[BOOST(1C)] = TurboCfg0.MaxRatio_1C;

	return (rc);
}

long Intel_Turbo_Config15C(CLOCK_ARG *pClockMod)
{
	long rc = 0;
	TURBO_RATIO_CONFIG1 TurboCfg1 = {.value = 0};
	RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);

	if (pClockMod != NULL) {
	    if (Proc->Features.Turbo_Unlock) {
		unsigned short WrRd15C = 0;
		switch (pClockMod->NC) {
		case 9:
			TurboCfg1.IVB_EP.MaxRatio_9C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		case 10:
			TurboCfg1.IVB_EP.MaxRatio_10C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		case 11:
			TurboCfg1.IVB_EP.MaxRatio_11C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		case 12:
			TurboCfg1.IVB_EP.MaxRatio_12C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		case 13:
			TurboCfg1.IVB_EP.MaxRatio_13C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		case 14:
			TurboCfg1.IVB_EP.MaxRatio_14C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		case 15:
			TurboCfg1.IVB_EP.MaxRatio_15C += pClockMod->Offset;
			WrRd15C = 1;
			break;
		}
		if (WrRd15C) {
			WRMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);
			RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);
			rc = 2;
		}
	    }
	} else
		Proc->Features.SpecTurboRatio += 7;

	Proc->Boost[BOOST(15C)] = TurboCfg1.IVB_EP.MaxRatio_15C;
	Proc->Boost[BOOST(14C)] = TurboCfg1.IVB_EP.MaxRatio_14C;
	Proc->Boost[BOOST(13C)] = TurboCfg1.IVB_EP.MaxRatio_13C;
	Proc->Boost[BOOST(12C)] = TurboCfg1.IVB_EP.MaxRatio_12C;
	Proc->Boost[BOOST(11C)] = TurboCfg1.IVB_EP.MaxRatio_11C;
	Proc->Boost[BOOST(10C)] = TurboCfg1.IVB_EP.MaxRatio_10C;
	Proc->Boost[BOOST(9C) ] = TurboCfg1.IVB_EP.MaxRatio_9C;

	return (rc);
}

long Intel_Turbo_Config16C(CLOCK_ARG *pClockMod)
{
	long rc = 0;
	TURBO_RATIO_CONFIG1 TurboCfg1 = {.value = 0};
	RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);

	if (pClockMod != NULL) {
	    if (Proc->Features.Turbo_Unlock) {
		unsigned short WrRd16C = 0;
		switch (pClockMod->NC) {
		case 9:
			TurboCfg1.HSW_EP.MaxRatio_9C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 10:
			TurboCfg1.HSW_EP.MaxRatio_10C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 11:
			TurboCfg1.HSW_EP.MaxRatio_11C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 12:
			TurboCfg1.HSW_EP.MaxRatio_12C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 13:
			TurboCfg1.HSW_EP.MaxRatio_13C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 14:
			TurboCfg1.HSW_EP.MaxRatio_14C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 15:
			TurboCfg1.HSW_EP.MaxRatio_15C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 16:
			TurboCfg1.HSW_EP.MaxRatio_16C += pClockMod->Offset;
			WrRd16C = 1;
			break;
		}
		if (WrRd16C) {
			WRMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);
			RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);
			rc = 2;
		}
	    }
	} else
		Proc->Features.SpecTurboRatio += 8;

	Proc->Boost[BOOST(16C)] = TurboCfg1.HSW_EP.MaxRatio_16C;
	Proc->Boost[BOOST(15C)] = TurboCfg1.HSW_EP.MaxRatio_15C;
	Proc->Boost[BOOST(14C)] = TurboCfg1.HSW_EP.MaxRatio_14C;
	Proc->Boost[BOOST(13C)] = TurboCfg1.HSW_EP.MaxRatio_13C;
	Proc->Boost[BOOST(12C)] = TurboCfg1.HSW_EP.MaxRatio_12C;
	Proc->Boost[BOOST(11C)] = TurboCfg1.HSW_EP.MaxRatio_11C;
	Proc->Boost[BOOST(10C)] = TurboCfg1.HSW_EP.MaxRatio_10C;
	Proc->Boost[BOOST(9C) ] = TurboCfg1.HSW_EP.MaxRatio_9C;

	return (rc);
}

long Intel_Turbo_Config18C(CLOCK_ARG *pClockMod)
{
	long rc = 0;
	TURBO_RATIO_CONFIG2 TurboCfg2 = {.value = 0};
	RDMSR(TurboCfg2, MSR_TURBO_RATIO_LIMIT2);

	if (pClockMod != NULL) {
		if (Proc->Features.Turbo_Unlock) {
			unsigned short WrRd18C = 0;
			switch (pClockMod->NC) {
			case 17:
				TurboCfg2.MaxRatio_17C += pClockMod->Offset;
				WrRd18C = 1;
				break;
			case 18:
				TurboCfg2.MaxRatio_18C += pClockMod->Offset;
				WrRd18C = 1;
				break;
			}
			if (WrRd18C) {
				WRMSR(TurboCfg2, MSR_TURBO_RATIO_LIMIT2);
				RDMSR(TurboCfg2, MSR_TURBO_RATIO_LIMIT2);
				rc = 2;
			}
		}
	} else
		Proc->Features.SpecTurboRatio += 2;

	Proc->Boost[BOOST(18C)] = TurboCfg2.MaxRatio_18C;
	Proc->Boost[BOOST(17C)] = TurboCfg2.MaxRatio_17C;

	return (rc);
}

long Skylake_X_Turbo_Config16C(CLOCK_ARG *pClockMod)
{
	long rc = 0;
	TURBO_RATIO_CONFIG1 TurboCfg1 = {.value = 0};
	RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);

	if (pClockMod != NULL) {
	    if (Proc->Features.Turbo_Unlock) {
		unsigned short WrRd16C = 0;
		switch (pClockMod->NC) {
		case 9:
			TurboCfg1.SKL_X.NUMCORE_0 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 10:
			TurboCfg1.SKL_X.NUMCORE_1 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 11:
			TurboCfg1.SKL_X.NUMCORE_2 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 12:
			TurboCfg1.SKL_X.NUMCORE_3 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 13:
			TurboCfg1.SKL_X.NUMCORE_4 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 14:
			TurboCfg1.SKL_X.NUMCORE_5 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 15:
			TurboCfg1.SKL_X.NUMCORE_6 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		case 16:
			TurboCfg1.SKL_X.NUMCORE_7 += pClockMod->Offset;
			WrRd16C = 1;
			break;
		}
		if (WrRd16C) {
			WRMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);
			RDMSR(TurboCfg1, MSR_TURBO_RATIO_LIMIT1);
			rc = 2;
		}
	    }
	} else
		Proc->Features.SpecTurboRatio += 8;

	Proc->Boost[BOOST(16C)] = TurboCfg1.SKL_X.NUMCORE_7;
	Proc->Boost[BOOST(15C)] = TurboCfg1.SKL_X.NUMCORE_6;
	Proc->Boost[BOOST(14C)] = TurboCfg1.SKL_X.NUMCORE_5;
	Proc->Boost[BOOST(13C)] = TurboCfg1.SKL_X.NUMCORE_4;
	Proc->Boost[BOOST(12C)] = TurboCfg1.SKL_X.NUMCORE_3;
	Proc->Boost[BOOST(11C)] = TurboCfg1.SKL_X.NUMCORE_2;
	Proc->Boost[BOOST(10C)] = TurboCfg1.SKL_X.NUMCORE_1;
	Proc->Boost[BOOST(9C) ] = TurboCfg1.SKL_X.NUMCORE_0;

	return (rc);
}

void Intel_Turbo_TDP_Config(void)
{
	TURBO_ACTIVATION TurboActivation = {.value = 0};
	CONFIG_TDP_NOMINAL NominalTDP = {.value = 0};
	CONFIG_TDP_CONTROL ControlTDP = {.value = 0};
	CONFIG_TDP_LEVEL ConfigTDP;

	RDMSR(TurboActivation, MSR_TURBO_ACTIVATION_RATIO);
	Proc->Boost[BOOST(ACT)] = TurboActivation.MaxRatio;
	Proc->Features.TurboActiv_Lock = TurboActivation.Ratio_Lock;

	RDMSR(NominalTDP, MSR_CONFIG_TDP_NOMINAL);
	Proc->Boost[BOOST(TDP)] = NominalTDP.Ratio;

	ConfigTDP.value = 0;
	RDMSR(ConfigTDP, MSR_CONFIG_TDP_LEVEL_2);
	Proc->Boost[BOOST(TDP2)] = ConfigTDP.Ratio;

	ConfigTDP.value = 0;
	RDMSR(ConfigTDP, MSR_CONFIG_TDP_LEVEL_1);
	Proc->Boost[BOOST(TDP1)] = ConfigTDP.Ratio;

	RDMSR(ControlTDP, MSR_CONFIG_TDP_CONTROL);
	Proc->Features.TDP_Cfg_Lock  = ControlTDP.Lock;
	Proc->Features.TDP_Cfg_Level = ControlTDP.Level;
}

static void PerCore_Intel_HWP_Notification(void *arg)
{
	CORE *Core = (CORE *) arg;

    if ((arg != NULL) && Proc->Features.Power.EAX.HWP_Int) {
	BITSET_CC(LOCKLESS, Proc->HWP_Mask, Core->Bind);
	/* HWP Notifications are fully disabled.			*/
	Core->PowerThermal.HWP_Interrupt.value = 0;
	WRMSR(Core->PowerThermal.HWP_Interrupt, MSR_HWP_INTERRUPT);

	RDMSR(Core->PowerThermal.HWP_Interrupt, MSR_HWP_INTERRUPT);
	if (Core->PowerThermal.HWP_Interrupt.value == 0) {
		BITSET_CC(LOCKLESS, Proc->HWP, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, Proc->HWP, Core->Bind);
	}
    }
}

static void PerCore_Intel_HWP_Ignition(void *arg)
{
    if ((arg != NULL) && Proc->Features.Power.EAX.HWP_EPP) {
	CORE *Core = (CORE *) arg;

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
    if (Proc->Features.Info.Vendor.CRC == CRC_INTEL) {
	PM_ENABLE PM_Enable = {.value = 0};
	HDC_CONTROL HDC_Control = {.value = 0};

	if (Proc->Features.Power.EAX.HWP_Reg)
	{
		/* MSR_IA32_PM_ENABLE is a Package register.		*/
		RDMSR(PM_Enable, MSR_IA32_PM_ENABLE);
		/* Is the HWP requested and its current state is off ?	*/
	  if ((HWP_Enable == 1) && (PM_Enable.HWP_Enable == 0))
	  {
		unsigned int cpu;
		/*	From last AP to BSP				*/
			cpu = Proc->CPU.Count;
		do {
			cpu--;

		    if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)) {
			/* Synchronous call.				*/
			smp_call_function_single(cpu,
						PerCore_Intel_HWP_Notification,
						&KPublic->Core[cpu], 1);
		    }
		  } while (cpu != 0) ;

	    if (BITCMP_CC(Proc->CPU.Count,LOCKLESS, Proc->HWP, Proc->HWP_Mask))
	    {
		/* Enable the Hardware-controlled Performance States.	*/
		PM_Enable.HWP_Enable = 1;
		WRMSR(PM_Enable, MSR_IA32_PM_ENABLE);
		RDMSR(PM_Enable, MSR_IA32_PM_ENABLE);

		if (PM_Enable.HWP_Enable)
		{
			cpu = Proc->CPU.Count;
		do {
			cpu--;

		    if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)) {
			/* Asynchronous call.				*/
			smp_call_function_single(cpu,
						PerCore_Intel_HWP_Ignition,
						&KPublic->Core[cpu], 0);
		    }
		  } while (cpu != 0) ;
		}
	    }
	  }
	}
	Proc->Features.HWP_Enable = PM_Enable.HWP_Enable;
	/*	Hardware Duty Cycling					*/
	if (Proc->Features.Power.EAX.HDC_Reg) {
		RDMSR(HDC_Control, MSR_IA32_PKG_HDC_CTL);
	}
	Proc->Features.HDC_Enable = HDC_Control.HDC_Enable;
    }
}

void SandyBridge_Uncore_Ratio(void)
{
	Proc->Uncore.Boost[UNCORE_BOOST(MIN)] = Proc->Boost[BOOST(MIN)];
	Proc->Uncore.Boost[UNCORE_BOOST(MAX)] = Proc->Boost[BOOST(MAX)];
}

long Haswell_Uncore_Ratio(CLOCK_ARG *pClockMod)
{
	long rc = 0;
	UNCORE_RATIO_LIMIT UncoreRatio = {.value = 0};
	RDMSR(UncoreRatio, MSR_HSW_UNCORE_RATIO_LIMIT);

	if (pClockMod != NULL) {
		unsigned short WrRdMSR = 0;
		switch (pClockMod->NC) {
		case CLOCK_MOD_MAX:
			UncoreRatio.MaxRatio += pClockMod->Offset;
			WrRdMSR = 1;
			break;
		case CLOCK_MOD_MIN:
			UncoreRatio.MinRatio += pClockMod->Offset;
			WrRdMSR = 1;
			break;
		}
		if (WrRdMSR) {
			WRMSR(UncoreRatio, MSR_HSW_UNCORE_RATIO_LIMIT);
			RDMSR(UncoreRatio, MSR_HSW_UNCORE_RATIO_LIMIT);
			rc = 2;
		}
	}

	Proc->Uncore.Boost[UNCORE_BOOST(MIN)] = UncoreRatio.MinRatio;
	Proc->Uncore.Boost[UNCORE_BOOST(MAX)] = UncoreRatio.MaxRatio;

	return (rc);
}

void SandyBridge_PowerInterface(void)
{
	RDMSR(Proc->PowerThermal.Unit, MSR_RAPL_POWER_UNIT);
	RDMSR(Proc->PowerThermal.PowerInfo, MSR_PKG_POWER_INFO);
}

void Nehalem_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C(NULL);
}

void IvyBridge_EP_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C(NULL);
	Intel_Turbo_Config15C(NULL);
}

void Haswell_EP_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C(NULL);
	Intel_Turbo_Config16C(NULL);
	Intel_Turbo_Config18C(NULL);
}

void Skylake_X_Platform_Info(void)
{
	Intel_Platform_Turbo();
	Intel_Turbo_Config8C(NULL);
	Skylake_X_Turbo_Config16C(NULL);
}

long TurboClock_IvyBridge_EP(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod)
		| Intel_Turbo_Config15C(pClockMod);
	return (rc);
}

long TurboClock_Haswell_EP(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod)
		| Intel_Turbo_Config16C(pClockMod)
		| Intel_Turbo_Config18C(pClockMod);
	return (rc);
}

long TurboClock_Skylake_X(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod)
		| Skylake_X_Turbo_Config16C(pClockMod);
	return (rc);
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

	Proc->Uncore.ChipID = dev->device;

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
{	/* Source: Mobile Intel 945 Express Chipset Family. */
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P945.DCC.value = readl(mchmap + 0x200);

	switch (Proc->Uncore.MC[0].P945.DCC.DAMC) {
	case 0b00:
	case 0b11:
		Proc->Uncore.MC[0].ChannelCount = 1;
		break;
	case 0b01:
	case 0b10:
		Proc->Uncore.MC[0].ChannelCount = 2;
		break;
	}

	Proc->Uncore.MC[0].SlotCount = 1;

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++)
	{
		unsigned short rank, rankCount;

		Proc->Uncore.MC[0].Channel[cha].P945.DRT0.value =
					readl(mchmap + 0x110 + 0x80 * cha);

		Proc->Uncore.MC[0].Channel[cha].P945.DRT1.value =
					readw(mchmap + 0x114 + 0x80 * cha);

		Proc->Uncore.MC[0].Channel[cha].P945.DRT2.value =
					readl(mchmap + 0x118 + 0x80 * cha);

		Proc->Uncore.MC[0].Channel[cha].P945.BANK.value =
					readw(mchmap + 0x10e + 0x80 * cha);

		Proc->Uncore.MC[0].Channel[cha].P945.WIDTH.value =
					readw(mchmap + 0x40c + 0x80 * cha);
	    if (cha == 0) {
		Proc->Uncore.MC[0].Channel[cha].P945.WIDTH.value &= 0b11111111;
		rankCount = 4;
	    } else {
		Proc->Uncore.MC[0].Channel[cha].P945.WIDTH.value &= 0b1111;
		rankCount = 2;
	    }
	    for (rank = 0; rank < rankCount; rank++)
		Proc->Uncore.MC[0].Channel[cha].P945.DRB[rank].value =
				readb(mchmap + 0x100 + rank + 0x80 * cha);
	}
}

void Query_P955(void __iomem *mchmap)
{	/* Source: Intel 82955X Memory Controller Hub (MCH) */
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P955.DCC.value = readl(mchmap + 0x200);

	switch (Proc->Uncore.MC[0].P955.DCC.DAMC) {
	case 0b00:
	case 0b11:
		Proc->Uncore.MC[0].ChannelCount = 1;
		break;
	case 0b01:
	case 0b10:
		Proc->Uncore.MC[0].ChannelCount = 2;
		break;
	}

	Proc->Uncore.MC[0].SlotCount = 1;

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++)
	{
		unsigned short rank;

		Proc->Uncore.MC[0].Channel[cha].P955.DRT1.value =
					readw(mchmap + 0x114 + 0x80 * cha);

		Proc->Uncore.MC[0].Channel[cha].P955.BANK.value =
					readw(mchmap + 0x10e + 0x80 * cha);

		Proc->Uncore.MC[0].Channel[cha].P955.WIDTH.value =
					readw(mchmap + 0x40c + 0x80 * cha);

	    for (rank = 0; rank < 4; rank++)
		Proc->Uncore.MC[0].Channel[cha].P955.DRB[rank].value =
				readb(mchmap + 0x100 + rank + 0x80 * cha);
	}
}

void Query_P965(void __iomem *mchmap)
{
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P965.CKE0.value = readl(mchmap + 0x260);
	Proc->Uncore.MC[0].P965.CKE1.value = readl(mchmap + 0x660);

	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].P965.CKE0.RankPop0 != 0)
				+ (Proc->Uncore.MC[0].P965.CKE1.RankPop0 != 0);

	Proc->Uncore.MC[0].SlotCount =
			  (Proc->Uncore.MC[0].P965.CKE0.SingleDimmPop ? 1 : 2)
			+ (Proc->Uncore.MC[0].P965.CKE1.SingleDimmPop ? 1 : 2);

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].P965.DRT0.value =
					readl(mchmap + 0x29c + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT1.value =
					readw(mchmap + 0x250 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT2.value =
					readl(mchmap + 0x252 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT3.value =
					readw(mchmap + 0x256 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P965.DRT4.value =
					readl(mchmap + 0x258 + 0x400 * cha);
	}
}

void Query_G965(void __iomem *mchmap)
{	/* Source: Mobile Intel 965 Express Chipset Family. */
	unsigned short cha, slot;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].G965.DRB0.value = readl(mchmap + 0x1200);
	Proc->Uncore.MC[0].G965.DRB1.value = readl(mchmap + 0x1300);

	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].G965.DRB0.Rank1Addr != 0)
				+ (Proc->Uncore.MC[0].G965.DRB1.Rank1Addr != 0);

	Proc->Uncore.MC[0].SlotCount=Proc->Uncore.MC[0].ChannelCount > 1 ? 1:2;

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].G965.DRT0.value =
					readl(mchmap + 0x1210 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT1.value =
					readl(mchmap + 0x1214 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT2.value =
					readl(mchmap + 0x1218 + 0x100 * cha);

		Proc->Uncore.MC[0].Channel[cha].G965.DRT3.value =
					readl(mchmap + 0x121c + 0x100 * cha);

		for (slot = 0; slot < Proc->Uncore.MC[0].SlotCount; slot++) {
			Proc->Uncore.MC[0].Channel[cha].DIMM[slot].DRA.value =
					readl(mchmap + 0x1208 + 0x100 * cha);
		}
	}
}

void Query_P35(void __iomem *mchmap)
{	/* Source: Intel® 3 Series Express Chipset Family. */
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	Proc->Uncore.MC[0].P35.CKE0.value = readl(mchmap + 0x260);
	Proc->Uncore.MC[0].P35.CKE1.value = readl(mchmap + 0x660);

	Proc->Uncore.MC[0].ChannelCount =
				  (Proc->Uncore.MC[0].P35.CKE0.RankPop0 != 0)
				+ (Proc->Uncore.MC[0].P35.CKE1.RankPop0 != 0);

	Proc->Uncore.MC[0].SlotCount =
			  (Proc->Uncore.MC[0].P35.CKE0.SingleDimmPop ? 1 : 2)
			+ (Proc->Uncore.MC[0].P35.CKE1.SingleDimmPop ? 1 : 2);

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].P35.DRT0.value =
					readw(mchmap + 0x265 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT1.value =
					readw(mchmap + 0x250 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT2.value =
					readl(mchmap + 0x252 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT3.value =
					readl(mchmap + 0x256 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT4.value =
					readl(mchmap + 0x258 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].P35.DRT5.value =
					readw(mchmap + 0x25d + 0x400 * cha);
	}
}

kernel_ulong_t Query_NHM_Timing(unsigned int did,
				unsigned short mc,
				unsigned short cha)
{	/*Source: Micron Technical Note DDR3 Power-Up, Initialization, & Reset*/
    struct pci_dev *dev = pci_get_device(PCI_VENDOR_ID_INTEL, did, NULL);
    if (dev != NULL) {
	pci_read_config_dword(dev, 0x70,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.MR0_1.value);

	pci_read_config_dword(dev, 0x74,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.MR2_3.value);

	pci_read_config_dword(dev ,0x80,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_A.value);

	pci_read_config_dword(dev ,0x84,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.Rank_B.value);

	pci_read_config_dword(dev ,0x88,
			      &Proc->Uncore.MC[mc].Channel[cha].NHM.Bank.value);

	pci_read_config_dword(dev ,0x8c,
			   &Proc->Uncore.MC[mc].Channel[cha].NHM.Refresh.value);

	pci_read_config_dword(dev, 0xb8,
			    &Proc->Uncore.MC[mc].Channel[cha].NHM.Params.value);

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
	if (dev != NULL) {
		unsigned short slot;

		for (slot = 0; slot < Proc->Uncore.MC[mc].SlotCount; slot++) {
		    pci_read_config_dword(dev, 0x48 + 4 * slot,
			&Proc->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.value);
		}
		pci_dev_put(dev);
		return (0);
	} else
		return (-ENODEV);
}

void Query_NHM_MaxDIMMs(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(	dev, 0x64,
				&Proc->Uncore.MC[mc].MaxDIMMs.NHM.DOD.value);

	switch (Proc->Uncore.MC[mc].MaxDIMMs.NHM.DOD.MAXNUMDIMMS) {
	case 0b00:
		Proc->Uncore.MC[mc].SlotCount = 1;
		break;
	case 0b01:
		Proc->Uncore.MC[mc].SlotCount = 2;
		break;
	case 0b10:
		Proc->Uncore.MC[mc].SlotCount = 3;
		break;
	default:
		Proc->Uncore.MC[mc].SlotCount = 0;
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

	pci_read_config_dword(dev,0x48, &Proc->Uncore.MC[mc].NHM.CONTROL.value);
	pci_read_config_dword(dev,0x4c, &Proc->Uncore.MC[mc].NHM.STATUS.value);

	Proc->Uncore.MC[mc].ChannelCount =
		  (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL0_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL2_ACTIVE != 0);

	for (cha = 0; (cha < Proc->Uncore.MC[mc].ChannelCount) && !rc; cha++) {
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

	pci_read_config_dword(dev,0x48, &Proc->Uncore.MC[mc].NHM.CONTROL.value);
	pci_read_config_dword(dev,0x4c, &Proc->Uncore.MC[mc].NHM.STATUS.value);

	Proc->Uncore.MC[mc].ChannelCount =
		  (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL0_ACTIVE != 0)
		+ (Proc->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0);

	for (cha = 0; (cha < Proc->Uncore.MC[mc].ChannelCount) && !rc; cha++) {
		rc = Query_NHM_Timing(did[0][cha], mc, cha)
		   & Query_NHM_DIMM(did[1][cha], mc, cha);
	}
	return (rc);
}

void Query_SNB_IMC(void __iomem *mchmap)
{	/* Sources:	2nd & 3rd Generation Intel® Core™ Processor Family
			Intel® Xeon Processor E3-1200 Family	*/
	unsigned short cha, dimmCount[2];

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.MC[0].SNB.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].SNB.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount = 0;

	dimmCount[0] = (Proc->Uncore.MC[0].SNB.MAD0.Dimm_A_Size > 0)
		     + (Proc->Uncore.MC[0].SNB.MAD0.Dimm_B_Size > 0);
	dimmCount[1] = (Proc->Uncore.MC[0].SNB.MAD1.Dimm_A_Size > 0)
		     + (Proc->Uncore.MC[0].SNB.MAD1.Dimm_B_Size > 0);

	for (cha = 0; cha < 2; cha++)
		Proc->Uncore.MC[0].ChannelCount += (dimmCount[cha] > 0);

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].SNB.DBP.value =
					readl(mchmap + 0x4000 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].SNB.RAP.value =
					readl(mchmap + 0x4004 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].SNB.RFTP.value =
					readl(mchmap + 0x4298 + 0x400 * cha);
	}
	/* Is Dual DIMM Per Channel Disable ? */
	Proc->Uncore.MC[0].SlotCount = (Proc->Uncore.Bus.SNB_Cap.DDPCD == 1) ?
					1 : Proc->Uncore.MC[0].ChannelCount;
}

void Query_Turbo_TDP_Config(void __iomem *mchmap)
{
	TURBO_ACTIVATION TurboActivation = {.value = 0};
	CONFIG_TDP_NOMINAL NominalTDP = {.value = 0};
	CONFIG_TDP_CONTROL ControlTDP = {.value = 0};
	CONFIG_TDP_LEVEL ConfigTDP;

	NominalTDP.value = readl(mchmap + 0x5f3c);
	Proc->Boost[BOOST(TDP)] = NominalTDP.Ratio;

	ConfigTDP.value = readq(mchmap + 0x5f40);
	Proc->Boost[BOOST(TDP1)] = ConfigTDP.Ratio;

	ConfigTDP.value = readq(mchmap + 0x5f48);
	Proc->Boost[BOOST(TDP2)] = ConfigTDP.Ratio;

	ControlTDP.value = readl(mchmap + 0x5f50);
	Proc->Features.TDP_Cfg_Lock  = ControlTDP.Lock;
	Proc->Features.TDP_Cfg_Level = ControlTDP.Level;

	TurboActivation.value = readl(mchmap + 0x5f54);
	Proc->Boost[BOOST(ACT)] = TurboActivation.MaxRatio;
	Proc->Features.TurboActiv_Lock = TurboActivation.Ratio_Lock;

	Proc->Features.TDP_Levels = 3;
}

void Query_HSW_IMC(void __iomem *mchmap)
{	/*Source: Desktop 4th & 5th Generation Intel® Core™ Processor Family.*/
	unsigned short cha, dimmCount[2];

	Proc->Uncore.CtrlCount = 1;

	Proc->Uncore.MC[0].SNB.MAD0.value = readl(mchmap + 0x5004);
	Proc->Uncore.MC[0].SNB.MAD1.value = readl(mchmap + 0x5008);

	Proc->Uncore.MC[0].ChannelCount = 0;

	dimmCount[0] = (Proc->Uncore.MC[0].SNB.MAD0.Dimm_A_Size > 0)
		     + (Proc->Uncore.MC[0].SNB.MAD0.Dimm_B_Size > 0);
	dimmCount[1] = (Proc->Uncore.MC[0].SNB.MAD1.Dimm_A_Size > 0)
		     + (Proc->Uncore.MC[0].SNB.MAD1.Dimm_B_Size > 0);

	for (cha = 0; cha < 2; cha++)
		Proc->Uncore.MC[0].ChannelCount += (dimmCount[cha] > 0);

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++)
	{
/*TODO( Unsolved: What is the channel #1 'X' factor of Haswell registers ? )
		Proc->Uncore.MC[0].Channel[cha].HSW.Timing.value =
					readl(mchmap + 0x4c04 + X * cha);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank_A.value =
					readl(mchmap + 0x4c08 + X * cha);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank_B.value =
					readl(mchmap + 0x4c0c + X * cha);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank.value =
					readl(mchmap + 0x4c14 + X * cha);

		Proc->Uncore.MC[0].Channel[cha].HSW.Refresh.value =
					readl(mchmap + 0x4e98 + X * cha);
*/
		Proc->Uncore.MC[0].Channel[cha].HSW.Timing.value =
					readl(mchmap + 0x4c04);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank_A.value =
					readl(mchmap + 0x4c08);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank_B.value =
					readl(mchmap + 0x4c0c);

		Proc->Uncore.MC[0].Channel[cha].HSW.Rank.value =
					readl(mchmap + 0x4c14);

		Proc->Uncore.MC[0].Channel[cha].HSW.Refresh.value =
					readl(mchmap + 0x4e98);
	}
	/* Is Dual DIMM Per Channel Disable ? */
	Proc->Uncore.MC[0].SlotCount = (Proc->Uncore.Bus.SNB_Cap.DDPCD == 1) ?
					1 : Proc->Uncore.MC[0].ChannelCount;

	Query_Turbo_TDP_Config(mchmap);
}

void Query_SKL_IMC(void __iomem *mchmap)
{	/*Source: 6th & 7th Generation Intel® Processor for S-Platforms Vol 2*/
	unsigned short cha;

	Proc->Uncore.CtrlCount = 1;
	/* Intra channel configuration */
	Proc->Uncore.MC[0].SKL.MADCH.value = readl(mchmap + 0x5000);
	if (Proc->Uncore.MC[0].SKL.MADCH.CH_L_MAP) {
		Proc->Uncore.MC[0].SKL.MADC0.value = readl(mchmap + 0x5008);
		Proc->Uncore.MC[0].SKL.MADC1.value = readl(mchmap + 0x5004);
	} else {
		Proc->Uncore.MC[0].SKL.MADC0.value = readl(mchmap + 0x5004);
		Proc->Uncore.MC[0].SKL.MADC1.value = readl(mchmap + 0x5008);
	}
	/* DIMM parameters */
	Proc->Uncore.MC[0].SKL.MADD0.value = readl(mchmap + 0x500c);
	Proc->Uncore.MC[0].SKL.MADD1.value = readl(mchmap + 0x5010);
	/* Sum up any present DIMM per channel. */
	Proc->Uncore.MC[0].ChannelCount =
		  ((Proc->Uncore.MC[0].SKL.MADD0.Dimm_L_Size != 0)
		|| (Proc->Uncore.MC[0].SKL.MADD0.Dimm_S_Size != 0))
		+ ((Proc->Uncore.MC[0].SKL.MADD1.Dimm_L_Size != 0)
		|| (Proc->Uncore.MC[0].SKL.MADD1.Dimm_S_Size != 0));
	/* Max of populated DIMMs L and DIMMs S */
	Proc->Uncore.MC[0].SlotCount = KMAX(
				(1 + Proc->Uncore.MC[0].SKL.MADC0.Dimm_L_Map),
				(1 + Proc->Uncore.MC[0].SKL.MADC1.Dimm_L_Map)
	);

	for (cha = 0; cha < Proc->Uncore.MC[0].ChannelCount; cha++) {
		Proc->Uncore.MC[0].Channel[cha].SKL.Timing.value =
					readl(mchmap + 0x4000 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].SKL.Sched.value =
					readl(mchmap + 0x401c + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].SKL.ODT.value =
					readl(mchmap + 0x4070 + 0x400 * cha);

		Proc->Uncore.MC[0].Channel[cha].SKL.Refresh.value =
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

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_NHM_IMC(dev, did, mc);

	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned short mc;

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_Lynnfield_IMC(dev, mc);

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

	Proc->Uncore.ChipID = dev->device;

	Proc->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < Proc->Uncore.CtrlCount) && !rc; mc++)
		rc = Query_NHM_IMC(dev, did, mc);

	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x50, &Proc->Uncore.Bus.DimmClock.value);

	return (0);
}

static PCI_CALLBACK NHM_NON_CORE(struct pci_dev *dev)
{
	NHM_CURRENT_UCLK_RATIO UncoreClock = {.value = 0};

	pci_read_config_dword(dev, 0xc0, &UncoreClock.value);

	Proc->Uncore.Boost[UNCORE_BOOST(MAX)] = UncoreClock.UCLK;
	Proc->Uncore.Boost[UNCORE_BOOST(MIN)] = UncoreClock.MinRatio;

	return (0);
}

static PCI_CALLBACK X58_QPI(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xd0, &Proc->Uncore.Bus.QuickPath.value);

	return (0);
}

static PCI_CALLBACK X58_VTD(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned int base = 0;

	pci_read_config_dword(dev, 0x180, &base);
	if (base) {
		Proc->Uncore.Bus.QuickPath.X58.VT_d = 0;
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
	} else
		Proc->Uncore.Bus.QuickPath.X58.VT_d = 1;

	return ((PCI_CALLBACK) rc);
}

static PCI_CALLBACK SNB_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4, &Proc->Uncore.Bus.SNB_Cap.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SNB_IMC));
}

static PCI_CALLBACK IVB_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4, &Proc->Uncore.Bus.SNB_Cap.value);
	pci_read_config_dword(dev, 0xe8, &Proc->Uncore.Bus.IVB_Cap.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SNB_IMC));
}

static PCI_CALLBACK SNB_EP_HB(struct pci_dev *dev)
{
	Proc->Uncore.ChipID = dev->device;

	return ((PCI_CALLBACK) 0);
}

static PCI_CALLBACK SNB_EP_CAP(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x84, &Proc->Uncore.Bus.SNB_EP_Cap0.value);
	pci_read_config_dword(dev, 0x88, &Proc->Uncore.Bus.SNB_EP_Cap1.value);
	pci_read_config_dword(dev, 0x8c, &Proc->Uncore.Bus.SNB_EP_Cap2.value);
	pci_read_config_dword(dev, 0x90, &Proc->Uncore.Bus.SNB_EP_Cap3.value);
	pci_read_config_dword(dev, 0x94, &Proc->Uncore.Bus.SNB_EP_Cap4.value);

	return ((PCI_CALLBACK) 0);
}

kernel_ulong_t SNB_EP_CTRL(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(dev, 0x7c,&Proc->Uncore.MC[mc].SNB_EP.TECH.value);
	pci_read_config_dword(dev, 0x80,&Proc->Uncore.MC[mc].SNB_EP.TAD.value);

	Proc->Uncore.MC[mc].ChannelCount=Proc->Uncore.MC[mc].SNB_EP.TAD.CH_WAY;
	Proc->Uncore.MC[mc].ChannelCount++;
/* TODO */
	Proc->Uncore.MC[mc].SlotCount = 2;

	return (0);
}

static PCI_CALLBACK SNB_EP_CTRL0(struct pci_dev *dev)
{
	if (Proc->Uncore.CtrlCount < 1) {
		Proc->Uncore.CtrlCount = 1;
	}
	SNB_EP_CTRL(dev, 0);

	return (0);
}

static PCI_CALLBACK SNB_EP_CTRL1(struct pci_dev *dev)
{
	if (Proc->Uncore.CtrlCount < 2) {
		Proc->Uncore.CtrlCount = 2;
	}
	SNB_EP_CTRL(dev, 1);

	return (0);
}

kernel_ulong_t SNB_EP_IMC(struct pci_dev *dev ,unsigned short mc,
						unsigned short cha)
{
	pci_read_config_dword(dev, 0x200,
			&Proc->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.value);

	pci_read_config_dword(dev, 0x204,
			&Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.value);

	pci_read_config_dword(dev, 0x208,
			&Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.value);

	pci_read_config_dword(dev, 0x214,
			&Proc->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.value);

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
			&Proc->Uncore.MC[mc].Channel[cha].DIMM[0].MTR.value);

	pci_read_config_dword(dev, 0x84,
			&Proc->Uncore.MC[mc].Channel[cha].DIMM[1].MTR.value);

	pci_read_config_dword(dev, 0x88,
			&Proc->Uncore.MC[mc].Channel[cha].DIMM[2].MTR.value);
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
	pci_read_config_dword(dev, 0xd4, &Proc->Uncore.Bus.QuickPath.value);

	return (0);
}

static PCI_CALLBACK HSW_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4, &Proc->Uncore.Bus.SNB_Cap.value);
	pci_read_config_dword(dev, 0xe8, &Proc->Uncore.Bus.IVB_Cap.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_HSW_IMC));
}

static PCI_CALLBACK SKL_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4, &Proc->Uncore.Bus.SKL_Cap_A.value);
	pci_read_config_dword(dev, 0xe8, &Proc->Uncore.Bus.SKL_Cap_B.value);
	pci_read_config_dword(dev, 0xec, &Proc->Uncore.Bus.SKL_Cap_C.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SKL_IMC));
}
/* TODO
static PCI_CALLBACK SKL_SA(struct pci_dev *dev)
{
	SKL_SA_PLL_RATIOS PllRatios = {.value = 0};

	pci_read_config_dword(dev, 0x5918, &PllRatios.value);

	Proc->Uncore.Boost[UNCORE_BOOST(MAX)] = PllRatios.UCLK;
	Proc->Uncore.Boost[UNCORE_BOOST(MIN)] = 0;

	pci_read_config_dword(dev, 0xe4, &Proc->Uncore.Bus.SKL_Cap_A.value);
	pci_read_config_dword(dev, 0xe8, &Proc->Uncore.Bus.SKL_Cap_B.value);

	return (Router(dev, 0x48, 64, 0x8000, Query_SKL_IMC));

	return (0);
}
*/
static PCI_CALLBACK AMD_0Fh_MCH(struct pci_dev *dev)
{	/* Source: BKDG for AMD NPT Family 0Fh Processors. */
	unsigned short cha, slot, chip;

	Proc->Uncore.ChipID = dev->device;
	/* Specs defined. */
	Proc->Uncore.CtrlCount = 1;
	/* DRAM Configuration low register. */
	pci_read_config_dword(dev, 0x90,
			&Proc->Uncore.MC[0].AMD0F.DCRL.value);
	/* DRAM Configuration high register. */
	pci_read_config_dword(dev, 0x94,
			&Proc->Uncore.MC[0].AMD0F.DCRH.value);
	/* 1 channel if 64 bits / 2 channels if 128 bits width. */
	Proc->Uncore.MC[0].ChannelCount = Proc->Uncore.MC[0].AMD0F.DCRL.Width128
					+ 1;
	/* DIMM Geometry. */
	for (chip = 0; chip < 8; chip++) {
		cha = chip >> 2;
		slot = chip % 4;
		pci_read_config_dword(dev, 0x40 + 4 * chip,
			&Proc->Uncore.MC[0].Channel[cha].DIMM[slot].MBA.value);

		Proc->Uncore.MC[0].SlotCount +=
			Proc->Uncore.MC[0].Channel[cha].DIMM[slot].MBA.CSEnable;
	}
	/* DIMM Size. */
	pci_read_config_dword(	dev, 0x80,
				&Proc->Uncore.MC[0].MaxDIMMs.AMD0F.CS.value);
	/* DRAM Timings. */
	pci_read_config_dword(dev, 0x88,
			&Proc->Uncore.MC[0].Channel[0].AMD0F.DTRL.value);
	/* Assume same timings for both channels. */
	Proc->Uncore.MC[0].Channel[1].AMD0F.DTRL.value =
			Proc->Uncore.MC[0].Channel[0].AMD0F.DTRL.value;

	return (0);
}

static PCI_CALLBACK AMD_0Fh_HTT(struct pci_dev *dev)
{
	unsigned int link;

	pci_read_config_dword(dev, 0x64, &Proc->Uncore.Bus.UnitID.value);

	for (link = 0; link < 3; link++) {
		pci_read_config_dword(dev, 0x88 + 0x20 * link,
				&Proc->Uncore.Bus.LDTi_Freq[link].value);
	};

	return (0);
}
#ifdef CONFIG_AMD_NB
static PCI_CALLBACK AMD_17h_ZenIF(struct pci_dev *dev)
{
	if (KPrivate->ZenIF_dev == NULL) {
		KPrivate->ZenIF_dev = dev;
	}
	return (0);
}
#endif /* CONFIG_AMD_NB */
/* TODO
static PCI_CALLBACK AMD_IOMMU(struct pci_dev *dev)
{
	void __iomem *mmio;
	unsigned long long base;
	unsigned int low = 0, high = 0;

	Proc->Uncore.ChipID = dev->device;

	pci_read_config_dword(dev, 0x4, &low);
	pci_read_config_dword(dev, 0x8, &high);

	base = ((low & 0b11111111111111111100000000000000) >> 14)
		+ ((unsigned long long) high << 32);

	if (BITVAL(low, 0)) {
		mmio = ioremap(base, 0x4000);
		if (mmio != NULL) {
			Proc->Uncore.Bus.IOMMU_CR = readq(mmio + 0x18);

			iounmap(mmio);

			return (0);
		} else
			return ((PCI_CALLBACK) -ENOMEM);
	}
	return ((PCI_CALLBACK) -ENOMEM);
}
*/
static int CoreFreqK_ProbePCI(void)
{
	struct pci_device_id *id = Arch[Proc->ArchID].PCI_ids;
	struct pci_dev *dev = NULL;
	int rc = -ENODEV;

	while (id->vendor || id->subvendor || id->class_mask) {
		dev = pci_get_device(id->vendor, id->device, NULL);
		if (dev != NULL) {
		    if (!pci_enable_device(dev)) {
			PCI_CALLBACK Callback = (PCI_CALLBACK) id->driver_data;
				rc =(int) Callback(dev);
			pci_disable_device(dev);
		    }
		    pci_dev_put(dev);
		}
		id++;
	}
	return (rc);
}

void Query_GenuineIntel(void)
{
	Intel_Core_Platform_Info();
	HyperThreading_Technology();
}

void Query_Core2(void)
{
	Intel_Core_Platform_Info();
	HyperThreading_Technology();
}

void Query_Nehalem(void)
{
	Nehalem_Platform_Info();
	HyperThreading_Technology();
}

void Query_SandyBridge(void)
{
	Nehalem_Platform_Info();
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio();
	SandyBridge_PowerInterface();
}

void Query_IvyBridge(void)
{
	Nehalem_Platform_Info();
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio();
	Intel_Turbo_TDP_Config();
	SandyBridge_PowerInterface();
}

void Query_IvyBridge_EP(void)
{
	IvyBridge_EP_Platform_Info();
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio();
	SandyBridge_PowerInterface();
}

void Query_Haswell(void)
{
	if (Proc->Features.Power.EAX.TurboIDA) {
		Proc->Features.Uncore_Unlock = 1;
	}
	Nehalem_Platform_Info();
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio();
	Intel_Turbo_TDP_Config();
	SandyBridge_PowerInterface();
}

void Query_Haswell_EP(void)
{
	if (Proc->Features.Power.EAX.TurboIDA) {
		Proc->Features.Uncore_Unlock = 1;
	}
	Haswell_EP_Platform_Info();
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	Intel_Turbo_TDP_Config();
	SandyBridge_PowerInterface();
}

void Query_Broadwell(void)
{
	if (Proc->Features.Power.EAX.TurboIDA) {
		Proc->Features.Uncore_Unlock = 1;
	}
	Nehalem_Platform_Info();
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	Intel_Turbo_TDP_Config();
	SandyBridge_PowerInterface();
	Intel_Hardware_Performance();
}

void Query_Broadwell_EP(void)
{
	Query_Haswell_EP();
	Intel_Hardware_Performance();
}

void Query_Skylake_X(void)
{
	if (Proc->Features.Power.EAX.TurboIDA) {
		Proc->Features.Uncore_Unlock = 1;
	}
	Skylake_X_Platform_Info();
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	Intel_Turbo_TDP_Config();
	SandyBridge_PowerInterface();
	Intel_Hardware_Performance();
}

void Query_AuthenticAMD(void)
{	/* Fallback algorithm for unspecified AMD architectures. */
	Proc->Boost[BOOST(MIN)] = 8;

	if (Proc->Features.AdvPower.EDX.HwPstate == 1) {
		COFVID CofVid = {.value = 0};

		switch (Arch[Proc->ArchID].Signature.ExtFamily) {
		case 0x1:
		case 0x6:
		case 0x7:
			RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
			Proc->Boost[BOOST(MAX)] = CofVid.Arch_COF.MaxCpuCof;
			break;
		case 0x2:
			RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
			Proc->Boost[BOOST(MAX)]=CofVid.Arch_Pll.MainPllOpFidMax;
			if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
				Proc->Boost[BOOST(MAX)] += 0x8;
			break;
		case 0x3:
		case 0x5:
			RDMSR(CofVid, MSR_AMD_COFVID_STATUS);
			Proc->Boost[BOOST(MAX)]=CofVid.Arch_Pll.MainPllOpFidMax;
			if (CofVid.Arch_Pll.MainPllOpFidMax > 0)
				Proc->Boost[BOOST(MAX)] += 0x10;
			break;
		}
	} else { /* No Solution! */
		Proc->Boost[BOOST(MAX)] = Proc->Boost[BOOST(MIN)];
	}

	Proc->Boost[BOOST(1C)] = Proc->Boost[BOOST(MAX)];

	Proc->Features.SpecTurboRatio = 0;

	HyperThreading_Technology();
}

void Query_AMD_Family_0Fh(void)
{   /* BKDG for AMD NPT Family 0Fh: §13.8 */
    if (Proc->Features.AdvPower.EDX.FID == 1) {
	/* Processor supports FID changes. */
	FIDVID_STATUS FidVidStatus = {.value = 0};

	RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

	Proc->Boost[BOOST(MIN)] = VCO[FidVidStatus.StartFID].MCF;
	Proc->Boost[BOOST(MAX)] = 8 + FidVidStatus.MaxFID;

	if (FidVidStatus.StartFID < 0b1000) {
	    unsigned int t;
	    for (t = 0; t < 5; t++) {
		Proc->Boost[BOOST(SIZE)-5+t]=VCO[FidVidStatus.StartFID].PCF[t];
	    }

	    Proc->Features.SpecTurboRatio = 5;
	} else {
		Proc->Boost[BOOST(1C)] = 8 + FidVidStatus.MaxFID;

		Proc->Features.SpecTurboRatio = 1;
	}
    } else {
	HWCR HwCfgRegister = {.value = 0};

	RDMSR(HwCfgRegister, MSR_K7_HWCR);

	Proc->Boost[BOOST(MIN)] = 8 + HwCfgRegister.Family_0Fh.StartFID;
	Proc->Boost[BOOST(MAX)] = Proc->Boost[BOOST(MIN)];
	Proc->Boost[BOOST(1C) ] = Proc->Boost[BOOST(MIN)];

	Proc->Features.SpecTurboRatio = 1;
    }

	HyperThreading_Technology();
}

void Query_AMD_Family_10h(void)
{
	unsigned int pstate, sort[5] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 4; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_10h.CpuFid + 0x10)
					  / (1 << PstateDef.Family_10h.CpuDid);
	}
	Proc->Features.SpecTurboRatio = 3;

	HyperThreading_Technology();
}

void Query_AMD_Family_11h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_10h.CpuFid + 0x8)
					  / (1 << PstateDef.Family_10h.CpuDid);
	}
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_12h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_12h.CpuFid + 0x10)
					  /  PstateDef.Family_12h.CpuDid;
	}
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_14h(void)
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

	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		ClockDiv = (PstateDef.Family_14h.CpuDidMSD + 1) * 4;
		ClockDiv += PstateDef.Family_14h.CpuDidLSD;

		Proc->Boost[sort[pstate]] = (MaxFreq * 4) / ClockDiv;
	}	/* @ MainPllOpFidMax MHz */
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();
}

void Query_AMD_Family_15h(void)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++) {
		PSTATEDEF PstateDef = {.value = 0};

		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		Proc->Boost[sort[pstate]] = (PstateDef.Family_15h.CpuFid + 0x10)
					  / (1 << PstateDef.Family_15h.CpuDid);
	}
	Proc->Features.SpecTurboRatio = 6;

	HyperThreading_Technology();

  /* Find micro-architecture based on the CPUID model. Bulldozer initialized */
    switch (Proc->Features.Std.EAX.ExtModel) {
    case 0x0:
	if ((Proc->Features.Std.EAX.Model >= 0x0)
	 && (Proc->Features.Std.EAX.Model <= 0xf)) {
		StrCopy(Proc->Architecture,
			Arch[Proc->ArchID].Architecture[CN_PILEDRIVER].CodeName,
			CODENAME_LEN);
	}
	break;
    case 0x1:
	if ((Proc->Features.Std.EAX.Model >= 0x0)
	 && (Proc->Features.Std.EAX.Model <= 0xf)) {
		StrCopy(Proc->Architecture,
			Arch[Proc->ArchID].Architecture[CN_PILEDRIVER].CodeName,
			CODENAME_LEN);
	}
	break;
    case 0x3:
	if ((Proc->Features.Std.EAX.Model >= 0x0)
	 && (Proc->Features.Std.EAX.Model <= 0xf)) {
		StrCopy(Proc->Architecture,
		    Arch[Proc->ArchID].Architecture[CN_STEAMROLLER].CodeName,
			CODENAME_LEN);
	}
	break;
    case 0x6:
    case 0x7:
	if ((Proc->Features.Std.EAX.Model >= 0x0)
	 && (Proc->Features.Std.EAX.Model <= 0xf)) {
		StrCopy(Proc->Architecture,
			Arch[Proc->ArchID].Architecture[CN_EXCAVATOR].CodeName,
			CODENAME_LEN);
	}
	break;
    };
}

unsigned int AMD_Zen_CoreCOF(unsigned int FID, unsigned int DID)
{/* Source: PPR for AMD Family 17h Model 01h, Revision B1 Processors
    CoreCOF = (PStateDef[CpuFid[7:0]] / PStateDef[CpuDfsId]) * 200 */
	unsigned int COF;
	if (DID != 0) {
		COF = (FID << 1) / DID;
	} else {
		COF = FID >> 2;
	}
	return (COF);
}

unsigned int AMD_Zen_CoreFID(unsigned int COF, unsigned int DID)
{
	unsigned int FID;
	if (DID != 0) {
		FID = (COF * DID) >> 1;
	} else {
		FID = COF << 2;
	}
	return (FID);
}

void Compute_AMD_Zen_Boost(void)
{
	PROCESSOR_SPECIFIC *pSpecific = LookupProcessor();
	unsigned int COF = 0, pstate, sort[8] = { /* P[0..7]-States */
		BOOST(MAX), BOOST(1C), BOOST(2C), BOOST(3C),
		BOOST(4C) , BOOST(5C), BOOST(6C), BOOST(7C)
	};
	HWCR HwCfgRegister = {.value = 0};
	PSTATEDEF PstateDef = {.value = 0};

	if (pSpecific != NULL) {
		/* Save thermal parameters for later use in the Daemon	*/
		Arch[Proc->ArchID].Specific[0].Param = pSpecific->Param;
		/* Override Processor CodeName & Locking capabilities	*/
		OverrideCodeNameString(pSpecific);
		OverrideUnlockCapability(pSpecific);
	} else {
		Arch[Proc->ArchID].Specific[0].Param.Target = 0;
	}
	for (pstate = BOOST(MIN); pstate < BOOST(SIZE); pstate++)
		Proc->Boost[pstate] = 0;

	/*Core & L3 frequencies < 400MHz are not supported by the architecture*/
	Proc->Boost[BOOST(MIN)] = 4;
	/* Loop over all frequency ids. */
	for (pstate = 0; pstate <= 7; pstate++) {
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));
		/* Handle only valid P-States. */
		if (PstateDef.Family_17h.PstateEn) {
			COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
						PstateDef.Family_17h.CpuDfsId);

			Proc->Boost[sort[pstate]] = COF;
		}
	}
	Proc->Features.SpecTurboRatio = pstate;

	/* If CPB is enabled then add Boost + XFR to the P0 ratio. */
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
	if (!HwCfgRegister.Family_17h.CpbDis)
	{
		Proc->Boost[BOOST(CPB)] = Proc->Boost[BOOST(MAX)];
		Proc->Boost[BOOST(XFR)] = Proc->Boost[BOOST(MAX)];

	    if (pSpecific != NULL) {
		Proc->Boost[BOOST(CPB)] += pSpecific->Boost[0]; /* Boost */

		Proc->Boost[BOOST(XFR)] += pSpecific->Boost[0]; /* Boost */
		Proc->Boost[BOOST(XFR)] += pSpecific->Boost[1]; /* XFR */
	    }
		Proc->Features.TDP_Levels = 2; /* Count the Xtra Boost ratios */
	} else {
		Proc->Features.TDP_Levels = 0; /* Disabled CPB: Hide ratios */
	}
}

typedef struct {
	CLOCK_ARG *pClockMod;
	unsigned long long PstateAddr;
	unsigned int BoostIndex;
} CLOCK_ZEN_ARG;

void ReCompute_AMD_Zen_Boost(CLOCK_ZEN_ARG *pClockZen)
{
	PSTATEDEF PstateDef = {.value = 0};
	unsigned int COF = 0;
	/* The target frequency P-State figures as an exception */
	if (pClockZen->BoostIndex == BOOST(TGT)) {
		PSTATECTRL PstateCtrl = {.value = 0};
		RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
		RDMSR(PstateDef, pClockZen->PstateAddr + PstateCtrl.PstateCmd);
	} else {
		RDMSR(PstateDef, pClockZen->PstateAddr);
	}
	COF = AMD_Zen_CoreCOF(	PstateDef.Family_17h.CpuFid,
				PstateDef.Family_17h.CpuDfsId );

	Proc->Boost[pClockZen->BoostIndex] = COF;
}

static void TargetClock_AMD_Zen_PerCore(void *arg)
{
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;
	unsigned int pstate , target = Proc->Boost[pClockZen->BoostIndex]
					+ pClockZen->pClockMod->Offset;
    /* Look-up for the first enabled P-State with the same target frequency */
    for (pstate = 0; pstate <= 7; pstate++) {
	PSTATEDEF PstateDef = {.value = 0};
	RDMSR(PstateDef, pClockZen->PstateAddr + pstate);

	if (PstateDef.Family_17h.PstateEn)
	{
		unsigned int COF=AMD_Zen_CoreCOF(PstateDef.Family_17h.CpuFid,
						PstateDef.Family_17h.CpuDfsId);

		if (COF == target) {
			PSTATECTRL PstateCtrl = {.value = 0};
			/* Command a new target P-state */
			RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
			PstateCtrl.PstateCmd = pstate;
			WRMSR(PstateCtrl, MSR_AMD_PERF_CTL);

			return;
		}
	}
    }
}

static void TurboClock_AMD_Zen_PerCore(void *arg)
{
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;
	PSTATEDEF PstateDef = {.value = 0};
	HWCR HwCfgRegister = {.value = 0};
	/* Make sure the Core Performance Boost is disabled. */
	RDMSR(HwCfgRegister, MSR_K7_HWCR);
	if (HwCfgRegister.Family_17h.CpbDis)
	{
		RDMSR(PstateDef, pClockZen->PstateAddr);
		/* Apply if and only if the P-State is enabled */
		if (PstateDef.Family_17h.PstateEn)
		{
			unsigned int FID = 0;
			/* Compute the Frequency ID from the offsetted ratio */
			FID = AMD_Zen_CoreFID(Proc->Boost[pClockZen->BoostIndex]
						+ pClockZen->pClockMod->Offset,
						PstateDef.Family_17h.CpuDfsId);
			/* Write the P-State MSR with the new FID */
			PstateDef.Family_17h.CpuFid = FID;
			WRMSR(PstateDef, pClockZen->PstateAddr);
		}
	}
}

void For_All_AMD_Zen_Clock(CLOCK_ZEN_ARG *pClockZen, void (*PerCore)(void *))
{
	unsigned int cpu = Proc->CPU.Count;
  do {
	cpu--;	/* From last AP to BSP */

    if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)
    && ((pClockZen->pClockMod->cpu == -1)||(pClockZen->pClockMod->cpu == cpu)))
    {
	smp_call_function_single(cpu, PerCore, pClockZen, 1);
    }
  } while (cpu != 0) ;
}

long TurboClock_AMD_Zen(CLOCK_ARG *pClockMod)
{
	if (pClockMod != NULL) {
	    if ((pClockMod->NC >= 1) && (pClockMod->NC <= 7))
	    {
		CLOCK_ZEN_ARG ClockZen = {	/* P[1..7]-States */
			.pClockMod  = pClockMod,
			.PstateAddr = MSR_AMD_PSTATE_DEF_BASE + pClockMod->NC,
			.BoostIndex = BOOST(SIZE) - pClockMod->NC
		};

		For_All_AMD_Zen_Clock(&ClockZen, TurboClock_AMD_Zen_PerCore);

		ReCompute_AMD_Zen_Boost(&ClockZen);
		return (2);	/* Report a platform change */
	    }
	}
	return (0);
}

long ClockMod_AMD_Zen(CLOCK_ARG *pClockMod)
{
    if (pClockMod != NULL) {
	switch (pClockMod->NC) {
	case CLOCK_MOD_MAX:
	    {
		CLOCK_ZEN_ARG ClockZen = { /* P[0]:Max non-boosted P-State */
			.pClockMod  = pClockMod,
			.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
			.BoostIndex = BOOST(MAX)
		};

		For_All_AMD_Zen_Clock(&ClockZen, TurboClock_AMD_Zen_PerCore);

		ReCompute_AMD_Zen_Boost(&ClockZen);
		return (2);
	    }
	case CLOCK_MOD_TGT:
	    {
		CLOCK_ZEN_ARG ClockZen = {	/* Target non-boosted P-State */
			.pClockMod  = pClockMod,
			.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
			.BoostIndex = BOOST(TGT)
		};

		For_All_AMD_Zen_Clock(&ClockZen, TargetClock_AMD_Zen_PerCore);

		ReCompute_AMD_Zen_Boost(&ClockZen);
		return (2);
	    }
	default:
		break;
	}
    }
	return (0);
}

void Query_AMD_Family_17h(void)
{
	CLOCK_ZEN_ARG ClockZen = {
		.pClockMod  = NULL,	/* Unused field at this time	*/
		.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
		.BoostIndex = BOOST(TGT)
	};
	Proc->Features.TgtRatio_Unlock = 1; /* Unlock P-state Control	*/
	Compute_AMD_Zen_Boost();
	ReCompute_AMD_Zen_Boost(&ClockZen); /* Initial Target P-State value */
	/* Apply same register bit fields as Intel RAPL_POWER_UNIT */
	RDMSR(Proc->PowerThermal.Unit, MSR_AMD_RAPL_POWER_UNIT);

	HyperThreading_Technology();
}

void Dump_CPUID(CORE *Core)
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
	for (i = 0; (i < CPUID_MAX_FUNC) && (Core->CpuID[i].func != 0x0); i++) {
		__asm__ volatile
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

void SpeedStep_Technology(CORE *Core)				/*Per Package*/
{
	if (Core->Bind == Proc->Service.Core) {
		if (Proc->Features.Std.ECX.EIST == 1) {
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
				BITSET_CC(LOCKLESS, Proc->SpeedStep, Core->Bind);
			else
				BITCLR_CC(LOCKLESS, Proc->SpeedStep, Core->Bind);

		} else {
			BITCLR_CC(LOCKLESS, Proc->SpeedStep, Core->Bind);
		}
		BITSET_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);
	}
}

typedef void (*SET_TARGET)(CORE*, unsigned int);

static void Set_Core2_Target(CORE *Core, unsigned int ratio)
{
	Core->PowerThermal.PerfControl.CORE.TargetRatio = ratio;
}

static void Set_Nehalem_Target(CORE *Core, unsigned int ratio)
{
	Core->PowerThermal.PerfControl.NHM.TargetRatio = ratio & 0xff;
}

static void Set_SandyBridge_Target(CORE *Core, unsigned int ratio)
{
	Core->PowerThermal.PerfControl.SNB.TargetRatio = ratio;
}

#define GET_CORE2_TARGET(Core)						\
(									\
	Core->PowerThermal.PerfControl.CORE.TargetRatio 		\
)

typedef unsigned int (*GET_TARGET)(CORE*);

static unsigned int Get_Core2_Target(CORE *Core)
{
	return (GET_CORE2_TARGET(Core));
}

#define GET_NEHALEM_TARGET(Core)					\
(									\
	Core->PowerThermal.PerfControl.NHM.TargetRatio & 0xff		\
)

static unsigned int Get_Nehalem_Target(CORE *Core)
{
	return (GET_NEHALEM_TARGET(Core));
}

#define GET_SANDYBRIDGE_TARGET(Core)					\
(									\
	Core->PowerThermal.PerfControl.SNB.TargetRatio			\
)

static unsigned int Get_SandyBridge_Target(CORE *Core)
{
	return (GET_SANDYBRIDGE_TARGET(Core));
}

#define GET_HWP_MINIMUM(Core)						\
(									\
	Core->PowerThermal.HWP_Request.Minimum_Perf			\
)

static unsigned int Get_HWP_Min(CORE *Core)
{
	return (GET_HWP_MINIMUM(Core));
}

#define GET_HWP_MAXIMUM(Core)						\
(									\
	Core->PowerThermal.HWP_Request.Maximum_Perf			\
)

static unsigned int Get_HWP_Max(CORE *Core)
{
	return (GET_HWP_MAXIMUM(Core));
}

#define GET_HWP_DESIRED(Core)						\
(									\
	Core->PowerThermal.HWP_Request.Desired_Perf			\
)

static unsigned int Get_HWP_Target(CORE *Core)
{
	return (GET_HWP_DESIRED(Core));
}

typedef int (*CMP_TARGET)(CORE*, unsigned int);

static int Cmp_Core2_Target(CORE *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.CORE.TargetRatio == ratio);
}

static int Cmp_Nehalem_Target(CORE *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.NHM.TargetRatio == ratio);
}

static int Cmp_SandyBridge_Target(CORE *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.SNB.TargetRatio > ratio);
}

void Aggregate_Reset(enum RATIO_BOOST boost, unsigned int ratio)
{
	Proc->Boost[boost] = ratio;
}

void Aggregate_Ratio(enum RATIO_BOOST boost, unsigned int ratio)
{
	if (ratio > Proc->Boost[boost])
	{
		Proc->Boost[boost] = ratio;
	}
}

void TurboBoost_Technology(CORE *Core,	SET_TARGET SetTarget,
					GET_TARGET GetTarget,
					CMP_TARGET CmpTarget,
					unsigned int TurboRatio,
					unsigned int ValidRatio)
{								/* Per SMT */
	int ToggleFeature;
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);

	BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask, Core->Bind);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

  if ((MiscFeatures.Turbo_IDA == 0) && (Proc->Features.Power.EAX.TurboIDA))
  {
    switch (TurboBoost_Enable) {
    case COREFREQ_TOGGLE_OFF:
	Core->PowerThermal.PerfControl.Turbo_IDA = 1;
	SetTarget(Core, Proc->Boost[BOOST(MAX)]); /* Restore nominal P-state */
	ToggleFeature = 1;
	break;
    case COREFREQ_TOGGLE_ON:
	Core->PowerThermal.PerfControl.Turbo_IDA = 0;
	SetTarget(Core, TurboRatio);	/* Request the Turbo P-state	*/
	ToggleFeature = 1;
	break;
    }
    if (ToggleFeature == 1) {
	WRMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
    }
    if (Core->PowerThermal.PerfControl.Turbo_IDA == 0) {
	BITSET_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
    } else {
	BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
    }
  } else {
	BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
  }

  if (Proc->Features.HWP_Enable)
  {
	RDMSR(Core->PowerThermal.HWP_Capabilities, MSR_IA32_HWP_CAPABILITIES);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

    if (Proc->Features.Power.EAX.HWP_EPP) {		/* EPP mode	*/
	if ((HWP_EPP >= 0) && (HWP_EPP <= 0xff)) {
		Core->PowerThermal.HWP_Request.Energy_Pref = HWP_EPP;
		WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
		RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	}
    } else {					/* EPB fallback mode	*/
	if (Proc->Registration.Driver.CPUfreq) {
		/* Turbo is a function of the Target P-state		*/
		if (!CmpTarget(Core, ValidRatio)) {
			BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		}
	}
    }
  } else {						/* EPB mode	*/
	if (Proc->Registration.Driver.CPUfreq) {
		if (!CmpTarget(Core, ValidRatio)) {
			BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		}
	}
  }
	Aggregate_Ratio(BOOST(TGT)	, GetTarget(Core));
	Aggregate_Ratio(BOOST(HWP_MIN)	, Get_HWP_Min(Core));
	Aggregate_Ratio(BOOST(HWP_MAX)	, Get_HWP_Max(Core));
	Aggregate_Ratio(BOOST(HWP_TGT)	, Get_HWP_Target(Core));
}

void DynamicAcceleration(CORE *Core)				/* Unique */
{
	if (Proc->Features.Power.EAX.TurboIDA) {
		TurboBoost_Technology(	Core,
					Set_Core2_Target,
					Get_Core2_Target,
					Cmp_Core2_Target,
					1 + Proc->Boost[BOOST(MAX)],
					1 + Proc->Boost[BOOST(MAX)] );
	} else {
		BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask, Core->Bind);
	}
}

typedef struct {
	CLOCK_ARG *pClockMod;
	SET_TARGET SetTarget;
	GET_TARGET GetTarget;
} CLOCK_PPC_ARG;

static void ClockMod_PPC_PerCore(void *arg)
{
	CLOCK_PPC_ARG *pClockPPC;
	CORE *Core;
	unsigned int cpu;

	pClockPPC = (CLOCK_PPC_ARG *) arg;
	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	pClockPPC->SetTarget(Core, (pClockPPC->pClockMod->cpu == -1) ?
		Proc->Boost[BOOST(TGT)] + pClockPPC->pClockMod->Offset
	:	pClockPPC->GetTarget(Core) + pClockPPC->pClockMod->Offset);

	WRMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
}

void For_All_PPC_Clock(CLOCK_PPC_ARG *pClockPPC)
{
	unsigned int cpu = Proc->CPU.Count;
  do {
	cpu--;	/* From last AP to BSP */

    if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)
    && ((pClockPPC->pClockMod->cpu == -1)||(pClockPPC->pClockMod->cpu == cpu)))
    {
	smp_call_function_single(cpu, ClockMod_PPC_PerCore, pClockPPC, 1);
    }
  } while (cpu != 0) ;
}

long ClockMod_Core2_PPC(CLOCK_ARG *pClockMod)
{
    if (pClockMod != NULL) {
	if (pClockMod->NC == CLOCK_MOD_TGT)
	{
	CLOCK_PPC_ARG ClockPPC = {
		.pClockMod = pClockMod,
		.SetTarget = Set_Core2_Target,
		.GetTarget = Get_Core2_Target
	};

	For_All_PPC_Clock(&ClockPPC);

	return (2);	/* Report a platform change */
	}
    }
	return (0);
}

long ClockMod_Nehalem_PPC(CLOCK_ARG *pClockMod)
{
    if (pClockMod != NULL) {
	if (pClockMod->NC == CLOCK_MOD_TGT)
	{
	CLOCK_PPC_ARG ClockPPC = {
		.pClockMod = pClockMod,
		.SetTarget = Set_Nehalem_Target,
		.GetTarget = Get_Nehalem_Target
	};

	For_All_PPC_Clock(&ClockPPC);

	return (2);
	}
    }
	return (0);
}

long ClockMod_SandyBridge_PPC(CLOCK_ARG *pClockMod)
{
    if (pClockMod != NULL) {
	if (pClockMod->NC == CLOCK_MOD_TGT)
	{
	CLOCK_PPC_ARG ClockPPC = {
		.pClockMod = pClockMod,
		.SetTarget = Set_SandyBridge_Target,
		.GetTarget = Get_SandyBridge_Target
	};

	For_All_PPC_Clock(&ClockPPC);

	return (2);
	}
    }
	return (0);
}

static void ClockMod_HWP_PerCore(void *arg)
{
	CLOCK_ARG *pClockMod;
	CORE *Core;
	unsigned int cpu;
	unsigned short WrRdHWP = 0;

	pClockMod = (CLOCK_ARG *) arg;
	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

    if (pClockMod->cpu == -1) {
      switch (pClockMod->NC) {
      case CLOCK_MOD_HWP_MIN:
	Core->PowerThermal.HWP_Request.Minimum_Perf=Proc->Boost[BOOST(HWP_MIN)]
							+ pClockMod->Offset;
	WrRdHWP = 1;
	break;
      case CLOCK_MOD_HWP_MAX:
	Core->PowerThermal.HWP_Request.Maximum_Perf=Proc->Boost[BOOST(HWP_MAX)]
							+ pClockMod->Offset;
	WrRdHWP = 1;
	break;
      case CLOCK_MOD_HWP_TGT:
	Core->PowerThermal.HWP_Request.Desired_Perf=Proc->Boost[BOOST(HWP_TGT)]
							+ pClockMod->Offset;
	WrRdHWP = 1;
	break;
      }
    } else {
      switch (pClockMod->NC) {
      case CLOCK_MOD_HWP_MIN:
	Core->PowerThermal.HWP_Request.Minimum_Perf += pClockMod->Offset;
	WrRdHWP = 1;
	break;
      case CLOCK_MOD_HWP_MAX:
	Core->PowerThermal.HWP_Request.Maximum_Perf += pClockMod->Offset;
	WrRdHWP = 1;
	break;
      case CLOCK_MOD_HWP_TGT:
	Core->PowerThermal.HWP_Request.Desired_Perf += pClockMod->Offset;
	WrRdHWP = 1;
	break;
      }
    }
    if (WrRdHWP == 1) {
	WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
    }
}

void For_All_HWP_Clock(CLOCK_ARG *pClockMod)
{
	unsigned int cpu = Proc->CPU.Count;
    do {
	cpu--;	/* From last AP to BSP */

	if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)
	&& ((pClockMod->cpu == -1) || (pClockMod->cpu == cpu)))
	{
	smp_call_function_single(cpu, ClockMod_HWP_PerCore, pClockMod, 1);
	}
    } while (cpu != 0) ;
}

long ClockMod_Intel_HWP(CLOCK_ARG *pClockMod)
{
    if (Proc->Features.HWP_Enable) {
	if (pClockMod != NULL)
	    switch (pClockMod->NC) {
	    case CLOCK_MOD_HWP_MIN:
	    case CLOCK_MOD_HWP_MAX:
	    case CLOCK_MOD_HWP_TGT:
		For_All_HWP_Clock(pClockMod);

		return (2);
	    case CLOCK_MOD_TGT:
		return (ClockMod_SandyBridge_PPC(pClockMod));
	    }
    } else {
	return (ClockMod_SandyBridge_PPC(pClockMod));
    }
	return (0);
}

void Query_AMD_Zen(CORE *Core)					/* Per SMT */
{
	unsigned long long CC6 = 0, PC6 = 0;
	int ToggleFeature;

	/* Read The Hardware Configuration Register. */
	HWCR HwCfgRegister = {.value = 0};
	RDMSR(HwCfgRegister, MSR_K7_HWCR);

	/* Query the SMM. */
	if (HwCfgRegister.Family_17h.SmmLock)
		BITSET_CC(LOCKLESS, Proc->SMM, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->SMM, Core->Bind);

	/* Enable or Disable the Core Performance Boost. */
	switch (TurboBoost_Enable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		HwCfgRegister.Family_17h.CpbDis = !TurboBoost_Enable;
		WRMSR(HwCfgRegister, MSR_K7_HWCR);
		RDMSR(HwCfgRegister, MSR_K7_HWCR);
		break;
	}
	if (!HwCfgRegister.Family_17h.CpbDis)
		BITSET_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);

	BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask, Core->Bind);

	/* Enable or Disable the Core C6 State. Bit[22,14,16] */
	RDMSR64(CC6, MSR_AMD_CC6_F17H_STATUS);
	switch (CC6_Enable) {
	case COREFREQ_TOGGLE_OFF:
		BITCLR(LOCKLESS, CC6, 22);
		BITCLR(LOCKLESS, CC6, 14);
		BITCLR(LOCKLESS, CC6, 16);
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		BITSET(LOCKLESS, CC6, 22);
		BITSET(LOCKLESS, CC6, 14);
		BITSET(LOCKLESS, CC6, 16);
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1) {
		WRMSR64(CC6, MSR_AMD_CC6_F17H_STATUS);
		RDMSR64(CC6, MSR_AMD_CC6_F17H_STATUS);
	}
	if (BITWISEAND(LOCKLESS, CC6, 0x404040LLU) == 0x404040LLU)
		BITSET_CC(LOCKLESS, Proc->CC6, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->CC6, Core->Bind);

	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);

	/* Enable or Disable the Package C6 State. Bit[32] */
	if (Core->Bind == Proc->Service.Core) {
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
		if (BITWISEAND(LOCKLESS, PC6, 0x100000000LLU) == 0x100000000LLU)
			BITSET_CC(LOCKLESS, Proc->PC6, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->PC6, Core->Bind);

		BITSET_CC(LOCKLESS, Proc->PC6_Mask, Core->Bind);
	}
	/* Package C-State: Configuration Control. */
	Core->Query.CfgLock = 1;
	/* Package C-State: I/O MWAIT Redirection. */
	Core->Query.IORedir = 0;

	if (Core->Bind == Proc->Service.Core) {
		PSTATEDEF PstateDef = {.value = 0};
		PSTATECTRL PStateCtl = {.value = 0};
		/* Convert the non-boosted P-state into the Target Ratio */
		RDMSR(PStateCtl, MSR_AMD_PERF_CTL);
		RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + PStateCtl.PstateCmd);
		/* Is this an enabled P-States ?			*/
	    if (PstateDef.Family_17h.PstateEn) {
		unsigned int COF=AMD_Zen_CoreCOF(PstateDef.Family_17h.CpuFid,
						PstateDef.Family_17h.CpuDfsId);

			Proc->Boost[BOOST(TGT)] = COF;
	    }
	}
}

void Query_Intel_C1E(CORE *Core)				/*Per Package*/
{
	if (Core->Bind == Proc->Service.Core) {
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
			BITSET_CC(LOCKLESS, Proc->C1E, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->C1E, Core->Bind);

		BITSET_CC(LOCKLESS, Proc->C1E_Mask, Core->Bind);
	}
}

void Query_AMD_Family_0Fh_C1E(CORE *Core)			/* Per Core */
{
	INT_PENDING_MSG IntPendingMsg = {.value = 0};

	RDMSR(IntPendingMsg, MSR_K8_INT_PENDING_MSG);

	if (IntPendingMsg.C1eOnCmpHalt & !IntPendingMsg.SmiOnCmpHalt)
		BITSET_CC(LOCKLESS, Proc->C1E, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->C1E, Core->Bind);

	BITSET_CC(LOCKLESS, Proc->C1E_Mask, Core->Bind);
}

void ThermalMonitor2_Set(CORE *Core)	/* Intel Core Solo Duo. */
{
	struct SIGNATURE whiteList[] = {
		_Core_Yonah,		/* 06_0E */
		_Core_Conroe,		/* 06_0F */
		_Core_Penryn,		/* 06_17 */
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft,		/* 06_27 */
		_Atom_Clovertrail,	/* 06_35 */
		_Atom_Saltwell,		/* 06_36 */
	};
	int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
	for (id = 0; id < ids; id++) {
		if((whiteList[id].ExtFamily == Proc->Features.Std.EAX.ExtFamily)
		 && (whiteList[id].Family == Proc->Features.Std.EAX.Family)
		 && (whiteList[id].ExtModel == Proc->Features.Std.EAX.ExtModel)
		 && (whiteList[id].Model == Proc->Features.Std.EAX.Model)) {

			if (Core->PowerThermal.TCC_Enable) {
				THERM2_CONTROL Therm2Control = {.value = 0};

				RDMSR(Therm2Control, MSR_THERM2_CTL);

				if (Therm2Control.TM_SELECT) {
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

void ThermalMonitor_Set(CORE *Core)
{
	TJMAX TjMax = {.value = 0};
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	THERM_STATUS ThermStatus = {.value = 0};
	int ClearBit;

	/* Silvermont + Xeon[06_57] + Nehalem + Sandy Bridge & superior arch. */
	RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);

	Core->PowerThermal.Param.Target = TjMax.Core.Target;
	if (Core->PowerThermal.Param.Target == 0)
		Core->PowerThermal.Param.Target = 100; /*TODO: TjMax database.*/

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

	if (Proc->Features.Power.EAX.PTM && (Core->Bind == Proc->Service.Core))
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
		Proc->PowerThermal.Events = (	(ThermStatus.StatusBit
						|ThermStatus.StatusLog) << 0)
					  | (ThermStatus.PROCHOTLog << 1)
					  | (ThermStatus.CriticalTempLog << 2)
					  | (	(ThermStatus.Threshold1Log
						|ThermStatus.Threshold2Log)<< 3)
					  | (ThermStatus.PwrLimitLog << 4);
	}
}

void PowerThermal(CORE *Core)
{
  struct {
	struct SIGNATURE Arch;
	unsigned short	grantPWR_MGMT	:  1-0,
			grantODCM	:  2-1,
			experimental	:  3-2,
			freeToUse	: 16-3;
  } whiteList[] = {
	{_Core_Yonah,		0, 1, 1},
	{_Core_Conroe,		0, 1, 0},
	{_Core_Kentsfield,	0, 1, 1},
	{_Core_Conroe_616,	0, 1, 1},
	{_Core_Penryn,		0, 1, 0},
	{_Core_Dunnington,	0, 1, 1},

	{_Atom_Bonnell ,	0, 1, 1},	/* 06_1C */
	{_Atom_Silvermont,	0, 1, 1},	/* 06_26 */
	{_Atom_Lincroft,	0, 1, 1},	/* 06_27 */
	{_Atom_Clovertrail,	0, 1, 1},	/* 06_35 */
	{_Atom_Saltwell,	0, 1, 1},	/* 06_36 */

	{_Silvermont_637,	0, 1, 1},	/* 06_37 */

	{_Atom_Avoton,		0, 1, 1},	/* 06_4D */
	{_Atom_Airmont ,	0, 0, 1},	/* 06_4C */
	{_Atom_Goldmont,	1, 0, 1},	/* 06_5C */
	{_Atom_Sofia,		0, 1, 1},	/* 06_5D */
	{_Atom_Merrifield,	0, 1, 1},	/* 06_4A */
	{_Atom_Moorefield,	0, 1, 1},	/* 06_5A */

	{_Nehalem_Bloomfield,	1, 1, 0},	/* 06_1A */
	{_Nehalem_Lynnfield,	1, 1, 0},	/* 06_1E */
	{_Nehalem_MB,		1, 1, 0},	/* 06_1F */
	{_Nehalem_EX,		1, 1, 0},	/* 06_2E */

	{_Westmere,		1, 1, 0},	/* 06_25 */
	{_Westmere_EP,		1, 1, 0},	/* 06_2C */
	{_Westmere_EX,		1, 1, 0},	/* 06_2F */

	{_SandyBridge,		1, 1, 0},	/* 06_2A */
	{_SandyBridge_EP,	1, 1, 0},	/* 06_2D */

	{_IvyBridge,		1, 0, 1},	/* 06_3A */
	{_IvyBridge_EP ,	1, 1, 0},	/* 06_3E */

	{_Haswell_DT,		1, 1, 0},	/* 06_3C */
	{_Haswell_EP,		1, 1, 1},	/* 06_3F */
	{_Haswell_ULT,		1, 1, 1},	/* 06_45 */
	{_Haswell_ULX,		1, 1, 1},	/* 06_46 */

	{_Broadwell,		1, 1, 1},	/* 06_3D */
	{_Broadwell_D,		1, 1, 1},	/* 06_56 */
	{_Broadwell_H,		1, 1, 1},	/* 06_47 */
	{_Broadwell_EP ,	1, 1, 1},	/* 06_4F */

	{_Skylake_UY,		1, 1, 1},	/* 06_4E */
	{_Skylake_S,		1, 0, 0},	/* 06_5E */
	{_Skylake_X,		1, 1, 1},	/* 06_55 */

	{_Xeon_Phi,		0, 1, 1},	/* 06_57 */

	{_Kabylake,		1, 0, 1},	/* 06_9E */
	{_Kabylake_UY,		1, 1, 1},	/* 06_8E */

	{_Cannonlake,		1, 1, 1},	/* 06_66 */
	{_Geminilake,		1, 0, 1},	/* 06_7A */
	{_Icelake_UY,		1, 1, 1},	/* 06_7E */
  };
  unsigned int id, ids = sizeof(whiteList) / sizeof(whiteList[0]);
  for (id = 0; id < ids; id++) {
	if((whiteList[id].Arch.ExtFamily == Proc->Features.Std.EAX.ExtFamily)
	 && (whiteList[id].Arch.Family == Proc->Features.Std.EAX.Family)
	 && (whiteList[id].Arch.ExtModel == Proc->Features.Std.EAX.ExtModel)
	 && (whiteList[id].Arch.Model == Proc->Features.Std.EAX.Model)) {
		break;
	}
  }
  if (Proc->Features.Info.LargestStdFunc >= 0x6)
  {
    struct THERMAL_POWER_LEAF Power = {{0}};

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
	   || (whiteList[id].experimental && Proc->Registration.Experimental))
	  {
	    Core->PowerThermal.PwrManagement.Perf_BIAS_Enable=PowerMGMT_Unlock;
	    WRMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
	    RDMSR(Core->PowerThermal.PwrManagement, MSR_MISC_PWR_MGMT);
	  }
	     break;
	}
	if (Core->PowerThermal.PwrManagement.Perf_BIAS_Enable)
		BITSET_CC(LOCKLESS, Proc->PowerMgmt, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->PowerMgmt, Core->Bind);
      } else
		BITCLR_CC(LOCKLESS, Proc->PowerMgmt, Core->Bind);

      if ((PowerPolicy >= 0) && (PowerPolicy <= 15))
      {
	if (!whiteList[id].experimental
	 || (whiteList[id].experimental && Proc->Registration.Experimental))
	{
	Core->PowerThermal.PerfEnergyBias.PowerPolicy = PowerPolicy;
	WRMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	RDMSR(Core->PowerThermal.PerfEnergyBias, MSR_IA32_ENERGY_PERF_BIAS);
	}
      }
    }
    else
	BITCLR_CC(LOCKLESS, Proc->PowerMgmt, Core->Bind);

    if ((Proc->Features.Std.EDX.ACPI == 1)
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
	   || (whiteList[id].experimental && Proc->Registration.Experimental))
	  {
	    WRMSR(ClockModulation, MSR_IA32_THERM_CONTROL);
	    RDMSR(Core->PowerThermal.ClockModulation, MSR_IA32_THERM_CONTROL);
	  }
	}
	Core->PowerThermal.ClockModulation.ECMD = Power.EAX.ECMD;
	if (Core->PowerThermal.ClockModulation.ODCM_Enable)
		BITSET_CC(LOCKLESS, Proc->ODCM, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->ODCM, Core->Bind);
    }
    else {
	BITCLR_CC(LOCKLESS, Proc->ODCM, Core->Bind);
    }
  }
  else {
	BITCLR_CC(LOCKLESS, Proc->PowerMgmt, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->ODCM, Core->Bind);
  }
  BITSET_CC(LOCKLESS, Proc->ODCM_Mask, Core->Bind);
  BITSET_CC(LOCKLESS, Proc->PowerMgmt_Mask, Core->Bind);
}

void Intel_CStatesConfiguration(enum CSTATES_CLASS encoding, CORE *Core)
{
	CSTATE_CONFIG CStateConfig = {.value = 0};
	CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
	int ToggleFeature = 0;

	RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

	switch (C3A_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			CStateConfig.C3autoDemotion = C3A_Enable;
			ToggleFeature = 1;
		break;
	}
	switch (C1A_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			CStateConfig.C1autoDemotion = C1A_Enable;
			ToggleFeature = 1;
		break;
	}
	if ((encoding == CSTATES_SNB) || (encoding == CSTATES_SKL)) {
		switch (C3U_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				CStateConfig.C3undemotion = C3U_Enable;
				ToggleFeature = 1;
			break;
		}
		switch (C1U_Enable) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				CStateConfig.C1undemotion = C1U_Enable;
				ToggleFeature = 1;
			break;
		}
	}
	if (ToggleFeature == 1) {
		WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
	}
	if (CStateConfig.CFG_Lock == 0) {
		ToggleFeature = 0;

		switch (IOMWAIT_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			CStateConfig.IO_MWAIT_Redir = IOMWAIT_Enable;
			ToggleFeature = 1;
			break;
		}

		if (encoding == CSTATES_NHM) {	/* NHM encoding */
			switch (PkgCStateLimit) {
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b100;
				ToggleFeature = 1;
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b011;
				ToggleFeature = 1;
				break;
			case 3:/*Cannot be used to limit package C-State to C3*/
				CStateConfig.Pkg_CStateLimit = 0b010;
				ToggleFeature = 1;
				break;
			case 1:
				CStateConfig.Pkg_CStateLimit = 0b001;
				ToggleFeature = 1;
				break;
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b000;
				ToggleFeature = 1;
				break;
			}
		} else if ((encoding == CSTATES_SNB)	/* SNB encoding */
			|| (encoding == CSTATES_SKL)) { /* SKL encoding */
			switch (PkgCStateLimit) {
			case 8:
				CStateConfig.Pkg_CStateLimit = 0b110;
				ToggleFeature = 1;
				break;
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b100;
				ToggleFeature = 1;
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b011;
				ToggleFeature = 1;
				break;
			case 3:
				CStateConfig.Pkg_CStateLimit = 0b010;
				ToggleFeature = 1;
				break;
			case 2:
				CStateConfig.Pkg_CStateLimit = 0b001;
				ToggleFeature = 1;
				break;
			case 1:
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b000;
				ToggleFeature = 1;
				break;
			}
		} else if (encoding == CSTATES_ULT) {	/* Haswell ULT */
			switch (PkgCStateLimit) {
			case 10:
				CStateConfig.Pkg_CStateLimit = 0b1000;
				ToggleFeature = 1;
				break;
			case 9:
				CStateConfig.Pkg_CStateLimit = 0b0111;
				ToggleFeature = 1;
				break;
			case 8:
				CStateConfig.Pkg_CStateLimit = 0b0110;
				ToggleFeature = 1;
				break;
			case 7:
				CStateConfig.Pkg_CStateLimit = 0b0100;
				ToggleFeature = 1;
				break;
			case 6:
				CStateConfig.Pkg_CStateLimit = 0b0011;
				ToggleFeature = 1;
				break;
			case 3:
				CStateConfig.Pkg_CStateLimit = 0b0010;
				ToggleFeature = 1;
				break;
			case 2:
				CStateConfig.Pkg_CStateLimit = 0b0001;
				ToggleFeature = 1;
				break;
			case 1:
			case 0:
				CStateConfig.Pkg_CStateLimit = 0b0000;
				ToggleFeature = 1;
				break;
			}
		}
		if (ToggleFeature == 1) {
			WRMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
			RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);
		}
	}

	if (CStateConfig.C3autoDemotion)
		BITSET_CC(LOCKLESS, Proc->C3A, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->C3A, Core->Bind);

	if (CStateConfig.C1autoDemotion)
		BITSET_CC(LOCKLESS, Proc->C1A, Core->Bind);
	else
		BITCLR_CC(LOCKLESS, Proc->C1A, Core->Bind);

	Core->Query.CfgLock = CStateConfig.CFG_Lock;
	Core->Query.IORedir = CStateConfig.IO_MWAIT_Redir;

	if (encoding == CSTATES_NHM) {
		switch (CStateConfig.Pkg_CStateLimit & 0x7) {
		case 0b100:
			Core->Query.CStateLimit = 7;
			break;
		case 0b011:
			Core->Query.CStateLimit = 6;
			break;
		case 0b010:
			Core->Query.CStateLimit = 3;
			break;
		case 0b001:
			Core->Query.CStateLimit = 1;
			break;
		case 0b000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}
	} else if ((encoding == CSTATES_SNB) || (encoding == CSTATES_SKL)) {
		if (CStateConfig.C3undemotion)
			BITSET_CC(LOCKLESS, Proc->C3U, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->C3U, Core->Bind);

		if (CStateConfig.C1undemotion)
			BITSET_CC(LOCKLESS, Proc->C1U, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->C1U, Core->Bind);

		switch (CStateConfig.Pkg_CStateLimit & 0x7) {
		case 0b110:
			Core->Query.CStateLimit = 8;
			break;
		case 0b101:
		case 0b100:
			Core->Query.CStateLimit = 7;
			break;
		case 0b011:
			Core->Query.CStateLimit = 6;
			break;
		case 0b010:
			Core->Query.CStateLimit = 3;
			break;
		case 0b001:
			Core->Query.CStateLimit = 2;
			break;
		case 0b000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}
	} else if (encoding == CSTATES_ULT) {
		if (CStateConfig.C3undemotion)
			BITSET_CC(LOCKLESS, Proc->C3U, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->C3U, Core->Bind);

		if (CStateConfig.C1undemotion)
			BITSET_CC(LOCKLESS, Proc->C1U, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->C1U, Core->Bind);

		switch (CStateConfig.Pkg_CStateLimit) {
		case 0b1000:
			Core->Query.CStateLimit = 10;
			break;
		case 0b0111:
			Core->Query.CStateLimit = 9;
			break;
		case 0b0110:
			Core->Query.CStateLimit = 8;
			break;
		case 0b0101:
		case 0b0100:
			Core->Query.CStateLimit = 7;
			break;
		case 0b0011:
			Core->Query.CStateLimit = 6;
			break;
		case 0b0010:
			Core->Query.CStateLimit = 3;
			break;
		case 0b0001:
			Core->Query.CStateLimit = 2;
			break;
		case 0b0000:
		default:
			Core->Query.CStateLimit = 0;
			break;
		}
	}
	BITSET_CC(LOCKLESS, Proc->C3A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask, Core->Bind);


	RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);

/*TODO: Atom (Avoton, Goldmont, Merrifield, Moorefield, SoFIA), Silvermont
	Not Atom Airmont [06_4Ch]

	0b100: C4	[ATOM, SLM]
	0b110: C6	[ATOM, SLM]
	0b111: C7	[ATOM, SLM]
*/

	if (CStateConfig.IO_MWAIT_Redir) {
		switch (CStateIORedir) {
		case 8:
			CState_IO_MWAIT.CStateRange = 0b011;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		case 7:
			CState_IO_MWAIT.CStateRange = 0b010;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		case 6:
			CState_IO_MWAIT.CStateRange = 0b001;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		case 3:
			CState_IO_MWAIT.CStateRange = 0b000;
			WRMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
			break;
		}
		if (CStateIORedir != -1) {
			RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
		}
	}

	switch (CState_IO_MWAIT.CStateRange) {
	case 0b011:
		Core->Query.CStateInclude = 8;
		break;
	case 0b010:
		Core->Query.CStateInclude = 7;
		break;
	case 0b001:
		Core->Query.CStateInclude = 6;
		break;
	case 0b000:
		Core->Query.CStateInclude = 3;
		break;
	default:
		Core->Query.CStateInclude = 0;
		break;
	}
}

void PerCore_AMD_Family_0Fh_PStates(CORE *Core)
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
		} else
			RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		if (loop == 0) {
			break;
		} else
			loop-- ;
	} while (FidVidStatus.FidVidPending == 1) ;
}

void SystemRegisters(CORE *Core)
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
	if (Proc->Features.Info.Vendor.CRC == CRC_INTEL) {
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
		/* Virtualization Technology. */
		if (BITVAL(Core->SystemRegister.EFCR, EXFCR_VMX_IN_SMX)
		  | BITVAL(Core->SystemRegister.EFCR, EXFCR_VMXOUT_SMX))
			BITSET_CC(LOCKLESS, Proc->VM, Core->Bind);
	}
	else if ( (Proc->Features.Info.Vendor.CRC == CRC_AMD)
		||(Proc->Features.Info.Vendor.CRC == CRC_HYGON) )
	{
		RDMSR(Core->SystemRegister.VMCR, MSR_VM_CR);
		/* Secure Virtual Machine. */
		if(!Core->SystemRegister.VMCR.SVME_Disable
		 && Core->SystemRegister.VMCR.SVM_Lock)
			BITSET_CC(LOCKLESS, Proc->VM, Core->Bind);
	}
	BITSET_CC(LOCKLESS, Proc->CR_Mask, Core->Bind);
}

void Intel_Mitigation_Mechanisms(CORE *Core)
{
	SPEC_CTRL Spec_Ctrl = {.value = 0};
	PRED_CMD  Pred_Cmd  = {.value = 0};
	FLUSH_CMD Flush_Cmd = {.value = 0};
	unsigned short WrRdMSR = 0;

	if (Proc->Features.ExtFeature.EDX.IBRS_IBPB_Cap
	 || Proc->Features.ExtFeature.EDX.STIBP_Cap
	 || Proc->Features.ExtFeature.EDX.SSBD_Cap)
	{
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);
	}
	if (Proc->Features.ExtFeature.EDX.IBRS_IBPB_Cap
	&& ((Mech_IBRS == COREFREQ_TOGGLE_OFF)
	 || (Mech_IBRS == COREFREQ_TOGGLE_ON)))
	{
		Spec_Ctrl.IBRS = Mech_IBRS;
		WrRdMSR = 1;
	}
	if (Proc->Features.ExtFeature.EDX.STIBP_Cap
	&& ((Mech_STIBP == COREFREQ_TOGGLE_OFF)
	 || (Mech_STIBP == COREFREQ_TOGGLE_ON)))
	{
		Spec_Ctrl.STIBP = Mech_STIBP;
		WrRdMSR = 1;
	}
	if (Proc->Features.ExtFeature.EDX.SSBD_Cap
	&& ((Mech_SSBD == COREFREQ_TOGGLE_OFF)
	 || (Mech_SSBD == COREFREQ_TOGGLE_ON)))
	{
		Spec_Ctrl.SSBD = Mech_SSBD;
		WrRdMSR = 1;
	}
	if (WrRdMSR == 1)
	{
		WRMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);
	}
	if (Spec_Ctrl.IBRS) {
		BITSET_CC(LOCKLESS, Proc->IBRS, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, Proc->IBRS, Core->Bind);
	}
	if (Spec_Ctrl.STIBP) {
		BITSET_CC(LOCKLESS, Proc->STIBP, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, Proc->STIBP, Core->Bind);
	}
	if (Spec_Ctrl.SSBD) {
		BITSET_CC(LOCKLESS, Proc->SSBD, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, Proc->SSBD, Core->Bind);
	}
	if (Proc->Features.ExtFeature.EDX.IBRS_IBPB_Cap
	&& ((Mech_IBPB == COREFREQ_TOGGLE_OFF)
	 || (Mech_IBPB == COREFREQ_TOGGLE_ON)))
	{
		Pred_Cmd.IBPB = Mech_IBPB;
		WRMSR(Pred_Cmd, MSR_IA32_PRED_CMD);
	}
	if (Proc->Features.ExtFeature.EDX.L1D_FLUSH_Cap
	&& ((Mech_L1D_FLUSH == COREFREQ_TOGGLE_OFF)
	 || (Mech_L1D_FLUSH == COREFREQ_TOGGLE_ON)))
	{
		Flush_Cmd.L1D_FLUSH_CMD = Mech_L1D_FLUSH;
		WRMSR(Flush_Cmd, MSR_IA32_FLUSH_CMD);
	}
	if (Proc->Features.ExtFeature.EDX.IA32_ARCH_CAP)
	{
		ARCH_CAPABILITIES Arch_Cap = {.value = 0};

		RDMSR(Arch_Cap, MSR_IA32_ARCH_CAPABILITIES);

		if (Arch_Cap.RDCL_NO) {
			BITSET_CC(LOCKLESS, Proc->RDCL_NO, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->RDCL_NO, Core->Bind);
		}
		if (Arch_Cap.IBRS_ALL) {
			BITSET_CC(LOCKLESS, Proc->IBRS_ALL, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->IBRS_ALL, Core->Bind);
		}
		if (Arch_Cap.RSBA) {
			BITSET_CC(LOCKLESS, Proc->RSBA, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->RSBA, Core->Bind);
		}
		if (Arch_Cap.L1DFL_VMENTRY_NO) {
			BITSET_CC(LOCKLESS, Proc->L1DFL_VMENTRY_NO,Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->L1DFL_VMENTRY_NO,Core->Bind);
		}
		if (Arch_Cap.SSB_NO) {
			BITSET_CC(LOCKLESS, Proc->SSB_NO, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->SSB_NO, Core->Bind);
		}
		if (Arch_Cap.MDS_NO) {
			BITSET_CC(LOCKLESS, Proc->MDS_NO, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->MDS_NO, Core->Bind);
		}
		if (Arch_Cap.PSCHANGE_MC_NO) {
			BITSET_CC(LOCKLESS, Proc->PSCHANGE_MC_NO, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->PSCHANGE_MC_NO, Core->Bind);
		}
		if (Arch_Cap.TAA_NO) {
			BITSET_CC(LOCKLESS, Proc->TAA_NO, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->TAA_NO, Core->Bind);
		}
	}
	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask, Core->Bind);
}

void Intel_VirtualMachine(CORE *Core)
{
	if (Proc->Features.Std.ECX.VMX) {
		VMX_BASIC VMX_Basic = {.value = 0};
		/* Basic VMX Information. */
		RDMSR(VMX_Basic, MSR_IA32_VMX_BASIC);

		if (VMX_Basic.SMM_DualMon)
			BITSET_CC(LOCKLESS, Proc->SMM, Core->Bind);
		else
			BITCLR_CC(LOCKLESS, Proc->SMM, Core->Bind);
	}
}

void Intel_Microcode(CORE *Core)
{
	MICROCODE_ID Microcode = {.value = 0};

	RDMSR(Microcode, MSR_IA32_UCODE_REV);
	Core->Query.Microcode = Microcode.Signature;
}

void AMD_Microcode(CORE *Core)
{
	unsigned long long value = 0;
	RDMSR64(value, MSR_AMD64_PATCH_LEVEL);
	Core->Query.Microcode = (unsigned int) value;
}

void PerCore_Reset(CORE *Core)
{
	BITCLR_CC(LOCKLESS, Proc->ODCM_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->PowerMgmt_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->TurboBoost_Mask,Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->HWP_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C1E_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C3A_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C1A_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C3U_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C1U_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->CC6_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->PC6_Mask	, Core->Bind);

	BITCLR_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);

	BITCLR_CC(LOCKLESS, Proc->ODCM		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->PowerMgmt	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->SpeedStep	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->TurboBoost	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->HWP		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C1E		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C3A		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C1A		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C3U		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->C1U		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->CC6		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->PC6		, Core->Bind);

	BITCLR_CC(LOCKLESS, Proc->IBRS		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->STIBP 	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->SSBD		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->RDCL_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->IBRS_ALL	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->RSBA		, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->L1DFL_VMENTRY_NO,Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->SSB_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->MDS_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->PSCHANGE_MC_NO, Core->Bind);
	BITCLR_CC(LOCKLESS, Proc->TAA_NO	, Core->Bind);
}

static void PerCore_Intel_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	BITSET_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1E_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->CC6_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask	, Proc->Service.Core);

	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_AuthenticAMD_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	if (Proc->Features.Std.EAX.ExtFamily >= 1) {
		AMD_Microcode(Core);
	}
	Dump_CPUID(Core);

	BITSET_CC(LOCKLESS, Proc->ODCM_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->CC6_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask	, Proc->Service.Core);

	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);
}

static void PerCore_Core2_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);
	DynamicAcceleration(Core);				/* Unique */

	BITSET_CC(LOCKLESS, Proc->C1E_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask, Proc->Service.Core);

	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);

	PowerThermal(Core);				/* Shared | Unique */

	ThermalMonitor_Set(Core);
}

static void PerCore_Nehalem_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

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
				1 + Proc->Boost[BOOST(MAX)],
				1 + Proc->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_NHM, Core);
	}

	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask, Proc->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_SandyBridge_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

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
				Proc->Boost[BOOST(1C)],
				Proc->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SNB, Core);
	}

	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask, Proc->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Haswell_EP_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	if (Proc->Registration.Experimental)
		Intel_Microcode(Core);

	Dump_CPUID(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_SandyBridge_Target,
				Proc->Boost[BOOST(1C)],
				Proc->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SNB, Core);
	}

	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask, Proc->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Haswell_ULT_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

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
				Proc->Boost[BOOST(1C)],
				Proc->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_ULT, Core);
	}

	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask, Proc->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Skylake_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

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
				Proc->Boost[BOOST(1C)],
				Proc->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SKL, Core);
	}

	BITSET_CC(LOCKLESS, Proc->CC6_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask, Proc->Service.Core);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_AMD_Family_0Fh_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	Dump_CPUID(Core);

	Query_AMD_Family_0Fh_C1E(Core);

	BITSET_CC(LOCKLESS, Proc->ODCM_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->CC6_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask	, Proc->Service.Core);

	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);

	PerCore_AMD_Family_0Fh_PStates(Core);
}

static void PerCore_AMD_Family_10h_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	AMD_Microcode(Core);

	Dump_CPUID(Core);

	Query_AMD_Family_0Fh_C1E(Core);

	BITSET_CC(LOCKLESS, Proc->ODCM_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->TurboBoost_Mask,Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->CC6_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PC6_Mask	, Proc->Service.Core);

	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);
}

static void PerCore_AMD_Family_17h_Query(void *arg)
{
	CORE *Core = (CORE*) arg;

	SystemRegisters(Core);

	AMD_Microcode(Core);

	Dump_CPUID(Core);

	BITSET_CC(LOCKLESS, Proc->ODCM_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->PowerMgmt_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->SpeedStep_Mask, Core->Bind);

	BITSET_CC(LOCKLESS, Proc->C3A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1A_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C3U_Mask	, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->C1U_Mask	, Core->Bind);

	BITSET_CC(LOCKLESS, Proc->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, Proc->ARCH_CAP_Mask , Core->Bind);

	Query_AMD_Zen(Core);

	Core->PowerThermal.Param = Arch[Proc->ArchID].Specific[0].Param;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 56)
void Sys_DumpTask(SYSGATE *SysGate)
{
        SysGate->taskCount = 0;
}
#else /* KERNEL_VERSION(3, 10, 56) */
void Sys_DumpTask(SYSGATE *SysGate)
{	/* Source: /include/linux/sched.h */
	struct task_struct *process, *thread;
	int cnt = 0;

	rcu_read_lock();
	for_each_process_thread(process, thread) {
#if defined(CONFIG_SCHED_MUQSS) || defined(CONFIG_SCHED_BMQ)
		SysGate->taskList[cnt].runtime  = tsk_seruntime(thread);
#else
		SysGate->taskList[cnt].runtime  = thread->se.sum_exec_runtime;
#endif /* CONFIG_SCHED_MUQSS */
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
#endif /* CONFIG_SCHED_BMQ */
		memcpy(SysGate->taskList[cnt].comm, thread->comm,TASK_COMM_LEN);

		if (cnt < TASK_LIMIT)
			cnt++;
	}
	rcu_read_unlock();
	SysGate->taskCount = cnt;
}
#endif /* KERNEL_VERSION(3, 10, 56) */

void Sys_MemInfo(SYSGATE *SysGate)
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
	if (Pkg->OS.Gate != NULL) {				\
		Pkg->tickStep--;				\
		if (!Pkg->tickStep) {				\
			Pkg->tickStep = Pkg->tickReset ;	\
			Sys_DumpTask(Pkg->OS.Gate);		\
			Sys_MemInfo(Pkg->OS.Gate);		\
		}						\
	}							\
})

static void InitTimer(void *Cycle_Function)
{
	unsigned int cpu = smp_processor_id();

	if (BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 0) {
		hrtimer_init(	&KPrivate->Join[cpu]->Timer,
				CLOCK_MONOTONIC,
				HRTIMER_MODE_REL_PINNED);

		KPrivate->Join[cpu]->Timer.function = Cycle_Function;
		BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, CREATED);
	}
}

void Controller_Init(void)
{
	CLOCK clock = {.Q = 0, .R = 0, .Hz = 0};
	unsigned int cpu = Proc->CPU.Count, ratio = 0;

	Package_Reset();

	if (Arch[Proc->ArchID].Query != NULL) {
		Arch[Proc->ArchID].Query();
	}
	ratio = Proc->Boost[BOOST(MAX)];

	if (Arch[Proc->ArchID].BaseClock != NULL) {
		clock = Arch[Proc->ArchID].BaseClock(ratio);
	}
	if (clock.Hz == 0) {	/* Fallback @ 100 MHz */
		clock.Q = 100;
		clock.R = 0;
		clock.Hz = 100000000LLU;
	}

	if (Proc->Features.Factory.Freq != 0) {
	    if (ratio == 0) /* Fix ratio. */
		ratio = Proc->Boost[BOOST(MAX)] = Proc->Features.Factory.Freq
						/ clock.Q;
	} else if (ratio > 0) { /* Fix factory frequency (MHz) */
		Proc->Features.Factory.Freq = (ratio * clock.Hz) / 1000000;
	}
	Proc->Features.Factory.Clock.Q  = clock.Q;
	Proc->Features.Factory.Clock.R  = clock.R;
	Proc->Features.Factory.Clock.Hz = clock.Hz;
	Proc->Features.Factory.Ratio	=(Proc->Features.Factory.Freq * 1000000)
					/ Proc->Features.Factory.Clock.Hz;

	if ((AutoClock & 0b01) && (ratio != 0)) {
		struct kmem_cache *hwCache = NULL;
		/* Allocate Cache aligned resources. */
		hwCache = kmem_cache_create(	"CoreFreqCache",
						STRUCT_SIZE, 0,
						SLAB_HWCACHE_ALIGN, NULL);
	    if (hwCache != NULL)
	    {
		do {	/* from last AP to BSP */
			cpu--;

		  if (!BITVAL(KPublic->Core[cpu]->OffLine, OS))
		  {
			COMPUTE_ARG Compute = {
				.TSC = {NULL, NULL},
				.Clock = {.Q = ratio, .R = 0, .Hz = 0}
			};

			Compute.TSC[0] = kmem_cache_alloc(hwCache, GFP_ATOMIC);
		    if (Compute.TSC[0] != NULL)
		    {
			Compute.TSC[1] = kmem_cache_alloc(hwCache, GFP_ATOMIC);
			if (Compute.TSC[1] != NULL)
			{
			KPublic->Core[cpu]->Clock = Compute_Clock(cpu,&Compute);

			/* Release resources. */
			kmem_cache_free(hwCache, Compute.TSC[1]);
			}
			kmem_cache_free(hwCache, Compute.TSC[0]);
		    }
		  }
		} while (cpu != 0) ;

		kmem_cache_destroy(hwCache);
	    }
	}
	/* Launch a high resolution timer for each online CPU. */
	for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		if (!BITVAL(KPublic->Core[cpu]->OffLine, OS)) {
			if (!KPublic->Core[cpu]->Clock.Hz)
				KPublic->Core[cpu]->Clock = clock;

			if (Arch[Proc->ArchID].Timer != NULL)
				Arch[Proc->ArchID].Timer(cpu);
		}
}

void Controller_Start(int wait)
{
	if (Arch[Proc->ArchID].Start != NULL) {
		unsigned int cpu;
		for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    if ((BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 1)
		     && (BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 0))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Start,
						NULL, wait);
	}
}

void Controller_Stop(int wait)
{
	if (Arch[Proc->ArchID].Stop != NULL) {
		unsigned int cpu;
		for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    if ((BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 1)
		     && (BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 1))
			smp_call_function_single(cpu,
						Arch[Proc->ArchID].Stop,
						NULL, wait);
	}
}

void Controller_Exit(void)
{
	unsigned int cpu;

	if (Arch[Proc->ArchID].Exit != NULL)
		Arch[Proc->ArchID].Exit();

	for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, CREATED);
}

void Intel_Core_Counters_Set(CORE *Core)
{
    if (Proc->Features.PerfMon.EAX.Version >= 2) {
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

	if (Proc->Features.PerfMon.EAX.Version >= 3) {
		if (!Proc->Features.HTT_Enable) {
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
	if (Proc->Features.PerfMon.EBX.InstrRetired == 0) {		\
		HWCR HwCfgRegister = {.value = 0};			\
									\
		RDMSR(HwCfgRegister, MSR_K7_HWCR);			\
		Core->SaveArea.Core_HardwareConfiguration = HwCfgRegister;\
		HwCfgRegister.PMU.IRPerfEn = 1;				\
		WRMSR(HwCfgRegister, MSR_K7_HWCR);			\
	}								\
})

#define Uncore_Counters_Set(PMU)					\
({									\
    if (Proc->Features.PerfMon.EAX.Version >= 3)			\
    {									\
	UNCORE_GLOBAL_PERF_CONTROL  Uncore_GlobalPerfControl;		\
	UNCORE_FIXED_PERF_CONTROL   Uncore_FixedPerfControl;		\
	UNCORE_GLOBAL_PERF_STATUS   Uncore_PerfOverflow = {.value = 0}; \
	UNCORE_GLOBAL_PERF_OVF_CTRL Uncore_PerfOvfControl = {.value = 0};\
									\
	RDMSR(Uncore_GlobalPerfControl, MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL);\
	Proc->SaveArea.Uncore_GlobalPerfControl = Uncore_GlobalPerfControl;\
	Uncore_GlobalPerfControl.PMU.EN_FIXED_CTR0  = 1;		\
	WRMSR(Uncore_GlobalPerfControl, MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL);\
									\
	RDMSR(Uncore_FixedPerfControl, MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL);\
	Proc->SaveArea.Uncore_FixedPerfControl = Uncore_FixedPerfControl;\
	Uncore_FixedPerfControl.PMU.EN_CTR0 = 1;			\
	WRMSR(Uncore_FixedPerfControl, MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL);\
									\
	RDMSR(Uncore_PerfOverflow, MSR_##PMU##_UNCORE_PERF_GLOBAL_STATUS);\
	if (Uncore_PerfOverflow.PMU.Overflow_CTR0) {			\
		RDMSR(Uncore_PerfOvfControl, MSR_UNCORE_PERF_GLOBAL_OVF_CTRL);\
		Uncore_PerfOvfControl.Clear_Ovf_CTR0 = 1;		\
		WRMSR(Uncore_PerfOvfControl, MSR_UNCORE_PERF_GLOBAL_OVF_CTRL);\
	}								\
    }									\
})

void Intel_Core_Counters_Clear(CORE *Core)
{
    if (Proc->Features.PerfMon.EAX.Version >= 2) {
	WRMSR(Core->SaveArea.Core_FixedPerfControl,
					MSR_CORE_PERF_FIXED_CTR_CTRL);
	WRMSR(Core->SaveArea.Core_GlobalPerfControl,
					MSR_CORE_PERF_GLOBAL_CTRL);
    }
}

void AMD_Core_Counters_Clear(CORE *Core)
{
	if (Proc->Features.PerfMon.EBX.InstrRetired == 0) {
		WRMSR(Core->SaveArea.Core_HardwareConfiguration, MSR_K7_HWCR);
	}
}

#define Uncore_Counters_Clear(PMU)					\
({									\
    if (Proc->Features.PerfMon.EAX.Version >= 3)			\
    {									\
	WRMSR(Proc->SaveArea.Uncore_FixedPerfControl,			\
				MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL);\
	WRMSR(Proc->SaveArea.Uncore_GlobalPerfControl,			\
				MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL);	\
    }									\
})

#define Counters_Generic(Core, T)					\
({									\
	RDTSC_COUNTERx2(Core->Counter[T].TSC,				\
			MSR_IA32_APERF, Core->Counter[T].C0.UCC,	\
			MSR_IA32_MPERF, Core->Counter[T].C0.URC);	\
	/* Derive C1 */							\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define Counters_Core2(Core, T)						\
({									\
    if (!Proc->Features.AdvPower.EDX.Inv_TSC)				\
 	{								\
	RDTSC_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	}								\
    else								\
	{								\
	RDTSCP_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	}								\
	/* Derive C1 */							\
	Core->Counter[T].C1 =						\
	  (Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?		\
	    Core->Counter[T].TSC - Core->Counter[T].C0.URC		\
	    : 0;							\
})

#define SMT_Counters_Nehalem(Core, T)					\
({									\
	register unsigned long long Cx = 0;				\
									\
	RDTSCP_COUNTERx5(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1 */							\
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
	register unsigned long long Cx = 0;				\
									\
	RDTSCP_COUNTERx6(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_C3_RESIDENCY,Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY,Core->Counter[T].C6,	\
			MSR_CORE_C7_RESIDENCY,Core->Counter[T].C7,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
	/* Derive C1 */							\
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
	RDTSCP_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_AMD_F17H_APERF, Core->Counter[T].C0.UCC,	\
			MSR_AMD_F17H_MPERF, Core->Counter[T].C0.URC,	\
			MSR_AMD_F17H_IRPERF, Core->Counter[T].INST);	\
	/* Derive C1 */							\
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
			Proc->Boost[BOOST(MAX)],			\
			Core->Delta.TSC,				\
			Proc->SleepInterval);				\
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
({	/* Delta of Instructions Retired */				\
	Core->Delta.INST = Core->Counter[1].INST			\
			 - Core->Counter[0].INST;			\
})

#define PKG_Counters_Generic(Core, T)					\
({									\
	Proc->Counter[T].PTSC = Core->Counter[T].TSC;			\
})

#define PKG_Counters_Nehalem(Core, T)					\
({									\
	RDTSCP_COUNTERx4(Proc->Counter[T].PTSC,				\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_NHM_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_SandyBridge(Core, T)				\
({									\
	RDTSCP_COUNTERx5(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_SNB_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_SandyBridge_EP(Core, T)				\
({									\
	RDTSCP_COUNTERx5(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_SNB_EP_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_Haswell_EP(Core, T)				\
({									\
	RDTSCP_COUNTERx5(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_HSW_EP_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);\
})

#define PKG_Counters_Haswell_ULT(Core, T)				\
({									\
	RDTSCP_COUNTERx7(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
			MSR_PKG_C8_RESIDENCY, Proc->Counter[T].PC08,	\
			MSR_PKG_C9_RESIDENCY, Proc->Counter[T].PC09,	\
			MSR_PKG_C10_RESIDENCY,Proc->Counter[T].PC10);	\
})

#define PKG_Counters_Skylake(Core, T)					\
({									\
	RDTSCP_COUNTERx5(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07,	\
	MSR_SKL_UNCORE_PERF_FIXED_CTR0, Proc->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_Skylake_X(Core, T) 				\
({									\
	RDTSCP_COUNTERx4(Proc->Counter[T].PTSC,				\
			MSR_PKG_C2_RESIDENCY, Proc->Counter[T].PC02,	\
			MSR_PKG_C3_RESIDENCY, Proc->Counter[T].PC03,	\
			MSR_PKG_C6_RESIDENCY, Proc->Counter[T].PC06,	\
			MSR_PKG_C7_RESIDENCY, Proc->Counter[T].PC07);	\
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

#define Delta_PWR_ACCU(Pkg, PwrDomain)					\
({									\
	Pkg->Delta.Power.ACCU[PWR_DOMAIN(PwrDomain)] =			\
	(Pkg->Counter[1].Power.ACCU[PWR_DOMAIN(PwrDomain)]		\
	- Pkg->Counter[0].Power.ACCU[PWR_DOMAIN(PwrDomain)]) & 0x7fffffffU;\
})

#define Save_PWR_ACCU(Pkg, PwrDomain)					\
({									\
	Pkg->Counter[0].Power.ACCU[PWR_DOMAIN(PwrDomain)] =		\
		Pkg->Counter[1].Power.ACCU[PWR_DOMAIN(PwrDomain)];	\
})

void Core_Intel_Temp(CORE *Core)
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

void Core_AMD_Family_0Fh_Temp(CORE *Core)
{
	if (Proc->Features.AdvPower.EDX.TTP == 1) {
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

void Core_AMD_Family_15h_Temp(CORE *Core)
{
	TCTL_REGISTER TctlSensor = {0};

	RDPCI(TctlSensor, PCI_AMD_TEMPERATURE_TCTL);
	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (Proc->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		RDPCI(ThermTrip, PCI_AMD_THERMTRIP_STATUS);

		Core->PowerThermal.Events = ThermTrip.SensorTrip << 0;
	}
}

/*TODO: Bulldozer/Excavator [need hardware to test with]
void Core_AMD_Family_15_60h_Temp(CORE *Core)
{
    if (Proc->Registration.Experimental) {
	TCTL_REGISTER TctlSensor = {.value = 0};

	Core_AMD_SMN_Read(Core ,	TctlSensor,
					SMU_AMD_THM_TCTL_REGISTER_F15H,
					SMU_AMD_INDEX_REGISTER_F15H,
					SMU_AMD_DATA_REGISTER_F15H);

	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (Proc->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		WRPCI(	SMU_AMD_THM_TRIP_REGISTER_F15H,
			SMU_AMD_INDEX_REGISTER_F15H);
		RDPCI(ThermTrip, SMU_AMD_DATA_REGISTER_F15H);

		Core->PowerThermal.Events = ThermTrip.SensorTrip << 0;
	}
    }
}
*/

void Core_AMD_Family_17h_Temp(CORE *Core)
{
	TCTL_REGISTER TctlSensor = {.value = 0};
#if defined(CONFIG_AMD_NB) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)) \
 && defined(HWM_CHIPSET) && (HWM_CHIPSET == COMPATIBLE)

    if (KPrivate->ZenIF_dev != NULL)
    {	FEAT_MSG("Building with Kernel amd_smn_read()")
	if (amd_smn_read(amd_pci_dev_to_node_id(KPrivate->ZenIF_dev),
			SMU_AMD_THM_TCTL_REGISTER_F17H, &TctlSensor.value))
	{
		pr_warn("CoreFreq: Failed to read TctlSensor\n");
	}
    } else {
	Core_AMD_SMN_Read(Core ,	TctlSensor,
					SMU_AMD_THM_TCTL_REGISTER_F17H,
					SMU_AMD_INDEX_REGISTER_F17H,
					SMU_AMD_DATA_REGISTER_F17H);
    }
#else /* CONFIG_AMD_NB */
	Core_AMD_SMN_Read(Core ,	TctlSensor,
					SMU_AMD_THM_TCTL_REGISTER_F17H,
					SMU_AMD_INDEX_REGISTER_F17H,
					SMU_AMD_DATA_REGISTER_F17H);
#endif /* CONFIG_AMD_NB */
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

static enum hrtimer_restart Cycle_GenuineIntel(struct hrtimer *pTimer)
{
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	if (!Proc->Features.AdvPower.EDX.Inv_TSC)
		RDTSC64(Core->Overhead.TSC);
	else
		RDTSCP64(Core->Overhead.TSC);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

			Delta_PTSC_OVH(Proc, Core);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Intel_Query(Core);

	Counters_Generic(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_AuthenticAMD(struct hrtimer *pTimer)
{
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	if (!Proc->Features.AdvPower.EDX.Inv_TSC)
		RDTSC64(Core->Overhead.TSC);
	else
		RDTSCP64(Core->Overhead.TSC);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

			Delta_PTSC_OVH(Proc, Core);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_AuthenticAMD_Query(Core);

	if (Core->Bind == Proc->Service.Core) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Core2(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	if (!Proc->Features.AdvPower.EDX.Inv_TSC)
		RDTSC64(Core->Overhead.TSC);
	else
		RDTSCP64(Core->Overhead.TSC);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Core2(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
			break;
		    }

			Delta_PTSC_OVH(Proc, Core);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.CORE.CurrFID;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core->PowerThermal.VID = PerfStatus.CORE.CurrVID;
			break;
		}

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_CORE2_TARGET(Core);

		Delta_INST(Core);

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Core2_Query(Core);

	Intel_Core_Counters_Set(Core);
	Counters_Core2(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_Nehalem(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_Nehalem(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Nehalem(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
		    #if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
			RDSIO(	Core->PowerThermal.VID, HWM_W83627_CPUVCORE,
				HWM_W83627_INDEX_PORT, HWM_W83627_DATA_PORT );
		    #endif
			break;
		    }

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Sys_Tick(Proc);
		} else {
		    #if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
			Core->PowerThermal.VID = 0;
		    #endif
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.NHM.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
		    #if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
			RDSIO(	Core->PowerThermal.VID, HWM_W83627_CPUVCORE,
				HWM_W83627_INDEX_PORT, HWM_W83627_DATA_PORT );
		    #endif
		    }
			break;
		case FORMULA_SCOPE_SMT:
		    #if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
			RDSIO(	Core->PowerThermal.VID, HWM_W83627_CPUVCORE,
				HWM_W83627_INDEX_PORT, HWM_W83627_DATA_PORT );
		    #endif
			break;
		}

		RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_NEHALEM_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Nehalem_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_Nehalem(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_Nehalem(NULL);
		PKG_Counters_Nehalem(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_Nehalem(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
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
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_SandyBridge(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
			break;
		    }

			PWR_ACCU_SandyBridge(Proc, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Delta_PWR_ACCU(Proc, PKG);

			Delta_PWR_ACCU(Proc, CORES);

			Delta_PWR_ACCU(Proc, UNCORE);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Save_PWR_ACCU(Proc, CORES);

			Save_PWR_ACCU(Proc, UNCORE);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_SANDYBRIDGE_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_SandyBridge(NULL);
		PKG_Counters_SandyBridge(Core, 0);
		PWR_ACCU_SandyBridge(Proc, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_SandyBridge(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
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
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_SandyBridge_EP(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
			break;
		    }

			PWR_ACCU_SandyBridge_EP(Proc, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Delta_PWR_ACCU(Proc, PKG);

			Delta_PWR_ACCU(Proc, CORES);

			Delta_PWR_ACCU(Proc, RAM);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Save_PWR_ACCU(Proc, CORES);

			Save_PWR_ACCU(Proc, RAM);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_SANDYBRIDGE_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_SandyBridge_EP(NULL);
		PKG_Counters_SandyBridge_EP(Core, 0);
		PWR_ACCU_SandyBridge_EP(Proc, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_SandyBridge_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_SandyBridge_EP(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Start_Uncore_SandyBridge_EP(void *arg)
{
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;

	RDMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	Proc->SaveArea.Uncore_FixedPerfControl = Uncore_FixedPerfControl;
	Uncore_FixedPerfControl.SNB.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_SNB_EP_PMON_GLOBAL_CTRL);

	Proc->SaveArea.Uncore_PMonGlobalControl = Uncore_PMonGlobalControl;
	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_SNB_EP_PMON_GLOBAL_CTRL);
}

static void Stop_Uncore_SandyBridge_EP(void *arg)
{	/* If fixed counter was disable at entry, force freezing	*/
	if (Proc->SaveArea.Uncore_FixedPerfControl.SNB.EN_CTR0 == 0) {
		Proc->SaveArea.Uncore_PMonGlobalControl.Freeze_All = 1;
	}
	WRMSR(	Proc->SaveArea.Uncore_PMonGlobalControl,
		MSR_SNB_EP_PMON_GLOBAL_CTRL);

	WRMSR(	Proc->SaveArea.Uncore_FixedPerfControl,
		MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);
}


static enum hrtimer_restart Cycle_Haswell_ULT(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Haswell_ULT(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
			break;
		    }

			PWR_ACCU_SandyBridge(Proc, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PC08(Proc);

			Delta_PC09(Proc);

			Delta_PC10(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Delta_PWR_ACCU(Proc, PKG);

			Delta_PWR_ACCU(Proc, CORES);

			Delta_PWR_ACCU(Proc, UNCORE);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PC08(Proc);

			Save_PC09(Proc);

			Save_PC10(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Save_PWR_ACCU(Proc, CORES);

			Save_PWR_ACCU(Proc, UNCORE);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_SANDYBRIDGE_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Haswell_ULT_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_Haswell_ULT(NULL);
		PKG_Counters_Haswell_ULT(Core, 0);
		PWR_ACCU_SandyBridge(Proc, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_Haswell_ULT(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Start_Uncore_Haswell_ULT(void *arg)
{
    if (Proc->Registration.Experimental)
	Uncore_Counters_Set(SNB);
}

static void Stop_Uncore_Haswell_ULT(void *arg)
{
    if (Proc->Registration.Experimental)
	Uncore_Counters_Clear(SNB);
}


static enum hrtimer_restart Cycle_Haswell_EP(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Haswell_EP(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
			break;
		    }

			PWR_ACCU_SandyBridge_EP(Proc, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Delta_PWR_ACCU(Proc, PKG);

			Delta_PWR_ACCU(Proc, CORES);

			Delta_PWR_ACCU(Proc, RAM);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Save_PWR_ACCU(Proc, CORES);

			Save_PWR_ACCU(Proc, RAM);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_SANDYBRIDGE_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_Haswell_EP_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_Haswell_EP(NULL);
		PKG_Counters_Haswell_EP(Core, 0);
		PWR_ACCU_SandyBridge_EP(Proc, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Haswell_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_Haswell_EP(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Start_Uncore_Haswell_EP(void *arg)
{
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;

	RDMSR(Uncore_FixedPerfControl, MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	Proc->SaveArea.Uncore_FixedPerfControl = Uncore_FixedPerfControl;
	Uncore_FixedPerfControl.HSW_EP.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_HSW_EP_PMON_GLOBAL_CTRL);

	Proc->SaveArea.Uncore_PMonGlobalControl = Uncore_PMonGlobalControl;
	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_HSW_EP_PMON_GLOBAL_CTRL);
}

static void Stop_Uncore_Haswell_EP(void *arg)
{
	if (Proc->SaveArea.Uncore_FixedPerfControl.HSW_EP.EN_CTR0 == 0) {
		Proc->SaveArea.Uncore_PMonGlobalControl.Freeze_All = 1;
	}
	WRMSR(	Proc->SaveArea.Uncore_PMonGlobalControl,
		MSR_HSW_EP_PMON_GLOBAL_CTRL);

	WRMSR(	Proc->SaveArea.Uncore_FixedPerfControl,
		MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);
}


static enum hrtimer_restart Cycle_Skylake(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Skylake(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
			break;
		    }

			PWR_ACCU_Skylake(Proc, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Delta_PWR_ACCU(Proc, PKG);

			Delta_PWR_ACCU(Proc, CORES);

			Delta_PWR_ACCU(Proc, UNCORE);

			Delta_PWR_ACCU(Proc, RAM);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Save_PWR_ACCU(Proc, CORES);

			Save_PWR_ACCU(Proc, UNCORE);

			Save_PWR_ACCU(Proc, RAM);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_SANDYBRIDGE_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_Skylake(NULL);
		PKG_Counters_Skylake(Core, 0);
		PWR_ACCU_Skylake(Proc, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_Skylake(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
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
	CORE *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_SandyBridge(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Skylake_X(Core, 1);

			Pkg_Intel_Temp(Proc);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_Intel_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
			Core->PowerThermal.VID = PerfStatus.SNB.CurrVID;
			break;
		    }

			PWR_ACCU_SandyBridge_EP(Proc, 1);

			Delta_PC02(Proc);

			Delta_PC03(Proc);

			Delta_PC06(Proc);

			Delta_PC07(Proc);

			Delta_PTSC_OVH(Proc, Core);

			Delta_UNCORE_FC0(Proc);

			Delta_PWR_ACCU(Proc, PKG);

			Delta_PWR_ACCU(Proc, CORES);

			Delta_PWR_ACCU(Proc, RAM);

			Save_PC02(Proc);

			Save_PC03(Proc);

			Save_PC06(Proc);

			Save_PC07(Proc);

			Save_PTSC(Proc);

			Save_UNCORE_FC0(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Save_PWR_ACCU(Proc, CORES);

			Save_PWR_ACCU(Proc, RAM);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
		Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_Intel_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_Intel_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
		Core->Ratio.Target = GET_SANDYBRIDGE_TARGET(Core);

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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_SandyBridge_Query(Core);

	Intel_Core_Counters_Set(Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == Proc->Service.Core) {
		Start_Uncore_Skylake_X(NULL);
		PKG_Counters_Skylake_X(Core, 0);
		PWR_ACCU_SandyBridge_EP(Proc, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_Skylake_X(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	Intel_Core_Counters_Clear(Core);

	if (Core->Bind == Proc->Service.Core)
		Stop_Uncore_Skylake_X(NULL);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		FIDVID_STATUS FidVidStatus = {.value = 0};

		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		RDMSR(FidVidStatus, MSR_K7_FID_VID_STATUS);

		Core->PowerThermal.VID	= FidVidStatus.CurrVID;
		Core->Ratio.Perf	= 8 + FidVidStatus.CurrFID;

		/* P-States */
		Core->Counter[1].C0.UCC = Core->Counter[0].C0.UCC
					+ Core->Ratio.Perf
					* Core->Clock.Hz;

		Core->Counter[1].C0.URC = Core->Counter[1].C0.UCC;

		Core->Counter[1].TSC	= Core->Counter[0].TSC
				+ (Proc->Boost[BOOST(MAX)] * Core->Clock.Hz);

		/* Derive C1 */
		Core->Counter[1].C1 =
		  (Core->Counter[1].TSC > Core->Counter[1].C0.URC) ?
		    Core->Counter[1].TSC - Core->Counter[1].C0.URC
		    : 0;

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_AMD_Family_0Fh_Temp(Core);
			break;
		    }

			Delta_PTSC(Proc);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		}

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
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
			REL_BCLK(Core->Clock,
				Proc->Boost[BOOST(MAX)],
				Core->Delta.TSC,
				Proc->SleepInterval);

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

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
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_AMD_Family_0Fh_Query(Core);

	if (Core->Bind == Proc->Service.Core) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Start_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_AMD_Family_10h_Query(Core);

	if (Core->Bind == Proc->Service.Core) {
		PKG_Counters_Generic(Core, 0);
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static enum hrtimer_restart Cycle_AMD_Family_15h(struct hrtimer *pTimer)
{
	PSTATESTAT PstateStat;
	PSTATEDEF PstateDef;
	CORE *Core;
	unsigned int pstate;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Counters_Generic(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_AMD_Family_15h_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			/* Read the current P-State number. */
			RDMSR(PstateStat, MSR_AMD_PERF_STATUS);
			/* Offset the P-State base register. */
			pstate = MSR_AMD_PSTATE_DEF_BASE + PstateStat.Current;
			/* Read the voltage ID at the offset */
			RDMSR(PstateDef, pstate);
			Core->PowerThermal.VID = PstateDef.Family_15h.CpuVid;
			break;
		    }

			Delta_PTSC_OVH(Proc, Core);

			Save_PTSC(Proc);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		RDMSR(PstateStat, MSR_AMD_PERF_STATUS);
		pstate = MSR_AMD_PSTATE_DEF_BASE + PstateStat.Current;
		RDMSR(PstateDef, pstate);

		Core->Ratio.Perf = PstateDef.Family_15h.CpuFid;

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if (Core->T.CoreID == 0) {
			Core_AMD_Family_15h_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_AMD_Family_15h_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
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

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

		return (HRTIMER_RESTART);
	} else
		return (HRTIMER_NORESTART);
}

void InitTimer_AMD_Family_15h(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_15h, 1);
}

static enum hrtimer_restart Cycle_AMD_Family_17h(struct hrtimer *pTimer)
{
	CORE *Core;
	PSTATEDEF PstateDef;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE *) KPublic->Core[cpu];

	Mark_OVH(Core);

	if (BITVAL(KPrivate->Join[cpu]->TSM, MUSTFWD) == 1) {
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		SMT_Counters_AMD_Family_17h(Core, 1);

		if (Core->Bind == Proc->Service.Core)
		{
			PKG_Counters_Generic(Core, 1);

		    switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		    case FORMULA_SCOPE_PKG:
			Core_AMD_Family_17h_Temp(Core);
			break;
		    }

		    switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		    case FORMULA_SCOPE_PKG:
			/* Read the boosted voltage VID.		*/
			RDMSR(PstateDef, MSR_AMD_PSTATE_F17H_BOOST);
			Core->PowerThermal.VID = PstateDef.Family_17h.CpuVid;
			break;
		    }

			RDCOUNTER(Proc->Counter[1].Power.ACCU[PWR_DOMAIN(PKG)],
					MSR_AMD_PKG_ENERGY_STATUS);

			Delta_PTSC_OVH(Proc, Core);

			Delta_PWR_ACCU(Proc, PKG);

			Save_PTSC(Proc);

			Save_PWR_ACCU(Proc, PKG);

			Sys_Tick(Proc);
		} else {
			Core->PowerThermal.VID = 0;
		}

		/* Read the boosted frequency FID.			*/
		RDMSR(PstateDef, MSR_AMD_PSTATE_F17H_BOOST);
		Core->Ratio.Perf=AMD_Zen_CoreCOF(PstateDef.Family_17h.CpuFid,
						PstateDef.Family_17h.CpuDfsId);

		switch (SCOPE_OF_FORMULA(Proc->thermalFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core_AMD_Family_17h_Temp(Core);
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core_AMD_Family_17h_Temp(Core);
			break;
		}

		switch (SCOPE_OF_FORMULA(Proc->voltageFormula)) {
		case FORMULA_SCOPE_CORE:
		    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
			Core->PowerThermal.VID = PstateDef.Family_17h.CpuVid;
		    }
			break;
		case FORMULA_SCOPE_SMT:
			Core->PowerThermal.VID = PstateDef.Family_17h.CpuVid;
			break;
		}

		/* Read the Physical Core RAPL counter. */
	    if (Core->T.ThreadID == 0)
	    {
		RDCOUNTER(Core->Counter[1].Power.ACCU,MSR_AMD_PP0_ENERGY_STATUS);
		Core->Counter[1].Power.ACCU &= 0xffffffff;

		Core->Delta.Power.ACCU  = Core->Counter[1].Power.ACCU
					- Core->Counter[0].Power.ACCU;

		Core->Counter[0].Power.ACCU = Core->Counter[1].Power.ACCU;
	    }
		Delta_INST(Core);

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_INST(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, Core->Sync.V, NTFY);

		return (HRTIMER_RESTART);
	} else
		return (HRTIMER_NORESTART);
}

void InitTimer_AMD_Family_17h(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_17h, 1);
}

static void Start_AMD_Family_17h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	PerCore_AMD_Family_17h_Query(Core);

	AMD_Core_Counters_Set(Core, Family_17h);
	SMT_Counters_AMD_Family_17h(Core, 0);

	if (Core->Bind == Proc->Service.Core)
	{
		PKG_Counters_Generic(Core, 0);

		RDCOUNTER(Proc->Counter[0].Power.ACCU[PWR_DOMAIN(PKG)],
				MSR_AMD_PKG_ENERGY_STATUS );
	}
	if (Core->T.ThreadID == 0)
	{
		RDCOUNTER(Core->Counter[0].Power.ACCU,MSR_AMD_PP0_ENERGY_STATUS);
		Core->Counter[0].Power.ACCU &= 0xffffffff;
	}

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_start(	&KPrivate->Join[cpu]->Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

static void Stop_AMD_Family_17h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, MUSTFWD);

	hrtimer_cancel(&KPrivate->Join[cpu]->Timer);

	AMD_Core_Counters_Clear(Core);

	PerCore_Reset(Core);

	BITCLR(LOCKLESS, KPrivate->Join[cpu]->TSM, STARTED);
}

long Sys_OS_Driver_Query(SYSGATE *SysGate)
{
    if (SysGate != NULL) {
	int rc;
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
	if ((rc = cpufreq_get_policy(&freqPolicy, Proc->Service.Core)) == 0)
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
	return (0);
    }
    else
	return (-1);
}

long Sys_Kernel(SYSGATE *SysGate)
{	/* Sources:	/include/generated/uapi/linux/version.h
			/include/uapi/linux/utsname.h		*/
	if (SysGate != NULL) {
		SysGate->kernelVersionNumber = LINUX_VERSION_CODE;
		memcpy(SysGate->sysname, utsname()->sysname, MAX_UTS_LEN);
		memcpy(SysGate->release, utsname()->release, MAX_UTS_LEN);
		memcpy(SysGate->version, utsname()->version, MAX_UTS_LEN);
		memcpy(SysGate->machine, utsname()->machine, MAX_UTS_LEN);

		return (0);
	}
	else
		return (-1);
}

long SysGate_OnDemand(void)
{
	long rc = -1;
	if (Proc->OS.Gate == NULL) {			/* Alloc on demand */
	    Proc->OS.Gate = alloc_pages_exact(Proc->OS.ReqMem.Size, GFP_KERNEL);
	    if (Proc->OS.Gate != NULL) {
		const size_t allocPages = PAGE_SIZE << Proc->OS.ReqMem.Order;
		memset(Proc->OS.Gate, 0, allocPages);
		rc = 0;
	    }
	}
	else						/* Already allocated */
		rc = 1;

	return (rc);
}

#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static int CoreFreqK_IdleHandler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{/* Source: /drivers/cpuidle/cpuidle.c					*/
	unsigned long MWAIT=(CoreFreqK.IdleDriver.states[index].flags>>24)&0xff;
	mwait_idle_with_hints(MWAIT, 1UL);
	return index;
}

static void CoreFreqK_S2IdleHandler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	unsigned long MWAIT=(CoreFreqK.IdleDriver.states[index].flags>>24)&0xff;
	mwait_idle_with_hints(MWAIT, 1UL);
}
#endif /* CONFIG_CPU_IDLE */

static void CoreFreqK_IdleDriver_UnInit(void)
{
#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	struct cpuidle_device *device;
	unsigned int cpu;

	for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
	    if (!BITVAL(KPublic->Core[cpu]->OffLine, HW)) {
		if ((device=per_cpu_ptr(CoreFreqK.IdleDevice, cpu)) != NULL) {
			cpuidle_unregister_device(device);
		}
	    }
	}
	cpuidle_unregister_driver(&CoreFreqK.IdleDriver);
	free_percpu(CoreFreqK.IdleDevice);
#endif /* CONFIG_CPU_IDLE */
}

static int CoreFreqK_IdleDriver_Init(void)
{
	int rc = -EPERM;
#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
  if (Arch[Proc->ArchID].SystemDriver != NULL)
  {
	IDLE_STATE *pIdleState = Arch[Proc->ArchID].SystemDriver->IdleState;
    if ((pIdleState != NULL) && Proc->Features.Std.ECX.MONITOR)
    {
	if((CoreFreqK.IdleDevice = alloc_percpu(struct cpuidle_device)) == NULL)
		rc = -ENOMEM;
	else {
		struct cpuidle_device *device;
		unsigned int cpu, enroll = 0;
		unsigned int subState[] = {
			Proc->Features.MWait.EDX.SubCstate_MWAIT0,  /*   C0  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT1,  /*   C1  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT1,  /*  C1E  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT2,  /*   C3  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT3,  /*   C6  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT4,  /*   C7  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT5,  /*   C8  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT6,  /*   C9  */
			Proc->Features.MWait.EDX.SubCstate_MWAIT7   /*  C10  */
		};
		const unsigned int subStateCount = sizeof(subState)
						 / sizeof(subState[0]);
		/* Kernel polling loop					*/
		cpuidle_poll_state_init(&CoreFreqK.IdleDriver);

		CoreFreqK.IdleDriver.state_count = 1;
		/* Idle States						*/
	    while (pIdleState->Name != NULL)
	    {
		if ((CoreFreqK.IdleDriver.state_count < subStateCount)
		&& (subState[CoreFreqK.IdleDriver.state_count] > 0))
		{
			StrCopy(CoreFreqK.IdleDriver.states[
					CoreFreqK.IdleDriver.state_count
				].name, pIdleState->Name, CPUIDLE_NAME_LEN);

			StrCopy(CoreFreqK.IdleDriver.states[
					CoreFreqK.IdleDriver.state_count
				].desc, pIdleState->Desc, CPUIDLE_NAME_LEN);

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].flags = pIdleState->flags;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].exit_latency = pIdleState->Latency;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].target_residency = pIdleState->Residency;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_IdleHandler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_S2IdleHandler;

			CoreFreqK.IdleDriver.state_count++;
		}
		pIdleState++;
	    }
	    if ((rc = cpuidle_register_driver(&CoreFreqK.IdleDriver)) == 0) {
		for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
		    if (!BITVAL(KPublic->Core[cpu]->OffLine, HW)) {
			device = per_cpu_ptr(CoreFreqK.IdleDevice, cpu);
		      if (device != NULL) {
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
		for (cpu = 0; cpu < enroll; cpu++) {
			device = per_cpu_ptr(CoreFreqK.IdleDevice, cpu);
		    if (device != NULL) {
			cpuidle_unregister_device(device);
		    }
		}
		cpuidle_unregister_driver(&CoreFreqK.IdleDriver);
		free_percpu(CoreFreqK.IdleDevice);
	    }
	}
    }
  }
#endif /* CONFIG_CPU_IDLE */
	return (rc);
}

#ifdef CONFIG_CPU_IDLE
static void CoreFreqK_Idle_State_Withdraw(int idx, bool disable)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
	struct cpuidle_device *device;
	unsigned int cpu;
  for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
  {
    if (!BITVAL(KPublic->Core[cpu]->OffLine, HW)
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
#endif
}
#endif

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
	rc = 0;
    }
    else if (target == 0)
    {
	for (idx = 0; idx < CoreFreqK.IdleDriver.state_count; idx++)
	{
		CoreFreqK_Idle_State_Withdraw(idx, false);

		floor = idx;
	}
	rc = 0;
    }
    if ((Proc->OS.Gate != NULL) && (floor != -1)) {
	Proc->OS.Gate->OS.IdleDriver.stateLimit = 1 + floor;
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
	if (policy->cpu < Proc->CPU.Count)
	{
		CORE *Core = (CORE *) KPublic->Core[policy->cpu];

		policy->cpuinfo.min_freq =(Proc->Boost[BOOST(MIN)]
					 * Core->Clock.Hz) / 1000LLU;

		policy->cpuinfo.max_freq =(Proc->Boost[BOOST(MAX)]
					 * Core->Clock.Hz) / 1000LLU;
		/* MANDATORY Per-CPU Initialization			*/
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
  && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 4, 25)))	\
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
    if ((cpu >= 0) && (cpu < Proc->CPU.Count) && (limit != NULL))
    {
	CORE *Core = (CORE *) KPublic->Core[cpu];

	(*limit) = (Proc->Boost[BOOST(MAX)] * Core->Clock.Hz) / 1000LLU;
    }
	return (0);
}

void Policy_Aggregate_Turbo(void)
{
    if (Proc->Registration.Driver.CPUfreq == 1) {
	CoreFreqK.FreqDriver.boost_enabled = (
				BITWISEAND_CC(	LOCKLESS,
						Proc->TurboBoost,
						Proc->TurboBoost_Mask ) != 0 );
    }
}

static int CoreFreqK_SetBoost(int state)
{
	Controller_Stop(1);
	TurboBoost_Enable = (state != 0);
	Aggregate_Reset(BOOST(TGT), Proc->Boost[BOOST(MIN)]);
	Aggregate_Reset(BOOST(HWP_MIN), 0);
	Aggregate_Reset(BOOST(HWP_MAX), 0);
	Aggregate_Reset(BOOST(HWP_TGT), 0);
	Controller_Start(1);
	TurboBoost_Enable = -1;
	Policy_Aggregate_Turbo();
	BITSET(BUS_LOCK, Proc->OS.Signal, NTFY); /* Notify Daemon	*/
	return (0);
}

static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy *policy,char *buf)
{
  if (policy != NULL) {
	CORE *Core;
    if (policy->cpu < Proc->CPU.Count)
    {
	Core = (CORE *) KPublic->Core[policy->cpu];
    } else {
	Core = (CORE *) KPublic->Core[Proc->Service.Core];
    }
	return ( sprintf(buf, "%7llu\n",
			(Core->Ratio.Target * Core->Clock.Hz) / 1000LLU) );
  }
	return (0);
}

static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy *policy,
					unsigned int freq)
{
    if (policy != NULL) {
	if ((policy->cpu < Proc->CPU.Count)
	 && (Arch[Proc->ArchID].SystemDriver->SetTarget != NULL))
	{
		CORE *Core = (CORE *) KPublic->Core[policy->cpu];
		unsigned int ratio = (freq * 1000LLU) / Core->Clock.Hz;

	    if (ratio > 0) {
		if (smp_call_function_single(policy->cpu,
				Arch[Proc->ArchID].SystemDriver->SetTarget,
				&ratio, 1) == 0)
		{
			BITSET(BUS_LOCK, Proc->OS.Signal, NTFY);
		}
	    }
		return (0);
	}
    }
	return (-EINVAL);
}

static unsigned int Policy_Intel_GetFreq(unsigned int cpu)
{
	unsigned int CPU_Freq = Proc->Features.Factory.Freq * 1000U;

    if (cpu < Proc->CPU.Count)
    {
	CORE *Core = (CORE *) KPublic->Core[cpu];

	unsigned int Core_Freq = Core->Delta.C0.UCC / Proc->SleepInterval;
	if (Core_Freq > 0) {	/* at least 1 interval must have elapsed */
		CPU_Freq = Core_Freq;
	}
    }
	return (CPU_Freq);
}

void For_All_Policy_Aggregate_Ratio( enum RATIO_BOOST boost,
					unsigned int ratio,
					unsigned int reset,
					GET_TARGET GetTarget )
{
	if (ratio > Proc->Boost[boost]) {
		Proc->Boost[boost] = ratio;
	} else {
		unsigned int cpu;

		Aggregate_Reset(boost, reset);

		for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
			CORE *Core = (CORE *) KPublic->Core[cpu];

			if (!BITVAL(Core->OffLine, OS)) {
				Aggregate_Ratio(boost, GetTarget(Core));
			}
		}
	}
}

void For_All_Policy_Aggregate_HWP(	unsigned int count,
					enum RATIO_BOOST boost[],
					unsigned int ratio[],
					unsigned int reset[],
					GET_TARGET GetTarget[] )
{
	unsigned int idx, cpu;
	for (idx = 0; idx < count; idx++) {
		Aggregate_Reset(boost[idx], reset[idx]);
	}
	for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
		CORE *Core = (CORE *) KPublic->Core[cpu];

	    if (!BITVAL(Core->OffLine, OS)) {
		for (idx = 0; idx < count; idx++) {
			Aggregate_Ratio(boost[idx], GetTarget[idx](Core));
		}
	    }
	}
}
#endif /* CONFIG_CPU_FREQ */

static void Policy_Core2_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
    if ((*ratio) <= Proc->Boost[BOOST(1C)])
    {
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Set_Core2_Target(Core, (*ratio));
	WRMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	if (Proc->Features.Power.EAX.TurboIDA) {
		if (Cmp_Core2_Target(Core, 1 + Proc->Boost[BOOST(MAX)])) {
			BITSET_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		}
	}
	For_All_Policy_Aggregate_Ratio( BOOST(TGT),
					(*ratio),
					Proc->Boost[BOOST(MIN)],
					Get_Core2_Target );
    }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_Nehalem_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
    if ((*ratio) <= Proc->Boost[BOOST(1C)])
    {
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Set_Nehalem_Target(Core, (*ratio));
	WRMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	if (Proc->Features.Power.EAX.TurboIDA) {
		if (Cmp_Nehalem_Target(Core, 1 + Proc->Boost[BOOST(MAX)])) {
			BITSET_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		}
	}
	For_All_Policy_Aggregate_Ratio( BOOST(TGT),
					(*ratio),
					Proc->Boost[BOOST(MIN)],
					Get_Nehalem_Target );
    }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_SandyBridge_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
	unsigned int *ratio = (unsigned int*) arg;
    if ((*ratio) <= Proc->Boost[BOOST(1C)])
    {
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Set_SandyBridge_Target(Core, (*ratio));
	WRMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

	if (Proc->Features.Power.EAX.TurboIDA) {
		if (Cmp_SandyBridge_Target(Core, Proc->Boost[BOOST(MAX)])) {
			BITSET_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, Proc->TurboBoost, Core->Bind);
		}
	}
	For_All_Policy_Aggregate_Ratio( BOOST(TGT),
					(*ratio),
					Proc->Boost[BOOST(MIN)],
					Get_SandyBridge_Target );
    }
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_HWP_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
    if (Proc->Features.HWP_Enable)
    {
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE *Core = (CORE *) KPublic->Core[cpu];

	RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
	if (((*ratio) >= Core->PowerThermal.HWP_Capabilities.Lowest)
	 && ((*ratio) <= Core->PowerThermal.HWP_Capabilities.Highest))
	{
		enum RATIO_BOOST boost[]= { BOOST(HWP_MAX), BOOST(HWP_TGT) };
		unsigned int ratios[]	= { (*ratio), (*ratio) };
		unsigned int reset[]	= { 0, 0 };
		GET_TARGET GetFunc[]	= { Get_HWP_Max, Get_HWP_Target };

		Core->PowerThermal.HWP_Request.Maximum_Perf =	\
		Core->PowerThermal.HWP_Request.Desired_Perf = (*ratio);
		WRMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);
		RDMSR(Core->PowerThermal.HWP_Request, MSR_IA32_HWP_REQUEST);

		For_All_Policy_Aggregate_HWP(2, boost, ratios, reset, GetFunc);
	}
    }
    else {
	Policy_SandyBridge_SetTarget(arg);
    }
#endif /* CONFIG_CPU_FREQ */
}

static void CoreFreqK_FreqDriver_UnInit(void)
{
#ifdef CONFIG_CPU_FREQ
	cpufreq_unregister_driver(&CoreFreqK.FreqDriver);
#endif /* CONFIG_CPU_FREQ */
}

static int CoreFreqK_FreqDriver_Init(void)
{
	int rc = -EPERM;
#ifdef CONFIG_CPU_FREQ
  if (Arch[Proc->ArchID].SystemDriver != NULL)
  {
    if (Arch[Proc->ArchID].SystemDriver->GetFreq != NULL)
    {
	CoreFreqK.FreqDriver.get = Arch[Proc->ArchID].SystemDriver->GetFreq;

	rc = cpufreq_register_driver(&CoreFreqK.FreqDriver);
    }
  }
#endif /* CONFIG_CPU_FREQ */
	return (rc);
}

static int CoreFreqK_Governor_Init(void)
{
	int rc = -EPERM;
#ifdef CONFIG_CPU_FREQ
    if (Arch[Proc->ArchID].SystemDriver != NULL)
    {
	if (Arch[Proc->ArchID].SystemDriver->SetTarget != NULL)
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

    for (seek = 0; seek < Proc->CPU.Count; seek++) {
	if(((exclude ^ cpu) > 0)
	&& (KPublic->Core[seek]->T.ApicID != KPublic->Core[cpu]->T.ApicID)
	&& (KPublic->Core[seek]->T.CoreID == KPublic->Core[cpu]->T.CoreID)
	&& (KPublic->Core[seek]->T.ThreadID != KPublic->Core[cpu]->T.ThreadID)
	&& (KPublic->Core[seek]->T.PackageID == KPublic->Core[cpu]->T.PackageID)
	&& (KPublic->Core[seek]->T.ThreadID == 0)
	&& !BITVAL(KPublic->Core[seek]->OffLine, OS)) {
		return ((signed int) seek);
	}
    }
	return (-1);
}

signed int Seek_Topology_Thread_Peer(unsigned int cpu, signed int exclude)
{
	unsigned int seek;

    for (seek = 0; seek < Proc->CPU.Count; seek++) {
	if(((exclude ^ cpu) > 0)
	&& (KPublic->Core[seek]->T.ApicID != KPublic->Core[cpu]->T.ApicID)
	&& (KPublic->Core[seek]->T.CoreID == KPublic->Core[cpu]->T.CoreID)
	&& (KPublic->Core[seek]->T.ThreadID != KPublic->Core[cpu]->T.ThreadID)
	&& (KPublic->Core[seek]->T.PackageID == KPublic->Core[cpu]->T.PackageID)
	&& (KPublic->Core[seek]->T.ThreadID > 0)
	&& !BITVAL(KPublic->Core[seek]->OffLine, OS)) {
		return ((signed int) seek);
	}
    }
	return (-1);
}

void MatchCoreForService(SERVICE_PROC *pService,unsigned int cpi,signed int cpx)
{
	unsigned int cpu;

    for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
	if(((cpx ^ cpu) > 0)
	&& (KPublic->Core[cpu]->T.PackageID == KPublic->Core[cpi]->T.PackageID)
	&& !BITVAL(KPublic->Core[cpu]->OffLine, OS))
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
	if (KPublic->Core[cpu]->T.ThreadID == 0)
	{
		if ((seek = Seek_Topology_Thread_Peer(cpu, cpx)) != -1) {
			pService->Core = cpu;
			pService->Thread = seek;
			return (0);
		}
	}
	else if (KPublic->Core[cpu]->T.ThreadID > 0)
	{
		if ((seek = Seek_Topology_Core_Peer(cpu, cpx)) != -1) {
			pService->Core = seek;
			pService->Thread = cpu;
			return (0);
		}
	}
	while (cpn < Proc->CPU.Count) {
		cpu = cpn++;
		if (!BITVAL(KPublic->Core[cpu]->OffLine, OS))
			goto MATCH;
	}
	return (-1);
}

void MatchPeerForDefaultService(SERVICE_PROC *pService, unsigned int cpu)
{
	if (Proc->Features.HTT_Enable) {
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
	if (Proc->Features.HTT_Enable)
	{
		signed int seek;

		if ((KPublic->Core[cpu]->T.ThreadID == 0)
		&& ((seek = Seek_Topology_Thread_Peer(cpu, -1)) != -1))
		{
			hService.Core = cpu;
			hService.Thread = seek;
		} else {
			if ((KPublic->Core[cpu]->T.ThreadID > 0)
			&& ((seek = Seek_Topology_Core_Peer(cpu, -1)) != -1))
			{
				hService.Core = seek;
				hService.Thread = cpu;
			}
		}
	}
	if (pService->Proc != DefaultSMT.Proc)
	{
		if (hService.Proc == DefaultSMT.Proc)
			pService->Proc = hService.Proc;
		else
			if ((pService->Thread == -1) && (hService.Thread > 0))
			{
				pService->Proc = hService.Proc;
			}
	}
}

void MatchPeerForDownService(SERVICE_PROC *pService, unsigned int cpu)
{
	int rc = -1;

	if (Proc->Features.HTT_Enable)
		rc = MatchPeerForService(pService, cpu, cpu);
	if (rc == -1)
		MatchCoreForService(pService, cpu, cpu);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
static int CoreFreqK_NMI_Handler(unsigned int type, struct pt_regs *pRegs)
{
	unsigned int cpu = smp_processor_id();

	switch (type) {
	case NMI_LOCAL:
		KPublic->Core[cpu]->Interrupt.NMI.LOCAL++;
		break;
	case NMI_UNKNOWN:
		KPublic->Core[cpu]->Interrupt.NMI.UNKNOWN++;
		break;
	case NMI_SERR:
		KPublic->Core[cpu]->Interrupt.NMI.PCISERR++;
		break;
	case NMI_IO_CHECK:
		KPublic->Core[cpu]->Interrupt.NMI.IOCHECK++;
		break;
	}
	return (NMI_DONE);
}

static void CoreFreqK_Register_NMI(void)
{
    if (BITVAL(Proc->Registration.NMI, BIT_NMI_LOCAL) == 0)
    {
	if(register_nmi_handler(NMI_LOCAL,
				CoreFreqK_NMI_Handler,
				0,
				"corefreqk") == 0)
	{
		BITSET(LOCKLESS, Proc->Registration.NMI, BIT_NMI_LOCAL);
	} else {
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_LOCAL);
	}
    }
    if (BITVAL(Proc->Registration.NMI, BIT_NMI_UNKNOWN) == 0)
    {
	if(register_nmi_handler(NMI_UNKNOWN,
				CoreFreqK_NMI_Handler,
				0,
				"corefreqk") == 0)
	{
		BITSET(LOCKLESS, Proc->Registration.NMI, BIT_NMI_UNKNOWN);
	} else {
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_UNKNOWN);
	}
    }
    if (BITVAL(Proc->Registration.NMI, BIT_NMI_SERR) == 0)
    {
	if(register_nmi_handler(NMI_SERR,
				CoreFreqK_NMI_Handler,
				0,
				"corefreqk") == 0)
	{
		BITSET(LOCKLESS, Proc->Registration.NMI, BIT_NMI_SERR);
	} else {
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_SERR);
	}
    }
    if (BITVAL(Proc->Registration.NMI, BIT_NMI_IO_CHECK) == 0)
    {
	if(register_nmi_handler(NMI_IO_CHECK,
				CoreFreqK_NMI_Handler,
				0,
				"corefreqk") == 0)
	{
		BITSET(LOCKLESS, Proc->Registration.NMI, BIT_NMI_IO_CHECK);
	} else {
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_IO_CHECK);
	}
    }
}

static void CoreFreqK_UnRegister_NMI(void)
{
	if (BITVAL(Proc->Registration.NMI, BIT_NMI_LOCAL) == 1)
	{
		unregister_nmi_handler(NMI_LOCAL, "corefreqk");
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_LOCAL);
	}
	if (BITVAL(Proc->Registration.NMI, BIT_NMI_UNKNOWN) == 1)
	{
		unregister_nmi_handler(NMI_UNKNOWN, "corefreqk");
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_UNKNOWN);
	}
	if (BITVAL(Proc->Registration.NMI, BIT_NMI_SERR) == 1)
	{
		unregister_nmi_handler(NMI_SERR, "corefreqk");
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_SERR);
	}
	if (BITVAL(Proc->Registration.NMI, BIT_NMI_IO_CHECK) == 1)
	{
		unregister_nmi_handler(NMI_IO_CHECK, "corefreqk");
		BITCLR(LOCKLESS, Proc->Registration.NMI, BIT_NMI_IO_CHECK);
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
	Proc->thermalFormula=(KIND_OF_FORMULA(Proc->thermalFormula)<<8)|scope;
	return (0);
    } else {
	return (-EINVAL);
    }
}

static long CoreFreqK_Voltage_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	Proc->voltageFormula=(KIND_OF_FORMULA(Proc->voltageFormula)<<8)|scope;
	return (0);
} else {
	return (-EINVAL);
}
}

static long CoreFreqK_Power_Scope(int scope)
{
    if ((scope >= FORMULA_SCOPE_NONE) && (scope <= FORMULA_SCOPE_PKG))
    {
	Proc->powerFormula = (KIND_OF_FORMULA(Proc->powerFormula) << 8)|scope;
	return (0);
} else {
	return (-EINVAL);
}
}

static void Compute_Clock_SMT(void)
{
	unsigned int cpu = Proc->CPU.Count;
  do {
	CLOCK Clock = {
		.Q  = Proc->Features.Factory.Clock.Q,
		.R  = Proc->Features.Factory.Clock.R,
		.Hz = Proc->Features.Factory.Clock.Hz
	};
	/* from last AP to BSP */
	cpu--;

    if (!BITVAL(KPublic->Core[cpu]->OffLine, OS))
    {
	COMPUTE_ARG Compute = {
		.TSC = {NULL, NULL},
		.Clock = {.Q = Proc->Boost[BOOST(MAX)], .R = 0, .Hz = 0}
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
	KPublic->Core[cpu]->Clock.Q  = Clock.Q;
	KPublic->Core[cpu]->Clock.R  = Clock.R;
	KPublic->Core[cpu]->Clock.Hz = Clock.Hz;
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
	rc = Sys_OS_Driver_Query(Proc->OS.Gate);
	rc = rc == 0 ? 1 : rc;
    break;

    case COREFREQ_IOCTL_SYSONCE:
	rc = Sys_OS_Driver_Query(Proc->OS.Gate)
	   & Sys_Kernel(Proc->OS.Gate);
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
		rc = 0;
		break;
	case COREFREQ_TOGGLE_ON:
		Aggregate_Reset(BOOST(TGT), Proc->Boost[BOOST(MIN)]);
		Aggregate_Reset(BOOST(HWP_MIN), 0);
		Aggregate_Reset(BOOST(HWP_MAX), 0);
		Aggregate_Reset(BOOST(HWP_TGT), 0);
		Controller_Start(1);
	#ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	#endif /* CONFIG_CPU_FREQ */
		rc = 2;
		break;
	}
	break;

      case MACHINE_INTERVAL:
	Controller_Stop(1);
	SleepInterval = prm.dl.lo;
	Compute_Interval();
	Controller_Start(1);
	rc = 0;
	break;

      case MACHINE_AUTOCLOCK:
	switch (prm.dl.lo)
	{
	case COREFREQ_TOGGLE_OFF:
		Controller_Stop(1);
		Compute_Clock_SMT();
		BITCLR(LOCKLESS, AutoClock, 1);
		Proc->Registration.AutoClock = AutoClock;
		Controller_Start(1);
		rc = 0;
		break;
	case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		BITSET(LOCKLESS, AutoClock, 1);
		Proc->Registration.AutoClock = AutoClock;
		Controller_Start(1);
		rc = 0;
		break;
	}
	break;

      case MACHINE_EXPERIMENTAL:
	switch (prm.dl.lo) {
	    case COREFREQ_TOGGLE_OFF:
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		Proc->Registration.Experimental = prm.dl.lo;
		Controller_Start(1);
	    if( Proc->Registration.Experimental && !Proc->Registration.PCI )
	    {
		Proc->Registration.PCI = CoreFreqK_ProbePCI() == 0;
	    }
		rc = 2;
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
		rc = 0;
		break;
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		CoreFreqK_Register_NMI();
		Controller_Start(1);
		rc = 0;
		break;
	}
	break;

      case MACHINE_LIMIT_IDLE:
	if (Proc->Registration.Driver.CPUidle) {
		rc = CoreFreqK_Limit_Idle(prm.dl.lo);
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
			rc = 0;
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
			rc = 0;
			break;
		}
		break;

	case TECHNOLOGY_TURBO:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				TurboBoost_Enable = prm.dl.lo;
				Aggregate_Reset(BOOST(TGT),
						Proc->Boost[BOOST(MIN)]);
				Aggregate_Reset(BOOST(HWP_MIN), 0);
				Aggregate_Reset(BOOST(HWP_MAX), 0);
				Aggregate_Reset(BOOST(HWP_TGT), 0);
				Controller_Start(1);
				TurboBoost_Enable = -1;
				if((Proc->ArchID == AMD_Family_17h)
				|| (Proc->ArchID == AMD_Family_18h)) {
					Compute_AMD_Zen_Boost();
				}
			#ifdef CONFIG_CPU_FREQ
				Policy_Aggregate_Turbo();
			#endif /* CONFIG_CPU_FREQ */
				rc = 2;
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
				rc = 0;
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
				rc = 0;
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
				rc = 0;
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
				rc = 0;
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
				rc = 0;
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
				rc = 0;
				break;
		}
		break;

	case TECHNOLOGY_PKG_CSTATE:
		Controller_Stop(1);
		PkgCStateLimit = prm.dl.lo;
		Controller_Start(1);
		PkgCStateLimit = -1;
		rc = 0;
		break;

	case TECHNOLOGY_IO_MWAIT:
		switch (prm.dl.lo) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				Controller_Stop(1);
				IOMWAIT_Enable = prm.dl.lo;
				Controller_Start(1);
				IOMWAIT_Enable = -1;
				rc = 0;
				break;
		}
		break;

	case TECHNOLOGY_IO_MWAIT_REDIR:
		Controller_Stop(1);
		CStateIORedir = prm.dl.lo;
		Controller_Start(1);
		CStateIORedir = -1;
		rc = 0;
		break;

	case TECHNOLOGY_ODCM:
		Controller_Stop(1);
		ODCM_Enable = prm.dl.lo;
		Controller_Start(1);
		ODCM_Enable = -1;
		rc = 0;
		break;

	case TECHNOLOGY_ODCM_DUTYCYCLE:
		Controller_Stop(1);
		ODCM_DutyCycle = prm.dl.lo;
		Controller_Start(1);
		ODCM_DutyCycle = -1;
		rc = 0;
		break;

	case TECHNOLOGY_POWER_POLICY:
		Controller_Stop(1);
		PowerPolicy = prm.dl.lo;
		Controller_Start(1);
		PowerPolicy = -1;
		rc = 0;
		break;

	case TECHNOLOGY_HWP:
		Controller_Stop(1);
		HWP_Enable = prm.dl.lo;
		Intel_Hardware_Performance();
		Controller_Start(1);
		HWP_Enable = -1;
		rc = 0;
		break;

	case TECHNOLOGY_HWP_EPP:
		Controller_Stop(1);
		HWP_EPP = prm.dl.lo;
		Controller_Start(1);
		HWP_EPP = -1;
		rc = 0;
		break;
	}
	break;
    }
    break;

    case COREFREQ_IOCTL_CPU_OFF:
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)
	{
	unsigned int cpu = (unsigned int) arg;

	if (cpu < Proc->CPU.Count) {
		if (!cpu_is_hotpluggable(cpu))
			rc = -EINVAL;
		else
			rc = cpu_down(cpu);
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

	if (cpu < Proc->CPU.Count) {
		if (!cpu_is_hotpluggable(cpu))
			rc = -EINVAL;
		else
			rc = cpu_up(cpu);
	    }
	}
    #else
	rc = -EINVAL;
    #endif
	break;

    case COREFREQ_IOCTL_TURBO_CLOCK:
	if (Arch[Proc->ArchID].TurboClock) {
		CLOCK_ARG clockMod = {.sllong = arg};
		Controller_Stop(1);
		rc = Arch[Proc->ArchID].TurboClock(&clockMod);
		Controller_Start(1);
	}
	break;

    case COREFREQ_IOCTL_RATIO_CLOCK:
	if (Arch[Proc->ArchID].ClockMod) {
		CLOCK_ARG clockMod = {.sllong = arg};
		Controller_Stop(1);
		rc = Arch[Proc->ArchID].ClockMod(&clockMod);
		Aggregate_Reset(BOOST(TGT), Proc->Boost[BOOST(MIN)]);
		Aggregate_Reset(BOOST(HWP_MIN), 0);
		Aggregate_Reset(BOOST(HWP_MAX), 0);
		Aggregate_Reset(BOOST(HWP_TGT), 0);
		Controller_Start(1);
	#ifdef CONFIG_CPU_FREQ
		Policy_Aggregate_Turbo();
	#endif /* CONFIG_CPU_FREQ */
	}
	break;

    case COREFREQ_IOCTL_UNCORE_CLOCK:
	if (Arch[Proc->ArchID].Uncore.ClockMod) {
		CLOCK_ARG clockMod = {.sllong = arg};
		Controller_Stop(1);
		rc = Arch[Proc->ArchID].Uncore.ClockMod(&clockMod);
		Controller_Start(1);
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
			rc = 2;
			break;
	}
	break;

    default:
	rc = -EINVAL;
    }
	return (rc);
}

static int CoreFreqK_mmap(struct file *pfile, struct vm_area_struct *vma)
{
	unsigned long reqSize = vma->vm_end - vma->vm_start, secSize = 0;

	if (vma->vm_pgoff == 0) {
	    if (Proc != NULL) {
	      secSize = ROUND_TO_PAGES(sizeof(PROC));
	      if (reqSize != secSize)
		return (-EAGAIN);

	      if (remap_pfn_range(vma,
			vma->vm_start,
			virt_to_phys((void *) Proc) >> PAGE_SHIFT,
			reqSize,
			vma->vm_page_prot) < 0)
				return (-EIO);
	    }
	    else
		return (-EIO);
	} else if (vma->vm_pgoff == 1) {
	    if (Proc != NULL) {
		switch (SysGate_OnDemand()) {
		case -1:
			return (-EIO);
		case 1:
			/* Fallthrough */
		case 0:
			secSize = PAGE_SIZE << Proc->OS.ReqMem.Order;
			if (reqSize != secSize)
				return (-EAGAIN);

			if (remap_pfn_range(vma,
				vma->vm_start,
				virt_to_phys((void *)Proc->OS.Gate)>>PAGE_SHIFT,
				reqSize,
				vma->vm_page_prot) < 0)
					return (-EIO);
			break;
		}
	    }
	    else
		return (-EIO);
	} else if (vma->vm_pgoff >= 10) {
		signed int cpu = vma->vm_pgoff - 10;

	  if (Proc != NULL) {
	    if ((cpu >= 0) && (cpu < Proc->CPU.Count)) {
	      if (KPublic->Core[cpu] != NULL) {
		secSize = ROUND_TO_PAGES(sizeof(CORE));
		if (reqSize != secSize)
			return (-EAGAIN);

		if (remap_pfn_range(vma,
			vma->vm_start,
			virt_to_phys((void *) KPublic->Core[cpu]) >> PAGE_SHIFT,
			reqSize,
			vma->vm_page_prot) < 0)
				return (-EIO);
	      }
	      else
		return (-EIO);
	    }
	    else
		return (-EIO);
	  }
	  else
		return (-EIO);
	}
	else
		return (-EIO);
	return (0);
}

static DEFINE_MUTEX(CoreFreqK_mutex);		/* Only one daemon instance. */

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
static int CoreFreqK_suspend(struct device *dev)
{
	Controller_Stop(1);

	printk(KERN_NOTICE "CoreFreq: Suspend\n");

	return (0);
}

static int CoreFreqK_resume(struct device *dev)
{
	if (Proc->Registration.PCI) {		/* Probe PCI again	*/
		Proc->Registration.PCI = CoreFreqK_ProbePCI() == 0;
	}

	Aggregate_Reset(BOOST(TGT), Proc->Boost[BOOST(MIN)]);
	Aggregate_Reset(BOOST(HWP_MIN), 0);
	Aggregate_Reset(BOOST(HWP_MAX), 0);
	Aggregate_Reset(BOOST(HWP_TGT), 0);

	Controller_Start(1);

#ifdef CONFIG_CPU_FREQ
	Policy_Aggregate_Turbo();
#endif /* CONFIG_CPU_FREQ */

	BITSET(BUS_LOCK, Proc->OS.Signal, NTFY); /* Notify Daemon	*/

	printk(KERN_NOTICE "CoreFreq: Resume\n");

	return (0);
}

static SIMPLE_DEV_PM_OPS(CoreFreqK_pm_ops, CoreFreqK_suspend, CoreFreqK_resume);
#define COREFREQ_PM_OPS (&CoreFreqK_pm_ops)
#else /* CONFIG_PM_SLEEP */
#define COREFREQ_PM_OPS NULL
#endif /* CONFIG_PM_SLEEP */


#ifdef CONFIG_HOTPLUG_CPU
static int CoreFreqK_hotplug_cpu_online(unsigned int cpu)
{
  if (cpu < Proc->CPU.Count)
  {
	/* Is this the very first time the processor is online ? */
   if (KPublic->Core[cpu]->T.ApicID == -1)
   {
    if (Core_Topology(cpu) == 0)
    {
     if (KPublic->Core[cpu]->T.ApicID >= 0)
     {
	BITCLR(LOCKLESS, KPublic->Core[cpu]->OffLine, HW);
	/* Is the BCLK frequency missing ? */
      if (KPublic->Core[cpu]->Clock.Hz == 0)
      {
       if (AutoClock & 0b01)
       {
	COMPUTE_ARG Compute = {
		.TSC = {NULL, NULL},
		.Clock = {.Q = Proc->Boost[BOOST(MAX)], .R = 0, .Hz = 0}
	};
	if ((Compute.TSC[0] = kmalloc(STRUCT_SIZE, GFP_KERNEL)) != NULL)
	{
	    if ((Compute.TSC[1] = kmalloc(STRUCT_SIZE, GFP_KERNEL)) != NULL)
	    {
		KPublic->Core[cpu]->Clock = Compute_Clock(cpu, &Compute);

		kfree(Compute.TSC[1]);
	    }
		kfree(Compute.TSC[0]);
	}
       }
       else
	KPublic->Core[cpu]->Clock = KPublic->Core[Proc->Service.Core]->Clock;
      }
     }
    } else
	BITSET(LOCKLESS, KPublic->Core[cpu]->OffLine, HW);
   }
	/* Start the collect timer dedicated to this CPU. */
   if (Arch[Proc->ArchID].Timer != NULL) {
	Arch[Proc->ArchID].Timer(cpu);
   }
   if ((BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 0)
    && (Arch[Proc->ArchID].Start != NULL)) {
		smp_call_function_single(cpu,
					Arch[Proc->ArchID].Start,
					NULL, 0);
   }
	Proc->CPU.OnLine++;
	BITCLR(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);

	MatchPeerForUpService(&Proc->Service, cpu);

#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
   if (Proc->Registration.Driver.CPUidle) {
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

static int CoreFreqK_hotplug_cpu_offline(unsigned int cpu)
{
    if (cpu < Proc->CPU.Count) {
	/* Stop the associated collect timer. */
	if((BITVAL(KPrivate->Join[cpu]->TSM, CREATED) == 1)
	&& (BITVAL(KPrivate->Join[cpu]->TSM, STARTED) == 1)
	&& (Arch[Proc->ArchID].Stop != NULL)) {
		smp_call_function_single(cpu,
					Arch[Proc->ArchID].Stop,
					NULL, 1);
	}
	Proc->CPU.OnLine--;
	BITSET(LOCKLESS, KPublic->Core[cpu]->OffLine, OS);

	/* Seek for an alternate Service Processor. */
	if ((cpu == Proc->Service.Core) || (cpu == Proc->Service.Thread))
	{
		MatchPeerForDownService(&Proc->Service, cpu);

	  if (Proc->Service.Core != cpu)
	  {
	    if (Arch[Proc->ArchID].Update != NULL)
		smp_call_function_single(Proc->Service.Core,
					Arch[Proc->ArchID].Update,
					KPublic->Core[Proc->Service.Core], 1);
#if CONFIG_HAVE_PERF_EVENTS==1
		/* Reinitialize PMU Uncore counters. */
	    if (Arch[Proc->ArchID].Uncore.Stop != NULL)
		smp_call_function_single(Proc->Service.Core,
					Arch[Proc->ArchID].Uncore.Stop,
					NULL, 1); /* Must wait! */

	    if (Arch[Proc->ArchID].Uncore.Start != NULL)
		smp_call_function_single(Proc->Service.Core,
					Arch[Proc->ArchID].Uncore.Start,
					NULL, 0); /* Don't wait */
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
static int CoreFreqK_hotplug(	struct notifier_block *nfb,
				unsigned long action,
				void *hcpu)
{
	unsigned int cpu = (unsigned long) hcpu, rc = 0;

	switch (action) {
	case CPU_ONLINE:
	case CPU_DOWN_FAILED:
/*TODO	case CPU_ONLINE_FROZEN: 					*/
		rc = CoreFreqK_hotplug_cpu_online(cpu);
		break;
	case CPU_DOWN_PREPARE:
/*TODO	case CPU_DOWN_PREPARE_FROZEN:					*/
		rc = CoreFreqK_hotplug_cpu_offline(cpu);
		break;
	default:
		break;
	}
	return (NOTIFY_OK);
}

static struct notifier_block CoreFreqK_notifier_block = {
	.notifier_call = CoreFreqK_hotplug,
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
		{ DMI_BIOS_VENDOR,	Proc->SMB.BIOS.Vendor	},
		{ DMI_BIOS_VERSION,	Proc->SMB.BIOS.Version	},
		{ DMI_BIOS_DATE,	Proc->SMB.BIOS.Release	},
		{ DMI_SYS_VENDOR,	Proc->SMB.System.Vendor },
		{ DMI_PRODUCT_NAME,	Proc->SMB.Product.Name	},
		{ DMI_PRODUCT_VERSION,	Proc->SMB.Product.Version},
		{ DMI_PRODUCT_SERIAL,	Proc->SMB.Product.Serial},
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
		{ DMI_PRODUCT_SKU,	Proc->SMB.Product.SKU	},
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
		{ DMI_PRODUCT_FAMILY,	Proc->SMB.Product.Family},
#endif
		{ DMI_BOARD_NAME,	Proc->SMB.Board.Name	},
		{ DMI_BOARD_VERSION,	Proc->SMB.Board.Version },
		{ DMI_BOARD_SERIAL,	Proc->SMB.Board.Serial	}
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

static int __init CoreFreqK_init(void)
{
	INIT_ARG iArg={.Features=NULL, .SMT_Count=0, .localProcessor=0, .rc=0};
	int rc = 0;

    if ((iArg.Features = kmalloc(sizeof(FEATURES), GFP_KERNEL)) == NULL) {
	rc = -ENOMEM;
	goto EXIT;
    }
	memset(iArg.Features, 0, sizeof(FEATURES));

    if (ServiceProcessor == -1) {	/* Query features on any processor. */
	iArg.localProcessor = get_cpu();
	Query_Features(&iArg);
	put_cpu();
	rc = iArg.rc;
    } else {
	if (ServiceProcessor >= 0) {
		iArg.localProcessor = ServiceProcessor;
		if((rc = smp_call_function_single(iArg.localProcessor,
						Query_Features,
						&iArg, 1)) == 0) {
			rc = iArg.rc;
		}
	} else
		rc = -ENXIO;
    }
    if (rc == 0) {
	unsigned int OS_Count = num_present_cpus();
	/* Rely on the operating system's cpu counting. */
	if (iArg.SMT_Count != OS_Count)
		iArg.SMT_Count = OS_Count;
    } else
	rc = -ENXIO;

  if (rc == 0)
  {
	CoreFreqK.kcdev = cdev_alloc();
	CoreFreqK.kcdev->ops = &CoreFreqK_fops;
	CoreFreqK.kcdev->owner = THIS_MODULE;

    if (alloc_chrdev_region(&CoreFreqK.nmdev, 0, 1, DRV_FILENAME) >= 0)
    {
	CoreFreqK.Major = MAJOR(CoreFreqK.nmdev);
	CoreFreqK.mkdev = MKDEV(CoreFreqK.Major, 0);

      if (cdev_add(CoreFreqK.kcdev, CoreFreqK.mkdev, 1) >= 0)
      {
	struct device *tmpDev;

	CoreFreqK.clsdev = class_create(THIS_MODULE, DRV_DEVNAME);
	CoreFreqK.clsdev->pm = COREFREQ_PM_OPS;

	if ((tmpDev = device_create(	CoreFreqK.clsdev, NULL,
					CoreFreqK.mkdev, NULL,
					DRV_DEVNAME)) != NULL)
	{
		unsigned int cpu = 0;
		unsigned long publicSize = 0,privateSize = 0,packageSize = 0;

		publicSize = sizeof(KPUBLIC) + sizeof(CORE*) * iArg.SMT_Count;

		privateSize = sizeof(KPRIVATE) + sizeof(JOIN*) * iArg.SMT_Count;

	  if (((KPublic = kmalloc(publicSize, GFP_KERNEL)) != NULL)
	   && ((KPrivate = kmalloc(privateSize, GFP_KERNEL)) != NULL))
	  {
		memset(KPublic, 0, publicSize);
		memset(KPrivate, 0, privateSize);

		packageSize = ROUND_TO_PAGES(sizeof(PROC));
	    if ((Proc = kmalloc(packageSize, GFP_KERNEL)) != NULL)
	    {
		memset(Proc, 0, packageSize);
		SET_FOOTPRINT(Proc->FootPrint,	COREFREQ_MAJOR, \
						COREFREQ_MINOR, \
						COREFREQ_REV	);

		Proc->CPU.Count = iArg.SMT_Count;
		/* PreCompute SysGate memory allocation. */
		Proc->OS.ReqMem.Size = sizeof(SYSGATE);
		Proc->OS.ReqMem.Order = get_order(Proc->OS.ReqMem.Size);

		Proc->Registration.AutoClock = AutoClock;
		Proc->Registration.Experimental = Experimental;

		Compute_Interval();

		memcpy(&Proc->Features, iArg.Features, sizeof(FEATURES));

		/* Initialize default uArch's codename with the CPUID brand. */
		Arch[0].Architecture->CodeName = Proc->Features.Info.Vendor.ID;

		publicSize  = ROUND_TO_PAGES(sizeof(CORE));
		privateSize = ROUND_TO_PAGES(sizeof(JOIN));

		if (((KPublic->Cache = kmem_cache_create("corefreqk-pub",
							publicSize, 0,
							SLAB_HWCACHE_ALIGN,
							NULL)) != NULL)
		 && ((KPrivate->Cache = kmem_cache_create("corefreqk-priv",
							privateSize, 0,
							SLAB_HWCACHE_ALIGN,
							NULL)) != NULL))
		{
			int allocPerCPU = 1;
			/* Allocation per CPU. */
		  for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
			void *kcache = NULL;
			kcache = kmem_cache_alloc(KPublic->Cache, GFP_KERNEL);
			if (kcache != NULL) {
				memset(kcache, 0, publicSize);
				KPublic->Core[cpu] = kcache;
			} else {
				allocPerCPU = 0;
				break;
			}
			kcache = kmem_cache_alloc(KPrivate->Cache, GFP_KERNEL);
			if (kcache != NULL) {
				memset(kcache, 0, privateSize);
				KPrivate->Join[cpu] = kcache;
			} else {
				allocPerCPU = 0;
				break;
			}
		  }
		  if (allocPerCPU)
		  {
		    for (cpu = 0; cpu < Proc->CPU.Count; cpu++) {
			BITCLR(LOCKLESS, KPublic->Core[cpu]->Sync.V, NTFY);

			KPublic->Core[cpu]->Bind = cpu;

			Define_CPUID(KPublic->Core[cpu], CpuIDforVendor);
		    }

		    switch (Proc->Features.Info.Vendor.CRC)
		    {
		    case CRC_INTEL: {
			Arch[0].Query	= Query_GenuineIntel;
			Arch[0].Update	= PerCore_Intel_Query;
			Arch[0].Start	= Start_GenuineIntel;
			Arch[0].Stop	= Stop_GenuineIntel;
			Arch[0].Timer	= InitTimer_GenuineIntel;
			Arch[0].BaseClock= BaseClock_GenuineIntel;

			Arch[0].thermalFormula = THERMAL_FORMULA_INTEL;

			Arch[0].voltageFormula = VOLTAGE_FORMULA_INTEL;

			Arch[0].powerFormula = POWER_FORMULA_INTEL;
			}
			break;
		    case CRC_HYGON:
			/* Fallthrough */
		    case CRC_AMD: {
			Arch[0].Query	= Query_AuthenticAMD;
			Arch[0].Update	= PerCore_AuthenticAMD_Query;
			Arch[0].Start	= Start_AuthenticAMD;
			Arch[0].Stop	= Stop_AuthenticAMD;
			Arch[0].Timer	= InitTimer_AuthenticAMD;
			Arch[0].BaseClock= BaseClock_AuthenticAMD;

			Arch[0].thermalFormula = THERMAL_FORMULA_AMD;

			Arch[0].voltageFormula = VOLTAGE_FORMULA_AMD;

			Arch[0].powerFormula = POWER_FORMULA_AMD;
			}
			break;
		    }
		#ifdef CONFIG_XEN
		    if (xen_pv_domain() || xen_hvm_domain())
		    {
			if (ArchID == -1)
				ArchID = SearchArchitectureID();

			if (Proc->Features.Std.ECX.Hyperv == 0)
				Proc->Features.Std.ECX.Hyperv = 1;

			Proc->HypervisorID = HYPERV_XEN;
		    }
		#endif /* CONFIG_XEN */
		    if((Proc->Features.Std.ECX.Hyperv == 1) && (ArchID == -1))
		    {
			ArchID = 0;
		    }
		    if((ArchID != -1)&&(ArchID >= 0)&&(ArchID < ARCHITECTURES))
		    {
			Proc->ArchID = ArchID;
		    }
		    else
		    {
			Proc->ArchID = SearchArchitectureID();
		    }
			Proc->thermalFormula=Arch[Proc->ArchID].thermalFormula;
			Proc->voltageFormula=Arch[Proc->ArchID].voltageFormula;
			Proc->powerFormula = Arch[Proc->ArchID].powerFormula;

			CoreFreqK_Thermal_Scope(ThermalScope);
			CoreFreqK_Voltage_Scope(VoltageScope);
			CoreFreqK_Power_Scope(PowerScope);

			/* Set the uArch's name with the first found codename */
			StrCopy(Proc->Architecture,
				Arch[Proc->ArchID].Architecture[0].CodeName,
				CODENAME_LEN);

			/* Copy various SMBIOS data [version 3.2]	*/
			SMBIOS_Collect();

			/* Initialize the CoreFreq controller		*/
			Controller_Init();

			MatchPeerForDefaultService(&Proc->Service,
						iArg.localProcessor);

			/* Register the Idle & Frequency sub-drivers	*/
		    if (Register_CPU_Idle == 1) {
			Proc->Registration.Driver.CPUidle =		\
					CoreFreqK_IdleDriver_Init() == 0;
		    }
		    if (Register_CPU_Freq == 1) {
			Proc->Registration.Driver.CPUfreq =		\
					CoreFreqK_FreqDriver_Init() == 0;
		    }
		    if (Register_Governor == 1) {
			Proc->Registration.Driver.Governor =		\
					CoreFreqK_Governor_Init() == 0;
		    }
		    if (NMI_Disable == 0) {
			CoreFreqK_Register_NMI();
		    }

			printk(KERN_INFO "CoreFreq(%u:%d):"	\
				" Processor [%2X%1X_%1X%1X]"	\
				" Architecture [%s] %3s [%u/%u]\n",
				Proc->Service.Core, Proc->Service.Thread,
				Proc->Features.Std.EAX.ExtFamily,
				Proc->Features.Std.EAX.Family,
				Proc->Features.Std.EAX.ExtModel,
				Proc->Features.Std.EAX.Model,
				Proc->Architecture,
				Proc->Features.HTT_Enable ? "SMT" : "CPU",
				Proc->CPU.OnLine,
				Proc->CPU.Count);

			Controller_Start(0);

			Proc->Registration.PCI = CoreFreqK_ProbePCI() == 0;

	#ifdef CONFIG_HOTPLUG_CPU
		#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)
			/* Always returns zero (kernel/notifier.c) */
			Proc->Registration.HotPlug = register_hotcpu_notifier(
						&CoreFreqK_notifier_block);
		#else	/* Continue with or without cpu hot-plugging. */
			Proc->Registration.HotPlug = cpuhp_setup_state_nocalls(
						CPUHP_AP_ONLINE_DYN,
						"corefreqk/cpu:online",
						CoreFreqK_hotplug_cpu_online,
						CoreFreqK_hotplug_cpu_offline);
		#endif /* KERNEL_VERSION(4, 6, 0) */
	#endif /* CONFIG_HOTPLUG_CPU */

		#ifdef CONFIG_CPU_FREQ
			Policy_Aggregate_Turbo();
		#endif /* CONFIG_CPU_FREQ */
		  }
		  else
		  {
		   if (KPublic->Cache != NULL) {
		    for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    {
		     if (KPublic->Core[cpu] != NULL)
			kmem_cache_free(KPublic->Cache, KPublic->Core[cpu]);
		    }
			kmem_cache_destroy(KPublic->Cache);
		   }
		   if (KPrivate->Cache != NULL) {
		    for (cpu = 0; cpu < Proc->CPU.Count; cpu++)
		    {
		     if (KPrivate->Join[cpu] != NULL)
			kmem_cache_free(KPrivate->Cache, KPrivate->Join[cpu]);
		    }
			kmem_cache_destroy(KPrivate->Cache);
		   }
			kfree(Proc);
			kfree(KPublic);
			kfree(KPrivate);

			device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
			class_destroy(CoreFreqK.clsdev);
			cdev_del(CoreFreqK.kcdev);
			unregister_chrdev_region(CoreFreqK.mkdev, 1);

			rc = -ENOMEM;
		  }
		} else {
			if (KPublic->Cache != NULL)
				kmem_cache_destroy(KPublic->Cache);
			if (KPrivate->Cache != NULL)
				kmem_cache_destroy(KPrivate->Cache);

			kfree(Proc);
			kfree(KPublic);
			kfree(KPrivate);

			device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
			class_destroy(CoreFreqK.clsdev);
			cdev_del(CoreFreqK.kcdev);
			unregister_chrdev_region(CoreFreqK.mkdev, 1);

			rc = -ENOMEM;
		}
	    } else {
		kfree(KPublic);
		kfree(KPrivate);

		device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
		class_destroy(CoreFreqK.clsdev);
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		rc = -ENOMEM;
	    }
	  } else {
		if (KPublic != NULL)
			kfree(KPublic);
		if (KPrivate != NULL)
			kfree(KPrivate);

		device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
		class_destroy(CoreFreqK.clsdev);
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		rc = -ENOMEM;
	  }
	} else {
		class_destroy(CoreFreqK.clsdev);
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		rc = -EBUSY;
	}
      } else {
	cdev_del(CoreFreqK.kcdev);
	unregister_chrdev_region(CoreFreqK.mkdev, 1);

	rc = -EBUSY;
      }
    } else {
	cdev_del(CoreFreqK.kcdev);

	rc = -EBUSY;
    }
  }
	kfree(iArg.Features);
EXIT:
	return (rc);
}

static void __exit CoreFreqK_cleanup(void)
{
	if (Proc != NULL) {
		unsigned int cpu = 0;

		if (Proc->Registration.Driver.Governor) {
			cpufreq_unregister_governor(&CoreFreqK.FreqGovernor);
		}
		if (Proc->Registration.Driver.CPUfreq) {
			CoreFreqK_FreqDriver_UnInit();
		}
		if (Proc->Registration.Driver.CPUidle) {
			CoreFreqK_IdleDriver_UnInit();
		}
		CoreFreqK_UnRegister_NMI();
#ifdef CONFIG_HOTPLUG_CPU
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
		unregister_hotcpu_notifier(&CoreFreqK_notifier_block);
	#else /* KERNEL_VERSION(4, 10, 0) */
		if (!(Proc->Registration.HotPlug < 0))
			cpuhp_remove_state_nocalls(Proc->Registration.HotPlug);
	#endif /* KERNEL_VERSION(4, 10, 0) */
#endif /* CONFIG_HOTPLUG_CPU */
		Controller_Stop(1);
		Controller_Exit();

		if (Proc->OS.Gate != NULL)
			free_pages_exact(Proc->OS.Gate, Proc->OS.ReqMem.Size);

	for (cpu = 0;(KPublic->Cache != NULL) && (cpu < Proc->CPU.Count); cpu++)
	{
		if (KPublic->Core[cpu] != NULL)
			kmem_cache_free(KPublic->Cache, KPublic->Core[cpu]);
		if (KPrivate->Join[cpu] != NULL)
			kmem_cache_free(KPrivate->Cache, KPrivate->Join[cpu]);
	}
		if (KPublic->Cache != NULL)
			kmem_cache_destroy(KPublic->Cache);
		if (KPrivate->Cache != NULL)
			kmem_cache_destroy(KPrivate->Cache);

		if (KPublic != NULL)
			kfree(KPublic);
		if (KPrivate != NULL)
			kfree(KPrivate);

		device_destroy(CoreFreqK.clsdev, CoreFreqK.mkdev);
		class_destroy(CoreFreqK.clsdev);
		cdev_del(CoreFreqK.kcdev);
		unregister_chrdev_region(CoreFreqK.mkdev, 1);

		printk(KERN_NOTICE "CoreFreq: Unload\n");

		kfree(Proc);
	}
}

module_init(CoreFreqK_init);
module_exit(CoreFreqK_cleanup);

