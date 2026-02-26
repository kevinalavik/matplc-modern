/*
 * (c) 2001 Mario de Sousa
 * (C) 2002 Hugh Jack
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
 * About:
 *    This module provides timing capabilities that are used by clients.
 *    At this point in time the routines support soft-realtime processes
 *    that will generally be run at regular intervals, but it is not
 *    guaranteed. By default the POSIX timers are used, and they are the
 *    best option. On some older systems the POSIX functions are not present
 *    or not fully implemented. In those cases the UNIX functions should
 *    be used instead, but they will conflict with 'sleep' and other timer
 *    functions because they also use the 'SIGALRM' signal. If you can't
 *    get it to compile you may go to the absolute worst case with
 *    'NO_TIMERS' which will cause some programs to break.
 *
 *    Eventually it would be nice to incorporate hard-realtime extensions.
 *    This could be done with RTLinux now, or with the features in the
 *    newer 2.5/2.6 kernels.
 */

/*
 * History:
 *    Mario - reviewed the code and made suggestions (Feb 2002)
 *    Hugh - added (hacked) some code for backwards compatibility to
 *          pre-posix standards (Feb 2002)
 *    Mario - original author (< Feb 2002)
*/


/*
 * NOTE: Define one (and only one) of the following options !!!!!!!!!!!
*/

#define SPIFFY_NEW_POSIX		/* Bells and whistles, the option of choice */
/* #define GOOD_OLD_UNIX */		/* Seems to work - but it will have problems with 'sleep' */
/* #define NO_TIMERS */			/* Ugly, but the timers won't block the processes */

/*
 * This file implements the functions in
 *   period.h
 *   period_private.h
 *
 */

#ifdef SPIFFY_NEW_POSIX
  #define _REENTRANT
/*  #define _POSIX_TIMERS */
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>    /* required for floor() */

#ifdef SPIFFY_NEW_POSIX
  #include <string.h> /* required for memset() */
  #include <pthread.h>
#endif

#include <log/log.h>
#include <conffile/conffile.h>
#include <cmm/cmm.h>
#include <misc/string_util.h>
#include <misc/signal_util.h>
#ifdef GOOD_OLD_UNIX
	#include <sys/time.h>
	#include <string.h>
#endif
#ifdef SPIFFY_NEW_POSIX
	#include <misc/timer_util.h>
#endif

#include "period.h"
#include "period_private.h"



static const int debug = 0;


/************************************************************/
/*****************                          *****************/
/*****************    Variables global to   *****************/
/*****************      period library      *****************/
/*****************                          *****************/
/************************************************************/

  /* the current status of the period library */
static status_t period_status_ = {0};

  /* pointer to the period cmm memory block */
static period_t *period_shm_ = NULL;

  /* the module name */
static const char *period_module_name_ = NULL;

 /* timer signal counter */
 /* MUST be defined as volatile, because it is used in the
    signal handler and outside it!!!
  */
static volatile u32 _timer_sigcount = 0;
#define MAX_TIMER_SIGCOUNT u32_MAX

#ifdef SPIFFY_NEW_POSIX
 /* the timer we are using... */
static timer_t _timer_id = -1;

 /* A condition variable... */
pthread_cond_t _condvar;

 /* A mutex... */
pthread_mutex_t _mutex;
#endif

#ifdef GOOD_OLD_UNIX
 /* the number of the signal we will use for the timer */
static int _timer_signum  = -1;

 /* the previous action associated with the signal we will be using */
static struct sigaction _old_action;
#endif




/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************     period_private.h     *****************/
/*****************                          *****************/
/************************************************************/

/* access functions for the status_ variable */
static inline int period_get_status(const status_t stat_bit) {
  return (period_status_.s & stat_bit.s) != 0;
}

static inline void period_set_status(const status_t stat_bit) {
  period_status_.s |= stat_bit.s;
}

static inline void period_rst_status(const status_t stat_bit) {
  period_status_.s &= (~stat_bit.s);
}


/************************************************************/
/*****************                          *****************/
/*****************    Usefull routines for  *****************/
/*****************      period library      *****************/
/*****************                          *****************/
/************************************************************/

#define __assert_period_not_fully_initialized(ret_val) {         \
          if (period_get_status(period_fully_initialized) != 0)  \
            return ret_val;                                      \
        }

#define __assert_period_fully_initialized(ret_val) {             \
          if (period_get_status(period_fully_initialized) == 0)  \
            return ret_val;                                      \
        }


/* function to parse the config file for PERIOD related values */
int period_parse_conf(double *scan_period)
{
  char *tmp_str;

  if (scan_period != NULL)
    if (conffile_get_value_d(SCAN_PERIOD_NAME, scan_period,
		             SCAN_PERIOD_MIN, SCAN_PERIOD_MAX, SCAN_PERIOD_DEF)
        < 0) {
      tmp_str = conffile_get_value(SCAN_PERIOD_NAME);
      plc_log_wrnmsg(1, "Invalid %s paramater %s. Should be a float "
                        "in [%f .. %f]. Using default %f",
		     SCAN_PERIOD_NAME,
		     tmp_str,
		     SCAN_PERIOD_MIN, SCAN_PERIOD_MAX, SCAN_PERIOD_DEF);
      free(tmp_str);
    }

  return 0;
}



#ifdef GOOD_OLD_UNIX
/* The timer signal handler function... */
static void timer_sighandler(int signum) {

  if (_timer_sigcount != MAX_TIMER_SIGCOUNT)
    _timer_sigcount++;

  return;
}
#endif



#ifdef SPIFFY_NEW_POSIX
/* The timer signal handler function... */
static void timer_sighandler(union sigval sigval) {

  if (pthread_mutex_lock(&_mutex) < 0) {
    perror("Error while locking _mutex in period subsystem");
    /* This kind of error should never occur! */
    /* What should we do if it does occur? Good question...
     * If we abort, something may break because the module is no longer running...
     * If we continue running (without guaranteeing the specified period,
     * strange things may occur too.
     *
     * As the Portuguese say 'Venha o diabo e escolha'
     * i.e 'let the devil make the choice'!
     */
    exit (EXIT_FAILURE);
  };

  if (_timer_sigcount != MAX_TIMER_SIGCOUNT)
    _timer_sigcount++;

  if (pthread_cond_signal(&_condvar) < 0) {
    perror("Error while signaling _condvar in period subsystem");
    /* Same comments as above... */
    exit (EXIT_FAILURE);
  }

  if (pthread_mutex_unlock(&_mutex) < 0) {
    perror("Error while unlocking _mutex in period subsystem");
    /* Same comments as above... */
    exit (EXIT_FAILURE);
  };
}
#endif



/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************         period.h         *****************/
/*****************                          *****************/
/************************************************************/


  /* WARNING: This function is not thread safe in a non-standard way! */
  /*          See more comments beloew...                             */
