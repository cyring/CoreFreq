/*
 * CoreFreq
 * Copyright (C) 2015-2022 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#ifndef MSR_SMI_COUNT
	#define MSR_SMI_COUNT			0x00000034
#endif

#ifndef MSR_IA32_FEAT_CTL
	#define MSR_IA32_FEAT_CTL		MSR_IA32_FEATURE_CONTROL
#endif

#ifndef MSR_PPIN_CTL
	#define MSR_PPIN_CTL			0x0000004e
#endif

#ifndef MSR_PPIN
	#define MSR_PPIN			0x0000004f
#endif

#ifndef MSR_IA32_SPEC_CTRL
	#define MSR_IA32_SPEC_CTRL		0x00000048
#endif

#ifndef MSR_IA32_PRED_CMD
	#define MSR_IA32_PRED_CMD		0x00000049
#endif

#ifndef MSR_IA32_FLUSH_CMD
	#define MSR_IA32_FLUSH_CMD		0x0000010b
#endif

#ifndef MSR_IA32_ARCH_CAPABILITIES
	#define MSR_IA32_ARCH_CAPABILITIES	0x0000010a
#endif

#ifndef MSR_IA32_XAPIC_DISABLE_STATUS
	#define MSR_IA32_XAPIC_DISABLE_STATUS	0x000000bd
#endif

#ifndef MSR_IA32_UARCH_MISC_CTRL
	#define MSR_IA32_UARCH_MISC_CTRL	0x00001b01
#endif

#ifndef MSR_IA32_MCU_OPT_CTRL
	#define MSR_IA32_MCU_OPT_CTRL		0x00000123
#endif

#ifndef MSR_IA32_CORE_CAPABILITIES
	#define MSR_IA32_CORE_CAPABILITIES	0x000000cf
#endif

#ifndef MSR_IA32_TSX_CTRL
	#define MSR_IA32_TSX_CTRL		0x00000122
#endif

#ifndef MSR_PLATFORM_INFO
	#define MSR_PLATFORM_INFO		0x000000ce
#endif

#ifndef MSR_MISC_FEATURE_CONTROL
	#define MSR_MISC_FEATURE_CONTROL	0x000001a4
#endif

#ifndef MSR_MISC_PWR_MGMT
	#define MSR_MISC_PWR_MGMT		0x000001aa
#endif

#ifndef MSR_NHM_TURBO_RATIO_LIMIT
	#define MSR_NHM_TURBO_RATIO_LIMIT	0x000001ad
#endif

#ifndef MSR_TURBO_RATIO_LIMIT
	#define MSR_TURBO_RATIO_LIMIT		MSR_NHM_TURBO_RATIO_LIMIT
#endif

#ifndef MSR_TURBO_RATIO_LIMIT1
	#define MSR_TURBO_RATIO_LIMIT1		0x000001ae
#endif

#ifndef MSR_TURBO_RATIO_LIMIT2
	#define MSR_TURBO_RATIO_LIMIT2		0x000001af
#endif

#ifndef MSR_TURBO_RATIO_LIMIT3
	#define MSR_TURBO_RATIO_LIMIT3		0x000001ac
#endif

#define MSR_SECONDARY_TURBO_RATIO_LIMIT 	0x00000650

#ifndef MSR_TURBO_POWER_CURRENT_LIMIT
	#define MSR_TURBO_POWER_CURRENT_LIMIT	0x000001ac
#endif

#ifndef MSR_IA32_POWER_CTL
	#define MSR_IA32_POWER_CTL		0x000001fc
#endif

#ifndef MSR_PKG_CST_CONFIG_CONTROL
	#define MSR_PKG_CST_CONFIG_CONTROL	MSR_NHM_SNB_PKG_CST_CFG_CTL
#endif

#ifndef MSR_PMG_IO_CAPTURE_BASE
	#define MSR_PMG_IO_CAPTURE_BASE 	0x000000e4
#endif

#ifndef MSR_CC6_DEMOTION_POLICY_CONFIG
	#define MSR_CC6_DEMOTION_POLICY_CONFIG	0x00000668
#endif

#ifndef MSR_MC6_DEMOTION_POLICY_CONFIG
	#define MSR_MC6_DEMOTION_POLICY_CONFIG	0x00000669
#endif

#define MSR_NHM_UNCORE_PERF_GLOBAL_CTRL 	0x00000391
#define MSR_SNB_UNCORE_PERF_GLOBAL_CTRL 	0x00000391
#define MSR_SKL_UNCORE_PERF_GLOBAL_CTRL 	0x00000e01
#define MSR_ADL_UNCORE_PERF_GLOBAL_CTRL 	0x00002ff0

#define MSR_IVB_EP_PMON_GLOBAL_CTRL		0x00000c00
#define MSR_HSW_EP_PMON_GLOBAL_CTRL		0x00000700

#define MSR_NHM_UNCORE_PERF_GLOBAL_STATUS	0x00000392
#define MSR_SNB_UNCORE_PERF_GLOBAL_STATUS	0x00000392
#define MSR_SKL_UNCORE_PERF_GLOBAL_STATUS	0x00000e02
#define MSR_ADL_UNCORE_PERF_GLOBAL_STATUS	0x00002ff2

#define MSR_UNCORE_PERF_GLOBAL_OVF_CTRL 	0x00000393

#define MSR_NHM_UNCORE_PERF_FIXED_CTR0		0x00000394
#define MSR_SNB_UNCORE_PERF_FIXED_CTR0		0x00000395
#define MSR_SKL_UNCORE_PERF_FIXED_CTR0		0x00000395
#define MSR_ADL_UNCORE_PERF_FIXED_CTR0		0x00002fdf
#define MSR_SNB_EP_UNCORE_PERF_FIXED_CTR0	0x00000c09
#define MSR_HSW_EP_UNCORE_PERF_FIXED_CTR0	0x00000704

#define MSR_NHM_UNCORE_PERF_FIXED_CTR_CTRL	0x00000395
#define MSR_SNB_UNCORE_PERF_FIXED_CTR_CTRL	0x00000394
#define MSR_SKL_UNCORE_PERF_FIXED_CTR_CTRL	0x00000394
#define MSR_ADL_UNCORE_PERF_FIXED_CTR_CTRL	0x00002fde
#define MSR_SNB_EP_UNCORE_PERF_FIXED_CTR_CTRL	0x00000c08
#define MSR_HSW_EP_UNCORE_PERF_FIXED_CTR_CTRL	0x00000703

#define MSR_HSW_UNCORE_RATIO_LIMIT		0x00000620

#ifndef MSR_CONFIG_TDP_NOMINAL
	#define MSR_CONFIG_TDP_NOMINAL		0x00000648
#endif

#ifndef MSR_CONFIG_TDP_LEVEL_1
	#define MSR_CONFIG_TDP_LEVEL_1		0x00000649
#endif

#ifndef MSR_CONFIG_TDP_LEVEL_2
	#define MSR_CONFIG_TDP_LEVEL_2		0x0000064a
#endif

#ifndef MSR_CONFIG_TDP_CONTROL
	#define MSR_CONFIG_TDP_CONTROL		0x0000064b
#endif

#ifndef MSR_TURBO_ACTIVATION_RATIO
	#define MSR_TURBO_ACTIVATION_RATIO	0x0000064c
#endif

#ifndef MSR_RAPL_POWER_UNIT
	#define MSR_RAPL_POWER_UNIT		0x00000606
#endif

#ifndef MSR_PKG_POWER_LIMIT
	#define MSR_PKG_POWER_LIMIT		0x00000610
#endif

#ifndef MSR_PKG_ENERGY_STATUS
	#define MSR_PKG_ENERGY_STATUS		0x00000611
#endif

#ifndef MSR_PKG_PERF_STATUS
	#define MSR_PKG_PERF_STATUS		0x00000613
#endif

#ifndef MSR_PKG_POWER_INFO
	#define MSR_PKG_POWER_INFO		0x00000614
#endif

#ifndef MSR_AVN_PKG_POWER_INFO
	#define MSR_AVN_PKG_POWER_INFO		0x0000066e
#endif

#ifndef MSR_DRAM_POWER_LIMIT
	#define MSR_DRAM_POWER_LIMIT		0x00000618
#endif

#ifndef MSR_DRAM_ENERGY_STATUS
	#define MSR_DRAM_ENERGY_STATUS		0x00000619
#endif

#ifndef MSR_DRAM_PERF_STATUS
	#define MSR_DRAM_PERF_STATUS		0x0000061b
#endif

#ifndef MSR_PP0_POWER_LIMIT
	#define MSR_PP0_POWER_LIMIT		0x00000638
#endif

#ifndef MSR_PP0_ENERGY_STATUS
	#define MSR_PP0_ENERGY_STATUS		0x00000639
#endif

#ifndef MSR_PP0_POLICY
	#define MSR_PP0_POLICY			0x0000063a
#endif

#ifndef MSR_PP0_PERF_STATUS
	#define MSR_PP0_PERF_STATUS		0x0000063b
#endif

#ifndef MSR_PP1_POWER_LIMIT
	#define MSR_PP1_POWER_LIMIT		0x00000640
#endif

#ifndef MSR_PP1_ENERGY_STATUS
	#define MSR_PP1_ENERGY_STATUS		0x00000641
#endif

#ifndef MSR_PLATFORM_POWER_LIMIT
	#define MSR_PLATFORM_POWER_LIMIT	0x0000065c
#endif

#ifndef MSR_PLATFORM_ENERGY_STATUS
	#define MSR_PLATFORM_ENERGY_STATUS	0x0000064d
#endif

#ifndef MSR_PKG_C2_RESIDENCY
	#define MSR_PKG_C2_RESIDENCY		0x0000060d
#endif

#ifndef MSR_ATOM_PKG_C2_RESIDENCY
	#define MSR_ATOM_PKG_C2_RESIDENCY	0x000003f8
#endif

#ifndef MSR_PKG_C3_RESIDENCY
	#define MSR_PKG_C3_RESIDENCY		0x000003f8
#endif

#ifndef MSR_ATOM_PKG_C4_RESIDENCY
	#define MSR_ATOM_PKG_C4_RESIDENCY	0x000003f9
#endif

#ifndef MSR_PKG_C6_RESIDENCY
	#define MSR_PKG_C6_RESIDENCY		0x000003f9
#endif

#ifndef MSR_ATOM_PKG_C6_RESIDENCY
	#define MSR_ATOM_PKG_C6_RESIDENCY	0x000003fa
#endif

#ifndef MSR_ATOM_MC6_RESIDENCY
	#define MSR_ATOM_MC6_RESIDENCY		0x00000664
#endif

#ifndef MSR_PKG_C7_RESIDENCY
	#define MSR_PKG_C7_RESIDENCY		0x000003fa
#endif

#ifndef MSR_PKG_C8_RESIDENCY
	#define MSR_PKG_C8_RESIDENCY		0x00000630
#endif

#ifndef MSR_PKG_C9_RESIDENCY
	#define MSR_PKG_C9_RESIDENCY		0x00000631
#endif

#ifndef MSR_PKG_C10_RESIDENCY
	#define MSR_PKG_C10_RESIDENCY		0x00000632
#endif

#ifndef MSR_CORE_C1_RESIDENCY
    #ifdef MSR_CORE_C1_RES
	#define MSR_CORE_C1_RESIDENCY		MSR_CORE_C1_RES
    #else
	#define MSR_CORE_C1_RESIDENCY		0x00000660
    #endif
#endif

#ifndef MSR_CORE_C3_RESIDENCY
	#define MSR_CORE_C3_RESIDENCY		0x000003fc
#endif

#ifndef MSR_CORE_C6_RESIDENCY
	#define MSR_CORE_C6_RESIDENCY		0x000003fd
#endif

#ifndef MSR_CORE_C7_RESIDENCY
	#define MSR_CORE_C7_RESIDENCY		0x000003fe
#endif

#ifndef MSR_KNL_CORE_C6_RESIDENCY
	#define MSR_KNL_CORE_C6_RESIDENCY	0x000003ff
#endif

#ifndef MSR_IA32_VMX_BASIC
	#define MSR_IA32_VMX_BASIC		0x00000480
#endif

#ifndef MSR_PM_ENABLE
	#define MSR_PM_ENABLE			0x00000770
#endif

#ifndef MSR_IA32_PM_ENABLE
	#define MSR_IA32_PM_ENABLE		MSR_PM_ENABLE
#endif

#ifndef MSR_HWP_CAPABILITIES
	#define MSR_HWP_CAPABILITIES		0x00000771
#endif

#ifndef MSR_IA32_HWP_CAPABILITIES
	#define MSR_IA32_HWP_CAPABILITIES	MSR_HWP_CAPABILITIES
#endif

#ifndef MSR_HWP_REQUEST_PKG
	#define MSR_HWP_REQUEST_PKG		0x00000772
#endif

#ifndef MSR_IA32_HWP_REQUEST_PKG
	#define MSR_IA32_HWP_REQUEST_PKG	MSR_HWP_REQUEST_PKG
#endif

#ifndef MSR_HWP_INTERRUPT
	#define MSR_HWP_INTERRUPT		0x00000773
#endif

#ifndef MSR_HWP_REQUEST
	#define MSR_HWP_REQUEST 		0x00000774
#endif

#ifndef MSR_IA32_HWP_REQUEST
	#define MSR_IA32_HWP_REQUEST		MSR_HWP_REQUEST
#endif

#ifndef MSR_IA32_PKG_HDC_CTL
	#define MSR_IA32_PKG_HDC_CTL		0x00000db0
#endif

/*	Source: Intel Atom Processor E3800 Product Family Datasheet	*/
#define MSR_PKG_TURBO_CFG			0x00000670
#define MSR_THERM_CFG1				0x00000673
#define MSR_THERM_CFG2				0x00000674

#define MSR_SKL_CORE_PERF_LIMIT_REASONS 	0x0000064f

#ifndef MSR_GRAPHICS_PERF_LIMIT_REASONS
	#define MSR_GRAPHICS_PERF_LIMIT_REASONS 0x000006b0
#endif

#ifndef MSR_RING_PERF_LIMIT_REASONS
	#define MSR_RING_PERF_LIMIT_REASONS	0x000006b1
#endif

/*	Package C-State Interrupt Response Limit			*/
#ifndef MSR_PKGC3_IRTL
	#define MSR_PKGC3_IRTL			0x0000060a
#endif

#ifndef MSR_PKGC6_IRTL
	#define MSR_PKGC6_IRTL			0x0000060b
#endif

#ifndef MSR_PKGC7_IRTL
	#define MSR_PKGC7_IRTL			0x0000060c
#endif

#define MSR_FLEX_RATIO				0x00000194
#define MSR_IA32_OVERCLOCKING_STATUS		0x00000195
#define MSR_IA32_MISC_PACKAGE_CTLS		0x000000bc

typedef union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		ReservedBits	: 32-0,
		Signature	: 64-32;
	};
} MICROCODE_ID;

typedef union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		MaxBusRatio	: 13-8,
		ReservedBits2	: 50-13,
		PlatformId	: 53-50,
		ReservedBits3	: 64-53;
	};
} PLATFORM_ID;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Bus_Speed	:  3-0,
		ReservedBits	: 64-3;
	};
} FSB_FREQ;

typedef union
{	/* R/W: bits are defined as SMT or Core scope.			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		IBRS		:  1-0,  /* CPUID(EAX=07H,ECX=0):EDX[26] == 1 */
		STIBP		:  2-1,  /* CPUID(EAX=07H,ECX=0):EDX[27] == 1 */
		SSBD		:  3-2,  /* CPUID(EAX=07H,ECX=0):EDX[31] == 1 */
		IPRED_DIS_U	:  4-3,  /* CPUID(EAX=07H,ECX=2):EDX[ 1] == 1 */
		IPRED_DIS_S	:  5-4,  /* CPUID(EAX=07H,ECX=2):EDX[ 1] == 1 */
		RRSBA_DIS_U	:  6-5,  /* CPUID(EAX=07H,ECX=2):EDX[ 2] == 1 */
		RRSBA_DIS_S	:  7-6,  /* CPUID(EAX=07H,ECX=2):EDX[ 2] == 1 */
		PSFD		:  8-7,  /* CPUID(EAX=07H,ECX=2):EDX[ 0] == 1 */
		ReservedBits1	: 10-8,
		BHI_DIS_S	: 11-10, /* CPUID(EAX=07H,ECX=2):EDX[ 4] == 1 */
		ReservedBits	: 64-11;
	};
} SPEC_CTRL;

typedef union
{	/* W/O: on-demand,issue commands that affect the state of predictors.*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		IBPB		:  1-0, /* CPUID(EAX=07H,ECX=0):EDX[26] == 1 */
		ReservedBits	: 64-1;
	};
} PRED_CMD;

typedef union
{	/* W/O: writeback & invalidate the L1 data cache, previous cachelines*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		L1D_FLUSH_CMD	:  1-0, /* CPUID(EAX=07H,ECX=0):EDX[28] == 1 */
		ReservedBits	: 64-1;
	};
} FLUSH_CMD;

typedef union
{	/* R/W: IA32_MISC_PACKAGE_CTLS MSR(0xbc) introduced in 12th Gen. */
	unsigned long long	value;
	struct
	{
		unsigned long long
		ENERGY_FILTERING_CTL	:  1-0, /* Power Filtering Control */
		ReservedBits		: 64-1;
	};
} MISC_PACKAGE_CTLS;

typedef union
{	/* R/O && CPUID.(EAX=07H,ECX=0):EDX[29] == 1			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		RDCL_NO 		:  1-0,
		IBRS_ALL		:  2-1,
		RSBA			:  3-2,
		L1DFL_VMENTRY_NO	:  4-3,
		SSB_NO			:  5-4,
		MDS_NO			:  6-5,
		PSCHANGE_MC_NO		:  7-6,
		TSX_CTRL		:  8-7,
		TAA_NO			:  9-8,
		ReservedBits1		: 10-9,
		MISC_PACKAGE_CTLS_SUP	: 11-10, /* IA32_MISC_PACKAGE_CTLS */
		ENERGY_FILTERING_CTL_SUP: 12-11, /* ENERGY_FILTERING_CTL */
		DOITM_UARCH_MISC_CTRL	: 13-12,
		SBDR_SSDP_NO		: 14-13,
		FBSDP_NO		: 15-14,
		PSDP_NO 		: 16-15,
		ReservedBits2		: 17-16,
		FB_CLEAR		: 18-17,
		FB_CLEAR_CTRL		: 19-18, /* IA32_MCU_OPT_CTRL[3] */
		RRSBA			: 20-19,
		BHI_NO			: 21-20,
		XAPIC_DISABLE_STATUS_MSR: 22-21, /* xAPIC disable status */
		ReservedBits3		: 23-22,
		OVERCLOCKING_STATUS_SUP : 24-23, /* IA32_OVERLOCKING_STATUS */
		PBRSB_NO		: 25-24,
		ReservedBits4		: 64-25;
	};
} ARCH_CAPABILITIES;

typedef union
{	/* ARCH_CAPABILITIES[XAPIC_DISABLE_STATUS] == 1			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		LEGACY_XAPIC_DISABLED	:  1-0,
		ReservedBits		: 64-1;
	};
} XAPIC_DISABLE_STATUS;

typedef union
{	/* ARCH_CAPABILITIES[DOITM_UARCH_MISC_CTRL] == 1		*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		DOITM			:  1-0,
		ReservedBits		: 64-1;
	};
} UARCH_MISC_CTRL;

typedef union
{	/* ARCH_CAPABILITIES[FB_CLEAR_CTRL] == 1			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		_RNGDS_MITG_DIS :  1-0, /*0: Mitigates RDRAND and RDSEED */
		_RTM_ALLOW	:  2-1, /*1: XBEGIN=IA32_TSX_CTRL[RTM_DISABLE]*/
		_RTM_LOCKED	:  3-2, /*1: RTM_ALLOW is locked at zero */
		_FB_CLEAR_DIS	:  4-3, /* VERW instruction will not perform */
		ReservedBits	: 64-4;
	};
} MCU_OPT_CTRL;	/* Microcode Update Option Control (R/W)		*/

typedef union
{	/* 06_86 [TREMONT], 06_8D, 06_8C [Tiger Lake]			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		STLB_SUPPORTED	:  1-0, /* STLB QoS MSRs (1A8FH-1A97H) */
		ReservedBits1	:  2-1,
		FUSA_SUPPORTED	:  3-2,
		RSM_IN_CPL0_ONLY:  4-3, /* RSM inst avail in all CPL if == 0 */
		UC_LOCK_DIS_SUP :  5-4, /* 1: supports UC load lock disable */
		SPLA_EXCEPTION	:  6-5, /* split locked access MSR (0x33) */
		SNOOP_FILTER_SUP:  7-6, /*Snoop Filter QoS Mask MSRs supported*/
		UC_STORE_THROT	:  8-7, /*1: UC Store throttle MSR_MEMORY_CTRL*/
		ReservedBits2	: 64-8;
	};
} CORE_CAPABILITIES;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		RTM_DISABLE	:  1-0, /*1:XBEGIN aborts w/ EAX=0	*/
		TSX_CPUID_CLEAR :  2-1, /*0:if TSX capable then RTM=0 & HLE=0*/
		ReservedBits	: 64-2;
	};
} TSX_CTRL;

typedef union
{	/* MSR_FLEX_RATIO(0x194): Core 2 Extreme, i9-9900K, 11th Gen, 12th Gen*/
	unsigned long long value;
	struct
	{
		unsigned long long
		UnknownBits1	: 16-0,
		OC_BINS 	: 24-16, /* OC Ratio = BCLK ratio + OC_BINS */
		UnknownBits2	: 64-24;
	};
} FLEX_RATIO;

typedef union
{	/* MSR IA32_OVERCLOCKING_STATUS(0x195)				*/
	unsigned long long value;
	struct
	{
		unsigned long long
		OC_Utilized	:  1-0,  /* 1:OC have been enabled	*/
		Undervolt	:  2-1,  /* 1:Dynamic OC Undervolt Protection*/
		OC_Unlock	:  3-2,  /* 1:OC unlocked by BIOS	*/
		ReservedBits	: 64-3;
	};
} OVERCLOCKING_STATUS;

typedef union
{	/* MSR IA32_PERF_STATUS(0x198): 0F_03 [NetBurst]		*/
	unsigned long long value;
	struct
	{
		unsigned long long
		CurrVID 	:  8-0,
		CurrFID 	: 16-8,
		ReservedBits1	: 31-16,
		XE_Enable	: 32-31, /* Intel Core			*/
		ReservedBits2	: 40-32,
		MaxBusRatio	: 45-40, /* Architectural		*/
		ReservedBits3	: 46-45,
		NonInt_BusRatio : 47-46,
		ReservedBits4	: 64-47;
	} CORE;
	struct
	{
		unsigned long long
		CurrentRatio	: 16-0,
		ReservedBits	: 64-16;
	} NHM;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		CurrentRatio	: 16-8,
		ReservedBits2	: 32-16,
		CurrVID 	: 48-32, /* Core Voltage ID (Sandy Bridge) */
		ReservedBits3	: 64-48;
	} SNB;
} PERF_STATUS;

typedef union
{	/* MSR IA32_PERF_CTL(0x199): 0F_03 [NetBurst]			*/
	unsigned long long value;
	struct
	{
	  union {
	    struct {
		unsigned int
		TargetVoltage	:  8-0,
		TargetRatio	: 16-8,
		ReservedBits2	: 32-16;
	    } CORE;
	    struct {
		unsigned int
		TargetRatio	: 16-0,
		ReservedBits	: 32-16;
	    } NHM;
	    struct {
		unsigned int
		ReservedBits1	:  8-0,
		TargetRatio	: 16-8,
		ReservedBits2	: 32-16;
	    } SNB;
	  };
	    struct {
		unsigned int
		Turbo_IDA	:  1-0, /* IDA Disengage bit: Mobile [06_0F] */
		ReservedBits	: 32-1;
	    };
	};
} PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		MaxNonTurboRatio: 16-8,
		ReservedBits2	: 23-16, /* Bit[16]=1 in Westmere ?	*/
		PPIN_CAP	: 24-23, /* R/O:IVB-E,BDW-E,SKL-S : MSR_PPIN */
		ReservedBits3	: 28-24,
		ProgrammableTurbo:29-28, /* Phi,SKL,BDW,HSW,IVB,SNB,NHM,GLM */
		ProgrammableTDP : 30-29, /* Nehalem: Turbo TDC-TDP Limit */
		ProgrammableTj	: 31-30, /* R/O: 1 = TjOffset is writable */
		ReservedBits4	: 32-31,
		LowPowerMode	: 33-32, /* R/O: 1 = LPM is supported.	*/
		ConfigTDPlevels : 35-33, /* Ivy Bridge, Haswell(-E), Phy */
		ReservedBits5	: 40-35,
		MinimumRatio	: 48-40, /* Phi,SKL,BDW,HSW,IVB,SNB,NHM,GLM */
		MinOperatRatio	: 56-48, /* Ivy Bridge, Haswell(-E)	*/
		ReservedBits6	: 64-56;
	};
} PLATFORM_INFO;
/*
	MSR_PLATFORM_INFO[0xCE:30] : Programmable TJ OFFSET : R/O : Package
	06_5Ch [Atom_Goldmont], 06_3Eh [IvyBridge_EP], 06_4Fh [Broadwell_EP],
	06_56h [Broadwell_D], 06_55h [Skylake_X], 06_57h [Xeon_Phi], 06_85h
*/

typedef union
{	/* MSR MISC_FEATURE_CONTROL(0x1a4)				*/
	unsigned long long	value;
	struct				/* R/W , Core scope , Disable=1 */
	{
		unsigned long long
		L2_HW_Prefetch		:  1-0,  /* Avoton, Goldmont, NHM, SNB*/
		L2_HW_CL_Prefetch	:  2-1,  /* NHM, SNB		*/
		L1_HW_Prefetch		:  3-2,  /* Avoton, Goldmont, NHM, SNB*/
		L1_HW_IP_Prefetch	:  4-3,  /* NHM, SNB		*/
		ReservedBits1		: 11-4,
		DISABLE_THREE_STRIKE_CNT: 12-11, /* Errata [ADL021]	*/
		ReservedBits2		: 64-12;
	};
	struct
	{
		unsigned long long
		L1_DCU_Prefetch	:  1-0,
		L2_HW_Prefetch	:  2-1,
		ReservedBits	: 64-2;
	} Phi;
} MISC_FEATURE_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		LockOut 	:  1-0, /* R/WO: iff PLATFORM_INFO.PPIN_CAP */
		Enable		:  2-1, /* R/W: iff PLATFORM_INFO.PPIN_CAP  */
		ReservedBits	: 64-2;
	};
} INTEL_PPIN_CTL;

typedef struct
{
	unsigned long long	value;
} INTEL_PPIN_NUM;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Pkg_CStateLimit :  4-0,
		ReservedBits1	: 10-4,
		IO_MWAIT_Redir	: 11-10,
		ReservedBits2	: 15-11,
		CFG_Lock	: 16-15,
		AutoCStateConv	: 17-16, /* BDW-D, BDW-EP, Phi		*/
		ReservedBits3	: 24-17,
		Int_Filtering	: 25-24, /* Nehalem			*/
		C3autoDemotion	: 26-25, /* Nehalem			*/
		C1autoDemotion	: 27-26, /* Nehalem			*/
		C3undemotion	: 28-27, /* Sandy Bridge		*/
		C1undemotion	: 29-28, /* Sandy Bridge		*/
		PkgCSTdemotion	: 30-29, /* NHM,HSW-E,Core-M,BDW-E,SKL-S,Phi */
		PkgCSTundemotion: 31-30, /* NHM,HSW-E,Core-M,BDW-E,SKL-S,Phi */
		ReservedBits4	: 64-31;
	};
} CSTATE_CONFIG;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		LVL2_BaseAddr	: 16-0,
		CStateRange	: 19-16, /*  R/W			*/
		ReservedBits	: 64-19;
	};
} CSTATE_IO_MWAIT;
/*
	*MSR_PMG_IO_CAPTURE_BASE(E4h)
	Silvermont, Airmont, Goldmont, Nehalem, Sandy Bridge, Ivy Bridge-E, Phi

	if MSR_PKG_CST_CONFIG_CONTROL(E2h).IO_MWAIT_Redir is enable then
	{
	Per Module	Phi.CStateRange =	100b	C4
						110b	C6
	Per Core	SNB & IVB-E.CStateRange=000b	C3
						001b	C6
						010b	C7
	Per Core	NHM.CStateRange =	000b	C3
						001b	C6
						010b	C7
	Per Module	Airmont.CStateRange =	000b	C3
						001b	Deep Power Down tech.
						010b	C7
	Per Module	Goldmont.CStateRange =	100b	C4
						110b	C6
						111b	C7
	}
*/

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CC6demotion	: 64-0;  /*	06_37h : R/W: 1=Enable	*/
	};
} CC6_CONFIG;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		MC6demotion	: 64-0;  /*	06_37h : R/W: 1=Enable	*/
	};
} MC6_CONFIG;

typedef union
{	/* MSR_TURBO_RATIO_LIMIT(1ADh)					*/
/*
	Atom Avoton [06_4D] & Goldmont [06_5C], Nehalem [06_1A, 06_1E, 06_1F]
	Xeon Westmere [06_25, 06_2C]
	*must blackList w/ Nehalem Xeon 7500 [06_2E] & Westmere Xeon E7 [06_2F]
	Sandy Bridge [06_2A] & Xeon SNB-E5 [06_2D]
	Haswell [06_3C, 06_45, 06_46] & Xeon E5v3 [06_3F]
	Broadwell [06_3D, 06_56] & Xeon E3v4 [06_47] & Xeon E5v4[06_4F]
	Skylake Client [06_4E, 06_5E] & Xeon Skylake-X Server [06_55]
	Kabylake [06_8E, 06_9E]
	Cannonlake [06_66]
*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		MaxRatio_1C	:  8-0,  /* NHM, SNB, HSW, BDW, SKL, KBL */
		MaxRatio_2C	: 16-8,  /* NHM, SNB, HSW, BDW, SKL, KBL */
		MaxRatio_3C	: 24-16, /* NHM, SNB, HSW, BDW, SKL, KBL */
		MaxRatio_4C	: 32-24, /* NHM, SNB, HSW, BDW, SKL, KBL */
		MaxRatio_5C	: 40-32, /* Westmere, SNB-E5, E3v4	*/
		MaxRatio_6C	: 48-40, /* Westmere, SNB-E5, E3v4	*/
		MaxRatio_7C	: 56-48, /* SNB-E5, E5v3, E5v4, SKL-X	*/
		MaxRatio_8C	: 64-56; /* AVT, GLM, SNB-E5, E5v3-v4, SKL-X */
	};
} TURBO_RATIO_CONFIG0;

typedef union
{	/* MSR_TURBO_RATIO_LIMIT1(1AEh) 				*/
	unsigned long long	value;
	struct
	{ /* Haswell-E5v3 [06_3F], Broadwell-E [06_56] & Xeon E5v4 [06_4F] */
		unsigned long long
		MaxRatio_9C	:  8-0,
		MaxRatio_10C	: 16-8,
		MaxRatio_11C	: 24-16,
		MaxRatio_12C	: 32-24,
		MaxRatio_13C	: 40-32,
		MaxRatio_14C	: 48-40,
		MaxRatio_15C	: 56-48,
		MaxRatio_16C	: 64-56;
	} HSW_EP;
	struct
	{	/* Xeon IvyBridge-EPv2 [06_3E]				*/
		unsigned long long
		MaxRatio_9C	:  8-0, 	/* E5 + E7		*/
		MaxRatio_10C	: 16-8, 	/* E5 + E7		*/
		MaxRatio_11C	: 24-16,	/* E5 + E7		*/
		MaxRatio_12C	: 32-24,	/* E5 + E7		*/
		MaxRatio_13C	: 40-32,	/* E7			*/
		MaxRatio_14C	: 48-40,	/* E7			*/
		MaxRatio_15C	: 56-48,	/* E7			*/
		ReservedBits	: 63-56,	/* E7			*/
		Semaphore	: 64-63;	/* E7			*/
	} IVB_EP;
	struct
	{	/* Skylake_X [06_55]					*/
		unsigned long long
		NUMCORE_0	:  8-0,
		NUMCORE_1	: 16-8,
		NUMCORE_2	: 24-16,
		NUMCORE_3	: 32-24,
		NUMCORE_4	: 40-32,
		NUMCORE_5	: 48-40,
		NUMCORE_6	: 56-48,
		NUMCORE_7	: 64-56;
	} SKL_X;
} TURBO_RATIO_CONFIG1;

typedef union
{	/* MSR_TURBO_RATIO_LIMIT2(1AFh) 				*/
	unsigned long long	value;
	struct
	{	/* Xeon Haswell-E5v3 [06_3F]				*/
		unsigned long long
		MaxRatio_17C	:  8-0,
		MaxRatio_18C	: 16-8,
		ReservedBits	: 63-16,
		Semaphore	: 64-63;
	};
} TURBO_RATIO_CONFIG2;

typedef union
{	/* MSR_TURBO_RATIO_LIMIT2(1ACh) 				*/
	unsigned long long	value;
	struct
	{	/* Xeon Broadwell-EP [06_4F]				*/
		unsigned long long
		ReservedBits	: 63-0,
		Semaphore	: 64-63;
	};
} TURBO_RATIO_CONFIG3;

/* Config TDP MSRs:	06_3A/06_3C/06_3F/06_45/06_46/06_4E/
			06_55/06_57/06_5C/06_5E/06_66/06_7A/06_8E	*/
typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned int
		Ratio		:  8-0, /* ratio x 100 MHz		*/
		ReservedBits	: 32-8;
		unsigned int	: 32-0;
	};
} CONFIG_TDP_NOMINAL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PkgPower	: 15-0,
		ReservedBits1	: 16-15,
		Ratio		: 24-16,
		ReservedBits2	: 32-24,
		MaxPower	: 47-32,
		ReservedBits3	: 48-47,
		MinPower	: 64-48;
	};
} CONFIG_TDP_LEVEL;

typedef union
{
	unsigned long long	value;
	struct
	{	/* 0:Default; 1:TDP_LEVEL_1; 2:TDP_LEVEL_2; 3:reserved	*/
		unsigned int
		Level		:  2-0,
		ReservedBits	: 31-2,
		Lock		: 32-31;
		unsigned int	: 32-0;
	};
} CONFIG_TDP_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned int
		MaxRatio	:  8-0,
		ReservedBits	: 31-8,
		Ratio_Lock	: 32-31;
		unsigned int	: 32-0;
	};
} TURBO_ACTIVATION;

typedef union
{	/* MSR_PKG_TURBO_CFG(0x00000670)				*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		TjMax_Turbo	:  3-0,
		UnknownBits1	:  8-3,
		DynamicTurbo	:  9-8,
		DynamicPolicy	: 12-9,
		UnknownBits2	: 64-12;
	};
} PKG_TURBO_CONFIG;
/* Source: android: kernel/x86_64/.../drivers/thermal/intel_soc_thermal.c
	SLM[06_37] set bits[0:2] to 0x2 to enable TjMax Turbo mode
	[UNKNOWN] set bit[8] to 0 to disable Dynamic Turbo
	[UNKNOWN] set bits[9:11] to 0 disable Dynamic Turbo Policy
*/

typedef union
{	/* P-Unit at Offset [Port: 0x04] + 4h				*/
	unsigned int		value;
	struct {
		unsigned int
		OVERRIDE_EN	:  1-0,  /* RW: Turbo MSR		*/
		SOC_TURBO_EN	:  2-1,  /* RW: PKG_TURBO_CFG1[SOC_TDP_EN] */
		SOC_TDP_POLICY	:  5-2,  /* RW: PKG_TURBO_CFG1[SOC_TDP_POLICY]*/
		RESERVED_4	: 32-5;  /* RO: reserved		*/
	};
} ATOM_TURBO_SOC_OVERRIDE;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		FastStrings	:  1-0,  /* Fast-Strings Enable		*/
		ReservedBits1	:  2-1,
		x87Compat_Enable:  3-2,  /* Pentium4, Xeon		*/
		TCC		:  4-3,  /* Automatic Thermal Control Circuit */
		SplitLockDisable:  5-4,  /* Pentium4, Xeon		*/
		ReservedBits2	:  6-5,
		L3Cache_Disable :  7-6,  /* Pentium4, Xeon		*/
		PerfMonitoring	:  8-7,  /* Performance Monitoring Available */
		SupprLock_Enable:  9-8,  /* Pentium4, Xeon		*/
		HW_Prefetch_Dis : 10-9,  /* Pentium4, Xeon, Core (R/W)	*/
		Int_FERR_Enable : 11-10, /* Pentium4, Xeon		*/
		BTS		: 12-11, /* Branch Trace Storage Unavailable */
		PEBS		: 13-12, /* No Precise Event Based Sampling */
		TM2_Enable	: 14-13,
		ReservedBits3	: 16-14,
		EIST		: 17-16, /* Enhanced Intel SpeedStep Tech. */
		BR_PROCHOT	: 18-17, /* Broadwell			*/
		FSM		: 19-18,
		PrefetchCacheDis: 20-19, /* Pentium4, Xeon		*/
		ReservedBits4	: 22-20,
		CPUID_MaxVal	: 23-22,
		xTPR		: 24-23,
		L1DataCacheMode : 25-24, /* Pentium4, Xeon		*/
		ReservedBits5	: 26-25,
		C2E		: 27-26, /* Broadwell			*/
		ReservedBits6	: 32-27,
		C4E		: 34-32, /* Broadwell Mobile		*/
		XD_Bit_Disable	: 35-34,
		ReservedBits7	: 37-35,
		DCU_L1_Prefetch : 38-37, /* Core (R/W) Disable=1	*/
		Turbo_IDA	: 39-38, /* Disable=1 -> CPUID(0x6).IDA=0 */
		IP_Prefetcher	: 40-39,
		ReservedBits8	: 64-40;
	};
} MISC_PROC_FEATURES;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long int
		BD_PROCHOT	:  1-0,  /* BiDirectional PROCHOT	*/
		C1E		:  2-1,
		ReservedBits1	: 19-2,
		R2H_Disable	: 20-19, /* SKL,KBL,CFL:Race To Halt Disable=1*/
		EEO_Disable	: 21-20, /* SKL,KBL,CFL: Energy opt. Disable=1*/
		ReservedBits2	: 25-21,
		EBP_OS_Control	: 26-25, /* SNB: 0=EBP controlled by OS */
		ReservedBits3	: 30-26,
		CST_PreWake_Dis : 31-30, /* 1=disable Cstate Pre-Wake [CFL/S] */
		ReservedBits4	: 64-31;
	};
} POWER_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		DutyCycle	:  4-0,/* OnDemand Clock Modulation Duty Cycle*/
		ODCM_Enable	:  5-4,
		ReservedBits	: 63-5,
		ECMD		: 64-63; /* Placeholder for CPUID(0x6)AX.5 */
	};
} CLOCK_MODULATION;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PowerPolicy	:  4-0,/*0=highest perf;15=Max energy saving* */
		ReservedBits	: 64-4;
	};
} ENERGY_PERF_BIAS;
/*
	*IA32_ENERGY_PERF_BIAS
	Package:	Westmere, Sandy Bridge,
			Ivy Bridge[06_3AH],
			Ivy Bridge-E v2[06_3EH],
			Haswell[06_3CH][06_45H][06_46H],
			Haswell-E[06_3F],
			Broadwell[06_3DH][06_47H][06_4FH][06_56H],
			Skylake[06_4EH][06_5EH]
	Per Core:	Silvermont
	Per Thread:	Nehalem, Knights Landing[06_57H]
	Shared/Unique:	Core Solo, Core Duo, Dual-Core-Xeon-LV
*/

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		HW_Coord_EIST	:  1-0,  /* Pkg: 0=Enable; 1=Disable	*/
		Perf_BIAS_Enable:  2-1,  /* SMT: 1=Enable; 0=Disable*	*/
		ReservedBits1	: 10-2,
		ENABLE_SDC	: 11-10, /* Errata [ADL010]		*/
		ReservedBits2	: 13-11,
		LOCK		: 14-13, /* Errata [ADL010]		*/
		ReservedBits3	: 22-14,
		Therm_Intr_Coord: 23-22, /* Pkg: Goldmont 0=Disable; 1=Enable */
		ReservedBits4	: 64-23;
	};
} MISC_PWR_MGMT;
/*
	*MSR_MISC_PWR_MGMT
	Per Thread:	Nehalem, Sandy Bridge
	Perf_BIAS_Enable bit makes the IA32_ENERGY_PERF_BIAS register (MSR 1B0h)
	visible to software with Ring 0 privileges. This bit’s status (1 or 0)
	is also reflected by CPUID.(EAX=06h):ECX[3]
*/

typedef union
{	/* 06_4E/06_4F/06_5E/06_55/06_56/06_8E/06_9E			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		HWP_Enable	:  1-0,  /* Pkg: R/W-Once; 1=Enable	*/
		ReservedBits	: 64-1;  /* **Must be zero**		*/
	};
} PM_ENABLE;

typedef union
{	/* 06_4E/06_4F/06_5E/06_55/06_56/06_66/06_8E/06_9E		*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		Highest 	:  8-0,
		Guaranteed	: 16-8,
		Most_Efficient	: 24-16,
		Lowest		: 32-24,
		ReservedBits	: 64-32; /* **Must be zero**		*/
	};
} HWP_CAPABILITIES;	/* SMT: If CPUID.06H:EAX.[7] = 1		*/

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_Guarantee_Chg:  1-0,
		EN_Excursion_Min:  2-1,
		EN_Highest_Chg	:  3-2,
		EN_PECI_OVERRIDE:  4-3,  /* If CPUID[6].EAX[16] = 1	*/
		ReservedBits	: 64-4;  /* **Must be zero**		*/
	};
} HWP_INTERRUPT; /* SMT[SKL,KBL,CFL,CNL] If CPUID.06H:EAX.[8] = 1	*/

typedef union
{	/* 06_4E/06_4F/06_5E/06_55/06_56/06_66/06_8E/06_9E		*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		Minimum_Perf	:  8-0,  /* BDW, SKL, KBL, CFL & Superiors */
		Maximum_Perf	: 16-8,  /* BDW, SKL, KBL, CFL & Superiors */
		Desired_Perf	: 24-16, /* BDW, SKL, KBL, CFL & Superiors */
		Energy_Pref	: 32-24, /* SKL, KBL, CFL & Superiors	*/
		Activity_Window : 42-32, /* SKL, KBL, CFL & Superiors	*/
		Package_Control : 43-42, /* SKL, KBL, CFL & Superiors	*/
		ReservedBits	: 59-43,
		Act_Window_Valid: 60-59, /* Activity_Window Valid; Default=0 */
		EPP_Valid	: 61-60, /*1:[HWP_REQUEST];0:[HWP_REQUEST_PKG]*/
		Desired_Valid	: 62-61, /* -> Desired_Perf		*/
		Maximum_Valid	: 63-62, /* -> Maximum_Perf		*/
		Minimum_Valid	: 64-63; /* -> Minimum_Perf		*/
	};
} HWP_REQUEST;
/*
	Per Thread: If (CPUID.06H:EAX.[7] == 1) && (PM_ENABLE.HWP_Enable == 1)
	MSR IA32_HWP_REQUEST_PKG has same bit layout as IA32_HWP_REQUEST[41-0]
*/

typedef union
{	/* 06_4E/06_5E/06_55/06_8E/06_9E				*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		HDC_Enable	:  1-0,  /* Pkg: R/W; 1=Enable		*/
		ReservedBits	: 64-1;  /* **Must be zero**		*/
	};
} HDC_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0 ,	/* PM2			*/
		Overflow_PMC1	:  2-1 ,	/* PM2			*/
		Overflow_PMC2	:  3-2 ,	/* PM3			*/
		Overflow_PMC3	:  4-3 ,	/* PM3			*/
		Overflow_PMCn	: 32-4 ,	/* PM3			*/
		Overflow_CTR0	: 33-32,	/* PM2			*/
		Overflow_CTR1	: 34-33,	/* PM2			*/
		Overflow_CTR2	: 35-34,	/* PM2			*/
		ReservedBits1	: 55-35,
		TraceToPAPMI	: 56-55,	/* PM4, PM3(Broadwell)	*/
		ReservedBits2	: 58-56,
		LBR_Frz 	: 59-58,	/* PM4			*/
		CTR_Frz 	: 60-59,	/* PM4			*/
		ASCI		: 61-60,	/* PM4			*/
		Overflow_UNC	: 62-61,	/* PM3			*/
		Overflow_Buf	: 63-62,	/* PM2			*/
		Ovf_CondChg	: 64-63;	/* PM2			*/
	};
} CORE_GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0 ,	/* PM2			*/
		Clear_Ovf_PMC1	:  2-1 ,	/* PM2			*/
		Clear_Ovf_PMC2	:  3-2 ,	/* PM3			*/
		Clear_Ovf_PMC3	:  4-3 ,	/* PM3			*/
		Clear_Ovf_PMCn	: 32-4 ,	/* PM3			*/
		Clear_Ovf_CTR0	: 33-32,	/* PM2			*/
		Clear_Ovf_CTR1	: 34-33,	/* PM2			*/
		Clear_Ovf_CTR2	: 35-34,	/* PM2			*/
		ReservedBits1	: 55-35,
		ClrTraceToPA_PMI: 56-55,	/* PM4, PM3(Broadwell)	*/
		ReservedBits2	: 61-56,
		Clear_Ovf_UNC	: 62-61,	/* PM3			*/
		Clear_Ovf_Buf	: 63-62,	/* PM2			*/
		Clear_CondChg	: 64-63;	/* PM2			*/
	};
} CORE_GLOBAL_PERF_OVF_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0 	:  1-0 ,	/* PM2			*/
		EN_PMC1 	:  2-1 ,	/* PM2			*/
		EN_PMC2 	:  3-2 ,	/* PM3			*/
		EN_PMC3 	:  4-3 ,	/* PM3			*/
		EN_PMCn 	: 32-4 ,	/* PM3			*/
		EN_FIXED_CTR0	: 33-32,	/* PM2			*/
		EN_FIXED_CTR1	: 34-33,	/* PM2			*/
		EN_FIXED_CTR2	: 35-34,	/* PM2			*/
		ReservedBits	: 64-35;
	};
} CORE_GLOBAL_PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN0_OS		:  1-0 ,	/* PM2			*/
		EN0_Usr 	:  2-1 ,	/* PM2			*/
		AnyThread_EN0	:  3-2 ,	/* PM3			*/
		EN0_PMI 	:  4-3 ,	/* PM2			*/
		EN1_OS		:  5-4 ,	/* PM2			*/
		EN1_Usr 	:  6-5 ,	/* PM2			*/
		AnyThread_EN1	:  7-6 ,	/* PM3			*/
		EN1_PMI 	:  8-7 ,	/* PM2			*/
		EN2_OS		:  9-8 ,	/* PM2			*/
		EN2_Usr 	: 10-9 ,	/* PM2			*/
		AnyThread_EN2	: 11-10,	/* PM3			*/
		EN2_PMI 	: 12-11,	/* PM2			*/
		ReservedBits	: 64-12;
	};
} CORE_FIXED_PERF_CONTROL;

typedef union
{	/* R/W: IA32_{PACKAGE_}THERM_INTERRUPT(19Bh{1B2h}) Core|Unique scope */
	unsigned long long	value;
	struct
	{
		unsigned long long
		High_Temp_Int	:  1-0,
		Low_Temp_Int	:  2-1,
		PROCHOT_Int	:  3-2,
		FORCEPR_Int	:  4-3,
		Crit_Temp_Int	:  5-4,
		ReservedBits1	:  8-5,
		Threshold1_Value: 15-8,
		Threshold1_Int	: 16-15,
		Threshold2_Value: 23-16,
		Threshold2_Int	: 24-23,
		PLN_Enable	: 25-24, /* Power Limit Notification	*/
		HWP_Interrupt	: 26-25, /* IA32_PACKAGE_THERM_INTERRUPT */
		ReservedBits2	: 64-26;
	};
} THERM_INTERRUPT;

typedef union
{	/* R/O-R/WC0: IA32_{PACKAGE_}THERM_STATUS(19Ch{1B1h})		*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		Thermal_Status	:  1-0,
		Thermal_Log	:  2-1,
		PROCHOT_Event	:  3-2,
		PROCHOT_Log	:  4-3,
		CriticalTemp	:  5-4,
		CriticalTemp_Log:  6-5,
		Threshold1	:  7-6,
		Threshold1_Log	:  8-7,
		Threshold2	:  9-8,
		Threshold2_Log	: 10-9,
		PwrLimit_Status : 11-10,
		PwrLimit_Log	: 12-11,
		CurLimit_Status : 13-12,	/* HWP Feedback 	*/
		CurLimit_Log	: 14-13,	/* HWP Feedback 	*/
		XDomLimit_Status: 15-14,	/* HWP Feedback 	*/
		XDomLimit_Log	: 16-15,	/* HWP Feedback 	*/
		DTS		: 23-16,
		ReservedBits1	: 26-23,
		HWP_Status	: 27-26, /* IA32_PACKAGE_THERM_STATUS	*/
		Resolution	: 31-27,
		ReadingValid	: 32-31,
		ReservedBits2	: 64-32;
	};
} THERM_STATUS;

typedef union
{	/* R/O-R/WC0: MSR_CORE_PERF_LIMIT_REASONS(64FH) Package scope	*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		PROCHOT_Event	:  1-0,  /* R/O */
		Thermal_Status	:  2-1,  /* R/O */
		ReservedBits1	:  4-2,
		Residency_Sts	:  5-4,  /* R/O */
		AvgThmLimit	:  6-5,  /* R/O */
		VR_ThmAlert	:  7-6,  /* R/O */
		VR_TDC_Status	:  8-7,  /* R/O */
		EDP_Status	:  9-8,  /* R/O */
		ReservedBits2	: 10-9,
		PL1_Status	: 11-10, /* R/O */
		PL2_Status	: 12-11, /* R/O */
		TurboLimit	: 13-12, /* R/O */
		TurboAtten	: 14-13, /* R/O */
		TVB_Status	: 15-14, /* Unspecified */
		ReservedBits3	: 16-15,
		PROCHOT_Log	: 17-16, /* R/WC0 */
		Thermal_Log	: 18-17, /* R/WC0 */
		ReservedBits4	: 20-18,
		Residency_Log	: 21-20, /* R/WC0 */
		AvgThmLimitLog	: 22-21, /* R/WC0 */
		VR_ThmAlertLog	: 23-22, /* R/WC0 */
		VR_TDC_Log	: 24-23, /* R/WC0 */
		EDP_Log 	: 25-24, /* R/WC0 */
		ReservedBits5	: 26-25,
		PL1_Log 	: 27-26, /* R/WC0 */
		PL2_Log 	: 28-27, /* R/WC0 */
		TurboLimitLog	: 29-28, /* R/WC0 */
		TurboAttenLog	: 30-29, /* R/WC0 */
		TVB_Log 	: 31-30, /* Unspecified */
		ReservedBits6	: 64-31;
	};
} CORE_PERF_LIMIT_REASONS;

typedef union
{	/* R/O-R/WC0: MSR_GRAPHICS_PERF_LIMIT_REASONS(6B0H) Package scope */
	unsigned long long	value;
	struct
	{
		unsigned long long
		PROCHOT_Event	:  1-0,  /* R/O */
		Thermal_Status	:  2-1,  /* R/O */
		ReservedBits1	:  5-2,
		AvgThmLimit	:  6-5,  /* R/O */
		VR_ThmAlert	:  7-6,  /* R/O */
		VR_TDC_Status	:  8-7,  /* R/O */
		EDP_Status	:  9-8,  /* R/O */
		ReservedBits2	: 10-9,
		PL1_Status	: 11-10, /* R/O */
		PL2_Status	: 12-11, /* R/O */
		Inefficiency	: 13-12, /* R/O */
		ReservedBits3	: 16-13,
		PROCHOT_Log	: 17-16, /* R/WC0 */
		Thermal_Log	: 18-17, /* R/WC0 */
		ReservedBits4	: 21-18,
		AvgThmLimitLog	: 22-21, /* R/WC0 */
		VR_ThmAlertLog	: 23-22, /* R/WC0 */
		VR_TDC_Log	: 24-23, /* R/WC0 */
		EDP_Log 	: 25-24, /* R/WC0 */
		ReservedBits5	: 26-25,
		PL1_Log 	: 27-26, /* R/WC0 */
		PL2_Log 	: 28-27, /* R/WC0 */
		InefficiencyLog : 29-28, /* R/WC0 */
		ReservedBits6	: 64-28;
	};
} GRAPHICS_PERF_LIMIT_REASONS;

typedef union
{	/* R/O-R/WC0: MSR_RING_PERF_LIMIT_REASONS(6B1H) Package scope	*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		PROCHOT_Event	:  1-0,  /* R/O */
		Thermal_Status	:  2-1,  /* R/O */
		ReservedBits1	:  5-2,
		AvgThmLimit	:  6-5,  /* R/O */
		VR_ThmAlert	:  7-6,  /* R/O */
		VR_TDC_Status	:  8-7,  /* R/O */
		EDP_Status	:  9-8,  /* R/O */
		ReservedBits2	: 10-9,
		PL1_Status	: 11-10, /* R/O */
		PL2_Status	: 12-11, /* R/O */
		ReservedBits3	: 16-12,
		PROCHOT_Log	: 17-16, /* R/WC0 */
		Thermal_Log	: 18-17, /* R/WC0 */
		ReservedBits4	: 21-18,
		AvgThmLimitLog	: 22-21, /* R/WC0 */
		VR_ThmAlertLog	: 23-22, /* R/WC0 */
		VR_TDC_Log	: 24-23, /* R/WC0 */
		EDP_Log 	: 25-24, /* R/WC0 */
		ReservedBits5	: 26-25,
		PL1_Log 	: 27-26, /* R/WC0 */
		PL2_Log 	: 28-27, /* R/WC0 */
		ReservedBits6	: 64-28;
	};
} RING_PERF_LIMIT_REASONS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		TM2_Target	: 16-0,  /* TM2 Transition Target Ratio/Vcore */
		TM_SELECT	: 17-16, /* Unique(Core2), Shared(Xeon, Atom) */
		ReservedBits	: 64-17;
	};
} THERM2_CONTROL;

typedef union
{	/* MSR_TEMPERATURE_TARGET(1A2h) Package scope			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		Target		: 24-16,	/* R/O: Thread scope	*/
		Offset		: 28-24,	/* Nehalem		*/
		ReservedBits2	: 64-28;
	};	/* Core, NHM, SNB and superior architectures		*/
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		Target		: 24-16,
		Offset		: 30-24,	/*  R/W*		*/
		ReservedBits2	: 64-30;
	} Atom; /* SLM, Xeon Phi [06_57/06_85]				*/
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		Target		: 24-16,
		Offset		: 28-24,	/*  R/W*		*/
		ReservedBits2	: 64-28;
	} EP; /* GLM, IVB-E, BDW-E, SKL-X				*/
} TJMAX;
/*
	*) if MSR_PLATFORM_INFO[30] == 1 then Tj Offset is programmable.
*/

typedef union
{	/* 06_3D/06_3F/06_47/06_4F/06_55/06_56/06_57/06_66/06_85/06_8E/06_9E */
	unsigned long long	value;
	struct
	{
		unsigned long long		/* Pkg: R/W		*/
		MaxRatio	:  7-0,
		ReservedBits1	:  8-7,
		MinRatio	: 15-8,
		ReservedBits2	: 64-15;
	};
} UNCORE_RATIO_LIMIT;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0 ,	/* R/O , NHM		*/
		Overflow_PMC1	:  2-1 ,	/* R/O , NHM		*/
		Overflow_PMC2	:  3-2 ,	/* R/O , NHM		*/
		Overflow_PMC3	:  4-3 ,	/* R/O , NHM		*/
		Overflow_PMC4	:  5-4 ,	/* R/O , NHM, *CPUID(0xa) */
		Overflow_PMC5	:  6-5 ,	/* R/O , NHM, *CPUID(0xa) */
		Overflow_PMC6	:  7-6 ,	/* R/O , NHM, *CPUID(0xa) */
		Overflow_PMC7	:  8-7 ,	/* R/O , NHM, *CPUID(0xa) */
		ReservedBits1	: 32-8 ,
		Overflow_CTR0	: 33-32,	/* R/O , NHM		*/
		ReservedBits2	: 61-33,
		Overflow_PMI	: 62-61,	/* R/W , NHM		*/
		Ovf_DSBuffer	: 63-62,	/* SNB			*/
		Ovf_CondChg	: 64-63;	/* R/W , NHM		*/
	} NHM;	/* PMU: 06_1AH						*/
	struct
	{
		unsigned long long
		Overflow_CTR0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_PMC0	:  4-3, 	/* Overflow_Cbox	*/
		ReservedBits2	: 64-4;
	} SNB;	/* PMU: 06_2AH/06_3CH/06_45H/06_46H/06_4EH/06_5EH	*/
	struct
	{
		unsigned long long
		Overflow_CTR0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_PMC0	:  4-3, 	/* Overflow_Cbox	*/
		ReservedBits2	: 64-4;
	} SKL;
	struct
	{
		unsigned long long
		Overflow_CTR0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_PMC0	:  4-3, 	/* Overflow_Cbox	*/
		ReservedBits2	: 64-4;
	} ADL;	/* PMU: 06_97/06_9A/06_BF				*/
} UNCORE_GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0 ,	/* NHM, SNB		*/
		Clear_Ovf_PMC1	:  2-1 ,	/* NHM, SNB		*/
		Clear_Ovf_PMC2	:  3-2 ,	/* NHM, SNB		*/
		Clear_Ovf_PMC3	:  4-3 ,	/* NHM, SNB		*/
		Clear_Ovf_PMC4	:  5-4 ,	/* NHM, SNB		*/
		Clear_Ovf_PMC5	:  6-5 ,	/* NHM, *CPUID(0xa)	*/
		Clear_Ovf_PMC6	:  7-6 ,	/* NHM, *CPUID(0xa)	*/
		Clear_Ovf_PMC7	:  8-7 ,	/* NHM, *CPUID(0xa)	*/
		ReservedBits1	: 32-8 ,
		Clear_Ovf_CTR0	: 33-32,	/* NHM, SNB		*/
		ReservedBits2	: 61-33,
		Clear_Ovf_PMI	: 62-61,	/* NHM, SNB		*/
		Clear_Ovf_DSBuf : 63-62,	/* SNB			*/
		Clear_CondChg	: 64-63;	/* NHM, SNB		*/
	};
} UNCORE_GLOBAL_PERF_OVF_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0 	:  1-0 ,	/* R/W , NHM		*/
		EN_PMC1 	:  2-1 ,	/* R/W			*/
		EN_PMC2 	:  3-2 ,	/* R/W			*/
		EN_PMC3 	:  4-3 ,	/* R/W			*/
		EN_PMC4 	:  5-4 ,	/* R/W , NHM, *CPUID(0xa) */
		EN_PMC5 	:  6-5 ,	/* R/W , NHM, *CPUID(0xa) */
		EN_PMC6 	:  7-6 ,	/* R/W , NHM, *CPUID(0xa) */
		EN_PMC7 	:  8-7 ,	/* R/W , NHM, *CPUID(0xa) */
		ReservedBits1	: 32-8 ,
		EN_FIXED_CTR0	: 33-32,	/* R/W , NHM		*/
		ReservedBits2	: 48-33,
		EN_PMI_CORE0	: 49-48,	/* R/W , NHM		*/
		EN_PMI_CORE1	: 50-49,	/* R/W , NHM		*/
		EN_PMI_CORE2	: 51-50,	/* R/W , NHM		*/
		EN_PMI_CORE3	: 52-51,	/* R/W , NHM		*/
		ReservedBits3	: 63-52,
		PMI_FRZ 	: 64-63;	/* R/W , NHM		*/
	} NHM;	/* PMU: 06_1AH						*/
	struct
	{
		unsigned long long
		EN_PMI_CORE0	:  1-0 ,	/* Slice 0		*/
		EN_PMI_CORE1	:  2-1 ,	/* Slice 1		*/
		EN_PMI_CORE2	:  3-2 ,	/* Slice 2		*/
		EN_PMI_CORE3	:  4-3 ,	/* Slice 3		*/
		ReservedBits1	: 29-4 ,	/* Slice 4		*/
		EN_FIXED_CTR0	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ 	: 32-31,
		ReservedBits2	: 64-32;
	} SNB;	/* PMU: 06_2AH/06_3CH/06_45H/06_46H/06_4EH/06_5EH	*/
	struct
	{
		unsigned long long
		EN_PMI_CORE0	:  1-0 ,	/* Slice 0		*/
		EN_PMI_CORE1	:  2-1 ,	/* Slice 1		*/
		EN_PMI_CORE2	:  3-2 ,	/* Slice 2		*/
		EN_PMI_CORE3	:  4-3 ,	/* Slice 3		*/
		ReservedBits1	: 29-4 ,	/* Slice 4		*/
		EN_FIXED_CTR0	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ 	: 32-31,
		ReservedBits2	: 64-32;
	} SKL;
	struct
	{
		unsigned long long
		EN_PMI_CORE0	:  1-0 ,	/* Slice 0		*/
		EN_PMI_CORE1	:  2-1 ,	/* Slice 1		*/
		EN_PMI_CORE2	:  3-2 ,	/* Slice 2		*/
		EN_PMI_CORE3	:  4-3 ,	/* Slice 3		*/
		EN_PMI_CORE4	:  5-4 ,	/* Slice 4		*/
		ReservedBits1	: 29-5 ,
		EN_FIXED_CTR0	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ 	: 32-31,
		ReservedBits2	: 64-32;
	} ADL;	/* PMU: 06_97/06_9A/06_BF		Table 2-49	*/
} UNCORE_GLOBAL_PERF_CONTROL;
/*
	*CPUID(0xa): if CPUID.0AH:EAX[15:8] == 8 then bit is valid
*/
typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_CTR0		:  1-0,
		ReservedBits1	:  2-1,
		EN_PMI		:  3-2,
		ReservedBits2	: 64-3;
	} NHM;	/* PMU: 06_1AH						*/
	struct
	{
		unsigned long long
		ReservedBits1	: 20-0,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_CTR0 	: 23-22,
		ReservedBits3	: 64-23;
	} SNB;	/* PMU: 06_2AH/06_3CH/06_45H/06_46H/06_4EH/06_5EH	*/
	struct
	{
		unsigned long long
		ReservedBits1	: 20-0,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_CTR0 	: 23-22,
		ReservedBits3	: 64-23;
	} SKL;
	struct
	{
		unsigned long long
		ReservedBits1	: 20-0,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_CTR0 	: 23-22,
		ReservedBits3	: 64-23;
	} ADL;	/* PMU: 06_97/06_9A/06_BF				*/
	struct
	{
		unsigned long long
		ReservedBits1	: 20-0,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_CTR0 	: 23-22,
		ReservedBits3	: 64-23;
	} HSW_EP; /* 06_3FH/06-4FH					*/
} UNCORE_FIXED_PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long		/* Pkg: R/W		*/
		PMI_Core_Select : 15-0,
		ReservedBits1	: 29-15,
		Unfreeze_All	: 30-29,
		WakeOnPMI	: 31-30,
		Freeze_All	: 32-31,
		ReservedBits2	: 64-32;
	};	/* 06_3EH/06_3FH					*/
} UNCORE_PMON_GLOBAL_CONTROL;

typedef union
{	/* MSR_TURBO_POWER_CURRENT_LIMIT(0x1ac):R/W, Package		*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		TDP_Limit	: 15-0,  /* limit in 1/8 Watt granularity */
		TDP_Override	: 16-15, /* 1=override is active	*/
		TDC_Limit	: 31-16, /* limit in 1/8 Amp granularity */
		TDC_Override	: 32-31,
		ReservedBits	: 64-32;
	};
} NEHALEM_POWER_LIMIT;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long		/* Pkg: R/O		*/
		PU		:  4-0,
		ReservedBits1	:  8-4,
		ESU		: 13-8,
		ReservedBits2	: 16-13,
		TU		: 20-16,
		ReservedBits3	: 64-20;
	};
} RAPL_POWER_UNIT;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ThermalSpecPower: 15-0,
		ReservedBits1	: 16-15,
		MinimumPower	: 31-16,
		ReservedBits2	: 32-31,
		MaximumPower	: 47-32,
		ReservedBits3	: 48-47,
		MaxTimeWindow	: 55-48,
		ReservedBits4	: 64-55;
	};
} DOMAIN_POWER_INFO;

typedef union
{ /* MSR_PKG_POWER_LIMIT(0x610):R/W & MSR_PLATFORM_POWER_LIMIT(0x65c):R/W-L */
	unsigned long long	value;
	struct
	{
		unsigned long long
		Domain_Limit1	: 15-0,  /* Atom: 06_37H/06_4AH/06_5AH/06_5DH */
		Enable_Limit1	: 16-15,
		Clamping1	: 17-16,
		TimeWindow1	: 24-17,
		ReservedBits1	: 31-24,
		PPn_LOCK	: 32-31, /* PP0/PP1/DRAM Domains	*/
		Domain_Limit2	: 47-32, /* 06_2AH/06_4DH/06_57H/06_5CH/06_85H*/
		Enable_Limit2	: 48-47,
		Clamping2	: 49-48,
		TimeWindow2	: 56-49,
		ReservedBits2	: 63-56,
		PKG_LOCK	: 64-63; /* Package/Platform Domains	*/
	};
	struct
	{
		unsigned long long
		MaskBits1	: 17-0,
		TimeWindow1_Y	: 22-17,
		TimeWindow1_Z	: 24-22,
		MaskBits2	: 49-24,
		TimeWindow2_Y	: 54-49,
		TimeWindow2_Z	: 56-54,
		MaskBits3	: 64-56;
	};
} DOMAIN_POWER_LIMIT;

#define PKG_POWER_LIMIT_LOCK_MASK	0x8000000000000000
#define PPn_POWER_LIMIT_LOCK_MASK	0x0000000080000000

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long		/* Pkg: R/W		*/
		TimeLimit	: 10-0,
		TimeUnit	: 13-10,
		ReservedBits1	: 15-13,
		Valid		: 16-15,
		ReservedBits2	: 64-16;
	};
} PKGCST_IRTL;
/*
	IRTL-PC:3/6/7	{ GDM[06_5C] Gemini Lake[06_7A] }
	IRTL-PC:3/6/7	{ SNB[06_2A] SNB/EP[06_2D] }
	IRTL-PC:6/7	{ HSW[06_3C/06_45/06_46] HSW/EP[06_3F] }
*/

/* TODO
typedef struct
{
	unsigned long long
		Type		:  8-0,
		ReservedBits1	: 10-8,
		FixeRange	: 11-10,
		Enable		: 12-11,
		ReservedBits2	: 64-12;
} MTRR_DEF_TYPE;

	§18.2.4 PM4

	IA32_PERF_GLOBAL_STATUS_RESET
	IA32_PERF_GLOBAL_STATUS_SET
	IA32_PERF_GLOBAL_INUSE
*/

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long		/* NHM, SNB: Thread	*/
		VMCS_RevId	: 31-0,
		ReservedBits1	: 32-31,
		VMCS_Size	: 48-32,
		PhysAddrWidth	: 49-48,
		SMM_DualMon	: 50-49,
		VMCS_Type	: 54-50,
		ReservedBits2	: 64-54;
	};
} VMX_BASIC;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Minor		:  4-0,
		Major		:  8-4,
		ReservedBits	: 32-8;
	};
} Intel_IOMMU_VER_REG;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ND		:  3-0,
		AFL		:  4-3,
		RWBF		:  5-4,
		PLMR		:  6-5,
		PHMR		:  7-6,
		CM		:  8-7,
		SAGAW		: 13-8,
		ReservedBits1	: 16-13,
		MGAW		: 22-16,
		ZLR		: 23-22,
		DEP		: 24-23,
		FRO		: 34-24,
		SLLPS		: 38-34,
		ReservedBits2	: 39-38,
		PSI		: 40-39,
		NFR		: 48-40,
		MAMV		: 54-48,
		DWD		: 55-54,
		DRD		: 56-55,
		FL1GP		: 57-56,
		ReservedBits3	: 59-57,
		PI		: 60-59,
		FL5LP		: 61-60,
		ReservedBits4	: 64-61;
	};
} Intel_IOMMU_CAP_REG;

typedef union
{
	unsigned short	value;
	struct {
		unsigned short
		ReservedBits1	:  9-0,
		VerbatimCopy	: 10-9,  /* Same bit read must be write back */
		ReservedBits2	: 11-10,
		TCO_TMR_HALT	: 12-11, /*1=WDT will halt; 0=WDT will count */
		ReservedBits3	: 16-12;
	};
} Intel_TCO1_CNT;

typedef union
{	/* PCH SMBus Device; 31 Function; 4				*/
	unsigned int	value;
	struct {
		unsigned int
		IOS		:  1-0,
		ReservedBits1	:  5-1,
		TCOBA		: 16-5,
		ReservedBits2	: 32-16;
	};
} Intel_TCOBASE;

typedef union
{	/* PCH SMBus Device; 31 Function; 4				*/
	unsigned int	value;
	struct {
		unsigned int
		BASE_LOCK	:  1-0,
		ReservedBits1	:  8-1,
		BASE_EN		:  9-8,
		ReservedBits2	: 32-9;
	};
} Intel_TCOCTL;

typedef union
{	/* Offset Channel0: 110h & Channel1: 190h			*/
	unsigned int		value;
	struct {
		unsigned int
		BtoB_RdRd	:  4-0,
		BtoB_WrRd	:  9-4,
		ReservedBits1	: 11-9,
		tRD		: 16-11,
		BtoB_RdRd_DR	: 17-16,
		ReservedBits2	: 18-17,
		BtoB_WrWr_DR	: 20-18,
		BtoB_RdWr	: 22-20,
		BtoB_WrRd_DR	: 24-22,
		tWTR		: 28-24,
		tWR		: 32-28;
	};
} P945_MC_DRAM_TIMING_R0;

typedef union
{	/* Offset Channel0: 114h & Channel1: 194h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  3-0,
		ReservedBits1	:  4-3,
		tRCD		:  7-4,
		ReservedBits2	:  8-7,
		tCL		: 10-8,
		tRFC		: 16-10,
		tRPALL		: 17-16,
		ReservedBits3	: 18-17,
		tRRD		: 19-18,
		tRAS		: 24-19,
		ReservedBits4	: 28-24,
		tRTPr		: 30-28,
		ReservedBits5	: 32-30;
	};
} P945_MC_DRAM_TIMING_R1;

typedef union
{	/* Offset Channel0: 118h & Channel1: 198h			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  8-0,
		ReservedBits2	: 11-8,
		ReservedBits3	: 16-11,
		ReservedBits4	: 18-16,
		ReservedBits5	: 30-18,
		tCKE		: 32-30;
	};
} P945_MC_DRAM_TIMING_R2;

typedef union
{	/* Offset Channel0: 100h, 101h, 102h, 103 & Channel1: 180h, 181h */
	unsigned char		value;
	struct {
		unsigned char
		Zeroed		:  2-0,
		Boundary	:  7-2,
		DRAM_4GB	:  8-7;
	};
} P945_MC_DRAM_RANK_BOUND;

typedef union
{	/* Offset Channel0: 108h + 109h & Channel 1: 188h		*/
	unsigned int		value;
	struct {
		unsigned int
		EvenRank_R0	:  3-0,
		ReservedBits1	:  4-3,
		OddRank_R1	:  7-4,
		ReservedBits2	:  8-7,
		EvenRank_R2	: 11-8,
		ReservedBits3	: 12-11,
		OddRank_R3	: 15-12,
		ReservedBits4	: 16-15;
	};
} P945_MC_DRAM_RANK_ATTRIB;

typedef union
{	/* Offset Channel0: 10Eh & Channel1: 18Eh			*/
	unsigned int		value;
	struct {
		unsigned int
		Rank0		:  2-0,
		Rank1		:  4-2,
		Rank2		:  6-4,
		Rank3		:  8-6,
		ReservedBits	: 16-8;
	};
} P945_MC_DRAM_BANK_ARCH;

typedef union
{	/* Offset Channel0: 200h					*/
	unsigned int		value;
	struct {
		unsigned int
		DAMC		:  2-0,
		SCS		:  3-2,
		ReservedBits1	:  9-3,
		Channel_XOR	: 10-9,
		Cha_XOR_Random	: 11-10,
		ReservedBits2	: 14-11,
		ReservedBits3	: 16-14,
		SMS		: 19-16,
		IC		: 20-19,
		IC_SMS_Ctrl	: 21-20,
		EMRS		: 23-21,
		ReservedBits4	: 24-23,
		ReservedBits5	: 29-24,
		ReservedBits6	: 32-29;
	};
} P945_MC_DCC;

typedef union
{	/* Offset Channel0: 40Ch & Channel1: 48Ch			*/
	unsigned int		value;
	struct {
		unsigned int
		Rank0		:  2-0,
		Rank1		:  4-2,
		Rank2		:  6-4,
		Rank3		:  8-6,
		ReservedBits	: 16-8;
	};
} P945_MC_DRAM_RANK_WIDTH;

typedef union
{	/* Offset Channel0: 114h & Channel1: 194h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  3-0,
		ReservedBits1	:  4-3,
		tRCD		:  7-4,
		ReservedBits2	:  8-7,
		tCL		: 10-8,
		ReservedBits3	: 20-10,
		tRAS		: 24-20,
		ReservedBits4	: 32-24;
	};
} P955_MC_DRAM_TIMING_R1;

typedef union
{	/* Offset Channel0: 1210h & Channel1: 1310h			*/
	unsigned int		value;
	struct {
		unsigned int
		BtoB_RdRd	:  3-0,
		ReservedBits1	:  5-3,
		BtoB_WrWr	:  8-5,
		ReservedBits2	: 10-8,
		BtoB_RdWr	: 14-10,
		ReservedBits3	: 15-14,
		BtoB_WrRd_DR	: 18-15,
		ReservedBits4	: 20-18,
		tWTR		: 24-20,
		ReservedBits5	: 26-24,
		tWR		: 31-26,
		ReservedBits6	: 32-31;
	};
} G965_MC_DRAM_TIMING_R0;

typedef union
{	/* Offset Channel0: 1214h & Channel1: 1314h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  3-0,
		ReservedBits1	:  5-3,
		tRCD		:  8-5,
		ReservedBits2	: 10-8,
		tRRD		: 13-10,
		ReservedBits3	: 15-13,
		tRPALL		: 16-15,
		ReservedBits4	: 18-16,
		tRP_DR_SR	: 19-18,
		ReservedBits5	: 21-19,
		tRAS		: 26-21,
		ReservedBits6	: 28-26,
		tRTPr		: 30-28,
		ReservedBits7	: 32-30;
	};
} G965_MC_DRAM_TIMING_R1;

typedef union
{	/* Offset Channel0: 1218h & Channel1: 1318h			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  6-0,
		tXPDLL		: 10-6,
		ReservedBits2	: 12-10,
		tXP		: 15-10,
		ReservedBits3	: 17-15,
		tFAW		: 22-17,
		ReservedBits4	: 24-22,
		tCKE		: 27-24,
		ReservedBits5	: 32-27;
	};
} G965_MC_DRAM_TIMING_R2;

typedef union
{	/* Offset Channel0: 121Ch & Channel1: 131Ch			*/
	unsigned int		value;
	struct {
		unsigned int
		tCWL		:  3-0,
		ReservedBits1	: 13-3,
		tRFC		: 21-13,
		ReservedBits2	: 23-21,
		tCL		: 26-23,
		tXS		: 28-26,
		ReservedBits3	: 32-28;
	};
} G965_MC_DRAM_TIMING_R3;

typedef union
{	/* Offset Channel0: 1200h & Channel1: 1300h			*/
	unsigned int		value;
	struct {
		unsigned int
		Rank0Addr	:  9-0,
		ReservedBits1	: 16-9,
		Rank1Addr	: 25-16,
		ReservedBits2	: 32-25;
	};
} G965_MC_DRB_0_1;

typedef union
{	/* Offset Channel0: 1208h					*/
	unsigned int		value;
	struct {
		unsigned int
		EvenRank0	:  3-0,
		ReservedBits1	:  4-3,
		OddRank1	:  7-4,
		ReservedBits2	: 16-7,
		Rank0Bank	: 18-16,
		ReservedBits3	: 19-18,
		Rank1Bank	: 21-19,
		ReservedBits4	: 32-21;
	};
} G965_MC_DRAM_RANK_ATTRIB;

typedef union
{	/* Offset Channel0: 250h & Channel1: 650h			*/
	unsigned short		value;
	struct {
		unsigned short
		tPCHG		:  2-0,
		tRD		:  6-2,
		tWR		: 11-6,
		tRAS		: 16-11;
	};
} P965_MC_CYCTRK_PCHG;

typedef union
{	/* Offset Channel0: 252h & Channel1: 652h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRFC		:  9-0,
		tRPALL		: 13-9,
		tRP		: 17-13,
		tRRD		: 21-17,
		ACT_Disable	: 22-21,
		ACT_Count	: 28-22,
		ReservedBits	: 32-28;
	};
} P965_MC_CYCTRK_ACT;

typedef union
{	/* Offset Channel0: 256h & Channel1: 656h			*/
	unsigned short		value;
	struct {
		unsigned short
		tRD_WR		:  4-0,
		tWR_WR_DR	:  8-4,
		tWR_WR_SR	: 12-8,
		tRCD_WR 	: 16-12;
	};
} P965_MC_CYCTRK_WR;

typedef union
{	/* Offset Channel0: 258h & Channel1: 658h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRD_RD_DR	:  4-0,
		tRD_RD_SR	:  8-4,
		tWR_RD_DR	: 12-8,
		tWR_RD_SR	: 17-12, /* tWTR			*/
		tRCD_RD 	: 21-17,
		ReservedBits	: 24-21,
		tREF		: 32-24; /* See P965_MC_CYCTRK_REFR	*/
	};
} P965_MC_CYCTRK_RD;

typedef union
{	/* Offset Channel0: 25Bh					*/
	unsigned short		value;
	struct {			/*	Atom N400-500		*/
		unsigned short
		RFSH_RFSH_SR	:  9-0,  /* Same Rank REF to REF Delayed */
		PALL_RFSH_SR	: 13-9,  /* Same Rank PALL to REF Delayed */
		ReservedBits	: 16-13;
	};
} P965_MC_CYCTRK_REFR;

typedef union
{	/* Offset Channel0: 260h & Channel1: 660h			*/
	unsigned int		value;
	struct {
		unsigned int
		SingleDimmPop	:  1-0,
		tXSNR		: 10-1,
		tXP		: 14-10,
		WrODT_Safe	: 15-14,
		RdODT_Safe	: 16-15,
		EN_PDN		: 17-16,
		tCKE_Low	: 20-17,
		RankPop0	: 21-20,
		RankPop1	: 22-21,
		RankPop2	: 23-22,
		RankPop3	: 24-23,
		tCKE_High	: 27-24,
		SRC_START	: 28-27,
		CLK_WrODT_Safe	: 30-28,
		CLK_RdODT_Safe	: 32-30;
	};
} P965_MC_CKECTRL;

typedef union
{	/* Offset Channel0: 29Ch & Channel1: 69Ch			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 17-0,
		tCL		: 20-17,  /* tCL = value + 3 memory clocks */
		MCH_ODT_Latency : 24-20,
		ReservedBits2	: 32-24;
	};
} P965_MC_ODTCTRL;

typedef union
{	/* Offset Channel0: 269h					*/
	unsigned int		value;
	struct {			/*	Atom N400-500		*/
		unsigned int
		REFTIMEOUT	: 14-0,
		REFLOWWM	: 16-14,
		REFHIGHWM	: 18-16,
		REFPANICWM	: 20-18,
		REFHYSTERISIS	: 22-20,
		DDR_INITDONE	: 23-22,
		REFEN		: 24-23,
		ALLRKREF	: 25-24,
		REFCNTEN	: 26-25,
		ZQCALEN		: 27-26,
		RCOMPWAIT	: 32-27;
	};
} N400_MC_REFRCTRL_LO48;

typedef union
{	/* Offset Channel0: 26Dh					*/
	unsigned short		value;
	struct {			/*	Atom N400-500		*/
		unsigned short
		INDIRQUIET	: 38-32,
		DIRQUIET	: 44-38,
		Init_RefrCnt	: 47-44,
		ReservedBits	: 48-47;
	};
} N400_MC_REFRCTRL_HI48;

typedef union
{	/* Offset Channel0: 29Ch					*/
	unsigned int		value;
	struct {
		unsigned int
		RD_IMC_ODT	:  4-0,  /* N400-500: IMC Read Duration	*/
		WR_DRAM_ODT	:  8-4,  /* N400-500: DRAM Write Duration */
		RD_DRAM_ODT	: 12-8,  /* N400-500: DRAM Read Duration */
		ReservedBits	: 32-12;
	};
} N400_MC_ODTCTRL;

typedef union
{	/* Offset Channel0: 200h, 202h, 204h, 206 & Channel1: +400h	*/
	unsigned short		value;
	struct {
		unsigned short
		Boundary	: 10-0,
		ReservedBits	: 16-10;
	};
} P35_MC_DRAM_RANK_BOUND;

typedef union
{	/* Offset Channel0: 208h, 20Ah & Channel1: 608h, 60Ah		*/
	unsigned short		value;
	struct {
		unsigned short
		Rank0		:  8-0,
		Rank1		: 16-8;
	};
} P35_MC_DRAM_BANK_RANK;

typedef union
{	/* Offset Channel0: 250h & Channel1: 650h			*/
	unsigned short		value;
	struct {
		unsigned short
		tPCHG		:  2-0,
		tRD		:  6-2,
		tWR		: 11-6,
		ReservedBits	: 16-11;
	};
} P35_MC_CYCTRK_PCHG;

typedef union
{	/* Offset Channel0: 252h & Channel1: 652h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRFC		:  9-0,
		tRPALL		: 13-9,
		tRP		: 17-13,
		tRRD		: 21-17,
		ACT_Disable	: 22-21,
		ACT_Count	: 28-22,
		ReservedBits	: 32-28;
	};
} P35_MC_CYCTRK_ACT;

typedef union
{	/* Offset Channel0: 256h & Channel1: 656h			*/
	unsigned short		value;
	struct {
		unsigned short
		tRD_WR		:  4-0,
		tWR_WR_DR	:  8-4,
		tWR_WR_SR	: 12-8,
		tRCD_WR 	: 16-12;
	};
} P35_MC_CYCTRK_WR;

typedef union
{	/* Offset Channel0: 258h & Channel1: 658h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRD_RD_DR	:  4-0,
		tRD_RD_SR	:  8-4,
		tWR_RD_DR	: 12-8,
		tWR_RD_SR	: 17-12, /* tWTR			*/
		tRCD_RD 	: 21-17,
		ReservedBits	: 24-21,
		tREF		: 32-24;	/* Offset 25Bh		*/
	};
} P35_MC_CYCTRK_RD;

typedef union
{	/* Offset: 265h							*/
	unsigned short		value;
	struct {
		unsigned short
		UnknownBits1	:  8-0,
		tCL		: 14-8,
		UnknownBits2	: 16-14;
	};
} P35_MC_UNKNOWN_R0;

typedef union
{	/* Offset: 25Dh							*/
	unsigned short		value;
	struct {
		unsigned short
		tRAS		:  6-0,
		UnknownBits	: 16-6;
	};
} P35_MC_UNKNOWN_R1;

typedef union
{	/* Offset Channel0: 260h & Channel1: 660h			*/
	unsigned int		value;
	struct {
		unsigned int
		SingleDimmPop	:  1-0,
		tXSNR		: 10-1,
		tXP		: 14-10,
		ReservedBits1	: 16-14,
		EN_PDN		: 17-16,
		tCKE_Low	: 20-17,
		RankPop0	: 21-20,
		RankPop1	: 22-21,
		RankPop2	: 23-22,
		RankPop3	: 24-23,
		tCKE_High	: 27-24,
		SRC_START	: 28-27,
		ReservedBits2	: 32-28;
	};
} P35_MC_CKECTRL;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		FSB_Select	:  3-0,
		ReservedBits1	:  4-3,
		RAM_Select	:  7-4,
		VHCLK_Polarity	:  8-7,
		ReservedBits2	: 14-8,
		EN_DynamicFSB	: 15-14,
		ReservedBits3	: 32-15;
	};
} MCH_CLKCFG;

typedef union	/*	Message Bus Control Register (MCR)		*/
{	/* Bus: 0 - Device: 0 - Function: 0 - Offset: D0h		*/
	unsigned int		value;
	struct {
		unsigned int
		MBZ		:  4-0,  /* Must be zeroed		*/
		Bytes		:  8-4,  /* Byte Enable			*/
		Offset		: 16-8,  /* Target register		*/
		Port		: 24-16, /* Target controller		*/
		OpCode		: 32-24; /* 0x10=Read ; 0x11=Write	*/
	};
} PCI_MCR;

/*
 *		Message Data Register (MDR)
 *	Bus: 0 - Device: 0 - Function: 0 - Offset: D4h
 *
 *	Message Control Register eXtension (MCRX) 
 *	Bus: 0 - Device: 0 - Function: 0 - Offset: D8h
*/

typedef union	/*	MCR: Port=0x01 & Offset=0x0			*/
{
	unsigned int		value;
	struct {
		unsigned int
		RKEN0		:  1-0,  /* DIMM 0, Rank 0 Enabled	*/
		RKEN1		:  2-1,  /* DIMM 0, Rank 1 Enabled	*/
		RKEN2		:  3-2,  /* DIMM 1, Rank 0 Enabled	*/
		RKNE3		:  4-3,  /* DIMM 1, Rank 1 Enabled	*/
		DIMMDWID0	:  6-4,  /* 0b00:x8 0b01:x16 0b10:x32 0b11:RSV*/
		DIMMDDEN0	:  8-6,  /* Density {1; 2; 4; 8} Gbit	*/
		ReservedBits1	:  9-8,
		DIMMDWID1	: 11-9,
		DIMMDDEN1	: 13-11,
		ReservedBits2	: 14-13,
		RSIEN		: 15-14, /* Rank Select Interleave Enabled */
		ReservedBits3	: 16-15,
		DIMMFLIP	: 17-16, /* One if DIMM1 > DIMM0 size	*/
		RANKREMAP	: 18-17,
		CKECOPY 	: 19-18,
		ReservedBits4	: 20-19,
		DIMM0MIRR	: 21-20,
		DIMM1MIRR	: 22-21,
		DRAMTYPE	: 23-22, /* 0:DDR3 ; 1:LPDDR2		*/
		ENLPDDR3	: 24-23,
		ReservedBits5	: 32-24;
	};
} SOC_MC_DRP;

typedef union	/*	MCR: Port=0x01 & Offset=0x1			*/
{
	unsigned int		value;
	struct {
		unsigned int
		DFREQ		:  2-0,  /* 0h=800;1h=1066;2h=1333;3h=1600 */
		ReservedBits1	:  4-2,
		tRP		:  8-4,  /* {5; 6 ... 15; 16; RSV}	*/
		tRCD		: 12-8,  /* {5; 6 ... 15; 16; RSV}	*/
		tCL		: 15-12, /* {5; 6 ... 10; 11; RSV}	*/
		ReservedBits2	: 16-15,
		tXS		: 17-16, /* 0h=256; 1h=384		*/
		ReservedBits3	: 18-17,
		tXSDLL		: 19-18, /* {tXS + 256 ; tXS + 384}	*/
		ReservedBits4	: 20-19,
		tZQCS		: 21-20, /* 0h=64; 1h=96		*/
		ReservedBits5	: 22-21,
		tZQoper 	: 23-22, /* 0h=256; 1h=384		*/
		ReservedBits6	: 24-23,
		PMEDLY		: 26-24, /* {6; 8; 10; 12}		*/
		ReservedBits7	: 28-26,
		CKEDLY		: 32-28; /*  Multiple of 256 DRAM clocks */
	};
} SOC_MC_DTR0;

typedef union	/*	MCR: Port=0x01 & Offset=0x2			*/
{
	unsigned int		value;
	struct {
		unsigned int
		tWCL		:  3-0,  /* {3; 4; 5; 6; 7; 8;RES} clocks*/
		ReservedBits1	:  4-3,
		tCMD		:  6-4,  /* 0h=1N ; 1h=2N ; 2h=3N	*/
		ReservedBits2	:  8-6,
		tWTP		: 12-8,  /* {14; 15 ... 24; 25; RES}	*/
		tCCD		: 14-12, /* CAS to CAS delay {4; 12; 18}*/
		ReservedBits3	: 16-14,
		tFAW		: 20-16, /* {RES;RES; 14; 16 ... 32; 34;RES}*/
		tRAS		: 24-20, /* {RES; 15; 16 ... 28; 29}	*/
		tRRD		: 26-24, /* 0b00=4; 0b01=5; 0b10=6; 0b11=7*/
		ReservedBits4	: 28-26,
		tRTP		: 31-28, /* 0b00=4; 0b01=5; 0b10=6; 0b11=7*/
		ReservedBits5	: 32-24;
	};
} SOC_MC_DTR1;

typedef union	/*	MCR: Port=0x01 & Offset=0x3			*/
{
	unsigned int		value;
	struct {
		unsigned int
		tRRDR		:  3-0,  /* {RES; 6; 7; 8; 9; 10; 11; RES} */
		ReservedBits1	:  4-3,
		tRRDD		:  7-4,  /* {RES; 6; 7; 8; 9; 10; 11; RES} */
		ReservedBits2	:  8-7,
		tWWDR		: 11-8,  /* {4; 5; 6; 7; 8; 9; 10; RES} */
		ReservedBits3	: 12-11,
		tWWDD		: 15-12, /* {4; 5; 6; 7; 8; 9; 10; RES} */
		ReservedBits4	: 16-15,
		tRWDR		: 20-16, /* {RES; 6; 7 ... 17; 18; RES} */
		ReservedBits5	: 21-20,
		tRWDD		: 25-21, /* {RES; 6; 7 ... 17; 18; RES} */
		ReservedBits6	: 32-25;
	};
} SOC_MC_DTR2;

typedef union	/*	MCR: Port=0x01 & Offset=0x4			*/
{
	unsigned int		value;
	struct {
		unsigned int
		tWRDR		:  3-0,  /* {RES; 4; 5; 6; 7; 8; 9; 10; RES}*/
		ReservedBits1	:  4-3,
		tWRDD		:  7-4,  /* {4; 5; 6; 7; 8; 9; 10; RES} */
		ReservedBits2	:  8-7,
		tRWSR		: 12-8,  /* {6; 7 ... 16; 17; RES}	*/
		ReservedBits3	: 13-12,
		tWRSR		: 17-13, /* {11; 12 ... 19; 20; RES}	*/
		ReservedBits4	: 22-17,
		tXP		: 24-22, /* {2; 3; 4; 5}		*/
		PWDDLY		: 28-24,
		ENDRATE 	: 29-28,
		DERATEOVR	: 30-29,
		DERATESTAT	: 31-30,
		ReservedBits5	: 32-31;
	};
} SOC_MC_DTR3;

typedef union	/*	MCR: Port=0x01 & Offset=0x8			*/
{
	unsigned int		value;
	struct {
		unsigned int
		REFWMLO 	:  4-0,  /* Refresh Watermark (low)	*/
		REFWMHI 	:  8-4,  /* Refresh Watermark (high)	*/
		REFWMPNC	: 12-8,  /* Refresh Watermark (panic)	*/
		tREFI		: 15-12, /* {DIS; RES; 3.9 us; 7.8 us} */
		ReservedBits1	: 16-15,
		REFCNTMAX	: 18-16, /* Maximum tREFI		*/
		ReservedBits2	: 20-18,
		REFSKWDIS	: 21-20, /* Refresh counters 1/4 tREFI	*/
		REFDBTCLR	: 22-21,
		ReservedBits3	: 24-22,
		CUREFRATE	: 27-24, /* Current Refresh Rate	*/
		ReservedBits4	: 32-27;
	};
} SOC_MC_DRFC;

typedef union	/*	MCR: Port=0x04 & Offset=0x6			*/
{
	unsigned int		value;
	struct {
		unsigned int
		USB_CACHING_EN	:  1-0,
		PCIE_PLLOFFOK_EN:  2-1,
		ReservedBits1	:  7-2,
		GFX_TURBO_DIS	:  8-7,
		DDRIO_PWRGATE	:  9-8,
		ReservedBits2	: 16-9,
		DFX_PDM_MODE	: 17-16,
		DFX_PWR_GATING	: 18-17,
		ReservedBits3	: 28-18,
		EFF_DUAL_CH_EN	: 29-28, /* 0:Single ; 1:Dual		*/
		EFF_ECC_EN	: 30-29, /* 0:Disabled ; 1: Enabled	*/
		DUAL_CH_DIS	: 31-30,
		ECC_EN		: 32-31;
	};
} SOC_MC_BIOS_CFG;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 58h			*/
	unsigned int		value;
	struct {
		unsigned int
		RESET_ON_TIME	:  5-0,  /*		2^n DCLKs	*/
		BLOCK_CKE_DELAY : 10-5,  /*		2^n DCLKs	*/
		PHY_FSM_DELAY	: 15-10, /* Physical layer training: 2^n DCLK */
		REGISTERED_DIMM : 16-15, /* Channel contains registered DIMMs */
		WRLEVEL_DELAY	: 17-16, /* Write leveling training:16|32 DCLK*/
		WRDQDQS_DELAY	: 22-17, /* WRDQDQS training delay	*/
		QUAD_RANK	: 23-22, /* 1: QUAD_RANK_PRESENT	*/
		SINGLE_QUAD_RANK: 24-23, /* 1: SINGLE_QUAD_RANK_PRESENT */
		THREE_DIMMS	: 25-24, /* 1: THREE_DIMMS_PRESENT	*/
		DIS_AI		: 26-25, /* 1: RDIMM supports auto MRS cycles */
		DIS_3T		: 27-26, /* 1: 3T mode will not be enabled*/
		ReservedBits	: 32-27;
	};
} NHM_IMC_DIMM_INIT_PARAMS;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 70h			*/
	unsigned int		value;
	struct { /* Source: Micron DDR3					*/
		unsigned int
		BL		:  2-0,
		ReservedBits1	:  3-2,
		BT		:  4-3,
		tCL		:  7-4,
		ReservedBits2	:  8-7,
		DLL		:  9-8,
		tWR		: 12-9,
		Pchg_PD 	: 13-12,
		MR0		: 16-13,
		MR1		: 32-16;
	};
} NHM_IMC_MRS_VALUE_0_1;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 74h			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  3-0,
		tCWL		:  6-3,
		ASR		:  7-6,
		SRT		:  8-7,
		ReservedBits2	:  9-8,
		ODT_Rtt_Wr	: 11-9,
		ReservedBits3	: 16-11,
		RC0		: 20-16,	/* RDIMMS		*/
		RC2		: 24-20,	/* RDIMMS		*/
		MR3		: 32-24;
	};
} NHM_IMC_MRS_VALUE_2_3;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 80h			*/
	unsigned int		value;
	struct {
		unsigned int
		tsrRdTRd	:  1-0,
		tdrRdTRd	:  4-1,
		tddRdTRd	:  7-4,
		tsrRdTWr	: 11-7,
		tdrRdTWr	: 15-11,
		tddRdTWr	: 19-15,
		tsrWrTRd	: 23-19,
		tdrWrTRd	: 26-23,
		tddWrTRd	: 29-26,
		ReservedBits	: 32-29;
	};
} NHM_IMC_RANK_TIMING_A;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 84h			*/
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  6-0,
		tRRD		:  9-6,
		tsrWrTWr	: 10-9,
		tdrWrTWr	: 13-10,
		tddWrTWr	: 16-13,
		B2B		: 21-16,
		ReservedBits	: 32-21;
	};
} NHM_IMC_RANK_TIMING_B;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 88h			*/
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  4-0,
		tRAS		:  9-4,
		tRCD		: 13-9,
		tRTPr		: 17-13,
		tWTPr		: 22-17,
		ReservedBits	: 32-22;
	};
} NHM_IMC_BANK_TIMING;

typedef union	/* Nehalem						*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 8Ch			*/
	unsigned int		value;
	struct {
		unsigned int
		tRFC		:  9-0,
		tREFI_8 	: 19-9,
		tTHROT_OPPREF	: 30-19,
		ReservedBits	: 32-30;
	};
} NHM_IMC_REFRESH_TIMING;

typedef union	/* Nehalem, Westmere					*/
{	/* Device: 4, 5, 6 - Function: 0 - Offset: 90h			*/
	unsigned int		value;
	struct {
		unsigned int
		tCKE		:  3-0,
		tXS		: 11-3,
		tXSDLL		: 21-11,
		tXP		: 24-21, /* 22-21: Cs4CkeTransition	*/
		tRANKIDLE	: 32-24;
	};
} NHM_IMC_CKE_TIMING;

typedef union	/* Nehalem, Westmere					*/
{	/* Device: 3 - Function: 0 - Offset: 48h			*/
	unsigned int		value;
	struct {
		unsigned int
		CLOSED_PAGE	:  1-0,
		ECC_ENABLED	:  2-1, /* Bloomfield			*/
		AUTOPRECHARGE	:  3-2,
		CHANNELRESET0	:  4-3,
		CHANNELRESET1	:  5-4,
		CHANNELRESET2	:  6-5, /* Bloomfield			*/
		DIVBY3EN	:  7-6,
		INIT_DONE	:  8-7,
		CHANNEL0_ACTIVE :  9-8,
		CHANNEL1_ACTIVE : 10-9,
		CHANNEL2_ACTIVE : 11-10, /* Bloomfield			*/
		ReservedBits	: 32-11;
	};
} NHM_IMC_CONTROL;

typedef union	/* Nehalem, Westmere					*/
{	/* Device: 3 - Function: 0 - Offset: 4Ch			*/
	unsigned int		value;
	struct {
		unsigned int
		CHANNEL0_DISABLE:  1-0,
		CHANNEL1_DISABLE:  2-1,
		CHANNEL2_DISABLE:  3-2, /* Bloomfield			*/
		ReservedBits1	:  4-3,
		ECC_ENABLED	:  5-4, /* Bloomfield			*/
		ReservedBits2	: 32-5;
	};
} NHM_IMC_STATUS;

typedef union	/* Nehalem						*/
{	/* Device: 3 - Function: 0 - Offset: 64h			*/
	unsigned int		value;
	struct {
		unsigned int
		MAXNUMDIMMS	:  2-0,
		MAXNUMRANK	:  4-2,
		MAXNUMBANK	:  6-4,
		MAXNUMROW	:  9-6,
		MAXNUMCOL	: 11-9,
		ReservedBits	: 32-11;
	};
} NHM_IMC_MAX_DOD;

typedef union	/* Bloomfield, Lynnfield				*/
{	/* Device: 3 - Function: 4 - Offset: 50h			*/
	unsigned int		value;
	struct {
		unsigned int
		QCLK_RATIO	:  5-0,
		ReservedBits1	: 24-5,
		MAX_RATIO	: 29-24,
		ReservedBits2	: 32-29;
	};
} NHM_IMC_CLK_RATIO_STATUS;

typedef union	/* Nehalem */
{	/* Device: 4, 5, 6 - Function: 0 - Offset: B8h			*/
	unsigned int		value;
	struct {
		unsigned int
		PRIORITY_CNT	:  3-0,
		ENABLE_2N_3N	:  5-3,  /* 00=1N, 01=2N, 10=3N, 11=Reserved */
		DIS_ISOC_RBC	:  6-5,
		PRE_CAS_THRSHLD : 11-6,
		FLOAT_EN	: 12-11,
		CS_FOR_CKE_TRANS: 13-12, /* Lynnfield: CS_ODT_TRISTATE_DISABLE*/
		DDR_CLK_TRISTATE: 14-13, /* Lynnfield: Disable status bit. */
		ReservedBits	: 32-14;
	};
} NHM_IMC_SCHEDULER_PARAMS;

typedef union	/* Nehalem						*/
{ /* Device: 4, 5, 6 - Function: 0 - Offset: 48h, 4Ch, 50h, Lynnfield(54h) */
	unsigned int		value;
	struct {
		unsigned int
		NUMCOL		:  2-0,
		NUMROW		:  5-2,
		NUMRANK 	:  7-5,
		NUMBANK 	:  9-7,
		DIMMPRESENT	: 10-9,
		RANKOFFSET	: 13-10,
		ReservedBits	: 32-13;
	};
} NHM_IMC_DOD_CHANNEL;

typedef union	/* Xeon C5500/C3500, Bloomfield, Lynnfield		*/
{	/* Device: 0 - Function: 0 - Offset: C0h			*/
	unsigned int		value;
	struct {
		unsigned int
		UCLK		:  7-0,
		ReservedBits1	:  8-7,
		MinRatio	: 15-8,
		ReservedBits2	: 32-15;
	};
} NHM_CURRENT_UCLK_RATIO;

typedef union
{
	unsigned int		value;
	struct {	/* IOH Control Status & RAS Registers		*/
			/* @ Dev: 20 - Func: 2 - Off: D0h		*/
		unsigned int
		QPIFREQSEL	:  2-0,  /* 00=4800 GT/s, 10=6400 GT/s	*/
		ReservedBits	: 31-2,
		VT_d		: 32-31; /* Placeholder for VT-d: 0=Enable */
	} X58;
	struct {	/* Xeon E7 v2 & Xeon E5 v1,v2,v3		*/
		unsigned int
		QPIFREQSEL	:  3-0,  /*010=5600,011=6400,100=7200,101=8000*/
		ReservedBits1	:  4-3,
		Slow_Mode	:  5-4,
		ReservedBits2	: 32-5;
	} EP; /* Defined in SNB_EP as QPIMISCSTAT. HSW_EP: 111=9600 */
} QPI_FREQUENCY;


/*
 * Xeon E5 and E7
 * Bus: 0 - Device: 5 - Function: 0 - Offset: 108h
 *
 * Xeon E5 and E7
 * Bus: 1 - Device: 11 - Function: 3 - Offset: D0h
*/
typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		IIO		:  8-0,  /* CPUBUSNO(0) Devices 0-15	*/
		UNC		: 16-8,  /* CPUBUSNO(1) Uncore		*/
		Valid		: 17-16,
		Segment		: 25-17, /* Xeon E7: PCI domain/segment */
		ReservedBits	: 32-25;
	} CFG;
	struct {
		unsigned int
		IIO		:  8-0,  /* CPUBUSNO(0) Devices 16-31	*/
		UNC		: 16-8,  /* CPUBUSNO(1)	Uncore		*/
		ReservedBits	: 31-16,
		Valid		: 32-31;
	} UBOX;
} CPUBUSNO;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 5E00h		*/
	unsigned int		value;
	struct {
		unsigned int
		MEMCLK		:  4-0,  /* 0101=1333, 0110=1600 MT/s	*/
		ReservedBits1	:  8-4,
		PLL_REF100	:  9-8,  /* 0=133,33 MHz , 1=100,00 MHz */
		ReservedBits2 	: 32-9;
	};
} BIOS_MEMCLOCK;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4000h & Channel1: 4400h */
	unsigned int		value;
	struct {
		unsigned int
		tRCD		:  4-0,
		tRP		:  8-4,
		tCL		: 12-8,
		tCWL		: 16-12, /* IVB 			*/
		tRAS		: 24-16, /* IVB 			*/
		ReservedBits	: 32-24;
	};
	/* Device: 16,30 - Function: 0,1,4,5 - Offset 0200h		*/
	struct {
		unsigned int
		tRCD		:  5-0,
		tRP		:  9-5,
		tCL		: 14-9,
		tCWL		: 19-14,
		tRAS		: 25-19,
		cmd_oe_on	: 26-25,
		cmd_oe_cs	: 27-26,
		ReservedBits	: 32-27;
	} EP;
} SNB_IMC_TC_DBP;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4004h & Channel1: 4404h */
	unsigned int		value;
	struct {
		unsigned int
		tRRD		:  4-0,
		tRTPr		:  8-4,
		tCKE		: 12-8,
		tWTPr		: 16-12,
		tFAW		: 24-16,
		tWR		: 29-24, /* IVB 			*/
		CMD_3ST 	: 30-29, /* IVB 			*/
		CMD_Stretch	: 32-30; /* IVB: 00 = 1N , 10 = 2N , 11 = 3N */
	};
	/* Device 16,30 - Function: 0,1,4,5 - Offset 204h		*/
	struct {
		unsigned int
		tRRD		:  4-0,
		tRTPr		:  8-4,
		tCKE		: 12-8,
		tWTPr		: 16-12,
		tFAW		: 22-16,
		T_PRPDEN	: 24-22,
		tWR		: 29-24,
		CMD_3ST 	: 30-29,
		CMD_Stretch	: 32-30; /* 00=1N, 01=N/A, 10=2N, 11=3N */
	} EP;
} SNB_IMC_TC_RAP;

