/*
 * CoreFreq
 * Copyright (C) 2015-2026 CYRIL COURTIAT
 * Licenses: GPL2
 */

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		CY		:  1-0, /* cycle register		*/
		TM		:  2-1, /* time register		*/
		IR		:  3-2, /* instret register		*/
		HPM3		:  4-3,
		HPM4		:  5-4,
		HPM5		:  6-5,
		HPM6		:  7-6,
		HPM7		:  8-7,
		HPM8		:  9-8,
		HPM9		: 10-9,
		HPM10		: 11-10,
		HPM11		: 12-11,
		HPM12		: 13-12,
		HPM13		: 14-13,
		HPM14		: 15-14,
		HPM15		: 16-15,
		HPM16		: 17-16,
		HPM17		: 18-17,
		HPM18		: 19-18,
		HPM19		: 20-19,
		HPM20		: 21-20,
		HPM21		: 22-21,
		HPM22		: 23-22,
		HPM23		: 24-23,
		HPM24		: 25-24,
		HPM25		: 26-25,
		HPM26		: 27-26,
		HPM27		: 28-27,
		HPM28		: 29-28,
		HPM29		: 30-29,
		HPM30		: 31-30,
		HPM31		: 32-31,
		ReservedBits	: 64-32;
	};
} SCOUNTEREN;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		WPRI0		:  1-0,
		SIE		:  2-1, /* Supervisor all Interrupts Enable */
		WPRI2		:  5-2,
		SPIE		:  6-5, /* Supervisor Prior Interrupt Enable */
		UBE		:  7-6, /* Endianness Control. 0:Little; 1:Big*/
		WPRI7		:  8-7,
		SPP		:  9-8, /* Supervisor Previous Privilege mode */
		VS		: 11-9, /* Vector Extension State	*/
		WPRI11		: 13-11,
		FS		: 15-13, /* Floating-point Unit State	*/
		XS		: 17-15, /* User-mode eXtensions State	*/
		WPRI17		: 18-17,
		SUM		: 19-18, /* Supervisor User Memory access */
		MXR		: 20-19, /* Make eXecutable Readable	*/
		WPRI20		: 32-20,
		UXL		: 34-32, /* U-mode UXLEN		*/
		WPRI34		: 63-34,
		SD		: 64-63;
	};
} SSTATUS;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Highest 	:  8-0,
		Guaranteed	: 16-8,
		Most_Efficient	: 24-16,
		Lowest		: 32-24,
		ReservedBits	: 64-32; /* **Must be zero**		*/
	};
} HWP_CAPABILITIES;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		EN_Guarantee_Chg:  1-0,
		EN_Excursion_Min:  2-1,
		EN_Highest_Chg	:  3-2,
		EN_PECI_OVERRIDE:  4-3,
		ReservedBits	: 64-4;  /* **Must be zero**		*/
	};
} HWP_INTERRUPT;

typedef union
{
	unsigned long long	value;
	struct
	{
		unsigned long long
		Minimum_Perf	:  8-0,
		Maximum_Perf	: 16-8,
		Desired_Perf	: 24-16,
		Energy_Pref	: 32-24,
		Activity_Window : 42-32,
		Package_Control : 43-42,
		ReservedBits	: 59-43,
		Act_Window_Valid: 60-59, /* Activity_Window Valid; Default=0 */
		EPP_Valid	: 61-60, /*1:[HWP_REQUEST];0:[HWP_REQUEST_PKG]*/
		Desired_Valid	: 62-61, /* -> Desired_Perf		*/
		Maximum_Valid	: 63-62, /* -> Maximum_Perf		*/
		Minimum_Valid	: 64-63; /* -> Minimum_Perf		*/
	};
} HWP_REQUEST;
