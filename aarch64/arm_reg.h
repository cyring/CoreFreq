/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

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
/*TODO(CleanUp)
#define PKG_POWER_LIMIT_LOCK_MASK	0x8000000000000000
#define PPn_POWER_LIMIT_LOCK_MASK	0x0000000080000000
*/