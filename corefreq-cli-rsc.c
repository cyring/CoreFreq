/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <string.h>
#include <locale.h>
#include <sched.h>
#include <errno.h>

#include "bitasm.h"
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
ASCII Rsc_Layout_Ruler_Frequency_Code_En[] = RSC_LAYOUT_RULER_FREQUENCY_CODE;
#define Rsc_Layout_Ruler_Frequency_Code_Fr Rsc_Layout_Ruler_Frequency_Code_En

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

#define Rsc_Layout_Ruler_Inst_Attr vColor
ASCII	Rsc_Layout_Ruler_Inst_Code_En[] = RSC_LAYOUT_RULER_INST_CODE;
#define Rsc_Layout_Ruler_Inst_Code_Fr Rsc_Layout_Ruler_Inst_Code_En

#define Rsc_Layout_Ruler_Cycles_Attr vColor
ASCII	Rsc_Layout_Ruler_Cycles_Code_En[] = RSC_LAYOUT_RULER_CYCLES_CODE;
#define Rsc_Layout_Ruler_Cycles_Code_Fr Rsc_Layout_Ruler_Cycles_Code_En

#define Rsc_Layout_Ruler_CStates_Attr vColor
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
ASCII	Rsc_Layout_Package_PC_Code_En[] = RSC_LAYOUT_PACKAGE_PC_CODE;
#define Rsc_Layout_Package_PC_Code_Fr Rsc_Layout_Package_PC_Code_En

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
ASCII	Rsc_Layout_Ruler_Power_Code_En[] = RSC_LAYOUT_RULER_POWER_CODE_EN;
#define Rsc_Layout_Ruler_Power_Code_Fr Rsc_Layout_Ruler_Power_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Voltage_Attr[] = RSC_LAYOUT_RULER_VOLTAGE_ATTR;
ASCII	Rsc_Layout_Ruler_Voltage_Code_En[] = RSC_LAYOUT_RULER_VOLTAGE_CODE_EN;
#define Rsc_Layout_Ruler_Voltage_Code_Fr Rsc_Layout_Ruler_Voltage_Code_En

ATTRIBUTE Rsc_Layout_Ruler_Energy_Attr[] = RSC_LAYOUT_RULER_ENERGY_ATTR;
ASCII	Rsc_Layout_Ruler_Energy_Code_En[] = RSC_LAYOUT_RULER_ENERGY_CODE_EN,
	Rsc_Layout_Ruler_Energy_Code_Fr[] = RSC_LAYOUT_RULER_ENERGY_CODE_FR;

ATTRIBUTE Rsc_Layout_Ruler_Slice_Attr[] = RSC_LAYOUT_RULER_SLICE_ATTR;
ASCII	Rsc_Layout_Ruler_Slice_Code_En[] = RSC_LAYOUT_RULER_SLICE_CODE_EN,
	Rsc_Layout_Ruler_Slice_Code_Fr[] = RSC_LAYOUT_RULER_SLICE_CODE_FR;

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

