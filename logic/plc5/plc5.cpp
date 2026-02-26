
//
//    plc5.cpp - a plc5 emulator
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


#include "plc5.h"
//#include "instructions.h"







plc5::plc5(){
//printf("aaa 0\n");
	mem = new memory();
	prog = new program(10);
	communications = new queue(10);
//printf("aaa 2\n");


	// array_memory_make("O0", "Output memory", 2, _BIT);
	// array_memory_make("I1", "Input memory", 2, _BIT);
	// array_memory_make("S2", "Status memory", 10, _INTEGER);
	// array_memory_make("B3", "Bit memory", 2, _BIT);
	// array_memory_make("T4", "Timer memory", 4, _TIMER);
	// array_memory_make("C5", "Counter memory", 2, _COUNTER);
	// array_memory_make("R6", "Control memory", 2, _CONTROL);
	// array_memory_make("N7", "Integer memory", 4, _INTEGER);
	// array_memory_make("F8", "Real memory", 4, _REAL);
	// array_memory_make("ST9", "String memory", 5, _STRING);

//printf("aaa 4\n");

	stack.count = -1;
	p = new parser();
	stack_exec = new execution_stack[MAXIMUM_STACK_SIZE];
	stack_pointer = -1;
	first_scan = TRUE;
//printf("aaa 10\n");

}


void plc5::array_memory_make(char* name, char* comment, int size, int type){
	int		i, j;
	variable	*temp, *temp2, *temp3;
	char		work1[20];
	char		work2[20];
	char		work3[20];

	temp = mem->add(NULL, name, name, comment, _ARRAY);
	temp->var->set_size(size);
	for(i = 0; i < size; i++){
		sprintf(work1, "%s%d", name, i);
		sprintf(work2, "%s:%d", name, i);
		sprintf(work3, "%s %d", comment, i);

		if(type == _BIT){
			temp2 = mem->add(NULL, work1, work2, work3, _INTEGER);
			temp->var->set_var(i, temp2);
			for(j = 0; j < 16; j++){
				sprintf(work1, "%d", j);
				sprintf(work2, "%s:%d/%d", name, i, j);
				sprintf(work3, "bit %d", j);
printf("adding bit in plc5.cpp..\n");
				temp3 = mem->add(temp2, work1, work2, work3, _BIT);
			}
		} else if((type == _TIMER) || (type == _COUNTER)){
			temp2 = mem->add(NULL, work1, work2, work3, _UNION);
			temp->var->set_var(i, temp2);
			sprintf(work1, "PRE");
			sprintf(work2, "%s:%d.PRE", name, i);
			sprintf(work3, "preset");
			temp3 = mem->add(temp2, work1, work2, work3, _INTEGER);
			sprintf(work1, "ACC");
			sprintf(work2, "%s:%d.ACC", name, i);
			sprintf(work3, "accumulator");
			temp3 = mem->add(temp2, work1, work2, work3, _INTEGER);
		} else if(type == _CONTROL){
			temp2 = mem->add(NULL, work1, work2, work3, _UNION);
			temp->var->set_var(i, temp2);
			sprintf(work1, "POS");
			sprintf(work2, "%s:%d.POS", name, i);
			sprintf(work3, "position");
			temp3 = mem->add(temp2, work1, work2, work3, _INTEGER);
			sprintf(work1, "LEN");
			sprintf(work2, "%s:%d.LEN", name, i);
			sprintf(work3, "length");
			temp3 = mem->add(temp2, work1, work2, work3, _INTEGER);
		} else if(type == _STRING) {
			temp2 = mem->add(NULL, work1, work2, work3, _STRING);
			temp->var->set_var(i, temp2);
		} else if(type == _REAL) {
			temp2 = mem->add(NULL, work1, work2, work3, _REAL);
			temp->var->set_var(i, temp2);
		} else {
			temp2 = mem->add(NULL, work1, work2, work3, _INTEGER);
			temp->var->set_var(i, temp2);
		}
	}
}


plc5::~plc5(){
	delete mem;
	delete prog;
	delete communications;
	delete p;
	delete stack_exec;
}


int plc5::make_program(int file){
	int	error;

	error = prog->program_add(file, _IL_PROGRAM);
	focus_program = file;
	first_scan = TRUE;

	return error;
}


int plc5::parse_line(char *text){
	int	error,
		i, j, count,
		type, args;
	error = NO_ERROR;
	p->parse(text, ' ');
	count = p->counter();
	i = 0;
	while((i < count) && (error == NO_ERROR)){
		error = find_function(p->token(i), &type, &args);
		if(error == NO_ERROR){
			error = prog->create_opcode(focus_program, type);
			for(j = 1; j <= args; j++){
				error = decode_argument(p->token(i+j));
			}
			error = prog->append_instruction(focus_program);
			i = i + args + 1;
//		} else if(strcmp(p->token(i), "//") == 0) {
//			prog->set_comment(focus_program, __OPCODE_COMMENT, __INSERT_COMMENT, 0, p->token_string(i+1, count-1));
		} else {
			error_log("ERROR: could not find opcode in list");
		}
	}
	first_scan = TRUE;

	return error;
}


int plc5::parse_line(int file, int line, char *text){
	int	error,
		i, j, count,
		type, args;
	error = NO_ERROR;
	p->parse(text, ' ');
	count = p->counter();
	i = 0;
	focus_program = file;
	while((i < count) && (error == NO_ERROR)){
		error = find_function(p->token(i), &type, &args);
		if(error == NO_ERROR){
			error = prog->create_opcode(focus_program, type);
			for(j = 1; j <= args; j++){
				error = decode_argument(p->token(i+j));
			}
			if(line < 0){
				error = prog->append_instruction(focus_program);
			} else {
				error = prog->insert_instruction(focus_program, line);
			}
			i = i + args + 1;
		} else {
			error_log("ERROR: could not find opcode in list");
			error = ERROR;
		}
	}
	first_scan = TRUE;

	return error;
}

int plc5::update_labels(int program){
	int	error,
		j,
		used, type;
	float	value;

	error = NO_ERROR;
	program_info(program, &used, &type);
	if((used == USED) && (type == _IL_PROGRAM)){
		prog->label(program, LABEL_CLEAR_ALL, 0, 0, NULL);
		for(j = 0; (j < 32000) && (prog->get_instruction(program, j) != ERROR); j++){
			prog->get_opcode(program, &type);
			if(type == IL_LBL){
				value = get_float_from_memory();
				prog->label(program, LABEL_SET_VALUE, (int)value, j, NULL);
			}
		}
	} else {
		error_log("Error: program type in memory not recognized");
		error = ERROR;
	}

	return error;
}


int plc5::find_function(char *text, int *_type, int *_args){
	int	error,
		i;

	error = ERROR;
	_type[0] = 0;
	for(i = 0; (_type[0] == 0) && (comms[i].length > 0); i++){
		if(strcmp(comms[i].label, text) == 0){
			_args[0] = comms[i].args;
			_type[0] = comms[i].type;
			error = NO_ERROR;
		}
	}

	return error;
}


