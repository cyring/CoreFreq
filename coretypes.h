/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define COREFREQ_VERSION	"1.22.0"

enum {	GenuineIntel,		\
	Core_Yonah,		\
	Core_Conroe,		\
	Core_Kentsfield,	\
	Core_Conroe_616,	\
	Core_Yorkfield,		\
	Core_Dunnington,	\
	Atom_Bonnell,		\
	Atom_Silvermont,	\
	Atom_Lincroft,		\
	Atom_Clovertrail,	\
	Atom_Saltwell,		\
	Silvermont_637,		\
	Atom_Avoton,		\
	Atom_Airmont,		\
	Atom_Goldmont,		\
	Atom_Sofia,		\
	Atom_Merrifield,	\
	Atom_Moorefield,	\
	Nehalem_Bloomfield,	\
	Nehalem_Lynnfield,	\
	Nehalem_MB,		\
	Nehalem_EX,		\
	Westmere,		\
	Westmere_EP,		\
	Westmere_EX,		\
	SandyBridge,		\
	SandyBridge_EP,		\
	IvyBridge,		\
	IvyBridge_EP,		\
	Haswell_DT,		\
	Haswell_EP,		\
	Haswell_ULT,		\
	Haswell_ULX,		\
	Broadwell,		\
	Broadwell_EP,		\
	Broadwell_H,		\
	Broadwell_EX,		\
	Skylake_UY,		\
	Skylake_S,		\
	Skylake_X,		\
	Xeon_Phi,		\
	Kabylake,		\
	Kabylake_UY,		\
	Cannonlake,		\
	Geminilake,		\
	Icelake_UY,		\
	AMD_Family_0Fh,		\
	AMD_Family_10h,		\
	AMD_Family_11h,		\
	AMD_Family_12h,		\
	AMD_Family_14h,		\
	AMD_Family_15h,		\
	AMD_Family_16h,		\
	AMD_Family_17h,		\
	ARCHITECTURES
};

enum SYS_REG {
	RFLAG_TF	= 8,
	RFLAG_IF	= 9,
	RFLAG_IOPL	= 12,	// [13:12]
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

	CR3_PWT		= 3,
	CR3_PCD		= 4,

	CR4_VME		= 0,
	CR4_PVI		= 1,
	CR4_TSD		= 2,
	CR4_DE		= 3,
	CR4_PSE		= 4,
	CR4_PAE		= 5,
	CR4_MCE		= 6,
	CR4_PGE		= 7,
	CR4_PCE		= 8,
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
	EXFCR_SENTER_LEN= 8,	// [14:8]
	EXFCR_SENTER_GEN= 15,
	EXFCR_SGX_LCE	= 17,
	EXFCR_SGX_GEN	= 18,
	EXFCR_LMCE	= 20,

	EXFER_SCE	= 0,
	EXFER_LME	= 8,
	EXFER_LMA	= 10,
	EXFER_NXE	= 11
};

#define THERMAL_FORMULA_NONE \
	0b0000000000000000000000000000000000000000000000000000000000000000
#define THERMAL_FORMULA_INTEL \
	0b0000000000000000000000000000000000000000000000000000000000000001
#define THERMAL_FORMULA_AMD \
	0b0000000000000000000000000000000100000000000000000000000000000000
#define THERMAL_FORMULA_AMD_0F \
	0b0000000000000000000000000000001100000000000000000000000000000000

#define VOLTAGE_FORMULA_NONE \
	0b0000000000000000000000000000000000000000000000000000000000000000
#define VOLTAGE_FORMULA_INTEL \
	0b0000000000000000000000000000000000000000000000000000000000000001
#define VOLTAGE_FORMULA_INTEL_MEROM \
	0b0000000000000000000000000000000000000000000000000000000000000011
#define VOLTAGE_FORMULA_INTEL_SNB \
	0b0000000000000000000000000000000000000000000000000000000010000001
#define VOLTAGE_FORMULA_INTEL_SKL_X \
	0b0000000000000000000000000000000000000000000000000000010000000001
#define VOLTAGE_FORMULA_AMD \
	0b0000000000000000000000000000000100000000000000000000000000000000
#define VOLTAGE_FORMULA_AMD_0F \
	0b0000000000000000000000000000001100000000000000000000000000000000

#define POWER_FORMULA_NONE \
	0b0000000000000000000000000000000000000000000000000000000000000000
#define POWER_FORMULA_INTEL \
	0b0000000000000000000000000000000000000000000000000000000000000001
#define POWER_FORMULA_INTEL_ATOM \
	0b0000000000000000000000000000000000000000000000000000000000000011
#define POWER_FORMULA_AMD \
	0b0000000000000000000000000000000100000000000000000000000000000000

#define ROUND_TO_PAGES(Size)	PAGE_SIZE * ((Size / PAGE_SIZE) 	\
				+ ((Size % PAGE_SIZE)? 1:0))

#define KMAX(M, m)	((M) > (m) ? (M) : (m))
#define KMIN(m, M)	((m) < (M) ? (m) : (M))

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
	RATIO_ACT,
	RATIO_TDP,
	RATIO_TDP1,
	RATIO_TDP2,
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
	DOMAIN_CORE,
	DOMAIN_UNCORE,
	DOMAIN_RAM,
	DOMAIN_SIZE
};

#define PWR_DOMAIN(NC) DOMAIN_##NC

#define CACHE_MAX_LEVEL (3 + 1)

#define VENDOR_INTEL	"GenuineIntel"
#define VENDOR_AMD	"AuthenticAMD"
#define CRC_INTEL	0x75a2ba39
#define CRC_AMD 	0x3485bbd3

enum OFFLINE
{
	HW,
	OS
};

typedef struct
{
	unsigned int		Q;
	unsigned long long	R;
	unsigned long long	Hz;
} CLOCK;

#define REL_FREQ(max_ratio, this_ratio, clock, interval)		\
		( ((this_ratio * clock.Q) * 1000L * interval)		\
		+ ((this_ratio * clock.R) / max_ratio))


/* Sources:
 * Intel® 64 and IA-32 Architectures Software Developer’s Manual; Vol. 2A
 * AMD64 Architecture Programmer’s Manual; Vol. 3
*/

#define CPUID_MAX_FUNC	60

typedef struct
{
	unsigned int	func,
			sub,
			reg[4];
} CPUID_STRUCT;

