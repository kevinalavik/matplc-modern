
//
//    program.cpp - program storage memory
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
// Last Modified: October 12, 2000
//


#include "program.h"




program::program(int size){
	int	i;

	if(size > 0){
		if(size > MAXIMUM_PROGRAM_LOCATIONS_EVER){
			size = MAXIMUM_PROGRAM_LOCATIONS_EVER;
			error_log("Warning P1: Program memory size was too large, so it was reduced to the maximum");
		}
		list = new program_list[size];
		if(list == NULL){
			error_log("ERROR P2: could not allocate program memory");
			maximum_size = 0;
		} else {
			maximum_size = size;
			for(i = 0; i < maximum_size; i++){
				list[i].used = UNUSED;
			}
		}
	} else {
		error_log("ERROR P3: Can't have a program size of zero or less");
	}
}




program::~program(){
	int	error,
		i;

	for(i = 0; i < maximum_size; i++){
		if(list[i].used == USED){
			error = program_delete(i);
		}
	}
	if(maximum_size > 0) delete list;
}


int	program::program_add(int file, int type){
	int	error;

	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		if(list[file].used == UNUSED){
			if(type == _IL_PROGRAM){
				list[file]._il = new program_il();
				list[file].used = USED;
				list[file].type = type;
			} else {
				error_log("Error P4: Unrecognized program type");
				exit(1);
			}
			error = NO_ERROR;
		} else {
			error_log("ERROR P5: program file unused");
		}
	} else {
		error_log("Error P6: requested program file outside valid range");
	}

	return error;
}



int	program::program_delete(int file){
	int	error;

	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		if(list[file].used == USED){
			if(list[file].type == _IL_PROGRAM){
				delete list[file]._il;
				list[file].used = UNUSED;
				error = NO_ERROR;
			} else {
				error_log("ERROR P7: Program type not recognized");
			}
		} else {
			error_log("Warning P8: program file unused");
		}
	} else {
		error_log("Error P9: requested program file outside valid range");
	}

	return error;
}

int	program::program_type(int file){
	if((file >= 0) && (file < maximum_size)){
		if(list[file].used == USED){
			return list[file].type;
		} else {
			return ERROR;
		}
	} else {
		error_log("Error P10: requested program file outside valid range");
		return ERROR;
	}
}



int	program::get_empty(int file){
	int	i;

	if((file >= 0) && (file < maximum_size)){
		if(list[file].used != USED) return file;
		for(i = 0; i < maximum_size; i++){
			if(list[i].used != USED){
				return i;
			}
		}
	} else {
		error_log("Error P100: requested program file outside valid range");
		return -1;
	}
	error_log("ERROR: No program memory free");
	return -1;
}


int	program::program_move(int dest_file, int source_file){
	if((source_file >= 0) && (source_file < maximum_size)){
		if((dest_file >= 0) && (dest_file < maximum_size)){
			if((list[source_file].used == USED) && (list[dest_file].used != USED)){
				list[dest_file].type = list[source_file].type;
				list[dest_file].used = list[source_file].used;
				list[dest_file]._il = list[source_file]._il;
				list[source_file].type = UNUSED;
				list[source_file]._il = NULL;
				return NO_ERROR;
			} else {
				error_log("ERROR P103: Program move source is not used, or dest is used");
				return ERROR;
			}
		} else {
			error_log("Error P102: dest program file outside valid range");
			return ERROR;
		}
	} else {
		error_log("Error P101: source program file outside valid range");
		return ERROR;
	}
}

void	program::dump_all(){
	int	i;

	for(i = 0; i < maximum_size; i++){
		if(list[i].used == USED){
			if(list[i].type == _IL_PROGRAM){
				printf("\n\nProgram %d : ", i);
				list[i]._il->dump_all();
			} else {
				error_log("ERROR P11: Program type not recognized");
			}
		}
	}
}


void	program::dump_all_programs(){
	int	i;

	for(i = 0; i < maximum_size; i++){
		if(list[i].used == USED){
			if(list[i].type == _IL_PROGRAM){
				printf("Program %d : Instruction List \n", i);
			} else {
				error_log("ERROR P12: Program type not recognized");
			}
		}
	}
}



int program::append_instruction(int file){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->append_instruction();
	} else {
		error_log("ERROR P13: location outside program files");
	}
	return error;
}


int program::insert_instruction(int file, int line){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->insert_instruction(line);
	} else {
		error_log("ERROR P14: location outside program files");
	}
	return error;
}


int program::delete_instruction(int file, int line){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->delete_instruction(line);
	} else {
		error_log("ERROR P15: location outside program files");
	}
	return error;
}


int program::get_instruction(int file, int line){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->get_instruction(line);
	} else {
		error_log("ERROR P16: location outside program files");
	}
	return error;
}


int program::get_next_instruction(int file){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->get_next_instruction();
	} else {
		error_log("ERROR P17: location outside program files");
	}
	return error;
}


int program::create_opcode(int file, int opcode){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->create_opcode(opcode);
	} else {
		error_log("ERROR P18: location outside program files");
	}
	return error;
}


int program::append_operand(int file, variable *var){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		list[file]._il->append_operand(var);
	} else {
		error_log("ERROR P19: location outside program files");
	}
	return error;
}



int program::get_opcode(int file, int *opcode){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->get_opcode(opcode);
	} else {
		error_log("ERROR P22: location outside program files");
	}
	return error;
}


variable *program::get_operand(int file){
	variable	*var;
	if((file >= 0) && (file < maximum_size)){
		var = list[file]._il->get_operand();
	} else {
		var = NULL;
		error_log("ERROR P23: location outside program files");
	}
	return var;
}



int program::get_operand_type(int file, int *type){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		type[0] = list[file]._il->get_operand_type();
	} else {
		error_log("ERROR P27: location outside program files");
	}
	return error;
}


int program::get_operand_text(int file, char *text){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		strcpy(text, list[file]._il->get_operand()->var->get_symbol());
	} else {
		error_log("ERROR P28: location outside program files");
	}
	return error;
}


int program::get_comment(int file, int type, int line, char *text){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->get_comment(type, line, text);
	} else {
		error_log("ERROR P29: location outside program files");
	}
	return error;
}


int program::set_comment(int file, int type, int operation, int line, char *text){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->set_comment(type, operation, line, text);
	} else {
		error_log("ERROR P30: location outside program files");
	}
	return error;
}


int program::label(int file, int command, int label_in, int inst_in, int *inst_out){
	int	error;
	error = ERROR;
	if((file >= 0) && (file < maximum_size)){
		error = list[file]._il->label(command, label_in, inst_in, inst_out);
	} else {
		error_log("ERROR P31: location outside program files");
	}
	return error;
}

