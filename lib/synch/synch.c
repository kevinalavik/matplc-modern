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


/*
 * This file implements the functions in
 *   synch.h
 *   synch_private.h
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>  /* required for memove() */

#include <log/log.h>
#include <conffile/conffile.h>
#include <cmm/cmm.h>
#include <rt/rt.h>
#include <misc/string_util.h>
#include <misc/mutex_util.h>

#include <state/state.h>  /* required for plc_scanbeg_extra_arcs(); */

#include "synch.h"
#include "synch_private.h"

#include "synch_sem.h"


/* #define DEBUG */



/************************************************************/
/*****************                          *****************/
/*****************    Variables global to   *****************/
/*****************       synch library      *****************/
/*****************                          *****************/
/************************************************************/

/* the current status of the synch library */
static status_t synch_status_ = {0};

static synch_t *synch_shm_ = NULL;

static const char * synch_module_name_ = NULL;

static plcsynchsem_t synch_semid_ = PLCSYNCHSEM_FAILED;

static plcmutex_t place_alloc_mutex_ = PLCMUTEX_FAILED;


/* synchpts used to synch by the synch_scan_xxx() functions */
static plc_synchpt_t synchpt_scanbeg_ = INVALID_SYNCHPT;
static plc_synchpt_t synchpt_scanend_ = INVALID_SYNCHPT;



/************************************************************/
/*****************                          *****************/
/*****************    Local Auxiliary       *****************/
/*****************       Functions          *****************/
/*****************                          *****************/
/************************************************************/

/* access functions for the status_ variable */
static inline int synch_get_status(const status_t stat_bit) {
  return (synch_status_.s & stat_bit.s) != 0;
}

static inline void synch_set_status(const status_t stat_bit) {
  synch_status_.s |= stat_bit.s;
}

static inline void synch_rst_status(const status_t stat_bit) {
  synch_status_.s &= (~stat_bit.s);
}


#define __assert_synch_not_fully_initialized(ret_val) {        \
          if (synch_get_status(synch_fully_initialized) != 0)  \
            return ret_val;                                    \
        }

#define __assert_synch_fully_initialized(ret_val) {            \
          if (synch_get_status(synch_fully_initialized) == 0)  \
            return ret_val;                                    \
        }



/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************      synch_private.h     *****************/
/*****************                          *****************/
/************************************************************/

/* Read a name = value pair from the config file, and interpret it as a */
/* transition. If no name=value pair exists, and use_default == 1, then */
/* returns handle to the transition named def_name.                     */
static int synch_parse_transition(const char *section, const char *name,
                                  plc_transition_t *transition,
                                  const char *def_name, int use_default)
{
  char *value;
  const char *transition_name;
  plc_transition_t tmp_transition;

  if (section == NULL) {
    value = conffile_get_value(name);
  } else {
    value = conffile_get_value_sec(section, name);
  }

  if ((value == NULL) && ((!use_default) || (def_name == NULL))) {
    plc_log_wrnmsg(1, "Value %s not specified, and no default to use.",
                   name);
    return -1;
  }

  if (value == NULL) {
    transition_name = def_name;
  } else {
    transition_name = value;
  }

  tmp_transition = plc_transition_by_name(transition_name);

  if (value != NULL) free(value);

  if (plc_transition_is_valid(tmp_transition)) {
    *transition = tmp_transition;
    return 0;
  }

  return -1;
}




/* function to parse the config file for SYNCH related values */
int synch_parse_conf(int *sem_key,
                     plc_transition_t * scanbeg_transition,
                     plc_transition_t * scanend_transition)
{
  if (sem_key != NULL)
    conffile_parse_i32(SYNCH_SK_SECTION, SYNCH_SK_NAME, sem_key,
                       SYNCH_SK_MIN, SYNCH_SK_MAX, DEF_SYNCH_SEM_KEY, 1);

  if (scanbeg_transition != NULL) {
    char *def_name = strdup2(synch_module_name_, SYNCH_TRANSITION_BEG_SUFFIX);
    if ((synch_parse_transition(SYNCH_SCANBEG_SECTION, SYNCH_SCANBEG_NAME,
                                scanbeg_transition, def_name, 1))
        < 0) {
      *scanbeg_transition = plc_transition_null();
    }
    free(def_name);
  }

  if (scanend_transition != NULL) {
    char *def_name = strdup2(synch_module_name_, SYNCH_TRANSITION_END_SUFFIX);
    if ((synch_parse_transition(SYNCH_SCANEND_SECTION, SYNCH_SCANEND_NAME,
                                scanend_transition, def_name, 1))
        < 0) {
      *scanend_transition = plc_transition_null();
    }
    free(def_name);
  }

  return 0;
}



