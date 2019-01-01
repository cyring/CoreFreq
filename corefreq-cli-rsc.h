/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

enum {
	RSC_LAYOUT_HEADER_PROC,
	RSC_LAYOUT_HEADER_CPU,
	RSC_LAYOUT_HEADER_ARCH,
	RSC_LAYOUT_HEADER_CACHE_L1,
	RSC_LAYOUT_HEADER_BCLK,
	RSC_LAYOUT_HEADER_CACHES,
	RSC_LAYOUT_RULLER_LOAD,
	RSC_LAYOUT_MONITOR_FREQUENCY,
	RSC_LAYOUT_MONITOR_INST,
	RSC_LAYOUT_MONITOR_COMMON,
	RSC_LAYOUT_MONITOR_TASKS,
	RSC_LAYOUT_RULLER_FREQUENCY,
	RSC_LAYOUT_RULLER_FREQUENCY_AVG,
	RSC_LAYOUT_RULLER_FREQUENCY_PKG,
	RSC_LAYOUT_RULLER_INST,
	RSC_LAYOUT_RULLER_CYCLES,
	RSC_LAYOUT_RULLER_CSTATES,
	RSC_LAYOUT_RULLER_INTERRUPTS,
	RSC_LAYOUT_RULLER_PACKAGE,
	RSC_LAYOUT_PACKAGE_PC,
	RSC_LAYOUT_PACKAGE_UNCORE,
	RSC_LAYOUT_RULLER_TASKS,
	RSC_LAYOUT_TASKS_TRACKING,
	RSC_LAYOUT_TASKS_STATE_SORTED,
	RSC_LAYOUT_TASKS_RUNTIME_SORTED,
	RSC_LAYOUT_TASKS_USRTIME_SORTED,
	RSC_LAYOUT_TASKS_SYSTIME_SORTED,
	RSC_LAYOUT_TASKS_PROCESS_SORTED,
	RSC_LAYOUT_TASKS_COMMAND_SORTED,
	RSC_LAYOUT_TASKS_REVERSE_SORT_OFF,
	RSC_LAYOUT_TASKS_REVERSE_SORT_ON,
	RSC_LAYOUT_TASKS_VALUE_SWITCH,
	RSC_LAYOUT_TASKS_VALUE_OFF,
	RSC_LAYOUT_TASKS_VALUE_ON,
	RSC_LAYOUT_RULLER_VOLTAGE,
	RSC_LAYOUT_POWER_MONITOR,
	RSC_LAYOUT_RULLER_SLICE,
	RSC_LAYOUT_FOOTER_TECH_X86,
	RSC_LAYOUT_FOOTER_TECH_INTEL,
	RSC_LAYOUT_FOOTER_TECH_AMD,
	RSC_LAYOUT_FOOTER_SYSTEM,
	RSC_LAYOUT_CARD_CORE_ONLINE,
	RSC_LAYOUT_CARD_CORE_OFFLINE,
	RSC_LAYOUT_CARD_CLK,
	RSC_LAYOUT_CARD_UNCORE,
	RSC_LAYOUT_CARD_BUS,
	RSC_LAYOUT_CARD_MC,
	RSC_LAYOUT_CARD_LOAD,
	RSC_LAYOUT_CARD_IDLE,
	RSC_LAYOUT_CARD_RAM,
	RSC_LAYOUT_CARD_TASK,
/* ATTRIBUTE */
	RSC_RUN_STATE_COLOR,
	RSC_UNINT_STATE_COLOR,
	RSC_ZOMBIE_STATE_COLOR,
	RSC_SLEEP_STATE_COLOR,
	RSC_WAIT_STATE_COLOR,
	RSC_OTHER_STATE_COLOR,
	RSC_TRACKER_STATE_COLOR,
	RSC_SYSINFO_CPUID_COND0,
	RSC_SYSINFO_CPUID_COND1,
	RSC_SYSINFO_CPUID_COND2,
	RSC_SYSINFO_CPUID_COND3,
	RSC_SYSTEM_REGISTERS_COND0,
	RSC_SYSTEM_REGISTERS_COND1,
	RSC_SYSTEM_REGISTERS_COND2,
	RSC_SYSINFO_PROC_COND0,
	RSC_SYSINFO_PROC_COND1,
	RSC_SYSINFO_PROC_COND2,
	RSC_SYSINFO_PROC_COND3,
	RSC_SYSINFO_ISA_COND_0_0,
	RSC_SYSINFO_ISA_COND_0_1,
	RSC_SYSINFO_ISA_COND_0_2,
	RSC_SYSINFO_ISA_COND_0_3,
	RSC_SYSINFO_ISA_COND_0_4,
	RSC_SYSINFO_ISA_COND_1_0,
	RSC_SYSINFO_ISA_COND_1_1,
	RSC_SYSINFO_ISA_COND_1_2,
	RSC_SYSINFO_ISA_COND_1_3,
	RSC_SYSINFO_ISA_COND_1_4,
	RSC_SYSINFO_ISA_COND_2_0,
	RSC_SYSINFO_ISA_COND_2_1,
	RSC_SYSINFO_ISA_COND_2_2,
	RSC_SYSINFO_ISA_COND_2_3,
	RSC_SYSINFO_ISA_COND_2_4,
	RSC_SYSINFO_ISA_COND_3_0,
	RSC_SYSINFO_ISA_COND_3_1,
	RSC_SYSINFO_ISA_COND_3_2,
	RSC_SYSINFO_ISA_COND_3_3,
	RSC_SYSINFO_ISA_COND_3_4,
	RSC_SYSINFO_FEATURES_COND0,
	RSC_SYSINFO_FEATURES_COND1,
	RSC_SYSINFO_FEATURES_COND2,
	RSC_SYSINFO_TECH_COND0,
	RSC_SYSINFO_TECH_COND1,
	RSC_SYSINFO_PERFMON_COND0,
	RSC_SYSINFO_PERFMON_COND1,
	RSC_SYSINFO_PERFMON_COND2,
	RSC_SYSINFO_PERFMON_COND3,
	RSC_SYSINFO_PWR_THERMAL_COND0,
	RSC_SYSINFO_PWR_THERMAL_COND1,
	RSC_SYSINFO_PWR_THERMAL_COND2,
	RSC_SYSINFO_PWR_THERMAL_COND3,
	RSC_SYSINFO_KERNEL,
	RSC_TOPOLOGY_COND0,
	RSC_TOPOLOGY_COND1,
	RSC_TOPOLOGY_COND2,
	RSC_MEMORY_CONTROLLER_COND0,
	RSC_MEMORY_CONTROLLER_COND1,
	RSC_CREATE_MENU_STOP,
	RSC_CREATE_MENU_FN_KEY,
	RSC_CREATE_MENU_SHORTKEY,
	RSC_CREATE_SETTINGS_COND0,
	RSC_CREATE_SETTINGS_COND1,
	RSC_CREATE_ADV_HELP_COND0,
	RSC_CREATE_ADV_HELP_COND1,
	RSC_CREATE_HOTPLUG_CPU_ENABLE,
	RSC_CREATE_HOTPLUG_CPU_DISABLE,
	RSC_CREATE_CORE_CLOCK_COND0,
	RSC_CREATE_CORE_CLOCK_COND1,
	RSC_CREATE_CORE_CLOCK_COND2,
	RSC_CREATE_UNCORE_CLOCK_COND0,
	RSC_CREATE_UNCORE_CLOCK_COND1,
/* ASCII */
	RSC_PROCESSOR_TITLE,
	RSC_PROCESSOR,
	RSC_ARCHITECTURE,
	RSC_VENDOR_ID,
	RSC_MICROCODE,
	RSC_SIGNATURE,
	RSC_STEPPING,
	RSC_ONLINE_CPU,
	RSC_BASE_CLOCK,
	RSC_FREQUENCY,
	RSC_RATIO,
	RSC_FACTORY,
	RSC_LEVEL,
	RSC_PROGRAMMABLE,
	RSC_CONFIGURATION,
	RSC_TURBO_ACTIVATION,
	RSC_NOMINAL,
	RSC_UNLOCK,
	RSC_LOCK,
	RSC_CPUID_TITLE,
	RSC_LARGEST_STD_FUNC,
	RSC_LARGEST_EXT_FUNC,
	RSC_SYS_REGS_TITLE,
	RSC_ISA_TITLE,
	RSC_FEATURES_TITLE,
	RSC_MISSING,
	RSC_PRESENT,
	RSC_VARIANT,
	RSC_INVARIANT,
	RSC_TECHNOLOGIES_TITLE,
	RSC_PERF_MON_TITLE,
	RSC_VERSION,
	RSC_COUNTERS,
	RSC_GENERAL_CTRS,
	RSC_FIXED_CTRS,
	RSC_POWER_THERMAL_TITLE,
	RSC_KERNEL_TITLE,
	RSC_TOPOLOGY_TITLE,
	RSC_MEM_CTRL_TITLE,
	RSC_SETTINGS_TITLE,
	RSC_HELP_TITLE,
	RSC_ADV_HELP_TITLE
};

