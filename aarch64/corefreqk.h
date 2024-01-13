/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define Atomic_Read_VPMC(_lock, _dest, _src)				\
{									\
/*	__asm__ volatile						\
	(								\
		"xorq	%%rax,	%%rax"		"\n\t"			\
	_lock	"cmpxchg %%rax, %[src]" 	"\n\t"			\
		"movq	%%rax,	%[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "m" (_src)					\
		: "%rax", "cc", "memory"				\
	);							*/	\
}

#define Atomic_Add_VPMC(_lock, _dest, _src)				\
{									\
/*	__asm__ volatile						\
	(								\
		"xorq	%%rax,	%%rax"		"\n\t"			\
	_lock	"cmpxchg %%rax, %[src]" 	"\n\t"			\
		"addq	%%rax,	%[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "m" (_src)					\
		: "%rax", "cc", "memory"				\
	);							*/	\
}

#define ASM_CODE_RDMSR(_msr, _reg)					\
	"# Read PMU counter."			"\n\t"			\
	"mrs	" #_reg ", " #_msr		"\n\t"

#define ASM_RDMSR(_msr, _reg) ASM_CODE_RDMSR(_msr, _reg)

#define ASM_COUNTERx1(	_reg0, _reg1,					\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1)					\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	"# Store values into memory."		"\n\t"			\
	"str	" #_reg0 ",	%0"		"\n\t"			\
	"str	" #_reg1 ",	%1"		"\n\t"			\
	"isb"								\
	: "=m" (mem_tsc), "=m" (_mem1)					\
	:								\
	: "%" #_reg0"", "%" #_reg1"",					\
	  "cc", "memory"						\
);


#define ASM_COUNTERx2(	_reg0, _reg1, _reg2,				\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2)			\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	"# Store values into memory."		"\n\t"			\
	"str	" #_reg0 ",	%0"		"\n\t"			\
	"str	" #_reg1 ",	%1"		"\n\t"			\
	"str	" #_reg2 ",	%2"		"\n\t"			\
	"isb"								\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2)			\
	:								\
	: "%" #_reg0"", "%" #_reg1"", "%" #_reg2"",			\
	  "cc", "memory"						\
);


#define ASM_COUNTERx3(	_reg0, _reg1, _reg2, _reg3,			\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3)	\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	"# Store values into memory."		"\n\t"			\
	"str	" #_reg0 ",	%0"		"\n\t"			\
	"str	" #_reg1 ",	%1"		"\n\t"			\
	"str	" #_reg2 ",	%2"		"\n\t"			\
	"str	" #_reg3 ",	%3"		"\n\t"			\
	"isb"								\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3)	\
	:								\
	: "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "cc", "memory"						\
);

