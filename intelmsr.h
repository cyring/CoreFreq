/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#ifndef MSR_MISC_PWR_MGMT
#define MSR_MISC_PWR_MGMT 0x1aa
#endif

#ifndef MSR_TURBO_RATIO_LIMIT
#define MSR_TURBO_RATIO_LIMIT MSR_NHM_TURBO_RATIO_LIMIT
#endif

#ifndef MSR_PKG_CST_CONFIG_CONTROL
#define MSR_PKG_CST_CONFIG_CONTROL MSR_NHM_SNB_PKG_CST_CFG_CTL
#endif

#ifndef MSR_PMG_IO_CAPTURE_BASE
#define MSR_PMG_IO_CAPTURE_BASE 0xe4
#endif

#define MSR_NHM_UNCORE_PERF_GLOBAL_CTRL 	0x391
#define MSR_SKL_UNCORE_PERF_GLOBAL_CTRL 	0xe01
#define MSR_NHM_UNCORE_PERF_GLOBAL_STATUS	0x392
#define MSR_SKL_UNCORE_PERF_GLOBAL_STATUS	0xe02
#define MSR_UNCORE_PERF_GLOBAL_OVF_CTRL 	0x393
#define MSR_UNCORE_PERF_FIXED_CTR0		0x394
#define MSR_UNCORE_PERF_FIXED_CTR_CTRL		0x395

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

typedef	union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		CurrVID		:  8-0,
		CurrFID		: 16-8,
		ReservedBits1	: 31-16,
		XE_Enable	: 32-31, // Intel Core
		ReservedBits2	: 40-32,
		MaxBusRatio	: 45-40, // Architectural
		ReservedBits3	: 46-45,
		NonInt_BusRatio	: 47-46,
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
		CurrentRatio	: 16-0,
		ReservedBits1	: 32-16,
		CurrVID		: 48-32, // Core Voltage ID (Sandy Bridge)
		ReservedBits2	: 64-48;
	} SNB;
} PERF_STATUS;

typedef	union
{
	unsigned long long value;
	struct
	{
		unsigned long long
		EIST_Target	: 16-0,
		ReservedBits1	: 32-16,
		Turbo_IDA	: 33-32, // IDA Disengage bit w/ Mobile [06_0F]
		ReservedBits2	: 64-33;
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
		ReservedBits2	: 28-16,
		Ratio_Limited	: 29-28,
		TDC_TDP_Limited	: 30-29,
		ReservedBits3	: 32-30,
		LowPowerMode	: 33-32,
		ConfigTDPlevels	: 35-33,
		ReservedBits4	: 40-35,
		MinimumRatio	: 48-40,
		MinOperatRatio	: 56-48, // Ivy Bridge, Haswell(-E)
		ReservedBits5	: 64-56;
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
		Int_Filtering	: 25-24, // Nehalem
		C3autoDemotion	: 26-25,
		C1autoDemotion	: 27-26,
		C3undemotion	: 28-27, // Sandy Bridge
		C1undemotion	: 29-28, // Sandy Bridge
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
		CStateRange	: 19-16, // R/W
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
		MaxRatio_1C	:  8-0,  // NHM, SNB, HSW, BDW, SKL, KBL
		MaxRatio_2C	: 16-8,  // NHM, SNB, HSW, BDW, SKL, KBL
		MaxRatio_3C	: 24-16, // NHM, SNB, HSW, BDW, SKL, KBL
		MaxRatio_4C	: 32-24, // NHM, SNB, HSW, BDW, SKL, KBL
		MaxRatio_5C	: 40-32, // Westmere, SNB-E5, E3v4
		MaxRatio_6C	: 48-40, // Westmere, SNB-E5, E3v4
		MaxRatio_7C	: 56-48, // SNB-E5, E5v3, E5v4, SKL-X
		MaxRatio_8C	: 64-56; // AVT, GLM, SNB-E5, E5v3, E5v4, SKL-X
	};
} TURBO_RATIO_CONFIG0;

