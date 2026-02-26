
//
//    store.cpp - a memory integration class
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


#include "store.h"

#include "../../lib/plc.h"



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

	switch (type) {
	case _INTEGER:
	  new_variable->var = new integer(_name);
          break;
	case _ARRAY:
	  new_variable->var = new array(_name);
          break;
	case _REAL:
	  new_variable->var = new real(_name);
          break;
	case _STRING:
	  new_variable->var = new string(_name);
          break;
	case _UNION:
	  new_variable->var = new integer(_name);
          break;
	case _BIT:
	  if (parent == NULL) 
	    new_variable->var = new bit(_name); 
	  else
            new_variable->var = new bit_alias(parent);
          break;
	default:
	  error = ERROR; 
          error_log("ERROR: Incorrect variable type");
	} // switch

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
char*	data::get_symbol(){return symbol;};
void	data::set_symbol(char*value){if(symbol != NULL) delete symbol; symbol = new char[strlen(value)+1]; strcpy(symbol, value);};
char*	data::get_name(){return name;};
void	data::set_name(char*value){if(name != NULL) delete name; name = new char[strlen(value)+1]; strcpy(name, value);};
char*	data::get_comment(){return comment;};
void	data::set_comment(char*value){if(comment != NULL) delete comment; comment = new char[strlen(value)+1]; strcpy(comment, value);};


static inline plc_pt_t protected_get_pt_by_name(char * name) {
  plc_pt_t pt_handle = plc_pt_by_name(name);

  if (pt_handle.valid == 0) {
    pt_handle = plc_pt_null();
    plc_log_wrnmsg(1, "Could not get handle to %s, using null point.", name);
  }

  return pt_handle;
}



char _return_string[30];
unsigned int __bits[16] = {
		0x0001, 0x0002, 0x0004, 0x0008, 
		0x0010, 0x0020, 0x0040, 0x0080,
		0x0100, 0x0200, 0x0400, 0x0800,
		0x1000, 0x2000, 0x4000, 0x8000};


array::array(){size = 0;};
array::array(char *_name){size = 0;};
array::~array(){for(int i=0; i < size; i++){delete list[i]; list[i] = NULL;}};
int	array::get_int(){return list[0]->var->get_int();};
f32	array::get_real(){return list[0]->var->get_real();};
char*	array::get_string(){return list[0]->var->get_string();};
void	array::set_int(int _value){list[0]->var->set_int(_value);};
void	array::set_real(f32 _value){list[0]->var->set_real(_value);};
void	array::set_string(char *_value){list[0]->var->set_string(_value);};
int	array::get_bit(int bit){return list[0]->var->get_bit(bit);};
void	array::set_bit(int bit, int _value){list[0]->var->set_bit(bit, _value);};
int	array::get_bit(){return list[0]->var->get_bit();};
void	array::set_bit(int value){list[0]->var->set_bit(value);};
void	array::set_var(int pos, variable* _var){if(list[pos] != NULL) delete list[pos]; list[pos] = _var;};
void	array::set_size(int _size){
		if(size > 0){for(int i=0; i < size; i++) {if(list[i] != NULL)delete list[i];};}
		size = _size; list = new (variable *)[size];
		for(int i = 0; i < size; i++) list[i] = NULL;};
int	array::get_size(){return size;};
variable*array::get_variable(int position){return list[position];};



/////////////
// Integer //
/////////////

integer::integer() { 
  _plc_pt_handle = plc_pt_null();
}

integer::integer(char *name)  {
  _plc_pt_handle = protected_get_pt_by_name(name);
}

integer::~integer(){};

int	integer::get_int()  {
  return plc_get(_plc_pt_handle);
}

f32	integer::get_real() {
  return (f32) get_int();
}

char*	integer::get_string() {
  sprintf(_return_string, "%d", get_int()); 
  return _return_string;
}

void	integer::set_int(int value) {
  plc_set(_plc_pt_handle, value);
}

void	integer::set_real(f32 value) {
  set_int((int)value);
}

void	integer::set_string(char *value) {
  set_int(atoi(value));
}

int	integer::get_bit(int bit) {
  if((get_int() & __bits[bit]) != 0) {
    return 1;
  } else {
    return 0;
  }
}

void	integer::set_bit(int bit, int value) {
  set_int((get_int() & ~__bits[bit]) | (value * __bits[bit]));
}

int	integer::get_bit() {
  return get_bit(0);
}

void	integer::set_bit(int value) {
  set_bit(0, value);
}

void	integer::set_var(int pos, variable* var) 
{}

void	integer::set_size(int size)
{}

variable*integer::get_variable(int position) {
  return NULL;
}

/////////
// Bit //
/////////

bit::bit() {
  _plc_pt_handle = plc_pt_null();
}

bit::bit(char *name){
  _plc_pt_handle = protected_get_pt_by_name(name);
}

bit::~bit()
{}

int	bit::get_int() {
  return get_bit();
}

f32 bit::get_real() {
  return (f32) get_bit();
}

