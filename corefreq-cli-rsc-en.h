/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define RSC_LAYOUT_HEADER_PROC_CODE_EN					\
{									\
	' ','P','r','o','c','e','s','s','o','r',' ','[' 		\
}

#define RSC_LAYOUT_HEADER_BCLK_CODE_EN					\
{									\
	' ','B','a','s','e',' ','C','l','o','c','k',' ',		\
	'~',' ','0','0','0',' ','0','0','0',' ','0','0','0',' ','H','z' \
}

#define RSC_LAYOUT_RULER_REL_LOAD_CODE_EN				\
{									\
	'R','e','l','a','t','i','v','e',' ','f','r','e','q','u','e','n',\
	'c','y' 							\
}

#define RSC_LAYOUT_RULER_ABS_LOAD_CODE_EN				\
{									\
	'A','b','s','o','l','u','t','e',' ','f','r','e','q','u','e','n',\
	'c','y' 							\
}

#define RSC_LAYOUT_RULER_FREQUENCY_AVG_CODE_EN				\
{									\
	'-','-','-','-','-','-',' ','%',' ','A','v','e','r','a','g','e',\
	's',' ','[',' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',']','-','-','-','-','-','-','-','-','-','-','-',\
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

#define RSC_LAYOUT_RULER_PACKAGE_CODE_EN				\
	"------------ Cycles ---- State -------------------- TSC Rati"	\
	"o ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_TASKS_CODE_EN					\
	"--- Freq(MHz) --- Tasks                    -----------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_TASKS_STATE_SORTED_CODE_EN				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','S','t','a','t','e',')',' ', '-','-','-'			\
}

#define RSC_LAYOUT_TASKS_RUNTIME_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','R','u','n','T','i','m','e', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_USRTIME_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','U','s','e','r','T','i','m', 'e',')',' '			\
}

#define RSC_LAYOUT_TASKS_SYSTIME_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','S','y','s','T','i','m','e', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_PROCESS_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','P','I','D',')',' ','-','-', '-','-','-'			\
}

#define RSC_LAYOUT_TASKS_COMMAND_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','C','o','m','m','a','n','d', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_EN			\
{									\
	' ', 'I','n','v','e','r','s','e',' ','[','O','F','F',']',' '	\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_ON_CODE_EN			\
{									\
	' ', 'I','n','v','e','r','s','e',' ','[',' ','O','N',']',' '	\
}

#define RSC_LAYOUT_TASKS_VALUE_SWITCH_CODE_EN				\
{									\
	' ', 'V','a','l','u','e',' ','[',' ',' ',' ',']',' '		\
}

#define RSC_LAYOUT_TASKS_TRACKING_CODE_EN				\
{									\
	' ','T','r','a','c','k','i', 'n','g',' ','P','I','D',' ','[',' ',\
	'O','F','F',' ',']',' ' 					\
}

#define RSC_LAYOUT_RULER_SENSORS_CODE_EN				\
	"--- Freq(MHz) --- Vcore --- TMP( ) --- Energy(J) --- Power(W"	\
	") ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_POWER_CODE_EN					\
	"---- RAM:   .     -- Uncore:   .     -- Package:   .     -- "	\
	"Cores:   .    ( ) ------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_VOLTAGE_CODE_EN				\
	"--- Freq(MHz) - VID --- Min ---- Vcore -- Max --------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_ENERGY_CODE_EN 				\
	"--- Freq(MHz) -- Accumulator --- Min - Energy(J) - Max -- Mi"	\
	"n - Power(W) - Max -----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_SLICE_CODE_EN					\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 ----------- Error -----------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_EN				\
{									\
	'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','B',']' 	\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_EN	"<   ENABLE >"
#define RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_EN	"<  DISABLE >"

#define RSC_LAYOUT_CARD_LOAD_CODE_EN					\
{									\
	'[',' ',' ','%','L','O','A','D',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_IDLE_CODE_EN					\
{									\
	'[',' ',' ','%','I','D','L','E',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_TASK_CODE_EN					\
{									\
	'[','T','a','s','k','s',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_PROCESSOR_TITLE_CODE_EN	" Processor "
#define RSC_PROCESSOR_CODE_EN		"Processor"
#define RSC_ARCHITECTURE_CODE_EN	"Architecture"
#define RSC_VENDOR_ID_CODE_EN		"Vendor ID"
#define RSC_MICROCODE_CODE_EN		"Microcode"
#define RSC_SIGNATURE_CODE_EN		"Signature"
#define RSC_STEPPING_CODE_EN		"Stepping"
#define RSC_ONLINE_CPU_CODE_EN		"Online CPU"
#define RSC_BASE_CLOCK_CODE_EN		"Base Clock"
#define RSC_FREQUENCY_CODE_EN		"Frequency"
#define RSC_RATIO_CODE_EN		"Ratio"
#define RSC_FACTORY_CODE_EN		"Factory"
#define RSC_PERFORMANCE_CODE_EN 	"Performance"
#define RSC_TARGET_CODE_EN		"Target"
#define RSC_LEVEL_CODE_EN		"Level"
#define RSC_PROGRAMMABLE_CODE_EN	"Programmable"
#define RSC_CONFIGURATION_CODE_EN	"Configuration"
#define RSC_TURBO_ACTIVATION_CODE_EN	"Turbo Activation"
#define RSC_NOMINAL_CODE_EN		"Nominal"
#define RSC_UNLOCK_CODE_EN		"UNLOCK"
#define RSC_LOCK_CODE_EN		"  LOCK"
#define RSC_ENABLE_CODE_EN		" Enable"
#define RSC_DISABLE_CODE_EN		"Disable"
#define RSC_CAPABILITIES_CODE_EN	"Capabilities"
#define RSC_LOWEST_CODE_EN		"Lowest"
#define RSC_EFFICIENT_CODE_EN		"Efficient"
#define RSC_GUARANTEED_CODE_EN		"Guaranteed"
#define RSC_HIGHEST_CODE_EN		"Highest"

#define RSC_SCOPE_NONE_CODE_EN		"None"
#define RSC_SCOPE_THREAD_CODE_EN	" SMT"
#define RSC_SCOPE_CORE_CODE_EN		"Core"
#define RSC_SCOPE_PACKAGE_CODE_EN	" Pkg"

#define RSC_CPUID_TITLE_EN		\
	" function           EAX          EBX          ECX          EDX "

#define RSC_LARGEST_STD_FUNC_CODE_EN	"Largest Standard Function"
#define RSC_LARGEST_EXT_FUNC_CODE_EN	"Largest Extended Function"

#define RSC_SYS_REGS_TITLE_CODE_EN	" System Registers "

#define RSC_ISA_TITLE_CODE_EN		" Instruction Set Extensions "

#define RSC_FEATURES_TITLE_CODE_EN	" Features "
#define RSC_NOT_AVAILABLE_CODE_EN	"N/A"
#define RSC_AUTOMATIC_CODE_EN		"AUTO"
#define RSC_MISSING_CODE_EN		"Missing"
#define RSC_PRESENT_CODE_EN		"Capable"
#define RSC_VARIANT_CODE_EN		"Variant"
#define RSC_INVARIANT_CODE_EN		"Invariant"
#define RSC_FEATURES_1GB_PAGES_CODE_EN	"1 GB Pages Support"
#define RSC_FEATURES_100MHZ_CODE_EN	"100 MHz multiplier Control"
#define RSC_FEATURES_ACPI_CODE_EN   "Advanced Configuration & Power Interface"
#define RSC_FEATURES_APIC_CODE_EN   "Advanced Programmable Interrupt Controller"
#define RSC_FEATURES_CORE_MP_CODE_EN	"Core Multi-Processing"
#define RSC_FEATURES_CNXT_ID_CODE_EN	"L1 Data Cache Context ID"
#define RSC_FEATURES_DCA_CODE_EN	"Direct Cache Access"
#define RSC_FEATURES_DE_CODE_EN 	"Debugging Extension"
#define RSC_FEATURES_DS_PEBS_CODE_EN "Debug Store & Precise Event Based Sampling"
#define RSC_FEATURES_DS_CPL_CODE_EN	"CPL Qualified Debug Store"
#define RSC_FEATURES_DTES_64_CODE_EN	"64-Bit Debug Store"
#define RSC_FEATURES_FAST_STR_CODE_EN	"Fast-String Operation"
#define RSC_FEATURES_FMA_CODE_EN	"Fused Multiply Add"
#define RSC_FEATURES_HLE_CODE_EN	"Hardware Lock Elision"
#define RSC_FEATURES_LM_CODE_EN 	"Long Mode 64 bits"
#define RSC_FEATURES_LWP_CODE_EN	"LightWeight Profiling"
#define RSC_FEATURES_MCA_CODE_EN	"Machine-Check Architecture"
#define RSC_FEATURES_MSR_CODE_EN	"Model Specific Registers"
#define RSC_FEATURES_MTRR_CODE_EN	"Memory Type Range Registers"
#define RSC_FEATURES_NX_CODE_EN 	"No-Execute Page Protection"
#define RSC_FEATURES_OSXSAVE_CODE_EN	"OS-Enabled Ext. State Management"
#define RSC_FEATURES_PAE_CODE_EN	"Physical Address Extension"
#define RSC_FEATURES_PAT_CODE_EN	"Page Attribute Table"
#define RSC_FEATURES_PBE_CODE_EN	"Pending Break Enable"
#define RSC_FEATURES_PCID_CODE_EN	"Process Context Identifiers"
#define RSC_FEATURES_PDCM_CODE_EN	"Perfmon and Debug Capability"
#define RSC_FEATURES_PGE_CODE_EN	"Page Global Enable"
#define RSC_FEATURES_PSE_CODE_EN	"Page Size Extension"
#define RSC_FEATURES_PSE36_CODE_EN	"36-bit Page Size Extension"
#define RSC_FEATURES_PSN_CODE_EN	"Processor Serial Number"
#define RSC_FEATURES_RTM_CODE_EN	"Restricted Transactional Memory"
#define RSC_FEATURES_SMX_CODE_EN	"Safer Mode Extensions"
#define RSC_FEATURES_SELF_SNOOP_CODE_EN "Self-Snoop"
#define RSC_FEATURES_SMEP_CODE_EN	"Supervisor-Mode Execution Prevention"
#define RSC_FEATURES_TSC_CODE_EN	"Time Stamp Counter"
#define RSC_FEATURES_TSC_DEADLN_CODE_EN "Time Stamp Counter Deadline"
#define RSC_FEATURES_VME_CODE_EN	"Virtual Mode Extension"
#define RSC_FEATURES_VMX_CODE_EN	"Virtual Machine Extensions"
#define RSC_FEATURES_X2APIC_CODE_EN	"Extended xAPIC Support"
#define RSC_FEATURES_XD_BIT_CODE_EN	"Execution Disable Bit Support"
#define RSC_FEATURES_XSAVE_CODE_EN	"XSAVE/XSTOR States"
#define RSC_FEATURES_XTPR_CODE_EN	"xTPR Update Control"
#define RSC_FEAT_SECTION_MECH_CODE_EN	"Mitigation mechanisms"

#define RSC_TECHNOLOGIES_TITLE_CODE_EN	" Technologies "
#define RSC_TECHNOLOGIES_SMM_CODE_EN	"System Management Mode"
#define RSC_TECHNOLOGIES_HTT_CODE_EN	"Hyper-Threading"
#define RSC_TECHNOLOGIES_EIST_CODE_EN	"SpeedStep"
#define RSC_TECHNOLOGIES_IDA_CODE_EN	"Dynamic Acceleration"
#define RSC_TECHNOLOGIES_TURBO_CODE_EN	"Turbo Boost"
#define RSC_TECHNOLOGIES_VM_CODE_EN	"Virtualization"
#define RSC_TECHNOLOGIES_IOMMU_CODE_EN	"I/O MMU"
#define RSC_TECHNOLOGIES_SMT_CODE_EN	"Simultaneous Multithreading"
#define RSC_TECHNOLOGIES_CNQ_CODE_EN	"PowerNow!"
#define RSC_TECHNOLOGIES_CPB_CODE_EN	"Core Performance Boost"
#define RSC_TECHNOLOGIES_HYPERV_CODE_EN "Hypervisor"

#define RSC_PERF_MON_TITLE_CODE_EN	" Performance Monitoring "
#define RSC_VERSION_CODE_EN		"Version"
#define RSC_COUNTERS_CODE_EN		"Counters"
#define RSC_GENERAL_CTRS_CODE_EN	"General"
#define RSC_FIXED_CTRS_CODE_EN		"Fixed"
#define RSC_PERF_MON_C1E_CODE_EN	"Enhanced Halt State"
#define RSC_PERF_MON_C1A_CODE_EN	"C1 Auto Demotion"
#define RSC_PERF_MON_C3A_CODE_EN	"C3 Auto Demotion"
#define RSC_PERF_MON_C1U_CODE_EN	"C1 UnDemotion"
#define RSC_PERF_MON_C3U_CODE_EN	"C3 UnDemotion"
#define RSC_PERF_MON_CC6_CODE_EN	"Core C6 State"
#define RSC_PERF_MON_PC6_CODE_EN	"Package C6 State"
#define RSC_PERF_MON_FID_CODE_EN	"Frequency ID control"
#define RSC_PERF_MON_VID_CODE_EN	"Voltage ID control"
#define RSC_PERF_MON_HWCF_CODE_EN	"P-State Hardware Coordination Feedback"
#define RSC_PERF_MON_HWP_CODE_EN	"Hardware-Controlled Performance States"
#define RSC_PERF_MON_HDC_CODE_EN	"Hardware Duty Cycling"
#define RSC_PERF_MON_PKG_CSTATE_CODE_EN "Package C-State"
#define RSC_PERF_MON_CFG_CTRL_CODE_EN	"Configuration Control"
#define RSC_PERF_MON_LOW_CSTATE_CODE_EN "Lowest C-State"
#define RSC_PERF_MON_IOMWAIT_CODE_EN	"I/O MWAIT Redirection"
#define RSC_PERF_MON_MAX_CSTATE_CODE_EN "Max C-State Inclusion"

#define RSC_PERF_MON_MONITOR_MWAIT_CODE_EN	"MONITOR/MWAIT"
#define RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_EN	"State index"
#define RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_EN	"Sub C-State"

#define RSC_PERF_MON_CORE_CYCLE_CODE_EN "Core Cycles"
#define RSC_PERF_MON_INST_RET_CODE_EN	"Instructions Retired"
#define RSC_PERF_MON_REF_CYCLE_CODE_EN	"Reference Cycles"
#define RSC_PERF_MON_REF_LLC_CODE_EN	"Last Level Cache References"
#define RSC_PERF_MON_MISS_LLC_CODE_EN	"Last Level Cache Misses"
#define RSC_PERF_MON_BRANCH_RET_CODE_EN "Branch Instructions Retired"
#define RSC_PERF_MON_BRANCH_MIS_CODE_EN "Branch Mispredicts Retired"

#define RSC_POWER_THERMAL_TITLE_CODE_EN " Power & Thermal "
#define RSC_POWER_THERMAL_ODCM_CODE_EN	"Clock Modulation"
#define RSC_POWER_THERMAL_DUTY_CODE_EN	"DutyCycle"
#define RSC_POWER_THERMAL_MGMT_CODE_EN	"Power Management"
#define RSC_POWER_THERMAL_BIAS_CODE_EN	"Energy Policy"
#define RSC_POWER_THERMAL_TJMAX_CODE_EN "Junction Temperature"
#define RSC_POWER_THERMAL_DTS_CODE_EN	"Digital Thermal Sensor"
#define RSC_POWER_THERMAL_PLN_CODE_EN	"Power Limit Notification"
#define RSC_POWER_THERMAL_PTM_CODE_EN	"Package Thermal Management"
#define RSC_POWER_THERMAL_TM1_CODE_EN	"Thermal Monitor 1"
#define RSC_POWER_THERMAL_TM2_CODE_EN	"Thermal Monitor 2"
#define RSC_POWER_THERMAL_UNITS_CODE_EN "Units"
#define RSC_POWER_THERMAL_POWER_CODE_EN "Power"
#define RSC_POWER_THERMAL_ENERGY_CODE_EN "Energy"
#define RSC_POWER_THERMAL_WINDOW_CODE_EN "Window"
#define RSC_POWER_THERMAL_WATT_CODE_EN	"watt"
#define RSC_POWER_THERMAL_JOULE_CODE_EN "joule"
#define RSC_POWER_THERMAL_SECOND_CODE_EN "second"
#define RSC_POWER_THERMAL_TDP_CODE_EN	"Thermal Design Power"
#define RSC_POWER_THERMAL_MIN_CODE_EN	"Minimum Power"
#define RSC_POWER_THERMAL_MAX_CODE_EN	"Maximum Power"

#define RSC_KERNEL_TITLE_CODE_EN	" Kernel "
#define RSC_KERNEL_TOTAL_RAM_CODE_EN	"Total RAM"
#define RSC_KERNEL_SHARED_RAM_CODE_EN	"Shared RAM"
#define RSC_KERNEL_FREE_RAM_CODE_EN	"Free RAM"
#define RSC_KERNEL_BUFFER_RAM_CODE_EN	"Buffer RAM"
#define RSC_KERNEL_TOTAL_HIGH_CODE_EN	"Total High"
#define RSC_KERNEL_FREE_HIGH_CODE_EN	"Free High"
#define RSC_KERNEL_GOVERNOR_CODE_EN	"Governor"
#define RSC_KERNEL_FREQ_DRIVER_CODE_EN	"CPU-Freq driver"
#define RSC_KERNEL_IDLE_DRIVER_CODE_EN	"CPU-Idle driver"
#define RSC_KERNEL_RELEASE_CODE_EN	"Release"
#define RSC_KERNEL_VERSION_CODE_EN	"Version"
#define RSC_KERNEL_MACHINE_CODE_EN	"Machine"
#define RSC_KERNEL_MEMORY_CODE_EN	"Memory"
#define RSC_KERNEL_STATE_CODE_EN	"State"
#define RSC_KERNEL_POWER_CODE_EN	"Power"
#define RSC_KERNEL_LATENCY_CODE_EN	"Latency"
#define RSC_KERNEL_RESIDENCY_CODE_EN	"Residency"
#define RSC_KERNEL_LIMIT_CODE_EN	"Idle Limit"

#define RSC_TOPOLOGY_TITLE_CODE_EN	" Topology "

#define RSC_MEM_CTRL_TITLE_CODE_EN	" Memory Controller "
#define RSC_MEM_CTRL_BLANK_CODE_EN		"     "
#define RSC_MEM_CTRL_SUBSECT1_0_CODE_EN 	"Contr"
#define RSC_MEM_CTRL_SUBSECT1_1_CODE_EN 	"oller"
#define RSC_MEM_CTRL_SUBSECT1_2_CODE_EN 	" #%-3u"
#define RSC_MEM_CTRL_SINGLE_CHA_0_CODE_EN	"Singl"
#define RSC_MEM_CTRL_SINGLE_CHA_1_CODE_EN	"e Cha"
#define RSC_MEM_CTRL_SINGLE_CHA_2_CODE_EN	"nnel "
#define RSC_MEM_CTRL_DUAL_CHA_0_CODE_EN 	" Dual"
#define RSC_MEM_CTRL_DUAL_CHA_1_CODE_EN 	" Chan"
#define RSC_MEM_CTRL_DUAL_CHA_2_CODE_EN 	"nel  "
#define RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_EN	"Tripl"
#define RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_EN	"e Cha"
#define RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_EN	"nnel "
#define RSC_MEM_CTRL_QUAD_CHA_0_CODE_EN 	" Quad"
#define RSC_MEM_CTRL_QUAD_CHA_1_CODE_EN 	" Chan"
#define RSC_MEM_CTRL_QUAD_CHA_2_CODE_EN 	"nel  "
#define RSC_MEM_CTRL_SIX_CHA_0_CODE_EN		"  Six"
#define RSC_MEM_CTRL_SIX_CHA_1_CODE_EN		" Chan"
#define RSC_MEM_CTRL_SIX_CHA_2_CODE_EN		"nel  "
#define RSC_MEM_CTRL_EIGHT_CHA_0_CODE_EN	"Eight"
#define RSC_MEM_CTRL_EIGHT_CHA_1_CODE_EN	" Chan"
#define RSC_MEM_CTRL_EIGHT_CHA_2_CODE_EN	"nel  "
#define RSC_MEM_CTRL_BUS_RATE_0_CODE_EN 	" Bus "
#define RSC_MEM_CTRL_BUS_RATE_1_CODE_EN 	"Rate "
#define RSC_MEM_CTRL_BUS_SPEED_0_CODE_EN	" Bus "
#define RSC_MEM_CTRL_BUS_SPEED_1_CODE_EN	"Speed"
#define RSC_MEM_CTRL_DRAM_SPEED_0_CODE_EN	"DRAM "
#define RSC_MEM_CTRL_DRAM_SPEED_1_CODE_EN	"Speed"
#define RSC_MEM_CTRL_SUBSECT2_0_CODE_EN 	" DIMM"
#define RSC_MEM_CTRL_SUBSECT2_1_CODE_EN 	" Geom"
#define RSC_MEM_CTRL_SUBSECT2_2_CODE_EN 	"etry "
#define RSC_MEM_CTRL_SUBSECT2_3_CODE_EN 	"for c"
#define RSC_MEM_CTRL_SUBSECT2_4_CODE_EN 	"hanne"
#define RSC_MEM_CTRL_SUBSECT2_5_CODE_EN 	"l #%-2u"
#define RSC_MEM_CTRL_DIMM_SLOT_CODE_EN		" Slot"
#define RSC_MEM_CTRL_DIMM_BANK_CODE_EN		" Bank"
#define RSC_MEM_CTRL_DIMM_RANK_CODE_EN		" Rank"
#define RSC_MEM_CTRL_DIMM_ROW_CODE_EN		"Rows "
#define RSC_MEM_CTRL_DIMM_COLUMN0_CODE_EN	"  Col"
#define RSC_MEM_CTRL_DIMM_COLUMN1_CODE_EN	"umns "
#define RSC_MEM_CTRL_DIMM_SIZE_0_CODE_EN	"   Me"
#define RSC_MEM_CTRL_DIMM_SIZE_1_CODE_EN	"mory "
#define RSC_MEM_CTRL_DIMM_SIZE_2_CODE_EN	"Size "
#define RSC_MEM_CTRL_DIMM_SIZE_3_CODE_EN	"(MB) "

#define RSC_TASKS_SORTBY_STATE_CODE_EN		" State    "
#define RSC_TASKS_SORTBY_RTIME_CODE_EN		" RunTime  "
#define RSC_TASKS_SORTBY_UTIME_CODE_EN		" UserTime "
#define RSC_TASKS_SORTBY_STIME_CODE_EN		" SysTime  "
#define RSC_TASKS_SORTBY_PID_CODE_EN		" PID      "
#define RSC_TASKS_SORTBY_COMM_CODE_EN		" Command  "

#define RSC_MENU_ITEM_MENU_CODE_EN		"     [F2] Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_EN		"     [F3] View          "
#define RSC_MENU_ITEM_WINDOW_CODE_EN		"    [F4] Window         "
#define RSC_MENU_ITEM_SETTINGS_CODE_EN		" Settings           [s] "
#define RSC_MENU_ITEM_SMBIOS_CODE_EN		" SMBIOS data        [B] "
#define RSC_MENU_ITEM_KERNEL_CODE_EN		" Kernel data        [k] "
#define RSC_MENU_ITEM_HOTPLUG_CODE_EN		" HotPlug CPU        [#] "
#define RSC_MENU_ITEM_TOOLS_CODE_EN		" Tools              [O] "
#define RSC_MENU_ITEM_ABOUT_CODE_EN		" About              [a] "
#define RSC_MENU_ITEM_HELP_CODE_EN		" Help               [h] "
#define RSC_MENU_ITEM_KEYS_CODE_EN		" Shortcuts         [F1] "
#define RSC_MENU_ITEM_LANG_CODE_EN		" Languages          [L] "
#define RSC_MENU_ITEM_QUIT_CODE_EN		" Quit        [Ctrl]+[x] "
#define RSC_MENU_ITEM_DASHBOARD_CODE_EN 	" Dashboard          [d] "
#define RSC_MENU_ITEM_FREQUENCY_CODE_EN 	" Frequency          [f] "
#define RSC_MENU_ITEM_INST_CYCLE_CODE_EN	" Inst cycles        [i] "
#define RSC_MENU_ITEM_CORE_CYCLE_CODE_EN	" Core cycles        [c] "
#define RSC_MENU_ITEM_IDLE_STATE_CODE_EN	" Idle C-States      [l] "
#define RSC_MENU_ITEM_PKG_CYCLE_CODE_EN 	" Package cycles     [g] "
#define RSC_MENU_ITEM_TASKS_MON_CODE_EN 	" Tasks Monitoring   [x] "
#define RSC_MENU_ITEM_SYS_INTER_CODE_EN 	" System Interrupts  [q] "
#define RSC_MENU_ITEM_SENSORS_CODE_EN		" Sensors            [C] "
#define RSC_MENU_ITEM_VOLTAGE_CODE_EN		"   Voltage          [V] "
#define RSC_MENU_ITEM_POWER_CODE_EN		"   Power            [W] "
#define RSC_MENU_ITEM_SLICE_CTR_CODE_EN 	" Slice counters     [T] "
#define RSC_MENU_ITEM_PROCESSOR_CODE_EN 	" Processor          [p] "
#define RSC_MENU_ITEM_TOPOLOGY_CODE_EN		" Topology           [m] "
#define RSC_MENU_ITEM_FEATURES_CODE_EN		" Features           [e] "
#define RSC_MENU_ITEM_ISA_EXT_CODE_EN		" ISA Extensions     [I] "
#define RSC_MENU_ITEM_TECH_CODE_EN		" Technologies       [t] "
#define RSC_MENU_ITEM_PERF_MON_CODE_EN		" Perf. Monitoring   [o] "
#define RSC_MENU_ITEM_POW_THERM_CODE_EN 	" Power & Thermal    [w] "
#define RSC_MENU_ITEM_CPUID_CODE_EN		" CPUID Hexa Dump    [u] "
#define RSC_MENU_ITEM_SYS_REGS_CODE_EN		" System Registers   [R] "
#define RSC_MENU_ITEM_MEM_CTRL_CODE_EN		" Memory Controller  [M] "

#define RSC_SETTINGS_TITLE_CODE_EN	      " Settings "
#define RSC_SETTINGS_DAEMON_CODE_EN	      " Daemon gate                    "
#define RSC_SETTINGS_INTERVAL_CODE_EN	      " Interval(ms)            <    > "
#define RSC_SETTINGS_RECORDER_CODE_EN	      " Recorder(sec)           <    > "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_EN       " Auto Clock               <   > "
#define RSC_SETTINGS_EXPERIMENTAL_CODE_EN     " Experimental             <   > "
#define RSC_SETTINGS_CPU_HOTPLUG_CODE_EN      " CPU Hot-Plug             [   ] "
#define RSC_SETTINGS_PCI_ENABLED_CODE_EN      " PCI enablement           [   ] "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_EN   " NMI registered           <   > "
#define RSC_SETTINGS_CPUIDLE_REGISTER_CODE_EN " CPU-IDLE driver          [   ] "
#define RSC_SETTINGS_CPUFREQ_REGISTER_CODE_EN " CPU-FREQ driver          [   ] "
#define RSC_SETTINGS_GOVERNOR_CPUFREQ_CODE_EN " Governor driver          [   ] "
#define RSC_SETTINGS_THERMAL_SCOPE_CODE_EN    " Thermal scope           <    > "
#define RSC_SETTINGS_VOLTAGE_SCOPE_CODE_EN    " Voltage scope           <    > "
#define RSC_SETTINGS_POWER_SCOPE_CODE_EN      " Power scope             <    > "

#define RSC_HELP_TITLE_CODE_EN		" Help "
#define RSC_HELP_KEY_ESCAPE_CODE_EN	" [Escape]         "
#define RSC_HELP_KEY_SHIFT_TAB_CODE_EN	" [Shift]+[Tab]    "
#define RSC_HELP_KEY_TAB_CODE_EN	" [Tab]            "
#define RSC_HELP_KEY_UP_CODE_EN 	"       [Up]       "
#define RSC_HELP_KEY_LEFT_RIGHT_CODE_EN " [Left]    [Right]"
#define RSC_HELP_KEY_DOWN_CODE_EN	"      [Down]      "
#define RSC_HELP_KEY_END_CODE_EN	" [End]            "
#define RSC_HELP_KEY_HOME_CODE_EN	" [Home]           "
#define RSC_HELP_KEY_ENTER_CODE_EN	" [Enter]          "
#define RSC_HELP_KEY_PAGE_UP_CODE_EN	" [Page-Up]        "
#define RSC_HELP_KEY_PAGE_DOWN_CODE_EN	" [Page-Dw]        "
#define RSC_HELP_KEY_MINUS_CODE_EN	" [Minus]          "
#define RSC_HELP_KEY_PLUS_CODE_EN	" [Plus]           "
#define RSC_HELP_MENU_CODE_EN		"             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_EN	"     Close window "
#define RSC_HELP_PREV_WINDOW_CODE_EN	"  Previous window "
#define RSC_HELP_NEXT_WINDOW_CODE_EN	"      Next window "
#define RSC_HELP_MOVE_WINDOW_CODE_EN	" [Shift] Move Win "
#define RSC_HELP_SIZE_WINDOW_CODE_EN	"  [Alt]  Size Win "
#define RSC_HELP_MOVE_SELECT_CODE_EN	"   Move selection "
#define RSC_HELP_LAST_CELL_CODE_EN	"        Last cell "
#define RSC_HELP_FIRST_CELL_CODE_EN	"       First cell "
#define RSC_HELP_TRIGGER_SELECT_CODE_EN "Trigger selection "
#define RSC_HELP_PREV_PAGE_CODE_EN	"    Previous page "
#define RSC_HELP_NEXT_PAGE_CODE_EN	"        Next page "
#define RSC_HELP_SCROLL_DOWN_CODE_EN	"  Scroll CPU down "
#define RSC_HELP_SCROLL_UP_CODE_EN	"    Scroll CPU up "

#define RSC_ADV_HELP_TITLE_CODE_EN	" Shortcuts "
#define RSC_ADV_HELP_SECT_FREQ_CODE_EN	" Frequency view:                      "
#define RSC_ADV_HELP_ITEM_AVG_CODE_EN	" %         Averages | Package C-States"
#define RSC_ADV_HELP_SECT_TASK_CODE_EN	" Task Monitoring view:                "
#define RSC_ADV_HELP_ITEM_ORDER_CODE_EN " b                Sorting tasks order "
#define RSC_ADV_HELP_ITEM_SEL_CODE_EN	" n               Select task tracking "
#define RSC_ADV_HELP_ITEM_REV_CODE_EN	" r              Reverse tasks sorting "
#define RSC_ADV_HELP_ITEM_HIDE_CODE_EN	" v          Show | Hide Kernel values "
#define RSC_ADV_HELP_SECT_ANY_CODE_EN	" Any view:                            "
#define RSC_ADV_HELP_ITEM_POWER_CODE_EN " $            Energy in Joule or Watt "
#define RSC_ADV_HELP_ITEM_TOP_CODE_EN	" .             Top frequency or Usage "
#define RSC_ADV_HELP_ITEM_UPD_CODE_EN	" *            Update CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_START_CODE_EN " {             Start CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_STOP_CODE_EN	" }              Stop CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_TOOLS_CODE_EN " F10            Stop tools processing "
#define RSC_ADV_HELP_ITEM_GO_UP_CODE_EN "  Up  PgUp                     Scroll "
#define RSC_ADV_HELP_ITEM_GO_DW_CODE_EN " Down PgDw                       CPU  "
#define RSC_ADV_HELP_TERMINAL_CODE_EN	" Terminal:                            "
#define RSC_ADV_HELP_PRT_SCR_CODE_EN	" [Ctrl]+[p]                      Copy "
#define RSC_ADV_HELP_REC_SCR_CODE_EN	" [Alt]+[p]                     Record "
#define RSC_ADV_HELP_FAHR_CELS_CODE_EN	" F              Fahrenheit or Celsius "
#define RSC_ADV_HELP_PROC_EVENT_CODE_EN " H            Manage Processor Events "
#define RSC_ADV_HELP_SECRET_CODE_EN	" Y            Show | Hide Secret Data "

#define RSC_TURBO_CLOCK_TITLE_CODE_EN	" Turbo Clock %1dC "
#define RSC_RATIO_CLOCK_TITLE_CODE_EN	" %s Clock Ratio "
#define RSC_UNCORE_CLOCK_TITLE_CODE_EN	" %s Clock Uncore "

#define RSC_SELECT_CPU_TITLE_CODE_EN	" CPU   Pkg  Core Thread "
#define RSC_SELECT_FREQ_TITLE_CODE_EN	" CPU   Pkg  Core Thread "	\
					"  Frequency   Ratio "

#define RSC_BOX_DISABLE_COND0_CODE_EN	"               Disable              "
#define RSC_BOX_DISABLE_COND1_CODE_EN	"           <   Disable  >           "
#define RSC_BOX_ENABLE_COND0_CODE_EN	"               Enable               "
#define RSC_BOX_ENABLE_COND1_CODE_EN	"           <   Enable   >           "

#define RSC_BOX_INTERVAL_TITLE_CODE_EN	" Interval "
#define RSC_BOX_AUTOCLOCK_TITLE_CODE_EN " Auto Clock "
#define RSC_BOX_MODE_TITLE_CODE_EN	" Experimental "

#define RSC_BOX_MODE_DESC_CODE_EN	"       CoreFreq Operation Mode       "
#define RSC_BOX_EIST_DESC_CODE_EN	"             SpeedStep              "
#define RSC_BOX_C1E_DESC_CODE_EN	"        Enhanced Halt State         "
#define RSC_BOX_TURBO_DESC_CODE_EN	" Turbo Boost/Core Performance Boost "
#define RSC_BOX_C1A_DESC_CODE_EN	"          C1 Auto Demotion          "
#define RSC_BOX_C3A_DESC_CODE_EN	"          C3 Auto Demotion          "
#define RSC_BOX_C1U_DESC_CODE_EN	"            C1 UnDemotion           "
#define RSC_BOX_C3U_DESC_CODE_EN	"            C3 UnDemotion           "
#define RSC_BOX_CC6_DESC_CODE_EN	"           Core C6 State            "
#define RSC_BOX_PC6_DESC_CODE_EN	"          Package C6 State          "
#define RSC_BOX_HWP_DESC_CODE_EN	"   Hardware-Controlled Performance  "
#define RSC_BOX_BLANK_DESC_CODE_EN	"                                    "

#define RSC_BOX_NOMINAL_MODE_COND0_CODE_EN "       Nominal operating mode       "
#define RSC_BOX_NOMINAL_MODE_COND1_CODE_EN "     < Nominal operating mode >     "
#define RSC_BOX_EXPER_MODE_COND0_CODE_EN   "     Experimental operating mode    "
#define RSC_BOX_EXPER_MODE_COND1_CODE_EN   "   < Experimental operating mode >  "

#define RSC_BOX_INTERRUPT_TITLE_CODE_EN " NMI Interrupts "

#define RSC_BOX_INT_REGISTER_ST0_CODE_EN   "              Register              "
#define RSC_BOX_INT_REGISTER_ST1_CODE_EN   "            < Register >            "
#define RSC_BOX_INT_UNREGISTER_ST0_CODE_EN "             Unregister             "
#define RSC_BOX_INT_UNREGISTER_ST1_CODE_EN "           < Unregister >           "

#define RSC_BOX_EVENT_TITLE_CODE_EN		" Clear Event "

#define RSC_BOX_EVENT_THERMAL_SENSOR_CODE_EN	"     Thermal Sensor     "
#define RSC_BOX_EVENT_PROCHOT_AGENT_CODE_EN	"     PROCHOT# Agent     "
#define RSC_BOX_EVENT_CRITICAL_TEMP_CODE_EN	"  Critical Temperature  "
#define RSC_BOX_EVENT_THERM_THRESHOLD_CODE_EN	"   Thermal Threshold    "
#define RSC_BOX_EVENT_POWER_LIMIT_CODE_EN	"    Power Limitation    "
#define RSC_BOX_EVENT_CURRENT_LIMIT_CODE_EN	"   Current Limitation   "
#define RSC_BOX_EVENT_CROSS_DOM_LIMIT_CODE_EN	"   Cross Domain Limit.  "

#define RSC_BOX_PKG_STATE_TITLE_CODE_EN 	" Package C-State Limit "

#define RSC_BOX_IO_MWAIT_TITLE_CODE_EN		" I/O MWAIT "
#define RSC_BOX_IO_MWAIT_DESC_CODE_EN	"        I/O MWAIT Redirection       "

#define RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_EN " I/O MWAIT Max C-State "

#define RSC_BOX_ODCM_TITLE_CODE_EN		" ODCM "
#define RSC_BOX_ODCM_DESC_CODE_EN	"          Clock Modulation          "

#define RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_EN	" Extended Duty Cycle "
#define RSC_BOX_DUTY_CYCLE_TITLE_CODE_EN	" Duty Cycle "

#define RSC_BOX_DUTY_CYCLE_RESERVED_CODE_EN	"          Reserved         "

#define RSC_BOX_POWER_POLICY_TITLE_CODE_EN	" Energy Policy "

#define RSC_BOX_POWER_POLICY_LOW_CODE_EN	"            0       LOW "
#define RSC_BOX_POWER_POLICY_HIGH_CODE_EN	"           15      HIGH "

#define RSC_BOX_HWP_POLICY_MIN_CODE_EN		"         Minimum        "
#define RSC_BOX_HWP_POLICY_MED_CODE_EN		"         Medium         "
#define RSC_BOX_HWP_POLICY_PWR_CODE_EN		"         Power          "
#define RSC_BOX_HWP_POLICY_MAX_CODE_EN		"         Maximum        "

#define RSC_BOX_TOOLS_TITLE_CODE_EN		" Tools "
#define RSC_BOX_TOOLS_STOP_CODE_EN		"            STOP           "
#define RSC_BOX_TOOLS_ATOMIC_CODE_EN		"        Atomic Burn        "
#define RSC_BOX_TOOLS_CRC32_CODE_EN		"       CRC32 Compute       "
#define RSC_BOX_TOOLS_CONIC_CODE_EN		"       Conic Compute...    "
#define RSC_BOX_TOOLS_RAND_CPU_CODE_EN		"      Turbo Random CPU     "
#define RSC_BOX_TOOLS_RR_CPU_CODE_EN		"      Turbo Round Robin    "
#define RSC_BOX_TOOLS_USR_CPU_CODE_EN		"      Turbo Select CPU...  "

#define RSC_BOX_CONIC_TITLE_CODE_EN		" Conic variations "
#define RSC_BOX_CONIC_ITEM_1_CODE_EN		"         Ellipsoid         "
#define RSC_BOX_CONIC_ITEM_2_CODE_EN		" Hyperboloid of one sheet  "
#define RSC_BOX_CONIC_ITEM_3_CODE_EN		" Hyperboloid of two sheets "
#define RSC_BOX_CONIC_ITEM_4_CODE_EN		"    Elliptical cylinder    "
#define RSC_BOX_CONIC_ITEM_5_CODE_EN		"    Hyperbolic cylinder    "
#define RSC_BOX_CONIC_ITEM_6_CODE_EN		"    Two parallel planes    "

#define RSC_BOX_LANG_TITLE_CODE_EN		" Languages "
#define RSC_BOX_LANG_ENGLISH_CODE_EN		"     English     "
#define RSC_BOX_LANG_FRENCH_CODE_EN		"     French      "

#define RSC_BOX_SCOPE_THERMAL_TITLE_CODE_EN	" Thermal scope "
#define RSC_BOX_SCOPE_VOLTAGE_TITLE_CODE_EN	" Voltage scope "
#define RSC_BOX_SCOPE_POWER_TITLE_CODE_EN	" Power scope "
#define RSC_BOX_SCOPE_NONE_CODE_EN		"       None       "
#define RSC_BOX_SCOPE_THREAD_CODE_EN		"      Thread      "
#define RSC_BOX_SCOPE_CORE_CODE_EN		"       Core       "
#define RSC_BOX_SCOPE_PACKAGE_CODE_EN		"      Package     "

#define RSC_ERROR_CMD_SYNTAX_CODE_EN					\
			"CoreFreq."					\
			"  Copyright (C) 2015-2020 CYRIL INGENIERIE\n\n"\
			"Usage:\t%s [-option <arguments>]\n"		\
			"\t-0,1,2\tMemory unit in K,M,G Byte\n" 	\
			"\t-F\tTemperature in Fahrenheit\n"		\
			"\t-J <#>\tSMBIOS string index number\n"	\
			"\t-Y\tShow Secret Data\n"			\
			"\t-t\tShow Top (default)\n"			\
			"\t-d\tShow Dashboard\n"			\
			"\t-C\tMonitor Sensors\n"			\
			"\t-V\tMonitor Voltage\n"			\
			"\t-W\tMonitor Power\n" 			\
			"\t-g\tMonitor Package\n"			\
			"\t-c\tMonitor Counters\n"			\
			"\t-i\tMonitor Instructions\n"			\
			"\t-s\tPrint System Information\n"		\
			"\t-j\tPrint System Information (json-encoded)\n"\
			"\t-M\tPrint Memory Controller\n"		\
			"\t-R\tPrint System Registers\n"		\
			"\t-m\tPrint Topology\n"			\
			"\t-u\tPrint CPUID\n"				\
			"\t-B\tPrint SMBIOS\n"				\
			"\t-k\tPrint Kernel\n"				\
			"\t-h\tPrint out this message\n"		\
			"\t-v\tPrint the version number\n"		\
			"\nExit status:\n"				\
			"\t0\tSUCCESS\t\tSuccessful execution\n"	\
			"\t1\tCMD_SYNTAX\tCommand syntax error\n"	\
			"\t2\tSHM_FILE\tShared memory file error\n"	\
			"\t3\tSHM_MMAP\tShared memory mapping error\n"	\
			"\t4\tPERM_ERR\tExecution not permitted\n"	\
			"\t5\tMEM_ERR\t\tMemory operation error\n"	\
			"\t6\tEXEC_ERR\tGeneral execution error\n"	\
			"\t15\tSYS_CALL\tSystem call error\n"		\
			"\nReport bugs to labs[at]cyring.fr\n"

#define RSC_ERROR_SHARED_MEM_CODE_EN					\
		"Daemon connection error code %d\n%s: '%s' @ line %d\n"

#define RSC_ERROR_SYS_CALL_CODE_EN	"System error code %d\n%s @ line %d\n"

#define RSC_BOX_IDLE_LIMIT_TITLE_CODE_EN " CPU-Idle Limit "

#define RSC_BOX_RECORDER_TITLE_CODE_EN	" Duration "

#define RSC_SMBIOS_TITLE_CODE_EN	" SMBIOS "

#define RSC_MECH_IBRS_CODE_EN	"Indirect Branch Restricted Speculation"
#define RSC_MECH_IBPB_CODE_EN	"Indirect Branch Prediction Barrier"
#define RSC_MECH_STIBP_CODE_EN	"Single Thread Indirect Branch Predictor"
#define RSC_MECH_SSBD_CODE_EN	"Speculative Store Bypass Disable"
#define RSC_MECH_L1D_FLUSH_CODE_EN "Writeback & invalidate the L1 data cache"
#define RSC_MECH_MD_CLEAR_CODE_EN  "Architectural - Buffer Overwriting"
#define RSC_MECH_RDCL_NO_CODE_EN   "Architectural - Rogue Data Cache Load"
#define RSC_MECH_IBRS_ALL_CODE_EN  "Architectural - Enhanced IBRS"
#define RSC_MECH_RSBA_CODE_EN	"Architectural - Return Stack Buffer Alternate"
#define RSC_MECH_L1DFL_VMENTRY_NO_CODE_EN \
				"Hypervisor - No flush L1D on VM entry"
#define RSC_MECH_SSB_NO_CODE_EN "Architectural - Speculative Store Bypass"
#define RSC_MECH_MDS_NO_CODE_EN \
			"Architectural - Microarchitectural Data Sampling"
#define RSC_MECH_PSCHANGE_MC_NO_CODE_EN "Architectural - Page Size Change MCE"
#define RSC_MECH_TAA_NO_CODE_EN "Architectural - TSX Asynchronous Abort"

#define RSC_CREATE_SELECT_FREQ_TGT_CODE_EN	"  TGT       Processor     "
#define RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_EN	"  HWP-TGT   Processor     "
#define RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_EN	"  HWP-MAX   Processor     "
#define RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_EN	"  HWP-MIN   Processor     "

