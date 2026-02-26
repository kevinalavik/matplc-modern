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


#ifndef __TYPECONV_T_H
#define __TYPECONV_T_H

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/

#define __TYPECONV_MAX_NUM_PTS 5


typedef enum {
  tc_type_u32,
  tc_type_i32,
  tc_type_u16,
  tc_type_i16,
  tc_type_f32
} tc_type_t;


typedef struct {
    /* Input plc points */
  plc_pt_t  in_pt[__TYPECONV_MAX_NUM_PTS];

    /* Output plc points */
  plc_pt_t out_pt[__TYPECONV_MAX_NUM_PTS];

    /* Algorithm Parameters */
  tc_type_t in_pt_type [__TYPECONV_MAX_NUM_PTS];
  tc_type_t out_pt_type[__TYPECONV_MAX_NUM_PTS];

  int      num_pts;

} fb_typeconv_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining a pid block
 */
#define TYPECONV_TABLE_ID "typeconv"
#define TYPECONV_i16_STR  "i16"
#define TYPECONV_u16_STR  "u16"
#define TYPECONV_i32_STR  "i32"
#define TYPECONV_u32_STR  "u32"
#define TYPECONV_f32_STR  "f32"


#endif /* __TYPECONV_T_H */
