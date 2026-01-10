/*
 * CoreFreq
 * Copyright (C) 2015-2026 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define DSU_DEVICE_TREE_LIST						\
{									\
	{ .compatible = "arm,dsu-pmu",	  .data = (void *) DSU_100 },	\
	{ .compatible = "arm,dsu-110-pmu",.data = (void *) DSU_110 },	\
	{ /* EOL */ }							\
}

#define DSU_ACPI_HID_LIST						\
{									\
	{ "ARMHD500",	DSU_100 	},				\
	{ "ARMHD510",	DSU_110 	},				\
	{ "",		0		}				\
}

#define CMN_DEVICE_TREE_LIST						\
{									\
	{ .compatible = "arm,cmn-600",	.data = (void *) CMN_600  },	\
	{ .compatible = "arm,cmn-650",	.data = (void *) CMN_650  },	\
	{ .compatible = "arm,cmn-700",	.data = (void *) CMN_700  },	\
	{ .compatible = "arm,cmn-s3",	.data = (void *) CMN_S3   },	\
	{ .compatible = "arm,ci-700",	.data = (void *) CMN_CI700},	\
	{ /* EOL */ }							\
}

#define CMN_ACPI_HID_LIST						\
{									\
	{ "ARMHC600",	CMN_600 	},				\
	{ "ARMHC650",	CMN_650 	},				\
	{ "ARMHC700",	CMN_700 	},				\
	{ "ARMHC003",	CMN_S3		},				\
	{ "ARMHC701",	CMN_CI700	},				\
	{ "",		0		}				\
}

#define CCN_DEVICE_TREE_LIST						\
{									\
	{ .compatible = "arm,ccn-502",	.data = (void *) CCN_502 },	\
	{ .compatible = "arm,ccn-504",	.data = (void *) CCN_504 },	\
	{ .compatible = "arm,ccn-508",	.data = (void *) CCN_508 },	\
	{ .compatible = "arm,ccn-512",	.data = (void *) CCN_512 },	\
	{ /* EOL */ }							\
}

#define CCI_DEVICE_TREE_LIST						\
{									\
	{ .compatible = "arm,cci-400",	.data = (void *) CCI_400 },	\
	{ .compatible = "arm,cci-500",	.data = (void *) CCI_500 },	\
	{ .compatible = "arm,cci-550",	.data = (void *) CCI_550 },	\
	{ /* EOL */ }							\
}

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

#if defined(CONFIG_ACPI)
/* Kernel Source: drivers/acpi/bus.c					*/
#define ACPI_DT_NAMESPACE_HID	"PRP0001"

static bool Zacpi_of_match_device(struct acpi_device *adev,
				 const struct of_device_id *of_match_table,
				 const struct of_device_id **of_id)
{
	const union acpi_object *of_compatible, *obj;
	int i, nval;

	if (!adev)
		return false;

	of_compatible = adev->data.of_compatible;
	if (!of_match_table || !of_compatible)
		return false;

	if (of_compatible->type == ACPI_TYPE_PACKAGE) {
		nval = of_compatible->package.count;
		obj = of_compatible->package.elements;
	} else {
		nval = 1;
		obj = of_compatible;
	}
	for (i = 0; i < nval; i++, obj++) {
		const struct of_device_id *id;

		for (id = of_match_table; id->compatible[0]; id++)
			if (!strcasecmp(obj->string.pointer, id->compatible)) {
				if (of_id)
					*of_id = id;
				return true;
			}
	}
	return false;
}

static bool Z__acpi_match_device_cls(const struct acpi_device_id *id,
				    struct acpi_hardware_id *hwid)
{
	int i, msk, byte_shift;
	char buf[3];

	if (!id->cls)
		return false;

	for (i = 1; i <= 3; i++) {
		byte_shift = 8 * (3 - i);
		msk = (id->cls_msk >> byte_shift) & 0xFF;
		if (!msk)
			continue;

		sprintf(buf, "%02x", (id->cls >> byte_shift) & msk);
		if (strncmp(buf, &hwid->id[(i - 1) * 2], 2))
			return false;
	}
	return true;
}