typedef struct {
	ATTRIBUTE *Attr;
	ASCII	  *Code[LOC_CNT];
} RESOURCE_ST;

extern RESOURCE_ST Resource[];

#define ATTR() Attr

#define CODE() Code[LOC()]

#define RSC(_ID) (Resource[RSC_##_ID])

#define RSZ(_ID) (strlen((char *) RSC(_ID).CODE()))

#define RSC_RUN_STATE_COLOR_ATTR		\
{						\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,	\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK 	\
}

#define RSC_UNINT_STATE_COLOR_ATTR		\
{						\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,	\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK 	\
}

#define RSC_ZOMBIE_STATE_COLOR_ATTR		\
{						\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW,	\
	LRW,LRW,LRW,LRW,LRW,LRW,LRW,LRW 	\
}

#define RSC_SLEEP_STATE_COLOR_ATTR		\
{						\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}

#define RSC_WAIT_STATE_COLOR_ATTR		\
{						\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,	\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 	\
}

#define RSC_OTHER_STATE_COLOR_ATTR		\
{						\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,	\
	LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK 	\
}

#define RSC_TRACKER_STATE_COLOR_ATTR		\
{						\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC,	\
	LKC,LKC,LKC,LKC,LKC,LKC,LKC,LKC 	\
}

#define RSC_LAYOUT_HEADER_PROC_ATTR					\
{									\
	HRK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK 		\
}

