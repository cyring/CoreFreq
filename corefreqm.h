/*
 * CoreFreq
 * Copyright (C) 2015-2020 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef void (*SLICE_FUNC)(SHM_STRUCT*, unsigned int, unsigned long);

void Slice_NOP(SHM_STRUCT*, unsigned int, unsigned long);

void Slice_Atomic(SHM_STRUCT*, unsigned int, unsigned long);

void Slice_CRC32(SHM_STRUCT*, unsigned int, unsigned long);

void Slice_Conic(SHM_STRUCT*, unsigned int, unsigned long);

void Slice_Turbo(SHM_STRUCT*, unsigned int, unsigned long);

typedef struct {
		RING_CTRL	ctrl;
		SLICE_FUNC	func;
	enum	PATTERN 	pattern;
} RING_SLICE;

extern RING_SLICE order_list[];

typedef void (*CALL_FUNC)(SHM_STRUCT*, unsigned int, SLICE_FUNC, unsigned long);

void CallWith_RDTSCP_RDPMC(	SHM_STRUCT*,
				unsigned int,
				SLICE_FUNC,
				unsigned long);

void CallWith_RDTSC_RDPMC(	SHM_STRUCT*,
				unsigned int,
				SLICE_FUNC,
				unsigned long);

void CallWith_RDTSCP_No_RDPMC(	SHM_STRUCT*,
				unsigned int,
				SLICE_FUNC,
				unsigned long);

void CallWith_RDTSC_No_RDPMC(	SHM_STRUCT*,
				unsigned int,
				SLICE_FUNC,
				unsigned long);

#define RESET_Slice(Slice)						\
({									\
	Slice.Counter[0].TSC = 0;					\
	Slice.Counter[1].TSC = 0;					\
	Slice.Counter[2].TSC = 0;					\
	Slice.Counter[0].INST= 0;					\
	Slice.Counter[1].INST= 0;					\
	Slice.Counter[2].INST= 0;					\
	Slice.Delta.TSC = 0;						\
	Slice.Delta.INST= 0;						\
	Slice.Error = 0;						\
})

#define SLICE_XCHG	0x436f757274696174LLU
#define SLICE_ATOM	0x436f726546726571LLU

#define TURBO_LOOP	0x17fffffffUL

#define CRC32_SRC	"CYRIL_INGENIERIE"
#define CRC32_EXP	0x44f9d7bc

#define CONIC_ERROR	1e-07

#define UNUSED(expr) do { (void)(expr); } while (0)
