//
//    network.cpp - This is a driver for tcp/ip network communication
//
//    Copyright (C) 2000-2 by Hugh Jack <jackh@gvsu.edu>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//
// Last Modified: April 29, 2002
//
//
// SOCKET ROUTINES
//
// These routines will set up sockets for reading and writing across the
// internet. There are ten basic routines for reading and writing. These
// are used in one of two ways. In all cases the read routines are used to
// set up a socket which will remain open until all communications are done.
// 
// The write routines are used in one of two ways. The first way involves
// the continual opening and closing of the write sockets. The second way is
// to leave them open continuously.
//
// The two routines provided 'sk_read()' and sk_write()' are simple routines
// which must be proceeded by calls to 'sk_set_remote_host()',
// 'sk_set_local_host()' and 'sk_init_read()'.
//


// The obligatory Header Files

#include "network_io.h"

#include <sys/poll.h>
#include <sys/ioctl.h>
#include <linux/tcp.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>



//#define	DEBUG


network_io::network_io(){
	type = _READ;
	level = 0;
}

network_io::~network_io(){
	if(type == _READ){
		if(level > 1){
			if(level > 2){
				end_read_connection();
			}
			deinit_read();
		}
	} else if (type == _WRITE){

	}
}



/*
 * INIIALIZATION ROUTINES - set up structures, but do not use them.
 */


int network_io::set_remote_host(char *_host_name, int host_socket){
	static int 		error;
	static int		nm_a,
				nm_b,
				nm_c,
				nm_d;
	struct hostent	*hp;
//	struct hostent	*gethostbyname();
	unsigned char	address[4];

	error = NO_ERROR;
	strcpy(host_name, _host_name);
	host_socket_number = host_socket;

	// Set up server descriptor, get host reference and error trap
	write_connection.sin_family = AF_INET;
	if((host_name[0] > '9') ||(host_name[0]<'0')){
		hp = gethostbyname(host_name);
	} else {
		sscanf(host_name, "%d.%d.%d.%d",
			&nm_a, &nm_b, &nm_c, &nm_d);
		address[0] = (unsigned char)(nm_a);
		address[1] = (unsigned char)(nm_b);
		address[2] = (unsigned char)(nm_c);
		address[3] = (unsigned char)(nm_d);
		hp = gethostbyaddr((char *)address, 4, AF_INET);
	}

	if(hp != 0){
		/* complete descriptor set up. */
		bcopy((char *)hp->h_addr,
			(char *)&(write_connection.sin_addr),
			hp->h_length);
	} else {
		error_log(MINOR, "ERROR: unknown network host");
		error = ERROR;
	}

	return error;
}



int network_io::set_local_host(int socket_num)
{
	static int error;

	error = NO_ERROR;
	socket_number = socket_num;

	return error;
}






//
// THESE ROUTINES ARE USED FOR A SINGLE COMMUNICATION BETWEEN TWO CLIENTS
// The routines will make and break a connection. The sockets must also be
// initialized with the following routines.
//


int network_io::writer(char *string)
{
	static int 		error;

	error = NO_ERROR;

	/*
	 * Open a new socket for the write, and check for errors.
	 */
	error = init_write();
	if(error == NO_ERROR) error = open_write_connection();
	if(error == NO_ERROR) error = write_to_connection(string);
	if(error == NO_ERROR) error = end_write_connection();
	if(deinit_write() == ERROR) error = ERROR;

	return(error);
}



int network_io::reader(char *buf, int length)
{
	static int	error;		// Error return variable

	error = NO_ERROR;

	// Wait for a socket connection request, then get its reference.


	buf[0] = 0;

	if(wait_read_connection() == NO_ERROR){
//		if(read_stuff_waiting() == NO_ERROR){
			if(read_from_connection(buf, length) == ERROR){
				// buf[0] = 0;
			}
//		}
		// Close socket connection to remote write client.
		end_read_connection();
	}

	return(error);
}




//
//
// GLOBAL WRITE ROUTINES - All of the fundamental write functions are held here
//
//


int network_io::init_write()
{
	static int 		error;
	struct hostent		*gethostbyname();

	error = NO_ERROR;

	/* Open a new socket for the write, and check for errors. */
	if((rw_socket = socket(AF_INET, SOCK_STREAM, 0)) >= 0){
		write_connection.sin_port = htons(host_socket_number);
	} else {
		error_log(MINOR, "ERROR: opening stream socket");
		error = ERROR;
	}

	return error;
}




