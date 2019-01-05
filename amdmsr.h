/*
 * CoreFreq
 * Copyright (C) 2015-2019 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#ifndef MSR_AMD_F17H_MPERF
	#define MSR_AMD_F17H_MPERF		0xc00000e7
#endif

#ifndef MSR_AMD_F17H_APERF
	#define MSR_AMD_F17H_APERF		0xc00000e8
#endif

#ifndef MSR_F17H_IRPERF
	#define MSR_AMD_F17H_IRPERF		0xc00000e9
#else
	#define MSR_AMD_F17H_IRPERF		MSR_F17H_IRPERF
#endif

#ifndef MSR_AMD_PERF_STATUS
	#define MSR_AMD_PERF_STATUS		0xc0010063
#endif

#ifndef MSR_AMD_PSTATE_DEF_BASE
	#define MSR_AMD_PSTATE_DEF_BASE 	0xc0010064
#endif

#ifndef MSR_AMD_COFVID_STATUS
	#define MSR_AMD_COFVID_STATUS		0xc0010071
#endif

#ifndef MSR_VM_CR
	#define MSR_VM_CR			0xc0010114
#endif

#ifndef MSR_SVM_LOCK_KEY
	#define MSR_SVM_LOCK_KEY		0xc0010118
#endif

#ifndef MSR_AMD_RAPL_POWER_UNIT
	#define MSR_AMD_RAPL_POWER_UNIT		0xc0010299
#endif

#ifndef MSR_AMD_PKG_ENERGY_STATUS
	#define MSR_AMD_PKG_ENERGY_STATUS	0xc001029b
#endif

#ifndef MSR_AMD_PP0_ENERGY_STATUS
	#define MSR_AMD_PP0_ENERGY_STATUS	0xc001029a
#endif

/* UNDOCUMENTED REGISTERS */
#ifndef MSR_AMD_PC6_F17_STATUS
	#define MSR_AMD_PC6_F17_STATUS		0xc0010292
#endif

#ifndef MSR_AMD_PSTATE_F17_BOOST
	#define MSR_AMD_PSTATE_F17_BOOST	0xc0010293
#endif

#ifndef MSR_AMD_CC6_F17_STATUS
	#define MSR_AMD_CC6_F17_STATUS		0xc0010296
#endif

