/*
 * CoreFreq
 * Copyright (C) 2015-2026 CYRIL COURTIAT
 * Licenses: GPL2
 */

typedef void (*SLICE_FUNC) (	RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
				unsigned int, unsigned long );

void Slice_NOP( RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
		unsigned int, unsigned long );

void Slice_Atomic(RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
		unsigned int, unsigned long);

void Slice_CRC32(RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
		unsigned int, unsigned long);

void Slice_Conic(RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
		unsigned int, unsigned long);

void Slice_Turbo(RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
		unsigned int, unsigned long);

void Slice_Monte_Carlo(RO(SHM_STRUCT)*, RW(SHM_STRUCT)*,
			unsigned int, unsigned long);

typedef struct {
		RING_CTRL	ctrl;
		SLICE_FUNC	func;
	enum	PATTERN 	pattern;
} RING_SLICE;

extern RING_SLICE order_list[];

typedef void (*CALL_FUNC)(	RO(SHM_STRUCT)*,
				RW(SHM_STRUCT)*,
				unsigned int,
				SLICE_FUNC,
				unsigned long );

void CallWith_RDTSC_RDPMC(	RO(SHM_STRUCT)*,
				RW(SHM_STRUCT)*,
				unsigned int,
				SLICE_FUNC,
				unsigned long );

void CallWith_RDTSC_No_RDPMC(	RO(SHM_STRUCT)*,
				RW(SHM_STRUCT)*,
				unsigned int,
				SLICE_FUNC,
				unsigned long );

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

#define TURBO_LOOP	0x7ffffffUL

#define CRC32_SRC	"CYRIL_INGENIERIE"
#define CRC32_EXP	0x44f9d7bc

#define CONIC_ERROR	1e-07

#define PI_TRIALS	100000000LLU
#define PI_ERROR	1.0e-04
#define PI_CONST	3.141592653589793
