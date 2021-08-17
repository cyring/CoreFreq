/*
 * CoreFreq
 * Copyright (C) 2015-2021 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#ifndef PCI_VENDOR_ID_HYGON
	#define PCI_VENDOR_ID_HYGON		0x1d94
#endif

#ifndef MSR_AMD_SPEC_CTRL
	#define MSR_AMD_SPEC_CTRL		0x00000048
#endif

#ifndef MSR_AMD_PRED_CMD
	#define MSR_AMD_PRED_CMD		0x00000049
#endif

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

#ifndef MSR_AMD_PSTATE_CURRENT_LIMIT
	#define MSR_AMD_PSTATE_CURRENT_LIMIT	0xc0010061
#endif

#ifndef MSR_AMD_PERF_CTL
	#define MSR_AMD_PERF_CTL		0xc0010062
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

#ifndef MSR_AMD_CSTATE_BAR
	#define MSR_AMD_CSTATE_BAR		0xc0010073
#endif

#define MSR_AMD_CPU_WDT_CFG			0xc0010074

#ifndef MSR_VM_CR
	#define MSR_VM_CR			0xc0010114
#endif

#ifndef MSR_SVM_LOCK_KEY
	#define MSR_SVM_LOCK_KEY		0xc0010118
#endif

#ifndef MSR_AMD_RAPL_POWER_UNIT
	#define MSR_AMD_RAPL_POWER_UNIT 	0xc0010299
#endif

#ifndef MSR_AMD_PKG_ENERGY_STATUS
	#define MSR_AMD_PKG_ENERGY_STATUS	0xc001029b
#endif

#ifndef MSR_AMD_PP0_ENERGY_STATUS
	#define MSR_AMD_PP0_ENERGY_STATUS	0xc001029a
#endif

#ifndef MSR_AMD_PPIN_CTL
	#define MSR_AMD_PPIN_CTL		0xc00102f0
#endif

#ifndef MSR_AMD_PPIN
	#define MSR_AMD_PPIN			0xc00102f1
#endif

#ifndef MSR_AMD_DC_CFG
	#define MSR_AMD_DC_CFG			0xc0011022
#endif

#ifndef MSR_AMD_CU_CFG3
	#define MSR_AMD_CU_CFG3			0xc001102b
#endif

/* Sources: BKDG for AMD Family 0Fh,15_00h-15_0Fh,15_10h-15_1Fh,15_30-15_3Fh */
#define PCI_AMD_TEMPERATURE_TCTL	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0xa4)
#define PCI_AMD_THERMTRIP_STATUS	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0xe4)

/* BKDG for AMD Family [15_00h - 15_0Fh]
	D18F3x1D4 Probe Filter Control
	D18F3x1C4 L3 Cache Parameter
*/
#define PCI_AMD_PROBE_FILTER_CTRL	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0x1d4)
#define PCI_AMD_L3_CACHE_PARAMETER	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0x1c4)

/* Sources:
 * BKDG for AMD Family [15_60h - 15_70h]
	SMU index/data pair registers, D0F0xB8 and D0F0xBC
 * BKDG for AMD Family 16h
	D0F0x60: miscellaneous index to access the registers at D0F0x64_x[FF:00]
*/
#define SMU_AMD_INDEX_REGISTER_F15H	PCI_CONFIG_ADDRESS(0, 0, 0, 0xb8)
#define SMU_AMD_DATA_REGISTER_F15H	PCI_CONFIG_ADDRESS(0, 0, 0, 0xbc)

#define SMU_AMD_INDEX_REGISTER_F17H	PCI_CONFIG_ADDRESS(0, 0, 0, 0x60)
#define SMU_AMD_DATA_REGISTER_F17H	PCI_CONFIG_ADDRESS(0, 0, 0, 0x64)

#define SMU_AMD_INDEX_REGISTER_HSMP	PCI_CONFIG_ADDRESS(0, 0, 0, 0xc4)
#define SMU_AMD_DATA_REGISTER_HSMP	PCI_CONFIG_ADDRESS(0, 0, 0, 0xc8)

/* Sources:
 * BKDG for AMD Family [15_60h - 15_70h]
	D0F0xBC_xD820_0CA4 Reported Temperature Control
 * OSRR for AMD Family 17h processors / Memory Map - SMN
	59800h: SMU::THM
*/
#define SMU_AMD_THM_TRIP_REGISTER_F15H		0xd8200ce4
#define SMU_AMD_THM_TCTL_REGISTER_F15H		0xd8200ca4
#define SMU_AMD_THM_TCTL_REGISTER_F17H		0x00059800

/* UNDOCUMENTED REGISTERS */
#define SMU_AMD_THM_TCTL_CCD_REGISTER_F17H	0x00059954

#define SMU_AMD_F17H_MTS_CPK_PWR		0x0005d2b4
#define SMU_AMD_F17H_MTS_CPK_TDP		0x0005d2b8
#define SMU_AMD_F17H_MTS_CPK_EDC		0x0005d2bc

#define SMU_AMD_F17H_MATISSE_COF		0x0005d2c4
#define SMU_AMD_F17H_CASTLEPEAK_COF		0x0005d324

#ifndef MSR_AMD_PC6_F17H_STATUS
	#define MSR_AMD_PC6_F17H_STATUS 	0xc0010292
#endif

#ifndef MSR_AMD_PSTATE_F17H_BOOST
	#define MSR_AMD_PSTATE_F17H_BOOST	0xc0010293
#endif

#ifndef MSR_AMD_CC6_F17H_STATUS
	#define MSR_AMD_CC6_F17H_STATUS 	0xc0010296
#endif

/* Sources: PPR Vol 2 for AMD Family 19h Model 01h B1			*/
#define SMU_HSMP_F19H	/*Cmd:*/0x3b10534, /*Arg:*/0x3b109e0, /*Rsp:*/0x3b10980