int plc5::decode_argument(char *text){
	int		error;
	variable	*temp;

	error = NO_ERROR;
//printf("decoding <%s>\n", text);
	if((temp = mem->find(text)) == NULL){
		temp = mem->add(NULL, NULL, NULL, NULL, _STRING); temp->var->set_string(text);
	}
	error = prog->append_operand(focus_program, temp);

	return error;
}


int plc5::decode_address(char *text){
	int	error;
	variable *temp;

	temp = create_address(text);
	if(temp != NULL){
		error = prog->append_operand(focus_program, temp);
	} else {
		error = ERROR;
	}

	return error;
}



variable *plc5::create_address(char *text){
	int	i,
		length,
		type,
		file,
		word,
		bit;
	char	work[30];
	variable	*temp1,
			*var;

//printf("decoding address <%s>\n", text);

	var = NULL;
	length = strlen(text);
	if(text[0] == 'I'){
		type = _INTEGER;
		file = 1;
	} else if(text[0] == 'O'){
		type = _INTEGER;
		file = 0;
	} else if(strncmp(text, "ST", 2) == 0){
		type = _STRING;
		file = atoi(&(text[2]));
	} else if(text[0] == 'S'){ // this must be after the ST check
		type = _INTEGER;
		file = 2;
	} else if(text[0] == 'B'){
		type = _INTEGER;
		file = atoi(&(text[1]));
	} else if(text[0] == 'T'){
		type = _UNION;
		file = atoi(&(text[1]));
	} else if(text[0] == 'C'){
		type = _UNION;
		file = atoi(&(text[1]));
	} else if(text[0] == 'R'){
		type = _UNION;
		file = atoi(&(text[1]));
	} else if(text[0] == 'N'){
		type = _INTEGER;
		file = atoi(&(text[1]));
	} else if(text[0] == 'F'){
		type = _REAL;
		file = atoi(&(text[1]));

	} else {
		error_log("ERROR: Memory locations must be preceded with type");
		return var;
	}
	
	// mem->memory_info(file, &used, &_type, &size);
	sprintf(work, "%c%d", text[0], file);
	temp1 = mem->find(NULL, work);
	if(temp1 == NULL){
		error_log("ERROR: Instruction address does not match memory");
		return var;
	}
	for(i = 1; (i < length) && (text[i] != ':'); i++){}
	if(i < length){
		word = atoi(&(text[i+1]));
		for(i = 1; (i < length) && (text[i] != '/'); i++){}
		if(i < length){
			if(strncmp(&(text[i+1]), "DN", 2) == 0){ bit = _DN;
			} else if(strncmp(&(text[i+1]), "EN", 2) == 0){ bit = _EN;
			} else if(strncmp(&(text[i+1]), "EU", 2) == 0){ bit = _EU;
			} else if(strncmp(&(text[i+1]), "EM", 2) == 0){ bit = _EM;
			} else if(strncmp(&(text[i+1]), "ER", 2) == 0){ bit = _ER;
			} else if(strncmp(&(text[i+1]), "UL", 2) == 0){ bit = _UL;
			} else if(strncmp(&(text[i+1]), "IN", 2) == 0){ bit = _IN;
			} else if(strncmp(&(text[i+1]), "FD", 2) == 0){ bit = _FD;
			} else if(strncmp(&(text[i+1]), "CU", 2) == 0){ bit = _CU;
			} else if(strncmp(&(text[i+1]), "CD", 2) == 0){ bit = _CD;
			} else if(strncmp(&(text[i+1]), "TT", 2) == 0){ bit = _TT;
			} else if(strncmp(&(text[i+1]), "OV", 2) == 0){ bit = _OV;
			} else if(strncmp(&(text[i+1]), "UN", 2) == 0){ bit = _UN;
			} else if((text[i+1] >= '0') && (text[i+1] <= '9')){
				bit = atoi(&(text[i+1]));
			} else {
				error_log("ERROR PLC4: bit address not recognized");
			}
			// type_addr[0] = _ADDRESS_BIT;
// add a bit here
			return var;
		}
		for(i = 1; (i < length) && (text[i] != '.'); i++){
		}
		if(i < length){
			if(strncmp(_PRESET_STRING, &(text[i+1]), strlen(_PRESET_STRING)) == 0){
// add a word here
				// type_addr[0] = _ADDRESS_WORD; bit =_PRESET;
			} else if(strncmp(_ACCUMULATOR_STRING, &(text[i+1]), strlen(_ACCUMULATOR_STRING)) == 0){
// add a word here
				// type_addr[0] = _ADDRESS_WORD; bit =_ACCUMULATOR;
			} else if(strncmp(_LENGTH_STRING, &(text[i+1]), strlen(_LENGTH_STRING)) == 0){
// add a word here
				// type_addr[0] = _ADDRESS_WORD; bit =_LENGTH;
			} else if(strncmp(_POSITION_STRING, &(text[i+1]), strlen(_POSITION_STRING)) == 0){
// add a word here
				// type_addr[0] = _ADDRESS_WORD; bit =_POSITION;
			} else {
				error_log("ERROR: incorrect word address");
			}
			return var;
		} else {
			bit = -1;
			// type_addr[0] = _ADDRESS_WORD, file;
// add a word here
		}
	} else {
		for(i = 1; (i < length) && (text[i] != '/'); i++){
		}
		if(i < length){
			bit = atoi(&(text[i+1]));
			word = (int)bit/16;
			bit = bit - 16*word;
			// type_addr[0] = _ADDRESS_BIT;
// add a bit here
			return var;
		} else {
			error_log("ERROR: There was a decoding error");
			return var;
		}
	}

	return var;
}


int plc5::scan(int prog_file){
	int	error, i,
		used, type;
	// timeb	time_value;
	

	error = NO_ERROR;

	// This is prescan stuff that should be moved to its own
	// subroutine later.
	if(first_scan == TRUE){
		// ftime(time_now);
		for(i = 0; i < prog->size(); i++){
			program_info(i, &used, &type);
			if((used == USED) && (type == _IL_PROGRAM)){
				update_labels(i);
			}
		}
		// ftime(time_start);
		ftime(&time_now);
	} else {
		time_last.millitm = time_now.millitm;
		time_last.time = time_now.time;
		time_last.timezone = time_now.timezone;
		time_last.dstflag = time_now.dstflag;

		ftime(&time_now);

		seconds = difftime(time_now.time, time_last.time) + (time_now.millitm - time_last.millitm)*0.001;
		// if(seconds > 0.0) printf("Time clock %f\n", seconds);
		// milliseconds = milliseconds + 100;
	}

	stack_pointer = 0;
	stack_exec[stack_pointer].ladder_flag = 0;
	stack_exec[stack_pointer].end_flag = 0;
	stack_exec[stack_pointer].prog_file = 2;
	stack_exec[stack_pointer].instruction = 0;
	stack.count = -1;
	for(i = 0; (stack_pointer >= 0) && ((error = scan_step()) != ERROR); i++){
	}
	first_scan = FALSE;
//communications->dump();

	return error;
}



