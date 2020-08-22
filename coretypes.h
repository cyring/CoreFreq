/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define COREFREQ_MAJOR	1
#define COREFREQ_MINOR	80
#define COREFREQ_REV	6

#define CORE_COUNT	256

#define CRC_INTEL	0x75a2ba39
#define CRC_AMD 	0x3485bbd3
#define CRC_HYGON	0x18044630
#define CRC_KVM 	0x0e8c8561
#define CRC_VBOX	0x5091f045
#define CRC_KBOX	0x02b76f04
#define CRC_VMWARE	0x2a974552
#define CRC_HYPERV	0x543a585e

enum {	GenuineArch = 0,
	AMD_Family_0Fh,
	AMD_Family_10h,
	AMD_Family_11h,
	AMD_Family_12h,
	AMD_Family_14h,
	AMD_Family_15h,
	AMD_Family_16h,
	AMD_Family_17h,
	AMD_Family_18h,
	Core_Yonah,
	Core_Conroe,
	Core_Kentsfield,
	Core_Conroe_616,
	Core_Penryn,
	Core_Dunnington,
	Atom_Bonnell,
	Atom_Silvermont,
	Atom_Lincroft,
	Atom_Clovertrail,
	Atom_Saltwell,
	Silvermont_637,
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
	Cannonlake,
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
	Atom_C3000,
	Tremont_Jacobsville,
	Tremont_Lakefield,
	Tremont_Elkhartlake,
	Tremont_Jasperlake,
	AMD_Zen,
	AMD_Zen_APU,
	AMD_ZenPlus,
	AMD_ZenPlus_APU,
	AMD_Zen_APU_Rv2,
	AMD_EPYC_Rome,
	AMD_Zen2_CPK,
	AMD_Zen2_APU,
	AMD_Zen2_MTS,
	ARCHITECTURES
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
	CR4_VMXE	= 13,
	CR4_SMXE	= 14,
	CR4_FSGSBASE	= 16,
	CR4_PCIDE	= 17,
	CR4_OSXSAVE	= 18,
	CR4_SMEP	= 20,
	CR4_SMAP	= 21,
	CR4_PKE		= 22,

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
	EXFER_FFXSE	= 14	/* AMD F17h				*/
};

enum THERM_PWR_EVENTS {
	EVENT_THERM_NONE	= 0b0000000,
	EVENT_THERM_SENSOR	= 0b0000001,
	EVENT_THERM_PROCHOT	= 0b0000010,
	EVENT_THERM_CRIT	= 0b0000100,
	EVENT_THERM_THOLD	= 0b0001000,
	EVENT_POWER_LIMIT	= 0b0010000,
	EVENT_CURRENT_LIMIT	= 0b0100000,
	EVENT_CROSS_DOMAIN	= 0b1000000
};

enum {
	SENSOR_LOWEST,
	SENSOR_HIGHEST,
	SENSOR_LIMITS_DIM
};

typedef union
{
	unsigned int	Target;
	unsigned short	Offset[2];
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
THERMAL_FORMULA_AMD_17h = (THERMAL_KIND_AMD_17h << 8)	| FORMULA_SCOPE_PKG
};

enum VOLTAGE_KIND {
	VOLTAGE_KIND_NONE	= 0b000000000000000000000000,
	VOLTAGE_KIND_INTEL	= 0b000000000000000000000001,
	VOLTAGE_KIND_INTEL_CORE2= 0b000000000000000000000011,
	VOLTAGE_KIND_INTEL_SNB	= 0b000000000000000010000001,
	VOLTAGE_KIND_INTEL_SKL_X= 0b000000000000010000000001,
	VOLTAGE_KIND_AMD	= 0b000000000001000000000000,
	VOLTAGE_KIND_AMD_0Fh	= 0b000000000011000000000000,
	VOLTAGE_KIND_AMD_15h	= 0b000001000001000000000000,
	VOLTAGE_KIND_AMD_17h	= 0b000100000001000000000000,
	VOLTAGE_KIND_WINBOND_IO = 0b001000000000000000000000
};

