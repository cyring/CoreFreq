/*
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	DRV_DEVNAME "intelfreq"
#define	DRV_FILENAME "/dev/"DRV_DEVNAME

#define	ROUND_TO_PAGES(Size)	PAGE_SIZE * ((Size / PAGE_SIZE) 	\
				+ ((Size % PAGE_SIZE)? 1:0));

#define MAX(M, m)	((M) > (m) ? (M) : (m))
#define MIN(m, M)	((m) < (M) ? (m) : (M))

#define	PRECISION	100
#define	LOOP_MIN_MS	100
#define LOOP_MAX_MS	5000
#define	LOOP_DEF_MS	1000

#define MAXCOUNTER(M, m)	((M) > (m) ? (M) : (m))
#define MINCOUNTER(m, M)	((m) < (M) ? (m) : (M))

#define RDCOUNTER(_val,  _cnt)						\
({									\
	unsigned int _lo, _hi;						\
									\
	asm volatile							\
	(								\
		"rdmsr"							\
                : "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_cnt)						\
	);								\
	_val=_lo | ((unsigned long long) _hi << 32);			\
})

#define WRCOUNTER(_val,  _cnt)						\
	asm volatile							\
	(								\
		"wrmsr"							\
		:							\
		: "c" (_cnt),						\
		  "a" ((unsigned int) _val & 0xFFFFFFFF),		\
		  "d" ((unsigned int) (_val >> 32))			\
	);

#define RDMSR(_data, _reg)						\
({									\
	unsigned int _lo, _hi;						\
									\
	asm volatile							\
	(								\
		"rdmsr"							\
                : "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_reg)						\
	);								\
	_data.value=_lo | ((unsigned long long) _hi << 32);		\
})

#define WRMSR(_data,  _reg)						\
	asm volatile							\
	(								\
		"wrmsr"							\
		:							\
		: "c" (_reg),						\
		  "a" ((unsigned int) _data.value & 0xFFFFFFFF),	\
		  "d" ((unsigned int) (_data.value >> 32))		\
	);

#define	RDTSC(_lo, _hi)							\
	asm volatile							\
	(								\
		"rdtsc"							\
		:"=a" (_lo),						\
		 "=d" (_hi)						\
	);

#define	RDTSCP(_lo, _hi, aux)						\
	asm volatile							\
	(								\
		"rdtscp"						\
		:"=a" (_lo),						\
		 "=d" (_hi),						\
		 "=c" (aux)						\
	);

#define	BARRIER()							\
	asm volatile							\
	(								\
		"mfence"						\
		:							\
		:							\
		:							\
	);

#define	RDTSC64(_val64)							\
	asm volatile							\
	(								\
		"mfence			\n\t"				\
		"rdtsc			\n\t"				\
		"mfence			\n\t"				\
		"movq	%%rax,	%%rsi	\n\t"				\
		"movq	%%rdx,	%%rdi	\n\t"				\
		"movq	$0x4,	%%rax	\n\t"				\
		"cpuid			\n\t"				\
		"shlq	$32,	%%rdi	\n\t"				\
		"orq	%%rdi,	%%rsi	\n\t"				\
		"movq	%%rsi,	%0"					\
		:"=m" (_val64)						\
		:							\
		:"%rax","%rbx","%rcx","%rdx","%rsi","%rdi","memory"	\
	);


typedef struct
{
	struct
	{
		unsigned char Chr[4];
	} AX, BX, CX, DX;
} BRAND;

typedef struct
{
	unsigned int	LargestStdFunc;
	struct
	{
		char	Brand[48],
			_pad48[2],
			VendorID[12],
			_pad62[2];
	};
	struct
	{
		struct SIGNATURE
		{
			unsigned int
			Stepping	:  4-0,
			Model		:  8-4,
			Family		: 12-8,
			ProcType	: 14-12,
			Unused1		: 16-14,
			ExtModel	: 20-16,
			ExtFamily	: 28-20,
			Unused2		: 32-28;
		} AX;
		struct
		{
			unsigned int
			Brand_ID	:  8-0,
			CLFSH_Size	: 16-8,
			MaxThread	: 24-16,
			Apic_ID		: 32-24;
		} BX;
		struct
		{
			unsigned int
			SSE3	:  1-0,
			PCLMULDQ:  2-1,
			DTES64	:  3-2,
			MONITOR	:  4-3,
			DS_CPL	:  5-4,
			VMX	:  6-5,
			SMX	:  7-6,
			EIST	:  8-7,
			TM2	:  9-8,
			SSSE3	: 10-9,
			CNXT_ID	: 11-10,
			Unused1	: 12-11,
			FMA	: 13-12,
			CX16	: 14-13,
			xTPR	: 15-14,
			PDCM	: 16-15,
			Unused2	: 17-16,
			PCID	: 18-17,
			DCA	: 19-18,
			SSE41	: 20-19,
			SSE42	: 21-20,
			x2APIC	: 22-21,
			MOVBE	: 23-22,
			POPCNT	: 24-23,
			TSCDEAD	: 25-24,
			AES	: 26-25,
			XSAVE	: 27-26,
			OSXSAVE	: 28-27,
			AVX	: 29-28,
			F16C	: 30-29,
			RDRAND	: 31-30,
			Unused3	: 32-31;
		} CX;
		struct
		{
			unsigned int
			FPU	:  1-0,
			VME	:  2-1,
			DE	:  3-2,
			PSE	:  4-3,
			TSC	:  5-4,
			MSR	:  6-5,
			PAE	:  7-6,
			MCE	:  8-7,
			CX8	:  9-8,
			APIC	: 10-9,
			Unused1	: 11-10,
			SEP	: 12-11,
			MTRR	: 13-12,
			PGE	: 14-13,
			MCA	: 15-14,
			CMOV	: 16-15,
			PAT	: 17-16,
			PSE36	: 18-17,
			PSN	: 19-18,
			CLFSH	: 20-19,
			Unused2	: 21-20,
			DS_PEBS	: 22-21,
			ACPI	: 23-22,
			MMX	: 24-23,
			FXSR	: 25-24,
			SSE	: 26-25,
			SSE2	: 27-26,
			SS	: 28-27,
			HTT	: 29-28,
			TM1	: 30-29,
			Unused3	: 31-30,
			PBE	: 32-31;
		} DX;
	} Std;
	struct
	{
		struct
		{
			unsigned int
			SmallestSize	: 16-0,
			ReservedBits	: 32-16;
		} AX;
		struct
		{
			unsigned int
			LargestSize	: 16-0,
			ReservedBits	: 32-16;
		} BX;
		struct
		{
			unsigned int
			ExtSupported	:  1-0,
			BK_Int_MWAIT	:  2-1,
			ReservedBits	: 32-2;
		} CX;
		struct
		{
			unsigned int
			Num_C0_MWAIT	:  4-0,
			Num_C1_MWAIT	:  8-4,
			Num_C2_MWAIT	: 12-8,
			Num_C3_MWAIT	: 16-12,
			Num_C4_MWAIT	: 20-16,
			ReservedBits	: 32-20;
		} DX;
	} MONITOR_MWAIT_Leaf;
	struct
	{
		struct
		{
			unsigned int
			DTS	:  1-0,
			TurboIDA:  2-1,
			ARAT	:  3-2,
			Unused1	:  4-3,
			PLN	:  5-4,
			ECMD	:  6-5,
			PTM	:  7-6,
			Unused2	: 32-7;
		} AX;
		struct
		{
			unsigned int
			Threshld:  4-0,
			Unused1	: 32-4;
		} BX;
		struct
		{
			unsigned int
			HCF_Cap	:  1-0,
			ACNT_Cap:  2-1,
			Unused1	:  3-2,
			PEB_Cap	:  4-3,
			Unused2	: 32-4;
		} CX;
		struct
		{
			unsigned int
			Unused1	: 32-0;
		} DX;
	} Thermal_Power_Leaf;
	struct
	{
		struct
		{
			unsigned int
			Version	:  8-0,
			MonCtrs	: 16-8,
			MonWidth: 24-16,
			VectorSz: 32-24;
		} AX;
		struct
		{
			unsigned int
			CoreCycles	:  1-0,
			InstrRetired	:  2-1,
			RefCycles	:  3-2,
			LLC_Ref		:  4-3,
			LLC_Misses	:  5-4,
			BranchRetired	:  6-5,
			BranchMispred	:  7-6,
			ReservedBits	: 32-7;
		} BX;
		struct
		{
			unsigned int
			Unused1	: 32-0;
		} CX;
		struct
		{
			unsigned int
			FixCtrs	:  5-0,
			FixWidth: 13-5,
			Unused1	: 32-13;
		} DX;
	} Perf_Monitoring_Leaf;
	struct
	{
		struct
		{
			unsigned int
			MaxSubLeaf	: 32-0;
		} AX;
		struct
		{
			unsigned int
			FSGSBASE	:  1-0,
			TSC_ADJUST	:  2-1,
			Unused1		:  3-2,
			BMI1		:  4-3,
			HLE		:  5-4,
			AVX2		:  6-5,
			Unused2		:  7-6,
			SMEP		:  8-7,
			BMI2		:  9-8,
			FastStrings	: 10-9,
			INVPCID		: 11-10,
			RTM		: 12-11,
			QM		: 13-12,
			FPU_CS_DS	: 14-13,
			Unused3		: 32-14;
		} BX;
			unsigned int
		CX			: 32-0,
		DX			: 32-0;
	} ExtFeature;
	unsigned int	LargestExtFunc;
	struct
	{
		struct
		{
			unsigned int
			LAHFSAHF:  1-0,
			Unused1	: 32-1;
		} CX;
		struct
		{
			unsigned int
			Unused1	: 11-0,
			SYSCALL	: 12-11,
			Unused2	: 20-12,
			XD_Bit	: 21-20,
			Unused3	: 26-21,
			PG_1GB	: 27-26,
			RdTSCP	: 28-27,
			Unused4	: 29-28,
			IA64	: 30-29,
			Unused5	: 32-30;
		} DX;
	} ExtFunc;
	unsigned int	InvariantTSC,
                        HTT_enabled,
			FactoryFreq;
} FEATURES;

//	[GenuineIntel]
#define	_GenuineIntel	{.ExtFamily=0x0, .Family=0x0, .ExtModel=0x0, .Model=0x0}
//	[Core]		06_0EH (32 bits)
#define	_Core_Yonah	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xE}
//	[Core2]		06_0FH, 06_15H, 06_17H, 06_1D
#define	_Core_Conroe	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xF}
#define	_Core_Kentsfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x5}
#define	_Core_Yorkfield	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x7}
#define	_Core_Dunnington \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xD}
//	[Atom]		06_1CH, 06_26H, 06_27H (32bits), 06_35H (32bits), 06_36H
#define	_Atom_Bonnell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xC}
#define	_Atom_Silvermont \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x6}
#define	_Atom_Lincroft	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x7}
#define	_Atom_Clovertrail \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x5}
#define	_Atom_Saltwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x6}
//	[Silvermont]	06_37H, 06_4DH
#define	_Silvermont_637	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x7}
#define	_Silvermont_64D	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xD}
//	[Nehalem]	06_1AH, 06_1EH, 06_1FH, 06_2EH
#define	_Nehalem_Bloomfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xA}
#define	_Nehalem_Lynnfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xE}
#define	_Nehalem_MB	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xF}
#define	_Nehalem_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xE}
//	[Westmere]	06_25H, 06_2CH, 06_2FH
#define	_Westmere	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x5}
#define	_Westmere_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xC}
#define	_Westmere_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xF}
//	[Sandy Bridge]	06_2AH, 06_2DH
#define	_SandyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xA}
#define	_SandyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xD}
//	[Ivy Bridge]	06_3AH, 06_3EH
#define	_IvyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xA}
#define	_IvyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xE}
//	[Haswell]	06_3CH, 06_3FH, 06_45H, 06_46H
#define	_Haswell_DT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xC}
#define	_Haswell_MB	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xF}
#define	_Haswell_ULT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x5}
#define	_Haswell_ULX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x6}

enum {	GenuineIntel,		\
	Core_Yonah,		\
	Core_Conroe,		\
	Core_Kentsfield,	\
	Core_Yorkfield,		\
	Core_Dunnington,	\
	Atom_Bonnell,		\
	Atom_Silvermont,	\
	Atom_Lincroft,		\
	Atom_Clovertrail,	\
	Atom_Saltwell,		\
	Silvermont_637,		\
	Silvermont_64D,		\
	Nehalem_Bloomfield,	\
	Nehalem_Lynnfield,	\
	Nehalem_MB,		\
	Nehalem_EX,		\
	Westmere,		\
	Westmere_EP,		\
	Westmere_EX,		\
	SandyBridge,		\
	SandyBridge_EP,		\
	IvyBridge,		\
	IvyBridge_EP,		\
	Haswell_DT,		\
	Haswell_MB,		\
	Haswell_ULT,		\
	Haswell_ULX,		\
	ARCHITECTURES
};


typedef	union
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
		CurrentRatio	: 16-0,
		ReservedBits1	: 31-16,
		XE		: 32-31,
		ReservedBits2	: 40-32,
		MaxBusRatio	: 45-40,
		ReservedBits3	: 46-45,
		NonInt_BusRatio	: 47-46,
		ReservedBits4	: 64-47;
	};
} PERF_STATUS;
/*
typedef	struct
{
	unsigned long long
		EIST_Target	: 16-0,
		ReservedBits1	: 32-16,
		Turbo_IDA	: 33-32,
		ReservedBits2	: 64-33;
} PERF_CONTROL;
*/
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
		MinOpeRatio	: 56-48,
		ReservedBits5	: 64-56;
	};
} PLATFORM_INFO;
/*
typedef struct
{
	unsigned long long
		Pkg_CST_Limit	:  3-0,
		ReservedBits1	: 10-3,
		IO_MWAIT_Redir	: 11-10,
		ReservedBits2	: 15-11,
		CFG_Lock	: 16-15,
		ReservedBits3	: 24-16,
		Int_Filtering	: 25-24,	// Nehalem
		C3autoDemotion	: 26-25,
		C1autoDemotion	: 27-26,
		C3undemotion	: 28-27,	// Sandy Bridge
		C1undemotion	: 29-28,	// Sandy Bridge
		ReservedBits4	: 64-29;
} CSTATE_CONFIG;
*/
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
} TURBO_RATIO;

