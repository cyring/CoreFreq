/*
 * CoreFreq
 * Copyright (C) 2015-2025 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define FEAT_MESSAGE(_msg)		_Pragma(#_msg)
#define FEAT_MSG(_msg)			FEAT_MESSAGE(message(#_msg))

enum {
	SYNC	=  0,
	SYNC0	= 10,
	SYNC1	= 11,
	BURN	= 31,
	COMP0	= 40,
	COMP1	= 41,
	NTFY0	= 50,
	NTFY1	= 51,
	NTFY	= 63
};

#define BIT_MASK_SYNC							\
	0b0000000000000000000000000000000000000000000000000000110000000000LLU

#define BIT_MASK_COMP							\
	0b0000000000000000000000110000000000000000000000000000000000000000LLU

#define BIT_MASK_NTFY							\
	0b0000000000001100000000000000000000000000000000000000000000000000LLU

#define MAXCOUNTER(M, m)	((M) > (m) ? (M) : (m))
#define MINCOUNTER(m, M)	((m) < (M) ? (m) : (M))

#define CORE_COUNT_MASK(_cc)	(_cc - 1)
#define CORE_WORD_TOP(_cc)	(CORE_COUNT_MASK(_cc) >> 6)
#define CORE_WORD_MOD(_cc_,_offset_) ((_offset_ & CORE_COUNT_MASK(_cc_)) & 0x3f)
#define CORE_WORD_POS(_cc_,_offset_) ((_offset_ & CORE_COUNT_MASK(_cc_)) >> 6)

typedef unsigned long long int	Bit1024[16];
typedef unsigned long long int	Bit512[8];
typedef unsigned long long int	Bit256[4];
typedef unsigned long long int	Bit128[2];
typedef unsigned long long int	Bit64;
typedef unsigned int		Bit32;

#define BitT2(_cc)		Bit##_cc
#define BitT1(_cc)		BitT2(_cc)
#define BitCC			BitT1(CORE_COUNT)

#if (CORE_COUNT == 64)
#define InitCC(_val)		(_val)
#else
#define InitCC(_val)		{[0 ... CORE_WORD_TOP(CORE_COUNT) - 1] = _val}
#endif

#define ATOMIC_SEED 0x436f726546726571LLU

#define BIT_IO_RETRIES_COUNT	80
#define BIT_IO_DELAY_INTERVAL	150	/*	in udelay() unit	*/

#define LOCKLESS " "
#define BUS_LOCK "lock "

#define BARRIER(pfx)							\
__asm__ volatile							\
(									\
	#pfx"fence"							\
	:								\
	:								\
	: "memory"							\
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
	"cpuid" 							\
	:								\
	:								\
	: "%rax", "%rbx", "%rcx", "%rdx"				\
)

#define RDTSC64(_mem64) 						\
__asm__ volatile							\
(									\
	"lfence"				"\n\t"			\
	"rdtsc" 				"\n\t"			\
	"shlq	$32	,	%%rdx"		"\n\t"			\
	"orq	%%rdx	,	%%rax"		"\n\t"			\
	"movq	%%rax	,	%0"					\
	: "=m" (_mem64) 						\
	:								\
	: "%rax", "%rcx", "%rdx", "cc", "memory"			\
)

#define RDTSCP64(_mem64)						\
__asm__ volatile							\
(									\
	"rdtscp"				"\n\t"			\
	"shlq	$32	,	%%rdx"		"\n\t"			\
	"orq	%%rdx	,	%%rax"		"\n\t"			\
	"movq	%%rax	,	%0"					\
	: "=m" (_mem64) 						\
	:								\
	: "%rax", "%rcx", "%rdx", "cc", "memory"			\
)

#define ASM_RDTSCP(_reg)						\
	"# Read invariant TSC." 		"\n\t"			\
	"rdtscp"				"\n\t"			\
	"shlq	$32	,	%%rdx"		"\n\t"			\
	"orq	%%rdx	,	%%rax"		"\n\t"			\
	"# Save TSC value."			"\n\t"			\
	"movq	%%rax	,	%%" #_reg	"\n\t"

#define ASM_RDTSC(_reg) 						\
	"# Read variant TSC."			"\n\t"			\
	"lfence"				"\n\t"			\
	"rdtsc" 				"\n\t"			\
	"shlq	$32	,	%%rdx"		"\n\t"			\
	"orq	%%rdx	,	%%rax"		"\n\t"			\
	"# Save TSC value."			"\n\t"			\
	"movq	%%rax	,	%%" #_reg	"\n\t"

