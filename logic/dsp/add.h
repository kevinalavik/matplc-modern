/*
 * (c) 2000 Mario de Sousa
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


#ifndef __ADD_H
#define __ADD_H

#include "add_t.h"
#include "dsp_t.h"

/**************************************************************/
/*****************                             ****************/
/*****************  Global (to dsp) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

int add_parse_parm (const char *table_name,
                    int table_index,
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */);

int add_step (unsigned int step_count,
	      dsp_time_t *dsp_time,
	      fb_parm_t *parm);

/* mainly used for debugging */
int add_dump_parm (fb_parm_t parm);


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* Default parameter values */
   /* default is to add all inputs */
#define ADD_DEF_MULT (1)


#endif /* __ADD_H */
