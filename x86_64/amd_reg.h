/*
 * CoreFreq
 * Copyright (C) 2015-2025 CYRIL COURTIAT
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

#define MSR_AMD_PREFETCH_CTRL			0xc0000108

#ifndef  MSR_AMD64_SYSCFG
	#define MSR_AMD64_SYSCFG		0xc0010010
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

#define MSR_AMD_F17H_PERF_CTL			0xc0010200
#define MSR_AMD_F17H_PERF_CTR			0xc0010201
#define MSR_AMD_F17H_L3_PERF_CTL		0xc0010230
#define MSR_AMD_F17H_L3_PERF_CTR		0xc0010231
#define MSR_AMD_F17H_DF_PERF_CTL		0xc0010240
#define MSR_AMD_F17H_DF_PERF_CTR		0xc0010241
#define MSR_AMD_F17H_PMGT_MISC			0xc0010292
#define MSR_AMD_F17H_HW_PSTATE_STATUS		0xc0010293
#define MSR_AMD_F17H_CSTATE_POLICY		0xc0010294
#define MSR_AMD_F17H_CSTATE_CONFIG		0xc0010296
#define MSR_AMD_F17H_PMGT_DEFAULT		0xc0010297

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

#ifndef MSR_AMD64_LS_CFG
	#define MSR_AMD64_LS_CFG		0xc0011020
#endif

#ifndef MSR_AMD_IC_CFG
	#define MSR_AMD_IC_CFG			0xc0011021
#endif

#ifndef MSR_AMD_DC_CFG
	#define MSR_AMD_DC_CFG			0xc0011022
#endif

#define MSR_AMD_TW_CFG				0xc0011023

#ifndef MSR_AMD64_DE_CFG
	#define MSR_AMD64_DE_CFG		0xc0011029
#endif

#ifndef MSR_AMD64_BU_CFG2
	#define MSR_AMD64_BU_CFG2		0xc001102a
#endif

#ifndef MSR_AMD_CU_CFG3
	#define MSR_AMD_CU_CFG3 		0xc001102b
#endif

/* Sources: TECHNICAL GUIDANCE FOR MITIGATING BRANCH TYPE CONFUSION	*/
#ifndef MSR_AMD_DE_CFG2
	#define MSR_AMD_DE_CFG2 		0xc00110e3
#endif

/* Sources: 56569-A1 Rev 3.03 - PPR for AMD Family 19h Model 51h A1	*/
#ifndef MSR_AMD_CPPC_CAP1
	#define MSR_AMD_CPPC_CAP1		0xc00102b0
#endif

#ifndef MSR_AMD_CPPC_ENABLE
	#define MSR_AMD_CPPC_ENABLE		0xc00102b1
#endif

#ifndef MSR_AMD_CPPC_CAP2
	#define MSR_AMD_CPPC_CAP2		0xc00102b2
#endif

#ifndef MSR_AMD_CPPC_REQ
	#define MSR_AMD_CPPC_REQ		0xc00102b3
#endif

#ifndef MSR_AMD_CPPC_STATUS
	#define MSR_AMD_CPPC_STATUS		0xc00102b4
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

#define SMU_AMD_INDEX_PORT_F17H 	0x60
#define SMU_AMD_DATA_PORT_F17H		0x64

#define SMU_AMD_INDEX_REGISTER_F17H					\
	PCI_CONFIG_ADDRESS(0, 0, 0, SMU_AMD_INDEX_PORT_F17H)

#define SMU_AMD_DATA_REGISTER_F17H					\
	PCI_CONFIG_ADDRESS(0, 0, 0, SMU_AMD_DATA_PORT_F17H)

#define AMD_HSMP_INDEX_PORT		0xc4
#define AMD_HSMP_DATA_PORT		0xc8

#define AMD_HSMP_INDEX_REGISTER 					\
	PCI_CONFIG_ADDRESS(0, 0, 0, AMD_HSMP_INDEX_PORT)

#define AMD_HSMP_DATA_REGISTER						\
	PCI_CONFIG_ADDRESS(0, 0, 0, AMD_HSMP_DATA_PORT)

/* Sources:
 * BKDG for AMD Family [15_60h - 15_70h]
	D0F0xBC_xD820_0CA4 Reported Temperature Control
 * OSRR for AMD Family 17h processors / Memory Map - SMN
	59800h: SMU::THM
*/
#define SMU_AMD_THM_TRIP_REGISTER_F15H		0xd8200ce4
#define SMU_AMD_THM_TCTL_REGISTER_F15H		0xd8200ca4
#define SMU_AMD_THM_TCTL_REGISTER_F17H		0x00059800

/* Sources: PPR for AMD Family 19h Model 51h A1 : REGx59800...x59B14	*/
#define SMU_AMD_THM_TCTL_CCD_REGISTER_F17H	0x00059954

#define SMU_AMD_THM_TCTL_CCD_REGISTER_F19H_11H				\
	(SMU_AMD_THM_TCTL_REGISTER_F17H + 0x300)

#define SMU_AMD_THM_TCTL_CCD_REGISTER_F19H_61H				\
	(SMU_AMD_THM_TCTL_REGISTER_F17H + 0x308)

#define SMU_AMD_F17H_ZEN2_MCM_PWR		0x0005d2b4
#define SMU_AMD_F17H_ZEN2_MCM_TDP		0x0005d2b8
#define SMU_AMD_F17H_ZEN2_MCM_EDC		0x0005d2bc

#define SMU_AMD_F17H_MATISSE_COF		0x0005d2c4
#define SMU_AMD_F17H_ZEN2_MCM_COF		0x0005d324

