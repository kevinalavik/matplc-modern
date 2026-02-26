
//
//    store.cpp - a memory integration class
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
//    Last Modified: April 3, 2001
//


#include "store.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


memory::memory(){
	list = new variable();
	list->next = NULL;
	list->child = NULL;
}


memory::~memory(){
	variable	*temp;
	variable	*stack[20];
	int		stack_point;
	stack_point = -1;
	if(list != NULL){ stack_point++; stack[stack_point] = list;}
	while(stack_point >= 0){
		if(stack[stack_point]->child != NULL){
			stack[stack_point+1] = stack[stack_point]->child;
			stack[stack_point]->child = NULL;
			stack_point++;
		} else if(stack[stack_point]->next != NULL){
			temp = stack[stack_point];
			stack[stack_point] = stack[stack_point]->next;
			delete temp;
		} else {
			delete stack[stack_point];
			stack_point--;
		}
	}
}


variable	*memory::add(variable *parent, char *_name, char *_symbol, char *_comment, int type){
	int		error;
	variable	*parent_variable,
			*new_variable;	

	error = NO_ERROR;
	if(parent == NULL){
		parent_variable = list;
	} else {
		parent_variable = parent;
	}
	new_variable = new variable();

	if(type == _INTEGER){		new_variable->var = new integer();
	} else if(type == _ARRAY){	new_variable->var = new array();
	} else if(type == _REAL){	new_variable->var = new real();
	} else if(type == _STRING){	new_variable->var = new string();
	} else if(type == _UNION){	new_variable->var = new integer();
	} else if(type == _BIT){	new_variable->var = new bit(); new_variable->var->set_var(0, parent);
	} else {					error = ERROR; error_log(MINOR, "ERROR: Incorrect variable type");
	}

	if(error == NO_ERROR){
		new_variable->var->type = _LITERAL;
		if(_name != NULL){new_variable->var->set_name(_name); new_variable->var->type = _VARIABLE;}
		if(_symbol != NULL){new_variable->var->set_symbol(_symbol); new_variable->var->type = _VARIABLE;}
		if(_comment != NULL){new_variable->var->set_comment(_comment);}
		new_variable->next = parent_variable->child;
		parent_variable->child = new_variable;
		new_variable->child = NULL;
		new_variable->type = type;
	} else {
		delete new_variable;
		new_variable = NULL;
	}

	return new_variable;
}


variable *memory::find(variable *parent, char *name){
	variable	*current;
	variable	*result;
	char		*temp;

	if(parent == NULL){
		current = list->child;
	} else {
		current = parent->child;
	}

	result = NULL;
	while((result == NULL) && (current != NULL)){
		if(current->var->type == _VARIABLE){
			temp = current->var->get_name();
			if((temp != NULL) && (strcmp(temp, name) == 0)) result = current;
			temp = current->var->get_symbol();
			if((temp != NULL) && (strcmp(temp, name) == 0)) result = current;
		}
		current = current->next;
	}

	return result;
}


variable *memory::find(char *symbol){
	variable	*stack[20];
	int		stack_point;
	variable	*result;
	char		*temp;

	stack_point = -1;
	result = NULL;
	if(list != NULL){ stack_point++; stack[stack_point] = list->child;}
	while((stack_point >= 0) && (result == NULL)){
		if(stack[stack_point] != NULL){
			temp = stack[stack_point]->var->get_symbol();
			if((temp != NULL) && (strcmp(temp, symbol) == 0)) result = stack[stack_point];
			temp = stack[stack_point]->var->get_name();
			if((temp != NULL) && (strcmp(temp, symbol) == 0)) result = stack[stack_point];
		}
		if(stack[stack_point] == NULL){
			stack_point--;
		} else if(stack[stack_point]->child != NULL){
			stack[stack_point+1] = stack[stack_point]->child;
			stack[stack_point] = stack[stack_point]->next;
			stack_point++;
		} else if(stack[stack_point]->next != NULL){
			stack[stack_point] = stack[stack_point]->next;
		} else {
			stack_point--;
		}
	}

	return result;
}


variable *memory::find_bit(variable *start, int bit){
	variable	*next;
	for(next = start->child; next != NULL; next = next->next){
		if(next->var->get_bit_number() == bit) return next;
	}
	return NULL;
}


void	memory::dump_all(){
	int		i;
	// variable	*temp;
	variable	*stack[20];
	int		stack_point;

	stack_point = -1;
	if(list != NULL){ stack_point++; stack[stack_point] = list->child;}
	printf("Memory dump\n");
	while(stack_point >= 0){
		if(stack[stack_point] != NULL){
			for(i = 0; i < stack_point; i++) printf("  ");
			if(stack[stack_point]->var->type == _VARIABLE){
				printf("VARIABLE: ");
			} else {
				printf("LITERAL: ");
			}
			if(stack[stack_point]->var->get_name()!=NULL)printf("%s: ", stack[stack_point]->var->get_name());
			if(stack[stack_point]->var->get_symbol()!=NULL)printf("%s: ", stack[stack_point]->var->get_symbol());
			if(stack[stack_point]->var->get_comment()!=NULL)printf("%s: ", stack[stack_point]->var->get_comment());
			if(stack[stack_point]->type == _INTEGER){printf(" = %d",stack[stack_point]->var->get_int());}
			if(stack[stack_point]->type == _BIT){ printf(" = %d", stack[stack_point]->var->get_bit());}
			if(stack[stack_point]->type == _REAL){ printf(" = %f",stack[stack_point]->var->get_real());}
			if(stack[stack_point]->type == _STRING){ printf(" = %s", stack[stack_point]->var->get_string());}
			if(stack[stack_point]->type == _UNION){ printf(" UNION = %d",stack[stack_point]->var->get_int());}
			if(stack[stack_point]->type == _ARRAY){ printf(" ARRAY[] ");}
			printf("\n");
		}
		if(stack[stack_point] == NULL){
			stack_point--;
		} else if(stack[stack_point]->child != NULL){
			stack[stack_point+1] = stack[stack_point]->child;
			stack[stack_point] = stack[stack_point]->next;
			stack_point++;
		} else if(stack[stack_point]->next != NULL){
			stack[stack_point] = stack[stack_point]->next;
		} else {
			stack_point--;
		}
	}
	printf("Done.\n");
}



data::data(){ name = NULL; symbol = NULL; comment = NULL;};
data::~data(){if(name != NULL) delete name; if(symbol != NULL) delete symbol; if(comment != NULL) delete comment;};
int	data::get_type(){return _VOID;};
char*	data::get_symbol(){return symbol;};
void	data::set_symbol(char*value){if(symbol != NULL) delete symbol; symbol = new char[strlen(value)+1]; strcpy(symbol, value);};
char*	data::get_name(){return name;};
void	data::set_name(char*value){if(name != NULL) delete name; name = new char[strlen(value)+1]; strcpy(name, value);};
char*	data::get_comment(){return comment;};
void	data::set_comment(char*value){if(comment != NULL) delete comment; comment = new char[strlen(value)+1]; strcpy(comment, value);};


char _return_string[30];
unsigned int __bits[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};


array::array(){size = 0;};
array::~array(){for(int i=0; i < size; i++){delete list[i]; list[i] = NULL;}};
int	array::get_type(){return _ARRAY;};
int	array::get_int(){return list[0]->var->get_int();};
double	array::get_real(){return list[0]->var->get_real();};
char*	array::get_string(){return list[0]->var->get_string();};
void	array::set_int(int _value){list[0]->var->set_int(_value);};
void	array::set_real(double _value){list[0]->var->set_real(_value);};
void	array::set_string(char *_value){list[0]->var->set_string(_value);};
int	array::get_bit(int bit){return list[0]->var->get_bit(bit);};
void	array::set_bit(int bit, int _value){list[0]->var->set_bit(bit, _value);};
int	array::get_bit(){return list[0]->var->get_bit();};
void	array::set_bit(int value){list[0]->var->set_bit(value);};
void	array::set_var(int pos, variable* _var){if(list[pos] != NULL) delete list[pos]; list[pos] = _var;};
void	array::set_size(int _size){
		if(size > 0){for(int i=0; i < size; i++) {if(list[i] != NULL)delete list[i];};}
		size = _size; list = new variable*[size];
		for(int i = 0; i < size; i++) list[i] = NULL;};
int	array::get_size(){return size;};
variable*array::get_variable(int position){return list[position];};


integer::integer(){};
integer::~integer(){};
int	integer::get_type(){return _INTEGER;};
int	integer::get_int(){return value;};
double	integer::get_real(){return (double) value;};
char*	integer::get_string(){sprintf(_return_string, "%d", value); return _return_string;};
void	integer::set_int(int _value){value = _value;};
void	integer::set_real(double _value){value = (int)_value;};
void	integer::set_string(char *_value){value = atoi(_value);};
int	integer::get_bit(int bit){if((value & __bits[bit]) != 0){return 1;}else{return 0;}};
void	integer::set_bit(int bit, int _value){value = (value & ~__bits[bit]) | (_value * __bits[bit]);};
int	integer::get_bit(){return get_bit(0);};
void	integer::set_bit(int value){set_bit(0, value);};
void	integer::set_var(int pos, variable* _var){};
void	integer::set_size(int size){};
variable*integer::get_variable(int position){return NULL;};


bit::bit(){var = NULL;};
bit::~bit(){};
int	bit::get_type(){return _BIT;};
int	bit::get_int(){return get_bit();};
double	bit::get_real(){return (double) get_bit();;};
char*	bit::get_string(){sprintf(_return_string, "%d", get_bit()); return _return_string;};
void	bit::set_int(int _value){set_bit(_value);};
void	bit::set_real(double _value){set_bit((int)_value);};
void	bit::set_string(char *_value){set_bit(atoi(_value));};
int	bit::get_bit(int __bit){_bit = __bit; return get_bit();};
int	bit::get_bit_number(){return _bit;};
void	bit::set_bit(int __bit, int value){_bit = __bit; set_bit(value);};
int	bit::get_bit(){if(var != NULL){return var->var->get_bit(_bit);}else{return _bit;}};
void	bit::set_bit(int value){if(var != NULL){var->var->set_bit(_bit, value);} else {_bit=value;}};
void	bit::set_var(int pos, variable* _var){var = _var;};
void	bit::set_size(int size){};
variable*bit::get_variable(int position){return NULL;};


real::real(){};
real::~real(){};
int	real::get_type(){return _REAL;};
int	real::get_int(){return (int)value;};
double	real::get_real(){return value;};
char*	real::get_string(){sprintf(_return_string, "%f", value); return _return_string;};
void	real::set_int(int _value){value = (double)_value;};
void	real::set_real(double _value){value = _value;};
void	real::set_string(char *_value){value = atof(_value);};
int	real::get_bit(int bit){if(((int)value && __bits[bit]) != 0){return 1;}else{return 0;}};
void	real::set_bit(int bit, int _value){value = (double)(((int)value & ~__bits[bit]) | (_value & __bits[bit]));};
int	real::get_bit(){return get_bit(0);};
void	real::set_bit(int value){set_bit(0, value);};
void	real::set_var(int pos, variable* _var){};
void	real::set_size(int size){};
variable*real::get_variable(int position){return NULL;};


string::string(){value = NULL;};
string::~string(){if(value != NULL) delete value;};
int	string::get_type(){return _STRING;};
int	string::get_int(){return atoi(value);};
double	string::get_real(){return atof(value);};
char*	string::get_string(){return value;};
void	string::set_int(int _value){sprintf(value, "%d", _value);};
void	string::set_real(double _value){sprintf(value, "%f", _value);};
void	string::set_string(char *_value){if(value != NULL) delete value; value = new char[strlen(_value)+1]; strcpy(value, _value);};
int	string::get_bit(int bit){if((atoi(value) && __bits[bit]) != 0){return 1;}else{return 0;}};
void	string::set_bit(int bit, int _value){set_int((atoi(value) & ~__bits[bit]) | (_value & __bits[bit]));};
int	string::get_bit(){return get_bit(0);};
void	string::set_bit(int value){set_bit(0, value);};
void	string::set_var(int pos, variable* _var){};
void	string::set_size(int size){};
variable*string::get_variable(int position){return NULL;};






#ifdef __cplusplus
}
#endif



