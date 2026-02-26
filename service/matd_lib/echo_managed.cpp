i/////////////////////////////////////////////////////////////////////////////////
//
// A test process to verify the operation of the LPC and other clients
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
//
//    Copyright (C) 2002 by Hugh Jack <jackh@gvsu.edu>
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
// Last Modified: May 5, 2002
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <module_library.h>
#ifdef __cplusplus	// do this so C++ code will link
extern "C" {
#endif



#define	STOP		2000
#define WAIT		2001
#define ACTIVE		2002


char	*destination;
char	*source;
int	mode;


int	init(){
	printf("ECHO: Initializing\n");
	destination = get_argument(0, "SERVICE");
	if(destination != NULL){
		source = get_argument(0, "MODULE_NAME");
		if(source != NULL){
			mode = STOP;
			return NO_ERROR;
		} else {
			printf("ECHO: the variable MODULE_NAME was not defined\n");
		}
	} else {
		printf("ECHO: the variable SERVICE was not defined\n");
	}

	return ERROR;
}



int	deinit(){
	char	temp[100];

	printf("ECHO: Shutting down\n");
	sprintf(temp, "MESSAGE %s STOP", destination);
	send_message(temp);
	printf("ECHO: sending message %s\n", temp);

	return NO_ERROR;
}


int	count;


int	step_run(){
	int	error;
	char	temp[100];

	//printf("ECHO: Run step ... tick \n");
	error = NO_ERROR;
	if(mode == STOP){
		printf("ECHO: Initializing the connection [%s], [%s] \n", destination, source);
		sprintf(temp, "MESSAGE %s LOGIC %s", destination, source);
		send_message(temp);
		printf("ECHO: sending message %s\n", temp);
		sprintf(temp, "MESSAGE %s WAIT", destination);
		send_message(temp);
		printf("ECHO: sending message %s\n", temp);
		count = 0;
		mode = WAIT;
	}

	return error;
}



int	step_idle(){
	int	error;

	printf("ECHO: Idle step\n");
	error = 0;

	return error;
}



int message_receive(char *text){
	char	*temp;

	printf("ECHO: Message received  [%s] \n", text);
	if(strncasecmp(text, "CONNECTED", 9) == 0){
		// do nothing, it is OK to be active
		char temp[100];
		sprintf(temp, "MESSAGE %s ACTIVE", destination);
		send_message(temp);
		printf("ECHO: send message %s\n", temp);
	} else if(strncasecmp(text, "STOP", 4) == 0){
		// do nothing, it is OK to stop
		mode = STOP;
		printf("ECHO: going to stop mode\n");
	} else if(strncasecmp(text, "ACTIVE", 6) == 0){
		// do nothing, it is OK to stop
		mode = ACTIVE;
		printf("ECHO: going to active mode\n");
	} else if(1 == 1 /*text[0] == '"'*/){ // must be a normal message
		if(strncasecmp(&(text[1]), "QUIT", 4) == 0){
			char temp[100];
			sprintf(temp, "MESSAGE %s STOP", destination);
			send_message(temp);
			printf("ECHO: sending message %s\n", temp);
			send_message("QUIT");
			printf("ECHO: quitting \n");
		} else {
			temp = new char[strlen(text) + strlen(destination) + 10];
			sprintf(temp, "MESSAGE %s %s", destination, text);
			send_message(temp);
			printf("ECHO: sending message %s\n", temp);
			delete temp;
		}
	} else {
		printf("ECHO: Got an unrecognized message format [%s] \n", text);
	}

	return NO_ERROR;
}




#ifdef __cplusplus
}
#endif
