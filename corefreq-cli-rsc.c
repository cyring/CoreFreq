/*
 * CoreFreq
 * Copyright (C) 2015-2021 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <sched.h>
#include <errno.h>

#include "bitasm.h"
#include "corefreq-ui.h"
#include "corefreq-cli-rsc.h"
#include "corefreq-cli-rsc-en.h"
#include "corefreq-cli-rsc-fr.h"

#ifndef NO_HEADER
ATTRIBUTE Rsc_Layout_Header_Proc_Attr[] = RSC_LAYOUT_HEADER_PROC_ATTR;
ASCII	Rsc_Layout_Header_Proc_Code_En[] = RSC_LAYOUT_HEADER_PROC_CODE_EN,
	Rsc_Layout_Header_Proc_Code_Fr[] = RSC_LAYOUT_HEADER_PROC_CODE_FR;

ATTRIBUTE Rsc_Layout_Header_CPU_Attr[] = RSC_LAYOUT_HEADER_CPU_ATTR;
ASCII	Rsc_Layout_Header_CPU_Code_En[] = RSC_LAYOUT_HEADER_CPU_CODE;
#define Rsc_Layout_Header_CPU_Code_Fr Rsc_Layout_Header_CPU_Code_En

ATTRIBUTE Rsc_Layout_Header_Arch_Attr[] = RSC_LAYOUT_HEADER_ARCH_ATTR;
ASCII	Rsc_Layout_Header_Arch_Code_En[] = RSC_LAYOUT_HEADER_ARCH_CODE;
#define Rsc_Layout_Header_Arch_Code_Fr Rsc_Layout_Header_Arch_Code_En

ATTRIBUTE Rsc_Layout_Header_Cache_L1_Attr[] = RSC_LAYOUT_HEADER_CACHE_L1_ATTR;
ASCII	Rsc_Layout_Header_Cache_L1_Code_En[] = RSC_LAYOUT_HEADER_CACHE_L1_CODE;
#define Rsc_Layout_Header_Cache_L1_Code_Fr Rsc_Layout_Header_Cache_L1_Code_En

ATTRIBUTE Rsc_Layout_Header_BClk_Attr[] = RSC_LAYOUT_HEADER_BCLK_ATTR;
ASCII	Rsc_Layout_Header_BClk_Code_En[] = RSC_LAYOUT_HEADER_BCLK_CODE_EN,
	Rsc_Layout_Header_BClk_Code_Fr[] = RSC_LAYOUT_HEADER_BCLK_CODE_FR;

ATTRIBUTE Rsc_Layout_Header_Caches_Attr[] = RSC_LAYOUT_HEADER_CACHES_ATTR;
ASCII	Rsc_Layout_Header_Caches_Code_En[] = RSC_LAYOUT_HEADER_CACHES_CODE;
#define Rsc_Layout_Header_Caches_Code_Fr Rsc_Layout_Header_Caches_Code_En
#endif /* NO_HEADER */

#ifndef NO_UPPER
ATTRIBUTE Rsc_Layout_Ruler_Load_Attr[] = RSC_LAYOUT_RULER_LOAD_ATTR;
ASCII	Rsc_Layout_Ruler_Load_Code_En[] = RSC_LAYOUT_RULER_LOAD_CODE;
#define Rsc_Layout_Ruler_Load_Code_Fr Rsc_Layout_Ruler_Load_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Var_Load_Attr[] = RSC_LAYOUT_RULER_VAR_LOAD_ATTR;
#define Rsc_Layout_Ruler_Rel_Load_Attr Rsc_Layout_Ruler_Var_Load_Attr
#define Rsc_Layout_Ruler_Abs_Load_Attr Rsc_Layout_Ruler_Var_Load_Attr
ASCII	Rsc_Layout_Ruler_Rel_Load_Code_En[] = RSC_LAYOUT_RULER_REL_LOAD_CODE_EN,
	Rsc_Layout_Ruler_Rel_Load_Code_Fr[] = RSC_LAYOUT_RULER_REL_LOAD_CODE_FR,
	Rsc_Layout_Ruler_Abs_Load_Code_En[] = RSC_LAYOUT_RULER_ABS_LOAD_CODE_EN,
	Rsc_Layout_Ruler_Abs_Load_Code_Fr[] = RSC_LAYOUT_RULER_ABS_LOAD_CODE_FR;
#endif /* NO_UPPER */

#ifndef NO_LOWER
ATTRIBUTE Rsc_Layout_Monitor_Frequency_Attr[]=RSC_LAYOUT_MONITOR_FREQUENCY_ATTR;
ASCII Rsc_Layout_Monitor_Frequency_Code_En[]=RSC_LAYOUT_MONITOR_FREQUENCY_CODE;
#define Rsc_Layout_Monitor_Frequency_Code_Fr \
					Rsc_Layout_Monitor_Frequency_Code_En

ATTRIBUTE Rsc_Layout_Monitor_Inst_Attr[] = RSC_LAYOUT_MONITOR_INST_ATTR;
ASCII	Rsc_Layout_Monitor_Inst_Code_En[] = RSC_LAYOUT_MONITOR_INST_CODE;
#define Rsc_Layout_Monitor_Inst_Code_Fr Rsc_Layout_Monitor_Inst_Code_En

ATTRIBUTE Rsc_Layout_Monitor_Common_Attr[] = RSC_LAYOUT_MONITOR_COMMON_ATTR;
ASCII	Rsc_Layout_Monitor_Common_Code_En[] = RSC_LAYOUT_MONITOR_COMMON_CODE;
#define Rsc_Layout_Monitor_Common_Code_Fr Rsc_Layout_Monitor_Common_Code_En

ATTRIBUTE Rsc_Layout_Monitor_Tasks_Attr[] = RSC_LAYOUT_MONITOR_TASKS_ATTR;
ASCII	Rsc_Layout_Monitor_Tasks_Code_En[] = RSC_LAYOUT_MONITOR_TASKS_CODE;
#define Rsc_Layout_Monitor_Tasks_Code_Fr Rsc_Layout_Monitor_Tasks_Code_En

ATTRIBUTE Rsc_Layout_Monitor_Slice_Attr[] = RSC_LAYOUT_MONITOR_SLICE_ATTR;
ASCII	Rsc_Layout_Monitor_Slice_Code_En[] = RSC_LAYOUT_MONITOR_SLICE_CODE;
#define Rsc_Layout_Monitor_Slice_Code_Fr Rsc_Layout_Monitor_Slice_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Frequency_Attr[] = RSC_LAYOUT_RULER_FREQUENCY_ATTR;
ASCII Rsc_Layout_Ruler_Frequency_Code_En[] = RSC_LAYOUT_RULER_FREQUENCY_CODE_EN,
      Rsc_Layout_Ruler_Frequency_Code_Fr[] = RSC_LAYOUT_RULER_FREQUENCY_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_Freq_Avg_Attr[] = \
					RSC_LAYOUT_RULER_FREQUENCY_AVG_ATTR;
ASCII	Rsc_Layout_Ruler_Freq_Avg_Code_En[] = \
					RSC_LAYOUT_RULER_FREQUENCY_AVG_CODE_EN,
	Rsc_Layout_Ruler_Freq_Avg_Code_Fr[] = \
					RSC_LAYOUT_RULER_FREQUENCY_AVG_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_Freq_Pkg_Attr[] = \
					RSC_LAYOUT_RULER_FREQUENCY_PKG_ATTR;
ASCII	Rsc_Layout_Ruler_Freq_Pkg_Code_En[] = \
					RSC_LAYOUT_RULER_FREQUENCY_PKG_CODE;
#define Rsc_Layout_Ruler_Freq_Pkg_Code_Fr Rsc_Layout_Ruler_Freq_Pkg_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Inst_Attr[]	= RSC_LAYOUT_RULER_INST_ATTR;
ASCII	Rsc_Layout_Ruler_Inst_Code_En[] = RSC_LAYOUT_RULER_INST_CODE;
#define Rsc_Layout_Ruler_Inst_Code_Fr Rsc_Layout_Ruler_Inst_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Cycles_Attr[] = RSC_LAYOUT_RULER_CYCLES_ATTR;
ASCII	Rsc_Layout_Ruler_Cycles_Code_En[] = RSC_LAYOUT_RULER_CYCLES_CODE;
#define Rsc_Layout_Ruler_Cycles_Code_Fr Rsc_Layout_Ruler_Cycles_Code_En

ATTRIBUTE Rsc_Layout_Ruler_CStates_Attr[] = RSC_LAYOUT_RULER_CSTATES_ATTR;
ASCII	Rsc_Layout_Ruler_CStates_Code_En[] = RSC_LAYOUT_RULER_CSTATES_CODE;
#define Rsc_Layout_Ruler_CStates_Code_Fr Rsc_Layout_Ruler_CStates_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Interrupts_Attr[] = \
					RSC_LAYOUT_RULER_INTERRUPTS_ATTR;
ASCII	Rsc_Layout_Ruler_Interrupts_Code_En[] = \
					RSC_LAYOUT_RULER_INTERRUPTS_CODE;
#define Rsc_Layout_Ruler_Interrupts_Code_Fr \
					Rsc_Layout_Ruler_Interrupts_Code_En

#define Rsc_Layout_Ruler_Package_Attr vColor
ASCII	Rsc_Layout_Ruler_Package_Code_En[] = RSC_LAYOUT_RULER_PACKAGE_CODE_EN,
	Rsc_Layout_Ruler_Package_Code_Fr[] = RSC_LAYOUT_RULER_PACKAGE_CODE_FR;

ATTRIBUTE Rsc_Layout_Package_PC_Attr[] = RSC_LAYOUT_PACKAGE_PC_ATTR;
ASCII	Rsc_Layout_Package_PC_Code_En[] = RSC_LAYOUT_PACKAGE_PC_CODE,
	Rsc_Layout_Package_PC02_Code_En[] = RSC_LAYOUT_PACKAGE_PC02_CODE,
	Rsc_Layout_Package_PC03_Code_En[] = RSC_LAYOUT_PACKAGE_PC03_CODE,
	Rsc_Layout_Package_PC04_Code_En[] = RSC_LAYOUT_PACKAGE_PC04_CODE,
	Rsc_Layout_Package_PC06_Code_En[] = RSC_LAYOUT_PACKAGE_PC06_CODE,
	Rsc_Layout_Package_PC07_Code_En[] = RSC_LAYOUT_PACKAGE_PC07_CODE,
	Rsc_Layout_Package_PC08_Code_En[] = RSC_LAYOUT_PACKAGE_PC08_CODE,
	Rsc_Layout_Package_PC09_Code_En[] = RSC_LAYOUT_PACKAGE_PC09_CODE,
	Rsc_Layout_Package_PC10_Code_En[] = RSC_LAYOUT_PACKAGE_PC10_CODE,
	Rsc_Layout_Package_MC06_Code_En[] = RSC_LAYOUT_PACKAGE_MC06_CODE;
#define Rsc_Layout_Package_PC_Code_Fr Rsc_Layout_Package_PC_Code_En
#define Rsc_Layout_Package_PC02_Code_Fr Rsc_Layout_Package_PC02_Code_En
#define Rsc_Layout_Package_PC03_Code_Fr Rsc_Layout_Package_PC03_Code_En
#define Rsc_Layout_Package_PC04_Code_Fr Rsc_Layout_Package_PC04_Code_En
#define Rsc_Layout_Package_PC06_Code_Fr Rsc_Layout_Package_PC06_Code_En
#define Rsc_Layout_Package_PC07_Code_Fr Rsc_Layout_Package_PC07_Code_En
#define Rsc_Layout_Package_PC08_Code_Fr Rsc_Layout_Package_PC08_Code_En
#define Rsc_Layout_Package_PC09_Code_Fr Rsc_Layout_Package_PC09_Code_En
#define Rsc_Layout_Package_PC10_Code_Fr Rsc_Layout_Package_PC10_Code_En
#define Rsc_Layout_Package_MC06_Code_Fr Rsc_Layout_Package_MC06_Code_En

ATTRIBUTE Rsc_Layout_Package_Uncore_Attr[] = RSC_LAYOUT_PACKAGE_UNCORE_ATTR;
ASCII	Rsc_Layout_Package_Uncore_Code_En[] = RSC_LAYOUT_PACKAGE_UNCORE_CODE;
#define Rsc_Layout_Package_Uncore_Code_Fr Rsc_Layout_Package_Uncore_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Tasks_Attr[] = RSC_LAYOUT_RULER_TASKS_ATTR;
ASCII	Rsc_Layout_Ruler_Tasks_Code_En[] = RSC_LAYOUT_RULER_TASKS_CODE_EN,
	Rsc_Layout_Ruler_Tasks_Code_Fr[] = RSC_LAYOUT_RULER_TASKS_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Tracking_Attr[] = RSC_LAYOUT_TASKS_TRACKING_ATTR;
ASCII	Rsc_Layout_Tasks_Tracking_Code_En[] = RSC_LAYOUT_TASKS_TRACKING_CODE_EN,
	Rsc_Layout_Tasks_Tracking_Code_Fr[] = RSC_LAYOUT_TASKS_TRACKING_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_State_Sorted_Attr[] = \
					RSC_LAYOUT_TASKS_STATE_SORTED_ATTR;
ASCII	Rsc_Layout_Tasks_State_Sorted_Code_En[] = \
					RSC_LAYOUT_TASKS_STATE_SORTED_CODE_EN,
	Rsc_Layout_Tasks_State_Sorted_Code_Fr[] = \
					RSC_LAYOUT_TASKS_STATE_SORTED_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_RunTime_Sorted_Attr[] = \
					RSC_LAYOUT_TASKS_RUNTIME_SORTED_ATTR;
ASCII	Rsc_Layout_Tasks_RunTime_Sorted_Code_En[] = \
					RSC_LAYOUT_TASKS_RUNTIME_SORTED_CODE_EN,
	Rsc_Layout_Tasks_RunTime_Sorted_Code_Fr[] = \
					RSC_LAYOUT_TASKS_RUNTIME_SORTED_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_UsrTime_Sorted_Attr[] = \
					RSC_LAYOUT_TASKS_USRTIME_SORTED_ATTR;
ASCII	Rsc_Layout_Tasks_UsrTime_Sorted_Code_En[] = \
					RSC_LAYOUT_TASKS_USRTIME_SORTED_CODE_EN,
	Rsc_Layout_Tasks_UsrTime_Sorted_Code_Fr[] = \
					RSC_LAYOUT_TASKS_USRTIME_SORTED_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_SysTime_Sorted_Attr[] = \
					RSC_LAYOUT_TASKS_SYSTIME_SORTED_ATTR;
ASCII	Rsc_Layout_Tasks_SysTime_Sorted_Code_En[] = \
					RSC_LAYOUT_TASKS_SYSTIME_SORTED_CODE_EN,
	Rsc_Layout_Tasks_SysTime_Sorted_Code_Fr[] = \
					RSC_LAYOUT_TASKS_SYSTIME_SORTED_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Process_Sorted_Attr[] = \
					RSC_LAYOUT_TASKS_PROCESS_SORTED_ATTR;
ASCII	Rsc_Layout_Tasks_Process_Sorted_Code_En[] = \
					RSC_LAYOUT_TASKS_PROCESS_SORTED_CODE_EN,
	Rsc_Layout_Tasks_Process_Sorted_Code_Fr[] = \
					RSC_LAYOUT_TASKS_PROCESS_SORTED_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Command_Sorted_Attr[] = \
					RSC_LAYOUT_TASKS_COMMAND_SORTED_ATTR;
ASCII	Rsc_Layout_Tasks_Command_Sorted_Code_En[] = \
					RSC_LAYOUT_TASKS_COMMAND_SORTED_CODE_EN,
	Rsc_Layout_Tasks_Command_Sorted_Code_Fr[] = \
					RSC_LAYOUT_TASKS_COMMAND_SORTED_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Reverse_Sort_Off_Attr[] = \
					RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_ATTR;
ASCII	Rsc_Layout_Tasks_Reverse_Sort_Off_Code_En[] = \
				RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_EN,
	Rsc_Layout_Tasks_Reverse_Sort_Off_Code_Fr[] = \
				RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Reverse_Sort_On_Attr[] = \
					RSC_LAYOUT_TASKS_REVERSE_SORT_ON_ATTR;
ASCII	Rsc_Layout_Tasks_Reverse_Sort_On_Code_En[] = \
				RSC_LAYOUT_TASKS_REVERSE_SORT_ON_CODE_EN,
	Rsc_Layout_Tasks_Reverse_Sort_On_Code_Fr[] = \
				RSC_LAYOUT_TASKS_REVERSE_SORT_ON_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Value_Switch_Attr[] = \
					RSC_LAYOUT_TASKS_VALUE_SWITCH_ATTR;
ASCII	Rsc_Layout_Tasks_Value_Switch_Code_En[] = \
					RSC_LAYOUT_TASKS_VALUE_SWITCH_CODE_EN,
	Rsc_Layout_Tasks_Value_Switch_Code_Fr[] = \
					RSC_LAYOUT_TASKS_VALUE_SWITCH_CODE_FR;

ATTRIBUTE Rsc_Layout_Tasks_Value_Off_Attr[] = RSC_LAYOUT_TASKS_VALUE_OFF_ATTR;
ASCII	Rsc_Layout_Tasks_Value_Off_Code_En[] = RSC_LAYOUT_TASKS_VALUE_OFF_CODE;
#define Rsc_Layout_Tasks_Value_Off_Code_Fr Rsc_Layout_Tasks_Value_Off_Code_En

ATTRIBUTE Rsc_Layout_Tasks_Value_On_Attr[] = RSC_LAYOUT_TASKS_VALUE_ON_ATTR;
ASCII	Rsc_Layout_Tasks_Value_On_Code_En[] = RSC_LAYOUT_TASKS_VALUE_ON_CODE;
#define Rsc_Layout_Tasks_Value_On_Code_Fr Rsc_Layout_Tasks_Value_On_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Sensors_Attr[] = RSC_LAYOUT_RULER_SENSORS_ATTR;
ASCII	Rsc_Layout_Ruler_Sensors_Code_En[] = RSC_LAYOUT_RULER_SENSORS_CODE_EN,
	Rsc_Layout_Ruler_Sensors_Code_Fr[] = RSC_LAYOUT_RULER_SENSORS_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_Power_Attr[] = RSC_LAYOUT_RULER_POWER_ATTR;
ASCII Rsc_Layout_Ruler_Pwr_Uncore_Code_En[]=RSC_LAYOUT_RULER_PWR_UNCORE_CODE_EN;
#define Rsc_Layout_Ruler_Pwr_Uncore_Code_Fr Rsc_Layout_Ruler_Pwr_Uncore_Code_En
ASCII	Rsc_Layout_Ruler_Pwr_SoC_Code_En[]=RSC_LAYOUT_RULER_PWR_SOC_CODE_EN;
#define Rsc_Layout_Ruler_Pwr_SoC_Code_Fr Rsc_Layout_Ruler_Pwr_SoC_Code_En
ASCII	Rsc_Layout_Ruler_Pwr_Pfm_Code_En[]=RSC_LAYOUT_RULER_PWR_PFM_CODE_EN;
#define Rsc_Layout_Ruler_Pwr_Pfm_Code_Fr Rsc_Layout_Ruler_Pwr_Pfm_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Voltage_Attr[] = RSC_LAYOUT_RULER_VOLTAGE_ATTR;
ASCII	Rsc_Layout_Ruler_Voltage_Code_En[] = RSC_LAYOUT_RULER_VOLTAGE_CODE_EN,
	Rsc_Layout_Ruler_Voltage_Code_Fr[] = RSC_LAYOUT_RULER_VOLTAGE_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_VPkg_SoC_Attr[] = RSC_LAYOUT_RULER_VPKG_SOC_ATTR;
ASCII	Rsc_Layout_Ruler_VPkg_SoC_Code_En[]=RSC_LAYOUT_RULER_VPKG_SOC_CODE_EN,
	Rsc_Layout_Ruler_VPkg_SoC_Code_Fr[]=RSC_LAYOUT_RULER_VPKG_SOC_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_Energy_Attr[] = RSC_LAYOUT_RULER_ENERGY_ATTR;
ASCII	Rsc_Layout_Ruler_Energy_Code_En[] = RSC_LAYOUT_RULER_ENERGY_CODE_EN,
	Rsc_Layout_Ruler_Energy_Code_Fr[] = RSC_LAYOUT_RULER_ENERGY_CODE_FR;

ASCII	Rsc_Layout_Ruler_Power_Code_En[] = RSC_LAYOUT_RULER_POWER_CODE_EN,
	Rsc_Layout_Ruler_Power_Code_Fr[] = RSC_LAYOUT_RULER_POWER_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_Slice_Attr[] = RSC_LAYOUT_RULER_SLICE_ATTR;
ASCII	Rsc_Layout_Ruler_Slice_Code_En[] = RSC_LAYOUT_RULER_SLICE_CODE_EN,
	Rsc_Layout_Ruler_Slice_Code_Fr[] = RSC_LAYOUT_RULER_SLICE_CODE_FR;
#endif /* NO_LOWER */

#ifndef NO_FOOTER
ATTRIBUTE Rsc_Layout_Footer_Tech_x86_Attr[] = RSC_LAYOUT_FOOTER_TECH_X86_ATTR;
ASCII	Rsc_Layout_Footer_Tech_x86_Code_En[] = RSC_LAYOUT_FOOTER_TECH_X86_CODE;
#define Rsc_Layout_Footer_Tech_x86_Code_Fr Rsc_Layout_Footer_Tech_x86_Code_En

ATTRIBUTE Rsc_Layout_Footer_Tech_Intel_Attr[] = \
					RSC_LAYOUT_FOOTER_TECH_INTEL_ATTR;
ASCII	Rsc_Layout_Footer_Tech_Intel_Code_En[] = \
					RSC_LAYOUT_FOOTER_TECH_INTEL_CODE;
#define Rsc_Layout_Footer_Tech_Intel_Code_Fr \
					Rsc_Layout_Footer_Tech_Intel_Code_En

ATTRIBUTE Rsc_Layout_Footer_Tech_AMD_Attr[] = RSC_LAYOUT_FOOTER_TECH_AMD_ATTR;
ASCII	Rsc_Layout_Footer_Tech_AMD_Code_En[] = RSC_LAYOUT_FOOTER_TECH_AMD_CODE;
#define Rsc_Layout_Footer_Tech_AMD_Code_Fr Rsc_Layout_Footer_Tech_AMD_Code_En

ATTRIBUTE Rsc_Layout_Footer_System_Attr[] = RSC_LAYOUT_FOOTER_SYSTEM_ATTR;
ASCII	Rsc_Layout_Footer_System_Code_En[] = RSC_LAYOUT_FOOTER_SYSTEM_CODE_EN,
	Rsc_Layout_Footer_System_Code_Fr[] = RSC_LAYOUT_FOOTER_SYSTEM_CODE_FR;
#endif /* NO_FOOTER */

ATTRIBUTE Rsc_Layout_Card_Core_Online_Attr[] = RSC_LAYOUT_CARD_CORE_ONLINE_ATTR;
ASCII	Rsc_Layout_Card_Core_Online_Code_En[2][12] = {
					RSC_LAYOUT_CARD_CORE_ONLINE_COND0_CODE,
					RSC_LAYOUT_CARD_CORE_ONLINE_COND1_CODE
	};
#define Rsc_Layout_Card_Core_Online_Code_Fr Rsc_Layout_Card_Core_Online_Code_En

ATTRIBUTE Rsc_Layout_Card_Core_Offline_Attr[]=RSC_LAYOUT_CARD_CORE_OFFLINE_ATTR;
ASCII  Rsc_Layout_Card_Core_Offline_Code_En[]=RSC_LAYOUT_CARD_CORE_OFFLINE_CODE;
#define Rsc_Layout_Card_Core_Offline_Code_Fr \
					Rsc_Layout_Card_Core_Offline_Code_En

ATTRIBUTE Rsc_Layout_Card_CLK_Attr[] = RSC_LAYOUT_CARD_CLK_ATTR;
ASCII	Rsc_Layout_Card_CLK_Code_En[] = RSC_LAYOUT_CARD_CLK_CODE;
#define Rsc_Layout_Card_CLK_Code_Fr Rsc_Layout_Card_CLK_Code_En

ATTRIBUTE Rsc_Layout_Card_Uncore_Attr[] = RSC_LAYOUT_CARD_UNCORE_ATTR;
ASCII	Rsc_Layout_Card_Uncore_Code_En[] = RSC_LAYOUT_CARD_UNCORE_CODE;
#define Rsc_Layout_Card_Uncore_Code_Fr Rsc_Layout_Card_Uncore_Code_En

ATTRIBUTE Rsc_Layout_Card_Bus_Attr[] = RSC_LAYOUT_CARD_BUS_ATTR;
ASCII	Rsc_Layout_Card_Bus_Code_En[] = RSC_LAYOUT_CARD_BUS_CODE;
#define Rsc_Layout_Card_Bus_Code_Fr Rsc_Layout_Card_Bus_Code_En

ATTRIBUTE Rsc_Layout_Card_MC_Attr[] = RSC_LAYOUT_CARD_MC_ATTR;
ASCII   Rsc_Layout_Card_MC_Code_En[] = RSC_LAYOUT_CARD_MC_CODE;
#define Rsc_Layout_Card_MC_Code_Fr Rsc_Layout_Card_MC_Code_En

ATTRIBUTE Rsc_Layout_Card_Load_Attr[] = RSC_LAYOUT_CARD_LOAD_ATTR;
ASCII	Rsc_Layout_Card_Load_Code_En[] = RSC_LAYOUT_CARD_LOAD_CODE_EN,
	Rsc_Layout_Card_Load_Code_Fr[] = RSC_LAYOUT_CARD_LOAD_CODE_FR;

ATTRIBUTE Rsc_Layout_Card_Idle_Attr[] = RSC_LAYOUT_CARD_IDLE_ATTR;
ASCII	Rsc_Layout_Card_Idle_Code_En[] = RSC_LAYOUT_CARD_IDLE_CODE_EN,
	Rsc_Layout_Card_Idle_Code_Fr[] = RSC_LAYOUT_CARD_IDLE_CODE_FR;

ATTRIBUTE Rsc_Layout_Card_RAM_Attr[] = RSC_LAYOUT_CARD_RAM_ATTR;
ASCII   Rsc_Layout_Card_RAM_Code_En[] = RSC_LAYOUT_CARD_RAM_CODE;
#define Rsc_Layout_Card_RAM_Code_Fr Rsc_Layout_Card_RAM_Code_En

ATTRIBUTE Rsc_Layout_Card_Task_Attr[] = RSC_LAYOUT_CARD_TASK_ATTR;
ASCII	Rsc_Layout_Card_Task_Code_En[] = RSC_LAYOUT_CARD_TASK_CODE_EN,
	Rsc_Layout_Card_Task_Code_Fr[] = RSC_LAYOUT_CARD_TASK_CODE_FR;

ATTRIBUTE	Rsc_SymbolRunColor_Attr[]	= RSC_RUN_STATE_COLOR_ATTR,
		Rsc_SymbolUnIntColor_Attr[]	= RSC_UNINT_STATE_COLOR_ATTR,
		Rsc_SymbolZombieColor_Attr[]	= RSC_ZOMBIE_STATE_COLOR_ATTR,
		Rsc_SymbolSleepColor_Attr[]	= RSC_SLEEP_STATE_COLOR_ATTR,
		Rsc_SymbolWaitColor_Attr[]	= RSC_WAIT_STATE_COLOR_ATTR,
		Rsc_SymbolOtherColor_Attr[]	= RSC_OTHER_STATE_COLOR_ATTR,
		Rsc_SymbolTrackerColor_Attr[]	= RSC_TRACKER_STATE_COLOR_ATTR;

ATTRIBUTE Rsc_SysInfoCPUID_Cond_Attr[4][74] = {
	RSC_SYSINFO_CPUID_COND0_ATTR,
	RSC_SYSINFO_CPUID_COND1_ATTR,
	RSC_SYSINFO_CPUID_COND2_ATTR,
	RSC_SYSINFO_CPUID_COND3_ATTR
};

ATTRIBUTE Rsc_SystemRegisters_Cond_Attr[5][4] = {
	RSC_SYSTEM_REGISTERS_COND0_ATTR,
	RSC_SYSTEM_REGISTERS_COND1_ATTR,
	RSC_SYSTEM_REGISTERS_COND2_ATTR,
	RSC_SYSTEM_REGISTERS_COND3_ATTR,
	RSC_SYSTEM_REGISTERS_COND4_ATTR
};

ATTRIBUTE Rsc_SysInfoProc_Cond_Attr[4][76] = {
	RSC_SYSINFO_PROC_COND0_ATTR,
	RSC_SYSINFO_PROC_COND1_ATTR,
	RSC_SYSINFO_PROC_COND2_ATTR,
	RSC_SYSINFO_PROC_COND3_ATTR
};

ATTRIBUTE Rsc_SysInfoISA_Cond_Attr[2][5][17] = {
	RSC_SYSINFO_ISA_COND0_ATTR,
	RSC_SYSINFO_ISA_COND1_ATTR
};

ATTRIBUTE Rsc_SysInfoFeatures_Cond_Attr[5][72] = {
	RSC_SYSINFO_FEATURES_COND0_ATTR,
	RSC_SYSINFO_FEATURES_COND1_ATTR,
	RSC_SYSINFO_FEATURES_COND2_ATTR,
	RSC_SYSINFO_FEATURES_COND3_ATTR,
	RSC_SYSINFO_FEATURES_COND4_ATTR
};

ATTRIBUTE Rsc_SysInfoTech_Cond_Attr[2][50] = {
	RSC_SYSINFO_TECH_COND0_ATTR,
	RSC_SYSINFO_TECH_COND1_ATTR
};

ATTRIBUTE Rsc_SysInfoPerfMon_Cond_Attr[5][74] = {
	RSC_SYSINFO_PERFMON_COND0_ATTR,
	RSC_SYSINFO_PERFMON_COND1_ATTR,
	RSC_SYSINFO_PERFMON_COND2_ATTR,
	RSC_SYSINFO_PERFMON_COND3_ATTR,
	RSC_SYSINFO_PERFMON_COND4_ATTR
};

ATTRIBUTE Rsc_SysInfoPerfMon_HWP_Cap_Attr[2][76] = {
	RSC_SYSINFO_PERFMON_HWP_CAP_COND0_ATTR,
	RSC_SYSINFO_PERFMON_HWP_CAP_COND1_ATTR
};

ATTRIBUTE Rsc_SysInfoPwrThermal_Cond_Attr[6][50] = {
	RSC_SYSINFO_PWR_THERMAL_COND0_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND1_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND2_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND3_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND4_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND5_ATTR
};

ATTRIBUTE Rsc_SysInfoKernel_Attr[] = RSC_SYSINFO_KERNEL_ATTR;

ATTRIBUTE Rsc_Topology_Cond_Attr[5][13] = {
	RSC_TOPOLOGY_COND0_ATTR,
	RSC_TOPOLOGY_COND1_ATTR,
	RSC_TOPOLOGY_COND2_ATTR,
	RSC_TOPOLOGY_COND3_ATTR,
	RSC_TOPOLOGY_COND4_ATTR
};

ATTRIBUTE Rsc_MemoryController_Cond_Attr[2][14] = {
	RSC_MEMORY_CONTROLLER_COND0_ATTR,
	RSC_MEMORY_CONTROLLER_COND1_ATTR
};

#define RSC_DDR3_CL_COMM_CODE_FR	RSC_DDR3_CL_COMM_CODE_EN
#define RSC_DDR3_RCD_COMM_CODE_FR	RSC_DDR3_RCD_COMM_CODE_EN
#define RSC_DDR3_RP_COMM_CODE_FR	RSC_DDR3_RP_COMM_CODE_EN
#define RSC_DDR3_RAS_COMM_CODE_FR	RSC_DDR3_RAS_COMM_CODE_EN
#define RSC_DDR3_RRD_COMM_CODE_FR	RSC_DDR3_RRD_COMM_CODE_EN
#define RSC_DDR3_RFC_COMM_CODE_FR	RSC_DDR3_RFC_COMM_CODE_EN
#define RSC_DDR3_WR_COMM_CODE_FR	RSC_DDR3_WR_COMM_CODE_EN
#define RSC_DDR3_RTP_COMM_CODE_FR	RSC_DDR3_RTP_COMM_CODE_EN
#define RSC_DDR3_WTP_COMM_CODE_FR	RSC_DDR3_WTP_COMM_CODE_EN
#define RSC_DDR3_FAW_COMM_CODE_FR	RSC_DDR3_FAW_COMM_CODE_EN
#define RSC_DDR3_B2B_COMM_CODE_FR	RSC_DDR3_B2B_COMM_CODE_EN
#define RSC_DDR3_CWL_COMM_CODE_FR	RSC_DDR3_CWL_COMM_CODE_EN
#define RSC_DDR3_CMD_COMM_CODE_FR	RSC_DDR3_CMD_COMM_CODE_EN
#define RSC_DDR3_REFI_COMM_CODE_FR	RSC_DDR3_REFI_COMM_CODE_EN
#define RSC_DDR3_DDWRTRD_COMM_CODE_FR	RSC_DDR3_DDWRTRD_COMM_CODE_EN
#define RSC_DDR3_DRWRTRD_COMM_CODE_FR	RSC_DDR3_DRWRTRD_COMM_CODE_EN
#define RSC_DDR3_SRWRTRD_COMM_CODE_FR	RSC_DDR3_SRWRTRD_COMM_CODE_EN
#define RSC_DDR3_DDRDTWR_COMM_CODE_FR	RSC_DDR3_DDRDTWR_COMM_CODE_EN
#define RSC_DDR3_DRRDTWR_COMM_CODE_FR	RSC_DDR3_DRRDTWR_COMM_CODE_EN
#define RSC_DDR3_SRRDTWR_COMM_CODE_FR	RSC_DDR3_SRRDTWR_COMM_CODE_EN
#define RSC_DDR3_DDRDTRD_COMM_CODE_FR	RSC_DDR3_DDRDTRD_COMM_CODE_EN
#define RSC_DDR3_DRRDTRD_COMM_CODE_FR	RSC_DDR3_DRRDTRD_COMM_CODE_EN
#define RSC_DDR3_SRRDTRD_COMM_CODE_FR	RSC_DDR3_SRRDTRD_COMM_CODE_EN
#define RSC_DDR3_DDWRTWR_COMM_CODE_FR	RSC_DDR3_DDWRTWR_COMM_CODE_EN
#define RSC_DDR3_DRWRTWR_COMM_CODE_FR	RSC_DDR3_DRWRTWR_COMM_CODE_EN
#define RSC_DDR3_SRWRTWR_COMM_CODE_FR	RSC_DDR3_SRWRTWR_COMM_CODE_EN
#define RSC_DDR3_CKE_COMM_CODE_FR	RSC_DDR3_CKE_COMM_CODE_EN
#define RSC_DDR3_ECC_COMM_CODE_FR	RSC_DDR3_ECC_COMM_CODE_EN

