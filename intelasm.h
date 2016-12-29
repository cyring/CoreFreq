/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

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

#define _BIT_TEST_GPR(_base, _offset)	\
({						\
	register unsigned char _ret;		\
	asm volatile				\
	(					\
		"xor	%[ret], %[ret]"	"\n\t"	\
		"btq	%%rdx,	%[base]""\n\t"	\
		"setc	%[ret]"			\
		: [ret]	"+r" (_ret)		\
		: [base] "m" (_base),		\
		  "d" (_offset)			\
		: "cc", "memory"		\
	);					\
	_ret;					\
})

#define _BIT_TEST_IMM(_base, _imm8)	\
({						\
	register unsigned char _ret;		\
	asm volatile				\
	(					\
		"xor	%[ret], %[ret]"	"\n\t"	\
		"btq	%[imm8], %[base]""\n\t"	\
		"setc	%[ret]"			\
		: [ret]	"+r" (_ret)		\
		: [base] "m" (_base),		\
		  [imm8] "i" (_imm8)		\
		: "cc", "memory"		\
	);					\
	_ret;					\
})

#define _BITWISEAND(_lock, _opl, _opr)		\
({						\
	volatile unsigned long long _ret=_opl;	\
	asm volatile				\
	(					\
	_lock	"andq %[opr], %[ret]"		\
		: [ret] "=m" (_ret)		\
		: [opr] "Jr" (_opr)		\
		: "memory"			\
	);					\
	_ret;					\
})

#define BITSET(_lock, _base, _offset)			\
(							\
	__builtin_constant_p(_offset) ?			\
		_BITSET_IMM(_lock, _base, _offset)	\
	: 	_BITSET_GPR(_lock, _base, _offset)	\
)

#define BITCLR(_lock, _base, _offset)			\
(							\
	__builtin_constant_p(_offset) ?			\
		_BITCLR_IMM(_lock, _base, _offset)	\
	:	_BITCLR_GPR(_lock, _base, _offset)	\
)

#define BITVAL(_base, _offset)				\
(							\
	__builtin_constant_p(_offset) ?			\
		_BIT_TEST_IMM(_base, _offset)		\
	:	_BIT_TEST_GPR(_base, _offset)		\
)

#define BITWISEAND(_lock, _opl, _opr)	_BITWISEAND(_lock, _opl, _opr)