enum VOLTAGE_FORMULAS {
VOLTAGE_FORMULA_NONE       =(VOLTAGE_KIND_NONE << 8)       | FORMULA_SCOPE_NONE,
VOLTAGE_FORMULA_INTEL      =(VOLTAGE_KIND_INTEL << 8)      | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_CORE2=(VOLTAGE_KIND_INTEL_CORE2 << 8)| FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_INTEL_SNB  =(VOLTAGE_KIND_INTEL_SNB << 8)  | FORMULA_SCOPE_PKG,
VOLTAGE_FORMULA_INTEL_SKL_X=(VOLTAGE_KIND_INTEL_SKL_X << 8)| FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD        =(VOLTAGE_KIND_AMD << 8)        | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_0Fh    =(VOLTAGE_KIND_AMD_0Fh << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_15h    =(VOLTAGE_KIND_AMD_15h << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_AMD_17h    =(VOLTAGE_KIND_AMD_17h << 8)    | FORMULA_SCOPE_SMT,
VOLTAGE_FORMULA_WINBOND_IO =(VOLTAGE_KIND_WINBOND_IO << 8) | FORMULA_SCOPE_PKG
};

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

#define SCOPE_OF_FORMULA(formula)	(formula & 0b0011)

#define KIND_OF_FORMULA(formula) ((formula >> 8) & 0b111111111111111111111111)

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

enum PWR_DOMAIN {
	DOMAIN_PKG,
	DOMAIN_CORES,
	DOMAIN_UNCORE,
	DOMAIN_RAM,
	DOMAIN_SIZE
};

#define PWR_DOMAIN(NC) DOMAIN_##NC

#define CACHE_MAX_LEVEL (3 + 1)

#define PRECISION	100

#define UNIT_KHz(_f)		(_f * 10 * PRECISION)
#define UNIT_MHz(_f)		(_f * UNIT_KHz(1000))
#define UNIT_GHz(_f)		(_f * UNIT_MHz(1000))
#define CLOCK_KHz(_t, _f)	(_f / UNIT_KHz((_t) 1))
#define CLOCK_MHz(_t, _f)	(_f / UNIT_MHz((_t) 1))
#define CLOCK_GHz(_t, _f)	(_f / UNIT_GHz((_t) 1))

#if defined(MAX_FREQ_HZ) && (MAX_FREQ_HZ >= 4850000000)
#define MAXCLOCK_TO_RATIO(_typeout, BaseClock) ( (_typeout) (		\
		MAX_FREQ_HZ / BaseClock					\
) )
#else
#define MAXCLOCK_TO_RATIO(_typeout, BaseClock) ( (_typeout) (		\
		5250000000 / BaseClock					\
) )
#endif

enum OFFLINE
{
	HW,
	OS
};

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

#define REL_FREQ_MHz(this_ratio, clock, interval)			\
	(clock.Hz * this_ratio * interval) / UNIT_MHz(interval)

#define ABS_FREQ_MHz(this_type, this_ratio, this_clock) 		\
(									\
	CLOCK_MHz(this_type, this_ratio * this_clock.Hz)		\
)

