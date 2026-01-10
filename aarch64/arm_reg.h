/*
 * CoreFreq
 * Copyright (C) 2015-2026 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define CPUPWRCTLR_EL1		sys_reg(0b11, 0b000, 0b1111, 0b0010, 0b111)
#define ID_AA64ISAR2_EL1	sys_reg(0b11, 0b000, 0b0000, 0b0110, 0b010)
#define ID_AA64ISAR3_EL1	sys_reg(0b11, 0b000, 0b0000, 0b0110, 0b011)
#define ID_AA64ISAR4_EL1	sys_reg(0b11, 0b000, 0b0000, 0b0110, 0b100)
#define ID_AA64MMFR2_EL1	sys_reg(0b11, 0b000, 0b0000, 0b0111, 0b010)
#define ID_AA64MMFR3_EL1	sys_reg(0b11, 0b000, 0b0000, 0b0111, 0b011)
#define ID_AA64SMFR0_EL1	sys_reg(0b11, 0b000, 0b0000, 0b0100, 0b101)
#define ID_AA64ZFR0_EL1 	sys_reg(0b11, 0b000, 0b0000, 0b0100, 0b100)
#define ID_AA64PFR2_EL1 	sys_reg(0b11, 0b000, 0b0000, 0b0100, 0b010)
#define SCTLR2_EL1		sys_reg(0b11, 0b000, 0b0001, 0b0000, 0b011)
#define MRS_SSBS2		sys_reg(0b11, 0b011, 0b0100, 0b0010, 0b110)
#define MRS_PAN 		sys_reg(0b11, 0b000, 0b0100, 0b0010, 0b011)
#define MRS_UAO 		sys_reg(0b11, 0b000, 0b0100, 0b0010, 0b100)
#define MRS_DIT 		sys_reg(0b11, 0b011, 0b0100, 0b0010, 0b101)
#define MRS_TCO 		sys_reg(0b11, 0b011, 0b0100, 0b0010, 0b111)
#define MRS_ALLINT		sys_reg(0b11, 0b000, 0b0100, 0b0011, 0b000)
#define MRS_PM			sys_reg(0b11, 0b000, 0b0100, 0b0011, 0b001)
#define MRS_SVCR		sys_reg(0b11, 0b011, 0b0100, 0b0010, 0b010)
#define MRS_FPMR		sys_reg(0b11, 0b011, 0b0100, 0b0100, 0b010)
#define AMCGCR_EL0		sys_reg(0b11, 0b011, 0b1101, 0b0010, 0b010)
#define CLUSTERCFR_EL1		sys_reg(0b11, 0b000, 0b1111, 0b0011, 0b000)
#define CLUSTERIDR_EL1		sys_reg(0b11, 0b000, 0b1111, 0b0011, 0b001)

typedef union
{
	unsigned long long	value;	/* CPU0:0x412fd050 ; CPU4:0x414fd0b0 */
	struct
	{
		unsigned long long
		Revision	:  4-0,
		PartNum 	: 16-4,
		Architecture	: 20-16, /* 0b1111=by CPUID scheme	*/
		Variant 	: 24-20,
		Implementer	: 32-24,
		RES0		: 64-32;
	};
} MIDR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Aff0		:  8-0,  /*	Thread ID		*/
		Aff1		: 16-8,  /*	Core ID: CPUID[12-8] L1 */
		Aff2		: 24-16, /*	Cluster ID - Level2	*/
		MT		: 25-24, /*	Multithreading		*/
		UNK		: 30-25,
		U		: 31-30, /*	0=Uniprocessor		*/
		RES1		: 32-31,
		Aff3		: 40-32, /*	Cluster ID - Level3	*/
		RES0		: 64-40;
	};
} MPIDR;
/*
CPU:MPIDR
  0:0x81000000
  1:0x81000100
  2:0x81000200
  3:0x81000300
  4:0x81000400
  5:0x81000500
  6:0x81000600
  7:0x81000700
*/

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Revision	: 32-0,
		RES0		: 64-32;
	};
} REVIDR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EnableAll	:  1-0,
		ResetEvent	:  2-1,
		ResetCycle	:  3-2,
		ClockDiv	:  4-3,  /* 1:PMCCNTR counts once / 64 cycles */
		ExportEvent	:  5-4,
		DisableCycle	:  6-5,  /* 1:PMCCNTR is disabled	*/
		EnableLongCycle :  7-6,
		EnableLongEvent :  8-7,
		RES0		:  9-8,
		FZO		: 10-9,  /* 1:Freeze-on-overflow	*/
		RES1		: 11-10,
		NumEvtCtrs	: 16-11,
		IDcode		: 24-16,
		Implementer	: 32-24,
		Freeze_On_SPE	: 33-32,
		RES2		: 64-33;
	};
} PMCR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		SEL		:  5-0,
		RES0		: 64-5;
	};
} PMSELR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CFILTR_EVTYPER	: 64-0;
	};
} PMXEVTYPER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		RES0		: 20-0,
		RLH		: 21-20,
		RLU		: 22-21,
		RLK		: 23-22,
		T		: 24-23,
		SH		: 25-24,
		RES1		: 26-25,
		M		: 27-26,
		NSH		: 28-27,
		NSU		: 29-28,
		NSK		: 30-29,
		U		: 31-30,
		P		: 32-31,
		RES2		: 56-32,
		VS		: 58-56,
		RES3		: 64-58;
	};
} PMCCFILTR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EvtCount	: 16-0,
		RES0		: 20-16,
		RLH		: 21-20,
		RLU		: 22-21,
		RLK		: 23-22,
		T		: 24-23,
		SH		: 25-24,
		MT		: 26-25,
		M		: 27-26,
		NSH		: 28-27,
		NSU		: 29-28,
		NSK		: 30-29,
		U		: 31-30,
		P		: 32-31,
		TH		: 44-32,
		RES1		: 54-44,
		TLC		: 56-54,
		VS		: 58-56,
		SYNC		: 59-58,
		RES2		: 60-59,
		TE		: 61-60,
		TC		: 64-61;
	};
} PMEVTYPER;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Pm		: 31-0,
		C		: 32-31,
		F0		: 33-32,
		RES0		: 64-33;
	};
} PMCNTENSET;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Pm		: 31-0,
		C		: 32-31,
		F0		: 33-32,
		RES0		: 64-33;
	};
} PMCNTENCLR;


typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN		:  1-0,
		SW		:  2-1,
		CR		:  3-2,
		ER		:  4-3,
		UEN		:  5-4,
		IR		:  6-5,
		TID		:  7-6,
		RES0		: 64-7;
	};
} PMUSERENR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ClockFreq_Hz	: 32-0,
		RES0		: 64-32;
	};
} CNTFRQ;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PhysicalCount	: 64-0;
	};
} CNTPCT;

typedef union
{
	unsigned long long	value;	/* SMT=0x0000000000000000	*/
	struct
	{
		unsigned long long
		CORE_PWRDN_EN	:  1-0,
		RES0		:  4-1,
		WFI_RET_CTRL	:  7-4,
		WFE_RET_CTRL	: 10-7,
		SIMD_RET_CTRL	: 13-10,
		RES1		: 32-13,
		RES2		: 64-32;
	};
} CPUPWRCTLR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		LineSz		:  3-0,
		Assoc		: 13-3,
		Set		: 28-13,
		WrAlloc 	: 29-28,
		RdAlloc 	: 30-29,
		WrBack		: 31-30,
		WrThrough	: 32-31,
		RES0		: 63-32,
		FEAT_CCIDX	: 64-63;
	};
	struct
	{
		unsigned long long
		LineSize	:  3-0,
		Associativity	: 24-3,
		RES1		: 32-24,
		NumSets 	: 56-32,
		RES2		: 64-56;
	};
} CCSIDR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		InD		:  1-0,
		Level		:  4-1,
		TnD		:  5-4,
		RES0		: 64-5;
	};
} CSSELR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Ctype1		:  3-0,
		Ctype2		:  6-3,
		Ctype3		:  9-6,
		RES0		: 21-9,
		LoUIS		: 24-21,
		LoC		: 27-24,
		LoUU		: 30-27,
		ICB		: 33-30,
		RES1		: 64-33;
	};
} CLIDR;

