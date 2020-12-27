/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define RDCOUNTER(_val, _cnt)						\
({									\
	unsigned int _lo, _hi;						\
									\
	__asm__ volatile						\
	(								\
		"rdmsr"							\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_cnt)						\
	);								\
	_val=_lo | ((unsigned long long) _hi << 32);			\
})

#define WRCOUNTER(_val, _cnt)						\
	__asm__ volatile						\
	(								\
		"wrmsr"							\
		:							\
		: "c" (_cnt),						\
		  "a" ((unsigned int) _val & 0xFFFFFFFF),		\
		  "d" ((_val > 0xFFFFFFFF) ?				\
			(unsigned int) (_val >> 32) : 0)		\
	);

#define RDMSR(_data, _reg)						\
({									\
	unsigned int _lo, _hi;						\
									\
	__asm__ volatile						\
	(								\
		"rdmsr"							\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_reg)						\
	);								\
	_data.value=_lo | ((unsigned long long) _hi << 32);		\
})

#define WRMSR(_data, _reg)						\
	__asm__ volatile						\
	(								\
		"wrmsr"							\
		:							\
		: "c" (_reg),						\
		  "a" ((unsigned int) _data.value & 0xFFFFFFFF),	\
		  "d" ((unsigned int) (_data.value >> 32))		\
	);

#define RDMSR64(_data, _reg)						\
	__asm__ volatile						\
	(								\
		"xorq	%%rax, %%rax"	"\n\t"				\
		"xorq	%%rdx, %%rdx"	"\n\t"				\
		"movq	%1,%%rcx"	"\n\t"				\
		"rdmsr"			"\n\t"				\
		"shlq	$32, %%rdx"	"\n\t"				\
		"orq	%%rdx, %%rax"	"\n\t"				\
		"movq	%%rax, %0"					\
		: "=m" (_data)						\
		: "i" (_reg)						\
		: "%rax", "%rcx", "%rdx"				\
	)

#define WRMSR64(_data, _reg)						\
	__asm__ volatile						\
	(								\
		"movq	%0, %%rax"		"\n\t"			\
		"movq	%%rax, %%rdx"		"\n\t"			\
		"shrq	$32, %%rdx"		"\n\t"			\
		"movq	%1, %%rcx"		"\n\t"			\
		"wrmsr"							\
		: "=m" (_data)						\
		: "i" (_reg)						\
		: "%rax", "%rcx", "%rdx"				\
	)

#define Atomic_Read_VPMC(_lock, _dest, _src)				\
{									\
	__asm__ volatile						\
	(								\
		"xorq	%%rax	, %%rax"	"\n\t"			\
	_lock	"cmpxchg %%rax	, %[src]"	"\n\t"			\
		"movq	%%rax	, %[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "m" (_src)					\
		: "%rax", "cc", "memory"				\
	);								\
}

#define Atomic_Add_VPMC(_lock, _dest, _src)				\
{									\
	__asm__ volatile						\
	(								\
		"xorq	%%rax	, %%rax"	"\n\t"			\
	_lock	"cmpxchg %%rax	, %[src]"	"\n\t"			\
		"addq	%%rax	, %[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "m" (_src)					\
		: "%rax", "cc", "memory"				\
	);								\
}

#define ASM_CODE_RDMSR(_msr, _reg) \
	"# Read MSR counter."		"\n\t"				\
	"movq	$" #_msr ", %%rcx"	"\n\t"				\
	"rdmsr"				"\n\t"				\
	"shlq	$32, %%rdx"		"\n\t"				\
	"orq	%%rdx, %%rax"		"\n\t"				\
	"# Save counter value."		"\n\t"				\
	"movq	%%rax, %%" #_reg	"\n\t"

#define ASM_RDMSR(_msr, _reg) ASM_CODE_RDMSR(_msr, _reg)


#define ASM_COUNTERx1(	_reg0, _reg1,					\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1)					\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"					\
	: "=m" (mem_tsc), "=m" (_mem1)					\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"",					\
	  "cc", "memory"						\
);


#define ASM_COUNTERx2(	_reg0, _reg1, _reg2,				\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2)			\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2)			\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"",			\
	  "cc", "memory"						\
);


#define ASM_COUNTERx3(	_reg0, _reg1, _reg2, _reg3,			\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3)	\
__asm__ volatile							\
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
	  "cc", "memory"						\
);


#define ASM_COUNTERx4(	_reg0, _reg1, _reg2, _reg3, _reg4,		\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4)					\
__asm__ volatile							\
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
	  "cc", "memory"						\
);


#define ASM_COUNTERx5(	_reg0, _reg1, _reg2, _reg3, _reg4, _reg5,	\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5)			\
__asm__ volatile							\
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
	  "cc", "memory"						\
);

#define ASM_COUNTERx5_STACK(_reg0, _reg1, _reg2, _reg3, 		\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5)			\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	"pushq	%%" #_reg3		"\n\t"				\
	ASM_RDMSR(_msr4, _reg3)						\
	"pushq	%%" #_reg3		"\n\t"				\
	ASM_RDMSR(_msr5, _reg3)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %5"	"\n\t"				\
	"popq	%%" #_reg3		"\n\t"				\
	"movq	%%" #_reg3 ", %4"	"\n\t"				\
	"popq	%%" #_reg3		"\n\t"				\
	"movq	%%" #_reg3 ", %3"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4), "=m" (_mem5)					\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "cc", "memory"						\
);


#define ASM_COUNTERx6(	_reg0, _reg1, _reg2, _reg3, _reg4, _reg5, _reg6,\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6)	\
__asm__ volatile							\
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
	  "cc", "memory"						\
);

#define ASM_COUNTERx6_STACK(_reg0, _reg1, _reg2, _reg3, 		\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6)	\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	ASM_RDMSR(_msr3, _reg3)						\
	"pushq	%%" #_reg3		"\n\t"				\
	ASM_RDMSR(_msr4, _reg3)						\
	"pushq	%%" #_reg3		"\n\t"				\
	ASM_RDMSR(_msr5, _reg3)						\
	"pushq	%%" #_reg3		"\n\t"				\
	ASM_RDMSR(_msr6, _reg3)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %2"	"\n\t"				\
	"movq	%%" #_reg3 ", %6"	"\n\t"				\
	"popq	%%" #_reg3		"\n\t"				\
	"movq	%%" #_reg3 ", %5"	"\n\t"				\
	"popq	%%" #_reg3		"\n\t"				\
	"movq	%%" #_reg3 ", %4"	"\n\t"				\
	"popq	%%" #_reg3		"\n\t"				\
	"movq	%%" #_reg3 ", %3"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4), "=m" (_mem5), "=m" (_mem6)			\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"", "%" #_reg3"",	\
	  "cc", "memory"						\
);

#define ASM_COUNTERx7(	_reg0, _reg1, _reg2, _reg3,			\
			_reg4, _reg5, _reg6, _reg7,			\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6,	\
			_msr7, _mem7)					\
__asm__ volatile							\
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
	  "cc", "memory"						\
);

#define ASM_COUNTERx7_STACK(_reg0, _reg1, _reg2,			\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1, _msr2, _mem2, _msr3, _mem3,	\
			_msr4, _mem4, _msr5, _mem5, _msr6, _mem6,	\
			_msr7, _mem7)					\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1)						\
	ASM_RDMSR(_msr2, _reg2)						\
	"pushq	%%" #_reg2		"\n\t"				\
	ASM_RDMSR(_msr3, _reg2)						\
	"pushq	%%" #_reg2		"\n\t"				\
	ASM_RDMSR(_msr4, _reg2)						\
	"pushq	%%" #_reg2		"\n\t"				\
	ASM_RDMSR(_msr5, _reg2)						\
	"pushq	%%" #_reg2		"\n\t"				\
	ASM_RDMSR(_msr6, _reg2)						\
	"pushq	%%" #_reg2		"\n\t"				\
	ASM_RDMSR(_msr7, _reg2)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"	"\n\t"				\
	"movq	%%" #_reg2 ", %7"	"\n\t"				\
	"popq	%%" #_reg2		"\n\t"				\
	"movq	%%" #_reg2 ", %6"	"\n\t"				\
	"popq	%%" #_reg2		"\n\t"				\
	"movq	%%" #_reg2 ", %5"	"\n\t"				\
	"popq	%%" #_reg2		"\n\t"				\
	"movq	%%" #_reg2 ", %4"	"\n\t"				\
	"popq	%%" #_reg2		"\n\t"				\
	"movq	%%" #_reg2 ", %3"	"\n\t"				\
	"popq	%%" #_reg2		"\n\t"				\
	"movq	%%" #_reg2 ", %2"					\
	: "=m" (mem_tsc), "=m" (_mem1), "=m" (_mem2), "=m" (_mem3),	\
	  "=m" (_mem4), "=m" (_mem5), "=m" (_mem6), "=m" (_mem7)	\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"", "%" #_reg2"",			\
	  "cc", "memory"						\
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


#if defined(OPTIM_LVL) && OPTIM_LVL == 0

