/*
 * (c) 2000 Jiri Baum
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
 * light chaser - sample code, modified for the shared library handler
 *
 * Last Revised: April 16, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


#define		num_lights		4	 /* how many lights in the chase, max 9 */
plc_pt_t	left,
		right,
		quit,
		L[num_lights];



int init(){
	char Lname[] = "L1";
	int i;

//printf("Chaser init \n");
	/* get the point handles */
	left = plc_pt_by_name("left");
	right = plc_pt_by_name("right");
	quit = plc_pt_by_name("quit");
	if(!left.valid || !right.valid || !quit.valid) return ERROR;
	for(i = 0; i < num_lights; i++, Lname[1]++){
		L[i] = plc_pt_by_name(Lname);
		if(!L[i].valid) return ERROR;
	}

	return NO_ERROR;
}



int step_run(){
	static int	dir = 0;
	static int	cur = num_lights - 1;
//printf("Chaser step \n");

	/* check for change of direction */
	if (plc_get(left) && !plc_get(right)){
//printf("Negative Direction\n");
		dir = -1;
	} else if (plc_get(right)){
//printf("Positive Direction\n");
		dir = 1;
	}
	/* move the light along */
//printf("cur_before = %d   %d   %d  \n", cur, dir, num_lights);
	plc_set(L[cur],0);
	cur = (cur + dir + num_lights) % num_lights; /* be careful about -1 % n */
//printf("cur_after = %d    \n", cur);
	plc_set(L[cur],1);


	if(plc_get(quit)) send_message("QUIT");

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif

