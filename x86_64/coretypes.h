/*
 * CoreFreq
 * Copyright (C) 2015-2023 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define COREFREQ_MAJOR	1
#define COREFREQ_MINOR	97
#define COREFREQ_REV	0

#if !defined(CORE_COUNT)
	#define CORE_COUNT	256
#elif !(CORE_COUNT == 64 || CORE_COUNT == 128 || CORE_COUNT == 256 \
	|| CORE_COUNT == 512 || CORE_COUNT == 1024)
	#error "CORE_COUNT must be 64, 128, 256, 512 or 1024"
#endif

enum CRC_MANUFACTURER
{
	CRC_INTEL	= 0x75a2ba39,
	CRC_AMD 	= 0x3485bbd3,
	CRC_HYGON	= 0x18044630,
	CRC_KVM 	= 0x0e8c8561,
	CRC_VBOX	= 0x5091f045,
	CRC_KBOX	= 0x02b76f04,
	CRC_VMWARE	= 0x2a974552,
	CRC_HYPERV	= 0x543a585e
};

enum {	GenuineArch = 0,
	AMD_Family_0Fh,
	AMD_Family_10h,
	AMD_Family_11h,
	AMD_Family_12h,
	AMD_Family_14h,
	AMD_Family_15h,
	AMD_Family_16h,
	AMD_Family_17h,
	Hygon_Family_18h,
	AMD_Family_19h,
	Core_Yonah,
	Core_Conroe,
	Core_Kentsfield,
	Core_Conroe_616,
	Core_Penryn,
	Core_Dunnington,
	Atom_Bonnell,
	Atom_Silvermont,
	Atom_Lincroft,
	Atom_Clover_Trail,
	Atom_Saltwell,
	Silvermont_Bay_Trail,
	Atom_Avoton,
	Atom_Airmont,
	Atom_Goldmont,
	Atom_Sofia,
	Atom_Merrifield,
	Atom_Moorefield,
	Nehalem_Bloomfield,
	Nehalem_Lynnfield,
	Nehalem_MB,
	Nehalem_EX,
	Westmere,
	Westmere_EP,
	Westmere_EX,
	SandyBridge,
	SandyBridge_EP,
	IvyBridge,
	IvyBridge_EP,
	Haswell_DT,
	Haswell_EP,
	Haswell_ULT,
	Haswell_ULX,
	Broadwell,
	Broadwell_D,
	Broadwell_H,
	Broadwell_EP,
	Skylake_UY,
	Skylake_S,
	Skylake_X,
	Xeon_Phi,
	Kabylake,
	Kabylake_UY,
	Cannonlake_U,
	Cannonlake_H,
	Spreadtrum,
	Geminilake,
	Icelake,
	Icelake_UY,
	Icelake_X,
	Icelake_D,
	Sunny_Cove,
	Tigerlake,
	Tigerlake_U,
	Cometlake,
	Cometlake_UY,
	Atom_Denverton,
	Tremont_Jacobsville,
	Tremont_Lakefield,
	Tremont_Elkhartlake,
	Tremont_Jasperlake,
	Sapphire_Rapids,
	Emerald_Rapids,
	Granite_Rapids_X,
	Granite_Rapids_D,
	Sierra_Forest,
	Grand_Ridge,
	Rocketlake,
	Rocketlake_U,
	Alderlake_S,
	Alderlake_H,
	Alderlake_N,
	Meteorlake_M,
	Meteorlake_N,
	Meteorlake_S,
	Raptorlake,
	Raptorlake_P,
	Raptorlake_S,
	LunarLake,
	ArrowLake,
	AMD_Zen,
	AMD_Zen_APU,
	AMD_ZenPlus,
	AMD_ZenPlus_APU,
	AMD_Zen_Dali,
	AMD_EPYC_Rome_CPK,
	AMD_Zen2_Renoir,
	AMD_Zen2_LCN,
	AMD_Zen2_MTS,
	AMD_Zen2_Ariel,
	AMD_Zen2_Jupiter,
	AMD_Zen2_MDN,
	AMD_Zen3_VMR,
	AMD_Zen3_CZN,
	AMD_EPYC_Milan,
	AMD_Zen3_Chagall,
	AMD_Zen3_Badami,
	AMD_Zen3Plus_RMB,
	AMD_Zen4_Genoa,
	AMD_Zen4_RPL,
	AMD_Zen4_PHX,
	AMD_Zen4_Bergamo,
	ARCHITECTURES
};

enum HYBRID_ARCH {
	Hybrid_RSVD1	= 0x10,
	Hybrid_Atom	= 0x20,
	Hybrid_RSVD2	= 0x30,
	Hybrid_Core	= 0x40
};

enum HYPERVISOR {
	HYPERV_NONE,
	BARE_METAL,
	HYPERV_XEN,
	HYPERV_KVM,
	HYPERV_VBOX,
	HYPERV_KBOX,
	HYPERV_VMWARE,
	HYPERV_HYPERV
};

#define HYPERVISORS	( 1 + HYPERV_HYPERV )

enum SYS_REG {
	RFLAG_TF	= 8,
	RFLAG_IF	= 9,
	RFLAG_IOPL	= 12,	/* [13:12]				*/
	RFLAG_NT	= 14,
	RFLAG_RF	= 16,
	RFLAG_VM	= 17,
	RFLAG_AC	= 18,
	RFLAG_VIF	= 19,
	RFLAG_VIP	= 20,
	RFLAG_ID	= 21,

	CR0_PE		= 0,
	CR0_MP		= 1,
	CR0_EM		= 2,
	CR0_TS		= 3,
	CR0_ET		= 4,
	CR0_NE		= 5,
	CR0_WP		= 16,
	CR0_AM		= 18,
	CR0_NW		= 29,
	CR0_CD		= 30,
	CR0_PG		= 31,

	CR3_PWT 	= 3,
	CR3_PCD 	= 4,
	CR3_LAM_U57	= 61,
	CR3_LAM_U48	= 62,

	CR4_VME 	= 0,
	CR4_PVI 	= 1,
	CR4_TSD 	= 2,
	CR4_DE		= 3,
	CR4_PSE 	= 4,
	CR4_PAE 	= 5,
	CR4_MCE 	= 6,
	CR4_PGE 	= 7,
	CR4_PCE 	= 8,
	CR4_OSFXSR	= 9,
	CR4_OSXMMEXCPT	= 10,
	CR4_UMIP	= 11,
	CR4_LA57	= 12,
	CR4_VMXE	= 13,
	CR4_SMXE	= 14,
	CR4_FSGSBASE	= 16,
	CR4_PCIDE	= 17,
	CR4_OSXSAVE	= 18,
	CR4_KL		= 19,
	CR4_SMEP	= 20,
	CR4_SMAP	= 21,
	CR4_PKE 	= 22,
	CR4_CET 	= 23,
	CR4_PKS 	= 24,
	CR4_UINTR	= 25,
	CR4_LAM_SUP	= 28,

	CR8_TPL 	= 0,	/* [3:0]				*/

	EXFCR_LOCK	= 0,
	EXFCR_VMX_IN_SMX= 1,
	EXFCR_VMXOUT_SMX= 2,
	EXFCR_SENTER_LEN= 8,	/* [14:8]				*/
	EXFCR_SENTER_GEN= 15,
	EXFCR_SGX_LCE	= 17,
	EXFCR_SGX_GEN	= 18,
	EXFCR_LMCE	= 20,

	EXFER_SCE	= 0,
	EXFER_LME	= 8,
	EXFER_LMA	= 10,
	EXFER_NXE	= 11,
	EXFER_SVME	= 12,	/* AMD F17h				*/
	EXFER_LMSLE	= 13,	/* AMD F17h				*/
	EXFER_FFXSE	= 14,	/* AMD F17h				*/
	EXFER_TCE	= 15,	/* AMD F17h				*/
	EXFER_MCOMMIT	= 17,	/* AMD F17h				*/
	EXFER_INT_WBINVD= 18,	/* AMD F17h				*/
	EXFER_UAIE	= 20,	/* AMD F17h				*/
	EXFER_AIBRSE	= 21,	/* AMD F17h				*/

	XCR0_FPU	= 0,
	XCR0_SSE	= 1,
	XCR0_AVX	= 2,
	XCR0_MPX	= 3,	/* [BNDCSR:4, BNDREG:3] 		*/
	XCR0_AVX512	= 5,	/* [7:5]				*/
	XCR0_PKRU	= 9,
	XCR0_CET_U	= 11,	/* AMD64, Intel 			*/
	XCR0_CET_S	= 12,	/* AMD64, Intel 			*/
	XCR0_AMX	= 17,	/* Sapphire Rapids [TILEDATA:18, TILECFG:17] */
	XCR0_APX	= 19,
	XCR0_LWP	= 62,	/* AMD64				*/

	SYSCFG_MFD	= 18,	/* AMD64: System Configuration Register */
	SYSCFG_MFDM	= 19,
	SYSCFG_MVDM	= 20,
	SYSCFG_TOM2	= 21,
	SYSCFG_FWB	= 22,
	SYSCFG_MEM	= 23,
	SYSCFG_SNP	= 24,
	SYSCFG_VMPL	= 25,
	SYSCFG_HMK	= 26,

	UNDEF_CR	= 64
};

enum CSTATES_ENCODING {
	_C0	= 0x0,
	_C1	= 0x1,
	_C2	= 0x2,
	_C3	= 0x3,
	_C4	= 0x4,
	_C6	= 0x6,
	_C6R	= 0xb,
	_C7	= 0x7,
	_C7S	= 0xc,
	_C8	= 0x8,
	_C9	= 0x9,
	_C10	= 0xa,
	_UNSPEC = 0xf
};

#define CSTATES_ENCODING_COUNT	12

enum THM_POINTS {
	THM_THRESHOLD_1,
	THM_THRESHOLD_2,
	THM_TRIP_LIMIT,
	THM_HTC_LIMIT,
	THM_HTC_HYST,
	THM_POINTS_DIM
};

typedef struct
{
	Bit64		Mask,	/*	1=Thermal Point is specified	*/
			Kind,	/*	0=Threshold ; 1=Limit		*/
			State __attribute__ ((aligned (8))); /*1=Enabled*/
	unsigned short	Value[THM_POINTS_DIM];
} THERMAL_POINT;

enum EVENT_LOG {
	/*	MSR_IA32_{PACKAGE}_THERM_STATUS:	x8		*/
	LSHIFT_THERMAL_LOG,
	LSHIFT_PROCHOT_LOG,
	LSHIFT_CRITIC_LOG,
	LSHIFT_THOLD1_LOG,
	LSHIFT_THOLD2_LOG,
	LSHIFT_POWER_LIMIT,
	LSHIFT_CURRENT_LIMIT,
	LSHIFT_CROSS_DOMAIN,
	/*	MSR_SKL_CORE_PERF_LIMIT_REASONS:	x11		*/
	LSHIFT_CORE_HOT_LOG,
	LSHIFT_CORE_THM_LOG,
	LSHIFT_CORE_RES_LOG,
	LSHIFT_CORE_AVG_LOG,
	LSHIFT_CORE_VRT_LOG,
	LSHIFT_CORE_TDC_LOG,
	LSHIFT_CORE_PL1_LOG,
	LSHIFT_CORE_PL2_LOG,
	LSHIFT_CORE_EDP_LOG,
	LSHIFT_CORE_BST_LOG,
	LSHIFT_CORE_ATT_LOG,
	LSHIFT_CORE_TVB_LOG,
	/*	MSR_GRAPHICS_PERF_LIMIT_REASONS:	x9		*/
	LSHIFT_GFX_HOT_LOG,
	LSHIFT_GFX_THM_LOG,
	LSHIFT_GFX_AVG_LOG,
	LSHIFT_GFX_VRT_LOG,
	LSHIFT_GFX_TDC_LOG,
	LSHIFT_GFX_PL1_LOG,
	LSHIFT_GFX_PL2_LOG,
	LSHIFT_GFX_EDP_LOG,
	LSHIFT_GFX_EFF_LOG,
	/*	MSR_RING_PERF_LIMIT_REASONS:		x8		*/
	LSHIFT_RING_HOT_LOG,
	LSHIFT_RING_THM_LOG,
	LSHIFT_RING_AVG_LOG,
	LSHIFT_RING_VRT_LOG,
	LSHIFT_RING_TDC_LOG,
	LSHIFT_RING_PL1_LOG,
	LSHIFT_RING_PL2_LOG,
	LSHIFT_RING_EDP_LOG
};

enum EVENT_STS {
	/*	MSR_IA32_{PACKAGE}_THERM_STATUS:	x4		*/
	LSHIFT_THERMAL_STS,
	LSHIFT_PROCHOT_STS,
	LSHIFT_CRITIC_TMP,
	LSHIFT_THOLD1_STS,
	LSHIFT_THOLD2_STS,
	/*	MSR_SKL_CORE_PERF_LIMIT_REASONS:	x11		*/
	LSHIFT_CORE_THM_STS,
	LSHIFT_CORE_HOT_STS,
	LSHIFT_CORE_RES_STS,
	LSHIFT_CORE_AVG_STS,
	LSHIFT_CORE_VRT_STS,
	LSHIFT_CORE_TDC_STS,
	LSHIFT_CORE_PL1_STS,
	LSHIFT_CORE_PL2_STS,
	LSHIFT_CORE_EDP_STS,
	LSHIFT_CORE_BST_STS,
	LSHIFT_CORE_ATT_STS,
	LSHIFT_CORE_TVB_STS,
	/*	MSR_GRAPHICS_PERF_LIMIT_REASONS:	x9		*/
	LSHIFT_GFX_THM_STS,
	LSHIFT_GFX_HOT_STS,
	LSHIFT_GFX_AVG_STS,
	LSHIFT_GFX_VRT_STS,
	LSHIFT_GFX_TDC_STS,
	LSHIFT_GFX_PL1_STS,
	LSHIFT_GFX_PL2_STS,
	LSHIFT_GFX_EDP_STS,
	LSHIFT_GFX_EFF_STS,
	/*	MSR_RING_PERF_LIMIT_REASONS:		x8		*/
	LSHIFT_RING_THM_STS,
	LSHIFT_RING_HOT_STS,
	LSHIFT_RING_AVG_STS,
	LSHIFT_RING_VRT_STS,
	LSHIFT_RING_TDC_STS,
	LSHIFT_RING_PL1_STS,
	LSHIFT_RING_PL2_STS,
	LSHIFT_RING_EDP_STS
};

enum {
	eLOG,
	eSTS,
	eDIM
};