#define RSC_DDR4_RDRD_SCL_COMM_CODE_FR	RSC_DDR4_RDRD_SCL_COMM_CODE_EN
#define RSC_DDR4_RDRD_SC_COMM_CODE_FR	RSC_DDR4_RDRD_SC_COMM_CODE_EN
#define RSC_DDR4_RDRD_SD_COMM_CODE_FR	RSC_DDR4_RDRD_SD_COMM_CODE_EN
#define RSC_DDR4_RDRD_DD_COMM_CODE_FR	RSC_DDR4_RDRD_DD_COMM_CODE_EN
#define RSC_DDR4_RDWR_SCL_COMM_CODE_FR	RSC_DDR4_RDWR_SCL_COMM_CODE_EN
#define RSC_DDR4_RDWR_SC_COMM_CODE_FR	RSC_DDR4_RDWR_SC_COMM_CODE_EN
#define RSC_DDR4_RDWR_SD_COMM_CODE_FR	RSC_DDR4_RDWR_SD_COMM_CODE_EN
#define RSC_DDR4_RDWR_DD_COMM_CODE_FR	RSC_DDR4_RDWR_DD_COMM_CODE_EN
#define RSC_DDR4_WRRD_SCL_COMM_CODE_FR	RSC_DDR4_WRRD_SCL_COMM_CODE_EN
#define RSC_DDR4_WRRD_SC_COMM_CODE_FR	RSC_DDR4_WRRD_SC_COMM_CODE_EN
#define RSC_DDR4_WRRD_SD_COMM_CODE_FR	RSC_DDR4_WRRD_SD_COMM_CODE_EN
#define RSC_DDR4_WRRD_DD_COMM_CODE_FR	RSC_DDR4_WRRD_DD_COMM_CODE_EN
#define RSC_DDR4_WRWR_SCL_COMM_CODE_FR	RSC_DDR4_WRWR_SCL_COMM_CODE_EN
#define RSC_DDR4_WRWR_SC_COMM_CODE_FR	RSC_DDR4_WRWR_SC_COMM_CODE_EN
#define RSC_DDR4_WRWR_SD_COMM_CODE_FR	RSC_DDR4_WRWR_SD_COMM_CODE_EN
#define RSC_DDR4_WRWR_DD_COMM_CODE_FR	RSC_DDR4_WRWR_DD_COMM_CODE_EN

#define RSC_DDR4_ZEN_RCD_R_COMM_CODE_FR RSC_DDR4_ZEN_RCD_R_COMM_CODE_EN
#define RSC_DDR4_ZEN_RCD_W_COMM_CODE_FR RSC_DDR4_ZEN_RCD_W_COMM_CODE_EN
#define RSC_DDR4_ZEN_RC_COMM_CODE_FR	RSC_DDR4_ZEN_RC_COMM_CODE_EN
#define RSC_DDR4_ZEN_RRD_S_COMM_CODE_FR RSC_DDR4_ZEN_RRD_S_COMM_CODE_EN
#define RSC_DDR4_ZEN_RRD_L_COMM_CODE_FR RSC_DDR4_ZEN_RRD_L_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTR_S_COMM_CODE_FR RSC_DDR4_ZEN_WTR_S_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTR_L_COMM_CODE_FR RSC_DDR4_ZEN_WTR_L_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_EN
#define RSC_DDR4_ZEN_RTP_COMM_CODE_FR	RSC_DDR4_ZEN_RTP_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDWR_COMM_CODE_FR	RSC_DDR4_ZEN_RDWR_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRRD_COMM_CODE_FR	RSC_DDR4_ZEN_WRRD_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_EN
#define RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_FR RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_FR RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_FR RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_FR RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_RFC1_COMM_CODE_FR	RSC_DDR4_ZEN_RFC1_COMM_CODE_EN
#define RSC_DDR4_ZEN_RFC2_COMM_CODE_FR	RSC_DDR4_ZEN_RFC2_COMM_CODE_EN
#define RSC_DDR4_ZEN_RFC4_COMM_CODE_FR	RSC_DDR4_ZEN_RFC4_COMM_CODE_EN
#define RSC_DDR4_ZEN_RCPB_COMM_CODE_FR	RSC_DDR4_ZEN_RCPB_COMM_CODE_EN
#define RSC_DDR4_ZEN_RPPB_COMM_CODE_FR	RSC_DDR4_ZEN_RPPB_COMM_CODE_EN
#define RSC_DDR4_ZEN_BGS_COMM_CODE_FR	RSC_DDR4_ZEN_BGS_COMM_CODE_EN
#define RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_FR RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_EN
#define RSC_DDR4_ZEN_BAN_COMM_CODE_FR	RSC_DDR4_ZEN_BAN_COMM_CODE_EN
#define RSC_DDR4_ZEN_RCPAGE_COMM_CODE_FR RSC_DDR4_ZEN_RCPAGE_COMM_CODE_EN
#define RSC_DDR4_ZEN_GDM_COMM_CODE_FR	RSC_DDR4_ZEN_GDM_COMM_CODE_EN

ATTRIBUTE Rsc_CreateMenu_Disable_Attr[] = RSC_CREATE_MENU_DISABLE_ATTR,
	Rsc_CreateMenu_Menu_Attr[]	= RSC_CREATE_MENU_ITEM_MENU_ATTR,
	Rsc_CreateMenu_View_Attr[]	= RSC_CREATE_MENU_ITEM_VIEW_ATTR,
	Rsc_CreateMenu_Window_Attr[]	= RSC_CREATE_MENU_ITEM_WINDOW_ATTR,
	Rsc_CreateMenu_FnKey_Attr[]	= RSC_CREATE_MENU_FN_KEY_ATTR,
	Rsc_CreateMenu_ShortKey_Attr[]	= RSC_CREATE_MENU_SHORTKEY_ATTR,
	Rsc_CreateMenu_CtrlKey_Attr[]	= RSC_CREATE_MENU_CTRL_KEY_ATTR;

ATTRIBUTE Rsc_CreateSettings_Cond_Attr[2][32] = {
	RSC_CREATE_SETTINGS_COND0_ATTR,
	RSC_CREATE_SETTINGS_COND1_ATTR
};

ATTRIBUTE Rsc_CreateAdvHelp_Cond_Attr[2][38] = {
	RSC_CREATE_ADV_HELP_COND0_ATTR,
	RSC_CREATE_ADV_HELP_COND1_ATTR
};

ATTRIBUTE Rsc_CreateHotPlugCPU_Enable_Attr[]=RSC_CREATE_HOTPLUG_CPU_ENABLE_ATTR,
	Rsc_CreateHotPlugCPU_Disable_Attr[]=RSC_CREATE_HOTPLUG_CPU_DISABLE_ATTR,
	Rsc_CreateHotPlugCPU_OnLine_Attr[]=RSC_CREATE_HOTPLUG_CPU_ONLINE_ATTR,
	Rsc_CreateHotPlugCPU_OffLine_Attr[]=RSC_CREATE_HOTPLUG_CPU_OFFLINE_ATTR;

ATTRIBUTE Rsc_CreateRatioClock_Cond_Attr[7][29] = {
	RSC_CREATE_RATIO_CLOCK_COND0_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND1_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND2_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND3_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND4_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND5_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND6_ATTR
};

ATTRIBUTE Rsc_CreateSelectCPU_Cond_Attr[3][36] = {
	RSC_CREATE_SELECT_CPU_COND0_ATTR,
	RSC_CREATE_SELECT_CPU_COND1_ATTR,
	RSC_CREATE_SELECT_CPU_COND2_ATTR
};

#ifndef NO_FOOTER
ATTRIBUTE Rsc_HotEvent_Cond_Attr[5][3] = {
	RSC_HOT_EVENT_COND0_ATTR,
	RSC_HOT_EVENT_COND1_ATTR,
	RSC_HOT_EVENT_COND2_ATTR,
	RSC_HOT_EVENT_COND3_ATTR,
	RSC_HOT_EVENT_COND4_ATTR
};
#endif

ATTRIBUTE Rsc_BoxEvent_Attr[] = RSC_BOX_EVENT_ATTR;

ATTRIBUTE Rsc_CreateRecorder_Attr[] = RSC_CREATE_RECORDER_ATTR;

ATTRIBUTE Rsc_SMBIOS_Item_Attr[] = RSC_SMBIOS_ITEM_ATTR;

ATTRIBUTE Rsc_CreateSelectFreq_Pkg_Attr[] = RSC_CREATE_SELECT_FREQ_PKG_ATTR;

ATTRIBUTE Rsc_CreateSelectFreq_Cond_Attr[2][46] = {
	RSC_CREATE_SELECT_FREQ_COND0_ATTR,
	RSC_CREATE_SELECT_FREQ_COND1_ATTR
};

#define RSC_SYS_REG_FLAGS_TF_CODE_FR	RSC_SYS_REG_FLAGS_TF_CODE_EN
#define RSC_SYS_REG_FLAGS_IF_CODE_FR	RSC_SYS_REG_FLAGS_IF_CODE_EN
#define RSC_SYS_REG_FLAGS_IOPL_CODE_FR	RSC_SYS_REG_FLAGS_IOPL_CODE_EN
#define RSC_SYS_REG_FLAGS_NT_CODE_FR	RSC_SYS_REG_FLAGS_NT_CODE_EN
#define RSC_SYS_REG_FLAGS_RF_CODE_FR	RSC_SYS_REG_FLAGS_RF_CODE_EN
#define RSC_SYS_REG_FLAGS_VM_CODE_FR	RSC_SYS_REG_FLAGS_VM_CODE_EN
#define RSC_SYS_REG_FLAGS_AC_CODE_FR	RSC_SYS_REG_FLAGS_AC_CODE_EN
#define RSC_SYS_REG_FLAGS_VIF_CODE_FR	RSC_SYS_REG_FLAGS_VIF_CODE_EN
#define RSC_SYS_REG_FLAGS_VIP_CODE_FR	RSC_SYS_REG_FLAGS_VIP_CODE_EN
#define RSC_SYS_REG_FLAGS_ID_CODE_FR	RSC_SYS_REG_FLAGS_ID_CODE_EN
#define RSC_SYS_REGS_CR0_CODE_FR	RSC_SYS_REGS_CR0_CODE_EN
#define RSC_SYS_REG_CR0_PE_CODE_FR	RSC_SYS_REG_CR0_PE_CODE_EN
#define RSC_SYS_REG_CR0_MP_CODE_FR	RSC_SYS_REG_CR0_MP_CODE_EN
#define RSC_SYS_REG_CR0_EM_CODE_FR	RSC_SYS_REG_CR0_EM_CODE_EN
#define RSC_SYS_REG_CR0_TS_CODE_FR	RSC_SYS_REG_CR0_TS_CODE_EN
#define RSC_SYS_REG_CR0_ET_CODE_FR	RSC_SYS_REG_CR0_ET_CODE_EN
#define RSC_SYS_REG_CR0_NE_CODE_FR	RSC_SYS_REG_CR0_NE_CODE_EN
#define RSC_SYS_REG_CR0_WP_CODE_FR	RSC_SYS_REG_CR0_WP_CODE_EN
#define RSC_SYS_REG_CR0_AM_CODE_FR	RSC_SYS_REG_CR0_AM_CODE_EN
#define RSC_SYS_REG_CR0_NW_CODE_FR	RSC_SYS_REG_CR0_NW_CODE_EN
#define RSC_SYS_REG_CR0_CD_CODE_FR	RSC_SYS_REG_CR0_CD_CODE_EN
#define RSC_SYS_REG_CR0_PG_CODE_FR	RSC_SYS_REG_CR0_PG_CODE_EN
#define RSC_SYS_REGS_CR3_CODE_FR	RSC_SYS_REGS_CR3_CODE_EN
#define RSC_SYS_REG_CR3_PWT_CODE_FR	RSC_SYS_REG_CR3_PWT_CODE_EN
#define RSC_SYS_REG_CR3_PCD_CODE_FR	RSC_SYS_REG_CR3_PCD_CODE_EN
#define RSC_SYS_REGS_CR4_CODE_FR	RSC_SYS_REGS_CR4_CODE_EN
#define RSC_SYS_REG_CR4_VME_CODE_FR	RSC_SYS_REG_CR4_VME_CODE_EN
#define RSC_SYS_REG_CR4_PVI_CODE_FR	RSC_SYS_REG_CR4_PVI_CODE_EN
#define RSC_SYS_REG_CR4_TSD_CODE_FR	RSC_SYS_REG_CR4_TSD_CODE_EN
#define RSC_SYS_REG_CR4_DE_CODE_FR	RSC_SYS_REG_CR4_DE_CODE_EN
#define RSC_SYS_REG_CR4_PSE_CODE_FR	RSC_SYS_REG_CR4_PSE_CODE_EN
#define RSC_SYS_REG_CR4_PAE_CODE_FR	RSC_SYS_REG_CR4_PAE_CODE_EN
#define RSC_SYS_REG_CR4_MCE_CODE_FR	RSC_SYS_REG_CR4_MCE_CODE_EN
#define RSC_SYS_REG_CR4_PGE_CODE_FR	RSC_SYS_REG_CR4_PGE_CODE_EN
#define RSC_SYS_REG_CR4_PCE_CODE_FR	RSC_SYS_REG_CR4_PCE_CODE_EN
#define RSC_SYS_REG_CR4_FX_CODE_FR	RSC_SYS_REG_CR4_FX_CODE_EN
#define RSC_SYS_REG_CR4_XMM_CODE_FR	RSC_SYS_REG_CR4_XMM_CODE_EN
#define RSC_SYS_REG_CR4_UMIP_CODE_FR	RSC_SYS_REG_CR4_UMIP_CODE_EN
#define RSC_SYS_REG_CR4_5LP_CODE_FR	RSC_SYS_REG_CR4_5LP_CODE_EN
#define RSC_SYS_REG_CR4_VMX_CODE_FR	RSC_SYS_REG_CR4_VMX_CODE_EN
#define RSC_SYS_REG_CR4_SMX_CODE_FR	RSC_SYS_REG_CR4_SMX_CODE_EN
#define RSC_SYS_REG_CR4_FS_CODE_FR	RSC_SYS_REG_CR4_FS_CODE_EN
#define RSC_SYS_REG_CR4_PCID_CODE_FR	RSC_SYS_REG_CR4_PCID_CODE_EN
#define RSC_SYS_REG_CR4_SAV_CODE_FR	RSC_SYS_REG_CR4_SAV_CODE_EN
#define RSC_SYS_REG_CR4_KL_CODE_FR	RSC_SYS_REG_CR4_KL_CODE_EN
#define RSC_SYS_REG_CR4_SME_CODE_FR	RSC_SYS_REG_CR4_SME_CODE_EN
#define RSC_SYS_REG_CR4_SMA_CODE_FR	RSC_SYS_REG_CR4_SMA_CODE_EN
#define RSC_SYS_REG_CR4_PKE_CODE_FR	RSC_SYS_REG_CR4_PKE_CODE_EN
#define RSC_SYS_REG_CR4_CET_CODE_FR	RSC_SYS_REG_CR4_CET_CODE_EN
#define RSC_SYS_REG_CR4_PKS_CODE_FR	RSC_SYS_REG_CR4_PKS_CODE_EN
#define RSC_SYS_REGS_CR8_CODE_FR	RSC_SYS_REGS_CR8_CODE_EN
#define RSC_SYS_REG_CR8_TPL_CODE_FR	RSC_SYS_REG_CR8_TPL_CODE_EN
#define RSC_SYS_REGS_EFCR_CODE_FR	RSC_SYS_REGS_EFCR_CODE_EN
#define RSC_SYS_REG_EFCR_LCK_CODE_FR	RSC_SYS_REG_EFCR_LCK_CODE_EN
#define RSC_SYS_REG_EFCR_VMX_CODE_FR	RSC_SYS_REG_EFCR_VMX_CODE_EN
#define RSC_SYS_REG_EFCR_SGX_CODE_FR	RSC_SYS_REG_EFCR_SGX_CODE_EN
#define RSC_SYS_REG_EFCR_LSE_CODE_FR	RSC_SYS_REG_EFCR_LSE_CODE_EN
#define RSC_SYS_REG_EFCR_GSE_CODE_FR	RSC_SYS_REG_EFCR_GSE_CODE_EN
#define RSC_SYS_REG_EFCR_LSGX_CODE_FR	RSC_SYS_REG_EFCR_LSGX_CODE_EN
#define RSC_SYS_REG_EFCR_GSGX_CODE_FR	RSC_SYS_REG_EFCR_GSGX_CODE_EN
#define RSC_SYS_REG_EFCR_LMC_CODE_FR	RSC_SYS_REG_EFCR_LMC_CODE_EN
#define RSC_SYS_REGS_EFER_CODE_FR	RSC_SYS_REGS_EFER_CODE_EN
#define RSC_SYS_REG_EFER_SCE_CODE_FR	RSC_SYS_REG_EFER_SCE_CODE_EN
#define RSC_SYS_REG_EFER_LME_CODE_FR	RSC_SYS_REG_EFER_LME_CODE_EN
#define RSC_SYS_REG_EFER_LMA_CODE_FR	RSC_SYS_REG_EFER_LMA_CODE_EN
#define RSC_SYS_REG_EFER_NXE_CODE_FR	RSC_SYS_REG_EFER_NXE_CODE_EN
#define RSC_SYS_REG_EFER_SVM_CODE_FR	RSC_SYS_REG_EFER_SVM_CODE_EN

#define RSC_ISA_3DNOW_COMM_CODE_FR	RSC_ISA_3DNOW_COMM_CODE_EN
#define RSC_ISA_ADX_COMM_CODE_FR	RSC_ISA_ADX_COMM_CODE_EN
#define RSC_ISA_AES_COMM_CODE_FR	RSC_ISA_AES_COMM_CODE_EN
#define RSC_ISA_AVX_COMM_CODE_FR	RSC_ISA_AVX_COMM_CODE_EN
#define RSC_ISA_BMI_COMM_CODE_FR	RSC_ISA_BMI_COMM_CODE_EN
#define RSC_ISA_CLWB_COMM_CODE_FR	RSC_ISA_CLWB_COMM_CODE_EN
#define RSC_ISA_CLFLUSH_COMM_CODE_FR	RSC_ISA_CLFLUSH_COMM_CODE_EN
#define RSC_ISA_AC_FLAG_COMM_CODE_FR	RSC_ISA_AC_FLAG_COMM_CODE_EN
#define RSC_ISA_CMOV_COMM_CODE_FR	RSC_ISA_CMOV_COMM_CODE_EN
#define RSC_ISA_XCHG8B_COMM_CODE_FR	RSC_ISA_XCHG8B_COMM_CODE_EN
#define RSC_ISA_XCHG16B_COMM_CODE_FR	RSC_ISA_XCHG16B_COMM_CODE_EN
#define RSC_ISA_F16C_COMM_CODE_FR	RSC_ISA_F16C_COMM_CODE_EN
#define RSC_ISA_FPU_COMM_CODE_FR	RSC_ISA_FPU_COMM_CODE_EN
#define RSC_ISA_FXSR_COMM_CODE_FR	RSC_ISA_FXSR_COMM_CODE_EN
#define RSC_ISA_LSHF_COMM_CODE_FR	RSC_ISA_LSHF_COMM_CODE_EN
#define RSC_ISA_MMX_COMM_CODE_FR	RSC_ISA_MMX_COMM_CODE_EN
#define RSC_ISA_MWAITX_COMM_CODE_FR	RSC_ISA_MWAITX_COMM_CODE_EN
#define RSC_ISA_MOVBE_COMM_CODE_FR	RSC_ISA_MOVBE_COMM_CODE_EN
#define RSC_ISA_PCLMULDQ_COMM_CODE_FR	RSC_ISA_PCLMULDQ_COMM_CODE_EN
#define RSC_ISA_POPCNT_COMM_CODE_FR	RSC_ISA_POPCNT_COMM_CODE_EN
#define RSC_ISA_RDRAND_COMM_CODE_FR	RSC_ISA_RDRAND_COMM_CODE_EN
#define RSC_ISA_RDSEED_COMM_CODE_FR	RSC_ISA_RDSEED_COMM_CODE_EN
#define RSC_ISA_RDTSCP_COMM_CODE_FR	RSC_ISA_RDTSCP_COMM_CODE_EN
#define RSC_ISA_SEP_COMM_CODE_FR	RSC_ISA_SEP_COMM_CODE_EN
#define RSC_ISA_SHA_COMM_CODE_FR	RSC_ISA_SHA_COMM_CODE_EN
#define RSC_ISA_SSE_COMM_CODE_FR	RSC_ISA_SSE_COMM_CODE_EN
#define RSC_ISA_SSE2_COMM_CODE_FR	RSC_ISA_SSE2_COMM_CODE_EN
#define RSC_ISA_SSE3_COMM_CODE_FR	RSC_ISA_SSE3_COMM_CODE_EN
#define RSC_ISA_SSSE3_COMM_CODE_FR	RSC_ISA_SSSE3_COMM_CODE_EN
#define RSC_ISA_SSE4_1_COMM_CODE_FR	RSC_ISA_SSE4_1_COMM_CODE_EN
#define RSC_ISA_SSE4_2_COMM_CODE_FR	RSC_ISA_SSE4_2_COMM_CODE_EN
#define RSC_ISA_SERIALIZE_COMM_CODE_FR	RSC_ISA_SERIALIZE_COMM_CODE_EN
#define RSC_ISA_SYSCALL_COMM_CODE_FR	RSC_ISA_SYSCALL_COMM_CODE_EN
#define RSC_ISA_RDPID_COMM_CODE_FR	RSC_ISA_RDPID_COMM_CODE_EN
#define RSC_ISA_UMIP_COMM_CODE_FR	RSC_ISA_UMIP_COMM_CODE_EN
#define RSC_ISA_SGX_COMM_CODE_FR	RSC_ISA_SGX_COMM_CODE_EN

#define LDV(attr_var, en_var, fr_var)					\
	.Attr = attr_var,						\
	.Code = {							\
		[LOC_EN] = (ASCII*) en_var,				\
		[LOC_FR] = (ASCII*) fr_var				\
	}								\

#define LDA(attr_var, en_var, fr_var)					\
{									\
	LDV(attr_var, en_var, fr_var),					\
	.Size = {							\
		[LOC_EN] = sizeof(en_var),				\
		[LOC_FR] = sizeof(fr_var)				\
	}								\
}

#define LDB(attr_var)							\
{									\
	LDV(attr_var, hSpace, hSpace),					\
	.Size = {							\
		[LOC_EN] = sizeof(HSPACE),				\
		[LOC_FR] = sizeof(HSPACE)				\
	}								\
}

#define LDN(attr_var, eq_var)						\
{									\
	LDV(attr_var, eq_var, eq_var),					\
	.Size = {							\
		[LOC_EN] = __builtin_strlen(eq_var),			\
		[LOC_FR] = __builtin_strlen(eq_var)			\
	}								\
}

#define LDS(attr_var, en_var, fr_var)					\
{									\
	LDV(attr_var, en_var, fr_var),					\
	.Size = {							\
		[LOC_EN] = __builtin_strlen(en_var),			\
		[LOC_FR] = __builtin_strlen(fr_var)			\
	}								\
}

#define LDT(en_var, fr_var)	LDS(vColor, en_var, fr_var)

#define LDQ(eq_var)		LDN(vColor, eq_var)

RESOURCE_ST Resource[] = {
#ifndef NO_HEADER
	[RSC_LAYOUT_HEADER_PROC] = LDA( Rsc_Layout_Header_Proc_Attr,
					Rsc_Layout_Header_Proc_Code_En,
					Rsc_Layout_Header_Proc_Code_Fr),
	[RSC_LAYOUT_HEADER_CPU] = LDA(	Rsc_Layout_Header_CPU_Attr,
					Rsc_Layout_Header_CPU_Code_En,
					Rsc_Layout_Header_CPU_Code_Fr),
	[RSC_LAYOUT_HEADER_ARCH] = LDA( Rsc_Layout_Header_Arch_Attr,
					Rsc_Layout_Header_Arch_Code_En,
					Rsc_Layout_Header_Arch_Code_Fr),
      [RSC_LAYOUT_HEADER_CACHE_L1]=LDA( Rsc_Layout_Header_Cache_L1_Attr,
					Rsc_Layout_Header_Cache_L1_Code_En,
					Rsc_Layout_Header_Cache_L1_Code_Fr),
	[RSC_LAYOUT_HEADER_BCLK] = LDA( Rsc_Layout_Header_BClk_Attr,
					Rsc_Layout_Header_BClk_Code_En,
					Rsc_Layout_Header_BClk_Code_Fr),
	[RSC_LAYOUT_HEADER_CACHES]=LDA( Rsc_Layout_Header_Caches_Attr,
					Rsc_Layout_Header_Caches_Code_En,
					Rsc_Layout_Header_Caches_Code_Fr),
#endif /* NO_HEADER */
#ifndef NO_UPPER
	[RSC_LAYOUT_RULER_LOAD] = LDA(	Rsc_Layout_Ruler_Load_Attr,
					Rsc_Layout_Ruler_Load_Code_En,
					Rsc_Layout_Ruler_Load_Code_Fr),
	[RSC_LAYOUT_RULER_REL_LOAD]=LDA(Rsc_Layout_Ruler_Rel_Load_Attr,
					Rsc_Layout_Ruler_Rel_Load_Code_En,
					Rsc_Layout_Ruler_Rel_Load_Code_Fr),
	[RSC_LAYOUT_RULER_ABS_LOAD]=LDA(Rsc_Layout_Ruler_Abs_Load_Attr,
					Rsc_Layout_Ruler_Abs_Load_Code_En,
					Rsc_Layout_Ruler_Abs_Load_Code_Fr),
#endif /* NO_UPPER */
#ifndef NO_LOWER
    [RSC_LAYOUT_MONITOR_FREQUENCY]=LDA( Rsc_Layout_Monitor_Frequency_Attr,
					Rsc_Layout_Monitor_Frequency_Code_En,
					Rsc_Layout_Monitor_Frequency_Code_Fr),
	[RSC_LAYOUT_MONITOR_INST] = LDA(Rsc_Layout_Monitor_Inst_Attr,
					Rsc_Layout_Monitor_Inst_Code_En,
					Rsc_Layout_Monitor_Inst_Code_Fr),
	[RSC_LAYOUT_MONITOR_COMMON]=LDA(Rsc_Layout_Monitor_Common_Attr,
					Rsc_Layout_Monitor_Common_Code_En,
					Rsc_Layout_Monitor_Common_Code_Fr),
	[RSC_LAYOUT_MONITOR_TASKS]=LDA( Rsc_Layout_Monitor_Tasks_Attr,
					Rsc_Layout_Monitor_Tasks_Code_En,
					Rsc_Layout_Monitor_Tasks_Code_Fr),
	[RSC_LAYOUT_MONITOR_SLICE]=LDA( Rsc_Layout_Monitor_Slice_Attr,
					Rsc_Layout_Monitor_Slice_Code_En,
					Rsc_Layout_Monitor_Slice_Code_Fr),
      [RSC_LAYOUT_RULER_FREQUENCY]=LDA( Rsc_Layout_Ruler_Frequency_Attr,
					Rsc_Layout_Ruler_Frequency_Code_En,
					Rsc_Layout_Ruler_Frequency_Code_Fr),
  [RSC_LAYOUT_RULER_FREQUENCY_AVG]= LDA(Rsc_Layout_Ruler_Freq_Avg_Attr,
					Rsc_Layout_Ruler_Freq_Avg_Code_En,
					Rsc_Layout_Ruler_Freq_Avg_Code_Fr),
  [RSC_LAYOUT_RULER_FREQUENCY_PKG]= LDA(Rsc_Layout_Ruler_Freq_Pkg_Attr,
					Rsc_Layout_Ruler_Freq_Pkg_Code_En,
					Rsc_Layout_Ruler_Freq_Pkg_Code_Fr),
	[RSC_LAYOUT_RULER_INST] = LDA(	Rsc_Layout_Ruler_Inst_Attr,
					Rsc_Layout_Ruler_Inst_Code_En,
					Rsc_Layout_Ruler_Inst_Code_Fr),
	[RSC_LAYOUT_RULER_CYCLES] = LDA(Rsc_Layout_Ruler_Cycles_Attr,
					Rsc_Layout_Ruler_Cycles_Code_En,
					Rsc_Layout_Ruler_Cycles_Code_Fr),
	[RSC_LAYOUT_RULER_CSTATES]=LDA( Rsc_Layout_Ruler_CStates_Attr,
					Rsc_Layout_Ruler_CStates_Code_En,
					Rsc_Layout_Ruler_CStates_Code_Fr),
      [RSC_LAYOUT_RULER_INTERRUPTS]=LDA(Rsc_Layout_Ruler_Interrupts_Attr,
					Rsc_Layout_Ruler_Interrupts_Code_En,
					Rsc_Layout_Ruler_Interrupts_Code_Fr),
	[RSC_LAYOUT_RULER_PACKAGE]=LDA( Rsc_Layout_Ruler_Package_Attr,
					Rsc_Layout_Ruler_Package_Code_En,
					Rsc_Layout_Ruler_Package_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC] = LDA(	Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC_Code_En,
					Rsc_Layout_Package_PC_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC02] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC02_Code_En,
					Rsc_Layout_Package_PC02_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC03] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC03_Code_En,
					Rsc_Layout_Package_PC03_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC04] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC04_Code_En,
					Rsc_Layout_Package_PC04_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC06] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC06_Code_En,
					Rsc_Layout_Package_PC06_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC07] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC07_Code_En,
					Rsc_Layout_Package_PC07_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC08] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC08_Code_En,
					Rsc_Layout_Package_PC08_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC09] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC09_Code_En,
					Rsc_Layout_Package_PC09_Code_Fr),
	[RSC_LAYOUT_PACKAGE_PC10] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_PC10_Code_En,
					Rsc_Layout_Package_PC10_Code_Fr),
	[RSC_LAYOUT_PACKAGE_MC06] = LDA(Rsc_Layout_Package_PC_Attr,
					Rsc_Layout_Package_MC06_Code_En,
					Rsc_Layout_Package_MC06_Code_Fr),
      [RSC_LAYOUT_PACKAGE_UNCORE] = LDA(Rsc_Layout_Package_Uncore_Attr,
					Rsc_Layout_Package_Uncore_Code_En,
					Rsc_Layout_Package_Uncore_Code_Fr),
	[RSC_LAYOUT_RULER_TASKS] = LDA( Rsc_Layout_Ruler_Tasks_Attr,
					Rsc_Layout_Ruler_Tasks_Code_En,
					Rsc_Layout_Ruler_Tasks_Code_Fr),
      [RSC_LAYOUT_TASKS_TRACKING] = LDA(Rsc_Layout_Tasks_Tracking_Attr,
					Rsc_Layout_Tasks_Tracking_Code_En,
					Rsc_Layout_Tasks_Tracking_Code_Fr),
    [RSC_LAYOUT_TASKS_STATE_SORTED]=LDA(Rsc_Layout_Tasks_State_Sorted_Attr,
					Rsc_Layout_Tasks_State_Sorted_Code_En,
					Rsc_Layout_Tasks_State_Sorted_Code_Fr),
  [RSC_LAYOUT_TASKS_RUNTIME_SORTED]=LDA(Rsc_Layout_Tasks_RunTime_Sorted_Attr,
					Rsc_Layout_Tasks_RunTime_Sorted_Code_En,
					Rsc_Layout_Tasks_RunTime_Sorted_Code_Fr),
  [RSC_LAYOUT_TASKS_USRTIME_SORTED]=LDA(Rsc_Layout_Tasks_UsrTime_Sorted_Attr,
					Rsc_Layout_Tasks_UsrTime_Sorted_Code_En,
					Rsc_Layout_Tasks_UsrTime_Sorted_Code_Fr),
  [RSC_LAYOUT_TASKS_SYSTIME_SORTED]=LDA(Rsc_Layout_Tasks_SysTime_Sorted_Attr,
					Rsc_Layout_Tasks_SysTime_Sorted_Code_En,
					Rsc_Layout_Tasks_SysTime_Sorted_Code_Fr),
  [RSC_LAYOUT_TASKS_PROCESS_SORTED]=LDA(Rsc_Layout_Tasks_Process_Sorted_Attr,
					Rsc_Layout_Tasks_Process_Sorted_Code_En,
					Rsc_Layout_Tasks_Process_Sorted_Code_Fr),
  [RSC_LAYOUT_TASKS_COMMAND_SORTED]=LDA(Rsc_Layout_Tasks_Command_Sorted_Attr,
					Rsc_Layout_Tasks_Command_Sorted_Code_En,
					Rsc_Layout_Tasks_Command_Sorted_Code_Fr),
[RSC_LAYOUT_TASKS_REVERSE_SORT_OFF]=LDA(Rsc_Layout_Tasks_Reverse_Sort_Off_Attr,
				      Rsc_Layout_Tasks_Reverse_Sort_Off_Code_En,
				      Rsc_Layout_Tasks_Reverse_Sort_Off_Code_Fr),