typedef union
{
	unsigned int		value;
	struct { /* MCHBAR - Offset Channel0: 4008h & Channel1: 4408h	*/
		unsigned int
		tRRDR		:  3-0,
		tRRSR		:  6-3,
		tWWDR		:  8-6,
		tWWDD		: 12-8,
		tRWDR		: 15-12,
		tWWSR		: 18-15,
		tWRDR		: 21-18,
		tWRDD		: 24-21,
		tRWSR		: 26-24,
		tRWDD		: 28-26,
		tCCD		: 30-28,
		tWRDR_UPPER	: 32-30;
	};
	struct { /* Device 16,30 - Function: 0,1,4,5 - Offset 208h	*/
		unsigned int
		tRRDR		:  3-0,
		tRRDD		:  6-3,
		tWWDR		:  9-6,
		tWWDD		: 12-9,
		tRWDR		: 15-12, /*	(1)	*/
		tRWDD		: 18-15, /*	(1)	*/
		tWRDR		: 21-18,
		tWRDD		: 24-21,
		tRWSR		: 27-24, /*	(1)	*/
		tCCD		: 30-27,
		tWRDR_UPPER	: 32-30; /* Xeon E7	*/
	} EP;
} SNB_IMC_TC_RWP;

/*
 * (1)	This field is no longer used starting in ES2 steppings.
 *	Please refer to TCOTHP for the new register field location.
 *
 * Goto SNB_IMC_TC_OTP
*/

typedef union
{
	unsigned int		value;	/*	0x000058d7	*/
	struct { /* MCHBAR - Offset Channel0: 400Ch & Channel1: 440Ch	*/
		unsigned int
		ReservedBits1	:  3-0,
		tRRDD		:  6-3,
		ReservedBits2	: 13-6,
		tWRSR		: 17-13,
		ReservedBits3	: 32-17;
	};
	/* Device 16,30 - Function: 0,1,4,5 - Offset 20Ch		*/
	struct {
		unsigned int
		tXPDLL		:  5-0,
		tXP		:  8-5,
		tCWL_ADJ	: 11-8,
		Shift_ODT_Early : 12-11,
		tRWDR		: 16-12,
		tRWDD		: 20-16,
		tRWSR		: 24-20,
		tODT_OE 	: 28-24,
		tCS_OE		: 32-28;
	} EP;
} SNB_IMC_TC_OTP;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 40B0h & Channel1: 44B0h */
	unsigned int		value;
	struct {
		unsigned int
		PDWN_Idle_Ctr	:  8-0,  /* Rank Power-down idle timer (DCLK) */
		PDWN_Mode	: 12-8,  /*	(1)			*/
		GLPDN		: 13-12, /*	(2)			*/
		ReservedBits	: 32-13;
	};
} SNB_IMC_PDWN;

/*
 * (1)	The value 0h (no power-down) is a don't care.
 *	0h = No Power Down
 *	1h = APD
 *	2h = PPD
 *	3h = APD - PPD
 *	6h = DLL Off
 *	7h = APD-DLL Off
 *
 * (2)	1 = Power-down decision is global for channel.
 *	0 = A separate decision is taken for each rank.
*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4294h & Channel1: 4694h */
	unsigned int		value;
	struct {
		unsigned int
		OREF_RI		:  8-0,
		REF_HP_WM	: 12-8,
		REF_PANIC_WM	: 16-12,
		DOUBLE_REF_CTRL : 18-16,
		ReservedBits	: 32-18;
	};
	/* Device 16,30 - Function: 0,1,4,5 - Offset 210h		*/
	struct {
		unsigned int
		OREFNI		:  8-0,
		REF_HI_WM	: 12-8,
		REF_PANIC_WM	: 16-12,
		ReservedBits	: 32-16;
	} EP;
} SNB_IMC_TC_RFP;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4298h & Channel1: 4698h */
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 25-16,
		tREFIx9 	: 32-25;
	};
	/* Device 16,30 - Function: 0,1,4,5 - Offset 214h		*/
	struct {
		unsigned int
		tREFI		: 15-0,
		tRFC		: 25-15,
		tREFIx9 	: 32-25;
	} EP;
} SNB_IMC_TC_RFTP;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 42A4h & Channel1: 46A4h */
	unsigned int		value;
	struct {
		unsigned int
		tXSDLL		: 12-0,  /* IVB: DDR SR exit to first RD/WR */
		tXS		: 16-12,
		tZQOPER 	: 26-16, /* IVB: ZQCL after SR exit */
		ReservedBits	: 28-26,
		tMOD		: 32-28; /* IVB: MRS command time in DCLK */
	};
	/* Device 16,30 - Function: 0,1,4,5 - Offset 218h		*/
	struct {
		unsigned int
		tXS_DLL 	: 12-0,
		tXS_XS_OFFSET	: 16-12,
		tZQOPER 	: 26-16,
		ReservedBits	: 27-16,
		tMOD		: 32-27;
	} EP;
} SNB_IMC_TC_SRFTP;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 5000h		*/
	unsigned int		value;
	struct {
		unsigned int
		CH_A		:  2-0,
		CH_B		:  4-2,
		CH_C		:  6-4,
		ReservedBits1	: 10-6,
		LPDDR		: 11-10, /* Since Sept. 2013 Haswell	*/
		ReservedBits2	: 32-11;
	};
} SNB_IMC_MAD_MAPPING;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 5004h & Channel1: 5008h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_A_Size	:  8-0,  /* Size of DIMM in 256 MB multiples */
		Dimm_B_Size	: 16-8,
		DAS		: 17-16, /* where DIMM A is selected	*/
		DANOR		: 18-17, /* DIMM A number of ranks	*/
		DBNOR		: 19-18, /* DIMM B number of ranks	*/
		DAW		: 20-19, /* DIMM A chips width:0=x8,1=x16 */
		DBW		: 21-20, /* DIMM B chips width		*/
		RI		: 22-21, /* Rank Interleave: 0=OFF, 1=ON */
		ENH_IM		: 23-22, /* Enhanced interleave mode	*/
		ReservedBits1	: 24-23,
		ECC		: 26-24, /* IVB: active?0=No,1=IO,2=NoIO,3=All*/
		ReservedBits2	: 32-26;
	};
} SNB_IMC_MAD_CHANNEL;

typedef union
{	/* Device: 0 - Function: 0 - Offset E4h 			*/
	unsigned int		value;
	struct {
		unsigned int
		DMFC		:  3-0,  /* DDR3 Maximum Frequency Capability */
		ReservedBits1	: 14-3,
		DDPCD		: 15-14, /* 2 DIMMS/Channel:0=Enable,1=Disable*/
		ReservedBits2	: 23-15,
		VT_d		: 24-23, /* VT-d: 0=Enable, 1=Disable	*/
		ReservedBits3	: 32-24;
	};	/* §2.5.33 CAPID0_A Capabilities A Register		*/
	struct {
		unsigned int
		DDR3L_EN	:  1-0,  /* DDR3L (1.35V) operation allowed */
		DDR_WRTVREF	:  2-1,  /* On-die DDR write Vref generation */
		OC_ENABLED_DSKU :  3-2,  /* IA Overclocking Enabled by DSKU */
		ReservedBits1	: 14-3,
		DDPCD		: 15-14, /* 2 DIMMS per Channel Disable */
		ReservedBits2	: 23-15,
		VT_d		: 24-23, /* VTd Disable 		*/
		ReservedBits3	: 25-24,
		ECCDIS		: 26-25, /* 0=ECC capable, 1=Not ECC capable */
		ReservedBits4	: 32-26;
	} IVB;
} SNB_CAPID_A;

typedef union
{	/* Device: 10 - Function: 3 - Offset: 84h			*/
	unsigned int		value;
	struct {
		unsigned int
		DE_SKTB2_UP	:  1-0,
		DE_SKTB2_EN	:  2-1,
		DE_SKTR_EP2S	:  3-2,
		DE_SKTR_EP4S	:  4-3,
		DE_SKTR1_EX	:  5-4,
		CACHESIZE	:  8-5,
		PRG_TDP_LIM_EN	:  9-8,
		LLC_WAY_EN	: 12-9,
		HT_DIS		: 13-12,
		VT_CPAUSE_EN	: 14-13,
		VT_REAL_MODE	: 15-14,
		VT_X3_EN	: 16-15,
		CORECONF_RES12	: 17-16,
		VMX_DIS 	: 18-17,
		SMX_DIS 	: 19-18,
		LT_PRODUCTION	: 20-19,
		LT_SX_EN	: 21-20,
		LT_SMM_INHIBIT	: 22-21,
		TSC_DEADLINE_DIS: 23-22,
		AES_DIS 	: 24-23,
		XSAVE_DIS	: 25-24,
		XSAVEOPT_DIS	: 26-25,
		GSSE256_DIS	: 27-26,
		SLC64_DIS	: 28-27,
		ART_DIS 	: 29-28,
		PECI_EN 	: 30-29,
		DCU_MODE	: 31-30,
		PCLMULQ_DIS	: 32-31;
	};
} SNB_EP_CAPID0;

typedef union
{	/* Device: 10 - Function: 3 - Offset: 88h			*/
	unsigned int		value;
	struct {
		unsigned int
		DCA_EN		:  1-0,
		CORE_RAS_EN	:  2-1,
		PPPE_EN 	:  4-2,
		GV3_DIS 	:  5-4,
		PWRBITS_DIS	:  6-5,
		CPU_HOT_ADD_EN	:  7-6,
		X2APIC_EN	:  8-7,
		ReservedBits1	:  9-8,
		QOS_DIS 	: 10-9,
		ReservedBits2	: 11-10,
		SSKU_P1_RATIO	: 17-11,
		SSKU_P0_RATIO	: 23-17,
		MEM_PA_SIZE	: 26-23,
		DMFC		: 30-26,
		LT_SUPPORT_DIS	: 31-30,
		MEM_MIRROR_DIS	: 32-31;
	};
} SNB_EP_CAPID1;

typedef union
{	/* Device: 10 - Function: 3 - Offset: 8Ch			*/
	unsigned int		value;
	struct {
		unsigned int
		PCIE_WS_DIS	:  1-0,
		PCIEx16_DIS	:  3-1,
		PCIE_XP_DIS	: 13-3,
		PCIE_DMI_DIS	: 14-13,
		PCIE_DMA_DIS	: 15-14,
		PCIE_GEN3_DIS	: 16-15,
		PCIE_LT_DIS	: 17-16,
		PCIE_LTSX_DIS	: 18-17,
		PCIE_RAID_DIS	: 19-18,
		PCIE_NTB_DIS	: 20-19,
		THERMAL_PROFILE : 22-20,
		SPARE_SIGNED_FW : 23-22,
		QPI_LINK0_DIS	: 24-23,
		QPI_LINK1_DIS	: 25-24,
		QPI_RATIO_DIS	: 30-25,
		QPI_LINK2_DIS	: 31-30,
		QPI_SPARE	: 32-31;
	};
} SNB_EP_CAPID2;

typedef union
{	/* Device: 10 - Function: 3 - Offset: 90h			*/
	unsigned int		value;
	struct {
		unsigned int
		CHANNEL_DIS	:  4-0,
		DISABLE_2_DPC	:  5-4,
		DISABLE_3_DPC	:  7-5,
		DDR3_8GBIT_DIS	:  8-7,
		DDR3_4GBIT_DIS	:  9-8,
		QR_DIMM_DIS	: 10-9,
		ECC_DIS 	: 11-10,
		DIR_DIS 	: 12-11,
		MODE_3N_DIS	: 13-12,
		RDIMM_DIS	: 14-13,
		UDIMM_DIS	: 15-14,
		CLTT_DIS	: 16-15,
		LOCKSTEP_DIS	: 17-16,
		SPARING_DIS	: 18-17,
		PATROL_SCRUB_DIS: 19-18,
		EXT_LAT_DIMM_DIS: 20-19,
		EXTADDR_DIMM_DIS: 21-20,
		RAID_OR_ADDR_DIS: 22-21,
		SMBUS_WRITE_DIS : 23-22,
		MONROE_TECH_DIS : 24-23,
		MC2GD		: 30-24,
		MC_SPARE	: 32-30;
	};
} SNB_EP_CAPID3;

typedef union
{	/* Device: 10 - Function: 3 - Offset: 94h			*/
	unsigned int		value;
	struct {
		unsigned int
		LLC_SLICE_IACORE: 15-0,
		ReservedBits1	: 19-15,
		SPARE_FUSES	: 27-19,
		TSC_RST_S3EXIT	: 28-27,
		ReservedBits2	: 29-28,
		I_TURBO_ENABLE	: 30-29,
		DRAM_RAPL_DIS	: 31-30,
		DRAM_POWER_DIS	: 32-31;
	};
} SNB_EP_CAPID4;

typedef union
{	/* Device: 15,29 - Function: 0 - Offset: 7Ch			*/
	unsigned int		value;
	struct {
		unsigned int		/* E5-1600/2400/2600/4600	*/
		CLOSE_PG	:  1-0,
		LS_EN		:  2-1,
		ECC_EN		:  3-2,
		DIR_EN		:  4-3,
		ReservedBits1	:  8-4,
		Normal_Mode	:  9-8,
		BANK_XOR_EN	: 10-9,
		CPGC_IOSAV	: 12-10,
		IMC_MODE	: 14-12,
		ReservedBits2	: 16-14,
		CHN_DISABLE	: 20-16,
		ReservedBits3	: 32-20;
	};
} SNB_EP_MC_TECH;

typedef union
{	/* Device: 15,29 - Function: 0 - Offset: 80h, 84h ... A8h, ACh	*/
	unsigned int		value;
	struct {
		unsigned int
		CH_TGT0 	:  2-0,
		CH_TGT1 	:  4-2,
		CH_TGT2 	:  6-4,
		CH_TGT3 	:  8-6,
		CH_WAY		: 10-8,
		SKT_WAY 	: 12-10,
		LIMIT		: 32-12;
	};
} SNB_EP_TADWAYNESS;

typedef union
{	/* Device: 15,29 - Function: 2,3,4,5 - Offset: 80h, 84h, 88h	*/
	unsigned int		value;
	struct {
		unsigned int
		CA_WIDTH	:  2-0,
		RA_WIDTH	:  5-2,
		DDR3_DNSTY	:  7-5,
		DDR3_WIDTH	:  9-7,
		ReservedBits1	: 12-9,
		RANK_CNT	: 14-12,
		DIMM_POP	: 15-14,
		ReservedBits2	: 16-15,
		RANK_DISABLE	: 20-16,
		ReservedBits3	: 32-20;
	};
} SNB_EP_DIMM_MTR;

typedef union
{	/* Device: 0 - Function: 0 - Offset E8h 			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  4-0,
		DMFC		:  7-4,
		ReservedBits2	: 17-7,
		ADDGFXCAP	: 18-17, /* Additive Graphics		*/
		ADDGFXEN	: 19-18,
		ReservedBits3	: 20-19,
		PEGG3_DIS	: 21-20, /* PCIe Gen 3 Disable		*/
		PLL_REF100_CFG	: 24-21, /* DDR3 Max. Freq. Capable @ 100MHz */
		ReservedBits4	: 25-24,
		CACHESZ 	: 28-25, /* Cache Size Capability	*/
		SMTCAP		: 29-28, /* SMT Capability		*/
		ReservedBits5	: 32-29;
	};
} IVB_CAPID_B;	/* §2.5.39 CAPID0_B Capabilities B Register		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4C00h		*/
	unsigned int		value;
	struct { /*TODO(No specification found: guessed timings position)*/
		unsigned int
		tRCD		:  5-0,
		tRP		: 10-5,
		tRAS		: 16-10,
		tRRD		: 20-16,
		UnknownBits	: 32-20;
	};
} HSW_DDR_TIMING_4C00;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4C04h		*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 12-0,
		tsrRdTRd	: 15-12,
		tdrRdTRd	: 19-15,
		tddRdTRd	: 23-19,
		ReservedBits2	: 30-23,
		CMD_Stretch	: 32-30; /* 00: 1N , 10: 2N , 11: 3N	*/
	};
} HSW_DDR_TIMING;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4C08h		*/
	unsigned int		value;
	struct {
		unsigned int
		tsrWrTRd	:  6-0,
		tdrWrTRd	: 10-6,
		tddWrTRd	: 14-10,
		tsrWrTWr	: 17-14,
		tdrWrTWr	: 21-17,
		tddWrTWr	: 25-21,
		ReservedBits	: 32-25;
	};
} HSW_DDR_RANK_TIMING_A;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4C0Ch		*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 14-0,
		tsrRdTWr	: 19-14,
		tdrRdTWr	: 24-19,
		tddRdTWr	: 29-24,
		ReservedBits2	: 32-29;
	};
} HSW_DDR_RANK_TIMING_B;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4C14h		*/
	unsigned int		value;
	struct {
		unsigned int
		tCL		:  5-0,
		tCWL		: 10-5,
		ReservedBits	: 32-10;
	};
} HSW_DDR_RANK_TIMING;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4CB0h		*/
	unsigned int		value;
	struct {
		unsigned int
		PDWN_Idle_Ctr 	: 12-0,  /* Power-down entrance in DCLK	*/
		PDWN_Mode	: 16-12, /* 1:APD,2:PPD,6:DLL-OFF,3-5,7-15:RSV*/
		ReservedBits 	: 32-16;
	};
} HSW_PM_POWER_DOWN;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4E94h		*/
	unsigned int		value;
	struct {
		unsigned int
		OREF_RI 	:  8-0,  /* Rank idle perriod in DCLK	*/
		Ref_HP_WM	: 12-8,  /* 8: Refresh priority to high	*/
		Ref_Panic_WM	: 16-12, /* 9: Refresh priority to panic */
		ReservedBits 	: 32-16;
	};
} HSW_TC_REFRESH_PARAM;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4E98h		*/
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 25-16,
		tREFIx9 	: 32-25;
	};
} HSW_TC_REFRESH_TIMING;

typedef union
{	/* Device: 19,22 - Function: 0 - Offset: 7Ch			*/
	unsigned int		value;
	struct {
		unsigned int		/* E5-1600/2600 and E7 v3 (HSW-2015) */
		CLOSE_PG	:  1-0,
		LS_EN		:  2-1,
		ECC_EN		:  3-2,
		DIR_EN		:  4-3,
		ReservedBits1	:  8-4,
		Normal_Mode	:  9-8,  /* 0: Training ; 1: Normal	*/
		ReservedBits2	: 12-9,
		IMC_MODE	: 14-12, /* 00:Native DDR; 10:SubCh; 11:Perf */
		DDR4_Mode	: 15-14,
		ReservedBits3	: 16-15,
		Pass76		: 18-16,
		CHN_DISABLE	: 22-18,
		ReservedBits4	: 32-22;
	};
} HSW_EP_MC_TECH;

#define HSW_EP_TADWAYNESS	SNB_EP_TADWAYNESS

