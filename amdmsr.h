/*
 * CoreFreq
 * Copyright (C) 2015-2016 CYRIL INGENIERIE
 * Licenses: GPL2
 */

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

typedef union
{
    struct
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
    };
} THERMTRIP_STATUS;
