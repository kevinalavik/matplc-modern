/*
 * (c) 2000 Mario de Sousa
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


#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include "types.h"
#include "gmm_private.h" /* needed for plc_pt_priv_t */





#define DEF_REMOTE_HOST "localhost"
#define DEF_REMOTE_SERV "64458"




/************************************************************************/
/*                                                                      */
/*          Data members used for IPC between gmm server and proxy.     */
/*                                                                      */
/************************************************************************/


/*******************************/
typedef struct {
  u8 max_ver_major; /* highest protocol version supported by client */
  u8 max_ver_minor;
  u8 min_ver_major; /* lowest protocol version supported by client */
  u8 min_ver_minor;
} plc_proto_ver_req_t;

typedef struct {
  u8 ver_major; /* protocol version accepted by server       */
  u8 ver_minor; /*  if none accepted, then major = minor = 0 */
} plc_proto_ver_rep_t;


/*******************************/
typedef struct {
  char owner[GMM_OWNER_MAX_LEN + 1]; 
  u32  priv_map_shmkey;
  u32  conf_map_shmkey;
} plc_init_req_t;

typedef struct {
  i32 result;
} plc_init_rep_t;


/*******************************/
typedef struct {
  u8  dummy;
} plc_done_req_t;

typedef struct {
  i32 result;
} plc_done_rep_t;


/*******************************/
typedef struct {
  char name[GMM_NAME_MAX_LEN + 1]; 
} plc_pt_by_name_req_t;

typedef struct {
  i16 magic;
  i16 ofs;
  u32 mask;
} plc_pt_by_name_rep_t;


/*******************************/
typedef struct {
  u8 dummy;
} plc_update_req_t;

typedef struct {
  i32 result;
} plc_update_rep_t;


/*******************************/
typedef struct {
  plc_pt_priv_t plc_pt;
} plc_update_point_req_t;

typedef struct {
  i32 result;
} plc_update_point_rep_t;


/*******************************/
#define __PLC_PROTO_MAX_NUM_POINTS 8
typedef struct {
  plc_pt_priv_t plc_pt[__PLC_PROTO_MAX_NUM_POINTS];
  u32 count;
} plc_update_points_req_t;

typedef struct {
  i32 result;
} plc_update_points_rep_t;


/*******************************/

typedef union {
  plc_proto_ver_req_t   plc_proto_ver_req;
  plc_proto_ver_rep_t   plc_proto_ver_rep;

  plc_init_req_t        plc_init_req;
  plc_init_rep_t        plc_init_rep;

  plc_done_req_t        plc_done_req;
  plc_done_rep_t        plc_done_rep;

  plc_pt_by_name_req_t  plc_pt_by_name_req;
  plc_pt_by_name_rep_t  plc_pt_by_name_rep;

  plc_update_req_t      plc_update_req;
  plc_update_rep_t      plc_update_rep;

  plc_update_point_req_t   plc_update_point_req;
  plc_update_point_rep_t   plc_update_point_rep;

  plc_update_points_req_t   plc_update_points_req;
  plc_update_points_rep_t   plc_update_points_rep;
} message_t;


/*
typedef enum {
  type_plc_proto_ver_req;
  type_plc_proto_ver_rep;

  type_plc_init_req;
  type_plc_init_rep;

  type_plc_done_req;
  type_plc_done_rep;

  type_plc_pt_by_name_req,
  type_plc_pt_by_name_rep,

  type_plc_update_req,
  type_plc_update_rep,

  type_plc_update_point_req,
  type_plc_update_point_rep

  type_plc_update_points_req,
  type_plc_update_points_rep
} message_type_t;
*/


#define type_plc_proto_ver_req    0x0000
#define type_plc_proto_ver_rep    0x0001

#define type_plc_init_req         0x0002
#define type_plc_init_rep         0x0003

#define type_plc_done_req         0x0004
#define type_plc_done_rep         0x0005

#define type_plc_pt_by_name_req   0x0006 
#define type_plc_pt_by_name_rep   0x0007

#define type_plc_update_req       0x0008
#define type_plc_update_rep       0x0009

#define type_plc_update_point_req  0x000A
#define type_plc_update_point_rep  0x000B
 
#define type_plc_update_points_req 0x000C
#define type_plc_update_points_rep 0x000D




typedef struct {
  u16 magic;
  u8  ver_major;
  u8  ver_minor;
  u16 message_type;
  u16 crc;
  u32 request_id;
  u32 packet_id;
  message_t message;
} plc_packet_t;


static const u16 PLC_PACKET_MAGIC = 0xB38A;


int recv_rep_packet (int sockid, 
                     plc_packet_t *packet,
                     u16 mesg_type,
                     u32 req_id);

int recv_req_packet (int sockid, 
                     plc_packet_t *packet);

int send_rep_packet (int sockid, 
                     plc_packet_t  packet,
                     u16 mesg_type);

int send_req_packet (int sockid, 
                     plc_packet_t  packet,
                     u16 mesg_type,
                     u32 *req_id);

int client_establish_connection(const char *host, const char *serv);
int client_close_connection(int sockid);

int server_establish_connection(int sockid);


#endif /* __PROTOCOL_H */
