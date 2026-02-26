
//
//    controller.cpp - a main controller class
//
//    Copyright (C) 2000 by Hugh Jack <jackh@gvsu.edu>
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
// Last Modified: October 12, 2000
//


#include "controller.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>		// for IO control functions

//#define _XOPEN_SOURCE
#include <unistd.h>		// for unix password encryption
#include <crypt.h>


#define		TRUE	1
#define		FALSE	0



int controller::init(){
	int	error=NO_ERROR,
		i;

	cpu = new plc5;
	in_out = new io();
	p = new parser;
	status = _RUNNING;
	// status = _IDLE;
	scan_list_count = 0;
	synchronous_list_count = 0;
	for(i = 0; i < MAXIMUM_PROGRAM_LIST_SIZE; i++){
		program_list[i] = -1;
	}
	for(i = 0; i < MAXIMUM_WORM_LIST_SIZE; i++){
		worm_list[i] = -1;
	}
	scan_count = 0;
	scan_count2 = 0;

	return error;
}


int controller::shutdown(){
	int	error=NO_ERROR;

	delete p;
	delete in_out;
	delete cpu;

	return error;
}



int controller::add_scan(int number, int type, int data, variable *_var, int position, int length, int channel){
	int	error;

	error = NO_ERROR;
	if(scan_list_count < MAXIMUM_SCAN_LIST_SIZE-1){
//printf("ADDING: %d   %d   %d   %s   %d   %d \n", number, type, data, _var->var->get_symbol(), position, length);
		scan_list[scan_list_count].number = number;
		scan_list[scan_list_count].type = type;
		scan_list[scan_list_count].data = data;
		scan_list[scan_list_count].var = _var;
		scan_list[scan_list_count].position = position;
		scan_list[scan_list_count].length = length;
		scan_list[scan_list_count].channel = channel;
		scan_list_count++;
	} else {
		error_log("ERROR: scan list full");
		error = ERROR;
	}

	return error;
}


int controller::scan_list_scan(int type){
	int	error,
		i;
	int	value;
	char	text[200];
	double	value2;
	// float	value3;

	error = NO_ERROR;
	for(i = 0; i < scan_list_count; i++){
		if((scan_list[i].type == _SCAN_INPUTS) && (type == _SCAN_INPUTS)){
			if(scan_list[i].data == _STRING){
				error = in_out->command(scan_list[i].number, _IO_READ, _STRING, scan_list[i].channel, 0, NULL, 0.0, NULL, text, 200, NULL);
				if((strlen(text) > 0) && (error != ERROR))
					scan_list[i].var->var->set_string(text);
					// error = cpu->mem->memory_set(scan_list[i].file, scan_list[i].word, text);
			} else if(scan_list[i].data == _FLOAT){
				error = in_out->command(scan_list[i].number, _IO_READ, _FLOAT, scan_list[i].channel, 0, NULL, 0.0, NULL, NULL, 0, &value2);
				if(error == NO_ERROR)
					scan_list[i].var->var->set_real(value2);
					// error = cpu->mem->memory_set(scan_list[i].file, scan_list[i].word, (float)value2);
			} else { // Assume its an integer
				error = in_out->command(scan_list[i].number, _IO_READ, scan_list[i].data, scan_list[i].channel, 0, NULL, 0.0, &value, NULL, 0, NULL);
				if(error == NO_ERROR)
					scan_list[i].var->var->set_int(value);
					// error = cpu->mem->memory_set(scan_list[i].file, scan_list[i].word, scan_list[i].position, value);
			}
		} else if((scan_list[i].type == _SCAN_OUTPUTS) && (type == _SCAN_OUTPUTS)){
			if(scan_list[i].data == _STRING){
				// text = ;
				// error = cpu->mem->memory_get(scan_list[i].file, scan_list[i].word, text);
				if((strlen(text) > 0) && (error != ERROR))
					error = in_out->command(scan_list[i].number, _IO_WRITE, _STRING, scan_list[i].channel, 0, scan_list[i].var->var->get_string(), 0.0, NULL, NULL, 0, NULL);
			} else if(scan_list[i].data == _FLOAT){
				// error = cpu->mem->memory_get(scan_list[i].file, scan_list[i].word, &value3); value2 = (double)value3;
				if(error == NO_ERROR)
					error = in_out->command(scan_list[i].number, _IO_WRITE, _FLOAT, scan_list[i].channel, 0, NULL, scan_list[i].var->var->get_real(), NULL, NULL, 0, NULL);
			} else { // Assume its an integer
				// error = cpu->get_memory_value(scan_list[i].file, scan_list[i].word, scan_list[i].position, &value);
				if(error == NO_ERROR){
					error = in_out->command(scan_list[i].number, _IO_WRITE, scan_list[i].data, scan_list[i].var->var->get_int(), scan_list[i].channel, NULL, 0.0, NULL, NULL, 0, NULL);
				}
			}
			// error = in_out->command(scan_list[i].number, _IO_WRITE, 0, 0, 0, NULL, 0.0, &value, text, 200, &value2);
		} else {
			// This scan items is not executed
			// error_log("ERROR: Other scan types not implemented yet .... please implement");
		}
	}

	return error;
}



int controller::add_worm(int position, int number){
	int	error;

	error = NO_ERROR;
	if((position >= 0) && (position < MAXIMUM_WORM_LIST_SIZE)){
		worm_list[position] = number;
	} else {
		error_log("ERROR CC000: Outside worm list");
		error = ERROR;
	}

	return error;
}


int controller::worm_list_number(int position){
	if((position >= 0) && (position < MAXIMUM_WORM_LIST_SIZE)){
		return worm_list[position];
	}
	return -1;
}


int controller::worm_list_scan(){
	int	error,
		i;
	char	text[200];

	error = NO_ERROR;
	for(i = 0; i < MAXIMUM_WORM_LIST_SIZE; i++){
		if(worm_list[i] >= 0){
			if(in_out->command(worm_list[i], _IO_READ, _STRING, 0, 0, NULL, 0.0, NULL, text, 200, NULL) == NO_ERROR){
				if(strlen(text) > 0){
					if(cpu->communications->add(text) == -1) error = ERROR;
				}
			}
		}
	}

	return error;
}




int controller::add_synchronous(int number){
	int	error;

	error = NO_ERROR;
	if(synchronous_list_count < MAXIMUM_SYNCHRONOUS_LIST_SIZE-1){
		synchronous_list[synchronous_list_count].number = number;
		synchronous_list[synchronous_list_count].status = _SYNC_NOT_CONNECTED;
		synchronous_list[synchronous_list_count].secure = 0;
		synchronous_list[synchronous_list_count].incoming_string_buffer = 0;
		synchronous_list_count++;
	} else {
		error_log("ERROR CC001: synchronous list full");
		error = ERROR;
	}

	return error;
}


