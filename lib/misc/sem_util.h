/*
 * (c) 2000 Jiri Baum
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

#ifndef __PLC_SEM_UTIL_H
#define __PLC_SEM_UTIL_H


/* At the moment, the MatPLC does not use the semaphore utility functions,
 * these having been replaced by the mutex utility functions. Nevertheless,
 * we leave this file here just in case it comes in usefull later...
 */


#if defined(PLC_HOSTOS_LINUX)
#define SEM_SYSV
#elif defined(PLC_HOSTOS_QNX)
#define SEM_POSIX
#else
#error "Don't know which version to use."
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


#ifdef SEM_POSIX
#include <semaphore.h>
#define PLCSEM_FAILED SEM_FAILED
typedef sem_t *plcsem_t ;
#define PLCSEMVMX SEM_VALUE_MAX
#endif

#ifdef SEM_SYSV
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/sem.h>
#define PLCSEM_FAILED (-1)
typedef int plcsem_t ;
#define PLCSEMVMX SEMVMX
#endif



/* create and open a semaphore with random key and 0666 permissions */
/* returns the plcsemaphore, or PLCSEM_FAILED (-1) on error                */
plcsem_t plcsem_createrand(int *sem_key, int init_value);

/* create and open a semaphore with 0666 permissions */
/* returns the plcsemaphore, or PLCSEM_FAILED (-1) on error                */
plcsem_t plcsem_create(int sem_key, int init_value);

/* delete a semaphore */
/* returns 0 on success, or -1 on error */
int plcsem_delete(int sem_key);

/* open the semahore with specified key */
/* returns the plcsemaphore, or PLCSEM_FAILED (-1) on error                */
plcsem_t plcsem_open(int sem_key);

/* close the semahore set with specified key */
/* returns 0 on success, or -1 on error  */
int plcsem_close(plcsem_t sem);

/* Do the wait operation on a semaphore */
/* returns 0 on success, or -1 on error */
int plcsem_wait(plcsem_t sem);

/* Do the signal operation on a semaphore */
/* returns 0 on success, or -1 on error */
int plcsem_signal(plcsem_t sem);

#endif				/* __PLC_SEM_UTIL_H */
