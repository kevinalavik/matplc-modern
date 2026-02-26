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

#ifndef __GLOBAL
#define __GLOBAL

	#include <stdio.h>
	#include <stdlib.h>
	#include <iostream>
	#include <math.h>
	#include <string.h>

	#define	__VERSION	"LPC 0.0.6.2"

	#define	NO_ERROR	-1
	#define ERROR 		0
	#define USED		1
	#define	UNUSED		0
	#define	TRUE		1
	#define	FALSE		0

	#ifndef PI
		#define	PI			3.141592654
	#endif

	#define	_CARRY		100
	#define	_OVERFLOW	101
	#define	_ZERO		102
	#define	_NEGATIVE	103

	#define	_FIRST_SCAN	200
	#define _LAST_SCAN	201

	#define	_INPUT		300
	#define	_OUTPUT		301
	#define	_STATUS		302
	#define	_BIT		303
	#define	_TIMER		304
	#define _COUNTER	305
	#define _CONTROL	306
	#define	_WORD		307
	#define	_FLOAT		308
	#define _LONGINT	309
	#define _CHAR		310
	#define	_BCD		312
	#define	_MSG		313
	#define	_STRING		5000

	#define	_IEC1131_IL	400
	#define	_IEC1131_ST	401
	#define	_AB_MNEMONICS	402

	#define	_LOG_TO_SCREEN		1
	#define	_LOG_TO_FILE		2
	void error_log(const char*);


#endif
