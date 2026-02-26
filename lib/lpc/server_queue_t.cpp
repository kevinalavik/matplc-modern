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
 * Queue (message) basic functions
 *
 * These function deal with basic process management.
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

#include "server_queue_t.h"
#include "global.h"


server_queue_t::server_queue_t(char *name){
	char	fifo_name[100];
	int	res;

	my_name = new char[strlen(name) + 1];
	strcpy(my_name, name);
	my_pid = getpid();

	sprintf(fifo_name, "/tmp/%s.mat", name);
	res = mkfifo(fifo_name, FIFO_PERMISSIONS);
	if((res == 0) || ((res != 0) && (errno == EEXIST))){
		fd = open(fifo_name, O_RDONLY | O_NONBLOCK);
		fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_ASYNC);
	} else {
		printf("ERROR: could not open the FIFO\n");
	}	
}



server_queue_t::~server_queue_t(){
	delete my_name;
	close(fd);
}



char *server_queue_t::read_text(){
	int	res;
	static char	*data=NULL;
	int	flag;
	char	temp[2];
	//int	count;
	//int	size;
	message_header_t	header;

	flag = 0;
	if(data != NULL){
		delete data;
		data = NULL;
	}
	while(flag >= 0){
		if((flag == 0) || (flag == 1)) res = read(fd, temp, 1);
		if(res > 0){
//printf("[%3d]", temp[0]);
			if(flag == 0){
				if(temp[0] == MSG_MAGIC_NUMBER_FIRST){
					flag = 1;
				} else {
					printf("WARNING: receiving unformated data\n");
				}
			} else if(flag == 1){
				if(temp[0] == MSG_MAGIC_NUMBER_SECOND){
					flag = 2;
					//count = 0;
				} else {
					printf("WARNING: receiving unformated data\n");
					flag = 0;
				}
			} else if(flag == 2){
				res = read(fd, &header, sizeof(message_header_t));
//printf("got   %d   %d  \n", res, sizeof(message_header_t));
				if(res == sizeof(message_header_t)){
					if(header.version == MSG_VERSION){
						if(header.type == MSG_T_TEXT){
							//count = 0;
							flag = 20;
							
						} else {
							printf("WARNING: unrecognized/unimplemented data type\n");
							flag = 20;
						}
						data = new char[header.size + 1];
					} else {
						printf("WARNING: unrecognized version number\n");
						flag = 0;
					}
				} else {
					printf("WARNING: Something happened while reading the message header\n");
					flag = 0;
				}
			} else if(flag == 20){
				res = read(fd, data, header.size);
printf("got %d [%s]\n", res, data);
				if(res == header.size){
					data[header.size] = 0;
printf("final = [%s]\n", data);
					flag = -1;
				} else {
					printf("Warning: incomplete data block\n");
					flag = 0;
				}
			} else {
				printf("WARNING: Something funny happened while reading a message\n");
				flag = 0;
			}
		} else {
			if(flag > 0){
				printf("WARNING: Incomplete message abandoned during transmission\n");
			}
			data = NULL;
			flag = -1;
		}
	}

	return data;
}


