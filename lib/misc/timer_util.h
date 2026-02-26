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


#ifndef TIMER_UTIL_H
#define TIMER_UTIL_H



 /* The system under which we are being compiled (Linux) suports POSIX timer functions.

    If compiling under a different system that does not support POSIX timer functions,
    then this file should be changed to one that emulates POSIX timers.
  */

 /* but we must still define _POSIX_C_SOURCE to be able
    to use the POSIX timers...
  */

/* For some strange reason, this define is causing trouble in my (Mario) Linux box.
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif
*/


#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <limits.h>   /* for TIMER_MAX -> maximum number of timers... */


#endif    /* TIMER_UTIL_H */
