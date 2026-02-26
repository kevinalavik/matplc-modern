/*
 * (c) 2002 Hugh Jack
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

/*
 * This header contains definitions for all modules and functions
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#define	ERROR		1	
#define NO_ERROR	0

#define	TRUE		1
#define	FALSE		0


//
// Error handling function
//
#define		CRITICAL	9
#define		MAJOR		7
#define		MINOR		5
#define		WARNING		2
void error_log(int, char*);


#endif


#ifdef __cplusplus
}
#endif