/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************          synch.h         *****************/
/*****************                          *****************/
/************************************************************/


int synch_init(const char *module_name) {
  int extra_arcs;
  u32 size;
  plc_transition_t transition_scanbeg, transition_scanend;

  __assert_synch_not_fully_initialized(-1);

  /* get the synch. library config memory block from the cmm */
  synch_shm_ = cmm_block_find(SYNCH_T_BLOCK_TYPE,
                              SYNCH_T_BLOCK_NAME,
                              &size);

  if (synch_shm_ == NULL)
    goto error_exit_0;

  /* If the following condition is false, we canot safely
   * access the synch_shm->place_count, etc.. variables!!
   */
  if (size < sizeof(synch_t))
    goto error_exit_0;

  check_synch_magic_ret(synch_shm_, -1);

  if (size != sizeof_synch_t(synch_shm_->place_count))
    goto error_exit_0;

  synch_module_name_ = module_name;

  /* get handles to the transitions used for the synch_scan_beg()
   * and synch_scan_end() functions
   */
  synch_parse_conf(NULL, /* we are not interested in the sem_key */
                   &transition_scanbeg,
                   &transition_scanend);

  if (!(plc_transition_is_valid(transition_scanbeg) &&
        plc_transition_is_valid(transition_scanend))) {
    plc_log_errmsg(1, "Could not get valid handle(s) to transition(s) "
                      "used for scan_beg/end.");
    goto error_exit_1;
    goto error_exit_3;
  }

  /* Create synchpts from the above transitions... */
  extra_arcs = plc_scanbeg_extra_arcs();
  if (extra_arcs < 0)
    goto error_exit_2;

  synchpt_scanbeg_ = plc_synchpt_new(transition_scanbeg,
                                     extra_arcs,
                                     plc_synchpt_blocking);
  if (!plc_synchpt_is_valid(synchpt_scanbeg_))
    goto error_exit_2;

  synchpt_scanend_ = plc_synchpt_new(transition_scanend, 0, plc_synchpt_blocking);
  if (!plc_synchpt_is_valid(synchpt_scanend_))
    goto error_exit_3;

  /* get access to the synch semaphore set... */
  synch_semid_ = plcsynchsem_open(synch_shm_->synch_semkey, rt_getpriority());
  if (synch_semid_ == PLCSYNCHSEM_FAILED)
    goto error_exit_4;
  
  /* get access to the place_alloc semaphor... */
  place_alloc_mutex_ = plcmutex_open(&(synch_shm_->place_alloc_mutexid));
  if (place_alloc_mutex_ == PLCMUTEX_FAILED)
    goto error_exit_5;
  
  synch_set_status(synch_fully_initialized);

  return 0;

/*
error_exit_6:
  plcmutex_close(place_alloc_mutex_);
  place_alloc_mutex_ = PLCMUTEX_FAILED;
*/
error_exit_5:
  plcsynchsem_close(synch_semid_);
  synch_semid_ = PLCSYNCHSEM_FAILED;
error_exit_4:
  plc_synchpt_free(synchpt_scanend_);
  synchpt_scanend_ = INVALID_SYNCHPT;
error_exit_3:
  plc_synchpt_free(synchpt_scanbeg_);
  synchpt_scanbeg_ = INVALID_SYNCHPT;
error_exit_2:
error_exit_1:
error_exit_0:
  synch_shm_ = NULL;
  return -1;
}


