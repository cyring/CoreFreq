/*
 * CoreFreq
 * Copyright (C) 2015-2025 CYRIL COURTIAT
 * Licenses: GPL2
 */

/* Shell tip to list French Unicode accents
 *
for h in 8 9 a b; do for l in 0 1 2 3 4 5 6 7 8 9 a b c d e f; \
do echo -en "$h$l\t""\xc3""\x$h$l""\t"; done; done;echo
*
*/

#define RSC_COPY0_CODE_FR "     par CyrIng                                     "
#define RSC_COPY1_CODE_FR "                                                    "
#define RSC_COPY2_CODE_FR "            (C)2015-2025 "			\
			  "CYRIL COURTIAT             "

#define RSC_LAYOUT_HEADER_PROC_CODE_FR					\
{									\
	' ','P','r','o','c','e','s','s','e','u','r','[' 		\
}

#define RSC_LAYOUT_HEADER_CPU_CODE_FR	RSC_LAYOUT_HEADER_CPU_CODE_EN

#define RSC_LAYOUT_HEADER_ARCH_CODE_FR	RSC_LAYOUT_HEADER_ARCH_CODE_EN

#define RSC_LAYOUT_HEADER_CACHE_L1_CODE_FR RSC_LAYOUT_HEADER_CACHE_L1_CODE_EN

#define RSC_LAYOUT_HEADER_BCLK_CODE_FR					\
{									\
	' ','H','o','r','l','o','g','e',' ',' ',' ',' ',		\
	'~',' ','0','0','0',' ','0','0','0',' ','0','0','0',' ','H','z' \
}

#define RSC_LAYOUT_HEADER_CACHES_CODE_FR RSC_LAYOUT_HEADER_CACHES_CODE_EN

#define RSC_LAYOUT_RULER_LOAD_CODE_FR	RSC_LAYOUT_RULER_LOAD_CODE_EN

#define RSC_LAYOUT_RULER_REL_LOAD_CODE_FR				\
{									\
	'F','r',0xa9,'q','u','e','n','c','e',' ','r','e','l','a','t','i',\
	'v','e' 							\
}

#define RSC_LAYOUT_RULER_ABS_LOAD_CODE_FR				\
{									\
	'F','r',0xa9,'q','u','e','n','c','e',' ','a','b','s','o','l','u',\
	'e',' ' 							\
}

#define RSC_LAYOUT_MONITOR_FREQUENCY_CODE_FR	\
					RSC_LAYOUT_MONITOR_FREQUENCY_CODE_EN

#define RSC_LAYOUT_MONITOR_INST_CODE_FR RSC_LAYOUT_MONITOR_INST_CODE_EN

#define RSC_LAYOUT_MONITOR_COMMON_CODE_FR RSC_LAYOUT_MONITOR_COMMON_CODE_EN

#define RSC_LAYOUT_MONITOR_TASKS_CODE_FR RSC_LAYOUT_MONITOR_TASKS_CODE_EN

#define RSC_LAYOUT_MONITOR_SLICE_CODE_FR RSC_LAYOUT_MONITOR_SLICE_CODE_EN

#define RSC_LAYOUT_CUSTOM_FIELD_CODE_FR RSC_LAYOUT_CUSTOM_FIELD_CODE_EN

