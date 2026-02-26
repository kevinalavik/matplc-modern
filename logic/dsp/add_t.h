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


#ifndef __ADD_T_H
#define __ADD_T_H

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/

#define __ADD_MAX_NUM_IN_PTS 10

typedef struct {
    /* Input plc points */
  plc_pt_t in_pt[__ADD_MAX_NUM_IN_PTS];

    /* Output plc points */
  plc_pt_t out_pt;

    /* Algorithm Parameters */
  f32 in_pt_mult[__ADD_MAX_NUM_IN_PTS];
  int      num_pts;

} fb_add_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining a pid block
 */
#define ADD_TABLE_ID "add"


#endif /* __ADD_T_H */
