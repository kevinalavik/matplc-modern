
//
//    program_il.cpp - instruction list orient program storage
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



#include "program_il.h"


program_il::program_il(){
	int	i;

	first = NULL;
	focus_in = NULL;
	focus_out = NULL;
	operand_in = NULL;
	operand_out = NULL;
	comments = new comments_list;
	labels = new label_list[LABEL_LIST_SIZE];
	for(i = 0; i < LABEL_LIST_SIZE; i++) labels[i].used = UNUSED;
}



program_il::~program_il(){
	opcode_il	*next,
			*next_temp;

	delete comments;
	delete labels;
	if(first != NULL){
		next = first;
		while(next != NULL){
			next_temp = next->next;
			delete_operands(next);
			delete next->comments;
			delete next;
			next = next_temp;		
		}
		first = NULL;
		focus_in = NULL;	
		operand_in = NULL;
		focus_out = NULL;	
		operand_out = NULL;
	}
}


int program_il::append_instruction(){
	int		error;
	opcode_il	*next;

	error = NO_ERROR;
	if(first != NULL){
		next = first;
		while(next->next != NULL){
			next = next->next;
		}
		next->next = focus_in;
	} else {
		first = focus_in;
	}

	focus_in->next = NULL;
	focus_in = NULL;
	operand_in = NULL;
	
	return error;
}


int program_il::insert_instruction(int line){
	int		error,
			i;
	opcode_il	*next,
			*last_next;

	error = ERROR;
	if(line == 0){
		focus_in->next = first;
		first = focus_in;
		error = NO_ERROR;
		focus_in = NULL;
		operand_in = NULL;
	} else if(first != NULL){
		next = first;
		for(i = 0; (next->next != NULL) && (i < line); i++){
			last_next = next;
			next = next->next;
		}
		if(i == line){
			focus_in->next = last_next->next;
			last_next->next = focus_in;
			focus_in = NULL;
			operand_in = NULL;
			error = NO_ERROR;
		} else {
			error_log("Error PIL1: insert line after end of program");
		}
	} else {
		error_log("Error PIL2: Can't insert into an empty program");
	}
	
	return error;
}



int program_il::delete_operands(opcode_il *_opcode){
	int	error;
	operand_il	*next,
			*next_temp;

	error = NO_ERROR;
	next = _opcode->first;
	_opcode->first = NULL;
	while(next != NULL){
		next_temp = next->next;
		// delete next->address;
		delete next->comments;
		delete next;
		next = next_temp;
	}

	return error;
}



int program_il::delete_instruction(int line){
	int		error,
			i;
	opcode_il	*next,
			*old_line;

	error = ERROR;
	if(first != NULL){
		if(line == 0){
			next = first->next;
			delete_operands(first);
			delete first->comments;
			delete first;
			first = next;
			error = NO_ERROR;
		} else {
			next = first;
			for(i = 0; (next->next != NULL) && (i < line); i++){
				old_line = next;
				next = next->next;
			}
			if(i == line){
				old_line->next = next->next;
				delete_operands(next);
				delete next->comments;
				delete next;
				error = NO_ERROR;
			} else {
				error_log("Error PIL3: delete line after end of program");
			}
		}
	} else {
		error_log("Error PIL4: Can't delete from an empty program");
	}
	
	return error;
}



int program_il::get_instruction(int line){
	int		error,
			i;
	opcode_il	*next;

	error = ERROR;
	if(first != NULL){
		next = first;
		for(i = 0; (next->next != NULL) && (i < line); i++){
			next = next->next;
		}
		if(i == line){
			focus_out = next;
			operand_out = focus_out->first;
			error = NO_ERROR;
		} else {
			focus_out = NULL;
			operand_out = NULL;
		}
	} else {
		// error_log("Error PILx: Can't get a line from an empty program");
	}
	
	return error;
}




int program_il::get_next_instruction(){
	int		error;

	error = ERROR;
	if(focus_out != NULL){
		focus_out = focus_out->next;
		if(focus_out != NULL){
			operand_out = focus_out->first;
			error = NO_ERROR;
		}
	} else {
		error_log("WARNING PIL5: Program does not have an END statement");
	}
	
	return error;
}


int program_il::create_opcode(int _opcode){
	int	error;

	error = NO_ERROR;
	if(focus_in == NULL){
		focus_in = new opcode_il();
		focus_in->comments = new comments_list;
		focus_in->opcode = _opcode;
		focus_in->first = NULL;
		focus_in->next = NULL;
		operand_in = NULL;
	} else {
		error_log("ERROR PIL6: focus already used");
		error = ERROR;
	}

	return error;
}


int program_il::append_operand(variable *var){
	int		error;

	error = NO_ERROR;
	if(focus_in != NULL){
		if(focus_in->first == NULL){
			focus_in->first = new operand_il();
			operand_in = focus_in->first;
		} else if(operand_in != NULL) {
			operand_in->next = new operand_il();
			operand_in = operand_in->next;
		} else {
			error_log("ERROR PIL7: operand list inconsistent");
			error = ERROR;
			return error;
		}
		operand_in->comments = new comments_list;
		operand_in->next = NULL;
		operand_in->address = var;
		// operand_in->address = new memory_address();
		// operand_in->address->set(type, file, word, bit);
	} else {
		error_log("ERROR PIL8: No opcode focus");
		error = ERROR;
	}

	return error;
}


