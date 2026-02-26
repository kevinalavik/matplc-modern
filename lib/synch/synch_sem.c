/*
 * (c) 2004 Mario de Sousa
 *
 *  NOTE: The SysV version portion of this file was based on /lib/misc/sem_util.c
 *            which in turn had been based on snippets of code
 *            from several other files originaly written by Jiri (if memory
 *            is not playing tricks on me...)
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
 * SysV Semaphore Emulation Routines, based on POSIX semaphores,
 * or SysV semaphores themselves!
 *
 * This file implements the routines in synch_sem.h
 *
 * These routines implement semaphore sets with similar semantics as those provided 
 * by SysV semaphore sets.
 * Three implementations are provided:
 *     - one based on POSIX semaphores (PLCSYNCHSEM_POSIX_SEM)
 *     - one based on POSIX mutexes (PLCSYNCHSEM_POSIX_MUTEX)
 *     - another based on the SysV semaphores (PLCSYNCHSEM_SYSV)
 *
 *  Choose which implemntation to use, based on the options supported
 *  by your OS, using either the PLCSYNCHSEM_POSIX or PLCSYNCHSEM_SYSV options.
 *
 * The POSIX semaphores version may not be used in a hard RT setting.
 * This is due to the fact that, even though we try to limit the possibility
 * of occurence of priority inversion with our implementation, unbounded
 * priority inversion may still occur.
 *
 * To solve the above, the version using POSIX mutexes must be used,
 * since those use priority inheritance protocol to bound the ocurrence
 * of priority inversion. Note that POSIX semaphores do not support
 * the use of priority inheritance protocols.
 */



#include <stdlib.h>
#include <time.h>     /* required for random seed generation */
#include <stdio.h>
#include <errno.h>

#include "synch_sem.h"

#ifndef PLCSYNCHSEM_POSIX_SEM
#ifndef PLCSYNCHSEM_POSIX_MUTEX
#ifndef PLCSYNCHSEM_SYSV
#error "This file must be compiled with exactly one of three available options. No option is currently set."
#endif
#endif
#endif
#ifdef PLCSYNCHSEM_POSIX_SEM
#ifdef PLCSYNCHSEM_SYSV
#error "This file must be compiled with only one of three available options. More than one are currently set."
#endif
#endif
#ifdef PLCSYNCHSEM_POSIX_MUTEX
#ifdef PLCSYNCHSEM_SYSV
#error "This file must be compiled with only one of three available options. More than one are currently set."
#endif
#endif
#ifdef PLCSYNCHSEM_POSIX_SEM
#ifdef PLCSYNCHSEM_POSIX_MUTEX
#error "This file must be compiled with only one of three available options. More than one are currently set."
#endif
#endif




#ifdef PLCSYNCHSEM_POSIX
#include <pthread.h>  /* required for pthread_getschedparam(), ... */
#include <cmm/cmm.h> /* we use some cmm shared memory blocks in the POSIX version! */
#endif



/*#define DEBUG*/



#ifdef PLCSYNCHSEM_POSIX
/* This function is currently only being used in the POSIX version.
 * Comment it out so the compiler does not give out an unused warning message
 * when compiling with the SysV version.
 */
#define ERROR error_exit(__FILE__,__LINE__)

static void error_exit(const char *file_name, int line_no) {
  fprintf(stderr, "\nInternal program error in file %s at line %d\n\n\n", file_name, line_no);
  exit(EXIT_FAILURE);
}

#endif


/**************************************/
/**************************************/
/***                                ***/
/*** The SysV  version...           ***/
/***                                ***/
/**************************************/
/**************************************/


#ifdef PLCSYNCHSEM_SYSV

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
  int val;                    /* value for SETVAL */
  struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
  unsigned short int *array;  /* array for GETALL, SETALL */
  struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif


/* returns 0 on success, or -1 on error                */
int plcsynchsem_create(int sem_key, int num_sem, int *init_values) {
  plcsynchsem_t sem;
  struct sembuf so;
  int i;

#ifdef DEBUG
printf("plcsynchsem_create(key=%d, sem_num=%d): called...\n", sem_key, num_sem);
#endif

  sem = semget(sem_key, num_sem, IPC_CREAT | IPC_EXCL | 0666);
  if (sem  < 0) {
    return -1;
  }

  /* signal the initial values, _not_ using SEM_UNDO!! */
  for (i=0; i < num_sem; i++) {
    so.sem_num = i;
    so.sem_op = init_values[i];
    so.sem_flg = 0;               /* don't undo this when we exit */
    while (semop(sem, &so, 1) < 0) {
      if (errno != EINTR) {
        plcsynchsem_delete(sem);
        return -1;;
      }
    }
  }

#ifdef DEBUG
printf("plcsynchsem_create(key=%d): sucess...\n", sem_key);
#endif
  return 0;
}


/* returns the plcsynchsemaphore, or PLCSYNCHSEM_FAILED (-1) on error                */
inline plcsynchsem_t plcsynchsem_open(int sem_key, int priority)
{return semget(sem_key, 0, 0);}



/* delete a semaphore set */
/* returns 0 on success, or -1 on error */
int plcsynchsem_delete(int sem_key) {
	/* glibc in PowerPC has a bug in semctl, resulting in a segmentation fault 
	 * if we pass a NULL in the fourth argument. Workaround requires passing 
	 * a pointer to a non-used union semun variable... 
	 */	 
union semun unused;  
plcsynchsem_t sem;
  sem = plcsynchsem_open(sem_key, 0);
  
  if (sem == PLCSYNCHSEM_FAILED)
    return -1;

  if (semctl(sem, 0, IPC_RMID, unused) < 0) {
    /* 
    if (errno == EINVAL) {
      perror("Semaphore set not found");
    } else {
      perror("Error removing semaphore set");
    }
    */
    return -1;  
  }

  return 0;
}


/* close the semahore set with specified key */
/* returns 0 on success, or -1 on error  */
int plcsynchsem_close(plcsynchsem_t sem)
{return 0;}



/* Synch on the semaphore set using sem_parm_t struct */
/*
 * Returns: 0 -> if synchronised succesfully
 *         -1 -> if cannot synchronise and nonblocking option is selected
 *         -2 -> error ocurred
 */
inline int plcsynchsem_synch(plcsynchsem_t sem, plcsynchsem_parm_t *parm, int parm_count) {
  int res;

  do {
    res = semop(sem, parm, parm_count);
  } while ((res < 0) && (errno == EINTR));

  if (res >= 0)
    return 0;

  if ((res < 0) && (errno == EAGAIN))
    return -1;

  return -2;
}

#endif  /* #ifdef PLCSYNCHSEM_SYSV */






/**************************************/
/**************************************/
/***                                ***/  
/*** The POSIX version...           ***/  
/***                                ***/  
/**************************************/
/**************************************/


#ifdef PLCSYNCHSEM_POSIX


/********************************************************/
/* some code structures only required for the POSIX version... */
/********************************************************/

#define PLCSYNCHSEM_T_GLOBALBLOCK_TYPE block_type_plcsynchsem_global_t 
#define PLCSYNCHSEM_T_LOCALBLOCK_TYPE   block_type_plcsynchsem_local_t
#define PLCSYNCHSEM_LOCALBLOCK_NAME "SYNCHSEM_LOCAL"

typedef unsigned int plcsynchsem_value_t_;
#if PLCSYNCHSEMVMX > UINT_MAX
#error The constant PLCSYNCHSEMVMX is set to the wrong value
#endif

/* A data structure that represents/contains/is the semaphore set.
 * This is the semaphore set we are emulating, that has the same
 * semantics as SysV semaphores.
 * 
 * This data structure is stored in PLCSYNCHSEM_T_GLOBALBLOCK_TYPE
 * cmm blocks...
 */
typedef struct {
#ifdef PLCSYNCHSEM_POSIX_SEM
  sem_t globalpsem;
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  pthread_mutex_t globalpmutex;
#endif;

  unsigned short num_sem;
  /* A 'de-referenced pointer' to the first process
   * on the blocked processes list. This is actually the offset
   * within the cmm_shm_ of the plcsynchsem_local_t_ block
   * of the first process on the list...
   * 0 for none!
   */
  u32 blocked_list_first; 
  /* followed by an array ... */
  /* plcsynchsem_value_t_ sem_values[num_sem]; */
} plcsynchsem_global_t_;



#define sizeof_global_t(num_sem) \
        (sizeof(plcsynchsem_global_t_) + num_sem*sizeof(plcsynchsem_value_t_))

 /* a pointer to the values array... */
#define sem_values(plcsynchsem_global_t_ptr) ((plcsynchsem_value_t_ *)                   \
        (((char *)(plcsynchsem_global_t_ptr)) + sizeof(plcsynchsem_global_t_)))

 /* the num_sem */
#define num_sem(plcsynchsem_global_t_ptr) (((plcsynchsem_global_t_ *)plcsynchsem_global_t_ptr)->num_sem)

 /* the blocked_list_first */
#define blocked_list_first(plcsynchsem_global_t_ptr) (((plcsynchsem_global_t_ *)plcsynchsem_global_t_ptr)->blocked_list_first)

#ifdef PLCSYNCHSEM_POSIX_SEM
 /* the globalpsem... */
#define globalpsem(plcsynchsem_global_t_ptr) (((plcsynchsem_global_t_ *)plcsynchsem_global_t_ptr)->globalpsem)
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
 /* the globalpmutex... */
#define globalpmutex(plcsynchsem_global_t_ptr) (((plcsynchsem_global_t_ *)plcsynchsem_global_t_ptr)->globalpmutex)
#endif;



/* Each process willing to wait/post on the semaphore set will get their own
 * following data structure. This data structure's address will be returned
 * as a reference to the emulated SysV semaphore set
 * (i.e. A plcsynchsem_t will be a pointer to this structure! )
 *
 * This structure contains a reference to the appropriate plcsynchsem_global_t_
 * data structure, as well as other per process data our emulation code
 * requires (such as process/thread priority).
 *
 * This data structure is stored in PLCSYNCHSEM_T_LOCALBLOCK_TYPE
 * cmm blocks...
 */
typedef struct {
#ifdef PLCSYNCHSEM_POSIX_SEM
  sem_t localpsem;
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  pthread_cond_t localpcondvar;
#endif;
  
  /* A pointer to the semaphore set we are waiting on... */
  plcsynchsem_global_t_ *globalsem_ptr;
  
  /* The priority of the process to which this data structure belongs */
  int priority;  
  
  /* A 'de-referenced pointer' to the next and previous process
   * on processes list. This is actually the offset
   * within the cmm_shm_ of the plcsynchsem_local_t_ block
   * of the next/previous process on the list...
   * 0 for none!
   *
   * This is actually used for two lists (blocked_list, and release_list), since the
   * process may only be in one of them at any time. 
   *  - a blocked_list (one for each global semaphore set).
   *  - release_list (local to the release_all_processes() function)
   *                       (only exists momentarily)
   */
  u32 list_next; 
  u32 list_prev; 
  
  /* A 'de-referenced pointer' to the 'plcsynchsem_parm_t *parm'
   * parameter array, containing the conditions we are waiting for to
   * become unlocked. 
   * 0 for none!
   */
  u32 blocked_parm;
  /* The number of elements in the previous blocked_parm array. */ 
  int blocked_parm_count;

} plcsynchsem_local_t_;



/***************************************************/
/* A global variable only required for the POSIX version... */
/***************************************************/

/* Get the address of the first byte of the cmm shared memory. */
/* This is required in order to de-reference pointers before
 * storing them in a shared memory block that will be accessed
 * by other processes.
 *
 * Set to NULL before being initialised...
 */
void *refptr_ = NULL; 


static inline u32 deref_local(plcsynchsem_local_t_ *ptr) {
  return ((char *)ptr) - ((char *)refptr_);
}

static inline plcsynchsem_local_t_ *reref_local(u32 offset) {
  return (plcsynchsem_local_t_ *)(((char *)refptr_) + offset);
}

static inline u32 deref_parm(plcsynchsem_parm_t *ptr) {
  return ((char *)ptr) - ((char *)refptr_);
}

static inline plcsynchsem_parm_t *reref_parm(u32 offset) {
  return (plcsynchsem_parm_t *)(((char *)refptr_) + offset);
}


/*********************/
/* A helper function */
/*********************/

#define PLCSYNCHSEM_SEMNAME "plcsynchsem-"

/* local helper function... */
static char *build_synchsemname(int sem_key) {
  char *name;
  int name_len;
 
  /* an extra 32 bytes should be more than enough for the key in decimal ascii */
  name_len = strlen(PLCSYNCHSEM_SEMNAME) + 32;
  name = (char *)malloc(name_len);
  if (name == NULL) return NULL;
    
   if (snprintf(name,  name_len, "%s%d", PLCSYNCHSEM_SEMNAME, sem_key) < 0) {
    free(name);
    return NULL;
  }
  
  return name;
}
   




/********************/
/* The real code... */
/********************/