int controller::synchronous_list_scan(){
	int	error,
		i, j;
	int	len;
	int	return_flag;
	int	secure;
	char	text[200];
	static char	text2[1006];
	static	int count = 0;
	static char	user_name[20];
	char	password[20];
	char	filter[100];
	int		type_;
	int		length;
	int	flag;
	char	log_string[200];
	time_t	now_time;


	error = NO_ERROR;
	count++; if(count > 100) count = 0;
	for(i = 0; i < synchronous_list_count; i++){
		if(synchronous_list[i].status == _SYNC_NOT_CONNECTED){
			// if((count % 10) == 0){ // Cut down the checks for connections
				if(in_out->command(synchronous_list[i].number,
					_IO_CONNECTION_WAIT, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL) != ERROR){
					synchronous_list[i].status = _SYNC_WAITING;
					time(&(synchronous_list[i].last_time));
					error_log("port connected");
				}
			// }
		} else if(synchronous_list[i].status == _SYNC_WAITING){
			text[0] = 0;
			time(&now_time);
			if((in_out->command(synchronous_list[i].number, _IO_IS_CONNECTED, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL) == ERROR) || (difftime(now_time, synchronous_list[i].last_time) > NETWORK_TIMEOUT)){
				synchronous_list[i].status = _SYNC_NOT_CONNECTED;
				in_out->command(synchronous_list[i].number, _IO_CONNECTION_END, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL);
				text2[0] = 0;
				synchronous_list[i].secure = 0;
				error_log("synchronous port disconnected after timeout");
			} else if(in_out->command(synchronous_list[i].number, _IO_CONNECTION_READ, _STRING, 0, 0, NULL, 0.0, NULL, text, 200, NULL) != ERROR){
				if(strlen(text) > 0) time(&(synchronous_list[i].last_time));
				flag = 0;
				for(j = 0; (j < 200) && (flag == 0); j++){if((text[j] == '\n') || (text[j] == '\r')){text[j] = 0; flag = 1;};};
				if(((synchronous_list[i].incoming_string_buffer == 1) || (flag == 0)) && (strlen(text) > 0)){
					if((strlen(text) + strlen(synchronous_list[i].incoming_string)) > 199){
						synchronous_list[i].incoming_string[199-strlen(text)] = 0;
					}
					if(synchronous_list[i].incoming_string_buffer == 1){
						strcat(synchronous_list[i].incoming_string, text);
						text[0] = 0;
					} else {
						synchronous_list[i].incoming_string_buffer = 1;
						strcpy(synchronous_list[i].incoming_string, text);
						text[0] = 0;
					}
				}
				if((flag == 1) && (synchronous_list[i].incoming_string_buffer == 1)){
					synchronous_list[i].incoming_string_buffer = 0;
					strcpy(text, synchronous_list[i].incoming_string);
					synchronous_list[i].incoming_string[0] = 0;
				}
				if(strlen(text) > 0){
				sprintf(log_string, "COMMAND STRING <%s>", text); error_log(log_string);
					secure = synchronous_list[i].secure;
					return_flag = 1;
					if((secure == 2) && (strncmp(text, "RUN", 3) == 0)){
						status = _RUNNING;
						strcpy(text2, "ACK: IN RUN MODE\n");
					} else if(strncmp(text, "HELP", 4) == 0){
						if(strlen(text) > 5){
							if(strncmp(&(text[5]), "RUN", 3) == 0){
								strcpy(text2, "RUN - put the PLC in run mode\n");
							} else if(strncmp(&(text[5]), "IDLE", 4) == 0){
								strcpy(text2, "IDLE - put the PLC in idle mode\n");
							} else if(strncmp(&(text[5]), "QUIT", 4) == 0){
								strcpy(text2, "QUIT - cause termination of the PLC program\n");
							} else if(strncmp(&(text[5]), "WORM", 4) == 0){
								strcpy(text2, "WORM - pass a worm message to the PLC\n");
							} else if(strncmp(&(text[5]), "GET", 3) == 0){
								strcpy(text2, "GET - returns an XML file for the PLC\n");
							} else if(strncmp(&(text[5]), "PUT", 3) == 0){
								strcpy(text2, "PUT - puts an XML file into the PLC\n");
							} else if(strncmp(&(text[5]), "STATUS", 6) == 0){
								strcpy(text2, "STATUS - returns the status of the PLC\n");
							} else if(strncmp(&(text[5]), "VERSION", 7) == 0){
								strcpy(text2, "VERSION - returns the version of the PLC\n");
							} else if(strncmp(&(text[5]), "DISCONNECT", 10) == 0){
								strcpy(text2, "DISCONNECT - terminates the connection to the PLC\n");
							} else if(strncmp(&(text[5]), "USER", 4) == 0){
								strcpy(text2, "USER - enter your user name\n");
							} else if(strncmp(&(text[5]), "PASS", 4) == 0){
								strcpy(text2, "PASS - enter your password\n");
							} else if(strncmp(&(text[5]), "PSWD", 4) == 0){
								strcpy(text2, "PSWD - change your password (disabled for now)\n");
							} else {
								strcpy(text2, "USAGE HELP [USER, PASS, RUN, IDLE, QUIT, WORM, GET, PUT, STATUS, VERSION, DISCONNECT, PSWD]\n");
							}
						} else {
							strcpy(text2, "USAGE HELP [USER, PASS, RUN, IDLE, QUIT, WORM, GET, PUT, STATUS, VERSION, DISCONNECT]\n");
						}
					} else if(strncmp(text, "USER", 4) == 0){
						// A cheap user check for now
						strncpy(user_name, &(text[5]), 19);
						user_name[19] = 0;
						for(j = 0; j < 20; j++){
							if((user_name[j] == '\n') || (user_name[j] == '\r'))user_name[j] = 0;
						}
						synchronous_list[i].secure = 1;
						strcpy(text2, "ACK: USER NAME\n");
					} else if(strncmp(text, "PASS", 4) == 0){
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
							if(password_check(user_name, password, &(synchronous_list[i].user)) == NO_ERROR){
								synchronous_list[i].secure = 2;
								strcpy(text2, "ACK: ACCESS GRANTED\n");
								sprintf(log_string, "USER LOGIN <%s>", user_name); error_log(log_string);
							} else {
								synchronous_list[i].secure = 0;
								sprintf(log_string, "USER DENIED <%s>", user_name); error_log(log_string);
								strcpy(text2, "ERROR: ACCESS DENIED\n");
							}
						}
					} else if((secure == 2) && (synchronous_list[i].user.admin_level > 1) && (strncmp(text, "IDLE", 4) == 0)){
						status = _IDLE;
						strcpy(text2, "ACK: IN IDLE MODE\n");
					} else if((secure == 2) && (synchronous_list[i].user.admin_level > 1) && (strncmp(text, "QUIT", 4) == 0)){
						status = _QUITTING;
						strcpy(text2, "ACK: QUITTING....\n");
						text2[0] = 0;
					} else if((secure == 2) && (synchronous_list[i].user.admin_level > 0) && (strncmp(text, "WORM", 4) == 0)){
						if(strlen(text) > 5){
							if(cpu->communications->add(&(text[5])) == -1) error = ERROR;
						}
						strcpy(text2, "ACK: GOT WORM\n");
					} else if((secure == 2) && (synchronous_list[i].user.read_level > 0) && (strncmp(text, "GET", 3) == 0)){
						length = strlen(text);
						type_ = -1000; filter[0] = 0;
						if(length < 5){
							type_ = _SAVE_ALL;
						} else if(strncmp(&(text[4]), "MEMORY", 6) == 0){
							type_ = _SAVE_MEMORY;
							if(length > 11) strcpy(filter, &(text[11]));
						} else if(strncmp(&(text[4]), "PROGRAM", 7) == 0){
							type_ = _SAVE_PROGRAM;
							if(length > 12) strcpy(filter, &(text[12]));
						} else if(strncmp(&(text[4]), "IO", 2) == 0){
							type_ = _SAVE_IO;
							if(length > 7) strcpy(filter, &(text[7]));
						} else {
							strcpy(text2, "ERROR: COMMAND NOT RECOGNIZED\n");
						}

						if(type_ != -1000){
							if(save_plc_file(".__tmp.plc", type_, filter) == NO_ERROR){
								if((synchronous_list[i].fp = open(".__tmp.plc", O_CREAT | O_RDONLY)) >= 0){
									synchronous_list[i].ack = TRUE;
									synchronous_list[i].status = _SYNC_SENDING;
									return_flag = 0;
									// strcpy(text2, "ACK: GETTING");
								} else {
									strcpy(text2, "ERROR: COULD NOT OPEN PROGRAM FILE\n");
								}
							} else {
								strcpy(text2, "ERROR: COULD NOT SAVE FILE\n");
							}
						}
					} else if((secure == 2) && (synchronous_list[i].user.write_level > 0) && (strncmp(text, "PUT", 3) == 0)){
						// if((synchronous_list[i].fp = open(".__tmp2.plc", O_CREAT | O_WRONLY)) >= 0){
						if((synchronous_list[i].fp_in = fopen(".__tmp2.plc", "w")) != NULL){
							synchronous_list[i].ack = TRUE;
							synchronous_list[i].status = _SYNC_RECEIVING;
							// return_flag = 0;
							strcpy(text2, "ACK: READY TO RECEIVE\n");
						} else {
							strcpy(text2, "ERROR: COULD NOT OPEN PROGRAM FILE\n");
						}
					} else if((secure == 2) && (synchronous_list[i].user.admin_level > 0) && (strncmp(text, "VERSION", 7) == 0)){
						sprintf(text2, "ACK: %s\n", __VERSION);
					} else if((secure == 2) && (synchronous_list[i].user.admin_level > 0) && (strncmp(text, "STATUS", 6) == 0)){
						if(status == _RUNNING) strcpy(text2, "ACK: RUNNING\n");
						if(status == _IDLE) strcpy(text2, "ACK: IDLE\n");
					} else if((secure == 2) && (synchronous_list[i].user.admin_level > 0) && (strncmp(text, "PSWD", 4) == 0)){
						strncpy(password, &(text[5]), 19);
						password[19] = 0;
						for(j = 0; (j < 20) && (password[j] != 0); j++){
							if((password[j] == '\n') || (password[j] == '\r'))password[j] = 0;
						}
						if(password_change(&(synchronous_list[i].user), password) == NO_ERROR){
							strcpy(text2, "ACK: PASSWORD CHANGED\n");
						} else {
							strcpy(text2, "ERROR: PASSWORD NOT CHANGED\n");
						}
					} else if(strncmp(text, "DISCONNECT", 10) == 0){
						synchronous_list[i].status = _SYNC_NOT_CONNECTED;
						in_out->command(synchronous_list[i].number,
							_IO_CONNECTION_END, _STRING, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL);
						strcpy(text2, "ACK: DISCONNECTED\n");
						text2[0] = 0;
					} else {
						error_log("ERROR: Synchronous command not recognized");
						strcpy(text2, "ERROR: INVALID COMMAND -- TRY HELP\n");
						/// error = ERROR;
					}
					if(return_flag == 1){
						if(text2[0] != 0) in_out->command(synchronous_list[i].number, _IO_CONNECTION_WRITE,
							_STRING, 0, 0, text2, 0.0, NULL, NULL, 0, NULL);
					}
			
					if(synchronous_list[i].status == _SYNC_NOT_CONNECTED){
						in_out->command(synchronous_list[i].number,
							_IO_CONNECTION_END, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL);
					}
				}
			} else if(in_out->command(synchronous_list[i].number, _IO_IS_WAITING, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL) == NO_ERROR){
				synchronous_list[i].status = _SYNC_NOT_CONNECTED;
				in_out->command(synchronous_list[i].number, _IO_CONNECTION_END, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL);
				text2[0] = 0;
				synchronous_list[i].secure = 0;
				error_log("port disconnected");
			}
		} else if(synchronous_list[i].status == _SYNC_RECEIVING){
			text2[0] = 0;
			time(&now_time);
			// do a timeout check here
			if((in_out->command(synchronous_list[i].number, _IO_IS_CONNECTED, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL) == ERROR) || (difftime(now_time, synchronous_list[i].last_time) > NETWORK_TIMEOUT)){
				synchronous_list[i].status = _SYNC_NOT_CONNECTED;
				in_out->command(synchronous_list[i].number, _IO_CONNECTION_END, 0, 0, 0, NULL, 0.0, NULL, NULL, 0, NULL);
				synchronous_list[i].secure = 0;
				error_log("port disconnected");
				fclose(synchronous_list[i].fp_in);
				error_log("ERROR: PUT connection timed out");
			}
			if(in_out->command(synchronous_list[i].number,
				_IO_CONNECTION_READ, _STRING, 0, 0, NULL, 0.0, NULL, text2, 1000, NULL) != ERROR){
				if(strlen(text2) > 0){
					time(&(synchronous_list[i].last_time));
					//printf("Get String [%s]\n", text2);
					// text2[strlen(text2)-1] = 0;
					if(strncmp(text2, "DONE", 4) == 0){
						// close(synchronous_list[i].fp);
						fclose(synchronous_list[i].fp_in);
						if(load_plc_file(".__tmp2.plc") == NO_ERROR){
							strcpy(text2, "ACK: TRANSFER SUCCESSFULLY COMPLETED\n");
							synchronous_list[i].status = _SYNC_WAITING;
							error_log("NOTE: program uploaded successfully");
						} else {
							strcpy(text2, "ERROR: THERE WAS A PROBLEM WITH THE PLC FILE\n");
							synchronous_list[i].status = _SYNC_WAITING;
							error_log("ERROR: plc file not uploaded correctly");
						}
						in_out->command(synchronous_list[i].number,
							_IO_CONNECTION_WRITE, _STRING, 0, 0, text2, 0.0, NULL, NULL, 0, NULL);
					} else {
						//printf("got string %d [%s] \n", strlen(text2), text2);
						fprintf(synchronous_list[i].fp_in, "%s", text2);
						// write(synchronous_list[i].fp, text2, strlen(text2)-2);
						strcpy(text2, "ACK: LINE RECEIVED\n");
						in_out->command(synchronous_list[i].number,
							_IO_CONNECTION_WRITE, _STRING, 0, 0, text2, 0.0, NULL, NULL, 0, NULL);
					}
				}
			}
		} else if(synchronous_list[i].status == _SYNC_SENDING){
			if(synchronous_list[i].ack == TRUE){
				// strcpy(text2, "LINE:");
				// len = read(synchronous_list[i].fp, &(text2[5]), 1000);
				len = read(synchronous_list[i].fp, text2, 1000);
				//text2[len+6] = 0;
				text2[len+1] = 0;
				// synchronous_list[i].ack = FALSE;
				if(len == 0){
					strcpy(text2, "\nACK: PROGRAM SENT\n");
					// close(synchronous_list[i].fp);
					// synchronous_list[i].status = _SYNC_WAITING;
					close(synchronous_list[i].fp);
					synchronous_list[i].ack = FALSE;
					synchronous_list[i].status = _SYNC_WAITING;
				}
				if(text2[0] != 0) in_out->command(synchronous_list[i].number,
					_IO_CONNECTION_WRITE, _STRING, 0, 0, text2, 0.0, NULL, NULL, 0, NULL);
			}
		} else {
			error_log("ERROR CC002: Synchronous scan list status not recognized");
			error = ERROR;
		}
	}

	return error;
}