int period_init(const char *module_name)
{
  u32 size;
  double scan_period;
  u32    scan_period_sec;
  u32    scan_period_nsec;
  struct timespec resolution;
#ifdef SPIFFY_NEW_POSIX
  struct sigevent event;
#endif

#ifdef NO_TIMERS
  /* The library_fully_initialized flag is not set to indicate that */
  /* library is not working normally. Any program that counts on it */
  /* should check for this set and exit if not set. */
  return 0;
#endif

  __assert_period_not_fully_initialized(-1);

  /* parse the config file for parameters */
  if (period_parse_conf(&scan_period) < 0)
    goto error_exit_0;
  scan_period_sec  = floor(scan_period);
  scan_period_nsec = 1e9 * (scan_period - scan_period_sec);

    /* Determine the clock's resolution... */
#ifdef GOOD_OLD_UNIX
  if (clock_getres(CLOCK_REALTIME, &resolution) < 0) {
    plc_log_wrnmsg(1, "Could not determine the system clock resolution.");
      /* Let's assume the smallest possible resolution, so we don't get to
         spit out any further warnings... */
    resolution.tv_sec = 0;
    resolution.tv_nsec = 1;
  }
#endif
#ifdef NO_TIMERS
    resolution.tv_sec = 0;
    resolution.tv_nsec = 1;
#endif
#ifdef SPIFFY_NEW_POSIX
  if (clock_getres(CLOCK_REALTIME, &resolution) < 0) {
    plc_log_wrnmsg(1, "Could not determine the system clock resolution.");
      /* Let's assume the smallest possible resolution, so we don't get to
         spit out any further warnings... */
    resolution.tv_sec = 0;
    resolution.tv_nsec = 1;
  }
#endif

  /* verify if a previous invocation of the module has already
     created the shared memory block */
  period_shm_ = cmm_block_find(PERIOD_T_BLOCK_TYPE,
                               module_name,
                               &size);
  if ((period_shm_ != NULL) && (size != sizeof(period_t))) {
    plc_log_errmsg(9, "period_init(): period shared memory block has wrong size.");
    goto error_exit_0;
  }

  if (period_shm_ == NULL) {
    /* no block found. We will create a new one */

      /* create configuration shared memory block */
    period_shm_ = cmm_block_alloc(PERIOD_T_BLOCK_TYPE,
                                  module_name,
                                  sizeof(period_t));

    if (period_shm_ == NULL) {
      plc_log_errmsg(9, "period_init(): could not alloc config memory block.");
      goto error_exit_0;
    }

     /* initialise the cmm block */
    period_shm_->period_sec  = scan_period_sec;
    period_shm_->period_nsec = scan_period_nsec;
  }

    /* set the first scan flag... */
  period_shm_->first_scan = 1;

    /* Verify if the module is currently executing
     * with a period different to the one specified
     * in the config file
     */
  if ((scan_period_sec  != period_shm_->period_sec ) ||
      (scan_period_nsec != period_shm_->period_nsec))
    plc_log_wrnmsg(1, "This module's period has previously been changed to %f sec, "
                      "which differs from the value specified in the config "
                      "file (%f sec). A period of %1$f sec will therefore be used.",
                   period_shm_->period_sec + period_shm_->period_nsec/1e9,
                   scan_period_sec + scan_period_nsec/1e9);

    /* Verify if the clock's resolution is small enough to
     * manage the required period.
     */
  if ((period_shm_->period_sec + period_shm_->period_nsec) != 0)
    if ((period_shm_->period_sec + period_shm_->period_nsec/1e9)
          < 2 * (resolution.tv_sec + resolution.tv_nsec/1e9))
      plc_log_wrnmsg(1, "The clock's resolution on this system (%f sec) is "
                        "not small enough to guarantee correct periodic "
                        "execution of the module at the required period (%f sec).",
	  	     resolution.tv_sec + resolution.tv_nsec/1e9,
                     period_shm_->period_sec + period_shm_->period_nsec/1e9);

#ifdef GOOD_OLD_UNIX
	if(signal(SIGALRM, &timer_sighandler) == SIG_ERR){
		plc_log_errmsg(9, "Error setting the signal handler for the period timer.");
		goto error_exit_1;
	}
#endif

#ifdef SPIFFY_NEW_POSIX
  /* initialise the mutexes and condition variable... */
  { int res;
    res = pthread_cond_init(&_condvar, NULL);
    if (res == 0)
      res = pthread_mutex_init(&_mutex, NULL);
    if (res != 0) {
      plc_log_errmsg(9, "period_init(): error while initialising mutexes and condition var.");
      goto error_exit_1;
    }
  }

    /* create the timer... */
  memset (&event, 0, sizeof (struct sigevent));
  event.sigev_value.sival_int = 0;
  event.sigev_notify = SIGEV_THREAD;
  event.sigev_notify_attributes = NULL;
  event.sigev_notify_function = timer_sighandler;

  if (timer_create(CLOCK_REALTIME, &event, &_timer_id) < 0) {
    plc_log_errmsg(9, "Error creating the period timer.");
    goto error_exit_2;
  }
#endif

    /* NOTE: We do not initialize the timer just yet.        */
    /*       We have no gurantee of the time the module will */
    /*       will take between the call to plc_init() (that  */
    /*       calls this function) and the first call to      */
    /*       plc_scan_beg(). This means that, if the period  */
    /*       is short enough, multiple periods could pass    */
    /*       between these two calls.                        */
    /*       The solution is to initilaze the timer only     */
    /*       on the first call to plc_scan_beg().            */

   /* remember the module's name */
  period_module_name_ = module_name;

  period_set_status(period_fully_initialized);
  return 0;

#ifdef SPIFFY_NEW_POSIX
error_exit_2:
    pthread_cond_destroy(&_condvar);
    pthread_mutex_destroy(&_mutex);
#endif

error_exit_1:
  cmm_block_free(period_shm_);

error_exit_0:
  period_shm_ = NULL;
  return -1;
}



int period_done(void)
{
  int res = 0;

  __assert_period_fully_initialized(0);

#ifdef SPIFFY_NEW_POSIX
    /* delete mutexes and condition variable... */
  pthread_cond_destroy(&_condvar);
  pthread_mutex_destroy(&_mutex);

    /* delete the timer we were using */
  res = timer_delete(_timer_id);
  _timer_id = -1;
#endif

    /* NOTE:                                                    */
    /* we do *not* free the cmm block. We leave it for possible */
    /* future modules that might be taking this module's place! */

  period_module_name_ = NULL;
  period_shm_ = NULL;

  period_rst_status(period_fully_initialized);
  return res;
}



