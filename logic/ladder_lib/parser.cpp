
//
//    parser.cpp - a quick and dirty parser
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


#include <stdlib.h>
#include <stream.h>
#include <string.h>

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif


parser::parser()
{
	int	max_size;

	max_size = MAX_NUMBER_TOKENS;
	end_of_list = 0;
	new_flag = 1;
	pnt_end_list = max_size - 1;
}

parser::~parser()
{
	int i;
	if(new_flag == 0){
		for(i = 0; i < end_of_list; i++){
			delete tokens[i];
		}
		delete tokens_string;
	}
}


int	parser::counter()
{
	return end_of_list;
}

int	parser::parse(char *string, char delimeter)
{
	int	error,
		i,
		count,
		pos,
		end,
		flag1,
		string_flag;

	if(new_flag == 0){
		for(i = 0; i < end_of_list; i++){
			delete tokens[i];
		}
		end_of_list = 0;
		delete tokens_string;
	}
	new_flag = 0;
	tokens_string = new char[strlen(string)+1];
	if(tokens_string == NULL){
		error_log(MINOR, "ERROR PAR1: Out of Memory");
		error = ERROR;
		//exit(1);
	}
	error = NO_ERROR;
	count = 0;
	pos = 0;
	flag1 = 1;
	string_flag = 0;
	tokens_string[0] = 0;
	for(end = strlen(string)-1; (end >= 0) && (string[end] == ' '); end--);
	for(i = 0; i <= end; i++){
		if((string[i] == '\"') && (string[i-1] != '\\')){
			if(string_flag == 0){string_flag = 1;
			} else {string_flag = 0;}
		}
		if(((string[i] != delimeter)
			&& (string[i] != 10) && (string[i] != '\t') && (string[i] != '\n'))
			|| (string_flag == 1)){
			flag1 = 0;
			if(string[i] != '\\'){
				tokens_string[pos] = string[i];
				pos++;
			}
		} else {
			if(flag1 == 0){
				tokens_string[pos] = 0;
				if(count >= pnt_end_list){
					error_log(MINOR, "ERROR PAR2: Out of Memory");
					error = ERROR;
					//exit(1);
				}
				tokens[count] = new char[strlen(tokens_string) + 1];
				if(tokens[count] == NULL){
					error_log(MINOR, "ERROR PAR3: Out of Memory");
					error = ERROR;
					//exit(1);
				}
				strcpy(tokens[count], tokens_string);
				count++;
				pos = 0;
				flag1 = 1;
				tokens_string[0] = 0;
			}
		}
	}
	if((count >= 0) && (strlen(tokens_string) < 1)){
		count--;
	} else {
		tokens_string[pos] = 0;
		if(count >= pnt_end_list){
			error_log(MINOR, "ERROR PAR4: Out of Memory ");
			exit(1);
		}
		tokens[count] = new char[strlen(tokens_string) + 1];
		if(tokens[count] == NULL){
			error_log(MINOR, "ERROR PAR5: Out of Memory");
			error = ERROR;
			//exit(1);
		}
		strcpy(tokens[count], tokens_string);
	}
	end_of_list = count + 1 ;

	return error;
}