int plc5::scan_step(){
	int	error;
	int	_type;

	error = NO_ERROR;
	if((prog->get_instruction(stack_exec[stack_pointer].prog_file, stack_exec[stack_pointer].instruction) != ERROR) && (stack_pointer >= 0)){
//printf("STEP  prog %d      inst %d \n", stack_exec[stack_pointer].prog_file, stack_exec[stack_pointer].instruction);
		prog->get_opcode(stack_exec[stack_pointer].prog_file, &_type);
		if((_type >= IL_BASIC_LOGIC_START) && (_type <= IL_BASIC_LOGIC_END)){
			error = instructions_basic_logic(_type);
		} else if((_type >= IL_TIMER_COUNTER_START) && (_type <= IL_TIMER_COUNTER_END)){
			error = instructions_timer_counter(_type);
		} else if((_type >= IL_MATH_START) && (_type <= IL_MATH_END)){
			error = instructions_math(_type);
		} else if((_type >= IL_DATA_START) && (_type <= IL_DATA_END)){
			error = instructions_data(_type);
		} else if((_type >= IL_PROGRAM_CONTROL_START) && (_type <= IL_PROGRAM_CONTROL_END)){
			error = instructions_program_control(_type);
		} else if((_type >= IL_FANCY_START) && (_type <= IL_FANCY_END)){
			error = instructions_fancy(_type);
		} else if((_type >= IL_ASCII_START) && (_type <= IL_ASCII_END)){
			error = instructions_ascii(_type);
		} else {
			error_log("ERROR: Unrecognized op code");
printf("The Culprit was %d\n", _type);
			error = ERROR;
			return error;
		}
	} else {
		error = ERROR; // the program is done without an end statement
		error_log("Warning: The program has ended without an END statement");
	}

	if(stack_exec[stack_pointer].end_flag != 0){
		stack_pointer--;
		stack_exec[stack_pointer].instruction++;
	} else {
		stack_exec[stack_pointer].instruction++;
	}

	return error;
}


int plc5::instructions_basic_logic(int _type){
	int	error,
		value, value2;
	variable	*var1;

	error = NO_ERROR;
	if(_type == IL_LD){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		error = push(var1->var->get_bit());
	} else if(_type == IL_NOT){
		pull(&value);
		if(value == 0){
			push(1);
		} else {
			push(0);
		}
	} else if(_type == IL_AFI){
		pull(&value);
		push(0);
	} else if(_type == IL_ATI){
		pull(&value);
		push(1);
	} else if(_type == IL_OTL){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0) var1->var->set_bit(1);
		error_log("Warning: S and R bits will not be retained after power off");
	} else if(_type == IL_SOR){
		error = push(1);
		stack_exec[stack_pointer].ladder_flag = 1;
	} else if(_type == IL_XOR){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		value = var1->var->get_bit();
		pull(&value2);
		error = push((~value * value2) | (value * ~value2));
	} else if(_type == IL_NXB){
		pull(&value);
		pull(&value2);
		push(value);
		push(value2);
	} else if(_type == IL_OR){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		pull(&value2);
		error = push(var1->var->get_bit() | value2);
	} else if((_type == IL_ORB) || (_type == IL_BND)){
		pull(&value);
		pull(&value2);
		error = push(value | value2);
	} else if(_type == IL_OTU){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0) var1->var->set_bit(0);
		error_log("Warning: S and R bits will not be retained after power off");
	} else if(_type == IL_OTE){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0) value = 1;
		var1->var->set_bit(value);
	} else if(_type == IL_ANB){
		pull(&value);
		pull(&value2);
		error = push(value & value2 & 1);
	} else if((_type == IL_AND) || (_type == IL_XIC)){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		pull(&value2);
		error = push(var1->var->get_bit() & value2);
	} else if(_type == IL_XIO){
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		pull(&value2);
		error = push(~var1->var->get_bit() & value2);
	} else if(_type == IL_BST){
		pull(&value);
		push(value);
		push(value);
	} else if(_type == IL_EOR){
		error = pull(&value);
		stack_exec[stack_pointer].ladder_flag = 0;
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}