typedef union {
	signed long long	sllong;
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
		signed int	Thread;
	};
} SERVICE_PROC;

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
	CPUID_8000001E_00000000_EXTENDED_IDENTIFIERS,
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
		unsigned int	CRC;
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
		Unused1 	: 16-14,
		ExtModel	: 20-16,
		ExtFamily	: 28-20,
		Unused2 	: 32-28;
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
		SSE3	:  1-0,  /* AMD Family 0Fh			*/
		PCLMULDQ:  2-1,
		DTES64	:  3-2,
		MONITOR :  4-3,
		DS_CPL	:  5-4,
		VMX	:  6-5,
		SMX	:  7-6,
		EIST	:  8-7,
		TM2	:  9-8,
		SSSE3	: 10-9,  /* AMD Family 0Fh			*/
		CNXT_ID : 11-10,
		SDBG	: 12-11, /* IA32_DEBUG_INTERFACE MSR support	*/
		FMA	: 13-12,
		CMPXCHG16:14-13,
		xTPR	: 15-14,
		PDCM	: 16-15,
		Unused1 : 17-16,
		PCID	: 18-17,
		DCA	: 19-18,
		SSE41	: 20-19,
		SSE42	: 21-20,
		x2APIC	: 22-21,
		MOVBE	: 23-22,
		POPCNT	: 24-23,
		TSCDEAD : 25-24,
		AES	: 26-25,
		XSAVE	: 27-26,
		OSXSAVE : 28-27,
		AVX	: 29-28,
		F16C	: 30-29,
		RDRAND	: 31-30,
		Hyperv	: 32-31;
	} ECX;
	struct
	{	/* Most common x86					*/
		unsigned int
		FPU	:  1-0,
		VME	:  2-1,
		DE	:  3-2,
		PSE	:  4-3,
		TSC	:  5-4,
		MSR	:  6-5,
		PAE	:  7-6,
		MCE	:  8-7,
		CMPXCHG8:  9-8,
		APIC	: 10-9,
		Unused1 : 11-10,
		SEP	: 12-11,
		MTRR	: 13-12,
		PGE	: 14-13,
		MCA	: 15-14,
		CMOV	: 16-15,
		PAT	: 17-16,
		PSE36	: 18-17,
		PSN	: 19-18, /* Intel Processor Serial Number	*/
		CLFLUSH : 20-19,
		Unused2 : 21-20,
		DS_PEBS : 22-21,
		ACPI	: 23-22,
		MMX	: 24-23,
		FXSR	: 25-24, /* FXSAVE and FXRSTOR instructions.	*/
		SSE	: 26-25,
		SSE2	: 27-26,
		SS	: 28-27, /* Intel				*/
		HTT	: 29-28,
		TM1	: 30-29, /* Intel				*/
		Unused3 : 31-30,
		PBE	: 32-31; /* Intel				*/
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
	{	/* Most Intel reserved.					*/
		unsigned int
		DTS	:  1-0, /*Digital temperature sensor availability*/
		TurboIDA:  2-1, /* Reports bit 38 of MSR 0x1a0		*/
		ARAT	:  3-2, /* Common x86				*/
		Unused1 :  4-3,
		PLN	:  5-4, /*Power limit notification controls support*/
		ECMD	:  6-5, /* Clock modulation duty cycle extension*/
		PTM	:  7-6, /* Package thermal management support	*/
		HWP_Reg :  8-7, /* Hardware Performance registers	*/
		HWP_Int :  9-8, /* IA32_HWP_INTERRUPT HWP_Notification. */
		HWP_Act : 10-9, /* IA32_HWP_REQUEST Activity_Window	*/
		HWP_EPP : 11-10,/* IA32_HWP_REQUEST Energy Perf. pref.	*/
		HWP_Pkg : 12-11,/* IA32_HWP_REQUEST_PKG 		*/
		Unused2 : 13-12,
		HDC_Reg : 14-13,/* Hardware Duty Cycling registers	*/
		Turbo_V3: 15-14,/* Intel Turbo Boost Max Technology 3.0 */
		HWP_HPrf: 16-15,/* Highest Performance change support.	*/
		HWP_PECI: 17-16,/* HWP PECI override support state.	*/
		HWP_Flex: 18-17,/* Flexible HWP is support state.	*/
		HWP_Fast: 19-18,/* IA32_HWP_REQUEST MSR fast access mode*/
		HWFB_Cap: 20-19,/* IA32 HW_FEEDBACK* MSR support	*/
		HWP_Idle: 21-20,/* Ignore (or not) Idle SMT Processor.	*/
		Unused4 : 32-21;
	} EAX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Threshld:  4-0, /* Number of Interrupt Thresholds in DTS*/
		Unused1 : 32-4;
	} EBX;
    union
    {
	struct
	{	/* Intel reserved.					*/
		unsigned int
		HCF_Cap :  1-0, /* MSR: IA32_MPERF (E7H) & IA32_APERF (E8H)*/
		ACNT_Cap:  2-1,
		Unused1 :  3-2,
		SETBH	:  4-3, /* MSR: IA32_ENERGY_PERF_BIAS (1B0H)	*/
		Unused2 : 32-4;
	};
	struct
	{	/* AMD reserved.					*/
		unsigned int
		EffFreq :  1-0, /* MSR0000_00E7 (MPERF) & MSR0000_00E8 (APERF)*/
		NotUsed : 32-1;
	};
    } ECX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		HWFB_Cap:  8-7, /* Hardware Feedback Interface bitmap	*/
		HWFB_pSz: 12-8, /* HW Feedback structure size (4K page) */
		Unused1 : 16-12,
		HWFB_Idx: 32-16; /* HW Feedback structure base 0 index	*/
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
		SGX_UMIP	:  3-2, /* Intel SGX Or AMD UMIP	*/
		BMI1		:  4-3, /* Common x86			*/
		HLE		:  5-4, /* Hardware Lock Elision	*/
		AVX2		:  6-5, /* Common x86			*/
		FDP_EXCPTN_x87	:  7-6, /* FPU Data Pointer exceptions	*/
		SMEP		:  8-7, /* x86: Supervisor-Mode exec.	*/
		BMI2		:  9-8, /* Common x86			*/
		FastStrings	: 10-9, /* Enhanced REP MOVSB/STOSB	*/
		INVPCID		: 11-10, /* Process-Context Identifiers */
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
		RDPID		: 23-22, /* AMD RDPID inst. & TSC_AUX MSR */
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
		Reserved1	:  8-7,
		GFNI		:  9-8, /* Galois Field SSE instructions*/
		VAES		: 10-9,
		VPCLMULQDQ	: 11-10,
		AVX512_VNNI	: 12-11,
		AVX512_BITALG	: 13-12,
		Reserved2	: 14-13,
		AVX512_VPOPCNTDQ: 15-14, /* Intel Xeon Phi		*/
		Reserved3	: 17-15,
		MAWAU		: 22-17, /* for BNDLDX & BNDSTX instructions*/
		RDPID		: 23-22, /* Intel RDPID inst. & IA32_TSC_AUX */
		Reserved4	: 25-23,
		CLDEMOTE	: 26-25, /* Support of cache line demote */
		Reserved5	: 27-26,
		MOVDIRI 	: 28-27, /* Move Doubleword as Direct Store*/
		MOVDIR64B	: 29-28, /* Move 64 Bytes as Direct Store*/
		ENQCMD		: 30-29, /* Support of Enqueue Stores	*/
		SGX_LC		: 31-30, /* SGX Launch Configuration support*/
		Reserved6	: 32-31;
	} ECX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Reserved1	:  2-0,
		AVX512_4VNNIW	:  3-2, /* Intel Xeon Phi		*/
		AVX512_4FMAPS	:  4-3, /* Intel Xeon Phi		*/
		FShort_REP_MOV	:  5-4, /* Fast Short REP MOV		*/
		Reserved2	:  8-5,
		AVX512_VP2INTER :  9-8, /* AVX512_VP2INTERSECT		*/
		Reserved3	: 10-9,
		MD_CLEAR_Cap	: 11-10,
		Reserved4	: 13-11,
		TSX_FORCE_ABORT : 14-13, /* MSR TSX_FORCE_ABORT capable	*/
		SERIALIZE	: 15-14, /* SERIALIZE instruction	*/
		Hybrid		: 16-15, /* Hybrid part processor	*/
		TSXLDTRK	: 17-16, /* TSX suspend load address tracking*/
		Reserved5	: 18-17,
		PCONFIG		: 19-18,
		Reserved6	: 26-19,
		IBRS_IBPB_Cap	: 27-26, /* IA32_SPEC_CTRL,IA32_PRED_CMD */
		STIBP_Cap	: 28-27, /* IA32_SPEC_CTRL[1]		*/
		L1D_FLUSH_Cap	: 29-28, /* IA32_FLUSH_CMD		*/
		IA32_ARCH_CAP	: 30-29, /* IA32_ARCH_CAPABILITIES	*/
		IA32_CORE_CAP	: 31-30, /* IA32_CORE_CAPABILITIES	*/
		SSBD_Cap	: 32-31; /* IA32_SPEC_CTRL[2]		*/
	} EDX;
} CPUID_0x00000007;

