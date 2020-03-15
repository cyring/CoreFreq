/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define RSC_LAYOUT_HEADER_PROC_CODE_FR					\
{									\
	' ','P','r','o','c','e','s','s','e','u','r','[' 		\
}

#define RSC_LAYOUT_HEADER_BCLK_CODE_FR					\
{									\
	' ','H','o','r','l','o','g','e',' ',' ',' ',' ',		\
	'~',' ','0','0','0',' ','0','0','0',' ','0','0','0',' ','H','z' \
}

#define RSC_LAYOUT_RULER_REL_LOAD_CODE_FR				\
{									\
	'F','r','e','q','u','e','n','c','e',' ','r','e','l','a','t','i',\
	'v','e' 							\
}

#define RSC_LAYOUT_RULER_ABS_LOAD_CODE_FR				\
{									\
	'F','r','e','q','u','e','n','c','e',' ','a','b','s','o','l','u',\
	'e',' ' 							\
}

#define RSC_LAYOUT_RULER_FREQUENCY_AVG_CODE_FR				\
{									\
	'-','-','-','-','-','-',' ','%',' ','M','o','y','e','n','n','e',\
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

#define RSC_LAYOUT_RULER_PACKAGE_CODE_FR				\
	"------------ Cycles ----  Etat -------------------- Ratio TS"	\
	"C ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_TASKS_CODE_FR					\
	"--- Freq(MHz) ---Taches                    -----------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_TASKS_STATE_SORTED_CODE_FR				\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ','E','t','a','t',' ',')',' ', '-','-','-'			\
}

#define RSC_LAYOUT_TASKS_RUNTIME_SORTED_CODE_FR 			\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ','R','u','n','T','i','m','e', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_USRTIME_SORTED_CODE_FR 			\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ','U','s','e','r','T','i','m', 'e',')',' '			\
}

#define RSC_LAYOUT_TASKS_SYSTIME_SORTED_CODE_FR 			\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ','S','y','s','T','i','m','e', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_PROCESS_SORTED_CODE_FR 			\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ','P','I','D',')',' ','-','-', '-','-','-'			\
}

