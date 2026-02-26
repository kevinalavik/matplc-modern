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
 * this file implements the dsp - Digital Signal Processor
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
#include "dsp.h"



/* The currently existing function blocks... */

fb_types_t fb_types[] = {
            {PID_TABLE_ID,
                   pid_parse_parm, pid_step},
            {ADD_TABLE_ID,
                   add_parse_parm, add_step},
            {ALARM_TABLE_ID,
                   alarm_parse_parm, alarm_step},
            {RAMP_TABLE_ID,
                   ramp_parse_parm, ramp_step},
            {FILTER_TABLE_ID,
                   filter_parse_parm, filter_step},
            {TYPECONV_TABLE_ID,
                   typeconv_parse_parm, typeconv_step},
            {NONLINEAR_TABLE_ID,
                   nonlinear_parse_parm, nonlinear_step},
            {MULT_TABLE_ID,
                   mult_parse_parm, mult_step},
            {POW_TABLE_ID,
                   pow_parse_parm, pow_step},
            {MULTIPLEXOR_TABLE_ID,
                   multiplexor_parse_parm, multiplexor_step},
            {NULL, NULL, NULL} /* last structure must be NULL */
          };




/* definitions */
static const int debug = 0;

fb_t *fb = NULL;
int  num_fb = 0;
plc_pt_t out_time_pt;



int run_loop(void)
{
 dsp_time_t dsp_time;
 int count;
 f32 tmp_f32;
 long double scan_0_time = 0;
 unsigned int scan_count;
#define MAX_SCAN_COUNT UINT_MAX

 scan_count = 0;

 while (1) {

  plc_scan_beg();
  plc_update();

  /* determine the time... */
    /* hopefully, in future, we will have a plc_update that returns the time the
        update was done, while the global memory lock is held. For the moment
        we live without it, but remember that timing errors may occur.
    */
  time_get(&dsp_time.t_now);
  dsp_time.ld_now = time_to_ld(dsp_time.t_now);

  if (scan_count == 0) {
    scan_0_time = dsp_time.ld_now;
    dsp_time.t_prev = dsp_time.t_now;
  }

  time_sub(&dsp_time.t_dif, dsp_time.t_now, dsp_time.t_prev);
  dsp_time.ld_dif = time_to_ld(dsp_time.t_dif);

  tmp_f32 = dsp_time.ld_now - scan_0_time;
  plc_set_f32 (out_time_pt, tmp_f32);

  for (count = 0; count < num_fb; count++) {
    fb[count].fb_step (scan_count, &dsp_time, &(fb[count].fb_parm));
  }

  dsp_time.t_prev  = dsp_time.t_now;
  dsp_time.ld_prev = dsp_time.ld_now;

  plc_update();
  plc_scan_end();

    /* count up to maximum value, and then stall */
  if (scan_count != MAX_SCAN_COUNT)
    scan_count++;
 } /* while (1) */

 return -1;
}






/* get everything from the config */
int get_config(void)
{
  char *tmp_str;
  int block_count, block_type_count, max_blocks;
  f32 scan_period;
  u32 period_sec, period_nsec;

  if (debug)
    printf("get_config(): ...\n");

  /* determine the scan_period...*/
  if (plc_period_get(&period_sec, &period_nsec) < 0) {
    plc_log_wrnmsg(1,
                   "Could not determine the module's scan period. "
                   "Any block that depends on it (e.g. filter) will be wrong!");
    period_sec = period_nsec = 0;
  }
  scan_period = period_sec + period_nsec/1e9;

  /* get the time output point */
  out_time_pt = plc_pt_null();
  if ((tmp_str = conffile_get_value("out_time_pt")) != NULL) {
    out_time_pt = plc_pt_by_name(tmp_str);
    if (out_time_pt.valid == 0) {
      plc_log_wrnmsg(1,
                     "Invalid output point %s for storing current time.",
                     tmp_str);
      out_time_pt = plc_pt_null();
    }
    free (tmp_str);
  }

  /* get the number of rows */
  max_blocks = conffile_get_table_rows(FBLOCK_TABLE_ID);
  if (max_blocks == 0) {
    /* no blocks defined */
    /* plc_log_wrnmsg("...) */
    return -1;
  }

  if (debug)
    printf("get_config(): found fblock table with %d rows.\n", max_blocks);

  if ((fb = (fb_t *)malloc(sizeof(fb_t) * max_blocks)) == NULL) {
    /* not enough memory */
    /* plc_log_wrnmsg("...) */
    return -1;
  }

  for (block_count = 0, num_fb = 0;
       block_count < max_blocks;
       block_count++) {

    tmp_str = conffile_get_table(FBLOCK_TABLE_ID, block_count, 0);
    if (tmp_str == NULL)
      continue;

    /* check whether the block is a known function block */
    for (block_type_count = 0;
         ((fb_types[block_type_count].conftable_id != NULL) &&
          (fb_types[block_type_count].parse_parm_func != NULL) &&
          (fb_types[block_type_count].step_func != NULL));
         block_type_count++) {

      if ( strcmp (tmp_str, fb_types[block_type_count].conftable_id) == 0) {
        if (fb_types[block_type_count].parse_parm_func (FBLOCK_TABLE_ID,
                                                        block_count,
                                                        &(fb[num_fb].fb_parm),
							scan_period)
            < 0) {
          /* invalid configuration of the function block */
          /* plc_log_wrnmsg("...) */
        }
        else {
        if (debug)
          printf("get_config(): correctly configured block of type %s\n",
                 tmp_str);

        fb[num_fb].fb_step = fb_types[block_type_count].step_func;
        /* only increment block counter if block was correctly initialized */
        num_fb++;
        }
      break; /* found block type. */
      }
    } /* for(block_type_count) */
    free (tmp_str);
  } /* for(block_count) */

  return 0;
}



int dump_config(void)
{
 if (num_fb == 0)
   plc_log_wrnmsg(1, "Dsp module running with no function blocks configured.");

 plc_log_trcmsg(2, "%d block(s) configured.\n",
                   num_fb);

 return 0;
}




int main(int argc,char *argv[])
{
  if (plc_init("DSP",argc,argv) < 0) {
      printf("Error connecting to PLC.\n");
      exit(EXIT_FAILURE);
  }

  if (get_config() < 0)
    exit (EXIT_FAILURE);

  if (dump_config() < 0)
    exit (EXIT_FAILURE);

  if (run_loop() < 0)
    exit (EXIT_FAILURE);

  return EXIT_SUCCESS;
}