int plc5::instructions_math(int _type){
	int	error,
		value, value2,
		_word,
		position, length, start,
		i;
	float	sourceA, sourceB, total, total2, total3;
	variable	*var1,
			*var2,
			*var3,
			*var4,
			*var5,
			*var6;

	error = NO_ERROR;
	if(_type == IL_ADD){
//printf("ADD\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			error = set_float_in_memory(sourceA + sourceB);
		}
	} else if(_type == IL_SUB){
//printf("SUB\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			error = set_float_in_memory(sourceA - sourceB);
		}
	} else if(_type == IL_MUL){
//printf("MUL\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			error = set_float_in_memory(sourceA * sourceB);
		}
	} else if(_type == IL_DIV){
//printf("DIV\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			if(sourceB != 0.0){
			error = set_float_in_memory(sourceA / sourceB);
			} else {
				error_log("ERROR: Divide by zero error");
			}
		}
	} else if(_type == IL_CLR){
//printf("CLR\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0)error = set_float_in_memory(0.0);

	} else if(_type == IL_NEG){
//printf("NEG\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(-sourceA);
		}
	} else if(_type == IL_BAND){
//printf("BAND\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3->var->set_int(var1->var->get_int() & var2->var->get_int());
		}
	} else if(_type == IL_BNOT){
//printf("BNOT\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			//error =mem->memory_get(_file,_word,_bit,&value);
			//var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var2->var->set_int(~var1->var->get_int());
		}
	} else if(_type == IL_BOR){
//printf("BOR\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3->var->set_int(var1->var->get_int() | var2->var->get_int());
		}
	} else if(_type == IL_BXOR){
//printf("BXOR\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			value = var1->var->get_int();
			value2 = var2->var->get_int();
			var3->var->set_int((value & ~value2) | (~value & value2));
		}
	} else if(_type == IL_SQR){
//printf("SQR\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			if(sourceA >= 0.0){
			error = set_float_in_memory(sqrt(sourceA));
			} else {
				error_log("ERROR: Complex Result");
			}
		}
	} else if(_type == IL_COS){
//printf("COS\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(cos(sourceA));
		}
	} else if(_type == IL_SIN){
//printf("SIN\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(sin(sourceA));
		}
	} else if(_type == IL_TAN){
//printf("TAN\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(tan(sourceA));
		}
	} else if(_type == IL_ACS){
//printf("ACS\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			if(sourceA >= 0.0){
			error = set_float_in_memory(acos(sourceA));
			} else {
				error_log("ERROR: Complex Result");
			}
		}
	} else if(_type == IL_ASN){
//printf("ASN\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(asin(sourceA));
		}
	} else if(_type == IL_ATN){
//printf("ATN\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(atan(sourceA));
		}
	} else if(_type == IL_LN){
//printf("LN\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			if(sourceA >= 0.0){
			error = set_float_in_memory(log(sourceA));
			} else {
				error_log("ERROR: Can't do a natural log of a negative number");
			}
		}
	} else if(_type == IL_LOG){
//printf("LOG\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			if(sourceA >= 0.0){
			error = set_float_in_memory(log10(sourceA));
			} else {
				error_log("ERROR: Can't do a log of a negative number");
			}
		}
	} else if(_type == IL_XPY){
//printf("XPY\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			error = set_float_in_memory(pow(sourceA, sourceB));
		}
	} else if(_type == IL_FRD){
//printf("FRD - Not implemented yet\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			// Put conversion here
			error_log("ERROR: TOD not implemented yet");
			error = set_float_in_memory(sourceA);
		}
	} else if(_type == IL_TOD){
//printf("TOD - Not implemented yet\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			if((sourceA >= 0) || (sourceA <= 9999)){
			// Put conversion here
			error_log("ERROR: TOD not implemented yet");
			error = set_float_in_memory(sourceA);
			} else {
				error_log("ERROR: Too large for BCD");
			}
		}
	} else if(_type == IL_DEG){
//printf("DEG\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(sourceA*180/PI);
		}
	} else if(_type == IL_RAD){
//printf("RAD\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(sourceA*PI/180);
		}
	} else if(_type == IL_SRT){
//printf("SRT - only works for floats now\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);

		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// status
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// start
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// length
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// position

		length = var4->var->get_int();
		start = var3->var->get_int();

		if(first_scan == TRUE){
			// mem->memory_set(_file3, _word3, _POSITION, position);
			var5->var->set_int(0);
		}
		if(value != 0){
			// mem->memory_get(_file3, _word3, _EN, &value);
			value = var2->var->get_bit(_EN);
			if(value == 0){
				var2->var->set_bit(_EN, 1);
				sourceA = (var1->var->get_variable(start))->var->get_real();
				// mem->memory_set(_file3, _word3, _EN, 1);
				// mem->memory_get(_file, _word, &sourceA);
				for(i = start+1; i < (start+length); i++){
					sourceB = (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceB);
					if(sourceA < sourceB){
						sourceA = sourceB;
					} else if(sourceA > sourceB){
						(var1->var->get_variable(i-1))->var->set_real(sourceB);
						// mem->memory_set(_file, i-1, sourceB);
						(var1->var->get_variable(i))->var->set_real(sourceA);
						// mem->memory_set(_file, i, sourceA);
					}
				}
				sourceA = (var1->var->get_variable(start+length-1))->var->get_real();
				// mem->memory_get(_file, _word+length-1, &sourceA);
				for(i = start+length-2; i >= start; i--){
					sourceB = (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceB);
					if(sourceA > sourceB){
						sourceA = sourceB;
					} else if(sourceA < sourceB){
						(var1->var->get_variable(i+1))->var->set_real(sourceB);
						(var1->var->get_variable(i))->var->set_real(sourceA);
						// mem->memory_set(_file, i+1, sourceB);
						// mem->memory_set(_file, i, sourceA);
					}
				}
				var2->var->set_bit(_DN, 1);
				
				// mem->memory_set(_file3, _word3, _DN, 1);
				// mem->memory_set(_file2, _word2, total/(float)length); // what did this do?
			}
		} else {
			var2->var->set_bit(_EN, 0);
			// mem->memory_set(_file3, _word3, _EN, 0);
		}
	} else if(_type == IL_AVE){
//printf("AVE\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// status
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// start
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// length
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// position
		var6 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// result
		
		start = var3->var->get_int();
		length = var4->var->get_int();
		position = var5->var->get_int();
		if(first_scan == TRUE){
			// var2->var->set_bit(_POSITION, 0);
			// mem->memory_set(_file3, _word3, _POSITION, position);
		}
		if(value != 0){
			value = var2->var->get_bit(_EN);
			// mem->memory_get(_file3, _word3, _EN, &value);
			if(value == 0){
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _EN, 1);
				total = 0.0;
				for(i = start; i < start + length; i++){
					total += (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceA);
					// total += sourceA;
				}
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _DN, 1);
				var6->var->set_real(total/(double) length);
				// mem->memory_set(_file2, _word2, total/(float)length);
			}
		} else {
			var2->var->set_bit(_EN, 0);
			// mem->memory_set(_file3, _word3, _EN, 0);
		}
	} else if(_type == IL_STD){
//printf("STD - only works for floats now\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// status
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// start
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// length
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// position
		var6 = prog->get_operand(stack_exec[stack_pointer].prog_file);	// result
		
		start = var3->var->get_int();
		length = var4->var->get_int();
		position = var5->var->get_int();
		if(first_scan == TRUE){
			// var2->var->set_bit(_POSITION, 0);
			// mem->memory_set(_file3, _word3, _POSITION, position);
		}
		if(value != 0){
			value = var2->var->get_bit(_EN);
			// mem->memory_get(_file3, _word3, _EN, &value);
			if(value == 0){
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _EN, 1);
				total = 0.0;
				for(i = start; i < start + length; i++){
					total += (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceA);
					// total += sourceA;
				}
				total = total / (float)length;
				total2 = 0.0;
				for(i = _word; i < _word+length; i++){
					sourceA = (var1->var->get_variable(i))->var->get_real();
					// mem->memory_get(_file, i, &sourceA);
					total3 = (sourceA - total);
					total2 += total3 * total3;
				}
				total2 = total2 / (float)(length-1);
				total = sqrt(total2);
				var2->var->set_bit(_EN, 1);
				// mem->memory_set(_file3, _word3, _DN, 1);
				var6->var->set_real(total/(double) length);
				// mem->memory_set(_file2, _word2, total);
			}
		} else {
			var2->var->set_bit(_EN, 0);
			// mem->memory_set(_file3, _word3, _EN, 0);
		}
	} else {
		error = ERROR;
		error_log("ERROR: math instruction type not found");
	}

	return error;
}