/* returns 0 on success, or -1 on error                */
int plcsynchsem_create(int sem_key, int num_sem, int *init_values) {
  int i;
  plcsynchsem_global_t_ *globalsem;
  plcsynchsem_t sem = PLCSYNCHSEM_FAILED;
  char *semname = build_synchsemname(sem_key);

#ifdef DEBUG
printf("plcsynchsem_create(key=%d): called...\n", sem_key);
#endif
  /* If this library not yet initialised, then do so now... */
  if (NULL == refptr_)
    if (NULL == (refptr_ = cmm_refptr()))
      goto error_exit_0;

  if (NULL == semname)
    goto error_exit_0;

  /* get hold of some shared memory from the cmm... */
  globalsem = cmm_block_alloc(PLCSYNCHSEM_T_GLOBALBLOCK_TYPE,
                              semname,
                              sizeof_global_t(num_sem));
  free(semname);
  if (NULL == globalsem)
    goto error_exit_0;

  /* initialise the global semaphore... */
  blocked_list_first(globalsem) = 0;
  num_sem(globalsem) = num_sem;

#ifdef PLCSYNCHSEM_POSIX_SEM
  if (sem_init(&(globalpsem(globalsem)), 1 /*pshared*/, 1 /*init_value*/) < 0)
    goto error_exit_1;
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  {
    pthread_mutexattr_t mutex_attr;
    if (pthread_mutexattr_init(&mutex_attr) < 0) goto error_exit_1;
    if (pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT) < 0) goto error_exit_1;
    if (pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED) < 0) goto error_exit_1;
    if (pthread_mutex_init(&(globalpmutex(globalsem)), &mutex_attr) < 0) goto error_exit_1;
    pthread_mutexattr_destroy(&mutex_attr);
  }  
#endif;

  /* signal the initial values... */
  for (i=0; i < num_sem; i++)
    sem_values(globalsem)[i] = init_values[i];

#ifdef DEBUG
printf("plcsynchsem_create(): success...\n");
#endif
  /* sucess... */
  return 0;

error_exit_1:
  cmm_block_free(sem);
error_exit_0:
#ifdef DEBUG
printf("plcsynchsem_create(): error...\n");
#endif
  return -1;
}


/*  a helper function for plcsynchsem_open() */
/* returns the plcsynchsemaphore, or PLCSYNCHSEM_FAILED (-1) on error                */
static plcsynchsem_global_t_ *plcsynchsem_open_global_(int sem_key)
{
  u32 size;
  plcsynchsem_global_t_ *globalsem;
  char *semname = build_synchsemname(sem_key);
  
  if (NULL == semname)
  return PLCSYNCHSEM_FAILED;

  /* get hold of some shared memory from the cmm... */
  globalsem = cmm_block_find(PLCSYNCHSEM_T_GLOBALBLOCK_TYPE,
                              semname,
                              &size);
  free(semname);
  if (NULL == globalsem)
    return NULL;

  if (sizeof_global_t(num_sem(globalsem)) != size)
    return NULL;

  /* sucess... */
  return globalsem;
}



/* returns the plcsynchsemaphore, or PLCSYNCHSEM_FAILED (-1) on error                */
plcsynchsem_t plcsynchsem_open(int sem_key, int priority)
{
  plcsynchsem_local_t_ *sem;
  plcsynchsem_global_t_ *globalsem;
  
#ifdef DEBUG
  printf("plcsynchsem_open(): called... sem_key=%d\n", sem_key);
#endif
  
  /* If this library not yet initialised, then do so now... */
  if (NULL == refptr_)
    if (NULL == (refptr_ = cmm_refptr()))
      return PLCSYNCHSEM_FAILED;
  
  globalsem = plcsynchsem_open_global_(sem_key);
  if (NULL == globalsem)
  return PLCSYNCHSEM_FAILED;

  /* get hold of some shared memory from the cmm... */
  sem = (plcsynchsem_local_t_ *)cmm_block_alloc(PLCSYNCHSEM_T_LOCALBLOCK_TYPE,
                              PLCSYNCHSEM_LOCALBLOCK_NAME,
                              sizeof(plcsynchsem_local_t_));
  if (NULL == sem)
    goto error_exit_0;
    
  /* Determine the calling thread's priority */
#if 0
  /* Get it from the OS... */
  { struct sched_param param;
    int policy;
    if (pthread_getschedparam(pthread_self(), &policy, &param ) < 0)
      goto error_exit_1;
    sem->priority = param.sched_priority;
  }  
#endif  
  /* Get it from this function's caller... */
  sem->priority = priority;
  
#ifdef PLCSYNCHSEM_POSIX_SEM
  /* initialise the local Posix semaphore */
  if (sem_init(&(sem->localpsem), 1 /*pshared*/, 0 /*init_value*/) < 0) {
    /* perror("Error initialising local sempahore -> sem_init()"); */
    goto error_exit_1;
  }  
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  /* initialise the local Posix condition variable */
  {
    pthread_condattr_t condvar_attr;
    if (pthread_condattr_init(&condvar_attr) < 0) goto error_exit_1;
    if (pthread_condattr_setpshared(&condvar_attr, PTHREAD_PROCESS_SHARED) < 0) goto error_exit_1;
    if (pthread_cond_init(&(sem->localpcondvar), &condvar_attr) < 0) goto error_exit_1;
    pthread_condattr_destroy(&condvar_attr);
  }  
#endif;

  /* initialise remaining parameters of local semaphore struct... */
  sem->list_next = 0;
  sem->list_prev = 0;
  sem->blocked_parm = 0;
  sem->blocked_parm_count = 0;
  sem->globalsem_ptr = globalsem;
    
  /* sucess... */    
#ifdef DEBUG
  printf("plcsynchsem_open(): returning success... sem=%p\n", sem);
#endif
  return (plcsynchsem_t)sem;      

error_exit_1:
  cmm_block_free((void *)sem);
error_exit_0:
#ifdef DEBUG
  printf("plcsynchsem_open(): returning error!\n");
#endif
  return PLCSYNCHSEM_FAILED;
}



