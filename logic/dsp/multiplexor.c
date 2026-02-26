/*
 * (c) 2001 Mario de Sousa
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
 * multiplexor.c
 *
 * this file implements the basic multiplexor block
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
#include "multiplexor.h"

/*
 * This is a multiplexor block  implementation.
 *
 * Limitations:
 *  Maximum number of inputs is defined in multiplexor_t.h
 */


static const int debug = 0;


int multiplexor_step(unsigned int step_count, 
		     dsp_time_t *dsp_time, 
	             fb_parm_t *parm)
{
 int scan_index, out_index;
 f32 ctrl;

 ctrl = plc_get(parm->multiplexor.ctrl_pt);

 /* figure out which point should be copied to the output... */
 for (out_index = 0, scan_index = 0;
      scan_index < parm->multiplexor.num_pts - 1;
      scan_index++) {
   if (ctrl >= parm->multiplexor.limit[scan_index])
     out_index = scan_index + 1;
 } /* for */

 plc_set (parm->multiplexor.out_pt, 
          plc_get(parm->multiplexor.in_pt[out_index]));

 return 0;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int multiplexor_parse_parm (const char *table_name,
                    int table_index,
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */)
/*
 * multiplexor function matplc.conf syntax:
 *   fblock multiplexor <out_pt> <ctrl_pt> <in_pt1> [<limit1> <in_pt2>] ...
 *
 * where:
 *   out_pt : matplc point to be used as output for the multiplexor block
 *   ctrl_pt: matplc point to used to decide which input to copy to the output
 *   in_ptx : matplc point to be used as input for the multiplexor block
 *   limitx : limit value at which the output switches from one input to another
 *
 *   out_pt = in_pt1 -> if (ctrl_pt <  limit1)
 *   out_pt = in_pt2 -> if (ctrl_pt >= limit1) AND (ctrl_pt < limit2)
 *   out_pt = in_pt3 -> if (ctrl_pt >= limit2) AND (ctrl_pt < limit3)
 *   ...
 */
/* NOTE: the "fblock" in the row is really the value  */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h ) */
/* NOTE: the "multiplexor" in the row is really the value of */
/* the MULTIPLEXOR_TABLE_ID constant.                        */
{
  char *tmp_val;
  int rowlen, pt_count;

  if (debug)
    printf("multiplexor_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if ((rowlen < 3) || (rowlen > (2 + 2*__MULTIPLEXOR_MAX_NUM_IN_PTS))) {
    plc_log_wrnmsg(1,
                   "fblock row %d has wrong number of parameters for"
                   " a multiplexor block.",
                   table_index);
    return -1;
  }

  /* check that the block is really a multiplexor block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, MULTIPLEXOR_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of an multiplexor function block.",
                   table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->multiplexor.out_pt = plc_pt_by_name(tmp_val);
  if (parm->multiplexor.out_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid output point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the ctrl point */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  parm->multiplexor.ctrl_pt = plc_pt_by_name(tmp_val);
  if (parm->multiplexor.ctrl_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid control point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get first input point */
  tmp_val = conffile_get_table(table_name, table_index, 3);
  parm->multiplexor.in_pt[0] = plc_pt_by_name(tmp_val);
  if (parm->multiplexor.in_pt[0].valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid input point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the limits and the other input points */
  parm->multiplexor.num_pts = 1;
  for (pt_count = 1; pt_count < __MULTIPLEXOR_MAX_NUM_IN_PTS; pt_count++) {

    /* check if any more in_pt and limit */
    tmp_val = conffile_get_table(table_name, table_index, 3 + pt_count*2);
    if (tmp_val == NULL)
      break; /* exit loop */
    free (tmp_val); 

    /* get limit x */
    if (conffile_get_table_f32(table_name, table_index, 2 + pt_count*2,
                               &(parm->multiplexor.limit[pt_count-1]),
                               -f32_MAX, f32_MAX, 
			       /* the default value is indifferent */
                               /* It will never get used!          */
			       0)
        < 0) {
      tmp_val = conffile_get_table(table_name, table_index, 2 + pt_count*2);
      plc_log_wrnmsg(1,
                     "Invalid limit value %s",
                     tmp_val);
      free (tmp_val);
      return -1;
    }

    /* get in_pt */
    tmp_val = conffile_get_table(table_name, table_index, 3 + pt_count*2);
      /* actually this should never occur, it's been checked above,
       * but we might just as well check again...
       */
    if (tmp_val == NULL)
      return -1; /* strange happenings... */
    parm->multiplexor.in_pt[pt_count] = plc_pt_by_name(tmp_val);
    if (parm->multiplexor.in_pt[pt_count].valid == 0) {
      plc_log_wrnmsg(1,
                     "Invalid input point %s",
                     tmp_val);
      free (tmp_val);
      return -1;
    }
    free (tmp_val);

    (parm->multiplexor.num_pts)++;

  } /* for(..) */

  if (debug)
    multiplexor_dump_parm(*parm);

  return 0;
}



int multiplexor_dump_parm(fb_parm_t parm)
{
 int index;

 printf("multiplexor_dump_parm(): num_pts = %d\n",
        parm.multiplexor.num_pts);

 for (index = 0; index < parm.multiplexor.num_pts - 1; index++) {
 printf("limit %d = %f\n", index, parm.multiplexor.limit[index]);
 }

 return 0;
}



