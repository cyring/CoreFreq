/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
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

#define LOCKLESS " "
#define BUS_LOCK "lock "

#define BARRIER(pfx)							\
__asm__ volatile							\
(									\
	"dsb"					"\n\t"			\
	"isb"								\
	:								\
	:								\
	: "memory"							\
)

#define WBINVD()							\
__asm__ volatile							\
(									\
	"ic iallu"							\
	:								\
	:								\
	: "memory"							\
)

#define SERIALIZE()							\
__asm__ volatile							\
(									\
	"isb" 								\
	:								\
	:								\
	: "memory"							\
)

#define RDTSC64(_mem64) 						\
__asm__ volatile							\
(									\
	"mrs	%0	,	cntvct_el0"	"\n\t"			\
	"isb"								\
	: "=r" (_mem64) 						\
	:								\
	: "cc", "memory"						\
)

#define ASM_RDTSC(_reg) 						\
	"# Read variant TSC."			"\n\t"			\
	"mrs	" #_reg ",	cntvct_el0"	"\n\t"

#define ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Read PMC counter."			"\n\t"			\
/*TODO	"movq	$" #_ctr ",	%%rcx"		"\n\t"			\
	"rdpmc" 				"\n\t"			\
	"shlq	$32	,	%%rdx"		"\n\t"			\
	"orq	%%rdx	,	%%rax"		"\n\t"		*/	\
	"# Save counter value." 		"\n\t"			\
/*TODO	"movq	%%rax	,	%%" #_reg	"\n\t"		*/

#define ASM_RDPMC(_ctr, _reg) ASM_CODE_RDPMC(_ctr, _reg)

#define RDPMC(_ctr, _reg, _mem) 					\
__asm__ volatile							\
(									\
	ASM_CODE_RDPMC(_ctr, _reg)					\
	"# Store values into memory."		"\n\t"			\
	"str	" #_reg ",	%0"					\
	: "=m" (_mem)							\
	:								\
	: "%" #_reg"", 							\
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
	"str	" #_reg0 ",	%0"		"\n\t"			\
	"str	" #_reg1 ",	%1"					\
	: "=m" (mem_tsc),	"=m" (_mem1)				\
	:								\
	: "%" #_reg0""	,	"%" #_reg1"",				\
	  "cc", "memory"						\
)

#define RDTSC_PMCx1(mem_tsc, ...)					\
ASM_RDTSC_PMCx1(x14, x15, ASM_RDTSC, mem_tsc, __VA_ARGS__)

#if defined(LEGACY) && LEGACY > 0

#define _BITSET_GPR(_lock, _base, _offset)				\
({									\
	const __typeof__(_base) _shl = 1LLU << _offset; 		\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) | (_shl);					\
	_ret;								\
})

#define _BITSET_IMM(_lock, _base, _imm6)				\
({									\
	const __typeof__(_base) _shl = 1LLU << _imm6;			\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) | (_shl);					\
	_ret;								\
})

#define _BITCLR_GPR(_lock, _base, _offset)				\
({									\
	const __typeof__(_base) _shl = 1LLU << _offset; 		\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) & ~(_shl);					\
	_ret;								\
})

#define _BITCLR_IMM(_lock, _base, _imm6)				\
({									\
	const __typeof__(_base) _shl = 1LLU << _imm6;			\
	const unsigned char _ret = ((_base) & (_shl)) != 0;		\
	_base = (_base) & ~(_shl);					\
	_ret;								\
})

#define _BIT_TEST_GPR(_base, _offset)					\
({									\
	const unsigned char _ret = ((_base) & (1LLU << _offset)) != 0;	\
	_ret;								\
})

#define _BIT_TEST_IMM(_base, _imm6)					\
({									\
	const unsigned char _ret = ((_base) & (1LLU << _imm6)) != 0;	\
	_ret;								\
})

#define _BITWISEAND(_lock, _opl, _opr)					\
({									\
	const Bit64 _dest __attribute__ ((aligned (8)))=(_opl) & (_opr);\
	_dest;								\
})

#define _BITWISEOR(_lock, _opl, _opr)					\
({									\
	const Bit64 _dest __attribute__ ((aligned (8)))=(_opl) | (_opr);\
	_dest;								\
})

