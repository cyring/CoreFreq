/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define RDCOUNTER(_val, _cnt)						\
({									\
	unsigned int _lo, _hi;						\
									\
	asm volatile							\
	(								\
		"rdmsr"							\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_cnt)						\
	);								\
	_val=_lo | ((unsigned long long) _hi << 32);			\
})

#define WRCOUNTER(_val, _cnt)						\
	asm volatile							\
	(								\
		"wrmsr"							\
		:							\
		: "c" (_cnt),						\
		  "a" ((unsigned int) _val & 0xFFFFFFFF),		\
		  "d" ((unsigned int) (_val >> 32))			\
	);

#define RDMSR(_data, _reg)						\
({									\
	unsigned int _lo, _hi;						\
									\
	asm volatile							\
	(								\
		"rdmsr"							\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_reg)						\
	);								\
	_data.value=_lo | ((unsigned long long) _hi << 32);		\
})

#define WRMSR(_data, _reg)						\
	asm volatile							\
	(								\
		"wrmsr"							\
		:							\
		: "c" (_reg),						\
		  "a" ((unsigned int) _data.value & 0xFFFFFFFF),	\
		  "d" ((unsigned int) (_data.value >> 32))		\
	);

#define ASM_CODE_RDMSR(_msr, _reg) \
	"# Read MSR counter."		"\n\t"		\
	"movq	$" #_msr ", %%rcx"	"\n\t"		\
	"rdmsr"				"\n\t"		\
	"shlq	$32, %%rdx"		"\n\t"		\
	"orq	%%rdx, %%rax"		"\n\t"		\
	"# Save counter value."		"\n\t"		\
	"movq	%%rax, %%" #_reg	"\n\t"

#define ASM_RDMSR(_msr, _reg) ASM_CODE_RDMSR(_msr, _reg)


#define ASM_COUNTERx1(	_reg0, _reg1,			\
			_tsc_inst, mem_tsc,		\
			_msr1, _mem1)			\
asm volatile						\
(							\
	_tsc_inst(_reg0)				\
	ASM_RDMSR(_msr1, _reg1)				\
	"# Store values into memory."	"\n\t"		\
	"movq	%%" #_reg0 ", %0"	"\n\t"		\
	"movq	%%" #_reg1 ", %1"			\
	: "=m" (mem_tsc), "=m" (_mem1)			\
	:						\
	: "%rax", "%rcx", "%rdx",			\
	  "%" #_reg0"", "%" #_reg1"",			\
	  "memory"					\
);


#define ASM_COUNTERx2(	_reg0, _reg1, _reg2,		\
			_tsc_inst, mem_tsc,		\
			_msr1, _mem1, _msr2, _mem2)	\
asm volatile						\
(							\
	_tsc_inst(_reg0)				\
	ASM_RDMSR(_msr1, _reg1)				\
	ASM_RDMSR(_msr2, _reg2)				\
	"# Store values into memory."	"\n\t"		\
	"movq	%%" #_reg0 ", %0"	"\n\t"		\
	"movq	%%" #_reg1 ", %1"	"\n\t"		\
	"movq	%%" #_reg2 ", %2"			\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2)	\
	:						\
	: "%rax", "%rcx", "%rdx",			\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"",	\
	  "memory"					\
);


#define ASM_COUNTERx3(	_reg0, _reg1, _reg2, _reg3,			\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3)	\
asm volatile								\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %3"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3)	\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "memory"							\
);


#define ASM_COUNTERx4(	_reg0, _reg1, _reg2, _reg3, _reg4,		\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4)					\
asm volatile								\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	ASM_RDMSR(_msr4, _reg4)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %3"	"\n\t"				\
	"movq	%%" #_reg4 ", %4"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4)							\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "%" #_reg4"",							\
	  "memory"							\
);


#define ASM_COUNTERx5(	_reg0, _reg1, _reg2, _reg3, _reg4, _reg5,	\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5)			\
asm volatile								\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	ASM_RDMSR(_msr4, _reg4)						\
	ASM_RDMSR(_msr5, _reg5)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %3"	"\n\t"				\
	"movq	%%" #_reg4 ", %4"	"\n\t"				\
	"movq	%%" #_reg5 ", %5"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4), "=m" (_mem5)					\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "%" #_reg4"", "%" #_reg5"",					\
	  "memory"							\
);


