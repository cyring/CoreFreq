/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#include <sys/types.h>

#include "corefreq-ui.h"
#include "corefreq-cli-rsc.h"
#include "corefreq-cli-rsc-en.h"
#include "corefreq-cli-rsc-fr.h"

ATTRIBUTE Layout_Header_Proc_Attr[] = LAYOUT_HEADER_PROC_ATTR;
ASCII	Layout_Header_Proc_Code_En[] = LAYOUT_HEADER_PROC_CODE_EN,
	Layout_Header_Proc_Code_Fr[] = LAYOUT_HEADER_PROC_CODE_FR;

ATTRIBUTE Layout_Header_CPU_Attr[] = LAYOUT_HEADER_CPU_ATTR;
ASCII	Layout_Header_CPU_Code_En[] = LAYOUT_HEADER_CPU_CODE;
#define Layout_Header_CPU_Code_Fr Layout_Header_CPU_Code_En

ATTRIBUTE Layout_Header_Arch_Attr[] = LAYOUT_HEADER_ARCH_ATTR;
ASCII	Layout_Header_Arch_Code_En[] = LAYOUT_HEADER_ARCH_CODE;
#define Layout_Header_Arch_Code_Fr Layout_Header_Arch_Code_En

ATTRIBUTE Layout_Header_Cache_L1_Attr[] = LAYOUT_HEADER_CACHE_L1_ATTR;
ASCII	Layout_Header_Cache_L1_Code_En[] = LAYOUT_HEADER_CACHE_L1_CODE;
#define Layout_Header_Cache_L1_Code_Fr Layout_Header_Cache_L1_Code_En

ATTRIBUTE Layout_Header_BClk_Attr[] = LAYOUT_HEADER_BCLK_ATTR;
ASCII	Layout_Header_BClk_Code_En[] = LAYOUT_HEADER_BCLK_CODE_EN,
	Layout_Header_BClk_Code_Fr[] = LAYOUT_HEADER_BCLK_CODE_FR;

ATTRIBUTE Layout_Header_Caches_Attr[] = LAYOUT_HEADER_CACHES_ATTR;
ASCII	Layout_Header_Caches_Code_En[] = LAYOUT_HEADER_CACHES_CODE;
#define Layout_Header_Caches_Code_Fr Layout_Header_Caches_Code_En

ATTRIBUTE Layout_Ruller_Load_Attr[] = LAYOUT_RULLER_LOAD_ATTR;
ASCII	Layout_Ruller_Load_Code_En[] = LAYOUT_RULLER_LOAD_CODE;
#define Layout_Ruller_Load_Code_Fr Layout_Ruller_Load_Code_En

ATTRIBUTE Layout_Monitor_Frequency_Attr[] = LAYOUT_MONITOR_FREQUENCY_ATTR;
ASCII	Layout_Monitor_Frequency_Code_En[] = LAYOUT_MONITOR_FREQUENCY_CODE;
#define Layout_Monitor_Frequency_Code_Fr Layout_Monitor_Frequency_Code_En

ATTRIBUTE Layout_Monitor_Inst_Attr[] = LAYOUT_MONITOR_INST_ATTR;
ASCII	Layout_Monitor_Inst_Code_En[] = LAYOUT_MONITOR_INST_CODE;
#define Layout_Monitor_Inst_Code_Fr Layout_Monitor_Inst_Code_En

ATTRIBUTE Layout_Monitor_Common_Attr[] = LAYOUT_MONITOR_COMMON_ATTR;
ASCII	Layout_Monitor_Common_Code_En[] = LAYOUT_MONITOR_COMMON_CODE;
#define Layout_Monitor_Common_Code_Fr Layout_Monitor_Common_Code_En

ATTRIBUTE Layout_Monitor_Tasks_Attr[] = LAYOUT_MONITOR_TASKS_ATTR;
ASCII	Layout_Monitor_Tasks_Code_En[] = LAYOUT_MONITOR_TASKS_CODE;
#define Layout_Monitor_Tasks_Code_Fr Layout_Monitor_Tasks_Code_En

ATTRIBUTE Layout_Ruller_Frequency_Attr[] = LAYOUT_RULLER_FREQUENCY_ATTR;
ASCII	Layout_Ruller_Frequency_Code_En[] = LAYOUT_RULLER_FREQUENCY_CODE;
#define Layout_Ruller_Frequency_Code_Fr Layout_Ruller_Frequency_Code_En

ATTRIBUTE Layout_Ruller_Freq_Avg_Attr[] = LAYOUT_RULLER_FREQUENCY_AVG_ATTR;
ASCII	Layout_Ruller_Freq_Avg_Code_En[] = LAYOUT_RULLER_FREQUENCY_AVG_CODE_EN,
	Layout_Ruller_Freq_Avg_Code_Fr[] = LAYOUT_RULLER_FREQUENCY_AVG_CODE_FR;

ATTRIBUTE Layout_Ruller_Freq_Pkg_Attr[] = LAYOUT_RULLER_FREQUENCY_PKG_ATTR;
ASCII	Layout_Ruller_Freq_Pkg_Code_En[] = LAYOUT_RULLER_FREQUENCY_PKG_CODE;
#define Layout_Ruller_Freq_Pkg_Code_Fr Layout_Ruller_Freq_Pkg_Code_En