/* Sources: PPR Vol 2 for AMD Family 19h Model 01h B1			*/
#define SMU_HSMP_CMD		0x3b10534
#define SMU_HSMP_ARG		0x3b109e0
#define SMU_HSMP_RSP		0x3b10980
/* Sources: PPR Vol 4 for AMD Family 1Ah Model 02h C1 (Turin)		*/
#define SMU_HSMP_CMD_F1A	0x3b10934

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
	HSMP_RD_DPM_LCLK= 0x13, /* In:NBIO[24:16];Out:Max[15:8],Min[7:0]*/
	HSMP_RD_DDR_BW	= 0x14, /* Max[31:20];Usage{Gbps[19:8],Pct[7:0]}*/
	HSMP_RD_PKG_TMP = 0x15, /* Socket temperature			*/
	HSMP_RD_DIMM_TR = 0x16, /* In:Addr[7:0]; Out:Rate[3], Range[2:0]*/
	HSMP_RD_DIMM_PWR= 0x17, /* In:Addr[7:0];Out:mWatt[31:17]ms[16:8]*/
	HSMP_RD_DIMM_THM= 0x18, /* In:Addr[7:0];Out:Tmp.C[31:21]ms[16:8]*/
	HSMP_RD_FRQ_SKTL= 0x19, /* Out: Socket Freq[31:16], Limit[15-0] */
	HSMP_RD_FRQ_CCLK= 0x1a, /* I:Apic[31:16]; O:Frq[31:16],Lim[15-0]*/
	HSMP_RD_SVI_PWR = 0x1b, /* Out: mWatt[31:0]			*/
	HSMP_RD_FMAX_SKT= 0x1c, /* Out: Fmax[31:16], Fmin[15:0] MHz	*/
	HSMP_RD_IO_BW	= 0x1d, /* In:Link[15:8],Type[2:0]; Out:BW[31:0]*/
	HSMP_RD_XGMI_BW = 0x1e, /* In:Link[15:8],Type[2:0]; Out:BW[31:0]*/
	HSMP_RW_GMI3	= 0x1f, /* RW_Op[31], Min[15:8], Max[7:0] width */
	HSMP_RW_PCIE	= 0x20, /* RW_Op[31], Link_Rate[7:0]		*/
	HSMP_RW_PWR_MODE= 0x21, /* RW_Op[31], Efficiency_Mode[2:0]	*/
	HSMP_RW_DF_PST	= 0x22, /* RW_Op[31], Min[15:8],Max[7:0] P-State*/
	HSMP_RD_MTRC_VER= 0x23, /* Metrics table version		*/
	HSMP_RD_MTRC_TBL= 0x24, /* Metrics table			*/
	HSMP_RD_MTRC_BAR= 0x25, /* Metrics table dram address		*/
	HSMP_RW_XGMI_PST= 0x26, /* RW_Op[31], Min[15:8],Max[7:0] P-State*/
	HSMP_RW_C_POLICY= 0x27, /* RW_Op[31], CPU Freq Policy[0]	*/
	HSMP_RW_DF_CST	= 0x28, /* RW_Op[31], Enable[0] DataFab C-State */
	HSMP_RESERVED	= 0x29,
	HSMP_RD_RAPL_U	= 0x30, /* TU[19:16], ESU[12:8] 1/(2^ESU) Joules*/
	HSMP_RD_RAPL_CCT= 0x31, /* In:Apic[15:0];Out:CoreCounter[Hi:Low]*/
	HSMP_RD_RAPL_PCT= 0x32, /* Pkg Counter[Hi:Low]=[Arg1:Arg0]	*/
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

typedef union
{
	unsigned int	value;
    struct {
	unsigned int
			addr	:  8-0,
			ms	: 17-8,
			mWatt	: 32-17;
    };
} ZEN_HSMP_DIMM_PWR;

/* Sources: BKDG for AMD Families 0Fh, 10h up to 16h			*/
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
	Reserved1	:  7-1,
	SBPB		:  8-7,  /* WO: Selective Branch Predictor Barrior */
	Reserved2	: 64-8;
    };
} AMD_PRED_CMD;

typedef union
{	/* MSR 0xc0000108 supported iff CPUID_Fn80000021_EAX[13]	*/
	unsigned long long value;
    struct
    {						/*	Scope[Core]	*/
	unsigned long long
	L1Stream	:  1-0,
	L1Stride	:  2-1,
	L1Region	:  3-2,
	L2Stream	:  4-3,
	ReservedBits1	:  5-4,
	UpDown		:  6-5,
	ReservedBits2	: 64-6;
    }; /* F19_M01, F19_M61, EPYC 9004					*/
} AMD_PREFETCH_CONTROL;

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
	SmmLock 	:  1-0,  /* RWO: BIOS SMM code lock		*/
	Reserved1	:  3-1,
	TlbCacheDis	:  4-3,  /* RW: 1=Disable cacheable PML4,PDP,PDE,PTE */
	INVDWBINVD	:  5-4,  /* RW: 1=Convert INVD to WBINVD	*/
	Reserved2	:  7-5,
	AllowFerrOnNe	:  8-7,  /* RW: 1=Legacy FERR signaling/exception */
	IgnneEm 	:  9-8,  /* RW: 1=Enable emulation of IGNNE port */
	MonMwaitDis	: 10-9,  /* RW: 1=Disable MONITOR & MWAIT opcodes */
	MonMwaitUserEn	: 11-10, /* RW: 1=MONITOR/MWAIT all privilege levels */
	Reserved3	: 13-11,
	SmiSpCycDis	: 14-13, /* 0=Generate SMI special bus cycle	*/
	RsmSpCycDis	: 15-14, /* 0=Generate RSM special bus cycle	*/
	Reserved4	: 17-15,
	Wrap32Dis	: 18-17, /* RW: 1=Above 4GB Memory in 32-bits mode */
	McStatusWrEn	: 19-18, /* RW: 1=Machine Check status writeable */
	Reserved5	: 20-19,
	IoCfgGpFault	: 21-20, /* RW: 1=IO-space config. causes GP fault */
	LockTscToCurrP0 : 22-21, /* RW: 1=Lock TSC to current P0 frequency */
	Reserved6	: 24-22,
	TscFreqSel	: 25-24, /* RO: 1=TSC increments at the P0 frequency */
	CpbDis		: 26-25, /* RW: 1=Core Performance Boost disable */
	EffFreqCntMwait : 27-26, /* RW: A-M-Perf increment during MWAIT */
	EffFreqROLock	: 28-27, /* W1: Lock A-M-Perf & IR-Perf counters */
	Reserved7	: 29-28,
	CSEnable	: 30-29,
	IRPerfEn	: 31-30, /* RW: Enable Instructions Retired counter */
	SmmBaseLock	: 32-31, /* MSR SMM_BASE saved/restored from save area*/
	TprLoweringDis	: 33-32, /* RW: FastTprLoweringDis: 1=Disabled	*/
	SmmPgCfgLock	: 34-33, /* SMM reserved and iff 8000_0021_EAX[3] */
	Reserved8	: 35-34,
	CpuidUserDis	: 36-35, /* CPUID User Disable iff 8000_0021_EAX[17] */
	Reserved9	: 64-36;
    } Family_17h;
    struct
    {
	unsigned long long
	SmmLock 	:  1-0,
	SLOWFENCE	:  2-1,  /* Slow SFENCE Enable. 		*/
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
	CpuVid		: 16-9,  /* Core Voltage ID. RW 		*/
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
	CpuVid		: 16-9,  /* Core Voltage ID. RW 		*/
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
	CpuVid		: 16-9,  /* Core Voltage ID. RW 		*/
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
	CpuVid		: 16-9,  /* Core Voltage ID. RW 		*/
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
	unsigned long long	 /* MSR 0xC001_0064 [P-state [7:0]]	*/
	CpuFid		:  8-0,  /* Core Frequency ID. RW: FFh-10h <Value>*25 */
	CpuDfsId	: 14-8,  /* Core Divisor ID. RW 		*/
	CpuVid		: 22-14, /* Core Voltage ID. RW 		*/
	IddValue	: 30-22, /* Current Dissipation in amps. RW	*/
	IddDiv		: 32-30, /* Current Dissipation Divisor. RW	*/
	Reserved	: 63-32,
	PstateEn	: 64-63; /* RW: Is this Pstate MSR valid ?	*/
    } Family_17h;
    struct
    {
	unsigned long long	 /* MSR 0xC001_006[4...B] P-state [7:0] */
	CpuFid		:  8-0,  /* RW: COF is defined as CpuFid * 5MHz */
	CpuDfsId	: 14-8,  /* RW: Core DID			*/
	CpuVid		: 22-14, /* RW: VID[7:0]			*/
	IddValue	: 30-22,
	IddDiv		: 32-30,
	CpuVid8 	: 33-32, /* RW: VID[8]				*/
	Reserved	: 63-33,
	PstateEn	: 64-63;
    } Family_19h; /* Model 70h_A0; Model 11h_B1; Model 61h_B1		*/
    struct
    {
	unsigned long long	 /* MSR 0xC001_006[4...B] P-state [7:0] */
	CpuFid		: 12-0,  /* CoreCOF = PStateDef[CpuFid[11:0]] * 5MHz */
	Reserved1	: 14-12,
	CpuVid		: 22-14, /* VID verified w/ 9950X		*/
	IddValue	: 30-22,
	IddDiv		: 32-30,
	CpuVid8 	: 33-32,
	Reserved2	: 63-33,
	PstateEn	: 64-63;
    } Family_1Ah; /* CPUID signature BF_44h, BF_02h			*/
} PSTATEDEF;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSR 0xC001_0293 Hardware P-state Status */
	CpuFid		:  8-0,  /* RO: Current Core Frequency ID	*/
	CpuDfsId	: 14-8,  /* RO: Current Core DID		*/
	CpuVid		: 22-14, /* RO: Current Core VID		*/
	CurHwPstate	: 25-22, /* RO: Current hardware P-state	*/
	Reserved	: 64-63;
    };
} HW_PSTATE_STATUS;

typedef union
{
	unsigned long long value;
    struct
    {	/* MSR 0xC001020{0,2,4,6,8,a} ; 0xC001000{0,1,2,3}		*/
	unsigned long long
	EventSelect00	:  8-0,
	UnitMask	: 16-8,
	OsUserMode	: 18-16,
	EdgeDetect	: 19-18,
	Reserved1	: 20-19,
	APIC_Interrupt	: 21-20,
	Reserved2	: 22-21,
	CounterEn	: 23-22,
	InvCntMask	: 24-23,
	CntMask 	: 32-24,
	EventSelect08	: 36-32,
	Reserved3	: 40-36,
	HostGuestOnly	: 42-40,
	Reserved4	: 64-42;
    };
} ZEN_PERF_CTL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	/* MSR 0xC001023{0,2,4,6,8,a}		*/
	EventSelect	:  8-0,
	UnitMask	: 16-8,
	Reserved1	: 22-16,
	CounterEn	: 23-22,
	Reserved2	: 42-23,
	CoreID		: 45-42,
	Reserved3	: 46-45,
	EnAllSlices	: 47-46,
	EnAllCores	: 48-47,
	SliceMask	: 52-48,
	Reserved4	: 56-52,
	ThreadMask	: 64-56;
    };
} ZEN_L3_PERF_CTL;

#define SMU_AMD_UMC_PERF_CTL_CLK(_umc)	(0x00050d00 + (_umc << 20))

typedef union
{
	unsigned int value;
    struct
    {	/*	SMU addresses = 0x{0,1,2,3,4,5,6,7}50d00		*/
	unsigned int
	GlblResetMsk	:  6-0, /* Six Counters can be reset by GlblReset */
	Reserved1	: 24-6,
	GlblReset	: 25-24,/* Reset Ctr not masked within GlblResetMsk */
	GlblMonEn	: 26-25,/* Global counter enable		*/
	Reserved2	: 31-26,
	CtrClkEn	: 32-31;
    };
} ZEN_UMC_PERF_CTL_CLK;

#define SMU_AMD_ZEN_UMC_PERF_CTL(_umc, _cha)				\
	(0x00050d04 + (_umc << 20) + (_cha << 2))

typedef union
{
	unsigned int value;
    struct
    {	/*	SMU addresses = 0x{0,1,2,3,4,5,6,7}50d{04,08,0c,10}	*/
	unsigned int
	EventSelect	:  8-0,
	RdWrMask	: 10-8, /* Masking: 0=None; 1=Writes; 2=Reads; 3=Rsvd */
	PriorityMask	: 14-10,/* Masking: 0=Low; 1=Medium; 2=High; 3=Urgent */
	ReqSizeMask	: 16-14,/* Transactions: 0=None; 1=32B; 2=64B; 3=Rsvd */
	ChipSelMask	: 20-16,/* Chip Select: 0=CS0; 1=CS1; 2=CS2; 3=CS3 */
	ChipIDSel	: 24-20,/* Only events from 0=C0; 1=C1; 2=C2; 3=Enable*/
	VCSel		: 29-24,/* Only events from 0=VC0; 1=VC1; 2=VC2; 3=VC3*/
	Reserved	: 31-29,
	CounterEn	: 32-31;
    };
} ZEN_UMC_PERF_CTL;

#define SMU_AMD_ZEN_UMC_PERF_CLK_LOW(_cha)				\
	(0x00050d20 + (_cha << 20))

#define SMU_AMD_ZEN_UMC_PERF_CLK_HIGH(_cha)				\
	(0x00050d24 + (_cha << 20))

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	/* MSR 0xC001024{0,2,4,6}		*/
	EventSelect00	:  8-0,
	UnitMask	: 16-8,
	Reserved1	: 22-16,
	CounterEn	: 23-22,
	Reserved2	: 32-23,
	EventSelect08	: 36-32,
	Reserved3	: 59-36,
	EventSelect12	: 61-59,
	Reserved4	: 64-61;
    };
} ZEN_DF_PERF_CTL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* MSR 0xC0010292			*/
	CurPstateLimit	:  3-0,  /* CurHwPstateLimit ; BOOST(MAX)	*/
	StartupPstate	:  6-3,  /* BOOST(MAX)				*/
	DFPstateDis	:  7-6,
	CurDFVid	: 15-7,
	MaxCpuCof	: 21-15,
	MaxDFCof	: 26-21,
	CpbCap		: 29-26,
	Reserved1	: 30-29,
	MaxCpuCofPlus50 : 31-30, /* RO: Add 50 MHz to Max CPU frequency */
	Reserved2	: 32-31,
	PC6En		: 33-32, /* RW: 0=Disable PC6. 1=Enable PC6	*/
	Reserved3	: 64-33;
    };
} ZEN_PMGT_MISC;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per Core: MSR 0xC0010294 (R/W)	*/
	CC1_TMRSEL	:  2-0,
	CC1_TMRLEN	:  7-2,
	HYST_TMRSEL	:  9-7,
	HYST_TMRLEN	: 14-9,
	CFOH_TMRLEN	: 21-14,
	CFOH_TMRSEL	: 22-21,
	C1E_TMRSEL	: 24-22,
	C1E_TMRLEN	: 29-24,
	C1E_EN		: 30-29,
	Reserved1	: 32-30,
	CFSM_DURATION	: 39-32,
	CFSM_THRESHOLD	: 42-39,
	CFSM_MISPREDACT : 44-42,
	IRM_DECRRATE	: 49-44,
	IRM_BURSTEN	: 52-49,
	IRM_THRESHOLD	: 56-52,
	IRM_MAXDEPTH	: 60-56,
	CIT_EN		: 61-60,
	CIT_FASTSAMPLE	: 62-61,
	CLT_EN		: 63-62,
	Reserved2	: 64-63;
    };
} ZEN_CSTATE_POLICY;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per Core: MSR 0xC0010296 (R/W)	*/
	CCR0_CC1DFSID	:  6-0,
	CCR0_CC6EN	:  7-6,
	Reserved1	:  8-7,
	CCR1_CC1DFSID	: 14-8,
	CCR1_CC6EN	: 15-14,
	Reserved2	: 16-15,
	CCR2_CC2DFSID	: 22-16,
	CCR2_CC6EN	: 23-22,
	Reserved3	: 32-23,
	CCR0_CFOHTMR_LEN: 39-32,
	CCR0_CC1E_EN	: 40-39,
	CCR1_CFOHTMR_LEN: 47-40,
	CCR1_CC1E_EN	: 48-47,
	CCR2_CFOHTMR_LEN: 55-48,
	CCR2_CC1E_EN	: 56-55,
	Reserved4	: 64-56;
    };
} ZEN_CSTATE_CONFIG;

typedef union
{
	unsigned long long value; /*	Per Core: MSR 0xC0010297 (R/0)	*/
    struct
    {
	unsigned long long
	CC6EXIT_DFSID	:  6-0,
	CC6EXIT_POPUP_EN:  7-6,
	CC6CF_DFSID	: 13-7,
	CC6CF_POPDN_EN	: 14-13,
	CC6EXIT_STRETCHEN:15-14,
	CC6EX_STRCLKDIV2: 16-15, /*	CC6EXIT_STRETCHCLKDIV2	*/
	CC6EX_STRALLDIV2: 17-16, /*	CC6EXIT_STRETCHALLDIV2	*/
	CC6CF_STRETCHEN : 18-17,
	CC6CF_STRCLKDIV2: 19-18, /*	CC6CF_STRETCHCLKDIV2	*/
	CC6CF_STRALLDIV2: 20-19, /*	CC6CF_STRETCHALLDIV2	*/
	Reserved	: 64-20;
    };
    struct
    {
	unsigned long long
	CC6CF_PSMID	: 14-0,
	CC6CF_DSMID	: 24-14,
	CC6CF_CKS_DSMID : 32-24,
	PSTATE_DFSID	: 38-32,
	PSTATE_POPDN_EN : 39-38,
	PSTATE_STRETCHEN: 40-39,
	CC6CF_EN	: 41-40,
	CC6CF_DFSID	: 47-41,
	CC6CF_STRETCHEN : 48-47,
	CC6CF_IVREN	: 49-48,
	CC6CF_CKS_DSMID2: 51-49,
	CC6EXIT_EN	: 52-51,
	CC6EXIT_DFSID	: 58-52,
	CC6EXIT_STRETCHEN:59-58,
	CC6EXIT_IVREN	: 60-59,
	CC6EXIT_CKS_DSMID:62-60,
	POSTPC6_EN	: 63-62,
	AUTOSEQCTL_EN	: 64-63;
    } HWPSTATE;
} ZEN_PMGT_DEFAULT;

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
	unsigned long long value; /* Per SMT: MSR 0xc0011020		*/
    struct
    {
	unsigned long long
	ReservedBits1	: 10-0,
	F17h_SSBD_EN	: 11-10, /* F17h: 1=Enable SSBD per SMT [low perf] */
	ReservedBits2	: 15-11,
	CVE_2013_6885	: 16-15, /* F16h erratum 793, CVE-2013-6885	*/
	ReservedBits3	: 26-16,
	HitCurPageOpt	: 27-26, /* Disable current table-walk page hit optim.*/
	ReservedBits4	: 28-27,
	StreamingStore	: 29-28, /* F10h...F15h: 1=Disable | Mainboard Enable */
	F16h_SSBD	: 30-29,
	ReservedBits5	: 33-30,
	F16h_SSBD_EN	: 34-33, /* F16h: 1=Enable SSBD per SMT 	*/
	F17h_AgenPick	: 35-34, /* Zen 2: Limited Early Redirect Window */
	ReservedBits6	: 54-35,
	F15h_SSBD	: 55-54, /* F15h,F16h,some F17h: disable SpecLockMap */
	ReservedBits7	: 64-55;
    };
} AMD_LS_CFG;

typedef union
{
	unsigned long long value; /* Scope[Core]: MSR 0xc0011021	*/
    struct
    {
	unsigned long long
	ReservedBits1	:  1-0,
	DisIcWayFilter	:  5-1,  /* F15h-C0: 1=Disable IC way access filter */
	HW_IP_Prefetch	:  6-5,  /* F17h: 1=Disable Instruction Cache	*/
	ReservedBits2	:  9-6,
	DisSpecTlbRld 	: 10-9,  /* F16h: 1=Disable speculative ITLB reloads */
	ReservedBits3	: 11-10,
	DIS_SEQ_PREFETCH: 12-11, /* K8: 1=Disable IC sequential prefetch */
	ReservedBits4	: 26-12,
	WIDEREAD_PWRSAVE: 27-26, /* F16h: 1=Disable wide read power mgmt */
	ReservedBits5	: 39-27,
	DisLoopPredictor: 40-39, /* F15h-C0: 1=Disable loop predictor	*/
	ReservedBits6	: 64-40;
    };
} AMD_IC_CFG;

typedef union
{
	unsigned long long value; /* Scope[Core]: MSR 0xc0011022	*/
    struct
    {
	unsigned long long
	ReservedBits1	:  4-0,
	DisSpecTlbRld	:  5-4, /* 1=Disable speculative DTLB reloads	*/
	ReservedBits2	:  8-5,
	Dis_WBTOL2	:  9-8, /* F12h: 1=DIS_CLR_WBTOL2_SMC_HIT	*/
	ReservedBits3	: 13-9,
	DisHwPf 	: 14-13,
	ReservedBits4	: 15-14,
	DisPfHwForSw	: 16-15,
	L1_HW_Prefetch	: 17-16, /* F17h (BIOS) , Disable=1		*/
	ReservedBits5	: 64-17;
    };
} AMD_DC_CFG; /* Family: 12h(BKDG) ... 17h(BIOS)			*/

typedef union
{
	unsigned long long value; /* Scope[Core]: MSR 0xc0011023	*/
    struct
    {
	unsigned long long
	ReservedBits1	: 49-0,
	CombineCr0Cd	: 50-49,
	ReservedBits5	: 64-50;
    };
} AMD_TW_CFG; /* Family: 10h(BKDG) ... 17h				*/

typedef union
{
	unsigned long long value; /* Scope[?]: MSR 0xc0011029		*/
    struct
    {
	unsigned long long
	ReservedBits1	:  1-0,
	LFENCE_SER	:  2-1,  /* F10h: LFENCE as serializing instruction */
	ReservedBits2	:  9-2,
	Cross_Proc_Leak : 10-9,  /* F17h: CVE-2023-20593		*/
	ReservedBits3	: 23-10,
	CLFLUSH_SER	: 24-23, /* F12h: CLFLUSH as serializing instruction */
	ReservedBits4	: 64-24;
    };
} AMD_DE_CFG; /* Family: 12h ... 17h					*/

typedef union
{
	unsigned long long value; /* SharedC: MSR 0xc001102a		*/
    struct
    {
	unsigned long long
	ReservedBits1	: 35-0,
	IcDisSpecTlbWr	: 36-35, /* F12h: 1=Dis Speculative writes to ITLB */
	ReservedBits2	: 50-36,
	RdMmExtCfgDwDis : 51-50, /* F12h: 1=Dis Read MMIO extended config */
	ReservedBits3	: 56-51,
	L2ClkGatingEn	: 57-56, /* F12h: 1=Enable L2 clock gating	*/
	L2HystCnt	: 59-57, /* F12h: Periodic clocks max number	*/
	ReservedBits4	: 64-59;
    };
} AMD_BU_CFG2; /* Family: 12h ... 17h					*/

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

typedef union
{
	unsigned long long value; /* Scope[SMT]: MSR 0xc00110e3 	*/
    struct
    {
	unsigned long long
	UnspecifiedBit	:  1-0, /* Dumped as one			*/
	SuppressBPOnNonBr: 2-1, /* 1: BTC-NOBR mitigation enabled	*/
	ReservedBits	: 64-2;
    };
} AMD_DE_CFG2; /* Zen2 Family: 17h Models: 30h-4Fh, 60h-7Fh, A0h-AFh	*/

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
	vCmdEn		: 53-52,
	vIommuEn	: 54-53,
	GAUpdateDis	: 55-54,
	GAPPIEn 	: 56-55,
	TMPMEn		: 57-56,
	ReservedBits3	: 58-57,
	GCR3TRPMode	: 59-58,
	IRTCacheDis	: 60-59,
	GstBufferTRPMode: 61-60,
	SNPAVICEn	: 64-61;
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
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per SMT: MSR 0xC00102B0 (RO)	*/
	Lowest		:  8-0,
	LowNonlinear	: 16-8,
	Nominal 	: 24-16,
	Highest 	: 32-24,
	Reserved	: 64-32;
    };
} AMD_CPPC_CAP1;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Package: MSR 0xC00102B1 (RW)	*/
	CPPC_Enable	:  1-0,
	Reserved	: 64-1;
    };
} AMD_CPPC_ENABLE;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per SMT: MSR 0xC00102B2 (RO)	*/
	ConstrainedMax	:  8-0,
	Reserved	: 64-8;
    };
} AMD_CPPC_CAP2;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Per SMT: MSR 0xC00102B3 (RW)	*/
	Minimum_Perf	:  8-0,
	Maximum_Perf	: 16-8,
	Desired_Perf	: 24-16,
	Energy_Pref	: 32-24,
	Reserved	: 64-32;
    };
} AMD_CPPC_REQUEST;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long	 /* Package: MSR 0xC00102B4 (RW)	*/
	Reserved1	:  1-0,
	Min_Excursion	:  2-1,
	Reserved2	: 64-2;
    };
} AMD_CPPC_STATUS;

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
		ReservedBits	: 16-13,
		CurTempTJselect : 18-16, /* Family: 15h, 16h		*/
		CurTempTJslewSel: 19-18,
		CurTempRangeSel : 20-19, /* Family: 17h 		*/
		MCM_EN		: 21-20,
		CurTmp		: 32-21; /* Family: 12h, 14h, 15h, 17h	*/
	};
} TCTL_REGISTER;

typedef union
{
	unsigned int		value;	/* Family: 17h, 19h @ SMU(0x59804) */
	struct
	{
		unsigned int
		HTC_EN		:  1-0,  /* 1: HTC feature is enabled	*/
		ReservedBits1	:  2-1,
		EXTERNAL_PROCHOT:  3-2,
		INTERNAL_PROCHOT:  4-3,
		HTC_ACTIVE	:  5-4,
		HTC_ACTIVE_LOG	:  6-5,  /* 1: HTC_ACTIVE is asserted	*/
		ReservedBits2	:  8-6,
		HTC_DIAG	:  9-8,  /* 1: Trigger HTC iff ACT & EN */
		PROCHOT_PIN_OUT : 10-9,  /* 1: Disable HTC to trigger PROCHOT*/
		HTC_TO_IH_EN	: 11-10, /* Internal PROCHOT Int Handler */
		PROCHOT_TO_IH_EN: 12-11, /* External PROCHOT Int Handler */
		PROCHOT_EVENTSRC: 15-12, /* Select 1=Ext, 2=Internal, 4=Both */
		PROCHOT_PIN_IN	: 16-15, /* 1: Disable external PROCHOT */
		HTC_TMP_LIMIT	: 23-16, /* HTC Temperature Limit	*/
		HTC_HYST_LIMIT	: 27-23,
		HTC_SLEW_SEL	: 29-27,
		ReservedBits3	: 32-29;
	};
} TCTL_HTC;

typedef union
{
	unsigned int		value;	/* Family: 17h, 19h @ SMU(0x59808) */
	struct
	{
		unsigned int
		CTF_PAD_POLARITY:  1-0,  /* Critical Temperature Fault	*/
		THERM_TP	:  2-1,  /* Asserted if THERM_TP_EN == 1 */
		CTF_THRESHOLD	:  3-2,  /* CTF_THRESHOLD_EXCEEDED	*/
		THERM_TP_SENSE	:  4-3,
		ReservedBits1	:  5-4,
		THERM_TP_EN	:  6-5,  /* 1: ThermTrip is enabled	*/
		THERM_TP_LIMIT	: 14-6,
		ReservedBits2	: 31-14,
		SW_THERM_TP	: 32-31; /* 1: Trigger ThermTrip (R/O)	*/
	};
} TCTL_THERM_TRIP;

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
	#define SMU_AMD_UMC_BASE_CHA_F17H(_cha) (0x00050000 + (_cha << 20))
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
{	/* SMU addresses:
		DIMM[0] = 0x{0,1,2,3,4,5,6,7}50030
		DIMM[1] = 0x{0,1,2,3,4,5,6,7}50034
	*/
	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits1	:  2-0,
		NumBankGroups	:  4-2,  /* 0=None; 1=2x; 2=4x; 3=8x BGs */
		NumRM		:  6-4,  /* 0=None; 1=2x; 2=4x; 3=8x RM */
		ReservedBits2	:  8-6,
		NumRowLo	: 12-8,  /* [0-8] = 10 + NumRowLo	*/
		NumRowHi	: 16-12,
		NumCol		: 20-16, /* [0-0xb] = 5 + NumCol	*/
		NumBanks	: 22-20, /* 0=8x; 1=16x; 2=32x Banks	*/
		ReservedBits3	: 32-22;
	};
	struct
	{	/* SMU addresses: 0x500{40,44,48,4c} [RMB]		*/
		unsigned int
		ReservedBits1	:  2-0,
		NumBankGroups	:  4-2,  /* 0=None; 1=2x; 2=4x; 3=8x BGs */
		NumRM		:  7-4,  /* 0=None; 1=2x; 2=4x; 3=8x RM */
		ReservedBits2	:  8-7,
		NumRow		: 12-8,  /* [0-8] = 10 + NumRowLo	*/
		ReservedBits3	: 16-12,
		NumCol		: 20-16, /* [0-0xb] = 5 + NumCol	*/
		NumBanks	: 22-20, /* 0=8x; 1=16x; 2=32x Banks	*/
		ReservedBits4	: 30-22,
		CSXor		: 32-30;
	} Zen4;
} AMD_ZEN_UMC_DRAM_ADDR_CFG;

typedef union
{	/* SMU addresses
		DIMM[0] = 0x{0,1,2,3,4,5,6,7}50080 ; 50090 [RMB]
		DIMM[1] = 0x{0,1,2,3,4,5,6,7}50084 ; 50094 [RMB]
	*/
	unsigned int		value;
	struct
	{
		unsigned int
		OnDimmMirror	:  1-0,
		OutputInvert	:  2-1,
		DRAM_3DS	:  3-2,
		CIsCS		:  4-3,
		RDIMM		:  5-4,
		LRDIMM		:  6-5, /* DDR4 iff not LR_DDR4 and not R_DDR4*/
		X4_DIMMS	:  7-6,
		X16_DIMMS	:  8-7,
		DqMapSwapDis	:  9-8,
		DimmRefDis	: 10-9,
		PkgRnkTimingAlign:11-10,
		ReservedBits	: 32-11;
	};
} AMD_17_UMC_DIMM_CFG;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50100			*/
	unsigned int		value;
	struct
	{
		unsigned int
		DdrType 	:  3-0, /* F19h Model:11h_B1:	1=DDR5	*/
		ReservedBits1	:  8-3,
		BurstLength	: 10-8,
		BurstCtrl	: 12-10,
		ECC_Support	: 13-12,
		ReservedBits2	: 31-13,
		DramReady	: 32-31;
	};
} AMD_ZEN_UMC_CONFIG;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50104			*/
	unsigned int		value;
	struct
	{
		unsigned int
		SdpFatalDatErr	:  1-0,
		SdpParityEn	:  2-1,
		ReservedBits1	:  3-2,
		SdpCancelEn	:  4-3,
		ReservedBits2	:  9-4,
		DramScrubCrdt	: 10-9,
		ReservedBits3	: 16-10,
		CmdBufferCount	: 23-16,
		ReservedBits4	: 24-23, /* Not for F19h Model:11h_B1	*/
		DatBufferCount	: 31-24,
		SdpInit 	: 32-31;
	};
	struct
	{
		unsigned int
		SdpFatalDatErr	:  1-0,
		SdpParityEn	:  2-1,
		ReservedBits1	:  3-2,
		SdpCancelEn	:  4-3,
		ReservedBits2	:  9-4,
		DramScrubCrdt	: 10-9,
		SdpSubChnTokenEn: 11-10, /* F1Ah: dual DCQ SDP token mgmt */
		ReservedBits3	: 12-11,
		SdpAckDly	: 16-12, /* F1Ah: <SdpAckDly> * 32 UCLKs */
		CmdBufferCount	: 24-16,
		DatBufferCount	: 31-24,
		SdpInit 	: 32-31;
	} Zen4;
} AMD_17_UMC_SDP_CTRL;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}5012c			*/
	unsigned int		value;
	struct {
		unsigned int
		DisAutoRefresh	:  1-0,  /* Disable periodic refresh	*/
		PerBankRefEn	:  2-1,  /* F19h:per-bank or same-bank refresh*/
		ReservedBits1	:  3-2,
		LpDis		:  4-3,  /* Disable DFI low power requests */
		UrgRefLimit	:  7-4,  /* UrgRefLimit Refresh range [1-6] */
		ReservedBits2	:  8-7,
		SubUrgRef	: 11-8,  /* SubUrgRefLowerBound <= UrgRefLimit*/
		Tlp_wakeup	: 16-11, /* F19h:lp_ctrl_wakeup lp_data_wakeup*/
		AutoRef_DDR4	: 19-16, /* {1X,2X,4X,RSVD,RSVD,OTF-2X,OTF-4X}*/
		AllBankRefEn	: 20-19, /* F19h: all-bank refresh. REFab */
		PchgCmdSep	: 24-20, /* CMD separation between PRE CMDs */
		AutoRefCmdSep	: 28-24, /* CMD separation between REF CMDs */
		PwrDownEn	: 29-28, /* 1: Enable DRAM Power Down Mode */
		PwrDownMode	: 30-29, /* 0: Full; 1: Partial Channel PD */
		AggrPwrDownEn	: 31-30, /* 1: Aggressive Power Down Mode */
		RefCntMode	: 32-31; /* SPAZ counter: 0: SRX; 1: ARB */
	};
} AMD_17_UMC_SPAZ_CTRL;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50144 (1) 		*/
	unsigned int		value;
	struct
	{
		unsigned int
		DataScrambleEn	:  1-0,  /* 0=Disable, 1=Enable 	*/
		ReservedBits1	:  8-1,
		DataEncrEn	:  9-8,  /* 1=Enable data encryption	*/
		ReservedBits2	: 11-9,
		ForceEncrEn	: 12-11, /* region 0 encrypt. for all requests*/
		Vmguard2Mode	: 13-12, /* 0=511 Keys. 1=255 VmGuard2 Keys */
		ReservedBits3	: 16-13,
		DisAddrTweak	: 20-16, /* Disable address tweaking by region*/
		ReservedBits4	: 32-20;
	};
} AMD_17_UMC_DATA_CTRL;

/* (1)
BIOS UMC Scramble[ENABLE][AUTO]
---
Channel 0
[0x00050144] READ(smu) = 0x00001101 (4353)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0001 0000 0001

Channel 1
[0x00150144] READ(smu) = 0x00001101 (4353)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0001 0000 0001

Channel 2...7
[0x00750144] READ(smu) = 0xffffffff (4294967295)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 1111 1111 1111 1111 1111 1111 1111 1111

BIOS UMC Scramble[DISABLE]
---
Channel 0
[0x00050144] READ(smu) = 0x00001100 (4352)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0001 0000 0000

Channel 1
[0x00150144] READ(smu) = 0x00001100 (4352)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0001 0000 0000

Channel 2...7
[0x00750144] READ(smu) = 0xffffffff (4294967295)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 1111 1111 1111 1111 1111 1111 1111 1111

(2)
UMC::CH::DataScrambleKey
[ENABLE]
---
[0x00050148] READ(smu) = 0xda7a5c11 (3665452049)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 1101 1010 0111 1010 0101 1100 0001 0001

[DISABLE]
---
[0x00050148] READ(smu) = 0xda7a5c11 (3665452049)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 1101 1010 0111 1010 0101 1100 0001 0001

(3)
BIOS UMC TSME[ENABLE]
---
Channel 0
[0x00050144] READ(smu) = 0x00001101 (4353)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 0001 0000 0001

BIOS UMC TSME[DISABLE][AUTO]
---
Channel 0
[0x00050144] READ(smu) = 0x000f1101 (987393)
   60   56   52   48   44   40   36   32   28   24   20   16   12   08   04   00
 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 1111 0001 0001 0000 0001
*/

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}5014c			*/
	unsigned int		value;
	struct
	{
		unsigned int
		WrEccEn 	:  1-0,
		ReservedBits1	:  4-1,
		BadDramSymEn	:  5-4,
		HardwareHistory :  6-5,
		BitInterleaving :  7-6,
		X8_Syndromes	:  8-7, /* X4 iff not X8 and not X16	*/
		UCFatalEn	:  9-8,
		X16_Syndromes	: 10-9,
		RdEccEn 	: 11-10,
		ReservedBits2	: 14-11,
		PinReducedEcc	: 15-14,
		AddrXorEn	: 16-15,
		D5BfEccEn	: 17-16, /* F1Ah:DDR5 Bounded Fault ECC Code */
		EccOvrRdCrcErr	: 18-17, /* F1Ah:override of RD CRC error */
		EccClkGaterDis	: 19-18, /* F1Ah:medium grain clock gating */
		DpprUCErrPoison : 20-19, /* F1Ah:DPPR trigger a poison scrub */
		ReservedBits3	: 32-20;
	};
} AMD_17_UMC_ECC_CTRL;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50d6c			*/
	unsigned int		value;
	struct
	{
		unsigned int
		RAM_Pstate	:  3-0,  /* Current Memory Pstate	*/
		UCLK_Divisor	:  4-3,  /* UCLK:MEMCLK 0[1:2], 1[1:1]	*/
		DFI_Initialized :  5-4,
		UMC_Ready	:  6-5,
		ReservedBits	: 32-6;
	};
} AMD_17_UMC_DEBUG_MISC;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50df0			*/
	unsigned int		value;
	struct
	{
		unsigned int
		DDR_MaxRate	:  8-0,
		ReservedBits1	: 16-8,
		Reg_DIMM_Dis	: 17-16, /* 1: RDIMM/LRDIMM support	*/
		Disable 	: 18-17, /* 1: ECC Support disabled	*/
		Encryption_Dis	: 19-18,
		MemChannel_Dis	: 20-19,
		ReservedBits2	: 32-20;
	};
} AMD_17_UMC_ECC_CAP_LO;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50df4			*/
	unsigned int		value;
	struct
	{
		unsigned int
		DDR_MaxRateEnf	:  8-0,
		ReservedBits	: 30-8,
		Enable		: 31-30, /* 1: ECC logic configured	*/
		ChipKill	: 32-31; /* 1: ECC chipkill configured	*/
	};
} AMD_17_UMC_ECC_CAP_HI;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{200,300,400,500}	*/
	unsigned int		value;
	struct
	{
		unsigned int
		MEMCLK		:  6-0,  /* UMC=((Value * 100) / 3) MHz */
		ReservedBits1	:  7-6,
		ReservedBits2	:  8-7,
		BankGroup	:  9-8,  /* 1: BankGroup is Enable	*/
		CMD_Rate	: 11-9,  /* 0b10 = 2T ; 0b00 = 1T	*/
		GearDownMode	: 12-11, /* BIOS match is OK		*/
		Preamble2T	: 13-12, /* 1: 2T DQS preambles enabled */
		ReservedBits3	: 32-13;
	} DDR4;
	struct
	{
		unsigned int
		MEMCLK		: 16-0,  /* DRAM = (Value * 2) MT/s	*/
		CMD_Rate	: 18-16,
		GearDownMode	: 19-18,
		ReservedBits1	: 32-19;
	} DDR5;
} AMD_ZEN_UMC_CFG_MISC;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{204,304,404,504}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{208,308,405,508}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{20c,30c,40c,50c}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{210,310,410,510}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{214,314,414,514}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{218,318,418,518}	*/
	unsigned int		value;
	struct {
		unsigned int
		tWR		:  7-0,
		ReservedBits1	: 32-7;
	};
} AMD_17_UMC_TIMING_DTR6;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{21c,31c,41c,51c}	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 20-0,
		tRCPage 	: 32-20; /*	Page Time Line Period	*/
	};
} AMD_17_UMC_TIMING_DTR7;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{220,320,420,520}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{224,324,424,524}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{228,328,428,528}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{22c,32c,42c,52c}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{230,330,430,530}	*/
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		ReservedBits	: 32-16;
	};
} AMD_17_UMC_TIMING_DTR12;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{234,334,434,534}	*/
	unsigned int		value;
	struct {
		unsigned int
		tMRD		:  6-0,
		ReservedBits1	:  8-6,
		tMOD		: 14-8,
		ReservedBits2	: 16-14,
		tMRD_PDA	: 22-16,
		ReservedBits3	: 24-22,
		tMOD_PDA	: 30-24,
		ReservedBits4	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR13;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{238,338,438,538}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{23c,33c,43c,53c}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{244,344,444,544}	*/
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
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{250,350,450,550}	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 16-0,
		tSTAG		: 24-16,  /* Min Delay between REF cmd	*/
		ReservedBits2	: 32-24;
	};
} AMD_17_UMC_TIMING_DTR20;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{254,354,454,554}	*/
	unsigned int		value;
	struct {
		unsigned int
		tXP		:  6-0,
		ReservedBits1	: 16-6,
		tCPDED		: 20-16, /* Command pass disable delay	*/
		ReservedBits2	: 24-20,
		tCKE		: 29-24, /*	Clock Enable Time	*/
		ReservedBits3	: 32-29;
	};
} AMD_17_UMC_TIMING_DTR21;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{258,358,458,558}	*/
	unsigned int		value;
	struct {
		unsigned int
		tRDDATA_EN	:  7-0,
		ReservedBits1	:  8-7,
		tPHY_WRLAT	: 13-8,
		ReservedBits2	: 16-13,
		tPHY_RDLAT	: 22-16,
		ReservedBits3	: 24-22,
		tPHY_WRDATA	: 27-24,
		ReservedBits4	: 28-27,
		tPARIN_LAT	: 30-28,
		ReservedBits5	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR22;

typedef union
{	/* SMU addresses
		DIMM[0] = 0x{0,1,2,3,4,5,6,7}50{260,360,460,560}
		DIMM[1] = 0x{0,1,2,3,4,5,6,7}50{264,364,464,564}
	*/
	unsigned int		value;
	struct {
		unsigned int
		tRFC1		: 11-0,
		tRFC2		: 22-11,
		tRFC4		: 32-22;
	} DDR4;
	struct {
		unsigned int
		tRFC1		: 16-0,
		tRFC2		: 32-16;
	} DDR5;
} AMD_ZEN_UMC_TIMING_DTRFC;

typedef union
{	/* SMU addresses = 0x50{2c0,2c4,2c8,2cc}			*/
	unsigned int		value;	/* Rembrandt = 0x00480138	*/
	struct {
		unsigned int
		tRFCsb		: 16-0,
		UnknownBits	: 32-16;
	} DDR5;
} AMD_ZEN_UMC_TIMING_RFCSB;

typedef union
{	/* SMU addresses = 0x{0,1,2,3,4,5,6,7}50{28c,38c,48c,58c}	*/
	unsigned int		value;
	struct {
		unsigned int
		RcvrWait	: 11-0,
		CmdStgCnt	: 22-11,
		ReservedBits1	: 24-22,
		tWR_MPR 	: 30-24,
		ReservedBits2	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR35;

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
} AMD_17_MTS_MCM_PWR;

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
} AMD_17_MTS_MCM_TDP;

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
} AMD_17_MTS_MCM_EDC;

