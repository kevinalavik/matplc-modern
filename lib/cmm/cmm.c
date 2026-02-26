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
 * CMM Manager - Configuration Memmory Manager library implementation
 *
 * This file implements the routines in cmm.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>
#include <time.h>  /* random seed generator */

#include "plc.h"

#include <misc/mutex_util.h>
#include "cmm_private.h"
#include "cmm_setup.h"
#include <conffile/conffile.h>
#include <log/log.h>
#include <misc/shmem_util.h>
#include <misc/string_util.h>
#include "../plc_private.h"



static int debug = 0;



/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

/* the current status of the cmm library (how far did init get?) */
static status_t cmm_status_;

static const status_t cmm_fully_initialized      = {0x0001};


/* access functions for the status_ variable */
static inline int cmm_get_status(const status_t stat_bit) {
  return (cmm_status_.s & stat_bit.s) != 0;
}

static inline void cmm_set_status(const status_t stat_bit) {
  cmm_status_.s |= stat_bit.s;
}

static inline void cmm_rst_status(const status_t stat_bit) {
  cmm_status_.s &= (~stat_bit.s);
}





/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************       cmm_private.h      *****************/
/*****************                          *****************/
/************************************************************/


/* name of this module, as given to plc_init or from the command line */
const char *cmm_module_name_ = NULL;

/* ID of plc being accesed */
int cmm_plc_id_ = -1;

/* pointer to cmm_map shared emmory */
cmm_t *cmm_shm_   = NULL;

/* semaphore protecting access to cmm_map shared memory */
plcmutex_t cmm_mutex_ = PLCMUTEX_FAILED;


#define __assert_cmm_initialized(ret_val)                             \
        { if (!cmm_get_status(cmm_fully_initialized))                 \
            {if (debug)                                               \
               printf ("Configuration Shared Memory not initialized " \
                       "at %s:%d.\n",                                 \
                      __FILE__, __LINE__);                            \
             return ret_val;                                          \
            };                                                        \
        };

#define __assert_cmm_not_initialized(ret_val)                         \
        { if (cmm_get_status(cmm_fully_initialized))                  \
            return ret_val;                                           \
        };


/* helper function for cmm_parse_conf() */
static void cmm_parse_conf_i32(const char *section, const char *name,
                               i32 *var, i32 min, i32 max, i32 def,
                               int use_default)
{
  int res = 0;

  if (!use_default)
    def = *var;

  if (section == NULL)
    res = conffile_get_value_i32(name, var, min, max, def);
  else
    res = conffile_get_value_sec_i32(section, name, var, min, max, def);

  if ((res < 0) && (use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s %s,"
                   " or value out of bounds [%d...%d]. Using default %d.\n",
                   name, conffile_get_value(name),
                   min, max, *var);
  }
}

/* helper function for cmm_parse_conf() */
static void cmm_parse_conf_u32(const char *section, const char *name,
                               u32 *var, u32 min, u32 max, u32 def,
                               int use_default)
{
  int res = 0;

  if (!use_default)
    def = *var;

  if (section == NULL)
    res = conffile_get_value_u32(name, var, min, max, def);
  else
    res = conffile_get_value_sec_u32(section, name, var, min, max, def);

  if ((res < 0) && (use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s %s,"
                   " or value out of bounds [%d...%d]. Using default %d.\n",
                   name, conffile_get_value(name),
                   min, max, *var);
  }
}

/* parse the config file for cmm related values */
/*
 * Note: this function will only really look for the confmap
 *       if this variable has been initialized to something < 0 (e.g. -1).
 */
void cmm_parse_conf(int *confmap_key,
                    u32 *confmap_size,
                     /*
                      * flag indicating if we should return the default values
                      * if the parameter has been left unspecified in the
                      * config file.
                      * 1 - yes, init to the default value if left unspecified
                      * 0 - no, leave parm. value unchanged if left unspecified
                      */
                    int use_default)
{
  if (confmap_key != NULL)
  if (*confmap_key < 0)
    /* NOTE: only use the value in config file if this parameter is not
     *       set on the command line arguments...
     */
    cmm_parse_conf_i32(CONF_CK_SECTION, CONF_CK_NAME, confmap_key,
                       CONF_CK_MIN, CONF_CK_MAX, CMM_DEF_CONFMAP_KEY,
                       use_default);

  if (confmap_size != NULL)
    cmm_parse_conf_u32(CONF_CP_SECTION, CONF_CP_NAME, confmap_size,
                       CONF_CP_MIN, CONF_CP_MAX, CMM_DEF_CONFMAP_PG,
                       use_default);
}




/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************          cmm.h           *****************/
/*****************                          *****************/
/************************************************************/


