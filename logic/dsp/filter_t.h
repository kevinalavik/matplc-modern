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


#ifndef __FILTER_T_H
#define __FILTER_T_H

#include "time_util.h"

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/


typedef struct {
    /* Algorithm Parameters */
  f32 C;
  f32 A1, A2;
  f32 B1, B2;

    /* State variables */
    /* the previous 2 'intermediate' results */
  long double z1, z2;
} second_order_section_t;


typedef struct {
    /* Input plc points */
  plc_pt_t in_pt;

    /* Output plc points */
  plc_pt_t out_pt;

    /* Algorithm Parameters */
  second_order_section_t *section;
  int num_sections;

} fb_filter_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining a pid block
 */
#define FILTER_TABLE_ID "filter"


#endif /* __FILTER_T_H */