typedef union
{	/* SMU: address = { 0x5d2c4 , 0x5d2c5 , 0x5d2c6 , 0x5d2c7 }	*/
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 17-0,
		BoostRatio	: 25-17, /* Frequence ID of Boosted P-State */
		MinRatio	: 32-25; /* Computed COF of P-State P2	*/
	};
} AMD_17_ZEN2_COF;

#ifndef SMU_AMD_F17H_SVI
	#define SMU_AMD_F17H_SVI(_plane)	(0x0005a00c + (_plane << 2))
#endif

#ifndef SMU_AMD_F17_60H_SVI
	#define SMU_AMD_F17_60H_SVI(_plane)	(0x0006f038 + (_plane << 2))
#endif

#ifndef SMU_AMD_RMB_SVI
	#define SMU_AMD_RMB_SVI(_plane) 	(0x0006f010 + (_plane << 2))
#endif

#ifndef SMU_AMD_F19H_SVI
	#define SMU_AMD_F19H_SVI(_plane)	(0x0007300c + (_plane << 2))
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
} AMD_F17H_SVI;

typedef union
{/*				--- SMU SVI [ Rembrandt ] ---
 *				[ CPU addr]	[ SoC addr]
 * ZEN3(+) [AF_44]		[ 0x6f010 ]	[ 0x6f014 ]
 */
	unsigned int		value;
	struct {
		unsigned int
		SVI0		:  8-0,
		SVI1		: 16-8,
		SVI2		: 24-16,
		SVI3		: 32-24;
	};
} AMD_RMB_SVI;

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
} AMD_F17H_CORE_VID;

typedef union
{/*				--- SMU SVI [ ZEN4 ] ---
 * Genoa [AF_11] 		[ 0x5a010 ]	[ 0x5a014 ]
 *			Idle:	0x00009a81	0x00019a81
 *			Load:	0x0000a401	0x0001a401
 * Raphael [AF_61]		[ 0x73010 ]	[ 0x73014 ]
 *			Idle:	0x0000bd41	0x0001bd41
 *			Load:	0x0000b9c1	0x0001b901
 */
	unsigned int		value;
	struct {
		unsigned int
		SVI0		:  8-0,
		SVI1		: 16-8,
		PKG		: 17-16,  /* 1 for 2nd processor socket */
		RSVD		: 32-17;
	};
} AMD_F19H_SVI;
