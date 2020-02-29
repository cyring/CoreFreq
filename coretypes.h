/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define COREFREQ_MAJOR	1
#define COREFREQ_MINOR	74
#define COREFREQ_REV	1

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

#define CORE_COUNT	256

enum {	GenuineIntel,
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
	Icelake_UY,
	AMD_Family_0Fh,
	AMD_Family_10h,
	AMD_Family_11h,
	AMD_Family_12h,
	AMD_Family_14h,
	AMD_Family_15h,
	AMD_Family_16h,
	AMD_Family_17h,
	AMD_Family_18h,
	ARCHITECTURES
};

enum CHIPSET {
	IC_CHIPSET,
	IC_LAKEPORT,
	IC_LAKEPORT_P,
	IC_LAKEPORT_X,
	IC_CALISTOGA,
	IC_BROADWATER,
	IC_CRESTLINE,
	IC_CANTIGA,
	IC_BEARLAKE_Q,
	IC_BEARLAKE_P,
	IC_BEARLAKE_QF,
	IC_BEARLAKE_X,
	IC_INTEL_3200,
	IC_EAGLELAKE_Q,
	IC_EAGLELAKE_P,
	IC_EAGLELAKE_G,
	IC_TYLERSBURG,
	IC_IBEXPEAK,
	IC_IBEXPEAK_M,
	IC_COUGARPOINT,
	IC_PATSBURG,
	IC_CAVECREEK,
	IC_WELLSBURG,
	IC_PANTHERPOINT,
	IC_PANTHERPOINT_M,
	IC_LYNXPOINT,
	IC_LYNXPOINT_M,
	IC_WILDCATPOINT,
	IC_WILDCATPOINT_M,
	IC_SUNRISEPOINT,
	IC_UNIONPOINT,
	IC_CANNONPOINT,
	IC_K8,
	IC_ZEN,
	CHIPSETS
};

enum CSTATES_CLASS {
	CSTATES_NHM	= 0x1,
	CSTATES_SNB	= 0x2,
	CSTATES_ULT	= 0x4,
	CSTATES_SKL	= 0x6
};

enum HYPERVISOR {
	HYPERV_BARE,
	HYPERV_XEN,
	HYPERV_KVM,
	HYPERV_VBOX,
	HYPERVISORS
};

#define CODENAME_LEN	32

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

#define COMPUTE_THERMAL_INTEL(Temp, Param, Sensor)			\
	(Temp = Param.Target - Sensor)

#define COMPUTE_THERMAL_AMD(Temp, Param, Sensor)			\
	/*( TODO )*/

#define COMPUTE_THERMAL_AMD_0Fh(Temp, Param, Sensor)			\
	(Temp = Sensor - (Param.Target * 2) - 49)

#define COMPUTE_THERMAL_AMD_15h(Temp, Param, Sensor)			\
	(Temp = Sensor * 5 / 40)

#define COMPUTE_THERMAL_AMD_17h(Temp, Param, Sensor)			\
	(Temp = ((Sensor * 5 / 40) - Param.Offset[1]) - Param.Offset[0])

#define COMPUTE_THERMAL(_ARCH_, Temp, Param, Sensor)			\
	COMPUTE_THERMAL_##_ARCH_(Temp, Param, Sensor)

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

#define COMPUTE_VOLTAGE_INTEL_CORE2(Vcore, VID) 			\
		(Vcore = 0.8875 + (double) (VID) * 0.0125)

#define COMPUTE_VOLTAGE_INTEL_SNB(Vcore, VID) 				\
		(Vcore = (double) (VID) / 8192.0)

#define COMPUTE_VOLTAGE_INTEL_SKL_X(Vcore, VID) 			\
		(Vcore = (double) (VID) / 8192.0)

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
		(Vcore = 1.550 -(0.00625 * (double) (VID)))

#define COMPUTE_VOLTAGE_AMD_17h(Vcore, VID)				\
		(Vcore = 1.550 -(0.00625 * (double) (VID)))

#define COMPUTE_VOLTAGE_WINBOND_IO(Vcore, VID)				\
		(Vcore = (double) (VID) * 0.008)

#define COMPUTE_VOLTAGE(_ARCH_, Vcore, VID)	\
		COMPUTE_VOLTAGE_##_ARCH_(Vcore, VID)

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