int	parser::parse_xml(char *string)
{
	int	error,
		i,
		count,
		pos,
		end,
		token_flag,
		string_flag,
		offset;

	offset = 'a' - 'A';
	if(new_flag == 0){
		for(i = 0; i < end_of_list; i++){
			delete tokens[i];
		}
		end_of_list = 0;
		delete tokens_string;
	}
	new_flag = 0;
	tokens_string = new char[strlen(string)+1];
	if(tokens_string == NULL){
		error_log(MINOR, "ERROR PAR1: Out of Memory");
		error = ERROR;
		//exit(1);
	}
	error = NO_ERROR;
	count = 0;
	pos = 0;
	token_flag = 0;
	string_flag = 0;
	tokens_string[0] = 0;
	for(end = strlen(string)-1; (end >= 0) && (string[end] == ' '); end--);
	for(i = 0; i <= end; i++){
		if(string[i] == '\"'){
			if(string_flag == 0){string_flag = 1;
			} else {string_flag = 0;}
		}
		if(string_flag == 0){	// Not a string
			if(string[i] == '<'){
				if(strlen(tokens_string) > 0){
					i--;
				} else {
					tokens_string[pos] = '<';
					pos++;
				}
				token_flag = 1;
			} else if(string[i] == '>'){
				if(strlen(tokens_string) > 0){
					i--;
				} else {
					tokens_string[pos] = '>';
					pos++;
				}
				token_flag = 1;
			} else if(string[i] == '/'){
				if(strlen(tokens_string) > 0){
					i--;
				} else {
					tokens_string[pos] = '/';
					pos++;
				}
				token_flag = 1;
			} else if((string[i] == ' ') || (string[i] == 10) || (string[i] == '\t') || (string[i] == '\n') || (string[i] == '\r') || (string[i] == '=') || (string[i] == '\"')){
				if(strlen(tokens_string) > 0) token_flag = 1;
			} else {
// leave the variables in upper or lower case
//				if((string[i] >= 'A') && (string[i] <= 'Z')){
//					tokens_string[pos] = string[i] + offset;
//				} else {
					tokens_string[pos] = string[i];
//				}
				pos++;
			}

		} else { // The item is a string
			if(string[i] != '\"'){
				if((string[i] == '\\') && (string[i+1] == '\"')){
					tokens_string[pos] = '\"';
					pos++;
					i++;
				} else if((string[i] == '\\') && (string[i+1] == '\\')){
					tokens_string[pos] = '\\';
					pos++;
					i++;
				} else {
					tokens_string[pos] = string[i];
					pos++;
				}
			}
		}


		if(token_flag == 1){
			tokens_string[pos] = 0;
			if(count >= pnt_end_list){
				error_log(MINOR, "ERROR PAR2: Out of Memory");
				error = ERROR;
				return error;
				//exit(1);
			}
			tokens[count] = new char[strlen(tokens_string) + 1];
			if(tokens[count] == NULL){
				error_log(MINOR, "ERROR PAR3: Out of Memory");
				error = ERROR;
				//exit(1);
			}
			strcpy(tokens[count], tokens_string);
			count++;
			pos = 0;
			token_flag = 0;
			tokens_string[0] = 0;
		}
	}
	if((count >= 0) && (strlen(tokens_string) < 1)){
		count--;
	} else {
		tokens_string[pos] = 0;
		if(count >= pnt_end_list){
			error_log(MINOR, "ERROR PAR4: Out of Memory ");
			exit(1);
		}
		tokens[count] = new char[strlen(tokens_string) + 1];
		if(tokens[count] == NULL){
			error_log(MINOR, "ERROR PAR5: Out of Memory");
			error = ERROR;
			//exit(1);
		}
		strcpy(tokens[count], tokens_string);
	}
	end_of_list = count + 1 ;

	return error;
}

char* parser::token(int number)
{
	if((number >= 0) && (number <= end_of_list)){
		return tokens[number];
	} else {
		error_log(MINOR, "ERROR PAR6: Token index outside list");
		return NULL;
	}
}

char* parser::token_string(int from, int to)
{
	int	i;

	tokens_string[0] = 0;
	for(i = from; (i <= to) && (i < end_of_list); i++){
		if((i != from) && (strlen(tokens[i]) > 0)){
			strcat(tokens_string, " ");
		}
		strcat(tokens_string, tokens[i]);
	}

	return tokens_string;
}

char* parser::token_string(int from, char *to)
{
	int	i;

	tokens_string[0] = 0;
	for(i = from; (strncmp(tokens[i], to, strlen(to)) != 0) && (i < end_of_list); i++){
		if((i != from) && (strlen(tokens[i]) > 0)){
			strcat(tokens_string, " ");
		}
		strcat(tokens_string, tokens[i]);
	}

	return tokens_string;
}



#ifdef __cplusplus
}
#endif


