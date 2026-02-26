/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
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
 * PLC Manager - access library implementation
 *
 * This file implements the routines in plc.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>

#include "plc.h"
#include "plc_private.h"

#include "misc/string_util.h"
#include "misc/signal_util.h"

static int debug = 0;



/************************************************************/
/*****************                          *****************/
/*****************    Global, but private   *****************/
/*****************       plc variables      *****************/
/*****************                          *****************/
/************************************************************/

static char *plc_module_name_ = NULL;


static struct sigaction sigterm_old_action_;
static struct sigaction  sigint_old_action_;


/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

/* the current status of the plc library (how far did init get?) */
static status_t plc_status_ = {0};

static const status_t plc_fully_initialised      = {0x0001};
static const status_t registered_exit_func       = {0x0002};
static const status_t registered_sigint_handler  = {0x0004};
static const status_t registered_sigterm_handler = {0x0008};



 /* access functions for the status_ variable */
static inline int plc_get_status(const status_t stat_bit) {
  return (plc_status_.s & stat_bit.s) != 0;
}

static inline void plc_set_status(const status_t stat_bit) {
  plc_status_.s |= stat_bit.s;
}

static inline void plc_rst_status(const status_t stat_bit) {
  plc_status_.s &= (~stat_bit.s);
}





#define __assert_plc_initialized(ret_val) {       \
  if (!plc_get_status(plc_fully_initialised))     \
    return (ret_val);                             \
};

#define __assert_plc_not_initialized(ret_val) {   \
  if (plc_get_status(plc_fully_initialised))      \
    return (ret_val);                             \
};



/************************************************************/
/*****************                          *****************/
/*****************   things declared in     *****************/
/*****************      plc_private.h       *****************/
/*****************                          *****************/
/************************************************************/


