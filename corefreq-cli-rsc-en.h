/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
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
	'O','F','F',' ',']',' '						\
}

#define RSC_LAYOUT_RULLER_VOLTAGE_CODE_EN				\
	"--- Freq(MHz) - VID - Vcore ------------------ Energy(J) ---"	\
	"-- Power(W) ------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_EN				\
{									\
	'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ','K','B',']' 	\
}

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
#define RSC_LEVEL_CODE_EN		(ASCII*) "Level"
#define RSC_PROGRAMMABLE_CODE_EN	(ASCII*) "Programmable"
#define RSC_CONFIGURATION_CODE_EN	(ASCII*) "Configuration"
#define RSC_TURBO_ACTIVATION_CODE_EN	(ASCII*) "Turbo Activation"
#define RSC_NOMINAL_CODE_EN		(ASCII*) "Nominal"
#define RSC_UNLOCK_CODE_EN		(ASCII*) "UNLOCK"
#define RSC_LOCK_CODE_EN		(ASCII*) "  LOCK"

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

#define RSC_TECHNOLOGIES_TITLE_CODE_EN	(ASCII*) " Technologies "

#define RSC_PERF_MON_TITLE_CODE_EN	(ASCII*) " Performance Monitoring "
#define RSC_VERSION_CODE_EN		(ASCII*) "Version"
#define RSC_COUNTERS_CODE_EN		(ASCII*) "Counters"
#define RSC_GENERAL_CTRS_CODE_EN	(ASCII*) "General"
#define RSC_FIXED_CTRS_CODE_EN		(ASCII*) "Fixed"

#define RSC_POWER_THERMAL_TITLE_CODE_EN (ASCII*) " Power & Thermal "

#define RSC_KERNEL_TITLE_CODE_EN	(ASCII*) " Kernel "
#define RSC_KERNEL_TOTAL_RAM_CODE_EN	(ASCII*) "Total RAM"
#define RSC_KERNEL_SHARED_RAM_CODE_EN	(ASCII*) "Shared RAM"
#define RSC_KERNEL_FREE_RAM_CODE_EN	(ASCII*) "Free RAM"
#define RSC_KERNEL_BUFFER_RAM_CODE_EN	(ASCII*) "Buffer RAM"
#define RSC_KERNEL_TOTAL_HIGH_CODE_EN	(ASCII*) "Total High"
#define RSC_KERNEL_FREE_HIGH_CODE_EN	(ASCII*) "Free High"
#define RSC_KERNEL_IDLE_DRIVER_CODE_EN	(ASCII*) "Idle driver"

#define RSC_RELEASE_CODE_EN		(ASCII*) "Release"
#define RSC_MACHINE_CODE_EN		(ASCII*) "Machine"
#define RSC_MEMORY_CODE_EN		(ASCII*) "Memory"
#define RSC_STATE_CODE_EN		(ASCII*) "State"
#define RSC_POWER_CODE_EN		(ASCII*) "Power"
#define RSC_LATENCY_CODE_EN		(ASCII*) "Latency"
#define RSC_RESIDENCY_CODE_EN		(ASCII*) "Residency"

#define RSC_TOPOLOGY_TITLE_CODE_EN	(ASCII*) " Topology "

#define RSC_MEM_CTRL_TITLE_CODE_EN	(ASCII*) " Memory Controller "

#define RSC_MENU_ITEM_MENU_CODE_EN	(ASCII*) "          Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_EN	(ASCII*) "          View          "
#define RSC_MENU_ITEM_WINDOW_CODE_EN	(ASCII*) "         Window         "
#define RSC_MENU_ITEM_SETTINGS_CODE_EN	(ASCII*) " Settings           [s] "
#define RSC_MENU_ITEM_KERNEL_CODE_EN	(ASCII*) " Kernel Data        [k] "
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
#define RSC_HELP_TITLE_CODE_EN		(ASCII*) " Help "
#define RSC_ADV_HELP_TITLE_CODE_EN	(ASCII*) " Shortcuts "
