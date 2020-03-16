/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#ifndef MSR_SMI_COUNT
	#define MSR_SMI_COUNT			0x00000034
#endif

#ifndef MSR_IA32_FEAT_CTL
	#define MSR_IA32_FEAT_CTL		MSR_IA32_FEATURE_CONTROL
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

#ifndef MSR_IA32_CORE_CAPABILITIES
	#define MSR_IA32_CORE_CAPABILITIES	0x000000cf
#endif

#ifndef MSR_IA32_TSX_CTRL
	#define MSR_IA32_TSX_CTRL		0x00000122
#endif

#ifndef MSR_PLATFORM_INFO
	#define MSR_PLATFORM_INFO		0x000000ce
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

#ifndef MSR_IA32_POWER_CTL
	#define MSR_IA32_POWER_CTL		0x000001fc
#endif

#ifndef MSR_PKG_CST_CONFIG_CONTROL
	#define MSR_PKG_CST_CONFIG_CONTROL	MSR_NHM_SNB_PKG_CST_CFG_CTL
#endif

#ifndef MSR_PMG_IO_CAPTURE_BASE
	#define MSR_PMG_IO_CAPTURE_BASE 	0x000000e4
#endif

#define MSR_NHM_UNCORE_PERF_GLOBAL_CTRL 	0x00000391
#define MSR_SNB_UNCORE_PERF_GLOBAL_CTRL 	0x00000391
#define MSR_SKL_UNCORE_PERF_GLOBAL_CTRL 	0x00000e01

#define MSR_SNB_EP_PMON_GLOBAL_CTRL		0x00000c00
#define MSR_HSW_EP_PMON_GLOBAL_CTRL		0x00000700

#define MSR_NHM_UNCORE_PERF_GLOBAL_STATUS	0x00000392
#define MSR_SNB_UNCORE_PERF_GLOBAL_STATUS	0x00000392
#define MSR_SKL_UNCORE_PERF_GLOBAL_STATUS	0x00000e02

#define MSR_UNCORE_PERF_GLOBAL_OVF_CTRL 	0x00000393

#define MSR_NHM_UNCORE_PERF_FIXED_CTR0		0x00000394
#define MSR_SNB_UNCORE_PERF_FIXED_CTR0		0x00000395
#define MSR_SKL_UNCORE_PERF_FIXED_CTR0		0x00000395
#define MSR_SNB_EP_UNCORE_PERF_FIXED_CTR0	0x00000c09
#define MSR_HSW_EP_UNCORE_PERF_FIXED_CTR0	0x00000704

#define MSR_NHM_UNCORE_PERF_FIXED_CTR_CTRL	0x00000395
#define MSR_SNB_UNCORE_PERF_FIXED_CTR_CTRL	0x00000394
#define MSR_SKL_UNCORE_PERF_FIXED_CTR_CTRL	0x00000394
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

#ifndef MSR_PKG_ENERGY_STATUS
	#define MSR_PKG_ENERGY_STATUS		0x00000611
#endif

#ifndef MSR_PKG_PERF_STATUS
	#define MSR_PKG_PERF_STATUS		0x00000613
#endif

#ifndef MSR_PKG_POWER_INFO
	#define MSR_PKG_POWER_INFO		0x00000614
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

#ifndef MSR_PP1_ENERGY_STATUS
	#define MSR_PP1_ENERGY_STATUS		0x00000641
#endif

#ifndef MSR_PKG_C3_RESIDENCY
	#define MSR_PKG_C3_RESIDENCY		0x000003f8
#endif

#ifndef MSR_PKG_C6_RESIDENCY
	#define MSR_PKG_C6_RESIDENCY		0x000003f9
#endif

#ifndef MSR_ATOM_PKG_C6_RESIDENCY
	#define MSR_ATOM_PKG_C6_RESIDENCY	0x000003fa
#endif

#ifndef MSR_PKG_C7_RESIDENCY
	#define MSR_PKG_C7_RESIDENCY		0x000003fa
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

#ifndef MSR_PKG_C2_RESIDENCY
	#define MSR_PKG_C2_RESIDENCY		0x0000060d
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

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		BSP		:  9-8,
		ReservedBits2	: 10-9,
		EXTD		: 11-10,
		EN		: 12-11,
		Addr		: 64-12;
	};
} LOCAL_APIC;

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
		IBRS		:  1-0, /* CPUID(EAX=07H,ECX=0):EDX[26] == 1 */
		STIBP		:  2-1, /* CPUID(EAX=07H,ECX=0):EDX[27] == 1 */
		SSBD		:  3-2, /* CPUID(EAX=07H,ECX=0):EDX[31] == 1 */
		ReservedBits	: 64-3;
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
{	/* R/O && CPUID.(EAX=07H,ECX=0):EDX[29] == 1			*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		RDCL_NO 	:  1-0,
		IBRS_ALL	:  2-1,
		RSBA		:  3-2,
		L1DFL_VMENTRY_NO:  4-3,
		SSB_NO		:  5-4,
		MDS_NO		:  6-5,
		PSCHANGE_MC_NO	:  7-6,
		TSX_CTRL	:  8-7,
		TAA_NO		:  9-8,
		ReservedBits	: 64-9;
	};
} ARCH_CAPABILITIES;

typedef union
{	/* 06_86 [TREMONT]						*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  5-0,
		SPLA_EXCEPTION	:  6-5, /*Exception for split locked accesses*/
		ReservedBits2	: 64-6;
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
		ProgrammableTDP : 30-29,
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
		ReservedBits3	: 24-16,
		Int_Filtering	: 25-24, /* Nehalem			*/
		C3autoDemotion	: 26-25,
		C1autoDemotion	: 27-26,
		C3undemotion	: 28-27, /* Sandy Bridge		*/
		C1undemotion	: 29-28, /* Sandy Bridge		*/
		ReservedBits4	: 64-29;
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
{	/* MSR_TURBO_RATIO_LIMIT1(1AEh)					*/
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
{	/* MSR_TURBO_RATIO_LIMIT2(1AFh)					*/
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
		Power		: 15-0,
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
		ReservedBits1	: 31-8,
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
		FastStrings	:  1-0,  /* Fast-Strings Enable		*/
		ReservedBits1	:  2-1,
		x87Compat_Enable:  3-2,  /* Pentium4, Xeon		*/
		TCC		:  4-3,  /* Automatic Thermal Control Circuit */
		SplitLockDisable:  5-4,  /* Pentium4, Xeon		*/
		ReservedBits2	:  6-5,
		L3Cache_Disable :  7-6,  /* Pentium4, Xeon		*/
		PerfMonitoring	:  8-7,  /* Performance Monitoring Available */
		SupprLock_Enable:  9-8,  /* Pentium4, Xeon		*/
		PrefetchQueueDis: 10-9,  /* Pentium4, Xeon		*/
		Int_FERR_Enable : 11-10, /* Pentium4, Xeon		*/
		BTS		: 12-11, /* Branch Trace Storage Unavailable */
		PEBS		: 13-12, /* No Precise Event Based Sampling */
		TM2_Enable	: 14-13,
		ReservedBits3	: 16-14,
		EIST		: 17-16, /* Enhanced Intel SpeedStep Tech. */
		ReservedBits4	: 18-17,
		FSM		: 19-18,
		PrefetchCacheDis: 20-19, /* Pentium4, Xeon		*/
		ReservedBits5	: 22-20,
		CPUID_MaxVal	: 23-22,
		xTPR		: 24-23,
		L1DataCacheMode : 25-24, /* Pentium4, Xeon		*/
		ReservedBits6	: 34-25,
		XD_Bit_Disable	: 35-34,
		ReservedBits7	: 37-35,
		DCU_Prefetcher	: 38-37,
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
		ReservedBits1	:  1-0,
		C1E		:  2-1,
		ReservedBits2	: 19-2,
		Race2Halt_Optim : 20-19, /* SKL, KBL, CFL: Disable=1	*/
		Energy_Optim	: 21-20, /* SKL, KBL, CFL: Disable=1	*/
		ReservedBits3	: 25-21,
		EBP_OS_Control	: 26-25, /* SNB: 0=EBP controlled by OS */
		ReservedBits4	: 64-26;
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
		ReservedBits1	: 22-2,
		Therm_Intr_Coord: 23-22, /* Pkg: Goldmont 0=Disable; 1=Enable */
		ReservedBits2	: 64-23;
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
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		StatusBit	:  1-0,
		StatusLog	:  2-1,
		PROCHOT 	:  3-2,
		PROCHOTLog	:  4-3,
		CriticalTemp	:  5-4,
		CriticalTempLog :  6-5,
		Threshold1	:  7-6,
		Threshold1Log	:  8-7,
		Threshold2	:  9-8,
		Threshold2Log	: 10-9,
		PwrLimitStatus	: 11-10,
		PwrLimitLog	: 12-11,
		CurLimitStatus	: 13-12,	/* HWP Feedback 	*/
		CurLimitLog	: 14-13,	/* HWP Feedback 	*/
		XDomLimitStatus : 15-14,	/* HWP Feedback 	*/
		XDomLimitLog	: 16-15,	/* HWP Feedback 	*/
		DTS		: 23-16,
		ReservedBits1	: 27-23,
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
		ReservedBits1	: 16-0,
		TM_SELECT	: 17-16, /* Unique(Core2), Shared(Xeon, Atom) */
		ReservedBits2	: 64-17;
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
		ReservedBits2	: 64-24;
	} Core; /* Core, NHM, SNB and superior architectures		*/
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
} PKG_POWER_INFO;

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
		tPCHG		:  1-0,
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
		tWR_RD		: 11-8,
		tWTR		: 16-11,
		tRCD_RD		: 20-16,
		ReservedBits	: 24-20,
		tREF		: 32-24;	/* Offset 25Bh		*/
	};
} P965_MC_CYCTRK_RD;