/* delete a semaphore set */
/* returns 0 on success, or -1 on error */
int plcsynchsem_delete(int sem_key)
{
  plcsynchsem_t sem = plcsynchsem_open_global_(sem_key);

  if (PLCSYNCHSEM_FAILED == sem)
  return -1;

  /* Hopefully, there aren't any other processes still holding on to a reference to this
   * plcsynchsemaphore, otherwise ugly things will happen!
   */
  return cmm_block_free(sem);
}



/* close the semahore set with specified key */
/* returns 0 on success, or -1 on error  */
int plcsynchsem_close(plcsynchsem_t sem)
{
  int res = 0;
#ifdef PLCSYNCHSEM_POSIX_SEM
  if (sem_destroy(&(((plcsynchsem_local_t_ *)sem)->localpsem)) < 0) res = -1;
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  if (pthread_cond_destroy(&(((plcsynchsem_local_t_ *)sem)->localpcondvar)) < 0) res = -1;
#endif;

  if (cmm_block_free(sem) < 0) res = -1;
  return res;
}



/* A helper function... */
/* Insert an element in the correct place of the priority ordered list.
 * head_ptr -> The address of the variable containing the 'de-referenced' 
 *             pointer to the first element currently in the list.
 * inselem  -> The address of the element we wish to insert in the list!
 */
static void insert_list(u32 *head_ptr, plcsynchsem_local_t_ *inselem) {
  plcsynchsem_local_t_ *prevelem = NULL;
  plcsynchsem_local_t_ *nextelem = NULL;

  if (0 == *head_ptr) {
    /* list is currently empty */
    *head_ptr = deref_local(inselem);
    inselem->list_prev = 0;
    inselem->list_next = 0;
    return;
  } 

  nextelem = reref_local(*head_ptr);

  if (inselem->priority > nextelem->priority) {
    /* must insert into first position... i.e. before nextelem! */
    inselem->list_prev = 0;
    inselem->list_next = *head_ptr;

    nextelem->list_prev = *head_ptr = deref_local(inselem);
    return;
  } 

  /* search for location where we must insert the new element... */  
  prevelem = nextelem;
  nextelem = NULL;
  while(prevelem->list_next != 0) {
    nextelem = reref_local(prevelem->list_next);
    if (inselem->priority > nextelem->priority)
      break; /* get out of the while loop... */    
    prevelem = nextelem;  
    nextelem = NULL;
  }  
    
  inselem->list_prev = deref_local(prevelem);
  inselem->list_next = prevelem->list_next;

  prevelem->list_next = deref_local(inselem);
  if (NULL != nextelem)
    nextelem->list_prev = prevelem->list_next; /* deref_local(inselem); */
}


/* A helper function... */
/* Remove an element from a list.
 * head_ptr -> The address of the variable containing the 'de-referenced' 
 *             pointer to the first element currently in the list.
 * remelem  -> The address of the element we wish to remove from the list!
 */
static void remove_list(u32 *head_ptr, plcsynchsem_local_t_ *remelem) {
  if (0 == remelem->list_prev) {
    /* element to be removed is first in the list... */
    *head_ptr = remelem->list_next;
  } else {
    reref_local(remelem->list_prev)->list_next = remelem->list_next;
  }

  if (0 == remelem->list_next) {
    /* element to be removed is last in the list... */
  } else {
    reref_local(remelem->list_next)->list_prev = remelem->list_prev;
  }
}


/* A helper function... */
/* Verifies if a process (referenced by localsem) may synch successfully
 * on a semaphore set (referenced by globalsem).
 *
 * If it may synch succesfully, all necessary changes to the values of the 
 * semaphore set are made, and 0 is returned.
 * If it cannot synch succesfully, then no changes are made to the values 
 * of the semaphore set, and the index (>=0) of the parameter due to which
 * the operation may not proceed is returned.
 *
 * Note that the parameters the process will check for in the semaphore set
 * must be available on
 *   localsem->blocked_parm
 *   localsem->blocked_parm_count
 */