static bool Z__acpi_match_device(struct acpi_device *device,
				const struct acpi_device_id *acpi_ids,
				const struct of_device_id *of_ids,
				const struct acpi_device_id **acpi_id,
				const struct of_device_id **of_id)
{
	const struct acpi_device_id *id;
	struct acpi_hardware_id *hwid;

	if (!device || !device->status.present)
		return false;

    list_for_each_entry(hwid, &device->pnp.ids, list) {
	if (acpi_ids) {
		for (id = acpi_ids; id->id[0] || id->cls; id++) {
			if (id->id[0] && !strcmp((char *)id->id, hwid->id))
				goto out_acpi_match;
			if (id->cls && Z__acpi_match_device_cls(id, hwid))
				goto out_acpi_match;
		}
	}
	if (!strcmp(ACPI_DT_NAMESPACE_HID, hwid->id))
		return Zacpi_of_match_device(device, of_ids, of_id);
    }
	return false;

out_acpi_match:
	if (acpi_id)
		*acpi_id = id;
	return true;
}

#undef ACPI_DT_NAMESPACE_HID
/* End of Kernel Source: drivers/acpi/bus.c				*/

static int ACPI_Match(	struct acpi_device *adev,
			const struct acpi_device_id *ids,
			const struct acpi_device_id **id )
{
	int rc = Z__acpi_match_device(adev, ids, NULL, id, NULL) ? 0 : -ENOENT;
	return rc;
}
#endif /* CONFIG_ACPI */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
	#define sys_reg(op0, op1, crn, crm, op2) ({	\
		UNUSED(op0);				\
		UNUSED(op1);				\
		UNUSED(crn);				\
		UNUSED(crm);				\
		UNUSED(op2);				\
	})
#endif

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
	"str	" #_reg1 ",	%1"					\
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
	"str	" #_reg2 ",	%2"					\
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
	"str	" #_reg3 ",	%3"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3)	\
	:								\
	: "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "cc", "memory"						\
);

#define RDTSC_COUNTERx1(mem_tsc, ...) \
ASM_COUNTERx1(x11, x12, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx2(mem_tsc, ...) \
ASM_COUNTERx2(x11, x12, x13, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx3(mem_tsc, ...) \
ASM_COUNTERx3(x11, x12, x13, x14, ASM_RDTSC, mem_tsc, __VA_ARGS__)

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
			PMCR		PMCR;
			PMSELR		PMSELR;
			PMXEVTYPER	PMTYPE[3];
			PMCCFILTR	PMCCFILTR;
			PMCNTENSET	PMCNTEN;
			PMUSERENR	PMUSER;
		    };
		} SaveArea;
#ifdef CONFIG_CPU_FREQ
		struct cpufreq_policy	FreqPolicy;
#endif /* CONFIG_CPU_FREQ */
#ifdef CONFIG_THERMAL
		struct thermal_zone_device *ThermalZone;
#endif /* CONFIG_THERMAL */
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
static void Query_DynamIQ(unsigned int cpu) ;
static void Query_CoherentMesh(unsigned int cpu) ;
static void Query_CacheCoherent(unsigned int cpu) ;
static void Query_DynamIQ_CMN(unsigned int cpu) ;
/*	[Void]								*/
#define _Void_Signature {.ExtFamily=0x00, .Family=0x0, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A320	{.ExtFamily=0xd8, .Family=0xf, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A34	{.ExtFamily=0xd0, .Family=0x2, .ExtModel=0x0, .Model=0x8}
#define _Cortex_A35	{.ExtFamily=0xd0, .Family=0x4, .ExtModel=0x0, .Model=0xa}
#define _Cortex_A510	{.ExtFamily=0xd4, .Family=0x6, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A520	{.ExtFamily=0xd8, .Family=0x0, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A520AE	{.ExtFamily=0xd8, .Family=0x8, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A53	{.ExtFamily=0xd0, .Family=0x3, .ExtModel=0x0, .Model=0x3}
#define _Cortex_A55	{.ExtFamily=0xd0, .Family=0x5, .ExtModel=0x4, .Model=0x5}
#define _Cortex_A57	{.ExtFamily=0xd0, .Family=0x7, .ExtModel=0x0, .Model=0x1}
#define _Cortex_A65	{.ExtFamily=0xd0, .Family=0x6, .ExtModel=0x4, .Model=0x6}
#define _Cortex_A65AE	{.ExtFamily=0xd4, .Family=0x3, .ExtModel=0x4, .Model=0x7}
#define _Cortex_A710	{.ExtFamily=0xd4, .Family=0x7, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A715	{.ExtFamily=0xd4, .Family=0xd, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A72	{.ExtFamily=0xd0, .Family=0x8, .ExtModel=0x0, .Model=0x2}
#define _Cortex_A720	{.ExtFamily=0xd8, .Family=0x1, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A720AE	{.ExtFamily=0xd8, .Family=0x9, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A725	{.ExtFamily=0xd8, .Family=0x7, .ExtModel=0x0, .Model=0x0}
#define _Cortex_A73	{.ExtFamily=0xd0, .Family=0x9, .ExtModel=0x0, .Model=0x4}
#define _Cortex_A75	{.ExtFamily=0xd0, .Family=0xa, .ExtModel=0x4, .Model=0xa}
#define _Cortex_A76	{.ExtFamily=0xd0, .Family=0xb, .ExtModel=0x0, .Model=0xb}
#define _Cortex_A76AE	{.ExtFamily=0xd0, .Family=0xe, .ExtModel=0x1, .Model=0x1}
#define _Cortex_A77	{.ExtFamily=0xd0, .Family=0xd, .ExtModel=0x1, .Model=0x0}
#define _Cortex_A78	{.ExtFamily=0xd4, .Family=0x1, .ExtModel=0x2, .Model=0x1}
#define _Cortex_A78AE	{.ExtFamily=0xd4, .Family=0x2, .ExtModel=0x2, .Model=0x2}
#define _Cortex_A78C	{.ExtFamily=0xd4, .Family=0xb, .ExtModel=0x2, .Model=0x4}
#define _Cortex_R82	{.ExtFamily=0xd1, .Family=0x5, .ExtModel=0x0, .Model=0x0}
#define _Cortex_R82AE	{.ExtFamily=0xd1, .Family=0x4, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X1	{.ExtFamily=0xd4, .Family=0x4, .ExtModel=0x2, .Model=0x3}
#define _Cortex_X1C	{.ExtFamily=0xd4, .Family=0xc, .ExtModel=0x2, .Model=0x5}
#define _Cortex_X2	{.ExtFamily=0xd4, .Family=0x8, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X3	{.ExtFamily=0xd4, .Family=0xe, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X4	{.ExtFamily=0xd8, .Family=0x2, .ExtModel=0x0, .Model=0x0}
#define _Cortex_X925	{.ExtFamily=0xd8, .Family=0x5, .ExtModel=0x0, .Model=0x0}
#define _DynamIQ_DSU	{.ExtFamily=0x00, .Family=0x0, .ExtModel=0x4, .Model=0x1}
#define _Neoverse_E1	{.ExtFamily=0xd4, .Family=0xa, .ExtModel=0x4, .Model=0x6}
#define _Neoverse_N1	{.ExtFamily=0xd0, .Family=0xc, .ExtModel=0x0, .Model=0xc}
#define _Neoverse_N2	{.ExtFamily=0xd4, .Family=0x9, .ExtModel=0x0, .Model=0x0}
#define _Neoverse_N3	{.ExtFamily=0xd8, .Family=0xe, .ExtModel=0x0, .Model=0x0}
#define _Neoverse_V1	{.ExtFamily=0xd4, .Family=0x0, .ExtModel=0x2, .Model=0x1}
#define _Neoverse_V2	{.ExtFamily=0xd4, .Family=0xf, .ExtModel=0x0, .Model=0x0}
#define _Neoverse_V3	{.ExtFamily=0xd8, .Family=0x4, .ExtModel=0x0, .Model=0x0}
#define _Neoverse_V3AE	{.ExtFamily=0xd8, .Family=0x3, .ExtModel=0x0, .Model=0x0}

typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static char *CodeName[CODENAMES] = {
	[    ARM64]	= "AArch64",
	[  ARMv8_R]	= "ARMv8-R",
	[  ARMv8_A]	= "ARMv8-A",
	[ARMv8_1_A]	= "ARMv8.1-A",
	[ARMv8_2_A]	= "ARMv8.2-A",
	[ARMv8_3_A]	= "ARMv8.3-A",
	[ARMv8_4_A]	= "ARMv8.4-A",
	[ARMv8_5_A]	= "ARMv8.5-A",
	[ARMv8_6_A]	= "ARMv8.6-A",
	[ARMv8_7_A]	= "ARMv8.7-A",
	[ARMv8_8_A]	= "ARMv8.8-A",
	[  ARMv9_A]	= "ARMv9-A",
	[ARMv9_1_A]	= "ARMv9.1-A",
	[ARMv9_2_A]	= "ARMv9.2-A",
	[ARMv9_3_A]	= "ARMv9.3-A",
	[ARMv9_4_A]	= "ARMv9.4-A",
	[  ARMv9_5]	= "ARMv9.5"
};

#define Arch_Misc_Processor {.Brand = ZLIST(NULL), .CN = ARM64}
#define Arch_Cortex_A320 {.Brand = ZLIST("Cortex-A320"), .CN = ARMv9_2_A}
#define Arch_Cortex_A34 {.Brand = ZLIST("Cortex-A34"), .CN = ARMv8_A}
#define Arch_Cortex_A35 {.Brand = ZLIST("Cortex-A35"), .CN = ARMv8_A}
#define Arch_Cortex_A510 {.Brand = ZLIST("Cortex-A510"), .CN = ARMv9_A}
#define Arch_Cortex_A520 {.Brand = ZLIST("Cortex-A520"), .CN = ARMv9_A}
#define Arch_Cortex_A520AE {.Brand = ZLIST("Cortex-A520AE"), .CN = ARMv9_2_A}
#define Arch_Cortex_A53 {.Brand = ZLIST("Cortex-A53"), .CN = ARMv8_A}
#define Arch_Cortex_A55 {.Brand = ZLIST("Cortex-A55"), .CN = ARMv8_2_A}
#define Arch_Cortex_A57 {.Brand = ZLIST("Cortex-A57"), .CN = ARMv8_A}
#define Arch_Cortex_A65 {.Brand = ZLIST("Cortex-A65"), .CN = ARMv8_2_A}
#define Arch_Cortex_A65AE {.Brand = ZLIST("Cortex-A65AE"), .CN = ARMv8_2_A}
#define Arch_Cortex_A710 {.Brand = ZLIST("Cortex-A710"), .CN = ARMv9_A}
#define Arch_Cortex_A715 {.Brand = ZLIST("Cortex-A715"), .CN = ARMv9_A}
#define Arch_Cortex_A72 {.Brand = ZLIST("Cortex-A72"), .CN = ARMv8_A}
#define Arch_Cortex_A720 {.Brand = ZLIST("Cortex-A720"), .CN = ARMv9_A}
#define Arch_Cortex_A720AE {.Brand = ZLIST("Cortex-A720AE"), .CN = ARMv9_2_A}
#define Arch_Cortex_A725 {.Brand = ZLIST("Cortex-A725"), .CN = ARMv9_2_A}
#define Arch_Cortex_A73 {.Brand = ZLIST("Cortex-A73"), .CN = ARMv8_A}
#define Arch_Cortex_A75 {.Brand = ZLIST("Cortex-A75"), .CN = ARMv8_2_A}
#define Arch_Cortex_A76 {.Brand = ZLIST("Cortex-A76"), .CN = ARMv8_2_A}
#define Arch_Cortex_A76AE {.Brand = ZLIST("Cortex-A76AE"), .CN = ARMv8_2_A}
#define Arch_Cortex_A77 {.Brand = ZLIST("Cortex-A77"), .CN = ARMv8_2_A}
#define Arch_Cortex_A78 {.Brand = ZLIST("Cortex-A78"), .CN = ARMv8_2_A}
#define Arch_Cortex_A78AE {.Brand = ZLIST("Cortex-A78AE"), .CN = ARMv8_2_A}
#define Arch_Cortex_A78C {.Brand = ZLIST("Cortex-A78C"), .CN = ARMv8_2_A}
#define Arch_Cortex_R82 {.Brand = ZLIST("Cortex-R82"), .CN = ARMv8_R}
#define Arch_Cortex_R82AE {.Brand = ZLIST("Cortex-R82AE"), .CN = ARMv8_R}
#define Arch_Cortex_X1 {.Brand = ZLIST("Cortex-X1"), .CN = ARMv8_2_A}
#define Arch_Cortex_X1C {.Brand = ZLIST("Cortex-X1C"), .CN = ARMv8_2_A}
#define Arch_Cortex_X2 {.Brand = ZLIST("Cortex-X2"), .CN = ARMv9_A}
#define Arch_Cortex_X3 {.Brand = ZLIST("Cortex-X3"), .CN = ARMv9_A}
#define Arch_Cortex_X4 {.Brand = ZLIST("Cortex-X4"), .CN = ARMv9_2_A}
#define Arch_Cortex_X925 {.Brand = ZLIST("Cortex-X925"), .CN = ARMv9_2_A}
#define Arch_DynamIQ_DSU {.Brand = ZLIST("DynamIQ DSU"), .CN = ARMv8_2_A}
#define Arch_Neoverse_E1 {.Brand = ZLIST("Neoverse E1"), .CN = ARMv8_2_A}
#define Arch_Neoverse_N1 {.Brand = ZLIST("Neoverse N1"), .CN = ARMv8_2_A}
#define Arch_Neoverse_N2 {.Brand = ZLIST("Neoverse N2"), .CN = ARMv9_A}
#define Arch_Neoverse_N3 {.Brand = ZLIST("Neoverse N3"), .CN = ARMv9_2_A}
#define Arch_Neoverse_V1 {.Brand = ZLIST("Neoverse V1"), .CN = ARMv8_4_A}
#define Arch_Neoverse_V2 {.Brand = ZLIST("Neoverse V2"), .CN = ARMv9_A}
#define Arch_Neoverse_V3 {.Brand = ZLIST("Neoverse V3"), .CN = ARMv9_2_A}
#define Arch_Neoverse_V3AE {.Brand = ZLIST("Neoverse V3AE"), .CN = ARMv9_2_A}

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
	.Architecture = Arch_Misc_Processor
	},
[Cortex_A320] = {
	.Signature = _Cortex_A320,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A320
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
	.Architecture = Arch_Cortex_A34
	},
[Cortex_A35] = {
	.Signature = _Cortex_A35,
	.Query = Query_CacheCoherent,
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
	.Architecture = Arch_Cortex_A35
	},
[Cortex_A510] = {
	.Signature = _Cortex_A510,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A510
	},
[Cortex_A520] = {
	.Signature = _Cortex_A520,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A520
	},
[Cortex_A520AE] = {
	.Signature = _Cortex_A520AE,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A520AE
	},
[Cortex_A53] = {
	.Signature = _Cortex_A53,
	.Query = Query_CacheCoherent,
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
	.Architecture = Arch_Cortex_A53
	},
[Cortex_A55] = {
	.Signature = _Cortex_A55,
	.Query = Query_DynamIQ,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_CELSIUS,
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
	.Architecture = Arch_Cortex_A55
	},
[Cortex_A57] = {
	.Signature = _Cortex_A57,
	.Query = Query_CacheCoherent,
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
	.Architecture = Arch_Cortex_A57
	},
[Cortex_A65] = {
	.Signature = _Cortex_A65,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A65
	},
[Cortex_A65AE] = {
	.Signature = _Cortex_A65AE,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A65AE
	},
