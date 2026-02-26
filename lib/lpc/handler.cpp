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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sched.h> // for the sched_yield function

#include "handler_t.h"



/*
 * dynamic loader interface 
 */

int	parse_args(mat_handler*, int, char**);
void	print_usage(char*, mat_handler*);
int	loop(mat_handler*, int, char**);


int main(int argc, char **argv){

	int	loop_flag;
	mat_handler	*handler = new mat_handler();	// create the handler

	loop_flag = 0;
	// the command line arguments will be passed to the argument stack.
	if(parse_args(handler, argc, argv) == NO_ERROR){
		while(loop_flag == NO_ERROR){
			// this loop takes care of message passing
			loop_flag = loop(handler, argc, argv);
			// Note: if you are looking for the control loop, it
			// is not here, it is in handler_t.cpp file.
		}
	} else {
		print_usage(argv[0], handler);
		delete handler;
		return ERROR;
	}

	delete handler;
	return NO_ERROR;
}


int parse_args(mat_handler *handler, int argc, char **argv){
// this routine parses command line arguments for the handler and
// pushes them onto the message stack. The messages on the stack
// are interpreted later.
	char	*temp;
	if(argc <= 1){
		return 1;
	}
	for(int i = 1; i < argc;){
		if(argv[i][0] == '-'){ // a command is found
			if(argv[i][1] == 'h'){
				return 1;
			} else if(argv[i][1] == 'm'){
				i++; if(i >= argc) return 1;
				char *temp = new char[strlen(argv[i])+8];
				sprintf(temp, "MODULE %s", argv[i]);
				handler->queue->push(temp);
				delete temp;
				// handler->set_module_name(argv[i]);
				i++;
			} else if(argv[i][1] == 'l'){
				i++; if(i >= argc) return 1;
				char *temp = new char[strlen(argv[i])+9];
				sprintf(temp, "LIBRARY %s", argv[i]);
				handler->queue->push(temp);
				delete temp;
				// handler->set_file_name(argv[i]);
				i++;
			} else if(argv[i][1] == 'c'){
				i++; if(i >= argc) return 1;
				temp = new char[strlen(argv[i])+7];
				sprintf(temp, "PARSE %s", argv[i]);
				handler->queue->push(temp);
				delete temp;
				i++;
			} else {
				return ERROR;
			}
		} else {
			return ERROR;
		}
	}
	return NO_ERROR;
}


void print_usage(char *name, mat_handler *handler){
	printf("\n%s, version code=%d, %s\n\n", name,
		handler->version(),
		handler->version_date());
	printf("usage: %s [-h] [-m module_name] [-c command_file] [-l module_library]\n\n", name);
	printf("  h : this help message\n");
	printf("  m : assign the name of the module in the mat PLC\n");
	printf("  c : the name of a file to parse command from \n");
	printf("      e.g., LOAD file, RUN, HALT, QUIT, PARSE file\n");
	printf("  l : provide the name of the shared library file of the module\n");
}


int loop(mat_handler *handler, int argc, char**argv){
// This routine runs aynchronously and it does not call the step_run and step_idle functions.
// What it does do is check for messages and forwards them to the process.
	int	status;

	status = handler->step(argc, argv);
	sched_yield();	// give up the processor to other processes

	return status;
}
