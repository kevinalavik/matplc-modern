
//
//    functions.cpp - the functions for the controller
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



#include <string.h>
#include "parser.h"
#include "ladder.h"


#ifdef __cplusplus
extern "C" {
#endif


parser *p;

ladder::ladder(){
	p = new parser();
	cpu = new core();
	io = new io_list();
}


ladder::~ladder(){
	delete p;
	delete cpu;
	delete io;
}



#define		___UNDEFINED	100
#define		___IGNORE		101
#define		___PLC			102
#define		___IO			103
#define		___MEMORY		104
#define		___PROGRAM		105
#define		___MEM_INTEGER	106
#define		___MEM_REAL		107
#define		___MEM_BIT		108
#define		___MEM_STRING	109
#define		___MEM_ARRAY	110
#define		___MEM_UNION	111
#define		___MEM_SET		112
#define		___PROG_LADDER	113
#define		___PROG_LINE	114
#define		___IO_INPUT		115
#define		___IO_OUTPUT	116
#define		___IO_INPUTOUTPUT 117




int ladder::load(char* file_name){
	int	error, i, j, k;
//	int	io_number;
	int	value_int;
//	int	file, word, channel;
	int	array_type;
	int	prog_flag;
	step_t*	program_temp;
	FILE	*fp_in;
	char	work[200],
		temp_comment[200],
		temp_symbol[200],
		temp_name[200],
		temp_data[200],
		temp_value[200],
		temp_value2[200],
		temp_value3[200];
//	char	temp_value4[200];
	int	token_stack[16],
		type[16], file[16], size[16],
		bit_number,
		token_count;
	variable	*var, *var_temp, *var_temp2, *var_parent;

	token_count = -1;
	error = NO_ERROR;
	j = 0;
	if((fp_in = fopen(file_name, "r")) != NULL){
		while ((feof(fp_in) == 0) && (error == NO_ERROR)) {
			j++;
			fgets(work, 200, fp_in);
//printf("\nLine %3d == %d: [%s]", j, token_count, work);
			p->parse_xml(work);
			for(i = 0; (i < p->counter()) && (error == NO_ERROR); i++){
//printf(":%s", p->token(i));
				if(token_count < 0){
					if(strcmp(p->token(i), "<") == 0){
						if(strncmp(p->token(i+1), "?", 1) == 0){
							token_count++;
							token_stack[token_count] = ___IGNORE;
							i++;
						} else if(strncmp(p->token(i+1), "!", 1) == 0){
							token_count++;
							token_stack[token_count] = ___IGNORE;
							i++;
						} else if(strncmp(p->token(i+1), "-->",3) == 0){
							token_count++;
							token_stack[token_count] = ___IGNORE;
							i++;
						} else if(strcmp(p->token(i+1), "plc") == 0){
							token_count++;
							token_stack[token_count] = ___PLC;
							i += 2;
						} else if(strcmp(p->token(i+1), "/") == 0){
							token_count--;
							i += 3;
						} else {
							error_log(MINOR, "ERROR CC020: file element not recognized");
							error = ERROR;
						}
					}
				} else if(token_stack[token_count] == ___IGNORE){
					if(strcmp(p->token(i), ">") == 0){
						if(token_stack[token_count] == ___IGNORE) token_count--;
					}
				} else if(token_stack[token_count] == ___PLC){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "memory") == 0){
							token_count++;
							token_stack[token_count] = ___MEMORY;
							i += 2;
						} else if(strcmp(p->token(i+1), "program") == 0){
							token_count++;
							token_stack[token_count] = ___PROGRAM;
							i += 2;
						} else if(strcmp(p->token(i+1), "io") == 0){
							token_count++;
							token_stack[token_count] = ___IO;
							i += 2;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--;
							i += 3;
						}
					} else {
						error_log(MINOR, "ERROR CC021: unrecognized plc content type");
					}
				} else if(token_stack[token_count] == ___MEMORY){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "integer") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_INTEGER;
							type[token_count] = _INTEGER;
						} else if(strcmp(p->token(i+1), "real") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_REAL;
							type[token_count] = _REAL;
						} else if(strcmp(p->token(i+1), "bit") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_BIT;
							type[token_count] = _BIT;
						} else if(strcmp(p->token(i+1), "string") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_STRING;
							type[token_count] = _STRING;
						} else if(strcmp(p->token(i+1), "array") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_ARRAY;
							type[token_count] = _ARRAY;
						} else if(strcmp(p->token(i+1), "union") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_UNION;
							type[token_count] = _UNION;
						} else if(strcmp(p->token(i+1), "set") == 0){
							token_count++; i++;
							token_stack[token_count] = ___MEM_SET;
							// type[token_count] = _STRING;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--; i += 3;
						} else {
							error_log(MINOR, "ERROR CC022: unrecognized memory content type");
							error = ERROR;
						}
						var = NULL;
						var_parent = NULL;
						var_temp = NULL;
						temp_name[0] = 0;
						temp_symbol[0] = 0;
						temp_comment[0] = 0;
						temp_value[0] = 0;
						temp_data[0] = 0;
					} else {
						error_log(MINOR, "ERROR CC023: Content of memory definition not recognized");
						error = ERROR;
					}
					if(error == NO_ERROR){
						file[token_count] = 0; size[token_count] = 0;
						temp_comment[0] = 0; temp_symbol[0] = 0;
					}
				} else if((token_stack[token_count] == ___MEM_INTEGER) || 
					(token_stack[token_count] == ___MEM_REAL) || 
					(token_stack[token_count] == ___MEM_STRING) || 
					(token_stack[token_count] == ___MEM_BIT) || 
					(token_stack[token_count] == ___MEM_ARRAY) || 
					(token_stack[token_count] == ___MEM_UNION)){
					if(strncmp(p->token(i), "name", 4) == 0){
						strcpy(temp_name, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "symbol", 6) == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "parent", 6) == 0){
						var_parent = mem->find(p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "number", 6) == 0){
						bit_number = atoi(p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "value", 5) == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "data", 4) == 0){
						strcpy(temp_data, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "size", 4) == 0){
						size[token_count] = atoi(p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), "comment", 7) == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if((strncmp(p->token(i), ">", 1) == 0) || ((strncmp(p->token(i), "/", 1) == 0) && (strncmp(p->token(i+1), ">", 1) == 0))){
						if(var_parent == NULL){
							var_temp = mem->find(temp_name);
						} else {
							var = var_parent;
						}
						// var = var_parent;
						if(token_stack[token_count] == ___MEM_INTEGER){
							if(var_temp == NULL) var_temp = mem->add(var, temp_symbol, temp_name, temp_comment, _INTEGER);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						} else if(token_stack[token_count] == ___MEM_BIT){
							if(var_temp == NULL) var_temp = mem->add(var, temp_symbol, temp_name, temp_comment, _BIT);
							value_int = 0; if(temp_value[0] != 0) value_int = atoi(temp_value);
							var_temp->var->set_bit(bit_number, value_int);
						} else if(token_stack[token_count] == ___MEM_REAL){
							if(var_temp == NULL) var_temp = mem->add(var, temp_symbol, temp_name, temp_comment, _REAL);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						} else if(token_stack[token_count] == ___MEM_STRING){
							if(var_temp == NULL) var_temp = mem->add(var, temp_symbol, temp_name, temp_comment, _STRING);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						} else if(token_stack[token_count] == ___MEM_ARRAY){
							if(var_temp == NULL) var_temp = mem->add(var, temp_symbol, temp_name, temp_comment, _ARRAY);
							var_temp->var->set_size(size[token_count]);
							temp_value2[0] = 0; temp_value3[0] = 0;
							array_type = _INTEGER;
							if(strcmp(temp_data, "real") == 0) array_type = _REAL;
							if(strcmp(temp_data, "bit") == 0) array_type = _BIT;
							if(strcmp(temp_data, "string") == 0) array_type = _STRING;
							if(strcmp(temp_data, "array") == 0) array_type = _ARRAY;
							if(strcmp(temp_data, "union") == 0) array_type = _UNION;
							for(k = 0; k < size[token_count]; k++){
								if(temp_name[0] != 0) sprintf(temp_value2, "%s%d", temp_name, k);
								if(temp_symbol[0] != 0) sprintf(temp_value3, "%s%d", temp_symbol, k);
								// printf("%s  %s  \n", temp_value2, temp_value3);
								var_temp2 = mem->add(var_temp, temp_value3, temp_value2, NULL, array_type);
								var_temp->var->set_var(k, var_temp2);
							}
						} else if(token_stack[token_count] == ___MEM_UNION){
							if(var_temp == NULL) var_temp = mem->add(var, temp_name, temp_symbol, temp_comment, _UNION);
							if(temp_value[0] != 0) var_temp->var->set_string(temp_value);
						}
						if(strncmp(p->token(i), "/", 1) == 0){ i+= 2; token_count--;}

					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						token_count--; i += 3;
					} else {
						error_log(MINOR, "ERROR CC024: memory definition type not correct");
						// printf(" %s=\"%s\"\n", p->token(i), p->token(i+1));
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___MEM_SET){ 
					if(strcmp(p->token(i), "name") == 0){
						var = mem->find(p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "symbol") == 0){
						strcpy(temp_symbol, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "value") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "number") == 0){
						bit_number = atoi(p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						strcpy(temp_comment, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "//", 1) == 0)){
						token_count--; i += 3;
					} else if((strcmp(p->token(i), ">") == 0) || ((strcmp(p->token(i), "/") == 0) && (strcmp(p->token(i+1), ">") == 0))){
						if(var != NULL){
							if(var->var->type == _BIT){
								var->var->set_bit(bit_number, atoi(temp_value));
							} else if(temp_value[0] != 0){
								var->var->set_string(temp_value);
							}
							if(temp_symbol[0] != 0) var->var->set_symbol(temp_symbol);
							if(temp_comment[0] != 0) var->var->set_comment(temp_comment);
						} else {
							error_log(MINOR, "ERROR CC025: Variable 'name' not found");
							error = ERROR;
						}
						if(strcmp(p->token(i), "/") == 0){ i += 2; token_count--;}
					} else {
						error_log(MINOR, "ERROR CC026: set argument not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___PROGRAM){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "ladder") == 0){
							token_count++; i++;
							token_stack[token_count] = ___PROG_LADDER;
							prog_flag = 0;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--; i += 3;
						} else {
							error_log(MINOR, "ERROR CC027: unrecognized program content type");
							error = ERROR;
						}
					} else {
						error_log(MINOR, "ERROR CC028: Content of program definition not recognized");
						error = ERROR;
					}
				} else if(token_stack[token_count] == ___PROG_LADDER){
					if((prog_flag == 1) && (strcmp(p->token(i), "<") == 0) && (strncmp(p->token(i+1), "/", 1) == 0)){
						if(program_temp != NULL){
						//	error = prog->program_delete(file[token_count]);
						//	error = prog->program_move(file[token_count], program_temp);
						}
						token_count--; i += 3;
					} else if(prog_flag == 1){
						error = parse_program_line(p->token(i));
					} else if(strcmp(p->token(i), "location") == 0){
						//file[token_count] = atoi(p->token(i+1));
						program_temp = cpu->add_to_prog(_CHILD, NULL, _STEP_LABEL, p->token(i+1), "a PLC program", NULL, NULL);
						init_program_line(program_temp);
						//program_temp = prog->get_empty(file[token_count]);
						if(program_temp == NULL) error = ERROR;
						i++;
					} else if(strcmp(p->token(i), "comment") == 0){
						program_temp->set_comment(p->token(i+1));
						//strcpy(temp_comment, p->token(i+1));
						i++;
					} else if(strncmp(p->token(i), ">", 1) == 0){
						prog_flag = 1;
						//error = cpu->make_program(program_temp);
						//if(error == ERROR) prog->program_delete(program_temp);
					} else {
						error_log(MINOR, "ERROR CC029: program definition type not correct");
						error = ERROR;
					//	prog->program_delete(program_temp);
					}
				} else if(token_stack[token_count] == ___IO){
					if(strcmp(p->token(i), "<") == 0){
						if(strcmp(p->token(i+1), "input") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_INPUT;
						}else if(strcmp(p->token(i+1), "output") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_OUTPUT;
						}else if(strcmp(p->token(i+1), "inputoutput") == 0){
							token_count++; i++;
							token_stack[token_count] = ___IO_INPUTOUTPUT;
						} else if(strncmp(p->token(i+1), "/", 1) == 0){
							token_count--; i += 3;
						} else {
							error_log(MINOR, "ERROR CC032: unrecognized io content type");
							error = ERROR;
						}
					} else if((strcmp(p->token(i), "/") == 0) && (strcmp(p->token(i+1), ">") == 0)){
						i += 2;
						token_count--;
					} else {
						error_log(MINOR, "ERROR CC033: Content of io definition not recognized");
						error = ERROR;
					}
				} else if((token_stack[token_count] == ___IO_INPUT) || (token_stack[token_count] == ___IO_OUTPUT) || (token_stack[token_count] == ___IO_INPUTOUTPUT)){ 
					if(strcmp(p->token(i), "local") == 0){
						strcpy(temp_value, p->token(i+1));
						i++;
					} else if(strcmp(p->token(i), "remote") == 0){
						strcpy(temp_value2, p->token(i+1));
						i++;
					} else if((strcmp(p->token(i), "/") == 0) && (strncmp(p->token(i+1), ">", 1) == 0)){
						variable	*var;
//printf("Setting IO link %d   %s==%s\n", token_stack[token_count], temp_value, temp_value2);
						if((var = mem->find(temp_value)) != NULL){
//printf("    GOT (%s) type=%d \n", var->var->get_symbol(), var->var->get_type());
							if(token_stack[token_count] == ___IO_INPUT){
								io->add(IO_INPUT, var, temp_value2);
							} else if(token_stack[token_count] == ___IO_OUTPUT){
								io->add(IO_OUTPUT, var, temp_value2);
							} else if(token_stack[token_count] == ___IO_INPUTOUTPUT){
								io->add(IO_INPUTOUTPUT, var, temp_value2);
							} else {
								error_log(MINOR, "ERROR CC034: Can't set that io connection");
								error = ERROR;
							}
						} else {
							printf("ERROR: can't find local variable \n");
							error = ERROR;
						}
						i+= 1; token_count--;
					} else {
						error_log(MINOR, "ERROR CC035: input/output argument not recognized");
						error = ERROR;
					}
				} else {
					error_log(MINOR, "ERROR CC045: Improperly formed plc file");
					error = ERROR;
				}
			}
		}
		fclose(fp_in);
	} else {
		error_log(MINOR, "ERROR CC046: Could Not open IO file");
		error = ERROR;
	}
	if(token_count >= 0 ){
		error_log(MINOR, "ERROR CC047: file was not properly formed");
		sprintf(temp_comment, "Line was [%s]\n", work);
		error_log(MINOR, temp_comment);
		error = ERROR;
	}

	return error;
}


int	stack_pnt;
	step_t		*stack[30];
	int		stack_cnt[30];

int	ladder::init_program_line(step_t *top){
	int	error;

	error = NO_ERROR;
	if(top->type == _STEP_LABEL){
		stack_pnt = 0;
		stack[stack_pnt] = top;
		stack_cnt[stack_pnt] = 0;
	} else {
		error = ERROR;
		printf("program line init only allowed for a label\n");
	}

	return error;
}


int	ladder::parse_program_line(char *token){
	int		error;
	function	*inst;
	variable	*var;

	error = NO_ERROR;
//for(int kk = 0; kk < stack_pnt; kk++) printf("   ");
//printf("%d: ", stack_pnt);
	if((inst = library->search(token)) != NULL){
//printf("Command token [%s] ", token);
		// found an instruction, interpret it
		stack[stack_pnt + 1] = cpu->add_to_prog(_CHILD, stack[stack_pnt], _STEP_INSTRUCTION, NULL, NULL, inst, NULL);
		if(stack_cnt[stack_pnt] > 0) stack_cnt[stack_pnt]--;
		if(inst->arg_cnt > 0){
//printf(" %d args ", inst->arg_cnt);
			stack_pnt++;
			stack_cnt[stack_pnt] = inst->arg_cnt;
		}
	} else if((var = mem->find(token)) != NULL){
//printf("Variable token [%s] ", token);
		// found a variable, link it
		cpu->add_to_prog(_CHILD, stack[stack_pnt], _STEP_VARIABLE, NULL, NULL, NULL, var);
		if(stack_cnt[stack_pnt] > 0) stack_cnt[stack_pnt]--;
	} else if((token[0] >= '0') && (token[0] <= '9') || (token[0] == '-') || (token[0] == '+')){
//printf("Number token [%s] ", token);
		// assume a real number
		var = mem->add(NULL, NULL, NULL, NULL, _REAL);	var->var->set_string(token);
		cpu->add_to_prog(_CHILD, stack[stack_pnt], _STEP_VARIABLE, NULL, NULL, NULL, var);
		if(stack_cnt[stack_pnt] > 0) stack_cnt[stack_pnt]--;
	} else {
//printf("String token [%s] ", token);
		// assume a literal string
		var = mem->add(NULL, NULL, NULL, NULL, _STRING);	var->var->set_string(token);
		cpu->add_to_prog(_CHILD, stack[stack_pnt], _STEP_VARIABLE, NULL, NULL, NULL, var);
		if(stack_cnt[stack_pnt] > 0) stack_cnt[stack_pnt]--;
	}
	for(; (stack_pnt > 0) && (stack_cnt[stack_pnt] == 0);){
		stack_pnt--;
	}
//printf("  %d \n", stack_pnt);


	return error;
}



int ladder::dump(){
	cpu->dump();
	io->dump();

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif


