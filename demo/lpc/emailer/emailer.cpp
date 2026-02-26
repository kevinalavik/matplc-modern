/////////////////////////////////////////////////////////////////////////////////
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
// Last Modified: May 31, 2002
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <module_library.h>
#ifdef __cplusplus	// do this so C++ code will link
extern "C" {
#endif


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


unsigned char *tmp;
int		count;

int	init(){
	printf("EMAILER: Initializing\n");
	return 0;
	count = 0;
}



int	deinit(){
	printf("EMAILER: Shutting down \n");
	return 0;
}




int	step_run(){
	int	error;

	error = 0;
	count++;
	printf("EMAILER: A Run Step # %d\n", count);
	if(count == 5){
		error = send_message("MESSAGE email jackh@gvsu.edu~a test message~the message body");
	}
	if(count == 20){
		printf("EMAILER: Maximum count reached.... quitting \n");
		error = send_message("QUIT");
	}

	return error;
}



int	step_idle(){
	int	error;

	error = 0;
	printf("EMAILER: An Idle Step\n");

	return error;
}



int message_receive(char *text){
	printf("EMAILER: GOT MESSAGE [%s]\n", text);
	return 0;
}




#ifdef __cplusplus
}
#endif
