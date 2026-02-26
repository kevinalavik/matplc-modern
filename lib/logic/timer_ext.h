#ifndef PLC_TIMER_EXT_H
#define PLC_TIMER_EXT_H
/*
 * A `timer' data type plus access functions - external timebase.
 */

/* need struct timeval */
#include <sys/time.h>
#include <unistd.h>

/*
 * general warning: floats are tricky. Be careful. Never add or subtract
 * two reals unless you know what you are doing. This message brought to
 * you by the League Of Programmers Against Loss Of Precision (LOPALOP).
 */

/*
 * cut-n-paste programming has been used for timer.[ch] and timer_ext.[ch]
 * Please be sure to fix bugs &c in both sets (use diff to make sure).
 * Sorry about that, but otherwise it'd be far too many macros...
 */

/*
 * a timer (to be considered opaque)
 */
typedef struct {
  /* char valid; */
  char opaque[12];
} plc_timer_t;

/*
 * Every timer must be initialized by one of the following two functions.
 * They may also be called later. No cleanup is required for timers.
 */

/* zero and disable a timer - previous value discarded */
#define plc_timer_clear_ext(timer, ext) plc_timer_clear_fn_ext(&(timer), ext)
void plc_timer_clear_fn_ext(plc_timer_t *, struct timeval ext);

/* start a timer, ie zero and enable - previous value discarded */
#define plc_timer_start_ext(timer, ext) plc_timer_start_fn_ext(&(timer), ext)
void plc_timer_start_fn_ext(plc_timer_t *, struct timeval ext);

/* Other functions */

/* enable a timer, retaining current value. No effect if already enabled. */
#define plc_timer_enable_ext(timer, ext) plc_timer_enable_fn_ext(&(timer), ext)
void plc_timer_enable_fn_ext(plc_timer_t *, struct timeval ext);

/* disable a timer, retaining current value. No effect if already disabled. */
#define plc_timer_disable_ext(timer, ext) plc_timer_disable_fn_ext(&(timer), ext)
void plc_timer_disable_fn_ext(plc_timer_t *, struct timeval ext);

/*
 * Advance a timer by a particular amount. Negative for retard. May be used
 * on enabled or disabled timers.
 */
#define plc_timer_add_ext(timer, value, ext) plc_timer_add_fn_ext(&(timer), value, ext)
void plc_timer_add_fn_ext(plc_timer_t *, double, struct timeval ext);

/*
 * set a timer to a particular value. The enabled/disabled status will
 * remain unchanged.
 */
#define plc_timer_set_ext(timer, value, ext) plc_timer_set_fn_ext(&(timer), value, ext)
void plc_timer_set_fn_ext(plc_timer_t *, double t, struct timeval ext);

/* Get the current time on a timer (number of seconds accumulated). */
double plc_timer_get_ext(plc_timer_t, struct timeval ext);

/* Has the timer reached the preset? */
int plc_timer_done_ext(plc_timer_t timer, double preset, struct timeval ext);

#endif				/* PLC_TIMER_EXT_H */
