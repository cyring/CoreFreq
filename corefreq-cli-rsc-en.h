/*
 * CoreFreq
 * Copyright (C) 2015-2019 CYRIL INGENIERIE
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

#define RSC_LAYOUT_RULLER_FREQUENCY_AVG_CODE_EN 			\
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

#define RSC_LAYOUT_RULLER_PACKAGE_CODE_EN				\
	"------------ Cycles ---- State -------------------- TSC Rati"	\
	"o ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULLER_TASKS_CODE_EN 				\
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

#define RSC_LAYOUT_RULLER_VOLTAGE_CODE_EN				\
	"--- Freq(MHz) - VID - Vcore ------------------ Energy(J) ---"	\
	"-- Power(W) ------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_DOMAIN_PACKAGE_CODE_EN				\
{									\
	'P','a','c','k','a','g','e'					\
}

#define RSC_LAYOUT_DOMAIN_CORES_CODE_EN 				\
{									\
	'C','o','r','e','s',' ',' '					\
}

#define RSC_LAYOUT_DOMAIN_UNCORE_CODE_EN				\
{									\
	'U','n','c','o','r','e',' '					\
}

#define RSC_LAYOUT_DOMAIN_MEMORY_CODE_EN				\
{									\
	'M','e','m','o','r','y',' '					\
}

#define RSC_LAYOUT_RULLER_SLICE_CODE_EN 				\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 ----------- Error -----------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_EN				\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
	'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','B',']' 	\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_EN	(ASCII*) "[   ENABLE ]"
#define RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_EN	(ASCII*) "[  DISABLE ]"

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

#define RSC_PROCESSOR_TITLE_CODE_EN	(ASCII*) " Processor "
#define RSC_PROCESSOR_CODE_EN		(ASCII*) "Processor"
#define RSC_ARCHITECTURE_CODE_EN	(ASCII*) "Architecture"
#define RSC_VENDOR_ID_CODE_EN		(ASCII*) "Vendor ID"
#define RSC_MICROCODE_CODE_EN		(ASCII*) "Microcode"
#define RSC_SIGNATURE_CODE_EN		(ASCII*) "Signature"
#define RSC_STEPPING_CODE_EN		(ASCII*) "Stepping"
#define RSC_ONLINE_CPU_CODE_EN		(ASCII*) "Online CPU"
#define RSC_BASE_CLOCK_CODE_EN		(ASCII*) "Base Clock"
#define RSC_FREQUENCY_CODE_EN		(ASCII*) "Frequency"
#define RSC_RATIO_CODE_EN		(ASCII*) "Ratio"
#define RSC_FACTORY_CODE_EN		(ASCII*) "Factory"
#define RSC_PERFORMANCE_CODE_EN 	(ASCII*) "Performance"
#define RSC_TARGET_CODE_EN		(ASCII*) "Target"
#define RSC_LEVEL_CODE_EN		(ASCII*) "Level"
#define RSC_PROGRAMMABLE_CODE_EN	(ASCII*) "Programmable"
#define RSC_CONFIGURATION_CODE_EN	(ASCII*) "Configuration"
#define RSC_TURBO_ACTIVATION_CODE_EN	(ASCII*) "Turbo Activation"
#define RSC_NOMINAL_CODE_EN		(ASCII*) "Nominal"
#define RSC_UNLOCK_CODE_EN		(ASCII*) "UNLOCK"
#define RSC_LOCK_CODE_EN		(ASCII*) "  LOCK"
#define RSC_ENABLE_CODE_EN		(ASCII*) " Enable"
#define RSC_DISABLE_CODE_EN		(ASCII*) "Disable"
#define RSC_CAPABILITIES_CODE_EN	(ASCII*) "Capabilities"
#define RSC_LOWEST_CODE_EN		(ASCII*) "Lowest"
#define RSC_EFFICIENT_CODE_EN		(ASCII*) "Efficient"
#define RSC_GUARANTEED_CODE_EN		(ASCII*) "Guaranteed"
#define RSC_HIGHEST_CODE_EN		(ASCII*) "Highest"

#define RSC_CPUID_TITLE_EN		\
(ASCII*) " function           EAX          EBX          ECX          EDX "

#define RSC_LARGEST_STD_FUNC_CODE_EN	(ASCII*) "Largest Standard Function"
#define RSC_LARGEST_EXT_FUNC_CODE_EN	(ASCII*) "Largest Extended Function"

#define RSC_SYS_REGS_TITLE_CODE_EN	(ASCII*) " System Registers "

#define RSC_ISA_TITLE_CODE_EN		(ASCII*) " Instruction Set Extensions "

#define RSC_FEATURES_TITLE_CODE_EN	(ASCII*) " Features "
#define RSC_MISSING_CODE_EN		(ASCII*) "Missing"
#define RSC_PRESENT_CODE_EN		(ASCII*) "Present"
#define RSC_VARIANT_CODE_EN		(ASCII*) "Variant"
#define RSC_INVARIANT_CODE_EN		(ASCII*) "Invariant"
#define RSC_FEATURES_1GB_PAGES_CODE_EN	(ASCII*) "1 GB Pages Support"
#define RSC_FEATURES_100MHZ_CODE_EN	(ASCII*) "100 MHz multiplier Control"
#define RSC_FEATURES_ACPI_CODE_EN					\
			(ASCII*) "Advanced Configuration & Power Interface"
#define RSC_FEATURES_APIC_CODE_EN					\
			(ASCII*) "Advanced Programmable Interrupt Controller"
#define RSC_FEATURES_CORE_MP_CODE_EN	(ASCII*) "Core Multi-Processing"
#define RSC_FEATURES_CNXT_ID_CODE_EN	(ASCII*) "L1 Data Cache Context ID"
#define RSC_FEATURES_DCA_CODE_EN	(ASCII*) "Direct Cache Access"
#define RSC_FEATURES_DE_CODE_EN 	(ASCII*) "Debugging Extension"
#define RSC_FEATURES_DS_PEBS_CODE_EN					\
			(ASCII*) "Debug Store & Precise Event Based Sampling"
#define RSC_FEATURES_DS_CPL_CODE_EN	(ASCII*) "CPL Qualified Debug Store"
#define RSC_FEATURES_DTES_64_CODE_EN	(ASCII*) "64-Bit Debug Store"
#define RSC_FEATURES_FAST_STR_CODE_EN	(ASCII*) "Fast-String Operation"
#define RSC_FEATURES_FMA_CODE_EN	(ASCII*) "Fused Multiply Add"
#define RSC_FEATURES_HLE_CODE_EN	(ASCII*) "Hardware Lock Elision"
#define RSC_FEATURES_LM_CODE_EN 	(ASCII*) "Long Mode 64 bits"
#define RSC_FEATURES_LWP_CODE_EN	(ASCII*) "LightWeight Profiling"
#define RSC_FEATURES_MCA_CODE_EN	(ASCII*) "Machine-Check Architecture"
#define RSC_FEATURES_MSR_CODE_EN	(ASCII*) "Model Specific Registers"
#define RSC_FEATURES_MTRR_CODE_EN	(ASCII*) "Memory Type Range Registers"
#define RSC_FEATURES_NX_CODE_EN 	(ASCII*) "No-Execute Page Protection"
#define RSC_FEATURES_OSXSAVE_CODE_EN					\
				(ASCII*) "OS-Enabled Ext. State Management"
#define RSC_FEATURES_PAE_CODE_EN	(ASCII*) "Physical Address Extension"
#define RSC_FEATURES_PAT_CODE_EN	(ASCII*) "Page Attribute Table"
#define RSC_FEATURES_PBE_CODE_EN	(ASCII*) "Pending Break Enable"
#define RSC_FEATURES_PCID_CODE_EN	(ASCII*) "Process Context Identifiers"
#define RSC_FEATURES_PDCM_CODE_EN	(ASCII*) "Perfmon and Debug Capability"
#define RSC_FEATURES_PGE_CODE_EN	(ASCII*) "Page Global Enable"
#define RSC_FEATURES_PSE_CODE_EN	(ASCII*) "Page Size Extension"
#define RSC_FEATURES_PSE36_CODE_EN	(ASCII*) "36-bit Page Size Extension"
#define RSC_FEATURES_PSN_CODE_EN	(ASCII*) "Processor Serial Number"
#define RSC_FEATURES_RTM_CODE_EN					\
				(ASCII*) "Restricted Transactional Memory"
#define RSC_FEATURES_SMX_CODE_EN	(ASCII*) "Safer Mode Extensions"
#define RSC_FEATURES_SELF_SNOOP_CODE_EN (ASCII*) "Self-Snoop"
#define RSC_FEATURES_TSC_CODE_EN	(ASCII*) "Time Stamp Counter"
#define RSC_FEATURES_TSC_DEADLN_CODE_EN (ASCII*) "Time Stamp Counter Deadline"
#define RSC_FEATURES_VME_CODE_EN	(ASCII*) "Virtual Mode Extension"
#define RSC_FEATURES_VMX_CODE_EN	(ASCII*) "Virtual Machine Extensions"
#define RSC_FEATURES_X2APIC_CODE_EN	(ASCII*) "Extended xAPIC Support"
#define RSC_FEATURES_XD_BIT_CODE_EN	(ASCII*) "Execution Disable Bit Support"
#define RSC_FEATURES_XSAVE_CODE_EN	(ASCII*) "XSAVE/XSTOR States"
#define RSC_FEATURES_XTPR_CODE_EN	(ASCII*) "xTPR Update Control"

#define RSC_TECHNOLOGIES_TITLE_CODE_EN	(ASCII*) " Technologies "
#define RSC_TECHNOLOGIES_SMM_CODE_EN	(ASCII*) "System Management Mode"
#define RSC_TECHNOLOGIES_HTT_CODE_EN	(ASCII*) "Hyper-Threading"
#define RSC_TECHNOLOGIES_EIST_CODE_EN	(ASCII*) "SpeedStep"
#define RSC_TECHNOLOGIES_IDA_CODE_EN	(ASCII*) "Dynamic Acceleration"
#define RSC_TECHNOLOGIES_TURBO_CODE_EN	(ASCII*) "Turbo Boost"
#define RSC_TECHNOLOGIES_VM_CODE_EN	(ASCII*) "Virtualization"
#define RSC_TECHNOLOGIES_IOMMU_CODE_EN	(ASCII*) "I/O MMU"
#define RSC_TECHNOLOGIES_SMT_CODE_EN	(ASCII*) "Simultaneous Multithreading"
#define RSC_TECHNOLOGIES_CNQ_CODE_EN	(ASCII*) "PowerNow!"
#define RSC_TECHNOLOGIES_CPB_CODE_EN	(ASCII*) "Core Performance Boost"
#define RSC_TECHNOLOGIES_HYPERV_CODE_EN (ASCII*) "Hypervisor"

#define RSC_PERF_MON_TITLE_CODE_EN	(ASCII*) " Performance Monitoring "
#define RSC_VERSION_CODE_EN		(ASCII*) "Version"
#define RSC_COUNTERS_CODE_EN		(ASCII*) "Counters"
#define RSC_GENERAL_CTRS_CODE_EN	(ASCII*) "General"
#define RSC_FIXED_CTRS_CODE_EN		(ASCII*) "Fixed"
#define RSC_PERF_MON_C1E_CODE_EN	(ASCII*) "Enhanced Halt State"
#define RSC_PERF_MON_C1A_CODE_EN	(ASCII*) "C1 Auto Demotion"
#define RSC_PERF_MON_C3A_CODE_EN	(ASCII*) "C3 Auto Demotion"
#define RSC_PERF_MON_C1U_CODE_EN	(ASCII*) "C1 UnDemotion"
#define RSC_PERF_MON_C3U_CODE_EN	(ASCII*) "C3 UnDemotion"
#define RSC_PERF_MON_CC6_CODE_EN	(ASCII*) "Core C6 State"
#define RSC_PERF_MON_PC6_CODE_EN	(ASCII*) "Package C6 State"
#define RSC_PERF_MON_FID_CODE_EN	(ASCII*) "Frequency ID control"
#define RSC_PERF_MON_VID_CODE_EN	(ASCII*) "Voltage ID control"
#define RSC_PERF_MON_HWCF_CODE_EN					\
			(ASCII*) "P-State Hardware Coordination Feedback"
#define RSC_PERF_MON_HWP_CODE_EN					\
			(ASCII*) "Hardware-Controlled Performance States"
#define RSC_PERF_MON_HDC_CODE_EN	(ASCII*) "Hardware Duty Cycling"
#define RSC_PERF_MON_PKG_CSTATE_CODE_EN (ASCII*) "Package C-State"
#define RSC_PERF_MON_CFG_CTRL_CODE_EN	(ASCII*) "Configuration Control"
#define RSC_PERF_MON_LOW_CSTATE_CODE_EN (ASCII*) "Lowest C-State"
#define RSC_PERF_MON_IOMWAIT_CODE_EN	(ASCII*) "I/O MWAIT Redirection"
#define RSC_PERF_MON_MAX_CSTATE_CODE_EN (ASCII*) "Max C-State Inclusion"

#define RSC_PERF_MON_MONITOR_MWAIT_CODE_EN				\
					(ASCII*) "MONITOR/MWAIT"
#define RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_EN				\
					(ASCII*) "State index"
#define RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_EN				\
					(ASCII*) "Sub C-State"

#define RSC_PERF_MON_CORE_CYCLE_CODE_EN (ASCII*) "Core Cycles"
#define RSC_PERF_MON_INST_RET_CODE_EN	(ASCII*) "Instructions Retired"
#define RSC_PERF_MON_REF_CYCLE_CODE_EN	(ASCII*) "Reference Cycles"
#define RSC_PERF_MON_REF_LLC_CODE_EN	(ASCII*) "Last Level Cache References"
#define RSC_PERF_MON_MISS_LLC_CODE_EN	(ASCII*) "Last Level Cache Misses"
#define RSC_PERF_MON_BRANCH_RET_CODE_EN (ASCII*) "Branch Instructions Retired"
#define RSC_PERF_MON_BRANCH_MIS_CODE_EN (ASCII*) "Branch Mispredicts Retired"

#define RSC_POWER_THERMAL_TITLE_CODE_EN (ASCII*) " Power & Thermal "
#define RSC_POWER_THERMAL_ODCM_CODE_EN	(ASCII*) "Clock Modulation"
#define RSC_POWER_THERMAL_DUTY_CODE_EN	(ASCII*) "DutyCycle"
#define RSC_POWER_THERMAL_MGMT_CODE_EN	(ASCII*) "Power Management"
#define RSC_POWER_THERMAL_BIAS_CODE_EN	(ASCII*) "Energy Policy"
#define RSC_POWER_THERMAL_TJMAX_CODE_EN (ASCII*) "Junction Temperature"
#define RSC_POWER_THERMAL_DTS_CODE_EN	(ASCII*) "Digital Thermal Sensor"
#define RSC_POWER_THERMAL_PLN_CODE_EN	(ASCII*) "Power Limit Notification"
#define RSC_POWER_THERMAL_PTM_CODE_EN	(ASCII*) "Package Thermal Management"
#define RSC_POWER_THERMAL_TM1_CODE_EN	(ASCII*) "Thermal Monitor 1"
#define RSC_POWER_THERMAL_TM2_CODE_EN	(ASCII*) "Thermal Monitor 2"
#define RSC_POWER_THERMAL_UNITS_CODE_EN (ASCII*) "Units"
#define RSC_POWER_THERMAL_POWER_CODE_EN (ASCII*) "Power"
#define RSC_POWER_THERMAL_ENERGY_CODE_EN (ASCII*)"Energy"
#define RSC_POWER_THERMAL_WINDOW_CODE_EN (ASCII*)"Window"
#define RSC_POWER_THERMAL_WATT_CODE_EN	(ASCII*) "watt"
#define RSC_POWER_THERMAL_JOULE_CODE_EN (ASCII*) "joule"
#define RSC_POWER_THERMAL_SECOND_CODE_EN (ASCII*)"second"

#define RSC_KERNEL_TITLE_CODE_EN	(ASCII*) " Kernel "
#define RSC_KERNEL_TOTAL_RAM_CODE_EN	(ASCII*) "Total RAM"
#define RSC_KERNEL_SHARED_RAM_CODE_EN	(ASCII*) "Shared RAM"
#define RSC_KERNEL_FREE_RAM_CODE_EN	(ASCII*) "Free RAM"
#define RSC_KERNEL_BUFFER_RAM_CODE_EN	(ASCII*) "Buffer RAM"
#define RSC_KERNEL_TOTAL_HIGH_CODE_EN	(ASCII*) "Total High"
#define RSC_KERNEL_FREE_HIGH_CODE_EN	(ASCII*) "Free High"
#define RSC_KERNEL_GOVERNOR_CODE_EN	(ASCII*) "Governor"
#define RSC_KERNEL_FREQ_DRIVER_CODE_EN	(ASCII*) "CPU-Freq driver"
#define RSC_KERNEL_IDLE_DRIVER_CODE_EN	(ASCII*) "CPU-Idle driver"
#define RSC_KERNEL_RELEASE_CODE_EN	(ASCII*) "Release"
#define RSC_KERNEL_VERSION_CODE_EN	(ASCII*) "Version"
#define RSC_KERNEL_MACHINE_CODE_EN	(ASCII*) "Machine"
#define RSC_KERNEL_MEMORY_CODE_EN	(ASCII*) "Memory"
#define RSC_KERNEL_STATE_CODE_EN	(ASCII*) "State"
#define RSC_KERNEL_POWER_CODE_EN	(ASCII*) "Power"
#define RSC_KERNEL_LATENCY_CODE_EN	(ASCII*) "Latency"
#define RSC_KERNEL_RESIDENCY_CODE_EN	(ASCII*) "Residency"
#define RSC_KERNEL_LIMIT_CODE_EN	(ASCII*) "Idle Limit"

#define RSC_TOPOLOGY_TITLE_CODE_EN	(ASCII*) " Topology "

#define RSC_MEM_CTRL_TITLE_CODE_EN	(ASCII*) " Memory Controller "
#define RSC_MEM_CTRL_BLANK_CODE_EN	  (ASCII*)"     "
#define RSC_MEM_CTRL_SUBSECT1_0_CODE_EN   (ASCII*)"Contr"
#define RSC_MEM_CTRL_SUBSECT1_1_CODE_EN   (ASCII*)"oller"
#define RSC_MEM_CTRL_SUBSECT1_2_CODE_EN   (ASCII*)" #%-3u"
#define RSC_MEM_CTRL_SINGLE_CHA_0_CODE_EN (ASCII*)"Singl"
#define RSC_MEM_CTRL_SINGLE_CHA_1_CODE_EN (ASCII*)"e Cha"
#define RSC_MEM_CTRL_SINGLE_CHA_2_CODE_EN (ASCII*)"nnel "
#define RSC_MEM_CTRL_DUAL_CHA_0_CODE_EN   (ASCII*)" Dual"
#define RSC_MEM_CTRL_DUAL_CHA_1_CODE_EN   (ASCII*)" Chan"
#define RSC_MEM_CTRL_DUAL_CHA_2_CODE_EN   (ASCII*)"nel  "
#define RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_EN (ASCII*)"Tripl"
#define RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_EN (ASCII*)"e Cha"
#define RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_EN (ASCII*)"nnel "
#define RSC_MEM_CTRL_QUAD_CHA_0_CODE_EN   (ASCII*)" Quad"
#define RSC_MEM_CTRL_QUAD_CHA_1_CODE_EN   (ASCII*)" Chan"
#define RSC_MEM_CTRL_QUAD_CHA_2_CODE_EN   (ASCII*)"nel  "
#define RSC_MEM_CTRL_SIX_CHA_0_CODE_EN	  (ASCII*)"  Six"
#define RSC_MEM_CTRL_SIX_CHA_1_CODE_EN	  (ASCII*)" Chan"
#define RSC_MEM_CTRL_SIX_CHA_2_CODE_EN	  (ASCII*)"nel  "
#define RSC_MEM_CTRL_EIGHT_CHA_0_CODE_EN  (ASCII*)"Eight"
#define RSC_MEM_CTRL_EIGHT_CHA_1_CODE_EN  (ASCII*)" Chan"
#define RSC_MEM_CTRL_EIGHT_CHA_2_CODE_EN  (ASCII*)"nel  "
#define RSC_MEM_CTRL_BUS_RATE_0_CODE_EN   (ASCII*)" Bus "
#define RSC_MEM_CTRL_BUS_RATE_1_CODE_EN   (ASCII*)"Rate "
#define RSC_MEM_CTRL_BUS_SPEED_0_CODE_EN  (ASCII*)" Bus "
#define RSC_MEM_CTRL_BUS_SPEED_1_CODE_EN  (ASCII*)"Speed"
#define RSC_MEM_CTRL_DRAM_SPEED_0_CODE_EN (ASCII*)"DRAM "
#define RSC_MEM_CTRL_DRAM_SPEED_1_CODE_EN (ASCII*)"Speed"
#define RSC_MEM_CTRL_SUBSECT2_0_CODE_EN   (ASCII*)" DIMM"
#define RSC_MEM_CTRL_SUBSECT2_1_CODE_EN   (ASCII*)" Geom"
#define RSC_MEM_CTRL_SUBSECT2_2_CODE_EN   (ASCII*)"etry "
#define RSC_MEM_CTRL_SUBSECT2_3_CODE_EN   (ASCII*)"for c"
#define RSC_MEM_CTRL_SUBSECT2_4_CODE_EN   (ASCII*)"hanne"
#define RSC_MEM_CTRL_SUBSECT2_5_CODE_EN   (ASCII*)"l #%-2u"
#define RSC_MEM_CTRL_DIMM_SLOT_CODE_EN	  (ASCII*)" Slot"
#define RSC_MEM_CTRL_DIMM_BANK_CODE_EN	  (ASCII*)" Bank"
#define RSC_MEM_CTRL_DIMM_RANK_CODE_EN	  (ASCII*)" Rank"
#define RSC_MEM_CTRL_DIMM_ROW_CODE_EN	  (ASCII*)"Rows "
#define RSC_MEM_CTRL_DIMM_COLUMN0_CODE_EN (ASCII*)"  Col"
#define RSC_MEM_CTRL_DIMM_COLUMN1_CODE_EN (ASCII*)"umns "
#define RSC_MEM_CTRL_DIMM_SIZE_0_CODE_EN  (ASCII*)"   Me"
#define RSC_MEM_CTRL_DIMM_SIZE_1_CODE_EN  (ASCII*)"mory "
#define RSC_MEM_CTRL_DIMM_SIZE_2_CODE_EN  (ASCII*)"Size "
#define RSC_MEM_CTRL_DIMM_SIZE_3_CODE_EN  (ASCII*)"(MB) "

#define RSC_TASKS_SORTBY_STATE_CODE_EN	(ASCII*) " State    "
#define RSC_TASKS_SORTBY_RTIME_CODE_EN	(ASCII*) " RunTime  "
#define RSC_TASKS_SORTBY_UTIME_CODE_EN	(ASCII*) " UserTime "
#define RSC_TASKS_SORTBY_STIME_CODE_EN	(ASCII*) " SysTime  "
#define RSC_TASKS_SORTBY_PID_CODE_EN	(ASCII*) " PID      "
#define RSC_TASKS_SORTBY_COMM_CODE_EN	(ASCII*) " Command  "

#define RSC_MENU_ITEM_MENU_CODE_EN	(ASCII*) "          Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_EN	(ASCII*) "          View          "
#define RSC_MENU_ITEM_WINDOW_CODE_EN	(ASCII*) "         Window         "
#define RSC_MENU_ITEM_SETTINGS_CODE_EN	(ASCII*) " Settings           [s] "
#define RSC_MENU_ITEM_SMBIOS_CODE_EN	(ASCII*) " SMBIOS data        [B] "
#define RSC_MENU_ITEM_KERNEL_CODE_EN	(ASCII*) " Kernel data        [k] "
#define RSC_MENU_ITEM_HOTPLUG_CODE_EN	(ASCII*) " HotPlug CPU        [#] "
#define RSC_MENU_ITEM_TOOLS_CODE_EN	(ASCII*) " Tools             [F3] "
#define RSC_MENU_ITEM_ABOUT_CODE_EN	(ASCII*) " About              [a] "
#define RSC_MENU_ITEM_HELP_CODE_EN	(ASCII*) " Help               [h] "
#define RSC_MENU_ITEM_KEYS_CODE_EN	(ASCII*) " Shortcuts         [F1] "
#define RSC_MENU_ITEM_LANG_CODE_EN	(ASCII*) " Languages          [L] "
#define RSC_MENU_ITEM_QUIT_CODE_EN	(ASCII*) " Quit              [F4] "
#define RSC_MENU_ITEM_DASHBOARD_CODE_EN (ASCII*) " Dashboard          [d] "
#define RSC_MENU_ITEM_FREQUENCY_CODE_EN (ASCII*) " Frequency          [f] "
#define RSC_MENU_ITEM_INST_CYCLE_CODE_EN (ASCII*)" Inst cycles        [i] "
#define RSC_MENU_ITEM_CORE_CYCLE_CODE_EN (ASCII*)" Core cycles        [c] "
#define RSC_MENU_ITEM_IDLE_STATE_CODE_EN (ASCII*)" Idle C-States      [l] "
#define RSC_MENU_ITEM_PKG_CYCLE_CODE_EN (ASCII*) " Package cycles     [g] "
#define RSC_MENU_ITEM_TASKS_MON_CODE_EN (ASCII*) " Tasks Monitoring   [x] "
#define RSC_MENU_ITEM_SYS_INTER_CODE_EN (ASCII*) " System Interrupts  [q] "
#define RSC_MENU_ITEM_POW_VOLT_CODE_EN	(ASCII*) " Power & Voltage    [V] "
#define RSC_MENU_ITEM_SLICE_CTR_CODE_EN (ASCII*) " Slice counters     [T] "
#define RSC_MENU_ITEM_PROCESSOR_CODE_EN (ASCII*) " Processor          [p] "
#define RSC_MENU_ITEM_TOPOLOGY_CODE_EN	(ASCII*) " Topology           [m] "
#define RSC_MENU_ITEM_FEATURES_CODE_EN	(ASCII*) " Features           [e] "
#define RSC_MENU_ITEM_ISA_EXT_CODE_EN	(ASCII*) " ISA Extensions     [I] "
#define RSC_MENU_ITEM_TECH_CODE_EN	(ASCII*) " Technologies       [t] "
#define RSC_MENU_ITEM_PERF_MON_CODE_EN	(ASCII*) " Perf. Monitoring   [o] "
#define RSC_MENU_ITEM_POW_THERM_CODE_EN (ASCII*) " Power & Thermal    [w] "
#define RSC_MENU_ITEM_CPUID_CODE_EN	(ASCII*) " CPUID Hexa Dump    [u] "
#define RSC_MENU_ITEM_SYS_REGS_CODE_EN	(ASCII*) " System Registers   [R] "
#define RSC_MENU_ITEM_MEM_CTRL_CODE_EN	(ASCII*) " Memory Controller  [M] "

#define RSC_SETTINGS_TITLE_CODE_EN	(ASCII*) " Settings "
#define RSC_SETTINGS_DAEMON_CODE_EN					\
				(ASCII*) " Daemon gate                    "
#define RSC_SETTINGS_INTERVAL_CODE_EN					\
				(ASCII*) " Interval(ms)            <    > "
#define RSC_SETTINGS_RECORDER_CODE_EN					\
				(ASCII*) " Recorder(sec)           <    > "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_EN 				\
				(ASCII*) " Auto Clock               <   > "
#define RSC_SETTINGS_EXPERIMENTAL_CODE_EN				\
				(ASCII*) " Experimental             <   > "
#define RSC_SETTINGS_CPU_HOTPLUG_CODE_EN				\
				(ASCII*) " CPU Hot-Plug             [   ] "
#define RSC_SETTINGS_PCI_ENABLED_CODE_EN				\
				(ASCII*) " PCI enablement           [   ] "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_EN				\
				(ASCII*) " NMI registered           <   > "
#define RSC_SETTINGS_CPUIDLE_REGISTERED_CODE_EN 			\
				(ASCII*) " CPU-IDLE driver          [   ] "
#define RSC_SETTINGS_CPUFREQ_REGISTERED_CODE_EN 			\
				(ASCII*) " CPU-FREQ driver          [   ] "

#define RSC_HELP_TITLE_CODE_EN		(ASCII*) " Help "
#define RSC_HELP_KEY_ESCAPE_CODE_EN	(ASCII*) " [Escape]         "
#define RSC_HELP_KEY_SHIFT_TAB_CODE_EN	(ASCII*) " [Shift]+[Tab]    "
#define RSC_HELP_KEY_TAB_CODE_EN	(ASCII*) " [Tab]            "
#define RSC_HELP_KEY_UP_CODE_EN 	(ASCII*) "       [Up]       "
#define RSC_HELP_KEY_LEFT_RIGHT_CODE_EN (ASCII*) " [Left]    [Right]"
#define RSC_HELP_KEY_DOWN_CODE_EN	(ASCII*) "      [Down]      "
#define RSC_HELP_KEY_END_CODE_EN	(ASCII*) " [End]            "
#define RSC_HELP_KEY_HOME_CODE_EN	(ASCII*) " [Home]           "
#define RSC_HELP_KEY_ENTER_CODE_EN	(ASCII*) " [Enter]          "
#define RSC_HELP_KEY_PAGE_UP_CODE_EN	(ASCII*) " [Page-Up]        "
#define RSC_HELP_KEY_PAGE_DOWN_CODE_EN	(ASCII*) " [Page-Dw]        "
#define RSC_HELP_KEY_MINUS_CODE_EN	(ASCII*) " [Minus]          "
#define RSC_HELP_KEY_PLUS_CODE_EN	(ASCII*) " [Plus]           "
#define RSC_HELP_MENU_CODE_EN		(ASCII*) "             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_EN	(ASCII*) "     Close window "
#define RSC_HELP_PREV_WINDOW_CODE_EN	(ASCII*) "  Previous window "
#define RSC_HELP_NEXT_WINDOW_CODE_EN	(ASCII*) "      Next window "
#define RSC_HELP_MOVE_WINDOW_CODE_EN	(ASCII*) "      Move window "
#define RSC_HELP_MOVE_SELECT_CODE_EN	(ASCII*) "   Move selection "
#define RSC_HELP_LAST_CELL_CODE_EN	(ASCII*) "        Last cell "
#define RSC_HELP_FIRST_CELL_CODE_EN	(ASCII*) "       First cell "
#define RSC_HELP_TRIGGER_SELECT_CODE_EN (ASCII*) "Trigger selection "
#define RSC_HELP_PREV_PAGE_CODE_EN	(ASCII*) "    Previous page "
#define RSC_HELP_NEXT_PAGE_CODE_EN	(ASCII*) "        Next page "
#define RSC_HELP_SCROLL_DOWN_CODE_EN	(ASCII*) "  Scroll CPU down "
#define RSC_HELP_SCROLL_UP_CODE_EN	(ASCII*) "    Scroll CPU up "

#define RSC_ADV_HELP_TITLE_CODE_EN	(ASCII*) " Shortcuts "
#define RSC_ADV_HELP_ITEM_1_CODE_EN					\
			(ASCII*) " Frequency view:                      "
#define RSC_ADV_HELP_ITEM_2_CODE_EN					\
			(ASCII*) " %         Averages | Package C-States"
#define RSC_ADV_HELP_ITEM_3_CODE_EN					\
			(ASCII*) " Task Monitoring view:                "
#define RSC_ADV_HELP_ITEM_4_CODE_EN					\
			(ASCII*) " b                Sorting tasks order "
#define RSC_ADV_HELP_ITEM_5_CODE_EN					\
			(ASCII*) " n               Select task tracking "
#define RSC_ADV_HELP_ITEM_6_CODE_EN					\
			(ASCII*) " r              Reverse tasks sorting "
#define RSC_ADV_HELP_ITEM_7_CODE_EN					\
			(ASCII*) " v          Show | Hide Kernel values "
#define RSC_ADV_HELP_ITEM_8_CODE_EN					\
			(ASCII*) " Any view:                            "
#define RSC_ADV_HELP_ITEM_9_CODE_EN					\
			(ASCII*) " .             Top frequency or Usage "
#define RSC_ADV_HELP_ITEM_10_CODE_EN					\
			(ASCII*) " {             Start CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_11_CODE_EN					\
			(ASCII*) " }              Stop CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_12_CODE_EN					\
			(ASCII*) " F10            Stop tools processing "
#define RSC_ADV_HELP_ITEM_13_CODE_EN					\
			(ASCII*) "  Up  PgUp                     Scroll "
#define RSC_ADV_HELP_ITEM_14_CODE_EN					\
			(ASCII*) " Down PgDw                       CPU  "
#define RSC_ADV_HELP_ITEM_TERMINAL_CODE_EN				\
			(ASCII*) " Terminal:                            "
#define RSC_ADV_HELP_ITEM_PRT_SCR_CODE_EN				\
			(ASCII*) " [Ctrl]+[p]                      Copy "
#define RSC_ADV_HELP_ITEM_REC_SCR_CODE_EN				\
			(ASCII*) " [Alt]+[p]                     Record "
#define RSC_ADV_HELP_ITEM_FAHR_CELS_CODE_EN				\
			(ASCII*) " F              Fahrenheit or Celsius "

#define RSC_TURBO_CLOCK_TITLE_CODE_EN	(ASCII*) " Turbo Clock %1dC "
#define RSC_RATIO_CLOCK_TITLE_CODE_EN	(ASCII*) " %s Clock Ratio "
#define RSC_UNCORE_CLOCK_TITLE_CODE_EN	(ASCII*) " %s Clock Uncore "

#define RSC_SELECT_CPU_TITLE_CODE_EN	(ASCII*) " CPU   Pkg  Core Thread "

#define RSC_BOX_DISABLE_COND0_CODE_EN					\
			(ASCII*) "               Disable              "
#define RSC_BOX_DISABLE_COND1_CODE_EN					\
			(ASCII*) "           <   Disable  >           "
#define RSC_BOX_ENABLE_COND0_CODE_EN					\
			(ASCII*) "               Enable               "
#define RSC_BOX_ENABLE_COND1_CODE_EN					\
			(ASCII*) "           <   Enable   >           "

#define RSC_BOX_INTERVAL_TITLE_CODE_EN	(ASCII*) " Interval "
#define RSC_BOX_AUTOCLOCK_TITLE_CODE_EN (ASCII*) " Auto Clock "
#define RSC_BOX_MODE_TITLE_CODE_EN	(ASCII*) " Experimental "

#define RSC_BOX_MODE_DESC_CODE_EN					\
			(ASCII*) "       CoreFreq Operation Mode       "
#define RSC_BOX_EIST_DESC_CODE_EN					\
			(ASCII*) "             SpeedStep              "
#define RSC_BOX_C1E_DESC_CODE_EN					\
			(ASCII*) "        Enhanced Halt State         "
#define RSC_BOX_TURBO_DESC_CODE_EN					\
			(ASCII*) " Turbo Boost/Core Performance Boost "
#define RSC_BOX_C1A_DESC_CODE_EN					\
			(ASCII*) "          C1 Auto Demotion          "
#define RSC_BOX_C3A_DESC_CODE_EN					\
			(ASCII*) "          C3 Auto Demotion          "
#define RSC_BOX_C1U_DESC_CODE_EN					\
			(ASCII*) "            C1 UnDemotion           "
#define RSC_BOX_C3U_DESC_CODE_EN					\
			(ASCII*) "            C3 UnDemotion           "
#define RSC_BOX_CC6_DESC_CODE_EN					\
			(ASCII*) "           Core C6 State            "
#define RSC_BOX_PC6_DESC_CODE_EN					\
			(ASCII*) "          Package C6 State          "
#define RSC_BOX_HWP_DESC_CODE_EN					\
			(ASCII*) "   Hardware-Controlled Performance  "
#define RSC_BOX_BLANK_DESC_CODE_EN					\
			(ASCII*) "                                    "

#define RSC_BOX_NOMINAL_MODE_COND0_CODE_EN				\
			(ASCII*) "       Nominal operating mode       "
#define RSC_BOX_NOMINAL_MODE_COND1_CODE_EN				\
			(ASCII*) "     < Nominal operating mode >     "
#define RSC_BOX_EXPERIMENT_MODE_COND0_CODE_EN				\
			(ASCII*) "     Experimental operating mode    "
#define RSC_BOX_EXPERIMENT_MODE_COND1_CODE_EN				\
			(ASCII*) "   < Experimental operating mode >  "

#define RSC_BOX_INTERRUPT_TITLE_CODE_EN (ASCII*) " NMI Interrupts "

#define RSC_BOX_INT_REGISTER_COND0_CODE_EN				\
			(ASCII*) "              Register              "
#define RSC_BOX_INT_REGISTER_COND1_CODE_EN				\
			(ASCII*) "            < Register >            "
#define RSC_BOX_INT_UNREGISTER_COND0_CODE_EN				\
			(ASCII*) "             Unregister             "
#define RSC_BOX_INT_UNREGISTER_COND1_CODE_EN				\
			(ASCII*) "           < Unregister >           "

#define RSC_BOX_EVENT_TITLE_CODE_EN	(ASCII*) " Clear Event "

#define RSC_BOX_EVENT_THERMAL_SENSOR_CODE_EN				\
					(ASCII*) "     Thermal Sensor     "
#define RSC_BOX_EVENT_PROCHOT_AGENT_CODE_EN				\
					(ASCII*) "     PROCHOT# Agent     "
#define RSC_BOX_EVENT_CRITICAL_TEMP_CODE_EN				\
					(ASCII*) "  Critical Temperature  "
#define RSC_BOX_EVENT_THERMAL_THRESHOLD_CODE_EN 			\
					(ASCII*) "   Thermal Threshold    "
#define RSC_BOX_EVENT_POWER_LIMITATION_CODE_EN				\
					(ASCII*) "    Power Limitation    "
#define RSC_BOX_EVENT_CURRENT_LIMITATION_CODE_EN			\
					(ASCII*) "   Current Limitation   "
#define RSC_BOX_EVENT_CROSS_DOMAIN_LIMIT_CODE_EN			\
					(ASCII*) "   Cross Domain Limit.  "

#define RSC_BOX_PKG_STATE_TITLE_CODE_EN (ASCII*) " Package C-State Limit "

#define RSC_BOX_IO_MWAIT_TITLE_CODE_EN	(ASCII*) " I/O MWAIT "
#define RSC_BOX_IO_MWAIT_DESC_CODE_EN					\
			(ASCII*) "        I/O MWAIT Redirection       "

#define RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_EN (ASCII*)" I/O MWAIT Max C-State "

#define RSC_BOX_ODCM_TITLE_CODE_EN	(ASCII*) " ODCM "
#define RSC_BOX_ODCM_DESC_CODE_EN					\
			(ASCII*) "          Clock Modulation          "

#define RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_EN	(ASCII*)" Extended Duty Cycle "
#define RSC_BOX_DUTY_CYCLE_TITLE_CODE_EN	(ASCII*) " Duty Cycle "

#define RSC_BOX_DUTY_CYCLE_RESERVED_CODE_EN				\
				(ASCII*) "          Reserved         "


#define RSC_BOX_POWER_POLICY_TITLE_CODE_EN	(ASCII*) " Energy Policy "

#define RSC_BOX_POWER_POLICY_LOW_CODE_EN  (ASCII*) "            0       LOW "
#define RSC_BOX_POWER_POLICY_HIGH_CODE_EN (ASCII*) "           15      HIGH "

#define RSC_BOX_HWP_POLICY_MIN_CODE_EN	(ASCII*) "         Minimum        "
#define RSC_BOX_HWP_POLICY_MED_CODE_EN	(ASCII*) "         Medium         "
#define RSC_BOX_HWP_POLICY_PWR_CODE_EN	(ASCII*) "         Power          "
#define RSC_BOX_HWP_POLICY_MAX_CODE_EN	(ASCII*) "         Maximum        "

#define RSC_BOX_TOOLS_TITLE_CODE_EN	(ASCII*) " Tools "
#define RSC_BOX_TOOLS_STOP_CODE_EN	(ASCII*) "            STOP           "
#define RSC_BOX_TOOLS_ATOMIC_CODE_EN	(ASCII*) "        Atomic Burn        "
#define RSC_BOX_TOOLS_CRC32_CODE_EN	(ASCII*) "       CRC32 Compute       "
#define RSC_BOX_TOOLS_CONIC_CODE_EN	(ASCII*) "       Conic Compute...    "
#define RSC_BOX_TOOLS_RAND_CPU_CODE_EN	(ASCII*) "      Turbo Random CPU     "
#define RSC_BOX_TOOLS_RR_CPU_CODE_EN	(ASCII*) "      Turbo Round Robin    "
#define RSC_BOX_TOOLS_USR_CPU_CODE_EN	(ASCII*) "      Turbo Select CPU...  "

#define RSC_BOX_CONIC_TITLE_CODE_EN	(ASCII*) " Conic variations "
#define RSC_BOX_CONIC_ITEM_1_CODE_EN	(ASCII*) "         Ellipsoid         "
#define RSC_BOX_CONIC_ITEM_2_CODE_EN	(ASCII*) " Hyperboloid of one sheet  "
#define RSC_BOX_CONIC_ITEM_3_CODE_EN	(ASCII*) " Hyperboloid of two sheets "
#define RSC_BOX_CONIC_ITEM_4_CODE_EN	(ASCII*) "    Elliptical cylinder    "
#define RSC_BOX_CONIC_ITEM_5_CODE_EN	(ASCII*) "    Hyperbolic cylinder    "
#define RSC_BOX_CONIC_ITEM_6_CODE_EN	(ASCII*) "    Two parallel planes    "

#define RSC_BOX_LANG_TITLE_CODE_EN	(ASCII*) " Languages "
#define RSC_BOX_LANG_ENGLISH_CODE_EN	(ASCII*) "     English     "
#define RSC_BOX_LANG_FRENCH_CODE_EN	(ASCII*) "     French      "

#define RSC_ERROR_CMD_SYNTAX_CODE_EN					\
		(ASCII*)"CoreFreq."					\
			"  Copyright (C) 2015-2019 CYRIL INGENIERIE\n\n"\
			"Usage:\t%s [-option <arguments>]\n"		\
			"\t-0,1,2\tMemory unit in K,M,G Byte\n" 	\
			"\t-F\tTemperature in Fahrenheit\n"		\
			"\t-t\tShow Top (default)\n"			\
			"\t-d\tShow Dashboard\n"			\
			"\t-V\tMonitor Power and Voltage\n"		\
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
		(ASCII*) "Daemon connection error code %d\n%s: '%s' @ line %d\n"

#define RSC_ERROR_SYS_CALL_CODE_EN					\
				(ASCII*) "System error code %d\n%s @ line %d\n"

#define RSC_BOX_IDLE_LIMIT_TITLE_CODE_EN (ASCII*) " CPU-Idle Limit "

#define RSC_BOX_RECORDER_TITLE_CODE_EN (ASCII*) " Duration "

#define RSC_SMBIOS_TITLE_CODE_EN	(ASCII*) " SMBIOS "
