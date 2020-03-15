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

typedef struct {
	FEATURES	*Features;
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
	Bit64			TSM __attribute__ ((aligned (8)));
} JOIN;

typedef struct
{
#ifdef CONFIG_AMD_NB
	struct pci_dev		*ZenIF_dev;
#endif
	struct kmem_cache	*Cache;
	JOIN			*Join[];
} KPRIVATE;


/* Sources:
 * Intel® 64 and IA-32 Architectures Software Developer’s Manual; Vol. 2A
 * AMD64 Architecture Programmer’s Manual; Vol. 3
*/

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
	{.func=0x0000000d, .sub=0x00000003},	/* BNDREGS state	*/
	{.func=0x0000000d, .sub=0x00000004},	/* BNDCSR state 	*/
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

#define LATCH_NONE		0b0000
#define LATCH_TGT_RATIO_UNLOCK	0b0001
#define LATCH_CLK_RATIO_UNLOCK	0b0010
#define LATCH_TURBO_UNLOCK	0b0100
#define LATCH_UNCORE_UNLOCK	0b1000

typedef struct {
	char			*CodeName;
} MICRO_ARCH;

typedef struct {
	char			*BrandSubStr;
	unsigned int		Boost[2];
	THERMAL_PARAM		Param;
	unsigned short		CodeNameIdx	:  8-0,
				TgtRatioUnlocked:  9-8,
				ClkRatioUnlocked: 10-9,
				TurboUnlocked	: 11-10,
				UncoreUnlocked	: 12-11,
				Latch		: 16-12; /* Bits 8-9-10-11 */
} PROCESSOR_SPECIFIC;