[RSC_LAYOUT_TASKS_REVERSE_SORT_ON]=LDA(Rsc_Layout_Tasks_Reverse_Sort_On_Attr,
				      Rsc_Layout_Tasks_Reverse_Sort_On_Code_En,
				      Rsc_Layout_Tasks_Reverse_Sort_On_Code_Fr),
    [RSC_LAYOUT_TASKS_VALUE_SWITCH]=LDA(Rsc_Layout_Tasks_Value_Switch_Attr,
					Rsc_Layout_Tasks_Value_Switch_Code_En,
					Rsc_Layout_Tasks_Value_Switch_Code_Fr),
      [RSC_LAYOUT_TASKS_VALUE_OFF]=LDA( Rsc_Layout_Tasks_Value_Off_Attr,
					Rsc_Layout_Tasks_Value_Off_Code_En,
					Rsc_Layout_Tasks_Value_Off_Code_Fr),
	[RSC_LAYOUT_TASKS_VALUE_ON]=LDA(Rsc_Layout_Tasks_Value_On_Attr,
					Rsc_Layout_Tasks_Value_On_Code_En,
					Rsc_Layout_Tasks_Value_On_Code_Fr),
	[RSC_LAYOUT_RULER_SENSORS]=LDA( Rsc_Layout_Ruler_Sensors_Attr,
					Rsc_Layout_Ruler_Sensors_Code_En,
					Rsc_Layout_Ruler_Sensors_Code_Fr),
      [RSC_LAYOUT_RULER_PWR_UNCORE]=LDA(Rsc_Layout_Ruler_Power_Attr,
					Rsc_Layout_Ruler_Pwr_Uncore_Code_En,
					Rsc_Layout_Ruler_Pwr_Uncore_Code_Fr),
	[RSC_LAYOUT_RULER_PWR_SOC]=LDA( Rsc_Layout_Ruler_Power_Attr,
					Rsc_Layout_Ruler_Pwr_SoC_Code_En,
					Rsc_Layout_Ruler_Pwr_SoC_Code_Fr),
    [RSC_LAYOUT_RULER_PWR_PLATFORM]=LDA(Rsc_Layout_Ruler_Power_Attr,
					Rsc_Layout_Ruler_Pwr_Pfm_Code_En,
					Rsc_Layout_Ruler_Pwr_Pfm_Code_Fr),
	[RSC_LAYOUT_RULER_VOLTAGE]=LDA( Rsc_Layout_Ruler_Voltage_Attr,
					Rsc_Layout_Ruler_Voltage_Code_En,
					Rsc_Layout_Ruler_Voltage_Code_Fr),
	[RSC_LAYOUT_RULER_VPKG_SOC]=LDA(Rsc_Layout_Ruler_VPkg_SoC_Attr,
					Rsc_Layout_Ruler_VPkg_SoC_Code_En,
					Rsc_Layout_Ruler_VPkg_SoC_Code_Fr),
	[RSC_LAYOUT_RULER_ENERGY] = LDA(Rsc_Layout_Ruler_Energy_Attr,
					Rsc_Layout_Ruler_Energy_Code_En,
					Rsc_Layout_Ruler_Energy_Code_Fr),
	[RSC_LAYOUT_RULER_POWER] = LDA( Rsc_Layout_Ruler_Energy_Attr,
					Rsc_Layout_Ruler_Power_Code_En,
					Rsc_Layout_Ruler_Power_Code_Fr),
	[RSC_LAYOUT_RULER_SLICE] = LDA( Rsc_Layout_Ruler_Slice_Attr,
					Rsc_Layout_Ruler_Slice_Code_En,
					Rsc_Layout_Ruler_Slice_Code_Fr),
#endif /* NO_LOWER */
#ifndef NO_FOOTER
      [RSC_LAYOUT_FOOTER_TECH_X86]=LDA( Rsc_Layout_Footer_Tech_x86_Attr,
					Rsc_Layout_Footer_Tech_x86_Code_En,
					Rsc_Layout_Footer_Tech_x86_Code_Fr),
    [RSC_LAYOUT_FOOTER_TECH_INTEL]= LDA(Rsc_Layout_Footer_Tech_Intel_Attr,
					Rsc_Layout_Footer_Tech_Intel_Code_En,
					Rsc_Layout_Footer_Tech_Intel_Code_Fr),
      [RSC_LAYOUT_FOOTER_TECH_AMD]=LDA( Rsc_Layout_Footer_Tech_AMD_Attr,
					Rsc_Layout_Footer_Tech_AMD_Code_En,
					Rsc_Layout_Footer_Tech_AMD_Code_Fr),
    [RSC_LAYOUT_FOOTER_SYSTEM]	= LDA(	Rsc_Layout_Footer_System_Attr,
					Rsc_Layout_Footer_System_Code_En,
					Rsc_Layout_Footer_System_Code_Fr),
#endif /* NO_FOOTER */
[RSC_LAYOUT_CARD_CORE_ONLINE_COND0]=LDA(Rsc_Layout_Card_Core_Online_Attr,
					Rsc_Layout_Card_Core_Online_Code_En[0],
					Rsc_Layout_Card_Core_Online_Code_Fr[0]),
[RSC_LAYOUT_CARD_CORE_ONLINE_COND1]=LDA(Rsc_Layout_Card_Core_Online_Attr,
					Rsc_Layout_Card_Core_Online_Code_En[1],
					Rsc_Layout_Card_Core_Online_Code_Fr[1]),
    [RSC_LAYOUT_CARD_CORE_OFFLINE]=LDA( Rsc_Layout_Card_Core_Offline_Attr,
					Rsc_Layout_Card_Core_Offline_Code_En,
					Rsc_Layout_Card_Core_Offline_Code_Fr),
	[RSC_LAYOUT_CARD_CLK]	= LDA(	Rsc_Layout_Card_CLK_Attr,
					Rsc_Layout_Card_CLK_Code_En,
					Rsc_Layout_Card_CLK_Code_Fr),
	[RSC_LAYOUT_CARD_UNCORE] = LDA( Rsc_Layout_Card_Uncore_Attr,
					Rsc_Layout_Card_Uncore_Code_En,
					Rsc_Layout_Card_Uncore_Code_Fr),
	[RSC_LAYOUT_CARD_BUS]	= LDA(	Rsc_Layout_Card_Bus_Attr,
					Rsc_Layout_Card_Bus_Code_En,
					Rsc_Layout_Card_Bus_Code_Fr),
	[RSC_LAYOUT_CARD_MC]	= LDA(	Rsc_Layout_Card_MC_Attr,
					Rsc_Layout_Card_MC_Code_En,
					Rsc_Layout_Card_MC_Code_Fr),
	[RSC_LAYOUT_CARD_LOAD]	= LDA(	Rsc_Layout_Card_Load_Attr,
					Rsc_Layout_Card_Load_Code_En,
					Rsc_Layout_Card_Load_Code_Fr),
	[RSC_LAYOUT_CARD_IDLE]	= LDA(	Rsc_Layout_Card_Idle_Attr,
					Rsc_Layout_Card_Idle_Code_En,
					Rsc_Layout_Card_Idle_Code_Fr),
	[RSC_LAYOUT_CARD_RAM]	= LDA(	Rsc_Layout_Card_RAM_Attr,
					Rsc_Layout_Card_RAM_Code_En,
					Rsc_Layout_Card_RAM_Code_Fr),
	[RSC_LAYOUT_CARD_TASK]	= LDA(	Rsc_Layout_Card_Task_Attr,
					Rsc_Layout_Card_Task_Code_En,
					Rsc_Layout_Card_Task_Code_Fr),
/* ATTRIBUTE */
    [RSC_RUN_STATE_COLOR]	= LDB(	Rsc_SymbolRunColor_Attr),
    [RSC_UNINT_STATE_COLOR]	= LDB(	Rsc_SymbolUnIntColor_Attr),
    [RSC_ZOMBIE_STATE_COLOR]	= LDB(	Rsc_SymbolZombieColor_Attr),
    [RSC_SLEEP_STATE_COLOR]	= LDB(	Rsc_SymbolSleepColor_Attr),
    [RSC_WAIT_STATE_COLOR]	= LDB(	Rsc_SymbolWaitColor_Attr),
    [RSC_OTHER_STATE_COLOR]	= LDB(	Rsc_SymbolOtherColor_Attr),
#ifndef NO_LOWER
    [RSC_TRACKER_STATE_COLOR]	= LDB(	Rsc_SymbolTrackerColor_Attr),
#endif
    [RSC_SYSINFO_CPUID_COND0]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[0]),
    [RSC_SYSINFO_CPUID_COND1]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[1]),
    [RSC_SYSINFO_CPUID_COND2]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[2]),
    [RSC_SYSINFO_CPUID_COND3]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[3]),
    [RSC_SYSTEM_REGISTERS_COND0]= LDB(	Rsc_SystemRegisters_Cond_Attr[0]),
    [RSC_SYSTEM_REGISTERS_COND1]= LDB(	Rsc_SystemRegisters_Cond_Attr[1]),
    [RSC_SYSTEM_REGISTERS_COND2]= LDB(	Rsc_SystemRegisters_Cond_Attr[2]),
    [RSC_SYSTEM_REGISTERS_COND3]= LDB(	Rsc_SystemRegisters_Cond_Attr[3]),
    [RSC_SYSTEM_REGISTERS_COND4]= LDB(	Rsc_SystemRegisters_Cond_Attr[4]),
    [RSC_SYSINFO_PROC_COND0]	= LDB(	Rsc_SysInfoProc_Cond_Attr[0]),
    [RSC_SYSINFO_PROC_COND1]	= LDB(	Rsc_SysInfoProc_Cond_Attr[1]),
    [RSC_SYSINFO_PROC_COND2]	= LDB(	Rsc_SysInfoProc_Cond_Attr[2]),
    [RSC_SYSINFO_PROC_COND3]	= LDB(	Rsc_SysInfoProc_Cond_Attr[3]),
    [RSC_SYSINFO_ISA_COND_0_0]	= LDB(	Rsc_SysInfoISA_Cond_Attr[0][0]),
    [RSC_SYSINFO_ISA_COND_0_1]	= LDB(	Rsc_SysInfoISA_Cond_Attr[0][1]),
    [RSC_SYSINFO_ISA_COND_0_2]	= LDB(	Rsc_SysInfoISA_Cond_Attr[0][2]),
    [RSC_SYSINFO_ISA_COND_0_3]	= LDB(	Rsc_SysInfoISA_Cond_Attr[0][3]),
    [RSC_SYSINFO_ISA_COND_0_4]	= LDB(	Rsc_SysInfoISA_Cond_Attr[0][4]),
    [RSC_SYSINFO_ISA_COND_1_0]	= LDB(	Rsc_SysInfoISA_Cond_Attr[1][0]),
    [RSC_SYSINFO_ISA_COND_1_1]	= LDB(	Rsc_SysInfoISA_Cond_Attr[1][1]),
    [RSC_SYSINFO_ISA_COND_1_2]	= LDB(	Rsc_SysInfoISA_Cond_Attr[1][2]),
    [RSC_SYSINFO_ISA_COND_1_3]	= LDB(	Rsc_SysInfoISA_Cond_Attr[1][3]),
    [RSC_SYSINFO_ISA_COND_1_4]	= LDB(	Rsc_SysInfoISA_Cond_Attr[1][4]),
    [RSC_SYSINFO_FEATURES_COND0]= LDB(	Rsc_SysInfoFeatures_Cond_Attr[0]),
    [RSC_SYSINFO_FEATURES_COND1]= LDB(	Rsc_SysInfoFeatures_Cond_Attr[1]),
    [RSC_SYSINFO_FEATURES_COND2]= LDB(	Rsc_SysInfoFeatures_Cond_Attr[2]),
    [RSC_SYSINFO_FEATURES_COND3]= LDB(	Rsc_SysInfoFeatures_Cond_Attr[3]),
    [RSC_SYSINFO_FEATURES_COND4]= LDB(	Rsc_SysInfoFeatures_Cond_Attr[4]),
    [RSC_SYSINFO_TECH_COND0]	= LDB(	Rsc_SysInfoTech_Cond_Attr[0]),
    [RSC_SYSINFO_TECH_COND1]	= LDB(	Rsc_SysInfoTech_Cond_Attr[1]),
    [RSC_SYSINFO_PERFMON_COND0] = LDB(	Rsc_SysInfoPerfMon_Cond_Attr[0]),
    [RSC_SYSINFO_PERFMON_COND1] = LDB(	Rsc_SysInfoPerfMon_Cond_Attr[1]),
    [RSC_SYSINFO_PERFMON_COND2] = LDB(	Rsc_SysInfoPerfMon_Cond_Attr[2]),
    [RSC_SYSINFO_PERFMON_COND3] = LDB(	Rsc_SysInfoPerfMon_Cond_Attr[3]),
    [RSC_SYSINFO_PERFMON_COND4] = LDB(	Rsc_SysInfoPerfMon_Cond_Attr[4]),
[RSC_SYSINFO_PERFMON_HWP_CAP_COND0]=LDB(Rsc_SysInfoPerfMon_HWP_Cap_Attr[0]),
[RSC_SYSINFO_PERFMON_HWP_CAP_COND1]=LDB(Rsc_SysInfoPerfMon_HWP_Cap_Attr[1]),
    [RSC_SYSINFO_PWR_THERMAL_COND0]=LDB(Rsc_SysInfoPwrThermal_Cond_Attr[0]),
    [RSC_SYSINFO_PWR_THERMAL_COND1]=LDB(Rsc_SysInfoPwrThermal_Cond_Attr[1]),
    [RSC_SYSINFO_PWR_THERMAL_COND2]=LDB(Rsc_SysInfoPwrThermal_Cond_Attr[2]),
    [RSC_SYSINFO_PWR_THERMAL_COND3]=LDB(Rsc_SysInfoPwrThermal_Cond_Attr[3]),
    [RSC_SYSINFO_PWR_THERMAL_COND4]=LDB(Rsc_SysInfoPwrThermal_Cond_Attr[4]),
    [RSC_SYSINFO_PWR_THERMAL_COND5]=LDB(Rsc_SysInfoPwrThermal_Cond_Attr[5]),
    [RSC_SYSINFO_KERNEL]	= LDB(	Rsc_SysInfoKernel_Attr),
    [RSC_TOPOLOGY_COND0]	= LDB(	Rsc_Topology_Cond_Attr[0]),
    [RSC_TOPOLOGY_COND1]	= LDB(	Rsc_Topology_Cond_Attr[1]),
    [RSC_TOPOLOGY_COND2]	= LDB(	Rsc_Topology_Cond_Attr[2]),
    [RSC_TOPOLOGY_COND3]	= LDB(	Rsc_Topology_Cond_Attr[3]),
    [RSC_TOPOLOGY_COND4]	= LDB(	Rsc_Topology_Cond_Attr[4]),
    [RSC_MEMORY_CONTROLLER_COND0]=LDB(	Rsc_MemoryController_Cond_Attr[0]),
    [RSC_MEMORY_CONTROLLER_COND1]=LDB(	Rsc_MemoryController_Cond_Attr[1]),
    [RSC_CREATE_MENU_DISABLE]	= LDB(	Rsc_CreateMenu_Disable_Attr),
    [RSC_CREATE_MENU_FN_KEY]	= LDB(	Rsc_CreateMenu_FnKey_Attr),
    [RSC_CREATE_MENU_SHORTKEY]	= LDB(	Rsc_CreateMenu_ShortKey_Attr),
    [RSC_CREATE_MENU_CTRL_KEY]	= LDB(	Rsc_CreateMenu_CtrlKey_Attr),
    [RSC_CREATE_SETTINGS_COND0] = LDS(	Rsc_CreateSettings_Cond_Attr[0],
					RSC_CREATE_SETTINGS_COND_CODE,
					RSC_CREATE_SETTINGS_COND_CODE),
    [RSC_CREATE_SETTINGS_COND1] = LDS(	Rsc_CreateSettings_Cond_Attr[1],
					RSC_CREATE_SETTINGS_COND_CODE,
					RSC_CREATE_SETTINGS_COND_CODE),
    [RSC_CREATE_ADV_HELP_COND0] = LDN(	Rsc_CreateAdvHelp_Cond_Attr[0],
					RSC_CREATE_ADV_HELP_BLANK_CODE),
    [RSC_CREATE_ADV_HELP_COND1] = LDN(	Rsc_CreateAdvHelp_Cond_Attr[1],
					RSC_CREATE_ADV_HELP_BLANK_CODE),
    [RSC_CREATE_HOTPLUG_CPU_ENABLE]=LDS(Rsc_CreateHotPlugCPU_Enable_Attr,
					RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_EN,
					RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_FR),
   [RSC_CREATE_HOTPLUG_CPU_DISABLE]=LDS(Rsc_CreateHotPlugCPU_Disable_Attr,
					RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_EN,
					RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_FR),
    [RSC_CREATE_HOTPLUG_CPU_ONLINE]=LDS(Rsc_CreateHotPlugCPU_OnLine_Attr,
					RSC_CREATE_HOTPLUG_CPU_ONLINE_CODE_EN,
					RSC_CREATE_HOTPLUG_CPU_ONLINE_CODE_FR),
   [RSC_CREATE_HOTPLUG_CPU_OFFLINE]=LDS(Rsc_CreateHotPlugCPU_OffLine_Attr,
					RSC_CREATE_HOTPLUG_CPU_OFFLINE_CODE_EN,
					RSC_CREATE_HOTPLUG_CPU_OFFLINE_CODE_FR),
    [RSC_CREATE_RATIO_CLOCK_COND0]=LDB( Rsc_CreateRatioClock_Cond_Attr[0]),
    [RSC_CREATE_RATIO_CLOCK_COND1]=LDB( Rsc_CreateRatioClock_Cond_Attr[1]),
    [RSC_CREATE_RATIO_CLOCK_COND2]=LDB( Rsc_CreateRatioClock_Cond_Attr[2]),
    [RSC_CREATE_RATIO_CLOCK_COND3]=LDB( Rsc_CreateRatioClock_Cond_Attr[3]),
    [RSC_CREATE_RATIO_CLOCK_COND4]=LDB( Rsc_CreateRatioClock_Cond_Attr[4]),
    [RSC_CREATE_RATIO_CLOCK_COND5]=LDB( Rsc_CreateRatioClock_Cond_Attr[5]),
    [RSC_CREATE_RATIO_CLOCK_COND6]=LDB( Rsc_CreateRatioClock_Cond_Attr[6]),
    [RSC_CREATE_SELECT_CPU_COND0] = LDB(Rsc_CreateSelectCPU_Cond_Attr[0]),
    [RSC_CREATE_SELECT_CPU_COND1] = LDB(Rsc_CreateSelectCPU_Cond_Attr[1]),
    [RSC_CREATE_SELECT_CPU_COND2] = LDB(Rsc_CreateSelectCPU_Cond_Attr[2]),
#ifndef NO_FOOTER
    [RSC_HOT_EVENT_COND0]	= LDB(	Rsc_HotEvent_Cond_Attr[0]),
    [RSC_HOT_EVENT_COND1]	= LDB(	Rsc_HotEvent_Cond_Attr[1]),
    [RSC_HOT_EVENT_COND2]	= LDB(	Rsc_HotEvent_Cond_Attr[2]),
    [RSC_HOT_EVENT_COND3]	= LDB(	Rsc_HotEvent_Cond_Attr[3]),
    [RSC_HOT_EVENT_COND4]	= LDB(	Rsc_HotEvent_Cond_Attr[4]),
#endif
	[RSC_BOX_EVENT] 	= LDB(	Rsc_BoxEvent_Attr),
	[RSC_CREATE_RECORDER]	= LDB(	Rsc_CreateRecorder_Attr),
	[RSC_SMBIOS_ITEM]	= LDB(	Rsc_SMBIOS_Item_Attr),
    [RSC_CREATE_SELECT_FREQ_PKG] = LDB( Rsc_CreateSelectFreq_Pkg_Attr),
    [RSC_CREATE_SELECT_FREQ_COND0]=LDB( Rsc_CreateSelectFreq_Cond_Attr[0]),
    [RSC_CREATE_SELECT_FREQ_COND1]=LDB( Rsc_CreateSelectFreq_Cond_Attr[1]),