int controller::add_program(int position, int number){
	int	error;

	error = NO_ERROR;
	if((position >= 0) && (position < MAXIMUM_PROGRAM_LIST_SIZE)){
		if(program_list[position] == -1){
			program_list[position] = number;
		} else {
			error_log("ERROR CC003: IO program slot already in use");
		}
	} else {
		error_log("ERROR CC004: program IO number outside list");
		error = ERROR;
	}

	return error;
}




int controller::program_list_scan(int position){
	if((position >= 0) && (position < MAXIMUM_PROGRAM_LIST_SIZE)){
		return program_list[position];
	} else {
		error_log("ERROR CC005: program IO number outside list");
		return -1;
	}
}



int controller::scan(){
	int	error=NO_ERROR,
		i;

	scan_count++;
	if(scan_count == 10000){
		scan_count = 0;
		scan_count2++;
		// printf("Tick [%d]\n", scan_count2);
	}

	if((status == _RUNNING) || (status == _IDLE)){
		error = scan_list_scan(_SCAN_INPUTS);
		if(error == NO_ERROR) error = synchronous_list_scan();
		if(error == NO_ERROR) error = worm_list_scan();
		if(error == NO_ERROR) error = cpu->scan(2);
	}
	for(i = 0; (i < cpu->communications->size()) /* && (error == NO_ERROR) */; i++){
		if(cpu->communications->status(i) == _WAITING){
			/* error = */ communication_update(i);
		}
	}
	if((status == _RUNNING) && (error == NO_ERROR)) error = scan_list_scan(_SCAN_OUTPUTS);
	if(status == _IDLE) sleep(1);
	if(status == _QUITTING) error = ERROR;

	return error;
}