ATTRIBUTE Layout_Ruller_Inst_Attr[] = {LWK};
ASCII	Layout_Ruller_Inst_Code_En[] = LAYOUT_RULLER_INST_CODE;
#define Layout_Ruller_Inst_Code_Fr Layout_Ruller_Inst_Code_En

ATTRIBUTE Layout_Ruller_Cycles_Attr[] = {LWK};
ASCII	Layout_Ruller_Cycles_Code_En[] = LAYOUT_RULLER_CYCLES_CODE;
#define Layout_Ruller_Cycles_Code_Fr Layout_Ruller_Cycles_Code_En

ATTRIBUTE Layout_Ruller_CStates_Attr[] = {LWK};
ASCII	Layout_Ruller_CStates_Code_En[] = LAYOUT_RULLER_CSTATES_CODE;
#define Layout_Ruller_CStates_Code_Fr Layout_Ruller_CStates_Code_En

ATTRIBUTE Layout_Ruller_Interrupts_Attr[] = LAYOUT_RULLER_INTERRUPTS_ATTR;
ASCII	Layout_Ruller_Interrupts_Code_En[] = LAYOUT_RULLER_INTERRUPTS_CODE;
#define Layout_Ruller_Interrupts_Code_Fr Layout_Ruller_Interrupts_Code_En

ATTRIBUTE Layout_Ruller_Package_Attr[] = {LWK};
ASCII	Layout_Ruller_Package_Code_En[] = LAYOUT_RULLER_PACKAGE_CODE_EN,
	Layout_Ruller_Package_Code_Fr[] = LAYOUT_RULLER_PACKAGE_CODE_FR;

ATTRIBUTE Layout_Package_Uncore_Attr[] = LAYOUT_PACKAGE_UNCORE_ATTR;
ASCII	Layout_Package_Uncore_Code_En[] = LAYOUT_PACKAGE_UNCORE_CODE;
#define Layout_Package_Uncore_Code_Fr Layout_Package_Uncore_Code_En

ATTRIBUTE Layout_Ruller_Tasks_Attr[] = LAYOUT_RULLER_TASKS_ATTR;
ASCII	Layout_Ruller_Tasks_Code_En[] = LAYOUT_RULLER_TASKS_CODE_EN,
	Layout_Ruller_Tasks_Code_Fr[] = LAYOUT_RULLER_TASKS_CODE_FR;

ATTRIBUTE Layout_Tasks_Tracking_Attr[] = LAYOUT_TASKS_TRACKING_ATTR;
ASCII	Layout_Tasks_Tracking_Code_En[] = LAYOUT_TASKS_TRACKING_CODE_EN,
	Layout_Tasks_Tracking_Code_Fr[] = LAYOUT_TASKS_TRACKING_CODE_FR;

ATTRIBUTE Layout_Tasks_State_Sorted_Attr[] = LAYOUT_TASKS_STATE_SORTED_ATTR;
ASCII   Layout_Tasks_State_Sorted_Code_En[] = LAYOUT_TASKS_STATE_SORTED_CODE_EN,
	Layout_Tasks_State_Sorted_Code_Fr[] = LAYOUT_TASKS_STATE_SORTED_CODE_FR;

ATTRIBUTE Layout_Tasks_RunTime_Sorted_Attr[] = LAYOUT_TASKS_RUNTIME_SORTED_ATTR;
ASCII Layout_Tasks_RunTime_Sorted_Code_En[]=LAYOUT_TASKS_RUNTIME_SORTED_CODE_EN,
      Layout_Tasks_RunTime_Sorted_Code_Fr[]=LAYOUT_TASKS_RUNTIME_SORTED_CODE_FR;

ATTRIBUTE Layout_Tasks_UsrTime_Sorted_Attr[] = LAYOUT_TASKS_USRTIME_SORTED_ATTR;
ASCII Layout_Tasks_UsrTime_Sorted_Code_En[]=LAYOUT_TASKS_USRTIME_SORTED_CODE_EN,
      Layout_Tasks_UsrTime_Sorted_Code_Fr[]=LAYOUT_TASKS_USRTIME_SORTED_CODE_FR;

ATTRIBUTE Layout_Tasks_SysTime_Sorted_Attr[] = LAYOUT_TASKS_SYSTIME_SORTED_ATTR;
ASCII Layout_Tasks_SysTime_Sorted_Code_En[]=LAYOUT_TASKS_SYSTIME_SORTED_CODE_EN,
      Layout_Tasks_SysTime_Sorted_Code_Fr[]=LAYOUT_TASKS_SYSTIME_SORTED_CODE_FR;

ATTRIBUTE Layout_Tasks_Process_Sorted_Attr[] = LAYOUT_TASKS_PROCESS_SORTED_ATTR;
ASCII Layout_Tasks_Process_Sorted_Code_En[]=LAYOUT_TASKS_PROCESS_SORTED_CODE_EN,
      Layout_Tasks_Process_Sorted_Code_Fr[]=LAYOUT_TASKS_PROCESS_SORTED_CODE_FR;

ATTRIBUTE Layout_Tasks_Command_Sorted_Attr[] = LAYOUT_TASKS_COMMAND_SORTED_ATTR;
ASCII Layout_Tasks_Command_Sorted_Code_En[]=LAYOUT_TASKS_COMMAND_SORTED_CODE_EN,
      Layout_Tasks_Command_Sorted_Code_Fr[]=LAYOUT_TASKS_COMMAND_SORTED_CODE_FR;