enum THERM_PWR_EVENTS {
	EVENT_THERM_NONE	= 0x0LLU,
	/*	MSR_IA32_{PACKAGE}_THERM_STATUS:			*/
	EVENT_THERMAL_STS	= 0x1LLU << LSHIFT_THERMAL_STS,
	EVENT_THERMAL_LOG	= 0x1LLU << LSHIFT_THERMAL_LOG,
	EVENT_PROCHOT_STS	= 0x1LLU << LSHIFT_PROCHOT_STS,
	EVENT_PROCHOT_LOG	= 0x1LLU << LSHIFT_PROCHOT_LOG,
	EVENT_CRITIC_TMP	= 0x1LLU << LSHIFT_CRITIC_TMP,
	EVENT_CRITIC_LOG	= 0x1LLU << LSHIFT_CRITIC_LOG,
	EVENT_THOLD1_STS	= 0x1LLU << LSHIFT_THOLD1_STS,
	EVENT_THOLD2_STS	= 0x1LLU << LSHIFT_THOLD2_STS,
	EVENT_THOLD1_LOG	= 0x1LLU << LSHIFT_THOLD1_LOG,
	EVENT_THOLD2_LOG	= 0x1LLU << LSHIFT_THOLD2_LOG,
	EVENT_POWER_LIMIT	= 0x1LLU << LSHIFT_POWER_LIMIT,
	EVENT_CURRENT_LIMIT	= 0x1LLU << LSHIFT_CURRENT_LIMIT,
	EVENT_CROSS_DOMAIN	= 0x1LLU << LSHIFT_CROSS_DOMAIN,
	/*	MSR_SKL_CORE_PERF_LIMIT_REASONS:			*/
	EVENT_CORE_THM_STS	= 0x1LLU << LSHIFT_CORE_THM_STS,
	EVENT_CORE_HOT_STS	= 0x1LLU << LSHIFT_CORE_HOT_STS,
	EVENT_CORE_HOT_LOG	= 0x1LLU << LSHIFT_CORE_HOT_LOG,
	EVENT_CORE_THM_LOG	= 0x1LLU << LSHIFT_CORE_THM_LOG,
	EVENT_CORE_RES_STS	= 0x1LLU << LSHIFT_CORE_RES_STS,
	EVENT_CORE_RES_LOG	= 0x1LLU << LSHIFT_CORE_RES_LOG,
	EVENT_CORE_AVG_STS	= 0x1LLU << LSHIFT_CORE_AVG_STS,
	EVENT_CORE_AVG_LOG	= 0x1LLU << LSHIFT_CORE_AVG_LOG,
	EVENT_CORE_VRT_STS	= 0x1LLU << LSHIFT_CORE_VRT_STS,
	EVENT_CORE_VRT_LOG	= 0x1LLU << LSHIFT_CORE_VRT_LOG,
	EVENT_CORE_TDC_STS	= 0x1LLU << LSHIFT_CORE_TDC_STS,
	EVENT_CORE_TDC_LOG	= 0x1LLU << LSHIFT_CORE_TDC_LOG,
	EVENT_CORE_PL1_STS	= 0x1LLU << LSHIFT_CORE_PL1_STS,
	EVENT_CORE_PL1_LOG	= 0x1LLU << LSHIFT_CORE_PL1_LOG,
	EVENT_CORE_PL2_STS	= 0x1LLU << LSHIFT_CORE_PL2_STS,
	EVENT_CORE_PL2_LOG	= 0x1LLU << LSHIFT_CORE_PL2_LOG,
	EVENT_CORE_EDP_STS	= 0x1LLU << LSHIFT_CORE_EDP_STS,
	EVENT_CORE_EDP_LOG	= 0x1LLU << LSHIFT_CORE_EDP_LOG,
	EVENT_CORE_BST_STS	= 0x1LLU << LSHIFT_CORE_BST_STS,
	EVENT_CORE_BST_LOG	= 0x1LLU << LSHIFT_CORE_BST_LOG,
	EVENT_CORE_ATT_STS	= 0x1LLU << LSHIFT_CORE_ATT_STS,
	EVENT_CORE_ATT_LOG	= 0x1LLU << LSHIFT_CORE_ATT_LOG,
	EVENT_CORE_TVB_STS	= 0x1LLU << LSHIFT_CORE_TVB_STS,
	EVENT_CORE_TVB_LOG	= 0x1LLU << LSHIFT_CORE_TVB_LOG,
	/*	MSR_GRAPHICS_PERF_LIMIT_REASONS:			*/
	EVENT_GFX_THM_STS	= 0x1LLU << LSHIFT_GFX_THM_STS,
	EVENT_GFX_HOT_STS	= 0x1LLU << LSHIFT_GFX_HOT_STS,
	EVENT_GFX_HOT_LOG	= 0x1LLU << LSHIFT_GFX_HOT_LOG,
	EVENT_GFX_THM_LOG	= 0x1LLU << LSHIFT_GFX_THM_LOG,
	EVENT_GFX_AVG_STS	= 0x1LLU << LSHIFT_GFX_AVG_STS,
	EVENT_GFX_AVG_LOG	= 0x1LLU << LSHIFT_GFX_AVG_LOG,
	EVENT_GFX_VRT_STS	= 0x1LLU << LSHIFT_GFX_VRT_STS,
	EVENT_GFX_VRT_LOG	= 0x1LLU << LSHIFT_GFX_VRT_LOG,
	EVENT_GFX_TDC_STS	= 0x1LLU << LSHIFT_GFX_TDC_STS,
	EVENT_GFX_TDC_LOG	= 0x1LLU << LSHIFT_GFX_TDC_LOG,
	EVENT_GFX_PL1_STS	= 0x1LLU << LSHIFT_GFX_PL1_STS,
	EVENT_GFX_PL1_LOG	= 0x1LLU << LSHIFT_GFX_PL1_LOG,
	EVENT_GFX_PL2_STS	= 0x1LLU << LSHIFT_GFX_PL2_STS,
	EVENT_GFX_PL2_LOG	= 0x1LLU << LSHIFT_GFX_PL2_LOG,
	EVENT_GFX_EDP_STS	= 0x1LLU << LSHIFT_GFX_EDP_STS,
	EVENT_GFX_EDP_LOG	= 0x1LLU << LSHIFT_GFX_EDP_LOG,
	EVENT_GFX_EFF_STS	= 0x1LLU << LSHIFT_GFX_EFF_STS,
	EVENT_GFX_EFF_LOG	= 0x1LLU << LSHIFT_GFX_EFF_LOG,
	/*	MSR_RING_PERF_LIMIT_REASONS:				*/
	EVENT_RING_THM_STS	= 0x1LLU << LSHIFT_RING_THM_STS,
	EVENT_RING_HOT_STS	= 0x1LLU << LSHIFT_RING_HOT_STS,
	EVENT_RING_HOT_LOG	= 0x1LLU << LSHIFT_RING_HOT_LOG,
	EVENT_RING_THM_LOG	= 0x1LLU << LSHIFT_RING_THM_LOG,
	EVENT_RING_AVG_STS	= 0x1LLU << LSHIFT_RING_AVG_STS,
	EVENT_RING_AVG_LOG	= 0x1LLU << LSHIFT_RING_AVG_LOG,
	EVENT_RING_VRT_STS	= 0x1LLU << LSHIFT_RING_VRT_STS,
	EVENT_RING_VRT_LOG	= 0x1LLU << LSHIFT_RING_VRT_LOG,
	EVENT_RING_TDC_STS	= 0x1LLU << LSHIFT_RING_TDC_STS,
	EVENT_RING_TDC_LOG	= 0x1LLU << LSHIFT_RING_TDC_LOG,
	EVENT_RING_PL1_STS	= 0x1LLU << LSHIFT_RING_PL1_STS,
	EVENT_RING_PL1_LOG	= 0x1LLU << LSHIFT_RING_PL1_LOG,
	EVENT_RING_PL2_STS	= 0x1LLU << LSHIFT_RING_PL2_STS,
	EVENT_RING_PL2_LOG	= 0x1LLU << LSHIFT_RING_PL2_LOG,
	EVENT_RING_EDP_STS	= 0x1LLU << LSHIFT_RING_EDP_STS,
	EVENT_RING_EDP_LOG	= 0x1LLU << LSHIFT_RING_EDP_LOG,
	/*	ALL EVENTS						*/
	EVENT_ALL_OF_THEM	= EVENT_RING_EDP_LOG << 1
};

enum SENSOR_LIMITS {
	SENSOR_LOWEST,
	SENSOR_HIGHEST,
	SENSOR_LIMITS_DIM
};

enum THERMAL_OFFSET {
	THERMAL_TARGET,
	THERMAL_OFFSET_P1,
	THERMAL_OFFSET_P2,
	THERMAL_OFFSET_DIM
};

typedef union
{
	unsigned long	Target;
	unsigned short	Offset[THERMAL_OFFSET_DIM];
} THERMAL_PARAM;

enum FORMULA_SCOPE {
	FORMULA_SCOPE_NONE	= 0,
	FORMULA_SCOPE_SMT	= 1,
	FORMULA_SCOPE_CORE	= 2,
	FORMULA_SCOPE_PKG	= 3
};

enum THERMAL_KIND {
	THERMAL_KIND_NONE	= 0b000000000000000000000000,
	THERMAL_KIND_INTEL	= 0b000000000000000000000001,
	THERMAL_KIND_AMD	= 0b000000000001000000000000,
	THERMAL_KIND_AMD_0Fh	= 0b000000000011000000000000,
	THERMAL_KIND_AMD_15h	= 0b000001000001000000000000,
	THERMAL_KIND_AMD_17h	= 0b000100000001000000000000
};

enum THERMAL_FORMULAS {
THERMAL_FORMULA_NONE	= (THERMAL_KIND_NONE << 8)	| FORMULA_SCOPE_NONE,
THERMAL_FORMULA_INTEL	= (THERMAL_KIND_INTEL << 8)	| FORMULA_SCOPE_SMT,
THERMAL_FORMULA_AMD	= (THERMAL_KIND_AMD << 8)	| FORMULA_SCOPE_SMT,
THERMAL_FORMULA_AMD_0Fh = (THERMAL_KIND_AMD_0Fh << 8)	| FORMULA_SCOPE_SMT,
THERMAL_FORMULA_AMD_15h = (THERMAL_KIND_AMD_15h << 8)	| FORMULA_SCOPE_CORE,
THERMAL_FORMULA_AMD_17h = (THERMAL_KIND_AMD_17h << 8)	| FORMULA_SCOPE_PKG,
THERMAL_FORMULA_AMD_ZEN2= (THERMAL_KIND_AMD_17h << 8)	| FORMULA_SCOPE_SMT
};

#define THERMAL_FORMULA_AMD_19h THERMAL_FORMULA_AMD_17h
#define THERMAL_FORMULA_AMD_ZEN3 THERMAL_FORMULA_AMD_ZEN2
#define THERMAL_FORMULA_AMD_ZEN4 THERMAL_FORMULA_AMD_ZEN3

enum VOLTAGE_KIND {
	VOLTAGE_KIND_NONE	= 0b000000000000000000000000,
	VOLTAGE_KIND_INTEL	= 0b000000000000000000000001,
	VOLTAGE_KIND_INTEL_CORE2= 0b000000000000000000000011,
	VOLTAGE_KIND_INTEL_SOC	= 0b000000000000000000000111,
	VOLTAGE_KIND_INTEL_SNB	= 0b000000000000000010000001,
	VOLTAGE_KIND_INTEL_SKL_X= 0b000000000000010000000001,
	VOLTAGE_KIND_INTEL_SAV	= 0b000000000000000100000001,
	VOLTAGE_KIND_AMD	= 0b000000000001000000000000,
	VOLTAGE_KIND_AMD_0Fh	= 0b000000000011000000000000,
	VOLTAGE_KIND_AMD_15h	= 0b000001000001000000000000,
	VOLTAGE_KIND_AMD_17h	= 0b000100000001000000000000,
	VOLTAGE_KIND_AMD_RMB	= 0b000010000001000000000000,
	VOLTAGE_KIND_WINBOND_IO = 0b001000000000000000000000,
	VOLTAGE_KIND_ITETECH_IO = 0b010000000000000000000000
};

