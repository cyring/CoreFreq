/*
 * CoreFreq
 * Copyright (C) 2015-2023 CYRIL COURTIAT
 * Licenses: GPL2
 */
/*TODO(CleanUp)
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

** Sources: TECHNICAL GUIDANCE FOR MITIGATING BRANCH TYPE CONFUSION	**
#ifndef MSR_AMD_DE_CFG2
	#define MSR_AMD_DE_CFG2 		0xc00110e3
#endif

** Sources: 56569-A1 Rev 3.03 - PPR for AMD Family 19h Model 51h A1	**
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

** Sources: BKDG for AMD Family 0Fh,15_00h-15_0Fh,15_10h-15_1Fh,15_30-15_3Fh **
#define PCI_AMD_TEMPERATURE_TCTL	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0xa4)
#define PCI_AMD_THERMTRIP_STATUS	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0xe4)

** BKDG for AMD Family [15_00h - 15_0Fh]
	D18F3x1D4 Probe Filter Control
	D18F3x1C4 L3 Cache Parameter
**
#define PCI_AMD_PROBE_FILTER_CTRL	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0x1d4)
#define PCI_AMD_L3_CACHE_PARAMETER	PCI_CONFIG_ADDRESS(0, 0x18, 0x3, 0x1c4)

** Sources:
 * BKDG for AMD Family [15_60h - 15_70h]
	SMU index/data pair registers, D0F0xB8 and D0F0xBC
 * BKDG for AMD Family 16h
	D0F0x60: miscellaneous index to access the registers at D0F0x64_x[FF:00]
**
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

** Sources:
 * BKDG for AMD Family [15_60h - 15_70h]
	D0F0xBC_xD820_0CA4 Reported Temperature Control
 * OSRR for AMD Family 17h processors / Memory Map - SMN
	59800h: SMU::THM
**
#define SMU_AMD_THM_TRIP_REGISTER_F15H		0xd8200ce4
#define SMU_AMD_THM_TCTL_REGISTER_F15H		0xd8200ca4
#define SMU_AMD_THM_TCTL_REGISTER_F17H		0x00059800

** Sources: PPR for AMD Family 19h Model 51h A1 : REGx59800...x59B14	**
#define SMU_AMD_THM_TCTL_CCD_REGISTER_F17H	0x00059954

#define SMU_AMD_THM_TCTL_CCD_REGISTER_F19H_61H				\
	(SMU_AMD_THM_TCTL_REGISTER_F17H + 0x308)

#define SMU_AMD_F17H_ZEN2_MCM_PWR		0x0005d2b4
#define SMU_AMD_F17H_ZEN2_MCM_TDP		0x0005d2b8
#define SMU_AMD_F17H_ZEN2_MCM_EDC		0x0005d2bc

#define SMU_AMD_F17H_MATISSE_COF		0x0005d2c4
#define SMU_AMD_F17H_ZEN2_MCM_COF		0x0005d324

** Sources: PPR Vol 2 for AMD Family 19h Model 01h B1			**
#define SMU_HSMP_CMD		0x3b10534
#define SMU_HSMP_ARG		0x3b109e0
#define SMU_HSMP_RSP		0x3b10980

enum HSMP_FUNC {
	HSMP_TEST_MSG	= 0x1,	** Returns [ARG0] + 1			**
	HSMP_RD_SMU_VER = 0x2,	** SMU FW Version			**
	HSMP_RD_VERSION = 0x3,	** Interface Version			**
	HSMP_RD_CUR_PWR = 0x4,	** Current Socket power (mWatts)	**
	HSMP_WR_PKG_PL1 = 0x5,	** Input within [31:0]; Limit (mWatts)	**
	HSMP_RD_PKG_PL1 = 0x6,	** Returns Socket power limit (mWatts)	**
	HSMP_RD_MAX_PPT = 0x7,	** Max Socket power limit (mWatts)	**
	HSMP_WR_SMT_BOOST=0x8,	** ApicId[31:16], Max Freq. (MHz)[15:0] **
	HSMP_WR_ALL_BOOST=0x9,	** Max Freq. (MHz)[15:0] for ALL	**
	HSMP_RD_SMT_BOOST=0xa,	** Input ApicId[15:0]; Dflt Fmax[15:0]	**
	HSMP_RD_PROCHOT = 0xb,	** 1 = PROCHOT is asserted		**
	HSMP_WR_XGMI_WTH= 0xc,	** 0 = x2, 1 = x8, 2 = x16		**
	HSMP_RD_APB_PST = 0xd,	** Data Fabric P-state[7-0]={0,1,2,3}	**
	HSMP_ENABLE_APB = 0xe,	** Data Fabric P-State Performance Boost**
	HSMP_RD_DF_MCLK = 0xf,	** FCLK[ARG:0], MEMCLK[ARG:1] (MHz)	**
	HSMP_RD_CCLK	= 0x10, ** CPU core clock limit (MHz)		**
	HSMP_RD_PC0	= 0x11, ** Socket C0 Residency (100%)		**
	HSMP_WR_DPM_LCLK= 0x12, ** NBIO[24:16]; Max[15:8], Min[7:0] DPM **
	HSMP_RESERVED	= 0x13,
	HSMP_RD_DDR_BW	= 0x14	** Max[31:20];Usage{Gbps[19:8],Pct[7:0]}**
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

** Sources: BKDG for AMD Families 0Fh, 10h up to 16h			**
const struct {
	unsigned int	MCF,
			PCF[5];
} VCO[0b1000] = {
** FID **
** 000000b **	{ 8, { 0,  0, 16, 17, 18}},
** 000001b **	{ 9, {16, 17, 18, 19, 20}},
** 000010b **	{10, {18, 19, 20, 21, 22}},
** 000011b **	{11, {20, 21, 22, 23, 24}},
** 000100b **	{12, {22, 23, 24, 25, 26}},
** 000101b **	{13, {24, 25, 26, 27, 28}},
** 000110b **	{14, {26, 27, 28, 29, 30}},
** 000111b **	{15, {28, 29, 30, 31, 32}},
};

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	IBRS		:  1-0,
	STIBP		:  2-1,
	SSBD		:  3-2,
	Reserved1	:  7-3,
	PSFD		:  8-7,
	Reserved2	: 64-8;
    };
} AMD_SPEC_CTRL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	IBPB		:  1-0,
	Reserved	: 64-1;
    };
} AMD_PRED_CMD;

typedef union
{
	unsigned long long value;
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
	Reserved3	: 12-11,
	HltXSpCycEn	: 13-12,
	SmiSpCycDis	: 14-13,
	RsmSpCycDis	: 15-14,
	Reserved4	: 17-15,
	Wrap32Dis	: 18-17,
	McStatusWrEn	: 19-18,
	Reserved5	: 20-19,
	IoCfgGpFault	: 21-20,
	Reserved6	: 23-21,
	ForceRdWrSzPrb	: 24-23,
	TscFreqSel	: 25-24,
	CpbDis		: 26-25,
	EffFreqCntMwait : 27-26,

	EffFreqROLock	: 28-27,
	SmuLock 	: 29-28,
	CSEnable	: 30-29,
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
	LockTscToCurrP0 : 22-21,
	Reserved6	: 24-22,
	TscFreqSel	: 25-24,
	CpbDis		: 26-25,
	EffFreqCntMwait : 27-26,
	EffFreqROLock	: 28-27,
	Reserved7	: 29-28,
	CSEnable	: 30-29,
	IRPerfEn	: 31-30,
	SmmBaseLock	: 32-31,
	TprLoweringDis	: 33-32,
	SmmPgCfgLock	: 34-33,
	Reserved8	: 35-34,
	CpuidUserDis	: 36-35,
	Reserved9	: 64-36;
    } Family_17h;
    struct
    {
	unsigned long long
	SmmLock 	:  1-0,
	SLOWFENCE	:  2-1,
	Reserved1	:  3-2,
	TlbCacheDis	:  4-3,
	INVDWBINVD	:  5-4,
	Reserved2	:  6-5,
	FFDIS		:  7-6,
	DISLOCK 	:  8-7,
	IgnneEm 	:  9-8,
	Reserved3	: 12-9,
	HltXSpCycEn	: 13-12,
	SmiSpCycDis	: 14-13,
	RsmSpCycDis	: 15-14,
	SSEDIS		: 16-15,
	Reserved4	: 17-16,
	Wrap32Dis	: 18-17,
	McStatusWrEn	: 19-18,
	Reserved5	: 24-19,
	StartFID	: 30-24,
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
	InitFidVid	: 17-16,
	Reserved3	: 32-17,
	StpGntTOCnt	: 52-32,
	Reserved4	: 64-52;
    };
} FIDVID_CONTROL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CurrFID 	:  6-0,
	Reserved1	:  8-6,
	StartFID	: 14-8,
	Reserved2	: 16-14,
	MaxFID		: 22-16,
	Reserved3	: 24-22,
	MaxRampVID	: 30-24,
	Reserved4	: 31-30,
	FidVidPending	: 32-31,
	CurrVID 	: 38-32,
	Reserved5	: 40-38,
	StartVID	: 46-40,
	Reserved6	: 48-46,
	MaxVID		: 54-48,
	Reserved7	: 56-54,
	PstateStep	: 57-56,
	AltVidOffset	: 60-57,
	Reserved8	: 61-60,
	IntPstateSup	: 62-61,
	Reserved9	: 64-62;
    };
} FIDVID_STATUS;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CpuFid		:  6-0,
	CpuDid		:  9-6,
	CpuVid		: 16-9,
	Reserved1	: 22-16,
	NbDid		: 23-22,
	Reserved2	: 25-23,
	NbVid		: 32-25,
	IddValue	: 40-32,
	IddDiv		: 42-40,
	Reserved3	: 63-42,
	PstateEn	: 64-63;
    } Family_10h;
    struct
    {
	unsigned long long
	CpuDid		:  4-0,
	CpuFid		:  9-4,
	CpuVid		: 16-9,
	Reserved1	: 32-16,
	IddValue	: 40-32,
	IddDiv		: 42-40,
	Reserved2	: 63-42,
	PstateEn	: 64-63;
    } Family_12h;
    struct
    {
	unsigned long long
	CpuDidLSD	:  4-0,
	CpuDidMSD	:  9-4,
	CpuVid		: 16-9,
	Reserved1	: 32-16,
	IddValue	: 40-32,
	IddDiv		: 42-40,
	Reserved2	: 63-42,
	PstateEn	: 64-63;
    } Family_14h;
    struct
    {
	unsigned long long
	CpuFid		:  6-0,
	CpuDid		:  9-6,
	CpuVid		: 16-9,
	CpuVid_bit	: 17-16,
	Reserved1	: 22-17,
	NbPstate	: 23-22,
	Reserved2	: 32-23,
	IddValue	: 40-32,
	IddDiv		: 42-40,
	Reserved3	: 63-42,
	PstateEn	: 64-63;
    } Family_15h;
    struct
    {
	unsigned long long
	CpuFid		:  8-0,
	CpuDfsId	: 14-8,
	CpuVid		: 22-14,
	IddValue	: 30-22,
	IddDiv		: 32-30,
	Reserved	: 63-32,
	PstateEn	: 64-63;
    } Family_17h;
} PSTATEDEF;

typedef union
{
	unsigned long long value;
    struct
    {
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
	unsigned long long
	EventSelect	:  8-0,
	UnitMask	: 16-8,
	Reserved1	: 22-16,
	CounterEn	: 23-22,
	Reserved2	: 48-23,
	SliceMask	: 52-48,
	Reserved3	: 56-52,
	ThreadMask	: 64-56;
    };
} ZEN_L3_PERF_CTL;
*/
#define SMU_AMD_UMC_PERF_CTL_CLK(_umc)	(0x00050d00 + (_umc << 20))
/*
typedef union
{
	unsigned int value;
    struct
    {
	unsigned int
	GlblResetMsk	:  6-0,
	Reserved1	: 24-6,
	GlblReset	: 25-24,
	GlblMonEn	: 26-25,
	Reserved2	: 31-26,
	CtrClkEn	: 32-31;
    };
} ZEN_UMC_PERF_CTL_CLK;
*/
#define SMU_AMD_ZEN_UMC_PERF_CTL(_umc, _cha)				\
	(0x00050d04 + (_umc << 20) + (_cha << 2))
