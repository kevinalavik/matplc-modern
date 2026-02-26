/////////////////////////////////////////////////////////////////////////////////
//
// A program to bridge TCP/IP networks to the LPC message interface protocol
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
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
// Last Modified: May 31, 2002
//


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


#include "../network_io/network_io.h" // the low level network functions


network_io 	*network;
int		mode;
//char		*params;
char		received_temp[200];
time_t		now_time;
time_t		last_time;
int		timeout;
int		state;
int		port;
char		*remote_addr;
int		managed;


char	*remote_process_name;


#define	STOP			3000
#define	WAIT			3001
#define CONNECTED		3002
#define ACTIVE			3003

#define	TALKER			3100
#define	LISTENER		3101

#define	TRUE					1
#define FALSE					0


int	stop_to_wait();


int init(){
	int	error;
	char	*temp;

	error = NO_ERROR;
	network = NULL;
	state = STOP;
	mode = LISTENER; // set the network as a server by default
	timeout = 60;		// the default timeout (60 = 1 minute)
	remote_process_name = NULL;
	port = 1234;		// the defort port number
	remote_addr = NULL;
	managed = FALSE;

	// initialize parameters using local/global variables
	if((temp = get_argument(0, "PORT")) != NULL){
		// printf("got a port at [%s] %d \n", temp, atoi(temp));
		port = atoi(temp);
	}
	if((temp = get_argument(0, "LOGIC")) != NULL){
		// printf("got a port at [%s] %d \n", temp, atoi(temp));
		remote_process_name = new char[strlen(temp) + 1];
		strcpy(remote_process_name, temp);
	}
	if((temp = get_argument(0, "TIMEOUT")) != NULL){
		int		timeout_temp;
		// printf("got a port at [%s] %d \n", temp, atoi(temp));
		timeout_temp = atoi(temp);
		if(timeout_temp > 0) timeout = timeout_temp;
	}
	if((temp = get_argument(0, "CONTROL")) != NULL){
		if(strncasecmp(temp, "MANAGED", 7) == 0){
			managed = TRUE;
		} else if(strncasecmp(temp, "UNMANAGED", 9) == 0){
			managed = FALSE;
		} else {
			error_log(WARNING, "Unrecognized control option");
		}
	}

	return error;
}


int deinit(){
	int	error;

	error = NO_ERROR;
	if(network != NULL) delete network;
	if(remote_addr != NULL) delete remote_addr;
	if(remote_process_name != NULL) delete remote_process_name;

	return error;
}



int step_run(){
	int	error;

	error = NO_ERROR;


//printf("STEPPING SCAN START \n");
	if(state == ACTIVE){
printf("STEPPING ACTIVE \n");
		if(network->read_from_connection(received_temp, 199) != ERROR){
			if(strlen(received_temp) > 0){
				char	*temp;
				temp = new char[strlen(received_temp)+strlen(remote_process_name)+13];
				for(int i = 0; received_temp[i] != 0; i++){
					if(	(received_temp[i] == '\n') || 
						(received_temp[i] == '\r') ){
						received_temp[i] = 0;
						break;
					}
				}
				sprintf(temp, "MESSAGE %s \"%s\"", remote_process_name, received_temp);
				send_message(temp);
				delete temp;
				time(&last_time);
			}
		}

		time(&now_time);
		if((network->check_connection() == ERROR) || ((mode == LISTENER) && (difftime(now_time, last_time) > timeout))){
			if(mode == LISTENER){
				network->end_read_connection();
			} else if(mode == TALKER){
				network->end_write_connection();
			}
			state = WAIT;
			if(managed == TRUE){
				char	temp[100];
				sprintf(temp, "MESSAGE %s WAIT", remote_process_name);
				send_message(temp);
			}
		}
	} else if(state == WAIT){
printf("STEPPING WAIT \n");
		if(mode == LISTENER){
			if(network->wait_read_connection() == NO_ERROR){
				if(managed == TRUE){
					char	text[200];
					state = CONNECTED;
					sprintf(text, "Got a connection from %s", network->get_remote_client());
					error_log(WARNING, text);
					sprintf(text, "MESSAGE %s CONNECTED", remote_process_name);
					send_message(text);
				} else {
					state = ACTIVE;
				}
				time(&last_time);
			}
		} else if(mode == TALKER){
			if(network->open_write_connection() == NO_ERROR){
				if(managed == TRUE){
					char	text[200];
					state = CONNECTED;
					sprintf(text, "Connected to remote server %s:%d", remote_addr, port);
					error_log(WARNING, text);
					sprintf(text, "MESSAGE %s CONNECTED", remote_process_name);
					send_message(text);
				} else {
					state = ACTIVE;
				}
				time(&last_time);
			}
		}
	} else if((state == STOP) && (managed == FALSE)){
printf("STEPPING STOP \n");
		if(remote_process_name == NULL){
			error_log(ERROR, "Can't start a process without a logic process");
			return NO_ERROR;
		}
		error = stop_to_wait();
	}
//printf("STEPPING SCAN END \n");

	return error;
}