int synch_done(void) {
  __assert_synch_fully_initialized(-1);

  synch_module_name_ = NULL;
  synch_shm_ = NULL;

  plc_synchpt_free(synchpt_scanbeg_);
  plc_synchpt_free(synchpt_scanend_);
  synchpt_scanbeg_ = INVALID_SYNCHPT;
  synchpt_scanend_ = INVALID_SYNCHPT;

  plcsynchsem_close(synch_semid_);
  synch_semid_ = PLCSYNCHSEM_FAILED;
  plcmutex_close(place_alloc_mutex_);
  place_alloc_mutex_ = PLCMUTEX_FAILED;

  synch_rst_status(synch_fully_initialized);

  return 0;
}




inline int synch_scan_beg(void) {
  __assert_synch_fully_initialized(-1);

  return plcsynchsem_synch(synch_semid_,
                       synchpt_sem_parm(synchpt_scanbeg_),
                       synchpt_priv(synchpt_scanbeg_)->used_entries);
}


inline int synch_scan_end() {
  __assert_synch_fully_initialized(-1);

  return plcsynchsem_synch(synch_semid_,
                       synchpt_sem_parm(synchpt_scanend_),
                       synchpt_priv(synchpt_scanend_)->used_entries);
}



/**************************************************/
/*                                                */
/*  Functions to be used at module startup...     */
/*                                                */
/**************************************************/


/*
 * Create a place
 */
plc_synchplace_t plc_synchplace_add(void) {
  int index;
  plc_synchplace_t place;

  __assert_synch_fully_initialized(INVALID_SYNCHPLACE);

  /* assume error */
  place = INVALID_SYNCHPLACE;

  /* grab the sempahore */
  plcmutex_lock(place_alloc_mutex_);

  /* look for a free place... */
  for (index = 0; index < synch_shm_->place_count; index++) {
    if (places_in_use(synch_shm_)[index] == 0) {
      /* found a free place... */
      places_in_use(synch_shm_)[index] = 1;
      place = index;
      break; /* jump out of the for loop */
    }
  } /* for(;;) */

  /* release the datamap */
  plcmutex_unlock(place_alloc_mutex_);

  return place;
}


/*
 * Delete a place
 */
int plc_synchplace_del(plc_synchplace_t synchplace) {

  __assert_synch_fully_initialized(-1);

  if ((synchplace < 0) || (synchplace >= synch_shm_->place_count))
    /* invalid synch place... */
    return -1;

  /* grab the sempahore */
  plcmutex_lock(place_alloc_mutex_);

  /* free the place... */
  places_in_use(synch_shm_)[synchplace] = 0;

  /* release the datamap */
  plcmutex_unlock(place_alloc_mutex_);

  return 0;
}





/*
 * Get a transition handle. (This is mostly a function of the Confmap rather
 * than the SYNCH proper, but the SYNCH gets a say, too.)
 */
plc_transition_t plc_transition_by_name(const char *name) {
  plc_transition_t plc_transition;
  u32 size;

  plc_transition = cmm_block_find(SYNCH_TRANSITION_BLOCK_TYPE, name, &size);

  if (plc_transition == NULL)
    return INVALID_TRANSITION;

  return plc_transition;
}



/*
 * Get a transition handle. (This is mostly a function of the CMM rather
 * than the SYNCH proper, but the SYNCH gets a say, too.)
 *
 * If (*name  != NULL), the caller must deallocate memory with free(*name)
 */
plc_transition_t plc_transition_by_index(int transition_index, char **name)
{
  plc_transition_t plc_transition;
  u32 size;

  plc_transition = cmm_block_get(SYNCH_TRANSITION_BLOCK_TYPE,
                                 transition_index,
                                 &size,
                                 name);

  if (plc_transition == NULL)
    return INVALID_TRANSITION;

  return plc_transition;
}



/*
 * Create a handle to a null synch point.
 *
 * This function creates a handle to a null anonymous synch point.
 * Calls to plc_synch with this synchpt will return imediately.
 */
plc_transition_t plc_transition_null(void) {
  return plc_transition_by_name(SYNCH_NULL_TRANSITION_NAME);
}


/*
 * Get the number of configured transitions
 */
int plc_transition_count(void) {
  return cmm_block_count(SYNCH_TRANSITION_BLOCK_TYPE);
}



/* Get the details of a transition... */
/* This is intended to be used mostly by control programs (i.e. matplc utility.
 * and not by general modules.
 */
int plc_transition_details(plc_transition_t transition,
                             /* number of entries in each of the following arrays... */
                           int *place_count,
                           plc_transition_swvalue_t *wait_values[],
                           plc_transition_swvalue_t *signal_values[]) {

  /* The following is not strictly required, we won't be accessing the
   * synch_shm_ variable...
   */
/*
  __assert_synch_fully_initialized(INVALID_SYNCHPT);
*/

  check_transition_magic_ret(transition, -1);

  if (place_count != NULL)
    *place_count = transition_priv(transition)->place_count;

  if (wait_values != NULL)
    *wait_values = transition_wait_values(transition);

  if (signal_values != NULL)
    *signal_values = transition_signal_values(transition);

  return 0;
}




/* Returns handle to the synchpt being used for scan_beg... */
/* Should probably only be used by the state library!!      */
plc_synchpt_t plc_synchpt_scan_beg(void) {
  __assert_synch_fully_initialized(INVALID_SYNCHPT);

  return synchpt_scanbeg_;
}



/*
 * Create a new synchpt, with no defined arcs.
 *
 * extra_arcs -> maximum number of arcs it will be possible to add to the synchpt
 *               later on by calling plc_synchpt_add_arc()
 *
 * In case of errors when creating the synchpt, a
 * INVALID_SYNCHPT will be returned.
 *
 * This involves allocating memory for the sycnhpt using malloc()
 * NOTE: The SysV semaphore emulation based on POSIX semaphores
 *       requires that the synchpts be acessible from every MatPLC module
 *       i.e., that they be acessible from all MatPLC process. In order to
 *       achieve this, synchpts are placed in the cmm shared memory  
 *       if SysV semaphore emulation is turned on.
 */
plc_synchpt_t plc_synchpt_new(plc_transition_t transition,
                              unsigned int extra_arcs,
                              plc_synchpt_blocking_t blocking_opt) {

  int index, arc_count, block_size;
  plc_synchpt_t synchpt = NULL;

  /* The following is not strictly required, we won't be accessing the
   * synch_shm_ variable...
   */
/*
  __assert_synch_fully_initialized(INVALID_SYNCHPT);
*/

  check_transition_magic_ret(transition, INVALID_SYNCHPT);

  /* How many arcs will the synchpt have...? */
  arc_count = 0;

  for (index = 0; index < transition_priv(transition)->place_count; index++)
    if (transition_wait_values(transition)[index] >= 0)
      arc_count++;

  for (index = 0; index < transition_priv(transition)->place_count; index++)
    if (transition_signal_values(transition)[index] >= 0)
      arc_count++;

  arc_count += extra_arcs;

  if (arc_count > u16_MAX)
    return INVALID_SYNCHPT;

  /* Allocate memory for the required number of arcs... */
  block_size = (arc_count * sizeof(plcsynchsem_parm_t)) + sizeof(plc_synchpt_priv_t);

#if defined (PLCSYNCHSEM_SYSV)
  synchpt = (plc_synchpt_t)malloc(block_size);
#elif defined (PLCSYNCHSEM_POSIX)
  synchpt = (plc_synchpt_t)cmm_block_alloc(SYNCH_SYNCHPT_BLOCK_TYPE,
                                           SYNCH_SYNCHPT_BLOCK_NAME,
                                           block_size);

#else
#error Do not know which memory allocation method to use.
#endif

  if (synchpt == NULL)
    return INVALID_SYNCHPT;

  /* initialise values... */
  synchpt_priv(synchpt)->magic         = SYNCHPT_MAGIC;
  synchpt_priv(synchpt)->sem_flg       = 0;
  synchpt_priv(synchpt)->total_entries = arc_count;
  synchpt_priv(synchpt)->used_entries  = 0;

  if (blocking_opt == plc_synchpt_nonblocking)
    synchpt_priv(synchpt)->sem_flg |=  IPC_NOWAIT;
  else
    synchpt_priv(synchpt)->sem_flg &= ~IPC_NOWAIT;

  /* initialise the arcs... */
  for (index = 0; index < transition_priv(transition)->place_count; index++)
    if (transition_wait_values(transition)[index] >= 0)
      plc_synchpt_add_arc(synchpt,
                          -transition_wait_values(transition)[index],
                          index);

  for (index = 0; index < transition_priv(transition)->place_count; index++)
    if (transition_signal_values(transition)[index] >= 0)
      plc_synchpt_add_arc(synchpt,
                          transition_signal_values(transition)[index],
                          index);

  /* set the magic... */
  synchpt_priv(synchpt)->magic = SYNCHPT_MAGIC;

  /* synchpt created... */
  return synchpt;
}