/*
typedef union
{
	unsigned int value;
    struct
    {
	unsigned int
	EventSelect	:  8-0,
	RdWrMask	: 10-8,
	PriorityMask	: 14-10,
	ReqSizeMask	: 16-14,
	ChipSelMask	: 20-16,
	ChipIDSel	: 24-20,
	VCSel		: 29-24,
	Reserved	: 31-29,
	CounterEn	: 32-31;
    };
} ZEN_UMC_PERF_CTL;
*/
#define SMU_AMD_ZEN_UMC_PERF_CLK_LOW(_cha)				\
	(0x00050d20 + (_cha << 20))

#define SMU_AMD_ZEN_UMC_PERF_CLK_HIGH(_cha)				\
	(0x00050d24 + (_cha << 20))
/*
typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
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
	unsigned long long
	CurPstateLimit	:  3-0,
	StartupPstate	:  6-3,
	DFPstateDis	:  7-6,
	CurDFVid	: 15-7,
	MaxCpuCof	: 21-15,
	MaxDFCof	: 26-21,
	CpbCap		: 29-26,
	Reserved1	: 32-29,
	PC6En		: 33-32,
	Reserved2	: 64-33;
    };
} ZEN_PMGT_MISC;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CC1_TMRSEL	:  2-0,
	CC1_TMRLEN	:  7-2,
	HYST_TMRSEL	:  9-7,
	HYST_TMRLEN	: 14-9,
	CFOH_TMRLEN	: 21-14,
	Reserved1	: 32-21,
	CFSM_DURATION	: 39-32,
	CFSM_THRESHOLD	: 42-39,
	CFSM_MISPREDACT : 44-42,
	IRM_DECRRATE	: 49-44,
	IRM_BURSTEN	: 52-49,
	IRM_THRESHOLD	: 56-52,
	IRM_MAXDEPTH	: 60-56,
	CIT_EN		: 61-60,
	CIT_FASTSAMPLE	: 62-61,
	Reserved2	: 64-62;
    };
} ZEN_CSTATE_POLICY;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CCR0_CC1DFSID	:  6-0,
	CCR0_CC6EN	:  7-6,
	Reserved1	:  8-7,
	CCR1_CC1DFSID	: 14-8,
	CCR1_CC6EN	: 15-14,
	Reserved2	: 16-15,
	CCR2_CC2DFSID	: 22-16,
	CCR2_CC6EN	: 23-22,
	Reserved3	: 64-23;
    };
} ZEN_CSTATE_CONFIG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CurPstateLimit	:  3-0,
	Reserved1	:  4-3,
	PstateMaxVal	:  7-4,
	Reserved2	: 64-7;
    } Family_17h;
} PSTATELIMIT;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	PstateCmd	:  3-0,
	Reserved	: 64-3;
    };
} PSTATECTRL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	Current 	:  3-0,
	Reserved	: 64-3;
    };
} PSTATESTAT;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
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
	unsigned long long
	CurCpuDidLSD	:  4-0,
	CurCpuDidMSD	:  9-4,
	CurCpuVid	: 16-9,
	CurPstate	: 19-16,
	Reserved1	: 20-19,
	PstateInProgress: 21-20,
	Reserved2	: 25-21,
	CurNbVid	: 32-25,
	StartupPstate	: 35-32,
	MaxVid		: 42-35,
	MinVid		: 49-42,
	MainPllOpFidMax : 55-49,
	Reserved3	: 56-55,
	CurPstateLimit	: 59-56,
	Reserved4	: 64-59;
    } Arch_Pll;
} COFVID;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	IOaddr		: 16-0,
	Reserved	: 64-16;
    };
} CSTATE_BASE_ADDR;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	IOMsgAddr	: 16-0,
	IOMsgData	: 24-16,
	IntrPndMsgDis	: 25-24,
	IntrPndMsg	: 26-25,
	IORd		: 27-26,
	SmiOnCmpHalt	: 28-27,
	C1eOnCmpHalt	: 29-28,
	BmStsClrOnHaltEn: 30-29,
	EnPmTmrCheckLoop: 31-30,
	Reserved	: 32-31,
	RAZ		: 64-32;
    };
} INT_PENDING_MSG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	DPD		:  1-0,
	InterceptInit	:  2-1,
	DisA20m 	:  3-2,
	SVM_Lock	:  4-3,
	SVME_Disable	:  5-4,
	Reserved1	: 32-5,
	Reserved2	: 64-32;
    };
} VM_CR;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	SvmLockKey	: 64-0;
    };
} SVM_LOCK_KEY;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	LockOut 	:  1-0,
	Enable		:  2-1,
	ReservedBits	: 64-2;
    };
} AMD_PPIN_CTL;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ReservedBits1	: 10-0,
	F17h_SSBD_EN	: 11-10,
	ReservedBits2	: 15-11,
	CVE_2013_6885	: 16-15,
	ReservedBits3	: 26-16,
	HitCurPageOpt	: 27-26,
	ReservedBits4	: 28-27,
	StreamingStore	: 29-28,
	F16h_SSBD	: 30-29,
	ReservedBits5	: 33-30,
	F16h_SSBD_EN	: 34-33,
	ReservedBits6	: 54-34,
	F15h_SSBD	: 55-54,
	ReservedBits7	: 64-55;
    };
} AMD_LS_CFG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ReservedBits1	:  1-0,
	DisIcWayFilter	:  5-1,
	HW_IP_Prefetch	:  6-5,
	ReservedBits2	:  9-6,
	DisSpecTlbRld 	: 10-9,
	ReservedBits3	: 11-10,
	DIS_SEQ_PREFETCH: 12-11,
	ReservedBits4	: 26-12,
	WIDEREAD_PWRSAVE: 27-26,
	ReservedBits5	: 39-27,
	DisLoopPredictor: 40-39,
	ReservedBits6	: 64-40;
    };
} AMD_IC_CFG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ReservedBits1	:  4-0,
	DisSpecTlbRld	:  5-4,
	ReservedBits2	:  8-5,
	Dis_WBTOL2	:  9-8,
	ReservedBits3	: 13-9,
	DisHwPf 	: 14-13,
	ReservedBits4	: 15-14,
	DisPfHwForSw	: 16-15,
	L1_HW_Prefetch	: 17-16,
	ReservedBits5	: 64-17;
    };
} AMD_DC_CFG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ReservedBits1	: 49-0,
	CombineCr0Cd	: 50-49,
	ReservedBits5	: 64-50;
    };
} AMD_TW_CFG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ReservedBits1	:  1-0,
	LFENCE_SER	:  2-1,
	ReservedBits2	: 23-2,
	CLFLUSH_SER	: 24-23,
	ReservedBits3	: 64-24;
    };
} AMD_DE_CFG;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ReservedBits1	: 35-0,
	IcDisSpecTlbWr	: 36-35,
	ReservedBits2	: 50-36,
	RdMmExtCfgDwDis : 51-50,
	ReservedBits3	: 56-51,
	L2ClkGatingEn	: 57-56,
	L2HystCnt	: 59-57,
	ReservedBits4	: 64-59;
    };
} AMD_BU_CFG2;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	L2_HW_Prefetch	:  1-0,
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
	DisWalkerSharing: 43-42,
	ReservedBits5	: 49-43,
	CombineCr0Cd	: 50-49,
	AsidIncFactor	: 51-50,
	AsidDecFactor	: 53-52,
	ReservedBits6	: 64-53;
    };
} AMD_CU_CFG3;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	UnspecifiedBit	:  1-0,
	SuppressBPOnNonBr: 2-1,
	ReservedBits	: 64-2;
    };
} AMD_DE_CFG2;

typedef struct
{
	unsigned long long value;
} AMD_PPIN_NUM;

typedef struct
{
	unsigned int
	Reserved1	:  1-0,
	SensorTrip	:  2-1,
	SensorCoreSelect:  3-2,
	Sensor0Trip	:  4-3,
	Sensor1Trip	:  5-4,
	SensorTripEnable:  6-5,
	SelectSensorCPU	:  7-6,
	Reserved2	:  8-7,
	DiodeOffset	: 14-8,
	Reserved3	: 16-14,
	CurrentTemp	: 24-16,
	TjOffset	: 29-24,
	Reserved4	: 31-29,
	SwThermTrip	: 32-31;
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
{
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
{
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
{
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
		BurstLength32	: 11-10,
		Width128	: 12-11,
		X4_DIMMS	: 16-12,
		UnbufferedDIMM	: 17-16,
		ReservedBits3	: 19-17,
		ECC_DIMM_Enable : 20-19,
		ReservedBits4	: 32-20;
	};
} AMD_0F_DRAM_CONFIG_LOW;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		MemClkFreq	:  3-0,
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
		SlowAccessMode	: 21-20,
		ReservedBits4	: 22-21,
		BankSwizzleMode : 24-22,
		DcqBypassMax	: 28-24,
		tFAW		: 32-28;
	};
} AMD_0F_DRAM_CONFIG_HIGH;

typedef union
{
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
{
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
{
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
	unsigned long long
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
	unsigned long long
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
	unsigned long long
	CPPC_Enable	:  1-0,
	Reserved	: 64-1;
    };
} AMD_CPPC_ENABLE;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	ConstrainedMax	:  8-0,
	Reserved	: 64-8;
    };
} AMD_CPPC_CAP2;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
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
	unsigned long long
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
		PerStepTimeUp	:  5-0,
		TmpMaxDiffUp	:  7-5,
		TmpSlewDnEn	:  8-7,
		PerStepTimeDn	: 13-8,
		ReservedBits	: 16-13,
		CurTempTJselect : 18-16,
		CurTempTJslewSel: 19-18,
		CurTempRangeSel : 20-19,
		MCM_EN		: 21-20,
		CurTmp		: 32-21;
	};
} TCTL_REGISTER;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		HTC_EN		:  1-0,
		ReservedBits1	:  2-1,
		EXTERNAL_PROCHOT:  3-2,
		INTERNAL_PROCHOT:  4-3,
		HTC_ACTIVE	:  5-4,
		HTC_ACTIVE_LOG	:  6-5,
		ReservedBits2	:  8-6,
		HTC_DIAG	:  9-8,
		PROCHOT_PIN_OUT : 10-9,
		HTC_TO_IH_EN	: 11-10,
		PROCHOT_TO_IH_EN: 12-11,
		PROCHOT_EVENTSRC: 15-12,
		PROCHOT_PIN_IN	: 16-15,
		HTC_TMP_LIMIT	: 23-16,
		HTC_HYST_LIMIT	: 27-23,
		HTC_SLEW_SEL	: 29-27,
		ReservedBits3	: 32-29;
	};
} TCTL_HTC;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		CTF_PAD_POLARITY:  1-0,
		THERM_TP	:  2-1,
		CTF_THRESHOLD	:  3-2,
		THERM_TP_SENSE	:  4-3,
		ReservedBits1	:  5-4,
		THERM_TP_EN	:  6-5,
		THERM_TP_LIMIT	: 14-6,
		ReservedBits2	: 31-14,
		SW_THERM_TP	: 32-31;
	};
} TCTL_THERM_TRIP;

typedef struct
{
	unsigned int
	Mode		:  2-0,
	WayNum		:  4-2,
	SubCacheSize0	:  6-4,
	SubCacheSize1	:  8-6,
	SubCacheSize2	: 10-8,
	SubCacheSize3	: 12-10,
	SubCache0En	: 13-12,
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
{
	unsigned int
	SubCacheSize0	:  4-0,
	SubCacheSize1	:  8-4,
	SubCacheSize2	: 12-8,
	SubCacheSize3	: 16-12,
	ReservedBits	: 31-16,
	L3TagInit	: 32-31;
} L3_CACHE_PARAMETER;

typedef struct
{
	unsigned short int
	Reserved1	:  4-0,
	C1eToC2En	:  5-4,
	C1eToC3En	:  6-5,
	Reserved2	: 16-6;
} AMD_17_PM_CSTATE;

typedef union
{
	unsigned short int	value;
	AMD_17_PM_CSTATE	CStateEn;
} PM16;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		CurTmp		: 11-0,
		CurTempRangeSel : 12-11,
		ReservedBits	: 31-12;
	};
} TCCD_REGISTER;
*/
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
/*TODO(CleanUp)
typedef union
{	unsigned int		value;
	struct
	{
		unsigned int
		ReservedBits1	:  2-0,
		NumBankGroups	:  4-2,
		NumRM		:  6-4,
		ReservedBits2	:  8-6,
		NumRowLo	: 12-8,
		NumRowHi	: 16-12,
		NumCol		: 20-16,
		NumBanks	: 22-20,
		ReservedBits3	: 32-22;
	};
	struct
	{
		unsigned int
		ReservedBits1	:  2-0,
		NumBankGroups	:  4-2,
		NumRM		:  7-4,
		ReservedBits2	:  8-7,
		NumRow		: 12-8,
		ReservedBits3	: 16-12,
		NumCol		: 20-16,
		NumBanks	: 22-20,
		ReservedBits4	: 30-22,
		CSXor		: 32-30;
	} Zen4;
} AMD_ZEN_UMC_DRAM_ADDR_CFG;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		OnDimmMirror	:  1-0,
		OutputInvert	:  2-1,
		DRAM_3DS	:  3-2,
		CIsCS		:  4-3,
		RDIMM		:  5-4,
		LRDIMM		:  6-5,
		X4_DIMMS	:  7-6,
		X16_DIMMS	:  8-7,
		DqMapSwapDis	:  9-8,
		DimmRefDis	: 10-9,
		PkgRnkTimingAlign:11-10,
		ReservedBits	: 32-11;
	};
} AMD_17_UMC_DIMM_CFG;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		DdrType 	:  3-0,
		ReservedBits1	:  8-3,
		BurstLength	: 10-8,
		BurstCtrl	: 12-10,
		ECC_Support	: 13-12,
		ReservedBits2	: 31-13,
		DramReady	: 32-31;
	};
} AMD_ZEN_UMC_CONFIG;

typedef union
{
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
		ReservedBits4	: 24-23,
		DatBufferCount	: 31-24,
		SdpInit 	: 32-31;
	};
} AMD_17_UMC_SDP_CTRL;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		DisAutoRefresh	:  1-0,
		ReservedBits1	:  3-1,
		LpDis		:  4-3,
		UrgRefLimit	:  7-4,
		ReservedBits2	:  8-7,
		SubUrgRef	: 11-8,
		ReservedBits3	: 16-11,
		AutoRef_DDR4	: 19-16,
		ReservedBits4	: 20-19,
		PchgCmdSep	: 24-20,
		AutoRefCmdSep	: 28-24,
		PwrDownEn	: 29-28,
		PwrDownMode	: 30-29,
		AggrPwrDownEn	: 31-30,
		RefCntMode	: 32-31;
	};
} AMD_17_UMC_SPAZ_CTRL;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		DataScrambleEn	:  1-0,
		ReservedBits1	:  8-1,
		DataEncrEn	:  9-8,
		ReservedBits2	: 11-9,
		ForceEncrEn	: 12-11,
		Vmguard2Mode	: 13-12,
		ReservedBits3	: 16-13,
		DisAddrTweak	: 20-16,
		ReservedBits4	: 32-20;
	};
} AMD_17_UMC_DATA_CTRL;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		WrEccEn 	:  1-0,
		ReservedBits1	:  4-1,
		BadDramSymEn	:  5-4,
		HardwareHistory :  6-5,
		BitInterleaving :  7-6,
		X8_Syndromes	:  8-7,
		UCFatalEn	:  9-8,
		X16_Syndromes	: 10-9,
		RdEccEn 	: 11-10,
		ReservedBits2	: 14-11,
		PinReducedEcc	: 15-14,
		AddrXorEn	: 16-15,
		ReservedBits3	: 32-16;
	};
} AMD_17_UMC_ECC_CTRL;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		RAM_Pstate	:  3-0,
		UCLK_Divisor	:  4-3,
		DFI_Initialized :  5-4,
		UMC_Ready	:  6-5,
		ReservedBits	: 32-6;
	};
} AMD_17_UMC_DEBUG_MISC;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		DDR_MaxRate	:  8-0,
		ReservedBits1	: 16-8,
		Reg_DIMM_Dis	: 17-16,
		Disable 	: 18-17,
		Encryption_Dis	: 19-18,
		MemChannel_Dis	: 20-19,
		ReservedBits2	: 32-20;
	};
} AMD_17_UMC_ECC_CAP_LO;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		DDR_MaxRateEnf	:  8-0,
		ReservedBits	: 30-8,
		Enable		: 31-30,
		ChipKill	: 32-31;
	};
} AMD_17_UMC_ECC_CAP_HI;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		MEMCLK		:  6-0,
		ReservedBits1	:  7-6,
		ReservedBits2	:  8-7,
		BankGroup	:  9-8,
		CMD_Rate	: 11-9,
		GearDownMode	: 12-11,
		Preamble2T	: 13-12,
		ReservedBits3	: 32-13;
	} DDR4;
	struct
	{
		unsigned int
		MEMCLK		: 16-0,
		CMD_Rate	: 18-16,
		GearDownMode	: 19-18,
		ReservedBits1	: 32-19;
	} DDR5;
} AMD_ZEN_UMC_CFG_MISC;

typedef union
{
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
{
	unsigned int		value;
	struct
	{
		unsigned int
		tRC		:  8-0,
		tRCPB		: 16-8,
		tRP		: 22-16,
		ReservedBits1	: 24-22,
		tRPPB		: 30-24,
		ReservedBits2	: 32-22;
	};
} AMD_17_UMC_TIMING_DTR2;

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		tRRDS		:  5-0,
		ReservedBits1	:  8-5,
		tRRDL		: 13-8,
		ReservedBits2	: 16-13,
		tRRDDLR 	: 21-16,
		ReservedBits3	: 24-21,
		tRTP		: 29-24,
		ReservedBits4	: 32-29;
	};
} AMD_17_UMC_TIMING_DTR3;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tFAW		:  7-0,
		ReservedBits1	: 18-7,
		tFAWSLR 	: 24-18,
		ReservedBits2	: 25-24,
		tFAWDLR 	: 31-25,
		ReservedBits4	: 32-31;
	};
} AMD_17_UMC_TIMING_DTR4;

typedef union
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		tWR		:  7-0,
		ReservedBits1	: 32-7;
	};
} AMD_17_UMC_TIMING_DTR6;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 20-0,
		tRCPage 	: 32-20;
	};
} AMD_17_UMC_TIMING_DTR7;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tddRdTRd	:  4-0,
		ReservedBits1	:  8-4,
		tsdRdTRd	: 12-8,
		ReservedBits2	: 16-12,
		tscRdTRd	: 20-16,
		tRdRdScDLR	: 24-20,
		tRdRdScl	: 28-24,
		ReservedBits3	: 30-28,
		tRdRdBan	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR8;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tddWrTWr	:  4-0,
		ReservedBits1	:  8-4,
		tsdWrTWr	: 12-8,
		ReservedBits2	: 16-12,
		tscWrTWr	: 20-16,
		tWrWrScDLR	: 24-20,
		tWrWrScl	: 30-24,
		tWrWrBan	: 32-30;
	};
} AMD_17_UMC_TIMING_DTR9;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tddWrTRd	:  4-0,
		ReservedBits1	:  8-4,
		tddRdTWr	: 13-8,
		ReservedBits2	: 16-13,
		tWrRdScDLR	: 21-16,
		ReservedBits3	: 32-21;
	};
} AMD_17_UMC_TIMING_DTR10;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tZQCS		:  8-0,
		tZQOPER 	: 20-8,
		ZqcsInterval	: 30-20,
		ReservedBits	: 31-30,
		ShortInit	: 32-31;
	};
} AMD_17_UMC_TIMING_DTR11;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tREFI		: 16-0,
		ReservedBits	: 32-16;
	};
} AMD_17_UMC_TIMING_DTR12;

typedef union
{
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
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		tPD		:  5-0,
		ReservedBits1	: 17-5,
		tPOWERDOWN	: 25-17,
		tPRE_PD 	: 31-25,
		ReservedBits2	: 32-31;
	};
} AMD_17_UMC_TIMING_DTR17;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	: 16-0,
		tSTAG		: 24-16,
		ReservedBits2	: 32-24;
	};
} AMD_17_UMC_TIMING_DTR20;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		tXP		:  6-0,
		ReservedBits1	: 16-6,
		tCPDED		: 20-16,
		ReservedBits2	: 24-20,
		tCKE		: 29-24,
		ReservedBits3	: 32-29;
	};
} AMD_17_UMC_TIMING_DTR21;

typedef union
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		tRFCsb		: 16-0,
		UnknownBits	: 32-16;
	} DDR5;
} AMD_ZEN_UMC_TIMING_RFCSB;

typedef union
{
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
{
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
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits1	:  3-0,
		TDP		: 12-3,
		TDP2		: 21-12,
		TDP3		: 30-21,
		ReservedBits2	: 32-30;
	};
} AMD_17_MTS_MCM_TDP;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		EDC		:  7-0,
		ReservedBits1	: 16-7,
		TDC		: 25-16,
		ReservedBits2	: 32-25;
	};
} AMD_17_MTS_MCM_EDC;

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 17-0,
		BoostRatio	: 25-17,
		MinRatio	: 32-25;
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

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		IDD		:  8-0,
		ReservedBits1	: 16-8,
		VID		: 24-16,
		ReservedBits2	: 32-24;
	};
} AMD_17_SVI;

typedef union
{
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

typedef union
{
	unsigned int		value;
	struct {
		unsigned int
		ReservedBits	: 24-0,
		VID		: 32-24;
	};
} AMD_17_CORE_VID;
*/