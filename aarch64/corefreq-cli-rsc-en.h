/*
 * CoreFreq
 * Copyright (C) 2015-2024 CYRIL COURTIAT
 * Licenses: GPL2
 */

#define RSC_COPY0_CODE_EN "      by CyrIng                                     "
#define RSC_COPY1_CODE_EN "                                                    "
#define RSC_COPY2_CODE_EN "            (C)2015-2024 CYRIL COURTIAT             "

#define RSC_LAYOUT_LCD_RESET_CODE	"::::"

#define RSC_LAYOUT_HEADER_PROC_CODE_EN					\
{									\
	' ','P','r','o','c','e','s','s','o','r',' ','[' 		\
}

#define RSC_LAYOUT_HEADER_CPU_CODE_EN					\
{									\
	']',' ',' ',' ',' ','/',' ',' ',' ',' ','C','P','U'		\
}

#define RSC_LAYOUT_HEADER_ARCH_CODE_EN					\
{									\
	' ','A','r','c','h','i','t','e','c','t','u','r','e',' ','['	\
}

#define RSC_LAYOUT_HEADER_CACHE_L1_CODE_EN				\
{									\
	']',' ','C','a','c','h','e','s',' ',				\
	'L','1',' ','I','n','s','t','=',' ',' ',' ',			\
	'D','a','t','a','=',' ',' ',' ','K','B' 			\
}

#define RSC_LAYOUT_HEADER_BCLK_CODE_EN					\
{									\
	' ','B','a','s','e',' ','C','l','o','c','k',' ',		\
	'~',' ','0','0','0',',','0','0','0',',','0','0','0',' ','H','z' \
}

#define RSC_LAYOUT_HEADER_CACHES_CODE_EN				\
{									\
	'L','2','=',' ',' ',' ',' ',' ',' ',' ',			\
	'L','3','=',' ',' ',' ',' ',' ',' ','K','B'			\
}

#define RSC_LAYOUT_RULER_LOAD_CODE_EN					\
{									\
	'-','-','-',' ', '!',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_REL_LOAD_CODE_EN				\
{									\
	'R','e','l','a','t','i','v','e',' ','f','r','e','q','u','e','n',\
	'c','y' 							\
}

#define RSC_LAYOUT_RULER_ABS_LOAD_CODE_EN				\
{									\
	'A','b','s','o','l','u','t','e',' ','f','r','e','q','u','e','n',\
	'c','y' 							\
}

#define RSC_LAYOUT_MONITOR_FREQUENCY_CODE_EN				\
{									\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',0x0,' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',0x0,' ',' ',' '		\
}

#define RSC_LAYOUT_MONITOR_INST_CODE_EN					\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,' ',' ',' ',' ',\
	' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0x0,' ',\
	' ',' ',' ',' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',0x0,' ',' ',' ',' ',' ',' ',0x0,0x0,' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
}

#define RSC_LAYOUT_MONITOR_COMMON_CODE_EN				\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
}

