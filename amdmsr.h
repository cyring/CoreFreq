/*
 * CoreFreq
 * Copyright (C) 2015-2017 CYRIL INGENIERIE
 * Licenses: GPL2
 */

static const CPUID_STRUCT CpuIDforAMD[CPUID_MAX_FUNC]={
	{.func=0x00000001, .sub=0x00000000},	/* Instruction set	*/
	{.func=0x00000005, .sub=0x00000000},	/* MONITOR/MWAIT	*/
	{.func=0x00000006, .sub=0x00000000},	/* Power & Thermal	*/
	{.func=0x00000007, .sub=0x00000000},	/* Extended Feature	*/
	{.func=0x0000000d, .sub=0x00000000},	/* Ext. State Main leaf	*/
	{.func=0x0000000d, .sub=0x00000001},	/* Ext. State Sub-leaf	*/
	{.func=0x0000000d, .sub=0x00000002},	/* Ext. State Sub-leaf	*/
	{.func=0x0000000d, .sub=0x0000003e},	/* Ext. State Sub-leaf	*/
	{.func=0x80000001, .sub=0x00000000},	/* Extended feature	*/
	{.func=0x80000002, .sub=0x00000000},	/* Processor Name Id.	*/
	{.func=0x80000003, .sub=0x00000000},	/* Processor Name Id.	*/
	{.func=0x80000004, .sub=0x00000000},	/* Processor Name Id.	*/
	{.func=0x80000005, .sub=0x00000000},	/* Caches L1D L1I & TLB	*/
	{.func=0x80000006, .sub=0x00000000},	/* Caches L2 L3 & TLB	*/
	{.func=0x80000007, .sub=0x00000000},	/* Advanced Power	*/
	{.func=0x80000008, .sub=0x00000000},	/* Address size		*/
	{.func=0x8000000a, .sub=0x00000000},	/* SVM Revision		*/
	{.func=0x80000019, .sub=0x00000000},	/* Caches & TLB 1G	*/
	{.func=0x8000001a, .sub=0x00000000},	/* Perf. Optimization	*/
	{.func=0x8000001b, .sub=0x00000000},	/* Inst. Based Sampling	*/
	{.func=0x8000001c, .sub=0x00000000},	/* Lightweight Profiling*/
	{.func=0x8000001d, .sub=0x00000000},	/* Cache L1D Properties	*/
	{.func=0x8000001d, .sub=0x00000001},	/* Cache L1I Properties	*/
	{.func=0x8000001d, .sub=0x00000002},	/* Cache L2 Properties	*/
	{.func=0x8000001d, .sub=0x00000003},	/* Cache Properties End	*/
	{.func=0x8000001e, .sub=0x00000000},	/* Extended Identifiers	*/
	{.func=0x40000000, .sub=0x00000000},	/* Hypervisor vendor	*/
	{.func=0x40000000, .sub=0x00000001},	/* Hypervisor interface */
	{.func=0x40000000, .sub=0x00000002},	/* Hypervisor version	*/
	{.func=0x40000000, .sub=0x00000003},	/* Hypervisor features	*/
	{.func=0x40000000, .sub=0x00000004},	/* Hyperv. requirements	*/
	{.func=0x40000000, .sub=0x00000005},	/* Hypervisor limits	*/
	{.func=0x40000000, .sub=0x00000006},	/* Hypervisor exploits	*/
	{.func=0x00000000, .sub=0x00000000},
};

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	SmmLock		:  1-0,  // SMM Configuration Lock.
	Reserved1	:  3-1,
	TlbCacheDis	:  4-3,  // Cacheable Memory Disable.
	INVDWBINVD	:  5-4,  // INVD to WBINVD Conversion.
	Reserved2	:  7-5,
	AllowFerrOnNe	:  8-7,  // Allow FERR on NE..
	IgnneEm		:  9-8,  // IGNNE port emulation enable.
	MonMwaitDis	: 10-9,  // 1 = MONITOR & MWAIT opcodes become invalid.
	MonMwaitUserEn	: 11-10, // MONITOR/MWAIT user mode enable. 0=pl0 only
	Reserved3	: 12-11,
	HltXSpCycEn	: 13-12, // halt-exit special bus cycle enable.
	SmiSpCycDis	: 14-13, // SMI special bus cycle disable.
	RsmSpCycDis	: 15-14, // RSM special bus cycle disable.
	Reserved4	: 17-15,
	Wrap32Dis	: 18-17, // 32-bit address wrap disable.
	McStatusWrEn	: 19-18, // Machine check status write enable.
	Reserved5	: 20-19,
	IoCfgGpFault	: 21-20, // IO-space configuration causes a GP fault.
	Reserved6	: 23-21,
	ForceRdWrSzPrb	: 24-23, // Force probes for RdSized and WrSized.
	TscFreqSel	: 25-24, // 1=The TSC increments at the P0 frequency.
	CpbDis		: 26-25, // 1=Core performance boost disable.
	EffFreqCntMwait	: 27-26, // Effective frequency counting during mwait.
	// Family 15h
	EffFreqROLock	: 28-27, // Read-only effective frequency counter lock.
	SmuLock		: 29-28,
	CSEnable	: 30-29, // Connected standby enable.
	Reserved7	: 32-30,
	Reserved	: 64-32;
    } Family_12h;
    struct
    {
	unsigned long long
	SmmLock		:  1-0,
	SLOWFENCE	:  2-1,  // Slow SFENCE Enable.
	Reserved1	:  3-2,
	TlbCacheDis	:  4-3,
	INVDWBINVD	:  5-4,
	Reserved2	:  6-5,
	FFDIS		:  7-6,  // TLB Flush Filter Disable.
	DISLOCK		:  8-7,  // Disable x86 LOCK prefix functionality.
	IgnneEm		:  9-8,
	Reserved3	: 12-9,
	HltXSpCycEn	: 13-12,
	SmiSpCycDis	: 14-13,
	RsmSpCycDis	: 15-14,
	SSEDIS		: 16-15, // SSE Instructions Disable.
	Reserved4	: 17-16,
	Wrap32Dis	: 18-17,
	McStatusWrEn	: 19-18,
	Reserved5	: 24-19,
	StartFID	: 30-24, // Startup FID Status.
	Reserved6	: 32-30,
	Reserved	: 64-32;
    } Family_0Fh;
} HWCR;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	CurrFID		:  6-0,	 // Current FID
	Reserved1	:  8-6,
	StartFID	: 14-8,	 // Startup FID
	Reserved2	: 16-14,
	MaxFID		: 22-16, // Max FID
	Reserved3	: 24-22,
	MaxRampVID	: 30-24, // Max Ramp VID
	Reserved4	: 31-30,
	FidVidPending	: 32-31, // 0b when the FID/VID change has completed.
	CurrVID		: 38-32, // Current VID
	Reserved5	: 40-38,
	StartVID	: 46-40, // Startup VID
	Reserved6	: 48-46,
	MaxVID		: 54-48, // Max VID
	Reserved7	: 56-54,
	PstateStep	: 57-56, // voltage reduction: 0b=25mV; 1b=50mV
	AltVidOffset	: 60-57, // [NA;-50;-100;-125;-150;-175;-200;-225]mV
	Reserved8	: 61-60,
	IntPstateSup	: 62-61, // 1b = Intermediate P-states is supported.
	Reserved9	: 64-62;
    };
} FIDVID_STATUS;

