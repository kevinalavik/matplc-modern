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
 * GMM module for isolate operation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>

#include <misc/mutex_util.h>
#include "gmm_private.h"
#include "gmm_isolate.h"
#include <cmm/cmm.h>
#include "protocol.h"
#include "plcproxy.h"
#include <misc/shmem_util.h>
#include <misc/string_util.h>


static int debug = 0;


static int isol_privmap_key_ = -1;	/* private map shared memory key */

static int rem_socket_ = -1;	/* socket id for remote connection */

/* pid of proxy server. In proxy_server process it has value 0. */
static int proxy_server_pid_ = -1;


static int plc_update_isolate(void)
{
  int res;
  plc_packet_t packet;
  u32 req_id;

  if (send_req_packet(rem_socket_, packet,
		      type_plc_update_req, &req_id) < 0) return -1;

  if (recv_rep_packet(rem_socket_, &packet,
		      type_plc_update_rep, req_id) < 0) return -1;

  res = packet.message.plc_update_rep.result;

  return res;
}


static int plc_update_pt_isolate(plc_pt_t p)
{
  plc_packet_t packet;
  u32 req_id;

  packet.message.plc_update_point_req.plc_pt = *((plc_pt_priv_t *) & p);

  if (send_req_packet(rem_socket_, packet,
		      type_plc_update_point_req, &req_id) < 0)
    return -1;

  if (recv_rep_packet(rem_socket_, &packet,
		      type_plc_update_point_rep, req_id) < 0)
    return -1;

  return packet.message.plc_update_point_rep.result;
}


static int plc_update_pts_isolate(plc_pt_t p[], int count)
{
  plc_packet_t packet;
  u32 req_id;
  int result, point_cnt;

  result = 0;			/* assume no error */

  /* we could use variable sized packets instead of sending */
  /* several packets until all points have been updated.    */
  /* Let's leave that for later...                          */
  while (count > 0) {
    for (point_cnt = 0;
	 (point_cnt < __PLC_PROTO_MAX_NUM_POINTS) && (count > 0);
	 point_cnt++, count--)
      packet.message.plc_update_points_req.plc_pt[point_cnt] =
	  *((plc_pt_priv_t *) & (p[count - 1]));

    packet.message.plc_update_points_req.count = point_cnt;
    if (send_req_packet(rem_socket_, packet,
			type_plc_update_points_req, &req_id) < 0) {
      result = -1;
      continue;
    }

    if (recv_rep_packet(rem_socket_, &packet,
			type_plc_update_points_rep, req_id) < 0) {
      result = -1;
      continue;
    }

    if (result == 0)
      result = packet.message.plc_update_points_rep.result;
  }				/* while (count > 0) */

  return result;
}


/*
 * There is no plc_pt_by_name_isolate(); instead, plc_pt_by_name_local() is
 * used.
 */


static int gmm_done_isolate(void);

int gmm_init_isolate(int privmap_key)
/* if privmap_key == 0 -> use random key for local maps shared memory  */
/* if privmap_key != 0 -> create shared memory with key for priv. maps */
/*
 * This function must be 'atomic', i.e. it should either
 * setup all the memory areas and semaphores correctly
 * or not do anything at all.
 */
{
  plc_packet_t packet;
  u32 req_id;
  void *privmap;

  if (debug)
    printf("gmm_init_isolate() with name %s, privmap_key=%d...\n",
	   gmm_module_name_, privmap_key);


  if (gmm_get_status(gmm_fully_initialized)) {
    if (debug)
      printf("Global Memory Manager library already initialized at %s:%d.\n",
	     __FILE__, __LINE__);
    return -1;
  }

  /* initialize function pointers to local functions */
  gmm_mod_.pt_count    = &plc_pt_count_local;
  gmm_mod_.pt_by_index = &plc_pt_by_index_local;
  gmm_mod_.pt_by_name  = &plc_pt_by_name_local;
  gmm_mod_.pt_by_loc   = &plc_pt_by_loc_local;
  gmm_mod_.update      = &plc_update_isolate;
  gmm_mod_.update_pt   = &plc_update_pt_isolate;
  gmm_mod_.update_pts  = &plc_update_pts_isolate;
  gmm_mod_.done        = &gmm_done_isolate;

  /* launch proxy process */
  proxy_server_pid_ = -1;
  rem_socket_ = -1;
  if (launch_proxy_server(NULL, &proxy_server_pid_, &rem_socket_) < 0) {
    if (debug)
      printf("Could not launch proxy process.\n");
    return gmm_init_error(launched_proxy_process);
  }

  gmm_set_status(launched_proxy_process);

  /* get hold of conf map shared memory */
  if (gmm_init_conf_struct() < 0)
    return gmm_init_error(attached_conf_struct);

  gmm_set_status(attached_conf_struct);

  /* create shared memmory for private maps */
  if (privmap_key > 0) {
    if (! (privmap = create_shmem(privmap_key, 2 * gmm_shm_->globalmap_size))) {
      if (debug)
	printf("Could not allocate shared memory for private map.\n");
      return gmm_init_error(created_privmap);
    }
    isol_privmap_key_ = privmap_key;
  } else
    /* try random keys */
  {
    if (! (privmap = create_shmem_rand(&privmap_key,
		          2 * gmm_shm_->globalmap_size))) {
      if (debug)
        printf("Could not allocate shared memory for private map.\n");
      return gmm_init_error(created_privmap);
    }
    else {
      isol_privmap_key_ = privmap_key;
    }
  }	/* end else (try random keys) */

  /* initialize pointers to private maps */
  gmm_privmap_ = (u32 *)privmap;
  gmm_mapmask_ = (u32 *)((char *) privmap + gmm_globalmap_size_);
  gmm_set_status(created_privmap);

  /* initialize isolate proxy */
  strncpy(packet.message.plc_init_req.owner, gmm_module_name_,
          GMM_OWNER_MAX_LEN);
  packet.message.plc_init_req.owner[GMM_OWNER_MAX_LEN] = '\0';
  strncat(packet.message.plc_init_req.owner, GMM_PROXY_MODULE_NAME_PREF,
          GMM_OWNER_MAX_LEN-strlen(packet.message.plc_init_req.owner));
  packet.message.plc_init_req.owner[GMM_OWNER_MAX_LEN] = '\0';
  packet.message.plc_init_req.priv_map_shmkey = privmap_key;
  packet.message.plc_init_req.conf_map_shmkey = cmm_plc_id();

  if (send_req_packet(rem_socket_, packet, type_plc_init_req, &req_id) < 0)
    return gmm_init_error(initialized_proxy);

  if (recv_rep_packet(rem_socket_, &packet, type_plc_init_rep, req_id) < 0)
    return gmm_init_error(initialized_proxy);

  if (packet.message.plc_init_rep.result < 0)
    return gmm_init_error(initialized_proxy);

  gmm_set_status(initialized_proxy);

  /* get current map from the shared memory */
  /* if (plc_update() < 0)                                          */
  /*   return gmm_init_error(gmm_fully_initialized);                */
  /* not required as we are using shared memory for private map,    */
  /* and the proxy will have called plc_update() on the same memory.*/

  /* setup the r/w mapmask */
  /* We can only do this after doing plc_update(), otherwise we shall      */
  /* overwrite on the globalmap all the points to which the current module */
  /* has write access (ownership)                                          */
  if (gmm_init_mapmask(gmm_module_name_, gmm_mapmask_, gmm_globalmap_size_) < 0)
    return gmm_init_error(gmm_fully_initialized);

  gmm_set_status(gmm_fully_initialized);

  return 0;
}


static int gmm_done_isolate()
{
  int res = 0;
  u32 req_id;
  plc_packet_t packet;

  if (debug)
    printf("gmm_done_isolate(): ...\n");

  gmm_rst_status(gmm_fully_initialized);

  /* shutdown isolate proxy */
  if (gmm_get_status(launched_proxy_process)) {
    if (send_req_packet(rem_socket_, packet, type_plc_done_req, &req_id) <
	0) res = -1;

    if (recv_rep_packet(rem_socket_, &packet, type_plc_done_rep, req_id) <
	0) res = -1;

    if (packet.message.plc_done_rep.result < 0)
      res = -1;

    client_close_connection(rem_socket_);
    rem_socket_ = -1;
    proxy_server_pid_ = -1;

    gmm_rst_status(launched_proxy_process);
    gmm_rst_status(initialized_proxy);
  }

  /* delete privmap shared memory */
  if (gmm_get_status(created_privmap)) {
    if (delete_shmem(isol_privmap_key_) < 0)
      res = -1;
    isol_privmap_key_ = -1;
    gmm_privmap_ = NULL;
/* TODO: how about the mapmask_ ??? */
    gmm_rst_status(created_privmap);
  }

  /* detach privmap shared memory */
  if (gmm_get_status(attached_privmap)) {
    if (detach_shmem(gmm_privmap_, 2 * gmm_shm_->globalmap_size) < 0)
      res = -1;
    isol_privmap_key_ = -1;
    gmm_privmap_ = NULL;
    gmm_rst_status(attached_privmap);
  }


  /* delete privmap malloced memory */
       /* NOTE: this doesn't really need to be here. Isolate never uses
        *       mallocs for the private map...
        */
  if (gmm_get_status(malloced_privmap)) {
    if (gmm_privmap_ != NULL) free(gmm_privmap_); 
    gmm_privmap_ = NULL;
    gmm_mapmask_ = NULL;
    gmm_rst_status(malloced_privmap);
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