#define RSC_LAYOUT_HEADER_CPU_ATTR					\
{									\
	HDK,HWK,HWK,HWK,HDK,HWK,HWK,HWK,LWK,LWK,LWK			\
}

#define RSC_LAYOUT_HEADER_CPU_CODE					\
{									\
	']',' ',' ',' ','/',' ',' ',' ','C','P','U'			\
}

#define RSC_LAYOUT_HEADER_ARCH_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK	\
}

#define RSC_LAYOUT_HEADER_ARCH_CODE					\
{									\
	' ','A','r','c','h','i','t','e','c','t','u','r','e',' ','['	\
}

#define RSC_LAYOUT_HEADER_CACHE_L1_ATTR 				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,				\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,			\
	LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,LWK,LWK 			\
}

#define RSC_LAYOUT_HEADER_CACHE_L1_CODE 				\
{									\
	']',' ','C','a','c','h','e','s',' ',				\
	'L','1',' ','I','n','s','t','=',' ',' ',' ',			\
	'D','a','t','a','=',' ',' ',' ','K','B' 			\
}

#define RSC_LAYOUT_HEADER_BCLK_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
	HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_HEADER_CACHES_ATTR					\
{									\
	LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,			\
	LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK			\
}

#define RSC_LAYOUT_HEADER_CACHES_CODE					\
{									\
	'L','2','=',' ',' ',' ',' ',' ',' ',' ',			\
	'L','3','=',' ',' ',' ',' ',' ',' ','K','B'			\
}

#define RSC_LAYOUT_RULLER_LOAD_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_RULLER_LOAD_CODE					\
{									\
	'-','-','-',' ','R','a','t','i','o',' ','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_MONITOR_FREQUENCY_ATTR				\
{									\
	HWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HWK,HWK,LWK,HWK,HWK,HDK,\
	LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,\
	LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,\
	LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,LWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,\
	LWK,LWK,HBK,HBK,HBK,HDK,LWK,LWK,LWK,HDK,LYK,LYK,LYK		\
}

#define RSC_LAYOUT_MONITOR_FREQUENCY_CODE				\
{									\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',0x0,' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',0x0,' ',' ',' '		\
}

#define RSC_LAYOUT_MONITOR_INST_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HDK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HDK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK			\
}

#define RSC_LAYOUT_MONITOR_INST_CODE					\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,' ',' ',' ',' ',\
	' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,' ',\
	' ',' ',' ',' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',0x0,' ',' ',' ',' ',' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '			\
}

#define RSC_LAYOUT_MONITOR_COMMON_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK		\
}

#define RSC_LAYOUT_MONITOR_COMMON_CODE					\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
}

#define RSC_LAYOUT_MONITOR_TASKS_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_MONITOR_TASKS_CODE					\
{									\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define RSC_LAYOUT_RULLER_FREQUENCY_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_RULLER_FREQUENCY_CODE				\
	"--- Freq(MHz) Ratio - Turbo --- C0 ---- C1 ---- C3 ---- C6 -"	\
	"--- C7 --Min TMP Max ---------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULLER_FREQUENCY_AVG_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_RULLER_FREQUENCY_PKG_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_RULLER_FREQUENCY_PKG_CODE				\
{									\
	'-','-','-','-','-',' ','%',' ','P','k','g',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULLER_INST_CODE					\
	"------------ IPS -------------- IPC -------------- CPI -----"	\
	"------------- INST -----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULLER_CYCLES_CODE					\
	"-------------- C0:UCC ---------- C0:URC ------------ C1 ----"	\
	"--------- TSC ----------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULLER_CSTATES_CODE					\
	"---------------- C1 -------------- C3 -------------- C6 ----"	\
	"---------- C7 ----------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULLER_INTERRUPTS_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_RULLER_INTERRUPTS_CODE				\
	"---------- SMI ------------ NMI[ LOCAL   UNKNOWN  PCI_SERR# "	\
	" IO_CHECK] -------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_PACKAGE_PC_ATTR					\
{									\
	LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,HDK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK \
}

#define RSC_LAYOUT_PACKAGE_PC_CODE					\
{									\
	'P','C','0','0',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define RSC_LAYOUT_PACKAGE_UNCORE_ATTR					\
{									\
	LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_PACKAGE_UNCORE_CODE					\
{									\
	' ','T','S','C',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ','U','N','C','O','R','E',':',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define RSC_LAYOUT_RULLER_TASKS_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,\
	LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LDK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_TASKS_STATE_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,HDK,LWK, LWK,LWK,LWK			\
}

#define RSC_LAYOUT_TASKS_RUNTIME_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK			\
}

#define RSC_LAYOUT_TASKS_USRTIME_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, LCK,HDK,LWK			\
}

#define RSC_LAYOUT_TASKS_SYSTIME_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK			\
}

#define RSC_LAYOUT_TASKS_PROCESS_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,HDK,LWK,LWK,LWK, LWK,LWK,LWK			\
}

#define RSC_LAYOUT_TASKS_COMMAND_SORTED_ATTR				\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,			\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK, HDK,LWK,LWK			\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK	\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_ON_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,HDK,LCK,LCK,LCK,HDK,LWK	\
}

#define RSC_LAYOUT_TASKS_VALUE_SWITCH_ATTR				\
{									\
	LWK,_HCK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK		\
}

#define RSC_LAYOUT_TASKS_VALUE_OFF_ATTR 				\
{									\
	LWK,LWK,LWK							\
}

#define RSC_LAYOUT_TASKS_VALUE_OFF_CODE 				\
{									\
	'O','F','F'							\
}

#define RSC_LAYOUT_TASKS_VALUE_ON_ATTR					\
{									\
	LCK,LCK,LCK							\
}

#define RSC_LAYOUT_TASKS_VALUE_ON_CODE					\
{									\
	' ','O','N'							\
}

#define RSC_LAYOUT_TASKS_TRACKING_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,\
	LWK,LWK,LWK,LWK,HDK,LWK						\
}

#define RSC_LAYOUT_RULLER_VOLTAGE_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_POWER_MONITOR_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,		\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK \
}

#define RSC_LAYOUT_POWER_MONITOR_CODE					\
	"                                       "

#define RSC_LAYOUT_RULLER_SLICE_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_LAYOUT_RULLER_SLICE_CODE					\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 -----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_TECH_X86_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK 	\
}

#define RSC_LAYOUT_FOOTER_TECH_X86_CODE					\
{									\
	'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',',' 	\
}

#define RSC_LAYOUT_FOOTER_TECH_INTEL_ATTR				\
{									\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,			\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,LWK,HDK,HDK,HDK,HDK,LWK,				\
	HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,				\
	LWK,HDK,HWK,LWK,HWK,HWK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK 	\
}

#define RSC_LAYOUT_FOOTER_TECH_INTEL_CODE				\
{									\
	'H','T','T',',','E','I','S','T',',','I','D','A',',',		\
	'T','U','R','B','O',',','C','1','E',',',			\
	' ','P','M',',','C','3','A',',','C','1','A',',',		\
	'C','3','U',',','C','1','U',',',				\
	'T','M',',','H','O','T',']',' ',' ',				\
	'V','[',' ','.',' ',' ',']',' ','T','[',' ',' ',' ',']' 	\
}

#define RSC_LAYOUT_FOOTER_TECH_AMD_ATTR 				\
{									\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,		\
	LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,HDK,HDK,HDK,LWK,		\
	HDK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,		\
	LWK,HDK,HWK,LWK,HWK,HWK,HDK,HDK,LWK,HDK,HDK,HDK,HDK,HDK 	\
}

#define RSC_LAYOUT_FOOTER_TECH_AMD_CODE 				\
{									\
	'S','M','T',',','P','o','w','e','r','N','o','w',',',		\
	'B','O','O','S','T',',','C','1','E',',','C','C','6',		\
	',','P','C','6',',',' ','P','M',',','D','T','S',',',		\
	'T','T','P',',','H','O','T',']',' ',' ',' ',' ',' ',		\
	'V','[',' ','.',' ',' ',']',' ','T','[',' ',' ',' ',']' 	\
}

#define RSC_LAYOUT_FOOTER_SYSTEM_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,	\
	LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,	\
	HWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,HDK 	\
}

#define RSC_LAYOUT_CARD_CORE_ONLINE_ATTR				\
{									\
	HDK,HDK,HDK,LCK,LCK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_CORE_ONLINE_CODE				\
{									\
	'[',' ','#',' ',' ',' ',' ',' ',' ','C',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CORE_OFFLINE_ATTR				\
{									\
	HDK,HDK,HDK,LBK,LBK,HDK,HDK,LWK,LWK,LWK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_CORE_OFFLINE_CODE				\
{									\
	'[',' ','#',' ',' ',' ',' ','O','F','F',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CLK_ATTR					\
{									\
	HDK,HDK,HWK,HWK,HWK,LWK,HWK,HDK,HDK,HDK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_CLK_CODE					\
{									\
	'[',' ','0','0','0','.','0',' ','M','H','z',']' 		\
}

#define RSC_LAYOUT_CARD_UNCORE_ATTR					\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,LCK,LCK,HDK 		\
}

#define RSC_LAYOUT_CARD_UNCORE_CODE					\
{									\
	'[','U','N','C','O','R','E',' ',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_BUS_ATTR					\
{									\
	HDK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,HDK 		\
}

#define RSC_LAYOUT_CARD_BUS_CODE					\
{									\
	'[','B','u','s',' ',' ',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_MC_ATTR 					\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK 		\
}

#define RSC_LAYOUT_CARD_MC_CODE						\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_LOAD_ATTR					\
{									\
	HDK,HDK,HDK,HDK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_IDLE_ATTR					\
{									\
	HDK,HDK,HDK,HDK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_RAM_ATTR					\
{									\
	HDK,HWK,HWK,HWK,HWK,HWK,LWK,HDK,HWK,HWK,LWK,HDK 		\
}

#define RSC_LAYOUT_CARD_RAM_CODE					\
{									\
	'[',' ',' ',' ',' ',' ',' ','/',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_TASK_ATTR					\
{									\
	HDK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HDK 		\
}

#define RSC_SYSINFO_CPUID_COND0_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LCK,LCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSINFO_CPUID_COND1_ATTR					\
{									\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSINFO_CPUID_COND2_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSINFO_CPUID_COND3_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,\
	LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,\
	LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSTEM_REGISTERS_COND0_ATTR 				\
{									\
	LWK,LWK,LWK,LWK 						\
}

#define RSC_SYSTEM_REGISTERS_COND1_ATTR 				\
{									\
	HBK,HBK,HBK,HBK 						\
}

#define RSC_SYSTEM_REGISTERS_COND2_ATTR 				\
{									\
	HWK,HWK,HWK,HWK 						\
}

#define RSC_SYSINFO_PROC_COND0_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 		\
}

#define RSC_SYSINFO_PROC_COND1_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,HGK,LWK,LWK 		\
}

#define RSC_SYSINFO_PROC_COND2_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK 		\
}

#define RSC_SYSINFO_PROC_COND3_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK 		\
}

#define RSC_SYSINFO_ISA_COND0_ATTR					\
{									\
    {	/*	[N] & [N/N]	2*(0|0)+(0<<0)			*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ /*	[Y]						*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,LWK					\
    },{ /*	[N/Y]	2*(0|1)+(0<<1)				*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,LWK					\
    },{ /*	[Y/N]	2*(1|0)+(1<<0)				*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ /*	[Y/Y]	2*(1|1)+(1<<1)				*/	\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,HGK,LWK					\
    }									\
}

#define RSC_SYSINFO_ISA_COND1_ATTR					\
{									\
    {									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,HGK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,HGK,LWK,LWK					\
    }									\
}

#define RSC_SYSINFO_ISA_COND2_ATTR					\
{									\
    {									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,LWK,HGK,LWK,LWK,LWK					\
    }									\
}

#define RSC_SYSINFO_ISA_COND3_ATTR					\
{									\
    {									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,HGK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,HGK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,LWK,LWK,LWK					\
    },{ 								\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,HGK,LWK,HGK,LWK,LWK					\
    }									\
}

#define RSC_SYSINFO_FEATURES_COND0_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK 							\
}

#define RSC_SYSINFO_FEATURES_COND1_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,			\
	LWK,LWK 							\
}

#define RSC_SYSINFO_FEATURES_COND2_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,			\
	LWK,LWK 							\
}

#define RSC_SYSINFO_TECH_COND0_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSINFO_TECH_COND1_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,HGK,HGK,LWK,LWK 			\
}

#define RSC_SYSINFO_PERFMON_COND0_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK 						\
}