#define RSC_LAYOUT_RULER_FREQUENCY_CODE_FR				\
{									\
	'-','-','-',' ','F','r',0xa9,'q','(','M','H','z',')',' ','R','a',\
	't','i','o',' ','-',' ','T','u','r','b','o',' ','-','-',' ','C',\
	'0',' ','-','-','-','-',' ','C','1',' ','-','-',' ','C','2',':',\
	'C','3',' ','-',' ','C','4',':','C','6',' ','-','-','-',' ','C',\
	'7',' ','-','-',' ','M','i','n',' ','T','M','P',' ','M','a','x',\
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

#define RSC_LAYOUT_RULER_FREQUENCY_PKG_CODE_FR	\
					RSC_LAYOUT_RULER_FREQUENCY_PKG_CODE_EN

#define RSC_LAYOUT_RULER_INST_CODE_FR	RSC_LAYOUT_RULER_INST_CODE_EN

#define RSC_LAYOUT_RULER_CYCLES_CODE_FR RSC_LAYOUT_RULER_CYCLES_CODE_EN

#define RSC_LAYOUT_RULER_CSTATES_CODE_FR RSC_LAYOUT_RULER_CSTATES_CODE_EN

#define RSC_LAYOUT_RULER_INTERRUPTS_CODE_FR	\
					RSC_LAYOUT_RULER_INTERRUPTS_CODE_EN

#define RSC_LAYOUT_RULER_PACKAGE_CODE_FR				\
	"------------ Cycles ----  ""\x89""tat ------------------ Ratio Horl" \
	"oge --------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_TASKS_CODE_FR					\
	"--- Fr""\xa9""q(MHz) ---T""\xa2""ches" 			\
	"                    -----------------" 			\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_PACKAGE_PC_CODE_FR	RSC_LAYOUT_PACKAGE_PC_CODE_EN

#define RSC_LAYOUT_PACKAGE_PC02_CODE_FR RSC_LAYOUT_PACKAGE_PC02_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC03_CODE_FR RSC_LAYOUT_PACKAGE_PC03_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC04_CODE_FR RSC_LAYOUT_PACKAGE_PC04_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC06_CODE_FR RSC_LAYOUT_PACKAGE_PC06_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC07_CODE_FR RSC_LAYOUT_PACKAGE_PC07_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC08_CODE_FR RSC_LAYOUT_PACKAGE_PC08_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC09_CODE_FR RSC_LAYOUT_PACKAGE_PC09_CODE_EN
#define RSC_LAYOUT_PACKAGE_PC10_CODE_FR RSC_LAYOUT_PACKAGE_PC10_CODE_EN
#define RSC_LAYOUT_PACKAGE_MC06_CODE_FR RSC_LAYOUT_PACKAGE_MC06_CODE_EN

#define RSC_LAYOUT_PACKAGE_UNCORE_CODE_FR RSC_LAYOUT_PACKAGE_UNCORE_CODE_EN

#define RSC_LAYOUT_TASKS_STATE_SORTED_CODE_FR				\
{									\
	'(','t','r','i','e','r',' ','[', 'b',']',			\
	' ',0x89,'t','a','t',' ',')',' ', '-','-','-'			\
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

#define RSC_LAYOUT_TASKS_VALUE_OFF_CODE_FR RSC_LAYOUT_TASKS_VALUE_OFF_CODE_EN

#define RSC_LAYOUT_TASKS_VALUE_ON_CODE_FR RSC_LAYOUT_TASKS_VALUE_ON_CODE_EN

#define RSC_LAYOUT_TASKS_TRACKING_CODE_FR				\
{									\
	'S','u','i','v','i',' ','[', 'n',']',' ','P','I','D',' ','[',' ',\
	' ','O','F','F',' ',' ',']',' ' 				\
}

#define RSC_LAYOUT_RULER_SENSORS_CODE_FR				\
	"--- Fr""\xa9""q(MHz) --- Vcore --- TMP( ) -- " 		\
	"\x89""nergie(J) Puissance(W"					\
	") ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_PWR_SOC_CODE_FR RSC_LAYOUT_RULER_PWR_SOC_CODE_EN

#define RSC_LAYOUT_RULER_VOLTAGE_CODE_FR				\
	"--- Fr""\xa9""q(MHz) - VID --- Min(V)-- Vcore -- Max(V) -----------" \
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_VPKG_SOC_CODE_FR				\
	"-Processeur[                                    ] ----- SoC "	\
	"[       ] [      V]-----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_ENERGY_CODE_FR 				\
	"--- Fr" "\xa9" "q(MHz) -- Accumulateur ------- Min ----- "	\
	"\x89" "nergie(J) -- Max ------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------------------------"

#define RSC_LAYOUT_RULER_POWER_CODE_FR					\
	"--- Fr" "\xa9" "q(MHz) -- Accumulateur ------- Min --- "	\
	"Puissance(W) -- Max ----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"----------------------------------"

#define RSC_LAYOUT_RULER_SLICE_CODE_FR					\
	"--- Fr""\xa9""q(MHz) ------ Cycles -- Instructions ------------ TSC" \
	" ------------ PMC0 ---------- Erreur -----------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_CUSTOM_CODE_FR RSC_LAYOUT_RULER_CUSTOM_CODE_EN

#define RSC_LAYOUT_FOOTER_TECH_ARM_CODE_FR RSC_LAYOUT_FOOTER_TECH_ARM_CODE_EN

#define RSC_LAYOUT_FOOTER_VOLT_TEMP_CODE_FR RSC_LAYOUT_FOOTER_VOLT_TEMP_CODE_EN

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_FR				\
{									\
	'T',0xa2,'c','h','e','s','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M',0xa9,'m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','B',']' 	\
}

#define RSC_LAYOUT_CARD_CORE_ONLINE_COND0_CODE_FR	\
				RSC_LAYOUT_CARD_CORE_ONLINE_COND0_CODE_EN

#define RSC_LAYOUT_CARD_CORE_ONLINE_COND1_CODE_FR	\
				RSC_LAYOUT_CARD_CORE_ONLINE_COND1_CODE_EN

#define RSC_LAYOUT_CARD_CORE_OFFLINE_CODE_FR \
					RSC_LAYOUT_CARD_CORE_OFFLINE_CODE_EN

#define RSC_LAYOUT_CARD_CLK_CODE_FR	RSC_LAYOUT_CARD_CLK_CODE_EN

#define RSC_LAYOUT_CARD_UNCORE_CODE_FR	RSC_LAYOUT_CARD_UNCORE_CODE_EN

#define RSC_LAYOUT_CARD_BUS_CODE_FR	RSC_LAYOUT_CARD_BUS_CODE_EN

#define RSC_LAYOUT_CARD_MC_CODE_FR	RSC_LAYOUT_CARD_MC_CODE_EN

#define RSC_LAYOUT_CARD_LOAD_CODE_FR					\
{									\
	'[',' ',' ','%','A','C','T','I','F',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_IDLE_CODE_FR					\
{									\
	'[',' ',' ','%','R','E','P','O','S',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_RAM_CODE_FR	RSC_LAYOUT_CARD_RAM_CODE_EN

#define RSC_LAYOUT_CARD_TASK_CODE_FR					\
{									\
	'[','T',0xa2,'c','h','e',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_FR	"<  ACTIVER >"
#define RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_FR	"<D""\x89""SACTIVER>"
#define RSC_CREATE_HOTPLUG_CPU_ONLINE_CODE_FR	" %03u   On   "
#define RSC_CREATE_HOTPLUG_CPU_OFFLINE_CODE_FR	" %03u  Off   "

#define RSC_PROCESSOR_TITLE_CODE_FR	" Processeur "
#define RSC_PROCESSOR_CODE_FR		"Processeur"
#define RSC_ARCHITECTURE_CODE_FR	"Architecture"
#define RSC_VENDOR_ID_CODE_FR		"ID vendeur"
#define RSC_REVISION_CODE_FR		"R""\xa9""vision"
#define RSC_SIGNATURE_CODE_FR		"Signature"
#define RSC_STEPPING_CODE_FR		"Stepping"
#define RSC_ONLINE_CPU_CODE_FR		"CPU en ligne"
#define RSC_BASE_CLOCK_CODE_FR		"Horloge de base"
#define RSC_FREQUENCY_CODE_FR		"Fr""\xa9""quence"
#define RSC_RATIO_CODE_FR		"Ratio"
#define RSC_FACTORY_CODE_FR		"Usine"
#define RSC_PERFORMANCE_CODE_FR 	"Performance"
#define RSC_TARGET_CODE_FR		"Cible"
#define RSC_UNLOCK_CODE_FR		"OUVERT"
#define RSC_LOCK_CODE_FR		"BLOQU""\x89"
#define RSC_ENABLE_CODE_FR		"  Actif"
#define RSC_DISABLE_CODE_FR		"Inactif"
#define RSC_CAPABILITIES_CODE_FR	"Capacit""\xa9""s"
#define RSC_LOWEST_CODE_FR		"Faible"
#define RSC_EFFICIENT_CODE_FR		"Efficace"
#define RSC_GUARANTEED_CODE_FR		"Guarantie"
#define RSC_HIGHEST_CODE_FR		"Elev""\xa9""e"
#define RSC_RECORDER_CODE_FR		"Enregistreur"
#define RSC_STRESS_CODE_FR		"Charge"

#define RSC_SCOPE_NONE_CODE_FR		"Sans"
#define RSC_SCOPE_THREAD_CODE_FR	" SMT"
#define RSC_SCOPE_CORE_CODE_FR		"Core"
#define RSC_SCOPE_PACKAGE_CODE_FR	" Pkg"

#define RSC_SYS_REGS_TITLE_CODE_FR	" Registres Syst""\xa8""me "
#define RSC_SYS_REG_PSTATE_CODE_FR	RSC_SYS_REG_PSTATE_CODE_EN
#define RSC_SYS_REG_FLAG_N_CODE_FR	RSC_SYS_REG_FLAG_N_CODE_EN
#define RSC_SYS_REG_FLAG_Z_CODE_FR	RSC_SYS_REG_FLAG_Z_CODE_EN
#define RSC_SYS_REG_FLAG_C_CODE_FR	RSC_SYS_REG_FLAG_C_CODE_EN
#define RSC_SYS_REG_FLAG_V_CODE_FR	RSC_SYS_REG_FLAG_V_CODE_EN
#define RSC_SYS_REG_FLAG_D_CODE_FR	RSC_SYS_REG_FLAG_D_CODE_EN
#define RSC_SYS_REG_FLAG_A_CODE_FR	RSC_SYS_REG_FLAG_A_CODE_EN
#define RSC_SYS_REG_FLAG_I_CODE_FR	RSC_SYS_REG_FLAG_I_CODE_EN
#define RSC_SYS_REG_FLAG_F_CODE_FR	RSC_SYS_REG_FLAG_F_CODE_EN
#define RSC_SYS_REG_FLAG_EL_CODE_FR	RSC_SYS_REG_FLAG_EL_CODE_EN
#define RSC_SYS_REG_FLAG_SM_CODE_FR	RSC_SYS_REG_FLAG_SM_CODE_EN
#define RSC_SYS_REG_FLAG_SSBS_CODE_FR	RSC_SYS_REG_FLAG_SSBS_CODE_EN
#define RSC_SYS_REG_FLAG_NMI_CODE_FR	RSC_SYS_REG_FLAG_NMI_CODE_EN
#define RSC_SYS_REG_FLAG_PAN_CODE_FR	RSC_SYS_REG_FLAG_PAN_CODE_EN
#define RSC_SYS_REG_FLAG_UAO_CODE_FR	RSC_SYS_REG_FLAG_UAO_CODE_EN
#define RSC_SYS_REG_FLAG_DIT_CODE_FR	RSC_SYS_REG_FLAG_DIT_CODE_EN
#define RSC_SYS_REG_FLAG_TCO_CODE_FR	RSC_SYS_REG_FLAG_TCO_CODE_EN
#define RSC_SYS_REG_FLAG_PM_CODE_FR	RSC_SYS_REG_FLAG_PM_CODE_EN

#define RSC_SYS_REG_HCR_CODE_FR 	RSC_SYS_REG_HCR_CODE_EN
#define RSC_SYS_REG_TWEDEL_CODE_FR	RSC_SYS_REG_TWEDEL_CODE_EN
#define RSC_SYS_REG_TWEDEN_CODE_FR	RSC_SYS_REG_TWEDEN_CODE_EN
#define RSC_SYS_REG_TID5_CODE_FR	RSC_SYS_REG_TID5_CODE_EN
#define RSC_SYS_REG_DCT_CODE_FR 	RSC_SYS_REG_DCT_CODE_EN
#define RSC_SYS_REG_TTLBOS_CODE_FR	RSC_SYS_REG_TTLBOS_CODE_EN
#define RSC_SYS_REG_TTLBIS_CODE_FR	RSC_SYS_REG_TTLBIS_CODE_EN
#define RSC_SYS_REG_ENSCXT_CODE_FR	RSC_SYS_REG_ENSCXT_CODE_EN
#define RSC_SYS_REG_TOCU_CODE_FR	RSC_SYS_REG_TOCU_CODE_EN
#define RSC_SYS_REG_AMVOFF_CODE_FR	RSC_SYS_REG_AMVOFF_CODE_EN
#define RSC_SYS_REG_TICAB_CODE_FR	RSC_SYS_REG_TICAB_CODE_EN
#define RSC_SYS_REG_TID4_CODE_FR	RSC_SYS_REG_TID4_CODE_EN
#define RSC_SYS_REG_GPF_CODE_FR 	RSC_SYS_REG_GPF_CODE_EN
#define RSC_SYS_REG_FIEN_CODE_FR	RSC_SYS_REG_FIEN_CODE_EN
#define RSC_SYS_REG_FWB_CODE_FR 	RSC_SYS_REG_FWB_CODE_EN
#define RSC_SYS_REG_NV2_CODE_FR 	RSC_SYS_REG_NV2_CODE_EN
#define RSC_SYS_REG_AT_CODE_FR		RSC_SYS_REG_AT_CODE_EN
#define RSC_SYS_REG_NV1_CODE_FR 	RSC_SYS_REG_NV1_CODE_EN
#define RSC_SYS_REG_NV_CODE_FR		RSC_SYS_REG_NV_CODE_EN
#define RSC_SYS_REG_API_CODE_FR 	RSC_SYS_REG_API_CODE_EN
#define RSC_SYS_REG_APK_CODE_FR 	RSC_SYS_REG_APK_CODE_EN
#define RSC_SYS_REG_MIOCNC_CODE_FR	RSC_SYS_REG_MIOCNC_CODE_EN
#define RSC_SYS_REG_TEA_CODE_FR 	RSC_SYS_REG_TEA_CODE_EN
#define RSC_SYS_REG_TERR_CODE_FR	RSC_SYS_REG_TERR_CODE_EN
#define RSC_SYS_REG_TLOR_CODE_FR	RSC_SYS_REG_TLOR_CODE_EN
#define RSC_SYS_REG_E2H_CODE_FR 	RSC_SYS_REG_E2H_CODE_EN
#define RSC_SYS_REG_ID_CODE_FR		RSC_SYS_REG_ID_CODE_EN
#define RSC_SYS_REG_CD_CODE_FR		RSC_SYS_REG_CD_CODE_EN
#define RSC_SYS_REG_RW_CODE_FR		RSC_SYS_REG_RW_CODE_EN
#define RSC_SYS_REG_TRVM_CODE_FR	RSC_SYS_REG_TRVM_CODE_EN
#define RSC_SYS_REG_HCD_CODE_FR 	RSC_SYS_REG_HCD_CODE_EN
#define RSC_SYS_REG_TDZ_CODE_FR 	RSC_SYS_REG_TDZ_CODE_EN
#define RSC_SYS_REG_TGE_CODE_FR 	RSC_SYS_REG_TGE_CODE_EN
#define RSC_SYS_REG_TVM_CODE_FR 	RSC_SYS_REG_TVM_CODE_EN
#define RSC_SYS_REG_TTLB_CODE_FR	RSC_SYS_REG_TTLB_CODE_EN
#define RSC_SYS_REG_TPU_CODE_FR 	RSC_SYS_REG_TPU_CODE_EN
#define RSC_SYS_REG_TPCP_CODE_FR	RSC_SYS_REG_TPCP_CODE_EN
#define RSC_SYS_REG_TSW_CODE_FR 	RSC_SYS_REG_TSW_CODE_EN
#define RSC_SYS_REG_TACR_CODE_FR	RSC_SYS_REG_TACR_CODE_EN
#define RSC_SYS_REG_TSC_CODE_FR 	RSC_SYS_REG_TSC_CODE_EN
#define RSC_SYS_REG_TID3_CODE_FR	RSC_SYS_REG_TID3_CODE_EN
#define RSC_SYS_REG_TID2_CODE_FR	RSC_SYS_REG_TID2_CODE_EN
#define RSC_SYS_REG_TID1_CODE_FR	RSC_SYS_REG_TID1_CODE_EN
#define RSC_SYS_REG_TID0_CODE_FR	RSC_SYS_REG_TID0_CODE_EN
#define RSC_SYS_REG_TWE_CODE_FR 	RSC_SYS_REG_TWE_CODE_EN
#define RSC_SYS_REG_TWI_CODE_FR 	RSC_SYS_REG_TWI_CODE_EN
#define RSC_SYS_REG_DC_CODE_FR		RSC_SYS_REG_DC_CODE_EN
#define RSC_SYS_REG_BSU_CODE_FR 	RSC_SYS_REG_BSU_CODE_EN
#define RSC_SYS_REG_FB_CODE_FR		RSC_SYS_REG_FB_CODE_EN
#define RSC_SYS_REG_VSE_CODE_FR 	RSC_SYS_REG_VSE_CODE_EN
#define RSC_SYS_REG_VI_CODE_FR		RSC_SYS_REG_VI_CODE_EN
#define RSC_SYS_REG_VF_CODE_FR		RSC_SYS_REG_VF_CODE_EN
#define RSC_SYS_REG_AMO_CODE_FR 	RSC_SYS_REG_AMO_CODE_EN
#define RSC_SYS_REG_IMO_CODE_FR 	RSC_SYS_REG_IMO_CODE_EN
#define RSC_SYS_REG_FMO_CODE_FR 	RSC_SYS_REG_FMO_CODE_EN
#define RSC_SYS_REG_PTW_CODE_FR 	RSC_SYS_REG_PTW_CODE_EN
#define RSC_SYS_REG_SWIO_CODE_FR	RSC_SYS_REG_SWIO_CODE_EN
#define RSC_SYS_REG_VM_CODE_FR		RSC_SYS_REG_VM_CODE_EN

#define RSC_SYS_REG_SCTL_CODE_FR	RSC_SYS_REG_SCTL_CODE_EN
#define RSC_SYS_REG_TIDCP_CODE_FR	RSC_SYS_REG_TIDCP_CODE_EN
#define RSC_SYS_REG_SPINT_CODE_FR	RSC_SYS_REG_SPINT_CODE_EN
#define RSC_SYS_REG_NMI_CODE_FR		RSC_SYS_REG_NMI_CODE_EN
#define RSC_SYS_REG_EnTP2_CODE_FR	RSC_SYS_REG_EnTP2_CODE_EN
#define RSC_SYS_REG_TCSO_CODE_FR	RSC_SYS_REG_TCSO_CODE_EN
#define RSC_SYS_REG_TCSO1_CODE_FR	RSC_SYS_REG_TCSO1_CODE_EN
#define RSC_SYS_REG_TCSO0_CODE_FR	RSC_SYS_REG_TCSO0_CODE_EN
#define RSC_SYS_REG_EPAN_CODE_FR	RSC_SYS_REG_EPAN_CODE_EN
#define RSC_SYS_REG_EnALS_CODE_FR	RSC_SYS_REG_EnALS_CODE_EN
#define RSC_SYS_REG_EnAS0_CODE_FR	RSC_SYS_REG_EnAS0_CODE_EN
#define RSC_SYS_REG_EnASR_CODE_FR	RSC_SYS_REG_EnASR_CODE_EN
#define RSC_SYS_REG_TME_CODE_FR 	RSC_SYS_REG_TME_CODE_EN
#define RSC_SYS_REG_TME1_CODE_FR	RSC_SYS_REG_TME1_CODE_EN
#define RSC_SYS_REG_TME0_CODE_FR	RSC_SYS_REG_TME0_CODE_EN
#define RSC_SYS_REG_TMT_CODE_FR 	RSC_SYS_REG_TMT_CODE_EN
#define RSC_SYS_REG_TMT1_CODE_FR	RSC_SYS_REG_TMT1_CODE_EN
#define RSC_SYS_REG_TMT0_CODE_FR	RSC_SYS_REG_TMT0_CODE_EN
#define RSC_SYS_REG_TWE_D_CODE_FR	RSC_SYS_REG_TWE_D_CODE_EN
#define RSC_SYS_REG_TWE_C_CODE_FR	RSC_SYS_REG_TWE_C_CODE_EN
#define RSC_SYS_REG_TWE_E_CODE_FR	RSC_SYS_REG_TWE_E_CODE_EN
#define RSC_SYS_REG_DSSBS_CODE_FR	RSC_SYS_REG_DSSBS_CODE_EN
#define RSC_SYS_REG_ATA_CODE_FR 	RSC_SYS_REG_ATA_CODE_EN
#define RSC_SYS_REG_ATA1_CODE_FR	RSC_SYS_REG_ATA1_CODE_EN
#define RSC_SYS_REG_ATA0_CODE_FR	RSC_SYS_REG_ATA0_CODE_EN
#define RSC_SYS_REG_TCF_CODE_FR 	RSC_SYS_REG_TCF_CODE_EN
#define RSC_SYS_REG_TCF1_CODE_FR	RSC_SYS_REG_TCF1_CODE_EN
#define RSC_SYS_REG_TCF0_CODE_FR	RSC_SYS_REG_TCF0_CODE_EN
#define RSC_SYS_REG_ITFSB_CODE_FR	RSC_SYS_REG_ITFSB_CODE_EN
#define RSC_SYS_REG_BT_CODE_FR		RSC_SYS_REG_BT_CODE_EN
#define RSC_SYS_REG_BT1_CODE_FR 	RSC_SYS_REG_BT1_CODE_EN
#define RSC_SYS_REG_BT0_CODE_FR 	RSC_SYS_REG_BT0_CODE_EN
#define RSC_SYS_REG_EnFPM_CODE_FR	RSC_SYS_REG_EnFPM_CODE_EN
#define RSC_SYS_REG_MSCEn_CODE_FR	RSC_SYS_REG_MSCEn_CODE_EN
#define RSC_SYS_REG_CMOW_CODE_FR	RSC_SYS_REG_CMOW_CODE_EN
#define RSC_SYS_REG_EnIA_CODE_FR	RSC_SYS_REG_EnIA_CODE_EN
#define RSC_SYS_REG_EnIB_CODE_FR	RSC_SYS_REG_EnIB_CODE_EN
#define RSC_SYS_REG_LSM_CODE_FR 	RSC_SYS_REG_LSM_CODE_EN
#define RSC_SYS_REG_LSMA_CODE_FR	RSC_SYS_REG_LSMA_CODE_EN
#define RSC_SYS_REG_LSMD_CODE_FR	RSC_SYS_REG_LSMD_CODE_EN
#define RSC_SYS_REG_EnDA_CODE_FR	RSC_SYS_REG_EnDA_CODE_EN
#define RSC_SYS_REG_UCI_CODE_FR 	RSC_SYS_REG_UCI_CODE_EN
#define RSC_SYS_REG_EE_CODE_FR		RSC_SYS_REG_EE_CODE_EN
#define RSC_SYS_REG_E0E_CODE_FR 	RSC_SYS_REG_E0E_CODE_EN
#define RSC_SYS_REG_SPAN_CODE_FR	RSC_SYS_REG_SPAN_CODE_EN
#define RSC_SYS_REG_EIS_CODE_FR 	RSC_SYS_REG_EIS_CODE_EN
#define RSC_SYS_REG_IESB_CODE_FR	RSC_SYS_REG_IESB_CODE_EN
#define RSC_SYS_REG_TSCXT_CODE_FR	RSC_SYS_REG_TSCXT_CODE_EN
#define RSC_SYS_REG_WXN_CODE_FR 	RSC_SYS_REG_WXN_CODE_EN
#define RSC_SYS_REG_nTWE_CODE_FR	RSC_SYS_REG_nTWE_CODE_EN
#define RSC_SYS_REG_nTWI_CODE_FR	RSC_SYS_REG_nTWI_CODE_EN
#define RSC_SYS_REG_UCT_CODE_FR 	RSC_SYS_REG_UCT_CODE_EN
#define RSC_SYS_REG_DZE_CODE_FR 	RSC_SYS_REG_DZE_CODE_EN
#define RSC_SYS_REG_EnDB_CODE_FR	RSC_SYS_REG_EnDB_CODE_EN
#define RSC_SYS_REG_I_CODE_FR		RSC_SYS_REG_I_CODE_EN
#define RSC_SYS_REG_EOS_CODE_FR 	RSC_SYS_REG_EOS_CODE_EN
#define RSC_SYS_REG_RCTX_CODE_FR	RSC_SYS_REG_RCTX_CODE_EN
#define RSC_SYS_REG_UMA_CODE_FR 	RSC_SYS_REG_UMA_CODE_EN
#define RSC_SYS_REG_SED_CODE_FR 	RSC_SYS_REG_SED_CODE_EN
#define RSC_SYS_REG_ITD_CODE_FR 	RSC_SYS_REG_ITD_CODE_EN
#define RSC_SYS_REG_nAA_CODE_FR 	RSC_SYS_REG_nAA_CODE_EN
#define RSC_SYS_REG_CP15B_CODE_FR	RSC_SYS_REG_CP15B_CODE_EN
#define RSC_SYS_REG_SA0_CODE_FR 	RSC_SYS_REG_SA0_CODE_EN
#define RSC_SYS_REG_SA1_CODE_FR 	RSC_SYS_REG_SA1_CODE_EN
#define RSC_SYS_REG_C_CODE_FR		RSC_SYS_REG_C_CODE_EN
#define RSC_SYS_REG_A_CODE_FR		RSC_SYS_REG_A_CODE_EN
#define RSC_SYS_REG_M_CODE_FR		RSC_SYS_REG_M_CODE_EN

#define RSC_SYS_REG_SCTL2_CODE_FR	RSC_SYS_REG_SCTL2_CODE_EN
#define RSC_SYS_REG_CPTM_CODE_FR	RSC_SYS_REG_CPTM_CODE_EN
#define RSC_SYS_REG_CPTM0_CODE_FR	RSC_SYS_REG_CPTM0_CODE_EN
#define RSC_SYS_REG_CPTM1_CODE_FR	RSC_SYS_REG_CPTM1_CODE_EN
#define RSC_SYS_REG_CPTA_CODE_FR	RSC_SYS_REG_CPTA_CODE_EN
#define RSC_SYS_REG_CPTA0_CODE_FR	RSC_SYS_REG_CPTA0_CODE_EN
#define RSC_SYS_REG_CPTA1_CODE_FR	RSC_SYS_REG_CPTA1_CODE_EN
#define RSC_SYS_REG_PACM_CODE_FR	RSC_SYS_REG_PACM_CODE_EN
#define RSC_SYS_REG_PACM0_CODE_FR	RSC_SYS_REG_PACM0_CODE_EN
#define RSC_SYS_REG_PACM1_CODE_FR	RSC_SYS_REG_PACM1_CODE_EN
#define RSC_SYS_REG_128SR_CODE_FR	RSC_SYS_REG_128SR_CODE_EN
#define RSC_SYS_REG_EASE_CODE_FR	RSC_SYS_REG_EASE_CODE_EN
#define RSC_SYS_REG_ANERR_CODE_FR	RSC_SYS_REG_ANERR_CODE_EN
#define RSC_SYS_REG_ADERR_CODE_FR	RSC_SYS_REG_ADERR_CODE_EN
#define RSC_SYS_REG_NMEA_CODE_FR	RSC_SYS_REG_NMEA_CODE_EN

#define RSC_SYS_REG_FPSR_CODE_FR	RSC_SYS_REG_FPSR_CODE_EN
#define RSC_SYS_REG_FPSR_QC_CODE_FR	RSC_SYS_REG_FPSR_QC_CODE_EN
#define RSC_SYS_REG_FPSR_IDC_CODE_FR	RSC_SYS_REG_FPSR_IDC_CODE_EN
#define RSC_SYS_REG_FPSR_IXC_CODE_FR	RSC_SYS_REG_FPSR_IXC_CODE_EN
#define RSC_SYS_REG_FPSR_UFC_CODE_FR	RSC_SYS_REG_FPSR_UFC_CODE_EN
#define RSC_SYS_REG_FPSR_OFC_CODE_FR	RSC_SYS_REG_FPSR_OFC_CODE_EN
#define RSC_SYS_REG_FPSR_DZC_CODE_FR	RSC_SYS_REG_FPSR_DZC_CODE_EN
#define RSC_SYS_REG_FPSR_IOC_CODE_FR	RSC_SYS_REG_FPSR_IOC_CODE_EN

#define RSC_SYS_REG_FPCR_CODE_FR	RSC_SYS_REG_FPCR_CODE_EN
#define RSC_SYS_REG_FPCR_AHP_CODE_FR	RSC_SYS_REG_FPCR_AHP_CODE_EN
#define RSC_SYS_REG_FPCR_DN_CODE_FR	RSC_SYS_REG_FPCR_DN_CODE_EN
#define RSC_SYS_REG_FPCR_FZ_CODE_FR	RSC_SYS_REG_FPCR_FZ_CODE_EN
#define RSC_SYS_REG_FPCR_RM_CODE_FR	RSC_SYS_REG_FPCR_RM_CODE_EN
#define RSC_SYS_REG_FPCR_FZH_CODE_FR	RSC_SYS_REG_FPCR_FZH_CODE_EN

#define RSC_SYS_REG_EL_CODE_FR		RSC_SYS_REG_EL_CODE_EN
#define RSC_SYS_REG_EL_EXEC_CODE_FR	RSC_SYS_REG_EL_EXEC_CODE_EN
#define RSC_SYS_REG_EL_SEC_CODE_FR	RSC_SYS_REG_EL_SEC_CODE_EN

#define RSC_SYS_REG_SVCR_CODE_FR	RSC_SYS_REG_SVCR_CODE_EN
#define RSC_SYS_REG_SVCR_ZA_CODE_FR	RSC_SYS_REG_SVCR_ZA_CODE_EN
#define RSC_SYS_REG_SVCR_SM_CODE_FR	RSC_SYS_REG_SVCR_SM_CODE_EN

#define RSC_SYS_REG_CPACR_CODE_FR	RSC_SYS_REG_CPACR_CODE_EN
#define RSC_SYS_REG_ACR_TCP_CODE_FR	RSC_SYS_REG_ACR_TCP_CODE_EN
#define RSC_SYS_REG_ACR_TAM_CODE_FR	RSC_SYS_REG_ACR_TAM_CODE_EN
#define RSC_SYS_REG_ACR_POE_CODE_FR	RSC_SYS_REG_ACR_POE_CODE_EN
#define RSC_SYS_REG_ACR_TTA_CODE_FR	RSC_SYS_REG_ACR_TTA_CODE_EN
#define RSC_SYS_REG_ACR_SME_CODE_FR	RSC_SYS_REG_ACR_SME_CODE_EN
#define RSC_SYS_REG_ACR_FP_CODE_FR	RSC_SYS_REG_ACR_FP_CODE_EN
#define RSC_SYS_REG_ACR_ZEN_CODE_FR	RSC_SYS_REG_ACR_ZEN_CODE_EN
#define RSC_SYS_REG_ACR_R8_CODE_FR	RSC_SYS_REG_ACR_R8_CODE_EN
#define RSC_SYS_REG_ACR_R0_CODE_FR	RSC_SYS_REG_ACR_R0_CODE_EN

#define RSC_ISA_TITLE_CODE_FR		" Jeu d'instructions ""\xa9""tendu "

#define RSC_ISA_AES_COMM_CODE_FR	RSC_ISA_AES_COMM_CODE_EN
#define RSC_ISA_LSE_COMM_CODE_FR	RSC_ISA_LSE_COMM_CODE_EN
#define RSC_ISA_LSFE_COMM_CODE_FR	RSC_ISA_LSFE_COMM_CODE_EN
#define RSC_ISA_CRC32_COMM_CODE_FR	RSC_ISA_CRC32_COMM_CODE_EN
#define RSC_ISA_DP_COMM_CODE_FR 	RSC_ISA_DP_COMM_CODE_EN
#define RSC_ISA_EPAC_COMM_CODE_FR	RSC_ISA_EPAC_COMM_CODE_EN
#define RSC_ISA_FCMA_COMM_CODE_FR	RSC_ISA_FCMA_COMM_CODE_EN
#define RSC_ISA_FAMINMAX_COMM_CODE_FR	RSC_ISA_FAMINMAX_COMM_CODE_EN
#define RSC_ISA_FHM_COMM_CODE_FR	RSC_ISA_FHM_COMM_CODE_EN
#define RSC_ISA_FP_COMM_CODE_FR 	RSC_ISA_FP_COMM_CODE_EN
#define RSC_ISA_FPAC_COMM_CODE_FR	RSC_ISA_FPAC_COMM_CODE_EN
#define RSC_ISA_FPACCOMBINE_COMM_CODE_FR RSC_ISA_FPACCOMBINE_COMM_CODE_EN
#define RSC_ISA_JSCVT_COMM_CODE_FR	RSC_ISA_JSCVT_COMM_CODE_EN
#define RSC_ISA_LRCPC_COMM_CODE_FR	RSC_ISA_LRCPC_COMM_CODE_EN
#define RSC_ISA_LSUI_COMM_CODE_FR	RSC_ISA_LSUI_COMM_CODE_EN
#define RSC_ISA_LUT_COMM_CODE_FR	RSC_ISA_LUT_COMM_CODE_EN
#define RSC_ISA_MOPS_COMM_CODE_FR	RSC_ISA_MOPS_COMM_CODE_EN
#define RSC_ISA_OCCMO_COMM_CODE_FR	RSC_ISA_OCCMO_COMM_CODE_EN
#define RSC_ISA_PACIMP_COMM_CODE_FR	RSC_ISA_PACIMP_COMM_CODE_EN
#define RSC_ISA_PACQARMA3_COMM_CODE_FR	RSC_ISA_PACQARMA3_COMM_CODE_EN
#define RSC_ISA_PACQARMA5_COMM_CODE_FR	RSC_ISA_PACQARMA5_COMM_CODE_EN
#define RSC_ISA_PAUTH_COMM_CODE_FR	RSC_ISA_PAUTH_COMM_CODE_EN
#define RSC_ISA_PAUTH2_COMM_CODE_FR	RSC_ISA_PAUTH2_COMM_CODE_EN
#define RSC_ISA_PAUTH_LR_COMM_CODE_FR	RSC_ISA_PAUTH_LR_COMM_CODE_EN
#define RSC_ISA_PCDPHINT_COMM_CODE_FR	RSC_ISA_PCDPHINT_COMM_CODE_EN
#define RSC_ISA_PMULL_COMM_CODE_FR	RSC_ISA_PMULL_COMM_CODE_EN
#define RSC_ISA_PRFMSLC_COMM_CODE_FR	RSC_ISA_PRFMSLC_COMM_CODE_EN
#define RSC_ISA_FPRCVT_COMM_CODE_FR	RSC_ISA_FPRCVT_COMM_CODE_EN
#define RSC_ISA_FRINTTS_COMM_CODE_FR	RSC_ISA_FRINTTS_COMM_CODE_EN
#define RSC_ISA_SPECRES_COMM_CODE_FR	RSC_ISA_SPECRES_COMM_CODE_EN
#define RSC_ISA_ATS1A_COMM_CODE_FR	RSC_ISA_ATS1A_COMM_CODE_EN
#define RSC_ISA_BF16_COMM_CODE_FR	RSC_ISA_BF16_COMM_CODE_EN
#define RSC_ISA_EBF16_COMM_CODE_FR	RSC_ISA_EBF16_COMM_CODE_EN
#define RSC_ISA_CLRBHB_COMM_CODE_FR	RSC_ISA_CLRBHB_COMM_CODE_EN
#define RSC_ISA_CONSTPACFLD_COMM_CODE_FR RSC_ISA_CONSTPACFLD_COMM_CODE_EN
#define RSC_ISA_CPA_COMM_CODE_FR	RSC_ISA_CPA_COMM_CODE_EN
#define RSC_ISA_CSSC_COMM_CODE_FR	RSC_ISA_CSSC_COMM_CODE_EN
#define RSC_ISA_HBC_COMM_CODE_FR	RSC_ISA_HBC_COMM_CODE_EN
#define RSC_ISA_I8MM_COMM_CODE_FR	RSC_ISA_I8MM_COMM_CODE_EN
#define RSC_ISA_RPRES_COMM_CODE_FR	RSC_ISA_RPRES_COMM_CODE_EN
#define RSC_ISA_SB_COMM_CODE_FR 	RSC_ISA_SB_COMM_CODE_EN
#define RSC_ISA_SYSREG128_COMM_CODE_FR	RSC_ISA_SYSREG128_COMM_CODE_EN
#define RSC_ISA_SYSINSTR128_COMM_CODE_FR RSC_ISA_SYSINSTR128_COMM_CODE_EN
#define RSC_ISA_TLBIW_COMM_CODE_FR	RSC_ISA_TLBIW_COMM_CODE_EN
#define RSC_ISA_WFxT_COMM_CODE_FR	RSC_ISA_WFxT_COMM_CODE_EN
#define RSC_ISA_XS_COMM_CODE_FR 	RSC_ISA_XS_COMM_CODE_EN
#define RSC_ISA_LS64_COMM_CODE_FR	RSC_ISA_LS64_COMM_CODE_EN
#define RSC_ISA_DGH_COMM_CODE_FR	RSC_ISA_DGH_COMM_CODE_EN
#define RSC_ISA_DPB_COMM_CODE_FR	RSC_ISA_DPB_COMM_CODE_EN
#define RSC_ISA_RAND_COMM_CODE_FR	RSC_ISA_RAND_COMM_CODE_EN
#define RSC_ISA_RDMA_COMM_CODE_FR	RSC_ISA_RDMA_COMM_CODE_EN
#define RSC_ISA_RNG_TRAP_COMM_CODE_FR	RSC_ISA_RNG_TRAP_COMM_CODE_EN
#define RSC_ISA_RPRFM_COMM_CODE_FR	RSC_ISA_RPRFM_COMM_CODE_EN
#define RSC_ISA_SHA_COMM_CODE_FR	RSC_ISA_SHA_COMM_CODE_EN
#define RSC_ISA_SM_COMM_CODE_FR 	RSC_ISA_SM_COMM_CODE_EN
#define RSC_ISA_SIMD_COMM_CODE_FR	RSC_ISA_SIMD_COMM_CODE_EN
#define RSC_ISA_SME_COMM_CODE_FR	RSC_ISA_SME_COMM_CODE_EN
#define RSC_ISA_SVE_COMM_CODE_FR	RSC_ISA_SVE_COMM_CODE_EN
#define RSC_ISA_FlagM_COMM_CODE_FR 	RSC_ISA_FlagM_COMM_CODE_EN

#define RSC_FEATURES_TITLE_CODE_FR	" Caract""\xa9""ristiques "
#define RSC_ON_CODE_FR			" ON"
#define RSC_OFF_CODE_FR 		"OFF"
#define RSC_FMW_CODE_FR 		"FMW"
#define RSC_NOT_AVAILABLE_CODE_FR	"N/A"
#define RSC_AUTOMATIC_CODE_FR		"AUTO"
#define RSC_UNABLE_CODE_FR		"Indispo"
#define RSC_MISSING_CODE_FR		"Absent"
#define RSC_PRESENT_CODE_FR		"Capable"
#define RSC_VARIANT_CODE_FR		"Variant"
#define RSC_INVARIANT_CODE_FR		"Invariant"

#define RSC_FEATURES_ACPI_CODE_FR	\
		"Configuration Avanc""\xa9""e & Interface d'Alimentation"

#define RSC_FEATURES_AMU_CODE_FR	\
			"Unit""\xa9"" de surveillance de l'activit""\xa9"

#define RSC_FEATURES_BIG_END_CODE_FR	"Endianisme Mixte"
#define RSC_FEATURES_BTI_CODE_FR	"Validation des cibles de branche"

#define RSC_FEATURES_EBEP_CODE_FR	\
	"Profilage d'""\xa9""v""\xa9""nements bas""\xa9"" sur les exceptions"

#define RSC_FEATURES_ECBHB_CODE_FR	\
				"Contr. Exploit. avec Tampon Hist. Branchement"

#define RSC_FEATURES_ECV_CODE_FR	\
			"Virtualisation am""\xa9""lior""\xa9""e des compteurs"

#define RSC_FEATURES_DF2_CODE_FR	"Extension de Double D""\xa9""faut"
#define RSC_FEATURES_DIT_CODE_FR	\
		"Chronom""\xa9""trage Ind""\xa9""pendant des Donn""\xa9""es"

#define RSC_FEATURES_EXS_CODE_FR	\
				"Sync. de Contexte & Gestion d'Exceptions"

#define RSC_FEATURES_FGT_CODE_FR	\
	"Contr""\xb4""les de Pi""\xa8""ges ""\xa0"" Granularit""\xa9"" Fine"

#define RSC_FEATURES_FPMR_CODE_FR    "Registre de mode de la virgule flottante"
#define RSC_FEATURES_PFAR_CODE_FR	\
				"Registres de D""\xa9""faut d'Adresse Physique"

#define RSC_FEATURES_GCS_CODE_FR	\
				"Pile de Contr""\xb4""le Prot""\xa9""g""\xa9""e"

#define RSC_FEATURES_GIC_CODE_FR	\
		"Contr""\xb4""leur d'interruption g""\xa9""n""\xa9""rique"

#define RSC_FEATURES_MTE_CODE_FR	\
				"Extension de marquage de la m""\xa9""moire"

#define RSC_FEATURES_MPAM_CODE_FR	\
			"Partitionnement et supervision de la m""\xa9""moire"

#define RSC_FEATURES_MTE_FAR_CODE_FR	"Signaler Tag Check Fault"
#define RSC_FEATURES_MTE_PERM_CODE_FR	\
				"Droits d'acc""\xa8""s aux balises d'allocation"

#define RSC_FEATURES_MTE_STOREONLY_CODE_FR \
				"V""\xa9""rification de la balise Store-only"

#define RSC_FEATURES_NMI_CODE_FR	"Interruption non masquable"
#define RSC_FEATURES_PA_CODE_FR 	"Plage d'Adressage Physique"
#define RSC_FEATURES_PAN_CODE_FR	"Aucun privil""\xa8""ge d'acc""\xa8""s"
#define RSC_FEATURES_RAS_CODE_FR	\
		"Fiabilit""\xa9"" Disponibilit""\xa9"" et Durabilit""\xa9"

#define RSC_FEATURES_RME_CODE_FR	"Extension de Gestion de Domaine"
#define RSC_FEATURES_SEL2_CODE_FR	\
			"Impl""\xa9""mentation S""\xa9""curis""\xa9""e de l'EL2"

#define RSC_FEATURES_THE_CODE_FR	\
				"Endurcissement ""\x89""tendu de la Translation"

#define RSC_FEATURES_TLB_CODE_FR	"Instructions de gestion TLB"
#define RSC_FEATURES_TME_CODE_FR	\
				"Extension de m""\xa9""moire transactionnelle"

#define RSC_FEATURES_TSC_CODE_FR	"Compteur d'horodatage"
#define RSC_FEATURES_UAO_CODE_FR	\
				"Surcharge de l'acc""\xa8""s utilisateur"

#define RSC_FEATURES_UINJ_CODE_FR	\
				"Injection d'instructions non d""\xa9""finies"

#define RSC_FEATURES_VA_CODE_FR 	"Plage d'adressage virtuel"
#define RSC_FEATURES_VHE_CODE_FR	\
				"Extensions d'H""\xb4""te de Virtualisation"

#define RSC_FEAT_SECTION_MECH_CODE_FR	"M""\xa9""canismes d'att""\xa9""nuation"
#define RSC_FEAT_SECTION_SEC_CODE_FR	"Fonctions de s""\xa9""curit""\xa9"

#define RSC_TECHNOLOGIES_TITLE_CODE_FR	" Technologies "
#define RSC_TECHNOLOGIES_DCU_CODE_FR  "Unit""\xa9"" de cache de donn""\xa9""es"
#define RSC_TECHNOLOGIES_ICU_CODE_FR  "Unit""\xa9"" de cache d'instructions"
#define RSC_TECHNOLOGIES_MMU_CODE_FR "Unit""\xa9"" de gestion de m""\xa9""moire"
#define RSC_TECHNOLOGIES_VM_CODE_FR	"Virtualisation"
#define RSC_TECHNOLOGIES_IOMMU_CODE_FR	"MMU E/S"
#define RSC_TECHNOLOGIES_SMT_CODE_FR	"Multithreading simultan""\xa9"
#define RSC_TECHNOLOGIES_HYBRID_CODE_FR "Technologie big.LITTLE"
#define RSC_TECHNOLOGIES_HYPERV_CODE_FR "Hyperviseur"
#define RSC_TECHNOLOGIES_VM_COMM_CODE_FR RSC_TECHNOLOGIES_VM_COMM_CODE_EN
#define RSC_TECHNOLOGIES_IOMMU_COMM_CODE_FR RSC_TECHNOLOGIES_IOMMU_COMM_CODE_EN
#define RSC_TECHNOLOGIES_ICN_CODE_FR	"Technologie d'interconnexion"
#define RSC_TECHNOLOGIES_DYNAMIQ_CODE_FR RSC_TECHNOLOGIES_DYNAMIQ_CODE_EN
#define RSC_TECHNOLOGIES_CMN_CODE_FR	RSC_TECHNOLOGIES_CMN_CODE_EN
#define RSC_TECHNOLOGIES_CCN_CODE_FR	RSC_TECHNOLOGIES_CCN_CODE_EN
#define RSC_TECHNOLOGIES_CCI_CODE_FR	RSC_TECHNOLOGIES_CCI_CODE_EN

#define RSC_PERF_MON_TITLE_CODE_FR	" Gestion de la performance "
#define RSC_PERF_CAPS_TITLE_CODE_FR	" Capacit""\xa9""s de performances "
#define RSC_VERSION_CODE_FR		"Version"
#define RSC_COUNTERS_CODE_FR		"Compteurs"
#define RSC_GENERAL_CTRS_CODE_FR	"G""\xa9""n""\xa9""raux"
#define RSC_FIXED_CTRS_CODE_FR		"Fixes"
#define RSC_PERF_MON_UNIT_BIT_CODE_FR	"bits"
#define RSC_PERF_MON_CPPC_CODE_FR	\
		"Contr""\xb4""le collaboratif des performances du processeur"

#define RSC_PERF_MON_PCT_CODE_FR	RSC_PERF_MON_PCT_CODE_EN
#define RSC_PERF_MON_PSS_CODE_FR	RSC_PERF_MON_PSS_CODE_EN
#define RSC_PERF_MON_PPC_CODE_FR	RSC_PERF_MON_PPC_CODE_EN
#define RSC_PERF_MON_CPC_CODE_FR	RSC_PERF_MON_CPC_CODE_EN
#define RSC_PERF_MON_CST_CODE_FR	RSC_PERF_MON_CST_CODE_EN
#define RSC_PERF_MON_HWP_CODE_FR	"Hardware-Controlled Performance States"
#define RSC_PERF_MON_LOW_PWR_CODE_FR	"M""\xa9""thodes de Basse Consommation"
#define RSC_PERF_MON_WFI_CODE_FR	"Attente d'Interruption"
#define RSC_PERF_MON_WFE_CODE_FR	"Attente d'""\x89""v""\xa9""nement"

#define RSC_PERF_MON_MONITOR_MWAIT_CODE_FR	"MONITOR/MWAIT"
#define RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_FR	"State index"
#define RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_FR	"Sub C-State"

#define RSC_PERF_MON_CORE_CYCLE_CODE_FR "Core Cycles Counter"
#define RSC_PERF_MON_INST_RET_CODE_FR	"Instructions Counter"

#define RSC_PERF_MON_PMC_COMM_CODE_FR	RSC_PERF_MON_PMC_COMM_CODE_EN

#define RSC_PERF_MON_CPPC_COMM_CODE_FR	" Micrologiciel "

#define RSC_POWER_THERMAL_TITLE_CODE_FR " Puissance, courant et thermique "
#define RSC_POWER_THERMAL_CPPC_CODE_FR	"CPPC Pr""\xa9""f""\xa9""rence " \
					"\xa9""nerg""\xa9""tique"

#define RSC_POWER_THERMAL_TJMAX_CODE_FR "Temp""\xa9""rature Offset:Jonction"
#define RSC_POWER_THERMAL_DTS_CODE_FR	"Capteur thermique num""\xa9""rique"
#define RSC_POWER_THERMAL_PLN_CODE_FR	"Notification de puissance"
#define RSC_POWER_THERMAL_PTM_CODE_FR	"Gestion thermique du Package"
#define RSC_POWER_THERMAL_UNITS_CODE_FR "Unit""\xa9""s"
#define RSC_POWER_THERMAL_POWER_CODE_FR "Puissance"
#define RSC_POWER_THERMAL_ENERGY_CODE_FR "\x89""nergie"
#define RSC_POWER_THERMAL_WINDOW_CODE_FR "Intervalle"
#define RSC_POWER_THERMAL_WATT_CODE_FR	"watt"
#define RSC_POWER_THERMAL_JOULE_CODE_FR "joule"
#define RSC_POWER_THERMAL_SECOND_CODE_FR "seconde"
#define RSC_POWER_THERMAL_TDP_CODE_FR	"Dissipation thermique"
#define RSC_POWER_THERMAL_MIN_CODE_FR	"Puissance minimale"
#define RSC_POWER_THERMAL_MAX_CODE_FR	"Puissance maximale"
#define RSC_POWER_THERMAL_TPL_CODE_FR	"Limite de puissance"
#define RSC_POWER_THERMAL_TW_CODE_FR	"Fen""\xaa""tre temporelle"
#define RSC_POWER_THERMAL_EDC_CODE_FR	"Limite de courant sup""\xa9""rieure"
#define RSC_POWER_THERMAL_TDC_CODE_FR	"Limite de courant sup""\xa9""rieure"
#define RSC_POWER_THERMAL_POINT_CODE_FR "Seuil thermique"

#define RSC_THERMAL_POINT_THRESHOLD_CODE_FR	"Seuil"
#define RSC_THERMAL_POINT_LIMIT_CODE_FR 	"Limite"
#define RSC_THERMAL_POINT_THRESHOLD_1_CODE_FR	"Seuil DTS #1"
#define RSC_THERMAL_POINT_THRESHOLD_2_CODE_FR	"Seuil DTS #2"
#define RSC_THERMAL_POINT_TRIP_LIMIT_CODE_FR	"D""\xa9""clencheur du" \
						" moniteur thermique"

#define RSC_THERMAL_POINT_HTC_LIMIT_CODE_FR	"Limite de temp"	\
						"\xa9""rature HTC"

#define RSC_THERMAL_POINT_HTC_HYST_CODE_FR	"Hyst""\xa9""r""\xa9""sis" \
						" de temp""\xa9""rature HTC"

#define RSC_THERMAL_OFFSET_TITLE_CODE_FR	" Offset thermique "

#define RSC_KERNEL_TITLE_CODE_FR	" Noyau "
#define RSC_KERNEL_TOTAL_RAM_CODE_FR	"RAM totale"
#define RSC_KERNEL_SHARED_RAM_CODE_FR	"RAM partag""\xa9""e"
#define RSC_KERNEL_FREE_RAM_CODE_FR	"RAM libre"
#define RSC_KERNEL_BUFFER_RAM_CODE_FR	"RAM Tampon"
#define RSC_KERNEL_TOTAL_HIGH_CODE_FR	"M""\xa9""moire haute totale"
#define RSC_KERNEL_FREE_HIGH_CODE_FR	"M""\xa9""moire haute libre"
#define RSC_KERNEL_GOVERNOR_CODE_FR	"Gouverneur"
#define RSC_KERNEL_FREQ_DRIVER_CODE_FR	"Pilote CPU-Freq"
#define RSC_KERNEL_IDLE_DRIVER_CODE_FR	"Pilote CPU-Idle"
#define RSC_KERNEL_CLOCK_SOURCE_CODE_FR "Source Horloge"
#define RSC_KERNEL_RELEASE_CODE_FR	"\x89""dition"
#define RSC_KERNEL_VERSION_CODE_FR	"Version"
#define RSC_KERNEL_MACHINE_CODE_FR	"Machine"
#define RSC_KERNEL_MEMORY_CODE_FR	"M""\xa9""moire"
#define RSC_KERNEL_STATE_CODE_FR	"\x89""tat"
#define RSC_KERNEL_POWER_CODE_FR	"Puissance"
#define RSC_KERNEL_LATENCY_CODE_FR	"Latence"
#define RSC_KERNEL_RESIDENCY_CODE_FR	"P""\xa9""riode"
#define RSC_KERNEL_LIMIT_CODE_FR	"Limite Idle"

#define RSC_TOPOLOGY_TITLE_CODE_FR	" Topologie "

#define RSC_MEM_CTRL_TITLE_CODE_FR	" Contr""\xb4""leur M""\xa9""moire "
#define RSC_MEM_CTRL_SUBSECT1_0_CODE_FR 	"Contr"
#define RSC_MEM_CTRL_SUBSECT1_1_CODE_FR 	"\xb4""leur"
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
#define RSC_MEM_CTRL_TWELVE_CHA_0_CODE_FR	"Douze"
#define RSC_MEM_CTRL_TWELVE_CHA_1_CODE_FR	" Cana"
#define RSC_MEM_CTRL_TWELVE_CHA_2_CODE_FR	"ux   "
#define RSC_MEM_CTRL_UNDEFINED_0_CODE_FR	"    I"
#define RSC_MEM_CTRL_UNDEFINED_1_CODE_FR	"nd""\xa9""fi"
#define RSC_MEM_CTRL_UNDEFINED_2_CODE_FR	"nie  "
#define RSC_MEM_CTRL_BUS_RATE_0_CODE_FR 	" D""\xa9""bi"
#define RSC_MEM_CTRL_BUS_RATE_1_CODE_FR 	"t Bus"
#define RSC_MEM_CTRL_BUS_SPEED_0_CODE_FR	"Trans"
#define RSC_MEM_CTRL_BUS_SPEED_1_CODE_FR	"fert "
#define RSC_MEM_CTRL_RAM_STD_0_CODE_FR		"     "
#define RSC_MEM_CTRL_RAM_STD_1_CODE_FR		"     "
#define RSC_MEM_CTRL_RAM_STD_2_CODE_FR		"   LP"
#define RSC_MEM_CTRL_RAM_STD_3_CODE_FR		" REG "
#define RSC_MEM_CTRL_DRAM_DDR2_0_CODE_FR	"DDR2 "
#define RSC_MEM_CTRL_DRAM_DDR3_0_CODE_FR	"DDR3 "
#define RSC_MEM_CTRL_DRAM_DDR4_0_CODE_FR	"DDR4 "
#define RSC_MEM_CTRL_DRAM_DDR5_0_CODE_FR	"DDR5 "
#define RSC_MEM_CTRL_DRAM_SPEED_0_CODE_FR	"DRAM "
#define RSC_MEM_CTRL_DRAM_SPEED_1_CODE_FR	"Fr""\xa9""q."
#define RSC_MEM_CTRL_SUBSECT2_0_CODE_FR 	" G""\xa9""om"
#define RSC_MEM_CTRL_SUBSECT2_1_CODE_FR 	"\xa9""trie"
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
#define RSC_MEM_CTRL_DIMM_SIZE_1_CODE_FR	"e M""\xa9""m"
#define RSC_MEM_CTRL_DIMM_SIZE_2_CODE_FR	"oire "
#define RSC_MEM_CTRL_DIMM_SIZE_3_CODE_FR	"(MB) "

#define RSC_DDR3_CL_COMM_CODE_FR	RSC_DDR3_CL_COMM_CODE_EN
#define RSC_DDR3_RCD_COMM_CODE_FR	RSC_DDR3_RCD_COMM_CODE_EN
#define RSC_DDR3_RP_COMM_CODE_FR	RSC_DDR3_RP_COMM_CODE_EN
#define RSC_DDR3_RAS_COMM_CODE_FR	RSC_DDR3_RAS_COMM_CODE_EN
#define RSC_DDR3_RRD_COMM_CODE_FR	RSC_DDR3_RRD_COMM_CODE_EN
#define RSC_DDR3_RFC_COMM_CODE_FR	RSC_DDR3_RFC_COMM_CODE_EN
#define RSC_DDR3_WR_COMM_CODE_FR	RSC_DDR3_WR_COMM_CODE_EN
#define RSC_DDR3_RTP_COMM_CODE_FR	RSC_DDR3_RTP_COMM_CODE_EN
#define RSC_DDR3_WTP_COMM_CODE_FR	RSC_DDR3_WTP_COMM_CODE_EN
#define RSC_DDR3_FAW_COMM_CODE_FR	RSC_DDR3_FAW_COMM_CODE_EN
#define RSC_DDR3_B2B_COMM_CODE_FR	RSC_DDR3_B2B_COMM_CODE_EN
#define RSC_DDR3_CWL_COMM_CODE_FR	RSC_DDR3_CWL_COMM_CODE_EN
#define RSC_DDR3_CMD_COMM_CODE_FR	RSC_DDR3_CMD_COMM_CODE_EN
#define RSC_DDR3_REFI_COMM_CODE_FR	RSC_DDR3_REFI_COMM_CODE_EN
#define RSC_DDR3_DDWRTRD_COMM_CODE_FR	RSC_DDR3_DDWRTRD_COMM_CODE_EN
#define RSC_DDR3_DRWRTRD_COMM_CODE_FR	RSC_DDR3_DRWRTRD_COMM_CODE_EN
#define RSC_DDR3_SRWRTRD_COMM_CODE_FR	RSC_DDR3_SRWRTRD_COMM_CODE_EN
#define RSC_DDR3_DDRDTWR_COMM_CODE_FR	RSC_DDR3_DDRDTWR_COMM_CODE_EN
#define RSC_DDR3_DRRDTWR_COMM_CODE_FR	RSC_DDR3_DRRDTWR_COMM_CODE_EN
#define RSC_DDR3_SRRDTWR_COMM_CODE_FR	RSC_DDR3_SRRDTWR_COMM_CODE_EN
#define RSC_DDR3_DDRDTRD_COMM_CODE_FR	RSC_DDR3_DDRDTRD_COMM_CODE_EN
#define RSC_DDR3_DRRDTRD_COMM_CODE_FR	RSC_DDR3_DRRDTRD_COMM_CODE_EN
#define RSC_DDR3_SRRDTRD_COMM_CODE_FR	RSC_DDR3_SRRDTRD_COMM_CODE_EN
#define RSC_DDR3_DDWRTWR_COMM_CODE_FR	RSC_DDR3_DDWRTWR_COMM_CODE_EN
#define RSC_DDR3_DRWRTWR_COMM_CODE_FR	RSC_DDR3_DRWRTWR_COMM_CODE_EN
#define RSC_DDR3_SRWRTWR_COMM_CODE_FR	RSC_DDR3_SRWRTWR_COMM_CODE_EN
#define RSC_DDR3_XS_COMM_CODE_FR	RSC_DDR3_XS_COMM_CODE_EN
#define RSC_DDR3_XP_COMM_CODE_FR	RSC_DDR3_XP_COMM_CODE_EN
#define RSC_DDR3_CKE_COMM_CODE_FR	RSC_DDR3_CKE_COMM_CODE_EN
#define RSC_DDR3_ECC_COMM_CODE_FR	RSC_DDR3_ECC_COMM_CODE_EN

#define RSC_DDR4_RCD_R_COMM_CODE_FR	RSC_DDR4_RCD_R_COMM_CODE_EN
#define RSC_DDR4_RCD_W_COMM_CODE_FR	RSC_DDR4_RCD_W_COMM_CODE_EN
#define RSC_DDR4_RDRD_SCL_COMM_CODE_FR	RSC_DDR4_RDRD_SCL_COMM_CODE_EN
#define RSC_DDR4_RDRD_SC_COMM_CODE_FR	RSC_DDR4_RDRD_SC_COMM_CODE_EN
#define RSC_DDR4_RDRD_SD_COMM_CODE_FR	RSC_DDR4_RDRD_SD_COMM_CODE_EN
#define RSC_DDR4_RDRD_DD_COMM_CODE_FR	RSC_DDR4_RDRD_DD_COMM_CODE_EN
#define RSC_DDR4_RDWR_SCL_COMM_CODE_FR	RSC_DDR4_RDWR_SCL_COMM_CODE_EN
#define RSC_DDR4_RDWR_SC_COMM_CODE_FR	RSC_DDR4_RDWR_SC_COMM_CODE_EN
#define RSC_DDR4_RDWR_SD_COMM_CODE_FR	RSC_DDR4_RDWR_SD_COMM_CODE_EN
#define RSC_DDR4_RDWR_DD_COMM_CODE_FR	RSC_DDR4_RDWR_DD_COMM_CODE_EN
#define RSC_DDR4_WRRD_SCL_COMM_CODE_FR	RSC_DDR4_WRRD_SCL_COMM_CODE_EN
#define RSC_DDR4_WRRD_SC_COMM_CODE_FR	RSC_DDR4_WRRD_SC_COMM_CODE_EN
#define RSC_DDR4_WRRD_SD_COMM_CODE_FR	RSC_DDR4_WRRD_SD_COMM_CODE_EN
#define RSC_DDR4_WRRD_DD_COMM_CODE_FR	RSC_DDR4_WRRD_DD_COMM_CODE_EN
#define RSC_DDR4_WRWR_SCL_COMM_CODE_FR	RSC_DDR4_WRWR_SCL_COMM_CODE_EN
#define RSC_DDR4_WRWR_SC_COMM_CODE_FR	RSC_DDR4_WRWR_SC_COMM_CODE_EN
#define RSC_DDR4_WRWR_SD_COMM_CODE_FR	RSC_DDR4_WRWR_SD_COMM_CODE_EN
#define RSC_DDR4_WRWR_DD_COMM_CODE_FR	RSC_DDR4_WRWR_DD_COMM_CODE_EN
#define RSC_DDR4_RRD_S_COMM_CODE_FR	RSC_DDR4_RRD_S_COMM_CODE_EN
#define RSC_DDR4_RRD_L_COMM_CODE_FR	RSC_DDR4_RRD_L_COMM_CODE_EN
#define RSC_DDR4_CPDED_COMM_CODE_FR	RSC_DDR4_CPDED_COMM_CODE_EN
#define RSC_DDR4_GEAR_COMM_CODE_FR	RSC_DDR4_GEAR_COMM_CODE_EN

#define RSC_DDR4_ZEN_RC_COMM_CODE_FR	RSC_DDR4_ZEN_RC_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTR_S_COMM_CODE_FR RSC_DDR4_ZEN_WTR_S_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTR_L_COMM_CODE_FR RSC_DDR4_ZEN_WTR_L_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_EN
#define RSC_DDR4_ZEN_RTP_COMM_CODE_FR	RSC_DDR4_ZEN_RTP_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDWR_COMM_CODE_FR	RSC_DDR4_ZEN_RDWR_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRRD_COMM_CODE_FR	RSC_DDR4_ZEN_WRRD_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_FR RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_FR RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_EN
#define RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_FR RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_FR RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_FR RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_FR RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_EN
#define RSC_DDR4_ZEN_RFC1_COMM_CODE_FR	RSC_DDR4_ZEN_RFC1_COMM_CODE_EN
#define RSC_DDR4_ZEN_RFC2_COMM_CODE_FR	RSC_DDR4_ZEN_RFC2_COMM_CODE_EN
#define RSC_DDR4_ZEN_RFC4_COMM_CODE_FR	RSC_DDR4_ZEN_RFC4_COMM_CODE_EN
#define RSC_DDR4_ZEN_RCPB_COMM_CODE_FR	RSC_DDR4_ZEN_RCPB_COMM_CODE_EN
#define RSC_DDR4_ZEN_RPPB_COMM_CODE_FR	RSC_DDR4_ZEN_RPPB_COMM_CODE_EN
#define RSC_DDR4_ZEN_BGS_COMM_CODE_FR	RSC_DDR4_ZEN_BGS_COMM_CODE_EN
#define RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_FR RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_EN
#define RSC_DDR4_ZEN_BAN_COMM_CODE_FR	RSC_DDR4_ZEN_BAN_COMM_CODE_EN
#define RSC_DDR4_ZEN_RCPAGE_COMM_CODE_FR RSC_DDR4_ZEN_RCPAGE_COMM_CODE_EN
#define RSC_DDR4_ZEN_GDM_COMM_CODE_FR	RSC_DDR4_ZEN_GDM_COMM_CODE_EN
#define RSC_DDR4_ZEN_MRD_COMM_CODE_FR	RSC_DDR4_ZEN_MRD_COMM_CODE_EN
#define RSC_DDR4_ZEN_MOD_COMM_CODE_FR	RSC_DDR4_ZEN_MOD_COMM_CODE_EN
#define RSC_DDR4_ZEN_MRD_PDA_COMM_CODE_FR RSC_DDR4_ZEN_MRD_PDA_COMM_CODE_EN
#define RSC_DDR4_ZEN_MOD_PDA_COMM_CODE_FR RSC_DDR4_ZEN_MOD_PDA_COMM_CODE_EN
#define RSC_DDR4_ZEN_WRMPR_COMM_CODE_FR RSC_DDR4_ZEN_WRMPR_COMM_CODE_EN
#define RSC_DDR4_ZEN_STAG_COMM_CODE_FR	RSC_DDR4_ZEN_STAG_COMM_CODE_EN
#define RSC_DDR4_ZEN_PDM_COMM_CODE_FR	RSC_DDR4_ZEN_PDM_COMM_CODE_EN
#define RSC_DDR4_ZEN_RDDATA_COMM_CODE_FR RSC_DDR4_ZEN_RDDATA_COMM_CODE_EN
#define RSC_DDR4_ZEN_PHYWRD_COMM_CODE_FR RSC_DDR4_ZEN_PHYWRD_COMM_CODE_EN
#define RSC_DDR4_ZEN_PHYWRL_COMM_CODE_FR RSC_DDR4_ZEN_PHYWRL_COMM_CODE_EN
#define RSC_DDR4_ZEN_PHYRDL_COMM_CODE_FR RSC_DDR4_ZEN_PHYRDL_COMM_CODE_EN
#define RSC_DDR5_ZEN_RFC_SB_COMM_CODE_FR RSC_DDR5_ZEN_RFC_SB_COMM_CODE_EN

#define RSC_TASKS_SORTBY_STATE_CODE_FR		" ""\x89""tat     "
#define RSC_TASKS_SORTBY_RTIME_CODE_FR		" RunTime  "
#define RSC_TASKS_SORTBY_UTIME_CODE_FR		" UserTime "
#define RSC_TASKS_SORTBY_STIME_CODE_FR		" SysTime  "
#define RSC_TASKS_SORTBY_PID_CODE_FR		" PID      "
#define RSC_TASKS_SORTBY_COMM_CODE_FR		" Commande "

#define RSC_MENU_ITEM_MENU_CODE_FR        "     [F2] Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_FR        "     [F3] Vue           "
#define RSC_MENU_ITEM_WINDOW_CODE_FR      "    [F4] Fen""\xaa""tre        "
#define RSC_MENU_ITEM_SPACER_CODE_FR      RSC_MENU_ITEM_SPACER_CODE_EN
#define RSC_MENU_ITEM_DATE_TIME_CODE_FR   RSC_MENU_ITEM_DATE_TIME_CODE_EN
#define RSC_MENU_ITEM_FULL_TIME_CODE_FR   RSC_MENU_ITEM_FULL_TIME_CODE_EN
#define RSC_MENU_ITEM_TINY_TIME_CODE_FR   RSC_MENU_ITEM_TINY_TIME_CODE_EN
#define RSC_MENU_ITEM_SETTINGS_CODE_FR    " R""\xa9""glages           [s] "
#define RSC_MENU_ITEM_SMBIOS_CODE_FR      " Infos SMBIOS       [B] "
#define RSC_MENU_ITEM_KERNEL_CODE_FR      " Infos Noyau        [k] "
#define RSC_MENU_ITEM_HOTPLUG_CODE_FR     " HotPlug CPU        [#] "
#define RSC_MENU_ITEM_TOOLS_CODE_FR       " Outils             [O] "
#define RSC_MENU_ITEM_THEME_CODE_FR       " Th""\xa8""me              [E] "
#define RSC_MENU_ITEM_ABOUT_CODE_FR       " ""\x80""-propos           [a] "
#define RSC_MENU_ITEM_HELP_CODE_FR        " Aide               [h] "
#define RSC_MENU_ITEM_KEYS_CODE_FR        " Raccourcis        [F1] "
#define RSC_MENU_ITEM_LANG_CODE_FR        " Langues            [L] "
#define RSC_MENU_ITEM_QUIT_CODE_FR        " Quitter     [Ctrl]+[x] "
#define RSC_MENU_ITEM_DASHBOARD_CODE_FR   " Tableau de bord    [d] "
#define RSC_MENU_ITEM_FREQUENCY_CODE_FR   " Fr""\xa9""quence          [f] "
#define RSC_MENU_ITEM_INST_CYCLES_CODE_FR " Cycles Instruction [i] "
#define RSC_MENU_ITEM_CORE_CYCLES_CODE_FR " Cycles des Coeurs  [c] "
#define RSC_MENU_ITEM_IDLE_STATES_CODE_FR " ""\x89""tats de sommeil   [l] "
#define RSC_MENU_ITEM_PKG_CYCLES_CODE_FR  " Cycles du Package  [g] "
#define RSC_MENU_ITEM_TASKS_MON_CODE_FR   " Suivi des t""\xa2""ches   [x] "
#define RSC_MENU_ITEM_SYS_INTER_CODE_FR   " Interruptions      [q] "
#define RSC_MENU_ITEM_SENSORS_CODE_FR     " Capteurs           [C] "
#define RSC_MENU_ITEM_VOLTAGE_CODE_FR     "   Voltage          [V] "
#define RSC_MENU_ITEM_POWER_CODE_FR       "   Puissance        [W] "
#define RSC_MENU_ITEM_SLICE_CTRS_CODE_FR  " Compteurs tranche  [T] "
#define RSC_MENU_ITEM_CUSTOM_CODE_FR	  " Vue personnalis""\xa9""e  [y] "
#define RSC_MENU_ITEM_PROCESSOR_CODE_FR   " Processeur         [p] "
#define RSC_MENU_ITEM_TOPOLOGY_CODE_FR    " Topologie          [m] "
#define RSC_MENU_ITEM_FEATURES_CODE_FR    " Caract""\xa9""ristiques   [e] "
#define RSC_MENU_ITEM_ISA_EXT_CODE_FR     " Jeu Instructions   [I] "
#define RSC_MENU_ITEM_TECH_CODE_FR        " Technologies       [t] "
#define RSC_MENU_ITEM_PERF_MON_CODE_FR    " Gestion de Perf.   [o] "
#define RSC_MENU_ITEM_PERF_CAPS_CODE_FR   " Capacit""\xa9""s de Perf. [z] "
#define RSC_MENU_ITEM_POW_THERM_CODE_FR   " Puissance-Therm.   [w] "
#define RSC_MENU_ITEM_SYS_REGS_CODE_FR    " Registres Syst""\xa8""me  [R] "
#define RSC_MENU_ITEM_MEM_CTRL_CODE_FR    " Contr""\xb4""leur " 	\
					  "M""\xa9""moire [M] "

#define RSC_MENU_ITEM_EVENTS_CODE_FR	  " ""\x89""v""\xa8"	\
					  "nements CPU     [H] "

#define RSC_SETTINGS_TITLE_CODE_FR	" R""\xa9""glages "

#define RSC_SETTINGS_DAEMON_CODE_FR	" Acc""\xa8""s d""\xa9""mon" \
					"                    "

#define RSC_SETTINGS_INTERVAL_CODE_FR	" Intervalle(ms)          <    > "
#define RSC_SETTINGS_SYS_TICK_CODE_FR	" Sys. Tick(ms)                  "
#define RSC_SETTINGS_POLL_WAIT_CODE_FR	" Poll Wait(ms)                  "
#define RSC_SETTINGS_RING_WAIT_CODE_FR	" Ring Wait(ms)                  "
#define RSC_SETTINGS_CHILD_WAIT_CODE_FR " Child Wait(ms)                 "
#define RSC_SETTINGS_SLICE_WAIT_CODE_FR " Slice Wait(ms)                 "
#define RSC_SETTINGS_RECORDER_CODE_FR  " Dur""\xa9""e(sec)              <    > "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_FR " Auto Clock               <   > "

#define RSC_SETTINGS_EXPERIMENTAL_CODE_FR " Exp""\xa9""rimental"	\
					  "             <   > "

#define RSC_SETTINGS_CPU_HOTPLUG_CODE_FR    " Hot-Plug CPU             [   ] "
#define RSC_SETTINGS_PCI_ENABLED_CODE_FR    " Activation PCI           [   ] "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_FR " Activation NMI           <   > "
#define RSC_SETTINGS_CPUIDLE_REGISTERED_CODE_FR \
					    " Pilote CPU-IDLE          <   > "

#define RSC_SETTINGS_CPUFREQ_REGISTERED_CODE_FR \
					    " Pilote CPU-FREQ          <   > "

#define RSC_SETTINGS_GOVERNOR_REGISTERED_CODE_FR \
					    " Gouverneur CPU-FREQ      <   > "

#define RSC_SETTINGS_CS_REGISTERED_CODE_FR  " Source d'Horloge         <   > "
#define RSC_SETTINGS_THERMAL_SCOPE_CODE_FR  " Capteur thermique       <    > "
#define RSC_SETTINGS_VOLTAGE_SCOPE_CODE_FR  " Capteur de tension      <    > "
#define RSC_SETTINGS_POWER_SCOPE_CODE_FR    " Capteur de puissance    <    > "
#define RSC_SETTINGS_IDLE_ROUTE_CODE_FR     " Route CPU-IDLE                 "

#define RSC_SETTINGS_ROUTE_TITLE_CODE_FR "Route"

#define RSC_HELP_TITLE_CODE_FR		" Aide "
#define RSC_HELP_KEY_ESCAPE_CODE_FR	" [""\x89""chap]          "
#define RSC_HELP_KEY_SHIFT_TAB_CODE_FR	" [Maj]+[Tab]      "
#define RSC_HELP_KEY_TAB_CODE_FR	" [Tab]            "
#define RSC_HELP_KEY_UP_CODE_FR 	"      [Haut]      "
#define RSC_HELP_KEY_LEFT_RIGHT_CODE_FR " [Gauche] [Droite]"
#define RSC_HELP_KEY_DOWN_CODE_FR	"      [Bas]       "
#define RSC_HELP_KEY_END_CODE_FR	" [Fin]            "
#define RSC_HELP_KEY_HOME_CODE_FR	" [D""\xa9""but]          "
#define RSC_HELP_KEY_ENTER_CODE_FR	" [Entr""\xa9""e]         "
#define RSC_HELP_KEY_PAGE_UP_CODE_FR	" [Page-Pr""\xa9""c]      "
#define RSC_HELP_KEY_PAGE_DOWN_CODE_FR	" [Page-Suiv]      "
#define RSC_HELP_KEY_MINUS_CODE_FR	" [Moins]          "
#define RSC_HELP_KEY_PLUS_CODE_FR	" [Plus]           "
#define RSC_HELP_KEY_MENU_CODE_FR	" [F2]             "
#define RSC_HELP_MENU_CODE_FR		"             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_FR	"   Fermer fen""\xaa""tre "
#define RSC_HELP_PREV_WINDOW_CODE_FR	"  Fen""\xaa""tre arri""\xa8""re "
#define RSC_HELP_NEXT_WINDOW_CODE_FR	" Fen""\xaa""tre suivante "
#define RSC_HELP_KEY_SHIFT_GR1_CODE_FR	"        [s]       "
#define RSC_HELP_KEY_SHIFT_GR2_CODE_FR	" [a|q]  [x]  [d] +"
#define RSC_HELP_MOVE_WINDOW_CODE_FR	" [Maj]   D""\xa9""placer "
#define RSC_HELP_KEY_ALT_GR3_CODE_FR	"                 +"
#define RSC_HELP_SIZE_WINDOW_CODE_FR	" [Alt]  Retailler "
#define RSC_HELP_MOVE_SELECT_CODE_FR	" D""\xa9""placer curseur "
#define RSC_HELP_LAST_CELL_CODE_FR	" Derni""\xa8""re cellule "
#define RSC_HELP_FIRST_CELL_CODE_FR	" Premi""\xa8""re cellule "
#define RSC_HELP_TRIGGER_SELECT_CODE_FR " Ex""\xa9""cuter cellule "
#define RSC_HELP_PREV_PAGE_CODE_FR	"  Page pr""\xa9""c""\xa9""dente "
#define RSC_HELP_NEXT_PAGE_CODE_FR	"    Page suivante "
#define RSC_HELP_SCROLL_DOWN_CODE_FR	"    D""\xa9""cro""\xae""tre CPU "
#define RSC_HELP_SCROLL_UP_CODE_FR	"    Accro""\xae""tre CPU "

#define RSC_ADV_HELP_TITLE_CODE_FR	" Raccourcis "

#define RSC_ADV_HELP_SECT_FREQ_CODE_FR	" Vue Fr""\xa9""quence:"	\
					"                       "

#define RSC_ADV_HELP_ITEM_AVG_CODE_FR	" %           Moyennes | "	\
					"\x89""tats-Package "

#define RSC_ADV_HELP_SECT_TASK_CODE_FR	" Vue Suivi des t""\xa2""ches:" \
					"                "

#define RSC_ADV_HELP_ITEM_ORDER_CODE_FR " b          Crit""\xa8""re de tri" \
					" des t""\xa2""ches "

#define RSC_ADV_HELP_ITEM_RST_CODE_FR	" N          Effacer la t""\xa2""che " \
					"\xa0"" suivre "

#define RSC_ADV_HELP_ITEM_SEL_CODE_FR	" n          Choisir la t""\xa2""che " \
					"\xa0"" suivre "

#define RSC_ADV_HELP_ITEM_REV_CODE_FR	" r         Inverser le tri des " \
					"t""\xa2""ches "

#define RSC_ADV_HELP_ITEM_HIDE_CODE_FR	" v          Afficher | Cacher valeurs "
#define RSC_ADV_HELP_SECT_ANY_CODE_FR	" Vue quelconque:                      "

#define RSC_ADV_HELP_ITEM_POWER_CODE_FR " $            ""\x89""nergie " \
					"en Joule | Watt "

#define RSC_ADV_HELP_ITEM_TOP_CODE_FR	" .             Fr""\xa9""quence Top" \
					" ou Usage "

#define RSC_ADV_HELP_ITEM_UPD_CODE_FR	" *                Rafra""\xae""chir " \
					"CoreFreq "

#define RSC_ADV_HELP_ITEM_START_CODE_FR " {                  D""\xa9""marrer "\
					"CoreFreq "

#define RSC_ADV_HELP_ITEM_STOP_CODE_FR	" }                   Arr""\xaa""ter "\
					"CoreFreq "

#define RSC_ADV_HELP_ITEM_TOOLS_CODE_FR " F10               Arr""\xaa""ter " \
					"les outils "

#define RSC_ADV_HELP_ITEM_GO_UP_CODE_FR " Haut Prec                 "	\
					"D""\xa9""filement "

#define RSC_ADV_HELP_ITEM_GO_DW_CODE_FR " Bas  Suiv                       CPU  "
#define RSC_ADV_HELP_ITEM_TERMINAL_CODE_FR \
					" Terminal:                            "

#define RSC_ADV_HELP_ITEM_PRT_SCR_CODE_FR \
					" [Ctrl]+[p]                Screenshot "

#define RSC_ADV_HELP_ITEM_REC_SCR_CODE_FR \
					" [Alt]+[p]                 Screencast "

#define RSC_ADV_HELP_ITEM_FAHR_CELS_CODE_FR \
					" F              Fahrenheit ou Celsius "

#define RSC_ADV_HELP_ITEM_SYSGATE_CODE_FR \
					" G            Basculer l'""\xa9""tat"\
					" SysGate "

#define RSC_ADV_HELP_ITEM_PROC_EVENT_CODE_FR \
					" H           G""\xa9""rer Alertes" \
					" Processeur "

#define RSC_ADV_HELP_ITEM_SECRET_CODE_FR \
					" Y          Basculer donn""\xa9""es" \
					" secr""\xa8""tes "

#define RSC_TURBO_CLOCK_TITLE_CODE_FR	" Fr""\xa9""q. Turbo %1dC "
#define RSC_RATIO_CLOCK_TITLE_CODE_FR	" %s Ratio Fr""\xa9""q. "
#define RSC_UNCORE_CLOCK_TITLE_CODE_FR	" %s Uncore Fr""\xa9""q. "

#define RSC_SELECT_CPU_TITLE_CODE_FR	" CPU   Pkg  Core Thread Planifier "

#define RSC_SELECT_FREQ_TITLE_CODE_FR	" CPU   Pkg  Core Thread "	\
					"  Fr""\xa9""quence   Ratio "

#define RSC_BOX_DISABLE_COND0_CODE_FR	"             D""\xa9""sactiver  " \
					"           "

#define RSC_BOX_DISABLE_COND1_CODE_FR	"           < D""\xa9""sactiver >" \
					"           "

#define RSC_BOX_ENABLE_COND0_CODE_FR	"               Activer              "
#define RSC_BOX_ENABLE_COND1_CODE_FR	"           <   Activer  >           "

#define RSC_BOX_INTERVAL_TITLE_CODE_FR	"Intervalle"
#define RSC_BOX_AUTO_CLOCK_TITLE_CODE_FR " Auto Clock "
#define RSC_BOX_MODE_TITLE_CODE_FR	" Exp""\xa9""rimental "

#define RSC_BOX_MODE_DESC_CODE_FR	"     CoreFreq mode "	\
					"op""\xa9""rationnel     "

#define RSC_BOX_HWP_DESC_CODE_FR	" Contr""\xb4""le Mat""\xa9""riel" \
					" de la Performance"

#define RSC_BOX_FMW_DESC_CODE_FR	" Contr""\xb4""le Firmware"	\
					" de la Performance"

#define RSC_BOX_NOMINAL_MODE_COND0_CODE_FR \
					"       Fonctionnement" \
					" nominal       "

#define RSC_BOX_NOMINAL_MODE_COND1_CODE_FR \
					"     < Fonctionnement" \
					" nominal >     "

#define RSC_BOX_EXPERIMENT_MODE_COND0_CODE_FR \
					"     Fonctionnement "	\
					"exp""\xa9""rimental    "

#define RSC_BOX_EXPERIMENT_MODE_COND1_CODE_FR \
					"   < Fonctionnement "	\
					"exp""\xa9""rimental >  "

#define RSC_BOX_INTERRUPT_TITLE_CODE_FR 	" Interruptions NMI "
#define RSC_BOX_CPU_IDLE_TITLE_CODE_FR		" Pilote CPU-IDLE "
#define RSC_BOX_CPU_FREQ_TITLE_CODE_FR		" Pilote CPU-FREQ "
#define RSC_BOX_GOVERNOR_TITLE_CODE_FR		" Gouverneur CPU-FREQ "
#define RSC_BOX_CLOCK_SOURCE_TITLE_CODE_FR	" Source d'Horloge "

#define RSC_BOX_OPS_REGISTER_COND0_CODE_FR	"             " \
						"Enregistrer            "

#define RSC_BOX_OPS_REGISTER_COND1_CODE_FR	"         <   " \
						"Enregistrer  >         "

#define RSC_BOX_OPS_UNREGISTER_COND0_CODE_FR	"           D""\xa9""s" \
						"enregistrer           "

#define RSC_BOX_OPS_UNREGISTER_COND1_CODE_FR	"         < D""\xa9""s" \
						"enregistrer >         "

#define RSC_BOX_EVENT_TITLE_CODE_FR		RSC_BOX_EVENT_TITLE_CODE_EN

#define RSC_BOX_EVENT_SPACE_CODE_FR		"                 "
#define RSC_BOX_EVENT_THERMAL_SENSOR_CODE_FR	" Capteur therm.  "
#define RSC_BOX_EVENT_PROCHOT_STS_CODE_FR	" PROCHOT         "
#define RSC_BOX_EVENT_CRITICAL_TEMP_CODE_FR	" Temp. critique  "
#define RSC_BOX_EVENT_THOLD1_STS_CODE_FR	" Seuil1 therm.   "
#define RSC_BOX_EVENT_THOLD2_STS_CODE_FR	" Seuil2 therm.   "
#define RSC_BOX_EVENT_POWER_LIMIT_CODE_FR	" Puissance max.  "
#define RSC_BOX_EVENT_CURRENT_LIMIT_CODE_FR	" Courant limit""\xa9""  "
#define RSC_BOX_EVENT_CROSS_DOM_LIMIT_CODE_FR	" Interdomaine    "
#define RSC_BOX_EVENT_RESIDENCY_CODE_FR 	" R""\xa9""sidence       "
#define RSC_BOX_EVENT_AVG_THERMAL_CODE_FR	" Moy. thermique  "
#define RSC_BOX_EVENT_VR_THERMAL_CODE_FR	" VR thermique    "
#define RSC_BOX_EVENT_VR_TDC_CODE_FR		" VR TDC          "
#define RSC_BOX_EVENT_POWER_PL1_CODE_FR 	" PL1 du Package  "
#define RSC_BOX_EVENT_POWER_PL2_CODE_FR 	" PL2 du Package  "
#define RSC_BOX_EVENT_ELECTRICAL_CODE_FR	" EDP ""\xa9""lectrique  "
#define RSC_BOX_EVENT_INEFFICIENCY_CODE_FR	" Inefficacit""\xa9""    "
#define RSC_BOX_EVENT_MAX_TURBO_CODE_FR 	" Turbo Max       "
#define RSC_BOX_EVENT_TURBO_ATTEN_CODE_FR	" Turbo Atten.    "
#define RSC_BOX_EVENT_THERMAL_TVB_CODE_FR	" TVB thermique   "
#define RSC_BOX_EVENT_ALL_OF_THEM_CODE_FR	"<  Tout effacer >"

#define RSC_BOX_DUTY_CYCLE_RESERVED_CODE_FR	"           "		\
						"R""\xa9""serv""\xa9""         "

#define RSC_BOX_POWER_POLICY_TITLE_CODE_FR	" R""\xa8""gle "	\
						"\xa9""nerg""\xa9""tique "

#define RSC_BOX_POWER_POLICY_LOW_CODE_FR	"            0       BAS "
#define RSC_BOX_POWER_POLICY_HIGH_CODE_FR	"           15      HAUT "

#define RSC_BOX_HWP_POLICY_MIN_CODE_FR		"         Minimale:0     "
#define RSC_BOX_HWP_POLICY_020_CODE_FR		"                 32     "
#define RSC_BOX_HWP_POLICY_040_CODE_FR		"                 64     "
#define RSC_BOX_HWP_POLICY_060_CODE_FR		"                 96     "
#define RSC_BOX_HWP_POLICY_MED_CODE_FR		"        Moyenne:128     "
#define RSC_BOX_HWP_POLICY_0A0_CODE_FR		"                160     "
#define RSC_BOX_HWP_POLICY_PWR_CODE_FR		"      Puissance:192     "
#define RSC_BOX_HWP_POLICY_0E0_CODE_FR		"                224     "
#define RSC_BOX_HWP_POLICY_MAX_CODE_FR		"       Maximale:255     "

#define RSC_BOX_TOOLS_TITLE_CODE_FR	    " Outils "
#define RSC_BOX_TOOLS_STOP_BURN_CODE_FR     "          ARR""\x8a""TER          "
#define RSC_BOX_TOOLS_ATOMIC_BURN_CODE_FR   "       Stress Atomic       "
#define RSC_BOX_TOOLS_CRC32_BURN_CODE_FR    "        Calcul CRC32       "
#define RSC_BOX_TOOLS_CONIC_BURN_CODE_FR    "   <   Calcul Conique   >  "
#define RSC_BOX_TOOLS_RANDOM_CPU_CODE_FR    "    Turbo CPU al""\xa9""atoire    "
#define RSC_BOX_TOOLS_ROUND_ROBIN_CPU_CODE_FR \
					    "    Turbo CPU circulaire   "
#define RSC_BOX_TOOLS_USER_CPU_CODE_FR	    "    Turbo < Choisir CPU >  "
#define RSC_BOX_TOOLS_MONTE_CARLO_CODE_FR   "   Calcul PI / Monte Carlo "

#define RSC_BOX_CONIC_TITLE_CODE_FR	" Variations Coniques "
#define RSC_BOX_CONIC_ITEM_1_CODE_FR	"         Ellipso""\xaf""de         "

#define RSC_BOX_CONIC_ITEM_2_CODE_FR	"  Hyperbolo""\xaf""de "\
					"\xa0"" une nappe  "

#define RSC_BOX_CONIC_ITEM_3_CODE_FR	" Hyperbolo""\xaf""de "\
					"\xa0"" deux nappes "

#define RSC_BOX_CONIC_ITEM_4_CODE_FR	"     Cylindre elliptique    "
#define RSC_BOX_CONIC_ITEM_5_CODE_FR	"     Cylindre hyperbolique  "
#define RSC_BOX_CONIC_ITEM_6_CODE_FR	"    Deux plans parall""\xa8""les   "

#define RSC_BOX_LANG_TITLE_CODE_FR		" Langues "
#define RSC_BOX_LANG_ENGLISH_CODE_FR		"     Anglais     "
#define RSC_BOX_LANG_FRENCH_CODE_FR		"     Fran""\xa7""ais    "

#define RSC_BOX_THEME_TITLE_CODE_FR		" Th""\xa8""mes "

#define RSC_BOX_SCOPE_THERMAL_TITLE_CODE_FR	" Temp""\xa9""rature "
#define RSC_BOX_SCOPE_VOLTAGE_TITLE_CODE_FR	" Tension "
#define RSC_BOX_SCOPE_POWER_TITLE_CODE_FR	" Puissance "
#define RSC_BOX_SCOPE_NONE_CODE_FR		"       Sans       "
#define RSC_BOX_SCOPE_THREAD_CODE_FR		"      Thread      "
#define RSC_BOX_SCOPE_CORE_CODE_FR		"       Coeur      "
#define RSC_BOX_SCOPE_PACKAGE_CODE_FR		"      Package     "

#define RSC_BOX_CFG_TDP_TITLE_CODE_FR		" Config TDP "
#define RSC_BOX_CFG_TDP_DESC_CODE_FR		"    Limite BIOS    "
#define RSC_BOX_CFG_TDP_LVL0_CODE_FR		"     Niveau 0      "
#define RSC_BOX_CFG_TDP_LVL1_CODE_FR		"     Niveau 1      "
#define RSC_BOX_CFG_TDP_LVL2_CODE_FR		"     Niveau 2      "

#define RSC_BOX_TDP_PKG_TITLE_CODE_FR		" TDP Package "
#define RSC_BOX_TDP_CORES_TITLE_CODE_FR 	" TDP Core "
#define RSC_BOX_TDP_UNCORE_TITLE_CODE_FR	" TDP Uncore "
#define RSC_BOX_TDP_RAM_TITLE_CODE_FR		" TDP DRAM "
#define RSC_BOX_TDP_PLATFORM_TITLE_CODE_FR	" TDP Plateforme "
#define RSC_BOX_PL1_DESC_CODE_FR	"      Limite de puissance PL1       "
#define RSC_BOX_PL2_DESC_CODE_FR	"      Limite de puissance PL2       "

#define RSC_BOX_TDC_TITLE_CODE_FR		" TDC du courant "
#define RSC_BOX_TDC_DESC_CODE_FR	"       Limite de courant TDC        "

#define RSC_ERROR_CMD_SYNTAX_CODE_FR					\
		"CoreFreq."						\
		"  Copyright (C) 2015-2025 CYRIL COURTIAT\n\n"	\
		"Usage:\t%s [-Option <argument>] [-Commande <argument>]\n"\
		"\n    Options de l'interface\n"			\
		"\t-Oa\tFréquence absolue\n"				\
		"\t-Op\tAfficher États-Package\n"			\
		"\t-Ok\tUnité mémoire en kilo-octet\n"			\
		"\t-Om\tUnité mémoire en méga-octet\n"			\
		"\t-Og\tUnité mémoire en giga-octet\n"			\
		"\t-OW\tChanger d'unité d'Énergie\n"			\
		"\t-OF\tTempérature en Fahrenheit\n"			\
		"\t-OJ #\tNuméro d'index de chaîne SMBIOS\n"		\
		"\t-OE #\tNuméro d'index du thème de couleurs\n"	\
		"\t-OY\tAfficher les données secrètes\n"		\
		"\n    Options de commande\n"				\
		"\t-t <v>\tAfficher Top (par défault); en option, la <v>ue:\n"\
		"\t\t{\tfrequency, instructions, core, idle, package, tasks,\n"\
		"\t\t\tinterrupts, sensors, voltage, power, slices, custom }\n"\
		"\t-d\tAfficher le tableau de bord\n"			\
		"\t-C <#>\tMoniteur des Capteurs\n"			\
		"\t-V <#>\tMoniteur de Voltage\n"			\
		"\t-W <#>\tMoniteur de Puissance\n"			\
		"\t-g <#>\tMoniteur du Package\n"			\
		"\t-c <#>\tMoniteur des Compteurs\n"			\
		"\t-i <#>\tMoniteur des Instructions\n" 		\
		"\t-s\tImprimer les Informations du système\n"		\
		"\t-j\tImprimer les Informations (format json)\n"	\
		"\t-z\tImprimer les Capacités de performances\n"	\
		"\t-M\tImprimer le Controlleur mémoire\n"		\
		"\t-R\tImprimer les Registres du système\n"		\
		"\t-m\tImprimer la Topologie\n" 			\
		"\t-B\tImprimer SMBIOS\n"				\
		"\t-k\tImprimer le Kernel\n"				\
		"\t-n\tNouvelle ligne\n"				\
		"\t-h\tAfficher ce message\n"				\
		"\t-v\tAfficher le numéro de version\n"			\
		"\nCode d'exécution:\n"					\
		"\t%u\tSUCCESS\t\tExécution réussie\n"			\
		"\t%u\tCMD_SYNTAX\tErreur de syntaxe de la commande\n"	\
		"\t%u\tSHM_FILE\tErreur du fichier de la mémoire partagée\n"\
		"\t%u\tSHM_MMAP\tErreur de mappage de la mémoire partagée\n"\
		"\t%u\tPERM_ERR\tExécution non autorisée\n"		\
		"\t%u\tMEM_ERR\t\tErreur de fonctionnement de la mémoire\n"\
		"\t%u\tEXEC_ERR\tErreur d'exécution générale\n"		\
		"\t%u\tSYS_CALL\tErreur d'appel système\n"		\
		"\nSignaler toutes anomalies à labs[at]cyring[.]fr\n"

#define RSC_ERROR_SHARED_MEM_CODE_FR					\
	"Erreur code %d de connexion au démon.\n%s: '%s' @ ligne %d\n"

#define RSC_ERROR_SYS_CALL_CODE_FR     "Erreur Système code %d\n%s @ ligne %d\n"

#define RSC_ERROR_UNIMPLEMENTED_CODE_FR "Fonctionnalit""\xa9"		\
					" non implement""\xa9""e"

#define RSC_ERROR_EXPERIMENTAL_CODE_FR	"Le mode exp""\xa9""rimental est requis"

#define RSC_ERROR_TURBO_PREREQ_CODE_FR	"D""\xa9""sactiver Turbo Boost |" \
					" Core Performance Boost"

#define RSC_ERROR_UNCORE_PREREQ_CODE_FR "Pr""\xa9""requis Uncore"	\
					" non valid""\xa9""s"

#define RSC_ERROR_PSTATE_NOT_FOUND_CODE_FR "Ce P-State de Fr\xa9quence " \
					"n'a pas \xa9t\xa9 trouv\xa9"

#define RSC_ERROR_CLOCKSOURCE_CODE_FR	"Source d'horloge incompatible"

#define RSC_BOX_IDLE_LIMIT_TITLE_CODE_FR " Limite CPU-Idle "

#define RSC_BOX_RECORDER_TITLE_CODE_FR	" Dur""\xa9""e "

#define RSC_SMBIOS_TITLE_CODE_FR	" SMBIOS "

#define RSC_MECH_CLRBHB_CODE_FR 	"Effacement de l'historique de branche"

#define RSC_MECH_SSBD_CODE_FR		\
	"Contournement Sp""\xa9""culatif de Magasin d""\xa9""sactiv""\xa9"

#define RSC_MECH_SSBS_CODE_FR		\
	"Contournement Sp""\xa9""culatif de Magasin Prot""\xa9""g""\xa9"

#define RSC_CREATE_SELECT_AUTO_TURBO_CODE_FR	"  %3s       Processeur    " \
						"   %s     %c%4u %c "

#define RSC_CREATE_SELECT_FREQ_TURBO_CODE_FR	"  %3s       Processeur    " \
						"%7.2f MHz %c%4u %c "

#define RSC_CREATE_SELECT_FREQ_TGT_CODE_FR	"  TGT       Processeur    "
#define RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_FR	"  HWP-TGT   Processeur    "
#define RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_FR	"  HWP-MAX   Processeur    "
#define RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_FR	"  HWP-MIN   Processeur    "
#define RSC_CREATE_SELECT_FREQ_MAX_CODE_FR	"  MAX       Processeur    "
#define RSC_CREATE_SELECT_FREQ_MIN_CODE_FR	"  MIN       Processeur    "

#define RSC_CREATE_SELECT_FREQ_OFFLINE_CODE_FR	"  %03u                    " \
						"               Off   "

#define RSC_POPUP_DRIVER_TITLE_CODE_FR		" Message du Pilote "

#define RSC_EXIT_TITLE_CODE_FR			" Sortie "

#define RSC_EXIT_HEADER_CODE_FR  " Les services suivants sont toujours " \
				 "en cours d'ex""\xa9""cution "

#define RSC_EXIT_CONFIRM_CODE_FR "                       < Confirmer > " \
				 "                     "

#define RSC_EXIT_FOOTER_CODE_FR  "                                     " \
				 "[""\x89""chap] pour annuler "

#define RSC_CREATE_SETTINGS_COND0_CODE_FR RSC_CREATE_SETTINGS_COND0_CODE_EN
#define RSC_CREATE_SETTINGS_COND1_CODE_FR RSC_CREATE_SETTINGS_COND1_CODE_EN

#define RSC_CREATE_ADV_HELP_COND0_CODE_FR RSC_CREATE_ADV_HELP_COND0_CODE_EN
#define RSC_CREATE_ADV_HELP_COND1_CODE_FR RSC_CREATE_ADV_HELP_BLANK_CODE_EN
