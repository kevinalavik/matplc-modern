/*
 * (c) 2001 Mario de Sousa
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
 * This file implements the functions in
 *   state.h
 *   state_private.h
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <plc.h>
#include <log/log.h>
#include <conffile/conffile.h>
#include <cmm/cmm.h>
#include <synch/synch.h>
#include <misc/string_util.h>
#include <misc/signal_util.h>
#include <misc/timer_util.h>

#include "state.h"
#include "state_private.h"



static const int debug = 0;


/************************************************************/
/*****************                          *****************/
/*****************    Variables global to   *****************/
/*****************      state library       *****************/
/*****************                          *****************/
/************************************************************/

  /* the current status of the state library */
static status_t state_status_ = {0};

  /* pointer to the MatPLC wide state cmm memory block */
static state_shared_data_t *state_shared_shm_ = NULL;

  /* pointer to the module's private state cmm memory block */
static state_module_data_t *state_module_shm_ = NULL;

  /* the module name */
static const char *state_module_name_ = NULL;



/************************************************************/
/*****************                          *****************/
/*****************    Local Auxiliary       *****************/
/*****************       Functions          *****************/
/*****************                          *****************/
/************************************************************/

/* access functions for the status_ variable */
static inline int state_get_status(const status_t stat_bit) {
  return (state_status_.s & stat_bit.s) != 0;
}

static inline void state_set_status(const status_t stat_bit) {
  state_status_.s |= stat_bit.s;
}

static inline void state_rst_status(const status_t stat_bit) {
  state_status_.s &= (~stat_bit.s);
}


#define __assert_state_not_fully_initialized(ret_val) {         \
          if (state_get_status(state_fully_initialized) != 0)  \
            return ret_val;                                      \
        }

#define __assert_state_fully_initialized(ret_val) {             \
          if (state_get_status(state_fully_initialized) == 0)  \
            return ret_val;                                      \
        }


/* get a module's run/stop mode...
 *
 * returns: 1 - RUN mode
 *          0 - STOP mode
 *         -1 - error...
 *
 */
static int get_runstop_mode(plc_synchplace_t place /* place used by the module... */)
{
  plc_transition_t transition;
  plc_synchpt_t synchpt;
  int res;

  /* create an apropriate synchpt... */
  transition = plc_transition_null();
  if (!plc_transition_is_valid(transition))
    goto error_exit_0;

  /* NOTE: we must create a non-blocking synchpt, in case
   * the module has already been stopped, and so we don't
   * block trying to stop it once again!
   */
  synchpt = plc_synchpt_new(transition, 2, plc_synchpt_nonblocking);
  if (!plc_synchpt_is_valid(synchpt))
    goto error_exit_0;

  /* Add an arc to remove a token from the RUN/STOP place...
   * ... and another to re-insert that same token into the RUN/STOP place...
   */
  if (plc_synchpt_add_arc(synchpt, -1, place) < 0)
    goto error_exit_1;
  if (plc_synchpt_add_arc(synchpt,  1, place) < 0)
    goto error_exit_1;

  /* We must make sure it becomes empty, even though it may have been
   * placed in run mode multiple times...
   */
  res = plc_synch(synchpt);
  plc_synchpt_free(synchpt);

  if (res >= 0)
    /* synchronised succesfully => RUN mode */
    return 1;

  if (res == -1)
    /* could not synchronise => STOP mode... */
    return 0;

  /* An error must have ocurred... */
  return -1;

error_exit_1:
  plc_synchpt_free(synchpt);
error_exit_0:
  return -1;
}




