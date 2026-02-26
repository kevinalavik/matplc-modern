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


#ifndef __FILTER_H
#define __FILTER_H

#include "time_util.h"
#include "filter_t.h"
#include "dsp_t.h"

/**************************************************************/
/*****************                             ****************/
/*****************  Global (to dsp) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

int filter_parse_parm (const char *table_name,
                       int table_index,
                       fb_parm_t *parm,
		       f32 T /* period this dsp module will work with */);

int filter_step (unsigned int step_count,
                 dsp_time_t *dsp_time,
                 fb_parm_t *parm);

/* mainly used for debugging */
int filter_dump_parm (fb_parm_t parm);


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* Default parameter values */

#define IIR_FILTER_APROX_ID "iir"
#define FIR_FILTER_APROX_ID "fir"
#define BUTTERWORTH_FILTER_APROX_ID "butterworth"
#define CHEBYSHEV_FILTER_APROX_ID "chebyshev"
#define ELLIPTIC_FILTER_APROX_ID "elliptic"

#define  LOWPASS_FILTER_SHAPE_ID  "lowpass"
#define HIGHPASS_FILTER_SHAPE_ID "highpass"
#define BANDPASS_FILTER_SHAPE_ID "bandpass"
#define BANDSTOP_FILTER_SHAPE_ID "bandstop"



   /* default is to have the output follow the input */
#define FILTER_DEF_C  (1)
#define FILTER_DEF_B1 (0)
#define FILTER_DEF_B2 (0)
#define FILTER_DEF_A1 (0)
#define FILTER_DEF_A2 (0)

#define FILTER_DEF_GAIN 	(1)
#define FILTER_DEF_PASSBAND_TOL (3)
#define FILTER_DEF_STOPBAND_TOL (6)

#endif /* __FILTER_H */
