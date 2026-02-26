/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
 *
 * (c) 2004 Mario de Sousa 
 *                - Added support for memory mapped files (the SHMEM_MF option)
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
 * Shared Memory Utility Routines
 *
 * This file implements the routines in shmem_util.h
 *
 * These routines merely make life simpler when working with
 * shared memory
 *
 * Two implementations are provided:
 *     - one based on the POSIX MF (Mapped Files) optional functions
 *     - another based on the SysV SHM (Shared Memory Objects) optional functions
 *
 *  Choose which implementation to use, based on the options supported
 *  by your POSIX OS, using either the SHMEM_MF or SHMEM_SHM options.
 */

#if defined(PLC_HOSTOS_LINUX)
#define SHMEM_SHM
#elif defined(PLC_HOSTOS_QNX)
#define SHMEM_MF
#else
#error "Don't know which version to use."
#endif



#ifndef SHMEM_SHM
#ifndef SHMEM_MF
#error "This file must be compiled with exactly one of the two available options (SHMEM_MF or SHMEM_SHM) set"
#endif
#endif

#ifdef SHMEM_SHM
#ifdef SHMEM_MF
#error "This file must be compiled with only one of the two available options (SHMEM_MF or SHMEM_SHM) set"
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>               /* random seed generator */
#include <errno.h>
#include <limits.h>

#ifdef SHMEM_MF
#include <fcntl.h>           /* required for O_RDWR, etc... */
#include <sys/mman.h>   /* required for shm_open(), shm_unlink(), mmap() */
#include <unistd.h>         /* required for ftruncate() */
#define SHMEM_FILENAME "/MatPLC-shmem-"
#define SHMEM_INVALIDKEY 0

/* local helper function... */
static char *build_filename(int shmkey) {
  char *filename;
 int filename_len;
 
  /* an extra 32 bytes should be more than enough for the key in decimal ascii */
  filename_len = strlen(SHMEM_FILENAME) + 32;
  filename = (char *)malloc(filename_len);
  if (filename == NULL) return NULL;
    
   if (snprintf(filename,  filename_len, "%s%d", SHMEM_FILENAME, shmkey) < 0) {
    free(filename); 
    return NULL;
  }
  
  return filename;
}
   

#endif
#ifdef SHMEM_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>   /* is this necessary ??? */
#define SHMEM_INVALIDKEY IPC_PRIVATE
#endif


#include "shmem_util.h"

static int shmem_debug = 0;


static int MAX_RAND_KEY_TRIES = 10;


int detach_shmem(void *mem_ptr, unsigned int mem_size)
#ifdef SHMEM_SHM
{ return shmdt (mem_ptr); }
#endif
#ifdef SHMEM_MF
{
  int res;
  
  if (mem_ptr == NULL) return -1;
  res = munmap(mem_ptr, mem_size);
  /* if not suported by current system... */
  if (res == ENOSYS) return 0;
  if (res < 0) return -1;
  return 0;
}  
#endif


/* shmbytes: size in bytes of the shared memory to create... */
void * create_shmem_rand(int *shmkey, unsigned int shmbytes)
{
  int local_shm_key, i; void *res;

  srand(time(0));
  for (i = 0; i < MAX_RAND_KEY_TRIES; i++) {
    while ((local_shm_key = rand()) == SHMEM_INVALIDKEY);
    if ((res = create_shmem(local_shm_key, shmbytes))) {
      if (shmkey != NULL) *shmkey = local_shm_key;
      return res;
    }
  } /* for */

 return NULL;
}


/* shmbytes: size in bytes of the shared memory to create... */
void * create_shmem(int shmkey, unsigned int shmbytes)
#ifdef SHMEM_SHM
{
 int shmid;
 void *local_mem_ptr;


  if (shmem_debug)
    printf("create_shmem(): creating shmem with key %d.\n", shmkey);

  shmid = shmget(shmkey, shmbytes, IPC_CREAT|IPC_EXCL|0666);

  if (shmid==-1) {
    /* perror("Error creating shared memory"); */
    return NULL;
  }

  local_mem_ptr = shmat(shmid, 0/* no preferred address */
                             , 0 /* use defaults */);

  if (local_mem_ptr == NULL) {
    /* perror("Error creating shared memory"); */
    delete_shmem(shmkey);
    return NULL;
  }

  memset(local_mem_ptr, 0, shmbytes);

  return local_mem_ptr;
}
#endif
#ifdef SHMEM_MF
{
 int shmfd;
 void *local_mem_ptr;
 char *filename = build_filename(shmkey);

  if (shmem_debug)
    printf("create_shmem(): creating shmem with key %d.\n", shmkey);

  if (filename == NULL) 
    goto error_exit_0;
    
  shmfd = shm_open(filename, O_RDWR | O_CREAT | O_EXCL, 0666 );
  if( shmfd == -1 ) {
    /* perror("Error creating shared memory"); */
    goto error_exit_1;
  }

  /* Set the memory object's size */
  if( ftruncate( shmfd, shmbytes) < 0 ) {
    /* perror("Error creating shared memory"); */
    goto error_exit_2;
  }
    
  /* Map the memory object */
  /* Use MAP_PHYS in QNX to indicate physical memory is required! */
  /* Use MAP_LOCKED in Linux to lock memory to physical memory! */
  /* NULL -> no preferred address */
  local_mem_ptr = mmap( NULL, shmbytes, PROT_READ | PROT_WRITE, MAP_SHARED /*| MAP_PHYS*/, shmfd, 0 );
  if( local_mem_ptr == MAP_FAILED ) {
    /* perror("Error creating shared memory"); */
    goto error_exit_2;
  }

  memset(local_mem_ptr, 0, shmbytes);
  close(shmfd);
  free(filename);
  return local_mem_ptr;

/*
error_exit_3:
  munmap();
*/
error_exit_2:
  close(shmfd);
error_exit_1:
  free(filename); 
error_exit_0:
  return NULL;
}
#endif










void *attach_shmem(int shmkey,  unsigned int shmbytes)
#ifdef SHMEM_SHM
{
  int shmid;

  if (shmem_debug)
    printf("attach_shmem(): attaching shmem with key %d.\n", shmkey);

  shmid = shmget(shmkey, 0, 0);

  if (shmid==-1) {
    /* perror("Error attaching shared memory"); */
    return NULL;
  }

  return shmat(shmid, 0/* no preferred address */, 0 /* use defaults */);
}
#endif
#ifdef SHMEM_MF
{
 int shmfd;
 void *local_mem_ptr;
 char *filename = build_filename(shmkey);
 
  if (shmem_debug)
    printf("attach_shmem(): attaching shmem with key %d.\n", shmkey);

  if (filename == NULL) 
    goto error_exit_0;
    
  shmfd = shm_open(filename, O_RDWR, 0);
  if( shmfd == -1 ) {
    /* perror("Error attaching shared memory"); */
    goto error_exit_1;
  }

  /* Map the memory object */
  /* Use MAP_PHYS in QNX to indicate physical memory is required! */
  /* Use MAP_LOCKED in Linux to lock memory to physical memory! */
  /* NULL -> no preferred address */
  local_mem_ptr = mmap( NULL, shmbytes, PROT_READ | PROT_WRITE, MAP_SHARED /*| MAP_PHYS*/, shmfd, 0 );
  if( local_mem_ptr == MAP_FAILED ) {
    /* perror("Error attaching shared memory"); */
    goto error_exit_2;
  }

  close(shmfd);
  free(filename);
  return local_mem_ptr;

/*
error_exit_3:
  munmap();
*/
error_exit_2:
  close(shmfd);
error_exit_1:
  free(filename); 
error_exit_0:
  return NULL;
}
#endif



int delete_shmem(int shmkey)
#ifdef SHMEM_SHM
{
 int shmid;

  if (shmem_debug)
    printf("delete_shmem(): deleting shmem with key %d.\n", shmkey);

 /* get hold of shared memmory */
  shmid = shmget(shmkey, 0, 0);

  if (shmid==-1) {
      /* perror("Error removing shared memory"); */
    return -1;
  }

 /* delete shared memory  */
  if (shmctl(shmid, IPC_RMID, 0)== -1) {
      /* perror("Error removing shared memory"); */
      return -1;
    }

 return 0;
}
#endif
#ifdef SHMEM_MF
{
 char *filename = build_filename(shmkey);
 
  if (shmem_debug)
    printf("delete_shmem(): deleting shmem with key %d.\n", shmkey);

  if (filename == NULL) return -1;
    
  if (shm_unlink(filename)< 0) {
    /* perror("Error removing shared memory"); */
    free(filename);
    return -1;
  }

  /* The shared memory will be removed once all processes call close() 
   * and unmap() this shared memory file!
   */ 
  free(filename); 
  return 0;
}
#endif

