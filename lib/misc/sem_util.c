/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
 *
 * (c) 2004 Mario de Sousa 
 *                - Added support for POSIX semaphores (the SEM_POSIX option)
 *                - Added check for EINTR on plcsem_wait/signal() on SysV version
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
 * Semaphore Utility Routines
 *
 * This file implements the routines in sem_util.h
 *
 * These routines merely make life simpler when working with semaphores
 * Two implementations are provided:
 *     - one based on POSIX semaphores (SEM_POSIX)
 *     - another based on the SysV semaphores (SEM_SYSV)
 *
 *  Choose which implemntation to use, based on the options supported
 *  by your OS, using either the SEM_POSIX or SEM_SYSV options.
 */


#include <stdlib.h>
#include <time.h>     /* required for random seed generation */
#include <stdio.h>
#include <errno.h>

#include "sem_util.h"

#ifndef SEM_POSIX
#ifndef SEM_SYSV
#error "This file must be compiled with exactly one of the two available options (SEM_POSIX or SEM_SYSV) set"
#endif
#endif

#ifdef SEM_POSIX
#ifdef SEM_SYSV
#error "This file must be compiled with only one of the two available options (SEM_POSIX or SEM_SYSV) set"
#endif
#endif



#ifdef SEM_SYSV


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


/* returns the plcsemaphore, or PLCSEM_FAILED (-1) on error                */
inline plcsem_t plcsem_open(int sem_key)
{return semget(sem_key, 0, 0);}

/* returns 0 on success, or -1 on error  */
inline int plcsem_close(plcsem_t sem)
{return 0;}


/* returns 0 on success, or -1 on error  */
inline int plcsem_delete(int sem_key) {
	/* glibc in PowerPC has a bug in semctl, resulting in a segmentation fault 
	 * if we pass a NULL in the fourth argument. Workaround requires passing 
	 * a pointer to a non-used union semun variable... 
	 */	 
  union semun unused;  
  plcsem_t sem = plcsem_open(sem_key);

  if (sem == PLCSEM_FAILED)
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



/* returns the plcsemaphore, or PLCSEM_FAILED (-1) on error                */
inline plcsem_t plcsem_create(int sem_key, int init_value) {
  plcsem_t sem;
  struct sembuf so;

  sem = semget(sem_key, 1 /*semnum*/, IPC_CREAT | IPC_EXCL | 0666);
  if (sem == PLCSEM_FAILED)
    return sem;

  /* signal the initial value, _not_ using SEM_UNDO!! */
  so.sem_num = 0;
  so.sem_op = init_value;
  so.sem_flg = 0;               /* don't undo this when we exit */
  while (semop(sem, &so, 1) < 0)
    if (errno != EINTR) {
      plcsem_delete(sem);
      return PLCSEM_FAILED;
    }

  return sem;
}



/* Do the wait operation on a semaphore */
/* returns 0 on success, or -1 on error  */
int plcsem_wait(plcsem_t sem) {
  struct sembuf so;
  so.sem_num = 0;
  so.sem_op = -1;
  so.sem_flg = SEM_UNDO;
  while (semop(sem, &so, 1) < 0)
    if (errno != EINTR)
      return -1;
  return 0;      
}


/* Do the signal operation on a single semaphore */
/* returns 0 on success, or -1 on error  */
int plcsem_signal(plcsem_t sem) {
  struct sembuf so;
  so.sem_num = 0;
  so.sem_op = 1;
  so.sem_flg = SEM_UNDO;
  while (semop(sem, &so, 1) < 0)
    if (errno != EINTR)
      return -1;
  return 0;      
}

#endif  /* #ifdef SEM_SYSV */

#ifdef SEM_POSIX

#include <fcntl.h>           /* required for O_CREAT, O_RDWR, etc... */

#define SEM_SEMNAME "/MatPLC-sem-"

/* local helper function... */
static char *build_semname(int sem_key) {
  char *semname;
  int semname_len;
 
  /* an extra 32 bytes should be more than enough for the key in decimal ascii */
  semname_len = strlen(SEM_SEMNAME) + 32;
  semname = (char *)malloc(semname_len);
  if (semname == NULL) return NULL;
    
   if (snprintf(semname,  semname_len, "%s%d", SEM_SEMNAME, sem_key) < 0) {
    free(semname);
    return NULL;
  }
  
  return semname;
}
   

/* returns semaphore on success, or PLCSEM_FAILED on error  */
inline plcsem_t plcsem_create(int sem_key, int init_value) {
  plcsem_t sem;
  char *semname = build_semname(sem_key);
 
  if (semname == NULL) return PLCSEM_FAILED;
   
  sem = sem_open(semname, O_CREAT | O_EXCL, 0666, init_value);
  free(semname);
  return sem;
}


/* returns 0 on success, or -1 on error  */
inline int plcsem_delete(int sem_key) {
  int res;
  char *semname = build_semname(sem_key);
 
  if (semname == NULL) return -1;
    
  res = sem_unlink(semname);
  free(semname);
  return res;
}


/* returns pointer to the semaphore, or -1 on error                */
inline plcsem_t plcsem_open(int sem_key) {
  plcsem_t sem;
  char *semname = build_semname(sem_key);
 
  if (semname == NULL) return PLCSEM_FAILED;
    
  sem = sem_open(semname, 0);
  free(semname);
  return sem;
}


/* close the semahore set with specified key */
int plcsem_close(plcsem_t sem) {
  if (sem == PLCSEM_FAILED)
    return -1;
  return sem_close(sem);
}
 


/* Do the wait operation on a single semaphore */
/* returns 0 on success, -1 on error... */
int plcsem_wait(plcsem_t sem) {
  while (sem_wait(sem) < 0) 
    if (errno != EINTR)
      return -1;
      
  return 0;    
}


/* Do the signal operation on a single semaphore */
int plcsem_signal(plcsem_t sem)
{return sem_post(sem);}

#endif /* #ifdef SEM_POSIX */


#ifdef SEM_POSIX
#define PLCSEM_INVALIDKEY 0
#endif
#ifdef SEM_SYSV
#define PLCSEM_INVALIDKEY IPC_PRIVATE
#endif

static int MAX_RAND_KEY_TRIES = 10;

/* creates semaphore with random key                                           */
/* returns the plcsemaphore, or PLCSEM_FAILED (-1) on error                */
plcsem_t plcsem_createrand(int *sem_key, int init_value)
{
  int local_key, i;
  plcsem_t sem;

  srand(time(0));
  for (i = 0; i < MAX_RAND_KEY_TRIES; i++) {
    while ((local_key = rand()) == PLCSEM_INVALIDKEY);
    if ((sem = plcsem_create(local_key, init_value)) != PLCSEM_FAILED) {
      if (sem_key != NULL) *sem_key = local_key;
      return sem;
    }
  } /* for */

  return PLCSEM_FAILED;
}
