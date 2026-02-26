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


#ifndef __TIME_UTIL_H
#define __TIME_UTIL_H

#include <sys/time.h>
#include <unistd.h>
#include <plc.h> /* for f32 */

typedef struct timeval timeval_t;

int time_get(timeval_t *t);
int time_sub(timeval_t *res, timeval_t t1, timeval_t t2);
f32 time_to_f32(timeval_t t);
long double time_to_ld(timeval_t t);

#endif /* __TIME_UTIL_H */
