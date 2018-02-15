/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef void (*SLICE_FUNC)(struct SLICE_STRUCT*, void*);

void Slice_NOP(struct SLICE_STRUCT*, void*) ;

void Slice_Atomic(struct SLICE_STRUCT*, void*) ;

void Slice_CRC32(struct SLICE_STRUCT*, void*) ;

void Slice_Conic(struct SLICE_STRUCT*, void*) ;

void Slice_Turbo(struct SLICE_STRUCT*, void*) ;

typedef struct {
	struct  RING_CTRL	ctrl;
		SLICE_FUNC	func;
} RING_SLICE;

extern RING_SLICE order_list[];

typedef void (*CALL_FUNC)(struct SLICE_STRUCT*, SLICE_FUNC, void*);

void CallWith_RDTSCP_RDPMC(struct SLICE_STRUCT*, SLICE_FUNC, void*) ;

void CallWith_RDTSC_RDPMC(struct SLICE_STRUCT*, SLICE_FUNC, void*) ;

void CallWith_RDTSCP_No_RDPMC(struct SLICE_STRUCT*, SLICE_FUNC, void*) ;

void CallWith_RDTSC_No_RDPMC(struct SLICE_STRUCT*, SLICE_FUNC, void*) ;

#define UNUSED(expr) do { (void)(expr); } while (0)