#define DRV_DEVNAME	"corefreqk"
#define DRV_FILENAME	"/dev/"DRV_DEVNAME

#define WAKEUP_RATIO	4
#define LOOP_MIN_MS	100
#define LOOP_MAX_MS	((1000 - 1) * WAKEUP_RATIO)
#define LOOP_DEF_MS	1000
#define TICK_DEF_MS	2000

#define SIG_RING_MS	(500 * 1000000LU)
#define CHILD_PS_MS	(500 * 1000000LU)
#define CHILD_TH_MS	(500 * 1000000LU)

#define PRECISION	100

#define UNIT_KHz(_f)		(_f * 10 * PRECISION)
#define UNIT_MHz(_f)		(_f * UNIT_KHz(1000))
#define UNIT_GHz(_f)		(_f * UNIT_MHz(1000))
#define CLOCK_KHz(_t, _f)	(_f / UNIT_KHz((_t) 1))
#define CLOCK_MHz(_t, _f)	(_f / UNIT_MHz((_t) 1))
#define CLOCK_GHz(_t, _f)	(_f / UNIT_GHz((_t) 1))

#define TIMESPEC(nsec)							\
({									\
	struct timespec tsec = {					\
		.tv_sec  = (time_t) 0,					\
		.tv_nsec = nsec						\
	};								\
	tsec;								\
})

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

#if defined(MAX_FREQ_HZ) && (MAX_FREQ_HZ >= 4850000000)
#define MAXCLOCK_TO_RATIO(_typeout, BaseClock) ( (_typeout) (		\
		MAX_FREQ_HZ / BaseClock					\
) )
#else
#define MAXCLOCK_TO_RATIO(_typeout, BaseClock) ( (_typeout) (		\
		5250000000 / BaseClock					\
) )
#endif

enum PWR_DOMAIN {
	DOMAIN_PKG,
	DOMAIN_CORES,
	DOMAIN_UNCORE,
	DOMAIN_RAM,
	DOMAIN_SIZE
};

#define PWR_DOMAIN(NC) DOMAIN_##NC

#define CACHE_MAX_LEVEL (3 + 1)

#define VENDOR_INTEL	"GenuineIntel"
#define VENDOR_AMD	"AuthenticAMD"
#define VENDOR_HYGON	"HygonGenuine"
#define CRC_INTEL	0x75a2ba39
#define CRC_AMD 	0x3485bbd3
#define CRC_HYGON	0x18044630

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
({	/* Compute Divisor in Interval				*/	\
	unsigned long long divisor = 1000LLU * ratio * interval;	\
	/* Compute Quotient					*/	\
	clock.Q  = delta_tsc / divisor;					\
	/* Compute Remainder					*/	\
	clock.R  = delta_tsc % divisor;					\
	/* Compute full Hertz					*/	\
	clock.Hz = (clock.Q * 1000000LLU)				\
		 + (clock.R * 1000000LLU) / divisor;			\
})

#define REL_FREQ(max_ratio, this_ratio, clock, interval)		\
		( ((this_ratio * clock.Q) * 1000LLU * interval) 	\
		+ ((this_ratio * clock.R) / max_ratio))

#define ABS_FREQ_MHz(this_type, this_ratio, this_clock) 		\
(									\
		CLOCK_MHz(this_type, this_ratio * this_clock.Hz)	\
)