/*
typedef	struct
{
	unsigned long long
		FastStrings	:  1-0,
		ReservedBits1	:  3-1,
		TCC		:  4-3,
		ReservedBits2	:  7-4,
		PerfMonitoring	:  8-7,
		ReservedBits3	: 11-8,
		BTS		: 12-11,
		PEBS		: 13-12,
		TM2_Enable	: 14-13,
		ReservedBits4	: 16-14,
		EIST		: 17-16,
		ReservedBits5	: 18-17,
		FSM		: 19-18,
		ReservedBits6	: 22-19,
		CPUID_MaxVal	: 23-22,
		xTPR		: 24-23,
		ReservedBits7	: 34-24,
		XD_Bit		: 35-34,
		ReservedBits8	: 37-35,
		DCU_Prefetcher	: 38-37,
		Turbo_IDA	: 39-38,
		IP_Prefetcher	: 40-39,
		ReservedBits9	: 64-40;
} MISC_PROC_FEATURES;

typedef struct
{
	unsigned long long
		Type		:  8-0,
		ReservedBits1	: 10-8,
		FixeRange	: 11-10,
		Enable		: 12-11,
		ReservedBits2	: 64-12;
} MTRR_DEF_TYPE;
*/
typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_PMC0		:  1-0,
		EN_PMC1		:  2-1,
		EN_PMC2		:  3-2,
		EN_PMC3		:  4-3,
		EN_PMCn		: 32-4,
		EN_FIXED_CTR0	: 33-32,
		EN_FIXED_CTR1	: 34-33,
		EN_FIXED_CTR2	: 35-34,
		ReservedBits2	: 64-35;
	};
} GLOBAL_PERF_COUNTER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN0_OS		:  1-0,
		EN0_Usr		:  2-1,
		AnyThread_EN0	:  3-2,
		EN0_PMI		:  4-3,
		EN1_OS		:  5-4,
		EN1_Usr		:  6-5,
		AnyThread_EN1	:  7-6,
		EN1_PMI		:  8-7,
		EN2_OS		:  9-8,
		EN2_Usr		: 10-9,
		AnyThread_EN2	: 11-10,
		EN2_PMI		: 12-11,
		ReservedBits	: 64-12;
	};
} FIXED_PERF_COUNTER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Overflow_PMC0	:  1-0,
		Overflow_PMC1	:  2-1,
		Overflow_PMC2	:  3-2,
		Overflow_PMC3	:  4-3,
		Overflow_PMCn	: 32-4,
		Overflow_CTR0	: 33-32,
		Overflow_CTR1	: 34-33,
		Overflow_CTR2	: 35-34,
		ReservedBits2	: 61-35,
		Overflow_UNC	: 62-61,
		Overflow_Buf	: 63-62,
		Ovf_CondChg	: 64-63;
	};
} GLOBAL_PERF_STATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Clear_Ovf_PMC0	:  1-0,
		Clear_Ovf_PMC1	:  2-1,
		Clear_Ovf_PMC2	:  3-2,
		Clear_Ovf_PMC3	:  4-3,
		Clear_Ovf_PMCn	: 32-2,
		Clear_Ovf_CTR0 	: 33-32,
		Clear_Ovf_CTR1	: 34-33,
		Clear_Ovf_CTR2	: 35-34,
		ReservedBits2	: 61-35,
		Clear_Ovf_UNC	: 62-61,
		Clear_Ovf_Buf	: 63-62,
		Clear_CondChg	: 64-63;
	};
} GLOBAL_PERF_OVF_CTRL;

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
		PowerLimit      : 11-10,
		PowerLimitLog   : 12-11,
		ReservedBits1   : 16-12,
		DTS             : 23-16,
		ReservedBits2   : 27-23,
		Resolution      : 31-27,
		ReadingValid    : 32-31,
		ReservedBits3   : 64-32;
	};
} THERM_STATUS;


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
		ReservedBits1	:  8-0,
		BSP		:  9-8,
		ReservedBits2	: 10-9,
		EXTD		: 11-10,
		EN		: 12-11,
		Addr		: 64-12;
	};
} LOCAL_APIC;

