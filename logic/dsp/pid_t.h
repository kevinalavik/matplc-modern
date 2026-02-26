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


#ifndef __PID_T_H
#define __PID_T_H

#include "time_util.h"

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/

typedef struct {
    /* Input plc points */
  plc_pt_t aut_in_pt;  /* input point for automatic mode */
  plc_pt_t man_in_pt;  /* input point for manual mode    */
  plc_pt_t man_mode_pt;  /* input point to select manual mode    */

    /* Output plc points */
  plc_pt_t out_pt;

    /* Algorithm Parameters */
  f32 P;
  f32 I;
  f32 D;
  f32 MinOut;
  f32 MaxOut;

    /* State variables */
  long double input_integral;
  long double prev_input;

} fb_pid_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining a pid block
 */
#define PID_TABLE_ID "pid"


#endif /* __PID_T_H */