enum HSMP_FUNC {
	HSMP_TEST_MSG	= 0x1,	/* Returns [ARG0] + 1			*/
	HSMP_RD_SMU_VER = 0x2,	/* SMU FW Version			*/
	HSMP_RD_VERSION = 0x3,	/* Interface Version			*/
	HSMP_RD_CUR_PWR = 0x4,	/* Current Socket power (mWatts)	*/
	HSMP_WR_PKG_PL1 = 0x5,	/* Input within [31:0]; Limit (mWatts)	*/
	HSMP_RD_PKG_PL1 = 0x6,	/* Returns Socket power limit (mWatts)	*/
	HSMP_RD_MAX_PPT = 0x7,	/* Max Socket power limit (mWatts)	*/
	HSMP_WR_SMT_BOOST=0x8,	/* ApicId[31:16], Max Freq. (MHz)[15:0] */
	HSMP_WR_ALL_BOOST=0x9,	/* Max Freq. (MHz)[15:0] for ALL	*/
	HSMP_RD_SMT_BOOST=0xa,	/* Input ApicId[15:0]; Dflt Fmax[15:0]	*/
	HSMP_RD_PROCHOT = 0xb,	/* 1 = PROCHOT is asserted		*/
	HSMP_WR_XGMI_WTH= 0xc,	/* 0 = x2, 1 = x8, 2 = x16		*/
	HSMP_RD_APB_PST = 0xd,	/* Data Fabric P-state[7-0]={0,1,2,3}	*/
	HSMP_ENABLE_APB = 0xe,	/* Data Fabric P-State Performance Boost*/
	HSMP_RD_DF_MCLK = 0xf,	/* FCLK[ARG:0], MEMCLK[ARG:1] (MHz)	*/
	HSMP_RD_CCLK	= 0x10, /* CPU core clock limit (MHz)		*/
	HSMP_RD_PC0	= 0x11, /* Socket C0 Residency (100%)		*/
	HSMP_WR_DPM_LCLK= 0x12, /* NBIO[24:16]; Max[15:8], Min[7:0] DPM */
	HSMP_RESERVED	= 0x13,
	HSMP_RD_DDR_BW	= 0x14	/* Max[31:20];Usage{Gbps[19:8],Pct[7:0]}*/
};

enum {
	HSMP_UNSPECIFIED= 0x0,
	HSMP_RESULT_OK	= 0x1,
	HSMP_FAIL_BGN	= 0x2,
	HSMP_FAIL_END	= 0xfd,
	HSMP_INVAL_MSG	= 0xfe,
	HSMP_INVAL_INPUT= 0xff
};

#define IS_HSMP_OOO(_rx) (_rx == HSMP_UNSPECIFIED			\
			|| (_rx >= HSMP_FAIL_BGN && _rx <= HSMP_FAIL_END))

/* Sources: PPR for AMD Family 17h					*/
#define AMD_FCH_PM_CSTATE_EN	0x0000007e

#define AMD_FCH_READ16(_data, _reg)					\
({									\
	__asm__ volatile						\
	(								\
		"movl	%1	,	%%eax"		"\n\t"		\
		"movl	$0xcd6	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"		"\n\t"		\
		"movl	$0xcd7	,	%%edx"		"\n\t"		\
		"inw	%%dx	,	%%ax"		"\n\t"		\
		"movw	%%ax	,	%0"				\
		: "=m"	(_data) 					\
		: "i"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define AMD_FCH_WRITE16(_data, _reg)					\
({									\
	__asm__ volatile						\
	(								\
		"movl	%1	,	%%eax"		"\n\t"		\
		"movl	$0xcd6	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"		"\n\t"		\
		"movw	%0	,	%%ax" 		"\n\t"		\
		"movl	$0xcd7	,	%%edx"		"\n\t"		\
		"outw	%%ax	,	%%dx"		"\n\t"		\
		:							\
		: "im"	(_data),					\
		  "i"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define AMD_FCH_PM_Read16(IndexRegister, DataRegister)			\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(AMD_FCH_LOCK)),		\
				ATOMIC_SEED) ;				\
	if (ret == 0) {							\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		AMD_FCH_READ16(DataRegister.value, IndexRegister);	\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(AMD_FCH_LOCK)),		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
})

#define AMD_FCH_PM_Write16(IndexRegister , DataRegister)		\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(AMD_FCH_LOCK)),		\
				ATOMIC_SEED );				\
	if (ret == 0) {							\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		AMD_FCH_WRITE16(DataRegister.value, IndexRegister);	\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(AMD_FCH_LOCK)),		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
})


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
{	/* Speculative Control: SMT MSR 0x00000048			*/
	unsigned long long value;
    struct
    {
	unsigned long long
	IBRS		:  1-0,  /*RW: Indirect Branch Restriction Speculation*/
	STIBP		:  2-1,  /*RW: Single Thread Indirect Branch Predictor*/
	SSBD		:  3-2,  /*RW: Speculative Store Bypass Disable */
	Reserved1	:  7-3,
	PSFD		:  8-7,  /* RW: Predictive Store Forwarding Disable */
	Reserved2	: 64-8;
    };
} AMD_SPEC_CTRL;

