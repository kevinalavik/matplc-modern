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
 * keyboard input - sample code
 *
 * Last Revised: April 16, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> /* only used for the memset function */


#include <termios.h>	/* keyboard stuff */
#include <unistd.h>

#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif



plc_pt_t	p[256];	// an array to map keys to PLC points
plc_pt_t	quit;

int init(){
	struct termios	t;		// the keyboard interface struct

	memset(p, 0, sizeof(p)); /* reset all the .valid fields */

	/* note: multiple handles to same point are OK */
	p['l'] = plc_pt_by_name("left"); if(!p['l'].valid) return ERROR;
	p['L'] = p['l'];
	p['r'] = plc_pt_by_name("right"); if(!p['r'].valid) return ERROR;
	p['R'] = p['r'];
	p['q'] = plc_pt_by_name("quit"); if(!p['q'].valid) return ERROR;
	p['Q'] = p['q'];
	p[3] = plc_pt_by_name("quit");  if(!p[3].valid) return ERROR; /* Ctrl-C */
	quit = p[3];
	
	/* set up the keyboard stuff to get single characters */
	tcgetattr ( 0, &t);
	t.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP);
	t.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG );
	tcsetattr ( 0, TCSANOW, &t);

	return NO_ERROR;
}


int step_run(){
	unsigned char ch;	

	if(plc_get(quit)){
		send_message("QUIT");
	} else {
		ch = getchar();
		if (p[ch].valid) {
			plc_set(p[ch], !plc_get(p[ch]));
		} else {
			printf("\a");
		}
	}

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif

