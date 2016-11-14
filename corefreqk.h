/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	ROUND_TO_PAGES(Size)	PAGE_SIZE * ((Size / PAGE_SIZE) 	\
				+ ((Size % PAGE_SIZE)? 1:0));

#define	LOOP_MIN_MS	500
#define LOOP_MAX_MS	5000
#define	LOOP_DEF_MS	1000

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

#define	RDTSC(_lo, _hi)							\
	asm volatile							\
	(								\
		"lfence			\n\t"				\
		"rdtsc"							\
		:"=a" (_lo),						\
		 "=d" (_hi)						\
	);

#define	RDTSCP(_lo, _hi, aux)						\
	asm volatile							\
	(								\
		"rdtscp"						\
		:"=a" (_lo),						\
		 "=d" (_hi),						\
		 "=c" (aux)						\
	);

#define	BARRIER()							\
	asm volatile							\
	(								\
		"lfence"						\
		:							\
		:							\
		:							\
	);

#define	RDTSC64(_val64)							\
	asm volatile							\
	(								\
		"lfence			\n\t"				\
		"rdtsc			\n\t"				\
		"shlq	$32,	%%rdx	\n\t"				\
		"orq	%%rdx,	%%rax	\n\t"				\
		"movq	%%rax,	%0"					\
		:"=m" (_val64)						\
		:							\
		:"%rax","%rcx","%rdx","memory"				\
	);

#define	RDTSCP64(_val64)						\
	asm volatile							\
	(								\
		"rdtscp			\n\t"				\
		"shlq	$32,	%%rdx	\n\t"				\
		"orq	%%rdx,	%%rax	\n\t"				\
		"movq	%%rax,	%0"					\
		:"=m" (_val64)						\
		:							\
		:"%rax","%rcx","%rdx","memory"				\
	);

#define ASM_RDTSCP(_reg) \
"# Read invariant TSC.""\n\t" \
"rdtscp""\n\t" \
"shlq	$32, %%rdx""\n\t" \
"orq	%%rdx, %%rax""\n\t" \
"# Save TSC value.""\n\t" \
"movq	%%rax, %%" #_reg "\n\t" \

#define ASM_RDTSC(_reg) \
"# Read variant TSC.""\n\t" \
"lfence""\n\t" \
"rdtsc""\n\t" \
"shlq	$32, %%rdx""\n\t" \
"orq	%%rdx, %%rax""\n\t" \
"# Save TSC value.""\n\t" \
"movq	%%rax, %%" #_reg "\n\t" \


#define ASM_CODE_RDMSR(_msr, _reg) \
"# Read MSR counter.""\n\t" \
"movq	$" #_msr ", %%rcx""\n\t" \
"rdmsr""\n\t" \
"shlq	$32, %%rdx""\n\t" \
"orq	%%rdx, %%rax""\n\t" \
"# Save counter value.""\n\t" \
"movq	%%rax, %%" #_reg "\n\t" \

#define ASM_RDMSR(_msr, _reg) ASM_CODE_RDMSR(_msr, _reg)


#define ASM_COUNTERx1(	_reg0, _reg1, \
			_tsc_inst, mem_tsc, \
			_msr1, _mem1) \
asm volatile \
( \
_tsc_inst(_reg0) \
ASM_RDMSR(_msr1, _reg1) \
"# Store values into memory.""\n\t" \
"movq	%%" #_reg0 ", %0""\n\t" \
"movq	%%" #_reg1 ", %1" \
: "=m" (mem_tsc), "=m" (_mem1) \
: \
:"%rax", "%rcx", "%rdx", \
"%" #_reg0"", "%" #_reg1"", \
"memory" \
);


#define ASM_COUNTERx2(	_reg0, _reg1, _reg2, \
			_tsc_inst, mem_tsc, \
			_msr1, _mem1, _msr2, _mem2) \
