/*
 * CoreFreq
 * Copyright (C) 2015 CYRIL INGENIERIE
 * Licenses: GPL2
 */

#define	BITSET(_base, _offset)			\
({						\
	asm volatile				\
	(					\
		"lock rex bts %1, %0"		\
		: "=m" (_base)			\
		: "Ir" (_offset)		\
		: "memory"			\
	);					\
})

#define	BITCLR(_base, _offset)			\
({						\
	asm volatile				\
	(					\
		"lock rex btr %1, %0"		\
		: "=m" (_base)			\
		: "Ir" (_offset)		\
		: "memory"			\
	);					\
})

#define	BITWISEAND(_opl, _opr)			\
({						\
	volatile unsigned long long _ret=_opl;	\
	asm volatile				\
	(					\
		"lock rex and %1, %0"		\
		: "=m" (_ret)			\
		: "Ir" (_opr)			\
		: "memory"			\
	);					\
	_ret;					\
})
