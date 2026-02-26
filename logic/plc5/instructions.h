
//
//    instructions.h - instructions for the software PLC
//
//    Copyright (C) 2000 by Hugh Jack <jackh@gvsu.edu>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//
// Last Modified: October 5, 2000
//


#ifndef __INSTRUCTIONS
#define __INSTRUCTIONS

	#include "program_il.h"

	#define	__INST_BASIC_LOGIC	400
	#define	__INST_MATH			401
	#define	__INST_LOGICAL		402
	#define	__INST_UNDEFINED	403

	struct plc5_commands {
		int		type;
		int		category;
		const char	*label;
		int		args;
		int		length;
		const char	*arg[10];
		const char	*description;
	};

	extern const plc5_commands comms[];

	// Here are the opcodes for the basic logic -- All of these are implemented
	#define		IL_BASIC_LOGIC_START	1000
	#define		IL_AND					1000	// Input Logic
	#define		IL_XIC					1001
	#define		IL_XIO					1002
	#define		IL_LD					1003
	#define		IL_OR					1004
	#define		IL_XOR					1005
	#define		IL_AFI					1006
	#define		IL_NOT					1007
	#define		IL_ATI					1008
	#define		IL_ANB					1009
	#define		IL_ORB					1010
	#define		IL_OTE					1100	// Output and latches
	#define		IL_OTL					1101
	#define		IL_OTU					1102
	#define		IL_BST					1300	// Ladder and Stack Structures
	#define		IL_BND					1301
	#define		IL_NXB					1302
	#define		IL_SOR					1303
	#define		IL_EOR					1304
	#define		IL_BASIC_LOGIC_END		1999


	// Here are the opcodes for timers and counters -- All of these are implemented
	#define		IL_TIMER_COUNTER_START		2000
	#define		IL_TON						2000
	#define		IL_TOF						2001
	#define		IL_RTO						2002
	#define		IL_CTU						2003
	#define		IL_CTD						2004
	#define		IL_RES						2005
	#define		IL_TIMER_COUNTER_END		2999


	// Here are the opcodes for the math functions
	#define		IL_MATH_START			3000
	#define		IL_ADD					3001	// Arithmetic
	#define		IL_DIV					3002
	#define		IL_MUL					3003
	#define		IL_NEG					3004
	#define		IL_CLR					3005
	#define		IL_SUB					3006
	#define		IL_BAND					3102	// Boolean functions
	#define		IL_BOR					3103
	#define		IL_BNOT					3104
	#define		IL_BXOR					3105
	#define		IL_SQR					3200	// Advanced Math Functions
	#define		IL_LN					3201
	#define		IL_LOG					3202
	#define		IL_XPY					3203
	#define		IL_COS					3204
	#define		IL_SIN					3205
	#define		IL_TAN					3206
	#define		IL_ACS					3207
	#define		IL_ASN					3208
	#define		IL_ATN					3209
	#define		IL_TOD					3210
	#define		IL_FRD					3211
	#define		IL_DEG					3212
	#define		IL_RAD					3213
	#define		IL_AVE					3214
	#define		IL_SRT					3215
	#define		IL_STD					3216
	#define		IL_CPT					3217
	#define		IL_MATH_END				3999


	// Here are the opcodes for the data functions
	#define		IL_DATA_START			4000
	#define		IL_EQ					4001	// Comparison Functions
	#define		IL_GT					4002
	#define		IL_GE					4003
	#define		IL_LE					4004
	#define		IL_LT					4005
	#define		IL_LIM					4006
	#define		IL_MEQ					4007
	#define		IL_NE					4008
	#define		IL_CMP					4009
	#define		IL_MVM					4100	// Data Moves and Manipulates
	#define		IL_MOV					4101
	#define		IL_BSL					4102
	#define		IL_BSR					4103
	#define		IL_BTD					4104
	#define		IL_COP					4105
	#define		IL_DDT					4106
	#define		IL_DTR					4107
	#define		IL_FAL					4108
	#define		IL_FBC					4109
	#define		IL_FFL					4110
	#define		IL_FFU					4111
	#define		IL_FLL					4112
	#define		IL_LFL					4113
	#define		IL_LFU					4114
	#define		IL_FSC					4115
	#define		IL_SQI					4116
	#define		IL_SQL					4117
	#define		IL_SQO					4118
	#define		IL_DATA_END				4999


	// Here are the opcodes for program control
	#define		IL_PROGRAM_CONTROL_START	5000
	#define		IL_UID						5000	// Interrupts
	#define		IL_UIE						5001
	#define		IL_LBL						5100	// Looping and flow
	#define		IL_NOP						5101
	#define		IL_TND						5102
	#define		IL_END						5103
	#define		IL_JMP						5201	// Function Calls
	#define		IL_RET						5202
	#define		IL_JSR						5203
	#define		IL_EOT						5300	// SFC Flow Control
	#define		IL_SFR						5301
	#define		IL_IIN						5400	// Input and output
	#define		IL_IOT						5401
	#define		IL_ONS						5402
	#define		IL_OSR						5403
	#define		IL_OSF						5404
	#define		IL_PROGRAM_CONTROL_END		5999

	// Here are the opcodes for advanced functions
	#define		IL_FANCY_START				8000
	#define		IL_BTR						8000
	#define		IL_BTW						8001
	#define		IL_MSG						8002
	#define		IL_PID						8003
	#define		IL_FANCY_END				8999

	// Here are the opcodes for ASCII Communications
	#define		IL_ASCII_START				9000

	#define		IL_ABL						9000
	#define		IL_ACB						9001
	#define		IL_ACI						9002
	#define		IL_ACN						9003
	#define		IL_AEX						9004
	#define		IL_AIC						9005
	#define		IL_ARD						9006
	#define		IL_ARL						9007
	#define		IL_ASC						9008
	#define		IL_ASR						9009
	#define		IL_AWA						9010
	#define		IL_AWT						9011
	#define		IL_ASCII_END				9999

	
	//
	// Bit definitions for the PLC 5
	//
	#define		_EN		15
	#define		_CU		15
	#define		_TT		14
	#define		_EU		14
	#define		_CD		14
	#define		_DN		13
	#define		_OV		12
	#define		_EM		12
	#define		_UN		11
	#define		_ER		11
	#define		_UL		10
	#define		_IN		9
	#define		_FD		8

	#define		_PRESET			1000
	#define		_ACCUMULATOR	1001
	#define		_LENGTH			1002
	#define		_POSITION		1003

	#define		_PRESET_STRING		"PRE"
	#define		_ACCUMULATOR_STRING	"ACC"
	#define		_LENGTH_STRING		"LEN"
	#define		_POSITION_STRING	"POS"


#endif
