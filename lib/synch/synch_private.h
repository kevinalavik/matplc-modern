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


#ifndef __SYNCH_PRIVATE_H
#define __SYNCH_PRIVATE_H


/* This file contains the private 'interface' of the synch   */
/* sub-module, i.e. it contains the variables and type       */
/* definitions that are used internally by the synch         */
/* sub-module, and should not be visible to outside users.   */


#include <misc/mutex_util.h>   /* required for PLCSYNCHSEMVMX */
#include <synch/synch.h>     /* required for plc_transition_swvalue_t */




/*
 *   T h e    S Y N C H    L I B R A R Y
 *  =====================================
 *
 *  This library uses the following entities/data types:
 *
 *    Synch Places:
 *    ------------
 *    These are implemented as semaphores in the synch semaphore set.
 *
 *    plc_synchplace_t : a handle to a place in the synchronisation petri
 *                       net. This is merely the number of the sempahore
 *                       in the synch sempahore set!
 *
 *    Transitions:
 *    -----------
 *    These are data structures stored in the cmm. All the
 *    MatPLC modules access the same memory location when referring to
 *    the same transition.
 *
 *    plc_transition_t : A pointer to the location of the
 *                       struct plc_transition_priv_t in the cmm.
 *
 *    Synchpt:
 *    -------
 *    These are data structures stored in the heap memory of each
 *    MatPLC module. The required memory is reserved with malloc(),
 *    or is obtained from the cmm (see Note below...).
 *    Synchronisation (plc_synch() ) is not done directly on a transition, but
 *    rather on a synchpt. Synchpts are created from based transition
 *    (plc_synchpt_new() ), but may be edited by adding a removing arcs.
 *    Only the module that instantiates the synchpt will see the changes
 *    it introduces in the synchpt.
 *
 *    NOTE: When the underlying OS supports SysV semaphores
 *          (i.e. if the code is compiled with PLCSYNCHSEM_SYSV defined)
 *          then these structures are placed in heap memory obtained via
 *          the malloc() call. 
 *          When the underlying OS does not support SysV semaphores  
 *          (i.e. if the code is compiled with PLCSYNCHSEM_POSIX defined)
 *          these are emulated using POSIX semaphores. This emulation also
 *          requires that the synchpts be accessible to all MatPLC modules,
 *          This means that in the above case the synchpt data structures 
 *          are placed in cmm shared memory, allocated with cmm_block_alloc().
 *
 *    plc_synchpt_t : A pointer to the location of the
 *                    struct plc_synchpt_priv_t in the local heap.
 *
 *
 *
 *
 *  This library also uses a single struct in the cmm to store its
 *  static MatPLC wide global variables.
 *  This is the synch_t, that keeps the ids of the sempahores
 *  used for the synchronisation petri net, and place allocation.
 */



/************************************************************/
/*****************                          *****************/
/*****************   Global (to the synch)  *****************/
/*****************    Variables and Type    *****************/
/***************** Definitions/Declarations *****************/
/*****************                          *****************/
/************************************************************/

/*************/
/*  Synchpt  */
/*************/

/* layout of a synch point handle (plc_synchpt_priv_t) */
/* Note: stored in the module's heap memory - malloc() */
typedef struct {
    u16 magic;

      /* The value that should be used to initialise the
       * sem_flg entry in each sem_parm_t
       *
       * This will allow us to have blocking and non-blocking
       * synchpts. This value will be set acording
       * to the wishes of the caller of plc_synchpt_add()
       */
    short sem_flg;

    u16 total_entries;
    u16 used_entries;
    /* The above values will be followed by an array of sem_parm_t
     * This array is passed directly to the plc_sem_synch() function
     * when synching on this synchpt is required.
     */
    /*
    sem_parm_t sem_parm[total_entries];
    */
} plc_synchpt_priv_t;

/* NOTE: should be different to 0 */
#define SYNCHPT_MAGIC 0xABBA   /* Waterloo... */

/* type cast to get access to the private members of a synchpt */
#define synchpt_priv(synchpt) ((plc_synchpt_priv_t *)(synchpt))

#define synchpt_sem_parm(synchpt)                     \
          ((plcsynchsem_parm_t *)(((char *)(synchpt)) +   \
           sizeof(plc_synchpt_priv_t)))

#define check_synchpt_magic_ret(p,val) {         \
  if (synchpt_priv(p)->magic != SYNCHPT_MAGIC)   \
    return (val);                                \
}

#define check_synchpt_magic(p) {                 \
  if (synchpt_priv(p)->magic != SYNCHPT_MAGIC)   \
    return;                                      \
}


/****************/
/*  Transition  */
/****************/

/* layout of info stored in the synch_block for a synch point */
typedef struct {
  u16  magic;
    /* number of plc_transition_swvalue_t in each of following arrays */
  i16  place_count;

  /* memory for the following arrays is malloced and follows imediately
   * after this strcut...
   * Note: plc_transition_swvalue_t is defined in synch.h
   *
   * The wait_values[] array stores the weight of all the arcs
   * arriving at the transition. If no arc is present a -1 is stored.
   * A 0 indicates a null arc.
   * The signal_values[] array stores the weight of all the arcs
   * departing from the transition. If no arc is present a -1 is stored.
   * A 0 is not allowed, as it does not make sense to have a null arc
   * departing from the transition.
   */
/*
  plc_transition_swvalue_t wait_values[place_count];
  plc_transition_swvalue_t signal_values[place_count];
*/
} plc_transition_priv_t;

#define TRANSITION_MAGIC 0xACDC  /* Highway to Hell... Yikes! I'm starting to show my age! */

#define transition_priv(transition) ((plc_transition_priv_t *)(transition))

#define sizeof_transition_t(place_count)                              \
          (sizeof(plc_transition_priv_t) +                            \
           (2 * place_count * sizeof(plc_transition_swvalue_t)))


#define transition_wait_values(transition)                            \
          ((plc_transition_swvalue_t *)(((char *)(transition)) +      \
           sizeof(plc_transition_priv_t)))

#define transition_signal_values(transition)                          \
          ((plc_transition_swvalue_t *)(((char *)(transition)) +      \
           sizeof(plc_transition_priv_t) +                            \
           (transition_priv(transition)->place_count) *               \
           sizeof(plc_transition_swvalue_t)))

#define check_transition_magic_ret(p,val) {          \
  if (transition_priv(p)->magic != TRANSITION_MAGIC) \
    return (val);                                    \
}

#define check_transition_magic(p) {                  \
  if (transition_priv(p)->magic != TRANSITION_MAGIC) \
    return;                                          \
}


/****************/
/*  SynchPlace  */
/****************/

typedef plc_synchplace_t plc_synchplace_priv_t;


/*************/
/*  synch_t  */
/*************/

typedef struct {
  u32 magic;              /* magic value */
  int synch_semkey;       /* the sem key of the sem set used by synchpts */
  plcmutex_id_t place_alloc_mutexid; /* the ID of the mutex used for place allocation */

  int place_count;  /* the total number of places the MatPLC has */

  /* This structure is followed by an array of PLACES_IN_USE_ARRAY_TYPE
   * Each entry of this array is used to store whether the corresponding
   * place is currently being used (has been allocated by
   * plc_synchplace_add()), or is still free to be allocated later.
   * Places in use will have the entry set to 1, while places that are
   * free will have the entry set to 0.
   *
   * Access to this array is protected by the place_alloc_semkey semaphore...!!
   *
   * The size of the array is
   * array_size = place_count;
   */
#define PLACES_IN_USE_ARRAY_TYPE u8
  /*
  PLACES_IN_USE_ARRAY_TYPE places_in_use[array_size];
  */
} synch_t;

#define SYNCH_MAGIC 0x887634AA

#define sizeof_synch_t(place_count) \
        (sizeof(synch_t) + place_count*sizeof(PLACES_IN_USE_ARRAY_TYPE))

 /* a pointer to the place_use_array... */
#define places_in_use(synch_t_ptr) ((PLACES_IN_USE_ARRAY_TYPE *)                   \
                                      (((char *)(synch_t_ptr)) + sizeof(synch_t)))


#define check_synch_magic_ret(p,val) {  \
  if ((p)->magic != SYNCH_MAGIC)        \
    return (val);                       \
}

#define check_synch_magic(p) {    \
  if ((p)->magic != SYNCH_MAGIC)  \
    return;                       \
}



/**********/
/* status */
/**********/

/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

/* the current status of the synch library */
extern status_t synch_status;

static const status_t synch_fully_initialized = {0x0001};



/****************************************************************/
/*****************                               ****************/
/*****************  Global (to synch) Functions  ****************/
/*****************                               ****************/
/****************************************************************/