#define _BITWISEXOR(_lock, _opl, _opr)					\
({									\
	const Bit64 _dest __attribute__ ((aligned (8)))=(_opl) ^ (_opr);\
	_dest;								\
})

#else /* LEGACY */

#define _BITSET_GPR(_lock, _base, _offset)				\
({									\
	const __typeof__(_base) *_adr = &_base; 			\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[offset]"	"\n\t"			\
		"ldr	x13, %[base]"		"\n\t"			\
		"ldr	x11, [x13]"		"\n\t"			\
		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"orr	x11, x11, x12"		"\n\t"			\
		"str	x11, [x13]"		"\n\t"			\
		"strb	w10, %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [base] "m" (_adr),					\
		  [offset] "r" (_offset)				\
		: "cc", "memory", "%w10", "%x11", "%x12", "%x13"	\
	);								\
	_ret;								\
})

#define _BITSET_IMM(_lock, _base, _imm6)				\
({									\
	const __typeof__(_base) *_adr = &_base; 			\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[imm6]"	"\n\t"			\
		"ldr	x13, %[base]"		"\n\t"			\
		"ldr	x11, [x13]"		"\n\t"			\
		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"orr	x11, x11, x12"		"\n\t"			\
		"str	x11, [x13]"		"\n\t"			\
		"strb	w10, %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [base] "m" (_adr),					\
		  [imm6] "i" (_imm6)					\
		: "cc", "memory", "%w10", "%x11", "%x12", "%x13"	\
	);								\
	_ret;								\
})

#define _BITCLR_GPR(_lock, _base, _offset)				\
({									\
	const __typeof__(_base) *_adr = &_base; 			\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[offset]"	"\n\t"			\
		"ldr	x13, %[base]"		"\n\t"			\
		"ldr	x11, [x13]"		"\n\t"			\
		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"bic	x11, x11, x12"		"\n\t"			\
		"str	x11, [x13]"		"\n\t"			\
		"strb	w10, %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [base] "m" (_adr),					\
		  [offset] "r" (_offset)				\
		: "cc", "memory", "%w10", "%x11", "%x12", "%x13"	\
	);								\
	_ret;								\
})

#define _BITCLR_IMM(_lock, _base, _imm6)				\
({									\
	const __typeof__(_base) *_adr = &_base; 			\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[imm6]"	"\n\t"			\
		"ldr	x13, %[base]"		"\n\t"			\
		"ldr	x11, [x13]"		"\n\t"			\
		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"bic	x11, x11, x12"		"\n\t"			\
		"str	x11, [x13]"		"\n\t"			\
		"strb	w10, %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [base] "m" (_adr),					\
		  [imm6] "i" (_imm6)					\
		: "cc", "memory", "%w10", "%x11", "%x12", "%x13"	\
	);								\
	_ret;								\
})

#define _BIT_TEST_GPR(_base, _offset)					\
({									\
	const __typeof__(_base) *_adr = &_base; 			\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[offset]"	"\n\t"			\
		"ldr	x13, %[base]"		"\n\t"			\
		"ldr	x11, [x13]"		"\n\t"			\
		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"strb	w10, %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [base] "m" (_adr),					\
		  [offset] "r" (_offset)				\
		: "cc", "memory", "%w10", "%x11", "%x12", "%x13"	\
	);								\
	_ret;								\
})

#define _BIT_TEST_IMM(_base, _imm6)					\
({									\
	const __typeof__(_base) *_adr = &_base; 			\
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"mov	x12, #1"		"\n\t"			\
		"lsl	x12, x12, %[imm6]"	"\n\t"			\
		"ldr	x13, %[base]"		"\n\t"			\
		"ldr	x11, [x13]"		"\n\t"			\
		"tst	x11, x12"		"\n\t"			\
		"cset	w10, ne"		"\n\t"			\
		"strb	w10, %[ret]"					\
		: [ret] "=m" (_ret)					\
		: [base] "m" (_adr),					\
		  [imm6] "i" (_imm6)					\
		: "cc", "memory", "%w10", "%x11", "%x12", "%x13"	\
	);								\
	_ret;								\
})

