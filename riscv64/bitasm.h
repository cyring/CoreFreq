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

#define LOCKLESS LOCK_LESS
#define BUS_LOCK FULL_LOCK

#define BARRIER(_option)						\
__asm__ volatile							\
(									\
	"fence iorw	, iorw"			"\n\t"			\
	"fence.i"							\
	:								\
	:								\
	: "memory"							\
)

#define WBINVD()							\
__asm__ volatile							\
(									\
	"fence.i"							\
	:								\
	:								\
	: "memory"							\
)

#define SERIALIZE()							\
__asm__ volatile							\
(									\
	"fence.i"							\
	:								\
	:								\
	: "memory"							\
)

#define RDTSC64(_mem64) 						\
__asm__ volatile							\
(									\
	"rdtime %0"							\
	: "=r" (_mem64) 						\
	:								\
	: "cc", "memory"						\
)

#define RDINST64(_mem64)						\
__asm__ volatile							\
(									\
	"rdinstret %0"							\
	: "=r" (_mem64) 						\
	:								\
	: "cc", "memory"						\
)

#define RDPMC64(_mem64) 						\
__asm__ volatile							\
(									\
	"rdcycle %0"							\
	: "=r" (_mem64) 						\
	:								\
	: "cc", "memory"						\
)

#define ASM_RDTSC(_reg) 						\
	"# Read variant TSC."			"\n\t"			\
	"rdtime " #_reg 			"\n\t"

#define ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Read PMU counter."			"\n\t"			\
	_ctr"	" #_reg 			"\n\t"

#define ASM_RDPMC(_ctr, _reg) ASM_CODE_RDPMC(_ctr, _reg)

#define RDPMC(_ctr, _reg, _mem) 					\
__asm__ volatile							\
(									\
	ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Store values into memory."		"\n\t"			\
	"sd	" #_reg ",	%0"					\
	: "=m" (_mem)							\
	:								\
	: "%" #_reg"",							\
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
	"sd	" #_reg0 ",	%0"		"\n\t"			\
	"sd	" #_reg1 ",	%1"					\
	: "=m" (mem_tsc),	"=m" (_mem1)				\
	:								\
	: "%" #_reg0""	,	"%" #_reg1"",				\
	  "cc", "memory"						\
)

#define RDTSC_PMCx1(mem_tsc, ...)					\
ASM_RDTSC_PMCx1(x14, x15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#define	_BITSET_PRE_INST_FULL_LOCK					\
	"1:"					"\n\t"			\
		"lr.d	x31, [%[addr]]" 	"\n\t"

#define	_BITSET_PRE_INST_LOCK_LESS					\
		"lr.d	x31, [%[addr]]" 	"\n\t"

#define	_BITSET_COMMON_INST						\
/*		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"strb	w10, %[ret]"		"\n\t"			\
		"orr	x11, x11, x12"		"\n\t"*/

#define	_BITSET_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define	_BITSET_POST_INST_LOCK_LESS					\
/*		"str	x11, [%[addr]]"*/

#define	_BITSET_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%w10", "%x11", "%x12"

#define	_BITSET_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%w10", "%x11", "%x12"

#define _BITSET_GPR(_lock, _base, _offset)				\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[offset]"	"\n\t"			\
		_BITSET_PRE_INST_##_lock				\
		_BITSET_COMMON_INST					\
		_BITSET_POST_INST_##_lock				\
		: [ret] "=m" (_ret)					\
		: [addr] "r" (&_base),					\
		  [offset] "r" (_offset)				\
		_BITSET_CLOBBERS_##_lock				\
	);*/								\
	const __typeof__(_base) _shl = 1LLU << _offset; 		\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) | (_shl);					\
	_ret;								\
})

#define _BITSET_IMM(_lock, _base, _imm6)				\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[imm6]"	"\n\t"			\
		_BITSET_PRE_INST_##_lock				\
		_BITSET_COMMON_INST					\
		_BITSET_POST_INST_##_lock				\
		: [ret] "=m" (_ret)					\
		: [addr] "r" (&_base),					\
		  [imm6] "i" (_imm6)					\
		_BITSET_CLOBBERS_##_lock				\
	);*/								\
	const __typeof__(_base) _shl = 1LLU << _imm6;			\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) | (_shl);					\
	_ret;								\
})