enum VOLTAGE_FORMULAS {
VOLTAGE_FORMULA_NONE       =(VOLTAGE_KIND_NONE << 8)       | FORMULA_SCOPE_NONE,
VOLTAGE_FORMULA_INTEL      =(VOLTAGE_KIND_INTEL << 8)      | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_CORE2=(VOLTAGE_KIND_INTEL_CORE2 << 8)| FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_SOC  =(VOLTAGE_KIND_INTEL_SOC << 8)  | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_SNB  =(VOLTAGE_KIND_INTEL_SNB << 8)  | FORMULA_SCOPE_PKG,
VOLTAGE_FORMULA_INTEL_SNB_E=(VOLTAGE_KIND_INTEL_SNB << 8)  | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_SKL_X=(VOLTAGE_KIND_INTEL_SKL_X << 8)| FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_SAV  =(VOLTAGE_KIND_INTEL_SAV << 8)  | FORMULA_SCOPE_CORE,
VOLTAGE_FORMULA_AMD        =(VOLTAGE_KIND_AMD << 8)        | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_0Fh    =(VOLTAGE_KIND_AMD_0Fh << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_15h    =(VOLTAGE_KIND_AMD_15h << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_17h    =(VOLTAGE_KIND_AMD_17h << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_RMB    =(VOLTAGE_KIND_AMD_RMB << 8)    | FORMULA_SCOPE_PKG,
VOLTAGE_FORMULA_AMD_ZEN4   =(VOLTAGE_KIND_AMD_RMB << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_WINBOND_IO =(VOLTAGE_KIND_WINBOND_IO << 8) | FORMULA_SCOPE_PKG,
VOLTAGE_FORMULA_ITETECH_IO =(VOLTAGE_KIND_ITETECH_IO << 8) | FORMULA_SCOPE_PKG
};

#define VOLTAGE_FORMULA_AMD_19h VOLTAGE_FORMULA_AMD_17h

enum POWER_KIND {
	POWER_KIND_NONE 	= 0b000000000000000000000000,
	POWER_KIND_INTEL	= 0b000000000000000000000001,
	POWER_KIND_INTEL_ATOM	= 0b000000000000000000000011,
	POWER_KIND_AMD		= 0b000000000001000000000000,
	POWER_KIND_AMD_17h	= 0b000100000001000000000000
};

enum POWER_FORMULAS {
POWER_FORMULA_NONE	=(POWER_KIND_NONE << 8) 	| FORMULA_SCOPE_NONE,
POWER_FORMULA_INTEL	=(POWER_KIND_INTEL << 8)	| FORMULA_SCOPE_NONE,
POWER_FORMULA_INTEL_ATOM=(POWER_KIND_INTEL_ATOM << 8)	| FORMULA_SCOPE_NONE,
POWER_FORMULA_AMD	=(POWER_KIND_AMD << 8)		| FORMULA_SCOPE_CORE,
POWER_FORMULA_AMD_17h	=(POWER_KIND_AMD_17h << 8)	| FORMULA_SCOPE_CORE
};

#define POWER_FORMULA_AMD_19h POWER_FORMULA_AMD_17h

#define SCOPE_OF_FORMULA(formula)	(formula & 0b0011)

#define KIND_OF_FORMULA(formula) ((formula >> 8) & 0b111111111111111111111111)

/* Sensors formulas and definitions.
  MIN = [SENSOR] > [TRIGGER] AND ([SENSOR] < [LOWEST] OR [LOWEST] <= [CAPPED])
  MAX = [SENSOR] > [HIGHEST]
*/

#define THRESHOLD_LOWEST_CAPPED_THERMAL 	1
#define THRESHOLD_LOWEST_CAPPED_VOLTAGE 	0.15
#define THRESHOLD_LOWEST_CAPPED_ENERGY		0.000001
#define THRESHOLD_LOWEST_CAPPED_POWER		0.000001
#define THRESHOLD_LOWEST_CAPPED_REL_FREQ	0.0
#define THRESHOLD_LOWEST_CAPPED_ABS_FREQ	0.0

#define THRESHOLD_LOWEST_TRIGGER_THERMAL	0
#define THRESHOLD_LOWEST_TRIGGER_VOLTAGE	0.0
#define THRESHOLD_LOWEST_TRIGGER_ENERGY 	0.0
#define THRESHOLD_LOWEST_TRIGGER_POWER		0.0
#define THRESHOLD_LOWEST_TRIGGER_REL_FREQ	0.0
#define THRESHOLD_LOWEST_TRIGGER_ABS_FREQ	0.0

#define _RESET_SENSOR_LIMIT(THRESHOLD, Limit)				\
({									\
	Limit =  THRESHOLD;						\
})

#define RESET_SENSOR_LOWEST(CLASS, Limit)				\
	_RESET_SENSOR_LIMIT(THRESHOLD_LOWEST_CAPPED_##CLASS,		\
				Limit[SENSOR_LOWEST])

#define RESET_SENSOR_HIGHEST(CLASS, Limit)				\
	_RESET_SENSOR_LIMIT(THRESHOLD_LOWEST_TRIGGER_##CLASS,		\
				Limit[SENSOR_HIGHEST])

#define RESET_SENSOR_LIMIT(CLASS, STAT, Limit)				\
	RESET_SENSOR_##STAT(CLASS, Limit)

#define TEST_SENSOR_LOWEST(CLASS, TRIGGER, CAPPED, Sensor, Limit)	\
	(Sensor > TRIGGER##CLASS) 					\
	&& ((Sensor < Limit) || (Limit <= CAPPED##CLASS))

#define TEST_SENSOR_HIGHEST(CLASS, TRIGGER, CAPPED, Sensor, Limit)	\
	(Sensor > Limit)

#define _TEST_SENSOR(CLASS, STAT, THRESHOLD, TRIGGER, CAPPED, Sensor, Limit) \
	TEST_SENSOR_##STAT(CLASS, THRESHOLD##TRIGGER, THRESHOLD##CAPPED, \
				Sensor, Limit)

#define TEST_SENSOR(CLASS, STAT, Sensor, Limit) 			\
	_TEST_SENSOR(CLASS, STAT, THRESHOLD_##STAT, _TRIGGER_, _CAPPED_, \
			Sensor, Limit[SENSOR_##STAT])

#define TEST_AND_SET_SENSOR(CLASS, STAT, Sensor, Limit) 		\
({									\
	if (TEST_SENSOR(CLASS, STAT, Sensor, Limit))			\
	{								\
		Limit[SENSOR_##STAT] = Sensor;				\
	}								\
})

#define COMPUTE_THERMAL_INVERSE_INTEL(Sensor, Param, Temp)		\
	(Sensor = Param.Offset[THERMAL_TARGET]				\
		- Param.Offset[THERMAL_OFFSET_P1] - Temp)

#define COMPUTE_THERMAL_INTEL(Temp, Param, Sensor)			\
	(Temp	= Param.Offset[THERMAL_TARGET]				\
		- Param.Offset[THERMAL_OFFSET_P1] - Sensor)

#define COMPUTE_THERMAL_AMD(Temp, Param, Sensor)			\
	UNUSED(Param);							\
	UNUSED(Sensor);							\
	/*( TODO )*/

#define COMPUTE_THERMAL_AMD_0Fh(Temp, Param, Sensor)			\
	(Temp = Sensor - (Param.Target * 2) - 49)

#define COMPUTE_THERMAL_AMD_15h(Temp, Param, Sensor)			\
	UNUSED(Param);							\
	(Temp = Sensor * 5 / 40)

#define COMPUTE_THERMAL_AMD_17h(Temp, Param, Sensor)			\
	(Temp	= ((Sensor * 5 / 40) - Param.Offset[THERMAL_OFFSET_P1]) \
		- Param.Offset[THERMAL_OFFSET_P2])

#define COMPUTE_THERMAL(_ARCH_, Temp, Param, Sensor)			\
	COMPUTE_THERMAL_##_ARCH_(Temp, Param, Sensor)

#define COMPUTE_VOLTAGE_INTEL_CORE2(Vcore, VID) 			\
		(Vcore = 0.8875 + (double) (VID) * 0.0125)

#define COMPUTE_VOLTAGE_INTEL_SOC(Vcore, VID) 				\
({									\
	switch (VID) {							\
	case 0x00:							\
		Vcore = 0.0f;						\
		break;							\
	case 0xfc:							\
	case 0xfe:							\
		Vcore = 1.495f;						\
		break;							\
	case 0xfd:							\
	case 0xff:							\
		Vcore = 1.5f;						\
		break;							\
	default:							\
		Vcore = 0.245 + (double) (VID) * 0.005;			\
		break;							\
	}								\
})

#define COMPUTE_VOLTAGE_INTEL_SNB(Vcore, VID) 				\
		(Vcore = (double) (VID) / 8192.0)

#define COMPUTE_VOLTAGE_INTEL_SKL_X(Vcore, VID) 			\
		(Vcore = (double) (VID) / 8192.0)

#define COMPUTE_VOLTAGE_INTEL_SAV COMPUTE_VOLTAGE_INTEL_SNB

#define COMPUTE_VOLTAGE_AMD(Vcore, VID)					\
		/*( TODO )*/

#define COMPUTE_VOLTAGE_AMD_0Fh(Vcore, VID)				\
({									\
	short	Vselect =(VID & 0b110000) >> 4, Vnibble = VID & 0b1111; \
									\
	switch (Vselect) {						\
	case 0b00:							\
		Vcore = 1.550 - (double) (Vnibble) * 0.025;		\
		break;							\
	case 0b01:							\
		Vcore = 1.150 - (double) (Vnibble) * 0.025;		\
		break;							\
	case 0b10:							\
		Vcore = 0.7625 - (double) (Vnibble) * 0.0125;		\
		break;							\
	case 0b11:							\
		Vcore = 0.5625 - (double) (Vnibble) * 0.0125;		\
		break;							\
	}								\
})

#define COMPUTE_VOLTAGE_AMD_15h(Vcore, VID)				\
		(Vcore = 1.550 - (0.00625 * (double) (VID)))

#define COMPUTE_VOLTAGE_AMD_17h(Vcore, VID)				\
		(Vcore = 1.550 - (0.00625 * (double) (VID)))

#define COMPUTE_VOLTAGE_AMD_RMB(Vcore, VID)				\
		(Vcore = 0.00625 * (double) (VID))

#define COMPUTE_VOLTAGE_WINBOND_IO(Vcore, VID)				\
		(Vcore = (double) (VID) * 0.008)

#define COMPUTE_VOLTAGE_ITETECH_IO(Vcore, VID)				\
		(Vcore = (double) (VID) * 0.016)

#define COMPUTE_VOLTAGE(_ARCH_, Vcore, VID)				\
		COMPUTE_VOLTAGE_##_ARCH_(Vcore, VID)

#define COMPUTE_TAU(Y, Z, TU) ( 					\
	(1LLU << Y) * (1.0 + Z / 4.0) * TU				\
)

#define COMPUTE_TW(Y, Z) (						\
	((unsigned char) Z << 5) | (unsigned char) Y			\
)

enum PWR_LIMIT {
	PL1 = 0,
	PL2 = 1,
	PWR_LIMIT_SIZE
};

enum PWR_DOMAIN {
	DOMAIN_PKG	= 0,
	DOMAIN_CORES	= 1,
	DOMAIN_UNCORE	= 2,
	DOMAIN_RAM	= 3,
	DOMAIN_PLATFORM = 4,
	DOMAIN_SIZE
};

#define PWR_DOMAIN(NC) DOMAIN_##NC

enum RATIO_BOOST {
	RATIO_MIN,
	RATIO_MAX,
	RATIO_TGT,
	RATIO_ACT,
	RATIO_TDP,
	RATIO_TDP1,
	RATIO_TDP2,
	RATIO_CPB = RATIO_TDP1,
	RATIO_XFR = RATIO_TDP2,
	RATIO_HWP_MIN,
	RATIO_HWP_MAX,
	RATIO_HWP_TGT,
	RATIO_18C,
	RATIO_17C,
	RATIO_16C,
	RATIO_15C,
	RATIO_14C,
	RATIO_13C,
	RATIO_12C,
	RATIO_11C,
	RATIO_10C,
	RATIO_9C,
	RATIO_8C,
	RATIO_7C,
	RATIO_6C,
	RATIO_5C,
	RATIO_4C,
	RATIO_3C,
	RATIO_2C,
	RATIO_1C,
	RATIO_SIZE
};

#define BOOST(NC) RATIO_##NC

enum UNCORE_BOOST {
	UNCORE_RATIO_MIN,
	UNCORE_RATIO_MAX,
	UNCORE_RATIO_SIZE
};

#define UNCORE_BOOST(NC) UNCORE_RATIO_##NC

#define CACHE_MAX_LEVEL (3 + 1)

#define PRECISION	100

#define UNIT_KHz(_f)		(_f * 10 * PRECISION)
#define UNIT_MHz(_f)		(_f * UNIT_KHz(1000))
#define UNIT_GHz(_f)		(_f * UNIT_MHz(1000))
#define CLOCK_KHz(_t, _f)	((_t)(_f) / (_t)UNIT_KHz(1))
#define CLOCK_MHz(_t, _f)	((_t)(_f) / (_t)UNIT_MHz(1))
#define CLOCK_GHz(_t, _f)	((_t)(_f) / (_t)UNIT_GHz(1))

#if !defined(MAX_FREQ_HZ)
	#define MAX_FREQ_HZ	6575000000
#elif (MAX_FREQ_HZ < 4850000000)
	#error "MAX_FREQ_HZ must be at least 4850000000 Hz"
#endif

#define MAXCLOCK_TO_RATIO(_typeout, BaseClock)				\
	( (_typeout) (MAX_FREQ_HZ / BaseClock) )

enum OFFLINE
{
	HW,
	OS
};

typedef struct
{
	unsigned short	Q,
			R;
} COF_ST;

typedef union {
	unsigned int	Perf;	/* STATUS or BOOST P-State */
	COF_ST		COF;
} COF_UNION;

typedef struct
{
	unsigned long long	Q,
				R,
				Hz;
} CLOCK;

#define REL_BCLK(clock, ratio, delta_tsc, interval)			\
({	/*		Compute Clock (Hertz)			*/	\
	clock.Hz = (1000LLU * delta_tsc) / (interval * ratio);		\
	/*		Compute Quotient (MHz)			*/	\
	clock.Q  = clock.Hz / (1000LLU * 1000LLU);			\
	/*		Compute Remainder (MHz)			*/	\
	clock.R  = clock.Hz % (1000LLU * 1000LLU);			\
})

#define REL_FREQ_MHz(this_type, this_ratio, clock, interval)		\
	( (this_type)(clock.Hz)						\
	* (this_type)(this_ratio)					\
	* (this_type)(interval))					\
	/ (this_type)UNIT_MHz(interval)

#define ABS_FREQ_MHz(this_type, this_ratio, this_clock) 		\
(									\
	CLOCK_MHz(this_type, this_ratio * this_clock.Hz)		\
)

typedef union {
	signed long long	sllong;
	unsigned long long	ullong;
	struct {
	    struct {
		union {
		signed short	Offset;
		signed short	Ratio;
		};
		signed short	cpu;
	    };
		unsigned int	NC;
	};
} CLOCK_ARG;

enum CLOCK_MOD_INDEX {
	CLOCK_MOD_ACT = 7,
	CLOCK_MOD_HWP_MIN = 6,
	CLOCK_MOD_HWP_MAX = 5,
	CLOCK_MOD_HWP_TGT = 4,
	CLOCK_MOD_MIN = 3,
	CLOCK_MOD_MAX = 2,
	CLOCK_MOD_TGT = 1
};

enum {	/* Stick to the Kernel enumeration in include/asm/nmi.h		*/
	BIT_NMI_LOCAL = 0,
	BIT_NMI_UNKNOWN,
	BIT_NMI_SERR,
	BIT_NMI_IO_CHECK
};

#define BIT_NMI_MASK	0x0fLLU

typedef union
{
	signed long long	Proc;
	struct {
		unsigned int	Core;
	    struct {
		signed short	Hybrid,
				Thread;
	    };
	};
} SERVICE_PROC;

#define RESET_SERVICE	{.Core = -1U, .Thread = -1, .Hybrid = -1}

enum CPUID_ENUM {
	CPUID_00000001_00000000_INSTRUCTION_SET,
/* Intel */
	CPUID_00000002_00000000_CACHE_AND_TLB,
	CPUID_00000003_00000000_PROC_SERIAL_NUMBER,
	CPUID_00000004_00000000_CACHE_L1I,
	CPUID_00000004_00000001_CACHE_L1D,
	CPUID_00000004_00000002_CACHE_L2,
	CPUID_00000004_00000003_CACHE_L3,
/* x86 */
	CPUID_00000005_00000000_MONITOR_MWAIT,
	CPUID_00000006_00000000_POWER_AND_THERMAL_MGMT,
	CPUID_00000007_00000000_EXTENDED_FEATURES,
	CPUID_00000007_00000001_EXT_FEAT_SUB_LEAF_1,
	CPUID_00000007_00000001_EXT_FEAT_SUB_LEAF_2,
/* Intel */
	CPUID_00000009_00000000_DIRECT_CACHE_ACCESS,
	CPUID_0000000A_00000000_PERF_MONITORING,
/* x86 */
	CPUID_0000000B_00000000_EXT_TOPOLOGY,
	CPUID_0000000D_00000000_EXT_STATE_MAIN_LEAF,
	CPUID_0000000D_00000001_EXT_STATE_SUB_LEAF,
/* AMD */
	CPUID_0000000D_00000002_EXT_STATE_SUB_LEAF,
	CPUID_0000000D_00000003_BNDREGS_STATE,
	CPUID_0000000D_00000004_BNDCSR_STATE,
/* AMD Family 19h */
	CPUID_0000000D_00000009_MPK_STATE_SUB_LEAF,
	CPUID_0000000D_00000009_CET_U_SUB_LEAF,
	CPUID_0000000D_00000009_CET_S_SUB_LEAF,
/* AMD Family 15h */
	CPUID_0000000D_0000003E_EXT_STATE_SUB_LEAF,
/* Intel */
	CPUID_0000000F_00000000_QOS_MONITORING_CAP,
	CPUID_0000000F_00000001_L3_QOS_MONITORING,
	CPUID_00000010_00000000_QOS_ENFORCEMENT_CAP,
	CPUID_00000010_00000001_L3_ALLOC_ENUMERATION,
	CPUID_00000010_00000002_L2_ALLOC_ENUMERATION,
	CPUID_00000010_00000003_RAM_BANDWIDTH_ENUM,
	CPUID_00000012_00000000_SGX_CAPABILITY,
	CPUID_00000012_00000001_SGX_ATTRIBUTES,
	CPUID_00000012_00000002_SGX_ENCLAVE_PAGE_CACHE,
	CPUID_00000014_00000000_PROCESSOR_TRACE,
	CPUID_00000014_00000001_PROC_TRACE_SUB_LEAF,
	CPUID_00000015_00000000_TIME_STAMP_COUNTER,
	CPUID_00000016_00000000_PROCESSOR_FREQUENCY,
	CPUID_00000017_00000000_SYSTEM_ON_CHIP,
	CPUID_00000017_00000001_SOC_ATTRIB_SUB_LEAF_1,
	CPUID_00000017_00000002_SOC_ATTRIB_SUB_LEAF_2,
	CPUID_00000017_00000003_SOC_ATTRIB_SUB_LEAF_3,
/* Intel */
	CPUID_00000018_00000000_ADDRESS_TRANSLATION,
	CPUID_00000018_00000001_DAT_SUB_LEAF_1,
	CPUID_00000018_00000001_DAT_SUB_LEAF_2,
	CPUID_00000018_00000001_DAT_SUB_LEAF_3,
	CPUID_00000018_00000001_DAT_SUB_LEAF_4,
	CPUID_00000019_00000000_KEY_LOCKER,
	CPUID_0000001A_00000000_HYBRID_INFORMATION,
	CPUID_0000001B_00000000_PCONFIG_INFORMATION,
	CPUID_0000001F_00000000_EXT_TOPOLOGY_V2,
/* x86 */
	CPUID_80000001_00000000_EXTENDED_FEATURES,
	CPUID_80000002_00000000_PROCESSOR_NAME_ID,
	CPUID_80000003_00000000_PROCESSOR_NAME_ID,
	CPUID_80000004_00000000_PROCESSOR_NAME_ID,
/* AMD */
	CPUID_80000005_00000000_CACHES_L1D_L1I_TLB,
/* x86 */
	CPUID_80000006_00000000_CACHE_L2_SIZE_WAY,
	CPUID_80000007_00000000_ADVANCED_POWER_MGMT,
	CPUID_80000008_00000000_LM_ADDRESS_SIZE,
/* AMD */
	CPUID_8000000A_00000000_SVM_REVISION,
	CPUID_80000019_00000000_CACHES_AND_TLB_1G,
	CPUID_8000001A_00000000_PERF_OPTIMIZATION,
	CPUID_8000001B_00000000_INST_BASED_SAMPLING,
	CPUID_8000001C_00000000_LIGHTWEIGHT_PROFILING,
	CPUID_8000001D_00000000_CACHE_L1D_PROPERTIES,
	CPUID_8000001D_00000001_CACHE_L1I_PROPERTIES,
	CPUID_8000001D_00000002_CACHE_L2_PROPERTIES,
	CPUID_8000001D_00000003_CACHE_PROPERTIES_END,
	CPUID_8000001D_00000004_CACHE_PROPERTIES_DONE,
	CPUID_8000001E_00000000_EXTENDED_IDENTIFIERS,
/* AMD Family 17h */
	CPUID_8000001F_00000000_SECURE_ENCRYPTION,
	CPUID_80000020_00000000_MBE_SUB_LEAF,
	CPUID_80000020_00000001_MBE_SUB_LEAF,
/* AMD Family 19h Model 11h, Revision B1 */
	CPUID_80000020_00000002_SMBE_SUB_LEAF,
	CPUID_80000020_00000003_BMEC_SUB_LEAF,
/* AMD Family 19h */
	CPUID_80000021_00000000_EXTENDED_FEATURE_2,
	CPUID_80000022_00000000_EXT_PERF_MON_DEBUG,
/* AMD64 Architecture Programmer’s Manual rev 4.05 */
	CPUID_80000023_00000000_MULTIKEY_ENCRYPTED_MEM,
	CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L0,
	CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L1,
	CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L2,
	CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L3,
/* x86 */
	CPUID_40000000_00000000_HYPERVISOR_VENDOR,
	CPUID_40000001_00000000_HYPERVISOR_INTERFACE,
	CPUID_40000002_00000000_HYPERVISOR_VERSION,
	CPUID_40000003_00000000_HYPERVISOR_FEATURES,
	CPUID_40000004_00000000_HYPERV_REQUIREMENTS,
	CPUID_40000005_00000000_HYPERVISOR_LIMITS,
	CPUID_40000006_00000000_HYPERVISOR_EXPLOITS,

	CPUID_MAX_FUNC
};

enum {
	REG_CPUID_EAX,
	REG_CPUID_EBX,
	REG_CPUID_ECX,
	REG_CPUID_EDX
};

typedef struct
{
	unsigned int	func,
			sub,
			reg[4];
} CPUID_STRUCT;

#define BRAND_PART	12
#define BRAND_LENGTH	(4 * BRAND_PART)
#define BRAND_SIZE	(BRAND_LENGTH + 4)

typedef struct
{		/* Common x86						*/
	unsigned int		LargestStdFunc, /* Largest Standard CPUID */
				LargestExtFunc, /* Largest Extended CPUID */
				LargestHypFunc; /* Largest Hypervisor CPUID */
	struct {
	enum CRC_MANUFACTURER	CRC;
		char		ID[12 + 4];
	} Vendor, Hypervisor;
	char			Brand[BRAND_SIZE];
} CPUID_FUNCTION;

typedef struct	/* Basic CPUID Function.				*/
{
		unsigned int LargestStdFunc, BX, CX, DX;
} CPUID_0x00000000;

typedef struct
{
	union
	{
	    struct SIGNATURE
	    {
		unsigned int
		Stepping	:  4-0,
		Model		:  8-4,
		Family		: 12-8,
		ProcType	: 14-12,
		Reserved1	: 16-14,
		ExtModel	: 20-16,
		ExtFamily	: 28-20,
		Reserved2	: 32-28;
	    } EAX;
		unsigned int Signature;
	};
	struct CPUID_0x00000001_EBX
	{
		unsigned int
		Brand_ID	:  8-0,
		CLFSH_Size	: 16-8,
		Max_SMT_ID	: 24-16,
		Init_APIC_ID	: 32-24;
	} EBX;
	struct
	{
		unsigned int
		SSE3		:  1-0,  /* AMD Family 0Fh		*/
		PCLMULDQ	:  2-1,
		DTES64		:  3-2,
		MONITOR 	:  4-3,
		DS_CPL		:  5-4,
		VMX		:  6-5,
		SMX		:  7-6,
		EIST		:  8-7,
		TM2		:  9-8,
		SSSE3		: 10-9,  /* AMD Family 0Fh		*/
		CNXT_ID 	: 11-10,
		SDBG		: 12-11, /* IA32_DEBUG_INTERFACE MSR support */
		FMA		: 13-12,
		CMPXCHG16	: 14-13,
		xTPR		: 15-14,
		PDCM		: 16-15,
		Reserved	: 17-16,
		PCID		: 18-17,
		DCA		: 19-18,
		SSE41		: 20-19,
		SSE42		: 21-20,
		x2APIC		: 22-21, /* x2APIC capability		*/
		MOVBE		: 23-22,
		POPCNT		: 24-23,
		TSC_DEADLINE	: 25-24,
		AES		: 26-25,
		XSAVE		: 27-26,
		OSXSAVE 	: 28-27,
		AVX		: 29-28,
		F16C		: 30-29,
		RDRAND		: 31-30,
		Hyperv		: 32-31; /* This bit is set by the Hypervisor */
	} ECX;
	struct
	{	/* Most common x86					*/
		unsigned int
		FPU		:  1-0,
		VME		:  2-1,
		DE		:  3-2,
		PSE		:  4-3,
		TSC		:  5-4,
		MSR		:  6-5,
		PAE		:  7-6,
		MCE		:  8-7,
		CMPXCHG8	:  9-8,
		APIC		: 10-9,
		Reserved1	: 11-10,
		SEP		: 12-11,
		MTRR		: 13-12,
		PGE		: 14-13,
		MCA		: 15-14,
		CMOV		: 16-15,
		PAT		: 17-16,
		PSE36		: 18-17,
		PSN		: 19-18, /* Intel Processor Serial Number */
		CLFLUSH 	: 20-19,
		Reserved2	: 21-20,
		DS_PEBS 	: 22-21,
		ACPI		: 23-22,
		MMX		: 24-23,
		FXSR		: 25-24, /* FXSAVE and FXRSTOR instructions. */
		SSE		: 26-25,
		SSE2		: 27-26,
		SS		: 28-27, /* Intel			*/
		HTT		: 29-28,
		TM1		: 30-29, /* Intel			*/
		Reserved3	: 31-30,
		PBE		: 32-31; /* Intel			*/
	} EDX;
} CPUID_0x00000001;

typedef struct	/* MONITOR & MWAIT Leaf.				*/
{		/* Common x86						*/
	struct
	{
		unsigned int
		SmallestSize	: 16-0,
		ReservedBits	: 32-16;
	} EAX;
	struct
	{
		unsigned int
		LargestSize	: 16-0,
		ReservedBits	: 32-16;
	} EBX;
	struct
	{
		unsigned int
		EMX_MWAIT	:  1-0,
		IBE_MWAIT	:  2-1,
		ReservedBits	: 32-2;
	} ECX;
	struct
	{	/* Intel reseved.					*/
		unsigned int
		SubCstate_MWAIT0:  4-0,
		SubCstate_MWAIT1:  8-4,
		SubCstate_MWAIT2: 12-8,
		SubCstate_MWAIT3: 16-12,
		SubCstate_MWAIT4: 20-16,
		SubCstate_MWAIT5: 24-20,
		SubCstate_MWAIT6: 28-24,
		SubCstate_MWAIT7: 32-28;
	} EDX;
} CPUID_0x00000005;


typedef struct THERMAL_POWER_LEAF
{	/* Thermal and Power Management Leaf.				*/
	struct
	{	/* Mostly Intel reserved.				*/
		unsigned int
		DTS		:  1-0, /* Digital temperature sensor avail. */
		TurboIDA	:  2-1, /* Reports bit 38 of MSR 0x1a0	*/
		ARAT		:  3-2, /* Common x86			*/
		Reserved1	:  4-3,
		PLN		:  5-4, /* Power limit notification controls */
		ECMD		:  6-5, /* Clock modulation duty cycle ext. */
		PTM		:  7-6, /* Package thermal management support */
		HWP_Reg 	:  8-7, /* Hardware Performance registers */
		HWP_Int 	:  9-8, /* IA32_HWP_INTERRUPT HWP_Notification*/
		HWP_Act 	: 10-9, /* IA32_HWP_REQUEST Activity_Window */
		HWP_EPP 	: 11-10,/* IA32_HWP_REQUEST Energy Perf. pref.*/
		HWP_Pkg 	: 12-11,/* IA32_HWP_REQUEST_PKG 	*/
		Reserved2	: 13-12,
		HDC_Reg 	: 14-13,/* Hardware Duty Cycling registers */
		Turbo_V3	: 15-14,/* Intel Turbo Boost Max Technology 3 */
		HWP_HPrf	: 16-15,/* Highest Performance change support */
		HWP_PECI	: 17-16,/* HWP PECI override support state. */
		HWP_Flex	: 18-17,/* Flexible HWP is support state. */
		HWP_Fast	: 19-18,/* IA32_HWP_REQUEST MSR fast access */
		HWFB_Cap	: 20-19,/* IA32 HW_FEEDBACK* MSR support */
		HWP_Idle	: 21-20,/* Ignore (or not) Idle SMT Processor */
		Reserved3	: 23-21,
		ITD_MSR 	: 24-23, /* HW_FEEDBACK_{CHAR,THREAD_CONFIG} */
		THERM_INT_MSR	: 25-24, /* IA32_THERM_INTERRUPT MSR	*/
		Reserved4	: 32-25;
	} EAX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		DTS_INT_Threshld:  4-0, /* # of Interrupt Thresholds in DTS */
		Reserved	: 32-4;
	} EBX;
    union
    {
	struct
	{	/* Intel reserved.					*/
		unsigned int
		HCF_Cap 	:  1-0, /* IA32_MPERF(E7h) & IA32_APERF(E8h) */
		ACNT_Cap	:  2-1,
		Reserved1	:  3-2,
		SETBH		:  4-3, /* MSR: IA32_ENERGY_PERF_BIAS (1B0H) */
		Reserved2	:  8-4,
		ITD_CLS 	: 16-8, /* Thread Director classes	*/
		Reserved3	: 32-16;
	};
	struct
	{	/* AMD reserved.					*/
		unsigned int
		EffFreq 	:  1-0, /* MPERF(0000_00E7) & APERF(0000_00E8)*/
		Reserved	: 32-1;
	};
    } ECX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		HWFB_Cap	:  8-7, /*Hardware Feedback Interface bitmap */
		HWFB_pSz	: 12-8, /*HW Feedback structure size (4K page)*/
		Reserved	: 16-12,
		HWFB_Idx	: 32-16; /*HW Feedback structure base 0 index */
	} EDX;
} CPUID_0x00000006;

typedef struct	/* Extended Feature Flags Enumeration Leaf.		*/
{
	struct
	{	/* Common x86						*/
		unsigned int
		MaxSubLeaf	: 32-0;
	} EAX;
	struct
	{
		unsigned int
		FSGSBASE	:  1-0, /* Common x86			*/
		TSC_ADJUST	:  2-1, /* IA32_TSC_ADJUST		*/
		SGX		:  3-2, /* Intel SGX			*/
		BMI1		:  4-3, /* Common x86			*/
		HLE		:  5-4, /* Hardware Lock Elision	*/
		AVX2		:  6-5, /* Common x86			*/
		FDP_EXCPTN_x87	:  7-6, /* FPU Data Pointer exceptions	*/
		SMEP		:  8-7, /* x86: Supervisor-Mode exec.	*/
		BMI2		:  9-8, /* Common x86			*/
		ERMS		: 10-9, /* Enhanced REP MOVSB/STOSB	*/
		INVPCID 	: 11-10, /* Invalidate TLB in Specified PCID*/
		RTM		: 12-11, /* Restricted Transactional Memory */
		PQM		: 13-12, /* Intel RDT-M capability ?	*/
		FPU_CS_DS	: 14-13,
		MPX		: 15-14, /* Memory Protection Extensions */
		PQE		: 16-15, /* Intel RDT-A capability ?	*/
		AVX_512F	: 17-16, /* AVX-512 Foundation Instructions */
		AVX_512DQ	: 18-17,
		RDSEED		: 19-18, /* RDSEED Instruction		*/
		ADX		: 20-19, /* Arbitrary-Precision Arithmetic */
		SMAP_CLAC_STAC	: 21-20, /*Supervisor-Mode Access & CLAC/STAC*/
		AVX512_IFMA	: 22-21,
		Reserved	: 23-22,
		CLFLUSHOPT	: 24-23, /* Flush Cache Line Optimized	*/
		CLWB		: 25-24, /* Cache Line Write Back	*/
		ProcessorTrace	: 26-25, /* CPUID.(EAX=14H, ECX=0)	*/
		AVX512PF	: 27-26, /* Intel Xeon Phi		*/
		AVX512ER	: 28-27, /* Intel Xeon Phi		*/
		AVX512CD	: 29-28,
		SHA		: 30-29, /* Intel/AMD Secure Hash Algorithm */
		AVX512BW	: 31-30,
		AVX512VL	: 32-31;
	} EBX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		PREFETCHWT1	:  1-0, /* Intel Xeon Phi		*/
		AVX512_VBMI	:  2-1,
		UMIP		:  3-2, /* User-Mode Instruction Prevention */
		PKU		:  4-3, /* Protection Keys User-Mode pages */
		OSPKE		:  5-4, /* RDPKRU/WRPKRU instructions	*/
		WAITPKG 	:  6-5, /* TPAUSE, UMONITOR, UMWAIT	*/
		AVX512_VBMI2	:  7-6,
		CET_SS		:  8-7, /* IA32_U_CET and IA32_S_CET	*/
		GFNI		:  9-8, /* Galois Field SSE instructions*/
		VAES		: 10-9,
		VPCLMULQDQ	: 11-10,
		AVX512_VNNI	: 12-11,
		AVX512_BITALG	: 13-12,
		TME_EN		: 14-13, /* IA32_TME_{CAP,ACTIVATE,EXCLUDE} */
		AVX512_VPOPCNTDQ: 15-14, /* Intel Xeon Phi		*/
		Reserved1	: 16-15,
		LA57		: 17-16,
		MAWAU		: 22-17, /* for BNDLDX & BNDSTX instructions*/
		RDPID		: 23-22, /* Intel RDPID inst. & IA32_TSC_AUX */
		KL		: 24-23, /* Key Locker			*/
		BUS_LOCK_DETECT : 25-24,
		CLDEMOTE	: 26-25, /* Support of cache line demote */
		Reserved2	: 27-26,
		MOVDIRI 	: 28-27, /* Move Doubleword as Direct Store*/
		MOVDIR64B	: 29-28, /* Move 64 Bytes as Direct Store*/
		ENQCMD		: 30-29, /* Support of Enqueue Stores	*/
		SGX_LC		: 31-30, /* SGX Launch Configuration support*/
		PKS		: 32-31; /* Protection Keys Supervisor-mode PG*/
	} ECX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Reserved1	:  1-0,
		SGX_KEYS	:  2-1,
		AVX512_4VNNIW	:  3-2,  /* Intel Xeon Phi		*/
		AVX512_4FMAPS	:  4-3,  /* Intel Xeon Phi		*/
		FSRM		:  5-4,  /* Fast Short REP MOVSB 	*/
		UINTR		:  6-5,  /* CLUI,SENDUIPI,STUI,TESTUI,UIRET */
		Reserved2	:  8-6,
		AVX512_VP2INTER :  9-8,  /* TGL: AVX512_VP2INTERSECT	*/
		SRBDS_CTRL	: 10-9,  /* IA32_MCU_OPT_CTRL		*/
		MD_CLEAR_Cap	: 11-10,
		RTM_ALWAYS_ABORT: 12-11,
		Reserved3	: 13-12,
		TSX_FORCE_ABORT : 14-13, /* MSR TSX_FORCE_ABORT capable	*/
		SERIALIZE	: 15-14, /* SERIALIZE instruction	*/
		Hybrid		: 16-15, /* Hybrid part processor	*/
		TSXLDTRK	: 17-16, /* TSX suspend load address tracking*/
		Reserved4	: 18-17,
		PCONFIG 	: 19-18,
		ArchitecturalLBRs:20-19,
		CET_IBT 	: 21-20, /* CET Indirect Branch Tracking */
		Reserved5	: 22-21,
		AMX_BF16	: 23-22,
		AVX512_FP16	: 24-23,
		AMX_TILE	: 25-24,
		AMX_INT8	: 26-25,
		IBRS_IBPB_Cap	: 27-26, /* IA32_SPEC_CTRL,IA32_PRED_CMD */
		STIBP_Cap	: 28-27, /* IA32_SPEC_CTRL[1]		*/
		L1D_FLUSH_Cap	: 29-28, /* IA32_FLUSH_CMD		*/
		IA32_ARCH_CAP	: 30-29, /* IA32_ARCH_CAPABILITIES	*/
		IA32_CORE_CAP	: 31-30, /* IA32_CORE_CAPABILITIES	*/
		SSBD_Cap	: 32-31; /* IA32_SPEC_CTRL[2]		*/
	} EDX;
} CPUID_0x00000007;

typedef struct	/* Extended Feature Flags Enumeration Leaf 1		*/
{		/* Intel reserved.					*/
	struct CPUID_0x00000007_1_EAX
	{
		unsigned int
		Reserved1	:  3-0,
		RAO_INT 	:  4-3,  /* Grand Ridge			*/
		AVX_VNNI_VEX	:  5-4,  /* Vector Neural Network Instructions*/
		AVX512_BF16	:  6-5,  /* BFLOAT16 support in AVX512	*/
		LASS		:  7-6,
		CMPCCXADD	:  8-7,  /* Sierra Forest, Grand Ridge	*/
		ArchPerfmonExt	:  9-8,
		Reserved2	: 10-9,
		FZRM		: 11-10, /* Fast Zero-length REP MOVSB	*/
		FSRS		: 12-11, /* Fast Short REP STOSB:Store String */
		FSRC		: 13-12, /* Fast Short REP CMPSB, REP SCASB */
		Reserved3	: 19-13,
		WRMSRNS 	: 20-19, /* Sierra Forest, Grand Ridge	*/
		Reserved4	: 21-20,
		AMX_FP16	: 22-21, /* Granite Rapids		*/
		HRESET		: 23-22, /* History Reset instruction	*/
		AVX_IFMA	: 24-23, /* Sierra Forest, Grand Ridge	*/
		Reserved5	: 26-24,
		LAM		: 27-26, /* Linear Address Masking	*/
		RDMSRLIST	: 28-27, /* Sierra Forest, Grand Ridge	*/
		Reserved6	: 32-28;
	} EAX;
	struct
	{
		unsigned int
		MSR_PPIN_CAP	:  1-0,  /* MSR IA32_PPIN and IA32_PPIN_CTL */
		Reserved	: 32-1;
	} EBX;
	struct
	{
		unsigned int
		Reserved	: 32-0;
	} ECX;
	struct CPUID_0x00000007_1_EDX
	{
		unsigned int
		Reserved1	:  4-0,
		AVX_VNNI_INT8	:  5-4,  /* Sierra Forest, Grand Ridge	*/
		AVX_NE_CONVERT	:  6-5,  /* Sierra Forest, Grand Ridge	*/
		Reserved2	: 14-6,
		PREFETCHI	: 15-14, /* Granite Rapids: IA32_UINTR	*/
		Reserved3	: 18-15,
		CET_SSS 	: 19-18,
		AVX10		: 20-19, /* AVX10 Converged Vector ISA	*/
		Reserved4	: 32-20;
	} EDX;
} CPUID_0x00000007_1;

typedef struct	/* Extended Feature Flags Leaf equal or greater than 2	*/
{
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Reserved	: 32-0;
	} EAX, EBX, ECX;
	struct CPUID_0x00000007_2_EDX
	{	/* Intel reserved.					*/
		unsigned int
		PSFD_SPEC_CTRL	:  1-0,
		IPRED_SPEC_CTRL :  2-1,
		RRSBA_SPEC_CTRL :  3-2,
		DDPD_U_SPEC_CTRL:  4-3,
		BHI_SPEC_CTRL	:  5-4,
		MCDT_NO 	:  6-5,
		Reserved	: 32-6;
	} EDX;
} CPUID_0x00000007_2;

typedef struct	/* Architectural Performance Monitoring Leaf.		*/
{	/* Intel reserved.						*/
	struct
	{
		unsigned int
		Version 	:  8-0,
		MonCtrs 	: 16-8,
		MonWidth	: 24-16,
		VectorSz	: 32-24;
	} EAX;
	struct
	{
		unsigned int
		CoreCycles	:  1-0,
		InstrRetired	:  2-1,
		RefCycles	:  3-2,
		LLC_Ref 	:  4-3,
		LLC_Misses	:  5-4,
		BranchRetired	:  6-5,
		BranchMispred	:  7-6,
		TopdownSlots	:  8-7,
		ReservedBits	: 32-8;
	} EBX;
	struct
	{
		unsigned int
		FixCtrs_Mask	: 32-0; /* Counter# supported at ECX[Bit#] */
	} ECX;
	struct
	{
		unsigned int
		FixCtrs 	:  5-0,
		FixWidth	: 13-5,
		Reserved1	: 15-13,
		AnyThread_Dprec : 16-15, /* AnyThread deprecation.	*/
		Reserved2	: 32-16;
	} EDX;
} CPUID_0x0000000a;

typedef struct	/* Intel Hybrid Information Enumeration Leaf		*/
{
		unsigned int
		Model_ID	: 24-0,
		CoreType	: 32-24; /* 0x20: Atom; 0x40: Core	*/
} CPUID_0x0000001a;

typedef struct	/* Extended CPUID Function.				*/
{
		unsigned int LargestExtFunc, EBX, ECX, EDX;
} CPUID_0x80000000;

typedef struct
{
	struct { /* AMD reserved, compatible with CPUID(0x00000001)	*/
		unsigned int
		Stepping	:  4-0,
		Model		:  8-4,
		Family		: 12-8,
		Reserved1	: 16-12,
		ExtModel	: 20-16,
		ExtFamily	: 28-20,
		Reserved2	: 32-28;
	} EAX;
	union { /* AMD reserved, same as CPUID(0x00000001)		*/
	    struct {
		unsigned int
		Brand_ID	:  8-0,
		CLFSH_Size	: 16-8,
		Max_SMT_ID	: 24-16,
		Init_APIC_ID	: 32-24;
	    };
	    struct {			/*	AMD Zen families	*/
		unsigned int
		Reserved	: 28-0,
		PackageType	: 32-28; /* [31-28] PkgType:0=FP6,2=AM4 */
	    };
	} EBX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		LAHFSAHF	:  1-0, /* LAHF and SAHF instruction support */
		Reserved1	:  5-1,
		LZCNT		:  6-5,
		Reserved2	:  8-6,
		PREFETCHW	:  9-8,
		Reserved3	: 32-9;
	};
	struct { /* AMD reserved.					*/
		unsigned int
		/* Family 0Fh :						*/
		LahfSahf:  1-0,
		MP_Mode :  2-1,  /* Core multi-processing legacy mode.	*/
		SVM	:  3-2,  /* Secure virtual machine.		*/
		xApicSpace:4-3,  /* Extended APIC space.		*/
		AltMov	:  5-4,  /* AltMovCr8				*/
		ABM	:  6-5,  /* LZCNT instruction support.		*/
		SSE4A	:  7-6,
		AlignSSE:  8-7,  /* Misaligned SSE mode.		*/
		PREFETCH:  9-8,  /* 3DNow PREFETCH, PREFETCHW instruction. */
		/* Families [10h, 15h - 17h]:				*/
		OSVW	: 10-9,  /* OS-visible workaround support.	*/
		IBS	: 11-10, /* Instruction based sampling.		*/
		XOP	: 12-11, /* Extended operation support.		*/
		SKINIT	: 13-12, /* SKINIT and STGI support.		*/
		WDT	: 14-13, /* Watchdog timer support.		*/
		NotUsed1: 15-14,
		LWP	: 16-15, /* Lightweight profiling support.	*/
		FMA4	: 17-16, /* Four-operand FMA instruction.	*/
		TCE	: 18-17, /* Translation Cache Extension.	*/
		NotUsed2: 19-18,
		NodeId	: 20-19, /* Family 10h				*/
		NotUsed3: 21-20,
		TBM	: 22-21, /* Trailing bit manipulation.		*/
		ExtApicId:23-22, /* Topology extensions in CPUID_0x8000001e */
		PerfCore: 24-23, /* PerfCtrExtCore MSR.			*/
		PerfNB	: 25-24, /* PerfCtrExtNB MSR.			*/
		NotUsed4: 26-25,
		Data_BP : 27-26, /* Data access breakpoint extension.	*/
		CU_PTSC : 28-27, /* Global TSC (CU_PTSC)		*/
		PerfLLC : 29-28, /* Last level Cache perf. counter extensions*/
		MWaitExt: 30-29, /* MWAITX/MONITORX support.		*/
		AdMskExt: 31-30, /* Addr Mask Ext support for Inst Breakpoint*/
		NotUsed5: 32-31;
	};
    } ECX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		Reserved1	: 11-0,
		SYSCALL 	: 12-11,
		Reserved2	: 20-12,
		XD_Bit		: 21-20,
		Reserved3	: 26-21,
		PG_1GB		: 27-26,
		RdTSCP		: 28-27,
		Reserved4	: 29-28,
		IA64		: 30-29,
		Reserved5	: 32-30;
	};
	struct { /* AMD reserved.					*/
		unsigned int	 /* Most bits equal to CPUID 0x01	*/
		FPU	:  1-0,
		VME	:  2-1,  /* Virtual-mode enhancements.		*/
		DE	:  3-2,  /* Debugging extensions.		*/
		PSE	:  4-3,  /* Page-size extensions.		*/
		TSC	:  5-4,
		MSR	:  6-5,  /* AMD MSR.				*/
		PAE	:  7-6,
		MCE	:  8-7,
		CMPXCH8 :  9-8,
		APIC	: 10-9,
		NotUsed1: 11-10,
		SEP	: 12-11,
		MTRR	: 13-12,
		PGE	: 14-13,
		MCA	: 15-14,
		CMOV	: 16-15,
		PAT	: 17-16,
		PSE36	: 18-17,
		NotUsed2: 20-18,
		NX	: 21-20, /* No-Execute Page Protection.		*/
		NotUsed3: 22-21,
		MMX_Ext : 23-22, /* MMX extensions.			*/
		MMX	: 24-23,
		FXSR	: 25-24,
		FFXSR	: 26-25, /* FXSAVE and FXRSTOR optimizations.	*/
		Page1GB : 27-26,
		RDTSCP	: 28-27,
		NotUsed4: 29-28,
		LM	: 30-29, /* Long mode.				*/
		_3DNowEx: 31-30, /* Extensions to 3DNow!		*/
		_3DNow	: 32-31; /* 3DNow! instructions.		*/
	};
    } EDX;
} CPUID_0x80000001;

typedef struct	/* Architectural Performance Monitoring Leaf.		*/
{
	struct
	{
		unsigned int
		Reserved	: 32-0;
	} EAX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		Reserved	: 32-0;
	};
	struct { /* AMD as April 2020					*/
		unsigned int
		MCA_Ovf 	:  1-0,/* MCA overflow recovery support */
		SUCCOR		:  2-1,/*SW uncorrectable error & recovery cap*/
		HWA		:  3-2,/* Hardware Assert MSR 0xc00110[df:c0].*/
		Scal_MCA	:  4-3,/* ScalableMca w/ 1 is supported */
		Rsvd_AMD	: 32-4;
	};
    } EBX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		Reserved	: 32-0;
	};
	struct { /* AMD as April 2020					*/
		unsigned int		/* Ratio of the compute unit power */
		CpuPwrSampleTimeRatio;  /* accumulator sample period to */
	};				/* the TSC counter period.	*/
    } ECX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		Reserved1	:  8-0,
		Inv_TSC 	:  9-8,  /* Invariant TSC available if 1 */
		Reserved2	: 32-9;
	};
      union
      { 	/* AMD Family 0Fh					*/
	struct {
		unsigned int
		TS	:  1-0,  /* Temperature sensor			*/
		FID	:  2-1,  /* Frequency ID control is supported.	*/
		VID	:  3-2,  /* Voltage ID control is supported.	*/
		TTP	:  4-3,  /* THERMTRIP is supported = 1.		*/
		TM	:  5-4,  /* Hardware thermal control (HTC).	*/
		STC	:  6-5,  /* K7-K8: Software thermal control (STC) */
		_100MHz :  7-6,  /* 100 MHz multiplier Control. 	*/
		NotUsed : 32-7;
	};
	struct { /* AMD starting at family 15h				*/
		unsigned int
		Fam_0Fh :  7-0,  /* Family 0Fh features.		*/
		HwPstate:  8-7,  /* Hardware P-state control MSR 0xc0010061-63*/
		TscInv	:  9-8,  /* Invariant TSC ?			*/
		CPB	: 10-9,  /* Core performance boost.		*/
		EffFrqRO: 11-10, /* Read-only effective freq. interf. msr ?   */
		ProcFb	: 12-11, /* Processor feedback interf. available if 1 */
		ProcPwr : 13-12, /* Core power reporting interface supported. */
		ConStdBy: 14-13, /* ConnectedStandby			*/
		RAPL	: 15-14, /* F17h and afterward: RAPL support	*/
		FastCPPC: 16-15, /* F19h Model 61h: Fast CPPC support	*/
		Reserved: 32-16;
	};
      };
    } EDX;
} CPUID_0x80000007;

typedef struct	/* Processor Capacity Leaf.				*/
{
	struct {
		unsigned int
		MaxPhysicalAddr :  8-0,  /* Common x86			*/
		MaxLinearAddr	: 16-8,  /* Common x86			*/
		MaxGuestPhysAddr: 24-16, /* AMD reserved		*/
		Reserved	: 32-24;
	} EAX;
	struct
	{	/* AMD Family 17h, 19h					*/
		unsigned int
		CLZERO		:  1-0,  /* Clear Zero Instruction	*/
		IRPerf		:  2-1,  /* Inst. Retired Counter support */
		XSaveErPtr	:  3-2,  /* FX___ error pointers support */
		INVLPGB 	:  4-3,  /* SMT TLB invalidate broadcast */
		RDPRU		:  5-4,  /* MPERF/APERF at user level	*/
		Reserved1	:  6-5,
		MBE		:  7-6,  /* Memory Bandwidth Enforcement */
		Reserved2	:  8-7,
		MCOMMIT 	:  9-8,  /* Memory Commit Instruction	*/
		WBNOINVD	: 10-9, /*Write Back & Do Not Invalidate Cache*/
		LBREXTN 	: 11-10, /* LBR Extensions		*/
		Reserved3	: 12-11,
		IBPB		: 13-12, /* Indirect Branch Prediction Barrier*/
		INT_WBINVD	: 14-13, /* Interruptible WBINVD,WBNOINVD */
		IBRS		: 15-14, /* IBR Speculation		*/
		STIBP		: 16-15, /* Single Thread Indirect Branch Pred*/
		IBRS_AlwaysOn	: 17-16,
		STIBP_AlwaysOn	: 18-17,
		IBRS_Preferred	: 19-18,
		IBRS_SameMode	: 20-19,
		MSR_EFER_LMSLE	: 21-20,
		TlbFlushNested	: 22-21,
		Reserved4	: 23-22,
		PPIN		: 24-23, /* Protected Processor Inventory Num */
		SSBD		: 25-24, /* Speculative Store Bypass Disable */
		SSBD_VirtSpecCtrl:26-25, /* Use VIRT_SPEC_CTL for SSBD */
		SSBD_NotRequired: 27-26,
		CPPC		: 28-27,
		PSFD		: 29-28, /* 1: SPEC_CTRL_MSR is supported */
		BTC_NO		: 30-29, /* No Branch Type Confusion	*/
		Reserved5	: 31-30,
		BranchSample	: 32-31;
	} EBX;
	struct { /* AMD reserved					*/
		unsigned int
		NC		:  8-0,  /* Zero based number of threads */
		Reserved1	: 12-8,
		ApicIdCoreIdSize: 16-12,/* Initial APIC ID size to compute MNC*/
		CU_PTSC_Size	: 18-16, /* 00b=40,01b=48,10b=56,11b=64 bits  */
		Reserved2	: 32-18;
	} ECX;
	struct
	{	/* AMD Family 17h					*/
		unsigned int
		INVLPGB_CountMax: 16-0,  /* Maximum count for INVLPGB inst. */
		RDPRU_Max	: 24-16, /* RDPRU Instruction max input */
		Reserved	: 32-24;
	} EDX;
} CPUID_0x80000008;

typedef struct	/* AMD Extended ID Leaf.				*/
{ /* Remark: all registers valid if CPUID(0x80000001).ECX.TopoEx == 1	*/
	struct {
		unsigned int
		ExtApicId	: 32-0;/* iff MSR(APIC_BAR[APIC_EN]) == 1 */
	} EAX;
    union
	{
	struct {	/* Family 15h					*/
		unsigned int
		CompUnitId	:  8-0,
		CoresPerCU	: 16-8, /* CoresPerComputeUnit + 1	*/
		Reserved_F15h	: 32-16;
		};
	struct {	/* Family 17h					*/
		unsigned int
		CoreId		:  8-0,
		ThreadsPerCore	: 16-8,
		Reserved_F17h	: 32-16;
		};
	} EBX;
	struct {
		unsigned int
		NodeId		:  8-0,
		NodesPerProc	: 11-8,  /* 000b=1,001b=2,011b=4 nodes	*/
		Reserved	: 32-11;
	} ECX;
	struct
	{
		unsigned int
		Reserved	: 32-0;
	} EDX;
} CPUID_0x8000001e;

enum	/*	Intel SGX Capability Enumeration Leaf.			*/
{
	CPUID_00000012_00000000_EAX_SGX1,
	CPUID_00000012_00000000_EAX_SGX2,
	CPUID_00000012_00000000_EAX_Reserved_02,
	CPUID_00000012_00000000_EAX_Reserved_03,
	CPUID_00000012_00000000_EAX_Reserved_04,
	CPUID_00000012_00000000_EAX_SGX_ENCLV,
	CPUID_00000012_00000000_EAX_SGX_ENCLS
};

enum
{	/*	Intel Processor Trace Enumeration Main Leaf		*/
	CPUID_00000014_00000000_EBX_CR3Filter,
	CPUID_00000014_00000000_EBX_PSBFreq,
	CPUID_00000014_00000000_EBX_IP_Filtering,
	CPUID_00000014_00000000_EBX_MTCEn,
	CPUID_00000014_00000000_EBX_PTWEn,
	CPUID_00000014_00000000_EBX_PwrEvtEn,
	CPUID_00000014_00000000_EBX_PendPSB,
	CPUID_00000014_00000000_EBX_EventEn,
	CPUID_00000014_00000000_EBX_DisTNT,
};

enum	/*	Intel AES Key Locker instructions.			*/
{
	CPUID_00000019_00000000_EBX_AESKLE,
	CPUID_00000019_00000000_EBX_Reserved_01,
	CPUID_00000019_00000000_EBX_WIDE_KL,
	CPUID_00000019_00000000_EBX_Reserved_03,
	CPUID_00000019_00000000_EBX_IWKeyBackup
};

enum	/*	AMD SVM Revision and Feature Identification.		*/
{
	CPUID_8000000A_00000000_EDX_NP,
	CPUID_8000000A_00000000_EDX_LBR_Virt,
	CPUID_8000000A_00000000_EDX_SVM_Lock,
	CPUID_8000000A_00000000_EDX_NRIP_Save,
	CPUID_8000000A_00000000_EDX_TSC_Rate_MSR,
	CPUID_8000000A_00000000_EDX_VMCB_Clean,
	CPUID_8000000A_00000000_EDX_FlushBy_ASID,
	CPUID_8000000A_00000000_EDX_DecodeAssists,
	CPUID_8000000A_00000000_EDX_Reserved_08,
	CPUID_8000000A_00000000_EDX_Reserved_09,
	CPUID_8000000A_00000000_EDX_PAUSE_Filter,
	CPUID_8000000A_00000000_EDX_Reserved_11,
	CPUID_8000000A_00000000_EDX_PAUSE_Thold,
	CPUID_8000000A_00000000_EDX_AVIC,
	CPUID_8000000A_00000000_EDX_Reserved_14,
	CPUID_8000000A_00000000_EDX_VMSAVE_Virt,
	CPUID_8000000A_00000000_EDX_VGIF,
	CPUID_8000000A_00000000_EDX_GMET,
	CPUID_8000000A_00000000_EDX_x2AVIC,
	CPUID_8000000A_00000000_EDX_SSS_Check,
	CPUID_8000000A_00000000_EDX_SPEC_CTRL,
	CPUID_8000000A_00000000_EDX_RO_GPT,
	CPUID_8000000A_00000000_EDX_Reserved_22,
	CPUID_8000000A_00000000_EDX_HOST_MCE_OVER,
	CPUID_8000000A_00000000_EDX_TLB_Intercept,
	CPUID_8000000A_00000000_EDX_VNMI,
	CPUID_8000000A_00000000_EDX_IBS_Virt,
	CPUID_8000000A_00000000_EDX_Reserved_27,
	CPUID_8000000A_00000000_EDX_Reserved_28,
	CPUID_8000000A_00000000_EDX_Reserved_29,
	CPUID_8000000A_00000000_EDX_Reserved_30,
	CPUID_8000000A_00000000_EDX_Reserved_31
};

enum	/*	AMD Performance Optimization Identifiers.		*/
{
	CPUID_8000001A_00000000_EAX_FP128	= 0,
	CPUID_8000001A_00000000_EAX_MOVU	= 1,
	CPUID_8000001A_00000000_EAX_FP256	= 2
};

enum	/*	AMD Encrypted Memory Capabilities.			*/
{
	CPUID_8000001F_00000000_EAX_SME,
	CPUID_8000001F_00000000_EAX_SEV,
	CPUID_8000001F_00000000_EAX_PageFlush_MSR,
	CPUID_8000001F_00000000_EAX_SEV_ES,
	CPUID_8000001F_00000000_EAX_SEV_SNP,
	CPUID_8000001F_00000000_EAX_VMPL,
	CPUID_8000001F_00000000_EAX_RMPQUERY,
	CPUID_8000001F_00000000_EAX_VMPL_SSS,
	CPUID_8000001F_00000000_EAX_Secure_TSC,
	CPUID_8000001F_00000000_EAX_TSC_AUX_Virt,
	CPUID_8000001F_00000000_EAX_HwCacheCoherency,
	CPUID_8000001F_00000000_EAX_SEV_64BitHost,
	CPUID_8000001F_00000000_EAX_RestrictedInject,
	CPUID_8000001F_00000000_EAX_AlternateInject,
	CPUID_8000001F_00000000_EAX_DebugSwap,
	CPUID_8000001F_00000000_EAX_Prevent_Host_IBS,
	CPUID_8000001F_00000000_EAX_VTE,
	CPUID_8000001F_00000000_EAX_VMGEXIT_Param,
	CPUID_8000001F_00000000_EAX_Virtual_TOM_MSR,
	CPUID_8000001F_00000000_EAX_IBS_Virt_SEV_ES,
	CPUID_8000001F_00000000_EAX_Reserved_20,
	CPUID_8000001F_00000000_EAX_Reserved_21,
	CPUID_8000001F_00000000_EAX_Reserved_22,
	CPUID_8000001F_00000000_EAX_Reserved_23,
	CPUID_8000001F_00000000_EAX_VMSA_Protection,
	CPUID_8000001F_00000000_EAX_SMT_Protection,
	CPUID_8000001F_00000000_EAX_Reserved_26,
	CPUID_8000001F_00000000_EAX_Reserved_27,
	CPUID_8000001F_00000000_EAX_SVSM_MSR,
	CPUID_8000001F_00000000_EAX_Virt_SNP_MSR,
	CPUID_8000001F_00000000_EAX_Reserved_30,
	CPUID_8000001F_00000000_EAX_Reserved_31
};

enum	/*	AMD Multi-Key Encrypted Memory Capabilities.		*/
{
	CPUID_80000023_00000000_EAX_MEM_HMK
};

typedef struct	/* AMD Extended Performance Monitoring and Debug.	*/
{
	struct {
		unsigned int
		PerfMonV2	:  1-0,
		LbrStack	:  2-1,
		LbrAndPmcFreeze :  3-2,
		Reserved	: 32-3;
	} EAX;
	struct {
		unsigned int
		NumPerfCtrCore	:  4-0,
		LbrStackSize	: 10-4,
		NumPerfCtrNB	: 16-10,
		Reserved	: 32-16;
	} EBX;
	struct {
		unsigned int
		Reserved	: 32-0;
	} ECX;
	struct
	{
		unsigned int
		Reserved	: 32-0;
	} EDX;
} CPUID_0x80000022;

typedef struct	/* AMD [Extended CPU Topology.				*/
{ /*
CPUID Fn8000_0026_E[D,C,B,A]X_x[3:0] specifies the hierarchy of logical cores
 from the SMT level through the processor socket level.
Software reads CPUID Fn8000_0026_E[C,B,A]X for ascending values
 of ECX until (CPUID Fn8000_0026_EBX[LogProcAtThisLevel] == 0).
Note: While CPUID Fn8000_0026 is a preferred superset to CPUID_Fn0000000B,
 CPUID_Fn0000000B information is valid for software for
 the supported levels on AMD.
*/
	struct {
		unsigned int
		CoreMaskWidth	:  5-0,
		Reserved	: 29-5,
		PowerRankingCap	: 30-29, /* ProcessorPowerEfficiencyRanking */
		CoreTopology	: 31-30, /* CoreType:HeterogeneousCoreTopology*/
		AsymmetricCores : 31-30;
	} EAX;
	struct {
		unsigned int
		LogProcThisLevel: 16-0,  /*Number of logical processors @Level*/
		PowerRanking	: 24-16, /* ProcessorPowerEfficiencyRanking */
		Native_Model_ID : 28-24, /* Iff Level Type=Core: 0=Zen4 Core */
		CoreType	: 32-28; /* 0: P-core ; 1: E-core	*/
	} EBX;
	struct {
		unsigned int
		ECX_Value	:  8-0,  /* ECX input value		*/
		LevelType	: 16-8,  /* 0:Rsv; 1:Core; 2:CCX; 3:CCD; 4:Skt*/
		Reserved	: 32-16;
	} ECX;
	struct
	{
		unsigned int
		Extended_APIC_ID: 32-0; /* Extended Local APIC ID	*/
	} EDX;
} CPUID_0x80000026;

