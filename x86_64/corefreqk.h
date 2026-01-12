/*
 * CoreFreq
 * Copyright (C) 2015-2026 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define RDCOUNTER(_val, _cnt)						\
({									\
	unsigned int _lo, _hi;						\
									\
	__asm__ volatile						\
	(								\
		"rdmsr" 						\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_cnt)						\
	);								\
	_val=_lo | ((unsigned long long) _hi << 32);			\
})

#define WRCOUNTER(_val, _cnt)						\
	__asm__ volatile						\
	(								\
		"wrmsr" 						\
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
		"rdmsr" 						\
		: "=a" (_lo),						\
		  "=d" (_hi)						\
		: "c" (_reg)						\
	);								\
	_data.value=_lo | ((unsigned long long) _hi << 32);		\
})

#define WRMSR(_data, _reg)						\
	__asm__ volatile						\
	(								\
		"wrmsr" 						\
		:							\
		: "c" (_reg),						\
		  "a" ((unsigned int) _data.value & 0xFFFFFFFF),	\
		  "d" ((unsigned int) (_data.value >> 32))		\
	);

#define RDMSR64(_data, _reg)						\
	__asm__ volatile						\
	(								\
		"xorq	%%rax,	%%rax"		"\n\t"			\
		"xorq	%%rdx,	%%rdx"		"\n\t"			\
		"movq	%1,	%%rcx"		"\n\t"			\
		"rdmsr" 			"\n\t"			\
		"shlq	$32,	%%rdx"		"\n\t"			\
		"orq	%%rdx,	%%rax"		"\n\t"			\
		"movq	%%rax,	%0"					\
		: "=m" (_data)						\
		: "i" (_reg)						\
		: "%rax", "%rcx", "%rdx"				\
	)

#define WRMSR64(_data, _reg)						\
	__asm__ volatile						\
	(								\
		"movq	%0,	%%rax"		"\n\t"			\
		"movq	%%rax,	%%rdx"		"\n\t"			\
		"shrq	$32,	%%rdx"		"\n\t"			\
		"movq	%1,	%%rcx"		"\n\t"			\
		"wrmsr" 						\
		: "=m" (_data)						\
		: "i" (_reg)						\
		: "%rax", "%rcx", "%rdx"				\
	)

#define Atomic_Read_VPMC(_lock, _dest, _src)				\
{									\
	__asm__ volatile						\
	(								\
		"xorq	%%rax,	%%rax"		"\n\t"			\
	_lock	"cmpxchg %%rax, %[src]" 	"\n\t"			\
		"movq	%%rax,	%[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "m" (_src)					\
		: "%rax", "cc", "memory"				\
	);								\
}

#define Atomic_Add_VPMC(_lock, _dest, _src)				\
{									\
	__asm__ volatile						\
	(								\
		"xorq	%%rax,	%%rax"		"\n\t"			\
	_lock	"cmpxchg %%rax, %[src]" 	"\n\t"			\
		"addq	%%rax,	%[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "m" (_src)					\
		: "%rax", "cc", "memory"				\
	);								\
}

#define ASM_CODE_RDMSR(_msr, _reg)					\
	"# Read MSR counter."			"\n\t"			\
	"movq	$" #_msr ", %%rcx"		"\n\t"			\
	"rdmsr" 				"\n\t"			\
	"shlq	$32,	%%rdx"			"\n\t"			\
	"orq	%%rdx,	%%rax"			"\n\t"			\
	"# Save counter value"			"\n\t"			\
	"movq	%%rax,	%%" #_reg		"\n\t"

#define ASM_RDMSR(_msr, _reg) ASM_CODE_RDMSR(_msr, _reg)


#define ASM_COUNTERx1(	_reg0, _reg1,					\
			_tsc_inst, mem_tsc,				\
			_msr1, _mem1)					\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDMSR(_msr1, _reg1) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%3"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	ASM_RDMSR(_msr4, _reg4) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%3"		"\n\t"			\
	"movq	%%" #_reg4 ",	%4"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	ASM_RDMSR(_msr4, _reg4) 					\
	ASM_RDMSR(_msr5, _reg5) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%3"		"\n\t"			\
	"movq	%%" #_reg4 ",	%4"		"\n\t"			\
	"movq	%%" #_reg5 ",	%5"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	"pushq	%%" #_reg3			"\n\t"			\
	ASM_RDMSR(_msr4, _reg3) 					\
	"pushq	%%" #_reg3			"\n\t"			\
	ASM_RDMSR(_msr5, _reg3) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%5"		"\n\t"			\
	"popq	%%" #_reg3			"\n\t"			\
	"movq	%%" #_reg3 ",	%4"		"\n\t"			\
	"popq	%%" #_reg3			"\n\t"			\
	"movq	%%" #_reg3 ",	%3"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	ASM_RDMSR(_msr4, _reg4) 					\
	ASM_RDMSR(_msr5, _reg5) 					\
	ASM_RDMSR(_msr6, _reg6) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%3"		"\n\t"			\
	"movq	%%" #_reg4 ",	%4"		"\n\t"			\
	"movq	%%" #_reg5 ",	%5"		"\n\t"			\
	"movq	%%" #_reg6 ",	%6"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	"pushq	%%" #_reg3			"\n\t"			\
	ASM_RDMSR(_msr4, _reg3) 					\
	"pushq	%%" #_reg3			"\n\t"			\
	ASM_RDMSR(_msr5, _reg3) 					\
	"pushq	%%" #_reg3			"\n\t"			\
	ASM_RDMSR(_msr6, _reg3) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%6"		"\n\t"			\
	"popq	%%" #_reg3			"\n\t"			\
	"movq	%%" #_reg3 ",	%5"		"\n\t"			\
	"popq	%%" #_reg3			"\n\t"			\
	"movq	%%" #_reg3 ",	%4"		"\n\t"			\
	"popq	%%" #_reg3			"\n\t"			\
	"movq	%%" #_reg3 ",	%3"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	ASM_RDMSR(_msr3, _reg3) 					\
	ASM_RDMSR(_msr4, _reg4) 					\
	ASM_RDMSR(_msr5, _reg5) 					\
	ASM_RDMSR(_msr6, _reg6) 					\
	ASM_RDMSR(_msr7, _reg7) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%2"		"\n\t"			\
	"movq	%%" #_reg3 ",	%3"		"\n\t"			\
	"movq	%%" #_reg4 ",	%4"		"\n\t"			\
	"movq	%%" #_reg5 ",	%5"		"\n\t"			\
	"movq	%%" #_reg6 ",	%6"		"\n\t"			\
	"movq	%%" #_reg7 ",	%7"					\
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
	ASM_RDMSR(_msr1, _reg1) 					\
	ASM_RDMSR(_msr2, _reg2) 					\
	"pushq	%%" #_reg2			"\n\t"			\
	ASM_RDMSR(_msr3, _reg2) 					\
	"pushq	%%" #_reg2			"\n\t"			\
	ASM_RDMSR(_msr4, _reg2) 					\
	"pushq	%%" #_reg2			"\n\t"			\
	ASM_RDMSR(_msr5, _reg2) 					\
	"pushq	%%" #_reg2			"\n\t"			\
	ASM_RDMSR(_msr6, _reg2) 					\
	"pushq	%%" #_reg2			"\n\t"			\
	ASM_RDMSR(_msr7, _reg2)						\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"		"\n\t"			\
	"movq	%%" #_reg2 ",	%7"		"\n\t"			\
	"popq	%%" #_reg2			"\n\t"			\
	"movq	%%" #_reg2 ",	%6"		"\n\t"			\
	"popq	%%" #_reg2			"\n\t"			\
	"movq	%%" #_reg2 ",	%5"		"\n\t"			\
	"popq	%%" #_reg2			"\n\t"			\
	"movq	%%" #_reg2 ",	%4"		"\n\t"			\
	"popq	%%" #_reg2			"\n\t"			\
	"movq	%%" #_reg2 ",	%3"		"\n\t"			\
	"popq	%%" #_reg2			"\n\t"			\
	"movq	%%" #_reg2 ",	%2"					\
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


#if defined(OPTIM_LVL) && (OPTIM_LVL == 0 || OPTIM_LVL == 1)

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
/*	#warning "Optimization" 					*/

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
	(0x80000000 | ((bus) << 16)|((dev) << 11)|((fn) << 8)|((reg) & ~3))

#define RDPCI(_data, _reg)						\
({									\
	__asm__ volatile						\
	(								\
		"movl	%1	,	%%eax"		"\n\t"		\
		"movl	$0xcf8	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"		"\n\t"		\
		"movl	$0xcfc	,	%%edx"		"\n\t"		\
		"inl	%%dx	,	%%eax"		"\n\t"		\
		"movl	%%eax	,	%0"				\
		: "=m"	(_data) 					\
		: "ir"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define WRPCI(_data, _reg)						\
({									\
	__asm__ volatile						\
	(								\
		"movl	%1	,	%%eax"		"\n\t"		\
		"movl	$0xcf8	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"		"\n\t"		\
		"movl	%0	,	%%eax"		"\n\t"		\
		"movl	$0xcfc	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"				\
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

/* Source: Winbond W83627 and ITE IT8720F datasheets			*/
#define HWM_SIO_INDEX_PORT	0x295
#define HWM_SIO_DATA_PORT	0x296
#define HWM_SIO_CPUVCORE	0x20

#define RDSIO(_data, _reg, _index_port, _data_port)			\
({									\
	__asm__ volatile						\
	(								\
		"movw	%1	,	%%ax"		"\n\t"		\
		"movw	%2	,	%%dx"		"\n\t"		\
		"outw	%%ax	,	%%dx"		"\n\t"		\
		"movw	%3	,	%%dx"		"\n\t"		\
		"inb	%%dx	,	%%al"		"\n\t"		\
		"movb	%%al	,	%0"				\
		: "=m"	(_data) 					\
		: "i"	(_reg) ,					\
		  "i"	(_index_port),					\
		  "i"	(_data_port)					\
		: "%ax", "%dx", "memory"				\
	);								\
})

/* Sources: PPR for AMD Family 17h					*/
#define AMD_FCH_PM_CSTATE_EN	0x0000007e

#define AMD_FCH_READ16(_data, _reg)					\
({									\
	__asm__ volatile						\
	(								\
		"movl	%1	,	%%eax"		"\n\t"		\
		"movl	$0xcd6	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"		"\n\t"		\
		"movl	$0xcd7	,	%%edx"		"\n\t"		\
		"inw	%%dx	,	%%ax"		"\n\t"		\
		"movw	%%ax	,	%0"				\
		: "=m"	(_data) 					\
		: "i"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define AMD_FCH_WRITE16(_data, _reg)					\
({									\
	__asm__ volatile						\
	(								\
		"movl	%1	,	%%eax"		"\n\t"		\
		"movl	$0xcd6	,	%%edx"		"\n\t"		\
		"outl	%%eax	,	%%dx"		"\n\t"		\
		"movw	%0	,	%%ax" 		"\n\t"		\
		"movl	$0xcd7	,	%%edx"		"\n\t"		\
		"outw	%%ax	,	%%dx"		"\n\t"		\
		:							\
		: "im"	(_data),					\
		  "i"	(_reg)						\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define AMD_FCH_PM_Read16(IndexRegister, DataRegister)			\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_FCH_LOCK,		\
				ATOMIC_SEED) ;				\
	if (ret == 0) {							\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		AMD_FCH_READ16(DataRegister.value, IndexRegister);	\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_FCH_LOCK,		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
})

#define AMD_FCH_PM_Write16(IndexRegister , DataRegister)		\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_FCH_LOCK,		\
				ATOMIC_SEED );				\
	if (ret == 0) {							\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		AMD_FCH_WRITE16(DataRegister.value, IndexRegister);	\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_FCH_LOCK,		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
})

/* Hardware Monitoring: Super I/O chipset identifiers			*/
#define COMPATIBLE		0xffff
#define W83627			0x5ca3
#define IT8720			0x8720
/* Voltage Curve Optimizer						*/
#define AMD_VCO			0xfacc

/*
 * --- Core_AMD_SMN_Read and Core_AMD_SMN_Write ---
 *
 * amd_smn_read() and amd_smn_write() protect any SMU access through
 * mutex_[un]lock functions which must not be used in interrupt context.
 *
 * The high resolution timers are bound to CPUs using smp_call_function_*
 * where context is interrupt; and where mutexes will freeze the kernel.
*/
#define PCI_AMD_SMN_Read(	SMN_Register,				\
				SMN_Address,				\
				SMU_IndexRegister,			\
				SMU_DataRegister )			\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED );				\
	if ( ret == 0 ) {						\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		WRPCI(SMN_Address, SMU_IndexRegister);			\
		RDPCI(SMN_Register.value, SMU_DataRegister);		\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
    if (tries == 0) {							\
	pr_warn("CoreFreq: PCI_AMD_SMN_Read(%x, %x) TryLock\n", 	\
		SMN_Register.value, SMN_Address);			\
    }									\
})

#define PCI_AMD_SMN_Write(	SMN_Register,				\
				SMN_Address,				\
				SMU_IndexRegister,			\
				SMU_DataRegister )			\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED );				\
	if ( ret == 0 ) {						\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		WRPCI(SMN_Address, SMU_IndexRegister);			\
		WRPCI(SMN_Register.value, SMU_DataRegister);		\
									\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
    if (tries == 0) {							\
	pr_warn("CoreFreq: PCI_AMD_SMN_Write(%x, %x) TryLock\n",	\
		SMN_Register.value, SMN_Address);			\
    }									\
})

#if defined(CONFIG_AMD_NB) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0) /* asm/amd_node.h */
static u16 amd_pci_dev_to_node_id(struct pci_dev *pdev)
{
	return PCI_SLOT(pdev->devfn) - AMD_NODE0_PCI_SLOT;
}

#define GetRootFromNode(_node)	pci_get_domain_bus_and_slot( 0x0, 0x0,	\
							PCI_DEVFN(0x0, 0x0) )
#else
#define GetRootFromNode(_node)	node_to_amd_nb(_node)->root
#endif

#define AMD_SMN_RW(node, address, value, write, indexPort, dataPort)	\
({									\
	struct pci_dev *root;						\
	signed int res = 0;						\
									\
  if (node < amd_nb_num())						\
  {									\
    if ((root = GetRootFromNode(node)) != NULL) 			\
    {									\
	res = pci_write_config_dword(root, indexPort, address);		\
									\
      if (write == true) {						\
	res = pci_write_config_dword(root, dataPort, value);		\
      } else {								\
	res = pci_read_config_dword(root, dataPort, &value);		\
      } 								\
    } else {								\
	res = -ENXIO;							\
    }									\
  } else {								\
	res = -EINVAL;							\
  }									\
	res;								\
})

#define AMD_SMN_Read(SMN_Register, SMN_Address, UMC_device)		\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED );				\
	if ( ret == 0 ) {						\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		if (AMD_SMN_RW( amd_pci_dev_to_node_id(UMC_device),	\
				SMN_Address, SMN_Register.value, false,	\
				SMU_AMD_INDEX_PORT_F17H,		\
				SMU_AMD_DATA_PORT_F17H ) != 0) {	\
			tries = 1;					\
		}							\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
    if (tries == 0) {							\
	pr_warn("CoreFreq: AMD_SMN_Read(%x, %x) TryLock\n",		\
		SMN_Register.value, SMN_Address);			\
    }									\
})

#define AMD_SMN_Write(SMN_Register, SMN_Address, UMC_device)		\
({									\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
    do {								\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED );				\
	if ( ret == 0 ) {						\
		udelay(BIT_IO_DELAY_INTERVAL);				\
	} else {							\
		if (AMD_SMN_RW( amd_pci_dev_to_node_id(UMC_device),	\
				SMN_Address, SMN_Register.value, true,	\
				SMU_AMD_INDEX_PORT_F17H,		\
				SMU_AMD_DATA_PORT_F17H ) != 0) {	\
			tries = 1;					\
		}							\
		BIT_ATOM_UNLOCK(BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_SMN_LOCK,		\
				ATOMIC_SEED);				\
	}								\
	tries--;							\
    } while ( (tries != 0) && (ret != 1) );				\
    if (tries == 0) {							\
	pr_warn("CoreFreq: AMD_SMN_Write(%x, %x) TryLock\n",		\
		SMN_Register.value, SMN_Address);			\
    }									\
})

#define Core_AMD_SMN_Read(SMN_Register, SMN_Address, UMC_device)	\
	if (UMC_device) {						\
		AMD_SMN_Read(SMN_Register, SMN_Address, UMC_device);	\
	} else {							\
		PCI_AMD_SMN_Read(SMN_Register,				\
				SMN_Address,				\
				SMU_AMD_INDEX_REGISTER_F17H,		\
				SMU_AMD_DATA_REGISTER_F17H);		\
}

#define Core_AMD_SMN_Write(SMN_Register, SMN_Address, UMC_device)	\
	if (UMC_device) {						\
		AMD_SMN_Write(SMN_Register, SMN_Address, UMC_device);	\
	} else {							\
		PCI_AMD_SMN_Write(SMN_Register,				\
				SMN_Address,				\
				SMU_AMD_INDEX_REGISTER_F17H,		\
				SMU_AMD_DATA_REGISTER_F17H);		\
}

#else /* CONFIG_AMD_NB */

#define Core_AMD_SMN_Read(SMN_Register, SMN_Address, UMC_device)	\
	PCI_AMD_SMN_Read(	SMN_Register,				\
				SMN_Address,				\
				SMU_AMD_INDEX_REGISTER_F17H,		\
				SMU_AMD_DATA_REGISTER_F17H )

#define Core_AMD_SMN_Write(SMN_Register, SMN_Address, UMC_device)	\
	PCI_AMD_SMN_Write(	SMN_Register,				\
				SMN_Address,				\
				SMU_AMD_INDEX_REGISTER_F17H,		\
				SMU_AMD_DATA_REGISTER_F17H )
#endif /* CONFIG_AMD_NB */

typedef union
{
	unsigned int		value;
	struct
	{
		unsigned int
		bits		: 32-0;
	};
} HSMP_ARG;

#define PCI_HSMP_Mailbox(	MSG_FUNC,				\
				MSG_ARG,				\
				HSMP_CmdRegister,			\
				HSMP_ArgRegister,			\
				HSMP_RspRegister,			\
				SMU_IndexRegister,			\
				SMU_DataRegister )			\
({									\
	HSMP_ARG MSG_RSP = {.value = 0x0};				\
	HSMP_ARG MSG_ID = {.value = MSG_FUNC};				\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	unsigned char ret;						\
  do {									\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_HSMP_LOCK, 	\
				ATOMIC_SEED );				\
    if ( ret == 0 ) {							\
	udelay(BIT_IO_DELAY_INTERVAL);					\
    }									\
    else								\
    {									\
	unsigned int idx;						\
	unsigned char wait;						\
									\
	WRPCI(HSMP_RspRegister	, SMU_IndexRegister);			\
	WRPCI(MSG_RSP.value	, SMU_DataRegister);			\
									\
	for (idx = 0; idx < 8; idx++) { 				\
		WRPCI(HSMP_ArgRegister + (idx << 2), SMU_IndexRegister);\
		WRPCI(MSG_ARG[idx].value, SMU_DataRegister);		\
	}								\
	WRPCI(HSMP_CmdRegister	, SMU_IndexRegister);			\
	WRPCI(MSG_ID.value	, SMU_DataRegister);			\
									\
	idx = BIT_IO_RETRIES_COUNT;					\
	do {								\
		WRPCI(HSMP_RspRegister	, SMU_IndexRegister);		\
		RDPCI(MSG_RSP.value	, SMU_DataRegister);		\
									\
		idx--;							\
		wait = (idx != 0) && (MSG_RSP.value == 0x0) ? 1 : 0;	\
		if (wait == 1) {					\
			udelay(BIT_IO_DELAY_INTERVAL);			\
		}							\
	} while (wait == 1);						\
	if (idx == 0) { 						\
		pr_warn("CoreFreq: PCI_HSMP_Mailbox(%x) Timeout\n",	\
			MSG_FUNC);					\
	}								\
	else if (MSG_RSP.value == 0x1)					\
	{								\
	    for (idx = 0; idx < 8; idx++) {				\
		WRPCI(HSMP_ArgRegister + (idx << 2), SMU_IndexRegister);\
		RDPCI(MSG_ARG[idx].value, SMU_DataRegister);		\
	    }								\
	}								\
	BIT_ATOM_UNLOCK(BUS_LOCK,					\
			PRIVATE(OF(Zen)).AMD_HSMP_LOCK, 		\
			ATOMIC_SEED);					\
    }									\
	tries--;							\
  } while ( (tries != 0) && (ret != 1) );				\
  if (tries == 0) {							\
	pr_warn("CoreFreq: PCI_HSMP_Mailbox(%x) TryLock\n", MSG_FUNC);	\
  }									\
	MSG_RSP.value;							\
})

#if defined(CONFIG_AMD_NB) && LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0)

#define AMD_HSMP_Mailbox(	MSG_FUNC,				\
				MSG_ARG,				\
				HSMP_CmdRegister,			\
				HSMP_ArgRegister,			\
				HSMP_RspRegister,			\
				UMC_device )				\
({									\
	HSMP_ARG MSG_RSP = {.value = 0x0};				\
	HSMP_ARG MSG_ID = {.value = MSG_FUNC};				\
	unsigned int tries = BIT_IO_RETRIES_COUNT;			\
	signed int res = 0;						\
	unsigned char ret;						\
  do {									\
	ret = BIT_ATOM_TRYLOCK( BUS_LOCK,				\
				PRIVATE(OF(Zen)).AMD_HSMP_LOCK, 	\
				ATOMIC_SEED );				\
    if ( ret == 0 ) {							\
	udelay(BIT_IO_DELAY_INTERVAL);					\
    }									\
    else								\
    {									\
	unsigned int idx;						\
	unsigned char wait;						\
									\
	res = AMD_SMN_RW(amd_pci_dev_to_node_id(UMC_device),		\
			HSMP_RspRegister, MSG_RSP.value, true,		\
			AMD_HSMP_INDEX_PORT, AMD_HSMP_DATA_PORT);	\
									\
	for (idx = 0; (idx < 8) && (res == 0); idx++) { 		\
		res = AMD_SMN_RW(amd_pci_dev_to_node_id(UMC_device),	\
				HSMP_ArgRegister + (idx << 2),		\
				MSG_ARG[idx].value, true,		\
				AMD_HSMP_INDEX_PORT, AMD_HSMP_DATA_PORT);\
	}								\
      if ( (res == 0)							\
      && ( (res = AMD_SMN_RW(amd_pci_dev_to_node_id(UMC_device),	\
			HSMP_CmdRegister, MSG_ID.value, true,		\
			AMD_HSMP_INDEX_PORT, AMD_HSMP_DATA_PORT)) == 0))\
      { 								\
	idx = BIT_IO_RETRIES_COUNT;					\
	do {								\
		res = AMD_SMN_RW(amd_pci_dev_to_node_id(UMC_device),	\
				HSMP_RspRegister, MSG_RSP.value, false, \
				AMD_HSMP_INDEX_PORT,AMD_HSMP_DATA_PORT);\
									\
		idx--;							\
		wait=(idx != 0) && (MSG_RSP.value == 0x0) && (res == 0) ? 1:0;\
		if (wait == 1) {					\
			udelay(BIT_IO_DELAY_INTERVAL);			\
		}							\
	} while (wait == 1);						\
	if ((idx == 0) || (res != 0)) {					\
		pr_warn("CoreFreq: AMD_HSMP_Mailbox(%x) Timeout\n",	\
			MSG_FUNC);					\
	}								\
	else if (MSG_RSP.value == 0x1)					\
	{								\
	    for (idx = 0; (idx < 8) && (res == 0); idx++) {		\
		res = AMD_SMN_RW(amd_pci_dev_to_node_id(UMC_device),	\
				HSMP_ArgRegister + (idx << 2),		\
				MSG_ARG[idx].value, false,		\
				AMD_HSMP_INDEX_PORT, AMD_HSMP_DATA_PORT);\
	    }								\
	}								\
      }									\
	BIT_ATOM_UNLOCK(BUS_LOCK,					\
			PRIVATE(OF(Zen)).AMD_HSMP_LOCK, 		\
			ATOMIC_SEED);					\
    }									\
	if (res != 0) { 						\
		tries = 1;						\
	}								\
	tries--;							\
  } while ( (tries != 0) && (ret != 1) );				\
  if (tries == 0) {							\
	pr_warn("CoreFreq: AMD_HSMP_Mailbox(%x) TryLock\n", MSG_FUNC);	\
  }									\
	res == 0 ? MSG_RSP.value : HSMP_UNSPECIFIED;			\
})