static const CPUID_STRUCT CpuIDforVendor[CPUID_MAX_FUNC]={
/* x86 */
	{.func=0x00000001, .sub=0x00000000},	/* Instruction set	*/
/* Intel */
	{.func=0x00000002, .sub=0x00000000},	/* Cache & TLB		*/
	{.func=0x00000003, .sub=0x00000000},	/* Proc. Serial Number	*/
	{.func=0x00000004, .sub=0x00000000},	/* Cache L1I		*/
	{.func=0x00000004, .sub=0x00000001},	/* Cache L1D		*/
	{.func=0x00000004, .sub=0x00000002},	/* Cache L2		*/
	{.func=0x00000004, .sub=0x00000003},	/* Cache L3		*/
/* x86 */
	{.func=0x00000005, .sub=0x00000000},	/* MONITOR/MWAIT	*/
	{.func=0x00000006, .sub=0x00000000},	/* Power & Thermal Mgmt	*/
	{.func=0x00000007, .sub=0x00000000},	/* Extended Features	*/
/* Intel */
	{.func=0x00000009, .sub=0x00000000},	/* Direct Cache Access	*/
	{.func=0x0000000a, .sub=0x00000000},	/* Perf. Monitoring	*/
/* x86 */
	{.func=0x0000000b, .sub=0x00000000},	/* Ext. Topology	*/
	{.func=0x0000000d, .sub=0x00000000},	/* Ext. State Main leaf	*/
	{.func=0x0000000d, .sub=0x00000001},	/* Ext. State Sub-leaf	*/
/* AMD */
	{.func=0x0000000d, .sub=0x00000002},	/* Ext. State Sub-leaf	*/
/* AMD Family 15h */
	{.func=0x0000000d, .sub=0x0000003e},	/* Ext. State Sub-leaf	*/
/* Intel */
	{.func=0x0000000f, .sub=0x00000000},	/* QoS Monitoring cap.	*/
	{.func=0x0000000f, .sub=0x00000001},	/* L3 QoS Monitoring	*/
	{.func=0x00000010, .sub=0x00000000},	/* QoS Enforcement cap.	*/
	{.func=0x00000010, .sub=0x00000001},	/* L3 Alloc Enumeration	*/
	{.func=0x00000010, .sub=0x00000002},	/* L2 Alloc Enumeration	*/
	{.func=0x00000010, .sub=0x00000003},	/* RAM Bandwidth Enum.	*/
	{.func=0x00000012, .sub=0x00000000},	/* SGX Capability	*/
	{.func=0x00000012, .sub=0x00000001},	/* SGX Attributes	*/
	{.func=0x00000012, .sub=0x00000002},	/* SGX EnclavePageCache	*/
	{.func=0x00000014, .sub=0x00000000},	/* Processor Trace	*/
	{.func=0x00000014, .sub=0x00000001},	/* Proc. Trace Sub-leaf	*/
	{.func=0x00000015, .sub=0x00000000},	/* Time Stamp Counter	*/
	{.func=0x00000016, .sub=0x00000000},	/* Processor Frequency	*/
	{.func=0x00000017, .sub=0x00000000},	/* System-On-Chip	*/
	{.func=0x00000017, .sub=0x00000001},	/* SOC Attrib. Sub-leaf	*/
	{.func=0x00000017, .sub=0x00000002},	/* SOC Attrib. Sub-leaf	*/
	{.func=0x00000017, .sub=0x00000003},	/* SOC Attrib. Sub-leaf	*/
/* x86 */
	{.func=0x80000001, .sub=0x00000000},	/* Extended Features	*/
	{.func=0x80000002, .sub=0x00000000},	/* Processor Name Id.	*/
	{.func=0x80000003, .sub=0x00000000},	/* Processor Name Id.	*/
	{.func=0x80000004, .sub=0x00000000},	/* Processor Name Id.	*/
/* AMD */
	{.func=0x80000005, .sub=0x00000000},	/* Caches L1D L1I TLB	*/
/* x86 */
	{.func=0x80000006, .sub=0x00000000},	/* Cache L2 Size & Way	*/
	{.func=0x80000007, .sub=0x00000000},	/* Advanced Power Mgmt	*/
	{.func=0x80000008, .sub=0x00000000},	/* LM Address Size	*/
/* AMD */
	{.func=0x8000000a, .sub=0x00000000},	/* SVM Revision		*/
	{.func=0x80000019, .sub=0x00000000},	/* Caches & TLB 1G	*/
	{.func=0x8000001a, .sub=0x00000000},	/* Perf. Optimization	*/
	{.func=0x8000001b, .sub=0x00000000},	/* Inst. Based Sampling	*/
	{.func=0x8000001c, .sub=0x00000000},	/* Lightweight Profiling*/
	{.func=0x8000001d, .sub=0x00000000},	/* Cache L1D Properties	*/
	{.func=0x8000001d, .sub=0x00000001},	/* Cache L1I Properties	*/
	{.func=0x8000001d, .sub=0x00000002},	/* Cache L2 Properties	*/
	{.func=0x8000001d, .sub=0x00000003},	/* Cache Properties End	*/
	{.func=0x8000001e, .sub=0x00000000},	/* Extended Identifiers	*/
/* x86 */
	{.func=0x40000000, .sub=0x00000000},	/* Hypervisor vendor	*/
	{.func=0x40000001, .sub=0x00000000},	/* Hypervisor interface	*/
	{.func=0x40000002, .sub=0x00000000},	/* Hypervisor version	*/
	{.func=0x40000003, .sub=0x00000000},	/* Hypervisor features	*/
	{.func=0x40000004, .sub=0x00000000},	/* Hyperv. requirements	*/
	{.func=0x40000005, .sub=0x00000000},	/* Hypervisor limits	*/
	{.func=0x40000006, .sub=0x00000000},	/* Hypervisor exploits	*/
	{.func=0x00000000, .sub=0x00000000},
};

typedef struct
{		// Common x86
	unsigned int		LargestStdFunc,//Largest CPUID Standard Function
				LargestExtFunc;//Largest CPUID Extended Function
	struct {
		unsigned int	CRC;
		char		ID[12 + 4];
	} Vendor;
	char			Brand[48 + 4];
} CPUID_FUNCTION;

