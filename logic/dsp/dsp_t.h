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
 * dsp.h
 */

#ifndef __DSP_T_H
#define __DSP_T_H


#include "pid_t.h"
#include "add_t.h"
#include "alarm_t.h"
#include "ramp_t.h"
#include "filter_t.h"
#include "typeconv_t.h"
#include "nonlinear_t.h"
#include "mult_t.h"
#include "pow_t.h"
#include "multiplexor_t.h"


/* Structure used to pass timing info to the function blocks */
typedef struct {
  timeval_t    t_now,  t_prev,  t_dif;
  long double ld_now, ld_prev, ld_dif;
} dsp_time_t;



/* Function Block Parameters type */
typedef union {
  fb_pid_t         pid;
  fb_add_t         add;
  fb_alarm_t       alarm;
  fb_ramp_t 	   ramp;
  fb_filter_t	   filter;
  fb_typeconv_t    typeconv;
  fb_nonlinear_t   nonlinear;
  fb_mult_t        mult;
  fb_pow_t         pow;
  fb_multiplexor_t multiplexor;
/*...*/

} fb_parm_t;


#include "pid.h"
#include "add.h"
#include "alarm.h"
#include "ramp.h"
#include "filter.h"
#include "typeconv.h"
#include "nonlinear.h"
#include "mult.h"
#include "pow.h"
#include "multiplexor.h"
#endif /* __DSP_T_H */