typedef union
{	/* Speculative Control: Per Core MSR 0x00000049 iff CPUID:IBPB	*/
	unsigned long long value;
    struct
    {
	unsigned long long
	IBPB		:  1-0,  /* WO: Indirect Branch Prediction Barrier */
	Reserved	: 64-1;
    };
} AMD_PRED_CMD;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	SmmLock 	:  1-0,  /* SMM Configuration Lock.		*/
	Reserved1	:  3-1,
	TlbCacheDis	:  4-3,  /* Cacheable Memory Disable.		*/
	INVDWBINVD	:  5-4,  /* INVD to WBINVD Conversion.		*/
	Reserved2	:  7-5,
	AllowFerrOnNe	:  8-7,  /* Allow FERR on NE..			*/
	IgnneEm 	:  9-8,  /* IGNNE port emulation enable.	*/
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
	SmuLock 	: 29-28,
	CSEnable	: 30-29, /* Connected standby enable.		*/
	Reserved7	: 32-30,
	Reserved	: 64-32;
    } Family_12h;
    struct
    {
	unsigned long long
	SmmLock 	:  1-0,
	Reserved1	:  3-1,
	TlbCacheDis	:  4-3,
	INVDWBINVD	:  5-4,
	Reserved2	:  7-5,
	AllowFerrOnNe	:  8-7,
	IgnneEm 	:  9-8,
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
	TscFreqSel	: 25-24, /* RO: 1=TSC increments at the P0 frequency */
	CpbDis		: 26-25, /* RW: 1=Core Performance Boost disable */
	EffFreqCntMwait : 27-26, /* RW: A-M-Perf increment during MWAIT */
	EffFreqROLock	: 28-27, /* W1: Lock A-M-Perf & IR-Perf counters */
	Reserved7	: 29-28,
	CSEnable	: 30-29,
	IRPerfEn	: 31-30, /* RW: enable instructions retired counter */
	Reserved8	: 32-31,
	Undefined	: 33-32, /* RW: enable by default		*/
	SmmPgCfgLock	: 34-33,
	Reserved9	: 64-34;
    } Family_17h;
    struct
    {
	unsigned long long
	SmmLock 	:  1-0,
	SLOWFENCE	:  2-1,  /* Slow SFENCE Enable.			*/
	Reserved1	:  3-2,
	TlbCacheDis	:  4-3,
	INVDWBINVD	:  5-4,  /* This bit is required to be set for CC6 */
	Reserved2	:  6-5,
	FFDIS		:  7-6,  /* TLB Flush Filter Disable.		*/
	DISLOCK 	:  8-7,  /* Disable x86 LOCK prefix functionality. */
	IgnneEm 	:  9-8,
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
	CurrFID 	:  6-0,  /* Current FID				*/
	Reserved1	:  8-6,
	StartFID	: 14-8,  /* Startup FID				*/
	Reserved2	: 16-14,
	MaxFID		: 22-16, /* Max FID				*/
	Reserved3	: 24-22,
	MaxRampVID	: 30-24, /* Max Ramp VID			*/
	Reserved4	: 31-30,
	FidVidPending	: 32-31, /* 0b when the FID/VID change has completed.*/
	CurrVID 	: 38-32, /* Current VID				*/
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
	unsigned long long	 /* MSR 0xC001_00[68:64] P-State [4:0]	*/
	CpuFid		:  6-0,  /* Core Frequency ID. RW: Value <= 2Fh */
	CpuDid		:  9-6,  /* Core Divisor ID. RW: 0h-4h divide by 1-16 */
	CpuVid		: 16-9,  /* Core Voltage ID. RW			*/
	Reserved1	: 22-16,
	NbDid		: 23-22, /* Northbridge Divisor ID. RW: 0-1 => 0-2 */
	Reserved2	: 25-23,
	NbVid		: 32-25, /* NB VID. RW: MSR 0xC0010071[MaxVid,MinVid]*/
	IddValue	: 40-32, /* Current Dissipation. RW:00-10b->1,10,100A */
	IddDiv		: 42-40, /* Current Dissipation Divisor. RW	*/
	Reserved3	: 63-42,
	PstateEn	: 64-63; /* Pstate enabled. RW			*/
    } Family_10h;
    struct
    {
	unsigned long long	 /* MSR 0xC001_00[6B:64] P-State [7:0]	*/
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
	unsigned long long	 /* MSR 0xC001_00[6B:64] P-State [7:0]	*/
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
	unsigned long long	 /* MSR 0xC001_00[6B:64] P-state [7:0]	*/
	CpuFid		:  6-0,  /* Core Frequency ID. RW		*/
	CpuDid		:  9-6,  /* Core Divisor ID. RW:0h-4h divide by 1-16 */
	CpuVid		: 16-9,  /* Core Voltage ID. RW			*/
	CpuVid_bit	: 17-16,
	Reserved1	: 22-17,
	NbPstate	: 23-22, /* Northbrige MSR 0xC001_0071[NbPstateDis] */
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
	unsigned long long	 /* MSR 0xC0010061 : iff HwPstate == 1	*/
	CurPstateLimit	:  3-0,  /* Lowest P-State (highest-performance)*/
	Reserved1	:  4-3,
	PstateMaxVal	:  7-4,  /* highest P-State (lowest-performance)*/
	Reserved2	: 64-7;
    } Family_17h;
} PSTATELIMIT;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSR 0xC0010062 : Family 10h up to 17h */
	PstateCmd	:  3-0,
	Reserved	: 64-3;
    };
} PSTATECTRL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSR 0xC0010063 : Family 10h up to 17h */
	Current 	:  3-0,
	Reserved	: 64-3;
    };
} PSTATESTAT;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSR 0xC001_0071 COFVID Status	*/
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
	unsigned long long	 /* MSR 0xC0010071 COFVID Status	*/
	CurCpuDidLSD	:  4-0,  /* Current Core Divisor ID. RO 	*/
	CurCpuDidMSD	:  9-4,
	CurCpuVid	: 16-9,  /* Current Core Voltage ID. RO 	*/
	CurPstate	: 19-16, /* Current P-state. RO 		*/
	Reserved1	: 20-19,
	PstateInProgress: 21-20, /* RO: 1=Change, 0=No change		*/
	Reserved2	: 25-21,
	CurNbVid	: 32-25, /* Current Northbridge VID. RO 	*/
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
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per SMT: MSR 0xC0010073 (RW)	*/
	IOaddr		: 16-0,  /* 0:dis, [0x1-0xFFF8]+[1...6] Six C-States */
	Reserved	: 64-16;
    };
} CSTATE_BASE_ADDR;

