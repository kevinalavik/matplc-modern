/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
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
 * CMM Manager - setup and shutdown functions
 *
 * This file implements the routines in cmm_setup.h
 *
 * In general, it should only be called by plc_setup() and plc_shutdown()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>

#include <misc/mutex_util.h>
#include "cmm_private.h"
#include "cmm_setup.h"
#include <misc/shmem_util.h>
#include <misc/string_util.h>
#include "plc_private.h"
#include "plc.h"

static const int debug = 0;


/*************************************/
/*                                   */
/*         F U N C T I O N S         */
/*                                   */
/*************************************/

int cmm_setup(int cmm_map_key)
/* returns 0 if successful               */
/* returns -1 on error                   */
/*
 * NOTE: use cmm_map_key = -1 to try get it's value from the config file or
 *       use the default.
 *       cmm_map_key = 0 will use a randomly generated key.
 *
 *       All other values are obtained from the config file.
 */
/*
 * This function should either setup all the memory areas and semaphores
 * correctly or not do anything at all.
 */
{
  cmm_t *cmm_shm;
  cmm_block_t *first_block;
  u32 cmm_map_size;

/*  const char *conffile = NULL;*/
/*  char *conffile_tmp = NULL; */

  if (debug)
    printf("cmm_setup(): cmm_map_key = %d\n", cmm_map_key);

  /* parse the config file for parameters */
  cmm_parse_conf( &cmm_map_key, &cmm_map_size,
                  1 /* return the default if parm. is left unspecified */);

  if (debug)
    printf("cmm_setup(): using cmm_map_key=%d, cmm_map_size=%d\n", 
           cmm_map_key, cmm_map_size);

/* TODO:                                                   */
/* should do some checking to see if values are consistent */

  /* create configuration shared memory */
  /* It doesn't really make much sense to create it with a
   *  random key, but just leave it for the moment...
   */
  if (cmm_map_key == 0)
    cmm_shm = create_shmem_rand(&cmm_map_key, cmm_map_size);
  else
    cmm_shm = create_shmem(cmm_map_key, cmm_map_size);
  if (! cmm_shm)
    goto error_exit_0;

  /* create (and open) the sempahores... */
  if (plcmutex_create(&(cmm_shm->mutexid)) < 0)
    goto error_exit_1;

  /* initialize the conf mem area */
  cmm_shm->magic = CMM_MAGIC;
  cmm_shm->confmap_size = cmm_map_size;

  /* We start off with a single block */
  cmm_shm->first_block = sizeof(cmm_t);
  cmm_shm->last_block  = sizeof(cmm_t);

  /* We now initialise that block with empty memory */
  first_block = (cmm_block_t *)((char *)cmm_shm + cmm_shm->first_block);
  first_block->type = block_type_free;
  first_block->next = 0;
  first_block->prev = 0;
  first_block->size = cmm_map_size - cmm_shm->first_block - sizeof(cmm_block_t);

  /* call cmm_init() */
  if (cmm_init("", cmm_map_key) < 0)
    goto error_exit_2;

  /* "the shared map is now open for business" */
  return 0;

  /* undo anything done up until the error */
error_exit_2:
  plcmutex_destroy(&(cmm_shm->mutexid));

error_exit_1:
  delete_shmem(cmm_map_key);

error_exit_0:
  return -1;
}


int cmm_shutdown(void)
{
  int cmm_map_key;
  int result = 0;
  plcmutex_id_t *mutex_id_ptr;

  if (debug)
    printf("cmm_shutdown(): ...\n");

  /* get the plc_id */
  cmm_map_key = cmm_plc_id();
  if (cmm_map_key < 0)
    /* If we can't get hold of the confmap, then we can't delete it
     * it either.
     */
    return -1;

  /* remove semaphores */
  if ((mutex_id_ptr = cmm_mutex_id_ptr()) != NULL)
    if (plcmutex_destroy(mutex_id_ptr) < 0)
      result = -1;

  /* remove cmm_map */
  if (delete_shmem(cmm_map_key) < 0)
    result = -1;

/*
  cmm_done();
*/
  return result;
}