int program_il::get_opcode(int *opcode){
	int	error;

	error = NO_ERROR;
	if(focus_out != NULL){
		opcode[0] = focus_out->opcode;
	} else {
		error_log("ERROR PIL13: Focus not assigned");
		error = ERROR;
	}

	return error;
}


int program_il::get_operand_type(){
	if(operand_out != NULL){
		return operand_out->address->var->type;
	} else {
		// error_log("ERROR PILx: no active operand");
		return _VALUE_UNDEFINED;
	}
}


variable *program_il::get_operand(){
	variable	*var;

	if(operand_out != NULL){
		var = operand_out->address;
		operand_out = operand_out->next;
	} else {
		var = NULL;
	}

	return var;
}



void program_il::dump_all(){
	int		_type,
			i, j, k;
	char		work[100];
	variable	*var;

	printf("Program Dump \n");
	for(k = 0; get_comment(__PROGRAM_COMMENT, k, work) != ERROR; k++){
		printf("program comment (%s)\n", work);
	}
	for(i = 0; get_instruction(i) != ERROR; i++){
		for(k = 0; get_comment(__OPCODE_COMMENT, k, work) != ERROR; k++){
			printf("com(%s): ", work);
		}
		get_opcode(&_type);
		printf("   Line %d : Opcode %d: Operands ( ", i, _type);
		j = 0;
		while((_type = get_operand_type()) != _VALUE_UNDEFINED){
			for(k = 0; get_comment(__OPERAND_COMMENT, k, work) != ERROR; k++){
				printf("C(%s)", work);
			}
			if(j > 0) printf(", ");
			j++;
			var = get_operand();
// printf("=== %d\n", var->var->type);
			if(var->var->type == _LITERAL){
				printf("lit: %s", var->var->get_string());
			} else {
				printf("var: %s", var->var->get_symbol());
			}
		}
		printf(" )\n");
	}
	for(i = 0; i < LABEL_LIST_SIZE; i++){
		if(labels[i].used == USED)
			printf("Label %d: Instruction %d\n", i, labels[i].instruction);
	}
}



int program_il::get_comment(int type, int line, char *text){
	int		error;

	error = NO_ERROR;
	if(type == __OPERAND_COMMENT){
		if(operand_out != NULL){
			error = operand_out->comments->get_comment(line, text);
		} else {
			error = ERROR;		// end of list
		}
	} else if(type == __OPCODE_COMMENT){
		if(focus_out != NULL){
			error = focus_out->comments->get_comment(line, text);
		} else {
			error_log("ERROR PIL16: Focus not assigned");
			error = ERROR;
		}
	} else if(type == __PROGRAM_COMMENT){
		error = comments->get_comment(line, text);
	} else {
		error_log("ERROR PIL17: comment type not recognized");
		error = ERROR;
	}

	return error;
}



int program_il::set_comment(int type, int operation, int line, char *text){
	int		error;

	error = NO_ERROR;
	if(type == __OPERAND_COMMENT){
		if(operand_in != NULL){
			error = operand_in->comments->set_comment(operation, line, text);
		} else {
			error = ERROR;		// end of list
		}
	} else if(type == __OPCODE_COMMENT){
		if(focus_in != NULL){
			error = focus_in->comments->set_comment(operation, line, text);
		} else {
			error_log("ERROR PIL18: Focus not assigned");
			error = ERROR;
		}
	} else if(type == __PROGRAM_COMMENT){
		error = comments->set_comment(operation, line, text);
	} else {
		error_log("ERROR PIL19: comment type not recognized");
		error = ERROR;
	}

	return error;
}



int program_il::label(int command, int label_num, int inst_num, int *inst_out){
	int		error,
			i;

	error = NO_ERROR;
	if(command == LABEL_CLEAR_ALL){
		for(i = 0; i < LABEL_LIST_SIZE; i++){
			labels[i].used = UNUSED;
		}
	} else if(command == LABEL_SET_VALUE){
		if((label_num >= 0) && (label_num < LABEL_LIST_SIZE)){
			if(labels[label_num].used == UNUSED){
				labels[label_num].used = USED;
				labels[label_num].instruction = inst_num;
			} else {
				error_log("ERROR PIL20: label number is already used");
				error = ERROR;
			}
		} else {
			error_log("ERROR PIL21: label number outside limits");
			error = ERROR;
		}
	} else if(command == LABEL_GET_VALUE){
		if((label_num >= 0) && (label_num < LABEL_LIST_SIZE)){
			if(labels[label_num].used == USED){
				inst_out[0] = labels[label_num].instruction;
			} else {
				error_log("ERROR PIL22: label number is not used");
				error = ERROR;
			}
		} else {
			error_log("ERROR PIL23: label number outside limits");
			error = ERROR;
		}
	} else {
		error_log("ERROR PIL24: Label command not recognized");
		error = ERROR;
	}

	return error;
}