#define _BITWISEAND(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _dest __attribute__ ((aligned (8)));		\
									\
	__asm__ volatile						\
	(								\
		"and	x10, %[opl], %[opr]"	"\n\t"			\
		"str	x10, %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [opl]  "Jr" (_opl),					\
		  [opr]  "Jr" (_opr)					\
		: "memory", "%x10"					\
	);								\
	_dest;								\
})

#define _BITWISEOR(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _dest __attribute__ ((aligned (8)));		\
									\
	__asm__ volatile						\
	(								\
		"orr	x10, %[opl], %[opr]"	"\n\t"			\
		"str	x10, %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [opl]  "Jr" (_opl),					\
		  [opr]  "Jr" (_opr)					\
		: "memory", "%x10"					\
	);								\
	_dest;								\
})

#define _BITWISEXOR(_lock, _opl, _opr)					\
({									\
	volatile Bit64 _dest __attribute__ ((aligned (8)));		\
									\
	__asm__ volatile						\
	(								\
		"eor	x10, %[opl], %[opr]"	"\n\t"			\
		"str	x10, %[dest]"					\
		: [dest] "=m" (_dest)					\
		: [opl]  "Jr" (_opl),					\
		  [opr]  "Jr" (_opr)					\
		: "memory", "%x10"					\
	);								\
	_dest;								\
})

#endif /* LEGACY */

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

#define BITVAL_2xPARAM(_base, _offset)					\
(									\
	__builtin_constant_p(_offset) ? 				\
		_BIT_TEST_IMM(_base, _offset)				\
	:	_BIT_TEST_GPR(_base, _offset)				\
)

#define BITVAL(...)	BITVAL_2xPARAM( __VA_ARGS__ )

#define BITCPL(_src)							\
({									\
	unsigned long long _dest;					\
									\
	__asm__ volatile						\
	(								\
		"neg	%x[dest], %x[src]"				\
		: [dest] "=rQ" (_dest)					\
		: [src] "Jr" (_src)					\
		: "cc", "memory"					\
	);								\
	_dest;								\
})

#define BITWISEAND(_lock, _opl, _opr)	_BITWISEAND(_lock, _opl, _opr)
#define BITWISEOR(_lock, _opl, _opr)	_BITWISEOR(_lock, _opl, _opr)
#define BITWISEXOR(_lock, _opl, _opr)	_BITWISEXOR(_lock, _opl, _opr)

#define BITSTOR(_lock, _dest, _src)					\
({									\
	__asm__ volatile						\
	(								\
		"str	%[src], %[dest]"				\
		: [dest] "=Q" (_dest)					\
		: [src] "r" (_src)					\
		: "cc", "memory"					\
	);								\
})

#define BITZERO(_lock, _src)						\
({									\
	volatile unsigned char _ret = 0;				\
									\
	__asm__ volatile						\
	(								\
		"clz	%x[ret], %x[src]"		"\n\t"		\
		"cmp	%x[ret], #64"			"\n\t"		\
		"cset	%x[ret], eq" 					\
		: [ret] "+r" (_ret)					\
		: [src] "r" (_src)					\
		: "cc", "memory"					\
	);								\
	_ret;								\
})

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

#define BITWISESET(_lock, _opl, _opr)					\
({									\
	__asm__ volatile						\
	(								\
		"ldr	x10,	%[opl]" 		"\n\t"		\
		"orr	x10,	x10, %[opr]"		"\n\t"		\
		"str	x10,	%[opl]" 		"\n\t"		\
		: [opl] "+m"	(_opl)					\
		: [opr] "Jr"	(_opr)					\
		: "memory", "%x10"					\
	);								\
})

#define BITWISECLR(_lock, _dest)					\
({									\
	__asm__ volatile						\
	(								\
		"str	xzr,	%[dest]"				\
		: [dest] "=m"	(_dest) 				\
		:							\
		: "memory"						\
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

#define ASM_CMPXCHG16B( _lock, _ret, _tmp,				\
			_val0, _val1, _reg0, _reg1, _off0, _off1 )	\
	"add " #_tmp "	,	" #_reg0 ",	#" #_off0"\n\t"		\
	"ldr " #_val0 " ,	[" #_tmp "]"		"\n\t"		\
									\
	"add " #_tmp "	,	" #_reg1 ",	#" #_off0"\n\t"		\
	"ldr " #_val1 "	,	[" #_tmp "]"		"\n\t"		\
									\
	"cmp " #_val0 " , 	" #_val1 		"\n\t"		\
	"cset " #_ret "	,	eq" 			"\n\t"		\
									\
	"add " #_tmp "	,	" #_reg0 ",	#" #_off1"\n\t"		\
	"ldr " #_val0 " ,	[" #_tmp "]"		"\n\t"		\
									\
	"add " #_tmp "	,	" #_reg1 ",	#" #_off1"\n\t"		\
	"ldr " #_val1 " ,	[" #_tmp "]"		"\n\t"		\
									\
	"cmp " #_val0 " ,	" #_val1		"\n\t"		\
	"cset " #_tmp " ,	eq" 			"\n\t"		\
									\
	"and " #_ret "	,	" #_ret ",	" #_tmp "\n\t"

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
		volatile unsigned char _ret;				\
									\
		__asm__ volatile					\
		(							\
			"cmp	%[opr]	,	%[opl]" 	"\n\t"	\
			"cset	%[ret]	,	eq"			\
			: [ret]	"=r"	(_ret)				\
			: [opl] "r"	(_opl[cw]),			\
			  [opr] "r"	(_opr[cw])			\
			: "cc", "memory"				\
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
	volatile unsigned char _ret;					\
									\
	__asm__ volatile						\
	(								\
		"ldr	x14	,	%[opr]" 		"\n\t"	\
		"ldr	x15	,	%[opl]" 		"\n\t"	\
		"cmp	x14	,	x15"			"\n\t"	\
		"cset	%[ret]	,	eq"				\
		: [ret]	"=r"	(_ret)					\
		: [opl]	"m"	(_opl), 				\
		  [opr]	"m"	(_opr)					\
		: "cc", "memory", "%x14", "%x15"			\
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
		"mov	x14	,	%[opr]" 		"\n\t"	\
		"mov	x15	,	%[opl]" 		"\n\t"	\
		ASM_CMPXCHG16B(_lock, x12,x11,x9,x10,x14,x15, 0, 8)"\n\t"\
		"str	x12	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret)				\
		: [ opl]	"r"	(_opl), 			\
		  [ opr]	"r"	(_opr)				\
		: "cc", "memory",					\
		  "%x9", "%x10", "%x11", "%x12", "%x14", "%x15"		\
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
		"mov	x14	,	%[opr]" 		"\n\t"	\
		"mov	x15	,	%[opl]" 		"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,16,24)"\n\t"\
		ASM_CMPXCHG16B(_lock, x12,x11,x9,x10,x14,x15, 0, 8)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		"str	x12	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret)				\
		: [ opl]	"r"	(_opl), 			\
		  [ opr]	"r"	(_opr)				\
		: "cc", "memory",					\
		  "%x9", "%x10", "%x11", "%x12", "%x13", "%x14", "%x15" \
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
		"mov	x14	,	%[opr]" 		"\n\t"	\
		"mov	x15	,	%[opl]" 		"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,48,56)"\n\t"\
		ASM_CMPXCHG16B(_lock, x12,x11,x9,x10,x14,x15,32,40)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,16,24)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15, 0, 8)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		"str	x12	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret)				\
		: [ opl]	"r"	(_opl), 			\
		  [ opr]	"r"	(_opr)				\
		: "cc", "memory",					\
		  "%x9", "%x10", "%x11", "%x12", "%x13", "%x14", "%x15" \
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
		"mov	x14	,	%[opr]" 		"\n\t"	\
		"mov	x15	,	%[opl]" 		"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,112,120)"\n\t"\
		ASM_CMPXCHG16B(_lock, x12,x11,x9,x10,x14,x15,96,104) "\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,80,88)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,64,72)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,48,56)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,32,40)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15,16,24)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		ASM_CMPXCHG16B(_lock, x13,x11,x9,x10,x14,x15, 0, 8)"\n\t"\
		"and	x12	,	x12	,	x13"	"\n\t"	\
		"str	x12	,	%[ret]" 			\
		: [ ret]	"+m"	(_ret)				\
		: [ opl]	"r"	(_opl), 			\
		  [ opr]	"r"	(_opr)				\
		: "cc", "memory",					\
		  "%x9", "%x10", "%x11", "%x12", "%x13", "%x14", "%x15" \
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