char*	bit::get_string() {
  sprintf(_return_string, "%d", get_bit()); 
  return _return_string;
}

void	bit::set_int(int value) {
  set_bit(value);
}

void	bit::set_real(f32 value) {
  set_bit((int)value);
}

void	bit::set_string(char *_value) {
  set_bit(atoi(_value));
}

int	bit::get_bit(int bit) {
  return get_bit();
}

int	bit::get_bit_number() {
  return 0;
}

void	bit::set_bit(int bit, int value) {
  set_bit(value);
}

int	bit::get_bit() {
  plc_get(_plc_pt_handle);
}

void	bit::set_bit(int value) {
  plc_set(_plc_pt_handle, value);
}

void	bit::set_var(int pos, variable* var) {
printf("bit::set_var() - called...!!!\n");
}

void	bit::set_size(int size)
{}

variable*bit::get_variable(int position) {
  return NULL;
}

///////////////
// Bit_alias //
///////////////

bit_alias::bit_alias() {
  _parent = NULL;
  _bit = 0;
}

bit_alias::bit_alias(variable *parent){
  _parent = parent;
  _bit = 0;
}

bit_alias::~bit_alias()
{}

int	bit_alias::get_int() {
  return get_bit();
}

f32 bit_alias::get_real() {
  return (f32) get_bit();
}

char*	bit_alias::get_string() {
  sprintf(_return_string, "%d", get_bit()); 
  return _return_string;
}

void	bit_alias::set_int(int value) {
  set_bit(value);
}

void	bit_alias::set_real(f32 value) {
  set_bit((int)value);
}

void	bit_alias::set_string(char *_value) {
  set_bit(atoi(_value));
}

int	bit_alias::get_bit(int bit) {
  _bit = bit;
  return get_bit();
}

int	bit_alias::get_bit_number() {
  return _bit;
}

void	bit_alias::set_bit(int bit, int value) {
  _bit = bit;
  set_bit(value);
}

int	bit_alias::get_bit() {
  return _parent->var->get_bit(_bit);
}

void	bit_alias::set_bit(int value) {
  _parent->var->set_bit(_bit, value);
}

void	bit_alias::set_var(int pos, variable* var) {
  _parent = var;
}

void	bit_alias::set_size(int size)
{}

variable*bit_alias::get_variable(int position) {
  return NULL;
}


//////////
// Real //
//////////

real::real()
{
  _plc_pt_handle = plc_pt_null();
}

real::real(char *name) {
  _plc_pt_handle = protected_get_pt_by_name(name);
}

real::~real() 
{}

int	real::get_int() {
  return (int)get_real();
}

f32	real::get_real() {
  i32 tmp_i32 = plc_get(_plc_pt_handle);
printf("real::get_real(): %s returning %f\n", name, *((f32 *)(&tmp_i32)));
  return *((f32 *)(&tmp_i32));
}

char*	real::get_string() {
  sprintf(_return_string, "%f", get_real()); 
  return _return_string;
}

void	real::set_int(int value) {
  set_real((f32)value);
}

void	real::set_real(f32 value) {
printf("real::set_real(): %s setting to %f\n", name, value);
  plc_set(_plc_pt_handle, *((i32 *)&value));
}

void	real::set_string(char *value) {
  set_real(atof(value));
}

int	real::get_bit(int bit) {
  if((get_int() && __bits[bit]) != 0) {
    return 1;
  } else {
    return 0;
  }
}

void	real::set_bit(int bit, int value) {
  set_real((f32)((get_int() & ~__bits[bit]) | (value & __bits[bit])));
}

int	real::get_bit() {
  return get_bit(0);
}

void	real::set_bit(int value) {
  set_bit(0, value);
}

void	real::set_var(int pos, variable* var)
{}

void	real::set_size(int size)
{}

variable*real::get_variable(int position) {
  return NULL;
}

////////////
// String //
////////////


string::string() {
  _value = NULL;
}

string::string(char *name) {
  _value = NULL;
}

string::~string() {
  if(_value != NULL) 
    delete _value;
}
int	string::get_int() {
  return atoi(_value);
}

f32	string::get_real() {
  return atof(_value);
}

char*	string::get_string() {
  return _value;
}

void	string::set_int(int value) {
  sprintf(_value, "%d", value);
}

void	string::set_real(f32 value) {
  sprintf(_value, "%f", value);
}

void	string::set_string(char *value) {
  if(_value != NULL) 
    delete _value; 

 _value = new char[strlen(value)+1]; 
 strcpy(_value, value);
}

int	string::get_bit(int bit) {
  if((atoi(_value) && __bits[bit]) != 0) {
    return 1;
  } else {
    return 0;
  }
}

void	string::set_bit(int bit, int value) {
  set_int((atoi(_value) & ~__bits[bit]) | (value & __bits[bit]));
}

int	string::get_bit() {
  return get_bit(0);
}

void	string::set_bit(int value) {
  set_bit(0, value);
}

void	string::set_var(int pos, variable* var)
{}

void	string::set_size(int size)
{}

variable*string::get_variable(int position) {
  return NULL;
}



