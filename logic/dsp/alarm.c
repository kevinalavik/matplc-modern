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

/*
 * alarm.c
 *
 * this file implements the basic alarm block
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>

#include <plc.h>
#include <misc/string_util.h>
#include "alarm.h"
#include "alarm_private.h"

/*
 * This is a alarm block implementation.
 *
 * Limitations:
 *  Maximum number of limits is defined in alarm_t.h
 */


static const int debug = 0;


int alarm_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 int index;
 f32 tmp_f32;
 u32 tmp_u32;

 tmp_f32 = plc_get_f32(parm->alarm.in_pt);

 if (parm->alarm.in_pt_val_handling == absolute_val)
   if (tmp_f32 < 0)
     tmp_f32 *= -1;

 for (index = 0;
      index < parm->alarm.num_alarms;
      index++) {
 switch(parm->alarm.limit_type[index]) {
   case greater_lt:
     tmp_u32 = (tmp_f32 > parm->alarm.limit_val[index])?1:0; break;
   case greater_or_equal_lt:
     tmp_u32 = (tmp_f32 >= parm->alarm.limit_val[index])?1:0; break;
   case smaller_lt:
     tmp_u32 = (tmp_f32 < parm->alarm.limit_val[index])?1:0; break;
   case smaller_or_equal_lt:
     tmp_u32 = (tmp_f32 <= parm->alarm.limit_val[index])?1:0; break;
   case equal_lt:
     tmp_u32 = (tmp_f32 == parm->alarm.limit_val[index])?1:0; break;
   case not_equal_lt:
     tmp_u32 = (tmp_f32 != parm->alarm.limit_val[index])?1:0; break;
   case invalid_lt: default:
     tmp_u32 = 0; break; /* this should never occur */ 
   } /* switch */

 plc_set (parm->alarm.out_pt[index], tmp_u32);
 } /* for */


 if (debug)
   printf("alarm_step(): in=%f\n", tmp_f32);

 return 0;
}