int plc5::instructions_data(int _type){
	int	error,
		i, start, length, start2,
		value, value2, value3;
	float	sourceA, sourceB, sourceC;
	variable	*var1,
			*var2,
			*var3,
			*var4,
			*var5;

	error = NO_ERROR;
	if(_type == IL_EQ){
//printf("EQ\n");
		pull(&value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			value = 0;
			if(sourceA == sourceB) value = 1;
		}
		push(value);
	} else if(_type == IL_GE){
//printf("GE\n");
		pull(&value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			value = 0;
			if(sourceA >= sourceB) value = 1;
		}
		push(value);
	} else if(_type == IL_GT){
//printf("GT\n");
		pull(&value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			value = 0;
			if(sourceA > sourceB) value = 1;
		}
		push(value);
	} else if(_type == IL_LE){
//printf("LE\n");
		pull(&value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			value = 0;
			if(sourceA <= sourceB) value = 1;
		}
		push(value);
	} else if(_type == IL_LIM){
//printf("LIM\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			sourceC = get_float_from_memory();
			value = 0;
			if(sourceA <= sourceC){
				if((sourceB >= sourceA)
				&& (sourceB <=sourceC)) value = 1;
			} else {
				if((sourceB < sourceA)
				|| (sourceB > sourceC)) value = 1;
			}
		}
		push(value);
	} else if(_type == IL_LT){
//printf("LT\n");
		pull(&value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			value = 0;
			if(sourceA < sourceB) value = 1;
		}
		push(value);
	} else if(_type == IL_MEQ){
//printf("MEQ\n");
		pull(&value);
		if(value != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			value = var1->var->get_int();
			value2 = var2->var->get_int();
			value3 = var3->var->get_int();
			if((value & value2) == (value2 & value3)){
				value = 1;
			}
		}
		push(value);
	} else if(_type == IL_MOV){
//printf("MOV\n");
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			sourceA = get_float_from_memory();
			error = set_float_in_memory(sourceA * sourceB);
		}
	} else if(_type == IL_MVM){
//printf("MVM\n");
		pull(&value2);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value2 != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			value = var1->var->get_int();
			value2 = var2->var->get_int();
			value3 = var3->var->get_int();
			value = (value & value2) | (value3 & ~value2);
			var3->var->set_int(value);
		}
	} else if(_type == IL_FLL){
//printf("FLL\n");
		pull(&value2);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value2 != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // array
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // start
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // langth
			var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // value
			sourceA = var4->var->get_real();
			start = var2->var->get_int();
			length = var3->var->get_int();
			// error=prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
			// sourceB = var3->var->get_real();
			for(i = start; i < length+start; i++){
				(var1->var->get_variable(i))->var->set_real(sourceA);
				// error=mem->memory_set(_file2, _word2+i, _bit, sourceA);
			}		
		}
	} else if(_type == IL_COP){
//printf("COP\n");
		pull(&value2);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value2 != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // source
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // start
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // langth
			var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination
			var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination start

			start = var2->var->get_int();
			length = var3->var->get_int();
			start2 = var5->var->get_int();
			//error=prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
			//error=prog->get_operand(stack_exec[stack_pointer].prog_file,&_file2,&_word2,&_bit2);
			// sourceA = get_float_from_memory();
			for(i = 0; i < length; i++){
				(var4->var->get_variable(start2 + i))->var->set_real(
					(var1->var->get_variable(start+i))->var->get_real());
				// error=mem->memory_get(_file, _word+i, _bit, &value2);
				// error=mem->memory_set(_file2, _word2+i, _bit, value2);
			}		
		}
	} else if(_type == IL_NE){
//printf("NE\n");
		pull(&value);
		if(value != 0){
			sourceA = get_float_from_memory();
			sourceB = get_float_from_memory();
			value = 0;
			if(sourceA != sourceB) value = 1;
		}
		push(value);
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}




int plc5::instructions_program_control(int _type){
	int	error,
		value, value2, value3;
	variable	*var1;

	error = NO_ERROR;
	if(_type == IL_TND){
//printf("TND\n");
		error = pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0) stack_exec[stack_pointer].end_flag = 1;
	} else if(_type == IL_END){
//printf("END\n");
		error = pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		stack_exec[stack_pointer].end_flag = 1;
	} else if(_type == IL_JSR){
//printf("JSR\n");
		error = pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
			if(error == NO_ERROR){
				stack_pointer++;
				stack_exec[stack_pointer].prog_file = var1->var->get_int();
				stack_exec[stack_pointer].instruction = -1;
				stack_exec[stack_pointer].end_flag = 0;
				stack_exec[stack_pointer].ladder_flag = 0;
			} else {
				error_log("ERROR: can't find program number");
			}
		}
	} else if(_type == IL_RET){
//printf("RET\n");
		error = pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			stack_exec[stack_pointer].end_flag = 1;
		}
	} else if(_type == IL_ONS){
//printf("ONS\n");
		error = pull(&value);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		if(value == 0){
			var1->var->set_bit(0);
		} else {
			value2 = var1->var->get_bit();
			if(value2 == 0){
				value = 1;
				var1->var->set_bit(1);
			}
		}
		push(value);
	} else if(_type == IL_JMP){
//printf("JMP\n");
		value2 = (int)get_float_from_memory();
		if(prog->label(stack_exec[stack_pointer].prog_file, LABEL_GET_VALUE, value2, 0, &value3) == NO_ERROR){
			stack_exec[stack_pointer].instruction = value3;
		} else {
			error_log("ERROR: Jump to label failed");
		}
	} else if(_type == IL_LBL){
//printf("LBL\n");
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		// do nothing here
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}


