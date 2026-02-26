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
 * Handler basic functions
 *
 * These function deal with basic process management.
 *
 * Last Revised: April 21, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>	// for the dynamic libraries
#include <signal.h> // for the real time signal stuff
#include <sys/time.h> // time definitions

#include "handler_t.h"




/*
 * dynamic loader API 
 */


#define API_NO_NAME			1000	// the module name has not been defined yet
#define	API_NO_MODULE		1001	// no module loaded or running
#define	API_MODULE_IDLE		1002	// module is in an idle state
#define	API_MODULE_RUN		1003	// module is running


mat_handler *mat_pointer;


int entered = 0;
void sigalarm(int sig){
	if(entered == 0){
		entered = 1;
		if(sig == SIGALRM){
			mat_pointer->scan();
		} else {
			printf("OOPS, got somebody elses signal\n");
		}
		entered = 0;
	} else {
		printf("OOPS, somebody is already in the function \n");
	}
}



// allows the library code to send a message
extern "C" int message_send(char *message){
//printf("SENDING MESSAGE >>%s<< [%s]\n", mat_pointer->module_name, message);
	return mat_pointer->queue->push(message);
}


// this routine allows the library to get arguments
extern "C" char *__get_argument(int number, char *var_name){
	argument_t	*current;

	current = mat_pointer->arg_first;
	if((number > 0) && (var_name == NULL)){ // look for it by number
		for(int i = 1; (i <= number) && (current != NULL); i++){
			if(i == number){
				break;
			} else {
				current = current->next;
			}
		}
		if(current == NULL) return NULL;
		return current->name;
	} else {
		for(; current != NULL; current = current->next){
			if(strcmp(var_name, current->name) == 0){
				return current->value;
			}
		}
		return NULL;
	}

	return NULL;
}



mat_handler::mat_handler(){
	mat_pointer = this;
	status = API_NO_NAME;	// indicate no module loaded;
	file_name = NULL;		// indicate no file name yet
	module_name = NULL;		// parse the commandline arguments
	handle = NULL;			// no library loaded yet
	delay = 100000;
	set_argument((char*)"DELAY", (char*)"100000");
	initialized = 0;
	//plc_timer_start(timer); // start the clock
	queue = new queue_t();
	server = NULL;
	client_first = NULL;
	arg_first = NULL;

	delay_set(0);
}


void mat_handler::delay_set(int setting){
	if(setting == 1) signal(SIGALRM, NULL);
	// set up the (soft) realtime clock
	long	usecs, secs;
	struct itimerval 	value;		// the real time timer value

	secs = delay/1000000;
	usecs = delay - secs*1000000;
	// printf(" Setting clock %ld s, %ld us \n", secs, usecs);
	memset(&value, 0, sizeof(value));
	value.it_interval.tv_sec = secs;			// set interval to s
	value.it_interval.tv_usec = usecs;			// set interval to us
	value.it_value.tv_sec = secs;				// set interval to s
	value.it_value.tv_usec = usecs;				// set interval to us
	if(setitimer(ITIMER_REAL, &value, NULL) != 0){
		printf("ERROR: The realtime timer was not set properly\n");
		exit(1);
	} else {
		signal(SIGALRM, sigalarm);
	}
}



mat_handler::~mat_handler(){
	signal(SIGALRM, NULL);
	unload_library();
	if(file_name != NULL) delete file_name;
	if(module_name != NULL) delete module_name;
	delete queue;
	if(server != NULL) delete server;
	while(client_first != NULL){	// clean out waiting messages
		client_queue_t *temp;
		temp = client_first;
		client_first = client_first->next;
		delete temp;
	}
	while(arg_first != NULL){	// empty out the argument list
		argument_t	*temp;
		temp = arg_first;
		if(arg_first->name != NULL) delete arg_first->name;
		if(arg_first->value != NULL) delete arg_first->value;
		arg_first = arg_first->next;
		delete temp;
	}
}



void mat_handler::set_file_name(char *name){
// define the library name
	if(file_name != NULL) delete file_name;
	file_name = new char[strlen(name) + 1];
	strcpy(file_name, name);
	set_argument((char*)"LIBRARY_NAME", name);
}



void mat_handler::set_module_name(char *name){
// this defines a matplc module name and creates a connection
	if(module_name != NULL){
		printf("MAJOR WARNING: Module name changed - this may cause problems\n");
		delete module_name;
	}
	if(server != NULL){
		delete server;
	}
	server = new server_queue_t(name);
	module_name = new char[strlen(name) + 1];
	strcpy(module_name, name);
	set_argument((char*)"MODULE_NAME", name);
}


// This routine deals with most of the message parsing. Depending upon the message
// type it may be intercepted and interpreted. If it is not a command message it
// will be passed to the library, or if targeted to a remote location, it will
// be put in the client queue to be passed to a remote client.
int mat_handler::parse_command(char *command, int argc, char **argv){
	int	len;
//printf("GOT STRING [%s]\n", command);
	len = clean_string(command);
	if(strncasecmp(command, "RUN", 3) == 0){
		if(status == API_MODULE_IDLE) status = API_MODULE_RUN;
	} else if(strncasecmp(command, "HALT", 4) == 0){
		if(status == API_MODULE_RUN) status = API_MODULE_IDLE;
	} else if(strncasecmp(command, "QUIT", 4) == 0){
		return ERROR;
	} else if(strncasecmp(command, "MODULE ", 7) == 0){
		set_module_name(&(command[7]));
		if (plc_init(module_name, argc, argv) < 0){
			printf("ERROR: Could not connect module %s to PLC\n", module_name);
			return ERROR;
		} else {
			char	*temp;
			temp = conffile_get_value((char*)"DELAY");
			//printf("Got a DELAY value of [%s]\n", temp);
			if((temp != NULL) && (atoi(temp) > 100)){
				delay = atoi(temp);
				delay_set(1);
			}
		}
		initialized = 1;
	} else if(strncasecmp(command, "SWAP ", 5) == 0){
		unload_library();
		set_file_name(&(command[5]));
		load_library();
	} else if(strncasecmp(command, "LIBRARY ", 8) == 0){
		set_file_name(&(command[8]));
	} else if(strncasecmp(command, "DELAY ", 6) == 0){
		if(atof(&(command[6])) > 100){ // some limit -- change later
			delay = atol(&(command[6]));
			set_argument((char*)"DELAY", &(command[6]));
			delay_set(1);
		} else {
			printf("WARNING: The delay of %s was too short and was ignored\n", &(command[6]));
		}
	} else if(strncasecmp(command, "PARSE ", 6) == 0){
		return parse_file(&(command[6]));
	} else if(strncasecmp(command, "HELP", 4) == 0){
		if(initialized == 1){
			if(argument_descriptions != NULL){
				char *temp;
				printf("Arguments:\n");
				for(int i = 0; ;i++){
					temp = argument_descriptions(i);
					if(temp != NULL){
						printf("    %s\n", temp);
					} else {
						return NO_ERROR;
					}
				}
			} else {
				printf("Arguments: No argument descirptions defined \n");
				return NO_ERROR;
			}
		} else {
			printf("ERROR: could not print arguments, PLC not initialized\n");
			return ERROR;
		}
	} else if(strncasecmp(command, "ARGUMENT ", 9) == 0){
		int	position;
		for(position = 9; command[position] != 0; position++){
			if(command[position] == ' '){
				command[position] = 0;
				return set_argument(&(command[9]), &(command[position+1]));
			}
		}
		return ERROR;
	} else if(strncasecmp(command, "MESSAGE ", 8) == 0){
		int	position;
		for(position = 8; command[position] != 0; position++){
			if(command[position] == ' '){
				command[position] = 0;
				if(strncasecmp(&(command[8]), "this", 4) == 0){
					if(message_receive != NULL){
						return (*message_receive)(&(command[position+1]));
					} else {
						printf("WARNING: message [%s] dropped because no client service\n", command);
						return NO_ERROR;
					}
				} else {
					return send_remote_message(&(command[8]), &(command[position+1]));
				}
			}
		}
		return ERROR;
	} else {
		if(message_receive != NULL){
//printf("SENDING MESSAGE TO FUNCTION [%s]\n", command);
			return (*message_receive)(command);
		} else {
			printf("WARNING: Unrecognized command [%s] being ignored \n", command);
			return NO_ERROR;
		}
	}

	return NO_ERROR;
}



int mat_handler::send_remote_message(char *remote_name, char *message){
	client_queue_t	*current;

	// try to locate an existing connection
	for(current = client_first; current != NULL; current = current->next){
		if(strcmp(current->remote_name, remote_name) == 0){
			break;
		}
	}

	// no connection found, need to create a new one
	if(current == NULL){
		//printf("Creating a new client connection\n");
		current = new client_queue_t(module_name, remote_name);
		if(current->status == ERROR){
			printf("Warning: Creation of new connection failed\n");
			delete current;
			return ERROR;
		}
		current->next = client_first;
		client_first = current;
	}

	current->writer(message);

	return NO_ERROR;
}



int mat_handler::parse_file(char *file_name){
	FILE	*fp_in;
	char	work[200];
	int	len;

	if((fp_in = fopen(file_name, "r")) != NULL){
		fgets(work, 200, fp_in); 
		while(feof(fp_in) == 0){
			len = clean_string(work);
			if(work[0] == '#'){
				// a comment line
			} else if(strlen(work) <= 1){
				// an empty line
			} else {
				queue->push(work);
			}
			fgets(work, 200, fp_in);
		}
		fclose(fp_in);
	} else {
		printf("ERROR: Could not open parse file \n");
		return ERROR;
	}

	return NO_ERROR;
}



int mat_handler::clean_string(char *work){
	int	i, len;
	int	start_flag,
		previous_flag,
		delimeter_flag,
		position;
	len = strlen(work);
	start_flag = 1;
	previous_flag = 0;
	delimeter_flag = 0;
	position = 0;
	for(i = 0; i < len; i++){
		if((work[i] == ' ') || (work[i] == '\t') || (work[i] == '\n')
			|| (work[i] == '\r') || (work[i] == ':')){
				delimeter_flag = 1;
		} else {
				delimeter_flag = 0;
		}
		if(start_flag == 1){
			if(delimeter_flag == 0){
				start_flag = 0;
				work[position] = work[i];
				position++;
			}
		} else {
			if(delimeter_flag == 1){
				if(previous_flag == 0){
					work[position] = ' ';
					position++;
				}
			} else {
				work[position] = work[i];
				position++;
			}
		}
		previous_flag = delimeter_flag;
	}
	if((position > 0) && (work[position-1] == ' ')){
		position--;
	}
	work[position] = 0;

	return position;
}



int mat_handler::step(int argc, char **argv){
	char	*temp;

	// check to see if there are any remote messages to receive
	if(server != NULL){
		temp = server->read_text();
		if(temp != NULL){
//printf("putting message on queue [%s]\n", temp);
			queue->push(temp);
//queue->dump();
		}
	}



	// check to see if there are any command messages to execute
	temp = queue->pull();
	if(temp != NULL){
//printf("ABOUT TO PARSE [%s]\n", temp);
		if(parse_command(temp, argc, argv) == ERROR) return ERROR;
	}

	// do updates based upon module status
	if(status == API_NO_MODULE){
		if(file_name != NULL){
			if(load_library() == NO_ERROR){
				status = API_MODULE_IDLE;
				return NO_ERROR;
			} else {
				return ERROR;
			}
		}
	} else if(status == API_NO_NAME){
		if(module_name != NULL){
			if(connect_to_mat() == NO_ERROR){
				status = API_NO_MODULE;
				return NO_ERROR;
			} else {
				return ERROR;
			}
		}
	}

	return NO_ERROR;
}


void mat_handler::scan(){
	int	res;

	if(initialized){
		res = plc_scan_beg();
		if(res == 0){
			plc_update();	// start up the scan
			plc_log_trcmsg(2, "scan loop started");
			if(status == API_MODULE_RUN){
				if(step_run != NULL) (*step_run)();
				// do running stuff here
			} else if(status == API_MODULE_IDLE){
				if(step_idle != NULL) (*step_idle)();
				// do idle stuff here
			}
			plc_update();	// wrap up the scan
			plc_scan_end();
		} else {
			printf("Warning: scan missed because PLC busy\n");
		}
	}
}



int mat_handler::connect_to_mat(){

	return NO_ERROR;
}



