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
 * ramp.c
 *
 * this file implements the first and second derivative limiter
 *  usually called the ramp function
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <math.h>

#include <plc.h>
#include <misc/string_util.h>

#include "time_util.h"
#include "ramp.h"

/*
 * This is a first and second derivative limiter implementation.
 *
 * Limitations:
 *
 */


static const int debug = 0;


int ramp_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 f32 tmp_f32;
 long double max_out_d2xdt2, max_neg_out_d2xdt2, max_out_dxdt, max_out;
 long double d_time, in, out;
 long double in_dxdt, in_d2xdt2, tmp_ld;

 d_time = dsp_time->ld_dif;

  /* read the 'in' value, which is also the current value */
 in = plc_get_f32(parm->ramp.in_pt);

  /* read the 'out' value, which is also the previous value */
 out = plc_get_f32(parm->ramp.out_pt);

  /* if it is the first iteration, then previous value is still invalid */
 if (step_count == 0) {
   out = in;
   parm->ramp.prev_in = in;
 }

  /* determine which d2xdt2 limit will be applied */
 if (out + d_time * parm->ramp.prev_out_dxdt > in ) {
   /* use negative d2xdt2 */
   max_out_d2xdt2     = parm->ramp.neg_d2xdt2;
   max_neg_out_d2xdt2 = parm->ramp.pos_d2xdt2;
 }
 else {
   /* use positive d2xdt2 */
   max_out_d2xdt2     = parm->ramp.pos_d2xdt2;
   max_neg_out_d2xdt2 = parm->ramp.neg_d2xdt2;
 }

  /* determine the maximum dxdt allowed for this iteration    */
  /*  this will be the lowest value out of the three limits:  */
  /*  A - parameter specified maximum dxdt limit              */
  /*  B - dxdt imposed by the maximum d2xdt2 limit            */
  /* d2xdt2 = 2 * (dxdt - prev_dxdt) / (d_time + prev_d_time) */
  /*  C - dxdt so we don't overshoot the input. This may occur*/
  /*       when we are limiting the d2xdt2 and the output     */
  /*       aproaches the input value with a very high dxdt    */

  /* Limit A */
 max_out_dxdt = parm->ramp.prev_out_dxdt + (max_out_d2xdt2 * (d_time + parm->ramp.prev_d_time) / 2);

  /* Limit B */
 if (max_out_dxdt > parm->ramp.pos_dxdt)
   max_out_dxdt = parm->ramp.pos_dxdt;
 if (max_out_dxdt < parm->ramp.neg_dxdt)
   max_out_dxdt = parm->ramp.neg_dxdt;

   /* Limit C */
  /* determine the maximum dxdt so that we don't overshoot */
  /*  What we are really doing here is garanteeing that if the
   *  output point Xo is to follow a parabolic curve at its
   *  maximum d2xdt2, it will reach the current input value
   *  Xi with the same dxdt that Xi will be experiencing
   *  when Xo reaches that value.
   *
   *  Actually what we do is:
   *   - consider the input Xi will continue with its present
   *     dxdt and d2xdt2. This means that we consider that Xi
   *     will follow a parabolic curve.
   *   - consider that the output value will follow a parabolic curve
   *     at its maximum d2xdt2.
   *   - If they are to converge at a single point in time, then they
   *     will reach that point with the same speed
   *     i.e. if the two parabolas are to intersect at a single point
   *     then they will be tangent to each other at that point.
   *
   *     If we write the two Xi and Xo equations, and determine when
   *     they intersect we have:
   *     Xo + Vo*t + 1/2*Ao*t*t = Xi + Vi*t + 1/2*Ai*t*t
   *     where Vo is the dxdt of the output
   *              (what we will be solving for, i.e. our unknown variable)
   *           Ao is the maximum d2xdt2 of the output
   *           Xo is the current output
   *           Xi the current input
   *           Vi the current dxdt of the input
   *           Ai the current d2xdt2 of the input
   *           t is the time
   *
   *     The above equation can be re-written as
   *       a*t*t + b*t + c = 0
   *
   *     If we want these two curves to intersect at only one point,
   *     then there can only be one value of t that may satisfy the equation.
   *     This means that b*b - 4*a*c = 0
   *     Solving for Vo we get:
   *      Vo = Vi + sqrt(  2 * (Xi - Xo) * (Ai - Ao)  )
   *         or
   *      Vo = Vi - sqrt(  2 * (Xi - Xo) * (Ai - Ao)  )
   *
   */

  /* determine the dxdt and d2xdt2 of the plc in point variable */
 in_dxdt   = (in - parm->ramp.prev_in) / d_time;
 in_d2xdt2 = 2 * (in_dxdt - parm->ramp.prev_in_dxdt) / (d_time + parm->ramp.prev_d_time);
  /* store the previous values related to the plc in point variable */
 parm->ramp.prev_in      = in;
 parm->ramp.prev_in_dxdt = in_dxdt;

 tmp_ld = sqrtl( 2 * (in - out) * (in_d2xdt2 - max_neg_out_d2xdt2));

/*
plc_log_trcmsg(1, "in=%10.15Lf, out=%10.15Lf, max_dxdt=%10.15Lf, Vo=%10.15Lf, in_d2xdt2=%10.15Lf max_d2xdt2=%10.15Lf \n",
                  in, out, max_out_dxdt, Vo, in_d2xdt2, max_out_d2xdt2);
*/



 if ((max_out_dxdt > in_dxdt + tmp_ld) && (max_out_d2xdt2 == parm->ramp.pos_d2xdt2))
   max_out_dxdt = in_dxdt + tmp_ld;
 if ((max_out_dxdt < in_dxdt - tmp_ld) && (max_out_d2xdt2 == parm->ramp.neg_d2xdt2))
   max_out_dxdt = in_dxdt - tmp_ld;

  /* Now that we have determined the maximum out_dxdt, */
  /* we now calculate the maximum output value         */

  /* determine the maximum allowed out value for this iteration */
 max_out = out + max_out_dxdt * d_time;

  /* place the new output value in the 'in' variable */
 if ((in > max_out) && (max_out_d2xdt2 == parm->ramp.pos_d2xdt2))
   in = max_out;
 /*
 else
   in = in;
 */
 if ((in < max_out) && (max_out_d2xdt2 == parm->ramp.neg_d2xdt2))
   in = max_out;
 /*
 else
   in = in;
 */

 if (step_count >= 1)
   parm->ramp.prev_out_dxdt = (in /* i.e. new_out */ - out /* prev_out */) / d_time;
 parm->ramp.prev_d_time = d_time;

 tmp_f32 = (f32)in;
 plc_set_f32 (parm->ramp.out_pt, tmp_f32);

 if (debug) {
   printf("in=%10.15f out=%10.15f dx_dt=%10.15Lf\n",
           plc_get_f32(parm->ramp.in_pt),
	   tmp_f32,
           parm->ramp.prev_out_dxdt);
 }

 return 0;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int ramp_parse_parm (const char *table_name,
                    int table_index,
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */)
/*
 * ramp function matplc.conf syntax:
 *   fblock ramp in_pt out_pt [dxdt xx] [pos_dxdt xx] [neg_dxdt xx]
 *               [d2xdt2 xx] [pos_d2xdt2 xx] [neg_d2xdt2 xx]
 *
 * where:
 *   in_pt      : matplc point to be used as input
 *   out_pt     : matplc point to be used as output
 *   dxdt       : max positive and negative dxdt of output value - defaults to f32_MAX
 *   pos_dxdt   : max positive dxdt of output value - defaults to f32_MAX
 *   neg_dxdt   : max negative dxdt of output value - defaults to f32_MAX
 *   d2xdt2     : max positive and negative d2xdt2 of output value - defaults to f32_MAX
 *   pos_d2xdt2 : max positive d2xdt2 of output value - defaults to f32_MAX
 *   neg_d2xdt2 : max negative d2xdt2 of output value - defaults to f32_MAX
 *
 *   Note: parameter values may appear in any order, or not at all.
 *         parameter values must all be >= 0
 *
 */
/* NOTE: the "fblock" in the row is really the value  */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h ) */
/* NOTE: the "ramp" in the row is really the value of */
/* the RAMP_TABLE_ID constant.                        */
{
  char *tmp_val;
  f32  *tmp_f32_ptr1, *tmp_f32_ptr2;
  int rowlen, index;

  if (debug)
    printf("ramp_parse_parm(): table=%s, index=%d\n",
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
                   table_index, RAMP_TABLE_ID);
    return -1;
  }

  /* check that the block is really a ramp block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, RAMP_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of a %s function block.",
                   table_index, RAMP_TABLE_ID);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->ramp.in_pt = plc_pt_by_name(tmp_val);
  if (parm->ramp.in_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid input point %s in fblock row %d. Skipping this fblock.",
                   tmp_val, table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  parm->ramp.out_pt = plc_pt_by_name(tmp_val);
  if (parm->ramp.out_pt.valid == 0) {
    plc_log_wrnmsg(1,
                   "Invalid output point %s in fblock row %d. Skipping this fblock.",
                   tmp_val, table_index);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* setup the default parameters */
  parm->ramp.pos_dxdt   = POS_DXDT_DEF;
  parm->ramp.neg_dxdt   = NEG_DXDT_DEF;
  parm->ramp.pos_d2xdt2 = POS_D2XDT2_DEF;
  parm->ramp.neg_d2xdt2 = NEG_D2XDT2_DEF;

  /* setup initial state variables */
  parm->ramp.prev_out_dxdt = 0;
  parm->ramp.prev_in_dxdt  = 0;
  parm->ramp.prev_d_time   = 0;
  parm->ramp.prev_in       = 0;  /* this is re-initialized in the ramp_step() function */


  /* get the parameter pairs */
  for (index = 0;
       index < rowlen;
       index++) {

    tmp_val = conffile_get_table(table_name, table_index, 3 + index*2);
    if (tmp_val == NULL)
      break;

    if      (strcmp(tmp_val, POS_DXDT_ID)   == 0) tmp_f32_ptr2 = tmp_f32_ptr1 = &parm->ramp.pos_dxdt;
    else if (strcmp(tmp_val, NEG_DXDT_ID)   == 0) tmp_f32_ptr2 = tmp_f32_ptr1 = &parm->ramp.neg_dxdt;
    else if (strcmp(tmp_val,     DXDT_ID)   == 0) {
           tmp_f32_ptr1 = &parm->ramp.pos_dxdt;
           tmp_f32_ptr2 = &parm->ramp.neg_dxdt;
	 }
    else if (strcmp(tmp_val, POS_D2XDT2_ID) == 0) tmp_f32_ptr2 = tmp_f32_ptr1 = &parm->ramp.pos_d2xdt2;
    else if (strcmp(tmp_val, NEG_D2XDT2_ID) == 0) tmp_f32_ptr2 = tmp_f32_ptr1 = &parm->ramp.neg_d2xdt2;
    else if (strcmp(tmp_val,     D2XDT2_ID) == 0) {
           tmp_f32_ptr1 = &parm->ramp.pos_d2xdt2;
           tmp_f32_ptr2 = &parm->ramp.neg_d2xdt2;
	 }
    else {
      plc_log_wrnmsg(1,
                     "Invalid parameter identification %s, "
                     "skipping this parameter.",
                     tmp_val);
      continue;
      }

    if (conffile_get_table_f32(table_name, table_index, 4 + index*2,
                               tmp_f32_ptr1, 0, f32_MAX, *tmp_f32_ptr1) < 0) {
      plc_log_wrnmsg(1,
                     "Invalid parameter value %s, "
                     "skipping this parameter.",
                     tmp_val);
      continue;
      }
    *tmp_f32_ptr2 = *tmp_f32_ptr1;

    /* parameter pair read correctly, go to next parameter... */
  } /* for(..) */

  /* set correct signs on neg_xxx parameters */
  parm->ramp.neg_dxdt   *= -1;
  parm->ramp.neg_d2xdt2 *= -1;

  if (debug)
    ramp_dump_parm(*parm);

  return 0;
}



int ramp_dump_parm(fb_parm_t parm)
{
 printf("ramp_dump_parm(): pos_dxdt=%f, neg_dxdt=%f, "
        "pos_d2xdt2=%f, neg_d2xdt2=%f\n",
        parm.ramp.pos_dxdt,
        parm.ramp.neg_dxdt,
        parm.ramp.pos_d2xdt2,
        parm.ramp.neg_d2xdt2);

 return 0;
}
