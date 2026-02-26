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


#ifndef __CMM_H
#define __CMM_H

#include "../types.h"


/*** Memory block types used in the cmm ***/

typedef enum {
    /* Used by the cmm for managing the list of memory blocks... */
  block_type_first,        /* block_type of first dummy block         */
  block_type_free,         /* block_type of free block                */
    /* Used by the gmm library */
  block_type_gmm_setup,    /* block type for gmm library setup info   */
  block_type_gmm,          /* block type for point info               */
    /* Used by the synch library */
  block_type_synch_setup,  /* block type for synch library setup info */
  block_type_synch_transition,  /* block type for transition info     */
  block_type_synch_synchpt,  /* block type for a synchpt    */
  block_type_plcsynchsem_global_t, /* block used for emulation of SysV semaphore sets */
  block_type_plcsynchsem_local_t, /* block used for emulation of SysV semaphore sets */
    /* Used by the period library */
  block_type_period,       /* block type for module period info       */
    /* Used by the state library */
  block_type_state_shared, /* block type for global state info        */
  block_type_state_module  /* block type for module pid & state info  */
} cmm_block_type_t;


#define CMM_NAME_MAX_LEN 31


/*** GENERAL ***/

int cmm_init(const char *module_name,
             int confmap_shm_key);

int cmm_done(void);

/* return the plc_id being used */
int cmm_plc_id(void);



/*** Memory Access ***/

/* return number of blocks of specified type */
int cmm_block_count(cmm_block_type_t block_type);

/* allocate a memory block */
void *cmm_block_alloc(cmm_block_type_t block_type,
                     const char *name,
                     u32 num_bytes);

/* free a memory block */
int cmm_block_free(void *block);

/* Search for a memory block */
void *cmm_block_find(cmm_block_type_t block_type, 
                     const char *name, 
                     u32 *num_bytes);

/* Get a memory block by index */
void *cmm_block_get(cmm_block_type_t block_type, 
                    u32 index,
                    u32 *num_bytes,
                    char **name);

/* Get the address of the first byte of the cmm shared memory. */
/* This is required in order to de-reference pointers before
 * storing them in a shared memory block that will be accessed
 * by other processes.
 *
 * The caller better be careful with what they do with this pointer!
 * Returns NULL on error!
 */
void *cmm_refptr(void);

/* Check whether the pointer ptr is referencing a memory location
 * within the cmm shared memory.
 *
 * Returns 1 if true.
 * Returns 0 if false.
 * Returns -1 on error (cmm not initialised...)
 */
int cmm_checkptr(void *ptr);

#endif /* __CMM_H */