int network_io::open_write_connection(){
	static int error;

	error = NO_ERROR;
	//fcntl(rw_socket, F_SETFL, O_NONBLOCK);
	if(connect(rw_socket, (struct sockaddr *) &(write_connection), sizeof(write_connection)) < 0){
		error = ERROR;
		error_log(MINOR, "ERROR: Connecting stream Socket");
	}

	return error;
}




int network_io::write_to_connection(char *text){
	static int error;

	error = NO_ERROR;
// printf("[%s]\n", text);
	if(write(rw_socket, text, strlen(text) /* +1 */) < 0){
		error_log(MINOR, "ERROR: writing on stream socket");
		error = ERROR;
	}

	return error;
}





int network_io::write_stuff_done(){
	int 	error;
//	int	count;
//	struct pollfd ufds;

	error = NO_ERROR;
//	ufds.fd = rw_socket;
//	ufds.events = POLLOUT | POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
//	count = poll(&ufds, 0, 0);
//printf("WRITE COUNT %d  %d \n", count, ufds.revents);
//	if(count >= 0){
//		if((ufds.revents & POLLOUT) == 0){
//printf("Blocking %d \n", ufds.revents);
//sleep(1);
//			error = ERROR;
//		}
//	}

	return error;
}




int network_io::check_connection(){
	int 	error;
	int	count;
	struct pollfd ufds;

	error = NO_ERROR;
	ufds.fd = rw_socket;
	ufds.events = POLLOUT | POLLIN | POLLPRI | POLLERR | POLLHUP | POLLNVAL;
	count = poll(&ufds, 1, 0);
	if((ufds.revents & 16) != 0) error = ERROR;

	return error;
}


int network_io::end_write_connection(){
	static int 	error;

	error = NO_ERROR;

	return error;
}



int network_io::deinit_write(){
	static int error;

	error = NO_ERROR;
	close(rw_socket);
	rw_socket = ANY;
//printf("closing B %d \n", socket_number);


	return error;
}



//
//
// THESE ARE GLOBAL READ ROUTINES - These contain the five fundamental
// calls required for reading.
// 
//


//
// OPEN A SOCKET FOR READING
//
// This routine will open a socket, and prepare it for reading. A process
// must have its read socket ready before another process can communicate
// with it.
//
// It is therefore important to call this function before attempting any
// socket based I/O.
//
//
int network_io::init_read(){
	static int			error;	// low level socket number
	unsigned			length;	// temporary work variable
	static struct sockaddr_in	server; // read socket descriptor
	static struct hostent		*hp;
	char			text[100];
	int				val;

	// Open internet socket, and check for error.
	error = ERROR;
	gethostname(text, 100);           /* who are we? */
	hp = gethostbyname(text); 
	if(hp == NULL){
		error_log(MINOR, "Could not look up host name, trying localhost");
		hp = gethostbyname("localhost");
	}
	if(hp == NULL){
		error_log(MINOR, "could not look up locahost, trying 127.0.0.1");
		hp = gethostbyname("127.0.0.1");
	}
	#ifdef DEBUG 
		printf("INITREAD  [%s] %d\n", text, socket_number);
	#endif
	if((hp != NULL) && (read_socket = socket(AF_INET, SOCK_STREAM, 0)) >= 0){
		// Set up server descriptor for binding name.
		memset(&server, 0, sizeof(struct sockaddr_in));
		server.sin_family = hp->h_addrtype;
//		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(socket_number);
		// Bind the socket, and check for error.
		level = 1;
		int flag = 1;
		val = setsockopt(read_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));
		#ifdef DEBUG
			printf("boo 1:   %d, %d, %d, %d, %d, %d\n", errno, EBADF, ENOTSOCK, ENOPROTOOPT, EFAULT, val);
		#endif
		if(bind(read_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) >= 0){

			// Check for valid socket binding
			length = sizeof(server);
			if(getsockname(read_socket, (struct sockaddr *)&server, &length) >= 0){
				error = NO_ERROR;
				// Set up variables for success
				// Zero because anything higher would allow
				// messages to arrive out of sequence.
				listen(read_socket, 0);
			} else {
				error_log(MINOR, "ERROR: getting socket name");
			}
		} else {
			error_log(MINOR, "ERROR: binding stream socket");
			#ifdef DEBUG
				printf("Socket number %d, %d \n", socket_number, read_socket);
			#endif
		}
	} else {
		error_log(MINOR, "ERROR: opening stream socket");
	}

	return(error);
}





int network_io::read_stuff_waiting(){
	int 	error,
		count;
	struct pollfd ufds;

	error = ERROR;
	ufds.fd = read_socket;
	ufds.events = POLLIN | POLLPRI | POLLOUT;
	count = poll(&ufds, 1, 0);
//printf("READ COUNT %d  %d  %d \n", count, ufds.events, ufds.revents);
	if((ufds.revents & 1) > 0){
		error = NO_ERROR;
	}

	return error;
}



int network_io::wait_read_connection(){
	static int 	error;
	unsigned	size;
	static struct sockaddr addr;

	error = NO_ERROR;
	size = sizeof(struct sockaddr);
	fcntl(read_socket, F_SETFL, O_NONBLOCK);
	rw_socket = accept(read_socket, &addr, &size);

	level = 2;
	if(rw_socket < 0){
		error = ERROR;
		// error_log("ERROR: warning:accept");
	}

	return error;
}



int network_io::read_from_connection(char *buf, int length){
	int	error;
	int	len;

	error = NO_ERROR;
	// Empty input buffer
	buf[0] = 0;
	// Read string into buffer from socket
	fcntl(rw_socket, F_SETFL, O_NONBLOCK);
	len = read(rw_socket, buf, length);
	if(len < 0){
		// error_log("ERROR: reading stream message");
		// error = ERROR;
		if(errno != 11){
//static int beeper = 0;
//if(beeper == 0)
			printf("errno=%d  ", errno);
//beeper++; if(beeper > 2000) beeper = 0;
		}
//		printf("  (%d, %d, %d, %d, %d, %d, %d)\n", EINTR, EAGAIN, EIO, EISDIR, EBADF, EINVAL, EFAULT);
	} else {
		buf[len] = 0;
//printf("incoming [%s] \n", buf);
	}

	return error;
}




int network_io::end_read_connection()
{
	int	error;
	int	a;

	error = NO_ERROR;
	// fcntl(rw_socket, TCP_TIME_WAIT, 30000);
	a = close(rw_socket);
	//b = shutdown(rw_socket, 2);
	// b = close(rw_socket);
	level = 1;
// printf("closing C %d, %d \n", socket_number, a);

	return error;
}



//
// CLOSE A SOCKET FOR READING
//
// This routine will close a socket that was opened for reading. This should
// only be done after all reading is complete.
//
// VARIABLES:	sock - The socket number opened for reading.
//
// RETURNS:	return() - NO_ERROR (at present).
//
int network_io::deinit_read(){
	int	error;
	int	a;

	error = NO_ERROR;

	// fcntl(read_socket, TCP_FIN_WAIT1, 0);
	// fcntl(read_socket, TCP_TIME_WAIT, 1);
	a = close(read_socket);
	// b = shutdown(read_socket, 2);
	// b = close(read_socket);

	level = 0;
// printf("closing  D %d, %d, \n", socket_number, a);

	return error;
}



//
//
// LEVEL GLOBAL ROUTINES FOR NON-READ/WRITE FUNCTIONS
//
//

//
// GET IP ADDRESS OF LOCAL MACHINE
//
// This function will query the operating system to find its IP address.
// This routine works for both SUN and SGI machines.
//
// Returns:	A string which holds the numeric IP address
//
char *network_io::get_address(){
	static char		work[MAXIMUM_HOST_NAME_LENGTH];
	struct sockaddr_in	address;
	int			i,
				addr[4];
	long 			int_address;
#ifndef SGI
	// Sun Version
	get_myaddress(&address);
	int_address = address.sin_addr.s_addr;
#else
	// SGI Version
	int_address = gethostid();
#endif
	// SUN & SGI version
	for(i = 0; i < 4; i++){
		addr[i]=int_address & 0xFF;
		int_address >>= 8;
	}
#ifdef OTHER_UNIX
	sprintf(work, "%d.%d.%d.%d",
		(int)addr[3], (int)addr[2], (int)addr[1], (int)addr[0]);
#else
	// This is for linux
	sprintf(work, "%d.%d.%d.%d",
		(int)addr[0], (int)addr[1], (int)addr[2], (int)addr[3]);
#endif 
	return work;
}



char *network_io::get_remote_client(){
	static	char		work[100];
	struct sockaddr		address;
	socklen_t			len;

	len = sizeof(address);
	if(getpeername(rw_socket, &address, &len) == 0){
//	if(getsockname(rw_socket, (struct sockaddr *)work, &len) == 0){
		sprintf(work, "%u.%u.%u.%u", (unsigned short)address.sa_data[2], (unsigned short)address.sa_data[3], (unsigned short)address.sa_data[4], (unsigned short)address.sa_data[5]);
		// printf("Got address [%s]\n", work);
		//strcpy(work, address);
	} else {
		strcpy(work, "unknown network address");
	}

	return work;
}

