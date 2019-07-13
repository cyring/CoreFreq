/*
 * CoreFreq
 * Copyright (C) 2015-2019 CYRIL INGENIERIE
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

#define RSC_LAYOUT_RULLER_FREQUENCY_AVG_CODE_FR 			\
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

#define RSC_LAYOUT_RULLER_PACKAGE_CODE_FR				\
	"------------ Cycles ----  Etat -------------------- Ratio TS"	\
	"C ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULLER_TASKS_CODE_FR 				\
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

#define RSC_LAYOUT_RULLER_VOLTAGE_CODE_FR				\
	"--- Freq(MHz) - VID - Vcore ----------------- Energie(J) - P"	\
	"uissance(W) ------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_DOMAIN_PACKAGE_CODE_FR				\
{									\
	'P','a','c','k','a','g','e'					\
}

#define RSC_LAYOUT_DOMAIN_CORES_CODE_FR 				\
{									\
	'C','o','r','e','s',' ',' '					\
}

#define RSC_LAYOUT_DOMAIN_UNCORE_CODE_FR				\
{									\
	'U','n','c','o','r','e',' '					\
}

#define RSC_LAYOUT_DOMAIN_MEMORY_CODE_FR				\
{									\
	'M','e','m','o','i','r','e'					\
}

#define RSC_LAYOUT_RULLER_SLICE_CODE_FR 				\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 ---------- Erreur -----------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_FR				\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',			\
	'T','a','c','h','e','s','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','B',']' 	\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_FR	(ASCII*) "[  ACTIVER ]"
#define RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_FR	(ASCII*) "[DESACTIVER]"

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

#define RSC_PROCESSOR_TITLE_CODE_FR	(ASCII*) " Processeur "
#define RSC_PROCESSOR_CODE_FR		(ASCII*) "Processeur"
#define RSC_ARCHITECTURE_CODE_FR	(ASCII*) "Architecture"
#define RSC_VENDOR_ID_CODE_FR		(ASCII*) "ID vendeur"
#define RSC_MICROCODE_CODE_FR		(ASCII*) "Microcode"
#define RSC_SIGNATURE_CODE_FR		(ASCII*) "Signature"
#define RSC_STEPPING_CODE_FR		(ASCII*) "Stepping"
#define RSC_ONLINE_CPU_CODE_FR		(ASCII*) "CPU en ligne"
#define RSC_BASE_CLOCK_CODE_FR		(ASCII*) "Horloge de base"
#define RSC_FREQUENCY_CODE_FR		(ASCII*) "Frequence"
#define RSC_RATIO_CODE_FR		(ASCII*) "Ratio"
#define RSC_FACTORY_CODE_FR		(ASCII*) "Usine"
#define RSC_PERFORMANCE_CODE_FR 	(ASCII*) "Performance"
#define RSC_TARGET_CODE_FR		(ASCII*) "Cible"
#define RSC_LEVEL_CODE_FR		(ASCII*) "Niveau"
#define RSC_PROGRAMMABLE_CODE_FR	(ASCII*) "Programmable"
#define RSC_CONFIGURATION_CODE_FR	(ASCII*) "Configuration"
#define RSC_TURBO_ACTIVATION_CODE_FR	(ASCII*) "Activation Turbo"
#define RSC_NOMINAL_CODE_FR		(ASCII*) "Nominal"
#define RSC_UNLOCK_CODE_FR		(ASCII*) "OUVERT"
#define RSC_LOCK_CODE_FR		(ASCII*) "BLOQUE"
#define RSC_ENABLE_CODE_FR		(ASCII*) "  Actif"
#define RSC_DISABLE_CODE_FR		(ASCII*) "Inactif"
#define RSC_CAPABILITIES_CODE_FR	(ASCII*) "Capacites"
#define RSC_LOWEST_CODE_FR		(ASCII*) "Faible"
#define RSC_EFFICIENT_CODE_FR		(ASCII*) "Efficace"
#define RSC_GUARANTEED_CODE_FR		(ASCII*) "Guarantie"
#define RSC_HIGHEST_CODE_FR		(ASCII*) "Elevee"

#define RSC_CPUID_TITLE_FR 		\
(ASCII*) " fonction           EAX          EBX          ECX          EDX "

#define RSC_LARGEST_STD_FUNC_CODE_FR	(ASCII*) "Fonction standard maximum"
#define RSC_LARGEST_EXT_FUNC_CODE_FR	(ASCII*) "Fonction etendue  maximum"

#define RSC_SYS_REGS_TITLE_CODE_FR	(ASCII*) " Registres Systeme "

#define RSC_ISA_TITLE_CODE_FR		(ASCII*) " Jeu d'instructions etendu "

#define RSC_FEATURES_TITLE_CODE_FR	(ASCII*) " Caracteristiques "
#define RSC_MISSING_CODE_FR		(ASCII*) "Absent"
#define RSC_PRESENT_CODE_FR		(ASCII*) "Present"
#define RSC_VARIANT_CODE_FR		(ASCII*) "Variant"
#define RSC_INVARIANT_CODE_FR		(ASCII*) "Invariant"
#define RSC_FEATURES_1GB_PAGES_CODE_FR	(ASCII*) "1 GB Pages Support"
#define RSC_FEATURES_100MHZ_CODE_FR	(ASCII*) "100 MHz multiplier Control"
#define RSC_FEATURES_ACPI_CODE_FR					\
			(ASCII*) "Advanced Configuration & Power Interface"
#define RSC_FEATURES_APIC_CODE_FR					\
			(ASCII*) "Advanced Programmable Interrupt Controller"
#define RSC_FEATURES_CORE_MP_CODE_FR	(ASCII*) "Core Multi-Processing"
#define RSC_FEATURES_CNXT_ID_CODE_FR	(ASCII*) "L1 Data Cache Context ID"
#define RSC_FEATURES_DCA_CODE_FR	(ASCII*) "Direct Cache Access"
#define RSC_FEATURES_DE_CODE_FR 	(ASCII*) "Debugging Extension"
#define RSC_FEATURES_DS_PEBS_CODE_FR					\
			(ASCII*) "Debug Store & Precise Event Based Sampling"
#define RSC_FEATURES_DS_CPL_CODE_FR	(ASCII*) "CPL Qualified Debug Store"
#define RSC_FEATURES_DTES_64_CODE_FR	(ASCII*) "64-Bit Debug Store"
#define RSC_FEATURES_FAST_STR_CODE_FR	(ASCII*) "Fast-String Operation"
#define RSC_FEATURES_FMA_CODE_FR	(ASCII*) "Fused Multiply Add"
#define RSC_FEATURES_HLE_CODE_FR	(ASCII*) "Hardware Lock Elision"
#define RSC_FEATURES_LM_CODE_FR 	(ASCII*) "Long Mode 64 bits"
#define RSC_FEATURES_LWP_CODE_FR	(ASCII*) "LightWeight Profiling"
#define RSC_FEATURES_MCA_CODE_FR	(ASCII*) "Machine-Check Architecture"
#define RSC_FEATURES_MSR_CODE_FR	(ASCII*) "Model Specific Registers"
#define RSC_FEATURES_MTRR_CODE_FR	(ASCII*) "Memory Type Range Registers"
#define RSC_FEATURES_NX_CODE_FR 	(ASCII*) "No-Execute Page Protection"
#define RSC_FEATURES_OSXSAVE_CODE_FR					\
				(ASCII*) "OS-Enabled Ext. State Management"
#define RSC_FEATURES_PAE_CODE_FR	(ASCII*) "Physical Address Extension"
#define RSC_FEATURES_PAT_CODE_FR	(ASCII*) "Page Attribute Table"
#define RSC_FEATURES_PBE_CODE_FR	(ASCII*) "Pending Break Enable"
#define RSC_FEATURES_PCID_CODE_FR	(ASCII*) "Process Context Identifiers"
#define RSC_FEATURES_PDCM_CODE_FR	(ASCII*) "Perfmon and Debug Capability"
#define RSC_FEATURES_PGE_CODE_FR	(ASCII*) "Page Global Enable"
#define RSC_FEATURES_PSE_CODE_FR	(ASCII*) "Page Size Extension"
#define RSC_FEATURES_PSE36_CODE_FR	(ASCII*) "36-bit Page Size Extension"
#define RSC_FEATURES_PSN_CODE_FR	(ASCII*) "Processor Serial Number"
#define RSC_FEATURES_RTM_CODE_FR					\
				(ASCII*) "Restricted Transactional Memory"
#define RSC_FEATURES_SMX_CODE_FR	(ASCII*) "Safer Mode Extensions"
#define RSC_FEATURES_SELF_SNOOP_CODE_FR (ASCII*) "Self-Snoop"
#define RSC_FEATURES_TSC_CODE_FR	(ASCII*) "Time Stamp Counter"
#define RSC_FEATURES_TSC_DEADLN_CODE_FR (ASCII*) "Time Stamp Counter Deadline"
#define RSC_FEATURES_VME_CODE_FR	(ASCII*) "Virtual Mode Extension"
#define RSC_FEATURES_VMX_CODE_FR	(ASCII*) "Virtual Machine Extensions"
#define RSC_FEATURES_X2APIC_CODE_FR	(ASCII*) "Extended xAPIC Support"
#define RSC_FEATURES_XD_BIT_CODE_FR	(ASCII*) "Execution Disable Bit Support"
#define RSC_FEATURES_XSAVE_CODE_FR	(ASCII*) "XSAVE/XSTOR States"
#define RSC_FEATURES_XTPR_CODE_FR	(ASCII*) "xTPR Update Control"

#define RSC_TECHNOLOGIES_TITLE_CODE_FR	(ASCII*) " Technologies "
#define RSC_TECHNOLOGIES_SMM_CODE_FR	(ASCII*) "Mode de Gestion Systeme"
#define RSC_TECHNOLOGIES_HTT_CODE_FR	(ASCII*) "Hyper-Threading"
#define RSC_TECHNOLOGIES_EIST_CODE_FR	(ASCII*) "SpeedStep"
#define RSC_TECHNOLOGIES_IDA_CODE_FR	(ASCII*) "Acceleration dynamique"
#define RSC_TECHNOLOGIES_TURBO_CODE_FR	(ASCII*) "Turbo Boost"
#define RSC_TECHNOLOGIES_VM_CODE_FR	(ASCII*) "Virtualisation"
#define RSC_TECHNOLOGIES_IOMMU_CODE_FR	(ASCII*) "MMU E/S"
#define RSC_TECHNOLOGIES_SMT_CODE_FR	(ASCII*) "Multithreading simultane"
#define RSC_TECHNOLOGIES_CNQ_CODE_FR	(ASCII*) "PowerNow!"
#define RSC_TECHNOLOGIES_CPB_CODE_FR	(ASCII*) "Core Performance Boost"
#define RSC_TECHNOLOGIES_HYPERV_CODE_FR (ASCII*) "Hyperviseur"

#define RSC_PERF_MON_TITLE_CODE_FR	(ASCII*) " Gestion de la performance "
#define RSC_VERSION_CODE_FR		(ASCII*) "Version"
#define RSC_COUNTERS_CODE_FR		(ASCII*) "Compteurs"
#define RSC_GENERAL_CTRS_CODE_FR	(ASCII*) "Generaux"
#define RSC_FIXED_CTRS_CODE_FR		(ASCII*) "Fixes"
#define RSC_PERF_MON_C1E_CODE_FR	(ASCII*) "Enhanced Halt State"
#define RSC_PERF_MON_C1A_CODE_FR	(ASCII*) "C1 Auto Demotion"
#define RSC_PERF_MON_C3A_CODE_FR	(ASCII*) "C3 Auto Demotion"
#define RSC_PERF_MON_C1U_CODE_FR	(ASCII*) "C1 UnDemotion"
#define RSC_PERF_MON_C3U_CODE_FR	(ASCII*) "C3 UnDemotion"
#define RSC_PERF_MON_CC6_CODE_FR	(ASCII*) "Core C6 State"
#define RSC_PERF_MON_PC6_CODE_FR	(ASCII*) "Package C6 State"
#define RSC_PERF_MON_FID_CODE_FR	(ASCII*) "Frequency ID control"
#define RSC_PERF_MON_VID_CODE_FR	(ASCII*) "Voltage ID control"
#define RSC_PERF_MON_HWCF_CODE_FR					\
			(ASCII*) "P-State Hardware Coordination Feedback"
#define RSC_PERF_MON_HWP_CODE_FR					\
			(ASCII*) "Hardware-Controlled Performance States"
#define RSC_PERF_MON_HDC_CODE_FR	(ASCII*) "Hardware Duty Cycling"
#define RSC_PERF_MON_PKG_CSTATE_CODE_FR (ASCII*) "Package C-State"
#define RSC_PERF_MON_CFG_CTRL_CODE_FR	(ASCII*) "Configuration Control"
#define RSC_PERF_MON_LOW_CSTATE_CODE_FR (ASCII*) "Lowest C-State"
#define RSC_PERF_MON_IOMWAIT_CODE_FR	(ASCII*) "I/O MWAIT Redirection"
#define RSC_PERF_MON_MAX_CSTATE_CODE_FR (ASCII*) "Max C-State Inclusion"

#define RSC_PERF_MON_MONITOR_MWAIT_CODE_FR				\
					(ASCII*) "MONITOR/MWAIT"
#define RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_FR				\
					(ASCII*) "State index"
#define RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_FR				\
					(ASCII*) "Sub C-State"

#define RSC_PERF_MON_CORE_CYCLE_CODE_FR (ASCII*) "Core Cycles"
#define RSC_PERF_MON_INST_RET_CODE_FR	(ASCII*) "Instructions Retired"
#define RSC_PERF_MON_REF_CYCLE_CODE_FR	(ASCII*) "Reference Cycles"
#define RSC_PERF_MON_REF_LLC_CODE_FR	(ASCII*) "Last Level Cache References"
#define RSC_PERF_MON_MISS_LLC_CODE_FR	(ASCII*) "Last Level Cache Misses"
#define RSC_PERF_MON_BRANCH_RET_CODE_FR (ASCII*) "Branch Instructions Retired"
#define RSC_PERF_MON_BRANCH_MIS_CODE_FR (ASCII*) "Branch Mispredicts Retired"

#define RSC_POWER_THERMAL_TITLE_CODE_FR (ASCII*) " Puissance et thermique "
#define RSC_POWER_THERMAL_ODCM_CODE_FR	(ASCII*) "Modulation d'horloge"
#define RSC_POWER_THERMAL_DUTY_CODE_FR	(ASCII*) "Cycle de service"
#define RSC_POWER_THERMAL_MGMT_CODE_FR	(ASCII*) "Gestion de la puissance"
#define RSC_POWER_THERMAL_BIAS_CODE_FR	(ASCII*) "Regle energetique"
#define RSC_POWER_THERMAL_TJMAX_CODE_FR (ASCII*) "Temperature de jonction"
#define RSC_POWER_THERMAL_DTS_CODE_FR	(ASCII*) "Capteur thermique numerique"
#define RSC_POWER_THERMAL_PLN_CODE_FR	(ASCII*) "Notification de puissance"
#define RSC_POWER_THERMAL_PTM_CODE_FR	(ASCII*) "Gestion thermique du Package"
#define RSC_POWER_THERMAL_TM1_CODE_FR	(ASCII*) "Controle Temperature 1"
#define RSC_POWER_THERMAL_TM2_CODE_FR	(ASCII*) "Controle Temperature 2"
#define RSC_POWER_THERMAL_UNITS_CODE_FR (ASCII*) "Unites"
#define RSC_POWER_THERMAL_POWER_CODE_FR (ASCII*) "Puissance"
#define RSC_POWER_THERMAL_ENERGY_CODE_FR (ASCII*)"Energie"
#define RSC_POWER_THERMAL_WINDOW_CODE_FR (ASCII*)"Intervalle"
#define RSC_POWER_THERMAL_WATT_CODE_FR	(ASCII*) "watt"
#define RSC_POWER_THERMAL_JOULE_CODE_FR (ASCII*) "joule"
#define RSC_POWER_THERMAL_SECOND_CODE_FR (ASCII*)"seconde"

#define RSC_KERNEL_TITLE_CODE_FR	(ASCII*) " Noyau "
#define RSC_KERNEL_TOTAL_RAM_CODE_FR	(ASCII*) "RAM totale"
#define RSC_KERNEL_SHARED_RAM_CODE_FR	(ASCII*) "RAM partagee"
#define RSC_KERNEL_FREE_RAM_CODE_FR	(ASCII*) "RAM libre"
#define RSC_KERNEL_BUFFER_RAM_CODE_FR	(ASCII*) "RAM Tampon"
#define RSC_KERNEL_TOTAL_HIGH_CODE_FR	(ASCII*) "Memoire haute totale"
#define RSC_KERNEL_FREE_HIGH_CODE_FR	(ASCII*) "Memoire haute libre"
#define RSC_KERNEL_GOVERNOR_CODE_FR	(ASCII*) "Gouverneur"
#define RSC_KERNEL_FREQ_DRIVER_CODE_FR	(ASCII*) "Pilote CPU-Freq"
#define RSC_KERNEL_IDLE_DRIVER_CODE_FR	(ASCII*) "Pilote CPU-Idle"
#define RSC_KERNEL_RELEASE_CODE_FR	(ASCII*) "Edition"
#define RSC_KERNEL_VERSION_CODE_FR	(ASCII*) "Version"
#define RSC_KERNEL_MACHINE_CODE_FR	(ASCII*) "Machine"
#define RSC_KERNEL_MEMORY_CODE_FR	(ASCII*) "Memoire"
#define RSC_KERNEL_STATE_CODE_FR	(ASCII*) "Etat"
#define RSC_KERNEL_POWER_CODE_FR	(ASCII*) "Puissance"
#define RSC_KERNEL_LATENCY_CODE_FR	(ASCII*) "Latence"
#define RSC_KERNEL_RESIDENCY_CODE_FR	(ASCII*) "Periode"
#define RSC_KERNEL_LIMIT_CODE_FR	(ASCII*) "Limite Idle"

#define RSC_TOPOLOGY_TITLE_CODE_FR	(ASCII*) " Topologie "

#define RSC_MEM_CTRL_TITLE_CODE_FR	(ASCII*) " Controleur Memoire "
#define RSC_MEM_CTRL_BLANK_CODE_FR	  (ASCII*)"     "
#define RSC_MEM_CTRL_SUBSECT1_0_CODE_FR   (ASCII*)"Contr"
#define RSC_MEM_CTRL_SUBSECT1_1_CODE_FR   (ASCII*)"oleur"
#define RSC_MEM_CTRL_SUBSECT1_2_CODE_FR   (ASCII*)" #%-3u"
#define RSC_MEM_CTRL_SINGLE_CHA_0_CODE_FR (ASCII*)"Simpl"
#define RSC_MEM_CTRL_SINGLE_CHA_1_CODE_FR (ASCII*)"e Can"
#define RSC_MEM_CTRL_SINGLE_CHA_2_CODE_FR (ASCII*)"al   "
#define RSC_MEM_CTRL_DUAL_CHA_0_CODE_FR   (ASCII*)" Doub"
#define RSC_MEM_CTRL_DUAL_CHA_1_CODE_FR   (ASCII*)"le Ca"
#define RSC_MEM_CTRL_DUAL_CHA_2_CODE_FR   (ASCII*)"nal  "
#define RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_FR (ASCII*)"Tripl"
#define RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_FR (ASCII*)"e Can"
#define RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_FR (ASCII*)"al   "
#define RSC_MEM_CTRL_QUAD_CHA_0_CODE_FR   (ASCII*)" Quat"
#define RSC_MEM_CTRL_QUAD_CHA_1_CODE_FR   (ASCII*)"re Ca"
#define RSC_MEM_CTRL_QUAD_CHA_2_CODE_FR   (ASCII*)"naux "
#define RSC_MEM_CTRL_SIX_CHA_0_CODE_FR	  (ASCII*)"  Six"
#define RSC_MEM_CTRL_SIX_CHA_1_CODE_FR	  (ASCII*)" Cana"
#define RSC_MEM_CTRL_SIX_CHA_2_CODE_FR	  (ASCII*)"ux   "
#define RSC_MEM_CTRL_EIGHT_CHA_0_CODE_FR  (ASCII*)"Huit "
#define RSC_MEM_CTRL_EIGHT_CHA_1_CODE_FR  (ASCII*)"Canau"
#define RSC_MEM_CTRL_EIGHT_CHA_2_CODE_FR  (ASCII*)"x    "
#define RSC_MEM_CTRL_BUS_RATE_0_CODE_FR   (ASCII*)" Debi"
#define RSC_MEM_CTRL_BUS_RATE_1_CODE_FR   (ASCII*)"t Bus"
#define RSC_MEM_CTRL_BUS_SPEED_0_CODE_FR  (ASCII*)"Trans"
#define RSC_MEM_CTRL_BUS_SPEED_1_CODE_FR  (ASCII*)"fert "
#define RSC_MEM_CTRL_DRAM_SPEED_0_CODE_FR (ASCII*)"Freq."
#define RSC_MEM_CTRL_DRAM_SPEED_1_CODE_FR (ASCII*)" DRAM"
#define RSC_MEM_CTRL_SUBSECT2_0_CODE_FR   (ASCII*)" Geom"
#define RSC_MEM_CTRL_SUBSECT2_1_CODE_FR   (ASCII*)"etrie"
#define RSC_MEM_CTRL_SUBSECT2_2_CODE_FR   (ASCII*)" DIMM"
#define RSC_MEM_CTRL_SUBSECT2_3_CODE_FR   (ASCII*)" du c"
#define RSC_MEM_CTRL_SUBSECT2_4_CODE_FR   (ASCII*)"anal "
#define RSC_MEM_CTRL_SUBSECT2_5_CODE_FR   (ASCII*)"#%-2u  "
#define RSC_MEM_CTRL_DIMM_SLOT_CODE_FR	  (ASCII*)" Slot"
#define RSC_MEM_CTRL_DIMM_BANK_CODE_FR	  (ASCII*)" Banq"
#define RSC_MEM_CTRL_DIMM_RANK_CODE_FR	  (ASCII*)" Rang"
#define RSC_MEM_CTRL_DIMM_ROW_CODE_FR	  (ASCII*)"Ligne"
#define RSC_MEM_CTRL_DIMM_COLUMN0_CODE_FR (ASCII*)"  Col"
#define RSC_MEM_CTRL_DIMM_COLUMN1_CODE_FR (ASCII*)"onne "
#define RSC_MEM_CTRL_DIMM_SIZE_0_CODE_FR  (ASCII*)"Taill"
#define RSC_MEM_CTRL_DIMM_SIZE_1_CODE_FR  (ASCII*)"e Mem"
#define RSC_MEM_CTRL_DIMM_SIZE_2_CODE_FR  (ASCII*)"oire "
#define RSC_MEM_CTRL_DIMM_SIZE_3_CODE_FR  (ASCII*)"(MB) "

#define RSC_TASKS_SORTBY_STATE_CODE_FR	(ASCII*) " Etat     "
#define RSC_TASKS_SORTBY_RTIME_CODE_FR	(ASCII*) " RunTime  "
#define RSC_TASKS_SORTBY_UTIME_CODE_FR	(ASCII*) " UserTime "
#define RSC_TASKS_SORTBY_STIME_CODE_FR	(ASCII*) " SysTime  "
#define RSC_TASKS_SORTBY_PID_CODE_FR	(ASCII*) " PID      "
#define RSC_TASKS_SORTBY_COMM_CODE_FR	(ASCII*) " Commande "

#define RSC_MENU_ITEM_MENU_CODE_FR	(ASCII*) "          Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_FR	(ASCII*) "           Vue          "
#define RSC_MENU_ITEM_WINDOW_CODE_FR	(ASCII*) "         Fenetre        "
#define RSC_MENU_ITEM_SETTINGS_CODE_FR	(ASCII*) " Reglages           [s] "
#define RSC_MENU_ITEM_SMBIOS_CODE_FR	(ASCII*) " Infos SMBIOS       [B] "
#define RSC_MENU_ITEM_KERNEL_CODE_FR	(ASCII*) " Infos Noyau        [k] "
#define RSC_MENU_ITEM_HOTPLUG_CODE_FR	(ASCII*) " HotPlug CPU        [#] "
#define RSC_MENU_ITEM_TOOLS_CODE_FR	(ASCII*) " Outils            [F3] "
#define RSC_MENU_ITEM_ABOUT_CODE_FR	(ASCII*) " Apropos            [a] "
#define RSC_MENU_ITEM_HELP_CODE_FR	(ASCII*) " Aide               [h] "
#define RSC_MENU_ITEM_KEYS_CODE_FR	(ASCII*) " Raccourcis        [F1] "
#define RSC_MENU_ITEM_LANG_CODE_FR	(ASCII*) " Langues            [L] "
#define RSC_MENU_ITEM_QUIT_CODE_FR	(ASCII*) " Quitter           [F4] "
#define RSC_MENU_ITEM_DASHBOARD_CODE_FR (ASCII*) " Tableau de bord    [d] "
#define RSC_MENU_ITEM_FREQUENCY_CODE_FR (ASCII*) " Frequence          [f] "
#define RSC_MENU_ITEM_INST_CYCLE_CODE_FR (ASCII*)" Cycles Instruction [i] "
#define RSC_MENU_ITEM_CORE_CYCLE_CODE_FR (ASCII*)" Cycles des Coeurs  [c] "
#define RSC_MENU_ITEM_IDLE_STATE_CODE_FR (ASCII*)" Etats de sommeil   [l] "
#define RSC_MENU_ITEM_PKG_CYCLE_CODE_FR (ASCII*) " Cycles du Package  [g] "
#define RSC_MENU_ITEM_TASKS_MON_CODE_FR (ASCII*) " Suivi des taches   [x] "
#define RSC_MENU_ITEM_SYS_INTER_CODE_FR (ASCII*) " Interruptions      [q] "
#define RSC_MENU_ITEM_POW_VOLT_CODE_FR	(ASCII*) " Puissance-Voltage  [V] "
#define RSC_MENU_ITEM_SLICE_CTR_CODE_FR (ASCII*) " Compteurs tranche  [T] "
#define RSC_MENU_ITEM_PROCESSOR_CODE_FR (ASCII*) " Processeur         [p] "
#define RSC_MENU_ITEM_TOPOLOGY_CODE_FR	(ASCII*) " Topologie          [m] "
#define RSC_MENU_ITEM_FEATURES_CODE_FR	(ASCII*) " Caracteristiques   [e] "
#define RSC_MENU_ITEM_ISA_EXT_CODE_FR	(ASCII*) " Jeu Instructions   [I] "
#define RSC_MENU_ITEM_TECH_CODE_FR	(ASCII*) " Technologies       [t] "
#define RSC_MENU_ITEM_PERF_MON_CODE_FR	(ASCII*) " Gestion de Perf.   [o] "
#define RSC_MENU_ITEM_POW_THERM_CODE_FR (ASCII*) " Puissance-Therm.   [w] "
#define RSC_MENU_ITEM_CPUID_CODE_FR	(ASCII*) " Extraction CPUID   [u] "
#define RSC_MENU_ITEM_SYS_REGS_CODE_FR	(ASCII*) " Registres Systeme  [R] "
#define RSC_MENU_ITEM_MEM_CTRL_CODE_FR	(ASCII*) " Controleur Memoire [M] "

#define RSC_SETTINGS_TITLE_CODE_FR	(ASCII*) " Reglages "
#define RSC_SETTINGS_DAEMON_CODE_FR					\
				(ASCII*) " Acces demon                    "
#define RSC_SETTINGS_INTERVAL_CODE_FR					\
				(ASCII*) " Intervalle(ms)          <    > "
#define RSC_SETTINGS_RECORDER_CODE_FR					\
				(ASCII*) " Enregistreur(sec)       <    > "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_FR 				\
				(ASCII*) " Auto Clock               <   > "
#define RSC_SETTINGS_EXPERIMENTAL_CODE_FR				\
				(ASCII*) " Experimental             <   > "
#define RSC_SETTINGS_CPU_HOTPLUG_CODE_FR				\
				(ASCII*) " Hot-Plug CPU             [   ] "
#define RSC_SETTINGS_PCI_ENABLED_CODE_FR				\
				(ASCII*) " Activation PCI           [   ] "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_FR				\
				(ASCII*) " Activation NMI           <   > "
#define RSC_SETTINGS_CPUIDLE_REGISTERED_CODE_FR 			\
				(ASCII*) " Pilote CPU-IDLE          [   ] "
#define RSC_SETTINGS_CPUFREQ_REGISTERED_CODE_FR 			\
				(ASCII*) " Pilote CPU-FREQ          [   ] "

#define RSC_HELP_TITLE_CODE_FR		(ASCII*) " Aide "
#define RSC_HELP_KEY_ESCAPE_CODE_FR	(ASCII*) " [Echap]          "
#define RSC_HELP_KEY_SHIFT_TAB_CODE_FR	(ASCII*) " [Maj]+[Tab]      "
#define RSC_HELP_KEY_TAB_CODE_FR	(ASCII*) " [Tab]            "
#define RSC_HELP_KEY_UP_CODE_FR 	(ASCII*) "      [Haut]      "
#define RSC_HELP_KEY_LEFT_RIGHT_CODE_FR (ASCII*) " [Gauche] [Droite]"
#define RSC_HELP_KEY_DOWN_CODE_FR	(ASCII*) "      [Bas]       "
#define RSC_HELP_KEY_END_CODE_FR	(ASCII*) " [Fin]            "
#define RSC_HELP_KEY_HOME_CODE_FR	(ASCII*) " [Debut]          "
#define RSC_HELP_KEY_ENTER_CODE_FR	(ASCII*) " [Entree]         "
#define RSC_HELP_KEY_PAGE_UP_CODE_FR	(ASCII*) " [Page-Prec]      "
#define RSC_HELP_KEY_PAGE_DOWN_CODE_FR	(ASCII*) " [Page-Suiv]      "
#define RSC_HELP_KEY_MINUS_CODE_FR	(ASCII*) " [Moins]          "
#define RSC_HELP_KEY_PLUS_CODE_FR	(ASCII*) " [Plus]           "
#define RSC_HELP_MENU_CODE_FR		(ASCII*) "             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_FR	(ASCII*) "   Fermer fenetre "
#define RSC_HELP_PREV_WINDOW_CODE_FR	(ASCII*) "  Fenetre arriere "
#define RSC_HELP_NEXT_WINDOW_CODE_FR	(ASCII*) " Fenetre suivante "
#define RSC_HELP_MOVE_WINDOW_CODE_FR	(ASCII*) " Deplacer fenetre "
#define RSC_HELP_MOVE_SELECT_CODE_FR	(ASCII*) " Deplacer curseur "
#define RSC_HELP_LAST_CELL_CODE_FR	(ASCII*) " Derniere cellule "
#define RSC_HELP_FIRST_CELL_CODE_FR	(ASCII*) " Premiere cellule "
#define RSC_HELP_TRIGGER_SELECT_CODE_FR (ASCII*) " Executer cellule "
#define RSC_HELP_PREV_PAGE_CODE_FR	(ASCII*) "  Page precedente "
#define RSC_HELP_NEXT_PAGE_CODE_FR	(ASCII*) "    Page suivante "
#define RSC_HELP_SCROLL_DOWN_CODE_FR	(ASCII*) "    Decroitre CPU "
#define RSC_HELP_SCROLL_UP_CODE_FR	(ASCII*) "    Accroitre CPU "

#define RSC_ADV_HELP_TITLE_CODE_FR	(ASCII*) " Raccourcis "
#define RSC_ADV_HELP_ITEM_1_CODE_FR					\
			(ASCII*) " Vue Frequence:                       "
#define RSC_ADV_HELP_ITEM_2_CODE_FR					\
			(ASCII*) " %           Moyennes | Etats-Package "
#define RSC_ADV_HELP_ITEM_3_CODE_FR					\
			(ASCII*) " Vue Suivi des taches:                "
#define RSC_ADV_HELP_ITEM_4_CODE_FR					\
			(ASCII*) " b          Critere de tri des taches "
#define RSC_ADV_HELP_ITEM_5_CODE_FR					\
			(ASCII*) " n          Choisir la tache a suivre "
#define RSC_ADV_HELP_ITEM_6_CODE_FR					\
			(ASCII*) " r         Inverser le tri des taches "
#define RSC_ADV_HELP_ITEM_7_CODE_FR					\
			(ASCII*) " v          Afficher | Cacher valeurs "
#define RSC_ADV_HELP_ITEM_8_CODE_FR					\
			(ASCII*) " Vue quelconque:                      "
#define RSC_ADV_HELP_ITEM_9_CODE_FR					\
			(ASCII*) " .             Frequence top ou Usage "
#define RSC_ADV_HELP_ITEM_10_CODE_FR					\
			(ASCII*) " {                  Demarrer CoreFreq "
#define RSC_ADV_HELP_ITEM_11_CODE_FR					\
			(ASCII*) " }                   Arreter CoreFreq "
#define RSC_ADV_HELP_ITEM_12_CODE_FR					\
			(ASCII*) " F10               Arreter les outils "
#define RSC_ADV_HELP_ITEM_13_CODE_FR					\
			(ASCII*) " Haut Prec                 Defilement "
#define RSC_ADV_HELP_ITEM_14_CODE_FR					\
			(ASCII*) " Bas  Suiv                       CPU  "
#define RSC_ADV_HELP_ITEM_TERMINAL_CODE_FR				\
			(ASCII*) " Terminal:                            "
#define RSC_ADV_HELP_ITEM_PRT_SCR_CODE_FR				\
			(ASCII*) " [Ctrl]+[p]                    Copier "
#define RSC_ADV_HELP_ITEM_REC_SCR_CODE_FR				\
			(ASCII*) " [Alt]+[p]                Enregistrer "
#define RSC_ADV_HELP_ITEM_FAHR_CELS_CODE_FR				\
			(ASCII*) " F              Fahrenheit ou Celsius "
#define RSC_ADV_HELP_ITEM_PROC_EVENT_CODE_FR				\
			(ASCII*) " H           Gerer Alertes Processeur "

#define RSC_TURBO_CLOCK_TITLE_CODE_FR	(ASCII*) " Freq. Turbo %1dC "
#define RSC_RATIO_CLOCK_TITLE_CODE_FR	(ASCII*) " %s Ratio Freq. "
#define RSC_UNCORE_CLOCK_TITLE_CODE_FR	(ASCII*) " %s Uncore Freq. "

#define RSC_SELECT_CPU_TITLE_CODE_FR	(ASCII*) " CPU   Pkg  Core Thread "

#define RSC_BOX_DISABLE_COND0_CODE_FR					\
			(ASCII*) "             Desactiver             "
#define RSC_BOX_DISABLE_COND1_CODE_FR					\
			(ASCII*) "           < Desactiver >           "
#define RSC_BOX_ENABLE_COND0_CODE_FR					\
			(ASCII*) "               Activer              "
#define RSC_BOX_ENABLE_COND1_CODE_FR					\
			(ASCII*) "           <   Activer  >           "

#define RSC_BOX_INTERVAL_TITLE_CODE_FR	(ASCII*) "Intervalle"
#define RSC_BOX_AUTOCLOCK_TITLE_CODE_FR (ASCII*) " Auto Clock "
#define RSC_BOX_MODE_TITLE_CODE_FR	(ASCII*) " Experimental "

#define RSC_BOX_MODE_DESC_CODE_FR					\
			(ASCII*) "     CoreFreq mode operationnel     "
#define RSC_BOX_EIST_DESC_CODE_FR					\
			(ASCII*) "             SpeedStep              "
#define RSC_BOX_C1E_DESC_CODE_FR					\
			(ASCII*) "        Etat de Pause etendu        "
#define RSC_BOX_TURBO_DESC_CODE_FR					\
			(ASCII*) " Turbo Boost/Core Performance Boost "
#define RSC_BOX_C1A_DESC_CODE_FR					\
			(ASCII*) "       Auto retrogradation C1       "
#define RSC_BOX_C3A_DESC_CODE_FR					\
			(ASCII*) "       Auto retrogradation C3       "
#define RSC_BOX_C1U_DESC_CODE_FR					\
			(ASCII*) "        Non-retrogradation C1       "
#define RSC_BOX_C3U_DESC_CODE_FR					\
			(ASCII*) "        Non-retrogradation C3       "
#define RSC_BOX_CC6_DESC_CODE_FR					\
			(ASCII*) "            Core Etat C6            "
#define RSC_BOX_PC6_DESC_CODE_FR					\
			(ASCII*) "           Package Etat C6          "
#define RSC_BOX_HWP_DESC_CODE_FR					\
			(ASCII*) " Controle Materiel de la Performance"
#define RSC_BOX_BLANK_DESC_CODE_FR					\
			(ASCII*) "                                    "

#define RSC_BOX_NOMINAL_MODE_COND0_CODE_FR				\
			(ASCII*) "       Fonctionnement nominal       "
#define RSC_BOX_NOMINAL_MODE_COND1_CODE_FR				\
			(ASCII*) "     < Fonctionnement nominal >     "
#define RSC_BOX_EXPERIMENT_MODE_COND0_CODE_FR				\
			(ASCII*) "     Fonctionnement experimental    "
#define RSC_BOX_EXPERIMENT_MODE_COND1_CODE_FR				\
			(ASCII*) "   < Fonctionnement experimental >  "

#define RSC_BOX_INTERRUPT_TITLE_CODE_FR (ASCII*) " Interruptions NMI "

#define RSC_BOX_INT_REGISTER_COND0_CODE_FR				\
			(ASCII*) "             Enregistrer            "
#define RSC_BOX_INT_REGISTER_COND1_CODE_FR				\
			(ASCII*) "         <   Enregistrer  >         "
#define RSC_BOX_INT_UNREGISTER_COND0_CODE_FR				\
			(ASCII*) "           Desenregistrer           "
#define RSC_BOX_INT_UNREGISTER_COND1_CODE_FR				\
			(ASCII*) "         < Desenregistrer >         "

#define RSC_BOX_EVENT_TITLE_CODE_FR	(ASCII*) " Effacer Evenement "

#define RSC_BOX_EVENT_THERMAL_SENSOR_CODE_FR				\
					(ASCII*) "    Capteur thermique    "
#define RSC_BOX_EVENT_PROCHOT_AGENT_CODE_FR				\
					(ASCII*) "      Agent PROCHOT#     "
#define RSC_BOX_EVENT_CRITICAL_TEMP_CODE_FR				\
					(ASCII*) "   Temperature critique  "
#define RSC_BOX_EVENT_THERMAL_THRESHOLD_CODE_FR 			\
					(ASCII*) "     Seuil thermique     "
#define RSC_BOX_EVENT_POWER_LIMITATION_CODE_FR				\
					(ASCII*) " Limitation de puissance "
#define RSC_BOX_EVENT_CURRENT_LIMITATION_CODE_FR			\
					(ASCII*) "  Limitation de courant  "
#define RSC_BOX_EVENT_CROSS_DOMAIN_LIMIT_CODE_FR			\
					(ASCII*) " Limitation interdomaine "

#define RSC_BOX_PKG_STATE_TITLE_CODE_FR (ASCII*) " Limite Etats Package "

#define RSC_BOX_IO_MWAIT_TITLE_CODE_FR	(ASCII*) " E/S MWAIT "
#define RSC_BOX_IO_MWAIT_DESC_CODE_FR					\
			(ASCII*) "        Redirection E/S MWAIT       "

#define RSC_BOX_MWAIT_MAX_STATE_TITLE_CODE_FR (ASCII*) " E/S MWAIT Etat Max "

#define RSC_BOX_ODCM_TITLE_CODE_FR	(ASCII*) " ODCM "
#define RSC_BOX_ODCM_DESC_CODE_FR					\
			(ASCII*) "        Modulation d'horloge        "

#define RSC_BOX_EXT_DUTY_CYCLE_TITLE_CODE_FR				\
					(ASCII*)" Cycle de service etendu "
#define RSC_BOX_DUTY_CYCLE_TITLE_CODE_FR	(ASCII*) " Cycle de service "

#define RSC_BOX_DUTY_CYCLE_RESERVED_CODE_FR				\
				(ASCII*) "           Reserve         "

#define RSC_BOX_POWER_POLICY_TITLE_CODE_FR	(ASCII*) " Regle energetique "

#define RSC_BOX_POWER_POLICY_LOW_CODE_FR  (ASCII*) "            0       BAS "
#define RSC_BOX_POWER_POLICY_HIGH_CODE_FR (ASCII*) "           15      HAUT "

#define RSC_BOX_HWP_POLICY_MIN_CODE_FR	(ASCII*) "        Minimale        "
#define RSC_BOX_HWP_POLICY_MED_CODE_FR	(ASCII*) "        Moyenne         "
#define RSC_BOX_HWP_POLICY_PWR_CODE_FR	(ASCII*) "        Puissance       "
#define RSC_BOX_HWP_POLICY_MAX_CODE_FR	(ASCII*) "        Maximale        "

#define RSC_BOX_TOOLS_TITLE_CODE_FR	(ASCII*) " Outils "
#define RSC_BOX_TOOLS_STOP_CODE_FR	(ASCII*) "          ARRETER          "
#define RSC_BOX_TOOLS_ATOMIC_CODE_FR	(ASCII*) "       Stress Atomic       "
#define RSC_BOX_TOOLS_CRC32_CODE_FR	(ASCII*) "        Calcul CRC32       "
#define RSC_BOX_TOOLS_CONIC_CODE_FR	(ASCII*) "      Calcul Conique...    "
#define RSC_BOX_TOOLS_RAND_CPU_CODE_FR	(ASCII*) "    Turbo CPU aleatoire    "
#define RSC_BOX_TOOLS_RR_CPU_CODE_FR	(ASCII*) "    Turbo CPU circulaire   "
#define RSC_BOX_TOOLS_USR_CPU_CODE_FR	(ASCII*) "    Turbo Choisir CPU...   "

#define RSC_BOX_CONIC_TITLE_CODE_FR	(ASCII*) " Variations Coniques "
#define RSC_BOX_CONIC_ITEM_1_CODE_FR	(ASCII*) "         Ellipsoide         "
#define RSC_BOX_CONIC_ITEM_2_CODE_FR	(ASCII*) "  Hyperboloide a une nappe  "
#define RSC_BOX_CONIC_ITEM_3_CODE_FR	(ASCII*) " Hyperboloide a deux nappes "
#define RSC_BOX_CONIC_ITEM_4_CODE_FR	(ASCII*) "     Cylindre elliptique    "
#define RSC_BOX_CONIC_ITEM_5_CODE_FR	(ASCII*) "     Cylindre hyperbolique  "
#define RSC_BOX_CONIC_ITEM_6_CODE_FR	(ASCII*) "    Deux plans paralleles   "

#define RSC_BOX_LANG_TITLE_CODE_FR	(ASCII*) " Langues "
#define RSC_BOX_LANG_ENGLISH_CODE_FR	(ASCII*) "     Anglais     "
#define RSC_BOX_LANG_FRENCH_CODE_FR	(ASCII*) "     Francais    "

#define RSC_ERROR_CMD_SYNTAX_CODE_FR					\
	(ASCII*)"CoreFreq."						\
		"  Copyright (C) 2015-2019 CYRIL INGENIERIE\n\n"	\
		"Usage:\t%s [-option <arguments>]\n"			\
		"\t-0,1,2\tUnité mémoire en K,M,G octet\n"		\
		"\t-F\tTemperature en Fahrenheit\n"			\
		"\t-J <#>\tNuméro d'index de chaîne SMBIOS\n"		\
		"\t-t\tAfficher Top (par défault)\n"			\
		"\t-d\tAfficher le tableau de bord\n"			\
		"\t-V\tMoniteur de Puissance et Voltage\n"		\
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
	(ASCII*) "Erreur code %d de connexion au démon.\n%s: '%s' @ ligne %d\n"

#define RSC_ERROR_SYS_CALL_CODE_FR					\
			(ASCII*) "Erreur Système code %d\n%s @ ligne %d\n"

#define RSC_BOX_IDLE_LIMIT_TITLE_CODE_FR (ASCII*) " Limite CPU-Idle "

#define RSC_BOX_RECORDER_TITLE_CODE_FR (ASCII*) " Duree "

#define RSC_SMBIOS_TITLE_CODE_FR	(ASCII*) " SMBIOS "