typedef struct	/* BSP CPUID features.					*/
{
	CPUID_FUNCTION		Info;

	CPUID_0x00000001	Std;
	CPUID_0x00000005	MWait;
	CPUID_0x00000006	Power;
	CPUID_0x00000007	ExtFeature;
 struct CPUID_0x00000007_1_EAX	ExtFeature_Leaf1_EAX;
 struct CPUID_0x00000007_1_EDX	ExtFeature_Leaf1_EDX;
 struct CPUID_0x00000007_2_EDX	ExtFeature_Leaf2_EDX;
	CPUID_0x0000000a	PerfMon;
	CPUID_0x80000001	ExtInfo;
	CPUID_0x80000007	AdvPower;
	CPUID_0x80000008	leaf80000008;

	struct {
		unsigned long long	PPIN;
			CLOCK		Clock;
			unsigned int	Freq,
					Ratio;
		struct {
			unsigned int	Interface;
		    union {
			unsigned int	Version;
			struct {
			unsigned char	Revision,
					Minor,
					Major,
					Other;
			};
		    };
		} SMU;
		struct {
			unsigned char	LLC,
					NB;
		} PMC;
	} Factory;

	struct {
		Bit64	InvariantTSC	:  8-0,
			HyperThreading	:  9-8,
			HTT_Enable	: 10-9,
			TgtRatio_Unlock : 11-10,
			ClkRatio_Unlock : 13-11, /* X.Y w/ X=Max and Y=Min */
			Turbo_Unlock	: 14-13,
			TDP_Unlock	: 15-14,
			TDP_Levels	: 17-15,
			TDP_Cfg_Lock	: 18-17,
			TDP_Cfg_Level	: 20-18,
			TurboActiv_Lock : 21-20,
			Uncore_Unlock	: 22-21,
			HWP_Enable	: 23-22,
			HDC_Enable	: 24-23,
			SpecTurboRatio	: 32-24,
			EEO_Capable	: 33-32,
			EEO_Enable	: 34-33,
			R2H_Capable	: 35-34,
			R2H_Enable	: 36-35,
			HSMP_Capable	: 37-36,
			HSMP_Enable	: 38-37,
			XtraCOF 	: 40-38, /* 1:CPB ; 2:{CPB and XFR} */
			ACPI_PCT_CAP	: 41-40,
			ACPI_PCT	: 42-41,
			ACPI_PSS_CAP	: 43-42,
			ACPI_PSS	: 47-43, /* 15 Pstate sub-packages */
			ACPI_PPC_CAP	: 48-47,
			ACPI_PPC	: 52-48, /* Range of states supported */
			ACPI_CPPC	: 53-52,
			OSPM_CPC	: 54-53,
			OSPM_EPP	: 55-54,
			ACPI_CST_CAP	: 56-55,
			ACPI_CST	: 60-56, /* 15 CState sub-packages */
			_pad64		: 64-60;
	};
} FEATURES;

/* Memory Controller' structures dimensions.				*/
#define MC_MAX_CTRL	8
#define MC_MAX_CHA	12
#define MC_MAX_DIMM	4

#define MC_3D_VECTOR_TO_SCALAR(_mc, _cha, _slot)			\
	((_mc * MC_MAX_CTRL) + (_cha * MC_MAX_CHA) + _slot)

#define MC_2D_VECTOR_TO_SCALAR(_mc, _cha)				\
	((_mc * MC_MAX_CTRL) + _cha)

#define MC_VECTOR_DISPATCH(_1, _2, _3, MC_VECTOR_CURSOR, ...)		\
	MC_VECTOR_CURSOR

#define MC_VECTOR_TO_SCALAR( ... )					\
	MC_VECTOR_DISPATCH(__VA_ARGS__, MC_3D_VECTOR_TO_SCALAR, /*3*/	\
					MC_2D_VECTOR_TO_SCALAR, /*2*/	\
					NULL)			/*1*/	\
							( __VA_ARGS__ )

#define MC_MHZ		0b00
#define MC_MTS		0b01
#define MC_MBS		0b10
#define MC_NIL		0b11

typedef struct
{
	unsigned int	tCL;
	union {
	unsigned int	tRCD;
	unsigned int	tRCD_RD;
	};
	unsigned int	tRCD_WR,
			tRP,
			tRAS,
			tRC,

			tRCPB,
			tRPPB;
	union {
	unsigned int	tRRD;
	unsigned int	tRRDS;
	};
	unsigned int	tRRDL,

			tFAW,
			tFAWSLR,
			tFAWDLR,

			tWTRS,
			tWTRL,
			tWR;

	unsigned int	tRCPage;
	union {
	unsigned int	tRTPr;
	unsigned int	tRTP;
	};
	unsigned int	tWTPr,
			tCWL;
	union {
		struct {
	unsigned int	tddRdTWr,
			tddWrTRd,
			tddWrTWr,
			tddRdTRd,
			tsrRdTRd,
			tdrRdTRd,
			tsrRdTWr,
			tdrRdTWr,
			tsrWrTRd,
			tdrWrTRd,
			tsrWrTWr,
			tdrWrTWr;
		}; /* DDR3 */
		struct {
	unsigned int	tRDRD_SG,
			tRDRD_DG,
			tRDRD_DR,
			tRDRD_DD,
			tRDWR_SG,
			tRDWR_DG,
			tRDWR_DR,
			tRDWR_DD,
			tWRRD_SG,
			tWRRD_DG,
			tWRRD_DR,
			tWRRD_DD,
			tWRWR_SG,
			tWRWR_DG,
			tWRWR_DR,
			tWRWR_DD;
		}; /* DDR4 & DDR5 */
		struct {
	unsigned int	tddRdTWr,
			tddWrTRd,
			tddWrTWr,
			tddRdTRd,
			tRdRdScl,
			tWrWrScl,
			tscWrTWr,
			tsdWrTWr,
			tscRdTRd,
			tsdRdTRd,
			tRdRdScDLR,
			tWrWrScDLR,
			tWrRdScDLR,
			tRRDDLR,
			tRdRdBan,
			tWrWrBan;
		} Zen;
	};
	unsigned int	tXS,
			tXP,
			tCKE,
			tCPDED;

	unsigned int	tREFI;
	union {
	unsigned int	tRFC;
	unsigned int	tRFC1;
	};
	union {
	unsigned int	tRFC4;
	unsigned int	tRFCsb;
	};
	unsigned int	tRFC2,
			tMRD,
			tMOD,
			tMRD_PDA,
			tMOD_PDA,
			tSTAG,
			tPHYWRD,
			tPHYWRL,
			tPHYRDL,
			tRDDATA,
			tWRMPR;

	unsigned int	CMD_Rate;
	union {
	  unsigned int	B2B;
	  unsigned int	GEAR;
	};
	struct {
	  unsigned int	GDM	:  1-0,
			BGS	:  2-1,
			BGS_ALT :  3-2,
			PDM_EN	:  4-3,
			PDM_MODE:  5-4,
			PDM_AGGR:  9-5,
			Scramble: 10-9,
			TSME	: 11-10,
			Unused	: 32-11;
	};
	unsigned int	ECC;
} RAM_TIMING;

typedef struct
{
	struct {
		unsigned int	Size,
				Rows,
				Cols;
		unsigned short	Banks,
				Ranks;
	};
} RAM_GEOMETRY;

enum RAM_STANDARD {
	RAM_STD_UNSPEC,
	RAM_STD_SDRAM,
	RAM_STD_LPDDR,
	RAM_STD_RDIMM
};

/* Source: /include/uapi/linux/utsname.h				*/
#ifdef __NEW_UTS_LEN
	#define MAX_UTS_LEN		__NEW_UTS_LEN
#else
	#define MAX_UTS_LEN		64
#endif

/* Sources: /include/linux/cpuidle.h  &  /include/linux/cpuidle.h	*/
#ifndef _LINUX_CPUIDLE_H
	#define CPUIDLE_STATE_MAX	10
	#define CPUIDLE_NAME_LEN	16
#endif
#define SUB_CSTATE_COUNT	(CPUIDLE_STATE_MAX - 1)
/* Borrow the Kernel Idle State Flags					*/
#ifndef CPUIDLE_FLAG_NONE
	#define CPUIDLE_FLAG_NONE		(0x00)
#endif
#ifndef CPUIDLE_FLAG_POLLING
	#define CPUIDLE_FLAG_POLLING		(1UL << (0))
#endif
#ifndef CPUIDLE_FLAG_COUPLED
	#define CPUIDLE_FLAG_COUPLED		(1UL << (1))
#endif
#ifndef CPUIDLE_FLAG_TIMER_STOP
	#define CPUIDLE_FLAG_TIMER_STOP 	(1UL << (2))
#endif
#ifndef CPUIDLE_FLAG_UNUSABLE
	#define CPUIDLE_FLAG_UNUSABLE		(1UL << (3))
#endif
#ifndef CPUIDLE_FLAG_OFF
	#define CPUIDLE_FLAG_OFF		(1UL << (4))
#endif
#ifndef CPUIDLE_FLAG_TLB_FLUSHED
	#define CPUIDLE_FLAG_TLB_FLUSHED	(1UL << (5))
#endif
#ifndef CPUIDLE_FLAG_RCU_IDLE
	#define CPUIDLE_FLAG_RCU_IDLE		(1UL << (6))
#endif

#ifndef _LINUX_CPUFREQ_H
	#define CPUFREQ_NAME_LEN	16
#endif

#define REGISTRATION_DISABLE	0b00
#define REGISTRATION_ENABLE	0b01
#define REGISTRATION_FULLCTRL	0b10

enum IDLE_ROUTE {
	ROUTE_DEFAULT	= 0,
	ROUTE_IO	= 1,
	ROUTE_HALT	= 2,
	ROUTE_MWAIT	= 3,
	ROUTE_SIZE
};

#define CLOCKSOURCE_PATH "/sys/devices/system/clocksource/clocksource0"

typedef struct {	/* 0: Disable; 1: Enable; 2: Full-control	*/
	enum IDLE_ROUTE Route;
	unsigned short	CPUidle :  2-0,
			CPUfreq :  4-2,
			Governor:  6-4,
			CS	:  8-6,
			unused	: 16-8;
} KERNEL_DRIVER;

typedef struct {
	struct {
		int		stateCount,
				stateLimit;
	    struct {
		unsigned int	exitLatency;		/* in US	*/
			int	powerUsage;		/* in mW	*/
		unsigned int	targetResidency;	/* in US	*/
			char	Name[CPUIDLE_NAME_LEN],
				Desc[CPUIDLE_NAME_LEN];
	    } State[CPUIDLE_STATE_MAX];
		char		Name[CPUIDLE_NAME_LEN];
	} IdleDriver;
	struct {
		char		Name[CPUFREQ_NAME_LEN],
				Governor[CPUFREQ_NAME_LEN];
	} FreqDriver;
} OS_DRIVER;

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN		16
#endif

#define CODENAME_LEN		32

enum SORTBYFIELD {F_STATE, F_RTIME, F_UTIME, F_STIME, F_PID, F_COMM};
#define SORTBYCOUNT	(1 + F_COMM)

typedef struct {
	unsigned long long	runtime,
				usertime,
				systime;
	pid_t			pid,	/* type of __kernel_pid_t is integer */
				tgid,
				ppid;
	short int		state;		/* TASK_STATE_MAX = 0x10000 */
	short int		wake_cpu;	/* limited to 64K CPUs	*/
	char			comm[TASK_COMM_LEN];
} TASK_MCB;

typedef struct {
	unsigned long		totalram,
				sharedram,
				freeram,
				bufferram,
				totalhigh,
				freehigh;
} MEM_MCB;

#define SYSGATE_STRUCT_SIZE	( sizeof(int)				\
				+ sizeof(unsigned int)			\
				+ 4 * MAX_UTS_LEN )

#if defined(TASK_ORDER) && (TASK_ORDER > 0)
#define TASK_LIMIT		(((4096 << TASK_ORDER) - SYSGATE_STRUCT_SIZE) \
				/ sizeof(TASK_MCB))
#else
#define TASK_LIMIT		(((4096 << 5) - SYSGATE_STRUCT_SIZE)	\
				/ sizeof(TASK_MCB))
#endif

/* Input-Output Control							*/
#define COREFREQ_TOGGLE_OFF	0x0
#define COREFREQ_TOGGLE_ON	0x1

#define COREFREQ_IOCTL_MAGIC	0xc3

enum {
	MACHINE_CONTROLLER,
	MACHINE_INTERVAL,
	MACHINE_AUTOCLOCK,
	MACHINE_EXPERIMENTAL,
	MACHINE_INTERRUPTS,
	MACHINE_LIMIT_IDLE,
	MACHINE_CPU_IDLE,
	MACHINE_CPU_FREQ,
	MACHINE_GOVERNOR,
	MACHINE_CLOCK_SOURCE,
	MACHINE_FORMULA_SCOPE,
	MACHINE_IDLE_ROUTE
};

enum {
	TECHNOLOGY_EIST,
	TECHNOLOGY_C1E,
	TECHNOLOGY_TURBO,
	TECHNOLOGY_C1A,
	TECHNOLOGY_C3A,
	TECHNOLOGY_C1U,
	TECHNOLOGY_C3U,
	TECHNOLOGY_CC6,
	TECHNOLOGY_PC6,
	TECHNOLOGY_PKG_CSTATE,
	TECHNOLOGY_IO_MWAIT,
	TECHNOLOGY_IO_MWAIT_REDIR,
	TECHNOLOGY_ODCM,
	TECHNOLOGY_ODCM_DUTYCYCLE,
	TECHNOLOGY_POWER_POLICY,
	TECHNOLOGY_HWP,
	TECHNOLOGY_HWP_EPP,
	TECHNOLOGY_HDC,
	TECHNOLOGY_EEO,
	TECHNOLOGY_R2H,
	TECHNOLOGY_L1_HW_PREFETCH,
	TECHNOLOGY_L1_HW_IP_PREFETCH,
	TECHNOLOGY_L1_SCRUBBING,
	TECHNOLOGY_L2_HW_PREFETCH,
	TECHNOLOGY_L2_HW_CL_PREFETCH,
	TECHNOLOGY_CFG_TDP_LVL,
	TECHNOLOGY_TDP_LIMITING,
	TECHNOLOGY_TDP_OFFSET,
	TECHNOLOGY_TDP_CLAMPING,
	TECHNOLOGY_TDC_LIMITING,
	TECHNOLOGY_TDC_OFFSET,
	TECHNOLOGY_THM_OFFSET,
	TECHNOLOGY_TW_POWER,
	TECHNOLOGY_WDT,
	TECHNOLOGY_HSMP
};

#define COREFREQ_ORDER_MAGIC	0xc6