typedef struct	// Basic CPUID Function.
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
		Unused1		: 16-14,
		ExtModel	: 20-16,
		ExtFamily	: 28-20,
		Unused2		: 32-28;
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
		SSE3	:  1-0,  // AMD Family 0Fh
		PCLMULDQ:  2-1,
		DTES64	:  3-2,
		MONITOR	:  4-3,
		DS_CPL	:  5-4,
		VMX	:  6-5,
		SMX	:  7-6,
		EIST	:  8-7,
		TM2	:  9-8,
		SSSE3	: 10-9,  // AMD Family 0Fh
		CNXT_ID	: 11-10,
		Unused1	: 12-11,
		FMA	: 13-12,
		CMPXCH16: 14-13,
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
		Hyperv	: 32-31;
	} ECX;
	struct
	{	// Most common x86
		unsigned int
		FPU	:  1-0,
		VME	:  2-1,
		DE	:  3-2,
		PSE	:  4-3,
		TSC	:  5-4,
		MSR	:  6-5,
		PAE	:  7-6,
		MCE	:  8-7,
		CMPXCH8	:  9-8,
		APIC	: 10-9,
		Unused1	: 11-10,
		SEP	: 12-11,
		MTRR	: 13-12,
		PGE	: 14-13,
		MCA	: 15-14,
		CMOV	: 16-15,
		PAT	: 17-16,
		PSE36	: 18-17,
		PSN	: 19-18, // Intel
		CLFSH	: 20-19,
		Unused2	: 21-20,
		DS_PEBS	: 22-21,
		ACPI	: 23-22,
		MMX	: 24-23,
		FXSR	: 25-24, // FXSAVE and FXRSTOR instructions.
		SSE	: 26-25,
		SSE2	: 27-26,
		SS	: 28-27, // Intel
		HTT	: 29-28,
		TM1	: 30-29, // Intel
		Unused3	: 31-30,
		PBE	: 32-31; // Intel
	} EDX;
} CPUID_0x00000001;

typedef struct	// MONITOR & MWAIT Leaf.
{		// Common x86
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
	{	// Intel reseved.
		unsigned int
		Num_C0_MWAIT	:  4-0,
		Num_C1_MWAIT	:  8-4,
		Num_C2_MWAIT	: 12-8,
		Num_C3_MWAIT	: 16-12,
		Num_C4_MWAIT	: 20-16,
		ReservedBits	: 32-20;
	} EDX;
}  CPUID_0x00000005;

typedef struct THERMAL_POWER_LEAF
{	// Thermal and Power Management Leaf.
	struct
	{	// Most Intel reserved.
		unsigned int
		DTS	:  1-0,
		TurboIDA:  2-1, // Reports bit 38 of MSR 0x1a0
		ARAT	:  3-2, // Common x86
		Unused1	:  4-3,
		PLN	:  5-4,
		ECMD	:  6-5,
		PTM	:  7-6,
		HWP_Reg	:  8-7, // Hardware Performance registers
		HWP_Int	:  9-8, // IA32_HWP_INTERRUPT HWP_Notification.
		HWP_Act	: 10-9, // IA32_HWP_REQUEST Activity_Window
		HWP_Prf	: 11-10,// IA32_HWP_REQUEST Performance_Pref.
		HWP_Lvl	: 12-11,// IA32_HWP_REQUEST_PKG
		Unused2	: 13-12,
		HDC_Reg	: 15-13,// Hardware Duty Cycling registers
		Unused3	: 32-15;
	} EAX;
	struct
	{	// Intel reserved.
		unsigned int
		Threshld:  4-0,
		Unused1	: 32-4;
	} EBX;
    union
    {
	struct
	{	// Intel reserved.
		unsigned int
		HCF_Cap	:  1-0, // MSR: IA32_MPERF (E7H) & IA32_APERF (E8H)
		ACNT_Cap:  2-1,
		Unused1	:  3-2,
		SETBH	:  4-3,
		Unused2	: 32-4;
	};
	struct
	{	// AMD reserved.
		unsigned int
		EffFreq	:  1-0, // MSR0000_00E7 (MPERF) & MSR0000_00E8 (APERF)
		NotUsed : 32-1;
	};
    } ECX;
	struct
	{	// Intel reserved.
		unsigned int
		Unused1	: 32-0;
	} EDX;
} CPUID_0x00000006;

