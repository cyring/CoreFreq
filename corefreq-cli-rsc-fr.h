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
#define RSC_LEVEL_CODE_FR		(ASCII*) "Niveau"
#define RSC_PROGRAMMABLE_CODE_FR	(ASCII*) "Programmable"
#define RSC_CONFIGURATION_CODE_FR	(ASCII*) "Configuration"
#define RSC_TURBO_ACTIVATION_CODE_FR	(ASCII*) "Activation Turbo"
#define RSC_NOMINAL_CODE_FR		(ASCII*) "Nominal"
#define RSC_UNLOCK_CODE_FR		(ASCII*) "OUVERT"
#define RSC_LOCK_CODE_FR		(ASCII*) "BLOQUE"
#define RSC_ENABLE_CODE_FR		(ASCII*) "  Actif"
#define RSC_DISABLE_CODE_FR		(ASCII*) "Inactif"

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

#define RSC_POWER_THERMAL_TITLE_CODE_FR (ASCII*) " Puissance et thermique "
#define RSC_POWER_THERMAL_ODCM_CODE_FR	(ASCII*) "Modulation d'horloge"
#define RSC_POWER_THERMAL_DUTY_CODE_FR	(ASCII*) "Cycle de service"
#define RSC_POWER_THERMAL_MGMT_CODE_FR	(ASCII*) "Gestion de la puissance"
#define RSC_POWER_THERMAL_BIAS_CODE_FR	(ASCII*) "Regle energitique"
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
#define RSC_KERNEL_IDLE_DRIVER_CODE_FR	(ASCII*) "Pilote d'etats"
#define RSC_KERNEL_RELEASE_CODE_FR	(ASCII*) "Edition"
#define RSC_KERNEL_VERSION_CODE_FR	(ASCII*) "Version"
#define RSC_KERNEL_MACHINE_CODE_FR	(ASCII*) "Machine"
#define RSC_KERNEL_MEMORY_CODE_FR	(ASCII*) "Memoire"
#define RSC_KERNEL_STATE_CODE_FR	(ASCII*) "Etat"
#define RSC_KERNEL_POWER_CODE_FR	(ASCII*) "Puissance"
#define RSC_KERNEL_LATENCY_CODE_FR	(ASCII*) "Latence"
#define RSC_KERNEL_RESIDENCY_CODE_FR	(ASCII*) "Periode"

#define RSC_TOPOLOGY_TITLE_CODE_FR	(ASCII*) " Topologie "

#define RSC_MEM_CTRL_TITLE_CODE_FR	(ASCII*) " Controleur Memoire "

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
				(ASCII*) " Intervalle(ms)                 "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_FR 				\
				(ASCII*) " Auto Clock                     "
#define RSC_SETTINGS_EXPERIMENTAL_CODE_FR				\
				(ASCII*) " Experimental                   "
#define RSC_SETTINGS_CPU_HOTPLUG_CODE_FR				\
				(ASCII*) " Hot-Plug CPU                   "
#define RSC_SETTINGS_PCI_ENABLED_CODE_FR				\
				(ASCII*) " Activation PCI                 "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_FR				\
				(ASCII*) " Activation NMI                 "

#define RSC_HELP_TITLE_CODE_FR		(ASCII*) " Aide "
#define RSC_HELP_MENU_CODE_FR		(ASCII*) "             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_FR	(ASCII*) "   Fermer fenetre "
#define RSC_HELP_PREV_WINDOW_CODE_FR	(ASCII*) "  Fenetre arriere "
#define RSC_HELP_NEXT_WINDOW_CODE_FR	(ASCII*) " Fenetre suivante "
#define RSC_HELP_MOVE_WINDOW_CODE_FR	(ASCII*) " Deplacer fenetre "
#define RSC_HELP_MOVE_SELECT_CODE_FR	(ASCII*) " Deplacer curseur "
#define RSC_HELP_LAST_CELL_CODE_FR	(ASCII*) " Derniere cellule "
#define RSC_HELP_FIRST_CELL_CODE_FR	(ASCII*) " Premiere cellule "
#define RSC_HELP_TRIGGER_SELECT_CODE_FR (ASCII*) " Executer cellule "
#define RSC_HELP_PREV_PAGE_CODE_FR	(ASCII*) "     Page arriere "
#define RSC_HELP_NEXT_PAGE_CODE_FR	(ASCII*) "    Page suivante "
#define RSC_HELP_SCROLL_DOWN_CODE_FR	(ASCII*) "    Decroitre CPU "
#define RSC_HELP_SCROLL_UP_CODE_FR	(ASCII*) "    Accroitre CPU "

#define RSC_ADV_HELP_TITLE_CODE_FR	(ASCII*) " Raccourcis "
#define RSC_ADV_HELP_ITEM_1_CODE_FR					\
			(ASCII*) " Vue Frequence:                       "
#define RSC_ADV_HELP_ITEM_2_CODE_FR					\
			(ASCII*) " %          Moyennes ou Etats-Package "
#define RSC_ADV_HELP_ITEM_3_CODE_FR					\
			(ASCII*) " Vue Suivi des taches:                "
#define RSC_ADV_HELP_ITEM_4_CODE_FR					\
			(ASCII*) " b          Critere de tri des taches "
#define RSC_ADV_HELP_ITEM_5_CODE_FR					\
			(ASCII*) " n          Choisir la tache a suivre "
#define RSC_ADV_HELP_ITEM_6_CODE_FR					\
			(ASCII*) " r         Inverser le tri des taches "
#define RSC_ADV_HELP_ITEM_7_CODE_FR					\
			(ASCII*) " v        Afficher|Cacher les valeurs "
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
			(ASCII*) "  Up  PgUp                 Defilement "
#define RSC_ADV_HELP_ITEM_14_CODE_FR					\
			(ASCII*) " Down PgDw                       CPU  "

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

#define RSC_BOX_TOOLS_TITLE_CODE_FR	(ASCII*) " Outils "
#define RSC_BOX_TOOLS_STOP_CODE_FR	(ASCII*) "          ARRETER          "
#define RSC_BOX_TOOLS_ATOMIC_CODE_FR	(ASCII*) "       Stress Atomic       "
#define RSC_BOX_TOOLS_CRC32_CODE_FR	(ASCII*) "        Calcul CRC32       "
#define RSC_BOX_TOOLS_CONIC_CODE_FR	(ASCII*) "      Calcul Conique...    "
#define RSC_BOX_TOOLS_RAND_CPU_CODE_FR	(ASCII*) "    Turbo CPU aleatoire    "
#define RSC_BOX_TOOLS_RR_CPU_CODE_FR	(ASCII*) "    Turbo CPU circulaire   "

#define RSC_BOX_CONIC_TITLE_CODE_FR	(ASCII*) " Variations Coniques "
#define RSC_BOX_CONIC_ITEM_1_CODE_FR	(ASCII*) "         Ellipsoide         "
#define RSC_BOX_CONIC_ITEM_2_CODE_FR	(ASCII*) "  Hyperboloide a une nappe  "
#define RSC_BOX_CONIC_ITEM_3_CODE_FR	(ASCII*) " Hyperboloide a deux nappes "
#define RSC_BOX_CONIC_ITEM_4_CODE_FR	(ASCII*) "     Cylindre elliptique    "
#define RSC_BOX_CONIC_ITEM_5_CODE_FR	(ASCII*) "     Cylindre hyperbolique  "
#define RSC_BOX_CONIC_ITEM_6_CODE_FR	(ASCII*) "    Deux plans paralleles   "