#define AMD_HSMP_Exec(MSG_FUNC, MSG_ARG)				\
({									\
	unsigned int rx;						\
	if (PRIVATE(OF(Zen)).Device.DF) {				\
		rx = AMD_HSMP_Mailbox(	MSG_FUNC,			\
					MSG_ARG,			\
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily < 0xB ?	\
					SMU_HSMP_CMD : SMU_HSMP_CMD_F1A,\
					SMU_HSMP_ARG,			\
					SMU_HSMP_RSP,			\
					PRIVATE(OF(Zen)).Device.DF );	\
	} else {							\
		rx = PCI_HSMP_Mailbox(	MSG_FUNC,			\
					MSG_ARG,			\
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily < 0xB ?	\
					SMU_HSMP_CMD : SMU_HSMP_CMD_F1A,\
					SMU_HSMP_ARG,			\
					SMU_HSMP_RSP,			\
					AMD_HSMP_INDEX_REGISTER,	\
					AMD_HSMP_DATA_REGISTER );	\
	}								\
	rx;								\
})

#else

#define AMD_HSMP_Exec(MSG_FUNC, MSG_ARG)				\
({									\
	unsigned int rx=PCI_HSMP_Mailbox(MSG_FUNC,			\
					MSG_ARG,			\
		PUBLIC(RO(Proc))->Features.Std.EAX.ExtFamily < 0xB ?	\
					SMU_HSMP_CMD : SMU_HSMP_CMD_F1A,\
					SMU_HSMP_ARG,			\
					SMU_HSMP_RSP,			\
					AMD_HSMP_INDEX_REGISTER,	\
					AMD_HSMP_DATA_REGISTER );	\
	rx;								\
})

#endif /* CONFIG_AMD_NB */

/* Driver' private and public data definitions.				*/
enum CSTATES_CLASS {
	CSTATES_NHM,
	CSTATES_SNB,
	CSTATES_ULT,
	CSTATES_SKL,
	CSTATES_SOC_SLM,
	CSTATES_SOC_GDM
};

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

	union {
	    struct {
		union {
			UNCORE_GLOBAL_PERF_CONTROL Uncore_GlobalPerfControl;
			UNCORE_PMON_GLOBAL_CONTROL Uncore_PMonGlobalControl;
		};
			UNCORE_FIXED_PERF_CONTROL  Uncore_FixedPerfControl;
	    } Intel;
	    struct {
			ZEN_DF_PERF_CTL 	Zen_DataFabricPerfControl;
	    } AMD;
	} SaveArea;

	union {
		struct {
		    struct {
			#ifdef CONFIG_AMD_NB
			struct pci_dev	*DF;
			#endif
		    } Device;
			Bit64	AMD_HSMP_LOCK __attribute__ ((aligned (8)));
			Bit64	AMD_SMN_LOCK __attribute__ ((aligned (8)));
			Bit64	AMD_FCH_LOCK __attribute__ ((aligned (8)));
		} Zen;
		struct {
			struct pci_dev	*HB;
			void __iomem	*BAR;
			unsigned int	ADDR;
		} PCU;
	};

	struct kmem_cache	*Cache;

	struct PRIV_CORE_ST {
		JOIN		Join;

		union SAVE_AREA_CORE {
		    struct	/* Intel				*/
		    {
			CORE_GLOBAL_PERF_CONTROL Core_GlobalPerfControl;
			CORE_FIXED_PERF_CONTROL  Core_FixedPerfControl;
		    };
		    struct	/* AMD					*/
		    {
			ZEN_PERF_CTL		Zen_PerformanceControl;
			ZEN_L3_PERF_CTL 	Zen_L3_Cache_PerfControl;
			HWCR			Core_HardwareConfiguration;
		    };
		} SaveArea;
	} *Core[];
} KPRIVATE;


/* Sources:
 * Intel 64 and IA-32 Architectures Software Developer’s Manual; Vol. 2A
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
	[CPUID_00000007_00000001_EXT_FEAT_SUB_LEAF_2]
	= {.func = 0x00000007, .sub = 0x00000002},
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
/* AMD Family 19h */
	[CPUID_0000000D_00000009_MPK_STATE_SUB_LEAF]
	= {.func = 0x0000000d, .sub = 0x00000009},
	[CPUID_0000000D_00000009_CET_U_SUB_LEAF]
	= {.func = 0x0000000d, .sub = 0x0000000b},
	[CPUID_0000000D_00000009_CET_S_SUB_LEAF]
	= {.func = 0x0000000d, .sub = 0x0000000c},
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
	[CPUID_00000018_00000001_DAT_SUB_LEAF_2]
	= {.func = 0x00000018, .sub = 0x00000002},
	[CPUID_00000018_00000001_DAT_SUB_LEAF_3]
	= {.func = 0x00000018, .sub = 0x00000003},
	[CPUID_00000018_00000001_DAT_SUB_LEAF_4]
	= {.func = 0x00000018, .sub = 0x00000004},
	[CPUID_00000019_00000000_KEY_LOCKER]
	= {.func = 0x00000019, .sub = 0x00000000},
	[CPUID_0000001A_00000000_HYBRID_INFORMATION]
	= {.func = 0x0000001a, .sub = 0x00000000},
	[CPUID_0000001B_00000000_PCONFIG_INFORMATION]
	= {.func = 0x0000001b, .sub = 0x00000000},
	[CPUID_0000001C_00000000_LAST_BRANCH_RECORDS]
	= {.func = 0x0000001c, .sub = 0x00000000},
	[CPUID_0000001D_00000000_TILE_PALETTE_MAIN_LEAF]
	= {.func = 0x0000001d, .sub = 0x00000000},
	[CPUID_0000001D_00000001_TILE_PALETTE_SUB_LEAF_1]
	= {.func = 0x0000001d, .sub = 0x00000001},
	[CPUID_0000001E_00000000_TMUL_MAIN_LEAF]
	= {.func = 0x0000001e, .sub = 0x00000000},
	[CPUID_0000001F_00000000_EXT_TOPOLOGY_V2]
	= {.func = 0x0000001f, .sub = 0x00000000},
	[CPUID_00000020_00000000_PROCESSOR_HRESET]
	= {.func = 0x00000020, .sub = 0x00000000},
	[CPUID_00000023_00000000_PM_EXT_MAIN_LEAF]
	= {.func = 0x00000023, .sub = 0x00000000},
	[CPUID_00000023_00000001_PM_EXT_SUB_LEAF_1]
	= {.func = 0x00000023, .sub = 0x00000001},
	[CPUID_00000023_00000002_PM_EXT_SUB_LEAF_2]
	= {.func = 0x00000023, .sub = 0x00000002},
	[CPUID_00000023_00000003_PM_EXT_SUB_LEAF_3]
	= {.func = 0x00000023, .sub = 0x00000003},
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
	[CPUID_8000001D_00000004_CACHE_PROPERTIES_DONE]
	= {.func = 0x8000001d, .sub = 0x00000004},
	[CPUID_8000001E_00000000_EXTENDED_IDENTIFIERS]
	= {.func = 0x8000001e, .sub = 0x00000000},
/* AMD Family 17h */
	[CPUID_8000001F_00000000_SECURE_ENCRYPTION]
	= {.func = 0x8000001f, .sub = 0x00000000},
	[CPUID_80000020_00000000_MBE_SUB_LEAF]
	= {.func = 0x80000020, .sub = 0x00000000},
	[CPUID_80000020_00000001_MBE_SUB_LEAF]
	= {.func = 0x80000020, .sub = 0x00000001},
/* AMD Family 19h Model 11h, Revision B1 */
	[CPUID_80000020_00000002_SMBE_SUB_LEAF]
	= {.func = 0x80000020, .sub = 0x00000002},
	[CPUID_80000020_00000003_BMEC_SUB_LEAF]
	= {.func = 0x80000020, .sub = 0x00000003},
/* AMD Family 19h */
	[CPUID_80000021_00000000_EXTENDED_FEATURE_2]
	= {.func = 0x80000021, .sub = 0x00000000},
	[CPUID_80000022_00000000_EXT_PERF_MON_DEBUG]
	= {.func = 0x80000022, .sub = 0x00000000},
/* AMD64 Architecture Programmer’s Manual rev 4.05 */
	[CPUID_80000023_00000000_MULTIKEY_ENCRYPTED_MEM]
	= {.func = 0x80000023, .sub = 0x00000000},
	[CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L0]
	= {.func = 0x80000026, .sub = 0x00000000},
	[CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L1]
	= {.func = 0x80000026, .sub = 0x00000001},
	[CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L2]
	= {.func = 0x80000026, .sub = 0x00000002},
	[CPUID_80000026_00000000_EXTENDED_CPU_TOPOLOGY_L3]
	= {.func = 0x80000026, .sub = 0x00000003},
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

static CLOCK BaseClock_GenuineIntel(unsigned int ratio) ;
static CLOCK BaseClock_AuthenticAMD(unsigned int ratio) ;
static CLOCK BaseClock_Core(unsigned int ratio) ;
static CLOCK BaseClock_Core2(unsigned int ratio) ;
static CLOCK BaseClock_Atom(unsigned int ratio) ;
static CLOCK BaseClock_Airmont(unsigned int ratio) ;
static CLOCK BaseClock_Silvermont(unsigned int ratio) ;
static CLOCK BaseClock_Nehalem(unsigned int ratio) ;
static CLOCK BaseClock_Westmere(unsigned int ratio) ;
static CLOCK BaseClock_SandyBridge(unsigned int ratio) ;
static CLOCK BaseClock_IvyBridge(unsigned int ratio) ;
static CLOCK BaseClock_Haswell(unsigned int ratio) ;
static CLOCK BaseClock_Skylake(unsigned int ratio) ;
static CLOCK BaseClock_AMD_Family_17h(unsigned int ratio) ;
#define BaseClock_AMD_Family_19h BaseClock_AMD_Family_17h
#define BaseClock_AMD_Family_1Ah BaseClock_AMD_Family_19h

static long Intel_Turbo_Config8C(CLOCK_ARG *pClockMod) ;
static long TurboClock_IvyBridge_EP(CLOCK_ARG *pClockMod) ;
static long TurboClock_Haswell_EP(CLOCK_ARG *pClockMod) ;
static long TurboClock_Broadwell_EP(CLOCK_ARG *pClockMod) ;
static long TurboClock_Skylake_X(CLOCK_ARG *pClockMod) ;
static long TurboClock_AMD_Zen(CLOCK_ARG *pClockMod) ;

static long ClockMod_Core2_PPC(CLOCK_ARG *pClockMod) ;
static long ClockMod_Nehalem_PPC(CLOCK_ARG *pClockMod) ;
static long ClockMod_SandyBridge_PPC(CLOCK_ARG *pClockMod) ;
static long ClockMod_Intel_HWP(CLOCK_ARG *pClockMod) ;
#define     ClockMod_Broadwell_EP_HWP ClockMod_Intel_HWP
#define     ClockMod_Skylake_HWP ClockMod_Intel_HWP
static long ClockMod_AMD_Zen(CLOCK_ARG *pClockMod) ;

static long Haswell_Uncore_Ratio(CLOCK_ARG *pClockMod) ;

static void Query_GenuineIntel(unsigned int cpu) ;
static void PerCore_Intel_Query(void *arg) ;
static void Start_GenuineIntel(void *arg) ;
static void Stop_GenuineIntel(void *arg) ;
static void InitTimer_GenuineIntel(unsigned int cpu) ;

static void Query_AuthenticAMD(unsigned int cpu) ;
static void PerCore_AuthenticAMD_Query(void *arg) ;
static void Start_AuthenticAMD(void *arg) ;
static void Stop_AuthenticAMD(void *arg) ;
static void InitTimer_AuthenticAMD(unsigned int cpu) ;

static void Query_Core2(unsigned int cpu) ;
static void PerCore_Core2_Query(void *arg) ;
static void Start_Core2(void *arg) ;
static void Stop_Core2(void *arg) ;
static void InitTimer_Core2(unsigned int cpu) ;

static void Query_Atom_Bonnell(unsigned int cpu) ;
static void PerCore_Atom_Bonnell_Query(void *arg) ;
#define     Start_Atom_Bonnell Start_Core2
#define     Stop_Atom_Bonnell Stop_Core2
#define     InitTimer_Atom_Bonnell InitTimer_Core2

static void Query_Silvermont(unsigned int cpu) ;
static void PerCore_Silvermont_Query(void *arg) ;
static void Start_Silvermont(void *arg) ;
static void Stop_Silvermont(void *arg) ;
static void InitTimer_Silvermont(unsigned int cpu) ;

static void Query_Goldmont(unsigned int cpu) ;
static void PerCore_Goldmont_Query(void *arg) ;
static void Start_Goldmont(void *arg) ;
static void Stop_Goldmont(void *arg) ;
static void InitTimer_Goldmont(unsigned int cpu) ;

static void Query_Airmont(unsigned int cpu) ;
static void PerCore_Airmont_Query(void *arg) ;

static void PerCore_Geminilake_Query(void *arg) ;
static void PerCore_Tremont_Query(void *arg) ;

static void Query_Nehalem(unsigned int cpu) ;
static void PerCore_Nehalem_Query(void *arg) ;
static void PerCore_Nehalem_EX_Query(void *arg) ;
static void Start_Nehalem(void *arg) ;
static void Stop_Nehalem(void *arg) ;
static void InitTimer_Nehalem(unsigned int cpu) ;
static void Start_Uncore_Nehalem(void *arg) ;
static void Stop_Uncore_Nehalem(void *arg) ;

static void Query_Nehalem_EX(unsigned int cpu) ;

static void Query_Avoton(unsigned int cpu) ;
static void PerCore_Avoton_Query(void *arg) ;

static void Query_SandyBridge(unsigned int cpu) ;
static void PerCore_SandyBridge_Query(void *arg) ;
static void Start_SandyBridge(void *arg) ;
static void Stop_SandyBridge(void *arg) ;
static void InitTimer_SandyBridge(unsigned int cpu) ;
static void Start_Uncore_SandyBridge(void *arg) ;
static void Stop_Uncore_SandyBridge(void *arg) ;

static void Query_SandyBridge_EP(unsigned int cpu) ;
static void PerCore_SandyBridge_EP_Query(void *arg) ;
static void Start_SandyBridge_EP(void *arg) ;
static void Stop_SandyBridge_EP(void *arg) ;
static void InitTimer_SandyBridge_EP(unsigned int cpu) ;
static void Start_Uncore_SandyBridge_EP(void *arg) ;
static void Stop_Uncore_SandyBridge_EP(void *arg) ;

static void Query_IvyBridge(unsigned int cpu) ;
static void PerCore_IvyBridge_Query(void *arg) ;

static void Query_IvyBridge_EP(unsigned int cpu) ;
static void PerCore_IvyBridge_EP_Query(void *arg) ;
static void Start_IvyBridge_EP(void *arg) ;
#define     Stop_IvyBridge_EP Stop_SandyBridge_EP
static void InitTimer_IvyBridge_EP(unsigned int cpu) ;
static void Start_Uncore_IvyBridge_EP(void *arg) ;
static void Stop_Uncore_IvyBridge_EP(void *arg) ;

static void Query_Haswell(unsigned int cpu) ;
static void PerCore_Haswell_Query(void *arg) ;

static void Query_Haswell_EP(unsigned int cpu) ;
static void PerCore_Haswell_EP_Query(void *arg) ;
static void Start_Haswell_EP(void *arg) ;
static void Stop_Haswell_EP(void *arg) ;
static void InitTimer_Haswell_EP(unsigned int cpu) ;
static void Start_Uncore_Haswell_EP(void *arg) ;
static void Stop_Uncore_Haswell_EP(void *arg) ;

static void Query_Haswell_ULT(unsigned int cpu) ;
static void PerCore_Haswell_ULT_Query(void *arg) ;
static void Start_Haswell_ULT(void *arg) ;
static void Stop_Haswell_ULT(void *arg) ;
static void InitTimer_Haswell_ULT(unsigned int cpu) ;
static void Start_Uncore_Haswell_ULT(void *arg) ;
static void Stop_Uncore_Haswell_ULT(void *arg) ;

static void Query_Haswell_ULX(unsigned int cpu) ;
static void PerCore_Haswell_ULX(void *arg) ;

static void Query_Broadwell(unsigned int cpu) ;
static void PerCore_Broadwell_Query(void *arg) ;
#define     Start_Broadwell Start_SandyBridge
#define     Stop_Broadwell Stop_SandyBridge
#define     InitTimer_Broadwell InitTimer_SandyBridge
#define     Start_Uncore_Broadwell Start_Uncore_SandyBridge
#define     Stop_Uncore_Broadwell Stop_Uncore_SandyBridge

static void Query_Broadwell_EP(unsigned int cpu) ;

static void Query_Skylake(unsigned int cpu) ;
static void PerCore_Skylake_Query(void *arg) ;
static void Start_Skylake(void *arg) ;
static void Stop_Skylake(void *arg) ;
static void InitTimer_Skylake(unsigned int cpu) ;
static void Start_Uncore_Skylake(void *arg) ;
static void Stop_Uncore_Skylake(void *arg) ;

static void Query_Skylake_X(unsigned int cpu) ;
static void PerCore_Skylake_X_Query(void *arg) ;
static void Start_Skylake_X(void *arg) ;
static void Stop_Skylake_X(void *arg) ;
static void InitTimer_Skylake_X(unsigned int cpu) ;
static void Start_Uncore_Skylake_X(void *arg) ;
static void Stop_Uncore_Skylake_X(void *arg) ;

static void InitTimer_Alderlake(unsigned int cpu) ;
static void Start_Alderlake(void *arg) ;
#define     Stop_Alderlake Stop_Skylake
static void Start_Uncore_Alderlake(void *arg) ;
static void Stop_Uncore_Alderlake(void *arg) ;

static void InitTimer_Arrowlake(unsigned int cpu) ;
static void Start_Arrowlake(void *arg) ;
#define     Stop_Arrowlake Stop_Skylake
static void Start_Uncore_Arrowlake(void *arg) ;
static void Stop_Uncore_Arrowlake(void *arg) ;

static void Power_ACCU_SKL_DEFAULT(PROC_RO *Pkg, unsigned int T) ;
static void Power_ACCU_SKL_PLATFORM(PROC_RO *Pkg, unsigned int T) ;
static void (*Power_ACCU_Skylake)(PROC_RO*,unsigned int)=Power_ACCU_SKL_DEFAULT;

static void Query_Kaby_Lake(unsigned int cpu) ;
static void PerCore_Kaby_Lake_Query(void *arg) ;

static void PerCore_Icelake_Query(void *arg) ;

static void PerCore_Tigerlake_Query(void *arg) ;

static void PerCore_Alderlake_Query(void *arg) ;

static void PerCore_Raptorlake_Query(void *arg) ;

static void PerCore_Meteorlake_Query(void *arg) ;

#define PerCore_Arrowlake_Query PerCore_Meteorlake_Query

static void Query_AMD_Family_0Fh(unsigned int cpu) ;
static void PerCore_AMD_Family_0Fh_Query(void *arg) ;
static void Start_AMD_Family_0Fh(void *arg) ;
static void Stop_AMD_Family_0Fh(void *arg) ;
static void InitTimer_AMD_Family_0Fh(unsigned int cpu) ;

static void Query_AMD_Family_10h(unsigned int cpu) ;
static void PerCore_AMD_Family_10h_Query(void *arg) ;
static void Start_AMD_Family_10h(void *arg) ;
static void Stop_AMD_Family_10h(void *arg) ;
#define     InitTimer_AMD_Family_10h InitTimer_AuthenticAMD

static void Query_AMD_Family_11h(unsigned int cpu) ;
static void PerCore_AMD_Family_11h_Query(void *arg) ;
static void Start_AMD_Family_11h(void *arg) ;
#define     Stop_AMD_Family_11h Stop_AMD_Family_10h
#define     InitTimer_AMD_Family_11h InitTimer_AuthenticAMD

static void Query_AMD_Family_12h(unsigned int cpu) ;
static void PerCore_AMD_Family_12h_Query(void *arg) ;
static void Start_AMD_Family_12h(void *arg) ;
#define     Stop_AMD_Family_12h Stop_AMD_Family_10h
#define     InitTimer_AMD_Family_12h InitTimer_AuthenticAMD

static void Query_AMD_Family_14h(unsigned int cpu) ;
static void PerCore_AMD_Family_14h_Query(void *arg) ;
static void Start_AMD_Family_14h(void *arg) ;
#define     Stop_AMD_Family_14h Stop_AMD_Family_10h
#define     InitTimer_AMD_Family_14h InitTimer_AuthenticAMD

static void Query_AMD_Family_15h(unsigned int cpu) ;
static void PerCore_AMD_Family_15h_Query(void *arg) ;
static void Start_AMD_Family_15h(void *arg) ;
#define     Stop_AMD_Family_15h Stop_AMD_Family_10h
static void InitTimer_AMD_Family_15h(unsigned int cpu) ;

#define     Query_AMD_Family_16h Query_AMD_Family_15h
static void PerCore_AMD_Family_16h_Query(void *arg) ;
#define     Start_AMD_Family_16h Start_AMD_Family_15h
#define     Stop_AMD_Family_16h Stop_AMD_Family_15h
#define     InitTimer_AMD_Family_16h InitTimer_AuthenticAMD

static void Exit_AMD_F17h(void) ;
static void Query_AMD_F17h_PerSocket(unsigned int cpu) ;
static void Query_AMD_F17h_PerCluster(unsigned int cpu) ;
static void PerCore_AMD_Family_17h_Query(void *arg) ;
static void Start_AMD_Family_17h(void *arg) ;
static void Stop_AMD_Family_17h(void *arg) ;
static void InitTimer_AMD_Family_17h(unsigned int cpu) ;
static void InitTimer_AMD_F17h_Zen(unsigned int cpu) ;
static void InitTimer_AMD_F17h_Zen2_SP(unsigned int cpu) ;
static void InitTimer_AMD_F17h_Zen2_MP(unsigned int cpu) ;
static void InitTimer_AMD_F17h_Zen2_APU(unsigned int cpu) ;
static void InitTimer_AMD_Zen3Plus_RMB(unsigned int cpu) ;
static void Start_Uncore_AMD_Family_17h(void *arg) ;
static void Stop_Uncore_AMD_Family_17h(void *arg) ;

static void Core_AMD_F17h_No_Thermal(CORE_RO *Core)
{
	UNUSED(Core);
}
static void CTL_AMD_Family_17h_Temp(CORE_RO *Core) ;
static void CCD_AMD_Family_17h_Zen2_Temp(CORE_RO *Core) ;
static void (*Core_AMD_Family_17h_Temp)(CORE_RO*) = Core_AMD_F17h_No_Thermal;

static void Pkg_AMD_Family_17h_Thermal(PROC_RO *Pkg, CORE_RO* Core)
{
	Core_AMD_Family_17h_Temp(Core);

	Pkg->PowerThermal.Sensor = Core->PowerThermal.Sensor;
}
static void Pkg_AMD_Family_19h_Genoa_Temp(PROC_RO *Pkg, CORE_RO* Core) ;
static void (*Pkg_AMD_Family_17h_Temp)(PROC_RO*, CORE_RO*) = \
	Pkg_AMD_Family_17h_Thermal;

static void Query_Hygon_F18h(unsigned int cpu);

#define     Exit_AMD_F19h Exit_AMD_F17h
#define     Query_AMD_F19h_PerSocket Query_AMD_F17h_PerSocket
#define     Query_AMD_F19h_PerCluster Query_AMD_F17h_PerCluster
#define     PerCore_AMD_Family_19h_Query PerCore_AMD_Family_17h_Query
#define     Start_AMD_Family_19h Start_AMD_Family_17h
#define     Stop_AMD_Family_19h Stop_AMD_Family_17h
#define     InitTimer_AMD_Family_19h InitTimer_AMD_Family_17h
#define     InitTimer_AMD_F19h_Zen3_SP InitTimer_AMD_F17h_Zen2_SP
#define     InitTimer_AMD_F19h_Zen3_MP InitTimer_AMD_F17h_Zen2_MP
#define     InitTimer_AMD_F19h_Zen3_APU InitTimer_AMD_F17h_Zen2_APU
#define     Start_Uncore_AMD_Family_19h Start_Uncore_AMD_Family_17h
#define     Stop_Uncore_AMD_Family_19h Stop_Uncore_AMD_Family_17h

static void CCD_AMD_Family_19h_Genoa_Temp(CORE_RO *Core) ;
static void CCD_AMD_Family_19h_Zen4_Temp(CORE_RO *Core) ;
static void Query_AMD_F19h_11h_PerCluster(unsigned int cpu) ;
static void Query_AMD_F19h_61h_PerCluster(unsigned int cpu) ;
static void InitTimer_AMD_Zen4_RPL(unsigned int cpu) ;

#define     Query_AMD_F19h_74h_PerSocket Query_AMD_F19h_61h_PerCluster
#define     InitTimer_AMD_Zen4_PHX InitTimer_AMD_Zen3Plus_RMB

static void InitTimer_AMD_Zen4_Genoa(unsigned int cpu) ;

#define     Exit_AMD_F1Ah Exit_AMD_F19h
#define     Query_AMD_F1Ah_PerCluster Query_AMD_F19h_PerCluster
#define     PerCore_AMD_Family_1Ah_Query PerCore_AMD_Family_19h_Query
#define     Start_AMD_Family_1Ah Start_AMD_Family_19h
#define     Stop_AMD_Family_1Ah Stop_AMD_Family_19h
#define     InitTimer_AMD_Family_1Ah InitTimer_AMD_Zen4_RPL
#define     Start_Uncore_AMD_Family_1Ah Start_Uncore_AMD_Family_19h
#define     Stop_Uncore_AMD_Family_1Ah Stop_Uncore_AMD_Family_19h

static void Query_AMD_F1Ah_24h_60h_70h_PerSocket(unsigned int cpu) ;
static void InitTimer_AMD_Zen5_STX(unsigned int cpu) ;

/*	[Void]								*/
#define _Void_Signature {.ExtFamily=0x0, .Family=0x0, .ExtModel=0x0, .Model=0x0}

/*	[Core]		06_0Eh (32 bits)				*/
#define _Core_Yonah	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x0, .Model=0xE}

/*	[Core2] 	06_0Fh, 06_15h, 06_16h, 06_17h, 06_1Dh		*/
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
/*	[SoFIA] 	06_5Dh						*/
#define _Atom_Sofia	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xD}
/*	[Merrifield]	06_4Ah						*/
#define _Atom_Merrifield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x4, .Model=0xA}
/*	[Moorefield]	06_5Ah						*/
#define _Atom_Moorefield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xA}

/*	[Denverton]	06_5Fh Stepping 0={A0,A1} 1={B0,B1}		*/
#define _Atom_Denverton {.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0xF}

/*	[Tremont/Jacobsville]	06_86h			[Snow Ridge]
	[Tremont/Lakefield]	06_8Ah
	[Sapphire Rapids]	06_8Fh				SPR
	[Emerald Rapids/X]	06_CFh			7 nm	EMR
	[Tremont/Elkhart Lake]	06_96h
	[Tremont/Jasper Lake]	06_9Ch					*/
#define _Tremont_Jacobsville \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0x6}
#define _Tremont_Lakefield \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xA}
#define _Sapphire_Rapids \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xF}
#define _Emerald_Rapids \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xC, .Model=0xF}
#define _Tremont_Elkhartlake \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0x6}
#define _Tremont_Jasperlake \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xC}

/*	[Granite Rapids/X]	06_ADh				GNR
	[Granite Rapids/D]	06_AEh
	[Sierra Forest] 	06_AFh				SRF
	[Grand Ridge]		06_B6h					*/
#define _Granite_Rapids_X \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0xD}
#define _Granite_Rapids_D \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0xE}
#define _Sierra_Forest \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0xF}
#define _Grand_Ridge \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0x6}

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
/*	[Skylake/X]	06_55h Stepping 4
	[Cascade Lake]	06_55h Stepping 7
	[Cooper Lake]	06_55h Stepping 10, 11				*/
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

/*	[Cannon Lake/U] 06_66h
	[Cannon Lake/H] 06_67h						*/
#define _Cannonlake_U	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0x6}
#define _Cannonlake_H	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0x7}

/*	[Spreadtrum]	06_75h	SC9853I-IA				*/
#define _Spreadtrum	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0x5}

/*	[Gemini Lake]	06_7Ah						*/
#define _Geminilake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xA}

/*	[Ice Lake]	06_7Dh
	[Ice Lake/UY]	06_7Eh
	[Ice Lake/X]	06_6Ah Stepping 5
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

/*	[Comet Lake]	06_A5h
	[Comet Lake/UL]	06_A6h
	[Rocket Lake]	06_A7h
	[Rocket Lake/U] 06_A8h						*/
#define _Cometlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0x5}
#define _Cometlake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0x6}
#define _Rocketlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0x7}
#define _Rocketlake_U	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0x8}

/*	[AlderLake/S]	06_97h
	[AlderLake/H]	06_9Ah
	[AlderLake/N]	06_BEh						*/
#define _Alderlake_S	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0x7}
#define _Alderlake_H	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xA}
#define _Alderlake_N	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0xE}

/*	[MeteorLake/M]	06_AAh
	[MeteorLake/N]	06_ABh
	[MeteorLake/S]	06_ACh
	[RaptorLake]	06_B7h
	[RaptorLake/P]	06_BAh
	[RaptorLake/S]	06_BFh						*/
#define _Meteorlake_M	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0xA}
#define _Meteorlake_N	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0xB}
#define _Meteorlake_S	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xA, .Model=0xC}
#define _Raptorlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0x7}
#define _Raptorlake_P	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0xA}
#define _Raptorlake_S	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0xF}

/*	[LunarLake]	06_BDh
	[ArrowLake]	06_C6h
	[ArrowLake/H]	06_C5h
	[ArrowLake/U]	06_B5h						*/
#define _Lunarlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0xD}
#define _Arrowlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xC, .Model=0x6}
#define _Arrowlake_H	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xC, .Model=0x5}
#define _Arrowlake_U	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xB, .Model=0x5}

/*	[PantherLake]	06_CCh						*/
#define _Pantherlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xC, .Model=0xC}

/*	[Clearwater Forest] 06_DDh					*/
#define _Clearwater_Forest \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0xD, .Model=0xD}

/*	[BartlettLake/S] 06_D7h 					*/
#define _Bartlettlake_S {.ExtFamily=0x0, .Family=0x6, .ExtModel=0xD, .Model=0x7}

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
	[EPYC/Snowy Owl] 	8F_01h Stepping 2	14 nm	SVR
	[Zen+ Pinnacle Ridge] 	8F_08h Stepping 2	12 nm
	[Zen+ Colfax]		8F_08h Stepping 2	12 nm	HEDT
	[Zen/Raven Ridge]	8F_11h Stepping 0	14 nm	APU
	[Zen+ Picasso]		8F_18h Stepping 1	12 nm	APU
	[Zen/Dali]		8F_20h Stepping 1	14 nm	APU/Raven2
	[EPYC/Rome]		8F_31h Stepping 0	 7 nm	SVR
	[Zen2/Castle Peak]	8F_31h Stepping 0	 7 nm	HEDT
	[Zen2/Renoir]		8F_60h Stepping 1	 7 nm	APU
	[Zen2/Lucienne] 	8F_68h Stepping 1	 7 nm	APU
	[Zen2/Matisse]		8F_71h Stepping 0	 7 nm
	[Zen2/Xbox		8F_74h Stepping 0	 7 nm
	[Zen2/Van Gogh] 	8F_90h Stepping 1	 7 nm	Valve Jupiter
	[Zen2/Van Gogh] 	8F_91h Stepping 0	 6 nm	Valve Galileo
	[Zen2/Mendocino]	8F_A0h Stepping 0	 6 nm	[MDN]	*/
#define _AMD_Zen	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x1}
#define _AMD_Zen_APU	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x1, .Model=0x1}
#define _AMD_ZenPlus	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x8}
#define _AMD_ZenPlus_APU {.ExtFamily=0x8,.Family=0xF, .ExtModel=0x1, .Model=0x8}
#define _AMD_Zen_Dali	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x2, .Model=0x0}
#define _AMD_EPYC_Rome_CPK	\
			{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x3, .Model=0x1}

#define _AMD_Zen2_Renoir {.ExtFamily=0x8,.Family=0xF, .ExtModel=0x6, .Model=0x0}
#define _AMD_Zen2_LCN	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x6, .Model=0x8}
#define _AMD_Zen2_MTS	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x7, .Model=0x1}
#define _AMD_Zen2_Ariel {.ExtFamily=0x8, .Family=0xF, .ExtModel=0x7, .Model=0x4}

#define _AMD_Zen2_Jupiter	\
			{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x9, .Model=0x0}

#define _AMD_Zen2_Galileo	\
			{.ExtFamily=0x8, .Family=0xF, .ExtModel=0x9, .Model=0x1}

#define _AMD_Zen2_MDN	{.ExtFamily=0x8, .Family=0xF, .ExtModel=0xA, .Model=0x0}

#define _AMD_Family_17h {.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 18h]		9F_00h
	[Hygon/Dhyana]		Stepping 1		14 nm
					"Hygon C86 XXXX NN-core Processor"
					7189(32),7185(32),7169(24),7165(24),
					7159(16),7155(16),7151(16),
					5280(16),
					5188(16),5185(16),5168(12),5165(12),
					5138( 8),5131( 8),
					3250( 8),
					3168( 6),3165( 6),
					3135( 4),3131( 4),3120( 4),
	[Hygon/Dhyana]		Stepping 2
					3188( 8),3185( 8),3138( 4),
	[Family 18h]		9F_01h
	[Hygon/Dhyana]		Stepping 1		14 nm
					"Hygon C86 XXXX NN-core Processor"
					7285(32),7280(64),7265(24),
					5285(16),
					3285( 8),3280( 8),3230( 4),
	[Family 18h]		9F_02h
				Stepping 2
					"Hygon C86 7375"		*/
#define _Hygon_Family_18h	\
			{.ExtFamily=0x9, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 19h]		AF_00h
	[Zen3/Vermeer]		AF_21h Stepping 0	 7 nm
	[Zen3/Cezanne]		AF_50h Stepping 0	 7 nm
	[EPYC/Milan]		AF_01h Stepping 1	 7 nm	[Genesis][GN]
	[EPYC/Milan-X]		AF_01h Stepping 2	 7 nm	SVR
	[Zen3/Chagall]		AF_08h Stepping 2	 7 nm	HEDT/TRX4
	[Zen3/Badami]		AF_30h			 7 nm	[BA]/SVR
	[Zen3+ Rembrandt]	AF_44h Stepping 1	 6 nm	[RMB]
	[Zen4/Genoa]		AF_11h Stepping 1	 5 nm	[GNA]/SVR
	[Zen4/Raphael]		AF_61h Stepping 2	 5 nm	[RPL]
	[Zen4/Dragon Range]	AF_61h Stepping 2	 5 nm	FL1
	[Zen4/Phoenix Point]	AF_74h			 4 nm	[PHX]
	[Zen4/Phoenix-R]	AF_75h			 4 nm	[PHX]
	[Zen4c/Phoenix2]	AF_78h			 4 nm	[PHX2]
	[Zen4c/Hawk Point]	AF_7Ch			 4 nm	[HWK]
	[Zen4c][Bergamo][Siena] AF_A0h Stepping [1][2]	 5 nm	SVR
	[Zen4/Storm Peak]	AF_18h Stepping 1	 5 nm	WS/SP6

	[Family 1Ah]		BF_00h
	[Zen5/5c/Strix Point]	BF_24h			 4 nm	[STX]/FP8
	[Zen5/Granite Ridge]	BF_44h			 4 nm	Eldora
	[Zen5/Turin]		BF_02h Stepping 1	 4 nm	SP5
	[Zen5c/Turin]		BF_11h Stepping 0	 3 nm	SP5
	[Zen5/5c/Krackan Point] BF_60h Stepping 0	 4 nm	[KRK]/FP8
	[Zen5/Strix Halo]	BF_70h Stepping 0	 4 nm	[STXH]/FP11
	[Zen5/Shimada Peak]	BF_08h Stepping 1	 4 nm	HEDT/sTR5 */
#define _AMD_Family_19h {.ExtFamily=0xA, .Family=0xF, .ExtModel=0x0, .Model=0x0}
#define _AMD_Zen3_VMR	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x2, .Model=0x1}
#define _AMD_Zen3_CZN	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x5, .Model=0x0}
#define _AMD_EPYC_Milan {.ExtFamily=0xA, .Family=0xF, .ExtModel=0x0, .Model=0x1}
#define _AMD_Zen3_Chagall	\
			{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x0, .Model=0x8}
#define _AMD_Zen3_Badami	\
			{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x3, .Model=0x0}
#define _AMD_Zen3Plus_RMB	\
			{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x4, .Model=0x4}

#define _AMD_Zen4_Genoa {.ExtFamily=0xA, .Family=0xF, .ExtModel=0x1, .Model=0x1}
#define _AMD_Zen4_RPL	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x6, .Model=0x1}
#define _AMD_Zen4_PHX	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x7, .Model=0x4}
#define _AMD_Zen4_PHXR	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x7, .Model=0x5}
#define _AMD_Zen4_PHX2	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x7, .Model=0x8}
#define _AMD_Zen4_HWK	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x7, .Model=0xC}
#define _AMD_Zen4_Bergamo	\
			{.ExtFamily=0xA, .Family=0xF, .ExtModel=0xa, .Model=0x0}

#define _AMD_Zen4_STP	{.ExtFamily=0xA, .Family=0xF, .ExtModel=0x1, .Model=0x8}

#define _AMD_Family_1Ah {.ExtFamily=0xB, .Family=0xF, .ExtModel=0x0, .Model=0x0}
#define _AMD_Zen5_STX	{.ExtFamily=0xB, .Family=0xF, .ExtModel=0x2, .Model=0x4}
#define _AMD_Zen5_Eldora	\
			{.ExtFamily=0xB, .Family=0xF, .ExtModel=0x4, .Model=0x4}

#define _AMD_Zen5_Turin {.ExtFamily=0xB, .Family=0xF, .ExtModel=0x0, .Model=0x2}
#define _AMD_Zen5_Turin_Dense	\
			{.ExtFamily=0xB, .Family=0xF, .ExtModel=0x1, .Model=0x1}

#define _AMD_Zen5_KRK	{.ExtFamily=0xB, .Family=0xF, .ExtModel=0x6, .Model=0x0}
#define _AMD_Zen5_STXH	{.ExtFamily=0xB, .Family=0xF, .ExtModel=0x7, .Model=0x0}
#define _AMD_Zen5_SHP	{.ExtFamily=0xB, .Family=0xF, .ExtModel=0x0, .Model=0x8}

typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

#if defined(ARCH_PMC)
PCI_CALLBACK GetMemoryBAR(int M, int B, int D, int F, unsigned int offset,
			unsigned int bsize, unsigned long long wsize,
			unsigned short range,
			struct pci_dev **device, void __iomem **memmap);

void PutMemoryBAR(struct pci_dev **device, void __iomem **memmap) ;
#endif /* ARCH_PMC */

static PCI_CALLBACK P945(struct pci_dev *dev) ;
static PCI_CALLBACK P955(struct pci_dev *dev) ;
static PCI_CALLBACK P965(struct pci_dev *dev) ;
static PCI_CALLBACK G965(struct pci_dev *dev) ;
static PCI_CALLBACK P35(struct pci_dev *dev) ;
static PCI_CALLBACK SoC_SLM(struct pci_dev *dev) ;
static PCI_CALLBACK SoC_AMT(struct pci_dev *dev) ;
static PCI_CALLBACK Nehalem_IMC(struct pci_dev *dev) ;
#define Bloomfield_IMC Nehalem_IMC
static PCI_CALLBACK Lynnfield_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK Jasper_Forest_IMC(struct pci_dev *dev) ;
#define Westmere_EP_IMC Nehalem_IMC
static PCI_CALLBACK NHM_IMC_TR(struct pci_dev *dev) ;
static PCI_CALLBACK NHM_NON_CORE(struct pci_dev *dev) ;
static PCI_CALLBACK X58_VTD(struct pci_dev *dev) ;
static PCI_CALLBACK X58_QPI(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK IVB_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_HB(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_QPI(struct pci_dev *dev) ;
static PCI_CALLBACK SNB_EP_CAP(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_CLK(struct pci_dev *dev) ;
#define HSW_EP_HB SNB_EP_HB
#define HSW_EP_QPI SNB_EP_QPI
#define HSW_EP_CAP SNB_EP_CAP
static PCI_CALLBACK HSW_EP_CTRL0(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_CTRL1(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL0_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_IMC_CTRL1_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL0_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA0(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA1(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA2(struct pci_dev *dev) ;
static PCI_CALLBACK HSW_EP_TAD_CTRL1_CHA3(struct pci_dev *dev) ;
static PCI_CALLBACK SKL_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK CML_PCH(struct pci_dev *dev) ;
#define RKL_PCH CML_PCH
static PCI_CALLBACK RKL_IMC(struct pci_dev *dev) ;
static PCI_CALLBACK TGL_IMC(struct pci_dev *dev) ;
#define TGL_PCH CML_PCH
static PCI_CALLBACK ADL_IMC(struct pci_dev *dev) ;
#define ADL_PCH CML_PCH
static PCI_CALLBACK GLK_IMC(struct pci_dev *dev) ;
#define RPL_IMC ADL_IMC
#define RPL_PCH CML_PCH
static PCI_CALLBACK MTL_IMC(struct pci_dev *dev) ;
#define MTL_PCH CML_PCH
#define ARL_IMC MTL_IMC
#define ARL_PCH MTL_PCH
#define LNL_IMC MTL_IMC
#define LNL_PCH MTL_PCH
static PCI_CALLBACK AMD_0Fh_MCH(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0Fh_HTT(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_Zen_IOMMU(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_DataFabric_Zeppelin(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Raven(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Matisse(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Starship(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Renoir(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Ariel(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Raven2(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Fireflight(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Arden(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_VanGogh(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Vermeer(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Cezanne(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Rembrandt(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Raphael(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Genoa(struct pci_dev *pdev) ;
static PCI_CALLBACK AMD_DataFabric_Phoenix(struct pci_dev *pdev) ;
#define AMD_DataFabric_Turin AMD_DataFabric_Genoa
#define AMD_DataFabric_Strix_Point AMD_DataFabric_Phoenix

static struct pci_device_id PCI_Void_ids[] = {
	{0, }
};

static struct pci_device_id PCI_Core2_ids[] = {
	{	/* 82945G - Lakeport					*/
		PCI_VDEVICE(INTEL, DID_INTEL_82945P_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	/* 82945GM - Calistoga					*/
		PCI_VDEVICE(INTEL, DID_INTEL_82945GM_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	/* 82945GME/SE - Calistoga				*/
		PCI_VDEVICE(INTEL, DID_INTEL_82945GME_HB),
		.driver_data = (kernel_ulong_t) P945
	},
	{	/* 82955X - Lakeport-X					*/
		PCI_VDEVICE(INTEL, DID_INTEL_82955_HB),
		.driver_data = (kernel_ulong_t) P955
	},
	{	/* 946PL/946GZ - Lakeport-PL/GZ				*/
		PCI_VDEVICE(INTEL, DID_INTEL_82946GZ_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	/* Q963/Q965 - Broadwater				*/
		PCI_VDEVICE(INTEL, DID_INTEL_82965Q_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	/* P965/G965 - Broadwater				*/
		PCI_VDEVICE(INTEL, DID_INTEL_82965G_HB),
		.driver_data = (kernel_ulong_t) P965
	},
	{	/* GM965 - Crestline					*/
		PCI_VDEVICE(INTEL, DID_INTEL_82965GM_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	/* GME965 - Crestline					*/
		PCI_VDEVICE(INTEL, DID_INTEL_82965GME_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	/* GM45 - Cantiga					*/
		PCI_VDEVICE(INTEL, DID_INTEL_GM45_HB),
		.driver_data = (kernel_ulong_t) G965
	},
	{	/* Q35 - Bearlake-Q					*/
		PCI_VDEVICE(INTEL, DID_INTEL_Q35_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* P35/G33 - Bearlake-PG+				*/
		PCI_VDEVICE(INTEL, DID_INTEL_G33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* Q33 - Bearlake-QF					*/
		PCI_VDEVICE(INTEL, DID_INTEL_Q33_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* X38/X48 - Bearlake-X					*/
		PCI_VDEVICE(INTEL, DID_INTEL_X38_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* 3200/3210 - Intel 3200				*/
		PCI_VDEVICE(INTEL, DID_INTEL_3200_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* Q45/Q43 - Eaglelake-Q				*/
		PCI_VDEVICE(INTEL, DID_INTEL_Q45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* P45/G45 - Eaglelake-P				*/
		PCI_VDEVICE(INTEL, DID_INTEL_G45_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* G41 - Eaglelake-G					*/
		PCI_VDEVICE(INTEL, DID_INTEL_G41_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{	/* N10 - Atom N400/N500				*/
		PCI_VDEVICE(INTEL, DID_INTEL_BONNELL_HB),
		.driver_data = (kernel_ulong_t) P35
	},
	{0, }
};

static struct pci_device_id PCI_SoC_ids[] = {
	{	/* Saltwell - Cedarview					*/
		PCI_VDEVICE(INTEL, DID_INTEL_SALTWELL_HB),
		.driver_data = (kernel_ulong_t) SoC_SLM
	},
	{	/* 82945G - Lakeport					*/
		PCI_VDEVICE(INTEL, DID_INTEL_SLM_PTR),
		.driver_data = (kernel_ulong_t) SoC_SLM
	},
	{	/* Atom - Airmont					*/
		PCI_VDEVICE(INTEL, DID_INTEL_AIRMONT_HB),
		.driver_data = (kernel_ulong_t) SoC_AMT
	},
	{0, }
};

/* 1st Generation							*/
static struct pci_device_id PCI_Nehalem_QPI_ids[] = {
	{	/* Bloomfield IMC					*/
		PCI_VDEVICE(INTEL, DID_INTEL_I7_MCR),
		.driver_data = (kernel_ulong_t) Bloomfield_IMC
	},
	{	/* Bloomfield IMC Test Registers			*/
		PCI_VDEVICE(INTEL, DID_INTEL_I7_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	/* Nehalem Control Status and RAS Registers		*/
		PCI_VDEVICE(INTEL, DID_INTEL_X58_HUB_CTRL),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_X58_HUB_CORE),
		.driver_data = (kernel_ulong_t) X58_VTD
	},
	{	/* Nehalem Bloomfield/Xeon C3500: Non-Core Registers	*/
		PCI_VDEVICE(INTEL, DID_INTEL_BLOOMFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{	/* Nehalem EP Xeon C5500: Non-Core Registers		*/
		PCI_VDEVICE(INTEL, DID_INTEL_C5500_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

static struct pci_device_id PCI_Nehalem_DMI_ids[] = {
	{	/* Lynnfield IMC					*/
		PCI_VDEVICE(INTEL, DID_INTEL_LYNNFIELD_MCR),
		.driver_data = (kernel_ulong_t) Lynnfield_IMC
	},
	{	/* Lynnfield IMC Test Registers				*/
		PCI_VDEVICE(INTEL, DID_INTEL_LYNNFIELD_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{ /* Lynnfield QuickPath Architecture Generic Non-core Registers */
		PCI_VDEVICE(INTEL, DID_INTEL_LYNNFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{ /* Clarksfield Processor Uncore Device 0, Function 0		*/
		PCI_VDEVICE(INTEL, DID_INTEL_CLARKSFIELD_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{ /* Westmere/Clarkdale QuickPath Architecture Non-core Registers */
		PCI_VDEVICE(INTEL, DID_INTEL_CLARKDALE_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{	/* Nehalem/C5500-C3500/Jasper Forest IMC		*/
		PCI_VDEVICE(INTEL, DID_INTEL_NHM_EC_MCR),
		.driver_data = (kernel_ulong_t) Jasper_Forest_IMC
	},
	{	/* Nehalem/C5500-C3500 IMC Test Registers		*/
		PCI_VDEVICE(INTEL, DID_INTEL_NHM_EC_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	/* Nehalem EP Xeon C5500: Non-Core Registers		*/
		PCI_VDEVICE(INTEL, DID_INTEL_C5500_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_IIO_CORE_REG),
		.driver_data = (kernel_ulong_t) X58_VTD
	},
	{0, }
};

static struct pci_device_id PCI_Westmere_EP_ids[] = {
	{	/* Westmere EP IMC */
		PCI_VDEVICE(INTEL, DID_INTEL_NHM_EP_MCR),
		.driver_data = (kernel_ulong_t) Westmere_EP_IMC
	},
	{	/* Westmere EP IMC Test Registers			*/
		PCI_VDEVICE(INTEL, DID_INTEL_NHM_EP_MC_TEST),
		.driver_data = (kernel_ulong_t) NHM_IMC_TR
	},
	{	/* Nehalem Control Status and RAS Registers		*/
		PCI_VDEVICE(INTEL, DID_INTEL_X58_HUB_CTRL),
		.driver_data = (kernel_ulong_t) X58_QPI
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_X58_HUB_CORE),
		.driver_data = (kernel_ulong_t) X58_VTD
	},
	{	/* Westmere EP: Non-Core Registers			*/
		PCI_VDEVICE(INTEL, DID_INTEL_NHM_EP_NON_CORE),
		.driver_data = (kernel_ulong_t) NHM_NON_CORE
	},
	{0, }
};

/* 2nd Generation
	Sandy Bridge ix-2xxx, Xeon E3-E5: IMC_HA=0x3ca0 / IMC_TA=0x3ca8
	TA0=0x3caa, TA1=0x3cab / TA2=0x3cac / TA3=0x3cad / TA4=0x3cae	*/
static struct pci_device_id PCI_SandyBridge_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SNB_IMC_HA0),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0100,0x0104		*/
		PCI_VDEVICE(INTEL, DID_INTEL_SNB_IMC_SA),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SNB_IMC_0104),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{0, }
};

/* 3rd Generation
	Ivy Bridge ix-3xxx, Xeon E7/E5 v2: IMC_HA=0x0ea0 / IMC_TA=0x0ea8
	TA0=0x0eaa / TA1=0x0eab / TA2=0x0eac / TA3=0x0ead		*/
static struct pci_device_id PCI_IvyBridge_ids[] = {
	{	/* Desktop: IMC_SystemAgent=0x0150			*/
		PCI_VDEVICE(INTEL, DID_INTEL_IVB_IMC_SA),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{	/* Mobile i5-3337U: IMC=0x0154				*/
		PCI_VDEVICE(INTEL, DID_INTEL_IVB_IMC_0154),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{0, }
};

static struct pci_device_id PCI_SandyBridge_EP_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SNB_EP_HOST_BRIDGE),
		.driver_data = (kernel_ulong_t) SNB_EP_HB
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_IVB_EP_HOST_BRIDGE),
		.driver_data = (kernel_ulong_t) SNB_EP_HB
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_IVB_EP_IIO_VTD),
		.driver_data = (kernel_ulong_t) X58_VTD
	},
	{0, }
};

/* 4th Generation
	Haswell ix-4xxx, Xeon E7/E5 v3: IMC_HA0=0x2fa0 / IMC_HA0_TA=0x2fa8
	TAD0=0x2faa / TAD1=0x2fab / TAD2=0x2fac / TAD3=0x2fad		*/
static struct pci_device_id PCI_Haswell_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HASWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0c00			*/
		PCI_VDEVICE(INTEL, DID_INTEL_HASWELL_IMC_SA),
		.driver_data = (kernel_ulong_t) HSW_CLK
	},
	{	/* Mobile M/H: Host Agent=0x0c04			*/
		PCI_VDEVICE(INTEL, DID_INTEL_HASWELL_MH_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_CLK
	},
	{	/* Mobile U/Y: Host Agent=0x0a04			*/
		PCI_VDEVICE(INTEL, DID_INTEL_HASWELL_UY_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
/*				Haswell-EP				*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_HOST_BRIDGE),
		.driver_data = (kernel_ulong_t) HSW_EP_HB
	},
/*	QPIMISCSTAT							*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_QPI_LINK0),
		.driver_data = (kernel_ulong_t) HSW_EP_QPI
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_QPI_LINK2),
		.driver_data = (kernel_ulong_t) HSW_EP_QPI
	},
/*	Power Control Unit						*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_CAPABILITY),
		.driver_data = (kernel_ulong_t) HSW_EP_CAP
	},
/*	Integrated Memory Controller # : IMC Configuration Registers	*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_E7_IMC_CTRL0_F0_CPGC),
		.driver_data = (kernel_ulong_t) HSW_EP_CTRL0
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_E7_IMC_CTRL0_F1_CPGC),
		.driver_data = (kernel_ulong_t) HSW_EP_CTRL0
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_E7_IMC_CTRL1_F0_CPGC),
		.driver_data = (kernel_ulong_t) HSW_EP_CTRL1
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_E7_IMC_CTRL1_F1_CPGC),
		.driver_data = (kernel_ulong_t) HSW_EP_CTRL1
	},
/*	Integrated Memory Controller # : Channel [m-M] Thermal Registers*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL0_CH0),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL0_CHA0
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL0_CH1),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL0_CHA1
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL0_CH2),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL0_CHA2
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL0_CH3),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL0_CHA3
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL1_CH0),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL1_CHA0
	},
	{
		PCI_VDEVICE(INTEL ,DID_INTEL_HSW_EP_IMC_CTRL1_CH1),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL1_CHA1
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL1_CH2),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL1_CHA2
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_IMC_CTRL1_CH3),
		.driver_data = (kernel_ulong_t) HSW_EP_IMC_CTRL1_CHA3
	},
/*	Integrated Memory Controller 0 : Channel # TAD Registers	*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL0_CH0),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL0_CHA0
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL0_CH1),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL0_CHA1
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL0_CH2),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL0_CHA2
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL0_CH3),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL0_CHA3
	},
	{
/*	Integrated Memory Controller 1 : Channel # TAD Registers	*/
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL1_CH0),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL1_CHA0
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL1_CH1),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL1_CHA1
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL1_CH2),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL1_CHA2
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_HSW_EP_TAD_CTRL1_CH3),
		.driver_data = (kernel_ulong_t) HSW_EP_TAD_CTRL1_CHA3
	},
	{0, }
};

/* 5th Generation
	Broadwell ix-5xxx: IMC_HA0=0x1604 / 0x1614			*/
static struct pci_device_id PCI_Broadwell_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_BROADWELL_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_BROADWELL_D_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_BROADWELL_H_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_BROADWELL_U_IMC_HA0),
		.driver_data = (kernel_ulong_t) HSW_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0c00			*/
		PCI_VDEVICE(INTEL, DID_INTEL_HASWELL_IMC_SA),
		.driver_data = (kernel_ulong_t) HSW_CLK
	},
	{0, }
};

/* 6th Generation							*/
static struct pci_device_id PCI_Skylake_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_U_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_Y_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_S_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_H_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_SKYLAKE_DT_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{0, }
};

static struct pci_device_id PCI_Skylake_X_ids[] = {
	{0, }
};

/* 7th & 8th up to 11th Generation: Chipsets might cross generations	*/
static struct pci_device_id PCI_Kabylake_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_H_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_U_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_Y_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_Y_IMC_HQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_S_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_DT_IMC_HA),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_U_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_KABYLAKE_X_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_S_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_S_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_U_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_U_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_H_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_H_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_H_IMC_HAO),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_W_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_W_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_W_IMC_HAO),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_S_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_S_IMC_HAS),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COFFEELAKE_R_S_IMC_HAO),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_WHISKEYLAKE_U_IMC_HAD),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_WHISKEYLAKE_U_IMC_HAQ),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_CANNONLAKE_U_IMC_HB),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_S_IMC_6C),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_S_IMC_10C),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_H_IMC_10C),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_W_IMC_10C),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_M_IMC_6C),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_U_IMC_HB),
		.driver_data = (kernel_ulong_t) RKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_U1_IMC),
		.driver_data = (kernel_ulong_t) RKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_U3_IMC),
		.driver_data = (kernel_ulong_t) RKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_S1_IMC),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_S2_IMC),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_S5_IMC),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_PREM_U_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_BASE_U_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_U_ES_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_Y_ES_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_Y_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_H470_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_Z490_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_Q470_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_HM470_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_QM480_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_WM490_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_COMETLAKE_W480_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ICELAKE_U_PCH),
		.driver_data = (kernel_ulong_t) CML_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ICELAKE_U_IMC),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ICELAKE_U_4C),
		.driver_data = (kernel_ulong_t) SKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_U1_IMC),
		.driver_data = (kernel_ulong_t) TGL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_U2_IMC),
		.driver_data = (kernel_ulong_t) TGL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_U3_IMC),
		.driver_data = (kernel_ulong_t) TGL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_U4_IMC),
		.driver_data = (kernel_ulong_t) TGL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_H_IMC),
		.driver_data = (kernel_ulong_t) TGL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_UP3_IMC),
		.driver_data = (kernel_ulong_t) TGL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_TIGERLAKE_UP4_IMC),
		.driver_data = (kernel_ulong_t) TGL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_S_8C_IMC_HB),
		.driver_data = (kernel_ulong_t) RKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_S_6C_IMC_HB),
		.driver_data = (kernel_ulong_t) RKL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_H510_PCH),
		.driver_data = (kernel_ulong_t) RKL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_B560_PCH),
		.driver_data = (kernel_ulong_t) RKL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_H570_PCH),
		.driver_data = (kernel_ulong_t) RKL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_Q570_PCH),
		.driver_data = (kernel_ulong_t) RKL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_Z590_PCH),
		.driver_data = (kernel_ulong_t) RKL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ROCKETLAKE_W580_PCH),
		.driver_data = (kernel_ulong_t) RKL_PCH
	},
	{0, }
};

/* 12th Generation							*/
static struct pci_device_id PCI_ADL_RPL_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_S_8P_8E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_S_8P_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_S_6P_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_S_6P_0E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_S_4P_0E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_S_2P_0E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_H_6P_8E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_H_6P_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_H_4P_8E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_H_4P_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_HL_6P_8E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_U_2P_8E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_U_2P_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_U_1P_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N300_8E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N200_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N100_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7835RE_8C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7433RE_4C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N97_4E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7425E_4C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N50_2E_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7213RE_2C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7211RE_2C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7213E_2C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7211E_2C_HB),
		.driver_data = (kernel_ulong_t) ADL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_H610_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_B660_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_H670_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_Z690_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_Q670_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_W680_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_WM690_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_HM670_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_PCH_P),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_PCH_U),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N305_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_N95_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7000E_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ALDERLAKE_X7000RE_PCH),
		.driver_data = (kernel_ulong_t) ADL_PCH
	},
/* 13th Generation							*/
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_8P_16E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_8P_12E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_8P_8E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_6P_8E_HB1),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_6P_4E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_6P_8E_HB2),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_S_4P_0E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_8P_16E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_8P_12E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_8P_8E_HB1),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_6P_8E_HB1),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_6P_4E_HB1),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_8P_8E_HB2),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_6P_8E_HB2),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HX_6P_4E_HB2),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_H_6P_8E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_H_4P_8E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_H_6P_4E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_H_4P_4E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_U_2P_8E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_U_2P_4E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_U_1P_4E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_E_8P_0E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_E_6P_0E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_E_4P_0E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_PX_6P_8E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_PX_4P_8E_HB),
		.driver_data = (kernel_ulong_t) RPL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_Z790_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_H770_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_B760_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_WM790_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_HM770_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_C262_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_RAPTORLAKE_C266_PCH),
		.driver_data = (kernel_ulong_t) RPL_PCH
	},
	{0, }
};

static struct pci_device_id PCI_Geminilake_ids[] = {
	{	/* Goldmont Plus					*/
		PCI_VDEVICE(INTEL, DID_INTEL_GEMINILAKE_HB),
		.driver_data = (kernel_ulong_t) GLK_IMC
	},
	{0, }
};

/* Meteor Lake								*/
static struct pci_device_id PCI_MTL_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_UT4_2_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_H_6_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_U_2_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_H_4_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_U_2_4_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_M_6_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_M_4_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_M_2_8_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_M_4_4_2_HB),
		.driver_data = (kernel_ulong_t) MTL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_H_PCH),
		.driver_data = (kernel_ulong_t) MTL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_U_PCH),
		.driver_data = (kernel_ulong_t) MTL_PCH
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_METEORLAKE_UT4_PCH),
		.driver_data = (kernel_ulong_t) MTL_PCH
	},
/* Arrow Lake */
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ARROWLAKE_S_8_16_HB),
		.driver_data = (kernel_ulong_t) ARL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ARROWLAKE_S_8_12_HB),
		.driver_data = (kernel_ulong_t) ARL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ARROWLAKE_S_6_8_HB),
		.driver_data = (kernel_ulong_t) ARL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_ARROWLAKE_S_PCH),
		.driver_data = (kernel_ulong_t) ARL_PCH
	},
	{0, }
};

/* Lunar Lake								*/
static struct pci_device_id PCI_LNL_ids[] = {
	{
		PCI_VDEVICE(INTEL, DID_INTEL_LUNARLAKE_V_4P_4E),
		.driver_data = (kernel_ulong_t) LNL_IMC
	},
	{
		PCI_VDEVICE(INTEL, DID_INTEL_LUNARLAKE_V_PCH),
		.driver_data = (kernel_ulong_t) LNL_PCH
	},
	{0, }
};

/* AMD Family 0Fh							*/
static struct pci_device_id PCI_AMD_0Fh_ids[] = {
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, DID_AMD_K8_NB_MEMCTL),
		.driver_data = (kernel_ulong_t) AMD_0Fh_MCH
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_AMD, DID_AMD_K8_NB),
		.driver_data = (kernel_ulong_t) AMD_0Fh_HTT
	},
	{0, }
};

/* AMD Family 17h							*/
static struct pci_device_id PCI_AMD_17h_ids[] = {
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ZEN_PLUS_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ZEPPELIN_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_RAVEN_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
/* AMD Families 17h and 19h: IOMMU					*/
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ZEN2_MTS_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_STARSHIP_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_RENOIR_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ZEN_APU_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ZEN2_APU_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_FIREFLIGHT_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ARDEN_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_JUPITER_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_ZEN3_RMB_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_ZEN4_RPL_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_ZEN4_GNA_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_ZEN4_PHX_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	/* Source: HYGON: PCI list					*/
	{
		PCI_VDEVICE(HYGON, DID_AMD_17H_ZEN_PLUS_NB_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	/* Source: SMU > Data Fabric > UMC				*/
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ZEPPELIN_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Zeppelin
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_RAVEN_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Raven
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_MATISSE_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Matisse
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_STARSHIP_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Starship
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_RENOIR_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Renoir
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ARIEL_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Ariel
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_RAVEN2_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Raven2
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_FIREFLIGHT_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Fireflight
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_ARDEN_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Arden
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_17H_JUPITER_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_VanGogh
	},
/* AMD Family 19h							*/
	/* Source: SMU > Data Fabric > UMC				*/
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_VERMEER_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Vermeer
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_CEZANNE_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Cezanne
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_REMBRANDT_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Rembrandt
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_RAPHAEL_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Raphael
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_GENOA_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Genoa
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_19H_PHOENIX_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Phoenix
	},
/* AMD Family 1Ah							*/
	{
		PCI_VDEVICE(AMD, DID_AMD_1AH_ZEN5_TURIN_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_1AH_TURIN_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Turin
	},
	/*			Strix Point				*/
	{
		PCI_VDEVICE(AMD, DID_AMD_1AH_ZEN5_STX_IOMMU),
		.driver_data = (kernel_ulong_t) AMD_Zen_IOMMU
	},
	{
		PCI_VDEVICE(AMD, DID_AMD_1AH_STX_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Strix_Point
	},
	/*			Strix Halo				*/
	{
		PCI_VDEVICE(AMD, DID_AMD_1AH_STXH_DF_UMC),
		.driver_data = (kernel_ulong_t) AMD_DataFabric_Strix_Point
	},
	{0, }
};

#define PCI_AMD_19h_ids PCI_AMD_17h_ids
#define PCI_AMD_1Ah_ids PCI_AMD_17h_ids

	 /*	Left as empty for initialization purpose.	*/
static char *Arch_Misc_Processor[]	=	ZLIST(NULL);

static char *Arch_Core_Yonah[]		=	ZLIST("Core/Yonah");
static char *Arch_Core_Conroe[] 	=	ZLIST("Core2/Conroe/Merom");
static char *Arch_Core_Kentsfield[]	=	ZLIST("Core2/Kentsfield");
static char *Arch_Core_Conroe_616[]	=	ZLIST("Core2/Conroe/Yonah");

enum {
	CN_PENRYN,
	CN_YORKFIELD,
	CN_WOLFDALE
};

static char *Arch_Core_Penryn[] = ZLIST(
			[CN_PENRYN]	=	"Core2/Penryn",
			[CN_YORKFIELD]	=	"Core2/Yorkfield",
			[CN_WOLFDALE]	=	"Core2/Wolfdale"
);

static char *Arch_Core_Dunnington[]	=	ZLIST("Xeon/Dunnington");
static char *Arch_Atom_Bonnell[]	=	ZLIST("Atom/Bonnell");
static char *Arch_Atom_Silvermont[]	=	ZLIST("Atom/Silvermont");
static char *Arch_Atom_Lincroft[]	=	ZLIST("Atom/Lincroft");
static char *Arch_Atom_Clover_Trail[]	=	ZLIST("Atom/Clovertrail");
static char *Arch_Atom_Saltwell[]	=	ZLIST("Atom/Saltwell");
static char *Arch_Silvermont_BYT[]	=	ZLIST("Silvermont/Bay Trail");
static char *Arch_Atom_Avoton[] 	=	ZLIST("Atom/Avoton");

enum {
	CN_AIRMONT,
	CN_BRASWELL,
	CN_CHERRYTRAIL
};

static char *Arch_Atom_Airmont[] = ZLIST(
		[CN_AIRMONT]		=	"Atom/Airmont",
		[CN_BRASWELL]		=	"Airmont/Braswell",
		[CN_CHERRYTRAIL]	=	"Airmont/Cherry Trail"
);

static char *Arch_Atom_Goldmont[]	=	ZLIST("Atom/Goldmont");
static char *Arch_Atom_Sofia[]		=	ZLIST("Atom/Sofia");
static char *Arch_Atom_Merrifield[]	=	ZLIST("Atom/Merrifield");
static char *Arch_Atom_Moorefield[]	=	ZLIST("Atom/Moorefield");
static char *Arch_Nehalem_Bloomfield[]	=	ZLIST("Nehalem/Bloomfield");

enum {
	CN_LYNNFIELD,
	CN_CLARKSFIELD,
	CN_JASPER_FOREST
};

static char *Arch_Nehalem_Lynnfield[] = ZLIST(
		[CN_LYNNFIELD]		=	"Nehalem/Lynnfield",
		[CN_CLARKSFIELD]	=	"Nehalem/Clarksfield",
		[CN_JASPER_FOREST]	=	"Nehalem/Jasper Forest"
);

static char *Arch_Nehalem_MB[]		=	ZLIST("Nehalem/Mobile");
static char *Arch_Nehalem_EX[]		=	ZLIST("Nehalem/eXtreme.EP");
static char *Arch_Westmere[]		=	ZLIST("Westmere");

enum {
	CN_WESTMERE_EP,
	CN_GULFTOWN
};

static char *Arch_Westmere_EP[] = ZLIST(
		[CN_WESTMERE_EP]	=	"Westmere/EP",
		[CN_GULFTOWN]		=	"Westmere/Gulftown"
);

static char *Arch_Westmere_EX[]		=	ZLIST("Westmere/eXtreme");
static char *Arch_SandyBridge[]		=	ZLIST("SandyBridge");

enum {
	CN_SANDYBRIDGE_EP,
	CN_SNB_ROMLEY_EP,
	CN_SNB_EXTREME
};

static char *Arch_SandyBridge_EP[] = ZLIST(
		[CN_SANDYBRIDGE_EP]	=	"SandyBridge/EP",
		[CN_SNB_ROMLEY_EP]	=	"SandyBridge/EP/Romley",
		[CN_SNB_EXTREME]	=	"SandyBridge/eXtreme"
);

static char *Arch_IvyBridge[]		=	ZLIST("IvyBridge");

enum {
	CN_IVYBRIDGE_EP,
	CN_IVB_ROMLEY_EP,
	CN_IVB_EXTREME
};

static char *Arch_IvyBridge_EP[] = ZLIST(
		[CN_IVYBRIDGE_EP]	=	"IvyBridge/EP",
		[CN_IVB_ROMLEY_EP]	=	"IvyBridge/EP/Romley",
		[CN_IVB_EXTREME]	=	"IvyBridge/eXtreme"
);

enum {
	CN_HASWELL_DESKTOP,
	CN_HASWELL_MOBILE_EX,
	CN_HASWELL_CRYSTALWELL,
	CN_HASWELL_CANYON,
	CN_HASWELL_DENLOW,
	CN_HASWELL_EMBEDDED,
	CN_HASWELL_MOBILE
};

static char *Arch_Haswell_DT[] = ZLIST(
		[CN_HASWELL_DESKTOP]	=	"Haswell/Desktop",
		[CN_HASWELL_MOBILE_EX]	=	"Haswell/Mobile/eXtreme",
		[CN_HASWELL_CRYSTALWELL]=	"Haswell/Crystal Well",
		[CN_HASWELL_CANYON]	=	"Haswell/Canyon",
		[CN_HASWELL_DENLOW]	=	"Haswell/Denlow",
		[CN_HASWELL_EMBEDDED]	=	"Haswell/Embedded",
		[CN_HASWELL_MOBILE]	=	"Haswell/Mobile"
);

enum {
	CN_HASWELL_EP,
	CN_HSW_GRANTLEY_EP,
	CN_HSW_EXTREME
};

static char *Arch_Haswell_EP[] = ZLIST(
		[CN_HASWELL_EP] 	=	"Haswell/EP",
		[CN_HSW_GRANTLEY_EP]	=	"Haswell/EP/Grantley",
		[CN_HSW_EXTREME]	=	"Haswell/eXtreme"
);

static char *Arch_Haswell_ULT[] =	ZLIST("Haswell/Ultra Low TDP");
static char *Arch_Haswell_ULX[] =	ZLIST("Haswell/Ultra Low eXtreme");
static char *Arch_Broadwell[]	=	ZLIST("Broadwell/Mobile");
static char *Arch_Broadwell_D[] =	ZLIST("Broadwell/D");
static char *Arch_Broadwell_H[] =	ZLIST("Broadwell/H");

enum {
	CN_BROADWELL_EP,
	CN_BDW_GRANTLEY_EP,
	CN_BDW_EXTREME
};
static char *Arch_Broadwell_EP[] = ZLIST(
		[CN_BROADWELL_EP]	=	"Broadwell/EP",
		[CN_BDW_GRANTLEY_EP]	=	"Broadwell/EP/Grantley",
		[CN_BDW_EXTREME]	=	"Broadwell/eXtreme"
);

static char *Arch_Skylake_UY[]		=	ZLIST("Skylake/UY");
static char *Arch_Skylake_S[]		=	ZLIST("Skylake/S");

enum {
	CN_SKYLAKE_X,
	CN_CASCADELAKE_X,
	CN_COOPERLAKE_X
};

static char *Arch_Skylake_X[] = ZLIST(
		[CN_SKYLAKE_X]		=	"Skylake/X",
		[CN_CASCADELAKE_X]	=	"Cascade Lake/X",
		[CN_COOPERLAKE_X]	=	"Cooper Lake/X"
);

static char *Arch_Xeon_Phi[]		=	ZLIST("Knights Landing");

enum {
	CN_KABYLAKE,
	CN_KABYLAKE_S,
	CN_KABYLAKE_X,
	CN_KABYLAKE_H,
	CN_KABYLAKE_G,
	CN_COFFEELAKE_S,
	CN_COFFEELAKE_H,
	CN_COFFEELAKE_R,
	CN_COFFEELAKE_HR
};

static char *Arch_Kabylake[] = ZLIST(
		[CN_KABYLAKE]		=	"Kaby Lake",
		[CN_KABYLAKE_S] 	=	"Kaby Lake/S",
		[CN_KABYLAKE_X] 	=	"Kaby Lake/X",
		[CN_KABYLAKE_H] 	=	"Kaby Lake/H",
		[CN_KABYLAKE_G] 	=	"Kaby Lake/G",
		[CN_COFFEELAKE_S]	=	"Coffee Lake/S",
		[CN_COFFEELAKE_H]	=	"Coffee Lake/H",
		[CN_COFFEELAKE_R]	=	"Coffee Lake/R",
		[CN_COFFEELAKE_HR]	=	"Coffee Lake/HR"
);

enum {
	CN_KABYLAKE_UY,
	CN_KABYLAKE_R,
	CN_COFFEELAKE_U,
	CN_WHISKEYLAKE_U,
	CN_AMBERLAKE_Y,
	CN_COMETLAKE_H,
	CN_COMETLAKE_U
};

static char *Arch_Kabylake_UY[] = ZLIST(
		[CN_KABYLAKE_UY]	=	"Kaby Lake/UY",
		[CN_KABYLAKE_R] 	=	"Kaby Lake/R",
		[CN_COFFEELAKE_U]	=	"Coffee Lake/U",
		[CN_WHISKEYLAKE_U]	=	"Whiskey Lake/U",
		[CN_AMBERLAKE_Y]	=	"Amber Lake/Y",
		[CN_COMETLAKE_H]	=	"Comet Lake/H",
		[CN_COMETLAKE_U]	=	"Comet Lake/U"
);

static char *Arch_Cannonlake_U[]	=	ZLIST("Cannon Lake/U");
static char *Arch_Cannonlake_H[]	=	ZLIST("Cannon Lake/H");
static char *Arch_Geminilake[]		=	ZLIST("Gemini Lake");
static char *Arch_Icelake[]		=	ZLIST("Ice Lake");
static char *Arch_Icelake_UY[]		=	ZLIST("Ice Lake/UY");
static char *Arch_Icelake_X[]		=	ZLIST("Ice Lake/X");
static char *Arch_Icelake_D[]		=	ZLIST("Ice Lake/D");
static char *Arch_Sunny_Cove[]		=	ZLIST("Sunny Cove");
static char *Arch_Tigerlake[]		=	ZLIST("Tiger Lake");
static char *Arch_Tigerlake_U[] 	=	ZLIST("Tiger Lake/U");
static char *Arch_Cometlake[]		=	ZLIST("Comet Lake");
static char *Arch_Cometlake_UY[]	=	ZLIST("Comet Lake/UY");

static char *Arch_Atom_Denverton[]	=	ZLIST("Atom/Denverton");

static char *Arch_Tremont_Jacobsville[] =	ZLIST("Tremont/Jacobsville");
static char *Arch_Tremont_Lakefield[]	=	ZLIST("Tremont/Lakefield");
static char *Arch_Tremont_Elkhartlake[] =	ZLIST("Tremont/Elkhart Lake");
static char *Arch_Tremont_Jasperlake[]	=	ZLIST("Tremont/Jasper Lake");
static char *Arch_Sapphire_Rapids[]	=	ZLIST("Sapphire Rapids");
static char *Arch_Emerald_Rapids[]	=	ZLIST("Emerald Rapids");
static char *Arch_Granite_Rapids_X[]	=	ZLIST("Granite Rapids/X");
static char *Arch_Granite_Rapids_D[]	=	ZLIST("Granite Rapids/D");
static char *Arch_Sierra_Forest[]	=	ZLIST("Sierra Forest");
static char *Arch_Grand_Ridge[] 	=	ZLIST("Grand Ridge");

static char *Arch_Rocketlake[]		=	ZLIST("Rocket Lake");
static char *Arch_Rocketlake_U[]	=	ZLIST("Rocket Lake/U");
static char *Arch_Alderlake_S[] 	=	ZLIST("Alder Lake");

enum {
	CN_ALDERLAKE_H,
	CN_ARIZONA_BEACH
};

static char *Arch_Alderlake_H[] = ZLIST(
		[CN_ALDERLAKE_H]	=	"Alder Lake/H",
		[CN_ARIZONA_BEACH]	=	"Arizona Beach"
);

enum {
	CN_ALDERLAKE_N,
	CN_TWIN_LAKE,
	CN_AMSTON_LAKE
};

static char *Arch_Alderlake_N[] = ZLIST(
		[CN_ALDERLAKE_N]	=	"Alder Lake/N",
		[CN_TWIN_LAKE]		=	"Twin Lake",
		[CN_AMSTON_LAKE]	=	"Amston Lake"
);

static char *Arch_Meteorlake_M[]	=	ZLIST("Meteor Lake/M");
static char *Arch_Meteorlake_N[]	=	ZLIST("Meteor Lake/N");
static char *Arch_Meteorlake_S[]	=	ZLIST("Meteor Lake/S");
static char *Arch_Raptorlake[]		=	ZLIST("Raptor Lake");
static char *Arch_Raptorlake_P[]	=	ZLIST("Raptor Lake/P");
static char *Arch_Raptorlake_S[]	=	ZLIST("Raptor Lake/S");
static char *Arch_Lunarlake[]		=	ZLIST("Lunar Lake");
static char *Arch_Arrowlake[]		=	ZLIST("Arrow Lake");
static char *Arch_Arrowlake_H[] 	=	ZLIST("Arrow Lake/H");
static char *Arch_Arrowlake_U[] 	=	ZLIST("Arrow Lake/U");
static char *Arch_Pantherlake[] 	=	ZLIST("Panther Lake");
static char *Arch_Clearwater_Forest[]	=	ZLIST("Clearwater Forest");
static char *Arch_Bartlettlake_S[]	=	ZLIST("Bartlett Lake/S");

enum {
	CN_BULLDOZER,
	CN_PILEDRIVER,
	CN_STEAMROLLER,
	CN_EXCAVATOR
};

static char *Arch_AMD_Family_0Fh[]	=	ZLIST("Hammer");
static char *Arch_AMD_Family_10h[]	=	ZLIST("K10");
static char *Arch_AMD_Family_11h[]	=	ZLIST("Turion");
static char *Arch_AMD_Family_12h[]	=	ZLIST("Fusion");
static char *Arch_AMD_Family_14h[] 	=	ZLIST("Bobcat");
static char *Arch_AMD_Family_15h[] = ZLIST(
		[CN_BULLDOZER]		=	"Bulldozer",
		[CN_PILEDRIVER] 	=	"Bulldozer/Piledriver",
		[CN_STEAMROLLER]	=	"Bulldozer/Steamroller",
		[CN_EXCAVATOR]		=	"Bulldozer/Excavator"
);
static char *Arch_AMD_Family_16h[]	=	ZLIST("Jaguar");

enum {
	CN_SUMMIT_RIDGE,
	CN_WHITEHAVEN,
	CN_NAPLES,
	CN_SNOWY_OWL
};
enum {
	CN_RAVEN_RIDGE,
	CN_GREAT_HORNED_OWL
};
enum {
	CN_PINNACLE_RIDGE,
	CN_COLFAX
};
enum {
	CN_PICASSO,
	CN_BANDED_KESTREL,
	CN_RIVER_HAWK
};
enum {
	CN_DALI,
	CN_POLLOCK
};
enum {
	CN_ROME,
	CN_CASTLE_PEAK
};
enum {
	CN_RENOIR,
	CN_GREY_HAWK
};
enum {
	CN_LUCIENNE
};
enum {
	CN_MATISSE
};
enum {
	CN_VANGOGH,
	CN_VANGOGH_HANDHELD
};
enum {
	CN_MENDOCINO
};
enum {
	CN_VERMEER
};
enum {
	CN_CEZANNE,
	CN_BARCELO,
	CN_BARCELO_R
};
enum {
	CN_MILAN
};
enum {
	CN_CHAGALL
};
enum {
	CN_REMBRANDT,
	CN_REMBRANDT_R,
	CN_RMB_HANDHELD
};
enum {
	CN_GENOA,
	CN_GENOA_X
};
enum {
	CN_RAPHAEL,
	CN_DRAGON_RANGE,
	CN_EPYC_RAPHAEL
};
enum {
	CN_PHOENIX
};
enum {
	CN_PHOENIX2
};
enum {
	CN_PHOENIX_R,
	CN_HAWK_POINT
};
enum {
	CN_HAWK_POINT2
};

enum {
	CN_DHYANA,
	CN_DHYANA_V1,
	CN_DHYANA_V2
};

enum {
	CN_BERGAMO,
	CN_SIENA
};

enum {
	CN_STORM_PEAK
};

enum {
	CN_STRIX_POINT,
	CN_STX_HANDHELD,
	CN_GORGON_POINT
};

enum {
	CN_ELDORA,
	CN_FIRE_RANGE
};

enum {
	CN_TURIN
};

enum {
	CN_TURIN_DENSE
};

enum {
	CN_KRACKAN_POINT
};

enum {
	CN_STRIX_HALO
};

enum {
	CN_SHIMADA_PEAK
};

static char *Arch_AMD_Zen[] = ZLIST(
		[CN_SUMMIT_RIDGE]	=	"Zen/Summit Ridge",
		[CN_WHITEHAVEN] 	=	"Zen/Whitehaven",
		[CN_NAPLES]		=	"Zen/EPYC/Naples",
		[CN_SNOWY_OWL]		=	"Zen/EPYC/Snowy Owl"
);
static char *Arch_AMD_Zen_APU[] = ZLIST(
		[CN_RAVEN_RIDGE]	=	"Zen/Raven Ridge",
		[CN_GREAT_HORNED_OWL]	=	"Zen/Great Horned Owl"
);
static char *Arch_AMD_ZenPlus[] = ZLIST(
		[CN_PINNACLE_RIDGE]	=	"Zen+ Pinnacle Ridge",
		[CN_COLFAX]		=	"Zen+ Colfax"
);
static char *Arch_AMD_ZenPlus_APU[] = ZLIST(
		[CN_PICASSO]		=	"Zen+ Picasso",
		[CN_BANDED_KESTREL]	=	"Zen+ Banded Kestrel",
		[CN_RIVER_HAWK] 	=	"Zen+ River Hawk"
);
static char *Arch_AMD_Zen_Dali[] = ZLIST(
		[CN_DALI]		=	"Zen/Dali",
		[CN_POLLOCK]		=	"Zen/Pollock"
);
static char *Arch_AMD_EPYC_Rome_CPK[] = ZLIST(
		[CN_ROME]		=	"Zen2/EPYC/Rome",
		[CN_CASTLE_PEAK]	=	"Zen2/Castle Peak"
);
static char *Arch_AMD_Zen2_Renoir[] = ZLIST(
		[CN_RENOIR]		=	"Zen2/Renoir",
		[CN_GREY_HAWK]		=	"Zen2/Grey Hawk"
);
static char *Arch_AMD_Zen2_LCN[] = ZLIST(
		[CN_LUCIENNE]		=	"Zen2/Lucienne"
);
static char *Arch_AMD_Zen2_MTS[] = ZLIST(
		[CN_MATISSE]		=	"Zen2/Matisse"
);
static char *Arch_AMD_Zen2_Ariel[]	=	ZLIST("Zen2/Ariel");

static char *Arch_AMD_Zen2_Jupiter[] = ZLIST(
		[CN_VANGOGH]		=	"Zen2/Van Gogh/Aerith",
		[CN_VANGOGH_HANDHELD]	=	"Zen2/Van Gogh/Handheld"
);
static char *Arch_AMD_Zen2_Galileo[] = ZLIST(
		[CN_VANGOGH]		=	"Zen2/Van Gogh/Sephiroth"
);
static char *Arch_AMD_Zen2_MDN[] = ZLIST(
		[CN_MENDOCINO]		=	"Zen2/Mendocino"
);
static char *Arch_AMD_Zen3_VMR[] = ZLIST(
		[CN_VERMEER]		=	"Zen3/Vermeer"
);
static char *Arch_AMD_Zen3_CZN[] = ZLIST(
		[CN_CEZANNE]		=	"Zen3/Cezanne",
		[CN_BARCELO]		=	"Zen3/Barcelo",
		[CN_BARCELO_R]		=	"Zen3/Barcelo-R"
);
static char *Arch_AMD_EPYC_Milan[] = ZLIST(
		[CN_MILAN]		=	"EPYC/Milan"
);
static char *Arch_AMD_Zen3_Chagall[] = ZLIST(
		[CN_CHAGALL]		=	"Zen3/Chagall"
);
static char *Arch_AMD_Zen3_Badami[]	=	ZLIST("Zen3/Milan-X");

static char *Arch_AMD_Zen3Plus_RMB[] = ZLIST(
		[CN_REMBRANDT]		=	"Zen3+ Rembrandt",
		[CN_REMBRANDT_R]	=	"Zen3+ Rembrandt-R",
		[CN_RMB_HANDHELD]	=	"Zen3+ Rembrandt/Handheld"
);
static char *Arch_AMD_Zen4_Genoa[] = ZLIST(
		[CN_GENOA]		=	"EPYC/Genoa",
		[CN_GENOA_X]		=	"EPYC/Genoa-X"
);
static char *Arch_AMD_Zen4_RPL[] = ZLIST(
		[CN_RAPHAEL]		=	"Zen4/Raphael",
		[CN_DRAGON_RANGE]	=	"Zen4/Dragon Range",
		[CN_EPYC_RAPHAEL]	=	"Zen4/EPYC/Raphael"
);
static char *Arch_AMD_Zen4_PHX[] = ZLIST(
		[CN_PHOENIX]		=	"Zen4/Phoenix Point"
);
static char *Arch_AMD_Zen4_PHXR[] = ZLIST(
		[CN_PHOENIX_R]		=	"Zen4/Phoenix Point-R",
		[CN_HAWK_POINT] 	=	"Zen4/Hawk Point"
);
static char *Arch_AMD_Zen4_PHX2[] = ZLIST(
		[CN_PHOENIX2]		=	"Zen4/Phoenix2"
);
static char *Arch_AMD_Zen4_HWK2[] = ZLIST(
		[CN_HAWK_POINT2]	=	"Zen4/Hawk Point 2"
);
static char *Arch_AMD_Zen4_Bergamo[] = ZLIST(
		[CN_BERGAMO]		=	"Zen4c/Bergamo",
		[CN_SIENA]		=	"Zen4c/Siena"
);
static char *Arch_AMD_Zen4_STP[] = ZLIST(
		[CN_STORM_PEAK] 	=	"Zen4/Storm Peak"
);
static char *Arch_AMD_Zen5_STX[] = ZLIST(
		[CN_STRIX_POINT]	=	"Zen5/Strix Point",
		[CN_STX_HANDHELD]	=	"Zen5/Strix Point/Handheld",
		[CN_GORGON_POINT]	=	"Zen5/Gorgon Point"
);
static char *Arch_AMD_Zen5_Eldora[] = ZLIST(
		[CN_ELDORA]		=	"Zen5/Granite Ridge",
		[CN_FIRE_RANGE] 	=	"Zen5/Fire Range"
);
static char *Arch_AMD_Zen5_Turin[] = ZLIST(
		[CN_TURIN]		=	"Zen5/Turin"
);
static char *Arch_AMD_Zen5_Turin_Dense[] = ZLIST(
		[CN_TURIN_DENSE]	=	"Zen5/Turin-Dense"
);
static char *Arch_AMD_Zen5_KRK[] = ZLIST(
		[CN_KRACKAN_POINT]	=	"Zen5/Krackan Point"
);
static char *Arch_AMD_Zen5_STXH[] = ZLIST(
		[CN_STRIX_HALO] 	=	"Zen5/Strix Halo"
);
static char *Arch_AMD_Zen5_SHP[] = ZLIST(
		[CN_SHIMADA_PEAK] 	=	"Zen5/Shimada Peak"
);

static char *Arch_AMD_Family_17h[]	=	ZLIST("AMD Family 17h");

static char *Arch_Hygon_Family_18h[] = ZLIST(
		[CN_DHYANA]		=	"Dhyana",
		[CN_DHYANA_V1]		=	"Dhyana V1",
		[CN_DHYANA_V2]		=	"Dhyana V2"
);

static char *Arch_AMD_Family_19h[]	=	ZLIST("AMD Family 19h");

static char *Arch_AMD_Family_1Ah[]	=	ZLIST("AMD Family 1Ah");

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

static PROCESSOR_SPECIFIC Silvermont_BYT_Specific[] = {
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

static PROCESSOR_SPECIFIC Airmont_Specific[] = {
	{
	.Brand = ZLIST( "Intel(R) Celeron(R) CPU J3060",
			"Intel(R) Celeron(R) CPU J3160",
			"Intel(R) Pentium(R) CPU J3710",
			"Intel(R) Celeron(R) CPU N3000",
			"Intel(R) Celeron(R) CPU N3010",
			"Intel(R) Celeron(R) CPU N3050",
			"Intel(R) Celeron(R) CPU N3060",
			"Intel(R) Celeron(R) CPU N3150",
			"Intel(R) Celeron(R) CPU N3160",
			"Intel(R) Pentium(R) CPU N3700",
			"Intel(R) Pentium(R) CPU N3710",
			"Intel(R) Atom(TM) x5-E8000"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_BRASWELL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "Intel(R) Atom(TM) x5-Z8300",
			"Intel(R) Atom(TM) x5-Z8330",
			"Intel(R) Atom(TM) x5-Z8350",
			"Intel(R) Atom(TM) x5-Z8500",
			"Intel(R) Atom(TM) x5-Z8550",
			"Intel(R) Atom(TM) x7-Z8700",
			"Intel(R) Atom(TM) x7-Z8750"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_CHERRYTRAIL,
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
	{
	.Brand = ZLIST("Intel(R) Xeon(R) CPU C55"),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_JASPER_FOREST,
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
	.Brand = ZLIST( "Intel(R) Pentium(R) Gold G56", \
			"Intel(R) Pentium(R) Gold G55", \
			"Intel(R) Pentium(R) Gold G54", \
			"Intel(R) Celeron(R) G49",	\
			"Intel(R) Xeon(R) E-2"		),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_9E		*/
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
	{		/*		06_8E		*/
	.Brand = ZLIST( "Intel(R) Core(TM) i7-766",	\
			"Intel(R) Core(TM) i7-760",	\
			"Intel(R) Core(TM) i7-756",	\
			"Intel(R) Core(TM) i7-750"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_UY,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_8E		*/
	.Brand = ZLIST( "Intel(R) Core(TM) i5-736",	\
			"Intel(R) Core(TM) i5-730",	\
			"Intel(R) Core(TM) i5-728",	\
			"Intel(R) Core(TM) i5-726",	\
			"Intel(R) Core(TM) i5-720"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_UY,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_8E		*/
	.Brand = ZLIST( "Intel(R) Core(TM) i7-7Y7",	\
			"Intel(R) Core(TM) i5-7Y5",	\
			"Intel(R) Core(TM) m3-7Y3"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_UY,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_8E		*/
	.Brand = ZLIST( "Intel(R) Core(TM) i3-7167U",	\
			"Intel(R) Core(TM) i3-7130U",	\
			"Intel(R) Core(TM) i3-7100U",	\
			"Intel(R) Core(TM) i3-7020U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_UY,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_8E		*/
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
	.Brand = ZLIST( "Intel(R) Pentium(R) Processor 44",	\
			"Intel(R) Celeron(R) Processor 3965Y"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_UY,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_8E		*/
	.Brand = ZLIST( "Intel(R) Celeron(R) Processor 3965U",	\
			"Intel(R) Celeron(R) Processor 3865U"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_KABYLAKE_UY,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{		/*		06_8E		*/
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

static PROCESSOR_SPECIFIC Alderlake_H_Specific[] = {
	{	/*		06_9A Stepping 4		*/
	.Brand = ZLIST( "Intel(R) Atom C1130",	\
			"Intel(R) Atom C1110",	\
			"Intel(R) Atom C1100"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ARIZONA_BEACH,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_UNCORE_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};

static PROCESSOR_SPECIFIC Alderlake_N_Specific[] = {
	{	/*		06_BE Stepping 0		*/
	.Brand = ZLIST( "Intel(R) N150",		\
			"Intel(R) N250",		\
			"Intel(R) Core(TM) 3 N350",	\
			"Intel(R) Core(TM) 3 N355"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TWIN_LAKE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
	},
	{	/*		06_BE Stepping 0		*/
	.Brand = ZLIST( "Intel(R) Atom(TM) x7211RE",	\
			"Intel(R) Atom(TM) x7203C",	\
			"Intel(R) Atom(TM) x7835RE",	\
			"Intel(R) Atom(TM) x7433RE",	\
			"Intel(R) Atom(TM) x7213RE",	\
			"Intel(R) Atom(TM) x7405C",	\
			"Intel(R) Atom(TM) x7809C"	),
	.Boost = {0, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_AMSTON_LAKE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b00,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK
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
	.Param.Offset = {105, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 20},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 20},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {68, 0, 27},
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
	.Param.Offset = {68, 0, 27},
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
	.Param.Offset = {68, 0, 27},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
/*	[EPYC/Naples]		8F_01h Stepping 2			*/
	{	/* AMD EPYC Server Processors				*/
	.Brand = ZLIST( "AMD EPYC Embedded 7251",	\
			"AMD EPYC 7251"			),
	.Boost = {+8, 0},
	.Param.Offset = {81, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7261",	\
			"AMD EPYC 7261"			),
	.Boost = {+4, 0},
	.Param.Offset = {85, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7281",	\
			"AMD EPYC 7281" 		),
	.Boost = {+6, 0},
	.Param.Offset = {85, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 735P",	\
			"AMD EPYC Embedded 7351",	\
			"AMD EPYC 7351P",		\
			"AMD EPYC 7351",		\
			"AMD EPYC 7301" 		),
	.Boost = {+5, 0},
	.Param.Offset = {85, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7371",	\
			"AMD EPYC Embedded 7301",	\
			"AMD EPYC 7371" 		),
	.Boost = {+7, 0},
	.Param.Offset = {85, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 740P",	\
			"AMD EPYC Embedded 7401",	\
			"AMD EPYC 7401P",		\
			"AMD EPYC 7401" 		),
	.Boost = {+8, +2},
	.Param.Offset = {85, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7451",	\
			"AMD EPYC 7451"			),
	.Boost = {+6, +3},
	.Param.Offset = {81, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 755P",	\
			"AMD EPYC Embedded 7551",	\
			"AMD EPYC Embedded 7501",	\
			"AMD EPYC 7551P",		\
			"AMD EPYC 7551",		\
			"AMD EPYC 7501" 		),
	.Boost = {+6, +4},
	.Param.Offset = {85, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7601",	\
			"AMD EPYC 7601" 		),
	.Boost = {+5, +5},
	.Param.Offset = {81, 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
/*	[Zen/Snowy Owl] 	8F_01h Stepping 2			*/
	{	/* AMD EPYC Embedded Processors 			*/
	.Brand = ZLIST( "AMD EPYC Embedded 3101",	\
			"AMD EPYC 3101" 		),
	.Boost = {+8, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 3151",	\
			"AMD EPYC 3151" 		),
	.Boost = {+2, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 3201",	\
			"AMD EPYC 3201" 		),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 3251",	\
			"AMD EPYC Embedded 3255",	\
			"AMD EPYC 3251",		\
			"AMD EPYC 3255" 		),
	.Boost = {+6, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 3351",	\
			"AMD EPYC 3351" 		),
	.Boost = {+11, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 3451",	\
			"AMD EPYC 3451" 		),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_SNOWY_OWL,
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
	.Brand = ZLIST( "AMD Athlon PRO 200GE", \
			"AMD Athlon 240GE",	\
			"AMD Athlon 220GE",	\
			"AMD Athlon 200GE"	),
	.Boost = {+0, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon PRO 200U"),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 2300U",	\
			"AMD Ryzen 3 2300U"		),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 2400GE"),
	.Boost = {+6, 0},
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 2500U",	\
			"AMD Ryzen 5 PRO 2500U" \
			"AMD Ryzen 7 PRO 2700U" ),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	/*		Ryzen Embedded V1000 Processor Family		*/
	{
	.Brand = ZLIST("AMD Ryzen Embedded V1202B"),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREAT_HORNED_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V1500B"),
	.Boost = {0, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREAT_HORNED_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen Embedded V1404I",	\
			"AMD Ryzen Embedded V1605B"	),
	.Boost = {+16, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREAT_HORNED_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V1756B"),
	.Boost = {+4, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREAT_HORNED_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V1780B"),
	.Boost = {+3, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREAT_HORNED_OWL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V1807B"),
	.Boost = {+5, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREAT_HORNED_OWL,
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
	.Param.Offset = {95, 0, 10},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 10},
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
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 2700X"),
	.Boost = {+5, +2},
	.Param.Offset = {85, 0, 10},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 2600X"),
	.Boost = {+5, +2},
	.Param.Offset = {95, 0, 10},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {68, 0, 27},
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
	.Param.Offset = {68, 0, 27},
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
	.Param.Offset = {68, 0, 27},
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
	.Brand = ZLIST("AMD Athlon 300U"),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon 3000G"),
	.Boost = {+0, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Athlon Silver 3050GE",	\
			"AMD Athlon Silver PRO 3125GE", \
			"AMD Athlon PRO 300GE"		),
	.Boost = {+0, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon PRO 300U"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Gold PRO 3150GE"),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Gold 3150GE"),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 3350GE"),
	.Boost = {+6, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Athlon Gold PRO 3150G",	\
			"AMD Ryzen 5 PRO 3350G"),
	.Boost = {+4, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Gold 3150G"),
	.Boost = {+4, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 3400GE"),
	.Boost = {+7, 0},
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 3350U",	\
			"AMD Ryzen 3 3300U",	\
			"AMD Ryzen 5 3450U"	),
	.Boost = {+14, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 3300U"),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 3580U",	\
			"AMD Ryzen 5 3550H",	\
			"AMD Ryzen 5 3500U",	\
			"AMD Ryzen 5 3500C"	),
	.Boost = {+16, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 3500U"),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
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
			"AMD Ryzen 7 3700U",	\
			"AMD Ryzen 7 3700C"	),
	.Boost = {+17, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 3700U"),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 3250C"),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 3250U"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 3200U"),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_PICASSO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	/*		Ryzen Embedded R1000 Processor Family		*/
	{
	.Brand = ZLIST("AMD Ryzen Embedded R1102G"),
	.Boost = {+14, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_BANDED_KESTREL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded R1305G"),
	.Boost = {+13, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_BANDED_KESTREL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen Embedded R1505G",	\
			"AMD Ryzen Embedded R1606G"	),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_BANDED_KESTREL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded R1600"),
	.Boost = {+5, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_BANDED_KESTREL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	/*		Ryzen Embedded R2000 Processor Family		*/
	{
	.Brand = ZLIST("AMD Ryzen Embedded R2312"),
	.Boost = {+8, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RIVER_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded R2314"),
	.Boost = {+14, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RIVER_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded R2514"),
	.Boost = {+16, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RIVER_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded R2544"),
	.Boost = {+4, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RIVER_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen_Dali_Specific[] = {
	{
	.Brand = ZLIST( "AMD Athlon PRO 3145B", \
			"AMD Athlon PRO 3045B"	),
	.Boost = {+9, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_DALI,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Athlon Gold 3150C",	\
			"AMD Athlon Silver 3050C",	\
			"AMD Athlon Gold 3150U",	\
			"AMD Athlon Silver 3050U",	\
			"AMD Ryzen 3"	/*3250U*/	),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_DALI,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD 3020e",		\
			"AMD Athlon Silver 3050e"),
	.Boost = {+14, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_DALI,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD 3015Ce",	\
			"AMD 3015e"	),
	.Boost = {+11, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_POLLOCK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_EPYC_Rome_CPK_Specific[] = {
/*	[EPYC/Rome]		8F_31h Stepping 0			*/
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7232P",	\
			"AMD EPYC Embedded 7252",	\
			"AMD EPYC 7232P",		\
			"AMD EPYC 7252" 		),
	.Boost = {+1, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7262",	\
			"AMD EPYC Embedded 7F32",	\
			"AMD EPYC 7262",		\
			"AMD EPYC 7F32" 		),
	.Boost = {+2, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7272",	\
			"AMD EPYC 7272" 		),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7282",	\
			"AMD EPYC Embedded 7F52",	\
			"AMD EPYC 7282",		\
			"AMD EPYC 7F52" 		),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7302P",	\
			"AMD EPYC Embedded 7302",	\
			"AMD EPYC 7302P",		\
			"AMD EPYC 7302" 		),
	.Boost = {+3, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7402P",	\
			"AMD EPYC Embedded 7402",	\
			"AMD EPYC 7402P",		\
			"AMD EPYC 7402" 		),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7352",	\
			"AMD EPYC 7352" 		),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7502P",	\
			"AMD EPYC Embedded 7502",	\
			"AMD EPYC Embedded 7532",	\
			"AMD EPYC 7502P",		\
			"AMD EPYC 7502",		\
			"AMD EPYC 7532" 		),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7542",	\
			"AMD EPYC Embedded 7F72",	\
			"AMD EPYC 7542",		\
			"AMD EPYC 7F72" 		),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7552",	\
			"AMD EPYC 7552" 		),
	.Boost = {+11, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7452",	\
			"AMD EPYC Embedded 7642",	\
			"AMD EPYC 7452",		\
			"AMD EPYC 7642" 		),
	.Boost = {+10, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7662",	\
			"AMD EPYC 7662" 		),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7702P",	\
			"AMD EPYC Embedded 7702",	\
			"AMD EPYC 7702P",		\
			"AMD EPYC 7702" 		),
	.Boost = {+14, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7742",	\
			"AMD EPYC 7742" 		),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7H12",	\
			"AMD EPYC 7H12" 		),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
/*	[Zen2/Castle Peak]	8F_31h Stepping 0			*/
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 3990X"),
	.Boost = {+14, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 3970X"),
	.Boost = {+8, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper 3960X"),
	.Boost = {+7, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3995WX"),
	.Boost = {+15, 0},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3975WX"),
	.Boost = {+7, 0},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3955WX"),
	.Boost = {+4, 0},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 3945WX"),
	.Boost = {+3, 0},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_Renoir_Specific[] = {
/*	[Zen2/Renoir]		8F_60h Stepping 1			*/
	{
	.Brand = ZLIST( "AMD Ryzen 3 4300U",	\
			"AMD Ryzen 5 4600H"	),
	.Boost = {+10, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Athlon Gold PRO 4150GE"),
	.Boost = {+5, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 4350GE",\
			"AMD Ryzen 3 PRO 4355GE" ),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 4350G",\
			"AMD Ryzen 3 PRO 4355G" ),
	.Boost = {+2, 0},
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 4500"),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 4655GE", \
			"AMD Ryzen 5 PRO 4650GE" ),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 4655G", \
			"AMD Ryzen 5 PRO 4650G"	),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {105, 0, 0},
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
	.Param.Offset = {105, 0, 0},
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
	.Param.Offset = {105, 0, 0},
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
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 4450U"),
	.Boost = {+12, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 4750GE"),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {95, 0, 0},
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
	.Param.Offset = {105, 0, 0},
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
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_RENOIR,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	/*		Ryzen Embedded V2000 Processor Family		*/
	{
	.Brand = ZLIST("AMD Ryzen Embedded V2516"),
	.Boost = {+19, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREY_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V2546"),
	.Boost = {+10, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREY_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V2718"),
	.Boost = {+25, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREY_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V2748"),
	.Boost = {+14, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREY_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V2A46"),
	.Boost = {+2, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_GREY_HAWK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_LCN_Specific[] = {
	{
	.Brand = ZLIST("AMD Ryzen 3 5300U"),
	.Boost = {+12, +1},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_LUCIENNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 5500U"),
	.Boost = {+19, +1},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_LUCIENNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 5700U"),
	.Boost = {+25, +1},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_LUCIENNE,
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
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 3600"),
	.Boost = {+6, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 3600XT"),
	.Boost = {+6, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 3600X",	\
			"AMD Ryzen 5 3600"	),
	.Boost = {+5, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 3700"),
	.Boost = {+8, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 3800XT"),
	.Boost = {+7, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 3300X",			\
			"AMD Ryzen 5 3500X",	/* zh-cn */	\
			"AMD Ryzen 5 3500",			\
			"AMD Ryzen 7 3800X"			),
	.Boost = {+5, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 PRO 3900"),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 3900XT"),
	.Boost = {+8, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 3700X",	\
			"AMD Ryzen 9 3900X"	),
	.Boost = {+7, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 3900"),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 3950X"),
	.Boost = {+11, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_Jupiter_Specific[] = {
/*	[Zen2/Van Gogh] 7 nm Valve Jupiter.	6 nm Sephiroth		*/
	{
	.Brand = ZLIST( "AMD Custom APU 0405",	\
			"AMD Custom APU 0932"	),
	.Boost = {+7, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_VANGOGH,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Z2 A"),
	.Boost = {+10, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_VANGOGH_HANDHELD,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen2_MDN_Specific[] = {
/*	[Zen2/Mendocino]						*/
	{
	.Brand = ZLIST( "AMD Ryzen 5 7520C",	\
			"AMD Ryzen 5 7520U"	),
	.Boost = {+15, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MENDOCINO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 7320C",	\
			"AMD Ryzen 3 7320U"	),
	.Boost = {+17, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MENDOCINO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Athlon Gold 7220C", \
			"AMD Athlon Gold 7220U" ),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MENDOCINO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Athlon Silver 7120C", \
			"AMD Athlon Silver 7120U" ),
	.Boost = {+11, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MENDOCINO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen3_VMR_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 5 5600X3D",	\
			"AMD Ryzen 7 5700X3D",	\
			"AMD Ryzen 7 5800X3D"	),
	.Boost = {+11, +1},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 5600XT",	\
			"AMD Ryzen 5 5600X",	\
			"AMD Ryzen 5 5600T"	),
	.Boost = {+9, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 5800X",	\
			"AMD Ryzen 5 5600",	\
			"AMD Ryzen 5 5500X3D"	),
	.Boost = {+9, +1},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 5800"),
	.Boost = {+12, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 5700X"),
	.Boost = {+12, +1},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 5900XT",	\
			"AMD Ryzen 9 5900X"	),
	.Boost = {+11, +1},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 5950X"),
	.Boost = {+15, +1},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 5900"),
	.Boost = {+17, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 PRO 5945"),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 5845"),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 5645"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Embedded 5600E",	\
			"AMD Ryzen Embedded 5800E"	),
	.Boost = {+3, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Embedded 5900E",	\
			"AMD Ryzen Embedded 5950E"	),
	.Boost = {+4, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_VERMEER,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen3_CZN_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 5350GE",	\
			"AMD Ryzen 3 PRO 5355GE"	),
	.Boost = {+6, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 PRO 5350G",	\
			"AMD Ryzen 3 PRO 5355G" 	),
	.Boost = {+2, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 5300GE"),
	.Boost = {+6, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 5300G"),
	.Boost = {+2, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 5650GE",	\
			"AMD Ryzen 5 PRO 5655GE"	),
	.Boost = {+10, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 5650G",	\
			"AMD Ryzen 5 PRO 5655G" 	),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 5600GE",	\
			"AMD Ryzen 5 5600GT"	),
	.Boost = {+10, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 5600G"),
	.Boost = {+5, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 5500"),
	.Boost = {+6, +1},
	.Param.Offset = {90, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 5650U"),
	.Boost = {+19, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 PRO 5750GE"),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 5750G",	\
			"AMD Ryzen 7 PRO 5755G" 	),
	.Boost = {+8, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 5700GE"),
	.Boost = {+14, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 5700G"	\
			"AMD Ryzen 7 5700",	\
			"AMD Ryzen 5 5500GT"	),
	.Boost = {+8, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 5400U",	\
			"AMD Ryzen 3 PRO 5450U" ),
	.Boost = {+14, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 5125C"),
	.Boost = {+0, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BARCELO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 5425C",	\
			"AMD Ryzen 3 5425U",	\
			"AMD Ryzen 3 PRO 5475U" ),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BARCELO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 5600HS"),
	.Boost = {+12, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 5500H"	\
			"AMD Ryzen 5 5600H"	),
	.Boost = {+9, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 5600U"),
	.Boost = {+19, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 5560U"),
	.Boost = {+17, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 5625C",	\
			"AMD Ryzen 5 5625U",	\
			"AMD Ryzen 5 PRO 5675U" ),
	.Boost = {+20, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BARCELO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 5800U",	\
			"AMD Ryzen 7 PRO 5850U" ),
	.Boost = {+25, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 5800HS",	\
			"AMD Ryzen 9 5900HS"	),
	.Boost = {+16, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 5800H"),
	.Boost = {+12, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 5825C",	\
			"AMD Ryzen 7 5825U",	\
			"AMD Ryzen 7 PRO 5875U" ),
	.Boost = {+25, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BARCELO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 5980HS"),
	.Boost = {+18, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 5900HX"),
	.Boost = {+13, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 5980HX"),
	.Boost = {+15, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_CEZANNE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 7730U",	\
			"AMD Ryzen 7 PRO 7730U",\
			"AMD Ryzen 5 7530U",	\
			"AMD Ryzen 5 PRO 7530U" ),
	.Boost = {+25, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BARCELO_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 7430U",	\
			"AMD Ryzen 3 7330U",	\
			"AMD Ryzen 3 PRO 7330U" ),
	.Boost = {+20, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BARCELO_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_EPYC_Milan_Specific[] = {
	{
	.Brand = ZLIST( "AMD EPYC 7763",	\
			"AMD EPYC 75F3",	\
			"AMD EPYC 7513" 	),
	.Boost = {+11, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7713P",	\
			"AMD EPYC Embedded 7713",	\
			"AMD EPYC 7713P",		\
			"AMD EPYC 7713" 		),
	.Boost = {+17, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 7663P",	\
			"AMD EPYC 7663" 	),
	.Boost = {+15, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7643",	\
			"AMD EPYC 7773X",		\
			"AMD EPYC 7643P",		\
			"AMD EPYC 7643" 		),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7543P",	\
			"AMD EPYC Embedded 7543",	\
			"AMD EPYC 7543P",		\
			"AMD EPYC 7543",		\
			"AMD EPYC 7473X"		),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 7573X",	\
			"AMD EPYC 7373X",	\
			"AMD EPYC 74F3" 	),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7313P",	\
			"AMD EPYC Embedded 7313",	\
			"AMD EPYC 7453",		\
			"AMD EPYC 7343",		\
			"AMD EPYC 7313P",		\
			"AMD EPYC 7313" 		),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7443P",	\
			"AMD EPYC Embedded 7443",	\
			"AMD EPYC 7443P",		\
			"AMD EPYC 7443" 		),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 7413",	\
			"AMD EPYC 7413",		\
			"AMD EPYC 7303P",		\
			"AMD EPYC 7303" 		),
	.Boost = {+10, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 73F3"),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 72F3"),
	.Boost = {+4, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 7203P",	\
			"AMD EPYC 7203" 	),
	.Boost = {+6, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_MILAN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen3Plus_RMB_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 6650HS",	\
			"AMD Ryzen 5 PRO 6650H",	\
			"AMD Ryzen 5 6600HS​",		\
			"AMD Ryzen 5 6600H"		),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 6850HS",	\
			"AMD Ryzen 7 PRO 6850H",	\
			"AMD Ryzen 7 6800HS",		\
			"AMD Ryzen 7 6800H"		),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 6850U",	\
			"AMD Ryzen 7 6800U​",		\
			"AMD Ryzen 7 PRO 6860Z"		),
	.Boost = {+20, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 PRO 6950HS",	/* zh-cn */	\
			"AMD Ryzen 9 PRO 6950H",	/* zh-cn */	\
			"AMD Ryzen 9 6900HS​",				\
			"AMD Ryzen 5 PRO 6650U",			\
			"AMD Ryzen 5 6600U"				),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 6900HX"),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 6980HS"),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 6980HX"),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 3 7335U",			\
			"AMD Ryzen 5 7535HS",			\
			"AMD Ryzen 5 7535H",	/* zh-cn */	\
			"AMD Ryzen 3 PRO 7335U",		\
			"AMD Ryzen 5 150"			),
	.Boost = {+13, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 7235HS",	\
			"AMD Ryzen 5 7235H"	),	/* zh-cn */
	.Boost = {+10, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 7535U",	\
			"AMD Ryzen 5 PRO 7535U",\
			"AMD Ryzen 5 130"	),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 7435HS",	\
			"AMD Ryzen 7 7435H"	),	/* zh-cn */
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 7735HS",				\
			"AMD Ryzen 7 7735H",		/* zh-cn */	\
			"AMD Ryzen 7 170"				),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 7735U",	\
			"AMD Ryzen 7 160"	),
	.Boost = {+21, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 7736U",	\
			"AMD Ryzen 7 PRO 7735U" ),
	.Boost = {+20, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_REMBRANDT_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	/*		Ryzen Embedded V3000 Processor Family		*/
	{
	.Brand = ZLIST("AMD Ryzen Embedded V3C48"),
	.Boost = {+5, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V3C44"),
	.Boost = {+3, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V3C18I"),
	.Boost = {+19, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V3C16"),
	.Boost = {+18, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Embedded V3C14"),
	.Boost = {+15, 0},
	.Param.Offset = {105, 0, 0},
	.CodeNameIdx = CN_REMBRANDT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Z2 Go"),
	.Boost = {+13, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RMB_HANDHELD,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen Z2"),
	.Boost = {+18, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RMB_HANDHELD,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen3_Chagall_Specific[] = {
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 5995WX"),
	.Boost = {+18, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CHAGALL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 5975WX"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CHAGALL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 5965WX"),
	.Boost = {+7, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CHAGALL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 5955WX"),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CHAGALL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 5945WX"),
	.Boost = {+4, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_CHAGALL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_Genoa_Specific[] = {
	{
	.Brand = ZLIST("AMD Eng Sample"),
	.Boost = {+10, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9654P",	\
			"AMD EPYC Embedded 9534",	\
			"AMD EPYC Embedded 9654",	\
			"AMD EPYC Embedded 9254",	\
			"AMD EPYC 9654P",		\
			"AMD EPYC 9654",		\
			"AMD EPYC 9534",		\
			"AMD EPYC 9254" 		),
	.Boost = {+13, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9634"),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9554P",	\
			"AMD EPYC Embedded 9554",	\
			"AMD EPYC Embedded 9124",	\
			"AMD EPYC 9554P",		\
			"AMD EPYC 9554",		\
			"AMD EPYC 9124" 		),
	.Boost = {+7, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 9474F",	\
			"AMD EPYC 9374F"	),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9454P",	\
			"AMD EPYC Embedded 9454",	\
			"AMD EPYC 9454P",		\
			"AMD EPYC 9454" 		),
	.Boost = {+11, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9354P",	\
			"AMD EPYC Embedded 9354",	\
			"AMD EPYC 9354P",		\
			"AMD EPYC 9354" 		),
	.Boost = {+6, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 9334",	\
			"AMD EPYC 9224" 	),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 9274F",	\
			"AMD EPYC 9174F"	),
	.Boost = {+3, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	/*	EPYC Genoa-X Family with 3D V-Cache Technology		*/
	{
	.Brand = ZLIST("AMD EPYC 9684X"),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA_X,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9384X"),
	.Boost = {+8, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA_X,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9184X"),
	.Boost = {+7, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_GENOA_X,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_RPL_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 9 PRO 7945", 	\
			"AMD Ryzen Embedded 7945"	),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 7745", 	\
			"AMD Ryzen Embedded 7745"	),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 7645", 	\
			"AMD Ryzen Embedded 7645"	),
	.Boost = {+13, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 7950X3D"),
	.Boost = {+15, 0},
	.Param.Offset = {89, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 7900X3D"),
	.Boost = {+12, 0},
	.Param.Offset = {89, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 7800X3D"),
	.Boost = {+10, 0},
	.Param.Offset = {89, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 7950X"),
	.Boost = {+12, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 7900X",		\
			"AMD Ryzen 7 7700X",		\
			"AMD Ryzen Embedded 7700X",	\
			"AMD Ryzen 5 7400F"		),
	.Boost = {+9, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 7600X3D"),
	.Boost = {+6, 0},
	.Param.Offset = {89, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 7600X",		\
			"AMD Ryzen Embedded 7600X"	),
	.Boost = {+6, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 7900"),
	.Boost = {+16, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 7700"),
	.Boost = {+14, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 7600",	\
			"AMD Ryzen 5 7500F"	),
	.Boost = {+12, +1},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 7945HX3D"),
	.Boost = {+31, 0},
	.Param.Offset = {89, 0, 0},
	.CodeNameIdx = CN_DRAGON_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 7945HX",	\
			"AMD Ryzen 9 8945HX"	),
	.Boost = {+29, +1},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_DRAGON_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 7940HX",	\
			"AMD Ryzen 9 8940HX"	),
	.Boost = {+28, +1},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_DRAGON_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 7845HX",	\
			"AMD Ryzen 9 7840HX",	\
			"AMD Ryzen 9 8840HX"	),
	.Boost = {+22, +1},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_DRAGON_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 7745HX",	\
			"AMD Ryzen 7 8745HX"	),
	.Boost = {+15, +1},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_DRAGON_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 7645HX"),
	.Boost = {+10, +1},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_DRAGON_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD EPYC 4464P"),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_EPYC_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 4584PX",	\
			"AMD EPYC 4344P"	),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_EPYC_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 4244P",	\
			"AMD EPYC 4124P"	),
	.Boost = {+13, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_EPYC_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 4564P",	\
			"AMD EPYC 4484PX"	),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_EPYC_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 4364P"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_EPYC_RAPHAEL,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_PHX_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 9 PRO 7940HS",	\
			"AMD Ryzen 9 7940HS",		\
			"AMD Ryzen 9 7940H" /* zh-cn */ ),
	.Boost = {+12, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 7840HS",	\
			"AMD Ryzen 7 7840HS",		\
			"AMD Ryzen 7 7840H" /* zh-cn */ ),
	.Boost = {+13, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 7640HS",	\
			"AMD Ryzen 5 7640HS",		\
			"AMD Ryzen 5 7640H" /* zh-cn */ ),
	.Boost = {+7, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 7840U",	\
			"AMD Ryzen 7 7840U",		\
			"AMD Ryzen Z1 Extreme"		),
	.Boost = {+18, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 7540U",	\
			"AMD Ryzen 5 7540U",		\
			"AMD Ryzen Z1"			),
	.Boost = {+17, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 7640U",	\
			"AMD Ryzen 5 7640U"		),
	.Boost = {+14, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_PHXR_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 9 PRO 8945HS",	\
			"AMD Ryzen 9 8945HS",		\
			"AMD Ryzen 9 8945H", /* zh-cn */\
			"AMD Ryzen 9 8940H"		),
	.Boost = {+12, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 8845HS",	\
			"AMD Ryzen 7 8845HS",		\
			"AMD Ryzen 7 8845H", /* zh-cn */\
			"AMD Ryzen Embedded 8845HS",	\
			"AMD Ryzen 7 260"		),
	.Boost = {+13, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 8840HS",	\
			"AMD Ryzen 7 PRO 8840U",	\
			"AMD Ryzen 7 8840HS",		\
			"AMD Ryzen 7 8840H", /* zh-cn */\
			"AMD Ryzen 7 8840U",		\
			"AMD Ryzen Embedded 8840U",	\
			"AMD Ryzen 7 PRO 250",		\
			"AMD Ryzen 7 255",		\
			"AMD Ryzen 7 250"		),
	.Boost = {+18, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 8745HS",/* zh-cn */\
			"AMD Ryzen 7 8745H"  /* zh-cn */),
	.Boost = {+11, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 8645HS",	\
			"AMD Ryzen 5 8645HS",		\
			"AMD Ryzen 5 8645H", /* zh-cn */\
			"AMD Ryzen Embedded 8645HS",	\
			"AMD Ryzen 5 240"		),
	.Boost = {+7, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 8640HS",	\
			"AMD Ryzen 5 PRO 8640U",	\
			"AMD Ryzen 5 8640HS",		\
			"AMD Ryzen 5 8640U",		\
			"AMD Ryzen Embedded 8640U",	\
			"AMD Ryzen 5 PRO 230"		\
			"AMD Ryzen 5 230"		),
	.Boost = {+14, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 8540U",	\
			"AMD Ryzen 5 8540U",		\
			"AMD Ryzen 3 8440U"		),
	.Boost = {+17, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 8700GE"),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 7 PRO 8700G",	\
			"AMD Ryzen 7 8700F",		\
			"AMD Ryzen 7 8700G"		),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 8600GE"),
	.Boost = {+11, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 8600G",	\
			"AMD Ryzen 5 8600G"		),
	.Boost = {+7, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 8400F"),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX_R,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_PHX2_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 7545U",	\
			"AMD Ryzen 5 7545U",		\
			"AMD Ryzen 3 7440U"		),
	.Boost = {+17, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_PHOENIX2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 8500G",	\
			"AMD Ryzen 3 PRO 8300G",	\
			"AMD Ryzen 5 8500G",		\
			"AMD Ryzen 3 8300G"		),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 8500GE"),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 8500GE"),
	.Boost = {+16, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 PRO 8300GE"),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 3 8300GE"),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_PHOENIX2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_HWK2_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen 5 PRO 220",		\
			"AMD Ryzen 5 220"		),
	.Boost = {+17, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD Ryzen 5 PRO 215"),
	.Boost = {+15, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_HAWK_POINT2,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_Bergamo_Specific[] = {
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9754S",	\
			"AMD EPYC Embedded 9754",	\
			"AMD EPYC 9754S",		\
			"AMD EPYC 9754" 		),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BERGAMO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9734",	\
			"AMD EPYC 9734" 		),
	.Boost = {+8, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_BERGAMO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 8534PN"),
	.Boost = {+11, 0},
	.Param.Offset = {75, 0, 0},
	.CodeNameIdx = CN_SIENA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 8534P",	\
			"AMD EPYC 8534P"		),
	.Boost = {+8, 0},
	.Param.Offset = {75, 0, 0},
	.CodeNameIdx = CN_SIENA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC 8434PN",		\
			"AMD EPYC 8324PN",		\
			"AMD EPYC 8224PN",		\
			"AMD EPYC 8124PN",		\
			"AMD EPYC 8024PN"		),
	.Boost = {+10, 0},
	.Param.Offset = {75, 0, 0},
	.CodeNameIdx = CN_SIENA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 8434P",	\
			"AMD EPYC Embedded 8124P",	\
			"AMD EPYC Embedded 8C24P",	\
			"AMD EPYC 8434P",		\
			"AMD EPYC 8124P",		\
			"AMD EPYC 8024P"		),
	.Boost = {+6, 0},
	.Param.Offset = {75, 0, 0},
	.CodeNameIdx = CN_SIENA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 8324P",	\
			"AMD EPYC 8324P"		),
	.Boost = {+4, 0},
	.Param.Offset = {75, 0, 0},
	.CodeNameIdx = CN_SIENA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 8224P",	\
			"AMD EPYC 8224P"		),
	.Boost = {+5, 0},
	.Param.Offset = {75, 0, 0},
	.CodeNameIdx = CN_SIENA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen4_STP_Specific[] = {
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 7995WX"),
	.Boost = {+26, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_STORM_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper PRO 7985WX",	\
			"AMD Ryzen Threadripper 7980X"		),
	.Boost = {+19, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_STORM_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper PRO 7975WX",	\
			"AMD Ryzen Threadripper 7970X"		),
	.Boost = {+13, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_STORM_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper PRO 7965WX",	\
			"AMD Ryzen Threadripper 7960X"		),
	.Boost = {+11, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_STORM_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 7955WX"),
	.Boost = {+8, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_STORM_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 7945WX"),
	.Boost = {+6, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_STORM_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen5_STX_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen AI 9 HX PRO 375",	\
			"AMD Ryzen AI 9 HX PRO 370",	\
			"AMD Ryzen AI 9 HX 375",	\
			"AMD Ryzen AI 9 HX 370" 	),
	.Boost = {+31, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STRIX_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen AI 7 PRO 360",		\
			"AMD Ryzen AI 9 H 365", /* zh-cn */	\
			"AMD Ryzen AI 9 365"			),
	.Boost = {+30, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STRIX_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen AI Z2 Extreme",	\
			"AMD Ryzen Z2 Extreme"		),
	.Boost = {+30, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STX_HANDHELD,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen AI 9 HX PRO 475",	\
			"AMD Ryzen AI 9 HX 475",	\
			"AMD Ryzen AI 9 HX 470" 	),
	.Boost = {+32, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_GORGON_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen AI 9 PRO 465",	\
			"AMD Ryzen AI 9 465"		),
	.Boost = {+30, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_GORGON_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen5_KRK_Specific[] = {
	{
	.Brand = ZLIST( "AMD Ryzen AI 7 PRO 350",		\
			"AMD Ryzen AI 7 H 350", /* zh-cn */	\
			"AMD Ryzen AI 7 350"			),
	.Boost = {+30, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_KRACKAN_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD Ryzen AI 5 PRO 340",		\
			"AMD Ryzen AI 5 H 340", /* zh-cn */	\
			"AMD Ryzen AI 5 340"			),
	.Boost = {+28, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_KRACKAN_POINT,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen5_Eldora_Specific[] = {
	{
	.Brand = ZLIST("AMD Ryzen 5 9600X"),
	.Boost = {+15, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 9950X3D",	\
			"AMD Ryzen 9 9950X",	\
			"AMD Ryzen 5 9600"	),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 9900X3D"),
	.Boost = {+11, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 9900X"),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 9850X3D"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 9800X3D"),
	.Boost = {+5, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 7 9700X"),
	.Boost = {+17, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_ELDORA,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen 9 9955HX3D", \
			"AMD Ryzen 9 9955HX"	),
	.Boost = {+29, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_FIRE_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen 9 9850HX"),
	.Boost = {+22, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_FIRE_RANGE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen5_Turin_Specific[] = {
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9965",\
			"AMD EPYC 9965" 	),	/* Model: 17	*/
	.Boost = {+15, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN_DENSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9845",\
			"AMD EPYC 9845" 	),
	.Boost = {+16, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN_DENSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9115"),
	.Boost = {+15, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9825"),
	.Boost = {+15, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN_DENSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9755",	\
			"AMD EPYC Embedded 9335",	\
			"AMD EPYC 9755", /* Model: 2 */ \
			"AMD EPYC 9335" 		),
	.Boost = {+14, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9745",\
			"AMD EPYC 9745" 	),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN_DENSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9645"),
	.Boost = {+14, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN_DENSE,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9455P",\
			"AMD EPYC Embedded 9455",\
			"AMD EPYC 9455P"	\
			"AMD EPYC 9455" 	),
	.Boost = {+13, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9655P",\
			"AMD EPYC Embedded 9655",\
			"AMD EPYC 9655P",	\
			"AMD EPYC 9655",	\
			"AMD EPYC 9535" 	),
	.Boost = {+19, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9575F"),	/* Model: 2	*/
	.Boost = {+17, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9555P",\
			"AMD EPYC Embedded 9555",\
			"AMD EPYC 9565",	\
			"AMD EPYC 9555P",	\
			"AMD EPYC 9555",	\
			"AMD EPYC 9475F"	),
	.Boost = {+12, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9375F"),
	.Boost = {+10, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9355P",\
			"AMD EPYC Embedded 9355",\
			"AMD EPYC 9365",	\
			"AMD EPYC 9355P",	\
			"AMD EPYC 9355" 	),
	.Boost = {+9, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9135",\
			"AMD EPYC 9275F",	\
			"AMD EPYC 9135" 	),
	.Boost = {+7, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9255",\
			"AMD EPYC 9255" 	),
	.Boost = {+11, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD EPYC 9175F"),
	.Boost = {+8, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD EPYC Embedded 9015",\
			"AMD EPYC 9015" 	),
	.Boost = {+5, 0},
	.Param.Offset = {0, 0, 0},
	.CodeNameIdx = CN_TURIN,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen5_STXH_Specific[] = {
	{
	.Brand = ZLIST( "AMD RYZEN AI MAX+ PRO 395",	\
			"AMD RYZEN AI MAX+ 395" 	),
	.Boost = {+21, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STRIX_HALO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD RYZEN AI MAX PRO 390",	\
			"AMD RYZEN AI MAX 390"		),
	.Boost = {+18, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STRIX_HALO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST( "AMD RYZEN AI MAX PRO 385",	\
			"AMD RYZEN AI MAX 385"		),
	.Boost = {+14, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STRIX_HALO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.Brand = ZLIST("AMD RYZEN AI MAX PRO 380"),
	.Boost = {+13, 0},
	.Param.Offset = {100, 0, 0},
	.CodeNameIdx = CN_STRIX_HALO,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 0,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{0}
};
static PROCESSOR_SPECIFIC AMD_Zen5_SHP_Specific[] = {
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 9995WX"),
	.Boost = {+29, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SHIMADA_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper PRO 9985WX",	\
			"AMD Ryzen Threadripper 9980X"		),
	.Boost = {+22, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SHIMADA_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper PRO 9975WX",	\
			"AMD Ryzen Threadripper 9970X"		),
	.Boost = {+14, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SHIMADA_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST( "AMD Ryzen Threadripper PRO 9965WX",	\
			"AMD Ryzen Threadripper 9960X"		),
	.Boost = {+12, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SHIMADA_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 9955WX"),
	.Boost = {+9, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SHIMADA_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{
	.Brand = ZLIST("AMD Ryzen Threadripper PRO 9945WX"),
	.Boost = {+7, 0},
	.Param.Offset = {95, 0, 0},
	.CodeNameIdx = CN_SHIMADA_PEAK,
	.TgtRatioUnlocked = 1,
	.ClkRatioUnlocked = 0b10,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.HSMP_Capable = 1,
	.Latch=LATCH_TGT_RATIO_UNLOCK|LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK\
		|LATCH_HSMP_CAPABLE
	},
	{0}
};

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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)	\
  || ((RHEL_MAJOR == 8)  && (RHEL_MINOR > 3))
static int CoreFreqK_SetBoost(struct cpufreq_policy*, int) ;
#else
static int CoreFreqK_SetBoost(int) ;
#endif
#endif
static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy*, char*);
static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy*, unsigned int) ;
#endif /* CONFIG_CPU_FREQ */

static unsigned int Policy_GetFreq(unsigned int cpu) ;
static void Policy_Core2_SetTarget(void *arg) ;
static void Policy_Nehalem_SetTarget(void *arg) ;
static void Policy_SandyBridge_SetTarget(void *arg) ;
static void Policy_HWP_SetTarget(void *arg) ;
#define Policy_Broadwell_EP_SetTarget	Policy_SandyBridge_SetTarget
static void Policy_Skylake_SetTarget(void *arg) ;
static void Policy_Zen_SetTarget(void *arg) ;
static void Policy_Zen_CPPC_SetTarget(void *arg) ;

#define VOID_Driver {							\
	.IdleState	= NULL ,					\
	.GetFreq	= NULL ,					\
	.SetTarget	= NULL						\
}

#define CORE2_Driver {							\
	.IdleState	= NULL ,					\
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

static IDLE_STATE Airmont_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "AMT-C1",
	.flags		= 0x00 << 24,
	.Latency	= 1,
	.Residency	= 1
	},
	{
	.Name		= "C6N",
	.Desc		= "AMT-C6N",
	.flags		= (0x58 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 80,
	.Residency	= 275
	},
	{
	.Name		= "C6S",
	.Desc		= "AMT-C6S",
	.flags		= (0x52 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 200,
	.Residency	= 560
	},
	{
	.Name		= "C7",
	.Desc		= "AMT-C7",
	.flags		= (0x60 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 1200,
	.Residency	= 4000
	},
	{
	.Name		= "C7S",
	.Desc		= "AMT-C7S",
	.flags		= (0x64 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 10000,
	.Residency	= 20000
	},
	{NULL}
};

#define Airmont_Driver {						\
	.IdleState	= Airmont_IdleState,				\
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

#define BDW_EP_Driver { 						\
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

static IDLE_STATE ICX_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "ICX-C1",
	.flags		= 0x00 << 24,
	.Latency	= 1,
	.Residency	= 1
	},
	{
	.Name		= "C1E",
	.Desc		= "ICX-C1E",
	.flags		= 0x01 << 24,
	.Latency	= 4,
	.Residency	= 4
	},
	{
	.Name		= "C6",
	.Desc		= "ICX-C6",
	.flags		= (0x20 << 24) | CPUIDLE_FLAG_TLB_FLUSHED,
	.Latency	= 170,
	.Residency	= 600
	},
	{NULL}
};

#define ICX_Driver {							\
	.IdleState	= ICX_IdleState,				\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Skylake_SetTarget			\
}

#define Intel_Driver {							\
	.IdleState	= NULL ,					\
	.GetFreq	= Policy_GetFreq,				\
	.SetTarget	= Policy_Skylake_SetTarget			\
}

static IDLE_STATE Zen_IdleState[] = {
	{
	.Name		= "C1",
	.Desc		= "ZEN-C1",
	.flags		= 0x00 << 24,
	.Latency	= 1,
	.Residency	= 1 * 2
	},
	{
	.Name		= "C2",
	.Desc		= "ZEN-C2",
	.flags		= 0x10 << 24,
	.Latency	= 20,
	.Residency	= 20 * 2
	},
	{
	.Name		= "C3",
	.Desc		= "ZEN-C3",
	.flags		= 0x20 << 24,
	.Latency	= 40,
	.Residency	= 40 * 2
	},
	{
	.Name		= "C4",
	.Desc		= "ZEN-C4",
	.flags		= 0x30 << 24,
	.Latency	= 60,
	.Residency	= 60 * 2
	},
	{
	.Name		= "C5",
	.Desc		= "ZEN-C5",
	.flags		= 0x40 << 24,
	.Latency	= 80,
	.Residency	= 80 * 2
	},
	{
	.Name		= "C6",
	.Desc		= "ZEN-C6",
	.flags		= 0x50 << 24,
	.Latency	= 100,
	.Residency	= 100 * 2
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
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_17h
	},
[Hygon_Family_18h] = {							/*  9*/
	.Signature = _Hygon_Family_18h,
	.Query = Query_Hygon_F18h,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_Hygon_Family_18h
	},
[AMD_Family_19h] = {							/* 10*/
	.Signature = _AMD_Family_19h,
	.Query = Query_AMD_F19h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Family_19h,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_19h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_19h
	},
[AMD_Family_1Ah] = {							/* 11*/
	.Signature = _AMD_Family_1Ah,
	.Query = Query_AMD_F1Ah_PerCluster,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Family_1Ah,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_1Ah,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = VOID_Driver,
	.Architecture = Arch_AMD_Family_1Ah
	},

[Core_Yonah] = {							/* 12*/
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
[Core_Conroe] = {							/* 13*/
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
[Core_Kentsfield] = {							/* 14*/
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
[Core_Conroe_616] = {							/* 15*/
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
[Core_Penryn] = {							/* 16*/
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
[Core_Dunnington] = {							/* 17*/
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

[Atom_Bonnell] = {							/* 18*/
	.Signature = _Atom_Bonnell,
	.Query = Query_Atom_Bonnell,
	.Update = PerCore_Atom_Bonnell_Query,
	.Start = Start_Atom_Bonnell,
	.Stop = Stop_Atom_Bonnell,
	.Exit = NULL,
	.Timer = InitTimer_Atom_Bonnell,
	.BaseClock = BaseClock_Atom,
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
	.Architecture = Arch_Atom_Bonnell
	},
[Atom_Silvermont] = {							/* 19*/
	.Signature = _Atom_Silvermont,
	.Query = Query_Atom_Bonnell,
	.Update = PerCore_Atom_Bonnell_Query,
	.Start = Start_Atom_Bonnell,
	.Stop = Stop_Atom_Bonnell,
	.Exit = NULL,
	.Timer = InitTimer_Atom_Bonnell,
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
[Atom_Lincroft] = {							/* 20*/
	.Signature = _Atom_Lincroft,
	.Query = Query_Atom_Bonnell,
	.Update = PerCore_Atom_Bonnell_Query,
	.Start = Start_Atom_Bonnell,
	.Stop = Stop_Atom_Bonnell,
	.Exit = NULL,
	.Timer = InitTimer_Atom_Bonnell,
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
[Atom_Clover_Trail] = { 						/* 21*/
	.Signature = _Atom_Clover_Trail,
	.Query = Query_Atom_Bonnell,
	.Update = PerCore_Atom_Bonnell_Query,
	.Start = Start_Atom_Bonnell,
	.Stop = Stop_Atom_Bonnell,
	.Exit = NULL,
	.Timer = InitTimer_Atom_Bonnell,
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
[Atom_Saltwell] = {							/* 22*/
	.Signature = _Atom_Saltwell,
	.Query = Query_Atom_Bonnell,
	.Update = PerCore_Atom_Bonnell_Query,
	.Start = Start_Atom_Bonnell,
	.Stop = Stop_Atom_Bonnell,
	.Exit = NULL,
	.Timer = InitTimer_Atom_Bonnell,
	.BaseClock = BaseClock_Atom,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_NONE,
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_SoC_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = CORE2_Driver,
	.Architecture = Arch_Atom_Saltwell
	},
[Silvermont_Bay_Trail] = {						/* 23*/
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
	.Specific = Silvermont_BYT_Specific,
	.SystemDriver = SLM_Driver,
	.Architecture = Arch_Silvermont_BYT
	},
[Atom_Avoton] = {							/* 24*/
	.Signature = _Atom_Avoton,
	.Query = Query_Avoton,
	.Update = PerCore_Avoton_Query,
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
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = SLM_Driver,
	.Architecture = Arch_Atom_Avoton
	},
[Atom_Airmont] = {							/* 25*/
	.Signature = _Atom_Airmont,
	.Query = Query_Airmont,
	.Update = PerCore_Airmont_Query,
	.Start = Start_Silvermont,
	.Stop = Stop_Silvermont,
	.Exit = NULL,
	.Timer = InitTimer_Silvermont,
	.BaseClock = BaseClock_Airmont,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SOC,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_SoC_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Airmont_Specific,
	.SystemDriver = Airmont_Driver,
	.Architecture = Arch_Atom_Airmont
	},
[Atom_Goldmont] = {							/* 26*/
	.Signature = _Atom_Goldmont,
	.Query = Query_Goldmont,
	.Update = PerCore_Goldmont_Query,
	.Start = Start_Goldmont,
	.Stop = Stop_Goldmont,
	.Exit = NULL,
	.Timer = InitTimer_Goldmont,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = SNB_Driver,
	.Architecture = Arch_Atom_Goldmont
	},
[Atom_Sofia] = {							/* 27*/
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
[Atom_Merrifield] = {							/* 28*/
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
[Atom_Moorefield] = {							/* 29*/
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

[Nehalem_Bloomfield] = {						/* 30*/
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
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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
[Nehalem_Lynnfield] = { 						/* 31*/
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
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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
[Nehalem_MB] = {							/* 32*/
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
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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
[Nehalem_EX] = {							/* 33*/
	.Signature = _Nehalem_EX,
	.Query = Query_Nehalem_EX,
	.Update = PerCore_Nehalem_EX_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Nehalem,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = NULL, /* Attempt to read/write MSR 0x1ad will cause #UD */
	.thermalFormula = THERMAL_FORMULA_INTEL,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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

[Westmere] = {								/* 34*/
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
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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
[Westmere_EP] = {							/* 35*/
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
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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
[Westmere_EX] = {							/* 36*/
	.Signature = _Westmere_EX,
	.Query = Query_Nehalem_EX, /* Xeon 7500 series-based platform	*/
	.Update = PerCore_Nehalem_EX_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Westmere,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = NULL, /* Attempt to read/write MSR 0x1ad will cause #UD */
	.thermalFormula = THERMAL_FORMULA_INTEL,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#elif (HWM_CHIPSET == IT8720)
	.voltageFormula = VOLTAGE_FORMULA_ITETECH_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_INTEL,
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

[SandyBridge] = {							/* 37*/
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
[SandyBridge_EP] = {							/* 38*/
	.Signature = _SandyBridge_EP,
	.Query = Query_SandyBridge_EP,
	.Update = PerCore_SandyBridge_EP_Query,
	.Start = Start_SandyBridge_EP,
	.Stop = Stop_SandyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_SandyBridge_EP,
	.BaseClock = BaseClock_SandyBridge,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB_E,
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

[IvyBridge]  = {							/* 39*/
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
[IvyBridge_EP] = {							/* 40*/
	.Signature = _IvyBridge_EP,
	.Query = Query_IvyBridge_EP,
	.Update = PerCore_IvyBridge_EP_Query,
	.Start = Start_IvyBridge_EP,
	.Stop = Stop_IvyBridge_EP,
	.Exit = NULL,
	.Timer = InitTimer_IvyBridge_EP,
	.BaseClock = BaseClock_IvyBridge,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = TurboClock_IvyBridge_EP,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB_E,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_SandyBridge_EP_ids,
	.Uncore = {
		.Start = Start_Uncore_IvyBridge_EP,
		.Stop = Stop_Uncore_IvyBridge_EP,
		.ClockMod = NULL
		},
	.Specific = IvyBridge_EP_Specific,
	.SystemDriver = IVB_Driver,
	.Architecture = Arch_IvyBridge_EP
	},

[Haswell_DT] = {							/* 41*/
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
[Haswell_EP] = {							/* 42*/
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
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB_E,
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
[Haswell_ULT] = {							/* 43*/
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
[Haswell_ULX] = {							/* 44*/
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

[Broadwell]  = {							/* 45*/
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
[Broadwell_D] = {							/* 46*/
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
[Broadwell_H] = {							/* 47*/
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
[Broadwell_EP] = {							/* 48*/
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
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB_E,
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

[Skylake_UY] = {							/* 49*/
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
[Skylake_S]  = {							/* 50*/
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
[Skylake_X]  = {							/* 51*/
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

[Xeon_Phi] = {								/* 52*/
	.Signature = _Xeon_Phi,
	.Query = Query_SandyBridge_EP,
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

[Kabylake] = {								/* 53*/
	.Signature = _Kabylake,
	.Query = Query_Kaby_Lake,
	.Update = PerCore_Kaby_Lake_Query,
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
[Kabylake_UY] = {							/* 54*/
	.Signature = _Kabylake_UY,
	.Query = Query_Kaby_Lake,
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

[Cannonlake_U] = {							/* 55*/
	.Signature = _Cannonlake_U,
	.Query = Query_Kaby_Lake,
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
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Cannonlake_U
	},
[Cannonlake_H] = {							/* 56*/
	.Signature = _Cannonlake_H,
	.Query = Query_Kaby_Lake,
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
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Cannonlake_H
	},

[Spreadtrum] = {							/* 57*/
	.Signature = _Spreadtrum,
	.Query = Query_Airmont,
	.Update = PerCore_Airmont_Query,
	.Start = Start_Silvermont,
	.Stop = Stop_Silvermont,
	.Exit = NULL,
	.Timer = InitTimer_Silvermont,
	.BaseClock = BaseClock_Airmont,
	.ClockMod = ClockMod_Core2_PPC,
	.TurboClock = NULL,
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
	.SystemDriver = Airmont_Driver,
	.Architecture = Arch_Atom_Airmont
	},
[Geminilake] = {							/* 58*/
	.Signature = _Geminilake,
	.Query = Query_Goldmont,
	.Update = PerCore_Geminilake_Query,
	.Start = Start_Goldmont,
	.Stop = Stop_Goldmont,
	.Exit = NULL,
	.Timer = InitTimer_Goldmont,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL_ATOM,
	.PCI_ids = PCI_Geminilake_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = SNB_Driver,
	.Architecture = Arch_Geminilake
	},

[Icelake] = {								/* 59*/
	.Signature = _Icelake,
	.Query = Query_Kaby_Lake,
	.Update = PerCore_Icelake_Query,
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
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Icelake
	},
[Icelake_UY] = {							/* 60*/
	.Signature = _Icelake_UY,
	.Query = Query_Kaby_Lake,
	.Update = PerCore_Icelake_Query,
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
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Icelake_UY
	},
[Icelake_X] = { 							/* 61*/
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
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = ICX_Driver,
	.Architecture = Arch_Icelake_X
	},
[Icelake_D] = { 							/* 62*/
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
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = ICX_Driver,
	.Architecture = Arch_Icelake_D
	},

[Sunny_Cove] = {							/* 63*/
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
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Sunny_Cove
	},

[Tigerlake] = { 							/* 64*/
	.Signature = _Tigerlake,
	.Query = Query_Skylake,
	.Update = PerCore_Tigerlake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Tigerlake
	},
[Tigerlake_U] = {							/* 65*/
	.Signature = _Tigerlake_U,
	.Query = Query_Skylake,
	.Update = PerCore_Tigerlake_Query,
	.Start = Start_Skylake,
	.Stop = Stop_Skylake,
	.Exit = NULL,
	.Timer = InitTimer_Skylake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Tigerlake_U
	},

[Cometlake] = { 							/* 66*/
	.Signature = _Cometlake,
	.Query = Query_Kaby_Lake,
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
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Cometlake
	},
[Cometlake_UY] = {							/* 67*/
	.Signature = _Cometlake_UY,
	.Query = Query_Kaby_Lake,
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
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Cometlake_UY
	},

[Atom_Denverton] = {							/* 68*/
	.Signature = _Atom_Denverton,
	.Query = Query_Goldmont,
	.Update = PerCore_Goldmont_Query,
	.Start = Start_Goldmont,
	.Stop = Stop_Goldmont,
	.Exit = NULL,
	.Timer = InitTimer_Goldmont,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_SandyBridge_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SNB,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = NULL,
		.Stop = NULL,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = SNB_Driver,
	.Architecture = Arch_Atom_Denverton
	},
[Tremont_Jacobsville] = {						/* 69*/
	.Signature = _Tremont_Jacobsville,
	.Query = Query_Goldmont,
	.Update = PerCore_Tremont_Query,
	.Start = Start_Goldmont,
	.Stop = Stop_Goldmont,
	.Exit = NULL,
	.Timer = InitTimer_Goldmont,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_SandyBridge_PPC,
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
	.SystemDriver = Intel_Driver,
	.Architecture = Arch_Tremont_Jacobsville
	},
[Tremont_Lakefield] = { 						/* 70*/
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
[Tremont_Elkhartlake] = {						/* 71*/
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
[Tremont_Jasperlake] = {						/* 72*/
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
[Sapphire_Rapids] = {							/* 73*/
	.Signature = _Sapphire_Rapids,
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
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake_X,
		.Stop = Stop_Uncore_Skylake_X,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKX_Driver,
	.Architecture = Arch_Sapphire_Rapids
	},
[Emerald_Rapids] = {							/* 74*/
	.Signature = _Emerald_Rapids,
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
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake_X,
		.Stop = Stop_Uncore_Skylake_X,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKX_Driver,
	.Architecture = Arch_Emerald_Rapids
	},
[Granite_Rapids_X] = {							/* 75*/
	.Signature = _Granite_Rapids_X,
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
	.Architecture = Arch_Granite_Rapids_X
	},
[Granite_Rapids_D] = {							/* 76*/
	.Signature = _Granite_Rapids_D,
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
	.Architecture = Arch_Granite_Rapids_D
	},
[Sierra_Forest] = {							/* 77*/
	.Signature = _Sierra_Forest,
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
	.Architecture = Arch_Sierra_Forest
	},
[Grand_Ridge] = {							/* 78*/
	.Signature = _Grand_Ridge,
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
	.Architecture = Arch_Grand_Ridge
	},

[Rocketlake] = {							/* 79*/
	.Signature = _Rocketlake,
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
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Rocketlake
	},
[Rocketlake_U] = {							/* 80*/
	.Signature = _Rocketlake_U,
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
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Kabylake_ids,
	.Uncore = {
		.Start = Start_Uncore_Skylake,
		.Stop = Stop_Uncore_Skylake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Rocketlake_U
	},

[Alderlake_S] = {							/* 81*/
	.Signature = _Alderlake_S,
	.Query = Query_Skylake,
	.Update = PerCore_Alderlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_ADL_RPL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Alderlake_S
	},
[Alderlake_H] = {							/* 82*/
	.Signature = _Alderlake_H,
	.Query = Query_Skylake,
	.Update = PerCore_Alderlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_ADL_RPL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Alderlake_H_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Alderlake_H
	},
[Alderlake_N] = {							/* 83*/
	.Signature = _Alderlake_N,
	.Query = Query_Skylake,
	.Update = PerCore_Kaby_Lake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_ADL_RPL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Alderlake_N_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Alderlake_N
	},

[Meteorlake_M] = {							/* 84*/
	.Signature = _Meteorlake_M,
	.Query = Query_Skylake,
	.Update = PerCore_Meteorlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_MTL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Meteorlake_M
	},
[Meteorlake_N] = {							/* 85*/
	.Signature = _Meteorlake_N,
	.Query = Query_Skylake,
	.Update = PerCore_Meteorlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_MTL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Meteorlake_N
	},
[Meteorlake_S] = {							/* 86*/
	.Signature = _Meteorlake_S,
	.Query = Query_Skylake,
	.Update = PerCore_Meteorlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_MTL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Meteorlake_S
	},

[Raptorlake] = {							/* 87*/
	.Signature = _Raptorlake,
	.Query = Query_Skylake,
	.Update = PerCore_Raptorlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_ADL_RPL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Raptorlake
	},
[Raptorlake_P] = {							/* 88*/
	.Signature = _Raptorlake_P,
	.Query = Query_Skylake,
	.Update = PerCore_Raptorlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_ADL_RPL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Raptorlake_P
	},
[Raptorlake_S] = {							/* 89*/
	.Signature = _Raptorlake_S,
	.Query = Query_Skylake,
	.Update = PerCore_Raptorlake_Query,
	.Start = Start_Alderlake,
	.Stop = Stop_Alderlake,
	.Exit = NULL,
	.Timer = InitTimer_Alderlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_ADL_RPL_ids,
	.Uncore = {
		.Start = Start_Uncore_Alderlake,
		.Stop = Stop_Uncore_Alderlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Raptorlake_S
	},
[LunarLake] = { 							/* 90*/
	.Signature = _Lunarlake,
	.Query = Query_Skylake,
	.Update = PerCore_Arrowlake_Query,
	.Start = Start_Arrowlake,
	.Stop = Stop_Arrowlake,
	.Exit = NULL,
	.Timer = InitTimer_Arrowlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_LNL_ids,
	.Uncore = {
		.Start = Start_Uncore_Arrowlake,
		.Stop = Stop_Uncore_Arrowlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Lunarlake
	},
[ArrowLake] = { 							/* 91*/
	.Signature = _Arrowlake,
	.Query = Query_Skylake,
	.Update = PerCore_Arrowlake_Query,
	.Start = Start_Arrowlake,
	.Stop = Stop_Arrowlake,
	.Exit = NULL,
	.Timer = InitTimer_Arrowlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_MTL_ids,
	.Uncore = {
		.Start = Start_Uncore_Arrowlake,
		.Stop = Stop_Uncore_Arrowlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Arrowlake
	},
[ArrowLake_H] = {							/* 92*/
	.Signature = _Arrowlake_H,
	.Query = Query_Skylake,
	.Update = PerCore_Arrowlake_Query,
	.Start = Start_Arrowlake,
	.Stop = Stop_Arrowlake,
	.Exit = NULL,
	.Timer = InitTimer_Arrowlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_MTL_ids,
	.Uncore = {
		.Start = Start_Uncore_Arrowlake,
		.Stop = Stop_Uncore_Arrowlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Arrowlake_H
	},
[ArrowLake_U] = {							/* 93*/
	.Signature = _Arrowlake_U,
	.Query = Query_Skylake,
	.Update = PerCore_Arrowlake_Query,
	.Start = Start_Arrowlake,
	.Stop = Stop_Arrowlake,
	.Exit = NULL,
	.Timer = InitTimer_Arrowlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_MTL_ids,
	.Uncore = {
		.Start = Start_Uncore_Arrowlake,
		.Stop = Stop_Uncore_Arrowlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Arrowlake_U
	},
[PantherLake] = {							/* 94*/
	.Signature = _Pantherlake,
	.Query = Query_Skylake,
	.Update = PerCore_Arrowlake_Query,
	.Start = Start_Arrowlake,
	.Stop = Stop_Arrowlake,
	.Exit = NULL,
	.Timer = InitTimer_Arrowlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Arrowlake,
		.Stop = Stop_Uncore_Arrowlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Pantherlake
	},
[Clearwater_Forest] = { 						/* 95*/
	.Signature = _Clearwater_Forest,
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
	.Architecture = Arch_Clearwater_Forest
	},
[BartlettLake_S] = { 							/* 96*/
	.Signature = _Bartlettlake_S,
	.Query = Query_Skylake,
	.Update = PerCore_Arrowlake_Query,
	.Start = Start_Arrowlake,
	.Stop = Stop_Arrowlake,
	.Exit = NULL,
	.Timer = InitTimer_Arrowlake,
	.BaseClock = BaseClock_Skylake,
	.ClockMod = ClockMod_Skylake_HWP,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
	.voltageFormula = VOLTAGE_FORMULA_INTEL_SAV,
	.powerFormula   = POWER_FORMULA_INTEL,
	.PCI_ids = PCI_Void_ids,
	.Uncore = {
		.Start = Start_Uncore_Arrowlake,
		.Stop = Stop_Uncore_Arrowlake,
		.ClockMod = Haswell_Uncore_Ratio
		},
	.Specific = Void_Specific,
	.SystemDriver = SKL_Driver,
	.Architecture = Arch_Bartlettlake_S
	},

[AMD_Zen] = {								/* 97*/
	.Signature = _AMD_Zen,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen
	},
[AMD_Zen_APU] = {							/* 98*/
	.Signature = _AMD_Zen_APU,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen_APU_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen_APU
	},
[AMD_ZenPlus] = {							/* 99*/
	.Signature = _AMD_ZenPlus,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_ZenPlus_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_ZenPlus
	},
[AMD_ZenPlus_APU] = {							/*100*/
	.Signature = _AMD_ZenPlus_APU,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_ZenPlus_APU_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_ZenPlus_APU
	},
[AMD_Zen_Dali] = {							/*101*/
	.Signature = _AMD_Zen_Dali,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_Family_17h,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen_Dali_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen_Dali
	},
[AMD_EPYC_Rome_CPK] = { 						/*102*/
	.Signature = _AMD_EPYC_Rome_CPK,
	.Query = Query_AMD_F17h_PerCluster,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_MP,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_EPYC_Rome_CPK_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_EPYC_Rome_CPK
	},
[AMD_Zen2_Renoir] = {							/*103*/
	.Signature = _AMD_Zen2_Renoir,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_APU,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_Renoir_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_Renoir
	},
[AMD_Zen2_LCN] = {							/*104*/
	.Signature = _AMD_Zen2_LCN,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_APU,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_17h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_LCN_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_LCN
	},
[AMD_Zen2_MTS] = {							/*105*/
	.Signature = _AMD_Zen2_MTS,
	.Query = Query_AMD_F17h_PerCluster,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_SP,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_MTS_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_MTS
	},
[AMD_Zen2_Ariel] = {							/*106*/
	.Signature = _AMD_Zen2_Ariel,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_SP,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_Ariel
	},
[AMD_Zen2_Jupiter] = {							/*107*/
	.Signature = _AMD_Zen2_Jupiter,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_SP,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_Jupiter_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_Jupiter
	},
[AMD_Zen2_Galileo] = {							/*108*/
	.Signature = _AMD_Zen2_Galileo,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_SP,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_Galileo
	},
[AMD_Zen2_MDN] = {							/*109*/
	.Signature = _AMD_Zen2_MDN,
	.Query = Query_AMD_F17h_PerSocket,
	.Update = PerCore_AMD_Family_17h_Query,
	.Start = Start_AMD_Family_17h,
	.Stop = Stop_AMD_Family_17h,
	.Exit = Exit_AMD_F17h,
	.Timer = InitTimer_AMD_F17h_Zen2_SP,
	.BaseClock = BaseClock_AMD_Family_17h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN2,
	.voltageFormula = VOLTAGE_FORMULA_AMD_17h,
	.powerFormula   = POWER_FORMULA_AMD_17h,
	.PCI_ids = PCI_AMD_17h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_17h,
		.Stop = Stop_Uncore_AMD_Family_17h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen2_MDN_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen2_MDN
	},
[AMD_Zen3_VMR] = {							/*110*/
	.Signature = _AMD_Zen3_VMR,
	.Query = Query_AMD_F19h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_SP,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN3,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == AMD_VCO)
	.voltageFormula = VOLTAGE_FORMULA_ZEN3_VCO,
#else
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
#endif
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen3_VMR_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen3_VMR
	},
[AMD_Zen3_CZN] = {							/*111*/
	.Signature = _AMD_Zen3_CZN,
	.Query = Query_AMD_F19h_PerSocket,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_APU,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN3,
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen3_CZN_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen3_CZN
	},
[AMD_EPYC_Milan] = {							/*112*/
	.Signature = _AMD_EPYC_Milan,
	.Query = Query_AMD_F19h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_MP,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN3,
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_EPYC_Milan_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_EPYC_Milan
	},
[AMD_Zen3_Chagall] = {							/*113*/
	.Signature = _AMD_Zen3_Chagall,
	.Query = Query_AMD_F19h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_MP,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN3,
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen3_Chagall_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen3_Chagall
	},
[AMD_Zen3_Badami] = {							/*114*/
	.Signature = _AMD_Zen3_Badami,
	.Query = Query_AMD_F19h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_MP,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN3,
	.voltageFormula = VOLTAGE_FORMULA_AMD_19h,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen3_Badami
	},
[AMD_Zen3Plus_RMB] = {							/*115*/
	.Signature = _AMD_Zen3Plus_RMB,
	.Query = Query_AMD_F19h_PerSocket,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen3Plus_RMB,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_19h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_RMB,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen3Plus_RMB_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen3Plus_RMB
	},
[AMD_Zen4_Genoa] = {							/*116*/
	.Signature = _AMD_Zen4_Genoa,
	.Query = Query_AMD_F19h_11h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen4_Genoa,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN4,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_Genoa_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_Genoa
	},
[AMD_Zen4_RPL] = {							/*117*/
	.Signature = _AMD_Zen4_RPL,
	.Query = Query_AMD_F19h_61h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen4_RPL,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN4,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == AMD_VCO)
	.voltageFormula = VOLTAGE_FORMULA_ZEN4_VCO,
#else
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
#endif
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_RPL_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_RPL
	},
[AMD_Zen4_PHX] = {							/*118*/
	.Signature = _AMD_Zen4_PHX,
	.Query = Query_AMD_F19h_74h_PerSocket,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen4_PHX,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN4,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_PHX_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_PHX
	},
[AMD_Zen4_PHXR] = {							/*119*/
	.Signature = _AMD_Zen4_PHXR,
	.Query = Query_AMD_F19h_PerSocket,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen4_PHX,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_19h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_PHXR_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_PHXR
	},
[AMD_Zen4_PHX2] = {							/*120*/
	.Signature = _AMD_Zen4_PHX2,
	.Query = Query_AMD_F19h_74h_PerSocket,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen4_PHX,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN4,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_PHX2_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_PHX2
	},
[AMD_Zen4_HWK] = {							/*121*/
	.Signature = _AMD_Zen4_HWK,
	.Query = Query_AMD_F19h_PerSocket,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_Zen4_PHX,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_19h,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_HWK2_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_HWK2
	},
[AMD_Zen4_Bergamo] = {							/*122*/
	.Signature = _AMD_Zen4_Bergamo,
	.Query = Query_AMD_F19h_11h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_MP,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN4,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_Bergamo_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_Bergamo
	},
[AMD_Zen4_STP] = {							/*123*/
	.Signature = _AMD_Zen4_STP,
	.Query = Query_AMD_F19h_11h_PerCluster,
	.Update = PerCore_AMD_Family_19h_Query,
	.Start = Start_AMD_Family_19h,
	.Stop = Stop_AMD_Family_19h,
	.Exit = Exit_AMD_F19h,
	.Timer = InitTimer_AMD_F19h_Zen3_MP,
	.BaseClock = BaseClock_AMD_Family_19h,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_ZEN4,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_19h,
	.PCI_ids = PCI_AMD_19h_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_19h,
		.Stop = Stop_Uncore_AMD_Family_19h,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen4_STP_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen4_STP
	},
[AMD_Zen5_STX] = {							/*124*/
	.Signature = _AMD_Zen5_STX,
	.Query = Query_AMD_F1Ah_24h_60h_70h_PerSocket,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Zen5_STX,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_STX_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_STX
	},
[AMD_Zen5_Eldora] = {							/*125*/
	.Signature = _AMD_Zen5_Eldora,
	.Query = Query_AMD_F19h_61h_PerCluster,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Family_1Ah,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
#if defined(HWM_CHIPSET)
#if (HWM_CHIPSET == AMD_VCO)
	.voltageFormula = VOLTAGE_FORMULA_ZEN5_VCO,
#else
	.voltageFormula = VOLTAGE_FORMULA_AMD_1Ah,
#endif
#else
	.voltageFormula = VOLTAGE_FORMULA_AMD_1Ah,
#endif
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_Eldora_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_Eldora
	},
[AMD_Zen5_Turin] = {							/*126*/
	.Signature = _AMD_Zen5_Turin,
	.Query = Query_AMD_F1Ah_PerCluster,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Family_1Ah,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_1Ah,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_Turin_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_Turin
	},
[AMD_Zen5_Turin_Dense] = {						/*127*/
	.Signature = _AMD_Zen5_Turin_Dense,
	.Query = Query_AMD_F1Ah_PerCluster,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Family_1Ah,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_1Ah,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_Turin_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_Turin_Dense
	},
[AMD_Zen5_KRK] = {							/*128*/
	.Signature = _AMD_Zen5_KRK,
	.Query = Query_AMD_F1Ah_24h_60h_70h_PerSocket,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Zen5_STX,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_KRK_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_KRK
	},
[AMD_Zen5_STXH] = {							/*129*/
	.Signature = _AMD_Zen5_STXH,
	.Query = Query_AMD_F1Ah_24h_60h_70h_PerSocket,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Zen5_STX,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_ZEN4,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_STXH_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_STXH
	},
[AMD_Zen5_SHP] = {							/*130*/
	.Signature = _AMD_Zen5_SHP,
	.Query = Query_AMD_F1Ah_PerCluster,
	.Update = PerCore_AMD_Family_1Ah_Query,
	.Start = Start_AMD_Family_1Ah,
	.Stop = Stop_AMD_Family_1Ah,
	.Exit = Exit_AMD_F1Ah,
	.Timer = InitTimer_AMD_Family_1Ah,
	.BaseClock = BaseClock_AMD_Family_1Ah,
	.ClockMod = ClockMod_AMD_Zen,
	.TurboClock = TurboClock_AMD_Zen,
	.thermalFormula = THERMAL_FORMULA_AMD_1Ah,
	.voltageFormula = VOLTAGE_FORMULA_AMD_1Ah,
	.powerFormula   = POWER_FORMULA_AMD_1Ah,
	.PCI_ids = PCI_AMD_1Ah_ids,
	.Uncore = {
		.Start = Start_Uncore_AMD_Family_1Ah,
		.Stop = Stop_Uncore_AMD_Family_1Ah,
		.ClockMod = NULL
		},
	.Specific = AMD_Zen5_SHP_Specific,
	.SystemDriver = AMD_Zen_Driver,
	.Architecture = Arch_AMD_Zen5_SHP
	}
};
