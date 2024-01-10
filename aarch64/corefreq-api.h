/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define DRV_DEVNAME	"corefreqk"
#define DRV_FILENAME	"/dev/"DRV_DEVNAME

#define ID_RO_VMA_PROC	(CORE_COUNT + 0)
#define ID_RW_VMA_PROC	(CORE_COUNT + ID_RO_VMA_PROC)
#define ID_RO_VMA_GATE	(CORE_COUNT + ID_RW_VMA_PROC)
#define ID_RO_VMA_CORE	(CORE_COUNT + ID_RO_VMA_GATE)
#define ID_RW_VMA_CORE	(CORE_COUNT + ID_RO_VMA_CORE)
#define ID_ANY_VMA_JAIL (CORE_COUNT + ID_RW_VMA_CORE)

#define WAKEUP_RATIO	4
#define LOOP_MIN_MS	100
#define LOOP_MAX_MS	((1000 - 1) * WAKEUP_RATIO)
#define LOOP_DEF_MS	1000
#define TICK_DEF_MS	2000

typedef struct
{
	struct
	{
		unsigned char Chr[4];
	} AX, BX, CX, DX;
} BRAND;

#define LEVEL_INVALID	0
#define LEVEL_THREAD	1
#define LEVEL_CORE	2

typedef struct {
	union {
		struct
		{
			unsigned int
			SHRbits :  5-0,
			Unused1 : 32-5;
		};
		unsigned int Register;
	} AX;
	union {
		struct
		{
			unsigned int
			Threads : 16-0,
			Unused1 : 32-16;
		};
		unsigned int Register;
	} BX;
	union {
		struct
		{
			unsigned int
			Level	:  8-0,
			Type	: 16-8,
			Unused1 : 32-16;
		};
		unsigned int Register;
	} CX;
	union {
		struct
		{
			unsigned int
			x2ApicID: 32-0;
		};
		unsigned int Register;
	} DX;
} CPUID_TOPOLOGY_LEAF;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		ReservedBits1	:  8-0,
		BSP		:  9-8,
		ReservedBits2	: 10-9,
		x2APIC_EN	: 11-10,
		APIC_EN 	: 12-11,
		Addr		: 64-12;
	};
} LOCAL_APIC;

typedef struct
{
	LOCAL_APIC		Base;
	signed int		ApicID,
				CoreID,
				ThreadID,
				PackageID;
	union {
		unsigned int	ID;
	    struct {
		unsigned int	Node:  8-0,
				CCX : 16-8,
				CCD : 24-16,
				CMP : 32-24;
	    };
	    CPUID_0x0000001a	Hybrid;
	} Cluster;

	struct CACHE_INFO
	{
		union CCSIDR
		{
			unsigned long long	value;
			struct
			{
				unsigned long long
				LineSz		:  3-0,
				Assoc		: 13-3,
				Set		: 28-13,
				WrAlloc 	: 29-28,
				RdAlloc 	: 30-29,
				WrBack		: 31-30,
				WrThrough	: 32-31,
				RES0		: 64-32;
			};
		} ccsid;
		unsigned int	Size;
	} Cache[CACHE_MAX_LEVEL];
} CACHE_TOPOLOGY;

typedef struct
{
	THERMAL_PARAM			Param;
	unsigned int			Sensor;
	signed int			VID;
	struct {
		enum THERM_PWR_EVENTS	Events[eDIM];
	};
	HWP_CAPABILITIES		HWP_Capabilities;
	HWP_INTERRUPT			HWP_Interrupt;
	HWP_REQUEST			HWP_Request;
	struct ACPI_CPPC_STRUCT {
		unsigned short		Highest,
					Guaranteed,
					Efficient,
					Lowest,
					Minimum,
					Maximum,
					Desired,
					Energy;
	} ACPI_CPPC;
} POWER_THERMAL;

/*				ACPI/OSPM

	Highest Performance ... .------,
				|######|
				|######|
	Nominal Performance ... |------|  ..
				|//////|    \
				|//////|    :
				|//////|    :
				|//////|    :
				|//////|    --- Guaranteed Performance
Lowest Nonlinear Performance .. --------    :	Allowed Range
				|OOOOOO|    :
				|OOOOOO|    :
				|OOOOOO|    :
	Lowest Performance .... --------  ../
				|++++++|
				|++++++|
				|++++++|
			0 ..... `------'
*/

typedef struct
{
	Bit64				OffLine __attribute__ ((aligned (8)));

	struct
	{
		unsigned long long	TSC;
	} Overhead __attribute__ ((aligned (8)));

	struct
	{
		unsigned long long	INST;
		struct
		{
		unsigned long long
				UCC,
				URC;
		}			C0;
		unsigned long long
					C3,
					C6,
					C7,
					TSC;

		unsigned long long	C1;

		struct
		{
		unsigned long long	ACCU;
		} Power;
	} Counter[2] __attribute__ ((aligned (8)));

	struct
	{
	unsigned long long
				C1,
				C2,
				C3,
				C4,
				C5,
				C6,
				C7;
	} VPMC __attribute__ ((aligned (8)));

	struct
	{
		unsigned long long
					INST;
		struct
		{
		unsigned long long
				UCC,
				URC;
		}			C0;
		unsigned long long
					C3,
					C6,
					C7,
					TSC,
					C1;

		struct
		{
		unsigned long long	ACCU;
		} Power;

		unsigned int		SMI;
	} Delta __attribute__ ((aligned (8)));

	POWER_THERMAL			PowerThermal;
	THERMAL_POINT			ThermalPoint;

	struct
	{
		unsigned int		SMI;
		struct {
			unsigned int
					LOCAL,
					UNKNOWN,
					PCISERR,
					IOCHECK;
		}			NMI;
	} Interrupt;

	struct
	{
		CPUID_0x00000000	StdFunc;
		CPUID_0x80000000	ExtFunc;

		struct {
		unsigned long long
					CfgLock :  1-0,  /* Core	*/
					IORedir :  2-1,  /* Core	*/
					Unused	: 32-3,
					Microcode:64-32; /* Thread	*/
		};
		unsigned short int	CStateLimit;
		struct {
		unsigned short int	CStateInclude;	/* Intel	*/
		unsigned short int	CStateBaseAddr; /* Any I/O BAR	*/
		};
	} Query;

	CACHE_TOPOLOGY			T;

	struct {
		Bit64			FLAGS	__attribute__ ((aligned (8)));
	} SystemRegister;

	unsigned int			Bind;

	CLOCK				Clock;

	unsigned int			Boost[BOOST(SIZE)];

	COF_UNION			Ratio;
} CORE_RO;

typedef struct
{
	struct	/* 64-byte cache line size.				*/
	{
		Bit64			V,
					_pad[7];
	} Sync __attribute__ ((aligned (8)));

} CORE_RW;

typedef struct
{
	struct {
		union {
		} DIMM[MC_MAX_DIMM];
	} Channel[MC_MAX_CHA];

	union {
	} MaxDIMMs;

	unsigned short		SlotCount, ChannelCount;
} MC_REGISTERS;

typedef struct
{
} BUS_REGISTERS;


typedef struct {
		int		taskCount;
		TASK_MCB	taskList[TASK_LIMIT];

		unsigned int	kernelVersionNumber;

		char		sysname[MAX_UTS_LEN],
				release[MAX_UTS_LEN],
				version[MAX_UTS_LEN],
				machine[MAX_UTS_LEN];
} SYSGATE_RO; /*		RO Pages				*/

#define CHIP_MAX_PCI	24

typedef struct
{
	struct
	{
	    unsigned long long	PCLK; /* Contextual Clock [TSC|UCLK|MCLK] */
	  union {
	    struct {
	    unsigned long long	PC02,
				PC03,
				PC04,
				PC06,
				PC07,
				PC08,
				PC09,
				PC10;
	    };
	    #if defined(ARCH_PMC)
		unsigned long long
			CTR[MC_VECTOR_TO_SCALAR(MC_MAX_CTRL, MC_MAX_CHA)];
	    #endif
	  };
	  union {
	    unsigned long long	MC6;
	    unsigned long long	FCLK;
	  };
	  struct {
	    unsigned long long	FC0; /* Uncore fixed counter #0 	*/
	  } Uncore;

	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Counter[2] __attribute__ ((aligned (8)));

	struct
	{
	    unsigned long long	PCLK;
	  union {
	    struct {
	    unsigned long long	PC02,
				PC03,
				PC04,
				PC06,
				PC07,
				PC08,
				PC09,
				PC10;
	    };
	    #if defined(ARCH_PMC)
		unsigned long long
			CTR[MC_VECTOR_TO_SCALAR(MC_MAX_CTRL, MC_MAX_CHA)];
	    #endif
	  };
	  union {
	    unsigned long long	MC6;
	    unsigned long long	FCLK;
	  };
	  struct {
	    unsigned long long	FC0;
	  } Uncore;
	} Delta __attribute__ ((aligned (8)));

	FEATURES		Features;

	BitCC			CR_Mask 	__attribute__ ((aligned (16)));
	BitCC			TurboBoost_Mask __attribute__ ((aligned (16)));
	BitCC			HWP_Mask __attribute__ ((aligned (16)));
	BitCC			SPEC_CTRL_Mask	__attribute__ ((aligned (16)));

	enum THERMAL_FORMULAS	thermalFormula;
	enum VOLTAGE_FORMULAS	voltageFormula;
	enum POWER_FORMULAS	powerFormula;

	unsigned int		SleepInterval,
				tickReset,
				tickStep;

	struct {
		unsigned int	Count,
				OnLine;
	} CPU;

	SERVICE_PROC		Service;

	signed int		ArchID;

	struct {
		unsigned int	Boost[UNCORE_BOOST(SIZE)];
		BUS_REGISTERS	Bus;
		MC_REGISTERS	MC[MC_MAX_CTRL];
		unsigned short	CtrlCount;
	    struct CHIP_ST {
		unsigned short	VID, DID;
	    } Chip[CHIP_MAX_PCI];
	} Uncore;

	struct {
		THERMAL_PARAM	Param;
		unsigned int	Sensor;
	    struct {
		signed int	CPU, SOC;
	    } VID;

	enum THERM_PWR_EVENTS	Events[eDIM];

	    struct {
		unsigned short		Minimum,
					Maximum;
	    } ACPI_CPPC;
	} PowerThermal;

	THERMAL_POINT		ThermalPoint;

	struct {
		struct {
			size_t	Size;
			int	Order;
		} ReqMem;
	} Gate;

	OS_DRIVER		OS;

	struct {
		Bit64		NMI;
		signed int	AutoClock,
				Experimental,
				HotPlug,
				PCI;
		KERNEL_DRIVER	Driver;
	} Registration;

	enum HYPERVISOR 	HypervisorID;
	char			Architecture[CODENAME_LEN];

	SMBIOS_ST		SMB;

	FOOTPRINT		FootPrint;
} PROC_RO; /*			RO Pages				*/

typedef struct
{
	struct
	{
	  struct {
	    unsigned long long	ACCU[PWR_DOMAIN(SIZE)];
	  } Power;
	} Delta __attribute__ ((aligned (8)));

	BitCC			TurboBoost	__attribute__ ((aligned (16)));
	BitCC			HWP		__attribute__ ((aligned (16)));
	BitCC			VM		__attribute__ ((aligned (16)));
	BitCC			SSBD		__attribute__ ((aligned (16)));
	BitCC			SSBS		__attribute__ ((aligned (16)));
	struct {
		Bit64		Signal	__attribute__ ((aligned (8)));
	} OS;
} PROC_RW; /*			RW Pages				*/
