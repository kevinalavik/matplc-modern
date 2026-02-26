/*
 * (c) 2004 Mario de Sousa
 *
 *  NOTE: This file was based on /lib/misc/sem_util.h
 *            which in turn had been loosely based on snippets of code
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

#ifndef __PLC_SYNCHSEM_H
#define __PLC_SYNCHSEM_H



#if defined(PLC_HOSTOS_LINUX)
#define PLCSYNCHSEM_SYSV
#elif defined(PLC_HOSTOS_QNX)
//#define PLCSYNCHSEM_POSIX_SEM
#define PLCSYNCHSEM_POSIX_MUTEX
#else
#error "Don't know which version to use."
#endif


#ifdef PLCSYNCHSEM_POSIX_SEM
#define PLCSYNCHSEM_POSIX
#endif
#ifdef PLCSYNCHSEM_POSIX_MUTEX
#define PLCSYNCHSEM_POSIX
#endif


/* In linux, this constant only seems to be getting defined if we
 * #include <linux/sem.h>
 * Unfortunately, that include redefines other structures already
 * defined by <sys/sem.h>
 *
 * The value we use (32767) is declared in both the
 * <linux/sem.h> file as well as in the man pages...
 */
#if PLC_HOSTOS_ == LINUX
#ifndef SEMVMX
#define SEMVMX 32767
#endif
#endif


#ifdef PLCSYNCHSEM_POSIX
#include <semaphore.h>
#define PLCSYNCHSEM_FAILED NULL
typedef void *plcsynchsem_t ;
typedef struct {
  unsigned short sem_num;
  short sem_flg;
  short sem_op;
} plcsynchsem_parm_t;
#define IPC_NOWAIT 004000
#define PLCSYNCHSEMVMX UINT_MAX
#endif

#ifdef PLCSYNCHSEM_SYSV
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define PLCSYNCHSEMVMX SEMVMX
#define PLCSYNCHSEM_FAILED (-1)
typedef int plcsynchsem_t ;
typedef struct sembuf plcsynchsem_parm_t;
#endif



/* create a semaphore set with random key and 0666 permissions */
/* returns 0 on success, or -1 on error                */
/* init_values[] is an array with num_sem positions, containing the initial 
 * values for each semaphore in the semaphore set. NULL is also acceptable!
 */
int plcsynchsem_createrand(int *sem_key, short num_sem, int *init_values);

/* create a semaphore set with 0666 permissions */
/* returns 0 on success, or -1 on error                */
/* init_values[] is an array with num_sem positions, containing the initial 
 * values for each semaphore in the semaphore set. NULL is also acceptable!
 */
int plcsynchsem_create(int sem_key, int num_sem, int *init_values);

/* delete a semaphore set */
/* returns 0 on success, or -1 on error */
int plcsynchsem_delete(int sem_key);

/* open the semahore set with specified key 
 *
 * priority is the scheduling priority of the process/thread that will be using
 * the semaphore. This is used when several processes are waiting
 * on a semaphore, and we need to choose which will get released.
 * Current algorithm is highest priority first.
 */
/* returns the plcsynchsemaphore, or PLCSYNCHSEM_FAILED (-1) on error                */
plcsynchsem_t plcsynchsem_open(int sem_key, int priority);

/* close the semahore set with specified key */
/* returns 0 on success, or -1 on error  */
int plcsynchsem_close(plcsynchsem_t sem);

/* Synch on the semaphore set using sem_parm_t struct */
/*
 * Returns: 0 -> if synchronised succesfully
 *         -1 -> if cannot synchronise and nonblocking option is selected
 *         -2 -> error ocurred
 */
int plcsynchsem_synch(plcsynchsem_t sem, plcsynchsem_parm_t *parm, int parm_count);


#endif	/* __PLC_SYNCHSEM_H */