typedef union
{
	unsigned long long value; /* MSR 0xC0010055			*/
    struct
    {
	unsigned long long
	IOMsgAddr	: 16-0,
	IOMsgData	: 24-16,
	IntrPndMsgDis	: 25-24,
	IntrPndMsg	: 26-25,
	IORd		: 27-26,
	SmiOnCmpHalt	: 28-27, /* SMI on Multi-core halt		*/
	C1eOnCmpHalt	: 29-28, /* C1E on Multi-core halt: Fam. 0Fh,10h,11h */
	BmStsClrOnHaltEn: 30-29, /* BM_STS clear on Halt enable: Fam. 10h,15h*/
	EnPmTmrCheckLoop: 31-30, /* Enable. Fam. 12h,14h,15h_60h-6Fh	*/
	Reserved	: 32-31,
	RAZ		: 64-32; /* [63:29]0Fh, [63:32]10h,11h,15h, [63:0]16h*/
    };
} INT_PENDING_MSG;

typedef union
{
	unsigned long long value; /* Per SMT: MSR 0xC0010114 (VM_CR)	*/
    struct
    {
	unsigned long long
	DPD		:  1-0,  /* Debug Port Disable. ReservedBits: F17h */
	InterceptInit	:  2-1,
	DisA20m 	:  3-2,  /* Disable A20 Masking. ReservedBits: F17h */
	SVM_Lock	:  4-3,  /* 0=SvmeDisable is read-write, 1=read-only */
	SVME_Disable	:  5-4,  /* 0 = MSR::EFER[SVME] is RW, 1 = read-only */
	Reserved1	: 32-5,
	Reserved2	: 64-32;
    };
} VM_CR;	/* Family: 17h, 16h, 15h, 14h, 12h, 11h, 10h, 0Fh	*/

typedef union
{
	unsigned long long value; /* Per SMT: MSR 0xC0010118		*/
    struct
    {
	unsigned long long
	SvmLockKey	: 64-0; /* Write if (Core::X86::Msr::VM_CR[Lock] == 0)*/
    };
} SVM_LOCK_KEY; /* Family: 17h, 16h, 15h, 14h, 12h, 11h, 10h		*/

typedef union
{
	unsigned long long value; /* Pkg: MSR 0xc00102f0		*/
    struct
    {
	unsigned long long
	LockOut 	:  1-0, /* R/WO: iff CPUID_Fn80000008_EBX.PPIN	*/
	Enable		:  2-1, /* R/W: iff CPUID_Fn80000008_EBX.PPIN  */
	ReservedBits	: 64-2;
    };
} AMD_PPIN_CTL; /* Family: 17h, UNK: 16h,15h,14h,12h,11h,10h. Not: 0Fh	*/

typedef union
{
	unsigned long long value; /* Scope[?]: MSR 0xc0011022		*/
    struct
    {
	unsigned long long
	ReservedBits1	:  4-0,
	DisSpecTlbRld	:  5-4, /* 1=Disable speculative TLB reloads	*/
	ReservedBits2	: 13-5,
	DisHwPf 	: 14-13,
	ReservedBits3	: 15-14,
	DisPfHwForSw	: 16-15,
	L1_HW_Prefetch	: 17-16, /* F17h (BIOS) , Disable=1		*/
	ReservedBits4	: 64-17;
    };
} AMD_DC_CFG; /* Family: 15h(BKDG), 17h(BIOS), Other(TODO)		*/

typedef union
{
	unsigned long long value; /* SharedC: MSR 0xc001102b		*/
    struct
    {
	unsigned long long
	L2_HW_Prefetch	:  1-0, /* F17h (BIOS) , Enable=1		*/
	ReservedBits1	:  3-1,
	PfcL1TrainDis	:  4-3,
	ReservedBits2	: 16-4,
	PfcRegionDis	: 17-16,
	PfcStrideDis	: 18-17,
	PfcDis		: 19-18,
	ReservedBits3	: 20-19,
	PfcStrideMul	: 22-20,
	PfcDoubleStride : 23-22,
	ReservedBits4	: 42-23,
	DisWalkerSharing: 43-42, /* PwcDisableWalkerSharing		*/
	ReservedBits5	: 49-43,
	CombineCr0Cd	: 50-49,
	AsidIncFactor	: 51-50, /* ASID Increment Scale Factor: [16,64]TLB*/
	AsidDecFactor	: 53-52, /* Decrement Factor [16,32,64,128]TLB	*/
	ReservedBits6	: 64-53;
    };
} AMD_CU_CFG3; /* Family: 15h(BKDG), 17h(BIOS), Other(TODO)		*/

typedef struct
{
	unsigned long long value; /* Pkg: MSR 0xc00102f1		*/
} AMD_PPIN_NUM;

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
{	/* HTT Node ID Register: Func: 0 - Off: 60h			*/
	unsigned int		value;
	struct {
		unsigned int
		Node		:  3-0,
		ReservedBits1	:  4-3,
		NodeCnt 	:  7-4,
		ReservedBits2	:  8-7,
		SbNode		: 11-8,
		ReservedBits3	: 12-11,
		LkNode		: 15-12,
		ReservedBits4	: 16-15,
		CPUCnt		: 20-16,
		ReservedBits5	: 32-20;
	};
} AMD_0F_HTT_NODE_ID;

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

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		CapId		:  8-0,
		CapPtr		: 16-8,
		CapType 	: 19-16,
		CapRev		: 24-19,
		IotlbSup	: 25-24,
		HtTunnel	: 26-25,
		NpCache 	: 27-26,
		EFRSup		: 28-27,
		CapExt		: 29-28,
		ReservedBits	: 32-29;
	};
} AMD_IOMMU_CAP_HEADER;