#define	LEVEL_INVALID	0
#define	LEVEL_THREAD	1
#define	LEVEL_CORE	2

typedef	struct {
	union {
		struct
		{
			unsigned int
			SHRbits	:  5-0,
			Unused1	: 32-5;
		};
		unsigned int Register;
	} AX;
	union {
		struct
		{
			unsigned int
			Threads	: 16-0,
			Unused1	: 32-16;
		};
		unsigned int Register;
	} BX;
	union {
		struct
		{
			unsigned int
			Level	:  8-0,
			Type	: 16-8,
			Unused1 : 32-16;
		};
		unsigned int Register;
	} CX;
	union {
		struct
		{
			unsigned int
			x2ApicID: 32-0;
		};
		unsigned int Register;
	} DX;
} CPUID_TOPOLOGY_LEAF;

#define	CACHE_MAX_LEVEL	3

typedef	struct
{
	LOCAL_APIC	Base;
	signed int	ApicID,
			CoreID,
			ThreadID;
	struct
	{
		union
		{
			struct
			{
				unsigned int
				Linez:	12-0,
				Parts:	22-12,
				Ways:	32-22;
			};
			unsigned int Register;
		};
		unsigned int	Sets,
				Size;
	} Cache[CACHE_MAX_LEVEL];
} TOPOLOGY;

enum { INIT, END, START, STOP };

typedef struct
{
	volatile struct
	{	// Cache line size aligned structure.
		unsigned long long	V,
					_pad64[7];
	} Sync;

	struct
	{
		unsigned long long 	INST;
		struct
		{
			unsigned long long
					UCC,
					URC;
			}		C0;
			unsigned long long
					C3,
					C6,
					C7,
					TSC;

		unsigned long long	C1;
	} Counter[2];

	struct
	{
		unsigned long long
					INST;
		struct
		{
		unsigned long long
					UCC,
					URC;
			}		C0;
		unsigned long long
					C3,
					C6,
					C7,
					TSC,
					C1;
	} Delta;

	THERM_STATUS			ThermStat;
	TJMAX				TjMax;

	struct SAVEAREA
	{
		GLOBAL_PERF_COUNTER	GlobalPerfCounter;
		FIXED_PERF_COUNTER	FixedPerfCounter;
	} SaveArea;

	TOPOLOGY			T;

	unsigned int			Bind,
					OffLine;


	struct task_struct		*TID;
} CORE;

typedef	struct
{
	struct	SIGNATURE	Signature;
		void		(*Arch_Controller)(unsigned int stage);
		char		*Architecture;
} ARCH;

typedef struct
{
	FEATURES		Features;

	unsigned int		msleep;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	unsigned char		Architecture[32];
	signed int		ArchID;
	unsigned int		Boost[1+1+8],
				PerCore;

	CLOCK			Clock;
} PROC;

typedef struct
{
	struct kmem_cache	*Cache;
	CORE			*Core[];
} KMEM;

extern void Arch_Genuine(unsigned int stage) ;
extern void Arch_Core2(unsigned int stage) ;
extern void Arch_Nehalem(unsigned int stage) ;
extern void Arch_SandyBridge(unsigned int stage) ;
