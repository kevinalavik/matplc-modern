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
 * GMM module for local operation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>
#include <time.h>  /* random seed generator */

#include <misc/mutex_util.h>
#include "gmm_private.h"
#include "gmm_local.h"
#include "protocol.h"
#include "plcproxy.h"
#include <misc/shmem_util.h>
#include <misc/string_util.h>


static int debug = 0;


static u32 *gmm_globmap_ = NULL; /* global map - data */
static plcmutex_t mutex_ = PLCMUTEX_FAILED;

static int local_privmap_key_ = -1; /* private map shared memory key */


static int plc_update_local(void);

static int plc_update_pt_local(plc_pt_t p);

static int plc_update_pts_local(plc_pt_t p[], int count);

static int gmm_done_local(void);


int gmm_init_local(int privmap_key)
/* if privmap_key == 0 -> use malloc for allocating local maps   */
/* if privmap_key != 0 -> attach shared memory for local maps    */
/*                        notes:                                 */
/*                         - shared memory must already exist    */
/*                         - shared memory for the local map is  */
/*                           used by the proxy process in the    */
/*                           isolate mode. For more info, see    */
/*                           gmm.c file                          */
/*
 * This function must be 'atomic', i.e. it should either
 * setup all the memory areas and semaphores correctly
 * or not do anything at all.
 */
{
  void *privmap;

  if (debug)
    printf
	("gmm_init_local() with name %s, privmap_key=%d\n",
	 gmm_module_name_, privmap_key);

  if (gmm_get_status(gmm_fully_initialized)) {
    if (debug)
      printf("Gobal Memory Manager library already initialized at %s:%d.\n",
             __FILE__, __LINE__);
    return -1;
  }

  /* initialize function pointers to local functions */
  gmm_mod_.pt_count = &plc_pt_count_local;
  gmm_mod_.pt_by_index = &plc_pt_by_index_local;
  gmm_mod_.pt_by_name = &plc_pt_by_name_local;
  gmm_mod_.pt_by_loc = &plc_pt_by_loc_local;
  gmm_mod_.update = &plc_update_local;
  gmm_mod_.update_pt = &plc_update_pt_local;
  gmm_mod_.update_pts = &plc_update_pts_local;
  gmm_mod_.done = &gmm_done_local;

  /* get hold of conf map shared memory */
  if (gmm_init_conf_struct() < 0)
    return gmm_init_error(attached_conf_struct);

  gmm_set_status(attached_conf_struct);
  /* open global map semaphore... */
  mutex_ = plcmutex_open(&(gmm_shm_->mutexid));
  if (mutex_ == PLCMUTEX_FAILED)
    return gmm_init_error(opened_globsem);
  gmm_set_status(opened_globsem);

  /* get hold of global map shared memory */
  if (!(gmm_globmap_=attach_shmem(gmm_shm_->globalmap_key, gmm_shm_->globalmap_size))) {
    if (debug)
      printf("Could not attach global map shared memory, key=%d\n",
             gmm_shm_->globalmap_key);
    return gmm_init_error(attached_globmap);
  }
  gmm_set_status(attached_globmap);

  /* allocate memory for private map */
  if (privmap_key == 0)
    /* use malloc for private maps */
  {
    gmm_privmap_ = malloc(2 * gmm_globalmap_size_);
    gmm_mapmask_ = NULL;

    if (gmm_privmap_ == NULL)
      return gmm_init_error(malloced_privmap);

    gmm_set_status(malloced_privmap);
    local_privmap_key_ = -1;

    gmm_mapmask_ = (u32 *)((char *)gmm_privmap_ + gmm_globalmap_size_);

  } else
    /* attach shared memmory for local maps */
  {
    if (! (privmap = attach_shmem(privmap_key, 2 * gmm_globalmap_size_))) {
      if (debug)
	printf("Could not get handle to private map shared memory.\n");
      return gmm_init_error(attached_privmap);
    }

    local_privmap_key_ = privmap_key;

    gmm_privmap_ = (u32 *)privmap;
    gmm_mapmask_ = (u32 *)((char *) privmap + gmm_globalmap_size_);
    gmm_set_status(attached_privmap);
  }

  /* get current map from the shared memory */
  if (plc_update() < 0)
    return gmm_init_error(gmm_fully_initialized);

  /* setup the r/w mapmask */
  /* We can only do this after doing plc_update(), otherwise we shall      */
  /* overwrite on the globalmap all the points to which the current module */
  /* has write access (ownership)                                          */
  if (gmm_init_mapmask(gmm_module_name_, gmm_mapmask_, gmm_globalmap_size_) < 0)
    return gmm_init_error(gmm_fully_initialized);

  gmm_set_status(gmm_fully_initialized);

  return 0;
}