#define _BITCLR_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	x11, [%[addr]]" 	"\n\t"*/

#define _BITCLR_PRE_INST_LOCK_LESS					\
/*		"ldr	x11, [%[addr]]" 	"\n\t"*/

#define _BITCLR_COMMON_INST						\
/*		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"strb	w10, %[ret]"		"\n\t"			\
		"bic	x11, x11, x12"		"\n\t"*/

#define _BITCLR_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITCLR_POST_INST_LOCK_LESS					\
/*		"str	x11, [%[addr]]"*/

#define _BITCLR_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%w10", "%x11", "%x12"

#define _BITCLR_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%w10", "%x11", "%x12"

#define _BITCLR_GPR(_lock, _base, _offset)				\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[offset]"	"\n\t"			\
		_BITCLR_PRE_INST_##_lock				\
		_BITCLR_COMMON_INST					\
		_BITCLR_POST_INST_##_lock				\
		: [ret] "=m" (_ret)					\
		: [addr] "r" (&_base),					\
		  [offset] "r" (_offset)				\
		_BITCLR_CLOBBERS_##_lock				\
	);*/								\
	const __typeof__(_base) _shl = 1LLU << _offset; 		\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) & ~(_shl);					\
	_ret;								\
})

#define _BITCLR_IMM(_lock, _base, _imm6)				\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[imm6]"	"\n\t"			\
		_BITCLR_PRE_INST_##_lock				\
		_BITCLR_COMMON_INST					\
		_BITCLR_POST_INST_##_lock				\
		: [ret] "=m" (_ret)					\
		: [addr] "r" (&_base),					\
		  [imm6] "i" (_imm6)					\
		_BITCLR_CLOBBERS_##_lock				\
	);*/								\
	const __typeof__(_base) _shl = 1LLU << _imm6;			\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) & ~(_shl);					\
	_ret;								\
})

#define _BIT_TEST_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	x11, [%[addr]]" 	"\n\t"*/

#define _BIT_TEST_PRE_INST_LOCK_LESS					\
/*		"ldr	x11, [%[addr]]" 	"\n\t"*/

#define _BIT_TEST_COMMON_INST						\
/*		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"strb	w10, %[ret]"		"\n\t"*/

#define _BIT_TEST_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BIT_TEST_POST_INST_LOCK_LESS					\
		"#	NOP"

#define _BIT_TEST_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%w10", "%x11", "%x12"

#define _BIT_TEST_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%w10", "%x11", "%x12"

#define _BIT_TEST_GPR(_lock, _base, _offset)				\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[offset]"	"\n\t"			\
		_BIT_TEST_PRE_INST_##_lock				\
		_BIT_TEST_COMMON_INST					\
		_BIT_TEST_POST_INST_##_lock				\
		: [ret] "=m" (_ret)					\
		: [addr] "r" (&_base),					\
		  [offset] "r" (_offset)				\
		_BIT_TEST_CLOBBERS_##_lock				\
	);*/								\
	const unsigned char _ret = ((_base) & (1LLU << _offset)) != 0;	\
	_ret;								\
})

#define _BIT_TEST_IMM(_lock, _base, _imm6)				\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[imm6]"	"\n\t"			\
		_BIT_TEST_PRE_INST_##_lock				\
		_BIT_TEST_COMMON_INST					\
		_BIT_TEST_POST_INST_##_lock				\
		: [ret] "=m" (_ret)					\
		: [addr] "r" (&_base),					\
		  [imm6] "i" (_imm6)					\
		_BIT_TEST_CLOBBERS_##_lock				\
	);*/								\
	const unsigned char _ret = ((_base) & (1LLU << _imm6)) != 0;	\
	_ret;								\
})

#define _BITWISEAND_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISEAND_PRE_INST_LOCK_LESS					\
/*		"ldr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISEAND_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITWISEAND_POST_INST_LOCK_LESS 				\
		"#	NOP"

#define _BITWISEAND_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%x10", "%x11"

#define _BITWISEAND_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%x10", "%x11"

#define _BITWISEAND(_lock, _opl, _opr)					\
({									\
/*	volatile Bit64 _dest __attribute__ ((aligned (8)));		\
									\
	__asm__ volatile						\
	(								\
		_BITWISEAND_PRE_INST_##_lock				\
		"and	x10, x11, %[opr]"	"\n\t"			\
		"str	x10, %[dest]"		"\n\t"			\
		 _BITWISEAND_POST_INST_##_lock				\
		: [dest] "=m" (_dest)					\
		: [addr] "r" (&_opl),					\
		  [opr]  "Lr" (_opr)					\
		_BITWISEAND_CLOBBERS_##_lock				\
	);*/								\
	const Bit64 _dest __attribute__ ((aligned (8)))=(_opl) & (_opr);\
	_dest;								\
})

#define _BITWISEOR_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISEOR_PRE_INST_LOCK_LESS					\
/*		"ldr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISEOR_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITWISEOR_POST_INST_LOCK_LESS					\
		"#	NOP"

#define _BITWISEOR_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%x10", "%x11"

#define _BITWISEOR_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%x10", "%x11"

#define _BITWISEOR(_lock, _opl, _opr)					\
({									\
/*	volatile Bit64 _dest __attribute__ ((aligned (8)));		\
									\
	__asm__ volatile						\
	(								\
		_BITWISEOR_PRE_INST_##_lock				\
		"orr	x10, x11, %[opr]"	"\n\t"			\
		"str	x10, %[dest]"		"\n\t"			\
		_BITWISEOR_POST_INST_##_lock				\
		: [dest] "=m" (_dest)					\
		: [addr] "r" (&_opl),					\
		  [opr]  "Lr" (_opr)					\
		_BITWISEOR_CLOBBERS_##_lock				\
	);*/								\
	const Bit64 _dest __attribute__ ((aligned (8)))=(_opl) | (_opr);\
	_dest;								\
})

#define _BITWISEXOR_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISEXOR_PRE_INST_LOCK_LESS					\
/*		"ldr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISEXOR_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITWISEXOR_POST_INST_LOCK_LESS					\
		"#	NOP"

#define _BITWISEXOR_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%x10", "%x11"

#define _BITWISEXOR_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%x10", "%x11"

#define _BITWISEXOR(_lock, _opl, _opr)					\
({									\
/*	volatile Bit64 _dest __attribute__ ((aligned (8)));		\
									\
	__asm__ volatile						\
	(								\
		_BITWISEXOR_PRE_INST_##_lock				\
		"eor	x10, x11, %[opr]"	"\n\t"			\
		"str	x10, %[dest]"		"\n\t"			\
		_BITWISEXOR_POST_INST_##_lock				\
		: [dest] "=m" (_dest)					\
		: [addr] "r" (&_opl),					\
		  [opr]  "Lr" (_opr)					\
		_BITWISEXOR_CLOBBERS_##_lock				\
	);*/								\
	const Bit64 _dest __attribute__ ((aligned (8)))=(_opl) ^ (_opr);\
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

#define BITVAL_3xPARAM(_lock, _base, _offset)				\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BIT_TEST_IMM(_lock, _base, _offset)			\
	:	_BIT_TEST_GPR(_lock, _base, _offset)			\
)

#define BITVAL_2xPARAM(_base, _offset)					\
(									\
	BITVAL_3xPARAM(LOCKLESS, _base, _offset)			\
)

#define BITVAL_DISPATCH( _1, _2, _3, BITVAL_CURSOR, ... )		\
	BITVAL_CURSOR

#define BITVAL(...)							\
	BITVAL_DISPATCH(__VA_ARGS__,	BITVAL_3xPARAM, /*3*/		\
					BITVAL_2xPARAM, /*2*/		\
					NULL)		/*1*/		\
							( __VA_ARGS__ )

#define BITCPL(_src)							\
({									\
	unsigned long long _dest;					\
									\
	__asm__ volatile						\
	(								\
		"neg	%x[dest], %x[src]"				\
		: [dest] "=rQ" (_dest)					\
		: [src] "Lr" (_src)					\
		: "cc", "memory"					\
	);								\
	_dest;								\
})

#define BITWISEAND(_lock, _opl, _opr)	_BITWISEAND(_lock, _opl, _opr)
#define BITWISEOR(_lock, _opl, _opr)	_BITWISEOR(_lock, _opl, _opr)
#define BITWISEXOR(_lock, _opl, _opr)	_BITWISEXOR(_lock, _opl, _opr)

#define _BITSTOR_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	xzr, [%[addr]]" 	"\n\t"*/

#define _BITSTOR_PRE_INST_LOCK_LESS					\
		"#	NOP"			"\n\t"

#define _BITSTOR_POST_INST_FULL_LOCK					\
/*		"stxr	w9, %[src], [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITSTOR_POST_INST_LOCK_LESS					\
/*		"str	%[src], [%[addr]]"*/

#define _BITSTOR_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9"

#define _BITSTOR_CLOBBERS_LOCK_LESS					\
		: "cc", "memory"

#define _BITSTOR(_lock, _dest, _src)					\
({									\
/*	__asm__ volatile						\
	(								\
		_BITSTOR_PRE_INST_##_lock				\
		_BITSTOR_POST_INST_##_lock				\
		:							\
		: [addr] "r" (&_dest),					\
		  [src] "r" (_src)					\
		_BITSTOR_CLOBBERS_##_lock				\
	);*/								\
	_dest = _src;							\
})

#define BITSTOR(_lock, _dest, _src)					\
	_BITSTOR(_lock, _dest, _src)

#define BITBSF(_base, _index)						\
({									\
	register unsigned char _ret;					\
	if (_base) {							\
		_index = 0;						\
		while (BITVAL(_base, _index) == 0) {			\
			if (_index < (8 * sizeof(_base)) - 1) {		\
				_index = _index + 1;			\
			} else {					\
				break;					\
			}						\
		}							\
		_ret = 0;						\
	} else {							\
		_index = 0;						\
		_ret = 1;						\
	}								\
	_ret;								\
})

#define BITBSR(_base, _index)						\
({									\
	register unsigned char _ret;					\
	if (_base) {							\
		_index = (8 * sizeof(_base)) - 1;			\
		while (BITVAL(_base, _index) == 0) {			\
			if (_index > 0) {				\
				_index = _index - 1;			\
			} else {					\
				break;					\
			}						\
		}							\
		_ret = 0;						\
	} else {							\
		_index = 0;						\
		_ret = 1;						\
	}								\
	_ret;								\
})

#define BITEXTRZ(_src, _offset, _length)				\
({									\
	unsigned long long _dest = ((1 << _length) - 1) 		\
				 & (_src >> _offset);			\
	_dest;								\
})

#define _BITWISESET_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISESET_PRE_INST_LOCK_LESS					\
/*		"ldr	x11, [%[addr]]" 	"\n\t"*/

#define _BITWISESET_POST_INST_FULL_LOCK					\
/*		"stxr	w9, x11, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITWISESET_POST_INST_LOCK_LESS					\
/*		"str	x11, [%[addr]]"*/

#define _BITWISESET_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%x11"

#define _BITWISESET_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%x11"

#define _BITWISESET(_lock, _opl, _opr)					\
({									\
/*	__asm__ volatile						\
	(								\
		_BITWISESET_PRE_INST_##_lock				\
		"orr	x11, x11, %[opr]"	"\n\t"			\
		_BITWISESET_POST_INST_##_lock				\
		:							\
		: [addr] "r" (&_opl),					\
		  [opr]  "Lr" (_opr)					\
		_BITWISESET_CLOBBERS_##_lock				\
	);*/								\
	_BITSTOR(_lock, _opl, _opr);					\
})

#define BITWISESET(_lock, _opl, _opr)					\
	_BITWISESET(_lock, _opl, _opr)

#define _BITWISECLR_PRE_INST_FULL_LOCK					\
/*	"1:"					"\n\t"			\
		"ldxr	xzr, [%[addr]]" 	"\n\t"*/

#define _BITWISECLR_PRE_INST_LOCK_LESS					\
		"#	NOP"			"\n\t"

#define _BITWISECLR_POST_INST_FULL_LOCK					\
/*		"stxr	w9, xzr, [%[addr]]"	"\n\t"			\
		"cbnz	w9, 1b" 		"\n\t"			\
		"dmb	ish"*/

#define _BITWISECLR_POST_INST_LOCK_LESS					\
/*		"str	xzr, [%[addr]]"*/

#define _BITWISECLR_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9"

