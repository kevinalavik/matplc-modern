#ifndef PLC_TIMER_H
#define PLC_TIMER_H
/*
 * A `timer' data type plus access functions.
 */

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
#define plc_timer_clear(timer) plc_timer_clear_fn(&(timer))
void plc_timer_clear_fn(plc_timer_t *);

/* start a timer, ie zero and enable - previous value discarded */
#define plc_timer_start(timer) plc_timer_start_fn(&(timer))
void plc_timer_start_fn(plc_timer_t *);

/* Other functions */

/* enable a timer, retaining current value. No effect if already enabled. */
#define plc_timer_enable(timer) plc_timer_enable_fn(&(timer))
void plc_timer_enable_fn(plc_timer_t *);

/* disable a timer, retaining current value. No effect if already disabled. */
#define plc_timer_disable(timer) plc_timer_disable_fn(&(timer))
void plc_timer_disable_fn(plc_timer_t *);

/*
 * Advance a timer by a particular amount. Negative for retard. May be used
 * on enabled or disabled timers.
 */
#define plc_timer_add(timer, value) plc_timer_add_fn(&(timer), value)
void plc_timer_add_fn(plc_timer_t *, double);

/*
 * set a timer to a particular value. The enabled/disabled status will
 * remain unchanged.
 */
#define plc_timer_set(timer, value) plc_timer_set_fn(&(timer), value)
void plc_timer_set_fn(plc_timer_t *, double t);

/* Get the current time on a timer (number of seconds accumulated). */
double plc_timer_get(plc_timer_t);

/* Has the timer reached the preset? */
int plc_timer_done(plc_timer_t timer, double preset);

#endif				/* PLC_TIMER_H */
