/////////////////////////////////////////////////////////////////////////////////
//
// A test process to verify the operation of the dynamic linker
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
//
//    Copyright (C) 2000-2 by Hugh Jack <jackh@gvsu.edu>
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
#include <string.h>
#include <stdlib.h>


user_t		*users;
user_data	*focus;


void menu_main();
void menu_edit();
void user_select();
void user_add();
void edit_password();
void edit_read_level();
void edit_write_level();
void edit_admin_level();
void edit_comment();

int main(){
	users = new user_t();
	users->load();
	focus = NULL;
	menu_main();

	delete users;

	return 0;
}



void	menu_main(){
	int	choice;
	char	text[100];
	int	flag;

	flag = 0;
	while(flag == 0){
		if(focus == NULL){
			printf("\nMAIN MENU: no user selected\n");
		} else {
			printf("\nMAIN MENU: user=[%s] \n", focus->identity());
		}
		printf("  1. Select User Account \n");
		printf("  2. Add User Account \n");
		printf("  3. Delete User Account \n");
		printf("  4. Edit User Account \n");
		printf("  5. Dump All Accounts \n");
		printf("  8. Save and quit\n");
		printf("  9. Quit without saving\n");
		printf("  -----------------------\n");
		printf("  Choice: ");
		gets(text);
		choice = atoi(text);
		if(choice == 1){
			user_select();
		} else if(choice == 2){
			user_add();
		} else if((choice == 3) && (focus != NULL)){
			users->delete_user(focus);
			focus = NULL;
		} else if((choice == 4) && (focus != NULL)){
			menu_edit();
		} else if(choice == 5){
			users->dump();
		} else if(choice == 8){
			users->save();
			flag = 1;
		} else if(choice == 9){
			flag = 1;
		} else {
			printf("Choice not recognized\n");
		}
	}
}




void	menu_edit(){
	int	choice;
	char	text[100];
	int	flag;

	flag = 0;
	while(flag == 0){
		printf("\nCURRENT USER----------\n");
		users->dump(focus);
		printf("EDIT MENU-------------\n");
		printf("  1. Change Password \n");
		printf("  2. Change Read Level \n");
		printf("  3. Change Write Level \n");
		printf("  4. Change Admin Level \n");
		printf("  5. Change User Comment \n");
		printf("  9. Return to main menu \n");
		printf("  -----------------------\n");
		printf("  Choice: ");
		gets(text);
		choice = atoi(text);
		if(choice == 1){
			edit_password();
		} else if(choice == 2){
			edit_read_level();
		} else if(choice == 3){
			edit_write_level();
		} else if(choice == 4){
			edit_admin_level();
		} else if(choice == 5){
			edit_comment();
		} else if(choice == 9){
			flag = 1;
		} else {
			printf("Choice not recognized\n");
		}
	}
}


void	user_select(){
	char	text[100];

	printf("  Enter User Name: ");
	gets(text);
	if(strlen(text) > 0){
		focus = users->find(text);
	} else {
		focus = NULL;
	}
}



void	user_add(){
	char	text[100];

	printf("  Enter New User Name: ");
	gets(text);
	if(strlen(text) > 0){
		focus = users->find(text);
		if(focus == NULL){
			focus = users->add_user(text);
		} else {
			printf("User already Exists \n");
		}
	} else {
		focus = NULL;
	}
}



void	edit_password(){
	char	text[100];

	printf("  New Password: ");
	gets(text);
	if(strlen(text) > 0){
		focus->encode_password(text);
	}
}



void	edit_comment(){
	char	text[100];

	printf("  New Comment: ");
	gets(text);
	focus->comment(text);
}




void	edit_read_level(){
	char	text[100];

	printf("  New Read Level: (%d=none, %d=all) \n", LEVEL_NONE, LEVEL_HIGH);
	gets(text);
	if((strlen(text) > 0) && (atoi(text) >= LEVEL_NONE) && (atoi(text) <= LEVEL_HIGH)){
		focus->read_level = atoi(text);
	}
}




void	edit_write_level(){
	char	text[100];

	printf("  New Write Level: (%d=none, %d=all) \n", LEVEL_NONE, LEVEL_HIGH);
	gets(text);
	if((strlen(text) > 0) && (atoi(text) >= LEVEL_NONE) && (atoi(text) <= LEVEL_HIGH)){
		focus->write_level = atoi(text);
	}
}




void	edit_admin_level(){
	char	text[100];

	printf("  New Admin Level: (%d=none, %d=all) \n", LEVEL_NONE, LEVEL_HIGH);
	gets(text);
	if((strlen(text) > 0) && (atoi(text) >= LEVEL_NONE) && (atoi(text) <= LEVEL_HIGH)){
		focus->admin_level = atoi(text);
	}
}


void error_log(int code, char *mess){
	printf("Error Message: Level %d: %s \n", code, mess);
}

