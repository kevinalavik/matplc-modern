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
// Last Modified: April 21, 2002
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>


#ifdef __cplusplus	// do this so C++ code will link
extern "C" {
#endif
#include <module_library.h>


int count;


char *mod_name;

int	init(){
//	char	*temp;

	mod_name = get_argument(0, "MODULE_NAME");
	error_log(MINOR, "Starting Handler");
	if(mod_name != NULL){
		printf("The module name is [%s]\n", mod_name);
	} else {
		printf("ERROR: Module name not defined\n");
	}
	printf("LIBRARY: Initializing\n");
	count = 0;

	return 0;
}



int	step_run(){
	int	error;
	char	*temp;
	int	i;

	error = 0;
	count++;
	printf("LIBRARY: A Run Step # %d\n", count);

	if(count == 5){
		printf("Defined argument variables -- LOCAL\n");
		for(i = 1; ; i++){
			temp = get_argument(i, NULL);
			if(temp != NULL){
				printf("  Var: [%s] ", temp);
				temp = get_argument(0, temp);
				if(temp != NULL){
					printf(" = [%s]", temp);
				}
				printf("\n");
			} else {
				break;
			}
		}

		printf("Defined argument variables -- GLOBAL\n");
		for(i = -1; ; i--){
			temp = get_argument(i, NULL);
			if(temp != NULL){
				if(temp != (char*) -1){
					printf("   Var--> %s", temp);
					temp = get_argument(0, temp);
					if(temp != NULL){
						printf(" = [%s]", temp);
					}
					printf("\n");
				} else {
					// somebody elses variable, ignore it
				}
			} else {
				break;	// end of the list
			}
		}

	}

	if(count == 10){
		printf("Maximum count reached.... quitting \n");
		error = send_message("QUIT");
	}

	return error;
}



char	*argument_descriptions(int number){
	if(number == 0) return (char*)"This is a test line";
	return NULL;
}



#ifdef __cplusplus
}
#endif