asm volatile \
( \
_tsc_inst(_reg0) \
ASM_RDMSR(_msr1, _reg1) \
ASM_RDMSR(_msr2, _reg2) \
"# Store values into memory.""\n\t" \
"movq	%%" #_reg0 ", %0""\n\t" \
"movq	%%" #_reg1 ", %1""\n\t" \
"movq	%%" #_reg2 ", %2" \
: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2) \
: \
:"%rax", "%rcx", "%rdx", \
"%" #_reg0"", "%" #_reg1"", "%" #_reg2"", \
"memory" \
);


#define ASM_COUNTERx3(	_reg0, _reg1, _reg2, _reg3, \
			_tsc_inst, mem_tsc, \
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3) \
asm volatile \
( \
_tsc_inst(_reg0) \
ASM_RDMSR(_msr1, _reg1) \
ASM_RDMSR(_msr2, _reg2) \
ASM_RDMSR(_msr3, _reg3) \
"# Store values into memory.""\n\t" \
"movq	%%" #_reg0 ", %0""\n\t" \
"movq	%%" #_reg1 ", %1""\n\t" \
"movq	%%" #_reg2 ", %2""\n\t" \
"movq	%%" #_reg3 ", %3" \
: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3) \
: \
:"%rax", "%rcx", "%rdx", \
"%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"", \
"memory" \
);


#define ASM_COUNTERx4(	_reg0, _reg1, _reg2, _reg3, _reg4, \
			_tsc_inst, mem_tsc, \
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3, \
			_msr4, _mem4) \
asm volatile \
( \
_tsc_inst(_reg0) \
ASM_RDMSR(_msr1, _reg1) \
ASM_RDMSR(_msr2, _reg2) \
ASM_RDMSR(_msr3, _reg3) \
ASM_RDMSR(_msr4, _reg4) \
"# Store values into memory.""\n\t" \
"movq	%%" #_reg0 ", %0""\n\t" \
"movq	%%" #_reg1 ", %1""\n\t" \
"movq	%%" #_reg2 ", %2""\n\t" \
"movq	%%" #_reg3 ", %3""\n\t" \
"movq	%%" #_reg4 ", %4" \
: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3), \
"=m" (_mem4) \
: \
:"%rax", "%rcx", "%rdx", \
"%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"", \
"%" #_reg4"", \
"memory" \
);


#define ASM_COUNTERx5(	_reg0, _reg1, _reg2, _reg3, _reg4, _reg5, \
			_tsc_inst, mem_tsc, \
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3, \
			_msr4, _mem4, _msr5, _mem5) \
asm volatile \
( \
_tsc_inst(_reg0) \
ASM_RDMSR(_msr1, _reg1) \
ASM_RDMSR(_msr2, _reg2) \
ASM_RDMSR(_msr3, _reg3) \
ASM_RDMSR(_msr4, _reg4) \
ASM_RDMSR(_msr5, _reg5) \
"# Store values into memory.""\n\t" \
"movq	%%" #_reg0 ", %0""\n\t" \
"movq	%%" #_reg1 ", %1""\n\t" \
"movq	%%" #_reg2 ", %2""\n\t" \
"movq	%%" #_reg3 ", %3""\n\t" \
"movq	%%" #_reg4 ", %4""\n\t" \
"movq	%%" #_reg5 ", %5" \
: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3), \
"=m" (_mem4), "=m" (_mem5) \
: \
:"%rax", "%rcx", "%rdx", \
"%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"", \
"%" #_reg4"", "%" #_reg5"", \
"memory" \
);


#define ASM_COUNTERx6(	_reg0, _reg1, _reg2, _reg3, _reg4, _reg5, _reg6, \
			_tsc_inst, mem_tsc, \
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3, \
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6) \
asm volatile \
( \
_tsc_inst(_reg0) \
ASM_RDMSR(_msr1, _reg1) \
ASM_RDMSR(_msr2, _reg2) \
ASM_RDMSR(_msr3, _reg3) \
ASM_RDMSR(_msr4, _reg4) \
ASM_RDMSR(_msr5, _reg5) \
ASM_RDMSR(_msr6, _reg6) \
"# Store values into memory.""\n\t" \
"movq	%%" #_reg0 ", %0""\n\t" \
"movq	%%" #_reg1 ", %1""\n\t" \
"movq	%%" #_reg2 ", %2""\n\t" \
"movq	%%" #_reg3 ", %3""\n\t" \
"movq	%%" #_reg4 ", %4""\n\t" \
"movq	%%" #_reg5 ", %5""\n\t" \
"movq	%%" #_reg6 ", %6" \
: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3), \
"=m" (_mem4), "=m" (_mem5), "=m" (_mem6) \
: \
:"%rax", "%rcx", "%rdx", \
"%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"", \
"%" #_reg4"", "%" #_reg5"", "%" #_reg6"", \
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
		: "i"	(_reg)						\
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
		  "i"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})


typedef struct
{
	struct kmem_cache	*Cache;
	CORE			*Core[];
} KPUBLIC;

typedef struct
{
	struct hrtimer		Timer;
	struct				// Timer State Machine.
	{
		unsigned long long
			created	:  1-0,	// hrtimer_init() || ?()
			started	:  2-1,	// hrtimer_start() || hrtimer_cancel()
			mustFwd	:  3-2,	// hrtimer_forward()
			_pad64	: 64-3;
	} tsm;
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
	void			(*Start)(void *arg);
	void			(*Stop)(void *arg);
	void			(*Exit)(void);
	void			(*Timer)(unsigned int cpu);
	CLOCK			(*Clock)(unsigned int ratio);
	char			*Architecture;
} ARCH;

extern CLOCK Clock_GenuineIntel(unsigned int ratio) ;
extern CLOCK Clock_AuthenticAMD(unsigned int ratio) ;
extern CLOCK Clock_Core(unsigned int ratio) ;
extern CLOCK Clock_Core2(unsigned int ratio) ;
extern CLOCK Clock_Atom(unsigned int ratio) ;
extern CLOCK Clock_Silvermont(unsigned int ratio) ;
extern CLOCK Clock_Nehalem(unsigned int ratio) ;
extern CLOCK Clock_Westmere(unsigned int ratio) ;
extern CLOCK Clock_SandyBridge(unsigned int ratio) ;
extern CLOCK Clock_IvyBridge(unsigned int ratio) ;
extern CLOCK Clock_Haswell(unsigned int ratio) ;
extern CLOCK Clock_Skylake(unsigned int ratio) ;

extern void Query_GenuineIntel(void) ;
extern void Start_GenuineIntel(void *arg) ;
extern void Stop_GenuineIntel(void *arg) ;
extern void InitTimer_GenuineIntel(unsigned int cpu) ;

extern void Query_AuthenticAMD(void) ;
extern void Start_AuthenticAMD(void *arg) ;
extern void Stop_AuthenticAMD(void *arg) ;
extern void InitTimer_AuthenticAMD(unsigned int cpu) ;

extern void Query_Core2(void) ;
extern void Start_Core2(void *arg) ;
extern void Stop_Core2(void *arg) ;
extern void InitTimer_Core2(unsigned int cpu) ;

extern void Query_Nehalem(void) ;
extern void Start_Nehalem(void *arg) ;
extern void Stop_Nehalem(void *arg) ;
extern void InitTimer_Nehalem(unsigned int cpu) ;

extern void Query_SandyBridge(void) ;
extern void Start_SandyBridge(void *arg) ;
extern void Stop_SandyBridge(void *arg) ;
extern void InitTimer_SandyBridge(unsigned int cpu) ;

//	[Void]
#define	_Void_Signature	{.ExtFamily=0x0, .Family=0x0, .ExtModel=0x0, .Model=0x0}

//	[Core]		06_0EH (32 bits)
#define	_Core_Yonah	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xE}

//	[Core2]		06_0FH, 06_15H, 06_16H, 06_17H, 06_1D
#define	_Core_Conroe	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xF}
#define	_Core_Kentsfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x5}
#define	_Core_Conroe_616 \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x6}
#define	_Core_Yorkfield	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x7}
#define	_Core_Dunnington \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xD}

//	[Atom]		06_1CH, 06_26H, 06_27H (32bits), 06_35H (32bits), 06_36H
#define	_Atom_Bonnell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xC}
#define	_Atom_Silvermont \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x6}
#define	_Atom_Lincroft	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x7}
#define	_Atom_Clovertrail \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x5}
#define	_Atom_Saltwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x6}

//	[Silvermont]	06_37H
#define	_Silvermont_637	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x7}

//	[Avoton]	06_4DH
#define	_Atom_Avoton	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xD}

//	[Airmont]	06_4CH
#define	_Atom_Airmont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xC}
//	[Goldmont]	06_5CH
#define	_Atom_Goldmont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xC}
//	[SoFIA]		06_5DH
#define	_Atom_Sofia	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xD}
//	[Merrifield]	06_4AH
#define	_Atom_Merrifield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xA}
//	[Moorefield]	06_5AH
#define	_Atom_Moorefield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xA}

//	[Nehalem]	06_1AH, 06_1EH, 06_1FH, 06_2EH
#define	_Nehalem_Bloomfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xA}
#define	_Nehalem_Lynnfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xE}
#define	_Nehalem_MB	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xF}
#define	_Nehalem_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xE}

//	[Westmere]	06_25H, 06_2CH, 06_2FH
#define	_Westmere	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x5}
#define	_Westmere_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xC}
#define	_Westmere_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xF}

//	[Sandy Bridge]	06_2AH, 06_2DH
#define	_SandyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xA}
#define	_SandyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xD}

//	[Ivy Bridge]	06_3AH, 06_3EH
#define	_IvyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xA}
#define	_IvyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xE}

//	[Haswell]	06_3CH, 06_3FH, 06_45H, 06_46H
#define	_Haswell_DT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xC}
#define	_Haswell_MB	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xF}
#define	_Haswell_ULT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x5}
#define	_Haswell_ULX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x6}

//	[Broadwell]	06_3DH, 06_56H, 06_47H, 06_4FH
#define	_Broadwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xD}
#define	_Broadwell_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x6}
#define	_Broadwell_H	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x7}
#define	_Broadwell_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xF}

//	[Skylake]	06_4EH, 06_5EH, 06_55H
#define	_Skylake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xE}
#define	_Skylake_S	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xE}
#define	_Skylake_E	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x5}

//	[Kabylake]	06_8E || 06_94 || 06_9E
#define	_Kabylake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xE}

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
	Haswell_MB,		\
	Haswell_ULT,		\
	Haswell_ULX,		\
	Broadwell,		\
	Broadwell_EP,		\
	Broadwell_H,		\
	Broadwell_EX,		\
	Skylake_UY,		\
	Skylake_S,		\
	Skylake_E,		\
	Kabylake,		\
	ARCHITECTURES
};

static ARCH Arch[ARCHITECTURES]=
{
/*  0*/	{
	_Void_Signature,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	},
/*  1*/	{
	_Core_Yonah,
	Query_GenuineIntel,
	Start_GenuineIntel,
	Stop_GenuineIntel,
	NULL,
	InitTimer_GenuineIntel,
	Clock_Core,
	"Core/Yonah"
	},
/*  2*/	{
	_Core_Conroe,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Core2,
	"Core2/Conroe"
	},
/*  3*/	{
	_Core_Kentsfield,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Core2,
	"Core2/Kentsfield"
	},
/*  4*/	{
	_Core_Conroe_616,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Core2,
	"Core2/Conroe/Yonah"
	},
/*  5*/	{
	_Core_Yorkfield,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Core2,
	"Core2/Yorkfield"
	},
/*  6*/	{
	_Core_Dunnington,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Core2,
	"Xeon/Dunnington"
	},

/*  7*/	{
	_Atom_Bonnell,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Bonnell"
	},
/*  8*/	{
	_Atom_Silvermont,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Silvermont"
	},
/*  9*/	{
	_Atom_Lincroft,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Lincroft"
	},
/* 10*/	{
	_Atom_Clovertrail,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Clovertrail"
	},
/* 11*/	{
	_Atom_Saltwell,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Saltwell"
	},

/* 12*/	{
	_Silvermont_637,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Silvermont,
	"Silvermont"
	},
/* 13*/	{
	_Atom_Avoton,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Silvermont,
	"Atom/Avoton"
	},

/* 14*/	{
	_Atom_Airmont,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Airmont"
	},
/* 15*/	{
	_Atom_Goldmont,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Goldmont"
	},
/* 16*/	{
	_Atom_Sofia,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Sofia"
	},
/* 17*/	{
	_Atom_Merrifield,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Merrifield"
	},
/* 18*/	{
	_Atom_Moorefield,
	Query_Core2,
	Start_Core2,
	Stop_Core2,
	NULL,
	InitTimer_Core2,
	Clock_Atom,
	"Atom/Moorefield"
	},

/* 19*/	{
	_Nehalem_Bloomfield,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Nehalem,
	"Nehalem/Bloomfield"
	},
/* 20*/	{
	_Nehalem_Lynnfield,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Nehalem,
	"Nehalem/Lynnfield"
	},
/* 21*/	{
	_Nehalem_MB,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Nehalem,
	"Nehalem/Mobile"
	},
/* 22*/	{
	_Nehalem_EX,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Nehalem,
	"Nehalem/eXtreme.EP"
	},

/* 23*/	{
	_Westmere,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Westmere,
	"Westmere"
	},
/* 24*/	{
	_Westmere_EP,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Westmere,
	"Westmere/EP"
	},
/* 25*/	{
	_Westmere_EX,
	Query_Nehalem,
	Start_Nehalem,
	Stop_Nehalem,
	NULL,
	InitTimer_Nehalem,
	Clock_Westmere,
	"Westmere/eXtreme"
	},

/* 26*/	{
	_SandyBridge,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_SandyBridge,
	"SandyBridge"
	},
/* 27*/	{
	_SandyBridge_EP,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_SandyBridge,
	"SandyBridge/eXtreme.EP"
	},

/* 28*/	{
	_IvyBridge,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_IvyBridge,
	"IvyBridge"
	},
/* 29*/	{
	_IvyBridge_EP,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_IvyBridge,
	"IvyBridge/EP"
	},

/* 30*/	{
	_Haswell_DT,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Haswell/Desktop"
	},
/* 31*/	{
	_Haswell_MB,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Haswell/Mobile"
	},
/* 32*/	{
	_Haswell_ULT,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Haswell/Ultra Low TDP"
	},
/* 33*/	{
	_Haswell_ULX,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Haswell/Ultra Low eXtreme"
	},

/* 34*/	{
	_Broadwell,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Broadwell/Mobile"
	},
/* 35*/	{
	_Broadwell_EP,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Broadwell/EP"
	},
/* 36*/	{
	_Broadwell_H,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Broadwell/H"
	},
/* 37*/	{
	_Broadwell_EX,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Haswell,
	"Broadwell/EX"
	},

/* 38*/	{
	_Skylake_UY,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Skylake,
	"Skylake/UY"
	},
/* 39*/	{
	_Skylake_S,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Skylake,
	"Skylake/S"
	},
/* 40*/	{
	_Skylake_E,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Skylake,
	"Skylake/E"
	},
/* 41*/	{
	_Kabylake,
	Query_SandyBridge,
	Start_SandyBridge,
	Stop_SandyBridge,
	NULL,
	InitTimer_SandyBridge,
	Clock_Skylake,
	"Kaby Lake"
	}
};