#define ASM_COUNTERx6(	_reg0, _reg1, _reg2, _reg3, _reg4, _reg5, _reg6,\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6)	\
asm volatile								\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	ASM_RDMSR(_msr4, _reg4)						\
	ASM_RDMSR(_msr5, _reg5)						\
	ASM_RDMSR(_msr6, _reg6)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %3"	"\n\t"				\
	"movq	%%" #_reg4 ", %4"	"\n\t"				\
	"movq	%%" #_reg5 ", %5"	"\n\t"				\
	"movq	%%" #_reg6 ", %6"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4), "=m" (_mem5), "=m" (_mem6)			\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "%" #_reg4"", "%" #_reg5"", "%" #_reg6"",			\
	  "memory"							\
);


#define ASM_COUNTERx7(	_reg0, _reg1, _reg2, _reg3,			\
			_reg4, _reg5, _reg6, _reg7,			\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6,	\
			_msr7, _mem7)					\
asm volatile								\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	ASM_RDMSR(_msr4, _reg4)						\
	ASM_RDMSR(_msr5, _reg5)						\
	ASM_RDMSR(_msr6, _reg6)						\
	ASM_RDMSR(_msr7, _reg7)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %3"	"\n\t"				\
	"movq	%%" #_reg4 ", %4"	"\n\t"				\
	"movq	%%" #_reg5 ", %5"	"\n\t"				\
	"movq	%%" #_reg6 ", %6"	"\n\t"				\
	"movq	%%" #_reg7 ", %7"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4), "=m" (_mem5), "=m" (_mem6), "=m" (_mem7)	\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "%" #_reg4"", "%" #_reg5"", "%" #_reg6"", "%" #_reg7"",	\
	  "memory" \
);


