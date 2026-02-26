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
 * add.c
 *
 * this file implements the basic add block
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
#include "add.h"

/*
 * This is a adding block  implementation.
 *
 * Limitations:
 *  Maximum number of inputs is defined in add_t.h
 */


static const int debug = 0;


int add_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 int index;
 f32 tmp_out;

 for (tmp_out = 0, index = 0;
      index < parm->add.num_pts;
      index++) {
 tmp_out += parm->add.in_pt_mult[index] * plc_get_f32(parm->add.in_pt[index]);
 } /* for */

 plc_set_f32 (parm->add.out_pt, tmp_out);

 if (debug)
   printf("add_step(): out=%f\n", tmp_out);

 return 0;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int add_parse_parm (const char *table_name,
                    int table_index,
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */)
/*
 * add function matplc.conf syntax:
 *   fblock add out_pt in_pt1 in_pt_mult1 [in_pt2 in_pt_2_mult] ...
 *
 * where:
 *   in_pt : matplc point to be used as input for the add block
 *   out_pt: matplc point to be used as output for the pid block
 *
 */
/* NOTE: the "fblock" in the row is really the value */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h )*/
/* NOTE: the "add" in the row is really the value of */
/* the ADD_TABLE_ID constant.                        */
{
  char *tmp_val;
  int rowlen, pt_count;

  if (debug)
    printf("add_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if ((rowlen < 3) || (rowlen > (2 + 2*__ADD_MAX_NUM_IN_PTS))) {
    plc_log_wrnmsg(1,
                   "fblock row %d has wrong number of parameters for"
                   " an add block.",
                   table_index);
    return -1;
  }

  /* check that the block is really a add block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, ADD_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of an add function block.",
                   table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->add.out_pt = plc_pt_by_name(tmp_val);
  if (parm->add.out_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid output point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input points */
  parm->add.num_pts = 0;
  for (pt_count = 0; pt_count < __ADD_MAX_NUM_IN_PTS; pt_count++) {

    /* get in_pt */
    tmp_val = conffile_get_table(table_name, table_index, 2 + pt_count*2);
    if (tmp_val == NULL)
      break; /* exit loop */
    parm->add.in_pt[pt_count] = plc_pt_by_name(tmp_val);
    if (parm->add.in_pt[pt_count].valid == 0) {
      plc_log_wrnmsg(1,
                     "Invalid input point %s",
                     tmp_val);
      free (tmp_val);
      return -1;
    }
    free (tmp_val);

    (parm->add.num_pts)++;

    /* get in_pt multiplier */
    if (conffile_get_table_f32(table_name, table_index, 3 + pt_count*2,
                               &(parm->add.in_pt_mult[pt_count]),
                               -f32_MAX, f32_MAX, ADD_DEF_MULT)
        < 0) {
      tmp_val = conffile_get_table(table_name, table_index, 2 + pt_count*2);
      plc_log_wrnmsg(1,
                     "Invalid multiplcation value for input point %s",
                     tmp_val);
      free (tmp_val);
      return -1;
    }
  } /* for(..) */

  if (debug)
    add_dump_parm(*parm);

  return 0;
}



int add_dump_parm(fb_parm_t parm)
{
 int index;

 printf("add_dump_parm(): num_pts = %d\n",
        parm.add.num_pts);

 for (index = 0; index < parm.add.num_pts; index++) {
 printf("in_pt%d_mult = %f\n", index, parm.add.in_pt_mult[index]);
 }

 return 0;
}

