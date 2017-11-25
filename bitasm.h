/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef unsigned long long int	Bit64;
typedef unsigned int		Bit32;

#define LOCKLESS " "
#define BUS_LOCK "lock "

#define _BITSET_GPR(_lock, _base, _offset)	\
({						\
	asm volatile				\
	(					\
	_lock	"btsq	%%rdx, %[base]"		\
		: [base] "=m" (_base)		\
		: "d" (_offset)			\
		: "cc", "memory"		\
	);					\
})

#define _BITSET_IMM(_lock, _base, _imm8)	\
({						\
	asm volatile				\
	(					\
	_lock	"btsq	%[imm8], %[base]"	\
		: [base] "=m" (_base)		\
		: [imm8] "i" (_imm8)		\
		: "cc", "memory"		\
	);					\
})

#define _BITCLR_GPR(_lock, _base, _offset)	\
({						\
	asm volatile				\
	(					\
	_lock	"btrq	%%rdx,	%[base]"	\
		: [base] "=m" (_base)		\
		: "d" (_offset)			\
		: "cc", "memory"		\
	);					\
})

#define _BITCLR_IMM(_lock, _base, _imm8)	\
({						\
	asm volatile				\
	(					\
	_lock	"btrq	%[imm8], %[base]"	\
		: [base] "=m" (_base)		\
		: [imm8] "i" (_imm8)		\
		: "cc", "memory"		\
	);					\
})

#define _BITBTC_GPR(_lock,_base, _offset)	\
({						\
	asm volatile				\
	(					\
	_lock	"btcq	%%rdx,	%[base]"	\
		: [base] "=m" (_base)		\
		: "d" (_offset)			\
		: "cc", "memory"		\
	);					\
})

#define _BITBTC_IMM(_lock, _base, _imm8)	\
({						\
	asm volatile				\
	(					\
	_lock	"btcq	%[imm8], %[base]"	\
		: [base] "=m" (_base)		\
		: [imm8] "i" (_imm8)		\
		: "cc", "memory"		\
	);					\
})

#define _BIT_TEST_GPR(_base, _offset)	\
({						\
	register unsigned char _ret = 0;	\
	asm volatile				\
	(					\
		"btq	%%rdx, %[base]"	"\n\t"	\
		"setc	%[ret]"			\
		: [ret]	"+r" (_ret)		\
		: [base] "m" (_base),		\
		  "d" (_offset)			\
		: "cc", "memory"		\
	);					\
	_ret;					\
})

#define _BIT_TEST_IMM(_base, _imm8)		\
({						\
	register unsigned char _ret = 0;	\
	asm volatile				\
	(					\
		"btq	%[imm8], %[base]""\n\t" \
		"setc	%[ret]"			\
		: [ret]	"+r" (_ret)		\
		: [base] "m" (_base),		\
		  [imm8] "i" (_imm8)		\
		: "cc", "memory"		\
	);					\
	_ret;					\
})

#define _BITWISEAND(_lock, _opl, _opr)				\
({								\
	volatile Bit64 _ret __attribute__ ((aligned (64)))=_opl;\
	asm volatile						\
	(							\
	_lock	"andq %[opr], %[ret]"				\
		: [ret] "=m" (_ret)				\
		: [opr] "Jr" (_opr)				\
		: "memory"					\
	);							\
	_ret;							\
})

#define _BITWISEOR(_lock, _opl, _opr)				\
({								\
	volatile Bit64 _ret __attribute__ ((aligned (64)))=_opl;\
	asm volatile						\
	(							\
	_lock	"orq %[opr], %[ret]"				\
		: [ret] "=m" (_ret)				\
		: [opr] "Jr" (_opr)				\
		: "memory"					\
	);							\
	_ret;							\
})

#define _BITWISEXOR(_lock, _opl, _opr)				\
({								\
	volatile Bit64 _ret __attribute__ ((aligned (64)))=_opl;\
	asm volatile						\
	(							\
	_lock	"xorq %[opr], %[ret]"				\
		: [ret] "=m" (_ret)				\
		: [opr] "Jr" (_opr)				\
		: "memory"					\
	);							\
	_ret;							\
})

#define BITMSK(_lock, _base, _offset)				\
({								\
	asm volatile						\
	(							\
	_lock	"orq	$0xffffffffffffffff, %[base]"	"\n\t"	\
	_lock	"btc	%[offset], %[base]"			\
		: [base] "=m" (_base)				\
		: [offset] "Jr" (_offset)			\
		: "cc", "memory"				\
	);							\
})

#define BITSET(_lock, _base, _offset)			\
(							\
	__builtin_constant_p(_offset) ?			\
		_BITSET_IMM(_lock, _base, _offset)	\
	:	_BITSET_GPR(_lock, _base, _offset)	\
)

#define BITCLR(_lock, _base, _offset)			\
(							\
	__builtin_constant_p(_offset) ?			\
		_BITCLR_IMM(_lock, _base, _offset)	\
	:	_BITCLR_GPR(_lock, _base, _offset)	\
)

#define BITBTC(_lock, _base, _offset)			\
(							\
	__builtin_constant_p(_offset) ?			\
		_BITBTC_IMM(_lock, _base, _offset)	\
	:	_BITBTC_GPR(_lock, _base, _offset)	\
)

#define BITVAL(_base, _offset)				\
(							\
	__builtin_constant_p(_offset) ?			\
		_BIT_TEST_IMM(_base, _offset)		\
	:	_BIT_TEST_GPR(_base, _offset)		\
)

#define BITCPL(_src)					\
({							\
	unsigned long long _dest;			\
	asm volatile					\
	(						\
		"mov	%[src], %[dest]"	"\n\t"	\
		"negq	%[dest]"			\
		: [dest] "=m" (_dest)			\
		: [src] "ir" (_src)			\
		: "memory"				\
	);						\
	_dest;						\
})

#define BITWISEAND(_lock, _opl, _opr)	_BITWISEAND(_lock, _opl, _opr)
#define BITWISEOR(_lock, _opl, _opr)	_BITWISEOR(_lock, _opl, _opr)
#define BITWISEXOR(_lock, _opl, _opr)	_BITWISEXOR(_lock, _opl, _opr)

#define BITSTOR(_lock, _dest, _src)			\
	_dest = BITWISEAND(_lock, _src, 0xffffffffffffffff)