typedef union
{
	unsigned long long	addr;
	struct
	{
		unsigned int	low;
		unsigned int	high;
	};
} AMD_IOMMU_CAP_BAR;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	IOMMU_En 	:  1-0,
	HtTunEn 	:  2-1,
	EventLogEn	:  3-2,
	EventIntEn	:  4-3,
	ComWaitIntEn	:  5-4,
	InvTimeOut	:  8-5,
	PassPW		:  9-8,
	ResPassPW	: 10-9,
	Coherent	: 11-10,
	Isoc		: 12-11,
	CmdBufEn	: 13-12,
	PPRLogEn	: 14-13,
	PprIntEn	: 15-14,
	PPREn		: 16-15,
	GTEn		: 17-16,
	GAEn		: 18-17,
	CRW		: 22-18,
	SmiFEn		: 23-22,
	SlfWBdis	: 24-23,
	SmiFLogEn	: 25-24,
	GAMEn		: 28-25,
	GALogEn 	: 29-28,
	GAIntEn 	: 30-29,
	DualPprLogEn	: 32-30,
	DualEventLogEn	: 34-32,
	DevTblSegEn	: 37-34,
	PrivAbrtEn	: 39-37,
	PprAutoRspEn	: 40-39,
	MarcEn		: 41-40,
	BlkStopMrkEn	: 42-41,
	PprAutoRspAon	: 43-42,
	DomainIDPNE	: 44-43,
	ReservedBits1	: 45-44,
	EPHEn		: 46-45,
	HADUpdate	: 48-46,
	GDUpdateDis	: 49-48,
	ReservedBits2	: 50-49,
	XTEn		: 51-50,
	IntCapXTEn	: 52-51,
	ReservedBits3	: 54-52,
	GAUpdateDis	: 55-54,
	ReservedBits4	: 64-55;
    };
} AMD_IOMMU_CTRL_REG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per Core: MSR 0xC0010074 (RW)	*/
	TmrCfgEn	:  1-0,
	TmrTimebaseSel	:  3-1,
	Reserved1	:  7-3,
	TmrCfgSeverity	: 10-7,
	Reserved2	: 64-10;
    };
} AMD_CPU_WDT_CFG;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		PerStepTimeUp	:  5-0,  /* Family: 12h, 14h, 15h	*/
		TmpMaxDiffUp	:  7-5,  /* Family: 12h, 14h, 15h	*/
		TmpSlewDnEn	:  8-7,  /* Family: 12h, 14h, 15h	*/
		PerStepTimeDn	: 13-8,  /* Family: 12h, 14h, 15h	*/
		ReservedBits1	: 16-13,
		CurTempTJselect : 18-16, /* Family: 15h, 16h		*/
		ReservedBits2	: 19-18,
		CurTempRangeSel : 20-19, /* Family: 17h 		*/
		ReservedBits3	: 21-20,
		CurTmp		: 32-21; /* Family: 12h, 14h, 15h, 17h	*/
	};
} TCTL_REGISTER;

typedef struct
{	/* Family: [15_00h - 15_0Fh]	Bus:0h,Dev:18h,Func:3h,Reg:1D4h */
	unsigned int
	Mode		:  2-0,  /* 00b: Disabled , 01b: Enabled	*/
	WayNum		:  4-2,  /* 00b: 1-way , 01b: 2-way		*/
	SubCacheSize0	:  6-4,  /* 00b: 1MB , 01b: 2MB 		*/
	SubCacheSize1	:  8-6,
	SubCacheSize2	: 10-8,
	SubCacheSize3	: 12-10,
	SubCache0En	: 13-12, /* Subcache bitmask #0, #1, #2 and #3	*/
	SubCache1En	: 14-13,
	SubCache2En	: 15-14,
	SubCache3En	: 16-15,
	DisDirectedPrb	: 17-16,
	WayHashEn	: 18-17,
	ReservedBits1	: 19-18,
	InitDone	: 20-19,
	PreferedSORepl	: 22-20,
	ErrInt		: 24-22,
	LvtOffset	: 28-24,
	EccError	: 29-28,
	LoIndexHashEn	: 30-29,
	ReservedBits2	: 32-30;
} PROBE_FILTER_CTRL;

typedef struct
{	/* Family: [15_00h - 15_0Fh]	Bus:0h,Dev:18h,Func:3h,Reg:1C4h */
	unsigned int
	SubCacheSize0	:  4-0,  /* 0x0c: 2MB , 0x0d: 1 MB , 0xe: 1 MB	*/
	SubCacheSize1	:  8-4,
	SubCacheSize2	: 12-8,
	SubCacheSize3	: 16-12,
	ReservedBits	: 31-16,
	L3TagInit	: 32-31;
} L3_CACHE_PARAMETER;

typedef struct
{	/* PM CStateEn: 16-bits offset I/O=0x7E or MMIO=0xFED80300	*/
	unsigned short int
	Reserved1	:  4-0,
	C1eToC2En	:  5-4,  /* RW: 1="Put APU into C2 in C1E"	*/
	C1eToC3En	:  6-5,  /* RW: 1="Put APU into C3 in C1E"	*/
	Reserved2	: 16-6;
} AMD_17_PM_CSTATE;

typedef union
{
	unsigned short int	value;
	AMD_17_PM_CSTATE	CStateEn;
} PM16;

typedef union
{	/* SMU: address = 0x59954 + ( 4 * CCD[ID] )			*/
	unsigned int		value;
	struct
	{
		unsigned int
		CurTmp		: 11-0,
		CurTempRangeSel : 12-11,
		ReservedBits	: 31-12;
	};
} TCCD_REGISTER;

