//
//    comments.cpp - Code for the comment and symbol memory
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
//


#include "comments.h"


comments_list::comments_list(){
	first = NULL;
}



comments_list::~comments_list(){
	while(delete_comment(0) != ERROR){};	
}



int comments_list::delete_comment(int line){
	int		error,
			i;
	comment_element	*next,
			*old_line;

	error = NO_ERROR;
	if(first != NULL){
		if(line == 0){
			next = first->next;
			delete first->text;
			delete first;
			first = next;
		} else {
			next = first;
			for(i = 0; (next->next != NULL) && (i < line); i++){
				old_line = next;
				next = next->next;
			}
			if(i == line){
				old_line->next = next->next;
				delete next->text;
				delete next;
			} else {
				error_log("Error CMT1: delete comment after end of list");
				error = ERROR;
			}
		}
	} else {
		// error_log("Error COMx: Can't delete from an empty list");
		error = ERROR;
	}
	
	return error;
}



int comments_list::append_comment(char *_text){
	int		error;
	comment_element	*next;

	error = NO_ERROR;
	if(first != NULL){
		next = first;
		while(next->next != NULL) next = next->next;

		next->next = new comment_element();
		next = next->next;
	} else {
		first = new comment_element();
		next = first;
	}

	next->text = new char[strlen(_text)+1];
	strcpy(next->text, _text);
	
	return error;
}



int comments_list::insert_comment(int line, char *_text){
	int		error,
			i;
	comment_element	*next,
			*new_line,
			*last_next;

	error = NO_ERROR;
	if(line == 0){
		new_line = new comment_element();
		new_line->next = first;
		first = new_line;
	} else if(first != NULL){
		next = first;
		for(i = 0; (next->next != NULL) && (i < line); i++){
			last_next = next;
			next = next->next;
		}
		if(i == line){
			new_line = new comment_element();
			new_line->next = last_next->next;
			last_next->next = new_line;
		} else {
			error_log("Error CMT2: insert comment after end of list");
			return ERROR;
		}
	} else {
		error_log("Error CMT3: Can't insert into an empty list");
		return ERROR;
	}

	new_line->text = new char[strlen(_text)+1];
	strcpy(new_line->text, _text);
	
	return error;
}



int comments_list::get_comment(int line, char *_text){
	int		error,
			i;
	comment_element	*next;

	error = ERROR;
	if(first != NULL){
		next = first;
		for(i = 0; (next->next != NULL) && (i < line); i++){
			next = next->next;
		}
		if(i == line){
			strcpy(_text, next->text);
			error = NO_ERROR;
		} else {
			// Stay quiet here
			// error_log("Warning CMTx: Can't get comment after end of list");
		}
	} else {
//		error_log("Error CMTx: Can't get a comment from an empty list");
	}
	
	return error;
}



int comments_list::set_comment(int instruction, int line, char *_text){
	int		error;

	error = ERROR;
	if(instruction == __APPEND_COMMENT){
		error = append_comment(_text);
	} else if(instruction == __REPLACE_COMMENT){
		error = insert_comment(line, _text);
		if(error == NO_ERROR) error = delete_comment(line+1);
	} else if(instruction == __INSERT_COMMENT){
		error = insert_comment(line, _text);
	} else if(instruction == __DELETE_COMMENT){
		error = delete_comment(line);
	} else {
		error_log("Error CMT4: Don't recognize comment edit command");
	}
	
	return error;
}



int comments_list::find_comment(char* _text, int *location){
	int		error;
	comment_element	*next;

	error = NO_ERROR;
	location[0] = 0;
	for(next = first, location[0] = 0;
		(next != NULL) && (strcmp(_text, next->text) != 0);
		next = next->next, location[0]++){}
	if(next == NULL) error = ERROR;

	return error;
}