typedef struct	/* Extended Feature Flags Enumeration Leaf 1		*/
{
	struct
	{
		unsigned int
		Reserved1	:  5-0,
		AVX512_BF16	:  6-5, /* BFLOAT16 support in AVX512	*/
		Reserved2	: 32-6;
	} EAX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Reserved	: 32-0;
	} EBX, ECX, EDX;
} CPUID_0x00000007_1;

typedef struct	/* Extended Feature Flags Leaf equal or greater than 2	*/
{
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Reserved	: 32-0;
	} EAX, EBX, ECX, EDX;
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
		ReservedBits	: 32-7;
	} EBX;
	struct
	{
		unsigned int
		Unused1 	: 32-0;
	} ECX;
	struct
	{
		unsigned int
		FixCtrs 	:  5-0,
		FixWidth	: 13-5,
		Unused1 	: 15-13,
		AnyThread_Dprec : 16-15, /* AnyThread deprecation.	*/
		Unused2 	: 32-16;
	} EDX;
} CPUID_0x0000000a;

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
		Unused1 	: 16-12,
		ExtModel	: 20-16,
		ExtFamily	: 28-20,
		Unused2 	: 32-28;
	} EAX;
	struct { /* AMD reserved, same as CPUID(0x00000001)		*/
		unsigned int
		Brand_ID	:  8-0,
		CLFSH_Size	: 16-8,
		Max_SMT_ID	: 24-16,
		Init_APIC_ID	: 32-24; /* [31-28] PkgType:0=FP6,2=AM4 */
	} EBX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		LAHFSAHF:  1-0,  /* LAHF and SAHF instruction support.	*/
		Unused1 :  5-1,
		LZCNT	:  6-5,
		Unused2 :  8-6,
		PREFETCHW: 9-8,
		Unused3 : 32-9;
	};
	struct { /* AMD reserved.					*/
		unsigned int
		/* Family 0Fh :						*/
		LahfSahf:  1-0,
		MP_Mode :  2-1,  /* Core multi-processing legacy mode.	*/
		SVM	:  3-2,  /* Secure virtual machine.		*/
		Ext_APIC:  4-3,  /* Extended APIC space.		*/
		AltMov	:  5-4,  /* AltMovCr8				*/
		ABM	:  6-5,  /* LZCNT instruction support.		*/
		SSE4A	:  7-6,
		AlignSSE:  8-7,  /* Misaligned SSE mode.		*/
		PREFETCH:  9-8,  /* 3DNow PREFETCH, PREFETCHW instruction. */
		/* Families [15h - 17h]:				*/
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
		TopoExt : 23-22, /* Topology extensions support.	*/
		PerfCore: 24-23, /* PerfCtrExtCore MSR.			*/
		PerfNB	: 25-24, /* PerfCtrExtNB MSR.			*/
		NotUsed4: 26-25,
		Data_BP : 27-26, /* Data access breakpoint extension.	*/
		PerfTSC : 28-27, /* Performance TSC MSR.		*/
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
		Unused1 : 11-0,
		SYSCALL : 12-11,
		Unused2 : 20-12,
		XD_Bit	: 21-20,
		Unused3 : 26-21,
		PG_1GB	: 27-26,
		RdTSCP	: 28-27,
		Unused4 : 29-28,
		IA64	: 30-29,
		Unused5 : 32-30;
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
		Reserved: 32-0;
	} EAX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		Reserved: 32-0;
	};
	struct { /* AMD as April 2020					*/
		unsigned int
		MCA_Ovf :  1-0,  /* MCA overflow recovery support	*/
		SUCCOR	:  2-1,  /* SW uncorrectable error & recovery cap. */
		HWA	:  3-2,  /* Hardware Assert MSR 0xc00110[df:c0].*/
		Scal_MCA:  4-3,  /* ScalableMca w/ 1 is supported	*/
		Rsvd_AMD: 32-4;
	};
    } EBX;
    union
    {
	struct { /* Intel reserved.					*/
		unsigned int
		Reserved: 32-0;
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
		Unused1 :  8-0,
		Inv_TSC :  9-8,  /* Invariant TSC available if 1	*/
		Unused2 : 32-9;
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
	struct { /* AMD Family 15h					*/
		unsigned int
		Fam_0Fh :  7-0,  /* Family 0Fh features.		*/
		HwPstate:  8-7,  /* Hardware P-state control MSR 0xc0010061-63*/
		TscInv	:  9-8,  /* Invariant TSC ?			*/
		CPB	: 10-9,  /* Core performance boost.		*/
		EffFrqRO: 11-10, /* Read-only effective freq. interf. msr ?   */
		ProcFb	: 12-11, /* Processor feedback interf. available if 1 */
		ProcPwr : 13-12, /* Core power reporting interface supported. */
		ConStdBy: 14-13, /* ConnectedStandby			*/
		RAPL	: 15-14, /* RAPL support ?			*/
		Reserved: 32-15;
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
	{	/* AMD Family 17h					*/
		unsigned int
		CLZERO		:  1-0,  /* Clear Zero Instruction	*/
		IRPerf		:  2-1,  /* Inst. Retired Counter support */
		XSaveErPtr	:  3-2,  /* FX___ error pointers support */
		Reserved1	:  4-3,
		RDPRU		:  5-4,  /* MPERF/APERF at user level	*/
		Reserved2	:  6-5,
		MBE		:  7-6,  /* Memory Bandwidth Enforcement */
		Reserved3	:  8-7,
		MCOMMIT 	:  9-8,  /* Memory Commit Instruction	*/
		WBNOINVD	: 10-9,
		Reserved4	: 12-10,
		IBPB		: 13-12, /* Indirect Branch Prediction Barrier*/
		INT_WBINVD	: 14-13, /* Interruptible WBINVD,WBNOINVD */
		IBRS		: 15-14, /* IBR Speculation		*/
		STIBP		: 16-15, /* Single Thread Indirect Branch Pred*/
		Reserved5	: 17-16,
		STIBP_AlwaysOn	: 18-17,
		IBRS_Preferred	: 19-18,
		IBRS_ProtectMode: 20-19,
		Reserved6	: 23-20,
		PPIN		: 24-23, /* Protected Processor Inventory Num */
		SSBD		: 25-24, /* Speculative Store Bypass Disable */
		Reserved	: 32-25;
	} EBX;
	struct { /* AMD reserved					*/
		unsigned int
		NC		:  8-0,  /* Zero based number of threads */
		Reserved1	: 12-8,
		ApicIdCoreIdSize: 16-12,/* Initial APIC ID size to compute MNC*/
		PerfTscSize	: 18-16, /* 00b=40,01b=48,10b=56,11b=64 bits  */
		Reserved2	: 32-18;
	} ECX;
	struct
	{	/* AMD Family 17h					*/
		unsigned int
		Reserved1	: 16-0,
		RdpruMax	: 24-16, /* RDPRU Instruction max input */
		Reserved2	: 32-24;
	} EDX;
} CPUID_0x80000008;

typedef struct	/* AMD Extended ID Leaf.				*/
{ /* Remark: all registers valid if CPUID(0x80000001).ECX.TopoEx == 1	*/
	struct {
		unsigned int
		ExtApicId	: 32-0;/* Valid if MSR(APIC_BAR[ApicEn]) != 0 */
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

typedef struct	/* BSP CPUID features.					*/
{
	CPUID_FUNCTION		Info;

	CPUID_0x00000001	Std;
	CPUID_0x00000005	MWait;
	CPUID_0x00000006	Power;
	CPUID_0x00000007	ExtFeature;
	CPUID_0x00000007_1	ExtFeature_Leaf1;
	CPUID_0x0000000a	PerfMon;
	CPUID_0x80000001	ExtInfo;
	CPUID_0x80000007	AdvPower;
	CPUID_0x80000008	leaf80000008;

	struct {
		CLOCK		Clock;
		unsigned int	Freq,
				Ratio;
	} Factory;

	struct {
		Bit32	InvariantTSC	:  8-0,
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
			SpecTurboRatio	: 32-24;
	};
} FEATURES;


/* Memory Controller' structures dimensions.				*/
#define MC_MAX_CTRL	2
#define MC_MAX_CHA	8
#define MC_MAX_DIMM	4

typedef union
{
	struct {
		unsigned int
		tCL,
		tRCD,
		tRP,
		tRAS,
		tRRD,
		tRFC,
		tWR,
		tRTPr,
		tWTPr,
		tFAW,
		B2B,
		tCWL,
		CMD_Rate,
		tsrRdTRd,
		tdrRdTRd,
		tddRdTRd,
		tsrRdTWr,
		tdrRdTWr,
		tddRdTWr,
		tsrWrTRd,
		tdrWrTRd,
		tddWrTRd,
		tsrWrTWr,
		tdrWrTWr,
		tddWrTWr,
		ECC;
	};
	struct {
		unsigned int
		tCL,
		tRCD_RD,
		tRCD_WR,
		tRP,
		tRAS,
		tRFC,
		TrrdS,
		TrrdL,
		tFAW,
		TwtrS,
		TwtrL,
		tWR,
		tRdRdScl,
		tWrWrScl,
		tCWL,
		tRTP,
		tddRdTWr,
		tddWrTRd,
		tscWrTWr,
		tsdWrTWr,
		tddWrTWr,
		tscRdTRd,
		tsdRdTRd,
		tddRdTRd,
		ECC,
		CMD_Rate;
	} DDR4;
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
#ifndef _LINUX_CPUFREQ_H
#define CPUFREQ_NAME_LEN	16
#endif

#define REGISTRATION_DISABLE	0b00
#define REGISTRATION_ENABLE	0b01
#define REGISTRATION_FULLCTRL	0b10

typedef struct {	/* 0: Disable; 1: Enable; 2: Full-control	*/
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
	short int		state;		/* TASK_STATE_MAX = 0x1000 */
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

#define SYSGATE_STRUCT_SIZE	( sizeof(OS_DRIVER)			\
				+ sizeof(int)				\
				+ sizeof(MEM_MCB)			\
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

#define COREFREQ_IOCTL_MAGIC 0xc3

#define COREFREQ_IOCTL_SYSUPDT		_IO(COREFREQ_IOCTL_MAGIC, 0x1)
#define COREFREQ_IOCTL_SYSONCE		_IO(COREFREQ_IOCTL_MAGIC, 0x2)
#define COREFREQ_IOCTL_MACHINE		_IO(COREFREQ_IOCTL_MAGIC, 0x3)
#define COREFREQ_IOCTL_TECHNOLOGY	_IO(COREFREQ_IOCTL_MAGIC, 0x4)
#define COREFREQ_IOCTL_CPU_OFF		_IO(COREFREQ_IOCTL_MAGIC, 0x5)
#define COREFREQ_IOCTL_CPU_ON		_IO(COREFREQ_IOCTL_MAGIC, 0x6)
#define COREFREQ_IOCTL_TURBO_CLOCK	_IO(COREFREQ_IOCTL_MAGIC, 0x7)
#define COREFREQ_IOCTL_RATIO_CLOCK	_IO(COREFREQ_IOCTL_MAGIC, 0x8)
#define COREFREQ_IOCTL_UNCORE_CLOCK	_IO(COREFREQ_IOCTL_MAGIC, 0x9)
#define COREFREQ_IOCTL_CLEAR_EVENTS	_IO(COREFREQ_IOCTL_MAGIC, 0xa)

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
	MACHINE_FORMULA_SCOPE
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
	TECHNOLOGY_HWP_EPP
};

#define COREFREQ_ORDER_MAGIC 0xc6

#define COREFREQ_ORDER_MACHINE	_IO(COREFREQ_ORDER_MAGIC, 0x1)
#define COREFREQ_ORDER_ATOMIC	_IO(COREFREQ_ORDER_MAGIC, 0x2)
#define COREFREQ_ORDER_CRC32	_IO(COREFREQ_ORDER_MAGIC, 0x3)
#define COREFREQ_ORDER_CONIC	_IO(COREFREQ_ORDER_MAGIC, 0x4)
#define COREFREQ_ORDER_TURBO	_IO(COREFREQ_ORDER_MAGIC, 0x5)

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
	RC_DRIVER_BASE	= 35,
	RC_UNIMPLEMENTED= RC_DRIVER_BASE + 1,
	RC_EXPERIMENTAL = RC_DRIVER_BASE + 2,
	RC_TURBO_PREREQ = RC_DRIVER_BASE + 3,
	RC_UNCORE_PREREQ= RC_DRIVER_BASE + 4,
	RC_DRIVER_LAST	= RC_DRIVER_BASE + 4
};

/* Definition of Ring structures - The message is 128 bits long.	*/
typedef struct {
	unsigned short	lo: 16,
			hi: 16;
} RING_ARG_DWORD;

typedef union {
	unsigned long	arg: 64;
    struct {
	RING_ARG_DWORD	dl;
	RING_ARG_DWORD	dh;
    };
} RING_ARG_QWORD;

typedef struct {
	union {
		unsigned long	arg: 64;
	    struct {
		RING_ARG_DWORD	dl;
		RING_ARG_DWORD	dh;
	    };
	};
	unsigned int		cmd: 32;
	union {
		unsigned int	sub: 32;
	    struct {
		unsigned int	drc:  6-0, /* 64 of errno & reason codes */
				tds: 32-6; /* Epoch time difference sec */
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

#if FEAT_DBG > 2
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
			.cmd = _cmd, .sub = _sub			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_1xPARAM( _sub, Ring, _cmd, _arg)			\
({									\
	RING_CTRL ctrl = {						\
			.arg = _arg,					\
			.cmd = _cmd, .sub = _sub			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_2xPARAM( _sub, Ring, _cmd, _dllo, _dlhi)		\
({									\
	RING_CTRL ctrl = {						\
			.dl = {.lo = _dllo, .hi = _dlhi},		\
			.dh = {.lo = 0x0U , .hi = 0x0U },		\
			.cmd = _cmd, .sub = _sub			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_3xPARAM( _sub, Ring, _cmd, _dllo, _dlhi, _dhlo)	\
({									\
	RING_CTRL ctrl = {						\
			.dl = {.lo = _dllo, .hi = _dlhi},		\
			.dh = {.lo = _dhlo, .hi = 0x0U },		\
			.cmd = _cmd, .sub = _sub			\
	};								\
	unsigned int head = Ring.head++ & (RING_SIZE - 1);		\
	RING_MOVE(Ring.buffer[head], ctrl);				\
})

#define RING_WRITE_4xPARAM(_sub, Ring, _cmd, _dllo, _dlhi, _dhlo, _dhhi)\
({									\
	RING_CTRL ctrl = {						\
			.dl = {.lo = _dllo, .hi = _dlhi},		\
			.dh = {.lo = _dhlo, .hi = _dhhi},		\
			.cmd = _cmd, .sub = _sub			\
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
	SMB_BOARD_NAME,
	SMB_BOARD_VERSION,
	SMB_BOARD_SERIAL,
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
			char	Name[MAX_UTS_LEN],
				Version[MAX_UTS_LEN],
				Serial[MAX_UTS_LEN];
		} Board;
	};
} SMBIOS_ST;

#define ROUND_TO_PAGES(Size)	PAGE_SIZE * ((Size / PAGE_SIZE) 	\
				+ ((Size % PAGE_SIZE)? 1:0))

#define KMAX(M, m)	((M) > (m) ? (M) : (m))
#define KMIN(m, M)	((m) < (M) ? (m) : (M))

#define StrCopy(_dest, _src, _max)					\
({									\
	size_t _min = KMIN((_max - 1), strlen(_src));			\
	memcpy(_dest, _src, _min);					\
	_dest[_min] = '\0';						\
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

#define FEAT_MESSAGE(_msg)		_Pragma(#_msg)
#define FEAT_MSG(_msg)			FEAT_MESSAGE(message(#_msg))

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
	unsigned short	major,
			minor,
			rev;
} FOOTPRINT;

#define SET_FOOTPRINT(_place, _major, _minor, _rev)			\
({									\
	_place.major	= _major;					\
	_place.minor	= _minor;					\
	_place.rev	= _rev ;					\
})

#define CHK_FOOTPRINT(_place, _major, _minor, _rev)			\
(									\
	(_place.major	== _major) &&					\
	(_place.minor	== _minor) &&					\
	(_place.rev	== _rev)					\
)

