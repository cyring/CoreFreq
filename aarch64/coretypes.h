/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#if !defined(CORE_COUNT)
	#define CORE_COUNT	256
#elif !(CORE_COUNT == 64 || CORE_COUNT == 128 || CORE_COUNT == 256 \
	|| CORE_COUNT == 512 || CORE_COUNT == 1024)
	#error "CORE_COUNT must be 64, 128, 256, 512 or 1024"
#endif

enum CRC_MANUFACTURER
{
	CRC_RESERVED	= 0x0,
	CRC_ARM 	= 0x1,
	CRC_BROADCOM	= 0x2,
	CRC_CAVIUM	= 0x3,
	CRC_DEC 	= 0x4,
	CRC_FUJITSU	= 0x5,
	CRC_INFINEON	= 0x6,
	CRC_MOTOROLA	= 0x7,
	CRC_NVIDIA	= 0x8,
	CRC_APM 	= 0x9,
	CRC_QUALCOMM	= 0xa,
	CRC_MARVELL	= 0xb,
	CRC_INTEL	= 0x75a2ba39,
	CRC_AMPERE	= 0xc,
	/* Virtualization */
	CRC_KVM 	= 0x0e8c8561,
	CRC_VBOX	= 0x5091f045,
	CRC_KBOX	= 0x02b76f04,
	CRC_VMWARE	= 0x2a974552,
	CRC_HYPERV	= 0x543a585e
};

enum CODENAME
{
	ARM64,
	ARMv8_R,
	ARMv8_A,
	ARMv8_2_A,
	ARMv8_3_A,
	ARMv8_4_A,
	ARMv8_5,
	ARMv8_6,
	ARMv8_7,
	ARMv9_A,
	ARMv9_4,
	ARMv9_5,
	CODENAMES
};

enum {	GenuineArch = 0,
	Cortex_A34,
	Cortex_A35,
	Cortex_A510,
	Cortex_A520,
	Cortex_A53,
	Cortex_A55,
	Cortex_A57,
	Cortex_A65,
	Cortex_A65AE,
	Cortex_A710,
	Cortex_A715,
	Cortex_A72,
	Cortex_A720,
	Cortex_A73,
	Cortex_A75,
	Cortex_A76,
	Cortex_A76AE,
	Cortex_A77,
	Cortex_A78,
	Cortex_A78AE,
	Cortex_A78C,
	Cortex_R82,
	Cortex_X1,
	Cortex_X1C,
	Cortex_X2,
	Cortex_X3,
	Cortex_X4,
	DynamIQ_DSU,
	Neoverse_E1,
	Neoverse_N1,
	Neoverse_N2,
	Neoverse_V1,
	Neoverse_V2,
	ARCHITECTURES
};

enum HYBRID_ARCH {
	Hybrid_Primary ,	/*	Big	*/
	Hybrid_Secondary	/*	Little	*/
};

enum MECH_CSV2 {
	CSV_NONE,
	CSV2_1p0,
	CSV2_1p1,
	CSV2_1p2,
	CSV2_2p0,
	CSV2_3p0
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
	FLAG_SP 	= 0,	/* [1:0] = [ZA:SM]			*/
	FLAG_EL 	= 2,	/* [3:2]	 			*/
	FLAG_F		= 6,
	FLAG_I		= 7,
	FLAG_A		= 8,
	FLAG_D		= 9,
	FLAG_SSBS	= 12,
	FLAG_NMI	= 13,
	FLAG_PAN	= 22,
	FLAG_UAO	= 23,
	FLAG_DIT	= 24,
	FLAG_TCO	= 25,
	FLAG_V		= 28,
	FLAG_C		= 29,
	FLAG_Z		= 30,
	FLAG_N		= 31,
	FLAG_PM 	= 32,