#define RDTSC_COUNTERx1(mem_tsc, ...) \
ASM_COUNTERx1(x1, x2, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx1(mem_tsc, ...) \
ASM_COUNTERx1(x1, x2, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx2(mem_tsc, ...) \
ASM_COUNTERx2(x1, x2, x3, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx2(mem_tsc, ...) \
ASM_COUNTERx2(x1, x2, x3, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx3(mem_tsc, ...) \
ASM_COUNTERx3(x1, x2, x3, x4, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx3(mem_tsc, ...) \
ASM_COUNTERx3(x1, x2, x3, x4, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

/* Manufacturers Identifier Strings.					*/
#define VENDOR_RESERVED "Reserved"
#define VENDOR_ARM	"Arm"
#define VENDOR_BROADCOM "Broadcom"
#define VENDOR_CAVIUM	"Cavium"
#define VENDOR_DEC	"DEC"		/* Digital Equipment Corporation */
#define VENDOR_FUJITSU	"Fujitsu"
#define VENDOR_INFINEON "Infineon"
#define VENDOR_MOTOROLA "Motorola"	/* Freescale Semiconductor Inc. */
#define VENDOR_NVIDIA	"NVIDIA"
#define VENDOR_APM	"APM"		/* Applied Micro Circuits Corporation */
#define VENDOR_QUALCOMM "Qualcomm"
#define VENDOR_MARVELL	"Marvell"
#define VENDOR_INTEL	"Intel"
#define VENDOR_AMPERE	"Ampere"	/* Ampere Computing		*/

#define VENDOR_KVM	"TCGTGTCGCGTC"
#define VENDOR_VBOX	"VBoxVBoxVBox"
#define VENDOR_KBOX	"KVMKM"
#define VENDOR_VMWARE	"VMwawarereVM"
#define VENDOR_HYPERV	"Micrt Hvosof"

#define LATCH_NONE		0b000000000000
#define LATCH_TGT_RATIO_UNLOCK	0b000000000001	/* <T>	TgtRatioUnlocked */
#define LATCH_CLK_RATIO_UNLOCK	0b000000000010	/* <X>	ClkRatioUnlocked */
#define LATCH_TURBO_UNLOCK	0b000000000100	/* <B>	TurboUnlocked	 */
#define LATCH_UNCORE_UNLOCK	0b000000001000	/* <U>	UncoreUnlocked	 */
#define LATCH_HSMP_CAPABLE	0b000000010000	/* <H>	HSMP Capability  */

typedef struct {
	char			**Brand;
	unsigned int		Boost[2];
	THERMAL_PARAM		Param;
	unsigned int		CodeNameIdx	:  8-0,
				TgtRatioUnlocked:  9-8,  /*	<T:1>	*/
				ClkRatioUnlocked: 11-9,  /*	<X:2>	*/
				TurboUnlocked	: 12-11, /*	<B:1>	*/
				UncoreUnlocked	: 13-12, /*	<U:1>	*/
				HSMP_Capable	: 14-13, /*	<H:1>	*/
				_UnusedLatchBits: 20-14,
				/* <R>-<H>-<U>-<B>-<X>-<T> */
				Latch		: 32-20;
} PROCESSOR_SPECIFIC;

typedef struct {
	FEATURES	*Features;
	unsigned int	SMT_Count,
			localProcessor;
	enum HYPERVISOR HypervisorID;
	signed int	rc;
} INIT_ARG;

typedef struct {			/* V[0] stores the previous TSC */
	unsigned long long V[2];	/* V[1] stores the current TSC	*/
} TSC_STRUCT;

#define OCCURRENCES 4
/* OCCURRENCES x 2 (TSC values) needs a 64-byte cache line size.	*/
#define STRUCT_SIZE (OCCURRENCES * sizeof(TSC_STRUCT))

typedef struct {
	TSC_STRUCT	*TSC[2];
	CLOCK		Clock;
} COMPUTE_ARG;

typedef struct
{
	PROC_RO 		*Proc_RO;
	PROC_RW 		*Proc_RW;
	SYSGATE_RO		*Gate;
	struct kmem_cache	*Cache;
	CORE_RO 		**Core_RO;
	CORE_RW 		**Core_RW;
} KPUBLIC;

enum { CREATED, STARTED, MUSTFWD };

typedef struct
{
	struct hrtimer		Timer;
/*
		TSM: Timer State Machine
			CREATED: 1-0
			STARTED: 2-1
			MUSTFWD: 3-2
*/
	Bit64			TSM __attribute__ ((aligned (8)));
} JOIN;

typedef struct
{
	PROCESSOR_SPECIFIC	*Specific;

	struct kmem_cache	*Cache;

	struct PRIV_CORE_ST {
		JOIN		Join;

		union SAVE_AREA_CORE {
		    struct
		    {
			PMCR		PMCR;
			PMSELR		PMSELR;
			PMXEVTYPER	PMTYPE[3];
			PMCCFILTR	PMCCFILTR;
			PMCNTENSET	PMCNTEN;
		    };
		} SaveArea;
	} *Core[];
} KPRIVATE;

struct SMBIOS16
{					/*	DMTF	2.7.1		*/
	u8	type;			/*	0x00	BYTE		*/
	u8	length; 		/*	0x01	BYTE		*/
	u16	handle; 		/*	0x02	WORD		*/
	u8	location;		/*	0x04	BYTE		*/
	u8	use;			/*	0x05	BYTE		*/
	u8	error_correction;	/*	0x06	BYTE		*/
	u32	maximum_capacity;	/*	0x07	DWORD		*/
	u16	error_handle;		/*	0x0b	WORD		*/
	u16	number_devices; 	/*	0x0d	WORD		*/
	u64	extended_capacity;	/*	0x0f	QWORD		*/
} __attribute__((__packed__))s;

struct SMBIOS17
{					/*	DMTF 2.7.1		*/
	u8	type;			/*	0x00	BYTE		*/
	u8	length; 		/*	0x01	BYTE		*/
	u16	handle; 		/*	0x02	WORD		*/
	u16	phys_mem_array_handle;	/*	0x04	WORD		*/
	u16	mem_err_info_handle;	/*	0x06	WORD		*/
	u16	total_width;		/*	0x08	WORD		*/
	u16	data_width;		/*	0x0a	WORD		*/
	u16	size;			/*	0x0c	WORD		*/
	u8	form_factor;		/*	0x0e	BYTE		*/
	u8	device_set;		/*	0x0f	BYTE		*/
	u8	device_locator_id;	/*	0x10	BYTE	STRING	*/
	u8	bank_locator_id;	/*	0x11	BYTE	STRING	*/
	u8	memory_type;		/*	0x12	BYTE		*/
	u16	type_detail;		/*	0x13	WORD		*/
	u16	speed;			/*	0x15	WORD		*/
	u8	manufacturer_id;	/*	0x17	BYTE	STRING	*/
	u8	serial_number_id;	/*	0x18	BYTE	STRING	*/
	u8	asset_tag_id;		/*	0x19	BYTE	STRING	*/
	u8	part_number_id; 	/*	0x1a	BYTE	STRING	*/
	u8	attributes;		/*	0x1b	BYTE		*/
	u32	extended_size;		/*	0x1c	DWORD		*/
	u16	conf_mem_clk_speed;	/*	0x20	WORD		*/
					/*	DMTF 3.2.0		*/
	u16	min_voltage;		/*	0x22	WORD		*/
	u16	max_voltage;		/*	0x24	WORD		*/
	u16	configured_voltage;	/*	0x26	WORD		*/
	u8	memory_tech;		/*	0x28	BYTE		*/
	u16	memory_capability;	/*	0x29	WORD		*/
	u8	firmware_version_id;	/*	0x2b	BYTE	STRING	*/
	u16	manufacturer_spd;	/*	0x2c	WORD		*/
	u16	product_spd;		/*	0x2e	WORD		*/
	u16	controller_mfr_spd;	/*	0x30	WORD		*/
	u16	controller_pdt_spd;	/*	0x32	WORD		*/
	u64	non_volatile_size;	/*	0x34	QWORD		*/
	u64	volatile_size;		/*	0x3c	QWORD		*/
	u64	cache_size;		/*	0x44	QWORD		*/
	u64	logical_size;		/*	0x4c	QWORD		*/
					/*	DMTF 3.3.0		*/
	u32	extended_speed; 	/*	0x54	DWORD		*/
	u32	extended_conf_speed;	/*	0x58	DWORD		*/
} __attribute__((__packed__));

#if !defined(RHEL_MAJOR)
	#define RHEL_MAJOR 0
#endif

#if !defined(RHEL_MINOR)
	#define RHEL_MINOR 0
#endif

typedef struct {
	char			*Name,
				Desc[CPUIDLE_NAME_LEN];
	unsigned long		flags;
	unsigned short		Latency,
				Residency;
} IDLE_STATE;

typedef struct {
	IDLE_STATE		*IdleState;
	unsigned int		(*GetFreq)(unsigned int cpu);
	void			(*SetTarget)(void *arg);
} SYSTEM_DRIVER;

typedef struct {
	char			**Brand;
	enum CODENAME		CN;
} ARCH_ST;

typedef struct
{
	struct	SIGNATURE	Signature;
	void			(*Query)(unsigned int cpu);
	void			(*Update)(void *arg);	/* Must be static */
	void			(*Start)(void *arg);	/* Must be static */
	void			(*Stop)(void *arg);	/* Must be static */
	void			(*Exit)(void);
	void			(*Timer)(unsigned int cpu);
	CLOCK			(*BaseClock)(unsigned int ratio);
	long			(*ClockMod)(CLOCK_ARG *pClockMod);
	long			(*TurboClock)(CLOCK_ARG *pClockMod);
	enum THERMAL_FORMULAS	thermalFormula;
	enum VOLTAGE_FORMULAS	voltageFormula;
	enum POWER_FORMULAS	powerFormula;
	struct pci_device_id	*PCI_ids;
	struct {
		void		(*Start)(void *arg);	/* Must be static */
		void		(*Stop)(void *arg);	/* Must be static */
		long		(*ClockMod)(CLOCK_ARG *pClockMod);
	} Uncore;
	PROCESSOR_SPECIFIC	*Specific;
	SYSTEM_DRIVER		SystemDriver;
	ARCH_ST			Architecture;
} ARCH;

static CLOCK BaseClock_GenericMachine(unsigned int ratio) ;
static void Query_GenericMachine(unsigned int cpu) ;
static void PerCore_GenericMachine(void *arg) ;
static void Start_GenericMachine(void *arg) ;
static void Stop_GenericMachine(void *arg) ;
static void InitTimer_GenericMachine(unsigned int cpu) ;
/*	[Void]								*/
#define _Void_Signature {.ExtFamily=0x00, .Family=0x0, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A5	{.ExtFamily=0xc0, .Family=0x5, .ExtModel=0x0, .Model=0x5}
#define _Cortex_A7	{.ExtFamily=0xc0, .Family=0x7, .ExtModel=0x0, .Model=0x7}
#define _Cortex_A9	{.ExtFamily=0xc0, .Family=0x9, .ExtModel=0x0, .Model=0x9}
#define _Cortex_A15	{.ExtFamily=0xc0, .Family=0xf, .ExtModel=0x0, .Model=0xf}
#define _Cortex_A17	{.ExtFamily=0xc0, .Family=0xe, .ExtModel=0x0, .Model=0xe}
#define _Cortex_A32	{.ExtFamily=0xd0, .Family=0x1, .ExtModel=0x0, .Model=0x6}
#define _Cortex_A34	{.ExtFamily=0xd0, .Family=0x2, .ExtModel=0x0, .Model=0x8}
#define _Cortex_A35	{.ExtFamily=0xd0, .Family=0x4, .ExtModel=0x0, .Model=0xa}
#define _Cortex_A510	{.ExtFamily=0xd4, .Family=0x6, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A520	{.ExtFamily=0xd8, .Family=0x0, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A53	{.ExtFamily=0xd0, .Family=0x3, .ExtModel=0x0, .Model=0x3}
#define _Cortex_A55	{.ExtFamily=0xd0, .Family=0x5, .ExtModel=0x4, .Model=0x5}
#define _Cortex_A57	{.ExtFamily=0xd0, .Family=0x7, .ExtModel=0x0, .Model=0x1}
#define _Cortex_A65	{.ExtFamily=0xd0, .Family=0x6, .ExtModel=0x4, .Model=0x6}
#define _Cortex_A65AE	{.ExtFamily=0xd4, .Family=0x3, .ExtModel=0x4, .Model=0x7}
#define _Cortex_A710	{.ExtFamily=0xd4, .Family=0x7, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A715	{.ExtFamily=0xd4, .Family=0xd, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A72	{.ExtFamily=0xd0, .Family=0x8, .ExtModel=0x0, .Model=0x2}
#define _Cortex_A720	{.ExtFamily=0xd8, .Family=0x1, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A73	{.ExtFamily=0xd0, .Family=0x9, .ExtModel=0x0, .Model=0x4}
#define _Cortex_A75	{.ExtFamily=0xd0, .Family=0xa, .ExtModel=0x4, .Model=0xa}
#define _Cortex_A76	{.ExtFamily=0xd0, .Family=0xb, .ExtModel=0x0, .Model=0xb}
#define _Cortex_A76AE	{.ExtFamily=0xd0, .Family=0xe, .ExtModel=0x1, .Model=0x1}
#define _Cortex_A77	{.ExtFamily=0xd0, .Family=0xd, .ExtModel=0x1, .Model=0x0}
#define _Cortex_A78	{.ExtFamily=0xd4, .Family=0x1, .ExtModel=0x2, .Model=0x1}
#define _Cortex_A78AE	{.ExtFamily=0xd4, .Family=0x2, .ExtModel=0x2, .Model=0x2}
#define _Cortex_A78C	{.ExtFamily=0xd4, .Family=0xb, .ExtModel=0x2, .Model=0x4}
#define _Cortex_R4	{.ExtFamily=0xc1, .Family=0x4, .ExtModel=0x1, .Model=0x4}
#define _Cortex_R5	{.ExtFamily=0xc1, .Family=0x5, .ExtModel=0x1, .Model=0x5}
#define _Cortex_R52	{.ExtFamily=0xd1, .Family=0x3, .ExtModel=0x1, .Model=0x3}
#define _Cortex_R52Plus {.ExtFamily=0xd1, .Family=0x6, .ExtModel=0x1, .Model=0x6}
#define _Cortex_R82	{.ExtFamily=0xd1, .Family=0x5, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X1	{.ExtFamily=0xd4, .Family=0x4, .ExtModel=0x2, .Model=0x3}
#define _Cortex_X1C	{.ExtFamily=0xd4, .Family=0xc, .ExtModel=0x2, .Model=0x5}
#define _Cortex_X2	{.ExtFamily=0xd4, .Family=0x8, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X3	{.ExtFamily=0xd4, .Family=0xe, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X4	{.ExtFamily=0xd8, .Family=0x2, .ExtModel=0x0, .Model=0x0}
#define _DynamIQ_DSU	{.ExtFamily=0x00, .Family=0x0, .ExtModel=0x4, .Model=0x1}
#define _Neoverse_E1	{.ExtFamily=0xd4, .Family=0xa, .ExtModel=0x4, .Model=0x6}
#define _Neoverse_N1	{.ExtFamily=0xd0, .Family=0xc, .ExtModel=0x0, .Model=0xc}
#define _Neoverse_N2	{.ExtFamily=0xd4, .Family=0x9, .ExtModel=0x0, .Model=0x0}
#define _Neoverse_V1	{.ExtFamily=0xd4, .Family=0x0, .ExtModel=0x2, .Model=0x1}
#define _Neoverse_V2	{.ExtFamily=0xd4, .Family=0xf, .ExtModel=0x0, .Model=0x0}

typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static char *CodeName[CODENAMES] = {
	[    ARMv1]	= "ARMv1",
	[  ARMv7_R]	= "ARMv7-R",
	[  ARMv7_A]	= "ARMv7-A",
	[  ARMv8_R]	= "ARMv8-R",
	[  ARMv8_A]	= "ARMv8-A",
	[ARMv8_2_A]	= "ARMv8.2-A",
	[ARMv8_3_A]	= "ARMv8.3-A",
	[ARMv8_4_A]	= "ARMv8.4-A",
	[  ARMv8_5]	= "ARMv8.5",
	[  ARMv8_6]	= "ARMv8.6",
	[  ARMv8_7]	= "ARMv8.7",
	[  ARMv9_A]	= "ARMv9-A",
	[  ARMv9_4]	= "ARMv9.4",
	[  ARMv9_5]	= "ARMv9.5"
};

const ARCH_ST Arch_Misc_Processor = {.Brand = ZLIST(NULL), .CN = ARMv1},
	Arch_Cortex_A5 = {.Brand = ZLIST("Cortex-A5"), .CN = ARMv7_A},
	Arch_Cortex_A7 = {.Brand = ZLIST("Cortex-A7"), .CN = ARMv7_A},
	Arch_Cortex_A9 = {.Brand = ZLIST("Cortex-A9"), .CN = ARMv7_A},
	Arch_Cortex_A15 = {.Brand = ZLIST("Cortex-A15"), .CN = ARMv7_A},
	Arch_Cortex_A17 = {.Brand = ZLIST("Cortex-A17"), .CN = ARMv7_A},
	Arch_Cortex_A32 = {.Brand = ZLIST("Cortex-A32"), .CN = ARMv8_A},
	Arch_Cortex_A34 = {.Brand = ZLIST("Cortex-A34"), .CN = ARMv8_A},
	Arch_Cortex_A35 = {.Brand = ZLIST("Cortex-A35"), .CN = ARMv8_A},
	Arch_Cortex_A510 = {.Brand = ZLIST("Cortex-A510"), .CN = ARMv9_A},
	Arch_Cortex_A520 = {.Brand = ZLIST("Cortex-A520"), .CN = ARMv9_A},
	Arch_Cortex_A53 = {.Brand = ZLIST("Cortex-A53"), .CN = ARMv8_A},
	Arch_Cortex_A55 = {.Brand = ZLIST("Cortex-A55"), .CN = ARMv8_2_A},
	Arch_Cortex_A57 = {.Brand = ZLIST("Cortex-A57"), .CN = ARMv8_A},
	Arch_Cortex_A65 = {.Brand = ZLIST("Cortex-A65"), .CN = ARMv8_2_A},
	Arch_Cortex_A65AE = {.Brand = ZLIST("Cortex-A65AE"), .CN = ARMv8_2_A},
	Arch_Cortex_A710 = {.Brand = ZLIST("Cortex-A710"), .CN = ARMv9_A},
	Arch_Cortex_A715 = {.Brand = ZLIST("Cortex-A715"), .CN = ARMv9_A},
	Arch_Cortex_A72 = {.Brand = ZLIST("Cortex-A72"), .CN = ARMv8_A},
	Arch_Cortex_A720 = {.Brand = ZLIST("Cortex-A720"), .CN = ARMv9_A},
	Arch_Cortex_A73 = {.Brand = ZLIST("Cortex-A73"), .CN = ARMv8_A},
	Arch_Cortex_A75 = {.Brand = ZLIST("Cortex-A75"), .CN = ARMv8_2_A},
	Arch_Cortex_A76 = {.Brand = ZLIST("Cortex-A76"), .CN = ARMv8_2_A},
	Arch_Cortex_A76AE = {.Brand = ZLIST("Cortex-A76AE"), .CN = ARMv8_2_A},
	Arch_Cortex_A77 = {.Brand = ZLIST("Cortex-A77"), .CN = ARMv8_2_A},
	Arch_Cortex_A78 = {.Brand = ZLIST("Cortex-A78"), .CN = ARMv8_2_A},
	Arch_Cortex_A78AE = {.Brand = ZLIST("Cortex-A78AE"), .CN = ARMv8_2_A},
	Arch_Cortex_A78C = {.Brand = ZLIST("Cortex-A78C"), .CN = ARMv8_2_A},
	Arch_Cortex_R4 = {.Brand = ZLIST("Cortex-R4"), .CN = ARMv7_R},
	Arch_Cortex_R5 = {.Brand = ZLIST("Cortex-R5"), .CN = ARMv7_R},
	Arch_Cortex_R52 = {.Brand = ZLIST("Cortex-R52"), .CN = ARMv8_R},
	Arch_Cortex_R52Plus = {.Brand = ZLIST("Cortex-R52+"), .CN = ARMv8_R},
	Arch_Cortex_R82 = {.Brand = ZLIST("Cortex-R82"), .CN = ARMv8_R},
	Arch_Cortex_X1 = {.Brand = ZLIST("Cortex-X1"), .CN = ARMv8_2_A},
	Arch_Cortex_X1C = {.Brand = ZLIST("Cortex-X1C"), .CN = ARMv8_2_A},
	Arch_Cortex_X2 = {.Brand = ZLIST("Cortex-X2"), .CN = ARMv9_A},
	Arch_Cortex_X3 = {.Brand = ZLIST("Cortex-X3"), .CN = ARMv9_A},
	Arch_Cortex_X4 = {.Brand = ZLIST("Cortex-X4"), .CN = ARMv9_A},
	Arch_DynamIQ_DSU = {.Brand = ZLIST("DynamIQ DSU"), .CN = ARMv8_2_A},
	Arch_Neoverse_E1 = {.Brand = ZLIST("Neoverse E1"), .CN = ARMv8_2_A},
	Arch_Neoverse_N1 = {.Brand = ZLIST("Neoverse N1"), .CN = ARMv8_2_A},
	Arch_Neoverse_N2 = {.Brand = ZLIST("Neoverse N2"), .CN = ARMv9_A},
	Arch_Neoverse_V1 = {.Brand = ZLIST("Neoverse V1"), .CN = ARMv8_4_A},
	Arch_Neoverse_V2 = {.Brand = ZLIST("Neoverse V2"), .CN = ARMv9_A};

static PROCESSOR_SPECIFIC Misc_Specific_Processor[] = {
	{0}
};

#ifdef CONFIG_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 19)
#define CPUFREQ_POLICY_UNKNOWN		(0)
#endif
static int CoreFreqK_Policy_Exit(struct cpufreq_policy*) ;
static int CoreFreqK_Policy_Init(struct cpufreq_policy*) ;
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 19))	\
  && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 5, 0)))	\
  || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 3))	\
  || (RHEL_MAJOR == 8)