/* ASCII */
    [RSC_PROCESSOR_TITLE]	= LDT(	RSC_PROCESSOR_TITLE_CODE_EN,
					RSC_PROCESSOR_TITLE_CODE_FR),
	[RSC_PROCESSOR] 	= LDT(	RSC_PROCESSOR_CODE_EN,
					RSC_PROCESSOR_CODE_FR),
	[RSC_ARCHITECTURE]	= LDT(	RSC_ARCHITECTURE_CODE_EN,
					RSC_ARCHITECTURE_CODE_FR),
	[RSC_VENDOR_ID] 	= LDT(	RSC_VENDOR_ID_CODE_EN,
					RSC_VENDOR_ID_CODE_FR),
	[RSC_FIRMWARE]		= LDQ(	RSC_FIRMWARE_CODE),
	[RSC_MICROCODE] 	= LDT(	RSC_MICROCODE_CODE_EN,
					RSC_MICROCODE_CODE_FR),
	[RSC_SIGNATURE] 	= LDT(	RSC_SIGNATURE_CODE_EN,
					RSC_SIGNATURE_CODE_FR),
	[RSC_STEPPING]		= LDT(	RSC_STEPPING_CODE_EN,
					RSC_STEPPING_CODE_FR),
	[RSC_ONLINE_CPU]	= LDT(	RSC_ONLINE_CPU_CODE_EN,
					RSC_ONLINE_CPU_CODE_FR),
	[RSC_BASE_CLOCK]	= LDT(	RSC_BASE_CLOCK_CODE_EN,
					RSC_BASE_CLOCK_CODE_FR),
	[RSC_FREQUENCY] 	= LDT(	RSC_FREQUENCY_CODE_EN,
					RSC_FREQUENCY_CODE_FR),
	[RSC_RATIO]		= LDT(	RSC_RATIO_CODE_EN,
					RSC_RATIO_CODE_FR),
	[RSC_FACTORY]		= LDT(	RSC_FACTORY_CODE_EN,
					RSC_FACTORY_CODE_FR),
	[RSC_PERFORMANCE]	= LDT(	RSC_PERFORMANCE_CODE_EN,
					RSC_PERFORMANCE_CODE_FR),
	[RSC_TARGET]		= LDT(	RSC_TARGET_CODE_EN,
					RSC_TARGET_CODE_FR),
	[RSC_LEVEL]		= LDT(	RSC_LEVEL_CODE_EN,
					RSC_LEVEL_CODE_FR),
	[RSC_PROGRAMMABLE]	= LDT(	RSC_PROGRAMMABLE_CODE_EN,
					RSC_PROGRAMMABLE_CODE_FR),
	[RSC_CONFIGURATION]	= LDT(	RSC_CONFIGURATION_CODE_EN,
					RSC_CONFIGURATION_CODE_FR),
	[RSC_TURBO_ACTIVATION]	= LDT(	RSC_TURBO_ACTIVATION_CODE_EN,
					RSC_TURBO_ACTIVATION_CODE_FR),
	[RSC_NOMINAL]		= LDT(	RSC_NOMINAL_CODE_EN,
					RSC_NOMINAL_CODE_FR),
	[RSC_UNLOCK]		= LDT(	RSC_UNLOCK_CODE_EN,
					RSC_UNLOCK_CODE_FR),
	[RSC_LOCK]		= LDT(	RSC_LOCK_CODE_EN,
					RSC_LOCK_CODE_FR),
	[RSC_ENABLE]		= LDT(	RSC_ENABLE_CODE_EN,
					RSC_ENABLE_CODE_FR),
	[RSC_DISABLE]		= LDT(	RSC_DISABLE_CODE_EN,
					RSC_DISABLE_CODE_FR),
	[RSC_CAPABILITIES]	= LDT(	RSC_CAPABILITIES_CODE_EN,
					RSC_CAPABILITIES_CODE_FR),
	[RSC_LOWEST]		= LDT(	RSC_LOWEST_CODE_EN,
					RSC_LOWEST_CODE_FR),
	[RSC_EFFICIENT] 	= LDT(	RSC_EFFICIENT_CODE_EN,
					RSC_EFFICIENT_CODE_FR),
	[RSC_GUARANTEED]	= LDT(	RSC_GUARANTEED_CODE_EN,
					RSC_GUARANTEED_CODE_FR),
	[RSC_HIGHEST]		= LDT(	RSC_HIGHEST_CODE_EN,
					RSC_HIGHEST_CODE_FR),
	[RSC_RECORDER]		= LDT(	RSC_RECORDER_CODE_EN,
					RSC_RECORDER_CODE_FR),
	[RSC_STRESS]		= LDT(	RSC_STRESS_CODE_EN,
					RSC_STRESS_CODE_FR),
	[RSC_FREQ_UNIT_MHZ]	= LDQ(	RSC_FREQ_UNIT_MHZ_CODE),
	[RSC_PPIN]		= LDQ(	RSC_PPIN_CODE),
	[RSC_PSTATE]		= LDQ(	RSC_PSTATE_CODE),
	[RSC_UNCORE]		= LDQ(	RSC_UNCORE_CODE),
	[RSC_BOOST]		= LDQ(	RSC_BOOST_CODE),
	[RSC_TURBO]		= LDQ(	RSC_TURBO_CODE),
	[RSC_MAX]		= LDQ(	RSC_MAX_CODE),
	[RSC_MIN]		= LDQ(	RSC_MIN_CODE),
	[RSC_TGT]		= LDQ(	RSC_TGT_CODE),
	[RSC_HWP]		= LDQ(	RSC_HWP_CODE),
	[RSC_XFR]		= LDQ(	RSC_XFR_CODE),
	[RSC_CPB]		= LDQ(	RSC_CPB_CODE),
	[RSC_TDP]		= LDQ(	RSC_TDP_CODE),
	[RSC_ACT]		= LDQ(	RSC_ACT_CODE),
	[RSC_SCOPE_NONE]	= LDT(	RSC_SCOPE_NONE_CODE_EN,
					RSC_SCOPE_NONE_CODE_FR),
	[RSC_SCOPE_THREAD]	= LDT(	RSC_SCOPE_THREAD_CODE_EN,
					RSC_SCOPE_THREAD_CODE_FR),
	[RSC_SCOPE_CORE]	= LDT(	RSC_SCOPE_CORE_CODE_EN,
					RSC_SCOPE_CORE_CODE_FR),
	[RSC_SCOPE_PACKAGE]	= LDT(	RSC_SCOPE_PACKAGE_CODE_EN,
					RSC_SCOPE_PACKAGE_CODE_FR),
	[RSC_CPUID_TITLE]	= LDT(	RSC_CPUID_TITLE_EN,
					RSC_CPUID_TITLE_FR),
	[RSC_LARGEST_STD_FUNC]	= LDT(	RSC_LARGEST_STD_FUNC_CODE_EN,
					RSC_LARGEST_STD_FUNC_CODE_FR),
	[RSC_LARGEST_EXT_FUNC]	= LDT(	RSC_LARGEST_EXT_FUNC_CODE_EN,
					RSC_LARGEST_EXT_FUNC_CODE_FR),
	[RSC_SYS_REGS_TITLE]	= LDT(	RSC_SYS_REGS_TITLE_CODE_EN,
					RSC_SYS_REGS_TITLE_CODE_FR),
	[RSC_SYS_REGS_SPACE]	= LDQ(	RSC_SYS_REGS_SPACE_CODE),
	[RSC_SYS_REGS_NA]	= LDQ(	RSC_SYS_REGS_NA_CODE),
	[RSC_SYS_REGS_HDR_CPU]	= LDQ(	RSC_SYS_REGS_HDR_CPU_CODE),
	[RSC_SYS_REG_HDR_FLAGS] = LDQ(	RSC_SYS_REG_HDR_FLAGS_CODE),
	[RSC_SYS_REG_HDR_TF]	= LDQ(	RSC_SYS_REG_HDR_TF_CODE),
	[RSC_SYS_REG_HDR_IF]	= LDQ(	RSC_SYS_REG_HDR_IF_CODE),
	[RSC_SYS_REG_HDR_IOPL]	= LDQ(	RSC_SYS_REG_HDR_IOPL_CODE),
	[RSC_SYS_REG_HDR_NT]	= LDQ(	RSC_SYS_REG_HDR_NT_CODE),
	[RSC_SYS_REG_HDR_RF]	= LDQ(	RSC_SYS_REG_HDR_RF_CODE),
	[RSC_SYS_REG_HDR_VM]	= LDQ(	RSC_SYS_REG_HDR_VM_CODE),
	[RSC_SYS_REG_HDR_AC]	= LDQ(	RSC_SYS_REG_HDR_AC_CODE),
	[RSC_SYS_REG_HDR_VIF]	= LDQ(	RSC_SYS_REG_HDR_VIF_CODE),
	[RSC_SYS_REG_HDR_VIP]	= LDQ(	RSC_SYS_REG_HDR_VIP_CODE),
	[RSC_SYS_REG_HDR_ID]	= LDQ(	RSC_SYS_REG_HDR_ID_CODE),
	[RSC_SYS_REG_FLAGS_TF]	= LDT(	RSC_SYS_REG_FLAGS_TF_CODE_EN,
					RSC_SYS_REG_FLAGS_TF_CODE_FR),
	[RSC_SYS_REG_FLAGS_IF]	= LDT(	RSC_SYS_REG_FLAGS_IF_CODE_EN,
					RSC_SYS_REG_FLAGS_IF_CODE_FR),
	[RSC_SYS_REG_FLAGS_IOPL]= LDT(	RSC_SYS_REG_FLAGS_IOPL_CODE_EN,
					RSC_SYS_REG_FLAGS_IOPL_CODE_FR),
	[RSC_SYS_REG_FLAGS_NT]	= LDT(	RSC_SYS_REG_FLAGS_NT_CODE_EN,
					RSC_SYS_REG_FLAGS_NT_CODE_FR),
	[RSC_SYS_REG_FLAGS_RF]	= LDT(	RSC_SYS_REG_FLAGS_RF_CODE_EN,
					RSC_SYS_REG_FLAGS_RF_CODE_FR),
	[RSC_SYS_REG_FLAGS_VM]	= LDT(	RSC_SYS_REG_FLAGS_VM_CODE_EN,
					RSC_SYS_REG_FLAGS_VM_CODE_FR),
	[RSC_SYS_REG_FLAGS_AC]	= LDT(	RSC_SYS_REG_FLAGS_AC_CODE_EN,
					RSC_SYS_REG_FLAGS_AC_CODE_FR),
	[RSC_SYS_REG_FLAGS_VIF] = LDT(	RSC_SYS_REG_FLAGS_VIF_CODE_EN,
					RSC_SYS_REG_FLAGS_VIF_CODE_FR),
	[RSC_SYS_REG_FLAGS_VIP] = LDT(	RSC_SYS_REG_FLAGS_VIP_CODE_EN,
					RSC_SYS_REG_FLAGS_VIP_CODE_FR),
	[RSC_SYS_REG_FLAGS_ID]	= LDT(	RSC_SYS_REG_FLAGS_ID_CODE_EN,
					RSC_SYS_REG_FLAGS_ID_CODE_FR),
	[RSC_SYS_REG_HDR_CR0]	= LDQ(	RSC_SYS_REG_HDR_CR0_CODE),
	[RSC_SYS_REG_HDR_CR0_PE] = LDQ( RSC_SYS_REG_HDR_CR0_PE_CODE),
	[RSC_SYS_REG_HDR_CR0_MP] = LDQ( RSC_SYS_REG_HDR_CR0_MP_CODE),
	[RSC_SYS_REG_HDR_CR0_EM] = LDQ( RSC_SYS_REG_HDR_CR0_EM_CODE),
	[RSC_SYS_REG_HDR_CR0_TS] = LDQ( RSC_SYS_REG_HDR_CR0_TS_CODE),
	[RSC_SYS_REG_HDR_CR0_ET] = LDQ( RSC_SYS_REG_HDR_CR0_ET_CODE),
	[RSC_SYS_REG_HDR_CR0_NE] = LDQ( RSC_SYS_REG_HDR_CR0_NE_CODE),
	[RSC_SYS_REG_HDR_CR0_WP] = LDQ( RSC_SYS_REG_HDR_CR0_WP_CODE),
	[RSC_SYS_REG_HDR_CR0_AM] = LDQ( RSC_SYS_REG_HDR_CR0_AM_CODE),
	[RSC_SYS_REG_HDR_CR0_NW] = LDQ( RSC_SYS_REG_HDR_CR0_NW_CODE),
	[RSC_SYS_REG_HDR_CR0_CD] = LDQ( RSC_SYS_REG_HDR_CR0_CD_CODE),
	[RSC_SYS_REG_HDR_CR0_PG] = LDQ( RSC_SYS_REG_HDR_CR0_PG_CODE),
	[RSC_SYS_REGS_CR0]	= LDT(	RSC_SYS_REGS_CR0_CODE_EN,
					RSC_SYS_REGS_CR0_CODE_FR),
	[RSC_SYS_REG_CR0_PE]	= LDT(	RSC_SYS_REG_CR0_PE_CODE_EN,
					RSC_SYS_REG_CR0_PE_CODE_FR),
	[RSC_SYS_REG_CR0_MP]	= LDT(	RSC_SYS_REG_CR0_MP_CODE_EN,
					RSC_SYS_REG_CR0_MP_CODE_FR),
	[RSC_SYS_REG_CR0_EM]	= LDT(	RSC_SYS_REG_CR0_EM_CODE_EN,
					RSC_SYS_REG_CR0_EM_CODE_FR),
	[RSC_SYS_REG_CR0_TS]	= LDT(	RSC_SYS_REG_CR0_TS_CODE_EN,
					RSC_SYS_REG_CR0_TS_CODE_FR),
	[RSC_SYS_REG_CR0_ET]	= LDT(	RSC_SYS_REG_CR0_ET_CODE_EN,
					RSC_SYS_REG_CR0_ET_CODE_FR),
	[RSC_SYS_REG_CR0_NE]	= LDT(	RSC_SYS_REG_CR0_NE_CODE_EN,
					RSC_SYS_REG_CR0_NE_CODE_FR),
	[RSC_SYS_REG_CR0_WP]	= LDT(	RSC_SYS_REG_CR0_WP_CODE_EN,
					RSC_SYS_REG_CR0_WP_CODE_FR),
	[RSC_SYS_REG_CR0_AM]	= LDT(	RSC_SYS_REG_CR0_AM_CODE_EN,
					RSC_SYS_REG_CR0_AM_CODE_FR),
	[RSC_SYS_REG_CR0_NW]	= LDT(	RSC_SYS_REG_CR0_NW_CODE_EN,
					RSC_SYS_REG_CR0_NW_CODE_FR),
	[RSC_SYS_REG_CR0_CD]	= LDT(	RSC_SYS_REG_CR0_CD_CODE_EN,
					RSC_SYS_REG_CR0_CD_CODE_FR),
	[RSC_SYS_REG_CR0_PG]	= LDT(	RSC_SYS_REG_CR0_PG_CODE_EN,
					RSC_SYS_REG_CR0_PG_CODE_FR),
	[RSC_SYS_REG_HDR_CR3]	= LDQ(	RSC_SYS_REG_HDR_CR3_CODE),
	[RSC_SYS_REG_HDR_CR3_PWT] = LDQ(RSC_SYS_REG_HDR_CR3_PWT_CODE),
	[RSC_SYS_REG_HDR_CR3_PCD] = LDQ(RSC_SYS_REG_HDR_CR3_PCD_CODE),
	[RSC_SYS_REGS_CR3]	= LDT(	RSC_SYS_REGS_CR3_CODE_EN,
					RSC_SYS_REGS_CR3_CODE_FR),
	[RSC_SYS_REG_CR3_PWT]	= LDT(	RSC_SYS_REG_CR3_PWT_CODE_EN,
					RSC_SYS_REG_CR3_PWT_CODE_FR),
	[RSC_SYS_REG_CR3_PCD]	= LDT(	RSC_SYS_REG_CR3_PCD_CODE_EN,
					RSC_SYS_REG_CR3_PCD_CODE_FR),
	[RSC_SYS_REG_HDR_CR4]	= LDQ(	RSC_SYS_REG_HDR_CR4_CODE),
	[RSC_SYS_REG_HDR_CR4_VME] = LDQ(RSC_SYS_REG_HDR_CR4_VME_CODE),
	[RSC_SYS_REG_HDR_CR4_PVI] = LDQ(RSC_SYS_REG_HDR_CR4_PVI_CODE),
	[RSC_SYS_REG_HDR_CR4_TSD] = LDQ(RSC_SYS_REG_HDR_CR4_TSD_CODE),
	[RSC_SYS_REG_HDR_CR4_DE] = LDQ( RSC_SYS_REG_HDR_CR4_DE_CODE),
	[RSC_SYS_REG_HDR_CR4_PSE] = LDQ(RSC_SYS_REG_HDR_CR4_PSE_CODE),
	[RSC_SYS_REG_HDR_CR4_PAE] = LDQ(RSC_SYS_REG_HDR_CR4_PAE_CODE),
	[RSC_SYS_REG_HDR_CR4_MCE] = LDQ(RSC_SYS_REG_HDR_CR4_MCE_CODE),
	[RSC_SYS_REG_HDR_CR4_PGE] = LDQ(RSC_SYS_REG_HDR_CR4_PGE_CODE),
	[RSC_SYS_REG_HDR_CR4_PCE] = LDQ(RSC_SYS_REG_HDR_CR4_PCE_CODE),
	[RSC_SYS_REG_HDR_CR4_FX] = LDQ( RSC_SYS_REG_HDR_CR4_FX_CODE),
	[RSC_SYS_REG_HDR_CR4_XMM] = LDQ(RSC_SYS_REG_HDR_CR4_XMM_CODE),
	[RSC_SYS_REG_HDR_CR4_UMIP]= LDQ(RSC_SYS_REG_HDR_CR4_UMIP_CODE),
	[RSC_SYS_REG_HDR_CR4_5LP] = LDQ(RSC_SYS_REG_HDR_CR4_5LP_CODE),
	[RSC_SYS_REG_HDR_CR4_VMX] = LDQ(RSC_SYS_REG_HDR_CR4_VMX_CODE),
	[RSC_SYS_REG_HDR_CR4_SMX] = LDQ(RSC_SYS_REG_HDR_CR4_SMX_CODE),
	[RSC_SYS_REG_HDR_CR4_FS] = LDQ( RSC_SYS_REG_HDR_CR4_FS_CODE),
	[RSC_SYS_REG_HDR_CR4_PCID]= LDQ(RSC_SYS_REG_HDR_CR4_PCID_CODE),
	[RSC_SYS_REG_HDR_CR4_SAV] = LDQ(RSC_SYS_REG_HDR_CR4_SAV_CODE),
	[RSC_SYS_REG_HDR_CR4_KL] = LDQ( RSC_SYS_REG_HDR_CR4_KL_CODE),
	[RSC_SYS_REG_HDR_CR4_SME] = LDQ(RSC_SYS_REG_HDR_CR4_SME_CODE),
	[RSC_SYS_REG_HDR_CR4_SMA] = LDQ(RSC_SYS_REG_HDR_CR4_SMA_CODE),
	[RSC_SYS_REG_HDR_CR4_PKE] = LDQ(RSC_SYS_REG_HDR_CR4_PKE_CODE),
	[RSC_SYS_REG_HDR_CR4_CET] = LDQ(RSC_SYS_REG_HDR_CR4_CET_CODE),
	[RSC_SYS_REG_HDR_CR4_PKS] = LDQ(RSC_SYS_REG_HDR_CR4_PKS_CODE),
	[RSC_SYS_REGS_CR4]	= LDT(	RSC_SYS_REGS_CR4_CODE_EN,
					RSC_SYS_REGS_CR4_CODE_FR),
	[RSC_SYS_REG_CR4_VME]	= LDT(	RSC_SYS_REG_CR4_VME_CODE_EN,
					RSC_SYS_REG_CR4_VME_CODE_FR),
	[RSC_SYS_REG_CR4_PVI]	= LDT(	RSC_SYS_REG_CR4_PVI_CODE_EN,
					RSC_SYS_REG_CR4_PVI_CODE_FR),
	[RSC_SYS_REG_CR4_TSD]	= LDT(	RSC_SYS_REG_CR4_TSD_CODE_EN,
					RSC_SYS_REG_CR4_TSD_CODE_FR),
	[RSC_SYS_REG_CR4_DE]	= LDT(	RSC_SYS_REG_CR4_DE_CODE_EN,
					RSC_SYS_REG_CR4_DE_CODE_FR),
	[RSC_SYS_REG_CR4_PSE]	= LDT(	RSC_SYS_REG_CR4_PSE_CODE_EN,
					RSC_SYS_REG_CR4_PSE_CODE_FR),
	[RSC_SYS_REG_CR4_PAE]	= LDT(	RSC_SYS_REG_CR4_PAE_CODE_EN,
					RSC_SYS_REG_CR4_PAE_CODE_FR),
	[RSC_SYS_REG_CR4_MCE]	= LDT(	RSC_SYS_REG_CR4_MCE_CODE_EN,
					RSC_SYS_REG_CR4_MCE_CODE_FR),
	[RSC_SYS_REG_CR4_PGE]	= LDT(	RSC_SYS_REG_CR4_PGE_CODE_EN,
					RSC_SYS_REG_CR4_PGE_CODE_FR),
	[RSC_SYS_REG_CR4_PCE]	= LDT(	RSC_SYS_REG_CR4_PCE_CODE_EN,
					RSC_SYS_REG_CR4_PCE_CODE_FR),
	[RSC_SYS_REG_CR4_FX]	= LDT(	RSC_SYS_REG_CR4_FX_CODE_EN,
					RSC_SYS_REG_CR4_FX_CODE_FR),
	[RSC_SYS_REG_CR4_XMM]	= LDT(	RSC_SYS_REG_CR4_XMM_CODE_EN,
					RSC_SYS_REG_CR4_XMM_CODE_FR),
	[RSC_SYS_REG_CR4_UMIP]	= LDT(	RSC_SYS_REG_CR4_UMIP_CODE_EN,
					RSC_SYS_REG_CR4_UMIP_CODE_FR),
	[RSC_SYS_REG_CR4_5LP]	= LDT(	RSC_SYS_REG_CR4_5LP_CODE_EN,
					RSC_SYS_REG_CR4_5LP_CODE_FR),
	[RSC_SYS_REG_CR4_VMX]	= LDT(	RSC_SYS_REG_CR4_VMX_CODE_EN,
					RSC_SYS_REG_CR4_VMX_CODE_FR),
	[RSC_SYS_REG_CR4_SMX]	= LDT(	RSC_SYS_REG_CR4_SMX_CODE_EN,
					RSC_SYS_REG_CR4_SMX_CODE_FR),
	[RSC_SYS_REG_CR4_FS]	= LDT(	RSC_SYS_REG_CR4_FS_CODE_EN,
					RSC_SYS_REG_CR4_FS_CODE_FR),
	[RSC_SYS_REG_CR4_PCID]	= LDT(	RSC_SYS_REG_CR4_PCID_CODE_EN,
					RSC_SYS_REG_CR4_PCID_CODE_FR),
	[RSC_SYS_REG_CR4_SAV]	= LDT(	RSC_SYS_REG_CR4_SAV_CODE_EN,
					RSC_SYS_REG_CR4_SAV_CODE_FR),
	[RSC_SYS_REG_CR4_KL]	= LDT(	RSC_SYS_REG_CR4_KL_CODE_EN,
					RSC_SYS_REG_CR4_KL_CODE_FR),
	[RSC_SYS_REG_CR4_SME]	= LDT(	RSC_SYS_REG_CR4_SME_CODE_EN,
					RSC_SYS_REG_CR4_SME_CODE_FR),
	[RSC_SYS_REG_CR4_SMA]	= LDT(	RSC_SYS_REG_CR4_SMA_CODE_EN,
					RSC_SYS_REG_CR4_SMA_CODE_FR),
	[RSC_SYS_REG_CR4_PKE]	= LDT(	RSC_SYS_REG_CR4_PKE_CODE_EN,
					RSC_SYS_REG_CR4_PKE_CODE_FR),
	[RSC_SYS_REG_CR4_CET]	= LDT(	RSC_SYS_REG_CR4_CET_CODE_EN,
					RSC_SYS_REG_CR4_CET_CODE_FR),
	[RSC_SYS_REG_CR4_PKS]	= LDT(	RSC_SYS_REG_CR4_PKS_CODE_EN,
					RSC_SYS_REG_CR4_PKS_CODE_FR),
	[RSC_SYS_REG_HDR_CR8]	= LDQ(	RSC_SYS_REG_HDR_CR8_CODE),
	[RSC_SYS_REG_HDR_CR8_TPL] = LDQ(RSC_SYS_REG_HDR_CR8_TPL_CODE),
	[RSC_SYS_REGS_CR8]	= LDT(	RSC_SYS_REGS_CR8_CODE_EN,
					RSC_SYS_REGS_CR8_CODE_FR),
	[RSC_SYS_REG_CR8_TPL]	= LDT(	RSC_SYS_REG_CR8_TPL_CODE_EN,
					RSC_SYS_REG_CR8_TPL_CODE_FR),
	[RSC_SYS_REG_HDR_EFCR]	= LDQ(	RSC_SYS_REG_HDR_EFCR_CODE),
	[RSC_SYS_REG_HDR_EFCR_LCK]= LDQ(RSC_SYS_REG_HDR_EFCR_LCK_CODE),
	[RSC_SYS_REG_HDR_EFCR_VMX]= LDQ(RSC_SYS_REG_HDR_EFCR_VMX_CODE),
	[RSC_SYS_REG_HDR_EFCR_SGX]= LDQ(RSC_SYS_REG_HDR_EFCR_SGX_CODE),
	[RSC_SYS_REG_HDR_EFCR_LSE]= LDQ(RSC_SYS_REG_HDR_EFCR_LSE_CODE),
	[RSC_SYS_REG_HDR_EFCR_GSE]= LDQ(RSC_SYS_REG_HDR_EFCR_GSE_CODE),
	[RSC_SYS_REG_HDR_EFCR_LSGX]=LDQ(RSC_SYS_REG_HDR_EFCR_LSGX_CODE),
	[RSC_SYS_REG_HDR_EFCR_GSGX]=LDQ(RSC_SYS_REG_HDR_EFCR_GSGX_CODE),
	[RSC_SYS_REG_HDR_EFCR_LMC]= LDQ(RSC_SYS_REG_HDR_EFCR_LMC_CODE),
	[RSC_SYS_REGS_EFCR]	= LDT(	RSC_SYS_REGS_EFCR_CODE_EN,
					RSC_SYS_REGS_EFCR_CODE_FR),
	[RSC_SYS_REG_EFCR_LCK]	= LDT(	RSC_SYS_REG_EFCR_LCK_CODE_EN,
					RSC_SYS_REG_EFCR_LCK_CODE_FR),
	[RSC_SYS_REG_EFCR_VMX]	= LDT(	RSC_SYS_REG_EFCR_VMX_CODE_EN,
					RSC_SYS_REG_EFCR_VMX_CODE_FR),
	[RSC_SYS_REG_EFCR_SGX]	= LDT(	RSC_SYS_REG_EFCR_SGX_CODE_EN,
					RSC_SYS_REG_EFCR_SGX_CODE_FR),
	[RSC_SYS_REG_EFCR_LSE]	= LDT(	RSC_SYS_REG_EFCR_LSE_CODE_EN,
					RSC_SYS_REG_EFCR_LSE_CODE_FR),
	[RSC_SYS_REG_EFCR_GSE]	= LDT(	RSC_SYS_REG_EFCR_GSE_CODE_EN,
					RSC_SYS_REG_EFCR_GSE_CODE_FR),
	[RSC_SYS_REG_EFCR_LSGX] = LDT(	RSC_SYS_REG_EFCR_LSGX_CODE_EN,
					RSC_SYS_REG_EFCR_LSGX_CODE_FR),
	[RSC_SYS_REG_EFCR_GSGX] = LDT(	RSC_SYS_REG_EFCR_GSGX_CODE_EN,
					RSC_SYS_REG_EFCR_GSGX_CODE_FR),
	[RSC_SYS_REG_EFCR_LMC]	= LDT(	RSC_SYS_REG_EFCR_LMC_CODE_EN,
					RSC_SYS_REG_EFCR_LMC_CODE_FR),
	[RSC_SYS_REG_HDR_EFER]	= LDQ(	RSC_SYS_REG_HDR_EFER_CODE),
	[RSC_SYS_REG_HDR_EFER_SCE]= LDQ(RSC_SYS_REG_HDR_EFER_SCE_CODE),
	[RSC_SYS_REG_HDR_EFER_LME]= LDQ(RSC_SYS_REG_HDR_EFER_LME_CODE),
	[RSC_SYS_REG_HDR_EFER_LMA]= LDQ(RSC_SYS_REG_HDR_EFER_LMA_CODE),
	[RSC_SYS_REG_HDR_EFER_NXE]= LDQ(RSC_SYS_REG_HDR_EFER_NXE_CODE),
	[RSC_SYS_REG_HDR_EFER_SVM]= LDQ(RSC_SYS_REG_HDR_EFER_SVM_CODE),
	[RSC_SYS_REGS_EFER]	= LDT(	RSC_SYS_REGS_EFER_CODE_EN,
					RSC_SYS_REGS_EFER_CODE_FR),
	[RSC_SYS_REG_EFER_SCE]	= LDT(	RSC_SYS_REG_EFER_SCE_CODE_EN,
					RSC_SYS_REG_EFER_SCE_CODE_FR),
	[RSC_SYS_REG_EFER_LME]	= LDT(	RSC_SYS_REG_EFER_LME_CODE_EN,
					RSC_SYS_REG_EFER_LME_CODE_FR),
	[RSC_SYS_REG_EFER_LMA]	= LDT(	RSC_SYS_REG_EFER_LMA_CODE_EN,
					RSC_SYS_REG_EFER_LMA_CODE_FR),
	[RSC_SYS_REG_EFER_NXE]	= LDT(	RSC_SYS_REG_EFER_NXE_CODE_EN,
					RSC_SYS_REG_EFER_NXE_CODE_FR),
	[RSC_SYS_REG_EFER_SVM]	= LDT(	RSC_SYS_REG_EFER_SVM_CODE_EN,
					RSC_SYS_REG_EFER_SVM_CODE_FR),
	[RSC_ISA_TITLE] 	= LDT(	RSC_ISA_TITLE_CODE_EN,
					RSC_ISA_TITLE_CODE_FR),
	[RSC_ISA_3DNOW] 	= LDQ(	RSC_ISA_3DNOW_CODE),
	[RSC_ISA_3DNOW_COMM]	= LDT(	RSC_ISA_3DNOW_COMM_CODE_EN,
					RSC_ISA_3DNOW_COMM_CODE_FR),
	[RSC_ISA_ADX]		= LDQ(	RSC_ISA_ADX_CODE),
	[RSC_ISA_ADX_COMM]	= LDT(	RSC_ISA_ADX_COMM_CODE_EN,
					RSC_ISA_ADX_COMM_CODE_FR),
	[RSC_ISA_AES]		= LDQ(	RSC_ISA_AES_CODE),
	[RSC_ISA_AES_COMM]	= LDT(	RSC_ISA_AES_COMM_CODE_EN,
					RSC_ISA_AES_COMM_CODE_FR),
	[RSC_ISA_AVX]		= LDQ(	RSC_ISA_AVX_CODE),
	[RSC_ISA_AVX_COMM]	= LDT(	RSC_ISA_AVX_COMM_CODE_EN,
					RSC_ISA_AVX_COMM_CODE_FR),
	[RSC_ISA_AVX512_F]	= LDQ(	RSC_ISA_AVX512_F_CODE),
	[RSC_ISA_AVX512_DQ]	= LDQ(	RSC_ISA_AVX512_DQ_CODE),
	[RSC_ISA_AVX512_IFMA]	= LDQ(	RSC_ISA_AVX512_IFMA_CODE),
	[RSC_ISA_AVX512_PF]	= LDQ(	RSC_ISA_AVX512_PF_CODE),
	[RSC_ISA_AVX512_ER]	= LDQ(	RSC_ISA_AVX512_ER_CODE),
	[RSC_ISA_AVX512_CD]	= LDQ(	RSC_ISA_AVX512_CD_CODE),
	[RSC_ISA_AVX512_BW]	= LDQ(	RSC_ISA_AVX512_BW_CODE),
	[RSC_ISA_AVX512_VL]	= LDQ(	RSC_ISA_AVX512_VL_CODE),
	[RSC_ISA_AVX512_VBMI]	= LDQ(	RSC_ISA_AVX512_VBMI_CODE),
	[RSC_ISA_AVX512_VBMI2]	= LDQ(	RSC_ISA_AVX512_VBMI2_CODE),
	[RSC_ISA_AVX512_VNMI]	= LDQ(	RSC_ISA_AVX512_VNMI_CODE),
	[RSC_ISA_AVX512_ALG]	= LDQ(	RSC_ISA_AVX512_ALG_CODE),
	[RSC_ISA_AVX512_VPOP]	= LDQ(	RSC_ISA_AVX512_VPOP_CODE),
	[RSC_ISA_AVX512_VNNIW]	= LDQ(	RSC_ISA_AVX512_VNNIW_CODE),
	[RSC_ISA_AVX512_FMAPS]	= LDQ(	RSC_ISA_AVX512_FMAPS_CODE),
	[RSC_ISA_AVX512_VP2I]	= LDQ(	RSC_ISA_AVX512_VP2I_CODE),
	[RSC_ISA_AVX512_BF16]	= LDQ(	RSC_ISA_AVX512_BF16_CODE),
	[RSC_ISA_BMI]		= LDQ(	RSC_ISA_BMI_CODE),
	[RSC_ISA_BMI_COMM]	= LDT(	RSC_ISA_BMI_COMM_CODE_EN,
					RSC_ISA_BMI_COMM_CODE_FR),
	[RSC_ISA_CLWB]		= LDQ(	RSC_ISA_CLWB_CODE),
	[RSC_ISA_CLWB_COMM]	= LDT(	RSC_ISA_CLWB_COMM_CODE_EN,
					RSC_ISA_CLWB_COMM_CODE_FR),
	[RSC_ISA_CLFLUSH]	= LDQ(	RSC_ISA_CLFLUSH_CODE),
	[RSC_ISA_CLFLUSH_COMM]	= LDT(	RSC_ISA_CLFLUSH_COMM_CODE_EN,
					RSC_ISA_CLFLUSH_COMM_CODE_FR),
	[RSC_ISA_AC_FLAG]	= LDQ(	RSC_ISA_AC_FLAG_CODE),
	[RSC_ISA_AC_FLAG_COMM]	= LDT(	RSC_ISA_AC_FLAG_COMM_CODE_EN,
					RSC_ISA_AC_FLAG_COMM_CODE_FR),
	[RSC_ISA_CMOV]		= LDQ(	RSC_ISA_CMOV_CODE),
	[RSC_ISA_CMOV_COMM]	= LDT(	RSC_ISA_CMOV_COMM_CODE_EN,
					RSC_ISA_CMOV_COMM_CODE_FR),
	[RSC_ISA_XCHG8B]	= LDQ(	RSC_ISA_XCHG8B_CODE),
	[RSC_ISA_XCHG8B_COMM]	= LDT(	RSC_ISA_XCHG8B_COMM_CODE_EN,
					RSC_ISA_XCHG8B_COMM_CODE_FR),
	[RSC_ISA_XCHG16B]	= LDQ(	RSC_ISA_XCHG16B_CODE),
	[RSC_ISA_XCHG16B_COMM]	= LDT(	RSC_ISA_XCHG16B_COMM_CODE_EN,
					RSC_ISA_XCHG16B_COMM_CODE_FR),
	[RSC_ISA_F16C]		= LDQ(	RSC_ISA_F16C_CODE),
	[RSC_ISA_F16C_COMM]	= LDT(	RSC_ISA_F16C_COMM_CODE_EN,
					RSC_ISA_F16C_COMM_CODE_FR),
	[RSC_ISA_FPU]		= LDQ(	RSC_ISA_FPU_CODE),
	[RSC_ISA_FPU_COMM]	= LDT(	RSC_ISA_FPU_COMM_CODE_EN,
					RSC_ISA_FPU_COMM_CODE_FR),
	[RSC_ISA_FXSR]		= LDQ(	RSC_ISA_FXSR_CODE),
	[RSC_ISA_FXSR_COMM]	= LDT(	RSC_ISA_FXSR_COMM_CODE_EN,
					RSC_ISA_FXSR_COMM_CODE_FR),
	[RSC_ISA_LSHF]		= LDQ(	RSC_ISA_LSHF_CODE),
	[RSC_ISA_LSHF_COMM]	= LDT(	RSC_ISA_LSHF_COMM_CODE_EN,
					RSC_ISA_LSHF_COMM_CODE_FR),
	[RSC_ISA_MMX]		= LDQ(	RSC_ISA_MMX_CODE),
	[RSC_ISA_MMX_COMM]	= LDT(	RSC_ISA_MMX_COMM_CODE_EN,
					RSC_ISA_MMX_COMM_CODE_FR),
	[RSC_ISA_MWAITX]	= LDQ(	RSC_ISA_MWAITX_CODE),
	[RSC_ISA_MWAITX_COMM]	= LDT(	RSC_ISA_MWAITX_COMM_CODE_EN,
					RSC_ISA_MWAITX_COMM_CODE_FR),
	[RSC_ISA_MOVBE] 	= LDQ(	RSC_ISA_MOVBE_CODE),
	[RSC_ISA_MOVBE_COMM]	= LDT(	RSC_ISA_MOVBE_COMM_CODE_EN,
					RSC_ISA_MOVBE_COMM_CODE_FR),
	[RSC_ISA_PCLMULDQ]	= LDQ(	RSC_ISA_PCLMULDQ_CODE),
	[RSC_ISA_PCLMULDQ_COMM] = LDT(	RSC_ISA_PCLMULDQ_COMM_CODE_EN,
					RSC_ISA_PCLMULDQ_COMM_CODE_FR),
	[RSC_ISA_POPCNT]	= LDQ(	RSC_ISA_POPCNT_CODE),
	[RSC_ISA_POPCNT_COMM]	= LDT(	RSC_ISA_POPCNT_COMM_CODE_EN,
					RSC_ISA_POPCNT_COMM_CODE_FR),
	[RSC_ISA_RDRAND]	= LDQ(	RSC_ISA_RDRAND_CODE),
	[RSC_ISA_RDRAND_COMM]	= LDT(	RSC_ISA_RDRAND_COMM_CODE_EN,
					RSC_ISA_RDRAND_COMM_CODE_FR),
	[RSC_ISA_RDSEED]	= LDQ(	RSC_ISA_RDSEED_CODE),
	[RSC_ISA_RDSEED_COMM]	= LDT(	RSC_ISA_RDSEED_COMM_CODE_EN,
					RSC_ISA_RDSEED_COMM_CODE_FR),
	[RSC_ISA_RDTSCP]	= LDQ(	RSC_ISA_RDTSCP_CODE),
	[RSC_ISA_RDTSCP_COMM]	= LDT(	RSC_ISA_RDTSCP_COMM_CODE_EN,
					RSC_ISA_RDTSCP_COMM_CODE_FR),
	[RSC_ISA_SEP]		= LDQ(	RSC_ISA_SEP_CODE),
	[RSC_ISA_SEP_COMM]	= LDT(	RSC_ISA_SEP_COMM_CODE_EN,
					RSC_ISA_SEP_COMM_CODE_FR),
	[RSC_ISA_SHA]		= LDQ(	RSC_ISA_SHA_CODE),
	[RSC_ISA_SHA_COMM]	= LDT(	RSC_ISA_SHA_COMM_CODE_EN,
					RSC_ISA_SHA_COMM_CODE_FR),
	[RSC_ISA_SSE]		= LDQ(	RSC_ISA_SSE_CODE),
	[RSC_ISA_SSE_COMM]	= LDT(	RSC_ISA_SSE_COMM_CODE_EN,
					RSC_ISA_SSE_COMM_CODE_FR),
	[RSC_ISA_SSE2]		= LDQ(	RSC_ISA_SSE2_CODE),
	[RSC_ISA_SSE2_COMM]	= LDT(	RSC_ISA_SSE2_COMM_CODE_EN,
					RSC_ISA_SSE2_COMM_CODE_FR),
	[RSC_ISA_SSE3]		= LDQ(	RSC_ISA_SSE3_CODE),
	[RSC_ISA_SSE3_COMM]	= LDT(	RSC_ISA_SSE3_COMM_CODE_EN,
					RSC_ISA_SSE3_COMM_CODE_FR),
	[RSC_ISA_SSSE3] 	= LDQ(	RSC_ISA_SSSE3_CODE),
	[RSC_ISA_SSSE3_COMM]	= LDT(	RSC_ISA_SSSE3_COMM_CODE_EN,
					RSC_ISA_SSSE3_COMM_CODE_FR),
	[RSC_ISA_SSE4_1]	= LDQ(	RSC_ISA_SSE4_1_CODE),
	[RSC_ISA_SSE4_1_COMM]	= LDT(	RSC_ISA_SSE4_1_COMM_CODE_EN,
					RSC_ISA_SSE4_1_COMM_CODE_FR),
	[RSC_ISA_SSE4_2]	= LDQ(	RSC_ISA_SSE4_2_CODE),
	[RSC_ISA_SSE4_2_COMM]	= LDT(	RSC_ISA_SSE4_2_COMM_CODE_EN,
					RSC_ISA_SSE4_2_COMM_CODE_FR),
	[RSC_ISA_SERIALIZE]	= LDQ(	RSC_ISA_SERIALIZE_CODE),
	[RSC_ISA_SERIALIZE_COMM]= LDT(	RSC_ISA_SERIALIZE_COMM_CODE_EN,
					RSC_ISA_SERIALIZE_COMM_CODE_FR),
	[RSC_ISA_SYSCALL]	= LDQ(	RSC_ISA_SYSCALL_CODE),
	[RSC_ISA_SYSCALL_COMM]	= LDT(	RSC_ISA_SYSCALL_COMM_CODE_EN,
					RSC_ISA_SYSCALL_COMM_CODE_FR),
	[RSC_ISA_RDPID_FMT1]	= LDQ(	RSC_ISA_RDPID_FMT1_CODE),
	[RSC_ISA_RDPID_FMT2]	= LDQ(	RSC_ISA_RDPID_FMT2_CODE),
	[RSC_ISA_RDPID_COMM]	= LDT(	RSC_ISA_RDPID_COMM_CODE_EN,
					RSC_ISA_RDPID_COMM_CODE_FR),
	[RSC_ISA_UMIP]		= LDQ(	RSC_ISA_UMIP_CODE),
	[RSC_ISA_UMIP_COMM]	= LDT(	RSC_ISA_UMIP_COMM_CODE_EN,
					RSC_ISA_UMIP_COMM_CODE_FR),
	[RSC_ISA_SGX]		= LDQ(	RSC_ISA_SGX_CODE),
	[RSC_ISA_SGX_COMM]	= LDT(	RSC_ISA_SGX_COMM_CODE_EN,
					RSC_ISA_SGX_COMM_CODE_FR),
	[RSC_FEATURES_TITLE]	= LDT(	RSC_FEATURES_TITLE_CODE_EN,
					RSC_FEATURES_TITLE_CODE_FR),
	[RSC_NOT_AVAILABLE]	= LDT(	RSC_NOT_AVAILABLE_CODE_EN,
					RSC_NOT_AVAILABLE_CODE_FR),
	[RSC_AUTOMATIC] 	= LDT(	RSC_AUTOMATIC_CODE_EN,
					RSC_AUTOMATIC_CODE_FR),
	[RSC_MISSING]		= LDT(	RSC_MISSING_CODE_EN,
					RSC_MISSING_CODE_FR),
	[RSC_PRESENT]		= LDT(	RSC_PRESENT_CODE_EN,
					RSC_PRESENT_CODE_FR),
	[RSC_VARIANT]		= LDT(	RSC_VARIANT_CODE_EN,
					RSC_VARIANT_CODE_FR),
	[RSC_INVARIANT] 	= LDT(	RSC_INVARIANT_CODE_EN,
					RSC_INVARIANT_CODE_FR),
	[RSC_XAPIC]		= LDQ(	RSC_XAPIC_CODE),
	[RSC_X2APIC]		= LDQ(	RSC_X2APIC_CODE),
	[RSC_FEATURES_1GB_PAGES]= LDT(	RSC_FEATURES_1GB_PAGES_CODE_EN,
					RSC_FEATURES_1GB_PAGES_CODE_FR),
	[RSC_FEATURES_100MHZ]	= LDT(	RSC_FEATURES_100MHZ_CODE_EN,
					RSC_FEATURES_100MHZ_CODE_FR),
	[RSC_FEATURES_ACPI]	= LDT(	RSC_FEATURES_ACPI_CODE_EN,
					RSC_FEATURES_ACPI_CODE_FR),
	[RSC_FEATURES_APIC]	= LDT(	RSC_FEATURES_APIC_CODE_EN,
					RSC_FEATURES_APIC_CODE_FR),
	[RSC_FEATURES_CORE_MP]	= LDT(	RSC_FEATURES_CORE_MP_CODE_EN,
					RSC_FEATURES_CORE_MP_CODE_FR),
	[RSC_FEATURES_CNXT_ID]	= LDT(	RSC_FEATURES_CNXT_ID_CODE_EN,
					RSC_FEATURES_CNXT_ID_CODE_FR),
	[RSC_FEATURES_DCA]	= LDT(	RSC_FEATURES_DCA_CODE_EN,
					RSC_FEATURES_DCA_CODE_FR),
	[RSC_FEATURES_DE]	= LDT(	RSC_FEATURES_DE_CODE_EN,
					RSC_FEATURES_DE_CODE_FR),
	[RSC_FEATURES_DS_PEBS]	= LDT(	RSC_FEATURES_DS_PEBS_CODE_EN,
					RSC_FEATURES_DS_PEBS_CODE_FR),
	[RSC_FEATURES_DS_CPL]	= LDT(	RSC_FEATURES_DS_CPL_CODE_EN,
					RSC_FEATURES_DS_CPL_CODE_FR),
	[RSC_FEATURES_DTES_64]	= LDT(	RSC_FEATURES_DTES_64_CODE_EN,
					RSC_FEATURES_DTES_64_CODE_FR),
	[RSC_FEATURES_FAST_STR] = LDT(	RSC_FEATURES_FAST_STR_CODE_EN,
					RSC_FEATURES_FAST_STR_CODE_FR),
	[RSC_FEATURES_FMA]	= LDT(	RSC_FEATURES_FMA_CODE_EN,
					RSC_FEATURES_FMA_CODE_FR),
	[RSC_FEATURES_HLE]	= LDT(	RSC_FEATURES_HLE_CODE_EN,
					RSC_FEATURES_HLE_CODE_FR),
	[RSC_FEATURES_IBS]	= LDT(	RSC_FEATURES_IBS_CODE_EN,
					RSC_FEATURES_IBS_CODE_FR),
	[RSC_FEATURES_LM]	= LDT(	RSC_FEATURES_LM_CODE_EN,
					RSC_FEATURES_LM_CODE_FR),
	[RSC_FEATURES_LWP]	= LDT(	RSC_FEATURES_LWP_CODE_EN,
					RSC_FEATURES_LWP_CODE_FR),
	[RSC_FEATURES_MCA]	= LDT(	RSC_FEATURES_MCA_CODE_EN,
					RSC_FEATURES_MCA_CODE_FR),
	[RSC_FEATURES_MPX]	= LDT(	RSC_FEATURES_MPX_CODE_EN,
					RSC_FEATURES_MPX_CODE_FR),
	[RSC_FEATURES_MSR]	= LDT(	RSC_FEATURES_MSR_CODE_EN,
					RSC_FEATURES_MSR_CODE_FR),
	[RSC_FEATURES_MTRR]	= LDT(	RSC_FEATURES_MTRR_CODE_EN,
					RSC_FEATURES_MTRR_CODE_FR),
	[RSC_FEATURES_NX]	= LDT(	RSC_FEATURES_NX_CODE_EN,
					RSC_FEATURES_NX_CODE_FR),
	[RSC_FEATURES_OSXSAVE]	= LDT(	RSC_FEATURES_OSXSAVE_CODE_EN,
					RSC_FEATURES_OSXSAVE_CODE_FR),
	[RSC_FEATURES_PAE]	= LDT(	RSC_FEATURES_PAE_CODE_EN,
					RSC_FEATURES_PAE_CODE_FR),
	[RSC_FEATURES_PAT]	= LDT(	RSC_FEATURES_PAT_CODE_EN,
					RSC_FEATURES_PAT_CODE_FR),
	[RSC_FEATURES_PBE]	= LDT(	RSC_FEATURES_PBE_CODE_EN,
					RSC_FEATURES_PBE_CODE_FR),
	[RSC_FEATURES_PCID]	= LDT(	RSC_FEATURES_PCID_CODE_EN,
					RSC_FEATURES_PCID_CODE_FR),
	[RSC_FEATURES_PDCM]	= LDT(	RSC_FEATURES_PDCM_CODE_EN,
					RSC_FEATURES_PDCM_CODE_FR),
	[RSC_FEATURES_PGE]	= LDT(	RSC_FEATURES_PGE_CODE_EN,
					RSC_FEATURES_PGE_CODE_FR),
	[RSC_FEATURES_PSE]	= LDT(	RSC_FEATURES_PSE_CODE_EN,
					RSC_FEATURES_PSE_CODE_FR),
	[RSC_FEATURES_PSE36]	= LDT(	RSC_FEATURES_PSE36_CODE_EN,
					RSC_FEATURES_PSE36_CODE_FR),
	[RSC_FEATURES_PSN]	= LDT(	RSC_FEATURES_PSN_CODE_EN,
					RSC_FEATURES_PSN_CODE_FR),
	[RSC_FEATURES_RDT_PQE]	= LDT(	RSC_FEATURES_RDT_PQE_CODE_EN,
					RSC_FEATURES_RDT_PQE_CODE_FR),
	[RSC_FEATURES_RDT_PQM]	= LDT(	RSC_FEATURES_RDT_PQM_CODE_EN,
					RSC_FEATURES_RDT_PQM_CODE_FR),
	[RSC_FEATURES_RTM]	= LDT(	RSC_FEATURES_RTM_CODE_EN,
					RSC_FEATURES_RTM_CODE_FR),
	[RSC_FEATURES_SMX]	= LDT(	RSC_FEATURES_SMX_CODE_EN,
					RSC_FEATURES_SMX_CODE_FR),
	[RSC_FEATURES_SELF_SNOOP]=LDT(	RSC_FEATURES_SELF_SNOOP_CODE_EN,
					RSC_FEATURES_SELF_SNOOP_CODE_FR),
	[RSC_FEATURES_SMAP]	= LDT(	RSC_FEATURES_SMAP_CODE_EN,
					RSC_FEATURES_SMAP_CODE_FR),
	[RSC_FEATURES_SMEP]	= LDT(	RSC_FEATURES_SMEP_CODE_EN,
					RSC_FEATURES_SMEP_CODE_FR),
	[RSC_FEATURES_TSC]	= LDT(	RSC_FEATURES_TSC_CODE_EN,
					RSC_FEATURES_TSC_CODE_FR),
	[RSC_FEATURES_TSC_DEADLN]=LDT(	RSC_FEATURES_TSC_DEADLN_CODE_EN,
					RSC_FEATURES_TSC_DEADLN_CODE_FR),
	[RSC_FEATURES_TSXABORT] = LDT(	RSC_FEATURES_TSXABORT_CODE_EN,
					RSC_FEATURES_TSXABORT_CODE_FR),
	[RSC_FEATURES_TSXLDTRK] = LDT(	RSC_FEATURES_TSXLDTRK_CODE_EN,
					RSC_FEATURES_TSXLDTRK_CODE_FR),
	[RSC_FEATURES_UMIP]	= LDT(	RSC_FEATURES_UMIP_CODE_EN,
					RSC_FEATURES_UMIP_CODE_FR),
	[RSC_FEATURES_VME]	= LDT(	RSC_FEATURES_VME_CODE_EN,
					RSC_FEATURES_VME_CODE_FR),
	[RSC_FEATURES_VMX]	= LDT(	RSC_FEATURES_VMX_CODE_EN,
					RSC_FEATURES_VMX_CODE_FR),
	[RSC_FEATURES_X2APIC]	= LDT(	RSC_FEATURES_X2APIC_CODE_EN,
					RSC_FEATURES_X2APIC_CODE_FR),
	[RSC_FEATURES_XD_BIT]	= LDT(	RSC_FEATURES_XD_BIT_CODE_EN,
					RSC_FEATURES_XD_BIT_CODE_FR),
	[RSC_FEATURES_XSAVE]	= LDT(	RSC_FEATURES_XSAVE_CODE_EN,
					RSC_FEATURES_XSAVE_CODE_FR),
	[RSC_FEATURES_XTPR]	= LDT(	RSC_FEATURES_XTPR_CODE_EN,
					RSC_FEATURES_XTPR_CODE_FR),
	[RSC_FEAT_SECTION_MECH] = LDT(	RSC_FEAT_SECTION_MECH_CODE_EN,
					RSC_FEAT_SECTION_MECH_CODE_FR),
	[RSC_TECHNOLOGIES_TITLE]= LDT(	RSC_TECHNOLOGIES_TITLE_CODE_EN,
					RSC_TECHNOLOGIES_TITLE_CODE_FR),
	[RSC_TECHNOLOGIES_DCU]  = LDT(	RSC_TECHNOLOGIES_DCU_CODE_EN,
					RSC_TECHNOLOGIES_DCU_CODE_FR),
	[RSC_TECH_L1_HW_PREFETCH] = LDT(RSC_TECH_L1_HW_PREFETCH_CODE_EN,
					RSC_TECH_L1_HW_PREFETCH_CODE_FR),
      [RSC_TECH_L1_HW_IP_PREFETCH] =LDT(RSC_TECH_L1_HW_IP_PREFETCH_CODE_EN,
					RSC_TECH_L1_HW_IP_PREFETCH_CODE_FR),
	[RSC_TECH_L2_HW_PREFETCH] = LDT(RSC_TECH_L2_HW_PREFETCH_CODE_EN,
					RSC_TECH_L2_HW_PREFETCH_CODE_FR),
      [RSC_TECH_L2_HW_CL_PREFETCH] =LDT(RSC_TECH_L2_HW_CL_PREFETCH_CODE_EN,
					RSC_TECH_L2_HW_CL_PREFETCH_CODE_FR),
	[RSC_TECHNOLOGIES_SMM]	= LDT(	RSC_TECHNOLOGIES_SMM_CODE_EN,
					RSC_TECHNOLOGIES_SMM_CODE_FR),
	[RSC_TECHNOLOGIES_HTT]	= LDT(	RSC_TECHNOLOGIES_HTT_CODE_EN,
					RSC_TECHNOLOGIES_HTT_CODE_FR),
	[RSC_TECHNOLOGIES_EIST] = LDT(	RSC_TECHNOLOGIES_EIST_CODE_EN,
					RSC_TECHNOLOGIES_EIST_CODE_FR),
	[RSC_TECHNOLOGIES_IDA]	= LDT(	RSC_TECHNOLOGIES_IDA_CODE_EN,
					RSC_TECHNOLOGIES_IDA_CODE_FR),
	[RSC_TECHNOLOGIES_TURBO]= LDT(	RSC_TECHNOLOGIES_TURBO_CODE_EN,
					RSC_TECHNOLOGIES_TURBO_CODE_FR),
	[RSC_TECHNOLOGIES_TBMT3] = LDT( RSC_TECHNOLOGIES_TBMT3_CODE_EN,
					RSC_TECHNOLOGIES_TBMT3_CODE_FR),
	[RSC_TECHNOLOGIES_VM]	= LDT(	RSC_TECHNOLOGIES_VM_CODE_EN,
					RSC_TECHNOLOGIES_VM_CODE_FR),
	[RSC_TECHNOLOGIES_IOMMU]= LDT(	RSC_TECHNOLOGIES_IOMMU_CODE_EN,
					RSC_TECHNOLOGIES_IOMMU_CODE_FR),
	[RSC_TECHNOLOGIES_SMT]	= LDT(	RSC_TECHNOLOGIES_SMT_CODE_EN,
					RSC_TECHNOLOGIES_SMT_CODE_FR),
	[RSC_TECHNOLOGIES_CNQ]	= LDT(	RSC_TECHNOLOGIES_CNQ_CODE_EN,
					RSC_TECHNOLOGIES_CNQ_CODE_FR),
	[RSC_TECHNOLOGIES_CPB]	= LDT(	RSC_TECHNOLOGIES_CPB_CODE_EN,
					RSC_TECHNOLOGIES_CPB_CODE_FR),
	[RSC_TECHNOLOGIES_EEO]	= LDT(	RSC_TECHNOLOGIES_EEO_CODE_EN,
					RSC_TECHNOLOGIES_EEO_CODE_FR),
	[RSC_TECHNOLOGIES_R2H]	= LDT(	RSC_TECHNOLOGIES_R2H_CODE_EN,
					RSC_TECHNOLOGIES_R2H_CODE_FR),
	[RSC_TECHNOLOGIES_HYPERV]=LDT(	RSC_TECHNOLOGIES_HYPERV_CODE_EN,
					RSC_TECHNOLOGIES_HYPERV_CODE_FR),
	[RSC_TECHNOLOGIES_WDT]	= LDQ(	RSC_TECHNOLOGIES_WDT_CODE),
	[RSC_TECH_HYPERV_NONE]	= LDQ(	RSC_TECH_HYPERV_NONE_CODE),
	[RSC_TECH_BARE_METAL]	= LDQ(	RSC_TECH_BARE_METAL_CODE),
	[RSC_TECH_HYPERV_XEN]	= LDQ(	RSC_TECH_HYPERV_XEN_CODE),
	[RSC_TECH_HYPERV_KVM]	= LDQ(	RSC_TECH_HYPERV_KVM_CODE),
	[RSC_TECH_HYPERV_VBOX]	= LDQ(	RSC_TECH_HYPERV_VBOX_CODE),
	[RSC_TECH_HYPERV_KBOX]	= LDQ(	RSC_TECH_HYPERV_KBOX_CODE),
	[RSC_TECH_HYPERV_VMWARE]= LDQ(	RSC_TECH_HYPERV_VMWARE_CODE),
	[RSC_TECH_HYPERV_HYPERV]= LDQ(	RSC_TECH_HYPERV_HYPERV_CODE),
	[RSC_PERF_MON_TITLE]	= LDT(	RSC_PERF_MON_TITLE_CODE_EN,
					RSC_PERF_MON_TITLE_CODE_FR),
	[RSC_VERSION]		= LDT(	RSC_VERSION_CODE_EN,
					RSC_VERSION_CODE_FR),
	[RSC_COUNTERS]		= LDT(	RSC_COUNTERS_CODE_EN,
					RSC_COUNTERS_CODE_FR),
	[RSC_GENERAL_CTRS]	= LDT(	RSC_GENERAL_CTRS_CODE_EN,
					RSC_GENERAL_CTRS_CODE_FR),
	[RSC_FIXED_CTRS]	= LDT(	RSC_FIXED_CTRS_CODE_EN,
					RSC_FIXED_CTRS_CODE_FR),
	[RSC_PERF_MON_UNIT_BIT] = LDT(	RSC_PERF_MON_UNIT_BIT_CODE_EN,
					RSC_PERF_MON_UNIT_BIT_CODE_FR),
	[RSC_PERF_MON_UNIT_HWP] = LDT(	RSC_PERF_MON_UNIT_HWP_CODE_EN,
					RSC_PERF_MON_UNIT_HWP_CODE_FR),
	[RSC_PERF_MON_C1E]	= LDT(	RSC_PERF_MON_C1E_CODE_EN,
					RSC_PERF_MON_C1E_CODE_FR),
	[RSC_PERF_MON_C1A]	= LDT(	RSC_PERF_MON_C1A_CODE_EN,
					RSC_PERF_MON_C1A_CODE_FR),
	[RSC_PERF_MON_C3A]	= LDT(	RSC_PERF_MON_C3A_CODE_EN,
					RSC_PERF_MON_C3A_CODE_FR),
	[RSC_PERF_MON_C1U]	= LDT(	RSC_PERF_MON_C1U_CODE_EN,
					RSC_PERF_MON_C1U_CODE_FR),
	[RSC_PERF_MON_C2U]	= LDT(	RSC_PERF_MON_C2U_CODE_EN,
					RSC_PERF_MON_C2U_CODE_FR),
	[RSC_PERF_MON_C3U]	= LDT(	RSC_PERF_MON_C3U_CODE_EN,
					RSC_PERF_MON_C3U_CODE_FR),
	[RSC_PERF_MON_C6D]	= LDT(	RSC_PERF_MON_C6D_CODE_EN,
					RSC_PERF_MON_C6D_CODE_FR),
	[RSC_PERF_MON_MC6]	= LDT(	RSC_PERF_MON_MC6_CODE_EN,
					RSC_PERF_MON_MC6_CODE_FR),
	[RSC_PERF_MON_CC6]	= LDT(	RSC_PERF_MON_CC6_CODE_EN,
					RSC_PERF_MON_CC6_CODE_FR),
	[RSC_PERF_MON_PC6]	= LDT(	RSC_PERF_MON_PC6_CODE_EN,
					RSC_PERF_MON_PC6_CODE_FR),
	[RSC_PERF_MON_FID]	= LDT(	RSC_PERF_MON_FID_CODE_EN,
					RSC_PERF_MON_FID_CODE_FR),
	[RSC_PERF_MON_VID]	= LDT(	RSC_PERF_MON_VID_CODE_EN,
					RSC_PERF_MON_VID_CODE_FR),
	[RSC_PERF_MON_HWCF]	= LDT(	RSC_PERF_MON_HWCF_CODE_EN,
					RSC_PERF_MON_HWCF_CODE_FR),
	[RSC_PERF_MON_HWP]	= LDT(	RSC_PERF_MON_HWP_CODE_EN,
					RSC_PERF_MON_HWP_CODE_FR),
	[RSC_PERF_MON_HDC]	= LDT(	RSC_PERF_MON_HDC_CODE_EN,
					RSC_PERF_MON_HDC_CODE_FR),
	[RSC_PERF_MON_PKG_CSTATE]=LDT(	RSC_PERF_MON_PKG_CSTATE_CODE_EN,
					RSC_PERF_MON_PKG_CSTATE_CODE_FR),
	[RSC_PERF_MON_CORE_CSTATE]=LDT( RSC_PERF_MON_CORE_CSTATE_CODE_EN,
					RSC_PERF_MON_CORE_CSTATE_CODE_FR),
	[RSC_PERF_MON_CFG_CTRL] = LDT(	RSC_PERF_MON_CFG_CTRL_CODE_EN,
					RSC_PERF_MON_CFG_CTRL_CODE_FR),
	[RSC_PERF_MON_LOW_CSTATE]=LDT(	RSC_PERF_MON_LOW_CSTATE_CODE_EN,
					RSC_PERF_MON_LOW_CSTATE_CODE_FR),
	[RSC_PERF_MON_IOMWAIT]	= LDT(	RSC_PERF_MON_IOMWAIT_CODE_EN,
					RSC_PERF_MON_IOMWAIT_CODE_FR),
	[RSC_PERF_MON_MAX_CSTATE]=LDT(	RSC_PERF_MON_MAX_CSTATE_CODE_EN,
					RSC_PERF_MON_MAX_CSTATE_CODE_FR),
	[RSC_PERF_MON_CSTATE_BAR]=LDT(	RSC_PERF_MON_CSTATE_BAR_CODE_EN,
					RSC_PERF_MON_CSTATE_BAR_CODE_FR),
    [RSC_PERF_MON_MONITOR_MWAIT]= LDT(	RSC_PERF_MON_MONITOR_MWAIT_CODE_EN,
					RSC_PERF_MON_MONITOR_MWAIT_CODE_FR),
    [RSC_PERF_MON_MWAIT_IDX_CSTATE]=LDT(RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_EN,
					RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_FR),
    [RSC_PERF_MON_MWAIT_SUB_CSTATE]=LDT(RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_EN,
					RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_FR),
	[RSC_PERF_MON_CORE_CYCLE]=LDT(	RSC_PERF_MON_CORE_CYCLE_CODE_EN,
					RSC_PERF_MON_CORE_CYCLE_CODE_FR),
	[RSC_PERF_MON_INST_RET] = LDT(	RSC_PERF_MON_INST_RET_CODE_EN,
					RSC_PERF_MON_INST_RET_CODE_FR),
	[RSC_PERF_MON_REF_CYCLE]= LDT(	RSC_PERF_MON_REF_CYCLE_CODE_EN,
					RSC_PERF_MON_REF_CYCLE_CODE_FR),
	[RSC_PERF_MON_REF_LLC]	= LDT(	RSC_PERF_MON_REF_LLC_CODE_EN,
					RSC_PERF_MON_REF_LLC_CODE_FR),
	[RSC_PERF_MON_MISS_LLC] = LDT(	RSC_PERF_MON_MISS_LLC_CODE_EN,
					RSC_PERF_MON_MISS_LLC_CODE_FR),
	[RSC_PERF_MON_BRANCH_RET]=LDT(	RSC_PERF_MON_BRANCH_RET_CODE_EN,
					RSC_PERF_MON_BRANCH_RET_CODE_FR),
	[RSC_PERF_MON_BRANCH_MIS]=LDT(	RSC_PERF_MON_BRANCH_MIS_CODE_EN,
					RSC_PERF_MON_BRANCH_MIS_CODE_FR),
	[RSC_PERF_MON_TSC]	= LDT(	RSC_PERF_MON_TSC_CODE_EN,
					RSC_PERF_MON_TSC_CODE_FR),
	[RSC_PERF_MON_NB_DF]	= LDT(	RSC_PERF_MON_NB_DF_CODE_EN,
					RSC_PERF_MON_NB_DF_CODE_FR),
	[RSC_PERF_MON_CORE]	= LDT(	RSC_PERF_MON_CORE_CODE_EN,
					RSC_PERF_MON_CORE_CODE_FR),
	[RSC_PERF_LABEL_VER]	= LDQ(	RSC_PERF_LABEL_VER_CODE),
	[RSC_PERF_LABEL_C1E]	= LDQ(	RSC_PERF_LABEL_C1E_CODE),
	[RSC_PERF_LABEL_C1A]	= LDQ(	RSC_PERF_LABEL_C1A_CODE),
	[RSC_PERF_LABEL_C3A]	= LDQ(	RSC_PERF_LABEL_C3A_CODE),
	[RSC_PERF_LABEL_C1U]	= LDQ(	RSC_PERF_LABEL_C1U_CODE),
	[RSC_PERF_LABEL_C2U]	= LDQ(	RSC_PERF_LABEL_C2U_CODE),
	[RSC_PERF_LABEL_C3U]	= LDQ(	RSC_PERF_LABEL_C3U_CODE),
	[RSC_PERF_LABEL_CC6]	= LDQ(	RSC_PERF_LABEL_CC6_CODE),
	[RSC_PERF_LABEL_PC6]	= LDQ(	RSC_PERF_LABEL_PC6_CODE),
	[RSC_PERF_LABEL_MC6]	= LDQ(	RSC_PERF_LABEL_MC6_CODE),
	[RSC_PERF_LABEL_FID]	= LDQ(	RSC_PERF_LABEL_FID_CODE),
	[RSC_PERF_LABEL_VID]	= LDQ(	RSC_PERF_LABEL_VID_CODE),
	[RSC_PERF_LABEL_HWCF]	= LDQ(	RSC_PERF_LABEL_HWCF_CODE),
	[RSC_PERF_LABEL_HWP]	= LDQ(	RSC_PERF_LABEL_HWP_CODE),
	[RSC_PERF_LABEL_HDC]	= LDQ(	RSC_PERF_LABEL_HDC_CODE),
	[RSC_PERF_LABEL_CFG_CTRL]=LDQ(	RSC_PERF_LABEL_CFG_CTRL_CODE),
	[RSC_PERF_LABEL_LOW_CST]= LDQ(	RSC_PERF_LABEL_LOW_CST_CODE),
	[RSC_PERF_LABEL_IOMWAIT]= LDQ(	RSC_PERF_LABEL_IOMWAIT_CODE),
	[RSC_PERF_LABEL_MAX_CST]= LDQ(	RSC_PERF_LABEL_MAX_CST_CODE),
	[RSC_PERF_LABEL_CST_BAR]= LDQ(	RSC_PERF_LABEL_CST_BAR_CODE),
	[RSC_PERF_LABEL_MWAIT_IDX]=LDQ( RSC_PERF_LABEL_MWAIT_IDX_CODE),
	[RSC_PERF_ENCODING_C0]	= LDQ(	RSC_PERF_ENCODING_C0_CODE),
	[RSC_PERF_ENCODING_C1]	= LDQ(	RSC_PERF_ENCODING_C1_CODE),
	[RSC_PERF_ENCODING_C2]	= LDQ(	RSC_PERF_ENCODING_C2_CODE),
	[RSC_PERF_ENCODING_C3]	= LDQ(	RSC_PERF_ENCODING_C3_CODE),
	[RSC_PERF_ENCODING_C4]	= LDQ(	RSC_PERF_ENCODING_C4_CODE),
	[RSC_PERF_ENCODING_C6]	= LDQ(	RSC_PERF_ENCODING_C6_CODE),
	[RSC_PERF_ENCODING_C6R]	= LDQ(	RSC_PERF_ENCODING_C6R_CODE),
	[RSC_PERF_ENCODING_C7]	= LDQ(	RSC_PERF_ENCODING_C7_CODE),
	[RSC_PERF_ENCODING_C7S] = LDQ(	RSC_PERF_ENCODING_C7S_CODE),
	[RSC_PERF_ENCODING_C8]	= LDQ(	RSC_PERF_ENCODING_C8_CODE),
	[RSC_PERF_ENCODING_C9]	= LDQ(	RSC_PERF_ENCODING_C9_CODE),
	[RSC_PERF_ENCODING_C10] = LDQ(	RSC_PERF_ENCODING_C10_CODE),
	[RSC_PERF_ENCODING_UNS] = LDQ(	RSC_PERF_ENCODING_UNS_CODE),
	[RSC_POWER_THERMAL_TITLE]=LDT(	RSC_POWER_THERMAL_TITLE_CODE_EN,
					RSC_POWER_THERMAL_TITLE_CODE_FR),
	[RSC_POWER_THERMAL_ODCM]=LDT(	RSC_POWER_THERMAL_ODCM_CODE_EN,
					RSC_POWER_THERMAL_ODCM_CODE_FR),
	[RSC_POWER_THERMAL_DUTY]=LDT(	RSC_POWER_THERMAL_DUTY_CODE_EN,
					RSC_POWER_THERMAL_DUTY_CODE_FR),
	[RSC_POWER_THERMAL_MGMT]=LDT(	RSC_POWER_THERMAL_MGMT_CODE_EN,
					RSC_POWER_THERMAL_MGMT_CODE_FR),
	[RSC_POWER_THERMAL_BIAS]=LDT(	RSC_POWER_THERMAL_BIAS_CODE_EN,
					RSC_POWER_THERMAL_BIAS_CODE_FR),
	[RSC_POWER_THERMAL_TJMAX]=LDT(	RSC_POWER_THERMAL_TJMAX_CODE_EN,
					RSC_POWER_THERMAL_TJMAX_CODE_FR),
	[RSC_POWER_THERMAL_DTS] = LDT(	RSC_POWER_THERMAL_DTS_CODE_EN,
					RSC_POWER_THERMAL_DTS_CODE_FR),
	[RSC_POWER_THERMAL_PLN] = LDT(	RSC_POWER_THERMAL_PLN_CODE_EN,
					RSC_POWER_THERMAL_PLN_CODE_FR),
	[RSC_POWER_THERMAL_PTM] = LDT(	RSC_POWER_THERMAL_PTM_CODE_EN,
					RSC_POWER_THERMAL_PTM_CODE_FR),
	[RSC_POWER_THERMAL_TM1] = LDT(	RSC_POWER_THERMAL_TM1_CODE_EN,
					RSC_POWER_THERMAL_TM1_CODE_FR),
	[RSC_POWER_THERMAL_TM2] = LDT(	RSC_POWER_THERMAL_TM2_CODE_EN,
					RSC_POWER_THERMAL_TM2_CODE_FR),
	[RSC_POWER_THERMAL_UNITS]=LDT(	RSC_POWER_THERMAL_UNITS_CODE_EN,
					RSC_POWER_THERMAL_UNITS_CODE_FR),
	[RSC_POWER_THERMAL_POWER]=LDT(	RSC_POWER_THERMAL_POWER_CODE_EN,
					RSC_POWER_THERMAL_POWER_CODE_FR),
	[RSC_POWER_THERMAL_ENERGY]=LDT( RSC_POWER_THERMAL_ENERGY_CODE_EN,
					RSC_POWER_THERMAL_ENERGY_CODE_FR),
	[RSC_POWER_THERMAL_WINDOW]=LDT( RSC_POWER_THERMAL_WINDOW_CODE_EN,
					RSC_POWER_THERMAL_WINDOW_CODE_FR),
	[RSC_POWER_THERMAL_WATT]= LDT(	RSC_POWER_THERMAL_WATT_CODE_EN,
					RSC_POWER_THERMAL_WATT_CODE_FR),
	[RSC_POWER_THERMAL_JOULE]=LDT(	RSC_POWER_THERMAL_JOULE_CODE_EN,
					RSC_POWER_THERMAL_JOULE_CODE_FR),
	[RSC_POWER_THERMAL_SECOND]=LDT( RSC_POWER_THERMAL_SECOND_CODE_EN,
					RSC_POWER_THERMAL_SECOND_CODE_FR),
	[RSC_POWER_THERMAL_TDP] = LDT(	RSC_POWER_THERMAL_TDP_CODE_EN,
					RSC_POWER_THERMAL_TDP_CODE_FR),
	[RSC_POWER_THERMAL_MIN] = LDT(	RSC_POWER_THERMAL_MIN_CODE_EN,
					RSC_POWER_THERMAL_MIN_CODE_FR),
	[RSC_POWER_THERMAL_MAX] = LDT(	RSC_POWER_THERMAL_MAX_CODE_EN,
					RSC_POWER_THERMAL_MAX_CODE_FR),
	[RSC_POWER_THERMAL_PPT] = LDT(	RSC_POWER_THERMAL_PPT_CODE_EN,
					RSC_POWER_THERMAL_PPT_CODE_FR),
	[RSC_POWER_THERMAL_TPL] = LDT(	RSC_POWER_THERMAL_TPL_CODE_EN,
					RSC_POWER_THERMAL_TPL_CODE_FR),
	[RSC_POWER_THERMAL_EDC] = LDT(	RSC_POWER_THERMAL_EDC_CODE_EN,
					RSC_POWER_THERMAL_EDC_CODE_FR),
	[RSC_POWER_THERMAL_TDC] = LDT(	RSC_POWER_THERMAL_TDC_CODE_EN,
					RSC_POWER_THERMAL_TDC_CODE_FR),
	[RSC_POWER_LABEL_ODCM]	= LDQ(	RSC_POWER_LABEL_ODCM_CODE),
	[RSC_POWER_LABEL_PWM]	= LDQ(	RSC_POWER_LABEL_PWM_CODE),
	[RSC_POWER_LABEL_BIAS]	= LDQ(	RSC_POWER_LABEL_BIAS_CODE),
	[RSC_POWER_LABEL_EPP]	= LDQ(	RSC_POWER_LABEL_EPP_CODE),
	[RSC_POWER_LABEL_TJ]	= LDQ(	RSC_POWER_LABEL_TJ_CODE),
	[RSC_POWER_LABEL_DTS]	= LDQ(	RSC_POWER_LABEL_DTS_CODE),
	[RSC_POWER_LABEL_PLN]	= LDQ(	RSC_POWER_LABEL_PLN_CODE),
	[RSC_POWER_LABEL_PTM]	= LDQ(	RSC_POWER_LABEL_PTM_CODE),
	[RSC_POWER_LABEL_TM1]	= LDQ(	RSC_POWER_LABEL_TM1_CODE),
	[RSC_POWER_LABEL_TM2]	= LDQ(	RSC_POWER_LABEL_TM2_CODE),
	[RSC_POWER_LABEL_TTP]	= LDQ(	RSC_POWER_LABEL_TTP_CODE),
	[RSC_POWER_LABEL_HTC]	= LDQ(	RSC_POWER_LABEL_HTC_CODE),
	[RSC_POWER_LABEL_TDP]	= LDQ(	RSC_POWER_LABEL_TDP_CODE),
	[RSC_POWER_LABEL_MIN]	= LDQ(	RSC_POWER_LABEL_MIN_CODE),
	[RSC_POWER_LABEL_MAX]	= LDQ(	RSC_POWER_LABEL_MAX_CODE),
	[RSC_POWER_LABEL_PPT]	= LDQ(	RSC_POWER_LABEL_PPT_CODE),
	[RSC_POWER_LABEL_PL1]	= LDQ(	RSC_POWER_LABEL_PL1_CODE),
	[RSC_POWER_LABEL_PL2]	= LDQ(	RSC_POWER_LABEL_PL2_CODE),
	[RSC_POWER_LABEL_EDC]	= LDQ(	RSC_POWER_LABEL_EDC_CODE),
	[RSC_POWER_LABEL_TDC]	= LDQ(	RSC_POWER_LABEL_TDC_CODE),
	[RSC_POWER_LABEL_PKG]	= LDQ(	RSC_POWER_LABEL_PKG_CODE),
	[RSC_POWER_LABEL_CORE]	= LDQ(	RSC_POWER_LABEL_CORE_CODE),
	[RSC_POWER_LABEL_UNCORE]= LDQ(	RSC_POWER_LABEL_UNCORE_CODE),
	[RSC_POWER_LABEL_DRAM]	= LDQ(	RSC_POWER_LABEL_DRAM_CODE),
	[RSC_POWER_LABEL_PLATFORM]=LDQ( RSC_POWER_LABEL_PLATFORM_CODE),
	[RSC_KERNEL_TITLE]	= LDT(	RSC_KERNEL_TITLE_CODE_EN,
					RSC_KERNEL_TITLE_CODE_FR),
	[RSC_KERNEL_TOTAL_RAM]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_TOTAL_RAM_CODE_EN,
					RSC_KERNEL_TOTAL_RAM_CODE_FR),
	[RSC_KERNEL_SHARED_RAM] = LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_SHARED_RAM_CODE_EN,
					RSC_KERNEL_SHARED_RAM_CODE_FR),
	[RSC_KERNEL_FREE_RAM]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_FREE_RAM_CODE_EN,
					RSC_KERNEL_FREE_RAM_CODE_FR),
	[RSC_KERNEL_BUFFER_RAM] = LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_BUFFER_RAM_CODE_EN,
					RSC_KERNEL_BUFFER_RAM_CODE_FR),
	[RSC_KERNEL_TOTAL_HIGH] = LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_TOTAL_HIGH_CODE_EN,
					RSC_KERNEL_TOTAL_HIGH_CODE_FR),
	[RSC_KERNEL_FREE_HIGH]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_FREE_HIGH_CODE_EN,
					RSC_KERNEL_FREE_HIGH_CODE_FR),
	[RSC_KERNEL_GOVERNOR]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_GOVERNOR_CODE_EN,
					RSC_KERNEL_GOVERNOR_CODE_FR),
	[RSC_KERNEL_FREQ_DRIVER]= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_FREQ_DRIVER_CODE_EN,
					RSC_KERNEL_FREQ_DRIVER_CODE_FR),
	[RSC_KERNEL_IDLE_DRIVER]= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_IDLE_DRIVER_CODE_EN,
					RSC_KERNEL_IDLE_DRIVER_CODE_FR),
	[RSC_KERNEL_RELEASE]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_RELEASE_CODE_EN,
					RSC_KERNEL_RELEASE_CODE_FR),
	[RSC_KERNEL_VERSION]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_VERSION_CODE_EN,
					RSC_KERNEL_VERSION_CODE_FR),
	[RSC_KERNEL_MACHINE]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_MACHINE_CODE_EN,
					RSC_KERNEL_MACHINE_CODE_FR),
	[RSC_KERNEL_MEMORY]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_MEMORY_CODE_EN,
					RSC_KERNEL_MEMORY_CODE_FR),
	[RSC_KERNEL_STATE]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_STATE_CODE_EN,
					RSC_KERNEL_STATE_CODE_FR),
	[RSC_KERNEL_POWER]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_POWER_CODE_EN,
					RSC_KERNEL_POWER_CODE_FR),
	[RSC_KERNEL_LATENCY]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_LATENCY_CODE_EN,
					RSC_KERNEL_LATENCY_CODE_FR),
	[RSC_KERNEL_RESIDENCY]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_RESIDENCY_CODE_EN,
					RSC_KERNEL_RESIDENCY_CODE_FR),
	[RSC_KERNEL_LIMIT]	= LDS(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_LIMIT_CODE_EN,
					RSC_KERNEL_LIMIT_CODE_FR),
	[RSC_TOPOLOGY_TITLE]	= LDT(	RSC_TOPOLOGY_TITLE_CODE_EN,
					RSC_TOPOLOGY_TITLE_CODE_FR),
	[RSC_TOPOLOGY_HDR_PKG]	= LDQ(	RSC_TOPOLOGY_HDR_PKG_CODE),
	[RSC_TOPOLOGY_HDR_SMT]	= LDQ(	RSC_TOPOLOGY_HDR_SMT_CODE),
	[RSC_TOPOLOGY_HDR_CACHE]= LDQ(	RSC_TOPOLOGY_HDR_CACHE_CODE),
	[RSC_TOPOLOGY_HDR_WRBAK]= LDQ(	RSC_TOPOLOGY_HDR_WRBAK_CODE),
	[RSC_TOPOLOGY_HDR_INCL] = LDQ(	RSC_TOPOLOGY_HDR_INCL_CODE),
	[RSC_TOPOLOGY_HDR_EMPTY]= LDQ(	RSC_TOPOLOGY_HDR_EMPTY_CODE),
	[RSC_TOPOLOGY_SUB_ITEM1]= LDQ(	RSC_TOPOLOGY_SUB_ITEM1_CODE),
	[RSC_TOPOLOGY_SUB_ITEM3]= LDQ(	RSC_TOPOLOGY_SUB_ITEM3_CODE),
	[RSC_TOPOLOGY_SUB_ITEM4]= LDQ(	RSC_TOPOLOGY_SUB_ITEM4_CODE),
	[RSC_TOPOLOGY_SUB_ITEM5]= LDQ(	RSC_TOPOLOGY_SUB_ITEM5_CODE),
	[RSC_TOPOLOGY_SUB_ITEM6]= LDQ(	RSC_TOPOLOGY_SUB_ITEM6_CODE),
	[RSC_TOPOLOGY_ALT_ITEM1]= LDQ(	RSC_TOPOLOGY_ALT_ITEM1_CODE),
	[RSC_TOPOLOGY_ALT_ITEM2]= LDQ(	RSC_TOPOLOGY_ALT_ITEM2_CODE),
	[RSC_TOPOLOGY_ALT_ITEM3]= LDQ(	RSC_TOPOLOGY_ALT_ITEM3_CODE),
	[RSC_MEM_CTRL_TITLE]	= LDT(	RSC_MEM_CTRL_TITLE_CODE_EN,
					RSC_MEM_CTRL_TITLE_CODE_FR),
	[RSC_MEM_CTRL_UNIT_MHZ] = LDQ(	RSC_MEM_CTRL_UNIT_MHZ_CODE),
	[RSC_MEM_CTRL_UNIT_MTS] = LDQ(	RSC_MEM_CTRL_UNIT_MTS_CODE),
	[RSC_MEM_CTRL_UNIT_MBS] = LDQ(	RSC_MEM_CTRL_UNIT_MBS_CODE),
	[RSC_MEM_CTRL_MTY_CELL] = LDQ(	RSC_MEM_CTRL_MTY_CELL_CODE),
	[RSC_MEM_CTRL_CHANNEL]	= LDQ(	RSC_MEM_CTRL_CHANNEL_CODE),
	[RSC_MEM_CTRL_SUBSECT1_0]=LDT(	RSC_MEM_CTRL_SUBSECT1_0_CODE_EN,
					RSC_MEM_CTRL_SUBSECT1_0_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT1_1]=LDT(	RSC_MEM_CTRL_SUBSECT1_1_CODE_EN,
					RSC_MEM_CTRL_SUBSECT1_1_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT1_2]=LDT(	RSC_MEM_CTRL_SUBSECT1_2_CODE_EN,
					RSC_MEM_CTRL_SUBSECT1_2_CODE_FR),
	[RSC_MEM_CTRL_SINGLE_CHA_0]=LDT(RSC_MEM_CTRL_SINGLE_CHA_0_CODE_EN,
					RSC_MEM_CTRL_SINGLE_CHA_0_CODE_FR),
	[RSC_MEM_CTRL_SINGLE_CHA_1]=LDT(RSC_MEM_CTRL_SINGLE_CHA_1_CODE_EN,
					RSC_MEM_CTRL_SINGLE_CHA_1_CODE_FR),
	[RSC_MEM_CTRL_SINGLE_CHA_2]=LDT(RSC_MEM_CTRL_SINGLE_CHA_2_CODE_EN,
					RSC_MEM_CTRL_SINGLE_CHA_2_CODE_FR),
	[RSC_MEM_CTRL_DUAL_CHA_0]=LDT(	RSC_MEM_CTRL_DUAL_CHA_0_CODE_EN,
					RSC_MEM_CTRL_DUAL_CHA_0_CODE_FR),
	[RSC_MEM_CTRL_DUAL_CHA_1]=LDT(	RSC_MEM_CTRL_DUAL_CHA_1_CODE_EN,
					RSC_MEM_CTRL_DUAL_CHA_1_CODE_FR),
	[RSC_MEM_CTRL_DUAL_CHA_2]=LDT(	RSC_MEM_CTRL_DUAL_CHA_2_CODE_EN,
					RSC_MEM_CTRL_DUAL_CHA_2_CODE_FR),
	[RSC_MEM_CTRL_TRIPLE_CHA_0]=LDT(RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_EN,
					RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_FR),
	[RSC_MEM_CTRL_TRIPLE_CHA_1]=LDT(RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_EN,
					RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_FR),
	[RSC_MEM_CTRL_TRIPLE_CHA_2]=LDT(RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_EN,
					RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_FR),
	[RSC_MEM_CTRL_QUAD_CHA_0]=LDT(	RSC_MEM_CTRL_QUAD_CHA_0_CODE_EN,
					RSC_MEM_CTRL_QUAD_CHA_0_CODE_FR),
	[RSC_MEM_CTRL_QUAD_CHA_1]=LDT(	RSC_MEM_CTRL_QUAD_CHA_1_CODE_EN,
					RSC_MEM_CTRL_QUAD_CHA_1_CODE_FR),
	[RSC_MEM_CTRL_QUAD_CHA_2]=LDT(	RSC_MEM_CTRL_QUAD_CHA_2_CODE_EN,
					RSC_MEM_CTRL_QUAD_CHA_2_CODE_FR),
	[RSC_MEM_CTRL_SIX_CHA_0]= LDT(	RSC_MEM_CTRL_SIX_CHA_0_CODE_EN,
					RSC_MEM_CTRL_SIX_CHA_0_CODE_FR),
	[RSC_MEM_CTRL_SIX_CHA_1]= LDT(	RSC_MEM_CTRL_SIX_CHA_1_CODE_EN,
					RSC_MEM_CTRL_SIX_CHA_1_CODE_FR),
	[RSC_MEM_CTRL_SIX_CHA_2]= LDT(	RSC_MEM_CTRL_SIX_CHA_2_CODE_EN,
					RSC_MEM_CTRL_SIX_CHA_2_CODE_FR),
	[RSC_MEM_CTRL_EIGHT_CHA_0]=LDT( RSC_MEM_CTRL_EIGHT_CHA_0_CODE_EN,
					RSC_MEM_CTRL_EIGHT_CHA_0_CODE_FR),
	[RSC_MEM_CTRL_EIGHT_CHA_1]=LDT( RSC_MEM_CTRL_EIGHT_CHA_1_CODE_EN,
					RSC_MEM_CTRL_EIGHT_CHA_1_CODE_FR),
	[RSC_MEM_CTRL_EIGHT_CHA_2]=LDT( RSC_MEM_CTRL_EIGHT_CHA_2_CODE_EN,
					RSC_MEM_CTRL_EIGHT_CHA_2_CODE_FR),
	[RSC_MEM_CTRL_BUS_RATE_0]=LDT(	RSC_MEM_CTRL_BUS_RATE_0_CODE_EN,
					RSC_MEM_CTRL_BUS_RATE_0_CODE_FR),
	[RSC_MEM_CTRL_BUS_RATE_1]=LDT(	RSC_MEM_CTRL_BUS_RATE_1_CODE_EN,
					RSC_MEM_CTRL_BUS_RATE_1_CODE_FR),
	[RSC_MEM_CTRL_BUS_SPEED_0]=LDT( RSC_MEM_CTRL_BUS_SPEED_0_CODE_EN,
					RSC_MEM_CTRL_BUS_SPEED_0_CODE_FR),
	[RSC_MEM_CTRL_BUS_SPEED_1]=LDT( RSC_MEM_CTRL_BUS_SPEED_1_CODE_EN,
					RSC_MEM_CTRL_BUS_SPEED_1_CODE_FR),
	[RSC_MEM_CTRL_DRAM_SPEED_0]=LDT(RSC_MEM_CTRL_DRAM_SPEED_0_CODE_EN,
					RSC_MEM_CTRL_DRAM_SPEED_0_CODE_FR),
	[RSC_MEM_CTRL_DRAM_SPEED_1]=LDT(RSC_MEM_CTRL_DRAM_SPEED_1_CODE_EN,
					RSC_MEM_CTRL_DRAM_SPEED_1_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT2_0]=LDT(	RSC_MEM_CTRL_SUBSECT2_0_CODE_EN,
					RSC_MEM_CTRL_SUBSECT2_0_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT2_1]=LDT(	RSC_MEM_CTRL_SUBSECT2_1_CODE_EN,
					RSC_MEM_CTRL_SUBSECT2_1_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT2_2]=LDT(	RSC_MEM_CTRL_SUBSECT2_2_CODE_EN,
					RSC_MEM_CTRL_SUBSECT2_2_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT2_3]=LDT(	RSC_MEM_CTRL_SUBSECT2_3_CODE_EN,
					RSC_MEM_CTRL_SUBSECT2_3_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT2_4]=LDT(	RSC_MEM_CTRL_SUBSECT2_4_CODE_EN,
					RSC_MEM_CTRL_SUBSECT2_4_CODE_FR),
	[RSC_MEM_CTRL_SUBSECT2_5]=LDT(	RSC_MEM_CTRL_SUBSECT2_5_CODE_EN,
					RSC_MEM_CTRL_SUBSECT2_5_CODE_FR),
	[RSC_MEM_CTRL_DIMM_SLOT]= LDT(	RSC_MEM_CTRL_DIMM_SLOT_CODE_EN,
					RSC_MEM_CTRL_DIMM_SLOT_CODE_FR),
	[RSC_MEM_CTRL_DIMM_BANK]= LDT(	RSC_MEM_CTRL_DIMM_BANK_CODE_EN,
					RSC_MEM_CTRL_DIMM_BANK_CODE_FR),
	[RSC_MEM_CTRL_DIMM_RANK]= LDT(	RSC_MEM_CTRL_DIMM_RANK_CODE_EN,
					RSC_MEM_CTRL_DIMM_RANK_CODE_FR),
	[RSC_MEM_CTRL_DIMM_ROW] = LDT(	RSC_MEM_CTRL_DIMM_ROW_CODE_EN,
					RSC_MEM_CTRL_DIMM_ROW_CODE_FR),
	[RSC_MEM_CTRL_DIMM_COLUMN0]=LDT(RSC_MEM_CTRL_DIMM_COLUMN0_CODE_EN,
					RSC_MEM_CTRL_DIMM_COLUMN0_CODE_FR),
	[RSC_MEM_CTRL_DIMM_COLUMN1]=LDT(RSC_MEM_CTRL_DIMM_COLUMN1_CODE_EN,
					RSC_MEM_CTRL_DIMM_COLUMN1_CODE_FR),
	[RSC_MEM_CTRL_DIMM_SIZE_0]=LDT( RSC_MEM_CTRL_DIMM_SIZE_0_CODE_EN,
					RSC_MEM_CTRL_DIMM_SIZE_0_CODE_FR),
	[RSC_MEM_CTRL_DIMM_SIZE_1]=LDT( RSC_MEM_CTRL_DIMM_SIZE_1_CODE_EN,
					RSC_MEM_CTRL_DIMM_SIZE_1_CODE_FR),
	[RSC_MEM_CTRL_DIMM_SIZE_2]=LDT( RSC_MEM_CTRL_DIMM_SIZE_2_CODE_EN,
					RSC_MEM_CTRL_DIMM_SIZE_2_CODE_FR),
	[RSC_MEM_CTRL_DIMM_SIZE_3]=LDT( RSC_MEM_CTRL_DIMM_SIZE_3_CODE_EN,
					RSC_MEM_CTRL_DIMM_SIZE_3_CODE_FR),
	[RSC_DDR3_CL]		= LDQ(	RSC_DDR3_CL_CODE),
	[RSC_DDR3_RCD]		= LDQ(	RSC_DDR3_RCD_CODE),
	[RSC_DDR3_RP]		= LDQ(	RSC_DDR3_RP_CODE),
	[RSC_DDR3_RAS]		= LDQ(	RSC_DDR3_RAS_CODE),
	[RSC_DDR3_RRD]		= LDQ(	RSC_DDR3_RRD_CODE),
	[RSC_DDR3_RFC]		= LDQ(	RSC_DDR3_RFC_CODE),
	[RSC_DDR3_WR]		= LDQ(	RSC_DDR3_WR_CODE),
	[RSC_DDR3_RTP]		= LDQ(	RSC_DDR3_RTP_CODE),
	[RSC_DDR3_WTP]		= LDQ(	RSC_DDR3_WTP_CODE),
	[RSC_DDR3_FAW]		= LDQ(	RSC_DDR3_FAW_CODE),
	[RSC_DDR3_B2B]		= LDQ(	RSC_DDR3_B2B_CODE),
	[RSC_DDR3_CWL]		= LDQ(	RSC_DDR3_CWL_CODE),
	[RSC_DDR3_CMD]		= LDQ(	RSC_DDR3_CMD_CODE),
	[RSC_DDR3_REFI] 	= LDQ(	RSC_DDR3_REFI_CODE),
	[RSC_DDR3_DDWRTRD]	= LDQ(	RSC_DDR3_DDWRTRD_CODE),
	[RSC_DDR3_DRWRTRD]	= LDQ(	RSC_DDR3_DRWRTRD_CODE),
	[RSC_DDR3_SRWRTRD]	= LDQ(	RSC_DDR3_SRWRTRD_CODE),
	[RSC_DDR3_DDRDTWR]	= LDQ(	RSC_DDR3_DDRDTWR_CODE),
	[RSC_DDR3_DRRDTWR]	= LDQ(	RSC_DDR3_DRRDTWR_CODE),
	[RSC_DDR3_SRRDTWR]	= LDQ(	RSC_DDR3_SRRDTWR_CODE),
	[RSC_DDR3_DDRDTRD]	= LDQ(	RSC_DDR3_DDRDTRD_CODE),
	[RSC_DDR3_DRRDTRD]	= LDQ(	RSC_DDR3_DRRDTRD_CODE),
	[RSC_DDR3_SRRDTRD]	= LDQ(	RSC_DDR3_SRRDTRD_CODE),
	[RSC_DDR3_DDWRTWR]	= LDQ(	RSC_DDR3_DDWRTWR_CODE),
	[RSC_DDR3_DRWRTWR]	= LDQ(	RSC_DDR3_DRWRTWR_CODE),
	[RSC_DDR3_SRWRTWR]	= LDQ(	RSC_DDR3_SRWRTWR_CODE),
	[RSC_DDR3_CKE]		= LDQ(	RSC_DDR3_CKE_CODE),
	[RSC_DDR3_ECC]		= LDQ(	RSC_DDR3_ECC_CODE),
	[RSC_DDR4_CL]		= LDQ(	RSC_DDR4_CL_CODE),
	[RSC_DDR4_RCD]		= LDQ(	RSC_DDR4_RCD_CODE),
	[RSC_DDR4_RP]		= LDQ(	RSC_DDR4_RP_CODE),
	[RSC_DDR4_RAS]		= LDQ(	RSC_DDR4_RAS_CODE),
	[RSC_DDR4_RRD]		= LDQ(	RSC_DDR4_RRD_CODE),
	[RSC_DDR4_RFC]		= LDQ(	RSC_DDR4_RFC_CODE),
	[RSC_DDR4_WR]		= LDQ(	RSC_DDR4_WR_CODE),
	[RSC_DDR4_RTP]		= LDQ(	RSC_DDR4_RTP_CODE),
	[RSC_DDR4_WTP]		= LDQ(	RSC_DDR4_WTP_CODE),
	[RSC_DDR4_FAW]		= LDQ(	RSC_DDR4_FAW_CODE),
	[RSC_DDR4_B2B]		= LDQ(	RSC_DDR4_B2B_CODE),
	[RSC_DDR4_CWL]		= LDQ(	RSC_DDR4_CWL_CODE),
	[RSC_DDR4_CMD]		= LDQ(	RSC_DDR4_CMD_CODE),
	[RSC_DDR4_REFI] 	= LDQ(	RSC_DDR4_REFI_CODE),
	[RSC_DDR4_RDRD_SCL]	= LDQ(	RSC_DDR4_RDRD_SCL_CODE),
	[RSC_DDR4_RDRD_SC]	= LDQ(	RSC_DDR4_RDRD_SC_CODE),
	[RSC_DDR4_RDRD_SD]	= LDQ(	RSC_DDR4_RDRD_SD_CODE),
	[RSC_DDR4_RDRD_DD]	= LDQ(	RSC_DDR4_RDRD_DD_CODE),
	[RSC_DDR4_RDWR_SCL]	= LDQ(	RSC_DDR4_RDWR_SCL_CODE),
	[RSC_DDR4_RDWR_SC]	= LDQ(	RSC_DDR4_RDWR_SC_CODE),
	[RSC_DDR4_RDWR_SD]	= LDQ(	RSC_DDR4_RDWR_SD_CODE),
	[RSC_DDR4_RDWR_DD]	= LDQ(	RSC_DDR4_RDWR_DD_CODE),
	[RSC_DDR4_WRRD_SCL]	= LDQ(	RSC_DDR4_WRRD_SCL_CODE),
	[RSC_DDR4_WRRD_SC]	= LDQ(	RSC_DDR4_WRRD_SC_CODE),
	[RSC_DDR4_WRRD_SD]	= LDQ(	RSC_DDR4_WRRD_SD_CODE),
	[RSC_DDR4_WRRD_DD]	= LDQ(	RSC_DDR4_WRRD_DD_CODE),
	[RSC_DDR4_WRWR_SCL]	= LDQ(	RSC_DDR4_WRWR_SCL_CODE),
	[RSC_DDR4_WRWR_SC]	= LDQ(	RSC_DDR4_WRWR_SC_CODE),
	[RSC_DDR4_WRWR_SD]	= LDQ(	RSC_DDR4_WRWR_SD_CODE),
	[RSC_DDR4_WRWR_DD]	= LDQ(	RSC_DDR4_WRWR_DD_CODE),
	[RSC_DDR4_CKE]		= LDQ(	RSC_DDR4_CKE_CODE),
	[RSC_DDR4_ECC]		= LDQ(	RSC_DDR4_ECC_CODE),
	[RSC_DDR4_ZEN_CL]	= LDQ(	RSC_DDR4_ZEN_CL_CODE),
	[RSC_DDR4_ZEN_RCD_R]	= LDQ(	RSC_DDR4_ZEN_RCD_R_CODE),
	[RSC_DDR4_ZEN_RCD_W]	= LDQ(	RSC_DDR4_ZEN_RCD_W_CODE),
	[RSC_DDR4_ZEN_RP]	= LDQ(	RSC_DDR4_ZEN_RP_CODE),
	[RSC_DDR4_ZEN_RAS]	= LDQ(	RSC_DDR4_ZEN_RAS_CODE),
	[RSC_DDR4_ZEN_RC]	= LDQ(	RSC_DDR4_ZEN_RC_CODE),
	[RSC_DDR4_ZEN_RRD_S]	= LDQ(	RSC_DDR4_ZEN_RRD_S_CODE),
	[RSC_DDR4_ZEN_RRD_L]	= LDQ(	RSC_DDR4_ZEN_RRD_L_CODE),
	[RSC_DDR4_ZEN_FAW]	= LDQ(	RSC_DDR4_ZEN_FAW_CODE),
	[RSC_DDR4_ZEN_WTR_S]	= LDQ(	RSC_DDR4_ZEN_WTR_S_CODE),
	[RSC_DDR4_ZEN_WTR_L]	= LDQ(	RSC_DDR4_ZEN_WTR_L_CODE),
	[RSC_DDR4_ZEN_WR]	= LDQ(	RSC_DDR4_ZEN_WR_CODE),
	[RSC_DDR4_ZEN_RDRD_SCL] = LDQ(	RSC_DDR4_ZEN_RDRD_SCL_CODE),
	[RSC_DDR4_ZEN_WRWR_SCL] = LDQ(	RSC_DDR4_ZEN_WRWR_SCL_CODE),
	[RSC_DDR4_ZEN_CWL]	= LDQ(	RSC_DDR4_ZEN_CWL_CODE),
	[RSC_DDR4_ZEN_RTP]	= LDQ(	RSC_DDR4_ZEN_RTP_CODE),
	[RSC_DDR4_ZEN_RDWR]	= LDQ(	RSC_DDR4_ZEN_RDWR_CODE),
	[RSC_DDR4_ZEN_WRRD]	= LDQ(	RSC_DDR4_ZEN_WRRD_CODE),
	[RSC_DDR4_ZEN_WRWR_SC]	= LDQ(	RSC_DDR4_ZEN_WRWR_SC_CODE),
	[RSC_DDR4_ZEN_WRWR_SD]	= LDQ(	RSC_DDR4_ZEN_WRWR_SD_CODE),
	[RSC_DDR4_ZEN_WRWR_DD]	= LDQ(	RSC_DDR4_ZEN_WRWR_DD_CODE),
	[RSC_DDR4_ZEN_RDRD_SC]	= LDQ(	RSC_DDR4_ZEN_RDRD_SC_CODE),
	[RSC_DDR4_ZEN_RDRD_SD]	= LDQ(	RSC_DDR4_ZEN_RDRD_SD_CODE),
	[RSC_DDR4_ZEN_RDRD_DD]	= LDQ(	RSC_DDR4_ZEN_RDRD_DD_CODE),
	[RSC_DDR4_ZEN_RTR_DLR]	= LDQ(	RSC_DDR4_ZEN_RTR_DLR_CODE),
	[RSC_DDR4_ZEN_WTW_DLR]	= LDQ(	RSC_DDR4_ZEN_WTW_DLR_CODE),
	[RSC_DDR4_ZEN_WTR_DLR]	= LDQ(	RSC_DDR4_ZEN_WTR_DLR_CODE),
	[RSC_DDR4_ZEN_RRD_DLR]	= LDQ(	RSC_DDR4_ZEN_RRD_DLR_CODE),
	[RSC_DDR4_ZEN_REFI]	= LDQ(	RSC_DDR4_ZEN_REFI_CODE),
	[RSC_DDR4_ZEN_RFC1]	= LDQ(	RSC_DDR4_ZEN_RFC1_CODE),
	[RSC_DDR4_ZEN_RFC2]	= LDQ(	RSC_DDR4_ZEN_RFC2_CODE),
	[RSC_DDR4_ZEN_RFC4]	= LDQ(	RSC_DDR4_ZEN_RFC4_CODE),
	[RSC_DDR4_ZEN_RCPB]	= LDQ(	RSC_DDR4_ZEN_RCPB_CODE),
	[RSC_DDR4_ZEN_RPPB]	= LDQ(	RSC_DDR4_ZEN_RPPB_CODE),
	[RSC_DDR4_ZEN_BGS]	= LDQ(	RSC_DDR4_ZEN_BGS_CODE),
	[RSC_DDR4_ZEN_BGS_ALT]	= LDQ(	RSC_DDR4_ZEN_BGS_ALT_CODE),
	[RSC_DDR4_ZEN_BAN]	= LDQ(	RSC_DDR4_ZEN_BAN_CODE),
	[RSC_DDR4_ZEN_RCPAGE]	= LDQ(	RSC_DDR4_ZEN_RCPAGE_CODE),
	[RSC_DDR4_ZEN_CKE]	= LDQ(	RSC_DDR4_ZEN_CKE_CODE),
	[RSC_DDR4_ZEN_CMD]	= LDQ(	RSC_DDR4_ZEN_CMD_CODE),
	[RSC_DDR4_ZEN_GDM]	= LDQ(	RSC_DDR4_ZEN_GDM_CODE),
	[RSC_DDR4_ZEN_ECC]	= LDQ(	RSC_DDR4_ZEN_ECC_CODE),
	[RSC_DDR3_CL_COMM]	= LDT(	RSC_DDR3_CL_COMM_CODE_EN,
					RSC_DDR3_CL_COMM_CODE_FR),
	[RSC_DDR3_RCD_COMM]	= LDT(	RSC_DDR3_RCD_COMM_CODE_EN,
					RSC_DDR3_RCD_COMM_CODE_FR),
	[RSC_DDR3_RP_COMM]	= LDT(	RSC_DDR3_RP_COMM_CODE_EN,
					RSC_DDR3_RP_COMM_CODE_FR),
	[RSC_DDR3_RAS_COMM]	= LDT(	RSC_DDR3_RAS_COMM_CODE_EN,
					RSC_DDR3_RAS_COMM_CODE_FR),
	[RSC_DDR3_RRD_COMM]	= LDT(	RSC_DDR3_RRD_COMM_CODE_EN,
					RSC_DDR3_RRD_COMM_CODE_FR),
	[RSC_DDR3_RFC_COMM]	= LDT(	RSC_DDR3_RFC_COMM_CODE_EN,
					RSC_DDR3_RFC_COMM_CODE_FR),
	[RSC_DDR3_WR_COMM]	= LDT(	RSC_DDR3_WR_COMM_CODE_EN,
					RSC_DDR3_WR_COMM_CODE_FR),
	[RSC_DDR3_RTP_COMM]	= LDT(	RSC_DDR3_RTP_COMM_CODE_EN,
					RSC_DDR3_RTP_COMM_CODE_FR),
	[RSC_DDR3_WTP_COMM]	= LDT(	RSC_DDR3_WTP_COMM_CODE_EN,
					RSC_DDR3_WTP_COMM_CODE_FR),
	[RSC_DDR3_FAW_COMM]	= LDT(	RSC_DDR3_FAW_COMM_CODE_EN,
					RSC_DDR3_FAW_COMM_CODE_FR),
	[RSC_DDR3_B2B_COMM]	= LDT(	RSC_DDR3_B2B_COMM_CODE_EN,
					RSC_DDR3_B2B_COMM_CODE_FR),
	[RSC_DDR3_CWL_COMM]	= LDT(	RSC_DDR3_CWL_COMM_CODE_EN,
					RSC_DDR3_CWL_COMM_CODE_FR),
	[RSC_DDR3_CMD_COMM]	= LDT(	RSC_DDR3_CMD_COMM_CODE_EN,
					RSC_DDR3_CMD_COMM_CODE_FR),
	[RSC_DDR3_REFI_COMM]	= LDT(	RSC_DDR3_REFI_COMM_CODE_EN,
					RSC_DDR3_REFI_COMM_CODE_FR),
	[RSC_DDR3_DDWRTRD_COMM] = LDT(	RSC_DDR3_DDWRTRD_COMM_CODE_EN,
					RSC_DDR3_DDWRTRD_COMM_CODE_FR),
	[RSC_DDR3_DRWRTRD_COMM] = LDT(	RSC_DDR3_DRWRTRD_COMM_CODE_EN,
					RSC_DDR3_DRWRTRD_COMM_CODE_FR),
	[RSC_DDR3_SRWRTRD_COMM] = LDT(	RSC_DDR3_SRWRTRD_COMM_CODE_EN,
					RSC_DDR3_SRWRTRD_COMM_CODE_FR),
	[RSC_DDR3_DDRDTWR_COMM] = LDT(	RSC_DDR3_DDRDTWR_COMM_CODE_EN,
					RSC_DDR3_DDRDTWR_COMM_CODE_FR),
	[RSC_DDR3_DRRDTWR_COMM] = LDT(	RSC_DDR3_DRRDTWR_COMM_CODE_EN,
					RSC_DDR3_DRRDTWR_COMM_CODE_FR),
	[RSC_DDR3_SRRDTWR_COMM] = LDT(	RSC_DDR3_SRRDTWR_COMM_CODE_EN,
					RSC_DDR3_SRRDTWR_COMM_CODE_FR),
	[RSC_DDR3_DDRDTRD_COMM] = LDT(	RSC_DDR3_DDRDTRD_COMM_CODE_EN,
					RSC_DDR3_DDRDTRD_COMM_CODE_FR),
	[RSC_DDR3_DRRDTRD_COMM] = LDT(	RSC_DDR3_DRRDTRD_COMM_CODE_EN,
					RSC_DDR3_DRRDTRD_COMM_CODE_FR),
	[RSC_DDR3_SRRDTRD_COMM] = LDT(	RSC_DDR3_SRRDTRD_COMM_CODE_EN,
					RSC_DDR3_SRRDTRD_COMM_CODE_FR),
	[RSC_DDR3_DDWRTWR_COMM] = LDT(	RSC_DDR3_DDWRTWR_COMM_CODE_EN,
					RSC_DDR3_DDWRTWR_COMM_CODE_FR),
	[RSC_DDR3_DRWRTWR_COMM] = LDT(	RSC_DDR3_DRWRTWR_COMM_CODE_EN,
					RSC_DDR3_DRWRTWR_COMM_CODE_FR),
	[RSC_DDR3_SRWRTWR_COMM] = LDT(	RSC_DDR3_SRWRTWR_COMM_CODE_EN,
					RSC_DDR3_SRWRTWR_COMM_CODE_FR),
	[RSC_DDR3_CKE_COMM]	= LDT(	RSC_DDR3_CKE_COMM_CODE_EN,
					RSC_DDR3_CKE_COMM_CODE_FR),
	[RSC_DDR3_ECC_COMM]	= LDT(	RSC_DDR3_ECC_COMM_CODE_EN,
					RSC_DDR3_ECC_COMM_CODE_FR),
	[RSC_DDR4_RDRD_SCL_COMM]= LDT(	RSC_DDR4_RDRD_SCL_COMM_CODE_EN,
					RSC_DDR4_RDRD_SCL_COMM_CODE_FR),
	[RSC_DDR4_RDRD_SC_COMM] = LDT(	RSC_DDR4_RDRD_SC_COMM_CODE_EN,
					RSC_DDR4_RDRD_SC_COMM_CODE_FR),
	[RSC_DDR4_RDRD_SD_COMM] = LDT(	RSC_DDR4_RDRD_SD_COMM_CODE_EN,
					RSC_DDR4_RDRD_SD_COMM_CODE_FR),
	[RSC_DDR4_RDRD_DD_COMM] = LDT(	RSC_DDR4_RDRD_DD_COMM_CODE_EN,
					RSC_DDR4_RDRD_DD_COMM_CODE_FR),
	[RSC_DDR4_RDWR_SCL_COMM]= LDT(	RSC_DDR4_RDWR_SCL_COMM_CODE_EN,
					RSC_DDR4_RDWR_SCL_COMM_CODE_FR),
	[RSC_DDR4_RDWR_SC_COMM] = LDT(	RSC_DDR4_RDWR_SC_COMM_CODE_EN,
					RSC_DDR4_RDWR_SC_COMM_CODE_FR),
	[RSC_DDR4_RDWR_SD_COMM] = LDT(	RSC_DDR4_RDWR_SD_COMM_CODE_EN,
					RSC_DDR4_RDWR_SD_COMM_CODE_FR),
	[RSC_DDR4_RDWR_DD_COMM] = LDT(	RSC_DDR4_RDWR_DD_COMM_CODE_EN,
					RSC_DDR4_RDWR_DD_COMM_CODE_FR),
	[RSC_DDR4_WRRD_SCL_COMM]= LDT(	RSC_DDR4_WRRD_SCL_COMM_CODE_EN,
					RSC_DDR4_WRRD_SCL_COMM_CODE_FR),
	[RSC_DDR4_WRRD_SC_COMM] = LDT(	RSC_DDR4_WRRD_SC_COMM_CODE_EN,
					RSC_DDR4_WRRD_SC_COMM_CODE_FR),
	[RSC_DDR4_WRRD_SD_COMM] = LDT(	RSC_DDR4_WRRD_SD_COMM_CODE_EN,
					RSC_DDR4_WRRD_SD_COMM_CODE_FR),
	[RSC_DDR4_WRRD_DD_COMM] = LDT(	RSC_DDR4_WRRD_DD_COMM_CODE_EN,
					RSC_DDR4_WRRD_DD_COMM_CODE_FR),
	[RSC_DDR4_WRWR_SCL_COMM]= LDT(	RSC_DDR4_WRWR_SCL_COMM_CODE_EN,
					RSC_DDR4_WRWR_SCL_COMM_CODE_FR),
	[RSC_DDR4_WRWR_SC_COMM] = LDT(	RSC_DDR4_WRWR_SC_COMM_CODE_EN,
					RSC_DDR4_WRWR_SC_COMM_CODE_FR),
	[RSC_DDR4_WRWR_SD_COMM] = LDT(	RSC_DDR4_WRWR_SD_COMM_CODE_EN,
					RSC_DDR4_WRWR_SD_COMM_CODE_FR),
	[RSC_DDR4_WRWR_DD_COMM] = LDT(	RSC_DDR4_WRWR_DD_COMM_CODE_EN,
					RSC_DDR4_WRWR_DD_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RCD_R_COMM]=LDT(	RSC_DDR4_ZEN_RCD_R_COMM_CODE_EN,
					RSC_DDR4_ZEN_RCD_R_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RCD_W_COMM]=LDT(	RSC_DDR4_ZEN_RCD_W_COMM_CODE_EN,
					RSC_DDR4_ZEN_RCD_W_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RC_COMM]	= LDT(	RSC_DDR4_ZEN_RC_COMM_CODE_EN,
					RSC_DDR4_ZEN_RC_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RRD_S_COMM]=LDT(	RSC_DDR4_ZEN_RRD_S_COMM_CODE_EN,
					RSC_DDR4_ZEN_RRD_S_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RRD_L_COMM]=LDT(	RSC_DDR4_ZEN_RRD_L_COMM_CODE_EN,
					RSC_DDR4_ZEN_RRD_L_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WTR_S_COMM]=LDT(	RSC_DDR4_ZEN_WTR_S_COMM_CODE_EN,
					RSC_DDR4_ZEN_WTR_S_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WTR_L_COMM]=LDT(	RSC_DDR4_ZEN_WTR_L_COMM_CODE_EN,
					RSC_DDR4_ZEN_WTR_L_COMM_CODE_FR),
      [RSC_DDR4_ZEN_RDRD_SCL_COMM]=LDT( RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_EN,
					RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_FR),
      [RSC_DDR4_ZEN_WRWR_SCL_COMM]=LDT( RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_EN,
					RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RTP_COMM] = LDT(	RSC_DDR4_ZEN_RTP_COMM_CODE_EN,
					RSC_DDR4_ZEN_RTP_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RDWR_COMM]= LDT(	RSC_DDR4_ZEN_RDWR_COMM_CODE_EN,
					RSC_DDR4_ZEN_RDWR_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WRRD_COMM]= LDT(	RSC_DDR4_ZEN_WRRD_COMM_CODE_EN,
					RSC_DDR4_ZEN_WRRD_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WRWR_SC_COMM]=LDT(RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_EN,
					RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WRWR_SD_COMM]=LDT(RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_EN,
					RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WRWR_DD_COMM]=LDT(RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_EN,
					RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RDRD_SC_COMM]=LDT(RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_EN,
					RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RDRD_SD_COMM]=LDT(RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_EN,
					RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RDRD_DD_COMM]=LDT(RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_EN,
					RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RTR_DLR_COMM]=LDT(RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_EN,
					RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WTW_DLR_COMM]=LDT(RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_EN,
					RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_FR),
	[RSC_DDR4_ZEN_WTR_DLR_COMM]=LDT(RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_EN,
					RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RRD_DLR_COMM]=LDT(RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_EN,
					RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RFC1_COMM] = LDT( RSC_DDR4_ZEN_RFC1_COMM_CODE_EN,
					RSC_DDR4_ZEN_RFC1_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RFC2_COMM] = LDT( RSC_DDR4_ZEN_RFC2_COMM_CODE_EN,
					RSC_DDR4_ZEN_RFC2_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RFC4_COMM] = LDT( RSC_DDR4_ZEN_RFC4_COMM_CODE_EN,
					RSC_DDR4_ZEN_RFC4_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RCPB_COMM] = LDT( RSC_DDR4_ZEN_RCPB_COMM_CODE_EN,
					RSC_DDR4_ZEN_RCPB_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RPPB_COMM] = LDT( RSC_DDR4_ZEN_RPPB_COMM_CODE_EN,
					RSC_DDR4_ZEN_RPPB_COMM_CODE_FR),
	[RSC_DDR4_ZEN_BGS_COMM] = LDT(	RSC_DDR4_ZEN_BGS_COMM_CODE_EN,
					RSC_DDR4_ZEN_BGS_COMM_CODE_FR),
	[RSC_DDR4_ZEN_BGS_ALT_COMM]=LDT(RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_EN,
					RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_FR),
	[RSC_DDR4_ZEN_BAN_COMM] = LDT(	RSC_DDR4_ZEN_BAN_COMM_CODE_EN,
					RSC_DDR4_ZEN_BAN_COMM_CODE_FR),
	[RSC_DDR4_ZEN_RCPAGE_COMM]=LDT( RSC_DDR4_ZEN_RCPAGE_COMM_CODE_EN,
					RSC_DDR4_ZEN_RCPAGE_COMM_CODE_FR),
	[RSC_DDR4_ZEN_GDM_COMM] = LDT(	RSC_DDR4_ZEN_GDM_COMM_CODE_EN,
					RSC_DDR4_ZEN_GDM_COMM_CODE_FR),
	[RSC_TASKS_SORTBY_STATE]= LDT(	RSC_TASKS_SORTBY_STATE_CODE_EN,
					RSC_TASKS_SORTBY_STATE_CODE_FR),
	[RSC_TASKS_SORTBY_RTIME]= LDT(	RSC_TASKS_SORTBY_RTIME_CODE_EN,
					RSC_TASKS_SORTBY_RTIME_CODE_FR),
	[RSC_TASKS_SORTBY_UTIME]= LDT(	RSC_TASKS_SORTBY_UTIME_CODE_EN,
					RSC_TASKS_SORTBY_UTIME_CODE_FR),
	[RSC_TASKS_SORTBY_STIME]= LDT(	RSC_TASKS_SORTBY_STIME_CODE_EN,
					RSC_TASKS_SORTBY_STIME_CODE_FR),
	[RSC_TASKS_SORTBY_PID]	= LDT(	RSC_TASKS_SORTBY_PID_CODE_EN,
					RSC_TASKS_SORTBY_PID_CODE_FR),
	[RSC_TASKS_SORTBY_COMM] = LDT(	RSC_TASKS_SORTBY_COMM_CODE_EN,
					RSC_TASKS_SORTBY_COMM_CODE_FR),
	[RSC_MENU_ITEM_MENU]	= LDS(	Rsc_CreateMenu_Menu_Attr,
					RSC_MENU_ITEM_MENU_CODE_EN,
					RSC_MENU_ITEM_MENU_CODE_FR),
	[RSC_MENU_ITEM_VIEW]	= LDS(	Rsc_CreateMenu_View_Attr,
					RSC_MENU_ITEM_VIEW_CODE_EN,
					RSC_MENU_ITEM_VIEW_CODE_FR),
	[RSC_MENU_ITEM_WINDOW]	= LDS(	Rsc_CreateMenu_Window_Attr,
					RSC_MENU_ITEM_WINDOW_CODE_EN,
					RSC_MENU_ITEM_WINDOW_CODE_FR),
	[RSC_MENU_ITEM_SETTINGS]= LDT(	RSC_MENU_ITEM_SETTINGS_CODE_EN,
					RSC_MENU_ITEM_SETTINGS_CODE_FR),
	[RSC_MENU_ITEM_SMBIOS]	= LDT(	RSC_MENU_ITEM_SMBIOS_CODE_EN,
					RSC_MENU_ITEM_SMBIOS_CODE_FR),
	[RSC_MENU_ITEM_KERNEL]	= LDT(	RSC_MENU_ITEM_KERNEL_CODE_EN,
					RSC_MENU_ITEM_KERNEL_CODE_FR),
	[RSC_MENU_ITEM_HOTPLUG] = LDT(	RSC_MENU_ITEM_HOTPLUG_CODE_EN,
					RSC_MENU_ITEM_HOTPLUG_CODE_FR),
	[RSC_MENU_ITEM_TOOLS]	= LDT(	RSC_MENU_ITEM_TOOLS_CODE_EN,
					RSC_MENU_ITEM_TOOLS_CODE_FR),
	[RSC_MENU_ITEM_ABOUT]	= LDT(	RSC_MENU_ITEM_ABOUT_CODE_EN,
					RSC_MENU_ITEM_ABOUT_CODE_FR),
	[RSC_MENU_ITEM_HELP]	= LDT(	RSC_MENU_ITEM_HELP_CODE_EN,
					RSC_MENU_ITEM_HELP_CODE_FR),
	[RSC_MENU_ITEM_KEYS]	= LDT(	RSC_MENU_ITEM_KEYS_CODE_EN,
					RSC_MENU_ITEM_KEYS_CODE_FR),
	[RSC_MENU_ITEM_LANG]	= LDT(	RSC_MENU_ITEM_LANG_CODE_EN,
					RSC_MENU_ITEM_LANG_CODE_FR),
	[RSC_MENU_ITEM_QUIT]	= LDT(	RSC_MENU_ITEM_QUIT_CODE_EN,
					RSC_MENU_ITEM_QUIT_CODE_FR),
	[RSC_MENU_ITEM_DASHBOARD]=LDT(	RSC_MENU_ITEM_DASHBOARD_CODE_EN,
					RSC_MENU_ITEM_DASHBOARD_CODE_FR),
	[RSC_MENU_ITEM_FREQUENCY]=LDT(	RSC_MENU_ITEM_FREQUENCY_CODE_EN,
					RSC_MENU_ITEM_FREQUENCY_CODE_FR),
	[RSC_MENU_ITEM_INST_CYCLES]=LDT(RSC_MENU_ITEM_INST_CYCLE_CODE_EN,
					RSC_MENU_ITEM_INST_CYCLE_CODE_FR),
	[RSC_MENU_ITEM_CORE_CYCLES]=LDT(RSC_MENU_ITEM_CORE_CYCLE_CODE_EN,
					RSC_MENU_ITEM_CORE_CYCLE_CODE_FR),
	[RSC_MENU_ITEM_IDLE_STATES]=LDT(RSC_MENU_ITEM_IDLE_STATE_CODE_EN,
					RSC_MENU_ITEM_IDLE_STATE_CODE_FR),
	[RSC_MENU_ITEM_PKG_CYCLES]=LDT( RSC_MENU_ITEM_PKG_CYCLE_CODE_EN,
					RSC_MENU_ITEM_PKG_CYCLE_CODE_FR),
	[RSC_MENU_ITEM_TASKS_MON]=LDT(	RSC_MENU_ITEM_TASKS_MON_CODE_EN,
					RSC_MENU_ITEM_TASKS_MON_CODE_FR),
	[RSC_MENU_ITEM_SYS_INTER]=LDT(	RSC_MENU_ITEM_SYS_INTER_CODE_EN,
					RSC_MENU_ITEM_SYS_INTER_CODE_FR),
	[RSC_MENU_ITEM_SENSORS] = LDT(	RSC_MENU_ITEM_SENSORS_CODE_EN,
					RSC_MENU_ITEM_SENSORS_CODE_FR),
	[RSC_MENU_ITEM_VOLTAGE] = LDT(	RSC_MENU_ITEM_VOLTAGE_CODE_EN,
					RSC_MENU_ITEM_VOLTAGE_CODE_FR),
	[RSC_MENU_ITEM_POWER] = LDT(	RSC_MENU_ITEM_POWER_CODE_EN,
					RSC_MENU_ITEM_POWER_CODE_FR),
	[RSC_MENU_ITEM_SLICE_CTRS]=LDT( RSC_MENU_ITEM_SLICE_CTR_CODE_EN,
					RSC_MENU_ITEM_SLICE_CTR_CODE_FR),
	[RSC_MENU_ITEM_PROCESSOR]=LDT(	RSC_MENU_ITEM_PROCESSOR_CODE_EN,
					RSC_MENU_ITEM_PROCESSOR_CODE_FR),
	[RSC_MENU_ITEM_TOPOLOGY]= LDT(	RSC_MENU_ITEM_TOPOLOGY_CODE_EN,
					RSC_MENU_ITEM_TOPOLOGY_CODE_FR),
	[RSC_MENU_ITEM_FEATURES]= LDT(	RSC_MENU_ITEM_FEATURES_CODE_EN,
					RSC_MENU_ITEM_FEATURES_CODE_FR),
	[RSC_MENU_ITEM_ISA_EXT] = LDT(	RSC_MENU_ITEM_ISA_EXT_CODE_EN,
					RSC_MENU_ITEM_ISA_EXT_CODE_FR),
	[RSC_MENU_ITEM_TECH]	= LDT(	RSC_MENU_ITEM_TECH_CODE_EN,
					RSC_MENU_ITEM_TECH_CODE_FR),
	[RSC_MENU_ITEM_PERF_MON]= LDT(	RSC_MENU_ITEM_PERF_MON_CODE_EN,
					RSC_MENU_ITEM_PERF_MON_CODE_FR),
	[RSC_MENU_ITEM_POW_THERM]=LDT(	RSC_MENU_ITEM_POW_THERM_CODE_EN,
					RSC_MENU_ITEM_POW_THERM_CODE_FR),
	[RSC_MENU_ITEM_CPUID]	= LDT(	RSC_MENU_ITEM_CPUID_CODE_EN,
					RSC_MENU_ITEM_CPUID_CODE_FR),
	[RSC_MENU_ITEM_SYS_REGS]= LDT(	RSC_MENU_ITEM_SYS_REGS_CODE_EN,
					RSC_MENU_ITEM_SYS_REGS_CODE_FR),
	[RSC_MENU_ITEM_MEM_CTRL]= LDT(	RSC_MENU_ITEM_MEM_CTRL_CODE_EN,
					RSC_MENU_ITEM_MEM_CTRL_CODE_FR),
	[RSC_SETTINGS_TITLE]	= LDT(	RSC_SETTINGS_TITLE_CODE_EN,
					RSC_SETTINGS_TITLE_CODE_FR),
	[RSC_SETTINGS_DAEMON]	= LDT(	RSC_SETTINGS_DAEMON_CODE_EN,
					RSC_SETTINGS_DAEMON_CODE_FR),
	[RSC_SETTINGS_INTERVAL] = LDT(	RSC_SETTINGS_INTERVAL_CODE_EN,
					RSC_SETTINGS_INTERVAL_CODE_FR),
	[RSC_SETTINGS_SYS_TICK] = LDT(	RSC_SETTINGS_SYS_TICK_CODE_EN,
					RSC_SETTINGS_SYS_TICK_CODE_FR),
	[RSC_SETTINGS_POLL_WAIT] = LDT( RSC_SETTINGS_POLL_WAIT_CODE_EN,
					RSC_SETTINGS_POLL_WAIT_CODE_FR),
	[RSC_SETTINGS_RING_WAIT] = LDT( RSC_SETTINGS_RING_WAIT_CODE_EN,
					RSC_SETTINGS_RING_WAIT_CODE_FR),
	[RSC_SETTINGS_CHILD_WAIT]= LDT( RSC_SETTINGS_CHILD_WAIT_CODE_EN,
					RSC_SETTINGS_CHILD_WAIT_CODE_FR),
	[RSC_SETTINGS_SLICE_WAIT]= LDT( RSC_SETTINGS_SLICE_WAIT_CODE_EN,
					RSC_SETTINGS_SLICE_WAIT_CODE_FR),
	[RSC_SETTINGS_RECORDER] = LDT(	RSC_SETTINGS_RECORDER_CODE_EN,
					RSC_SETTINGS_RECORDER_CODE_FR),
	[RSC_SETTINGS_AUTO_CLOCK]=LDT(	RSC_SETTINGS_AUTO_CLOCK_CODE_EN,
					RSC_SETTINGS_AUTO_CLOCK_CODE_FR),
	[RSC_SETTINGS_EXPERIMENTAL]=LDT(RSC_SETTINGS_EXPERIMENTAL_CODE_EN,
					RSC_SETTINGS_EXPERIMENTAL_CODE_FR),
	[RSC_SETTINGS_CPU_HOTPLUG]=LDT( RSC_SETTINGS_CPU_HOTPLUG_CODE_EN,
					RSC_SETTINGS_CPU_HOTPLUG_CODE_FR),
	[RSC_SETTINGS_PCI_ENABLED]=LDT( RSC_SETTINGS_PCI_ENABLED_CODE_EN,
					RSC_SETTINGS_PCI_ENABLED_CODE_FR),
    [RSC_SETTINGS_NMI_REGISTERED]=LDT(	RSC_SETTINGS_NMI_REGISTERED_CODE_EN,
					RSC_SETTINGS_NMI_REGISTERED_CODE_FR),
  [RSC_SETTINGS_CPUIDLE_REGISTERED]=LDT(RSC_SETTINGS_CPUIDLE_REGISTER_CODE_EN,
					RSC_SETTINGS_CPUIDLE_REGISTER_CODE_FR),
  [RSC_SETTINGS_CPUFREQ_REGISTERED]=LDT(RSC_SETTINGS_CPUFREQ_REGISTER_CODE_EN,
					RSC_SETTINGS_CPUFREQ_REGISTER_CODE_FR),
 [RSC_SETTINGS_GOVERNOR_REGISTERED]=LDT(RSC_SETTINGS_GOVERNOR_CPUFREQ_CODE_EN,
					RSC_SETTINGS_GOVERNOR_CPUFREQ_CODE_FR),
     [RSC_SETTINGS_CS_REGISTERED] = LDT(RSC_SETTINGS_CLOCK_SOURCE_CODE_EN,
					RSC_SETTINGS_CLOCK_SOURCE_CODE_FR),
    [RSC_SETTINGS_THERMAL_SCOPE] = LDT( RSC_SETTINGS_THERMAL_SCOPE_CODE_EN,
					RSC_SETTINGS_THERMAL_SCOPE_CODE_FR),
    [RSC_SETTINGS_VOLTAGE_SCOPE] = LDT( RSC_SETTINGS_VOLTAGE_SCOPE_CODE_EN,
					RSC_SETTINGS_VOLTAGE_SCOPE_CODE_FR),
	[RSC_SETTINGS_POWER_SCOPE]=LDT( RSC_SETTINGS_POWER_SCOPE_CODE_EN,
					RSC_SETTINGS_POWER_SCOPE_CODE_FR),
	[RSC_SETTINGS_IDLE_ROUTE]=LDT(	RSC_SETTINGS_IDLE_ROUTE_CODE_EN,
					RSC_SETTINGS_IDLE_ROUTE_CODE_FR),
	[RSC_SETTINGS_ROUTE_DFLT]=LDQ(	RSC_SETTINGS_ROUTE_DEFAULT_CODE),
	[RSC_SETTINGS_ROUTE_IO] = LDQ(	RSC_SETTINGS_ROUTE_IO_CODE),
	[RSC_SETTINGS_ROUTE_HALT]=LDQ(	RSC_SETTINGS_ROUTE_HALT_CODE),
	[RSC_SETTINGS_ROUTE_MWAIT]=LDQ( RSC_SETTINGS_ROUTE_MWAIT_CODE),
	[RSC_HELP_TITLE]	= LDT(	RSC_HELP_TITLE_CODE_EN,
					RSC_HELP_TITLE_CODE_FR),
	[RSC_HELP_KEY_ESCAPE]	= LDT(	RSC_HELP_KEY_ESCAPE_CODE_EN,
					RSC_HELP_KEY_ESCAPE_CODE_FR),
	[RSC_HELP_KEY_SHIFT_TAB]= LDT(	RSC_HELP_KEY_SHIFT_TAB_CODE_EN,
					RSC_HELP_KEY_SHIFT_TAB_CODE_FR),
	[RSC_HELP_KEY_TAB]	= LDT(	RSC_HELP_KEY_TAB_CODE_EN,
					RSC_HELP_KEY_TAB_CODE_FR),
	[RSC_HELP_KEY_UP]	= LDT(	RSC_HELP_KEY_UP_CODE_EN,
					RSC_HELP_KEY_UP_CODE_FR),
	[RSC_HELP_KEY_LEFT_RIGHT]=LDT(	RSC_HELP_KEY_LEFT_RIGHT_CODE_EN,
					RSC_HELP_KEY_LEFT_RIGHT_CODE_FR),
	[RSC_HELP_KEY_DOWN]	= LDT(	RSC_HELP_KEY_DOWN_CODE_EN,
					RSC_HELP_KEY_DOWN_CODE_FR),
	[RSC_HELP_KEY_END]	= LDT(	RSC_HELP_KEY_END_CODE_EN,
					RSC_HELP_KEY_END_CODE_FR),
	[RSC_HELP_KEY_HOME]	= LDT(	RSC_HELP_KEY_HOME_CODE_EN,
					RSC_HELP_KEY_HOME_CODE_FR),
	[RSC_HELP_KEY_ENTER]	= LDT(	RSC_HELP_KEY_ENTER_CODE_EN,
					RSC_HELP_KEY_ENTER_CODE_FR),
	[RSC_HELP_KEY_PAGE_UP]	= LDT(	RSC_HELP_KEY_PAGE_UP_CODE_EN,
					RSC_HELP_KEY_PAGE_UP_CODE_FR),
	[RSC_HELP_KEY_PAGE_DOWN]= LDT(	RSC_HELP_KEY_PAGE_DOWN_CODE_EN,
					RSC_HELP_KEY_PAGE_DOWN_CODE_FR),
	[RSC_HELP_KEY_MINUS]	= LDT(	RSC_HELP_KEY_MINUS_CODE_EN,
					RSC_HELP_KEY_MINUS_CODE_FR),
	[RSC_HELP_KEY_PLUS]	= LDT(	RSC_HELP_KEY_PLUS_CODE_EN,
					RSC_HELP_KEY_PLUS_CODE_FR),
	[RSC_HELP_BLANK]	= LDQ(	RSC_CREATE_HELP_BLANK_CODE),
	[RSC_HELP_KEY_MENU]	= LDT(	RSC_HELP_KEY_MENU_CODE_EN,
					RSC_HELP_KEY_MENU_CODE_FR),
	[RSC_HELP_MENU] 	= LDT(	RSC_HELP_MENU_CODE_EN,
					RSC_HELP_MENU_CODE_FR),
	[RSC_HELP_CLOSE_WINDOW] = LDT(	RSC_HELP_CLOSE_WINDOW_CODE_EN,
					RSC_HELP_CLOSE_WINDOW_CODE_FR),
	[RSC_HELP_PREV_WINDOW]	= LDT(	RSC_HELP_PREV_WINDOW_CODE_EN,
					RSC_HELP_PREV_WINDOW_CODE_FR),
	[RSC_HELP_NEXT_WINDOW]	= LDT(	RSC_HELP_NEXT_WINDOW_CODE_EN,
					RSC_HELP_NEXT_WINDOW_CODE_FR),
	[RSC_HELP_KEY_SHIFT_GR1]= LDT(	RSC_HELP_KEY_SHIFT_GR1_CODE_EN,
					RSC_HELP_KEY_SHIFT_GR1_CODE_FR),
	[RSC_HELP_KEY_SHIFT_GR2]= LDT(	RSC_HELP_KEY_SHIFT_GR2_CODE_EN,
					RSC_HELP_KEY_SHIFT_GR2_CODE_FR),
	[RSC_HELP_MOVE_WINDOW]	= LDT(	RSC_HELP_MOVE_WINDOW_CODE_EN,
					RSC_HELP_MOVE_WINDOW_CODE_FR),
	[RSC_HELP_KEY_ALT_GR3]	= LDT(	RSC_HELP_KEY_ALT_GR3_CODE_EN,
					RSC_HELP_KEY_ALT_GR3_CODE_FR),
	[RSC_HELP_SIZE_WINDOW]	= LDT(	RSC_HELP_SIZE_WINDOW_CODE_EN,
					RSC_HELP_SIZE_WINDOW_CODE_FR),
	[RSC_HELP_MOVE_SELECT]	= LDT(	RSC_HELP_MOVE_SELECT_CODE_EN,
					RSC_HELP_MOVE_SELECT_CODE_FR),
	[RSC_HELP_LAST_CELL]	= LDT(	RSC_HELP_LAST_CELL_CODE_EN,
					RSC_HELP_LAST_CELL_CODE_FR),
	[RSC_HELP_FIRST_CELL]	= LDT(	RSC_HELP_FIRST_CELL_CODE_EN,
					RSC_HELP_FIRST_CELL_CODE_FR),
	[RSC_HELP_TRIGGER_SELECT]=LDT(	RSC_HELP_TRIGGER_SELECT_CODE_EN,
					RSC_HELP_TRIGGER_SELECT_CODE_FR),
	[RSC_HELP_PREV_PAGE]	= LDT(	RSC_HELP_PREV_PAGE_CODE_EN,
					RSC_HELP_PREV_PAGE_CODE_FR),
	[RSC_HELP_NEXT_PAGE]	= LDT(	RSC_HELP_NEXT_PAGE_CODE_EN,
					RSC_HELP_NEXT_PAGE_CODE_FR),
	[RSC_HELP_SCROLL_DOWN]	= LDT(	RSC_HELP_SCROLL_DOWN_CODE_EN,
					RSC_HELP_SCROLL_DOWN_CODE_FR),
	[RSC_HELP_SCROLL_UP]	= LDT(	RSC_HELP_SCROLL_UP_CODE_EN,
					RSC_HELP_SCROLL_UP_CODE_FR),
	[RSC_ADV_HELP_TITLE]	= LDT(	RSC_ADV_HELP_TITLE_CODE_EN,
					RSC_ADV_HELP_TITLE_CODE_FR),
	[RSC_ADV_HELP_SECT_FREQ] = LDT( RSC_ADV_HELP_SECT_FREQ_CODE_EN,
					RSC_ADV_HELP_SECT_FREQ_CODE_FR),
	[RSC_ADV_HELP_ITEM_AVG] = LDT(	RSC_ADV_HELP_ITEM_AVG_CODE_EN,
					RSC_ADV_HELP_ITEM_AVG_CODE_FR),
	[RSC_ADV_HELP_SECT_TASK] = LDT( RSC_ADV_HELP_SECT_TASK_CODE_EN,
					RSC_ADV_HELP_SECT_TASK_CODE_FR),
	[RSC_ADV_HELP_ITEM_ORDER] = LDT(RSC_ADV_HELP_ITEM_ORDER_CODE_EN,
					RSC_ADV_HELP_ITEM_ORDER_CODE_FR),
	[RSC_ADV_HELP_ITEM_SEL]	= LDT(	RSC_ADV_HELP_ITEM_SEL_CODE_EN,
					RSC_ADV_HELP_ITEM_SEL_CODE_FR),
	[RSC_ADV_HELP_ITEM_REV]	= LDT(	RSC_ADV_HELP_ITEM_REV_CODE_EN,
					RSC_ADV_HELP_ITEM_REV_CODE_FR),
	[RSC_ADV_HELP_ITEM_HIDE] = LDT( RSC_ADV_HELP_ITEM_HIDE_CODE_EN,
					RSC_ADV_HELP_ITEM_HIDE_CODE_FR),
	[RSC_ADV_HELP_SECT_ANY] = LDT(	RSC_ADV_HELP_SECT_ANY_CODE_EN,
					RSC_ADV_HELP_SECT_ANY_CODE_FR),
	[RSC_ADV_HELP_ITEM_POWER] = LDT(RSC_ADV_HELP_ITEM_POWER_CODE_EN,
					RSC_ADV_HELP_ITEM_POWER_CODE_FR),
	[RSC_ADV_HELP_ITEM_TOP]	= LDT(	RSC_ADV_HELP_ITEM_TOP_CODE_EN,
					RSC_ADV_HELP_ITEM_TOP_CODE_FR),
	[RSC_ADV_HELP_ITEM_UPD] = LDT(	RSC_ADV_HELP_ITEM_UPD_CODE_EN,
					RSC_ADV_HELP_ITEM_UPD_CODE_FR),
	[RSC_ADV_HELP_ITEM_START] = LDT(RSC_ADV_HELP_ITEM_START_CODE_EN,
					RSC_ADV_HELP_ITEM_START_CODE_FR),
	[RSC_ADV_HELP_ITEM_STOP] = LDT( RSC_ADV_HELP_ITEM_STOP_CODE_EN,
					RSC_ADV_HELP_ITEM_STOP_CODE_FR),
	[RSC_ADV_HELP_ITEM_TOOLS] = LDT(RSC_ADV_HELP_ITEM_TOOLS_CODE_EN,
					RSC_ADV_HELP_ITEM_TOOLS_CODE_FR),
	[RSC_ADV_HELP_ITEM_GO_UP] = LDT(RSC_ADV_HELP_ITEM_GO_UP_CODE_EN,
					RSC_ADV_HELP_ITEM_GO_UP_CODE_FR),
	[RSC_ADV_HELP_ITEM_GO_DW] = LDT(RSC_ADV_HELP_ITEM_GO_DW_CODE_EN,
					RSC_ADV_HELP_ITEM_GO_DW_CODE_FR),
       [RSC_ADV_HELP_ITEM_TERMINAL]=LDT(RSC_ADV_HELP_TERMINAL_CODE_EN,
					RSC_ADV_HELP_TERMINAL_CODE_FR),
	[RSC_ADV_HELP_ITEM_PRT_SCR]=LDT(RSC_ADV_HELP_PRT_SCR_CODE_EN,
					RSC_ADV_HELP_PRT_SCR_CODE_FR),
	[RSC_ADV_HELP_ITEM_REC_SCR]=LDT(RSC_ADV_HELP_REC_SCR_CODE_EN,
					RSC_ADV_HELP_REC_SCR_CODE_FR),
      [RSC_ADV_HELP_ITEM_FAHR_CELS]=LDT(RSC_ADV_HELP_FAHR_CELS_CODE_EN,
					RSC_ADV_HELP_FAHR_CELS_CODE_FR),
     [RSC_ADV_HELP_ITEM_PROC_EVENT]=LDT(RSC_ADV_HELP_PROC_EVENT_CODE_EN,
					RSC_ADV_HELP_PROC_EVENT_CODE_FR),
	[RSC_ADV_HELP_ITEM_SECRET]= LDT(RSC_ADV_HELP_SECRET_CODE_EN,
					RSC_ADV_HELP_SECRET_CODE_FR),
	[RSC_TURBO_CLOCK_TITLE] = LDT(	RSC_TURBO_CLOCK_TITLE_CODE_EN,
					RSC_TURBO_CLOCK_TITLE_CODE_FR),
	[RSC_RATIO_CLOCK_TITLE] = LDT(	RSC_RATIO_CLOCK_TITLE_CODE_EN,
					RSC_RATIO_CLOCK_TITLE_CODE_FR),
	[RSC_UNCORE_CLOCK_TITLE]= LDT(	RSC_UNCORE_CLOCK_TITLE_CODE_EN,
					RSC_UNCORE_CLOCK_TITLE_CODE_FR),
	[RSC_SELECT_CPU_TITLE]	= LDT(	RSC_SELECT_CPU_TITLE_CODE_EN,
					RSC_SELECT_CPU_TITLE_CODE_FR),
	[RSC_SELECT_FREQ_TITLE] = LDT(	RSC_SELECT_FREQ_TITLE_CODE_EN,
					RSC_SELECT_FREQ_TITLE_CODE_FR),
	[RSC_BOX_DISABLE_COND0] = LDT(	RSC_BOX_DISABLE_COND0_CODE_EN,
					RSC_BOX_DISABLE_COND0_CODE_FR),
	[RSC_BOX_DISABLE_COND1] = LDT(	RSC_BOX_DISABLE_COND1_CODE_EN,
					RSC_BOX_DISABLE_COND1_CODE_FR),
	[RSC_BOX_ENABLE_COND0]	= LDT(	RSC_BOX_ENABLE_COND0_CODE_EN,
					RSC_BOX_ENABLE_COND0_CODE_FR),
	[RSC_BOX_ENABLE_COND1]	= LDT(	RSC_BOX_ENABLE_COND1_CODE_EN,
					RSC_BOX_ENABLE_COND1_CODE_FR),
	[RSC_BOX_INTERVAL_TITLE]= LDT(	RSC_BOX_INTERVAL_TITLE_CODE_EN,
					RSC_BOX_INTERVAL_TITLE_CODE_FR),
	[RSC_BOX_INTERVAL_STEP1] = LDQ( RSC_BOX_INTERVAL_STEP1_CODE),
	[RSC_BOX_INTERVAL_STEP2] = LDQ( RSC_BOX_INTERVAL_STEP2_CODE),
	[RSC_BOX_INTERVAL_STEP3] = LDQ( RSC_BOX_INTERVAL_STEP3_CODE),
	[RSC_BOX_INTERVAL_STEP4] = LDQ( RSC_BOX_INTERVAL_STEP4_CODE),
	[RSC_BOX_INTERVAL_STEP5] = LDQ( RSC_BOX_INTERVAL_STEP5_CODE),
	[RSC_BOX_INTERVAL_STEP6] = LDQ( RSC_BOX_INTERVAL_STEP6_CODE),
	[RSC_BOX_INTERVAL_STEP7] = LDQ( RSC_BOX_INTERVAL_STEP7_CODE),
	[RSC_BOX_INTERVAL_STEP8] = LDQ( RSC_BOX_INTERVAL_STEP8_CODE),
	[RSC_BOX_INTERVAL_STEP9] = LDQ( RSC_BOX_INTERVAL_STEP9_CODE),
	[RSC_BOX_INTERVAL_STEP10]= LDQ( RSC_BOX_INTERVAL_STEP10_CODE),
	[RSC_BOX_AUTO_CLOCK_TITLE]=LDT( RSC_BOX_AUTOCLOCK_TITLE_CODE_EN,
					RSC_BOX_AUTOCLOCK_TITLE_CODE_FR),
	[RSC_BOX_MODE_TITLE]	= LDT(	RSC_BOX_MODE_TITLE_CODE_EN,
					RSC_BOX_MODE_TITLE_CODE_FR),
	[RSC_BOX_MODE_DESC]	= LDT(	RSC_BOX_MODE_DESC_CODE_EN,
					RSC_BOX_MODE_DESC_CODE_FR),
	[RSC_BOX_DCU_L1_TITLE]	= LDQ(	RSC_BOX_DCU_L1_TITLE_CODE),
	[RSC_BOX_DCU_L1_IP_TITLE]=LDQ(	RSC_BOX_DCU_L1_IP_TITLE_CODE),
	[RSC_BOX_DCU_L2_TITLE]	= LDQ(	RSC_BOX_DCU_L2_TITLE_CODE),
	[RSC_BOX_DCU_L2_CL_TITLE]=LDQ(	RSC_BOX_DCU_L2_CL_TITLE_CODE),
	[RSC_BOX_EIST_TITLE]	= LDQ(	RSC_BOX_EIST_TITLE_CODE),
	[RSC_BOX_EIST_DESC]	= LDT(	RSC_BOX_EIST_DESC_CODE_EN,
					RSC_BOX_EIST_DESC_CODE_FR),
	[RSC_BOX_C1E_TITLE]	= LDQ(	RSC_BOX_C1E_TITLE_CODE),
	[RSC_BOX_C1E_DESC]	= LDT(	RSC_BOX_C1E_DESC_CODE_EN,
					RSC_BOX_C1E_DESC_CODE_FR),
	[RSC_BOX_TURBO_TITLE]	= LDQ(	RSC_BOX_TURBO_TITLE_CODE),
	[RSC_BOX_TURBO_DESC]	= LDT(	RSC_BOX_TURBO_DESC_CODE_EN,
					RSC_BOX_TURBO_DESC_CODE_FR),
	[RSC_BOX_C1A_TITLE]	= LDQ(	RSC_BOX_C1A_TITLE_CODE),
	[RSC_BOX_C1A_DESC]	= LDT(	RSC_BOX_C1A_DESC_CODE_EN,
					RSC_BOX_C1A_DESC_CODE_FR),
	[RSC_BOX_C3A_TITLE]	= LDQ(	RSC_BOX_C3A_TITLE_CODE),
	[RSC_BOX_C3A_DESC]	= LDT(	RSC_BOX_C3A_DESC_CODE_EN,
					RSC_BOX_C3A_DESC_CODE_FR),
	[RSC_BOX_C1U_TITLE]	= LDQ(	RSC_BOX_C1U_TITLE_CODE),
	[RSC_BOX_C1U_DESC]	= LDT(	RSC_BOX_C1U_DESC_CODE_EN,
					RSC_BOX_C1U_DESC_CODE_FR),
	[RSC_BOX_C2U_TITLE]	= LDQ(	RSC_BOX_C2U_TITLE_CODE),
	[RSC_BOX_C2U_DESC]	= LDT(	RSC_BOX_C2U_DESC_CODE_EN,
					RSC_BOX_C2U_DESC_CODE_FR),
	[RSC_BOX_C3U_TITLE]	= LDQ(	RSC_BOX_C3U_TITLE_CODE),
	[RSC_BOX_C3U_DESC]	= LDT(	RSC_BOX_C3U_DESC_CODE_EN,
					RSC_BOX_C3U_DESC_CODE_FR),
	[RSC_BOX_C6D_DESC]	= LDT(	RSC_BOX_C6D_DESC_CODE_EN,
					RSC_BOX_C6D_DESC_CODE_FR),
	[RSC_BOX_MC6_TITLE]	= LDQ(	RSC_BOX_MC6_TITLE_CODE),
	[RSC_BOX_MC6_DESC]	= LDT(	RSC_BOX_MC6_DESC_CODE_EN,
					RSC_BOX_MC6_DESC_CODE_FR),
	[RSC_BOX_CC6_TITLE]	= LDQ(	RSC_BOX_CC6_TITLE_CODE),
	[RSC_BOX_C6D_TITLE]	= LDQ(	RSC_BOX_C6D_TITLE_CODE),
	[RSC_BOX_CC6_DESC]	= LDT(	RSC_BOX_CC6_DESC_CODE_EN,
					RSC_BOX_CC6_DESC_CODE_FR),
	[RSC_BOX_PC6_TITLE]	= LDQ(	RSC_BOX_PC6_TITLE_CODE),
	[RSC_BOX_PC6_DESC]	= LDT(	RSC_BOX_PC6_DESC_CODE_EN,
					RSC_BOX_PC6_DESC_CODE_FR),
	[RSC_BOX_HWP_TITLE]	= LDQ(	RSC_BOX_HWP_TITLE_CODE),
	[RSC_BOX_HWP_DESC]	= LDT(	RSC_BOX_HWP_DESC_CODE_EN,
					RSC_BOX_HWP_DESC_CODE_FR),
	[RSC_BOX_HDC_TITLE]	= LDQ(	RSC_BOX_HDC_TITLE_CODE),
	[RSC_BOX_HDC_DESC]	= LDT(	RSC_BOX_HDC_DESC_CODE_EN,
					RSC_BOX_HDC_DESC_CODE_FR),
	[RSC_BOX_EEO_TITLE]	= LDQ(	RSC_BOX_EEO_TITLE_CODE),
	[RSC_BOX_EEO_DESC]	= LDT(	RSC_BOX_EEO_DESC_CODE_EN,
					RSC_BOX_EEO_DESC_CODE_FR),
	[RSC_BOX_R2H_TITLE]	= LDQ(	RSC_BOX_R2H_TITLE_CODE),
	[RSC_BOX_R2H_DESC]	= LDT(	RSC_BOX_R2H_DESC_CODE_EN,
					RSC_BOX_R2H_DESC_CODE_FR),
	[RSC_BOX_BLANK_DESC]	= LDQ(	RSC_BOX_BLANK_DESC_CODE),
    [RSC_BOX_NOMINAL_MODE_COND0]= LDT(	RSC_BOX_NOM_MODE_COND0_CODE_EN,
					RSC_BOX_NOM_MODE_COND0_CODE_FR),
    [RSC_BOX_NOMINAL_MODE_COND1]= LDT(	RSC_BOX_NOM_MODE_COND1_CODE_EN,
					RSC_BOX_NOM_MODE_COND1_CODE_FR),
    [RSC_BOX_EXPERIMENT_MODE_COND0]=LDT(RSC_BOX_EXP_MODE_COND0_CODE_EN,
					RSC_BOX_EXP_MODE_COND0_CODE_FR),
    [RSC_BOX_EXPERIMENT_MODE_COND1]=LDT(RSC_BOX_EXP_MODE_COND1_CODE_EN,
					RSC_BOX_EXP_MODE_COND1_CODE_FR),
	[RSC_BOX_INTERRUPT_TITLE]=LDT(	RSC_BOX_INTERRUPT_TITLE_CODE_EN,
					RSC_BOX_INTERRUPT_TITLE_CODE_FR),
	[RSC_BOX_CPU_IDLE_TITLE] = LDT( RSC_BOX_CPU_IDLE_TITLE_CODE_EN,
					RSC_BOX_CPU_IDLE_TITLE_CODE_FR),
	[RSC_BOX_CPU_FREQ_TITLE] = LDT( RSC_BOX_CPU_FREQ_TITLE_CODE_EN,
					RSC_BOX_CPU_FREQ_TITLE_CODE_FR),
	[RSC_BOX_GOVERNOR_TITLE] = LDT( RSC_BOX_GOVERNOR_TITLE_CODE_EN,
					RSC_BOX_GOVERNOR_TITLE_CODE_FR),
    [RSC_BOX_CLOCK_SOURCE_TITLE] = LDT( RSC_BOX_CLOCK_SOURCE_TITLE_CODE_EN,
					RSC_BOX_CLOCK_SOURCE_TITLE_CODE_FR),
    [RSC_BOX_OPS_REGISTER_COND0]= LDT(	RSC_BOX_OPS_REGISTER_ST0_CODE_EN,
					RSC_BOX_OPS_REGISTER_ST0_CODE_FR),
    [RSC_BOX_OPS_REGISTER_COND1]= LDT(	RSC_BOX_OPS_REGISTER_ST1_CODE_EN,
					RSC_BOX_OPS_REGISTER_ST1_CODE_FR),
    [RSC_BOX_OPS_UNREGISTER_COND0]=LDT( RSC_BOX_OPS_UNREGIST_ST0_CODE_EN,
					RSC_BOX_OPS_UNREGIST_ST0_CODE_FR),
    [RSC_BOX_OPS_UNREGISTER_COND1]=LDT( RSC_BOX_OPS_UNREGIST_ST1_CODE_EN,
					RSC_BOX_OPS_UNREGIST_ST1_CODE_FR),
	[RSC_BOX_EVENT_TITLE]	= LDT(	RSC_BOX_EVENT_TITLE_CODE_EN,
					RSC_BOX_EVENT_TITLE_CODE_FR),
    [RSC_BOX_EVENT_THERMAL_SENSOR]= LDT(RSC_BOX_EVENT_THERMAL_SENSOR_CODE_EN,
					RSC_BOX_EVENT_THERMAL_SENSOR_CODE_FR),
    [RSC_BOX_EVENT_PROCHOT_AGENT]=LDT(	RSC_BOX_EVENT_PROCHOT_AGENT_CODE_EN,
					RSC_BOX_EVENT_PROCHOT_AGENT_CODE_FR),
    [RSC_BOX_EVENT_CRITICAL_TEMP]=LDT(	RSC_BOX_EVENT_CRITICAL_TEMP_CODE_EN,
					RSC_BOX_EVENT_CRITICAL_TEMP_CODE_FR),
    [RSC_BOX_EVENT_THERM_THRESHOLD]=LDT(RSC_BOX_EVENT_THERM_THRESHOLD_CODE_EN,
					RSC_BOX_EVENT_THERM_THRESHOLD_CODE_FR),
    [RSC_BOX_EVENT_POWER_LIMIT] = LDT(	RSC_BOX_EVENT_POWER_LIMIT_CODE_EN,
					RSC_BOX_EVENT_POWER_LIMIT_CODE_FR),
    [RSC_BOX_EVENT_CURRENT_LIMIT]=LDT(	RSC_BOX_EVENT_CURRENT_LIMIT_CODE_EN,
					RSC_BOX_EVENT_CURRENT_LIMIT_CODE_FR),
    [RSC_BOX_EVENT_CROSS_DOM_LIMIT]=LDT(RSC_BOX_EVENT_CROSS_DOM_LIMIT_CODE_EN,
					RSC_BOX_EVENT_CROSS_DOM_LIMIT_CODE_FR),
	[RSC_BOX_STATE_UNSPECIFIED]=LDT(RSC_BOX_STATE_UNSPECIFIED_CODE_EN,
					RSC_BOX_STATE_UNSPECIFIED_CODE_FR),
	[RSC_BOX_STATE_C8]	= LDQ(	RSC_BOX_STATE_C8_CODE),
	[RSC_BOX_STATE_C7]	= LDQ(	RSC_BOX_STATE_C7_CODE),
	[RSC_BOX_STATE_C6]	= LDQ(	RSC_BOX_STATE_C6_CODE),
	[RSC_BOX_STATE_C4]	= LDQ(	RSC_BOX_STATE_C4_CODE),
	[RSC_BOX_STATE_C3]	= LDQ(	RSC_BOX_STATE_C3_CODE),
    [RSC_BOX_PKG_STATE_LIMIT_TITLE]=LDT(RSC_BOX_PKG_STATE_TITLE_CODE_EN,
					RSC_BOX_PKG_STATE_TITLE_CODE_FR),
      [RSC_BOX_PKG_STATE_LIMIT_C10]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C10_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C9]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C9_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C8]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C8_CODE),
      [RSC_BOX_PKG_STATE_LIMIT_C7S]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C7S_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C7]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C7_CODE),
      [RSC_BOX_PKG_STATE_LIMIT_C6R]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C6R_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C6]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C6_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C4]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C4_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C3]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C3_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C2]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C2_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C1]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C1_CODE),
       [RSC_BOX_PKG_STATE_LIMIT_C0]=LDQ(RSC_BOX_PKG_STATE_LIMIT_C0_CODE),
	[RSC_BOX_IO_MWAIT_TITLE]= LDT(	RSC_BOX_IO_MWAIT_TITLE_CODE_EN,
					RSC_BOX_IO_MWAIT_TITLE_CODE_FR),
	[RSC_BOX_IO_MWAIT_DESC] = LDT(	RSC_BOX_IO_MWAIT_DESC_CODE_EN,
					RSC_BOX_IO_MWAIT_DESC_CODE_FR),
    [RSC_BOX_MWAIT_MAX_STATE_TITLE]=LDT(RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_EN,
					RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_FR),
	[RSC_BOX_ODCM_TITLE]	= LDT(	RSC_BOX_ODCM_TITLE_CODE_EN,
					RSC_BOX_ODCM_TITLE_CODE_FR),
	[RSC_BOX_ODCM_DESC]	= LDT(	RSC_BOX_ODCM_DESC_CODE_EN,
					RSC_BOX_ODCM_DESC_CODE_FR),
    [RSC_BOX_EXT_DUTY_CYCLE_TITLE]=LDT( RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_EN,
					RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_FR),
	[RSC_BOX_DUTY_CYCLE_TITLE]= LDT(RSC_BOX_DUTY_CYCLE_TITLE_CODE_EN,
					RSC_BOX_DUTY_CYCLE_TITLE_CODE_FR),
    [RSC_BOX_DUTY_CYCLE_RESERVED]= LDT( RSC_BOX_DUTY_CYCLE_RESERVED_CODE_EN,
					RSC_BOX_DUTY_CYCLE_RESERVED_CODE_FR),
	[RSC_BOX_DUTY_CYCLE_PCT1] = LDQ(RSC_BOX_DUTY_CYCLE_PCT1_CODE),
	[RSC_BOX_DUTY_CYCLE_PCT2] = LDQ(RSC_BOX_DUTY_CYCLE_PCT2_CODE),
	[RSC_BOX_DUTY_CYCLE_PCT3] = LDQ(RSC_BOX_DUTY_CYCLE_PCT3_CODE),
	[RSC_BOX_DUTY_CYCLE_PCT4] = LDQ(RSC_BOX_DUTY_CYCLE_PCT4_CODE),
	[RSC_BOX_DUTY_CYCLE_PCT5] = LDQ(RSC_BOX_DUTY_CYCLE_PCT5_CODE),
	[RSC_BOX_DUTY_CYCLE_PCT6] = LDQ(RSC_BOX_DUTY_CYCLE_PCT6_CODE),
	[RSC_BOX_DUTY_CYCLE_PCT7] = LDQ(RSC_BOX_DUTY_CYCLE_PCT7_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT1]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT1_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT2]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT2_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT3]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT3_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT4]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT4_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT5]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT5_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT6]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT6_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT7]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT7_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT8]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT8_CODE),
      [RSC_BOX_EXT_DUTY_CYCLE_PCT9]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT9_CODE),
     [RSC_BOX_EXT_DUTY_CYCLE_PCT10]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT10_CODE),
     [RSC_BOX_EXT_DUTY_CYCLE_PCT11]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT11_CODE),
     [RSC_BOX_EXT_DUTY_CYCLE_PCT12]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT12_CODE),
     [RSC_BOX_EXT_DUTY_CYCLE_PCT13]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT13_CODE),
     [RSC_BOX_EXT_DUTY_CYCLE_PCT14]=LDQ(RSC_BOX_EXT_DUTY_CYCLE_PCT14_CODE),
    [RSC_BOX_POWER_POLICY_TITLE]= LDT(	RSC_BOX_POWER_POLICY_TITLE_CODE_EN,
					RSC_BOX_POWER_POLICY_TITLE_CODE_FR),
	[RSC_BOX_POWER_POLICY_LOW]=LDT( RSC_BOX_POWER_POLICY_LOW_CODE_EN,
					RSC_BOX_POWER_POLICY_LOW_CODE_FR),
	[RSC_BOX_POWER_POLICY_1] = LDQ( RSC_BOX_POWER_POLICY_1_CODE),
	[RSC_BOX_POWER_POLICY_2] = LDQ( RSC_BOX_POWER_POLICY_2_CODE),
	[RSC_BOX_POWER_POLICY_3] = LDQ( RSC_BOX_POWER_POLICY_3_CODE),
	[RSC_BOX_POWER_POLICY_4] = LDQ( RSC_BOX_POWER_POLICY_4_CODE),
	[RSC_BOX_POWER_POLICY_5] = LDQ( RSC_BOX_POWER_POLICY_5_CODE),
	[RSC_BOX_POWER_POLICY_6] = LDQ( RSC_BOX_POWER_POLICY_6_CODE),
	[RSC_BOX_POWER_POLICY_7] = LDQ( RSC_BOX_POWER_POLICY_7_CODE),
	[RSC_BOX_POWER_POLICY_8] = LDQ( RSC_BOX_POWER_POLICY_8_CODE),
	[RSC_BOX_POWER_POLICY_9] = LDQ( RSC_BOX_POWER_POLICY_9_CODE),
	[RSC_BOX_POWER_POLICY_10]= LDQ( RSC_BOX_POWER_POLICY_10_CODE),
	[RSC_BOX_POWER_POLICY_11]= LDQ( RSC_BOX_POWER_POLICY_11_CODE),
	[RSC_BOX_POWER_POLICY_12]= LDQ( RSC_BOX_POWER_POLICY_12_CODE),
	[RSC_BOX_POWER_POLICY_13]= LDQ( RSC_BOX_POWER_POLICY_13_CODE),
	[RSC_BOX_POWER_POLICY_14]= LDQ( RSC_BOX_POWER_POLICY_14_CODE),
	[RSC_BOX_POWER_POLICY_HIGH]=LDT(RSC_BOX_POWER_POLICY_HIGH_CODE_EN,
					RSC_BOX_POWER_POLICY_HIGH_CODE_FR),
	[RSC_BOX_HWP_POLICY_MIN] = LDT( RSC_BOX_HWP_POLICY_MIN_CODE_EN,
					RSC_BOX_HWP_POLICY_MIN_CODE_FR),
	[RSC_BOX_HWP_POLICY_020] = LDT( RSC_BOX_HWP_POLICY_020_CODE_EN,
					RSC_BOX_HWP_POLICY_020_CODE_FR),
	[RSC_BOX_HWP_POLICY_040] = LDT( RSC_BOX_HWP_POLICY_040_CODE_EN,
					RSC_BOX_HWP_POLICY_040_CODE_FR),
	[RSC_BOX_HWP_POLICY_060] = LDT( RSC_BOX_HWP_POLICY_060_CODE_EN,
					RSC_BOX_HWP_POLICY_060_CODE_FR),
	[RSC_BOX_HWP_POLICY_MED] = LDT( RSC_BOX_HWP_POLICY_MED_CODE_EN,
					RSC_BOX_HWP_POLICY_MED_CODE_FR),
	[RSC_BOX_HWP_POLICY_0A0] = LDT( RSC_BOX_HWP_POLICY_0A0_CODE_EN,
					RSC_BOX_HWP_POLICY_0A0_CODE_FR),
	[RSC_BOX_HWP_POLICY_PWR] = LDT( RSC_BOX_HWP_POLICY_PWR_CODE_EN,
					RSC_BOX_HWP_POLICY_PWR_CODE_FR),
	[RSC_BOX_HWP_POLICY_0E0] = LDT( RSC_BOX_HWP_POLICY_0E0_CODE_EN,
					RSC_BOX_HWP_POLICY_0E0_CODE_FR),
	[RSC_BOX_HWP_POLICY_MAX] = LDT( RSC_BOX_HWP_POLICY_MAX_CODE_EN,
					RSC_BOX_HWP_POLICY_MAX_CODE_FR),
	[RSC_BOX_CFG_TDP_TITLE] = LDT(	RSC_BOX_CFG_TDP_TITLE_CODE_EN,
					RSC_BOX_CFG_TDP_TITLE_CODE_FR),
	[RSC_BOX_CFG_TDP_DESC]	= LDT(	RSC_BOX_CFG_TDP_DESC_CODE_EN,
					RSC_BOX_CFG_TDP_DESC_CODE_FR),
	[RSC_BOX_CFG_TDP_BLANK] = LDQ(	RSC_BOX_CFG_TDP_BLANK_CODE),
	[RSC_BOX_CFG_TDP_LVL0]	= LDT(	RSC_BOX_CFG_TDP_LVL0_CODE_EN,
					RSC_BOX_CFG_TDP_LVL0_CODE_FR),
	[RSC_BOX_CFG_TDP_LVL1]	= LDT(	RSC_BOX_CFG_TDP_LVL1_CODE_EN,
					RSC_BOX_CFG_TDP_LVL1_CODE_FR),
	[RSC_BOX_CFG_TDP_LVL2]	= LDT(	RSC_BOX_CFG_TDP_LVL2_CODE_EN,
					RSC_BOX_CFG_TDP_LVL2_CODE_FR),
	[RSC_BOX_TDP_PKG_TITLE] = LDT(	RSC_BOX_TDP_PKG_TITLE_CODE_EN,
					RSC_BOX_TDP_PKG_TITLE_CODE_FR),
	[RSC_BOX_TDP_CORES_TITLE]=LDT(	RSC_BOX_TDP_CORES_TITLE_CODE_EN,
					RSC_BOX_TDP_CORES_TITLE_CODE_FR),
	[RSC_BOX_TDP_UNCORE_TITLE]=LDT( RSC_BOX_TDP_UNCORE_TITLE_CODE_EN,
					RSC_BOX_TDP_UNCORE_TITLE_CODE_FR),
	[RSC_BOX_TDP_RAM_TITLE] = LDT(	RSC_BOX_TDP_RAM_TITLE_CODE_EN,
					RSC_BOX_TDP_RAM_TITLE_CODE_FR),
    [RSC_BOX_TDP_PLATFORM_TITLE]= LDT(	RSC_BOX_TDP_PLATFORM_TITLE_CODE_EN,
					RSC_BOX_TDP_PLATFORM_TITLE_CODE_FR),
	[RSC_BOX_PL1_DESC]	= LDT(	RSC_BOX_PL1_DESC_CODE_EN,
					RSC_BOX_PL1_DESC_CODE_FR),
	[RSC_BOX_PL2_DESC]	= LDT(	RSC_BOX_PL2_DESC_CODE_EN,
					RSC_BOX_PL2_DESC_CODE_FR),
	[RSC_BOX_PWR_OFFSET_00] = LDQ(	RSC_BOX_PWR_OFFSET_00_CODE),
	[RSC_BOX_PWR_OFFSET_01] = LDQ(	RSC_BOX_PWR_OFFSET_01_CODE),
	[RSC_BOX_PWR_OFFSET_02] = LDQ(	RSC_BOX_PWR_OFFSET_02_CODE),
	[RSC_BOX_PWR_OFFSET_03] = LDQ(	RSC_BOX_PWR_OFFSET_03_CODE),
	[RSC_BOX_PWR_OFFSET_04] = LDQ(	RSC_BOX_PWR_OFFSET_04_CODE),
	[RSC_BOX_PWR_OFFSET_05] = LDQ(	RSC_BOX_PWR_OFFSET_05_CODE),
	[RSC_BOX_PWR_OFFSET_06] = LDQ(	RSC_BOX_PWR_OFFSET_06_CODE),
	[RSC_BOX_PWR_OFFSET_07] = LDQ(	RSC_BOX_PWR_OFFSET_07_CODE),
	[RSC_BOX_PWR_OFFSET_08] = LDQ(	RSC_BOX_PWR_OFFSET_08_CODE),
	[RSC_BOX_PWR_OFFSET_09] = LDQ(	RSC_BOX_PWR_OFFSET_09_CODE),
	[RSC_BOX_PWR_OFFSET_10] = LDQ(	RSC_BOX_PWR_OFFSET_10_CODE),
	[RSC_BOX_PWR_OFFSET_11] = LDQ(	RSC_BOX_PWR_OFFSET_11_CODE),
	[RSC_BOX_PWR_OFFSET_12] = LDQ(	RSC_BOX_PWR_OFFSET_12_CODE),
	[RSC_BOX_PWR_OFFSET_13] = LDQ(	RSC_BOX_PWR_OFFSET_13_CODE),
   [RSC_BOX_CLAMPING_OFF_COND0] = LDQ(	RSC_BOX_CLAMPING_OFF_COND0_CODE),
   [RSC_BOX_CLAMPING_OFF_COND1] = LDQ(	RSC_BOX_CLAMPING_OFF_COND1_CODE),
    [RSC_BOX_CLAMPING_ON_COND0] = LDQ(	RSC_BOX_CLAMPING_ON_COND0_CODE),
    [RSC_BOX_CLAMPING_ON_COND1] = LDQ(	RSC_BOX_CLAMPING_ON_COND1_CODE),
	[RSC_BOX_TDC_TITLE]	= LDT(	RSC_BOX_TDC_TITLE_CODE_EN,
					RSC_BOX_TDC_TITLE_CODE_FR),
	[RSC_BOX_TDC_DESC]	= LDT(	RSC_BOX_TDC_DESC_CODE_EN,
					RSC_BOX_TDC_DESC_CODE_FR),
	[RSC_BOX_AMP_OFFSET_00] = LDQ(	RSC_BOX_AMP_OFFSET_00_CODE),
	[RSC_BOX_AMP_OFFSET_01] = LDQ(	RSC_BOX_AMP_OFFSET_01_CODE),
	[RSC_BOX_AMP_OFFSET_02] = LDQ(	RSC_BOX_AMP_OFFSET_02_CODE),
	[RSC_BOX_AMP_OFFSET_03] = LDQ(	RSC_BOX_AMP_OFFSET_03_CODE),
	[RSC_BOX_AMP_OFFSET_04] = LDQ(	RSC_BOX_AMP_OFFSET_04_CODE),
	[RSC_BOX_AMP_OFFSET_05] = LDQ(	RSC_BOX_AMP_OFFSET_05_CODE),
	[RSC_BOX_AMP_OFFSET_06] = LDQ(	RSC_BOX_AMP_OFFSET_06_CODE),
	[RSC_BOX_AMP_OFFSET_07] = LDQ(	RSC_BOX_AMP_OFFSET_07_CODE),
	[RSC_BOX_AMP_OFFSET_08] = LDQ(	RSC_BOX_AMP_OFFSET_08_CODE),
	[RSC_BOX_AMP_OFFSET_09] = LDQ(	RSC_BOX_AMP_OFFSET_09_CODE),
	[RSC_BOX_AMP_OFFSET_10] = LDQ(	RSC_BOX_AMP_OFFSET_10_CODE),
	[RSC_BOX_AMP_OFFSET_11] = LDQ(	RSC_BOX_AMP_OFFSET_11_CODE),
	[RSC_BOX_AMP_OFFSET_12] = LDQ(	RSC_BOX_AMP_OFFSET_12_CODE),
	[RSC_BOX_AMP_OFFSET_13] = LDQ(	RSC_BOX_AMP_OFFSET_13_CODE),
	[RSC_BOX_TOOLS_TITLE]	= LDT(	RSC_BOX_TOOLS_TITLE_CODE_EN,
					RSC_BOX_TOOLS_TITLE_CODE_FR),
	[RSC_BOX_TOOLS_STOP_BURN]=LDT(	RSC_BOX_TOOLS_STOP_CODE_EN,
					RSC_BOX_TOOLS_STOP_CODE_FR),
	[RSC_BOX_TOOLS_ATOMIC_BURN]=LDT(RSC_BOX_TOOLS_ATOMIC_CODE_EN,
					RSC_BOX_TOOLS_ATOMIC_CODE_FR),
	[RSC_BOX_TOOLS_CRC32_BURN]=LDT( RSC_BOX_TOOLS_CRC32_CODE_EN,
					RSC_BOX_TOOLS_CRC32_CODE_FR),
	[RSC_BOX_TOOLS_CONIC_BURN]=LDT( RSC_BOX_TOOLS_CONIC_CODE_EN,
					RSC_BOX_TOOLS_CONIC_CODE_FR),
	[RSC_BOX_TOOLS_RANDOM_CPU]=LDT( RSC_BOX_TOOLS_RAND_CPU_CODE_EN,
					RSC_BOX_TOOLS_RAND_CPU_CODE_FR),
    [RSC_BOX_TOOLS_ROUND_ROBIN_CPU]=LDT(RSC_BOX_TOOLS_RR_CPU_CODE_EN,
					RSC_BOX_TOOLS_RR_CPU_CODE_FR),
	[RSC_BOX_TOOLS_USER_CPU]= LDT(	RSC_BOX_TOOLS_USR_CPU_CODE_EN,
					RSC_BOX_TOOLS_USR_CPU_CODE_FR),
	[RSC_BOX_CONIC_TITLE]	= LDT(	RSC_BOX_CONIC_TITLE_CODE_EN,
					RSC_BOX_CONIC_TITLE_CODE_FR),
	[RSC_BOX_CONIC_ITEM_1]	= LDT(	RSC_BOX_CONIC_ITEM_1_CODE_EN,
					RSC_BOX_CONIC_ITEM_1_CODE_FR),
	[RSC_BOX_CONIC_ITEM_2]	= LDT(	RSC_BOX_CONIC_ITEM_2_CODE_EN,
					RSC_BOX_CONIC_ITEM_2_CODE_FR),
	[RSC_BOX_CONIC_ITEM_3]	= LDT(	RSC_BOX_CONIC_ITEM_3_CODE_EN,
					RSC_BOX_CONIC_ITEM_3_CODE_FR),
	[RSC_BOX_CONIC_ITEM_4]	= LDT(	RSC_BOX_CONIC_ITEM_4_CODE_EN,
					RSC_BOX_CONIC_ITEM_4_CODE_FR),
	[RSC_BOX_CONIC_ITEM_5]	= LDT(	RSC_BOX_CONIC_ITEM_5_CODE_EN,
					RSC_BOX_CONIC_ITEM_5_CODE_FR),
	[RSC_BOX_CONIC_ITEM_6]	= LDT(	RSC_BOX_CONIC_ITEM_6_CODE_EN,
					RSC_BOX_CONIC_ITEM_6_CODE_FR),
	[RSC_BOX_LANG_TITLE]	= LDT(	RSC_BOX_LANG_TITLE_CODE_EN,
					RSC_BOX_LANG_TITLE_CODE_FR),
	[RSC_BOX_LANG_ENGLISH]	= LDT(	RSC_BOX_LANG_ENGLISH_CODE_EN,
					RSC_BOX_LANG_ENGLISH_CODE_FR),
	[RSC_BOX_LANG_FRENCH]	= LDT(	RSC_BOX_LANG_FRENCH_CODE_EN,
					RSC_BOX_LANG_FRENCH_CODE_FR),
      [RSC_BOX_SCOPE_THERMAL_TITLE]=LDT(RSC_BOX_SCOPE_THERMAL_TITLE_CODE_EN,
					RSC_BOX_SCOPE_THERMAL_TITLE_CODE_FR),
      [RSC_BOX_SCOPE_VOLTAGE_TITLE]=LDT(RSC_BOX_SCOPE_VOLTAGE_TITLE_CODE_EN,
					RSC_BOX_SCOPE_VOLTAGE_TITLE_CODE_FR),
	[RSC_BOX_SCOPE_POWER_TITLE]=LDT(RSC_BOX_SCOPE_POWER_TITLE_CODE_EN,
					RSC_BOX_SCOPE_POWER_TITLE_CODE_FR),
	[RSC_BOX_SCOPE_NONE]	= LDT(	RSC_BOX_SCOPE_NONE_CODE_EN,
					RSC_BOX_SCOPE_NONE_CODE_FR),
	[RSC_BOX_SCOPE_THREAD]	= LDT(	RSC_BOX_SCOPE_THREAD_CODE_EN,
					RSC_BOX_SCOPE_THREAD_CODE_FR),
	[RSC_BOX_SCOPE_CORE]	= LDT(	RSC_BOX_SCOPE_CORE_CODE_EN,
					RSC_BOX_SCOPE_CORE_CODE_FR),
	[RSC_BOX_SCOPE_PACKAGE]	= LDT(	RSC_BOX_SCOPE_PACKAGE_CODE_EN,
					RSC_BOX_SCOPE_PACKAGE_CODE_FR),
	[RSC_ERROR_CMD_SYNTAX]	= LDT(	RSC_ERROR_CMD_SYNTAX_CODE_EN,
					RSC_ERROR_CMD_SYNTAX_CODE_FR),
	[RSC_ERROR_SHARED_MEM]	= LDT(	RSC_ERROR_SHARED_MEM_CODE_EN,
					RSC_ERROR_SHARED_MEM_CODE_FR),
	[RSC_ERROR_SYS_CALL]	= LDT(	RSC_ERROR_SYS_CALL_CODE_EN,
					RSC_ERROR_SYS_CALL_CODE_FR),
	[RSC_ERROR_UNIMPLEMENTED] = LDT(RSC_ERROR_UNIMPLEMENTED_CODE_EN,
					RSC_ERROR_UNIMPLEMENTED_CODE_FR),
	[RSC_ERROR_EXPERIMENTAL] = LDT( RSC_ERROR_EXPERIMENTAL_CODE_EN,
					RSC_ERROR_EXPERIMENTAL_CODE_FR),
	[RSC_ERROR_TURBO_PREREQ] = LDT( RSC_ERROR_TURBO_PREREQ_CODE_EN,
					RSC_ERROR_TURBO_PREREQ_CODE_FR),
	[RSC_ERROR_UNCORE_PREREQ] = LDT(RSC_ERROR_UNCORE_PREREQ_CODE_EN,
					RSC_ERROR_UNCORE_PREREQ_CODE_FR),
      [RSC_ERROR_PSTATE_NOT_FOUND] =LDT(RSC_ERROR_PSTATE_NOT_FOUND_CODE_EN,
					RSC_ERROR_PSTATE_NOT_FOUND_CODE_FR),
	[RSC_BOX_IDLE_LIMIT_TITLE]=LDT( RSC_BOX_IDLE_LIMIT_TITLE_CODE_EN,
					RSC_BOX_IDLE_LIMIT_TITLE_CODE_FR),
	[RSC_BOX_IDLE_LIMIT_RESET]=LDQ( RSC_BOX_IDLE_LIMIT_RESET_CODE),
	[RSC_BOX_RECORDER_TITLE]= LDT(	RSC_BOX_RECORDER_TITLE_CODE_EN,
					RSC_BOX_RECORDER_TITLE_CODE_FR),
	[RSC_SMBIOS_TITLE]	= LDT(	RSC_SMBIOS_TITLE_CODE_EN,
					RSC_SMBIOS_TITLE_CODE_FR),
	[RSC_MECH_IBRS] 	= LDT(	RSC_MECH_IBRS_CODE_EN,
					RSC_MECH_IBRS_CODE_FR),
	[RSC_MECH_IBPB] 	= LDT(	RSC_MECH_IBPB_CODE_EN,
					RSC_MECH_IBPB_CODE_FR),
	[RSC_MECH_STIBP]	= LDT(	RSC_MECH_STIBP_CODE_EN,
					RSC_MECH_STIBP_CODE_FR),
	[RSC_MECH_SSBD] 	= LDT(	RSC_MECH_SSBD_CODE_EN,
					RSC_MECH_SSBD_CODE_FR),
	[RSC_MECH_L1D_FLUSH]	= LDT(	RSC_MECH_L1D_FLUSH_CODE_EN,
					RSC_MECH_L1D_FLUSH_CODE_FR),
	[RSC_MECH_MD_CLEAR]	= LDT(	RSC_MECH_MD_CLEAR_CODE_EN,
					RSC_MECH_MD_CLEAR_CODE_FR),
	[RSC_MECH_RDCL_NO]	= LDT(	RSC_MECH_RDCL_NO_CODE_EN,
					RSC_MECH_RDCL_NO_CODE_FR),
	[RSC_MECH_IBRS_ALL]	= LDT(	RSC_MECH_IBRS_ALL_CODE_EN,
					RSC_MECH_IBRS_ALL_CODE_FR),
	[RSC_MECH_RSBA] 	= LDT(	RSC_MECH_RSBA_CODE_EN,
					RSC_MECH_RSBA_CODE_FR),
	[RSC_MECH_L1DFL_VMENTRY_NO]=LDT(RSC_MECH_L1DFL_VMENTRY_NO_CODE_EN,
					RSC_MECH_L1DFL_VMENTRY_NO_CODE_FR),
	[RSC_MECH_SSB_NO]	= LDT(	RSC_MECH_SSB_NO_CODE_EN,
					RSC_MECH_SSB_NO_CODE_FR),
	[RSC_MECH_MDS_NO]	= LDT(	RSC_MECH_MDS_NO_CODE_EN,
					RSC_MECH_MDS_NO_CODE_FR),
	[RSC_MECH_PSCHANGE_MC_NO]=LDT(	RSC_MECH_PSCHANGE_MC_NO_CODE_EN,
					RSC_MECH_PSCHANGE_MC_NO_CODE_FR),
	[RSC_MECH_TAA_NO]	= LDT(	RSC_MECH_TAA_NO_CODE_EN,
					RSC_MECH_TAA_NO_CODE_FR),
	[RSC_MECH_SPLA] 	= LDT(	RSC_MECH_SPLA_CODE_EN,
					RSC_MECH_SPLA_CODE_FR),
	[RSC_LOGO_ROW_0]	= LDT(	RSC_LOGO_R0,
					RSC_LOGO_R0),
	[RSC_LOGO_ROW_1]	= LDT(	RSC_LOGO_R1,
					RSC_LOGO_R1),
	[RSC_LOGO_ROW_2]	= LDT(	RSC_LOGO_R2,
					RSC_LOGO_R2),
	[RSC_LOGO_ROW_3]	= LDT(	RSC_LOGO_R3,
					RSC_LOGO_R3),
	[RSC_LOGO_ROW_4]	= LDT(	RSC_LOGO_R4,
					RSC_LOGO_R4),
	[RSC_LOGO_ROW_5]	= LDT(	RSC_LOGO_R5,
					RSC_LOGO_R5),
	[RSC_COPY_ROW_0]	= LDT(	RSC_COPY_R0_EN,
					RSC_COPY_R0_FR),
	[RSC_COPY_ROW_1]	= LDT(	RSC_COPY_R1_EN,
					RSC_COPY_R1_FR),
	[RSC_COPY_ROW_2]	= LDT(	RSC_COPY_R2_EN,
					RSC_COPY_R2_FR),
    [RSC_CREATE_SELECT_AUTO_TURBO]=LDT( RSC_CREATE_SELECT_AUTO_TURBO_CODE_EN,
					RSC_CREATE_SELECT_AUTO_TURBO_CODE_FR),
    [RSC_CREATE_SELECT_FREQ_TURBO]=LDT( RSC_CREATE_SELECT_FREQ_TURBO_CODE_EN,
					RSC_CREATE_SELECT_FREQ_TURBO_CODE_FR),
      [RSC_CREATE_SELECT_FREQ_TGT]=LDT( RSC_CREATE_SELECT_FREQ_TGT_CODE_EN,
					RSC_CREATE_SELECT_FREQ_TGT_CODE_FR),
  [RSC_CREATE_SELECT_FREQ_HWP_TGT]=LDT( RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_EN,
					RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_FR),
  [RSC_CREATE_SELECT_FREQ_HWP_MAX]=LDT( RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_EN,
					RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_FR),
  [RSC_CREATE_SELECT_FREQ_HWP_MIN]=LDT( RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_EN,
					RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_FR),
      [RSC_CREATE_SELECT_FREQ_MAX]=LDT( RSC_CREATE_SELECT_FREQ_MAX_CODE_EN,
					RSC_CREATE_SELECT_FREQ_MAX_CODE_FR),
      [RSC_CREATE_SELECT_FREQ_MIN]=LDT( RSC_CREATE_SELECT_FREQ_MIN_CODE_EN,
					RSC_CREATE_SELECT_FREQ_MIN_CODE_FR),
   [RSC_CREATE_SELECT_FREQ_OFFLINE]=LDT(RSC_CREATE_SELECT_FREQ_OFFLINE_CODE_EN,
					RSC_CREATE_SELECT_FREQ_OFFLINE_CODE_FR),
	[RSC_POPUP_DRIVER_TITLE] = LDT( RSC_POPUP_DRIVER_TITLE_CODE_EN,
					RSC_POPUP_DRIVER_TITLE_CODE_FR),
	[RSC_EXIT_TITLE]	= LDT(	RSC_EXIT_TITLE_CODE_EN,
					RSC_EXIT_TITLE_CODE_FR),
	[RSC_EXIT_HEADER]	= LDT(	RSC_EXIT_HEADER_CODE_EN,
					RSC_EXIT_HEADER_CODE_FR),
	[RSC_EXIT_CONFIRM]	= LDT(	RSC_EXIT_CONFIRM_CODE_EN,
					RSC_EXIT_CONFIRM_CODE_FR),
	[RSC_EXIT_FOOTER]	= LDT(	RSC_EXIT_FOOTER_CODE_EN,
					RSC_EXIT_FOOTER_CODE_FR)
};

#undef LDV
#undef LDA
#undef LDB
#undef LDN
#undef LDS
#undef LDT
#undef LDQ