int plc5::instructions_fancy(int _type){
	int	error;
	int	int_val,
		i,
		start, length,
		_en, _ul, _dn, _er,
		number, slot, state,
		val, val2,
		mask, position;
	float	value;
	variable	*var1,
			*var2,
			*var3,
			*var4,
			*var5,
			*var6,
			*var7;

	error = NO_ERROR;
	if(_type == IL_BSL){
//printf("BSL\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		value = get_float_from_memory();
		_en = var2->var->get_bit(_EN);
		if(first_scan == TRUE){
			_en = 0; _ul = 0;
		}
		if((int_val != 0) && (_en == 0)){
			val = var3->var->get_bit(_EN);
error_log("ERROR: Not fully implemented yet - need to look at K&R");
		}
		var2->var->set_bit(_EN, int_val);
		var2->var->set_bit(_UL, int_val);
	} else if(_type == IL_BSR){
//printf("BSR\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file);
		value = get_float_from_memory();
		_en = var1->var->get_bit(_EN);
		if(first_scan == TRUE){
			_en = 0; _ul = 0;
		}
		if((int_val != 0) && (_en == 0)){
			val = var3->var->get_bit();
error_log("ERROR: Not fully implemented yet - I need to look at K&R");
		}
		var2->var->set_bit(_EN, int_val);
		var2->var->set_bit(_EU, int_val);
	} else if((_type == IL_FFL) && (_type == IL_LFL)){
//printf("FFL and LFL\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // stack array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // stack start
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination,source
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
		var6 = prog->get_operand(stack_exec[stack_pointer].prog_file); // position

		// value = get_float_from_memory();
		// value2 = get_float_from_memory();
		start = var2->var->get_int();
		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		length = var5->var->get_int();
		position = var6->var->get_int();

		// error = mem->memory_get(_file, _word, _bit, &val);
		// error = mem->memory_get_bit(_file3, _word3, _EN, &_en);
		// error = mem->memory_get_bit(_file3, _word3, _DN, &_dn);
		// error = mem->memory_get(_file3, _word3, _POSITION, &position);
		// error = mem->memory_get(_file3, _word3, _LENGTH, &length);
		if(first_scan == TRUE){
			_en = 0; _dn = 0;
			length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)position;
			// if(position >= length) _dn = 1;
		}
		if((int_val != 0) && (_en == 0) && (_dn == 0)){
			position++;
			if(position >= length) _dn = 1;
			(var1->var->get_variable(start+position))->var->set_real(var2->var->get_real());
			// error = mem->memory_set(_file2, _word2+position, _bit2, val);
		}
		_en = int_val;
		var4->var->set_bit(_EN, int_val);
		var4->var->set_bit(_UL, int_val);
		var6->var->set_int(position);

		//error = mem->memory_set_bit(_file2, _word2, _EN, int_val);
		//error = mem->memory_set_bit(_file2, _word2, _UL, int_val);
		//error = mem->memory_set(_file2, _word2, _POSITION, position);
	} else if(_type == IL_FFU){
//printf("FFU\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // stack array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // stack start
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination,source
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
		var6 = prog->get_operand(stack_exec[stack_pointer].prog_file); // position
		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		position = var6->var->get_int();
		length = var5->var->get_int();

		if(first_scan == TRUE){
			_en = 0; _dn = 0;
			// length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)value2;
			if(position >= length) _dn = 1;
		}
		if((int_val != 0) && (_en == 0) && (position >= 0)){
			var3->var->set_real((var1->var->get_variable(start))->var->get_real());
			// error = mem->memory_set(_file2, _word2, val);
			for(i = start; i < start+position; i++){
				(var1->var->get_variable(i))->var->set_real((var1->var->get_variable(i+1))->var->get_real());
				// mem->memory_get(_file, _word+i, _bit, &val);
				// mem->memory_set(_file, _word+i-1, _bit, val);
			}
			position--;
			if(position < 0) position = 0;
			if(position < length) _dn = 0;
		}
		_en = int_val;
		
		var4->var->set_bit(_EN, int_val);
		var4->var->set_bit(_UL, int_val);
		var6->var->set_int(position);
		// error = mem->memory_set_bit(_file2, _word2, _EN, int_val);
		// error = mem->memory_set_bit(_file2, _word2, _UL, int_val);
		// error = mem->memory_set(_file2, _word2, _POSITION, position);
	} else if(_type == IL_LFU){
//printf("LFU\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // stack array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // stack start
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination,source
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
		var6 = prog->get_operand(stack_exec[stack_pointer].prog_file); // position

		_en = var4->var->get_bit(_EN);
		_dn = var4->var->get_bit(_DN);
		start = var2->var->get_int();
		length = var5->var->get_int();
		position = var6->var->get_int();

		if(first_scan == TRUE){
			_en = 0; _dn = 0;
			// length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)value2;
			if(position >= length) _dn = 1;
		}
		if((int_val != 0) && (_en == 0) && (position >= 0)){
			var3->var->set_real((var1->var->get_variable(start+position))->var->get_real());
			position--;
			if(position < 0) position = 0;
			if(position < length) _dn = 0;
		}
		_en = int_val;
		var4->var->set_bit(_EN, _en);
		var4->var->set_bit(_DN, _dn);
		var6->var->set_int(position);
	} else if(_type == IL_SQO){
//printf("SQO\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // sequencer array
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // sequencer start
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination,source
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
		var6 = prog->get_operand(stack_exec[stack_pointer].prog_file); // position
		var7 = prog->get_operand(stack_exec[stack_pointer].prog_file); // mask

		_en = var4->var->get_bit(_EN);
		start = var2->var->get_int();
		length = var5->var->get_int();
		position = var6->var->get_int();
		mask = var7->var->get_int();

		if(first_scan == TRUE){
			_en = 0;
			// length = (int)value;
			// error = mem->memory_set(_file2, _word2, _LENGTH, length);
			// position = (int)value2;
			if(position >= length) position = 0;
		}
		if((int_val != 0) && (_en == 0)){
			position++;
			if(position > length) position = 1;
			val = (var1->var->get_variable(start+position))->var->get_int();
			val2 = var3->var->get_int();
			var3->var->set_int((val & mask) + (val2 & ~mask));
			// error = mem->memory_get(_file, _word+position, _bit, &val);
			// error = mem->memory_set(_file2, _word2, _bit2, val);
		}
		_en = int_val;
		var4->var->set_bit(_EN, _en);
		var6->var->set_int(position);
	} else if(_type == IL_SQI){
printf("SQI - not implemented now\n");
	} else if(_type == IL_SQL){
printf("SQL - not implemented now\n");
	} else if(_type == IL_MSG){
//printf("MSG\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // message source
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // slot
		_en = var2->var->get_bit(_EN);
		_dn = var2->var->get_bit(_DN);
		_er = var2->var->get_bit(_ER);

		if(first_scan == TRUE){
			_en = 0; _dn = 0; _er = 0;
		}
		if((int_val != 0) && (_en == 0)){
			// error = mem->memory_get(_file, _word, text);
			number = communications->add(var1->var->get_string());
//printf("Message is [%s] \n", var1->var->get_string());
			_en = 1; _dn = 0; _er = 0;
			communications->status(number, _WAITING);
			var3->var->set_int(number);
			// error = mem->memory_set(_file3, _word3, _POSITION, number);
		} else if((_en != 0) && (_dn == 0)){
			slot = var3->var->get_int();
			// error = mem->memory_get(_file3, _word3, _POSITION, &slot);
			state = communications->status(slot);
			if(state == _DONE){
				_dn = 1;
				communications->release(slot);
			} else if(state == _ERROR){
				_dn = 1; _er = 1;
				communications->release(slot);
			}
		}
		if(_dn == 1) _en = int_val;
		var2->var->set_bit(_EN, _en);
		var2->var->set_bit(_DN, _dn);
		var2->var->set_bit(_ER, _er);
	} else {
		error = ERROR;
		error_log("ERROR: Fancy functions not recognized");
	}

	return error;
}




