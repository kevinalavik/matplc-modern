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


#ifndef __NONLINEAR_H
#define __NONLINEAR_H

#include "nonlinear.h"
#include "dsp_t.h"

/**************************************************************/
/*****************                             ****************/
/*****************  Global (to dsp) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

int nonlinear_parse_parm (const char *table_name,
                         int table_index,
                         fb_parm_t *parm,
		       f32 T /* period this dsp module will work with */);

int nonlinear_step (unsigned int step_count,
		    dsp_time_t *dsp_time,
		    fb_parm_t *parm);

/* mainly used for debugging */
int nonlinear_dump_parm (fb_parm_t parm);


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* Default parameter values */

#define CUTOFF_TOP_ID   "cutoff_top"
#define CUTOFF_BOT_ID   "cutoff_bot"
#define DEADBAND_TOP_ID "deadband_top"
#define DEADBAND_BOT_ID "deadband_bot"
#define DEADBAND_OUT_ID "deadband_out"
#define ABS_GAIN_ID     "gain"

/* default parameters make this a pass-through function block... */
#define CUTOFF_TOP_DEF   (f32_MAX)
#define CUTOFF_BOT_DEF   (-f32_MAX)
#define DEADBAND_TOP_DEF 0
#define DEADBAND_BOT_DEF 0
#define DEADBAND_OUT_DEF 0
#define ABS_GAIN_DEF     1



#endif /* __NONLINEAR_H */