[Cortex_A710] = {
	.Signature = _Cortex_A710,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A710
	},
[Cortex_A715] = {
	.Signature = _Cortex_A715,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A715
	},
[Cortex_A72] = {
	.Signature = _Cortex_A72,
	.Query = Query_CacheCoherent,
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
	.Architecture = Arch_Cortex_A72
	},
[Cortex_A720] = {
	.Signature = _Cortex_A720,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A720
	},
[Cortex_A720AE] = {
	.Signature = _Cortex_A720AE,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A720AE
	},
[Cortex_A725] = {
	.Signature = _Cortex_A725,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A725
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
	.Architecture = Arch_Cortex_A73
	},
[Cortex_A75] = {
	.Signature = _Cortex_A75,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A75
	},
[Cortex_A76] = {
	.Signature = _Cortex_A76,
	.Query = Query_DynamIQ,
	.Update = PerCore_GenericMachine,
	.Start = Start_GenericMachine,
	.Stop = Stop_GenericMachine,
	.Exit = NULL,
	.Timer = InitTimer_GenericMachine,
	.BaseClock = BaseClock_GenericMachine,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_CELSIUS,
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
	.Architecture = Arch_Cortex_A76
	},
[Cortex_A76AE] = {
	.Signature = _Cortex_A76AE,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A76AE
	},
[Cortex_A77] = {
	.Signature = _Cortex_A77,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A77
	},
[Cortex_A78] = {
	.Signature = _Cortex_A78,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A78
	},
[Cortex_A78AE] = {
	.Signature = _Cortex_A78AE,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A78AE
	},
[Cortex_A78C] = {
	.Signature = _Cortex_A78C,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_A78C
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
	.Architecture = Arch_Cortex_R82
	},
[Cortex_R82AE] = {
	.Signature = _Cortex_R82AE,
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
	.Architecture = Arch_Cortex_R82AE
	},
[Cortex_X1] = {
	.Signature = _Cortex_X1,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_X1
	},
[Cortex_X1C] = {
	.Signature = _Cortex_X1C,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_X1C
	},
[Cortex_X2] = {
	.Signature = _Cortex_X2,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_X2
	},
[Cortex_X3] = {
	.Signature = _Cortex_X3,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_X3
	},
[Cortex_X4] = {
	.Signature = _Cortex_X4,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_X4
	},
[Cortex_X925] = {
	.Signature = _Cortex_X925,
	.Query = Query_DynamIQ,
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
	.Architecture = Arch_Cortex_X925
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
	.Architecture = Arch_DynamIQ_DSU
	},
[Neoverse_E1] = {
	.Signature = _Neoverse_E1,
	.Query = Query_CoherentMesh,
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
	.Architecture = Arch_Neoverse_E1
	},
[Neoverse_N1] = {
	.Signature = _Neoverse_N1,
	.Query = Query_CoherentMesh,
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
	.Architecture = Arch_Neoverse_N1
	},
[Neoverse_N2] = {
	.Signature = _Neoverse_N2,
	.Query = Query_CoherentMesh,
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
	.Architecture = Arch_Neoverse_N2
	},
[Neoverse_N3] = {
	.Signature = _Neoverse_N3,
	.Query = Query_DynamIQ_CMN,
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
	.Architecture = Arch_Neoverse_N3
	},
[Neoverse_V1] = {
	.Signature = _Neoverse_V1,
	.Query = Query_DynamIQ_CMN,
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
	.Architecture = Arch_Neoverse_V1
	},
[Neoverse_V2] = {
	.Signature = _Neoverse_V2,
	.Query = Query_CoherentMesh,
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
	.Architecture = Arch_Neoverse_V2
	},
[Neoverse_V3] = {
	.Signature = _Neoverse_V3,
	.Query = Query_DynamIQ_CMN,
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
	.Architecture = Arch_Neoverse_V3
	},
[Neoverse_V3AE] = {
	.Signature = _Neoverse_V3AE,
	.Query = Query_DynamIQ_CMN,
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
	.Architecture = Arch_Neoverse_V3AE
	}
};