const struct {
	unsigned int	MCF,
			PCF[5];
} VCO[0b1000] = {
/* FID */
/* 000000b */	{ 8, { 0,  0, 16, 17, 18}},
/* 000001b */	{ 9, {16, 17, 18, 19, 20}},
/* 000010b */	{10, {18, 19, 20, 21, 22}},
/* 000011b */	{11, {20, 21, 22, 23, 24}},
/* 000100b */	{12, {22, 23, 24, 25, 26}},
/* 000101b */	{13, {24, 25, 26, 27, 28}},
/* 000110b */	{14, {26, 27, 28, 29, 30}},
/* 000111b */	{15, {28, 29, 30, 31, 32}},
};

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	SmmLock		:  1-0,  /* SMM Configuration Lock.		*/
	Reserved1	:  3-1,
	TlbCacheDis	:  4-3,  /* Cacheable Memory Disable.		*/
	INVDWBINVD	:  5-4,  /* INVD to WBINVD Conversion.		*/
	Reserved2	:  7-5,
	AllowFerrOnNe	:  8-7,  /* Allow FERR on NE..			*/
	IgnneEm		:  9-8,  /* IGNNE port emulation enable.	*/
	MonMwaitDis	: 10-9,  /* 1=MONITOR & MWAIT opcodes become invalid. */
	MonMwaitUserEn	: 11-10, /* MONITOR/MWAIT user mode enable. 0=pl0 only*/
	Reserved3	: 12-11,
	HltXSpCycEn	: 13-12, /* halt-exit special bus cycle enable. */
	SmiSpCycDis	: 14-13, /* SMI special bus cycle disable.	*/
	RsmSpCycDis	: 15-14, /* RSM special bus cycle disable.	*/
	Reserved4	: 17-15,
	Wrap32Dis	: 18-17, /* 32-bit address wrap disable.	*/
	McStatusWrEn	: 19-18, /* Machine check status write enable.	*/
	Reserved5	: 20-19,
	IoCfgGpFault	: 21-20, /* IO-space configuration causes a GP fault. */
	Reserved6	: 23-21,
	ForceRdWrSzPrb	: 24-23, /* Force probes for RdSized and WrSized. */
	TscFreqSel	: 25-24, /* 1=The TSC increments at the P0 frequency. */
	CpbDis		: 26-25, /* 1=Core performance boost disable.	*/
	EffFreqCntMwait : 27-26, /* Effective frequency counting during mwait.*/
	/* Family 15h */
	EffFreqROLock	: 28-27, /* Read-only effective frequency counter lock*/
	SmuLock		: 29-28,
	CSEnable	: 30-29, /* Connected standby enable.		*/
	Reserved7	: 32-30,
	Reserved	: 64-32;
    } Family_12h;
    struct
    {
	unsigned long long
	SmmLock		:  1-0,
	Reserved1	:  3-1,
	TlbCacheDis	:  4-3,
	INVDWBINVD	:  5-4,
	Reserved2	:  7-5,
	AllowFerrOnNe	:  8-7,
	IgnneEm		:  9-8,
	MonMwaitDis	: 10-9,
	MonMwaitUserEn	: 11-10,
	Reserved3	: 13-11,
	SmiSpCycDis	: 14-13,
	RsmSpCycDis	: 15-14,
	Reserved4	: 17-15,
	Wrap32Dis	: 18-17,
	McStatusWrEn	: 19-18,
	Reserved5	: 20-19,
	IoCfgGpFault	: 21-20,
	LockTscToCurrP0 : 22-21,/* RW:lock the TSC to the current P0 frequency*/
	Reserved6	: 24-22,
	TscFreqSel	: 25-24,
	CpbDis		: 26-25,
	EffFreqCntMwait : 27-26,
	EffFreqROLock	: 28-27,
	Reserved7	: 29-28,
	CSEnable	: 30-29,
	IRPerfEn	: 31-30, /* RW: enable instructions retired counter */
	Reserved	: 64-31;
    } Family_17h;
    struct
    {
	unsigned long long
	SmmLock		:  1-0,
	SLOWFENCE	:  2-1,  /* Slow SFENCE Enable.			*/
	Reserved1	:  3-2,
	TlbCacheDis	:  4-3,
	INVDWBINVD	:  5-4,
	Reserved2	:  6-5,
	FFDIS		:  7-6,  /* TLB Flush Filter Disable.		*/
	DISLOCK		:  8-7,  /* Disable x86 LOCK prefix functionality. */
	IgnneEm		:  9-8,
	Reserved3	: 12-9,
	HltXSpCycEn	: 13-12,
	SmiSpCycDis	: 14-13,
	RsmSpCycDis	: 15-14,
	SSEDIS		: 16-15, /* SSE Instructions Disable.		*/
	Reserved4	: 17-16,
	Wrap32Dis	: 18-17,
	McStatusWrEn	: 19-18,
	Reserved5	: 24-19,
	StartFID	: 30-24, /* Startup FID Status.			*/
	Reserved6	: 32-30,
	Reserved	: 64-32;
    } Family_0Fh;
} HWCR;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	NewFID		:  6-0,
	Reserved1	:  8-6,
	NewVID		: 14-8,
	Reserved2	: 16-14,
	InitFidVid	: 17-16,	/* Initiate FID/VID Change	*/
	Reserved3	: 32-17,
	StpGntTOCnt	: 52-32,	/* Stop Grant Time-Out Count	*/
	Reserved4	: 64-52;
    };
} FIDVID_CONTROL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CurrFID		:  6-0,  /* Current FID				*/
	Reserved1	:  8-6,
	StartFID	: 14-8,  /* Startup FID				*/
	Reserved2	: 16-14,
	MaxFID		: 22-16, /* Max FID				*/
	Reserved3	: 24-22,
	MaxRampVID	: 30-24, /* Max Ramp VID			*/
	Reserved4	: 31-30,
	FidVidPending	: 32-31, /* 0b when the FID/VID change has completed.*/
	CurrVID		: 38-32, /* Current VID				*/
	Reserved5	: 40-38,
	StartVID	: 46-40, /* Startup VID				*/
	Reserved6	: 48-46,
	MaxVID		: 54-48, /* Max VID				*/
	Reserved7	: 56-54,
	PstateStep	: 57-56, /* voltage reduction: 0b=25mV; 1b=50mV */
	AltVidOffset	: 60-57, /* [NA;-50;-100;-125;-150;-175;-200;-225]mV */
	Reserved8	: 61-60,
	IntPstateSup	: 62-61, /* 1b = Intermediate P-states is supported. */
	Reserved9	: 64-62;
    };
} FIDVID_STATUS;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSRC001_00[68:64] P-State [4:0]	*/
	CpuFid		:  6-0,  /* Core Frequency ID. RW: Value <= 2Fh */
	CpuDid		:  9-6,  /* Core Divisor ID. RW: 0h-4h divide by 1-16 */
	CpuVid		: 16-9,  /* Core Voltage ID. RW			*/
	Reserved1	: 22-16,
	NbDid		: 23-22, /* Northbridge Divisor ID. RW: 0-1 => 0-2 */
	Reserved2	: 25-23,
	NbVid		: 32-25, /* NB VID. RW:in MSRC001_0071[MaxVid,MinVid] */
	IddValue	: 40-32, /* Current Dissipation. RW:00-10b->1,10,100A */
	IddDiv		: 42-40, /* Current Dissipation Divisor. RW	*/
	Reserved3	: 63-42,
	PstateEn	: 64-63; /* Pstate enabled. RW			*/
    } Family_10h;
    struct
    {
	unsigned long long	 /* MSRC001_00[6B:64] P-State [7:0]	*/
	CpuDid		:  4-0,  /* Core Divisor ID. RW			*/
	CpuFid		:  9-4,  /* Core Frequency ID. RW		*/
	CpuVid		: 16-9,  /* Core Voltage ID. RW			*/
	Reserved1	: 32-16,
	IddValue	: 40-32, /* Current value field. RW		*/
	IddDiv		: 42-40, /* Current divisor field. RW		*/
	Reserved2	: 63-42,
	PstateEn	: 64-63; /* Pstate enabled. RW			*/
    } Family_12h;
    struct
    {
	unsigned long long	 /* MSRC001_00[6B:64] P-State [7:0]	*/
	CpuDidLSD	:  4-0,  /* Core Divisor ID least significant digit.RW*/
	CpuDidMSD	:  9-4,  /* Core Divisor ID most significant digit. RW*/
	CpuVid		: 16-9,  /* Core Voltage ID. RW			*/
	Reserved1	: 32-16,
	IddValue	: 40-32, /* Current value field. RW		*/
	IddDiv		: 42-40, /* Current divisor field. RW		*/
	Reserved2	: 63-42,
	PstateEn	: 64-63; /* Pstate enabled. RW			*/
    } Family_14h;
    struct
    {
	unsigned long long	 /* MSRC001_00[6B:64] P-state [7:0]	*/
	CpuFid		:  6-0,  /* Core Frequency ID. RW		*/
	CpuDid		:  9-6,  /* Core Divisor ID. RW:0h-4h divide by 1-16 */
	CpuVid		: 16-9,  /* Core Voltage ID. RW			*/
	CpuVid_bit	: 17-16,
	Reserved1	: 22-17,
	NbPstate	: 23-22, /* Northbr. P-state.MSRC001_0071[NbPstateDis]*/
	Reserved2	: 32-23,
	IddValue	: 40-32, /* Max Current Dissipation:00-10b->1,10,100A */
	IddDiv		: 42-40, /* Current Dissipation Divisor. RW	*/
	Reserved3	: 63-42,
	PstateEn	: 64-63; /* Pstate enabled. RW			*/
    } Family_15h;
    struct
    {
	unsigned long long	 /* MSRC001_0064 [P-state [7:0]]	*/
	CpuFid		:  8-0,  /* Core Frequency ID. RW: FFh-10h <Value>*25 */
	CpuDfsId	: 14-8,  /* Core Divisor ID. RW			*/
	CpuVid		: 22-14, /* Core Voltage ID. RW			*/
	IddValue	: 30-22, /* Current Dissipation in amps. RW	*/
	IddDiv		: 32-30, /* Current Dissipation Divisor. RW	*/
	Reserved	: 63-32,
	PstateEn	: 64-63; /* RW: Is this Pstate MSR valid ?	*/
    } Family_17h;
} PSTATEDEF;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSRC001_0063			*/
	Current		:  3-0,
	Reserved	: 64-3;
    };
} PSTATESTAT;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSRC001_0071 COFVID Status		*/
	CurCpuFid	:  6-0,
	CurCpuDid	:  9-6,
	CurCpuVid	: 16-9,
	CurPstate	: 19-16,
	Reserved1	: 20-19,
	CurCpuVid_bit	: 21-20,
	Reserved2	: 23-21,
	NbPstateDis	: 24-23,
	CurNbVid	: 32-24,
	StartupPstate	: 35-32,
	Reserved3	: 49-35,
	MaxCpuCof	: 55-49,
	Reserved4	: 56-55,
	CurPstateLimit	: 59-56,
	MaxNbCof	: 64-59;
    } Arch_COF;
    struct
    {
	unsigned long long	 /* MSRC001_0071 COFVID Status		*/
	CurCpuDidLSD	:  4-0,  /* Current Core Divisor ID. RO		*/
	CurCpuDidMSD	:  9-4,
	CurCpuVid	: 16-9,  /* Current Core Voltage ID. RO		*/
	CurPstate	: 19-16, /* Current P-state. RO			*/
	Reserved1	: 20-19,
	PstateInProgress: 21-20, /* RO: 1=Change, 0=No change		*/
	Reserved2	: 25-21,
	CurNbVid	: 32-25, /* Current Northbridge VID. RO		*/
	StartupPstate	: 35-32, /* Startup P-state Number. RO		*/
	MaxVid		: 42-35, /* Maximum Voltage ID. RO		*/
	MinVid		: 49-42, /* Minimum Voltage ID. RO		*/
	MainPllOpFidMax : 55-49, /* Main Pll Operating Frequency ID maximum.RO*/
	Reserved3	: 56-55,
	CurPstateLimit	: 59-56, /* Current P-state Limit. RO		*/
	Reserved4	: 64-59;
    } Arch_Pll;
} COFVID;

