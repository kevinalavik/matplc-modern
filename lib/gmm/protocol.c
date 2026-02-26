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


/*
 * Protocol Implementation
 *
 * This file implements the routines in protocol.h
 *
 * These routines implement the communication protocol
 * between the gmm manager and the plcproxy server
 * when the --PLCisolate option is used
 */




#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>


#include <misc/sin_util.h>

#include "protocol.h"


static int prot_debug = 0;

static u32  next_request_id = 0;
static u8   negotiated_ver_major = 0;
static u8   negotiated_ver_minor = 0;

static const u8 known_ver_major = 1;
static const u8 known_ver_minor = 0;




static inline void init_rep_packet(plc_packet_t *packet, u16 mesg_type)
{
  packet->magic = PLC_PACKET_MAGIC;
  packet->ver_major = negotiated_ver_major;
  packet->ver_minor = negotiated_ver_minor;
  packet->message_type = (mesg_type);
  packet->crc = 0;
  /*(packet).packet_id = ...; */
  /* initialized inside send_packet function. */
}

static inline void init_req_packet(plc_packet_t * packet,
				   u16 mesg_type, u32 * this_request_id)
{
  init_rep_packet(packet, mesg_type);
  packet->request_id = *this_request_id = next_request_id++;
}

static inline int check_req_packet(plc_packet_t packet)
{
  return (packet.magic != PLC_PACKET_MAGIC)
      || (packet.ver_major != negotiated_ver_major)
      || (packet.ver_minor != negotiated_ver_minor)
      || (packet.crc != 0);
}


static inline int check_rep_packet(plc_packet_t packet, u16 mesg_type,
				   u32 req_id)
{
  return check_req_packet(packet)
      || (packet.message_type != (mesg_type))
      || (packet.request_id != (req_id));
}


/* forward declarations */
static int recv_packet_impl(int sockid, 
                            plc_packet_t *packet);

static int send_packet_impl(int sockid, 
                            plc_packet_t packet);






int client_establish_connection(const char *host, const char *serv)
/* returns socket_id of established connection */
/* returns -1 if not succesfull                */
{
  int rem_socket = 0;
  plc_packet_t packet;
  u32 req_id;

  if (prot_debug)
     printf("client_establish_connection(): host=%s, serv=%s...\n",
            host, serv);

  if ((rem_socket = connect_socket (host, serv, "tcp")) < 0) {
    if (prot_debug) printf("Error connecting to the MatPLC proxy server.\n");
    return -1;
    }

/* negotiate protocol versions */
     /* our lowest known version */
  packet.message.plc_proto_ver_req.min_ver_major = known_ver_major;
  packet.message.plc_proto_ver_req.min_ver_minor = known_ver_minor;
     /* our highest known version */
  packet.message.plc_proto_ver_req.max_ver_major = known_ver_major;
  packet.message.plc_proto_ver_req.max_ver_minor = known_ver_minor;

  if (send_req_packet (rem_socket, packet, type_plc_proto_ver_req, &req_id) < 0)
    {close (rem_socket);
     rem_socket = 0;
     return -1;
    }

  if (recv_rep_packet (rem_socket, &packet, type_plc_proto_ver_rep, req_id) < 0)
    {close (rem_socket);
     rem_socket = 0;
     return -1;
    }

  negotiated_ver_major = packet.message.plc_proto_ver_rep.ver_major;
  negotiated_ver_minor = packet.message.plc_proto_ver_rep.ver_minor;

  return rem_socket;
}




int client_close_connection(int sockid)
{ return close(sockid); };





int server_establish_connection(int sockid)
{
 plc_packet_t packet;

/* negotiate protocol versions */
  if (recv_req_packet (sockid, &packet) < 0 )
     return -1;

  if (packet.message_type != type_plc_proto_ver_req)
     return -1;

     /* our only known version must be between the two proposed limits */
  if ((packet.message.plc_proto_ver_req.min_ver_major == known_ver_major) &&
      (packet.message.plc_proto_ver_req.min_ver_minor == known_ver_minor) &&
      (packet.message.plc_proto_ver_req.max_ver_major >= known_ver_major) && 
#if known_ver_minor==0
      1 /* avoid warning "comparison is always true" */
#else
      (packet.message.plc_proto_ver_req.max_ver_minor >= known_ver_minor)
#endif
     )
    /* the highest commonly supported protocol */
    {packet.message.plc_proto_ver_rep.ver_major = known_ver_major;
     packet.message.plc_proto_ver_rep.ver_minor = known_ver_minor;
    }
    else
    /* no commonly supported protocol */
    {packet.message.plc_proto_ver_rep.ver_major = 0;
     packet.message.plc_proto_ver_rep.ver_minor = 0;
    }

  if (send_rep_packet (sockid, packet, type_plc_proto_ver_rep) < 0)
     return -1;

  /* must only establish the negotiated protocol version after sending */
  /* the plc_proto_ver_rep packet. This packet must go with            */
  /* ver_major = ver_minor = 0                                         */
  negotiated_ver_major = known_ver_major;
  negotiated_ver_minor = known_ver_minor;

  return 0;
};









int recv_req_packet(int sockid, 
                    plc_packet_t *packet)
{
 if (recv_packet_impl(sockid, packet) < 0)
  return -1;

 if (check_req_packet(*packet))
   return -1;

 if (prot_debug) printf("recv_req_packet(): received OK!\n");

 return 0;
}




int recv_rep_packet(int sockid, 
                    plc_packet_t *packet,
                    u16 mesg_type,
                    u32 req_id )
{
 if (recv_packet_impl(sockid, packet) < 0)
  return -1;

 if (check_rep_packet(*packet, mesg_type, req_id))
   return -1;

 return 0;
}




static int recv_packet_impl(int sockid, 
                     plc_packet_t *packet)
{
 int offset, size, res;

 offset = 0;
 size = sizeof (*packet);

 while (offset < size)
  {res = read(sockid, (void *)((char *)(packet) + offset), size - offset);
   if (res >= 0) {offset += res; continue;}
   /* error ? */
   if ((errno == EAGAIN) || (errno == EINTR)) {continue;}
   /* real error */
   return -1;
  }

 /* if we ever use TCP/IP sockets, then this is the place    */
 /*  where we should do the marshalling of the data stream.  */
 /*  marshalling = htons() ntohs() htonl() ntohl() ...       */

 return 0;
};




int send_req_packet(int sockid, 
                    plc_packet_t packet,
                    u16 mesg_type,
                    u32 *req_id )
{
 init_req_packet(&packet, mesg_type, req_id);

 return send_packet_impl(sockid, packet);
}



int send_rep_packet(int sockid, 
                    plc_packet_t packet,
                    u16 mesg_type)
{
 init_rep_packet(&packet, mesg_type);

 return send_packet_impl(sockid, packet);
}




static int send_packet_impl(int sockid, 
                     plc_packet_t packet)
{
 int res, offset, size;


 /* if we ever use TCP/IP sockets, then this is the place    */
 /*  where we should do the marshalling of the data stream.  */
 /*  marshalling = htons() ntohs() htonl() ntohl() ...       */

 offset = 0;
 size = sizeof (packet);

 while (offset < size)
  {res = write(sockid, (void *)((char *)(&packet) + offset), size - offset);
   if (res >= 0) {offset += res; continue;}
   /* error ? */
   if ((errno == EAGAIN) || (errno == EINTR)) {continue;}
   /* real error */
   return -1;
  }

 if (prot_debug) printf("send_packet_impl(): sent %d bytes\n", offset);

 return 0;
};