ATTRIBUTE Rsc_SysInfoPwrThermal_Cond_Attr[5][50] = {
	RSC_SYSINFO_PWR_THERMAL_COND0_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND1_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND2_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND3_ATTR,
	RSC_SYSINFO_PWR_THERMAL_COND4_ATTR
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
	Rsc_CreateHotPlugCPU_Disable_Attr[]=RSC_CREATE_HOTPLUG_CPU_DISABLE_ATTR;

ATTRIBUTE Rsc_CreateRatioClock_Cond_Attr[4][29] = {
	RSC_CREATE_RATIO_CLOCK_COND0_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND1_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND2_ATTR,
	RSC_CREATE_RATIO_CLOCK_COND3_ATTR
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

ATTRIBUTE Rsc_CreateRecorder_Attr[] = RSC_CREATE_RECORDER_ATTR;

ATTRIBUTE Rsc_SMBIOS_Item_Attr[] = RSC_SMBIOS_ITEM_ATTR;

ATTRIBUTE Rsc_CreateSelectFreq_Pkg_Attr[] = RSC_CREATE_SELECT_FREQ_PKG_ATTR;

ATTRIBUTE Rsc_CreateSelectFreq_Cond_Attr[2][46] = {
	RSC_CREATE_SELECT_FREQ_COND0_ATTR,
	RSC_CREATE_SELECT_FREQ_COND1_ATTR
};

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

#define LDS(attr_var, en_var, fr_var)					\
{									\
	LDV(attr_var, en_var, fr_var),					\
	.Size = {							\
		[LOC_EN] = __builtin_strlen(en_var),			\
		[LOC_FR] = __builtin_strlen(fr_var)			\
	}								\
}

#define LDT(en_var, fr_var)	LDS(vColor, en_var, fr_var)

RESOURCE_ST Resource[] = {
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
	[RSC_LAYOUT_RULER_LOAD] = LDA(	Rsc_Layout_Ruler_Load_Attr,
					Rsc_Layout_Ruler_Load_Code_En,
					Rsc_Layout_Ruler_Load_Code_Fr),
	[RSC_LAYOUT_RULER_REL_LOAD]=LDA(Rsc_Layout_Ruler_Rel_Load_Attr,
					Rsc_Layout_Ruler_Rel_Load_Code_En,
					Rsc_Layout_Ruler_Rel_Load_Code_Fr),
	[RSC_LAYOUT_RULER_ABS_LOAD]=LDA(Rsc_Layout_Ruler_Abs_Load_Attr,
					Rsc_Layout_Ruler_Abs_Load_Code_En,
					Rsc_Layout_Ruler_Abs_Load_Code_Fr),
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
	[RSC_LAYOUT_RULER_POWER] = LDA( Rsc_Layout_Ruler_Power_Attr,
					Rsc_Layout_Ruler_Power_Code_En,
					Rsc_Layout_Ruler_Power_Code_Fr),
	[RSC_LAYOUT_RULER_VOLTAGE]=LDA( Rsc_Layout_Ruler_Voltage_Attr,
					Rsc_Layout_Ruler_Voltage_Code_En,
					Rsc_Layout_Ruler_Voltage_Code_Fr),
	[RSC_LAYOUT_RULER_ENERGY]=LDA(	Rsc_Layout_Ruler_Energy_Attr,
					Rsc_Layout_Ruler_Energy_Code_En,
					Rsc_Layout_Ruler_Energy_Code_Fr),
	[RSC_LAYOUT_RULER_SLICE] = LDA( Rsc_Layout_Ruler_Slice_Attr,
					Rsc_Layout_Ruler_Slice_Code_En,
					Rsc_Layout_Ruler_Slice_Code_Fr),
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
    [RSC_TRACKER_STATE_COLOR]	= LDB(	Rsc_SymbolTrackerColor_Attr),
    [RSC_SYSINFO_CPUID_COND0]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[0]),
    [RSC_SYSINFO_CPUID_COND1]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[1]),
    [RSC_SYSINFO_CPUID_COND2]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[2]),
    [RSC_SYSINFO_CPUID_COND3]	= LDB(	Rsc_SysInfoCPUID_Cond_Attr[3]),
    [RSC_SYSTEM_REGISTERS_COND0]= LDB(	Rsc_SystemRegisters_Cond_Attr[0]),
    [RSC_SYSTEM_REGISTERS_COND1]= LDB(	Rsc_SystemRegisters_Cond_Attr[1]),
    [RSC_SYSTEM_REGISTERS_COND2]= LDB(	Rsc_SystemRegisters_Cond_Attr[2]),
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
    [RSC_SYSINFO_ISA_COND_2_0]	= LDB(	Rsc_SysInfoISA_Cond_Attr[2][0]),
    [RSC_SYSINFO_ISA_COND_2_1]	= LDB(	Rsc_SysInfoISA_Cond_Attr[2][1]),
    [RSC_SYSINFO_ISA_COND_2_2]	= LDB(	Rsc_SysInfoISA_Cond_Attr[2][2]),
    [RSC_SYSINFO_ISA_COND_2_3]	= LDB(	Rsc_SysInfoISA_Cond_Attr[2][3]),
    [RSC_SYSINFO_ISA_COND_2_4]	= LDB(	Rsc_SysInfoISA_Cond_Attr[2][4]),
    [RSC_SYSINFO_ISA_COND_3_0]	= LDB(	Rsc_SysInfoISA_Cond_Attr[3][0]),
    [RSC_SYSINFO_ISA_COND_3_1]	= LDB(	Rsc_SysInfoISA_Cond_Attr[3][1]),
    [RSC_SYSINFO_ISA_COND_3_2]	= LDB(	Rsc_SysInfoISA_Cond_Attr[3][2]),
    [RSC_SYSINFO_ISA_COND_3_3]	= LDB(	Rsc_SysInfoISA_Cond_Attr[3][3]),
    [RSC_SYSINFO_ISA_COND_3_4]	= LDB(	Rsc_SysInfoISA_Cond_Attr[3][4]),
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
    [RSC_SYSINFO_KERNEL]	= LDB(	Rsc_SysInfoKernel_Attr),
    [RSC_TOPOLOGY_COND0]	= LDB(	Rsc_Topology_Cond_Attr[0]),
    [RSC_TOPOLOGY_COND1]	= LDB(	Rsc_Topology_Cond_Attr[1]),
    [RSC_TOPOLOGY_COND2]	= LDB(	Rsc_Topology_Cond_Attr[2]),
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
    [RSC_CREATE_ADV_HELP_COND0] = LDS(	Rsc_CreateAdvHelp_Cond_Attr[0],
					RSC_CREATE_ADV_HELP_BLANK_CODE,
					RSC_CREATE_ADV_HELP_BLANK_CODE),
    [RSC_CREATE_ADV_HELP_COND1] = LDS(	Rsc_CreateAdvHelp_Cond_Attr[1],
					RSC_CREATE_ADV_HELP_BLANK_CODE,
					RSC_CREATE_ADV_HELP_BLANK_CODE),
    [RSC_CREATE_HOTPLUG_CPU_ENABLE]=LDA(Rsc_CreateHotPlugCPU_Enable_Attr,
					RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_EN,
					RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_FR),
   [RSC_CREATE_HOTPLUG_CPU_DISABLE]=LDA(Rsc_CreateHotPlugCPU_Disable_Attr,
					RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_EN,
					RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_FR),
    [RSC_CREATE_RATIO_CLOCK_COND0]=LDB( Rsc_CreateRatioClock_Cond_Attr[0]),
    [RSC_CREATE_RATIO_CLOCK_COND1]=LDB( Rsc_CreateRatioClock_Cond_Attr[1]),
    [RSC_CREATE_RATIO_CLOCK_COND2]=LDB( Rsc_CreateRatioClock_Cond_Attr[2]),
    [RSC_CREATE_RATIO_CLOCK_COND3]=LDB( Rsc_CreateRatioClock_Cond_Attr[3]),
    [RSC_CREATE_SELECT_CPU_COND0] = LDB(Rsc_CreateSelectCPU_Cond_Attr[0]),
    [RSC_CREATE_SELECT_CPU_COND1] = LDB(Rsc_CreateSelectCPU_Cond_Attr[1]),
    [RSC_HOT_EVENT_COND0]	= LDB(	Rsc_HotEvent_Cond_Attr[0]),
    [RSC_HOT_EVENT_COND1]	= LDB(	Rsc_HotEvent_Cond_Attr[1]),
    [RSC_HOT_EVENT_COND2]	= LDB(	Rsc_HotEvent_Cond_Attr[2]),
    [RSC_HOT_EVENT_COND3]	= LDB(	Rsc_HotEvent_Cond_Attr[3]),
    [RSC_HOT_EVENT_COND4]	= LDB(	Rsc_HotEvent_Cond_Attr[4]),
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
	[RSC_ISA_TITLE] 	= LDT(	RSC_ISA_TITLE_CODE_EN,
					RSC_ISA_TITLE_CODE_FR),
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
	[RSC_FEATURES_LM]	= LDT(	RSC_FEATURES_LM_CODE_EN,
					RSC_FEATURES_LM_CODE_FR),
	[RSC_FEATURES_LWP]	= LDT(	RSC_FEATURES_LWP_CODE_EN,
					RSC_FEATURES_LWP_CODE_FR),
	[RSC_FEATURES_MCA]	= LDT(	RSC_FEATURES_MCA_CODE_EN,
					RSC_FEATURES_MCA_CODE_FR),
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
	[RSC_FEATURES_RTM]	= LDT(	RSC_FEATURES_RTM_CODE_EN,
					RSC_FEATURES_RTM_CODE_FR),
	[RSC_FEATURES_SMX]	= LDT(	RSC_FEATURES_SMX_CODE_EN,
					RSC_FEATURES_SMX_CODE_FR),
	[RSC_FEATURES_SELF_SNOOP]=LDT(	RSC_FEATURES_SELF_SNOOP_CODE_EN,
					RSC_FEATURES_SELF_SNOOP_CODE_FR),
	[RSC_FEATURES_SMEP]	= LDT(	RSC_FEATURES_SMEP_CODE_EN,
					RSC_FEATURES_SMEP_CODE_FR),
	[RSC_FEATURES_TSC]	= LDT(	RSC_FEATURES_TSC_CODE_EN,
					RSC_FEATURES_TSC_CODE_FR),
	[RSC_FEATURES_TSC_DEADLN]=LDT(	RSC_FEATURES_TSC_DEADLN_CODE_EN,
					RSC_FEATURES_TSC_DEADLN_CODE_FR),
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
	[RSC_TECHNOLOGIES_HYPERV]=LDT(	RSC_TECHNOLOGIES_HYPERV_CODE_EN,
					RSC_TECHNOLOGIES_HYPERV_CODE_FR),
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
	[RSC_PERF_MON_C1E]	= LDT(	RSC_PERF_MON_C1E_CODE_EN,
					RSC_PERF_MON_C1E_CODE_FR),
	[RSC_PERF_MON_C1A]	= LDT(	RSC_PERF_MON_C1A_CODE_EN,
					RSC_PERF_MON_C1A_CODE_FR),
	[RSC_PERF_MON_C3A]	= LDT(	RSC_PERF_MON_C3A_CODE_EN,
					RSC_PERF_MON_C3A_CODE_FR),
	[RSC_PERF_MON_C1U]	= LDT(	RSC_PERF_MON_C1U_CODE_EN,
					RSC_PERF_MON_C1U_CODE_FR),
	[RSC_PERF_MON_C3U]	= LDT(	RSC_PERF_MON_C3U_CODE_EN,
					RSC_PERF_MON_C3U_CODE_FR),
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
	[RSC_PERF_MON_CFG_CTRL] = LDT(	RSC_PERF_MON_CFG_CTRL_CODE_EN,
					RSC_PERF_MON_CFG_CTRL_CODE_FR),
	[RSC_PERF_MON_LOW_CSTATE]=LDT(	RSC_PERF_MON_LOW_CSTATE_CODE_EN,
					RSC_PERF_MON_LOW_CSTATE_CODE_FR),
	[RSC_PERF_MON_IOMWAIT]	= LDT(	RSC_PERF_MON_IOMWAIT_CODE_EN,
					RSC_PERF_MON_IOMWAIT_CODE_FR),
	[RSC_PERF_MON_MAX_CSTATE]=LDT(	RSC_PERF_MON_MAX_CSTATE_CODE_EN,
					RSC_PERF_MON_MAX_CSTATE_CODE_FR),
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
	[RSC_KERNEL_TITLE]	= LDT(	RSC_KERNEL_TITLE_CODE_EN,
					RSC_KERNEL_TITLE_CODE_FR),
	[RSC_KERNEL_TOTAL_RAM]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_TOTAL_RAM_CODE_EN,
					RSC_KERNEL_TOTAL_RAM_CODE_FR),
	[RSC_KERNEL_SHARED_RAM] = LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_SHARED_RAM_CODE_EN,
					RSC_KERNEL_SHARED_RAM_CODE_FR),
	[RSC_KERNEL_FREE_RAM]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_FREE_RAM_CODE_EN,
					RSC_KERNEL_FREE_RAM_CODE_FR),
	[RSC_KERNEL_BUFFER_RAM] = LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_BUFFER_RAM_CODE_EN,
					RSC_KERNEL_BUFFER_RAM_CODE_FR),
	[RSC_KERNEL_TOTAL_HIGH] = LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_TOTAL_HIGH_CODE_EN,
					RSC_KERNEL_TOTAL_HIGH_CODE_FR),
	[RSC_KERNEL_FREE_HIGH]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_FREE_HIGH_CODE_EN,
					RSC_KERNEL_FREE_HIGH_CODE_FR),
	[RSC_KERNEL_GOVERNOR]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_GOVERNOR_CODE_EN,
					RSC_KERNEL_GOVERNOR_CODE_FR),
	[RSC_KERNEL_FREQ_DRIVER]= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_FREQ_DRIVER_CODE_EN,
					RSC_KERNEL_FREQ_DRIVER_CODE_FR),
	[RSC_KERNEL_IDLE_DRIVER]= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_IDLE_DRIVER_CODE_EN,
					RSC_KERNEL_IDLE_DRIVER_CODE_FR),
	[RSC_KERNEL_RELEASE]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_RELEASE_CODE_EN,
					RSC_KERNEL_RELEASE_CODE_FR),
	[RSC_KERNEL_VERSION]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_VERSION_CODE_EN,
					RSC_KERNEL_VERSION_CODE_FR),
	[RSC_KERNEL_MACHINE]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_MACHINE_CODE_EN,
					RSC_KERNEL_MACHINE_CODE_FR),
	[RSC_KERNEL_MEMORY]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_MEMORY_CODE_EN,
					RSC_KERNEL_MEMORY_CODE_FR),
	[RSC_KERNEL_STATE]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_STATE_CODE_EN,
					RSC_KERNEL_STATE_CODE_FR),
	[RSC_KERNEL_POWER]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_POWER_CODE_EN,
					RSC_KERNEL_POWER_CODE_FR),
	[RSC_KERNEL_LATENCY]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_LATENCY_CODE_EN,
					RSC_KERNEL_LATENCY_CODE_FR),
	[RSC_KERNEL_RESIDENCY]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_RESIDENCY_CODE_EN,
					RSC_KERNEL_RESIDENCY_CODE_FR),
	[RSC_KERNEL_LIMIT]	= LDA(	Rsc_SysInfoKernel_Attr,
					RSC_KERNEL_LIMIT_CODE_EN,
					RSC_KERNEL_LIMIT_CODE_FR),
	[RSC_TOPOLOGY_TITLE]	= LDT(	RSC_TOPOLOGY_TITLE_CODE_EN,
					RSC_TOPOLOGY_TITLE_CODE_FR),
	[RSC_MEM_CTRL_TITLE]	= LDT(	RSC_MEM_CTRL_TITLE_CODE_EN,
					RSC_MEM_CTRL_TITLE_CODE_FR),
	[RSC_MEM_CTRL_BLANK]	= LDT(	RSC_MEM_CTRL_BLANK_CODE_EN,
					RSC_MEM_CTRL_BLANK_CODE_FR),
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
	[RSC_MENU_ITEM_MENU]	= LDA(	Rsc_CreateMenu_Menu_Attr,
					RSC_MENU_ITEM_MENU_CODE_EN,
					RSC_MENU_ITEM_MENU_CODE_FR),
	[RSC_MENU_ITEM_VIEW]	= LDA(	Rsc_CreateMenu_View_Attr,
					RSC_MENU_ITEM_VIEW_CODE_EN,
					RSC_MENU_ITEM_VIEW_CODE_FR),
	[RSC_MENU_ITEM_WINDOW]	= LDA(	Rsc_CreateMenu_Window_Attr,
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
    [RSC_SETTINGS_THERMAL_SCOPE] = LDT( RSC_SETTINGS_THERMAL_SCOPE_CODE_EN,
					RSC_SETTINGS_THERMAL_SCOPE_CODE_FR),
    [RSC_SETTINGS_VOLTAGE_SCOPE] = LDT( RSC_SETTINGS_VOLTAGE_SCOPE_CODE_EN,
					RSC_SETTINGS_VOLTAGE_SCOPE_CODE_FR),
	[RSC_SETTINGS_POWER_SCOPE] =LDT(RSC_SETTINGS_POWER_SCOPE_CODE_EN,
					RSC_SETTINGS_POWER_SCOPE_CODE_FR),
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
	[RSC_HELP_BLANK]	= LDS(	vColor,
					RSC_CREATE_HELP_BLANK_CODE,
					RSC_CREATE_HELP_BLANK_CODE),
	[RSC_HELP_MENU] 	= LDT(	RSC_HELP_MENU_CODE_EN,
					RSC_HELP_MENU_CODE_FR),
	[RSC_HELP_CLOSE_WINDOW] = LDT(	RSC_HELP_CLOSE_WINDOW_CODE_EN,
					RSC_HELP_CLOSE_WINDOW_CODE_FR),
	[RSC_HELP_PREV_WINDOW]	= LDT(	RSC_HELP_PREV_WINDOW_CODE_EN,
					RSC_HELP_PREV_WINDOW_CODE_FR),
	[RSC_HELP_NEXT_WINDOW]	= LDT(	RSC_HELP_NEXT_WINDOW_CODE_EN,
					RSC_HELP_NEXT_WINDOW_CODE_FR),
	[RSC_HELP_MOVE_WINDOW]	= LDT(	RSC_HELP_MOVE_WINDOW_CODE_EN,
					RSC_HELP_MOVE_WINDOW_CODE_FR),
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
	[RSC_BOX_AUTO_CLOCK_TITLE]=LDT( RSC_BOX_AUTOCLOCK_TITLE_CODE_EN,
					RSC_BOX_AUTOCLOCK_TITLE_CODE_FR),
	[RSC_BOX_MODE_TITLE]	= LDT(	RSC_BOX_MODE_TITLE_CODE_EN,
					RSC_BOX_MODE_TITLE_CODE_FR),
	[RSC_BOX_MODE_DESC]	= LDT(	RSC_BOX_MODE_DESC_CODE_EN,
					RSC_BOX_MODE_DESC_CODE_FR),
	[RSC_BOX_EIST_DESC]	= LDT(	RSC_BOX_EIST_DESC_CODE_EN,
					RSC_BOX_EIST_DESC_CODE_FR),
	[RSC_BOX_C1E_DESC]	= LDT(	RSC_BOX_C1E_DESC_CODE_EN,
					RSC_BOX_C1E_DESC_CODE_FR),
	[RSC_BOX_TURBO_DESC]	= LDT(	RSC_BOX_TURBO_DESC_CODE_EN,
					RSC_BOX_TURBO_DESC_CODE_FR),
	[RSC_BOX_C1A_DESC]	= LDT(	RSC_BOX_C1A_DESC_CODE_EN,
					RSC_BOX_C1A_DESC_CODE_FR),
	[RSC_BOX_C3A_DESC]	= LDT(	RSC_BOX_C3A_DESC_CODE_EN,
					RSC_BOX_C3A_DESC_CODE_FR),
	[RSC_BOX_C1U_DESC]	= LDT(	RSC_BOX_C1U_DESC_CODE_EN,
					RSC_BOX_C1U_DESC_CODE_FR),
	[RSC_BOX_C3U_DESC]	= LDT(	RSC_BOX_C3U_DESC_CODE_EN,
					RSC_BOX_C3U_DESC_CODE_FR),
	[RSC_BOX_CC6_DESC]	= LDT(	RSC_BOX_CC6_DESC_CODE_EN,
					RSC_BOX_CC6_DESC_CODE_FR),
	[RSC_BOX_PC6_DESC]	= LDT(	RSC_BOX_PC6_DESC_CODE_EN,
					RSC_BOX_PC6_DESC_CODE_FR),
	[RSC_BOX_HWP_DESC]	= LDT(	RSC_BOX_HWP_DESC_CODE_EN,
					RSC_BOX_HWP_DESC_CODE_FR),
	[RSC_BOX_BLANK_DESC]	= LDT(	RSC_BOX_BLANK_DESC_CODE_EN,
					RSC_BOX_BLANK_DESC_CODE_FR),
    [RSC_BOX_NOMINAL_MODE_COND0]= LDT(	RSC_BOX_NOMINAL_MODE_COND0_CODE_EN,
					RSC_BOX_NOMINAL_MODE_COND0_CODE_FR),
    [RSC_BOX_NOMINAL_MODE_COND1]= LDT(	RSC_BOX_NOMINAL_MODE_COND1_CODE_EN,
					RSC_BOX_NOMINAL_MODE_COND1_CODE_FR),
    [RSC_BOX_EXPERIMENT_MODE_COND0]=LDT(RSC_BOX_EXPER_MODE_COND0_CODE_EN,
					RSC_BOX_EXPER_MODE_COND0_CODE_FR),
    [RSC_BOX_EXPERIMENT_MODE_COND1]=LDT(RSC_BOX_EXPER_MODE_COND1_CODE_EN,
					RSC_BOX_EXPER_MODE_COND1_CODE_FR),
	[RSC_BOX_INTERRUPT_TITLE]=LDT(	RSC_BOX_INTERRUPT_TITLE_CODE_EN,
					RSC_BOX_INTERRUPT_TITLE_CODE_FR),
    [RSC_BOX_INT_REGISTER_COND0]= LDT(	RSC_BOX_INT_REGISTER_ST0_CODE_EN,
					RSC_BOX_INT_REGISTER_ST0_CODE_FR),
    [RSC_BOX_INT_REGISTER_COND1]= LDT(	RSC_BOX_INT_REGISTER_ST1_CODE_EN,
					RSC_BOX_INT_REGISTER_ST1_CODE_FR),
    [RSC_BOX_INT_UNREGISTER_COND0]=LDT( RSC_BOX_INT_UNREGISTER_ST0_CODE_EN,
					RSC_BOX_INT_UNREGISTER_ST0_CODE_FR),
    [RSC_BOX_INT_UNREGISTER_COND1]=LDT( RSC_BOX_INT_UNREGISTER_ST1_CODE_EN,
					RSC_BOX_INT_UNREGISTER_ST1_CODE_FR),
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
    [RSC_BOX_PKG_STATE_LIMIT_TITLE]=LDT(RSC_BOX_PKG_STATE_TITLE_CODE_EN,
					RSC_BOX_PKG_STATE_TITLE_CODE_FR),
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
    [RSC_BOX_POWER_POLICY_TITLE]= LDT(	RSC_BOX_POWER_POLICY_TITLE_CODE_EN,
					RSC_BOX_POWER_POLICY_TITLE_CODE_FR),
	[RSC_BOX_POWER_POLICY_LOW]=LDT( RSC_BOX_POWER_POLICY_LOW_CODE_EN,
					RSC_BOX_POWER_POLICY_LOW_CODE_FR),
	[RSC_BOX_POWER_POLICY_HIGH]=LDT(RSC_BOX_POWER_POLICY_HIGH_CODE_EN,
					RSC_BOX_POWER_POLICY_HIGH_CODE_FR),
	[RSC_BOX_HWP_POLICY_MIN]= LDT(	RSC_BOX_HWP_POLICY_MIN_CODE_EN,
					RSC_BOX_HWP_POLICY_MIN_CODE_FR),
	[RSC_BOX_HWP_POLICY_MED]= LDT(	RSC_BOX_HWP_POLICY_MED_CODE_EN,
					RSC_BOX_HWP_POLICY_MED_CODE_FR),
	[RSC_BOX_HWP_POLICY_PWR]= LDT(	RSC_BOX_HWP_POLICY_PWR_CODE_EN,
					RSC_BOX_HWP_POLICY_PWR_CODE_FR),
	[RSC_BOX_HWP_POLICY_MAX]= LDT(	RSC_BOX_HWP_POLICY_MAX_CODE_EN,
					RSC_BOX_HWP_POLICY_MAX_CODE_FR),
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
	[RSC_BOX_IDLE_LIMIT_TITLE]=LDT( RSC_BOX_IDLE_LIMIT_TITLE_CODE_EN,
					RSC_BOX_IDLE_LIMIT_TITLE_CODE_FR),
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
	[RSC_COPY_ROW_0]	= LDT(	RSC_COPY_R0,
					RSC_COPY_R0),
	[RSC_COPY_ROW_1]	= LDT(	RSC_COPY_R1,
					RSC_COPY_R1),
	[RSC_COPY_ROW_2]	= LDT(	RSC_COPY_R2,
					RSC_COPY_R2),
      [RSC_CREATE_SELECT_FREQ_TGT]=LDT( RSC_CREATE_SELECT_FREQ_TGT_CODE_EN,
					RSC_CREATE_SELECT_FREQ_TGT_CODE_FR),
  [RSC_CREATE_SELECT_FREQ_HWP_TGT]=LDT( RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_EN,
					RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_FR),
  [RSC_CREATE_SELECT_FREQ_HWP_MAX]=LDT( RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_EN,
					RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_FR),
  [RSC_CREATE_SELECT_FREQ_HWP_MIN]=LDT( RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_EN,
					RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_FR)
};