int mat_handler::load_library(){
//	char *temp[10];
//	temp[0] = NULL;		// an empty argument list for now

	// open the dynamically linked library
	handle = dlopen(file_name, RTLD_NOW);
//	handle = dlopen(file_name, RTLD_NOW & ~RTLD_GLOBAL);
	if(handle != NULL){ // if it was opened ok continue
		// find and check the version number for the library
		__module_version = (int(*)())dlsym(handle, "__module_version");
		if(__module_version != NULL){

			if((__module_version)() > MODULE_VERSION_NUMBER){
				printf("WARNING: Library version number (%d) is newer than the handler's (%d)\n",
					(__module_version)(), MODULE_VERSION_NUMBER);
				if(SAFETY_LEVEL < 3){
					printf("         Execution will continue, but things may not work correctly!\n");
				} else {
					printf("         Execution will halt\n");
					dlclose(handle);
					handle = NULL;
					return ERROR;
				}
			}

			if(((__module_version)() < MODULE_VERSION_NUMBER) && (SAFETY_LEVEL > 7)){
				printf("ERROR: Library version number (%d) is older than the handler's (%d)\n",
					(__module_version)(), MODULE_VERSION_NUMBER);
				dlclose(handle);
				handle = NULL;
				return ERROR;
			}

			__set_message_pointer = (void(*)(int(*)(char*)))dlsym(handle, "__set_message_pointer");
			if(__set_message_pointer != NULL){
				// no need to check for success, without the module can't send messages
				(*__set_message_pointer)(message_send);
			} else {
				printf("WARNING: message point function not found, the library can't send any messages\n");
			}

			__set_argument_pointer = (void(*)(char*(*)(int, char*)))dlsym(handle, "__set_argument_pointer");
			if(__set_argument_pointer != NULL){
				// no need to check for success, without the module can't send messages
				(*__set_argument_pointer)(__get_argument);
			}

			// printf("Loading a library: version %d\n", (__module_version)());
			init = (int(*)())dlsym(handle, "__library__init");
			if(init != NULL){
				(*init)();	// Add in argument passing here later
			}

			deinit = (int(*)())dlsym(handle, "deinit");
			step_idle = (int(*)())dlsym(handle, "step_idle");
			step_run = (int(*)())dlsym(handle, "step_run");
			if(step_run == NULL){
				printf("WARNING: A 'step_run' function was not found, this could be a sign of trouble\n");
			}
			message_receive = (int(*)(char*)) dlsym(handle, "message_receive");
			argument_descriptions = (char*(*)(int)) dlsym(handle, "argument_descriptions");
			return NO_ERROR;
		} else {
			printf("ERROR: Library file is not a known MAT PLC version\n");
			printf("       Message: %s \n", dlerror());
			dlclose(handle);
			handle = NULL;
			return ERROR;
		}
	} else {
		printf("ERROR: Could not open library [%s]\n", file_name);
		printf("       %s\n", dlerror());
		return ERROR;
	}
}



void mat_handler::unload_library(){
	if(handle != NULL){
		if(deinit != NULL) (*deinit)();
		dlclose(handle);
		handle = NULL;
	}
}



int mat_handler::set_argument(char *name, char *value){
	argument_t		*current;

	if(name == NULL) return ERROR;
	current = arg_first;
	for(; current != NULL; current = current->next){
		if(strcmp(name, current->name) == 0) break;
	}
	if(current == NULL){
		current = new argument_t;
		current->name = new char[strlen(name)+1];
		strcpy(current->name, name);
		current->value = NULL;
		current->next = arg_first;
		arg_first = current;
	}
	if(current->value != NULL){
		delete current->value;
		current->value = NULL;
	}
	if(value != NULL){
		current->value = new char[strlen(value) + 1];
		strcpy(current->value, value);
	}

	return NO_ERROR;
}



void mat_handler::dump(int level){
	printf("BIG DUMP\n");
	if(level & 1){
		printf("   Module state is: ");
		if(status == API_NO_NAME) printf("API_NO_NAME");
		if(status == API_NO_MODULE) printf("API_NO_MODULE");
		if(status == API_MODULE_IDLE) printf("API_MODULE_IDLE");
		if(status == API_MODULE_RUN) printf("API_MODULE_RUN");

		printf("\n");
	}
	if(level & 2){
		queue->dump();
	}
}


