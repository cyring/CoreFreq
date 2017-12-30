/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define MAXCOUNTER(M, m)	((M) > (m) ? (M) : (m))
#define MINCOUNTER(m, M)	((m) < (M) ? (m) : (M))


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

#define RDTSC(_lo, _hi)							\
	asm volatile							\
	(								\
		"lfence"		"\n\t"				\
		"rdtsc"							\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
	);

#define RDTSCP(_lo, _hi, aux)						\
	asm volatile							\
	(								\
		"rdtscp"						\
		: "=a" (_lo),						\
		  "=d" (_hi),						\
		  "=c" (aux)						\
	);

#define BARRIER()							\
	asm volatile							\
	(								\
		"lfence"						\
		:							\
		:							\
		:							\
	);

#define RDTSC64(_val64)							\
	asm volatile							\
	(								\
		"lfence"		"\n\t"				\
		"rdtsc"			"\n\t"				\
		"shlq	$32,	%%rdx"	"\n\t"				\
		"orq	%%rdx,	%%rax"	"\n\t"				\
		"movq	%%rax,	%0"					\
		: "=m" (_val64)						\
		:							\
		: "%rax","%rcx","%rdx","memory"				\
	);

#define RDTSCP64(_val64)						\
	asm volatile							\
	(								\
		"rdtscp"		"\n\t"				\
		"shlq	$32,	%%rdx"	"\n\t"				\
		"orq	%%rdx,	%%rax"	"\n\t"				\
		"movq	%%rax,	%0"					\
		: "=m" (_val64)						\
		:							\
		: "%rax","%rcx","%rdx","memory"				\
	);

#define ASM_RDTSCP(_reg) \
	"# Read invariant TSC."		"\n\t"		\
	"rdtscp"			"\n\t"		\
	"shlq	$32, %%rdx"		"\n\t"		\
	"orq	%%rdx, %%rax"		"\n\t"		\
	"# Save TSC value."		"\n\t"		\
	"movq	%%rax, %%" #_reg	"\n\t"

#define ASM_RDTSC(_reg) \
	"# Read variant TSC."		"\n\t"		\
	"lfence"			"\n\t"		\
	"rdtsc"				"\n\t"		\
	"shlq	$32, %%rdx"		"\n\t"		\
	"orq	%%rdx, %%rax"		"\n\t"		\
	"# Save TSC value."		"\n\t"		\
	"movq	%%rax, %%" #_reg	"\n\t"


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
	void			(*Start)(void *arg);	// Must be static
	void			(*Stop)(void *arg);	// Must be static
	void			(*Exit)(void);
	void			(*Timer)(unsigned int cpu);
	CLOCK			(*Clock)(unsigned int ratio);
	char			*Architecture;
	unsigned long long	thermalFormula,
				voltageFormula;
	struct pci_device_id	*PCI_ids;
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
static void Start_GenuineIntel(void *arg) ;
static void Stop_GenuineIntel(void *arg) ;
extern void InitTimer_GenuineIntel(unsigned int cpu) ;

extern void Query_AuthenticAMD(void) ;
static void Start_AuthenticAMD(void *arg) ;
static void Stop_AuthenticAMD(void *arg) ;
extern void InitTimer_AuthenticAMD(unsigned int cpu) ;

extern void Query_Core2(void) ;
static void Start_Core2(void *arg) ;
static void Stop_Core2(void *arg) ;
extern void InitTimer_Core2(unsigned int cpu) ;

extern void Query_Nehalem(void) ;
static void Start_Nehalem(void *arg) ;
static void Stop_Nehalem(void *arg) ;
extern void InitTimer_Nehalem(unsigned int cpu) ;

#define     Query_SandyBridge Query_Nehalem
static void Start_SandyBridge(void *arg) ;
static void Stop_SandyBridge(void *arg) ;
extern void InitTimer_SandyBridge(unsigned int cpu) ;

extern void Query_IvyBridge_EP(void) ;

extern void Query_Haswell_EP(void) ;
static void Start_Haswell_ULT(void *arg) ;
static void Stop_Haswell_ULT(void *arg) ;
extern void InitTimer_Haswell_ULT(unsigned int cpu) ;

#define     Query_Broadwell Query_SandyBridge
#define     Start_Broadwell Start_SandyBridge
#define     Stop_Broadwell Stop_SandyBridge
#define     InitTimer_Broadwell InitTimer_SandyBridge

extern void Query_Skylake_X(void) ;
static void Start_Skylake(void *arg) ;
static void Stop_Skylake(void *arg) ;
extern void InitTimer_Skylake(unsigned int cpu) ;

extern void Query_AMD_Family_0Fh(void) ;
static void Start_AMD_Family_0Fh(void *arg) ;
static void Stop_AMD_Family_0Fh(void *arg) ;
extern void InitTimer_AMD_Family_0Fh(unsigned int cpu) ;

extern void Query_AMD_Family_10h(void) ;
static void Start_AMD_Family_10h(void *arg) ;
static void Stop_AMD_Family_10h(void *arg) ;

extern void Query_AMD_Family_11h(void) ;
#define     Start_AMD_Family_11h Start_AMD_Family_10h
#define     Stop_AMD_Family_11h Stop_AMD_Family_10h

extern void Query_AMD_Family_12h(void) ;

extern void Query_AMD_Family_14h(void) ;

extern void Query_AMD_Family_15h(void) ;

extern void Query_AMD_Family_17h(void) ;

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

static PCI_CALLBACK P965(struct pci_dev *dev) ;
static PCI_CALLBACK G965(struct pci_dev *dev) ;
static PCI_CALLBACK P35(struct pci_dev *dev) ;
static PCI_CALLBACK Bloomfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev) ;
static PCI_CALLBACK X58_QPI(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK IVB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK SKL_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0F_MCH(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0F_HTT(struct pci_dev *dev) ;

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static struct pci_device_id PCI_Core2_ids[] = {
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
	{	// Desktop: IMC_SystemAgent=0x0100
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA),
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
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
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

static ARCH Arch[ARCHITECTURES]=
{
/*  0*/	{
	.Signature = _Void_Signature,
	.Query = NULL,
	.Start = NULL,
	.Stop = NULL,
	.Exit = NULL,
	.Timer = NULL,
	.Clock = NULL,
	.Architecture = NULL,
	.thermalFormula = THERMAL_FORMULA_NONE,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/*  1*/	{
	.Signature = _Core_Yonah,
	.Query = Query_GenuineIntel,
	.Start = Start_GenuineIntel,
	.Stop = Stop_GenuineIntel,
	.Exit = NULL,
	.Timer = InitTimer_GenuineIntel,
	.Clock = Clock_Core,
	.Architecture = "Core/Yonah",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids
	},
/*  2*/	{
	.Signature = _Core_Conroe,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Conroe/Merom",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_MEROM,
	.PCI_ids = PCI_Core2_ids
	},
/*  3*/	{
	.Signature = _Core_Kentsfield,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Kentsfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids
	},
/*  4*/	{
	.Signature = _Core_Conroe_616,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Conroe/Yonah",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids
	},
/*  5*/	{
	.Signature = _Core_Yorkfield,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Core2/Yorkfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids
	},
/*  6*/	{
	.Signature = _Core_Dunnington,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Core2,
	.Architecture = "Xeon/Dunnington",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids
	},

/*  7*/	{
	.Signature = _Atom_Bonnell,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Bonnell",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/*  8*/	{
	.Signature = _Atom_Silvermont,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Silvermont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/*  9*/	{
	.Signature = _Atom_Lincroft,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Lincroft",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 10*/	{
	.Signature = _Atom_Clovertrail,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Clovertrail",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 11*/	{
	.Signature = _Atom_Saltwell,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Saltwell",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},

/* 12*/	{
	.Signature = _Silvermont_637,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Silvermont,
	.Architecture = "Silvermont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 13*/	{
	.Signature = _Atom_Avoton,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Silvermont,
	.Architecture = "Atom/Avoton",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},

/* 14*/	{
	.Signature = _Atom_Airmont,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Airmont,
	.Architecture = "Atom/Airmont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 15*/	{
	.Signature = _Atom_Goldmont,
	.Query = Query_SandyBridge,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Atom/Goldmont",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 16*/	{
	.Signature = _Atom_Sofia,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Sofia",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 17*/	{
	.Signature = _Atom_Merrifield,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Merrifield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},
/* 18*/	{
	.Signature = _Atom_Moorefield,
	.Query = Query_Core2,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.Clock = Clock_Atom,
	.Architecture = "Atom/Moorefield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},

/* 19*/	{
	.Signature = _Nehalem_Bloomfield,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/Bloomfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids
	},
/* 20*/	{
	.Signature = _Nehalem_Lynnfield,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/Lynnfield",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids
	},
/* 21*/	{
	.Signature = _Nehalem_MB,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/Mobile",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids
	},
/* 22*/	{
	.Signature = _Nehalem_EX,
	.Query = Query_Core2,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Nehalem,
	.Architecture = "Nehalem/eXtreme.EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids
	},

/* 23*/	{
	.Signature = _Westmere,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Westmere,
	.Architecture = "Westmere",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids
	},
/* 24*/	{
	.Signature = _Westmere_EP,
	.Query = Query_Nehalem,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Westmere,
	.Architecture = "Westmere/EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids
	},
/* 25*/	{
	.Signature = _Westmere_EX,
	.Query = Query_Core2,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.Clock = Clock_Westmere,
	.Architecture = "Westmere/eXtreme",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids
	},

/* 26*/	{
	.Signature = _SandyBridge,
	.Query = Query_SandyBridge,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_SandyBridge,
	.Architecture = "SandyBridge",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_SandyBridge_ids
	},
/* 27*/	{
	.Signature = _SandyBridge_EP,
	.Query = Query_SandyBridge,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_SandyBridge,
	.Architecture = "SandyBridge/eXtreme.EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_SandyBridge_ids
	},

/* 28*/	{
	.Signature = _IvyBridge,
	.Query = Query_SandyBridge,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_IvyBridge,
	.Architecture = "IvyBridge",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_IvyBridge_ids
	},
/* 29*/	{
	.Signature = _IvyBridge_EP,
	.Query = Query_IvyBridge_EP,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_IvyBridge,
	.Architecture = "IvyBridge/EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_IvyBridge_ids
	},

/* 30*/	{
	.Signature = _Haswell_DT,
	.Query = Query_SandyBridge,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/Desktop",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Haswell_ids
	},
/* 31*/	{
	.Signature = _Haswell_EP,
	.Query = Query_Haswell_EP,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/EP/Mobile",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Haswell_ids
	},
/* 32*/	{
	.Signature = _Haswell_ULT,
	.Query = Query_SandyBridge,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/Ultra Low TDP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Haswell_ids
	},
/* 33*/	{
	.Signature = _Haswell_ULX,
	.Query = Query_SandyBridge,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Haswell,
	.Architecture = "Haswell/Ultra Low eXtreme",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Haswell_ids
	},

/* 34*/	{
	.Signature = _Broadwell,
	.Query = Query_Broadwell,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/Mobile",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Broadwell_ids
	},
/* 35*/	{
	.Signature = _Broadwell_EP,
	.Query = Query_Haswell_EP,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/EP",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Broadwell_ids
	},
/* 36*/	{
	.Signature = _Broadwell_H,
	.Query = Query_Broadwell,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/H",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Broadwell_ids
	},
/* 37*/	{
	.Signature = _Broadwell_EX,
	.Query = Query_Haswell_EP,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Broadwell/EX",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Broadwell_ids
	},

/* 38*/	{
	.Signature = _Skylake_UY,
	.Query = Query_SandyBridge,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Skylake/UY",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Skylake_ids
	},
/* 39*/	{
	.Signature = _Skylake_S,
	.Query = Query_SandyBridge,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Skylake/S",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Skylake_ids
	},
/* 40*/	{
	.Signature = _Skylake_X,
	.Query = Query_Skylake_X,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Skylake,
	.Architecture = "Skylake/X",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Skylake_ids
	},

/* 41*/	{
	.Signature = _Xeon_Phi,
	.Query = Query_SandyBridge,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.Clock = Clock_Skylake,
	.Architecture = "Knights Landing",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},

/* 42*/	{
	.Signature = _Kabylake,
	.Query = Query_SandyBridge,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Kaby/Coffee Lake",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Kabylake_ids
	},
/* 43*/	{
	.Signature = _Kabylake_UY,
	.Query = Query_SandyBridge,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Kaby Lake/UY",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Kabylake_ids
	},

/* 44*/	{
	.Signature = _Cannonlake,
	.Query = Query_SandyBridge,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Cannon Lake",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Void_ids
	},

/* 45*/	{
	.Signature = _Geminilake,
	.Query = Query_SandyBridge,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.Clock = Clock_Haswell,
	.Architecture = "Atom/Gemini Lake",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids
	},

/* 46*/	{
	.Signature = _Icelake_UY,
	.Query = Query_SandyBridge,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.Clock = Clock_Skylake,
	.Architecture = "Ice Lake/UY",
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_0Fh,
	.Query = Query_AMD_Family_0Fh,
	.Start = Start_AMD_Family_0Fh,
	.Stop = Stop_AMD_Family_0Fh,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_0Fh,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 0Fh",
	.thermalFormula = THERMAL_FORMULA_AMD_0F,
	.voltageFormula = VOLTAGE_FORMULA_AMD_0F,
	.PCI_ids = PCI_AMD_0F_ids
	},

	{
	.Signature = _AMD_Family_10h,
	.Query = Query_AMD_Family_10h,
	.Start = Start_AMD_Family_10h,
	.Stop = Stop_AMD_Family_10h,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 10h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_11h,
	.Query = Query_AMD_Family_11h,
	.Start = Start_AMD_Family_11h,
	.Stop = Stop_AMD_Family_11h,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 11h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_12h,
	.Query = Query_AMD_Family_12h,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 12h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_14h,
	.Query = Query_AMD_Family_14h,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 14h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_15h,
	.Query = Query_AMD_Family_15h,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 15h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_16h,
	.Query = Query_AMD_Family_15h,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AuthenticAMD,
	.Architecture = "Family 16h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},

	{
	.Signature = _AMD_Family_17h,
	.Query = Query_AMD_Family_17h,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
	.Clock = Clock_AMD_Family_17h,
	.Architecture = "Family 17h",
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids
	},
};