/* Place a module in run mode... */
static int activate_run_mode(plc_synchplace_t place /* place used by the module... */)
{
  plc_transition_t transition;
  plc_synchpt_t synchpt;

  if (debug)
    printf("activate_run_mode(): called... \n");

  if (get_runstop_mode(place) == 1)
    /* module already running... */
    /* NOTE: We have a race condition here, but it doesn't matter...
     *       Details...
     *         Several processes may call this function to place the
     *         same module in RUN mode. In that case, they may all think
     *         that the module is not running, so they all
     *         proceed on to execute the remaining code in this function.
     *         The result will be that the RUN/STOP place will get more
     *         than a single token, but that is OK.
     *         When placing the module in STOP mode, we will be running
     *         a loop, removing one token at a time until the place becomes
     *         empty, so it doesn't really matter how many tokens we get to
     *         insert into the RUN/STOP place...
     *         Then, why do we check whether the module is already in RUN
     *         mode at all, instead of simply inserting an extra token and
     *         be done with it? This is beacuse the sempahore may reach the
     *         maximum number of tokens limit, and we would then be returning
     *         an error, saying we couldn't place the module in RUN mode,
     *         when it already is in RUN mode...
     *         The above may still happen if lot's of race conditions occur,
     *         but it's probably not owrth bothering about.... ???
     */
    return 0;

  /* create an apropriate synchpt... */
  transition = plc_transition_null();
  if (!plc_transition_is_valid(transition))
    goto error_exit_0;

  synchpt = plc_synchpt_new(transition, 1, plc_synchpt_blocking);
  if (!plc_synchpt_is_valid(synchpt))
    goto error_exit_0;

  /* Add an arc to insert a token into the RUN/STOP place... */
  if (plc_synchpt_add_arc(synchpt, 1, place) < 0)
    goto error_exit_1;

  if (plc_synch(synchpt) < 0)
    goto error_exit_1;

  plc_synchpt_free(synchpt);

  if (debug)
    printf("activate_run_mode(): returning success... \n");
  return 0;

error_exit_1:
  plc_synchpt_free(synchpt);
error_exit_0:
  if (debug)
    printf("activate_run_mode(): returning error... \n");
  return -1;
}



/* Place a module in stop mode... */
static int activate_stop_mode(plc_synchplace_t place /* place used by the module... */)
{
  plc_transition_t transition;
  plc_synchpt_t synchpt;
  int res;

  if (debug)
    printf("activate_stop_mode(): called... \n");

  /* create an apropriate synchpt... */
  transition = plc_transition_null();
  if (!plc_transition_is_valid(transition))
    goto error_exit_0;

  /* NOTE: we must create a non-blocking synchpt, in case
   * the module has already been stopped, and so we don't
   * block trying to stop it once again!
   */
  synchpt = plc_synchpt_new(transition, 1, plc_synchpt_nonblocking);
  if (!plc_synchpt_is_valid(synchpt))
    goto error_exit_0;

  /* Add an arc to remove a token from the RUN/STOP plcae... */
  if (plc_synchpt_add_arc(synchpt, -1, place) < 0)
    goto error_exit_1;

  /* We must make sure it becomes empty, even though it may have been
   * placed in run mode multiple times...
   * ... so we remove one token at a time, until the RUN/STOP place
   * become empty!
   */
  while ((res = plc_synch(synchpt)) >= 0);

  /* If we stopped because of an error, and not because there
   * are no more places to remove...
   */
  if (res != -1)
    goto error_exit_1;

  plc_synchpt_free(synchpt);

  if (debug)
    printf("activate_stop_mode(): returning success... \n");
  return 0;

error_exit_1:
  plc_synchpt_free(synchpt);
error_exit_0:
  if (debug)
    printf("activate_stop_mode(): returning error... \n");
  return -1;
}



/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************     state_private.h     *****************/
/*****************                          *****************/
/************************************************************/





/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************         state.h          *****************/
/*****************                          *****************/
/************************************************************/


 /* (force_init > 0) => initialise the module, even if we find
  * another process runing what seems to be a PLC module
  * with the same name!
  */