ATTRIBUTE \
	Layout_Tasks_Reverse_Sort_Off_Attr[]=LAYOUT_TASKS_REVERSE_SORT_OFF_ATTR;
ASCII \
  Layout_Tasks_Reverse_Sort_Off_Code_En[]=LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_EN,
  Layout_Tasks_Reverse_Sort_Off_Code_Fr[]=LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_FR;

ATTRIBUTE Layout_Tasks_Reverse_Sort_On_Attr[]=LAYOUT_TASKS_REVERSE_SORT_ON_ATTR;
ASCII \
  Layout_Tasks_Reverse_Sort_On_Code_En[]=LAYOUT_TASKS_REVERSE_SORT_ON_CODE_EN,
  Layout_Tasks_Reverse_Sort_On_Code_Fr[]=LAYOUT_TASKS_REVERSE_SORT_ON_CODE_FR;

ATTRIBUTE Layout_Tasks_Value_Switch_Attr[] = LAYOUT_TASKS_VALUE_SWITCH_ATTR;
ASCII	Layout_Tasks_Value_Switch_Code_En[] = LAYOUT_TASKS_VALUE_SWITCH_CODE_EN,
	Layout_Tasks_Value_Switch_Code_Fr[] = LAYOUT_TASKS_VALUE_SWITCH_CODE_FR;

ATTRIBUTE Layout_Ruller_Voltage_Attr[] = LAYOUT_RULLER_VOLTAGE_ATTR;
ASCII	Layout_Ruller_Voltage_Code_En[] = LAYOUT_RULLER_VOLTAGE_CODE_EN,
	Layout_Ruller_Voltage_Code_Fr[] = LAYOUT_RULLER_VOLTAGE_CODE_FR;

ATTRIBUTE Layout_Power_Monitor_Attr[] = LAYOUT_POWER_MONITOR_ATTR;
ASCII	Layout_Power_Monitor_Code_En[] = LAYOUT_POWER_MONITOR_CODE;
#define Layout_Power_Monitor_Code_Fr Layout_Power_Monitor_Code_En

ATTRIBUTE Layout_Ruller_Slice_Attr[] = LAYOUT_RULLER_SLICE_ATTR;
ASCII	Layout_Ruller_Slice_Code_En[] = LAYOUT_RULLER_SLICE_CODE;
#define Layout_Ruller_Slice_Code_Fr Layout_Ruller_Slice_Code_En

ATTRIBUTE Layout_Footer_Tech_x86_Attr[] = LAYOUT_FOOTER_TECH_X86_ATTR;
ASCII	Layout_Footer_Tech_x86_Code_En[] = LAYOUT_FOOTER_TECH_X86_CODE;
#define Layout_Footer_Tech_x86_Code_Fr Layout_Footer_Tech_x86_Code_En

ATTRIBUTE Layout_Footer_Tech_Intel_Attr[] = LAYOUT_FOOTER_TECH_INTEL_ATTR;
ASCII	Layout_Footer_Tech_Intel_Code_En[] = LAYOUT_FOOTER_TECH_INTEL_CODE;
#define Layout_Footer_Tech_Intel_Code_Fr Layout_Footer_Tech_Intel_Code_En

ATTRIBUTE Layout_Footer_Tech_AMD_Attr[] = LAYOUT_FOOTER_TECH_AMD_ATTR;
ASCII	Layout_Footer_Tech_AMD_Code_En[] = LAYOUT_FOOTER_TECH_AMD_CODE;
#define Layout_Footer_Tech_AMD_Code_Fr Layout_Footer_Tech_AMD_Code_En

ATTRIBUTE Layout_Footer_System_Attr[] = LAYOUT_FOOTER_SYSTEM_ATTR;
ASCII	Layout_Footer_System_Code_En[] = LAYOUT_FOOTER_SYSTEM_CODE_EN,
	Layout_Footer_System_Code_Fr[] = LAYOUT_FOOTER_SYSTEM_CODE_FR;

ATTRIBUTE Layout_Card_Core_Online_Attr[] = LAYOUT_CARD_CORE_ONLINE_ATTR;
ASCII	Layout_Card_Core_Online_Code_En[] = LAYOUT_CARD_CORE_ONLINE_CODE;
#define Layout_Card_Core_Online_Code_Fr Layout_Card_Core_Online_Code_En

ATTRIBUTE Layout_Card_Core_Offline_Attr[] = LAYOUT_CARD_CORE_OFFLINE_ATTR;
ASCII	Layout_Card_Core_Offline_Code_En[] = LAYOUT_CARD_CORE_OFFLINE_CODE;
#define Layout_Card_Core_Offline_Code_Fr Layout_Card_Core_Offline_Code_En

ATTRIBUTE Layout_Card_CLK_Attr[] = LAYOUT_CARD_CLK_ATTR;
ASCII	Layout_Card_CLK_Code_En[] = LAYOUT_CARD_CLK_CODE;
#define Layout_Card_CLK_Code_Fr Layout_Card_CLK_Code_En

ATTRIBUTE Layout_Card_Uncore_Attr[] = LAYOUT_CARD_UNCORE_ATTR;
ASCII	Layout_Card_Uncore_Code_En[] = LAYOUT_CARD_UNCORE_CODE;
#define Layout_Card_Uncore_Code_Fr Layout_Card_Uncore_Code_En