typedef union
{	/* Device: 19,22 - Function: 2,3,4,5 - Offset: 80h, 84h, 88h	*/
	unsigned int		value;
	struct {
		unsigned int
		CA_WIDTH	:  2-0,
		RA_WIDTH	:  5-2,
		DDR3_DNSTY	:  7-5,
		DDR3_WIDTH	:  9-7,
		ReservedBits1	: 12-9,
		RANK_CNT	: 14-12,
		DIMM_POP	: 15-14,
		ReservedBits2	: 16-15,
		RANK_DISABLE	: 20-16,
		DDR4_Mode	: 21-20, /* Must be the same on all channels */
		HDRL		: 22-21, /* High Density Reduced Load mode */
		HDRL_Parity	: 23-22,
		ReservedBits3	: 32-23;
	};
} HSW_EP_DIMM_MTR;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4000h & Channel1: 4400h */
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  6-0,  /* Holds parameter tRP (and tRCD) ! */
		tRPab_ext	:  8-6,
		tRAS		: 15-8,
		ReservedBits1	: 16-15,
		tRDPRE		: 20-16,
		ReservedBits2	: 24-20,
		tWRPRE		: 31-24,
		ReservedBits3	: 32-31;
	};
} SKL_IMC_CR_TC_PRE;	/* Timing constraints to PRE commands		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4000h & Channel1: 4400h */
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  6-0,  /* (incl. tRCD) - Range: 8-59	*/
		tRPab_ext	:  9-6,  /* Range: 0-6. Unknown: 0b111	*/
		tRAS		: 16-9,  /* Range: 28-90		*/
		tRDPRE		: 21-16, /* Range:  6-15		*/
		tPPD		: 24-21, /* Range:  4-7 		*/
		tWRPRE		: 32-24; /* Range: 18-159		*/
	};
} RKL_IMC_CR_TC_PRE;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4000h & Channel1: 4400h */
	unsigned long long	value;
	struct {
		unsigned long long
		tRP		:  7-0,  /* (incl. tRCD) - Range: 8-60	*/
		tRPab_ext	: 11-7,  /* Range: 0-6. Unknown: 0b111	*/
		tRDPRE		: 17-11, /* Range:  4-32		*/
		tPPD		: 21-17, /* Range:  4-7 		*/
		tWRPRE		: 30-21, /* Range: 18-200		*/
		ReservedBits1	: 33-30,
		tRAS		: 41-33, /* Range: 28-136		*/
		tRCD		: 48-41, /* Range:  8-59		*/
		ReservedBits2	: 64-48;
	};
} TGL_IMC_CR_TC_PRE;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E000h & Channel1: E800h */
	unsigned long long	value;
	struct {
		unsigned long long
		tRP		:  8-0,  /* (incl. tRCD) - Range: 8-60	*/
		tRPab_ext	: 13-8,  /* Range:  0-6 		*/
		tRDPRE		: 20-13, /* Range:  4-32		*/
		tPPD		: 24-20, /* Range:  4-7; Not for DDR5	*/
		ReservedBits1	: 32-24,
		tWRPRE		: 42-32, /* Range: 18-200		*/
		tRAS		: 51-42, /* Range: 28-136		*/
		tRCD		: 59-51, /* Range:  8-59		*/
		DERATING_EXT	: 63-59, /* Range:  0-4 		*/
		ReservedBits2	: 64-63;
	};
} ADL_IMC_CR_TC_PRE;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4004h & Channel1: 4404h */
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  7-0,  /* Four activates window: 16-88 */
		ReservedBits1	:  8-7,
		tRRD_SG 	: 13-8,  /* Same Bank Group: 4-15; RKL: 4-22 */
		tRRD_DG 	: 18-13, /* Diff Bank Group: 4-22	*/
		DERATING_EXT	: 21-18,
		tRCD_WR 	: 27-21, /* if DDR4-E else same as tRP	*/
		ReservedBits2	: 32-27;
	};
} SKL_IMC_CR_TC_ACT;	/* Timing constraints to ACT commands		*/

#define RKL_IMC_CR_TC_ACT	SKL_IMC_CR_TC_ACT

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4008h & Channel1: 4408h */
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  8-0,  /* Range: 16-88		*/
		tRRD_SG 	: 14-8,  /* Range:  4-32		*/
		tRRD_DG 	: 20-14, /* Range:  4-32		*/
		DERATING_EXT	: 23-20, /* Range:  0-4 		*/
		ReservedBits	: 32-23;
	};
} TGL_IMC_CR_TC_ACT;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E008h & Channel1: E808h */
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  9-0,  /* Range: 16-88		*/
		tRRD_SG 	: 15-9,  /* Range:  4-32	tRRD_L	*/
		tRRD_DG 	: 22-15, /* Range:  4-32	tRRD	*/
		ReservedBits	: 24-22,
		tREFSBRD	: 32-24;
	};
} ADL_IMC_CR_TC_ACT;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 400Ch & Channel1: 440Ch */
	unsigned int		value;
	struct {
		unsigned int
		tRDRD_SG	:  6-0,  /* RD to RD, same bank: 4-54	*/
		ReservedBits1	:  8-6,
		tRDRD_DG	: 14-8,  /* RD to RD, different bank: 4-54 */
		ReservedBits2	: 16-14,
		tRDRD_DR	: 22-16, /* RD to RD, different rank: 4-54 */
		ReservedBits3	: 24-22,
		tRDRD_DD	: 30-24, /* RD to RD, different DIMM: 4-54 */
		ReservedBits4	: 32-30;
	};
} SKL_IMC_CR_TC_RDRD;	/* Timing between read and read transactions	*/

#define RKL_IMC_CR_TC_RDRD	SKL_IMC_CR_TC_RDRD

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 400Ch & Channel1: 440Ch */
	unsigned int		value;
	struct {
		unsigned int
		tRDRD_SG	:  6-0,  /* RD to RD, same bank: 4-54	*/
		ReservedBits1	:  8-6,
		tRDRD_DG	: 14-8,  /* RD to RD, different bank: 4-54 */
		ReservedBits2	: 16-14,
		tRDRD_DR	: 23-16, /* RD to RD, different rank: 4-54 */
		ReservedBits3	: 24-23,
		tRDRD_DD	: 31-24, /* RD to RD, different DIMM: 4-54 */
		ReservedBits4	: 32-31;
	};
} TGL_IMC_CR_TC_RDRD;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E00Ch & Channel1: E80Ch */
	unsigned int		value;
	struct {
		unsigned int
		tRDRD_SG	:  7-0,  /* RD to RD, same bank: 4-54	*/
		TWODCLK_B2B_LPDDR: 8-7,
		tRDRD_DG	: 15-8,  /* RD to RD, different bank: 4-54 */
		ReservedBits	: 16-15,
		tRDRD_DR	: 24-16, /* RD to RD, different rank: 4-54 */
		tRDRD_DD	: 32-24; /* RD to RD, different DIMM: 4-54 */
	};
} ADL_IMC_CR_TC_RDRD;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4010h & Channel1: 4410h */
	unsigned int		value;
	struct {
		unsigned int
		tRDWR_SG	:  6-0,  /* RD to WR, same bank: 4-54	*/
		ReservedBits1	:  8-6,
		tRDWR_DG	: 14-8,  /* RD to WR, different bank: 4-54 */
		ReservedBits2	: 16-14,
		tRDWR_DR	: 22-16, /* RD to WR, different rank: 4-54 */
		ReservedBits3	: 24-22,
		tRDWR_DD	: 30-24, /* RD to WR, different DIMM: 4-54 */
		ReservedBits4	: 32-30;
	};
} SKL_IMC_CR_TC_RDWR;	/* Timing between read and write transactions	*/

#define RKL_IMC_CR_TC_RDWR	SKL_IMC_CR_TC_RDWR

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4010h & Channel1: 4410h */
	unsigned int		value;
	struct {
		unsigned int
		tRDWR_SG	:  7-0,  /* RD to WR, same bank: 4-54	*/
		ReservedBits1	:  8-7,
		tRDWR_DG	: 15-8,  /* RD to WR, different bank: 4-54 */
		ReservedBits2	: 16-15,
		tRDWR_DR	: 23-16, /* RD to WR, different rank: 4-54 */
		ReservedBits3	: 24-23,
		tRDWR_DD	: 31-24, /* RD to WR, different DIMM: 4-54 */
		ReservedBits4	: 32-31;
	};
} TGL_IMC_CR_TC_RDWR;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E010h & Channel1: E810h */
	unsigned int		value;
	struct {
		unsigned int
		tRDWR_SG	:  8-0,  /* RD to WR, same bank: 4-54	*/
		tRDWR_DG	: 16-8,  /* RD to WR, different bank: 4-54 */
		tRDWR_DR	: 24-16, /* RD to WR, different rank: 4-54 */
		tRDWR_DD	: 32-24; /* RD to WR, different DIMM: 4-54 */
	};
} ADL_IMC_CR_TC_RDWR;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4014h & Channel1: 4414h */
	unsigned int		value;
	struct {
		unsigned int
		tWRRD_SG	:  8-0,  /* WR to RD, same bank: 4-145	*/
		tWRRD_DG	: 15-8,  /* WR to RD, different bank: 4-65 */
		ReservedBits1	: 16-15,
		tWRRD_DR	: 22-16, /* WR to RD, different rank: 4-54 */
		ReservedBits2	: 24-22,
		tWRRD_DD	: 30-24, /* WR to RD, different DIMM: 4-54 */
		ReservedBits3	: 32-30;
	};
} SKL_IMC_CR_TC_WRRD;	/* Timing between write and read transactions	*/

#define RKL_IMC_CR_TC_WRRD	SKL_IMC_CR_TC_WRRD

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4014h & Channel1: 4414h */
	unsigned int		value;
	struct {
		unsigned int
		tWRRD_SG	:  8-0,  /* WR to RD, same bank: 4-145	*/
		tWRRD_DG	: 16-8,  /* WR to RD, different bank: 4-65 */
		tWRRD_DR	: 22-16, /* WR to RD, different rank: 4-54 */
		ReservedBits1	: 24-22,
		tWRRD_DD	: 30-24, /* WR to RD, different DIMM: 4-54 */
		ReservedBits2	: 32-30;
	};
} TGL_IMC_CR_TC_WRRD;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E014h & Channel1: E814h */
	unsigned int		value;
	struct {
		unsigned int
		tWRRD_SG	:  9-0,  /* WR to RD, same bank: 4-145	*/
		tWRRD_DG	: 18-9,  /* WR to RD, different bank: 4-65 */
		tWRRD_DR	: 25-18, /* WR to RD, different rank: 4-54 */
		tWRRD_DD	: 32-25; /* WR to RD, different DIMM: 4-54 */
	};
} ADL_IMC_CR_TC_WRRD;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4018h & Channel1: 4418h */
	unsigned int		value;
	struct {
		unsigned int
		tWRWR_SG	:  6-0,  /* WR to WR, same bank: 4-54	*/
		ReservedBits1	:  8-6,
		tWRWR_DG	: 14-8,  /* WR to WR, different bank: 4-54 */
		ReservedBits2	: 16-14,
		tWRWR_DR	: 22-16, /* WR to WR, different rank: 4-54 */
		ReservedBits3	: 24-22,
		tWRWR_DD	: 30-24, /* WR to WR, different DIMM: 4-54 */
		ReservedBits4	: 32-30;
	};
} SKL_IMC_CR_TC_WRWR;	/* Timing between write and write transactions	*/

#define RKL_IMC_CR_TC_WRWR	SKL_IMC_CR_TC_WRWR

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4018h & Channel1: 4418h */
	unsigned int		value;
	struct {
		unsigned int
		tWRWR_SG	:  6-0,  /* WR to WR, same bank: 4-54	*/
		ReservedBits1	:  8-6,
		tWRWR_DG	: 14-8,  /* WR to WR, different bank: 4-54 */
		ReservedBits2	: 16-14,
		tWRWR_DR	: 22-16, /* WR to WR, different rank: 4-54 */
		ReservedBits3	: 24-22,
		tWRWR_DD	: 31-24, /* WR to WR, different DIMM: 4-54 */
		ReservedBits4	: 32-31;
	};
} TGL_IMC_CR_TC_WRWR;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E018h & Channel1: E818h */
	unsigned int		value;
	struct {
		unsigned int
		tWRWR_SG	:  7-0,  /* WR to WR, same bank: 4-54	*/
		ReservedBits1	:  8-7,
		tWRWR_DG	: 15-8,  /* WR to WR, different bank: 4-54 */
		ReservedBits2	: 16-15,
		tWRWR_DR	: 23-16, /* WR to WR, different rank: 4-54 */
		ReservedBits3	: 24-23,
		tWRWR_DD	: 32-24; /* WR to WR, different DIMM: 4-54 */
	};
} ADL_IMC_CR_TC_WRWR;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 401Ch & Channel1: 441Ch */
	unsigned int		value;
	struct {
		unsigned int
		DRAM_Tech	:  2-0,/* 00:DDR4 01:DDR3 10:LPDDR3 11:Illegal*/
		CMD_Stretch	:  4-2,  /* 00:1N , 01:2N , 10:3N , 11:1N */
		N_to_1_ratio	:  7-4,
		ReservedBits	:  8-7,
		Addr_Mirroring	: 10-8,
		x8_device_Dimm0 : 11-10, /* LSB is for DIMM 0		*/
		x8_device_Dimm1 : 12-11, /* MSB is for DIMM 1		*/
		tCPDED		: 15-12,
		LPDDR_2N_CS	: 16-15,
		Reset_OnCmd	: 20-16,
		Reset_Delay	: 23-20,
		CMD_3st 	: 24-23,
		tCKE		: 27-24,
		EN_ODT_Matrix	: 28-27,
		ProbelessLowFreq: 29-28,
		tCAL		: 32-29;
	};
} SKL_IMC_CR_SC_CFG;	/* Scheduler configuration			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4088h & Channel1: 4488h */
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	:  3-0,
		CMD_Stretch	:  5-3,  /* 00:1N , 01:2N , 10:3N , 11:1N */
		N_to_1_ratio	:  8-5,  /* B2B cycles; Range:  1-7	*/
		Addr_Mirroring	: 12-8,
		tCPDED		: 15-12, /* Range:  1-7 @ 1N		*/
		ReservedBits2	: 28-15,
		x8_device_Dimm0 : 29-28, /* LSB is for DIMM 0		*/
		x8_device_Dimm1 : 30-29, /* MSB is for DIMM 1		*/
		NO_GEAR2_PRM_DIV: 31-30,
		GEAR2		: 32-31,
		DDR4_1_DPC	: 34-32,
		ReservedBits3	: 64-34;
	};
} RKL_IMC_SC_GS_CFG;	/* Scheduler configuration			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4088h & Channel1: 4488h */
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	:  3-0,
		CMD_Stretch	:  5-3,  /* 00:1N , 01:2N , 10:3N , 11:1N */
		N_to_1_ratio	:  8-5,  /* B2B cycles; Range:  1-7	*/
		Addr_Mirroring	: 12-8,
		tCPDED		: 15-12, /* Range:  1-7 @ 1N		*/
		ReservedBits2	: 28-15,
		x8_device_Dimm0 : 29-28, /* LSB is for DIMM 0		*/
		x8_device_Dimm1 : 30-29, /* MSB is for DIMM 1		*/
		NO_GEAR2_PRM_DIV: 31-30,
		GEAR2		: 32-31,
		DDR4_1_DPC	: 34-32,
		ReservedBits3	: 47-34,
		WRITE0_EN	: 48-47, /* 1: Enable for power saving	*/
		ReservedBits4	: 51-48,
		WCK_DIFF_LOW_IDLE:52-51,
		ReservedBits5	: 64-52;
	};
} TGL_IMC_SC_GS_CFG;	/* Scheduler configuration			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E088h & Channel1: E888h */
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	:  3-0,
		CMD_Stretch	:  5-3,  /* 00:1N , 01:2N , 10:3N , 11:1N */
		N_to_1_ratio	:  8-5,  /* B2B cycles; Range:  1-7	*/
		Addr_Mirroring	: 12-8,
		ReservedBits2	: 15-12,
		GEAR4		: 16-15, /* Gear4 Mode			*/
		NO_GEAR4_PRM_DIV: 17-16, /* 0:RU[param]/4;1:RU[param]/2 */
		ReservedBits3	: 28-17,
		x8_device_Dimm0 : 29-28, /* LSB is for DIMM 0		*/
		x8_device_Dimm1 : 30-29, /* MSB is for DIMM 1		*/
		NO_GEAR2_PRM_DIV: 31-30,
		GEAR2		: 32-31,
		DDR_1_DPC	: 34-32, /* DDR5 values = {0x1, 0x2}	*/
		ReservedBits4	: 49-34,
		WRITE0_EN	: 50-49, /* 1: Enable for power saving	*/
		ReservedBits5	: 54-50,
		WCK_DIFF_LOW_IDLE:55-54,
		ReservedBits6	: 56-55,
		tCPDED		: 61-56, /* Range:  1-7 @ 1N		*/
		ReservedBits7	: 64-61;
	};
} ADL_IMC_SC_GS_CFG;	/* Scheduler configuration			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4050h & Channel1: 4450h */
	unsigned long long	value;
	struct {
		unsigned long long
		tCKE		:  5-0,  /* Range:  4-16		*/
		ReservedBits1	:  8-5,
		tXP		: 13-8,  /* Range:  4-16		*/
		ReservedBits2	: 16-13,
		tXPDLL		: 22-16, /* Range:  4-63		*/
		tPRPDEN 	: 24-22, /* Range:  1-3 		*/
		tRDPDEN 	: 31-24, /* Range:  4-95		*/
		ReservedBits3	: 32-31,
		tWRPDEN 	: 40-32, /* Range:  4-159		*/
		ReservedBits4	: 64-40;
	};
} RKL_IMC_TC_PWDEN;	/* Power Down Timing				*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4050h & Channel1: 4450h */
	unsigned long long	value;
	struct {
		unsigned long long
		tCKE		:  6-0,  /* Range:  4-24		*/
		ReservedBits1	:  8-6,
		tXP		: 14-8,  /* Range:  4-24		*/
		ReservedBits2	: 16-14,
		tXPDLL		: 22-16, /* Range:  4-63		*/
		ReservedBits3	: 24-22,
		tRDPDEN 	: 31-24, /* Range:  4-100		*/
		ReservedBits4	: 32-31,
		tWRPDEN 	: 41-32, /* Range:  4-204		*/
		tCSH		: 46-41,
		tCSL		: 51-46,
		ReservedBits5	: 56-51,
		tPRPDEN 	: 61-56,
		ReservedBits6	: 64-61;
	};
} TGL_IMC_TC_PWDEN;	/* Power Down Timing				*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E050h & Channel1: E850h */
	unsigned long long	value;
	struct {
		unsigned long long
		tCKE		:  7-0,  /* Range:  4-24		*/
		tXP		: 14-7,  /* Range:  4-24		*/
		tXPDLL		: 21-14, /* Range:  4-63		*/
		tRDPDEN 	: 29-21, /* Range:  4-100		*/
		ReservedBits1	: 32-29,
		tWRPDEN 	: 42-32, /* Range:  4-204		*/
		tCSH		: 48-42,
		tCSL		: 54-48,
		ReservedBits2	: 59-54,
		tPRPDEN 	: 64-59;
	};
} ADL_IMC_TC_PWDEN;	/* Power Down Timing				*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4070h & Channel1: 4470h */
	unsigned int		value;
	struct {
		unsigned int
		Read_Duration	:  3-0,
		ReservedBits1	:  4-3,
		Read_Delay	:  7-4,
		ReservedBits2	:  8-7,
		Write_Duration	: 11-8,
		ReservedBits3	: 12-11,
		Write_Delay	: 15-12,
		Write_Early	: 16-15,
		tCL		: 21-16,
		tCWL		: 26-21,
		tAONPD		: 31-26,
		Always_Rank0	: 32-31;
	};
} SKL_IMC_CR_TC_ODT;	/* ODT timing parameters			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4070h & Channel1: 4470h */
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	: 16-0,
		tCL		: 22-16, /* Range:  4-36		*/
		tCWL		: 28-22, /* LPDDR4: 4-34; DDR4: 5-34 @ 1N */
		ReservedBits2	: 64-28;
	};
} RKL_IMC_CR_TC_ODT;	/* ODT timing parameters			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 4070h & Channel1: 4470h */
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	: 16-0,
		tCL		: 23-16, /* Range:  4-72		*/
		tCWL		: 30-23, /* LPDDR4: 4-64; DDR4: 5-64 @ 1N */
		ReservedBits2	: 64-30;
	};
} TGL_IMC_CR_TC_ODT;	/* ODT timing parameters			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E070h & Channel1: E870h */
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	: 16-0,
		tCL		: 23-16, /* Range:  4-72		*/
		ReservedBits2	: 24-23,
		tCWL		: 32-24, /* LPDDR4: 4-64; DDR4: 5-64 @ 1N */
		ReservedBits3	: 64-32;
	};
} ADL_IMC_CR_TC_ODT;	/* ODT timing parameters			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 423Ch & Channel1: 463Ch */
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 26-16,
		ReservedBits	: 32-26;
	};
} SKL_IMC_REFRESH_TC;	/* Refresh timing parameters			*/

#define RKL_IMC_REFRESH_TC	SKL_IMC_REFRESH_TC

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 423Ch & Channel1: 463Ch */
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 17-0,
		tRFC		: 29-17,
		ReservedBits	: 32-29;
	};
} TGL_IMC_REFRESH_TC;	/* Refresh timing parameters			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E43Ch & Channel1: EC3Ch */
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 18-0,
		tRFC		: 31-18,
		ReservedBits	: 32-31;
	};
} ADL_IMC_REFRESH_TC;	/* Refresh timing parameters			*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 42C4h & Channel1: 46C4h */
	unsigned int		value;
	struct {
		unsigned int
		tXSR		: 10-0,
		ReservedBits	: 32-10;
	};
} RKL_IMC_SREXITTP;	/* Self-Refresh Exit timing parameters		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 42C0h & Channel1: 46C0h */
	unsigned long long	value;
	struct {
		unsigned long long
		tXSR		: 12-0,
		ReservedBits	: 64-12;
	};
} TGL_IMC_SREXITTP;	/* Self-Refresh Exit timing parameters		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: E4C0h & Channel1: ECC0h */
	unsigned long long	value;
	struct {
		unsigned long long
		tXSR		: 13-0,
		ReservedBits1	: 52-13,
		tSR		: 58-52,
		ReservedBits2	: 64-58;
	};
} ADL_IMC_SREXITTP;	/* Self-Refresh Exit timing parameters		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset 5000h			*/
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  2-0,  /* 00:DDR4, 01:DDR3, 10:LPDDR3 */
		ReservedBits1	:  4-2,
		CH_L_MAP	:  5-4,  /* 0:Channel0 , 1:Channel1	*/
		ReservedBits2	: 12-5,
		CH_S_SIZE	: 19-12,/*Channel S size in multiplies of 1GB*/
		ReservedBits3	: 32-19;
	};
} SKL_IMC_MAD_MAPPING;