static int CoreFreqK_Policy_Verify(struct cpufreq_policy_data*) ;
#else
static int CoreFreqK_Policy_Verify(struct cpufreq_policy*) ;
#endif
static int CoreFreqK_SetPolicy(struct cpufreq_policy*) ;
static int CoreFreqK_Bios_Limit(int, unsigned int*) ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)	\
  || ((RHEL_MAJOR == 8)  && (RHEL_MINOR > 3))
static int CoreFreqK_SetBoost(struct cpufreq_policy*, int) ;
#else
static int CoreFreqK_SetBoost(int) ;
#endif
static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy*, char*);
static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy*, unsigned int) ;
#endif /* CONFIG_CPU_FREQ */

static unsigned int Policy_GetFreq(unsigned int cpu) ;

#define VOID_Driver {							\
	.IdleState	= NULL ,					\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= NULL						\
}

static ARCH Arch[ARCHITECTURES] = {
[GenuineArch] = {							/*  0*/
	.Signature = _Void_Signature,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Misc_Processor
	},
[Cortex_A5] = {
	.Signature = _Cortex_A5,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A5
	},
[Cortex_A7] = {
	.Signature = _Cortex_A7,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A7
	},
[Cortex_A9] = {
	.Signature = _Cortex_A9,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A9
	},
[Cortex_A15] = {
	.Signature = _Cortex_A15,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A15
	},
[Cortex_A17] = {
	.Signature = _Cortex_A17,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A17
	},
[Cortex_A32] = {
	.Signature = _Cortex_A32,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A32
	},
[Cortex_A34] = {
	.Signature = _Cortex_A34,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A34
	},
[Cortex_A35] = {
	.Signature = _Cortex_A35,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A35
	},
[Cortex_A510] = {
	.Signature = _Cortex_A510,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A510
	},
[Cortex_A520] = {
	.Signature = _Cortex_A520,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A520
	},
[Cortex_A53] = {
	.Signature = _Cortex_A53,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A53
	},
[Cortex_A55] = {
	.Signature = _Cortex_A55,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A55
	},
[Cortex_A57] = {
	.Signature = _Cortex_A57,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A57
	},
[Cortex_A65] = {
	.Signature = _Cortex_A65,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A65
	},
[Cortex_A65AE] = {
	.Signature = _Cortex_A65AE,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A65AE
	},
[Cortex_A710] = {
	.Signature = _Cortex_A710,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A710
	},
[Cortex_A715] = {
	.Signature = _Cortex_A715,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A715
	},
[Cortex_A72] = {
	.Signature = _Cortex_A72,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A72
	},
[Cortex_A720] = {
	.Signature = _Cortex_A720,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A720
	},
[Cortex_A73] = {
	.Signature = _Cortex_A73,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A73
	},
[Cortex_A75] = {
	.Signature = _Cortex_A75,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A75
	},
[Cortex_A76] = {
	.Signature = _Cortex_A76,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A76
	},
[Cortex_A76AE] = {
	.Signature = _Cortex_A76AE,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A76AE
	},
[Cortex_A77] = {
	.Signature = _Cortex_A77,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A77
	},
[Cortex_A78] = {
	.Signature = _Cortex_A78,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A78
	},
[Cortex_A78AE] = {
	.Signature = _Cortex_A78AE,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A78AE
	},
[Cortex_A78C] = {
	.Signature = _Cortex_A78C,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_A78C
	},
[Cortex_R4] = {
	.Signature = _Cortex_R4,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_R4
	},
[Cortex_R5] = {
	.Signature = _Cortex_R5,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_R5
	},
[Cortex_R52] = {
	.Signature = _Cortex_R52,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_R52
	},
[Cortex_R52Plus] = {
	.Signature = _Cortex_R52Plus,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_R52Plus
	},
[Cortex_R82] = {
	.Signature = _Cortex_R82,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_R82
	},
[Cortex_X1] = {
	.Signature = _Cortex_X1,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_X1
	},
[Cortex_X1C] = {
	.Signature = _Cortex_X1C,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_X1C
	},
[Cortex_X2] = {
	.Signature = _Cortex_X2,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_X2
	},
[Cortex_X3] = {
	.Signature = _Cortex_X3,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_X3
	},
[Cortex_X4] = {
	.Signature = _Cortex_X4,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Cortex_X4
	},
[DynamIQ_DSU] = {
	.Signature = _DynamIQ_DSU,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_DynamIQ_DSU
	},
[Neoverse_E1] = {
	.Signature = _Neoverse_E1,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Neoverse_E1
	},
[Neoverse_N1] = {
	.Signature = _Neoverse_N1,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Neoverse_N1
	},
[Neoverse_N2] = {
	.Signature = _Neoverse_N2,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Neoverse_N2
	},
[Neoverse_V1] = {
	.Signature = _Neoverse_V1,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Neoverse_V1
	},
[Neoverse_V2] = {
	.Signature = _Neoverse_V2,
	.Query = Query_GenericMachine,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Neoverse_V2
	}
};
