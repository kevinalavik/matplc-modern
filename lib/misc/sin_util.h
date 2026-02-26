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


#ifndef SIN_UTIL_H
#define SIN_UTIL_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



int sin_init_addr(struct sockaddr_in *addr,
                  const char *host,
                  const char *service,
                  const char *protocol);
/* initialize an address structure */


int bind_socket(const char *host,
                const char *service,
                const char *protocol);
/* create a socket and bind to a local port */


int connect_socket(const char *host,
                   const char *service,
                   const char *protocol);
/* create a socket and connect to a remote host */


int get_socket_port(int sock);
/* returns the port to which the socket is bound */

#endif