#define ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Read PMC counter."			"\n\t"			\
	"movq	$" #_ctr ",	%%rcx"		"\n\t"			\
	"rdpmc" 				"\n\t"			\
	"shlq	$32	,	%%rdx"		"\n\t"			\
	"orq	%%rdx	,	%%rax"		"\n\t"			\
	"# Save counter value." 		"\n\t"			\
	"movq	%%rax	,	%%" #_reg	"\n\t"

#define ASM_RDPMC(_ctr, _reg) ASM_CODE_RDPMC(_ctr, _reg)

#define RDPMC(_ctr, _reg, _mem) 					\
__asm__ volatile							\
(									\
	ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg ",	%0"					\
	: "=m" (_mem)							\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg"", 							\
	  "cc", "memory"						\
)

#define ASM_RDTSC_PMCx1(_reg0, _reg1,					\
			_tsc_inst, mem_tsc,				\
			_ctr1, _mem1)					\
__asm__ volatile							\
(									\
	_tsc_inst(_reg0)						\
	ASM_RDPMC(_ctr1, _reg1) 					\
	"# Store values into memory."		"\n\t"			\
	"movq	%%" #_reg0 ",	%0"		"\n\t"			\
	"movq	%%" #_reg1 ",	%1"					\
	: "=m" (mem_tsc), "=m" (_mem1)					\
	:								\
	: "%rax", "%rcx", "%rdx",					\
	  "%" #_reg0""	,	"%" #_reg1"",				\
	  "cc", "memory"						\
)

#define RDTSC_PMCx1(mem_tsc, ...)					\
ASM_RDTSC_PMCx1(r14, r15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define RDTSCP_PMCx1(mem_tsc, ...)					\
ASM_RDTSC_PMCx1(r14, r15, ASM_RDTSCP, mem_tsc, __VA_ARGS__)


#define _BITSET_GPR(_lock, _base, _offset)				\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"btsq	%%rdx,	%[base]"	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret),					\
		  [base] "=m" (_base)					\
		: "d" (_offset)						\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITSET_IMM(_lock, _base, _imm8)				\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"btsq	%[imm8], %[base]"	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret),					\
		  [base] "=m" (_base)					\
		: [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITCLR_GPR(_lock, _base, _offset)				\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"btrq	%%rdx,	%[base]"	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret),					\
		  [base] "=m" (_base)					\
		: "d" (_offset)						\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITCLR_IMM(_lock, _base, _imm8)				\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"btrq	%[imm8], %[base]"	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret),					\
		  [base] "=m" (_base)					\
		: [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITBTC_GPR(_lock,_base, _offset)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"btcq	%%rdx,	%[base]"				\
		: [base] "=m" (_base)					\
		: "d" (_offset) 					\
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

#define _BITVAL_GPR(_lock,_base, _offset)				\
({									\
	Bit64 _tmp __attribute__ ((aligned (8))) = _base;		\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"btcq	%%rdx,	%[tmp]"		"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret)					\
		: [tmp] "m" (_tmp),					\
		  "d" (_offset) 					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITVAL_IMM(_lock, _base, _imm8)				\
({									\
	Bit64 _tmp __attribute__ ((aligned (8))) = _base;		\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"btcq	%[imm8], %[tmp]"	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret)					\
		: [tmp] "m"  (_tmp),					\
		  [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BIT_TEST_GPR(_base, _offset)					\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"btq	%%rdx, %[base]" 	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret)					\
		: [base] "m" (_base),					\
		  "d" ( _offset )					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BIT_TEST_IMM(_base, _imm8)					\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"btq	%[imm8], %[base]"	"\n\t"			\
		"setc	%[ret]" 					\
		: [ret] "+m" (_ret)					\
		: [base] "m" (_base),					\
		  [imm8] "i" (_imm8)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define _BITWISEAND(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _dest __attribute__ ((aligned (8))) = _opl;	\
									\
	__asm__ volatile						\
	(								\
	_lock	"andq %[opr], %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [opr]  "Jr" (_opr)					\
		: "cc", "memory"					\
	);								\
	_dest;								\
})

#define _BITWISEOR(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _dest __attribute__ ((aligned (8))) = _opl;	\
									\
	__asm__ volatile						\
	(								\
	_lock	"orq %[opr], %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [opr]  "Jr" (_opr)					\
		: "cc", "memory"					\
	);								\
	_dest;								\
})

#define _BITWISEXOR(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _dest __attribute__ ((aligned (8))) = _opl;	\
									\
	__asm__ volatile						\
	(								\
	_lock	"xorq %[opr], %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [opr]  "Jr" (_opr)					\
		: "cc", "memory"					\
	);								\
	_dest;								\
})

#define BITSET(_lock, _base, _offset)					\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BITSET_IMM(_lock, _base, _offset)			\
	:	_BITSET_GPR(_lock, _base, _offset)			\
)

#define BITCLR(_lock, _base, _offset)					\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BITCLR_IMM(_lock, _base, _offset)			\
	:	_BITCLR_GPR(_lock, _base, _offset)			\
)

#define BITBTC(_lock, _base, _offset)					\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BITBTC_IMM(_lock, _base, _offset)			\
	:	_BITBTC_GPR(_lock, _base, _offset)			\
)

#define BITVAL_2xPARAM(_base, _offset)					\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BIT_TEST_IMM(_base, _offset)				\
	:	_BIT_TEST_GPR(_base, _offset)				\
)

#define BITVAL_3xPARAM(_lock, _base, _offset)				\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BITVAL_IMM(_lock, _base, _offset)			\
	:	_BITVAL_GPR(_lock, _base, _offset)			\
)

#define BITVAL_DISPATCH(_1,_2,_3,BITVAL_CURSOR, ...)			\
	BITVAL_CURSOR

#define BITVAL(...)							\
	BITVAL_DISPATCH( __VA_ARGS__ ,	BITVAL_3xPARAM ,		\
					BITVAL_2xPARAM ,		\
					NULL)( __VA_ARGS__ )

#define BITCPL(_src)							\
({									\
	unsigned long long _dest;					\
									\
	__asm__ volatile						\
	(								\
		"movq	%[src], %[dest]"	"\n\t"			\
		"negq	%[dest]"					\
		: [dest] "=m" (_dest)					\
		: [src] "ir" (_src)					\
		: "cc", "memory"					\
	);								\
	_dest;								\
})

#define BITWISEAND(_lock, _opl, _opr)	_BITWISEAND(_lock, _opl, _opr)
#define BITWISEOR(_lock, _opl, _opr)	_BITWISEOR(_lock, _opl, _opr)
#define BITWISEXOR(_lock, _opl, _opr)	_BITWISEXOR(_lock, _opl, _opr)

#define _BITADD_GPR(_lock, _dest, _src) 				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"addq	%%rax, %[dest]" 				\
		: [dest] "=m" (_dest)					\
		: "a" (_src)						\
		: "cc", "memory"					\
	);								\
})

#define _BITADD_IMM(_lock, _dest, _imm32)				\
({									\
	__asm__ volatile						\
	(								\
	_lock	"addq	%[imm32], %[dest]"				\
		: [dest] "=m" (_dest)					\
		: [imm32] "i" (_imm32)					\
		: "cc", "memory"					\
	);								\
})

#define BITADD(_lock, _dest, _src)					\
(									\
	__builtin_constant_p(_src) ?					\
		_BITADD_IMM(_lock , _dest , _src)			\
	:	_BITADD_GPR(_lock , _dest , _src)			\
)

#define BITSTOR(_lock, _dest, _src)					\
({									\
	__asm__ volatile						\
	(								\
		"movq	%[src], %%rsi"			"\n\t"		\
	_lock	"orq	$0xffffffffffffffff, %[dest]"	"\n\t"		\
	_lock	"andq	%%rsi , %[dest]"				\
		: [dest] "=m" (_dest)					\
		: [src] "Jmr" (_src)					\
		: "cc", "memory", "%rsi"				\
	);								\
})

#define BITZERO(_lock, _src)						\
({									\
	Bit64 _tmp __attribute__ ((aligned (8))) = _src;		\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
	_lock	"orq	$0x0, %[tmp]"		"\n\t"			\
		"setz	%[ret]" 					\
		: [ret] "+m" (_ret)					\
		: [tmp] "m" (_tmp)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define BITBSF(_base, _index)						\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"bsf	%[base], %[index]"	"\n\t"			\
		"setz	%[ret]" 					\
		: [ret]   "+m" (_ret),					\
		  [index] "=r" (_index) 				\
		: [base]  "rm" (_base)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define BITBSR(_base, _index)						\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"bsr	%[base], %[index]"	"\n\t"			\
		"setz	%[ret]" 					\
		: [ret]   "+m" (_ret),					\
		  [index] "=r" (_index) 				\
		: [base]  "rm" (_base)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

#define BITEXTRZ(_src, _offset, _length)				\
({									\
	volatile unsigned long long _dest;				\
									\
	__asm__ volatile						\
	(								\
		"movq	$1	,	%%rdx"		"\n\t"		\
		"mov	%[len]	,	%%ecx"		"\n\t"		\
		"shlq	%%cl	,	%%rdx"		"\n\t"		\
		"decq	%%rdx"				"\n\t"		\
		"mov	%[ofs]	,	%%ecx"		"\n\t"		\
		"shlq	%%cl	,	%%rdx"		"\n\t"		\
		"andq	%[src]	,	%%rdx"		"\n\t"		\
		"shrq	%%cl	,	%%rdx"		"\n\t"		\
		"movq	%%rdx	,	%[dest]" 			\
		: [dest]	"=m"	(_dest) 			\
		: [src] 	"irm"	(_src) ,			\
		  [ofs] 	"irm"	(_offset),			\
		  [len] 	"irm"	(_length) 			\
		: "%ecx", "%rdx", "cc", "memory"			\
	);								\
	_dest;								\
})

#define BIT_ATOM_INIT(atom, seed)					\
({									\
	__asm__ volatile						\
	(								\
		"leaq	%[_atom],	%%rdx"		"\n\t"		\
		"movq	%[_seed],	%%rax"		"\n\t"		\
		"movq	%%rax	,	(%%rdx)" 			\
		:							\
		: [_atom]	"m"	(atom) ,			\
		  [_seed]	"i"	(seed)				\
		: "%rax", "%rdx", "memory"				\
	);								\
})

#define BIT_ATOM_TRYLOCK(_lock, atom, seed)				\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"movq	%[_seed],	%%rdx"		"\n\t"		\
		"movl	%%edx	,	%%eax"		"\n\t"		\
		"movq	$0xffffffff,	%%rbx"		"\n\t"		\
		"andq	%%rbx	,	%%rax"		"\n\t"		\
		"shrq	$32	,	%%rdx"		"\n\t"		\
		"xorq	%%rcx	,	%%rcx"		"\n\t"		\
		"xorq	%%rbx	,	%%rbx"		"\n\t"		\
		_lock	"cmpxchg8b	%[_atom]"	"\n\t"		\
		"setz	%[_ret]"					\
		: [_ret]	"+m"	(_ret)				\
		: [_atom]	"m"	(atom) ,			\
		  [_seed]	"i"	(seed)				\
		: "%rax", "%rbx", "%rcx", "%rdx", "cc", "memory"	\
	);								\
	_ret;								\
})

#define BIT_ATOM_UNLOCK(_lock, atom, seed)				\
({									\
	volatile unsigned long long tries;				\
									\
	__asm__ volatile						\
	(								\
		"movq	%[count],	%%r12"		"\n\t"		\
	"1:"						"\n\t"		\
		"subq	$1	,	%%r12"		"\n\t"		\
		"jz	2f"				"\n\t"		\
		"movq	%[_seed],	%%rcx"		"\n\t"		\
		"movl	%%ecx	,	%%ebx"		"\n\t"		\
		"movq	$0xffffffff,	%%rax"		"\n\t"		\
		"andq	%%rax	,	%%rbx"		"\n\t"		\
		"shrq	$32	,	%%rcx"		"\n\t"		\
		"xorq	%%rdx	,	%%rdx"		"\n\t"		\
		"xorq	%%rax	,	%%rax"		"\n\t"		\
		_lock	"cmpxchg8b	%[_atom]"	"\n\t"		\
		"jnz	1b"				"\n\t"		\
	"2:"						"\n\t"		\
		"movq	%%r12	,	%[tries]"			\
		: [tries]	"+m"	(tries) 			\
		: [_atom]	"m"	(atom) ,			\
		  [_seed]	"i"	(seed) ,			\
		  [count]	"i"	(BIT_IO_RETRIES_COUNT)		\
		: "%rax", "%rbx", "%rcx", "%rdx", "%r12", "cc", "memory"\
	);								\
	tries;								\
})

#define BITWISESET(_lock, _opl, _opr)					\
({									\
	__asm__ volatile						\
	(								\
	_lock	"orq	%[opr], %[dest]"				\
		: [dest] "=m" (_opl)					\
		: [opr]  "Jr" (_opr)					\
		: "cc", "memory"					\
	);								\
})

#define BITWISECLR(_lock, _dest)					\
({									\
	__asm__ volatile						\
	(								\
	_lock	"andq $0, %[dest]"					\
		: [dest] "=m" (_dest)					\
		:							\
		: "cc", "memory"					\
	);								\
})

#if (CORE_COUNT == 64)
#define BITSET_CC(_lock, _base, _offset) BITSET(_lock, _base, _offset)
#else
#define BITSET_CC(_lock, _base, _offset)				\
(									\
	BITSET(_lock,	_base[ CORE_WORD_TOP(CORE_COUNT)		\
				- CORE_WORD_POS(CORE_COUNT, _offset) ] ,\
			CORE_WORD_MOD(CORE_COUNT, _offset) )		\
)
#endif

#if (CORE_COUNT == 64)
#define BITCLR_CC(_lock, _base, _offset) BITCLR(_lock, _base, _offset)
#else
#define BITCLR_CC(_lock, _base, _offset)				\
(									\
	BITCLR(_lock,	_base[ CORE_WORD_TOP(CORE_COUNT)		\
				- CORE_WORD_POS(CORE_COUNT, _offset) ] ,\
			CORE_WORD_MOD(CORE_COUNT, _offset) )		\
)
#endif

#if (CORE_COUNT == 64)
#define BITVAL_CC(_base, _offset) BITVAL(_base, _offset)
#else
#define BITVAL_CC(_base, _offset)					\
(									\
	BITVAL(_base[CORE_WORD_TOP(CORE_COUNT)				\
		- CORE_WORD_POS(CORE_COUNT, _offset)],			\
		CORE_WORD_MOD(CORE_COUNT, _offset) )			\
)
#endif

#if (CORE_COUNT == 64)
#define BITWISEAND_CC(_lock, _opl, _opr) BITWISEAND(_lock, _opl, _opr)
#else
#define BITWISEAND_CC(_lock, _opl, _opr)				\
({									\
	volatile Bit64 _ret __attribute__ ((aligned (8))) = 0;		\
	unsigned int cw = 0;						\
	do {								\
		_ret |= _BITWISEAND(_lock, _opl[cw], _opr[cw]) ;	\
	} while (++cw <= CORE_WORD_TOP(CORE_COUNT));			\
	_ret;								\
})
#endif

#if (CORE_COUNT == 64)
#define BITSTOR_CC(_lock, _dest, _src) BITSTOR(_lock, _dest, _src)
#else
#define BITSTOR_CC(_lock, _dest, _src)					\
({									\
	unsigned int cw = 0;						\
	do {								\
		BITSTOR(_lock, _dest[cw], _src[cw]);			\
	} while (++cw <= CORE_WORD_TOP(CORE_COUNT));			\
})
#endif

#define ASM_CMPXCHG16B(_lock, _ret, _reg0, _reg1, _off0, _off1)		\
	"movq	" #_off0 "(%%" #_reg0 "),	%%rax"	"\n\t"		\
	"movq	" #_off1 "(%%" #_reg0 "),	%%rdx"	"\n\t"		\
	"movq	" #_off0 "(%%" #_reg1 "),	%%rbx"	"\n\t"		\
	"movq	" #_off1 "(%%" #_reg1 "),	%%rcx"	"\n\t"		\
_lock	"cmpxchg16b " #_off0 "(%%" #_reg1 ")"		"\n\t"		\
	"setz	" #_ret

#if defined(LEGACY) && (LEGACY > 0)
FEAT_MSG("LEGACY Level 1: BITCMP_CC() built without asm cmpxchg16b")

#if (CORE_COUNT == 64)
#error "LEGACY Level 1: Unimplemented BITCMP_CC() and CORE_COUNT(64)"
#else
#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	unsigned char ret = 1;						\
	unsigned int cw = 0;						\
	do {								\
		Bit64 _tmp __attribute__ ((aligned (8))) = _opl[cw];	\
		volatile unsigned char _ret;				\
									\
		__asm__ volatile					\
		(							\
			"movq	%[opr]	,	%%rbx"	"\n\t"		\
		_lock	"xorq	%%rbx	,	%[tmp]"	"\n\t"		\
			"setz	%[ret]" 				\
			: [ ret]	"+m" (_ret)			\
			: [ tmp]	"m" (_tmp),			\
			  [ opr]	"m" (_opr[cw])			\
			: "cc", "memory", "%rbx"			\
		);							\
		ret &= _ret;						\
	} while (++cw <= CORE_WORD_TOP(CORE_COUNT));			\
	ret;								\
})
#endif
/*	---	---	---	cmpxchg16b	---	---	---	*/
#elif (CORE_COUNT == 64)

#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	Bit64 _tmp __attribute__ ((aligned (8))) = _opl;		\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"movq	%[opr]	,	%%rbx"	"\n\t"			\
	_lock	"xorq	%%rbx	,	%[tmp]"	"\n\t"			\
		"setz	%[ret]" 					\
		: [ ret]	"+m" (_ret)				\
		: [ tmp]	"m" (_tmp),				\
		  [ opr]	"m" (_opr)				\
		: "cc", "memory", "%rbx"				\
	);								\
	_ret;								\
})

#elif (CORE_COUNT == 128)

#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"leaq	%[opr]	,	%%r12"		"\n\t"		\
		"leaq	%[opl]	,	%%r13"		"\n\t"		\
		ASM_CMPXCHG16B(_lock, %[ret], r12, r13, 0, 8)		\
		: [ ret]	"+m"	(_ret),				\
		  [ opl]	"=m"	(_opl)				\
		: [ opr]	"m"	(_opr)				\
		: "cc", "memory",					\
		  "%rax", "%rbx", "%rcx", "%rdx",			\
		  "%r12", "r13"						\
	);								\
	_ret;								\
})

#elif (CORE_COUNT == 256)

#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"movq	$0x1	,	%%rsi"			"\n\t"	\
		"xorq	%%rdi	,	%%rdi"			"\n\t"	\
		"leaq	%[opr]	,	%%r12"			"\n\t"	\
		"leaq	%[opl]	,	%%r13"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13, 16, 24)	"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13,  0,  8)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		"mov	%%dil	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret),				\
		  [ opl]	"=m"	(_opl)				\
		: [ opr]	"m"	(_opr)				\
		: "cc", "memory",					\
		  "%rax", "%rbx", "%rcx", "%rdx",			\
		  "%r12", "%r13", "%rdi", "%rsi"			\
	);								\
	_ret;								\
})

#elif (CORE_COUNT == 512)

#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"movq	$0x1	,	%%rsi"			"\n\t"	\
		"xorq	%%rdi	,	%%rdi"			"\n\t"	\
		"leaq	%[opr]	,	%%r12"			"\n\t"	\
		"leaq	%[opl]	,	%%r13"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13, 48, 56)	"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13, 32, 40)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13, 16, 24)	"\n\t"	\
		"andq	%%rdi	,	%%rsi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13,  0,  8)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		"mov	%%dil	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret),				\
		  [ opl]	"=m"	(_opl)				\
		: [ opr]	"m"	(_opr)				\
		: "cc", "memory",					\
		  "%rax", "%rbx", "%rcx", "%rdx",			\
		  "%r12", "%r13", "%rdi", "%rsi"			\
	);								\
	_ret;								\
})

#elif (CORE_COUNT == 1024)

#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"movq	$0x1	,	%%rsi"			"\n\t"	\
		"xorq	%%rdi	,	%%rdi"			"\n\t"	\
		"leaq	%[opr]	,	%%r12"			"\n\t"	\
		"leaq	%[opl]	,	%%r13"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13,112,120)	"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13, 96,104)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13, 80, 88)	"\n\t"	\
		"andq	%%rdi	,	%%rsi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13, 64, 72)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13, 48, 56)	"\n\t"	\
		"andq	%%rdi	,	%%rsi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13, 32, 40)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%sil, r12, r13, 16, 24)	"\n\t"	\
		"andq	%%rdi	,	%%rsi"			"\n\t"	\
		ASM_CMPXCHG16B(_lock, %%dil, r12, r13,  0,  8)	"\n\t"	\
		"andq	%%rsi	,	%%rdi"			"\n\t"	\
		"mov	%%dil	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret),				\
		  [ opl]	"=m"	(_opl)				\
		: [ opr]	"m"	(_opr)				\
		: "cc", "memory",					\
		  "%rax", "%rbx", "%rcx", "%rdx",			\
		  "%r12", "%r13", "%rdi", "%rsi"			\
	);								\
	_ret;								\
})