#define RSC_LAYOUT_TASKS_COMMAND_SORTED_CODE_FR 			\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ','C','o','m','m','a','n','d', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_FR			\
{									\
	' ', 'I','n','v','e','r','s','e','r','[','O','F','F',']',' '	\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_ON_CODE_FR			\
{									\
	' ', 'I','n','v','e','r','s','e','r','[',' ','O','N',']',' '	\
}

#define RSC_LAYOUT_TASKS_VALUE_SWITCH_CODE_FR				\
{									\
	' ', 'V','a','l','e','u','r','[',' ',' ',' ',']',' '		\
}

#define RSC_LAYOUT_TASKS_TRACKING_CODE_FR				\
{									\
	'S','u','i','v','i',' ','[', 'n',']',' ','P','I','D',' ','[',' ',\
	'O','F','F',' ',']',' ' 					\
}

#define RSC_LAYOUT_RULER_SENSORS_CODE_FR				\
	"--- Freq(MHz) --- Vcore --- TMP( ) -- Energie(J) Puissance(W"	\
	") ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_ENERGY_CODE_FR 				\
	"--- Freq(MHz) -- Accumulateur -- Min   Energie(J)  Max -- Mi"	\
	"n Puissance(W) Max -----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_SLICE_CODE_FR					\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 ---------- Erreur -----------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_FR				\
{									\
	'T','a','c','h','e','s','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','B',']' 	\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_FR	"<  ACTIVER >"
#define RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_FR	"<DESACTIVER>"

#define RSC_LAYOUT_CARD_LOAD_CODE_FR					\
{									\
	'[',' ',' ','%','A','C','T','I','F',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_IDLE_CODE_FR					\
{									\
	'[',' ',' ','%','R','E','P','O','S',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_TASK_CODE_FR					\
{									\
	'[','T','a','c','h','e',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_PROCESSOR_TITLE_CODE_FR	" Processeur "
#define RSC_PROCESSOR_CODE_FR		"Processeur"
#define RSC_ARCHITECTURE_CODE_FR	"Architecture"
#define RSC_VENDOR_ID_CODE_FR		"ID vendeur"
#define RSC_MICROCODE_CODE_FR		"Microcode"
#define RSC_SIGNATURE_CODE_FR		"Signature"
#define RSC_STEPPING_CODE_FR		"Stepping"
#define RSC_ONLINE_CPU_CODE_FR		"CPU en ligne"
#define RSC_BASE_CLOCK_CODE_FR		"Horloge de base"
#define RSC_FREQUENCY_CODE_FR		"Frequence"
#define RSC_RATIO_CODE_FR		"Ratio"
#define RSC_FACTORY_CODE_FR		"Usine"
#define RSC_PERFORMANCE_CODE_FR 	"Performance"
#define RSC_TARGET_CODE_FR		"Cible"
#define RSC_LEVEL_CODE_FR		"Niveau"
#define RSC_PROGRAMMABLE_CODE_FR	"Programmable"
#define RSC_CONFIGURATION_CODE_FR	"Configuration"
#define RSC_TURBO_ACTIVATION_CODE_FR	"Activation Turbo"
#define RSC_NOMINAL_CODE_FR		"Nominal"
#define RSC_UNLOCK_CODE_FR		"OUVERT"
#define RSC_LOCK_CODE_FR		"BLOQUE"
#define RSC_ENABLE_CODE_FR		"  Actif"
#define RSC_DISABLE_CODE_FR		"Inactif"
#define RSC_CAPABILITIES_CODE_FR	"Capacites"
#define RSC_LOWEST_CODE_FR		"Faible"
#define RSC_EFFICIENT_CODE_FR		"Efficace"
#define RSC_GUARANTEED_CODE_FR		"Guarantie"
#define RSC_HIGHEST_CODE_FR		"Elevee"

#define RSC_SCOPE_NONE_CODE_FR		"Sans"
#define RSC_SCOPE_THREAD_CODE_FR	" SMT"
#define RSC_SCOPE_CORE_CODE_FR		"Core"
#define RSC_SCOPE_PACKAGE_CODE_FR	" Pkg"

#define RSC_CPUID_TITLE_FR 		\
	" fonction           EAX          EBX          ECX          EDX "

#define RSC_LARGEST_STD_FUNC_CODE_FR	"Fonction standard maximum"
#define RSC_LARGEST_EXT_FUNC_CODE_FR	"Fonction etendue  maximum"

#define RSC_SYS_REGS_TITLE_CODE_FR	" Registres Systeme "

#define RSC_ISA_TITLE_CODE_FR		" Jeu d'instructions etendu "

#define RSC_FEATURES_TITLE_CODE_FR	" Caracteristiques "
#define RSC_NOT_AVAILABLE_CODE_FR	"N/A"
#define RSC_AUTOMATIC_CODE_FR		"AUTO"
#define RSC_MISSING_CODE_FR		"Absent"
#define RSC_PRESENT_CODE_FR		"Capable"
#define RSC_VARIANT_CODE_FR		"Variant"
#define RSC_INVARIANT_CODE_FR		"Invariant"
#define RSC_FEATURES_1GB_PAGES_CODE_FR	"1 GB Pages Support"
#define RSC_FEATURES_100MHZ_CODE_FR	"100 MHz multiplier Control"
#define RSC_FEATURES_ACPI_CODE_FR   "Advanced Configuration & Power Interface"
#define RSC_FEATURES_APIC_CODE_FR   "Advanced Programmable Interrupt Controller"
#define RSC_FEATURES_CORE_MP_CODE_FR	"Core Multi-Processing"
#define RSC_FEATURES_CNXT_ID_CODE_FR	"L1 Data Cache Context ID"
#define RSC_FEATURES_DCA_CODE_FR	"Direct Cache Access"
#define RSC_FEATURES_DE_CODE_FR 	"Debugging Extension"
#define RSC_FEATURES_DS_PEBS_CODE_FR "Debug Store & Precise Event Based Sampling"
#define RSC_FEATURES_DS_CPL_CODE_FR	"CPL Qualified Debug Store"
#define RSC_FEATURES_DTES_64_CODE_FR	"64-Bit Debug Store"
#define RSC_FEATURES_FAST_STR_CODE_FR	"Fast-String Operation"
#define RSC_FEATURES_FMA_CODE_FR	"Fused Multiply Add"
#define RSC_FEATURES_HLE_CODE_FR	"Hardware Lock Elision"
#define RSC_FEATURES_LM_CODE_FR 	"Long Mode 64 bits"
#define RSC_FEATURES_LWP_CODE_FR	"LightWeight Profiling"
#define RSC_FEATURES_MCA_CODE_FR	"Machine-Check Architecture"
#define RSC_FEATURES_MSR_CODE_FR	"Model Specific Registers"
#define RSC_FEATURES_MTRR_CODE_FR	"Memory Type Range Registers"
#define RSC_FEATURES_NX_CODE_FR 	"No-Execute Page Protection"
#define RSC_FEATURES_OSXSAVE_CODE_FR	"OS-Enabled Ext. State Management"
#define RSC_FEATURES_PAE_CODE_FR	"Physical Address Extension"
#define RSC_FEATURES_PAT_CODE_FR	"Page Attribute Table"
#define RSC_FEATURES_PBE_CODE_FR	"Pending Break Enable"
#define RSC_FEATURES_PCID_CODE_FR	"Process Context Identifiers"
#define RSC_FEATURES_PDCM_CODE_FR	"Perfmon and Debug Capability"
#define RSC_FEATURES_PGE_CODE_FR	"Page Global Enable"
#define RSC_FEATURES_PSE_CODE_FR	"Page Size Extension"
#define RSC_FEATURES_PSE36_CODE_FR	"36-bit Page Size Extension"
#define RSC_FEATURES_PSN_CODE_FR	"Processor Serial Number"
#define RSC_FEATURES_RTM_CODE_FR	"Restricted Transactional Memory"
#define RSC_FEATURES_SMX_CODE_FR	"Safer Mode Extensions"
#define RSC_FEATURES_SELF_SNOOP_CODE_FR "Self-Snoop"
#define RSC_FEATURES_SMEP_CODE_FR	"Supervisor-Mode Execution Prevention"
#define RSC_FEATURES_TSC_CODE_FR	"Time Stamp Counter"
#define RSC_FEATURES_TSC_DEADLN_CODE_FR "Time Stamp Counter Deadline"
#define RSC_FEATURES_VME_CODE_FR	"Virtual Mode Extension"
#define RSC_FEATURES_VMX_CODE_FR	"Virtual Machine Extensions"
#define RSC_FEATURES_X2APIC_CODE_FR	"Extended xAPIC Support"
#define RSC_FEATURES_XD_BIT_CODE_FR	"Execution Disable Bit Support"
#define RSC_FEATURES_XSAVE_CODE_FR	"XSAVE/XSTOR States"
#define RSC_FEATURES_XTPR_CODE_FR	"xTPR Update Control"
#define RSC_FEAT_SECTION_MECH_CODE_FR	"Mecanismes d'attenuation"

#define RSC_TECHNOLOGIES_TITLE_CODE_FR	" Technologies "
#define RSC_TECHNOLOGIES_SMM_CODE_FR	"Mode de Gestion Systeme"
#define RSC_TECHNOLOGIES_HTT_CODE_FR	"Hyper-Threading"
#define RSC_TECHNOLOGIES_EIST_CODE_FR	"SpeedStep"
#define RSC_TECHNOLOGIES_IDA_CODE_FR	"Acceleration dynamique"
#define RSC_TECHNOLOGIES_TURBO_CODE_FR	"Turbo Boost"
#define RSC_TECHNOLOGIES_VM_CODE_FR	"Virtualisation"
#define RSC_TECHNOLOGIES_IOMMU_CODE_FR	"MMU E/S"
#define RSC_TECHNOLOGIES_SMT_CODE_FR	"Multithreading simultane"
#define RSC_TECHNOLOGIES_CNQ_CODE_FR	"PowerNow!"
#define RSC_TECHNOLOGIES_CPB_CODE_FR	"Core Performance Boost"
#define RSC_TECHNOLOGIES_HYPERV_CODE_FR "Hyperviseur"

#define RSC_PERF_MON_TITLE_CODE_FR	" Gestion de la performance "
#define RSC_VERSION_CODE_FR		"Version"
#define RSC_COUNTERS_CODE_FR		"Compteurs"
#define RSC_GENERAL_CTRS_CODE_FR	"Generaux"
#define RSC_FIXED_CTRS_CODE_FR		"Fixes"
#define RSC_PERF_MON_C1E_CODE_FR	"Enhanced Halt State"
#define RSC_PERF_MON_C1A_CODE_FR	"C1 Auto Demotion"
#define RSC_PERF_MON_C3A_CODE_FR	"C3 Auto Demotion"
#define RSC_PERF_MON_C1U_CODE_FR	"C1 UnDemotion"
#define RSC_PERF_MON_C3U_CODE_FR	"C3 UnDemotion"
#define RSC_PERF_MON_CC6_CODE_FR	"Core C6 State"
#define RSC_PERF_MON_PC6_CODE_FR	"Package C6 State"
#define RSC_PERF_MON_FID_CODE_FR	"Frequency ID control"
#define RSC_PERF_MON_VID_CODE_FR	"Voltage ID control"
#define RSC_PERF_MON_HWCF_CODE_FR	"P-State Hardware Coordination Feedback"
#define RSC_PERF_MON_HWP_CODE_FR	"Hardware-Controlled Performance States"
#define RSC_PERF_MON_HDC_CODE_FR	"Hardware Duty Cycling"
#define RSC_PERF_MON_PKG_CSTATE_CODE_FR "Package C-State"
#define RSC_PERF_MON_CFG_CTRL_CODE_FR	"Configuration Control"
#define RSC_PERF_MON_LOW_CSTATE_CODE_FR "Lowest C-State"
#define RSC_PERF_MON_IOMWAIT_CODE_FR	"I/O MWAIT Redirection"
#define RSC_PERF_MON_MAX_CSTATE_CODE_FR "Max C-State Inclusion"

#define RSC_PERF_MON_MONITOR_MWAIT_CODE_FR	"MONITOR/MWAIT"
#define RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_FR	"State index"
#define RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_FR	"Sub C-State"

#define RSC_PERF_MON_CORE_CYCLE_CODE_FR "Core Cycles"
#define RSC_PERF_MON_INST_RET_CODE_FR	"Instructions Retired"
#define RSC_PERF_MON_REF_CYCLE_CODE_FR	"Reference Cycles"
#define RSC_PERF_MON_REF_LLC_CODE_FR	"Last Level Cache References"
#define RSC_PERF_MON_MISS_LLC_CODE_FR	"Last Level Cache Misses"
#define RSC_PERF_MON_BRANCH_RET_CODE_FR "Branch Instructions Retired"
#define RSC_PERF_MON_BRANCH_MIS_CODE_FR "Branch Mispredicts Retired"

#define RSC_POWER_THERMAL_TITLE_CODE_FR " Puissance et thermique "
#define RSC_POWER_THERMAL_ODCM_CODE_FR	"Modulation d'horloge"
#define RSC_POWER_THERMAL_DUTY_CODE_FR	"Cycle de service"
#define RSC_POWER_THERMAL_MGMT_CODE_FR	"Gestion de la puissance"
#define RSC_POWER_THERMAL_BIAS_CODE_FR	"Regle energetique"
#define RSC_POWER_THERMAL_TJMAX_CODE_FR "Temperature de jonction"
#define RSC_POWER_THERMAL_DTS_CODE_FR	"Capteur thermique numerique"
#define RSC_POWER_THERMAL_PLN_CODE_FR	"Notification de puissance"
#define RSC_POWER_THERMAL_PTM_CODE_FR	"Gestion thermique du Package"
#define RSC_POWER_THERMAL_TM1_CODE_FR	"Controle Temperature 1"
#define RSC_POWER_THERMAL_TM2_CODE_FR	"Controle Temperature 2"
#define RSC_POWER_THERMAL_UNITS_CODE_FR "Unites"
#define RSC_POWER_THERMAL_POWER_CODE_FR "Puissance"
#define RSC_POWER_THERMAL_ENERGY_CODE_FR "Energie"
#define RSC_POWER_THERMAL_WINDOW_CODE_FR "Intervalle"
#define RSC_POWER_THERMAL_WATT_CODE_FR	"watt"
#define RSC_POWER_THERMAL_JOULE_CODE_FR "joule"
#define RSC_POWER_THERMAL_SECOND_CODE_FR "seconde"
#define RSC_POWER_THERMAL_TDP_CODE_FR	"Dissipation thermique"
#define RSC_POWER_THERMAL_MIN_CODE_FR	"Puissance minimale"
#define RSC_POWER_THERMAL_MAX_CODE_FR	"Puissance maximum"

#define RSC_KERNEL_TITLE_CODE_FR	" Noyau "
#define RSC_KERNEL_TOTAL_RAM_CODE_FR	"RAM totale"
#define RSC_KERNEL_SHARED_RAM_CODE_FR	"RAM partagee"
#define RSC_KERNEL_FREE_RAM_CODE_FR	"RAM libre"
#define RSC_KERNEL_BUFFER_RAM_CODE_FR	"RAM Tampon"
#define RSC_KERNEL_TOTAL_HIGH_CODE_FR	"Memoire haute totale"
#define RSC_KERNEL_FREE_HIGH_CODE_FR	"Memoire haute libre"
#define RSC_KERNEL_GOVERNOR_CODE_FR	"Gouverneur"
#define RSC_KERNEL_FREQ_DRIVER_CODE_FR	"Pilote CPU-Freq"
#define RSC_KERNEL_IDLE_DRIVER_CODE_FR	"Pilote CPU-Idle"
#define RSC_KERNEL_RELEASE_CODE_FR	"Edition"
#define RSC_KERNEL_VERSION_CODE_FR	"Version"
#define RSC_KERNEL_MACHINE_CODE_FR	"Machine"
#define RSC_KERNEL_MEMORY_CODE_FR	"Memoire"
#define RSC_KERNEL_STATE_CODE_FR	"Etat"
#define RSC_KERNEL_POWER_CODE_FR	"Puissance"
#define RSC_KERNEL_LATENCY_CODE_FR	"Latence"
#define RSC_KERNEL_RESIDENCY_CODE_FR	"Periode"
#define RSC_KERNEL_LIMIT_CODE_FR	"Limite Idle"

#define RSC_TOPOLOGY_TITLE_CODE_FR	" Topologie "

#define RSC_MEM_CTRL_TITLE_CODE_FR	" Controleur Memoire "
#define RSC_MEM_CTRL_BLANK_CODE_FR		"     "
#define RSC_MEM_CTRL_SUBSECT1_0_CODE_FR 	"Contr"
#define RSC_MEM_CTRL_SUBSECT1_1_CODE_FR 	"oleur"
#define RSC_MEM_CTRL_SUBSECT1_2_CODE_FR 	" #%-3u"
#define RSC_MEM_CTRL_SINGLE_CHA_0_CODE_FR	"Simpl"
#define RSC_MEM_CTRL_SINGLE_CHA_1_CODE_FR	"e Can"
#define RSC_MEM_CTRL_SINGLE_CHA_2_CODE_FR	"al   "
#define RSC_MEM_CTRL_DUAL_CHA_0_CODE_FR 	" Doub"
#define RSC_MEM_CTRL_DUAL_CHA_1_CODE_FR 	"le Ca"
#define RSC_MEM_CTRL_DUAL_CHA_2_CODE_FR 	"nal  "
#define RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_FR	"Tripl"
#define RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_FR	"e Can"
#define RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_FR	"al   "
#define RSC_MEM_CTRL_QUAD_CHA_0_CODE_FR 	" Quat"
#define RSC_MEM_CTRL_QUAD_CHA_1_CODE_FR 	"re Ca"
#define RSC_MEM_CTRL_QUAD_CHA_2_CODE_FR 	"naux "
#define RSC_MEM_CTRL_SIX_CHA_0_CODE_FR		"  Six"
#define RSC_MEM_CTRL_SIX_CHA_1_CODE_FR		" Cana"
#define RSC_MEM_CTRL_SIX_CHA_2_CODE_FR		"ux   "
#define RSC_MEM_CTRL_EIGHT_CHA_0_CODE_FR	"Huit "
#define RSC_MEM_CTRL_EIGHT_CHA_1_CODE_FR	"Canau"
#define RSC_MEM_CTRL_EIGHT_CHA_2_CODE_FR	"x    "
#define RSC_MEM_CTRL_BUS_RATE_0_CODE_FR 	" Debi"
#define RSC_MEM_CTRL_BUS_RATE_1_CODE_FR 	"t Bus"
#define RSC_MEM_CTRL_BUS_SPEED_0_CODE_FR	"Trans"
#define RSC_MEM_CTRL_BUS_SPEED_1_CODE_FR	"fert "
#define RSC_MEM_CTRL_DRAM_SPEED_0_CODE_FR	"Freq."
#define RSC_MEM_CTRL_DRAM_SPEED_1_CODE_FR	" DRAM"
#define RSC_MEM_CTRL_SUBSECT2_0_CODE_FR 	" Geom"
#define RSC_MEM_CTRL_SUBSECT2_1_CODE_FR 	"etrie"
#define RSC_MEM_CTRL_SUBSECT2_2_CODE_FR 	" DIMM"
#define RSC_MEM_CTRL_SUBSECT2_3_CODE_FR 	" du c"
#define RSC_MEM_CTRL_SUBSECT2_4_CODE_FR 	"anal "
#define RSC_MEM_CTRL_SUBSECT2_5_CODE_FR 	"#%-2u  "
#define RSC_MEM_CTRL_DIMM_SLOT_CODE_FR		" Slot"
#define RSC_MEM_CTRL_DIMM_BANK_CODE_FR		" Banq"
#define RSC_MEM_CTRL_DIMM_RANK_CODE_FR		" Rang"
#define RSC_MEM_CTRL_DIMM_ROW_CODE_FR		"Ligne"
#define RSC_MEM_CTRL_DIMM_COLUMN0_CODE_FR	"  Col"
#define RSC_MEM_CTRL_DIMM_COLUMN1_CODE_FR	"onne "
#define RSC_MEM_CTRL_DIMM_SIZE_0_CODE_FR	"Taill"
#define RSC_MEM_CTRL_DIMM_SIZE_1_CODE_FR	"e Mem"
#define RSC_MEM_CTRL_DIMM_SIZE_2_CODE_FR	"oire "
#define RSC_MEM_CTRL_DIMM_SIZE_3_CODE_FR	"(MB) "

#define RSC_TASKS_SORTBY_STATE_CODE_FR		" Etat     "
#define RSC_TASKS_SORTBY_RTIME_CODE_FR		" RunTime  "
#define RSC_TASKS_SORTBY_UTIME_CODE_FR		" UserTime "
#define RSC_TASKS_SORTBY_STIME_CODE_FR		" SysTime  "
#define RSC_TASKS_SORTBY_PID_CODE_FR		" PID      "
#define RSC_TASKS_SORTBY_COMM_CODE_FR		" Commande "

#define RSC_MENU_ITEM_MENU_CODE_FR		"     [F2] Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_FR		"     [F3] Vue           "
#define RSC_MENU_ITEM_WINDOW_CODE_FR		"    [F4] Fenetre        "
#define RSC_MENU_ITEM_SETTINGS_CODE_FR		" Reglages           [s] "
#define RSC_MENU_ITEM_SMBIOS_CODE_FR		" Infos SMBIOS       [B] "
#define RSC_MENU_ITEM_KERNEL_CODE_FR		" Infos Noyau        [k] "
#define RSC_MENU_ITEM_HOTPLUG_CODE_FR		" HotPlug CPU        [#] "
#define RSC_MENU_ITEM_TOOLS_CODE_FR		" Outils             [O] "
#define RSC_MENU_ITEM_ABOUT_CODE_FR		" Apropos            [a] "
#define RSC_MENU_ITEM_HELP_CODE_FR		" Aide               [h] "
#define RSC_MENU_ITEM_KEYS_CODE_FR		" Raccourcis        [F1] "
#define RSC_MENU_ITEM_LANG_CODE_FR		" Langues            [L] "
#define RSC_MENU_ITEM_QUIT_CODE_FR		" Quitter     [Ctrl]+[x] "
#define RSC_MENU_ITEM_DASHBOARD_CODE_FR 	" Tableau de bord    [d] "
#define RSC_MENU_ITEM_FREQUENCY_CODE_FR 	" Frequence          [f] "
#define RSC_MENU_ITEM_INST_CYCLE_CODE_FR	" Cycles Instruction [i] "
#define RSC_MENU_ITEM_CORE_CYCLE_CODE_FR	" Cycles des Coeurs  [c] "
#define RSC_MENU_ITEM_IDLE_STATE_CODE_FR	" Etats de sommeil   [l] "
#define RSC_MENU_ITEM_PKG_CYCLE_CODE_FR 	" Cycles du Package  [g] "
#define RSC_MENU_ITEM_TASKS_MON_CODE_FR 	" Suivi des taches   [x] "
#define RSC_MENU_ITEM_SYS_INTER_CODE_FR 	" Interruptions      [q] "
#define RSC_MENU_ITEM_SENSORS_CODE_FR		" Capteurs           [C] "
#define RSC_MENU_ITEM_VOLTAGE_CODE_FR		"   Voltage          [V] "
#define RSC_MENU_ITEM_POWER_CODE_FR		"   Puissance        [W] "
#define RSC_MENU_ITEM_SLICE_CTR_CODE_FR 	" Compteurs tranche  [T] "
#define RSC_MENU_ITEM_PROCESSOR_CODE_FR 	" Processeur         [p] "
#define RSC_MENU_ITEM_TOPOLOGY_CODE_FR		" Topologie          [m] "
#define RSC_MENU_ITEM_FEATURES_CODE_FR		" Caracteristiques   [e] "
#define RSC_MENU_ITEM_ISA_EXT_CODE_FR		" Jeu Instructions   [I] "
#define RSC_MENU_ITEM_TECH_CODE_FR		" Technologies       [t] "
#define RSC_MENU_ITEM_PERF_MON_CODE_FR		" Gestion de Perf.   [o] "
#define RSC_MENU_ITEM_POW_THERM_CODE_FR 	" Puissance-Therm.   [w] "
#define RSC_MENU_ITEM_CPUID_CODE_FR		" Extraction CPUID   [u] "
#define RSC_MENU_ITEM_SYS_REGS_CODE_FR		" Registres Systeme  [R] "
#define RSC_MENU_ITEM_MEM_CTRL_CODE_FR		" Controleur Memoire [M] "

#define RSC_SETTINGS_TITLE_CODE_FR	      " Reglages "
#define RSC_SETTINGS_DAEMON_CODE_FR	      " Acces demon                    "
#define RSC_SETTINGS_INTERVAL_CODE_FR	      " Intervalle(ms)          <    > "
#define RSC_SETTINGS_RECORDER_CODE_FR	      " Enregistreur(sec)       <    > "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_FR       " Auto Clock               <   > "
#define RSC_SETTINGS_EXPERIMENTAL_CODE_FR     " Experimental             <   > "
#define RSC_SETTINGS_CPU_HOTPLUG_CODE_FR      " Hot-Plug CPU             [   ] "
#define RSC_SETTINGS_PCI_ENABLED_CODE_FR      " Activation PCI           [   ] "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_FR   " Activation NMI           <   > "
#define RSC_SETTINGS_CPUIDLE_REGISTER_CODE_FR " Pilote CPU-IDLE          [   ] "
#define RSC_SETTINGS_CPUFREQ_REGISTER_CODE_FR " Pilote CPU-FREQ          [   ] "
#define RSC_SETTINGS_GOVERNOR_CPUFREQ_CODE_FR " Gouverneur CPU-FREQ      [   ] "
#define RSC_SETTINGS_THERMAL_SCOPE_CODE_FR    " Capteur thermique       <    > "
#define RSC_SETTINGS_VOLTAGE_SCOPE_CODE_FR    " Capteur de tension      <    > "
#define RSC_SETTINGS_POWER_SCOPE_CODE_FR      " Capteur de puissance    <    > "

#define RSC_HELP_TITLE_CODE_FR		" Aide "
#define RSC_HELP_KEY_ESCAPE_CODE_FR	" [Echap]          "
#define RSC_HELP_KEY_SHIFT_TAB_CODE_FR	" [Maj]+[Tab]      "
#define RSC_HELP_KEY_TAB_CODE_FR	" [Tab]            "
#define RSC_HELP_KEY_UP_CODE_FR 	"      [Haut]      "
#define RSC_HELP_KEY_LEFT_RIGHT_CODE_FR " [Gauche] [Droite]"
#define RSC_HELP_KEY_DOWN_CODE_FR	"      [Bas]       "
#define RSC_HELP_KEY_END_CODE_FR	" [Fin]            "
#define RSC_HELP_KEY_HOME_CODE_FR	" [Debut]          "
#define RSC_HELP_KEY_ENTER_CODE_FR	" [Entree]         "
#define RSC_HELP_KEY_PAGE_UP_CODE_FR	" [Page-Prec]      "
#define RSC_HELP_KEY_PAGE_DOWN_CODE_FR	" [Page-Suiv]      "
#define RSC_HELP_KEY_MINUS_CODE_FR	" [Moins]          "
#define RSC_HELP_KEY_PLUS_CODE_FR	" [Plus]           "
#define RSC_HELP_MENU_CODE_FR		"             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_FR	"   Fermer fenetre "
#define RSC_HELP_PREV_WINDOW_CODE_FR	"  Fenetre arriere "
#define RSC_HELP_NEXT_WINDOW_CODE_FR	" Fenetre suivante "
#define RSC_HELP_MOVE_WINDOW_CODE_FR	" [Maj]   Deplacer "
#define RSC_HELP_SIZE_WINDOW_CODE_FR	" [Alt]  Retailler "
#define RSC_HELP_MOVE_SELECT_CODE_FR	" Deplacer curseur "
#define RSC_HELP_LAST_CELL_CODE_FR	" Derniere cellule "
#define RSC_HELP_FIRST_CELL_CODE_FR	" Premiere cellule "
#define RSC_HELP_TRIGGER_SELECT_CODE_FR " Executer cellule "
#define RSC_HELP_PREV_PAGE_CODE_FR	"  Page precedente "
#define RSC_HELP_NEXT_PAGE_CODE_FR	"    Page suivante "
#define RSC_HELP_SCROLL_DOWN_CODE_FR	"    Decroitre CPU "
#define RSC_HELP_SCROLL_UP_CODE_FR	"    Accroitre CPU "

#define RSC_ADV_HELP_TITLE_CODE_FR	" Raccourcis "
#define RSC_ADV_HELP_SECT_FREQ_CODE_FR	" Vue Frequence:                       "
#define RSC_ADV_HELP_ITEM_AVG_CODE_FR	" %           Moyennes | Etats-Package "
#define RSC_ADV_HELP_SECT_TASK_CODE_FR	" Vue Suivi des taches:                "
#define RSC_ADV_HELP_ITEM_ORDER_CODE_FR " b          Critere de tri des taches "
#define RSC_ADV_HELP_ITEM_SEL_CODE_FR	" n          Choisir la tache a suivre "
#define RSC_ADV_HELP_ITEM_REV_CODE_FR	" r         Inverser le tri des taches "
#define RSC_ADV_HELP_ITEM_HIDE_CODE_FR	" v          Afficher | Cacher valeurs "
#define RSC_ADV_HELP_SECT_ANY_CODE_FR	" Vue quelconque:                      "
#define RSC_ADV_HELP_ITEM_POWER_CODE_FR " $            Energie en Joule | Watt "
#define RSC_ADV_HELP_ITEM_TOP_CODE_FR	" .             Frequence Top ou Usage "
#define RSC_ADV_HELP_ITEM_UPD_CODE_FR	" *                Rafraichir CoreFreq "
#define RSC_ADV_HELP_ITEM_START_CODE_FR " {                  Demarrer CoreFreq "
#define RSC_ADV_HELP_ITEM_STOP_CODE_FR	" }                   Arreter CoreFreq "
#define RSC_ADV_HELP_ITEM_TOOLS_CODE_FR " F10               Arreter les outils "
#define RSC_ADV_HELP_ITEM_GO_UP_CODE_FR " Haut Prec                 Defilement "
#define RSC_ADV_HELP_ITEM_GO_DW_CODE_FR " Bas  Suiv                       CPU  "
#define RSC_ADV_HELP_TERMINAL_CODE_FR	" Terminal:                            "
#define RSC_ADV_HELP_PRT_SCR_CODE_FR	" [Ctrl]+[p]                    Copier "
#define RSC_ADV_HELP_REC_SCR_CODE_FR	" [Alt]+[p]                Enregistrer "
#define RSC_ADV_HELP_FAHR_CELS_CODE_FR	" F              Fahrenheit ou Celsius "
#define RSC_ADV_HELP_PROC_EVENT_CODE_FR " H           Gerer Alertes Processeur "
#define RSC_ADV_HELP_SECRET_CODE_FR	" Y          Basculer donnees secretes "

#define RSC_TURBO_CLOCK_TITLE_CODE_FR	" Freq. Turbo %1dC "
#define RSC_RATIO_CLOCK_TITLE_CODE_FR	" %s Ratio Freq. "
#define RSC_UNCORE_CLOCK_TITLE_CODE_FR	" %s Uncore Freq. "

#define RSC_SELECT_CPU_TITLE_CODE_FR	" CPU   Pkg  Core Thread "
#define RSC_SELECT_FREQ_TITLE_CODE_FR	" CPU   Pkg  Core Thread "	\
					"  Frequence   Ratio "

#define RSC_BOX_DISABLE_COND0_CODE_FR	"             Desactiver             "
#define RSC_BOX_DISABLE_COND1_CODE_FR	"           < Desactiver >           "
#define RSC_BOX_ENABLE_COND0_CODE_FR	"               Activer              "
#define RSC_BOX_ENABLE_COND1_CODE_FR	"           <   Activer  >           "

#define RSC_BOX_INTERVAL_TITLE_CODE_FR	"Intervalle"
#define RSC_BOX_AUTOCLOCK_TITLE_CODE_FR " Auto Clock "
#define RSC_BOX_MODE_TITLE_CODE_FR	" Experimental "

#define RSC_BOX_MODE_DESC_CODE_FR	"     CoreFreq mode operationnel     "
#define RSC_BOX_EIST_DESC_CODE_FR	"             SpeedStep              "
#define RSC_BOX_C1E_DESC_CODE_FR	"        Etat de Pause etendu        "
#define RSC_BOX_TURBO_DESC_CODE_FR	" Turbo Boost/Core Performance Boost "
#define RSC_BOX_C1A_DESC_CODE_FR	"       Auto retrogradation C1       "
#define RSC_BOX_C3A_DESC_CODE_FR	"       Auto retrogradation C3       "
#define RSC_BOX_C1U_DESC_CODE_FR	"        Non-retrogradation C1       "
#define RSC_BOX_C3U_DESC_CODE_FR	"        Non-retrogradation C3       "
#define RSC_BOX_CC6_DESC_CODE_FR	"            Core Etat C6            "
#define RSC_BOX_PC6_DESC_CODE_FR	"           Package Etat C6          "
#define RSC_BOX_HWP_DESC_CODE_FR	" Controle Materiel de la Performance"
#define RSC_BOX_BLANK_DESC_CODE_FR	"                                    "

#define RSC_BOX_NOMINAL_MODE_COND0_CODE_FR "       Fonctionnement nominal       "
#define RSC_BOX_NOMINAL_MODE_COND1_CODE_FR "     < Fonctionnement nominal >     "
#define RSC_BOX_EXPER_MODE_COND0_CODE_FR   "     Fonctionnement experimental    "
#define RSC_BOX_EXPER_MODE_COND1_CODE_FR   "   < Fonctionnement experimental >  "

#define RSC_BOX_INTERRUPT_TITLE_CODE_FR " Interruptions NMI "

#define RSC_BOX_INT_REGISTER_ST0_CODE_FR   "             Enregistrer            "
#define RSC_BOX_INT_REGISTER_ST1_CODE_FR   "         <   Enregistrer  >         "
#define RSC_BOX_INT_UNREGISTER_ST0_CODE_FR "           Desenregistrer           "
#define RSC_BOX_INT_UNREGISTER_ST1_CODE_FR "         < Desenregistrer >         "

#define RSC_BOX_EVENT_TITLE_CODE_FR		" Effacer Evenement "

#define RSC_BOX_EVENT_THERMAL_SENSOR_CODE_FR	"    Capteur thermique    "
#define RSC_BOX_EVENT_PROCHOT_AGENT_CODE_FR	"      Agent PROCHOT#     "
#define RSC_BOX_EVENT_CRITICAL_TEMP_CODE_FR	"   Temperature critique  "
#define RSC_BOX_EVENT_THERM_THRESHOLD_CODE_FR	"     Seuil thermique     "
#define RSC_BOX_EVENT_POWER_LIMIT_CODE_FR	" Limitation de puissance "
#define RSC_BOX_EVENT_CURRENT_LIMIT_CODE_FR	"  Limitation de courant  "
#define RSC_BOX_EVENT_CROSS_DOM_LIMIT_CODE_FR	" Limitation interdomaine "

#define RSC_BOX_PKG_STATE_TITLE_CODE_FR 	" Limite Etats Package "

#define RSC_BOX_IO_MWAIT_TITLE_CODE_FR		" E/S MWAIT "
#define RSC_BOX_IO_MWAIT_DESC_CODE_FR	"        Redirection E/S MWAIT       "

#define RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_FR	" E/S MWAIT Etat Max "

#define RSC_BOX_ODCM_TITLE_CODE_FR		" ODCM "
#define RSC_BOX_ODCM_DESC_CODE_FR	"        Modulation d'horloge        "

#define RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_FR	" Cycle de service etendu "
#define RSC_BOX_DUTY_CYCLE_TITLE_CODE_FR	" Cycle de service "

#define RSC_BOX_DUTY_CYCLE_RESERVED_CODE_FR	"           Reserve         "

#define RSC_BOX_POWER_POLICY_TITLE_CODE_FR	" Regle energetique "

#define RSC_BOX_POWER_POLICY_LOW_CODE_FR	"            0       BAS "
#define RSC_BOX_POWER_POLICY_HIGH_CODE_FR	"           15      HAUT "

#define RSC_BOX_HWP_POLICY_MIN_CODE_FR		"        Minimale        "
#define RSC_BOX_HWP_POLICY_MED_CODE_FR		"        Moyenne         "
#define RSC_BOX_HWP_POLICY_PWR_CODE_FR		"        Puissance       "
#define RSC_BOX_HWP_POLICY_MAX_CODE_FR		"        Maximale        "

#define RSC_BOX_TOOLS_TITLE_CODE_FR		" Outils "
#define RSC_BOX_TOOLS_STOP_CODE_FR		"          ARRETER          "
#define RSC_BOX_TOOLS_ATOMIC_CODE_FR		"       Stress Atomic       "
#define RSC_BOX_TOOLS_CRC32_CODE_FR		"        Calcul CRC32       "
#define RSC_BOX_TOOLS_CONIC_CODE_FR		"      Calcul Conique...    "
#define RSC_BOX_TOOLS_RAND_CPU_CODE_FR		"    Turbo CPU aleatoire    "
#define RSC_BOX_TOOLS_RR_CPU_CODE_FR		"    Turbo CPU circulaire   "
#define RSC_BOX_TOOLS_USR_CPU_CODE_FR		"    Turbo Choisir CPU...   "

#define RSC_BOX_CONIC_TITLE_CODE_FR		" Variations Coniques "
#define RSC_BOX_CONIC_ITEM_1_CODE_FR		"         Ellipsoide         "
#define RSC_BOX_CONIC_ITEM_2_CODE_FR		"  Hyperboloide a une nappe  "
#define RSC_BOX_CONIC_ITEM_3_CODE_FR		" Hyperboloide a deux nappes "
#define RSC_BOX_CONIC_ITEM_4_CODE_FR		"     Cylindre elliptique    "
#define RSC_BOX_CONIC_ITEM_5_CODE_FR		"     Cylindre hyperbolique  "
#define RSC_BOX_CONIC_ITEM_6_CODE_FR		"    Deux plans paralleles   "

#define RSC_BOX_LANG_TITLE_CODE_FR		" Langues "
#define RSC_BOX_LANG_ENGLISH_CODE_FR		"     Anglais     "
#define RSC_BOX_LANG_FRENCH_CODE_FR		"     Francais    "

#define RSC_BOX_SCOPE_THERMAL_TITLE_CODE_FR	" Temperature "
#define RSC_BOX_SCOPE_VOLTAGE_TITLE_CODE_FR	" Tension "
#define RSC_BOX_SCOPE_POWER_TITLE_CODE_FR	" Puissance "
#define RSC_BOX_SCOPE_NONE_CODE_FR		"       Sans       "
#define RSC_BOX_SCOPE_THREAD_CODE_FR		"      Thread      "
#define RSC_BOX_SCOPE_CORE_CODE_FR		"       Coeur      "
#define RSC_BOX_SCOPE_PACKAGE_CODE_FR		"      Package     "

#define RSC_ERROR_CMD_SYNTAX_CODE_FR					\
		"CoreFreq."						\
		"  Copyright (C) 2015-2020 CYRIL INGENIERIE\n\n"	\
		"Usage:\t%s [-option <arguments>]\n"			\
		"\t-0,1,2\tUnité mémoire en K,M,G octet\n"		\
		"\t-F\tTemperature en Fahrenheit\n"			\
		"\t-J <#>\tNuméro d'index de chaîne SMBIOS\n"		\
		"\t-Y\tAfficher les données secrètes\n" 		\
		"\t-t\tAfficher Top (par défault)\n"			\
		"\t-d\tAfficher le tableau de bord\n"			\
		"\t-C\tMoniteur des Capteurs\n" 			\
		"\t-V\tMoniteur de Voltage\n"				\
		"\t-W\tMoniteur de Puissance\n" 			\
		"\t-g\tMoniteur du Package\n"				\
		"\t-c\tMoniteur des Compteurs\n"			\
		"\t-i\tMoniteur des Instructions\n"			\
		"\t-s\tImprimer les Informations du système\n"		\
		"\t-j\tImprimer les Informations (format json)\n"	\
		"\t-M\tImprimer le Controlleur mémoire\n"		\
		"\t-R\tImprimer les Registres du système\n"		\
		"\t-m\tImprimer la Topologie\n" 			\
		"\t-B\tImprimer SMBIOS\n"				\
		"\t-u\tImprimer CPUID\n"				\
		"\t-k\tImprimer les données du Kernel\n"		\
		"\t-h\tAfficher ce message\n"				\
		"\t-v\tAfficher le numéro de version\n"			\
		"\nCode d'exécution:\n"					\
		"\t0\tSUCCESS\t\tExécution réussie\n"			\
		"\t1\tCMD_SYNTAX\tErreur de syntaxe de la commande\n"	\
		"\t2\tSHM_FILE\tErreur du fichier de la mémoire partagée\n"\
		"\t3\tSHM_MMAP\tErreur de mappage de la mémoire partagée\n"\
		"\t4\tPERM_ERR\tExécution non autorisée\n"		\
		"\t5\tMEM_ERR\t\tErreur de fonctionnement de la mémoire\n"\
		"\t6\tEXEC_ERR\tErreur d'exécution générale\n"		\
		"\t15\tSYS_CALL\tErreur d’appel système\n"		\
		"\nSignaler toutes anomalies à labs[at]cyring.fr\n"

#define RSC_ERROR_SHARED_MEM_CODE_FR					\
	"Erreur code %d de connexion au démon.\n%s: '%s' @ ligne %d\n"

#define RSC_ERROR_SYS_CALL_CODE_FR	"Erreur Système code %d\n%s @ ligne %d\n"

#define RSC_BOX_IDLE_LIMIT_TITLE_CODE_FR " Limite CPU-Idle "

#define RSC_BOX_RECORDER_TITLE_CODE_FR	" Duree "

#define RSC_SMBIOS_TITLE_CODE_FR	" SMBIOS "

#define RSC_MECH_IBRS_CODE_FR	"Indirect Branch Restricted Speculation"
#define RSC_MECH_IBPB_CODE_FR	"Indirect Branch Prediction Barrier"
#define RSC_MECH_STIBP_CODE_FR	"Single Thread Indirect Branch Predictor"
#define RSC_MECH_SSBD_CODE_FR	"Speculative Store Bypass Disable"
#define RSC_MECH_L1D_FLUSH_CODE_FR "Writeback & invalidate the L1 data cache"
#define RSC_MECH_MD_CLEAR_CODE_FR  "Architectural - Buffer Overwriting"
#define RSC_MECH_RDCL_NO_CODE_FR   "Architectural - Rogue Data Cache Load"
#define RSC_MECH_IBRS_ALL_CODE_FR  "Architectural - Enhanced IBRS"
#define RSC_MECH_RSBA_CODE_FR	"Architectural - Return Stack Buffer Alternate"
#define RSC_MECH_L1DFL_VMENTRY_NO_CODE_FR \
				"Hypervisor - No flush L1D on VM entry"
#define RSC_MECH_SSB_NO_CODE_FR "Architectural - Speculative Store Bypass"
#define RSC_MECH_MDS_NO_CODE_FR \
			"Architectural - Microarchitectural Data Sampling"
#define RSC_MECH_PSCHANGE_MC_NO_CODE_FR "Architectural - Page Size Change MCE"
#define RSC_MECH_TAA_NO_CODE_FR "Architectural - TSX Asynchronous Abort"

#define RSC_CREATE_SELECT_FREQ_TGT_CODE_FR	"  TGT       Processeur    "
#define RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_FR	"  HWP-TGT   Processeur    "
#define RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_FR	"  HWP-MAX   Processeur    "
#define RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_FR	"  HWP-MIN   Processeur    "