typedef union
{	/*	CPU0:0x00000000003033ff ; CPU4:0x0000000000300000	*/
	unsigned long long	value;
	struct
	{
		unsigned long long
		RES0		: 16-0,
		ZEN		: 18-16,
		RES1		: 20-18,
		FPEN		: 22-20,
		RES2		: 24-22,
		SMEN		: 26-24,
		RES3		: 28-26,
		TTA		: 29-28,
		E0POE		: 30-29,
		RES4		: 64-30;
	};
} CPACR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CG0NC		:  8-0,
		CG1NC		: 16-8,
		RES0		: 64-16;
	};
} AMCGCR;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		DebugVer	:  4-0,
		TraceVer	:  8-4,
		PMUVer		: 12-8,
		BRPs		: 16-12,
		PMSS		: 20-16,
		WRPs		: 24-20,
		SEBEP		: 28-24,
		CTX_CMPs	: 32-28,
		PMSVer		: 36-32,
		DoubleLock	: 40-36,
		TraceFilt	: 44-40,
		TraceBuffer	: 48-40,
		MTPMU		: 52-48,
		BRBE		: 56-52,
		ExtTrcBuff	: 60-56,
		HPMN0		: 64-60;
	};
} AA64DFR0;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		SYSPMUID	:  8-0,
		BRPs		: 16-8,
		WRPs		: 24-16,
		CTX_CMPs	: 32-24,
		SPMU		: 36-32,
		PMICNTR 	: 40-36,
		ABLE		: 44-40,
		ITE		: 48-44,
		EBEP		: 52-48,
		DPFZS		: 56-52,
		ABL_CMPs	: 64-56;
	};
} AA64DFR1;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		STEP		:  4-0,
		BWE		:  8-4,
		RES0		: 64-8;
	};
} AA64DFR2;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		RES0		:  4-0,
		AES		:  8-4,
		SHA1		: 12-8,
		SHA2		: 16-12,
		CRC32		: 20-16,
		Atomic		: 24-20,
		TME		: 28-24,
		RDM		: 32-28,
		SHA3		: 36-32,
		SM3		: 40-36,
		SM4		: 44-40,
		DP		: 48-44,
		FHM		: 52-48,
		TS		: 56-52,
		TLB		: 60-56,
		RNDR		: 64-60;
	};
} AA64ISAR0;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		DPB		:  4-0,
		APA		:  8-4,
		API		: 12-8,
		JSCVT		: 16-12,
		FCMA		: 20-16,
		LRCPC		: 24-20,
		GPA		: 28-24,
		GPI		: 32-28,
		FRINTTS 	: 36-32,
		SB		: 40-36,
		SPECRES 	: 44-40,
		BF16		: 48-44,
		DGH		: 52-48,
		I8MM		: 56-52,
		XS		: 60-56,
		LS64		: 64-60;
	};
} AA64ISAR1;

typedef union
{
	unsigned long long	value;	/* CPU0:0x00000000 ; CPU4:0x00000000 */
	struct
	{
		unsigned long long
		WFxT		:  4-0,
		RPRES		:  8-4,
		GPA3		: 12-8,
		APA3		: 16-12,
		MOPS		: 20-16,
		BC		: 24-20,
		PAC_frac	: 28-24,
		CLRBHB		: 32-28,
		SYSREG_128	: 36-32,
		SYSINSTR_128	: 40-36,
		PRFMSLC 	: 44-40,
		PCDPHINT	: 48-44,
		RPRFM		: 52-48,
		CSSC		: 56-52,
		LUT		: 60-56,
		ATS1A		: 64-60;
	};
} AA64ISAR2;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CPA		:  4-0,
		FAMINMAX	:  8-4,
		TLBIW		: 12-8,
		PACM		: 16-12,
		LSFE		: 20-16,
		OCCMO		: 24-20,
		LSUI		: 28-24,
		FPRCVT		: 32-28,
		RES0		: 64-32;
	};
} AA64ISAR3;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PARange 	:  4-0,
		ASID_Bits	:  8-4,
		BigEnd		: 12-8,
		SNSMem		: 16-12,
		BigEnd_EL0	: 20-16,
		TGran16 	: 24-20,
		TGran64 	: 28-24,
		TGran4		: 32-28,
		TGran16_2	: 36-32,
		TGran64_2	: 40-36,
		TGran4_2	: 44-40,
		ExS		: 48-44,
		RES0		: 56-48,
		FGT		: 60-56,
		ECV		: 64-60;
	};
} AA64MMFR0;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		HAFDBS		:  4-0,
		VMID_Bits	:  8-4,
		VH		: 12-8,
		HPDS		: 16-12,
		LO		: 20-16,
		PAN		: 24-20,
		SpecSEI		: 28-24,
		XNX		: 32-28,
		TWED		: 36-32,
		ETS		: 40-36,
		HCX		: 44-40,
		AFP		: 48-44,
		nTLBPA		: 52-48,
		TIDCP1		: 56-52,
		CMOW		: 60-56,
		ECBHB		: 64-60;
	};
} AA64MMFR1;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CnP		:  4-0,
		UAO		:  8-4,
		LSM		: 12-8,
		IESB		: 16-12,
		VARange 	: 20-16,
		CCIDX		: 24-20,
		NV		: 28-24,
		ST		: 32-28,
		AT		: 36-32,
		IDS		: 40-36,
		FWB		: 44-40,
		RES0		: 48-44,
		TTL		: 52-48,
		BBM		: 56-52,
		EVT		: 60-56,
		E0PD		: 64-60;
	};
} AA64MMFR2;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		TCRX		:  4-0,
		SCTLRX		:  8-4,
		S1PIE		: 12-8,
		S2PIE		: 16-12,
		S1POE		: 20-16,
		S2POE		: 24-20,
		AIE		: 28-24,
		MEC		: 32-28,
		D128		: 36-32,
		D128_2		: 40-36,
		SNERR		: 44-40,
		ANERR		: 48-44,
		RES0		: 52-48,
		SDERR		: 56-52,
		ADERR		: 60-56,
		Spec_FPACC	: 64-60;
	};
} AA64MMFR3;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		PoPS		:  4-0,
		EIESB		:  8-4,
		ASID2		: 12-8,
		HACDBS		: 16-12,
		FGWTE3		: 20-16,
		NV_frac 	: 24-20,
		E2H0		: 28-24,
		RMEGDI		: 32-28,
		RES0		: 36-32,
		E3DSE		: 40-36,
		RES1		: 44-40,
		SRMASK		: 48-44,
		RES2		: 64-48;
	};
} AA64MMFR4;