typedef struct {
	char			*Name,
				*Desc;
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
	void			(*Query)(void);
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
	SYSTEM_DRIVER		*SystemDriver;
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
static void Start_Uncore_SandyBridge_EP(void *arg) ;
static void Stop_Uncore_SandyBridge_EP(void *arg) ;

extern void Query_IvyBridge(void) ;
#define     PerCore_IvyBridge_Query PerCore_SandyBridge_Query
extern void Query_IvyBridge_EP(void) ;
#define     PerCore_IvyBridge_EP_Query PerCore_SandyBridge_EP_Query

extern void Query_Haswell(void) ;

extern void Query_Haswell_EP(void) ;
static void PerCore_Haswell_EP_Query(void *arg) ;
static void Start_Haswell_EP(void *arg) ;
static void Stop_Haswell_EP(void *arg) ;
extern void InitTimer_Haswell_EP(unsigned int cpu) ;
static void Start_Uncore_Haswell_EP(void *arg) ;
static void Stop_Uncore_Haswell_EP(void *arg) ;

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

extern void Query_Broadwell_EP(void) ;

static void PerCore_Skylake_Query(void *arg) ;
static void Start_Skylake(void *arg) ;
static void Stop_Skylake(void *arg) ;
extern void InitTimer_Skylake(unsigned int cpu) ;
static void Start_Uncore_Skylake(void *arg) ;
static void Stop_Uncore_Skylake(void *arg) ;

extern void Query_Skylake_X(void) ;
#define     PerCore_Skylake_X_Query PerCore_Skylake_Query
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
extern void InitTimer_AMD_Family_15h(unsigned int cpu) ;

extern void Query_AMD_Family_17h(void) ;
static void PerCore_AMD_Family_17h_Query(void *arg) ;
static void Start_AMD_Family_17h(void *arg) ;
static void Stop_AMD_Family_17h(void *arg) ;
extern void InitTimer_AMD_Family_17h(unsigned int cpu) ;

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
#define _Atom_Clovertrail \
			{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x5}
#define _Atom_Saltwell	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x6}

/*	[Silvermont]	06_37h						*/
#define _Silvermont_637	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x3, .Model=0x7}

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
/*	[Tremont]	06_86h						*/
#define _Atom_Tremont	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0x6}

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
#define _Skylake_X	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x5}

/*	[Xeon Phi]	06_57h, 06_85h					*/
#define _Xeon_Phi	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x5, .Model=0x7}

/*	[Kaby Lake]	06_8Eh Stepping 9, 06_9Eh,
	[Whiskey Lake]	06_8Eh Stepping 11
	[Comet Lake]	06_8Eh Stepping 12				*/
#define _Kabylake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x9, .Model=0xE}
#define _Kabylake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x8, .Model=0xE}

/*	[Cannon Lake]	06_66h						*/
#define _Cannonlake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x6, .Model=0x6}

/*	[Geminilake]	06_7Ah						*/
#define _Geminilake	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xA}

/*	[Ice Lake]	06_7Eh						*/
#define _Icelake_UY	{.ExtFamily=0x0, .Family=0x6, .ExtModel=0x7, .Model=0xE}

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

/*	[Family 17h]	8F_00h						*/
#define _AMD_Family_17h {.ExtFamily=0x8, .Family=0xF, .ExtModel=0x0, .Model=0x0}

/*	[Family 18h]	9F_00h						*/
#define _AMD_Family_18h {.ExtFamily=0x9, .Family=0xF, .ExtModel=0x0, .Model=0x0}


typedef kernel_ulong_t (*PCI_CALLBACK)(struct pci_dev *);

static PCI_CALLBACK P945(struct pci_dev *dev) ;
static PCI_CALLBACK P955(struct pci_dev *dev) ;
static PCI_CALLBACK P965(struct pci_dev *dev) ;
static PCI_CALLBACK G965(struct pci_dev *dev) ;
static PCI_CALLBACK P35(struct pci_dev *dev) ;
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
static PCI_CALLBACK AMD_0Fh_MCH(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_0Fh_HTT(struct pci_dev *dev) ;
#ifdef CONFIG_AMD_NB
static PCI_CALLBACK AMD_17h_ZenIF(struct pci_dev *dev) ;
#endif
/* TODO:
static PCI_CALLBACK SKL_SA(struct pci_dev *dev) ;
static PCI_CALLBACK AMD_IOMMU(struct pci_dev *dev) ;
*/
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
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL,PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_HA0),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{	/* Desktop: IMC_SystemAgent=0x0100,0x0104		*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_SA),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_SBRIDGE_IMC_0104),
		.driver_data = (kernel_ulong_t) SNB_IMC
	},
	{0, }
};

/* 3rd Generation
	Ivy Bridge ix-3xxx, Xeon E7/E5 v2: IMC_HA=0x0ea0 / IMC_TA=0x0ea8
	TA0=0x0eaa / TA1=0x0eab / TA2=0x0eac / TA3=0x0ead		*/
static struct pci_device_id PCI_IvyBridge_ids[] = {
	{	/* Desktop: IMC_SystemAgent=0x0150			*/
	    PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_SA),
		.driver_data = (kernel_ulong_t) IVB_IMC
	},
	{	/* Mobile i5-3337U: IMC=0x0154				*/
	  PCI_DEVICE(PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_IBRIDGE_IMC_0154),
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
#ifdef CONFIG_AMD_NB
static struct pci_device_id PCI_AMD_17h_ids[] = {
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
	{0, }
};
#else
static struct pci_device_id PCI_AMD_17h_ids[] = {
	{0, }
};
#endif /* CONFIG_AMD_NB */


static MICRO_ARCH Arch_Void[] = {{NULL}};
static MICRO_ARCH Arch_Core_Yonah[] = {{"Core/Yonah"}, {NULL}};
static MICRO_ARCH Arch_Core_Conroe[] = {{"Core2/Conroe/Merom"}, {NULL}};
static MICRO_ARCH Arch_Core_Kentsfield[] = {{"Core2/Kentsfield"}, {NULL}};
static MICRO_ARCH Arch_Core_Conroe_616[] = {{"Core2/Conroe/Yonah"}, {NULL}};

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

static MICRO_ARCH Arch_Core_Dunnington[] = {{"Xeon/Dunnington"}, {NULL}};
static MICRO_ARCH Arch_Atom_Bonnell[] = {{"Atom/Bonnell"}, {NULL}};
static MICRO_ARCH Arch_Atom_Silvermont[] = {{"Atom/Silvermont"}, {NULL}};
static MICRO_ARCH Arch_Atom_Lincroft[] = {{"Atom/Lincroft"}, {NULL}};
static MICRO_ARCH Arch_Atom_Clovertrail[] = {{"Atom/Clovertrail"}, {NULL}};
static MICRO_ARCH Arch_Atom_Saltwell[] = {{"Atom/Saltwell"}, {NULL}};
static MICRO_ARCH Arch_Silvermont_637[] = {{"Silvermont"}, {NULL}};
static MICRO_ARCH Arch_Atom_Avoton[] = {{"Atom/Avoton"}, {NULL}};
static MICRO_ARCH Arch_Atom_Airmont[] = {{"Atom/Airmont"}, {NULL}};
static MICRO_ARCH Arch_Atom_Goldmont[] = {{"Atom/Goldmont"}, {NULL}};
static MICRO_ARCH Arch_Atom_Sofia[] = {{"Atom/Sofia"}, {NULL}};
static MICRO_ARCH Arch_Atom_Merrifield[] = {{"Atom/Merrifield"}, {NULL}};
static MICRO_ARCH Arch_Atom_Moorefield[] = {{"Atom/Moorefield"}, {NULL}};
static MICRO_ARCH Arch_Nehalem_Bloomfield[] = {{"Nehalem/Bloomfield"}, {NULL}};

enum {
	CN_LYNNFIELD,
	CN_CLARKSFIELD
};

static MICRO_ARCH Arch_Nehalem_Lynnfield[] = {
	[CN_LYNNFIELD]		= {"Nehalem/Lynnfield"},
	[CN_CLARKSFIELD]	= {"Nehalem/Clarksfield"},
	{NULL}
};

static MICRO_ARCH Arch_Nehalem_MB[] = {{"Nehalem/Mobile"}, {NULL}};
static MICRO_ARCH Arch_Nehalem_EX[] = {{"Nehalem/eXtreme.EP"}, {NULL}};
static MICRO_ARCH Arch_Westmere[] = {{"Westmere"}, {NULL}};

enum {
	CN_WESTMERE_EP,
	CN_GULFTOWN
};

static MICRO_ARCH Arch_Westmere_EP[] = {
	[CN_WESTMERE_EP]	= {"Westmere/EP"},
	[CN_GULFTOWN]		= {"Westmere/Gulftown"},
	{NULL}
};

static MICRO_ARCH Arch_Westmere_EX[] = {{"Westmere/eXtreme"}, {NULL}};
static MICRO_ARCH Arch_SandyBridge[] = {{"SandyBridge"}, {NULL}};
static MICRO_ARCH Arch_SandyBridge_EP[] = {{"SandyBridge/eXtreme.EP"}, {NULL}};
static MICRO_ARCH Arch_IvyBridge[] = {{"IvyBridge"}, {NULL}};
static MICRO_ARCH Arch_IvyBridge_EP[] = {{"IvyBridge/EP"}, {NULL}};

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

static MICRO_ARCH Arch_Haswell_EP[] = {{"Haswell/EP/Mobile"}, {NULL}};
static MICRO_ARCH Arch_Haswell_ULT[] = {{"Haswell/Ultra Low TDP"}, {NULL}};
static MICRO_ARCH Arch_Haswell_ULX[] = {{"Haswell/Ultra Low eXtreme"}, {NULL}};
static MICRO_ARCH Arch_Broadwell[] = {{"Broadwell/Mobile"}, {NULL}};
static MICRO_ARCH Arch_Broadwell_D[] = {{"Broadwell/D"}, {NULL}};
static MICRO_ARCH Arch_Broadwell_H[] = {{"Broadwell/H"}, {NULL}};
static MICRO_ARCH Arch_Broadwell_EP[] = {{"Broadwell/EP/EX"}, {NULL}};
static MICRO_ARCH Arch_Skylake_UY[] = {{"Skylake/UY"}, {NULL}};
static MICRO_ARCH Arch_Skylake_S[] = {{"Skylake/S"}, {NULL}};
static MICRO_ARCH Arch_Skylake_X[] = {{"Skylake/X"}, {NULL}};
static MICRO_ARCH Arch_Xeon_Phi[] = {{"Knights Landing"}, {NULL}};

enum {
	CN_KABYLAKE,
	CN_KABYLAKE_S,
	CN_COFFEELAKE_S,
	CN_COFFEELAKE_R
};

static MICRO_ARCH Arch_Kabylake[] = {
	[CN_KABYLAKE]		= {"Kaby Lake"},
	[CN_KABYLAKE_S] 	= {"Kaby Lake/S"},
	[CN_COFFEELAKE_S]	= {"Coffee Lake/S"},
	[CN_COFFEELAKE_R]	= {"Coffee Lake/R"},
	{NULL}
};

static MICRO_ARCH Arch_Kabylake_UY[] = {{"Kaby Lake/UY"}, {NULL}};
static MICRO_ARCH Arch_Cannonlake[] = {{"Cannon Lake"}, {NULL}};
static MICRO_ARCH Arch_Geminilake[] = {{"Atom/Gemini Lake"}, {NULL}};
static MICRO_ARCH Arch_Icelake_UY[] = {{"Ice Lake/UY"}, {NULL}};

enum {
	CN_BULLDOZER,
	CN_PILEDRIVER,
	CN_STEAMROLLER,
	CN_EXCAVATOR
};

static MICRO_ARCH Arch_AMD_Family_0Fh[] = {{"Hammer"}, {NULL}};
static MICRO_ARCH Arch_AMD_Family_10h[] = {{"K10"}, {NULL}};
static MICRO_ARCH Arch_AMD_Family_11h[] = {{"Turion"}, {NULL}};
static MICRO_ARCH Arch_AMD_Family_12h[] = {{"Fusion"}, {NULL}};
static MICRO_ARCH Arch_AMD_Family_14h[] = {{"Bobcat"}, {NULL}};
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
	CN_RAVEN_RIDGE,
	CN_PINNACLE_RIDGE,
	CN_MATISSE,
	CN_WHITEHAVEN,
	CN_COLFAX,
	CN_CASTLE_PEAK,
	CN_NAPLES,
	CN_ROME,
};

static MICRO_ARCH Arch_AMD_Family_17h[] = {
	[CN_SUMMIT_RIDGE]	= {"Zen/Summit Ridge"},
	[CN_RAVEN_RIDGE]	= {"Zen/Raven Ridge"},
	[CN_PINNACLE_RIDGE]	= {"Zen+ Pinnacle Ridge"},
	[CN_MATISSE]		= {"Zen2/Matisse"},
	[CN_WHITEHAVEN] 	= {"Zen/Whitehaven"},
	[CN_COLFAX]		= {"Zen+ Colfax"},
	[CN_CASTLE_PEAK]	= {"Zen2/Castle Peak"},
	[CN_NAPLES]		= {"EPYC/Naples"},
	[CN_ROME]		= {"EPYC/Rome"},
	{NULL}
};

static MICRO_ARCH Arch_AMD_Family_18h[] = {{"Dhyana"}, {NULL}};

static PROCESSOR_SPECIFIC Void_Specific[] = {
	{
	.BrandSubStr = NULL,
	.Boost = {0, 0},
	.Param = {0, .Offset = { 0, 0}},
	.CodeNameIdx = 0,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	}
};

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
	.BrandSubStr = "Intel(R) Core(TM)2 Quad CPU Q84",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM)2 Quad CPU Q94",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM)2 Quad CPU Q95",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM)2 Quad CPU Q96",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM)2 Extreme CPU X96",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM)2 Extreme CPU X97",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_YORKFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
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
	.BrandSubStr = "Intel(R) Celeron(R) CPU E3",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_WOLFDALE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Pentium(R) Dual-Core CPU E5",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_WOLFDALE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Pentium(R) Dual-Core CPU E6",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_WOLFDALE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
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
	.BrandSubStr = "Intel(R) Core(TM)2 Extreme CPU Q9300",
	.Boost = {+2, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PENRYN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{NULL}
};

static PROCESSOR_SPECIFIC Nehalem_Bloomfield_Specific[] = {
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU 920",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = 0,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_TURBO_UNLOCK|LATCH_UNCORE_UNLOCK
	},
	{NULL}
};

static PROCESSOR_SPECIFIC Nehalem_Lynnfield_Specific[] = {
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU Q",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_CLARKSFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU X",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_CLARKSFIELD,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{NULL}
};

static PROCESSOR_SPECIFIC Westmere_EP_Specific[] = {
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU 970",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU 980",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU X 980",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7 CPU X 990",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Xeon(R) CPU W3690",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_GULFTOWN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{NULL}
};

static PROCESSOR_SPECIFIC Haswell_DT_Specific[] = {
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4940MX",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE_EX,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4930MX",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE_EX,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4910MQ",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4900MQ",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4810MQ",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4800MQ",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4710HQ",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-4700HQ",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CRYSTALWELL,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-47",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CANYON,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i5-46",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_CANYON,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Xeon(R) CPU E3-12",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_DENLOW,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) 4th Gen Core(TM) i",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_EMBEDDED,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i5-4300M",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i5-4200H",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i3-4000M",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Pentium(R) CPU 3",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Celeron(R) CPU 2",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_HASWELL_MOBILE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{NULL}
};

static PROCESSOR_SPECIFIC Kabylake_Specific[] = {
	{
	.BrandSubStr = "Intel(R) Core(TM) i3-7",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_KABYLAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_UNCORE_UNLOCK
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i5-7",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_KABYLAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_UNCORE_UNLOCK
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-7",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_KABYLAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 1,
	.Latch = LATCH_UNCORE_UNLOCK
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i3-8",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i5-8",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-8",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Pentium(R) Gold G5",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Celeron(R) G4",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_S,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i5-9600K",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_R,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i7-9700K",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_R,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{
	.BrandSubStr = "Intel(R) Core(TM) i9-9900K",
	.Boost = {0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_COFFEELAKE_R,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 0,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_NONE
	},
	{NULL}
};

/*	AMD Family 17h
	Remarks:Thermal Offset from Linux/k10temp.c
		+0.5 XFR rounded to +1
*/
static PROCESSOR_SPECIFIC Family_17h_Specific[] = {
	{	/* Index 0 is a placeholder: must be present!		*/
	.BrandSubStr = VENDOR_AMD,
	.Boost = {+0, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = 0,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Embedded V",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Embedded R",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon Silver",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon Gold",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon 3000G",
	.Boost = {+5, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon 300U",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon 2",
	.Boost = {+1, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon 3",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Athlon PRO",
	.Boost = {+5, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 PRO 1200",
	.Boost = {+3, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 1200",
	.Boost = {+3, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 PRO 1300",
	.Boost = {+2, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 1300X",
	.Boost = {+2, +2},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 PRO 2200GE",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 PRO 2200G",
	.Boost = {+2, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 2200GE",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 2200G",
	.Boost = {+2, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 2300X",
	.Boost = {+4, +1},
	.Param.Offset = {10, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 1400",
	.Boost = {+2, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 2400GE",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 2400G",
	.Boost = {+3, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2400GE",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2400G",
	.Boost = {+3, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 1500",
	.Boost = {+2, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 1500X",
	.Boost = {+2, +2},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2500U",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2500X",
	.Boost = {+4, +1},
	.Param.Offset = {10, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 1600",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 1600X",
	.Boost = {+4, +1},
	.Param.Offset = {20, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 1600",
	.Boost = {+4, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2600H",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 2600",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2600X",
	.Boost = {+5, +2},
	.Param.Offset = {10, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2600E",
	.Boost = {+8, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 2600",
	.Boost = {+3, +2},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 PRO 1700X",
	.Boost = {+4, 0},
	.Param.Offset = {20, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 PRO 1700",
	.Boost = {+7, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 1700X",
	.Boost = {+4, +1},
	.Param.Offset = {20, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 1700",
	.Boost = {+7, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 1800X",
	.Boost = {+4, +1},
	.Param.Offset = {20, 0},
	.CodeNameIdx = CN_SUMMIT_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 2800H",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 2700U",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_RAVEN_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 PRO 2700X",
	.Boost = {+5, 0},
	.Param.Offset = {10, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 PRO 2700",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 2700X",
	.Boost = {+5, +2},
	.Param.Offset = {10, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 2700E",
	.Boost = {+12, 0},
	.Param.Offset = {0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 2700",
	.Boost = {+8, +2},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 PRO 3200GE",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 PRO 3200G",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 3200GE",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 3 3200G",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 3400GE",
	.Boost = {+7, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 3400G",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3400GE",
	.Boost = {+7, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3400G",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3580U",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3550H",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3500U",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3500X",
	.Boost = {+5, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 PRO 3600",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3600X",
	.Boost = {+6, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 5 3600",
	.Boost = {+6, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 3780U",
	.Boost = {+17, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 3750H",
	.Boost = {+17, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 3700U",
	.Boost = {+17, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_PINNACLE_RIDGE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 PRO 3700",
	.Boost = {+8, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 3700X",
	.Boost = {+7, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 7 3800X",
	.Boost = {+5, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 9 PRO 3900",
	.Boost = {+12, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 0,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 9 3900X",
	.Boost = {+7, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen 9 3950X",
	.Boost = {+11, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_MATISSE,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 1950X",
	.Boost = {+6, +2},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 1920X",
	.Boost = {+5, +2},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 1900X",
	.Boost = {+2, +2},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_WHITEHAVEN,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 2990",
	.Boost = {+12, 0},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 2970",
	.Boost = {+12, 0},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 2950",
	.Boost = {+9, 0},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 2920",
	.Boost = {+8, 0},
	.Param.Offset = {27, 0},
	.CodeNameIdx = CN_COLFAX,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 3990X",
	.Boost = {+14, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 3970X",
	.Boost = {+8, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD Ryzen Threadripper 3960X",
	.Boost = {+7, +1},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_CASTLE_PEAK,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{	/* AMD EPYC Embedded Processors 			*/
	.BrandSubStr = "AMD EPYC 3",
	.Boost = {+16, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7351P",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7401P",
	.Boost = {+8, +2},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7551P",
	.Boost = {+6, +4},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7251",
	.Boost = {+8, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7261",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7281",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7301",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7351",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7401",
	.Boost = {+8, +2},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7451",
	.Boost = {+6, +3},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7501",
	.Boost = {+6, +4},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7551",
	.Boost = {+6, +4},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7601",
	.Boost = {+5, +5},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_NAPLES,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7232P",
	.Boost = {+1, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7252P",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7302P",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7402P",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7502P",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7702P",
	.Boost = {+14, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7252",
	.Boost = {+4, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7262",
	.Boost = {+2, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7272",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7282",
	.Boost = {+12, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7302",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7352",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7402",
	.Boost = {+6, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7452",
	.Boost = {+10, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7502",
	.Boost = {+9, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7542",
	.Boost = {+5, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7552",
	.Boost = {+12, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7642",
	.Boost = {+10, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7702",
	.Boost = {+14, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7742",
	.Boost = {+12, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{
	.BrandSubStr = "AMD EPYC 7H12",
	.Boost = {+7, 0},
	.Param.Offset = { 0, 0},
	.CodeNameIdx = CN_ROME,
	.TgtRatioUnlocked = 0,
	.ClkRatioUnlocked = 1,
	.TurboUnlocked = 1,
	.UncoreUnlocked = 0,
	.Latch = LATCH_CLK_RATIO_UNLOCK|LATCH_TURBO_UNLOCK
	},
	{NULL}
};

#ifdef CONFIG_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 19)
#define CPUFREQ_POLICY_UNKNOWN		(0)
#endif
static int CoreFreqK_Policy_Exit(struct cpufreq_policy*) ;
static int CoreFreqK_Policy_Init(struct cpufreq_policy*) ;
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 19))   \
  && (LINUX_VERSION_CODE <= KERNEL_VERSION(5, 4, 25)))  \
  || (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 3))
static int CoreFreqK_Policy_Verify(struct cpufreq_policy_data*) ;
#else
static int CoreFreqK_Policy_Verify(struct cpufreq_policy*) ;
#endif
static int CoreFreqK_SetPolicy(struct cpufreq_policy*) ;
static int CoreFreqK_Bios_Limit(int, unsigned int*) ;
static int CoreFreqK_SetBoost(int) ;
static ssize_t CoreFreqK_Show_SetSpeed(struct cpufreq_policy*, char*);
static int CoreFreqK_Store_SetSpeed(struct cpufreq_policy*, unsigned int) ;
#endif /* CONFIG_CPU_FREQ */
static unsigned int Policy_Intel_GetFreq(unsigned int cpu) ;
static void Policy_Core2_SetTarget(void *arg) ;
static void Policy_Nehalem_SetTarget(void *arg) ;
static void Policy_SandyBridge_SetTarget(void *arg) ;
static void Policy_HWP_SetTarget(void *arg) ;
#define Policy_Broadwell_EP_SetTarget	Policy_HWP_SetTarget
#define Policy_Skylake_SetTarget	Policy_HWP_SetTarget

static SYSTEM_DRIVER CORE2_Driver = {
	.IdleState	= NULL,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_Core2_SetTarget
};

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
	.flags		= (0x10 << 24) | 0x10000,
	.Latency	= 20,
	.Residency	= 80
	},
	{
	.Name		= "C6",
	.Desc		= "NHM-C6",
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 200,
	.Residency	= 800
	},
	{NULL}
};

static SYSTEM_DRIVER NHM_Driver = {
	.IdleState	= NHM_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_Nehalem_SetTarget
};

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
	.flags		= (0x10 << 24) | 0x10000,
	.Latency	= 80,
	.Residency	= 211
	},
	{
	.Name		= "C6",
	.Desc		= "SNB-C6",
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 104,
	.Residency	= 345
	},
	{
	.Name		= "C7",
	.Desc		= "SNB-C7",
	.flags		= (0x30 << 24) | 0x10000,
	.Latency	= 109,
	.Residency	= 345
	},
	{NULL}
};

static SYSTEM_DRIVER SNB_Driver = {
	.IdleState	= SNB_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_SandyBridge_SetTarget
};

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
	.flags		= (0x10 << 24) | 0x10000,
	.Latency	= 59,
	.Residency	= 156
	},
	{
	.Name		= "C6",
	.Desc		= "IVB-C6",
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 80,
	.Residency	= 300
	},
	{
	.Name		= "C7",
	.Desc		= "IVB-C7",
	.flags		= (0x30 << 24) | 0x10000,
	.Latency	= 87,
	.Residency	= 300
	},
	{NULL}
};

static SYSTEM_DRIVER IVB_Driver = {
	.IdleState	= IVB_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_SandyBridge_SetTarget
};

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
	.flags		= (0x10 << 24) | 0x10000,
	.Latency	= 33,
	.Residency	= 100
	},
	{
	.Name		= "C6",
	.Desc		= "HSW-C6",
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 133,
	.Residency	= 400
	},
	{
	.Name		= "C7",
	.Desc		= "HSW-C7",
	.flags		= (0x32 << 24) | 0x10000,
	.Latency	= 166,
	.Residency	= 500
	},
	{
	.Name		= "C8",
	.Desc		= "HSW-C8",
	.flags		= (0x40 << 24) | 0x10000,
	.Latency	= 300,
	.Residency	= 900
	},
	{
	.Name		= "C9",
	.Desc		= "HSW-C9",
	.flags		= (0x50 << 24) | 0x10000,
	.Latency	= 600,
	.Residency	= 1800
	},
	{
	.Name		= "C10",
	.Desc		= "HSW-C10",
	.flags		= (0x60 << 24) | 0x10000,
	.Latency	= 2600,
	.Residency	= 7700
	},
	{NULL}
};

static SYSTEM_DRIVER HSW_Driver = {
	.IdleState	= HSW_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_SandyBridge_SetTarget
};

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
	.flags		= (0x10 << 24) | 0x10000,
	.Latency	= 40,
	.Residency	= 100
	},
	{
	.Name		= "C6",
	.Desc		= "BDW-C6",
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 133,
	.Residency	= 400
	},
	{
	.Name		= "C7",
	.Desc		= "BDW-C7",
	.flags		= (0x32 << 24) | 0x10000,
	.Latency	= 166,
	.Residency	= 500
	},
	{
	.Name		= "C8",
	.Desc		= "BDW-C8",
	.flags		= (0x40 << 24) | 0x10000,
	.Latency	= 300,
	.Residency	= 900
	},
	{
	.Name		= "C9",
	.Desc		= "BDW-C9",
	.flags		= (0x50 << 24) | 0x10000,
	.Latency	= 600,
	.Residency	= 1800
	},
	{
	.Name		= "C10",
	.Desc		= "BDW-C10",
	.flags		= (0x60 << 24) | 0x10000,
	.Latency	= 2600,
	.Residency	= 7700
	},
	{NULL}
};

static SYSTEM_DRIVER BDW_Driver = {
	.IdleState	= BDW_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_SandyBridge_SetTarget
};

static SYSTEM_DRIVER BDW_EP_Driver = {
	.IdleState	= BDW_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_Broadwell_EP_SetTarget
};

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
	.flags		= (0x10 << 24) | 0x10000,
	.Latency	= 70,
	.Residency	= 100
	},
	{
	.Name		= "C6",
	.Desc		= "SKL-C6",
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 85,
	.Residency	= 200
	},
	{
	.Name		= "C7",
	.Desc		= "SKL-C7",
	.flags		= (0x33 << 24) | 0x10000,
	.Latency	= 124,
	.Residency	= 800
	},
	{
	.Name		= "C8",
	.Desc		= "SKL-C8",
	.flags		= (0x40 << 24) | 0x10000,
	.Latency	= 200,
	.Residency	= 800
	},
	{
	.Name		= "C9",
	.Desc		= "SKL-C9",
	.flags		= (0x50 << 24) | 0x10000,
	.Latency	= 480,
	.Residency	= 5000
	},
	{
	.Name		= "C10",
	.Desc		= "SKL-C10",
	.flags		= (0x60 << 24) | 0x10000,
	.Latency	= 890,
	.Residency	= 5000
	},
	{NULL}
};

static SYSTEM_DRIVER SKL_Driver = {
	.IdleState	= SKL_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_Skylake_SetTarget
};

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
	.flags		= (0x20 << 24) | 0x10000,
	.Latency	= 133,
	.Residency	= 600
	},
	{NULL}
};

static SYSTEM_DRIVER SKX_Driver = {
	.IdleState	= SKX_IdleState,
	.GetFreq	= Policy_Intel_GetFreq,
	.SetTarget	= Policy_Skylake_SetTarget
};

static ARCH Arch[ARCHITECTURES] = {
[GenuineIntel] = {							/*  0*/
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
	.Specific = Void_Specific,
	.SystemDriver = NULL,
	.Architecture = Arch_Void
	},
[Core_Yonah] = {							/*  1*/
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
	.SystemDriver = NULL,
	.Architecture = Arch_Core_Yonah
	},
[Core_Conroe] = {							/*  2*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Core_Conroe
	},
[Core_Kentsfield] = {							/*  3*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Core_Kentsfield
	},
[Core_Conroe_616] = {							/*  4*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Core_Conroe_616
	},
[Core_Penryn] = {							/*  5*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Core_Penryn
	},
[Core_Dunnington] = {							/*  6*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Core_Dunnington
	},

[Atom_Bonnell] = {							/*  7*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Bonnell
	},
[Atom_Silvermont] = {							/*  8*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Silvermont
	},
[Atom_Lincroft] = {							/*  9*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Lincroft
	},
[Atom_Clovertrail] = {							/* 10*/
	.Signature = _Atom_Clovertrail,
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Clovertrail
	},
[Atom_Saltwell] = {							/* 11*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Saltwell
	},

[Silvermont_637] = {							/* 12*/
	.Signature = _Silvermont_637,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Silvermont,
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Silvermont_637
	},
[Atom_Avoton] = {							/* 13*/
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
	.SystemDriver = NULL,
	.Architecture = Arch_Atom_Avoton
	},

[Atom_Airmont] = {							/* 14*/
	.Signature = _Atom_Airmont,
	.Query = Query_Core2,
	.Update = PerCore_Core2_Query,
	.Start = Start_Core2,
	.Stop = Stop_Core2,
	.Exit = NULL,
	.Timer = InitTimer_Core2,
	.BaseClock = BaseClock_Airmont,
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Airmont
	},
[Atom_Goldmont] = {							/* 15*/
	.Signature = _Atom_Goldmont,
	.Query = Query_SandyBridge,
	.Update = PerCore_Haswell_ULT_Query,
	.Start = Start_Haswell_ULT,
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
	.SystemDriver = &SNB_Driver,
	.Architecture = Arch_Atom_Goldmont
	},
[Atom_Sofia] = {							/* 16*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Sofia
	},
[Atom_Merrifield] = {							/* 17*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Merrifield
	},
[Atom_Moorefield] = {							/* 18*/
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
	.SystemDriver = &CORE2_Driver,
	.Architecture = Arch_Atom_Moorefield
	},

[Nehalem_Bloomfield] = {						/* 19*/
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
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Nehalem_Bloomfield_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Nehalem_Bloomfield
	},
[Nehalem_Lynnfield] = { 						/* 20*/
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
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Nehalem_Lynnfield_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Nehalem_Lynnfield
	},
[Nehalem_MB] = {							/* 21*/
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
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Nehalem_MB
	},
[Nehalem_EX] = {							/* 22*/
	.Signature = _Nehalem_EX,
	.Query = Query_Core2,
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Nehalem,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = NULL,
	.thermalFormula = THERMAL_FORMULA_INTEL,
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Nehalem_EX
	},

[Westmere] = {								/* 23*/
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
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_DMI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Westmere
	},
[Westmere_EP] = {							/* 24*/
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
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Westmere_EP_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Westmere_EP_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Westmere_EP
	},
[Westmere_EX] = {							/* 25*/
	.Signature = _Westmere_EX,
	.Query = Query_Core2, /* Xeon processor 7500 series-based platform */
	.Update = PerCore_Nehalem_Query,
	.Start = Start_Nehalem,
	.Stop = Stop_Nehalem,
	.Exit = NULL,
	.Timer = InitTimer_Nehalem,
	.BaseClock = BaseClock_Westmere,
	.ClockMod = ClockMod_Nehalem_PPC,
	.TurboClock = Intel_Turbo_Config8C,
	.thermalFormula = THERMAL_FORMULA_INTEL,
#if defined(HWM_CHIPSET) && (HWM_CHIPSET == W83627)
	.voltageFormula = VOLTAGE_FORMULA_WINBOND_IO,
#else
	.voltageFormula = VOLTAGE_FORMULA_NONE,
#endif
	.powerFormula   = POWER_FORMULA_NONE,
	.PCI_ids = PCI_Nehalem_QPI_ids,
	.Uncore = {
		.Start = Start_Uncore_Nehalem,
		.Stop = Stop_Uncore_Nehalem,
		.ClockMod = NULL
		},
	.Specific = Void_Specific,
	.SystemDriver = &NHM_Driver,
	.Architecture = Arch_Westmere_EX
	},

[SandyBridge] = {							/* 26*/
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
	.SystemDriver = &SNB_Driver,
	.Architecture = Arch_SandyBridge
	},
[SandyBridge_EP] = {							/* 27*/
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
	.Specific = Void_Specific,
	.SystemDriver = &SNB_Driver,
	.Architecture = Arch_SandyBridge_EP
	},

[IvyBridge]  = {							/* 28*/
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
	.SystemDriver = &IVB_Driver,
	.Architecture = Arch_IvyBridge
	},
[IvyBridge_EP] = {							/* 29*/
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
	.Specific = Void_Specific,
	.SystemDriver = &IVB_Driver,
	.Architecture = Arch_IvyBridge_EP
	},

[Haswell_DT] = {							/* 30*/
	.Signature = _Haswell_DT,
	.Query = Query_Haswell,
	.Update = PerCore_IvyBridge_Query,
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
	.SystemDriver = &HSW_Driver,
	.Architecture = Arch_Haswell_DT
	},
[Haswell_EP] = {							/* 31*/
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
	.Specific = Void_Specific,
	.SystemDriver = &HSW_Driver,
	.Architecture = Arch_Haswell_EP
	},
[Haswell_ULT] = {							/* 32*/
	.Signature = _Haswell_ULT,
	.Query = Query_IvyBridge,
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
	.SystemDriver = &HSW_Driver,
	.Architecture = Arch_Haswell_ULT
	},
[Haswell_ULX] = {							/* 33*/
	.Signature = _Haswell_ULX,
	.Query = Query_IvyBridge,
	.Update = PerCore_IvyBridge_Query,
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
	.SystemDriver = &HSW_Driver,
	.Architecture = Arch_Haswell_ULX
	},

[Broadwell]  = {							/* 34*/
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
	.SystemDriver = &BDW_Driver,
	.Architecture = Arch_Broadwell
	},
[Broadwell_D] = {							/* 35*/
	.Signature = _Broadwell_D,
	.Query = Query_Broadwell_EP,
	.Update = PerCore_Haswell_EP_Query,
	.Start = Start_Haswell_EP,
	.Stop = Stop_Haswell_EP,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_EP,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_Broadwell_EP_HWP,
	.TurboClock = TurboClock_Haswell_EP,
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
	.SystemDriver = &BDW_EP_Driver,
	.Architecture = Arch_Broadwell_D
	},
[Broadwell_H] = {							/* 36*/
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
	.SystemDriver = &BDW_Driver,
	.Architecture = Arch_Broadwell_H
	},
[Broadwell_EP] = {							/* 37*/
	.Signature = _Broadwell_EP,
	.Query = Query_Broadwell_EP,
	.Update = PerCore_Haswell_EP_Query,
	.Start = Start_Haswell_EP,
	.Stop = Stop_Haswell_EP,
	.Exit = NULL,
	.Timer = InitTimer_Haswell_EP,
	.BaseClock = BaseClock_Haswell,
	.ClockMod = ClockMod_Broadwell_EP_HWP,
	.TurboClock = TurboClock_Haswell_EP,
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
	.SystemDriver = &BDW_EP_Driver,
	.Architecture = Arch_Broadwell_EP
	},

[Skylake_UY] = {							/* 38*/
	.Signature = _Skylake_UY,
	.Query = Query_Broadwell,
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
	.SystemDriver = &SKL_Driver,
	.Architecture = Arch_Skylake_UY
	},
[Skylake_S]  = {							/* 39*/
	.Signature = _Skylake_S,
	.Query = Query_Broadwell,
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
	.SystemDriver = &SKL_Driver,
	.Architecture = Arch_Skylake_S
	},
[Skylake_X]  = {							/* 40*/
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
	.Specific = Void_Specific,
	.SystemDriver = &SKX_Driver,
	.Architecture = Arch_Skylake_X
	},

[Xeon_Phi] = {								/* 41*/
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
	.SystemDriver = NULL,
	.Architecture = Arch_Xeon_Phi
	},

[Kabylake] = {								/* 42*/
	.Signature = _Kabylake,
	.Query = Query_Broadwell,
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
	.SystemDriver = &SKL_Driver,
	.Architecture = Arch_Kabylake
	},
[Kabylake_UY] = {							/* 43*/
	.Signature = _Kabylake_UY,
	.Query = Query_Broadwell,
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
	.SystemDriver = &SKL_Driver,
	.Architecture = Arch_Kabylake_UY
	},

[Cannonlake] = {							/* 44*/
	.Signature = _Cannonlake,
	.Query = Query_Broadwell,
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
	.SystemDriver = NULL,
	.Architecture = Arch_Cannonlake
	},

[Geminilake] = {							/* 45*/
	.Signature = _Geminilake,
	.Query = Query_SandyBridge,
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
	.SystemDriver = NULL,
	.Architecture = Arch_Geminilake
	},

[Icelake_UY] = {							/* 46*/
	.Signature = _Icelake_UY,
	.Query = Query_SandyBridge,
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
	.SystemDriver = NULL,
	.Architecture = Arch_Icelake_UY
	},

[AMD_Family_0Fh] = {
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_0Fh
	},

[AMD_Family_10h] = {
	.Signature = _AMD_Family_10h,
	.Query = Query_AMD_Family_10h,
	.Update = PerCore_AMD_Family_10h_Query,
	.Start = Start_AMD_Family_10h,
	.Stop = Stop_AMD_Family_10h,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_10h
	},

[AMD_Family_11h] = {
	.Signature = _AMD_Family_11h,
	.Query = Query_AMD_Family_11h,
	.Update = PerCore_AMD_Family_11h_Query,
	.Start = Start_AMD_Family_11h,
	.Stop = Stop_AMD_Family_11h,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_11h
	},

[AMD_Family_12h] = {
	.Signature = _AMD_Family_12h,
	.Query = Query_AMD_Family_12h,
	.Update = PerCore_AMD_Family_12h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_12h
	},

[AMD_Family_14h] = {
	.Signature = _AMD_Family_14h,
	.Query = Query_AMD_Family_14h,
	.Update = PerCore_AMD_Family_14h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_14h
	},

[AMD_Family_15h] = {
	.Signature = _AMD_Family_15h,
	.Query = Query_AMD_Family_15h,
	.Update = PerCore_AMD_Family_15h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_15h
	},

[AMD_Family_16h] = {
	.Signature = _AMD_Family_16h,
	.Query = Query_AMD_Family_15h,
	.Update = PerCore_AMD_Family_15h_Query,
	.Start = Start_AuthenticAMD,
	.Stop = Stop_AuthenticAMD,
	.Exit = NULL,
	.Timer = InitTimer_AuthenticAMD,
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_16h
	},

[AMD_Family_17h] = {
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
	.Specific = Family_17h_Specific,
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_17h
	},

[AMD_Family_18h] = {
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
	.SystemDriver = NULL,
	.Architecture = Arch_AMD_Family_18h
	}
};