typedef union
{
	unsigned long long value;
    struct
    {
	unsigned long long
	IOMsgAddr	: 16-0,
	IOMsgData	: 24-16,
	IntrPndMsgDis	: 25-24,
	IntrPndMsg	: 26-25,
	IORd		: 27-26,
	SmiOnCmpHalt	: 28-27, // SMI on Multi-core halt
	C1eOnCmpHalt	: 29-28, // C1E on Multi-core halt
	Reserved1	: 32-29,
	Reserved2	: 64-32;
    };
} INT_PENDING_MSG;

typedef struct
{
	unsigned int
	Reserved1	:  1-0,
	SensorTrip	:  2-1,  // 1 if temp. sensor trip occurs & was enabled
	SensorCoreSelect:  3-2,  // 0b: CPU1 Therm Sensor. 1b: CPU0 Therm Sensor
	Sensor0Trip	:  4-3,  // 1 if trip @ CPU0 (single), or @ CPU1 (dual)
	Sensor1Trip	:  5-4,  // 1 if sensor trip occurs @ CPU0 (dual core)
	SensorTripEnable:  6-5,  // a THERMTRIP High event causes a PLL shutdown
	SelectSensorCPU	:  7-6,  // 0b: CPU[0,1] Sensor 0. 1b: CPU[0,1] Sensor 1
	Reserved2	:  8-7,
	DiodeOffset	: 14-8,  // offset should be added to the external temp.
	Reserved3	: 16-14,
	CurrentTemp	: 24-16, // 00h = -49C , 01h = -48C ... ffh = 206C 
	TjOffset	: 29-24, // Tcontrol = CurTmp - TjOffset * 2 - 49
	Reserved4	: 31-29,
	SwThermTrip	: 32-31; // diagnostic bit, for testing purposes only.
} THERMTRIP_STATUS;

typedef union {
	unsigned int		value;
	struct {
		unsigned int
		CSEnable	:  1-0,
		Spare		:  2-1,
		MemTestFailed	:  3-2,
		ReservedBits1	:  5-3,
		BaseAddrLo	: 14-5,
		ReservedBits2	: 19-14,
		BaseAddrHi	: 29-19,
		ReservedBits3	: 32-29;
	};
} AMD_0F_DRAM_CS_BASE_ADDR;

typedef union
{	// Function: 2 - Offset: 80h
	unsigned int		value;
	struct {
		unsigned int
			CS10	:  4-0,
			CS32	:  8-4,
			CS54	: 12-8,
			CS76	: 16-12,
		ReservedBits	: 32-16;
	};
} AMD_0F_DRAM_CS_MAPPING;

typedef union
{	// Function: 2 - Offset: 88h
	unsigned int		value;
	struct {
		unsigned int
		tCL		:  3-0,
		ReservedBits1	:  4-3,
		tRCD		:  6-4,
		ReservedBits2	:  8-6,
		tRP		: 10-8,
		ReservedBits3	: 11-10,
		tRTPr		: 12-11,
		tRAS		: 16-12,
		tRC		: 20-16,
		tWR		: 22-20,
		tRRD		: 24-22,
		MemClkDis	: 32-24;
	};
} AMD_0F_DRAM_TIMING_LOW;

typedef union
{	// Function: 2 - Offset: 90h
	unsigned int		value;
	struct {
		unsigned int
		InitializeDRAM	:  1-0,
		ExitSelfRefresh :  2-1,
		ReservedBits1	:  4-2,
		DRAM_Term	:  6-4,
		ReservedBits2	:  7-6,
		DRAM_DrvWeak	:  8-7,
		Parity_Enable	:  9-8,
		SelfRefRateEn	: 10-9,
		BurstLength32	: 11-10,  // 0b: 64-Byte, 1b: 32-Byte
		Width128	: 12-11,  // 0b: 64-bits, 1b: 128-bits
		X4_DIMMS	: 16-12,
		UnbufferedDIMM	: 17-16,
		ReservedBits3	: 19-17,
		ECC_DIMM_Enable : 20-19,
		ReservedBits4	: 32-20;
	};
} AMD_0F_DRAM_CONFIG_LOW;

typedef union
{	// Function: 2 - Offset: 94h
	unsigned int		value;
	struct {
		unsigned int
		MemClkFreq	:  3-0,  // 000b:200,001b:266,010b:333,011b:400
		MemClkFreqValid :  4-3,
		MaxAsyncLatency :  8-4,
		ReservedBits1	: 12-8,
		ReadDQS_Enable	: 13-12,
		ReservedBits2	: 14-13,
		DisDRAMInterface: 15-14,
		PowerDown_Enable: 16-15,
		PowerDownMode	: 17-16,
		FourRankSODimm	: 18-17,
		FourRankRDimm	: 19-18,
		ReservedBits3	: 20-19,
		SlowAccessMode	: 21-20,  // 2T Mode=[0b:1T , 1b:2T]
		ReservedBits4	: 22-21,
		BankSwizzleMode : 24-22,
		DcqBypassMax	: 28-24,
		tFAW		: 32-28;
	};
} AMD_0F_DRAM_CONFIG_HIGH;

typedef union
{	// HTT Unit ID Register: Func: 0 - Off: 64h
	unsigned int		value;
	struct {
		unsigned int
		C0Unit		:  2-0,
		C1Unit		:  4-2,
		McUnit		:  6-4,
		HbUnit		:  8-6,
		SbLink		: 10-8,
		ReservedBits	: 32-10;
	};
} AMD_0F_HTT_UNIT_ID;

typedef union
{	// HTT Link Frequency Capabilities: Func: 0 - Off: 88h, a8h, c8h
	unsigned int		value;
	struct {
		unsigned int
		MinRev		:  5-0,
		MajRev		:  8-5,
		LinkFreqMax	: 12-8,
		Error		: 16-12,
		LinkFreqCap	: 32-16;
	};
} AMD_0F_HTT_FREQUENCY;