/* function to parse the config file for SYNCH related values */
int synch_parse_conf(int *sem_key,
                     plc_synchpt_t *scanbeg_synchpt,
                     plc_synchpt_t *scanend_synchpt);

/* This also used to take an "int use_defaults", but it was always 1, so I
 * cut it out. Put it back in if you need it. --Jiri 25.10.00 */



/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* The block type used to store plc_transition_priv_t struct */
#define SYNCH_TRANSITION_BLOCK_TYPE block_type_synch_transition

/* The block type used to store synchpt_t structs, 
 * (only used if SysV semaphore emulation is compiled in with PLCSYNCHSEM_POSIX)
  */
#define SYNCH_SYNCHPT_BLOCK_TYPE block_type_synch_synchpt

/* The block name used to store synchpt_t structs, 
 * (only used if SysV semaphore emulation is compiled in with PLCSYNCHSEM_POSIX)
  */
#define SYNCH_SYNCHPT_BLOCK_NAME "synchpt"

/* The block type used to store synch_t struct */
#define SYNCH_T_BLOCK_TYPE block_type_synch_setup

/* The block name used to store synch_t struct */
#define SYNCH_T_BLOCK_NAME "synch_t"

/* Default semaphore key */
/*  Use 0 for random key */
#define DEF_SYNCH_SEM_KEY 0

/* Default number of semaphores for semaphore set */
/* Must be > 0 */
#define DEF_SYNCH_NUM_SEM 16

/* The suffixes appended to the module names so as to name the synchpts
 * used by synch_scan_beg() and synch_scan_end()
 */
#define SYNCH_TRANSITION_BEG_SUFFIX ".beg"
#define SYNCH_TRANSITION_END_SUFFIX ".end"

/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/

/* name of null synchpt, that does not synch to anything */
#define SYNCH_NULL_TRANSITION_NAME "NULL"

/* sem_key to use for synch semaphore set */
#define SYNCH_SK_NAME   "synchsem_key"  /* name to search for in conf file    */
#define SYNCH_SK_SECTION "PLC"          /* section to search for in conf file */
#define SYNCH_SK_MIN     0              /* minimum valid value for parameter  */
#define SYNCH_SK_MAX     INT_MAX        /* maximum valid value for parameter  */

/* synchpt used by synch_scan_beg() */
#define SYNCH_SCANBEG_NAME   "scan_beg" /* name to search for in conf file    */
#define SYNCH_SCANBEG_SECTION NULL      /* section to search for in conf file */

/* synchpt used by synch_scan_end() */
#define SYNCH_SCANEND_NAME   "scan_end" /* name to search for in conf file    */
#define SYNCH_SCANEND_SECTION NULL      /* section to search for in conf file */


/***********************************************/
/* table names, etc.. in the config file       */
/***********************************************/
#define SYNCH_PLACES_TABLE_SECTION "PLC"
#define SYNCH_PLACES_TABLE_NAME "place"
#define SYNCH_PLACES_MIN_TOKEN 0
#define SYNCH_PLACES_MAX_TOKEN PLCSYNCHSEMVMX /* maximum value for plcsynchsemval */
#define SYNCH_PLACES_DEF_TOKEN 0

#define SYNCH_TRANS_TABLE_SECTION "PLC"
#define SYNCH_TRANS_TABLE_NAME "transition"

#define SYNCH_ARCS_TABLE_SECTION "PLC"
#define SYNCH_ARCS_TABLE_NAME "arc"
#define SYNCH_ARCS_DEF_WEIGHT 1
#define SYNCH_ARCS_MIN_WEIGHT 0
#define SYNCH_ARCS_MAX_WEIGHT PLCSYNCHSEMVMX

#define SYNCH_SYNCHPT_TABLE_SECTION "PLC"
#define SYNCH_SYNCHPT_TABLE_NAME "synchpt"
#define SYNCH_SYNCHPT_TRANS_TYPE "transition"
#define SYNCH_SYNCHPT_TRANSBEG_TYPE "transition.beg"
#define SYNCH_SYNCHPT_TRANSEND_TYPE "transition.end"

#define SYNCH_SIMPLE_TABLE_SECTION "PLC"
#define SYNCH_SIMPLE_TABLE_NAME    "synch"

#define SYNCH_SIMPLESTART_TABLE_SECTION "PLC"
#define SYNCH_SIMPLESTART_TABLE_NAME    "synch_start"


#endif /* __SYNCH_PRIVATE_H */
