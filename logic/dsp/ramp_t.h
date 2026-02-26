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


#ifndef __RAMP_T_H
#define __RAMP_T_H

#include "time_util.h"

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/

typedef struct {
    /* Input plc points */
  plc_pt_t in_pt;

    /* Output plc points */
  plc_pt_t out_pt;

    /* Algorithm Parameters */
    /* maximum values of first  derivative */
  f32 pos_dxdt, neg_dxdt;
    /* maximum values of second derivative */
  f32 pos_d2xdt2, neg_d2xdt2;

    /* State variables */
  long double prev_d_time;
  long double prev_out_dxdt; /* previous dxdt of the output */
  long double prev_in_dxdt;  /* previous dxdt of the input  */
  f32         prev_in;       /* previous input value        */

} fb_ramp_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining a ramp block
 */
#define RAMP_TABLE_ID "ramp"


#endif /* __RAMP_T_H */