#define RSC_LAYOUT_MONITOR_TASKS_CODE_EN				\
{									\
	' ',' ',' ',' ',' ',0x0,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define RSC_LAYOUT_MONITOR_SLICE_CODE_EN				\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '	\
}

#define RSC_LAYOUT_CUSTOM_FIELD_CODE_EN 				\
{									\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '		\
}

#define RSC_LAYOUT_RULER_FREQUENCY_CODE_EN				\
{									\
	'-','-','-',' ','F','r','e','q','(','M','H','z',')',' ','R','a',\
	't','i','o',' ','-',' ','T','u','r','b','o',' ','-','-',' ','C',\
	'0',' ','-','-','-','-',' ','C','1',' ','-','-',' ','C','2',':',\
	'C','3',' ','-',' ','C','4',':','C','6',' ','-','-','-',' ','C',\
	'7',' ','-','-',' ','M','i','n',' ','T','M','P',' ','M','a','x',\
	' ','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_FREQUENCY_AVG_CODE_EN				\
{									\
	'-','-','-','-','-','-',' ','%',' ','A','v','e','r','a','g','e',\
	's',' ','[',' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',' ',' ',' ',0x0,' ',' ',0x0,' ',' ',' ',' ',0x0,\
	' ',' ',0x0,' ',']','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_FREQUENCY_PKG_CODE_EN				\
{									\
	'%',' ','P','k','g',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',']',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_INST_CODE_EN					\
{									\
	'-','-','-','-','-','-','-','-','-','-','-','-',' ','I','P','S',\
	' ','-','-','-','-','-','-','-','-','-','-','-','-','-','-',' ',\
	'I','P','C',' ','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-',' ','C','P','I',' ','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-',' ','I','N','S','T',' ','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_CYCLES_CODE_EN 				\
{									\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-',' ','C',\
	'0',':','U','C','C',' ','-','-','-','-','-','-','-','-','-','-',\
	' ','C','0',':','U','R','C',' ','-','-','-','-','-','-','-','-',\
	'-','-','-','-',' ','C','1',' ','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-',' ','T','S','C',' ','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_CSTATES_CODE_EN				\
{									\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	' ','C','1',' ','-','-','-','-','-','-','-','-','-','-','-','-',\
	' ','C','2',':','C','3',' ','-','-','-','-','-','-','-','-','-',\
	'-','-',' ','C','4',':','C','6',' ','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-',' ','C','7',' ','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_INTERRUPTS_CODE_EN				\
{									\
	'-','-','-','-','-','-','-','-','-','-',' ','S','M','I',' ','-',\
	'-','-','-','-','-','-','-','-','-','-','-',' ','N','M','I','[',\
	' ','L','O','C','A','L',' ',' ',' ','U','N','K','N','O','W','N',\
	' ',' ','P','C','I','_','S','E','R','R','#',' ',' ','I','O','_',\
	'C','H','E','C','K',']',' ','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',\
	'-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-' \
}

#define RSC_LAYOUT_RULER_PACKAGE_CODE_EN				\
	"------------ Cycles ---- State ------------------ Clock Rati"	\
	"o ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_TASKS_CODE_EN					\
	"--- Freq(MHz) --- Tasks                    -----------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_PACKAGE_PC_CODE_EN					\
{									\
	' ',' ','0','0',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define RSC_LAYOUT_PACKAGE_PC02_CODE_EN {'P', 'C', '0', '2'}
#define RSC_LAYOUT_PACKAGE_PC03_CODE_EN {'P', 'C', '0', '3'}
#define RSC_LAYOUT_PACKAGE_PC04_CODE_EN {'P', 'C', '0', '4'}
#define RSC_LAYOUT_PACKAGE_PC06_CODE_EN {'P', 'C', '0', '6'}
#define RSC_LAYOUT_PACKAGE_PC07_CODE_EN {'P', 'C', '0', '7'}
#define RSC_LAYOUT_PACKAGE_PC08_CODE_EN {'P', 'C', '0', '8'}
#define RSC_LAYOUT_PACKAGE_PC09_CODE_EN {'P', 'C', '0', '9'}
#define RSC_LAYOUT_PACKAGE_PC10_CODE_EN {'P', 'C', '1', '0'}
#define RSC_LAYOUT_PACKAGE_MC06_CODE_EN {'M', 'C', '0', '6'}

#define RSC_LAYOUT_PACKAGE_UNCORE_CODE_EN				\
{									\
	' ','T','S','C',':',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ','U','N','C','O','R','E',':',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',\
	' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' \
}

#define RSC_LAYOUT_TASKS_STATE_SORTED_CODE_EN				\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','S','t','a','t','e',')',' ', '-','-','-'			\
}

#define RSC_LAYOUT_TASKS_RUNTIME_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','R','u','n','T','i','m','e', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_USRTIME_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','U','s','e','r','T','i','m', 'e',')',' '			\
}

#define RSC_LAYOUT_TASKS_SYSTIME_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','S','y','s','T','i','m','e', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_PROCESS_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','P','I','D',')',' ','-','-', '-','-','-'			\
}

#define RSC_LAYOUT_TASKS_COMMAND_SORTED_CODE_EN 			\
{									\
	'(','s','o','r','t','e','d',' ', 'b','y',			\
	' ','C','o','m','m','a','n','d', ')',' ','-'			\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_OFF_CODE_EN			\
{									\
	' ', 'I','n','v','e','r','s','e',' ','[','O','F','F',']',' '	\
}

#define RSC_LAYOUT_TASKS_REVERSE_SORT_ON_CODE_EN			\
{									\
	' ', 'I','n','v','e','r','s','e',' ','[',' ','O','N',']',' '	\
}

#define RSC_LAYOUT_TASKS_VALUE_SWITCH_CODE_EN				\
{									\
	' ', 'V','a','l','u','e',' ','[',' ',' ',' ',']',' '		\
}

#define RSC_LAYOUT_TASKS_VALUE_OFF_CODE_EN				\
{									\
	'O','F','F'							\
}

#define RSC_LAYOUT_TASKS_VALUE_ON_CODE_EN				\
{									\
	' ','O','N'							\
}

#define RSC_LAYOUT_TASKS_TRACKING_CODE_EN				\
{									\
	' ','T','r','a','c','k','i', 'n','g',' ','P','I','D',' ','[',' ',\
	' ','O','F','F',' ',' ',']',' ' 				\
}

#define RSC_LAYOUT_RULER_SENSORS_CODE_EN				\
	"--- Freq(MHz) --- Vcore --- TMP( ) --- Energy(J) --- Power(W"	\
	") ----------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_PWR_SOC_CODE_EN				\
	"-RAM:   .    ( ) --- SoC :   .    ( ) -- Pkg[ ]:   .    ( ) "	\
	"- Cores:   .    ( )-----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_VOLTAGE_CODE_EN				\
	"--- Freq(MHz) - VID --- Min(V) - Vcore -- Max(V) -----------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_VPKG_SOC_CODE_EN				\
	"- Processor[                                    ] ----- SoC "	\
	"[       ] [      V]-----------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_ENERGY_CODE_EN 				\
	"--- Freq(MHz) -- Accumulator -------- Min ------ Energy(J) -"	\
	"- Max ------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_POWER_CODE_EN					\
	"--- Freq(MHz) -- Accumulator -------- Min ------- Power(W) -"	\
	"- Max ------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_SLICE_CODE_EN					\
	"--- Freq(MHz) ------ Cycles -- Instructions ------------ TSC"	\
	" ------------ PMC0 ----------- Error -----------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_RULER_CUSTOM_CODE_EN 				\
	"----- Min - Relative - Max ---- Min - Absolute - Max - Min T"	\
	"MP Max - Min(V) - Vcore - Max(V) - Min( ) - Power -- Max( ) "	\
	"- Turbo -- C0 -- C1 -- C2:C3  C4:C6 -- C7 --- IPS ----- IPC "	\
	"----- CPI --------------------------------------------------"	\
	"------------------------------------------------------------"	\
	"--------------------"

#define RSC_LAYOUT_FOOTER_TECH_TSC_CODE_EN				\
{									\
	'T','e','c','h',' ','[',' ',' ','T','S','C',' ',' ',',' 	\
}

#define RSC_LAYOUT_FOOTER_VOLT_TEMP_CODE_EN				\
{									\
	'V','[',' ','.',' ',' ',']',' ','T','[',' ',' ',' ',' ',']'	\
}

#define RSC_LAYOUT_FOOTER_SYSTEM_CODE_EN				\
{									\
	'T','a','s','k','s',' ','[',' ',' ',' ',' ',' ',' ',']',	\
	' ','M','e','m',' ','[',' ',' ',' ',' ',' ',' ',' ',' ',	\
	' ','/',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','B',']' 	\
}

#define RSC_LAYOUT_FOOTER_TSC_NONE_CODE "  TSC  "
#define RSC_LAYOUT_FOOTER_TSC_VAR_CODE	"TSC-VAR"
#define RSC_LAYOUT_FOOTER_TSC_INV_CODE	"TSC-INV"

#define RSC_LAYOUT_CARD_CORE_ONLINE_COND0_CODE_EN			\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ','C',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CORE_ONLINE_COND1_CODE_EN			\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ','F',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CORE_OFFLINE_CODE_EN				\
{									\
	'[',' ',' ',' ',' ',' ',' ','O','F','F',' ',']' 		\
}

#define RSC_LAYOUT_CARD_CLK_CODE_EN					\
{									\
	'[',' ','0','0','0','.','0',' ','M','H','z',']' 		\
}

#define RSC_LAYOUT_CARD_UNCORE_CODE_EN					\
{									\
	'[','U','N','C','O','R','E',' ',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_BUS_CODE_EN					\
{									\
	'[','B','u','s',' ',' ',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_MC_CODE_EN					\
{									\
	'[',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_LOAD_CODE_EN					\
{									\
	'[',' ',' ','%','L','O','A','D',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_IDLE_CODE_EN					\
{									\
	'[',' ',' ','%','I','D','L','E',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_RAM_CODE_EN					\
{									\
	'[',' ',' ',' ',' ',' ',' ','/',' ',' ',' ',']' 		\
}

#define RSC_LAYOUT_CARD_TASK_CODE_EN					\
{									\
	'[','T','a','s','k','s',' ',' ',' ',' ',' ',']' 		\
}

#define RSC_CREATE_HOTPLUG_CPU_TITLE_CODE	" CPU "
#define RSC_CREATE_HOTPLUG_CPU_ENABLE_CODE_EN	"<   ENABLE >"
#define RSC_CREATE_HOTPLUG_CPU_DISABLE_CODE_EN	"<  DISABLE >"
#define RSC_CREATE_HOTPLUG_CPU_ONLINE_CODE_EN	" %03u   On   "
#define RSC_CREATE_HOTPLUG_CPU_OFFLINE_CODE_EN	" %03u  Off   "

#define RSC_COREFREQ_TITLE_CODE 	" CoreFreq "
#define RSC_PROCESSOR_TITLE_CODE_EN	" Processor "
#define RSC_PROCESSOR_CODE_EN		"Processor"
#define RSC_ARCHITECTURE_CODE_EN	"Architecture"
#define RSC_VENDOR_ID_CODE_EN		"Vendor ID"
#define RSC_REVISION_CODE_EN		"Revision"
#define RSC_SIGNATURE_CODE_EN		"Signature"
#define RSC_STEPPING_CODE_EN		"Stepping"
#define RSC_ONLINE_CPU_CODE_EN		"Online CPU"
#define RSC_BASE_CLOCK_CODE_EN		"Base Clock"
#define RSC_FREQUENCY_CODE_EN		"Frequency"
#define RSC_RATIO_CODE_EN		"Ratio"
#define RSC_FACTORY_CODE_EN		"Factory"
#define RSC_PERFORMANCE_CODE_EN 	"Performance"
#define RSC_TARGET_CODE_EN		"Target"
#define RSC_UNLOCK_CODE_EN		"UNLOCK"
#define RSC_LOCK_CODE_EN		"  LOCK"
#define RSC_ENABLE_CODE_EN		" Enable"
#define RSC_DISABLE_CODE_EN		"Disable"
#define RSC_CAPABILITIES_CODE_EN	"Capabilities"
#define RSC_LOWEST_CODE_EN		"Lowest"
#define RSC_EFFICIENT_CODE_EN		"Efficient"
#define RSC_GUARANTEED_CODE_EN		"Guaranteed"
#define RSC_HIGHEST_CODE_EN		"Highest"
#define RSC_RECORDER_CODE_EN		"Recorder"
#define RSC_STRESS_CODE_EN		"Stress"
#define RSC_SYSGATE_CODE		"SysGate"

#define RSC_SCOPE_NONE_CODE_EN		"None"
#define RSC_SCOPE_THREAD_CODE_EN	" SMT"
#define RSC_SCOPE_CORE_CODE_EN		"Core"
#define RSC_SCOPE_PACKAGE_CODE_EN	" Pkg"

#define RSC_SYS_REGS_TITLE_CODE_EN	" System Registers "
#define RSC_SYS_REG_PSTATE_CODE_EN	" Process State (PSTATE) "
#define RSC_SYS_REG_FLAG_N_CODE_EN	" Negative "
#define RSC_SYS_REG_FLAG_Z_CODE_EN	" Zero Flag "
#define RSC_SYS_REG_FLAG_C_CODE_EN	" Carry Flag "
#define RSC_SYS_REG_FLAG_V_CODE_EN	" Overflow "
#define RSC_SYS_REG_FLAG_D_CODE_EN	" Endianness "
#define RSC_SYS_REG_FLAG_A_CODE_EN	" SError "
#define RSC_SYS_REG_FLAG_I_CODE_EN	" IRQ "
#define RSC_SYS_REG_FLAG_F_CODE_EN	" FIQ "
#define RSC_SYS_REG_FLAG_EL_CODE_EN	" Current EL "
#define RSC_SYS_REG_FLAG_SM_CODE_EN	" SME ZA storage : Streaming SVE "

#define RSC_SYS_REG_FLAG_SSBS_CODE_EN	\
	" Speculative Store Bypass Safe (MSR & MRS) "

#define RSC_SYS_REG_FLAG_NMI_CODE_EN	" Non-maskable Interrupt "
#define RSC_SYS_REG_FLAG_PAN_CODE_EN	" Privileged Access Never "
#define RSC_SYS_REG_FLAG_UAO_CODE_EN	" User Access Override "
#define RSC_SYS_REG_FLAG_DIT_CODE_EN	" Data Independent Timing "
#define RSC_SYS_REG_FLAG_TCO_CODE_EN	" Tag Check Override "
#define RSC_SYS_REG_FLAG_PM_CODE_EN	" PMU Exception Mask "

#define RSC_SYS_REG_SCTL_CODE_EN	" System Control Register (SCTLR_EL1) "
#define RSC_SYS_REG_TIDCP_CODE_EN " Implementation Defined System instructions "
#define RSC_SYS_REG_SPINT_CODE_EN	" SP Interrupt Mask enable "
#define RSC_SYS_REG_NMI_CODE_EN		" Non-Maskable Interrupt enable "
#define RSC_SYS_REG_EnTP2_CODE_EN " Read/Write Software Thread ID (TPIDR2_EL0) "
#define RSC_SYS_REG_TCSO_CODE_EN	" Tag Checking Store Only "
#define RSC_SYS_REG_TCSO1_CODE_EN	" Tag Checking Store Only (EL1) "
#define RSC_SYS_REG_TCSO0_CODE_EN	" Tag Checking Store Only (EL0) "
#define RSC_SYS_REG_EPAN_CODE_EN	" Enhanced Privileged Access Never "
#define RSC_SYS_REG_EnALS_CODE_EN	" LD64B and ST64B instructions "
#define RSC_SYS_REG_EnAS0_CODE_EN	" ST64BV0 instruction "
#define RSC_SYS_REG_EnASR_CODE_EN	" ST64BV instruction "
#define RSC_SYS_REG_TME_CODE_EN 	" Transactional Memory Extension "
#define RSC_SYS_REG_TME1_CODE_EN	" Transactional Memory Extension (EL1) "
#define RSC_SYS_REG_TME0_CODE_EN	" Transactional Memory Extension (EL0) "
#define RSC_SYS_REG_TMT_CODE_EN 	" Trivial implementation of TME "
#define RSC_SYS_REG_TMT1_CODE_EN	" TMT (EL1) "
#define RSC_SYS_REG_TMT0_CODE_EN	" TMT (EL0) "
#define RSC_SYS_REG_TWE_D_CODE_EN	" TWE Delay "
#define RSC_SYS_REG_TWE_C_CODE_EN	" TWE Delay cycles "
#define RSC_SYS_REG_TWE_E_CODE_EN	" TWE Delay enable "
#define RSC_SYS_REG_DSSBS_CODE_EN " Speculative Store Bypass Safe (Exception) "
#define RSC_SYS_REG_ATA_CODE_EN 	" Allocation Tag Access "
#define RSC_SYS_REG_ATA1_CODE_EN	" Allocation Tag Access (EL1) "
#define RSC_SYS_REG_ATA0_CODE_EN	" Allocation Tag Access (EL0) "
#define RSC_SYS_REG_TCF_CODE_EN 	" Tag Check Fault "
#define RSC_SYS_REG_TCF1_CODE_EN	" Tag Check Fault (EL1) "
#define RSC_SYS_REG_TCF0_CODE_EN	" Tag Check Fault (EL0) "
#define RSC_SYS_REG_ITFSB_CODE_EN " Tag Fault Status (TFSRE0_EL1 & TFSR_EL1) "
#define RSC_SYS_REG_BT_CODE_EN		" Branch Target Identification "
#define RSC_SYS_REG_BT1_CODE_EN 	" PAC Branch Type compatibility (EL1) "
#define RSC_SYS_REG_BT0_CODE_EN 	" PAC Branch Type compatibility (EL0) "
#define RSC_SYS_REG_EnFPM_CODE_EN	" Floating-Point Mode enable "
#define RSC_SYS_REG_MSCEn_CODE_EN " Memory Copy and Memory Set instructions "
#define RSC_SYS_REG_CMOW_CODE_EN " Control for Cache Maintenance permission "
#define RSC_SYS_REG_EnIA_CODE_EN	" Pointer Authentication (APIAKey_EL1) "
#define RSC_SYS_REG_EnIB_CODE_EN	" Pointer Authentication (APIBKey_EL1) "
#define RSC_SYS_REG_LSM_CODE_EN 	" Load Multiple and Store Multiple "
#define RSC_SYS_REG_LSMA_CODE_EN	" LSM Atomicity and Ordering "
#define RSC_SYS_REG_LSMD_CODE_EN " LSM to Device-{nGRE,nGnRE,nGnRnE} memory "
#define RSC_SYS_REG_EnDA_CODE_EN	" Pointer Authentication (APDAKey_EL1) "
#define RSC_SYS_REG_UCI_CODE_EN 	" Cache Maintenance Instructions "
#define RSC_SYS_REG_EE_CODE_EN		" Exception Endianness (EL1) "
#define RSC_SYS_REG_E0E_CODE_EN 	" Exception Endianness (EL0) "
#define RSC_SYS_REG_SPAN_CODE_EN	" Set Privileged Access Never "
#define RSC_SYS_REG_EIS_CODE_EN " Context Synchronizing Exception Entry "
#define RSC_SYS_REG_IESB_CODE_EN	" Implicit Error Synchronization Event "
#define RSC_SYS_REG_TSCXT_CODE_EN	" Read/Write Software Context Number "
#define RSC_SYS_REG_WXN_CODE_EN 	" Write Execute-Never "
#define RSC_SYS_REG_nTWE_CODE_EN	" WFE instructions "
#define RSC_SYS_REG_nTWI_CODE_EN	" WFI instructions "
#define RSC_SYS_REG_UCT_CODE_EN 	" Cache Type Register (CTR_EL0) "
#define RSC_SYS_REG_DZE_CODE_EN 	" Data Cache Zero instructions "
#define RSC_SYS_REG_EnDB_CODE_EN	" Pointer Authentication (APDBKey_EL1) "
#define RSC_SYS_REG_I_CODE_EN	" Instruction access Cacheability control "
#define RSC_SYS_REG_EOS_CODE_EN 	" Context Synchronizing Exception Exit "
#define RSC_SYS_REG_RCTX_CODE_EN  " Restriction by Context system instructions "
#define RSC_SYS_REG_UMA_CODE_EN 	" User Mask Access "
#define RSC_SYS_REG_SED_CODE_EN 	" SETEND instruction disable "
#define RSC_SYS_REG_ITD_CODE_EN 	" IT instructions disable "
#define RSC_SYS_REG_nAA_CODE_EN 	" Non-Aligned Access "
#define RSC_SYS_REG_CP15B_CODE_EN	" Memory Barrier system instructions "
#define RSC_SYS_REG_SA0_CODE_EN 	" SP Alignment check (EL0) "
#define RSC_SYS_REG_SA1_CODE_EN 	" SP Alignment check (EL1) "
#define RSC_SYS_REG_C_CODE_EN	" Cacheability control for data accesses "
#define RSC_SYS_REG_A_CODE_EN		" Alignment check enable "
#define RSC_SYS_REG_M_CODE_EN		" Memory Management Unit enable "

#define RSC_SYS_REG_SCTL2_CODE_EN	" System Control Register (SCTLR2_EL1) "
#define RSC_SYS_REG_CPTM_CODE_EN " Check Pointer Arithmetic for Multiplication "
#define RSC_SYS_REG_CPTM0_CODE_EN	" CPTM (EL0) "
#define RSC_SYS_REG_CPTM1_CODE_EN	" CPTM (EL1) "
#define RSC_SYS_REG_CPTA_CODE_EN " Check Pointer Arithmetic for Addition "
#define RSC_SYS_REG_CPTA0_CODE_EN	" CPTA (EL0) "
#define RSC_SYS_REG_CPTA1_CODE_EN	" CPTA (EL1) "
#define RSC_SYS_REG_PACM_CODE_EN	" Pointer Authentication Modifier "
#define RSC_SYS_REG_PACM0_CODE_EN	" PACM (EL0) "
#define RSC_SYS_REG_PACM1_CODE_EN	" PACM (EL1) "
#define RSC_SYS_REG_128SR_CODE_EN	" Enable 128-bit System registers "
#define RSC_SYS_REG_EASE_CODE_EN	" External Aborts to SError Exception "
#define RSC_SYS_REG_ANERR_CODE_EN	" Asynchronous Normal Read Error "
#define RSC_SYS_REG_ADERR_CODE_EN	" Asynchronous Device Read Error "
#define RSC_SYS_REG_NMEA_CODE_EN	" Non-Maskable External Aborts "

#define RSC_SYS_REG_FPSR_CODE_EN	" Floating-point Status Register "
#define RSC_SYS_REG_FPSR_QC_CODE_EN	" Cumulative Saturation "
#define RSC_SYS_REG_FPSR_IDC_CODE_EN	" Input Denormal Cumulative "
#define RSC_SYS_REG_FPSR_IXC_CODE_EN	" Inexact Cumulative "
#define RSC_SYS_REG_FPSR_UFC_CODE_EN	" Underflow Cumulative "
#define RSC_SYS_REG_FPSR_OFC_CODE_EN	" Overflow Cumulative "
#define RSC_SYS_REG_FPSR_DZC_CODE_EN	" Divide by Zero Cumulative "
#define RSC_SYS_REG_FPSR_IOC_CODE_EN	" Invalid Operation Cumulative "

#define RSC_SYS_REG_EL_CODE_EN		" Exception Level "
#define RSC_SYS_REG_EL_EXEC_CODE_EN	" Executes in AArch64 or AArch32 "
#define RSC_SYS_REG_EL_SEC_CODE_EN	" Secure Exception Level "

#define RSC_ISA_TITLE_CODE_EN		" Instruction Set Extensions "

#define RSC_ISA_AES_COMM_CODE_EN	" Advanced Encryption Standard "
#define RSC_ISA_LSE_COMM_CODE_EN	" Atomic instructions "
#define RSC_ISA_CRC32_COMM_CODE_EN	" Cyclic Redundancy Check "
#define RSC_ISA_DP_COMM_CODE_EN 	" Dot Product instructions "
#define RSC_ISA_EPAC_COMM_CODE_EN 	" Enhanced Pointer Authentication "
#define RSC_ISA_FCMA_COMM_CODE_EN	\
			" Floating-point Complex Multiplication & Addition "

#define RSC_ISA_FHM_COMM_CODE_EN	\
				" Floating-point Half-precision Multiplication "

#define RSC_ISA_FP_COMM_CODE_EN 	" Floating Point "
#define RSC_ISA_FPAC_COMM_CODE_EN 	" Faulting Pointer Authentication Code "
#define RSC_ISA_FPACCOMBINE_COMM_CODE_EN \
				" Faulting on Combined Pointer Authentication "

#define RSC_ISA_JSCVT_COMM_CODE_EN	" JavaScript Conversion "
#define RSC_ISA_LRCPC_COMM_CODE_EN	" Load-Acquire RCpc instructions "
#define RSC_ISA_MOPS_COMM_CODE_EN	\
				" Memory Copy and Memory Set instructions "

#define RSC_ISA_PACIMP_COMM_CODE_EN	\
			" Pointer Authentication Code, using Generic key "

#define RSC_ISA_PACQARMA3_COMM_CODE_EN	\
		" Pointer Authentication Code, using the QARMA3 algorithm "

#define RSC_ISA_PACQARMA5_COMM_CODE_EN	\
		" Pointer Authentication Code, using the QARMA5 algorithm "

#define RSC_ISA_PAUTH_COMM_CODE_EN	" Pointer Authentication "
#define RSC_ISA_PAUTH2_COMM_CODE_EN	" Enhanced Pointer Authentication "
#define RSC_ISA_PAUTH_LR_COMM_CODE_EN	" Pointer Authentication Link Register "
#define RSC_ISA_PRFMSLC_COMM_CODE_EN	" PRFM instructions support SLC target "
#define RSC_ISA_FRINTTS_COMM_CODE_EN	" Floating-point to Integer "
#define RSC_ISA_SPECRES_COMM_CODE_EN	" Prediction Invalidation "
#define RSC_ISA_BF16_COMM_CODE_EN	" BFloat16 instructions "
#define RSC_ISA_EBF16_COMM_CODE_EN	" Extended BFloat16 "
#define RSC_ISA_CSSC_COMM_CODE_EN	" Common Short Sequence Compression "
#define RSC_ISA_HBC_COMM_CODE_EN	" Hinted Conditional Branch "
#define RSC_ISA_I8MM_COMM_CODE_EN	" Int8 Matrix Multiplication "
#define RSC_ISA_RPRES_COMM_CODE_EN	\
		" Reciprocal Estimate & Reciprocal Square Root Estimate "

#define RSC_ISA_SB_COMM_CODE_EN 	" Speculation Barrier "
#define RSC_ISA_SYSREG128_COMM_CODE_EN	\
			" Instructions to access 128-bit System Registers "

#define RSC_ISA_SYSINSTR128_COMM_CODE_EN \
			" System instructions that can take 128-bit inputs "

#define RSC_ISA_WFxT_COMM_CODE_EN 	" WFE & WFI instructions with timeout "
#define RSC_ISA_XS_COMM_CODE_EN 	" XS attribute for memory "
#define RSC_ISA_LS64_COMM_CODE_EN	" Atomic 64-byte loads and stores "
#define RSC_ISA_DGH_COMM_CODE_EN	" Data Gathering Hint "
#define RSC_ISA_DPB_COMM_CODE_EN	" Data Persistence writeback "
#define RSC_ISA_RAND_COMM_CODE_EN	" Read Random Number "
#define RSC_ISA_RDMA_COMM_CODE_EN	" Rounding Double Multiply Accumulate "
#define RSC_ISA_RPRFM_COMM_CODE_EN	" RPRFM hint instruction "
#define RSC_ISA_SHA_COMM_CODE_EN	" Secure Hash Algorithms extensions "
#define RSC_ISA_SM_COMM_CODE_EN 	" Chinese cryptography algorithm "
#define RSC_ISA_SIMD_COMM_CODE_EN	" Advanced SIMD Extensions "
#define RSC_ISA_SME_COMM_CODE_EN	" Scalable Matrix Extension "
#define RSC_ISA_SVE_COMM_CODE_EN	" Scalable Vector Extension "
#define RSC_ISA_FlagM_COMM_CODE_EN 	" Flag manipulation instructions "

#define RSC_FEATURES_TITLE_CODE_EN	" Features "
#define RSC_ON_CODE_EN			" ON"
#define RSC_OFF_CODE_EN 		"OFF"
#define RSC_FMW_CODE_EN 		"FMW"
#define RSC_NOT_AVAILABLE_CODE_EN	"N/A"
#define RSC_AUTOMATIC_CODE_EN		"AUTO"
#define RSC_UNABLE_CODE_EN		"Unable"
#define RSC_MISSING_CODE_EN		"Missing"
#define RSC_PRESENT_CODE_EN		"Capable"
#define RSC_VARIANT_CODE_EN		"Variant"
#define RSC_INVARIANT_CODE_EN		"Invariant"

#define RSC_FEATURES_ACPI_CODE_EN    "Advanced Configuration & Power Interface"
#define RSC_FEATURES_AMU_CODE_EN	"Activity Monitor Unit"
#define RSC_FEATURES_BIG_END_CODE_EN	"Mixed-Endianness"
#define RSC_FEATURES_BTI_CODE_EN	"Branch Target Identification"
#define RSC_FEATURES_EBEP_CODE_EN	"Exception-based event profiling"
#define RSC_FEATURES_ECV_CODE_EN	"Enhanced Counter Virtualization"
#define RSC_FEATURES_DIT_CODE_EN	"Data Independent Timing"
#define RSC_FEATURES_EXS_CODE_EN  "Context Synchronization & Exception Handling"
#define RSC_FEATURES_FGT_CODE_EN	"Fine-Grained Trap controls"
#define RSC_FEATURES_GCS_CODE_EN	"Guarded Control Stack"
#define RSC_FEATURES_GIC_CODE_EN	"Generic Interrupt Controller"
#define RSC_FEATURES_MPAM_CODE_EN	"Memory Partitioning and Monitoring"
#define RSC_FEATURES_MTE_CODE_EN	"Memory Tagging Extension"
#define RSC_FEATURES_NMI_CODE_EN	"Non Maskable Interrupt"
#define RSC_FEATURES_PA_CODE_EN 	"Physical Address range"
#define RSC_FEATURES_PAN_CODE_EN	"Privileged Access Never"
#define RSC_FEATURES_RAS_CODE_EN     "Reliability Availability & Serviceability"
#define RSC_FEATURES_RME_CODE_EN	"Realm Management Extension"
#define RSC_FEATURES_THE_CODE_EN	"Translation Hardening Extension"
#define RSC_FEATURES_TLB_CODE_EN	"TLB maintenance instructions"
#define RSC_FEATURES_TME_CODE_EN	"Transactional Memory Extension"
#define RSC_FEATURES_TSC_CODE_EN	"Time Stamp Counter"
#define RSC_FEATURES_UAO_CODE_EN	"User Access Override"
#define RSC_FEATURES_VA_CODE_EN 	"Virtual Address range"
#define RSC_FEATURES_VHE_CODE_EN	"Virtualization Host Extensions"
#define RSC_FEAT_SECTION_MECH_CODE_EN	"Mitigation mechanisms"
#define RSC_FEAT_SECTION_SEC_CODE_EN	"Security Features"

#define RSC_TECHNOLOGIES_TITLE_CODE_EN	" Technologies "
#define RSC_TECHNOLOGIES_DCU_CODE_EN	"Data Cache Unit"
#define RSC_TECHNOLOGIES_ICU_CODE_EN	"Instruction Cache Unit"
#define RSC_TECHNOLOGIES_VM_CODE_EN	"Virtualization"
#define RSC_TECHNOLOGIES_IOMMU_CODE_EN	"I/O MMU"
#define RSC_TECHNOLOGIES_SMT_CODE_EN	"Simultaneous Multithreading"
#define RSC_TECHNOLOGIES_HYBRID_CODE_EN "big.LITTLE technology"
#define RSC_TECHNOLOGIES_HYPERV_CODE_EN "Hypervisor"
#define RSC_TECHNOLOGIES_VM_COMM_CODE_EN " Virtual Machine Hypervisor "
#define RSC_TECHNOLOGIES_IOMMU_COMM_CODE_EN " I/O MMU virtualization  "

#define RSC_PERF_MON_TITLE_CODE_EN	" Performance Monitoring "
#define RSC_PERF_CAPS_TITLE_CODE_EN	" Performance Capabilities "
#define RSC_VERSION_CODE_EN		"Version"
#define RSC_COUNTERS_CODE_EN		"Counters"
#define RSC_GENERAL_CTRS_CODE_EN	"General"
#define RSC_FIXED_CTRS_CODE_EN		"Fixed"
#define RSC_PERF_MON_UNIT_BIT_CODE_EN	"bits"
#define RSC_PERF_MON_CPPC_CODE_EN  "Collaborative Processor Performance Control"
#define RSC_PERF_MON_PCT_CODE_EN	"Processor Performance Control"
#define RSC_PERF_MON_PSS_CODE_EN	"Performance Supported States"
#define RSC_PERF_MON_PPC_CODE_EN	"Performance Present Capabilities"
#define RSC_PERF_MON_CPC_CODE_EN	"Continuous Performance Control"
#define RSC_PERF_MON_CST_CODE_EN	"ACPI Processor C-States"
#define RSC_PERF_MON_HWP_CODE_EN	"Hardware-Controlled Performance States"
#define RSC_PERF_MON_CORE_CSTATE_CODE_EN "Core C-States"
#define RSC_PERF_MON_CSTATE_BAR_CODE_EN "C-States Base Address"

#define RSC_PERF_MON_MONITOR_MWAIT_CODE_EN	"MONITOR/MWAIT"
#define RSC_PERF_MON_MWAIT_IDX_CSTATE_CODE_EN	"State index"
#define RSC_PERF_MON_MWAIT_SUB_CSTATE_CODE_EN	"Sub C-State"

#define RSC_PERF_MON_CORE_CYCLE_CODE_EN "Core Cycles Counter"
#define RSC_PERF_MON_INST_RET_CODE_EN	"Instructions Counter"

#define RSC_PERF_MON_PMC_COMM_CODE_EN	\
		" { Core performance monitoring, AMU-CG0NC, AMU-CG1NC } "

#define RSC_PERF_MON_CPPC_COMM_CODE_EN	" Firmware "

#define RSC_POWER_THERMAL_TITLE_CODE_EN " Power, Current & Thermal "
#define RSC_POWER_THERMAL_CPPC_CODE_EN	"CPPC Energy Preference"
#define RSC_POWER_THERMAL_TJMAX_CODE_EN "Temperature Offset:Junction"
#define RSC_POWER_THERMAL_DTS_CODE_EN	"Digital Thermal Sensor"
#define RSC_POWER_THERMAL_PLN_CODE_EN	"Power Limit Notification"
#define RSC_POWER_THERMAL_PTM_CODE_EN	"Package Thermal Management"
#define RSC_POWER_THERMAL_UNITS_CODE_EN "Units"
#define RSC_POWER_THERMAL_POWER_CODE_EN "Power"
#define RSC_POWER_THERMAL_ENERGY_CODE_EN "Energy"
#define RSC_POWER_THERMAL_WINDOW_CODE_EN "Window"
#define RSC_POWER_THERMAL_WATT_CODE_EN	"watt"
#define RSC_POWER_THERMAL_JOULE_CODE_EN "joule"
#define RSC_POWER_THERMAL_SECOND_CODE_EN "second"
#define RSC_POWER_THERMAL_TDP_CODE_EN	"Thermal Design Power"
#define RSC_POWER_THERMAL_MIN_CODE_EN	"Minimum Power"
#define RSC_POWER_THERMAL_MAX_CODE_EN	"Maximum Power"
#define RSC_POWER_THERMAL_TPL_CODE_EN	"Power Limit"
#define RSC_POWER_THERMAL_TW_CODE_EN	"Time Window"
#define RSC_POWER_THERMAL_EDC_CODE_EN	"Electrical Design Current"
#define RSC_POWER_THERMAL_TDC_CODE_EN	"Thermal Design Current"
#define RSC_POWER_THERMAL_POINT_CODE_EN "Thermal Point"

#define RSC_THERMAL_POINT_THRESHOLD_CODE_EN	"Threshold"
#define RSC_THERMAL_POINT_LIMIT_CODE_EN 	"Limit"
#define RSC_THERMAL_POINT_THRESHOLD_1_CODE_EN	"DTS Threshold #1"
#define RSC_THERMAL_POINT_THRESHOLD_2_CODE_EN	"DTS Threshold #2"
#define RSC_THERMAL_POINT_TRIP_LIMIT_CODE_EN	"Thermal Monitor Trip"
#define RSC_THERMAL_POINT_HTC_LIMIT_CODE_EN	"HTC Temperature Limit"
#define RSC_THERMAL_POINT_HTC_HYST_CODE_EN	"HTC Temperature Hysteresis"

#define RSC_THERMAL_OFFSET_TITLE_CODE_EN	" Thermal Offset "

#define RSC_KERNEL_TITLE_CODE_EN	" Kernel "
#define RSC_KERNEL_TOTAL_RAM_CODE_EN	"Total RAM"
#define RSC_KERNEL_SHARED_RAM_CODE_EN	"Shared RAM"
#define RSC_KERNEL_FREE_RAM_CODE_EN	"Free RAM"
#define RSC_KERNEL_BUFFER_RAM_CODE_EN	"Buffer RAM"
#define RSC_KERNEL_TOTAL_HIGH_CODE_EN	"Total High"
#define RSC_KERNEL_FREE_HIGH_CODE_EN	"Free High"
#define RSC_KERNEL_GOVERNOR_CODE_EN	"Governor"
#define RSC_KERNEL_FREQ_DRIVER_CODE_EN	"CPU-Freq driver"
#define RSC_KERNEL_IDLE_DRIVER_CODE_EN	"CPU-Idle driver"
#define RSC_KERNEL_CLOCK_SOURCE_CODE_EN "Clock Source"
#define RSC_KERNEL_RELEASE_CODE_EN	"Release"
#define RSC_KERNEL_VERSION_CODE_EN	"Version"
#define RSC_KERNEL_MACHINE_CODE_EN	"Machine"
#define RSC_KERNEL_MEMORY_CODE_EN	"Memory"
#define RSC_KERNEL_STATE_CODE_EN	"State"
#define RSC_KERNEL_POWER_CODE_EN	"Power"
#define RSC_KERNEL_LATENCY_CODE_EN	"Latency"
#define RSC_KERNEL_RESIDENCY_CODE_EN	"Residency"
#define RSC_KERNEL_LIMIT_CODE_EN	"Idle Limit"

#define RSC_TOPOLOGY_TITLE_CODE_EN	" Topology "

#define RSC_MEM_CTRL_TITLE_CODE_EN	" Memory Controller "
#define RSC_MEM_CTRL_SUBSECT1_0_CODE_EN 	"Contr"
#define RSC_MEM_CTRL_SUBSECT1_1_CODE_EN 	"oller"
#define RSC_MEM_CTRL_SUBSECT1_2_CODE_EN 	" #%-3u"
#define RSC_MEM_CTRL_SINGLE_CHA_0_CODE_EN	"Singl"
#define RSC_MEM_CTRL_SINGLE_CHA_1_CODE_EN	"e Cha"
#define RSC_MEM_CTRL_SINGLE_CHA_2_CODE_EN	"nnel "
#define RSC_MEM_CTRL_DUAL_CHA_0_CODE_EN 	" Dual"
#define RSC_MEM_CTRL_DUAL_CHA_1_CODE_EN 	" Chan"
#define RSC_MEM_CTRL_DUAL_CHA_2_CODE_EN 	"nel  "
#define RSC_MEM_CTRL_TRIPLE_CHA_0_CODE_EN	"Tripl"
#define RSC_MEM_CTRL_TRIPLE_CHA_1_CODE_EN	"e Cha"
#define RSC_MEM_CTRL_TRIPLE_CHA_2_CODE_EN	"nnel "
#define RSC_MEM_CTRL_QUAD_CHA_0_CODE_EN 	" Quad"
#define RSC_MEM_CTRL_QUAD_CHA_1_CODE_EN 	" Chan"
#define RSC_MEM_CTRL_QUAD_CHA_2_CODE_EN 	"nel  "
#define RSC_MEM_CTRL_SIX_CHA_0_CODE_EN		"  Six"
#define RSC_MEM_CTRL_SIX_CHA_1_CODE_EN		" Chan"
#define RSC_MEM_CTRL_SIX_CHA_2_CODE_EN		"nel  "
#define RSC_MEM_CTRL_EIGHT_CHA_0_CODE_EN	"Eight"
#define RSC_MEM_CTRL_EIGHT_CHA_1_CODE_EN	" Chan"
#define RSC_MEM_CTRL_EIGHT_CHA_2_CODE_EN	"nel  "
#define RSC_MEM_CTRL_DISABLED_0_CODE_EN 	"     "
#define RSC_MEM_CTRL_DISABLED_1_CODE_EN 	"Disab"
#define RSC_MEM_CTRL_DISABLED_2_CODE_EN 	"led  "
#define RSC_MEM_CTRL_BUS_RATE_0_CODE_EN 	" Bus "
#define RSC_MEM_CTRL_BUS_RATE_1_CODE_EN 	"Rate "
#define RSC_MEM_CTRL_BUS_SPEED_0_CODE_EN	" Bus "
#define RSC_MEM_CTRL_BUS_SPEED_1_CODE_EN	"Speed"
#define RSC_MEM_CTRL_RAM_STD_0_CODE_EN		"     "
#define RSC_MEM_CTRL_RAM_STD_1_CODE_EN		"     "
#define RSC_MEM_CTRL_RAM_STD_2_CODE_EN		"   LP"
#define RSC_MEM_CTRL_RAM_STD_3_CODE_EN		" REG "
#define RSC_MEM_CTRL_DRAM_DDR2_0_CODE_EN	"DDR2 "
#define RSC_MEM_CTRL_DRAM_DDR3_0_CODE_EN	"DDR3 "
#define RSC_MEM_CTRL_DRAM_DDR4_0_CODE_EN	"DDR4 "
#define RSC_MEM_CTRL_DRAM_DDR5_0_CODE_EN	"DDR5 "
#define RSC_MEM_CTRL_DRAM_SPEED_0_CODE_EN	"DRAM "
#define RSC_MEM_CTRL_DRAM_SPEED_1_CODE_EN	"Speed"
#define RSC_MEM_CTRL_SUBSECT2_0_CODE_EN 	" DIMM"
#define RSC_MEM_CTRL_SUBSECT2_1_CODE_EN 	" Geom"
#define RSC_MEM_CTRL_SUBSECT2_2_CODE_EN 	"etry "
#define RSC_MEM_CTRL_SUBSECT2_3_CODE_EN 	"for c"
#define RSC_MEM_CTRL_SUBSECT2_4_CODE_EN 	"hanne"
#define RSC_MEM_CTRL_SUBSECT2_5_CODE_EN 	"l #%-2u"
#define RSC_MEM_CTRL_DIMM_SLOT_CODE_EN		" Slot"
#define RSC_MEM_CTRL_DIMM_BANK_CODE_EN		" Bank"
#define RSC_MEM_CTRL_DIMM_RANK_CODE_EN		" Rank"
#define RSC_MEM_CTRL_DIMM_ROW_CODE_EN		"Rows "
#define RSC_MEM_CTRL_DIMM_COLUMN0_CODE_EN	"  Col"
#define RSC_MEM_CTRL_DIMM_COLUMN1_CODE_EN	"umns "
#define RSC_MEM_CTRL_DIMM_SIZE_0_CODE_EN	"   Me"
#define RSC_MEM_CTRL_DIMM_SIZE_1_CODE_EN	"mory "
#define RSC_MEM_CTRL_DIMM_SIZE_2_CODE_EN	"Size "
#define RSC_MEM_CTRL_DIMM_SIZE_3_CODE_EN	"(MB) "

#define RSC_DDR3_CL_COMM_CODE_EN	" tCL ( CAS Latency ) "
#define RSC_DDR3_RCD_COMM_CODE_EN	" tRCD ( Activate to CAS ) "
#define RSC_DDR3_RP_COMM_CODE_EN	" tRP ( RAS Precharge to Activate ) "
#define RSC_DDR3_RAS_COMM_CODE_EN	" tRAS ( Activate to Precharge ) "
#define RSC_DDR3_RRD_COMM_CODE_EN   " tRRD ( Activate to Activate, Same Rank ) "
#define RSC_DDR3_RFC_COMM_CODE_EN	" tRFC ( Refresh to Refresh ) "
#define RSC_DDR3_WR_COMM_CODE_EN	" tWR ( Write Recovery ) "
#define RSC_DDR3_RTP_COMM_CODE_EN	" tRTPr ( Read CAS to Precharge ) "
#define RSC_DDR3_WTP_COMM_CODE_EN	" tWTPr ( Write CAS to Precharge ) "
#define RSC_DDR3_FAW_COMM_CODE_EN	" tFAW ( Four Activate Window ) "
#define RSC_DDR3_B2B_COMM_CODE_EN	" tCCD ( CAS commands spacing ) "
#define RSC_DDR3_CWL_COMM_CODE_EN	" tCWL ( CAS Write Latency ) "
#define RSC_DDR3_CMD_COMM_CODE_EN	" CMD ( Command Rate ) "
#define RSC_DDR3_REFI_COMM_CODE_EN	" tREFI ( Refresh Interval ) "

#define RSC_DDR3_DDWRTRD_COMM_CODE_EN	\
				" tddWrTRd ( Write to Read, Different DIMM ) "

#define RSC_DDR3_DRWRTRD_COMM_CODE_EN	\
		" tdrWrTRd ( Write to Read, Different Rank, Same DIMM ) "

#define RSC_DDR3_SRWRTRD_COMM_CODE_EN  " tsrWrTRd ( Write to Read, Same Rank ) "

#define RSC_DDR3_DDRDTWR_COMM_CODE_EN	\
				" tddRdTWr ( Read to Write, Different DIMM ) "

#define RSC_DDR3_DRRDTWR_COMM_CODE_EN	\
		" tdrRdTWr ( Read to Write, Different Rank, Same DIMM ) "

#define RSC_DDR3_SRRDTWR_COMM_CODE_EN  " tsrRdTWr ( Read to Write, Same Rank ) "

#define RSC_DDR3_DDRDTRD_COMM_CODE_EN	\
				" tddRdTRd ( Read to Read, Different DIMM ) "

#define RSC_DDR3_DRRDTRD_COMM_CODE_EN	\
			" tdrRdTRd ( Read to Read, Different Rank, Same DIMM ) "

#define RSC_DDR3_SRRDTRD_COMM_CODE_EN	" tsrRdTRd ( Read to Read, Same Rank ) "

#define RSC_DDR3_DDWRTWR_COMM_CODE_EN	\
				" tddWrTWr ( Write to Write, Different DIMM ) "

#define RSC_DDR3_DRWRTWR_COMM_CODE_EN	\
		" tdrWrTWr ( Write to Write, Different Rank, Same DIMM ) "

#define RSC_DDR3_SRWRTWR_COMM_CODE_EN " tsrWrTWr ( Write to Write, Same Rank ) "

#define RSC_DDR3_XS_COMM_CODE_EN	" tXS ( Exit Self refresh to commands" \
					" not requiring a locked DLL ) "

#define RSC_DDR3_XP_COMM_CODE_EN	" tXP ( Exit Power-down with DLL on " \
					"to any valid command ) "

#define RSC_DDR3_CKE_COMM_CODE_EN	" tCKE ( ClocK Enable ) "
#define RSC_DDR3_ECC_COMM_CODE_EN	" ECC ( Error Correcting Code ) "

#define RSC_DDR4_RCD_R_COMM_CODE_EN	" tRCD_R ( Activate to Read CAS ) "
#define RSC_DDR4_RCD_W_COMM_CODE_EN	" tRCD_W ( Activate to Write CAS ) "
#define RSC_DDR4_RDRD_SCL_COMM_CODE_EN	" tRDRD ( Read to Read, Same Bank ) "
#define RSC_DDR4_RDRD_SC_COMM_CODE_EN " tRDRD ( Read to Read, Different Bank ) "
#define RSC_DDR4_RDRD_SD_COMM_CODE_EN " tRDRD ( Read to Read, Different Rank ) "
#define RSC_DDR4_RDRD_DD_COMM_CODE_EN " tRDRD ( Read to Read, Different DIMM ) "
#define RSC_DDR4_RDWR_SCL_COMM_CODE_EN	" tRDWR ( Read to Write, Same Bank ) "

#define RSC_DDR4_RDWR_SC_COMM_CODE_EN	\
				" tRDWR ( Read to Write, Different Bank ) "

#define RSC_DDR4_RDWR_SD_COMM_CODE_EN	\
				" tRDWR ( Read to Write, Different Rank ) "

#define RSC_DDR4_RDWR_DD_COMM_CODE_EN	\
				" tRDWR ( Read to Write, Different DIMM ) "

#define RSC_DDR4_WRRD_SCL_COMM_CODE_EN	" tWRRD ( Write to Read, Same Bank ) "

#define RSC_DDR4_WRRD_SC_COMM_CODE_EN	\
				" tWRRD ( Write to Read, Different Bank ) "

#define RSC_DDR4_WRRD_SD_COMM_CODE_EN	\
				" tWRRD ( Write to Read, Different Rank ) "

#define RSC_DDR4_WRRD_DD_COMM_CODE_EN	\
				" tWRRD ( Write to Read, Different DIMM ) "

#define RSC_DDR4_WRWR_SCL_COMM_CODE_EN	" tWRWR ( Write to Write, Same Bank ) "

#define RSC_DDR4_WRWR_SC_COMM_CODE_EN	\
				" tWRWR ( Write to Write, Different Bank ) "

#define RSC_DDR4_WRWR_SD_COMM_CODE_EN	\
				" tWRWR ( Write to Write, Different Rank ) "

#define RSC_DDR4_WRWR_DD_COMM_CODE_EN	\
				" tWRWR ( Write to Write, Different DIMM ) "

#define RSC_DDR4_RRD_S_COMM_CODE_EN \
		" tRRD_S ( Activate to Activate, Different Bank Group ) "

#define RSC_DDR4_RRD_L_COMM_CODE_EN \
			" tRRD_L ( Activate to Activate, Same Bank Group ) "

#define RSC_DDR4_CPDED_COMM_CODE_EN " tCPDED ( Command Pass Disable Delay ) "
#define RSC_DDR4_GEAR_COMM_CODE_EN	" GEAR ( Clock Gear Mode ) "

#define RSC_DDR4_ZEN_RC_COMM_CODE_EN	" tRC ( Activate to Activate ) "

#define RSC_DDR4_ZEN_WTR_S_COMM_CODE_EN \
			" tWTR_S ( Write to Read, Different Bank Group ) "

#define RSC_DDR4_ZEN_WTR_L_COMM_CODE_EN \
				" tWTR_L ( Write to Read, Same Bank Group ) "

#define RSC_DDR4_ZEN_RDRD_SCL_COMM_CODE_EN \
				" tRDRD[SCL] ( Read to Read, Same Bank Group ) "

#define RSC_DDR4_ZEN_WRWR_SCL_COMM_CODE_EN \
			" tWRWR[SCL] ( Write to Write, Same Bank Group ) "

#define RSC_DDR4_ZEN_RTP_COMM_CODE_EN	" tRTP ( Read To Precharge ) "
#define RSC_DDR4_ZEN_RDWR_COMM_CODE_EN	" tRDWR ( Read Write Command Spacing ) "
#define RSC_DDR4_ZEN_WRRD_COMM_CODE_EN	" tWRRD ( Write Read Command Spacing ) "

#define RSC_DDR4_ZEN_WRWR_SC_COMM_CODE_EN \
				" tWRWR[SC] ( tWRWR, Different Bank Group ) "

#define RSC_DDR4_ZEN_WRWR_SD_COMM_CODE_EN \
					" tWRWR[SD] ( tWRWR, Different Rank) "

#define RSC_DDR4_ZEN_WRWR_DD_COMM_CODE_EN \
					" tWRWR[DD] ( tWRWR, Different DIMM ) "

#define RSC_DDR4_ZEN_RDRD_SC_COMM_CODE_EN \
				" tRDRD[SC] ( tRDRD, Different Bank Group ) "

#define RSC_DDR4_ZEN_RDRD_SD_COMM_CODE_EN \
					" tRDRD[SD] ( tRDRD, Different Rank ) "

#define RSC_DDR4_ZEN_RDRD_DD_COMM_CODE_EN \
					" tRDRD[DD] ( tRDRD, Different DIMM ) "

#define RSC_DDR4_ZEN_RTR_DLR_COMM_CODE_EN \
				" tRTR[DLR] ( tRTR, Different Logical Rank ) "

#define RSC_DDR4_ZEN_WTW_DLR_COMM_CODE_EN \
				" tWTW[DLR] ( tWTW, Different Logical Rank ) "

#define RSC_DDR4_ZEN_WTR_DLR_COMM_CODE_EN \
				" tWTR[DLR] ( tWTR, Different Logical Rank ) "

#define RSC_DDR4_ZEN_RRD_DLR_COMM_CODE_EN \
				" tRRD[DLR] ( tRRD, Different Logical Rank ) "

#define RSC_DDR4_ZEN_RFC1_COMM_CODE_EN " tRFC1 ( Refresh to Refresh, 1X mode ) "
#define RSC_DDR4_ZEN_RFC2_COMM_CODE_EN " tRFC2 ( Refresh to Refresh, 2X mode ) "
#define RSC_DDR4_ZEN_RFC4_COMM_CODE_EN " tRFC4 ( Refresh to Refresh, 4X mode ) "
#define RSC_DDR4_ZEN_RCPB_COMM_CODE_EN	" tRCPB ( Row Cycle Time, Per-Bank ) "

#define RSC_DDR4_ZEN_RPPB_COMM_CODE_EN \
				" tRPPB ( Row Precharge Time, Per-Bank ) "

#define RSC_DDR4_ZEN_BGS_COMM_CODE_EN	" BGS ( BankGroupSwap ) "

#define RSC_DDR4_ZEN_BGS_ALT_COMM_CODE_EN \
					" BGS Alt ( BankGroupSwap Alternate ) "

#define RSC_DDR4_ZEN_BAN_COMM_CODE_EN	" tBAN ( Timing Ban, RTR | WTW ) "
#define RSC_DDR4_ZEN_RCPAGE_COMM_CODE_EN " tRCPage ( Row Cycle Page Time ) "
#define RSC_DDR4_ZEN_GDM_COMM_CODE_EN	" GDM ( Gear Down Mode ) "

#define RSC_DDR4_ZEN_MRD_COMM_CODE_EN	\
			" tMRD ( MRS to another MRS command, Same CS ) "

#define RSC_DDR4_ZEN_MOD_COMM_CODE_EN	\
			" tMOD ( MRS to another non-MRS command, Same CS ) "

#define RSC_DDR4_ZEN_MRD_PDA_COMM_CODE_EN \
			" tMRD_PDA ( MRS PDA to another MRS command, Same CS ) "

#define RSC_DDR4_ZEN_MOD_PDA_COMM_CODE_EN \
		" tMOD_PDA ( MRS PDA to another non-MRS command, Same CS ) "

#define RSC_DDR4_ZEN_WRMPR_COMM_CODE_EN \
			" tWRMPR ( Number of clocks greater than tMOD ) "

#define RSC_DDR4_ZEN_STAG_COMM_CODE_EN	\
		" tSTAG ( Min timing between REF commands, Different CS ) "

#define RSC_DDR4_ZEN_PDM_COMM_CODE_EN	" PDM ( DRAM Power Down Mode ) " \
					" [ Aggressive:Full|Partial:Enable ] "

#define RSC_DDR4_ZEN_RDDATA_COMM_CODE_EN \
			" tRDDATA ( Read command to dfi_rddata_en delay ) "

#define RSC_DDR4_ZEN_PHYWRD_COMM_CODE_EN \
			" tPHYWRD ( dfi_wrdata_en to dfi_wrdata delay ) "

#define RSC_DDR4_ZEN_PHYWRL_COMM_CODE_EN \
			" tPHYWRL ( Write command to dfi_wrdata_en delay ) "

#define RSC_DDR4_ZEN_PHYRDL_COMM_CODE_EN \
			" tPHYRDL ( dfi_rddata_en to dfi_rddata_vld delay ) "

#define RSC_DDR5_ZEN_RFC_SB_COMM_CODE_EN \
				" tRFCsb ( Refresh Recovery, Same Bank ) "

#define RSC_TASKS_SORTBY_STATE_CODE_EN		" State    "
#define RSC_TASKS_SORTBY_RTIME_CODE_EN		" RunTime  "
#define RSC_TASKS_SORTBY_UTIME_CODE_EN		" UserTime "
#define RSC_TASKS_SORTBY_STIME_CODE_EN		" SysTime  "
#define RSC_TASKS_SORTBY_PID_CODE_EN		" PID      "
#define RSC_TASKS_SORTBY_COMM_CODE_EN		" Command  "

#define RSC_MENU_ITEM_MENU_CODE_EN		"     [F2] Menu          "
#define RSC_MENU_ITEM_VIEW_CODE_EN		"     [F3] View          "
#define RSC_MENU_ITEM_WINDOW_CODE_EN		"    [F4] Window         "
#define RSC_MENU_ITEM_SPACER_CODE_EN		"\x020\x020\x020"
#define RSC_MENU_ITEM_DATE_TIME_CODE_EN 	"%x %k:%M:%S"
#define RSC_MENU_ITEM_FULL_TIME_CODE_EN 	"%k:%M:%S"
#define RSC_MENU_ITEM_TINY_TIME_CODE_EN 	"%k:%M"
#define RSC_MENU_ITEM_SETTINGS_CODE_EN		" Settings           [s] "
#define RSC_MENU_ITEM_SMBIOS_CODE_EN		" SMBIOS data        [B] "
#define RSC_MENU_ITEM_KERNEL_CODE_EN		" Kernel data        [k] "
#define RSC_MENU_ITEM_HOTPLUG_CODE_EN		" HotPlug CPU        [#] "
#define RSC_MENU_ITEM_TOOLS_CODE_EN		" Tools              [O] "
#define RSC_MENU_ITEM_THEME_CODE_EN		" Theme              [E] "
#define RSC_MENU_ITEM_ABOUT_CODE_EN		" About              [a] "
#define RSC_MENU_ITEM_HELP_CODE_EN		" Help               [h] "
#define RSC_MENU_ITEM_KEYS_CODE_EN		" Shortcuts         [F1] "
#define RSC_MENU_ITEM_LANG_CODE_EN		" Languages          [L] "
#define RSC_MENU_ITEM_QUIT_CODE_EN		" Quit        [Ctrl]+[x] "
#define RSC_MENU_ITEM_DASHBOARD_CODE_EN 	" Dashboard          [d] "
#define RSC_MENU_ITEM_FREQUENCY_CODE_EN 	" Frequency          [f] "
#define RSC_MENU_ITEM_INST_CYCLES_CODE_EN	" Inst cycles        [i] "
#define RSC_MENU_ITEM_CORE_CYCLES_CODE_EN	" Core cycles        [c] "
#define RSC_MENU_ITEM_IDLE_STATES_CODE_EN	" Idle C-States      [l] "
#define RSC_MENU_ITEM_PKG_CYCLES_CODE_EN	" Package cycles     [g] "
#define RSC_MENU_ITEM_TASKS_MON_CODE_EN 	" Tasks Monitoring   [x] "
#define RSC_MENU_ITEM_SYS_INTER_CODE_EN 	" System Interrupts  [q] "
#define RSC_MENU_ITEM_SENSORS_CODE_EN		" Sensors            [C] "
#define RSC_MENU_ITEM_VOLTAGE_CODE_EN		"   Voltage          [V] "
#define RSC_MENU_ITEM_POWER_CODE_EN		"   Power            [W] "
#define RSC_MENU_ITEM_SLICE_CTRS_CODE_EN	" Slice counters     [T] "
#define RSC_MENU_ITEM_CUSTOM_CODE_EN		" Custom view        [y] "
#define RSC_MENU_ITEM_PROCESSOR_CODE_EN 	" Processor          [p] "
#define RSC_MENU_ITEM_TOPOLOGY_CODE_EN		" Topology           [m] "
#define RSC_MENU_ITEM_FEATURES_CODE_EN		" Features           [e] "
#define RSC_MENU_ITEM_ISA_EXT_CODE_EN		" ISA Extensions     [I] "
#define RSC_MENU_ITEM_TECH_CODE_EN		" Technologies       [t] "
#define RSC_MENU_ITEM_PERF_MON_CODE_EN		" Perf. Monitoring   [o] "
#define RSC_MENU_ITEM_PERF_CAPS_CODE_EN 	" Perf. Capabilities [z] "
#define RSC_MENU_ITEM_POW_THERM_CODE_EN 	" Power & Thermal    [w] "
#define RSC_MENU_ITEM_SYS_REGS_CODE_EN		" System Registers   [R] "
#define RSC_MENU_ITEM_MEM_CTRL_CODE_EN		" Memory Controller  [M] "
#define RSC_MENU_ITEM_EVENTS_CODE_EN		" Processor Events   [H] "

#define RSC_SETTINGS_TITLE_CODE_EN	    " Settings "
#define RSC_SETTINGS_DAEMON_CODE_EN	    " Daemon gate                    "
#define RSC_SETTINGS_INTERVAL_CODE_EN	    " Interval(ms)            <    > "
#define RSC_SETTINGS_SYS_TICK_CODE_EN	    " Sys. Tick(ms)                  "
#define RSC_SETTINGS_POLL_WAIT_CODE_EN	    " Poll Wait(ms)                  "
#define RSC_SETTINGS_RING_WAIT_CODE_EN	    " Ring Wait(ms)                  "
#define RSC_SETTINGS_CHILD_WAIT_CODE_EN     " Child Wait(ms)                 "
#define RSC_SETTINGS_SLICE_WAIT_CODE_EN     " Slice Wait(ms)                 "
#define RSC_SETTINGS_RECORDER_CODE_EN	    " Duration(sec)           <    > "
#define RSC_SETTINGS_AUTO_CLOCK_CODE_EN     " Auto Clock               <   > "
#define RSC_SETTINGS_EXPERIMENTAL_CODE_EN   " Experimental             <   > "
#define RSC_SETTINGS_CPU_HOTPLUG_CODE_EN    " CPU Hot-Plug             [   ] "
#define RSC_SETTINGS_PCI_ENABLED_CODE_EN    " PCI enablement           [   ] "
#define RSC_SETTINGS_NMI_REGISTERED_CODE_EN " NMI registered           <   > "
#define RSC_SETTINGS_CPUIDLE_REGISTERED_CODE_EN \
					    " CPU-IDLE driver          <   > "

#define RSC_SETTINGS_CPUFREQ_REGISTERED_CODE_EN \
					    " CPU-FREQ driver          <   > "

#define RSC_SETTINGS_GOVERNOR_REGISTERED_CODE_EN \
					    " Governor driver          <   > "

#define RSC_SETTINGS_CS_REGISTERED_CODE_EN  " Clock Source             <   > "
#define RSC_SETTINGS_THERMAL_SCOPE_CODE_EN  " Thermal scope           <    > "
#define RSC_SETTINGS_VOLTAGE_SCOPE_CODE_EN  " Voltage scope           <    > "
#define RSC_SETTINGS_POWER_SCOPE_CODE_EN    " Power scope             <    > "
#define RSC_SETTINGS_IDLE_ROUTE_CODE_EN     " CPU-IDLE route                 "

#define RSC_HELP_TITLE_CODE_EN		" Help "
#define RSC_HELP_KEY_ESCAPE_CODE_EN	" [Escape]         "
#define RSC_HELP_KEY_SHIFT_TAB_CODE_EN	" [Shift]+[Tab]    "
#define RSC_HELP_KEY_TAB_CODE_EN	" [Tab]            "
#define RSC_HELP_KEY_UP_CODE_EN 	"       [Up]       "
#define RSC_HELP_KEY_LEFT_RIGHT_CODE_EN " [Left]    [Right]"
#define RSC_HELP_KEY_DOWN_CODE_EN	"      [Down]      "
#define RSC_HELP_KEY_END_CODE_EN	" [End]            "
#define RSC_HELP_KEY_HOME_CODE_EN	" [Home]           "
#define RSC_HELP_KEY_ENTER_CODE_EN	" [Enter]          "
#define RSC_HELP_KEY_PAGE_UP_CODE_EN	" [Page-Up]        "
#define RSC_HELP_KEY_PAGE_DOWN_CODE_EN	" [Page-Dw]        "
#define RSC_HELP_KEY_MINUS_CODE_EN	" [Minus]          "
#define RSC_HELP_KEY_PLUS_CODE_EN	" [Plus]           "
#define RSC_HELP_KEY_MENU_CODE_EN	" [F2]             "
#define RSC_HELP_MENU_CODE_EN		"             Menu "
#define RSC_HELP_CLOSE_WINDOW_CODE_EN	"     Close window "
#define RSC_HELP_PREV_WINDOW_CODE_EN	"  Previous window "
#define RSC_HELP_NEXT_WINDOW_CODE_EN	"      Next window "
#define RSC_HELP_KEY_SHIFT_GR1_CODE_EN	"        [s]       "
#define RSC_HELP_KEY_SHIFT_GR2_CODE_EN	" [a|q]  [x]  [d] +"
#define RSC_HELP_MOVE_WINDOW_CODE_EN	" [Shift] Move Win "
#define RSC_HELP_KEY_ALT_GR3_CODE_EN	"                 +"
#define RSC_HELP_SIZE_WINDOW_CODE_EN	"  [Alt]  Size Win "
#define RSC_HELP_MOVE_SELECT_CODE_EN	"   Move selection "
#define RSC_HELP_LAST_CELL_CODE_EN	"        Last cell "
#define RSC_HELP_FIRST_CELL_CODE_EN	"       First cell "
#define RSC_HELP_TRIGGER_SELECT_CODE_EN "Trigger selection "
#define RSC_HELP_PREV_PAGE_CODE_EN	"    Previous page "
#define RSC_HELP_NEXT_PAGE_CODE_EN	"        Next page "
#define RSC_HELP_SCROLL_DOWN_CODE_EN	"  Scroll CPU down "
#define RSC_HELP_SCROLL_UP_CODE_EN	"    Scroll CPU up "

#define RSC_ADV_HELP_TITLE_CODE_EN	" Shortcuts "
#define RSC_ADV_HELP_SECT_FREQ_CODE_EN	" Frequency view:                      "
#define RSC_ADV_HELP_ITEM_AVG_CODE_EN	" %         Averages | Package C-States"
#define RSC_ADV_HELP_SECT_TASK_CODE_EN	" Task Monitoring view:                "
#define RSC_ADV_HELP_ITEM_ORDER_CODE_EN " b                Sorting tasks order "
#define RSC_ADV_HELP_ITEM_RST_CODE_EN	" N                Reset task tracking "
#define RSC_ADV_HELP_ITEM_SEL_CODE_EN	" n               Select task tracking "
#define RSC_ADV_HELP_ITEM_REV_CODE_EN	" r              Reverse tasks sorting "
#define RSC_ADV_HELP_ITEM_HIDE_CODE_EN	" v          Show | Hide Kernel values "
#define RSC_ADV_HELP_SECT_ANY_CODE_EN	" Any view:                            "
#define RSC_ADV_HELP_ITEM_POWER_CODE_EN " $            Energy in Joule or Watt "
#define RSC_ADV_HELP_ITEM_TOP_CODE_EN	" .             Top frequency or Usage "
#define RSC_ADV_HELP_ITEM_UPD_CODE_EN	" *           Refresh CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_START_CODE_EN " {             Start CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_STOP_CODE_EN	" }              Stop CoreFreq Machine "
#define RSC_ADV_HELP_ITEM_TOOLS_CODE_EN " F10            Stop tools processing "
#define RSC_ADV_HELP_ITEM_GO_UP_CODE_EN "  Up  PgUp                     Scroll "
#define RSC_ADV_HELP_ITEM_GO_DW_CODE_EN " Down PgDw                       CPU  "
#define RSC_ADV_HELP_ITEM_TERMINAL_CODE_EN \
					" Terminal:                            "

#define RSC_ADV_HELP_ITEM_PRT_SCR_CODE_EN \
					" [Ctrl]+[p]                Screenshot "

#define RSC_ADV_HELP_ITEM_REC_SCR_CODE_EN \
					" [Alt]+[p]                 Screencast "

#define RSC_ADV_HELP_ITEM_FAHR_CELS_CODE_EN \
					" F              Fahrenheit or Celsius "

#define RSC_ADV_HELP_ITEM_SYSGATE_CODE_EN \
					" G               Toggle SysGate state "

#define RSC_ADV_HELP_ITEM_PROC_EVENT_CODE_EN \
					" H            Manage Processor Events "

#define RSC_ADV_HELP_ITEM_SECRET_CODE_EN \
					" Y            Show | Hide Secret Data "

#define RSC_TURBO_CLOCK_TITLE_CODE_EN	" Turbo Clock %1dC "
#define RSC_RATIO_CLOCK_TITLE_CODE_EN	" %s Clock Ratio "
#define RSC_UNCORE_CLOCK_TITLE_CODE_EN	" %s Clock Uncore "

#define RSC_SELECT_CPU_TITLE_CODE_EN	" CPU   Pkg  Core Thread Scheduled "
#define RSC_SELECT_FREQ_TITLE_CODE_EN	" CPU   Pkg  Core Thread "	\
					"  Frequency   Ratio "

#define RSC_BOX_DISABLE_COND0_CODE_EN	"               Disable              "
#define RSC_BOX_DISABLE_COND1_CODE_EN	"           <   Disable  >           "
#define RSC_BOX_ENABLE_COND0_CODE_EN	"               Enable               "
#define RSC_BOX_ENABLE_COND1_CODE_EN	"           <   Enable   >           "

#define RSC_BOX_INTERVAL_TITLE_CODE_EN	"Interval"
#define RSC_BOX_AUTO_CLOCK_TITLE_CODE_EN " Auto Clock "
#define RSC_BOX_MODE_TITLE_CODE_EN	" Experimental "

#define RSC_BOX_MODE_DESC_CODE_EN	"       CoreFreq Operation Mode       "
#define RSC_BOX_HWP_DESC_CODE_EN	"   Hardware-Controlled Performance  "
#define RSC_BOX_FMW_DESC_CODE_EN	"   Firmware Controlled Performance  "

#define RSC_BOX_NOMINAL_MODE_COND0_CODE_EN \
					"       Nominal operating mode       "

#define RSC_BOX_NOMINAL_MODE_COND1_CODE_EN \
					"     < Nominal operating mode >     "

#define RSC_BOX_EXPERIMENT_MODE_COND0_CODE_EN \
					"     Experimental operating mode    "

#define RSC_BOX_EXPERIMENT_MODE_COND1_CODE_EN \
					"   < Experimental operating mode >  "

#define RSC_BOX_INTERRUPT_TITLE_CODE_EN 	" NMI Interrupts "
#define RSC_BOX_CPU_IDLE_TITLE_CODE_EN		" CPU-IDLE driver "
#define RSC_BOX_CPU_FREQ_TITLE_CODE_EN		" CPU-FREQ driver "
#define RSC_BOX_GOVERNOR_TITLE_CODE_EN		" Governor driver "
#define RSC_BOX_CLOCK_SOURCE_TITLE_CODE_EN	" Clock Source "

#define RSC_BOX_OPS_REGISTER_COND0_CODE_EN \
					"              Register              "

#define RSC_BOX_OPS_REGISTER_COND1_CODE_EN \
					"            < Register >            "

#define RSC_BOX_OPS_UNREGISTER_COND0_CODE_EN \
					"             Unregister             "

#define RSC_BOX_OPS_UNREGISTER_COND1_CODE_EN \
					"           < Unregister >           "

#define RSC_BOX_EVENT_TITLE_CODE_EN \
		" DTS ------------ Core ----------- GFX ------------ Ring "

#define RSC_BOX_EVENT_SPACE_CODE_EN		"                 "
#define RSC_BOX_EVENT_THERMAL_SENSOR_CODE_EN	" Thermal Sensor  "
#define RSC_BOX_EVENT_PROCHOT_STS_CODE_EN	" PROCHOT         "
#define RSC_BOX_EVENT_CRITICAL_TEMP_CODE_EN	" Critical Temp.  "
#define RSC_BOX_EVENT_THOLD1_STS_CODE_EN	" Th. Threshold1  "
#define RSC_BOX_EVENT_THOLD2_STS_CODE_EN	" Th. Threshold2  "
#define RSC_BOX_EVENT_POWER_LIMIT_CODE_EN	" Power Limit.    "
#define RSC_BOX_EVENT_CURRENT_LIMIT_CODE_EN	" Current Limit.  "
#define RSC_BOX_EVENT_CROSS_DOM_LIMIT_CODE_EN	" XDomain Limit.  "
#define RSC_BOX_EVENT_RESIDENCY_CODE_EN 	" Residency       "
#define RSC_BOX_EVENT_AVG_THERMAL_CODE_EN	" Avg Thermal     "
#define RSC_BOX_EVENT_VR_THERMAL_CODE_EN	" VR Thermal      "
#define RSC_BOX_EVENT_VR_TDC_CODE_EN		" VR TDC          "
#define RSC_BOX_EVENT_POWER_PL1_CODE_EN 	" Package PL1     "
#define RSC_BOX_EVENT_POWER_PL2_CODE_EN 	" Package PL2     "
#define RSC_BOX_EVENT_ELECTRICAL_CODE_EN	" Electrical EDP  "
#define RSC_BOX_EVENT_INEFFICIENCY_CODE_EN	" Inefficiency    "
#define RSC_BOX_EVENT_MAX_TURBO_CODE_EN 	" Max Turbo       "
#define RSC_BOX_EVENT_TURBO_ATTEN_CODE_EN	" Turbo Atten.    "
#define RSC_BOX_EVENT_THERMAL_TVB_CODE_EN	" Thermal TVB     "
#define RSC_BOX_EVENT_ALL_OF_THEM_CODE_EN	"<   Clear All   >"

#define RSC_BOX_POWER_POLICY_TITLE_CODE_EN	" Energy Policy "
#define RSC_BOX_POWER_POLICY_LOW_CODE_EN	"            0       LOW "
#define RSC_BOX_POWER_POLICY_HIGH_CODE_EN	"           15      HIGH "

#define RSC_BOX_HWP_POLICY_MIN_CODE_EN		"         Minimum:0      "
#define RSC_BOX_HWP_POLICY_020_CODE_EN		"                32      "
#define RSC_BOX_HWP_POLICY_040_CODE_EN		"                64      "
#define RSC_BOX_HWP_POLICY_060_CODE_EN		"                96      "
#define RSC_BOX_HWP_POLICY_MED_CODE_EN		"        Medium:128      "
#define RSC_BOX_HWP_POLICY_0A0_CODE_EN		"               160      "
#define RSC_BOX_HWP_POLICY_PWR_CODE_EN		"         Power:192      "
#define RSC_BOX_HWP_POLICY_0E0_CODE_EN		"               224      "
#define RSC_BOX_HWP_POLICY_MAX_CODE_EN		"       Maximum:255      "

#define RSC_BOX_TOOLS_TITLE_CODE_EN		" Tools "
#define RSC_BOX_TOOLS_STOP_BURN_CODE_EN 	"            STOP           "
#define RSC_BOX_TOOLS_ATOMIC_BURN_CODE_EN	"        Atomic Burn        "
#define RSC_BOX_TOOLS_CRC32_BURN_CODE_EN	"       CRC32 Compute       "
#define RSC_BOX_TOOLS_CONIC_BURN_CODE_EN	"   <   Conic Compute   >   "
#define RSC_BOX_TOOLS_RANDOM_CPU_CODE_EN	"      Turbo Random CPU     "
#define RSC_BOX_TOOLS_ROUND_ROBIN_CPU_CODE_EN	"      Turbo Round Robin    "
#define RSC_BOX_TOOLS_USER_CPU_CODE_EN		"    Turbo < Select CPU >   "
#define RSC_BOX_TOOLS_MONTE_CARLO_CODE_EN	" Estimate PI / Monte Carlo "

#define RSC_BOX_CONIC_TITLE_CODE_EN		" Conic variations "
#define RSC_BOX_CONIC_ITEM_1_CODE_EN		"         Ellipsoid         "
#define RSC_BOX_CONIC_ITEM_2_CODE_EN		" Hyperboloid of one sheet  "
#define RSC_BOX_CONIC_ITEM_3_CODE_EN		" Hyperboloid of two sheets "
#define RSC_BOX_CONIC_ITEM_4_CODE_EN		"    Elliptical cylinder    "
#define RSC_BOX_CONIC_ITEM_5_CODE_EN		"    Hyperbolic cylinder    "
#define RSC_BOX_CONIC_ITEM_6_CODE_EN		"    Two parallel planes    "

#define RSC_BOX_LANG_TITLE_CODE_EN		" Languages "
#define RSC_BOX_LANG_ENGLISH_CODE_EN		"     English     "
#define RSC_BOX_LANG_FRENCH_CODE_EN		"     French      "
#define RSC_BOX_LANG_BLANK_CODE 		"                 "

#define RSC_BOX_THEME_TITLE_CODE_EN		" Themes "
#define RSC_BOX_THEME_BLANK_CODE			\
"                                                                        "

#define RSC_BOX_SCOPE_THERMAL_TITLE_CODE_EN	" Thermal scope "
#define RSC_BOX_SCOPE_VOLTAGE_TITLE_CODE_EN	" Voltage scope "
#define RSC_BOX_SCOPE_POWER_TITLE_CODE_EN	" Power scope "
#define RSC_BOX_SCOPE_NONE_CODE_EN		"       None       "
#define RSC_BOX_SCOPE_THREAD_CODE_EN		"      Thread      "
#define RSC_BOX_SCOPE_CORE_CODE_EN		"       Core       "
#define RSC_BOX_SCOPE_PACKAGE_CODE_EN		"      Package     "

#define RSC_BOX_CFG_TDP_TITLE_CODE_EN		" Config TDP "
#define RSC_BOX_CFG_TDP_DESC_CODE_EN		" System BIOS limit "
#define RSC_BOX_CFG_TDP_LVL0_CODE_EN		"      Level 0      "
#define RSC_BOX_CFG_TDP_LVL1_CODE_EN		"      Level 1      "
#define RSC_BOX_CFG_TDP_LVL2_CODE_EN		"      Level 2      "

#define RSC_BOX_TDP_PKG_TITLE_CODE_EN		" TDP Package "
#define RSC_BOX_TDP_CORES_TITLE_CODE_EN 	" TDP Core "
#define RSC_BOX_TDP_UNCORE_TITLE_CODE_EN	" TDP Uncore "
#define RSC_BOX_TDP_RAM_TITLE_CODE_EN		" TDP DRAM "
#define RSC_BOX_TDP_PLATFORM_TITLE_CODE_EN	" TDP Platform "
#define RSC_BOX_PL1_DESC_CODE_EN	"          PL1 Power Limit           "
#define RSC_BOX_PL2_DESC_CODE_EN	"          PL2 Power Limit           "

#define RSC_BOX_TDC_TITLE_CODE_EN		" TDC Current "
#define RSC_BOX_TDC_DESC_CODE_EN	"         TDC Current Limit          "

#define RSC_ERROR_CMD_SYNTAX_CODE_EN					\
		"CoreFreq."						\
		"  Copyright (C) 2015-2024 CYRIL COURTIAT\n\n"	\
		"Usage:\t%s [-Option <argument>] [-Command <argument>]\n"\
		"\n    Interface options\n"				\
		"\t-Oa\tAbsolute frequency\n"				\
		"\t-Op\tShow Package C-States\n"			\
		"\t-Ok\tMemory unit in kilobyte\n"			\
		"\t-Om\tMemory unit in megabyte\n"			\
		"\t-Og\tMemory unit in gigabyte\n"			\
		"\t-OW\tToggle Energy units\n"				\
		"\t-OF\tTemperature in Fahrenheit\n"			\
		"\t-OJ #\tSMBIOS string index number\n" 		\
		"\t-OE #\tColor theme index number\n"			\
		"\t-OY\tShow Secret Data\n"				\
		"\n    Command options\n"				\
		"\t-t <v>\tShow Top (default) with optional <v>iew:\n"	\
		"\t\t{\tfrequency, instructions, core, idle, package, tasks,\n"\
		"\t\t\tinterrupts, sensors, voltage, power, slices, custom }\n"\
		"\t-d\tShow Dashboard\n"				\
		"\t-C <#>\tMonitor Sensors\n"				\
		"\t-V <#>\tMonitor Voltage\n"				\
		"\t-W <#>\tMonitor Power\n"				\
		"\t-g <#>\tMonitor Package\n"				\
		"\t-c <#>\tMonitor Counters\n"				\
		"\t-i <#>\tMonitor Instructions\n"			\
		"\t-s\tPrint System Information\n"			\
		"\t-j\tPrint System Information (json-encoded)\n"	\
		"\t-z\tPrint Performance Capabilities\n"		\
		"\t-M\tPrint Memory Controller\n"			\
		"\t-R\tPrint System Registers\n"			\
		"\t-m\tPrint Topology\n"				\
		"\t-B\tPrint SMBIOS\n"					\
		"\t-k\tPrint Kernel\n"					\
		"\t-n\tNew line\n"					\
		"\t-h\tPrint out this message\n"			\
		"\t-v\tPrint the version number\n"			\
		"\nExit status:\n"					\
		"\t%u\tSUCCESS\t\tSuccessful execution\n"		\
		"\t%u\tCMD_SYNTAX\tCommand syntax error\n"		\
		"\t%u\tSHM_FILE\tShared memory file error\n"		\
		"\t%u\tSHM_MMAP\tShared memory mapping error\n"		\
		"\t%u\tPERM_ERR\tExecution not permitted\n"		\
		"\t%u\tMEM_ERR\t\tMemory operation error\n"		\
		"\t%u\tEXEC_ERR\tGeneral execution error\n"		\
		"\t%u\tSYS_CALL\tSystem call error\n"			\
		"\nReport bugs to labs[at]cyring.fr\n"

#define RSC_ERROR_SHARED_MEM_CODE_EN					\
		"Daemon connection error code %d\n%s: '%s' @ line %d\n"

#define RSC_ERROR_SYS_CALL_CODE_EN	"System error code %d\n%s @ line %d\n"
#define RSC_ERROR_UNIMPLEMENTED_CODE_EN "Feature is not implemented"
#define RSC_ERROR_EXPERIMENTAL_CODE_EN	"Experimental mode is required"

#define RSC_ERROR_TURBO_PREREQ_CODE_EN	"Turbo Boost | Core Performance Boost"\
					" must be disabled"

#define RSC_ERROR_UNCORE_PREREQ_CODE_EN "Invalid Uncore prerequisites"

#define RSC_ERROR_PSTATE_NOT_FOUND_CODE_EN				\
					"This Frequency P-State was not found"

#define RSC_ERROR_CLOCKSOURCE_CODE_EN	"Incompatible clock source"

#define RSC_BOX_IDLE_LIMIT_TITLE_CODE_EN " CPU-Idle Limit "

#define RSC_BOX_RECORDER_TITLE_CODE_EN	" Duration "

#define RSC_SMBIOS_TITLE_CODE_EN	" SMBIOS "

#define RSC_MECH_CLRBHB_CODE_EN		"Clear Branch History instruction"
#define RSC_MECH_SSBD_CODE_EN		"Speculative Store Bypass Disable"
#define RSC_MECH_SSBS_CODE_EN		"Speculative Store Bypass Safe"

#define RSC_CREATE_SELECT_AUTO_TURBO_CODE_EN	"  %3s       Processor     " \
						"   %s     %c%4u %c "

#define RSC_CREATE_SELECT_FREQ_TURBO_CODE_EN	"  %3s       Processor     " \
						"%7.2f MHz %c%4u %c "

#define RSC_CREATE_SELECT_FREQ_TGT_CODE_EN	"  TGT       Processor     "
#define RSC_CREATE_SELECT_FREQ_HWP_TGT_CODE_EN	"  HWP-TGT   Processor     "
#define RSC_CREATE_SELECT_FREQ_HWP_MAX_CODE_EN	"  HWP-MAX   Processor     "
#define RSC_CREATE_SELECT_FREQ_HWP_MIN_CODE_EN	"  HWP-MIN   Processor     "
#define RSC_CREATE_SELECT_FREQ_MAX_CODE_EN	"  MAX       Processor     "
#define RSC_CREATE_SELECT_FREQ_MIN_CODE_EN	"  MIN       Processor     "

#define RSC_CREATE_SELECT_FREQ_OFFLINE_CODE_EN	"  %03u                    " \
						"               Off   "

#define RSC_POPUP_DRIVER_TITLE_CODE_EN		" Driver Message "

#define RSC_EXIT_TITLE_CODE_EN			" Exit "
#define RSC_EXIT_HEADER_CODE_EN  " The following services are still running "
#define RSC_EXIT_CONFIRM_CODE_EN "              < Force-Quit >              "
#define RSC_EXIT_FOOTER_CODE_EN  "                          [ESC] to cancel "

#define RSC_CREATE_SETTINGS_COND_CODE_EN	\
					"                                "

#define RSC_CREATE_SETTINGS_COND0_CODE_EN RSC_CREATE_SETTINGS_COND_CODE_EN
#define RSC_CREATE_SETTINGS_COND1_CODE_EN RSC_CREATE_SETTINGS_COND_CODE_EN

#define RSC_HELP_BLANK_CODE		"                  "

#define RSC_CREATE_ADV_HELP_BLANK_CODE_EN	\
					"                                      "

#define RSC_CREATE_ADV_HELP_COND0_CODE_EN RSC_CREATE_ADV_HELP_BLANK_CODE_EN
#define RSC_CREATE_ADV_HELP_COND1_CODE_EN RSC_CREATE_ADV_HELP_BLANK_CODE_EN

#define RSC_BOX_BLANK_DESC_CODE 	"                                    "

#define RSC_BOX_INTERVAL_STEP1_CODE	"    100   "
#define RSC_BOX_INTERVAL_STEP2_CODE	"    150   "
#define RSC_BOX_INTERVAL_STEP3_CODE	"    250   "
#define RSC_BOX_INTERVAL_STEP4_CODE	"    500   "
#define RSC_BOX_INTERVAL_STEP5_CODE	"    750   "
#define RSC_BOX_INTERVAL_STEP6_CODE	"   1000   "
#define RSC_BOX_INTERVAL_STEP7_CODE	"   1500   "
#define RSC_BOX_INTERVAL_STEP8_CODE	"   2000   "
#define RSC_BOX_INTERVAL_STEP9_CODE	"   2500   "
#define RSC_BOX_INTERVAL_STEP10_CODE	"   3000   "

#define RSC_SETTINGS_ROUTE_TITLE_CODE_EN "Route"
#define RSC_SETTINGS_ROUTE_DFLT_CODE	"DEFAULT"
#define RSC_SETTINGS_ROUTE_IO_CODE	"    I/O"
#define RSC_SETTINGS_ROUTE_HALT_CODE	"   HALT"
#define RSC_SETTINGS_ROUTE_MWAIT_CODE	"  MWAIT"

#define RSC_BOX_CPPC_TITLE_CODE		" CPPC "
#define RSC_BOX_HWP_TITLE_CODE		" HWP "

#define RSC_BOX_CFG_TDP_BLANK_CODE	"                   "

#define RSC_BOX_PWR_OFFSET_00_CODE	"              +50 watts             "
#define RSC_BOX_PWR_OFFSET_01_CODE	"              +10 watts             "
#define RSC_BOX_PWR_OFFSET_02_CODE	"               +5 watts             "
#define RSC_BOX_PWR_OFFSET_03_CODE	"               +4 watts             "
#define RSC_BOX_PWR_OFFSET_04_CODE	"               +3 watts             "
#define RSC_BOX_PWR_OFFSET_05_CODE	"               +2 watts             "
#define RSC_BOX_PWR_OFFSET_06_CODE	"               +1 watt              "
#define RSC_BOX_PWR_OFFSET_07_CODE	"               -1 watt              "
#define RSC_BOX_PWR_OFFSET_08_CODE	"               -2 watts             "
#define RSC_BOX_PWR_OFFSET_09_CODE	"               -3 watts             "
#define RSC_BOX_PWR_OFFSET_10_CODE	"               -4 watts             "
#define RSC_BOX_PWR_OFFSET_11_CODE	"               -5 watts             "
#define RSC_BOX_PWR_OFFSET_12_CODE	"              -10 watts             "
#define RSC_BOX_PWR_OFFSET_13_CODE	"              -50 watts             "

#define RSC_BOX_CLAMPING_OFF_COND0_CODE "            Clamping OFF            "
#define RSC_BOX_CLAMPING_OFF_COND1_CODE "          < Clamping OFF >          "
#define RSC_BOX_CLAMPING_ON_COND0_CODE	"            Clamping ON             "
#define RSC_BOX_CLAMPING_ON_COND1_CODE	"          < Clamping ON  >          "

#define RSC_BOX_AMP_OFFSET_00_CODE	"              +50 amps              "
#define RSC_BOX_AMP_OFFSET_01_CODE	"              +10 amps              "
#define RSC_BOX_AMP_OFFSET_02_CODE	"               +5 amps              "
#define RSC_BOX_AMP_OFFSET_03_CODE	"               +4 amps              "
#define RSC_BOX_AMP_OFFSET_04_CODE	"               +3 amps              "
#define RSC_BOX_AMP_OFFSET_05_CODE	"               +2 amps              "
#define RSC_BOX_AMP_OFFSET_06_CODE	"               +1 amp               "
#define RSC_BOX_AMP_OFFSET_07_CODE	"               -1 amp               "
#define RSC_BOX_AMP_OFFSET_08_CODE	"               -2 amp               "
#define RSC_BOX_AMP_OFFSET_09_CODE	"               -3 amp               "
#define RSC_BOX_AMP_OFFSET_10_CODE	"               -4 amp               "
#define RSC_BOX_AMP_OFFSET_11_CODE	"               -5 amps              "
#define RSC_BOX_AMP_OFFSET_12_CODE	"              -10 amps              "
#define RSC_BOX_AMP_OFFSET_13_CODE	"              -50 amps              "

#define RSC_BOX_IDLE_LIMIT_RESET_CODE	"            0     RESET "

#define RSC_CF0_CODE "      ______                ______                  "
#define RSC_CF1_CODE "     / ____/___  ________  / ____/_______  ____ _   "
#define RSC_CF2_CODE "    / /   / __ \\/ ___/ _ \\/ /_  / ___/ _ \\/ __ `/   "
#define RSC_CF3_CODE "   / /___/ /_/ / /  /  __/ __/ / /  /  __/ /_/ /    "
#define RSC_CF4_CODE "   \\____/\\____/_/   \\___/_/   /_/   \\___/\\__, /     "
#define RSC_CF5_CODE "                                           /_/      "

#define RSC_FREQ_UNIT_MHZ_CODE		"(MHz)"
#define RSC_PPIN_CODE			"PPIN#"
#define RSC_OPP_CODE			"OPP"
#define RSC_UNCORE_CODE 		"Uncore"
#define RSC_TURBO_CODE			"Turbo"
#define RSC_CPPC_CODE			"CPPC"
#define RSC_MAX_CODE			"Max"
#define RSC_MIN_CODE			"Min"
#define RSC_TGT_CODE			"TGT"
#define RSC_TBH_CODE			"TBH"
#define RSC_TBO_CODE			"TBO"
#define RSC_ACT_CODE			"Activ"
#define RSC_HYBRID_CODE 		"Hybrid"

#define RSC_TOPOLOGY_FMT0_CODE	"%03u:\x20\x20-\x20\x20\x20\x20-\x20"
#define RSC_TOPOLOGY_FMT1_CODE	"\x20\x20\x20\x20\x20\x20\x20-\x20\x20-\x20\x20"
#define RSC_TOPOLOGY_OFF_0_CODE "\x20\x20\x20\x20-\x20\x20\x20\x20\x20\x20-\x20"
#define RSC_TOPOLOGY_OFF_1_CODE "\x20\x20-\x20\x20\x20-\x20\x20\x20\x20\x20-"
#define RSC_TOPOLOGY_OFF_2_CODE "\x20\x20-\x20\x20-\x20\x20\x20-\x20\x20-"
#define RSC_TOPOLOGY_OFF_3_CODE "\x20-\x20\x20\x20-\x20\x20\x20-\x20\x20-"
#define RSC_TOPOLOGY_HDR_PKG_CODE	"CPU Pkg  Main"
#define RSC_TOPOLOGY_HDR_SMT_CODE	"  Core/Thread"
#define RSC_TOPOLOGY_HDR_CACHE_CODE	"  Caches     "
#define RSC_TOPOLOGY_HDR_WRBAK_CODE	" (w)rite-Back"
#define RSC_TOPOLOGY_HDR_INCL_CODE	" (i)nclusive "
#define RSC_TOPOLOGY_HDR_EMPTY_CODE	"             "
#define RSC_TOPOLOGY_SUB_ITEM1_CODE	" #   ID   ID "
#define RSC_TOPOLOGY_SUB_ITEM3_CODE	" L1-Inst Way "
#define RSC_TOPOLOGY_SUB_ITEM4_CODE	" L1-Data Way "
#define RSC_TOPOLOGY_SUB_ITEM5_CODE	"     L2  Way "
#define RSC_TOPOLOGY_SUB_ITEM6_CODE	"     L3  Way "
#define RSC_TOPOLOGY_ALT_ITEM1_CODE	"   ID     ID "
#define RSC_TOPOLOGY_ALT_ITEM2_CODE	" CMP ID    ID"
#define RSC_TOPOLOGY_ALT_ITEM3_CODE	"CCD CCX ID/ID"
#define RSC_TOPOLOGY_ALT_ITEM4_CODE	" Hybrid ID/ID"
#define RSC_TOPOLOGY_BSP_COMM_CODE	" Boot Strap Processor "

#define RSC_TECH_HYPERV_NONE_CODE	"          "
#define RSC_TECH_BARE_METAL_CODE	"Bare-Metal"
#define RSC_TECH_HYPERV_XEN_CODE	"       Xen"
#define RSC_TECH_HYPERV_KVM_CODE	"       KVM"
#define RSC_TECH_HYPERV_VBOX_CODE	"VirtualBox"
#define RSC_TECH_HYPERV_KBOX_CODE	"  KVM/VBox"
#define RSC_TECH_HYPERV_VMWARE_CODE	"    VMware"
#define RSC_TECH_HYPERV_HYPERV_CODE	"MS Hyper-V"

#define RSC_PERF_LABEL_VER_CODE 	"PM"
#define RSC_PERF_LABEL_HWCF_CODE	"MPERF/APERF"
#define RSC_PERF_LABEL_CPPC_CODE	"CPPC"
#define RSC_PERF_LABEL_PCT_CODE 	"_PCT"
#define RSC_PERF_LABEL_PSS_CODE 	"_PSS"
#define RSC_PERF_LABEL_PPC_CODE 	"_PPC"
#define RSC_PERF_LABEL_CPC_CODE 	"_CPC"
#define RSC_PERF_LABEL_CST_CODE 	"_CST"
#define RSC_PERF_LABEL_HWP_CODE 	"HWP"
#define RSC_PERF_LABEL_CST_BAR_CODE	"BAR"
#define RSC_PERF_LABEL_MWAIT_IDX_CODE	\
				"#0    #1    #2    #3    #4    #5    #6    #7"

#define RSC_PERF_ENCODING_C0_CODE	" C0"
#define RSC_PERF_ENCODING_C1_CODE	" C1"
#define RSC_PERF_ENCODING_C2_CODE	" C2"
#define RSC_PERF_ENCODING_C3_CODE	" C3"
#define RSC_PERF_ENCODING_C4_CODE	" C4"
#define RSC_PERF_ENCODING_C6_CODE	" C6"
#define RSC_PERF_ENCODING_C6R_CODE	"C6R"
#define RSC_PERF_ENCODING_C7_CODE	" C7"
#define RSC_PERF_ENCODING_C7S_CODE	"C7S"
#define RSC_PERF_ENCODING_C8_CODE	" C8"
#define RSC_PERF_ENCODING_C9_CODE	" C9"
#define RSC_PERF_ENCODING_C10_CODE	"C10"
#define RSC_PERF_ENCODING_UNS_CODE	"UNS"

#define RSC_POWER_LABEL_CPPC_CODE	"EPP"
#define RSC_POWER_LABEL_TJ_CODE 	"TjMax"
#define RSC_POWER_LABEL_DTS_CODE	"DTS"
#define RSC_POWER_LABEL_PLN_CODE	"PLN"
#define RSC_POWER_LABEL_PTM_CODE	"PTM"
#define RSC_POWER_LABEL_TDP_CODE	"TDP"
#define RSC_POWER_LABEL_MIN_CODE	"Min"
#define RSC_POWER_LABEL_MAX_CODE	"Max"
#define RSC_POWER_LABEL_PL1_CODE	"PL1"
#define RSC_POWER_LABEL_PL2_CODE	"PL2"
#define RSC_POWER_LABEL_TW1_CODE	"TW1"
#define RSC_POWER_LABEL_TW2_CODE	"TW2"
#define RSC_POWER_LABEL_EDC_CODE	"EDC"
#define RSC_POWER_LABEL_TDC_CODE	"TDC"
#define RSC_POWER_LABEL_PKG_CODE	"Package"
#define RSC_POWER_LABEL_CORE_CODE	"Core"
#define RSC_POWER_LABEL_UNCORE_CODE	"Uncore"
#define RSC_POWER_LABEL_DRAM_CODE	"DRAM"
#define RSC_POWER_LABEL_PLATFORM_CODE	"Platform"

#define RSC_MEM_CTRL_UNIT_MHZ_CODE	" MHz "
#define RSC_MEM_CTRL_UNIT_MTS_CODE	" MT/s"
#define RSC_MEM_CTRL_UNIT_MBS_CODE	" MB/s"

#define RSC_MEM_CTRL_MTY_CELL_CODE	"     "
#define RSC_MEM_CTRL_CHANNEL_CODE	" Cha "

#define RSC_DDR3_CL_CODE		"   CL"
#define RSC_DDR3_RCD_CODE		"  RCD"
#define RSC_DDR3_RP_CODE		"   RP"
#define RSC_DDR3_RAS_CODE		"  RAS"
#define RSC_DDR3_RRD_CODE		"  RRD"
#define RSC_DDR3_RFC_CODE		"  RFC"
#define RSC_DDR3_WR_CODE		"   WR"
#define RSC_DDR3_RTP_CODE		" RTPr"
#define RSC_DDR3_WTP_CODE		" WTPr"
#define RSC_DDR3_FAW_CODE		"  FAW"
#define RSC_DDR3_B2B_CODE		"  B2B"
#define RSC_DDR3_CWL_CODE		"  CWL"
#define RSC_DDR3_CMD_CODE		" CMD "
#define RSC_DDR3_REFI_CODE		" REFI"
#define RSC_DDR3_DDWRTRD_CODE		" ddWR"
#define RSC_DDR3_DRWRTRD_CODE		" drWR"
#define RSC_DDR3_SRWRTRD_CODE		" srWR"
#define RSC_DDR3_DDRDTWR_CODE		" ddRW"
#define RSC_DDR3_DRRDTWR_CODE		" drRW"
#define RSC_DDR3_SRRDTWR_CODE		" srRW"
#define RSC_DDR3_DDRDTRD_CODE		" ddRR"
#define RSC_DDR3_DRRDTRD_CODE		" drRR"
#define RSC_DDR3_SRRDTRD_CODE		" srRR"
#define RSC_DDR3_DDWRTWR_CODE		" ddWW"
#define RSC_DDR3_DRWRTWR_CODE		" drWW"
#define RSC_DDR3_SRWRTWR_CODE		" srWW"
#define RSC_DDR3_XS_CODE		"  XS "
#define RSC_DDR3_XP_CODE		"  XP "
#define RSC_DDR3_CKE_CODE		" CKE "
#define RSC_DDR3_ECC_CODE		"  ECC"

#define RSC_DDR4_CL_CODE		"   CL"
#define RSC_DDR4_RCD_R_CODE		" RCDr"
#define RSC_DDR4_RCD_W_CODE		" RCDw"
#define RSC_DDR4_RP_CODE		"   RP"
#define RSC_DDR4_RAS_CODE		"  RAS"
#define RSC_DDR4_RRD_CODE		"  RRD"
#define RSC_DDR4_RFC_CODE		"  RFC"
#define RSC_DDR4_WR_CODE		"   WR"
#define RSC_DDR4_RTP_CODE		" RTPr"
#define RSC_DDR4_WTP_CODE		" WTPr"
#define RSC_DDR4_FAW_CODE		"  FAW"
#define RSC_DDR4_GEAR_CODE		" GEAR"
#define RSC_DDR4_CWL_CODE		"  CWL"
#define RSC_DDR4_CMD_CODE		"  CMD"
#define RSC_DDR4_REFI_CODE		" REFI"
#define RSC_DDR4_RDRD_SCL_CODE		" sgRR"
#define RSC_DDR4_RDRD_SC_CODE		" dgRR"
#define RSC_DDR4_RDRD_SD_CODE		" drRR"
#define RSC_DDR4_RDRD_DD_CODE		" ddRR"
#define RSC_DDR4_RDWR_SCL_CODE		" sgRW"
#define RSC_DDR4_RDWR_SC_CODE		" dgRW"
#define RSC_DDR4_RDWR_SD_CODE		" drRW"
#define RSC_DDR4_RDWR_DD_CODE		" ddRW"
#define RSC_DDR4_WRRD_SCL_CODE		" sgWR"
#define RSC_DDR4_WRRD_SC_CODE		" dgWR"
#define RSC_DDR4_WRRD_SD_CODE		" drWR"
#define RSC_DDR4_WRRD_DD_CODE		" ddWR"
#define RSC_DDR4_WRWR_SCL_CODE		" sgWW"
#define RSC_DDR4_WRWR_SC_CODE		" dgWW"
#define RSC_DDR4_WRWR_SD_CODE		" drWW"
#define RSC_DDR4_WRWR_DD_CODE		" ddWW"
#define RSC_DDR4_RRD_S_CODE		" RRDs"
#define RSC_DDR4_RRD_L_CODE		" RRDl"
#define RSC_DDR4_CKE_CODE		"  CKE"
#define RSC_DDR4_CPDED_CODE		"CPDED"
#define RSC_DDR4_ECC_CODE		"  ECC"

#define RSC_DDR4_ZEN_CL_CODE		"  CL "
#define RSC_DDR4_ZEN_RP_CODE		"  RP "
#define RSC_DDR4_ZEN_RAS_CODE		" RAS "
#define RSC_DDR4_ZEN_RC_CODE		"  RC "
#define RSC_DDR4_ZEN_FAW_CODE		" FAW "
#define RSC_DDR4_ZEN_WTR_S_CODE 	" WTRs"
#define RSC_DDR4_ZEN_WTR_L_CODE 	" WTRl"
#define RSC_DDR4_ZEN_WR_CODE		"  WR "
#define RSC_DDR4_ZEN_RDRD_SCL_CODE	" clRR"
#define RSC_DDR4_ZEN_WRWR_SCL_CODE	" clWW"
#define RSC_DDR4_ZEN_CWL_CODE		" CWL "
#define RSC_DDR4_ZEN_RTP_CODE		" RTP "
#define RSC_DDR4_ZEN_RDWR_CODE		"RdWr "
#define RSC_DDR4_ZEN_WRRD_CODE		"WrRd "
#define RSC_DDR4_ZEN_WRWR_SC_CODE	"scWW "
#define RSC_DDR4_ZEN_WRWR_SD_CODE	"sdWW "
#define RSC_DDR4_ZEN_WRWR_DD_CODE	"ddWW "
#define RSC_DDR4_ZEN_RDRD_SC_CODE	"scRR "
#define RSC_DDR4_ZEN_RDRD_SD_CODE	"sdRR "
#define RSC_DDR4_ZEN_RDRD_DD_CODE	"ddRR "
#define RSC_DDR4_ZEN_RTR_DLR_CODE	"drRR "
#define RSC_DDR4_ZEN_WTW_DLR_CODE	"drWW "
#define RSC_DDR4_ZEN_WTR_DLR_CODE	"drWR "
#define RSC_DDR4_ZEN_RRD_DLR_CODE	"drRRD"
#define RSC_DDR4_ZEN_REFI_CODE		" REFI"
#define RSC_DDR4_ZEN_RFC1_CODE		" RFC1"
#define RSC_DDR4_ZEN_RFC2_CODE		" RFC2"
#define RSC_DDR4_ZEN_RFC4_CODE		" RFC4"
#define RSC_DDR4_ZEN_RCPB_CODE		" RCPB"
#define RSC_DDR4_ZEN_RPPB_CODE		" RPPB"
#define RSC_DDR4_ZEN_BGS_CODE		"  BGS"
#define RSC_DDR4_ZEN_BGS_ALT_CODE	":Alt "
#define RSC_DDR4_ZEN_BAN_CODE		" Ban "
#define RSC_DDR4_ZEN_RCPAGE_CODE	" Page"
#define RSC_DDR4_ZEN_GDM_CODE		"  GDM"
#define RSC_DDR4_ZEN_ECC_CODE		"  ECC"
#define RSC_DDR4_ZEN_MRD_CODE		" MRD:"
#define RSC_DDR4_ZEN_MOD_CODE		" MOD:"
#define RSC_DDR4_ZEN_MRD_PDA_CODE	"PDA  "
#define RSC_DDR4_ZEN_MOD_PDA_CODE	"PDA  "
#define RSC_DDR4_ZEN_WRMPR_CODE 	"WRMPR"
#define RSC_DDR4_ZEN_STAG_CODE		" STAG"
#define RSC_DDR4_ZEN_PDM_CODE		" PDM "
#define RSC_DDR4_ZEN_RDDATA_CODE	"RDDAT"
#define RSC_DDR4_ZEN_PHYWRD_CODE	"A WRD"
#define RSC_DDR4_ZEN_PHYWRL_CODE	"  WRL"
#define RSC_DDR4_ZEN_PHYRDL_CODE	"  RDL"
#define RSC_DDR5_ZEN_RFC_SB_CODE	" RFCs"
#define RSC_DDR5_ZEN_RCPB_CODE		"b RCP"
#define RSC_DDR5_ZEN_RPPB_CODE		"B RPP"
#define RSC_DDR5_ZEN_BGS_CODE		"B BGS"

#define RSC_SYS_REGS_SPACE_CODE 	"    "
#define RSC_SYS_REGS_NA_CODE		"  - "
#define RSC_SYS_REGS_HDR_CPU_CODE	"CPU "

#define RSC_SYS_REG_HDR_FLAGS_CODE	\
	"FLAG\0  PM\0  N \0  Z \0  C \0  V \0 TCO\0 DIT\0" \
	" UAO\0 PAN\0 NMI\0 SSB\0S D \0  A \0  I \0  F \0 EL \0  M "

#define RSC_SYS_REG_HDR11_SCTL_CODE	\
	"    \0 TID\0  SP\0 NMI\0 TPI\0  TC\0SO  \0 PAN\0" \
	" ALS\0 AS0\0 ASR\0   T\0ME  \0   T\0MT  \0   T\0WE  "

#define RSC_SYS_REG_HDR12_SCTL_CODE	\
	"SCTL\0  CP\0 INT\0    \0 DR2\0 EL1\0 EL0\0    \0" \
	"    \0    \0    \0 EL1\0 EL0\0 EL1\0 EL0\0 DLY\0  En"

#define RSC_SYS_REG_HDR21_SCTL_CODE	\
	"    \0SSBS\0   A\0TA  \0   T\0CF  \0  IT\0   B\0" \
	"TI  \0 FPM\0 MC \0CMOW\0 IA \0 IB \0   L\0SM  \0 DA "

#define RSC_SYS_REG_HDR22_SCTL_CODE	\
	"SCTL\0    \0 EL1\0 EL0\0 EL1\0 EL0\0 FSB\0 EL1\0" \
	" EL0\0    \0 MS \0    \0    \0    \0 AOE\0 DnT\0    "

#define RSC_SYS_REG_HDR31_SCTL_CODE	\
	"SCTL\0 UCI\0 EE \0E0E \0SPAN\0 EIS\0 IES\0 SCN\0" \
	" WXN\0 TWE\0 TWI\0 UCT\0 DZE\0 DB \0  I \0EOS \0RCTX"

#define RSC_SYS_REG_HDR41_SCTL_CODE	\
	"SCTL\0 UMA\0 SED\0 ITD\0 AA \0CP15\0 SA0\0 SA1\0  C \0  A \0  M "

#define RSC_SYS_REG_HDR11_SCTL2_CODE	\
	"    \0  CP\0TM  \0  CP\0TA  \0  PA\0CM  \0IDCP\0 EAS\0 AN \0 AD \0NMEA"

#define RSC_SYS_REG_HDR12_SCTL2_CODE	\
	"SCT2\0 EL0\0 EL1\0 EL0\0 EL1\0 EL0\0 EL1\0 128\0    \0ERR \0ERR \0    "

#define RSC_SYS_REG_HDR_FPSR_CODE	\
	"FPSR\0  N \0  Z \0  C \0  V \0 QC \0 IDC\0 IXC\0 UFC\0 OFC\0 DZC\0 IOC"

#define RSC_SYS_REG_HDR11_EL_CODE	\
	" EL \0    \0"" Lev\0el0 \0    \0 Lev\0el1 \0    \0" \
	"   L\0evel\0""2   \0    \0 Lev\0el3 "

#define RSC_SYS_REG_HDR12_EL_CODE	\
	"Exec\0:   \0 64 \0 32 \0    \0 64 \0 32 \0    \0" \
	" 64 \0 32 \0 SEC\0    \0 64 \0 32 "

#define RSC_ISA_AES_CODE		"          AES [%c]"
#define RSC_ISA_PMULL_CODE		"        PMULL [%c]"
#define RSC_ISA_LSE_CODE		"          LSE [%c]"
#define RSC_ISA_LSE128_CODE		"       LSE128 [%c]"
#define RSC_ISA_CRC32_CODE		"        CRC32 [%c]"
#define RSC_ISA_DP_CODE 		"           DP [%c]"
#define RSC_ISA_EPAC_CODE		"         EPAC [%c]"
#define RSC_ISA_FCMA_CODE		"         FCMA [%c]"
#define RSC_ISA_FHM_CODE		"          FHM [%c]"
#define RSC_ISA_FP_CODE 		"           FP [%c]"
#define RSC_ISA_FPAC_CODE		"         FPAC [%c]"
#define RSC_ISA_FPACCOMBINE_CODE	"  FPACCOMBINE [%c]"
#define RSC_ISA_JSCVT_CODE		"        JSCVT [%c]"
#define RSC_ISA_LRCPC_CODE		"        LRCPC [%c]"
#define RSC_ISA_LRCPC2_CODE		"       LRCPC2 [%c]"
#define RSC_ISA_LRCPC3_CODE		"       LRCPC3 [%c]"
#define RSC_ISA_MOPS_CODE		"         MOPS [%c]"
#define RSC_ISA_PACGA_CODE		"        PACGA [%c]"
#define RSC_ISA_PACQARMA3_CODE		"    PACQARMA3 [%c]"
#define RSC_ISA_PACQARMA5_CODE		"    PACQARMA5 [%c]"
#define RSC_ISA_PAUTH_CODE		"        PAuth [%c]"
#define RSC_ISA_PAUTH2_CODE		"       PAuth2 [%c]"
#define RSC_ISA_PAUTH_LR_CODE		"     PAuth_LR [%c]"
#define RSC_ISA_PRFMSLC_CODE		"      PRFMSLC [%c]"
#define RSC_ISA_FRINTTS_CODE		"      FRINTTS [%c]"
#define RSC_ISA_SPECRES_CODE		"      SPECRES [%c]"
#define RSC_ISA_SPECRES2_CODE		"     SPECRES2 [%c]"
#define RSC_ISA_BF16_CODE		"         BF16 [%c]"
#define RSC_ISA_EBF16_CODE		"        EBF16 [%c]"
#define RSC_ISA_CSSC_CODE		"         CSSC [%c]"
#define RSC_ISA_HBC_CODE		"          HBC [%c]"
#define RSC_ISA_I8MM_CODE		"         I8MM [%c]"
#define RSC_ISA_RPRES_CODE		"        RPRES [%c]"
#define RSC_ISA_SB_CODE 		"           SB [%c]"
#define RSC_ISA_SYSREG128_CODE		"    SYSREG128 [%c]"
#define RSC_ISA_SYSINSTR128_CODE	"  SYSINSTR128 [%c]"
#define RSC_ISA_WFxT_CODE		"         WFxT [%c]"
#define RSC_ISA_XS_CODE 		"           XS [%c]"
#define RSC_ISA_LS64_CODE		"         LS64 [%c]"
#define RSC_ISA_LS64_V_CODE		"       LS64_V [%c]"
#define RSC_ISA_LS64_ACCDATA_CODE	" LS64_ACCDATA [%c]"
#define RSC_ISA_DGH_CODE		"          DGH [%c]"
#define RSC_ISA_DPB_CODE		"          DPB [%c]"
#define RSC_ISA_DPB2_CODE		"         DPB2 [%c]"
#define RSC_ISA_RAND_CODE		"         RAND [%c]"
#define RSC_ISA_RDMA_CODE		"         RDMA [%c]"
#define RSC_ISA_RPRFM_CODE		"        RPRFM [%c]"
#define RSC_ISA_SHA1_CODE		"         SHA1 [%c]"
#define RSC_ISA_SHA256_CODE		"       SHA256 [%c]"
#define RSC_ISA_SHA512_CODE		"       SHA512 [%c]"
#define RSC_ISA_SHA3_CODE		"         SHA3 [%c]"
#define RSC_ISA_SIMD_CODE		"        ASIMD [%c]"
#define RSC_ISA_SM3_CODE		"          SM3 [%c]"
#define RSC_ISA_SM4_CODE		"          SM4 [%c]"
#define RSC_ISA_SME_CODE		"          SME [%c]"
#define RSC_ISA_SVE_CODE		"          SVE [%c]"
#define RSC_ISA_SVE_F64MM_CODE		"    SVE_F64MM [%c]"
#define RSC_ISA_SVE_F32MM_CODE		"    SVE_F32MM [%c]"
#define RSC_ISA_SVE_I8MM_CODE		"     SVE_I8MM [%c]"
#define RSC_ISA_SVE_SM4_CODE		"      SVE_SM4 [%c]"
#define RSC_ISA_SVE_SHA3_CODE		"     SVE_SHA3 [%c]"
#define RSC_ISA_SVE_BF16_CODE		"     SVE_BF16 [%c]"
#define RSC_ISA_SVE_EBF16_CODE		"    SVE_EBF16 [%c]"
#define RSC_ISA_SVE_BitPerm_CODE	"  SVE_BitPerm [%c]"
#define RSC_ISA_SVE_AES_CODE		"      SVE_AES [%c]"
#define RSC_ISA_SVE_PMULL128_CODE	" SVE_PMULL128 [%c]"
#define RSC_ISA_SVE2_CODE		"         SVE2 [%c]"
#define RSC_ISA_SME2_CODE		"         SME2 [%c]"
#define RSC_ISA_SME2p1_CODE		"       SME2p1 [%c]"
#define RSC_ISA_SME_FA64_CODE		"     SME_FA64 [%c]"
#define RSC_ISA_SME_LUTv2_CODE		"    SME_LUTv2 [%c]"
#define RSC_ISA_SME_I16I64_CODE 	"   SME_I16I64 [%c]"
#define RSC_ISA_SME_F64F64_CODE 	"   SME_F64F64 [%c]"
#define RSC_ISA_SME_I16I32_CODE 	"   SME_I16I32 [%c]"
#define RSC_ISA_SME_B16B16_CODE 	"   SME_B16B16 [%c]"
#define RSC_ISA_SME_F16F16_CODE 	"   SME_F16F16 [%c]"
#define RSC_ISA_SME_F8F16_CODE		"    SME_F8F16 [%c]"
#define RSC_ISA_SME_F8F32_CODE		"    SME_F8F32 [%c]"
#define RSC_ISA_SME_I8I32_CODE		"    SME_I8I32 [%c]"
#define RSC_ISA_SME_F16F32_CODE 	"   SME_F16F32 [%c]"
#define RSC_ISA_SME_B16F32_CODE 	"   SME_B16F32 [%c]"
#define RSC_ISA_SME_BI32I32_CODE	"  SME_BI32I32 [%c]"
#define RSC_ISA_SME_F32F32_CODE 	"   SME_F32F32 [%c]"
#define RSC_ISA_SME_SF8FMA_CODE 	"   SME_SF8FMA [%c]"
#define RSC_ISA_SME_SF8DP4_CODE 	"   SME_SF8DP4 [%c]"
#define RSC_ISA_SME_SF8DP2_CODE 	"   SME_SF8DP2 [%c]"
#define RSC_ISA_FlagM_CODE		"        FlagM [%c]"
#define RSC_ISA_FlagM2_CODE		"       FlagM2 [%c]"
