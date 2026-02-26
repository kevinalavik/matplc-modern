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
 * simple ladder logic test
 *
 * Last Revised: June 10, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


plc_pt_t	A,
		B;


int count;


int init(){
	/* get the point handles */
	A = plc_pt_by_name("A");
	B = plc_pt_by_name("B");
	if(!A.valid || !B.valid) return ERROR;
	count = 0;

	return NO_ERROR;
}



int step_run(){
	printf("LADDER_TEST: step \n");
	count++; if(count > 100) send_message("QUIT");
	plc_set(A, count);

	printf("LADDER_TEST: Setting=%d     Got=%d\n", count, plc_get(B));

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif

