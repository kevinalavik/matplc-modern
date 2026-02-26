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


#ifndef __PLC_PROXY_H
#define __PLC_PROXY_H


int launch_proxy_server(const char *service_port, 
                        int *server_pid, 
                        int *rem_socket);
/* returns pid of child process if succesfull */
/* returns -1 if unsuccesfull                 */


int proxy_server_factory(const char *service_port);
/*
 * A proxy server factory implementation.
 * This factory listens on a service_port and when
 * a connection request arrives it creates
 * a child proxy_server process
 * to handle that connection.
 */


int connect_to_proxy_server_factory(const char * remote_plc_host,
                                    const char * remote_plc_serv);
/*
 * Returns the socket for the connection if succesfull.
 * Returns -1 if unsuccesfull.
 */



#endif /* __PLC_PROXY_H */