/* Sources: drivers/edac/amd64_edac.h					*/
#ifndef SMU_AMD_UMC_BASE_CHA_F17H
	#define SMU_AMD_UMC_BASE_CHA_F17H(_cha)	(0x00050000 + (_cha << 20))
#endif

/*
SMU: address = 0x50058 (BankGroupSwap) per channel

BGS[ON]
---
zencli smu 0x50050
[0x00050050] READ(smu) = 0x87654321 (2271560481)
zencli smu 0x50054
[0x00050054] READ(smu) = 0xa9876543 (2844222787)
zencli smu 0x50058
[0x00050058] READ(smu) = 0xcba65321 (3416675105)

BGS[OFF][AUTO]
---
zencli smu 0x50050
[0x00050050] READ(smu) = 0x87654321 (2271560481)
zencli smu 0x50054
[0x00050054] READ(smu) = 0xa9876543 (2844222787)
zencli smu 0x50058
[0x00050058] READ(smu) = 0x87654321 (2271560481)
*/
#define AMD_17_UMC_BGS_MASK_OFF 	0x87654321

/*
SMU: address = 0x500d0 (BankGroupSwap Alternate) per channel

BGS_Alt[ON][AUTO]
---
zencli smu 0x500d0
[0x000500d0] READ(smu) = 0x111107f1 (286328817)
zencli smu 0x500d4
[0x000500d4] READ(smu) = 0x22220001 (572653569)
zencli smu 0x500d8
[0x000500d8] READ(smu) = 0x00000000 (0)

BGS_Alt[OFF]
---
zencli smu 0x500d0
[0x000500d0] READ(smu) = 0x11110001 (286326785)
zencli smu 0x500d4
[0x000500d4] READ(smu) = 0x22220001 (572653569)
zencli smu 0x500d8
[0x000500d8] READ(smu) = 0x00000000 (0)

Remark: if BGS_Alt[ON][AUTO] is set then BGS[OFF]
*/
#define AMD_17_UMC_BGS_ALT_MASK_ON	0x000007f0

typedef union
{	/* SMU: address = 0x50080					*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits1	:  4-0,
		R_DDR4		:  5-4,
		LR_DDR4 	:  6-5, /* DDR4 iff not LR_DDR4 and not R_DDR4*/
		X4_DIMMS	:  7-6,
		X16_DIMMS	:  8-7,
		ReservedBits2	: 32-8;
	};
} AMD_17_UMC_DIMM_CFG;

typedef union
{	/* SMU: address = { 0x50100 , 0x50101 , 0x50102 , 0x50103 }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits1	:  9-0,
		Bit09		: 12-9,  /* when DIMM populated ?	*/
		ECC_Support	: 13-12,
		ReservedBits2	: 31-13,
		Bit31		: 32-31; /* when DIMM populated ?	*/
	};
} AMD_17_UMC_CONFIG;

typedef union
{	/* SMU: address = { 0x50104 , 0x50105 , 0x50106 , 0x50107 }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits	: 31-0,
		INIT		: 32-31;
	};
} AMD_17_UMC_SDP_CTRL;

typedef union
{	/* SMU: address = 0x5014c					*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits1	:  7-0,
		X8_Syndromes	:  8-7, /* X4 iff not X8 and not X16	*/
		ReservedBits2	:  9-8,
		X16_Syndromes	: 10-9,
		ReservedBits3	: 32-10;
	};
} AMD_17_UMC_ECC_CTRL;

typedef union
{	/* SMU: address = { 0x50df0 , 0x50df1 , 0x50df2 , 0x50df3 }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits1	:  4-0,
		Bit04		:  5-4,  /* when DIMM populated ?	*/
		Bit05		:  6-5,  /* when DIMM populated ?	*/
		ReservedBits2	: 16-6,
		Bit16		: 17-16, /* when DIMM populated ?	*/
		ReservedBits3	: 32-17;
	};
} AMD_17_UMC_ECC_CAP;

typedef union
{	/* SMU: address = 0x50df4					*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits	: 30-0,
		Enable		: 31-30,
		ChipKill	: 32-31;
	};
} AMD_17_UMC_ECC_CAP_HI;

typedef union
{	/* SMU: address = { 0x50200, 0x50201 , 0x50202 , 0x50203 }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		MEMCLK		:  7-0,  /* ((Value * 100) / 3) MHz	*/
		ReservedBits1	:  8-7,
		BankGroup	:  9-8,  /* 1: BankGroup is Enable	*/
		CMD_Rate	: 11-9,  /* 0b10 = 2T ; 0b00 = 1T	*/
		GearDownMode	: 12-11, /* BIOS match is OK		*/
		Preamble2T	: 13-12, /* 1: 2T DQS preambles enabled */
		ReservedBits2	: 32-13;
	};
} AMD_17_UMC_CFG_MISC;

typedef union
{	/* SMU: address = { 0x50204 , 0x50205 , 0x50206 , 0x50207 }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		tCL		:  6-0,
		ReservedBits1	:  8-6,
		tRAS		: 15-8,
		ReservedBits2	: 16-15,
		tRCD_RD 	: 22-16,
		ReservedBits3	: 24-22,
		tRCD_WR 	: 30-24,
		ReservedBits4	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR1;

typedef union
{	/* SMU: address = { 0x50208 ,0x50209 , 0x5020a , 0x5020b }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		tRC		:  8-0,
		tRCPB		: 16-8,	 /* Row Cycle Time, Per-Bank	*/
		tRP		: 22-16,
		ReservedBits1	: 24-22,
		tRPPB		: 30-24, /* Row Precharge Time, Per-Bank */
		ReservedBits2	: 32-22;
	};
} AMD_17_UMC_TIMING_DTR2;

