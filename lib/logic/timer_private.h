#ifndef PLC_TIMER_PRIVATE_H
#define PLC_TIMER_PRIVATE_H

#include <sys/time.h>
#include <unistd.h>

/*
 * the private definition of the timer.
 *
 * The enabled field contains 1 when the timer is running, 0 otherwise.
 * The contents of the tv field depends on enabled:
 *   when it is 0, it contains the accumulated time
 *   when it is 1, it contains the clock time corresponding to "timer==0"
 *     (this is the effective start time, taking into account interruptions
 *     and adjustments)
 */

typedef struct {
  struct timeval tv;
  char enabled;
} plc_timer_private_t;

#endif				/* PLC_TIMER_PRIVATE_H */
