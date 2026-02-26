/////////////////////////////////////////////////////////////////////////////////
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
// Last Modified: May 10, 2002
//



#include <global.h>
#include "user.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>

//////////////////////////////////////////////
//
// Individual User data class
//
//////////////////////////////////////////////

user_data::user_data(){
	write_level = LEVEL_NONE;
	read_level = LEVEL_NONE;
	admin_level = LEVEL_NONE;
	passwd = NULL;
	id = NULL;
	comments = NULL;
}

user_data::~user_data(){
	if(passwd != NULL) delete passwd;
	if(id != NULL) delete id;
	if(comments != NULL) delete comments;
}

int user_data::password(char* _passwd){
	if(passwd != NULL) delete passwd;
	passwd = new char[strlen(_passwd) + 1];
	strcpy(passwd, _passwd);
	return NO_ERROR;
}

int user_data::encode_password(char* _passwd){
	char	seed[2];
	seed[0] = id[0];
	seed[1] = id[1];
	return password(crypt(_passwd, seed));
}

char *user_data::password(){ return passwd;}

int user_data::identity(char* _id){

	if(id != NULL) delete id;
	id = new char[strlen(_id) + 1];
	strcpy(id, _id);
	return NO_ERROR;
}

char *user_data::identity(){ return id;}

int user_data::comment(char* _comments){
	if(comments != NULL) delete comments;
	comments = new char[strlen(_comments) + 1];
	strcpy(comments, _comments);
	return NO_ERROR;
}

char *user_data::comment(){ return comments;}


////////////////////////////////////////////////
//
// User information file
//
////////////////////////////////////////////////


user_t::user_t(){
	first = NULL;
}


user_t::~user_t(){
	while(first != NULL) delete_user(first);
}


user_data *user_t::find(char *name){
	user_data	*tmp;

	for(tmp = first; tmp != NULL; tmp = tmp->next){
		if(strcmp(name, tmp->id) == 0) break;
	}

	return tmp;
}


user_data *user_t::add_user(char *_id){
	user_data	*tmp;

	tmp = NULL;
	if((tmp = find(_id)) == NULL){
		tmp = new user_data();
		tmp->next = NULL;
		tmp->prev = NULL;
		tmp->identity(_id);
		tmp->password("*");
		tmp->read_level = LEVEL_NONE;
		tmp->write_level = LEVEL_NONE;
		tmp->admin_level = LEVEL_NONE;
		if(first == NULL){
			first = tmp; 
		} else {
			user_data	*next;
			for(next = first; next->next != NULL; next = next->next){}
			tmp->prev = next;
			next->next = tmp;
		}
	} else {
		error_log(MINOR, "User account already exists, using old");
	}

	return tmp;
}


int user_t::delete_user(user_data *focus){
	if(focus->next != NULL) focus->next->prev = focus->prev;
	if(focus->prev != NULL) focus->prev->next = focus->next;
	if(focus == first){
		first = focus->next;
	}
	delete focus;

	return NO_ERROR;
}


int user_t::load(){
	int	error;
	FILE	*fp_in;
	char	work[200];
	// char	work3[20];
	// char	seed[2];
	int	p[10], p_cnt;
	int	len;

	error = NO_ERROR;
	if((fp_in = fopen(PASSWORD_FILE, "r")) != NULL){
		fgets(work, 200, fp_in);
		while((feof(fp_in) == 0) && (error == NO_ERROR)){
			p_cnt = 0;
			p[p_cnt] = 0;
			len = strlen(work);
			for(int i = 0; i < len; i++){
				if(work[i] == ':'){
					work[i] = 0;
					p_cnt++;
					p[p_cnt] = i+1;
				}
			}
			if(p_cnt == 0){
				// do nothing, it is an empty line
			} else if(p_cnt == 5){
				user_data	*tmp;
				tmp = add_user(work);
				tmp->password(&(work[p[1]]));
				tmp->read_level = atoi(&(work[p[2]]));
				tmp->write_level = atoi(&(work[p[3]]));
				tmp->admin_level = atoi(&(work[p[4]]));
				tmp->comment(&(work[p[5]]));
			} else {
				error_log(MINOR, "Password file line not properly formed");
				error = ERROR;
			}
			
			fgets(work, 200, fp_in);
		}
		fclose(fp_in);
	} else {
		error_log(MINOR, "Could not open the password file");
		error = ERROR;
	}

	return error;
}



int user_t::save(){
	int	error;
	FILE	*fp_out;
	user_data	*tmp;

	error = NO_ERROR;
	if((first != NULL) && ((fp_out = fopen(PASSWORD_FILE, "w")) != NULL)){
		for(tmp = first; tmp != NULL; tmp = tmp->next){
			fprintf(fp_out, "%s:", tmp->identity());
			fprintf(fp_out, "%s:", tmp->password());
			fprintf(fp_out, "%d:", tmp->read_level);
			fprintf(fp_out, "%d:", tmp->write_level);
			fprintf(fp_out, "%d:", tmp->admin_level);
			if(tmp->comment() != NULL)
				fprintf(fp_out, "%s", tmp->comment());
			fprintf(fp_out, "\n");
		}
		fclose(fp_out);
	} else {
		error_log(MINOR, "Could not open password output file");
		error = ERROR;
	}

	return error;
}


int user_t::verify_password(user_data *data, char*_passwd){
	int	error;
	char	seed[2];

	error = NO_ERROR;
	if(data != NULL){
		seed[0] = data->id[0];
		seed[1] = data->id[1];
		if(strcmp(data->passwd, crypt(_passwd, seed)) == 0){
			// Do nothing, it was OK
		} else {
			error_log(MINOR, "User password check failed");
			error = ERROR;
		}
	} else {
		error_log(MINOR, "User data not defined");
		error = ERROR;
	}

	return error;
}




int user_t::dump(user_data *focus){
	int	error;

	error = NO_ERROR;
	if(focus != NULL){
		printf("Identity: [%s]\n", focus->identity());
		printf("    password: [%s]\n", focus->password());
		printf("    read level: %d\n", focus->read_level);
		printf("    write level: %d\n", focus->write_level);
		printf("    admin level: %d\n", focus->admin_level);
		if(focus->comment() != NULL)
			printf("    comment: [%s]\n", focus->comment());
	} else {
		printf("No user defined\n");
	}

	return error;
}



int user_t::dump(){
	int	error;
	user_data	*tmp;

	error = NO_ERROR;
	if(first != NULL){
		for(tmp = first; tmp != NULL; tmp = tmp->next){
			dump(tmp);
		}
	} else {
		printf("EMPTY LIST\n");
	}

	return error;
}