static int gmm_done_local(void)
{
 int res = 0;

  if (debug) printf ("plc_done(): ...\n");

  gmm_rst_status(gmm_fully_initialized);

 /* delete privmap shared memory */
  if (gmm_get_status(created_privmap))
  {
   if (delete_shmem(local_privmap_key_) < 0)
     res = -1;
   local_privmap_key_ = -1;
   gmm_privmap_ = NULL;
   gmm_rst_status(created_privmap);
  }

 /* detach privmap shared memory */
  if (gmm_get_status(attached_privmap))
  {
   if ( detach_shmem(gmm_privmap_, 2 * gmm_globalmap_size_) < 0)
     res = -1;
   local_privmap_key_ = -1;
   gmm_privmap_ = NULL;
   gmm_rst_status(attached_privmap);
  }


 /* delete privmap malloced memory */
  if (gmm_get_status(malloced_privmap))
  {
   if (gmm_privmap_ != NULL) free(gmm_privmap_);
   gmm_privmap_ = NULL;
   gmm_mapmask_ = NULL;
   gmm_rst_status(malloced_privmap);
  }

 /* detach globalmap shared memory */
  if (gmm_get_status(attached_globmap))
  {
   if ( detach_shmem(gmm_globmap_, gmm_shm_->globalmap_size) < 0)
     res = -1;
   gmm_globmap_ = NULL;
   gmm_rst_status(attached_globmap);
  }

 /* close global map semaphore */
  if (gmm_get_status(opened_globsem)) {
    if (plcmutex_close(mutex_) < 0)
      res = -1;
    mutex_ = PLCMUTEX_FAILED;
   gmm_rst_status(opened_globsem);
  }

 /* remove pointer to gmm_t conf struct */
  if (gmm_get_status(attached_conf_struct))
  {
   gmm_shm_ = NULL;
   gmm_globalmap_size_ = 0;
   gmm_globalmap_len_ = 0;
   gmm_rst_status(attached_conf_struct);
  }

  return res;
}


#define pp (*(plc_pt_priv_t*)&(p)) /* over-ride the type system */
#define ppp(i) (*(plc_pt_priv_t *)(p + (i)))

#define check_pt_magic(p) check_pt_magic2(p,__FILE__,__LINE__)
static inline void check_pt_magic2(plc_pt_t p, const char*f, int l) {
  if (pp.magic==PT_MAGIC) return;
  printf("Error - point handle has bad magic at %s:%d\n",f,l);
  exit(1);
}

int plc_pt_count_local(void)
{
  return gmm_config_pt_numb(); 
}


plc_pt_t plc_pt_by_loc_local(i32 offset, u8 bit, u8 length)
{
  plc_pt_priv_t p;
  const u32 mask_all_one = ~0;/* make sure we have the correct length of ones */

  if (debug) printf("plc_pt_by_loc_local():...\n");

  p.magic = 0;		/* Assume error. Also resets p.valid */

  if ( ((bit + length) > (8*sizeof(p.mask))) /* point overflows to next reg */
     || (offset < 0)
     || (offset >= gmm_globalmap_len_) ) {
    /* invalid parameters */
    return *((plc_pt_t *)&p);
  }

  p.magic = PT_MAGIC;		/* also sets p.valid */
  p.ofs = offset;
  p.length = length;
  p.bit_shift = bit;
  p.mask = (~(mask_all_one << length)) << bit;
   /* NOTE: the following condition is required because it seems that the
    *       the compiler (or something else) is optimizing the code, and
    *       not doing the shift left (<<) when length has the maximum value.
    *       Has anybody a better explanation for what is occuring ?
    */
  if (length == 8*sizeof(p.mask))
    p.mask = mask_all_one;

  return *((plc_pt_t *)&p);
};


