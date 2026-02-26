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


#ifndef __ALARM_PRIVATE_H
#define __ALARM_PRIVATE_H

/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* Default parameter values */
   /* default is to add all inputs */

#define ALARM_DEF_LIMIT_VAL 0

#define ALARM_VAL_HANDLING_TRUE_STR "true_val"
#define ALARM_VAL_HANDLING_ABS_STR  "abs_val"
#define ALARM_DEF_VAL_HANDLING     true_val
#define ALARM_DEF_VAL_HANDLING_STR ALARM_VAL_HANDLING_TRUE_STR

typedef struct {
  limit_type_t type;
  const char * str;
} limit_type_strings_t;

static limit_type_strings_t limit_type_strings[] = {
         {greater_lt, "greater"},
         {greater_lt, "gt"},
         {greater_or_equal_lt, "greater_or_equal"},
         {greater_or_equal_lt, "ge"},
         {smaller_lt, "smaller"},
         {smaller_lt, "st"},
         {smaller_lt, "less"},
         {smaller_lt, "lt"},
         {smaller_or_equal_lt, "smaller_or_equal"},
         {smaller_or_equal_lt, "se"},
         {smaller_or_equal_lt, "less_or_equal"},
         {smaller_or_equal_lt, "le"},
         {equal_lt, "equal"},
         {equal_lt, "eq"},
         {not_equal_lt, "not_equal"},
         {not_equal_lt, "ne"},
         {invalid_lt, NULL}      /* end of list */
       };



#endif /* __ALARM_PRIVATE_H */