typedef struct	// Extended Feature Flags Enumeration Leaf.
{
	struct
	{	// Common x86
		unsigned int
		MaxSubLeaf	: 32-0;
	} EAX;
	struct
	{
		unsigned int
		FSGSBASE	:  1-0, // Common x86
		TSC_ADJUST	:  2-1,
		SGX		:  3-2,
		BMI1		:  4-3, // Common x86
		HLE		:  5-4,
		AVX2		:  6-5, // Common x86
		Unused1		:  7-6,
		SMEP		:  8-7, // Common x86
		BMI2		:  9-8, // Common x86
		FastStrings	: 10-9,
		INVPCID		: 11-10,
		RTM		: 12-11,
		PQM		: 13-12,
		FPU_CS_DS	: 14-13,
		MPX		: 15-14,
		PQE		: 16-15,
		Unused2		: 18-16,
		RDSEED		: 19-18,
		ADX		: 20-19,
		SMAP		: 21-20,
		Unused3		: 25-21,
		ProcessorTrace	: 26-25,
		Unused4		: 32-26;
	} EBX;
	struct
	{	// Intel reserved.
		unsigned int
		PREFETCHWT1	:  1-0,
		Unused1		:  3-1,
		PKU		:  4-3,
		OSPKE		:  5-4,
		Unused2		: 32-5;
	} ECX;
		unsigned int
	EDX			: 32-0; // Intel reserved.
} CPUID_0x00000007;

typedef struct	// Architectural Performance Monitoring Leaf.
{	// Intel reserved.
	struct
	{
		unsigned int
		Version	:  8-0,
		MonCtrs	: 16-8,
		MonWidth: 24-16,
		VectorSz: 32-24;
	} EAX;
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
	} EBX;
	struct
	{
		unsigned int
		Unused1	: 32-0;
	} ECX;
	struct
	{
		unsigned int
		FixCtrs	:  5-0,
		FixWidth: 13-5,
		Unused1	: 32-13;
	} EDX;
} CPUID_0x0000000a;

typedef struct	// Extended CPUID Function.
{
		unsigned int LargestExtFunc, EBX, ECX, EDX;
} CPUID_0x80000000;

typedef struct
{
    union
    {
	struct { // Intel reserved.
		unsigned int
		LAHFSAHF:  1-0,  // LAHF and SAHF instruction support.
		Unused1	: 32-1;
	};
	struct { // AMD reserved.
		unsigned int
		// Family 0Fh :
		LahfSahf:  1-0,
		MP_Mode	:  2-1,  // Core multi-processing legacy mode.
		SVM	:  3-2,  // Secure virtual machine.
		Ext_APIC:  4-3,  // Extended APIC space.
		AltMov	:  5-4,  // AltMovCr8
		ABM	:  6-5,  // LZCNT instruction support.
		SSE4A	:  7-6,
		AlignSSE:  8-7,  // Misaligned SSE mode.
		PREFETCH:  9-8,  // 3DNow PREFETCH, PREFETCHW instruction.
		// Family 15h :
		OSVW	: 10-9,  // OS-visible workaround support.
		IBS	: 11-10, // Instruction based sampling.
		XOP	: 12-11, // Extended operation support.
		SKINIT	: 13-12, // SKINIT and STGI support.
		WDT	: 14-13, // Watchdog timer support.
		NotUsed1: 15-14,
		LWP	: 16-15, // Lightweight profiling support.
		FMA4	: 17-16, // Four-operand FMA instruction.
		TCE	: 18-17, // Translation Cache Extension.
		NotUsed2: 21-18,
		TBM	: 22-21, // Trailing bit manipulation.
		TopoExt	: 23-22, // Topology extensions support.
		PerfCore: 24-23, // PerfCtrExtCore MSR.
		PerfNB	: 25-24, // PerfCtrExtNB MSR.
		NotUsed3: 26-25,
		Data_BP	: 27-26, // Data access breakpoint extension.
		PerfTSC	: 28-27, // Performance TSC MSR.
		PerfL2I	: 29-28, // L2I performance counter extensions support.
		MWaitExt: 30-29, // MWAITX/MONITORX support.
		NotUsed4: 32-30;
	};
    } ECX;
    union
    {
	struct { // Intel reserved.
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
	};
	struct { // AMD reserved.
		unsigned int	 // Most bits equal to CPUID 0x01
		FPU	:  1-0,
		VME	:  2-1,  // Virtual-mode enhancements.
		DE	:  3-2,  // Debugging extensions.
		PSE	:  4-3,  // Page-size extensions.
		TSC	:  5-4,
		MSR	:  6-5,  // AMD MSR.
		PAE	:  7-6,
		MCE	:  8-7,
		CMPXCH8	:  9-8,
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
		NX	: 21-20, // No-execute page protection.
		NotUsed3: 22-21,
		MMX_Ext : 23-22, // MMX extensions.
		MMX	: 24-23,
		FXSR	: 25-24,
		FFXSR	: 26-25, // FXSAVE and FXRSTOR optimizations.
		Page1GB	: 27-26,
		RDTSCP	: 28-27,
		NotUsed4: 29-28,
		LM	: 30-29, // Long mode.
		_3DNowEx: 31-30, // Extensions to 3DNow!
		_3DNow	: 32-31; // 3DNow! instructions.
	};
    } EDX;
} CPUID_0x80000001;

typedef struct	// Architectural Performance Monitoring Leaf.
{
	struct
	{
		unsigned int
		Unused1	: 32-0;
	} EAX, EBX, ECX;
    union
    {
	struct { // Intel reserved.
		unsigned int
		Unused1	:  8-0,
		Inv_TSC	:  9-8, // Invariant TSC available if 1
		Unused2	: 32-9;
	};
      union
      {		// AMD Family 0Fh
	struct {
		unsigned int
		TS	:  1-0,  // Temperature sensor
		FID	:  2-1,  // Frequency ID control is supported.
		VID	:  3-2,  // Voltage ID control is supported.
		TTP	:  4-3,  // THERMTRIP is supported = 1.
		TM	:  5-4,  // Hardware thermal control (HTC).
		STC	:  6-5,  // K7-K8: Software thermal control (STC)
		_100MHz	:  7-6,  // 100 MHz multiplier Control.
		NotUsed	: 32-7;
	};
	struct { // AMD Family 15h
		unsigned int
		Fam_0Fh	:  7-0,  // Family 0Fh features.
		HwPstate:  8-7,  // Hardware P-state control msr exist ?
		TscInv	:  9-8,  // Invariant TSC ?
		CPB	: 10-9,  // Core performance boost.
		EffFrqRO: 11-10, // Read-only effective frequency interf. msr ?
		ProcFb	: 12-11, // Processor feedback interface available if 1
		ProcPwr	: 13-12, // Core power reporting interface supported.
		Reserved: 32-13;
	};
      };
    } EDX;
} CPUID_0x80000007;

typedef struct	// Processor Capacity Leaf.
{
	struct {
		unsigned int
		MaxPhysicalAddr :  8-0,  // Common x86
		MaxLinearAddr	: 16-8,  // Common x86
		MaxGuestPhysAddr: 24-16, // AMD reserved
		Reserved	: 32-24;
	} EAX;
	struct { // AMD reserved
		unsigned int
		NC		:  8-0,  // Zero based number of physical cores 
		Reserved1	: 12-8,
		ApicIdCoreIdSize: 16-12, // Initial APIC ID size to compute MNC
		PerfTscSize	: 18-16, // 00b=40, 01b=48, 10b=56, 11b=64 bits
		Reserved2	: 32-18;
	} ECX;
	struct
	{
		unsigned int
		Reserved	: 32-0;
	} EBX, EDX;
} CPUID_0x80000008;