inline int period_scan_beg(void) {
 __assert_period_fully_initialized(-1);

 if (period_shm_->first_scan == 1) {
   period_shm_->first_scan = 0;

     /* initialise the timer... */
   return plc_period_set(period_shm_->period_sec, period_shm_->period_nsec);
 }

  /* Check if we should be running wthout any delays. */
  /* If so, the timer will be set to zero (i.e. stoped), so we won't
     be receiving any signals. We simply return...
   */
 if ((period_shm_->period_sec == 0) && (period_shm_->period_nsec == 0))
   return 0;

#ifdef GOOD_OLD_UNIX
 /* NOTE: A previous version I coded of this file used signals with a POSIX timer
  *       for the asynchronous notification of the timer event.
  *       It seems that Hugh decided that the same code would also
  *       work for the good'ol UNIX version, so I'll leave it here...
  *       Nevertheless, I (Mario) have my doubts this will work
  *       as Hugh intended!
  */
    /* High magic is working here...                               */
    /* Note that sigsuspend() unblocks the signals while it waits  */
    /* for a signal to arrive, and blocks them again before        */
    /* returning!                                                  */

 /* Set up the mask of signals to temporarily block. */
 sigemptyset (&mask);
 sigaddset (&mask, _timer_signum);

 /* Wait for a signal to arrive. */
// sigprocmask (SIG_BLOCK, &mask, &oldmask);
   /* The signal is blocked, so we can reliably access the variable... */
 while (_timer_sigcount == 0);
//   sigsuspend (&oldmask);
 _timer_sigcount--;
   /* unblock the signal... */
// sigprocmask (SIG_UNBLOCK, &mask, NULL);
#endif

#ifdef SPIFFY_NEW_POSIX
 if (pthread_mutex_lock(&_mutex) < 0) {
   perror("Error while locking _mutex in period subsystem");
   /* This kind of error should never occur! */
   /* What should we do if it does occur? Good question...
    * If we abort, something may break because the module is no longer running...
    * If we continue running (without guaranteeing the specified period,
    * strange things may occur too.
    *
    * As the Portuguese say 'Venha o diabo e escolha'
    * i.e 'let the devil make the choice'!
    */
   exit (EXIT_FAILURE);
 };

 while (_timer_sigcount == 0) {
   if (pthread_cond_wait(&_condvar, &_mutex) < 0) {
     perror("Error while waiting on _condvar in period subsystem");
     /* Same comments as above... */
     exit (EXIT_FAILURE);
   }
 }

 _timer_sigcount--;

 if (pthread_mutex_unlock(&_mutex) < 0) {
   perror("Error while unlocking _mutex in period subsystem");
   /* Same comments as above... */
   exit (EXIT_FAILURE);
 };
#endif

 return 0;
}


inline int period_scan_end()
{
 /* At the moment, not required... */
 /* __assert_period_fully_initialized(-1); */

 return 0;
}



/*
 * Get the currently configured period
 */
int plc_period_get(u32 *period_sec, u32 *period_nsec)
{
  __assert_period_fully_initialized(-1);

  if (period_sec != NULL)
    *period_sec = period_shm_->period_sec;
  if (period_nsec != NULL)
    *period_nsec = period_shm_->period_nsec;

  return 0;
}


/*
 * Set the period
 */
int plc_period_set(u32 period_sec, u32 period_nsec)
{
#ifdef GOOD_OLD_UNIX
  struct itimerval time_value;
  memset(&time_value, 0, sizeof(time_value));
  __assert_period_fully_initialized(-1);

  period_shm_->period_sec = period_sec;
  period_shm_->period_nsec = period_nsec;
  time_value.it_interval.tv_sec = period_sec;
  time_value.it_interval.tv_usec = period_nsec/1000;
  time_value.it_value.tv_sec = period_sec;
  time_value.it_value.tv_usec = period_nsec/1000;
  return setitimer(ITIMER_REAL, &time_value, NULL);
#endif
#ifdef NO_TIMERS
  return -1;
#endif
#ifdef SPIFFY_NEW_POSIX
  struct itimerspec time_value;
  __assert_period_fully_initialized(-1);

  period_shm_->period_sec = period_sec;
  period_shm_->period_nsec = period_nsec;
  time_value.it_interval.tv_sec = period_sec;
  time_value.it_interval.tv_nsec = period_nsec;
  time_value.it_value.tv_sec = period_sec;
  time_value.it_value.tv_nsec = period_nsec;
  return timer_settime(_timer_id, 0, &time_value, NULL);
#endif

}