#endif /* LEGACY */

/* Micro-benchmark. Prerequisites: CPU affinity, RDTSC[P] optionnaly RDPMC */
#if defined(UBENCH) && UBENCH == 1

#define UBENCH_DECLARE()						\
static unsigned long long uBenchCounter[2][4] __attribute__((aligned(8)))=\
{									\
/* TSC*/{0,		0,		0,		0},		\
/*INST*/{0,		0,		0,		0}		\
/*	[0]DELTA,	[1]PREV,	[2]LAST,	[3]OVERHEAD  */ \
};									\
									\
inline static void UBENCH_RDCOUNTER_VOID(unsigned int idx) {}		\
									\
inline static void UBENCH_With_RDTSCP_No_RDPMC(unsigned int idx)	\
{									\
	RDTSCP64(uBenchCounter[0][idx]);				\
}									\
									\
inline static void UBENCH_With_RDTSC_No_RDPMC(unsigned int idx) 	\
{									\
	RDTSC64(uBenchCounter[0][idx]) ;				\
}									\
									\
inline static void UBENCH_With_RDTSCP_RDPMC(unsigned int idx)		\
{									\
	RDTSCP_PMCx1(	uBenchCounter[0][idx],				\
			0x40000000,					\
			uBenchCounter[1][idx]) ;			\
}									\
									\
									\
inline static void UBENCH_With_RDTSC_RDPMC(unsigned int idx)		\
{									\
	RDTSC_PMCx1(	uBenchCounter[0][idx],				\
			0x40000000,					\
			uBenchCounter[1][idx]) ;			\
}									\
									\
static void (*UBENCH_RDCOUNTER)(unsigned int) = UBENCH_RDCOUNTER_VOID;

#define UBENCH_COMPUTE()						\
({									\
	uBenchCounter[0][0] = uBenchCounter[0][2] - uBenchCounter[0][1];\
	uBenchCounter[1][0] = uBenchCounter[1][2] - uBenchCounter[1][1];\
	uBenchCounter[0][0] -= uBenchCounter[0][3];			\
	uBenchCounter[1][0] -= uBenchCounter[1][3];			\
})

#define UBENCH_METRIC(metric) (uBenchCounter[metric][0])

#define UBENCH_SETUP(withRDTSCP , withRDPMC)				\
({									\
	void (*MatrixCallFunc[2][2])(unsigned int) = {			\
		{UBENCH_With_RDTSC_No_RDPMC, UBENCH_With_RDTSC_RDPMC},	\
		{UBENCH_With_RDTSCP_No_RDPMC,UBENCH_With_RDTSCP_RDPMC}	\
	};								\
	UBENCH_RDCOUNTER = MatrixCallFunc[withRDTSCP][withRDPMC];	\
									\
	UBENCH_RDCOUNTER(0);						\
	UBENCH_RDCOUNTER(3);						\
									\
	uBenchCounter[0][0] = uBenchCounter[0][3]- uBenchCounter[0][0]; \
	uBenchCounter[1][0] = uBenchCounter[1][3]- uBenchCounter[1][0]; \
	uBenchCounter[0][3] = uBenchCounter[0][0];			\
	uBenchCounter[1][3] = uBenchCounter[1][0];			\
})

#else /* UBENCH == 1 */

#define UBENCH_DECLARE()			/* UBENCH_DECLARE()	*/
#define UBENCH_RDCOUNTER(idx)			/* UBENCH_RDCOUNTER()	*/
#define UBENCH_COMPUTE()			/* UBENCH_COMPUTE()	*/
#define UBENCH_METRIC(metric)			/* UBENCH_METRIC()	*/
#define UBENCH_SETUP(withTSCP , withPMC)	/* UBENCH_SETUP()	*/

#endif /* UBENCH == 1 */
