#include <stdlib.h> /* required for exit() */
#include <stdio.h>

#include "timer_ext.h"
#include "timer_private.h"
/*
 * A `timer' data type plus access functions - external timebase.
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

/* Type-overriding thingy. Like in the GMM, it is assumed that the timer
 * parameter is "t", and a macro "tt" is defined which is "t" cast to the
 * correct type. Unlike the GMM, it must also handle a pointer parameter;
 * that's the "ttt" macro (and "tp" parameter).
 */

#define tt (*(plc_timer_private_t*)&(t))
#define ttt (*(plc_timer_private_t*)(tp))

/* lazy */
#define tte (tt.enabled)
#define ttte (ttt.enabled)
#define tts (tt.tv.tv_sec)
#define ttts (ttt.tv.tv_sec)
#define ttu (tt.tv.tv_usec)
#define tttu (ttt.tv.tv_usec)

/* I don't trust myself to count that many zeros consistently :-) */
static const int million = 1000000;

static inline void normalize(plc_timer_t * tp)
{
  while (tttu >= million) {
    tttu -= million;
    ttts++;
  }
  while (tttu < 0) {
    tttu += million;
    ttts--;
  }
}

/* zero and disable a timer - previous value discarded */
void plc_timer_clear_fn_ext(plc_timer_t * tp, struct timeval ext)
{
  if (sizeof(plc_timer_t) != sizeof(plc_timer_private_t)) {
    printf("Error in timer library - type size mismatch!!!\n");
    exit(1);
  }
  ttts = 0;
  tttu = 0;
  ttte = 0;
}

void plc_timer_start_fn_ext(plc_timer_t * tp, struct timeval ext)
{
  if (sizeof(plc_timer_t) != sizeof(plc_timer_private_t)) {
    printf("Error in timer library - type size mismatch!!!\n");
    exit(1);
  }
  ttt.tv=ext;
  ttte = 1;
}

/* Other functions */

/* enable a timer, retaining current value. No effect if already enabled. */
void plc_timer_enable_fn_ext(plc_timer_t * tp, struct timeval ext)
{
  const struct timeval now=ext;

  if (ttte)
    return;

  /* tv = now - tv, ie: start = now - accum */
  tttu = now.tv_usec - tttu;
  ttts = now.tv_sec - ttts;

  if (tttu < 0) {
    tttu += million;
    ttts--;
  }

  ttte = 1;
}

/* disable a timer, retaining current value. No effect if already disabled. */
/* Note: exactly the same as previous function - yes that's right */
void plc_timer_disable_fn_ext(plc_timer_t * tp, struct timeval ext)
{
  const struct timeval now=ext;

  if (!ttte)
    return;

  /* tv = now - tv, ie: accum = now - start */
  tttu = now.tv_usec - tttu;
  ttts = now.tv_sec - ttts;

  if (tttu < 0) {
    tttu += million;
    ttts--;
  }

  ttte = 0;
}

/*
 * Advance a timer by a particular amount. Negative for retard. May be used
 * on enabled or disabled timers.
 */
void plc_timer_add_fn_ext(plc_timer_t * tp, double incr, struct timeval ext)
{
  int s;
  if (ttte)
    incr = -incr;

  s = incr;
  if (s) {
    ttts += s;
    incr -= s;
  }
  tttu += incr * million;
  /* TODO: is the rounding correct or is it 0.5 us off ? */

  normalize(tp);
}

/*
 * set a timer to a particular value. The enabled/disabled status will
 * remain unchanged.
 */
void plc_timer_set_fn_ext(plc_timer_t * tp, double val, struct timeval ext)
{
  if (ttte) {
    int s;
    ttt.tv=ext;
    s = val;
    ttts -= s;
    val -= s;
    tttu -= val * million;
  } else {
    ttts = val;
    val -= ttts;
    tttu = (val * million);
  }

  normalize(tp);
  /* TODO: is the rounding correct or is it 0.5 us off ? */
}

/* Get the current time on a timer (number of seconds accumulated). */
inline double plc_timer_get_ext(plc_timer_t t, struct timeval ext)
{
  if (tte) {
    const struct timeval now=ext;

    return now.tv_sec - tts + ((now.tv_usec - ttu) * 1e-6);
  } else {
    return tts + (ttu * 1e-6);
  }
}

/* Has the timer reached the preset? */
int plc_timer_done_ext(plc_timer_t timer, double preset, struct timeval ext)
{
    return plc_timer_get_ext(timer, ext) >= preset;
}