	SCTLR_TIDCP	= 63,
	SCTLR_SPINT	= 62,
	SCTLR_NMI	= 61,
	SCTLR_EnTP2	= 60,
	SCTLR_TCSO1	= 59,
	SCTLR_TCSO0	= 58,
	SCTLR_EPAN	= 57,
	SCTLR_EnALS	= 56,
	SCTLR_EnAS0	= 55,
	SCTLR_EnASR	= 54,
	SCTLR_TME1	= 53,
	SCTLR_TME0	= 52,
	SCTLR_TMT1	= 51,
	SCTLR_TMT0	= 50,
	SCTLR_TWEDEL	= 46,	/* [49:46]				*/
	SCTLR_TWEDEn	= 45,
	SCTLR_DSSBS	= 44,
	SCTLR_ATA1	= 43,
	SCTLR_ATA0	= 42,
	SCTLR_TCF1	= 40,	/* [41:40]				*/
	SCTLR_TCF0	= 38,	/* [39:38]				*/
	SCTLR_ITFSB	= 37,
	SCTLR_BT1	= 36,
	SCTLR_BT0	= 35,
	SCTLR_EnFPM	= 34,
	SCTLR_MSCEn	= 33,
	SCTLR_CMOW	= 32,
	SCTLR_EnIA	= 31,
	SCTLR_EnIB	= 30,
	SCTLR_LSMAOE	= 29,
	SCTLR_nTLSMD	= 28,
	SCTLR_EnDA	= 27,
	SCTLR_UCI	= 26,
	SCTLR_EE	= 25,
	SCTLR_E0E	= 24,
	SCTLR_SPAN	= 23,
	SCTLR_EIS	= 22,
	SCTLR_IESB	= 21,
	SCTLR_TSCXT	= 20,
	SCTLR_WXN	= 19,
	SCTLR_nTWE	= 18,
	SCTLR_nTWI	= 16,
	SCTLR_UCT	= 15,
	SCTLR_DZE	= 14,
	SCTLR_EnDB	= 13,
	SCTLR_I 	= 12,
	SCTLR_EOS	= 11,
	SCTLR_EnRCTX	= 10,
	SCTLR_UMA	= 9,
	SCTLR_SED	= 8,
	SCTLR_ITD	= 7,
	SCTLR_nAA	= 6,
	SCTLR_CP15B	= 5,
	SCTLR_SA0	= 4,
	SCTLR_SA1	= 3,
	SCTLR_C 	= 2,
	SCTLR_A 	= 1,
	SCTLR_M 	= 0,

	SCTLR2_CPTM0	= 12,
	SCTLR2_CPTM1	= 11,
	SCTLR2_CPTA0	= 10,
	SCTLR2_CPTA1	= 9,
	SCTLR2_EnPACM0	= 8,
	SCTLR2_EnPACM1	= 7,
	SCTLR2_IDCP128	= 6,
	SCTLR2_EASE	= 5,
	SCTLR2_EnANERR	= 4,
	SCTLR2_EnADERR	= 3,
	SCTLR2_NMEA	= 2,

	EL0_64		= 0,
	EL0_32		= 1,
	EL1_64		= 2,
	EL1_32		= 3,
	EL2_64		= 4,
	EL2_32		= 5,
	EL2_SEC 	= 6,
	EL3_64		= 7,
	EL3_32		= 8,

	FPSR_N		= 31,
	FPSR_Z		= 30,
	FPSR_C		= 29,
	FPSR_V		= 28,
	FPSR_QC 	= 27,
	FPSR_IDC	= 7,
	FPSR_IXC	= 4,
	FPSR_UFC	= 3,
	FPSR_OFC	= 2,
	FPSR_DZC	= 1,
	FPSR_IOC	= 0,

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

#define	INST_COUNTER_OVERFLOW	0x7fffffffLLU

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
	THERMAL_KIND_NONE	= 0b000000000000000000000000
};

enum THERMAL_FORMULAS {
	THERMAL_FORMULA_NONE = (THERMAL_KIND_NONE << 8) | FORMULA_SCOPE_NONE
};

enum VOLTAGE_KIND {
	VOLTAGE_KIND_NONE	= 0b000000000000000000000000,
	VOLTAGE_KIND_OPP	= 0b000000000000000000000001
};

enum VOLTAGE_FORMULAS {
	VOLTAGE_FORMULA_NONE = (VOLTAGE_KIND_NONE << 8) | FORMULA_SCOPE_NONE,
	VOLTAGE_FORMULA_OPP  = (VOLTAGE_KIND_OPP << 8)  | FORMULA_SCOPE_SMT
};

enum POWER_KIND {
	POWER_KIND_NONE 	= 0b000000000000000000000000
};

enum POWER_FORMULAS {
	POWER_FORMULA_NONE = (POWER_KIND_NONE << 8) | FORMULA_SCOPE_NONE
};

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

#define COMPUTE_THERMAL(_ARCH_, Temp, Param, Sensor)			\
	COMPUTE_THERMAL_##_ARCH_(Temp, Param, Sensor)

#define COMPUTE_VOLTAGE_OPP(Vcore, VID) 				\
		(Vcore = (double) (VID << 5) / 1000000.0)

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
	RATIO_TBO = RATIO_TDP1,
	RATIO_TBH = RATIO_TDP2,
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
	#define MAX_FREQ_HZ	7125000000
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

typedef union {
	struct
	{
		unsigned int	Q :  8-0,
				R : 32-8;
	};
} COF_ST;

typedef struct
{
	unsigned long long	Q,
				R,
				Hz;
} CLOCK;

#define REL_BCLK(clock, ratio, delta_tsc, interval)			\
({	/*		Compute Clock (Hertz)			*/	\
	clock.Hz= (1000LLU * PRECISION * delta_tsc)			\
		/ (interval * ratio);					\
	/*		Compute Quotient (MHz)			*/	\
	clock.Q = clock.Hz / (1000LLU * 1000LLU);			\
	/*		Compute Remainder (MHz) 		*/	\
	clock.R = clock.Hz % (1000LLU * 1000LLU);			\
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

struct SIGNATURE {
	unsigned int
	Stepping	:  8-0,
	Model		: 12-8,
	Family		: 16-12,
	ExtModel	: 20-16,
	ExtFamily	: 28-20,
	Reserved	: 32-28;
};

#define BRAND_PART	12
#define BRAND_LENGTH	(4 * BRAND_PART)
#define BRAND_SIZE	(BRAND_LENGTH + 4)

typedef struct
{
	struct SIGNATURE	Signature;
	struct {
	enum CRC_MANUFACTURER	CRC;
		char		ID[12 + 4];
	} Vendor, Hypervisor;
	char			Brand[BRAND_SIZE];
} PROCESSOR_ID;

struct CLUSTER_ST {
	union {
		unsigned int	ID;
		struct {
		unsigned int	Node	:  8-0,
				CCX	: 16-8,
				CCD	: 24-16,
				CMP	: 32-24;
		};
	};
	enum HYBRID_ARCH	Hybrid_ID;
};

typedef struct	/* BSP features.					*/
{
	PROCESSOR_ID		Info;

	struct {
		Bit64
			AES		:  1-0,
			SHA1		:  2-1,
			SHA256		:  3-2,
			SHA512		:  4-3,
			CRC32		:  5-4,
			LSE		:  6-5,
			SHA3		:  7-6,
			RAND		:  8-7,
			FP		:  9-8,
			SIMD		: 10-9,
			GIC_vers	: 11-10,
			SVE		: 12-11,
			VHE		: 13-12,
			SME		: 14-13,
			TME		: 15-14,
			RDMA		: 16-15,
			DP		: 17-16,
			SM3		: 18-17,
			SM4		: 19-18,
			FHM		: 20-19,
			LSE128		: 21-20,
			TLBIOS		: 22-21,
			FCMA		: 23-22,
			LRCPC		: 24-23,
			JSCVT		: 25-24,
			FRINTTS 	: 26-25,
			SPECRES		: 27-26,
			SPECRES2	: 28-27,
			BF16		: 29-28,
			EBF16		: 30-29,
			I8MM		: 31-30,
			SB		: 32-31,
			XS		: 33-32,
			LS64		: 34-33,
			LS64_V		: 35-34,
			LS64_ACCDATA	: 36-35,
			DGH		: 37-36,
			DPB		: 38-37,
			DPB2		: 39-38,
			FlagM		: 40-39,
			FlagM2		: 41-40,
			PMULL		: 42-41,
			GIC_frac	: 43-42,
			ECV		: 44-43,
			FGT		: 45-44,
			FGT2		: 46-45,
			ExS		: 47-46,
			BigEnd_EL0	: 48-47,
			BigEnd_EE	: 49-48,
			PARange 	: 53-49,
			VARange 	: 62-59,
			TLBIRANGE	: 63-62,
			PACIMP		: 64-63;

		Bit64	CSV2		:  4-0,
			SSBS		:  8-4,
			PAN		:  9-8,
			UAO		: 10-9,
			DIT		: 11-10,
			BTI		: 12-11,
			NMI		: 13-12,
			EBEP		: 14-13,
			RAS		: 15-14,
			RAS_frac	: 16-15,
			MPAM_vers	: 17-16,
			MPAM_frac	: 18-17,
			AMU_vers	: 19-18,
			AMU_frac	: 20-19,
			RME		: 21-20,
			SEL2		: 22-21,
			ECBHB		: 23-22,
			GCS		: 24-23,
			THE		: 25-24,
			SVE_F64MM	: 26-25,
			SVE_F32MM	: 27-26,
			SVE_I8MM	: 28-27,
			SVE_SM4 	: 29-28,
			SVE_SHA3	: 30-29,
			SVE_BF16	: 31-30,
			SVE_EBF16	: 32-31,
			SVE_BitPerm	: 33-32,
			SVE_AES 	: 34-33,
			SVE_PMULL128	: 35-34,
			SVE2		: 36-35,
			SME_FA64	: 37-36,
			SME_LUTv2	: 38-37,
			SME2		: 39-38,
			SME2p1		: 40-39,
			SME_I16I64	: 41-40,
			SME_F64F64	: 42-41,
			SME_I16I32	: 43-42,
			SME_B16B16	: 44-43,
			SME_F16F16	: 45-44,
			SME_F8F16	: 46-45,
			SME_F8F32	: 47-46,
			SME_I8I32	: 48-47,
			SME_F16F32	: 49-48,
			SME_B16F32	: 50-49,
			SME_BI32I32	: 51-50,
			SME_F32F32	: 52-51,
			SME_SF8FMA	: 53-52,
			SME_SF8DP4	: 54-53,
			SME_SF8DP2	: 55-54,
			PACQARMA5	: 56-55,
			LRCPC2		: 57-56,
			LRCPC3		: 58-57,
			PACQARMA3	: 59-58,
			PAuth		: 60-59,
			EPAC		: 61-60,
			PAuth2		: 62-61,
			FPAC		: 63-62,
			FPACCOMBINE	: 64-63;

		Bit64	PAuth_LR	:  1-0,
			WFxT		:  2-1,
			RPRES		:  3-2,
			MOPS		:  4-3,
			HBC		:  5-4,
			SYSREG128	:  6-5,
			SYSINSTR128	:  7-6,
			PRFMSLC 	:  8-7,
			RPRFM		:  9-8,
			CSSC		: 10-9,
			LUT		: 11-10,
			ATS1A		: 12-11,
			CONSTPACFIELD	: 13-12,
			RNG_TRAP	: 14-13,
			MTE		: 17-14,
			DF2		: 18-17,
			PFAR		: 19-18,
			_Unused1_	: 64-19;

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
			Turbo_OPP	: 21-20,
			Uncore_Unlock	: 22-21,
			HWP_Enable	: 23-22,
			Other_Capable	: 24-23,
			SpecTurboRatio	: 32-24,
			HTT		: 33-32,
			TSC		: 34-33,
			MONITOR 	: 35-34,
			Inv_TSC 	: 36-35,
			RDTSCP		: 37-36,
			Hybrid		: 38-37,
			ACPI		: 39-38,
			Hyperv		: 40-39,
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
			_Unused2_	: 64-60;
	};
	struct
	{
		unsigned int
		SubCstate_MWAIT0:  4-0,
		SubCstate_MWAIT1:  8-4,
		SubCstate_MWAIT2: 12-8,
		SubCstate_MWAIT3: 16-12,
		SubCstate_MWAIT4: 20-16,
		SubCstate_MWAIT5: 24-20,
		SubCstate_MWAIT6: 28-24,
		SubCstate_MWAIT7: 32-28;
	} MWait;

	struct {
		unsigned int
		Version 	:  4-0,
		MonCtrs 	:  9-4,
		MonWidth	: 17-9,
		FixCtrs 	: 22-17,
		FixWidth	: 30-22,
		CoreCycles	: 31-30,
		InstrRetired	: 32-31;
	} PerfMon;

	struct {
		unsigned short	CG0NC,
				CG1NC;
	} AMU;

	struct
	{
		unsigned int
		DTS		:  1-0,
		_Unused1_	:  3-1,
		PLN		:  4-3,
		PTM		:  5-4,
		HWP_Reg 	:  6-5,
/*		HCF_Cap 	:  7-6,
		HWP_Int 	:  1
		HWP_Act 	:  1
		HWP_EPP 	:  1
		HWP_Pkg 	:  1
		HWP_HPrf	:  1
		HWP_PECI	:  1
		HWP_Flex	:  1
		HWP_Fast	:  1
		HWFB_Cap	:  1
		HWP_Idle	:  1	*/
		_Unused2_	: 32-6;
	} Power;

	struct {
		unsigned long long	PPIN;
			CLOCK		Clock;
			unsigned int	Freq,
					Ratio;
	} Factory;
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
	TECHNOLOGY_HWP,
	TECHNOLOGY_HWP_EPP,
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
/*	COREFREQ_IOCTL_<FREE>		= _IO(COREFREQ_IOCTL_MAGIC, 0x9),*/
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