int plc5::instructions_ascii(int _type){
	int	error;
	int	int_val,
		length, start,
		_en, _dn, _er,
//		dest,
		number, slot, state;
	char	text1[100];
	char	*texttemp;
	float	value;
	variable	*var1, *var2, *var3, *var4, *var5;

	error = NO_ERROR;
	if(_type == IL_AIC){
//printf("AIC\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // integer source
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // string destination
			var2->var->set_string(var1->var->get_string());
		}
	} else if(_type == IL_ACI){
//printf("ACI\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // string source
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // number destination
			var2->var->set_real(var1->var->get_real());
		}
	} else if(_type == IL_ASC){
//printf("ASC\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
error_log("WARNING: Not implemented - I wasn't sure about this - no manuals at the time");
		if(int_val != 0){
//			error=prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
//			error = mem->memory_get(_file, _word, text1);
//			error = set_float_in_memory(atof(text1));
		}
	} else if(_type == IL_ACN){
//printf("ACN\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // string base
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // appended string
			texttemp = new char[strlen(var1->var->get_string())+strlen(var2->var->get_string())+1];
			strcpy(texttemp, var1->var->get_string());
			strcat(texttemp, var2->var->get_string());
			var1->var->set_string(texttemp);
		}
	} else if(_type == IL_ASR){
//printf("ASR\n");
		pull(&int_val);
		if(int_val != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // string A
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // string B
			if(strcmp(var1->var->get_string(), var2->var->get_string()) != 0) int_val = 0;
		}
		push(int_val);
	} else if(_type == IL_AEX){
//printf("AEX\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		if(int_val != 0){
			var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // source string
			var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // start
			var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
			var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination string

			texttemp = new char[length+1];
			strncpy(texttemp, &(var1->var->get_string()[start]), length);
			var4->var->set_string(texttemp);
		}
	} else if(_type == IL_ARL){
//printf("ARL\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // channel
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // destination
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // position

		value = var1->var->get_int();
		_en = var3->var->get_bit(_EN);
		_dn = var3->var->get_bit(_DN);
		_er = var3->var->get_bit(_ER);

		if(first_scan == TRUE){
			_en = 0; _dn = 0; _er = 0;
		}
		if((int_val != 0) && (_en == 0)){
			if((int)value == 0){ 
				sprintf(text1, "APPEND COM1 SEND MEMORY %s", var2->var->get_symbol());
			} else if((int)value == 1){
				sprintf(text1, "APPEND COM2 SEND MEMORY %s", var2->var->get_symbol());
			} else {
				error_log("WARNING: Channel number was incorrect using channel 0");
				sprintf(text1, "APPEND COM1 SEND MEMORY %s", var2->var->get_symbol());
			}
			number = communications->add(text1);
			_en = 1; _dn = 0; _er = 0;
			communications->status(number, _WAITING);
			var5->var->set_int(number);
			// error = mem->memory_set(_file2, _word2, _POSITION, number);
		} else if((_en != 0) && (_dn == 0)){
			slot = var5->var->get_int();
			// error = mem->memory_get(_file2, _word2, _POSITION, &slot);
			state = communications->status(slot);
			if(state == _DONE){
				_dn = 1;
				communications->release(slot);
			} else if(state == _ERROR){
				_dn = 1; _er = 1;
				communications->release(slot);
			}
		}
		if(_dn == 1){
//printf("soup 5 \n");
			_en = int_val;
		}
		var3->var->set_bit(_EN, _en);
		var3->var->set_bit(_DN, _dn);
		var3->var->set_bit(_ER, _er);
	} else if(_type == IL_AWT){
//printf("AWT\n");
		pull(&int_val);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(int_val);
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // channel
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // source
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // length
		var5 = prog->get_operand(stack_exec[stack_pointer].prog_file); // position

		value = var1->var->get_int();
		length = var4->var->get_int();
		_en = var3->var->get_bit(_EN);
		_dn = var3->var->get_bit(_DN);
		_er = var3->var->get_bit(_ER);

		if(first_scan == TRUE){
			_en = 0; _dn = 0; _er = 0;
		}
		if((int_val != 0) && (_en == 0)){
			if((int)value == 0){
				sprintf(text1, "APPEND MEMORY %s SEND COM1", var2->var->get_symbol());
			} else if((int)value == 1){
				sprintf(text1, "APPEND MEMORY %s SEND COM2", var2->var->get_symbol());
			} else {
				error_log("WARNING: Channel number was incorrect using channel 0");
				sprintf(text1, "APPEND MEMORY %s SEND COM1", var2->var->get_symbol());
			}
			number = communications->add(text1);
			_en = 1; _dn = 0; _er = 0;
			communications->status(number, _WAITING);
			var5->var->set_int(number);
			// error = mem->memory_set(_file2, _word2, _POSITION, number);
		} else if((_en != 0) && (_dn == 0)){
			slot = var5->var->get_int();
			// error = mem->memory_get(_file2, _word2, _POSITION, &slot);
			state = communications->status(slot);
			if(state == _DONE){
				_dn = 1;
				communications->release(slot);
			} else if(state == _ERROR){
				_dn = 1; _er = 1;
				communications->release(slot);
			}
		}
		if(_dn == 1) _en = int_val;
		var3->var->set_bit(_EN, _en);
		var3->var->set_bit(_DN, _dn);
		var3->var->set_bit(_ER, _er);
	} else {
		error_log("ERROR: ASCII Function not recognized");
		error = ERROR;
	}

	return error;
}