typedef struct	// BSP CPUID features.
{
	CPUID_FUNCTION Info;

	CPUID_0x00000001 Std;
	CPUID_0x00000005 MWait;
	CPUID_0x00000006 Power;
	CPUID_0x00000007 ExtFeature;
	CPUID_0x0000000a PerfMon;
	CPUID_0x80000001 ExtInfo;
	CPUID_0x80000007 AdvPower;

	unsigned int	FactoryFreq;

	struct {
		Bit32	InvariantTSC	:  8-0,
			HyperThreading	:  9-8,
			HTT_Enable	: 10-9,
			Ratio_Unlock	: 11-10,
			TDP_Unlock	: 12-11,
			TDP_Levels	: 14-12,
			TDP_Cfg_Lock	: 15-14,
			TDP_Cfg_Level	: 17-15,
			TurboRatio_Lock : 18-17,
			UnusedBits	: 24-18,
			SpecTurboRatio	: 32-24;
	};
} FEATURES;

#define MC_MAX_CTRL	2
#define MC_MAX_CHA	4
#define MC_MAX_DIMM	4

// Source: /drivers/char/agp/intel-agp.h
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
// Source: /drivers/edac/x38_edac.c
#ifndef PCI_DEVICE_ID_INTEL_X38_HB
	#define PCI_DEVICE_ID_INTEL_X38_HB		0x29e0
#endif
// Source: /drivers/edac/i3200_edac.c
#ifndef PCI_DEVICE_ID_INTEL_3200_HB
	#define PCI_DEVICE_ID_INTEL_3200_HB		0x29f0
#endif
// Source: /drivers/char/agp/intel-agp.h
#ifndef PCI_DEVICE_ID_INTEL_Q45_HB
	#define PCI_DEVICE_ID_INTEL_Q45_HB		0x2e10
#endif
#ifndef PCI_DEVICE_ID_INTEL_G45_HB
	#define PCI_DEVICE_ID_INTEL_G45_HB		0x2e20
#endif
#ifndef PCI_DEVICE_ID_INTEL_G41_HB
	#define PCI_DEVICE_ID_INTEL_G41_HB		0x2e30
#endif
// Source: /include/linux/pci_ids.h
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
#ifndef PCI_DEVICE_ID_INTEL_NEHALEM_EP_NON_CORE
	#define PCI_DEVICE_ID_INTEL_NEHALEM_EP_NON_CORE 0x2c58
#endif
#ifndef PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE
	#define PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE	0x2c51
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
// Source: Intel X58 Express Chipset Datasheet
#define PCI_DEVICE_ID_INTEL_X58_HUB_CTRL		0x3423
// Source: /include/linux/pci_ids.h
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0	0x3ca0
#endif
// Source: 2nd Generation Intel® Core™ Processor Family Vol2
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA	0x0100
#endif
#ifndef PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104
	#define PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104	0x0104
#endif
// Source: /drivers/edac/sb_edac.c
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0	0x0ea0
#endif
// Source: 3rd Generation Intel® Core™ Processor Family Vol2
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA	0x0150
#endif
#ifndef PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154
	#define PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154	0x0154
#endif
// Source: 4th, 5th Generation Intel® Core™ Processor Family Vol2 §3.0
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0	0x2fa0
#endif
#ifndef PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA
	#define PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA	0x0c00
#endif
#ifndef PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0
	#define PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0	0x1604
#endif
// Source: 6th Generation Intel® Processor Datasheet for S-Platforms Vol2
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD	0x190f
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ	0x191f
#endif
// Source: 6th Generation Intel® Processor Datasheet for H-Platforms Vol2
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD	0x1900
#endif
#ifndef PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ	0x1910
#endif
// Source: 7th Generation Intel® Processor for S-Platforms & Core X-Series Vol2
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA	0x5904
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA	0x590c
#endif
#ifndef PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD
	#define PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD	0x590f
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
// Source: 8th Generation Intel® Processor for S-Platforms Datasheet Vol2
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ 0x3e1f
#endif
#ifndef PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAH
	#define PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAH 0x3ec2
#endif
// Source: /include/linux/pci_ids.h
#ifndef PCI_DEVICE_ID_AMD_K8_NB_MEMCTL
	#define PCI_DEVICE_ID_AMD_K8_NB_MEMCTL		0x1102
#endif
#ifndef PCI_DEVICE_ID_AMD_K8_NB
	#define PCI_DEVICE_ID_AMD_K8_NB			0x1100
#endif

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

// Source: /include/uapi/linux/utsname.h
#ifdef __NEW_UTS_LEN
#define MAX_UTS_LEN		__NEW_UTS_LEN
#else
#define MAX_UTS_LEN		64
#endif

