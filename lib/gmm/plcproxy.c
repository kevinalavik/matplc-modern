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
 * MatPLC proxy server library
 *
 * This file implements the routines defined in plcproxy.h
 * 
 * These routines are used to implement the proxy server
 * required when using --PLCisolate
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>  /* random seed generator */
#include <errno.h>
#include <sched.h>
#include <string.h>  /* required for strncpy() */

#include "plc.h"
#include <misc/sin_util.h>
#include "protocol.h"

#include "plcproxy.h"

static int proxy_debug = 0;


const static int QUEUE_SIZE=4;
const static int _plcproxy_max_buf_size_=128;


int handle_init(int com_socket, plc_packet_t *packet)
{
 char *my_argv[3];
 char buffer0[_plcproxy_max_buf_size_];
 char buffer1[_plcproxy_max_buf_size_];
 char buffer2[_plcproxy_max_buf_size_];
 int my_argc = 0;
 int res;
 u32 priv_map_shmkey, conf_map_shmkey;

 if (proxy_debug) printf("handle_init()...\n");

 my_argv[0] = buffer0;
 my_argv[1] = buffer1;
 my_argv[2] = buffer2;
 buffer0[0] = '\0';
 buffer1[0] = '\0';
 buffer2[0] = '\0';
 
 priv_map_shmkey = (*packet).message.plc_init_req.priv_map_shmkey;
 conf_map_shmkey = (*packet).message.plc_init_req.conf_map_shmkey;

 strncpy (my_argv[0], CLO_loc_local, _plcproxy_max_buf_size_);
 my_argc = 1;

 if (priv_map_shmkey > 0) {
   strncpy (my_argv[my_argc], CLO_privmap_key, _plcproxy_max_buf_size_);
   snprintf (my_argv[my_argc]+strlen(my_argv[my_argc]), 
             _plcproxy_max_buf_size_ - strlen (my_argv[my_argc]),
             "%d",
             priv_map_shmkey);
   my_argc++;
   }

 if (conf_map_shmkey > 0) {
   strncpy (my_argv[my_argc], CLO_plc_id, _plcproxy_max_buf_size_);
   snprintf (my_argv[my_argc]+strlen(my_argv[my_argc]), 
             _plcproxy_max_buf_size_ - strlen (my_argv[my_argc]),
             "%d",
             conf_map_shmkey);
   my_argc++;
   }

 res = plc_init ((*packet).message.plc_init_req.owner, my_argc, my_argv);

 (*packet).message.plc_init_rep.result = res;

 if (send_rep_packet (com_socket, *packet, type_plc_init_rep) < 0) return -1; 

 return res;
}





int handle_done(int com_socket, plc_packet_t *packet)
{
 int res;

 if (proxy_debug) printf("handle_done()...\n");

 res = plc_done();

 (*packet).message.plc_done_rep.result  = res;

 if (send_rep_packet (com_socket, *packet, type_plc_done_rep) < 0)
   return -1;

 exit (EXIT_SUCCESS);

 /*return 0;*/
}







int handle_pt_by_name(int com_socket, plc_packet_t *packet)
{
 plc_pt_t plc_pt;

 if (proxy_debug) printf("handle_pt_by_name()...\n");

 plc_pt = plc_pt_by_name((*packet).message.plc_pt_by_name_req.name);

 (*packet).message.plc_pt_by_name_rep.magic = ((plc_pt_priv_t *)&plc_pt)->magic;
 (*packet).message.plc_pt_by_name_rep.ofs   = ((plc_pt_priv_t *)&plc_pt)->ofs;
 (*packet).message.plc_pt_by_name_rep.mask  = ((plc_pt_priv_t *)&plc_pt)->mask;

 if (send_rep_packet (com_socket, *packet, type_plc_pt_by_name_rep) < 0) 
   return -1; 

 return 0;
}



int handle_update(int com_socket, plc_packet_t *packet)
{
 int res;

 if (proxy_debug) printf("handle_update()...\n");

 res = plc_update();

 (*packet).message.plc_update_rep.result  = res;
 
 if (send_rep_packet (com_socket, *packet, type_plc_update_rep) < 0)
   return -1;

 return 0;
}



int handle_update_point(int com_socket, plc_packet_t *packet)
{
 int res;

 if (proxy_debug) printf("handle_update_point()...\n");

 res = plc_update_pt(
        (*(plc_pt_t *)&((*packet).message.plc_update_point_req.plc_pt)));

 (*packet).message.plc_update_rep.result = res;

 if (send_rep_packet (com_socket, *packet, type_plc_update_point_rep) < 0)
   return -1;

 return 0;
}




int handle_update_points(int com_socket, plc_packet_t *packet)
{
 int res;

 if (proxy_debug) printf("handle_update_points()...\n");

 res = plc_update_pts(
        (plc_pt_t *)&((*packet).message.plc_update_points_req.plc_pt),
        (*packet).message.plc_update_points_req.count);

 (*packet).message.plc_update_points_rep.result = res;

 if (send_rep_packet (com_socket, *packet, type_plc_update_points_rep) < 0)
   return -1;

 return 0;
}




