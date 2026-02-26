/*
 * (c) 2004 Mario de Sousa
 *      (inspired on Jiri's original version, file rt_ns.c)
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

#include <string.h>  /* required for strerror() */
#include <errno.h>
#include <pthread.h>   /* required for pthread_setschedparam(), ... */
#include <sys/mman.h>  /* required for mlockall() */

#include <plc.h>
#include <rt/rt.h>
#include <rt/rt_private.h>


int old_sched_protocol_;
struct sched_param old_sched_param_;

static int requested_priority_ = -1;
static int current_priority_ = -1;

static int RT_PRIORITY_MIN = -1;
static int RT_PRIORITY_MAX = -1;



static void rt_parse_conf(int *priority,
		          int *memlock,
		            /* use_default: only used if a parameter is not set in the config file.
			     *      use_default = 1 -> store default values into parameters
			     *      use_default = 0 -> leave parameters unchanged
			     */
		          int use_default ) {

  if (priority != NULL) {
    int i, res;
    if (use_default) *priority = CONF_RTPRIORITY_DEF;
    res = conffile_get_value_i32(CONF_RTPRIORITY_NAME, &i,
	                         CONF_RTPRIORITY_MIN, CONF_RTPRIORITY_MAX, CONF_RTPRIORITY_DEF);

    if (res >= 0)
      *priority = i;
    else
      plc_log_wrnmsg(1, "cannot understand %s = %s,"
                     " or value out of bounds [%d...%d]. Using %d",
                     CONF_RTPRIORITY_NAME, conffile_get_value(CONF_RTPRIORITY_NAME),
		     CONF_RTPRIORITY_MIN, CONF_RTPRIORITY_MAX, *priority);
  }

  if (memlock != NULL) {
    int i, res;
    if (use_default) *memlock = CONF_RTMEMLOCK_DEF;
    res = conffile_get_value_sec_i32(CONF_RTMEMLOCK_SECTION, CONF_RTMEMLOCK_NAME, &i,
	                             CONF_RTMEMLOCK_MIN, CONF_RTMEMLOCK_MAX, CONF_RTMEMLOCK_DEF);

    if (res >= 0)
      *memlock = i;
    else
      plc_log_wrnmsg(1, "cannot understand %s = %s,"
                     " or value out of bounds [%d...%d]. Using %d",
                     CONF_RTMEMLOCK_NAME, conffile_get_value_sec(CONF_RTMEMLOCK_SECTION, CONF_RTMEMLOCK_NAME),
		     CONF_RTMEMLOCK_MIN, CONF_RTMEMLOCK_MAX, *memlock);
  }
}








int rt_init(const char *module_name) {
  int memlock;

  /* If the functions return an error (i.e. -1), then it means the
   * OS we are running on does not support SCHED_FIFO, in which case
   * the MatPLC will also not allow the user to specify any priority.
   *
   * This will be handled automatically, by limiting the allowed
   * values for priotiy from [-1, -1]
   */
  RT_PRIORITY_MIN = sched_get_priority_min(RT_SCHEDPOLICY);
  RT_PRIORITY_MAX = sched_get_priority_max(RT_SCHEDPOLICY);

  requested_priority_ = CONF_RTPRIORITY_DEF;
  current_priority_ = CONF_RTPRIORITY_DEF;

  rt_parse_conf(&requested_priority_, &memlock, 1 /* use defaults! */);

  /* get current scheduling parameters, so we can reset them once again
   * when we close down the RT services...
   */
  if (pthread_getschedparam(pthread_self(), &old_sched_protocol_, &old_sched_param_) != 0)
    return -1;

  if (memlock != 0)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) {
      plc_log_errmsg(1, "mlockall = 1 -> Error locking memory into RAM  (mlockall(2): %s)", strerror(errno));
      return -1;
    }

  /* We don't set the priority just yet.
   * Note that the calling process may want to do some more initialising
   * under a low priority after plc_init() gets called and returns.
   * We only get to escalate our priority to the requested priority
   * at the begining of the scan.
   */
  return 0;
}




int rt_done(void) {
  /* reset initial scheduling paramters... */
  if (pthread_setschedparam(pthread_self(), old_sched_protocol_, &old_sched_param_) != 0)
    return -1;

  current_priority_ = CONF_RTPRIORITY_DEF;

  return 0;
}




int rt_scan_beg(void) {
  /* NOTE that RT_PRIORITY_MAX will be set to -1 if the OS we are running on
   * does not support our required scheduling policy...
   */
  if ((RT_PRIORITY_MAX >= 0) && (requested_priority_ != current_priority_)) {
    /* set the correct priority... */
    /* use pthread_setsched() instead of sched_setscheduler()
     * because, even though the MatPLC is not thread safe, it may
     * be used within a single thread of a multi-threaded application!
     */
    struct sched_param param;
    param.sched_priority = requested_priority_;
    if (pthread_setschedparam(pthread_self(), RT_SCHEDPOLICY, &param) != 0)
      return -1;
    current_priority_ = requested_priority_;
  }

  return 0;
}


int rt_scan_end(void)
{return 0;}


/* returns the requested priority for this MatPLC module */
/* returns -1 on error */
int rt_getpriority(void) {
  return requested_priority_;
}