typedef union
{
	unsigned long long value; /* MSR C001_0055h			*/
    struct
    {
	unsigned long long
	IOMsgAddr	: 16-0,
	IOMsgData	: 24-16,
	IntrPndMsgDis	: 25-24,
	IntrPndMsg	: 26-25,
	IORd		: 27-26,
	SmiOnCmpHalt	: 28-27, /* SMI on Multi-core halt		*/
	C1eOnCmpHalt	: 29-28, /* C1E on Multi-core halt: Family 0Fh,10h,11h*/
	Reserved1	: 32-29,
	Reserved2	: 64-32;
    };
} INT_PENDING_MSG;

typedef union
{
	unsigned long long value; /* Per SMT: MSR C001_0114h (VM_CR)	*/
    struct
    {
	unsigned long long
	DPD		:  1-0,  /* Debug Port Disable. ReservedBits: F17h */
	InterceptInit	:  2-1,
	DisA20m		:  3-2,  /* Disable A20 Masking. ReservedBits: F17h */
	SVM_Lock	:  4-3,  /* 0=SvmeDisable is read-write, 1=read-only */
	SVME_Disable	:  5-4,  /* 0 = Msr::EFER[SVME] is RW, 1 = read-only */
	Reserved1	: 32-5,
	Reserved2	: 64-32;
    };
} VM_CR;	/* Family: 17h, 16h, 15h, 14h, 12h, 11h, 10h, 0Fh	*/

