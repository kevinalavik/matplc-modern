
//
//    io.cpp - Lowlevel IO Routines
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
// Last Modified: April 10, 2001
//


#include "io_list.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif



io_list::io_list(){
	first = NULL;
	scan_focus = NULL;
}


io_list::~io_list(){
	for(; first != NULL;){
		remove(first);
	}
}


int io_list::remove(io_point *focus){
	int	error;

	error = NO_ERROR;
	if(focus == first){
		if(focus->next != NULL){
			first = first->next;
		} else {
			first = NULL;
		}
	} else {
		if(focus->next != NULL){
			focus->next->prev = focus->prev;
		}
		if(focus->prev != NULL){
			focus->prev->next = focus->next;
		}
	}
	delete focus->remote_name;
	delete focus;

	return error;
}


int io_list::add(int type, variable *var, char *remote){
	int error;
	io_point	*focus;

	error = NO_ERROR;
	focus = new io_point();
	focus->type = type;
	focus->var = var;
	focus->prev = NULL;
	focus->remote_name = new char[strlen(remote) + 1];
//	focus->var_remote = -1;
	strcpy(focus->remote_name, remote);
	if(first == NULL){
		first = focus;
		focus->next = NULL;
	} else {
		focus->next = first;
		first->prev = focus;
		first = focus;
	}

	return error;
}


int io_list::scan_first(int type){
	scan_focus = NULL;
	scan_type = type;
	return NO_ERROR;
}

//
// For an input scan all values are read in, but for an 
// output scan, only the output variables are returned
//
io_point *io_list::next(){
	if(scan_focus == NULL){
		for(scan_focus = first; scan_focus != NULL; scan_focus = scan_focus->next){
			if((scan_focus->type & scan_type) != 0) break;
		} 
	} else {
		for(scan_focus = scan_focus->next; scan_focus != NULL; scan_focus = scan_focus->next){
			if((scan_focus->type & scan_type) != 0) break;
		} 
	}
// if(scan_focus != NULL)printf("Match %d==%d :: ", scan_focus->type, scan_type);

	return scan_focus;
}



int io_list::dump(){
	io_point	*focus;

	printf("IO List DUMP:\n");
	for(focus = first; focus != NULL; focus = focus->next){
		if(focus->type && IO_INPUT) printf("   INPUT:  ");
		if(focus->type && IO_OUTPUT) printf("   OUTPUT: ");
		printf("var=%s    rem=%s  \n", focus->var->var->get_name(), focus->remote_name);
	}

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif


