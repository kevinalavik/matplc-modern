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
 * time_util.c
 *
 * this file implements basic time functions, whose implementations are
 * heavily dependent on the OS on which the code will be compiled.
 * For now, assume SVr4, BSD 4.3, or Linux
 * 
 */


#include "time_util.h"


inline int time_get(timeval_t *t)
{
  return gettimeofday(t, NULL); 
}


int time_sub(timeval_t *res, timeval_t t1, timeval_t t2)
{
  res->tv_usec = t1.tv_usec - t2.tv_usec;
  res->tv_sec  = t1.tv_sec  - t2.tv_sec;

  if ((res->tv_sec > 0) && (res->tv_usec < 0)) {
    res->tv_sec--;
    res->tv_usec += 1000000;
  }

  if ((res->tv_sec < 0) && (res->tv_usec > 0)) {
    res->tv_sec++;
    res->tv_usec -= 1000000;
  }

  return 0;
}


inline long double time_to_ld(timeval_t t)
{
  return (long double)t.tv_sec + (long double)t.tv_usec / (long double)1000000; 
}

inline f32 time_to_f32(timeval_t t)
{
  return (f32)t.tv_sec + (f32)t.tv_usec / (f32)1000000; 
}