plc_pt_t plc_pt_by_index_local(int index, char **name, char **owner)
{
  plc_pt_priv_t p;
  i32 offset;
  u8 bit, length;
  char *tmp_name, *tmp_owner;

  if (debug) printf("plc_pt_by_index_local():...\n");

  p.magic = 0;          /* Assume error. Also resets p.valid */

  if (gmm_config_pt_by_index(index, &tmp_name, &tmp_owner,
                             &offset, &bit, &length) < 0)
    /* point doesn't exist or other problem */
    return *((plc_pt_t *)&p);

  if (name != NULL) {
    if (tmp_name != NULL)
      *name = strdup(tmp_name);
    else
      *name = NULL;
  }

  if (owner != NULL) {
    if (tmp_owner != NULL)
      *owner = strdup(tmp_owner);
    else
      *owner = NULL;
  }

  return plc_pt_by_loc_local(offset, bit, length);
}


plc_pt_t plc_pt_by_name_local(const char *name)
{
  plc_pt_priv_t p;
  i32 offset;
  u8  bit, length;

  if (debug) printf("plc_pt_by_name_local():...\n");

  p.magic = 0;		/* Assume error. Also resets p.valid */

  if (gmm_config_pt_by_name(name, &offset, &bit, &length) < 0)
    /* point doesn't exist or other problem */
    return *((plc_pt_t *)&p);
  
  return plc_pt_by_loc_local(offset, bit, length);
}


static int plc_update_local(void)
{
  i32 i;

  /* grab the datamap */
  plcmutex_lock(mutex_);

  /* do the update (direction is determined by the mapmask for each bit) */
  for (i = 0; i < gmm_globalmap_len_; i++) {
    gmm_privmap_[i] = gmm_globmap_[i] =
	(gmm_globmap_[i] & ~gmm_mapmask_[i]) |
	(gmm_privmap_[i] & gmm_mapmask_[i]);
  }

  /* release the datamap */
  plcmutex_unlock(mutex_);

  return 0;
}


static int plc_update_pt_local(plc_pt_t p)
{
  check_pt_magic(p);

  /* grab the datamap */
  plcmutex_lock(mutex_);

  gmm_globmap_[pp.ofs] =
      (gmm_globmap_[pp.ofs] & ~(gmm_mapmask_[pp.ofs] & pp.mask)) |
      (gmm_privmap_[pp.ofs] &  (gmm_mapmask_[pp.ofs] & pp.mask));
  gmm_privmap_[pp.ofs] =
      (gmm_globmap_[pp.ofs] &  pp.mask) |
      (gmm_privmap_[pp.ofs] & ~pp.mask);

  /* release the datamap */
  plcmutex_unlock(mutex_);

  return 0;
}


static int plc_update_pts_local(plc_pt_t p[], i32 count)
{
  i32 i;

  /* grab the datamap */
  plcmutex_lock(mutex_);

  for (i = 0; i < count; i++) {
    check_pt_magic(p[i]);
    gmm_globmap_[ppp(i).ofs] =
      (gmm_globmap_[ppp(i).ofs] & ~(gmm_mapmask_[ppp(i).ofs] & ppp(i).mask)) |
      (gmm_privmap_[ppp(i).ofs] & (gmm_mapmask_[ppp(i).ofs] & ppp(i).mask));
    gmm_privmap_[ppp(i).ofs] =
	(gmm_globmap_[ppp(i).ofs] & ppp(i).mask) |
	(gmm_privmap_[ppp(i).ofs] & ~ppp(i).mask);
  }				/* for(...) */

  /* release the datamap */
  plcmutex_unlock(mutex_);

  return 0;
}