int handle_requests(int com_socket)
{
 plc_packet_t packet;
 int res;
 
 if (proxy_debug) printf("handle_requests()...\n");

 while (1) {
  if (recv_req_packet(com_socket, &packet) < 0) continue;

  res = 0;
  switch (packet.message_type) {
   case type_plc_init_req:
        {res = handle_init (com_socket, &packet); break;}
   case type_plc_done_req: 
        {res = handle_done (com_socket, &packet); break;}
   case type_plc_pt_by_name_req: 
        {res = handle_pt_by_name (com_socket, &packet); break;}
   case type_plc_update_req: 
        {res = handle_update (com_socket, &packet); break;}
   case type_plc_update_point_req: 
        {res = handle_update_point (com_socket, &packet); break;}
   case type_plc_update_points_req: 
        {res = handle_update_points (com_socket, &packet); break;}
   default: continue;
   } /* switch */ 
 
 } /* while(1) */
}



int launch_proxy_server(const char *service_port, 
                        int *server_pid, 
                        int *rem_socket)
/* returns pid of child process if succesfull */
/* returns -1 if unsuccesfull                 */
/* 
 * Even though this function launches a proxy server
 * process that will work as a server in a client/server
 * relantionship, it is the client that sets up a
 * socket to wait for a connection request, while
 * the server establishes the connection with 
 * the client. The roles have been reversed to 
 * allow the client (parent process) to choose
 * a random connection port.
 *
 */
{
 char serv[_plcproxy_max_buf_size_];
 int con_port, con_socket, com_socket;
 int child_pid = -1;

 if (proxy_debug) printf("launch_proxy_server()...\n");

 /* setup listening socket */
 if ((con_socket = bind_socket("localhost", service_port, "tcp")) < 0)
   return -1;

 if (listen (con_socket, QUEUE_SIZE) < 0) 
   {close(con_socket); return -1;}

 if ((con_port = get_socket_port(con_socket)) < 0) 
   {close(con_socket); return -1;}

 if (snprintf (serv, _plcproxy_max_buf_size_, "%d", con_port) < 0)
   {close(con_socket); return -1;}

 if ((child_pid = fork()) < 0)
   {close(con_socket); return -1;}

 if (child_pid != 0)
   /* parent process */
   {
    if ((*rem_socket = accept(con_socket, NULL, NULL)) < 0)
      {close(con_socket); return -1;}
    if (server_establish_connection (*rem_socket) < 0 ) 
      {close(*rem_socket); close(con_socket); return -1;}
    *server_pid = child_pid;
    return child_pid;
   }
  else
   /* child process */
   {
    close(con_socket); /* child won't need this socket... */
    com_socket = client_establish_connection ("localhost", serv);
    if (com_socket >= 0 ) {
      /* undo any partial setup of the plc by the parent process */
      plc_done();
      if (handle_requests(com_socket) == 0)
        exit (EXIT_SUCCESS);
    }

    exit (EXIT_FAILURE);
   }

 return -1; /* should never reach this point */
}



int proxy_server_factory(const char *service_port)
/*
 * A proxy server factory implementation.
 * This factory listens on a service_port and when
 * a connection request arrives it creates
 * a child proxy_server process
 * to handle that connection.
 */
{
 int list_sock, com_sock, child_pid;

 if (proxy_debug) printf("proxy_server_factory()...\n");

 if (strcmp(service_port, "") == 0)
   service_port = DEF_REMOTE_SERV;

 /* setup listening socket */
 if ( (list_sock = bind_socket ("localhost", service_port, "tcp")) < 0)
   return -1;

 if (listen (list_sock, QUEUE_SIZE) < 0)
   return -1;

 while (1) {
   /* wait for connection request */
   if ( (com_sock = accept(list_sock, NULL, NULL)) < 0)
     return -1;

  /* launch server process */
   if ((child_pid = fork()) < 0)
     return -1;

   if (child_pid == 0)
     /* child process */
     {
      if (server_establish_connection (com_sock) < 0) {
        close (com_sock);
        exit (EXIT_FAILURE);
      }

      if (handle_requests(com_sock) == 0)
        exit (EXIT_SUCCESS);

      exit (EXIT_FAILURE);
     }

 }  /* while (1) */

 return -1;
 
}


int connect_to_proxy_server_factory(const char * remote_plc_host,
                                    const char * remote_plc_serv)
/*
 * Returns the socket for the connection if succesfull.
 * Returns -1 if unsuccesfull.
 */
{
 int com_sock;

 if (proxy_debug) printf("connect_to_proxy_server_factory()...\n");

  if (strcmp(remote_plc_host, "") == 0) 
{
 if (proxy_debug) printf("connect_to_proxy_server_factory(): 1...\n");
    remote_plc_host = DEF_REMOTE_HOST;
}

 if (proxy_debug) printf("connect_to_proxy_server_factory(): 1...\n");

  if (strcmp(remote_plc_serv, "") == 0)
    remote_plc_serv = DEF_REMOTE_SERV;

 if (proxy_debug) printf("connect_to_proxy_server_factory(): 2...\n");

  com_sock = client_establish_connection (remote_plc_host, remote_plc_serv);

  return com_sock;
}
