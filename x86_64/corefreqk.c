/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 *
 * Upadacitinib [07.29.2024]
 * Infliximab [11.17.2023]
 * Vedolizumab [05.25.2023]
 *
 * CYRIL INGENIERIE[11.30.2022]
 * Company closed down
 *
 * Time Capsule[02.02.2022]
 * Cyril to Marcel Courtiat
 * RIP Daddy; love you forever
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
#ifdef CONFIG_HAVE_NMI
#include <asm/nmi.h>
#endif
#ifdef CONFIG_XEN
#include <xen/xen.h>
#endif /* CONFIG_XEN */
#include <asm/mwait.h>
#ifdef CONFIG_AMD_NB
#include <asm/amd_nb.h>
#endif
#ifdef CONFIG_ACPI
#include <linux/acpi.h>
#include <acpi/processor.h>
#endif
#ifdef CONFIG_ACPI_CPPC_LIB
#include <acpi/cppc_acpi.h>
#endif

#include "bitasm.h"
#include "amd_reg.h"
#include "intel_reg.h"
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

static signed short Config_TDP_Level = -1;
module_param(Config_TDP_Level, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Config_TDP_Level, "Config TDP Control Level");

static unsigned int Custom_TDP_Count = 0;
static signed short Custom_TDP_Offset[PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE)] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
module_param_array(Custom_TDP_Offset, short, &Custom_TDP_Count ,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Custom_TDP_Offset, "TDP Limit Offset (watt)");

static unsigned int PowerTimeWindow_Count = 0;
static signed short PowerTimeWindow[PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE)] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
module_param_array(PowerTimeWindow, short, &PowerTimeWindow_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PowerTimeWindow, "Power Limit PL(n) Time Window");

static unsigned int TDP_Limiting_Count = 0;
static signed short Activate_TDP_Limit[PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE)] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
module_param_array(Activate_TDP_Limit, short, &TDP_Limiting_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Activate_TDP_Limit, "Activate TDP Limiting");

static unsigned int TDP_Clamping_Count = 0;
static signed short Activate_TDP_Clamp[PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE)] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
module_param_array(Activate_TDP_Clamp, short, &TDP_Clamping_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Activate_TDP_Clamp, "Activate TDP Clamping");

static unsigned short Custom_TDC_Offset = 0;
module_param(Custom_TDC_Offset, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Custom_TDC_Offset, "TDC Limit Offset (amp)");

static signed short Activate_TDC_Limit = -1;
module_param(Activate_TDC_Limit, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Activate_TDC_Limit, "Activate TDC Limiting");

static signed short L1_HW_PREFETCH_Disable = -1;
module_param(L1_HW_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_HW_PREFETCH_Disable, "Disable L1 HW Prefetcher");

static signed short L1_HW_IP_PREFETCH_Disable = -1;
module_param(L1_HW_IP_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_HW_IP_PREFETCH_Disable, "Disable L1 HW IP Prefetcher");

static signed short L1_NPP_PREFETCH_Disable = -1;
module_param(L1_NPP_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_NPP_PREFETCH_Disable, "Disable L1 NPP Prefetcher");

static signed short L1_Scrubbing_Enable = -1;
module_param(L1_Scrubbing_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_Scrubbing_Enable, "Enable L1 Scrubbing");

static signed short L2_HW_PREFETCH_Disable = -1;
module_param(L2_HW_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L2_HW_PREFETCH_Disable, "Disable L2 HW Prefetcher");

static signed short L2_HW_CL_PREFETCH_Disable = -1;
module_param(L2_HW_CL_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L2_HW_CL_PREFETCH_Disable, "Disable L2 HW CL Prefetcher");

static signed short L2_AMP_PREFETCH_Disable = -1;
module_param(L2_AMP_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L2_AMP_PREFETCH_Disable, "Adaptive Multipath Probability");

static signed short L2_NLP_PREFETCH_Disable = -1;
module_param(L2_NLP_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L2_NLP_PREFETCH_Disable, "Disable L2 NLP Prefetcher");

static signed short L1_STRIDE_PREFETCH_Disable = -1;
module_param(L1_STRIDE_PREFETCH_Disable, short,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_STRIDE_PREFETCH_Disable, "Disable L1 Stride Prefetcher");

static signed short L1_REGION_PREFETCH_Disable = -1;
module_param(L1_REGION_PREFETCH_Disable, short,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_REGION_PREFETCH_Disable, "Disable L1 Region Prefetcher");

static signed short L1_BURST_PREFETCH_Disable = -1;
module_param(L1_BURST_PREFETCH_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L1_BURST_PREFETCH_Disable, "Disable L1 Burst Prefetcher");

static signed short L2_STREAM_PREFETCH_Disable = -1;
module_param(L2_STREAM_PREFETCH_Disable, short,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L2_STREAM_PREFETCH_Disable, "Disable L2 Stream Prefetcher");

static signed short L2_UPDOWN_PREFETCH_Disable = -1;
module_param(L2_UPDOWN_PREFETCH_Disable, short,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(L2_UPDOWN_PREFETCH_Disable, "Disable L2 Up/Down Prefetcher");

static signed short LLC_Streamer_Disable = -1;
module_param(LLC_Streamer_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(LLC_Streamer_Disable, "Disable LLC Streamer");

static signed short SpeedStep_Enable = -1;
module_param(SpeedStep_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(SpeedStep_Enable, "Enable SpeedStep");

static signed short C1E_Enable = -1;
module_param(C1E_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1E_Enable, "Enable SpeedStep C1E");

static unsigned int TurboBoost_Enable_Count = 1;
static signed short TurboBoost_Enable[2] = {-1, -1};
module_param_array(TurboBoost_Enable, short, &TurboBoost_Enable_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
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
module_param_named(C1U_Enable,C1U_Enable,short,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
module_param_named(C2U_Enable,C1U_Enable,short,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(C1U_Enable, "Enable C1 UnDemotion");
MODULE_PARM_DESC(C2U_Enable, "Enable C2 UnDemotion");

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

static signed short Turbo_Activation_Ratio = -1;
module_param(Turbo_Activation_Ratio, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Turbo_Activation_Ratio, "Turbo Activation Ratio");

static signed int PState_FID = -1;
module_param(PState_FID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PState_FID, "P-State Frequency Id");

static signed int PState_VID = -1;
module_param(PState_VID, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PState_VID, "P-State Voltage Id");

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

static signed short HDC_Enable = -1;
module_param(HDC_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HDC_Enable, "Hardware Duty Cycling");

static signed short EEO_Disable = -1;
module_param(EEO_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(EEO_Disable, "Disable Energy Efficiency Optimization");

static signed short R2H_Disable = -1;
module_param(R2H_Disable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(R2H_Disable, "Disable Race to Halt");

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
static unsigned long Clear_Events = 0;
module_param(Clear_Events, ulong, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#else
static unsigned long long Clear_Events = 0;
module_param(Clear_Events, ullong, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
#endif
MODULE_PARM_DESC(Clear_Events, "Clear Thermal and Power Events");

static unsigned int ThermalPoint_Count = 0;
static signed short ThermalPoint[THM_POINTS_DIM] = {
			-1, -1, -1, -1, -1
};
module_param_array(ThermalPoint, short, &ThermalPoint_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ThermalPoint, "Thermal Point");

static unsigned int PkgThermalPoint_Count = 0;
static signed short PkgThermalPoint[THM_POINTS_DIM] = {
			-1, -1, -1, -1, -1
};
module_param_array(PkgThermalPoint, short, &PkgThermalPoint_Count,	\
					S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(PkgThermalPoint, "Package Thermal Point");

static signed short ThermalOffset = 0;
module_param(ThermalOffset, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(ThermalOffset, "Thermal Offset");

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

static signed short Mech_SBPB = -1;
module_param(Mech_SBPB, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_SBPB, "Mitigation Mechanism SBPB");

static signed short Mech_L1D_FLUSH = -1;
module_param(Mech_L1D_FLUSH, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_L1D_FLUSH, "Mitigation Mechanism Cache L1D Flush");

static signed short Mech_PSFD = -1;
module_param(Mech_PSFD, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_PSFD, "Mitigation Mechanism PSFD");

static signed short Mech_BTC_NOBR = -1;
module_param(Mech_BTC_NOBR, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_BTC_NOBR, "Mitigation Mechanism BTC-NOBR");

static signed short Mech_XPROC_LEAK = -1;
module_param(Mech_XPROC_LEAK, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_XPROC_LEAK, "Mitigation Mech. Cross Processor Leak");

static signed short Mech_AGENPICK = -1;
module_param(Mech_AGENPICK, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(Mech_AGENPICK, "Mitigation Mech. LsCfgDisAgenPick");

static signed short WDT_Enable = -1;
module_param(WDT_Enable, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(WDT_Enable, "Watchdog Hardware Timer");

static signed short HSMP_Attempt = -1;
module_param(HSMP_Attempt, short, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
MODULE_PARM_DESC(HSMP_Attempt, "Attempt the HSMP interface");

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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
			.bios_limit= CoreFreqK_Bios_Limit,
			.set_boost = CoreFreqK_SetBoost
#else
			.bios_limit= CoreFreqK_Bios_Limit
#endif
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

static unsigned int FixMissingRatioAndFrequency(unsigned int r32, CLOCK *pClock)
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

		pr_debug("%s: Freq_KHz[%u] Kernel CPU_KHZ[%u] TSC_KHZ[%u]\n" \
			"LPJ[%lu] mask[%llx] mult[%u] shift[%u]\n",
		CoreFreqK_CS.name, Freq_KHz, cpu_khz, tsc_khz, loops_per_jiffy,
		CoreFreqK_CS.mask, CoreFreqK_CS.mult, CoreFreqK_CS.shift);

		break;
	    }
	}
    } else {
		PUBLIC(RO(Proc))->Registration.Driver.CS = REGISTRATION_DISABLE;
    }
	return rc;
}

static void VendorFromCPUID(char *pVendorID, unsigned int *pLargestFunc,
			unsigned int *pCRC, enum HYPERVISOR *pHypervisor,
			unsigned long leaf, unsigned long subLeaf)
{
static const struct {
		char			*vendorID;
		size_t			vendorLen;
		enum CRC_MANUFACTURER	mfrCRC;
		enum HYPERVISOR 	hypervisor;
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
	const unsigned int mfrSize = sizeof(mfrTbl) / sizeof(mfrTbl[0]);
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

    for (eax = 0; eax < mfrSize; eax++) {
	if (!strncmp(pVendorID, mfrTbl[eax].vendorID, mfrTbl[eax].vendorLen))
	{
		(*pCRC) = mfrTbl[eax].mfrCRC;
		(*pHypervisor) = mfrTbl[eax].hypervisor;

		return;
	}
    }
}

static signed int SearchArchitectureID(void)
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

static void BrandCleanup(char *pBrand, char inOrder[])
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

static void BrandFromCPUID(char *buffer)
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

static unsigned int Intel_Brand(char *pBrand, char buffer[])
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
		frequency  = (int) (buffer[ix-4] - '0') * multiplier;
		frequency += (int) (buffer[ix-2] - '0') * (multiplier / 10);
		frequency += (int) (buffer[ix-1] - '0') * (multiplier / 100);
	    } else {
		frequency  = (int) (buffer[ix-4] - '0') * 1000;
		frequency += (int) (buffer[ix-3] - '0') * 100;
		frequency += (int) (buffer[ix-2] - '0') * 10;
		frequency += (int) (buffer[ix-1] - '0');
		frequency *= frequency;
	    }
	}

	BrandCleanup(pBrand, buffer);

	return frequency;
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
	fallthrough;
      case 7:
	iArg->Features->MWait.EDX.SubCstate_MWAIT6 = Override_SubCstate[6];
	fallthrough;
      case 6:
	iArg->Features->MWait.EDX.SubCstate_MWAIT5 = Override_SubCstate[5];
	fallthrough;
      case 5:
	iArg->Features->MWait.EDX.SubCstate_MWAIT4 = Override_SubCstate[4];
	fallthrough;
      case 4:
	iArg->Features->MWait.EDX.SubCstate_MWAIT3 = Override_SubCstate[3];
	fallthrough;
      case 3:
	iArg->Features->MWait.EDX.SubCstate_MWAIT2 = Override_SubCstate[2];
	fallthrough;
      case 2:
	iArg->Features->MWait.EDX.SubCstate_MWAIT1 = Override_SubCstate[1];
	fallthrough;
      case 1:
	iArg->Features->MWait.EDX.SubCstate_MWAIT0 = Override_SubCstate[0];
	break;
      };

	CoreFreqK.SubCstate[0] = iArg->Features->MWait.EDX.SubCstate_MWAIT0;
	CoreFreqK.SubCstate[1] = iArg->Features->MWait.EDX.SubCstate_MWAIT1;
/*C1E*/ CoreFreqK.SubCstate[2] = iArg->Features->MWait.EDX.SubCstate_MWAIT1;
	CoreFreqK.SubCstate[3] = iArg->Features->MWait.EDX.SubCstate_MWAIT2;
	CoreFreqK.SubCstate[4] = iArg->Features->MWait.EDX.SubCstate_MWAIT3;
	CoreFreqK.SubCstate[5] = iArg->Features->MWait.EDX.SubCstate_MWAIT4;
	CoreFreqK.SubCstate[6] = iArg->Features->MWait.EDX.SubCstate_MWAIT5;
	CoreFreqK.SubCstate[7] = iArg->Features->MWait.EDX.SubCstate_MWAIT6;
	CoreFreqK.SubCstate[8] = iArg->Features->MWait.EDX.SubCstate_MWAIT7;
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
			: "=r" (iArg->Features->ExtFeature_Leaf1_EAX),
			  "=r" (ebx),
			  "=r" (ecx),
			  "=r" (iArg->Features->ExtFeature_Leaf1_EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	if (iArg->Features->ExtFeature.EAX.MaxSubLeaf >= 2)
	{
		__asm__ volatile
		(
			"movq	$0x7,  %%rax	\n\t"
			"movq	$0x2,  %%rcx    \n\t"
			"xorq	%%rbx, %%rbx    \n\t"
			"xorq	%%rdx, %%rdx    \n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0	\n\t"
			"mov	%%ebx, %1	\n\t"
			"mov	%%ecx, %2	\n\t"
			"mov	%%edx, %3"
			: "=r" (eax),
			  "=r" (ebx),
			  "=r" (ecx),
			  "=r" (iArg->Features->ExtFeature_Leaf2_EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
    }
    if (iArg->Features->Info.LargestStdFunc >= 0xd)
    {
	__asm__ volatile
	(
		"movq	$0xd,  %%rax	\n\t"
		"xorq	%%rbx, %%rbx    \n\t"
		"xorq	%%rcx, %%rcx    \n\t"
		"xorq	%%rdx, %%rdx    \n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->ExtState.EAX),
		  "=r" (iArg->Features->ExtState.EBX),
		  "=r" (iArg->Features->ExtState.ECX),
		  "=r" (iArg->Features->ExtState.EDX)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);
	__asm__ volatile
	(
		"movq	$0xd,  %%rax	\n\t"
		"movq	$0x1,  %%rcx    \n\t"
		"xorq	%%rbx, %%rbx    \n\t"
		"xorq	%%rdx, %%rdx    \n\t"
		"cpuid			\n\t"
		"mov	%%eax, %0	\n\t"
		"mov	%%ebx, %1	\n\t"
		"mov	%%ecx, %2	\n\t"
		"mov	%%edx, %3"
		: "=r" (iArg->Features->ExtState_Leaf1.EAX),
		  "=r" (iArg->Features->ExtState_Leaf1.EBX),
		  "=r" (iArg->Features->ExtState_Leaf1.ECX),
		  "=r" (iArg->Features->ExtState_Leaf1.EDX)
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
	iArg->Features->PerfMon.EBX.TopdownSlots  = 1;

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
    {	/*	AMD PM version: Use Intel bits as AMD placeholder.	*/
	iArg->Features->PerfMon.EAX.Version = 1;
	/*	Specified as Core Performance 48 bits General Counters. */
	iArg->Features->PerfMon.EAX.MonWidth = 48;

	if (iArg->Features->ExtInfo.ECX.PerfCore) {
		switch (iArg->Features->Std.EAX.ExtFamily) {
		case 0x6:
		case 0x8 ... 0xa:
		/* PPR: six core performance event counters per thread	*/
			iArg->Features->PerfMon.EAX.MonCtrs = 6;
			break;
		case 0x0 ... 0x5:
		case 0x7:
			iArg->Features->PerfMon.EAX.MonCtrs = 4;
			break;
		}
	} else {	/*	CPUID 0F_00h		*/
		iArg->Features->PerfMon.EAX.MonCtrs = 4;
	}
	/* Specified as Data Fabric or Northbridge Performance Events Counter */
	if (iArg->Features->ExtInfo.ECX.PerfNB)
	{	/* PPR: four Data Fabric performance events counters	*/
		iArg->Features->Factory.PMC.NB = 4;
	} else {
		iArg->Features->Factory.PMC.NB = 0;
	}
	/* Specified as L3 Cache or L2I-ext Performance Events Counters */
	if (iArg->Features->ExtInfo.ECX.PerfLLC) {
		switch (iArg->Features->Std.EAX.ExtFamily) {
		case 0x8 ... 0xB:
		/* PPR: six performance events counters per L3 complex	*/
			iArg->Features->Factory.PMC.LLC = 6;
			break;
		case 0x6:
			if (iArg->Features->Std.EAX.ExtModel < 0x6) {
				break;
			}
			fallthrough;
		case 0x7:
			iArg->Features->Factory.PMC.LLC = 4;
			break;
		default:
			iArg->Features->Factory.PMC.LLC = 4;
			break;
		}
	} else {
		iArg->Features->Factory.PMC.LLC = 0;
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
	    if (iArg->Features->Std.EAX.ExtFamily < 0xB) {
		iArg->SMT_Count = iArg->Features->leaf80000008.ECX.NC + 1;
	    } else {
		iArg->SMT_Count = iArg->Features->leaf80000008.ECX.F1Ah.NC + 1;
	    }
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
	if (iArg->Features->Info.LargestExtFunc >= 0x80000021)
	{
		__asm__ volatile
		(
			"movq	$0x80000021, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (iArg->Features->ExtFeature2_EAX),
			  "=r" (ebx),
			  "=r" (ecx),
			  "=r" (edx)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}
	if (iArg->Features->Info.LargestExtFunc >= 0x80000022)
	{
		CPUID_0x80000022 leaf80000022 = {
			.EAX = {0}, .EBX = {0}, .ECX = {0}, .EDX = {0}
		};
		__asm__ volatile
		(
			"movq	$0x80000022, %%rax	\n\t"
			"xorq	%%rbx, %%rbx		\n\t"
			"xorq	%%rcx, %%rcx		\n\t"
			"xorq	%%rdx, %%rdx		\n\t"
			"cpuid				\n\t"
			"mov	%%eax, %0		\n\t"
			"mov	%%ebx, %1		\n\t"
			"mov	%%ecx, %2		\n\t"
			"mov	%%edx, %3"
			: "=r" (leaf80000022.EAX),
			  "=r" (leaf80000022.EBX),
			  "=r" (leaf80000022.ECX),
			  "=r" (leaf80000022.EDX)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);

	    iArg->Features->PerfMon.EAX.Version += leaf80000022.EAX.PerfMonV2;

	  if (leaf80000022.EBX.NumPerfCtrCore > 0) {
	    iArg->Features->PerfMon.EAX.MonCtrs=leaf80000022.EBX.NumPerfCtrCore;
	  }
	  if (leaf80000022.EBX.NumPerfCtrNB > 0) {
	    iArg->Features->Factory.PMC.NB = leaf80000022.EBX.NumPerfCtrNB;
	  }
	}
	BrandFromCPUID(iArg->Brand);
	BrandCleanup(iArg->Features->Info.Brand, iArg->Brand);
    }
}

static void Compute_Interval(void)
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
/*			udelay() built with TSC implementation		*/
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

static CLOCK Compute_Clock(unsigned int cpu, COMPUTE_ARG *pCompute)
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

inline void ClockToHz(CLOCK *clock)
{
	clock->Hz  = clock->Q * 1000000L;
	clock->Hz += clock->R * PRECISION;
}

/* [Genuine Intel] */
static CLOCK BaseClock_GenuineIntel(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};

	if ((PUBLIC(RO(Proc))->Features.Factory.Freq > 0) && (ratio > 0))
	{
		clock.Hz = (PUBLIC(RO(Proc))->Features.Factory.Freq * 1000000L)
			/ ratio;

		clock.Q = PUBLIC(RO(Proc))->Features.Factory.Freq / ratio;

		clock.R = (PUBLIC(RO(Proc))->Features.Factory.Freq % ratio)
			* PRECISION;
	}
	return clock;
};

/* [Authentic AMD] */
static CLOCK BaseClock_AuthenticAMD(unsigned int ratio)
{	/* For AMD Families 0Fh, 10h up to 16h */
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
	UNUSED(ratio);
	return clock;
};

/* [Core] */
static CLOCK BaseClock_Core(unsigned int ratio)
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
	return clock;
};

/* [Core2] */
static CLOCK BaseClock_Core2(unsigned int ratio)
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
	return clock;
};

/* [Atom] */
static CLOCK BaseClock_Atom(unsigned int ratio)
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
	return clock;
};

/* [Airmont] */
static CLOCK BaseClock_Airmont(unsigned int ratio)
{
	CLOCK clock = {.Q = 87, .R = 5};
	FSB_FREQ FSB = {.value = 0};

	RDMSR(FSB, MSR_FSB_FREQ);
	switch (FSB.Airmont.Bus_Speed) {
	case 0b0000: {
		clock.Q = 83;
		clock.R = 3333;
		}
		break;
	case 0b0001: {
		clock.Q = 100;
		clock.R = 0000;
		}
		break;
	case 0b0010: {
		clock.Q = 133;
		clock.R = 3333;
		}
		break;
	case 0b0011: {
		clock.Q = 116;
		clock.R = 6666;
		}
		break;
	case 0b0100: {
		clock.Q = 80;
		clock.R = 0000;
		}
		break;
	case 0b0101: {
		clock.Q = 93;
		clock.R = 3333;
		}
		break;
	case 0b0110: {
		clock.Q = 90;
		clock.R = 0000;
		}
		break;
	case 0b0111: {
		clock.Q = 88;
		clock.R = 9000;
		}
		break;
	case 0b1000:
		break;
	}
	ClockToHz(&clock);
	clock.R *= ratio;
	return clock;
};

/* [Silvermont] */
static CLOCK BaseClock_Silvermont(unsigned int ratio)
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
	return clock;
};

/* [Nehalem] */
static CLOCK BaseClock_Nehalem(unsigned int ratio)
{
	CLOCK clock = {.Q = 133, .R = 3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return clock;
};

/* [Westmere] */
static CLOCK BaseClock_Westmere(unsigned int ratio)
{
	CLOCK clock = {.Q = 133, .R = 3333};
	ClockToHz(&clock);
	clock.R *= ratio;
	return clock;
};

/* [SandyBridge] */
static CLOCK BaseClock_SandyBridge(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return clock;
};

/* [IvyBridge] */
static CLOCK BaseClock_IvyBridge(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return clock;
};

/* [Haswell] */
static CLOCK BaseClock_Haswell(unsigned int ratio)
{
	CLOCK clock = {.Q = 100, .R = 0};
	ClockToHz(&clock);
	clock.R *= ratio;
	return clock;
};

/* [Skylake] */
static CLOCK BaseClock_Skylake(unsigned int ratio)
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
	return clock;
};

static CLOCK BaseClock_AMD_Family_17h(unsigned int ratio)
{	/* Source: AMD PPR Family 17h Chap. 1.4/ Table 11: REFCLK = 100 MHz */
	CLOCK clock = {.Q = 100, .R = 0, .Hz = 100000000L};
	UNUSED(ratio);
	return clock;
};

static void Define_CPUID(CORE_RO *Core, const CPUID_STRUCT CpuIDforVendor[])
{	/*	Per vendor, define a CPUID dump table to query .	*/
	enum CPUID_ENUM i;
	for (i = 0; i < CPUID_MAX_FUNC; i++) {
		Core->CpuID[i].func = CpuIDforVendor[i].func;
		Core->CpuID[i].sub  = CpuIDforVendor[i].sub;
	}
}

static void Cache_Topology(CORE_RO *Core)
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

static unsigned int L3_SubCache_AMD_Piledriver(unsigned int bits)
{	/* Return the AMD Piledriver L3 Sub-Cache size in unit of 512 KB  */
	switch (bits) {
	case 0xc:
		return 4;
	case 0xd:
	case 0xe:
		return 2;
	default:
		return 0;
	}
}

static void Map_AMD_Topology(void *arg)
{
    if (arg != NULL)
    {
	CORE_RO *Core = (CORE_RO *) arg;

	struct CPUID_0x00000001_EBX leaf1_ebx = {0};

	CPUID_0x80000008 leaf80000008 = {
		.EAX = {0}, .EBX = {0}, .ECX = {{0}}, .EDX = {0}
	};

	bool CPU_Complex = true;

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
			.EAX = {0}, .EBX = {{0}}, .ECX = {{0}}, .EDX = {{0}}
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
		fallthrough;
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
		fallthrough;
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
	/* Zen APU */
	case AMD_Zen_APU:
	case AMD_ZenPlus_APU:
	case AMD_Zen_Dali:
	case AMD_Zen2_Renoir:
	case AMD_Zen2_LCN:
	case AMD_Zen2_Ariel:
	case AMD_Zen2_Jupiter:
	case AMD_Zen2_Galileo:
	case AMD_Zen2_MDN:
	case AMD_Zen3_CZN:
	case AMD_Zen3Plus_RMB:
	case AMD_Zen4_PHX:
	case AMD_Zen4_PHXR:
	case AMD_Zen4_PHX2:
	case AMD_Zen4_HWK:
	case AMD_Zen5_STX:
		CPU_Complex = false;
		fallthrough;
	/* Zen CPU Complex */
	case AMD_Zen:
	case AMD_ZenPlus:
	case AMD_EPYC_Rome_CPK:
	case AMD_Zen2_MTS:
	case AMD_Zen3_VMR:
	case AMD_EPYC_Milan:
	case AMD_Zen3_Chagall:
	case AMD_Zen3_Badami:
	case AMD_Zen4_Genoa:
	case AMD_Zen4_RPL:
	case AMD_Zen4_Bergamo:
	case AMD_Zen4_STP:
	case AMD_Zen5_Eldora:
	case AMD_Zen5_Turin:
	case AMD_Zen5_Turin_Dense:
	case AMD_Family_17h:
	case Hygon_Family_18h:
	case AMD_Family_19h:
	case AMD_Family_1Ah:
	    if (PUBLIC(RO(Proc))->Features.ExtInfo.ECX.ExtApicId == 1)
	    {
		struct CACHE_INFO CacheInfo = {
			.AX = 0, .BX = 0, .CX = 0, .DX = 0, .Size = 0
		};
		CPUID_0x8000001e leaf8000001e = {
			.EAX = {0}, .EBX = {{0}}, .ECX = {0}, .EDX = {0}
		};
		/*	Fn8000_001D Cache Properties.			*/
		unsigned long idx, level[CACHE_MAX_LEVEL] = {1, 0, 2, 3};
		/*	Skip one CDD on two with Threadripper.		*/
		unsigned int factor = 0;

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

	      if (leaf8000001e.EBX.ThreadsPerCore > 0)
	      { 	/*		SMT is enabled .		*/
			Core->T.ThreadID  = leaf8000001e.EAX.ExtApicId & 1;

		/* CCD factor for [x24 ... x384] SMT EPYC & Threadripper */
		factor	=  (leaf80000008.ECX.NC == 0xff)
			|| (leaf80000008.ECX.NC == 0xdf)
			|| (leaf80000008.ECX.NC == 0xbf)
			|| (leaf80000008.ECX.NC == 0xa7)
			|| (leaf80000008.ECX.NC == 0x8f)
			|| (leaf80000008.ECX.NC == 0x7f)
			|| (leaf80000008.ECX.NC == 0x5f)
			|| (leaf80000008.ECX.NC == 0x3f)
			|| (leaf80000008.ECX.NC == 0x2f)

			|| ((leaf80000008.ECX.F1Ah.NC == 0x17f)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin_Dense)))

			|| ((leaf80000008.ECX.F1Ah.NC == 0x13f)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin_Dense)))

			|| ((leaf80000008.ECX.F1Ah.NC == 0x11f)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin_Dense)))

			|| ((leaf80000008.ECX.NC == 0x1f)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Rome_CPK)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Milan)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Chagall)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Badami)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Genoa)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Bergamo)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_STP)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin_Dense)))

			|| ((leaf80000008.ECX.NC == 0x17)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Rome_CPK)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Milan)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Chagall)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Badami)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Genoa)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Bergamo)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_STP)));
	      }
	      else
	      { 	/*		SMT is disabled.		*/
			Core->T.ThreadID  = 0;

		/* CCD factor for [x12 ... x192] physical EPYC & Threadripper */
		factor	=  (leaf80000008.ECX.NC == 0xbf)
			|| (leaf80000008.ECX.NC == 0x9f)
			|| (leaf80000008.ECX.NC == 0x8f)
			|| (leaf80000008.ECX.NC == 0x7f)
			|| (leaf80000008.ECX.NC == 0x6f)
			|| (leaf80000008.ECX.NC == 0x5f)
			|| (leaf80000008.ECX.NC == 0x53)
			|| (leaf80000008.ECX.NC == 0x3f)
			|| (leaf80000008.ECX.NC == 0x2f)
			|| (leaf80000008.ECX.NC == 0x1f)
			|| (leaf80000008.ECX.NC == 0x17)

			|| ((leaf80000008.ECX.NC == 0x0f)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Rome_CPK)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Milan)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Chagall)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Badami)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Genoa)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Bergamo)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_STP)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin)
			 || (PUBLIC(RO(Proc))->ArchID == AMD_Zen5_Turin_Dense)))

			|| ((leaf80000008.ECX.NC == 0x0b)
			 && ((PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Rome_CPK)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_EPYC_Milan)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Chagall)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen3_Badami)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Genoa)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_Bergamo)
			  || (PUBLIC(RO(Proc))->ArchID == AMD_Zen4_STP)));
	      }
		/* CCD has to remain within range values from 0 to 7	*/
		factor = factor & (Core->T.CoreID < 32);

		Core->T.Cluster.Node = leaf8000001e.ECX.NodeId;

	      if (CPU_Complex == true ) {
		Core->T.Cluster.CCD = (Core->T.CoreID >> 3) << factor;
		Core->T.Cluster.CCX = Core->T.CoreID >> 2;
	      }
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
 Sources: Intel Software Developer's Manual vol 3A Chap. 8.9 /
	  Intel whitepaper: Detecting Hyper-Threading Technology /
*/
inline unsigned short FindMaskWidth(unsigned short maxCount)
{
	unsigned short maskWidth = 0, count = (maxCount - 1);

	if (BITBSR(count, maskWidth) == 0)
		maskWidth++;

	return maskWidth;
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

	if (CORE_Mask_Width != 0) {
	   SMT_Mask_Width = FindMaskWidth(SMT_Mask_Width) / CORE_Mask_Width;
	}
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

	if ((PUBLIC(RO(Proc))->Features.Info.LargestStdFunc >= 0xa)
	  && PUBLIC(RO(Proc))->Features.ExtFeature.EDX.Hybrid)
	{
		__asm__ volatile
		(
			"movq	$0x1a, %%rax	\n\t"
			"xorq	%%rbx, %%rbx	\n\t"
			"xorq	%%rcx, %%rcx	\n\t"
			"xorq	%%rdx, %%rdx	\n\t"
			"cpuid			\n\t"
			"mov	%%eax, %0"
			: "=r" (Core->T.Cluster.ID)
			:
			: "%rax", "%rbx", "%rcx", "%rdx"
		);
	}

	Cache_Topology(Core);
    }
}

static int Core_Topology(unsigned int cpu)
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
	return rc;
}

static unsigned int Proc_Topology(void)
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

static void Package_Init_Reset(void)
{
	PUBLIC(RO(Proc))->Features.TgtRatio_Unlock = 1;
	PUBLIC(RO(Proc))->Features.ClkRatio_Unlock = 0;
	PUBLIC(RO(Proc))->Features.TDP_Unlock = 0;
	PUBLIC(RO(Proc))->Features.Turbo_Unlock = 0;
	PUBLIC(RO(Proc))->Features.TurboActiv_Lock = 1;
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Lock = 1;
	PUBLIC(RO(Proc))->Features.Uncore_Unlock = 0;
}

static void Default_Unlock_Reset(void)
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
    switch (HSMP_Attempt) {
    case COREFREQ_TOGGLE_OFF:
    case COREFREQ_TOGGLE_ON:
	PUBLIC(RO(Proc))->Features.HSMP_Capable = HSMP_Attempt;
	break;
    }
}

static void OverrideCodeNameString(PROCESSOR_SPECIFIC *pSpecific)
{
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[
			PUBLIC(RO(Proc))->ArchID
		].Architecture[pSpecific->CodeNameIdx], CODENAME_LEN);
}

static void OverrideUnlockCapability(PROCESSOR_SPECIFIC *pSpecific)
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

static PROCESSOR_SPECIFIC *LookupProcessor(void)
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

static void Intel_FlexRatio(void)
{
    static struct {
		struct SIGNATURE Arch;
		unsigned short	grantFlex	:  1-0,
				experimental	:  2-1,
				freeToUse	:  8-2,
				bitsLayout	: 16-8;
    } list[] = {
		{_Core_Yonah,		0, 1, 0, 1},
		{_Core_Conroe,		0, 1, 0, 1},
		{_Core_Kentsfield,	0, 1, 0, 1},
		{_Core_Conroe_616,	0, 1, 0, 1},
		{_Core_Penryn,		1, 1, 0, 1},	/* 06_17 */
		{_Core_Dunnington,	0, 1, 0, 1},

		{_Atom_Bonnell, 	0, 1, 0, 0},	/* 06_1C */
		{_Atom_Silvermont,	0, 1, 0, 0},	/* 06_26 */
		{_Atom_Lincroft,	0, 1, 0, 0},	/* 06_27 */
		{_Atom_Clover_Trail,	0, 1, 0, 0},	/* 06_35 */
		{_Atom_Saltwell,	0, 1, 0, 0},	/* 06_36 */

		{_Silvermont_Bay_Trail, 0, 1, 0, 0},	/* 06_37 */

		{_Atom_Avoton,		0, 1, 0, 0},	/* 06_4D */
		{_Atom_Airmont, 	0, 1, 0, 0},	/* 06_4C */
		{_Atom_Goldmont,	1, 1, 0, 0},	/* 06_5C */
		{_Atom_Sofia,		1, 1, 0, 0},	/* 06_5D */
		{_Atom_Merrifield,	1, 1, 0, 0},	/* 06_4A */
		{_Atom_Moorefield,	1, 1, 0, 0},	/* 06_5A */

		{_Nehalem_Bloomfield,	0, 0, 0, 1},	/* 06_1A */
		{_Nehalem_Lynnfield,	1, 1, 0, 1},	/* 06_1E */
		{_Nehalem_MB,		1, 1, 0, 1},	/* 06_1F */
		{_Nehalem_EX,		1, 1, 0, 1},	/* 06_2E */

		{_Westmere,		1, 1, 0, 1},	/* 06_25 */
		{_Westmere_EP,		1, 0, 0, 1},	/* 06_2C : R/W */
		{_Westmere_EX,		1, 1, 0, 1},	/* 06_2F */

		{_SandyBridge,		1, 1, 0, 0},	/* 06_2A */
		{_SandyBridge_EP,	1, 1, 0, 0},	/* 06_2D */

		{_IvyBridge,		1, 0, 0, 0},	/* 06_3A */
		{_IvyBridge_EP, 	1, 1, 0, 0},	/* 06_3E */

		{_Haswell_DT,		1, 1, 0, 0},	/* 06_3C */
		{_Haswell_EP,		1, 1, 0, 0},	/* 06_3F */
		{_Haswell_ULT,		1, 1, 0, 0},	/* 06_45 */
		{_Haswell_ULX,		1, 1, 0, 0},	/* 06_46 */

		{_Broadwell,		1, 1, 0, 0},	/* 06_3D */
		{_Broadwell_D,		1, 1, 0, 0},	/* 06_56 */
		{_Broadwell_H,		1, 1, 0, 0},	/* 06_47 */
		{_Broadwell_EP, 	1, 1, 0, 0},	/* 06_4F */

		{_Skylake_UY,		1, 1, 0, 0},	/* 06_4E */
		{_Skylake_S,		1, 1, 0, 0},	/* 06_5E */
		{_Skylake_X,		1, 1, 0, 0},	/* 06_55 */

		{_Xeon_Phi,		0, 1, 0, 0},	/* 06_57 */

		{_Kabylake,		1, 1, 0, 0},	/* 06_9E */
		{_Kabylake_UY,		1, 1, 0, 0},	/* 06_8E */

		{_Cannonlake_U, 	1, 1, 0, 0},	/* 06_66 */
		{_Cannonlake_H, 	1, 1, 0, 0},
		{_Geminilake,		1, 1, 0, 0},	/* 06_7A */
		{_Icelake_UY,		1, 1, 0, 0},	/* 06_7E */

		{_Icelake_X,		1, 1, 0, 0},
		{_Icelake_D,		1, 1, 0, 0},
		{_Sunny_Cove,		1, 1, 0, 0},
		{_Tigerlake,		1, 1, 0, 0},
		{_Tigerlake_U,		1, 0, 0, 0},	/* 06_8C : R/W */
		{_Cometlake,		1, 1, 0, 0},
		{_Cometlake_UY, 	1, 1, 0, 0},
		{_Atom_Denverton,	1, 1, 0, 0},
		{_Tremont_Jacobsville,	1, 1, 0, 0},
		{_Tremont_Lakefield,	1, 1, 0, 0},
		{_Tremont_Elkhartlake,	1, 1, 0, 0},
		{_Tremont_Jasperlake,	1, 1, 0, 0},
		{_Sapphire_Rapids,	1, 1, 0, 0},
		{_Emerald_Rapids,	1, 1, 0, 0},
		{_Granite_Rapids_X,	1, 1, 0, 0},
		{_Granite_Rapids_D,	1, 1, 0, 0},
		{_Sierra_Forest,	1, 1, 0, 0},
		{_Grand_Ridge,		1, 1, 0, 0},
		{_Rocketlake,		1, 1, 0, 0},
		{_Rocketlake_U, 	1, 1, 0, 0},
		{_Alderlake_S,		1, 0, 0, 0},	/* 06_97 */
		{_Alderlake_H,		1, 0, 0, 0},	/* 06_9A */
		{_Alderlake_N,		1, 1, 0, 0},
		{_Meteorlake_M, 	1, 1, 0, 0},
		{_Meteorlake_N, 	1, 1, 0, 0},
		{_Meteorlake_S, 	1, 1, 0, 0},
		{_Raptorlake,		1, 0, 0, 0},	/* 06_B7 */
		{_Raptorlake_P, 	1, 1, 0, 0},
		{_Raptorlake_S, 	1, 1, 0, 0},
		{_Lunarlake,		1, 1, 0, 0},	/* 06_BD */
		{_Arrowlake,		1, 1, 0, 0},	/* 06_C6 */
		{_Arrowlake_H,		1, 1, 0, 0},	/* 06_C5 */
		{_Arrowlake_U,		1, 1, 0, 0},	/* 06_B5 */
		{_Pantherlake,		1, 1, 0, 0},	/* 06_CC */
		{_Clearwater_Forest,	1, 1, 0, 0}	/* 06_DD */
    };
	const unsigned int ids = sizeof(list) / sizeof(list[0]);
	unsigned int id;
  for (id = 0; id < ids; id++) {
   if ((list[id].Arch.ExtFamily == PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
    && (list[id].Arch.Family == PUBLIC(RO(Proc))->Features.Std.EAX.Family)
    && (list[id].Arch.ExtModel == PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
    && (list[id].Arch.Model == PUBLIC(RO(Proc))->Features.Std.EAX.Model))
    {
	if (list[id].grantFlex) {
	  if (!list[id].experimental
	   || (list[id].experimental
	   && PUBLIC(RO(Proc))->Registration.Experimental))
	  {
		FLEX_RATIO flexReg = {.value = 0};
		RDMSR(flexReg, MSR_FLEX_RATIO);

	    switch (list[id].bitsLayout) {
	    default:
	    case 0:
		PUBLIC(RO(Proc))->Features.OC_Enable = flexReg.OC_ENABLED;
		PUBLIC(RO(Proc))->Features.Factory.Bins = flexReg.OC_BINS;
		PUBLIC(RO(Proc))->Features.OC_Lock = flexReg.OC_LOCK;
		break;
	    case 1:
		PUBLIC(RO(Proc))->Features.OC_Enable = flexReg.OC_ENABLED;
		PUBLIC(RO(Proc))->Features.Factory.Bins=flexReg.CLOCK_FLEX_MAX;
		break;
	    }
		PUBLIC(RO(Proc))->Features.Factory.Overclock = \
		ABS_FREQ_MHz(	signed int,
				PUBLIC(RO(Proc))->Features.Factory.Bins,
				PUBLIC(RO(Proc))->Features.Factory.Clock );
	  }
	}
	break;
    }
   }
}

static int Intel_MaxBusRatio(PLATFORM_ID *PfID)
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
		_Atom_Bonnell,		/* 06_1C */
		_Atom_Airmont		/* 06_4C */
	};
	const int ids = sizeof(whiteList) / sizeof(whiteList[0]);
	int id;
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
			return 0;
		}
	}
	return -1;
}

static void Intel_Core_Platform_Info(unsigned int cpu)
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
	if (PerfStatus.value != 0) {			/* Chap. 18.18.3.4 */
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

static void Compute_Intel_Core_Burst(unsigned int cpu)
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

static PLATFORM_INFO Intel_Platform_Info(unsigned int cpu)
{
	PLATFORM_INFO PfInfo = {.value = 0};
	RDMSR(PfInfo, MSR_PLATFORM_INFO);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = PfInfo.MinimumRatio;
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = PfInfo.MaxNonTurboRatio;

	return PfInfo;
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

static void Assign_8C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
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

static void Assign_15C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(15C)] = pConfig->Cfg1.IVB_EP.MaxRatio_15C;
	pBoost[BOOST(14C)] = pConfig->Cfg1.IVB_EP.MaxRatio_14C;
	pBoost[BOOST(13C)] = pConfig->Cfg1.IVB_EP.MaxRatio_13C;
	pBoost[BOOST(12C)] = pConfig->Cfg1.IVB_EP.MaxRatio_12C;
	pBoost[BOOST(11C)] = pConfig->Cfg1.IVB_EP.MaxRatio_11C;
	pBoost[BOOST(10C)] = pConfig->Cfg1.IVB_EP.MaxRatio_10C;
	pBoost[BOOST(9C) ] = pConfig->Cfg1.IVB_EP.MaxRatio_9C;
}

static void Assign_16C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
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

static void Assign_18C_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
{
	pBoost[BOOST(18C)] = pConfig->Cfg2.MaxRatio_18C;
	pBoost[BOOST(17C)] = pConfig->Cfg2.MaxRatio_17C;
}

static void Assign_SKL_X_Boost(unsigned int *pBoost, TURBO_CONFIG *pConfig)
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

static long For_All_Turbo_Clock(CLOCK_ARG *pClockMod, void (*ConfigFunc)(void*))
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

	return rc;
}

static void Intel_Turbo_Cfg8C_PerCore(void *arg)
{
	CLOCK_TURBO_ARG *pClockCfg8C = (CLOCK_TURBO_ARG *) arg;
	unsigned int registerAddress = MSR_TURBO_RATIO_LIMIT;

	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.Hybrid == 1)
	{
		CORE_RO *Core;
		unsigned int cpu = smp_processor_id();
		Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

		if (Core->T.Cluster.Hybrid.CoreType == Hybrid_Atom) {
			registerAddress = MSR_SECONDARY_TURBO_RATIO_LIMIT;
		}
	} else if (PUBLIC(RO(Proc))->ArchID == Atom_Airmont) {
		registerAddress = MSR_ATOM_CORE_TURBO_RATIOS;
	}
	RDMSR(pClockCfg8C->Config.Cfg0, registerAddress);

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
	WRMSR(pClockCfg8C->Config.Cfg0, registerAddress);
	RDMSR(pClockCfg8C->Config.Cfg0, registerAddress);
	pClockCfg8C->rc = RC_OK_COMPUTE;
      }
    } else {
	pClockCfg8C->rc = -RC_TURBO_PREREQ;
    }
  }
}

static long Intel_Turbo_Config8C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg8C_PerCore);

	return rc;
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

static long Intel_Turbo_Config15C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg15C_PerCore);

	return rc;
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

static long Intel_Turbo_Config16C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg16C_PerCore);

	return rc;
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

static long Intel_Turbo_Config18C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg18C_PerCore);

	return rc;
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

static long Skylake_X_Turbo_Config16C(CLOCK_ARG *pClockMod)
{
	long rc = For_All_Turbo_Clock(pClockMod, Intel_Turbo_Cfg_SKL_X_PerCore);

	return rc;
}

static long TurboClock_IvyBridge_EP(CLOCK_ARG *pClockMod)
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
	return rc;
}

static long TurboClock_Haswell_EP(CLOCK_ARG *pClockMod)
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
	return rc;
}

static long TurboClock_Broadwell_EP(CLOCK_ARG *pClockMod)
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
	return rc;
}

static long TurboClock_Skylake_X(CLOCK_ARG *pClockMod)
{
	long rc = Intel_Turbo_Config8C(pClockMod);
	if (rc >= RC_SUCCESS) {
		rc = Skylake_X_Turbo_Config16C(pClockMod);
	}
	return rc;
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

static void Intel_Hardware_Performance(void)
{
	PM_ENABLE PM_Enable = {.value = 0};
	HDC_CONTROL HDC_Control = {.value = 0};

	if (PUBLIC(RO(Proc))->Features.Power.EAX.HWP_Reg)
	{
		/*	MSR_IA32_PM_ENABLE is a Package register.	*/
		RDMSR(PM_Enable, MSR_IA32_PM_ENABLE);
		/*	Is the HWP requested and its current state is off ? */
	  if ((HWP_Enable == COREFREQ_TOGGLE_ON) && (PM_Enable.HWP_Enable == 0))
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

	    if (BITCMP_CC(LOCKLESS, \
			PUBLIC(RW(Proc))->HWP, PUBLIC(RO(Proc))->HWP_Mask) )
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

static void Skylake_PowerControl(void)
{
	unsigned short WrRdMSR = 0;

	POWER_CONTROL PowerCtrl = {.value = 0};
	RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);

	switch (EEO_Disable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PowerCtrl.EEO_Disable = EEO_Disable;
			WrRdMSR = 1;
		break;
	}
	switch (R2H_Disable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PowerCtrl.R2H_Disable = R2H_Disable;
			WrRdMSR = 1;
		break;
	}
	if (WrRdMSR) {
		WRMSR(PowerCtrl, MSR_IA32_POWER_CTL);
		RDMSR(PowerCtrl, MSR_IA32_POWER_CTL);
	}
	PUBLIC(RO(Proc))->Features.EEO_Enable = !PowerCtrl.EEO_Disable;
	PUBLIC(RO(Proc))->Features.R2H_Enable = !PowerCtrl.R2H_Disable;
}

static void SandyBridge_Uncore_Ratio(unsigned int cpu)
{
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)];
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)];
}

static long Haswell_Uncore_Ratio(CLOCK_ARG *pClockMod)
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

	return rc;
}

static void Nehalem_PowerLimit(void)
{
	unsigned short WrRdMSR = 0;

	NEHALEM_POWER_LIMIT PowerLimit = {.value = 0};
	RDMSR(PowerLimit, MSR_TURBO_POWER_CURRENT_LIMIT);

    if (Custom_TDP_Count > 0) {
	if (Custom_TDP_Offset[PWR_DOMAIN(PKG)] != 0)
	{	/*	Register is an 8 watt multiplier	*/
		signed short	TDP_Limit = PowerLimit.TDP_Limit >> 3;
				TDP_Limit += Custom_TDP_Offset[PWR_DOMAIN(PKG)];
	    if (TDP_Limit >= 0)
	    {
		PowerLimit.TDP_Limit = TDP_Limit << 3;
		WrRdMSR = 1;
	    }
	}
    }
    if (TDP_Limiting_Count > 0) {
	switch (Activate_TDP_Limit[PWR_DOMAIN(PKG)]) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PowerLimit.TDP_Override = Activate_TDP_Limit[PWR_DOMAIN(PKG)];
		WrRdMSR = 1;
		break;
	}
    }
    if (Custom_TDC_Offset != 0) {
	signed short	TDC_Limit = PowerLimit.TDC_Limit >> 3;
			TDC_Limit += Custom_TDC_Offset;
	if (TDC_Limit > 0)
	{
		PowerLimit.TDC_Limit = TDC_Limit << 3;
		WrRdMSR = 1;
	}
    }
	switch (Activate_TDC_Limit) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PowerLimit.TDC_Override = Activate_TDC_Limit;
		WrRdMSR = 1;
		break;
	}
	if (WrRdMSR) {
		if (PUBLIC(RO(Proc))->Features.TDP_Unlock) {
			WRMSR(PowerLimit, MSR_TURBO_POWER_CURRENT_LIMIT);
			RDMSR(PowerLimit, MSR_TURBO_POWER_CURRENT_LIMIT);
		}
	}
	PUBLIC(RO(Proc))->PowerThermal.Domain[PWR_DOMAIN(PKG)].Unlock = \
					PUBLIC(RO(Proc))->Features.TDP_Unlock;

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Domain_Limit1 = PowerLimit.TDP_Limit;

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Enable_Limit1 = PowerLimit.TDP_Override;
	/*	TDP: 1/(2 << (3-1)) = 1/8 watt	*/
	PUBLIC(RO(Proc))->PowerThermal.Unit.PU = 3;
	/*	TDC: 1/8 amp			*/
	PUBLIC(RO(Proc))->PowerThermal.TDC = PowerLimit.TDC_Limit >> 3;
	PUBLIC(RO(Proc))->PowerThermal.Enable_Limit.TDC=PowerLimit.TDC_Override;
}

static void Intel_PowerInterface(void)
{
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.Unit, MSR_RAPL_POWER_UNIT);
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.PowerInfo, MSR_PKG_POWER_INFO);
}

static void Intel_DomainPowerLimit(	unsigned int MSR_DOMAIN_POWER_LIMIT,
					unsigned long long PowerLimitLockMask,
					enum PWR_DOMAIN pw )
{
	const unsigned int lt = PWR_LIMIT_SIZE * pw, rt = 1 + lt;
	unsigned short WrRdMSR = 0;

	DOMAIN_POWER_LIMIT PowerLimit = {.value = 0};
	RDMSR(PowerLimit, MSR_DOMAIN_POWER_LIMIT);

	PUBLIC(RO(Proc))->PowerThermal.Domain[pw].Unlock = \
	!((PowerLimit.value & PowerLimitLockMask) == PowerLimitLockMask);

    if (PUBLIC(RO(Proc))->PowerThermal.Unit.PU > 0)
    {
	unsigned short	pwrUnits = PUBLIC(RO(Proc))->PowerThermal.Unit.PU - 1;
			pwrUnits = 2 << pwrUnits;

	if (Custom_TDP_Count > lt) {
	    if (Custom_TDP_Offset[lt] != 0)
	    {
		signed short	TDP_Limit = PowerLimit.Domain_Limit1 / pwrUnits;
				TDP_Limit = TDP_Limit + Custom_TDP_Offset[lt];
		if (TDP_Limit >= 0) {
			PowerLimit.Domain_Limit1 = pwrUnits * TDP_Limit;
			WrRdMSR = 1;
		}
	    }
	}
	if (PowerLimitLockMask == PKG_POWER_LIMIT_LOCK_MASK) {
	  if (Custom_TDP_Count > rt) {
	    if (Custom_TDP_Offset[rt] != 0)
	    {
		signed short	TDP_Limit = PowerLimit.Domain_Limit2 / pwrUnits;
				TDP_Limit = TDP_Limit + Custom_TDP_Offset[rt];
		if (TDP_Limit >= 0) {
			PowerLimit.Domain_Limit2 = pwrUnits * TDP_Limit;
			WrRdMSR = 1;
		}
	    }
	  }
	}
	switch (pw) {
	case PWR_DOMAIN(PKG):
	case PWR_DOMAIN(CORES):
	case PWR_DOMAIN(UNCORE):
	case PWR_DOMAIN(RAM):
	case PWR_DOMAIN(PLATFORM):
		if (PowerTimeWindow_Count > lt) {
			if ((PowerTimeWindow[lt] >= 0)
			 && (PowerTimeWindow[lt] <= 0b1111111))
			{
				PowerLimit.TimeWindow1 = PowerTimeWindow[lt];
				WrRdMSR = 1;
			}
		}
		break;
	case PWR_DOMAIN(SIZE):
		break;
	}
	switch (pw) {
	case PWR_DOMAIN(PLATFORM):
		if (!PUBLIC(RO(Proc))->Registration.Experimental) {
			break;
		}
		fallthrough;
	case PWR_DOMAIN(PKG):
		if (PowerTimeWindow_Count > rt) {
			if ((PowerTimeWindow[rt] >= 0)
			 && (PowerTimeWindow[rt] <= 0b1111111))
			{
				PowerLimit.TimeWindow2 = PowerTimeWindow[rt];
				WrRdMSR = 1;
			}
		}
		break;
	case PWR_DOMAIN(CORES):
	case PWR_DOMAIN(UNCORE):
	case PWR_DOMAIN(RAM):
	case PWR_DOMAIN(SIZE):
		break;
	}
    }
	if (TDP_Limiting_Count > lt) {
		switch (Activate_TDP_Limit[lt]) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PowerLimit.Enable_Limit1 = Activate_TDP_Limit[lt];
			WrRdMSR = 1;
			break;
		}
	}
	switch (pw) {
	case PWR_DOMAIN(RAM):
		if (!PUBLIC(RO(Proc))->Registration.Experimental) {
			break;
		}
		fallthrough;
	default:
		if (TDP_Clamping_Count > lt) {
			switch (Activate_TDP_Clamp[lt]) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
			    if (PUBLIC(RO(Proc))->Features.Power.EAX.PLN) {
				PowerLimit.Clamping1 = Activate_TDP_Clamp[lt];
				WrRdMSR = 1;
			    }
				break;
			}
		}
		break;
	}
	if (PowerLimitLockMask == PKG_POWER_LIMIT_LOCK_MASK) {
	    if (TDP_Limiting_Count > rt) {
		switch (Activate_TDP_Limit[rt]) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PowerLimit.Enable_Limit2 = Activate_TDP_Limit[rt];
			WrRdMSR = 1;
			break;
		}
	    }
	    switch (pw) {
	    case PWR_DOMAIN(RAM):
		if (!PUBLIC(RO(Proc))->Registration.Experimental) {
			break;
		}
		fallthrough;
	    default:
		if (TDP_Clamping_Count > rt) {
			switch (Activate_TDP_Clamp[rt]) {
			case COREFREQ_TOGGLE_OFF:
			case COREFREQ_TOGGLE_ON:
				PowerLimit.Clamping2 = Activate_TDP_Clamp[rt];
				WrRdMSR = 1;
				break;
			}
		}
		break;
	    }
	}
	if (WrRdMSR && PUBLIC(RO(Proc))->PowerThermal.Domain[pw].Unlock) {
		WRMSR(PowerLimit, MSR_DOMAIN_POWER_LIMIT);
	}
	RDMSR(PowerLimit, MSR_DOMAIN_POWER_LIMIT);
	PUBLIC(RO(Proc))->PowerThermal.Domain[pw].PowerLimit = PowerLimit;
}

static void Intel_Pkg_CST_IRTL(const unsigned int MSR, PKGCST_IRTL *PCST)
{
	RDMSR((*PCST), MSR);
}

static long Intel_ThermalOffset(bool programmableTj)
{
	long rc = -EINVAL;
    if (ThermalOffset != 0) {
	if (programmableTj)
	{
		TJMAX TjMax = {.value = 0};
		RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);

		switch (PUBLIC(RO(Proc))->ArchID) {
		case Atom_Goldmont:
		case Xeon_Phi:	/*	TODO(06_85h)	*/ {
			const short offset = TjMax.Atom.Offset + ThermalOffset;
			if  ((offset >= 0) && (offset <= 0b111111)) {
				TjMax.Atom.Offset = offset;
				WRMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);
				RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);
				rc = TjMax.Atom.Offset == offset ? RC_OK_COMPUTE
							: -RC_UNIMPLEMENTED;
			}
		    }
			break;
		case IvyBridge_EP:
		case Broadwell_EP:
		case Broadwell_D:
		case Skylake_X: {
			const short offset = TjMax.EP.Offset + ThermalOffset;
			if ((offset >= 0) && (offset <= 0b1111)) {
				TjMax.EP.Offset = offset;
				WRMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);
				RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);
				rc = TjMax.EP.Offset == offset ? RC_OK_COMPUTE
							: -RC_UNIMPLEMENTED;
			}
		    }
			break;
		default: {
			const short offset = TjMax.Offset + ThermalOffset;
			if ((offset >= 0) && (offset <= 0b1111)) {
				TjMax.Offset = offset;
				WRMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);
				RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);
				rc = TjMax.Offset == offset ? RC_OK_COMPUTE
							: -RC_UNIMPLEMENTED;
			}
		    }
			break;
		case Core_Yonah ... Core_Dunnington:
			rc = -RC_UNIMPLEMENTED;
			break;
		}
	} else {
		rc = -RC_UNIMPLEMENTED;
	}
    } else {
	rc = RC_SUCCESS;
    }
	return rc;
}

static void Intel_Processor_PIN(bool capable)
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

static void AMD_Processor_PIN(bool capable)
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

static long AMD_F17h_CPPC(void)
{
	AMD_CPPC_ENABLE CPPC_Enable = {.value = 0};

    if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.CPPC)
    {
	RDMSR(CPPC_Enable, MSR_AMD_CPPC_ENABLE);
      if ((HWP_Enable == COREFREQ_TOGGLE_ON) && (CPPC_Enable.CPPC_Enable == 0))
      {
		CPPC_Enable.CPPC_Enable = 1;
		WRMSR(CPPC_Enable, MSR_AMD_CPPC_ENABLE);
		RDMSR(CPPC_Enable, MSR_AMD_CPPC_ENABLE);
      }
	PUBLIC(RO(Proc))->Features.HWP_Enable = CPPC_Enable.CPPC_Enable;

	return 0;
    }
	return -ENODEV;
}

static void Compute_ACPI_CPPC_Bounds(unsigned int cpu)
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

static signed int Get_ACPI_CPPC_Registers(unsigned int cpu, void *arg)
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
			#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
			.Desired	= CPPC_Perf.reference_perf,
			#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
			.Desired	= CPPC_Caps.reference_perf,
			#else
			.Desired	= 0,
			#endif
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

static signed int Get_EPP_ACPI_CPPC(unsigned int cpu)
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

static signed int Put_EPP_ACPI_CPPC(unsigned int cpu, signed short epp)
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

static signed int Set_EPP_ACPI_CPPC(unsigned int cpu, void *arg)
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

static signed int Read_ACPI_CPPC_Registers(unsigned int cpu, void *arg)
{
	signed int rc = Get_ACPI_CPPC_Registers(cpu, arg);

	Compute_ACPI_CPPC_Bounds(cpu);

	if (Get_EPP_ACPI_CPPC(cpu) == 0) {
		PUBLIC(RO(Proc))->Features.OSPM_EPP = 1;
	}
	Set_EPP_ACPI_CPPC(cpu, arg);

	return rc;
}

static signed int Read_ACPI_PCT_Registers(unsigned int cpu)
{
#if defined(CONFIG_ACPI)
	signed int rc = 0;

	struct acpi_processor *pr = per_cpu(processors, cpu);
    if (pr == NULL) {
	PUBLIC(RO(Proc))->Features.ACPI_PCT_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_PCT = 0;
	rc = -ENODEV;
    } else {
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	acpi_status status=acpi_evaluate_object(pr->handle,"_PCT",NULL,&buffer);
	if (ACPI_FAILURE(status)) {
		PUBLIC(RO(Proc))->Features.ACPI_PCT_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PCT = 0;
		rc = -ENODEV;
	} else {
		union acpi_object *pct = (union acpi_object *) buffer.pointer;
	    if ((pct == NULL) || (pct->type != ACPI_TYPE_PACKAGE)
	     || (pct->package.count < 2))
	    {
		PUBLIC(RO(Proc))->Features.ACPI_PCT_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PCT = 0;
		rc = -EFAULT;
	    } else {
		PUBLIC(RO(Proc))->Features.ACPI_PCT_CAP = 1;
		PUBLIC(RO(Proc))->Features.ACPI_PCT = 1;
	    }
	}
	kfree(buffer.pointer);
    }
	return rc;
#else
	PUBLIC(RO(Proc))->Features.ACPI_PCT_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_PCT = 0;
	return -ENODEV;
#endif
}

static signed int Read_ACPI_PSS_Registers(unsigned int cpu)
{
#if defined(CONFIG_ACPI)
	signed int rc = 0;

	struct acpi_processor *pr = per_cpu(processors, cpu);
    if (pr == NULL) {
	PUBLIC(RO(Proc))->Features.ACPI_PSS_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_PSS = 0;
	rc = -ENODEV;
    } else {
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	acpi_status status=acpi_evaluate_object(pr->handle,"_PSS",NULL,&buffer);
	if (ACPI_FAILURE(status)) {
		PUBLIC(RO(Proc))->Features.ACPI_PSS_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PSS = 0;
		rc = -ENODEV;
	} else {
		union acpi_object *pss = (union acpi_object *) buffer.pointer;
	    if ((pss == NULL) || (pss->type != ACPI_TYPE_PACKAGE)
	     || (pss->package.count == 0))
	    {
		PUBLIC(RO(Proc))->Features.ACPI_PSS_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PSS = 0;
		rc = -EFAULT;
	    } else {
		PUBLIC(RO(Proc))->Features.ACPI_PSS_CAP = 1;
		PUBLIC(RO(Proc))->Features.ACPI_PSS = pss->package.count & 0xf;
	    }
	}
	kfree(buffer.pointer);
    }
	return rc;
#else
	PUBLIC(RO(Proc))->Features.ACPI_PSS_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_PSS = 0;
	return -ENODEV;
#endif
}

static signed int Read_ACPI_PPC_Registers(unsigned int cpu)
{
#if defined(CONFIG_ACPI)
	signed int rc = 0;

	struct acpi_processor *pr = per_cpu(processors, cpu);
    if (pr == NULL) {
	PUBLIC(RO(Proc))->Features.ACPI_PPC_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_PPC = 0;
	rc = -ENODEV;
    } else {
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	acpi_status status=acpi_evaluate_object(pr->handle,"_PPC",NULL,&buffer);
	if (ACPI_FAILURE(status)) {
		PUBLIC(RO(Proc))->Features.ACPI_PPC_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PPC = 0;
		rc = -ENODEV;
	} else {
		union acpi_object *ppc = (union acpi_object *) buffer.pointer;
	    if ((ppc == NULL) || (ppc->type != ACPI_TYPE_INTEGER))
	    {
		PUBLIC(RO(Proc))->Features.ACPI_PPC_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PPC = 0;
		rc = -EFAULT;
	    } else {
		union acpi_object *obj = buffer.pointer;
	      if ((obj != NULL) && (obj->type == ACPI_TYPE_INTEGER)) {
		PUBLIC(RO(Proc))->Features.ACPI_PPC_CAP = 1;
		PUBLIC(RO(Proc))->Features.ACPI_PPC = obj->integer.value & 0xf;
	      } else {
		PUBLIC(RO(Proc))->Features.ACPI_PPC_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_PPC = 0;
	      }
	    }
	}
	kfree(buffer.pointer);
    }
	return rc;
#else
	PUBLIC(RO(Proc))->Features.ACPI_PPC_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_PPC = 0;
	return -ENODEV;
#endif
}

static signed int Read_ACPI_CST_Registers(unsigned int cpu)
{
#if defined(CONFIG_ACPI)
	signed int rc = 0;

	struct acpi_processor *pr = per_cpu(processors, cpu);
    if (pr == NULL) {
	PUBLIC(RO(Proc))->Features.ACPI_CST_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_CST = 0;
	rc = -ENODEV;
    } else {
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	acpi_status status=acpi_evaluate_object(pr->handle,"_CST",NULL,&buffer);
	if (ACPI_FAILURE(status)) {
		PUBLIC(RO(Proc))->Features.ACPI_CST_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_CST = 0;
		rc = -ENODEV;
	} else {
		union acpi_object *cst = (union acpi_object *) buffer.pointer;
	    if ((cst == NULL) || (cst->type != ACPI_TYPE_PACKAGE)
	     || (cst->package.count == 0))
	    {
		PUBLIC(RO(Proc))->Features.ACPI_CST_CAP = 0;
		PUBLIC(RO(Proc))->Features.ACPI_CST = 0;
		rc = -EFAULT;
	    } else {
		PUBLIC(RO(Proc))->Features.ACPI_CST_CAP = 1;
		PUBLIC(RO(Proc))->Features.ACPI_CST = \
				cst->package.elements[0].integer.value & 0xf;
	    }
	}
	kfree(buffer.pointer);
    }
	return rc;
#else
	PUBLIC(RO(Proc))->Features.ACPI_CST_CAP = 0;
	PUBLIC(RO(Proc))->Features.ACPI_CST = 0;
	return -ENODEV;
#endif
}

static void For_All_ACPI_CPPC(	signed int (*CPPC_Func)(unsigned int, void*),
				void *arg )
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

static void Query_Same_Platform_Features(unsigned int cpu)
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

	Intel_ThermalOffset(PfInfo.ProgrammableTj);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio = 0;
}

static void Nehalem_Platform_Info(unsigned int cpu)
{
	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
}

static void IvyBridge_EP_Platform_Info(unsigned int cpu)
{
	const unsigned int NC = \
	PUBLIC(RO(Proc))->CPU.Count >> PUBLIC(RO(Proc))->Features.HTT_Enable;

	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
    if (NC > 8) {
	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 7; /*	15C	*/
    }
}

static void Haswell_EP_Platform_Info(unsigned int cpu)
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

static void Skylake_X_Platform_Info(unsigned int cpu)
{
	const unsigned int NC = \
	PUBLIC(RO(Proc))->CPU.Count >> PUBLIC(RO(Proc))->Features.HTT_Enable;

    switch (PUBLIC(RO(Proc))->Features.Std.EAX.Stepping) {
    case 11:
    case 10:
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_COOPERLAKE_X],
		CODENAME_LEN);
	break;
    case 7:
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_CASCADELAKE_X],
		CODENAME_LEN);
	break;
    case 4:
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_SKYLAKE_X],
		CODENAME_LEN);
	break;
    }

	Query_Same_Platform_Features(cpu);

	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	8C	*/
    if (NC > 8) {
	PUBLIC(RO(Proc))->Features.SpecTurboRatio += 8; /*	16C	*/
    }
}

static void Probe_AMD_DataFabric(void)
{
#ifdef CONFIG_AMD_NB
    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	if (PRIVATE(OF(Zen)).Device.DF == NULL)
	{
		struct pci_dev *dev;
		dev = pci_get_domain_bus_and_slot(0x0,0x0,PCI_DEVFN(0x18, 0x0));
		if (dev != NULL)
		{
			PRIVATE(OF(Zen)).Device.DF = dev;
		}
	}
    }
#endif /* CONFIG_AMD_NB */
}

typedef void (*ROUTER)(void __iomem *mchmap, unsigned short mc);

static PCI_CALLBACK Router(	struct pci_dev *dev, unsigned int offset,
				unsigned int bsize, unsigned long long wsize,
				ROUTER route, unsigned short mc )
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
	mchbarEnable = BITVAL(mchbar.addr, 0);
	if (mchbarEnable) {
		mchbar.addr &= wmask;
		mchbar.addr += wsize * mc;
		mchmap = ioremap(mchbar.addr, wsize);
		if (mchmap != NULL) {
			route(mchmap, mc);

			iounmap(mchmap);

			return 0;
		} else
			return (PCI_CALLBACK) -ENOMEM;
	} else
		return (PCI_CALLBACK) -ENOMEM;
}

inline PCI_CALLBACK GetMemoryBAR(int M, int B, int D, int F,unsigned int offset,
				unsigned int bsize, unsigned long long wsize,
				unsigned short range,
				struct pci_dev **device, void __iomem **memmap)
{
  if ((*device) == NULL) {
    if (((*device) = pci_get_domain_bus_and_slot(M, B, PCI_DEVFN(D,F))) != NULL)
    {
	union {
		unsigned long long addr;
		struct {
			unsigned int low;
			unsigned int high;
		};
	} membar;
	unsigned long long wmask = BITCPL(wsize);
	unsigned char membarEnable = 0;

	switch (bsize) {
	case 32:
		pci_read_config_dword((*device), offset    , &membar.low);
		membar.high = 0;
		break;
	case 64:
		pci_read_config_dword((*device), offset    , &membar.low);
		pci_read_config_dword((*device), offset + 4, &membar.high);
		break;
	}
	membarEnable = BITVAL(membar.addr, 0);
	if (membarEnable) {
		membar.addr &= wmask;
		membar.addr += wsize * range;
		if (((*memmap) = ioremap(membar.addr, wsize)) != NULL) {
			return 0;
		} else
			return (PCI_CALLBACK) -ENOMEM;
	} else
		return (PCI_CALLBACK) -ENOMEM;
    }
	return (PCI_CALLBACK) -EINVAL;
  }
	return (PCI_CALLBACK) -EEXIST;
}

inline void PutMemoryBAR(struct pci_dev **device, void __iomem **memmap)
{
	if ((*memmap) != NULL) {
		iounmap((*memmap));
		(*memmap) = NULL;
	}
	if ((*device) != NULL)
	{
		pci_dev_put((*device));
		(*device) = NULL;
	}
}

static void Query_P945(void __iomem *mchmap, unsigned short mc)
{	/* Source: Mobile Intel 945 Express Chipset Family.		*/
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].P945.DCC.value = readl(mchmap + 0x200);

	switch (PUBLIC(RO(Proc))->Uncore.MC[mc].P945.DCC.DAMC) {
	case 0b00:
	case 0b11:
		PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 1;
		break;
	case 0b01:
	case 0b10:
		PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 2;
		break;
	}

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 1;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	unsigned short rank, rankCount;

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.DRT0.value = \
					readl(mchmap + 0x110 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.DRT1.value = \
					readw(mchmap + 0x114 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.DRT2.value = \
					readl(mchmap + 0x118 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.BANK.value = \
					readw(mchmap + 0x10e + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.WIDTH.value = \
					readw(mchmap + 0x40c + 0x80 * cha);
      if (cha == 0) {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.WIDTH.value &= \
								0b11111111;
	rankCount = 4;
      } else {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.WIDTH.value &= 0b1111;
	rankCount = 2;
      }
      for (rank = 0; rank < rankCount; rank++)
      {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P945.DRB[rank].value = \
				readb(mchmap + 0x100 + rank + 0x80 * cha);
      }
    }
}

static void Query_P955(void __iomem *mchmap, unsigned short mc)
{	/* Source: Intel 82955X Memory Controller Hub (MCH)		*/
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].P955.DCC.value = readl(mchmap + 0x200);

	switch (PUBLIC(RO(Proc))->Uncore.MC[mc].P955.DCC.DAMC) {
	case 0b00:
	case 0b11:
		PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 1;
		break;
	case 0b01:
	case 0b10:
		PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 2;
		break;
	}

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 1;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	unsigned short rank;

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P955.DRT1.value = \
					readw(mchmap + 0x114 + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P955.BANK.value = \
					readw(mchmap + 0x10e + 0x80 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P955.WIDTH.value = \
					readw(mchmap + 0x40c + 0x80 * cha);

      for (rank = 0; rank < 4; rank++)
      {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P955.DRB[rank].value = \
				readb(mchmap + 0x100 + rank + 0x80 * cha);
      }
    }
}

static void Query_P965(void __iomem *mchmap, unsigned short mc)
{
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].P965.CKE0.value = readl(mchmap + 0x260);
	PUBLIC(RO(Proc))->Uncore.MC[mc].P965.CKE1.value = readl(mchmap + 0x660);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[mc].P965.CKE0.RankPop0 != 0)
		+ (PUBLIC(RO(Proc))->Uncore.MC[mc].P965.CKE1.RankPop0 != 0);

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = \
		PUBLIC(RO(Proc))->Uncore.MC[mc].P965.CKE0.SingleDimmPop ? 1 : 2;

	if (PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount > 1) {
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount += \
		PUBLIC(RO(Proc))->Uncore.MC[mc].P965.CKE1.SingleDimmPop ? 1 : 2;
	}
	for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
	{
		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P965.DRT0.value = \
					readl(mchmap + 0x29c + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P965.DRT1.value = \
					readw(mchmap + 0x250 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P965.DRT2.value = \
					readl(mchmap + 0x252 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P965.DRT3.value = \
					readw(mchmap + 0x256 + 0x400 * cha);

		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P965.DRT4.value = \
					readl(mchmap + 0x258 + 0x400 * cha);
	}
}

static void Query_G965(void __iomem *mchmap, unsigned short mc)
{	/* Source: Mobile Intel 965 Express Chipset Family.		*/
	unsigned short cha, slot;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].G965.DRB0.value = readl(mchmap+0x1200);
	PUBLIC(RO(Proc))->Uncore.MC[mc].G965.DRB1.value = readl(mchmap+0x1300);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[mc].G965.DRB0.Rank1Addr != 0)
		+ (PUBLIC(RO(Proc))->Uncore.MC[mc].G965.DRB1.Rank1Addr != 0);

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = \
			PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount > 1 ? 1:2;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].G965.DRT0.value = \
					readl(mchmap + 0x1210 + 0x100 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].G965.DRT1.value = \
					readl(mchmap + 0x1214 + 0x100 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].G965.DRT2.value = \
					readl(mchmap + 0x1218 + 0x100 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].G965.DRT3.value = \
					readl(mchmap + 0x121c + 0x100 * cha);

      for (slot = 0; slot < PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount; slot++)
      {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[slot].DRA.value = \
					readl(mchmap + 0x1208 + 0x100 * cha);
      }
    }
}

static void Query_P35(void __iomem *mchmap, unsigned short mc)
{	/* Source: Intel 3 Series Express Chipset Family.		*/
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].P35.CKE0.value = readl(mchmap + 0x260);
	PUBLIC(RO(Proc))->Uncore.MC[mc].P35.CKE1.value = readl(mchmap + 0x660);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
		  (PUBLIC(RO(Proc))->Uncore.MC[mc].P35.CKE0.RankPop0 != 0)
		+ (PUBLIC(RO(Proc))->Uncore.MC[mc].P35.CKE1.RankPop0 != 0);

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = \
		PUBLIC(RO(Proc))->Uncore.MC[mc].P35.CKE0.SingleDimmPop ? 1 : 2;

	if (PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount > 1) {
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount += \
		PUBLIC(RO(Proc))->Uncore.MC[mc].P35.CKE1.SingleDimmPop ? 1 : 2;
	}

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	unsigned short rank;

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRT0.value = \
					readw(mchmap + 0x265 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRT1.value = \
					readw(mchmap + 0x250 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRT2.value = \
					readl(mchmap + 0x252 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRT3.value = \
					readw(mchmap + 0x256 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRT4.value = \
					readl(mchmap + 0x258 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRT5.value = \
					readw(mchmap + 0x25d + 0x400 * cha);

	for (rank = 0; rank < 4; rank++)
	{
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRB[rank].value = \
			readw(mchmap + 0x200 + (2 * rank) + 0x400 * cha);
	}
	for (rank = 0; rank < 2; rank++)
	{
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].P35.DRA[rank].value = \
				readw(mchmap + 0x208 + (2 * rank) + 0x400 * cha);
	}
    }
}

static kernel_ulong_t Query_NHM_Timing( struct pci_dev *pdev,
					unsigned short mc,
					unsigned short cha )
{	/*Source:Micron Technical Note DDR3 Power-Up, Initialization, & Reset*/
	struct pci_dev *dev = pci_get_domain_bus_and_slot(
						pci_domain_nr(pdev->bus),
						pdev->bus->number,
						PCI_DEVFN(4, 0)
				);
    if (dev != NULL)
    {
	pci_read_config_dword(dev, 0x58,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.DIMM_Init.value);

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
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].NHM.Sched.value);

	pci_dev_put(dev);
	return 0;
    } else
	return -ENODEV;
}

static kernel_ulong_t Query_NHM_DIMM(	struct pci_dev *pdev,
					unsigned short mc,
					unsigned short cha )
{
	struct pci_dev *dev = pci_get_domain_bus_and_slot(
						pci_domain_nr(pdev->bus),
						pdev->bus->number,
						PCI_DEVFN(4, 1)
				);
    if (dev != NULL)
    {
	unsigned short slot;
	for (slot = 0; slot < PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount; slot++)
	{
	pci_read_config_dword(dev, 0x48 + 4 * slot,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[slot].DOD.value);
	}
	pci_dev_put(dev);
	return 0;
    } else {
	return -ENODEV;
    }
}

static void Query_NHM_MaxDIMMs(struct pci_dev *dev, unsigned short mc)
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

static kernel_ulong_t Query_NHM_IMC(struct pci_dev *dev, unsigned short mc)
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
	(cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount) && !rc; cha++)
    {
	rc = Query_NHM_Timing(dev, mc, cha) & Query_NHM_DIMM(dev, mc, cha);
    }
	return rc;
}

static kernel_ulong_t Query_Lynnfield_IMC(struct pci_dev *dev,unsigned short mc)
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
	+ (PUBLIC(RO(Proc))->Uncore.MC[mc].NHM.CONTROL.CHANNEL1_ACTIVE != 0);

    for (cha = 0;
	(cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount) && !rc; cha++)
    {
	rc = Query_NHM_Timing(dev, mc, cha) & Query_NHM_DIMM(dev, mc, cha);
    }
	return rc;
}

inline void BIOS_DDR(void __iomem *mchmap)
{
	PUBLIC(RO(Proc))->Uncore.Bus.BIOS_DDR.value = readl(mchmap + 0x5e00);
}

static void Query_SNB_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Sources:	2nd & 3rd Generation Intel Core Processor Family
			Intel Xeon Processor E3-1200 Family		*/
	unsigned short cha, dimmCount[2];

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MADCH.value = readl(mchmap + 0x5000);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD0.value = readl(mchmap + 0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD1.value = readl(mchmap + 0x5008);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 0;

	dimmCount[0]=(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD0.Dimm_A_Size > 0)
		    +(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD0.Dimm_B_Size > 0);
	dimmCount[1]=(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD1.Dimm_A_Size > 0)
		    +(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD1.Dimm_B_Size > 0);

    for (cha = 0; cha < 2; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount += (dimmCount[cha] > 0);
    }
    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.DBP.value = \
					readl(mchmap + 0x4000 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.RAP.value = \
					readl(mchmap + 0x4004 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.RWP.value = \
					readl(mchmap + 0x4008 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.OTP.value = \
					readl(mchmap + 0x400c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.PDWN.value = \
					readl(mchmap + 0x40b0 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.RFP.value = \
					readl(mchmap + 0x4294 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.RFTP.value = \
					readl(mchmap + 0x4298 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB.SRFTP.value = \
					readl(mchmap + 0x42a4 + 0x400 * cha);
    }
	/*		Is Dual DIMM Per Channel Disable ?		*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = \
			(PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.DDPCD == 1) ?
			1 : PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount;

	BIOS_DDR(mchmap);
}

static void Query_Turbo_TDP_Config(void __iomem *mchmap)
{
	TURBO_ACTIVATION TurboActivation = {.value = 0};
	CONFIG_TDP_NOMINAL NominalTDP = {.value = 0};
	CONFIG_TDP_CONTROL ControlTDP = {.value = 0};
	CONFIG_TDP_LEVEL ConfigTDP[2] = {{.value = 0}, {.value = 0}};
	unsigned int cpu, local = get_cpu();	/* TODO(preempt_disable) */

	NominalTDP.value = readl(mchmap + 0x5f3c);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP)] = NominalTDP.Ratio;

	ConfigTDP[0].value = readq(mchmap + 0x5f40);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP1)] = ConfigTDP[0].Ratio;

	ConfigTDP[1].value = readq(mchmap + 0x5f48);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(TDP2)] = ConfigTDP[1].Ratio;

	ControlTDP.value = readl(mchmap + 0x5f50);
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Lock  = ControlTDP.Lock;
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Level = ControlTDP.Level;

	TurboActivation.value = readl(mchmap + 0x5f54);
	PUBLIC(RO(Core, AT(local)))->Boost[BOOST(ACT)]=TurboActivation.MaxRatio;
	PUBLIC(RO(Proc))->Features.TurboActiv_Lock = TurboActivation.Ratio_Lock;

	put_cpu();	/* TODO(preempt_enable) */

    switch (PUBLIC(RO(Proc))->Features.TDP_Cfg_Level) {
    case 2:
	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.ThermalSpecPower = \
							ConfigTDP[1].PkgPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MinimumPower = \
							ConfigTDP[1].MinPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MaximumPower = \
							ConfigTDP[1].MaxPower;
	break;
    case 1:
	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.ThermalSpecPower = \
							ConfigTDP[0].PkgPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MinimumPower = \
							ConfigTDP[0].MinPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MaximumPower = \
							ConfigTDP[0].MaxPower;
	break;
    case 0:
	/*TODO(Unknown CSR for Nominal {Pkg, Min, Max} Power settings ?) */
	break;
    }

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

static void Query_HSW_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Source: Desktop 4th & 5th Generation Intel Core Processor Family */
	unsigned short cha, dimmCount[2];

	PUBLIC(RO(Proc))->Uncore.Bus.ClkCfg.value = readl(mchmap + 0xc00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MADCH.value= readl(mchmap + 0x5000);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD0.value = readl(mchmap + 0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD1.value = readl(mchmap + 0x5008);

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 0;

	dimmCount[0]=(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD0.Dimm_A_Size > 0)
		    +(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD0.Dimm_B_Size > 0);
	dimmCount[1]=(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD1.Dimm_A_Size > 0)
		    +(PUBLIC(RO(Proc))->Uncore.MC[mc].SNB.MAD1.Dimm_B_Size > 0);

    for (cha = 0; cha < 2; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount += (dimmCount[cha] > 0);
    }
    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
/*TODO( Unsolved: What is the channel #1 'X' factor of Haswell registers ? )*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.REG4C00.value = \
							readl(mchmap + 0x4c00);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.Timing.value = \
							readl(mchmap + 0x4c04);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.Rank_A.value = \
							readl(mchmap + 0x4c08);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.Rank_B.value = \
							readl(mchmap + 0x4c0c);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.Rank.value = \
							readl(mchmap + 0x4c14);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.Refresh.value = \
							readl(mchmap + 0x4e98);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].HSW.PDWN.value = \
							readl(mchmap + 0x4cb0);
    }
	/*		Is Dual DIMM Per Channel Disable ?		*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = \
			(PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.DDPCD == 1) ?
			1 : PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount;
    if (mc == 0) {
	Query_Turbo_TDP_Config(mchmap);
    }
}

static void Query_HSW_CLK(void __iomem *mchmap, unsigned short mc)
{
	BIOS_DDR(mchmap);

	Query_HSW_IMC(mchmap, mc);
}

inline void SKL_SA(void __iomem *mchmap)
{
	PUBLIC(RO(Proc))->Uncore.Bus.SKL_SA_Pll.value = readl(mchmap + 0x5918);
}

static void Query_SKL_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Source: 6th & 7th Generation Intel Processor for S-Platforms Vol 2*/
	unsigned short cha;
	/*		Intra channel configuration			*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADCH.value = readl(mchmap+0x5000);
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADCH.CH_L_MAP)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADC0.value = readl(mchmap+0x5008);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADC1.value = readl(mchmap+0x5004);
    } else {
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADC0.value = readl(mchmap+0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADC1.value = readl(mchmap+0x5008);
    }
	/*		DIMM parameters					*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADD0.value = readl(mchmap+0x500c);
	PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADD1.value = readl(mchmap+0x5010);
	/*		Sum up any present DIMM per channel.		*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  ((PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADD0.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADD0.Dimm_S_Size != 0))
	+ ((PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADD1.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].SKL.MADD1.Dimm_S_Size != 0));

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 2;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.Timing.value = \
					readl(mchmap + 0x4000 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.ACT.value = \
					readl(mchmap + 0x4004 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.RDRD.value = \
					readl(mchmap + 0x400c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.RDWR.value = \
					readl(mchmap + 0x4010 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.WRRD.value = \
					readl(mchmap + 0x4014 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.WRWR.value = \
					readl(mchmap + 0x4018 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.Sched.value = \
					readl(mchmap + 0x401c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.ODT.value = \
					readl(mchmap + 0x4070 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SKL.Refresh.value = \
					readl(mchmap + 0x423c + 0x400 * cha);
    }
    if (mc == 0) {
	Query_Turbo_TDP_Config(mchmap);
	SKL_SA(mchmap);
    }
}

inline void RKL_SA(void __iomem *mchmap)
{
	PUBLIC(RO(Proc))->Uncore.Bus.ADL_SA_Pll.value = readq(mchmap+0x5918);

	PUBLIC(RO(Proc))->PowerThermal.VID.SOC = \
			PUBLIC(RO(Proc))->Uncore.Bus.ADL_SA_Pll.SA_VOLTAGE;
}

static void Query_RKL_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Source: 11th Generation Intel Core Processor Datasheet Vol 2 */
	unsigned short cha;
	/*		Intra channel configuration			*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADCH.value = readl(mchmap+0x5000);
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADCH.CH_L_MAP)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADC0.value = readl(mchmap+0x5008);
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADC1.value = readl(mchmap+0x5004);
    } else {
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADC0.value = readl(mchmap+0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADC1.value = readl(mchmap+0x5008);
    }
	/*		DIMM parameters					*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADD0.value = readl(mchmap+0x500c);
	PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADD1.value = readl(mchmap+0x5010);
	/*		Sum up any present DIMM per channel.		*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  ((PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADD0.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADD0.Dimm_S_Size != 0))
	+ ((PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADD1.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].RKL.MADD1.Dimm_S_Size != 0));

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 2;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.Timing.value = \
					readl(mchmap + 0x4000 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.ACT.value = \
					readl(mchmap + 0x4004 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.RDRD.value = \
					readl(mchmap + 0x400c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.RDWR.value = \
					readl(mchmap + 0x4010 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.WRRD.value = \
					readl(mchmap + 0x4014 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.WRWR.value = \
					readl(mchmap + 0x4018 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.Sched.value = \
					readq(mchmap + 0x4088 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.PWDEN.value = \
					readq(mchmap + 0x4050 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.ODT.value = \
					readq(mchmap + 0x4070 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.Refresh.value = \
					readl(mchmap + 0x423c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].RKL.SRExit.value = \
					readl(mchmap + 0x42c4 + 0x400 * cha);
    }
    if (mc == 0) {
	Query_Turbo_TDP_Config(mchmap);
	BIOS_DDR(mchmap);
	RKL_SA(mchmap);
    }
}

#define TGL_SA	RKL_SA

static void Query_TGL_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Source: 11th Generation Intel Core Processor Datasheet Vol 2 */
	unsigned short cha;

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 0;
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 0;

	/*		Intra channel configuration			*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADCH.value = readl(mchmap+0x5000);

    if (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADCH.value == 0xffffffff)
    {
		goto EXIT_TGL_IMC;
    }
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADCH.CH_L_MAP)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADC0.value = readl(mchmap+0x5008);
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADC1.value = readl(mchmap+0x5004);
    } else {
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADC0.value = readl(mchmap+0x5004);
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADC1.value = readl(mchmap+0x5008);
    }
    if ( (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADC0.value == 0xffffffff)
      || (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADC1.value == 0xffffffff) )
    {
		goto EXIT_TGL_IMC;
    }
	/*		DIMM parameters					*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD0.value = readl(mchmap+0x500c);
	PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD1.value = readl(mchmap+0x5010);

    if ( (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD0.value == 0xffffffff)
      || (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD1.value == 0xffffffff) )
    {
		goto EXIT_TGL_IMC;
    }
	/*		Sum up any present DIMM per channel.		*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  ((PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD0.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD0.Dimm_S_Size != 0))
	+ ((PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD1.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].TGL.MADD1.Dimm_S_Size != 0));

	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 2;

    for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.Timing.value = \
					readq(mchmap + 0x4000 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.ACT.value = \
					readl(mchmap + 0x4008 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.RDRD.value = \
					readl(mchmap + 0x400c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.RDWR.value = \
					readl(mchmap + 0x4010 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.WRRD.value = \
					readl(mchmap + 0x4014 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.WRWR.value = \
					readl(mchmap + 0x4018 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.Sched.value = \
					readq(mchmap + 0x4088 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.PWDEN.value = \
					readq(mchmap + 0x4050 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.ODT.value = \
					readq(mchmap + 0x4070 + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.Refresh.value = \
					readl(mchmap + 0x423c + 0x400 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].TGL.SRExit.value = \
					readq(mchmap + 0x42c0 + 0x400 * cha);
    }
    if (mc == 0) {
	Query_Turbo_TDP_Config(mchmap);
	BIOS_DDR(mchmap);
	TGL_SA(mchmap);
    }
EXIT_TGL_IMC:
	EMPTY_STMT();
}

#define ADL_SA	TGL_SA

static void Query_ADL_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Source: 12th Generation Intel Core Processor Datasheet Vol 2 */
	unsigned short cha, virtualCount;

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 0;
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 0;

	/*		Intra channel configuration			*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADCH.value = readl(mchmap+0xd800);

    if (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADCH.value == 0xffffffff)
    {
		goto EXIT_ADL_IMC;
    }
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADCH.CH_L_MAP)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADC0.value = readl(mchmap+0xd808);
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADC1.value = readl(mchmap+0xd804);
    } else {
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADC0.value = readl(mchmap+0xd804);
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADC1.value = readl(mchmap+0xd808);
    }
    if ( (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADC0.value == 0xffffffff)
      || (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADC1.value == 0xffffffff) )
    {
		goto EXIT_ADL_IMC;
    }
	/*		DIMM parameters					*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD0.value = readl(mchmap+0xd80c);
	PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD1.value = readl(mchmap+0xd810);

    if ( (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD0.value == 0xffffffff)
      || (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD1.value == 0xffffffff) )
    {
		goto EXIT_ADL_IMC;
    }
	/*	Check for 2 DIMMs Per Channel is enabled		*/
    if (PUBLIC(RO(Proc))->Uncore.Bus.ADL_Cap_A.DDPCD == 0)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 2;

	switch (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADCH.DDR_TYPE) {
	case 0b00:	/*	DDR4	*/
		virtualCount = 1;
		break;
	case 0b11:	/*	LPDDR4	*/
	case 0b01:	/*	DDR5	*/
	case 0b10:	/*	LPDDR5	*/
	default:
		virtualCount = 2;
		break;
	}
    } else {
	/*	Guessing activated channel from the populated DIMM.	*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  ((PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD0.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD0.Dimm_S_Size != 0))
	+ ((PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD1.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADD1.Dimm_S_Size != 0));

	virtualCount = PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount;
    }
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 1;

    for (cha = 0 ; cha < virtualCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.Timing.value = \
					readq(mchmap + 0xe000 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.ACT.value = \
					readl(mchmap + 0xe008 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.RDRD.value = \
					readl(mchmap + 0xe00c + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.RDWR.value = \
					readl(mchmap + 0xe010 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.WRRD.value = \
					readl(mchmap + 0xe014 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.WRWR.value = \
					readl(mchmap + 0xe018 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.Sched.value = \
					readq(mchmap + 0xe088 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.PWDEN.value = \
					readq(mchmap + 0xe050 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.ODT.value = \
					readq(mchmap + 0xe070 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.Refresh.value = \
					readl(mchmap + 0xe43c + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.SRExit.value = \
					readq(mchmap + 0xe4c0 + 0x800 * cha);
    }
    if (mc == 0) {
	Query_Turbo_TDP_Config(mchmap);
	BIOS_DDR(mchmap);
	ADL_SA(mchmap);
    }
EXIT_ADL_IMC:
	EMPTY_STMT();
}

#define MTL_SA	ADL_SA

static void Query_MTL_IMC(void __iomem *mchmap, unsigned short mc)
{	/* Source: 13th and 14th Gen. Intel Core Processors Datasheet Vol 2 */
	unsigned short cha, virtualCount;

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 0;
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 0;

	/*		Intra channel configuration			*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADCH.value = readl(mchmap+0xd800);

    if (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADCH.value == 0xffffffff)
    {
		goto EXIT_MTL_IMC;
    }
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADCH.CH_L_MAP)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADC0.value = readl(mchmap+0xd808);
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADC1.value = readl(mchmap+0xd804);
    } else {
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADC0.value = readl(mchmap+0xd804);
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADC1.value = readl(mchmap+0xd808);
    }
    if ( (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADC0.value == 0xffffffff)
      || (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADC1.value == 0xffffffff) )
    {
		goto EXIT_MTL_IMC;
    }
	/*		DIMM parameters					*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD0.value = readl(mchmap+0xd80c);
	PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD1.value = readl(mchmap+0xd810);

    if ( (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD0.value == 0xffffffff)
      || (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD1.value == 0xffffffff) )
    {
		goto EXIT_MTL_IMC;
    }
	/*	Check for 2 DIMMs Per Channel is enabled		*/
    if (PUBLIC(RO(Proc))->Uncore.Bus.MTL_Cap_A.DDPCD == 0)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 2;

	switch (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADCH.DDR_TYPE) {
	case 0b00:	/*	DDR4	*/
		virtualCount = 1;
		break;
	case 0b11:	/*	LPDDR4	*/
	case 0b01:	/*	DDR5	*/
	case 0b10:	/*	LPDDR5	*/
	default:
		virtualCount = 2;
		break;
	}
    } else {
	/*	Guessing activated channel from the populated DIMM.	*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	  ((PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD0.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD0.Dimm_S_Size != 0))
	+ ((PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD1.Dimm_L_Size != 0)
	|| (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADD1.Dimm_S_Size != 0));

	virtualCount = PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount;
    }
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 1;

    for (cha = 0 ; cha < virtualCount; cha++)
    {
	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.Timing.value = \
					readq(mchmap + 0xe000 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.ACT.value = \
					readq(mchmap + 0xe138 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.RDRD.value = \
					readl(mchmap + 0xe00c + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.RDWR.value = \
					readl(mchmap + 0xe010 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.WRRD.value = \
					readl(mchmap + 0xe014 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.WRWR.value = \
					readl(mchmap + 0xe018 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.Sched.value = \
					readq(mchmap + 0xe088 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.PWDEN.value = \
					readq(mchmap + 0xe050 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.ODT.value = \
					readq(mchmap + 0xe070 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.Refresh.value = \
					readq(mchmap + 0xe4a0 + 0x800 * cha);

	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.SRExit.value = \
					readq(mchmap + 0xe4c0 + 0x800 * cha);
    }
    if (mc == 0) {
	Query_Turbo_TDP_Config(mchmap);
	BIOS_DDR(mchmap);
	MTL_SA(mchmap);
    }
EXIT_MTL_IMC:
	EMPTY_STMT();
}

static void Query_GLK_IMC(void __iomem *mchmap, unsigned short mc)
{ /* Source: Intel Pentium Silver and Intel Celeron Processors Vol 2	*/
	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = \
	PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = 0;
}

static PCI_CALLBACK P945(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x44, 32, 0x4000, Query_P945, 0);
}

static PCI_CALLBACK P955(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x44, 32, 0x4000, Query_P955, 0);
}

static PCI_CALLBACK P965(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x48, 64, 0x4000, Query_P965, 0);
}

static PCI_CALLBACK G965(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x48, 64, 0x4000, Query_G965, 0);
}

static PCI_CALLBACK P35(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x48, 64, 0x4000, Query_P35, 0);
}

#define WDT_Technology(TCO1_CNT, Halt_Get_Method, Halt_Set_Method)	\
({									\
	Halt_Get_Method							\
									\
	switch (WDT_Enable) {						\
	case COREFREQ_TOGGLE_OFF:					\
	case COREFREQ_TOGGLE_ON:					\
		TCO1_CNT.TCO_TMR_HALT = !WDT_Enable;			\
		Halt_Set_Method						\
		Halt_Get_Method						\
		break;							\
	}								\
	if (TCO1_CNT.TCO_TMR_HALT) {					\
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->WDT,		\
				    PUBLIC(RO(Proc))->Service.Core);	\
	} else {							\
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->WDT,		\
				    PUBLIC(RO(Proc))->Service.Core);	\
	}								\
})

static PCI_CALLBACK ICH_TCO(struct pci_dev *dev)
{
	kernel_ulong_t rc;
    if ((rc = pci_request_regions(dev, DRV_DEVNAME)) == 0)
    {
	Intel_TCO1_CNT TCO1_CNT = {.value = 0};

	WDT_Technology( TCO1_CNT,
		{ pci_read_config_word(dev, 0x40 + 8, &TCO1_CNT.value); },
		{ pci_write_config_word(dev, 0x40 + 8, TCO1_CNT.value); } );

	pci_release_regions(dev);
    }
	return (PCI_CALLBACK) rc;
}

static PCI_CALLBACK TCOBASE(struct pci_dev *dev)
{
	kernel_ulong_t rc = -ENODEV;
	struct device *parent = &dev->dev;

	Intel_TCOCTL CTL = {.value = 0};
	Intel_TCOBASE TCO = {.value = 0};

	pci_read_config_dword(dev, 0x50, &TCO.value);
	pci_read_config_dword(dev, 0x54, &CTL.value);

    if (TCO.IOS && CTL.BASE_EN) {
	if (!devm_request_region(parent, TCO.TCOBA, 32, DRV_DEVNAME))
	{
		Intel_TCO1_CNT TCO1_CNT = {.value = 0};

		WDT_Technology( TCO1_CNT,
				{ TCO1_CNT.value = inw(TCO.TCOBA + 8); },
				{ outw(TCO1_CNT.value, TCO.TCOBA + 8); } );

		devm_request_region(parent, TCO.TCOBA, 32, DRV_DEVNAME);

		rc = RC_SUCCESS;
	}
    }
	return (PCI_CALLBACK) rc;
}

#undef WDT_Technology

static PCI_CALLBACK SoC_MCR(struct pci_dev *dev)
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
/* DRMC */
	MsgCtrlReg.Offset = 0xb;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRMC.value);
/* BIOS_CFG */
	MsgCtrlReg.Port = 0x4;
	MsgCtrlReg.Offset = 0x6;

	pci_write_config_dword(dev, 0xd0, MsgCtrlReg.value);

	pci_read_config_dword(dev, 0xd4,
		&PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.value);

	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK SoC_SLM(struct pci_dev *dev)
{
	PCI_CALLBACK rc = SoC_MCR(dev);
    if ((PCI_CALLBACK) 0 == rc)
    {
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 1
		+ ( PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.EFF_DUAL_CH_EN
		| !PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.DUAL_CH_DIS );

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = (
			PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN0
		|	PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN1
	) + (
			PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN2
		|	PUBLIC(RO(Proc))->Uncore.MC[0].SLM.DRP.RKEN3
	);
    }
	return rc;
}

static PCI_CALLBACK SoC_AMT(struct pci_dev *dev)
{
	PCI_CALLBACK rc = SoC_MCR(dev);
    if ((PCI_CALLBACK) 0 == rc)
    {
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 1
		+ ( PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.EFF_DUAL_CH_EN
		| !PUBLIC(RO(Proc))->Uncore.MC[0].SLM.BIOS_CFG.DUAL_CH_DIS );

	PUBLIC(RO(Proc))->Uncore.MC[0].SlotCount = 1;
    }
	return rc;
}

static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev)
{	/*		Clarksfield; Lynnfield				*/
	kernel_ulong_t rc = 0;
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < PUBLIC(RO(Proc))->Uncore.CtrlCount) && !rc; mc++) {
		rc = Query_Lynnfield_IMC(dev, 0);
	}
	return (PCI_CALLBACK) rc;
}

static PCI_CALLBACK Jasper_Forest_IMC(struct pci_dev *pdev)
{
	kernel_ulong_t rc = 0;
	unsigned int cpu = 0, SocketID = 0, bus_number = pdev->bus->number;
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 0;

    while ((cpu < PUBLIC(RO(Proc))->CPU.Count) && !rc)
    {
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	if (Core->T.PackageID == SocketID)
	{
		struct pci_dev *dev = pci_get_domain_bus_and_slot(
						pci_domain_nr(pdev->bus),
						bus_number,
						PCI_DEVFN(3, 0) );
	    if (dev != NULL)
	    {
		const unsigned short mc = ((unsigned short) SocketID)
					% MC_MAX_CTRL;
		rc = Query_NHM_IMC(dev, mc);
		pci_dev_put(dev);
	    }
		PUBLIC(RO(Proc))->Uncore.CtrlCount++;

		if (bus_number < 0xff) {
			bus_number++;
		}
		SocketID++;
	}
	cpu++;
    }
	return (PCI_CALLBACK) rc;
}

static PCI_CALLBACK Nehalem_IMC(struct pci_dev *dev)
{ /* Arrandale; Beckton; Bloomfield; Clarkdale; Eagleton; Gainestown; Gulftown*/
	kernel_ulong_t rc = 0;
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	for (mc = 0; (mc < PUBLIC(RO(Proc))->Uncore.CtrlCount) && !rc; mc++) {
		rc = Query_NHM_IMC(dev, mc);
	}
	return (PCI_CALLBACK) rc;
}

static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0x50,
				&PUBLIC(RO(Proc))->Uncore.Bus.DimmClock.value);

	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK NHM_NON_CORE(struct pci_dev *dev)
{
	NHM_CURRENT_UCLK_RATIO UncoreClock = {.value = 0};

	pci_read_config_dword(dev, 0xc0, &UncoreClock.value);

	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)]=UncoreClock.UCLK;
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)]=UncoreClock.MinRatio;

	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK X58_QPI(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xd0,
				&PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.value);

	Intel_FlexRatio();

	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK X58_VTD(struct pci_dev *dev)
{
	kernel_ulong_t rc = 0;
	unsigned int VTBAR = 0;

	pci_read_config_dword(dev, 0x180, &VTBAR);
  if (BITVAL(VTBAR, 0) == 1)
  {
	void __iomem *VT_d_MMIO;
	const unsigned int VT_d_Bar = VTBAR & 0xffffe000;

	VT_d_MMIO = ioremap(VT_d_Bar, 0x1000);
    if (VT_d_MMIO != NULL)
    {
	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_Ver.value = readl(VT_d_MMIO + 0x0);
	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_Cap.value = readq(VT_d_MMIO + 0x8);

	iounmap(VT_d_MMIO);
    }
	PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.X58.VT_d = 0;
  } else {
	PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.X58.VT_d = 1;
  }
	return (PCI_CALLBACK) rc;
}

static PCI_CALLBACK SNB_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.value);

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x48, 64, 0x8000, Query_SNB_IMC, 0);
}

static PCI_CALLBACK IVB_IMC(struct pci_dev *dev)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.IVB_Cap.value);

	Intel_FlexRatio();

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x48, 64, 0x8000, Query_SNB_IMC, 0);
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

	return (PCI_CALLBACK) 0;
}

static kernel_ulong_t SNB_EP_CTRL(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(dev, 0x7c,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].SNB_EP.TECH.value);

	pci_read_config_dword(dev, 0x80,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].SNB_EP.TAD.value);
	return 0;
}

static kernel_ulong_t SNB_EP_IMC(struct pci_dev *dev , unsigned short mc,
						unsigned short cha)
{
	pci_read_config_dword(dev, 0x200,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.DBP.value);

	pci_read_config_dword(dev, 0x204,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RAP.value);

	pci_read_config_dword(dev, 0x208,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RWP.value);

	pci_read_config_dword(dev, 0x20c,
		&PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.OTP.value);

	pci_read_config_dword(dev, 0x210,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RFP.value);

	pci_read_config_dword(dev, 0x214,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.RFTP.value);

	pci_read_config_dword(dev, 0x218,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].SNB_EP.SRFTP.value);

	return 0;
}

static kernel_ulong_t SNB_EP_TAD(struct pci_dev *dev,	unsigned short mc,
						unsigned short cha)
{
	unsigned short slotCount;

	pci_read_config_dword(dev, 0x80,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[0].MTR.value);

	pci_read_config_dword(dev, 0x84,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[1].MTR.value);

	pci_read_config_dword(dev, 0x88,
	    &PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[2].MTR.value);

	slotCount = \
	  PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[0].MTR.DIMM_POP
	+ PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[1].MTR.DIMM_POP
	+ PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[2].MTR.DIMM_POP;

	if (slotCount > PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount) {
		PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount = slotCount;
	}
	return 0;
}

static PCI_CALLBACK SNB_EP_QPI(struct pci_dev *dev)
{
	QPI_FREQUENCY QuickPath = {.value = 0};
	pci_read_config_dword(dev, 0xd4, &QuickPath.value);

	if (QuickPath.EP.QPIFREQSEL > \
		PUBLIC(RO(Proc))->Uncore.Bus.QuickPath.EP.QPIFREQSEL)
	{
		PUBLIC(RO(Proc))->Uncore.Bus.QuickPath = QuickPath;
	}
	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK SNB_EP_HB(struct pci_dev *pdev)
{
	struct pci_dev *dev;
	kernel_ulong_t rc = 0;
	CPUBUSNO BUSNO[MC_MAX_CTRL] = {{.value = 0}};
	const unsigned int HostBridge = pdev->bus->number;
	const unsigned int domain = pci_domain_nr(pdev->bus);
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 0;

	dev = pci_get_domain_bus_and_slot(domain, HostBridge, PCI_DEVFN(5, 0));
  if (dev != NULL)
  {
	pci_read_config_dword(dev, 0x108, &BUSNO[0].value);
	pci_dev_put(dev);
  }
  if (BUSNO[0].CFG.Valid)
  {
	PUBLIC(RO(Proc))->Uncore.CtrlCount++;

	dev = pci_get_domain_bus_and_slot(domain, BUSNO[0].CFG.UNC + 1,
						PCI_DEVFN(5, 0));
    if (dev != NULL)
    {
	pci_read_config_dword(dev, 0x108, &BUSNO[1].value);
	pci_dev_put(dev);

      if (BUSNO[1].CFG.Valid) {
	PUBLIC(RO(Proc))->Uncore.CtrlCount++;
      }
    }
  }
  for (mc = 0; mc < PUBLIC(RO(Proc))->Uncore.CtrlCount; mc++)
  {
	const unsigned int TAD = mc < 2 ? 15 : 29;
	const unsigned int TCR = mc < 2 ? 16 : 30;
	const unsigned int fun[4] = {0, 1, 4, 5};
	const unsigned int QPI[3] = {8, 9, 24};
	unsigned short cha, link;

	dev = pci_get_domain_bus_and_slot(domain, BUSNO[mc].CFG.UNC,
						PCI_DEVFN(15, 0));
    if (dev != NULL)
    {
	rc = SNB_EP_CTRL(dev, mc);
	pci_dev_put(dev);
    }

	PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 0;
    for (cha = 0; cha < 4; cha++)
    {
	dev = pci_get_domain_bus_and_slot(domain, BUSNO[mc].CFG.UNC,
						PCI_DEVFN(TCR, fun[cha]));
	if (dev != NULL)
	{
		rc = SNB_EP_IMC(dev, mc, cha);
		pci_dev_put(dev);

		dev = pci_get_domain_bus_and_slot(domain, BUSNO[mc].CFG.UNC,
						PCI_DEVFN(TAD, 2 + cha));
		if (dev != NULL)
		{
			rc = SNB_EP_TAD(dev, mc, cha);
			pci_dev_put(dev);
		}

		PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount = 1 + cha;
	}
    }
    for (link = 0; link < 3; link++)
    {
	dev = pci_get_domain_bus_and_slot(domain, BUSNO[mc].CFG.UNC,
						PCI_DEVFN(QPI[link], 0));
	if (dev != NULL)
	{
		rc = (kernel_ulong_t) SNB_EP_QPI(dev);
		pci_dev_put(dev);
	}
    }

	dev = pci_get_domain_bus_and_slot(domain, BUSNO[mc].CFG.UNC,
						PCI_DEVFN(10, 3));
	if (dev != NULL)
	{
		rc = (kernel_ulong_t) SNB_EP_CAP(dev);
		pci_dev_put(dev);
	}
  }
	return (PCI_CALLBACK) rc;
}

static PCI_CALLBACK HSW_HOST(struct pci_dev *dev, ROUTER Query)
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SNB_Cap.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.IVB_Cap.value);

	Intel_FlexRatio();

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return Router(dev, 0x48, 64, 0x8000, Query, 0);
}

static PCI_CALLBACK HSW_IMC(struct pci_dev *dev)
{
	return HSW_HOST(dev, Query_HSW_IMC);
}

static PCI_CALLBACK HSW_CLK(struct pci_dev *dev)
{
	return HSW_HOST(dev, Query_HSW_CLK);
}

static kernel_ulong_t HSW_EP_CTRL(struct pci_dev *dev, unsigned short mc)
{
	pci_read_config_dword(dev, 0x7c,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].HSW_EP.TECH.value);

	pci_read_config_dword(dev, 0x80,
			&PUBLIC(RO(Proc))->Uncore.MC[mc].HSW_EP.TAD.value);
	return 0;
}

static PCI_CALLBACK HSW_EP_CTRL0(struct pci_dev *dev)
{
	if (PUBLIC(RO(Proc))->Uncore.CtrlCount < 1) {
		PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;
	}
	HSW_EP_CTRL(dev, 0);

	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK HSW_EP_CTRL1(struct pci_dev *dev)
{
	if (PUBLIC(RO(Proc))->Uncore.CtrlCount < 2) {
		PUBLIC(RO(Proc))->Uncore.CtrlCount = 2;
	}
	HSW_EP_CTRL(dev, 1);

	return (PCI_CALLBACK) 0;
}

#define HSW_EP_IMC	SNB_EP_IMC

static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA0(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[0].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 0) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount < 1) {
			PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 1;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 0, 0);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA1(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[0].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 1) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount < 2) {
			PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 2;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 0, 1);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA2(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[0].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 2) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount < 3) {
			PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 3;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 0, 2);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA3(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[0].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 3) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount < 4) {
			PUBLIC(RO(Proc))->Uncore.MC[0].ChannelCount = 4;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 0, 3);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA0(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[1].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 0) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount < 1) {
			PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount = 1;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 1, 0);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA1(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[1].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 1) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount < 2) {
			PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount = 2;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 1, 1);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA2(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[1].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 2) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount < 3) {
			PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount = 3;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 1, 2);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA3(struct pci_dev *dev)
{
	const unsigned short
	channelMap = PUBLIC(RO(Proc))->Uncore.MC[1].HSW_EP.TECH.CHN_DISABLE;

	if (BITVAL(channelMap, 3) == 0) {
		if (PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount < 4) {
			PUBLIC(RO(Proc))->Uncore.MC[1].ChannelCount = 4;
		}
		return (PCI_CALLBACK) HSW_EP_IMC(dev, 1, 3);
	} else {
		return (PCI_CALLBACK) -ENODEV;
	}
}

#define HSW_EP_TAD	SNB_EP_TAD

static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA0(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 0, 0);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA1(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 0, 1);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA2(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 0, 2);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA3(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 0, 3);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA0(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 1, 0);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA1(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 1, 1);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA2(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 1, 2);
}

static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA3(struct pci_dev *dev)
{
	return (PCI_CALLBACK) HSW_EP_TAD(dev, 1, 3);
}

static void SoC_SKL_VTD(void)
{
  if (PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_A.VT_d == 0)
  {
	void __iomem *VT_d_MMIO;
	const unsigned int VTBAR = 0xfed90000;

	VT_d_MMIO = ioremap(VTBAR, 0x1000);
    if (VT_d_MMIO != NULL)
    {
	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_Ver.value = readl(VT_d_MMIO + 0x0);
	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_Cap.value = readq(VT_d_MMIO + 0x8);

	iounmap(VT_d_MMIO);
    }
  }
}

static PCI_CALLBACK SKL_HOST(	struct pci_dev *dev,
				ROUTER Query,
				unsigned long long wsize,
				unsigned short mc )
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_A.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_B.value);

	pci_read_config_dword(dev, 0xec,
				&PUBLIC(RO(Proc))->Uncore.Bus.SKL_Cap_C.value);

	Intel_FlexRatio();

	SoC_SKL_VTD();

	return Router(dev, 0x48, 64, wsize, Query, mc);
}

static PCI_CALLBACK SKL_IMC(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return SKL_HOST(dev, Query_SKL_IMC, 0x8000, 0);
}

static PCI_CALLBACK CML_PCH(struct pci_dev *dev)
{
	UNUSED(dev);
	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK RKL_IMC(struct pci_dev *dev)
{	/* Same address offsets as used in Skylake			*/
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	return SKL_HOST(dev, Query_RKL_IMC, 0x8000, 0);
}

static PCI_CALLBACK TGL_IMC(struct pci_dev *dev)
{	/* Source: drivers/edac/igen6_edac.c				*/
	PCI_CALLBACK rc = 0;
	unsigned short mc;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 0;
	/*	Controller #0 is not necessary activated but enabled.	*/
	for (mc = 0; mc < MC_MAX_CTRL; mc++)
	{
		rc = SKL_HOST(dev, Query_TGL_IMC, 0x10000, mc);

		if ( ((PCI_CALLBACK) 0 == rc)
		  && (PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount > 0) ) {
			PUBLIC(RO(Proc))->Uncore.CtrlCount = mc + 1;
		}
	}
	pci_read_config_dword(dev, 0xf0,
				&PUBLIC(RO(Proc))->Uncore.Bus.TGL_Cap_E.value);
	return rc;
}

static PCI_CALLBACK ADL_HOST(	struct pci_dev *dev,
				ROUTER Query,
				unsigned long long wsize,
				unsigned short mc )
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.ADL_Cap_A.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.ADL_Cap_B.value);

	pci_read_config_dword(dev, 0xec,
				&PUBLIC(RO(Proc))->Uncore.Bus.ADL_Cap_C.value);

	pci_read_config_dword(dev, 0xf0,
				&PUBLIC(RO(Proc))->Uncore.Bus.ADL_Cap_E.value);

	Intel_FlexRatio();

	SoC_SKL_VTD();

	return Router(dev, 0x48, 64, wsize, Query, mc);
}

static PCI_CALLBACK ADL_IMC(struct pci_dev *dev)
{	/* Source: 12th Generation Intel Core Processors datasheet, vol 2 */
	PCI_CALLBACK rc = 0;
	unsigned short mc, cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 0;
	/* MCHBAR matches bits 41 to 17 ; two MC x 64KB memory space	*/
  for (mc = 0; mc < MC_MAX_CTRL; mc++)
  {
	rc = ADL_HOST(dev, Query_ADL_IMC, 0x10000, mc);

   if ( (PCI_CALLBACK) 0 == rc)
   {
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADCH.value != 0xffffffff) {
      switch (PUBLIC(RO(Proc))->Uncore.MC[mc].ADL.MADCH.DDR_TYPE) {
	case 0b01:	/*	DDR5	*/
	case 0b10:	/*	LPDDR5	*/
	for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
	{
	 PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].ADL.Sched.ReservedBits1=0;
	}
	break;
      }
    }
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount > 0)
    {
	PUBLIC(RO(Proc))->Uncore.CtrlCount = mc + 1;
    }
   }
  }
	return rc;
}

static PCI_CALLBACK MTL_HOST(	struct pci_dev *dev,
				ROUTER Query,
				unsigned long long wsize,
				unsigned short mc )
{
	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.MTL_Cap_A.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.MTL_Cap_B.value);

	pci_read_config_dword(dev, 0xec,
				&PUBLIC(RO(Proc))->Uncore.Bus.MTL_Cap_C.value);

	pci_read_config_dword(dev, 0xf0,
				&PUBLIC(RO(Proc))->Uncore.Bus.MTL_Cap_E.value);

	Intel_FlexRatio();

	SoC_SKL_VTD();

	return Router(dev, 0x48, 64, wsize, Query, mc);
}

static PCI_CALLBACK MTL_IMC(struct pci_dev *dev)
{	/* Source: 13th and 14th Gen. Intel Core Processors datasheet, vol 2 */
	PCI_CALLBACK rc = 0;
	unsigned short mc, cha;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 0;
	/* MCHBAR matches bits 41 to 17 ; two MC x 64KB memory space	*/
  for (mc = 0; mc < MC_MAX_CTRL; mc++)
  {
	rc = MTL_HOST(dev, Query_MTL_IMC, 0x10000, mc);

   if ( (PCI_CALLBACK) 0 == rc)
   {
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADCH.value != 0xffffffff) {
      switch (PUBLIC(RO(Proc))->Uncore.MC[mc].MTL.MADCH.DDR_TYPE) {
	case 0b01:	/*	DDR5	*/
	case 0b10:	/*	LPDDR5	*/
	for (cha = 0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount; cha++)
	{
	 PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].MTL.Sched.ReservedBits1=0;
	}
	break;
      }
    }
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount > 0)
    {
	PUBLIC(RO(Proc))->Uncore.CtrlCount = mc + 1;
    }
   }
  }
	return rc;
}

static PCI_CALLBACK GLK_IMC(struct pci_dev *dev)
{
	PUBLIC(RO(Proc))->Uncore.CtrlCount = 1;

	pci_read_config_dword(dev, 0xe4,
				&PUBLIC(RO(Proc))->Uncore.Bus.GLK_Cap_A.value);

	pci_read_config_dword(dev, 0xe8,
				&PUBLIC(RO(Proc))->Uncore.Bus.GLK_Cap_B.value);

	Intel_FlexRatio();

	SoC_SKL_VTD();

	return Router(dev, 0x48, 64, 0x8000, Query_GLK_IMC, 0);
}

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

	return (PCI_CALLBACK) 0;
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

	return (PCI_CALLBACK) 0;
}

static PCI_CALLBACK AMD_Zen_IOMMU(struct pci_dev *dev)
{
/* Sources:
*	AMD I/O Virtualization Technology (IOMMU) Specification Jan. 2020
*	coreboot/src/soc/amd/picasso/agesa_acpi.c
*/
	AMD_IOMMU_CAP_BAR IOMMU_Cap_Bar;
	unsigned char IOMMU_UnLock = 0;

	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_CR.value = 0x0;

	pci_read_config_dword(	dev, 0x40,
				&PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_HDR.value );

	pci_read_config_dword(dev, 0x44, &IOMMU_Cap_Bar.low);
	pci_read_config_dword(dev, 0x48, &IOMMU_Cap_Bar.high);

	IOMMU_UnLock = IOMMU_Cap_Bar.addr & 0x1;
	IOMMU_Cap_Bar.addr = IOMMU_Cap_Bar.addr & 0xffffc000;
    if ((IOMMU_Cap_Bar.addr != 0x0) && IOMMU_UnLock)
    {
	void __iomem *IOMMU_MMIO;
	const size_t bsize = PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_HDR.EFRSup ?
				0x80000 : 0x4000;

      if ((IOMMU_MMIO = ioremap(IOMMU_Cap_Bar.addr, bsize)) != NULL)
      {
	PUBLIC(RO(Proc))->Uncore.Bus.IOMMU_CR.value = readq(IOMMU_MMIO + 0x18);

	iounmap(IOMMU_MMIO);
      }
      else
      {
	return (PCI_CALLBACK) -ENOMEM;
      }
    }
	return (PCI_CALLBACK) 0;
}

static void AMD_Zen_UMC_Aggregate(unsigned short mc, unsigned short cha,
				bool *Got_Mem_Clock, bool *Got_Div_Clock)
{
  if ((*Got_Mem_Clock) == false)
  {
	enum UNCORE_BOOST Mem_Clock = 0;
    switch (	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h\
		.CONFIG.BurstLength )
    {
    case 0x0:	/* BL2 */
    case 0x1:	/* BL4 */
    case 0x2:	/* BL8 */
      if (PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.MISC.DDR4.MEMCLK)
      {
	Mem_Clock = PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h\
			.MISC.DDR4.MEMCLK;

	(*Got_Mem_Clock) = true;
      }
	break;
    case 0x3:	/* BL16 */
      if (PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.MISC.DDR5.MEMCLK)
      {
	Mem_Clock = PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h\
			.MISC.DDR5.MEMCLK;

	Mem_Clock = DIV_ROUND_CLOSEST(Mem_Clock, 100U);

	(*Got_Mem_Clock) = true;
      }
	break;
    }
    if ((*Got_Mem_Clock) == true) {
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MAX)] = \
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)] = Mem_Clock;
    }
  }
  if ((*Got_Div_Clock) == false)
  {
	enum UNCORE_BOOST Div_Clock = 1;
    if (PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DbgMisc.UMC_Ready)
    {
      switch(	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h\
		.CONFIG.BurstLength )
      {
      case 0x0:	/* BL2 */
      case 0x1:	/* BL4 */
      case 0x2:	/* BL8 */
	Div_Clock = PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h\
			.DbgMisc.UCLK_Divisor == 0 ? 2 : 1;

	Div_Clock = Div_Clock * 3;

	(*Got_Div_Clock) = true;
	break;
      case 0x3:	/* BL16 */
	Div_Clock = PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h\
			.DbgMisc.UCLK_Divisor == 0 ? 1 : 2;

	(*Got_Div_Clock) = true;
	break;
      }
    }
    if ((*Got_Div_Clock) == true) {
	PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)] = \
		DIV_ROUND_CLOSEST(
			PUBLIC(RO(Proc))->Uncore.Boost[UNCORE_BOOST(MIN)],
			Div_Clock
		);
    }
  }
}

static void AMD_Zen_UMC(struct pci_dev *dev,
			const unsigned int UMC_BAR,
			const unsigned int CS_MASK[2][2],
			const unsigned int UMC_DAC,
			const unsigned int UMC_DIMM_CFG,
			unsigned short mc, unsigned short cha,
			bool *Got_Mem_Clock, bool *Got_Div_Clock)
{
	unsigned int CHIP_BAR[2][2] = {
	[0] =	{
		[0] = UMC_BAR + CS_MASK[0][0],
		[1] = UMC_BAR + CS_MASK[0][1]
		},
	[1] =	{
		[0] = UMC_BAR + CS_MASK[1][0],
		[1] = UMC_BAR + CS_MASK[1][1]
		}
	};
	unsigned short chip;
    for (chip = 0; chip < 4; chip++)
    {
	const unsigned short slot = chip & 1;
	unsigned short sec;

	Core_AMD_SMN_Read(
	    PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.DAC,
	    UMC_BAR + UMC_DAC + (slot << 2), dev
	);
	Core_AMD_SMN_Read(
	    PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].DIMM[slot].AMD17h.CFG,
	    UMC_BAR + UMC_DIMM_CFG + (slot << 2), dev
	);
	for (sec = 0; sec < 2; sec++)
	{
		unsigned int addr[2];

		addr[1] = CHIP_BAR[sec][1] + ((chip >> 1) << 2);

		Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
				.AMD17h.CHIP[chip][sec].Mask,
				addr[1], dev );

		addr[0] = CHIP_BAR[sec][0] + ((chip - (chip > 2)) << 2);

		Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
				.AMD17h.CHIP[chip][sec].Chip,
				addr[0], dev );
	}
    }
   Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.CONFIG,
			UMC_BAR + 0x100, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.SPAZ,
			UMC_BAR + 0x12c, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.ENCR,
			UMC_BAR + 0x144, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.MISC,
			UMC_BAR + 0x200, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR1,
			UMC_BAR + 0x204, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR2,
			UMC_BAR + 0x208, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR3,
			UMC_BAR + 0x20c, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR4,
			UMC_BAR + 0x210, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR5,
			UMC_BAR + 0x214, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR6,
			UMC_BAR + 0x218, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR7,
			UMC_BAR + 0x21c, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR8,
			UMC_BAR + 0x220, dev);

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR9,
			UMC_BAR + 0x224, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR10,
			UMC_BAR + 0x228, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR12,
			UMC_BAR + 0x230, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR13,
			UMC_BAR + 0x234, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR14,
			UMC_BAR + 0x238, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR20,
			UMC_BAR + 0x250, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR21,
			UMC_BAR + 0x254, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR22,
			UMC_BAR + 0x258, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTRFC,
			UMC_BAR + 0x260, dev );

    switch (	PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha]\
		.AMD17h.CONFIG.BurstLength ) {
    case 0x3:	/* BL16 */
      {
	unsigned int offset;	/* DWORD */
	for (offset = 0x2c0; offset < 0x2d0; offset = offset + 0x4)
	{
	    Core_AMD_SMN_Read(
		PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.RFCSB,
		UMC_BAR + offset, dev );

	    if (PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.RFCSB.value)
	    {
		break;
	    }
	}
      }
	break;
    }

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DTR35,
			UMC_BAR + 0x28c, dev );

    Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.BGS,
			UMC_BAR + 0x58, dev );

  Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.BGS_ALT,
			UMC_BAR + 0xd0, dev );

   Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.ECC.Lo,
			UMC_BAR + 0xdf0, dev );

   Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.ECC.Hi,
			UMC_BAR + 0xdf4, dev );

  Core_AMD_SMN_Read(PUBLIC(RO(Proc))->Uncore.MC[mc].Channel[cha].AMD17h.DbgMisc,
			UMC_BAR + 0xd6c, dev );

	AMD_Zen_UMC_Aggregate(mc, cha, Got_Mem_Clock, Got_Div_Clock);
}

static PCI_CALLBACK AMD_17h_DataFabric( struct pci_dev *pdev,
					const unsigned int CS_MASK[2][2],
					const unsigned int UMC_DAC,
					const unsigned int UMC_DIMM_CFG,
					const unsigned short umc_max,
					const unsigned short cha_max,
					const unsigned int devfn[] )
{
	struct pci_dev *dev;
	unsigned short umc;
	bool Got_Mem_Clock = false, Got_Div_Clock = false;

	PUBLIC(RO(Proc))->Uncore.CtrlCount = 0;
  for (umc = 0; umc < umc_max; umc++)
  {
	const unsigned int domain = pci_domain_nr(pdev->bus);
	unsigned short cha = 0, cnt;

	dev = pci_get_domain_bus_and_slot(domain, 0x0, devfn[umc]);
   if (dev != NULL)
   {
		PUBLIC(RO(Proc))->Uncore.CtrlCount++;
		PUBLIC(RO(Proc))->Uncore.MC[umc].SlotCount = 2;

    for (cnt = 0; cnt < cha_max; cnt++)
    {
	AMD_17_UMC_SDP_CTRL SDP_CTRL = {.value = 0};

	Core_AMD_SMN_Read(SDP_CTRL, SMU_AMD_UMC_BASE_CHA_F17H(cnt)+0x104, dev);

     if ((SDP_CTRL.value != 0xffffffff) && (SDP_CTRL.SdpInit))
     {
	AMD_Zen_UMC(	dev,
			SMU_AMD_UMC_BASE_CHA_F17H(cnt),
			CS_MASK,
			UMC_DAC, UMC_DIMM_CFG,
			umc, cha,
			&Got_Mem_Clock, &Got_Div_Clock );
	cha++;
     }
    }
	PUBLIC(RO(Proc))->Uncore.MC[umc].ChannelCount = cha;
	pci_dev_put(dev);
   } else {
	pr_err( "CoreFreq: AMD_17h_DataFabric()"		\
		" Break UMC(%hu) probing @ PCI(0x%x:0x0:0x%x)\n",
		umc, domain, devfn[umc]);
	break;
   }
  }
	return (PCI_CALLBACK) 0;
}

static void AMD_UMC_Normalize_Channels(void)
{
	unsigned short umc;
  for (umc = 0; umc < PUBLIC(RO(Proc))->Uncore.CtrlCount; umc++)
  { /* If UMC is quad channels (in 2 x 32-bits) then unpopulate odd channels*/
    if (PUBLIC(RO(Proc))->Uncore.MC[umc].ChannelCount >= 4) {
		unsigned short cha;
      for (cha=0; cha < PUBLIC(RO(Proc))->Uncore.MC[umc].ChannelCount; cha++)
      {
	if (cha & 1) {
		unsigned short slot;
	  for(slot=0;slot < PUBLIC(RO(Proc))->Uncore.MC[umc].SlotCount;slot++)
	  {
		const unsigned short chipselect_pair = slot << 1;

		BITCLR(LOCKLESS, PUBLIC(RO(Proc))->Uncore.MC[umc].Channel[cha]\
				.AMD17h.CHIP[chipselect_pair][0].Chip.value, 0);
	  }
	}
      }
    }
  }
}

static PCI_CALLBACK AMD_DataFabric_Zeppelin(struct pci_dev *pdev)
{
    if (strncmp(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_WHITEHAVEN],
		CODENAME_LEN) == 0)
    {		/*		Two controllers 			*/
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					2, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0),
					PCI_DEVFN(0x19, 0x0)} );
    }
    else if (strncmp(PUBLIC(RO(Proc))->Architecture,
			Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_NAPLES],
			CODENAME_LEN) == 0)
    {		/*		Four controllers			*/
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					4, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0),
					PCI_DEVFN(0x19, 0x0),
					PCI_DEVFN(0x1a, 0x0),
					PCI_DEVFN(0x1b, 0x0)} );
    }
    else	/*	CN_SNOWY_OWL, CN_SUMMIT_RIDGE			*/
    {		/*		One controller				*/
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
    }
}

static PCI_CALLBACK AMD_DataFabric_Raven(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Matisse(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Starship(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Renoir(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Ariel(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Raven2(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Fireflight(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Arden(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_VanGogh(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Vermeer(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Cezanne(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x28}
					},
					0x30, 0x80,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Rembrandt(struct pci_dev *pdev)
{
	PCI_CALLBACK ret = AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x30}
					},
					0x40, 0x90,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );

	if ((PCI_CALLBACK) 0 == ret) {
		AMD_UMC_Normalize_Channels();
	}
	return ret;
}

static PCI_CALLBACK AMD_DataFabric_Raphael(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x30}
					},
					0x44, 0x90,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Genoa(struct pci_dev *pdev)
{
	return AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x30}
					},
					0x40, 0x90,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );
}

static PCI_CALLBACK AMD_DataFabric_Phoenix(struct pci_dev *pdev)
{
	PCI_CALLBACK ret = AMD_17h_DataFabric(	pdev,
					(const unsigned int[2][2]) {
						{ 0x0, 0x20},
						{0x10, 0x30}
					},
					0x44, 0x90,
					1, MC_MAX_CHA,
		(const unsigned int[]) {PCI_DEVFN(0x18, 0x0)} );

	if ((PCI_CALLBACK) 0 == ret) {
		AMD_UMC_Normalize_Channels();
	}
	return ret;
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

static void Query_Same_Genuine_Features(void)
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

static void Query_GenuineIntel(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	Intel_Core_Platform_Info(cpu);
	HyperThreading_Technology();
}

static void Query_Core2(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	Intel_Core_Platform_Info(cpu);
	HyperThreading_Technology();
}

static void Query_Atom_Bonnell(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	Intel_Core_Platform_Info(cpu);
	HyperThreading_Technology();
}

static void Query_Silvermont(unsigned int cpu)
{					/* Tables 2-6, 2-7, 2-8(BT), 2-9(BT) */
	/*	Query the Min and Max frequency ratios			*/
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
}

static void Query_Goldmont(unsigned int cpu) /* Tables 2-6, 2-12	*/
{
	PLATFORM_INFO PfInfo = {.value = 0};
	RDMSR(PfInfo, MSR_PLATFORM_INFO);
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = PfInfo.MinimumRatio;
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = PfInfo.MaxNonTurboRatio;
	PUBLIC(RO(Proc))->Features.SpecTurboRatio=PUBLIC(RO(Proc))->CPU.Count;
	if ((PRIVATE(OF(Specific)) = LookupProcessor()) != NULL)
	{
		OverrideCodeNameString(PRIVATE(OF(Specific)));
		OverrideUnlockCapability(PRIVATE(OF(Specific)));
	}
	Default_Unlock_Reset();

	HyperThreading_Technology();
	Intel_PowerInterface();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Airmont(unsigned int cpu) /* Tables 2-6, 2-7, 2-8, 2-11 */
{
	Query_Silvermont(cpu);
}

static void Query_Nehalem(unsigned int cpu)	/*	Table 2-15	*/
{
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Nehalem_EX(unsigned int cpu) /* Tables 2-15, 2-17	*/
{
	Query_Same_Genuine_Features();
	Intel_Core_Platform_Info(cpu);
	HyperThreading_Technology();
}

static void Query_Avoton(unsigned int cpu)	/*	Table 2-10	*/
{
	Query_Silvermont(cpu);

	RDMSR(PUBLIC(RO(Proc))->PowerThermal.PowerInfo, MSR_AVN_PKG_POWER_INFO);
}

static void Query_SandyBridge(unsigned int cpu)
{
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	Intel_PowerInterface();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_SandyBridge_EP(unsigned int cpu)
{
	Query_SandyBridge(cpu);
}

static void Query_IvyBridge(unsigned int cpu)
{
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	Intel_PowerInterface();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_IvyBridge_EP(unsigned int cpu)
{
	IvyBridge_EP_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	Intel_PowerInterface();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Haswell(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	SandyBridge_Uncore_Ratio(cpu);
	Intel_PowerInterface();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Haswell_EP(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Haswell_EP_Platform_Info(cpu);
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	Intel_PowerInterface();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Haswell_ULT(unsigned int cpu)
{
	Query_IvyBridge(cpu);
}

static void Query_Haswell_ULX(unsigned int cpu)
{
	Query_IvyBridge(cpu);
}

static void Query_Broadwell(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Nehalem_Platform_Info(cpu);
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	Intel_PowerInterface();
	Intel_Hardware_Performance();

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Broadwell_EP(unsigned int cpu)
{
	Query_Haswell_EP(cpu);
	Intel_Hardware_Performance();
}

static void Query_Skylake(unsigned int cpu)
{
	Query_Broadwell(cpu);

	PUBLIC(RO(Proc))->Features.EEO_Capable = 1;
	PUBLIC(RO(Proc))->Features.R2H_Capable = 1;
	Skylake_PowerControl();
}

static void Query_Kaby_Lake(unsigned int cpu)
{
	Power_ACCU_Skylake = Power_ACCU_SKL_PLATFORM;
	Query_Skylake(cpu);
}

static void Query_Skylake_X(unsigned int cpu)
{
	if (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA)
	{
		PUBLIC(RO(Proc))->Features.Uncore_Unlock = 1;
	}
	Skylake_X_Platform_Info(cpu);
	HyperThreading_Technology();
	Haswell_Uncore_Ratio(NULL);
	Intel_PowerInterface();
	Intel_Hardware_Performance();
}

static unsigned short Compute_AuthenticAMD_Boost(unsigned int cpu)
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
	return SpecTurboRatio;
}

static void Query_VirtualMachine(unsigned int cpu)
{
	Query_Same_Genuine_Features();
	/* Reset Max ratio to call the clock estimation in Controller_Init() */
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = 0;
    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
    {
	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = 10;
    }
    else if ( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
	||  (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) )
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

static void Query_AuthenticAMD(unsigned int cpu)
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

static unsigned short Compute_AMD_Family_0Fh_Boost(unsigned int cpu)
{	/* Source: BKDG for AMD NPT Family 0Fh: Chap. 13.8			*/
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
	RDMSR(PUBLIC(RO(Core, AT(cpu)))->SystemRegister.HWCR, MSR_K7_HWCR);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] =	8
	+ PUBLIC(RO(Core, AT(cpu)))->SystemRegister.HWCR.Family_0Fh.StartFID;

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MAX)] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)];

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(1C) ] = \
				PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)];
	SpecTurboRatio = 1;
    }
	return SpecTurboRatio;
}

static void Query_AMD_Family_0Fh(unsigned int cpu)
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

static void Compute_AMD_Family_10h_Boost(unsigned int cpu)
{
	unsigned int pstate, sort[5] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 4; pstate++)
	{
		unsigned int fid, did;
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		fid = PstateDef.Family_10h.CpuFid + 0x10;
		did = 1 << PstateDef.Family_10h.CpuDid;

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = fid / did;
	}
}

static void Query_AMD_Family_10h(unsigned int cpu)
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

static void Compute_AMD_Family_11h_Boost(unsigned int cpu)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++)
	{
		unsigned int fid, did;
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		fid = PstateDef.Family_10h.CpuFid + 0x8;
		did = 1 << PstateDef.Family_10h.CpuDid;

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = fid / did;
	}
}

static void Query_AMD_Family_11h(unsigned int cpu)
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

static void Compute_AMD_Family_12h_Boost(unsigned int cpu)
{
	unsigned int pstate, sort[8] = {
		BOOST(1C), BOOST(MAX), BOOST(2C), BOOST(3C),
		BOOST(4C), BOOST(5C) , BOOST(6C), BOOST(MIN)
	};
	for (pstate = 0; pstate <= 7; pstate++)
	{
		unsigned int fid, did;
		PSTATEDEF PstateDef = {.value = 0};
		RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE + pstate));

		fid = PstateDef.Family_12h.CpuFid + 0x10;
		did = PstateDef.Family_12h.CpuDid;

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = fid / did;
	}
}

static void Query_AMD_Family_12h(unsigned int cpu)
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

static void Compute_AMD_Family_14h_Boost(unsigned int cpu)
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

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = \
		(MaxFreq * 4) / ClockDiv;
	}	/*	Frequency @ MainPllOpFidMax (MHz)		*/
}

static void Query_AMD_Family_14h(unsigned int cpu)
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

	return COF;
}

inline unsigned int AMD_F15h_CoreFID(unsigned int COF, unsigned int DID)
{
	unsigned int FID = (COF * (1 << DID)) - 0x10;

	return FID;
}

static void Compute_AMD_Family_15h_Boost(unsigned int cpu)
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

static void Query_AMD_Family_15h(unsigned int cpu)
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
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_PILEDRIVER],
		CODENAME_LEN);
      }
	break;
    case 0x1:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_PILEDRIVER],
		CODENAME_LEN);
      }
	break;
    case 0x3:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_STEAMROLLER],
		CODENAME_LEN);
      }
	break;
    case 0x6:
    case 0x7:
      if ( (PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
	&& (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf) )
      {
	StrCopy(PUBLIC(RO(Proc))->Architecture,
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[CN_EXCAVATOR],
		CODENAME_LEN);
	/*	One thermal sensor through the SMU interface		*/
	PUBLIC(RO(Proc))->thermalFormula = \
				CoreFreqK_Thermal_Scope(FORMULA_SCOPE_PKG);
      }
	break;
    }
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

inline COF_ST AMD_Zen_CoreCOF(PSTATEDEF PStateDef)
{/* Source: PPR for AMD Family 17h Model 01h, Revision B1 Processors
    CoreCOF = (PStateDef[CpuFid[7:0]] / PStateDef[CpuDfsId]) * 200	*/
	unsigned long remainder;
	COF_ST COF;
    if (PStateDef.Family_17h.CpuDfsId > 0b111) {
	COF.Q	= (PStateDef.Family_17h.CpuFid << 1)
		/ PStateDef.Family_17h.CpuDfsId;
	remainder = (UNIT_KHz(1) * (PStateDef.Family_17h.CpuFid
		  - ((COF.Q * PStateDef.Family_17h.CpuDfsId) >> 1))) >> 2;
	COF.R	= (unsigned short) remainder;
    } else switch (PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily) {
	case 0xB:	/*	Zen5: Granite Ridge, Strix Point, Turin */
		COF.Q = (PStateDef.Family_1Ah.CpuFid >> 1) / 10;
		remainder = (PRECISION * PStateDef.Family_1Ah.CpuFid) >> 1;
		remainder = remainder - (UNIT_KHz(1) * COF.Q);
		COF.R = (unsigned short) remainder;
		break;
	default:
		COF.Q = PStateDef.Family_17h.CpuFid >> 2;
		COF.R = 0;
		break;
	}
	return COF;
}

inline unsigned short AMD_Zen_Compute_FID_DID(	unsigned int COF,
						unsigned int *FID,
						unsigned int *DID )
{
	unsigned int tmp = (*FID);
	if ((*DID) != 0) {
		tmp = (COF * (*DID)) >> 1;
	} else {
		tmp = COF << 2;
	}
	if (tmp < (1 << 8)) {
		(*FID) = tmp;
		return 0;
	} else {
		return 1;
	}
}

static unsigned short AMD_Zen_CoreFID(	unsigned int COF,
					unsigned int *FID,
					unsigned int *DID )
{
	unsigned short ret = AMD_Zen_Compute_FID_DID(COF, FID, DID);
	while ((ret != 0) && ((*DID) > 0))
	{
		(*DID) = (*DID) >> 1;
		ret = AMD_Zen_Compute_FID_DID(COF, FID, DID);
	}
	return ret;
}

static bool Compute_AMD_Zen_Boost(unsigned int cpu)
{
	COF_ST COF = {.Q = 0, .R = 0};
	unsigned int pstate, sort[8] = { /* P[0..7]-States */
		BOOST(MAX), BOOST(1C), BOOST(2C), BOOST(3C),
		BOOST(4C) , BOOST(5C), BOOST(6C), BOOST(7C)
	};
	PSTATEDEF PstateDef = {.value = 0};
	PSTATECTRL PstateCtrl = {.value = 0};

    for (pstate = BOOST(MIN); pstate < BOOST(SIZE); pstate++) {
		PUBLIC(RO(Core, AT(cpu)))->Boost[pstate] = 0;
    }
    if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.HwPstate)
    {
	PSTATELIMIT PstateLimit = {.value = 0};

	RDMSR(PstateLimit, MSR_AMD_PSTATE_CURRENT_LIMIT);

	RDMSR(PstateDef, (MSR_AMD_PSTATE_DEF_BASE
			+ PstateLimit.Family_17h.PstateMaxVal));

	COF = AMD_Zen_CoreCOF(PstateDef);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)] = COF.Q;
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
		COF = AMD_Zen_CoreCOF(PstateDef);

		PUBLIC(RO(Core, AT(cpu)))->Boost[sort[pstate]] = COF.Q;
	}
    }
	PUBLIC(RO(Proc))->Features.SpecTurboRatio = pstate;

	/*		Read the Target P-State				*/
	RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
	RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + PstateCtrl.PstateCmd);

	COF = AMD_Zen_CoreCOF(PstateDef);

	PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(TGT)] = COF.Q;

	/*	If CPB is enabled then add Boost + XFR to the P0 ratio. */
	RDMSR(PUBLIC(RO(Core, AT(cpu)))->SystemRegister.HWCR, MSR_K7_HWCR);
    if (PUBLIC(RO(Core, AT(cpu)))->SystemRegister.HWCR.Family_17h.CpbDis == 0)
    {
	AMD_17_ZEN2_COF XtraCOF = {.value = 0};

	switch (PUBLIC(RO(Proc))->ArchID) {
	case AMD_Zen5_Eldora:
	case AMD_Zen5_STX:
	case AMD_Zen4_HWK:
	case AMD_Zen4_PHX2:
	case AMD_Zen4_PHXR:
	case AMD_Zen4_PHX:
	case AMD_Zen4_RPL:
	case AMD_Zen3Plus_RMB:
	case AMD_Zen3_VMR:
	case AMD_Zen2_MTS:
		Core_AMD_SMN_Read(XtraCOF,
				SMU_AMD_F17H_MATISSE_COF,
				PRIVATE(OF(Zen)).Device.DF);
		break;
	case AMD_Zen5_Turin:
	case AMD_Zen5_Turin_Dense:
	case AMD_Zen4_Bergamo:
	case AMD_EPYC_Rome_CPK:
		Core_AMD_SMN_Read(XtraCOF,
				SMU_AMD_F17H_ZEN2_MCM_COF,
				PRIVATE(OF(Zen)).Device.DF);
		break;
	case AMD_Zen4_Genoa:
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
	return true;	/* Have CPB: inform to add the Xtra Boost ratios */
    } else {
	return false;	/* CPB is disabled				*/
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
	COF_ST COF = {.Q = 0, .R = 0};
	unsigned int pstate,
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
		COF = AMD_Zen_CoreCOF(PstateDef);
		if (COF.Q == target) {
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
	COF_ST COF = {.Q = 0, .R = 0};
	unsigned int FID, DID;
	const unsigned int smp = pClockZen->pClockMod->cpu == -1 ?
		smp_processor_id() : pClockZen->pClockMod->cpu;

	/*	Make sure the Core Performance Boost is disabled.	*/
	RDMSR(PUBLIC(RO(Core, AT(smp)))->SystemRegister.HWCR, MSR_K7_HWCR);
  if (PUBLIC(RO(Core, AT(smp)))->SystemRegister.HWCR.Family_17h.CpbDis)
  {
	/*	Apply if and only if the P-State is enabled ?		*/
	RDMSR(PstateDef, pClockZen->PstateAddr);
    if (PstateDef.Family_17h.PstateEn)
    {
	if (pClockZen->pClockMod->cpu == -1) { /* Request an absolute COF */
		COF.Q = pClockZen->pClockMod->Ratio;
	} else { /* Compute the requested COF based on a frequency offset */
		COF = AMD_Zen_CoreCOF(PstateDef);

		COF.Q = COF.Q + pClockZen->pClockMod->Offset;
	}
	FID = PstateDef.Family_17h.CpuFid;
	DID = PstateDef.Family_17h.CpuDfsId;
	/*	Attempt to write the P-State MSR with new FID and DID	*/
	if (AMD_Zen_CoreFID(COF.Q, &FID, &DID) == 0)
	{
		PstateDef.Family_17h.CpuFid = FID;
		PstateDef.Family_17h.CpuDfsId = DID;
		WRMSR(PstateDef, pClockZen->PstateAddr);

		pClockZen->rc = RC_OK_COMPUTE;
	} else {
		pClockZen->rc = -RC_PSTATE_NOT_FOUND;
	}
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
	COMPUTE_ARG Compute = { .TSC = { NULL, NULL } };

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
			AMD_Zen_CoreCOF(PstateDef).Q;

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
			AMD_Zen_CoreCOF(PstateDef).Q;

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

inline unsigned short CPPC_AMD_Zen_ScaleRatio(	CORE_RO *Core,
						unsigned int scale,
						unsigned short hint,
						unsigned short CPB )
{
	unsigned int max_1C;
	unsigned short scaled;
	if (CPB) {
		max_1C = Core->Boost[BOOST(CPB)] * hint;
	} else {
		max_1C = Core->Boost[BOOST(MAX)] * hint;
	}
	if (scale > 0) {
		scaled = DIV_ROUND_CLOSEST(max_1C, scale);
		scaled = scaled & 0xff;
	} else {
		scaled = 0;
	}
	return scaled;
}

inline unsigned int CPPC_AMD_Zen_ScaleHint(	CORE_RO *Core,
						unsigned int scale,
						signed int ratio,
						unsigned short CPB )
{
	enum RATIO_BOOST boost;
	unsigned int flag = 1 << 31;

	if (CPB) {
		boost = BOOST(CPB);
	} else {
		boost = BOOST(MAX);
	}
	if ((ratio >= 0) && Core->Boost[boost] && (ratio <= Core->Boost[boost]))
	{
		unsigned short hint;
		hint = DIV_ROUND_CLOSEST(scale * ratio, Core->Boost[boost]);
		flag = flag ^ (1 << 31);
		flag = flag | hint;
	}
	return flag;
}

static signed int Put_ACPI_CPPC_Registers(unsigned int cpu, void *arg)
{
#if defined (CONFIG_ACPI_CPPC_LIB) \
 && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;

	struct cppc_perf_fb_ctrs CPPC_Perf;
	struct cppc_perf_caps CPPC_Caps;

	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);

    if ((cppc_get_perf_ctrs(Core->Bind, &CPPC_Perf) == 0)
     && (cppc_get_perf_caps(Core->Bind, &CPPC_Caps) == 0))
    {
	unsigned short WrRdCPPC = 0;

	struct cppc_perf_ctrls perf_ctrls = {
		.max_perf = CPPC_Caps.highest_perf,
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
		.min_perf = CPPC_AMD_Zen_ScaleHint(
				Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				CPPC_Caps.lowest_freq / PRECISION,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			),
	    #else
		.min_perf = CPPC_Caps.lowest_perf,
	    #endif
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
		.desired_perf = CPPC_Perf.reference_perf
	    #else
		.desired_perf = CPPC_Caps.reference_perf
	    #endif
	};
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
	unsigned long long desired_perf = 0;
	if (cppc_get_desired_perf(Core->Bind, &desired_perf) == 0) {
		perf_ctrls.desired_perf = desired_perf;
	} else {
		pr_debug("CoreFreq: cppc_get_desired_perf(cpu=%u) error\n",
			Core->Bind);
	}
    #endif

	switch (pClockZen->pClockMod->NC) {
	case CLOCK_MOD_HWP_MIN:
		pClockZen->rc = -RC_UNIMPLEMENTED;
		break;
	case CLOCK_MOD_HWP_MAX:
		pClockZen->rc = -RC_UNIMPLEMENTED;
		break;
	case CLOCK_MOD_HWP_TGT:
	{
		unsigned int hint;
	    if (pClockZen->pClockMod->cpu == -1) {
		hint = CPPC_AMD_Zen_ScaleHint(
				Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				pClockZen->pClockMod->Ratio,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
		perf_ctrls.desired_perf = hint & 0xff;
		WrRdCPPC = 1;
	    } else if (pClockZen->pClockMod->cpu == cpu) {
		hint = CPPC_AMD_Zen_ScaleHint(
				Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				Core->Boost[BOOST(HWP_TGT)]
				+ pClockZen->pClockMod->Offset,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
		perf_ctrls.desired_perf = hint & 0xff;
		WrRdCPPC = 1;
	    }
	}
		break;
	default:
		pClockZen->rc = -RC_UNIMPLEMENTED;
		break;
	}
	if (WrRdCPPC == 1)
	{
		cppc_set_perf(cpu, &perf_ctrls);

	    if (cppc_get_perf_caps(cpu, &CPPC_Caps) == 0)
	    {
		perf_ctrls.max_perf = CPPC_Caps.highest_perf;
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
		perf_ctrls.min_perf = \
			CPPC_AMD_Zen_ScaleHint(
				Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				CPPC_Caps.lowest_freq / PRECISION,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
		Core->PowerThermal.ACPI_CPPC.Minimum = CPPC_Caps.lowest_freq;
	    #else
		perf_ctrls.min_perf = CPPC_Caps.lowest_perf;

		Core->PowerThermal.ACPI_CPPC.Minimum = \
			CPPC_AMD_Zen_ScaleRatio(
				Core,
				255U,
				CPPC_Caps.lowest_perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
	    #endif

		Core->PowerThermal.ACPI_CPPC.Maximum = CPPC_Caps.highest_perf;
	    } else {
		pr_debug("CoreFreq: cppc_get_perf_caps(cpu=%u) error\n", cpu);
	    }
	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 1, 0)
	    if (cppc_get_desired_perf(cpu, &desired_perf) == 0) {
		perf_ctrls.desired_perf = desired_perf;

		Core->PowerThermal.ACPI_CPPC.Desired = desired_perf;
	    } else {
		pr_debug("CoreFreq: cppc_get_desired_perf(cpu=%u) error\n",cpu);
	    }
	    #endif
		pClockZen->rc = RC_OK_COMPUTE;
	}
    } else {
	pr_debug("CoreFreq: cppc_get_perf_*(cpu=%u) error\n", Core->Bind);
    }
	return 0;
#else
	return -ENODEV;
#endif /* CONFIG_ACPI_CPPC_LIB */
}

static signed int Write_ACPI_CPPC_Registers(unsigned int cpu, void *arg)
{
	signed int rc = Put_ACPI_CPPC_Registers(cpu, arg);

	Compute_ACPI_CPPC_Bounds(cpu);

	return rc;
}

static void CPPC_AMD_Zen_PerCore(void *arg)
{
	CLOCK_ZEN_ARG *pClockZen = (CLOCK_ZEN_ARG *) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	AMD_CPPC_REQUEST CPPC_Req = {.value = 0};
	unsigned short WrRdCPPC = 0;

	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);
	RDMSR(CPPC_Req, MSR_AMD_CPPC_REQ);

    switch (pClockZen->pClockMod->NC) {
    case CLOCK_MOD_HWP_MIN:
    {
	unsigned int hint;
      if (pClockZen->pClockMod->cpu == -1) {
	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U,
				pClockZen->pClockMod->Ratio,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
      } else {
	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U,
				Core->Boost[BOOST(HWP_MIN)]
				+ pClockZen->pClockMod->Offset,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
      }
      if ((hint & (1 << 31)) == (1 << 31))
      {
	pClockZen->rc = -ERANGE;
      } else {
	CPPC_Req.Minimum_Perf = hint & 0xff;
	WrRdCPPC = 1;
      }
    }
	break;
    case CLOCK_MOD_HWP_MAX:
    {
	unsigned int hint;
      if (pClockZen->pClockMod->cpu == -1) {
	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U,
				pClockZen->pClockMod->Ratio,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
      } else {
	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U,
				Core->Boost[BOOST(HWP_MAX)]
				+ pClockZen->pClockMod->Offset,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
      }
      if ((hint & (1 << 31)) == (1 << 31))
      {
	pClockZen->rc = -ERANGE;
      } else {
	CPPC_Req.Maximum_Perf = hint & 0xff;
	WrRdCPPC = 1;
      }
    }
	break;
    case CLOCK_MOD_HWP_TGT:
    {
	unsigned int hint;
      if (pClockZen->pClockMod->cpu == -1) {
	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U,
				pClockZen->pClockMod->Ratio,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
      } else {
	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U,
				Core->Boost[BOOST(HWP_TGT)]
				+ pClockZen->pClockMod->Offset,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
      }
      if ((hint & (1 << 31)) == (1 << 31))
      {
	pClockZen->rc = -ERANGE;
      } else {
	CPPC_Req.Desired_Perf = hint & 0xff;
	WrRdCPPC = 1;
      }
    }
	break;
    default:
	pClockZen->rc = -RC_UNIMPLEMENTED;
	break;
    }
    if (WrRdCPPC == 1)
    {
	WRMSR(CPPC_Req, MSR_AMD_CPPC_REQ);
	RDMSR(CPPC_Req, MSR_AMD_CPPC_REQ);

	Core->PowerThermal.HWP_Request.Minimum_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Minimum_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Request.Maximum_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Maximum_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Request.Desired_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Desired_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->Boost[BOOST(HWP_MIN)]=Core->PowerThermal.HWP_Request.Minimum_Perf;
	Core->Boost[BOOST(HWP_MAX)]=Core->PowerThermal.HWP_Request.Maximum_Perf;
	Core->Boost[BOOST(HWP_TGT)]=Core->PowerThermal.HWP_Request.Desired_Perf;

	pClockZen->rc = RC_OK_COMPUTE;
    }
}

static long For_All_AMD_Zen_Clock(	CLOCK_ZEN_ARG *pClockZen,
					void (*PerCore)(void *) )
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

	return rc;
}

static long ClockSource_OC_Granted(void)
{
	long rc = -RC_CLOCKSOURCE;

	char *clockName = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (clockName != NULL)
    {
	struct file *clockFile=filp_open(CLOCKSOURCE_PATH"/current_clocksource",
					O_RDWR, (umode_t) 0);
      if (clockFile != NULL)
      {
	loff_t pos = 0;
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	ssize_t len = kernel_read(clockFile, clockName, PAGE_SIZE - 1, &pos);
	#else
	int len = kernel_read(clockFile, pos, clockName, PAGE_SIZE - 1);
	#endif
	if (len > 0)
	{
		const struct {
			char *name;
			size_t len;
		} whiteList[] = {
			{ "hpet",	__builtin_strlen("hpet")	},
			{ "acpi_pm",	__builtin_strlen("acpi_pm")	},
			{ "jiffies",	__builtin_strlen("jiffies")	}
		};
		const size_t dim = sizeof(whiteList) / sizeof(whiteList[0]);
		size_t idx;

		for (idx = 0; idx < dim; idx++) {
			if (0 == strncmp(whiteList[idx].name, clockName,
					whiteList[idx].len))
			{
				rc = RC_SUCCESS;
				break;
			}
		}
	} else {
		rc = len == 0 ? -EAGAIN : len;
	}
	filp_close(clockFile, NULL);
      } else {
	rc = -ENOENT;
      }
	kfree(clockName);
    } else {
	rc = -ENOMEM;
    }
	return rc;
}

static long For_All_AMD_Zen_BaseClock( CLOCK_ZEN_ARG *pClockZen,
					void (*PerCore)(void*) )
{
	long rc;
	unsigned int cpu = PUBLIC(RO(Proc))->Service.Core;

  if ((rc = ClockSource_OC_Granted()) == RC_SUCCESS)
  {
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
  }
	return rc;
}

static long TurboClock_AMD_Zen(CLOCK_ARG *pClockMod)
{
  if (pClockMod != NULL) {
    if ((pClockMod->NC >= 1) && (pClockMod->NC <= 7))
    {
	CLOCK_ZEN_ARG ClockZen = {	/* P[1..7]-States allowed	*/
		.pClockMod  = pClockMod,
		.PstateAddr = MSR_AMD_PSTATE_DEF_BASE + pClockMod->NC,
		.BoostIndex = BOOST(SIZE) - pClockMod->NC,
		.rc = RC_SUCCESS
	};
	return For_All_AMD_Zen_Clock(&ClockZen, TurboClock_AMD_Zen_PerCore);
    } else {
	return -RC_UNIMPLEMENTED;
    }
  } else {
	return -EINVAL;
  }
}

static long ClockMod_AMD_Zen(CLOCK_ARG *pClockMod)
{
  if (pClockMod != NULL) {
    switch (pClockMod->NC) {
    case CLOCK_MOD_MAX:
      {
	CLOCK_ZEN_ARG ClockZen = {	/* P[0]:Max non-boosted P-State */
		.pClockMod  = pClockMod,
		.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
		.BoostIndex = BOOST(MAX),
		.rc = RC_SUCCESS
	};
	return(For_All_AMD_Zen_BaseClock(&ClockZen, BaseClock_AMD_Zen_PerCore));
      }
    case CLOCK_MOD_TGT:
      {
	CLOCK_ZEN_ARG ClockZen = {	/* Target non-boosted P-State*/
		.pClockMod  = pClockMod,
		.PstateAddr = MSR_AMD_PSTATE_DEF_BASE,
		.BoostIndex = BOOST(TGT),
		.rc = RC_SUCCESS
	};
	return For_All_AMD_Zen_Clock(&ClockZen, TargetClock_AMD_Zen_PerCore);
      }
    case CLOCK_MOD_HWP_MIN:
    case CLOCK_MOD_HWP_MAX:
    case CLOCK_MOD_HWP_TGT:
      {
	CLOCK_ZEN_ARG ClockZen = {
		.pClockMod = pClockMod,
		.rc = RC_SUCCESS
	};
	if (PUBLIC(RO(Proc))->Features.HWP_Enable == 1) {
		return For_All_AMD_Zen_Clock(&ClockZen, CPPC_AMD_Zen_PerCore);
	}
	else if (PUBLIC(RO(Proc))->Features.ACPI_CPPC == 1)
	{
		For_All_ACPI_CPPC(Write_ACPI_CPPC_Registers, &ClockZen);
		return ClockZen.rc;
	} else {
		return -RC_UNIMPLEMENTED;
	}
      }
    default:
	return -RC_UNIMPLEMENTED;
    }
  } else {
	return -EINVAL;
  }
}

static void Query_AMD_F17h_Power_Limits(PROC_RO *Pkg)
{	/*		Package Power Tracking				*/
	Core_AMD_SMN_Read( Pkg->PowerThermal.Zen.PWR,
				SMU_AMD_F17H_ZEN2_MCM_PWR,
				PRIVATE(OF(Zen)).Device.DF );
	/*		Junction Temperature				*/
	if (Pkg->PowerThermal.Zen.PWR.TjMax > 0) {
		Pkg->PowerThermal.Param.Offset[THERMAL_TARGET] = \
					Pkg->PowerThermal.Zen.PWR.TjMax;
	}
	/*		Thermal Design Power				*/
	Core_AMD_SMN_Read( Pkg->PowerThermal.Zen.TDP,
				SMU_AMD_F17H_ZEN2_MCM_TDP,
				PRIVATE(OF(Zen)).Device.DF );
	/*		Electric Design Current				*/
	Core_AMD_SMN_Read( Pkg->PowerThermal.Zen.EDC,
				SMU_AMD_F17H_ZEN2_MCM_EDC,
				PRIVATE(OF(Zen)).Device.DF );
}

static unsigned int Query_AMD_HSMP_Interface(void)
{
	HSMP_ARG arg[8];
	unsigned int rx = HSMP_UNSPECIFIED;

	if (PUBLIC(RO(Proc))->Features.HSMP_Capable)
	{ /* Mark the SMU as Enable if the reachability test is successful */
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_TEST_MSG, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 1;
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	} else {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	}

	PUBLIC(RO(Proc))->Features.Factory.SMU.Version = 0;
	if (PUBLIC(RO(Proc))->Features.HSMP_Enable)
	{
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_RD_SMU_VER, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->Features.Factory.SMU.Version = arg[0].value;
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	}

	PUBLIC(RO(Proc))->Features.Factory.SMU.Interface = 0;
	if (PUBLIC(RO(Proc))->Features.HSMP_Enable)
	{
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_RD_VERSION, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->Features.Factory.SMU.Interface = arg[0].value;
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	}

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Enable_Limit1 = \

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Clamping1 = \

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].Unlock = \

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Domain_Limit1 = 0;
	if (PUBLIC(RO(Proc))->Features.HSMP_Enable)
	{
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_RD_PKG_PL1, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Domain_Limit1 = arg[0].value / 1000;

		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Enable_Limit1 = \

		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Clamping1 = \

		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].Unlock = \
		/* Assumed PL1 is enabled, clamped, unlocked if value exists */
		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Domain_Limit1 > 0 ? 1 : 0;
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	}

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Enable_Limit2 = \

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Clamping2 = \

	PUBLIC(RO(Proc))->PowerThermal.Domain[
		PWR_DOMAIN(PKG)
	].PowerLimit.Domain_Limit2 = 0;
	if (PUBLIC(RO(Proc))->Features.HSMP_Enable)
	{
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_RD_MAX_PPT, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Domain_Limit2 = arg[0].value / 1000;

		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Enable_Limit2 = \

		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Clamping2 = \
		/*	Assumed PL2 is enabled, clamped if value exists */
		PUBLIC(RO(Proc))->PowerThermal.Domain[
			PWR_DOMAIN(PKG)
		].PowerLimit.Domain_Limit2 > 0 ? 1 : 0;
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	}

	PUBLIC(RO(Proc))->Counter[0].FCLK = \
	PUBLIC(RO(Proc))->Counter[1].FCLK = \
	PUBLIC(RO(Proc))->Delta.FCLK = 0;
	if (PUBLIC(RO(Proc))->Features.HSMP_Enable)
	{
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_RD_DF_MCLK, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->Counter[0].FCLK = \
		PUBLIC(RO(Proc))->Counter[1].FCLK = \
		PUBLIC(RO(Proc))->Delta.FCLK = UNIT_MHz(arg[0].value);
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	}
	return rx;
}

static void Query_AMD_Family_17h(unsigned int cpu)
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

	if (Compute_AMD_Zen_Boost(cpu) == true)
	{	/*	Count the Xtra Boost ratios			*/
		PUBLIC(RO(Proc))->Features.XtraCOF = 2;
	}
	else {	/*	Disabled CPB: Hide ratios			*/
		PUBLIC(RO(Proc))->Features.XtraCOF = 0;
	}
	/*	Apply same register bit fields as Intel RAPL_POWER_UNIT */
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.Unit, MSR_AMD_RAPL_POWER_UNIT);

	HyperThreading_Technology();

	AMD_Processor_PIN(PUBLIC(RO(Proc))->Features.leaf80000008.EBX.PPIN);

	Query_AMD_HSMP_Interface();
}

static void Exit_AMD_F17h(void)
{
#ifdef CONFIG_AMD_NB
	if (PRIVATE(OF(Zen)).Device.DF != NULL)
	{
		pci_dev_put(PRIVATE(OF(Zen)).Device.DF);
		PRIVATE(OF(Zen)).Device.DF = NULL;
	}
#endif /* CONFIG_AMD_NB */
}

static void Query_AMD_F17h_PerSocket(unsigned int cpu)
{
	Core_AMD_Family_17h_Temp = CTL_AMD_Family_17h_Temp;

	Probe_AMD_DataFabric();

	Query_AMD_Family_17h(cpu);

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Query_AMD_F17h_Power_Limits(PUBLIC(RO(Proc)));
		if (AMD_F17h_CPPC() == -ENODEV) {
			For_All_ACPI_CPPC(Read_ACPI_CPPC_Registers, NULL);
		}
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_AMD_F17h_PerCluster(unsigned int cpu)
{
	Core_AMD_Family_17h_Temp = CCD_AMD_Family_17h_Zen2_Temp;

	Probe_AMD_DataFabric();

	Query_AMD_Family_17h(cpu);

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Query_AMD_F17h_Power_Limits(PUBLIC(RO(Proc)));
		if (AMD_F17h_CPPC() == -ENODEV) {
			For_All_ACPI_CPPC(Read_ACPI_CPPC_Registers, NULL);
		}
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_AMD_F19h_11h_PerCluster(unsigned int cpu)
{
	Core_AMD_Family_17h_Temp = CCD_AMD_Family_19h_Genoa_Temp;

	Probe_AMD_DataFabric();

	Query_AMD_Family_17h(cpu);

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Query_AMD_F17h_Power_Limits(PUBLIC(RO(Proc)));
		if (AMD_F17h_CPPC() == -ENODEV) {
			For_All_ACPI_CPPC(Read_ACPI_CPPC_Registers, NULL);
		}
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_AMD_F19h_61h_PerCluster(unsigned int cpu)
{
	Core_AMD_Family_17h_Temp = CCD_AMD_Family_19h_Zen4_Temp;

	Probe_AMD_DataFabric();

	Query_AMD_Family_17h(cpu);

	if (cpu == PUBLIC(RO(Proc))->Service.Core) {
		Query_AMD_F17h_Power_Limits(PUBLIC(RO(Proc)));
		if (AMD_F17h_CPPC() == -ENODEV) {
			For_All_ACPI_CPPC(Read_ACPI_CPPC_Registers, NULL);
		}
		Read_ACPI_PCT_Registers(cpu);
		Read_ACPI_PSS_Registers(cpu);
		Read_ACPI_PPC_Registers(cpu);
		Read_ACPI_CST_Registers(cpu);
	}
}

static void Query_Hygon_F18h(unsigned int cpu)
{
	switch (PUBLIC(RO(Proc))->Features.Std.EAX.Model) {
	case 0x0:
		switch (PUBLIC(RO(Proc))->Features.Std.EAX.Stepping) {
		case 0x2:
			StrCopy(PUBLIC(RO(Proc))->Architecture,
				Arch_Hygon_Family_18h[CN_DHYANA_V1],
				CODENAME_LEN);
			break;
		}
		break;
	case 0x1:
		switch (PUBLIC(RO(Proc))->Features.Std.EAX.Stepping) {
		case 0x1:
			StrCopy(PUBLIC(RO(Proc))->Architecture,
				Arch_Hygon_Family_18h[CN_DHYANA_V2],
				CODENAME_LEN);
			break;
		}
		break;
	}
	Query_AMD_F17h_PerSocket(cpu);
}

static void Dump_CPUID(CORE_RO *Core)
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

static void AMD_F17h_DCU_Technology(CORE_RO *Core)		/* Per Core */
{
	AMD_DC_CFG DC_Cfg1 = {.value = 0};
	AMD_CU_CFG3 CU_Cfg3 = {.value = 0};
	AMD_IC_CFG IC_Cfg = {.value = 0};

	RDMSR(DC_Cfg1, MSR_AMD_DC_CFG);
	switch (L1_HW_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		DC_Cfg1.L1_HW_Prefetch = L1_HW_PREFETCH_Disable;
		WRMSR(DC_Cfg1, MSR_AMD_DC_CFG);
		RDMSR(DC_Cfg1, MSR_AMD_DC_CFG);
		break;
	}

	RDMSR(CU_Cfg3, MSR_AMD_CU_CFG3);
	switch (L2_HW_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		CU_Cfg3.L2_HW_Prefetch = !L2_HW_PREFETCH_Disable;
		WRMSR(CU_Cfg3, MSR_AMD_CU_CFG3);
		RDMSR(CU_Cfg3, MSR_AMD_CU_CFG3);
		break;
	}

	RDMSR(IC_Cfg, MSR_AMD_IC_CFG);
	switch (L1_HW_IP_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		IC_Cfg.HW_IP_Prefetch = L1_HW_IP_PREFETCH_Disable;
		WRMSR(IC_Cfg, MSR_AMD_IC_CFG);
		RDMSR(IC_Cfg, MSR_AMD_IC_CFG);
		break;
	}

    if (DC_Cfg1.L1_HW_Prefetch)
    {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_Prefetch, Core->Bind);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_Prefetch, Core->Bind);
    }
    if (CU_Cfg3.L2_HW_Prefetch)
    {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_Prefetch, Core->Bind);
    } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_Prefetch, Core->Bind);
    }
    if (IC_Cfg.HW_IP_Prefetch == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_IP_Prefetch, Core->Bind);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_IP_Prefetch, Core->Bind);
    }

    if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.PrefetchCtl_MSR == 1)
    {
	int ToggleFeature = 0;
	AMD_PREFETCH_CONTROL PrefetchCtl = {.value = 0};
	RDMSR(PrefetchCtl, MSR_AMD_PREFETCH_CTRL);

	switch (L1_STRIDE_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PrefetchCtl.L1Stride = L1_STRIDE_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L1_REGION_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PrefetchCtl.L1Region = L1_REGION_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L1_BURST_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PrefetchCtl.L1Stream = L1_BURST_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L2_STREAM_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PrefetchCtl.L2Stream = L2_STREAM_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L2_UPDOWN_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		PrefetchCtl.UpDown = L2_UPDOWN_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	if (ToggleFeature == 1) {
		WRMSR(PrefetchCtl, MSR_AMD_PREFETCH_CTRL);
		RDMSR(PrefetchCtl, MSR_AMD_PREFETCH_CTRL);
	}
	if (PrefetchCtl.L1Stride)
	{
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Stride_Pf, Core->Bind);
	} else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Stride_Pf, Core->Bind);
	}
	if (PrefetchCtl.L1Region)
	{
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Region_Pf, Core->Bind);
	} else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Region_Pf, Core->Bind);
	}
	if (PrefetchCtl.L1Stream)
	{
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Burst_Pf, Core->Bind);
	} else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Burst_Pf, Core->Bind);
	}
	if (PrefetchCtl.L2Stream)
	{
	    BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_Stream_HW_Pf, Core->Bind);
	} else {
	    BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_Stream_HW_Pf, Core->Bind);
	}
	if (PrefetchCtl.UpDown)
	{
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_UpDown_Pf, Core->Bind);
	} else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_UpDown_Pf, Core->Bind);
	}
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->DCU_Mask, Core->Bind);
}

static void Intel_DCU_Technology(CORE_RO *Core) 		/*Per Core */
{ /* Avoton[06_4D], GDM[06_5C], NHM[06_1A, 06_1E, 06_1F, 06_2E], SNB+, Phi */
  if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1))
  {
	int ToggleFeature = 0;
	MISC_FEATURE_CONTROL MiscFeatCtrl = {.value = 0};
	RDMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);

	switch (L2_HW_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatCtrl.L2_HW_Prefetch = L2_HW_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L2_HW_CL_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatCtrl.L2_HW_CL_Prefetch = L2_HW_CL_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L1_HW_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatCtrl.L1_HW_Prefetch = L1_HW_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
	switch (L1_HW_IP_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatCtrl.L1_HW_IP_Prefetch = L1_HW_IP_PREFETCH_Disable;
		ToggleFeature = 1;
		break;
	}
    if (ToggleFeature == 1)
    {
	WRMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);
	RDMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);
    }
    if (MiscFeatCtrl.L2_HW_Prefetch == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_Prefetch, Core->Bind);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_Prefetch, Core->Bind);
    }
    if (MiscFeatCtrl.L2_HW_CL_Prefetch == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_CL_Prefetch, Core->Bind);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_CL_Prefetch, Core->Bind);
    }
    if (MiscFeatCtrl.L1_HW_Prefetch == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_Prefetch, Core->Bind);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_Prefetch, Core->Bind);
    }
    if (MiscFeatCtrl.L1_HW_IP_Prefetch == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_IP_Prefetch, Core->Bind);
    } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_IP_Prefetch, Core->Bind);
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->DCU_Mask, Core->Bind);

    switch (Core->T.Cluster.Hybrid.CoreType) {
    case Hybrid_Atom:
     {
	ATOM_L2_PREFETCH_0X1321 MLC_Ctrl = {.value = 0};
	ATOM_L2_PREFETCH_0X1320 LLC_Ctrl = {.value = 0};

	RDMSR(MLC_Ctrl, MSR_ATOM_L2_PREFETCH_0X1321);
	RDMSR(LLC_Ctrl, MSR_ATOM_L2_PREFETCH_0X1320);

      switch (L2_NLP_PREFETCH_Disable) {
      case COREFREQ_TOGGLE_OFF:
      case COREFREQ_TOGGLE_ON:
	MLC_Ctrl.L2_DISABLE_NEXT_LINE_PREFETCH = L2_NLP_PREFETCH_Disable;
	WRMSR(MLC_Ctrl, MSR_ATOM_L2_PREFETCH_0X1321);
	RDMSR(MLC_Ctrl, MSR_ATOM_L2_PREFETCH_0X1321);
	break;
      }
      if (MLC_Ctrl.L2_DISABLE_NEXT_LINE_PREFETCH == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_NLP_Prefetch, Core->Bind);
      } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_NLP_Prefetch, Core->Bind);
      }

      switch (LLC_Streamer_Disable) {
      case COREFREQ_TOGGLE_OFF:
      case COREFREQ_TOGGLE_ON:
	LLC_Ctrl.LLC_STREAM_DISABLE = LLC_Streamer_Disable;
	WRMSR(LLC_Ctrl, MSR_ATOM_L2_PREFETCH_0X1320);
	RDMSR(LLC_Ctrl, MSR_ATOM_L2_PREFETCH_0X1320);
	break;
      }
      if (LLC_Ctrl.LLC_STREAM_DISABLE == 1) {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->LLC_Streamer, Core->Bind);
      } else {
	BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->LLC_Streamer, Core->Bind);
      }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ECORE_Mask, Core->Bind);
     }
	break;
    case Hybrid_Core:
	/*	Invalid MSR_ATOM_L2_PREFETCH registers with P-Core	*/
	break;
    case Hybrid_RSVD1:
    case Hybrid_RSVD2:
    default:
	break;
    }
  }
}

static void Intel_Core_MicroArchControl(CORE_RO *Core)
{
	CORE_UARCH_CTL Core_Uarch_Ctl = {.value = 0};

	RDMSR(Core_Uarch_Ctl, MSR_CORE_UARCH_CTL);

	switch (L1_Scrubbing_Enable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		Core_Uarch_Ctl.L1_Scrubbing_En = L1_Scrubbing_Enable;
		WRMSR(Core_Uarch_Ctl, MSR_CORE_UARCH_CTL);
		RDMSR(Core_Uarch_Ctl, MSR_CORE_UARCH_CTL);
		break;
	}
	if (Core_Uarch_Ctl.L1_Scrubbing_En == 1) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Scrubbing, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Scrubbing, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->L1_Scrub_Mask, Core->Bind);
}

static void Intel_Core_MicroArchitecture(CORE_RO *Core) 	/* Per P-Core */
{ /* 06_7D, 06_7E, 06_8C, 06_8D, 06_97, 06_9A, 06_B7, 06_BA, 06_BF, MTL */
  if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1))
  {
    switch (Core->T.Cluster.Hybrid.CoreType) {
    case Hybrid_Atom:
	/*	No MSR_CORE_UARCH_CTL(0x541) register with E-Core	*/
	break;
    case Hybrid_Core:
      {
	MISC_FEATURE_CONTROL MiscFeatCtrl = {.value = 0};

	RDMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);

	switch (L2_AMP_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatCtrl.L2_AMP_Prefetch = L2_AMP_PREFETCH_Disable;
		WRMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);
		RDMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);
		break;
	}
	if (MiscFeatCtrl.L2_AMP_Prefetch == 1) {
	    BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_AMP_Prefetch, Core->Bind);
	} else {
	    BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_AMP_Prefetch, Core->Bind);
	}

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->L2_AMP_Mask, Core->Bind);

	Intel_Core_MicroArchControl(Core);
      }
	break;
    case Hybrid_RSVD1:
    case Hybrid_RSVD2:
    default:
	/*	Unspecified MSR_CORE_UARCH_CTL(0x541) register with IDs */
	break;
    }
  }
}

static void Intel_Ultra7_MicroArchitecture(CORE_RO *Core)
{	/* 06_AA, 06_AB, 06_AC						*/
	MISC_FEATURE_CONTROL MiscFeatCtrl = {.value = 0};
	RDMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);

	switch (L1_NPP_PREFETCH_Disable) {
	case COREFREQ_TOGGLE_OFF:
	case COREFREQ_TOGGLE_ON:
		MiscFeatCtrl.L1_NPP_Prefetch = L1_NPP_PREFETCH_Disable;
		WRMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);
		RDMSR(MiscFeatCtrl, MSR_MISC_FEATURE_CONTROL);
		break;
	}
	if (MiscFeatCtrl.L1_NPP_Prefetch == 1) {
	    BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_NPP_Prefetch, Core->Bind);
	} else {
	    BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_NPP_Prefetch, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->DCU_Mask, Core->Bind);
}

static void SpeedStep_Technology(CORE_RO *Core) 		/*Per Package*/
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

static void Intel_Turbo_Config(CORE_RO *Core, void (*ConfigFunc)(void*),
			void (*AssignFunc)(unsigned int*, TURBO_CONFIG*))
{								/*Per Package*/
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
	return GET_CORE2_TARGET(Core);
}

#define GET_NEHALEM_TARGET(Core)					\
(									\
	Core->PowerThermal.PerfControl.NHM.TargetRatio & 0xff		\
)

static unsigned int Get_Nehalem_Target(CORE_RO *Core)
{
	return GET_NEHALEM_TARGET(Core);
}

#define GET_SANDYBRIDGE_TARGET(Core)					\
(									\
	Core->PowerThermal.PerfControl.SNB.TargetRatio			\
)

static unsigned int Get_SandyBridge_Target(CORE_RO *Core)
{
	return GET_SANDYBRIDGE_TARGET(Core);
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

static int Cmp_Skylake_Target(CORE_RO *Core, unsigned int ratio)
{
	return (Core->PowerThermal.PerfControl.SNB.TargetRatio >= ratio);
}

static bool IsPerformanceControlCapable(void)
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
	return true;
}

static bool WritePerformanceControl(PERF_CONTROL *pPerfControl)
{
	bool isCapable = IsPerformanceControlCapable();
	if (isCapable == true)
	{
		WRMSR((*pPerfControl), MSR_IA32_PERF_CTL);
	}
	return isCapable;
}

static void TurboBoost_Technology(CORE_RO *Core,SET_TARGET SetTarget,
						GET_TARGET GetTarget,
						CMP_TARGET CmpTarget,
						unsigned int TurboRatio,
						unsigned int ValidRatio)
{								/* Per SMT */
	int ToggleFeature;
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
	UNUSED(GetTarget);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);
	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);

  if ( (MiscFeatures.Turbo_IDA == 0)
	&& (PUBLIC(RO(Proc))->Features.Power.EAX.TurboIDA) )
  {
    switch (TurboBoost_Enable[0]) {
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

static void DynamicAcceleration(CORE_RO *Core)			/* Unique */
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

	switch (TurboBoost_Enable[0]) {
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

static void SoC_Turbo_Override(CORE_RO *Core)
{
	int ToggleFeature;

	PKG_TURBO_CONFIG TurboCfg = {.value = 0};
	RDMSR(TurboCfg, MSR_PKG_TURBO_CFG);

	switch (TurboBoost_Enable[0]) {
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

static long For_All_PPC_Clock(CLOCK_PPC_ARG *pClockPPC)
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
	return rc;
}

static long ClockMod_Core2_PPC(CLOCK_ARG *pClockMod)
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
			return For_All_PPC_Clock(&ClockPPC);
		} else {
			return -RC_UNIMPLEMENTED;
		}
	} else {
		return -EINVAL;
	}
}

static long ClockMod_Nehalem_PPC(CLOCK_ARG *pClockMod)
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
			return For_All_PPC_Clock(&ClockPPC);
		} else {
			return -RC_UNIMPLEMENTED;
		}
	} else {
		return -EINVAL;
	}
}

static long ClockMod_SandyBridge_PPC(CLOCK_ARG *pClockMod)
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
			return For_All_PPC_Clock(&ClockPPC);
		} else {
			return -RC_UNIMPLEMENTED;
		}
	} else {
		return -EINVAL;
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

static long For_All_HWP_Clock(CLOCK_HWP_ARG *pClockHWP)
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
	return rc;
}

static long ClockMod_Intel_HWP(CLOCK_ARG *pClockMod)
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
				return For_All_HWP_Clock(&ClockHWP);
			    }
			case CLOCK_MOD_TGT:
				return ClockMod_SandyBridge_PPC(pClockMod);
			default:
				return -RC_UNIMPLEMENTED;
			}
		} else {
			return -EINVAL;
		}
	} else {
		return ClockMod_SandyBridge_PPC(pClockMod);
	}
}


static void AMD_Watchdog(CORE_RO *Core)
{	/*		CPU Watchdog Timer.				*/
	if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1))
	{
		AMD_CPU_WDT_CFG CPU_WDT_CFG = {.value = 0};
		RDMSR(CPU_WDT_CFG, MSR_AMD_CPU_WDT_CFG);

		switch (WDT_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			CPU_WDT_CFG.TmrCfgEn = WDT_Enable;
			WRMSR(CPU_WDT_CFG, MSR_AMD_CPU_WDT_CFG);
			RDMSR(CPU_WDT_CFG, MSR_AMD_CPU_WDT_CFG);
			break;
		}
		if (CPU_WDT_CFG.TmrCfgEn) {
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->WDT, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->WDT, Core->Bind);
		}
		BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
	}
}

static void PerCore_AMD_CState_BAR(CORE_RO *Core)
{	/* Families: 10h, 12h, 14h, 15h, 16h, 17h: I/O C-State Base Address. */
	CSTATE_BASE_ADDR CStateBaseAddr = {.value = 0};
	RDMSR(CStateBaseAddr, MSR_AMD_CSTATE_BAR);
	Core->Query.CStateBaseAddr = CStateBaseAddr.IOaddr;
}

static void PerCore_Query_AMD_Zen_Features(CORE_RO *Core)	/* Per SMT */
{
	ZEN_CSTATE_CONFIG CStateCfg = {.value = 0};
	int ToggleFeature;

	/*		Read The Hardware Configuration Register.	*/
	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);

	/* Query the SMM. */
	if (Core->SystemRegister.HWCR.Family_17h.SmmLock) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM, Core->Bind);
	}
	/*		Enable or Disable the Core Performance Boost.	*/
	switch (TurboBoost_Enable[0]) {
	case COREFREQ_TOGGLE_OFF:
		Core->SystemRegister.HWCR.Family_17h.CpbDis = 1;
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		Core->SystemRegister.HWCR.Family_17h.CpbDis = 0;
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1)
	{
		WRMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);
		RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);
	}
	if (Core->SystemRegister.HWCR.Family_17h.CpbDis == 0)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TurboBoost, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TurboBoost_Mask, Core->Bind);

	/*	Enable or Disable the Core C6 State. Bit[22,14,16]	*/
	RDMSR(CStateCfg, MSR_AMD_F17H_CSTATE_CONFIG);

	switch (CC6_Enable) {
	case COREFREQ_TOGGLE_OFF:
		CStateCfg.CCR2_CC6EN = 0;
		CStateCfg.CCR1_CC6EN = 0;
		CStateCfg.CCR0_CC6EN = 0;
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		CStateCfg.CCR2_CC6EN = 1;
		CStateCfg.CCR1_CC6EN = 1;
		CStateCfg.CCR0_CC6EN = 1;
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1)
	{
		WRMSR(CStateCfg, MSR_AMD_F17H_CSTATE_CONFIG);
		RDMSR(CStateCfg, MSR_AMD_F17H_CSTATE_CONFIG);
	}
	if (CStateCfg.CCR2_CC6EN && CStateCfg.CCR1_CC6EN && CStateCfg.CCR0_CC6EN
		&& Core->SystemRegister.HWCR.Family_17h.INVDWBINVD)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->CC6, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	/*	Enable or Disable the Package C6 State . Bit[32]	*/
    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	ZEN_PMGT_MISC PmgtMisc = {.value = 0};
	RDMSR(PmgtMisc, MSR_AMD_F17H_PMGT_MISC);

	switch (PC6_Enable) {
	case COREFREQ_TOGGLE_OFF:
		PmgtMisc.PC6En = 0;
		ToggleFeature = 1;
		break;
	case COREFREQ_TOGGLE_ON:
		PmgtMisc.PC6En = 1;
		ToggleFeature = 1;
		break;
	default:
		ToggleFeature = 0;
		break;
	}
	if (ToggleFeature == 1) {
		WRMSR(PmgtMisc, MSR_AMD_F17H_PMGT_MISC);
		RDMSR(PmgtMisc, MSR_AMD_F17H_PMGT_MISC);
	}
	if (PmgtMisc.PC6En == 1)
	{
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PC6, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->PC6_Mask, Core->Bind);

	PUBLIC(RO(Proc))->PowerThermal.Events[eSTS] = EVENT_THERM_NONE;
	if (PUBLIC(RO(Proc))->Features.HSMP_Enable)
	{
		unsigned int rx;
		HSMP_ARG arg[8];
		RESET_ARRAY(arg, 8, 0, .value);
		rx = AMD_HSMP_Exec(HSMP_RD_PROCHOT, arg);
	    if (rx == HSMP_RESULT_OK)
	    {
		PUBLIC(RO(Proc))->PowerThermal.Events[eSTS] = (
			((Bit64)arg[0].value & 0x1) << LSHIFT_PROCHOT_STS
		);
	    }
	    else if (IS_HSMP_OOO(rx))
	    {
		PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
	    }
	}
    }
	/*		SMT C-State Base Address.			*/
	PerCore_AMD_CState_BAR(Core);
	/*		Package C-State: Configuration Control .	*/
	Core->Query.CfgLock = 1;
	/*		Package C-State: I/O MWAIT Redirection .	*/
	Core->Query.IORedir = 0;

	AMD_Watchdog(Core);
}

static void Intel_Watchdog(CORE_RO *Core)
{
	struct pci_device_id PCI_WDT_ids[] = {
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ICH10_LPC),
		.driver_data = (kernel_ulong_t) ICH_TCO
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_PCH_C216_LPC),
		.driver_data = (kernel_ulong_t) ICH_TCO
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_EHL_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_JSL_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_C620_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_C620_SUPER_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_KBL_PCH_H_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_SPT_LP_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_SPT_H_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_CNL_PCH_LP_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_CNL_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_CML_PCH_LP_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_CML_PCH_V_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_CML_H_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ICL_LP_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ICL_PCH_NG_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_TGL_PCH_LP_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_TGL_H_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ADL_PCH_P_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ADL_PCH_M_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ADL_S_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_RPL_D_PCH_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_M_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ARL_MTL_PCH_S_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_ARROWLAKE_S_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{
		PCI_VDEVICE(INTEL, DID_INTEL_LUNARLAKE_V_SMBUS),
		.driver_data = (kernel_ulong_t) TCOBASE
		},
		{0, }
	};
	if (CoreFreqK_ProbePCI(PCI_WDT_ids, NULL, NULL) < RC_SUCCESS) {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->WDT, Core->Bind);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
}

static void Intel_Turbo_Activation_Ratio(CORE_RO *Core)
{
	TURBO_ACTIVATION TurboActivation = {.value = 0};

	RDMSR(TurboActivation, MSR_TURBO_ACTIVATION_RATIO);

    if (!TurboActivation.Ratio_Lock && (Turbo_Activation_Ratio >= 0)
     && (Turbo_Activation_Ratio != TurboActivation.MaxRatio))
    {
	const short MaxRatio = \
	MAXCLOCK_TO_RATIO(short, PUBLIC(RO(Core, AT(Core->Bind))->Clock.Hz));

	if (Turbo_Activation_Ratio <=  MaxRatio)
	{
		TurboActivation.MaxRatio = Turbo_Activation_Ratio;
		WRMSR(TurboActivation, MSR_TURBO_ACTIVATION_RATIO);
		RDMSR(TurboActivation, MSR_TURBO_ACTIVATION_RATIO);
	}
    }
	Core->Boost[BOOST(ACT)] = TurboActivation.MaxRatio;

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	PUBLIC(RO(Proc))->Features.TurboActiv_Lock = TurboActivation.Ratio_Lock;
    }
}

static void Intel_Turbo_TDP_Config(CORE_RO *Core)
{
	CONFIG_TDP_NOMINAL NominalTDP = {.value = 0};
	CONFIG_TDP_CONTROL ControlTDP = {.value = 0};
	CONFIG_TDP_LEVEL ConfigTDP[2] = {{.value = 0}, {.value = 0}};

	RDMSR(NominalTDP, MSR_CONFIG_TDP_NOMINAL);
	Core->Boost[BOOST(TDP)] = NominalTDP.Ratio;

	RDMSR(ConfigTDP[1], MSR_CONFIG_TDP_LEVEL_2);
	Core->Boost[BOOST(TDP2)] = ConfigTDP[1].Ratio;

	RDMSR(ConfigTDP[0], MSR_CONFIG_TDP_LEVEL_1);
	Core->Boost[BOOST(TDP1)] = ConfigTDP[0].Ratio;

  if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
  {
	RDMSR(ControlTDP, MSR_CONFIG_TDP_CONTROL);

	if ((ControlTDP.Lock == 0) && (Config_TDP_Level >= 0))
	{
		ControlTDP.Level = Config_TDP_Level;
		WRMSR(ControlTDP, MSR_CONFIG_TDP_CONTROL);
		RDMSR(ControlTDP, MSR_CONFIG_TDP_CONTROL);
	}
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Lock  = ControlTDP.Lock;
	PUBLIC(RO(Proc))->Features.TDP_Cfg_Level = ControlTDP.Level;

    switch (PUBLIC(RO(Proc))->Features.TDP_Cfg_Level) {
    case 2:
	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.ThermalSpecPower = \
							ConfigTDP[1].PkgPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MinimumPower = \
							ConfigTDP[1].MinPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MaximumPower = \
							ConfigTDP[1].MaxPower;
	break;
    case 1:
	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.ThermalSpecPower = \
							ConfigTDP[0].PkgPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MinimumPower = \
							ConfigTDP[0].MinPower;

	PUBLIC(RO(Proc))->PowerThermal.PowerInfo.MaximumPower = \
							ConfigTDP[0].MaxPower;
	break;
    case 0:
	RDMSR(PUBLIC(RO(Proc))->PowerThermal.PowerInfo, MSR_PKG_POWER_INFO);
	break;
    }
  }
}

static void Query_Intel_C1E(CORE_RO *Core)			/*Per Package*/
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

static void Query_AMD_Family_0Fh_C1E(CORE_RO *Core)		/* Per Core */
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

static void ThermalMonitor2_Set(CORE_RO *Core, MISC_PROC_FEATURES MiscFeatures)
{	/* Intel Core Solo Duo. */
	struct SIGNATURE whiteList[] = {
		_Core_Yonah	,	/* 06_0E */
		_Core_Conroe	,	/* 06_0F */
		_Core_Penryn	,	/* 06_17 */
		_Atom_Bonnell	,	/* 06_1C */
		_Atom_Silvermont,	/* 06_26 */
		_Atom_Lincroft ,	/* 06_27 */
		_Atom_Clover_Trail,	/* 06_35 */
		_Atom_Saltwell	,	/* 06_36 */
		_Atom_Airmont	,	/* 06_4C */
		_Tigerlake_U	,	/* 06_8C */
		_Alderlake_S		/* 06_97 */
	};
	const int ids = sizeof(whiteList) / sizeof(whiteList[0]);
	int id;
  for (id = 0; id < ids; id++)
  {
    if((whiteList[id].ExtFamily == PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily)
    && (whiteList[id].Family == PUBLIC(RO(Proc))->Features.Std.EAX.Family)
    && (whiteList[id].ExtModel == PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel)
    && (whiteList[id].Model == PUBLIC(RO(Proc))->Features.Std.EAX.Model))
    {
	if (MiscFeatures.TCC)
	{
		THERM2_CONTROL Therm2Control = {.value = 0};

		RDMSR(Therm2Control, MSR_THERM2_CTL);

		if (Therm2Control.TM_SELECT)
		{
			BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM1, Core->Bind);
			BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2, Core->Bind);
		} else {
			BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2, Core->Bind);
		}
	}
	break;
    }
  }
}

static void ThermalMonitor_IA32(CORE_RO *Core)
{
	MISC_PROC_FEATURES MiscFeatures = {.value = 0};
	THERM_STATUS ThermStatus = {.value = 0};
	/*		Query the TM1 and TM2 features state.		*/
	RDMSR(MiscFeatures, MSR_IA32_MISC_ENABLE);
	if (MiscFeatures.TCC) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TM1, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM1, Core->Bind);
	}
	if (MiscFeatures.TM2_Enable) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2, Core->Bind);
	}
	ThermalMonitor2_Set(Core, MiscFeatures);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask, Core->Bind);

    if (PUBLIC(RO(Proc))->Features.Std.EDX.ACPI)
    {	/*	Clear Thermal Events if requested by User.		*/
	THERM_INTERRUPT ThermInterrupt = {.value = 0};
	unsigned short TjMax, ClearBit = 0;

	COMPUTE_THERMAL(INTEL, TjMax, Core->PowerThermal.Param, 0);

	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);

	if (Clear_Events & EVENT_THERMAL_LOG) {
		ThermStatus.Thermal_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_PROCHOT_LOG) {
		ThermStatus.PROCHOT_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CRITIC_LOG) {
		ThermStatus.CriticalTemp_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THOLD1_LOG) {
		ThermStatus.Threshold1_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THOLD2_LOG) {
		ThermStatus.Threshold2_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_POWER_LIMIT) {
		ThermStatus.PwrLimit_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CURRENT_LIMIT) {
		ThermStatus.CurLimit_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CROSS_DOMAIN) {
		ThermStatus.XDomLimit_Log = 0;
		ClearBit = 1;
	}

	RDMSR(ThermInterrupt, MSR_IA32_THERM_INTERRUPT);

	if (ThermalPoint_Count < THM_POINTS_DIM)
	{
		unsigned short WrRdMSR = 0;

	    if ((ThermalPoint_Count > THM_THRESHOLD_1)
	     && (ThermalPoint[THM_THRESHOLD_1] <= TjMax))
	    {
		if (ThermalPoint[THM_THRESHOLD_1] > 0)
		{
			COMPUTE_THERMAL(INVERSE_INTEL,
					ThermInterrupt.Threshold1_Value,
					Core->PowerThermal.Param,
					ThermalPoint[THM_THRESHOLD_1]);

			ThermInterrupt.Threshold1_Int = 1;
			WrRdMSR = 1;
		}
		else if (ThermalPoint[THM_THRESHOLD_1] == 0)
		{
			ThermInterrupt.Threshold1_Value = 0;
			ThermInterrupt.Threshold1_Int = 0;
			WrRdMSR = 1;
		}
	    }
	    if ((ThermalPoint_Count > THM_THRESHOLD_2)
	     && (ThermalPoint[THM_THRESHOLD_2] <= TjMax))
	    {
		if (ThermalPoint[THM_THRESHOLD_2] > 0)
		{
			COMPUTE_THERMAL(INVERSE_INTEL,
					ThermInterrupt.Threshold2_Value,
					Core->PowerThermal.Param,
					ThermalPoint[THM_THRESHOLD_2]);

			ThermInterrupt.Threshold2_Int = 1;
			WrRdMSR = 1;
		}
		else if (ThermalPoint[THM_THRESHOLD_2] == 0)
		{
			ThermInterrupt.Threshold2_Value = 0;
			ThermInterrupt.Threshold2_Int = 0;
			WrRdMSR = 1;
		}
	    }
	    if (WrRdMSR == 1) {
		WRMSR(ThermInterrupt, MSR_IA32_THERM_INTERRUPT);
		RDMSR(ThermInterrupt, MSR_IA32_THERM_INTERRUPT);
	    }
	}
	COMPUTE_THERMAL(INTEL,
			Core->ThermalPoint.Value[THM_THRESHOLD_1],
			Core->PowerThermal.Param,
			ThermInterrupt.Threshold1_Value);

	COMPUTE_THERMAL(INTEL,
			Core->ThermalPoint.Value[THM_THRESHOLD_2],
			Core->PowerThermal.Param,
			ThermInterrupt.Threshold2_Value);

	if (ThermInterrupt.Threshold1_Int) {
		BITSET(LOCKLESS, Core->ThermalPoint.State, THM_THRESHOLD_1);
	} else {
		BITCLR(LOCKLESS, Core->ThermalPoint.State, THM_THRESHOLD_1);
	}
	BITSET(LOCKLESS, Core->ThermalPoint.Mask, THM_THRESHOLD_1);
	BITCLR(LOCKLESS, Core->ThermalPoint.Kind, THM_THRESHOLD_1);

	if (ThermInterrupt.Threshold2_Int) {
		BITSET(LOCKLESS, Core->ThermalPoint.State, THM_THRESHOLD_2);
	} else {
		BITCLR(LOCKLESS, Core->ThermalPoint.State, THM_THRESHOLD_2);
	}
	BITSET(LOCKLESS, Core->ThermalPoint.Mask, THM_THRESHOLD_2);
	BITCLR(LOCKLESS, Core->ThermalPoint.Kind, THM_THRESHOLD_2);

	if (ClearBit)
	{
		if (!((ThermInterrupt.High_Temp_Int|ThermInterrupt.Low_Temp_Int)
			&& PUBLIC(RO(Proc))->Features.Power.EAX.HWFB_Cap))
		{
			WRMSR(ThermStatus, MSR_IA32_THERM_STATUS);
			RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);
		}
	}
	Core->PowerThermal.Events[eLOG] = \
		  ((Bit64) ThermStatus.Thermal_Log	<< LSHIFT_THERMAL_LOG)
		| ((Bit64) ThermStatus.PROCHOT_Log	<< LSHIFT_PROCHOT_LOG)
		| ((Bit64) ThermStatus.CriticalTemp_Log << LSHIFT_CRITIC_LOG)
		| ((Bit64) ThermStatus.Threshold1_Log	<< LSHIFT_THOLD1_LOG)
		| ((Bit64) ThermStatus.Threshold2_Log	<< LSHIFT_THOLD2_LOG)
		| ((Bit64) ThermStatus.PwrLimit_Log	<< LSHIFT_POWER_LIMIT)
		| ((Bit64) ThermStatus.CurLimit_Log	<< LSHIFT_CURRENT_LIMIT)
		| ((Bit64) ThermStatus.XDomLimit_Log	<< LSHIFT_CROSS_DOMAIN);

	Core->PowerThermal.Events[eSTS] = \
		  ((Bit64) ThermStatus.Thermal_Status	<< LSHIFT_THERMAL_STS)
		| ((Bit64) ThermStatus.PROCHOT_Event	<< LSHIFT_PROCHOT_STS)
		| ((Bit64) ThermStatus.CriticalTemp	<< LSHIFT_CRITIC_TMP)
		| ((Bit64) ThermStatus.Threshold1	<< LSHIFT_THOLD1_STS)
		| ((Bit64) ThermStatus.Threshold2	<< LSHIFT_THOLD2_STS);

      if (PUBLIC(RO(Proc))->Features.Power.EAX.PTM
      && (Core->Bind == PUBLIC(RO(Proc))->Service.Core))
      {
	ClearBit = 0;
	ThermStatus.value = 0;
	RDMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);

	if (Clear_Events & EVENT_THERMAL_LOG) {
		ThermStatus.Thermal_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_PROCHOT_LOG) {
		ThermStatus.PROCHOT_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CRITIC_LOG) {
		ThermStatus.CriticalTemp_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THOLD1_LOG) {
		ThermStatus.Threshold1_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_THOLD2_LOG) {
		ThermStatus.Threshold2_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_POWER_LIMIT) {
		ThermStatus.PwrLimit_Log = 0;
		ClearBit = 1;
	}

	RDMSR(ThermInterrupt, MSR_IA32_PACKAGE_THERM_INTERRUPT);

	if (PkgThermalPoint_Count < THM_POINTS_DIM)
	{
		unsigned short WrRdMSR = 0;

	    if ((PkgThermalPoint_Count > THM_THRESHOLD_1)
	     && (PkgThermalPoint[THM_THRESHOLD_1] <= TjMax))
	    {
		if (PkgThermalPoint[THM_THRESHOLD_1] > 0)
		{
			COMPUTE_THERMAL(INVERSE_INTEL,
					ThermInterrupt.Threshold1_Value,
					Core->PowerThermal.Param,
					PkgThermalPoint[THM_THRESHOLD_1]);

			ThermInterrupt.Threshold1_Int = 1;
			WrRdMSR = 1;
		}
		else if (PkgThermalPoint[THM_THRESHOLD_1] == 0)
		{
			ThermInterrupt.Threshold1_Value = 0;
			ThermInterrupt.Threshold1_Int = 0;
			WrRdMSR = 1;
		}
	    }
	    if ((PkgThermalPoint_Count > THM_THRESHOLD_2)
	     && (PkgThermalPoint[THM_THRESHOLD_2] <= TjMax))
	    {
		if (PkgThermalPoint[THM_THRESHOLD_2] > 0)
		{
			COMPUTE_THERMAL(INVERSE_INTEL,
					ThermInterrupt.Threshold2_Value,
					Core->PowerThermal.Param,
					PkgThermalPoint[THM_THRESHOLD_2]);

			ThermInterrupt.Threshold2_Int = 1;
			WrRdMSR = 1;
		}
		else if (PkgThermalPoint[THM_THRESHOLD_2] == 0)
		{
			ThermInterrupt.Threshold2_Value = 0;
			ThermInterrupt.Threshold2_Int = 0;
			WrRdMSR = 1;
		}
	    }
	    if (WrRdMSR == 1) {
		WRMSR(ThermInterrupt, MSR_IA32_PACKAGE_THERM_INTERRUPT);
		RDMSR(ThermInterrupt, MSR_IA32_PACKAGE_THERM_INTERRUPT);
	    }
	}

	COMPUTE_THERMAL(INTEL,
			PUBLIC(RO(Proc))->ThermalPoint.Value[THM_THRESHOLD_1],
			Core->PowerThermal.Param,
			ThermInterrupt.Threshold1_Value);

	COMPUTE_THERMAL(INTEL,
			PUBLIC(RO(Proc))->ThermalPoint.Value[THM_THRESHOLD_2],
			Core->PowerThermal.Param,
			ThermInterrupt.Threshold2_Value);

	if (ThermInterrupt.Threshold1_Int) {
		BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_THRESHOLD_1);
	} else {
		BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_THRESHOLD_1);
	}
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Mask, THM_THRESHOLD_1);
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Kind, THM_THRESHOLD_1);

	if (ThermInterrupt.Threshold2_Int) {
		BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_THRESHOLD_2);
	} else {
		BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_THRESHOLD_2);
	}
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Mask, THM_THRESHOLD_2);
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Kind, THM_THRESHOLD_2);

	if (ClearBit)
	{
		if (!((ThermInterrupt.High_Temp_Int|ThermInterrupt.Low_Temp_Int)
			&& PUBLIC(RO(Proc))->Features.Power.EAX.HWFB_Cap))
		{
			WRMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);
			RDMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);
		}
	}
	PUBLIC(RO(Proc))->PowerThermal.Events[eLOG] = \
		  ((Bit64) ThermStatus.Thermal_Log	<< LSHIFT_THERMAL_LOG)
		| ((Bit64) ThermStatus.PROCHOT_Log	<< LSHIFT_PROCHOT_LOG)
		| ((Bit64) ThermStatus.CriticalTemp_Log << LSHIFT_CRITIC_LOG)
		| ((Bit64) ThermStatus.Threshold1_Log	<< LSHIFT_THOLD1_LOG)
		| ((Bit64) ThermStatus.Threshold2_Log	<< LSHIFT_THOLD2_LOG)
		| ((Bit64) ThermStatus.PwrLimit_Log	<< LSHIFT_POWER_LIMIT);

	PUBLIC(RO(Proc))->PowerThermal.Events[eSTS] = \
		  ((Bit64) ThermStatus.Thermal_Status	<< LSHIFT_THERMAL_STS)
		| ((Bit64) ThermStatus.PROCHOT_Event	<< LSHIFT_PROCHOT_STS)
		| ((Bit64) ThermStatus.CriticalTemp	<< LSHIFT_CRITIC_TMP)
		| ((Bit64) ThermStatus.Threshold1	<< LSHIFT_THOLD1_STS)
		| ((Bit64) ThermStatus.Threshold2	<< LSHIFT_THOLD2_STS);
      }
    }
}

static void ThermalMonitor_Atom_Bonnell(CORE_RO *Core)
{
	Core->PowerThermal.Param.Offset[THERMAL_TARGET] = 100;
	Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = 0;

	ThermalMonitor_IA32(Core);
}

static void ThermalMonitor_Set(CORE_RO *Core)
{
	TJMAX TjMax = {.value = 0};
	PLATFORM_INFO PfInfo = {.value = 0};

	/* Silvermont + Xeon[06_57] + Nehalem + Sandy Bridge & superior arch. */
	RDMSR(TjMax, MSR_IA32_TEMPERATURE_TARGET);

	Core->PowerThermal.Param.Offset[THERMAL_TARGET] = TjMax.Target;
    if (Core->PowerThermal.Param.Offset[THERMAL_TARGET] == 0)
    {
	Core->PowerThermal.Param.Offset[THERMAL_TARGET] = 100;
    }

	RDMSR(PfInfo, MSR_PLATFORM_INFO);

    if (PfInfo.ProgrammableTj)
    {
      switch (PUBLIC(RO(Proc))->ArchID) {
      case Atom_Goldmont:
      case Xeon_Phi:	/* case 06_85h: */
	Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = TjMax.Atom.Offset;
	break;
      case IvyBridge_EP:
      case Broadwell_EP:
      case Broadwell_D:
      case Skylake_X:
	Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = TjMax.EP.Offset;
	break;
      default:
	Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = TjMax.Offset;
	break;
      case Core_Yonah ... Core_Dunnington:
	break;
      }
    }

	ThermalMonitor_IA32(Core);
}

static void CorePerfLimitReasons(CORE_RO *Core)
{
    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	CORE_PERF_LIMIT_REASONS limit = {.value = 0};
	unsigned short ClearBit = 0;
	RDMSR(limit, MSR_SKL_CORE_PERF_LIMIT_REASONS);

	if (Clear_Events & EVENT_CORE_HOT_LOG) {
		limit.PROCHOT_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_THM_LOG) {
		limit.Thermal_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_RES_LOG) {
		limit.Residency_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_AVG_LOG) {
		limit.AvgThmLimitLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_VRT_LOG) {
		limit.VR_ThmAlertLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_TDC_LOG) {
		limit.VR_TDC_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_PL1_LOG) {
		limit.PL1_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_PL2_LOG) {
		limit.PL2_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_EDP_LOG) {
		limit.EDP_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_BST_LOG) {
		limit.TurboLimitLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_ATT_LOG) {
		limit.TurboAttenLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_CORE_TVB_LOG) {
		limit.TVB_Log = 0;
		ClearBit = 1;
	}
	if (ClearBit)
	{
		WRMSR(limit, MSR_SKL_CORE_PERF_LIMIT_REASONS);
		RDMSR(limit, MSR_SKL_CORE_PERF_LIMIT_REASONS);
	}
	PUBLIC(RO(Proc))->PowerThermal.Events[eLOG] |= (
		  ((Bit64) limit.PROCHOT_Log	<< LSHIFT_CORE_HOT_LOG)
		| ((Bit64) limit.Thermal_Log	<< LSHIFT_CORE_THM_LOG)
		| ((Bit64) limit.Residency_Log	<< LSHIFT_CORE_RES_LOG)
		| ((Bit64) limit.AvgThmLimitLog << LSHIFT_CORE_AVG_LOG)
		| ((Bit64) limit.VR_ThmAlertLog << LSHIFT_CORE_VRT_LOG)
		| ((Bit64) limit.VR_TDC_Log	<< LSHIFT_CORE_TDC_LOG)
		| ((Bit64) limit.PL1_Log	<< LSHIFT_CORE_PL1_LOG)
		| ((Bit64) limit.PL2_Log	<< LSHIFT_CORE_PL2_LOG)
		| ((Bit64) limit.EDP_Log	<< LSHIFT_CORE_EDP_LOG)
		| ((Bit64) limit.TurboLimitLog	<< LSHIFT_CORE_BST_LOG)
		| ((Bit64) limit.TurboAttenLog	<< LSHIFT_CORE_ATT_LOG)
		| ((Bit64) limit.TVB_Log	<< LSHIFT_CORE_TVB_LOG)
	);
	PUBLIC(RO(Proc))->PowerThermal.Events[eSTS] |= (
		  ((Bit64) limit.Thermal_Status << LSHIFT_CORE_THM_STS)
		| ((Bit64) limit.PROCHOT_Event	<< LSHIFT_CORE_HOT_STS)
		| ((Bit64) limit.Residency_Sts	<< LSHIFT_CORE_RES_STS)
		| ((Bit64) limit.AvgThmLimit	<< LSHIFT_CORE_AVG_STS)
		| ((Bit64) limit.VR_ThmAlert	<< LSHIFT_CORE_VRT_STS)
		| ((Bit64) limit.VR_TDC_Status	<< LSHIFT_CORE_TDC_STS)
		| ((Bit64) limit.PL1_Status	<< LSHIFT_CORE_PL1_STS)
		| ((Bit64) limit.PL2_Status	<< LSHIFT_CORE_PL2_STS)
		| ((Bit64) limit.EDP_Status	<< LSHIFT_CORE_EDP_STS)
		| ((Bit64) limit.TurboLimit	<< LSHIFT_CORE_BST_STS)
		| ((Bit64) limit.TurboAtten	<< LSHIFT_CORE_ATT_STS)
		| ((Bit64) limit.TVB_Status	<< LSHIFT_CORE_TVB_STS)
	);
    }
}

static void GraphicsPerfLimitReasons(CORE_RO *Core)
{
    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	GRAPHICS_PERF_LIMIT_REASONS limit = {.value = 0};
	unsigned short ClearBit = 0;
	RDMSR(limit, MSR_GRAPHICS_PERF_LIMIT_REASONS);

	if (Clear_Events & EVENT_GFX_HOT_LOG) {
		limit.PROCHOT_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_THM_LOG) {
		limit.Thermal_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_AVG_LOG) {
		limit.AvgThmLimitLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_VRT_LOG) {
		limit.VR_ThmAlertLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_TDC_LOG) {
		limit.VR_TDC_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_PL1_LOG) {
		limit.PL1_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_PL2_LOG) {
		limit.PL2_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_EDP_LOG) {
		limit.EDP_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_GFX_EFF_LOG) {
		limit.InefficiencyLog = 0;
		ClearBit = 1;
	}
	if (ClearBit)
	{
		WRMSR(limit, MSR_GRAPHICS_PERF_LIMIT_REASONS);
		RDMSR(limit, MSR_GRAPHICS_PERF_LIMIT_REASONS);
	}
	PUBLIC(RO(Proc))->PowerThermal.Events[eLOG] |= (
		  ((Bit64) limit.PROCHOT_Log	<< LSHIFT_GFX_HOT_LOG)
		| ((Bit64) limit.Thermal_Log	<< LSHIFT_GFX_THM_LOG)
		| ((Bit64) limit.AvgThmLimitLog << LSHIFT_GFX_AVG_LOG)
		| ((Bit64) limit.VR_ThmAlertLog << LSHIFT_GFX_VRT_LOG)
		| ((Bit64) limit.VR_TDC_Log	<< LSHIFT_GFX_TDC_LOG)
		| ((Bit64) limit.PL1_Log	<< LSHIFT_GFX_PL1_LOG)
		| ((Bit64) limit.PL2_Log	<< LSHIFT_GFX_PL2_LOG)
		| ((Bit64) limit.EDP_Log	<< LSHIFT_GFX_EDP_LOG)
		| ((Bit64) limit.InefficiencyLog<< LSHIFT_GFX_EFF_LOG)
	);
	PUBLIC(RO(Proc))->PowerThermal.Events[eSTS] |= (
		  ((Bit64) limit.Thermal_Status << LSHIFT_GFX_THM_STS)
		| ((Bit64) limit.PROCHOT_Event	<< LSHIFT_GFX_HOT_STS)
		| ((Bit64) limit.AvgThmLimit	<< LSHIFT_GFX_AVG_STS)
		| ((Bit64) limit.VR_ThmAlert	<< LSHIFT_GFX_VRT_STS)
		| ((Bit64) limit.VR_TDC_Status	<< LSHIFT_GFX_TDC_STS)
		| ((Bit64) limit.PL1_Status	<< LSHIFT_GFX_PL1_STS)
		| ((Bit64) limit.PL2_Status	<< LSHIFT_GFX_PL2_STS)
		| ((Bit64) limit.EDP_Status	<< LSHIFT_GFX_EDP_STS)
		| ((Bit64) limit.Inefficiency	<< LSHIFT_GFX_EFF_STS)
	);
    }
}

static void RingPerfLimitReasons(CORE_RO *Core)
{
    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	RING_PERF_LIMIT_REASONS limit = {.value = 0};
	unsigned short ClearBit = 0;
	RDMSR(limit, MSR_RING_PERF_LIMIT_REASONS);

	if (Clear_Events & EVENT_RING_HOT_LOG) {
		limit.PROCHOT_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_THM_LOG) {
		limit.Thermal_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_AVG_LOG) {
		limit.AvgThmLimitLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_VRT_LOG) {
		limit.VR_ThmAlertLog = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_TDC_LOG) {
		limit.VR_TDC_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_PL1_LOG) {
		limit.PL1_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_PL2_LOG) {
		limit.PL2_Log = 0;
		ClearBit = 1;
	}
	if (Clear_Events & EVENT_RING_EDP_LOG) {
		limit.EDP_Log = 0;
		ClearBit = 1;
	}
	if (ClearBit)
	{
		WRMSR(limit, MSR_RING_PERF_LIMIT_REASONS);
		RDMSR(limit, MSR_RING_PERF_LIMIT_REASONS);
	}
	PUBLIC(RO(Proc))->PowerThermal.Events[eLOG] |= (
		  ((Bit64) limit.PROCHOT_Log	<< LSHIFT_RING_HOT_LOG)
		| ((Bit64) limit.Thermal_Log	<< LSHIFT_RING_THM_LOG)
		| ((Bit64) limit.AvgThmLimitLog << LSHIFT_RING_AVG_LOG)
		| ((Bit64) limit.VR_ThmAlertLog << LSHIFT_RING_VRT_LOG)
		| ((Bit64) limit.VR_TDC_Log	<< LSHIFT_RING_TDC_LOG)
		| ((Bit64) limit.PL1_Log	<< LSHIFT_RING_PL1_LOG)
		| ((Bit64) limit.PL2_Log	<< LSHIFT_RING_PL2_LOG)
		| ((Bit64) limit.EDP_Log	<< LSHIFT_RING_EDP_LOG)
	);
	PUBLIC(RO(Proc))->PowerThermal.Events[eSTS] |= (
		  ((Bit64) limit.Thermal_Status << LSHIFT_RING_THM_STS)
		| ((Bit64) limit.PROCHOT_Event	<< LSHIFT_RING_HOT_STS)
		| ((Bit64) limit.AvgThmLimit	<< LSHIFT_RING_AVG_STS)
		| ((Bit64) limit.VR_ThmAlert	<< LSHIFT_RING_VRT_STS)
		| ((Bit64) limit.VR_TDC_Status	<< LSHIFT_RING_TDC_STS)
		| ((Bit64) limit.PL1_Status	<< LSHIFT_RING_PL1_STS)
		| ((Bit64) limit.PL2_Status	<< LSHIFT_RING_PL2_STS)
		| ((Bit64) limit.EDP_Status	<< LSHIFT_RING_EDP_STS)
	);
    }
}

static void PowerThermal(CORE_RO *Core)
{
static	struct {
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

		{_Atom_Bonnell, 	0, 1, 1, 0},	/* 06_1C */
		{_Atom_Silvermont,	0, 1, 1, 0},	/* 06_26 */
		{_Atom_Lincroft,	0, 1, 1, 0},	/* 06_27 */
		{_Atom_Clover_Trail,	0, 1, 1, 0},	/* 06_35 */
		{_Atom_Saltwell,	0, 1, 1, 0},	/* 06_36 */

		{_Silvermont_Bay_Trail, 0, 1, 0, 0},	/* 06_37 */

		{_Atom_Avoton,		0, 1, 1, 0},	/* 06_4D */
		{_Atom_Airmont, 	0, 1, 0, 0},	/* 06_4C */
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

		{_IvyBridge,		1, 1, 0, 0},	/* 06_3A */
		{_IvyBridge_EP, 	1, 1, 0, 0},	/* 06_3E */

		{_Haswell_DT,		1, 1, 0, 0},	/* 06_3C */
		{_Haswell_EP,		1, 1, 1, 0},	/* 06_3F */
		{_Haswell_ULT,		1, 1, 0, 0},	/* 06_45 */
		{_Haswell_ULX,		1, 1, 1, 0},	/* 06_46 */

		{_Broadwell,		1, 1, 1, 0},	/* 06_3D */
		{_Broadwell_D,		1, 1, 1, 0},	/* 06_56 */
		{_Broadwell_H,		1, 1, 1, 0},	/* 06_47 */
		{_Broadwell_EP, 	1, 1, 1, 0},	/* 06_4F */

		{_Skylake_UY,		1, 1, 1, 0},	/* 06_4E */
		{_Skylake_S,		1, 0, 0, 0},	/* 06_5E */
		{_Skylake_X,		1, 1, 1, 0},	/* 06_55 */

		{_Xeon_Phi,		0, 1, 1, 0},	/* 06_57 */

		{_Kabylake,		1, 1, 0, 0},	/* 06_9E */
		{_Kabylake_UY,		1, 1, 1, 0},	/* 06_8E */

		{_Cannonlake_U, 	1, 1, 1, 0},	/* 06_66 */
		{_Cannonlake_H, 	1, 1, 1, 0},
		{_Geminilake,		1, 0, 1, 0},	/* 06_7A */
		{_Icelake_UY,		1, 1, 1, 0},	/* 06_7E */

		{_Icelake_X,		1, 1, 1, 0},
		{_Icelake_D,		1, 1, 1, 0},
		{_Sunny_Cove,		1, 1, 1, 0},
		{_Tigerlake,		1, 1, 1, 0},
		{_Tigerlake_U,		1, 1, 0, 0},	/* 06_8C */
		{_Cometlake,		1, 1, 1, 0},
		{_Cometlake_UY, 	1, 1, 1, 0},
		{_Atom_Denverton,	1, 1, 1, 0},
		{_Tremont_Jacobsville,	1, 1, 1, 0},
		{_Tremont_Lakefield,	1, 1, 1, 0},
		{_Tremont_Elkhartlake,	1, 1, 1, 0},
		{_Tremont_Jasperlake,	1, 1, 1, 0},
		{_Sapphire_Rapids,	1, 1, 1, 0},
		{_Emerald_Rapids,	1, 1, 1, 0},
		{_Granite_Rapids_X,	1, 1, 1, 0},
		{_Granite_Rapids_D,	1, 1, 1, 0},
		{_Sierra_Forest,	1, 1, 1, 0},
		{_Grand_Ridge,		1, 1, 1, 0},
		{_Rocketlake,		1, 1, 1, 0},
		{_Rocketlake_U, 	1, 1, 1, 0},
		{_Alderlake_S,		1, 1, 0, 0},	/* 06_97 */
		{_Alderlake_H,		1, 1, 0, 0},
		{_Alderlake_N,		1, 1, 0, 0},
		{_Meteorlake_M, 	1, 1, 1, 0},
		{_Meteorlake_N, 	1, 1, 1, 0},
		{_Meteorlake_S, 	1, 1, 1, 0},
		{_Raptorlake,		1, 1, 0, 0},	/* 06_B7 */
		{_Raptorlake_P, 	1, 1, 0, 0},
		{_Raptorlake_S, 	1, 1, 0, 0},
		{_Lunarlake,		1, 1, 1, 0},	/* 06_BD */
		{_Arrowlake,		1, 1, 1, 0},	/* 06_C6 */
		{_Arrowlake_H,		1, 1, 1, 0},	/* 06_C5 */
		{_Arrowlake_U,		1, 1, 1, 0},	/* 06_B5 */
		{_Pantherlake,		1, 1, 1, 0},	/* 06_CC */
		{_Clearwater_Forest,	1, 1, 1, 0}	/* 06_DD */
	};
	const unsigned int ids = sizeof(whiteList) / sizeof(whiteList[0]);
	unsigned int id;
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
		return 1;						\
	}								\
	return 0;							\
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

static void Control_IO_MWAIT(	struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core )
{
	CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
	RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*		SMT C-State Base Address.			*/
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

static void Control_CSTATES_NHM(struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core)
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

static void Control_CSTATES_COMMON(	struct CSTATES_ENCODING_ST Limit[],
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

static void Control_CSTATES_SNB(struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core)
{	/* Family: 06_2A, 06_3A, 06_3E, 06_3F, 06_4F, 06_56, 06_57, 06_85 */
	Control_CSTATES_COMMON(Limit, IORedir, Core, 0b0111, 0b1000);
}

static void Control_CSTATES_ULT(struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core)
{	/* Family: 06_3C, 06_3D, 06_45, 06_46, 06_47, 06_86		*/
	Control_CSTATES_COMMON(Limit, IORedir, Core, 0b1111, 0b0000);
}

static void Control_CSTATES_SKL(struct CSTATES_ENCODING_ST Limit[],
				struct CSTATES_ENCODING_ST IORedir[],
				CORE_RO *Core)
{	/* Family: 06_4E, 06_5E, 06_55, 06_66, 06_7D, 06_7E, 06_8E, 06_9E */
	Control_CSTATES_COMMON(Limit, IORedir, Core, 0b0111, 0b1000);
}

static void Control_CSTATES_SOC_ATOM(	struct CSTATES_ENCODING_ST Limit[],
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

static void Control_CSTATES_SOC_SLM(	struct CSTATES_ENCODING_ST Limit[],
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

static void Control_CSTATES_SOC_GDM(	struct CSTATES_ENCODING_ST Limit[],
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

static void PerCore_AMD_Family_0Fh_PStates(CORE_RO *Core)
{
	FIDVID_STATUS FidVidStatus = {.value = 0};
	FIDVID_CONTROL FidVidControl = {.value = 0};
	int NewFID = -1, NewVID = -1, loop = 100;
	UNUSED(Core);

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

static void SystemRegisters(CORE_RO *Core)
{
	unsigned long long mem64;

	__asm__ volatile
	(
		"xorq	%%rbx, %%rbx"		"\n\t"
		"xorq	%%rcx, %%rcx"		"\n\t"
		"xorq	%%rdx, %%rdx"		"\n\t"
		"# Carry, Sign, Overflow Flags" "\n\t"
		"xorq	%%rax, %%rax"		"\n\t"
		"cmpl	$-0x80000000, %%eax"	"\n\t"
		"# Zero Flag"			"\n\t"
		"movq	%%rax, %1"		"\n\t"
		"cmpxchg8b %1"			"\n\t"
		"# Direction Flag"		"\n\t"
		"std"				"\n\t"
		"# Interrupt Flag"		"\n\t"
		"sti"				"\n\t"
		"# Save all RFLAGS"		"\n\t"
		"pushfq"			"\n\t"
		"popq	%0"			"\n\t"
		"# Reset Flags"			"\n\t"
		"cli"				"\n\t"
		"cld"
		: "=r" (Core->SystemRegister.RFLAGS)
		: "m" (mem64)
		: "%rax", "%rbx", "%rcx", "%rdx", "cc"
	);
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
		"movq	%%cr0, %0"	"\n\t"
		"movq	%%cr3, %1"	"\n\t"
		"movq	%%cr4, %2"	"\n\t"
		"movq	%%cr8, %3"	"\n\t"
		"# EFER"		"\n\t"
		"xorq	%%rax, %%rax"	"\n\t"
		"xorq	%%rdx, %%rdx"	"\n\t"
		"movq	%5,%%rcx"	"\n\t"
		"rdmsr"			"\n\t"
		"shlq	$32, %%rdx"	"\n\t"
		"orq	%%rdx, %%rax"	"\n\t"
		"movq	%%rax, %4"
		: "=r" (Core->SystemRegister.CR0),
		  "=r" (Core->SystemRegister.CR3),
		  "=r" (Core->SystemRegister.CR4),
		  "=r" (Core->SystemRegister.CR8),
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
		/*		System Configuration Register.		*/
		__asm__ volatile
		(
			"# SYSCFG"		"\n\t"
			"xorq	%%rax, %%rax"	"\n\t"
			"xorq	%%rdx, %%rdx"	"\n\t"
			"movq	%1,%%rcx"	"\n\t"
			"rdmsr"			"\n\t"
			"shlq	$32, %%rdx"	"\n\t"
			"orq	%%rdx, %%rax"	"\n\t"
			"movq	%%rax, %0"
			: "=r" (Core->SystemRegister.SYSCFG)
			: "i" (MSR_AMD64_SYSCFG)
			: "%rax", "%rcx", "%rdx"
		);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CR_Mask, Core->Bind);

	if (PUBLIC(RO(Proc))->Features.Std.ECX.XSAVE
	 && BITVAL(Core->SystemRegister.CR4, CR4_OSXSAVE)) {
		__asm__ volatile
		(
			"# XCR0"		"\n\t"
			"xorq	%%rcx, %%rcx"	"\n\t"
			"xgetbv"		"\n\t"
			"shlq	$32, %%rdx"	"\n\t"
			"orq	%%rdx, %%rax"	"\n\t"
			"movq	%%rax, %0"
			: "=r" (Core->SystemRegister.XCR0)
			:
			: "%rax", "%rcx", "%rdx"
		);
	}
	if (PUBLIC(RO(Proc))->Features.ExtState_Leaf1.EAX.IA32_XSS) {
		__asm__ volatile
		(
			"# XSS" 		"\n\t"
			"xorq	%%rax, %%rax"	"\n\t"
			"xorq	%%rdx, %%rdx"	"\n\t"
			"movq	%1,%%rcx"	"\n\t"
			"rdmsr"			"\n\t"
			"shlq	$32, %%rdx"	"\n\t"
			"orq	%%rdx, %%rax"	"\n\t"
			"movq	%%rax, %0"
			: "=r" (Core->SystemRegister.XSS)
			: "i" (MSR_IA32_XSS)
			: "%rax", "%rcx", "%rdx"
		);
	}
}

static void Intel_Mitigation_Mechanisms(CORE_RO *Core)
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

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSFD, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_U, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_S, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_U, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_S, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DDPD_U_DIS, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_DIS_S, Core->Bind);

    if (PUBLIC(RO(Proc))->Features.ExtFeature.EAX.MaxSubLeaf >= 2)
    {
	if (PUBLIC(RO(Proc))->Features.ExtFeature_Leaf2_EDX.PSFD_SPEC_CTRL) {
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);

	    if (Spec_Ctrl.PSFD) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PSFD, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSFD, Core->Bind);
	    }
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature_Leaf2_EDX.IPRED_SPEC_CTRL) {
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);

	    if (Spec_Ctrl.IPRED_DIS_U) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_U, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_U, Core->Bind);
	    }
	    if (Spec_Ctrl.IPRED_DIS_S) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_S, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_S, Core->Bind);
	    }
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature_Leaf2_EDX.RRSBA_SPEC_CTRL) {
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);

	    if (Spec_Ctrl.RRSBA_DIS_U) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_U, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_U, Core->Bind);
	    }
	    if (Spec_Ctrl.RRSBA_DIS_S) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_S, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_S, Core->Bind);
	    }
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature_Leaf2_EDX.DDPD_U_SPEC_CTRL) {
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);

	    if (Spec_Ctrl.DDPD_U_DIS) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->DDPD_U_DIS, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DDPD_U_DIS, Core->Bind);
	    }
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature_Leaf2_EDX.BHI_SPEC_CTRL) {
		RDMSR(Spec_Ctrl, MSR_IA32_SPEC_CTRL);

	    if (Spec_Ctrl.BHI_DIS_S) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_DIS_S, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_DIS_S, Core->Bind);
	    }
	}
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
	if (Arch_Cap.DOITM_UARCH_MISC_CTRL) {
		UARCH_MISC_CTRL uARCH_Ctrl = {.value = 0};
		RDMSR(uARCH_Ctrl, MSR_IA32_UARCH_MISC_CTRL);

	    if (uARCH_Ctrl.DOITM) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->DOITM_EN, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DOITM_EN, Core->Bind);
	    }
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->DOITM_MSR, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DOITM_MSR, Core->Bind);
	}
	if (Arch_Cap.SBDR_SSDP_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SBDR_SSDP_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SBDR_SSDP_NO, Core->Bind);
	}
	if (Arch_Cap.FBSDP_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->FBSDP_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->FBSDP_NO, Core->Bind);
	}
	if (Arch_Cap.PSDP_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PSDP_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSDP_NO, Core->Bind);
	}
	if (Arch_Cap.FB_CLEAR) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->FB_CLEAR, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->FB_CLEAR, Core->Bind);
	}
	if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.SRBDS_CTRL) {
	  if (Arch_Cap.FB_CLEAR_CTRL) {
		MCU_OPT_CTRL MCU_Ctrl = {.value = 0};
		RDMSR(MCU_Ctrl, MSR_IA32_MCU_OPT_CTRL);

	    if (MCU_Ctrl._RNGDS_MITG_DIS) {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RNGDS, Core->Bind);
	    } else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RNGDS, Core->Bind);
	    }
	    if (MCU_Ctrl._RTM_ALLOW && !MCU_Ctrl._RTM_LOCKED) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RTM, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RTM, Core->Bind);
	    }
	    if (MCU_Ctrl._FB_CLEAR_DIS) {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->VERW, Core->Bind);
	    } else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->VERW, Core->Bind);
	    }
	  }
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SRBDS_MSR, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SRBDS_MSR, Core->Bind);
	}
	if (Arch_Cap.RRSBA) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA, Core->Bind);
	}
	if (Arch_Cap.BHI_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_NO, Core->Bind);
	}
	if (Arch_Cap.XAPIC_DISABLE_STATUS_MSR) {
		XAPIC_DISABLE_STATUS xAPIC_Status = {.value = 0};
		RDMSR(xAPIC_Status, MSR_IA32_XAPIC_DISABLE_STATUS);

	    if (xAPIC_Status.LEGACY_XAPIC_DIS) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->XAPIC_DIS, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XAPIC_DIS, Core->Bind);
	    }
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->XAPIC_MSR, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XAPIC_MSR, Core->Bind);
	}
	if (Arch_Cap.PBRSB_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PBRSB_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PBRSB_NO, Core->Bind);
	}
	if (Arch_Cap.OVERCLOCKING_STATUS_MSR) {
		OVERCLOCKING_STATUS OC_Status = {.value = 0};
		RDMSR(OC_Status, MSR_IA32_OVERCLOCKING_STATUS);

	    if (OC_Status.OC_Utilized) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UTILIZED, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UTILIZED, Core->Bind);
	    }
	    if (OC_Status.OC_Undervolt) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UNDERVOLT, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UNDERVOLT, Core->Bind);
	    }
	    if (OC_Status.OC_Unlocked) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UNLOCKED, Core->Bind);
	    } else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UNLOCKED, Core->Bind);
	    }
	}
	if (Arch_Cap.GDS_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->GDS_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->GDS_NO, Core->Bind);
	}
	if (Arch_Cap.RFDS_NO) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RFDS_NO, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RFDS_NO, Core->Bind);
	}
    }
    if (PUBLIC(RO(Proc))->Features.ExtFeature.EDX.IA32_CORE_CAP)
    {
	CORE_CAPABILITIES Core_Cap = {.value = 0};

	RDMSR(Core_Cap, MSR_IA32_CORE_CAPABILITIES);

	if (Core_Cap.STLB_SUPPORTED) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->STLB, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->STLB, Core->Bind);
	}
	if (Core_Cap.FUSA_SUPPORTED) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->FUSA, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->FUSA, Core->Bind);
	}
	if (Core_Cap.RSM_IN_CPL0_ONLY) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->RSM_CPL0, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RSM_CPL0, Core->Bind);
	}
	if (Core_Cap.SPLA_EXCEPTION) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA, Core->Bind);
	}
	if (Core_Cap.SNOOP_FILTER_SUP) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->SNOOP_FILTER, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SNOOP_FILTER, Core->Bind);
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

static void AMD_Mitigation_Mechanisms(CORE_RO *Core)
{
	AMD_SPEC_CTRL Spec_Ctrl = {.value = 0};
	AMD_PRED_CMD  Pred_Cmd  = {.value = 0};
	unsigned short WrRdMSR = 0;

    if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.IBRS
     || PUBLIC(RO(Proc))->Features.leaf80000008.EBX.STIBP
     || PUBLIC(RO(Proc))->Features.leaf80000008.EBX.SSBD
     || PUBLIC(RO(Proc))->Features.leaf80000008.EBX.PSFD)
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
	if ((Mech_PSFD == COREFREQ_TOGGLE_OFF)
	 || (Mech_PSFD == COREFREQ_TOGGLE_ON))
	{
	    if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.PSFD) {
		Spec_Ctrl.PSFD = Mech_PSFD;
		WrRdMSR = 1;
	    }
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
	if (Spec_Ctrl.PSFD) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->PSFD, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSFD, Core->Bind);
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
	if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.SBPB
	&& ((Mech_SBPB == COREFREQ_TOGGLE_OFF)
	 || (Mech_SBPB == COREFREQ_TOGGLE_ON)))
	{
	    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1))
	    {
		Pred_Cmd.SBPB = Mech_SBPB;
		WRMSR(Pred_Cmd, MSR_AMD_PRED_CMD);
	    }
	}
    if ((PUBLIC(RO(Proc))->Features.leaf80000008.EBX.SSBD_VirtSpecCtrl == 0)
     && (BITVAL_CC(PUBLIC(RW(Proc))->SSBD, Core->Bind) == 0))
    {
	AMD_LS_CFG LS_CFG = {.value = 0};
	CPUID_0x8000001e leaf8000001e = {
			.EAX = {0}, .EBX = {{0}}, .ECX = {0}, .EDX = {0}
	};
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
      if (leaf8000001e.EBX.ThreadsPerCore == 1)
      {
	RDMSR(LS_CFG, MSR_AMD64_LS_CFG);

	if ((Mech_SSBD == COREFREQ_TOGGLE_OFF)
	 || (Mech_SSBD == COREFREQ_TOGGLE_ON))
	{
		LS_CFG.F17h_SSBD_EN = Mech_SSBD;
		WRMSR(LS_CFG, MSR_AMD64_LS_CFG);
		RDMSR(LS_CFG, MSR_AMD64_LS_CFG);
	}
	if (LS_CFG.F17h_SSBD_EN == 1) {
	    BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->AMD_LS_CFG_SSBD, Core->Bind);
	} else {
	    BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->AMD_LS_CFG_SSBD, Core->Bind);
	}
      } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->AMD_LS_CFG_SSBD, Core->Bind);
      }
    } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->AMD_LS_CFG_SSBD, Core->Bind);
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->SPEC_CTRL_Mask, Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->ARCH_CAP_Mask, Core->Bind);

    switch (PUBLIC(RO(Proc))->ArchID) {
    case AMD_EPYC_Rome_CPK:
    case AMD_Zen2_Renoir:
    case AMD_Zen2_LCN:
    case AMD_Zen2_MTS:
    case AMD_Zen2_Ariel:
    case AMD_Zen2_Jupiter:
    case AMD_Zen2_Galileo:
    case AMD_Zen2_MDN:
      {
	AMD_LS_CFG LS_CFG = {.value = 0};
	AMD_DE_CFG2 DE_CFG = {.value = 0};

	RDMSR(DE_CFG, MSR_AMD_DE_CFG2);

	if (((Mech_BTC_NOBR == COREFREQ_TOGGLE_OFF)
	  || (Mech_BTC_NOBR == COREFREQ_TOGGLE_ON))
	  && (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.STIBP == 1))
	{
		DE_CFG.SuppressBPOnNonBr = Mech_BTC_NOBR;
		WRMSR(DE_CFG, MSR_AMD_DE_CFG2);
		RDMSR(DE_CFG, MSR_AMD_DE_CFG2);
	}
	if (DE_CFG.SuppressBPOnNonBr) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->BTC_NOBR, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BTC_NOBR, Core->Bind);
	}

	RDMSR(LS_CFG, MSR_AMD64_LS_CFG);

	if ( (Mech_AGENPICK == COREFREQ_TOGGLE_OFF)
	  || (Mech_AGENPICK == COREFREQ_TOGGLE_ON) )
	{
		LS_CFG.F17h_AgenPick = Mech_AGENPICK;
		WRMSR(LS_CFG, MSR_AMD64_LS_CFG);
		RDMSR(LS_CFG, MSR_AMD64_LS_CFG);
	}
	if (LS_CFG.F17h_AgenPick == 1) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->AGENPICK, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->AGENPICK, Core->Bind);
	}
      }
	break;
    default:
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BTC_NOBR, Core->Bind);
	break;
    }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask, Core->Bind);

    if ((Core->T.ThreadID == 0) || (Core->T.ThreadID == -1)) {
      switch (PUBLIC(RO(Proc))->ArchID) {
      case AMD_EPYC_Rome_CPK:
      case AMD_Zen2_Renoir:
      case AMD_Zen2_LCN:
      case AMD_Zen2_MTS:
      case AMD_Zen2_Ariel:
      case AMD_Zen2_Jupiter:
      case AMD_Zen2_Galileo:
      case AMD_Zen2_MDN:
       {
	AMD_DE_CFG DE_CFG = {.value = 0};
	RDMSR(DE_CFG, MSR_AMD64_DE_CFG);

	if ((Mech_XPROC_LEAK == COREFREQ_TOGGLE_OFF)
	 || (Mech_XPROC_LEAK == COREFREQ_TOGGLE_ON))
	{
		DE_CFG.Cross_Proc_Leak = Mech_XPROC_LEAK;
		WRMSR(DE_CFG, MSR_AMD64_DE_CFG);
		RDMSR(DE_CFG, MSR_AMD64_DE_CFG);
	}
	if (DE_CFG.Cross_Proc_Leak) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->XPROC_LEAK, Core->Bind);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XPROC_LEAK, Core->Bind);
	}
       }
	break;
      default:
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XPROC_LEAK, Core->Bind);
	break;
      }
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->XPROC_LEAK_Mask, Core->Bind);
    } else {
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->XPROC_LEAK_Mask, Core->Bind);
    }
}

static void Intel_VirtualMachine(CORE_RO *Core)
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

static void Intel_Microcode(CORE_RO *Core)
{
	MICROCODE_ID Microcode = {.value = 0};

	RDMSR(Microcode, MSR_IA32_UCODE_REV);
	Core->Query.Microcode = Microcode.Signature;
}

static void AMD_Microcode(CORE_RO *Core)
{
	unsigned long long value = 0;
	RDMSR64(value, MSR_AMD64_PATCH_LEVEL);
	Core->Query.Microcode = (unsigned int) value;
}

#define Pkg_Reset_ThermalPoint(Pkg)					\
({									\
	BITWISECLR(LOCKLESS, Pkg->ThermalPoint.Mask);			\
	BITWISECLR(LOCKLESS, Pkg->ThermalPoint.Kind);			\
	BITWISECLR(LOCKLESS, Pkg->ThermalPoint.State);			\
})

static void PerCore_Reset(CORE_RO *Core)
{
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->ODCM_Mask , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->DCU_Mask  , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->L1_Scrub_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->L2_AMP_Mask, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->ECORE_Mask, Core->Bind);
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
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->XPROC_LEAK_Mask,Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->CR_Mask	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask	, Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM1	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->ODCM	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_Prefetch	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_HW_IP_Prefetch , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_NPP_Prefetch	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Scrubbing	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_Prefetch	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_HW_CL_Prefetch , Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_AMP_Prefetch	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Stride_Pf	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Region_Pf	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1_Burst_Pf	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_Stream_HW_Pf	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L2_UpDown_Pf	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->LLC_Streamer	, Core->Bind);
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
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSFD	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RDCL_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IBRS_ALL	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RSBA	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->L1DFL_VMENTRY_NO,Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SSB_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->MDS_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSCHANGE_MC_NO, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TAA_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->STLB	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->FUSA	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RSM_CPL0	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SPLA	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SNOOP_FILTER, Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SMM	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->VM	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->WDT	, Core->Bind);

	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->AMD_LS_CFG_SSBD, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DOITM_EN	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DOITM_MSR	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SBDR_SSDP_NO, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->FBSDP_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PSDP_NO 	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->FB_CLEAR	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->SRBDS_MSR	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RNGDS	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RTM	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->VERW	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XAPIC_MSR	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XAPIC_DIS	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->PBRSB_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_U, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->IPRED_DIS_S, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_U, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RRSBA_DIS_S, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->DDPD_U_DIS, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BHI_DIS_S	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->BTC_NOBR	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->XPROC_LEAK, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UTILIZED, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UNDERVOLT, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->OC_UNLOCKED, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->GDS_NO	, Core->Bind);
	BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->RFDS_NO	, Core->Bind);

	BITWISECLR(LOCKLESS, Core->ThermalPoint.Mask);
	BITWISECLR(LOCKLESS, Core->ThermalPoint.Kind);
	BITWISECLR(LOCKLESS, Core->ThermalPoint.State);
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

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask	, Core->Bind);
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->WDT_Mask,
				PUBLIC(RO(Proc))->Service.Core);
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->WDT_Mask,
				PUBLIC(RO(Proc))->Service.Core);

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

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask	, Core->Bind);
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->WDT_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);				/* Shared | Unique */

	ThermalMonitor_Set(Core);
}

static void PerCore_Atom_Bonnell_Query(void *arg)
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->WDT_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);				/* Shared | Unique */

	ThermalMonitor_Atom_Bonnell(Core);
}

static void Compute_Intel_Silvermont_Burst(CORE_RO *Core)
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

      if ((initialize == true) && (PUBLIC(RO(Proc))->ArchID != Atom_Airmont))
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

	Intel_CStatesConfiguration(CSTATES_SOC_SLM, Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->WDT_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	PowerThermal(Core);				/* Shared | Unique */

	ThermalMonitor_Set(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,	/* Table 2-8 */
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_Watchdog(Core);
	}
}

static void PerCore_Airmont_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Silvermont_Query(arg);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PP0_POWER_LIMIT,	/* Table 2-11 */
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(CORES) );
	}
}

static void PerCore_Atom_Goldmont_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	Intel_DCU_Technology(Core);

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
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,	/* Table 2-12 */
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_DomainPowerLimit( MSR_DRAM_POWER_LIMIT,	/* Table 2-12 */
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(RAM) );

	    if (PUBLIC(RO(Proc))->Registration.Experimental) {
		Intel_Pkg_CST_IRTL(MSR_PKGC3_IRTL,
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC03);

		Intel_Pkg_CST_IRTL(MSR_PKGC6_IRTL,
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC06);

		Intel_Pkg_CST_IRTL(MSR_PKGC7_IRTL,
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC07);
	    }
		Intel_Watchdog(Core);
	}
}

static void PerCore_Goldmont_Query(void *arg)
{
	PerCore_Atom_Goldmont_Query(arg);
}

static void PerCore_Geminilake_Query(void *arg)
{
	PerCore_Atom_Goldmont_Query(arg);
}

static void PerCore_Tremont_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Atom_Goldmont_Query(arg);

	Intel_Turbo_Activation_Ratio(Core);
}

static void PerCore_Nehalem_Same_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	Intel_DCU_Technology(Core);

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
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
}

static void PerCore_Nehalem_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);
	PerCore_Nehalem_Same_Query(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Nehalem_PowerLimit();		/* Table 2-15	*/

		Intel_Watchdog(Core);
	}
}

static void PerCore_Nehalem_EX_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Platform_Info(Core->Bind);
	Intel_Core_Platform_Info(Core->Bind);
	PerCore_Nehalem_Same_Query(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Nehalem_PowerLimit();		/* Table 2-15	*/

		Intel_Watchdog(Core);
	}
}

static void PerCore_Avoton_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Silvermont_Query(arg);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,	/* Table 2-10 */
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );
	}
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

	Intel_DCU_Technology(Core);

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
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_DomainPowerLimit( MSR_PP0_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(CORES) );

	    if (PUBLIC(RO(Proc))->Registration.Experimental) {
		Intel_Pkg_CST_IRTL(MSR_PKGC3_IRTL,
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC03);

		Intel_Pkg_CST_IRTL(MSR_PKGC6_IRTL,
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC06);

		Intel_Pkg_CST_IRTL(MSR_PKGC7_IRTL,
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC07);

		Intel_Watchdog(Core);
	    } else {
		BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
	    }
	}
}

static void PerCore_SandyBridge_EP_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_SandyBridge_Query(arg);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_DRAM_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(RAM) );
	}
}

static void PerCore_IvyBridge_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_SandyBridge_Query(arg);
	Intel_Turbo_Activation_Ratio( (CORE_RO*) arg );
	Intel_Turbo_TDP_Config( (CORE_RO*) arg );

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PP1_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(UNCORE) );
	}
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
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_SandyBridge_Query(arg);
	Intel_Turbo_Activation_Ratio(Core);
	Intel_Turbo_TDP_Config(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PP1_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(UNCORE) );
	}
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

	Intel_DCU_Technology(Core);

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
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);

	Intel_Turbo_Activation_Ratio(Core);
	Intel_Turbo_TDP_Config(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_DomainPowerLimit( MSR_PP0_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(CORES) );

		Intel_DomainPowerLimit( MSR_DRAM_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(RAM) );

	    if (PUBLIC(RO(Proc))->Registration.Experimental) {
		Intel_Pkg_CST_IRTL(MSR_PKGC6_IRTL,	/* Table 2-29	*/
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC06);

		Intel_Pkg_CST_IRTL(MSR_PKGC7_IRTL,	/* Table 2-29	*/
				&PUBLIC(RO(Proc))->PowerThermal.IRTL.PC07);
	    }
		Intel_Watchdog(Core);
	}
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

	Intel_DCU_Technology(Core);

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
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_DomainPowerLimit( MSR_PP0_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(CORES) );

		Intel_DomainPowerLimit( MSR_PP1_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(UNCORE) );

		Intel_Watchdog(Core);
	}
}

static void PerCore_Haswell_ULX(void *arg)
{
	PerCore_IvyBridge_Query(arg);
}

static void PerCore_Broadwell_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_SandyBridge_Query(arg);
	Intel_Turbo_Activation_Ratio(Core);
	Intel_Turbo_TDP_Config(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PP1_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(UNCORE) );
	}
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

	Intel_DCU_Technology(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_Skylake_Target,
				Core->Boost[BOOST(TDP)] > 0 ?
				Core->Boost[BOOST(TDP)]:Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SKL, Core);
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
	CorePerfLimitReasons(Core);
	GraphicsPerfLimitReasons(Core);
	RingPerfLimitReasons(Core);

	Intel_Turbo_Activation_Ratio(Core);
	Intel_Turbo_TDP_Config(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_DomainPowerLimit( MSR_PP0_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(CORES) );

		Intel_DomainPowerLimit( MSR_PP1_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(UNCORE) );

		Intel_DomainPowerLimit( MSR_PLATFORM_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PLATFORM) );

		Intel_Watchdog(Core);
	}
}

static void PerCore_Skylake_X_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Intel_Turbo_Config(	Core,
				Intel_Turbo_Cfg_SKL_X_PerCore,
				Assign_SKL_X_Boost );

	Intel_Platform_Info(Core->Bind);
	Intel_Turbo_Config(Core, Intel_Turbo_Cfg8C_PerCore, Assign_8C_Boost);

	SystemRegisters(Core);

	Intel_Mitigation_Mechanisms(Core);

	Intel_VirtualMachine(Core);

	Intel_Microcode(Core);

	Dump_CPUID(Core);

	Intel_DCU_Technology(Core);

	SpeedStep_Technology(Core);

	TurboBoost_Technology(	Core,
				Set_SandyBridge_Target,
				Get_SandyBridge_Target,
				Cmp_Skylake_Target,
				Core->Boost[BOOST(TDP)] > 0 ?
				Core->Boost[BOOST(TDP)]:Core->Boost[BOOST(1C)],
				Core->Boost[BOOST(MAX)] );

	Query_Intel_C1E(Core);

	if (Core->T.ThreadID == 0) {				/* Per Core */
		Intel_CStatesConfiguration(CSTATES_SKL, Core);
	} else {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
	/*	Store the C-State Base Address used by I/O-MWAIT	*/
		Core->Query.CStateBaseAddr = CState_IO_MWAIT.LVL2_BaseAddr;
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->CC6_Mask, Core->Bind);

	BITSET_CC(LOCKLESS,	PUBLIC(RO(Proc))->PC6_Mask,
				PUBLIC(RO(Proc))->Service.Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);

	PowerThermal(Core);

	ThermalMonitor_Set(Core);
	CorePerfLimitReasons(Core);
/*TODO(Unsolved)
	GraphicsPerfLimitReasons(Core);
*/
	RingPerfLimitReasons(Core);

	Intel_Turbo_Activation_Ratio(Core);
	Intel_Turbo_TDP_Config(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_PKG_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PKG) );

		Intel_DomainPowerLimit( MSR_PP0_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(CORES) );
/*TODO(Unsupported MSR and CSR registers)
		Intel_DomainPowerLimit( MSR_PLATFORM_POWER_LIMIT,
					PKG_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(PLATFORM) );

		Intel_DomainPowerLimit( MSR_DRAM_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(RAM) );
*/
	    if (PUBLIC(RO(Proc))->Registration.Experimental) {
		Intel_Watchdog(Core);
	    } else {
		BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
	    }
	}
}

static void PerCore_Kaby_Lake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Skylake_Query(arg);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		Intel_DomainPowerLimit( MSR_DRAM_POWER_LIMIT,
					PPn_POWER_LIMIT_LOCK_MASK,
					PWR_DOMAIN(RAM) );
	}
}

static void PerCore_Icelake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Skylake_Query(arg);

	Intel_Core_MicroArchControl(Core);
}

static void PerCore_Tigerlake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Kaby_Lake_Query(arg);

	Intel_Core_MicroArchControl(Core);
}

static void PerCore_Alderlake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Kaby_Lake_Query(arg);

	Intel_Core_MicroArchitecture(Core);
}

static void PerCore_Raptorlake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Skylake_Query(arg);

	Intel_Core_MicroArchitecture(Core);
}

static void PerCore_Meteorlake_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_Kaby_Lake_Query(arg);

	Intel_Core_MicroArchitecture(Core);
	Intel_Ultra7_MicroArchitecture(Core);
}

static void PerCore_AMD_Family_0Fh_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	PerCore_AMD_Family_0Fh_PStates(Core);		/* Alter P-States */
	Compute_AMD_Family_0Fh_Boost(Core->Bind);	/* Query P-States */

	SystemRegisters(Core);

	Dump_CPUID(Core);

	Query_AMD_Family_0Fh_C1E(Core);

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask	, Core->Bind);
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
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

	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask	, Core->Bind);
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
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->BTC_NOBR_Mask , Core->Bind);
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->WDT_Mask, Core->Bind);
}

static void PerCore_AMD_Family_10h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_10h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
	/*TODO(Errata 438) PerCore_AMD_CState_BAR(Core);		*/
	AMD_Watchdog(Core);
}

static void PerCore_AMD_Family_11h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_11h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
	/*TODO(Unspecified) PerCore_AMD_CState_BAR(Core);		*/
	/*TODO(Watchdog): D18F3x{40,44} MCA NB Registers - Hardware needed */
}

static void PerCore_AMD_Family_12h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_12h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
	PerCore_AMD_CState_BAR(Core);
	AMD_Watchdog(Core);
}

static void PerCore_AMD_Family_14h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_14h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
	PerCore_AMD_CState_BAR(Core);
	AMD_Watchdog(Core);
}

static void PerCore_AMD_Family_15h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_15h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
	PerCore_AMD_CState_BAR(Core);

	switch (PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel) {
	case 0x0:
	case 0x1:
	/*TODO(Watchdog): D18F3x{40,44} MCA NB Registers - Hardware needed */
		break;
	case 0x3:
	case 0x6:
	case 0x7:
		if ((PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
		 && (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf))
		{
			AMD_Watchdog(Core);
		}
		break;
	}
}

static void PerCore_AMD_Family_16h_Query(void *arg)
{
	CORE_RO *Core = (CORE_RO *) arg;

	Compute_AMD_Family_15h_Boost(Core->Bind);
	PerCore_AMD_Family_Same_Query(Core);
	PerCore_AMD_CState_BAR(Core);
	AMD_Watchdog(Core);
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
	/*	Query and set features per Package			*/
    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	TCTL_THERM_TRIP ThermTrip = {.value = 0};
	TCTL_HTC HTC = {.value = 0};
	unsigned int rx;
	HSMP_ARG arg[8];

	/*	Query the HTC and THERM_TRIP features from SMUTHM	*/
	Core_AMD_SMN_Read(	HTC,
				SMU_AMD_THM_TCTL_REGISTER_F17H + 0x4,
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->ThermalPoint.Value[THM_HTC_LIMIT] = HTC.HTC_TMP_LIMIT;
	PUBLIC(RO(Proc))->ThermalPoint.Value[THM_HTC_HYST] = HTC.HTC_HYST_LIMIT;

	if (HTC.HTC_EN) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2, Core->Bind);

		BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_HTC_LIMIT);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM2, Core->Bind);

		BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_HTC_LIMIT);
	}
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Mask, THM_HTC_LIMIT);
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Kind, THM_HTC_LIMIT);

	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State, THM_HTC_HYST);
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Mask, THM_HTC_HYST);
	BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Kind, THM_HTC_HYST);

	Core_AMD_SMN_Read(	ThermTrip,
				SMU_AMD_THM_TCTL_REGISTER_F17H + 0x8,
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->ThermalPoint.Value[THM_TRIP_LIMIT] = \
					ThermTrip.THERM_TP_LIMIT - 49;
	if (ThermTrip.THERM_TP_EN) {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->TM1, Core->Bind);

		BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_TRIP_LIMIT);
	} else {
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->TM1, Core->Bind);

		BITCLR(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.State,
				 THM_TRIP_LIMIT);
	}
	BITSET_CC(LOCKLESS, PUBLIC(RO(Proc))->TM_Mask, Core->Bind);
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Mask, THM_TRIP_LIMIT);
	BITSET(LOCKLESS, PUBLIC(RO(Proc))->ThermalPoint.Kind, THM_TRIP_LIMIT);
	/*		Count CPB and XFR ratios			*/
	if (CPB_State == true)
	{
		PUBLIC(RO(Proc))->Features.XtraCOF = 2;
	}
	else {
		PUBLIC(RO(Proc))->Features.XtraCOF = 0;
	}
	#define _lt (PWR_LIMIT_SIZE * PWR_DOMAIN(PKG))
	if (Custom_TDP_Count > _lt) {
	    if (Custom_TDP_Offset[_lt] != 0)
	    {
		signed int TDP_Limit;
		TDP_Limit = PUBLIC(RO(Proc))->PowerThermal.Domain[
					PWR_DOMAIN(PKG)
			].PowerLimit.Domain_Limit1;

		TDP_Limit = TDP_Limit + Custom_TDP_Offset[_lt];

		if (PUBLIC(RO(Proc))->Features.HSMP_Enable && (TDP_Limit >= 0))
		{
			RESET_ARRAY(arg, 8, 0, .value);
			arg[0].value = 1000 * TDP_Limit;
			rx = AMD_HSMP_Exec(HSMP_WR_PKG_PL1, arg);
		    if (rx == HSMP_RESULT_OK)
		    {
			RESET_ARRAY(arg, 8, 0, .value);
			rx = AMD_HSMP_Exec(HSMP_RD_PKG_PL1, arg);
		    }
		    if (rx == HSMP_RESULT_OK)
		    {
			PUBLIC(RO(Proc))->PowerThermal.Domain[
				PWR_DOMAIN(PKG)
			].PowerLimit.Domain_Limit1 = arg[0].value / 1000;

			PUBLIC(RO(Proc))->PowerThermal.Domain[
				PWR_DOMAIN(PKG)
			].PowerLimit.Enable_Limit1 = \

			PUBLIC(RO(Proc))->PowerThermal.Domain[
				PWR_DOMAIN(PKG)
			].PowerLimit.Clamping1 = \

			PUBLIC(RO(Proc))->PowerThermal.Domain[
				PWR_DOMAIN(PKG)
			].Unlock = \

			PUBLIC(RO(Proc))->PowerThermal.Domain[
				PWR_DOMAIN(PKG)
			].PowerLimit.Domain_Limit1 > 0 ? 1 : 0;
		    }
		    else if (IS_HSMP_OOO(rx))
		    {
			PUBLIC(RO(Proc))->Features.HSMP_Enable = 0;
		    }
		}
	    }
	}
	#undef _lt
    }
	SystemRegisters(Core);

	AMD_Microcode(Core);

	Dump_CPUID(Core);

	AMD_F17h_DCU_Technology(Core);

	AMD_Mitigation_Mechanisms(Core);

	/*	Query the FCH for various registers			*/
	AMD_FCH_PM_Read16(AMD_FCH_PM_CSTATE_EN, PM);
	switch (C1E_Enable) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			PM.CStateEn.C1eToC3En = \
			PM.CStateEn.C1eToC2En = !C1E_Enable;
			ToggleFeature = 1;
		break;
	}
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
	if (PM.CStateEn.C1eToC2En || PM.CStateEn.C1eToC3En)
	{
		BITCLR_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E, Core->Bind);
	} else {
		BITSET_CC(LOCKLESS, PUBLIC(RW(Proc))->C1E, Core->Bind);
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
	/*	Per SMT, initialize with the saved thermal parameters	*/
    if (Core->PowerThermal.Sensor == 0) {
	Core->PowerThermal.Param = PUBLIC(RO(Proc))->PowerThermal.Param;
    }
	/*	Collaborative Processor Performance Control	*/
    if (PUBLIC(RO(Proc))->Features.HWP_Enable)
    {
	AMD_CPPC_CAP1 CPPC_Cap = {.value = 0};
	AMD_CPPC_REQUEST CPPC_Req = {.value = 0};

	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);
	RDMSR(CPPC_Cap, MSR_AMD_CPPC_CAP1);

	Core->PowerThermal.HWP_Capabilities.Highest = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Cap.Highest,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Capabilities.Guaranteed = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Cap.Nominal,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Capabilities.Most_Efficient = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Cap.LowNonlinear,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Capabilities.Lowest = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Cap.Lowest,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	RDMSR(CPPC_Req, MSR_AMD_CPPC_REQ);
	if ((HWP_EPP >= 0) && (HWP_EPP <= 0xff))
	{
		CPPC_Req.Energy_Pref = HWP_EPP;
		WRMSR(CPPC_Req, MSR_AMD_CPPC_REQ);
		RDMSR(CPPC_Req, MSR_AMD_CPPC_REQ);
	}

	Core->PowerThermal.HWP_Request.Minimum_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Minimum_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Request.Maximum_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Maximum_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Request.Desired_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Desired_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Request.Energy_Pref  = CPPC_Req.Energy_Pref;

	Core->Boost[BOOST(HWP_MIN)]=Core->PowerThermal.HWP_Request.Minimum_Perf;
	Core->Boost[BOOST(HWP_MAX)]=Core->PowerThermal.HWP_Request.Maximum_Perf;
	Core->Boost[BOOST(HWP_TGT)]=Core->PowerThermal.HWP_Request.Desired_Perf;
    }
    else if (PUBLIC(RO(Proc))->Features.ACPI_CPPC)
    {
	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);

	Core->PowerThermal.HWP_Capabilities.Highest = \
		CPPC_AMD_Zen_ScaleRatio(Core,
			PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum,
			Core->PowerThermal.ACPI_CPPC.Highest,
			!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
	Core->PowerThermal.HWP_Capabilities.Guaranteed = \
		CPPC_AMD_Zen_ScaleRatio(Core,
			PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum,
			Core->PowerThermal.ACPI_CPPC.Guaranteed,
			!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0)
	Core->PowerThermal.HWP_Capabilities.Most_Efficient = \
			Core->PowerThermal.ACPI_CPPC.Efficient / PRECISION;

	Core->PowerThermal.HWP_Capabilities.Lowest = \
			Core->PowerThermal.ACPI_CPPC.Lowest / PRECISION;

	Core->PowerThermal.HWP_Request.Minimum_Perf = \
			Core->PowerThermal.ACPI_CPPC.Minimum / PRECISION;
	#else
	Core->PowerThermal.HWP_Capabilities.Most_Efficient = \
		CPPC_AMD_Zen_ScaleRatio(Core,
			PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum,
			Core->PowerThermal.ACPI_CPPC.Efficient,
			!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
	Core->PowerThermal.HWP_Capabilities.Lowest = \
		CPPC_AMD_Zen_ScaleRatio(Core,
			PUBLIC(RO(Proc))->PowerThermal.ACPI_CPPC.Maximum,
			Core->PowerThermal.ACPI_CPPC.Lowest,
			!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);
	Core->PowerThermal.HWP_Request.Minimum_Perf = \
			CPPC_AMD_Zen_ScaleRatio(Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				Core->PowerThermal.ACPI_CPPC.Minimum,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
	#endif
	Core->PowerThermal.HWP_Request.Maximum_Perf = \
			CPPC_AMD_Zen_ScaleRatio(Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				Core->PowerThermal.ACPI_CPPC.Maximum,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
	Core->PowerThermal.HWP_Request.Desired_Perf = \
			CPPC_AMD_Zen_ScaleRatio(Core,
				Core->PowerThermal.ACPI_CPPC.Highest,
				Core->PowerThermal.ACPI_CPPC.Desired,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
			);
	Core->PowerThermal.HWP_Request.Energy_Pref = \
					Core->PowerThermal.ACPI_CPPC.Energy;

	Core->Boost[BOOST(HWP_MIN)]=Core->PowerThermal.HWP_Request.Minimum_Perf;
	Core->Boost[BOOST(HWP_MAX)]=Core->PowerThermal.HWP_Request.Maximum_Perf;
	Core->Boost[BOOST(HWP_TGT)]=Core->PowerThermal.HWP_Request.Desired_Perf;
    }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 56)
inline void Sys_DumpTask(SYSGATE_RO *SysGate)
{
	SysGate->taskCount = 0;
}
#else /* KERNEL_VERSION(3, 10, 56) */
static void Sys_DumpTask(SYSGATE_RO *SysGate)
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0) \
 || (defined(RHEL_MAJOR) && (RHEL_MAJOR == 8) && (RHEL_MINOR >= 9))
		SysGate->taskList[cnt].state    = (short int) thread->__state;
#else
		SysGate->taskList[cnt].state    = (short int) thread->state;
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

static void Controller_Init(void)
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

static void Controller_Start(int wait)
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

static void Controller_Stop(int wait)
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

static void Controller_Exit(void)
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

static void Intel_Core_Counters_Set(union SAVE_AREA_CORE *Save, CORE_RO *Core)
{
	UNUSED(Core);

    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 2) {
	CORE_GLOBAL_PERF_CONTROL	Core_GlobalPerfControl = {.value = 0};
	CORE_FIXED_PERF_CONTROL 	Core_FixedPerfControl = {.value = 0};
	CORE_GLOBAL_PERF_STATUS 	Core_PerfOverflow = {.value = 0};
	CORE_GLOBAL_PERF_OVF_CTRL	Core_PerfOvfControl = {.value = 0};

	RDMSR(Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);
	Save->Core_GlobalPerfControl = Core_GlobalPerfControl;
	Core_GlobalPerfControl.EN_FIXED_CTR0  = 1;
#if defined(MSR_CORE_PERF_UCC) && MSR_CORE_PERF_UCC == MSR_CORE_PERF_FIXED_CTR1
	Core_GlobalPerfControl.EN_FIXED_CTR1  = 1;
#endif
#if defined(MSR_CORE_PERF_URC) && MSR_CORE_PERF_URC == MSR_CORE_PERF_FIXED_CTR2
	Core_GlobalPerfControl.EN_FIXED_CTR2  = 1;
#endif
	WRMSR(Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);

	RDMSR(Core_FixedPerfControl, MSR_CORE_PERF_FIXED_CTR_CTRL);
	Save->Core_FixedPerfControl = Core_FixedPerfControl;
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

#define AMD_Core_Counters_Set(Save, Core, PMC)				\
({									\
    if (PUBLIC(RO(Proc))->Features.PerfMon.EBX.InstrRetired == 0)	\
    {									\
	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);			\
	Save->Core_HardwareConfiguration = Core->SystemRegister.HWCR;	\
	Core->SystemRegister.HWCR.PMC.IRPerfEn = 1 ;			\
	WRMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);			\
    }									\
})

#define AMD_Zen_PMC_L3_Set(Save, Core)					\
({									\
	const unsigned int bitwiseID = Core->T.ApicID			\
	& PUBLIC(RO(Proc))->Features.leaf80000008.ECX.ApicIdCoreIdSize; \
									\
    if ((PUBLIC(RO(Proc))->Features.ExtInfo.ECX.PerfLLC == 1)		\
    && (Core->T.ThreadID == 0) && (bitwiseID == 0))			\
    {									\
	ZEN_L3_PERF_CTL Zen_L3_Cache_PerfControl = {.value = 0};	\
									\
	RDMSR(Zen_L3_Cache_PerfControl, MSR_AMD_F17H_L3_PERF_CTL);	\
	Save->Zen_L3_Cache_PerfControl=Zen_L3_Cache_PerfControl;	\
	Zen_L3_Cache_PerfControl.EventSelect =	0x90;			\
	Zen_L3_Cache_PerfControl.UnitMask =	0x00;			\
	Zen_L3_Cache_PerfControl.CounterEn =	1;			\
	Zen_L3_Cache_PerfControl.CoreID =	0;			\
	Zen_L3_Cache_PerfControl.EnAllSlices =	0;			\
	Zen_L3_Cache_PerfControl.EnAllCores =	0;			\
	Zen_L3_Cache_PerfControl.SliceMask =	0x0f;			\
	Zen_L3_Cache_PerfControl.ThreadMask =	0xff;			\
	WRMSR(Zen_L3_Cache_PerfControl, MSR_AMD_F17H_L3_PERF_CTL);	\
    }									\
})

#define AMD_Zen_PMC_PERF_Set(Save, Core)				\
({									\
	const unsigned int bitwiseID = Core->T.ApicID			\
	& PUBLIC(RO(Proc))->Features.leaf80000008.ECX.ApicIdCoreIdSize; \
									\
    if ((Core->T.ThreadID == 0) && (bitwiseID == 0))			\
    {									\
	ZEN_PERF_CTL Zen_PerformanceControl = {.value = 0};		\
									\
	RDMSR(Zen_PerformanceControl, MSR_AMD_F17H_PERF_CTL);		\
	Save->Zen_PerformanceControl = Zen_PerformanceControl;		\
	Zen_PerformanceControl.EventSelect00 =	0xc1;			\
	Zen_PerformanceControl.UnitMask =	0x00;			\
	Zen_PerformanceControl.OsUserMode =	0x03;			\
	Zen_PerformanceControl.EdgeDetect =	0;			\
	Zen_PerformanceControl.APIC_Interrupt = 0;			\
	Zen_PerformanceControl.CounterEn =	1;			\
	Zen_PerformanceControl.InvCntMask =	0;			\
	Zen_PerformanceControl.CntMask =	0x00;			\
	Zen_PerformanceControl.EventSelect08 =	0x00;			\
	Zen_PerformanceControl.HostGuestOnly =	0x00;			\
	WRMSR(Zen_PerformanceControl, MSR_AMD_F17H_PERF_CTL);		\
    }									\
})

#define AMD_Zen_PMC_UMC_Set(Save, Core)			({ /* NOP */ })

#define AMD_Zen_PMC_PCU_Set(Save, Core)			({ /* NOP */ })

#define _AMD_Zen_PMC_Set_(Save, Core, _PMC_)				\
({									\
	AMD_Zen_PMC_##_PMC_##_Set(Save, Core);				\
})
#define AMD_Zen_PMC_Set(Save, Core, _PMC_)				\
	_AMD_Zen_PMC_Set_(Save, Core, _PMC_)

#define AMD_Zen_PMC_ARCH_PMC_Set(Save, Core)		({ /* NOP */ })

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
	PRIVATE(OF(SaveArea)).Intel.Uncore_GlobalPerfControl =		\
						Uncore_GlobalPerfControl;\
									\
	Uncore_GlobalPerfControl.PMU.EN_FIXED_CTR0  = 1;		\
	WRMSR(	Uncore_GlobalPerfControl,				\
		MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL );			\
									\
	RDMSR(	Uncore_FixedPerfControl,				\
		MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL );		\
									\
	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl =		\
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

static void Intel_Core_Counters_Clear(union SAVE_AREA_CORE *Save, CORE_RO *Core)
{
	UNUSED(Core);

    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 2)
    {
	WRMSR(Save->Core_FixedPerfControl, MSR_CORE_PERF_FIXED_CTR_CTRL);

	WRMSR(Save->Core_GlobalPerfControl, MSR_CORE_PERF_GLOBAL_CTRL);
    }
}

static void AMD_Core_Counters_Clear(union SAVE_AREA_CORE *Save, CORE_RO *Core)
{
	UNUSED(Core);

	if (PUBLIC(RO(Proc))->Features.PerfMon.EBX.InstrRetired == 0)
	{
		WRMSR(Save->Core_HardwareConfiguration, MSR_K7_HWCR);
	}
}

#define AMD_Zen_PMC_L3_Clear(Save, Core)				\
({									\
	const unsigned int bitwiseID = Core->T.ApicID			\
	& PUBLIC(RO(Proc))->Features.leaf80000008.ECX.ApicIdCoreIdSize; \
									\
    if ((PUBLIC(RO(Proc))->Features.ExtInfo.ECX.PerfLLC == 1)		\
     && (Core->T.ThreadID == 0) && (bitwiseID == 0))			\
    {									\
	WRMSR(Save->Zen_L3_Cache_PerfControl, MSR_AMD_F17H_L3_PERF_CTL);\
    }									\
})

#define AMD_Zen_PMC_PERF_Clear(Save, Core)				\
({									\
	const unsigned int bitwiseID = Core->T.ApicID			\
	& PUBLIC(RO(Proc))->Features.leaf80000008.ECX.ApicIdCoreIdSize; \
									\
    if ((Core->T.ThreadID == 0) && (bitwiseID == 0))			\
    {									\
	WRMSR(Save->Zen_PerformanceControl, MSR_AMD_F17H_PERF_CTL);	\
    }									\
})

#define AMD_Zen_PMC_UMC_Clear(Save, Core)		({ /* NOP */ })

#define AMD_Zen_PMC_PCU_Clear(Save, Core)		({ /* NOP */ })

#define _AMD_Zen_PMC_Clear_(Save, Core, _PMC_)				\
({									\
	AMD_Zen_PMC_##_PMC_##_Clear(Save, Core);			\
})
#define AMD_Zen_PMC_Clear(Save, Core, _PMC_)				\
	_AMD_Zen_PMC_Clear_(Save, Core, _PMC_)

#define AMD_Zen_PMC_ARCH_PMC_Clear(Save, Core)		({ /* NOP */ })

#define Uncore_Counters_Clear(PMU)					\
({									\
    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version >= 3)		\
    {									\
	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl,	\
		MSR_##PMU##_UNCORE_PERF_FIXED_CTR_CTRL );		\
									\
	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_GlobalPerfControl,	\
		MSR_##PMU##_UNCORE_PERF_GLOBAL_CTRL );			\
    }									\
})

#define Counters_VirtualMachine(Core, T)				\
({									\
    if (!PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC) {		\
	RDTSC64(Core->Counter[T].TSC);					\
    } else {								\
	RDTSCP64(Core->Counter[T].TSC); 				\
    }									\
    switch (PUBLIC(RO(Proc))->Features.Info.Hypervisor.CRC) {		\
    HCF_MSR:								\
    case CRC_VBOX:							\
    case CRC_KBOX:							\
    default:								\
	RDCOUNTER(Core->Counter[T].C0.UCC, MSR_CORE_PERF_UCC);		\
	RDCOUNTER(Core->Counter[T].C0.URC, MSR_CORE_PERF_URC);		\
	break;								\
    case CRC_KVM:							\
    case CRC_VMWARE:							\
    case CRC_HYPERV:							\
	/* HV_PARTITION_PRIVILEGE_MASK: AccessVpRunTimeReg	*/	\
	if (BITVAL(Core->CpuID[						\
			CPUID_40000003_00000000_HYPERVISOR_FEATURES	\
			].reg[REG_CPUID_EAX], 0))			\
	{/* HV_X64_MSR_VP_RUNTIME: vcpu runtime in 100ns units	*/	\
		RDCOUNTER(Core->Counter[T].C0.URC, 0x40000010); 	\
									\
		Core->Counter[T].C0.URC = Core->Counter[T].C0.URC	\
		 * PUBLIC(RO(Proc))->Features.Factory.Ratio * 10;	\
									\
		Core->Counter[T].C0.UCC = Core->Counter[T].C0.URC;	\
	} else								\
		goto HCF_MSR;						\
	break;								\
    }									\
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

#define Counters_Goldmont(Core, T) Counters_SLM(Core, T)

#define SMT_Counters_Nehalem(Core, T)					\
({									\
	register  unsigned long long Cx;				\
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
	register  unsigned long long Cx;				\
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

#define SMT_Counters_Alderlake_Ecore(Core, T)				\
({									\
	RDTSC_COUNTERx7(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_C1_RESIDENCY, Core->Counter[T].C1,	\
			MSR_CORE_C3_RESIDENCY, Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY, Core->Counter[T].C6,	\
			MSR_CORE_C7_RESIDENCY, Core->Counter[T].C7,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
})

#define SMT_Counters_Alderlake_Pcore(Core, T)				\
({									\
	RDTSC_COUNTERx7(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_CORE_C1_RESIDENCY, Core->Counter[T].C1,	\
			MSR_CORE_C3_RESIDENCY, Core->Counter[T].C3,	\
			MSR_CORE_C6_RESIDENCY, Core->Counter[T].C6,	\
			MSR_CORE_C7_RESIDENCY, Core->Counter[T].C7,	\
			MSR_CORE_PERF_FIXED_CTR0,Core->Counter[T].INST);\
})

#define SMT_Counters_AMD_Family_17h(Core, T)				\
({									\
	RDTSCP_COUNTERx3(Core->Counter[T].TSC,				\
			MSR_CORE_PERF_UCC, Core->Counter[T].C0.UCC,	\
			MSR_CORE_PERF_URC, Core->Counter[T].C0.URC,	\
			MSR_AMD_F17H_IRPERF, Core->Counter[T].INST);	\
									\
    if(PUBLIC(RO(Proc))->Registration.Driver.CPUidle == REGISTRATION_ENABLE)\
    {	/* Read Virtual PMC and cumulative store: */			\
	Atomic_Read_VPMC(LOCKLESS, Core->Counter[T].C1, Core->VPMC.C1); \
	Atomic_Read_VPMC(LOCKLESS, Core->Counter[T].C3, Core->VPMC.C2); \
	Atomic_Add_VPMC (LOCKLESS, Core->Counter[T].C3, Core->VPMC.C3); \
	Atomic_Read_VPMC(LOCKLESS, Core->Counter[T].C6, Core->VPMC.C4); \
	Atomic_Add_VPMC (LOCKLESS, Core->Counter[T].C6, Core->VPMC.C5); \
	Atomic_Add_VPMC (LOCKLESS, Core->Counter[T].C6, Core->VPMC.C6); \
    }									\
    else								\
    {									\
	Core->Counter[T].C1 =						\
		(Core->Counter[T].TSC > Core->Counter[T].C0.URC) ?	\
			Core->Counter[T].TSC - Core->Counter[T].C0.URC	\
			: 0;						\
    }									\
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
		 Core->Counter[1].C1) ? 				\
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
	PUBLIC(RO(Proc))->Counter[T].PCLK = Core->Counter[T].TSC;	\
})

#define PKG_Counters_Generic(Core, T)					\
({									\
	PUBLIC(RO(Proc))->Counter[T].PCLK = Core->Counter[T].TSC;	\
})

#define PKG_Counters_SLM(Core, T)					\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
	MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,	\
	MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,	\
	MSR_ATOM_PKG_C4_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC04,	\
	MSR_ATOM_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,	\
	MSR_ATOM_MC6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].MC6);	\
})

#define PKG_Counters_Nehalem(Core, T)					\
({									\
    RDTSCP_COUNTERx4(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_NHM_UNCORE_PERF_FIXED_CTR0 ,			\
			PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_SandyBridge(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_SNB_UNCORE_PERF_FIXED_CTR0 ,			\
			PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);	\
})

#define PKG_COUNTERS_SANDYBRIDGE_EP(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_SNB_EP_UNCORE_PERF_FIXED_CTR0,			\
			PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);	\
})

static void PKG_Counters_SandyBridge_EP(CORE_RO *Core, unsigned int T)
{
	UNUSED(Core);

	PKG_COUNTERS_SANDYBRIDGE_EP(Core, T);
}

#define PKG_COUNTERS_IVYBRIDGE_EP(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_SNB_EP_UNCORE_PERF_FIXED_CTR0,			\
			PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);	\
})

static void PKG_Counters_IvyBridge_EP(CORE_RO *Core, unsigned int T)
{
	UNUSED(Core);

	PKG_COUNTERS_IVYBRIDGE_EP(Core, T);
}

#define PKG_Counters_Haswell_EP(Core, T)				\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_HSW_EP_UNCORE_PERF_FIXED_CTR0,			\
			PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_Haswell_ULT(Core, T)				\
({									\
    RDTSCP_COUNTERx7(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_PKG_C8_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC08,\
		MSR_PKG_C9_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC09,\
		MSR_PKG_C10_RESIDENCY,PUBLIC(RO(Proc))->Counter[T].PC10);\
									\
	RDCOUNTER(	PUBLIC(RO(Proc))->Counter[T].Uncore.FC0,	\
			MSR_SNB_UNCORE_PERF_FIXED_CTR0 );		\
})

#define PKG_Counters_Goldmont(Core, T)					\
({									\
    RDTSCP_COUNTERx4(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C10_RESIDENCY,PUBLIC(RO(Proc))->Counter[T].PC10);\
})

#define PKG_Counters_Skylake(Core, T)					\
({									\
    RDTSCP_COUNTERx5(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07,\
		MSR_SKL_UNCORE_PERF_FIXED_CTR0 ,			\
			PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);	\
})

#define PKG_Counters_Skylake_X(Core, T) 				\
({									\
    RDTSCP_COUNTERx4(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
		MSR_PKG_C2_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC02,\
		MSR_PKG_C3_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC03,\
		MSR_PKG_C6_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC06,\
		MSR_PKG_C7_RESIDENCY, PUBLIC(RO(Proc))->Counter[T].PC07);\
})

#define PKG_Counters_Alderlake_Ecore(Core, T)				\
({									\
    RDCOUNTER(PUBLIC(RO(Proc))->Counter[T].PC04, MSR_ATOM_PKG_C4_RESIDENCY);\
    RDCOUNTER(PUBLIC(RO(Proc))->Counter[T].PC10, MSR_PKG_C10_RESIDENCY);\
    RDCOUNTER(PUBLIC(RO(Proc))->Counter[T].MC6, MSR_ATOM_MC6_RESIDENCY);\
})

#define PKG_Counters_Alderlake_Pcore(Core, T)				\
({									\
    RDTSCP_COUNTERx7(PUBLIC(RO(Proc))->Counter[T].PCLK ,		\
	MSR_PKG_C2_RESIDENCY,	PUBLIC(RO(Proc))->Counter[T].PC02,	\
	MSR_PKG_C3_RESIDENCY,	PUBLIC(RO(Proc))->Counter[T].PC03,	\
	MSR_PKG_C6_RESIDENCY,	PUBLIC(RO(Proc))->Counter[T].PC06,	\
	MSR_PKG_C7_RESIDENCY,	PUBLIC(RO(Proc))->Counter[T].PC07,	\
	MSR_PKG_C8_RESIDENCY,	PUBLIC(RO(Proc))->Counter[T].PC08,	\
	MSR_PKG_C9_RESIDENCY,	PUBLIC(RO(Proc))->Counter[T].PC09,	\
	MSR_ADL_UNCORE_PERF_FIXED_CTR0 ,				\
				PUBLIC(RO(Proc))->Counter[T].Uncore.FC0);\
})

/*
 *		Counter | Description
 *		--------|------------
 *		0x5828	| Cycle Sum of All Active Cores
 *		0x5830	| Cycle Sum of Any Active Core
 *		0x5838	| Cycle Sum of Active Graphics
 *		0x5840	| Cycle Sum of Overlapping Active GT and Core
 *		0x5848	| Cycle Sum of Any Active GT Slice
 *		0x5850	| Cycle Sum of All Active GT Slice
 *		0x5858	| Cycle Sum of Any GT Media Engine
 *		0x5860	| Ratio Sum of Any Active Core
 *		0x5868	| Ratio Sum of Active GT
 *		0x5870	| Ratio Sum of Active GT Slice
 */
#define Pkg_Intel_PMC_PCU_Set(...)					\
({									\
    if (GetMemoryBAR(0, 0, 0, 0, 0x48, 64, 0x10000, 0,			\
		&PRIVATE(OF(PCU)).HB, &PRIVATE(OF(PCU)).BAR) == 0)	\
    {									\
	PRIVATE(OF(PCU)).ADDR = __VA_ARGS__;				\
    } else {								\
	PRIVATE(OF(PCU)).ADDR = 0x0;					\
    }									\
})

#define Pkg_Intel_PMC_L3_Set(...)			({ /* NOP */ })

#define Pkg_Intel_PMC_PERF_Set(...)			({ /* NOP */ })

#define Pkg_Intel_PMC_UMC_Set(...)			({ /* NOP */ })

#define _Pkg_Intel_PMC_Set_(_PMC_ , ...)				\
({									\
	Pkg_Intel_PMC_##_PMC_##_Set(__VA_ARGS__);			\
})
#define Pkg_Intel_PMC_Set(_PMC_, ...)					\
	_Pkg_Intel_PMC_Set_(_PMC_ , __VA_ARGS__)

#define Pkg_Intel_PMC_ARCH_PMC_Set(_PMC_, ...)		({ /* NOP */ })

#define Pkg_Intel_PMC_L3_Clear()			({ /* NOP */ })

#define Pkg_Intel_PMC_PERF_Clear()			({ /* NOP */ })

#define Pkg_Intel_PMC_UMC_Clear()			({ /* NOP */ })

#define Pkg_Intel_PMC_PCU_Clear()					\
({									\
	PutMemoryBAR(&PRIVATE(OF(PCU)).HB, &PRIVATE(OF(PCU)).BAR);	\
	PRIVATE(OF(PCU)).ADDR = 0x0;					\
})

#define _Pkg_Intel_PMC_Clear_(_PMC_)					\
({									\
	Pkg_Intel_PMC_##_PMC_##_Clear();				\
})
#define Pkg_Intel_PMC_Clear(_PMC_)					\
	_Pkg_Intel_PMC_Clear_(_PMC_)

#define Pkg_Intel_PMC_ARCH_PMC_Clear()			({ /* NOP */ })

#define Pkg_Intel_PMC_PCU_Counters(Core, _T)				\
({									\
  if (PRIVATE(OF(PCU)).BAR != NULL) {					\
    if (PRIVATE(OF(PCU)).ADDR != 0x0)					\
    {									\
	PUBLIC(RO(Proc))->Counter[_T].MC6 =				\
		readq(PRIVATE(OF(PCU)).BAR + PRIVATE(OF(PCU)).ADDR);	\
    }									\
    switch (PUBLIC(RO(Proc))->ArchID) { 				\
    case Tigerlake:							\
    case Tigerlake_U:							\
	TGL_SA(PRIVATE(OF(PCU)).BAR);					\
	break;								\
    case Rocketlake:							\
    case Rocketlake_U:							\
	RKL_SA(PRIVATE(OF(PCU)).BAR);					\
	break;								\
    case Alderlake_S:							\
    case Alderlake_H:							\
    case Alderlake_N:							\
    case Raptorlake:							\
    case Raptorlake_P:							\
    case Raptorlake_S:							\
	ADL_SA(PRIVATE(OF(PCU)).BAR);					\
	break;								\
    case Meteorlake_M:							\
    case Meteorlake_N:							\
    case Meteorlake_S:							\
    case LunarLake:							\
    case ArrowLake:							\
    case ArrowLake_H:							\
    case ArrowLake_U:							\
    case PantherLake:							\
	MTL_SA(PRIVATE(OF(PCU)).BAR);					\
	break;								\
    }									\
  }									\
})

#define Pkg_Intel_PMC_L3_Counters(Core, _T)		({ /* NOP */ })

#define Pkg_Intel_PMC_PERF_Counters(Core, _T)		({ /* NOP */ })

#define Pkg_Intel_PMC_UMC_Counters(Core, _T)		({ /* NOP */ })

#define _Pkg_Intel_PMC_Counters_(Core, _T, _PMC_)			\
({									\
	Pkg_Intel_PMC_##_PMC_##_Counters(Core, _T);			\
})
#define Pkg_Intel_PMC_Counters(Core, _T, _PMC_) 			\
	_Pkg_Intel_PMC_Counters_(Core, _T, _PMC_)

#define Pkg_Intel_PMC_ARCH_PMC_Counters(Core, _T)	({ /* NOP */ })

#define AMD_Zen_PMC_L3_Closure(Core, _T)				\
({									\
	PUBLIC(RO(Proc))->Delta.CTR[CCX] =				\
				PUBLIC(RO(Proc))->Counter[1].CTR[CCX]	\
				- PUBLIC(RO(Proc))->Counter[0].CTR[CCX];\
									\
	PUBLIC(RO(Proc))->Counter[0].CTR[CCX] = 			\
				PUBLIC(RO(Proc))->Counter[1].CTR[CCX];	\
})

#define AMD_Zen_PMC_L3_Counters(Core, _T, ...)				\
({	/* Read the Cache L3 performance counter per Complex	*/	\
	const unsigned int bitwiseID = Core->T.ApicID			\
	& PUBLIC(RO(Proc))->Features.leaf80000008.ECX.ApicIdCoreIdSize; \
									\
    if ((PUBLIC(RO(Proc))->Features.ExtInfo.ECX.PerfLLC == 1)		\
     && (Core->T.PackageID == 0) && (bitwiseID == 0))			\
    {									\
	const unsigned short CCX = Core->T.Cluster.CCX & 0b111; 	\
									\
	RDCOUNTER(	PUBLIC(RO(Proc))->Counter[_T].CTR[CCX], 	\
			MSR_AMD_F17H_L3_PERF_CTR );			\
									\
	PUBLIC(RO(Proc))->Counter[_T].CTR[CCX] &= 0xffffffffffff;	\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
    }									\
})

#define AMD_Zen_PMC_PERF_Closure(Core, _T)				\
({									\
	PUBLIC(RO(Proc))->Delta.CTR[CCX] =				\
				PUBLIC(RO(Proc))->Counter[1].CTR[CCX]	\
				- PUBLIC(RO(Proc))->Counter[0].CTR[CCX];\
									\
	PUBLIC(RO(Proc))->Counter[0].CTR[CCX] = 			\
				PUBLIC(RO(Proc))->Counter[1].CTR[CCX];	\
})

#define AMD_Zen_PMC_PERF_Counters(Core, _T, ...) 			\
({									\
	const unsigned int bitwiseID = Core->T.ApicID			\
	& PUBLIC(RO(Proc))->Features.leaf80000008.ECX.ApicIdCoreIdSize; \
									\
    if ((Core->T.PackageID == 0) && (bitwiseID == 0))			\
    {									\
	const unsigned short CCX = Core->T.Cluster.CCX & 0b111; 	\
									\
	RDCOUNTER(	PUBLIC(RO(Proc))->Counter[_T].CTR[CCX], 	\
			MSR_AMD_F17H_PERF_CTR );			\
									\
	PUBLIC(RO(Proc))->Counter[_T].CTR[CCX] &= 0xffffffffffff;	\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
    }									\
})

#define AMD_Zen_PMC_UMC_Closure(Core, _T)		({ /* NOP */ })

#define AMD_Zen_PMC_UMC_Counters(Core, _T , ...)			\
({									\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
})

#define AMD_Zen_PMC_PCU_Closure(Core, _T)		({ /* NOP */ })

#define AMD_Zen_PMC_PCU_Counters(Core, _T , ...)			\
({									\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
})

#define _AMD_Zen_PMC_Closure_(Core, _T, _PMC_)				\
	AMD_Zen_PMC_##_PMC_##_Closure(Core, _T)

#define AMD_Zen_PMC_Closure(Core, _T, _PMC_)				\
	_AMD_Zen_PMC_Closure_(Core, _T, _PMC_)

#define _AMD_Zen_PMC_Counters_(Core, _T, _PMC_, ...)			\
({									\
	AMD_Zen_PMC_##_PMC_##_Counters(Core, _T, __VA_ARGS__);		\
})
#define AMD_Zen_PMC_Counters(Core, _T, _PMC_, ...)			\
	_AMD_Zen_PMC_Counters_(Core, _T, _PMC_, __VA_ARGS__)

#define AMD_Zen_PMC_ARCH_PMC_Counters(Core, _T, ...)	({ /* NOP */ })

#define Pkg_AMD_Zen_PMC_L3_Set(Core)			({ /* NOP */ })

#define Pkg_AMD_Zen_PMC_PERF_Set(Core)			({ /* NOP */ })

#define Pkg_AMD_Zen_PMC_UMC_Set(Core)					\
({									\
	ZEN_UMC_PERF_CTL_CLK Zen_UMC_PerfCtlClk = {			\
		.value=0, .GlblReset=1, .GlblMonEn=1, .CtrClkEn=1	\
	};								\
									\
	unsigned short mc;						\
  for (mc = 0; mc < PUBLIC(RO(Proc))->Uncore.CtrlCount; mc++)		\
  {									\
	unsigned short cha;						\
   for (cha=0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount;cha++)\
   {									\
	unsigned int slot;						\
    for (slot=0;slot < PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount;slot++)\
    {									\
	ZEN_UMC_PERF_CTL Zen_UMC_PerfControl = {			\
		.value = 0, .EventSelect = 0x13 + slot, .CounterEn = 1	\
	};								\
									\
	Core_AMD_SMN_Write(	Zen_UMC_PerfControl,			\
				SMU_AMD_ZEN_UMC_PERF_CTL(cha, slot),	\
				PRIVATE(OF(Zen)).Device.DF );		\
    }									\
	Core_AMD_SMN_Write(	Zen_UMC_PerfCtlClk,			\
				SMU_AMD_UMC_PERF_CTL_CLK(cha),		\
				PRIVATE(OF(Zen)).Device.DF );		\
   }									\
  }									\
})

#define Pkg_AMD_Zen_PMC_PCU_Set(Core)			({ /* NOP */ })

#define _Pkg_AMD_Zen_PMC_Set_(Core, _PMC_)				\
({									\
	Pkg_AMD_Zen_PMC_##_PMC_##_Set(Core);				\
})
#define Pkg_AMD_Zen_PMC_Set(Core, _PMC_)				\
	_Pkg_AMD_Zen_PMC_Set_(Core, _PMC_)

#define Pkg_AMD_Zen_PMC_ARCH_PMC_Set(Core)		({ /* NOP */ })

#define Pkg_AMD_Zen_PMC_L3_Clear(Core)			({ /* NOP */ })

#define Pkg_AMD_Zen_PMC_PERF_Clear(Core)		({ /* NOP */ })

#define Pkg_AMD_Zen_PMC_UMC_Clear(Core) 				\
({									\
	ZEN_UMC_PERF_CTL_CLK Zen_UMC_PerfCtlClk = { .value = 0 };	\
									\
	unsigned short mc;						\
  for (mc = 0; mc < PUBLIC(RO(Proc))->Uncore.CtrlCount; mc++)		\
  {									\
	unsigned short cha;						\
   for (cha=0; cha < PUBLIC(RO(Proc))->Uncore.MC[mc].ChannelCount;cha++)\
   {									\
	unsigned int slot;						\
    for (slot=0;slot < PUBLIC(RO(Proc))->Uncore.MC[mc].SlotCount;slot++)\
    {									\
	ZEN_UMC_PERF_CTL Zen_UMC_PerfControl = { .value = 0 };		\
									\
	Core_AMD_SMN_Write(	Zen_UMC_PerfControl,			\
				SMU_AMD_ZEN_UMC_PERF_CTL(cha, slot),	\
				PRIVATE(OF(Zen)).Device.DF );		\
    }									\
	Core_AMD_SMN_Write(	Zen_UMC_PerfCtlClk,			\
				SMU_AMD_UMC_PERF_CTL_CLK(cha),		\
				PRIVATE(OF(Zen)).Device.DF );		\
   }									\
  }									\
})

#define Pkg_AMD_Zen_PMC_PCU_Clear(Core) 		({ /* NOP */ })

#define _Pkg_AMD_Zen_PMC_Clear_(Core, _PMC_)				\
({									\
	Pkg_AMD_Zen_PMC_##_PMC_##_Clear(Core);				\
})
#define Pkg_AMD_Zen_PMC_Clear(Core, _PMC_)				\
	_Pkg_AMD_Zen_PMC_Clear_(Core, _PMC_)

#define Pkg_AMD_Zen_PMC_ARCH_PMC_Clear(Core)		({ /* NOP */ })

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

#define PWR_ACCU_Goldmont(Pkg, T)					\
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

static void Power_ACCU_SKL_DEFAULT(PROC_RO *Pkg, unsigned int T)
{
	PWR_ACCU_Skylake(Pkg, T);
}

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

static void Power_ACCU_SKL_PLATFORM(PROC_RO *Pkg, unsigned int T)
{
	PWR_ACCU_SKL_PLATFORM(Pkg, T);
}

#define PWR_ACCU_Alderlake(Pkg, T)					\
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
									\
        RDCOUNTER(Pkg->Counter[T].Power.ACCU[PWR_DOMAIN(PLATFORM)],	\
					MSR_PLATFORM_ENERGY_STATUS);	\
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

#define Pkg_AMD_Zen_PMC_L3_Closure(Pkg, Core, _T)			\
({									\
	Delta_PTSC_OVH(Pkg, Core);					\
	Save_PTSC(Pkg); 						\
})

#define Pkg_AMD_Zen_PMC_ARCH_PMC_Closure(Pkg, Core, _T) 		\
({									\
	Delta_PTSC_OVH(Pkg, Core);					\
	Save_PTSC(Pkg); 						\
})

#define Pkg_AMD_Zen_PMC_L3_Counters(Pkg, Core, T, ...)			\
({									\
	Pkg->Counter[T].PCLK = Core->Counter[T].TSC;			\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
									\
	RDCOUNTER(Pkg->Counter[T].Uncore.FC0, MSR_AMD_F17H_DF_PERF_CTR);\
})

#define Pkg_AMD_Zen_PMC_PERF_Closure(Pkg, Core, _T)			\
({									\
	Delta_PTSC_OVH(Pkg, Core);					\
	Save_PTSC(Pkg); 						\
})

#define Pkg_AMD_Zen_PMC_PERF_Counters(Pkg, Core, _T, ...)		\
({									\
	Pkg->Counter[_T].PCLK = Core->Counter[_T].TSC;			\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
									\
	RDCOUNTER(Pkg->Counter[_T].Uncore.FC0, MSR_AMD_F17H_DF_PERF_CTR);\
})

#define Pkg_AMD_Zen_PMC_UMC_Closure(Pkg, Core, _T)			\
({									\
	/*			Delta with previous counter	*/	\
	Pkg->Delta.CTR[idx]	= Pkg->Counter[1].CTR[idx]		\
				- Pkg->Counter[0].CTR[idx];		\
	/*			Save current counter		*/	\
	Pkg->Counter[0].CTR[idx] = Pkg->Counter[1].CTR[idx];		\
})

#define Pkg_AMD_Zen_PMC_UMC_Counters(Pkg, Core, _T, ...)		\
({									\
	unsigned short umc;						\
	bool Got_Mem_Clock = false;					\
									\
  for (umc = 0; umc < Pkg->Uncore.CtrlCount; umc++)			\
  {									\
	unsigned short cha;						\
   for (cha = 0; cha < Pkg->Uncore.MC[umc].ChannelCount; cha++)	\
   {									\
	const unsigned short idx = MC_VECTOR_TO_SCALAR(umc, cha);	\
									\
	union { 							\
			unsigned long long	ctr48;			\
		struct {						\
			struct {					\
				unsigned int	value;			\
			} low32;					\
			struct {					\
				unsigned int	value;			\
			} high16;					\
		};							\
	} data; 							\
									\
	/*			48-bits UMC counter		*/	\
	Core_AMD_SMN_Read(data.low32,	SMU_AMD_ZEN_UMC_PERF_CLK_LOW(cha),\
					PRIVATE(OF(Zen)).Device.DF);	\
									\
	Core_AMD_SMN_Read(data.high16,	SMU_AMD_ZEN_UMC_PERF_CLK_HIGH(cha),\
					PRIVATE(OF(Zen)).Device.DF);	\
	data.high16.value &= 0xffff;					\
									\
	Pkg->Counter[_T].CTR[idx] = data.ctr48 ;			\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
	/*		Update memory clock frequency (Hz)	*/	\
    if (Got_Mem_Clock == false)					\
    {									\
     switch(Pkg->Uncore.MC[umc].Channel[cha].AMD17h.CONFIG.BurstLength) {\
     case 0x0:	/* BL2 */						\
     case 0x1:	/* BL4 */						\
     case 0x2:	/* BL8 */						\
	Pkg->Counter[_T].PCLK = Core->Clock.Hz				\
		* Pkg->Uncore.MC[umc].Channel[cha].AMD17h.MISC.DDR4.MEMCLK;\
									\
	Pkg->Counter[_T].PCLK = DIV_ROUND_CLOSEST(Pkg->Counter[_T].PCLK, 3);\
	/*		Apply the memory clock divisor		*/	\
	Pkg->Counter[_T].PCLK >>= 					\
	  !Pkg->Uncore.MC[umc].Channel[cha].AMD17h.DbgMisc.UCLK_Divisor;\
									\
	break;								\
     case 0x3:	/* BL16 */						\
	Pkg->Counter[_T].PCLK = Core->Clock.Hz * 10LLU			\
		* Pkg->Uncore.MC[umc].Channel[cha].AMD17h.MISC.DDR5.MEMCLK;\
	break;								\
     }									\
	Pkg->Delta.PCLK = Pkg->Counter[_T].PCLK;			\
									\
	Got_Mem_Clock = true;						\
    }									\
   }									\
  }									\
	/*			Data Fabric Counter		*/	\
	RDCOUNTER(Pkg->Counter[_T].Uncore.FC0, MSR_AMD_F17H_DF_PERF_CTR);\
})

#define Pkg_AMD_Zen_PMC_PCU_Closure(Pkg, Core, _T)			\
({									\
	Delta_PTSC_OVH(Pkg, Core);					\
	Save_PTSC(Pkg); 						\
})

#define Pkg_AMD_Zen_PMC_PCU_Counters(Pkg, Core, _T, ...)		\
({									\
	Pkg->Counter[_T].PCLK = Core->Counter[_T].TSC;			\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
})

#define _Pkg_AMD_Zen_PMC_Closure_(Pkg, Core, _T, _PMC_) 		\
	Pkg_AMD_Zen_PMC_##_PMC_##_Closure(Pkg, Core, _T)

#define Pkg_AMD_Zen_PMC_Closure(Pkg, Core, _T, _PMC_)			\
	_Pkg_AMD_Zen_PMC_Closure_(Pkg, Core, _T, _PMC_)

#define _Pkg_AMD_Zen_PMC_Counters_(Pkg, Core , _T, _PMC_, ...)		\
({									\
	Pkg_AMD_Zen_PMC_##_PMC_##_Counters(Pkg, Core, _T, __VA_ARGS__); \
})

#define Pkg_AMD_Zen_PMC_Counters(Pkg, Core, _T, _PMC_, ...)		\
	_Pkg_AMD_Zen_PMC_Counters_(Pkg, Core, _T, _PMC_, __VA_ARGS__)

#define Pkg_AMD_Zen_PMC_ARCH_PMC_Counters(Pkg, Core, _T, ...)		\
({									\
	Pkg->Counter[_T].PCLK = Core->Counter[_T].TSC;			\
	/*			Closure statement		*/	\
	__VA_ARGS__;							\
})

static void Core_Intel_Temp(CORE_RO *Core)
{
	THERM_STATUS ThermStatus = {.value = 0};
	RDMSR(ThermStatus, MSR_IA32_THERM_STATUS);	/*All Intel families.*/

	Core->PowerThermal.Sensor = ThermStatus.DTS;

	Core->PowerThermal.Events[eLOG] = \
		  ((Bit64) ThermStatus.Thermal_Log	<< LSHIFT_THERMAL_LOG)
		| ((Bit64) ThermStatus.PROCHOT_Log	<< LSHIFT_PROCHOT_LOG)
		| ((Bit64) ThermStatus.CriticalTemp_Log << LSHIFT_CRITIC_LOG)
		| ((Bit64) ThermStatus.Threshold1_Log	<< LSHIFT_THOLD1_LOG)
		| ((Bit64) ThermStatus.Threshold2_Log	<< LSHIFT_THOLD2_LOG)
		| ((Bit64) ThermStatus.PwrLimit_Log	<< LSHIFT_POWER_LIMIT)
		| ((Bit64) ThermStatus.CurLimit_Log	<< LSHIFT_CURRENT_LIMIT)
		| ((Bit64) ThermStatus.XDomLimit_Log	<< LSHIFT_CROSS_DOMAIN);

	Core->PowerThermal.Events[eSTS] = \
		  ((Bit64) ThermStatus.Thermal_Status	<< LSHIFT_THERMAL_STS)
		| ((Bit64) ThermStatus.PROCHOT_Event	<< LSHIFT_PROCHOT_STS)
		| ((Bit64) ThermStatus.CriticalTemp	<< LSHIFT_CRITIC_TMP)
		| ((Bit64) ThermStatus.Threshold1	<< LSHIFT_THOLD1_STS)
		| ((Bit64) ThermStatus.Threshold2	<< LSHIFT_THOLD2_STS);
}

#define Pkg_Intel_Temp(Pkg)						\
({									\
    if (Pkg->Features.Power.EAX.PTM)					\
    {									\
	THERM_STATUS ThermStatus = {.value = 0};			\
	RDMSR(ThermStatus, MSR_IA32_PACKAGE_THERM_STATUS);		\
	Pkg->PowerThermal.Sensor = ThermStatus.DTS;			\
									\
	Pkg->PowerThermal.Events[eLOG] =				\
	  ((Bit64) ThermStatus.Thermal_Log	<< LSHIFT_THERMAL_LOG)	\
	| ((Bit64) ThermStatus.PROCHOT_Log	<< LSHIFT_PROCHOT_LOG)	\
	| ((Bit64) ThermStatus.CriticalTemp_Log << LSHIFT_CRITIC_LOG)	\
	| ((Bit64) ThermStatus.Threshold1_Log	<< LSHIFT_THOLD1_LOG)	\
	| ((Bit64) ThermStatus.Threshold2_Log	<< LSHIFT_THOLD2_LOG)	\
	| ((Bit64) ThermStatus.PwrLimit_Log	<< LSHIFT_POWER_LIMIT); \
									\
	Pkg->PowerThermal.Events[eSTS] =				\
	  ((Bit64) ThermStatus.Thermal_Status	<< LSHIFT_THERMAL_STS)	\
	| ((Bit64) ThermStatus.PROCHOT_Event	<< LSHIFT_PROCHOT_STS)	\
	| ((Bit64) ThermStatus.CriticalTemp	<< LSHIFT_CRITIC_TMP)	\
	| ((Bit64) ThermStatus.Threshold1	<< LSHIFT_THOLD1_STS)	\
	| ((Bit64) ThermStatus.Threshold2	<< LSHIFT_THOLD2_STS);	\
    }									\
})

static void Monitor_CorePerfLimitReasons(PROC_RO *Pkg)
{
	CORE_PERF_LIMIT_REASONS limit = {.value = 0};
	RDMSR(limit, MSR_SKL_CORE_PERF_LIMIT_REASONS);

	Pkg->PowerThermal.Events[eLOG] |= (
		  ((Bit64) limit.PROCHOT_Log	<< LSHIFT_CORE_HOT_LOG)
		| ((Bit64) limit.Thermal_Log	<< LSHIFT_CORE_THM_LOG)
		| ((Bit64) limit.Residency_Log	<< LSHIFT_CORE_RES_LOG)
		| ((Bit64) limit.AvgThmLimitLog << LSHIFT_CORE_AVG_LOG)
		| ((Bit64) limit.VR_ThmAlertLog << LSHIFT_CORE_VRT_LOG)
		| ((Bit64) limit.VR_TDC_Log	<< LSHIFT_CORE_TDC_LOG)
		| ((Bit64) limit.PL1_Log	<< LSHIFT_CORE_PL1_LOG)
		| ((Bit64) limit.PL2_Log	<< LSHIFT_CORE_PL2_LOG)
		| ((Bit64) limit.EDP_Log	<< LSHIFT_CORE_EDP_LOG)
		| ((Bit64) limit.TurboLimitLog	<< LSHIFT_CORE_BST_LOG)
		| ((Bit64) limit.TurboAttenLog	<< LSHIFT_CORE_ATT_LOG)
		| ((Bit64) limit.TVB_Log	<< LSHIFT_CORE_TVB_LOG)
	);

	Pkg->PowerThermal.Events[eSTS] |= (
		  ((Bit64) limit.Thermal_Status << LSHIFT_CORE_THM_STS)
		| ((Bit64) limit.PROCHOT_Event	<< LSHIFT_CORE_HOT_STS)
		| ((Bit64) limit.Residency_Sts	<< LSHIFT_CORE_RES_STS)
		| ((Bit64) limit.AvgThmLimit	<< LSHIFT_CORE_AVG_STS)
		| ((Bit64) limit.VR_ThmAlert	<< LSHIFT_CORE_VRT_STS)
		| ((Bit64) limit.VR_TDC_Status	<< LSHIFT_CORE_TDC_STS)
		| ((Bit64) limit.PL1_Status	<< LSHIFT_CORE_PL1_STS)
		| ((Bit64) limit.PL2_Status	<< LSHIFT_CORE_PL2_STS)
		| ((Bit64) limit.EDP_Status	<< LSHIFT_CORE_EDP_STS)
		| ((Bit64) limit.TurboLimit	<< LSHIFT_CORE_BST_STS)
		| ((Bit64) limit.TurboAtten	<< LSHIFT_CORE_ATT_STS)
		| ((Bit64) limit.TVB_Status	<< LSHIFT_CORE_TVB_STS)
	);
}

static void Monitor_GraphicsPerfLimitReasons(PROC_RO *Pkg)
{
	GRAPHICS_PERF_LIMIT_REASONS limit = {.value = 0};
	RDMSR(limit, MSR_GRAPHICS_PERF_LIMIT_REASONS);

	Pkg->PowerThermal.Events[eLOG] |= (
		  ((Bit64) limit.PROCHOT_Log	<< LSHIFT_GFX_HOT_LOG)
		| ((Bit64) limit.Thermal_Log	<< LSHIFT_GFX_THM_LOG)
		| ((Bit64) limit.AvgThmLimitLog << LSHIFT_GFX_AVG_LOG)
		| ((Bit64) limit.VR_ThmAlertLog << LSHIFT_GFX_VRT_LOG)
		| ((Bit64) limit.VR_TDC_Log	<< LSHIFT_GFX_TDC_LOG)
		| ((Bit64) limit.PL1_Log	<< LSHIFT_GFX_PL1_LOG)
		| ((Bit64) limit.PL2_Log	<< LSHIFT_GFX_PL2_LOG)
		| ((Bit64) limit.EDP_Log	<< LSHIFT_GFX_EDP_LOG)
		| ((Bit64) limit.InefficiencyLog<< LSHIFT_GFX_EFF_LOG)
	);

	Pkg->PowerThermal.Events[eSTS] |= (
		  ((Bit64) limit.Thermal_Status << LSHIFT_GFX_THM_STS)
		| ((Bit64) limit.PROCHOT_Event	<< LSHIFT_GFX_HOT_STS)
		| ((Bit64) limit.AvgThmLimit	<< LSHIFT_GFX_AVG_STS)
		| ((Bit64) limit.VR_ThmAlert	<< LSHIFT_GFX_VRT_STS)
		| ((Bit64) limit.VR_TDC_Status	<< LSHIFT_GFX_TDC_STS)
		| ((Bit64) limit.PL1_Status	<< LSHIFT_GFX_PL1_STS)
		| ((Bit64) limit.PL2_Status	<< LSHIFT_GFX_PL2_STS)
		| ((Bit64) limit.EDP_Status	<< LSHIFT_GFX_EDP_STS)
		| ((Bit64) limit.Inefficiency	<< LSHIFT_GFX_EFF_STS)
	);
}

static void Monitor_RingPerfLimitReasons(PROC_RO *Pkg)
{
	RING_PERF_LIMIT_REASONS limit = {.value = 0};
	RDMSR(limit, MSR_RING_PERF_LIMIT_REASONS);

	Pkg->PowerThermal.Events[eLOG] |= (
		  ((Bit64) limit.PROCHOT_Log	<< LSHIFT_RING_HOT_LOG)
		| ((Bit64) limit.Thermal_Log	<< LSHIFT_RING_THM_LOG)
		| ((Bit64) limit.AvgThmLimitLog << LSHIFT_RING_AVG_LOG)
		| ((Bit64) limit.VR_ThmAlertLog << LSHIFT_RING_VRT_LOG)
		| ((Bit64) limit.VR_TDC_Log	<< LSHIFT_RING_TDC_LOG)
		| ((Bit64) limit.PL1_Log	<< LSHIFT_RING_PL1_LOG)
		| ((Bit64) limit.PL2_Log	<< LSHIFT_RING_PL2_LOG)
		| ((Bit64) limit.EDP_Log	<< LSHIFT_RING_EDP_LOG)
	);

	Pkg->PowerThermal.Events[eSTS] |= (
		  ((Bit64) limit.Thermal_Status << LSHIFT_RING_THM_STS)
		| ((Bit64) limit.PROCHOT_Event	<< LSHIFT_RING_HOT_STS)
		| ((Bit64) limit.AvgThmLimit	<< LSHIFT_RING_AVG_STS)
		| ((Bit64) limit.VR_ThmAlert	<< LSHIFT_RING_VRT_STS)
		| ((Bit64) limit.VR_TDC_Status	<< LSHIFT_RING_TDC_STS)
		| ((Bit64) limit.PL1_Status	<< LSHIFT_RING_PL1_STS)
		| ((Bit64) limit.PL2_Status	<< LSHIFT_RING_PL2_STS)
		| ((Bit64) limit.EDP_Status	<< LSHIFT_RING_EDP_STS)
	);
}

static void Core_AMD_Family_0Fh_Temp(CORE_RO *Core)
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

		Core->PowerThermal.Events[eSTS] = \
			(Bit64)ThermTrip.SensorTrip << LSHIFT_THERMAL_STS;
	}
}

static void Core_AMD_Family_15h_00h_Temp(CORE_RO *Core)
{
	TCTL_REGISTER TctlSensor = {.value = 0};

	RDPCI(TctlSensor, PCI_AMD_TEMPERATURE_TCTL);
	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		RDPCI(ThermTrip, PCI_AMD_THERMTRIP_STATUS);

		Core->PowerThermal.Events[eSTS] = \
			(Bit64)ThermTrip.SensorTrip << LSHIFT_THERMAL_STS;
	}
}

/*	Bulldozer/Excavator through SMU interface			*/
static void Core_AMD_Family_15_60h_Temp(CORE_RO *Core)
{
	TCTL_REGISTER TctlSensor = {.value = 0};

	PCI_AMD_SMN_Read(	TctlSensor,
				SMU_AMD_THM_TCTL_REGISTER_F15H,
				SMU_AMD_INDEX_REGISTER_F15H,
				SMU_AMD_DATA_REGISTER_F15H );

	Core->PowerThermal.Sensor = TctlSensor.CurTmp;

	if (PUBLIC(RO(Proc))->Features.AdvPower.EDX.TTP == 1) {
		THERMTRIP_STATUS ThermTrip = {0};

		WRPCI(	SMU_AMD_THM_TRIP_REGISTER_F15H,
			SMU_AMD_INDEX_REGISTER_F15H);
		RDPCI(ThermTrip, SMU_AMD_DATA_REGISTER_F15H);

		Core->PowerThermal.Events[eSTS] = \
			(Bit64)ThermTrip.SensorTrip << LSHIFT_THERMAL_STS;
	}
}

static void Core_AMD_Family_15h_Temp(CORE_RO *Core)
{
	switch (PUBLIC(RO(Proc))->Features.Std.EAX.ExtModel) {
	case 0x6:
	case 0x7:
		if ((PUBLIC(RO(Proc))->Features.Std.EAX.Model >= 0x0)
		 && (PUBLIC(RO(Proc))->Features.Std.EAX.Model <= 0xf)) {
			Core_AMD_Family_15_60h_Temp(Core);
		} else {
			Core_AMD_Family_15h_00h_Temp(Core);
		}
		break;
	default:
		Core_AMD_Family_15h_00h_Temp(Core);
		break;
	}
}

static void Core_AMD_Family_17h_ThermTrip(CORE_RO *Core)
{
	TCTL_THERM_TRIP ThermTrip = {.value = 0};

	Core_AMD_SMN_Read(	ThermTrip,
				SMU_AMD_THM_TCTL_REGISTER_F17H + 0x8,
				PRIVATE(OF(Zen)).Device.DF );

	if (ThermTrip.THERM_TP_EN) {
		Core->PowerThermal.Events[eSTS] = \
				(Bit64)ThermTrip.THERM_TP << LSHIFT_THERMAL_STS;
	}
	Core->PowerThermal.Events[eSTS] |= \
			(Bit64)ThermTrip.CTF_THRESHOLD << LSHIFT_CRITIC_TMP;
}

static void Core_AMD_Zen_Filter_Temp( CORE_RO *Core,	unsigned int CurTmp,
							bool scaleRangeSel )
{
	/* Register: SMU::THM::THM_TCON_CUR_TMP - Bit 19: CUR_TEMP_RANGE_SEL
		0 = Report on 0C to 225C scale range.
		1 = Report on -49C to 206C scale range.
	*/
	Core->PowerThermal.Sensor = CurTmp;

	if (scaleRangeSel == true) {
	    if (Core->PowerThermal.Sensor >= (49 << 3))
	    {
		Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = 49;

		if (Core->PowerThermal.Sensor > (255 << 3)) {
			Core->PowerThermal.Events[eLOG] |= EVENT_THOLD2_LOG;
		}
	    } else {
		Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = 0;

		Core->PowerThermal.Events[eLOG] |= EVENT_THOLD1_LOG;
	    }
	} else {
		Core->PowerThermal.Param.Offset[THERMAL_OFFSET_P1] = 0;

		if (Core->PowerThermal.Sensor > (225 << 3)) {
			Core->PowerThermal.Events[eLOG] |= EVENT_THOLD2_LOG;
		}
	}
}

static void CTL_AMD_Family_17h_Temp(CORE_RO *Core)
{
	TCTL_REGISTER TctlSensor = {.value = 0};

	Core_AMD_SMN_Read(	TctlSensor,
				SMU_AMD_THM_TCTL_REGISTER_F17H,
				PRIVATE(OF(Zen)).Device.DF );

	Core_AMD_Zen_Filter_Temp( Core, TctlSensor.CurTmp,
					TctlSensor.CurTempRangeSel == 1 );

	Core_AMD_Family_17h_ThermTrip(Core);
}

static void CCD_AMD_Family_17h_Zen2_Temp(CORE_RO *Core)
{
	TCCD_REGISTER TccdSensor = {.value = 0};

	Core_AMD_SMN_Read(	TccdSensor,
				(SMU_AMD_THM_TCTL_CCD_REGISTER_F17H
				+ (Core->T.Cluster.CCD << 2)),
				PRIVATE(OF(Zen)).Device.DF );

	Core_AMD_Zen_Filter_Temp( Core, TccdSensor.CurTmp,
					TccdSensor.CurTempRangeSel == 1 );

	Core_AMD_Family_17h_ThermTrip(Core);
}

#define Pkg_AMD_Family_17h_Temp(Pkg, Core)				\
({									\
	Core_AMD_Family_17h_Temp(Core);					\
									\
	Pkg->PowerThermal.Sensor = Core->PowerThermal.Sensor;		\
})

static void CCD_AMD_Family_19h_Genoa_Temp(CORE_RO *Core)
{
	TCCD_REGISTER TccdSensor = {.value = 0};

	Core_AMD_SMN_Read(	TccdSensor,
				(SMU_AMD_THM_TCTL_CCD_REGISTER_F19H_11H
				+ (Core->T.Cluster.CCD << 2)),
				PRIVATE(OF(Zen)).Device.DF );

	Core_AMD_Zen_Filter_Temp( Core, TccdSensor.CurTmp,
					TccdSensor.CurTempRangeSel == 1 );

	Core_AMD_Family_17h_ThermTrip(Core);
}

static void CCD_AMD_Family_19h_Zen4_Temp(CORE_RO *Core)
{
	TCCD_REGISTER TccdSensor = {.value = 0};

	Core_AMD_SMN_Read(	TccdSensor,
				(SMU_AMD_THM_TCTL_CCD_REGISTER_F19H_61H
				+ (Core->T.Cluster.CCD << 2)),
				PRIVATE(OF(Zen)).Device.DF );

	Core_AMD_Zen_Filter_Temp( Core, TccdSensor.CurTmp,
					TccdSensor.CurTempRangeSel == 1 );

	Core_AMD_Family_17h_ThermTrip(Core);
}


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
	if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

		return HRTIMER_RESTART;
	} else
		return HRTIMER_NORESTART;
}

static void InitTimer_VirtualMachine(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_VirtualMachine, 1);
}

static void Start_VirtualMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static enum hrtimer_restart Cycle_Safe_VirtualMachine(struct hrtimer *pTimer)
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

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		return HRTIMER_RESTART;
	} else
		return HRTIMER_NORESTART;
}

static void InitTimer_Safe_VirtualMachine(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Safe_VirtualMachine, 1);
}

static void Start_Safe_VirtualMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_VirtualMachine(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
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
	if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

		return HRTIMER_RESTART;
	} else
		return HRTIMER_NORESTART;
}

static void InitTimer_GenuineIntel(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_GenuineIntel, 1);
}

static void Start_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_GenuineIntel(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
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

		Delta_C0(Core);

		Delta_TSC_OVH(Core);

		Delta_C1(Core);

		Save_TSC(Core);

		Save_C0(Core);

		Save_C1(Core);

		BITSET(LOCKLESS, PUBLIC(RW(Core, AT(cpu)))->Sync.V, NTFY);

		return HRTIMER_RESTART;
	} else
		return HRTIMER_NORESTART;
}

static void InitTimer_AuthenticAMD(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AuthenticAMD, 1);
}

static void Start_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_AuthenticAMD(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
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
    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Core2(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Core2, 1);
}

static void Start_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	Counters_Core2(Core, 0);

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

static void Stop_Core2(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static enum hrtimer_restart Cycle_Silvermont(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

  if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
  } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Silvermont(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Silvermont, 1);
}

static void Start_Silvermont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Silvermont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static enum hrtimer_restart Cycle_Nehalem(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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
			HWM_SIO_CPUVCORE,
			HWM_SIO_INDEX_PORT, HWM_SIO_DATA_PORT );
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
		RDSIO(	Core->PowerThermal.VID, HWM_SIO_CPUVCORE,
			HWM_SIO_INDEX_PORT, HWM_SIO_DATA_PORT );
	    #endif
	    }
		break;
	case FORMULA_SCOPE_SMT:
	    #if defined(HWM_CHIPSET) \
	    && ((HWM_CHIPSET == W83627) || (HWM_CHIPSET == IT8720))
		RDSIO(	Core->PowerThermal.VID, HWM_SIO_CPUVCORE,
			HWM_SIO_INDEX_PORT, HWM_SIO_DATA_PORT );
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Nehalem(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Nehalem, 1);
}

static void Start_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_Nehalem(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Nehalem(Core, 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Nehalem(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_Nehalem(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Set(NHM);
}

static void Stop_Uncore_Nehalem(void *arg)
{
	UNUSED(arg);

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

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_SandyBridge(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_SandyBridge, 1);
}

static void Start_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_SandyBridge(Core, 0);
		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_SandyBridge(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_SandyBridge(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Set(SNB);
}

static void Stop_Uncore_SandyBridge(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Clear(SNB);
}


static enum hrtimer_restart Cycle_Intel_Xeon_EP(struct hrtimer *pTimer,
			void (*PKG_Counters_Intel_EP)(CORE_RO*, unsigned int))
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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
		PKG_Counters_Intel_EP(Core, 1);

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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart Cycle_SandyBridge_EP(struct hrtimer *pTimer)
{
	return Cycle_Intel_Xeon_EP(pTimer, PKG_Counters_SandyBridge_EP);
}

static void InitTimer_SandyBridge_EP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_SandyBridge_EP, 1);
}

static void Entry_Intel_Xeon_EP(void *arg,
			void (*PKG_Counters_Intel_EP)(CORE_RO*, unsigned int))
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Intel_EP(Core, 0);
		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_SandyBridge_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_SandyBridge_EP(void *arg)
{
	Entry_Intel_Xeon_EP(arg, PKG_Counters_SandyBridge_EP);
}

/*	SandyBridge/EP Uncore PMU MSR for Xeon family 06_2Dh		*/
static void Start_Uncore_SandyBridge_EP(void *arg)
{
	UNUSED(arg);

    if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version > 3)
    {
	/*	Tables 2-20 , 2-23 , 2-24				*/
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;

	RDMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl = \
						Uncore_FixedPerfControl;

	Uncore_FixedPerfControl.SNB.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_SNB_UNCORE_PERF_GLOBAL_CTRL);

	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl = \
						Uncore_PMonGlobalControl;

	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_SNB_UNCORE_PERF_GLOBAL_CTRL);
    }
}

static void Stop_Uncore_SandyBridge_EP(void *arg)
{
	UNUSED(arg);

  if (PUBLIC(RO(Proc))->Features.PerfMon.EAX.Version > 3)
  {
    if(PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl.SNB.EN_CTR0 == 0)
    {
	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl.Freeze_All = 1;
    }
	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl,
		MSR_SNB_UNCORE_PERF_GLOBAL_CTRL);

	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl,
		MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);
  }
}

static enum hrtimer_restart Cycle_IvyBridge_EP(struct hrtimer *pTimer)
{
	return Cycle_Intel_Xeon_EP(pTimer, PKG_Counters_IvyBridge_EP);
}

static void InitTimer_IvyBridge_EP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_IvyBridge_EP, 1);
}

static void Start_IvyBridge_EP(void *arg)
{
	Entry_Intel_Xeon_EP(arg, PKG_Counters_IvyBridge_EP);
}

/*	IvyBridge/EP Uncore PMU MSR for Xeon family 06_3E		*/
static void Start_Uncore_IvyBridge_EP(void *arg)
{	/*	Tables 2-20 , 2-24 , 2-26 , 2-27 , 2-28 		*/
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;
	UNUSED(arg);

	RDMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl = \
						Uncore_FixedPerfControl;

	Uncore_FixedPerfControl.SNB.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_IVB_EP_PMON_GLOBAL_CTRL);

	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl = \
						Uncore_PMonGlobalControl;

	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_IVB_EP_PMON_GLOBAL_CTRL);
}

static void Stop_Uncore_IvyBridge_EP(void *arg)
{
	UNUSED(arg);
	/* If fixed counter was disable at entry, force freezing	*/
  if (PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl.SNB.EN_CTR0 == 0)
  {
	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl.Freeze_All = 1;
  }
	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl,
		MSR_IVB_EP_PMON_GLOBAL_CTRL);

	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl,
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

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Haswell_ULT(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Haswell_ULT, 1);
}

static void Start_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Haswell_ULT(Core, 0);
		PWR_ACCU_SandyBridge(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Haswell_ULT(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_Haswell_ULT(void *arg)
{
	UNUSED(arg);

    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	Uncore_Counters_Set(SNB);
    }
}

static void Stop_Uncore_Haswell_ULT(void *arg)
{
	UNUSED(arg);

    if (PUBLIC(RO(Proc))->Registration.Experimental) {
	Uncore_Counters_Clear(SNB);
    }
}


static enum hrtimer_restart Cycle_Goldmont(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	Counters_Goldmont(Core, 1);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Goldmont(Core, 1);

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

		PWR_ACCU_Goldmont(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC10(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, UNCORE);

		Delta_PWR_ACCU(Proc, RAM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC10(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Goldmont(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Goldmont, 1);
}

static void Start_Goldmont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	Counters_Goldmont(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Goldmont(Core, 0);
		PWR_ACCU_Goldmont(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Goldmont(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}


static enum hrtimer_restart Cycle_Haswell_EP(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Haswell_EP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Haswell_EP, 1);
}

static void Start_Haswell_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Haswell_EP(Core, 0);
		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Haswell_EP(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_Haswell_EP(void *arg)
{
	UNCORE_FIXED_PERF_CONTROL Uncore_FixedPerfControl;
	UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;
	UNUSED(arg);

	RDMSR(Uncore_FixedPerfControl, MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl = \
						Uncore_FixedPerfControl;

	Uncore_FixedPerfControl.HSW_EP.EN_CTR0 = 1;

	WRMSR(Uncore_FixedPerfControl, MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL);

	RDMSR(Uncore_PMonGlobalControl, MSR_HSW_EP_PMON_GLOBAL_CTRL);

	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl = \
						Uncore_PMonGlobalControl;

	Uncore_PMonGlobalControl.Unfreeze_All = 1;

	WRMSR(Uncore_PMonGlobalControl, MSR_HSW_EP_PMON_GLOBAL_CTRL);
}

static void Stop_Uncore_Haswell_EP(void *arg)
{
	UNUSED(arg);

  if(PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl.HSW_EP.EN_CTR0 == 0)
  {
	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl.Freeze_All = 1;
  }
	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_PMonGlobalControl,
		MSR_HSW_EP_PMON_GLOBAL_CTRL );

	WRMSR(	PRIVATE(OF(SaveArea)).Intel.Uncore_FixedPerfControl,
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

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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
		Pkg_Intel_PMC_Counters(Core, 1, ARCH_PMC);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

		Monitor_CorePerfLimitReasons(PUBLIC(RO(Proc)));
		Monitor_GraphicsPerfLimitReasons(PUBLIC(RO(Proc)));
		Monitor_RingPerfLimitReasons(PUBLIC(RO(Proc)));

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

		Power_ACCU_Skylake(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_MC6(PUBLIC(RO(Proc)));

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

		Save_MC6(PUBLIC(RO(Proc)));

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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Skylake(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Skylake, 1);
}

static void Start_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Skylake(Core, 0);
		Pkg_Intel_PMC_Counters(Core, 0, ARCH_PMC);

		Power_ACCU_Skylake(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Skylake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_Skylake(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Set(SKL);

	Pkg_Intel_PMC_Set(ARCH_PMC, 0x5838);
}

static void Stop_Uncore_Skylake(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Clear(SKL);

	Pkg_Intel_PMC_Clear(ARCH_PMC);
}

static enum hrtimer_restart Cycle_Skylake_X(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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
		Pkg_Intel_PMC_Counters(Core, 1, ARCH_PMC);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

		Monitor_CorePerfLimitReasons(PUBLIC(RO(Proc)));
/*TODO(Unsolved)
		Monitor_GraphicsPerfLimitReasons(PUBLIC(RO(Proc)));
*/
		Monitor_RingPerfLimitReasons(PUBLIC(RO(Proc)));

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

		Delta_MC6(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, RAM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_MC6(PUBLIC(RO(Proc)));

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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Skylake_X(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_Skylake_X, 1);
}

static void Start_Skylake_X(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);
	SMT_Counters_SandyBridge(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Skylake_X(Core, 0);
		Pkg_Intel_PMC_Counters(Core, 0, ARCH_PMC);
		PWR_ACCU_SandyBridge_EP(PUBLIC(RO(Proc)), 0);
	}

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_Skylake_X(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	Intel_Core_Counters_Clear(Save, Core);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_Skylake_X(void *arg)
{
	UNUSED(arg);
/*TODO(Unsolved)
	Uncore_Counters_Set(SKL_X);
*/
	Pkg_Intel_PMC_Set(ARCH_PMC, 0x5838);
}

static void Stop_Uncore_Skylake_X(void *arg)
{
	UNUSED(arg);
/*TODO(Unsolved)
	Uncore_Counters_Clear(SKL_X);
*/
	Pkg_Intel_PMC_Clear(ARCH_PMC);
}


static enum hrtimer_restart Cycle_Alderlake_Ecore(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_Alderlake_Ecore(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Hybrid)
	{
		PKG_Counters_Alderlake_Ecore(Core, 1);

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

		Delta_PC04(PUBLIC(RO(Proc)));

		Delta_PC10(PUBLIC(RO(Proc)));

		Delta_MC6(PUBLIC(RO(Proc)));

		Save_PC04(PUBLIC(RO(Proc)));

		Save_PC10(PUBLIC(RO(Proc)));

		Save_MC6(PUBLIC(RO(Proc)));
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart Cycle_Alderlake_Pcore(struct hrtimer *pTimer)
{
	PERF_STATUS PerfStatus = {.value = 0};
	CORE_RO *Core;
	unsigned int cpu;

	cpu = smp_processor_id();
	Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	Mark_OVH(Core);

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
    {
	hrtimer_forward(pTimer,
			hrtimer_cb_get_time(pTimer),
			RearmTheTimer);

	SMT_Counters_Alderlake_Pcore(Core, 1);

	RDMSR(Core->PowerThermal.PerfControl, MSR_IA32_PERF_CTL);
	Core->Boost[BOOST(TGT)] = GET_SANDYBRIDGE_TARGET(Core);

	RDMSR(PerfStatus, MSR_IA32_PERF_STATUS);
	Core->Ratio.Perf = PerfStatus.SNB.CurrentRatio;

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		PKG_Counters_Alderlake_Pcore(Core, 1);

		Pkg_Intel_Temp(PUBLIC(RO(Proc)));

		Monitor_CorePerfLimitReasons(PUBLIC(RO(Proc)));
		Monitor_GraphicsPerfLimitReasons(PUBLIC(RO(Proc)));
		Monitor_RingPerfLimitReasons(PUBLIC(RO(Proc)));

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

		PWR_ACCU_Alderlake(PUBLIC(RO(Proc)), 1);

		Delta_PC02(PUBLIC(RO(Proc)));

		Delta_PC03(PUBLIC(RO(Proc)));

		Delta_PC06(PUBLIC(RO(Proc)));

		Delta_PC07(PUBLIC(RO(Proc)));

		Delta_PC08(PUBLIC(RO(Proc)));

		Delta_PC09(PUBLIC(RO(Proc)));

		Delta_PTSC_OVH(PUBLIC(RO(Proc)), Core);

		Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

		Delta_PWR_ACCU(Proc, PKG);

		Delta_PWR_ACCU(Proc, CORES);

		Delta_PWR_ACCU(Proc, UNCORE);

		Delta_PWR_ACCU(Proc, RAM);

		Delta_PWR_ACCU(Proc, PLATFORM);

		Save_PC02(PUBLIC(RO(Proc)));

		Save_PC03(PUBLIC(RO(Proc)));

		Save_PC06(PUBLIC(RO(Proc)));

		Save_PC07(PUBLIC(RO(Proc)));

		Save_PC08(PUBLIC(RO(Proc)));

		Save_PC09(PUBLIC(RO(Proc)));

		Save_PTSC(PUBLIC(RO(Proc)));

		Save_UNCORE_FC0(PUBLIC(RO(Proc)));

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PKG);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), CORES);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), UNCORE);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), RAM);

		Save_PWR_ACCU(PUBLIC(RO(Proc)), PLATFORM);

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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_Alderlake(unsigned int cpu)
{
    switch (PUBLIC(RO(Core, AT(cpu)))->T.Cluster.Hybrid.CoreType) {
    case Hybrid_Atom:
	smp_call_function_single(cpu, InitTimer, Cycle_Alderlake_Ecore, 1);
	break;
    case Hybrid_Core:
    case Hybrid_RSVD1:
    case Hybrid_RSVD2:
    default:
	smp_call_function_single(cpu, InitTimer, Cycle_Alderlake_Pcore, 1);
	break;
    }
}

static void Start_Alderlake(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	Intel_Core_Counters_Set(Save, Core);

    switch (PUBLIC(RO(Core, AT(cpu)))->T.Cluster.Hybrid.CoreType) {
    case Hybrid_Atom:
	SMT_Counters_Alderlake_Ecore(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Hybrid)
	{
		PKG_Counters_Alderlake_Ecore(Core, 0);
	}
	break;
    case Hybrid_Core:
    case Hybrid_RSVD1:
    case Hybrid_RSVD2:
    default:
	SMT_Counters_Alderlake_Pcore(Core, 0);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
		}
		PKG_Counters_Alderlake_Pcore(Core, 0);

		PWR_ACCU_Alderlake(PUBLIC(RO(Proc)), 0);
	}
	break;
    }

	RDCOUNTER(Core->Interrupt.SMI, MSR_SMI_COUNT);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_Alderlake(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Set(ADL);
}

static void Stop_Uncore_Alderlake(void *arg)
{
	UNUSED(arg);

	Uncore_Counters_Clear(ADL);
}


static enum hrtimer_restart Cycle_AMD_Family_0Fh(struct hrtimer *pTimer)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_AMD_Family_0Fh(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_0Fh, 1);
}

static void Start_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

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

static void Stop_AMD_Family_0Fh(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_AMD_Family_10h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core) {
		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_AMD_Family_11h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_AMD_Family_12h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_AMD_Family_14h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
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

    if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
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

	return HRTIMER_RESTART;
    } else
	return HRTIMER_NORESTART;
}

static void InitTimer_AMD_Family_15h(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Family_15h, 1);
}

static void Start_AMD_Family_15h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	UNUSED(arg);

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

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Cycle_AMD_Family_17h(CORE_RO *Core,
			void (*Call_SMU)(const unsigned int, const unsigned int,
					const unsigned long long),
			const unsigned int plane0, const unsigned int plane1,
			const unsigned long long factor)
{
	PSTATEDEF PstateDef;
	PSTATECTRL PstateCtrl;
	COF_ST COF;

	SMT_Counters_AMD_Family_17h(Core, 1);

	/*		Read the Target P-State.			*/
	RDMSR(PstateCtrl, MSR_AMD_PERF_CTL);
	RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + PstateCtrl.PstateCmd);

	COF = AMD_Zen_CoreCOF(PstateDef);

	PUBLIC(RO(Core, AT(Core->Bind)))->Boost[BOOST(TGT)] = COF.Q;

	/*	Read the Boosted Frequency and voltage VID.		*/
	RDMSR(PstateDef, MSR_AMD_F17H_HW_PSTATE_STATUS);
	COF = AMD_Zen_CoreCOF(PstateDef);
	Core->Ratio.COF = COF;

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	Pkg_AMD_Zen_PMC_Counters(PUBLIC(RO(Proc)), Core, 1, ARCH_PMC,
		Pkg_AMD_Zen_PMC_Closure(PUBLIC(RO(Proc)), Core, 1, ARCH_PMC)
	);

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

	RDCOUNTER( PUBLIC(RO(Proc))->Counter[1].Power.ACCU[PWR_DOMAIN(PKG)],
			MSR_AMD_PKG_ENERGY_STATUS );

	Delta_UNCORE_FC0(PUBLIC(RO(Proc)));

	Delta_PWR_ACCU(Proc, PKG);

	Save_UNCORE_FC0(PUBLIC(RO(Proc)));

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
		fallthrough;
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
		fallthrough;
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

	AMD_Zen_PMC_Counters(Core, 1, ARCH_PMC,
		AMD_Zen_PMC_Closure(Core, 1, ARCH_PMC)
	);

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

inline void SoC_RAPL(AMD_17_SVI SVI, const unsigned long long factor)
{
	unsigned long long VCC, ICC, ACCU;
	/*	PLATFORM RAPL workaround to provide the SoC power	*/
	VCC = 155000LLU - (625LLU * SVI.VID);
	ICC = SVI.IDD * factor;
	ACCU = VCC * ICC;
	ACCU = ACCU << PUBLIC(RO(Proc))->PowerThermal.Unit.ESU;
	ACCU = ACCU / (100000LLU * 1000000LLU);
	ACCU = (PUBLIC(RO(Proc))->SleepInterval * ACCU) / 1000LLU;
	PUBLIC(RW(Proc))->Delta.Power.ACCU[PWR_DOMAIN(UNCORE)] = ACCU;
}

static void Call_SVI(	const unsigned int plane0, const unsigned int plane1,
			const unsigned long long factor )
{
	AMD_17_SVI SVI = {.value = 0};

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_F17H_SVI(plane0),
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->PowerThermal.VID.CPU = SVI.VID;

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_F17H_SVI(plane1),
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->PowerThermal.VID.SOC = SVI.VID;

	SoC_RAPL(SVI, factor);
}

static void Call_SVI_APU(const unsigned int plane0, const unsigned int plane1,
			const unsigned long long factor)
{
	AMD_17_SVI SVI = {.value = 0};

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_F17_60H_SVI(plane0),
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->PowerThermal.VID.CPU = SVI.VID;

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_F17_60H_SVI(plane1),
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->PowerThermal.VID.SOC = SVI.VID;

	SoC_RAPL(SVI, factor);
}

static void Call_SVI_RMB(const unsigned int plane0, const unsigned int plane1,
			const unsigned long long factor)
{
	AMD_RMB_SVI SVI = {.value = 0};
	UNUSED(factor);

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_RMB_SVI(plane0),
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->PowerThermal.VID.CPU = SVI.SVI1;

	Core_AMD_SMN_Read(	SVI,
				SMU_AMD_RMB_SVI(plane1),
				PRIVATE(OF(Zen)).Device.DF );

	PUBLIC(RO(Proc))->PowerThermal.VID.SOC = SVI.SVI0;
}

static void Call_DFLT(	const unsigned int plane0, const unsigned int plane1,
			const unsigned long long factor )
{
	UNUSED(plane0);
	UNUSED(plane1);
	UNUSED(factor);

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

	if (BITVAL(PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD) == 1)
	{
		hrtimer_forward(pTimer,
				hrtimer_cb_get_time(pTimer),
				RearmTheTimer);

		Cycle_AMD_Family_17h(Core, Call_SMU, plane0, plane1, factor);

		return HRTIMER_RESTART;
	} else
		return HRTIMER_NORESTART;
}

static enum hrtimer_restart Cycle_AMD_F17h_Zen(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_SVI, 0, 1, 360772LLU);
}
static enum hrtimer_restart Cycle_AMD_F17h_Zen2_SP(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_SVI, 1, 0, 294300LLU);
}
static enum hrtimer_restart Cycle_AMD_F17h_Zen2_MP(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_SVI, 2, 1, 294300LLU);
}
static enum hrtimer_restart Cycle_AMD_F17h_Zen2_APU(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_SVI_APU, 0, 1, 294300LLU);
}
static enum hrtimer_restart Cycle_AMD_Zen3Plus_RMB(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_SVI_RMB, 0, 2, 0LLU);
}
static enum hrtimer_restart Cycle_AMD_Zen4_RPL(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_DFLT, 0, 0, 0LLU);
}
static enum hrtimer_restart Cycle_AMD_F17h(struct hrtimer *pTimer)
{
	return Entry_AMD_F17h(pTimer, Call_DFLT, 0, 0, 0LLU);
}

static void InitTimer_AMD_Family_17h(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h, 1);
}

static void InitTimer_AMD_F17h_Zen(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen, 1);
}

static void InitTimer_AMD_F17h_Zen2_SP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen2_SP, 1);
}

static void InitTimer_AMD_F17h_Zen2_MP(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen2_MP, 1);
}

static void InitTimer_AMD_F17h_Zen2_APU(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_F17h_Zen2_APU, 1);
}

static void InitTimer_AMD_Zen3Plus_RMB(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Zen3Plus_RMB, 1);
}

static void InitTimer_AMD_Zen4_RPL(unsigned int cpu)
{
	smp_call_function_single(cpu, InitTimer, Cycle_AMD_Zen4_RPL, 1);
}

static void Start_AMD_Family_17h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

#ifdef CONFIG_PM_SLEEP
	if (CoreFreqK.ResumeFromSuspend == true) {
		WRCOUNTER(0x0LLU, MSR_IA32_MPERF);
		WRCOUNTER(0x0LLU, MSR_IA32_APERF);
	}
#endif /* CONFIG_PM_SLEEP */

	if (Arch[PUBLIC(RO(Proc))->ArchID].Update != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Update(Core);
	}

	AMD_Core_Counters_Set(Save, Core, Family_17h);

	AMD_Zen_PMC_Set(Save, Core, ARCH_PMC);

	SMT_Counters_AMD_Family_17h(Core, 0);

    if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
    {
	Pkg_AMD_Zen_PMC_Set(Core, ARCH_PMC);

	if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Start(NULL);
	}
	Pkg_AMD_Zen_PMC_Counters(PUBLIC(RO(Proc)), Core, 0, ARCH_PMC);

	RDCOUNTER(PUBLIC(RO(Proc))->Counter[0].Power.ACCU[PWR_DOMAIN(PKG)],
			MSR_AMD_PKG_ENERGY_STATUS );
    }
    if (Core->T.ThreadID == 0)
    {
	RDCOUNTER(Core->Counter[0].Power.ACCU,MSR_AMD_PP0_ENERGY_STATUS);
	Core->Counter[0].Power.ACCU &= 0xffffffff;
    }

	AMD_Zen_PMC_Counters(Core, 0, ARCH_PMC);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_start(	&PRIVATE(OF(Core, AT(cpu)))->Join.Timer,
			RearmTheTimer,
			HRTIMER_MODE_REL_PINNED);

	BITSET(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Stop_AMD_Family_17h(void *arg)
{
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	union SAVE_AREA_CORE *Save = &PRIVATE(OF(Core, AT(cpu)))->SaveArea;
	UNUSED(arg);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, MUSTFWD);

	hrtimer_cancel(&PRIVATE(OF(Core, AT(cpu)))->Join.Timer);

	AMD_Core_Counters_Clear(Save, Core);

	AMD_Zen_PMC_Clear(Save, Core, ARCH_PMC);

	if (Core->Bind == PUBLIC(RO(Proc))->Service.Core)
	{
		Pkg_AMD_Zen_PMC_Clear(Core, ARCH_PMC);

		if (Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop != NULL) {
			Arch[PUBLIC(RO(Proc))->ArchID].Uncore.Stop(NULL);
		}
		Pkg_Reset_ThermalPoint(PUBLIC(RO(Proc)));
	}
	PerCore_Reset(Core);

	BITCLR(LOCKLESS, PRIVATE(OF(Core, AT(cpu)))->Join.TSM, STARTED);
}

static void Start_Uncore_AMD_Family_17h(void *arg)
{
	ZEN_DF_PERF_CTL Zen_DataFabricPerfControl;
	UNUSED(arg);

	RDMSR(Zen_DataFabricPerfControl, MSR_AMD_F17H_DF_PERF_CTL);

	PRIVATE(OF(SaveArea)).AMD.Zen_DataFabricPerfControl = \
						Zen_DataFabricPerfControl;

	Zen_DataFabricPerfControl.EventSelect00 = 0x0f;
	Zen_DataFabricPerfControl.UnitMask	= 0xff;
	Zen_DataFabricPerfControl.CounterEn	= 1;
	Zen_DataFabricPerfControl.EventSelect08 = 0x00;
	Zen_DataFabricPerfControl.EventSelect12 = 0x00;

	WRMSR(Zen_DataFabricPerfControl, MSR_AMD_F17H_DF_PERF_CTL);
}

static void Stop_Uncore_AMD_Family_17h(void *arg)
{
	UNUSED(arg);

	WRMSR(	PRIVATE(OF(SaveArea)).AMD.Zen_DataFabricPerfControl,
		MSR_AMD_F17H_DF_PERF_CTL );
}


static long Sys_OS_Driver_Query(void)
{
	int rc = RC_SUCCESS;
#ifdef CONFIG_CPU_FREQ
	const char *pFreqDriver;
	struct cpufreq_policy *pFreqPolicy;
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_CPU_IDLE
	struct cpuidle_driver *pIdleDriver;
#endif /* CONFIG_CPU_IDLE */
	memset(&PUBLIC(RO(Proc))->OS, 0, sizeof(OS_DRIVER));
#ifdef CONFIG_CPU_IDLE
    if ((pIdleDriver = cpuidle_get_driver()) != NULL)
    {
	int idx;

	StrCopy(PUBLIC(RO(Proc))->OS.IdleDriver.Name,
		pIdleDriver->name,
		CPUIDLE_NAME_LEN);

      if (pIdleDriver->state_count < CPUIDLE_STATE_MAX) {
	PUBLIC(RO(Proc))->OS.IdleDriver.stateCount = pIdleDriver->state_count;
      } else {
	PUBLIC(RO(Proc))->OS.IdleDriver.stateCount = CPUIDLE_STATE_MAX;
      }
	PUBLIC(RO(Proc))->OS.IdleDriver.stateLimit = pIdleDriver->state_count;

      for (idx = 0; idx < PUBLIC(RO(Proc))->OS.IdleDriver.stateCount; idx++)
      {
	StrCopy(PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].Name,
		pIdleDriver->states[idx].name, CPUIDLE_NAME_LEN);

	StrCopy(PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].Desc,
		pIdleDriver->states[idx].desc, CPUIDLE_NAME_LEN);

	PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].exitLatency = \
					pIdleDriver->states[idx].exit_latency;

	PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].powerUsage = \
					pIdleDriver->states[idx].power_usage;

	PUBLIC(RO(Proc))->OS.IdleDriver.State[idx].targetResidency = \
				pIdleDriver->states[idx].target_residency;
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
  if ((pFreqPolicy=kzalloc(sizeof(struct cpufreq_policy),GFP_KERNEL)) != NULL) {
    if((rc=cpufreq_get_policy(pFreqPolicy,PUBLIC(RO(Proc))->Service.Core)) == 0)
    {
	struct cpufreq_governor *pGovernor = pFreqPolicy->governor;
	if (pGovernor != NULL) {
		StrCopy(PUBLIC(RO(Proc))->OS.FreqDriver.Governor,
			pGovernor->name, CPUFREQ_NAME_LEN);
	} else {
		PUBLIC(RO(Proc))->OS.FreqDriver.Governor[0] = '\0';
	}
    } else {
	PUBLIC(RO(Proc))->OS.FreqDriver.Governor[0] = '\0';
    }
	kfree(pFreqPolicy);
  } else {
	rc = -ENOMEM;
  }
#endif /* CONFIG_CPU_FREQ */
	return rc;
}

static long Sys_Kernel(SYSGATE_RO *SysGate)
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

static long SysGate_OnDemand(void)
{
	long rc = -1;
    if (PUBLIC(OF(Gate)) == NULL)
    {	/*			On-demand allocation.			*/
	PUBLIC(OF(Gate)) = alloc_pages_exact(PUBLIC(RO(Proc))->Gate.ReqMem.Size,
						GFP_KERNEL);
	if (PUBLIC(OF(Gate)) != NULL)
	{
		memset(PUBLIC(OF(Gate)), 0, PUBLIC(RO(Proc))->Gate.ReqMem.Size);
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

#if defined(CONFIG_CPU_IDLE) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	/*			MWAIT Idle methods			*/
static int CoreFreqK_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{/*	Source: /drivers/cpuidle/cpuidle.c				*/
	unsigned long MWAIT=(CoreFreqK.IdleDriver.states[index].flags>>24)&0xff;
	UNUSED(pIdleDevice);
	UNUSED(pIdleDriver);

	__asm__ volatile
	(
		"xorq	%%rcx	,	%%rcx"	"\n\t"
		"xorq	%%rdx	,	%%rdx"	"\n\t"
		"leaq	%[addr] ,	%%rax"	"\n\t"
		"monitor"			"\n\t"
		"# Bit 0 of ECX must be set"	"\n\t"
		"movq	$0x1	,	%%rcx"	"\n\t"
		"movq	%[hint] ,	%%rax"	"\n\t"
		"mwait"
		:
		: [addr] "m" (current_thread_info()->flags),
		  [hint] "ir" (MWAIT)
		: "%rax", "%rcx", "%rdx", "memory"
	);

	return index;
}

static int CoreFreqK_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return CoreFreqK_MWAIT_Handler(pIdleDevice, pIdleDriver, index);
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
static void CoreFreqK_S2_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#else
static int CoreFreqK_S2_MWAIT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#endif /* 5.9.0 */
{
	unsigned long MWAIT=(CoreFreqK.IdleDriver.states[index].flags>>24)&0xff;
	UNUSED(pIdleDevice);
	UNUSED(pIdleDriver);

	__asm__ volatile
	(
		"xorq	%%rcx	,	%%rcx"	"\n\t"
		"xorq	%%rdx	,	%%rdx"	"\n\t"
		"leaq	%[addr] ,	%%rax"	"\n\t"
		"monitor"			"\n\t"
		"# Bit 0 of ECX must be set"	"\n\t"
		"movq	$0x1	,	%%rcx"	"\n\t"
		"movq	%[hint] ,	%%rax"	"\n\t"
		"mwait"
		:
		: [addr] "m" (current_thread_info()->flags),
		  [hint] "ir" (MWAIT)
		: "%rax", "%rcx", "%rdx", "memory"
	);

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) || (RHEL_MINOR >= 4))
	return index;
#endif /* 5.9.0 */
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
static void CoreFreqK_S2_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	CoreFreqK_S2_MWAIT_Handler(pIdleDevice, pIdleDriver, index);
}
#else
static int CoreFreqK_S2_MWAIT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return CoreFreqK_S2_MWAIT_Handler(pIdleDevice, pIdleDriver, index);
}
#endif /* 5.9.0 */
	/*			HALT Idle methods			*/
static int CoreFreqK_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	UNUSED(pIdleDevice);
	UNUSED(pIdleDriver);
/*	Source: /arch/x86/include/asm/irqflags.h: native_safe_halt();	*/
	__asm__ volatile
	(
		"sti"		"\n\t"
		"hlt"		"\n\t"
		"cli"
		:
		:
		: "cc"
	);
	return index;
}

static int CoreFreqK_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return CoreFreqK_HALT_Handler(pIdleDevice, pIdleDriver, index);
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
static void CoreFreqK_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#else
static int CoreFreqK_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#endif /* 5.9.0 */
{
	UNUSED(pIdleDevice);
	UNUSED(pIdleDriver);

	__asm__ volatile
	(
		"sti"		"\n\t"
		"hlt"		"\n\t"
		"cli"
		:
		:
		: "cc"
	);
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) || (RHEL_MINOR >= 4))
	return index;
#endif /* 5.9.0 */
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
static void CoreFreqK_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	CoreFreqK_S2_HALT_Handler(pIdleDevice, pIdleDriver, index);
}
#else
static int CoreFreqK_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return CoreFreqK_S2_HALT_Handler(pIdleDevice, pIdleDriver, index);
}
#endif /* 5.9.0 */
	/*			I/O Idle methods			*/
static int CoreFreqK_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	UNUSED(pIdleDevice);
	UNUSED(pIdleDriver);

	__asm__ volatile
	(
		"xorw	%%ax,	%%ax"	"\n\t"
		"movw	%0,	%%dx"	"\n\t"
		"inb	%%dx,	%%al"	"\n\t"
		:
		: "ir" (Core->Query.CStateBaseAddr)
		: "%ax", "%dx"
	);
	return index;
}

static int CoreFreqK_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return CoreFreqK_IO_Handler(pIdleDevice, pIdleDriver, index);
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
static void CoreFreqK_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#else
static int CoreFreqK_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
#endif /* 5.9.0 */
{
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

	UNUSED(pIdleDevice);
	UNUSED(pIdleDriver);

	__asm__ volatile
	(
		"xorw	%%ax,	%%ax"	"\n\t"
		"movw	%0,	%%dx"	"\n\t"
		"inb	%%dx,	%%al"	"\n\t"
		:
		: "ir" (Core->Query.CStateBaseAddr)
		: "%ax", "%dx"
	);
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) || (RHEL_MINOR >= 4))
	return index;
#endif /* 5.9.0 */
}

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
static void CoreFreqK_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	CoreFreqK_S2_IO_Handler(pIdleDevice, pIdleDriver, index);
}
#else
static int CoreFreqK_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
	return CoreFreqK_S2_IO_Handler(pIdleDevice, pIdleDriver, index);
}
#endif /* 5.9.0 */
	/*		Idle Cycles callback functions			*/
static int Alternative_Computation_Of_Cycles(
	int (*Handler)(struct cpuidle_device*, struct cpuidle_driver*, int),
			struct cpuidle_device *pIdleDevice,
			struct cpuidle_driver *pIdleDriver, int index
)
{
	unsigned long long TSC[3] __attribute__ ((aligned (8)));
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	const unsigned short lvl = \
			(CoreFreqK.IdleDriver.states[index].flags >> 28) & 0xf;

	if ((PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (PUBLIC(RO(Proc))->Features.ExtInfo.EDX.RDTSCP == 1))
	{
		RDTSCP64(TSC[0]);

		RDTSCP64(TSC[1]);

		Handler(pIdleDevice, pIdleDriver, index);

		RDTSCP64(TSC[2]);
	}
	else
	{
		RDTSC64(TSC[0]);

		RDTSCP64(TSC[1]);

		Handler(pIdleDevice, pIdleDriver, index);

		RDTSC64(TSC[2]);
	}
	TSC[2]	= TSC[2] - TSC[1]
		- (TSC[1] > TSC[0] ? TSC[1] - TSC[0] : TSC[0] - TSC[1]);

	Atomic_Write_VPMC(Core, TSC[2], lvl);

	return index;
}
#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
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
	unsigned long long TSC[3] __attribute__ ((aligned (8)));
	const unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));
	const unsigned short lvl = \
			(CoreFreqK.IdleDriver.states[index].flags >> 28) & 0xf;

	if ((PUBLIC(RO(Proc))->Features.AdvPower.EDX.Inv_TSC == 1)
	||  (PUBLIC(RO(Proc))->Features.ExtInfo.EDX.RDTSCP == 1))
	{
		RDTSCP64(TSC[0]);

		RDTSCP64(TSC[1]);

		S2_Handler(pIdleDevice, pIdleDriver, index);

		RDTSCP64(TSC[2]);
	}
	else
	{
		RDTSC64(TSC[0]);

		RDTSC64(TSC[1]);

		S2_Handler(pIdleDevice, pIdleDriver, index);

		RDTSCP64(TSC[2]);
	}
	TSC[2]	= TSC[2] - TSC[1]
		- (TSC[1] > TSC[0] ? TSC[1] - TSC[0] : TSC[0] - TSC[1]);

	Atomic_Write_VPMC(Core, TSC[2], lvl);

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) || (RHEL_MINOR >= 4))
	return index;
#endif /* 5.9.0 */
}
#undef Atomic_Write_VPMC

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

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)) && (RHEL_MAJOR == 0)) \
 || ((RHEL_MAJOR == 8) && (RHEL_MINOR < 4))
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
static void CoreFreqK_Alt_S2_MWAIT_AMD_Handler(
					struct cpuidle_device *pIdleDevice,
					struct cpuidle_driver *pIdleDriver,
						int index)
{
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_AMD_Handler,
							pIdleDevice,
							pIdleDriver,
							index );
}

static void CoreFreqK_Alt_S2_HALT_AMD_Handler(
					struct cpuidle_device *pIdleDevice,
					struct cpuidle_driver *pIdleDriver,
						int index)
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
#if (RHEL_MAJOR == 8)
	int rx = index;
#else
	int rx =
#endif
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
	return rx;
}

static int CoreFreqK_Alt_S2_HALT_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
#if (RHEL_MAJOR == 8)
	int rx = index;
#else
	int rx =
#endif
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_HALT_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
	return rx;
}

static int CoreFreqK_Alt_S2_IO_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
#if (RHEL_MAJOR == 8)
	int rx = index;
#else
	int rx =
#endif
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_IO_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
	return rx;
}

static int CoreFreqK_Alt_S2_MWAIT_AMD_Handler(
					struct cpuidle_device *pIdleDevice,
					struct cpuidle_driver *pIdleDriver,
						int index)
{
#if (RHEL_MAJOR == 8)
	int rx = index;
#else
	int rx =
#endif
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_MWAIT_AMD_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
	return rx;
}

static int CoreFreqK_Alt_S2_HALT_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
#if (RHEL_MAJOR == 8)
	int rx = index;
#else
	int rx =
#endif
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_HALT_AMD_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
	return rx;
}

static int CoreFreqK_Alt_S2_IO_AMD_Handler(struct cpuidle_device *pIdleDevice,
				struct cpuidle_driver *pIdleDriver, int index)
{
#if (RHEL_MAJOR == 8)
	int rx = index;
#else
	int rx =
#endif
	Alternative_Computation_Of_Cycles_S2( CoreFreqK_S2_IO_AMD_Handler,
						pIdleDevice,
						pIdleDriver,
						index );
	return rx;
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
	    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
	    {
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter = CoreFreqK_Alt_MWAIT_Handler;

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter_s2idle = CoreFreqK_Alt_S2_MWAIT_Handler;
	    }
	    else if ((PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		|| (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON))
	    {
		HWCR HwCfgRegister;
		RDMSR(HwCfgRegister, MSR_K7_HWCR);
	      if (BITVAL(HwCfgRegister.value, 9) == 0)
	      {
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter = CoreFreqK_Alt_MWAIT_AMD_Handler;

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter_s2idle = CoreFreqK_Alt_S2_MWAIT_AMD_Handler;
	      } else {
		goto IDLE_WARNING;
	      }
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

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_MWAIT;
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
	    else if ((PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		|| (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON))
	    {
		HWCR HwCfgRegister;
		RDMSR(HwCfgRegister, MSR_K7_HWCR);
	      if (BITVAL(HwCfgRegister.value, 9) == 0)
	      {
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter = CoreFreqK_Alt_HALT_AMD_Handler;

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter_s2idle = CoreFreqK_Alt_S2_HALT_AMD_Handler;
	      } else {
		goto IDLE_WARNING;
	      }
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

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_HALT;
		break;

	  case ROUTE_IO:
	  {
	    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
	    {
		CSTATE_IO_MWAIT CState_IO_MWAIT = {.value = 0};
		CSTATE_CONFIG CStateConfig = {.value = 0};

		RDMSR(CState_IO_MWAIT, MSR_PMG_IO_CAPTURE_BASE);
		RDMSR(CStateConfig, MSR_PKG_CST_CONFIG_CONTROL);

		if ((CState_IO_MWAIT.LVL2_BaseAddr != 0x0)
		 && (CStateConfig.IO_MWAIT_Redir == 1))
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
	    else if ((PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		|| (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON))
	    {
		HWCR HwCfgRegister;
		RDMSR(HwCfgRegister, MSR_K7_HWCR);
	      if (BITVAL(HwCfgRegister.value, 9) == 0)
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
	      } else {
		goto IDLE_WARNING;
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

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_IO;
		break;

	  case ROUTE_DEFAULT:
	  IDLE_DEFAULT:
	  default:
		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_DEFAULT;

	    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
	    {
		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter = CoreFreqK_MWAIT_Handler;

		CoreFreqK.IdleDriver.states[
			CoreFreqK.IdleDriver.state_count
		].enter_s2idle = CoreFreqK_S2_MWAIT_Handler;

		PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_MWAIT;
	    }
	    else if ((PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		|| (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON))
	    {	/* Avoid kernel crash if the MWAIT opcode has been disabled */
		HWCR HwCfgRegister;
		RDMSR(HwCfgRegister, MSR_K7_HWCR);
	      if (BITVAL(HwCfgRegister.value, 9) == 0)
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
			].enter_s2idle = CoreFreqK_Alt_S2_IO_AMD_Handler;

			PUBLIC(RO(Proc))->Registration.Driver.Route = ROUTE_IO;
		}
		else
		{
			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter = CoreFreqK_Alt_HALT_AMD_Handler;

			CoreFreqK.IdleDriver.states[
				CoreFreqK.IdleDriver.state_count
			].enter_s2idle = CoreFreqK_Alt_S2_HALT_AMD_Handler;

			PUBLIC(RO(Proc))->Registration.Driver.Route=ROUTE_HALT;
		}
	      } else {
		goto IDLE_WARNING;
	      }
	    }
	    else {
IDLE_WARNING:	pr_warn("CoreFreq: "					\
			"No Idle implementation for Vendor CRC 0x%x\n",
			PUBLIC(RO(Proc))->Features.Info.Vendor.CRC);
	    }
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void CoreFreqK_Policy_Exit(struct cpufreq_policy *policy)
{
	UNUSED(policy);
}
#else
static int CoreFreqK_Policy_Exit(struct cpufreq_policy *policy)
{
	UNUSED(policy);
	return 0;
}
#endif

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

static void Policy_Aggregate_Turbo(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
    if (PUBLIC(RO(Proc))->Registration.Driver.CPUfreq & REGISTRATION_ENABLE) {
	CoreFreqK.FreqDriver.boost_enabled = (
			BITWISEAND_CC(	LOCKLESS,
					PUBLIC(RW(Proc))->TurboBoost,
					PUBLIC(RO(Proc))->TurboBoost_Mask ) != 0
	);
    }
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
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
#endif /* 3.14.0 */

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
    if (PUBLIC(RO(Proc))->Features.HWP_Enable
    && (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)) {
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

    if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL) {
	SetTarget = Policy_HWP_SetTarget;
    } else if ( (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
	||	(PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON) ) {
	SetTarget = Policy_Zen_CPPC_SetTarget;
    } else {
	SetTarget = Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget;
    }
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

static void Policy_Skylake_SetTarget(void *arg)
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
	    if (Cmp_Skylake_Target(Core, Core->Boost[BOOST(TDP)] > 0 ?
			Core->Boost[BOOST(TDP)] : Core->Boost[BOOST(MAX)]))
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
	if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget(arg);
	}
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
	COF_ST COF = {.Q = 0, .R = 0};
	unsigned int pstate;
	unsigned short WrModRd = 0;
	/* Look-up for the first enabled P-State with the same target ratio */
	for (pstate = 0; pstate <= 7; pstate++)
	{
		PstateDef.value = 0;
		RDMSR(PstateDef, MSR_AMD_PSTATE_DEF_BASE + pstate);

	    if (PstateDef.Family_17h.PstateEn)
	    {
		COF = AMD_Zen_CoreCOF(PstateDef);
		if (COF.Q == (*ratio)) {
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

		COF = AMD_Zen_CoreCOF(PstateDef);

		Core->Boost[BOOST(TGT)] = COF.Q;
	}
#endif /* CONFIG_CPU_FREQ */
}

static void Policy_Zen_CPPC_SetTarget(void *arg)
{
#ifdef CONFIG_CPU_FREQ
  if (PUBLIC(RO(Proc))->Features.HWP_Enable)
  {
	unsigned int *ratio = (unsigned int*) arg;
	unsigned int cpu = smp_processor_id();
	CORE_RO *Core = (CORE_RO *) PUBLIC(RO(Core, AT(cpu)));

    if (((*ratio) >= Core->PowerThermal.HWP_Capabilities.Lowest)
     && ((*ratio) <= Core->PowerThermal.HWP_Capabilities.Highest))
    {
	AMD_CPPC_REQUEST CPPC_Req = {.value = 0};
	unsigned int hint;

	RDMSR(Core->SystemRegister.HWCR, MSR_K7_HWCR);
	RDMSR(CPPC_Req, MSR_AMD_CPPC_REQ);

	hint = CPPC_AMD_Zen_ScaleHint(
				Core, 255U, (*ratio),
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

      if ((hint & (1 << 31)) != (1 << 31))
      {
	CPPC_Req.Maximum_Perf = CPPC_Req.Desired_Perf = hint & 0xff;
	WRMSR(CPPC_Req, MSR_AMD_CPPC_REQ);
	RDMSR(CPPC_Req, MSR_AMD_CPPC_REQ);

	Core->PowerThermal.HWP_Request.Maximum_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Maximum_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->PowerThermal.HWP_Request.Desired_Perf = \
		CPPC_AMD_Zen_ScaleRatio(
				Core, 255U, CPPC_Req.Desired_Perf,
				!Core->SystemRegister.HWCR.Family_17h.CpbDis
		);

	Core->Boost[BOOST(HWP_MAX)]=Core->PowerThermal.HWP_Request.Maximum_Perf;
	Core->Boost[BOOST(HWP_TGT)]=Core->PowerThermal.HWP_Request.Desired_Perf;
      }
    }
  } else {
	if (Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget != NULL) {
		Arch[PUBLIC(RO(Proc))->ArchID].SystemDriver.SetTarget(arg);
	}
  }
#endif /* CONFIG_CPU_FREQ */
}

static int CoreFreqK_FreqDriver_UnInit(void)
{
	int rc = -EINVAL;
#ifdef CONFIG_CPU_FREQ
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)) && (!defined(CONFIG_CACHY)) \
 && (!defined(RHEL_MAJOR))
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

static signed int Seek_Topology_Core_Peer(unsigned int cpu, signed int exclude)
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

static signed int Seek_Topology_Thread_Peer(unsigned int cpu,signed int exclude)
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

static signed int Seek_Topology_Hybrid_Core(unsigned int cpu)
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

static void MatchCoreForService(SERVICE_PROC *pService,
				unsigned int cpi, signed int cpx)
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

static int MatchPeerForService(SERVICE_PROC *pService,
				unsigned int cpi, signed int cpx)
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

static void MatchPeerForDefaultService(SERVICE_PROC *pService, unsigned int cpu)
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

static void MatchPeerForUpService(SERVICE_PROC *pService, unsigned int cpu)
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

static void MatchPeerForDownService(SERVICE_PROC *pService, unsigned int cpu)
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
#ifdef CONFIG_HAVE_NMI
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
#ifdef CONFIG_HAVE_NMI
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
#ifdef CONFIG_HAVE_NMI
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
	case TECHNOLOGY_L1_HW_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_HW_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L1_HW_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L1_HW_IP_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_HW_IP_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L1_HW_IP_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L1_NPP_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_NPP_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L1_NPP_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L1_SCRUBBING:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_Scrubbing_Enable = prm.dl.lo;
			Controller_Start(1);
			L1_Scrubbing_Enable =  -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L2_HW_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L2_HW_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L2_HW_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L2_HW_CL_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L2_HW_CL_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L2_HW_CL_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L2_AMP_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L2_AMP_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L2_AMP_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L2_NLP_PREFETCH:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L2_NLP_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L2_NLP_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_L1_STRIDE_PREFETCH:
	    if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.PrefetchCtl_MSR) {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_STRIDE_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L1_STRIDE_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_L1_REGION_PREFETCH:
	    if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.PrefetchCtl_MSR) {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_REGION_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L1_REGION_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_L1_BURST_PREFETCH:
	    if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.PrefetchCtl_MSR) {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L1_BURST_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L1_BURST_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_L2_STREAM_HW_PREFETCH:
	    if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.PrefetchCtl_MSR) {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L2_STREAM_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L2_STREAM_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_L2_UPDOWN_PREFETCH:
	    if (PUBLIC(RO(Proc))->Features.ExtFeature2_EAX.PrefetchCtl_MSR) {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			L2_UPDOWN_PREFETCH_Disable = !prm.dl.lo;
			Controller_Start(1);
			L2_UPDOWN_PREFETCH_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_LLC_STREAMER:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			LLC_Streamer_Disable = !prm.dl.lo;
			Controller_Start(1);
			LLC_Streamer_Disable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

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
	    switch (prm.dl.lo) {
	    case COREFREQ_TOGGLE_ON:
		Controller_Stop(1);
		HWP_Enable = prm.dl.lo;
	      if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL) {
		Intel_Hardware_Performance();
	      } else if ((PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		|| (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON))
	      {
		if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.CPPC) {
			AMD_F17h_CPPC();
		} else if (PUBLIC(RO(Proc))->Features.ACPI_CPPC) {
			For_All_ACPI_CPPC(Enable_ACPI_CPPC, NULL);
		}
	      }
		Controller_Start(1);
		HWP_Enable = -1;
		rc = RC_SUCCESS;
		break;
	    case COREFREQ_TOGGLE_OFF:
	      if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL) {
		rc = -RC_UNIMPLEMENTED;
	      } else if ((PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_AMD)
		|| (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_HYGON))
	      {
		if (PUBLIC(RO(Proc))->Features.leaf80000008.EBX.CPPC) {
			rc = -RC_UNIMPLEMENTED;
		} else if (PUBLIC(RO(Proc))->Features.ACPI_CPPC) {
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

	case TECHNOLOGY_HDC:
		Controller_Stop(1);
		HDC_Enable = prm.dl.lo;
		Intel_Hardware_Performance();
		Controller_Start(1);
		HDC_Enable = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_EEO:
	    if (PUBLIC(RO(Proc))->Features.EEO_Capable)
	    {
		EEO_Disable = prm.dl.lo;
		Skylake_PowerControl();
		EEO_Disable = -1;
		rc = RC_SUCCESS;
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_R2H:
	    if (PUBLIC(RO(Proc))->Features.R2H_Capable)
	    {
		R2H_Disable = prm.dl.lo;
		Skylake_PowerControl();
		R2H_Disable = -1;
		rc = RC_SUCCESS;
	    } else {
		rc = -ENXIO;
	    }
		break;

	case TECHNOLOGY_CFG_TDP_LVL:
		Controller_Stop(1);
		Config_TDP_Level = prm.dl.lo;
		Controller_Start(1);
		Config_TDP_Level = -1;
		rc = RC_SUCCESS;
		break;

	case TECHNOLOGY_TDP_LIMITING:
	    {
		const enum PWR_DOMAIN	pw = prm.dh.lo;
		const enum PWR_LIMIT	pl = prm.dh.hi;

		const unsigned int idx = (PWR_LIMIT_SIZE * pw) + pl;
	      if (idx < PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE))
	      {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			TDP_Limiting_Count = PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE);
			RESET_ARRAY(Activate_TDP_Limit, TDP_Limiting_Count, -1);
			Activate_TDP_Limit[idx] = prm.dl.lo;

			Controller_Start(1);
			RESET_ARRAY(Activate_TDP_Limit, TDP_Limiting_Count, -1);
			TDP_Limiting_Count = 0;
			rc = RC_SUCCESS;
			break;
		}
	      }
	    }
		break;

	case TECHNOLOGY_TDP_CLAMPING:
	  {
		const enum PWR_DOMAIN	pw = prm.dh.lo;

	    if ( !( (pw == PWR_DOMAIN(RAM))
		&& !PUBLIC(RO(Proc))->Registration.Experimental) )
	    {
		const enum PWR_LIMIT	pl = prm.dh.hi;

		const unsigned int idx = (PWR_LIMIT_SIZE * pw) + pl;
	      if (idx < PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE))
	      {
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			TDP_Clamping_Count = PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE);
			RESET_ARRAY(Activate_TDP_Clamp, TDP_Clamping_Count, -1);
			Activate_TDP_Clamp[idx] = prm.dl.lo;

			Controller_Start(1);
			RESET_ARRAY(Activate_TDP_Clamp, TDP_Clamping_Count, -1);
			TDP_Clamping_Count = 0;
			rc = RC_SUCCESS;
			break;
		}
	      }
	    } else {
		rc = -RC_EXPERIMENTAL;
	    }
	  }
		break;

	case TECHNOLOGY_TDP_OFFSET:
	    {
		const enum PWR_DOMAIN	pw = prm.dh.lo;
		const enum PWR_LIMIT	pl = prm.dh.hi;
		/* Offset is capped within [ -127 , +128 ] watt */
		const signed short offset = (signed char) prm.dl.lo;

		const unsigned int idx = (PWR_LIMIT_SIZE * pw) + pl;
	      if (idx < PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE))
	      {
		Controller_Stop(1);
		Custom_TDP_Count = PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE);
		RESET_ARRAY(Custom_TDP_Offset, Custom_TDP_Count, 0);
		Custom_TDP_Offset[idx] = offset;

		Controller_Start(1);
		RESET_ARRAY(Custom_TDP_Offset, Custom_TDP_Count, 0);
		Custom_TDP_Count = 0;
		rc = RC_SUCCESS;
	      }
	    }
		break;

	case TECHNOLOGY_TDC_LIMITING:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			Activate_TDC_Limit = prm.dl.lo;
			Controller_Start(1);
			Activate_TDC_Limit = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_TDC_OFFSET:
	    {
		const signed short offset = (signed char) prm.dl.lo;
		Controller_Stop(1);
		Custom_TDC_Offset = offset;
		Controller_Start(1);
		Custom_TDC_Offset = 0;
		rc = RC_SUCCESS;
	    }
		break;

	case TECHNOLOGY_THM_OFFSET:
	    {
		const signed short offset = (signed short) prm.dl.lo;
		if (PUBLIC(RO(Proc))->Features.Info.Vendor.CRC == CRC_INTEL)
		{
			PLATFORM_INFO PfInfo = {.value = 0};
			RDMSR(PfInfo, MSR_PLATFORM_INFO);
			Controller_Stop(1);
			ThermalOffset = offset;
			rc = Intel_ThermalOffset(PfInfo.ProgrammableTj);
			Controller_Start(1);
			ThermalOffset = 0;
		} else {
			rc = -RC_UNIMPLEMENTED;
		}
	    }
		break;

	case TECHNOLOGY_TW_POWER:
	    {
		const enum PWR_DOMAIN	pw = prm.dh.lo;
		const enum PWR_LIMIT	pl = prm.dh.hi;
		const signed short	tw = prm.dl.lo;

		const unsigned int idx = (PWR_LIMIT_SIZE * pw) + pl;
	      if (idx < PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE))
	      {
		Controller_Stop(1);
		PowerTimeWindow_Count = PWR_LIMIT_SIZE * PWR_DOMAIN(SIZE);
		RESET_ARRAY(PowerTimeWindow, PowerTimeWindow_Count, -1);
		PowerTimeWindow[idx] = tw;

		Controller_Start(1);
		RESET_ARRAY(PowerTimeWindow, PowerTimeWindow_Count, -1);
		PowerTimeWindow_Count = 0;
		rc = RC_SUCCESS;
	      }
	    }
		break;

	case TECHNOLOGY_WDT:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
			Controller_Stop(1);
			WDT_Enable = prm.dl.lo;
			Controller_Start(1);
			WDT_Enable = -1;
			rc = RC_SUCCESS;
			break;
		}
		break;

	case TECHNOLOGY_HSMP:
		switch (prm.dl.lo) {
		case COREFREQ_TOGGLE_OFF:
		case COREFREQ_TOGGLE_ON:
		    {
			unsigned int rx;
			Controller_Stop(1);
			PUBLIC(RO(Proc))->Features.HSMP_Capable = prm.dl.lo;
			rx = Query_AMD_HSMP_Interface();
			Controller_Start(1);
			rc = (rx >= HSMP_FAIL_BGN && rx <= HSMP_FAIL_END) ?
				-RC_UNIMPLEMENTED : RC_SUCCESS;
		    }
			break;
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

    case COREFREQ_IOCTL_CONFIG_TDP: {
		CLOCK_ARG clockMod = {.sllong = arg};
		const short MaxRatio = MAXCLOCK_TO_RATIO(short, \
		PUBLIC(RO(Core,AT(PUBLIC(RO(Proc))->Service.Core))->Clock.Hz));

		switch (clockMod.NC) {
		case CLOCK_MOD_ACT:
			if ((clockMod.Ratio >= 0)&&(clockMod.Ratio <= MaxRatio))
			{
				Controller_Stop(1);
				Turbo_Activation_Ratio = clockMod.Ratio;
				Controller_Start(1);
				Turbo_Activation_Ratio = -1;
				rc = RC_OK_COMPUTE;
			}
			break;
		}
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
		    #if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
			Clear_Events = (unsigned long)(-1);
		    #else
			Clear_Events = (unsigned long long)(-1);
		    #endif
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
		if (reqSize != PUBLIC(RO(Proc))->Gate.ReqMem.Size) {
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
inline void Print_SuspendResume(void)
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

static void SMBIOS_Collect(void)
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
	const size_t count = sizeof(dmi_collect) / sizeof(dmi_collect[0]);
	size_t idx;
	for (idx = 0; idx < count; idx++) {
		const char *pInfo = dmi_get_system_info(dmi_collect[idx].field);
		if ((pInfo != NULL) && (strlen(pInfo) > 0)) {
			StrCopy(dmi_collect[idx].recipient, pInfo, MAX_UTS_LEN);
		}
	}
#endif /* CONFIG_DMI */
}

#ifdef CONFIG_DMI
static char *SMBIOS_String(const struct dmi_header *dh, u8 id)
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

static void SMBIOS_Entries(const struct dmi_header *dh, void *priv)
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

inline void SMBIOS_Decoder(void)
{
#ifdef CONFIG_DMI
	size_t count = 0;
	dmi_walk(SMBIOS_Entries, &count);
#endif /* CONFIG_DMI */
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 2, 0) \
 || ((RHEL_MAJOR == 8) && ((RHEL_MINOR < 3) || (RHEL_MINOR > 8))) \
 || ((RHEL_MAJOR >= 9) && (RHEL_MINOR > 0) && (RHEL_MINOR < 99))
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
	pArg->Brand = kmalloc(BRAND_SIZE, GFP_KERNEL);
	if (pArg->Brand == NULL)
	{
		return -ENOMEM;
	} else {
		memset(pArg->Brand, 0x20, BRAND_SIZE);
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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0) \
 || (defined(RHEL_MAJOR) && (RHEL_MAJOR >= 9) \
 && (RHEL_MINOR > 0) && (RHEL_MINOR < 99))
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

	PUBLIC(RO(Proc))->Gate.ReqMem.Size = \
			PAGE_SIZE << PUBLIC(RO(Proc))->Gate.ReqMem.Order;

	PUBLIC(RO(Proc))->Registration.AutoClock = AutoClock;
	PUBLIC(RO(Proc))->Registration.Experimental = Experimental;

	Compute_Interval();

	memcpy(&PUBLIC(RO(Proc))->Features, pArg->Features, sizeof(FEATURES));

	/* Initialize default uArch's codename with the CPUID brand. */
	Arch[GenuineArch].Architecture[0] = \
				PUBLIC(RO(Proc))->Features.Info.Vendor.ID;
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

		Define_CPUID(PUBLIC(RO(Core, AT(cpu))), CpuIDforVendor);
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
	BIT_ATOM_INIT(PRIVATE(OF(Zen)).AMD_SMN_LOCK, ATOMIC_SEED);
	BIT_ATOM_INIT(PRIVATE(OF(Zen)).AMD_FCH_LOCK, ATOMIC_SEED);

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
		fallthrough;
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
	case CRC_KVM:
	case CRC_VBOX:
	case CRC_KBOX:
	case CRC_VMWARE:
	case CRC_HYPERV:
		/* Unexpected */
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
		Arch[PUBLIC(RO(Proc))->ArchID].Architecture[0],
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
			PUBLIC(RO(Proc))->ArchID = GenuineArch;
			Arch[GenuineArch].Query = Query_VirtualMachine;
			Arch[GenuineArch].Update= PerCore_VirtualMachine;
			Arch[GenuineArch].Start = Start_Safe_VirtualMachine;
			Arch[GenuineArch].Stop	= Stop_VirtualMachine;
			Arch[GenuineArch].Timer = InitTimer_Safe_VirtualMachine;

			Arch[GenuineArch].thermalFormula = THERMAL_FORMULA_NONE;
			Arch[GenuineArch].voltageFormula = VOLTAGE_FORMULA_NONE;
			Arch[GenuineArch].powerFormula = POWER_FORMULA_NONE;
			break;
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
	SMBIOS_Decoder();

	/*	Initialize the CoreFreq controller			*/
	Controller_Init();

	/*	Seek for an appropriate service processor		*/
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
  if (PUBLIC(RO(Proc))->ArchID != AMD_Family_0Fh)
  {
	const unsigned int cpu = PUBLIC(RO(Proc))->Service.Core;

	const signed int MinFID = PUBLIC(RO(Core, AT(cpu)))->Boost[BOOST(MIN)],

	MaxFID = MAXCLOCK_TO_RATIO(	signed int,
					PUBLIC(RO(Core, AT(cpu))->Clock.Hz) );

    if (PState_FID != -1) {
      if ((PState_FID >= MinFID) && (PState_FID <= MaxFID)) {
	if (Arch[PUBLIC(RO(Proc))->ArchID].ClockMod != NULL)
	{
		long rc;
		CLOCK_ARG clockMod = {
			.NC = CLOCK_MOD_MAX,
			.Ratio = PState_FID,
			.cpu = -1
		};

		Controller_Stop(1);
		rc = Arch[PUBLIC(RO(Proc))->ArchID].ClockMod(&clockMod);
		Controller_Start(1);

	  if (rc < RC_SUCCESS) {
		pr_warn("CoreFreq: "					\
			"'PState_FID' Execution failure code %ld\n", rc);
	  }
	} else {
		pr_warn("CoreFreq: "					\
			"Unsupported architecture #%d for 'PState_FID'\n",
			PUBLIC(RO(Proc))->ArchID);
	}
      } else {
	pr_warn("CoreFreq: "						\
		"'PState_FID' is out of range [%d, %d]\n", MinFID, MaxFID);
      }
	PState_FID = -1;
    }
  } /* else handled by function PerCore_AMD_Family_0Fh_PStates()	*/
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