typedef union
{				/* CPU0:0x11112222 ; CPU4:0x1100000011111112 */
	unsigned long long	value;
	struct
	{
		unsigned long long
		EL0		:  4-0,  /*0b0001=AArch64;0b0010=AArch{32,64}*/
		EL1		:  8-4,
		EL2		: 12-8,
		EL3		: 16-12,
		FP		: 20-16,
		AdvSIMD 	: 24-20,
		GIC		: 28-24,
		RAS		: 32-28,
		SVE		: 36-32,
		SEL2		: 40-36,
		MPAM		: 44-40,
		AMU		: 48-44,
		DIT		: 52-48,
		RME		: 56-52,
		CSV2		: 60-56,
		CSV3		: 64-60;
	};
} AA64PFR0;
/*
CPU:AA64PFR0
 1:0x0000000011112222
 0:0x0000000011112222
 2:0x0000000011112222
 3:0x0000000011112222
 4:0x1100000011111112
 5:0x1100000011111112
 6:0x1100000011111112
 7:0x1100000011111112
*/
typedef union
{
	unsigned long long	value;	/*	Pkg:0x00000010		*/
	struct
	{
		unsigned long long
		BT		:  4-0,
		SSBS		:  8-4,
		MTE		: 12-8,
		RAS_frac	: 16-12,
		MPAM_frac	: 20-16,
		SME		: 28-24,
		RNDR_trap	: 32-28,
		CSV2_frac	: 36-32,
		NMI		: 40-36,
		MTE_frac	: 44-40,
		GCS		: 48-44,
		THE		: 52-48,
		MTEX		: 56-52,
		DF2		: 60-56,
		PFAR		: 64-60;
	};
} AA64PFR1;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		MTEPERM 	:  4-0,
		MTESTOREONLY	:  8-4,
		MTEFAR		: 12-8,
		RES0		: 16-12,
		UINJ		: 20-16,
		RES1		: 32-20,
		FPMR		: 36-32,
		RES2		: 64-36;
	};
} AA64PFR2;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		RES0		: 28-0,
		SF8DP2		: 29-28,
		SF8DP4		: 30-29,
		SF8FMA		: 31-30,
		RES1		: 32-31,
		F32F32		: 33-32,
		BI32I32 	: 34-33,
		B16F32		: 35-34,
		F16F32		: 36-35,
		I8I32		: 40-36,
		F8F32		: 41-40,
		F8F16		: 42-41,
		F16F16		: 43-42,
		B16B16		: 44-43,
		I16I32		: 48-44,
		F64F64		: 49-48,
		RES2		: 52-49,
		I16I64		: 56-52,
		SMEver		: 60-56,
		LUTv2		: 61-60,
		RES3		: 63-61,
		FA64		: 64-63;
	};
} AA64SMFR0;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		SVE_Ver 	:  4-0,
		SVE_AES 	:  8-4,
		RES0		: 16-8,
		BitPerm 	: 20-16,
		SVE_BF16	: 24-20,
		B16B16		: 28-24,
		RES1		: 32-28,
		SVE_SHA3	: 36-32,
		RES2		: 40-36,
		SVE_SM4 	: 44-40,
		SVE_I8MM	: 48-44,
		RES3		: 52-48,
		SVE_F32MM	: 56-52,
		SVE_F64MM	: 60-56,
		RES4		: 64-60;
	};
} AA64ZFR0;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		RES0		: 12-0,
		SSBS		: 13-12,
		RES1		: 32-13,
		RES2		: 64-32;
	};
} SSBS2;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		SIMDReg 	:  4-0,
		FPSP		:  8-4,
		FPDP		: 12-8,
		FPTrap		: 16-12,
		FPDivide	: 20-16,
		FPSqrt		: 24-20,
		FPShVec 	: 28-24,
		FPRound 	: 32-28,
		RES0		: 64-32;
	};
} MVFR0;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		FPFtZ		:  4-0,
		FPDNaN		:  8-4,
		SIMDLS		: 12-8,
		SIMDInt 	: 16-12,
		SIMDSP		: 20-16,
		SIMDHP		: 24-20,
		FPHP		: 28-24,
		SIMDFMAC 	: 32-28,
		RES0		: 64-32;
	};
} MVFR1;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		SIMDMisc	:  4-0,
		FPMisc		:  8-4,
		RES0		: 32-8,
		RES1		: 64-32;
	};
} MVFR2;

