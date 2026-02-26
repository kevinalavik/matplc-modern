//
//        Function Names and definitions for network.cpp
//        These deal with the lowest level of socket interface.
//
//    Copyright (C) 2000-1 by Hugh Jack <jackh@gvsu.edu>
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
// Last Modified: January 15, 2001
//


#ifndef _NETWORK_IO
#define _NETWORK_IO


#include <errno.h>
#include <linux/socket.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <unistd.h>


//#undef TRUE
//#undef FALSE
#include <global.h>


#define ANY		0		// Indicates no socket number prechosen


#define		MAXIMUM_HOST_NAME_LENGTH	100
class network_io{
	public:
		int		socket_number;
		int		rw_socket;
		// int		read_connection;
		char	host_name[MAXIMUM_HOST_NAME_LENGTH];
		int		host_socket_number;
		int		read_socket;
		struct sockaddr_in	write_connection;
		// char	incoming_string[MAXIMUM_STRING_SIZE];

		int		type;
		int		level;
		#define	_READ	200
		#define	_WRITE	201
				network_io();
				~network_io();
		int		set_remote_host(char*, int);
		int		set_local_host(int);
		int		reader(char*, int);
		int		writer(char*);
		int		init_write();
		int		open_write_connection();
		int		write_to_connection(char*);
		// int		write_to_read_connection(char*);
		int		end_write_connection();
		int		deinit_write();
		int		init_read();
		int		wait_read_connection();
		int		read_from_connection(char*, int);
		// int		read_from_write_connection(char*, int);
		int		end_read_connection();
		int		deinit_read();
		int		read_stuff_waiting();
		int		write_stuff_done();
		int		check_connection();
		char*	get_remote_client();

		char	*get_address();
	};

#endif
