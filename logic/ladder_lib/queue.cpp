
//
//    queue.cpp - a message queue manager
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
// Last Modified: April 3, 2001
//


#include "queue.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


queue::queue(int size){
	int	i;

	if(size >= MAXIMUM_QUEUE_LENGTH){
		error_log(MINOR, "WARNING: Requested queue size was too large and it is being reduced");
		size = MAXIMUM_QUEUE_LENGTH;
	}
	list = new communication_atom[size];
	if(list != NULL){
		maximum_size = size;
		for(i = 0; i < maximum_size; i++){
			list[i].state = _UNUSED;
			list[i].message = NULL;
			list[i].managed = _MANAGED;
		}
	} else {
		maximum_size = 0;
		error_log(MINOR, "ERROR: couldn't create communication queue");
	}
}


queue::~queue(){
	int	i;

	for(i = 0; i < maximum_size; i++){
		if((list[i].state != _UNUSED) && (list[i].state != _DONE)){
			error_log(MINOR, "ERROR: Communication was abandoned before completion");
		}
//printf("DIE 0   (%d) \n", i);
		if(list[i].message != NULL) delete list[i].message;
//printf("DIE 1 \n");
	}
	if(list != NULL) delete list;
}

int queue::add(char *description){
	int	number,
		i;

	number = -1;
//printf("ADDING <%s>\n", description);
	for(i = 0; (number == -1) && (i < maximum_size); i++){
		if(list[i].state == _UNUSED){
			number = i;
			list[number].managed = _MANAGED;
			if(description != NULL){
				list[number].message = new char[strlen(description)+1];
				strcpy(list[number].message, description);
				list[number].state	= _WAITING;
			} else {
				error_log(MINOR, "ERROR: Can't accept an empty message");
				number = -1;
			}
			break;
		}
	}
 	if(i >= maximum_size){
		error_log(MINOR, "ERROR: communication table is full");
	}

	return number;
}


int queue::manage(int location, int managed_state){
	int	error;

	if((location >= 0) && (location < maximum_size)){
		list[location].managed = managed_state;
	} else {
		error_log(MINOR, "ERROR: location outside valid range");
		error = ERROR;
	}

	return error;
}


int queue::update(int location, char *description){
	int	error;
	char	*temp;

	error = NO_ERROR;
	if((location >= 0) && (location < maximum_size)){
		temp = new char[strlen(description)+1];
		strcpy(temp, description);
		if(list[location].message != NULL) delete list[location].message;
		list[location].message = temp;
	} else {
		error_log(MINOR, "ERROR: location outside valid range");
		error = ERROR;
	}

	return error;
}



int queue::append(int location, char *description){
	int	error;
	char	*temp;

	error = NO_ERROR;
	if((location >= 0) && (location < maximum_size)){
		temp = new char[strlen(list[location].message) + strlen(description) + 2];
		strcpy(temp, list[location].message);
		strcat(temp, " ");
		strcat(temp, description);
		if(list[location].message != NULL) delete list[location].message;
		list[location].message = temp;
	} else {
		error_log(MINOR, "ERROR: location outside valid range");
		error = ERROR;
	}

	return error;
}



int queue::scan(int location, char **description){
	int	error;

	error = NO_ERROR;
	if((location >= 0) && (location < maximum_size)){
		if(list[location].state == _UNUSED){
			// error_log(MINOR, "Warning: an unused location was scanned");
			error = ERROR;
		} else {
			description[0] = list[location].message;
// printf("String is <%s>\n", list[location].message);
		}
	} else {
		error_log(MINOR, "ERROR: requested communication location outside list");
		error = ERROR;
	}

	return error;
}


int queue::status(int location){
	if((location >= 0) && (location < maximum_size)){
		return list[location].state;
	} else {
		error_log(MINOR, "ERROR: Location outside communication table");
		return -1;
	}
}


int queue::status(int location, int state){
	int	error;

	error = NO_ERROR;
	if((location >= 0) && (location < maximum_size)){
		if((state == _DONE) && (list[location].managed == _UNMANAGED)){
			list[location].managed = _MANAGED;
			release(location);
		} else {
			list[location].state = state;
		}
	} else {
		error_log(MINOR, "ERROR: Location outside communication table");
		error = ERROR;
	}

	return error;
}


int queue::release(int location){
	int	error;

	error = status(location, _UNUSED);
	if(list[location].message != NULL){
		delete list[location].message;
		list[location].message = NULL;
	}

	return error;
}



void queue::dump(){
	int	i;

	printf("\nCommunications queue contents\n");
	for(i = 0; i < maximum_size; i++){
		if(list[i].state == _UNUSED)
			printf("%3d: (_UNUSED)  (NULL)\n", i);
		if(list[i].state == _WAITING)
			printf("%3d: (_WAITING) (%s)\n", i, list[i].message);
		if(list[i].state == _DONE)
			printf("%3d: (_DONE)    (%s)\n", i, list[i].message);
		if(list[i].state == _ERROR)
			printf("%3d: (_ERROR)   (%s)\n", i, list[i].message);
	}
}





#ifdef __cplusplus
}
#endif