typedef union
{	/* R82; A55; A75; A76; A76AE; A77; A78; A78AE; A78C; X1; X1C; N3; V1 */
	unsigned long long	value;	/*	Pkg:0x0000000007bfda77	*/
	struct
	{
		unsigned long long
		NUMCORE 	:  4-0,
		NUMPE		:  9-4,
		L3		: 10-9,
		WRLAT		: 12-10,
		RDLAT		: 13-12,
		RDSLC		: 14-13,
		ECC		: 15-14,
		NUMMAS		: 17-15,
		MAS		: 18-17,
		RAZ1		: 19-18,
		ACPW		: 20-19,
		ACP		: 21-20,
		RAZ2		: 22-21,
		PPW		: 23-22,
		PP		: 24-23,
		RAZ3		: 25-24,
		TRSH		: 29-25,
		TRSV		: 33-29,
		RAZ4		: 51-33,
		L3SLC		: 54-51,
		RAZ5		: 55-24,
		SFIDX		: 59-55,
		SFWAY		: 61-59,
		NODES		: 64-61;
	} IMP;
	struct
	{
		unsigned long long
		NUMCORE 	:  3-0,
		RAZ1		:  4-3,
		L3		:  5-4,
		WRLAT		:  6-5,
		RDLAT		:  7-6,
		RDSLC		:  8-7,
		ECC		:  9-8,
		MAS		: 11-9,
		ACP		: 12-11,
		PP		: 13-12,
		MAS_Ext 	: 14-13,
		CPUSLC		: 22-14,
		RAZ2		: 23-22,
		WRDLY		: 24-23,
		NUMPE		: 28-24,
		RAZ3		: 30-28,
		SafetyMode	: 32-30,
		RSVD		: 64-32;
	} DSU;
} CLUSTERCFR;

typedef union
{
	unsigned long long	value;	/*	Pkg:0x0000000000000041	*/
	struct
	{
		unsigned long long
		Revision	:  4-0,
		Variant 	:  8-4,
		RAZ		: 32-8,
		RSVD		: 64-32;
	};
} CLUSTERIDR;

typedef union
{
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
} HWP_CAPABILITIES;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_Guarantee_Chg:  1-0,
		EN_Excursion_Min:  2-1,
		EN_Highest_Chg	:  3-2,
		EN_PECI_OVERRIDE:  4-3,
		ReservedBits	: 64-4;  /* **Must be zero**		*/
	};
} HWP_INTERRUPT;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Minimum_Perf	:  8-0,
		Maximum_Perf	: 16-8,
		Desired_Perf	: 24-16,
		Energy_Pref	: 32-24,
		Activity_Window : 42-32,
		Package_Control : 43-42,
		ReservedBits	: 59-43,
		Act_Window_Valid: 60-59, /* Activity_Window Valid; Default=0 */
		EPP_Valid	: 61-60, /*1:[HWP_REQUEST];0:[HWP_REQUEST_PKG]*/
		Desired_Valid	: 62-61, /* -> Desired_Perf		*/
		Maximum_Valid	: 63-62, /* -> Maximum_Perf		*/
		Minimum_Valid	: 64-63; /* -> Minimum_Perf		*/
	};
} HWP_REQUEST;
