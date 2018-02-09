/*
 * CoreFreq
 * Copyright (C) 2015-2018 CYRIL INGENIERIE
 * Licenses: GPL2
 */

typedef void (*SLICE_FUNC)(void*);

void Slice_Atomic(void*) ;

void Slice_CRC32(void*) ;

void Slice_Conic(void*) ;

typedef struct {
	struct  RING_CTRL	ctrl;
		SLICE_FUNC	func;
} RING_SLICE;

extern RING_SLICE order_list[];

#define UNUSED(expr) do { (void)(expr); } while (0)
