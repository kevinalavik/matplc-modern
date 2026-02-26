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
 * pid.c
 *
 * this file implements the basic pid block
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

#include "time_util.h"
#include "pid.h"
#include "dsp.h" /* for FBLOCK_TABLE_ID */

/*
 * This is a quick-n-dirty pid controller implementation.
 *
 * Limitations:
 *
 */


static const int debug = 0;


int pid_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 f32 out_f32;
 long double d_time, in, out, P, D;

 d_time = dsp_time->ld_dif;

 in = plc_get_f32(parm->pid.aut_in_pt);

 parm->pid.input_integral +=  in * d_time;

 if (step_count == 0)
   D = 0;
 else
   D = (in - parm->pid.prev_input) * (parm->pid.D / d_time);

 P = (parm->pid.P * in);

 out = P + D + (parm->pid.input_integral * parm->pid.I);

 if (out > parm->pid.MaxOut) {
   out = parm->pid.MaxOut;
   if (parm->pid.I != 0)
     parm->pid.input_integral = (out - P - D) / parm->pid.I;
 }

 if (out < parm->pid.MinOut) {
   out = parm->pid.MinOut;
   if (parm->pid.I != 0)
     parm->pid.input_integral = (out - P - D) / parm->pid.I;
 }

 if (plc_get(parm->pid.man_mode_pt)) {
   out = plc_get_f32(parm->pid.man_in_pt);
   if (parm->pid.I != 0)
     parm->pid.input_integral = (out - P - D) / parm->pid.I;
 }

 if (debug)
   printf("diff=%10.15Lf, di/dt * D=%10.15Lf, D=%10.15f\n",
           in - parm->pid.prev_input,
           (in - parm->pid.prev_input)*parm->pid.D/d_time,
           parm->pid.D);

 parm->pid.prev_input = in;

 out_f32 = (f32)out;
 plc_set_f32 (parm->pid.out_pt, out_f32);

 if (debug)
   printf("in=%10.15Lf, out=%10.15Lf, in_int=%10.15Lf\n",
           parm->pid.prev_input, out, parm->pid.input_integral);

 return 0;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int pid_parse_parm (const char *table_name,
                    int table_index,
                    fb_parm_t *parm,
		    f32 T /* period this dsp module will work with */)
/*
 * pid function matplc.conf syntax:
 *   fblock pid in_pt out_pt [P [I [D]]] [MinOut <lower_limit>] [MaxOut <upper_limit>]
 *
 * where:
 *   in_pt : matplc point to be used as input for the pid block
 *   out_pt: matplc point to be used as output for the pid block
 *   P     :
 *   I     :
 *   D     :
 *
 *   <lower_limit> : Minimum value that the pid block should have at it's output
 *   <upper_limit> : Maximum value that the pid block should have at it's output
 *
 */
/* NOTE: the "fblock" in the row is really the value */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h )*/
/* NOTE: the "pid" in the row is really the value of */
/* the PID_TABLE_ID constant.                        */
{
  char *tmp_val;
  int rowlen;
  int field;
  int P_already_parsed, I_already_parsed, D_already_parsed;
  int MinOut_already_parsed, MaxOut_already_parsed;
  int ManOut_already_parsed;
  int ManMode_already_parsed;
  char *tmp_str, *tmp_parm_str;

  if (debug)
    printf("pid_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if ((rowlen < 3) || (rowlen > 10)) {
    /* plc_log_wrnmsg("...) */
    return -1;
  }

  /* check that the block is really a pid block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, PID_TABLE_ID) != 0) {
    /* Row is not configuration of a pid function block */
    /* plc_log_wrnmsg("...) */
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the input point */
  tmp_val = conffile_get_table(table_name, table_index, 1);
  parm->pid.aut_in_pt = plc_pt_by_name(tmp_val);
  if (parm->pid.aut_in_pt.valid == 0) {
    /* invalid input point */
    /* plc_log_wrnmsg("...) */
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the output point */
  tmp_val = conffile_get_table(table_name, table_index, 2);
  parm->pid.out_pt = plc_pt_by_name(tmp_val);
  if (parm->pid.out_pt.valid == 0) {
    /* invalid output point */
    /* plc_log_wrnmsg("...) */
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /****************************************************************************/
  /* From the fourth field onwards, we have optional parameters, so we use a  */
  /*  field variable and loop though the user parameters...                   */
  /****************************************************************************/
    /* the field currently being parsed...*/
  field = 3;  /* first field is indexed as 0! */
    /* flags indicating we have not yet encountered a field specifying the       */
    /* P / I / D / MinOut / MaxOut parameter.                                    */
    /*  These flags are required because we do not have a keyword before the     */
    /*  P / I / D  fields, and to make sure multiple copies of the same parameter*/
    /*  are not parsed without giving out a warning...                           */
  P_already_parsed = 0;
  I_already_parsed = 0;
  D_already_parsed = 0;
  MinOut_already_parsed = 0;
  MaxOut_already_parsed = 0;
  ManOut_already_parsed  = 0;
  ManMode_already_parsed  = 0;
    /* Start off by initialising parameters to default values... */
  parm->pid.P = DEF_P;
  parm->pid.I = DEF_I;
  parm->pid.D = DEF_D;
  parm->pid.MinOut = DEF_MINOUT;
  parm->pid.MaxOut = DEF_MAXOUT;
  parm->pid.man_in_pt = plc_pt_null();
  parm->pid.man_mode_pt = plc_pt_null();

   /* OK. Now we get to do the loop... */
  while ((tmp_str = conffile_get_table(table_name, table_index, field++))
         != NULL) {

    /* See if we have the "MinOut" keyword... */
    if (strcmp(tmp_str, MINOUT_ID) == 0) {
      /*********************************************/
      /* parse the  MinOut <lower_limit> parameter */
      /*********************************************/

      /* read in <lower_limit> in the following field */
      if ((tmp_parm_str = conffile_get_table(table_name, table_index, field++))
           == NULL) {
          /* ERROR! No parameter given after the MinOut keyword! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "no parameter given after %s keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_str);
        field--;
        goto loop_end;
      }

      /*
       *  But if the MINOUT has already been specified, then this
       *  is an error...
       */
      if (MinOut_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field and it's parameter %s.",
                          tmp_str, FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_parm_str);
        free(tmp_parm_str); tmp_parm_str = NULL;
        goto loop_end;
      }

      MinOut_already_parsed = 1;

      /* now really parse the MinOut value */
      /* NOTE: the string_str_to_f32 does not change the value of pid->MinOut if
       *       it encounters an error parsing the tmp_parm_str
       */
      if (string_str_to_f32(tmp_parm_str, &(parm->pid.MinOut),
                            -f32_MAX,
                            parm->pid.MaxOut) /* MinOut must be <= MaxOut */
          < 0) {
          /* NO! -> ERROR! We cannot parse the MinOut value! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "error parsing the parameter %s "
                          "given after '%s' keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID,
                          tmp_parm_str, tmp_str);
      }

      free(tmp_parm_str); tmp_parm_str = NULL;
      goto loop_end;
    } /* parse MinOut <lower_limit> parameter */

    /* See if we have the "MaxOut" keyword... */
    if (strcmp(tmp_str, MAXOUT_ID) == 0) {
      /*********************************************/
      /* parse the  MaxOut <upper_limit> parameter */
      /*********************************************/

      /* read in <upper_limit> in the following field */
      if ((tmp_parm_str = conffile_get_table(table_name, table_index, field++))
           == NULL) {
          /* ERROR! No parameter given after the MinOut keyword! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "no parameter given after %s keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_str);
        field--;
        goto loop_end;
      }

      /*
       *  But if the MAXOUT has already been specified, then this
       *  is an error...
       */
      if (MaxOut_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field and it's parameter %s.",
                          tmp_str, FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_parm_str);
        free(tmp_parm_str); tmp_parm_str = NULL;
        goto loop_end;
      }

      MaxOut_already_parsed = 1;

      /* now really parse the MaxOut value */
      /* NOTE: the string_str_to_f32 does not change the value of pid->MaxOut if
       *       it encounters an error parsing the tmp_parm_str
       */
      if (string_str_to_f32(tmp_parm_str, &(parm->pid.MaxOut),
                            parm->pid.MinOut,  /* MaxOut must be >= MinOut */
                            f32_MAX)
          < 0) {
          /* NO! -> ERROR! We cannot parse the MaxOut value! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "error parsing the parameter %s "
                          "given after '%s' keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID,
                          tmp_parm_str, tmp_str);
      }

      free(tmp_parm_str); tmp_parm_str = NULL;
      goto loop_end;
    } /* parse MaxOut <sup_limit> parameter */

    /* See if we have the "ManOut" keyword... */
    if (strcmp(tmp_str, MANOUT_ID) == 0) {
      /***************************************/
      /* parse the  ManOut <plc_pt> parameter */
      /***************************************/

      /* read in <plc_pt> in the following field */
      if ((tmp_parm_str = conffile_get_table(table_name, table_index, field++))
           == NULL) {
          /* ERROR! No parameter given after the ManOut keyword! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "no parameter given after %s keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_str);
        field--;
        goto loop_end;
      }

      /*
       *  But if the ManOut has already been specified, then this
       *  is an error...
       */
      if (ManOut_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field and it's parameter %s.",
                          tmp_str, FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_parm_str);
        free(tmp_parm_str); tmp_parm_str = NULL;
        goto loop_end;
      }

      ManOut_already_parsed = 1;

      /* now really parse the <plc_pt> value */
      if ((parm->pid.man_in_pt = plc_pt_by_name(tmp_parm_str)).valid == 0) {
          /* NO! -> ERROR! We cannot parse the plc_pt value! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "unknown plc point %s given after '%s' keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID,
                          tmp_parm_str, tmp_str);
      }

      free(tmp_parm_str); tmp_parm_str = NULL;
      goto loop_end;
    } /* parse ManOut <plc_pt> parameter */

    /* See if we have the "ManMode" keyword... */
    if (strcmp(tmp_str, MANMODE_ID) == 0) {
      /***************************************/
      /* parse the  ManMode <plc_pt> parameter */
      /***************************************/

      /* read in <plc_pt> in the following field */
      if ((tmp_parm_str = conffile_get_table(table_name, table_index, field++))
           == NULL) {
          /* ERROR! No parameter given after the ManMode keyword! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "no parameter given after %s keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_str);
        field--;
        goto loop_end;
      }

      /*
       *  But if the ManMode has already been specified, then this
       *  is an error...
       */
      if (ManMode_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field and it's parameter %s.",
                          tmp_str, FBLOCK_TABLE_ID, PID_TABLE_ID, tmp_parm_str);
        free(tmp_parm_str); tmp_parm_str = NULL;
        goto loop_end;
      }

      ManMode_already_parsed = 1;

      /* now really parse the <plc_pt> value */
      if ((parm->pid.man_mode_pt = plc_pt_by_name(tmp_parm_str)).valid == 0) {
          /* NO! -> ERROR! We cannot parse the plc_pt value! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "unknown plc point %s given after '%s' keyword. "
                          "Ignoring this field.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID,
                          tmp_parm_str, tmp_str);
      }

      free(tmp_parm_str); tmp_parm_str = NULL;
      goto loop_end;
    } /* parse ManMode <plc_pt> parameter */

    /***********************/
    /* Parse the P, I or D */
    /***********************/
    {
      /* if none of the above, then it must be the one of P, I or D, in this order...
       *  But if P I and D values have all already been specified, then this
       *  is an error...
       */
      int *already_parsed = NULL;
      f32 *PID_parm = NULL;
      const char *parm_str_id = NULL;

      if ((P_already_parsed != 0) && (I_already_parsed != 0) &&
          (D_already_parsed != 0))       {
        plc_log_wrnmsg(1, "Unknown field %s in %s %s spefication. "
                          "Ignoring this field.",
                          tmp_str, FBLOCK_TABLE_ID, PID_TABLE_ID);
        goto loop_end;
      }

       /* figure out which field it is we are going to parse... */
      if (D_already_parsed == 0) {
        already_parsed = &D_already_parsed;
        PID_parm = &(parm->pid.D);
        parm_str_id = "D";
      } else if (I_already_parsed == 0) {
        already_parsed = &I_already_parsed;
        PID_parm = &(parm->pid.I);
        parm_str_id = "I";
      } else {
        already_parsed = &P_already_parsed;
        PID_parm = &(parm->pid.P);
        parm_str_id = "P";
      }

      *already_parsed = 1;

      if (string_str_to_f32(tmp_str, PID_parm, -f32_MAX, f32_MAX)
          < 0) {
          /* ERROR! We cannot parse the parm value! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "error parsing the %s parameter (%s). "
                          "Using default %f instead.",
                          FBLOCK_TABLE_ID, PID_TABLE_ID,
                          parm_str_id, tmp_str, PID_parm);
      }
      goto loop_end;
    }  /* parse P, I or D parameter */

    loop_end:
    free(tmp_str); tmp_str = NULL;

  } /* while() -> parsing 4h field, onwards... */

  return 0;
}



int pid_dump_parm(fb_parm_t parm)
{
 printf("P = %f, I = %f, D = %f\n"
        "in_point = %s, out_pt = %s\n",
        parm.pid.P, parm.pid.I, parm.pid.D, "", "");

 return 0;
}

