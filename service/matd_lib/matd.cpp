/////////////////////////////////////////////////////////////////////////////////
//
// A test process to verify the operation of the dynamic linker
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
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
// Last Modified: May 10, 2002
//

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <plc.h>

//#include "../include/process.h"

#include <module_library.h>
#ifdef __cplusplus	// do this so C++ code will link
extern "C" {
#endif


#include "matd.h"


#include <fcntl.h>


//////////////////////////////////////////////////////////////////////////////////
//
// Functions specific to this program follow. i.e. you write them
//
// note:	Write the step_function() with care, it should run
//		once in a fraction of a second.
//
// required functions:
//		init() - called when the program is first loaded
//		deinit() - called before the program is unloaded
//		step() - called each scan - it should run once - fast!
//
/////////////////////////////////////////////////////////////////////////
//
// This program will receive and send messages from the kernel to
// a device driver. It doesn't try to interpret or modify messages
// it simply passes them back and forth.
//

//#include "../include/message_io.h"

char	text[100];
//char	*params;
int	local_state;
//char	*remote_name;
user_t	*users;
user_data *user_focus;


// variables in the remote IO module
//int	*command;
//int	*operand1;
//int	*operand2;
//char	*operand3;
//int	*change_lock;
//int	*error_flag;
//int	*state;
//char	*received;
//int	*received_flag;
//char	*send_buf;
//int	*send_flag;




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
	if(destination != NULL) delete destination;
	if(source != NULL) delete source;

	return NO_ERROR;
}




int	count;