int message_receive(char *message){
	int	error;

	error = NO_ERROR;
//printf("message has arrived [%s] \n", message);
	if(strncasecmp(message, "LOGIC", 5) == 0){
		if(remote_process_name != NULL){
			error_log(WARNING, "there is already a source name defined, this might cause eratic behaviour (old, new)\n");
			error_log(WARNING, remote_process_name);
			error_log(WARNING, message);
			delete remote_process_name;
			delete message;
		}
		remote_process_name = new char[strlen(&(message[6])) + 1];
		strcpy(remote_process_name, &(message[6]));
		//printf("Got a process service %s \n", remote_process_name);
	} else if(strncasecmp(message, "SERVER", 6) == 0){
		mode = LISTENER;
	} else if(strncasecmp(message, "MANAGED", 7) == 0){
		managed = TRUE;
	} else if(strncasecmp(message, "UNMANAGED", 9) == 0){
		managed = FALSE;
	} else if(strncasecmp(message, "CLIENT", 6) == 0){
		mode = TALKER;
	} else if(strncasecmp(message, "SERVERNAME", 10) == 0){
		if(remote_addr != NULL) delete remote_addr;
		remote_addr = new char[strlen(&(message[11])) + 1];
		strcpy(remote_addr, &(message[11]));
	} else if(strncasecmp(message, "PORT", 4) == 0){
		int		port_tmp;
		port_tmp = atoi(&(message[5]));
		if((port_tmp > 0) && (port_tmp < 64000)){
			port = port_tmp;
		} else {
			error_log(ERROR, "specified port number outside range");
		}
	} else if(strncasecmp(message, "TIMEOUT", 7) == 0){
		int		time_tmp;
		//printf("SETTING TIMEOUT VALUE TO %s \n", message);
		time_tmp = atoi(&(message[8]));
		if(time_tmp > 0){
			timeout = time_tmp;
		} else {
			error_log(ERROR, "specified timeout number outside range");
		}
	} else if(strncasecmp(message, "ACTIVE", 6) == 0){
		if(state == CONNECTED){
			state = ACTIVE;
		} else {
			error_log(MINOR, "Attempt to move to ACTIVE from wrong state");
		}
	} else if(strncasecmp(message, "WAIT", 10) == 0){
		if(state == STOP){
			error = stop_to_wait();
		} else if(state == ACTIVE){
			if(mode == TALKER){
				network->end_write_connection();
			} else if (mode == LISTENER){
				network->end_read_connection();
			}
			state = WAIT;
		} else {
			error_log(MINOR, "Cannot go to wait state while already waiting");
		}
	} else if(strncasecmp(message, "STOP", 10) == 0){
		if(state > WAIT){
			if(mode == TALKER){
				network->end_write_connection();
			} else if (mode == LISTENER){
				network->end_read_connection();
			}
		}
		if(state > STOP){
			if(mode == TALKER){
				network->deinit_write();
			} else if (mode == LISTENER){
				network->deinit_read();
			}
			delete network;
			network = NULL;
			state = STOP;
		}
	} else {	// not a recognized message, so send it out the network or dump it
		if(state == ACTIVE){
			network->write_to_connection(message);
			// network->write_to_connection((char *) " = MESSAGE \n");
			time(&last_time); // You can keep the connection alive by writing
			// but this doesn't guarantee that the remote client is still there
		} else {
			error_log(WARNING, "message discarded bacause connection not active");
			error_log(WARNING, message);
		}
	}

	return error;
}




int stop_to_wait(){
	if(mode == LISTENER){
		//printf("setting up server at port %d \n", port);
		network = new network_io();
		network->set_local_host(port);
		if(network->init_read() == NO_ERROR){
			//now ready to listen
			state = WAIT;
		} else {
			char	temper[100];
			if(managed == TRUE){
				sprintf(temper, "MESSAGE %s STOP", remote_process_name);
				send_message(temper);
				sprintf(temper, "Server socket %d could not be opened", port);
				error_log(MINOR, temper);
			}
			state = STOP;
		}
	} else if(mode == TALKER){
		if(remote_addr != NULL){
			//printf("connecting to server [%s] at port %d \n", remote_addr, port);
			network = new network_io();
			network->set_remote_host(remote_addr, port);
			if(network->init_write() == NO_ERROR){
				//now ready to listen
				state = WAIT;
			} else {
				char	temper[100];
				if(managed == TRUE){
					sprintf(temper, "MESSAGE %s STOP", remote_process_name);
					send_message(temper);
				}
				state = STOP;
			}
		} else {
			error_log(MINOR, "remote server name not defined");
		}
	} else {
		error_log(MINOR, "ERROR: Mode not defined yet");
	}

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif




