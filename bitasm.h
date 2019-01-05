/*
 * CoreFreq
 * Copyright (C) 2015-2019 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define MAXCOUNTER(M, m)	((M) > (m) ? (M) : (m))
#define MINCOUNTER(m, M)	((m) < (M) ? (m) : (M))

typedef unsigned long long int	Bit64;
typedef unsigned int		Bit32;

#define LOCKLESS " "
#define BUS_LOCK "lock "

#define RDTSC(_lo, _hi) 						\
__asm__ volatile							\
(									\
	"lfence"		"\n\t"					\
	"rdtsc"								\
	: "=a" (_lo),							\
	  "=d" (_hi)							\
)

#define RDTSCP(_lo, _hi, aux)						\
__asm__ volatile							\
(									\
	"rdtscp"							\
	: "=a" (_lo),							\
	  "=d" (_hi),							\
	  "=c" (aux)							\
)

#define BARRIER(pfx)							\
__asm__ volatile							\
(									\
	#pfx"fence"							\
	:								\
	:								\
	:								\
)

#define WBINVD()							\
__asm__ volatile							\
(									\
	"wbinvd"							\
	:								\
	:								\
	: "memory"							\
)

#define SERIALIZE()							\
__asm__ volatile							\
(									\
	"xorq %%rax,%%rax"	"\n\t"					\
	"cpuid"								\
	:								\
	:								\
	: "%rax"							\
)

#define RDTSC64(_val64) 						\
__asm__ volatile							\
(									\
	"lfence"		"\n\t"					\
	"rdtsc"			"\n\t"					\
	"shlq	$32,	%%rdx"	"\n\t"					\
	"orq	%%rdx,	%%rax"	"\n\t"					\
	"movq	%%rax,	%0"						\
	: "=m" (_val64)							\
	:								\
	: "%rax","%rcx","%rdx","memory"					\
)

#define RDTSCP64(_val64)						\
__asm__ volatile							\
(									\
	"rdtscp"		"\n\t"					\
	"shlq	$32,	%%rdx"	"\n\t"					\
	"orq	%%rdx,	%%rax"	"\n\t"					\
	"movq	%%rax,	%0"						\
	: "=m" (_val64)							\
	:								\
	: "%rax","%rcx","%rdx","memory"					\
)

#define ASM_RDTSCP(_reg)						\
	"# Read invariant TSC."		"\n\t"				\
	"rdtscp"			"\n\t"				\
	"shlq	$32, %%rdx"		"\n\t"				\
	"orq	%%rdx, %%rax"		"\n\t"				\
	"# Save TSC value."		"\n\t"				\
	"movq	%%rax, %%" #_reg	"\n\t"

#define ASM_RDTSC(_reg) 						\
	"# Read variant TSC."		"\n\t"				\
	"lfence"			"\n\t"				\
	"rdtsc"				"\n\t"				\
	"shlq	$32, %%rdx"		"\n\t"				\
	"orq	%%rdx, %%rax"		"\n\t"				\
	"# Save TSC value."		"\n\t"				\
	"movq	%%rax, %%" #_reg	"\n\t"

#define ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Read PMC counter."		"\n\t"				\
	"movq	$" #_ctr ", %%rcx"	"\n\t"				\
	"rdpmc"				"\n\t"				\
	"shlq	$32, %%rdx"		"\n\t"				\
	"orq	%%rdx, %%rax"		"\n\t"				\
	"# Save counter value."		"\n\t"				\
	"movq	%%rax, %%" #_reg	"\n\t"

#define ASM_RDPMC(_ctr, _reg) ASM_CODE_RDPMC(_ctr, _reg)

#define RDPMC(_ctr, _reg, _mem) 					\
__asm__ volatile							\
(									\
	ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg ", %0"					\
	: "=m" (_mem)							\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg"", 							\
	  "memory"							\
)

#define ASM_RDTSC_PMCx1(_reg0, _reg1,					\
			_tsc_inst, mem_tsc,				\
			_ctr1, _mem1)					\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDPMC(_ctr1, _reg1)						\
	"# Store values into memory."	"\n\t"				\
	"movq	%%" #_reg0 ", %0"	"\n\t"				\
	"movq	%%" #_reg1 ", %1"					\
	: "=m" (mem_tsc), "=m" (_mem1)					\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0"", "%" #_reg1"",					\
	  "memory"							\
)

#define RDTSC_PMCx1(mem_tsc, ...)					\
ASM_RDTSC_PMCx1(r14, r15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_PMCx1(mem_tsc, ...)					\
ASM_RDTSC_PMCx1(r14, r15, ASM_RDTSCP, mem_tsc, __VA_ARGS__)


#define _BITSET_GPR(_lock, _base, _offset)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btsq	%%rdx, %[base]"					\
		: [base] "=m" (_base)					\
		: "d" (_offset)						\
		: "cc", "memory"					\
	);								\
})

#define _BITSET_IMM(_lock, _base, _imm8)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btsq	%[imm8], %[base]"				\
		: [base] "=m" (_base)					\
		: [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
})

#define _BITCLR_GPR(_lock, _base, _offset)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btrq	%%rdx,	%[base]"				\
		: [base] "=m" (_base)					\
		: "d" (_offset)						\
		: "cc", "memory"					\
	);								\
})

#define _BITCLR_IMM(_lock, _base, _imm8)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btrq	%[imm8], %[base]"				\
		: [base] "=m" (_base)					\
		: [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
})

#define _BITBTC_GPR(_lock,_base, _offset)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btcq	%%rdx,	%[base]"				\
		: [base] "=m" (_base)					\
		: "d" (_offset)						\
		: "cc", "memory"					\
	);								\
})

#define _BITBTC_IMM(_lock, _base, _imm8)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btcq	%[imm8], %[base]"				\
		: [base] "=m" (_base)					\
		: [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
})

#define _BIT_TEST_GPR(_base, _offset)					\
({									\
	register unsigned char _ret = 0;				\
	__asm__ volatile						\
	(								\
		"btq	%%rdx, %[base]"	"\n\t"				\
		"setc	%[ret]"						\
		: [ret]	"+r" (_ret)					\
		: [base] "m" (_base),					\
		  "d" (_offset)						\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BIT_TEST_IMM(_base, _imm8)					\
({									\
	register unsigned char _ret = 0;				\
	__asm__ volatile						\
	(								\
		"btq	%[imm8], %[base]""\n\t" 			\
		"setc	%[ret]"						\
		: [ret]	"+r" (_ret)					\
		: [base] "m" (_base),					\
		  [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITWISEAND(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _ret __attribute__ ((aligned (64)))=_opl;	\
	__asm__ volatile						\
	(								\
	_lock	"andq %[opr], %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [opr] "Jr" (_opr)					\
		: "memory"						\
	);								\
	_ret;								\
})

#define _BITWISEOR(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _ret __attribute__ ((aligned (64))) = _opl;	\
	__asm__ volatile						\
	(								\
	_lock	"orq %[opr], %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [opr] "Jr" (_opr)					\
		: "memory"						\
	);								\
	_ret;								\
})

#define _BITWISEXOR(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _ret __attribute__ ((aligned (64))) = _opl;	\
	__asm__ volatile						\
	(								\
	_lock	"xorq %[opr], %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [opr] "Jr" (_opr)					\
		: "memory"						\
	);								\
	_ret;								\
})

#define BITMSK(_lock, _base, _offset)					\
({									\
	__asm__ volatile						\
	(								\
	_lock	"orq	$0xffffffffffffffff, %[base]"	"\n\t"		\
	_lock	"btc	%[offset], %[base]"				\
		: [base] "=m" (_base)					\
		: [offset] "Jr" (_offset)				\
		: "cc", "memory"					\
	);								\
})

#define BITSET(_lock, _base, _offset)					\
(									\
	__builtin_constant_p(_offset) ?					\
		_BITSET_IMM(_lock, _base, _offset)			\
	:	_BITSET_GPR(_lock, _base, _offset)			\
)

#define BITCLR(_lock, _base, _offset)					\
(									\
	__builtin_constant_p(_offset) ?					\
		_BITCLR_IMM(_lock, _base, _offset)			\
	:	_BITCLR_GPR(_lock, _base, _offset)			\
)

#define BITBTC(_lock, _base, _offset)					\
(									\
	__builtin_constant_p(_offset) ?					\
		_BITBTC_IMM(_lock, _base, _offset)			\
	:	_BITBTC_GPR(_lock, _base, _offset)			\
)

#define BITVAL(_base, _offset)						\
(									\
	__builtin_constant_p(_offset) ?					\
		_BIT_TEST_IMM(_base, _offset)				\
	:	_BIT_TEST_GPR(_base, _offset)				\
)

#define BITCPL(_src)							\
({									\
	unsigned long long _dest;					\
	__asm__ volatile						\
	(								\
		"movq	%[src], %[dest]"	"\n\t"			\
		"negq	%[dest]"					\
		: [dest] "=m" (_dest)					\
		: [src] "ir" (_src)					\
		: "memory"						\
	);								\
	_dest;								\
})

#define BITWISEAND(_lock, _opl, _opr)	_BITWISEAND(_lock, _opl, _opr)
#define BITWISEOR(_lock, _opl, _opr)	_BITWISEOR(_lock, _opl, _opr)
#define BITWISEXOR(_lock, _opl, _opr)	_BITWISEXOR(_lock, _opl, _opr)

#define BITSTOR(_lock, _dest, _src)					\
	_dest = BITWISEAND(_lock, _src, 0xffffffffffffffff)

#define BITBSF(_base, _index)						\
({									\
	register unsigned char _ret = 0;				\
	__asm__ volatile						\
	(								\
		"bsf	%[base], %[index]"	"\n\t"			\
		"setz	%[ret]"						\
		: [ret]   "+r" (_ret),					\
		  [index] "=r" (_index) 				\
		: [base]  "rm" (_base)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define BITBSR(_base, _index)						\
({									\
	register unsigned char _ret = 0;				\
	__asm__ volatile						\
	(								\
		"bsr	%[base], %[index]"	"\n\t"			\
		"setz	%[ret]"						\
		: [ret]   "+r" (_ret),					\
		  [index] "=r" (_index) 				\
		: [base]  "rm" (_base)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define BITEXTRZ(_src, _offset, _length)				\
({									\
	unsigned long long _dest;					\
	__asm__ volatile						\
	(								\
		"movq	$1, %%rdx"		"\n\t"			\
		"mov	%[len], %%ecx"		"\n\t"			\
		"shlq	%%cl, %%rdx"		"\n\t"			\
		"decq	%%rdx"			"\n\t"			\
		"mov	%[ofs], %%ecx"		"\n\t"			\
		"shlq	%%cl, %%rdx"		"\n\t"			\
		"andq	%[src], %%rdx"		"\n\t"			\
		"shrq	%%cl, %%rdx"		"\n\t"			\
		"movq	%%rdx, %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [src] "ir" (_src),					\
		  [ofs] "ir" (_offset),					\
		  [len] "ir" (_length)					\
		: "%ecx", "%rdx", "memory"				\
	);								\
	_dest;								\
})
