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


#ifndef __PID_H
#define __PID_H

#include "time_util.h"
#include "pid_t.h"
#include "dsp_t.h"

/**************************************************************/
/*****************                             ****************/
/*****************  Global (to dsp) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

int pid_parse_parm (const char *table_name,
                    int table_index, 
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */);

int pid_step (unsigned int step_count,
              dsp_time_t *dsp_time,
              fb_parm_t *parm);

/* mainly used for debugging */
int pid_dump_parm (fb_parm_t parm);


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/


#define MINOUT_ID  "min_out"
#define MAXOUT_ID  "max_out"
#define DEF_MINOUT (-f32_MAX)
#define DEF_MAXOUT   f32_MAX

#define MANOUT_ID  "man_out"
#define MANMODE_ID  "man_mode"

/* Default parameter values */
   /* default is to have the output follow the input */
#define DEF_P (1)
#define DEF_I (0)
#define DEF_D (0)


#endif /* __PID_H */