typedef union
{	// MSR_TURBO_RATIO_LIMIT1(1AEh)
	unsigned long long	value;
	struct
	{	// Haswell-E5v3 [06_3F], Broadwell-E [06_56] & Xeon E5v4 [06_4F]
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
	{	// Xeon IvyBridge-EPv2 [06_3E]
		unsigned long long
		MaxRatio_9C	:  8-0, 	// E5 + E7
		MaxRatio_10C	: 16-8, 	// E5 + E7
		MaxRatio_11C	: 24-16,	// E5 + E7
		MaxRatio_12C	: 32-24,	// E5 + E7
		MaxRatio_13C	: 40-32,	// E7
		MaxRatio_14C	: 48-40,	// E7
		MaxRatio_15C	: 56-48,	// E7
		ReservedBits	: 63-56,	// E7
		Semaphore	: 64-63;	// E7
	} IVB_EP;
	struct
	{	// Skylake_X [06_55]
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
{	// MSR_TURBO_RATIO_LIMIT2(1AFh)
	unsigned long long	value;
	struct
	{	// Xeon Haswell-E5v3 [06_3F]
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
		FastStrings	:  1-0,  // Fast-Strings Enable
		ReservedBits1	:  2-1,
		x87Compat_Enable:  3-2,  // Pentium4, Xeon
		TCC		:  4-3,  // Automatic Thermal Control Circuit
		SplitLockDisable:  5-4,  // Pentium4, Xeon
		ReservedBits2	:  6-5,
		L3Cache_Disable	:  7-6,  // Pentium4, Xeon
		PerfMonitoring	:  8-7,  // Performance Monitoring Available
		SupprLock_Enable:  9-8,  // Pentium4, Xeon
		PrefetchQueueDis: 10-9,  // Pentium4, Xeon
		Int_FERR_Enable	: 11-10, // Pentium4, Xeon
		BTS		: 12-11, // Branch Trace Storage Unavailable
		PEBS		: 13-12, // Precise Event Based Sampling Unavail
		TM2_Enable	: 14-13,
		ReservedBits3	: 16-14,
		EIST		: 17-16, // Enhanced Intel SpeedStep Technology
		ReservedBits4	: 18-17,
		FSM		: 19-18,
		PrefetchCacheDis: 20-19, // Pentium4, Xeon
		ReservedBits5	: 22-20,
		CPUID_MaxVal	: 23-22,
		xTPR		: 24-23,
		L1DataCacheMode	: 25-24, // Pentium4, Xeon
		ReservedBits6	: 34-25,
		XD_Bit_Disable	: 35-34,
		ReservedBits7	: 37-35,
		DCU_Prefetcher	: 38-37,
		Turbo_IDA	: 39-38, // Disable=1 -> CPUID(0x6).IDA=0
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
		ReservedBits2	: 64-2;
	};
} POWER_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		DutyCycle	:  4-0,  // OnDemand Clock Modulation Duty Cycle
		ODCM_Enable	:  5-4,
		ReservedBits	: 63-5,
		ECMD		: 64-63; // Placeholder for CPUID(0x6)AX.5
	};
} CLOCK_MODULATION;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PowerPolicy	:  4-0, // 0=highest perf; 15=Max energy saving*
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
		HW_Coord_EIST	:  1-0,  // Pkg: 0=Enable; 1=Disable
		Perf_BIAS_Enable:  2-1,  // SMT: 1=Enable; 0=Disable*
		ReservedBits1	: 22-2,
		Therm_Intr_Coord: 23-22, // Pkg: Goldmont 0=Disable; 1=Enable
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
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0,		// PM2
		Overflow_PMC1	:  2-1,		// PM2
		Overflow_PMC2	:  3-2,		// PM3
		Overflow_PMC3	:  4-3,		// PM3
		Overflow_PMCn	: 32-4,		// PM3
		Overflow_CTR0	: 33-32,	// PM2
		Overflow_CTR1	: 34-33,	// PM2
		Overflow_CTR2	: 35-34,	// PM2
		ReservedBits1	: 55-35,
		TraceToPAPMI	: 56-55,	// PM4, PM3(Broadwell)
		ReservedBits2	: 58-56,
		LBR_Frz		: 59-58,	// PM4
		CTR_Frz		: 60-59,	// PM4
		ASCI		: 61-60,	// PM4
		Overflow_UNC	: 62-61,	// PM3
		Overflow_Buf	: 63-62,	// PM2
		Ovf_CondChg	: 64-63;	// PM2
	};
} CORE_GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0,		// PM2
		Clear_Ovf_PMC1	:  2-1,		// PM2
		Clear_Ovf_PMC2	:  3-2,		// PM3
		Clear_Ovf_PMC3	:  4-3,		// PM3
		Clear_Ovf_PMCn	: 32-4,		// PM3
		Clear_Ovf_CTR0 	: 33-32,	// PM2
		Clear_Ovf_CTR1	: 34-33,	// PM2
		Clear_Ovf_CTR2	: 35-34,	// PM2
		ReservedBits1	: 55-35,
		ClrTraceToPA_PMI: 56-55,	// PM4, PM3(Broadwell)
		ReservedBits2	: 61-56,
		Clear_Ovf_UNC	: 62-61,	// PM3
		Clear_Ovf_Buf	: 63-62,	// PM2
		Clear_CondChg	: 64-63;	// PM2
	};
} CORE_GLOBAL_PERF_OVF_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0		:  1-0,		// PM2
		EN_PMC1		:  2-1,		// PM2
		EN_PMC2		:  3-2,		// PM3
		EN_PMC3		:  4-3,		// PM3
		EN_PMCn		: 32-4,		// PM3
		EN_FIXED_CTR0	: 33-32,	// PM2
		EN_FIXED_CTR1	: 34-33,	// PM2
		EN_FIXED_CTR2	: 35-34,	// PM2
		ReservedBits	: 64-35;
	};
} CORE_GLOBAL_PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN0_OS		:  1-0,		// PM2
		EN0_Usr		:  2-1,		// PM2
		AnyThread_EN0	:  3-2,		// PM3
		EN0_PMI		:  4-3,		// PM2
		EN1_OS		:  5-4,		// PM2
		EN1_Usr		:  6-5,		// PM2
		AnyThread_EN1	:  7-6,		// PM3
		EN1_PMI		:  8-7,		// PM2
		EN2_OS		:  9-8,		// PM2
		EN2_Usr		: 10-9,		// PM2
		AnyThread_EN2	: 11-10,	// PM3
		EN2_PMI		: 12-11,	// PM2
		ReservedBits	: 64-12;
	};
} CORE_FIXED_PERF_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		StatusBit       :  1-0,
		StatusLog       :  2-1,
		PROCHOT         :  3-2,
		PROCHOTLog      :  4-3,
		CriticalTemp    :  5-4,
		CriticalTempLog :  6-5,
		Threshold1      :  7-6,
		Threshold1Log   :  8-7,
		Threshold2      :  9-8,
		Threshold2Log   : 10-9,
		PwrLimitStatus	: 11-10,
		PwrLimitLog	: 12-11,
		CurLimitStatus	: 13-12,
		CurLimitLog	: 14-13,
		CrDomLimitStatus: 15-14,
		CrDomLimitLog	: 16-15,
		DTS             : 23-16,
		ReservedBits1   : 27-23,
		Resolution      : 31-27,
		ReadingValid    : 32-31,
		ReservedBits2   : 64-32;
	};
} THERM_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	: 16-0,
		TM_SELECT	: 17-16, // Unique(Core2), Shared(Xeon, Atom)
		ReservedBits2	: 64-17;
	};
} THERM2_CONTROL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1   : 16-0,
		Target          : 24-16,
		ReservedBits2   : 64-24;
	};
} TJMAX;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0,		// R/O , NHM, SNB
		Overflow_PMC1	:  2-1,		// R/O , NHM, SNB
		Overflow_PMC2	:  3-2,		// R/O , NHM, SNB
		Overflow_PMC3	:  4-3,		// R/O , NHM, SNB
		Overflow_PMC4	:  5-4,		// R/O , NHM, *CPUID(0xa)
		Overflow_PMC5	:  6-5,		// R/O , NHM, *CPUID(0xa)
		Overflow_PMC6	:  7-6,		// R/O , NHM, *CPUID(0xa)
		Overflow_PMC7	:  8-7,		// R/O , NHM, *CPUID(0xa)
		ReservedBits1	: 32-8,
		Overflow_CTR0	: 33-32,	// R/O , NHM
		Overflow_CTR1	: 34-33,	// R/O , SNB
		Overflow_CTR2	: 35-34,	// R/O , SNB
		ReservedBits2	: 61-35,
		Overflow_PMI	: 62-61,	// R/W , NHM, SNB
		Ovf_DSBuffer	: 63-62,	// SNB
		Ovf_CondChg	: 64-63;	// R/W , NHM, SNB
	} NHM;	// PMU: 06_1AH
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0,
		Overflow_ARB	:  2-1,
		ReservedBits1	:  3-2,
		Overflow_Cbox	:  4-3,
		ReservedBits2	: 64-4;
	} SNB;	// PMU: 06_2AH/06_3CH/06_45H/06_46H/06_4EH/06_5EH
} UNCORE_GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0,		// NHM, SNB
		Clear_Ovf_PMC1	:  2-1,		// NHM, SNB
		Clear_Ovf_PMC2	:  3-2,		// NHM, SNB
		Clear_Ovf_PMC3	:  4-3,		// NHM, SNB
		Clear_Ovf_PMC4	:  5-4,		// NHM, SNB
		Clear_Ovf_PMC5	:  6-5,		// NHM, *CPUID(0xa)
		Clear_Ovf_PMC6	:  7-6,		// NHM, *CPUID(0xa)
		Clear_Ovf_PMC7	:  8-7,		// NHM, *CPUID(0xa)
		ReservedBits1	: 32-8,
		Clear_Ovf_CTR0 	: 33-32,	// NHM, SNB
		Clear_Ovf_CTR1 	: 34-33,	// SNB
		Clear_Ovf_CTR2 	: 35-34,	// SNB
		ReservedBits2	: 61-35,
		Clear_Ovf_PMI 	: 62-61,	// NHM, SNB
		Clear_Ovf_DSBuf : 63-62,	// SNB
		Clear_CondChg	: 64-63;	// NHM, SNB
	};
} UNCORE_GLOBAL_PERF_OVF_CTRL;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0		:  1-0,		// R/W , NHM
		EN_PMC1		:  2-1,		// R/W
		EN_PMC2		:  3-2,		// R/W
		EN_PMC3		:  4-3,		// R/W
		EN_PMC4		:  5-4,		// R/W , NHM, *CPUID(0xa)
		EN_PMC5		:  6-5,		// R/W , NHM, *CPUID(0xa)
		EN_PMC6		:  7-6,		// R/W , NHM, *CPUID(0xa)
		EN_PMC7		:  8-7,		// R/W , NHM, *CPUID(0xa)
		ReservedBits1	: 32-8,
		EN_FIXED_CTR0	: 33-32,	// R/W , NHM, SNB
		EN_FIXED_CTR1	: 34-33,	// R/W , SNB
		EN_FIXED_CTR2	: 35-34,	// R/W , SNB
		ReservedBits2	: 48-35,
		EN_PMI_CORE0	: 49-48,	// R/W , NHM
		EN_PMI_CORE1	: 50-49,	// R/W , NHM
		EN_PMI_CORE2	: 51-50,	// R/W , NHM
		EN_PMI_CORE3	: 52-51,	// R/W , NHM
		ReservedBits3	: 63-52,
		PMI_FRZ		: 64-63;	// R/W , NHM
	} NHM;	// PMU: 06_1AH
	struct
	{
		unsigned long long
		EN_Slice0	:  1-0,
		EN_Slice1	:  2-1,
		EN_Slice2	:  3-2,
		EN_Slice3	:  4-3,
		EN_Slice4	:  5-4,
		ReservedBits1	: 29-5,
		EN_COUNTERS	: 30-29,
		EN_WakeOn_PMI	: 31-30,
		PMI_FRZ		: 32-31,
		ReservedBits2	: 64-32;
	} SNB;	// PMU: 06_2AH/06_3CH/06_45H/06_46H/06_4EH/06_5EH
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
		EN_PMC0		:  1-0,
		ReservedBits1	:  2-1,
		EN_PMI		:  3-2,
		ReservedBits2	: 64-3;
	} NHM;	// PMU: 06_1AH
	struct
	{
		unsigned long long
		ReservedBits1	: 20-1,
		EN_Overflow	: 21-20,
		ReservedBits2	: 22-21,
		EN_PMC0		: 23-22,
		ReservedBits3	: 64-23;
	} SNB;	// PMU: 06_2AH/06_3CH/06_45H/06_46H/06_4EH/06_5EH
} UNCORE_FIXED_PERF_CONTROL;