/*
 * Delete a synchpt
 */
void plc_synchpt_free(plc_synchpt_t synchpt)
{
  /* The following is not strictly required, we won't be accessing the
   * synch_shm_ variable...
   */
/*
  __assert_synch_fully_initialized(INVALID_SYNCHPT);
*/

  check_synchpt_magic(synchpt);

  /* we will be freeing the memory, but just make sure that,
   * if we are ever passed the same pointer again (even after being
   * free()'d), we will be able to detect that it is no longer
   * valid...
   */
  synchpt_priv(synchpt)->magic = 0;

#if defined (PLCSYNCHSEM_SYSV)
  free(synchpt);
#elif defined (PLCSYNCHSEM_POSIX)
  cmm_block_free(synchpt);
#else
#error Do not know which memory allocation method to use.
#endif
}




/*
 * Add an arc to a synchpt
 *
 * NOTE:
 *          wieght < 0 => arc from place to transition (synchpt)
 *          wieght = 0 => null arc from place to transition (synchpt)
 *          wieght > 0 => arc from transition (synchpt) to place
 */
int plc_synchpt_add_arc(plc_synchpt_t synchpt, int weight, plc_synchplace_t place) {
  int entry;

  check_synchpt_magic_ret(synchpt, -1);

  /* Search for a previous arc connecting the same synchpt and place,
   * with the correct direction...
   */
  for (entry = 0; entry < synchpt_priv(synchpt)->used_entries; entry++) {
    if (synchpt_sem_parm(synchpt)[entry].sem_num == place)
      if (((synchpt_sem_parm(synchpt)[entry].sem_op < 0) && (weight < 0)) ||
          ((synchpt_sem_parm(synchpt)[entry].sem_op > 0) && (weight > 0)))
        break;
  }

  if (entry < synchpt_priv(synchpt)->used_entries)
    /* we did not reach the end of the loop =>
     * => we probably found an already existing arc we can add to...
     */
    if (synchpt_sem_parm(synchpt)[entry].sem_num == place)
      if (((synchpt_sem_parm(synchpt)[entry].sem_op < 0) && (weight < 0)) ||
          ((synchpt_sem_parm(synchpt)[entry].sem_op > 0) && (weight > 0))) {
        /* insert the new arc... */
        synchpt_sem_parm(synchpt)[entry].sem_op += weight;
        return 0;
      }

  /* We will need to use a new entry for this arc... */
  if (synchpt_priv(synchpt)->used_entries >=
      synchpt_priv(synchpt)->total_entries)
    /* no free entries to be used... */
    return -1;

  /* Due to the method the semaphores are implemented, we must
   * place all the negative values (i.e. wait arcs, i.e. arcs from
   * a place to a transition) before all the positive arcs (i.e.
   * signal arcs, i.e. arcs from a transition to a place).
   *
   * This issue only arises if the synchpt has to simultaneously
   * signal and wait on the same semaphore. It we first signal and then
   * wait, the condition will always be valid. On the other hand, the
   * petri net requires that we first wait and only then do we signal.
   * That is why we must first have all the negative values (i.e. the
   * wait arcs) and only then the psoitive values (i.e. the signal arcs).
   */
  if (weight < 0) {
  /* If we are inserting a wait value (negative value), then we will be
   * inserting it into the first place in the array. We must therefore
   * shift all the existing entries up one place...
   */
    memmove(&synchpt_sem_parm(synchpt)[1], &synchpt_sem_parm(synchpt)[0],
            sizeof(synchpt_sem_parm(synchpt)[0]) * synchpt_priv(synchpt)->used_entries);
            /* NOTE: do not use sizeof(plc_sem_parm_t). If the type is later changed,
             *       we would have to come back and correct the above line of code !
             */
    entry = 0;
  } else {
    /* we will insert it into the first free location... */
    entry = synchpt_priv(synchpt)->used_entries;
  }

  /* ...insert the new arc. */
  synchpt_sem_parm(synchpt)[entry].sem_num = place;
  synchpt_sem_parm(synchpt)[entry].sem_op  = weight;
  synchpt_sem_parm(synchpt)[entry].sem_flg = synchpt_priv(synchpt)->sem_flg;

  synchpt_priv(synchpt)->used_entries += 1;

  return 0;
}



