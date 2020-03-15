/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

enum {
	RSC_LAYOUT_HEADER_PROC,
	RSC_LAYOUT_HEADER_CPU,
	RSC_LAYOUT_HEADER_ARCH,
	RSC_LAYOUT_HEADER_CACHE_L1,
	RSC_LAYOUT_HEADER_BCLK,
	RSC_LAYOUT_HEADER_CACHES,
	RSC_LAYOUT_RULER_LOAD,
	RSC_LAYOUT_RULER_REL_LOAD,
	RSC_LAYOUT_RULER_ABS_LOAD,
	RSC_LAYOUT_MONITOR_FREQUENCY,
	RSC_LAYOUT_MONITOR_INST,
	RSC_LAYOUT_MONITOR_COMMON,
	RSC_LAYOUT_MONITOR_TASKS,
	RSC_LAYOUT_MONITOR_SLICE,
	RSC_LAYOUT_RULER_FREQUENCY,
	RSC_LAYOUT_RULER_FREQUENCY_AVG,
	RSC_LAYOUT_RULER_FREQUENCY_PKG,
	RSC_LAYOUT_RULER_INST,
	RSC_LAYOUT_RULER_CYCLES,
	RSC_LAYOUT_RULER_CSTATES,
	RSC_LAYOUT_RULER_INTERRUPTS,
	RSC_LAYOUT_RULER_PACKAGE,
	RSC_LAYOUT_PACKAGE_PC,
	RSC_LAYOUT_PACKAGE_UNCORE,
	RSC_LAYOUT_RULER_TASKS,
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
	RSC_LAYOUT_RULER_SENSORS,
	RSC_LAYOUT_RULER_POWER,
	RSC_LAYOUT_RULER_VOLTAGE,
	RSC_LAYOUT_RULER_ENERGY,
	RSC_LAYOUT_RULER_SLICE,
	RSC_LAYOUT_FOOTER_TECH_X86,
	RSC_LAYOUT_FOOTER_TECH_INTEL,
	RSC_LAYOUT_FOOTER_TECH_AMD,
	RSC_LAYOUT_FOOTER_SYSTEM,
	RSC_LAYOUT_CARD_CORE_ONLINE_COND0,
	RSC_LAYOUT_CARD_CORE_ONLINE_COND1,
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
	RSC_SYSINFO_FEATURES_COND3,
	RSC_SYSINFO_FEATURES_COND4,
	RSC_SYSINFO_TECH_COND0,
	RSC_SYSINFO_TECH_COND1,
	RSC_SYSINFO_PERFMON_COND0,
	RSC_SYSINFO_PERFMON_COND1,
	RSC_SYSINFO_PERFMON_COND2,
	RSC_SYSINFO_PERFMON_COND3,
	RSC_SYSINFO_PERFMON_COND4,
	RSC_SYSINFO_PERFMON_HWP_CAP_COND0,
	RSC_SYSINFO_PERFMON_HWP_CAP_COND1,
	RSC_SYSINFO_PWR_THERMAL_COND0,
	RSC_SYSINFO_PWR_THERMAL_COND1,
	RSC_SYSINFO_PWR_THERMAL_COND2,
	RSC_SYSINFO_PWR_THERMAL_COND3,
	RSC_SYSINFO_PWR_THERMAL_COND4,
	RSC_SYSINFO_KERNEL,
	RSC_TOPOLOGY_COND0,
	RSC_TOPOLOGY_COND1,
	RSC_TOPOLOGY_COND2,
	RSC_MEMORY_CONTROLLER_COND0,
	RSC_MEMORY_CONTROLLER_COND1,
	RSC_CREATE_MENU_DISABLE,
	RSC_CREATE_MENU_FN_KEY,
	RSC_CREATE_MENU_SHORTKEY,
	RSC_CREATE_MENU_CTRL_KEY,
	RSC_CREATE_SETTINGS_COND0,
	RSC_CREATE_SETTINGS_COND1,
	RSC_CREATE_ADV_HELP_COND0,
	RSC_CREATE_ADV_HELP_COND1,
	RSC_CREATE_HOTPLUG_CPU_ENABLE,
	RSC_CREATE_HOTPLUG_CPU_DISABLE,
	RSC_CREATE_RATIO_CLOCK_COND0,
	RSC_CREATE_RATIO_CLOCK_COND1,
	RSC_CREATE_RATIO_CLOCK_COND2,
	RSC_CREATE_RATIO_CLOCK_COND3,
	RSC_CREATE_SELECT_CPU_COND0,
	RSC_CREATE_SELECT_CPU_COND1,
	RSC_HOT_EVENT_COND0,
	RSC_HOT_EVENT_COND1,
	RSC_HOT_EVENT_COND2,
	RSC_HOT_EVENT_COND3,
	RSC_HOT_EVENT_COND4,
	RSC_BOX_EVENT,
	RSC_CREATE_RECORDER,
	RSC_SMBIOS_ITEM,
	RSC_CREATE_SELECT_FREQ_PKG,
	RSC_CREATE_SELECT_FREQ_COND0,
	RSC_CREATE_SELECT_FREQ_COND1,
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
	RSC_PERFORMANCE,
	RSC_TARGET,
	RSC_LEVEL,
	RSC_PROGRAMMABLE,
	RSC_CONFIGURATION,
	RSC_TURBO_ACTIVATION,
	RSC_NOMINAL,
	RSC_UNLOCK,
	RSC_LOCK,
	RSC_ENABLE,
	RSC_DISABLE,
	RSC_CAPABILITIES,
	RSC_LOWEST,
	RSC_EFFICIENT,
	RSC_GUARANTEED,
	RSC_HIGHEST,
	RSC_SCOPE_NONE,
	RSC_SCOPE_THREAD,
	RSC_SCOPE_CORE,
	RSC_SCOPE_PACKAGE,
	RSC_CPUID_TITLE,
	RSC_LARGEST_STD_FUNC,
	RSC_LARGEST_EXT_FUNC,
	RSC_SYS_REGS_TITLE,
	RSC_ISA_TITLE,
	RSC_FEATURES_TITLE,
	RSC_NOT_AVAILABLE,
	RSC_AUTOMATIC,
	RSC_MISSING,
	RSC_PRESENT,
	RSC_VARIANT,
	RSC_INVARIANT,
	RSC_FEATURES_1GB_PAGES,
	RSC_FEATURES_100MHZ,
	RSC_FEATURES_ACPI,
	RSC_FEATURES_APIC,
	RSC_FEATURES_CORE_MP,
	RSC_FEATURES_CNXT_ID,
	RSC_FEATURES_DCA,
	RSC_FEATURES_DE,
	RSC_FEATURES_DS_PEBS,
	RSC_FEATURES_DS_CPL,
	RSC_FEATURES_DTES_64,
	RSC_FEATURES_FAST_STR,
	RSC_FEATURES_FMA,
	RSC_FEATURES_HLE,
	RSC_FEATURES_LM,
	RSC_FEATURES_LWP,
	RSC_FEATURES_MCA,
	RSC_FEATURES_MSR,
	RSC_FEATURES_MTRR,
	RSC_FEATURES_NX,
	RSC_FEATURES_OSXSAVE,
	RSC_FEATURES_PAE,
	RSC_FEATURES_PAT,
	RSC_FEATURES_PBE,
	RSC_FEATURES_PCID,
	RSC_FEATURES_PDCM,
	RSC_FEATURES_PGE,
	RSC_FEATURES_PSE,
	RSC_FEATURES_PSE36,
	RSC_FEATURES_PSN,
	RSC_FEATURES_RTM,
	RSC_FEATURES_SMX,
	RSC_FEATURES_SELF_SNOOP,
	RSC_FEATURES_SMEP,
	RSC_FEATURES_TSC,
	RSC_FEATURES_TSC_DEADLN,
	RSC_FEATURES_VME,
	RSC_FEATURES_VMX,
	RSC_FEATURES_X2APIC,
	RSC_FEATURES_XD_BIT,
	RSC_FEATURES_XSAVE,
	RSC_FEATURES_XTPR,
	RSC_FEAT_SECTION_MECH,
	RSC_TECHNOLOGIES_TITLE,
	RSC_TECHNOLOGIES_SMM,
	RSC_TECHNOLOGIES_HTT,
	RSC_TECHNOLOGIES_EIST,
	RSC_TECHNOLOGIES_IDA,
	RSC_TECHNOLOGIES_TURBO,
	RSC_TECHNOLOGIES_VM,
	RSC_TECHNOLOGIES_IOMMU,
	RSC_TECHNOLOGIES_SMT,
	RSC_TECHNOLOGIES_CNQ,
	RSC_TECHNOLOGIES_CPB,
	RSC_TECHNOLOGIES_HYPERV,
	RSC_PERF_MON_TITLE,
	RSC_VERSION,
	RSC_COUNTERS,
	RSC_GENERAL_CTRS,
	RSC_FIXED_CTRS,
	RSC_PERF_MON_C1E,
	RSC_PERF_MON_C1A,
	RSC_PERF_MON_C3A,
	RSC_PERF_MON_C1U,
	RSC_PERF_MON_C3U,
	RSC_PERF_MON_CC6,
	RSC_PERF_MON_PC6,
	RSC_PERF_MON_FID,
	RSC_PERF_MON_VID,
	RSC_PERF_MON_HWCF,
	RSC_PERF_MON_HWP,
	RSC_PERF_MON_HDC,
	RSC_PERF_MON_PKG_CSTATE,
	RSC_PERF_MON_CFG_CTRL,
	RSC_PERF_MON_LOW_CSTATE,
	RSC_PERF_MON_IOMWAIT,
	RSC_PERF_MON_MAX_CSTATE,
	RSC_PERF_MON_MONITOR_MWAIT,
	RSC_PERF_MON_MWAIT_IDX_CSTATE,
	RSC_PERF_MON_MWAIT_SUB_CSTATE,
	RSC_PERF_MON_CORE_CYCLE,
	RSC_PERF_MON_INST_RET,
	RSC_PERF_MON_REF_CYCLE,
	RSC_PERF_MON_REF_LLC,
	RSC_PERF_MON_MISS_LLC,
	RSC_PERF_MON_BRANCH_RET,
	RSC_PERF_MON_BRANCH_MIS,
	RSC_POWER_THERMAL_TITLE,
	RSC_POWER_THERMAL_ODCM,
	RSC_POWER_THERMAL_DUTY,
	RSC_POWER_THERMAL_MGMT,
	RSC_POWER_THERMAL_BIAS,
	RSC_POWER_THERMAL_TJMAX,
	RSC_POWER_THERMAL_DTS,
	RSC_POWER_THERMAL_PLN,
	RSC_POWER_THERMAL_PTM,
	RSC_POWER_THERMAL_TM1,
	RSC_POWER_THERMAL_TM2,
	RSC_POWER_THERMAL_UNITS,
	RSC_POWER_THERMAL_POWER,
	RSC_POWER_THERMAL_ENERGY,
	RSC_POWER_THERMAL_WINDOW,
	RSC_POWER_THERMAL_WATT,
	RSC_POWER_THERMAL_JOULE,
	RSC_POWER_THERMAL_SECOND,
	RSC_POWER_THERMAL_TDP,
	RSC_POWER_THERMAL_MIN,
	RSC_POWER_THERMAL_MAX,
	RSC_KERNEL_TITLE,
	RSC_KERNEL_TOTAL_RAM,
	RSC_KERNEL_SHARED_RAM,
	RSC_KERNEL_FREE_RAM,
	RSC_KERNEL_BUFFER_RAM,
	RSC_KERNEL_TOTAL_HIGH,
	RSC_KERNEL_FREE_HIGH,
	RSC_KERNEL_GOVERNOR,
	RSC_KERNEL_FREQ_DRIVER,
	RSC_KERNEL_IDLE_DRIVER,
	RSC_KERNEL_RELEASE,
	RSC_KERNEL_VERSION,
	RSC_KERNEL_MACHINE,
	RSC_KERNEL_MEMORY,
	RSC_KERNEL_STATE,
	RSC_KERNEL_POWER,
	RSC_KERNEL_LATENCY,
	RSC_KERNEL_RESIDENCY,
	RSC_KERNEL_LIMIT,
	RSC_TOPOLOGY_TITLE,
	RSC_MEM_CTRL_TITLE,
	RSC_MEM_CTRL_BLANK,
	RSC_MEM_CTRL_SUBSECT1_0,
	RSC_MEM_CTRL_SUBSECT1_1,
	RSC_MEM_CTRL_SUBSECT1_2,
	RSC_MEM_CTRL_SINGLE_CHA_0,
	RSC_MEM_CTRL_SINGLE_CHA_1,
	RSC_MEM_CTRL_SINGLE_CHA_2,
	RSC_MEM_CTRL_DUAL_CHA_0,
	RSC_MEM_CTRL_DUAL_CHA_1,
	RSC_MEM_CTRL_DUAL_CHA_2,
	RSC_MEM_CTRL_TRIPLE_CHA_0,
	RSC_MEM_CTRL_TRIPLE_CHA_1,
	RSC_MEM_CTRL_TRIPLE_CHA_2,
	RSC_MEM_CTRL_QUAD_CHA_0,
	RSC_MEM_CTRL_QUAD_CHA_1,
	RSC_MEM_CTRL_QUAD_CHA_2,
	RSC_MEM_CTRL_SIX_CHA_0,
	RSC_MEM_CTRL_SIX_CHA_1,
	RSC_MEM_CTRL_SIX_CHA_2,
	RSC_MEM_CTRL_EIGHT_CHA_0,
	RSC_MEM_CTRL_EIGHT_CHA_1,
	RSC_MEM_CTRL_EIGHT_CHA_2,
	RSC_MEM_CTRL_BUS_RATE_0,
	RSC_MEM_CTRL_BUS_RATE_1,
	RSC_MEM_CTRL_BUS_SPEED_0,
	RSC_MEM_CTRL_BUS_SPEED_1,
	RSC_MEM_CTRL_DRAM_SPEED_0,
	RSC_MEM_CTRL_DRAM_SPEED_1,
	RSC_MEM_CTRL_SUBSECT2_0,
	RSC_MEM_CTRL_SUBSECT2_1,
	RSC_MEM_CTRL_SUBSECT2_2,
	RSC_MEM_CTRL_SUBSECT2_3,
	RSC_MEM_CTRL_SUBSECT2_4,
	RSC_MEM_CTRL_SUBSECT2_5,
	RSC_MEM_CTRL_DIMM_SLOT,
	RSC_MEM_CTRL_DIMM_BANK,
	RSC_MEM_CTRL_DIMM_RANK,
	RSC_MEM_CTRL_DIMM_ROW,
	RSC_MEM_CTRL_DIMM_COLUMN0,
	RSC_MEM_CTRL_DIMM_COLUMN1,
	RSC_MEM_CTRL_DIMM_SIZE_0,
	RSC_MEM_CTRL_DIMM_SIZE_1,
	RSC_MEM_CTRL_DIMM_SIZE_2,
	RSC_MEM_CTRL_DIMM_SIZE_3,
	RSC_TASKS_SORTBY_STATE,
	RSC_TASKS_SORTBY_RTIME,
	RSC_TASKS_SORTBY_UTIME,
	RSC_TASKS_SORTBY_STIME,
	RSC_TASKS_SORTBY_PID,
	RSC_TASKS_SORTBY_COMM,
	RSC_MENU_ITEM_MENU,
	RSC_MENU_ITEM_VIEW,
	RSC_MENU_ITEM_WINDOW,
	RSC_MENU_ITEM_SETTINGS,
	RSC_MENU_ITEM_SMBIOS,
	RSC_MENU_ITEM_KERNEL,
	RSC_MENU_ITEM_HOTPLUG,
	RSC_MENU_ITEM_TOOLS,
	RSC_MENU_ITEM_ABOUT,
	RSC_MENU_ITEM_HELP,
	RSC_MENU_ITEM_KEYS,
	RSC_MENU_ITEM_LANG,
	RSC_MENU_ITEM_QUIT,
	RSC_MENU_ITEM_DASHBOARD,
	RSC_MENU_ITEM_FREQUENCY,
	RSC_MENU_ITEM_INST_CYCLES,
	RSC_MENU_ITEM_CORE_CYCLES,
	RSC_MENU_ITEM_IDLE_STATES,
	RSC_MENU_ITEM_PKG_CYCLES,
	RSC_MENU_ITEM_TASKS_MON,
	RSC_MENU_ITEM_SYS_INTER,
	RSC_MENU_ITEM_SENSORS,
	RSC_MENU_ITEM_VOLTAGE,
	RSC_MENU_ITEM_POWER,
	RSC_MENU_ITEM_SLICE_CTRS,
	RSC_MENU_ITEM_PROCESSOR,
	RSC_MENU_ITEM_TOPOLOGY,
	RSC_MENU_ITEM_FEATURES,
	RSC_MENU_ITEM_ISA_EXT,
	RSC_MENU_ITEM_TECH,
	RSC_MENU_ITEM_PERF_MON,
	RSC_MENU_ITEM_POW_THERM,
	RSC_MENU_ITEM_CPUID,
	RSC_MENU_ITEM_SYS_REGS,
	RSC_MENU_ITEM_MEM_CTRL,
	RSC_SETTINGS_TITLE,
	RSC_SETTINGS_DAEMON,
	RSC_SETTINGS_INTERVAL,
	RSC_SETTINGS_RECORDER,
	RSC_SETTINGS_AUTO_CLOCK,
	RSC_SETTINGS_EXPERIMENTAL,
	RSC_SETTINGS_CPU_HOTPLUG,
	RSC_SETTINGS_PCI_ENABLED,
	RSC_SETTINGS_NMI_REGISTERED,
	RSC_SETTINGS_CPUIDLE_REGISTERED,
	RSC_SETTINGS_CPUFREQ_REGISTERED,
	RSC_SETTINGS_GOVERNOR_REGISTERED,
	RSC_SETTINGS_THERMAL_SCOPE,
	RSC_SETTINGS_VOLTAGE_SCOPE,
	RSC_SETTINGS_POWER_SCOPE,
	RSC_HELP_TITLE,
	RSC_HELP_KEY_ESCAPE,
	RSC_HELP_KEY_SHIFT_TAB,
	RSC_HELP_KEY_TAB,
	RSC_HELP_KEY_UP,
	RSC_HELP_KEY_LEFT_RIGHT,
	RSC_HELP_KEY_DOWN,
	RSC_HELP_KEY_END,
	RSC_HELP_KEY_HOME,
	RSC_HELP_KEY_ENTER,
	RSC_HELP_KEY_PAGE_UP,
	RSC_HELP_KEY_PAGE_DOWN,
	RSC_HELP_KEY_MINUS,
	RSC_HELP_KEY_PLUS,
	RSC_HELP_BLANK,
	RSC_HELP_MENU,
	RSC_HELP_CLOSE_WINDOW,
	RSC_HELP_PREV_WINDOW,
	RSC_HELP_NEXT_WINDOW,
	RSC_HELP_MOVE_WINDOW,
	RSC_HELP_SIZE_WINDOW,
	RSC_HELP_MOVE_SELECT,
	RSC_HELP_LAST_CELL,
	RSC_HELP_FIRST_CELL,
	RSC_HELP_TRIGGER_SELECT,
	RSC_HELP_PREV_PAGE,
	RSC_HELP_NEXT_PAGE,
	RSC_HELP_SCROLL_DOWN,
	RSC_HELP_SCROLL_UP,
	RSC_ADV_HELP_TITLE,
	RSC_ADV_HELP_SECT_FREQ,
	RSC_ADV_HELP_ITEM_AVG,
	RSC_ADV_HELP_SECT_TASK,
	RSC_ADV_HELP_ITEM_ORDER,
	RSC_ADV_HELP_ITEM_SEL,
	RSC_ADV_HELP_ITEM_REV,
	RSC_ADV_HELP_ITEM_HIDE,
	RSC_ADV_HELP_SECT_ANY,
	RSC_ADV_HELP_ITEM_POWER,
	RSC_ADV_HELP_ITEM_TOP,
	RSC_ADV_HELP_ITEM_UPD,
	RSC_ADV_HELP_ITEM_START,
	RSC_ADV_HELP_ITEM_STOP,
	RSC_ADV_HELP_ITEM_TOOLS,
	RSC_ADV_HELP_ITEM_GO_UP,
	RSC_ADV_HELP_ITEM_GO_DW,
	RSC_ADV_HELP_ITEM_TERMINAL,
	RSC_ADV_HELP_ITEM_PRT_SCR,
	RSC_ADV_HELP_ITEM_REC_SCR,
	RSC_ADV_HELP_ITEM_FAHR_CELS,
	RSC_ADV_HELP_ITEM_PROC_EVENT,
	RSC_ADV_HELP_ITEM_SECRET,
	RSC_TURBO_CLOCK_TITLE,
	RSC_RATIO_CLOCK_TITLE,
	RSC_UNCORE_CLOCK_TITLE,
	RSC_SELECT_CPU_TITLE,
	RSC_SELECT_FREQ_TITLE,
	RSC_BOX_DISABLE_COND0,
	RSC_BOX_DISABLE_COND1,
	RSC_BOX_ENABLE_COND0,
	RSC_BOX_ENABLE_COND1,
	RSC_BOX_INTERVAL_TITLE,
	RSC_BOX_AUTO_CLOCK_TITLE,
	RSC_BOX_MODE_TITLE,
	RSC_BOX_MODE_DESC,
	RSC_BOX_EIST_DESC,
	RSC_BOX_C1E_DESC,
	RSC_BOX_TURBO_DESC,
	RSC_BOX_C1A_DESC,
	RSC_BOX_C3A_DESC,
	RSC_BOX_C1U_DESC,
	RSC_BOX_C3U_DESC,
	RSC_BOX_CC6_DESC,
	RSC_BOX_PC6_DESC,
	RSC_BOX_HWP_DESC,
	RSC_BOX_BLANK_DESC,
	RSC_BOX_NOMINAL_MODE_COND0,
	RSC_BOX_NOMINAL_MODE_COND1,
	RSC_BOX_EXPERIMENT_MODE_COND0,
	RSC_BOX_EXPERIMENT_MODE_COND1,
	RSC_BOX_INTERRUPT_TITLE,
	RSC_BOX_INT_REGISTER_COND0,
	RSC_BOX_INT_REGISTER_COND1,
	RSC_BOX_INT_UNREGISTER_COND0,
	RSC_BOX_INT_UNREGISTER_COND1,
	RSC_BOX_EVENT_TITLE,
	RSC_BOX_EVENT_THERMAL_SENSOR,
	RSC_BOX_EVENT_PROCHOT_AGENT,
	RSC_BOX_EVENT_CRITICAL_TEMP,
	RSC_BOX_EVENT_THERM_THRESHOLD,
	RSC_BOX_EVENT_POWER_LIMIT,
	RSC_BOX_EVENT_CURRENT_LIMIT,
	RSC_BOX_EVENT_CROSS_DOM_LIMIT,
	RSC_BOX_PKG_STATE_LIMIT_TITLE,
	RSC_BOX_IO_MWAIT_TITLE,
	RSC_BOX_IO_MWAIT_DESC,
	RSC_BOX_MWAIT_MAX_STATE_TITLE,
	RSC_BOX_ODCM_TITLE,
	RSC_BOX_ODCM_DESC,
	RSC_BOX_EXT_DUTY_CYCLE_TITLE,
	RSC_BOX_DUTY_CYCLE_TITLE,
	RSC_BOX_DUTY_CYCLE_RESERVED,
	RSC_BOX_POWER_POLICY_TITLE,
	RSC_BOX_POWER_POLICY_LOW,
	RSC_BOX_POWER_POLICY_HIGH,
	RSC_BOX_HWP_POLICY_MIN,
	RSC_BOX_HWP_POLICY_MED,
	RSC_BOX_HWP_POLICY_PWR,
	RSC_BOX_HWP_POLICY_MAX,
	RSC_BOX_TOOLS_TITLE,
	RSC_BOX_TOOLS_STOP_BURN,
	RSC_BOX_TOOLS_ATOMIC_BURN,
	RSC_BOX_TOOLS_CRC32_BURN,
	RSC_BOX_TOOLS_CONIC_BURN,
	RSC_BOX_TOOLS_RANDOM_CPU,
	RSC_BOX_TOOLS_ROUND_ROBIN_CPU,
	RSC_BOX_TOOLS_USER_CPU,
	RSC_BOX_CONIC_TITLE,
	RSC_BOX_CONIC_ITEM_1,
	RSC_BOX_CONIC_ITEM_2,
	RSC_BOX_CONIC_ITEM_3,
	RSC_BOX_CONIC_ITEM_4,
	RSC_BOX_CONIC_ITEM_5,
	RSC_BOX_CONIC_ITEM_6,
	RSC_BOX_LANG_TITLE,
	RSC_BOX_LANG_ENGLISH,
	RSC_BOX_LANG_FRENCH,
	RSC_BOX_SCOPE_THERMAL_TITLE,
	RSC_BOX_SCOPE_VOLTAGE_TITLE,
	RSC_BOX_SCOPE_POWER_TITLE,
	RSC_BOX_SCOPE_NONE,
	RSC_BOX_SCOPE_THREAD,
	RSC_BOX_SCOPE_CORE,
	RSC_BOX_SCOPE_PACKAGE,
	RSC_ERROR_CMD_SYNTAX,
	RSC_ERROR_SHARED_MEM,
	RSC_ERROR_SYS_CALL,
	RSC_BOX_IDLE_LIMIT_TITLE,
	RSC_BOX_RECORDER_TITLE,
	RSC_SMBIOS_TITLE,
	RSC_MECH_IBRS,
	RSC_MECH_IBPB,
	RSC_MECH_STIBP,
	RSC_MECH_SSBD,
	RSC_MECH_L1D_FLUSH,
	RSC_MECH_MD_CLEAR,
	RSC_MECH_RDCL_NO,
	RSC_MECH_IBRS_ALL,
	RSC_MECH_RSBA,
	RSC_MECH_L1DFL_VMENTRY_NO,
	RSC_MECH_SSB_NO,
	RSC_MECH_MDS_NO,
	RSC_MECH_PSCHANGE_MC_NO,
	RSC_MECH_TAA_NO,
	RSC_LOGO_ROW_0,
	RSC_LOGO_ROW_1,
	RSC_LOGO_ROW_2,
	RSC_LOGO_ROW_3,
	RSC_LOGO_ROW_4,
	RSC_LOGO_ROW_5,
	RSC_COPY_ROW_0,
	RSC_COPY_ROW_1,
	RSC_COPY_ROW_2,
	RSC_CREATE_SELECT_FREQ_TGT,
	RSC_CREATE_SELECT_FREQ_HWP_TGT,
	RSC_CREATE_SELECT_FREQ_HWP_MAX,
	RSC_CREATE_SELECT_FREQ_HWP_MIN
};

typedef struct {
	ATTRIBUTE	*Attr;
	ASCII		*Code[LOC_CNT];
	const int	Size[LOC_CNT];
} RESOURCE_ST;

extern RESOURCE_ST Resource[];

#define ATTR() Attr

#define CODE() Code[GET_LOCALE()]

#define RSC(_ID) (Resource[RSC_##_ID])

#define RSZ(_ID) (RSC(_ID).Size[GET_LOCALE()])

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
	HDK,HWK,HWK,HWK,HWK,HDK,HWK,HWK,HWK,HWK,LWK,LWK,LWK		\
}

#define RSC_LAYOUT_HEADER_CPU_CODE					\
{									\
	']',' ',' ',' ',' ','/',' ',' ',' ',' ','C','P','U'		\
}

#define RSC_LAYOUT_HEADER_ARCH_ATTR					\
{									\
	HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK	\
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

#define RSC_LAYOUT_RULER_LOAD_ATTR					\
{									\
	LWK,LWK,LWK,LWK,_HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
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

#define RSC_LAYOUT_RULER_VAR_LOAD_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK 							\
}

#define RSC_LAYOUT_RULER_LOAD_CODE					\
{									\
	'-','-','-',' ', '!',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ','-','-','-','-','-','-','-',\
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
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK 		\
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

#define RSC_LAYOUT_MONITOR_SLICE_ATTR					\
{									\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HRK,HRK,HRK,\
	HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK,HRK	\
}

#define RSC_LAYOUT_MONITOR_SLICE_CODE					\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '	\
}

#define RSC_LAYOUT_RULER_FREQUENCY_ATTR 				\
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

#define RSC_LAYOUT_RULER_FREQUENCY_CODE 				\
{									\
	'-','-','-',' ','F','r','e','q','(','M','H','z',')',' ','R','a',\
	't','i','o',' ','-',' ','T','u','r','b','o',' ','-','-','-',' ',\
	'C','0',' ','-','-','-','-',' ','C','1',' ','-','-','-','-',' ',\
	'C','3',' ','-','-','-','-',' ','C','6',' ','-','-','-','-',' ',\
	'C','7',' ','-','-','M','i','n',' ','T','M','P',' ','M','a','x',\
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

#define RSC_LAYOUT_RULER_FREQUENCY_AVG_ATTR				\
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

#define RSC_LAYOUT_RULER_FREQUENCY_PKG_ATTR				\
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

#define RSC_LAYOUT_RULER_FREQUENCY_PKG_CODE				\
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

#define RSC_LAYOUT_RULER_INST_CODE					\
{									\
	'-','-','-','-','-','-','-','-','-','-','-','-',' ','I','P','S',\
	' ','-','-','-','-','-','-','-','-','-','-','-','-','-','-',' ',\
	'I','P','C',' ','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-',' ','C','P','I',' ','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-',' ','I','N','S','T',' ','-',\
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

#define RSC_LAYOUT_RULER_CYCLES_CODE					\
{									\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-',' ','C',\
	'0',':','U','C','C',' ','-','-','-','-','-','-','-','-','-','-',\
	' ','C','0',':','U','R','C',' ','-','-','-','-','-','-','-','-',\
	'-','-','-','-',' ','C','1',' ','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-',' ','T','S','C',' ','-','-','-','-','-','-',\
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

#define RSC_LAYOUT_RULER_CSTATES_CODE					\
{									\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	' ','C','1',' ','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-',' ','C','3',' ','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-',' ','C','6',' ','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-',' ','C','7',' ','-','-','-','-','-','-',\
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

#define RSC_LAYOUT_RULER_INTERRUPTS_ATTR				\
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

#define RSC_LAYOUT_RULER_INTERRUPTS_CODE				\
{									\
	'-','-','-','-','-','-','-','-','-','-',' ','S','M','I',' ','-',\
	'-','-','-','-','-','-','-','-','-','-','-',' ','N','M','I','[',\
	' ','L','O','C','A','L',' ',' ',' ','U','N','K','N','O','W','N',\
	' ',' ','P','C','I','_','S','E','R','R','#',' ',' ','I','O','_',\
	'C','H','E','C','K',']',' ','-','-','-','-','-','-','-','-','-',\
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

#define RSC_LAYOUT_RULER_TASKS_ATTR					\
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

#define RSC_LAYOUT_RULER_SENSORS_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,HDK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
	LWK,HDK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,HDK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,LWK,HDK,LWK,LWK,LWK,\
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

#define RSC_LAYOUT_RULER_POWER_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,\
	HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HWK,HWK,HWK,HWK,\
	HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,\
	HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,HDK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,HDK,LWK,HDK,LWK,LWK,LWK,\
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

#define RSC_LAYOUT_RULER_VOLTAGE_ATTR					\
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

#define RSC_LAYOUT_RULER_ENERGY_ATTR					\
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

#define RSC_LAYOUT_RULER_SLICE_ATTR					\
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
	HDK,HDK,LCK,LCK,LCK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_CORE_ONLINE_COND0_CODE				\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ','C',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CORE_ONLINE_COND1_CODE				\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ','F',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CORE_OFFLINE_ATTR				\
{									\
	HDK,HDK,LBK,LBK,LBK,HDK,HDK,LWK,LWK,LWK,HDK,HDK 		\
}

#define RSC_LAYOUT_CARD_CORE_OFFLINE_CODE				\
{									\
	'[',' ',' ',' ',' ',' ',' ','O','F','F',' ',']' 		\
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

#define RSC_LAYOUT_CARD_MC_CODE 					\
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
	LWK,LWK,LWK,LWK,LWK,LCK,LCK,LCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 			\
}

#define RSC_SYSINFO_CPUID_COND1_ATTR					\
{									\
	HBK,HBK,HBK,HBK,HBK,HBK,HBK,HBK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
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
	LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,LWK,LWK 		\
}

#define RSC_SYSINFO_PROC_COND2_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK 		\
}

#define RSC_SYSINFO_PROC_COND3_ATTR					\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,HWK,HWK,HWK,HWK,HWK,HWK,HWK,LWK,LWK 		\
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
	LWK,LWK,LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,			\
	LWK,LWK 							\
}

#define RSC_SYSINFO_FEATURES_COND3_ATTR 				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,HGK,HGK,HGK,HGK,HGK,HGK,HGK,			\
	LWK,LWK 							\
}

#define RSC_SYSINFO_FEATURES_COND4_ATTR 				\
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

#define RSC_SYSINFO_PERFMON_COND4_ATTR					\
{									\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK 						\
}

#define RSC_SYSINFO_PERFMON_HWP_CAP_COND0_ATTR				\
{									\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 		\
}

#define RSC_SYSINFO_PERFMON_HWP_CAP_COND1_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 		\
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

#define RSC_SYSINFO_PWR_THERMAL_COND4_ATTR				\
{									\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,			\
	HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK,HDK 			\
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
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 				\
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

#define RSC_CREATE_MENU_DISABLE_ATTR					\
{									\
	HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,		\
	HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW,HKW 		\
}

#define RSC_CREATE_MENU_ITEM_MENU_ATTR					\
{									\
	LKW,LKW,LKW,LKW,LKW,HKW,_LKW,_LKW,HKW,LKW,LKW,LKW,		\
	LKW,LKW,LKW,LKW,LKW,LKW, LKW, LKW,LKW,LKW,LKW,LKW		\
}

#define RSC_CREATE_MENU_ITEM_VIEW_ATTR					\
{									\
	LKW,LKW,LKW,LKW,LKW,HKW,_LKW,_LKW,HKW,LKW,LKW,LKW,		\
	LKW,LKW,LKW,LKW,LKW,LKW, LKW, LKW,LKW,LKW,LKW,LKW		\
}

#define RSC_CREATE_MENU_ITEM_WINDOW_ATTR				\
{									\
	LKW,LKW,LKW,LKW,HKW,_LKW,_LKW,HKW,LKW,LKW,LKW,LKW,		\
	LKW,LKW,LKW,LKW,LKW, LKW, LKW,LKW,LKW,LKW,LKW,LKW		\
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

#define RSC_CREATE_MENU_CTRL_KEY_ATTR					\
{									\
	LKW,LKW, LKW, LKW, LKW, LKW,LKW,LKW,LKW, LKW,LKW,LKW,		\
	LKW,HKW,_LKW,_LKW,_LKW,_LKW,HKW,HKW,HKW,_LKW,HKW,LKW		\
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
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 				\
}

#define RSC_CREATE_ADV_HELP_COND1_ATTR					\
{									\
	LWK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,HCK,			\
	HCK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK 				\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_ATTR				\
{									\
	LWK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LGK,LWK,LDK,LDK,LDK	\
}

#define RSC_CREATE_HOTPLUG_CPU_DISABLE_ATTR				\
{									\
	LWK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,LCK,LWK,LDK,LDK,LDK	\
}

#define RSC_CREATE_RATIO_CLOCK_COND0_ATTR				\
{									\
	LWK,HWK,HWK,HWK,HWK,LWK,HWK,HWK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK	\
}

#define RSC_CREATE_RATIO_CLOCK_COND1_ATTR				\
{									\
	LWK,HBK,HBK,HBK,HBK,LBK,HBK,HBK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK	\
}

#define RSC_CREATE_RATIO_CLOCK_COND2_ATTR				\
{									\
	LWK,HRK,HRK,HRK,HRK,LRK,HRK,HRK,LWK,HDK,HDK,HDK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK	\
}

#define RSC_CREATE_RATIO_CLOCK_COND3_ATTR				\
{									\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,	\
	LWK,LWK,HWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK	\
}

#define RSC_CREATE_SELECT_CPU_COND0_ATTR				\
{									\
	LWK,LWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK		\
}

#define RSC_CREATE_SELECT_CPU_COND1_ATTR				\
{									\
	LBK,LBK,HBK,HBK,HBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,		\
	LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK		\
}

#define RSC_HOT_EVENT_COND0_ATTR					\
{									\
	HDK,HDK,HDK							\
}

#define RSC_HOT_EVENT_COND1_ATTR					\
{									\
	HRK,HRK,HRK							\
}

#define RSC_HOT_EVENT_COND2_ATTR					\
{									\
	{.fg=CYAN,.bg=BLACK,.bf=1,.un=1},				\
	{.fg=CYAN,.bg=BLACK,.bf=0,.un=0},				\
	{.fg=CYAN,.bg=BLACK,.bf=0,.un=0}				\
}

#define RSC_HOT_EVENT_COND3_ATTR					\
{									\
	HWK,HWK,HWK							\
}

#define RSC_HOT_EVENT_COND4_ATTR					\
{									\
	{.fg=CYAN,.bg=MAGENTA,.bf=0,.un=1},				\
	{.fg=CYAN,.bg=MAGENTA,.bf=0,.un=0},				\
	{.fg=CYAN,.bg=MAGENTA,.bf=0,.un=0}				\
}

#define RSC_BOX_EVENT_ATTR						\
{									\
	LWK,								\
	LMK,								\
	HYK								\
}

#define RSC_CREATE_RECORDER_ATTR					\
{									\
	LWK,LWK,LWK,LWK,HDK,LWK,LWK,HDK,LWK,LWK,LWK,LWK 		\
}

#define RSC_SMBIOS_ITEM_ATTR						\
{									\
	HDK,LCK,LCK,HDK,HDK,						\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK \
}

#define RSC_CREATE_SETTINGS_COND_CODE	"                                "

#define RSC_CREATE_HELP_BLANK_CODE	"                  "

#define RSC_CREATE_ADV_HELP_BLANK_CODE	"                                      "

#define RSC_LOGO_R0 "      ______                ______                  "
#define RSC_LOGO_R1 "     / ____/___  ________  / ____/_______  ____ _   "
#define RSC_LOGO_R2 "    / /   / __ \\/ ___/ _ \\/ /_  / ___/ _ \\/ __ `/   "
#define RSC_LOGO_R3 "   / /___/ /_/ / /  /  __/ __/ / /  /  __/ /_/ /    "
#define RSC_LOGO_R4 "   \\____/\\____/_/   \\___/_/   /_/   \\___/\\__, /     "
#define RSC_LOGO_R5 "                                           /_/      "
#define RSC_COPY_R0 "      by CyrIng                                     "
#define RSC_COPY_R1 "                                                    "
#define RSC_COPY_R2 "            (C)2015-2020 CYRIL INGENIERIE           "

#define RSC_CREATE_SELECT_FREQ_PKG_ATTR 				\
{									\
	HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,HYK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HYK,HYK,HYK,HYK,HYK,HYK,\
	HYK,HYK,HDK,HDK,HDK,LWK,LWK,HYK,HYK,HYK,HYK,HYK,LWK,LWK 	\
}

#define RSC_CREATE_SELECT_FREQ_COND0_ATTR				\
{									\
	LWK,LWK,HWK,HWK,HWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,		\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,HDK,HDK,HDK,			\
	LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK,LWK				\
}

#define RSC_CREATE_SELECT_FREQ_COND1_ATTR				\
{									\
	LBK,LBK,HBK,HBK,HBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,		\
	LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,		\
	LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,			\
	LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK,LBK				\
}

