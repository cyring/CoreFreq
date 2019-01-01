/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
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
	"------------ Cycles ----  Etat -------------------- TSC Rati"	\
	"o ----------------------------------------------------------"	\
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
	'O','F','F',' ',']',' '						\
}

#define RSC_LAYOUT_RULLER_VOLTAGE_CODE_FR				\
	"--- Freq(MHz) - VID - Vcore ----------------- Energie(J) - P"	\
	"uissance(W) ------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_FR				\
{									\
	'T','a','c','h','e','s','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ','K','B',']' 	\
}

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
#define RSC_LEVEL_CODE_FR		(ASCII*) "Niveau"
#define RSC_PROGRAMMABLE_CODE_FR	(ASCII*) "Programmable"
#define RSC_CONFIGURATION_CODE_FR	(ASCII*) "Configuration"
#define RSC_TURBO_ACTIVATION_CODE_FR	(ASCII*) "Activation Turbo"
#define RSC_NOMINAL_CODE_FR		(ASCII*) "Nominal"
#define RSC_UNLOCK_CODE_FR		(ASCII*) "OUVERT"
#define RSC_LOCK_CODE_FR		(ASCII*) "BLOQUE"

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

#define RSC_TECHNOLOGIES_TITLE_CODE_FR	(ASCII*) " Technologies "

#define RSC_PERF_MON_TITLE_CODE_FR	(ASCII*) " Gestion de la performance "
#define RSC_VERSION_CODE_FR		(ASCII*) "Version"
#define RSC_COUNTERS_CODE_FR		(ASCII*) "Compteurs"
#define RSC_GENERAL_CTRS_CODE_FR	(ASCII*) "Generaux"
#define RSC_FIXED_CTRS_CODE_FR		(ASCII*) "Fixes"

#define RSC_POWER_THERMAL_TITLE_CODE_FR (ASCII*) " Puissance et thermique "

#define RSC_KERNEL_TITLE_CODE_FR	(ASCII*) " Noyau "

#define RSC_TOPOLOGY_TITLE_CODE_FR	(ASCII*) " Topologie "

#define RSC_MEM_CTRL_TITLE_CODE_FR	(ASCII*) " Controleur Memoire "

#define RSC_MENU_ITEM_MENU_CODE_FR	(ASCII*) "          Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_FR	(ASCII*) "           Vue          "
#define RSC_MENU_ITEM_WINDOW_CODE_FR	(ASCII*) "         Fenetre        "
#define RSC_MENU_ITEM_SETTINGS_CODE_FR	(ASCII*) " Reglages           [s] "
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
#define RSC_MENU_ITEM_CORE_CYCLE_CODE_FR (ASCII*)" Cycles des Cores   [c] "
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
#define RSC_HELP_TITLE_CODE_FR		(ASCII*) " Aide "
#define RSC_ADV_HELP_TITLE_CODE_FR	(ASCII*) " Raccourcis "