// Source: /include/linux/cpuidle.h
#ifndef _LINUX_CPUIDLE_H
#define CPUIDLE_STATE_MAX	10
#define CPUIDLE_NAME_LEN	16
#endif

typedef	struct {
	int			stateCount;
	struct {
		unsigned int	exitLatency;		/* in US */
			int	powerUsage;		/* in mW */
		unsigned int	targetResidency;	/* in US */
			char	Name[CPUIDLE_NAME_LEN];
	} State[CPUIDLE_STATE_MAX];
	char			Name[CPUIDLE_NAME_LEN],
				Governor[CPUIDLE_NAME_LEN];
} IDLEDRIVER;

#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN		16
#endif

#ifndef PID_MAX_DEFAULT
#define PID_MAX_DEFAULT		(1<<15)
#endif

enum SORTBYFIELD {F_STATE, F_RTIME, F_UTIME, F_STIME, F_PID, F_COMM};
#define SORTBYCOUNT	(1 + F_COMM)

typedef struct {
	unsigned long long	runtime,
				usertime,
				systime;
	pid_t			pid,		// type of __kernel_pid_t = int
				tgid,
				ppid;
	short int		state;		// TASK_STATE_MAX = 0x1000
	short int		wake_cpu;	// limited to 64K CPU
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

// Input-Output Control
#define COREFREQ_TOGGLE_OFF	0x0000000000000000L
#define COREFREQ_TOGGLE_ON	0x0000000000000001L

#define COREFREQ_IOCTL_MAGIC 0xc3

#define COREFREQ_IOCTL_SYSUPDT	_IO(COREFREQ_IOCTL_MAGIC, 0x1)
#define COREFREQ_IOCTL_SYSONCE	_IO(COREFREQ_IOCTL_MAGIC, 0x2)
#define COREFREQ_IOCTL_MACHINE	_IO(COREFREQ_IOCTL_MAGIC, 0x3)
#define COREFREQ_IOCTL_EIST	_IO(COREFREQ_IOCTL_MAGIC, 0x4)
#define COREFREQ_IOCTL_C1E	_IO(COREFREQ_IOCTL_MAGIC, 0x5)
#define COREFREQ_IOCTL_TURBO	_IO(COREFREQ_IOCTL_MAGIC, 0x6)
#define COREFREQ_IOCTL_C1A	_IO(COREFREQ_IOCTL_MAGIC, 0x7)
#define COREFREQ_IOCTL_C3A	_IO(COREFREQ_IOCTL_MAGIC, 0x8)
#define COREFREQ_IOCTL_C1U	_IO(COREFREQ_IOCTL_MAGIC, 0x9)
#define COREFREQ_IOCTL_C3U	_IO(COREFREQ_IOCTL_MAGIC, 0xa)
#define COREFREQ_IOCTL_PKGCST	_IO(COREFREQ_IOCTL_MAGIC, 0xb)
#define COREFREQ_IOCTL_IOMWAIT	_IO(COREFREQ_IOCTL_MAGIC, 0xc)
#define COREFREQ_IOCTL_IORCST	_IO(COREFREQ_IOCTL_MAGIC, 0xd)
#define COREFREQ_IOCTL_ODCM	_IO(COREFREQ_IOCTL_MAGIC, 0xe)
#define COREFREQ_IOCTL_ODCM_DC	_IO(COREFREQ_IOCTL_MAGIC, 0xf)

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
	RR_SMT
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

// Circular buffer
#define RING_SIZE	16

#define RING_NULL(Ring)							\
({									\
	((Ring.head - Ring.tail) == 0);					\
})

#define RING_FULL(Ring)							\
({									\
	((Ring.head - Ring.tail) == RING_SIZE);				\
})

#define RING_READ(Ring)							\
({									\
	Ring.buffer[Ring.tail++ & (RING_SIZE - 1)];			\
})

#define RING_WRITE(Ring, _cmd, _arg)					\
({									\
	struct RING_CTRL ctrl = {.arg = _arg, .cmd = _cmd};		\
	Ring.buffer[Ring.head++ & (RING_SIZE - 1)] = ctrl;		\
})
