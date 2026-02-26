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
 * nonlinear.c
 *

  This function block implements a nonlinear block supporting both
  a deadband function centered around a configured offset, and a limiter.
  In addition, it also allows for a linear gain and offset top be applied
  the output of it's nonlinear function. These last two are essentially to
  reduce the number of function blocks eventually required to implement
  a specific global user function.



  The nonlinear part, implements the following function:


     out = nl_f(in):
			   out
			    ^
			    |
			    |
		     co_top |.............................--------
			    |                            /
			    |                           /
			    |                          /
			    |                         /
			    |                        /
			    |                       /
			    |                      /
			    |                     / inclination = 1
			    |                    /
			    |                   /
		     db_out |...----------------
			    |  /.              .
			    | / .              .
			    |/  .              .
			    |   .              .
		   	   /|   .              .
		          / |   .              .
		         /  |   .              .
		        /   |   .              .
  <--------------------------------------------------------------> in
		      /     | db_bot         db_top
		     /      |
		    /       |
		   /        |
   ----------------.........| co_bot
			    |
			    |
			    |
			    v

	(co = cutoff      db = deadband)


  The linear part implements the following function:
   out = l_f(in) = in * gain


  The output of the complete nonlinear block is:
   out =  nl_f( in * gain )
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
#include "typeconv.h"

/*
 * This is a nonlinear block implementation.
 *
 * Limitations:
 *
 */


static const int debug = 0;


int nonlinear_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 f32 tmp_f32, input;

 if (debug)
   printf("nonlinear_step():...\n");

 tmp_f32 = input = plc_get_f32(parm->nonlinear.in_pt);

 tmp_f32 *= parm->nonlinear.abs_gain;
 tmp_f32 += parm->nonlinear.deadband_out - parm->nonlinear.deadband_bot; /* i.e. tmp_f32 += abs_offset; */;

 if (tmp_f32 > parm->nonlinear.deadband_out) {
   tmp_f32 -= parm->nonlinear.deadband_top - parm->nonlinear.deadband_bot;
   if (tmp_f32 < parm->nonlinear.deadband_out)
      tmp_f32 = parm->nonlinear.deadband_out;
   }

 if (tmp_f32 < parm->nonlinear.cutoff_bot)
    tmp_f32 = parm->nonlinear.cutoff_bot;

 if (tmp_f32 > parm->nonlinear.cutoff_top)
    tmp_f32 = parm->nonlinear.cutoff_top;

 plc_set_f32 (parm->nonlinear.out_pt, tmp_f32);

 if (debug)
   printf("nonlinear_step(): input=%f, output=%f\n",
          input, tmp_f32);

 return 0;
}



/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int nonlinear_parse_parm (const char *table_name,
                          int table_index,
                          fb_parm_t *parm,
		          f32 T /* period this dsp module will work with */)
/*
 * nonlinear function matplc.conf syntax:
 *   fblock nonlinear in_pt out_pt [cutoff_top xx] [cutoff_bot xx]
 *             [deadband_top xx] [deadband_bot xx] [deadband_out xx]
 *             [gain xx]
 *
 * where:
 *   in_pt        : matplc point to be used as input
 *   out_pt       : matplc point to be used as output
 *   cutoff_top   : value to use for this parameter - defaults to +f32_MAX
 *   cutoff_bot   : value to use for this parameter - defaults to -f32_MAX
 *   deadband_bot : value to use for this parameter - defaults to 0
 *   deadband_top : value to use for this parameter - defaults to 0
 *   deadband_out : value to use for this parameter - defaults to 0
 *   gain         : value to use for this parameter - defaults to 1
 *
 *   Note: parameter values may appear in any order, or not at all.
 *
 */
/* NOTE: the "fblock" in the row is really the value */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h) */
/* NOTE: the "nonlinear" in the row is really the    */
/* value of the NONLINEAR_TABLE_ID constant.         */
{
  char *tmp_val;
  f32  *tmp_f32_ptr;
  int rowlen, index;

  if (debug)
    printf("nonlinear_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if (rowlen < 3) {
    plc_log_wrnmsg(1,
                   "fblock row %d has wrong number of parameters for"
                   " an %s block.",
                   table_index, NONLINEAR_TABLE_ID);
    return -1;
  }

  /* check that the block is really a nonlinear block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, NONLINEAR_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of a %s function block.",
                   table_index, NONLINEAR_TABLE_ID);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->nonlinear.in_pt = plc_pt_by_name(tmp_val);
  if (parm->nonlinear.in_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid input point %s in fblock row %d. Skipping this fblock.",
                   tmp_val, table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  parm->nonlinear.out_pt = plc_pt_by_name(tmp_val);
  if (parm->nonlinear.out_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid output point %s in fblock row %d. Skipping this fblock.",
                   tmp_val, table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* setup the default parameters */
  parm->nonlinear.cutoff_top   = CUTOFF_TOP_DEF;
  parm->nonlinear.cutoff_bot   = CUTOFF_TOP_DEF;
  parm->nonlinear.deadband_top = DEADBAND_TOP_DEF;
  parm->nonlinear.deadband_bot = DEADBAND_BOT_DEF;
  parm->nonlinear.deadband_out = DEADBAND_OUT_DEF;
  parm->nonlinear.abs_gain     = ABS_GAIN_DEF;

  /* get the parameter pairs */
  for (index = 0;
       index < rowlen;
       index++) {

    tmp_val = conffile_get_table(table_name, table_index, 3 + index*2);
    if (tmp_val == NULL)
      break;

    if      (strcmp(tmp_val, CUTOFF_TOP_ID)   == 0) tmp_f32_ptr = &parm->nonlinear.cutoff_top;
    else if (strcmp(tmp_val, CUTOFF_BOT_ID)   == 0) tmp_f32_ptr = &parm->nonlinear.cutoff_bot;
    else if (strcmp(tmp_val, DEADBAND_TOP_ID) == 0) tmp_f32_ptr = &parm->nonlinear.deadband_top;
    else if (strcmp(tmp_val, DEADBAND_BOT_ID) == 0) tmp_f32_ptr = &parm->nonlinear.deadband_bot;
    else if (strcmp(tmp_val, DEADBAND_OUT_ID) == 0) tmp_f32_ptr = &parm->nonlinear.deadband_out;
    else if (strcmp(tmp_val, ABS_GAIN_ID)     == 0) tmp_f32_ptr = &parm->nonlinear.abs_gain;
    else {
      plc_log_wrnmsg(1,
                     "Invalid parameter identification %s, "
                     "skipping this parameter.",
                     tmp_val);
      continue;
      }

    if (conffile_get_table_f32(table_name, table_index, 4 + index*2,
                               tmp_f32_ptr, -f32_MAX, f32_MAX, *tmp_f32_ptr) < 0) {
      plc_log_wrnmsg(1,
                     "Invalid parameter value %s, "
                     "skipping this parameter.",
                     tmp_val);
      continue;
      }

    /* parameter pair read correctly, go to next parameter... */
  } /* for(..) */

  if (debug)
    nonlinear_dump_parm(*parm);

  return 0;
}



int nonlinear_dump_parm(fb_parm_t parm)
{
 printf("nonlinear_dump_parm(): cutoff_top=%f, cutoff_bot=%f, "
        "deadband_top=%f, deadband_bot=%f, deadband_out=%f"
        "gain=%f\n",
        parm.nonlinear.cutoff_top,
        parm.nonlinear.cutoff_bot,
        parm.nonlinear.deadband_top,
        parm.nonlinear.deadband_bot,
        parm.nonlinear.deadband_out,
        parm.nonlinear.abs_gain);

 return 0;
}