typedef union
{	/* SMU: address = { 0x5020c , 0x5020d , 0x5020e , 0x5020f }	*/
	unsigned int		value;
	struct
	{
		unsigned int
		tRRDS		:  5-0,
		ReservedBits1	:  8-5,
		tRRDL		: 13-8,
		ReservedBits2	: 16-13,
		tRRDDLR 	: 21-16, /* tRRD(Different Logical Ranks) */
		ReservedBits3	: 24-21,
		tRTP		: 29-24,
		ReservedBits4	: 32-29;
	};
} AMD_17_UMC_TIMING_DTR3;

typedef union
{	/* SMU: address = { 0x50210 , 0x50211 , 0x50212 , 0x50213 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  7-0,
		ReservedBits1	: 18-7,
		tFAWSLR 	: 24-18, /* tFAW(Same Logical Rank)	*/
		ReservedBits2	: 25-24,
		tFAWDLR 	: 31-25, /* FAW(Different Logical Ranks) */
		ReservedBits4	: 32-31;
	};
} AMD_17_UMC_TIMING_DTR4;

typedef union
{	/* SMU: address = { 0x50214 , 0x50215 , 0x50216 , 0x50217 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tCWL		:  6-0,
		ReservedBits1	:  8-6,
		tWTRS		: 13-8,
		ReservedBits2	: 16-13,
		tWTRL		: 23-16,
		ReservedBits3	: 32-23;
	};
} AMD_17_UMC_TIMING_DTR5;

typedef union
{	/* SMU: address = { 0x50218 , 0x50219 , 0x5021a , 0x5021b }	*/
	unsigned int		value;
	struct {
		unsigned int
		tWR		:  7-0,
		ReservedBits1	: 32-7;
	};
} AMD_17_UMC_TIMING_DTR6;

typedef union
{	/* SMU: address = { 0x5021c , 0x5021d , 0x5021e , 0x5021f }	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 20-0,
		tRCPage		: 32-20; /*	Page Time Line Period	*/
	};
} AMD_17_UMC_TIMING_DTR7;

typedef union
{	/* SMU: address = { 0x50220 , 0x50221 , 0x50222 , 0x50223 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tddRdTRd	:  4-0,
		ReservedBits1	:  8-4,
		tsdRdTRd	: 12-8,
		ReservedBits2	: 16-12,
		tscRdTRd	: 20-16,
		tRdRdScDLR	: 24-20, /* tRdRdSc(Different Logical Ranks) */
		tRdRdScl	: 28-24,
		ReservedBits3	: 30-28,
		tRdRdBan	: 32-30; /* Read to Read Timing Ban	*/
	};				/*  Ban: 00=None, 01=One, 1x=Two */
} AMD_17_UMC_TIMING_DTR8;

typedef union
{	/* SMU: address = { 0x50224 , 0x50225 , 0x50226 , 0x50227 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tddWrTWr	:  4-0,
		ReservedBits1	:  8-4,
		tsdWrTWr	: 12-8,
		ReservedBits2	: 16-12,
		tscWrTWr	: 20-16,
		tWrWrScDLR	: 24-20, /* tWrWrSc(Different Logical Ranks) */
		tWrWrScl	: 30-24,
		tWrWrBan	: 32-30; /* Write to Write Timing Ban	*/
	};
} AMD_17_UMC_TIMING_DTR9;

typedef union
{	/* SMU: address = { 0x50228 , 0x50229 , 0x5022a , 0x5022b }	*/
	unsigned int		value;
	struct {
		unsigned int
		tddWrTRd	:  4-0,
		ReservedBits1	:  8-4,
		tddRdTWr	: 13-8,
		ReservedBits2	: 16-13,
		tWrRdScDLR	: 21-16, /* tWrRdSc(Different Logical Ranks) */
		ReservedBits3	: 32-21;
	};
} AMD_17_UMC_TIMING_DTR10;

typedef union
{	/* SMU: address = { 0x5022c , 0x5022d , 0x5022e , 0x5022f }	*/
	unsigned int		value;
	struct {
		unsigned int /* 0000 1110 0100 0010 0000 0000 1000 0000 */
		tZQCS		:  8-0,
		tZQOPER 	: 20-8,
		ZqcsInterval	: 30-20, /* Value x (2 ^ Exp)		*/
		ReservedBits	: 31-30,
		ShortInit	: 32-31; /* if 1 then Exp=10 else Exp=20 */
	};
} AMD_17_UMC_TIMING_DTR11;

