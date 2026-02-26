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


/*
 * Mutex Utility Routines
 *
 * This file implements the routines in mutex_util.h
 *
 * These routines provide mutex semantics, using several possible different
 * OS services (semaphores, mutexes, etc...)
 * Three implementations are provided:
 *     - one based on named POSIX semaphores (PLCMUTEX_POSIX_NAMEDSEM)
 *     - one based on anonymous POSIX semaphores (PLCMUTEX_POSIX_ANONSEM)
 *          (not yet implemented...)
 *     - another based on the SysV semaphores (PLCMUTEX_SYSV)
 *     - another based on POSIX mutexes (PLCMUTEX_POSIX_MUTEX)
 *          (this version supports priority inheritance protocol! )
 *
 *  Choose which implemntation to use, based on the options supported
 *  by your OS...
 */


#include <stdlib.h>
#include <time.h>     /* required for random seed generation */
#include <stdio.h>
#include <errno.h>

#include "mutex_util.h"

#ifndef PLCMUTEX_SYSV
#ifndef PLCMUTEX_POSIX_MUTEX
#ifndef PLCMUTEX_POSIX_NAMEDSEM
#ifndef PLCMUTEX_POSIX_ANONSEM
#error "This file must be compiled with exactly one of four available options set."
#endif
#endif
#endif
#endif


#ifdef PLCMUTEX_POSIX_ANONSEM
#error "Option not yet implemented."
#endif



#define PLCMUTEX_MAX_RAND_KEY_TRIES 10


#ifdef PLCMUTEX_SYSV


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


/* open the plcmutex with specified id */
/* NOTE: mutex_id must point to the SAME memory used in the plcmutex_create() call.
 *       If the calls are made under two processes, shared memory will have to be used
 *       to store the plcmutec_id_t variable!
 */
/* returns the plcmutex, or PLCMUTEX_FAILED on error */
plcmutex_t plcmutex_open(plcmutex_id_t *mutex_id)
{return semget(*mutex_id, 0, 0);}

/* close the plcmutex */
/* returns 0 on success, or -1 on error  */
int plcmutex_close(plcmutex_t mutex)
{return 0;}


/* delete a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_destroy(plcmutex_id_t *mutex_id) {
	/* glibc in PowerPC has a bug in semctl, resulting in a segmentation fault 
	 * if we pass a NULL in the fourth argument. Workaround requires passing 
	 * a pointer to a non-used union semun variable... 
	 */	 
  union semun unused;  
  plcmutex_t sem = plcmutex_open(mutex_id);

  if (sem == PLCMUTEX_FAILED)
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



/* local helper function... */
static int plcmutex_create_(plcmutex_id_t *mutex_id) {
  struct sembuf so;
  int sem;

  if ((sem = semget(*mutex_id, 1 /*sem count*/, IPC_CREAT | IPC_EXCL | 0666)) < 0)
    return -1;

  /* signal the initial value, _not_ using SEM_UNDO!! */
  so.sem_num = 0;
  so.sem_op = 1; /* intial value */
  so.sem_flg = 0;               /* don't undo this when we exit */
  while (semop(sem, &so, 1) < 0)
    if (errno != EINTR) {
      plcmutex_destroy(mutex_id);
      return -1;
    }

  return 0;
}


/* create a plcmutex with random id, and store the id in the mutex_id variable. */
/* The mutex is created unlocked! */
/* returns 0 on success, or -1 on error */
int plcmutex_create(plcmutex_id_t *mutex_id) {
  int i;
  srand(time(0));
  for (i = 0; i < PLCMUTEX_MAX_RAND_KEY_TRIES; i++) {
    while ((*mutex_id = rand()) == IPC_PRIVATE);
    if (plcmutex_create_(mutex_id) == 0)
      return 0;
  } /* for */

  return -1;
}



/* Lock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_lock(plcmutex_t mutex) {
  struct sembuf so;
  so.sem_num = 0;
  so.sem_op = -1;
  so.sem_flg = SEM_UNDO;
  while (semop(mutex, &so, 1) < 0)
    if (errno != EINTR)
      return -1;
  return 0;
}


/* Unlock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_unlock(plcmutex_t mutex) {
  struct sembuf so;
  so.sem_num = 0;
  so.sem_op = 1;
  so.sem_flg = 0;
  while (semop(mutex, &so, 1) < 0)
    if (errno != EINTR)
      return -1;
  return 0;
}

/* Try to lock a plcmutex */
/* returns 0 on success, or -1 on error, -2 if no error but the plcmutex is already locked */
int plcmutex_trylock(plcmutex_t mutex) {
  struct sembuf so;
  so.sem_num = 0;
  so.sem_op = -1;
  so.sem_flg = IPC_NOWAIT;
  while (semop(mutex, &so, 1) < 0) {
    if (errno == EAGAIN) return -2;
    if (errno != EINTR)  return -1;
  }
  return 0;
}

#endif  /* #ifdef PLCMUTEX_SYSV */




#ifdef PLCMUTEX_POSIX_NAMEDSEM

#include <fcntl.h>           /* required for O_CREAT, O_RDWR, etc... */

#define PLCMUTEX_SEMNAME "/MatPLC-sem-"

/* local helper function... */
static char *build_semname(int sem_key) {
  char *semname;
  int semname_len;

  /* an extra 32 bytes should be more than enough for the key in decimal ascii */
  semname_len = strlen(PLCMUTEX_SEMNAME) + 32;
  semname = (char *)malloc(semname_len);
  if (semname == NULL) return NULL;

   if (snprintf(semname,  semname_len, "%s%d", PLCMUTEX_SEMNAME, sem_key) < 0) {
    free(semname);
    return NULL;
  }

  return semname;
}




/* local helper function... */
static int plcmutex_create_(plcmutex_id_t *mutex_id) {
  plcmutex_t sem;
  char *semname = build_semname(*mutex_id);

  if (semname == NULL) return -1;

  sem = sem_open(semname, O_CREAT | O_EXCL, 0666, 1 /* init_value */);
  free(semname);
  if (sem == SEM_FAILED)
    return -1;
  sem_close(sem);
  return 0;
}


/* create a plcmutex with random id, and store the id in the mutex_id variable. */
/* The mutex is created unlocked! */
/* returns 0 on success, or -1 on error */
int plcmutex_create(plcmutex_id_t *mutex_id) {
  int i;

  srand(time(0));
  for (i = 0; i < PLCMUTEX_MAX_RAND_KEY_TRIES; i++) {
    *mutex_id = rand();
    if (plcmutex_create_(mutex_id) >= 0)
      return 0;
  } /* for */

  return -1;
}


/* delete a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_destroy(plcmutex_id_t *mutex_id) {
  int res;
  char *semname = build_semname(*mutex_id);

  if (semname == NULL) return -1;

  res = sem_unlink(semname);
  free(semname);
  return res;
}


/* open the plcmutex with specified id */
/* NOTE: mutex_id must point to the SAME memory used in the plcmutex_create() call.
 *       If the calls are made under two processes, shared memory will have to be used
 *       to store the plcmutec_id_t variable!
 */
/* returns the plcmutex, or PLCMUTEX_FAILED on error */
plcmutex_t plcmutex_open(plcmutex_id_t *mutex_id) {
  plcmutex_t sem;
  char *semname = build_semname(*mutex_id);

  if (semname == NULL) return PLCMUTEX_FAILED;

  sem = sem_open(semname, 0);
  free(semname);
  return sem;
}


/* close the plcmutex */
/* returns 0 on success, or -1 on error  */
int plcmutex_close(plcmutex_t mutex) {
  if (mutex == PLCMUTEX_FAILED)
    return -1;
  return sem_close(mutex);
}



/* Lock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_lock(plcmutex_t mutex) {
  while (sem_wait(mutex) < 0)
    if (errno != EINTR)
      return -1;

  return 0;
}

/* Unlock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_unlock(plcmutex_t mutex)
{return sem_post(mutex);}

/* Try to lock a plcmutex */
/* returns 0 on success, or -1 on error, -2 if no error but the plcmutex is already locked */
int plcmutex_trylock(plcmutex_t mutex) {
  while (sem_trywait(mutex) < 0) {
    if (errno == EAGAIN) return -2;
    if (errno != EINTR)  return -1;
  }

  return 0;
}



#endif /* #ifdef PLCMUTEX_POSIX_NAMEDSEM */





#ifdef PLCMUTEX_POSIX_MUTEX

/* create a plcmutex with random id, and store the id in the mutex_id variable. */
/* The mutex is created unlocked! */
/* returns 0 on success, or -1 on error */
int plcmutex_create(plcmutex_id_t *mutex_id) {
  pthread_mutexattr_t mutexattr;

  if (pthread_mutexattr_init(&mutexattr) != 0)
    goto error_exit_0;
  if (pthread_mutexattr_setprotocol(&mutexattr, PTHREAD_PRIO_INHERIT) != 0)
    goto error_exit_1;
  if (pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) != 0)
    goto error_exit_1;

  if (pthread_mutex_init(mutex_id, &mutexattr) != 0)
    goto error_exit_1;

  pthread_mutexattr_destroy(&mutexattr);
  return 0;

error_exit_1:
  pthread_mutexattr_destroy(&mutexattr);
error_exit_0:
  return -1;
}


/* delete a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_destroy(plcmutex_id_t *mutex_id) {
  if (pthread_mutex_destroy(mutex_id) != 0)
    return -1;
  return 0;
}


/* open the plcmutex with specified id */
/* NOTE: mutex_id must point to the SAME memory used in the plcmutex_create() call.
 *       If the calls are made under two processes, shared memory will have to be used
 *       to store the plcmutec_id_t variable!
 */
/* returns the plcmutex, or PLCMUTEX_FAILED on error */
plcmutex_t plcmutex_open(plcmutex_id_t *mutex_id) {
  /* We don't really need to do anything...
   * ...but we nevertheless check whether the mutex exists to
   * catch any errors early on.
   */
   /* NOTE: unfortunately, QNX seems to have a bug as it always
    * returns an error, saying the mutex does not exist (EINVAL), even if it does.
    * So, for now, we comment it out...
    */
#if 0    
  int priorityceil;
  if (pthread_mutex_getprioceiling(mutex_id, &priorityceil) != 0) {
    return PLCMUTEX_FAILED;
  }  
#endif  
  return mutex_id;
}


/* close the plcmutex */
/* returns 0 on success, or -1 on error  */
int plcmutex_close(plcmutex_t mutex)
{return 0;}



/* Lock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_lock(plcmutex_t mutex) {
  if (pthread_mutex_lock(mutex) != 0)
    return -1;
  return 0;
}

/* Unlock a plcmutex */
/* returns 0 on success, or -1 on error */
int plcmutex_unlock(plcmutex_t mutex) {
  if (pthread_mutex_unlock(mutex) != 0)
    return -1;
  return 0;
}

/* Try to lock a plcmutex */
/* returns 0 on success, or -1 on error, -2 if no error but the plcmutex is already locked */
int plcmutex_trylock(plcmutex_t mutex) {
  switch (pthread_mutex_trylock(mutex)) {
    case 0:     return 0;
    case EBUSY: return -2;
    default:    return -1;
  }
  /* humour the compiler... */
  return 0;
}



#endif /* #ifdef PLCMUTEX_POSIX_MUTEX */