typedef union
{	/* Device: 0 - Function: 0 - Offset 5000h			*/
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  3-0,  /* 000:DDR4, 001:DDR5		*/
		ECHM		:  4-3,  /* LPDDR4: 2x32 or 1x64-bits channel */
		CH_L_MAP	:  5-4,  /* 0:Channel0 , 1:Channel1	*/
		ReservedBits1	: 12-5,
		CH_S_SIZE	: 20-12, /* Channel S size multiplies of 0.5GB*/
		ReservedBits2	: 27-20,
		CH_WIDTH	: 29-27, /* 0=x16, 1=x32, 2=x64, 3=Rsvd */
		ReservedBits3	: 32-29;
	};
} RKL_IMC_MAD_MAPPING;

typedef union
{	/* Device: 0 - Function: 0 - Offset 5000h			*/
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  3-0,  /* 0=DDR4, 1=DDR5, 2=LPDDR5, 3=LPDDR4*/
		ReservedBits1	:  4-3,
		CH_L_MAP	:  5-4,  /* 0:Channel0 , 1:Channel1	*/
		ReservedBits2	: 12-5,
		CH_S_SIZE	: 20-12, /* Channel S size, unit=512MB, 0-64GB*/
		ReservedBits3	: 27-20,
		CH_WIDTH	: 29-27, /* 0=x16, 1=x32, 2=x64, 3=Rsvd */
		ReservedBits4	: 31-29,
		HALF_CL_MODE	: 32-31; /* Half Cacheline Mode w/ 32B chunks */
	};
} TGL_IMC_MAD_MAPPING;

typedef union
{	/* Device: 0 - Function: 0 - Offset D800h			*/
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  3-0,  /* 0=DDR4, 1=DDR5, 2=LPDDR5, 3=LPDDR4*/
		ReservedBits1	:  4-3,
		CH_L_MAP	:  5-4,  /* 0:Channel0 , 1:Channel1	*/
		ReservedBits2	: 12-5,
		CH_S_SIZE	: 20-12, /* Channel S size, unit=512MB, 0-64GB*/
		ReservedBits3	: 27-20,
		CH_WIDTH	: 29-27, /* 0=x16, 1=x32, 2=x64, 3=Rsvd */
		ReservedBits4	: 31-29,
		HALF_CL_MODE	: 32-31; /* Half Cacheline Mode w/ 32B chunks */
	};
} ADL_IMC_MAD_MAPPING;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 5004h & Channel1: 5008h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Map	:  1-0,  /* DIMM L mapping: 0=DIMM0, 1=DIMM1 */
		ReservedBits1	:  4-1,
		RI		:  5-4,  /* Rank interleaving enable	*/
		ReservedBits2	:  8-5,
		EIM		:  9-8,  /* Enhanced mode enable	*/
		ReservedBits3	: 12-9,
		ECC		: 14-12,
		ReservedBits4	: 24-14,
		HORI		: 25-24, /* High order RI enable	*/
		ReservedBits5	: 28-25,
		HORI_ADDR	: 31-28,
		ReservedBits6	: 32-31;
	};
} SKL_IMC_MAD_CHANNEL;

#define RKL_IMC_MAD_CHANNEL	SKL_IMC_MAD_CHANNEL

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 5004h & Channel1: 5008h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Map	:  1-0,  /* DIMM L mapping: 0=DIMM0, 1=DIMM1 */
		ReservedBits1	:  8-1,
		EIM		:  9-8,  /* Enhanced mode enable	*/
		ReservedBits2	: 12-9,
		ECC		: 14-12,
		ReservedBits3	: 32-14;
	};
} TGL_IMC_MAD_CHANNEL;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: D804h & Channel1: D808h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Map	:  1-0,  /* DIMM L mapping: 0=DIMM0, 1=DIMM1 */
		ReservedBits1	:  8-1,
		EIM		:  9-8,  /* Enhanced mode enable	*/
		ReservedBits2	: 12-9,
		ECC		: 14-12,
		CRC		: 15-14, /* 1 = CRC Mode is enabled	*/
		ReservedBits3	: 32-15;
	};
} ADL_IMC_MAD_CHANNEL;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 500Ch & Channel1: 5010h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  6-0,  /* Size of DIMM in 1024 MB multiples */
		ReservedBits1	:  8-6,
		DLW		: 10-8, /*DIMM L chips width:0=x8,1=x16,10=x32*/
		DLNOR		: 11-10, /* DIMM L number of ranks	*/
		DL8Gb		: 12-11, /* DDR3 DIMM L: 1=8Gb or not 8Gb */
		ReservedBits2	: 16-12,
		Dimm_S_Size	: 22-16,
		ReservedBits3	: 24-22,
		DSW		: 26-24, /* DIMM S chips width		*/
		DSNOR		: 27-26, /* DIMM S # of ranks: 0=1 , 1=2 */
		DS8Gb		: 28-27,
		ReservedBits4	: 32-28;
	}; /* Gen: 6-UY, 6-H, 7-UY, 7-SX, 8-UQ, 8-UHS, 9-HS, 10-HS(Comet) */
} SKL_IMC_MAD_DIMM;

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 500Ch & Channel1: 5010h */
	/* Gen: Comet(10); Rocket(11); Tiger(11)			*/
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  7-0,  /* Size of DIMM in 512 MB multiples */
		DLW		:  9-7,  /* DIMM L width: 0=x8, 1=x16, 2=x32 */
		DLNOR		: 11-9,  /* DIMM L ranks: 0=1, 1=2, 2=3, 3=4 */
		ReservedBits1	: 16-11,
		Dimm_S_Size	: 23-16, /* DIMM S size in 512 MB multiples */
		ReservedBits2	: 24-23,
		DSW		: 26-24, /* DIMM S width: 0=x8, 1=x16, 2=x32 */
		DSNOR		: 28-26, /* DIMM S # of ranks: 0=1 , 1=2 */
		ReservedBits3	: 29-28,
		DLS_BG0_BIT_11	: 30-29,
		ReservedBits4	: 32-30;
	}; /* Gen: 10-U, 11-S, 11-WS, 11-UP3-UP4-H35, 11-H		*/
} CML_U_IMC_MAD_DIMM;

#define RKL_IMC_MAD_DIMM	CML_U_IMC_MAD_DIMM

#define TGL_IMC_MAD_DIMM	RKL_IMC_MAD_DIMM

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: D80Ch & Channel1: D810h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  7-0,  /* Size of DIMM in 512 MB multiples */
		DLW		:  9-7,  /* DIMM L width: 0=x8, 1=x16, 2=x32 */
		DLNOR		: 11-9,  /* DIMM L ranks: 0=1, 1=2, 2=3, 3=4 */
		DDR5_DS_8GB	: 12-11, /* 1=8Gb , 0=more than 8Gb capacity */
		DDR5_DL_8GB	: 13-12,
		ReservedBits1	: 16-13,
		Dimm_S_Size	: 23-16, /* DIMM S size in 512 MB multiples */
		ReservedBits2	: 24-23,
		DSW		: 26-24, /* DIMM S width: 0=x8, 1=x16, 2=x32 */
		DSNOR		: 28-26, /* DIMM S # of ranks: 0=1 , 1=2 */
		BG0_BIT_OPTIONS : 30-28,
		DECODER_EBH	: 32-30; /* 1=Extended Bank Hashing enabled */
	};
} ADL_IMC_MAD_DIMM;

typedef union
{	/* Device: 0 - Function: 0 - Offset 5918h			*/
	unsigned int		value;
	struct {
		unsigned int
		QCLK		:  7-0,
		QCLK_REF	:  8-7,  /* 0=133Mhz, 1=100Mhz		*/
		FCLK		: 16-8,
		ICLK		: 24-16,
		UCLK		: 32-24;
	};
} SKL_SA_PLL_RATIOS;	/* 06_4E/06_5E/06_9E				*/

typedef union
{	/* Device: 0 - Function: 0 - Offset 5918h			*/
	unsigned long long	value;
	struct {
		unsigned long long
		LAST_DE_WP_REQ_SERVED	:   2-0,
		QCLK_RATIO		:  10-2, /* DDR QCLK Ratio	*/
		QCLK_REF		: 11-10, /* 0=400/3Mhz; 1=100Mhz*/
		OPI_LINK_SPEED		: 12-11, /* 0=2Gb/s ; 1=4Gb/s	*/
		IPU_IS_DIVISOR		: 18-12, /* = 1600MHz / Divisor */
		IPU_PS_RATIO		: 24-18, /* = 25Mhz * Ratio	*/
		UCLK_RATIO		: 32-24, /* Ring UCLK at 100MHz */
		PSF0_RATIO		: 40-32, /* = 16.67 MHz * Ratio */
		SA_VOLTAGE		: 56-40, /* = 1/8192V		*/
		ReservedBits		: 64-56;
	};
} RKL_SA_PERF_STATUS;

#define TGL_SA_PERF_STATUS	RKL_SA_PERF_STATUS

#define ADL_SA_PERF_STATUS	TGL_SA_PERF_STATUS

typedef union
{	/* Device: 0 - Function: 0 - Offset E4h 			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 12-0,
		PDCD		: 13-12, /* 0:Capable Dual Channels, 1:Single */
		X2APIC_EN	: 14-13, /* 1: Supports Extended APIC mode */
		DDPCD		: 15-14,
		ReservedBits2	: 23-15,
		VT_d		: 24-23,
		ReservedBits3	: 25-24,
		ECCDIS		: 26-25, /* 0:ECC capable, 1:Not ECC capable */
		ReservedBits4	: 32-26;
	};	/* §3.39 CAPID0_A Capabilities A Register		*/
} SKL_CAPID_A;

typedef union
{	/* Device: 0 - Function: 0 - Offset E4h 			*/
	unsigned int		value;
	struct {
		unsigned int
		NVME_F7D	:  1-0,  /* 1: Disable NVMe at Dev 3 Func 7 */
		ReservedBits1	:  3-1,
		DDR_OVERCLOCK	:  4-3,  /* 1: Enable Memory Overclocking */
		CRID		:  8-4,  /* Compatibility Revision ID	*/
		ReservedBits2	: 10-8,
		DID0OE		: 11-10, /* 1: Allow DID override	*/
		IGD		: 12-11, /* 1: Disable iGPU		*/
		PDCD		: 13-12, /* 0:Capable Dual Channels, 1:Single */
		X2APIC_EN	: 14-13, /* 1: Supports Extended APIC mode */
		DDPCD		: 15-14,
		CDD		: 16-15, /* 1: Disable DTT device	*/
		ReservedBits3	: 17-16,
		D1NM		: 18-17, /* 1: Disable DRAM 1N Timing	*/
		PEG60D		: 19-18, /* 1: Disable PCIe at Dev 6 Func 0 */
		DDRSZ		: 21-19, /* 0=64GB, 1=8GB, 2=4GB, 3=2GB */
		PEGG2DIS	: 22-21, /* 1: DMI Gen2 PCIe disabled	*/
		DMIG2DIS	: 23-22, /* 1: DMI Gen2 mode disabled	*/
		VT_d		: 24-23, /* 1: VT-d is no supported	*/
		FDEE		: 25-24, /* Force DRAM ECC Enable	*/
		ECCDIS		: 26-25, /* 0:ECC capable, 1:Not ECC capable */
		DW		: 27-26, /* DMI Width: 0=x4 , 1=x2	*/
		PELWUD		: 28-27, /* 0: PCIe Link Width Up-Config */
		PEG10D		: 29-28, /* 1: Disable PCIe at Dev 1 Func 0 */
		PEG11D		: 30-29,
		PEG12D		: 31-30,
		NVME_FOD	: 32-31; /* 1: Disable NVMe at Dev 3 Func 0 */
	};	/* RKL: §3.1.38 ; TGL: $3.1.40				*/
} RKL_CAPID_A;

#define TGL_CAPID_A	RKL_CAPID_A

typedef union
{	/* Device: 0 - Function: 0 - Offset E4h 			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  1-0,
		NVME_F7D	:  2-1,
		ReservedBits2	:  3-2,
		DDR_OVERCLOCK	:  4-3,
		CRID		:  8-4,
		TWOLM_SUPPORTED :  9-8,
		ReservedBits3	: 10-9,
		DID0OE		: 11-10,
		IGD		: 12-11,
		PDCD		: 13-12,
		X2APIC_EN	: 14-13,
		DDPCD		: 15-14,
		CDD		: 16-15,
		ReservedBits4	: 17-16,
		D1NM		: 18-17,
		PEG60D		: 19-18,
		DDRSZ		: 21-19,
		ReservedBits5	: 22-21,
		DMIG2DIS	: 23-22,
		VT_d		: 24-23,
		FDEE		: 25-24,
		ECCDIS		: 26-25,
		DW		: 27-26,
		PELWUD		: 28-27,
		PEG10D		: 29-28,
		PEG11D		: 30-29,
		PEG12D		: 31-30,
		NVME_FOD	: 32-31;
	};
} ADL_CAPID_A;

typedef union
{	/* Device: 0 - Function: 0 - Offset E8h 			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  2-0,
		LPDDR3_EN	:  3-2,  /* Allow LPDDR3 operation	*/
		ReservedBits2	:  4-3,
		DMFC_DDR3	:  7-4,
		ReservedBits3	:  8-7,
		GMM_DIS 	:  9-8,  /* Device 8 associated memory spaces */
		ReservedBits4	: 15-9,
		DMIG3DIS	: 16-15, /* DMI Gen 3 Disable fuse	*/
		ReservedBits5	: 17-16,
		ADDGFXCAP	: 18-17,
		ADDGFXEN	: 19-18,
		ReservedBits6	: 20-19,
		PEGG3_DIS	: 21-20,
		PLL_REF100_CFG	: 24-21,
		ReservedBits7	: 25-24,
		CACHESZ 	: 28-25,
		SMTCAP		: 29-28,
		ReservedBits8	: 31-29,
		IMGU_DIS	: 32-31; /* Device 5 associated memory spaces */
	};	/* §3.40 CAPID0_B Capabilities B Register		*/
} SKL_CAPID_B;

typedef union
{	/* Device: 0 - Function: 0 - Offset E8h 			*/
	unsigned int		value;
	struct {
		unsigned int
		SPEGFX1 	:  1-0,
		DPEGFX1 	:  2-1,
		VMD_DIS 	:  3-2,  /* 1: Disable VMD		*/
		SH_OPI_EN	:  4-3,  /* 0=DMI or 1=OPI		*/
		ReservedBits1	:  7-4,
		DDD		:  8-7,  /* Mode 0=Debug or 1=Production */
		GNA_DIS 	:  9-8,  /* 1: Disable Dev 8		*/
		ReservedBits2	: 10-9,
		LVL_MEMORY	: 11-10, /* 0=1LM , 1=2LM		*/
		HDCPD		: 12-11, /* 1: HDCP is disabled 	*/
		LTECH		: 15-12, /* 0=1LM, 1=EDRAM0, 3=RAM0+1, 4:2LM */
		DMIG3DIS	: 16-15, /* DMI Gen 3 Disable fuse	*/
		PEGX16D 	: 17-16, /* x16 PCIe port is disabled	*/
		ADDGFXCAP	: 18-17,
		ADDGFXEN	: 19-18,
		PKGTYP		: 20-19, /* CPU Package Type		*/
		PEGG3_DIS	: 21-20,
		PLL_REF100_CFG	: 24-21,
		SVM_DISABLE	: 25-24,
		CACHESZ 	: 28-25,
		SMTCAP		: 29-28,
		OC_ENABLED	: 30-29, /* 0: Overclocking is disabled */
		TRACE_HUB_DIS	: 31-30, /* Trace Hub & I/O are disabled */
		IMGU_DIS	: 32-31; /* Device 5 associated memory spaces */
	};	/* RKL: §3.1.39 ; TGL: $3.1.41				*/
} RKL_CAPID_B;

#define TGL_CAPID_B	RKL_CAPID_B

typedef union
{	/* Device: 0 - Function: 0 - Offset E8h 			*/
	unsigned int		value;
	struct {
		unsigned int
		SPEGFX1 	:  1-0,
		DPEGFX1 	:  2-1,
		VMD_DIS 	:  3-2,
		SH_OPI_EN	:  4-3,
		ReservedBits1	:  7-4,
		DDD		:  8-7,
		GNA_DIS 	:  9-8,
		ReservedBits2	: 10-9,
		DEV10_DIS	: 11-10, /* 1: Device 10 disabled & locked */
		HDCPD		: 12-11,
		LTECH		: 15-12,
		DMIG3DIS	: 16-15,
		PEGX16D 	: 17-16,
		ReservedBits3	: 19-17,
		PKGTYP		: 20-19,
		PEGG3_DIS	: 21-20,
		PLL_REF100_CFG	: 24-21,
		SVM_DISABLE	: 25-24,
		CACHESZ 	: 28-25,
		SMTCAP		: 29-28,
		OC_ENABLED	: 30-29,
		TRACE_HUB_DIS	: 31-30,
		IMGU_DIS	: 32-31;
	};
} ADL_CAPID_B;

typedef union
{	/* Device: 0 - Function: 0 - Offset ECh 			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 14-0,
		DMFC_LPDDR3	: 17-14,
		DMFC_DDR4	: 20-17,
		ReservedBits2	: 32-20;
	};	/* §3.41 CAPID0_C Capabilities C Register		*/
} SKL_CAPID_C;

typedef union
{	/* Device: 0 - Function: 0 - Offset ECh 			*/
	unsigned int		value;
	struct {
		unsigned int
		DATA_RATE_GEAR1 :  5-0,  /* MC: mult 266 MHz iff DDR_OVERCLOCK*/
		DISPLAY_PIPE3	:  6-5,  /* 3rd display is enabled	*/
		IDD		:  7-6,  /* 1:Intel display is disabled */
		BCLK_OC_FREQ	:  9-7,  /* 0=Dis, 1=115, 2=130MHz, 3:NoLimit */
		SGX_DIS 	: 10-9,  /* 1:SGX is disabled		*/
		ReservedBits1	: 14-10,
		QCLK_GV_DIS	: 15-14, /* 1: Dyn Mem Freq Chg is disabled */
		IB_ECC		: 16-15,
		LPDDR4_EN	: 17-16, /* 1: LPDDR4 is supported	*/
		DATA_RATE_LPDDR4: 22-17, /* mult of 266 MHz iff DDR_OVERCLOCK */
		DDR4_EN 	: 23-22, /* 1: DDR4 is supported	*/
		DATA_RATE_DDR4	: 28-23, /* mult of 266 MHz iff DDR_OVERCLOCK */
		PEGG4_DIS	: 29-28,
		ReservedBits2	: 32-29;
	};	/* RKL: §3.1.40 ; TGL: $3.1.42				*/
} RKL_CAPID_C;

#define TGL_CAPID_C	RKL_CAPID_C

typedef union
{	/* Device: 0 - Function: 0 - Offset ECh 			*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  5-0,
		DISPLAY_PIPE3	:  6-5,
		IDD		:  7-6,
		BCLK_OC_FREQ	:  9-7,
		SGX_DIS 	: 10-9,
		ReservedBits2	: 14-10,
		QCLK_GV_DIS	: 15-14,
		ReservedBits3	: 16-15,
		LPDDR4_EN	: 17-16,
		DATA_RATE_LPDDR4: 22-17,
		DDR4_EN 	: 23-22,
		DATA_RATE_DDR4	: 28-23,
		PEGG4_DIS	: 29-28,
		PEGG5_DIS	: 30-29,
		PEG61_DIS	: 31-30,
		PEG62_DIS	: 32-31;
	};
} ADL_CAPID_C;

typedef union
{	/* Device: 0 - Function: 0 - Offset F0h 			*/
	unsigned int		value;
	struct {
		unsigned int
		LPDDR5_EN	:  1-0,
		DATA_RATE_LPDDR5:  6-1,
		DDR5_EN 	:  7-6,
		DATA_RATE_DDR5	: 12-7,
		IB_ECC_DIS	: 13-12, /* 1: IBECC is disabled	*/
		VDDQ_VOLTAGE	: 24-13, /* Max VID of VDDQ_TX		*/
		CRASHLOG_DIS	: 25-24, /* 1: Device 10 is disabled	*/
		ReservedBits	: 32-25;
	};	/* TGL: $3.1.43 					*/
} TGL_CAPID_E;

typedef union
{	/* Device: 0 - Function: 0 - Offset F0h 			*/
	unsigned int		value;
	struct {
		unsigned int
		LPDDR5_EN	:  1-0,
		DATA_RATE_LPDDR5:  6-1,
		DDR5_EN 	:  7-6,
		DATA_RATE_DDR5	: 12-7,
		IB_ECC_DIS	: 13-12,
		VDDQ_VOLTAGE	: 24-13,
		CRASHLOG_DIS	: 25-24,
		ReservedBits	: 32-25;
	};
} ADL_CAPID_E;
