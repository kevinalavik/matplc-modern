/*
 * (c) 2002  Mario de Sousa
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
 * GMM module for shared operation
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
#include "gmm_shared.h"
#include "protocol.h"
#include "plcproxy.h"
#include <misc/shmem_util.h>
#include <misc/string_util.h>


static int debug = 0;


static u32 *gmm_globmap_; /* global map - data */

static int plc_update_shared(void);

static int plc_update_pt_shared(plc_pt_t p);

static int plc_update_pts_shared(plc_pt_t p[], int count);

static int gmm_done_shared(void);


int gmm_init_shared(int dummy /* unused parameter */)
/*
 * This function must be 'atomic', i.e. it should either
 * setup all the memory areas and semaphores correctly
 * or not do anything at all.
 */
{

  if (debug)
    printf ("gmm_init_shared() with name %s\n", gmm_module_name_);

  if (gmm_get_status(gmm_fully_initialized)) {
    if (debug)
      printf("Gobal Memory Manager library already initialized at %s:%d.\n",
             __FILE__, __LINE__);
    return -1;
  }

  /* initialize function pointers to shared functions */
  gmm_mod_.pt_count = &plc_pt_count_local;
  gmm_mod_.pt_by_index = &plc_pt_by_index_local;
  gmm_mod_.pt_by_name = &plc_pt_by_name_local;
  gmm_mod_.pt_by_loc = &plc_pt_by_loc_local;
  gmm_mod_.update = &plc_update_shared;
  gmm_mod_.update_pt = &plc_update_pt_shared;
  gmm_mod_.update_pts = &plc_update_pts_shared;
  gmm_mod_.done = &gmm_done_shared;

  /* get hold of conf map shared memory */
  if (gmm_init_conf_struct() < 0)
    return gmm_init_error(attached_conf_struct);

  gmm_set_status(attached_conf_struct);

  /* get hold of global map shared memory */
  if (!(gmm_globmap_ = attach_shmem(gmm_shm_->globalmap_key, gmm_shm_->globalmap_size))) {
    if (debug)
      printf("Could not attach global map shared memory, key=%d\n",
             gmm_shm_->globalmap_key);
    return gmm_init_error(attached_globmap);
  }
  gmm_set_status(attached_globmap);

  /* private map is the same as the global map! */
  /* i.e., we use the global map as if it were the private map! */
  gmm_privmap_ = gmm_globmap_;

  /* allocate memory for map mask */
  gmm_mapmask_ = malloc(gmm_globalmap_size_);
  if (gmm_mapmask_ == NULL)
    return gmm_init_error(malloced_mapmask);
  gmm_set_status(malloced_mapmask);

  /* setup the r/w mapmask */
  if (gmm_init_mapmask(gmm_module_name_, gmm_mapmask_, gmm_globalmap_size_) < 0)
    return gmm_init_error(gmm_fully_initialized);

  gmm_set_status(gmm_fully_initialized);

  return 0;
}


static int gmm_done_shared(void)
{
 int res = 0;

  if (debug) printf ("plc_done(): ...\n");

  gmm_rst_status(gmm_fully_initialized);

 /* delete map mask malloced memory */
  if (gmm_get_status(malloced_mapmask)) {
    free(gmm_mapmask_);
    gmm_mapmask_ = NULL;
    gmm_rst_status(malloced_mapmask);
  }

 /* detach globalmap shared memory */
  if (gmm_get_status(attached_globmap)) {
    if ( detach_shmem(gmm_globmap_ , gmm_shm_->globalmap_size) < 0)
      res = -1;
    gmm_globmap_ = NULL;
    gmm_privmap_ = NULL;
    gmm_rst_status(attached_globmap);
  }

 /* remove pointer to gmm_t conf struct */
  if (gmm_get_status(attached_conf_struct)) {
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


static int plc_update_shared(void)
{
  return 0;
}


static int plc_update_pt_shared(plc_pt_t p)
{
  check_pt_magic(p);
  return 0;
}


static int plc_update_pts_shared(plc_pt_t p[], i32 count)
{
  i32 i;

  for (i = 0; i < count; i++)
    check_pt_magic(p[i]);

  return 0;
}