int plc5::instructions_timer_counter(int _type){
	int	error,
		value,
		// time_diff,
		_en, _dn, _tt, _cu, _cd, _ov, _un;
	double	_preset, _accumulator;
		//, _time_base;
	variable	*var1, *var2, *var3, *var4;

	error = NO_ERROR;
	if((_type == IL_TON) || (_type == IL_RTO)){
//printf("TON and RTO\n");
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		// var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // base
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // delay
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // accumulator

		_preset = var3->var->get_real();
		_accumulator = var4->var->get_real();
		_en = var1->var->get_bit(_EN);
		_dn = var1->var->get_bit(_DN);
		_tt = var1->var->get_bit(_TT);

		if(first_scan == TRUE){
			// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
			// mem->memory_set(_file, _word, _PRESET, (int)_preset);
		} else {
			// mem->memory_get(_file, _word, _PRESET, &value);
			// _preset = (float)value;
			// mem->memory_get(_file,_word,_ACCUMULATOR, &value);
			// _accumulator = (float)value;
		}
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			_en = 1;
			_accumulator += seconds; //  / _time_base;
			// (int)(milliseconds/(1000.0 * _time_base))-
			// (int)(milliseconds_last/(1000.0 * _time_base));
			//_accumulator += time_diff;
			if(_accumulator >= _preset){
if(_dn != 1) printf("DONE------- %f ----\n", _accumulator);
				_accumulator = _preset;
				_tt = 0;
				_dn = 1;
			} else {
				_tt = 1;
				_dn = 0;
			}
		} else {
			_tt = 0;
			_en = 0;
			if(_type == IL_TON){
				_accumulator = 0;
				_dn = 0;
			}
		}
		var1->var->set_bit(_EN, _en);
		var1->var->set_bit(_DN, _dn);
		var1->var->set_bit(_TT, _tt);
		var4->var->set_real(_accumulator);
		// mem->memory_set(_file, _word, _EN, _en);
		// mem->memory_set(_file, _word, _DN, _dn);
		// mem->memory_set(_file, _word, _TT, _tt);
		// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
	} else if(_type == IL_TOF){
//printf("TOF\n");
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		// var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // base
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // delay
		var4 = prog->get_operand(stack_exec[stack_pointer].prog_file); // accumulator

		// _time_base = var2->var->get_real();
		_preset = var3->var->get_real();
		_accumulator = var4->var->get_real();
		_en = var1->var->get_bit(_EN);
		_dn = var1->var->get_bit(_DN);
		_tt = var1->var->get_bit(_TT);

		if(first_scan == TRUE){
			// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
			// mem->memory_set(_file, _word, _PRESET, (int)_preset);
		} else {
			// mem->memory_get(_file, _word, _PRESET, &value);
			// _preset = (float)value;
			// mem->memory_get(_file,_word,_ACCUMULATOR, &value);
			// _accumulator = (float)value;
		}

		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			_en = 1;
			_dn = 1;
			_tt = 0;
			_accumulator = 0;
		} else {
			_en = 0;
			if(_tt == 1){
				_accumulator += seconds; // (int)(seconds / _time_base);
				// time_diff = (int)(milliseconds/(1000.0*_time_base))-(int)(milliseconds_last/(1000.0*_time_base));
				// _accumulator += time_diff;
				if(_accumulator >= _preset){
					_accumulator = 0;
					_tt = 0;
					_dn = 0;
				}
			}
		}
		var1->var->set_bit(_EN, _en);
		var1->var->set_bit(_DN, _dn);
		var1->var->set_bit(_TT, _tt);
		var4->var->set_real(_accumulator);
	} else if((_type == IL_CTU) || (_type == IL_CTD)){
//printf("CTU and CTD\n");
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		var2 = prog->get_operand(stack_exec[stack_pointer].prog_file); // limit
		var3 = prog->get_operand(stack_exec[stack_pointer].prog_file); // accumulator

		// error =prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
		_preset = var2->var->get_int();
		_accumulator = var3->var->get_int();
		_cu = var1->var->get_bit(_CU);
		_cd = var1->var->get_bit(_CD);
		_dn = var1->var->get_bit(_DN);
		_ov = var1->var->get_bit(_OV);
		_un = var1->var->get_bit(_UN);
		if(first_scan == TRUE){
			// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
			// mem->memory_set(_file, _word, _PRESET, (int)_preset);
		} else {
			// mem->memory_get(_file, _word, _PRESET, &value);
			// _preset = (float)value;
			// mem->memory_get(_file,_word,_ACCUMULATOR, &value);
			// _accumulator = (float)value;
		}

		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if((_type == IL_CTU) && (value != 0) && (_cu == 0)){
			_cu = 1;
			if(_accumulator >= 32767){
				_ov = 1;
				_accumulator = -32768;
				error_log("ERROR: counter overflow");
				_dn = 0;
			} else {
				_accumulator++;
				if(_accumulator >= _preset){
					_dn = 1;
				} else {
					_dn = 0;
				}
			}
		} else if((_type == IL_CTD) && (value != 0) &&(_cd==0)){
			_cd = 1;
			if(_accumulator <= -32768){
				_un = 1;
				_accumulator = 32767;
				error_log("ERROR: counter underflow");
				_dn = 0;
			} else {
				_accumulator--;
				if(_accumulator <= _preset){
					_dn = 1;
				} else {
					_dn = 0;
				}
			}
		} else if(value == 0) {
			_cu = 0;
			_cd = 0;
		}
		var1->var->set_bit(_CU, _cu);
		var1->var->set_bit(_CD, _cd);
		var1->var->set_bit(_DN, _dn);
		var1->var->set_bit(_OV, _ov);
		var1->var->set_bit(_UN, _un);
		var3->var->set_int(_accumulator);

		// mem->memory_set(_file, _word, _ACCUMULATOR, (int)_accumulator);
	} else if(_type == IL_RES){
//printf("RES\n");
		var1 = prog->get_operand(stack_exec[stack_pointer].prog_file); // status
		// error =prog->get_operand(stack_exec[stack_pointer].prog_file,&_file,&_word,&_bit);
		// mem->memory_info(_file, &_used, &type_temp, &_size);
		pull(&value);
		if(stack_exec[stack_pointer].ladder_flag == 1) push(value);
		if(value != 0){
			var1->var->set_int(0);
		}
	} else {
		error = ERROR;
		error_log("ERROR: instruction type not found");
	}

	return error;
}



float plc5::get_float_from_memory(){
	variable	*var1;

	var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);

	return var1->var->get_real();
}


int plc5::set_float_in_memory(float value){
	int	temp;
	variable	*var1;

	var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
	var1->var->set_real(value);

	return temp;
}
	


int plc5::push(int value){
	int	error;

	error = ERROR;
	if(stack.count < STACK_SIZE){
		stack.count++;
		stack.list[stack.count] = value;
		error = NO_ERROR;
	} else {
		error_log("too many arguments on stack");
	}

	return error;
}


int plc5::pull(int *value){
	int	error;

	error = ERROR;
	if(stack.count >=0){
		value[0] = stack.list[stack.count];
		stack.count--;
		error = NO_ERROR;
	} else {
		value[0] = 1;
		error_log("stack is empty");
	}
	return error;
}


void plc5::dump_all(){
	prog->dump_all();
	mem->dump_all();
}



int plc5::program_info(int file, int *used, int *type){
	int	error;

	error = NO_ERROR;
	if((type[0] = prog->program_type(file)) == ERROR){
		used[0] = UNUSED;
	} else {
		used[0] = USED;
	}

	return error;
}


int plc5::program_line(int prog_file, int line, char *text){
	int	error,
		j,
		_type;
	variable	*var1;

	error = NO_ERROR;
	focus_program = prog_file;
	error = prog->get_instruction(prog_file, line);
	if(error == NO_ERROR){
		prog->get_opcode(prog_file, &_type);
		for(j = 0; (comms[j].length > 0) && (comms[j].type != _type);j++){
		}
		if(comms[j].length > 0){
			strcpy(text, comms[j].label);

			prog->get_operand_type(prog_file, &_type);
			while(_type != _VALUE_UNDEFINED){
				strcat(text, " ");
				var1 = prog->get_operand(prog_file);
				if(var1->var->type == _LITERAL){
					strcat(text, var1->var->get_string());
				} else {
					strcat(text, var1->var->get_name());
				}
				prog->get_operand_type(prog_file, &_type);
			}

			// prog->get_operand_type(prog_file, &operand_type);
			// for(j = 0;((var1 = prog->get_operand(stack_exec[stack_pointer].prog_file)) != NULL) && (error != ERROR); j++){
				// var1 = prog->get_operand(stack_exec[stack_pointer].prog_file);
				// strcat(text, " ");
				// if(var1->var->get_name() != NULL){
				// 	strcat(text, var1->var->get_name());
				// prog->get_operand_type(prog_file,&operand_type);
				// } else {
				// 	strcat(text, var1->var->get_string());
				// }
			// }
		} else {
			error_log("ERROR: Opcode type not found");
			error = ERROR;
		}
	
	}

	return error;
}



