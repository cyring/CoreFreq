/*
 * CoreFreq
 * Copyright (C) 2015-2019 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <locale.h>
#include <sched.h>
#include <errno.h>

#include "corefreq-ui.h"
#include "corefreq-cli-rsc.h"
#include "corefreq-cli-rsc-en.h"
#include "corefreq-cli-rsc-fr.h"

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

ATTRIBUTE Rsc_Layout_Ruller_Load_Attr[] = RSC_LAYOUT_RULLER_LOAD_ATTR;
ASCII	Rsc_Layout_Ruller_Load_Code_En[] = RSC_LAYOUT_RULLER_LOAD_CODE;
#define Rsc_Layout_Ruller_Load_Code_Fr Rsc_Layout_Ruller_Load_Code_En

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

ATTRIBUTE Rsc_Layout_Ruller_Frequency_Attr[] = RSC_LAYOUT_RULLER_FREQUENCY_ATTR;
ASCII Rsc_Layout_Ruller_Frequency_Code_En[] = RSC_LAYOUT_RULLER_FREQUENCY_CODE;
#define Rsc_Layout_Ruller_Frequency_Code_Fr Rsc_Layout_Ruller_Frequency_Code_En

ATTRIBUTE Rsc_Layout_Ruller_Freq_Avg_Attr[] = \
					RSC_LAYOUT_RULLER_FREQUENCY_AVG_ATTR;
ASCII	Rsc_Layout_Ruller_Freq_Avg_Code_En[] = \
					RSC_LAYOUT_RULLER_FREQUENCY_AVG_CODE_EN,
	Rsc_Layout_Ruller_Freq_Avg_Code_Fr[] = \
					RSC_LAYOUT_RULLER_FREQUENCY_AVG_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruller_Freq_Pkg_Attr[] = \
					RSC_LAYOUT_RULLER_FREQUENCY_PKG_ATTR;
ASCII	Rsc_Layout_Ruller_Freq_Pkg_Code_En[] = \
					RSC_LAYOUT_RULLER_FREQUENCY_PKG_CODE;
#define Rsc_Layout_Ruller_Freq_Pkg_Code_Fr Rsc_Layout_Ruller_Freq_Pkg_Code_En

#define Rsc_Layout_Ruller_Inst_Attr vColor
ASCII	Rsc_Layout_Ruller_Inst_Code_En[] = RSC_LAYOUT_RULLER_INST_CODE;
#define Rsc_Layout_Ruller_Inst_Code_Fr Rsc_Layout_Ruller_Inst_Code_En

#define Rsc_Layout_Ruller_Cycles_Attr vColor
ASCII	Rsc_Layout_Ruller_Cycles_Code_En[] = RSC_LAYOUT_RULLER_CYCLES_CODE;
#define Rsc_Layout_Ruller_Cycles_Code_Fr Rsc_Layout_Ruller_Cycles_Code_En

#define Rsc_Layout_Ruller_CStates_Attr vColor
ASCII	Rsc_Layout_Ruller_CStates_Code_En[] = RSC_LAYOUT_RULLER_CSTATES_CODE;
#define Rsc_Layout_Ruller_CStates_Code_Fr Rsc_Layout_Ruller_CStates_Code_En

ATTRIBUTE Rsc_Layout_Ruller_Interrupts_Attr[] = \
					RSC_LAYOUT_RULLER_INTERRUPTS_ATTR;
ASCII	Rsc_Layout_Ruller_Interrupts_Code_En[] = \
					RSC_LAYOUT_RULLER_INTERRUPTS_CODE;
#define Rsc_Layout_Ruller_Interrupts_Code_Fr \
					Rsc_Layout_Ruller_Interrupts_Code_En

#define Rsc_Layout_Ruller_Package_Attr vColor
ASCII	Rsc_Layout_Ruller_Package_Code_En[] = RSC_LAYOUT_RULLER_PACKAGE_CODE_EN,
	Rsc_Layout_Ruller_Package_Code_Fr[] = RSC_LAYOUT_RULLER_PACKAGE_CODE_FR;

ATTRIBUTE Rsc_Layout_Package_PC_Attr[] = RSC_LAYOUT_PACKAGE_PC_ATTR;
ASCII	Rsc_Layout_Package_PC_Code_En[] = RSC_LAYOUT_PACKAGE_PC_CODE;
#define Rsc_Layout_Package_PC_Code_Fr Rsc_Layout_Package_PC_Code_En

ATTRIBUTE Rsc_Layout_Package_Uncore_Attr[] = RSC_LAYOUT_PACKAGE_UNCORE_ATTR;
ASCII	Rsc_Layout_Package_Uncore_Code_En[] = RSC_LAYOUT_PACKAGE_UNCORE_CODE;
#define Rsc_Layout_Package_Uncore_Code_Fr Rsc_Layout_Package_Uncore_Code_En

ATTRIBUTE Rsc_Layout_Ruller_Tasks_Attr[] = RSC_LAYOUT_RULLER_TASKS_ATTR;
ASCII	Rsc_Layout_Ruller_Tasks_Code_En[] = RSC_LAYOUT_RULLER_TASKS_CODE_EN,
	Rsc_Layout_Ruller_Tasks_Code_Fr[] = RSC_LAYOUT_RULLER_TASKS_CODE_FR;

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

ATTRIBUTE Rsc_Layout_Ruller_Voltage_Attr[] = RSC_LAYOUT_RULLER_VOLTAGE_ATTR;
ASCII	Rsc_Layout_Ruller_Voltage_Code_En[] = RSC_LAYOUT_RULLER_VOLTAGE_CODE_EN,
	Rsc_Layout_Ruller_Voltage_Code_Fr[] = RSC_LAYOUT_RULLER_VOLTAGE_CODE_FR;

#define Rsc_Layout_Domain_Package_Attr vColor
#define Rsc_Layout_Domain_Cores_Attr vColor
#define Rsc_Layout_Domain_Uncore_Attr vColor
#define Rsc_Layout_Domain_Memory_Attr vColor
ASCII	Rsc_Layout_Domain_Package_Code_En[7]= RSC_LAYOUT_DOMAIN_PACKAGE_CODE_EN,
	Rsc_Layout_Domain_Package_Code_Fr[7]= RSC_LAYOUT_DOMAIN_PACKAGE_CODE_FR,
	Rsc_Layout_Domain_Cores_Code_En[7]  = RSC_LAYOUT_DOMAIN_CORES_CODE_EN,
	Rsc_Layout_Domain_Cores_Code_Fr[7]  = RSC_LAYOUT_DOMAIN_CORES_CODE_FR,
	Rsc_Layout_Domain_Uncore_Code_En[7] = RSC_LAYOUT_DOMAIN_UNCORE_CODE_EN,
	Rsc_Layout_Domain_Uncore_Code_Fr[7] = RSC_LAYOUT_DOMAIN_UNCORE_CODE_FR,
	Rsc_Layout_Domain_Memory_Code_En[7] = RSC_LAYOUT_DOMAIN_MEMORY_CODE_EN,
	Rsc_Layout_Domain_Memory_Code_Fr[7] = RSC_LAYOUT_DOMAIN_MEMORY_CODE_FR;

ATTRIBUTE Rsc_Layout_Power_Monitor_Attr[] = RSC_LAYOUT_POWER_MONITOR_ATTR;
ASCII	Rsc_Layout_Power_Monitor_Code_En[] = RSC_LAYOUT_POWER_MONITOR_CODE;
#define Rsc_Layout_Power_Monitor_Code_Fr Rsc_Layout_Power_Monitor_Code_En

ATTRIBUTE Rsc_Layout_Ruller_Slice_Attr[] = RSC_LAYOUT_RULLER_SLICE_ATTR;
ASCII	Rsc_Layout_Ruller_Slice_Code_En[] = RSC_LAYOUT_RULLER_SLICE_CODE_EN,
	Rsc_Layout_Ruller_Slice_Code_Fr[] = RSC_LAYOUT_RULLER_SLICE_CODE_FR;

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

ATTRIBUTE Rsc_Layout_Card_Core_Online_Attr[] = RSC_LAYOUT_CARD_CORE_ONLINE_ATTR;
ASCII	Rsc_Layout_Card_Core_Online_Code_En[]= RSC_LAYOUT_CARD_CORE_ONLINE_CODE;
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

ATTRIBUTE Rsc_SystemRegisters_Cond_Attr[3][4] = {
	RSC_SYSTEM_REGISTERS_COND0_ATTR,
	RSC_SYSTEM_REGISTERS_COND1_ATTR,
	RSC_SYSTEM_REGISTERS_COND2_ATTR
};

ATTRIBUTE Rsc_SysInfoProc_Cond_Attr[4][76] = {
	RSC_SYSINFO_PROC_COND0_ATTR,
	RSC_SYSINFO_PROC_COND1_ATTR,
	RSC_SYSINFO_PROC_COND2_ATTR,
	RSC_SYSINFO_PROC_COND3_ATTR
};

ATTRIBUTE Rsc_SysInfoISA_Cond_Attr[4][5][17] = {
	RSC_SYSINFO_ISA_COND0_ATTR,
	RSC_SYSINFO_ISA_COND1_ATTR,
	RSC_SYSINFO_ISA_COND2_ATTR,
	RSC_SYSINFO_ISA_COND3_ATTR
};

ATTRIBUTE Rsc_SysInfoFeatures_Cond_Attr[3][72] = {
	RSC_SYSINFO_FEATURES_COND0_ATTR,
	RSC_SYSINFO_FEATURES_COND1_ATTR,
	RSC_SYSINFO_FEATURES_COND2_ATTR
};

ATTRIBUTE Rsc_SysInfoTech_Cond_Attr[2][50] = {
	RSC_SYSINFO_TECH_COND0_ATTR,
	RSC_SYSINFO_TECH_COND1_ATTR
};

ATTRIBUTE Rsc_SysInfoPerfMon_Cond_Attr[4][74] = {
	RSC_SYSINFO_PERFMON_COND0_ATTR,
	RSC_SYSINFO_PERFMON_COND1_ATTR,
	RSC_SYSINFO_PERFMON_COND2_ATTR,
	RSC_SYSINFO_PERFMON_COND3_ATTR
};

ATTRIBUTE Rsc_SysInfoPwrThermal_Cond_Attr[4][50] = {
	RSC_SYSINFO_PWR_THERMAL_COND0_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND1_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND2_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND3_ATTR
};

ATTRIBUTE Rsc_SysInfoKernel_Attr[] = RSC_SYSINFO_KERNEL_ATTR;

ATTRIBUTE Rsc_Topology_Cond_Attr[3][13] = {
	RSC_TOPOLOGY_COND0_ATTR,
	RSC_TOPOLOGY_COND1_ATTR,
	RSC_TOPOLOGY_COND2_ATTR
};

ATTRIBUTE Rsc_MemoryController_Cond_Attr[2][14] = {
	RSC_MEMORY_CONTROLLER_COND0_ATTR,
	RSC_MEMORY_CONTROLLER_COND1_ATTR
};

ATTRIBUTE Rsc_CreateMenu_Stop_Attr[] = RSC_CREATE_MENU_STOP_ATTR,
	  Rsc_CreateMenu_FnKey_Attr[] = RSC_CREATE_MENU_FN_KEY_ATTR,
	  Rsc_CreateMenu_ShortKey_Attr[] = RSC_CREATE_MENU_SHORTKEY_ATTR;

ATTRIBUTE Rsc_CreateSettings_Cond_Attr[2][32] = {
	RSC_CREATE_SETTINGS_COND0_ATTR,
	RSC_CREATE_SETTINGS_COND1_ATTR
};

ASCII	Rsc_CreateSettings_Blank_Code[] = "                                ";

ASCII	Rsc_CreateHelp_Blank_Code[] = "                  ";

ATTRIBUTE Rsc_CreateAdvHelp_Cond_Attr[2][38] = {
	RSC_CREATE_ADV_HELP_COND0_ATTR,
	RSC_CREATE_ADV_HELP_COND1_ATTR
};

ASCII Rsc_CreateAdvHelp_Blank_Code[] = "                                      ";

ATTRIBUTE Rsc_CreateHotPlugCPU_Enable_Attr[]=RSC_CREATE_HOTPLUG_CPU_ENABLE_ATTR,
	Rsc_CreateHotPlugCPU_Disable_Attr[]=RSC_CREATE_HOTPLUG_CPU_DISABLE_ATTR;

ATTRIBUTE Rsc_CreateRatioClock_Cond_Attr[3][28] = {
	RSC_CREATE_RATIO_CLOCK_COND0_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND1_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND2_ATTR
};
ATTRIBUTE Rsc_CreateSelectCPU_Cond_Attr[2][26] = {
	RSC_CREATE_SELECT_CPU_COND0_ATTR,
	RSC_CREATE_SELECT_CPU_COND1_ATTR
};

ATTRIBUTE Rsc_HotEvent_Cond_Attr[5][3] = {
	RSC_HOT_EVENT_COND0_ATTR,
	RSC_HOT_EVENT_COND1_ATTR,
	RSC_HOT_EVENT_COND2_ATTR,
	RSC_HOT_EVENT_COND3_ATTR,
	RSC_HOT_EVENT_COND4_ATTR
};

ATTRIBUTE Rsc_BoxEvent_Attr[] = RSC_BOX_EVENT_ATTR;

RESOURCE_ST Resource[] = {
	[RSC_LAYOUT_HEADER_PROC] = {
		.Attr = Rsc_Layout_Header_Proc_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Header_Proc_Code_En,
			[LOC_FR] = Rsc_Layout_Header_Proc_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_CPU] = {
		.Attr = Rsc_Layout_Header_CPU_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Header_CPU_Code_En,
			[LOC_FR] = Rsc_Layout_Header_CPU_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_ARCH] = {
		.Attr = Rsc_Layout_Header_Arch_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Header_Arch_Code_En,
			[LOC_FR] = Rsc_Layout_Header_Arch_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_CACHE_L1] = {
		.Attr = Rsc_Layout_Header_Cache_L1_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Header_Cache_L1_Code_En,
			[LOC_FR] = Rsc_Layout_Header_Cache_L1_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_BCLK] = {
		.Attr = Rsc_Layout_Header_BClk_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Header_BClk_Code_En,
			[LOC_FR] = Rsc_Layout_Header_BClk_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_CACHES] = {
		.Attr = Rsc_Layout_Header_Caches_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Header_Caches_Code_En,
			[LOC_FR] = Rsc_Layout_Header_Caches_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_LOAD] = {
		.Attr = Rsc_Layout_Ruller_Load_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Load_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Load_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_FREQUENCY] = {
		.Attr = Rsc_Layout_Monitor_Frequency_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Monitor_Frequency_Code_En,
			[LOC_FR] = Rsc_Layout_Monitor_Frequency_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_INST] = {
		.Attr = Rsc_Layout_Monitor_Inst_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Monitor_Inst_Code_En,
			[LOC_FR] = Rsc_Layout_Monitor_Inst_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_COMMON] = {
		.Attr = Rsc_Layout_Monitor_Common_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Monitor_Common_Code_En,
			[LOC_FR] = Rsc_Layout_Monitor_Common_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_TASKS] = {
		.Attr = Rsc_Layout_Monitor_Tasks_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Monitor_Tasks_Code_En,
			[LOC_FR] = Rsc_Layout_Monitor_Tasks_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_SLICE] = {
		.Attr = Rsc_Layout_Monitor_Slice_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Monitor_Slice_Code_En,
			[LOC_FR] = Rsc_Layout_Monitor_Slice_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_FREQUENCY] = {
		.Attr = Rsc_Layout_Ruller_Frequency_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Frequency_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Frequency_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_FREQUENCY_AVG] = {
		.Attr = Rsc_Layout_Ruller_Freq_Avg_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Freq_Avg_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Freq_Avg_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_FREQUENCY_PKG] = {
		.Attr = Rsc_Layout_Ruller_Freq_Pkg_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Freq_Pkg_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Freq_Pkg_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_INST] = {
		.Attr = Rsc_Layout_Ruller_Inst_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Inst_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Inst_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_CYCLES] = {
		.Attr = Rsc_Layout_Ruller_Cycles_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Cycles_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Cycles_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_CSTATES] = {
		.Attr = Rsc_Layout_Ruller_CStates_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_CStates_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_CStates_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_INTERRUPTS] = {
		.Attr = Rsc_Layout_Ruller_Interrupts_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Interrupts_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Interrupts_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_PACKAGE] = {
		.Attr = Rsc_Layout_Ruller_Package_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Package_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Package_Code_Fr
		}
	},
	[RSC_LAYOUT_PACKAGE_PC] = {
		.Attr = Rsc_Layout_Package_PC_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Package_PC_Code_En,
			[LOC_FR] = Rsc_Layout_Package_PC_Code_Fr
		}
	},
	[RSC_LAYOUT_PACKAGE_UNCORE] = {
		.Attr = Rsc_Layout_Package_Uncore_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Package_Uncore_Code_En,
			[LOC_FR] = Rsc_Layout_Package_Uncore_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_TASKS] = {
		.Attr = Rsc_Layout_Ruller_Tasks_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Tasks_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Tasks_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_TRACKING] = {
		.Attr = Rsc_Layout_Tasks_Tracking_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Tracking_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Tracking_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_STATE_SORTED] = {
		.Attr = Rsc_Layout_Tasks_State_Sorted_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_State_Sorted_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_State_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_RUNTIME_SORTED] = {
		.Attr = Rsc_Layout_Tasks_RunTime_Sorted_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_RunTime_Sorted_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_RunTime_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_USRTIME_SORTED] = {
		.Attr = Rsc_Layout_Tasks_UsrTime_Sorted_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_UsrTime_Sorted_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_UsrTime_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_SYSTIME_SORTED] = {
		.Attr = Rsc_Layout_Tasks_SysTime_Sorted_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_SysTime_Sorted_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_SysTime_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_PROCESS_SORTED] = {
		.Attr = Rsc_Layout_Tasks_Process_Sorted_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Process_Sorted_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Process_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_COMMAND_SORTED] = {
		.Attr = Rsc_Layout_Tasks_Command_Sorted_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Command_Sorted_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Command_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_REVERSE_SORT_OFF] = {
		.Attr = Rsc_Layout_Tasks_Reverse_Sort_Off_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Reverse_Sort_Off_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Reverse_Sort_Off_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_REVERSE_SORT_ON] = {
		.Attr = Rsc_Layout_Tasks_Reverse_Sort_On_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Reverse_Sort_On_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Reverse_Sort_On_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_VALUE_SWITCH] = {
		.Attr = Rsc_Layout_Tasks_Value_Switch_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Value_Switch_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Value_Switch_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_VALUE_OFF] = {
		.Attr = Rsc_Layout_Tasks_Value_Off_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Value_Off_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Value_Off_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_VALUE_ON] = {
		.Attr = Rsc_Layout_Tasks_Value_On_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Tasks_Value_On_Code_En,
			[LOC_FR] = Rsc_Layout_Tasks_Value_On_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_VOLTAGE] = {
		.Attr = Rsc_Layout_Ruller_Voltage_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Voltage_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Voltage_Code_Fr
		}
	},
	[RSC_LAYOUT_DOMAIN_PACKAGE] = {
		.Attr = Rsc_Layout_Domain_Package_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Domain_Package_Code_En,
			[LOC_FR] = Rsc_Layout_Domain_Package_Code_Fr
		}
	},
	[RSC_LAYOUT_DOMAIN_CORES] = {
		.Attr = Rsc_Layout_Domain_Cores_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Domain_Cores_Code_En,
			[LOC_FR] = Rsc_Layout_Domain_Cores_Code_Fr
		}
	},
	[RSC_LAYOUT_DOMAIN_UNCORE] = {
		.Attr = Rsc_Layout_Domain_Uncore_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Domain_Uncore_Code_En,
			[LOC_FR] = Rsc_Layout_Domain_Uncore_Code_Fr
		}
	},
	[RSC_LAYOUT_DOMAIN_MEMORY] = {
		.Attr = Rsc_Layout_Domain_Memory_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Domain_Memory_Code_En,
			[LOC_FR] = Rsc_Layout_Domain_Memory_Code_Fr
		}
	},
	[RSC_LAYOUT_POWER_MONITOR] = {
		.Attr = Rsc_Layout_Power_Monitor_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Power_Monitor_Code_En,
			[LOC_FR] = Rsc_Layout_Power_Monitor_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_SLICE] = {
		.Attr = Rsc_Layout_Ruller_Slice_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Ruller_Slice_Code_En,
			[LOC_FR] = Rsc_Layout_Ruller_Slice_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_TECH_X86] = {
		.Attr = Rsc_Layout_Footer_Tech_x86_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Footer_Tech_x86_Code_En,
			[LOC_FR] = Rsc_Layout_Footer_Tech_x86_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_TECH_INTEL] = {
		.Attr = Rsc_Layout_Footer_Tech_Intel_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Footer_Tech_Intel_Code_En,
			[LOC_FR] = Rsc_Layout_Footer_Tech_Intel_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_TECH_AMD] = {
		.Attr = Rsc_Layout_Footer_Tech_AMD_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Footer_Tech_AMD_Code_En,
			[LOC_FR] = Rsc_Layout_Footer_Tech_AMD_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_SYSTEM] = {
		.Attr = Rsc_Layout_Footer_System_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Footer_System_Code_En,
			[LOC_FR] = Rsc_Layout_Footer_System_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_CORE_ONLINE] = {
		.Attr = Rsc_Layout_Card_Core_Online_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Core_Online_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Core_Online_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_CORE_OFFLINE] = {
		.Attr = Rsc_Layout_Card_Core_Offline_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Core_Offline_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Core_Offline_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_CLK] = {
		.Attr = Rsc_Layout_Card_CLK_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_CLK_Code_En,
			[LOC_FR] = Rsc_Layout_Card_CLK_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_UNCORE] = {
		.Attr = Rsc_Layout_Card_Uncore_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Uncore_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Uncore_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_BUS] = {
		.Attr = Rsc_Layout_Card_Bus_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Bus_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Bus_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_MC] = {
		.Attr = Rsc_Layout_Card_MC_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_MC_Code_En,
			[LOC_FR] = Rsc_Layout_Card_MC_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_LOAD] = {
		.Attr = Rsc_Layout_Card_Load_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Load_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Load_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_IDLE] = {
		.Attr = Rsc_Layout_Card_Idle_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Idle_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Idle_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_RAM] = {
		.Attr = Rsc_Layout_Card_RAM_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_RAM_Code_En,
			[LOC_FR] = Rsc_Layout_Card_RAM_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_TASK] = {
		.Attr = Rsc_Layout_Card_Task_Attr,
		.Code = {
			[LOC_EN] = Rsc_Layout_Card_Task_Code_En,
			[LOC_FR] = Rsc_Layout_Card_Task_Code_Fr
		}
	},
/* ATTRIBUTE */
	[RSC_RUN_STATE_COLOR] = {
		.Attr = Rsc_SymbolRunColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_UNINT_STATE_COLOR] = {
		.Attr = Rsc_SymbolUnIntColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_ZOMBIE_STATE_COLOR] = {
		.Attr = Rsc_SymbolZombieColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SLEEP_STATE_COLOR] = {
		.Attr = Rsc_SymbolSleepColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_WAIT_STATE_COLOR] = {
		.Attr = Rsc_SymbolWaitColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_OTHER_STATE_COLOR] = {
		.Attr = Rsc_SymbolOtherColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_TRACKER_STATE_COLOR] = {
		.Attr = Rsc_SymbolTrackerColor_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_CPUID_COND0] = {
		.Attr = Rsc_SysInfoCPUID_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_CPUID_COND1] = {
		.Attr = Rsc_SysInfoCPUID_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_CPUID_COND2] = {
		.Attr = Rsc_SysInfoCPUID_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_CPUID_COND3] = {
		.Attr = Rsc_SysInfoCPUID_Cond_Attr[3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSTEM_REGISTERS_COND0] = {
		.Attr = Rsc_SystemRegisters_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSTEM_REGISTERS_COND1] = {
		.Attr = Rsc_SystemRegisters_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSTEM_REGISTERS_COND2] = {
		.Attr = Rsc_SystemRegisters_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PROC_COND0] = {
		.Attr = Rsc_SysInfoProc_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PROC_COND1] = {
		.Attr = Rsc_SysInfoProc_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PROC_COND2] = {
		.Attr = Rsc_SysInfoProc_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PROC_COND3] = {
		.Attr = Rsc_SysInfoProc_Cond_Attr[3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_0_0] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[0][0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_0_1] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[0][1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_0_2] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[0][2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_0_3] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[0][3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_0_4] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[0][4],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_1_0] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[1][0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_1_1] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[1][1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_1_2] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[1][2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_1_3] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[1][3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_1_4] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[1][4],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_2_0] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[2][0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_2_1] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[2][1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_2_2] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[2][2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_2_3] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[2][3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_2_4] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[2][4],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_3_0] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[3][0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_3_1] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[3][1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_3_2] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[3][2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_3_3] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[3][3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_ISA_COND_3_4] = {
		.Attr = Rsc_SysInfoISA_Cond_Attr[3][4],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_FEATURES_COND0] = {
		.Attr = Rsc_SysInfoFeatures_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_FEATURES_COND1] = {
		.Attr = Rsc_SysInfoFeatures_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_FEATURES_COND2] = {
		.Attr = Rsc_SysInfoFeatures_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_TECH_COND0] = {
		.Attr = Rsc_SysInfoTech_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_TECH_COND1] = {
		.Attr = Rsc_SysInfoTech_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PERFMON_COND0] = {
		.Attr = Rsc_SysInfoPerfMon_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PERFMON_COND1] = {
		.Attr = Rsc_SysInfoPerfMon_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PERFMON_COND2] = {
		.Attr = Rsc_SysInfoPerfMon_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PERFMON_COND3] = {
		.Attr = Rsc_SysInfoPerfMon_Cond_Attr[3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PWR_THERMAL_COND0] = {
		.Attr = Rsc_SysInfoPwrThermal_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PWR_THERMAL_COND1] = {
		.Attr = Rsc_SysInfoPwrThermal_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PWR_THERMAL_COND2] = {
		.Attr = Rsc_SysInfoPwrThermal_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_PWR_THERMAL_COND3] = {
		.Attr = Rsc_SysInfoPwrThermal_Cond_Attr[3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_SYSINFO_KERNEL] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_TOPOLOGY_COND0] = {
		.Attr = Rsc_Topology_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_TOPOLOGY_COND1] = {
		.Attr = Rsc_Topology_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_TOPOLOGY_COND2] = {
		.Attr = Rsc_Topology_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_MEMORY_CONTROLLER_COND0] = {
		.Attr = Rsc_MemoryController_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_MEMORY_CONTROLLER_COND1] = {
		.Attr = Rsc_MemoryController_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_MENU_STOP] = {
		.Attr = Rsc_CreateMenu_Stop_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_MENU_FN_KEY] = {
		.Attr = Rsc_CreateMenu_FnKey_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_MENU_SHORTKEY] = {
		.Attr = Rsc_CreateMenu_ShortKey_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_SETTINGS_COND0] = {
		.Attr = Rsc_CreateSettings_Cond_Attr[0],
		.Code = {
			[LOC_EN] = Rsc_CreateSettings_Blank_Code,
			[LOC_FR] = Rsc_CreateSettings_Blank_Code
		}
	},
	[RSC_CREATE_SETTINGS_COND1] = {
		.Attr = Rsc_CreateSettings_Cond_Attr[1],
		.Code = {
			[LOC_EN] = Rsc_CreateSettings_Blank_Code,
			[LOC_FR] = Rsc_CreateSettings_Blank_Code
		}
	},
	[RSC_CREATE_ADV_HELP_COND0] = {
		.Attr = Rsc_CreateAdvHelp_Cond_Attr[0],
		.Code = {
			[LOC_EN] = Rsc_CreateAdvHelp_Blank_Code,
			[LOC_FR] = Rsc_CreateAdvHelp_Blank_Code
		}
	},
	[RSC_CREATE_ADV_HELP_COND1] = {
		.Attr = Rsc_CreateAdvHelp_Cond_Attr[1],
		.Code = {
			[LOC_EN] = Rsc_CreateAdvHelp_Blank_Code,
			[LOC_FR] = Rsc_CreateAdvHelp_Blank_Code
		}
	},
	[RSC_CREATE_HOTPLUG_CPU_ENABLE] = {
		.Attr = Rsc_CreateHotPlugCPU_Enable_Attr,
		.Code = {
			[LOC_EN] = RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_EN,
			[LOC_FR] = RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_FR
		}
	},
	[RSC_CREATE_HOTPLUG_CPU_DISABLE] = {
		.Attr = Rsc_CreateHotPlugCPU_Disable_Attr,
		.Code = {
			[LOC_EN] = RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_EN,
			[LOC_FR] = RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_FR
			}
	},
	[RSC_CREATE_RATIO_CLOCK_COND0] = {
		.Attr = Rsc_CreateRatioClock_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_RATIO_CLOCK_COND1] = {
		.Attr = Rsc_CreateRatioClock_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_RATIO_CLOCK_COND2] = {
		.Attr = Rsc_CreateRatioClock_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_SELECT_CPU_COND0] = {
		.Attr = Rsc_CreateSelectCPU_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_CREATE_SELECT_CPU_COND1] = {
		.Attr = Rsc_CreateSelectCPU_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_HOT_EVENT_COND0] = {
		.Attr = Rsc_HotEvent_Cond_Attr[0],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_HOT_EVENT_COND1] = {
		.Attr = Rsc_HotEvent_Cond_Attr[1],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_HOT_EVENT_COND2] = {
		.Attr = Rsc_HotEvent_Cond_Attr[2],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_HOT_EVENT_COND3] = {
		.Attr = Rsc_HotEvent_Cond_Attr[3],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_HOT_EVENT_COND4] = {
		.Attr = Rsc_HotEvent_Cond_Attr[4],
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
	[RSC_BOX_EVENT] = {
		.Attr = Rsc_BoxEvent_Attr,
		.Code = {[LOC_EN] = hSpace, [LOC_FR] = hSpace}
	},
/* ASCII */
	[RSC_PROCESSOR_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PROCESSOR_TITLE_CODE_EN,
			[LOC_FR] = RSC_PROCESSOR_TITLE_CODE_FR
		}
	},
	[RSC_PROCESSOR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PROCESSOR_CODE_EN,
			[LOC_FR] = RSC_PROCESSOR_CODE_FR
		}
	},
	[RSC_ARCHITECTURE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ARCHITECTURE_CODE_EN,
			[LOC_FR] = RSC_ARCHITECTURE_CODE_FR
		}
	},
	[RSC_VENDOR_ID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_VENDOR_ID_CODE_EN,
			[LOC_FR] = RSC_VENDOR_ID_CODE_FR
		}
	},
	[RSC_MICROCODE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MICROCODE_CODE_EN,
			[LOC_FR] = RSC_MICROCODE_CODE_FR
		}
	},
	[RSC_SIGNATURE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SIGNATURE_CODE_EN,
			[LOC_FR] = RSC_SIGNATURE_CODE_FR
		}
	},
	[RSC_STEPPING] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_STEPPING_CODE_EN,
			[LOC_FR] = RSC_STEPPING_CODE_FR
		}
	},
	[RSC_ONLINE_CPU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ONLINE_CPU_CODE_EN,
			[LOC_FR] = RSC_ONLINE_CPU_CODE_FR
		}
	},
	[RSC_BASE_CLOCK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BASE_CLOCK_CODE_EN,
			[LOC_FR] = RSC_BASE_CLOCK_CODE_FR
		}
	},
	[RSC_FREQUENCY] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FREQUENCY_CODE_EN,
			[LOC_FR] = RSC_FREQUENCY_CODE_FR
		}
	},
	[RSC_RATIO] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_RATIO_CODE_EN,
			[LOC_FR] = RSC_RATIO_CODE_FR
		}
	},
	[RSC_FACTORY] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FACTORY_CODE_EN,
			[LOC_FR] = RSC_FACTORY_CODE_FR
		}
	},
	[RSC_PERFORMANCE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERFORMANCE_CODE_EN,
			[LOC_FR] = RSC_PERFORMANCE_CODE_FR
		}
	},
	[RSC_TARGET] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TARGET_CODE_EN,
			[LOC_FR] = RSC_TARGET_CODE_FR
		}
	},
	[RSC_LEVEL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_LEVEL_CODE_EN,
			[LOC_FR] = RSC_LEVEL_CODE_FR
		}
	},
	[RSC_PROGRAMMABLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PROGRAMMABLE_CODE_EN,
			[LOC_FR] = RSC_PROGRAMMABLE_CODE_FR
		}
	},
	[RSC_CONFIGURATION] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_CONFIGURATION_CODE_EN,
			[LOC_FR] = RSC_CONFIGURATION_CODE_FR
		}
	},
	[RSC_TURBO_ACTIVATION] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TURBO_ACTIVATION_CODE_EN,
			[LOC_FR] = RSC_TURBO_ACTIVATION_CODE_FR
		}
	},
	[RSC_NOMINAL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_NOMINAL_CODE_EN,
			[LOC_FR] = RSC_NOMINAL_CODE_FR
		}
	},
	[RSC_UNLOCK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_UNLOCK_CODE_EN,
			[LOC_FR] = RSC_UNLOCK_CODE_FR
		}
	},
	[RSC_LOCK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_LOCK_CODE_EN,
			[LOC_FR] = RSC_LOCK_CODE_FR
		}
	},
	[RSC_ENABLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ENABLE_CODE_EN,
			[LOC_FR] = RSC_ENABLE_CODE_FR
		}
	},
	[RSC_DISABLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_DISABLE_CODE_EN,
			[LOC_FR] = RSC_DISABLE_CODE_FR
		}
	},
	[RSC_CAPABILITIES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_CAPABILITIES_CODE_EN,
			[LOC_FR] = RSC_CAPABILITIES_CODE_FR
		}
	},
	[RSC_LOWEST] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_LOWEST_CODE_EN,
			[LOC_FR] = RSC_LOWEST_CODE_FR
		}
	},
	[RSC_EFFICIENT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_EFFICIENT_CODE_EN,
			[LOC_FR] = RSC_EFFICIENT_CODE_FR
		}
	},
	[RSC_GUARANTEED] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_GUARANTEED_CODE_EN,
			[LOC_FR] = RSC_GUARANTEED_CODE_FR
		}
	},
	[RSC_HIGHEST] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HIGHEST_CODE_EN,
			[LOC_FR] = RSC_HIGHEST_CODE_FR
		}
	},
	[RSC_CPUID_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_CPUID_TITLE_EN,
			[LOC_FR] = RSC_CPUID_TITLE_FR
		}
	},
	[RSC_LARGEST_STD_FUNC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_LARGEST_STD_FUNC_CODE_EN,
			[LOC_FR] = RSC_LARGEST_STD_FUNC_CODE_FR
		}
	},
	[RSC_LARGEST_EXT_FUNC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_LARGEST_EXT_FUNC_CODE_EN,
			[LOC_FR] = RSC_LARGEST_EXT_FUNC_CODE_FR
		}
	},
	[RSC_SYS_REGS_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SYS_REGS_TITLE_CODE_EN,
			[LOC_FR] = RSC_SYS_REGS_TITLE_CODE_FR
		}
	},
	[RSC_ISA_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ISA_TITLE_CODE_EN,
			[LOC_FR] = RSC_ISA_TITLE_CODE_FR
		}
	},
	[RSC_FEATURES_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_TITLE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_TITLE_CODE_FR
		}
	},
	[RSC_MISSING] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MISSING_CODE_EN,
			[LOC_FR] = RSC_MISSING_CODE_FR
		}
	},
	[RSC_PRESENT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PRESENT_CODE_EN,
			[LOC_FR] = RSC_PRESENT_CODE_FR
		}
	},
	[RSC_VARIANT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_VARIANT_CODE_EN,
			[LOC_FR] = RSC_VARIANT_CODE_FR
		}
	},
	[RSC_INVARIANT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_INVARIANT_CODE_EN,
			[LOC_FR] = RSC_INVARIANT_CODE_FR
		}
	},
	[RSC_FEATURES_1GB_PAGES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_1GB_PAGES_CODE_EN,
			[LOC_FR] = RSC_FEATURES_1GB_PAGES_CODE_FR
		}
	},
	[RSC_FEATURES_100MHZ] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_100MHZ_CODE_EN,
			[LOC_FR] = RSC_FEATURES_100MHZ_CODE_FR
		}
	},
	[RSC_FEATURES_ACPI] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_ACPI_CODE_EN,
			[LOC_FR] = RSC_FEATURES_ACPI_CODE_FR
		}
	},
	[RSC_FEATURES_APIC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_APIC_CODE_EN,
			[LOC_FR] = RSC_FEATURES_APIC_CODE_FR
		}
	},
	[RSC_FEATURES_CORE_MP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_CORE_MP_CODE_EN,
			[LOC_FR] = RSC_FEATURES_CORE_MP_CODE_FR
		}
	},
	[RSC_FEATURES_CNXT_ID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_CNXT_ID_CODE_EN,
			[LOC_FR] = RSC_FEATURES_CNXT_ID_CODE_FR
		}
	},
	[RSC_FEATURES_DCA] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_DCA_CODE_EN,
			[LOC_FR] = RSC_FEATURES_DCA_CODE_FR
		}
	},
	[RSC_FEATURES_DE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_DE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_DE_CODE_FR
		}
	},
	[RSC_FEATURES_DS_PEBS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_DS_PEBS_CODE_EN,
			[LOC_FR] = RSC_FEATURES_DS_PEBS_CODE_FR
		}
	},
	[RSC_FEATURES_DS_CPL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_DS_CPL_CODE_EN,
			[LOC_FR] = RSC_FEATURES_DS_CPL_CODE_FR
		}
	},
	[RSC_FEATURES_DTES_64] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_DTES_64_CODE_EN,
			[LOC_FR] = RSC_FEATURES_DTES_64_CODE_FR
		}
	},
	[RSC_FEATURES_FAST_STR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_FAST_STR_CODE_EN,
			[LOC_FR] = RSC_FEATURES_FAST_STR_CODE_FR
		}
	},
	[RSC_FEATURES_FMA] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_FMA_CODE_EN,
			[LOC_FR] = RSC_FEATURES_FMA_CODE_FR
		}
	},
	[RSC_FEATURES_HLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_HLE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_HLE_CODE_FR
		}
	},
	[RSC_FEATURES_LM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_LM_CODE_EN,
			[LOC_FR] = RSC_FEATURES_LM_CODE_FR
		}
	},
	[RSC_FEATURES_LWP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_LWP_CODE_EN,
			[LOC_FR] = RSC_FEATURES_LWP_CODE_FR
		}
	},
	[RSC_FEATURES_MCA] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_MCA_CODE_EN,
			[LOC_FR] = RSC_FEATURES_MCA_CODE_FR
		}
	},
	[RSC_FEATURES_MSR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_MSR_CODE_EN,
			[LOC_FR] = RSC_FEATURES_MSR_CODE_FR
		}
	},
	[RSC_FEATURES_MTRR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_MTRR_CODE_EN,
			[LOC_FR] = RSC_FEATURES_MTRR_CODE_FR
		}
	},
	[RSC_FEATURES_NX] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_NX_CODE_EN,
			[LOC_FR] = RSC_FEATURES_NX_CODE_FR
		}
	},
	[RSC_FEATURES_OSXSAVE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_OSXSAVE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_OSXSAVE_CODE_FR
		}
	},
	[RSC_FEATURES_PAE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PAE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PAE_CODE_FR
		}
	},
	[RSC_FEATURES_PAT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PAT_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PAT_CODE_FR
		}
	},
	[RSC_FEATURES_PBE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PBE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PBE_CODE_FR
		}
	},
	[RSC_FEATURES_PCID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PCID_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PCID_CODE_FR
		}
	},
	[RSC_FEATURES_PDCM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PDCM_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PDCM_CODE_FR
		}
	},
	[RSC_FEATURES_PGE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PGE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PGE_CODE_FR
		}
	},
	[RSC_FEATURES_PSE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PSE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PSE_CODE_FR
		}
	},
	[RSC_FEATURES_PSE36] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PSE36_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PSE36_CODE_FR
		}
	},
	[RSC_FEATURES_PSN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_PSN_CODE_EN,
			[LOC_FR] = RSC_FEATURES_PSN_CODE_FR
		}
	},
	[RSC_FEATURES_RTM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_RTM_CODE_EN,
			[LOC_FR] = RSC_FEATURES_RTM_CODE_FR
		}
	},
	[RSC_FEATURES_SMX] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_SMX_CODE_EN,
			[LOC_FR] = RSC_FEATURES_SMX_CODE_FR
		}
	},
	[RSC_FEATURES_SELF_SNOOP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_SELF_SNOOP_CODE_EN,
			[LOC_FR] = RSC_FEATURES_SELF_SNOOP_CODE_FR
		}
	},
	[RSC_FEATURES_TSC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_TSC_CODE_EN,
			[LOC_FR] = RSC_FEATURES_TSC_CODE_FR
		}
	},
	[RSC_FEATURES_TSC_DEADLN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_TSC_DEADLN_CODE_EN,
			[LOC_FR] = RSC_FEATURES_TSC_DEADLN_CODE_FR
		}
	},
	[RSC_FEATURES_VME] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_VME_CODE_EN,
			[LOC_FR] = RSC_FEATURES_VME_CODE_FR
		}
	},
	[RSC_FEATURES_VMX] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_VMX_CODE_EN,
			[LOC_FR] = RSC_FEATURES_VMX_CODE_FR
		}
	},
	[RSC_FEATURES_X2APIC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_X2APIC_CODE_EN,
			[LOC_FR] = RSC_FEATURES_X2APIC_CODE_FR
		}
	},
	[RSC_FEATURES_XD_BIT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_XD_BIT_CODE_EN,
			[LOC_FR] = RSC_FEATURES_XD_BIT_CODE_FR
		}
	},
	[RSC_FEATURES_XSAVE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_XSAVE_CODE_EN,
			[LOC_FR] = RSC_FEATURES_XSAVE_CODE_FR
		}
	},
	[RSC_FEATURES_XTPR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FEATURES_XTPR_CODE_EN,
			[LOC_FR] = RSC_FEATURES_XTPR_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_TITLE_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_TITLE_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_SMM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_SMM_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_SMM_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_HTT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_HTT_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_HTT_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_EIST] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_EIST_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_EIST_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_IDA] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_IDA_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_IDA_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_TURBO] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_TURBO_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_TURBO_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_VM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_VM_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_VM_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_IOMMU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_IOMMU_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_IOMMU_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_SMT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_SMT_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_SMT_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_CNQ] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_CNQ_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_CNQ_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_CPB] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_CPB_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_CPB_CODE_FR
		}
	},
	[RSC_TECHNOLOGIES_HYPERV] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TECHNOLOGIES_HYPERV_CODE_EN,
			[LOC_FR] = RSC_TECHNOLOGIES_HYPERV_CODE_FR
		}
	},
	[RSC_PERF_MON_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_TITLE_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_TITLE_CODE_FR
		}
	},
	[RSC_VERSION] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_VERSION_CODE_EN,
			[LOC_FR] = RSC_VERSION_CODE_FR
		}
	},
	[RSC_COUNTERS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_COUNTERS_CODE_EN,
			[LOC_FR] = RSC_COUNTERS_CODE_FR
		}
	},
	[RSC_GENERAL_CTRS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_GENERAL_CTRS_CODE_EN,
			[LOC_FR] = RSC_GENERAL_CTRS_CODE_FR
		}
	},
	[RSC_FIXED_CTRS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_FIXED_CTRS_CODE_EN,
			[LOC_FR] = RSC_FIXED_CTRS_CODE_FR
		}
	},
	[RSC_PERF_MON_C1E] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_C1E_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_C1E_CODE_FR
		}
	},
	[RSC_PERF_MON_C1A] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_C1A_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_C1A_CODE_FR
		}
	},
	[RSC_PERF_MON_C3A] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_C3A_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_C3A_CODE_FR
		}
	},
	[RSC_PERF_MON_C1U] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_C1U_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_C1U_CODE_FR
		}
	},
	[RSC_PERF_MON_C3U] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_C3U_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_C3U_CODE_FR
		}
	},
	[RSC_PERF_MON_CC6] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_CC6_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_CC6_CODE_FR
		}
	},
	[RSC_PERF_MON_PC6] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_PC6_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_PC6_CODE_FR
		}
	},
	[RSC_PERF_MON_FID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_FID_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_FID_CODE_FR
		}
	},
	[RSC_PERF_MON_VID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_VID_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_VID_CODE_FR
		}
	},
	[RSC_PERF_MON_HWCF] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_HWCF_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_HWCF_CODE_FR
		}
	},
	[RSC_PERF_MON_HWP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_HWP_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_HWP_CODE_FR
		}
	},
	[RSC_PERF_MON_HDC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_HDC_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_HDC_CODE_FR
		}
	},
	[RSC_PERF_MON_PKG_CSTATE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_PKG_CSTATE_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_PKG_CSTATE_CODE_FR
		}
	},
	[RSC_PERF_MON_CFG_CTRL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_CFG_CTRL_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_CFG_CTRL_CODE_FR
		}
	},
	[RSC_PERF_MON_LOW_CSTATE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_LOW_CSTATE_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_LOW_CSTATE_CODE_FR
		}
	},
	[RSC_PERF_MON_IOMWAIT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_IOMWAIT_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_IOMWAIT_CODE_FR
		}
	},
	[RSC_PERF_MON_MAX_CSTATE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_MAX_CSTATE_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_MAX_CSTATE_CODE_FR
		}
	},
	[RSC_PERF_MON_MWAIT_CTRS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_MWAIT_CTRS_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_MWAIT_CTRS_CODE_FR
		}
	},
	[RSC_PERF_MON_CORE_CYCLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_CORE_CYCLE_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_CORE_CYCLE_CODE_FR
		}
	},
	[RSC_PERF_MON_INST_RET] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_INST_RET_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_INST_RET_CODE_FR
		}
	},
	[RSC_PERF_MON_REF_CYCLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_REF_CYCLE_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_REF_CYCLE_CODE_FR
		}
	},
	[RSC_PERF_MON_REF_LLC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_REF_LLC_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_REF_LLC_CODE_FR
		}
	},
	[RSC_PERF_MON_MISS_LLC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_MISS_LLC_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_MISS_LLC_CODE_FR
		}
	},
	[RSC_PERF_MON_BRANCH_RET] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_BRANCH_RET_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_BRANCH_RET_CODE_FR
		}
	},
	[RSC_PERF_MON_BRANCH_MIS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_PERF_MON_BRANCH_MIS_CODE_EN,
			[LOC_FR] = RSC_PERF_MON_BRANCH_MIS_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_TITLE_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_TITLE_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_ODCM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_ODCM_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_ODCM_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_DUTY] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_DUTY_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_DUTY_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_MGMT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_MGMT_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_MGMT_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_BIAS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_BIAS_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_BIAS_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_TJMAX] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_TJMAX_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_TJMAX_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_DTS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_DTS_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_DTS_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_PLN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_PLN_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_PLN_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_PTM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_PTM_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_PTM_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_TM1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_TM1_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_TM1_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_TM2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_TM2_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_TM2_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_UNITS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_UNITS_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_UNITS_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_POWER] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_POWER_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_POWER_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_ENERGY] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_ENERGY_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_ENERGY_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_WINDOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_WINDOW_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_WINDOW_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_WATT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_WATT_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_WATT_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_JOULE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_JOULE_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_JOULE_CODE_FR
		}
	},
	[RSC_POWER_THERMAL_SECOND] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_POWER_THERMAL_SECOND_CODE_EN,
			[LOC_FR] = RSC_POWER_THERMAL_SECOND_CODE_FR
		}
	},
	[RSC_KERNEL_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_KERNEL_TITLE_CODE_EN,
			[LOC_FR] = RSC_KERNEL_TITLE_CODE_FR
		}
	},
	[RSC_KERNEL_TOTAL_RAM] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_TOTAL_RAM_CODE_EN,
			[LOC_FR] = RSC_KERNEL_TOTAL_RAM_CODE_FR
		}
	},
	[RSC_KERNEL_SHARED_RAM] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_SHARED_RAM_CODE_EN,
			[LOC_FR] = RSC_KERNEL_SHARED_RAM_CODE_FR
		}
	},
	[RSC_KERNEL_FREE_RAM] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_FREE_RAM_CODE_EN,
			[LOC_FR] = RSC_KERNEL_FREE_RAM_CODE_FR
		}
	},
	[RSC_KERNEL_BUFFER_RAM] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_BUFFER_RAM_CODE_EN,
			[LOC_FR] = RSC_KERNEL_BUFFER_RAM_CODE_FR
		}
	},
	[RSC_KERNEL_TOTAL_HIGH] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_TOTAL_HIGH_CODE_EN,
			[LOC_FR] = RSC_KERNEL_TOTAL_HIGH_CODE_FR
		}
	},
	[RSC_KERNEL_FREE_HIGH] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_FREE_HIGH_CODE_EN,
			[LOC_FR] = RSC_KERNEL_FREE_HIGH_CODE_FR
		}
	},
	[RSC_KERNEL_IDLE_DRIVER] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_IDLE_DRIVER_CODE_EN,
			[LOC_FR] = RSC_KERNEL_IDLE_DRIVER_CODE_FR
		}
	},
	[RSC_KERNEL_RELEASE] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_RELEASE_CODE_EN,
			[LOC_FR] = RSC_KERNEL_RELEASE_CODE_FR
		}
	},
	[RSC_KERNEL_VERSION] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_VERSION_CODE_EN,
			[LOC_FR] = RSC_KERNEL_VERSION_CODE_FR
		}
	},
	[RSC_KERNEL_MACHINE] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_MACHINE_CODE_EN,
			[LOC_FR] = RSC_KERNEL_MACHINE_CODE_FR
		}
	},
	[RSC_KERNEL_MEMORY] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_MEMORY_CODE_EN,
			[LOC_FR] = RSC_KERNEL_MEMORY_CODE_FR
		}
	},
	[RSC_KERNEL_STATE] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_STATE_CODE_EN,
			[LOC_FR] = RSC_KERNEL_STATE_CODE_FR
		}
	},
	[RSC_KERNEL_POWER] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_POWER_CODE_EN,
			[LOC_FR] = RSC_KERNEL_POWER_CODE_FR
		}
	},
	[RSC_KERNEL_LATENCY] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_LATENCY_CODE_EN,
			[LOC_FR] = RSC_KERNEL_LATENCY_CODE_FR
		}
	},
	[RSC_KERNEL_RESIDENCY] = {
		.Attr = Rsc_SysInfoKernel_Attr,
		.Code = {
			[LOC_EN] = RSC_KERNEL_RESIDENCY_CODE_EN,
			[LOC_FR] = RSC_KERNEL_RESIDENCY_CODE_FR
		}
	},
	[RSC_TOPOLOGY_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TOPOLOGY_TITLE_CODE_EN,
			[LOC_FR] = RSC_TOPOLOGY_TITLE_CODE_FR
		}
	},
	[RSC_MEM_CTRL_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_TITLE_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_TITLE_CODE_FR
		}
	},
	[RSC_MEM_CTRL_BLANK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_BLANK_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_BLANK_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT1_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT1_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT1_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT1_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT1_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT1_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT1_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT1_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT1_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SINGLE_CHA_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SINGLE_CHA_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SINGLE_CHA_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SINGLE_CHA_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SINGLE_CHA_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SINGLE_CHA_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SINGLE_CHA_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SINGLE_CHA_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SINGLE_CHA_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DUAL_CHA_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DUAL_CHA_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DUAL_CHA_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DUAL_CHA_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DUAL_CHA_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DUAL_CHA_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DUAL_CHA_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DUAL_CHA_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DUAL_CHA_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_TRIPLE_CHA_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_TRIPLE_CHA_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_TRIPLE_CHA_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_QUAD_CHA_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_QUAD_CHA_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_QUAD_CHA_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_QUAD_CHA_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_QUAD_CHA_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_QUAD_CHA_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_QUAD_CHA_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_QUAD_CHA_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_QUAD_CHA_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SIX_CHA_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SIX_CHA_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SIX_CHA_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SIX_CHA_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SIX_CHA_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SIX_CHA_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SIX_CHA_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SIX_CHA_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SIX_CHA_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_EIGHT_CHA_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_EIGHT_CHA_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_EIGHT_CHA_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_EIGHT_CHA_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_EIGHT_CHA_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_EIGHT_CHA_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_EIGHT_CHA_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_EIGHT_CHA_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_EIGHT_CHA_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_BUS_RATE_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_BUS_RATE_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_BUS_RATE_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_BUS_RATE_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_BUS_RATE_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_BUS_RATE_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_BUS_SPEED_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_BUS_SPEED_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_BUS_SPEED_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_BUS_SPEED_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_BUS_SPEED_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_BUS_SPEED_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DRAM_SPEED_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DRAM_SPEED_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DRAM_SPEED_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DRAM_SPEED_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DRAM_SPEED_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DRAM_SPEED_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT2_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT2_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT2_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT2_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT2_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT2_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT2_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT2_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT2_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT2_3] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT2_3_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT2_3_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT2_4] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT2_4_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT2_4_CODE_FR
		}
	},
	[RSC_MEM_CTRL_SUBSECT2_5] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_SUBSECT2_5_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_SUBSECT2_5_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_SLOT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_SLOT_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_SLOT_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_BANK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_BANK_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_BANK_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_RANK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_RANK_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_RANK_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_ROW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_ROW_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_ROW_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_COLUMN0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_COLUMN0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_COLUMN0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_COLUMN1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_COLUMN1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_COLUMN1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_SIZE_0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_SIZE_0_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_SIZE_0_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_SIZE_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_SIZE_1_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_SIZE_1_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_SIZE_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_SIZE_2_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_SIZE_2_CODE_FR
		}
	},
	[RSC_MEM_CTRL_DIMM_SIZE_3] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MEM_CTRL_DIMM_SIZE_3_CODE_EN,
			[LOC_FR] = RSC_MEM_CTRL_DIMM_SIZE_3_CODE_FR
		}
	},
	[RSC_TASKS_SORTBY_STATE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TASKS_SORTBY_STATE_CODE_EN,
			[LOC_FR] = RSC_TASKS_SORTBY_STATE_CODE_FR
		}
	},
	[RSC_TASKS_SORTBY_RTIME] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TASKS_SORTBY_RTIME_CODE_EN,
			[LOC_FR] = RSC_TASKS_SORTBY_RTIME_CODE_FR
		}
	},
	[RSC_TASKS_SORTBY_UTIME] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TASKS_SORTBY_UTIME_CODE_EN,
			[LOC_FR] = RSC_TASKS_SORTBY_UTIME_CODE_FR
		}
	},
	[RSC_TASKS_SORTBY_STIME] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TASKS_SORTBY_STIME_CODE_EN,
			[LOC_FR] = RSC_TASKS_SORTBY_STIME_CODE_FR
		}
	},
	[RSC_TASKS_SORTBY_PID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TASKS_SORTBY_PID_CODE_EN,
			[LOC_FR] = RSC_TASKS_SORTBY_PID_CODE_FR
		}
	},
	[RSC_TASKS_SORTBY_COMM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TASKS_SORTBY_COMM_CODE_EN,
			[LOC_FR] = RSC_TASKS_SORTBY_COMM_CODE_FR
		}
	},
	[RSC_MENU_ITEM_MENU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_MENU_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_MENU_CODE_FR
		}
	},
	[RSC_MENU_ITEM_VIEW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_VIEW_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_VIEW_CODE_FR
		}
	},
	[RSC_MENU_ITEM_WINDOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_WINDOW_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_WINDOW_CODE_FR
		}
	},
	[RSC_MENU_ITEM_SETTINGS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_SETTINGS_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_SETTINGS_CODE_FR
		}
	},
	[RSC_MENU_ITEM_KERNEL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_KERNEL_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_KERNEL_CODE_FR
		}
	},
	[RSC_MENU_ITEM_HOTPLUG] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_HOTPLUG_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_HOTPLUG_CODE_FR
		}
	},
	[RSC_MENU_ITEM_TOOLS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_TOOLS_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_TOOLS_CODE_FR
		}
	},
	[RSC_MENU_ITEM_ABOUT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_ABOUT_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_ABOUT_CODE_FR
		}
	},
	[RSC_MENU_ITEM_HELP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_HELP_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_HELP_CODE_FR
		}
	},
	[RSC_MENU_ITEM_KEYS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_KEYS_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_KEYS_CODE_FR
		}
	},
	[RSC_MENU_ITEM_LANG] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_LANG_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_LANG_CODE_FR
		}
	},
	[RSC_MENU_ITEM_QUIT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_QUIT_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_QUIT_CODE_FR
		}
	},
	[RSC_MENU_ITEM_DASHBOARD] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_DASHBOARD_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_DASHBOARD_CODE_FR
		}
	},
	[RSC_MENU_ITEM_FREQUENCY] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_FREQUENCY_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_FREQUENCY_CODE_FR
		}
	},
	[RSC_MENU_ITEM_INST_CYCLES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_INST_CYCLE_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_INST_CYCLE_CODE_FR
		}
	},
	[RSC_MENU_ITEM_CORE_CYCLES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_CORE_CYCLE_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_CORE_CYCLE_CODE_FR
		}
	},
	[RSC_MENU_ITEM_IDLE_STATES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_IDLE_STATE_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_IDLE_STATE_CODE_FR
		}
	},
	[RSC_MENU_ITEM_PKG_CYCLES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_PKG_CYCLE_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_PKG_CYCLE_CODE_FR
		}
	},
	[RSC_MENU_ITEM_TASKS_MON] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_TASKS_MON_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_TASKS_MON_CODE_FR
		}
	},
	[RSC_MENU_ITEM_SYS_INTER] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_SYS_INTER_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_SYS_INTER_CODE_FR
		}
	},
	[RSC_MENU_ITEM_POW_VOLT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_POW_VOLT_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_POW_VOLT_CODE_FR
		}
	},
	[RSC_MENU_ITEM_SLICE_CTRS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_SLICE_CTR_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_SLICE_CTR_CODE_FR
		}
	},
	[RSC_MENU_ITEM_PROCESSOR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_PROCESSOR_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_PROCESSOR_CODE_FR
		}
	},
	[RSC_MENU_ITEM_TOPOLOGY] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_TOPOLOGY_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_TOPOLOGY_CODE_FR
		}
	},
	[RSC_MENU_ITEM_FEATURES] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_FEATURES_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_FEATURES_CODE_FR
		}
	},
	[RSC_MENU_ITEM_ISA_EXT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_ISA_EXT_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_ISA_EXT_CODE_FR
		}
	},
	[RSC_MENU_ITEM_TECH] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_TECH_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_TECH_CODE_FR
		}
	},
	[RSC_MENU_ITEM_PERF_MON] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_PERF_MON_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_PERF_MON_CODE_FR
		}
	},
	[RSC_MENU_ITEM_POW_THERM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_POW_THERM_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_POW_THERM_CODE_FR
		}
	},
	[RSC_MENU_ITEM_CPUID] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_CPUID_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_CPUID_CODE_FR
		}
	},
	[RSC_MENU_ITEM_SYS_REGS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_SYS_REGS_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_SYS_REGS_CODE_FR
		}
	},
	[RSC_MENU_ITEM_MEM_CTRL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_MENU_ITEM_MEM_CTRL_CODE_EN,
			[LOC_FR] = RSC_MENU_ITEM_MEM_CTRL_CODE_FR
		}
	},
	[RSC_SETTINGS_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_TITLE_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_TITLE_CODE_FR
		}
	},
	[RSC_SETTINGS_DAEMON] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_DAEMON_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_DAEMON_CODE_FR
		}
	},
	[RSC_SETTINGS_INTERVAL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_INTERVAL_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_INTERVAL_CODE_FR
		}
	},
	[RSC_SETTINGS_AUTO_CLOCK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_AUTO_CLOCK_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_AUTO_CLOCK_CODE_FR
		}
	},
	[RSC_SETTINGS_EXPERIMENTAL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_EXPERIMENTAL_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_EXPERIMENTAL_CODE_FR
		}
	},
	[RSC_SETTINGS_CPU_HOTPLUG] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_CPU_HOTPLUG_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_CPU_HOTPLUG_CODE_FR
		}
	},
	[RSC_SETTINGS_PCI_ENABLED] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_PCI_ENABLED_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_PCI_ENABLED_CODE_FR
		}
	},
	[RSC_SETTINGS_NMI_REGISTERED] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SETTINGS_NMI_REGISTERED_CODE_EN,
			[LOC_FR] = RSC_SETTINGS_NMI_REGISTERED_CODE_FR
		}
	},
	[RSC_HELP_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_TITLE_CODE_EN,
			[LOC_FR] = RSC_HELP_TITLE_CODE_FR
		}
	},
	[RSC_HELP_KEY_ESCAPE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_ESCAPE_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_ESCAPE_CODE_FR
		}
	},
	[RSC_HELP_KEY_SHIFT_TAB] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_SHIFT_TAB_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_SHIFT_TAB_CODE_FR
		}
	},
	[RSC_HELP_KEY_TAB] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_TAB_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_TAB_CODE_FR
		}
	},
	[RSC_HELP_KEY_UP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_UP_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_UP_CODE_FR
		}
	},
	[RSC_HELP_KEY_LEFT_RIGHT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_LEFT_RIGHT_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_LEFT_RIGHT_CODE_FR
		}
	},
	[RSC_HELP_KEY_DOWN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_DOWN_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_DOWN_CODE_FR
		}
	},
	[RSC_HELP_KEY_END] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_END_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_END_CODE_FR
		}
	},
	[RSC_HELP_KEY_HOME] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_HOME_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_HOME_CODE_FR
		}
	},
	[RSC_HELP_KEY_ENTER] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_ENTER_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_ENTER_CODE_FR
		}
	},
	[RSC_HELP_KEY_PAGE_UP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_PAGE_UP_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_PAGE_UP_CODE_FR
		}
	},
	[RSC_HELP_KEY_PAGE_DOWN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_PAGE_DOWN_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_PAGE_DOWN_CODE_FR
		}
	},
	[RSC_HELP_KEY_MINUS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_MINUS_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_MINUS_CODE_FR
		}
	},
	[RSC_HELP_KEY_PLUS] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_KEY_PLUS_CODE_EN,
			[LOC_FR] = RSC_HELP_KEY_PLUS_CODE_FR
		}
	},
	[RSC_HELP_BLANK] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = Rsc_CreateHelp_Blank_Code,
			[LOC_FR] = Rsc_CreateHelp_Blank_Code
		}
	},
	[RSC_HELP_MENU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_MENU_CODE_EN,
			[LOC_FR] = RSC_HELP_MENU_CODE_FR
		}
	},
	[RSC_HELP_CLOSE_WINDOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_CLOSE_WINDOW_CODE_EN,
			[LOC_FR] = RSC_HELP_CLOSE_WINDOW_CODE_FR
		}
	},
	[RSC_HELP_PREV_WINDOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_PREV_WINDOW_CODE_EN,
			[LOC_FR] = RSC_HELP_PREV_WINDOW_CODE_FR
		}
	},
	[RSC_HELP_NEXT_WINDOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_NEXT_WINDOW_CODE_EN,
			[LOC_FR] = RSC_HELP_NEXT_WINDOW_CODE_FR
		}
	},
	[RSC_HELP_MOVE_WINDOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_MOVE_WINDOW_CODE_EN,
			[LOC_FR] = RSC_HELP_MOVE_WINDOW_CODE_FR
		}
	},
	[RSC_HELP_MOVE_SELECT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_MOVE_SELECT_CODE_EN,
			[LOC_FR] = RSC_HELP_MOVE_SELECT_CODE_FR
		}
	},
	[RSC_HELP_LAST_CELL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_LAST_CELL_CODE_EN,
			[LOC_FR] = RSC_HELP_LAST_CELL_CODE_FR
		}
	},
	[RSC_HELP_FIRST_CELL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_FIRST_CELL_CODE_EN,
			[LOC_FR] = RSC_HELP_FIRST_CELL_CODE_FR
		}
	},
	[RSC_HELP_TRIGGER_SELECT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_TRIGGER_SELECT_CODE_EN,
			[LOC_FR] = RSC_HELP_TRIGGER_SELECT_CODE_FR
		}
	},
	[RSC_HELP_PREV_PAGE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_PREV_PAGE_CODE_EN,
			[LOC_FR] = RSC_HELP_PREV_PAGE_CODE_FR
		}
	},
	[RSC_HELP_NEXT_PAGE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_NEXT_PAGE_CODE_EN,
			[LOC_FR] = RSC_HELP_NEXT_PAGE_CODE_FR
		}
	},
	[RSC_HELP_SCROLL_DOWN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_SCROLL_DOWN_CODE_EN,
			[LOC_FR] = RSC_HELP_SCROLL_DOWN_CODE_FR
		}
	},
	[RSC_HELP_SCROLL_UP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_HELP_SCROLL_UP_CODE_EN,
			[LOC_FR] = RSC_HELP_SCROLL_UP_CODE_FR
		}
	},
	[RSC_ADV_HELP_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_TITLE_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_TITLE_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_1_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_1_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_2_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_2_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_3] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_3_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_3_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_4] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_4_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_4_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_5] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_5_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_5_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_6] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_6_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_6_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_7] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_7_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_7_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_8] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_8_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_8_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_9] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_9_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_9_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_10] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_10_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_10_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_11] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_11_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_11_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_12] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_12_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_12_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_13] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_13_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_13_CODE_FR
		}
	},
	[RSC_ADV_HELP_ITEM_14] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ADV_HELP_ITEM_14_CODE_EN,
			[LOC_FR] = RSC_ADV_HELP_ITEM_14_CODE_FR
		}
	},
	[RSC_TURBO_CLOCK_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_TURBO_CLOCK_TITLE_CODE_EN,
			[LOC_FR] = RSC_TURBO_CLOCK_TITLE_CODE_FR
		}
	},
	[RSC_RATIO_CLOCK_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_RATIO_CLOCK_TITLE_CODE_EN,
			[LOC_FR] = RSC_RATIO_CLOCK_TITLE_CODE_FR
		}
	},
	[RSC_UNCORE_CLOCK_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_UNCORE_CLOCK_TITLE_CODE_EN,
			[LOC_FR] = RSC_UNCORE_CLOCK_TITLE_CODE_FR
		}
	},
	[RSC_SELECT_CPU_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_SELECT_CPU_TITLE_CODE_EN,
			[LOC_FR] = RSC_SELECT_CPU_TITLE_CODE_FR
		}
	},
	[RSC_BOX_DISABLE_COND0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_DISABLE_COND0_CODE_EN,
			[LOC_FR] = RSC_BOX_DISABLE_COND0_CODE_FR
		}
	},
	[RSC_BOX_DISABLE_COND1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_DISABLE_COND1_CODE_EN,
			[LOC_FR] = RSC_BOX_DISABLE_COND1_CODE_FR
		}
	},
	[RSC_BOX_ENABLE_COND0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_ENABLE_COND0_CODE_EN,
			[LOC_FR] = RSC_BOX_ENABLE_COND0_CODE_FR
		}
	},
	[RSC_BOX_ENABLE_COND1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_ENABLE_COND1_CODE_EN,
			[LOC_FR] = RSC_BOX_ENABLE_COND1_CODE_FR
		}
	},
	[RSC_BOX_INTERVAL_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_INTERVAL_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_INTERVAL_TITLE_CODE_FR
		}
	},
	[RSC_BOX_AUTO_CLOCK_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_AUTOCLOCK_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_AUTOCLOCK_TITLE_CODE_FR
		}
	},
	[RSC_BOX_MODE_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_MODE_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_MODE_TITLE_CODE_FR
		}
	},
	[RSC_BOX_MODE_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_MODE_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_MODE_DESC_CODE_FR
		}
	},
	[RSC_BOX_EIST_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EIST_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_EIST_DESC_CODE_FR
		}
	},
	[RSC_BOX_C1E_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_C1E_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_C1E_DESC_CODE_FR
		}
	},
	[RSC_BOX_TURBO_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TURBO_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_TURBO_DESC_CODE_FR
		}
	},
	[RSC_BOX_C1A_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_C1A_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_C1A_DESC_CODE_FR
		}
	},
	[RSC_BOX_C3A_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_C3A_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_C3A_DESC_CODE_FR
		}
	},
	[RSC_BOX_C1U_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_C1U_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_C1U_DESC_CODE_FR
		}
	},
	[RSC_BOX_C3U_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_C3U_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_C3U_DESC_CODE_FR
		}
	},
	[RSC_BOX_CC6_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CC6_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_CC6_DESC_CODE_FR
		}
	},
	[RSC_BOX_PC6_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_PC6_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_PC6_DESC_CODE_FR
		}
	},
	[RSC_BOX_HWP_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_HWP_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_HWP_DESC_CODE_FR
		}
	},
	[RSC_BOX_BLANK_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_BLANK_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_BLANK_DESC_CODE_FR
		}
	},
	[RSC_BOX_NOMINAL_MODE_COND0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_NOMINAL_MODE_COND0_CODE_EN,
			[LOC_FR] = RSC_BOX_NOMINAL_MODE_COND0_CODE_FR
		}
	},
	[RSC_BOX_NOMINAL_MODE_COND1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_NOMINAL_MODE_COND1_CODE_EN,
			[LOC_FR] = RSC_BOX_NOMINAL_MODE_COND1_CODE_FR
		}
	},
	[RSC_BOX_EXPERIMENT_MODE_COND0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EXPERIMENT_MODE_COND0_CODE_EN,
			[LOC_FR] = RSC_BOX_EXPERIMENT_MODE_COND0_CODE_FR
		}
	},
	[RSC_BOX_EXPERIMENT_MODE_COND1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EXPERIMENT_MODE_COND1_CODE_EN,
			[LOC_FR] = RSC_BOX_EXPERIMENT_MODE_COND1_CODE_FR
		}
	},
	[RSC_BOX_INTERRUPT_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_INTERRUPT_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_INTERRUPT_TITLE_CODE_FR
		}
	},
	[RSC_BOX_INT_REGISTER_COND0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_INT_REGISTER_COND0_CODE_EN,
			[LOC_FR] = RSC_BOX_INT_REGISTER_COND0_CODE_FR
		}
	},
	[RSC_BOX_INT_REGISTER_COND1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_INT_REGISTER_COND1_CODE_EN,
			[LOC_FR] = RSC_BOX_INT_REGISTER_COND1_CODE_FR
		}
	},
	[RSC_BOX_INT_UNREGISTER_COND0] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_INT_UNREGISTER_COND0_CODE_EN,
			[LOC_FR] = RSC_BOX_INT_UNREGISTER_COND0_CODE_FR
		}
	},
	[RSC_BOX_INT_UNREGISTER_COND1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_INT_UNREGISTER_COND1_CODE_EN,
			[LOC_FR] = RSC_BOX_INT_UNREGISTER_COND1_CODE_FR
		}
	},
	[RSC_BOX_EVENT_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_TITLE_CODE_FR
		}
	},
	[RSC_BOX_EVENT_THERMAL_SENSOR] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_THERMAL_SENSOR_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_THERMAL_SENSOR_CODE_FR
		}
	},
	[RSC_BOX_EVENT_PROCHOT_AGENT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_PROCHOT_AGENT_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_PROCHOT_AGENT_CODE_FR
		}
	},
	[RSC_BOX_EVENT_CRITICAL_TEMP] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_CRITICAL_TEMP_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_CRITICAL_TEMP_CODE_FR
		}
	},
	[RSC_BOX_EVENT_THERMAL_THRESHOLD] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_THERMAL_THRESHOLD_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_THERMAL_THRESHOLD_CODE_FR
		}
	},
	[RSC_BOX_EVENT_POWER_LIMITATION] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_POWER_LIMITATION_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_POWER_LIMITATION_CODE_FR
		}
	},
	[RSC_BOX_EVENT_CURRENT_LIMITATION] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_CURRENT_LIMITATION_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_CURRENT_LIMITATION_CODE_FR
		}
	},
	[RSC_BOX_EVENT_CROSS_DOMAIN_LIMIT] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EVENT_CROSS_DOMAIN_LIMIT_CODE_EN,
			[LOC_FR] = RSC_BOX_EVENT_CROSS_DOMAIN_LIMIT_CODE_FR
		}
	},
	[RSC_BOX_PACKAGE_STATE_LIMIT_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_PKG_STATE_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_PKG_STATE_TITLE_CODE_FR
		}
	},
	[RSC_BOX_IO_MWAIT_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_IO_MWAIT_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_IO_MWAIT_TITLE_CODE_FR
		}
	},
	[RSC_BOX_IO_MWAIT_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_IO_MWAIT_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_IO_MWAIT_DESC_CODE_FR
		}
	},
	[RSC_BOX_MWAIT_MAX_STATE_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_FR
		}
	},
	[RSC_BOX_ODCM_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_ODCM_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_ODCM_TITLE_CODE_FR
		}
	},
	[RSC_BOX_ODCM_DESC] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_ODCM_DESC_CODE_EN,
			[LOC_FR] = RSC_BOX_ODCM_DESC_CODE_FR
		}
	},
	[RSC_BOX_EXTENDED_DUTY_CYCLE_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_FR
		}
	},
	[RSC_BOX_DUTY_CYCLE_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_DUTY_CYCLE_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_DUTY_CYCLE_TITLE_CODE_FR
		}
	},
	[RSC_BOX_DUTY_CYCLE_RESERVED] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_DUTY_CYCLE_RESERVED_CODE_EN,
			[LOC_FR] = RSC_BOX_DUTY_CYCLE_RESERVED_CODE_FR
		}
	},
	[RSC_BOX_POWER_POLICY_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_POWER_POLICY_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_POWER_POLICY_TITLE_CODE_FR
		}
	},
	[RSC_BOX_POWER_POLICY_LOW] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_POWER_POLICY_LOW_CODE_EN,
			[LOC_FR] = RSC_BOX_POWER_POLICY_LOW_CODE_FR
		}
	},
	[RSC_BOX_POWER_POLICY_HIGH] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_POWER_POLICY_HIGH_CODE_EN,
			[LOC_FR] = RSC_BOX_POWER_POLICY_HIGH_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_TITLE_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_STOP_BURN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_STOP_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_STOP_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_ATOMIC_BURN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_ATOMIC_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_ATOMIC_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_CRC32_BURN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_CRC32_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_CRC32_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_CONIC_BURN] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_CONIC_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_CONIC_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_RANDOM_CPU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_RAND_CPU_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_RAND_CPU_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_ROUND_ROBIN_CPU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_RR_CPU_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_RR_CPU_CODE_FR
		}
	},
	[RSC_BOX_TOOLS_USER_CPU] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_TOOLS_USR_CPU_CODE_EN,
			[LOC_FR] = RSC_BOX_TOOLS_USR_CPU_CODE_FR
		}
	},
	[RSC_BOX_CONIC_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_TITLE_CODE_FR
		}
	},
	[RSC_BOX_CONIC_ITEM_1] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_ITEM_1_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_ITEM_1_CODE_FR
		}
	},
	[RSC_BOX_CONIC_ITEM_2] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_ITEM_2_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_ITEM_2_CODE_FR
		}
	},
	[RSC_BOX_CONIC_ITEM_3] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_ITEM_3_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_ITEM_3_CODE_FR
		}
	},
	[RSC_BOX_CONIC_ITEM_4] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_ITEM_4_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_ITEM_4_CODE_FR
		}
	},
	[RSC_BOX_CONIC_ITEM_5] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_ITEM_5_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_ITEM_5_CODE_FR
		}
	},
	[RSC_BOX_CONIC_ITEM_6] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_CONIC_ITEM_6_CODE_EN,
			[LOC_FR] = RSC_BOX_CONIC_ITEM_6_CODE_FR
		}
	},
	[RSC_BOX_LANG_TITLE] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_LANG_TITLE_CODE_EN,
			[LOC_FR] = RSC_BOX_LANG_TITLE_CODE_FR
		}
	},
	[RSC_BOX_LANG_ENGLISH] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_LANG_ENGLISH_CODE_EN,
			[LOC_FR] = RSC_BOX_LANG_ENGLISH_CODE_FR
		}
	},
	[RSC_BOX_LANG_FRENCH] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_BOX_LANG_FRENCH_CODE_EN,
			[LOC_FR] = RSC_BOX_LANG_FRENCH_CODE_FR
		}
	},
	[RSC_ERROR_CMD_SYNTAX] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ERROR_CMD_SYNTAX_CODE_EN,
			[LOC_FR] = RSC_ERROR_CMD_SYNTAX_CODE_FR
		}
	},
	[RSC_ERROR_SHARED_MEM] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ERROR_SHARED_MEM_CODE_EN,
			[LOC_FR] = RSC_ERROR_SHARED_MEM_CODE_FR
		}
	},
	[RSC_ERROR_SYS_CALL] = {
		.Attr = vColor,
		.Code = {
			[LOC_EN] = RSC_ERROR_SYS_CALL_CODE_EN,
			[LOC_FR] = RSC_ERROR_SYS_CALL_CODE_FR
		}
	}
};