/* returns -1 if released.
 * returns x (>= 0) if not released.
 *   x is the index of the parm due to which the operation was not performed.
 */
static int try_release_process(plcsynchsem_global_t_ *globalsem,
                               plcsynchsem_local_t_ *localsem) {

  int i;
  int undo;
  int parm_count = localsem->blocked_parm_count;
  plcsynchsem_parm_t *parm = reref_parm(localsem->blocked_parm);

  /* NOTE: To be entirely correct, we should first check all the 'signal'
   *       operations (i.e. when parm[i].sem_op <= 0), only then process
   *       the 'post' operations (i.e. when parm[i].sem_op > 0).
   *       This would guarantee that a simultaneous signal and post operation
   *       on the same semaphore, in the semaphore set, would be processed correctly.
   *       Nevertheless, the above does not seem to be necessary, nor even 
   *       perhaps the correct semantics of this function, as the SysV 
   *       semaphores in Linux do not seem to provide the above semantics either.
   *
   *       To overcome this the caller must guarantee that the parm[] doing the signal
   *       operation comes earlier in the array than the parm[] doing the 'post'
   *       operation. This is already the case with the reamining MatPLC code,
   *       so it doesn't seem necessary to worry about it here.
   */

#if 0
printf("%d:", deref_local(globalsem));  
for (i = 0; i < num_sem(globalsem); i++) 
  printf("<%d>", sem_values(globalsem)[i]);  
printf("\n");  

printf("   ");  
for (i = 0; i < parm_count; i++) 
  printf("<%d:%d>", parm[i].sem_num, parm[i].sem_op);
printf("\n");  
#endif

  /* lets do the requested operations... */
  undo = -1;
  for (i = 0; i < parm_count; i++) {
    if (parm[i].sem_op > 0) {
      /* simply increment the semaphore in question... */
      sem_values(globalsem)[parm[i].sem_num] += parm[i].sem_op;
    } else if (0 == parm[i].sem_op) {
      if (sem_values(globalsem)[parm[i].sem_num] != 0) {
        /* no success! */
        undo = i;
        break; /* get out of the for() loop! */
      }  
    } else /*if (parm[i].sem_op < 0)*/ {
      if (sem_values(globalsem)[parm[i].sem_num] < -parm[i].sem_op) {
        /* no success! */
        undo = i;
        break; /* get out of the for() loop! */
      } else {
        /* simply decrement the semaphore in question... */
        sem_values(globalsem)[parm[i].sem_num] += parm[i].sem_op;
      }
    }
  }  

  /* In case of no success, we undo everything we did previously... */
  for (i = 0; i < undo; i++) {
    sem_values(globalsem)[parm[i].sem_num] -= parm[i].sem_op;
  }  

#if 0
printf("%d:", deref_local(globalsem));  
for (i = 0; i < num_sem(globalsem); i++) 
  printf("<%d>", sem_values(globalsem)[i]);  
printf("\n");  
printf("returning undo=%d\n", undo);  
#endif

  return undo;
}




/* A helper function... */
/* Try to do the synching on each process currently blocked on a semaphore set. */
/* This is done starting off with the first process on the list (highest priority),
 * and progressing onwards untill the end of the list.
 * Once a process in the list is found that may be released, then the necessary 
 * changes are made to the values of the semaphore set, and the process is placed
 * on a list of processes to be released. We then re-start scanning all processes
 * from the begining of the list once again, as some of the processes that 
 * couldn't be released earlier may now be allowed to release because of the 
 * changes introduced by the release of the earlier process.
 * The above is repeated untill no more processes on the blocked list may
 * be released!
 *
 * Only then are the processes on the relaesed list signaled to continue their
 * processing. This list is priority ordered, and this guarantees that the processes
 * are signaled in decreasing order of their priority.
 * The above is required because, if the current process that is doing all the checking
 * and releasing of processes is of a low priority, it would get swaped out of
 * the processor as soon as it signals a middle priority process to continue. This could
 * introduce priority inversion if the lower priority process were to then signal 
 * a high priority process to continue execution, since the lower priority process
 * has been swapped out, and hence any signalling to the higher priority process
 * is delayed by the time required to execute the middle priority process!
 */
void  release_all_processes(plcsynchsem_global_t_ *globalsem) {
  plcsynchsem_local_t_ *localsem = NULL;
  u32 nextprocess;
#ifdef PLCSYNCHSEM_POSIX_SEM
  u32 signal_list = 0;
#endif
  
#ifdef DEBUG
    printf("plcsynchsem:release_all_processes(): called!\n");
#endif

  do {

    for(localsem = reref_local(nextprocess = blocked_list_first(globalsem));
        0 != nextprocess;
        localsem = reref_local(nextprocess = localsem->list_next)) {
        
      /* Check whether localsem may be released... */
      if (try_release_process(globalsem, localsem) == -1) {
        /* yes, it was released... */
        /* We now remove it from the blocked_list */
        remove_list(&(blocked_list_first(globalsem)), localsem);
        
#ifdef PLCSYNCHSEM_POSIX_SEM
        /* We may be releasing several processes, and the next process
         * we will release (in the next loop iteration) may have a higher priority
         * than the (middle) priority process we are releasing now.
         * If the process doing the releasing is of a lower priority, it will
         * get pre-empted by the (middle) priority process we are relaseing,
         * which will imply that the higher priority process we will be releasing
         * in the next loop iteration will suffer unbounded priority inversion
         * from the middle priority process.
         * To work around this, we do not release the (middle) priority process
         * just yet, but rather add it to a priority ordered list of processes
         * to be released, and then later release them by decreasing priority order!
         */
        /* ... so ... we add it to the list of processes to be signaled... */
        insert_list(&signal_list, localsem);
#endif
#ifdef PLCSYNCHSEM_POSIX_MUTEX
        /* For the mutex/condition variable based version, none of the
         * above applies, as the actual release of all the processes 
         * whose condition variables we will be signaling will occur
         * simultaneously when we release the global mutex
         * on which all the condition variables depend!
         */
        /* ... so ... no list for us, we simply signal the condition variable... */ 
        if (pthread_cond_signal(&(localsem->localpcondvar)) < 0) {
          /* YIKES!! This is a serious error, probaly due to some bug somewhere
           * else in our code! Since we can't signal the POSIX condiction variable, 
           * that process will be blocked indefinately!!!
           * What should we do??? Simply crash the system???
           * I (Mario) have no idea. For the moment, we simply crash
           * to call atention to the fact that we will probably need to fuind the bug!
           */
          ERROR;
        }  
#endif;
        
        /* ...and break out of the inner loop! */
        break;
      }
      /* if not... continue to next blocked process. */
    } /* for(nextproces...;;) */
  } while(0 != nextprocess); /* repeat until the inner loop reaches the end of the list... */
  
#ifdef PLCSYNCHSEM_POSIX_SEM
  /* Only now do we get to signal all the processes on the released list, but
   * in decreasing order of their priority!
   */
  for(localsem = reref_local(nextprocess = signal_list);
      0 != nextprocess;
      localsem = reref_local(nextprocess = localsem->list_next)) {
    if (sem_post(&(localsem->localpsem)) < 0) {
      /* YIKES!! This is a serious error, probaly due to some bug somewhere
       * else in our code! Since we can't release the
       * local POSIX semaphore, that process will be blocked indefinately!!!
       * What should we do??? Simply crash the system???
       * I (Mario) have no idea. For the moment, we simply crash
       * to call atention to the fact that we will probably need to fuind the bug!
       */
      ERROR;
    }  
  }    
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  /* we don't need to do anything! */
#endif;

#ifdef DEBUG
    printf("plcsynchsem:release_all_processes(): returning...!\n");
#endif
}



/* Synch on the semaphore set using sem_parm_t struct */
/*
 * Returns: 0 -> if synchronised succesfully
 *         -1 -> if cannot synchronise and nonblocking option is selected
 *         -2 -> error ocurred
 */
inline int plcsynchsem_synch(plcsynchsem_t sem, plcsynchsem_parm_t *parm, int parm_count) {
  plcsynchsem_local_t_ *localsem = (plcsynchsem_local_t_ *)sem;
  plcsynchsem_global_t_ *globalsem = localsem->globalsem_ptr;
  int undo;

#ifdef DEBUG
{ int i;
  printf("plcsynchsem_synch(): called... sem=%p, parm=%p, parm_count=%d", sem, parm, parm_count);
  for (i = 0; i < parm_count; i++)
    printf("<%d:%d>", parm[i].sem_num, parm[i].sem_op);  
  printf("\n");  
}
#endif

  /* Start off by checking that the *parm array is in shared memory! */
  /* NOTE: Explicitly comparing to 1 with '!= 1' also confirms that no errors 
   * ocurred, i.e. the function call did not return -1
   */
  if (cmm_checkptr((void *)parm) != 1)
    return -2;
  
  /* Now initialise the pointers to the parmaters we will be blocking for... */
  localsem->blocked_parm = deref_parm(parm);
  localsem->blocked_parm_count = parm_count;

  /* first grab the global semaphore, to get access to the shared data structure. */
#ifdef PLCSYNCHSEM_POSIX_SEM
  while (sem_wait(&(globalpsem(globalsem))) < 0)
    if (errno != EINTR)
      return -2;
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  if (pthread_mutex_lock(&(globalpmutex(globalsem))) < 0)
    return -2;
#endif;
  
  /* lets do the requested operations... */
  undo = try_release_process(globalsem, localsem);

  /* was it a success? */
  if (-1 == undo) {
    /* YES!!!
     * Release all other currently blocked process that may now be released
     * as a consequence of this process' synching with the semaphore set!
     */
    release_all_processes(globalsem);
     
    /* Now simply Release the global semaphore, and return! */
#ifdef PLCSYNCHSEM_POSIX_SEM
    if (sem_post(&(globalpsem(globalsem))) < 0) {
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
    if (pthread_mutex_unlock(&(globalpmutex(globalsem))) < 0) {
#endif;
      /* YIKES!! This is a serious error! Since we can't release the
       * global POSIX semaphore, everything is going to get blocked!!!
       * What should we do??? Simply crash the system???
       * I (Mario) have no idea. For the moment, we simply crash the system
        * to call attention to the fact that our code probably contains a bug!
       */
      ERROR;
    }  
#ifdef DEBUG
    printf("plcsynchsem_synch(): returning sucesfully without getting blocked!\n");
#endif
    return 0;  
  }

  /* No, it wasn't a success... */
  /* Should we return without blocking? */
  if (parm[undo].sem_flg && IPC_NOWAIT) {
    /* Yes, we should not block... */
    /* First unlock the global semaphore... */
#ifdef PLCSYNCHSEM_POSIX_SEM
    if (sem_post(&(globalpsem(globalsem))) < 0) {
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
    if (pthread_mutex_unlock(&(globalpmutex(globalsem))) < 0) {
#endif;
      /* Same comment as above regarding release of global semaphore... */
      ERROR;
    }  
    
    /* return the non-blocking error...*/
#ifdef DEBUG
    printf("plcsynchsem_synch(): returning due to non-blocking option!\n");
#endif
    return -1;
  }

  /* We should block! */
  /* Start off by getting ourselves on the list of processes blocked on this semaphore... */
  insert_list(&(blocked_list_first(globalsem)), localsem);
  
  /* Now simply release the global posix semaphore, and
   * wait on our local posix semaphore until some other process
   * tells us it is OK to proceed!
   */
#ifdef PLCSYNCHSEM_POSIX_SEM
  if (sem_post(&(globalpsem(globalsem))) < 0) {
    /* Same comment as above regarding release of global semaphore... */
    ERROR;
  }  
  while (sem_wait(&(localsem->localpsem)) < 0)
    if (errno != EINTR)
      return -2;
#endif;
#ifdef PLCSYNCHSEM_POSIX_MUTEX
  if (pthread_cond_wait(&(localsem->localpcondvar), &(globalpmutex(globalsem))) < 0) {
    /* Same comment as above regarding release of global semaphore... */
    ERROR;
  }  
  if (pthread_mutex_unlock(&(globalpmutex(globalsem))) < 0) {
    /* Same comment as above regarding release of global semaphore... */
    ERROR;
  }  
#endif;

#ifdef DEBUG
  printf("plcsynchsem_synch(): returning after becoming blocked and subsequently released!\n");
#endif
  return 0;  
}


#endif /* #ifdef PLCSYNCHSEM_POSIX */


#if defined(PLCSYNCHSEM_POSIX)
#define PLCSYNCHSEM_INVALIDKEY 0
#elif defined(PLCSYNCHSEM_SYSV)
#define PLCSYNCHSEM_INVALIDKEY IPC_PRIVATE
#elif 
#error Compiling option missing...
#endif

static int MAX_RAND_KEY_TRIES = 10;

/* creates semaphore with random key  */
/* returns 0 on success, or -1 on error                */
int plcsynchsem_createrand(int *sem_key, short num_sem, int *init_values) {
  int local_key, i;

#ifdef DEBUG
printf("plcsynchsem_createrand(): called...\n");
#endif

  srand(time(0));
  for (i = 0; i < MAX_RAND_KEY_TRIES; i++) {
    while ((local_key = rand()) == PLCSYNCHSEM_INVALIDKEY);
    if (plcsynchsem_create(local_key, num_sem, init_values) == 0) {
      if (sem_key != NULL) *sem_key = local_key;
      return 0;
    }
  } /* for */

#ifdef DEBUG
printf("plcsynchsem_createrand(): error...\n");
#endif
  return -1;
}
