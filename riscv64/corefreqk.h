/*
 * CoreFreq
 * Copyright (C) 2015-2025 CYRIL COURTIAT
 * Licenses: GPL2
 */

#if defined(CONFIG_OF) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
#define of_cpu_device_node_get(cpu)					\
({									\
	struct device_node *cpu_node;					\
	struct device *cpu_dev; 					\
	cpu_dev = get_cpu_device(cpu);					\
	if (!cpu_dev) { 						\
		cpu_node = of_get_cpu_node(cpu, NULL);			\
	} else {							\
		cpu_node = of_node_get(cpu_dev->of_node);		\
	}								\
	cpu_node;							\
})
#endif

#if defined(CONFIG_OF) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
#define of_device_compatible_match(_device_, _compat_)			\
({									\
	const struct device_node *device = _device_;			\
	const char *const *compat = _compat_;				\
	unsigned int tmp, score = 0;					\
									\
	if (compat != NULL)						\
		while (*compat) {					\
			tmp = of_device_is_compatible(device, *compat); \
			if (tmp > score)				\
				score = tmp;				\
			compat++;					\
		}							\
	score;								\
})
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	#define sys_reg(op0, op1, crn, crm, op2) ({	\
		UNUSED(op0);				\
		UNUSED(op1);				\
		UNUSED(crn);				\
		UNUSED(crm);				\
		UNUSED(op2);				\
	})
#endif
/*
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
	#define SysRegRead(_reg)	read_sysreg_s(_reg)
	#define SysRegWrite(_val, _reg) write_sysreg_s(_val, _reg)
#else
	#define SysRegRead(_reg) ({			\
		UNUSED(_reg);				\
		0;					\
	})
	#define SysRegWrite(_val, _reg) ({		\
		UNUSED(_val);				\
		UNUSED(_reg);				\
	})
#endif
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

/* Manufacturers Identifier Strings.					*/
#define VENDOR_RESERVED "Reserved"
#define VENDOR_RISC	"RISC"
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
#define LATCH_OTHER_CAPABLE	0b000000010000	/* <H>	Other Capability */

typedef struct {
	char			**Brand;
	unsigned int		Boost[2];
	THERMAL_PARAM		Param;
	unsigned int		CodeNameIdx	:  8-0,
				TgtRatioUnlocked:  9-8,  /*	<T:1>	*/
				ClkRatioUnlocked: 11-9,  /*	<X:2>	*/
				TurboUnlocked	: 12-11, /*	<B:1>	*/
				UncoreUnlocked	: 13-12, /*	<U:1>	*/
				Other_Capable	: 14-13, /*	<H:1>	*/
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
			SCOUNTEREN scounteren;
		    };
		} SaveArea;
#ifdef CONFIG_CPU_FREQ
		struct cpufreq_policy	FreqPolicy;
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_PM_OPP
		struct {
			signed int	VID;
		} OPP[BOOST(SIZE) - BOOST(18C)];
#endif /* CONFIG_PM_OPP */
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

#define FREQ2COF(_frequency, _COF)					\
({									\
	_COF.Q = (_frequency) / UNIT_KHz(PRECISION),			\
	_COF.R = (_frequency) - (_COF.Q * UNIT_KHz(PRECISION)); 	\
})

#define COF2FREQ(_COF)	( (_COF.Q * UNIT_KHz(PRECISION)) + _COF.R )

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
	char			**Architecture;
} ARCH;

static CLOCK BaseClock_GenericMachine(unsigned int ratio) ;
static void Query_GenericMachine(unsigned int cpu) ;
static void PerCore_GenericMachine(void *arg) ;
static void Start_GenericMachine(void *arg) ;
static void Stop_GenericMachine(void *arg) ;
static void InitTimer_GenericMachine(unsigned int cpu) ;

/*	[Void]								*/
#define _Void_Signature {.ExtFamily=0x00,.Family=0x0, .ExtModel=0x0,.Model=0x0}
#define _Microchip {.ExtFamily=0x02, .Family=0x9, .ExtModel=0x0, .Model=0x0}
#define _Andes {.ExtFamily=0x31, .Family=0xE, .ExtModel=0x0, .Model=0x0}
#define _SiFive {.ExtFamily=0x48, .Family=0x9, .ExtModel=0x0, .Model=0x0}
#define _T_Head {.ExtFamily=0x5B, .Family=0x7, .ExtModel=0x0, .Model=0x0}
#define _Veyron {.ExtFamily=0x61, .Family=0xF, .ExtModel=0x0, .Model=0x0}
#define _SpacemiT {.ExtFamily=0x71, .Family=0x0, .ExtModel=0x0, .Model=0x0}

typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static char *Arch_Generic[]	=	ZLIST("RISC-V");
static char *Arch_Microchip[]	=	ZLIST("Microchip");
static char *Arch_Andes[]	=	ZLIST("Andes");
static char *Arch_SiFive[]	=	ZLIST("SiFive");
static char *Arch_T_Head[]	=	ZLIST("T-Head");
static char *Arch_Veyron[]	=	ZLIST("Veyron");
static char *Arch_SpacemiT[]	=	ZLIST("SpacemiT");

static PROCESSOR_SPECIFIC Void_Specific[] = { {0} };

static PROCESSOR_SPECIFIC Misc_Specific_Processor[] = {
	{0}
};

#ifdef CONFIG_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 19)
#define CPUFREQ_POLICY_UNKNOWN		(0)
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void CoreFreqK_Policy_Exit(struct cpufreq_policy *policy) ;
#else
static int CoreFreqK_Policy_Exit(struct cpufreq_policy*) ;
#endif
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
[GenericArch] = {							/*  0*/
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Misc_Specific_Processor,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Generic
	},
[Microchip] = {
	.Signature = _Microchip,
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Microchip
	},
[Andes] = {
	.Signature = _Andes,
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Andes
	},
[SiFive] = {
	.Signature = _SiFive,
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_SiFive
	},
[T_Head] = {
	.Signature = _T_Head,
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_T_Head
	},
[Veyron] = {
	.Signature = _Veyron,
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Veyron
	},
[SpacemiT] = {
	.Signature = _SpacemiT,
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
#ifdef CONFIG_PM_OPP
	.voltageFormula = VOLTAGE_FORMULA_OPP,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_SpacemiT
	}
};
