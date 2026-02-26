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


#ifndef __ALARM_T_H
#define __ALARM_T_H

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/

#define __MAX_NUM_ALARMS 10

typedef enum {
  greater_lt,
  greater_or_equal_lt,
  smaller_lt,
  smaller_or_equal_lt,
  equal_lt,
  not_equal_lt,
  invalid_lt              /* invalid limit type */
} limit_type_t;


typedef enum {
  true_val,
  absolute_val
} val_handling_t;


typedef struct {
    /* Output plc points */
  plc_pt_t out_pt[__MAX_NUM_ALARMS];

    /* Input plc point */
  plc_pt_t in_pt;
  val_handling_t in_pt_val_handling;

    /* Algorithm Parameters */
  f32          limit_val[__MAX_NUM_ALARMS];
  limit_type_t limit_type[__MAX_NUM_ALARMS];
  int      num_alarms;

} fb_alarm_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining an alarm  block
 */
#define ALARM_TABLE_ID "alarm"



#endif /* __ALARM_T_H */