ATTRIBUTE Layout_Card_Bus_Attr[] = LAYOUT_CARD_BUS_ATTR;
ASCII	Layout_Card_Bus_Code_En[] = LAYOUT_CARD_BUS_CODE;
#define Layout_Card_Bus_Code_Fr Layout_Card_Bus_Code_En

ATTRIBUTE Layout_Card_MC_Attr[] = LAYOUT_CARD_MC_ATTR;
ASCII   Layout_Card_MC_Code_En[] = LAYOUT_CARD_MC_CODE;
#define Layout_Card_MC_Code_Fr Layout_Card_MC_Code_En

ATTRIBUTE Layout_Card_Load_Attr[] = LAYOUT_CARD_LOAD_ATTR;
ASCII	Layout_Card_Load_Code_En[] = LAYOUT_CARD_LOAD_CODE_EN,
	Layout_Card_Load_Code_Fr[] = LAYOUT_CARD_LOAD_CODE_FR;

ATTRIBUTE Layout_Card_Idle_Attr[] = LAYOUT_CARD_IDLE_ATTR;
ASCII	Layout_Card_Idle_Code_En[] = LAYOUT_CARD_IDLE_CODE_EN,
	Layout_Card_Idle_Code_Fr[] = LAYOUT_CARD_IDLE_CODE_FR;

ATTRIBUTE Layout_Card_RAM_Attr[] = LAYOUT_CARD_RAM_ATTR;
ASCII   Layout_Card_RAM_Code_En[] = LAYOUT_CARD_RAM_CODE;
#define Layout_Card_RAM_Code_Fr Layout_Card_RAM_Code_En

ATTRIBUTE Layout_Card_Task_Attr[] = LAYOUT_CARD_TASK_ATTR;
ASCII	Layout_Card_Task_Code_En[] = LAYOUT_CARD_TASK_CODE_EN,
	Layout_Card_Task_Code_Fr[] = LAYOUT_CARD_TASK_CODE_FR;

ATTRIBUTE	SymbolRunColor_Attr[]		= RUN_STATE_COLOR_ATTR,
		SymbolUnIntColor_Attr[] 	= UNINT_STATE_COLOR_ATTR,
		SymbolZombieColor_Attr[]	= ZOMBIE_STATE_COLOR_ATTR,
		SymbolSleepColor_Attr[] 	= SLEEP_STATE_COLOR_ATTR,
		SymbolWaitColor_Attr[]		= WAIT_STATE_COLOR_ATTR,
		SymbolOtherColor_Attr[] 	= OTHER_STATE_COLOR_ATTR,
		SymbolTrackerColor_Attr[]	= TRACKER_STATE_COLOR_ATTR;

ATTRIBUTE Win_SysInfoCPUID_Cond_Attr[4][74] = {
	WIN_SYSINFO_CPUID_COND0_ATTR,
	WIN_SYSINFO_CPUID_COND1_ATTR,
	WIN_SYSINFO_CPUID_COND2_ATTR,
	WIN_SYSINFO_CPUID_COND3_ATTR
};

ATTRIBUTE Win_SystemRegisters_Cond_Attr[3][4] = {
	WIN_SYSTEM_REGISTERS_COND0_ATTR,
	WIN_SYSTEM_REGISTERS_COND1_ATTR,
	WIN_SYSTEM_REGISTERS_COND2_ATTR
};

ATTRIBUTE Win_SysInfoProc_Cond_Attr[4][76] = {
	WIN_SYSINFO_PROC_COND0_ATTR,
	WIN_SYSINFO_PROC_COND1_ATTR,
	WIN_SYSINFO_PROC_COND2_ATTR,
	WIN_SYSINFO_PROC_COND3_ATTR
};

ATTRIBUTE Win_SysInfoISA_Cond_Attr[4][5][17] = {
	WIN_SYSINFO_ISA_COND0_ATTR,
	WIN_SYSINFO_ISA_COND1_ATTR,
	WIN_SYSINFO_ISA_COND2_ATTR,
	WIN_SYSINFO_ISA_COND3_ATTR
};

ATTRIBUTE Win_SysInfoFeatures_Cond_Attr[3][72] = {
	WIN_SYSINFO_FEATURES_COND0_ATTR,
	WIN_SYSINFO_FEATURES_COND1_ATTR,
	WIN_SYSINFO_FEATURES_COND2_ATTR
};

ATTRIBUTE Win_SysInfoTech_Cond_Attr[2][50] = {
	WIN_SYSINFO_TECH_COND0_ATTR,
	WIN_SYSINFO_TECH_COND1_ATTR
};

ATTRIBUTE Win_SysInfoPerfMon_Cond_Attr[4][74] = {
	WIN_SYSINFO_PERFMON_COND0_ATTR,
	WIN_SYSINFO_PERFMON_COND1_ATTR,
	WIN_SYSINFO_PERFMON_COND2_ATTR,
	WIN_SYSINFO_PERFMON_COND3_ATTR
};

ATTRIBUTE Win_SysInfoPwrThermal_Cond_Attr[4][50] = {
	WIN_SYSINFO_PWR_THERMAL_COND0_ATTR,
	WIN_SYSINFO_PWR_THERMAL_COND1_ATTR,
	WIN_SYSINFO_PWR_THERMAL_COND2_ATTR,
	WIN_SYSINFO_PWR_THERMAL_COND3_ATTR
};

ATTRIBUTE Win_SysInfoKernel_Attr[] = WIN_SYSINFO_KERNEL_ATTR;

ATTRIBUTE Win_Topology_Cond_Attr[3][13] = {
	WIN_TOPOLOGY_COND0_ATTR,
	WIN_TOPOLOGY_COND1_ATTR,
	WIN_TOPOLOGY_COND2_ATTR
};

ATTRIBUTE Win_MemoryController_Cond_Attr[2][14] = {
	WIN_MEMORY_CONTROLLER_COND0_ATTR,
	WIN_MEMORY_CONTROLLER_COND1_ATTR
};

RESOURCE_ST Resource[] = {
	[RSC_LAYOUT_HEADER_PROC] = {
		.Attr = Layout_Header_Proc_Attr,
		.Code = {
			[LOC_EN] = Layout_Header_Proc_Code_En,
			[LOC_FR] = Layout_Header_Proc_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_CPU] = {
		.Attr = Layout_Header_CPU_Attr,
		.Code = {
			[LOC_EN] = Layout_Header_CPU_Code_En,
			[LOC_FR] = Layout_Header_CPU_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_ARCH] = {
		.Attr = Layout_Header_Arch_Attr,
		.Code = {
			[LOC_EN] = Layout_Header_Arch_Code_En,
			[LOC_FR] = Layout_Header_Arch_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_CACHE_L1] = {
		.Attr = Layout_Header_Cache_L1_Attr,
		.Code = {
			[LOC_EN] = Layout_Header_Cache_L1_Code_En,
			[LOC_FR] = Layout_Header_Cache_L1_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_BCLK] = {
		.Attr = Layout_Header_BClk_Attr,
		.Code = {
			[LOC_EN] = Layout_Header_BClk_Code_En,
			[LOC_FR] = Layout_Header_BClk_Code_Fr
		}
	},
	[RSC_LAYOUT_HEADER_CACHES] = {
		.Attr = Layout_Header_Caches_Attr,
		.Code = {
			[LOC_EN] = Layout_Header_Caches_Code_En,
			[LOC_FR] = Layout_Header_Caches_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_LOAD] = {
		.Attr = Layout_Ruller_Load_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Load_Code_En,
			[LOC_FR] = Layout_Ruller_Load_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_FREQUENCY] = {
		.Attr = Layout_Monitor_Frequency_Attr,
		.Code = {
			[LOC_EN] = Layout_Monitor_Frequency_Code_En,
			[LOC_FR] = Layout_Monitor_Frequency_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_INST] = {
		.Attr = Layout_Monitor_Inst_Attr,
		.Code = {
			[LOC_EN] = Layout_Monitor_Inst_Code_En,
			[LOC_FR] = Layout_Monitor_Inst_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_COMMON] = {
		.Attr = Layout_Monitor_Common_Attr,
		.Code = {
			[LOC_EN] = Layout_Monitor_Common_Code_En,
			[LOC_FR] = Layout_Monitor_Common_Code_Fr
		}
	},
	[RSC_LAYOUT_MONITOR_TASKS] = {
		.Attr = Layout_Monitor_Tasks_Attr,
		.Code = {
			[LOC_EN] = Layout_Monitor_Tasks_Code_En,
			[LOC_FR] = Layout_Monitor_Tasks_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_FREQUENCY] = {
		.Attr = Layout_Ruller_Frequency_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Frequency_Code_En,
			[LOC_FR] = Layout_Ruller_Frequency_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_FREQUENCY_AVG] = {
		.Attr = Layout_Ruller_Freq_Avg_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Freq_Avg_Code_En,
			[LOC_FR] = Layout_Ruller_Freq_Avg_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_FREQUENCY_PKG] = {
		.Attr = Layout_Ruller_Freq_Pkg_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Freq_Pkg_Code_En,
			[LOC_FR] = Layout_Ruller_Freq_Pkg_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_INST] = {
		.Attr = Layout_Ruller_Inst_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Inst_Code_En,
			[LOC_FR] = Layout_Ruller_Inst_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_CYCLES] = {
		.Attr = Layout_Ruller_Cycles_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Cycles_Code_En,
			[LOC_FR] = Layout_Ruller_Cycles_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_CSTATES] = {
		.Attr = Layout_Ruller_CStates_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_CStates_Code_En,
			[LOC_FR] = Layout_Ruller_CStates_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_INTERRUPTS] = {
		.Attr = Layout_Ruller_Interrupts_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Interrupts_Code_En,
			[LOC_FR] = Layout_Ruller_Interrupts_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_PACKAGE] = {
		.Attr = Layout_Ruller_Package_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Package_Code_En,
			[LOC_FR] = Layout_Ruller_Package_Code_Fr
		}
	},
	[RSC_LAYOUT_PACKAGE_UNCORE] = {
		.Attr = Layout_Package_Uncore_Attr,
		.Code = {
			[LOC_EN] = Layout_Package_Uncore_Code_En,
			[LOC_FR] = Layout_Package_Uncore_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_TASKS] = {
		.Attr = Layout_Ruller_Tasks_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Tasks_Code_En,
			[LOC_FR] = Layout_Ruller_Tasks_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_TRACKING] = {
		.Attr = Layout_Tasks_Tracking_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_Tracking_Code_En,
			[LOC_FR] = Layout_Tasks_Tracking_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_STATE_SORTED] = {
		.Attr = Layout_Tasks_State_Sorted_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_State_Sorted_Code_En,
			[LOC_FR] = Layout_Tasks_State_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_RUNTIME_SORTED] = {
		.Attr = Layout_Tasks_RunTime_Sorted_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_RunTime_Sorted_Code_En,
			[LOC_FR] = Layout_Tasks_RunTime_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_USRTIME_SORTED] = {
		.Attr = Layout_Tasks_UsrTime_Sorted_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_UsrTime_Sorted_Code_En,
			[LOC_FR] = Layout_Tasks_UsrTime_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_SYSTIME_SORTED] = {
		.Attr = Layout_Tasks_SysTime_Sorted_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_SysTime_Sorted_Code_En,
			[LOC_FR] = Layout_Tasks_SysTime_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_PROCESS_SORTED] = {
		.Attr = Layout_Tasks_Process_Sorted_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_Process_Sorted_Code_En,
			[LOC_FR] = Layout_Tasks_Process_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_COMMAND_SORTED] = {
		.Attr = Layout_Tasks_Command_Sorted_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_Command_Sorted_Code_En,
			[LOC_FR] = Layout_Tasks_Command_Sorted_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_REVERSE_SORT_OFF] = {
		.Attr = Layout_Tasks_Reverse_Sort_Off_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_Reverse_Sort_Off_Code_En,
			[LOC_FR] = Layout_Tasks_Reverse_Sort_Off_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_REVERSE_SORT_ON] = {
		.Attr = Layout_Tasks_Reverse_Sort_On_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_Reverse_Sort_On_Code_En,
			[LOC_FR] = Layout_Tasks_Reverse_Sort_On_Code_Fr
		}
	},
	[RSC_LAYOUT_TASKS_VALUE_SWITCH] = {
		.Attr = Layout_Tasks_Value_Switch_Attr,
		.Code = {
			[LOC_EN] = Layout_Tasks_Value_Switch_Code_En,
			[LOC_FR] = Layout_Tasks_Value_Switch_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_VOLTAGE] = {
		.Attr = Layout_Ruller_Voltage_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Voltage_Code_En,
			[LOC_FR] = Layout_Ruller_Voltage_Code_Fr
		}
	},
	[RSC_LAYOUT_POWER_MONITOR] = {
		.Attr = Layout_Power_Monitor_Attr,
		.Code = {
			[LOC_EN] = Layout_Power_Monitor_Code_En,
			[LOC_FR] = Layout_Power_Monitor_Code_Fr
		}
	},
	[RSC_LAYOUT_RULLER_SLICE] = {
		.Attr = Layout_Ruller_Slice_Attr,
		.Code = {
			[LOC_EN] = Layout_Ruller_Slice_Code_En,
			[LOC_FR] = Layout_Ruller_Slice_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_TECH_X86] = {
		.Attr = Layout_Footer_Tech_x86_Attr,
		.Code = {
			[LOC_EN] = Layout_Footer_Tech_x86_Code_En,
			[LOC_FR] = Layout_Footer_Tech_x86_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_TECH_INTEL] = {
		.Attr = Layout_Footer_Tech_Intel_Attr,
		.Code = {
			[LOC_EN] = Layout_Footer_Tech_Intel_Code_En,
			[LOC_FR] = Layout_Footer_Tech_Intel_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_TECH_AMD] = {
		.Attr = Layout_Footer_Tech_AMD_Attr,
		.Code = {
			[LOC_EN] = Layout_Footer_Tech_AMD_Code_En,
			[LOC_FR] = Layout_Footer_Tech_AMD_Code_Fr
		}
	},
	[RSC_LAYOUT_FOOTER_SYSTEM] = {
		.Attr = Layout_Footer_System_Attr,
		.Code = {
			[LOC_EN] = Layout_Footer_System_Code_En,
			[LOC_FR] = Layout_Footer_System_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_CORE_ONLINE] = {
		.Attr = Layout_Card_Core_Online_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Core_Online_Code_En,
			[LOC_FR] = Layout_Card_Core_Online_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_CORE_OFFLINE] = {
		.Attr = Layout_Card_Core_Offline_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Core_Offline_Code_En,
			[LOC_FR] = Layout_Card_Core_Offline_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_CLK] = {
		.Attr = Layout_Card_CLK_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_CLK_Code_En,
			[LOC_FR] = Layout_Card_CLK_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_UNCORE] = {
		.Attr = Layout_Card_Uncore_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Uncore_Code_En,
			[LOC_FR] = Layout_Card_Uncore_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_BUS] = {
		.Attr = Layout_Card_Bus_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Bus_Code_En,
			[LOC_FR] = Layout_Card_Bus_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_MC] = {
		.Attr = Layout_Card_MC_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_MC_Code_En,
			[LOC_FR] = Layout_Card_MC_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_LOAD] = {
		.Attr = Layout_Card_Load_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Load_Code_En,
			[LOC_FR] = Layout_Card_Load_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_IDLE] = {
		.Attr = Layout_Card_Idle_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Idle_Code_En,
			[LOC_FR] = Layout_Card_Idle_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_RAM] = {
		.Attr = Layout_Card_RAM_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_RAM_Code_En,
			[LOC_FR] = Layout_Card_RAM_Code_Fr
		}
	},
	[RSC_LAYOUT_CARD_TASK] = {
		.Attr = Layout_Card_Task_Attr,
		.Code = {
			[LOC_EN] = Layout_Card_Task_Code_En,
			[LOC_FR] = Layout_Card_Task_Code_Fr
		}
	},
	[RSC_PROCESSOR] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Processor",
			[LOC_FR] = (ASCII*) "Processeur"
		}
	},
	[RSC_ARCHITECTURE] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Architecture",
			[LOC_FR] = (ASCII*) "Architecture"
		}
	},
	[RSC_VENDOR_ID] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Vendor ID",
			[LOC_FR] = (ASCII*) "ID vendeur"
		}
	},
	[RSC_MICROCODE] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Microcode",
			[LOC_FR] = (ASCII*) "Microcode"
		}
	},
	[RSC_SIGNATURE] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Signature",
			[LOC_FR] = (ASCII*) "Signature"
		}
	},
	[RSC_STEPPING] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Stepping",
			[LOC_FR] = (ASCII*) "Stepping"
		}
	},
	[RSC_ONLINE_CPU] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Online CPU",
			[LOC_FR] = (ASCII*) "CPU en ligne"
		}
	},
	[RSC_BASE_CLOCK] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Base Clock",
			[LOC_FR] = (ASCII*) "Horloge de base"
		}
	},
	[RSC_FREQUENCY] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Frequency",
			[LOC_FR] = (ASCII*) "Frequence"
		}
	},
	[RSC_RATIO] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Ratio",
			[LOC_FR] = (ASCII*) "Ratio"
		}
	},
	[RSC_FACTORY] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Factory",
			[LOC_FR] = (ASCII*) "Usine"
		}
	},
	[RSC_LEVEL] = {
		.Code = {
			[LOC_EN] = (ASCII*) "Level",
			[LOC_FR] = (ASCII*) "Niveau"
		}
	},
	[RSC_RUN_STATE_COLOR] = {
		.Attr = SymbolRunColor_Attr
	},
	[RSC_UNINT_STATE_COLOR] = {
		.Attr = SymbolUnIntColor_Attr
	},
	[RSC_ZOMBIE_STATE_COLOR] = {
		.Attr = SymbolZombieColor_Attr
	},
	[RSC_SLEEP_STATE_COLOR] = {
		.Attr = SymbolSleepColor_Attr
	},
	[RSC_WAIT_STATE_COLOR] = {
		.Attr = SymbolWaitColor_Attr
	},
	[RSC_OTHER_STATE_COLOR] = {
		.Attr = SymbolOtherColor_Attr
	},
	[RSC_TRACKER_STATE_COLOR] = {
		.Attr = SymbolTrackerColor_Attr
	},
	[RSC_WIN_SYSINFO_CPUID_COND0] = {
		.Attr = Win_SysInfoCPUID_Cond_Attr[0]
	},
	[RSC_WIN_SYSINFO_CPUID_COND1] = {
		.Attr = Win_SysInfoCPUID_Cond_Attr[1]
	},
	[RSC_WIN_SYSINFO_CPUID_COND2] = {
		.Attr = Win_SysInfoCPUID_Cond_Attr[2]
	},
	[RSC_WIN_SYSINFO_CPUID_COND3] = {
		.Attr = Win_SysInfoCPUID_Cond_Attr[3]
	},
	[RSC_WIN_SYSTEM_REGISTERS_COND0] = {
		.Attr = Win_SystemRegisters_Cond_Attr[0]
	},
	[RSC_WIN_SYSTEM_REGISTERS_COND1] = {
		.Attr = Win_SystemRegisters_Cond_Attr[1]
	},
	[RSC_WIN_SYSTEM_REGISTERS_COND2] = {
		.Attr = Win_SystemRegisters_Cond_Attr[2]
	},
	[RSC_WIN_SYSINFO_PROC_COND0] = {
		.Attr = Win_SysInfoProc_Cond_Attr[0]
	},
	[RSC_WIN_SYSINFO_PROC_COND1] = {
		.Attr = Win_SysInfoProc_Cond_Attr[1]
	},
	[RSC_WIN_SYSINFO_PROC_COND2] = {
		.Attr = Win_SysInfoProc_Cond_Attr[2]
	},
	[RSC_WIN_SYSINFO_PROC_COND3] = {
		.Attr = Win_SysInfoProc_Cond_Attr[3]
	},
	[RSC_WIN_SYSINFO_ISA_COND_0_0] = {
		.Attr = Win_SysInfoISA_Cond_Attr[0][0]
	},
	[RSC_WIN_SYSINFO_ISA_COND_0_1] = {
		.Attr = Win_SysInfoISA_Cond_Attr[0][1]
	},
	[RSC_WIN_SYSINFO_ISA_COND_0_2] = {
		.Attr = Win_SysInfoISA_Cond_Attr[0][2]
	},
	[RSC_WIN_SYSINFO_ISA_COND_0_3] = {
		.Attr = Win_SysInfoISA_Cond_Attr[0][3]
	},
	[RSC_WIN_SYSINFO_ISA_COND_0_4] = {
		.Attr = Win_SysInfoISA_Cond_Attr[0][4]
	},
	[RSC_WIN_SYSINFO_ISA_COND_1_0] = {
		.Attr = Win_SysInfoISA_Cond_Attr[1][0]
	},
	[RSC_WIN_SYSINFO_ISA_COND_1_1] = {
		.Attr = Win_SysInfoISA_Cond_Attr[1][1]
	},
	[RSC_WIN_SYSINFO_ISA_COND_1_2] = {
		.Attr = Win_SysInfoISA_Cond_Attr[1][2]
	},
	[RSC_WIN_SYSINFO_ISA_COND_1_3] = {
		.Attr = Win_SysInfoISA_Cond_Attr[1][3]
	},
	[RSC_WIN_SYSINFO_ISA_COND_1_4] = {
		.Attr = Win_SysInfoISA_Cond_Attr[1][4]
	},
	[RSC_WIN_SYSINFO_ISA_COND_2_0] = {
		.Attr = Win_SysInfoISA_Cond_Attr[2][0]
	},
	[RSC_WIN_SYSINFO_ISA_COND_2_1] = {
		.Attr = Win_SysInfoISA_Cond_Attr[2][1]
	},
	[RSC_WIN_SYSINFO_ISA_COND_2_2] = {
		.Attr = Win_SysInfoISA_Cond_Attr[2][2]
	},
	[RSC_WIN_SYSINFO_ISA_COND_2_3] = {
		.Attr = Win_SysInfoISA_Cond_Attr[2][3]
	},
	[RSC_WIN_SYSINFO_ISA_COND_2_4] = {
		.Attr = Win_SysInfoISA_Cond_Attr[2][4]
	},
	[RSC_WIN_SYSINFO_ISA_COND_3_0] = {
		.Attr = Win_SysInfoISA_Cond_Attr[3][0]
	},
	[RSC_WIN_SYSINFO_ISA_COND_3_1] = {
		.Attr = Win_SysInfoISA_Cond_Attr[3][1]
	},
	[RSC_WIN_SYSINFO_ISA_COND_3_2] = {
		.Attr = Win_SysInfoISA_Cond_Attr[3][2]
	},
	[RSC_WIN_SYSINFO_ISA_COND_3_3] = {
		.Attr = Win_SysInfoISA_Cond_Attr[3][3]
	},
	[RSC_WIN_SYSINFO_ISA_COND_3_4] = {
		.Attr = Win_SysInfoISA_Cond_Attr[3][4]
	},
	[RSC_WIN_SYSINFO_FEATURES_COND0] = {
		.Attr = Win_SysInfoFeatures_Cond_Attr[0]
	},
	[RSC_WIN_SYSINFO_FEATURES_COND1] = {
		.Attr = Win_SysInfoFeatures_Cond_Attr[1]
	},
	[RSC_WIN_SYSINFO_FEATURES_COND2] = {
		.Attr = Win_SysInfoFeatures_Cond_Attr[2]
	},
	[RSC_WIN_SYSINFO_TECH_COND0] = {
		.Attr = Win_SysInfoTech_Cond_Attr[0]
	},
	[RSC_WIN_SYSINFO_TECH_COND1] = {
		.Attr = Win_SysInfoTech_Cond_Attr[1]
	},
	[RSC_WIN_SYSINFO_PERFMON_COND0] = {
		.Attr = Win_SysInfoPerfMon_Cond_Attr[0]
	},
	[RSC_WIN_SYSINFO_PERFMON_COND1] = {
		.Attr = Win_SysInfoPerfMon_Cond_Attr[1]
	},
	[RSC_WIN_SYSINFO_PERFMON_COND2] = {
		.Attr = Win_SysInfoPerfMon_Cond_Attr[2]
	},
	[RSC_WIN_SYSINFO_PERFMON_COND3] = {
		.Attr = Win_SysInfoPerfMon_Cond_Attr[3]
	},
	[RSC_WIN_SYSINFO_PWR_THERMAL_COND0] = {
		.Attr = Win_SysInfoPwrThermal_Cond_Attr[0]
	},
	[RSC_WIN_SYSINFO_PWR_THERMAL_COND1] = {
		.Attr = Win_SysInfoPwrThermal_Cond_Attr[1]
	},
	[RSC_WIN_SYSINFO_PWR_THERMAL_COND2] = {
		.Attr = Win_SysInfoPwrThermal_Cond_Attr[2]
	},
	[RSC_WIN_SYSINFO_PWR_THERMAL_COND3] = {
		.Attr = Win_SysInfoPwrThermal_Cond_Attr[3]
	},
	[RSC_WIN_SYSINFO_KERNEL] = {
		.Attr = Win_SysInfoKernel_Attr
	},
	[RSC_WIN_TOPOLOGY_COND0] = {
		.Attr = Win_Topology_Cond_Attr[0]
	},
	[RSC_WIN_TOPOLOGY_COND1] = {
		.Attr = Win_Topology_Cond_Attr[1]
	},
	[RSC_WIN_TOPOLOGY_COND2] = {
		.Attr = Win_Topology_Cond_Attr[2]
	},
	[RSC_WIN_MEMORY_CONTROLLER_COND0] = {
		.Attr = Win_MemoryController_Cond_Attr[0]
	},
	[RSC_WIN_MEMORY_CONTROLLER_COND1] = {
		.Attr = Win_MemoryController_Cond_Attr[1]
	}
};