/* helper function for the parse_parm() function... */
static int string_to_limit_type(const char * str, limit_type_t *lt_ptr)
{
  int limit_type_str_count;

  for (limit_type_str_count = 0;
       limit_type_strings[limit_type_str_count].str != NULL;
       limit_type_str_count++) {

    if ( strcmp (str, limit_type_strings[limit_type_str_count].str) == 0) {
      *lt_ptr = limit_type_strings[limit_type_str_count].type;
      return 0;
    }
  } /* for(block_type_count) */

  /* not found, return error... */
  return -1;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int alarm_parse_parm (const char *table_name,
                      int table_index,
                      fb_parm_t *parm,
		      f32 T /* period this dsp module will work with */)
/*
 * alarm function matplc.conf syntax:
 *   fblock alarm in_pt {true_val | abs_val} out_pt1 limit_type_1 limit_val_1 [out_pt2 limit_type_2 limit_val_2] ...
 *
 * where:
 *   in_pt     : matplc point to be used as input for the alarm block
 *   true_val  : use the value in in_pt without any changes
 *   abs_val   : use the absolute value in in_pt for determining the alarms
 *   out_pt    : matplc point to be used as output for the alarm block
 *   limit_val : f32 value used for the alarm comparison
 *   limit_type: specifies when the alarm should be set.
 *               one of:
 *               {less | lt | smaller | st | less_or_equal | le | smaller_or_equal | se |
 *                greater | gt | greater_or_equal | ge |
 *                equal | eq | not_equal | ne}
 *               Note: less, lt, smaller, and st, are all equivalent
 *                     greater and gt are equivalent
 *                     greater_or_equal and ge are equivalent
 *                     etc...
 *
 * Example:
 *   fblock alarm in_pt out_1 10 lt out_2 10 gt out_3 20.55 eq
 *
 *          consider in_pt_val the value currently stored in the in_pt PLC point.
 *          then the above config line will have the efect of:
 *             out_1 being true (1) when in_pt_val < 10, and 0 otherwise
 *             out_2 being true (1) when in_pt_val > 10, and 0 otherwise
 *             out_1 being true (1) when in_pt_val = 20.55, and 0 otherwise
 *
 */
/* NOTE: the "fblock" in the row is really the value   */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h )  */
/* NOTE: the "alarm" in the row is really the value of */
/* the ALARM_TABLE_ID constant.                        */
{
  char *tmp_val;
  int rowlen, pt_count;

  if (debug)
    printf("alarm_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if ((rowlen < 2) || (rowlen > (2 + 3 * __MAX_NUM_ALARMS))) {
    plc_log_wrnmsg(1,
                   "fblock row %d has wrong number of parameters for"
                   " an %slock.",
                   table_index, ALARM_TABLE_ID);
    return -1;
  }

  /* check that the block is really an alarm block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, ALARM_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of an %s function block.",
                   table_index, ALARM_TABLE_ID);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->alarm.in_pt = plc_pt_by_name(tmp_val);
  if (parm->alarm.in_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid input point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point value handling */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  if (strcmp(tmp_val, ALARM_VAL_HANDLING_TRUE_STR) == 0) {
    parm->alarm.in_pt_val_handling = true_val;
  }
  else if (strcmp(tmp_val, ALARM_VAL_HANDLING_ABS_STR) == 0) {
    parm->alarm.in_pt_val_handling = absolute_val;
  }
  else {
    plc_log_wrnmsg(1,
                   "Invalid input value handling %s. "
                   "Reverting to default of %s.",
                   tmp_val, ALARM_DEF_VAL_HANDLING_STR);
    parm->alarm.in_pt_val_handling = ALARM_DEF_VAL_HANDLING;
  }
  free (tmp_val);

  /* get the output points */
  parm->alarm.num_alarms = 0;
  for (pt_count = 0; pt_count < __MAX_NUM_ALARMS; pt_count++) {

    /* get out_pt */
    tmp_val = conffile_get_table(table_name, table_index, 3 + pt_count*3);
    if (tmp_val == NULL)
      break; /* exit loop */
    parm->alarm.out_pt[parm->alarm.num_alarms] = plc_pt_by_name(tmp_val);
    if (parm->alarm.out_pt[parm->alarm.num_alarms].valid == 0) {
      plc_log_wrnmsg(1,
                     "Invalid output point %s. "
                     "Skipping this output point.",
                     tmp_val);
      free (tmp_val);
      continue;
    }
    free (tmp_val);

    /* get in_pt limit_type */
    tmp_val = conffile_get_table(table_name, table_index, 4 + pt_count*3);
    if (tmp_val == NULL) {
      tmp_val = conffile_get_table(table_name, table_index, 3 + pt_count*3);
      plc_log_wrnmsg(1,
                     "No limit type specified for output point %s. "
                     "Skipping this point.",
                     tmp_val);
      free (tmp_val);
      continue;
    }
    if (string_to_limit_type(tmp_val, &(parm->alarm.limit_type[parm->alarm.num_alarms]))
        < 0) {
      plc_log_wrnmsg(1,
                     "Invalid limit type %s. Skipping this output point.",
                     tmp_val);
      free (tmp_val);
      continue;
    }
    free (tmp_val);

    /* get in_pt limit_val */
    if (conffile_get_table_f32(table_name, table_index, 5 + pt_count*3,
                               &(parm->alarm.limit_val[parm->alarm.num_alarms]),
                               -f32_MAX, f32_MAX, ALARM_DEF_LIMIT_VAL)
        < 0) {
      tmp_val = conffile_get_table(table_name, table_index, 3 + pt_count*3);
      plc_log_wrnmsg(1,
                     "Invalid limit value for output point %s. "
                     "Skipping this output point.",
                     tmp_val);
      free (tmp_val);
      continue;
    }

    (parm->alarm.num_alarms)++;
  } /* for(..) */

  if (debug)
    alarm_dump_parm(*parm);

  return 0;
}



int alarm_dump_parm(fb_parm_t parm)
{
 int index;

 printf("alarm_dump_parm(): num_pts = %d\n",
        parm.alarm.num_alarms);

 for (index = 0; index < parm.alarm.num_alarms; index++) {
 printf("out_pt %d limit value = %f\n", index, parm.alarm.limit_val[index]);
 }

 return 0;
}

