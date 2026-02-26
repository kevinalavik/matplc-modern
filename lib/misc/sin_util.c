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
 * Socket INET utility routines
 *
 * This file implements the routines in sin_util.h
 *
 * These routines merely make life simpler when working with
 * internet protocol sockets
 */



#include "sin_util.h"



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>   /* required for strcmp() */
#include <strings.h>   /* required for bzero(), bcopy() */






#define ERR_EXIT(mesg, err_num) {perror(mesg);return err_num;}
#define MSG_EXIT(mesg, err_num) {fprintf(stderr, "%s\n", mesg);return err_num;}


int sin_init_dot_addr(struct sockaddr_in *sad_in,
                  char *dot_addr,
                  unsigned short int port)
{
sad_in->sin_family = AF_INET;

if( inet_aton(dot_addr, &(sad_in->sin_addr)) == 0 )
   ERR_EXIT("inet_aton", -1);

sad_in->sin_port = htons(port);

return 1;
}  /* sin_init_dot_addr */





int sin_init_addr(struct sockaddr_in *addr,
                  const char *host,
                  const char *service,
                  const char *protocol)
/* initialize an address structure */
{
int port_flag;
long int tmp_numb;
long long int lli = 1;
char *error_char;

struct servent *serv_entry_ptr;
struct hostent *host_entry_ptr;

bzero ((char *)addr, sizeof(*addr));
addr->sin_family = AF_INET;

/* Map service name to port number */
port_flag = 0;
if (port_flag == 0)
 if ((service == NULL) || (strcmp(service, "") == 0))
   {addr->sin_port = htons(0); /* OS will sugest a port when binding */
    port_flag = 1;
   }

if (port_flag == 0)
   if ((serv_entry_ptr = getservbyname (service, protocol)) )
      {addr->sin_port = serv_entry_ptr->s_port;
       port_flag = 1;
      }

if (port_flag == 0)
   {tmp_numb = strtol(service, &error_char, 0);
    if ((*error_char == '\0') && (error_char != service) && 
        (tmp_numb >= 0) && (tmp_numb < (lli << (8*sizeof(addr->sin_port)))))
       {addr->sin_port = htons(tmp_numb);
        port_flag = 1;
       }
   }

if (port_flag == 0)
   MSG_EXIT("sin_init_addr: Could not determine the port number.", -1);
 
/* Map host name to IP address, allowing dotted decimal */
addr->sin_addr.s_addr = INADDR_NONE;
 
if (addr->sin_addr.s_addr == INADDR_NONE)
   if ( (host_entry_ptr = gethostbyname (host)) )
      bcopy (host_entry_ptr->h_addr, 
             (char *)&(addr->sin_addr), 
             host_entry_ptr->h_length);

if (addr->sin_addr.s_addr == INADDR_NONE)
   inet_aton (host, (struct in_addr *)addr);  

if (addr->sin_addr.s_addr == INADDR_NONE)
   MSG_EXIT("sin_init_addr: Could not determine the host IP address.", -1);

return 1;
} /* sin_init_addr(...) */




int sin_init_proto(int *type,
                   int *protocol_num,
                   const char *protocol)
{
struct protoent *proto_entry_ptr;

/* determine the protocol */
*type = SOCK_RDM;
if (strcasecmp (protocol, "udp") == 0) *type = SOCK_DGRAM;
if (strcasecmp (protocol, "tcp") == 0) *type = SOCK_STREAM;
if (strcasecmp (protocol, "raw") == 0) *type = SOCK_RAW;
if (*type == SOCK_RDM)
   MSG_EXIT ("sin_init_proto: protocol not supported.", -1);

/* map protocol name to protocol number */
if ( (proto_entry_ptr = getprotobyname (protocol)) == 0 )
   MSG_EXIT ("sin_init_proto: can't get protocol number.", -1);
*protocol_num = proto_entry_ptr->p_proto;

return 1;
} /* sin_init_proto(...) */


int bind_socket(const char *host,
                const char *service,
                const char *protocol)
/* create a socket and bind to a local port */
{
struct sockaddr_in sock_addr;
int socket_id, type, protocol_num;

/* initialize the sock_addr sturcture */
if ( (sin_init_addr (&sock_addr, host, service, protocol)) < 0)
   MSG_EXIT ("bind_socket: wrong address format", -1);

/* determine the protocol */
if ( (sin_init_proto (&type, &protocol_num, protocol)) < 0)
   MSG_EXIT ("bind_socket: wrong protocol format", -1);

/* create the socket */
if ( (socket_id = socket (PF_INET, type, protocol_num)) < 0)
   ERR_EXIT ("socket", -1);

/* bind the socket */
if ( bind (socket_id, (struct sockaddr *)&sock_addr, sizeof (sock_addr)) < 0)
   ERR_EXIT ("bind", -1);

return socket_id;
} /* bind_socket(...) */



int connect_socket(const char *host,
                   const char *service,
                   const char *protocol)
/* create a socket and connect to a remote host */
{
struct sockaddr_in sock_addr;
int socket_id, type, protocol_num;

/* initialize the sock_addr sturcture */
if ( (sin_init_addr (&sock_addr, host, service, protocol)) < 0)
   MSG_EXIT ("bind_socket: wrong address format", -1);

/* determine the protocol */
if ( (sin_init_proto (&type, &protocol_num, protocol)) < 0)
   MSG_EXIT ("bind_socket: wrong protocol format", -1);

/* create the socket */
if ( (socket_id = socket (PF_INET, type, protocol_num)) < 0)
   ERR_EXIT ("socket", -1);

/* bind the socket */
if ( connect(socket_id, (struct sockaddr *)&sock_addr, sizeof (sock_addr)) < 0)
   ERR_EXIT ("bind", -1);

return socket_id;
} /* connect_socket(...) */



#ifndef socklen_t
/* At least in QNX this doesn't get defined! */
typedef unsigned int socklen_t;
#endif

int get_socket_port(int sock)
/* returns the port to which the socket is bound */ 
{
 socklen_t length;
 struct sockaddr_in srv_addr;  

 length = sizeof(srv_addr);
 if (getsockname(sock, (struct sockaddr *)&srv_addr, &length) < 0)
   return -1;

 if (srv_addr.sin_family == AF_INET)
   return ntohs(srv_addr.sin_port);

 return -1; /* not INET socket... */
} /* get_socket_port(...) */