#define RDTSC_COUNTERx5(mem_tsc, ...) \
ASM_COUNTERx5_STACK(r12, r13, r14, r15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx5(mem_tsc, ...) \
ASM_COUNTERx5_STACK(r12, r13, r14, r15, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx6(mem_tsc, ...) \
ASM_COUNTERx6_STACK(r12, r13, r14, r15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx6(mem_tsc, ...) \
ASM_COUNTERx6_STACK(r12, r13, r14, r15, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#define RDTSC_COUNTERx7(mem_tsc, ...) \
ASM_COUNTERx7_STACK(r13, r14, r15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_COUNTERx7(mem_tsc, ...) \
ASM_COUNTERx7_STACK(r13, r14, r15, ASM_RDTSCP, mem_tsc, __VA_ARGS__)

#else
/*	#warning "Optimization"						*/

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

#endif


#define PCI_CONFIG_ADDRESS(bus, dev, fn, reg) \
	(0x80000000 | (bus << 16) | (dev << 11) | (fn << 8) | (reg & ~3))

#define RDPCI(_data, _reg)						\
({									\
	__asm__ volatile						\
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
	__asm__ volatile						\
	(								\
		"movl	%1,	%%eax"	"\n\t"				\
		"movl	$0xcf8,	%%edx"	"\n\t"				\
		"outl	%%eax,	%%dx"	"\n\t"				\
		"movl	%0,	%%eax"	"\n\t"				\
		"movl	$0xcfc,	%%edx"	"\n\t"				\
		"outl	%%eax,	%%dx"					\
		:							\
		: "irm" (_data),					\
		  "ir"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

/* Manufacturers Identifier Strings.					*/
#define VENDOR_INTEL	"GenuineIntel"
#define VENDOR_AMD	"AuthenticAMD"
#define VENDOR_HYGON	"HygonGenuine"
#define VENDOR_KVM	"TCGTGTCGCGTC"
#define VENDOR_VBOX	"VBoxVBoxVBox"
#define VENDOR_KBOX	"KVMKM"
#define VENDOR_VMWARE	"VMwawarereVM"
#define VENDOR_HYPERV	"Micrt Hvosof"

/* Source: Winbond W83627 datasheet					*/
#define HWM_W83627_INDEX_PORT	0x295
#define HWM_W83627_DATA_PORT	0x296
#define HWM_W83627_CPUVCORE	0x20

#define RDSIO(_data, _reg, _index_port, _data_port)			\
({									\
	__asm__ volatile						\
	(								\
		"movw	%1,	%%ax"	"\n\t"				\
		"movw	%2,	%%dx"	"\n\t"				\
		"outw	%%ax,	%%dx"	"\n\t"				\
		"movw	%3,	%%dx"	"\n\t"				\
		"inb	%%dx,	%%al"	"\n\t"				\
		"movb	%%al,	%0"					\
		: "=m"	(_data) 					\
		: "i"	(_reg) ,					\
		  "i"	(_index_port),					\
		  "i"	(_data_port)					\
		: "%ax", "%dx", "memory"				\
	);								\
})

/* Hardware Monitoring: Super I/O chipsets				*/
#define COMPATIBLE		0xffff
#define W83627			0x5ca3
#define IT8720			0x8720

/*
 * --- Core_AMD_SMN_Read ---
 *
 * amd_smn_read() protects the SMU access through mutex_[un]lock
 * functions which must not be used in interrupt context.
 *
 * The high resolution timers are bound to CPUs with smp_call_function_*
 * where context is in interrupt; and where mutex will freeze the kernel.
*/

#if defined(CONFIG_AMD_NB) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0))\
 && defined(LEGACY) && (LEGACY > 1)
FEAT_MSG("LEGACY Level 2: Core_AMD_SMN_Read() built with amd_smn_read()")
#define Core_AMD_SMN_Read(	SMN_Register,				\
				SMN_Address,				\
				SMU_IndexRegister,			\
				SMU_DataRegister )			\
({									\
    if (PRIVATE(OF(ZenIF_dev)) != NULL)					\
    {									\
	if (amd_smn_read(amd_pci_dev_to_node_id(PRIVATE(OF(ZenIF_dev))),\
			SMN_Address, &SMN_Register.value))		\
	{								\
		pr_warn("CoreFreq: Failed to read amd_smn_read()\n");	\
	}								\
    }									\
})
#else
#define Core_AMD_SMN_Read(	SMN_Register,				\
				SMN_Address,				\
				SMU_IndexRegister,			\
				SMU_DataRegister )			\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(AMD_SMN_LOCK)),		\
				ATOMIC_SEED );				\
	if ( ret == 0 ) {						\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		WRPCI(SMN_Address, SMU_IndexRegister);			\
		RDPCI(SMN_Register.value, SMU_DataRegister);		\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(AMD_SMN_LOCK)),		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
	if (tries == 0) {						\
		pr_warn("CoreFreq: Core_AMD_SMN_Read(%x, %x) failed\n", \
			SMN_Register.value, SMN_Address);		\
	}								\
})
#endif /* CONFIG_AMD_NB and LEGACY */


/* Driver' private and public data definitions.				*/
enum CSTATES_CLASS {
	CSTATES_NHM,
	CSTATES_SNB,
	CSTATES_ULT,
	CSTATES_SKL,
	CSTATES_SOC_SLM,
	CSTATES_SOC_GDM
};

#define LATCH_NONE		0b0000
#define LATCH_TGT_RATIO_UNLOCK	0b0001	/* <T>	TgtRatioUnlocked	*/
#define LATCH_CLK_RATIO_UNLOCK	0b0010	/* <X>	ClkRatioUnlocked	*/
#define LATCH_TURBO_UNLOCK	0b0100	/* <B>	TurboUnlocked		*/
#define LATCH_UNCORE_UNLOCK	0b1000	/* <U>	UncoreUnlocked		*/

typedef struct {
	char			*CodeName;
} MICRO_ARCH;

typedef struct {
	char			**Brand;
	unsigned int		Boost[2];
	THERMAL_PARAM		Param;
	unsigned int		CodeNameIdx	:  8-0,
				TgtRatioUnlocked:  9-8,  /*	<T:1>	*/
				ClkRatioUnlocked: 11-9,  /*	<X:2>	*/
				TurboUnlocked	: 12-11, /*	<B:1>	*/
				UncoreUnlocked	: 13-12, /*	<U:1>	*/
				_UnusedSpecBits : 28-13,
				Latch		: 32-28; /* <U>-<B>-<X>-<T> */
} PROCESSOR_SPECIFIC;

typedef struct {
	FEATURES	*Features;
	char		*Brand;
	unsigned int	SMT_Count,
			localProcessor;
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
	CORE_RO			**Core_RO;
	CORE_RW			**Core_RW;
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
#ifdef CONFIG_AMD_NB
	struct pci_dev		*ZenIF_dev;
#endif
	Bit64			AMD_SMN_LOCK __attribute__ ((aligned (8)));
	Bit64			AMD_FCH_LOCK __attribute__ ((aligned (8)));
	struct kmem_cache	*Cache;
	JOIN			*Join[];
} KPRIVATE;


/* Sources:
 * Intel® 64 and IA-32 Architectures Software Developer’s Manual; Vol. 2A
 * AMD64 Architecture Programmer’s Manual; Vol. 3
*/

static const CPUID_STRUCT CpuIDforVendor[CPUID_MAX_FUNC] = {
/* x86 */
	[CPUID_00000001_00000000_INSTRUCTION_SET]
	= {.func = 0x00000001, .sub = 0x00000000},
/* Intel */
	[CPUID_00000002_00000000_CACHE_AND_TLB]
	= {.func = 0x00000002, .sub = 0x00000000},
	[CPUID_00000003_00000000_PROC_SERIAL_NUMBER]
	= {.func = 0x00000003, .sub = 0x00000000},
	[CPUID_00000004_00000000_CACHE_L1I]
	= {.func = 0x00000004, .sub = 0x00000000},
	[CPUID_00000004_00000001_CACHE_L1D]
	= {.func = 0x00000004, .sub = 0x00000001},
	[CPUID_00000004_00000002_CACHE_L2]
	= {.func = 0x00000004, .sub = 0x00000002},
	[CPUID_00000004_00000003_CACHE_L3]
	= {.func = 0x00000004, .sub = 0x00000003},
/* x86 */
	[CPUID_00000005_00000000_MONITOR_MWAIT]
	= {.func = 0x00000005, .sub = 0x00000000},
	[CPUID_00000006_00000000_POWER_AND_THERMAL_MGMT]
	= {.func = 0x00000006, .sub = 0x00000000},
	[CPUID_00000007_00000000_EXTENDED_FEATURES]
	= {.func = 0x00000007, .sub = 0x00000000},
	[CPUID_00000007_00000001_EXT_FEAT_SUB_LEAF_1]
	= {.func = 0x00000007, .sub = 0x00000001},
/* Intel */
	[CPUID_00000009_00000000_DIRECT_CACHE_ACCESS]
	= {.func = 0x00000009, .sub = 0x00000000},
	[CPUID_0000000A_00000000_PERF_MONITORING]
	= {.func = 0x0000000a, .sub = 0x00000000},
/* x86 */
	[CPUID_0000000B_00000000_EXT_TOPOLOGY]
	= {.func = 0x0000000b, .sub = 0x00000000},
	[CPUID_0000000D_00000000_EXT_STATE_MAIN_LEAF]
	= {.func = 0x0000000d, .sub = 0x00000000},
	[CPUID_0000000D_00000001_EXT_STATE_SUB_LEAF]
	= {.func = 0x0000000d, .sub = 0x00000001},
/* AMD */
	[CPUID_0000000D_00000002_EXT_STATE_SUB_LEAF]
	= {.func = 0x0000000d, .sub = 0x00000002},
	[CPUID_0000000D_00000003_BNDREGS_STATE]
	= {.func = 0x0000000d, .sub = 0x00000003},
	[CPUID_0000000D_00000004_BNDCSR_STATE]
	= {.func = 0x0000000d, .sub = 0x00000004},
/* AMD Family 15h */
	[CPUID_0000000D_0000003E_EXT_STATE_SUB_LEAF]
	= {.func = 0x0000000d, .sub = 0x0000003e},
/* Intel */
	[CPUID_0000000F_00000000_QOS_MONITORING_CAP]
	= {.func = 0x0000000f, .sub = 0x00000000},
	[CPUID_0000000F_00000001_L3_QOS_MONITORING]
	= {.func = 0x0000000f, .sub = 0x00000001},
	[CPUID_00000010_00000000_QOS_ENFORCEMENT_CAP]
	= {.func = 0x00000010, .sub = 0x00000000},
	[CPUID_00000010_00000001_L3_ALLOC_ENUMERATION]
	= {.func = 0x00000010, .sub = 0x00000001},
	[CPUID_00000010_00000002_L2_ALLOC_ENUMERATION]
	= {.func = 0x00000010, .sub = 0x00000002},
	[CPUID_00000010_00000003_RAM_BANDWIDTH_ENUM]
	= {.func = 0x00000010, .sub = 0x00000003},
	[CPUID_00000012_00000000_SGX_CAPABILITY]
	= {.func = 0x00000012, .sub = 0x00000000},
	[CPUID_00000012_00000001_SGX_ATTRIBUTES]
	= {.func = 0x00000012, .sub = 0x00000001},
	[CPUID_00000012_00000002_SGX_ENCLAVE_PAGE_CACHE]
	= {.func = 0x00000012, .sub = 0x00000002},
	[CPUID_00000014_00000000_PROCESSOR_TRACE]
	= {.func = 0x00000014, .sub = 0x00000000},
	[CPUID_00000014_00000001_PROC_TRACE_SUB_LEAF]
	= {.func = 0x00000014, .sub = 0x00000001},
	[CPUID_00000015_00000000_TIME_STAMP_COUNTER]
	= {.func = 0x00000015, .sub = 0x00000000},
	[CPUID_00000016_00000000_PROCESSOR_FREQUENCY]
	= {.func = 0x00000016, .sub = 0x00000000},
	[CPUID_00000017_00000000_SYSTEM_ON_CHIP]
	= {.func = 0x00000017, .sub = 0x00000000},
	[CPUID_00000017_00000001_SOC_ATTRIB_SUB_LEAF_1]
	= {.func = 0x00000017, .sub = 0x00000001},
	[CPUID_00000017_00000002_SOC_ATTRIB_SUB_LEAF_2]
	= {.func = 0x00000017, .sub = 0x00000002},
	[CPUID_00000017_00000003_SOC_ATTRIB_SUB_LEAF_3]
	= {.func = 0x00000017, .sub = 0x00000003},
/* Intel */
	[CPUID_00000018_00000000_ADDRESS_TRANSLATION]
	= {.func = 0x00000018, .sub = 0x00000000},
	[CPUID_00000018_00000001_DAT_SUB_LEAF_1]
	= {.func = 0x00000018, .sub = 0x00000001},
	[CPUID_0000001A_00000000_HYBRID_INFORMATION]
	= {.func = 0x0000001a, .sub = 0x00000000},
	[CPUID_0000001B_00000000_PCONFIG_INFORMATION]
	= {.func = 0x0000001b, .sub = 0x00000000},
	[CPUID_0000001F_00000000_EXT_TOPOLOGY_V2]
	= {.func = 0x0000001f, .sub = 0x00000000},
/* x86 */
	[CPUID_80000001_00000000_EXTENDED_FEATURES]
	= {.func = 0x80000001, .sub = 0x00000000},
	[CPUID_80000002_00000000_PROCESSOR_NAME_ID]
	= {.func = 0x80000002, .sub = 0x00000000},
	[CPUID_80000003_00000000_PROCESSOR_NAME_ID]
	= {.func = 0x80000003, .sub = 0x00000000},
	[CPUID_80000004_00000000_PROCESSOR_NAME_ID]
	= {.func = 0x80000004, .sub = 0x00000000},
/* AMD */
	[CPUID_80000005_00000000_CACHES_L1D_L1I_TLB]
	= {.func = 0x80000005, .sub=0x00000000},
/* x86 */
	[CPUID_80000006_00000000_CACHE_L2_SIZE_WAY]
	= {.func = 0x80000006, .sub = 0x00000000},
	[CPUID_80000007_00000000_ADVANCED_POWER_MGMT]
	= {.func = 0x80000007, .sub = 0x00000000},
	[CPUID_80000008_00000000_LM_ADDRESS_SIZE]
	= {.func = 0x80000008, .sub = 0x00000000},
/* AMD */
	[CPUID_8000000A_00000000_SVM_REVISION]
	= {.func = 0x8000000a, .sub = 0x00000000},
	[CPUID_80000019_00000000_CACHES_AND_TLB_1G]
	= {.func = 0x80000019, .sub = 0x00000000},
	[CPUID_8000001A_00000000_PERF_OPTIMIZATION]
	= {.func = 0x8000001a, .sub = 0x00000000},
	[CPUID_8000001B_00000000_INST_BASED_SAMPLING]
	= {.func = 0x8000001b, .sub = 0x00000000},
	[CPUID_8000001C_00000000_LIGHTWEIGHT_PROFILING]
	= {.func = 0x8000001c, .sub = 0x00000000},
	[CPUID_8000001D_00000000_CACHE_L1D_PROPERTIES]
	= {.func = 0x8000001d, .sub = 0x00000000},
	[CPUID_8000001D_00000001_CACHE_L1I_PROPERTIES]
	= {.func = 0x8000001d, .sub = 0x00000001},
	[CPUID_8000001D_00000002_CACHE_L2_PROPERTIES]
	= {.func = 0x8000001d, .sub = 0x00000002},
	[CPUID_8000001D_00000003_CACHE_PROPERTIES_END]
	= {.func = 0x8000001d, .sub = 0x00000003},
	[CPUID_8000001E_00000000_EXTENDED_IDENTIFIERS]
	= {.func = 0x8000001e, .sub = 0x00000000},
/* x86 */
	[CPUID_40000000_00000000_HYPERVISOR_VENDOR]
	= {.func = 0x40000000, .sub = 0x00000000},
	[CPUID_40000001_00000000_HYPERVISOR_INTERFACE]
	= {.func = 0x40000001, .sub = 0x00000000},
	[CPUID_40000002_00000000_HYPERVISOR_VERSION]
	= {.func = 0x40000002, .sub = 0x00000000},
	[CPUID_40000003_00000000_HYPERVISOR_FEATURES]
	= {.func = 0x40000003, .sub = 0x00000000},
	[CPUID_40000004_00000000_HYPERV_REQUIREMENTS]
	= {.func = 0x40000004, .sub = 0x00000000},
	[CPUID_40000005_00000000_HYPERVISOR_LIMITS]
	= {.func = 0x40000005, .sub = 0x00000000},
	[CPUID_40000006_00000000_HYPERVISOR_EXPLOITS]
	= {.func = 0x40000006, .sub = 0x00000000},
};

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
	MICRO_ARCH		*Architecture;
} ARCH;

extern CLOCK BaseClock_GenuineIntel(unsigned int ratio) ;
extern CLOCK BaseClock_AuthenticAMD(unsigned int ratio) ;
extern CLOCK BaseClock_Core(unsigned int ratio) ;
extern CLOCK BaseClock_Core2(unsigned int ratio) ;
extern CLOCK BaseClock_Atom(unsigned int ratio) ;
extern CLOCK BaseClock_Airmont(unsigned int ratio) ;
extern CLOCK BaseClock_Silvermont(unsigned int ratio) ;
extern CLOCK BaseClock_Nehalem(unsigned int ratio) ;
extern CLOCK BaseClock_Westmere(unsigned int ratio) ;
extern CLOCK BaseClock_SandyBridge(unsigned int ratio) ;
extern CLOCK BaseClock_IvyBridge(unsigned int ratio) ;
extern CLOCK BaseClock_Haswell(unsigned int ratio) ;
extern CLOCK BaseClock_Skylake(unsigned int ratio) ;
extern CLOCK BaseClock_AMD_Family_17h(unsigned int ratio) ;

extern long Intel_Turbo_Config8C(CLOCK_ARG *pClockMod) ;
extern long TurboClock_IvyBridge_EP(CLOCK_ARG *pClockMod) ;
extern long TurboClock_Haswell_EP(CLOCK_ARG *pClockMod) ;
extern long TurboClock_Broadwell_EP(CLOCK_ARG *pClockMod) ;
extern long TurboClock_Skylake_X(CLOCK_ARG *pClockMod) ;
extern long TurboClock_AMD_Zen(CLOCK_ARG *pClockMod) ;

extern long ClockMod_Core2_PPC(CLOCK_ARG *pClockMod) ;
extern long ClockMod_Nehalem_PPC(CLOCK_ARG *pClockMod) ;
extern long ClockMod_SandyBridge_PPC(CLOCK_ARG *pClockMod) ;
extern long ClockMod_Intel_HWP(CLOCK_ARG *pClockMod) ;
#define     ClockMod_Broadwell_EP_HWP ClockMod_Intel_HWP
#define     ClockMod_Skylake_HWP ClockMod_Intel_HWP
extern long ClockMod_AMD_Zen(CLOCK_ARG *pClockMod) ;

extern long Haswell_Uncore_Ratio(CLOCK_ARG *pClockMod) ;

extern void Query_GenuineIntel(unsigned int cpu) ;
static void PerCore_Intel_Query(void *arg) ;
static void Start_GenuineIntel(void *arg) ;
static void Stop_GenuineIntel(void *arg) ;
extern void InitTimer_GenuineIntel(unsigned int cpu) ;

extern void Query_AuthenticAMD(unsigned int cpu) ;
static void PerCore_AuthenticAMD_Query(void *arg) ;
static void Start_AuthenticAMD(void *arg) ;
static void Stop_AuthenticAMD(void *arg) ;
extern void InitTimer_AuthenticAMD(unsigned int cpu) ;

extern void Query_Core2(unsigned int cpu) ;
static void PerCore_Core2_Query(void *arg) ;
static void Start_Core2(void *arg) ;
static void Stop_Core2(void *arg) ;
extern void InitTimer_Core2(unsigned int cpu) ;

extern void Query_Silvermont(unsigned int cpu) ;
static void PerCore_Silvermont_Query(void *arg) ;
static void Start_Silvermont(void *arg) ;
static void Stop_Silvermont(void *arg) ;
extern void InitTimer_Silvermont(unsigned int cpu) ;

extern void Query_Nehalem(unsigned int cpu) ;
static void PerCore_Nehalem_Query(void *arg) ;
static void PerCore_Nehalem_EX_Query(void *arg) ;
static void Start_Nehalem(void *arg) ;
static void Stop_Nehalem(void *arg) ;
extern void InitTimer_Nehalem(unsigned int cpu) ;
static void Start_Uncore_Nehalem(void *arg) ;
static void Stop_Uncore_Nehalem(void *arg) ;

extern void Query_SandyBridge(unsigned int cpu) ;
static void PerCore_SandyBridge_Query(void *arg) ;
static void Start_SandyBridge(void *arg) ;
static void Stop_SandyBridge(void *arg) ;
extern void InitTimer_SandyBridge(unsigned int cpu) ;
static void Start_Uncore_SandyBridge(void *arg) ;
static void Stop_Uncore_SandyBridge(void *arg) ;

static void PerCore_SandyBridge_EP_Query(void *arg) ;
static void Start_SandyBridge_EP(void *arg) ;
static void Stop_SandyBridge_EP(void *arg) ;
extern void InitTimer_SandyBridge_EP(unsigned int cpu) ;
static void Start_Uncore_SandyBridge_EP(void *arg) ;
static void Stop_Uncore_SandyBridge_EP(void *arg) ;

extern void Query_IvyBridge(unsigned int cpu) ;
static void PerCore_IvyBridge_Query(void *arg) ;
extern void Query_IvyBridge_EP(unsigned int cpu) ;
static void PerCore_IvyBridge_EP_Query(void *arg) ;

extern void Query_Haswell(unsigned int cpu) ;
static void PerCore_Haswell_Query(void *arg) ;

extern void Query_Haswell_EP(unsigned int cpu) ;
static void PerCore_Haswell_EP_Query(void *arg) ;
static void Start_Haswell_EP(void *arg) ;
static void Stop_Haswell_EP(void *arg) ;
extern void InitTimer_Haswell_EP(unsigned int cpu) ;
static void Start_Uncore_Haswell_EP(void *arg) ;
static void Stop_Uncore_Haswell_EP(void *arg) ;

extern void Query_Haswell_ULT(unsigned int cpu) ;
static void PerCore_Haswell_ULT_Query(void *arg) ;
static void Start_Haswell_ULT(void *arg) ;
static void Stop_Haswell_ULT(void *arg) ;
extern void InitTimer_Haswell_ULT(unsigned int cpu) ;
static void Start_Uncore_Haswell_ULT(void *arg) ;
static void Stop_Uncore_Haswell_ULT(void *arg) ;

static void PerCore_Goldmont_Query(void *arg) ;
static void Start_Goldmont(void *arg) ;

extern void Query_Haswell_ULX(unsigned int cpu) ;
static void PerCore_Haswell_ULX(void *arg) ;

extern void Query_Broadwell(unsigned int cpu) ;
static void PerCore_Broadwell_Query(void *arg) ;
#define     Start_Broadwell Start_SandyBridge
#define     Stop_Broadwell Stop_SandyBridge
#define     InitTimer_Broadwell InitTimer_SandyBridge
#define     Start_Uncore_Broadwell Start_Uncore_SandyBridge
#define     Stop_Uncore_Broadwell Stop_Uncore_SandyBridge

extern void Query_Broadwell_EP(unsigned int cpu) ;

extern void Query_Skylake(unsigned int cpu) ;
static void PerCore_Skylake_Query(void *arg) ;
static void Start_Skylake(void *arg) ;
static void Stop_Skylake(void *arg) ;
extern void InitTimer_Skylake(unsigned int cpu) ;
static void Start_Uncore_Skylake(void *arg) ;
static void Stop_Uncore_Skylake(void *arg) ;

extern void Query_Skylake_X(unsigned int cpu) ;
static void PerCore_Skylake_X_Query(void *arg) ;
static void Start_Skylake_X(void *arg) ;
static void Stop_Skylake_X(void *arg) ;
extern void InitTimer_Skylake_X(unsigned int cpu) ;
static void Start_Uncore_Skylake_X(void *arg) ;
static void Stop_Uncore_Skylake_X(void *arg) ;

extern void Query_AMD_Family_0Fh(unsigned int cpu) ;
static void PerCore_AMD_Family_0Fh_Query(void *arg) ;
static void Start_AMD_Family_0Fh(void *arg) ;
static void Stop_AMD_Family_0Fh(void *arg) ;
extern void InitTimer_AMD_Family_0Fh(unsigned int cpu) ;

extern void Query_AMD_Family_10h(unsigned int cpu) ;
static void PerCore_AMD_Family_10h_Query(void *arg) ;
static void Start_AMD_Family_10h(void *arg) ;
static void Stop_AMD_Family_10h(void *arg) ;
#define     InitTimer_AMD_Family_10h InitTimer_AuthenticAMD

extern void Query_AMD_Family_11h(unsigned int cpu) ;
static void PerCore_AMD_Family_11h_Query(void *arg) ;
static void Start_AMD_Family_11h(void *arg) ;
#define     Stop_AMD_Family_11h Stop_AMD_Family_10h
#define     InitTimer_AMD_Family_11h InitTimer_AuthenticAMD

extern void Query_AMD_Family_12h(unsigned int cpu) ;
static void PerCore_AMD_Family_12h_Query(void *arg) ;
static void Start_AMD_Family_12h(void *arg) ;
#define     Stop_AMD_Family_12h Stop_AMD_Family_10h
#define     InitTimer_AMD_Family_12h InitTimer_AuthenticAMD

extern void Query_AMD_Family_14h(unsigned int cpu) ;
static void PerCore_AMD_Family_14h_Query(void *arg) ;
static void Start_AMD_Family_14h(void *arg) ;
#define     Stop_AMD_Family_14h Stop_AMD_Family_10h
#define     InitTimer_AMD_Family_14h InitTimer_AuthenticAMD

extern void Query_AMD_Family_15h(unsigned int cpu) ;
static void PerCore_AMD_Family_15h_Query(void *arg) ;
static void Start_AMD_Family_15h(void *arg) ;
#define     Stop_AMD_Family_15h Stop_AMD_Family_10h
extern void InitTimer_AMD_Family_15h(unsigned int cpu) ;

#define     Query_AMD_Family_16h Query_AMD_Family_15h
#define     PerCore_AMD_Family_16h_Query PerCore_AMD_Family_15h_Query
#define     Start_AMD_Family_16h Start_AMD_Family_15h
#define     Stop_AMD_Family_16h Stop_AMD_Family_15h
#define     InitTimer_AMD_Family_16h InitTimer_AuthenticAMD

extern void Query_AMD_Family_17h(unsigned int cpu) ;
static void PerCore_AMD_Family_17h_Query(void *arg) ;
static void Start_AMD_Family_17h(void *arg) ;
static void Stop_AMD_Family_17h(void *arg) ;
extern void InitTimer_AMD_Family_17h(unsigned int cpu) ;

void Core_AMD_F17h_No_Thermal(CORE_RO *Core)
{
}
void CTL_AMD_Family_17h_Temp(CORE_RO *Core) ;
void CCD_AMD_Family_17h_Zen2_Temp(CORE_RO *Core) ;
void (*Core_AMD_Family_17h_Temp)(CORE_RO*) = Core_AMD_F17h_No_Thermal;

/*	[Void]								*/
#define _Void_Signature {.ExtFamily=0x0, .Family=0x0, .ExtModel=0x0, .Model=0x0}

/*	[Core]		06_0Eh (32 bits)				*/
#define _Core_Yonah	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xE}

/*	[Core2]		06_0Fh, 06_15h, 06_16h, 06_17h, 06_1Dh		*/
#define _Core_Conroe	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xF}
#define _Core_Kentsfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x5}
#define _Core_Conroe_616 \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x6}
#define _Core_Penryn	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0x7}
#define _Core_Dunnington \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xD}

/*	[Atom]	06_1Ch, 06_26h, 06_27h (32bits), 06_35h (32bits), 06_36h */
#define _Atom_Bonnell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xC}
#define _Atom_Silvermont \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x6}
#define _Atom_Lincroft	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x7}
#define _Atom_Clover_Trail \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x5}
#define _Atom_Saltwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x6}

/*	[Silvermont/Bay_Trail]	06_37h					*/
#define _Silvermont_Bay_Trail \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x7}

/*	[Avoton]	06_4Dh						*/
#define _Atom_Avoton	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xD}

/*	[Airmont]	06_4Ch						*/
#define _Atom_Airmont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xC}
/*	[Goldmont]	06_5Ch						*/
#define _Atom_Goldmont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xC}
/*	[SoFIA]		06_5Dh						*/
#define _Atom_Sofia	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xD}
/*	[Merrifield]	06_4Ah						*/
#define _Atom_Merrifield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xA}
/*	[Moorefield]	06_5Ah						*/
#define _Atom_Moorefield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xA}

/*	[C3000] 	06_5Fh Stepping 0={A0,A1} 1={B0,B1}		*/
#define _Atom_C3000	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xF}

/*	[Tremont/Jacobsville]	06_86h
	[Tremont/Lakefield]	06_8Ah
	[Tremont/Elkhart Lake]	06_96h
	[Tremont/Jasper Lake]	06_9Ch
	[Snow Ridge]							*/
#define _Tremont_Jacobsville \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0x6}
#define _Tremont_Lakefield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xA}
#define _Tremont_Elkhartlake \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0x6}
#define _Tremont_Jasperlake \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xC}

/*	[Nehalem]	06_1Ah, 06_1Eh, 06_1Fh, 06_2Eh			*/
#define _Nehalem_Bloomfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xA}
#define _Nehalem_Lynnfield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xE}
#define _Nehalem_MB	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x1, .Model=0xF}
#define _Nehalem_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xE}

/*	[Westmere]	06_25h, 06_2Ch, 06_2Fh				*/
#define _Westmere	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0x5}
#define _Westmere_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xC}
#define _Westmere_EX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xF}

/*	[Sandy Bridge]	06_2Ah, 06_2Dh					*/
#define _SandyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xA}
#define _SandyBridge_EP {.ExtFamily=0x0, .Family=0x6, .ExtModel=0x2, .Model=0xD}

/*	[Ivy Bridge]	06_3Ah, 06_3Eh					*/
#define _IvyBridge	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xA}
#define _IvyBridge_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xE}

/*	[Haswell]	06_3Ch, 06_3Fh, 06_45h, 06_46h			*/
#define _Haswell_DT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xC}
#define _Haswell_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xF}
#define _Haswell_ULT	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x5}
#define _Haswell_ULX	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x6}

/*	[Broadwell]	06_3Dh, 06_56h, 06_47h, 06_4Fh			*/
#define _Broadwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0xD}
#define _Broadwell_D	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x6}
#define _Broadwell_H	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0x7}
#define _Broadwell_EP	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xF}

/*	[Skylake]	06_4Eh, 06_5Eh, 06_55h				*/
#define _Skylake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xE}
#define _Skylake_S	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xE}
/*	[Skylake]	06_55h Stepping 4
	[Cascade Lake]	06_55h Stepping 7				*/
#define _Skylake_X	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x5}

/*	[Xeon Phi]	06_57h, 06_85h					*/
#define _Xeon_Phi	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x7}

/*	[Kaby Lake]	06_9Eh Stepping 9
	[Coffee Lake]	06_9Eh Stepping 10 and 11			*/
#define _Kabylake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xE}
/*	[Kaby Lake/UY]	06_8Eh Stepping 9
	[Whiskey Lake/U] 06_8Eh Stepping 11
	[Amber Lake/Y]	06_8Eh Stepping 9
	[Comet Lake/U]	06_8Eh Stepping 12				*/
#define _Kabylake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xE}

/*	[Cannon Lake]	06_66h						*/
#define _Cannonlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0x6}

/*	[Gemini Lake]	06_7Ah						*/
#define _Geminilake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xA}

/*	[Ice Lake]	06_7Dh
	[Ice Lake/UY]	06_7Eh
	[Ice Lake/X]	06_6Ah
	[Ice Lake/D]	06_6Ch						*/
#define _Icelake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xD}
#define _Icelake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xE}
#define _Icelake_X	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0xA}
#define _Icelake_D	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0xC}

/*	[Sunny Cove]	06_9Dh						*/
#define _Sunny_Cove	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xD}

/*	[Tiger Lake]	06_8D
	[Tiger Lake/U]	06_8C						*/
#define _Tigerlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xD}
#define _Tigerlake_U	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xC}

/*	[Comet Lake]	06_A5
	[Comet Lake/UL]	06_A6						*/
#define _Cometlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0x5}
#define _Cometlake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0x6}

/*	[Family 0Fh]	0F_00h						*/
#define _AMD_Family_0Fh {.ExtFamily=0x0, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 10h]	1F_00h						*/
#define _AMD_Family_10h {.ExtFamily=0x1, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 11h]	2F_00h						*/
#define _AMD_Family_11h {.ExtFamily=0x2, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 12h]	3F_00h						*/
#define _AMD_Family_12h {.ExtFamily=0x3, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 14h]	5F_00h						*/
#define _AMD_Family_14h {.ExtFamily=0x5, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 15h]	6F_00h						*/
#define _AMD_Family_15h {.ExtFamily=0x6, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 16h]	7F_00h						*/
#define _AMD_Family_16h {.ExtFamily=0x7, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 17h]		8F_00h
	[Zen/Summit Ridge]	8F_01h Stepping 1	14 nm
	[Zen/Whitehaven]	8F_01h Stepping 1	14 nm	HEDT
	[EPYC/Naples]		8F_01h Stepping 2	14 nm	SVR
	[Zen+ Pinnacle Ridge] 	8F_08h Stepping 2	12 nm
	[Zen+ Colfax]		8F_08h Stepping 2	12 nm	HEDT
	[Zen/Raven Ridge]	8F_11h Stepping 0	14 nm	APU
	[Zen/Snowy Owl]		8F_11h Stepping 0	14 nm	SOC
	[Zen+ Picasso]		8F_18h Stepping 1	12 nm	APU
	[Zen2/Raven2]		8F_20h Stepping 0	14 nm	APU
	[EPYC/Rome]		8F_30h Stepping 0	 7 nm	SVR
	[Zen2/Castle Peak]	8F_31h Stepping 0	 7 nm	HEDT
	[Zen2/Renoir]		8F_60h Stepping 1	 7 nm	APU
	[Zen2/Matisse]		8F_71h Stepping 0	 7 nm		*/
#define _AMD_Zen	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x1}
#define _AMD_Zen_APU	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x1, .Model=0x1}
#define _AMD_ZenPlus	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x8}
#define _AMD_ZenPlus_APU {.ExtFamily=0x8,.Family=0xF, .ExtModel=0x1, .Model=0x8}
#define _AMD_Zen_APU_Rv2 {.ExtFamily=0x8,.Family=0xF, .ExtModel=0x2, .Model=0x0}
#define _AMD_EPYC_Rome	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x3, .Model=0x0}
#define _AMD_Zen2_CPK	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x3, .Model=0x1}
#define _AMD_Zen2_APU	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x6, .Model=0x0}
#define _AMD_Zen2_MTS	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x7, .Model=0x1}

#define _AMD_Family_17h {.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 18h]	9F_00h						*/
#define _AMD_Family_18h {.ExtFamily=0x9, .Family=0xF, .ExtModel=0x0, .Model=0x0}


typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

static PCI_CALLBACK P945(struct pci_dev *dev) ;
static PCI_CALLBACK P955(struct pci_dev *dev) ;
static PCI_CALLBACK P965(struct pci_dev *dev) ;
static PCI_CALLBACK G965(struct pci_dev *dev) ;
static PCI_CALLBACK P35(struct pci_dev *dev) ;
static PCI_CALLBACK SoC_SLM(struct pci_dev *dev) ;
static PCI_CALLBACK Bloomfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK Westmere_EP_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev) ;
static PCI_CALLBACK NHM_NON_CORE(struct pci_dev *dev) ;
static PCI_CALLBACK X58_VTD(struct pci_dev *dev) ;
static PCI_CALLBACK X58_QPI(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK IVB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_HB(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_QPI(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_CAP(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_CTRL0(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_CTRL1(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL0_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_IMC_CTRL1_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL0_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_TAD_CTRL1_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK SKL_IMC(struct pci_dev *dev) ;
/* TODO:
static PCI_CALLBACK SKL_SA(struct pci_dev *dev) ;
*/
static PCI_CALLBACK AMD_0Fh_MCH(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0Fh_HTT(struct pci_dev *dev) ;
#ifdef CONFIG_AMD_NB
static PCI_CALLBACK AMD_17h_ZenIF(struct pci_dev *dev) ;
#endif
static PCI_CALLBACK AMD_Zen_IOMMU(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_17h_UMC(struct pci_dev *dev) ;

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static struct pci_device_id PCI_Core2_ids[] = {
	{	/* 82945G - Lakeport					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82945P_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	/* 82945GM - Calistoga					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82945GM_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	/* 82945GME/SE - Calistoga				*/
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82945GME_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	/* 82955X - Lakeport-X					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82955_HB),
		.driver_data = (kernel_ulong_t) P955
	},
	{	/* 946PL/946GZ - Lakeport-PL/GZ				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82946GZ_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	/* Q963/Q965 - Broadwater				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965Q_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	/* P965/G965 - Broadwater				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965G_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	/* GM965 - Crestline					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965GM_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	/* GME965 - Crestline					*/
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82965GME_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	/* GM45 - Cantiga					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_GM45_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	/* Q35 - Bearlake-Q					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q35_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* P35/G33 - Bearlake-PG+				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* Q33 - Bearlake-QF					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* X38/X48 - Bearlake-X					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X38_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* 3200/3210 - Intel 3200				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_3200_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* Q45/Q43 - Eaglelake-Q				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_Q45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* P45/G45 - Eaglelake-P				*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* G41 - Eaglelake-G					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_G41_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{0, }
};

static struct pci_device_id PCI_SoC_ids[] = {
	{	/* 82945G - Lakeport					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SLM_PTR),
		.driver_data = (kernel_ulong_t) SoC_SLM
	},
	{0, }
};

/* 1st Generation							*/
static struct pci_device_id PCI_Nehalem_QPI_ids[] = {
	{	/* Bloomfield IMC					*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MCR),
		.driver_data = (kernel_ulong_t) Bloomfield_IMC
	},
	{	/* Bloomfield IMC Test Registers			*/
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_I7_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	/* Nehalem Control Status and RAS Registers		*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X58_HUB_CTRL),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X58_HUB_CORE),
		.driver_data = (kernel_ulong_t) X58_VTD
	},
	{	/* Nehalem Bloomfield/Xeon C3500: Non-Core Registers	*/
	PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_BLOOMFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{	/* Nehalem EP Xeon C5500: Non-Core Registers		*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_C5500_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

static struct pci_device_id PCI_Nehalem_DMI_ids[] = {
	{	/* Lynnfield IMC					*/
	      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_LYNNFIELD_MCR),
		.driver_data = (kernel_ulong_t) Lynnfield_IMC
	},
	{	/* Lynnfield IMC Test Registers				*/
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_LYNNFIELD_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{ /* Lynnfield QuickPath Architecture Generic Non-core Registers */
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_LYNNFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{ /* Clarksfield Processor Uncore Device 0, Function 0		*/
      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_CLARKSFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{ /* Westmere/Clarkdale QuickPath Architecture Non-core Registers */
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_CLARKDALE_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

static struct pci_device_id PCI_Westmere_EP_ids[] = {
	{	/* Westmere EP IMC */
		PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_NHM_EP_MCR),
		.driver_data = (kernel_ulong_t) Westmere_EP_IMC
	},
	{	/* Westmere EP IMC Test Registers			*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_NHM_EP_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	/* Nehalem Control Status and RAS Registers		*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X58_HUB_CTRL),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_X58_HUB_CORE),
		.driver_data = (kernel_ulong_t) X58_VTD
	},
	{	/* Westmere EP: Non-Core Registers			*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_NHM_EP_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

/* 2nd Generation
	Sandy Bridge ix-2xxx, Xeon E3-E5: IMC_HA=0x3ca0 / IMC_TA=0x3ca8
	TA0=0x3caa, TA1=0x3cab / TA2=0x3cac / TA3=0x3cad / TA4=0x3cae	*/
static struct pci_device_id PCI_SandyBridge_ids[] = {
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SNB_IMC_HA0),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0100,0x0104		*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SNB_IMC_SA),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SNB_IMC_0104),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{0, }
};

/* 3rd Generation
	Ivy Bridge ix-3xxx, Xeon E7/E5 v2: IMC_HA=0x0ea0 / IMC_TA=0x0ea8
	TA0=0x0eaa / TA1=0x0eab / TA2=0x0eac / TA3=0x0ead		*/
static struct pci_device_id PCI_IvyBridge_ids[] = {
	{	/* Desktop: IMC_SystemAgent=0x0150			*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_IMC_SA),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{	/* Mobile i5-3337U: IMC=0x0154				*/
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_IMC_0154),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{0, }
};

static struct pci_device_id PCI_SandyBridge_EP_ids[] = {
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_HOST_BRIDGE),
		.driver_data = (kernel_ulong_t) SNB_EP_HB
	},
	{
/*	QPIMISCSTAT							*/
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_QPI_LINK0),
		.driver_data = (kernel_ulong_t) SNB_EP_QPI
	},
	{
/*	Power Control Unit						*/
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_IVB_EP_CAPABILITY),
		.driver_data = (kernel_ulong_t) SNB_EP_CAP
	},
	{
/*	Integrated Memory Controller # : General and MemHot Registers	*/
      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CPGC),
		.driver_data = (kernel_ulong_t) SNB_EP_CTRL0
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CPGC),
		.driver_data = (kernel_ulong_t) SNB_EP_CTRL1
	},
	{
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH0),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL0_CHA0
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH1),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL0_CHA1
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH2),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL0_CHA2
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL0_CH3),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL0_CHA3
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH0),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL1_CHA0
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL ,PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH1),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL1_CHA1
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH2),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL1_CHA2
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_IMC_CTRL1_CH3),
		.driver_data = (kernel_ulong_t) SNB_EP_IMC_CTRL1_CHA3
	},
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH0),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL0_CHA0
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH1),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL0_CHA1
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH2),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL0_CHA2
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL0_CH3),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL0_CHA3
	},
	{
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH0),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL1_CHA0
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH1),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL1_CHA1
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH2),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL1_CHA2
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IVB_EP_TAD_CTRL1_CH3),
		.driver_data = (kernel_ulong_t) SNB_EP_TAD_CTRL1_CHA3
	},
	{0, }
};