int plc_parse_args(int        argc,
                   char       **argv,
		   gmm_loc_t  *location,
                   int        *local_shm_key,
                   int        *confmap_shm_key, /* a.k.a. the plc_id */
                   const char **conffile,
                   char       **mod_name,
                   int        *log_level,
                   const char **log_file,
                   int        *force_init,
		   int        remove_args  /* set to 1 if args should   */
		 			   /* be removed from **argv    */
		  )
{
  int option;
  int tmp_val;

  if (argc < 0)
    return -1;

  for (option = 0; option < argc; option++) {
    /* --PLClocal */
    if (location != NULL) {
      if (strcmp(argv[option], CLO_loc_local) == 0) {
        *location = loc_local;
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLCisolate */
    if (location != NULL) {
      if (strcmp(argv[option], CLO_loc_isolate) == 0) {
        *location = loc_isolate;
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLCshared */
    if (location != NULL) {
      if (strcmp(argv[option], CLO_loc_shared) == 0) {
        *location = loc_shared;
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLClocal_map_key=xx */
    if (local_shm_key != NULL) {
      if (strncmp(argv[option],
	  	  CLO_privmap_key, strlen(CLO_privmap_key))
	  == 0) {
        if (string_str_to_int(argv[option] + strlen(CLO_privmap_key),
			      &tmp_val, 1, INT_MAX)
	    >= 0) {
	  *local_shm_key = tmp_val;
          if (remove_args == 1)
	    argv[option][0] = '\0';
          continue;
        }
      }
    }

    /* --PLCplc_id=xx */
    if (confmap_shm_key != NULL) {
      if (strncmp(argv[option], CLO_plc_id, strlen(CLO_plc_id))
	  == 0) {
        if (string_str_to_int(argv[option] + strlen(CLO_plc_id),
			      &tmp_val, 1, INT_MAX)
	    >= 0) {
	  *confmap_shm_key = tmp_val;
          if (remove_args == 1)
	    argv[option][0] = '\0';
          continue;
        }
      }
    }

    /* --PLCdebug=xx */
    if (log_level != NULL) {
      if (strncmp(argv[option], CLO_log_level, strlen(CLO_log_level))
	  == 0) {
        if (string_str_to_int(argv[option] + strlen(CLO_log_level),
			      &tmp_val, 0, MAX_LOG_LEVEL)
	    >= 0) {
	  *log_level = tmp_val;
          if (remove_args == 1)
	    argv[option][0] = '\0';
          continue;
        }
      }
    }

    /* --PLCconf=foo */
    if (conffile != NULL) {
      if (strncmp(argv[option], CLO_config_file, strlen(CLO_config_file))
	  == 0) {
        *conffile = strdup(argv[option] + strlen(CLO_config_file));
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLCmodule=foo */
    if (mod_name != NULL) {
      if (strncmp(argv[option], CLO_module_name, strlen(CLO_module_name))
	  == 0) {
        *mod_name = strdup(argv[option] + strlen(CLO_module_name));
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLClogfile=foo */
    if (log_file != NULL) {
      if (strncmp(argv[option], CLO_log_file, strlen(CLO_log_file))
	  == 0) {
        *log_file = strdup(argv[option] + strlen(CLO_log_file));
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLCforce-init */
    if (location != NULL) {
      if (strcmp(argv[option], CLO_force_init) == 0) {
        *force_init = 1;
        if (remove_args == 1)
          argv[option][0] = '\0';
        continue;
      }
    }

    /* --PLCxxxxxxxxx */
    if (strncmp(argv[option], CLO_LEADER, strlen(CLO_LEADER))
	== 0) {
      printf("unrecognized " CLO_LEADER " option: %s\n", argv[option]);
      return -1;
    }
  }				/* for (option ...) */
  return 0;
}


/************************************************************/
/*****************                          *****************/
/*****************     Private routines     *****************/
/*****************                          *****************/
/************************************************************/

/* The INT signal handler function... */
static void sigint_handler(int signum)
{
  int raise_flag = 1;

  if (signal_action(SIGINT, &sigint_old_action_, NULL) < 0)
    /* something strange happened. We won't be able to
     * re-raise the signal!
     */
    raise_flag = 0;

  plc_done();

  if (raise_flag == 1)
    raise(SIGINT);
  else
    return;
}


/* The TERM signal handler function... */
static void sigterm_handler(int signum)
{
  int raise_flag = 1;

  if (signal_action(SIGTERM, &sigterm_old_action_, NULL) < 0)
    /* something strange happened. We won't be able to
     * re-raise the signal!
     */
    raise_flag = 0;

  plc_done();

  if (raise_flag == 1)
    raise(SIGTERM);
  else
    return;
}


/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************          plc.h           *****************/
/*****************                          *****************/
/************************************************************/


int plc_print_usage(FILE *output)
{
  fprintf(output,
    CLO_loc_local   " : access PLC memory through local copy.\n"
    CLO_loc_isolate " : access PLC memory through proxy process.\n"
    CLO_loc_shared  " : access PLC memory directly.\n"
    CLO_plc_id      "xx : access PLC with id xx instead of default\n"
    CLO_force_init  " : initialise the module, even when there seems to be another"
                    " module already running under the same name. Use with caution!"
                    " If the other module really is running, things will get REALLY"
                    " messed up!\n"
    CLO_privmap_key "xx : use shared memory for private map with key xx\n"
    CLO_config_file "fname : use fname as the config file instead of "
                     DEF_CONFIG_FILE "\n"
    CLO_module_name "foo : use `foo' as the module name instead of default\n"
    CLO_log_file    "foo : do the logging to the file foo\n"
    CLO_log_level   "xx : set logging level to xx, where xx in [0..%d]\n",
                    MAX_LOG_LEVEL
 );
 return 0;
}

 /*
  *  This function initializes the plc.
  * If atomic == 0, then it uses best effort semantics, i.e.
  *                 it initliases the most sub-components as
  *                 possible.
  *                 It exits successfully if every sub-componenet was
  *                 correctly initialised, otherwise it exits with an error,
  *                 but leaves initialised all the correctly initialised
  *                 sub-componets.
  * If atomic != 0, then it uses atomic semantics, i.e.
  *                 if it cannot initialise all sub-components correctly,
  *                 then it will undo anything it did untill that moment,
  *                 and exit with an error.
  *                 This is, if it exits with an error, then no sub-component
  *                 is initialised. If it exits succesfully, then every
  *                 sub-component is correctly initialised.
  */
static int plc_init__(char const *mod_name, int argc, char **argv, int atomic)
{
  int privmap_shm_key = 0;
  int confmap_shm_key = 0;
  int force_init;
  int log_level = DEF_LOG_LEVEL;
  const char *log_file = NULL;
  const char *errlog_file = NULL;
  const char *wrnlog_file = NULL;
  const char *trclog_file = NULL;
  const char *conffile = NULL;
  gmm_loc_t location = loc_default;

  if (debug)
    printf("plc_init__(...): module_name = %s\n", mod_name);

  __assert_plc_not_initialized(-1);

  /* register exit function */
  if (!plc_get_status(registered_exit_func))
    if (atexit((void *) &plc_done) < 0)
      return -1;
  plc_set_status(registered_exit_func);

  /* set the defaults */
  confmap_shm_key = -1;  /* unspecified, let the cmm decide for the default */
  privmap_shm_key = -1;  /* unspecified, let the gmm decide for the default */
  conffile = DEF_CONFIG_FILE;
  location = loc_default;
  log_level = DEF_LOG_LEVEL;
  errlog_file = DEF_ERR_LOG_FILE;
  trclog_file = DEF_TRC_LOG_FILE;
  wrnlog_file = DEF_WRN_LOG_FILE;
  force_init = DEF_FORCE_INIT;

  /* parse arguments */
  if (plc_parse_args(argc, argv,
		     &location,
		     &privmap_shm_key, &confmap_shm_key,
		     &conffile, &plc_module_name_,
                     &log_level,
                     &log_file,
                     &force_init,
                     1 /* remove parsed args from **argv */)
      < 0)
    return -1;

  if (log_file != NULL)
    trclog_file = wrnlog_file = errlog_file = log_file;

  /* if it wasn't set from the command line, use the default */
  if (plc_module_name_ == NULL)
    if ((plc_module_name_ = strdup(mod_name)) == NULL) {
      printf("Could not allocate memory for module name...\n");
      goto error_exit;
    }

  /* initialize the logging module */
  if (plc_log_init(log_level, plc_module_name_,
                   trclog_file, wrnlog_file, errlog_file)
      < 0) {
    printf("Error initializing the logging sub-system...\n");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the logging system...");

  /* initialize conffile module */
  if (conffile_init(conffile, plc_module_name_) < 0) {
    plc_log_errmsg(1, "Error initializing the config file parser...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the config file parser...");

  /* call the cmm init function */
  if (cmm_init(plc_module_name_, confmap_shm_key) < 0) {
    plc_log_errmsg(1, "Error initializing access to the plc config memory...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the plc config memory...");

  /* call the gmm init function */
  if (gmm_init(plc_module_name_, location, privmap_shm_key) < 0) {
    plc_log_errmsg(1,
                   "Error initializing the access to the plc state memory...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the plc state memory...");

  /* call the rt init function */
  if (rt_init(plc_module_name_) < 0) {
    plc_log_errmsg(1, "Error initializing the real-time sub-system...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the real-time system...");

  /* call the synch init function */
  if (synch_init(plc_module_name_) < 0) {
    plc_log_errmsg(1, "Error initializing the synchronisation sub-system...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the synchronisation system...");

  /* call the period init function */
  if (period_init(plc_module_name_) < 0) {
    plc_log_errmsg(1, "Error initializing the period sub-system...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the period system...");

  /* call the state init function */
  if (state_init(plc_module_name_, force_init) < 0) {
    plc_log_errmsg(1, "Error initializing the state sub-system...");
    goto error_exit;
  }
  plc_log_trcmsg(9, "Correctly initialised the state system...");


   /* setup the TERM signal interrupt handler so we cleanup
    * correctly when asked to shutdown...
    */
  if (!plc_get_status(registered_sigterm_handler))
    if (signal_sethandler(SIGTERM, &sigterm_handler, &sigterm_old_action_) < 0) {
      plc_log_errmsg(1, "Error setting the TERM signal handler.");
      goto error_exit;
    }
  plc_set_status(registered_sigterm_handler);

   /* setup the INT signal interrupt handler so we cleanup
    * correctly when asked to shutdown...
    */
  if (!plc_get_status(registered_sigint_handler))
    if (signal_sethandler(SIGINT, &sigint_handler, &sigint_old_action_) < 0) {
      plc_log_errmsg(1, "Error setting the INT signal handler.");
      goto error_exit;
    }
  plc_set_status(registered_sigint_handler);

  plc_set_status(plc_fully_initialised);
  plc_log_trcmsg(1, "Module started.");
  return 0;

  /* cleanup on error */
error_exit:
  if (atomic != 0)
    plc_done();
  return -1;
}



int plc_init    (char const *mod_name, int argc, char **argv) {
  return plc_init__(mod_name, argc, argv, 1);
}

  /* A module will normally not be calling this function         */
  /* This function is mainly to be used by the plc_shutdown()    */
  /* function that first tries to setup access to plc resources. */
  /* If we want to support shutting down a partially setup plc,  */
  /* then we must also support partial initialisation of that    */
  /* same (brocken) plc.                                         */
int plc_init_try(char const *mod_name, int argc, char **argv) {
  return plc_init__(mod_name, argc, argv, 0);
}


/*
 * Make this function robust, i.e. may be called multiple times, and with
 * and with partially initialized plc.
 *
 * The above means each of the sub-system _done() functions must also
 * be re-entrant (i.e. be called multiple times...)
 */
int plc_done(void)
{
  int tmp_res, res = 0;

  if (debug)
    printf("plc_done():\n");

  /* We do not assert the plc is fully initialized. */
  /* We want to be able to shutdown a partially initialized plc... */
  /*
   * Currently (31-10-2000) the isolate version of the gmm is using this
   *  "feature".
   *
   * Normal plc shutdown, i.e. the plc_shutdown() function, also
   * uses this feature, along with the plc_init_try() function to
   * shutdown a partially setup (and therefore brocken) plc.
   */
/*__assert_plc_initialized(-1);*/

   /* reset the TERM signal interrupt handler */
  if (signal_action(SIGTERM, &sigterm_old_action_, NULL) < 0) {
    plc_log_errmsg(9, "Error resetting the TERM signal handler.");
    /* TODO: check whether the previous handler was *our* handler,
    * otherwise, we have the signal handlers all mixed up, and it is
    * probably best to leave things as they were.
    */
  }

   /* reset the INT signal interrupt handler */
  if (signal_action(SIGINT, &sigint_old_action_, NULL) < 0) {
    plc_log_errmsg(9, "Error resetting the INT signal handler.");
    /* TODO: check whether the previous handler was *our* handler,
    * otherwise, we have the signal handlers all mixed up, and it is
    * probably best to leave things as they were.
    */
  }

  if ((tmp_res = state_done()) < 0)
    plc_log_errmsg(9, "Error closing down the state sub-system.");
  else
    plc_log_trcmsg(9, "Sate sub-system correctly closed down.");
  res |= tmp_res;

  if ((tmp_res = period_done()) < 0)
    plc_log_errmsg(9, "Error closing down the period sub-system.");
  else
    plc_log_trcmsg(9, "Period sub-system correctly closed down.");
  res |= tmp_res;

  if ((tmp_res = synch_done()) < 0)
    plc_log_errmsg(9, "Error closing down the synchronisation sub-system.");
  else
    plc_log_trcmsg(9, "Sycnhronisation sub-system correctly closed down.");
  res |= tmp_res;

  if ((tmp_res = rt_done()) < 0)
    plc_log_errmsg(9, "Error closing down the real-time sub-system.");
  else
    plc_log_trcmsg(9, "Real-time sub-system correctly closed down.");
  res |= tmp_res;

  if ((tmp_res = gmm_done()) < 0)
    plc_log_errmsg(9, "Error closing down access to the global memory.");
  else
    plc_log_trcmsg(9, "Access to the global memory correctly closed down.");
  res |= tmp_res;

  if ((tmp_res = cmm_done()) < 0)
    plc_log_errmsg(9, "Error closing down access to the configuration memory.");
  else
    plc_log_trcmsg(9, "Access to the config. memory correctly closed down.");
  res |= tmp_res;

  if ((tmp_res = conffile_done()) < 0)
    plc_log_errmsg(9, "Error closing down the config. file parser.");
  else
    plc_log_trcmsg(9, "Config. file parser correctly closed down.");
  res |= tmp_res;

  res |= plc_log_done();

  free(plc_module_name_);
  plc_module_name_ = NULL;

  plc_rst_status(plc_fully_initialised);

  return (res == 0)?0:-1;
}


int plc_scan_beg(void)
{
  int res = 0;

  __assert_plc_initialized(-1);

  res |= rt_scan_beg(); 
  res |= period_scan_beg(); 
  res |= synch_scan_beg();

  return res;
}


int plc_scan_end(void)
{
  int res = 0;

  __assert_plc_initialized(-1);

  res |= synch_scan_end();
  res |= period_scan_end();
/*  res |= rt_scan_end();  */ /* not required for now... */

  return res;
}


const char * plc_module_name(void)
{
 __assert_plc_initialized(NULL);

 return strdup(plc_module_name_);
}



