/*
 * (c) 2002 Hugh Jack
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
 * outgoing message handler
 *
 * this was originally designed to allow both text and binary transmissions,
 * although text is the only thing configured.
 *
 * Last Revised: April 18, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> // for the FIFO
#include <sys/stat.h> // for the FIFO
#include <fcntl.h> // for the FIFO
#include <errno.h> // for the mkfifo

#include "client_queue_t.h"
#include "global.h"





client_queue_t::client_queue_t(char *_my_name, char *_remote_name){
	int	res;
	char	fifo_name[100];

	my_name = new char[strlen(_my_name) + 1];
	strcpy(my_name, _my_name);
	remote_name = new char[strlen(_remote_name) + 1];
	strcpy(remote_name, _remote_name);

	sprintf(fifo_name, "/tmp/%s.mat", remote_name);
	res = mkfifo(fifo_name, 0700);
	if((res == 0) || ((res != 0) && (errno == EEXIST))){
		fd = open(fifo_name, O_WRONLY | O_NONBLOCK);
		if(errno == ENXIO){
			status = ERROR;
//printf("got error ");
		} else {
			status = NO_ERROR;
		}
//printf("errno=%d\n", errno);
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_ASYNC);
	} else {
		printf("could not open the FIFO\n");
	}

	my_pid = getpid();
	remote_pid = 0; // later use the MAT smm to get the pid

	// construct the common parts of the outgoing message header
	header.version 				= MSG_VERSION;
	header.source				= my_pid;
	header.destination			= remote_pid;

	header_size = sizeof(header);
}



client_queue_t::~client_queue_t(){
	delete my_name;
	delete remote_name;
	close(fd);
}



void client_queue_t::writer(char *message){
	char	*temp;
//	char	temp2[1];
	int	message_size;
//	int	res;

	header.type = MSG_T_TEXT;
	message_size = strlen(message);
	header.size = message_size;
//printf("message size %d   %d    [%s] \n", message_size, header_size, message);
	temp = new char[message_size + header_size + 2];
	temp[0] = MSG_MAGIC_NUMBER_FIRST;
	temp[1] = MSG_MAGIC_NUMBER_SECOND;
//printf("header size = %d \n", header_size);
	memcpy(&(temp[2]), &header, header_size);
	memcpy(&(temp[header_size + 2]), message, message_size);

	// 
	// If the remote server has quit, the write function below will
	// issue a message 'Broken Pipe' and then exit, instead of returning
	// an error code. I don't know why anybody would use an 'exit' function
	// inside such a fundamental piece of code, but they have, so later a
	// work around will be needed to check to see if the other end of the
	// connection is still open. This will not affect the basic operation
	// of the system, but it will make it much less robust. I expect this
	// problem will arise when a tool is added for doing on-line system
	// modifications. When a system is running in a stable/static condition
	// this will not be a problem.
	//
	write(fd, temp, header_size + message_size + 2);
	// printf("got errno=%d\n", errno);
}