/* 4th Generation
	Haswell ix-4xxx, Xeon E7/E5 v3: IMC_HA0=0x2fa0 / IMC_HA0_TA=0x2fa8
	TAD0=0x2faa / TAD1=0x2fab / TAD2=0x2fac / TAD3=0x2fad		*/
static struct pci_device_id PCI_Haswell_ids[] = {
	{
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_HASWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0c00			*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	/* Mobile M/H: Host Agent=0x0c04			*/
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HASWELL_MH_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	/* Mobile U/Y: Host Agent=0x0a04			*/
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HASWELL_UY_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{0, }
};

/* 5th Generation
	Broadwell ix-5xxx: IMC_HA0=0x1604 / 0x1614			*/
static struct pci_device_id PCI_Broadwell_ids[] = {
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_BROADWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_BROADWELL_D_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_BROADWELL_H_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0c00			*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_HASWELL_IMC_SA),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{0, }
};

/* 6th Generation							*/
static struct pci_device_id PCI_Skylake_ids[] = {
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SKYLAKE_U_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SKYLAKE_Y_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
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
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SKYLAKE_DT_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{0, }
};

static struct pci_device_id PCI_Skylake_X_ids[] = {
	{0, }
};

/* 7th & 8th Generation							*/
static struct pci_device_id PCI_Kabylake_ids[] = {
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
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
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_KABYLAKE_DT_IMC_HA),
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
      PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_S_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_U_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_H_IMC_HAO),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_W_IMC_HAO),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_COFFEELAKE_R_S_IMC_HAO),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
      PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_WHISKEYLAKE_U_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{0, }
};

/* AMD Family 0Fh							*/
static struct pci_device_id PCI_AMD_0Fh_ids[] = {
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_K8_NB_MEMCTL),
		.driver_data = (kernel_ulong_t) AMD_0Fh_MCH
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_K8_NB),
		.driver_data = (kernel_ulong_t) AMD_0Fh_HTT
	},
	{0, }
};

/* AMD Family 17h							*/
static struct pci_device_id PCI_AMD_17h_ids[] = {
#ifdef CONFIG_AMD_NB
	/* Source: /drivers/hwmon/k10temp.c				*/
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RAVEN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_MATISSE_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_STARSHIP_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RENOIR_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ARIEL_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ARDEN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
	/* Source: HYGON: /arch/x86/kernel/amd_nb.c: DF_F3 = 0x1463	*/
	{
		PCI_VDEVICE(HYGON, PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_ZenIF
	},
#endif /* CONFIG_AMD_NB */
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEN_PLUS_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEPPELIN_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RAVEN_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEN2_MTS_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_STARSHIP_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RENOIR_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEN_APU_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEN2_APU_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ARDEN_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	/* Source: HYGON: PCI list					*/
	{
		PCI_VDEVICE(HYGON, PCI_DEVICE_ID_AMD_17H_ZEN_PLUS_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	/* Source: SMU > Data Fabric > UMC				*/
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ZEPPELIN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RAVEN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_MATISSE_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_STARSHIP_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RENOIR_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ARIEL_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_RAVEN2_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_FIREFLIGHT_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{
		PCI_VDEVICE(AMD, PCI_DEVICE_ID_AMD_17H_ARDEN_DF_F3),
		.driver_data = (kernel_ulong_t) AMD_17h_UMC
	},
	{0, }
};


static MICRO_ARCH Arch_Misc_Processor[] = {
	{NULL}, /* Left as Void for initialization purpose.		*/
	{"EPYC/Naples"},
	{NULL}
};

static MICRO_ARCH Arch_Core_Yonah[]	= {{"Core/Yonah"}	, {NULL}};
static MICRO_ARCH Arch_Core_Conroe[]	= {{"Core2/Conroe/Merom"},{NULL}};
static MICRO_ARCH Arch_Core_Kentsfield[]= {{"Core2/Kentsfield"} , {NULL}};
static MICRO_ARCH Arch_Core_Conroe_616[]= {{"Core2/Conroe/Yonah"},{NULL}};

enum {
	CN_PENRYN,
	CN_YORKFIELD,
	CN_WOLFDALE
};

static MICRO_ARCH Arch_Core_Penryn[] = {
	[CN_PENRYN]		= {"Core2/Penryn"},
	[CN_YORKFIELD]		= {"Core2/Yorkfield"},
	[CN_WOLFDALE]		= {"Core2/Wolfdale"},
	{NULL}
};

static MICRO_ARCH Arch_Core_Dunnington[]= {{"Xeon/Dunnington"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Bonnell[]	= {{"Atom/Bonnell"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Silvermont[]= {{"Atom/Silvermont"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Lincroft[]	= {{"Atom/Lincroft"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Clover_Trail[]={{"Atom/Clovertrail"}, {NULL}};
static MICRO_ARCH Arch_Atom_Saltwell[]	= {{"Atom/Saltwell"}	, {NULL}};
static MICRO_ARCH Arch_Silvermont_Bay_Trail[]={{"Silvermont/SoC"},{NULL}};
static MICRO_ARCH Arch_Atom_Avoton[]	= {{"Atom/Avoton"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Airmont[]	= {{"Atom/Airmont"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Goldmont[]	= {{"Atom/Goldmont"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Sofia[]	= {{"Atom/Sofia"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Merrifield[]= {{"Atom/Merrifield"}	, {NULL}};
static MICRO_ARCH Arch_Atom_Moorefield[]= {{"Atom/Moorefield"}	, {NULL}};
static MICRO_ARCH Arch_Nehalem_Bloomfield[]={{"Nehalem/Bloomfield"},{NULL}};

enum {
	CN_LYNNFIELD,
	CN_CLARKSFIELD
};

static MICRO_ARCH Arch_Nehalem_Lynnfield[] = {
	[CN_LYNNFIELD]		= {"Nehalem/Lynnfield"},
	[CN_CLARKSFIELD]	= {"Nehalem/Clarksfield"},
	{NULL}
};

static MICRO_ARCH Arch_Nehalem_MB[]	= {{"Nehalem/Mobile"}	, {NULL}};
static MICRO_ARCH Arch_Nehalem_EX[]	= {{"Nehalem/eXtreme.EP"},{NULL}};
static MICRO_ARCH Arch_Westmere[]	= {{"Westmere"} 	, {NULL}};

enum {
	CN_WESTMERE_EP,
	CN_GULFTOWN
};

static MICRO_ARCH Arch_Westmere_EP[] = {
	[CN_WESTMERE_EP]	= {"Westmere/EP"},
	[CN_GULFTOWN]		= {"Westmere/Gulftown"},
	{NULL}
};

static MICRO_ARCH Arch_Westmere_EX[]	= {{"Westmere/eXtreme"} , {NULL}};
static MICRO_ARCH Arch_SandyBridge[]	= {{"SandyBridge"}	, {NULL}};

enum {
	CN_SANDYBRIDGE_EP,
	CN_SNB_ROMLEY_EP,
	CN_SNB_EXTREME
};

static MICRO_ARCH Arch_SandyBridge_EP[] = {
	[CN_SANDYBRIDGE_EP]	= {"SandyBridge/EP"},
	[CN_SNB_ROMLEY_EP]	= {"SandyBridge/EP/Romley"},
	[CN_SNB_EXTREME]	= {"SandyBridge/eXtreme"},
	{NULL}
};

static MICRO_ARCH Arch_IvyBridge[] = {{"IvyBridge"}, {NULL}};

enum {
	CN_IVYBRIDGE_EP,
	CN_IVB_ROMLEY_EP,
	CN_IVB_EXTREME
};

static MICRO_ARCH Arch_IvyBridge_EP[]	= {
	[CN_IVYBRIDGE_EP]	= {"IvyBridge/EP"},
	[CN_IVB_ROMLEY_EP]	= {"IvyBridge/EP/Romley"},
	[CN_IVB_EXTREME]	= {"IvyBridge/eXtreme"},
	{NULL}
};

enum {
	CN_HASWELL_DESKTOP,
	CN_HASWELL_MOBILE_EX,
	CN_HASWELL_CRYSTALWELL,
	CN_HASWELL_CANYON,
	CN_HASWELL_DENLOW,
	CN_HASWELL_EMBEDDED,
	CN_HASWELL_MOBILE
};

static MICRO_ARCH Arch_Haswell_DT[] = {
	[CN_HASWELL_DESKTOP]	= {"Haswell/Desktop"},
	[CN_HASWELL_MOBILE_EX]	= {"Haswell/Mobile/eXtreme"},
	[CN_HASWELL_CRYSTALWELL]= {"Haswell/Crystal Well"},
	[CN_HASWELL_CANYON]	= {"Haswell/Canyon"},
	[CN_HASWELL_DENLOW]	= {"Haswell/Denlow"},
	[CN_HASWELL_EMBEDDED]	= {"Haswell/Embedded"},
	[CN_HASWELL_MOBILE]	= {"Haswell/Mobile"},
	{NULL}
};

enum {
	CN_HASWELL_EP,
	CN_HSW_GRANTLEY_EP,
	CN_HSW_EXTREME
};

static MICRO_ARCH Arch_Haswell_EP[]	= {
	[CN_HASWELL_EP] 	= {"Haswell/EP"},
	[CN_HSW_GRANTLEY_EP]	= {"Haswell/EP/Grantley"},
	[CN_HSW_EXTREME]	= {"Haswell/eXtreme"},
	{NULL}
};

static MICRO_ARCH Arch_Haswell_ULT[]	= {{"Haswell/Ultra Low TDP"},{NULL}};
static MICRO_ARCH Arch_Haswell_ULX[]	={{"Haswell/Ultra Low eXtreme"},{NULL}};
static MICRO_ARCH Arch_Broadwell[]	= {{"Broadwell/Mobile"} , {NULL}};
static MICRO_ARCH Arch_Broadwell_D[]	= {{"Broadwell/D"}	, {NULL}};
static MICRO_ARCH Arch_Broadwell_H[]	= {{"Broadwell/H"}	, {NULL}};

enum {
	CN_BROADWELL_EP,
	CN_BDW_GRANTLEY_EP,
	CN_BDW_EXTREME
};
static MICRO_ARCH Arch_Broadwell_EP[]	= {
	[CN_BROADWELL_EP]	= {"Broadwell/EP"},
	[CN_BDW_GRANTLEY_EP]	= {"Broadwell/EP/Grantley"},
	[CN_BDW_EXTREME]	= {"Broadwell/eXtreme"},
	{NULL}
};

static MICRO_ARCH Arch_Skylake_UY[]	= {{"Skylake/UY"}	, {NULL}};
static MICRO_ARCH Arch_Skylake_S[]	= {{"Skylake/S"}	, {NULL}};

enum {
	CN_SKYLAKE_X,
	CN_CASCADELAKE_X
};

static MICRO_ARCH Arch_Skylake_X[] = {
	[CN_SKYLAKE_X]		= {"Skylake/X"},
	[CN_CASCADELAKE_X]	= {"Cascade Lake/X"},
	{NULL}
};

static MICRO_ARCH Arch_Xeon_Phi[] = {{"Knights Landing"}, {NULL}};

enum {
	CN_KABYLAKE,
	CN_KABYLAKE_S,
	CN_KABYLAKE_X,
	CN_KABYLAKE_H,
	CN_KABYLAKE_U,
	CN_KABYLAKE_Y,
	CN_KABYLAKE_R,
	CN_KABYLAKE_G,
	CN_COFFEELAKE_S,
	CN_COFFEELAKE_H,
	CN_COFFEELAKE_U,
	CN_COFFEELAKE_R,
	CN_COFFEELAKE_HR
};

static MICRO_ARCH Arch_Kabylake[] = {
	[CN_KABYLAKE]		= {"Kaby Lake"},
	[CN_KABYLAKE_S] 	= {"Kaby Lake/S"},
	[CN_KABYLAKE_X] 	= {"Kaby Lake/X"},
	[CN_KABYLAKE_H] 	= {"Kaby Lake/H"},
	[CN_KABYLAKE_U] 	= {"Kaby Lake/U"},
	[CN_KABYLAKE_Y] 	= {"Kaby Lake/Y"},
	[CN_KABYLAKE_R] 	= {"Kaby Lake/R"},
	[CN_KABYLAKE_G] 	= {"Kaby Lake/G"},
	[CN_COFFEELAKE_S]	= {"Coffee Lake/S"},
	[CN_COFFEELAKE_H]	= {"Coffee Lake/H"},
	[CN_COFFEELAKE_U]	= {"Coffee Lake/U"},
	[CN_COFFEELAKE_R]	= {"Coffee Lake/R"},
	[CN_COFFEELAKE_HR]	= {"Coffee Lake/HR"},
	{NULL}
};

enum {
	CN_KABYLAKE_UY,
	CN_WHISKEYLAKE_U,
	CN_AMBERLAKE_Y,
	CN_COMETLAKE_H,
	CN_COMETLAKE_U
};

static MICRO_ARCH Arch_Kabylake_UY[] = {
	[CN_KABYLAKE_UY]	= {"Kaby Lake/UY"},
	[CN_WHISKEYLAKE_U]	= {"Whiskey Lake/U"},
	[CN_AMBERLAKE_Y]	= {"Amber Lake/Y"},
	[CN_COMETLAKE_H]	= {"Comet Lake/H"},
	[CN_COMETLAKE_U]	= {"Comet Lake/U"},
	{NULL}
};

static MICRO_ARCH Arch_Cannonlake[]	= {{"Cannon Lake"}	, {NULL}};
static MICRO_ARCH Arch_Geminilake[]	= {{"Atom/Gemini Lake"} , {NULL}};
static MICRO_ARCH Arch_Icelake[]	= {{"Ice Lake"} 	, {NULL}};
static MICRO_ARCH Arch_Icelake_UY[]	= {{"Ice Lake/UY"}	, {NULL}};
static MICRO_ARCH Arch_Icelake_X[]	= {{"Ice Lake/X"}	, {NULL}};
static MICRO_ARCH Arch_Icelake_D[]	= {{"Ice Lake/D"}	, {NULL}};
static MICRO_ARCH Arch_Sunny_Cove[]	= {{"Sunny Cove"}	, {NULL}};
static MICRO_ARCH Arch_Tigerlake[]	= {{"Tiger Lake"}	, {NULL}};
static MICRO_ARCH Arch_Tigerlake_U[]	= {{"Tiger Lake/U"}	, {NULL}};
static MICRO_ARCH Arch_Cometlake[]	= {{"Comet Lake"}	, {NULL}};
static MICRO_ARCH Arch_Cometlake_UY[]	= {{"Comet Lake/UY"}	, {NULL}};

static MICRO_ARCH Arch_Atom_C3000[]	= {{"Atom/C3000"}	, {NULL}};

static MICRO_ARCH Arch_Tremont_Jacobsville[]={{"Tremont/Jacobsville"} ,{NULL}};
static MICRO_ARCH Arch_Tremont_Lakefield[]  ={{"Tremont/Lakefield"}   ,{NULL}};
static MICRO_ARCH Arch_Tremont_Elkhartlake[]={{"Tremont/Elkhart Lake"},{NULL}};
static MICRO_ARCH Arch_Tremont_Jasperlake[] ={{"Tremont/Jasper Lake"} ,{NULL}};

enum {
	CN_BULLDOZER,
	CN_PILEDRIVER,
	CN_STEAMROLLER,
	CN_EXCAVATOR
};

static MICRO_ARCH Arch_AMD_Family_0Fh[] = {{"Hammer"}	, {NULL}};
static MICRO_ARCH Arch_AMD_Family_10h[] = {{"K10"}	, {NULL}};
static MICRO_ARCH Arch_AMD_Family_11h[] = {{"Turion"}	, {NULL}};
static MICRO_ARCH Arch_AMD_Family_12h[] = {{"Fusion"}	, {NULL}};
static MICRO_ARCH Arch_AMD_Family_14h[] = {{"Bobcat"}	, {NULL}};
static MICRO_ARCH Arch_AMD_Family_15h[] = {
	[CN_BULLDOZER]		= {"Bulldozer"},
	[CN_PILEDRIVER] 	= {"Bulldozer/Piledriver"},
	[CN_STEAMROLLER]	= {"Bulldozer/Steamroller"},
	[CN_EXCAVATOR]		= {"Bulldozer/Excavator"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Family_16h[] = {{"Jaguar"}, {NULL}};

enum {
	CN_SUMMIT_RIDGE,
	CN_WHITEHAVEN,
	CN_NAPLES
};
enum {
	CN_RAVEN_RIDGE,
	CN_SNOWY_OWL
};
enum {
	CN_PINNACLE_RIDGE,
	CN_COLFAX
};
enum {
	CN_PICASSO
};
enum {
	CN_RAVEN2
};
enum {
	CN_ROME
};
enum {
	CN_CASTLE_PEAK
};
enum {
	CN_RENOIR
};
enum {
	CN_MATISSE
};

static MICRO_ARCH Arch_AMD_Zen[] = {
	[CN_SUMMIT_RIDGE]	= {"Zen/Summit Ridge"},
	[CN_WHITEHAVEN] 	= {"Zen/Whitehaven"},
	[CN_NAPLES]		= {"EPYC/Naples"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Zen_APU[] = {
	[CN_RAVEN_RIDGE]	= {"Zen/Raven Ridge"},
	[CN_SNOWY_OWL]		= {"Zen/Snowy Owl"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_ZenPlus[] = {
	[CN_PINNACLE_RIDGE]	= {"Zen+ Pinnacle Ridge"},
	[CN_COLFAX]		= {"Zen+ Colfax"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_ZenPlus_APU[] = {
	[CN_PICASSO]		= {"Zen+ Picasso"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Zen_APU_Rv2[] = {
	[CN_RAVEN2]		= {"Zen/Raven 2"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_EPYC_Rome[] = {
	[CN_ROME]		= {"EPYC/Rome"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Zen2_CPK[] = {
	[CN_CASTLE_PEAK]	= {"Zen2/Castle Peak"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Zen2_APU[] = {
	[CN_RENOIR]		= {"Zen2/Renoir"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Zen2_MTS[] = {
	[CN_MATISSE]		= {"Zen2/Matisse"},
	{NULL}
};
static MICRO_ARCH Arch_AMD_Family_17h[] = {{"AMD Zen"}, {NULL}};

static MICRO_ARCH Arch_AMD_Family_18h[] = {{"Dhyana"}, {NULL}};

static PROCESSOR_SPECIFIC Void_Specific[] = { {0} };

static PROCESSOR_SPECIFIC Core_Penryn_Specific[] = {
/* Yorkfield
		06_17h		"Intel(R) Core(TM)2 Quad CPU Q8400"
		''		"Intel(R) Core(TM)2 Quad CPU Q9400"
		''		"Intel(R) Core(TM)2 Quad CPU Q9450"
		''		"Intel(R) Core(TM)2 Quad CPU Q9550"
		''		"Intel(R) Core(TM)2 Quad CPU Q9650"
		''		Core 2 Quad Q9700
		''		Core 2 Quad Q9705
		''		"Intel(R) Core(TM)2 Extreme CPU X9650"
		''		"Intel(R) Core(TM)2 Extreme CPU X9770"
		''		"Intel(R) Core(TM)2 Extreme CPU X9775"
*/
	{
	.Brand = ZLIST( "Intel(R) Core(TM)2 Extreme CPU X97",	\
			"Intel(R) Core(TM)2 Extreme CPU X96",	\
			"Intel(R) Core(TM)2 Quad CPU Q96",	\
			"Intel(R) Core(TM)2 Quad CPU Q95",	\
			"Intel(R) Core(TM)2 Quad CPU Q94",	\
			"Intel(R) Core(TM)2 Quad CPU Q84"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
/* Wolfdale
	06_17h			"Intel(R) Celeron(R) CPU E3200"
				"Intel(R) Celeron(R) CPU E3300"
				"Intel(R) Celeron(R) CPU E3400"
				"Intel(R) Celeron(R) CPU E3500"
				"Pentium(R) Dual-Core CPU E5200"
				"Pentium(R) Dual-Core CPU E5300"
				"Pentium(R) Dual-Core CPU E5400"
				"Pentium(R) Dual-Core CPU E5500"
				"Pentium(R) Dual-Core CPU E5700"
				"Pentium(R) Dual-Core CPU E5800"
				"Pentium(R) Dual-Core CPU E6300"
				"Pentium(R) Dual-Core CPU E6500"
				"Pentium(R) Dual-Core CPU E6600"
				"Pentium(R) Dual-Core CPU E6700"
				"Pentium(R) Dual-Core CPU E6800"
*/
	{
	.Brand = ZLIST( "Pentium(R) Dual-Core CPU E6",	\
			"Pentium(R) Dual-Core CPU E5",	\
			"Intel(R) Celeron(R) CPU E3"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_WOLFDALE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
/* Penryn
	06_17h			Celeron M 722
				Celeron M 723
				Celeron M 743
				Celeron M 763
				Core 2 Solo SU3300
	06_17h Stepping[0Ah]	"Intel(R) Core(TM)2 Solo CPU U3500"
		''		"Genuine Intel(R) CPU U2300"
		''		"Genuine Intel(R) CPU U2700"
		''		"Genuine Intel(R) CPU U4100"
		''		"Celeron(R) Dual-Core CPU T3100"
		''		"Celeron(R) Dual-Core CPU T3300"
		''		"Celeron(R) Dual-Core CPU T3500"
		''		"Pentium(R) Dual-Core CPU T4200"
		''		"Pentium(R) Dual-Core CPU T4300"
		''		"Pentium(R) Dual-Core CPU T4400"
		''		"Pentium(R) Dual-Core CPU T4500"
		''		"Intel(R) Core(TM)2 Duo CPU P8400"
		''		"Intel(R) Core(TM)2 Duo CPU P8600"
		''		"Intel(R) Core(TM)2 Duo CPU P8800"
		''		"Intel(R) Core(TM)2 Duo CPU T9550"
		''		"Intel(R) Core(TM)2 Duo CPU T9600"
		''		"Intel(R) Core(TM)2 Duo CPU T9900"
	06_17h Stepping[06h]	"Intel(R) Core(TM)2 Duo CPU T9300"
		''		"Intel(R) Core(TM)2 Duo CPU T9400"
		''		"Intel(R) Core(TM)2 Duo CPU T9500"
		''		"Intel(R) Core(TM)2 Duo CPU P9300"
		''		"Intel(R) Core(TM)2 Duo CPU P9400"
		''		Core 2 Duo SP9600
		''		"Intel(R) Core(TM)2 Extreme CPU X9000"
	06_17h Stepping[0Ah]	"Intel(R) Core(TM)2 Extreme CPU X9100"
		''		"Intel(R) Core(TM)2 Quad CPU Q9000"
		''		"Intel(R) Core(TM)2 Quad CPU Q9100"
		''		"Intel(R) Core(TM)2 Extreme CPU Q9300"
*/
	{
	.Brand = ZLIST("Intel(R) Core(TM)2 Extreme CPU Q9300"),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PENRYN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Silvermont_Bay_Trail_Specific[] = {
	{
	.Brand = ZLIST("Intel(R) Celeron(R) CPU N29"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = 0,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Nehalem_Bloomfield_Specific[] = {
	{
	.Brand = ZLIST("Intel(R) Core(TM) i7 CPU 920"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = 0,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_TURBO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Nehalem_Lynnfield_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7 CPU X",	\
			"Intel(R) Core(TM) i7 CPU Q"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CLARKSFIELD,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Westmere_EP_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7 CPU X 990",	\
			"Intel(R) Core(TM) i7 CPU X 980",	\
			"Intel(R) Core(TM) i7 CPU 980" ,	\
			"Intel(R) Core(TM) i7 CPU 970"		),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("Intel(R) Xeon(R) CPU W3690"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC SandyBridge_EP_Specific[] = {
	{
	.Brand = ZLIST("Intel(R) Xeon(R) CPU E5-26"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SNB_ROMLEY_EP,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-3970X",	\
			"Intel(R) Core(TM) i7-3960X",	\
			"Intel(R) Core(TM) i7-3930K",	\
			"Intel(R) Core(TM) i7-3820"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SNB_EXTREME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC IvyBridge_EP_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Xeon(R) CPU E5-1650 v2",	\
			"Intel(R) Xeon(R) CPU E5-1660 v2",	\
			"Intel(R) Xeon(R) CPU E5-1680 v2"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_IVYBRIDGE_EP,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_TURBO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Xeon(R) CPU E5-26",	\
			"CPU E2697V"			),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_IVB_ROMLEY_EP,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-4960X",	\
			"Intel(R) Core(TM) i7-4930K",	\
			"Intel(R) Core(TM) i7-4820K"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_IVB_EXTREME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Haswell_DT_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-4940MX",	\
			"Intel(R) Core(TM) i7-4930MX"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE_EX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-4910MQ",	\
			"Intel(R) Core(TM) i7-4900MQ",	\
			"Intel(R) Core(TM) i7-4810MQ",	\
			"Intel(R) Core(TM) i7-4800MQ",	\
			"Intel(R) Core(TM) i7-4710HQ",	\
			"Intel(R) Core(TM) i7-4700HQ"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-47",	\
			"Intel(R) Core(TM) i5-46"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_CANYON,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("Intel(R) Xeon(R) CPU E3-12"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_DENLOW,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("Intel(R) 4th Gen Core(TM) i"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_EMBEDDED,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i5-4300M",	\
			"Intel(R) Core(TM) i5-4200H",	\
			"Intel(R) Core(TM) i3-4000M",	\
			"Intel(R) Pentium(R) CPU 3",	\
			"Intel(R) Celeron(R) CPU 2"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Haswell_EP_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Xeon(R) CPU E5-1650 v3",	\
			"Intel(R) Xeon(R) CPU E5-1660 v3",	\
			"Intel(R) Xeon(R) CPU E5-1680 v3"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HASWELL_EP,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_TURBO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST("Intel(R) Xeon(R) CPU E5-26"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HSW_GRANTLEY_EP,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-5960X",	\
			"Intel(R) Core(TM) i7-5930K",	\
			"Intel(R) Core(TM) i7-5820K"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_HSW_EXTREME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Broadwell_EP_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Xeon(R) CPU E5-26",	\
			"Intel(R) Xeon(R) CPU E5-46"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_BDW_GRANTLEY_EP,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-6950X",	\
			"Intel(R) Core(TM) i7-6900K",	\
			"Intel(R) Core(TM) i7-6850K",	\
			"Intel(R) Core(TM) i7-6800K"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_BDW_EXTREME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Skylake_X_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i9-10980XE", \
			"Intel(R) Core(TM) i9-10940X" , \
			"Intel(R) Core(TM) i9-10920X" , \
			"Intel(R) Core(TM) i9-10900X" , \
			"Intel(R) Xeon(R) Platinum 92", \
			"Intel(R) Xeon(R) Platinum 82", \
			"Intel(R) Xeon(R) Gold 62",	\
			"Intel(R) Xeon(R) Gold 52",	\
			"Intel(R) Xeon(R) Silver 42",	\
			"Intel(R) Xeon(R) Bronze 32",	\
			"Intel(R) Xeon(R) W-32",	\
			"Intel(R) Xeon(R) W-22" 	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASCADELAKE_X,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Kabylake_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-7920HQ",	\
			"Intel(R) Core(TM) i7-7820HQ",	\
			"Intel(R) Core(TM) i7-7700HQ",	\
			"Intel(R) Core(TM) i5-7440HQ",	\
			"Intel(R) Core(TM) i5-7300HQ",	\
			"Intel(R) Core(TM) i3-7100H"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_H,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-7740X",	\
			"Intel(R) Core(TM) i5-7640X"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_X,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-766",	\
			"Intel(R) Core(TM) i7-760",	\
			"Intel(R) Core(TM) i7-756",	\
			"Intel(R) Core(TM) i7-750"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-770"	\
			"Intel(R) Core(TM) i5-760",	\
			"Intel(R) Core(TM) i5-750",	\
			"Intel(R) Core(TM) i5-740"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_S,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i5-736",	\
			"Intel(R) Core(TM) i5-730",	\
			"Intel(R) Core(TM) i5-728",	\
			"Intel(R) Core(TM) i5-726",	\
			"Intel(R) Core(TM) i5-720"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-7Y7",	\
			"Intel(R) Core(TM) i5-7Y5",	\
			"Intel(R) Core(TM) m3-7Y3"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_Y,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i3-7167U",	\
			"Intel(R) Core(TM) i3-7130U",	\
			"Intel(R) Core(TM) i3-7100U",	\
			"Intel(R) Core(TM) i3-7020U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i3-735",	\
			"Intel(R) Core(TM) i3-732",	\
			"Intel(R) Core(TM) i3-730",	\
			"Intel(R) Core(TM) i3-710"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_S,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-8650U",	\
			"Intel(R) Core(TM) i7-8550U",	\
			"Intel(R) Core(TM) i5-8350U",	\
			"Intel(R) Core(TM) i5-8250U",	\
			"Intel(R) Core(TM) i3-8130U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-8809G",	\
			"Intel(R) Core(TM) i7-8709G",	\
			"Intel(R) Core(TM) i7-8706G",	\
			"Intel(R) Core(TM) i7-8705G",	\
			"Intel(R) Core(TM) i7-8305G"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_G,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Pentium(R) CPU G46",	\
			"Intel(R) Pentium(R) CPU G45",	\
			"Intel(R) Celeron(R) CPU G39"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_S,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Pentium(R) Processor 44",	\
			"Intel(R) Celeron(R) Processor 3965Y"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_Y,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Celeron(R) Processor 3965U",	\
			"Intel(R) Celeron(R) Processor 3865U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Pentium(R) Gold G56", \
			"Intel(R) Pentium(R) Gold G55", \
			"Intel(R) Pentium(R) Gold G54", \
			"Intel(R) Celeron(R) G49"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i9-8950H",	\
			"Intel(R) Core(TM) i7-8850H",	\
			"Intel(R) Core(TM) i7-8750H",	\
			"Intel(R) Core(TM) i7-8700B",	\
			"Intel(R) Core(TM) i5-8500B",	\
			"Intel(R) Core(TM) i5-8400B",	\
			"Intel(R) Core(TM) i5-8400H",	\
			"Intel(R) Core(TM) i5-8300H",	\
			"Intel(R) Core(TM) i3-8100B",	\
			"Intel(R) Core(TM) i3-8100H"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_H,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-8086K",	\
			"Intel(R) Core(TM) i7-8700",	\
			"Intel(R) Core(TM) i5-8600",	\
			"Intel(R) Core(TM) i5-8500",	\
			"Intel(R) Core(TM) i5-8400",	\
			"Intel(R) Core(TM) i3-8350K",	\
			"Intel(R) Core(TM) i3-8300",	\
			"Intel(R) Core(TM) i3-8100"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-8569U",	\
			"Intel(R) Core(TM) i7-8559U",	\
			"Intel(R) Core(TM) i7-8557U",	\
			"Intel(R) Core(TM) i5-8279U",	\
			"Intel(R) Core(TM) i5-8269U",	\
			"Intel(R) Core(TM) i5-8259U",	\
			"Intel(R) Core(TM) i5-8257U",	\
			"Intel(R) Core(TM) i3-8109U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i9-9980H",	\
			"Intel(R) Core(TM) i9-9880H",	\
			"Intel(R) Core(TM) i7-9850H",	\
			"Intel(R) Core(TM) i7-9750H",	\
			"Intel(R) Core(TM) i5-9400H",	\
			"Intel(R) Core(TM) i5-9300H"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_HR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i9-990",	\
			"Intel(R) Core(TM) i7-970",	\
			"Intel(R) Core(TM) i5-960",	\
			"Intel(R) Core(TM) i5-950",	\
			"Intel(R) Core(TM) i5-940",	\
			"Intel(R) Core(TM) i3-935",	\
			"Intel(R) Core(TM) i3-932",	\
			"Intel(R) Core(TM) i3-930",	\
			"Intel(R) Core(TM) i3-910"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Kabylake_UY_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-8665U",	\
			"Intel(R) Core(TM) i7-8565U",	\
			"Intel(R) Core(TM) i5-8365U",	\
			"Intel(R) Core(TM) i5-8265U",	\
			"Intel(R) Core(TM) i3-8145U",	\
			"Intel(R) Pentium(R) CPU 5405U",\
			"Intel(R) Celeron(R) CPU 4305U",\
			"Intel(R) Celeron(R) CPU 4205U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_WHISKEYLAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-10510Y",	\
			"Intel(R) Core(TM) i5-10310Y",	\
			"Intel(R) Core(TM) i5-10210Y",	\
			"Intel(R) Core(TM) i3-10110Y",	\
			"Intel(R) Core(TM) i7-8500Y" ,	\
			"Intel(R) Core(TM) i5-8310Y" ,	\
			"Intel(R) Core(TM) i5-8210Y" ,	\
			"Intel(R) Core(TM) i5-8200Y" ,	\
			"Intel(R) Core(TM) m3-8100Y"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_AMBERLAKE_Y,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i9-10980HK", \
			"Intel(R) Core(TM) i7-10875H",	\
			"Intel(R) Core(TM) i7-10850H",	\
			"Intel(R) Core(TM) i7-10750H",	\
			"Intel(R) Core(TM) i5-10400H",	\
			"Intel(R) Core(TM) i5-10300H"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COMETLAKE_H,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Core(TM) i7-10710U",	\
			"Intel(R) Core(TM) i7-10510U",	\
			"Intel(R) Core(TM) i5-10210U",	\
			"Intel(R) Core(TM) i3-10110U",	\
			"Intel(R) Pentium(R) Gold 6405U",\
			"Intel(R) Celeron(R) CPU 5205U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COMETLAKE_U,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{0}
};

/*	AMD Family 17h
	Remarks:Thermal Offset taken from Linux/k10temp.c
		+0.5 XFR is rounded to +1 multiplier bin
*/
static PROCESSOR_SPECIFIC AMD_Zen_Specific[] = {
/*	[Zen/Summit Ridge]	8F_01h Stepping 1			*/
	{
	.Brand = ZLIST("AMD Ryzen Embedded V"),
	.Boost = {+16, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Silver PRO 3125GE"),
	.Boost = {+0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Gold PRO 3150GE"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Gold PRO 3150G"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen Embedded R", \
			"AMD Athlon Silver",	\
			"AMD Athlon Gold"	),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 1200"),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 1300", \
			"AMD Ryzen 5 PRO 1500"	),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 1600"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 1700X"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 20},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 1700"),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 1300X",	\
			"AMD Ryzen 5 1500X"	),
	.Boost = {+2, +2},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 1600X",	\
			"AMD Ryzen 7 1700X",	\
			"AMD Ryzen 7 1800X"	),
	.Boost = {+4, +1},
	.Param.Offset = {0, 0, 20},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 1200"),
	.Boost = {+3, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 1400"),
	.Boost = {+2, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 1600"),
	.Boost = {+4, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 1700"),
	.Boost = {+7, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
/*	[Zen/Whitehaven]	8F_01h Stepping 1			*/
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 1950X"),
	.Boost = {+6, +2},
	.Param.Offset = {0, 0, 27},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 1920X"),
	.Boost = {+5, +2},
	.Param.Offset = {0, 0, 27},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 1900X"),
	.Boost = {+2, +2},
	.Param.Offset = {0, 0, 27},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
/*	[EPYC/Naples]		8F_01h Stepping 2			*/
	{	/* AMD EPYC Server Processors				*/
	.Brand = ZLIST("AMD EPYC 7251"),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7261"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7281"),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7351P",	\
			"AMD EPYC 7351",	\
			"AMD EPYC 7301" 	),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7401P",	\
			"AMD EPYC 7401" 	),
	.Boost = {+8, +2},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7451"),
	.Boost = {+6, +3},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7551P",	\
			"AMD EPYC 7551",	\
			"AMD EPYC 7501" 	),
	.Boost = {+6, +4},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7601"),
	.Boost = {+5, +5},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen_APU_Specific[] = {
/*	[Zen/Raven Ridge]	8F_11h Stepping 0			*/
	{
	.Brand = ZLIST( "AMD Athlon 240GE",	\
			"AMD Athlon 220GE",	\
			"AMD Athlon 200GE"	),
	.Boost = {+0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 2200GE",	\
			"AMD Ryzen 5 2600H"		),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 2200G"),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 2200GE"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 2200G"),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 2400GE"),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 2400G"),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 2400GE"),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 2400G"),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 2500U"),
	.Boost = {+16, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 2800H"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 2700U"),
	.Boost = {+16, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
/*	[Zen/Snowy Owl]		8F_11h Stepping 0			*/
	{	/* AMD EPYC Embedded Processors 			*/
	.Brand = ZLIST( "AMD EPYC 31",	\
			"AMD EPYC 32"	),
	.Boost = {+0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_ZenPlus_Specific[] = {
/*	[Zen+ Pinnacle Ridge] 	8F_08h Stepping 2			*/
	{
	.Brand = ZLIST("AMD Athlon 3000G"),
	.Boost = {+5, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon 300U"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon 3"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon PRO"),
	.Boost = {+5, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 2300X",	\
			"AMD Ryzen 5 2500X"	),
	.Boost = {+4, +1},
	.Param.Offset = {0, 0, 10},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 2600E"),
	.Boost = {+8, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 2600"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 2600"),
	.Boost = {+3, +2},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 2700X"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 10},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 2700"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 2700X",	\
			"AMD Ryzen 5 2600X"	),
	.Boost = {+5, +2},
	.Param.Offset = {0, 0, 10},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 2700E"),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 2700"),
	.Boost = {+8, +2},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
/*	[Zen+ Colfax]		8F_08h Stepping 2			*/
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper 2990",	\
			"AMD Ryzen Threadripper 2970"	),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 27},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 2950"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 27},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 2920"),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 27},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_ZenPlus_APU_Specific[] = {
/*	[Zen+ Picasso]		8F_18h Stepping 1			*/
	{
	.Brand = ZLIST("AMD Athlon PRO 300GE"),
	.Boost = {+0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 3400GE"),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 3400G",	\
			"AMD Ryzen 3 PRO 3200GE"	),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 3200G"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 3400GE"),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 3200GE",	\
			"AMD Ryzen 5 3400G"	),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 3200G"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 3580U",	\
			"AMD Ryzen 5 3550H",	\
			"AMD Ryzen 5 3500U"	),
	.Boost = {+16, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 3780U",	\
			"AMD Ryzen 7 3750H",	\
			"AMD Ryzen 7 3700U"	),
	.Boost = {+17, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen_APU_Rv2_Specific[] = {
	{
	.Brand = ZLIST( "AMD Athlon",	\
			"AMD Ryzen 3"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RAVEN2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_EPYC_Rome_Specific[] = {
/*	[EPYC/Rome]		8F_30h Stepping 0			*/
	{
	.Brand = ZLIST( "AMD EPYC 7232P",	\
			"AMD EPYC 7252" 	),
	.Boost = {+1, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7262",	\
			"AMD EPYC 7F32" 	),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7272"),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7282",	\
			"AMD EPYC 7F52" 	),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7302P",	\
			"AMD EPYC 7302" 	),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7402P",	\
			"AMD EPYC 7402" 	),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7352",	\
			"AMD EPYC 7502P",	\
			"AMD EPYC 7502" 	),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7542",	\
			"AMD EPYC 7F72" 	),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7552"),
	.Boost = {+11, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7452",	\
			"AMD EPYC 7642" 	),
	.Boost = {+10, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7662"),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC 7702P",	\
			"AMD EPYC 7702" 	),
	.Boost = {+14, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7742"),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 7H12"),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_CPK_Specific[] = {
/*	[Zen2/Castle Peak]	8F_31h Stepping 0			*/
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 3990X"),
	.Boost = {+14, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 3970X"),
	.Boost = {+8, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 3960X"),
	.Boost = {+7, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3995WX"),
	.Boost = {+15, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3975WX"),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3955WX"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3945WX"),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_APU_Specific[] = {
/*	[Zen2/Renoir]		8F_60h Stepping 1			*/
	{
	.Brand = ZLIST( "AMD Ryzen 3 4300U",	\
			"AMD Ryzen 5 4600H"	),
	.Boost = {+10, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 4350GE"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 4350G"),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 4600GE"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 4300GE",	\
			"AMD Ryzen 5 4600G"	),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 4300G"),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 4500U"),
	.Boost = {+17, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 4650GE"),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 4650G"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 4650U", \
			"AMD Ryzen 5 4600U"	),
	.Boost = {+19, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 4700U"),
	.Boost = {+21, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST(	"AMD Ryzen 7 PRO 4750U", \
			"AMD Ryzen 7 4800U"	),
	.Boost = {+24, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 4800H",	\
			"AMD Ryzen 9 4900HS"	),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 4450U", \
			"AMD Ryzen 7 PRO 4750GE" ),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 4700GE"),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 4750G"),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 4700G"),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 4900U"),
	.Boost = {+24, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 4900H"),
	.Boost = {+11, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_MTS_Specific[] = {
/*	[Zen2/Matisse]		8F_71h Stepping 0			*/
	{
	.Brand = ZLIST("AMD Ryzen 3 3100"),
	.Boost = {+3, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 3600"),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 3600XT"),
	.Boost = {+6, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 3600X",	\
			"AMD Ryzen 5 3600"	),
	.Boost = {+5, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 3700"),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 3800XT"),
	.Boost = {+7, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 3300X",	\
			"AMD Ryzen 5 3500X",	\
			"AMD Ryzen 7 3800X"	),
	.Boost = {+5, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 PRO 3900"),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 3900XT"),
	.Boost = {+8, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 3700X",	\
			"AMD Ryzen 9 3900X"	),
	.Boost = {+7, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 3950X"),
	.Boost = {+11, +1},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Misc_Specific_Processor[] = {
	{
	.Brand = ZLIST("AMD EPYC Processor"),	/* QEMU virtualized EPYC */
	.Boost = {+1, +1},	/* Add one multiplier to each ratio 1C, 2C */
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = 1,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};

#ifdef CONFIG_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 19)
#define CPUFREQ_POLICY_UNKNOWN		(0)
#endif
static int CoreFreqK_Policy_Exit(struct cpufreq_policy*) ;
static int CoreFreqK_Policy_Init(struct cpufreq_policy*) ;
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 19))   \
  && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 5, 0)))  \
  || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 3))
static int CoreFreqK_Policy_Verify(struct cpufreq_policy_data*) ;
#else
static int CoreFreqK_Policy_Verify(struct cpufreq_policy*) ;
#endif
static int CoreFreqK_SetPolicy(struct cpufreq_policy*) ;
static int CoreFreqK_Bios_Limit(int, unsigned int*) ;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
static int CoreFreqK_SetBoost(struct cpufreq_policy*, int) ;
#else
static int CoreFreqK_SetBoost(int) ;
#endif
static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy*, char*);
static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy*, unsigned int) ;
#endif /* CONFIG_CPU_FREQ */
static unsigned int Policy_GetFreq(unsigned int cpu) ;
static void Policy_Core2_SetTarget(void *arg) ;
static void Policy_Nehalem_SetTarget(void *arg) ;
static void Policy_SandyBridge_SetTarget(void *arg) ;
static void Policy_HWP_SetTarget(void *arg) ;
#define Policy_Broadwell_EP_SetTarget	Policy_HWP_SetTarget
#define Policy_Skylake_SetTarget	Policy_HWP_SetTarget
static void Policy_Zen_SetTarget(void *arg) ;

#define VOID_Driver {							\
	.IdleState	= NULL,						\
	.GetFreq	= NULL,						\
	.SetTarget	= NULL						\
}

#define CORE2_Driver {							\
	.IdleState	= NULL,						\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Core2_SetTarget			\
}

static IDLE_STATE SLM_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "SLM-C1",
	.flags		= 0x00 << 24,
	.Latency	= 1,
	.Residency	= 1
	},
	{
	.Name		= "C6N",
	.Desc		= "SLM-C6N",
	.flags		= (0x58 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 300,
	.Residency	= 275
	},
	{
	.Name		= "C6S",
	.Desc		= "SLM-C6S",
	.flags		= (0x52 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 500,
	.Residency	= 560
	},
	{
	.Name		= "C7",
	.Desc		= "SLM-C7",
	.flags		= (0x60 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 1200,
	.Residency	= 4000
	},
	{
	.Name		= "C7S",
	.Desc		= "SLM-C7S",
	.flags		= (0x64 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 10000,
	.Residency	= 20000
	},
	{NULL}
};

#define SLM_Driver {							\
	.IdleState	= SLM_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Core2_SetTarget			\
}

/* Source: /drivers/idle/intel_idle.c					*/
static IDLE_STATE NHM_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "NHM-C1",
	.flags		= 0x00 << 24,
	.Latency	= 3,
	.Residency	= 6
	},
	{
	.Name		= "C1E",
	.Desc		= "NHM-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C3",
	.Desc		= "NHM-C3",
	.flags		= (0x10 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 20,
	.Residency	= 80
	},
	{
	.Name		= "C6",
	.Desc		= "NHM-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 200,
	.Residency	= 800
	},
	{NULL}
};

#define NHM_Driver {							\
	.IdleState	= NHM_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Nehalem_SetTarget			\
}

static IDLE_STATE SNB_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "SNB-C1",
	.flags		= 0x00 << 24,
	.Latency	= 2,
	.Residency	= 2
	},
	{
	.Name		= "C1E",
	.Desc		= "SNB-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C3",
	.Desc		= "SNB-C3",
	.flags		= (0x10 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 80,
	.Residency	= 211
	},
	{
	.Name		= "C6",
	.Desc		= "SNB-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 104,
	.Residency	= 345
	},
	{
	.Name		= "C7",
	.Desc		= "SNB-C7",
	.flags		= (0x30 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 109,
	.Residency	= 345
	},
	{NULL}
};

#define SNB_Driver {							\
	.IdleState	= SNB_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_SandyBridge_SetTarget			\
}

static IDLE_STATE IVB_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "IVB-C1",
	.flags		= 0x00 << 24,
	.Latency	= 1,
	.Residency	= 1
	},
	{
	.Name		= "C1E",
	.Desc		= "IVB-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C3",
	.Desc		= "IVB-C3",
	.flags		= (0x10 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 59,
	.Residency	= 156
	},
	{
	.Name		= "C6",
	.Desc		= "IVB-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 80,
	.Residency	= 300
	},
	{
	.Name		= "C7",
	.Desc		= "IVB-C7",
	.flags		= (0x30 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 87,
	.Residency	= 300
	},
	{NULL}
};

#define IVB_Driver {							\
	.IdleState	= IVB_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_SandyBridge_SetTarget			\
}

static IDLE_STATE HSW_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "HSW-C1",
	.flags		= 0x00 << 24,
	.Latency	= 2,
	.Residency	= 2
	},
	{
	.Name		= "C1E",
	.Desc		= "HSW-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C3",
	.Desc		= "HSW-C3",
	.flags		= (0x10 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 33,
	.Residency	= 100
	},
	{
	.Name		= "C6",
	.Desc		= "HSW-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 133,
	.Residency	= 400
	},
	{
	.Name		= "C7",
	.Desc		= "HSW-C7",
	.flags		= (0x32 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 166,
	.Residency	= 500
	},
	{
	.Name		= "C8",
	.Desc		= "HSW-C8",
	.flags		= (0x40 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 300,
	.Residency	= 900
	},
	{
	.Name		= "C9",
	.Desc		= "HSW-C9",
	.flags		= (0x50 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 600,
	.Residency	= 1800
	},
	{
	.Name		= "C10",
	.Desc		= "HSW-C10",
	.flags		= (0x60 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 2600,
	.Residency	= 7700
	},
	{NULL}
};

#define HSW_Driver {							\
	.IdleState	= HSW_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_SandyBridge_SetTarget			\
}

static IDLE_STATE BDW_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "BDW-C1",
	.flags		= 0x00 << 24,
	.Latency	= 2,
	.Residency	= 2
	},
	{
	.Name		= "C1E",
	.Desc		= "BDW-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C3",
	.Desc		= "BDW-C3",
	.flags		= (0x10 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 40,
	.Residency	= 100
	},
	{
	.Name		= "C6",
	.Desc		= "BDW-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 133,
	.Residency	= 400
	},
	{
	.Name		= "C7",
	.Desc		= "BDW-C7",
	.flags		= (0x32 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 166,
	.Residency	= 500
	},
	{
	.Name		= "C8",
	.Desc		= "BDW-C8",
	.flags		= (0x40 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 300,
	.Residency	= 900
	},
	{
	.Name		= "C9",
	.Desc		= "BDW-C9",
	.flags		= (0x50 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 600,
	.Residency	= 1800
	},
	{
	.Name		= "C10",
	.Desc		= "BDW-C10",
	.flags		= (0x60 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 2600,
	.Residency	= 7700
	},
	{NULL}
};

#define BDW_Driver {							\
	.IdleState	= BDW_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_SandyBridge_SetTarget			\
}

#define BDW_EP_Driver {							\
	.IdleState	= BDW_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Broadwell_EP_SetTarget			\
}

static IDLE_STATE SKL_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "SKL-C1",
	.flags		= 0x00 << 24,
	.Latency	= 2,
	.Residency	= 2
	},
	{
	.Name		= "C1E",
	.Desc		= "SKL-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C3",
	.Desc		= "SKL-C3",
	.flags		= (0x10 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 70,
	.Residency	= 100
	},
	{
	.Name		= "C6",
	.Desc		= "SKL-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 85,
	.Residency	= 200
	},
	{
	.Name		= "C7",
	.Desc		= "SKL-C7",
	.flags		= (0x33 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 124,
	.Residency	= 800
	},
	{
	.Name		= "C8",
	.Desc		= "SKL-C8",
	.flags		= (0x40 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 200,
	.Residency	= 800
	},
	{
	.Name		= "C9",
	.Desc		= "SKL-C9",
	.flags		= (0x50 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 480,
	.Residency	= 5000
	},
	{
	.Name		= "C10",
	.Desc		= "SKL-C10",
	.flags		= (0x60 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 890,
	.Residency	= 5000
	},
	{NULL}
};

#define SKL_Driver {							\
	.IdleState	= SKL_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Skylake_SetTarget			\
}

static IDLE_STATE SKX_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "SKX-C1",
	.flags		= 0x00 << 24,
	.Latency	= 2,
	.Residency	= 2
	},
	{
	.Name		= "C1E",
	.Desc		= "SKX-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 10,
	.Residency	= 20
	},
	{
	.Name		= "C6",
	.Desc		= "SKX-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 133,
	.Residency	= 600
	},
	{NULL}
};

#define SKX_Driver {							\
	.IdleState	= SKX_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Skylake_SetTarget			\
}

#define Intel_Driver {							\
	.IdleState	= NULL,						\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Skylake_SetTarget			\
}

static IDLE_STATE Zen_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "ZEN-C1",
	.flags		= 0x00 << 24,
	.Latency	= 1,
	.Residency	= 2
	},
	{
	.Name		= "C2",
	.Desc		= "ZEN-C2",
	.flags		= 0x10 << 24,
	.Latency	= 100,
	.Residency	= 100 * 2
	},
	{
	.Name		= "C3",
	.Desc		= "ZEN-C3",
	.flags		= 0x20 << 24,
	.Latency	= 200,
	.Residency	= 200 * 2
	},
	{
	.Name		= "C4",
	.Desc		= "ZEN-C4",
	.flags		= 0x30 << 24,
	.Latency	= 600,
	.Residency	= 600 * 2
	},
	{
	.Name		= "C5",
	.Desc		= "ZEN-C5",
	.flags		= 0x40 << 24,
	.Latency	= 800,
	.Residency	= 800 * 2
	},
	{
	.Name		= "C6",
	.Desc		= "ZEN-C6",
	.flags		= 0x50 << 24,
	.Latency	= 1000,
	.Residency	= 1000 * 2
	},
	{NULL}
};

#define AMD_Zen_Driver {						\
	.IdleState	= Zen_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Zen_SetTarget				\
}

static ARCH Arch[ARCHITECTURES] = {
[GenuineArch] = {							/*  0*/
	.Signature = _Void_Signature,
	.Query = NULL,
	.Update = NULL,
	.Start = NULL,
	.Stop = NULL,
	.Exit = NULL,
	.Timer = NULL,
	.BaseClock = NULL,
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

[AMD_Family_0Fh] = {							/*  1*/
	.Signature = _AMD_Family_0Fh,
	.Query = Query_AMD_Family_0Fh,
	.Update = PerCore_AMD_Family_0Fh_Query,
	.Start = Start_AMD_Family_0Fh,
	.Stop = Stop_AMD_Family_0Fh,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_0Fh,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD_0Fh,
	.voltageFormula = VOLTAGE_FORMULA_AMD_0Fh,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_AMD_0Fh_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_0Fh
	},
[AMD_Family_10h] = {							/*  2*/
	.Signature = _AMD_Family_10h,
	.Query = Query_AMD_Family_10h,
	.Update = PerCore_AMD_Family_10h_Query,
	.Start = Start_AMD_Family_10h,
	.Stop = Stop_AMD_Family_10h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_10h,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_10h
	},
[AMD_Family_11h] = {							/*  3*/
	.Signature = _AMD_Family_11h,
	.Query = Query_AMD_Family_11h,
	.Update = PerCore_AMD_Family_11h_Query,
	.Start = Start_AMD_Family_11h,
	.Stop = Stop_AMD_Family_11h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_11h,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_11h
	},
[AMD_Family_12h] = {							/*  4*/
	.Signature = _AMD_Family_12h,
	.Query = Query_AMD_Family_12h,
	.Update = PerCore_AMD_Family_12h_Query,
	.Start = Start_AMD_Family_12h,
	.Stop = Stop_AMD_Family_12h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_12h,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_12h
	},
[AMD_Family_14h] = {							/*  5*/
	.Signature = _AMD_Family_14h,
	.Query = Query_AMD_Family_14h,
	.Update = PerCore_AMD_Family_14h_Query,
	.Start = Start_AMD_Family_14h,
	.Stop = Stop_AMD_Family_14h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_14h,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_14h
	},
[AMD_Family_15h] = {							/*  6*/
	.Signature = _AMD_Family_15h,
	.Query = Query_AMD_Family_15h,
	.Update = PerCore_AMD_Family_15h_Query,
	.Start = Start_AMD_Family_15h,
	.Stop = Stop_AMD_Family_15h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_15h,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD_15h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_15h,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_15h
	},
[AMD_Family_16h] = {							/*  7*/
	.Signature = _AMD_Family_16h,
	.Query = Query_AMD_Family_16h,
	.Update = PerCore_AMD_Family_16h_Query,
	.Start = Start_AMD_Family_16h,
	.Stop = Stop_AMD_Family_16h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_16h,
	.BaseClock = BaseClock_AuthenticAMD,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_AMD,
	.voltageFormula = VOLTAGE_FORMULA_AMD,
	.powerFormula   = POWER_FORMULA_AMD,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_16h
	},
[AMD_Family_17h] = {							/*  8*/
	.Signature = _AMD_Family_17h,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_17h
	},
[AMD_Family_18h] = {							/*  9*/
	.Signature = _AMD_Family_18h,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_18h
	},

[Core_Yonah] = {							/* 10*/
	.Signature = _Core_Yonah,
	.Query = Query_GenuineIntel,
	.Update = PerCore_Intel_Query,
	.Start = Start_GenuineIntel,
	.Stop = Stop_GenuineIntel,
	.Exit = NULL,
	.Timer = InitTimer_GenuineIntel,
	.BaseClock = BaseClock_Core,
	.ClockMod = NULL,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Core_Yonah
	},
[Core_Conroe] = {							/* 11*/
	.Signature = _Core_Conroe,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Core2,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_CORE2,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Core_Conroe
	},
[Core_Kentsfield] = {							/* 12*/
	.Signature = _Core_Kentsfield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Core2,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Core_Kentsfield
	},
[Core_Conroe_616] = {							/* 13*/
	.Signature = _Core_Conroe_616,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Core2,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Core_Conroe_616
	},
[Core_Penryn] = {							/* 14*/
	.Signature = _Core_Penryn,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Core2,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Core_Penryn_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Core_Penryn
	},
[Core_Dunnington] = {							/* 15*/
	.Signature = _Core_Dunnington,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Core2,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Core2_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Core_Dunnington
	},

[Atom_Bonnell] = {							/* 16*/
	.Signature = _Atom_Bonnell,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Bonnell
	},
[Atom_Silvermont] = {							/* 17*/
	.Signature = _Atom_Silvermont,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Silvermont
	},
[Atom_Lincroft] = {							/* 18*/
	.Signature = _Atom_Lincroft,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Lincroft
	},
[Atom_Clover_Trail] = {							/* 19*/
	.Signature = _Atom_Clover_Trail,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Clover_Trail
	},
[Atom_Saltwell] = {							/* 20*/
	.Signature = _Atom_Saltwell,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Saltwell
	},

[Silvermont_Bay_Trail] = {						/* 21*/
	.Signature = _Silvermont_Bay_Trail,
	.Query = Query_Silvermont,
	.Update = PerCore_Silvermont_Query,
	.Start = Start_Silvermont,
	.Stop = Stop_Silvermont,
	.Exit = NULL,
	.Timer = InitTimer_Silvermont,
	.BaseClock = BaseClock_Silvermont,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SOC,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_SoC_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Silvermont_Bay_Trail_Specific,
	.SystemDriver = SLM_Driver,
	.Architecture = Arch_Silvermont_Bay_Trail
	},
[Atom_Avoton] = {							/* 22*/
	.Signature = _Atom_Avoton,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Silvermont,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Atom_Avoton
	},

[Atom_Airmont] = {							/* 23*/
	.Signature = _Atom_Airmont,
	.Query = Query_Silvermont,
	.Update = PerCore_Silvermont_Query,
	.Start = Start_Silvermont,
	.Stop = Stop_Silvermont,
	.Exit = NULL,
	.Timer = InitTimer_Silvermont,
	.BaseClock = BaseClock_Airmont,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SOC,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Airmont
	},
[Atom_Goldmont] = {							/* 24*/
	.Signature = _Atom_Goldmont,
	.Query = Query_SandyBridge,
	.Update = PerCore_Goldmont_Query,
	.Start = Start_Goldmont,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = SNB_Driver,
	.Architecture = Arch_Atom_Goldmont
	},
[Atom_Sofia] = {							/* 25*/
	.Signature = _Atom_Sofia,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Sofia
	},
[Atom_Merrifield] = {							/* 26*/
	.Signature = _Atom_Merrifield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Merrifield
	},
[Atom_Moorefield] = {							/* 27*/
	.Signature = _Atom_Moorefield,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Moorefield
	},

[Nehalem_Bloomfield] = {						/* 28*/
	.Signature = _Nehalem_Bloomfield,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Nehalem,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Nehalem_Bloomfield_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Nehalem_Bloomfield
	},
[Nehalem_Lynnfield] = { 						/* 29*/
	.Signature = _Nehalem_Lynnfield,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Nehalem,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Nehalem_Lynnfield_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Nehalem_Lynnfield
	},
[Nehalem_MB] = {							/* 30*/
	.Signature = _Nehalem_MB,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Nehalem,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Nehalem_MB
	},
[Nehalem_EX] = {							/* 31*/
	.Signature = _Nehalem_EX,
	.Query = Query_Core2,
	.Update = PerCore_Nehalem_EX_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Nehalem,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Nehalem_EX
	},

[Westmere] = {								/* 32*/
	.Signature = _Westmere,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Westmere,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Westmere
	},
[Westmere_EP] = {							/* 33*/
	.Signature = _Westmere_EP,
	.Query = Query_Nehalem,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Westmere,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Westmere_EP_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Westmere_EP_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Westmere_EP
	},
[Westmere_EX] = {							/* 34*/
	.Signature = _Westmere_EX,
	.Query = Query_Core2, /* Xeon processor 7500 series-based platform */
	.Update = PerCore_Nehalem_EX_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Westmere,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#endif
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = NHM_Driver,
	.Architecture = Arch_Westmere_EX
	},

[SandyBridge] = {							/* 35*/
	.Signature = _SandyBridge,
	.Query = Query_SandyBridge,
	.Update = PerCore_SandyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.BaseClock = BaseClock_SandyBridge,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_SandyBridge_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = SNB_Driver,
	.Architecture = Arch_SandyBridge
	},
[SandyBridge_EP] = {							/* 36*/
	.Signature = _SandyBridge_EP,
	.Query = Query_SandyBridge,
	.Update = PerCore_SandyBridge_EP_Query,
	.Start = Start_SandyBridge_EP,
	.Stop = Stop_SandyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge_EP,
	.BaseClock = BaseClock_SandyBridge,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_SandyBridge_EP_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge_EP,
		.Stop = Stop_Uncore_SandyBridge_EP,
		.ClockMod = NULL
		},
	.Specific = SandyBridge_EP_Specific,
	.SystemDriver = SNB_Driver,
	.Architecture = Arch_SandyBridge_EP
	},

[IvyBridge]  = {							/* 37*/
	.Signature = _IvyBridge,
	.Query = Query_IvyBridge,
	.Update = PerCore_IvyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.BaseClock = BaseClock_IvyBridge,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_IvyBridge_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = IVB_Driver,
	.Architecture = Arch_IvyBridge
	},
[IvyBridge_EP] = {							/* 38*/
	.Signature = _IvyBridge_EP,
	.Query = Query_IvyBridge_EP,
	.Update = PerCore_IvyBridge_EP_Query,
	.Start = Start_SandyBridge_EP,
	.Stop = Stop_SandyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge_EP,
	.BaseClock = BaseClock_IvyBridge,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = TurboClock_IvyBridge_EP,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_SandyBridge_EP_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge_EP,
		.Stop = Stop_Uncore_SandyBridge_EP,
		.ClockMod = NULL
		},
	.Specific = IvyBridge_EP_Specific,
	.SystemDriver = IVB_Driver,
	.Architecture = Arch_IvyBridge_EP
	},

[Haswell_DT] = {							/* 39*/
	.Signature = _Haswell_DT,
	.Query = Query_Haswell,
	.Update = PerCore_Haswell_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge,
		.ClockMod = NULL
		},
	.Specific = Haswell_DT_Specific,
	.SystemDriver = HSW_Driver,
	.Architecture = Arch_Haswell_DT
	},
[Haswell_EP] = {							/* 40*/
	.Signature = _Haswell_EP,
	.Query = Query_Haswell_EP,
	.Update = PerCore_Haswell_EP_Query,
	.Start = Start_Haswell_EP,
	.Stop = Stop_Haswell_EP,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_EP,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = TurboClock_Haswell_EP,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_EP,
		.Stop = Stop_Uncore_Haswell_EP,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Haswell_EP_Specific,
	.SystemDriver = HSW_Driver,
	.Architecture = Arch_Haswell_EP
	},
[Haswell_ULT] = {							/* 41*/
	.Signature = _Haswell_ULT,
	.Query = Query_Haswell_ULT,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = HSW_Driver,
	.Architecture = Arch_Haswell_ULT
	},
[Haswell_ULX] = {							/* 42*/
	.Signature = _Haswell_ULX,
	.Query = Query_Haswell_ULX,
	.Update = PerCore_Haswell_ULX,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Haswell_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = HSW_Driver,
	.Architecture = Arch_Haswell_ULX
	},

[Broadwell]  = {							/* 43*/
	.Signature = _Broadwell,
	.Query = Query_Broadwell,
	.Update = PerCore_Broadwell_Query,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Broadwell,
		.Stop = Stop_Uncore_Broadwell,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = BDW_Driver,
	.Architecture = Arch_Broadwell
	},
[Broadwell_D] = {							/* 44*/
	.Signature = _Broadwell_D,
	.Query = Query_Broadwell_EP,
	.Update = PerCore_Haswell_EP_Query,
	.Start = Start_Haswell_EP,
	.Stop = Stop_Haswell_EP,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_EP,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_Broadwell_EP_HWP,
	.TurboClock = TurboClock_Broadwell_EP,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_EP,
		.Stop = Stop_Uncore_Haswell_EP,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = BDW_EP_Driver,
	.Architecture = Arch_Broadwell_D
	},
[Broadwell_H] = {							/* 45*/
	.Signature = _Broadwell_H,
	.Query = Query_Broadwell,
	.Update = PerCore_Broadwell_Query,
	.Start = Start_Broadwell,
	.Stop = Stop_Broadwell,
	.Exit = NULL,
	.Timer = InitTimer_Broadwell,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Broadwell,
		.Stop = Stop_Uncore_Broadwell,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = BDW_Driver,
	.Architecture = Arch_Broadwell_H
	},
[Broadwell_EP] = {							/* 46*/
	.Signature = _Broadwell_EP,
	.Query = Query_Broadwell_EP,
	.Update = PerCore_Haswell_EP_Query,
	.Start = Start_Haswell_EP,
	.Stop = Stop_Haswell_EP,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_EP,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_Broadwell_EP_HWP,
	.TurboClock = TurboClock_Broadwell_EP,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Broadwell_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_EP,
		.Stop = Stop_Uncore_Haswell_EP,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Broadwell_EP_Specific,
	.SystemDriver = BDW_EP_Driver,
	.Architecture = Arch_Broadwell_EP
	},

[Skylake_UY] = {							/* 47*/
	.Signature = _Skylake_UY,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Skylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Skylake_UY
	},
[Skylake_S]  = {							/* 48*/
	.Signature = _Skylake_S,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Skylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Skylake_S
	},
[Skylake_X]  = {							/* 49*/
	.Signature = _Skylake_X,
	.Query = Query_Skylake_X,
	.Update = PerCore_Skylake_X_Query,
	.Start = Start_Skylake_X,
	.Stop = Stop_Skylake_X,
	.Exit = NULL,
	.Timer = InitTimer_Skylake_X,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = TurboClock_Skylake_X,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SKL_X,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Skylake_X_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake_X,
		.Stop = Stop_Uncore_Skylake_X,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Skylake_X_Specific,
	.SystemDriver = SKX_Driver,
	.Architecture = Arch_Skylake_X
	},

[Xeon_Phi] = {								/* 50*/
	.Signature = _Xeon_Phi,
	.Query = Query_SandyBridge,
	.Update = PerCore_SandyBridge_Query,
	.Start = Start_SandyBridge,
	.Stop = Stop_SandyBridge,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_SandyBridge,
		.Stop = Stop_Uncore_SandyBridge,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Xeon_Phi
	},

[Kabylake] = {								/* 51*/
	.Signature = _Kabylake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Kabylake_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Kabylake
	},
[Kabylake_UY] = {							/* 52*/
	.Signature = _Kabylake_UY,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Kabylake_UY_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Kabylake_UY
	},

[Cannonlake] = {							/* 53*/
	.Signature = _Cannonlake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Cannonlake
	},

[Geminilake] = {							/* 54*/
	.Signature = _Geminilake,
	.Query = Query_Haswell_ULT,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
	.Stop = Stop_Haswell_ULT,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_ULT,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Haswell_ULT,
		.Stop = Stop_Uncore_Haswell_ULT,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Geminilake
	},

[Icelake] = {								/* 55*/
	.Signature = _Icelake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Icelake
	},
[Icelake_UY] = {							/* 56*/
	.Signature = _Icelake_UY,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Icelake_UY
	},
[Icelake_X] = {								/* 57*/
	.Signature = _Icelake_X,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Icelake_X
	},
[Icelake_D] = {								/* 58*/
	.Signature = _Icelake_D,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Icelake_D
	},

[Sunny_Cove] = {							/* 59*/
	.Signature = _Sunny_Cove,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Sunny_Cove
	},

[Tigerlake] = {								/* 60*/
	.Signature = _Tigerlake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tigerlake
	},
[Tigerlake_U] = {							/* 61*/
	.Signature = _Tigerlake_U,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tigerlake_U
	},

[Cometlake] = {								/* 62*/
	.Signature = _Cometlake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Cometlake
	},
[Cometlake_UY] = {							/* 63*/
	.Signature = _Cometlake_UY,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Cometlake_UY
	},

[Atom_C3000] = {							/* 64*/
	.Signature = _Atom_C3000,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Atom_C3000
	},
[Tremont_Jacobsville] = {						/* 65*/
	.Signature = _Tremont_Jacobsville,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tremont_Jacobsville
	},
[Tremont_Lakefield] = {							/* 66*/
	.Signature = _Tremont_Lakefield,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tremont_Lakefield
	},
[Tremont_Elkhartlake] = {						/* 67*/
	.Signature = _Tremont_Elkhartlake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tremont_Elkhartlake
	},
[Tremont_Jasperlake] = {						/* 68*/
	.Signature = _Tremont_Jasperlake,
	.Query = Query_Skylake,
	.Update = PerCore_Skylake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tremont_Jasperlake
	},

[AMD_Zen] = {								/* 69*/
	.Signature = _AMD_Zen,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen
	},
[AMD_Zen_APU] = {							/* 70*/
	.Signature = _AMD_Zen_APU,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen_APU_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen_APU
	},
[AMD_ZenPlus] = {							/* 71*/
	.Signature = _AMD_ZenPlus,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_ZenPlus_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_ZenPlus
	},
[AMD_ZenPlus_APU] = {							/* 72*/
	.Signature = _AMD_ZenPlus_APU,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_ZenPlus_APU_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_ZenPlus_APU
	},
[AMD_Zen_APU_Rv2] = {							/* 73*/
	.Signature = _AMD_Zen_APU_Rv2,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen_APU_Rv2_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen_APU_Rv2
	},
[AMD_EPYC_Rome] = {							/* 74*/
	.Signature = _AMD_EPYC_Rome,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_EPYC_Rome_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_EPYC_Rome
	},
[AMD_Zen2_CPK] = {							/* 75*/
	.Signature = _AMD_Zen2_CPK,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_CPK_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_CPK
	},
[AMD_Zen2_APU] = {							/* 76*/
	.Signature = _AMD_Zen2_APU,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_APU_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_APU
	},
[AMD_Zen2_MTS] = {							/* 77*/
	.Signature = _AMD_Zen2_MTS,
	.Query = Query_AMD_Family_17h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = NULL,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_MTS_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_MTS
	}
};