int controller::communication_update(int number){
	int	error,
		io_num;
/*
	char	text[200];
	char	text2[200];
	char	*temp_string;
	variable	*var;
*/

	error = NO_ERROR;
/*
	cpu->communications->scan(number, &temp_string);
	p->parse(temp_string, ' ');
//printf("------------------ start dump\n");
//cpu->communications->dump();
printf("message string is <%s> %d\n", temp_string, p->counter());
	if(strcmp(p->token(0), "APPEND") == 0){
		if(strcmp(p->token(1), "MEMORY") == 0){
			var = cpu->mem->find(p->token(2));
			// error = cpu->decode_address(p->token(2), &type_addr, &type, &file, &word, &bit);
			if(error == NO_ERROR){
				// if(type == _STRING){
					cpu->communications->update(number, p->token_string(3, 1000));
//printf("New string [%s ", p->token_string(3, 1000));
					//cpu->mem->memory_get(file, word, &(text2[1]));
					strcpy(&(text2[1]), var->var->get_string());
					text2[0] = '\"'; strcat(text2, "\"");
//printf("%s] \n", var->var->get_string());
					cpu->communications->append(number, text2);
				// } else {
				// 	error_log("ERROR: Some data types not available yet for communication");
				// 	error = ERROR;
				// }
			}
		} else if(strncmp(p->token(1), "#", 1) == 0){
//printf("getting input\n");
			if((io_num = program_list_scan(atoi(&(p->token(1)[1])))) >= 0){
				if(in_out->command(io_num, _IO_READ, _STRING, 0, 0, NULL, 0.0, NULL, &(text[1]), 200-2, NULL) == NO_ERROR){
//printf("got input <%s> %d \n", &(text[1]), error);
					cpu->communications->update(number, p->token_string(2, 1000));
					text[0] = '\"'; strcat(text, "\"");
					cpu->communications->append(number, text);
				}
			} else {
				error_log("ERROR CC006: not a valid IO number");
				error = ERROR;
			}
		} else if(strcmp(p->token(1), "COMMAND") == 0){
			cpu->communications->update(number, p->token_string(3, 1000));
			(p->token(2))[strlen(p->token(2)) - 1] = 0;
			cpu->communications->append(number, &(p->token(2)[1]));
		} else {
			error_log("ERROR CC007: Don't recognize APPEND type");
			error = ERROR;
		}
	} else if(strcmp(p->token(0), "SEND") == 0){
		if(strncmp(p->token(1), "#", 1) == 0){
			if((io_num = program_list_scan(atoi(&(p->token(1)[1])))) >= 0){
				(p->token(2))[strlen(p->token(2)) - 1] = 0;
				error = in_out->command(io_num, _IO_WRITE, _STRING, 0, 0, &((p->token(2))[1]), 0.0, NULL, NULL, 0, NULL);
				if(error == NO_ERROR){
					cpu->communications->update(number, p->token_string(2, 1000));
					text[0] = '\"'; strcat(text, "\"");
					cpu->communications->append(number, text);
				}
			} else {
				error_log("ERROR CC008: not a valid IO number");
				error = ERROR;
			}
			if(p->counter() < 4){
				cpu->communications->status(number, _DONE);
			} else {
				cpu->communications->update(number, p->token_string(3, 1000));
			}
//		} else if(strcmp(p->token(1), "NETWORK") == 0){
//			strcpy(text2, p->token(2));
//			for(i = 0; (text2[i] != 0) && (text2[i] != ':'); i++){};
//			socket = _DEFAULT_SOCKET;
//			if(text2[i] != 0){
//				socket = atoi(&text2[i+1]);
//				text2[i] = 0;
//			}
//			(p->token(3))[strlen(p->token(3)) - 1] = 0;
//			error = in_out.output_network_start(text2, socket, &((p->token(3))[1]));
//			error = in_out.output_network_end();
//			if(p->counter() < 5){
//				cpu->communications->status(number, _DONE);
//			} else {
//				cpu->communications->update(number, p->token_string(4, 1000));
//			}
		} else if(strcmp(p->token(1), "MEMORY") == 0){
			(cpu->mem->find(p->token(2)))->var->set_string(p->token(3));
			// error = cpu->set_memory_value(p->token(2), p->token(3));
			if(p->counter() < 5){
				cpu->communications->status(number, _DONE);
			} else {
				cpu->communications->update(number, p->token_string(4, 1000));
			}
		} else {
			error_log("ERROR CC009: Don't recognize SEND location");
			error = ERROR;
		}
	} else if(strcmp(p->token(0), "WORM") == 0){
		if(strncmp(p->token(1), "#", 1) == 0){
//printf("worm 0 \n");
			if((io_num = worm_list_number(atoi(&(p->token(1)[1])))) >= 0){
//printf("worm 1 \n");
				error = in_out->command(io_num, _IO_WRITE, _STRING, 0, 0,
					p->token_string(2, 1000), 0.0, NULL, NULL, 0, NULL);
				}
//printf("worm 2 \n");
				cpu->communications->status(number, _DONE);
		} else if(strcmp(p->token(1), "NETWORK") == 0){
			int i, socket;
			strcpy(text2, p->token(2));
			for(i = 0; (text2[i] != 0) && (text2[i] != ':'); i++){};
			socket = _DEFAULT_SOCKET;
			if(text2[i] != 0){
				socket = atoi(&text2[i+1]);
				text2[i] = 0;
			}
			network N;
			if(N.set_remote_host(text2, socket) == NO_ERROR){
				N.writer(p->token_string(3, 1000));
			}
			cpu->communications->status(number, _DONE);
		} else {
			error_log("ERROR CC010: Don't recognize WORM location");
			error = ERROR;
		}
	} else if(strcmp(p->token(0), "PROCESSOR") == 0){
		if((strcmp(p->token(1), "RUN") == 0) && (status != _FAULT)){
			status = _RUNNING;
		} else if(strcmp(p->token(1), "IDLE") == 0){
			status = _IDLE;
		} else if(strcmp(p->token(1), "CLEAR") == 0){
			status = _IDLE;
		} else if(strcmp(p->token(1), "QUIT") == 0){
			status = _QUITTING;
		} else {
			error_log("ERROR CC011: Don't recognize command");
			error = ERROR;
		}
		if(p->counter() < 3){
			cpu->communications->status(number, _DONE);
		} else {
			cpu->communications->update(number, p->token_string(2, 1000));
		}
	} else {
		error_log("ERROR CC012: don't recognize the command");
		error = ERROR;
	}
	if(error == ERROR) cpu->communications->status(number, _ERROR);
*/
	return error;
}


int controller::string_extract(char* stringy, int *start, int *end, int *next){
	int	error,
		i, len;

	error = NO_ERROR;
	len = strlen(stringy);
	start[0] = -1; end[0] = -1; next[0] = -1;
	for(i = 0; i < len; i++){
		if(stringy[i] == '\"'){
			if(start[0] == -1){
				start[0] = i;
			} else {
				end[0] = i;
				break;
			}
		}
	}
	if(end[0] != -1){
		for(i = end[0]+1; i < len; i++){
			if((stringy[i] != ' ') && (stringy[i] != '\t')){
				next[0] = i;
				break;
			}
		}
	} else {
		error_log("ERROR CC013: string is not properly formed");
		error = ERROR;
	}

	return error;
}



#define		___UNDEFINED	100
#define		___IGNORE		101
#define		___PLC			102
#define		___IO			103
#define		___MEMORY		104
#define		___PROGRAM		105
#define		___MEM_INTEGER	106
#define		___MEM_REAL		107
#define		___MEM_BIT		108
#define		___MEM_STRING	109
#define		___MEM_ARRAY	110
#define		___MEM_UNION	111
#define		___MEM_SET		112
#define		___PROG_LADDER	113
#define		___PROG_LINE	114
#define		___IO_NETWORK	115
#define		___IO_SERIAL	116
#define		___IO_CARD		117
#define		___IO_STREAM	118
#define		___IO_SCAN		119


int controller::load_plc_file(const char* file_name){
	int	error, i, j, k;
	int	io_number, value_int;
//	int	file, word, channel;
	int	array_type;
	int	program_temp;
	FILE	*fp_in;
	char	work[200],
		temp_comment[200],
		temp_symbol[200],
		temp_name[200],
		temp_data[200],
		temp_value[200],
		temp_value2[200],
		temp_value3[200],
		temp_value4[200];
	int	token_stack[16],
		type[16], file[16], size[16],
		bit_number,
		token_count;
	variable	*var, *var_temp, *var_temp2, *var_parent;

	token_count = -1;
	error = NO_ERROR;
	j = 0;
	if((fp_in = fopen(file_name, "r")) != NULL){
		while ((feof(fp_in) == 0) && (error == NO_ERROR)) {
			j++;
			fgets(work, 200, fp_in);
//printf("\nLine %3d == %d: [%s]", j, token_count, work);
			p->parse_xml(work);
			for(i = 0; (i < p->counter()) && (error == NO_ERROR); i++){
//printf(":%s", p->token(i));
				if(token_count < 0){
					if(strcmp(p->token(i), "<") == 0){
						if(strncmp(p->token(i+1), "?", 1) == 0){
							token_count++;
							token_stack[token_count] = ___IGNORE;
							i++;
						} else if(strncmp(p->token(i+1), "!", 1) == 0){
							token_count++;
							token_stack[token_count] = ___IGNORE;
							i++;
						} else if(strncmp(p->token(i+1), "-->",3) == 0){
							token_count++;
							token_stack[token_count] = ___IGNORE;
							i++;
						} else if(strcmp(p->token(i+1), "plc") == 0){
							token_count++;
							token_stack[token_count] = ___PLC;
							i += 2;
						} else if(strcmp(p->token(i+1), "/") == 0){
							token_count--;
							i += 3;
						} else {
							error_log("ERROR CC020: file element not recognized");
							error = ERROR;
						}
					}
				} else if(token_stack[token_count] == ___IGNORE){
					if(strcmp(p->token(i), ">") == 0){
						if(token_stack[token_count] == ___IGNORE) token_count--;
					}
				} else if(token_stack[token_count] == ___PLC){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "memory") == 0){
							token_count++;
							token_stack[token_count] = ___MEMORY;
							i += 2;
						} else if(strcmp(p->token(i+1), "program") == 0){
							token_count++;
							token_stack[token_count] = ___PROGRAM;
							i += 2;
						} else if(strcmp(p->token(i+1), "io") == 0){
							token_count++;
							token_stack[token_count] = ___IO;
							i += 2;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--;
							i += 3;
						}
					} else {
						error_log("ERROR CC021: unrecognized plc content type");
					}
				} else if(token_stack[token_count] == ___MEMORY){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "integer") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_INTEGER;
							type[token_count] = _INTEGER;
						} else if(strcmp(p->token(i+1), "real") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_REAL;
							type[token_count] = _REAL;
						} else if(strcmp(p->token(i+1), "bit") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_BIT;
							type[token_count] = _BIT;
						} else if(strcmp(p->token(i+1), "string") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_STRING;
							type[token_count] = _STRING;
						} else if(strcmp(p->token(i+1), "array") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_ARRAY;
							type[token_count] = _ARRAY;
						} else if(strcmp(p->token(i+1), "union") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_UNION;
							type[token_count] = _UNION;
						} else if(strcmp(p->token(i+1), "set") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_SET;
							// type[token_count] = _STRING;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--; i += 3;
						} else {
							error_log("ERROR CC022: unrecognized memory content type");
							error = ERROR;
						}
						var = NULL;
						var_parent = NULL;
						temp_name[0] = 0;
						temp_symbol[0] = 0;
						temp_comment[0] = 0;
						temp_value[0] = 0;
						temp_data[0] = 0;
					} else {
						error_log("ERROR CC023: Content of memory definition not recognized");
						error = ERROR;
					}
					if(error == NO_ERROR){
						file[token_count] = 0; size[token_count] = 0;
						temp_comment[0] = 0; temp_symbol[0] = 0;
					}
				} else if((token_stack[token_count] == ___MEM_INTEGER) || 
					(token_stack[token_count] == ___MEM_REAL) || 
					(token_stack[token_count] == ___MEM_STRING) || 
					(token_stack[token_count] == ___MEM_BIT) || 
					(token_stack[token_count] == ___MEM_ARRAY) || 
					(token_stack[token_count] == ___MEM_UNION)){
					if(strncmp(p->token(i), "name", 4) == 0){
						strcpy(temp_name, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "symbol", 6) == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "parent", 6) == 0){
						var_parent = cpu->mem->find(p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "number", 6) == 0){
						bit_number = atoi(p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "value", 5) == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "data", 4) == 0){
						strcpy(temp_data, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "size", 4) == 0){
						size[token_count] = atoi(p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "comment", 7) == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if((strncmp(p->token(i), ">", 1) == 0) || ((strncmp(p->token(i), "/", 1) == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						if(var_parent == NULL){
							var = cpu->mem->find(temp_name);
						} else {
							var = var_parent;
						}
						// var = var_parent;
						if(token_stack[token_count] == ___MEM_INTEGER){
							var_temp = cpu->mem->add(var, temp_name, temp_symbol, temp_comment, _INTEGER);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						} else if(token_stack[token_count] == ___MEM_BIT){
							var_temp = cpu->mem->add(var, temp_name, temp_symbol, temp_comment, _BIT);
							value_int = 0; if(temp_value[0] != 0) value_int = atoi(temp_value);
							var_temp->var->set_bit(bit_number, value_int);
						} else if(token_stack[token_count] == ___MEM_REAL){
							var_temp = cpu->mem->add(var, temp_name, temp_symbol, temp_comment, _REAL);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						} else if(token_stack[token_count] == ___MEM_STRING){
							var_temp = cpu->mem->add(var, temp_name, temp_symbol, temp_comment, _STRING);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						} else if(token_stack[token_count] == ___MEM_ARRAY){
							var_temp = cpu->mem->add(var, temp_name, temp_symbol, temp_comment, _ARRAY);
							var_temp->var->set_size(size[token_count]);
							temp_value2[0] = 0; temp_value3[0] = 0;
							array_type = _INTEGER;
							if(strcmp(temp_data, "real") == 0) array_type = _REAL;
							if(strcmp(temp_data, "bit") == 0) array_type = _BIT;
							if(strcmp(temp_data, "string") == 0) array_type = _STRING;
							if(strcmp(temp_data, "array") == 0) array_type = _ARRAY;
							if(strcmp(temp_data, "union") == 0) array_type = _UNION;
							for(k = 0; k < size[token_count]; k++){
								if(temp_name[0] != 0) sprintf(temp_value2, "%s%d", temp_name, k);
								if(temp_symbol[0] != 0) sprintf(temp_value3, "%s%d", temp_symbol, k);
								// printf("%s  %s  \n", temp_value2, temp_value3);
								var_temp2 = cpu->mem->add(var_temp, temp_value2, temp_value3, NULL, array_type);
								var_temp->var->set_var(k, var_temp2);
							}
						} else if(token_stack[token_count] == ___MEM_UNION){
							var_temp = cpu->mem->add(var, temp_name, temp_symbol, temp_comment, _UNION);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						}
						if(strncmp(p->token(i), "/", 1) == 0){ i+= 2; token_count--;}

					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						token_count--; i += 3;
					} else {
						error_log("ERROR CC024: memory definition type not correct");
						// printf(" %s=\"%s\"\n", p->token(i), p->token(i+1));
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___MEM_SET){ 
					if(strcmp(p->token(i), "name") == 0){
						var = cpu->mem->find(p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "value") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "number") == 0){
						bit_number = atoi(p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "//", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strcmp(p->token(i+1), ">") == 0))){
						if(var != NULL){
							if(var->var->type == _BIT){
								var->var->set_bit(bit_number, atoi(temp_value));
							} else if(temp_value[0] != 0){
								var->var->set_string(temp_value);
							}
							if(temp_symbol[0] != 0) var->var->set_symbol(temp_symbol);
							if(temp_comment[0] != 0) var->var->set_comment(temp_comment);
						} else {
							error_log("ERROR CC025: Variable 'name' not found");
							error = ERROR;
						}
						if(strcmp(p->token(i), "/") == 0){ i += 2; token_count--;}
					} else {
						error_log("ERROR CC026: set argument not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___PROGRAM){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "ladder") == 0){
							token_count++; i++;
							token_stack[token_count] = ___PROG_LADDER;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--; i += 3;
						} else {
							error_log("ERROR CC027: unrecognized program content type");
							error = ERROR;
						}
					} else {
						error_log("ERROR CC028: Content of program definition not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___PROG_LADDER){
					if(strcmp(p->token(i), "location") == 0){
						file[token_count] = atoi(p->token(i+1));
						program_temp = cpu->prog->get_empty(file[token_count]);
						if(program_temp == -1) error = ERROR;
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), ">", 1) == 0){
						error = cpu->make_program(program_temp);
						if(error == ERROR) cpu->prog->program_delete(program_temp);
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						if(program_temp != file[token_count]){
							error = cpu->prog->program_delete(file[token_count]);
							error = cpu->prog->program_move(file[token_count], program_temp);
						}
						token_count--; i += 3;
					} else if((strcmp(p->token(i), "<") == 0) && (strcmp(p->token(i+1), "line") == 0)){
						token_count++; i++;
						token_stack[token_count] = ___PROG_LINE;
					} else {
						error_log("ERROR CC029: program definition type not correct");
						error = ERROR;
						cpu->prog->program_delete(program_temp);
					}
				} else if(token_stack[token_count] == ___PROG_LINE){ 
					if(strcmp(p->token(i), "instruction") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "//", 1) == 0)){
						token_count--; i += 3;
					} else if(strcmp(p->token(i), ">") == 0){
						if(token_stack[token_count-1] == ___PROG_LADDER){
							error = cpu->parse_line(temp_value);
						} else {
							error_log("ERROR CC030: Can't set that program Type");
							error = ERROR;
						}
					} else if((strcmp(p->token(i), "/") == 0) && (strcmp(p->token(i+1), ">") == 0)){
						if(token_stack[token_count-1] == ___PROG_LADDER){
							error = cpu->parse_line(temp_value);
						} else {
							error_log("ERROR CC030: Can't set that program Type");
							error = ERROR;
						}
						i += 2;
						token_count--;
					} else {
						error_log("ERROR CC031: set argument not recognized");
						error = ERROR;
					}
					if(error == ERROR) cpu->prog->program_delete(program_temp);
				} else if(token_stack[token_count] == ___IO){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "network") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_NETWORK;
						}else if(strcmp(p->token(i+1), "serial") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_SERIAL;
						}else if(strcmp(p->token(i+1), "card") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_CARD;
						}else if(strcmp(p->token(i+1), "stream") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_STREAM;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--; i += 3;
						} else {
							error_log("ERROR CC032: unrecognized io content type");
							error = ERROR;
						}
					} else if((strcmp(p->token(i), "/") == 0) && (strcmp(p->token(i+1), ">") == 0)){
						i += 2;
						token_count--;
					} else {
						error_log("ERROR CC033: Content of io definition not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___IO_NETWORK){ 
					if(strcmp(p->token(i), "port") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "connection") == 0){
						strcpy(temp_value2, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						if(strncmp(temp_value2, "worm", 4) == 0){
							io_number = in_out->add(_IO_NETWORK, _IO_ASYNC, atoi(temp_value), 0, NULL);
							error = add_worm(atoi(&(temp_value2[5])), io_number);
						} else if(strcmp(temp_value2, "synchronous") == 0){
							io_number = in_out->add(_IO_NETWORK, _IO_SYNC, atoi(temp_value), 0, temp_value);
							error = add_synchronous(io_number);
						} else if(strncmp(temp_value2, "program", 7) == 0){
							io_number = in_out->add(_IO_NETWORK, _IO_ASYNC, atoi(temp_value), 0, NULL);
							error = add_program(atoi(&(temp_value2[7])), io_number);
						} else {
							error_log("ERROR CC034: Can't set that network connection Type");
							error = ERROR;
						}
						if(strcmp(p->token(i), "<") == 0){ i+= 2; token_count--;}
					} else {
						error_log("ERROR CC035: network argument not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___IO_SERIAL){
					if(strcmp(p->token(i), "port") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "connection") == 0){
						strcpy(temp_value2, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "//", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						io_number = in_out->add(_IO_SERIAL, 0, 0, 0, temp_value);
						if(strncmp(temp_value2, "worm", 4) == 0){
							error = add_worm(atoi(&(temp_value2[5])), io_number);
						} else if(strcmp(temp_value2, "synchronous") == 0){
							error = add_synchronous(io_number);
						} else if(strncmp(temp_value2, "program", 7) == 0){
							error = add_program(atoi(&(temp_value2[7])), io_number);
						} else {
							error_log("ERROR CC036: Can't set that serial connection Type");
							error = ERROR;
						}
						if(strcmp(p->token(i), "/") == 0){ i += 2; token_count--;}
					} else {
						error_log("ERROR CC037: serial argument not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___IO_STREAM){
					if(strcmp(p->token(i), "port") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "connection") == 0){
						strcpy(temp_value2, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strcmp(p->token(i+1), "scan") == 0)){
						token_count++; i++;
						token_stack[token_count] = ___IO_SCAN;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "//", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						io_number = in_out->add(_IO_STREAM, 0, 0, 0, temp_value);
						if(strncmp(temp_value2, "worm", 4) == 0){
							error = add_worm(atoi(&(temp_value2[5])), io_number);
						} else if(strcmp(temp_value2, "synchronous") == 0){
							error = add_synchronous(io_number);
						} else if(strncmp(temp_value2, "program", 7) == 0){
							error = add_program(atoi(&(temp_value2[7])), io_number);
						} else {
							error_log("ERROR CC038: Can't set that keyboard connection Type");
							error = ERROR;
						}
						if(strcmp(p->token(i), "/") == 0){ i += 2; token_count--;}
					} else {
						error_log("ERROR CC039: keyboard argument not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___IO_CARD){
					if(strcmp(p->token(i), "connection") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "base") == 0){
						strcpy(temp_value2, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "type") == 0){
						strcpy(temp_value3, p->token(i+1));
						i++;

					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strcmp(p->token(i+1), "scan") == 0)){
						token_count++; i++;
						token_stack[token_count] = ___IO_SCAN;
						var = NULL;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						if(strcmp(temp_value3, "das08") == 0){
/*							io_number = in_out->add(_IO_DAS08, atoi(temp_value2), BIP10VOLTS, BIP10VOLTS, NULL); */
						} else {
							error_log("ERROR CC040: Card type not recognized");
							error = ERROR;
						}
						//if(strcmp(temp_value, "scan") == 0){
						//} else if(strncmp(temp_value, "program", 7) == 0){
						//	error = add_program(atoi(&(temp_value[7])), io_number);
						//} else {
						//	error_log("ERROR CC041: Can't set that card connection Type");
						//	error = ERROR;
						//}
						if(strcmp(p->token(i), "/") == 0){ token_count--; i += 2;}
					} else {
						error_log("ERROR CC042: card argument not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___IO_SCAN){
					if(strcmp(p->token(i), "memory") == 0){
						var = cpu->mem->find(p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "type") == 0){
						strcpy(temp_value2, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "channel") == 0){
						strcpy(temp_value3, p->token(i+1));
						i++;
					}else if(strcmp(p->token(i), "length") == 0){ // doesn't do anything now
						strcpy(temp_value4, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						// int file, word, bit, data_type, addr_type, channel;
						int	channel, data_type;
						error = in_out->command(io_number, _IO_DECODE, 0, 0, 0, temp_value3, 0.0, &channel, NULL, 0, NULL);

						// var = cpu->mem->find(temp_value);

						// cpu->decode_address(temp_value, &addr_type, &data_type, &file, &word, &bit);
						if(strcmp(temp_value2, "input") == 0){
							add_scan(io_number, _SCAN_INPUTS, data_type, var, 0, 1, channel);
						} else if(strcmp(temp_value2, "output") == 0){
							add_scan(io_number, _SCAN_OUTPUTS, data_type, var, 0, 1, channel);
						} else {
							error_log("ERROR CC043: input/output type not recognized");
							error = ERROR;
						}
						if(strcmp(p->token(i), "/") == 0){ i += 2; token_count--;}
					} else {
						error_log("ERROR CC044: scan argument not recognized");
						error = ERROR;
					}


				} else {
					error_log("ERROR CC045: Improperly formed plc file");
					error = ERROR;
				}
			}
		}
		fclose(fp_in);
	} else {
		error_log("ERROR CC046: Could Not open IO file");
		error = ERROR;
	}
	if(token_count >= 0 ){
		error_log("ERROR CC047: file was not properly formed");
		sprintf(temp_comment, "Line was [%s]\n", work);
		error_log(temp_comment);
		error = ERROR;
	}

	return error;
}





int controller::save_plc_file(const char *file_name, int parts, const char *filter){
	int	error;
	FILE	*fp_out;

	error = NO_ERROR;
//printf("SAVING  %d    [%s], [%s]\n", parts, file_name, filter);
	if((fp_out = fopen(file_name, "w")) != NULL){
		fprintf(fp_out, "<?xml version=\"1.0\" standalone=\"yes\"?>\n");
		fprintf(fp_out, "<!DOCTYPE plc SYSTEM \"plc.dtd\">\n");
		fprintf(fp_out, "\n<plc>\n");
		if((parts & _SAVE_MEMORY) != 0) error = save_memory(fp_out, filter);
		if((error == NO_ERROR) && ((parts & _SAVE_IO) != 0)) error = save_io_config(fp_out);
		if((error == NO_ERROR) && ((parts & _SAVE_PROGRAM) != 0)) error = save_program(fp_out);
		fprintf(fp_out, "\n</plc>\n");
		fclose(fp_out);
	}

	return error;
}




int controller::save_memory(FILE *fp_out, const char *filter){
	int		error;
	int		i;
	variable	*stack[20],
			*parent[20],
			*var_temp;
	int		array_flag[20];
	int		stack_pnt;
	char		*temp_name, *temp_symbol, *temp_comment, *temp_parent;

	error = NO_ERROR;
	fprintf(fp_out, "\n\t<memory>\n");
	stack_pnt = 0;
	stack[stack_pnt] = cpu->mem->list->child;
	parent[stack_pnt] = NULL;
	array_flag[0] = FALSE;
	while((stack_pnt >= 0) && (error == NO_ERROR)){
		if((stack[stack_pnt] != NULL) && (stack[stack_pnt]->var->type == _VARIABLE)){
			if((filter == NULL) || (filter[0] == 0) || ((check_filter(stack[stack_pnt]->var->get_name(), filter) == TRUE) || (check_filter(stack[stack_pnt]->var->get_symbol(), filter) == TRUE))){
				for(i = 0; i < 2+stack_pnt; i++) fprintf(fp_out, "\t");
				if(array_flag[stack_pnt] == TRUE){
					fprintf(fp_out, "<set value=\"%s\" ", stack[stack_pnt]->var->get_string());
				} else {
					if(stack[stack_pnt]->type == _INTEGER){
						fprintf(fp_out, "<integer value=\"%s\" ", stack[stack_pnt]->var->get_string());
						array_flag[stack_pnt+1] = FALSE;
					} else if(stack[stack_pnt]->type == _BIT){
						fprintf(fp_out, "<bit value=\"%d\" number=\"%d\" ", stack[stack_pnt]->var->get_bit(), stack[stack_pnt]->var->get_bit_number());
						array_flag[stack_pnt+1] = FALSE;
					} else if(stack[stack_pnt]->type == _REAL){
						fprintf(fp_out, "<real value=\"%s\" ", stack[stack_pnt]->var->get_string());
						array_flag[stack_pnt+1] = FALSE;
					} else if(stack[stack_pnt]->type == _STRING){
						fprintf(fp_out, "<string value=\"%s\" ", stack[stack_pnt]->var->get_string());
						array_flag[stack_pnt+1] = FALSE;
					} else if(stack[stack_pnt]->type == _ARRAY){
						fprintf(fp_out, "<array size=\"%d\" ", stack[stack_pnt]->var->get_size());
						var_temp = stack[stack_pnt]->var->get_variable(0);
						if(var_temp->type == _INTEGER) fprintf(fp_out, "data=\"integer\" ");
						if(var_temp->type == _BIT) fprintf(fp_out, "data=\"bit\" ");
						if(var_temp->type == _REAL) fprintf(fp_out, "data=\"real\" ");
						if(var_temp->type == _STRING) fprintf(fp_out, "data=\"string\" ");
						if(var_temp->type == _ARRAY) fprintf(fp_out, "data=\"array\" ");
						if(var_temp->type == _UNION) fprintf(fp_out, "data=\"union\" ");
						array_flag[stack_pnt+1] = TRUE;
					} else if(stack[stack_pnt]->type == _UNION){
						fprintf(fp_out, "<union value=\"%s\" ", stack[stack_pnt]->var->get_string());
						array_flag[stack_pnt+1] = FALSE;
					}
					if(parent[stack_pnt] != NULL){
						temp_parent = parent[stack_pnt]->var->get_name();
						if((temp_parent != NULL) && (strlen(temp_parent) > 0)) fprintf(fp_out, "parent=\"%s\" ", temp_parent);
					}
				}

				temp_name = stack[stack_pnt]->var->get_name();
				temp_symbol = stack[stack_pnt]->var->get_symbol();
				temp_comment = stack[stack_pnt]->var->get_comment();
				if((temp_name != NULL) && (strlen(temp_name) > 0))fprintf(fp_out, "name=\"%s\" ", temp_name);
				if((temp_symbol != NULL) && (strlen(temp_symbol) > 0))fprintf(fp_out, "symbol=\"%s\" ", temp_symbol);
				if((temp_comment != NULL) && (strlen(temp_comment) > 0))fprintf(fp_out, "comment=\"%s\" ", temp_comment);
				fprintf(fp_out, "/>\n");
			}
		}

		if(stack[stack_pnt] == NULL){
			stack_pnt--;
		} else if(stack[stack_pnt]->child != NULL){
			stack[stack_pnt+1] = stack[stack_pnt]->child;
			parent[stack_pnt+1] = stack[stack_pnt];
			stack[stack_pnt] = stack[stack_pnt]->next;
			stack_pnt++;
		} else if(stack[stack_pnt]->next != NULL){
			stack[stack_pnt] = stack[stack_pnt]->next;
		} else {
			stack_pnt--;
		}

//		 if(stack[stack_pnt] == NULL){
//			stack_pnt--;
//		} else if(stack[stack_pnt]->child != NULL){
//			stack[stack_pnt+1] = stack[stack_pnt]->child;
//			stack[stack_pnt] = stack[stack_pnt]->next;
//			stack_pnt++;
//		} else if(stack[stack_pnt]->next != NULL){
//
//			stack[stack_pnt] = stack[stack_pnt]->next;
//			// stack_pnt++;
//		} else {
//
//			stack_pnt--;
//		}
	}

	fprintf(fp_out, "\n\t</memory>\n");

	return error;
}


int	controller::check_filter(const char*text, const char*filter){
	int	state;
	int	flag;
	int	i;

	flag = TRUE;
	state = FALSE;
	for(i = 0; (i < 100 /*keep it safe*/) && (state == FALSE) && (flag == TRUE); i++){
		if(filter[i] == 0){
			if(text[i] == 0){
				state = TRUE;
			} else {
				flag = FALSE;
			}
		} else if(filter[i] == '*'){
			state = TRUE;
		} else if(text[i] == 0){
			flag = FALSE;
		} else if(filter[i] == '.'){
		} else if(filter[i] != text[i]){
			flag = FALSE;
		}
	}
	if(flag == TRUE) state = TRUE;

	return state;
}



int controller::save_io_config(FILE *fp_out){
	int	error;

	error = NO_ERROR;
	fprintf(fp_out, "\n\t<io>\n");
	
	fprintf(fp_out, "\n\t</io>\n");

	return error;
}


int controller::save_program(FILE *fp_out){
	int	error,
		i, j,
		type, used;
	char	text[100];

	error = NO_ERROR;
	fprintf(fp_out, "\n\t<program>\n");
	for(i = 0; i < cpu->prog->size(); i++){
		cpu->program_info(i, &used, &type);
		if(used == USED){
			if(type == _IL_PROGRAM){
				fprintf(fp_out, "\t\t<ladder location=\"%d\">\n", i);
			} else {
				error_log("Error CC054: program type in memory not recognized");
				error = ERROR;
			}
			for(j = 0; (j < 32000) && (error == NO_ERROR); j++){
				if(cpu->program_line(i, j, text) == NO_ERROR)
					fprintf(fp_out, "\t\t\t<line instruction=\"%s\"></line>\n", text);
			}
			fprintf(fp_out, "\t\t</ladder>\n");
		}
	}
	fprintf(fp_out, "\n\t</program>\n");

	return error;
}


int controller::password_check(char*user_name, char*password, user_data* data){
	int	error;
	FILE	*fp_in;
	char	work[200];
	char	work3[20];
	char	seed[2];
	int	user_number;
	int	flag1, flag2;
	int	p1, p2, p3, p4, p5, p6;

	error = ERROR;
	flag1 = 0; flag2 = 0;
	if((fp_in = fopen("passwd", "r")) != NULL){
		fgets(work, 200, fp_in);
		while((feof(fp_in) == 0) && (flag1 == 0)){
			for(p1 = 0; (p1 < 200) && (work[p1] != ':'); p1++){};
			for(p2 = p1+1; (p2 < 200) && (work[p2] != ':'); p2++){};
			for(p3 = p2+1; (p3 < 200) && (work[p3] != ':'); p3++){};
			if(p1 < 200){
//printf("Comparing [%s]==[%s] with [%s] \n", work, user_name, password);
				if(strncmp(work, user_name, p1) == 0){
					flag1 = 1;
					if((p3-p2) > 1){
						user_number = atoi(&(work[p1+1]));
						seed[0] = (char)(user_number & 255);
						seed[1] = (char)(user_number/256);
						strncpy(work3, &(work[p2+1]), p3-p2-1);
//printf("crypted is [%s]==[%s]  %d  \n",  crypt(password, seed), work3, p3-p2-1);
						if(strncmp(work3, crypt(password, seed), p3-p2-1) == 0){
							flag2 = 1;
						} else {
							error_log("WARNING: Invalid login attempt");
						}
					} else if((p3 < 200) && (strlen(password) == 0)) {
						user_number = atoi(&(work[p1+1]));
						flag2 = 1;
					} else {
						error_log("WARNING: password file improperly formed");
						error_log(work);
					}
				}
			} else {
				error_log("ERROR: Password file is improperly formed");
				error_log(work);
			}
			if(flag2 == 0) fgets(work, 200, fp_in);
		}
		if((flag1 == 1) && (flag2 == 1)){
			error = NO_ERROR;
			for(p4 = p3+1; (p4 < 200) && (work[p4] != ':'); p4++){};
			for(p5 = p4+1; (p5 < 200) && (work[p5] != ':'); p5++){};
			for(p6 = p5+1; (p6 < 200) && (work[p6] != ':'); p6++){};
			data->user_number = user_number;
			if(p4 < 200){ data->read_level = atoi(&(work[p3+1])); } else {data->read_level = 0;}
			if(p5 < 200){ data->write_level = atoi(&(work[p4+1])); } else {data->write_level = 0;}
			if(p5 < 200){ data->admin_level = atoi(&(work[p5+1])); } else {data->admin_level = 0;}
		} else if(flag1 == 0){
			error_log("WARNING: login user not found");
		}
		fclose(fp_in);
	} else {
		error_log("ERROR CC055: Password file not found");
	}

	return error;
}


int controller::password_change(user_data* user, char*password){
	int	error;
	FILE	*fp_in, *fp_out;
	char	work[200];
	char	work2[200];
	char	seed[2];
	int	p1, p2, p3;
	int	length;

	error = ERROR;
	if((fp_in = fopen("passwd", "r")) != NULL){
		if((fp_out = fopen("passwd_new", "w")) != NULL){
			fgets(work, 200, fp_in);
			while(feof(fp_in) == 0){
				length = strlen(work);
				for(p1 = 0; (p1 < 200) && (work[p1] != ':') && (p1 < length); p1++){};
				for(p2 = p1+1; (p2 < 200) && (work[p2] != ':') && (p2 < length); p2++){};
				for(p3 = p2+1; (p3 < 200) && (work[p3] != ':') && (p3 < length); p3++){};

				if(p2 < length){
					if(atoi(&(work[p1+1])) == user->user_number){
						work2[0] = 0;
						strncat(work2, work, p2+1);
						seed[0] = (char)(user->user_number & 255);
						seed[1] = (char)(user->user_number/256);
						strcat(work2, crypt(password, seed));
						strcat(work2, &(work[p3]));
						fprintf(fp_out, "%s", work2);
						error = NO_ERROR;
					} else {
						if(strlen(work) > 0) fprintf(fp_out, "%s", work);
					}
				} else {
					error_log("WARNING password file is not well formed");
					error_log(work);
					if(p1 < length) fprintf(fp_out, "%s", work);
				}
				work[0] = 0;
				fgets(work, 200, fp_in);
			}
			fclose(fp_out);
			fclose(fp_in);
			if(error == NO_ERROR){
				if((fp_in = fopen("passwd_new", "r")) != NULL){
					if((fp_out = fopen("passwd", "w")) != NULL){
						fgets(work, 200, fp_in);
						while(feof(fp_in) == 0){
							fprintf(fp_out, "%s", work);
							fgets(work, 200, fp_in);
						}
						fclose(fp_out);
					}
					fclose(fp_in);
				}
			}
		} else {
			error_log("ERROR: couldn't open password output file");
			fclose(fp_in);
			return error;
		}
	} else {
		error_log("ERROR: Can't open password file in");
	}

	return error;
}