int	step_run(){
	int	error;
	char	temp[100];

	//printf("ECHO: Run step ... tick \n");
	error = NO_ERROR;
	if(mode == STOP){
		printf("MATD: Initializing the connection [%s], [%s] \n", destination, source);
		sprintf(temp, "MESSAGE %s LOGIC %s", destination, source);
		send_message(temp);
		printf("MATD: sending message %s\n", temp);
		sprintf(temp, "MESSAGE %s WAIT", destination);
		send_message(temp);
		printf("MATD: sending message %s\n", temp);
		count = 0;
		mode = WAIT;
	} else if(mode == ACTIVE){
		sync_scan(NULL);
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
	//char	*temp;

	printf("MATD: Message received  [%s] \n", text);
	if(strncasecmp(text, "CONNECTED", 9) == 0){
		// do nothing, it is OK to be active
		char temp[100];
		sprintf(temp, "MESSAGE %s ACTIVE", destination);
		send_message(temp);
		printf("MATD: send message %s\n", temp);
		mode = ACTIVE;
		sync_init();
		printf("MATD: going to active mode\n");
	} else if(strncasecmp(text, "STOP", 4) == 0){
		// do nothing, it is OK to stop
		sync_deinit();
		mode = STOP;
		printf("MATD: going to stop mode\n");
//	} else if(strncasecmp(text, "ACTIVE", 6) == 0){
//		// do nothing, it is OK to stop
//		mode = ACTIVE;
//		sync_init();
//		printf("MATD: going to active mode\n");
	} else if(1 == 1 /*text[0] == '"'*/){ // must be a normal message
		// remove the quotes from the string
		int	i;
		for(i = 1; ; i++){
			if(text[i] == '\"') text[i] = 0;
			if(text[i] == 0) break;
		}
		sync_scan(&(text[1]));
//		if(strncasecmp(&(text[1]), "QUIT", 4) == 0){
//			char temp[100];
//			sprintf(temp, "MESSAGE %s STOP", destination);
//			send_message(temp);
//			printf("ECHO: sending message %s\n", temp);
//			send_message("QUIT");
//			printf("ECHO: quitting \n");
//		} else {
//			temp = new char[strlen(text) + strlen(destination) + 10];
//			sprintf(temp, "MESSAGE %s %s", destination, text);
//			send_message(temp);
//			printf("ECHO: sending message %s\n", temp);
//			delete temp;
//		}
	} else {
		printf("MATD: Got an unrecognized message format [%s] \n", text);
	}

	return NO_ERROR;
}





int sync_init(){
// This function sets up a new user connection
	int	error;

	error = NO_ERROR;
	//synchronous_list.number = number;
	synchronous_list.status = _SYNC_WAITING;
	synchronous_list.secure = 0;
	synchronous_list.incoming_string_buffer = 0;
	synchronous_list.strobe = new strobe_list();
	users = new user_t();
	users->load();
	user_focus = NULL;
	// time(&(synchronous_list.last_time));

	return error;
}



int sync_deinit(){
	int	error;

	error = NO_ERROR;
	synchronous_list.status = _SYNC_NOT_CONNECTED;
	delete synchronous_list.strobe;
	delete users;

	return error;
}


int sync_scan(char *message){
	int	error;

	error = NO_ERROR;
//printf("MATD: sync scan message %d  %s\n", synchronous_list.status, message);
	if((message != NULL) && (synchronous_list.status == _SYNC_WAITING)){
		error = sync_message(message);
	} else if((message != NULL) && (synchronous_list.status == _SYNC_RECEIVING)){
		error = sync_receive(message);
	} else if(synchronous_list.status == _SYNC_SENDING){
		error = sync_send();
	} else if(synchronous_list.status == _SYNC_STROBING){
		error = sync_strobe();
	}

	return error;
}



int sync_message(char *text){
	int	error;
	//int	i;
	int	j;
	int	len;
	int	return_flag;
	int	secure;
	static char	text2[1006];
	//static	int count = 0;
	static char	user_name[20];
	char	password[20];
	//char	filter[100];
	//int		type_;
	int		length;
	int	flag;
	char	log_string[200];
	// time_t	now_time;
	//int	slot;

	error = NO_ERROR;
	flag = 0;
	len = strlen(text);
//printf("MATD: interpreting message raw=[%s]\n", text);
//flag = 1;
//	for(j = 0; (j < len) && (flag == 0); j++){if((text[j] == '\n') || (text[j] == '\r')){text[j] = 0; flag = 1;};};
//	if(((synchronous_list.incoming_string_buffer == 1) || (flag == 0)) && (strlen(text) > 0)){
//		if(synchronous_list.incoming_string_buffer == 1){
//			strcat(synchronous_list.incoming_string, text);
//			text[0] = 0;
//		} else {
//			synchronous_list.incoming_string_buffer = 1;
//			strcpy(synchronous_list.incoming_string, text);
//			text[0] = 0;
//		}
//	}
//	if((flag == 1) && (synchronous_list.incoming_string_buffer == 1)){
//		synchronous_list.incoming_string_buffer = 0;
//		strcpy(text, synchronous_list.incoming_string);
//		synchronous_list.incoming_string[0] = 0;
//	}
	if(strlen(text) > 0){
		sprintf(log_string, "COMMAND STRING <%s>", text); error_log(WARNING, log_string);
		secure = synchronous_list.secure;
		return_flag = 1;
		if(strncasecmp(text, "HELP", 4) == 0){
			sprintf(text2, "MESSAGE %s ", destination);
			if(strlen(text) > 5){
				if(strncmp(&(text[5]), "QUIT", 4) == 0){
					strcat(text2, "QUIT - cause termination of the LPC program\n");
				} else if(strncmp(&(text[5]), "WORM", 4) == 0){
					strcat(text2, "WORM worm_string... - pass a worm message to the LPC loader\n");
				} else if(strncmp(&(text[5]), "MESSAGE", 7) == 0){
					strcat(text2, "MESSAGE dest message_string... - pass a message string to the dest\n");
				} else if(strncmp(&(text[5]), "GET", 3) == 0){
					strcat(text2, "GET filename - returns a text file from the LPC\n");
				} else if(strncmp(&(text[5]), "PUT", 3) == 0){
					strcat(text2, "PUT filename - puts a text file into the LPC\n");
				} else if(strncmp(&(text[5]), "PEEK", 4) == 0){
					strcat(text2, "PEEK variable - gets a memory value from the LPC\n");
				} else if(strncmp(&(text[5]), "POKE", 4) == 0){
					strcat(text2, "POKE variable = value - sets a memory value in the LPC\n");
				} else if(strncmp(&(text[5]), "PING", 4) == 0){
					strcat(text2, "PING - checks to see if the LPC is alive\n");
				} else if(strncmp(&(text[5]), "VERSION", 7) == 0){
					strcat(text2, "VERSION - returns the version of the LPC\n");
				} else if(strncmp(&(text[5]), "DISCONNECT", 10) == 0){
					strcat(text2, "DISCONNECT - terminates the connection to the LPC\n");
				} else if(strncmp(&(text[5]), "STROBE", 6) == 0){
					strcat(text2, "STROBE [ADD name | REMOVE name | REPORT] - work with a strobe list\n");
				} else if(strncmp(&(text[5]), "USER", 4) == 0){
					strcat(text2, "USER username - enter your user name\n");
				} else if(strncmp(&(text[5]), "PASS", 4) == 0){
					strcat(text2, "PASS password - enter your password\n");
				} else if(strncmp(&(text[5]), "PSWD", 4) == 0){
					strcat(text2, "PSWD password - change your password (disabled for now)\n");
				} else {
					strcat(text2, "USAGE HELP [USER, PASS, QUIT, WORM, MESSAGE, GET, PUT, PING, STROBE, VERSION, DISCONNECT, PSWD, PEEK, POKE]\n");
				}
			} else {
				strcat(text2, "USAGE HELP [USER, PASS, QUIT, WORM, MESSAGE, GET, PUT, PING, VERSION, DISCONNECT, PSWD, PEEK, POKE]\n");
			}
		} else if(strncasecmp(text, "USER", 4) == 0){
			// A cheap user check for now
			strncpy(user_name, &(text[5]), 19);
			user_name[19] = 0;
			for(j = 0; j < 20; j++){
				if((user_name[j] == '\n') || (user_name[j] == '\r'))user_name[j] = 0;
			}
			user_focus = users->find(user_name);
			// don't check the user name here, it makes random guessing harder
			synchronous_list.secure = 1;
			sprintf(text2, "MESSAGE %s ACK: USER NAME\n", destination);
		} else if(strncasecmp(text, "PASS", 4) == 0){
			if(strlen(text) >5){
				strncpy(password, &(text[5]), 19);
				password[19] = 0;
			} else {
				password[0] = 0;
			}
			for(j = 0; (j < 20) && (password[j] != 0); j++){
				if((password[j] == '\n') || (password[j] == '\r'))password[j] = 0;
			}
			if((secure == 1)){
				// if(password_check(user_name, password, &(synchronous_list.user)) == NO_ERROR){
				if((user_focus != NULL) && (users->verify_password(user_focus, password) == NO_ERROR)){
					synchronous_list.secure = 2;
					sprintf(text2, "MESSAGE %s ACK: ACCESS GRANTED\n", destination);
					sprintf(log_string, "USER LOGIN <%s>", user_name); error_log(WARNING, log_string);
				} else {
					synchronous_list.secure = 0;
					sprintf(log_string, "USER DENIED <%s>", user_name); error_log(WARNING, log_string);
					sprintf(text2, "MESSAGE %s ERROR: ACCESS DENIED\n", destination);
				}
			}
		} else if((secure == 2) && (user_focus->admin_level >= LEVEL_HIGH) && (strncasecmp(text, "QUIT", 4) == 0)){
			//status = _QUITTING;
			send_message((char*)"QUIT");
			sprintf(text2, "MESSAGE %s ACK: QUITTING....\n", destination);
			text2[0] = 0;
		} else if((secure == 2) && (user_focus->admin_level >= LEVEL_MEDIUM) && (strncasecmp(text, "WORM", 4) == 0)){
			if(strlen(text) > 5){
				send_message((char*)&(text[5]));
			}
			sprintf(text2, "MESSAGE %s ACK: GOT WORM\n", destination);
		} else if((secure == 2) && (user_focus->admin_level >= LEVEL_MEDIUM) && (strncasecmp(text, "MESSAGE", 7) == 0)){
			if(strlen(text) > 8){
				int		i;
				for(i = 8; text[i] != 0; i++){
					if((text[i] == ' ') || (text[i] == '\t')){
						text[i] = 0;
						break;
					}
				}
				send_message((char*)&(text[i+1]));
			}
			sprintf(text2, "MESSAGE %s ACK: GOT MESSAGE\n", destination);
		} else if((secure == 2) && (user_focus->read_level >= LEVEL_LOW) && (strncasecmp(text, "PEEK", 4) == 0)){
			if(strlen(text) > 5){
				plc_pt_t tmp;
				tmp = plc_pt_by_name(&(text[5]));
				if(tmp.valid){
					sprintf(text2, "MESSAGE %s ACK: %d\n", destination, plc_get(tmp));
				} else {
					sprintf(text2, "MESSAGE %s ERROR: Variable not found\n", destination);
				}
			} else {
				sprintf(text2, "MESSAGE %s ERROR: No variable name sent\n", destination);
			}
		} else if((secure == 2) && (user_focus->write_level >= LEVEL_MEDIUM) && (strncasecmp(text, "POKE", 4) == 0)){
//printf("%s\n", text);
			if(strlen(text) > 5){
				plc_pt_t	tmp;
				int j, len;
				len = strlen(text);
				for(j = 5; j < len; j++){
					if(text[j] == '='){
						text[j] = 0;
						break;
					}
				}
				tmp = plc_pt_by_name(&(text[5]));
				if(tmp.valid){
					if(j < len){
						plc_set(tmp, atoi(&(text[j+1])));
						sprintf(text2, "MESSAGE %s ACK: Value set OK\n", destination);
					} else {
						sprintf(text2, "MESSAGE %s ERROR: no value given\n", destination);
					}
				} else {
					sprintf(text2, "MESSAGE %s ERROR: Variable not found\n", destination);
				}
			} else {
				sprintf(text2, "MESSAGE %s ERROR: No variable name sent\n", destination);
			}
		} else if((secure == 2) && (user_focus->read_level >= LEVEL_LOW) && (strncasecmp(text, "STROBE ADD", 10) == 0)){
//printf("GOT COMMAND [%s]\n", text);
			if(strlen(text) > 11){
				if(synchronous_list.strobe->add(&(text[11])) == NO_ERROR){
					sprintf(text2, "MESSAGE %s ACK: Variable added to scan list\n", destination);
				} else {
					sprintf(text2, "MESSAGE %s ERROR: Variable not found\n", destination);
				}
			} else {
				sprintf(text2, "MESSAGE %s ERROR: No variable name sent\n", destination);
			}
		} else if((secure == 2) && (user_focus->read_level >= LEVEL_LOW) &&(strncasecmp(text, "STROBE REMOVE", 13) == 0)){
			if(strlen(text) > 14){
				strobe_list_item *tmp;
				if((tmp = synchronous_list.strobe->find(&(text[14]))) != NULL){
					synchronous_list.strobe->remove(tmp);
					sprintf(text2, "MESSAGE %s ACK: Variable removed from scan list\n", destination);
				} else {
					sprintf(text2, "MESSAGE %s ERROR: Variable not found\n", destination);
				}
			} else {
				sprintf(text2, "MESSAGE %s ERROR: No variable name sent\n", destination);
			}
		} else if((secure == 2) && (user_focus->read_level >= LEVEL_LOW) &&(strncasecmp(text, "STROBE REPORT", 13) == 0)){
			synchronous_list.status = _SYNC_STROBING;
			return_flag = 0;
		} else if((secure == 2) && (user_focus->read_level >= LEVEL_HIGH) && (strncmp(text, "GET", 3) == 0)){
			char	file_name[100];
			length = strlen(text);
			if((length > 4) && (filename_is_secure(&(text[4])) == TRUE)){
				sprintf(file_name, "shared/%s", &(text[4]));
				if((synchronous_list.fp = open(file_name, /* O_CREAT | */ O_RDONLY)) >= 0){
					synchronous_list.ack = TRUE;
					synchronous_list.status = _SYNC_SENDING;
					return_flag = 0;
					// strcpy(text2, "ACK: GETTING");
				} else {
					sprintf(text2, "MESSAGE %s ERROR: COULD NOT OPEN PROGRAM FILE\n", destination);
				}				
				//type_ = _SAVE_ALL;
			} else {
				sprintf(text2, "MESSAGE %s ERROR: GET filename is invalid\n", destination);
			}
		} else if((secure == 2) && (user_focus->write_level >= LEVEL_HIGH) && (strncasecmp(text, "PUT", 3) == 0)){
			// if((synchronous_list.fp = open(".__tmp2.plc", O_CREAT | O_WRONLY)) >= 0){
			char	file_name[100];
			if((strlen(text) > 4) && (filename_is_secure(&(text[4])) == TRUE)){
				sprintf(file_name, "shared/%s", &(text[4]));
				if((synchronous_list.fp_in = fopen(file_name, "w")) != NULL){
					synchronous_list.ack = TRUE;
					synchronous_list.status = _SYNC_RECEIVING;
					// return_flag = 0;
					sprintf(text2, "MESSAGE %s ACK: READY TO RECEIVE\n", destination);
				} else {
					sprintf(text2, "MESSAGE %s ERROR: COULD NOT OPEN PROGRAM FILE\n", destination);
				}
			} else {
				sprintf(text2, "MESSAGE %s ERROR: PUT filename is invalid\n", destination);
			}
		} else if((secure == 2) && (user_focus->admin_level >= LEVEL_LOW) && (strncasecmp(text, "VERSION", 7) == 0)){
			sprintf(text2, "MESSAGE %s ACK: %d, %s\n", destination, MODULE_VERSION_NUMBER, MODULE_VERSION_DATE);
		} else if((secure == 2) && (user_focus->admin_level >= LEVEL_LOW) && (strncasecmp(text, "PING", 4) == 0)){
			sprintf(text2, "MESSAGE %s ACK: ALIVE\n", destination);
		} else if((secure == 2) && (user_focus->admin_level >= LEVEL_HIGH) && (strncasecmp(text, "PSWD", 4) == 0)){
			strncpy(password, &(text[5]), 19);
			password[19] = 0;
			for(j = 0; (j < 20) && (password[j] != 0); j++){
				if((password[j] == '\n') || (password[j] == '\r'))password[j] = 0;
			}
			if(user_focus->encode_password(password) == NO_ERROR){
				sprintf(text2, "MESSAGE %s ACK: PASSWORD CHANGED\n", destination);
			} else {
				sprintf(text2, "MESSAGE %s ERROR: PASSWORD NOT CHANGED\n", destination);
			}
		} else if(strncasecmp(text, "DISCONNECT", 10) == 0){
			synchronous_list.status = _SYNC_NOT_CONNECTED;
			sprintf(text2, "MESSAGE %s ACK: DISCONNECTED\n", destination);
			text2[0] = 0;
		} else {
			error_log(MINOR, "ERROR: Synchronous command not recognized");
			sprintf(text2, "MESSAGE %s ERROR: INVALID COMMAND -- TRY HELP\n", destination);
		}
		if(return_flag == 1){
			send_message(text2);
		}

		if(synchronous_list.status == _SYNC_NOT_CONNECTED){\
			char	temp[100];
			sprintf(temp, "MESSAGE %s STOP", destination);
			send_message(temp);
			//*command = DISCONNECT; message_io_wait(change_lock);
		}
	}

	return error;
}



int sync_send(){
	int	error;
	int	len;
	static char	text2[1006];

	error = NO_ERROR;
	if(synchronous_list.ack == TRUE){
		len = read(synchronous_list.fp, text2, 1000);
		text2[len+1] = 0;
		if(len == 0){
			strcpy(text2, "\nACK: FILE SENT\n");
			close(synchronous_list.fp);
			synchronous_list.ack = FALSE;
			synchronous_list.status = _SYNC_WAITING;
		}
		if(text2[0] != 0){
			send_message(text2);
			//strcpy(send_buf, text2);
			//*send_flag = TRUE;
		}
	}

	return error;
}



int sync_receive(char *text2){
	int	error;

	error = NO_ERROR;
	if(strlen(text2) > 0){
		int		ii, ij;
		int		done_flag;

		done_flag = 0;
		for(ii = strlen(text2)-5, ij = 0;  ij < 3; ii--, ij++){
			// a kludgy fix for the DONE tacked onto the end of the string
			if(strncmp(&(text2[ii]), "DONE", 4) == 0){
				done_flag = 1;
				text2[ii] = 0;
				break;
			}
		}
		fprintf(synchronous_list.fp_in, "%s", text2);
		if(done_flag == 1){
			fclose(synchronous_list.fp_in);
			sprintf(text2, "MESSAGE %s ACK: TRANSFER SUCCESSFULLY COMPLETED\n", destination);
			synchronous_list.status = _SYNC_WAITING;
			error_log(WARNING, "NOTE: program uploaded successfully");
			send_message(text2);
			//strcpy(send_buf, text2);
			//*send_flag = TRUE;
		}
	}

	return error;
}




int sync_strobe(){
	int	error;
	static char	text1[100], text2[5000]; // NOTE: This limit is a quick and dirty fix, later add dynamic sending
	//plc_pt_t	var;

	error = NO_ERROR;
	sprintf(text2, "MESSAGE %s ", destination);
	//synchronous_list.strobe->scan_init();
	//while((var = synchronous_list.strobe->scan_next()) != NULL){
	
	for(strobe_list_item *tmp = synchronous_list.strobe->first; tmp != NULL; tmp = tmp->next){
		sprintf(text1, "%s=%d\n", tmp->name, plc_get(tmp->var));
		strcat(text2, text1);
	}
	//while(*send_flag == TRUE){}		// This could be bad, but it will keep things sane
	//strcpy(send_buf, text2);
	send_message(text2);
//printf("FFFO   [%s] \n", send_buf);
	//*send_flag = TRUE;

//for(int i = 0; i < 30000; i++){
//	for(int j = 0; j < 30000; j++){
//		int k = 1;
//	}
//}

	//while(*send_flag == TRUE){}		// This could be bad, but it will keep things sane
	//strcpy(send_buf, "ACK: STROBE SENT\n");
	//*send_flag = TRUE;
	//while(*send_flag == TRUE){}		// This could be bad, but it will keep things sane
	//synchronous_list.status = _SYNC_WAITING;
	char	temp[100];
	sprintf(temp, "MESSAGE %s ACK: STROBE SENT\n", destination);
	send_message(temp);

	return error;
}




int filename_is_secure(char *file_name){
	int	flag;
	int	i;

	flag = TRUE;
	for(i = 0; (file_name[i] != 0) && (flag == TRUE); i++){
		if(		(file_name[i] == '~')
			|| 	(file_name[i] == '/')
			||	((file_name[i] == '.') && (file_name[i+1] == '.'))) flag = FALSE;
	}	

	return flag;
}


//
// Routines for maintining a variable scan list
//
// This was added in to allow bulk variable transfer between a remote scanner, such
// as an HMI and the server. Without this there are multiple individual requests that
// must be made, slowing things down.
//


strobe_list::strobe_list(){
	first = NULL;
}


strobe_list::~strobe_list(){
	for(; first != NULL;){
		remove(first);
	}
}


int strobe_list::remove(strobe_list_item *focus){
	int	error;

	error = NO_ERROR;
	if(focus->prev != NULL){
		focus->prev->next = focus->next;
	}
	if(focus->next != NULL){
		focus->next->prev = focus->prev;
	}
	if(focus == first){
		first = focus->next;
	}
	delete focus->name;
	delete focus;

	return error;
}


int strobe_list::add(char *var_name){
//
// This will append the new variable to the list
//
	int				error;
	plc_pt_t			var;
	strobe_list_item	*now,
					*last;

	error = NO_ERROR;
	var = plc_pt_by_name(var_name);
	if(var.valid){
		now = new strobe_list_item();
		now->name = new char[strlen(var_name) + 1];
		strcpy(now->name, var_name);
		now->var = var;
		if(first == NULL){
			first = now;
			now->prev = NULL;
			now->next = NULL;
		} else {
			for(last = first; last->next != NULL; last = last->next){
			}
			now->prev = last;
			last->next = now;
			now->next = NULL;
		}
	} else {
		error_log(MINOR, "Could not find strobed variable");
		error = ERROR;
	}

	return error;
}


strobe_list_item *strobe_list::find(char *var_name){
	plc_pt_t			var;
	strobe_list_item	*focus;

	// printf("FINDING  [%s] \n", var_name);
	var = plc_pt_by_name(var_name);
	if(var.valid){
		for(focus = first; focus != NULL; focus = focus->next){
			if(strcmp(var_name, focus->name) == 0) break;
			//if(var == focus->var) break;
		}
	} else {
		error_log(MINOR, "Could not find requested varaible name");
		focus = NULL;
	}

	return focus;
}


int strobe_list::scan_init(){
	int	error;

	error = NO_ERROR;
	scan = NULL;

	return error;
}


strobe_list_item *strobe_list::scan_next(){
	if(scan == NULL){
		scan = first;
	} else {
		scan = scan->next;
	}

	return scan;
}






#ifdef __cplusplus
}
#endif












