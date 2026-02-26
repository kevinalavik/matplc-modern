/*
 * (c) 2004 Mario de Sousa
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

#ifndef __PLC_MUTEX_UTIL_H
#define __PLC_MUTEX_UTIL_H


#if defined(PLC_HOSTOS_LINUX)
#define PLCMUTEX_SYSV
#elif defined(PLC_HOSTOS_QNX)
//#define PLCMUTEX_POSIX_NAMEDSEM
//#define PLCMUTEX_POSIX_ANONSEM
#define PLCMUTEX_POSIX_MUTEX  /* supports priority inheritance! */
#else
#error "Don't know which version to use."
#endif




/* This file provides primitives to mutexes that may synchronise
 * threads in different processes.
 *
 * Three implementations are provided:
 *  - based on SysV named semaphores
 *  - based on POSIX named semaphores
 *  - based on POSIX anonymous semaphores
 *  - based on POSIX mutexes  (allows for priority inheritance protocol)!
 *
 *
 * Note that SysV semaphores are always named, whereas POSIX mutexes are
 * always anonymous. This makes it difficult to define an API that will
 * map nicely to every implementation.
 * The best option I could think of is to provide a named mutex API,
 * based on two specific data types. Their is an additional restriction
 * that the plcmutex_id_t variable must be located on memory
 * accessible to all processes/threads using the mutex (located in shared
 * memory, if necessary). Actually, this restriction is only used if the anonymous
 * semaphore or mutex implementation is chosen, but since the calling
 * programs should be agnostic as to which implementation is being used,
 * the above restriction should always be complied with.
 *  plcmutex_id_t -> this is a data type containing the
 *                   identification of the mutex, from which
 *                   a mutex reference is obtained.
 *  plcmutex_t -> this is a reference to the mutex, which must be passed
 *                to every function call of lock() and unlock().
 *
 * The API:
 *
 * int        plcmutex_create(plcmutex_id_t *mutex_id)
 * int        plcmutex_destroy(plcmuted_id_t *mutex_id)
 *
 * plcmutex_t plcmutex_open(plcmutex_id_t *mutex_id)
 * int        plcmutex_close(plcmutex_t mutex)
 *
 * int        plcmutex_lock(plcmutex_t mutex)
 * int        plcmutex_unlock(plcmutex_t mutex)
 * int        plcmutex_trylock(plcmutex_t mutex)
 *
 * The mutex_id, passed to plcmutex_create() and plcmutex_open()
 * must reference the exact same memory location. If these functions
 * are called from different processes, then this memory must
 * be acessable from both processes, using some shared memory mechanism.
 *
 * The SysV semaphore version, uses:
 *  typedef int plcmutex_id_t;  -> the SysV semaphore key
 *  typedef int plcmutex_t;     -> the SysV semaphore id!!
 *  plcmutex_create() will create a SysV semaphore with a
 *  random key, and store the key value in the plcmutex_id
 *  variable!
 *
 * The named POSIX semaphore version uses:
 *  typedef int   plcmutex_id_t
 *  typedef sem_t *plcmutex_t
 *  plcmutex_create() will create a named POSIX semaphore with a
 *  name that includes a random key, and store the key value in the
 *  plcmutex_id variable!
 *
 * The anonymous POSIX semaphore version uses:
 *  typedef sem_t plcmutex_id_t
 *  typedef sem_t *plcmutex_t
 *  plcmutex_create() will initialise an anonymous POSIX semaphore
 *  in the plcmutex_id variable! The plcmutex_t will be a pointer
 *  to that same variable.
 *
 * The anonymous POSIX mutex version uses:
 *  typedef pthread_mutex_t plcmutex_id_t
 *  typedef pthread_mutex_t *plcmutex_t
 *  plcmutex_create() will initialise an anonymous POSIX mutex
 *  in the plcmutex_id variable! The plcmutex_t will be a pointer
 *  to that same variable.
 *
 *
 * Expected use of the above API will be:
 *    Process 1:
 *        // create the plcmutex
 *        plcmutex_id_t mutex_id;
 *        plcmutex_t mutex;
 *
 *        plcmutex_create(&mutex_id);
 *        mutex = plcmutex_open(&mutex_id);
 *        plcmutex_lock(mutex);
 *        plcmutex_unlock(mutex);
 *        plcmutex_close(mutex);
 *        plcmutex_destroy(&mutex_id);
 *
 *    Process 1:
 *        // create the plcmutex
 *        plcmutex_id_t *mutex_id = ...
 *           // mutex_id must point to the SAME memory where mutex_id of process 1 was stored!
 *           // If under two processes, shared memory will have to be used!
 *        plcmutex_t mutex;
 *
 *        mutex = plcmutex_open(&mutex_id);
 *        plcmutex_lock(mutex);
 *        plcmutex_unlock(mutex);
 *        plcmutex_close(mutex);
 */




#ifdef PLCMUTEX_SYSV
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/sem.h>
typedef int plcmutex_id_t;  /* the SysV semaphore key */
typedef int plcmutex_t;     /* the SysV semaphore id!! */
#define PLCMUTEX_FAILED (-1)
#endif

#ifdef PLCMUTEX_POSIX_NAMEDSEM
#include <semaphore.h>
typedef int   plcmutex_id_t;
typedef sem_t *plcmutex_t;
#define PLCMUTEX_FAILED NULL
#endif

#ifdef PLCMUTEX_POSIX_ANONSEM
#include <semaphore.h>
typedef sem_t plcmutex_id_t;
typedef sem_t *plcmutex_t;
#define PLCMUTEX_FAILED NULL
#endif


#ifdef PLCMUTEX_POSIX_MUTEX
#include <pthread.h>
typedef pthread_mutex_t plcmutex_id_t;
typedef pthread_mutex_t *plcmutex_t;
#define PLCMUTEX_FAILED NULL
#endif



/* create a plcmutex with random id, and store the id in the mutex_id variable. */
/* The mutex is created unlocked! */
/* returns 0 on success, or -1 on error */
int plcmutex_create(plcmutex_id_t *mutex_id);

/* delete a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_destroy(plcmutex_id_t *mutex_id);

/* open the plcmutex with specified id */
/* NOTE: mutex_id must point to the SAME memory used in the plcmutex_create() call.
 *       If the calls are made under two processes, shared memory will have to be used
 *       to store the plcmutec_id_t variable!
 */
/* returns the plcmutex, or PLCMUTEX_FAILED on error */
plcmutex_t plcmutex_open(plcmutex_id_t *mutex_id);

/* close the plcmutex */
/* returns 0 on success, or -1 on error  */
int plcmutex_close(plcmutex_t mutex);

/* Lock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_lock(plcmutex_t mutex);

/* Unlock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_unlock(plcmutex_t mutex);

/* Try to lock a plcmutex */
/* returns 0 on success, or -1 on error, -2 if no error but the plcmutex is already locked */
int plcmutex_trylock(plcmutex_t mutex);

#endif				/* __PLC_MUTEX_UTIL_H */