/* ToDo
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
{	// Offset Channel0: 1210h & Channel1: 1310h
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
{	// Offset Channel0: 1214h & Channel1: 1314h
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
{	// Offset Channel0: 1218h & Channel1: 1318h
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
{	// Offset Channel0: 121Ch & Channel1: 131Ch
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
{	// Offset Channel0: 1200h & Channel1: 1300h
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
{	// Offset Channel0: 1208h
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
{	// Offset Channel0: 250h & Channel1: 650h
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
{	// Offset Channel0: 252h & Channel1: 652h
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
{	// Offset Channel0: 256h & Channel1: 656h
	unsigned short		value;
	struct {
		unsigned short
		tRD_WR		:  4-0,
		tWR_WR_DR	:  8-4,
		tWR_WR_SR	: 12-8,
		tRCD_WR		: 16-12;
	};
} P965_MC_CYCTRK_WR;

typedef union
{	// Offset Channel0: 258h & Channel1: 658h
	unsigned int		value;
	struct {
		unsigned int
		tRD_RD_DR	:  4-0,
		tRD_RD_SR	:  8-4,
		tWR_RD		: 11-8,
		tWTR		: 16-11,
		tRCD_RD		: 20-16,
		ReservedBits	: 24-20,
		tREF		: 32-24;	// Offset 25Bh
	};
} P965_MC_CYCTRK_RD;

typedef union
{	// Offset Channel0: 29Ch & Channel1: 69Ch
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
{	// Offset Channel0: 260h & Channel1: 660h
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
{	// Offset Channel0: 250h & Channel1: 650h
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
{	// Offset Channel0: 252h & Channel1: 652h
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
{	// Offset Channel0: 256h & Channel1: 656h
	unsigned short		value;
	struct {
		unsigned short
		tRD_WR		:  4-0,
		tWR_WR_DR	:  8-4,
		tWR_WR_SR	: 12-8,
		tRCD_WR		: 16-12;
	};
} P35_MC_CYCTRK_WR;

typedef union
{	// Offset Channel0: 258h & Channel1: 658h
	unsigned int		value;
	struct {
		unsigned int
		tRD_RD_DR	:  4-0,
		tRD_RD_SR	:  8-4,
		tWR_RD		: 12-8,
		tWTR		: 17-12,
		tRCD_RD		: 21-17,
		ReservedBits	: 24-21,
		tREF		: 32-24;	// Offset 25Bh
	};
} P35_MC_CYCTRK_RD;

typedef union
{	// Offset: 265h
	unsigned short		value;
	struct {
		unsigned short
		UnknownBits1	:  8-0,
		tCL		: 14-8,
		UnknownBits2	: 16-14;
	};
} P35_MC_UNKNOWN_R0;

typedef union
{	// Offset: 25Dh
	unsigned short		value;
	struct {
		unsigned short
		tRAS		:  6-0,
		UnknownBits	: 16-6;
	};
} P35_MC_UNKNOWN_R1;

typedef union
{	// Offset Channel0: 260h & Channel1: 660h
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


typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 70h
	unsigned int		value;
	struct { // Source: Micron DDR3
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

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 74h
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
		RC0		: 20-16,	// RDIMMS
		RC2		: 24-20,	// RDIMMS
		MR3		: 32-24;
	};
} NHM_IMC_MRS_VALUE_2_3;

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 80h
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

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 84h
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

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 88h
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

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 8Ch
	unsigned int		value;
	struct {
		unsigned int
		tRFC		:  9-0,
		tREFI_8		: 19-9,
		tTHROT_OPPREF	: 30-19,
		ReservedBits	: 32-30;
	};
} NHM_IMC_REFRESH_TIMING;

typedef union	// Bloomfield, Lynnfield
{	// Device: 3 - Function: 0 - Offset: 48h
	unsigned int		value;
	struct {
		unsigned int
		CLOSED_PAGE	:  1-0,
		EN_ECC		:  2-1, // Bloomfield
		AUTOPRECHARGE	:  3-2,
		CHANNELRESET0	:  4-3,
		CHANNELRESET1	:  5-4,
		CHANNELRESET2	:  6-5, // Bloomfield
		DIVBY3EN	:  7-6,
		INIT_DONE	:  8-7,
		CHANNEL0_ACTIVE :  9-8,
		CHANNEL1_ACTIVE : 10-9,
		CHANNEL2_ACTIVE : 11-10, // Bloomfield
		ReservedBits	: 32-11;
	};
} NHM_IMC_CONTROL;

typedef union	// Bloomfield, Lynnfield
{	// Device: 3 - Function: 0 - Offset: 4Ch
	unsigned int		value;
	struct {
		unsigned int
		CHANNEL0_DISABLE:  1-0,
		CHANNEL1_DISABLE:  2-1,
		CHANNEL2_DISABLE:  3-2, // Bloomfield
		ReservedBits	:  4-3,
		ECC_ENABLED	:  5-4; // Bloomfield
	};
} NHM_IMC_STATUS;

typedef union	// Bloomfield, Lynnfield
{	// Device: 3 - Function: 0 - Offset: 64h
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

typedef union	// Bloomfield, Lynnfield
{	// Device: 3 - Function: 4 - Offset: 50h
	unsigned int		value;
	struct {
		unsigned int
		QCLK_RATIO	:  5-0,
		ReservedBits1	: 24-5,
		MAX_RATIO	: 29-24,
		ReservedBits2	: 32-29;
	};
} NHM_IMC_CLK_RATIO_STATUS;

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: B8h
	unsigned int		value;
	struct {
		unsigned int
		PRIORITY_CNT	:  3-0,
		ENABLE_2N_3N	:  5-3, // 00=1N , 01=2N , 10=3N , 11=Reserved
		DIS_ISOC_RBC	:  6-5,
		PRE_CAS_THRSHLD : 11-6,
		FLOAT_EN	: 12-11,
		CS_FOR_CKE_TRANS: 13-12, // Lynnfield: CS_ODT_TRISTATE_DISABLE.
		DDR_CLK_TRISTATE: 14-13, // Lynnfield: Disable status bit.
		ReservedBits	: 32-14;
	};
} NHM_IMC_SCHEDULER_PARAMS;

typedef union	// Bloomfield, Lynnfield
{	// Device: 4, 5, 6 - Function: 0 - Offset: 48h, 4Ch, 50h, Lynnfield(54h)
	unsigned int		value;
	struct {
		unsigned int
		NUMCOL		:  2-0,
		NUMROW		:  5-2,
		NUMRANK		:  7-5,
		NUMBANK		:  9-7,
		DIMMPRESENT	: 10-9,
		RANKOFFSET	: 13-10,
		ReservedBits	: 32-13;
	};
} NHM_IMC_DOD_CHANNEL;

typedef union
{	// X58 IOH Control Status & RAS Registers: Dev: 20 - Func: 2 - Off: D0h
	unsigned int		value;
	struct {
		unsigned int
		QPIFREQSEL	:  2-0, // 00=4800 GT/s, 10=6400 GT/s
		ReservedBits	: 32-2;
	};
} X58_QPI_FREQUENCY;


typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 4000h & Channel1: 4400h
	unsigned int		value;
	struct {
		unsigned int
		tRCD		:  4-0,
		tRP		:  8-4,
		tCL		: 12-8,
		ReservedBits	: 32-12;
	};
} C200_TC_DBP;

typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 4004h & Channel1: 4400h
	unsigned int		value;
	struct {
		unsigned int
		tRRD		:  4-0,
		tCAS_RTPr	:  8-4,
		ReservedBits1	: 12-8,
		tiWrTRd		: 16-12,
		srACT		: 24-16,
		ReservedBits2	: 32-24;
	};
} C200_TC_RAP;

typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 4298h & Channel1: 4698h
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 25-16,
		tREFIx9		: 32-25;
	};
} C200_TC_RFTP;

typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 5004h & Channel1: 5008h
	unsigned int		value;
	struct {
		unsigned int
		Dimm_A_Size	:  8-0,
		Dimm_B_Size	: 16-8,
		DAS		: 17-16,	// where DIMM A is selected
		DANOR		: 18-17,	// DIMM A number of ranks
		DBNOR		: 19-18,	// DIMM B number of ranks
		DAW		: 20-19,	// DIMM A chips width:0=x8,1=x16
		DBW		: 21-20,	// DIMM B chips width
		RI		: 22-21,	// Rank Interleave: 0=OFF, 1=ON
		ENH_IM		: 23-22,	// Enhanced interleave mode
		ReservedBits1	: 24-23,
		ECC		: 26-24,	// active?0=No,1=IO,2=NoIO,3=All
		ReservedBits2	: 32-26;
	};
} C200_MAD_CHANNEL;


typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 4c04h
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 30-0,
		CMD_Stretch	: 32-30;
	};
} C220_DDR_TIMING;

typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 4c14h
	unsigned int		value;
	struct {
		unsigned int
		tCL		:  5-0,
		tCWL		: 10-5,
		ReservedBits	: 32-10;
	};
} C220_DDR_RANK_TIMING;

typedef union
{	// Device: 0 - Function: 0 - Offset Channel0: 4e98h
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		tRFC		: 25-16,
		tREFIx9		: 32-25;
	};
} C220_TC_REFRESH_TIMING;