typedef union
{	/* Offset Channel0: 29Ch & Channel1: 69Ch			*/
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
{	/* Offset Channel0: 250h & Channel1: 650h			*/
	unsigned short		value;
	struct {
		unsigned short
		tPCHG		:  1-0,
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
		tWR_RD		: 12-8,
		tWTR		: 17-12,
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
		Pchg_PD		: 13-12,
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
	struct {	/* Xeon E7 v2 & Xeon E5 v2			*/
		unsigned int
		QPIFREQSEL	:  3-0,  /*010=5600,011=6400,100=7200,101=8000*/
		ReservedBits1	:  4-3,
		Slow_Mode	:  5-4,
		ReservedBits2	: 32-5;
	} IVB_EP; /*TODO( was first defined in SNB_EP as QPIMISCSTAT )	*/
} QPI_FREQUENCY;


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
{	/* Device: 0 - Function: 0 - Offset Channel0: 4004h & Channel1: 4400h */
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
	/* Device 16,30 - Function: 0,1,4,5 - Offset 208h		*/
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
	};
} SNB_CAPID;	/* §2.5.33 CAPID0_A Capabilities A Register		*/

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
		unsigned int
		CLOSE_PG	:  1-0,
		LS_EN		:  2-1,
		ECC_EN		:  3-2,
		DIR_EN		:  4-3,
		ReservedBits1	:  8-4,
		Mode		:  9-8,
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
} IVB_CAPID;	/* §2.5.39 CAPID0_B Capabilities B Register		*/

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
{	/* Device: 0 - Function: 0 - Offset Channel0: 423Ch & Channel1: 463Ch */
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 26-16,
		ReservedBits	: 32-26;
	};
} SKL_IMC_REFRESH_TC;	/* Refresh timing parameters			*/

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

typedef union
{	/* Device: 0 - Function: 0 - Offset Channel0: 500Ch & Channel1: 5010h */
	unsigned int		value;
	struct {
		unsigned int
		Dimm_L_Size	:  6-0,  /* Size of DIMM in 1024 MB multiples */
		ReservedBits1	:  8-6,
		DLW		: 10-8, /*DIMM L chips width:0=x8,1=x16,10=x32*/
		DLNOR		: 11-10, /* DIMM L number of ranks	*/
		DL8Gb		: 12-11, /* DDR3 DIMM L: 1=8Gb or zero	*/
		ReservedBits2	: 16-12,
		Dimm_S_Size	: 22-16,
		ReservedBits3	: 24-22,
		DSW		: 26-24, /* DIMM S chips width		*/
		DSNOR		: 27-26, /* DIMM S # of ranks: 0=1 , 1=2 */
		DS8Gb		: 28-27,
		ReservedBits4	: 32-28;
	};
} SKL_IMC_MAD_DIMM;

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
} SKL_SA_PLL_RATIOS;	/* 06_4E/06_5E					*/

typedef union
{	/* Device: 0 - Function: 0 - Offset E4h				*/
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
	};
} SKL_CAPID_A;	/* §3.39 CAPID0_A Capabilities A Register		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset E8h				*/
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
	};
} SKL_CAPID_B;	/* §3.40 CAPID0_B Capabilities B Register		*/

typedef union
{	/* Device: 0 - Function: 0 - Offset ECh				*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 14-0,
		DMFC_LPDDR3	: 17-14,
		DMFC_DDR4	: 20-17,
		ReservedBits2	: 32-20;
	};
} SKL_CAPID_C;	/* §3.41 CAPID0_C Capabilities C Register		*/