typedef union
{
	unsigned long long value; /* Per SMT: MSR C001_0118h		*/
    struct
    {
	unsigned long long
	SvmLockKey	: 64-0; /* Write if (Core::X86::Msr::VM_CR[Lock] == 0)*/
    };
} SVM_LOCK_KEY;	/* Family: 17h, 16h, 15h, 14h, 12h, 11h, 10h		*/

typedef struct
{
	unsigned int
	Reserved1	:  1-0,
	SensorTrip	:  2-1,  /*1 if temp. sensor trip occurs & was enabled*/
	SensorCoreSelect:  3-2,  /*0b:CPU1 Therm Sensor. 1b:CPU0 Therm Sensor */
	Sensor0Trip	:  4-3,  /*1 if trip @ CPU0 (single), or @ CPU1 (dual)*/
	Sensor1Trip	:  5-4,  /*1 if sensor trip occurs @ CPU0 (dual core) */
	SensorTripEnable:  6-5,  /*THERMTRIP High event causes a PLL shutdown */
	SelectSensorCPU	:  7-6,  /*0b:CPU[0,1] Sensor 0. 1b:CPU[0,1] Sensor 1 */
	Reserved2	:  8-7,
	DiodeOffset	: 14-8,  /*offset should be added to the external temp*/
	Reserved3	: 16-14,
	CurrentTemp	: 24-16, /* 00h = -49C , 01h = -48C ... ffh = 206C */
	TjOffset	: 29-24, /* Tcontrol = CurTmp - TjOffset * 2 - 49  */
	Reserved4	: 31-29,
	SwThermTrip	: 32-31; /* diagnostic bit, for testing purposes only.*/
} THERMTRIP_STATUS;

typedef union {
	unsigned int		value;
	struct {
		unsigned int
		CSEnable	:  1-0,
		Spare		:  2-1,
		MemTestFailed	:  3-2,
		ReservedBits1	:  5-3,
		BaseAddrLo	: 14-5,
		ReservedBits2	: 19-14,
		BaseAddrHi	: 29-19,
		ReservedBits3	: 32-29;
	};
} AMD_0F_DRAM_CS_BASE_ADDR;

typedef union
{	/* Function: 2 - Offset: 80h					*/
	unsigned int		value;
	struct {
		unsigned int
			CS10	:  4-0,
			CS32	:  8-4,
			CS54	: 12-8,
			CS76	: 16-12,
		ReservedBits	: 32-16;
	};
} AMD_0F_DRAM_CS_MAPPING;

typedef union
{	/* Function: 2 - Offset: 88h					*/
	unsigned int		value;
	struct {
		unsigned int
		tCL		:  3-0,
		ReservedBits1	:  4-3,
		tRCD		:  6-4,
		ReservedBits2	:  8-6,
		tRP		: 10-8,
		ReservedBits3	: 11-10,
		tRTPr		: 12-11,
		tRAS		: 16-12,
		tRC		: 20-16,
		tWR		: 22-20,
		tRRD		: 24-22,
		MemClkDis	: 32-24;
	};
} AMD_0F_DRAM_TIMING_LOW;

typedef union
{	/* Function: 2 - Offset: 90h					*/
	unsigned int		value;
	struct {
		unsigned int
		InitializeDRAM	:  1-0,
		ExitSelfRefresh :  2-1,
		ReservedBits1	:  4-2,
		DRAM_Term	:  6-4,
		ReservedBits2	:  7-6,
		DRAM_DrvWeak	:  8-7,
		Parity_Enable	:  9-8,
		SelfRefRateEn	: 10-9,
		BurstLength32	: 11-10,  /* 0b: 64-Byte, 1b: 32-Byte	*/
		Width128	: 12-11,  /* 0b: 64-bits, 1b: 128-bits	*/
		X4_DIMMS	: 16-12,
		UnbufferedDIMM	: 17-16,
		ReservedBits3	: 19-17,
		ECC_DIMM_Enable : 20-19,
		ReservedBits4	: 32-20;
	};
} AMD_0F_DRAM_CONFIG_LOW;

typedef union
{	/* Function: 2 - Offset: 94h					*/
	unsigned int		value;
	struct {
		unsigned int
		MemClkFreq	:  3-0,  /*000b:200,001b:266,010b:333,011b:400*/
		MemClkFreqValid :  4-3,
		MaxAsyncLatency :  8-4,
		ReservedBits1	: 12-8,
		ReadDQS_Enable	: 13-12,
		ReservedBits2	: 14-13,
		DisDRAMInterface: 15-14,
		PowerDown_Enable: 16-15,
		PowerDownMode	: 17-16,
		FourRankSODimm	: 18-17,
		FourRankRDimm	: 19-18,
		ReservedBits3	: 20-19,
		SlowAccessMode	: 21-20,  /* 2T Mode=[0b:1T , 1b:2T]	*/
		ReservedBits4	: 22-21,
		BankSwizzleMode : 24-22,
		DcqBypassMax	: 28-24,
		tFAW		: 32-28;
	};
} AMD_0F_DRAM_CONFIG_HIGH;

typedef union
{	/* HTT Unit ID Register: Func: 0 - Off: 64h			*/
	unsigned int		value;
	struct {
		unsigned int
		C0Unit		:  2-0,
		C1Unit		:  4-2,
		McUnit		:  6-4,
		HbUnit		:  8-6,
		SbLink		: 10-8,
		ReservedBits	: 32-10;
	};
} AMD_0F_HTT_UNIT_ID;

typedef union
{	/* HTT Link Frequency Capabilities: Func: 0 - Off: 88h, a8h, c8h */
	unsigned int		value;
	struct {
		unsigned int
		MinRev		:  5-0,
		MajRev		:  8-5,
		LinkFreqMax	: 12-8,
		Error		: 16-12,
		LinkFreqCap	: 32-16;
	};
} AMD_0F_HTT_FREQUENCY;