typedef union
{	/* SMU: address = { 0x50230 , 0x50231 , 0x50232 , 0x50233 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		ReservedBits	: 32-16;
	};
} AMD_17_UMC_TIMING_DTR12;

typedef union
{	/* SMU: address = 0x50234					*/
	unsigned int		value;
	struct {
		unsigned int
		tMRS		:  6-0,
		ReservedBits1	:  8-6,
		tMOD		: 14-8,
		ReservedBits2	: 16-14,
		tMRS_PDA	: 22-16,
		ReservedBits3	: 24-22,
		tMOD_PDA	: 30-24,
		ReservedBits4	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR13;

typedef union
{	/* SMU: address = 0x50238					*/
	unsigned int		value;
	struct {
		unsigned int
		tXS		: 11-0,
		ReservedBits1	: 16-11,
		tDLL		: 27-16,
		ReservedBits2	: 32-27;
	};
} AMD_17_UMC_TIMING_DTR14;

typedef union
{	/* SMU: address = 0x5023c					*/
	unsigned int		value;
	struct {
		unsigned int
		tALERT_CRC	:  7-0,
		ReservedBits1	:  8-7,
		tALERT_PARITY	: 15-8,
		ReservedBits2	: 16-15,
		CmdParityLatency: 20-16,
		ReservedBits3	: 24-20,
		tRANK_BUSY	: 31-24,
		ReservedBits4	: 32-31;
	};
} AMD_17_UMC_TIMING_DTR15;

typedef union
{	/* SMU: address = 0x50244					*/
	unsigned int		value;
	struct {
		unsigned int
		tPD		:  5-0,  /* Powerdown Min Delay 	*/
		ReservedBits1	: 17-5,
		tPOWERDOWN	: 25-17, /* Powerdown Delay		*/
		tPRE_PD 	: 31-25, /* Precharge Powerdown 	*/
		ReservedBits2	: 32-31;
	};
} AMD_17_UMC_TIMING_DTR17;

typedef union
{	/* SMU: address = 0x50250					*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 16-0,
		tSTAG		: 24-16,  /* Min Delay between REF cmd	*/
		ReservedBits2	: 32-24;
	};
} AMD_17_UMC_TIMING_DTR20;

typedef union
{	/* SMU: address = { 0x50254 , 0x50255 , 0x50256 , 0x50257 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tXP		:  6-0,
		ReservedBits1	: 16-6,
		tPDE		: 20-16, /* POWERDOWN Entry to CMD Bus	*/
		ReservedBits2	: 24-20,
		tCKE		: 29-24, /*	Clock Enable Time	*/
		ReservedBits3	: 32-29;
	};
} AMD_17_UMC_TIMING_DTR54;

typedef union
{	/* SMU: address = { 0x50260 , 0x50261 , 0x50262 , 0x50263 }	*/
	unsigned int		value;
	struct {
		unsigned int
		tRFC1		: 11-0,
		tRFC2		: 22-11,
		tRFC4		: 32-22;
	};
} AMD_17_UMC_TIMING_DTR60;

typedef union
{	/* SMU: address = { 0x5d2b4 , 0x5d2b5 , 0x5d2b6 , 0x5d2b7 }	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 10-0,
		TjMax		: 17-10,
		PPT		: 26-17,
		ReservedBits2	: 32-26;
	};
} AMD_17_MTS_CPK_PWR;

typedef union
{	/* SMU: address = { 0x5d2b8 , 0x5d2b9 , 0x5d2ba , 0x5d2bb }	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  3-0,
		TDP		: 12-3,
		TDP2		: 21-12, /* Same value returned as TDP! */
		TDP3		: 30-21, /* Same value returned as TDP! */
		ReservedBits2	: 32-30;
	};
} AMD_17_MTS_CPK_TDP;

typedef union
{	/* SMU: address = { 0x5d2bc , 0x5d2bd , 0x5d2be , 0x5d2bf }	*/
	unsigned int		value;
	struct {
		unsigned int
		EDC		:  7-0, /* Returns 35 mult by 4 = 140	*/
		ReservedBits1	: 16-7,
		TDC		: 25-16,
		ReservedBits2	: 32-25;
	};
} AMD_17_MTS_CPK_EDC;

typedef union
{	/* SMU: address = { 0x5d2c4 , 0x5d2c5 , 0x5d2c6 , 0x5d2c7 }	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 17-0,
		BoostRatio	: 25-17, /* Frequence ID of Boosted P-State */
		MinRatio	: 32-25; /* Computed COF of P-State P2	*/
	};
} AMD_17_MTS_CPK_COF;

#ifndef SMU_AMD_F17H_SVI
	#define SMU_AMD_F17H_SVI(_plane)	(0x0005a00c + (_plane << 2))
#endif

#ifndef SMU_AMD_F17_60H_SVI
	#define SMU_AMD_F17_60H_SVI(_plane)	(0x0006f038 + (_plane << 2))
#endif

typedef union
{/*		--- SMU SVI [ 0x5a00c ; 0x5a010 ; 0x5a014 ; 0x6f038] ---
 *				[ CPU addr]	[ SoC addr]
 * ZEN	[8F_01h]		[ 0x5a00c ]	[ 0x5a010 ]
 * ZEN(+) [8F_08h]		[ 0x5a00c ]	[ 0x5a010 ]
 * ZEN(+) [8F_11h ; 8F_18h]	[ 0x5a00c ]	[ 0x5a010 ]
 * ZEN2 [8F_71h]		[ 0x5a010 ]	[ 0x5a00c ]
 * ZEN2 [8F_60h]		[ 0x6f038 ]	[ 0x6f03c ]
 * ZEN2 [8F_31h]		[ 0x5a014 ]	[ 0x5a010 ]
 * ZEN3 [9F_21h]		[ 0x5a010 ]	[ 0x5a00c ]
 */
	unsigned int		value;
	struct {
		unsigned int
		IDD		:  8-0,  /* Current: SVI{0,1}_PLANE0_IDDCOR */
		ReservedBits1	: 16-8,
		VID		: 24-16, /* Voltage: SVI{0,1}_PLANE0_VDDCOR */
		ReservedBits2	: 32-24;
	};
} AMD_17_SVI;

#ifndef SMU_AMD_F17H_CORE_VID
	#define SMU_AMD_F17H_CORE_VID(_mod)	(0x0005a04c + (_mod << 2))
#endif
/*
 * where addr = { 0x5a04c ... 0x5a04f || 0x5a050 ... 0x5a053 }
 * and '_mod' register offset could be equaled to:
 * 0x0		: Zen & Zen+		[UNTESTED]
 * 0x1		: Zen2/Matisse		[VERIFIED]
 * 0x2		: Zen2/CastlePeak	[UNTESTED]
 * 0x5404	: Renoir		[UNTESTED]
*/
typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 24-0, 	/*	MTS: All zeros	*/
		VID		: 32-24;	/*	Voltage ID	*/
	};
} AMD_17_CORE_VID;