#define _BITWISECLR_CLOBBERS_LOCK_LESS					\
		: "cc", "memory"

#define _BITWISECLR(_lock, _dest)					\
({									\
/*	__asm__ volatile						\
	(								\
		_BITWISECLR_PRE_INST_##_lock				\
		_BITWISECLR_POST_INST_##_lock				\
		:							\
		: [addr] "r" (&_dest)					\
		_BITWISECLR_CLOBBERS_##_lock				\
	);*/								\
	_dest = 0;							\
})

#define BITWISECLR(_lock, _dest)					\
	_BITWISECLR(_lock, _dest)

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
#define BITWISEAND_CC(_lock, _opl, _opr) _BITWISEAND(_lock, _opl, _opr)
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
#define BITSTOR_CC(_lock, _dest, _src) _BITSTOR(_lock, _dest, _src)
#else
#define BITSTOR_CC(_lock, _dest, _src)					\
({									\
	unsigned int cw = 0;						\
	do {								\
		_BITSTOR(_lock, _dest[cw], _src[cw]);			\
	} while (++cw <= CORE_WORD_TOP(CORE_COUNT));			\
})
#endif

#define _BITCMP_PRE_INST_FULL_LOCK					\
/*	"1:"						"\n\t"		\
		"ldxr	x11,	[%[addr]]"		"\n\t"*/

#define _BITCMP_PRE_INST_LOCK_LESS					\
/*		"ldr	x11,	[%[addr]]"		"\n\t"*/

#define _BITCMP_POST_INST_FULL_LOCK					\
/*		"stxr	w9,	x11, [%[addr]]" 	"\n\t"		\
		"cbnz	w9,	1b"			"\n\t"		\
		"dmb	ish"*/

#define _BITCMP_POST_INST_LOCK_LESS					\
		"#	NOP"

#define _BITCMP_CLOBBERS_FULL_LOCK					\
		: "cc", "memory", "%w9", "%w10", "%x11"

#define _BITCMP_CLOBBERS_LOCK_LESS					\
		: "cc", "memory", "%w10", "%x11"

#define _BITCMP(_lock, _opl, _opr)					\
({									\
/*	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		_BITCMP_PRE_INST_##_lock				\
		"cmp	x11,	%[opr]" 		"\n\t"		\
		"cset	w10,	eq"			"\n\t"		\
		"strb	w10,	%[ret]" 		"\n\t"		\
		_BITCMP_POST_INST_##_lock				\
		: [ret] "=m"	(_ret)					\
		: [addr] "r"	(&_opl), 				\
		  [opr]	"Lr"	(_opr)					\
		_BITCMP_CLOBBERS_##_lock				\
	);*/								\
	const unsigned char _ret = _opl == _opr ? 1 : 0;		\
	_ret;								\
})

#define BITZERO(_lock, _src)						\
	_BITCMP(_lock, _src, 0)

#if (CORE_COUNT == 64)
#define BITCMP_CC(_lock, _opl, _opr) _BITCMP(_lock, _opl, _opr)
#else
#define BITCMP_CC(_lock, _opl, _opr)					\
({									\
	unsigned char ret = 1;						\
	unsigned int cw = 0;						\
	do {								\
		ret &= _BITCMP(_lock, _opl[cw], _opr[cw]);		\
	} while (++cw <= CORE_WORD_TOP(CORE_COUNT));			\
	ret;								\
})
#endif

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
inline static void UBENCH_With_RDTSC_No_RDPMC(unsigned int idx) 	\
{									\
	RDTSC64(uBenchCounter[0][idx]) ;	SERIALIZE();		\
}									\
									\
inline static void UBENCH_With_RDTSCP_RDPMC(unsigned int idx)		\
{									\
	RDTSC_PMCx1(	uBenchCounter[0][idx],				\
			"rdcycle",					\
			uBenchCounter[1][idx]) ;			\
}									\
									\
									\
inline static void UBENCH_With_RDTSC_RDPMC(unsigned int idx)		\
{									\
	RDTSC_PMCx1(	uBenchCounter[0][idx],				\
			"rdcycle",					\
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
		{UBENCH_With_RDTSC_No_RDPMC, UBENCH_With_RDTSCP_RDPMC}	\
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
