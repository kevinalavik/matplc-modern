
//
//    io.cpp - an IO scanner
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
// Last Modified: October 5, 2000
//


#include "queue.h"


queue::queue(int size){
	int	i;

	if(size >= MAXIMUM_QUEUE_LENGTH){
		error_log("WARNING: Requested queue size was too large and it is being reduced");
		size = MAXIMUM_QUEUE_LENGTH;
	}
	list = new communication_atom[size];
	if(list != NULL){
		maximum_size = size;
		for(i = 0; i < maximum_size; i++){
			list[i].state = _UNUSED;
			list[i].message = NULL;
		}
	} else {
		maximum_size = 0;
		error_log("ERROR: couldn't create communication queue");
	}
}


queue::~queue(){
	int	i;

	for(i = 0; i < maximum_size; i++){
		if((list[i].state != _UNUSED) && (list[i].state != _DONE)){
			error_log("ERROR: Communication was abandoned before completion");
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
			if(description != NULL){
				list[number].message = new char[strlen(description)+1];
				strcpy(list[number].message, description);
				list[number].state	= _WAITING;
			} else {
				error_log("ERROR: Can't accept an empty message");
				number = -1;
			}
			break;
		}
	}
 	if(i >= maximum_size){
		error_log("ERROR: communication table is full");
	}

	return number;
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
		error_log("ERROR: location outside valid range");
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
		error_log("ERROR: location outside valid range");
		error = ERROR;
	}

	return error;
}



int queue::scan(int location, char **description){
	int	error;

	error = NO_ERROR;
	if((location >= 0) && (location < maximum_size)){
		if(list[location].state == _UNUSED){
			// error_log("Warning: an unused location was scanned");
			error = ERROR;
		} else {
			description[0] = list[location].message;
// printf("String is <%s>\n", list[location].message);
		}
	} else {
		error_log("ERROR: requested communication location outside list");
		error = ERROR;
	}

	return error;
}


int queue::status(int location){
	if((location >= 0) && (location < maximum_size)){
		return list[location].state;
	} else {
		error_log("ERROR: Location outside communication table");
		return -1;
	}
}


int queue::status(int location, int state){
	int	error;

	error = NO_ERROR;
	if((location >= 0) && (location < maximum_size)){
		list[location].state = state;
	} else {
		error_log("ERROR: Location outside communication table");
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











