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


#ifndef __RAMP_H
#define __RAMP_H

#include "time_util.h"
#include "ramp_t.h"
#include "dsp_t.h"

/**************************************************************/
/*****************                             ****************/
/*****************  Global (to dsp) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

int ramp_parse_parm (const char *table_name,
                     int table_index,
                     fb_parm_t *parm,
		     f32 T /* period this dsp module will work with */);

int ramp_step (unsigned int step_count,
               dsp_time_t *dsp_time,
               fb_parm_t *parm);

/* mainly used for debugging */
int ramp_dump_parm (fb_parm_t parm);


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* Default parameter values */
   /* default is to have the output follow the input */
#define POS_DXDT_DEF   (f32_MAX)
#define NEG_DXDT_DEF   (f32_MAX)
#define POS_D2XDT2_DEF (f32_MAX)
#define NEG_D2XDT2_DEF (f32_MAX)

/* parameter configuration pairs */
#define     DXDT_ID       "dxdt"
#define POS_DXDT_ID   "pos_dxdt"
#define NEG_DXDT_ID   "neg_dxdt"
#define     D2XDT2_ID     "d2xdt2"
#define POS_D2XDT2_ID "pos_d2xdt2"
#define NEG_D2XDT2_ID "neg_d2xdt2"


#endif /* __RAMP_H */