int state_init(const char *module_name, int force_init)
{
  u32 size;
  plc_synchplace_t place;
  plc_synchpt_t synchpt;

  __assert_state_not_fully_initialized(-1);

  /* find the struct with the shared state data... */
  state_shared_shm_ = cmm_block_find(STATE_SHARED_T_BLOCK_TYPE,
                                     STATE_SHARED_T_BLOCK_NAME,
                                     &size);
  if ((state_shared_shm_ == NULL) || (size != sizeof(state_shared_data_t))) {
    plc_log_errmsg(9, "state_init(): error accessing state shared memory block.");
    goto error_exit_0;
  }

  /* verify if a module with the same name has already
   * created the shared memory block
   */
  state_module_shm_ = cmm_block_find(STATE_MODULE_T_BLOCK_TYPE,
                                     module_name,
                                     &size);
  if ((state_module_shm_ != NULL) && (size != sizeof(state_module_data_t))) {
    plc_log_errmsg(9, "state_init(): module's state memory block has wrong size.");
    goto error_exit_0;
  }

  if (state_module_shm_ != NULL) {
    /* A cmm_block has been found! This means that either:
     *    - a module with the same name is currently already running.
     *    - a module with the same name has run before, but has crashed without
     *      cleaning up!
     *
     * We will now verify which of the above two is correct.
     */

     /* Does the process exist? */
    if (kill(state_module_shm_->pid, 0) >= 0) {
       /* yes it does!! */

     /* Unfortunately, there is no gurantee that the running process
      * is still the same process running the PLC module. Remember that
      * process id's get re-used, so it could be a completely unrelated
      * process. In other words, we could be getting false positrives.
      *
      * The only way to really distinguish is to establish a communication
      * path to that process and ask it.
      *  - Using semaphores -
      *    The first process would setup a semaphore and set it's value
      *    to 0. It would then spawn a pthread to wait on this sempahore.
      *    The semaphore id would be stored in the state_shm_ struct, along
      *    with a response counter also initialised to 0.
      *    The new process would read the response counter, signal
      *    the semaphore, and wait for a given time-out period. The old
      *    process (actually it's pthread) would then increment the response
      *    counter and wait again on the semaphore.
      *    The new process, after the time-out, would read the response
      *    counter to see if it had been incremented.
      *
      *  - Using named FIFO's -
      *  - Using ...          -
      *
      *  Actually, practically any IPC mechanism that has blocking calls
      *  could be used, but this mechanism basically *sucks* !!, and in the
      *  end we still have no proof that the old process is a PLC module. If
      *  the system is heavily loaded it could take longer than the timeout
      *  period to increment the response counter. In other words, we could
      *  get false negatives, though they should be less in number than the
      *  false positives without this hack.
      *
      *  But which is more dangerous, a false positive or a false negative?
      *
      * We might just as well leave all this ugly hacking out, and ask the user
      * for assistance. This is also used by Netscape and other mainstream
      * programs, so there probably is no better fix.
      *
      */

      if (force_init <= 0) {
       /* OK, so let's ask the user if it really is a PLC process. */

        plc_log_errmsg(1, "A module named %s is already running with pid %d. "
                          "If you are sure that this process is not a PLC module "
                          "then re-issue the command using the %s command line option",
                       module_name, state_module_shm_->pid,
                       CLO_force_init);
        goto error_exit_0;

      } else {
        /* The user is asking us to dis-regard the running process,
         * and launch the new module anyway.
         * This means we assume the running process has nothing to
         * do with our PLC program.
         *
         * I sure hope the user knows what he is doing...!
         *
         * Since we don't want multiple state cmm_blocks under the same name,
         * we first remove the existing cmm_block.
         */
        pid_t tmp_pid = state_module_shm_->pid;

        if (cmm_block_free(state_module_shm_) < 0) {
          plc_log_errmsg(1, "A module named %s is already running with pid %d. "
                            "You wish to dis-regard this process (%s), but an error "
                            "ocurred while removing it's state info from the plc "
                            "so no further action will be taken...",
                         module_name, tmp_pid,
                         CLO_force_init);
          goto error_exit_0;
        }

        state_module_shm_ = NULL;

        /*
         * We now let it fall through to the normal procedures of a new module,
         * i.e. create a new cmm_block.
         *
         * Note that the new block _may_ even re-use the same memory previously use
         * by the block we have just released, and that the existing process does
         * not know we released it!
         *
         * Having multiple processes writing to the same cmm_block sure will
         * mess things up!
         *
         * Once again, I sure hope the user knows what he is doing...!
         */
      }
    }
  }

    /* no block found. We will create a new one */

      /* create configuration shared memory block */
  state_module_shm_ = cmm_block_alloc(STATE_MODULE_T_BLOCK_TYPE,
                                      module_name,
                                      sizeof(state_module_data_t));

  if (state_module_shm_ == NULL) {
    plc_log_errmsg(1, "state_init(): could not alloc config memory block.");
    goto error_exit_0;
  }

   /* initialise the cmm block */
  state_module_shm_->pid  = getpid();

   /* remember the module's name */
  state_module_name_ = module_name;

  /******************************************************************/
  /* setup the synchronisation mechanisms for the RUN/STOP modes... */
  /******************************************************************/
  /* Add the RUN/STOP place... */
  synchpt = plc_synchpt_scan_beg();
  if (!plc_synchpt_is_valid(synchpt))
    goto error_exit_1;

  place = plc_synchplace_add();
  if (!plc_synchplace_is_valid(place))
    goto error_exit_1;

   /* remember the module's RUN/STOP place */
  state_module_shm_->run_stop_place  = place;

  /* Add an arc to wait on the module RUN/STOP place, and
   * another to simultaneously signal that place, to the
   * scan_beg sycnhpt...
   */
  if (plc_synchpt_add_arc(synchpt,  1, state_module_shm_->run_stop_place) < 0)
    goto error_exit_2;
  if (plc_synchpt_add_arc(synchpt, -1, state_module_shm_->run_stop_place) < 0)
    goto error_exit_2;

  /* Add an arc to wait on the MatPLC wide RUN/STOP place,
   * and another to simultaneously signal that place, to
   * the scan_beg sycnhpt...
   */
  if (plc_synchpt_add_arc(synchpt,  1, state_shared_shm_->run_stop_place) < 0)
    goto error_exit_2;
  if (plc_synchpt_add_arc(synchpt, -1, state_shared_shm_->run_stop_place) < 0)
    goto error_exit_2;


   /* by default, module starts off in run mode... */
  if (activate_run_mode(place) < 0)
    goto error_exit_2;

  state_set_status(state_fully_initialized);
  return 0;


error_exit_2:
  plc_synchplace_del(place);
error_exit_1:
  cmm_block_free(state_module_shm_);
error_exit_0:
  state_module_shm_ = NULL;
  state_shared_shm_ = NULL;
  return -1;
}