#define RSC_SYSINFO_PERFMON_COND1_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HGK,			\
	HGK,HGK,LWK,LWK 						\
}

#define RSC_SYSINFO_PERFMON_COND2_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HBK,HBK,HBK,HBK,HBK,			\
	HBK,HBK,LWK,LWK 						\
}

#define RSC_SYSINFO_PERFMON_COND3_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,			\
	HGK,HGK,LWK,LWK 						\
}

#define RSC_SYSINFO_PWR_THERMAL_COND0_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSINFO_PWR_THERMAL_COND1_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK 			\
}

#define RSC_SYSINFO_PWR_THERMAL_COND2_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LWK,LWK 			\
}

#define RSC_SYSINFO_PWR_THERMAL_COND3_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,LWK,LWK 			\
}

#define RSC_SYSINFO_KERNEL_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK 					\
}

#define RSC_TOPOLOGY_COND0_ATTR 					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK		\
}

#define RSC_TOPOLOGY_COND1_ATTR 					\
{									\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK		\
}

#define RSC_TOPOLOGY_COND2_ATTR 					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK		\
}

#define RSC_MEMORY_CONTROLLER_COND0_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK						\
}

#define RSC_MEMORY_CONTROLLER_COND1_ATTR				\
{									\
	HWK,HWK,HWK,HWK,HWK						\
}

#define RSC_CREATE_MENU_STOP_ATTR					\
{									\
	HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,		\
	HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW 		\
}

#define RSC_CREATE_MENU_FN_KEY_ATTR					\
{									\
	LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW, LKW,LKW,LKW,LKW,		\
	LKW,LKW,LKW,LKW,LKW,LKW,LKW,HKW,_LKW,_LKW,HKW,LKW		\
}

#define RSC_CREATE_MENU_SHORTKEY_ATTR					\
{									\
	LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW, LKW,LKW,LKW,		\
	LKW,LKW,LKW,LKW,LKW,LKW,LKW,LKW,HKW,_LKW,HKW,LKW		\
}

#define RSC_CREATE_SETTINGS_COND0_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK \
}

#define RSC_CREATE_SETTINGS_COND1_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HGK,HGK,HGK,HDK,LWK \
}

#define RSC_CREATE_ADV_HELP_COND0_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
}

#define RSC_CREATE_ADV_HELP_COND1_ATTR					\
{									\
	LWK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK					\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_ATTR				\
{									\
	LWK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LWK,LDK,LDK,LDK			\
}

#define RSC_CREATE_HOTPLUG_CPU_DISABLE_ATTR				\
{									\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,LWK,LDK,LDK,LDK			\
}

#define RSC_CREATE_CORE_CLOCK_COND0_ATTR				\
{									\
	LWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}

#define RSC_CREATE_CORE_CLOCK_COND1_ATTR				\
{									\
	LWK,HBK,HBK,HBK,HBK,LBK,HBK,HBK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}

#define RSC_CREATE_CORE_CLOCK_COND2_ATTR				\
{									\
	LWK,HRK,HRK,HRK,HRK,LRK,HRK,HRK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}

#define RSC_CREATE_UNCORE_CLOCK_COND0_ATTR				\
{									\
	LWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}

#define RSC_CREATE_UNCORE_CLOCK_COND1_ATTR				\
{									\
	LWK,HRK,HRK,HRK,HRK,LRK,HRK,HRK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 	\
}
