/*
 * CoreFreq
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

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


typedef	struct
{
	LOCAL_APIC	Base;
	int		ApicID,
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
