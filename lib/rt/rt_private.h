/*
 * (c) 2004 Mario de Sousa
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

/* The Real-Time section...
 * 
 * This section attempts to guarantee that the MatPLc module works
 * under Real-Time constraints. Whether these constraints are hard or soft
 * will mostly depend on the underlying OS. 
 * In any event, the MatPLC simply tries to lock all memory into RAM
 * (if memlock = 1 is chosen in the config file), and
 * to set the apropriate scheduling priority, along with the SCHED_FIFO
 * scheduling policy if a priority > PRIORITY_MIN is chosen.
 */


#ifndef __RT_PRIVATE_H
#define __RT_PRIVATE_H


#define RT_SCHEDPOLICY SCHED_FIFO

#define CONF_RTMEMLOCK_SECTION "PLC"
#define CONF_RTMEMLOCK_NAME "memlock"
#define CONF_RTMEMLOCK_MIN 0
#define CONF_RTMEMLOCK_MAX 1
#define CONF_RTMEMLOCK_DEF 0


#define CONF_RTPRIORITY_NAME "priority"
#define CONF_RTPRIORITY_MIN RT_PRIORITY_MIN
#define CONF_RTPRIORITY_MAX RT_PRIORITY_MAX
#define CONF_RTPRIORITY_DEF RT_PRIORITY_MIN


#endif  /* #ifndef __RT_PRIVATE_H */



