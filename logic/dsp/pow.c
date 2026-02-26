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
 * pow.c
 *
 * this file implements the basic pow (power) block
 *  output = input to the power of x
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <plc.h>
#include <misc/string_util.h>
#include "pow.h"

/*
 * This is a power block  implementation.
 *
 * Limitations:
 */


static const int debug = 0;


int pow_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 f32 tmp_out, input;

 input = plc_get_f32(parm->pow.in_pt);
 tmp_out = pow(input, parm->pow.in_pt_pow);

 plc_set_f32 (parm->pow.out_pt, tmp_out);

 if (debug)
   printf("pow_step(): out=%f\n", tmp_out);

 return 0;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int pow_parse_parm (const char *table_name,
                    int table_index,
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */)
/*
 * pow function matplc.conf syntax:
 *   fblock pow <out_pt> <in_pt> <in_pt_pow>
 *
 * where:
 *   out_pt    : matplc point to be used as output for the pid block
 *   in_pt     : matplc point to be used as input for the pow block
 *   in_pt_pow : raise in_pt to in_pt_pow (f32 value)
 *
 *                in_pt_pow
 *   out_pt = in_pt
 */
/* NOTE: the "fblock" in the row is really the value  */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h ) */
/* NOTE: the "pow" in the row is really the value of */
/* the POW_TABLE_ID constant.                        */
{
  char *tmp_val;
  int rowlen;

  if (debug)
    printf("pow_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if ((rowlen < 4) || (rowlen > 4)) {
    plc_log_wrnmsg(1,
                   "fblock row %d has wrong number of parameters for"
                   " a pow block.",
                   table_index);
    return -1;
  }

  /* check that the block is really a pow block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, POW_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of an pow function block.",
                   table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->pow.out_pt = plc_pt_by_name(tmp_val);
  if (parm->pow.out_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid output point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  parm->pow.in_pt = plc_pt_by_name(tmp_val);
  if (parm->pow.in_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid input point %s",
                   tmp_val);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get in_pt_pow */
  if (conffile_get_table_f32(table_name, table_index, 3,
                             &(parm->pow.in_pt_pow),
                             -f32_MAX, f32_MAX, POW_DEF_POW)
      < 0) {
    tmp_val = conffile_get_table(table_name, table_index, 3);
    plc_log_wrnmsg(1,
                   "Invalid power value (%s)",
                   tmp_val); 
    free (tmp_val);
    return -1;
  }

  if (debug)
    pow_dump_parm(*parm);

  return 0;

}



int pow_dump_parm(fb_parm_t parm)
{
 printf("pow_dump_parm(): in_pts = %s, out_pt = %s, pow = %f\n",
        "", "", parm.pow.in_pt_pow);

 return 0;
}