/********************************************************/
/*                                                      */
/* Init and Done functions                              */
/*                                                      */
/********************************************************/

int cmm_init(const char *mod_name, 
             int confmap_key)
{
  u32 confmap_pg    = 0;

  __assert_cmm_not_initialized(-1);

  /* read the values from the config file */
  cmm_parse_conf(&confmap_key, &confmap_pg,
                 1 /* return the default values if parm. is left unspecified*/);

  /* get hold of conf map shared memory */
  if (confmap_key <= 0) {
    plc_log_errmsg(1, "Invalid plc_id = %d, cannot access plc.", confmap_key);
    goto error_exit_0;
  }

  {
    cmm_t *cmm_header;
    u32   confmap_size;
    if (!(cmm_header = attach_shmem(confmap_key, sizeof(*cmm_header)))) {
      if (debug)
        printf("Could not attach configuration shared memory.\n");
      goto error_exit_0;
    }
    /* do the consistency checks */
    if (cmm_header->magic != CMM_MAGIC) {
      fprintf(stderr, "PLC ERROR - configuration shared memory has bad magic!\n");
      detach_shmem((void *) cmm_header, sizeof(*cmm_header));
      goto error_exit_0;
    }

    confmap_size = cmm_header->confmap_size;
    /* no longer required... */
    detach_shmem((void *) cmm_header, sizeof(*cmm_header));

    if (!(cmm_shm_ = attach_shmem(confmap_key, confmap_size))) {
      if (debug)
        printf("Could not attach configuration shared memory.\n");
      detach_shmem((void *) cmm_header, sizeof(*cmm_header));
      goto error_exit_0;
    }
    /* do the consistency checks */
    /* Should not need to do it again, but we do it nevertheless.... */
    if (cmm_shm_->magic != CMM_MAGIC) {
      fprintf(stderr, "PLC ERROR - configuration shared memory has bad magic!\n");
      goto error_exit_1;
    }
  }

  /* get access to the cmm semaphore... */
  cmm_mutex_ = plcmutex_open(&(cmm_shm_->mutexid));
  if (cmm_mutex_ == PLCMUTEX_FAILED)
    goto error_exit_1;

  cmm_plc_id_ = confmap_key;
  cmm_module_name_ = mod_name;
  cmm_set_status(cmm_fully_initialized);
  return 0;

/*
error_exit_2:
  plcmutex_close(&(cmm_shm_->mutexid));
  cmm_sem_ = PLCSEM_FAILED;
*/
error_exit_1:
  detach_shmem((void *) cmm_shm_, cmm_shm_->confmap_size);
  cmm_shm_ = NULL;
error_exit_0:
  return -1;
}


int cmm_done(void)
{
  int res;

  if (debug) printf ("cmm_done(): called...\n");

  cmm_rst_status(cmm_fully_initialized);

 /* close semaphore */
  if (plcmutex_close(cmm_mutex_) < 0)
    res = -1;
  cmm_mutex_ = PLCMUTEX_FAILED;
    
 /* detach confmap shared memory */
  if (cmm_shm_ != NULL)
    if (detach_shmem(cmm_shm_, cmm_shm_->confmap_size) < 0)
      res = -1;
  cmm_shm_ = NULL;

  cmm_module_name_ = NULL;
  cmm_plc_id_ = -1;

  if (debug) printf ("cmm_done(): returning...\n");
  return 0;
}



/********************************************************/
/*                                                      */
/* Memory access functions...                           */
/*                                                      */
/********************************************************/


/* return plc_id being used */
int cmm_plc_id(void)
{
  __assert_cmm_initialized(-1);

  return cmm_plc_id_;
}


/* return cmm sem_key being used */
plcmutex_id_t *cmm_mutex_id_ptr(void)
{
  __assert_cmm_initialized(NULL);

  return &(cmm_shm_->mutexid);
}


/* get number of memory blocks of specified type */
int cmm_block_count(cmm_block_type_t block_type)
{
  int count = 0;
  cmm_block_t *iter_block;

  __assert_cmm_initialized(-1);

  /* grab the semaphore */
  plcmutex_lock(cmm_mutex_);

  for(iter_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_shm_->first_block);
      (char *)iter_block != (char *)cmm_shm_;
      iter_block = (cmm_block_t *)((char *)cmm_shm_ + iter_block->next))
    if (iter_block->type == block_type)
      count++;

  /* release the semaphore */
  plcmutex_unlock(cmm_mutex_);

  return count;
}


/* allocate a memory block */
void *cmm_block_alloc(cmm_block_type_t block_type,
                      const char *name,
                      u32 num_bytes)
{
  cmm_block_t * new_block;
  cmm_block_t *free_block;
  cmm_block_t *iter_block;

  if (debug) {
    if (name == NULL)
      printf("cmm_block_alloc(): called... type=%d, name=<NULL>, size=%d\n", block_type, num_bytes);
    else
      printf("cmm_block_alloc(): called... type=%d, name=%s, size=%d\n", block_type, name, num_bytes);
  }

  __assert_cmm_initialized(NULL);

  /* check input paramters... */
  if ((NULL == name) || (0 == num_bytes))
    return NULL;

  /* grab the semaphore */
  plcmutex_lock(cmm_mutex_);

  /* look for the smallest free memory block that is large enough
   * for our needs.
   */
  free_block = NULL;
  for(iter_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_shm_->first_block);
      (char *)iter_block != (char *)cmm_shm_;
      iter_block = (cmm_block_t *)((char *)cmm_shm_ + iter_block->next)) {

    if ((iter_block->type == block_type_free) && (iter_block->size >= num_bytes)) {
      if (free_block == NULL)
        free_block = iter_block;
      else
        if (free_block->size > iter_block->size)
          free_block = iter_block;
    }
  }
  if (free_block == NULL)
    /* not enough memory*/
    goto error_exit;

   /* use the free block for the new block */
  new_block = free_block;

  /* Does the new_block have enough bytes left over to
   * create a new free_block?
   */
  if (new_block->size > num_bytes + sizeof(cmm_block_t)) {
    /* YES!
     * We will create a new free_block with the remaining memory
     */
    /*
     * NOTE: new_block is the block we will be returning to this function's caller
     *       free_block is a new block with the remaining free memory.
     *       new_block is not really new, it is the original free_block that will
     *                 be brocken up
     *
     * Yes, I know, the variable could have better names...
     */

    free_block = (cmm_block_t *)((char *)new_block + num_bytes + sizeof(cmm_block_t));

    free_block->type = block_type_free;

    free_block->size = new_block->size - num_bytes - sizeof(cmm_block_t);
    new_block ->size = num_bytes;

    free_block->next = new_block->next;
    free_block->prev = (char *)new_block  - (char *)cmm_shm_; /* prev = new_block */
    new_block->next  = (char *)free_block - (char *)cmm_shm_; /* prev = new_block */

    if (cmm_shm_->last_block == free_block->prev) /* i.e. == new_block  */
      cmm_shm_->last_block = new_block->next;     /* i.e.  = free_block */
  }

  new_block->type = block_type;
  strncpy(new_block->name,  name,  CMM_NAME_MAX_LEN);

  /* release the datamap */
  plcmutex_unlock(cmm_mutex_);

  /* clear out the memory block we will be giving out... */
  memset((void *)((char *)new_block + sizeof(cmm_block_t)), 0, num_bytes);

  if (debug) printf("cmm_block_alloc(): returning success (ptr=%p)\n", (void *)((char *)new_block + sizeof(cmm_block_t)));
  return (void *)((char *)new_block + sizeof(cmm_block_t));

error_exit:
  /* release the datamap */
  plcmutex_unlock(cmm_mutex_);
  if (debug) printf("cmm_block_alloc(): returning error (ptr=%p)\n", (void *)NULL);
  return NULL;
}


 /* merge two free_blocks into one */
 /*
  * The cmm semaphore must be held by the calling function!
  */
static int cmm_merge_free_blocks(cmm_block_t *cmm_block, int next_prev_flag)
{
  cmm_block_t *merge_block = NULL;

  switch (next_prev_flag) {
    case  1: /* merge with next block */
             if (cmm_block->next == 0)
               return -1;
             merge_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_block->next);
             break;

    case -1: /* merge with prev block */
             if (cmm_block->prev == 0)
               return -1;
             merge_block = cmm_block;
             cmm_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_block->prev);
             break;

    default: return -1;
  } /* switch() */

  if ((merge_block->type != block_type_free) ||
      (  cmm_block->type != block_type_free))
    return -1;

  cmm_block->next = merge_block->next;
  cmm_block->size += merge_block->size + sizeof(cmm_block_t);

  return 0;
}



/* Free a memory block */
int cmm_block_free(void *del_block_mem)
{
  cmm_block_t *iter_block;
  cmm_block_t * del_block;

  if (debug) printf("cmm_block_free(): called... ptr=%p\n", del_block_mem);

  __assert_cmm_initialized(-1);

  if (del_block_mem == NULL)
    return 0;

  /* grab the semaphore */
  plcmutex_lock(cmm_mutex_);

  /* first check to see if it is a real memmory block in the cmm list... */
  del_block = NULL;
  for(iter_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_shm_->first_block);
      (char *)iter_block != (char *)cmm_shm_;
      iter_block = (cmm_block_t *)((char *)cmm_shm_ + iter_block->next)) {

    if ((char *)iter_block + sizeof(cmm_block_t) == (char *)del_block_mem) {
      /* block found! */
      del_block = iter_block;
      break;
    }
  }

  if (del_block == NULL)
    goto error_exit;

  /* block found! */

  /* mark block as free... */
  del_block->type = block_type_free;

  cmm_merge_free_blocks(del_block,  1);
  cmm_merge_free_blocks(del_block, -1);

  /* release the datamap */
  plcmutex_unlock(cmm_mutex_);

  if (debug) printf("cmm_block_free(): returning sucess...\n");
  return 0;

error_exit:
  /* release the datamap */
  plcmutex_unlock(cmm_mutex_);
  if (debug) printf("cmm_block_free(): returning error...\n");
  return -1;
}


/* Search for a memory block */
void *cmm_block_find(cmm_block_type_t block_type,
                     const char *name,
                     u32 *num_bytes)
{
  cmm_block_t *iter_block;

  if (debug) printf("cmm_block_find(%d, %s): called... \n", block_type, name);
  
  if (num_bytes != NULL) *num_bytes = 0;
  __assert_cmm_initialized(NULL);

  if (name == NULL)
    return NULL;

  /* grab the semaphore */
  plcmutex_lock(cmm_mutex_);

  for(iter_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_shm_->first_block);
      (char *)iter_block != (char *)cmm_shm_;
      iter_block = (cmm_block_t *)((char *)cmm_shm_ + iter_block->next)) {

    if (iter_block->type == block_type)
      if (strcmp(iter_block->name, name) == 0)
        break;
  }

  /* release the datamap */
    /* we access the memory block in the following lines,
       so it may seem we shouldn't be releasing the semaphore
       just yet, but since we will be returning a pointer to the
       memory block, it doesn't make much difference really. The
       caller of the function may still de acessing a deleted block!
      */
  plcmutex_unlock(cmm_mutex_);

  if ((char *)iter_block == (char *)cmm_shm_)
    /* block not found! */
    return NULL;

  if (num_bytes != NULL)
    *num_bytes = iter_block->size;

  if (debug) printf("cmm_block_find(%d, %s): returning success -> %p \n",  block_type, name, (void *)((char *)iter_block + sizeof(cmm_block_t)));
  
  return (void *)((char *)iter_block + sizeof(cmm_block_t));
}


/* Get a memory block by index */
void *cmm_block_get(cmm_block_type_t block_type,
                    u32 index,
                    u32 *num_bytes,
                    char **name)
{
  cmm_block_t *iter_block;

  if (num_bytes != NULL) *num_bytes = 0;
  if (name      != NULL) *name = NULL;
  __assert_cmm_initialized(NULL);

  /* grab the semaphore */
  plcmutex_lock(cmm_mutex_);

  for(iter_block = (cmm_block_t *)((char *)cmm_shm_ + cmm_shm_->first_block);
      (char *)iter_block != (char *)cmm_shm_;
      iter_block = (cmm_block_t *)((char *)cmm_shm_ + iter_block->next)) {

    if (iter_block->type == block_type)
      if (index-- == 0)
        break;
  }

  /* release the datamap */
    /* we access the memory block in the following lines,
       so it may seem we shouldn't be releasing the semaphore
       just yet, but since we will be returning a pointer to the
       memory block, it doesn't make much difference really. The
       caller of the function may still de acessing a deleted block!
      */
  plcmutex_unlock(cmm_mutex_);

  if ((char *)iter_block == (char *)cmm_shm_)
    return NULL;  /* block not found */

  if (num_bytes != NULL)
    *num_bytes = iter_block->size;

  if (name != NULL)
    *name = iter_block->name;

  return (void *)((char *)iter_block + sizeof(cmm_block_t));
}


/* Get the address of the first byte of the cmm shared memory. */
/* This is required in order to de-reference pointers before
 * storing them in a shared memory block that will be accessed
 * by other processes.
 *
 * The caller better be careful with what they do with this pointer!
 * Returns NULL on error!
 */
void *cmm_refptr(void) {
  __assert_cmm_initialized(NULL);
  return cmm_shm_;
}

/* Check whether the pointer ptr is referencing a memory location
 * within the cmm shared memory.
 *
 * Returns 1 if true.
 * Returns 0 if false.
 * Returns -1 on error (cmm not initialised...)
 */
int cmm_checkptr(void *ptr) {
  __assert_cmm_initialized(-1);
  return (((char *)cmm_shm_ <= (char *)ptr) &&
          ((char *)ptr - (char *)cmm_shm_ < cmm_shm_->confmap_size));
}