int state_done(void)
{
  int res = 0;

  __assert_state_fully_initialized(-1);

     /* free the module's RUN/STOP place */
  if (plc_synchplace_del(state_module_shm_->run_stop_place) < 0)
    res = -1;

    /* module will no longer be running, so we remove it's state info from the cmm */
  if (cmm_block_free(state_module_shm_) < 0) {
    plc_log_errmsg(1, "state_init(): could not free config memory block.");
    res = -1;
  }

  state_module_shm_  = NULL;
  state_module_name_ = NULL;

  state_rst_status(state_fully_initialized);

  return res;
}




/*
 * Get the number of extra arcs the state library will be adding to the
 * scanbeg synchpt...
 */
int plc_scanbeg_extra_arcs(void) {
  return STATE_LIB_EXTRA_ARCS;
}



/*
 * Get the number of currently installed modules
 */
int plc_module_count(void)
{
  return cmm_block_count(STATE_MODULE_T_BLOCK_TYPE);
}



/*
 * Get the module name by index
 */
int plc_module_by_index(int module_index, char **module_name)
{
  state_module_data_t *module_state;
  u32 size;

  module_state = cmm_block_get(STATE_MODULE_T_BLOCK_TYPE,
                               module_index,
                               &size,
                               module_name);

  if ((module_state == NULL) || (size != sizeof(state_module_data_t)))
    return -1;

  return 0;
}


/*
 *  Shutdown a module
 */
int plc_module_shutdown(char *name)
{
  state_module_data_t *module_state;
  u32 size;

  module_state = cmm_block_find(STATE_MODULE_T_BLOCK_TYPE,
                                name,
                                &size);

  if ((module_state == NULL) || (size != sizeof(state_module_data_t)))
    return -1;

  if (kill(module_state->pid, SIGTERM) < 0) {
    if (errno == ESRCH) {
      /* process does not exist... */
      /* If the process died for some unkonown reason,
       * but left behind it's module state block,
       * we simply remove that block ourselves...
       */
      cmm_block_free(module_state);
    }

    if (errno == EPERM) {
      plc_log_wrnmsg(1, "You do not have permision to shut down the "
                        "%s module (pid = %d).",
                     name, module_state->pid);
    }

    return -1;
  }

  return 0;
}


/*
 * Shutdown all the modules
 * (except the calling process, if it too is a running module)
 * The calling process can easily kill itself by simply calling exit()
 * if it so wishes.
 */
int plc_module_shutdown_all(void)
{
  state_module_data_t *module_state;
  char *name;
  u32 size;
  int module_index, module_count;
  int res = 0;
  char **module_name_array;
  pid_t calling_module_pid = getpid();

  /* NOTE: We cannot loop through the modules and kill them off
   *       as we go. As we kill the module, the indexing of
   *       the module state blocks in the cmm changes!!
   *       Since we have no guarantees when the process
   *       being killed actually dies and removes it's block
   *       from the cmm, we have no alternative than to
   *       alloc some memory for ourselves and kill off the
   *       processes after copying the relevant data to our local
   *       memory...
   */

  if((module_count = plc_module_count()) <= 0) {
    /* it can never be 0. The module that called this function must
     * be running, so the minimum will always be 1!
     */
    plc_log_wrnmsg(9, "Cannot find any running modules.");
    return -1;
  }

  if ((module_name_array = (char **)malloc(sizeof(char *) * module_count)) == NULL) {
    plc_log_wrnmsg(9, "Ran out of memory (malloc() == NULL).");
    return -1;
  }

   /* assume no errors */
  res = 0;

   /* read in all the module names */
  for(module_index = 0; module_index < module_count; module_index++) {

    module_name_array[module_index] = NULL;

    module_state = cmm_block_get(STATE_MODULE_T_BLOCK_TYPE, module_index, &size, &name);
    if (module_state == NULL) {
      plc_log_wrnmsg(9, "Error obtaining module's %d state info.", module_index);
      goto loop_1_error;
    }

    if (size != sizeof(state_module_data_t)) {
      plc_log_wrnmsg(9, "Found an error in module's %d state struct size.",
                     module_index);
      goto loop_1_error;
    }

    if (module_state->pid != calling_module_pid)
       /* everything works out OK. */
      module_name_array[module_index] = name;

    continue;

  loop_1_error:
    res = -1;
    continue;
  }

   /* Now kill off all the modules */
  for(module_index = 0; module_index < module_count; module_index++) {

    if (module_name_array[module_index] != NULL)
      if (plc_module_shutdown(module_name_array[module_index]) < 0)
        res = -1;
  }

  free(module_name_array);

  return res;
}



/* Place a module in run mode... */
int plc_module_run(const char *name /* the module name... */)
{
  u32 size;
  state_module_data_t *module_state;

  module_state = cmm_block_find(STATE_MODULE_T_BLOCK_TYPE, name, &size);

  if ((module_state == NULL) || (size != sizeof(state_module_data_t)))
    return -1;

  if (activate_run_mode(module_state->run_stop_place) < 0)
    return -1;

  return 0;
}


/* Place a module in stop mode... */
int plc_module_stop(const char *name /* the module name... */)
{
  u32 size;
  state_module_data_t *module_state;

  module_state = cmm_block_find(STATE_MODULE_T_BLOCK_TYPE, name, &size);

  if ((module_state == NULL) || (size != sizeof(state_module_data_t)))
    return -1;

  if (activate_stop_mode(module_state->run_stop_place) < 0)
    return -1;

  return 0;
}


/* Place the MatPLC in run mode... */
int plc_run(void)
{
  u32 size;
  state_shared_data_t *shared_state;

  shared_state = cmm_block_find(STATE_SHARED_T_BLOCK_TYPE,
                                STATE_SHARED_T_BLOCK_NAME,
                                &size);

  if ((shared_state == NULL) || (size != sizeof(state_shared_data_t)))
    return -1;

  if (activate_run_mode(shared_state->run_stop_place) < 0)
    return -1;

  return 0;
}


/* Place the MatPLC in stop mode... */
int plc_stop(void)
{
  u32 size;
  state_shared_data_t *shared_state;

  shared_state = cmm_block_find(STATE_SHARED_T_BLOCK_TYPE,
                                STATE_SHARED_T_BLOCK_NAME,
                                &size);

  if ((shared_state == NULL) || (size != sizeof(state_shared_data_t)))
    return -1;

  if (activate_stop_mode(shared_state->run_stop_place) < 0)
    return -1;

  return 0;
}


/* get details from the module state entry in the cmm */
int plc_module_details(const char *name, /* the module name - compulsory          */
		        pid_t *pid,      /* the module's pid                      */
                                 /* the place being used for the RUN/STOP mode... */
                        plc_synchplace_t *place,
                        int *mode,       /* The current RUN=1 / STOP=0 mode...    */
                        int *active      /* does the process seem to be running ? */
                       )
{
  u32 size;
  state_module_data_t *module_state;

  module_state = cmm_block_find(STATE_MODULE_T_BLOCK_TYPE, name, &size);

  if ((module_state == NULL) || (size != sizeof(state_module_data_t)))
    return -1;

  if (pid != NULL)
    *pid = module_state->pid;
  if (place != NULL)
    *place = module_state->run_stop_place;
  if (mode != NULL)
    *mode = get_runstop_mode(module_state->run_stop_place);
  if (active != NULL)
    *active = (kill(module_state->pid,0) < 0)?0:1;

  return 0;
}
