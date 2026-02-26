
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

#include "instructions.h"


const plc5_commands comms[] = {
		{	IL_ACI,		__INST_UNDEFINED,		"ACI",	2,	3, {"String", "Integer", "", "", "", "", "", "", "", ""}, "String to Integer"},
		{	IL_ACN,		__INST_UNDEFINED,		"ACN",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Concatenate Strings"},
		{	IL_ACS,		__INST_UNDEFINED,		"ACS",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "ArcCosine"},
		{	IL_ADD,		__INST_UNDEFINED,		"ADD",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Addition"},
		{	IL_AEX,		__INST_UNDEFINED,		"AEX",	4,	3, {"Source", "Start", "Length", "Dest", "", "", "", "", "", ""}, "Extract String"},
		{	IL_AFI,		__INST_UNDEFINED,		"AFI",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Always False"},
		{	IL_AIC,		__INST_UNDEFINED,		"AIC",	2,	3, {"Integer", "String", "", "", "", "", "", "", "", ""}, "Integer to String"},
		{	IL_ANB,		__INST_UNDEFINED,		"ANB",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "AND Using Stack"},
		{	IL_ARL,		__INST_UNDEFINED,		"ARL",	5,	3, {"Channel", "Source", "Control", "Length", "chars read", "", "", "", "", ""}, "ASCII Read Line"},
		{	IL_ASN,		__INST_UNDEFINED,		"ASN",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "ArcSine"},
		{	IL_ATI,		__INST_UNDEFINED,		"ATI",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Always True"},
		{	IL_ATN,		__INST_UNDEFINED,		"ATN",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "ArcTangent"},
		{	IL_AND,		__INST_UNDEFINED,		"AND",	1,	3, {"Source", "", "", "", "", "", "", "", "", ""}, "AND Stack with Memory"},
		{	IL_ASC,		__INST_UNDEFINED,		"ASC",	4,	3, {"Source", "Index", "Search", "Result", "", "", "", "", "", ""}, "Search String"},
		{	IL_ASR,		__INST_UNDEFINED,		"ASR",	2,	3, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Compare Strings"},
		{	IL_AVE,		__INST_UNDEFINED,		"AVE",	5,	3, {"File", "Dest", "Control", "Length", "Position", "", "", "", "", ""}, "Average"},
		{	IL_AWT,		__INST_UNDEFINED,		"AWT",	5,	3, {"Channel", "Source", "Control", "Length", "Sent", "", "", "", "", ""}, "ASCII from Chan"},
		{	IL_BAND,	__INST_UNDEFINED,		"BAND",	3,	4, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Boolean AND"},
		{	IL_BOR,		__INST_UNDEFINED,		"BOR",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Boolean OR"},
		{	IL_BND,		__INST_UNDEFINED,		"BND",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Branch End"},
		{	IL_BNOT,	__INST_UNDEFINED,		"BNOT",	2,	4, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Boolean Not"},
		{	IL_BSL,		__INST_UNDEFINED,		"BSL",	4,	3, {"File", "Control", "Source Bit", "Length", "", "", "", "", "", ""}, "Bit Shift Left"},
		{	IL_BSR,		__INST_UNDEFINED,		"BSR",	4,	3, {"File", "Control", "Source Bit", "Length", "", "", "", "", "", ""}, "Bit Shift Right"},
		{	IL_BST,		__INST_UNDEFINED,		"BST",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Branch Start"},
		{	IL_BXOR,	__INST_UNDEFINED,		"BXOR",	3,	4, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Boolean XOR"},
		{	IL_CLR,		__INST_UNDEFINED,		"CLR",	1,	3, {"Dest", "", "", "", "", "", "", "", "", ""}, "Clear"},
		{	IL_COP,		__INST_UNDEFINED,		"COP",	3,	3, {"Source", "Dest", "Length", "", "", "", "", "", "", ""}, "File Copy"},
		{	IL_COS,		__INST_UNDEFINED,		"COS",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Cosine"},
		{	IL_CTU,		__INST_UNDEFINED,		"CTU",	3,	3, {"Counter", "Preset", "Accumulator", "", "", "", "", "", "", ""}, "Count UP"},
		{	IL_CTD,		__INST_UNDEFINED,		"CTD",	3,	3, {"Counter", "Preset", "Accumulator", "", "", "", "", "", "", ""}, "Count Down"},
		{	IL_DEG,		__INST_UNDEFINED,		"DEG",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "To Degrees"},
		{	IL_DIV,		__INST_UNDEFINED,		"DIV",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Divide"},
		{	IL_END,		__INST_UNDEFINED,		"END",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "End of Program"},
		{	IL_EOR,		__INST_UNDEFINED,		"EOR",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "EOR with Stack"},
		{	IL_EQ,		__INST_UNDEFINED,		"EQ",	2,	2, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Equals"},
		{	IL_FFL,		__INST_UNDEFINED,		"FFL",	5,	3, {"Source", "FIFO", "Control", "Length", "Position", "", "", "", "", ""}, "FIFI Unload"},
		{	IL_FFU,		__INST_UNDEFINED,		"FFU",	5,	3, {"FIFO", "Dest", "Control", "Length", "Position", "", "", "", "", ""}, "FIFO Load"},
		{	IL_FLL,		__INST_UNDEFINED,		"FLL",	3,	3, {"Source", "File", "Length", "", "", "", "", "", "", ""}, "File Copy"},
		{	IL_FRD,		__INST_UNDEFINED,		"FRD",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "From BCD"},
		{	IL_GT,		__INST_UNDEFINED,		"GT",	2,	2, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Greater Than"},
		{	IL_GE,		__INST_UNDEFINED,		"GE",	2,	2, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Greater of Equals"},
		{	IL_JMP,		__INST_UNDEFINED,		"JMP",	1,	3, {"Label", "", "", "", "", "", "", "", "", ""}, "Jump to Label"},
		{	IL_JSR,		__INST_UNDEFINED,		"JSR",	1,	3, {"Program", "", "", "", "", "", "", "", "", ""}, "Jump to Subroutine"},
		{	IL_LBL,		__INST_UNDEFINED,		"LBL",	1,	3, {"Label", "", "", "", "", "", "", "", "", ""}, "Label"},
		{	IL_LD,		__INST_UNDEFINED,		"LD",	1,	2, {"Source", "", "", "", "", "", "", "", "", ""}, "Load Stack"},
		{	IL_LE,		__INST_UNDEFINED,		"LE",	2,	2, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Less or Equal"},
		{	IL_LIM,		__INST_UNDEFINED,		"LIM",	3,	3, {"High Limit", "Test Value", "Low Limit", "", "", "", "", "", "", ""}, "Limit Test"},
		{	IL_LFL,		__INST_UNDEFINED,		"LFL",	5,	3, {"Source", "LIFO", "Control", "Length", "Position", "", "", "", "", ""}, "LIFO Load"},
		{	IL_LFU,		__INST_UNDEFINED,		"LFU",	5,	3, {"LIFO", "Dest", "Control", "Length", "Position", "", "", "", "", ""}, "LIFO Unload"},
		{	IL_LN,		__INST_UNDEFINED,		"LN",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Natural Log"},
		{	IL_LOG,		__INST_UNDEFINED,		"LOG",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Base 10 Log"},
		{	IL_LT,		__INST_UNDEFINED,		"LT",	2,	2, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Less Than"},
		{	IL_MEQ,		__INST_UNDEFINED,		"MEQ",	3,	3, {"SourceA", "Mask", "SourceB", "", "", "", "", "", "", ""}, "Masked Equals"},
		{	IL_MOV,		__INST_UNDEFINED,		"MOV",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Move Data"},
		{	IL_MSG,		__INST_UNDEFINED,		"MSG",	3,	3, {"Command", "Control", "Slot", "", "", "", "", "", "", ""}, "Message"},
		{	IL_MUL,		__INST_UNDEFINED,		"MUL",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Multiply"},
		{	IL_MVM,		__INST_UNDEFINED,		"MVM",	3,	3, {"Source", "Mask", "Dest", "", "", "", "", "", "", ""}, "Move Masked"},
		{	IL_NE,		__INST_UNDEFINED,		"NE",	2,	2, {"SourceA", "SourceB", "", "", "", "", "", "", "", ""}, "Not Equal"},
		{	IL_NEG,		__INST_UNDEFINED,		"NEG",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Negative"},
		{	IL_NOP,		__INST_UNDEFINED,		"NOP",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "No Operation"},
		{	IL_NOT,		__INST_UNDEFINED,		"NOT",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Not Stack"},
		{	IL_NXB,		__INST_UNDEFINED,		"NXB",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Next Branch"},
		{	IL_ONS,		__INST_UNDEFINED,		"ONS",	1,	3, {"Bit", "", "", "", "", "", "", "", "", ""}, "One Shot"},
		{	IL_OR,		__INST_UNDEFINED,		"OR",	1,	2, {"Source", "", "", "", "", "", "", "", "", ""}, "OR Stack with Memory"},
		{	IL_ORB,		__INST_UNDEFINED,		"ORB",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "OR with Stack"},
		{	IL_OTE,		__INST_UNDEFINED,		"OTE",	1,	3, {"Dest", "", "", "", "", "", "", "", "", ""}, "Output Enable"},
		{	IL_OTL,		__INST_UNDEFINED,		"OTL",	1,	1, {"Dest", "", "", "", "", "", "", "", "", ""}, "Output Latch"},
		{	IL_OTU,		__INST_UNDEFINED,		"OTU",	1,	1, {"Dest", "", "", "", "", "", "", "", "", ""}, "Output Unlatch"},
		{	IL_RAD,		__INST_UNDEFINED,		"RAD",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "To Radians"},
		{	IL_RES,		__INST_UNDEFINED,		"RES",	1,	3, {"Dest", "", "", "", "", "", "", "", "", ""}, "Reset"},
		{	IL_RET,		__INST_UNDEFINED,		"RET",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Return from JSR"},
		{	IL_RTO,		__INST_UNDEFINED,		"RTO",	3,	3, {"Timer", "Preset", "Accumulator", "", "", "", "", "", "", ""}, "Retentive Timer OFF"},
		{	IL_SIN,		__INST_UNDEFINED,		"SIN",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Sine"},
		{	IL_SOR,		__INST_UNDEFINED,		"SOR",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Start of Rung"},
		{	IL_SQI,		__INST_UNDEFINED,		"SQI",	6,	3, {"File", "Mask", "Source", "Control", "Length", "Position", "", "", "", ""}, "Sequencer Input"},
		{	IL_SQL,		__INST_UNDEFINED,		"SQL",	5,	3, {"File", "Source", "Control", "Length", "Position", "", "", "", "", ""}, "Sequencer Load"},
		{	IL_SQO,		__INST_UNDEFINED,		"SQO",	6,	3, {"File", "Mask", "Destination", "Control", "Length", "Position", "", "", "", ""}, "Sequencer Output"},
		{	IL_SQR,		__INST_UNDEFINED,		"SQR",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Square Root"},
		{	IL_SRT,		__INST_UNDEFINED,		"SRT",	4,	3, {"File", "Control", "Length", "Position", "", "", "", "", "", ""}, "Sort"},
		{	IL_STD,		__INST_UNDEFINED,		"STD",	5,	3, {"File", "Destination", "Control", "Length", "Position", "", "", "", "", ""}, "Standard Deviation"},
		{	IL_SUB,		__INST_UNDEFINED,		"SUB",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "Subtract"},
		{	IL_TAN,		__INST_UNDEFINED,		"TAN",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "Tangent"},
		{	IL_TND,		__INST_UNDEFINED,		"TND",	0,	3, {"", "", "", "", "", "", "", "", "", ""}, "Temporary End"},
		{	IL_TOD,		__INST_UNDEFINED,		"TOD",	2,	3, {"Source", "Dest", "", "", "", "", "", "", "", ""}, "To BCD"},
		{	IL_TON,		__INST_UNDEFINED,		"TON",	3,	3, {"Timer", "Preset", "Accumulator", "", "", "", "", "", "", ""}, "On-delay Timer"},
		{	IL_TOF,		__INST_UNDEFINED,		"TOF",	3,	3, {"Timer", "Preset", "Accumulator", "", "", "", "", "", "", ""}, "Off-delay Timer"},
		{	IL_XIC,		__INST_UNDEFINED,		"XIC",	1,	3, {"Source", "", "", "", "", "", "", "", "", ""}, "Examine if Closed (NO)"},
		{	IL_XIO,		__INST_UNDEFINED,		"XIO",	1,	3, {"Source", "", "", "", "", "", "", "", "", ""}, "Examine if Open (NC)"},
		{	IL_XOR,		__INST_UNDEFINED,		"XOR",	1,	3, {"Source", "", "", "", "", "", "", "", "", ""}, "XOR Stack and Memory"},
		{	IL_XPY,		__INST_UNDEFINED,		"XPY",	3,	3, {"SourceA", "SourceB", "Dest", "", "", "", "", "", "", ""}, "X to Power Y"},
		{	0,		__INST_UNDEFINED,		"",	0,	0}};