/*
 * Delete an arc from a synchpt <=> Delete an arc from a transition
 *
 * NOTE:
 *          wieght < 0 => arc from place to transition (synchpt)
 *          wieght = 0 => null arc from place to transition (synchpt)
 *          wieght > 0 => arc from transition (synchpt) to place
 */
int plc_synchpt_del_arc(plc_synchpt_t synchpt, int weight, plc_synchplace_t place) {
  int entry;

  check_synchpt_magic_ret(synchpt, -1);

  /* Search for the arc in question... */
  for (entry = 0; entry < synchpt_priv(synchpt)->used_entries; entry++) {
    if (synchpt_sem_parm(synchpt)[entry].sem_num == place)
      if (((synchpt_sem_parm(synchpt)[entry].sem_op < 0) && (weight < 0)) ||
          ((synchpt_sem_parm(synchpt)[entry].sem_op > 0) && (weight > 0)) ||
          ((synchpt_sem_parm(synchpt)[entry].sem_op ==0) && (weight ==0)))
        break;
  }

  if (entry >= synchpt_priv(synchpt)->used_entries)
    /* we reached the end of the loop =>
     * Arc not found!
     */
    return -1;

  if (!(((synchpt_sem_parm(synchpt)[entry].sem_op < 0) && (weight < 0)) ||
        ((synchpt_sem_parm(synchpt)[entry].sem_op > 0) && (weight > 0)) ||
        ((synchpt_sem_parm(synchpt)[entry].sem_op ==0) && (weight ==0))))
    /* Arc not found! */
    return -1;

  if (((synchpt_sem_parm(synchpt)[entry].sem_op < 0) && (weight < 0)) ||
      ((synchpt_sem_parm(synchpt)[entry].sem_op > 0) && (weight > 0))) {
    /* delete the arc... by subtracting the weight!
     * If it is a null arc, we will fall through and let the entry be removed
     * from the plc_sem_parm_t array...
     */
    synchpt_sem_parm(synchpt)[entry].sem_op -= weight;
    if (synchpt_sem_parm(synchpt)[entry].sem_op != 0)
      /* if other arcs, from this transition to the same place still remain,
       * then our job is done and we simply return...
       * ...otherwise we let it fall through to the code that will remove
       * this entry, as it is no longer required...
       */
      return 0;
  }

  /* If this entry no longer required (fell through from the above condition,
   * OR it is a null arc,
   * THEN we must remove the entry itself!!
   */
  /* shift down all remaining entries... */
  memmove(&synchpt_sem_parm(synchpt)[entry], &synchpt_sem_parm(synchpt)[entry+1],
          sizeof(synchpt_sem_parm(synchpt)[0]) *
            (synchpt_priv(synchpt)->used_entries - 1 - entry));
          /* NOTE: do not use sizeof(plc_sem_parm_t). If the type is later changed,
           *       we would have to come back and correct the above line of code !
           */

  /* decrement the used entries counter... */
  synchpt_priv(synchpt)->used_entries--;

  return 0;
}




/*
 * SYNCHRONISATION functions;
 */
int plc_synch(plc_synchpt_t synchpt)
{
  __assert_synch_fully_initialized(-1);

  check_synchpt_magic_ret(synchpt, -1);

  return plcsynchsem_synch(synch_semid_,
                       synchpt_sem_parm(synchpt),
                       synchpt_priv(synchpt)->used_entries);
}





