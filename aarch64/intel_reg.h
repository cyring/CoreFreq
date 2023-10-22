/*
 * CoreFreq
 * Copyright (C) 2015-2023 CYRIL COURTIAT
 * Licenses: GPL2
 */
/*TODO(CleanUp)
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

**	MSR registers related to Hardware Duty Cycling (HDC)		**
#ifndef MSR_IA32_PKG_HDC_CTL
	#define MSR_IA32_PKG_HDC_CTL		0x00000db0
#endif

#ifndef MSR_IA32_PM_CTL1
	#define MSR_IA32_PM_CTL1		0x00000db1
#endif

#ifndef MSR_IA32_THREAD_STALL
	#define MSR_IA32_THREAD_STALL		0x00000db2
#endif

**	Source: Intel Atom Processor E3800 Product Family Datasheet	**
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

**	Source: Linux kernel /arch/x86/include/asm/msr-index.h		**
#ifndef MSR_ATOM_CORE_RATIOS
	#define MSR_ATOM_CORE_RATIOS		0x0000066a
#endif

#ifndef MSR_ATOM_CORE_VIDS
	#define MSR_ATOM_CORE_VIDS		0x0000066b
#endif

#ifndef MSR_ATOM_CORE_TURBO_RATIOS
	#define MSR_ATOM_CORE_TURBO_RATIOS	0x0000066c
#endif

#ifndef MSR_ATOM_CORE_TURBO_VIDS
	#define MSR_ATOM_CORE_TURBO_VIDS	0x0000066d
#endif

**	Package C-State Interrupt Response Limit			**
#ifndef MSR_PKGC3_IRTL
	#define MSR_PKGC3_IRTL			0x0000060a
#endif

#ifndef MSR_PKGC6_IRTL
	#define MSR_PKGC6_IRTL			0x0000060b
#endif

#ifndef MSR_PKGC7_IRTL
	#define MSR_PKGC7_IRTL			0x0000060c
#endif

**	Additional MSRs supported by 6th up to 13th Gen	and Xeon scalable **
#ifndef MSR_PPERF
	#define MSR_PPERF			0x0000064e
#endif

#ifndef MSR_ANY_CORE_C0
    #ifdef MSR_PKG_ANY_CORE_C0_RES
	#define MSR_ANY_CORE_C0 		MSR_PKG_ANY_CORE_C0_RES
    #else
	#define MSR_ANY_CORE_C0 		0x00000659
    #endif
#endif

#define MSR_CORE_UARCH_CTL		 	0x00000541

**	Partially documented registers					**
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
	struct
	{
		unsigned long long
		Bus_Speed	:  4-0,
		ReservedBits	: 64-4;
	} Airmont;
} FSB_FREQ;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		IBRS		:  1-0,
		STIBP		:  2-1,
		SSBD		:  3-2,
		IPRED_DIS_U	:  4-3,
		IPRED_DIS_S	:  5-4,
		RRSBA_DIS_U	:  6-5,
		RRSBA_DIS_S	:  7-6,
		PSFD		:  8-7,
		DDPD_U_DIS	:  9-8,
		ReservedBits1	: 10-9,
		BHI_DIS_S	: 11-10,
		ReservedBits2	: 64-11;
	};
} SPEC_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		IBPB		:  1-0,
		ReservedBits	: 64-1;
	};
} PRED_CMD;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		L1D_FLUSH_CMD	:  1-0,
		ReservedBits	: 64-1;
	};
} FLUSH_CMD;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ENERGY_FILTERING_CTL	:  1-0,
		ReservedBits		: 64-1;
	};
} MISC_PACKAGE_CTLS;

typedef union
{
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
		MISC_PACKAGE_CTLS_SUP	: 11-10,
		ENERGY_FILTERING_CTL_SUP: 12-11,
		DOITM_UARCH_MISC_CTRL	: 13-12,
		SBDR_SSDP_NO		: 14-13,
		FBSDP_NO		: 15-14,
		PSDP_NO 		: 16-15,
		ReservedBits2		: 17-16,
		FB_CLEAR		: 18-17,
		FB_CLEAR_CTRL		: 19-18,
		RRSBA			: 20-19,
		BHI_NO			: 21-20,
		XAPIC_DISABLE_STATUS_MSR: 22-21,
		ReservedBits3		: 23-22,
		OVERCLOCKING_STATUS_SUP : 24-23,
		PBRSB_NO		: 25-24,
		ReservedBits4		: 64-25;
	};
} ARCH_CAPABILITIES;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		LEGACY_XAPIC_DIS	:  1-0,
		ReservedBits		: 64-1;
	};
} XAPIC_DISABLE_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		DOITM			:  1-0,
		ReservedBits		: 64-1;
	};
} UARCH_MISC_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		_RNGDS_MITG_DIS :  1-0,
		_RTM_ALLOW	:  2-1,
		_RTM_LOCKED	:  3-2,
		_FB_CLEAR_DIS	:  4-3,
		ReservedBits	: 64-4;
	};
} MCU_OPT_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		STLB_SUPPORTED	:  1-0,
		ReservedBits1	:  2-1,
		FUSA_SUPPORTED	:  3-2,
		RSM_IN_CPL0_ONLY:  4-3,
		UC_LOCK_DIS_SUP :  5-4,
		SPLA_EXCEPTION	:  6-5,
		SNOOP_FILTER_SUP:  7-6,
		UC_STORE_THROT	:  8-7,
		ReservedBits2	: 64-8;
	};
} CORE_CAPABILITIES;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		RTM_DISABLE	:  1-0,
		TSX_CPUID_CLEAR :  2-1,
		ReservedBits	: 64-2;
	};
} TSX_CTRL;

typedef union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		UnknownBits1	: 16-0,
		OC_BINS 	: 24-16,
		UnknownBits2	: 64-24;
	};
} FLEX_RATIO;

typedef union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		OC_Utilized	:  1-0,
		Undervolt	:  2-1,
		OC_Unlock	:  3-2,
		ReservedBits	: 64-3;
	};
} OVERCLOCKING_STATUS;

typedef union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		CurrVID 	:  8-0,
		CurrFID 	: 16-8,
		ReservedBits1	: 31-16,
		XE_Enable	: 32-31,
		ReservedBits2	: 40-32,
		MaxBusRatio	: 45-40,
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
		CurrVID 	: 48-32,
		ReservedBits3	: 64-48;
	} SNB;
} PERF_STATUS;

typedef union
{
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
		Turbo_IDA	:  1-0,
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
		ReservedBits2	: 23-16,
		PPIN_CAP	: 24-23,
		ReservedBits3	: 28-24,
		ProgrammableTurbo:29-28,
		ProgrammableTDP : 30-29,
		ProgrammableTj	: 31-30,
		ReservedBits4	: 32-31,
		LowPowerMode	: 33-32,
		ConfigTDPlevels : 35-33,
		ReservedBits5	: 40-35,
		MinimumRatio	: 48-40,
		MinOperatRatio	: 56-48,
		ReservedBits6	: 64-56;
	};
} PLATFORM_INFO;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		L2_HW_Prefetch		:  1-0,
		L2_HW_CL_Prefetch	:  2-1,
		L1_HW_Prefetch		:  3-2,
		L1_HW_IP_Prefetch	:  4-3,
		ReservedBits1		: 11-4,
		DISABLE_THREE_STRIKE_CNT: 12-11,
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
		LockOut 	:  1-0,
		Enable		:  2-1,
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
		AutoCStateConv	: 17-16,
		ReservedBits3	: 24-17,
		Int_Filtering	: 25-24,
		C3autoDemotion	: 26-25,
		C1autoDemotion	: 27-26,
		C3undemotion	: 28-27,
		C1undemotion	: 29-28,
		PkgCSTdemotion	: 30-29,
		PkgCSTundemotion: 31-30,
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
		CStateRange	: 19-16,
		ReservedBits	: 64-19;
	};
} CSTATE_IO_MWAIT;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CC6demotion	: 64-0;
	};
} CC6_CONFIG;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		MC6demotion	: 64-0;
	};
} MC6_CONFIG;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		MaxRatio_1C	:  8-0,
		MaxRatio_2C	: 16-8,
		MaxRatio_3C	: 24-16,
		MaxRatio_4C	: 32-24,
		MaxRatio_5C	: 40-32,
		MaxRatio_6C	: 48-40,
		MaxRatio_7C	: 56-48,
		MaxRatio_8C	: 64-56;
	};
} TURBO_RATIO_CONFIG0;

typedef union
{
	unsigned long long	value;
	struct
	{
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
	{
		unsigned long long
		MaxRatio_9C	:  8-0,
		MaxRatio_10C	: 16-8,
		MaxRatio_11C	: 24-16,
		MaxRatio_12C	: 32-24,
		MaxRatio_13C	: 40-32,
		MaxRatio_14C	: 48-40,
		MaxRatio_15C	: 56-48,
		ReservedBits	: 63-56,
		Semaphore	: 64-63;
	} IVB_EP;
	struct
	{
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
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		MaxRatio_17C	:  8-0,
		MaxRatio_18C	: 16-8,
		ReservedBits	: 63-16,
		Semaphore	: 64-63;
	};
} TURBO_RATIO_CONFIG2;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits	: 63-0,
		Semaphore	: 64-63;
	};
} TURBO_RATIO_CONFIG3;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned int
		Ratio		:  8-0,
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
	{
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
{
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

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		OVERRIDE_EN	:  1-0,
		SOC_TURBO_EN	:  2-1,
		SOC_TDP_POLICY	:  5-2, 
		RESERVED_4	: 32-5; 
	};
} ATOM_TURBO_SOC_OVERRIDE;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		FastStrings	:  1-0,
		ReservedBits1	:  2-1,
		x87Compat_Enable:  3-2,
		TCC		:  4-3,
		SplitLockDisable:  5-4,
		ReservedBits2	:  6-5,
		L3Cache_Disable :  7-6,
		PerfMonitoring	:  8-7,
		SupprLock_Enable:  9-8,
		HW_Prefetch_Dis : 10-9,
		Int_FERR_Enable : 11-10,
		BTS		: 12-11,
		PEBS		: 13-12,
		TM2_Enable	: 14-13,
		ReservedBits3	: 16-14,
		EIST		: 17-16,
		BR_PROCHOT	: 18-17,
		FSM		: 19-18,
		PrefetchCacheDis: 20-19,
		ReservedBits4	: 22-20,
		CPUID_MaxVal	: 23-22,
		xTPR		: 24-23,
		L1DataCacheMode : 25-24,
		ReservedBits5	: 26-25,
		C2E		: 27-26,
		ReservedBits6	: 32-27,
		C4E		: 34-32,
		XD_Bit_Disable	: 35-34,
		ReservedBits7	: 37-35,
		DCU_L1_Prefetch : 38-37,
		Turbo_IDA	: 39-38,
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
		BD_PROCHOT	:  1-0,
		C1E		:  2-1,
		ReservedBits1	: 19-2,
		R2H_Disable	: 20-19,
		EEO_Disable	: 21-20,
		ReservedBits2	: 25-21,
		EBP_OS_Control	: 26-25,
		ReservedBits3	: 30-26,
		CST_PreWake_Dis : 31-30,
		ReservedBits4	: 64-31;
	};
} POWER_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		DutyCycle	:  4-0,
		ODCM_Enable	:  5-4,
		ReservedBits	: 63-5,
		ECMD		: 64-63;
	};
} CLOCK_MODULATION;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PowerPolicy	:  4-0,
		ReservedBits	: 64-4;
	};
} ENERGY_PERF_BIAS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		HW_Coord_EIST	:  1-0,
		Perf_BIAS_Enable:  2-1,
		ReservedBits1	: 10-2,
		ENABLE_SDC	: 11-10,
		ReservedBits2	: 13-11,
		LOCK		: 14-13,
		ReservedBits3	: 22-14,
		Therm_Intr_Coord: 23-22,
		ReservedBits4	: 64-23;
	};
} MISC_PWR_MGMT;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		HWP_Enable	:  1-0,
		ReservedBits	: 64-1;
	};
} PM_ENABLE;
*/
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
/*
typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		HDC_Enable	:  1-0,
		ReservedBits	: 64-1;
	};
} HDC_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0 ,
		Overflow_PMC1	:  2-1 ,
		Overflow_PMC2	:  3-2 ,
		Overflow_PMC3	:  4-3 ,
		Overflow_PMCn	: 32-4 ,
		Overflow_CTR0	: 33-32,
		Overflow_CTR1	: 34-33,
		Overflow_CTR2	: 35-34,
		ReservedBits1	: 55-35,
		TraceToPAPMI	: 56-55,
		ReservedBits2	: 58-56,
		LBR_Frz 	: 59-58,
		CTR_Frz 	: 60-59,
		ASCI		: 61-60,
		Overflow_UNC	: 62-61,
		Overflow_Buf	: 63-62,
		Ovf_CondChg	: 64-63;
	};
} CORE_GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0 ,
		Clear_Ovf_PMC1	:  2-1 ,
		Clear_Ovf_PMC2	:  3-2 ,
		Clear_Ovf_PMC3	:  4-3 ,
		Clear_Ovf_PMCn	: 32-4 ,
		Clear_Ovf_CTR0	: 33-32,
		Clear_Ovf_CTR1	: 34-33,
		Clear_Ovf_CTR2	: 35-34,
		ReservedBits1	: 55-35,
		ClrTraceToPA_PMI: 56-55,
		ReservedBits2	: 61-56,
		Clear_Ovf_UNC	: 62-61,
		Clear_Ovf_Buf	: 63-62,
		Clear_CondChg	: 64-63;
	};
} CORE_GLOBAL_PERF_OVF_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0 	:  1-0 ,
		EN_PMC1 	:  2-1 ,
		EN_PMC2 	:  3-2 ,
		EN_PMC3 	:  4-3 ,
		EN_PMCn 	: 32-4 ,
		EN_FIXED_CTR0	: 33-32,
		EN_FIXED_CTR1	: 34-33,
		EN_FIXED_CTR2	: 35-34,
		ReservedBits	: 64-35;
	};
} CORE_GLOBAL_PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN0_OS		:  1-0 ,
		EN0_Usr 	:  2-1 ,
		AnyThread_EN0	:  3-2 ,
		EN0_PMI 	:  4-3 ,
		EN1_OS		:  5-4 ,
		EN1_Usr 	:  6-5 ,
		AnyThread_EN1	:  7-6 ,
		EN1_PMI 	:  8-7 ,
		EN2_OS		:  9-8 ,
		EN2_Usr 	: 10-9 ,
		AnyThread_EN2	: 11-10,
		EN2_PMI 	: 12-11,
		ReservedBits	: 64-12;
	};
} CORE_FIXED_PERF_CONTROL;

typedef union
{
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
		PLN_Enable	: 25-24,
		HWP_Interrupt	: 26-25,
		ReservedBits2	: 64-26;
	};
} THERM_INTERRUPT;

typedef union
{
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
		CurLimit_Status : 13-12,
		CurLimit_Log	: 14-13,
		XDomLimit_Status: 15-14,
		XDomLimit_Log	: 16-15,
		DTS		: 23-16,
		ReservedBits1	: 26-23,
		HWP_Status	: 27-26,
		Resolution	: 31-27,
		ReadingValid	: 32-31,
		ReservedBits2	: 64-32;
	};
} THERM_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PROCHOT_Event	:  1-0,
		Thermal_Status	:  2-1,
		ReservedBits1	:  4-2,
		Residency_Sts	:  5-4,
		AvgThmLimit	:  6-5,
		VR_ThmAlert	:  7-6,
		VR_TDC_Status	:  8-7,
		EDP_Status	:  9-8,
		ReservedBits2	: 10-9,
		PL1_Status	: 11-10,
		PL2_Status	: 12-11,
		TurboLimit	: 13-12,
		TurboAtten	: 14-13,
		TVB_Status	: 15-14,
		ReservedBits3	: 16-15,
		PROCHOT_Log	: 17-16,
		Thermal_Log	: 18-17,
		ReservedBits4	: 20-18,
		Residency_Log	: 21-20,
		AvgThmLimitLog	: 22-21,
		VR_ThmAlertLog	: 23-22,
		VR_TDC_Log	: 24-23,
		EDP_Log 	: 25-24,
		ReservedBits5	: 26-25,
		PL1_Log 	: 27-26,
		PL2_Log 	: 28-27,
		TurboLimitLog	: 29-28,
		TurboAttenLog	: 30-29,
		TVB_Log 	: 31-30,
		ReservedBits6	: 64-31;
	};
} CORE_PERF_LIMIT_REASONS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PROCHOT_Event	:  1-0,
		Thermal_Status	:  2-1,
		ReservedBits1	:  5-2,
		AvgThmLimit	:  6-5,
		VR_ThmAlert	:  7-6,
		VR_TDC_Status	:  8-7,
		EDP_Status	:  9-8,
		ReservedBits2	: 10-9,
		PL1_Status	: 11-10,
		PL2_Status	: 12-11,
		Inefficiency	: 13-12,
		ReservedBits3	: 16-13,
		PROCHOT_Log	: 17-16,
		Thermal_Log	: 18-17,
		ReservedBits4	: 21-18,
		AvgThmLimitLog	: 22-21,
		VR_ThmAlertLog	: 23-22,
		VR_TDC_Log	: 24-23,
		EDP_Log 	: 25-24,
		ReservedBits5	: 26-25,
		PL1_Log 	: 27-26,
		PL2_Log 	: 28-27,
		InefficiencyLog : 29-28,
		ReservedBits6	: 64-28;
	};
} GRAPHICS_PERF_LIMIT_REASONS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PROCHOT_Event	:  1-0,
		Thermal_Status	:  2-1,
		ReservedBits1	:  5-2,
		AvgThmLimit	:  6-5,
		VR_ThmAlert	:  7-6,
		VR_TDC_Status	:  8-7,
		EDP_Status	:  9-8,
		ReservedBits2	: 10-9,
		PL1_Status	: 11-10,
		PL2_Status	: 12-11,
		ReservedBits3	: 16-12,
		PROCHOT_Log	: 17-16,
		Thermal_Log	: 18-17,
		ReservedBits4	: 21-18,
		AvgThmLimitLog	: 22-21,
		VR_ThmAlertLog	: 23-22,
		VR_TDC_Log	: 24-23,
		EDP_Log 	: 25-24,
		ReservedBits5	: 26-25,
		PL1_Log 	: 27-26,
		PL2_Log 	: 28-27,
		ReservedBits6	: 64-28;
	};
} RING_PERF_LIMIT_REASONS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		TM2_Target	: 16-0,
		TM_SELECT	: 17-16,
		ReservedBits	: 64-17;
	};
} THERM2_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		Target		: 24-16,
		Offset		: 28-24,
		ReservedBits2	: 64-28;
	};
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		Target		: 24-16,
		Offset		: 30-24,
		ReservedBits2	: 64-30;
	} Atom;
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		Target		: 24-16,
		Offset		: 28-24,
		ReservedBits2	: 64-28;
	} EP;
} TJMAX;


typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
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
		Overflow_PMC0	:  1-0 ,
		Overflow_PMC1	:  2-1 ,
		Overflow_PMC2	:  3-2 ,
		Overflow_PMC3	:  4-3 ,
		Overflow_PMC4	:  5-4 ,
		Overflow_PMC5	:  6-5 ,
		Overflow_PMC6	:  7-6 ,
		Overflow_PMC7	:  8-7 ,
		ReservedBits1	: 32-8 ,
		Overflow_CTR0	: 33-32,
		ReservedBits2	: 61-33,
		Overflow_PMI	: 62-61,
		Ovf_DSBuffer	: 63-62,
		Ovf_CondChg	: 64-63;
	} NHM;
	struct
	{
		unsigned long long
		Overflow_CTR0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_PMC0	:  4-3,
		ReservedBits2	: 64-4;
	} SNB;
	struct
	{
		unsigned long long
		Overflow_CTR0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_PMC0	:  4-3,
		ReservedBits2	: 64-4;
	} SKL;
	struct
	{
		unsigned long long
		Overflow_CTR0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_PMC0	:  4-3,
		ReservedBits2	: 64-4;
	} ADL;
} UNCORE_GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0 ,
		Clear_Ovf_PMC1	:  2-1 ,
		Clear_Ovf_PMC2	:  3-2 ,
		Clear_Ovf_PMC3	:  4-3 ,
		Clear_Ovf_PMC4	:  5-4 ,
		Clear_Ovf_PMC5	:  6-5 ,
		Clear_Ovf_PMC6	:  7-6 ,
		Clear_Ovf_PMC7	:  8-7 ,
		ReservedBits1	: 32-8 ,
		Clear_Ovf_CTR0	: 33-32,
		ReservedBits2	: 61-33,
		Clear_Ovf_PMI	: 62-61,
		Clear_Ovf_DSBuf : 63-62,
		Clear_CondChg	: 64-63;
	};
} UNCORE_GLOBAL_PERF_OVF_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0 	:  1-0 ,
		EN_PMC1 	:  2-1 ,
		EN_PMC2 	:  3-2 ,
		EN_PMC3 	:  4-3 ,
		EN_PMC4 	:  5-4 ,
		EN_PMC5 	:  6-5 ,
		EN_PMC6 	:  7-6 ,
		EN_PMC7 	:  8-7 ,
		ReservedBits1	: 32-8 ,
		EN_FIXED_CTR0	: 33-32,
		ReservedBits2	: 48-33,
		EN_PMI_CORE0	: 49-48,
		EN_PMI_CORE1	: 50-49,
		EN_PMI_CORE2	: 51-50,
		EN_PMI_CORE3	: 52-51,
		ReservedBits3	: 63-52,
		PMI_FRZ 	: 64-63;
	} NHM;
	struct
	{
		unsigned long long
		EN_PMI_CORE0	:  1-0 ,
		EN_PMI_CORE1	:  2-1 ,
		EN_PMI_CORE2	:  3-2 ,
		EN_PMI_CORE3	:  4-3 ,
		ReservedBits1	: 29-4 ,
		EN_FIXED_CTR0	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ 	: 32-31,
		ReservedBits2	: 64-32;
	} SNB;
	struct
	{
		unsigned long long
		EN_PMI_CORE0	:  1-0 ,
		EN_PMI_CORE1	:  2-1 ,
		EN_PMI_CORE2	:  3-2 ,
		EN_PMI_CORE3	:  4-3 ,
		ReservedBits1	: 29-4 ,
		EN_FIXED_CTR0	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ 	: 32-31,
		ReservedBits2	: 64-32;
	} SKL;
	struct
	{
		unsigned long long
		EN_PMI_CORE0	:  1-0 ,
		EN_PMI_CORE1	:  2-1 ,
		EN_PMI_CORE2	:  3-2 ,
		EN_PMI_CORE3	:  4-3 ,
		EN_PMI_CORE4	:  5-4 ,
		ReservedBits1	: 29-5 ,
		EN_FIXED_CTR0	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ 	: 32-31,
		ReservedBits2	: 64-32;
	} ADL;
} UNCORE_GLOBAL_PERF_CONTROL;

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
	} NHM;
	struct
	{
		unsigned long long
		ReservedBits1	: 20-0,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_CTR0 	: 23-22,
		ReservedBits3	: 64-23;
	} SNB;
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
	} ADL;
	struct
	{
		unsigned long long
		ReservedBits1	: 20-0,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_CTR0 	: 23-22,
		ReservedBits3	: 64-23;
	} HSW_EP;
} UNCORE_FIXED_PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		PMI_Core_Select : 15-0,
		ReservedBits1	: 29-15,
		Unfreeze_All	: 30-29,
		WakeOnPMI	: 31-30,
		Freeze_All	: 32-31,
		ReservedBits2	: 64-32;
	};
} UNCORE_PMON_GLOBAL_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		TDP_Limit	: 15-0,
		TDP_Override	: 16-15,
		TDC_Limit	: 31-16,
		TDC_Override	: 32-31,
		ReservedBits	: 64-32;
	};
} NEHALEM_POWER_LIMIT;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
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
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Domain_Limit1	: 15-0,
		Enable_Limit1	: 16-15,
		Clamping1	: 17-16,
		TimeWindow1	: 24-17,
		ReservedBits1	: 31-24,
		PPn_LOCK	: 32-31,
		Domain_Limit2	: 47-32,
		Enable_Limit2	: 48-47,
		Clamping2	: 49-48,
		TimeWindow2	: 56-49,
		ReservedBits2	: 63-56,
		PKG_LOCK	: 64-63;
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
*/
#define PKG_POWER_LIMIT_LOCK_MASK	0x8000000000000000
#define PPn_POWER_LIMIT_LOCK_MASK	0x0000000080000000
/*TODO(CleanUp)
typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		TimeLimit	: 10-0,
		TimeUnit	: 13-10,
		ReservedBits1	: 15-13,
		Valid		: 16-15,
		ReservedBits2	: 64-16;
	};
} PKGCST_IRTL;

typedef struct
{
	unsigned long long
		Type		:  8-0,
		ReservedBits1	: 10-8,
		FixeRange	: 11-10,
		Enable		: 12-11,
		ReservedBits2	: 64-12;
} MTRR_DEF_TYPE;


typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
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
	unsigned long long	value;
	struct {
		unsigned long long
		L1_Scrubbing_En :  1-0,
		ReservedBits	: 64-1;
	};
} CORE_UARCH_CTL;

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
		VerbatimCopy	: 10-9,
		ReservedBits2	: 11-10,
		TCO_TMR_HALT	: 12-11,
		ReservedBits3	: 16-12;
	};
} Intel_TCO1_CNT;

typedef union
{
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
{
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
{
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
{
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
{
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
{
	unsigned char		value;
	struct {
		unsigned char
		Zeroed		:  2-0,
		Boundary	:  7-2,
		DRAM_4GB	:  8-7;
	};
} P945_MC_DRAM_RANK_BOUND;

typedef union
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		tRD_RD_DR	:  4-0,
		tRD_RD_SR	:  8-4,
		tWR_RD_DR	: 12-8,
		tWR_RD_SR	: 17-12,
		tRCD_RD 	: 21-17,
		ReservedBits	: 24-21,
		tREF		: 32-24;
	};
} P965_MC_CYCTRK_RD;

typedef union
{
	unsigned short		value;
	struct {
		unsigned short
		RFSH_RFSH_SR	:  9-0,
		PALL_RFSH_SR	: 13-9,
		ReservedBits	: 16-13;
	};
} P965_MC_CYCTRK_REFR;

typedef union
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 17-0,
		tCL		: 20-17,
		MCH_ODT_Latency : 24-20,
		ReservedBits2	: 32-24;
	};
} P965_MC_ODTCTRL;

typedef union
{
	unsigned int		value;
	struct {
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
{
	unsigned short		value;
	struct {
		unsigned short
		INDIRQUIET	: 38-32,
		DIRQUIET	: 44-38,
		Init_RefrCnt	: 47-44,
		ReservedBits	: 48-47;
	};
} N400_MC_REFRCTRL_HI48;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		RD_IMC_ODT	:  4-0,
		WR_DRAM_ODT	:  8-4,
		RD_DRAM_ODT	: 12-8,
		ReservedBits	: 32-12;
	};
} N400_MC_ODTCTRL;

typedef union
{
	unsigned short		value;
	struct {
		unsigned short
		Boundary	: 10-0,
		ReservedBits	: 16-10;
	};
} P35_MC_DRAM_RANK_BOUND;

typedef union
{
	unsigned short		value;
	struct {
		unsigned short
		Rank0		:  8-0,
		Rank1		: 16-8;
	};
} P35_MC_DRAM_BANK_RANK;

typedef union
{
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
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		tRD_RD_DR	:  4-0,
		tRD_RD_SR	:  8-4,
		tWR_RD_DR	: 12-8,
		tWR_RD_SR	: 17-12,
		tRCD_RD 	: 21-17,
		ReservedBits	: 24-21,
		tREF		: 32-24;
	};
} P35_MC_CYCTRK_RD;

typedef union
{
	unsigned short		value;
	struct {
		unsigned short
		UnknownBits1	:  8-0,
		tCL		: 14-8,
		UnknownBits2	: 16-14;
	};
} P35_MC_UNKNOWN_R0;

typedef union
{
	unsigned short		value;
	struct {
		unsigned short
		tRAS		:  6-0,
		UnknownBits	: 16-6;
	};
} P35_MC_UNKNOWN_R1;

typedef union
{
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

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		MBZ		:  4-0,
		Bytes		:  8-4,
		Offset		: 16-8,
		Port		: 24-16,
		OpCode		: 32-24;
	};
} PCI_MCR;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		RKEN0		:  1-0,
		RKEN1		:  2-1,
		RKEN2		:  3-2,
		RKEN3		:  4-3,
		DIMMDWID0	:  6-4,
		DIMMDDEN0	:  8-6,
		ReservedBits1	:  9-8,
		DIMMDWID1	: 11-9,
		DIMMDDEN1	: 13-11,
		ReservedBits2	: 14-13,
		RSIEN		: 15-14,
		ReservedBits3	: 16-15,
		DIMMFLIP	: 17-16,
		RANKREMAP	: 18-17,
		CKECOPY 	: 19-18,
		ReservedBits4	: 20-19,
		DIMM0MIRR	: 21-20,
		DIMM1MIRR	: 22-21,
		DRAMTYPE	: 23-22,
		ENLPDDR3	: 24-23,
		ReservedBits5	: 32-24;
	};
	struct {
		unsigned int
		RKEN0		:  1-0,
		RKEN1		:  2-1,
		Rsvd_0		:  4-2,
		DIMMDWID0	:  6-4,
		DIMMDDEN0	:  9-6,
		Rsvd_1		: 12-9,
		SRPP		: 13-12,
		BLMODE		: 14-13,
		RSIEN		: 15-14,
		BAHEN		: 16-15,
		SRRVD		: 20-16,
		DIMM0MIRR	: 21-20,
		ECCEN		: 22-21,
		DRAMTYPE	: 24-22,
		Rsvd_4		: 32-24;
	} AMT;
} SOC_MC_DRP;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DFREQ		:  2-0,
		ReservedBits1	:  4-2,
		tRP		:  8-4,
		tRCD		: 12-8,
		tCL		: 15-12,
		ReservedBits2	: 16-15,
		tXS		: 17-16,
		ReservedBits3	: 18-17,
		tXSDLL		: 19-18,
		ReservedBits4	: 20-19,
		tZQCS		: 21-20,
		ReservedBits5	: 22-21,
		tZQoper 	: 23-22,
		ReservedBits6	: 24-23,
		PMEDLY		: 26-24,
		ReservedBits7	: 28-26,
		CKEDLY		: 32-28;
	};
	struct {
		unsigned int
		Rsvd_5		:  3-0,
		tRP		:  8-3,
		tRCD		: 13-8,
		tCL		: 17-13,
		tXS		: 19-17,
		Rsvd_9		: 20-19,
		tZQCS		: 21-20,
		Rsvd_11 	: 22-21,
		tZQoper 	: 24-22,
		PMEDLY		: 26-24,
		CKEDLY		: 32-26;
	} AMT;
} SOC_MC_DTR0;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWCL		:  3-0,
		ReservedBits1	:  4-3,
		tCMD		:  6-4,
		ReservedBits2	:  8-6,
		tWTP		: 12-8,
		tCCD		: 14-12,
		ReservedBits3	: 16-14,
		tFAW		: 20-16,
		tRAS		: 24-20,
		tRRD		: 26-24,
		ReservedBits4	: 28-26,
		tRTP		: 31-28,
		ReservedBits5	: 32-24;
	};
	struct {
		unsigned int
		tWCL		:  3-0,
		Rsvd_14 	:  4-3,
		tCMD		:  6-4,
		Rsvd_15 	:  8-6,
		tWTP		: 12-8,
		tCCD		: 14-12,
		Rsvd_16 	: 16-14,
		tFAW		: 21-16,
		tRAS		: 26-21,
		tRRD		: 29-26,
		tRTP		: 32-29;
	} AMT;
} SOC_MC_DTR1;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRRDR		:  3-0,
		ReservedBits1	:  4-3,
		tRRDD		:  7-4,
		ReservedBits2	:  8-7,
		tWWDR		: 11-8,
		ReservedBits3	: 12-11,
		tWWDD		: 15-12,
		ReservedBits4	: 16-15,
		tRWDR		: 20-16,
		ReservedBits5	: 21-20,
		tRWDD		: 25-21,
		ReservedBits6	: 32-25;
	};
	struct {
		unsigned int
		ODTEN		:  1-0,
		tRRDR		:  4-1,
		Rsvd0_DTR2	:  8-4,
		tWWDR		: 11-8,
		Rsvd2_DTR2	: 15-11,
		tRWDR		: 20-15,
		DFREQ		: 23-20,
		nREFI		: 32-23;
	} AMT;
} SOC_MC_DTR2;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRDR		:  3-0,
		ReservedBits1	:  4-3,
		tWRDD		:  7-4,
		ReservedBits2	:  8-7,
		tRWSR		: 12-8,
		ReservedBits3	: 13-12,
		tWRSR		: 17-13,
		ReservedBits4	: 22-17,
		tXP		: 24-22,
		PWDDLY		: 28-24,
		ENDRATE 	: 29-28,
		DERATEOVR	: 30-29,
		DERATESTAT	: 31-30,
		ReservedBits5	: 32-31;
	};
	struct {
		unsigned int
		tWRDR		:  4-0,
		Rsvd0_DTR3	:  7-4,
		tRWSR		: 12-7,
		Rsvd2_DTR3	: 13-12,
		tWRSR		: 17-13,
		Rsvd3_DTR3	: 20-17,
		tXP		: 24-20,
		PWDDLY		: 28-24,
		Rsvd4_DTR3	: 32-28;
	} AMT;
} SOC_MC_DTR3;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		WRODTSTRT	:  2-0,
		Rsvd_32 	:  4-2,
		WRODTSTOP	:  7-4,
		Rsvd_7		:  8-7,
		RDODTSTRT	: 11-8,
		Rsvd_24 	: 12-11,
		RDODTSTOP	: 15-12,
		Rsvd_15 	: 16-15,
		TRGSTRDIS	: 17-16,
		RDODTDIS	: 18-17,
		WRBODTDIS	: 19-18,
		Rsvd_31 	: 32-19;
	};
	struct {
		unsigned int
		WRODTSTRT	:  2-0,
		Rsvd_22 	:  4-2,
		WRODTSTOP	:  7-4,
		Rsvd_23 	:  8-7,
		RDODTSTRT	: 11-8,
		Rsvd_24 	: 12-11,
		RDODTSTOP	: 15-12,
		Rsvd_25 	: 16-15,
		TRGSTRDIS	: 17-16,
		RDODTDIS	: 18-17,
		WRBODTDIS	: 19-18,
		Rsvd_27 	: 23-19,
		nRFCab		: 32-23;
	} AMT;
} SOC_MC_DTR4;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		SREDLY		:  8-0,
		PMOP		: 13-8,
		Rsvd_15 	: 16-13,
		PCLSTO		: 19-16,
		Rsvd_19 	: 20-19,
		PCLSWKOK	: 21-20,
		PREAPWDEN	: 22-21,
		Rsvd_22 	: 23-22,
		DYNSREN 	: 24-23,
		CLKGTDIS	: 25-24,
		DISPWRDN	: 26-25,
		BLMODE		: 27-26,
		Rsvd_27 	: 28-27,
		REUTCLKGTDIS	: 29-28,
		ENPHYCLKGATE	: 30-29,
		ENCKTRI		: 31-30,
		ENCORECLKGATE	: 32-31;
	};
	struct {
		unsigned int
		SREDLY		:  8-0,
		DYNSREN 	:  9-8,
		Rsvd_28 	: 11-9,
		DYNPMOP 	: 16-11,
		SUSPMOP 	: 21-16,
		SELFREQB	: 22-21,
		WFZQEN		: 23-22,
		CSTRIST 	: 24-23,
		PASR		: 32-24;
	} AMT;
} SOC_MC_DPMC0;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		CSTRIST 	:  1-0,
		Rsvd_3		:  4-1,
		CMDTRIST	:  6-4,
		Rsvd_31 	: 32-6;
	};
	struct {
		unsigned int
		DISPWRDN	:  1-0,
		DPMC1_Rsvd	:  2-1,
		PCLSWKOK	:  3-2,
		PREAPWDEN	:  4-3,
		PCLSTO		:  7-4,
		CKTRIST 	:  8-7,
		Rsvd_31 	: 24-8,
		CMDTRIST	: 26-24,
		Rsvd_dzhu	: 27-26,
		CLKGTDIS	: 28-27,
		REUTCLKGTDIS	: 29-28,
		tWTW		: 32-29;
	} AMT;
} SOC_MC_DPMC1;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		REFWMLO 	:  4-0,
		REFWMHI 	:  8-4,
		REFWMPNC	: 12-8,
		tREFI		: 15-12,
		ReservedBits1	: 16-15,
		REFCNTMAX	: 18-16,
		ReservedBits2	: 20-18,
		REFSKWDIS	: 21-20,
		REFDBTCLR	: 22-21,
		ReservedBits3	: 24-22,
		CUREFRATE	: 27-24,
		ReservedBits4	: 32-27;
	};
	struct {
		unsigned int
		REFWMLO 	:  4-0,
		REFWMHI 	:  8-4,
		REFWMPNC	: 12-8,
		tREFI		: 15-12,
		Rsvd_34 	: 16-15,
		REFCNTMAX	: 18-16,
		Rsvd_35 	: 20-18,
		REFSKWDIS	: 21-20,
		REFDBTCLR	: 22-21,
		Rsvd_DRFC_0	: 32-22;
	} AMT;
} SOC_MC_DRFC;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		CKEVAL		:  4-0,
		CKEMODE		:  5-4,
		Rsvd_5		:  8-5,
		ODTVAL		: 12-8,
		ODTMODE 	: 13-12,
		Rsvd_13 	: 16-13,
		COLDWAKE	: 17-16,
		Rsvd_17 	: 32-17;
	};
	struct {
		unsigned int
		CKEVAL		:  2-0,
		Rsvd_2		:  4-2,
		CKEMODE 	:  5-4,
		Rsvd_5		:  8-5,
		ODTVAL		: 10-8,
		Rsvd_10 	: 12-10,
		ODTMODE 	: 13-12,
		Rsvd_13 	: 16-13,
		COLDWAKE	: 17-16,
		DBPTRCLR	: 18-17,
		Rsvd_18 	: 32-18;
	} AMT;
} SOC_MC_DRMC;

typedef union
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
		EFF_DUAL_CH_EN	: 29-28,
		EFF_ECC_EN	: 30-29,
		DUAL_CH_DIS	: 31-30,
		ECC_EN		: 32-31;
	};
} SOC_MC_BIOS_CFG;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		RESET_ON_TIME	:  5-0,
		BLOCK_CKE_DELAY : 10-5,
		PHY_FSM_DELAY	: 15-10,
		REGISTERED_DIMM : 16-15,
		WRLEVEL_DELAY	: 17-16,
		WRDQDQS_DELAY	: 22-17,
		QUAD_RANK	: 23-22,
		SINGLE_QUAD_RANK: 24-23,
		THREE_DIMMS	: 25-24,
		DIS_AI		: 26-25,
		DIS_3T		: 27-26,
		ReservedBits	: 32-27;
	};
} NHM_IMC_DIMM_INIT_PARAMS;

typedef union
{
	unsigned int		value;
	struct {
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

typedef union
{
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
		RC0		: 20-16,
		RC2		: 24-20,
		MR3		: 32-24;
	};
} NHM_IMC_MRS_VALUE_2_3;

typedef union
{
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

typedef union
{
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

typedef union
{
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

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRFC		:  9-0,
		tREFI_8 	: 19-9,
		tTHROT_OPPREF	: 30-19,
		ReservedBits	: 32-30;
	};
} NHM_IMC_REFRESH_TIMING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tCKE		:  3-0,
		tXS		: 11-3,
		tXSDLL		: 21-11,
		tXP		: 24-21,
		tRANKIDLE	: 32-24;
	};
} NHM_IMC_CKE_TIMING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		CLOSED_PAGE	:  1-0,
		ECC_ENABLED	:  2-1,
		AUTOPRECHARGE	:  3-2,
		CHANNELRESET0	:  4-3,
		CHANNELRESET1	:  5-4,
		CHANNELRESET2	:  6-5,
		DIVBY3EN	:  7-6,
		INIT_DONE	:  8-7,
		CHANNEL0_ACTIVE :  9-8,
		CHANNEL1_ACTIVE : 10-9,
		CHANNEL2_ACTIVE : 11-10,
		ReservedBits	: 32-11;
	};
} NHM_IMC_CONTROL;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		CHANNEL0_DISABLE:  1-0,
		CHANNEL1_DISABLE:  2-1,
		CHANNEL2_DISABLE:  3-2,
		ReservedBits1	:  4-3,
		ECC_ENABLED	:  5-4,
		ReservedBits2	: 32-5;
	};
} NHM_IMC_STATUS;

typedef union
{
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

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		QCLK_RATIO	:  5-0,
		ReservedBits1	: 24-5,
		MAX_RATIO	: 29-24,
		ReservedBits2	: 32-29;
	};
} NHM_IMC_CLK_RATIO_STATUS;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		PRIORITY_CNT	:  3-0,
		ENABLE_2N_3N	:  5-3,
		DIS_ISOC_RBC	:  6-5,
		PRE_CAS_THRSHLD : 11-6,
		FLOAT_EN	: 12-11,
		CS_FOR_CKE_TRANS: 13-12,
		DDR_CLK_TRISTATE: 14-13,
		ReservedBits	: 32-14;
	};
} NHM_IMC_SCHEDULER_PARAMS;

typedef union
{
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

typedef union
{
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
	struct {
		unsigned int
		QPIFREQSEL	:  2-0,
		ReservedBits	: 31-2,
		VT_d		: 32-31;
	} X58;
	struct {
		unsigned int
		QPIFREQSEL	:  3-0,
		ReservedBits1	:  4-3,
		Slow_Mode	:  5-4,
		ReservedBits2	: 32-5;
	} EP;
} QPI_FREQUENCY;


typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		IIO		:  8-0,
		UNC		: 16-8,
		Valid		: 17-16,
		Segment		: 25-17,
		ReservedBits	: 32-25;
	} CFG;
	struct {
		unsigned int
		IIO		:  8-0,
		UNC		: 16-8,
		ReservedBits	: 31-16,
		Valid		: 32-31;
	} UBOX;
} CPUBUSNO;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		MEMCLK		:  4-0,
		ReservedBits1	:  8-4,
		PLL_REF100	:  9-8,
		ReservedBits2 	: 32-9;
	};
	struct {
		unsigned int
		MC_PLL_RATIO	:  8-0,
		MC_PLL_REF	: 12-8,
		GEAR		: 14-12,
		ReservedBits 	: 17-14,
		REQ_VDDQ_TX_VOLT: 27-17,
		REQ_VDDQ_TX_ICC : 31-27,
		RUN_BUSY	: 32-31;
	};
} BIOS_MEMCLOCK;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRCD		:  4-0,
		tRP		:  8-4,
		tCL		: 12-8,
		tCWL		: 16-12,
		tRAS		: 24-16,
		ReservedBits	: 32-24;
	};
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
{
	unsigned int		value;
	struct {
		unsigned int
		tRRD		:  4-0,
		tRTPr		:  8-4,
		tCKE		: 12-8,
		tWTPr		: 16-12,
		tFAW		: 24-16,
		tWR		: 29-24,
		CMD_3ST 	: 30-29,
		CMD_Stretch	: 32-30;
	};
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
		CMD_Stretch	: 32-30;
	} EP;
} SNB_IMC_TC_RAP;

typedef union
{
	unsigned int		value;
	struct {
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
	struct {
		unsigned int
		tRRDR		:  3-0,
		tRRDD		:  6-3,
		tWWDR		:  9-6,
		tWWDD		: 12-9,
		tRWDR		: 15-12,
		tRWDD		: 18-15,
		tWRDR		: 21-18,
		tWRDD		: 24-21,
		tRWSR		: 27-24,
		tCCD		: 30-27,
		tWRDR_UPPER	: 32-30;
	} EP;
} SNB_IMC_TC_RWP;


typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  3-0,
		tRRDD		:  6-3,
		ReservedBits2	: 13-6,
		tWRSR		: 17-13,
		ReservedBits3	: 32-17;
	};
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
{
	unsigned int		value;
	struct {
		unsigned int
		PDWN_Idle_Ctr	:  8-0,
		PDWN_Mode	: 12-8,
		GLPDN		: 13-12,
		ReservedBits	: 32-13;
	};
} SNB_IMC_PDWN;


typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		OREF_RI		:  8-0,
		REF_HP_WM	: 12-8,
		REF_PANIC_WM	: 16-12,
		DOUBLE_REF_CTRL : 18-16,
		ReservedBits	: 32-18;
	};
	struct {
		unsigned int
		OREFNI		:  8-0,
		REF_HI_WM	: 12-8,
		REF_PANIC_WM	: 16-12,
		ReservedBits	: 32-16;
	} EP;
} SNB_IMC_TC_RFP;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 25-16,
		tREFIx9 	: 32-25;
	};
	struct {
		unsigned int
		tREFI		: 15-0,
		tRFC		: 25-15,
		tREFIx9 	: 32-25;
	} EP;
} SNB_IMC_TC_RFTP;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tXSDLL		: 12-0,
		tXS		: 16-12,
		tZQOPER 	: 26-16,
		ReservedBits	: 28-26,
		tMOD		: 32-28;
	};
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
{
	unsigned int		value;
	struct {
		unsigned int
		CH_A		:  2-0,
		CH_B		:  4-2,
		CH_C		:  6-4,
		ReservedBits1	: 10-6,
		LPDDR		: 11-10,
		ReservedBits2	: 32-11;
	};
} SNB_IMC_MAD_MAPPING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_A_Size	:  8-0,
		Dimm_B_Size	: 16-8,
		DAS		: 17-16,
		DANOR		: 18-17,
		DBNOR		: 19-18,
		DAW		: 20-19,
		DBW		: 21-20,
		RI		: 22-21,
		ENH_IM		: 23-22,
		ReservedBits1	: 24-23,
		ECC		: 26-24,
		ReservedBits2	: 32-26;
	};
} SNB_IMC_MAD_CHANNEL;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DMFC		:  3-0,
		ReservedBits1	: 14-3,
		DDPCD		: 15-14,
		ReservedBits2	: 23-15,
		VT_d		: 24-23,
		ReservedBits3	: 32-24;
	};
	struct {
		unsigned int
		DDR3L_EN	:  1-0,
		DDR_WRTVREF	:  2-1,
		OC_ENABLED_DSKU :  3-2,
		ReservedBits1	: 14-3,
		DDPCD		: 15-14,
		ReservedBits2	: 23-15,
		VT_d		: 24-23,
		ReservedBits3	: 25-24,
		ECCDIS		: 26-25,
		ReservedBits4	: 32-26;
	} IVB;
} SNB_CAPID_A;

typedef union
{
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
{
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
{
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
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
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
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  4-0,
		DMFC		:  7-4,
		ReservedBits2	: 17-7,
		ADDGFXCAP	: 18-17,
		ADDGFXEN	: 19-18,
		ReservedBits3	: 20-19,
		PEGG3_DIS	: 21-20,
		PLL_REF100_CFG	: 24-21,
		ReservedBits4	: 25-24,
		CACHESZ 	: 28-25,
		SMTCAP		: 29-28,
		ReservedBits5	: 32-29;
	};
} IVB_CAPID_B;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRCD		:  5-0,
		tRP		: 10-5,
		tRAS		: 16-10,
		tRRD		: 20-16,
		UnknownBits	: 32-20;
	};
} HSW_DDR_TIMING_4C00;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 12-0,
		tsrRdTRd	: 15-12,
		tdrRdTRd	: 19-15,
		tddRdTRd	: 23-19,
		ReservedBits2	: 30-23,
		CMD_Stretch	: 32-30;
	};
} HSW_DDR_TIMING;

typedef union
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		tCL		:  5-0,
		tCWL		: 10-5,
		ReservedBits	: 32-10;
	};
} HSW_DDR_RANK_TIMING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		PDWN_Idle_Ctr 	: 12-0,
		PDWN_Mode	: 16-12,
		ReservedBits 	: 32-16;
	};
} HSW_PM_POWER_DOWN;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		OREF_RI 	:  8-0,
		Ref_HP_WM	: 12-8,
		Ref_Panic_WM	: 16-12,
		ReservedBits 	: 32-16;
	};
} HSW_TC_REFRESH_PARAM;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 25-16,
		tREFIx9 	: 32-25;
	};
} HSW_TC_REFRESH_TIMING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		CLOSE_PG	:  1-0,
		LS_EN		:  2-1,
		ECC_EN		:  3-2,
		DIR_EN		:  4-3,
		ReservedBits1	:  8-4,
		Normal_Mode	:  9-8,
		ReservedBits2	: 12-9,
		IMC_MODE	: 14-12,
		DDR4_Mode	: 15-14,
		ReservedBits3	: 16-15,
		Pass76		: 18-16,
		CHN_DISABLE	: 22-18,
		ReservedBits4	: 32-22;
	};
} HSW_EP_MC_TECH;

#define HSW_EP_TADWAYNESS	SNB_EP_TADWAYNESS

typedef union
{
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
		DDR4_Mode	: 21-20,
		HDRL		: 22-21,
		HDRL_Parity	: 23-22,
		ReservedBits3	: 32-23;
	};
} HSW_EP_DIMM_MTR;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  6-0,
		tRPab_ext	:  8-6,
		tRAS		: 15-8,
		ReservedBits1	: 16-15,
		tRDPRE		: 20-16,
		ReservedBits2	: 24-20,
		tWRPRE		: 31-24,
		ReservedBits3	: 32-31;
	};
} SKL_IMC_CR_TC_PRE;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRP		:  6-0,
		tRPab_ext	:  9-6,
		tRAS		: 16-9,
		tRDPRE		: 21-16,
		tPPD		: 24-21,
		tWRPRE		: 32-24;
	};
} RKL_IMC_CR_TC_PRE;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tRP		:  7-0,
		tRPab_ext	: 11-7,
		tRDPRE		: 17-11,
		tPPD		: 21-17,
		tWRPRE		: 30-21,
		ReservedBits1	: 33-30,
		tRAS		: 41-33,
		tRCD		: 48-41,
		ReservedBits2	: 64-48;
	};
} TGL_IMC_CR_TC_PRE;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tRP		:  8-0,
		tRPab_ext	: 13-8,
		tRDPRE		: 20-13,
		tPPD		: 24-20,
		tRCDW		: 32-24,
		tWRPRE		: 42-32,
		tRAS		: 51-42,
		tRCD		: 59-51,
		DERATING_EXT	: 63-59,
		ReservedBits	: 64-63;
	};
} ADL_IMC_CR_TC_PRE;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  7-0,
		ReservedBits1	:  8-7,
		tRRD_SG 	: 13-8,
		tRRD_DG 	: 18-13,
		DERATING_EXT	: 21-18,
		tRCD_WR 	: 27-21,
		ReservedBits2	: 32-27;
	};
} SKL_IMC_CR_TC_ACT;

#define RKL_IMC_CR_TC_ACT	SKL_IMC_CR_TC_ACT

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  8-0,
		tRRD_SG 	: 14-8,
		tRRD_DG 	: 20-14,
		DERATING_EXT	: 23-20,
		ReservedBits	: 32-23;
	};
} TGL_IMC_CR_TC_ACT;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  9-0,
		tRRD_SG 	: 15-9,
		tRRD_DG 	: 22-15,
		ReservedBits	: 24-22,
		tREFSBRD	: 32-24;
	};
} ADL_IMC_CR_TC_ACT;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRDRD_SG	:  6-0,
		ReservedBits1	:  8-6,
		tRDRD_DG	: 14-8,
		ReservedBits2	: 16-14,
		tRDRD_DR	: 22-16,
		ReservedBits3	: 24-22,
		tRDRD_DD	: 30-24,
		ReservedBits4	: 32-30;
	};
} SKL_IMC_CR_TC_RDRD;

#define RKL_IMC_CR_TC_RDRD	SKL_IMC_CR_TC_RDRD

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRDRD_SG	:  6-0,
		ReservedBits1	:  8-6,
		tRDRD_DG	: 14-8,
		ReservedBits2	: 16-14,
		tRDRD_DR	: 23-16,
		ReservedBits3	: 24-23,
		tRDRD_DD	: 31-24,
		ReservedBits4	: 32-31;
	};
} TGL_IMC_CR_TC_RDRD;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRDRD_SG	:  7-0,
		TWODCLK_B2B_LPDDR: 8-7,
		tRDRD_DG	: 15-8,
		ReservedBits	: 16-15,
		tRDRD_DR	: 24-16,
		tRDRD_DD	: 32-24;
	};
} ADL_IMC_CR_TC_RDRD;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRDWR_SG	:  6-0,
		ReservedBits1	:  8-6,
		tRDWR_DG	: 14-8,
		ReservedBits2	: 16-14,
		tRDWR_DR	: 22-16,
		ReservedBits3	: 24-22,
		tRDWR_DD	: 30-24,
		ReservedBits4	: 32-30;
	};
} SKL_IMC_CR_TC_RDWR;

#define RKL_IMC_CR_TC_RDWR	SKL_IMC_CR_TC_RDWR

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRDWR_SG	:  7-0,
		ReservedBits1	:  8-7,
		tRDWR_DG	: 15-8,
		ReservedBits2	: 16-15,
		tRDWR_DR	: 23-16,
		ReservedBits3	: 24-23,
		tRDWR_DD	: 31-24,
		ReservedBits4	: 32-31;
	};
} TGL_IMC_CR_TC_RDWR;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tRDWR_SG	:  8-0,
		tRDWR_DG	: 16-8,
		tRDWR_DR	: 24-16,
		tRDWR_DD	: 32-24;
	};
} ADL_IMC_CR_TC_RDWR;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRRD_SG	:  8-0,
		tWRRD_DG	: 15-8,
		ReservedBits1	: 16-15,
		tWRRD_DR	: 22-16,
		ReservedBits2	: 24-22,
		tWRRD_DD	: 30-24,
		ReservedBits3	: 32-30;
	};
} SKL_IMC_CR_TC_WRRD;

#define RKL_IMC_CR_TC_WRRD	SKL_IMC_CR_TC_WRRD

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRRD_SG	:  8-0,
		tWRRD_DG	: 16-8, 
		tWRRD_DR	: 22-16,
		ReservedBits1	: 24-22,
		tWRRD_DD	: 30-24,
		ReservedBits2	: 32-30;
	};
} TGL_IMC_CR_TC_WRRD;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRRD_SG	:  9-0,
		tWRRD_DG	: 18-9,
		tWRRD_DR	: 25-18,
		tWRRD_DD	: 32-25;
	};
} ADL_IMC_CR_TC_WRRD;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRWR_SG	:  6-0,
		ReservedBits1	:  8-6,
		tWRWR_DG	: 14-8,
		ReservedBits2	: 16-14,
		tWRWR_DR	: 22-16,
		ReservedBits3	: 24-22,
		tWRWR_DD	: 30-24,
		ReservedBits4	: 32-30;
	};
} SKL_IMC_CR_TC_WRWR;

#define RKL_IMC_CR_TC_WRWR	SKL_IMC_CR_TC_WRWR

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRWR_SG	:  6-0,
		ReservedBits1	:  8-6,
		tWRWR_DG	: 14-8,
		ReservedBits2	: 16-14,
		tWRWR_DR	: 22-16,
		ReservedBits3	: 24-22,
		tWRWR_DD	: 31-24,
		ReservedBits4	: 32-31;
	};
} TGL_IMC_CR_TC_WRWR;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tWRWR_SG	:  7-0,
		ReservedBits1	:  8-7,
		tWRWR_DG	: 15-8,
		ReservedBits2	: 16-15,
		tWRWR_DR	: 23-16,
		ReservedBits3	: 24-23,
		tWRWR_DD	: 32-24;
	};
} ADL_IMC_CR_TC_WRWR;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DRAM_Tech	:  2-0,
		CMD_Stretch	:  4-2,
		N_to_1_ratio	:  7-4,
		ReservedBits	:  8-7,
		Addr_Mirroring	: 10-8,
		x8_device_Dimm0 : 11-10,
		x8_device_Dimm1 : 12-11,
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
} SKL_IMC_CR_SC_CFG;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	:  3-0,
		CMD_Stretch	:  5-3,
		N_to_1_ratio	:  8-5,
		Addr_Mirroring	: 12-8,
		tCPDED		: 15-12,
		ReservedBits2	: 28-15,
		x8_device_Dimm0 : 29-28,
		x8_device_Dimm1 : 30-29,
		NO_GEAR2_PRM_DIV: 31-30,
		GEAR2		: 32-31,
		DDR4_1_DPC	: 34-32,
		ReservedBits3	: 64-34;
	};
} RKL_IMC_SC_GS_CFG;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	:  3-0,
		CMD_Stretch	:  5-3,
		N_to_1_ratio	:  8-5,
		Addr_Mirroring	: 12-8,
		tCPDED		: 15-12,
		ReservedBits2	: 28-15,
		x8_device_Dimm0 : 29-28,
		x8_device_Dimm1 : 30-29,
		NO_GEAR2_PRM_DIV: 31-30,
		GEAR2		: 32-31,
		DDR4_1_DPC	: 34-32,
		ReservedBits3	: 47-34,
		WRITE0_EN	: 48-47,
		ReservedBits4	: 51-48,
		WCK_DIFF_LOW_IDLE:52-51,
		ReservedBits5	: 64-52;
	};
} TGL_IMC_SC_GS_CFG;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	:  3-0,
		CMD_Stretch	:  5-3,
		N_to_1_ratio	:  8-5,
		Addr_Mirroring	: 12-8,
		ReservedBits2	: 15-12,
		GEAR4		: 16-15,
		NO_GEAR4_PRM_DIV: 17-16,
		ReservedBits3	: 28-17,
		x8_device_Dimm0 : 29-28,
		x8_device_Dimm1 : 30-29,
		NO_GEAR2_PRM_DIV: 31-30,
		GEAR2		: 32-31,
		DDR_1_DPC	: 34-32,
		ReservedBits4	: 49-34,
		WRITE0_EN	: 50-49,
		ReservedBits5	: 54-50,
		WCK_DIFF_LOW_IDLE:55-54,
		ReservedBits6	: 56-55,
		tCPDED		: 61-56,
		ReservedBits7	: 64-61;
	};
} ADL_IMC_SC_GS_CFG;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tCKE		:  5-0,
		ReservedBits1	:  8-5,
		tXP		: 13-8,
		ReservedBits2	: 16-13,
		tXPDLL		: 22-16,
		tPRPDEN 	: 24-22,
		tRDPDEN 	: 31-24,
		ReservedBits3	: 32-31,
		tWRPDEN 	: 40-32,
		ReservedBits4	: 64-40;
	};
} RKL_IMC_TC_PWDEN;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tCKE		:  6-0,
		ReservedBits1	:  8-6,
		tXP		: 14-8,
		ReservedBits2	: 16-14,
		tXPDLL		: 22-16,
		ReservedBits3	: 24-22,
		tRDPDEN 	: 31-24,
		ReservedBits4	: 32-31,
		tWRPDEN 	: 41-32,
		tCSH		: 46-41,
		tCSL		: 51-46,
		ReservedBits5	: 56-51,
		tPRPDEN 	: 61-56,
		ReservedBits6	: 64-61;
	};
} TGL_IMC_TC_PWDEN;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tCKE		:  7-0,
		tXP		: 14-7,
		tXPDLL		: 21-14,
		tRDPDEN 	: 29-21,
		ReservedBits1	: 32-29,
		tWRPDEN 	: 42-32,
		tCSH		: 48-42,
		tCSL		: 54-48,
		ReservedBits2	: 59-54,
		tPRPDEN 	: 64-59;
	};
} ADL_IMC_TC_PWDEN;

typedef union
{
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
} SKL_IMC_CR_TC_ODT;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	: 16-0,
		tCL		: 22-16,
		tCWL		: 28-22,
		ReservedBits2	: 64-28;
	};
} RKL_IMC_CR_TC_ODT;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	: 16-0,
		tCL		: 23-16,
		tCWL		: 30-23,
		ReservedBits2	: 64-30;
	};
} TGL_IMC_CR_TC_ODT;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		ReservedBits1	: 16-0,
		tCL		: 23-16,
		ReservedBits2	: 24-23,
		tCWL		: 32-24,
		ReservedBits3	: 64-32;
	};
} ADL_IMC_CR_TC_ODT;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 26-16,
		ReservedBits	: 32-26;
	};
} SKL_IMC_REFRESH_TC;

#define RKL_IMC_REFRESH_TC	SKL_IMC_REFRESH_TC

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 17-0,
		tRFC		: 29-17,
		ReservedBits	: 32-29;
	};
} TGL_IMC_REFRESH_TC;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 18-0,
		tRFC		: 31-18,
		ReservedBits	: 32-31;
	};
} ADL_IMC_REFRESH_TC;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tXSR		: 10-0,
		ReservedBits	: 32-10;
	};
} RKL_IMC_SREXITTP;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tXSR		: 12-0,
		ReservedBits	: 64-12;
	};
} TGL_IMC_SREXITTP;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		tXSR		: 13-0,
		ReservedBits1	: 52-13,
		tSR		: 58-52,
		ReservedBits2	: 64-58;
	};
} ADL_IMC_SREXITTP;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  2-0,
		ReservedBits1	:  4-2,
		CH_L_MAP	:  5-4,
		ReservedBits2	: 12-5,
		CH_S_SIZE	: 19-12,
		ReservedBits3	: 32-19;
	};
} SKL_IMC_MAD_MAPPING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  3-0,
		ECHM		:  4-3,
		CH_L_MAP	:  5-4,
		ReservedBits1	: 12-5,
		CH_S_SIZE	: 20-12,
		ReservedBits2	: 27-20,
		CH_WIDTH	: 29-27,
		ReservedBits3	: 32-29;
	};
} RKL_IMC_MAD_MAPPING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  3-0,
		ReservedBits1	:  4-3,
		CH_L_MAP	:  5-4,
		ReservedBits2	: 12-5,
		CH_S_SIZE	: 20-12,
		ReservedBits3	: 27-20,
		CH_WIDTH	: 29-27,
		ReservedBits4	: 31-29,
		HALF_CL_MODE	: 32-31;
	};
} TGL_IMC_MAD_MAPPING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DDR_TYPE	:  3-0,
		ReservedBits1	:  4-3,
		CH_L_MAP	:  5-4,
		ReservedBits2	: 12-5,
		CH_S_SIZE	: 20-12,
		ReservedBits3	: 27-20,
		CH_WIDTH	: 29-27,
		ReservedBits4	: 31-29,
		HALF_CL_MODE	: 32-31;
	};
} ADL_IMC_MAD_MAPPING;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Map	:  1-0,
		ReservedBits1	:  4-1,
		RI		:  5-4,
		ReservedBits2	:  8-5,
		EIM		:  9-8,
		ReservedBits3	: 12-9,
		ECC		: 14-12,
		ReservedBits4	: 24-14,
		HORI		: 25-24,
		ReservedBits5	: 28-25,
		HORI_ADDR	: 31-28,
		ReservedBits6	: 32-31;
	};
} SKL_IMC_MAD_CHANNEL;

#define RKL_IMC_MAD_CHANNEL	SKL_IMC_MAD_CHANNEL

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Map	:  1-0,
		ReservedBits1	:  8-1,
		EIM		:  9-8,
		ReservedBits2	: 12-9,
		ECC		: 14-12,
		ReservedBits3	: 32-14;
	};
} TGL_IMC_MAD_CHANNEL;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Map	:  1-0,
		ReservedBits1	:  8-1,
		EIM		:  9-8,
		ReservedBits2	: 12-9,
		ECC		: 14-12,
		CRC		: 15-14,
		ReservedBits3	: 32-15;
	};
} ADL_IMC_MAD_CHANNEL;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  6-0,
		ReservedBits1	:  8-6,
		DLW		: 10-8,
		DLNOR		: 11-10,
		DL8Gb		: 12-11,
		ReservedBits2	: 16-12,
		Dimm_S_Size	: 22-16,
		ReservedBits3	: 24-22,
		DSW		: 26-24,
		DSNOR		: 27-26,
		DS8Gb		: 28-27,
		ReservedBits4	: 32-28;
	};
} SKL_IMC_MAD_DIMM;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  7-0,
		DLW		:  9-7,
		DLNOR		: 11-9,
		ReservedBits1	: 16-11,
		Dimm_S_Size	: 23-16,
		ReservedBits2	: 24-23,
		DSW		: 26-24,
		DSNOR		: 28-26,
		ReservedBits3	: 29-28,
		DLS_BG0_BIT_11	: 30-29,
		ReservedBits4	: 32-30;
	};
} CML_U_IMC_MAD_DIMM;

#define RKL_IMC_MAD_DIMM	CML_U_IMC_MAD_DIMM

#define TGL_IMC_MAD_DIMM	RKL_IMC_MAD_DIMM

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  7-0,
		DLW		:  9-7,
		DLNOR		: 11-9,
		DDR5_DS_8GB	: 12-11,
		DDR5_DL_8GB	: 13-12,
		ReservedBits1	: 16-13,
		Dimm_S_Size	: 23-16,
		ReservedBits2	: 24-23,
		DSW		: 26-24,
		DSNOR		: 28-26,
		BG0_BIT_OPTIONS : 30-28,
		DECODER_EBH	: 32-30;
	};
} ADL_IMC_MAD_DIMM;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		QCLK		:  7-0,
		QCLK_REF	:  8-7,
		FCLK		: 16-8,
		ICLK		: 24-16,
		UCLK		: 32-24;
	};
} SKL_SA_PLL_RATIOS;

typedef union
{
	unsigned long long	value;
	struct {
		unsigned long long
		LAST_DE_WP_REQ_SERVED	:   2-0,
		QCLK_RATIO		:  10-2,
		QCLK_REF		: 11-10,
		OPI_LINK_SPEED		: 12-11,
		IPU_IS_DIVISOR		: 18-12,
		IPU_PS_RATIO		: 24-18,
		UCLK_RATIO		: 32-24,
		PSF0_RATIO		: 40-32,
		SA_VOLTAGE		: 56-40,
		ReservedBits		: 64-56;
	};
} RKL_SA_PERF_STATUS;

#define TGL_SA_PERF_STATUS	RKL_SA_PERF_STATUS

#define ADL_SA_PERF_STATUS	TGL_SA_PERF_STATUS

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 12-0,
		PDCD		: 13-12,
		X2APIC_EN	: 14-13,
		DDPCD		: 15-14,
		ReservedBits2	: 23-15,
		VT_d		: 24-23,
		ReservedBits3	: 25-24,
		ECCDIS		: 26-25,
		ReservedBits4	: 32-26;
	};
} SKL_CAPID_A;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		NVME_F7D	:  1-0,
		ReservedBits1	:  3-1,
		DDR_OVERCLOCK	:  4-3,
		CRID		:  8-4,
		ReservedBits2	: 10-8,
		DID0OE		: 11-10,
		IGD		: 12-11,
		PDCD		: 13-12,
		X2APIC_EN	: 14-13,
		DDPCD		: 15-14,
		CDD		: 16-15,
		ReservedBits3	: 17-16,
		D1NM		: 18-17,
		PEG60D		: 19-18,
		DDRSZ		: 21-19,
		PEGG2DIS	: 22-21,
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
} RKL_CAPID_A;

#define TGL_CAPID_A	RKL_CAPID_A

typedef union
{
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

#define GKL_CAPID_A	SKL_CAPID_A

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  2-0,
		LPDDR3_EN	:  3-2,
		ReservedBits2	:  4-3,
		DMFC_DDR3	:  7-4,
		ReservedBits3	:  8-7,
		GMM_DIS 	:  9-8,
		ReservedBits4	: 15-9,
		DMIG3DIS	: 16-15,
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
		IMGU_DIS	: 32-31;
	};
} SKL_CAPID_B;

typedef union
{
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
		LVL_MEMORY	: 11-10,
		HDCPD		: 12-11,
		LTECH		: 15-12,
		DMIG3DIS	: 16-15,
		PEGX16D 	: 17-16,
		ADDGFXCAP	: 18-17,
		ADDGFXEN	: 19-18,
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
} RKL_CAPID_B;

#define TGL_CAPID_B	RKL_CAPID_B

typedef union
{
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
		DEV10_DIS	: 11-10,
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

#define GKL_CAPID_B	SKL_CAPID_B

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 14-0,
		DMFC_LPDDR3	: 17-14,
		DMFC_DDR4	: 20-17,
		ReservedBits2	: 32-20;
	};
} SKL_CAPID_C;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DATA_RATE_GEAR1 :  5-0,
		DISPLAY_PIPE3	:  6-5,
		IDD		:  7-6,
		BCLK_OC_FREQ	:  9-7,
		SGX_DIS 	: 10-9,
		ReservedBits1	: 14-10,
		QCLK_GV_DIS	: 15-14,
		IB_ECC		: 16-15,
		LPDDR4_EN	: 17-16,
		DATA_RATE_LPDDR4: 22-17,
		DDR4_EN 	: 23-22,
		DATA_RATE_DDR4	: 28-23,
		PEGG4_DIS	: 29-28,
		ReservedBits2	: 32-29;
	};
} RKL_CAPID_C;

#define TGL_CAPID_C	RKL_CAPID_C

typedef union
{
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
{
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
} TGL_CAPID_E;

typedef union
{
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
*/