#define RDTSC_COUNTERx1(mem_tsc, ...) \
ASM_COUNTERx1(r10, r11, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx1(mem_tsc, ...) \
ASM_COUNTERx1(r10, r11, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx2(mem_tsc, ...) \
ASM_COUNTERx2(r10, r11, r12, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx2(mem_tsc, ...) \
ASM_COUNTERx2(r10, r11, r12, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx3(mem_tsc, ...) \
ASM_COUNTERx3(r10, r11, r12, r13, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx3(mem_tsc, ...) \
ASM_COUNTERx3(r10, r11, r12, r13, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx4(mem_tsc, ...) \
ASM_COUNTERx4(r10, r11, r12, r13, r14, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx4(mem_tsc, ...) \
ASM_COUNTERx4(r10, r11, r12, r13, r14, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx5(mem_tsc, ...) \
ASM_COUNTERx5(r10, r11, r12, r13, r14, r15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx5(mem_tsc, ...) \
ASM_COUNTERx5(r10, r11, r12, r13, r14, r15, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx6(mem_tsc, ...) \
ASM_COUNTERx6(r10, r11, r12, r13, r14, r15, r9, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx6(mem_tsc, ...) \
ASM_COUNTERx6(r10, r11, r12, r13, r14, r15, r9, ASM_RDTSCP, mem_tsc,__VA_ARGS__)

#define RDTSC_COUNTERx7(mem_tsc, ...) \
ASM_COUNTERx7(r10, r11, r12, r13, r14, r15,r9,r8,ASM_RDTSC,mem_tsc,__VA_ARGS__)

#define RDTSCP_COUNTERx7(mem_tsc, ...) \
ASM_COUNTERx7(r10, r11, r12, r13, r14, r15,r9,r8,ASM_RDTSCP,mem_tsc,__VA_ARGS__)


#define PCI_CONFIG_ADDRESS(bus, dev, fn, reg) \
	(0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3))

#define RDPCI(_data, _reg)						\
({									\
	asm volatile							\
	(								\
		"movl	%1,	%%eax"	"\n\t"				\
		"movl	$0xcf8,	%%edx"	"\n\t"				\
		"outl	%%eax,	%%dx"	"\n\t"				\
		"movl	$0xcfc,	%%edx"	"\n\t"				\
		"inl	%%dx,	%%eax"	"\n\t"				\
		"movl	%%eax,	%0"					\
		: "=m"	(_data)						\
		: "ir"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define WRPCI(_data, _reg)						\
({									\
	asm volatile							\
	(								\
		"movl	%1,	%%eax"	"\n\t"				\
		"movl	$0xcf8,	%%edx"	"\n\t"				\
		"outl	%%eax,	%%dx"	"\n\t"				\
		"movl	%0,	%%eax"	"\n\t"				\
		"movl	$0xcfc,	%%edx"	"\n\t"				\
		"outl	%%eax,	%%dx"					\
		:							\
		: "m"	(_data),					\
		  "ir"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

typedef struct {
	FEATURES	Features;
	unsigned int	SMT_Count,
			localProcessor;
	signed int	rc;
} INIT_ARG;

typedef struct {			// V[0] stores previous TSC
	unsigned long long V[2];	// V[1] stores current TSC
} TSC_STRUCT;

#define OCCURRENCES 4
// OCCURRENCES x 2 (TSC values) needs a 64-byte cache line size.
#define STRUCT_SIZE (OCCURRENCES * sizeof(TSC_STRUCT))

typedef struct {
	TSC_STRUCT	*TSC[2];
	CLOCK		Clock;
} COMPUTE_ARG;

typedef struct
{
	struct kmem_cache	*Cache;
	CORE			*Core[];
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
	Bit64			TSM __attribute__ ((aligned (64)));
} JOIN;

typedef struct
{
	struct kmem_cache	*Cache;
	JOIN			*Join[];
} KPRIVATE;


typedef	struct
{
	struct	SIGNATURE	Signature;
	void			(*Query)(void);
	void			(*Update)(void *arg);	// Must be static
	void			(*Start)(void *arg);	// Must be static
	void			(*Stop)(void *arg);	// Must be static
	void			(*Exit)(void);
	void			(*Timer)(unsigned int cpu);
	CLOCK			(*Clock)(unsigned int ratio);
	char			*Architecture;
	unsigned long long	thermalFormula,
				voltageFormula,
				powerFormula;
	struct pci_device_id	*PCI_ids;
	struct {
		void		(*Start)(void *arg);	// Must be static
		void		(*Stop)(void *arg);	// Must be static
	} Uncore;
} ARCH;

extern CLOCK Clock_GenuineIntel(unsigned int ratio) ;
extern CLOCK Clock_AuthenticAMD(unsigned int ratio) ;
extern CLOCK Clock_Core(unsigned int ratio) ;
extern CLOCK Clock_Core2(unsigned int ratio) ;
extern CLOCK Clock_Atom(unsigned int ratio) ;
extern CLOCK Clock_Airmont(unsigned int ratio) ;
extern CLOCK Clock_Silvermont(unsigned int ratio) ;
extern CLOCK Clock_Nehalem(unsigned int ratio) ;
extern CLOCK Clock_Westmere(unsigned int ratio) ;
extern CLOCK Clock_SandyBridge(unsigned int ratio) ;
extern CLOCK Clock_IvyBridge(unsigned int ratio) ;
extern CLOCK Clock_Haswell(unsigned int ratio) ;
extern CLOCK Clock_Skylake(unsigned int ratio) ;
extern CLOCK Clock_AMD_Family_17h(unsigned int ratio) ;

extern void Query_GenuineIntel(void) ;
static void PerCore_Intel_Query(void *arg) ;
static void Start_GenuineIntel(void *arg) ;
static void Stop_GenuineIntel(void *arg) ;
extern void InitTimer_GenuineIntel(unsigned int cpu) ;

extern void Query_AuthenticAMD(void) ;
static void PerCore_AuthenticAMD_Query(void *arg) ;
static void Start_AuthenticAMD(void *arg) ;
static void Stop_AuthenticAMD(void *arg) ;
extern void InitTimer_AuthenticAMD(unsigned int cpu) ;

extern void Query_Core2(void) ;
static void PerCore_Core2_Query(void *arg) ;
static void Start_Core2(void *arg) ;
static void Stop_Core2(void *arg) ;
extern void InitTimer_Core2(unsigned int cpu) ;

extern void Query_Nehalem(void) ;
static void PerCore_Nehalem_Query(void *arg) ;
static void Start_Nehalem(void *arg) ;
static void Stop_Nehalem(void *arg) ;
extern void InitTimer_Nehalem(unsigned int cpu) ;
static void Start_Uncore_Nehalem(void *arg) ;
static void Stop_Uncore_Nehalem(void *arg) ;

extern void Query_SandyBridge(void) ;
static void PerCore_SandyBridge_Query(void *arg) ;
static void Start_SandyBridge(void *arg) ;
static void Stop_SandyBridge(void *arg) ;
extern void InitTimer_SandyBridge(unsigned int cpu) ;
static void Start_Uncore_SandyBridge(void *arg) ;
static void Stop_Uncore_SandyBridge(void *arg) ;

#define     PerCore_SandyBridge_EP_Query PerCore_SandyBridge_Query
static void Start_SandyBridge_EP(void *arg) ;
static void Stop_SandyBridge_EP(void *arg) ;
extern void InitTimer_SandyBridge_EP(unsigned int cpu) ;
#define     Start_Uncore_SandyBridge_EP Start_Uncore_SandyBridge
#define     Stop_Uncore_SandyBridge_EP Stop_Uncore_SandyBridge

extern void Query_IvyBridge(void) ;
#define     PerCore_IvyBridge_Query PerCore_SandyBridge_Query
extern void Query_IvyBridge_EP(void) ;
#define     PerCore_IvyBridge_EP_Query PerCore_SandyBridge_EP_Query

extern void Query_Haswell_EP(void) ;

static void PerCore_Haswell_ULT_Query(void *arg) ;
static void Start_Haswell_ULT(void *arg) ;
static void Stop_Haswell_ULT(void *arg) ;
extern void InitTimer_Haswell_ULT(unsigned int cpu) ;
static void Start_Uncore_Haswell_ULT(void *arg) ;
static void Stop_Uncore_Haswell_ULT(void *arg) ;

extern void Query_Broadwell(void) ;
#define     PerCore_Broadwell_Query PerCore_SandyBridge_Query
#define     Start_Broadwell Start_SandyBridge
#define     Stop_Broadwell Stop_SandyBridge
#define     InitTimer_Broadwell InitTimer_SandyBridge
#define     Start_Uncore_Broadwell Start_Uncore_SandyBridge
#define     Stop_Uncore_Broadwell Stop_Uncore_SandyBridge

#define     PerCore_Broadwell_EP_Query PerCore_SandyBridge_EP_Query
#define     Start_Broadwell_EP Start_SandyBridge_EP
#define     Stop_Broadwell_EP Stop_SandyBridge_EP
#define     InitTimer_Broadwell_EP InitTimer_SandyBridge_EP
#define     Start_Uncore_Broadwell_EP Start_Uncore_SandyBridge_EP
#define     Stop_Uncore_Broadwell_EP Stop_Uncore_SandyBridge_EP

#define     PerCore_Skylake_Query PerCore_SandyBridge_Query
static void Start_Skylake(void *arg) ;
static void Stop_Skylake(void *arg) ;
extern void InitTimer_Skylake(unsigned int cpu) ;
static void Start_Uncore_Skylake(void *arg) ;
static void Stop_Uncore_Skylake(void *arg) ;

extern void Query_Skylake_X(void) ;
#define     PerCore_Skylake_X_Query PerCore_SandyBridge_Query
static void Start_Skylake_X(void *arg) ;
static void Stop_Skylake_X(void *arg) ;
extern void InitTimer_Skylake_X(unsigned int cpu) ;
static void Start_Uncore_Skylake_X(void *arg) ;
static void Stop_Uncore_Skylake_X(void *arg) ;

extern void Query_AMD_Family_0Fh(void) ;
static void PerCore_AMD_Family_0Fh_Query(void *arg) ;
static void Start_AMD_Family_0Fh(void *arg) ;
static void Stop_AMD_Family_0Fh(void *arg) ;
extern void InitTimer_AMD_Family_0Fh(unsigned int cpu) ;

extern void Query_AMD_Family_10h(void) ;
static void PerCore_AMD_Family_10h_Query(void *arg) ;
static void Start_AMD_Family_10h(void *arg) ;
static void Stop_AMD_Family_10h(void *arg) ;

extern void Query_AMD_Family_11h(void) ;
#define     PerCore_AMD_Family_11h_Query PerCore_AMD_Family_10h_Query
#define     Start_AMD_Family_11h Start_AMD_Family_10h
#define     Stop_AMD_Family_11h Stop_AMD_Family_10h

extern void Query_AMD_Family_12h(void) ;
#define     PerCore_AMD_Family_12h_Query PerCore_AMD_Family_10h_Query

extern void Query_AMD_Family_14h(void) ;
#define     PerCore_AMD_Family_14h_Query PerCore_AMD_Family_10h_Query

extern void Query_AMD_Family_15h(void) ;
#define     PerCore_AMD_Family_15h_Query PerCore_AMD_Family_10h_Query

extern void Query_AMD_Family_17h(void) ;
static void PerCore_AMD_Family_17h_Query(void *arg) ;
static void Start_AMD_Family_17h(void *arg) ;
static void Stop_AMD_Family_17h(void *arg) ;
extern void InitTimer_AMD_Family_17Fh(unsigned int cpu) ;

//	[Void]
#define _Void_Signature {.ExtFamily=0x0, .Family=0x0, .ExtModel=0x0, .Model=0x0}

//	[Core]		06_0Eh (32 bits)
#define _Core_Yonah	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xE}

//	[Core2]		06_0Fh, 06_15h, 06_16h, 06_17h, 06_1Dh
#define _Core_Conroe	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xF}
#define _Core_Kentsfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x5}
#define _Core_Conroe_616 \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x6}
#define _Core_Yorkfield {.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x7}
#define _Core_Dunnington \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xD}

//	[Atom]		06_1Ch, 06_26h, 06_27h (32bits), 06_35h (32bits), 06_36h
#define _Atom_Bonnell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xC}
#define _Atom_Silvermont \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x6}
#define _Atom_Lincroft	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x7}
#define _Atom_Clovertrail \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x5}
#define _Atom_Saltwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x6}

//	[Silvermont]	06_37h
#define _Silvermont_637	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x7}

//	[Avoton]	06_4Dh
#define _Atom_Avoton	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xD}

//	[Airmont]	06_4Ch
#define _Atom_Airmont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xC}
//	[Goldmont]	06_5Ch
#define _Atom_Goldmont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xC}
//	[SoFIA]		06_5Dh
#define _Atom_Sofia	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xD}
//	[Merrifield]	06_4Ah
#define _Atom_Merrifield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xA}
//	[Moorefield]	06_5Ah
#define _Atom_Moorefield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xA}

//	[Nehalem]	06_1Ah, 06_1Eh, 06_1Fh, 06_2Eh
#define _Nehalem_Bloomfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xA}
#define _Nehalem_Lynnfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xE}
#define _Nehalem_MB	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xF}
#define _Nehalem_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xE}

//	[Westmere]	06_25h, 06_2Ch, 06_2Fh
#define _Westmere	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x5}
#define _Westmere_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xC}
#define _Westmere_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xF}

//	[Sandy Bridge]	06_2Ah, 06_2Dh
#define _SandyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xA}
#define _SandyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xD}

//	[Ivy Bridge]	06_3Ah, 06_3Eh
#define _IvyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xA}
#define _IvyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xE}

//	[Haswell]	06_3Ch, 06_3Fh, 06_45h, 06_46h
#define _Haswell_DT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xC}
#define _Haswell_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xF}
#define _Haswell_ULT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x5}
#define _Haswell_ULX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x6}

//	[Broadwell]	06_3Dh, 06_56h, 06_47h, 06_4Fh
#define _Broadwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xD}
#define _Broadwell_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x6}
#define _Broadwell_H	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x7}
#define _Broadwell_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xF}

//	[Skylake]	06_4Eh, 06_5Eh, 06_55h
#define _Skylake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xE}
#define _Skylake_S	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xE}
#define _Skylake_X	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x5}

//	[Xeon Phi]	06_57h, 06_85h
#define _Xeon_Phi	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x7}

//	[Kabylake]	06_8Eh, 06_9Eh
#define _Kabylake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xE}
#define _Kabylake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xE}

//	[Cannonlake]	06_66h
#define _Cannonlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0x6}

//	[Geminilake]	06_7Ah
#define _Geminilake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xA}

//	[Icelake]	06_7Eh
#define _Icelake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xE}

//	[Family 0Fh]	0F_00h
#define _AMD_Family_0Fh {.ExtFamily=0x0, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 10h]	1F_00h
#define _AMD_Family_10h {.ExtFamily=0x1, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 11h]	2F_00h
#define _AMD_Family_11h {.ExtFamily=0x2, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 12h]	3F_00h
#define _AMD_Family_12h {.ExtFamily=0x3, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 14h]	5F_00h
#define _AMD_Family_14h {.ExtFamily=0x5, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 15h]	6F_00h
#define _AMD_Family_15h {.ExtFamily=0x6, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 16h]	7F_00h
#define _AMD_Family_16h {.ExtFamily=0x7, .Family=0xF, .ExtModel=0x0, .Model=0x0}

//	[Family 17h]	8F_00h
#define _AMD_Family_17h {.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x0}


typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

static PCI_CALLBACK P945(struct pci_dev *dev) ;
static PCI_CALLBACK P955(struct pci_dev *dev) ;
static PCI_CALLBACK P965(struct pci_dev *dev) ;
static PCI_CALLBACK G965(struct pci_dev *dev) ;
static PCI_CALLBACK P35(struct pci_dev *dev) ;
static PCI_CALLBACK Bloomfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev) ;
static PCI_CALLBACK NHM_NON_CORE(struct pci_dev *dev) ;
static PCI_CALLBACK X58_QPI(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK IVB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK SKL_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK SKL_SA(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0F_MCH(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0F_HTT(struct pci_dev *dev) ;

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static struct pci_device_id PCI_Core2_ids[] = {
	{	// 82945G - Lakeport
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82945P_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	// 82945GM - Lakeport
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82945GM_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	// 82945GME/SE - Calistoga
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82945GME_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	// 82955X - Lakeport
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82955_HB),
		.driver_data = (kernel_ulong_t) P955
	},
	{	// 946PL/946GZ - Lakeport
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82946GZ_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// Q963/Q965 - Broadwater
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965Q_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// P965/G965 - Broadwater
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965G_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	// GM965 - Crestline
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965GM_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// GME965 - Crestline
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965GME_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// GM45 - Cantiga
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_GM45_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	// Q35 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// P35/G33 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Q33 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// X38/X48 - Bearlake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X38_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// 3200/3210 - Unknown
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_3200_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// Q45/Q43 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// P45/G45 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	// G41 - Eaglelake
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G41_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{0, }
};

	// 1st Generation
static struct pci_device_id PCI_Nehalem_QPI_ids[] = {
	{	// Bloomfield IMC
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MCR),
		.driver_data = (kernel_ulong_t) Bloomfield_IMC
	},
	{	// Bloomfield IMC Test Registers
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	// Nehalem Control Status and RAS Registers
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X58_HUB_CTRL),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{	// Nehalem Bloomfield/Xeon C3500: Non-Core Registers
	PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_BLOOMFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{	// Nehalem EP Xeon C5500: Non-Core Registers
	PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_NEHALEM_EP_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

static struct pci_device_id PCI_Nehalem_DMI_ids[] = {
	{	// Lynnfield IMC
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR),
		.driver_data = (kernel_ulong_t) Lynnfield_IMC
	},
	{	// Lynnfield IMC Test Registers
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	// Lynnfield QuickPath Architecture Generic Non-core Registers
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{	// Westmere/Clarkdale QuickPath Architecture Non-core Registers
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_CLARKDALE_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

	// 2nd Generation
	// Sandy Bridge ix-2xxx, Xeon E3-E5: IMC_HA=0x3ca0 / IMC_TA=0x3ca8 /
	// TA0=0x3caa, TA1=0x3cab / TA2=0x3cac / TA3=0x3cad / TA4=0x3cae
static struct pci_device_id PCI_SandyBridge_ids[] = {
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{	// Desktop: IMC_SystemAgent=0x0100,0x0104
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{0, }
};

	// 3rd Generation
	// Ivy Bridge ix-3xxx, Xeon E7/E5 v2: IMC_HA=0x0ea0 / IMC_TA=0x0ea8
	// TA0=0x0eaa / TA1=0x0eab / TA2=0x0eac / TA3=0x0ead
static struct pci_device_id PCI_IvyBridge_ids[] = {
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_HA0),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{	// Desktop: IMC_SystemAgent=0x0150
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{	// Mobile i5-3337U: IMC=0x0154
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{0, }
};

	// 4th Generation
	// Haswell ix-4xxx, Xeon E7/E5 v3: IMC_HA0=0x2fa0 / IMC_HA0_TA=0x2fa8
	// TAD0=0x2faa / TAD1=0x2fab / TAD2=0x2fac / TAD3=0x2fad
static struct pci_device_id PCI_Haswell_ids[] = {
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	// Desktop: IMC_SystemAgent=0x0c00
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{0, }
};

	// 5th Generation
	// Broadwell ix-5xxx: IMC_HA0=0x1604
static struct pci_device_id PCI_Broadwell_ids[] = {
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	// Desktop: IMC_SystemAgent=0x0c00
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{0, }
};

	// 6th Generation
static struct pci_device_id PCI_Skylake_ids[] = {
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_SA
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_SA
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_SA
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_SA
	},
	{0, }
};

static struct pci_device_id PCI_Skylake_X_ids[] = {
	{0, }
};

	// 7th & 8th Generation
static struct pci_device_id PCI_Kabylake_ids[] = {
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_Y_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_U_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_X_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAH),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{0, }
};

	// AMD Family 0Fh
static struct pci_device_id PCI_AMD_0F_ids[] = {
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_K8_NB_MEMCTL),
		.driver_data = (kernel_ulong_t) AMD_0F_MCH
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_K8_NB),
		.driver_data = (kernel_ulong_t) AMD_0F_HTT
	},
	{0, }
};

static ARCH Arch[ARCHITECTURES] = {
/*  0*/	{
	.Signature = _Void_Signature,
	.Query = NULL,
	.Update = NULL,
	.Start = NULL,
	.Stop = NULL,
	.Exit = NULL,
	.Timer = NULL,
	.Clock = NULL,
	.Architecture = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  1*/	{
	.Signature = _Core_Yonah,
	.Query = Query_GenuineIntel,
	.Update = PerCore_Intel_Query,
	.Start = Start_GenuineIntel,
	.Stop = Stop_GenuineIntel,
	.Exit = NULL,
	.Timer = InitTimer_GenuineIntel,
	.Clock = Clock_Core,
	.Architecture = "Core/Yonah",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  2*/	{
	.Signature = _Core_Conroe,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Conroe/Merom",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_MEROM,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  3*/	{
	.Signature = _Core_Kentsfield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Kentsfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  4*/	{
	.Signature = _Core_Conroe_616,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Conroe/Yonah",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  5*/	{
	.Signature = _Core_Yorkfield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Yorkfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  6*/	{
	.Signature = _Core_Dunnington,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Xeon/Dunnington",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

/*  7*/	{
	.Signature = _Atom_Bonnell,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Bonnell",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  8*/	{
	.Signature = _Atom_Silvermont,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Silvermont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/*  9*/	{
	.Signature = _Atom_Lincroft,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Lincroft",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/* 10*/	{
	.Signature = _Atom_Clovertrail,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Clovertrail",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/* 11*/	{
	.Signature = _Atom_Saltwell,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Saltwell",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

/* 12*/	{
	.Signature = _Silvermont_637,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Silvermont,
	.Architecture = "Silvermont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/* 13*/	{
	.Signature = _Atom_Avoton,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Silvermont,
	.Architecture = "Atom/Avoton",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},

/* 14*/	{
	.Signature = _Atom_Airmont,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Airmont,
	.Architecture = "Atom/Airmont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/* 15*/	{
	.Signature = _Atom_Goldmont,
	.Query = Query_SandyBridge,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Atom/Goldmont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT
		}
	},
/* 16*/	{
	.Signature = _Atom_Sofia,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Sofia",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/* 17*/	{
	.Signature = _Atom_Merrifield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Merrifield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
/* 18*/	{
	.Signature = _Atom_Moorefield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Moorefield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

/* 19*/	{
	.Signature = _Nehalem_Bloomfield,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/Bloomfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},
/* 20*/	{
	.Signature = _Nehalem_Lynnfield,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/Lynnfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},
/* 21*/	{
	.Signature = _Nehalem_MB,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/Mobile",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},
/* 22*/	{
	.Signature = _Nehalem_EX,
	.Query = Query_Core2,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/eXtreme.EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},

/* 23*/	{
	.Signature = _Westmere,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Westmere,
	.Architecture = "Westmere",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},
/* 24*/	{
	.Signature = _Westmere_EP,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Westmere,
	.Architecture = "Westmere/EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},
/* 25*/	{
	.Signature = _Westmere_EX,
	.Query = Query_Core2,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Westmere,
	.Architecture = "Westmere/eXtreme",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem
		}
	},

/* 26*/	{
	.Signature = _SandyBridge,
	.Query = Query_SandyBridge,
	.Update = PerCore_SandyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_SandyBridge,
	.Architecture = "SandyBridge",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_SandyBridge_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge
		}
	},
/* 27*/	{
	.Signature = _SandyBridge_EP,
	.Query = Query_SandyBridge,
	.Update = PerCore_SandyBridge_EP_Query,
	.Start = Start_SandyBridge_EP,
	.Stop = Stop_SandyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge_EP,
	.Clock = Clock_SandyBridge,
	.Architecture = "SandyBridge/eXtreme.EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_SandyBridge_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge_EP,
		.Stop = Stop_Uncore_SandyBridge_EP
		}
	},

/* 28*/	{
	.Signature = _IvyBridge,
	.Query = Query_IvyBridge,
	.Update = PerCore_IvyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_IvyBridge,
	.Architecture = "IvyBridge",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_IvyBridge_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge
		}
	},
/* 29*/	{
	.Signature = _IvyBridge_EP,
	.Query = Query_IvyBridge_EP,
	.Update = PerCore_IvyBridge_EP_Query,
	.Start = Start_SandyBridge_EP,
	.Stop = Stop_SandyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge_EP,
	.Clock = Clock_IvyBridge,
	.Architecture = "IvyBridge/EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_IvyBridge_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge_EP,
		.Stop = Stop_Uncore_SandyBridge_EP
		}
	},

/* 30*/	{
	.Signature = _Haswell_DT,
	.Query = Query_IvyBridge,
	.Update = PerCore_IvyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/Desktop",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge
		}
	},
/* 31*/	{
	.Signature = _Haswell_EP,
	.Query = Query_Haswell_EP,
	.Update = PerCore_SandyBridge_EP_Query,
	.Start = Start_SandyBridge_EP,
	.Stop = Stop_SandyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge_EP,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/EP/Mobile",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge_EP,
		.Stop = Stop_Uncore_SandyBridge_EP
		}
	},
/* 32*/	{
	.Signature = _Haswell_ULT,
	.Query = Query_IvyBridge,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/Ultra Low TDP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT
		}
	},
/* 33*/	{
	.Signature = _Haswell_ULX,
	.Query = Query_IvyBridge,
	.Update = PerCore_IvyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/Ultra Low eXtreme",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge
		}
	},

/* 34*/	{
	.Signature = _Broadwell,
	.Query = Query_Broadwell,
	.Update = PerCore_Broadwell_Query,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/Mobile",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Broadwell,
		.Stop = Stop_Uncore_Broadwell
		}
	},
/* 35*/	{
	.Signature = _Broadwell_EP,
	.Query = Query_Haswell_EP,
	.Update = PerCore_Broadwell_EP_Query,
	.Start = Start_Broadwell_EP,
	.Stop = Stop_Broadwell_EP,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell_EP,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Broadwell_EP,
		.Stop = Stop_Uncore_Broadwell_EP
		}
	},
/* 36*/	{
	.Signature = _Broadwell_H,
	.Query = Query_Broadwell,
	.Update = PerCore_Broadwell_Query,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/H",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Broadwell,
		.Stop = Stop_Uncore_Broadwell
		}
	},
/* 37*/	{
	.Signature = _Broadwell_EX,
	.Query = Query_Haswell_EP,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/EX",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT
		}
	},

/* 38*/	{
	.Signature = _Skylake_UY,
	.Query = Query_SandyBridge,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Skylake/UY",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Skylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake
		}
	},
/* 39*/	{
	.Signature = _Skylake_S,
	.Query = Query_SandyBridge,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Skylake/S",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Skylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake
		}
	},
/* 40*/	{
	.Signature = _Skylake_X,
	.Query = Query_Skylake_X,
	.Update = PerCore_Skylake_X_Query,
	.Start = Start_Skylake_X,
	.Stop = Stop_Skylake_X,
	.Exit = NULL,
	.Timer = InitTimer_Skylake_X,
	.Clock = Clock_Skylake,
	.Architecture = "Skylake/X",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SKL_X,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Skylake_X_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake_X,
		.Stop = Stop_Uncore_Skylake_X
		}
	},

/* 41*/	{
	.Signature = _Xeon_Phi,
	.Query = Query_SandyBridge,
	.Update = PerCore_SandyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Skylake,
	.Architecture = "Knights Landing",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge
		}
	},

/* 42*/	{
	.Signature = _Kabylake,
	.Query = Query_Broadwell,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Kaby/Coffee Lake",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake
		}
	},
/* 43*/	{
	.Signature = _Kabylake_UY,
	.Query = Query_Broadwell,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Kaby Lake/UY",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake
		}
	},

/* 44*/	{
	.Signature = _Cannonlake,
	.Query = Query_Broadwell,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Cannon Lake",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake
		}
	},

/* 45*/	{
	.Signature = _Geminilake,
	.Query = Query_SandyBridge,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Atom/Gemini Lake",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT
		}
	},

/* 46*/	{
	.Signature = _Icelake_UY,
	.Query = Query_SandyBridge,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Ice Lake/UY",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake
		}
	},

	{
	.Signature = _AMD_Family_0Fh,
	.Query = Query_AMD_Family_0Fh,
	.Update = PerCore_AMD_Family_0Fh_Query,
	.Start = Start_AMD_Family_0Fh,
	.Stop = Stop_AMD_Family_0Fh,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_0Fh,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 0Fh",
	.thermalFormula = THERMAL_FORMULA_AMD_0F,
	.voltageFormula = VOLTAGE_FORMULA_AMD_0F,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_AMD_0F_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_10h,
	.Query = Query_AMD_Family_10h,
	.Update = PerCore_AMD_Family_10h_Query,
	.Start = Start_AMD_Family_10h,
	.Stop = Stop_AMD_Family_10h,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 10h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_11h,
	.Query = Query_AMD_Family_11h,
	.Update = PerCore_AMD_Family_11h_Query,
	.Start = Start_AMD_Family_11h,
	.Stop = Stop_AMD_Family_11h,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 11h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_12h,
	.Query = Query_AMD_Family_12h,
	.Update = PerCore_AMD_Family_12h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 12h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_14h,
	.Query = Query_AMD_Family_14h,
	.Update = PerCore_AMD_Family_14h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 14h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_15h,
	.Query = Query_AMD_Family_15h,
	.Update = PerCore_AMD_Family_15h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 15h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_16h,
	.Query = Query_AMD_Family_15h,
	.Update = PerCore_AMD_Family_15h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 16h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},

	{
	.Signature = _AMD_Family_17h,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17Fh,
	.Clock = Clock_AMD_Family_17h,
	.Architecture = "Family 17h",
	.thermalFormula = THERMAL_FORMULA_AMD_17F,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL
		}
	},
};