enum COREFREQ_MAGIC_COMMAND {
/*			Master Ring Commands				*/
	COREFREQ_IOCTL_SYSUPDT		= _IO(COREFREQ_IOCTL_MAGIC, 0x1),
	COREFREQ_IOCTL_SYSONCE		= _IO(COREFREQ_IOCTL_MAGIC, 0x2),
	COREFREQ_IOCTL_MACHINE		= _IO(COREFREQ_IOCTL_MAGIC, 0x3),
	COREFREQ_IOCTL_TECHNOLOGY	= _IO(COREFREQ_IOCTL_MAGIC, 0x4),
	COREFREQ_IOCTL_CPU_OFF		= _IO(COREFREQ_IOCTL_MAGIC, 0x5),
	COREFREQ_IOCTL_CPU_ON		= _IO(COREFREQ_IOCTL_MAGIC, 0x6),
	COREFREQ_IOCTL_TURBO_CLOCK	= _IO(COREFREQ_IOCTL_MAGIC, 0x7),
	COREFREQ_IOCTL_RATIO_CLOCK	= _IO(COREFREQ_IOCTL_MAGIC, 0x8),
	COREFREQ_IOCTL_CONFIG_TDP	= _IO(COREFREQ_IOCTL_MAGIC, 0x9),
	COREFREQ_IOCTL_UNCORE_CLOCK	= _IO(COREFREQ_IOCTL_MAGIC, 0xa),
	COREFREQ_IOCTL_CLEAR_EVENTS	= _IO(COREFREQ_IOCTL_MAGIC, 0xb),
/*			Child Ring Commands				*/
	COREFREQ_ORDER_MACHINE		= _IO(COREFREQ_ORDER_MAGIC, 0x1),
	COREFREQ_ORDER_ATOMIC		= _IO(COREFREQ_ORDER_MAGIC, 0x2),
	COREFREQ_ORDER_CRC32		= _IO(COREFREQ_ORDER_MAGIC, 0x3),
	COREFREQ_ORDER_CONIC		= _IO(COREFREQ_ORDER_MAGIC, 0x4),
	COREFREQ_ORDER_TURBO		= _IO(COREFREQ_ORDER_MAGIC, 0x5),
	COREFREQ_ORDER_MONTE_CARLO	= _IO(COREFREQ_ORDER_MAGIC, 0x6),
	COREFREQ_KERNEL_MISC		= _IO(COREFREQ_ORDER_MAGIC, 0xc),
	COREFREQ_SESSION_APP		= _IO(COREFREQ_ORDER_MAGIC, 0xd),
	COREFREQ_TASK_MONITORING	= _IO(COREFREQ_ORDER_MAGIC, 0xe),
	COREFREQ_TOGGLE_SYSGATE 	= _IO(COREFREQ_ORDER_MAGIC, 0xf)
};

enum PATTERN {
	RESET_CSP,
	ALL_SMT,
	RAND_SMT,
	RR_SMT,
	USR_CPU
};

enum {
	CONIC_ELLIPSOID,
	CONIC_HYPERBOLOID_ONE_SHEET,
	CONIC_HYPERBOLOID_TWO_SHEETS,
	CONIC_ELLIPTICAL_CYLINDER,
	CONIC_HYPERBOLIC_CYLINDER,
	CONIC_TWO_PARALLEL_PLANES,
	CONIC_VARIATIONS
};

enum {
	SESSION_CLI,
	SESSION_GUI
};

enum {
	TASK_TRACKING,
	TASK_SORTING,
	TASK_INVERSING
};

/* Linked list								*/
#define GetPrev(node)		(node->prev)

#define GetNext(node)		(node->next)

#define GetHead(list)		(list)->head

#define SetHead(list, node)	GetHead(list) = node

#define GetTail(list)		(list)->tail

#define SetDead(list)		SetHead(list, NULL)

#define IsHead(list, node)	(GetHead(list) == node)

#define IsDead(list)		(GetHead(list) == NULL)

#define IsCycling(node) (						\
	(GetNext(node) == node) && (GetPrev(node) == node)		\
)

#define GetFocus(list)		GetHead(list)

#define RemoveNodeFromList(node, list)					\
({									\
	GetNext(GetPrev(node)) = GetNext(node); 			\
	GetPrev(GetNext(node)) = GetPrev(node); 			\
})

#define AppendNodeToList(node, list)					\
({									\
	GetPrev(node) = GetHead(list);					\
	GetNext(node) = GetNext(GetHead(list)); 			\
	GetPrev(GetNext(GetHead(list))) = node; 			\
	GetNext(GetHead(list)) = node;					\
})

/* Error Reasons management: Kernel errno is interleaved into reason codes. */
enum REASON_CLASS {
	RC_SUCCESS	= 0,
	RC_OK_SYSGATE	= 1,
	RC_OK_COMPUTE	= 2,
	RC_CMD_SYNTAX	= 3,
	RC_SHM_FILE	= 4,
	RC_SHM_MMAP	= 5,
	RC_PERM_ERR	= 6,
	RC_MEM_ERR	= 7,
	RC_EXEC_ERR	= 8,
	RC_SYS_CALL	= 9,
	/* Source: /include/uapi/asm-generic/errno-base.h @ ERANGE + 1	*/
	RC_DRIVER_BASE		=	35,
	RC_UNIMPLEMENTED	=	RC_DRIVER_BASE + 1,
	RC_EXPERIMENTAL 	=	RC_DRIVER_BASE + 2,
	RC_TURBO_PREREQ 	=	RC_DRIVER_BASE + 3,
	RC_UNCORE_PREREQ	=	RC_DRIVER_BASE + 4,
	RC_PSTATE_NOT_FOUND	=	RC_DRIVER_BASE + 5,
	RC_CLOCKSOURCE		=	RC_DRIVER_BASE + 6,
	RC_DRIVER_LAST		=	RC_DRIVER_BASE + 6
};

/* Definition of Ring structures - The message is 128 bits long.	*/
typedef struct {
	unsigned short	lo: 16,
			hi: 16;
} RING_ARG_DWORD;

typedef union {
    unsigned long long	arg: 64;
    struct {
	RING_ARG_DWORD	dl;
	RING_ARG_DWORD	dh;
    };
} RING_ARG_QWORD;

typedef struct {
	union {
		unsigned long long	arg: 64;
		struct {
			RING_ARG_DWORD	dl;
			RING_ARG_DWORD	dh;
		};
	};
	enum COREFREQ_MAGIC_COMMAND	cmd: 32;
	union {
		unsigned int		sub: 32;
		struct {
			unsigned int	drc:  6-0,/*64 of errno & reason codes*/
					tds: 32-6; /*Epoch time difference sec*/
		};
	};
} RING_CTRL;

#define RING_NULL(Ring) 						\
({									\
	( (Ring.head - Ring.tail) == 0 );				\
})

#define RING_FULL(Ring) 						\
({									\
	( (Ring.head - Ring.tail) == RING_SIZE );			\
})

#if defined(FEAT_DBG) && (FEAT_DBG > 2) && (FEAT_DBG < 100)
FEAT_MSG("Macroing:RING_MOVE(XMM)")
#define RING_MOVE(_dst, _src)						\
({									\
	__asm__ volatile						\
	(								\
		"movdqa %[src] , %%xmm1"	"\n\t"			\
		"movdqa %%xmm1 , %[dst]"				\
		:[dst] "=m" ( _dst )					\
		:[src]  "m" ( _src )					\
		: "%xmm1","memory"					\
	);								\
})
#else
#define RING_MOVE(_dst, _src)						\
({									\
	_dst.arg = _src.arg;						\
	_dst.cmd = _src.cmd;						\
	_dst.sub = _src.sub;						\
})
#endif

#define RING_READ(Ring, _ctrl)						\
({									\
	unsigned int tail = Ring.tail++ & (RING_SIZE - 1);		\
	RING_MOVE(_ctrl, Ring.buffer[tail]);				\
})

#define RING_WRITE_0xPARAM( _sub, Ring, _cmd)				\
({									\
	RING_CTRL ctrl = {						\
			.arg = 0x0LU,					\
			.cmd = (_cmd), .sub = (_sub)			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_1xPARAM( _sub, Ring, _cmd, _arg)			\
({									\
	RING_CTRL ctrl = {						\
			.arg = (_arg),					\
			.cmd = (_cmd), .sub = (_sub)			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_2xPARAM( _sub, Ring, _cmd, _dllo, _dlhi)		\
({									\
	RING_CTRL ctrl = {						\
			.dl = {.lo = (_dllo), .hi = (_dlhi)},		\
			.dh = {.lo = 0x0U , .hi = 0x0U },		\
			.cmd = (_cmd), .sub = (_sub)			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_3xPARAM( _sub, Ring, _cmd, _dllo, _dlhi, _dhlo)	\
({									\
	RING_CTRL ctrl = {						\
			.dl = {.lo = (_dllo), .hi = (_dlhi)},		\
			.dh = {.lo = (_dhlo), .hi = 0x0U },		\
			.cmd = (_cmd), .sub = (_sub)			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_4xPARAM(_sub, Ring, _cmd, _dllo, _dlhi, _dhlo, _dhhi)\
({									\
	RING_CTRL ctrl = {						\
			.dl = {.lo = (_dllo), .hi = (_dlhi)},		\
			.dh = {.lo = (_dhlo), .hi = (_dhhi)},		\
			.cmd = (_cmd), .sub = (_sub)			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_DISPATCH(_1,_2,_3,_4,_5,_6,_7,RING_WRITE_CURSOR, ...)\
	RING_WRITE_CURSOR

#define RING_WRITE_SUB_CMD( ... )					\
	RING_WRITE_DISPATCH(__VA_ARGS__,RING_WRITE_4xPARAM,	/*7*/	\
					RING_WRITE_3xPARAM,	/*6*/	\
					RING_WRITE_2xPARAM,	/*5*/	\
					RING_WRITE_1xPARAM,	/*4*/	\
					RING_WRITE_0xPARAM,	/*3*/	\
						NULL,		/*2*/	\
						NULL)		/*1*/	\
							( __VA_ARGS__ )

#define RING_WRITE( ... ) RING_WRITE_SUB_CMD( 0x0U, __VA_ARGS__ )

enum SMB_STRING {
	SMB_BIOS_VENDOR,
	SMB_BIOS_VERSION,
	SMB_BIOS_RELEASE,
	SMB_SYSTEM_VENDOR,
	SMB_PRODUCT_NAME,
	SMB_PRODUCT_VERSION,
	SMB_PRODUCT_SERIAL,
	SMB_PRODUCT_SKU,
	SMB_PRODUCT_FAMILY,
	SMB_BOARD_VENDOR,
	SMB_BOARD_NAME,
	SMB_BOARD_VERSION,
	SMB_BOARD_SERIAL,
	SMB_PHYS_MEM_ARRAY,
	SMB_MEM_0_LOCATOR,
	SMB_MEM_1_LOCATOR,
	SMB_MEM_2_LOCATOR,
	SMB_MEM_3_LOCATOR,
	SMB_MEM_0_MANUFACTURER,
	SMB_MEM_1_MANUFACTURER,
	SMB_MEM_2_MANUFACTURER,
	SMB_MEM_3_MANUFACTURER,
	SMB_MEM_0_PARTNUMBER,
	SMB_MEM_1_PARTNUMBER,
	SMB_MEM_2_PARTNUMBER,
	SMB_MEM_3_PARTNUMBER,
	SMB_STRING_COUNT
};

typedef union {
		char String[SMB_STRING_COUNT][MAX_UTS_LEN];
	struct {
		struct {
			char	Vendor[MAX_UTS_LEN],
				Version[MAX_UTS_LEN],
				Release[MAX_UTS_LEN];
		} BIOS;
		struct {
			char	Vendor[MAX_UTS_LEN];
		} System;
		struct {
			char	Name[MAX_UTS_LEN],
				Version[MAX_UTS_LEN],
				Serial[MAX_UTS_LEN],
				SKU[MAX_UTS_LEN],
				Family[MAX_UTS_LEN];
		} Product;
		struct {
			char	Vendor[MAX_UTS_LEN],
				Name[MAX_UTS_LEN],
				Version[MAX_UTS_LEN],
				Serial[MAX_UTS_LEN];
		} Board;
		struct {
		    struct {
			char	Array[MAX_UTS_LEN];
		    } Memory;
		} Phys;
		struct {
			char	Locator[MC_MAX_DIMM][MAX_UTS_LEN],
				Manufacturer[MC_MAX_DIMM][MAX_UTS_LEN],
				PartNumber[MC_MAX_DIMM][MAX_UTS_LEN];
		} Memory;
	};
} SMBIOS_ST;

#define ROUND_TO_PAGES(_size)						\
(									\
	(__typeof__(_size)) PAGE_SIZE					\
	* (((_size) + (__typeof__(_size)) PAGE_SIZE - 1)		\
	/ (__typeof__(_size)) PAGE_SIZE)				\
)

#define KMAX(M, m)	((M) > (m) ? (M) : (m))
#define KMIN(m, M)	((m) < (M) ? (m) : (M))

#define StrCopy(_dest, _src, _max)					\
({									\
	size_t _min = KMIN((_max - 1), strlen(_src));			\
	memcpy(_dest, _src, _min);					\
	_dest[_min] = '\0';						\
})

#define StrFormat( _str, _size, _fmt, ... )				\
	snprintf((char*) _str, (size_t) _size, (char*) _fmt, __VA_ARGS__)

#define StrLenFormat( _ret, ... )					\
({									\
	int lret = StrFormat( __VA_ARGS__ );				\
	_ret = lret > 0 ? ( __typeof__ (_ret) ) lret : 0;		\
})

#define ZLIST( ... ) (char *[]) { __VA_ARGS__ , NULL }

#define TIMESPEC(nsec)							\
({									\
	struct timespec tsec = {					\
		.tv_sec  = (time_t) 0,					\
		.tv_nsec = nsec						\
	};								\
	tsec;								\
})

#define COREFREQ_STRINGIFY(_number)	#_number

#define COREFREQ_SERIALIZE(_major, _minor, _rev)			\
	COREFREQ_STRINGIFY(_major)	"."				\
	COREFREQ_STRINGIFY(_minor)	"."				\
	COREFREQ_STRINGIFY(_rev)

#define COREFREQ_VERSION	COREFREQ_SERIALIZE(	COREFREQ_MAJOR, \
							COREFREQ_MINOR, \
							COREFREQ_REV	)

#define COREFREQ_FORMAT_STR(_length) "%" COREFREQ_STRINGIFY(_length) "s"

typedef struct {
    struct {
	unsigned long long max_freq_Hz;
	unsigned short	core_count,
			task_order;
    } builtIn;
	unsigned short	major,
			minor,
			rev;
} FOOTPRINT;

#define SET_FOOTPRINT(_place, _Hz, _CC, _order, _major, _minor, _rev)	\
({									\
	_place.builtIn.max_freq_Hz = _Hz;				\
	_place.builtIn.core_count = _CC;				\
	_place.builtIn.task_order = _order;				\
	_place.major	= _major;					\
	_place.minor	= _minor;					\
	_place.rev	= _rev ;					\
})

#define CHK_FOOTPRINT(_place, _Hz, _CC, _order, _major, _minor, _rev)	\
(									\
	(_place.builtIn.max_freq_Hz == _Hz) &&				\
	(_place.builtIn.core_count == _CC) &&				\
	(_place.builtIn.task_order == _order) &&			\
	(_place.major	== _major) &&					\
	(_place.minor	== _minor) &&					\
	(_place.rev	== _rev)					\
)

#define UNUSED(expr) do { (void)(expr); } while (0)

#define EMPTY_STMT() do { } while (0)

#ifndef fallthrough
	#if defined __has_attribute
		#if __has_attribute(fallthrough)
			#define fallthrough	__attribute__((fallthrough))
		#else
			#define fallthrough	/* Fallthrough */
		#endif
	#else
		#define fallthrough	/* Fallthrough */
	#endif
#endif