typedef union {
	signed long long	sllong;
	struct {
	    struct {
		signed short	Offset;
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

#define CPUID_MAX_FUNC	62

typedef struct
{
	unsigned int	func,
			sub,
			reg[4];
} CPUID_STRUCT;

typedef struct
{		/* Common x86						*/
	unsigned int		LargestStdFunc, /* Largest CPUID Standard Func*/
				LargestExtFunc; /* Largest CPUID Extended Func*/
	struct {
		unsigned int	CRC;
		char		ID[12 + 4];
	} Vendor;
	char			Brand[48 + 4];
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
		Unused1 : 12-11,
		FMA	: 13-12,
		CMPXCHG16:14-13,
		xTPR	: 15-14,
		PDCM	: 16-15,
		Unused2 : 17-16,
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
		PSN	: 19-18, /* Intel				*/
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
		Unused3 : 20-19,
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
		Unused1 : 32-0;
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
		SGX		:  3-2, /* Software Guard Extensions	*/
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
		SMAP		: 21-20, /*Supervisor-Mode Access & CLAC/STAC*/
		AVX512_IFMA	: 22-21,
		Reserved1	: 23-22,
		CLFLUSHOPT	: 24-23, /* Flush Cache Line Optimized	*/
		CLWB		: 25-24, /* Cache Line Write Back	*/
		ProcessorTrace	: 26-25, /* CPUID.(EAX=14H, ECX=0)	*/
		AVX512PF	: 27-26, /* Intel Xeon Phi		*/
		AVX512ER	: 28-27, /* Intel Xeon Phi		*/
		AVX512CD	: 29-28,
		SHA		: 30-29, /* Intel Secure Hash Algorithm */
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
		Reserved1	:  8-6,
		GFNI		:  9-8, /* Galois Field SSE instructions*/
		Reserved2	: 14-9,
		AVX512_VPOPCNTDQ: 15-14, /* Intel Xeon Phi		*/
		Reserved3	: 17-15,
		MAWAU		: 22-17, /* for BNDLDX & BNDSTX instructions*/
		RDPID		: 23-22, /* RDPID & IA32_TSC_AUX availability*/
		Reserved4	: 25-23,
		CLDEMOTE	: 26-25, /* Support of cache line demote */
		Reserved5	: 27-26,
		MOVDIRI 	: 28-27, /* Move Doubleword as Direct Store*/
		MOVDIR64B	: 29-28, /* Move 64 Bytes as Direct Store*/
		Reserved6	: 30-29,
		SGX_LC		: 31-30, /* SGX Launch Configuration support*/
		Reserved7	: 32-31;
	} ECX;
	struct
	{	/* Intel reserved.					*/
		unsigned int
		Reserved1	:  1-0,
		AVX512_4VNNIW	:  2-1, /* Intel Xeon Phi		*/
		AVX512_4FMAPS	:  3-2, /* Intel Xeon Phi		*/
		Reserved2	: 10-3,
		MD_CLEAR_Cap	: 11-10,
		Reserved3	: 26-11,
		IBRS_IBPB_Cap	: 27-26, /* IA32_SPEC_CTRL,IA32_PRED_CMD */
		STIBP_Cap	: 28-27, /* IA32_SPEC_CTRL[1]		*/
		L1D_FLUSH_Cap	: 29-28, /* IA32_FLUSH_CMD		*/
		IA32_ARCH_CAP	: 30-29, /* IA32_ARCH_CAPABILITIES	*/
		IA32_CORE_CAP	: 31-30, /* IA32_CORE_CAPABILITIES	*/
		SSBD_Cap	: 32-31; /* IA32_SPEC_CTRL[2]		*/
	} EDX;
} CPUID_0x00000007;

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
		/* Family 15h :						*/
		OSVW	: 10-9,  /* OS-visible workaround support.	*/
		IBS	: 11-10, /* Instruction based sampling.		*/
		XOP	: 12-11, /* Extended operation support.		*/
		SKINIT	: 13-12, /* SKINIT and STGI support.		*/
		WDT	: 14-13, /* Watchdog timer support.		*/
		NotUsed1: 15-14,
		LWP	: 16-15, /* Lightweight profiling support.	*/
		FMA4	: 17-16, /* Four-operand FMA instruction.	*/
		TCE	: 18-17, /* Translation Cache Extension.	*/
		NotUsed2: 21-18,
		TBM	: 22-21, /* Trailing bit manipulation.		*/
		TopoExt : 23-22, /* Topology extensions support.	*/
		PerfCore: 24-23, /* PerfCtrExtCore MSR.			*/
		PerfNB	: 25-24, /* PerfCtrExtNB MSR.			*/
		NotUsed3: 26-25,
		Data_BP : 27-26, /* Data access breakpoint extension.	*/
		PerfTSC : 28-27, /* Performance TSC MSR.		*/
		PerfL2I : 29-28, /* L2I performance counter extensions support*/
		MWaitExt: 30-29, /* MWAITX/MONITORX support.		*/
		NotUsed4: 32-30;
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
		Unused1 : 32-0;
	} EAX, EBX, ECX;
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
		HwPstate:  8-7,  /* Hardware P-state control msr exist ? */
		TscInv	:  9-8,  /* Invariant TSC ?			*/
		CPB	: 10-9,  /* Core performance boost.		*/
		EffFrqRO: 11-10, /* Read-only effective freq. interf. msr ?   */
		ProcFb	: 12-11, /* Processor feedback interf. available if 1 */
		ProcPwr : 13-12, /* Core power reporting interface supported. */
		Reserved: 32-13;
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
	{
		unsigned int
		CLZERO		:  1-0,  /* AMD Clear Zero Instruction	*/
		IRPerf		:  2-1,  /* AMD Inst. Retired Counter support */
		XSaveErPtr	:  3-2,  /* AMD FX___ error pointers suuport  */
		Reserved	: 32-3;
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
	{
		unsigned int
		Reserved	: 32-0;
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
			ClkRatio_Unlock : 12-11,
			Turbo_Unlock	: 13-12,
			TDP_Unlock	: 14-13,
			TDP_Levels	: 16-14,
			TDP_Cfg_Lock	: 17-16,
			TDP_Cfg_Level	: 19-17,
			TurboActiv_Lock : 20-19,
			Uncore_Unlock	: 21-20,
			HWP_Enable	: 22-21,
			HDC_Enable	: 23-22,
			_UnusedFeatBits : 24-23,
			SpecTurboRatio	: 32-24;
	};
} FEATURES;

#define MC_MAX_CTRL	2
#define MC_MAX_CHA	4
#define MC_MAX_DIMM	4

#ifndef PCI_DEVICE_ID_INTEL_82945P_HB
	#define PCI_DEVICE_ID_INTEL_82945P_HB		0x2770
#endif
#ifndef PCI_DEVICE_ID_INTEL_82945GM_HB
	#define PCI_DEVICE_ID_INTEL_82945GM_HB		0x27a0
#endif
#ifndef PCI_DEVICE_ID_INTEL_82955_HB
	#define PCI_DEVICE_ID_INTEL_82955_HB		0x2774
#endif
/* Source: /drivers/char/agp/intel-agp.h				*/
#ifndef PCI_DEVICE_ID_INTEL_82945GME_HB
	#define PCI_DEVICE_ID_INTEL_82945GME_HB		0x27ac
#endif
#ifndef PCI_DEVICE_ID_INTEL_82946GZ_HB
	#define PCI_DEVICE_ID_INTEL_82946GZ_HB		0x2970
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965Q_HB
	#define PCI_DEVICE_ID_INTEL_82965Q_HB		0x2990
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965G_HB
	#define PCI_DEVICE_ID_INTEL_82965G_HB		0x29a0
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965GM_HB
	#define PCI_DEVICE_ID_INTEL_82965GM_HB		0x2a00
#endif
#ifndef PCI_DEVICE_ID_INTEL_82965GME_HB
	#define PCI_DEVICE_ID_INTEL_82965GME_HB 	0x2a10
#endif
#ifndef PCI_DEVICE_ID_INTEL_GM45_HB
	#define PCI_DEVICE_ID_INTEL_GM45_HB		0x2a40
#endif
#ifndef PCI_DEVICE_ID_INTEL_Q35_HB
	#define PCI_DEVICE_ID_INTEL_Q35_HB		0x29b0
#endif
#ifndef PCI_DEVICE_ID_INTEL_G33_HB
	#define PCI_DEVICE_ID_INTEL_G33_HB		0x29c0
#endif
#ifndef PCI_DEVICE_ID_INTEL_Q33_HB
	#define PCI_DEVICE_ID_INTEL_Q33_HB		0x29d0
#endif
/* Source: /drivers/edac/x38_edac.c					*/
#ifndef PCI_DEVICE_ID_INTEL_X38_HB
	#define PCI_DEVICE_ID_INTEL_X38_HB		0x29e0
#endif
/* Source: /drivers/edac/i3200_edac.c					*/
#ifndef PCI_DEVICE_ID_INTEL_3200_HB
	#define PCI_DEVICE_ID_INTEL_3200_HB		0x29f0
#endif
/* Source: /drivers/char/agp/intel-agp.h				*/
#ifndef PCI_DEVICE_ID_INTEL_Q45_HB
	#define PCI_DEVICE_ID_INTEL_Q45_HB		0x2e10
#endif
#ifndef PCI_DEVICE_ID_INTEL_G45_HB
	#define PCI_DEVICE_ID_INTEL_G45_HB		0x2e20
#endif
#ifndef PCI_DEVICE_ID_INTEL_G41_HB
	#define PCI_DEVICE_ID_INTEL_G41_HB		0x2e30
#endif
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_INTEL_I7_MCR
	#define PCI_DEVICE_ID_INTEL_I7_MCR		0x2c18
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH0_CTRL	0x2c20
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH1_CTRL	0x2c28
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH2_CTRL	0x2c30
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_TEST
	#define PCI_DEVICE_ID_INTEL_I7_MC_TEST		0x2c1c
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH0_ADDR
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH0_ADDR	0x2c21
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH1_ADDR
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH1_ADDR	0x2c29
#endif
#ifndef PCI_DEVICE_ID_INTEL_I7_MC_CH2_ADDR
	#define PCI_DEVICE_ID_INTEL_I7_MC_CH2_ADDR	0x2c31
#endif
#ifndef PCI_DEVICE_ID_INTEL_BLOOMFIELD_NON_CORE
	#define PCI_DEVICE_ID_INTEL_BLOOMFIELD_NON_CORE 0x2c41
#endif
#ifndef PCI_DEVICE_ID_INTEL_C5500_NON_CORE
	#define PCI_DEVICE_ID_INTEL_C5500_NON_CORE	0x2c58
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE	0x2c51
#endif
#ifndef PCI_DEVICE_ID_INTEL_CLARKSFIELD_NON_CORE
    #define PCI_DEVICE_ID_INTEL_CLARKSFIELD_NON_CORE	0x2c52
#endif
#ifndef PCI_DEVICE_ID_INTEL_CLARKDALE_NON_CORE
	#define PCI_DEVICE_ID_INTEL_CLARKDALE_NON_CORE	0x2c61
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR	0x2c98
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_CTRL
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_CTRL	0x2ca0
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_CTRL
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_CTRL	0x2ca8
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST	0x2c9c
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_ADDR
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH0_ADDR	0x2ca1
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_ADDR
    #define PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_CH1_ADDR	0x2ca9
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MCR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MCR		0x2d98
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_CTRL
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_CTRL	0x2da0
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_CTRL
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_CTRL	0x2da8
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_CTRL
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_CTRL	0x2db0
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_TEST
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_TEST	0x2d9c
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_ADDR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH0_ADDR	0x2da1
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_ADDR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH1_ADDR	0x2da9
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_ADDR
	#define PCI_DEVICE_ID_INTEL_NHM_EP_MC_CH2_ADDR	0x2db1
#endif
#ifndef PCI_DEVICE_ID_INTEL_NHM_EP_NON_CORE
	#define PCI_DEVICE_ID_INTEL_NHM_EP_NON_CORE	0x2c70
#endif
/* Source: Intel X58 Express Chipset Datasheet				*/
#define PCI_DEVICE_ID_INTEL_X58_HUB_CORE		0x342e
#define PCI_DEVICE_ID_INTEL_X58_HUB_CTRL		0x3423
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0	0x3ca0
#endif
/* Source: 2nd Generation Intel® Core™ Processor Family Vol2		*/
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA	0x0100
#endif
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104	0x0104
#endif
/* Source: /drivers/edac/sb_edac.c					*/
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0	0x0ea0
#endif
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA1
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA1	0x0e60
#endif
/* Source: 3rd Generation Intel® Core™ Processor Family Vol2		*/
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA	0x0150
#endif
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154	0x0154
#endif
/* Source: Intel Xeon Processor E5 & E7 v1 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_HOST_BRIDGE
	#define PCI_DEVICE_ID_INTEL_SNB_EP_HOST_BRIDGE	0x3c00
#endif
/*	QPIMISCSTAT: Device=8 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_QPI_LINK0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_QPI_LINK0	0x3c80
#endif
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=15 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CPGC
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CPGC 0x3ca8
#endif
/*TODO( Nehalem/Xeon E7 - CPGC: Device=?? - Function=? )
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CPGC
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CPGC 0x0
#endif									*/
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=16 - Function=0,1,2,3			*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH0 0x3cb0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH1 0x3cb1
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH2 0x3cb2
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL0_CH3 0x3cb3
#endif
/*	Controller #1: Device=16 - Function=4,5,6,7			*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH0 0x3cb4
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH1 0x3cb5
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH2 0x3cb6
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_IMC_CTRL1_CH3 0x3cb7
#endif
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=15 - Function=2,3,4,5,6	*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH0 0x3caa
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH1 0x3cab
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH2 0x3cac
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH3 0x3cad
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH4
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL0_CH4 0x3cae
#endif
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*TODO( Nehalem/Xeon E7 - TAD Controller #1: Device=?? - Function=? )
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH0 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH1 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH2 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH3 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH4
	#define PCI_DEVICE_ID_INTEL_SNB_EP_TAD_CTRL1_CH4 0x0
#endif									*/
/*	Power Control Unit						*/
/*TODO( PCU: Device=10 - Function=3 )					*/
#ifndef PCI_DEVICE_ID_INTEL_SNB_EP_CAPABILITY
	#define PCI_DEVICE_ID_INTEL_SNB_EP_CAPABILITY	0x3cd0
#endif
/* Source: Intel Xeon Processor E5 & E7 v2 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_HOST_BRIDGE
	#define PCI_DEVICE_ID_INTEL_IVB_EP_HOST_BRIDGE	0x0e00
#endif
/*	QPIMISCSTAT: Device=8 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_QPI_LINK0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_QPI_LINK0	0x0e80
#endif
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=15 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CPGC
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CPGC 0x0ea8
#endif
/*	Xeon E7 - CPGC: Device=29 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CPGC
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CPGC 0x0e68
#endif
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*	Controller #0: Device=16 - Function=0,1,2,3			*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH0 0x0eb0
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH1 0x0eb1
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH2 0x0eb2
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH3 0x0eb3
#endif
/*	Controller #1: Device=16 - Function=4,5,6,7			*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH0 0x0eb4
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH1 0x0eb5
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH2 0x0eb6
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH3 0x0eb7
#endif
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=15 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH0 0x0eaa
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH1 0x0eab
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH2 0x0eac
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH3 0x0ead
#endif
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*	Xeon E7 - TAD Controller #1: Device=29 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH0 0x0e6a
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH1 0x0e6b
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH2 0x0e6c
#endif
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH3 0x0e6d
#endif
/*	Power Control Unit						*/
/*	PCU: Device=10 - Function=3					*/
#ifndef PCI_DEVICE_ID_INTEL_IVB_EP_CAPABILITY
	#define PCI_DEVICE_ID_INTEL_IVB_EP_CAPABILITY	0x0ec3
#endif
/* Source: Intel Xeon Processor E5 & E7 v3 Datasheet Vol 2		*/
/*	DMI2: Device=0 - Function=0					*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_HOST_BRIDGE
	#define PCI_DEVICE_ID_INTEL_HSW_EP_HOST_BRIDGE	0x2f00
#endif
/*	QPIMISCSTAT: Device=8 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_QPI_LINK0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_QPI_LINK0	0x2f80
#endif
/*	Integrated Memory Controller # : General and MemHot Registers	*/
/*	Xeon E5 - CPGC: Device=19 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CPGC
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CPGC 0x2fa8
#endif
/*	Xeon E7 - CPGC: Device=22 - Function=0				*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CPGC
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CPGC 0x2f68
#endif
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
/*TODO( Controller #0: Device=?? - Function=0,1,2,3 )
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH0 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH1 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH2 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL0_CH3 0x0
#endif									*/
/*TODO( Controller #1: Device=?? - Function=4,5,6,7 )
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH0 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH1 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH2 0x0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_IMC_CTRL1_CH3 0x0
#endif									*/
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
/*	Xeon E5 - TAD Controller #0: Device=19 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH0 0x2faa
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH1 0x2fab
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH2 0x2fac
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL0_CH3 0x2fad
#endif
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
/*	Xeon E7 - TAD Controller #1: Device=22 - Function=2,3,4,5	*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH0
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH0 0x2f6a
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH1
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH1 0x2f6b
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH2
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH2 0x2f6c
#endif
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH3
	#define PCI_DEVICE_ID_INTEL_HSW_EP_TAD_CTRL1_CH3 0x2f6d
#endif
/*	Power Control Unit						*/
/*	PCU: Device=30 - Function=3					*/
#ifndef PCI_DEVICE_ID_INTEL_HSW_EP_CAPABILITY
	#define PCI_DEVICE_ID_INTEL_HSW_EP_CAPABILITY	0x2fc0
#endif
/* Source: 4th, 5th Generation Intel® Core™ Processor Family Vol2 §3.0	*/
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0	0x2fa0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA
	#define PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA	0x0c00
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_MH_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_MH_IMC_HA0	0x0c04
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_UY_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_UY_IMC_HA0	0x0a04
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0	0x1604
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_D_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_D_IMC_HA0 0x1610
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0 0x1614
#endif
/* Source: 6th Generation Intel® Processor Datasheet for U/Y-Platforms Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA	0x1904
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA	0x190c
#endif
/* Source: 6th Generation Intel® Processor Datasheet for S-Platforms Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD	0x190f
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ	0x191f
#endif
/* Source: 6th Generation Intel® Processor Datasheet for H-Platforms Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD	0x1900
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ	0x1910
#endif
/* Source: Intel Xeon Processor E3-1200 v5 Product Family		*/
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_DT_IMC_HA
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_DT_IMC_HA	0x1918
#endif
/* Source:7th Generation Intel® Processor for S-Platforms & Core X-Series Vol2*/
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD	0x5900
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA	0x5904
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA	0x590c
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD	0x590f
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ	0x5910
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_DT_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_DT_IMC_HA	0x5918
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ	0x5914
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ	0x591f
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ	0x5906
#endif
/* Source: 8th Generation Intel® Processor for S-Platforms Datasheet Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ 0x3e1f
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAS 0x3ec2
#endif
/* Source: 8th and 9th Generation Intel® Core™ and Xeon™ E Processor Families */
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAD 0x3e0f
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAD 0x3ecc
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAQ 0x3ed0
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAQ 0x3e10
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAS 0x3ec4
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAO
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAO 0x3e30
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAQ 0x3e18
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAS 0x3ec6
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAO
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAO 0x3e31
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAQ 0x3e33
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAS
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAS 0x3eca
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAO
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAO 0x3e32
#endif
/* Source: 8th Generation Intel® Core™ Processor Families Datasheet Vol2 */
#ifndef PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAD 0x3e35
#endif
#ifndef PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAQ 0x3e34
#endif
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_AMD_K8_NB_MEMCTL
	#define PCI_DEVICE_ID_AMD_K8_NB_MEMCTL		0x1102
#endif
#ifndef PCI_DEVICE_ID_AMD_K8_NB
	#define PCI_DEVICE_ID_AMD_K8_NB			0x1100
#endif
/* Source: AMD I/O Virtualization Technology (IOMMU) Specification	*/
#ifndef PCI_DEVICE_ID_AMD_17H_IOMMU
	#define PCI_DEVICE_ID_AMD_17H_IOMMU		0x1451
#endif
/* Source: /include/linux/pci_ids.h					*/
#ifndef PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3	0x1463	/* Zeppelin */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_RAVEN_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_RAVEN_DF_F3	0x15eb	/* Raven */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_MATISSE_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_MATISSE_DF_F3	0x1443	/* Zen2 */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_STARSHIP_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_STARSHIP_DF_F3	0x1493	/* Zen2 */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_RENOIR_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_RENOIR_DF_F3	0x144b	/* Renoir */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_ARIEL_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_ARIEL_DF_F3	0x13f3	/* Ariel */
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_DF_F3	0x15f3	/* FireFlight*/
#endif
#ifndef PCI_DEVICE_ID_AMD_17H_ARDEN_DF_F3
	#define PCI_DEVICE_ID_AMD_17H_ARDEN_DF_F3	0x160b	/* Arden */
#endif

/* Hardware Monitoring: Super I/O chipsets				*/
#define COMPATIBLE		0xffff
#define W83627			0x5ca3

typedef struct
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

typedef struct {			/* 0: Disable, 1: Enable	*/
	unsigned short	CPUidle :  1-0,
			CPUfreq :  2-1,
			Governor:  3-2,
			unused	: 16-3;
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

/* Circular buffer							*/
#define RING_SIZE	16

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
	unsigned int		cmd: 32,
				sub: 32;
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

