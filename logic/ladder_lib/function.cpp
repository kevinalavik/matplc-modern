//
//    function.cpp - a definition of the functions in the logic core
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


#define	rung_state()	estack[stack_pointer].true_flag		// this will cut down some bulk



#include "function.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////
//
// Processor status bits
//
////////////////////////////////////////////////////////////////

processor_status::processor_status(){
	first_scan = mem->add(NULL, NULL, "first_scan", "LPC first scan", _BIT);
	error_routine = mem->add(NULL, NULL, "error_routine", "LPC error handler", _STRING); 
	error = mem->add(NULL, NULL, "error", "LPC error flags", _INTEGER); 
	core_flags = mem->add(NULL, NULL, "core_flags", "LPC core flags", _INTEGER); 
}


processor_status::~processor_status(){
	delete	first_scan;
	delete	error_routine;
	delete	error;
	delete	core_flags;
}


////////////////////////////////////////////////////////////////
//
// Add new functions to the list below
//
////////////////////////////////////////////////////////////////

function_library::function_library(){
	//list = new function[100];
	list_cnt = -1;

	// Basic bit oriented program functions
	list_cnt++; list[list_cnt] = new f_sor();
	list_cnt++; list[list_cnt] = new f_eor();
	list_cnt++; list[list_cnt] = new f_xic();
	list_cnt++; list[list_cnt] = new f_xio();
	list_cnt++; list[list_cnt] = new f_ote();
	list_cnt++; list[list_cnt] = new f_otl();
	list_cnt++; list[list_cnt] = new f_otu();
	list_cnt++; list[list_cnt] = new f_bit();
	list_cnt++; list[list_cnt] = new f_bst();
	list_cnt++; list[list_cnt] = new f_nxb();
	list_cnt++; list[list_cnt] = new f_bnd();
	list_cnt++; list[list_cnt] = new f_lbl();

	// Basic structure functions
	list_cnt++; list[list_cnt] = new f_end();

	// Basic data oriented functions
	list_cnt++; list[list_cnt] = new f_mov();
	list_cnt++; list[list_cnt] = new f_clr();

	// Boolean word oriented functions
	list_cnt++; list[list_cnt] = new f_and();
	list_cnt++; list[list_cnt] = new f_or();
	list_cnt++; list[list_cnt] = new f_xor();
	list_cnt++; list[list_cnt] = new f_not();

	list_cnt++; list[list_cnt] = new f_r_and();
	list_cnt++; list[list_cnt] = new f_r_or();
	list_cnt++; list[list_cnt] = new f_r_xor();
	list_cnt++; list[list_cnt] = new f_r_not();

	// Math oriented program functions
	list_cnt++; list[list_cnt] = new f_add();
	list_cnt++; list[list_cnt] = new f_mul();
	list_cnt++; list[list_cnt] = new f_sub();
	list_cnt++; list[list_cnt] = new f_div();
	list_cnt++; list[list_cnt] = new f_xpy();
	list_cnt++; list[list_cnt] = new f_neg();
	list_cnt++; list[list_cnt] = new f_sqr();
	list_cnt++; list[list_cnt] = new f_cos();
	list_cnt++; list[list_cnt] = new f_sin();
	list_cnt++; list[list_cnt] = new f_tan();
	list_cnt++; list[list_cnt] = new f_acs();
	list_cnt++; list[list_cnt] = new f_asn();
	list_cnt++; list[list_cnt] = new f_atn();
	list_cnt++; list[list_cnt] = new f_ln();
	list_cnt++; list[list_cnt] = new f_log();

	list_cnt++; list[list_cnt] = new f_r_add();
	list_cnt++; list[list_cnt] = new f_r_mul();
	list_cnt++; list[list_cnt] = new f_r_sub();
	list_cnt++; list[list_cnt] = new f_r_div();
	list_cnt++; list[list_cnt] = new f_r_xpy();
	list_cnt++; list[list_cnt] = new f_r_neg();
	list_cnt++; list[list_cnt] = new f_r_sqr();
	list_cnt++; list[list_cnt] = new f_r_cos();
	list_cnt++; list[list_cnt] = new f_r_sin();
	list_cnt++; list[list_cnt] = new f_r_tan();
	list_cnt++; list[list_cnt] = new f_r_acs();
	list_cnt++; list[list_cnt] = new f_r_asn();
	list_cnt++; list[list_cnt] = new f_r_atn();
	list_cnt++; list[list_cnt] = new f_r_ln();
	list_cnt++; list[list_cnt] = new f_r_log();

	// Comparison functions
	list_cnt++; list[list_cnt] = new f_eq();
	list_cnt++; list[list_cnt] = new f_gt();
	list_cnt++; list[list_cnt] = new f_lt();
	list_cnt++; list[list_cnt] = new f_geq();
	list_cnt++; list[list_cnt] = new f_leq();
	list_cnt++; list[list_cnt] = new f_ne();
	list_cnt++; list[list_cnt] = new f_lim();

	list_cnt++; list[list_cnt] = new f_r_eq();
	list_cnt++; list[list_cnt] = new f_r_gt();
	list_cnt++; list[list_cnt] = new f_r_lt();
	list_cnt++; list[list_cnt] = new f_r_geq();
	list_cnt++; list[list_cnt] = new f_r_leq();
	list_cnt++; list[list_cnt] = new f_r_ne();
	list_cnt++; list[list_cnt] = new f_r_lim();

	// Higher Level functions
	list_cnt++; list[list_cnt] = new f_msg();

	// Timer counter functions
	list_cnt++; list[list_cnt] = new f_ton();
	list_cnt++; list[list_cnt] = new f_rto();
	list_cnt++; list[list_cnt] = new f_tof();
	list_cnt++; list[list_cnt] = new f_res();


	/////////////////////////////////////////////////////////////////
	// warning: if the list is longer than 100, change the size of list[]
};


function_library::~function_library(){
	for(int i = 0; i < /* <= */ list_cnt; i++) delete list[i];
};


function *function_library::search(char *name){
	for(int i = 0; i <= list_cnt; i++){
		if(strcmp(name, list[i]->name()) == 0){
			// changes needed here //////////////////////////////
			// printf("------------Got %s at location %d\n", name, i);
			return list[i];
		}
	}

	// printf("------------Didn't find %s \n", name);
	return NULL;
}


////////////////////////////////////////////////////////////////
//
// The main function prototype
//
////////////////////////////////////////////////////////////////


char*	function::name(){return "Undefined";};
char*	function::description(){return "Undefined";};
function::function(){arg_cnt=0; return_type=_VOID;};
function::~function(){};
int	function::pre_step(){return NO_ERROR;};
int	function::step(){return NO_ERROR;};
function* function::clone(){return new function();};

////////////////////////////////////////////////////////////////
//
// Code for core PLC functions and single bit logic
//
////////////////////////////////////////////////////////////////


char* f_sor::name(){return "SOR";};
char* f_sor::description(){return "Start Of Rung";};
function* f_sor::clone(){return new f_sor();};
f_sor::f_sor(){arg_cnt=0;return_type=_VOID;};
f_sor::~f_sor(){};
int f_sor::step(){
	truth->push(TRUE);
	estack[stack_pointer].ladder_flag = 1;
	return NO_ERROR;
};


char* f_eor::name(){return "EOR";};
char* f_eor::description(){return "End Of Rung";};
function* f_eor::clone(){return new f_eor();};
f_eor::f_eor(){arg_cnt=0;return_type=_VOID;};
f_eor::~f_eor(){};
int f_eor::step(){
	truth->pop();
	estack[stack_pointer].ladder_flag = 0;
	return NO_ERROR;
};


char* f_xic::name(){return "XIC";};
char* f_xic::description(){return "eXamine If Closed (NO)";};
function* f_xic::clone(){return new f_xic();};
f_xic::f_xic(){arg_cnt=1;return_type=_VOID;};
f_xic::~f_xic(){};
int f_xic::step(){
	variable	*var1;
	var1 = heap->pop();
	truth->push(truth->pop() & var1->var->get_bit());
	return NO_ERROR;
};


char* f_xio::name(){return "XIO";};
char* f_xio::description(){return "eXamine If Open (NC)";};
function* f_xio::clone(){return new f_xio();};
f_xio::f_xio(){arg_cnt=1;return_type=_VOID;};
f_xio::~f_xio(){};
int f_xio::step(){
	variable	*var1;
	var1 = heap->pop();
	truth->push(truth->pop() & !var1->var->get_bit());
	return NO_ERROR;
};


char* f_ote::name(){return "OTE";};
char* f_ote::description(){return "Output Terminal Enable";};
function* f_ote::clone(){return new f_ote();};
f_ote::f_ote(){arg_cnt=1;return_type=_VOID;};
f_ote::~f_ote(){};
int f_ote::step(){
	variable	*var1;
	var1 = heap->pop();
	var1->var->set_bit(truth->top());
	return NO_ERROR;
};


char* f_otl::name(){return "OTL";};
char* f_otl::description(){return "OutpuT Latch - lost after power failure";};
function* f_otl::clone(){return new f_otl();};
f_otl::f_otl(){arg_cnt=1;return_type=_VOID;};
f_otl::~f_otl(){};
int f_otl::step(){
	variable	*var1;
	var1 = heap->pop();
	if(truth->top() != 0) var1->var->set_bit(TRUE);
	return NO_ERROR;
};


char* f_otu::name(){return "OTU";};
char* f_otu::description(){return "OutpuT Unlatch - lost after power failure";};
function* f_otu::clone(){return new f_otu();};
f_otu::f_otu(){arg_cnt=1;return_type=_VOID;};
f_otu::~f_otu(){};
int f_otu::step(){
	variable	*var1;
	var1 = heap->pop();
	if(truth->top() != 0) var1->var->set_bit(FALSE);
	return NO_ERROR;
};


char* f_bit::name(){return "BIT";};
char* f_bit::description(){return "get BIT address from integer";};
function* f_bit::clone(){return new f_bit();};
f_bit::f_bit(){arg_cnt=2;return_type=_VOID;};
f_bit::~f_bit(){};
int f_bit::step(){
	variable	*var1, *var2, *var3;
	int		bit_num;
	var2 = heap->pop();
	var1 = heap->pop(); // This will be slow for now - it is better to precompile bit addresses if possible
	if(var2->var->get_type() == _STRING){
		char	*temp;
		temp = var2->var->get_string();
		if(strcmp("DN", temp) == 0) bit_num = _DN;
		if(strcmp("EN", temp) == 0) bit_num = _EN;
		if(strcmp("TT", temp) == 0) bit_num = _TT;
	} else {
		bit_num = var2->var->get_int();
	}
	var3 = mem->find_bit(var1, bit_num);
	if(var3 == NULL){	// define a bit
		var3 = mem->add(var1, NULL, NULL, NULL, _BIT);
		// This is a somewhat circular definition, but it saves complexity elsewhere
		var3->var->set_bit(bit_num, var1->var->get_bit(bit_num));
	}
	heap->push(var3);
	return NO_ERROR;
};


char* f_bst::name(){return "BST";};
char* f_bst::description(){return "Branch STart";};
function* f_bst::clone(){return new f_bst();};
f_bst::f_bst(){arg_cnt=0;return_type=_VOID;};
f_bst::~f_bst(){};
int f_bst::step(){
	int	val_rung;
	val_rung = truth->pop();
	truth->push(val_rung);
	truth->push(FALSE);
	truth->push(val_rung);
	return NO_ERROR;
};


char* f_nxb::name(){return "NXB";};
char* f_nxb::description(){return "NeXt Branch";};
function* f_nxb::clone(){return new f_nxb();};
f_nxb::f_nxb(){arg_cnt=0;return_type=_VOID;};
f_nxb::~f_nxb(){};
int f_nxb::step(){
	int	val_rung, val_set, val_branch;
	val_branch = truth->pop();
	val_set = truth->pop();
	val_rung = truth->pop();
	if(val_branch != FALSE) val_set = TRUE;
	truth->push(val_rung);
	truth->push(val_set);
	truth->push(val_rung);
	return NO_ERROR;
};


char* f_bnd::name(){return "BND";};
char* f_bnd::description(){return "Branch eND";};
function* f_bnd::clone(){return new f_bnd();};
f_bnd::f_bnd(){arg_cnt=0;return_type=_VOID;};
f_bnd::~f_bnd(){};
int f_bnd::step(){
	int	val_rung, val_set, val_branch;
	val_branch = truth->pop();
	val_set = truth->pop();
	val_rung = truth->pop();
	if(val_branch != FALSE) val_set = TRUE;
	if((val_rung != FALSE) && (val_set != FALSE)){ val_rung = TRUE;
	} else { val_rung = FALSE;}
	truth->push(val_rung);
	return NO_ERROR;
};


char* f_lbl::name(){return "LBL";};
char* f_lbl::description(){return "LaBeL";};
function* f_lbl::clone(){return new f_lbl();};
f_lbl::f_lbl(){arg_cnt=0;return_type=_VOID;};
f_lbl::~f_lbl(){};
int f_lbl::step(){
	// This is just a place holder
	return NO_ERROR;
};


//////////////////////////////////////////////////////////////////
//
// Program structures
//
//////////////////////////////////////////////////////////////////

char* f_end::name(){return "END";};
char* f_end::description(){return "END of program";};
function* f_end::clone(){return new f_end();};
f_end::f_end(){arg_cnt=1;return_type=_VOID;};
f_end::~f_end(){};
int f_end::step(){estack[stack_pointer].end_flag = 1; return NO_ERROR;};



///////////////////////////////////////////////////////////////////
//
// Data Oriented Functions
//
///////////////////////////////////////////////////////////////////

char* f_mov::name(){return "MOV";};
char* f_mov::description(){return "MOVe data values";};
function* f_mov::clone(){return new f_mov();};
f_mov::f_mov(){arg_cnt=2;return_type=_VOID;};
f_mov::~f_mov(){};
int f_mov::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
//printf("MOV-A");
	if(truth->top() != FALSE){
//printf("MOV B \n");
		var1->var->set_real(var2->var->get_real());
	}
//printf("\n");

	return NO_ERROR;
};



char* f_clr::name(){return "CLR";};
char* f_clr::description(){return "CLeaR memory location";};
function* f_clr::clone(){return new f_clr();};
f_clr::f_clr(){arg_cnt=1;return_type=_VOID;};
f_clr::~f_clr(){};
int f_clr::step(){
	variable	*var1;
	var1 = heap->pop();
	if(truth->top() != FALSE)
		var1->var->set_real(0.0);
	return NO_ERROR;
};




////////////////////////////////////////////////////////////////////
//
// Boolean word manipulation functions WITHOUT return values
//
////////////////////////////////////////////////////////////////////


char* f_and::name(){return "AND";};
char* f_and::description(){return "AND binary numbers";};
function* f_and::clone(){return new f_and();};
f_and::f_and(){arg_cnt=3;return_type=_VOID;};
f_and::~f_and(){};
int f_and::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_int(var2->var->get_int() & var3->var->get_int());
	return NO_ERROR;
};

char* f_or::name(){return "OR";};
char* f_or::description(){return "OR binary numbers";};
function* f_or::clone(){return new f_or();};
f_or::f_or(){arg_cnt=3;return_type=_VOID;};
f_or::~f_or(){};
int f_or::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_int(var2->var->get_int() | var3->var->get_int());
	return NO_ERROR;
};

char* f_xor::name(){return "XOR";};
char* f_xor::description(){return "eXclusive OR binary numbers";};
function* f_xor::clone(){return new f_xor();};
f_xor::f_xor(){arg_cnt=3;return_type=_VOID;};
f_xor::~f_xor(){};
int f_xor::step(){
	int	val1, val2;
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	val1 = var2->var->get_int();
	val2 = var3->var->get_int();
	if(truth->top() != 0)
		var1->var->set_int((~val1 & val2) | (val1 & ~val2));
	return NO_ERROR;
};

char* f_not::name(){return "NOT";};
char* f_not::description(){return "NOT a binary number";};
function* f_not::clone(){return new f_not();};
f_not::f_not(){arg_cnt=2;return_type=_VOID;};
f_not::~f_not(){};
int f_not::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_int(~var2->var->get_int());
	return NO_ERROR;
};



////////////////////////////////////////////////////////////////////
//
// Boolean word manipulation functions WITH return values
//
////////////////////////////////////////////////////////////////////


char* f_r_and::name(){return "R_AND";};
char* f_r_and::description(){return "AND binary numbers";};
function* f_r_and::clone(){return new f_r_and();};
f_r_and::f_r_and(){var = mem->add(NULL,NULL,NULL,NULL,_INTEGER);arg_cnt=2;return_type=_INTEGER;};
f_r_and::~f_r_and(){};
int f_r_and::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	var->var->set_int(var1->var->get_int() & var2->var->get_int());
	heap->push(var);
	return NO_ERROR;
};

char* f_r_or::name(){return "R_OR";};
char* f_r_or::description(){return "OR binary numbers";};
function* f_r_or::clone(){return new f_r_or();};
f_r_or::f_r_or(){var = mem->add(NULL,NULL,NULL,NULL,_INTEGER);arg_cnt=2;return_type=_INTEGER;};
f_r_or::~f_r_or(){};
int f_r_or::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	var->var->set_int(var1->var->get_int() | var2->var->get_int());
	heap->push(var);
	return NO_ERROR;
};

char* f_r_xor::name(){return "R_XOR";};
char* f_r_xor::description(){return "eXclusive OR binary numbers";};
function* f_r_xor::clone(){return new f_r_xor();};
f_r_xor::f_r_xor(){var = mem->add(NULL,NULL,NULL,NULL,_INTEGER);arg_cnt=2;return_type=_INTEGER;};
f_r_xor::~f_r_xor(){};
int f_r_xor::step(){
	int	val1, val2;
	val1 = (heap->pop())->var->get_int();
	val2 = (heap->pop())->var->get_int();
	var->var->set_int((~val1 & val2) | (val1 & ~val2));
	heap->push(var);
	return NO_ERROR;
};

char* f_r_not::name(){return "R_NOT";};
char* f_r_not::description(){return "NOT a binary number";};
function* f_r_not::clone(){return new f_r_not();};
f_r_not::f_r_not(){var = mem->add(NULL,NULL,NULL,NULL,_INTEGER);arg_cnt=1;return_type=_INTEGER;};
f_r_not::~f_r_not(){};
int f_r_not::step(){
	variable	*var1;
	var1 = heap->pop();
	var->var->set_int(~var1->var->get_int());
	heap->push(var);
	return NO_ERROR;
};





//////////////////////////////////////////////////////////////////////////////
//
// Math Functions WITHOUT return values
//
//////////////////////////////////////////////////////////////////////////////


// First the two argument operations

char* f_add::name(){return "ADD";};
char* f_add::description(){return "ADD two numbers";};
function* f_add::clone(){return new f_add();};
f_add::f_add(){arg_cnt=3;return_type=_VOID;};
f_add::~f_add(){};
int f_add::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0){
		var1->var->set_real(var3->var->get_real() + var2->var->get_real());
	}
	return NO_ERROR;
};

char* f_sub::name(){return "SUB";};
char* f_sub::description(){return "SUBtract a number";};
function* f_sub::clone(){return new f_sub();};
f_sub::f_sub(){arg_cnt=3;return_type=_VOID;};
f_sub::~f_sub(){};
int f_sub::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(var3->var->get_real() - var2->var->get_real());

	return NO_ERROR;
};

char* f_mul::name(){return "MUL";};
char* f_mul::description(){return "MULtiply two numbers";};
function* f_mul::clone(){return new f_mul();};
f_mul::f_mul(){arg_cnt=3;return_type=_VOID;};
f_mul::~f_mul(){};
int f_mul::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(var3->var->get_real() * var2->var->get_real());
	return NO_ERROR;
};

char* f_div::name(){return "DIV";};
char* f_div::description(){return "DIVide";};
function* f_div::clone(){return new f_div();};
f_div::f_div(){arg_cnt=3;return_type=_VOID;};
f_div::~f_div(){};
int f_div::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0){
		if(var2->var->get_real() != 0.0){
			var1->var->set_real(var3->var->get_real() / var2->var->get_real());
		} else {
			var1->var->set_real(1e30);
			error_log(MINOR, "ERROR: Divide by zero error");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}
	return NO_ERROR;
};

char* f_xpy::name(){return "XPY";};
char* f_xpy::description(){return "DIVide";};
function* f_xpy::clone(){return new f_xpy();};
f_xpy::f_xpy(){arg_cnt=3;return_type=_VOID;};
f_xpy::~f_xpy(){};
int f_xpy::step(){
	variable	*var1, *var2, *var3;
	var1 = heap->pop();
	var2 = heap->pop();
	var3 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(pow(var3->var->get_real(), var2->var->get_real()));
	return NO_ERROR;
};

/////////////// Now do the single argument functions

char* f_neg::name(){return "NEG";};
char* f_neg::description(){return "NEGative of memory value";};
function* f_neg::clone(){return new f_neg();};
f_neg::f_neg(){arg_cnt=2;return_type=_VOID;};
f_neg::~f_neg(){};
int f_neg::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(-var2->var->get_real());
	return NO_ERROR;
};

char* f_sqr::name(){return "SQR";};
char* f_sqr::description(){return "SQuare Root";};
function* f_sqr::clone(){return new f_sqr();};
f_sqr::f_sqr(){arg_cnt=2;return_type=_VOID;};
f_sqr::~f_sqr(){};
int f_sqr::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		if(var1->var->get_real() >= 0.0){
			var1->var->set_real(sqrt(var2->var->get_real()));
		} else {
			error_log(MINOR, "ERROR: complex result");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}

	return NO_ERROR;
};

char* f_cos::name(){return "COS";};
char* f_cos::description(){return "COSine";};
function* f_cos::clone(){return new f_cos();};
f_cos::f_cos(){arg_cnt=2;return_type=_VOID;};
f_cos::~f_cos(){};
int f_cos::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(cos(var2->var->get_real()));
	return NO_ERROR;
};

char* f_sin::name(){return "SIN";};
char* f_sin::description(){return "SINe";};
function* f_sin::clone(){return new f_sin();};
f_sin::f_sin(){arg_cnt=2;return_type=_VOID;};
f_sin::~f_sin(){};
int f_sin::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(sin(var2->var->get_real()));
	return NO_ERROR;
};

char* f_tan::name(){return "TAN";};
char* f_tan::description(){return "TANgent";};
function* f_tan::clone(){return new f_tan();};
f_tan::f_tan(){arg_cnt=2;return_type=_VOID;};
f_tan::~f_tan(){};
int f_tan::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0)
		var1->var->set_real(tan(var2->var->get_real()));
	return NO_ERROR;
};

char* f_acs::name(){return "ACS";};
char* f_acs::description(){return "ArcCoSine - inverse cosine";};
function* f_acs::clone(){return new f_acs();};
f_acs::f_acs(){arg_cnt=2;return_type=_VOID;};
f_acs::~f_acs(){};
int f_acs::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		if(fabs(var2->var->get_real()) <= 1.0){
			var1->var->set_real(acos(var2->var->get_real()));
		} else {
			error_log(MINOR, "ERROR: ACS value outside +1 to -1 range");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}
	return NO_ERROR;
};

char* f_asn::name(){return "ASN";};
char* f_asn::description(){return "ArcCoSine - inverse cosine";};
function* f_asn::clone(){return new f_asn();};
f_asn::f_asn(){arg_cnt=2;return_type=_VOID;};
f_asn::~f_asn(){};
int f_asn::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		if(fabs(var2->var->get_real()) <= 1.0){
			var1->var->set_real(asin(var2->var->get_real()));
		} else {
			error_log(MINOR, "ERROR: ASN value outside +1 to -1 range");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}
	return NO_ERROR;
};

char* f_atn::name(){return "ATN";};
char* f_atn::description(){return "ArcCoSine - inverse cosine";};
function* f_atn::clone(){return new f_atn();};
f_atn::f_atn(){arg_cnt=2;return_type=_VOID;};
f_atn::~f_atn(){};
int f_atn::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		if(fabs(var2->var->get_real()) <= 1.0){
			var1->var->set_real(atan(var2->var->get_real()));
		} else {
			error_log(MINOR, "ERROR: ATN value outside +1 to -1 range");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}
	return NO_ERROR;
};

char* f_ln::name(){return "LN";};
char* f_ln::description(){return "Log Natural - base e";};
function* f_ln::clone(){return new f_ln();};
f_ln::f_ln(){arg_cnt=2;return_type=_VOID;};
f_ln::~f_ln(){};
int f_ln::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		if(fabs(var2->var->get_real()) > 0.0){
			var1->var->set_real(log(var2->var->get_real()));
		} else {
			error_log(MINOR, "ERROR: can't get a loge of a negative number");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}
	return NO_ERROR;
};

char* f_log::name(){return "LOG";};
char* f_log::description(){return "Log Natural - base e";};
function* f_log::clone(){return new f_log();};
f_log::f_log(){arg_cnt=2;return_type=_VOID;};
f_log::~f_log(){};
int f_log::step(){
	variable *var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		if(fabs(var2->var->get_real()) > 0.0){
			var1->var->set_real(log10(var2->var->get_real()));
		} else {
			error_log(MINOR, "ERROR: can't get a log10 of a negative number");
			stat->error->var->set_bit(ERROR_MATH, 1);
		}
	}
	return NO_ERROR;
};






//////////////////////////////////////////////////////////////////////////////
//
// Math Functions WITH return values
//
//////////////////////////////////////////////////////////////////////////////


// First the two argument operations

char* f_r_add::name(){return "R_ADD";};
char* f_r_add::description(){return "ADD two numbers";};
function* f_r_add::clone(){return new f_r_add();};
f_r_add::f_r_add(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=3;return_type=_REAL;};
f_r_add::~f_r_add(){};
int f_r_add::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	var->var->set_real(var2->var->get_real() + var1->var->get_real());
	heap->push(var);
	return NO_ERROR;
};

char* f_r_sub::name(){return "R_SUB";};
char* f_r_sub::description(){return "SUBtract a number";};
function* f_r_sub::clone(){return new f_r_sub();};
f_r_sub::f_r_sub(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=2;return_type=_REAL;};
f_r_sub::~f_r_sub(){};
int f_r_sub::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	var->var->set_real(var2->var->get_real() - var1->var->get_real());
	heap->push(var);
	return NO_ERROR;
};

char* f_r_mul::name(){return "R_MUL";};
char* f_r_mul::description(){return "MULtiply two numbers";};
function* f_r_mul::clone(){return new f_r_mul();};
f_r_mul::f_r_mul(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=2;return_type=_REAL;};
f_r_mul::~f_r_mul(){};
int f_r_mul::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	var->var->set_real(var2->var->get_real() * var1->var->get_real());
	heap->push(var);
	return NO_ERROR;
};

char* f_r_div::name(){return "R_DIV";};
char* f_r_div::description(){return "DIVide";};
function* f_r_div::clone(){return new f_r_div();};
f_r_div::f_r_div(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=2;return_type=_REAL;};
f_r_div::~f_r_div(){};
int f_r_div::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(var1->var->get_real() != 0.0){
		var->var->set_real(var2->var->get_real() / var1->var->get_real());
		heap->push(var);
	} else {
		var->var->set_real(1e30);
		error_log(MINOR, "ERROR: Divide by zero error");
			stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_xpy::name(){return "R_XPY";};
char* f_r_xpy::description(){return "DIVide";};
function* f_r_xpy::clone(){return new f_r_xpy();};
f_r_xpy::f_r_xpy(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=2;return_type=_REAL;};
f_r_xpy::~f_r_xpy(){};
int f_r_xpy::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	var->var->set_real(pow(var2->var->get_real(), var1->var->get_real()));
	heap->push(var);
	return NO_ERROR;
};

/////////////// Now do the single argument functions

char* f_r_neg::name(){return "R_NEG";};
char* f_r_neg::description(){return "NEGative of memory value";};
function* f_r_neg::clone(){return new f_r_neg();};
f_r_neg::f_r_neg(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_neg::~f_r_neg(){};
int f_r_neg::step(){
	variable *var1;
	var1 = heap->pop();
	var->var->set_real(-var1->var->get_real());
	heap->push(var);
	return NO_ERROR;
};

char* f_r_sqr::name(){return "R_SQR";};
char* f_r_sqr::description(){return "SQuare Root";};
function* f_r_sqr::clone(){return new f_r_sqr();};
f_r_sqr::f_r_sqr(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_sqr::~f_r_sqr(){};
int f_r_sqr::step(){
	variable *var1;
	var1 = heap->pop();
	if(var1->var->get_real() >= 0.0){
		var->var->set_real(sqrt(var1->var->get_real()));
	} else {
		error_log(MINOR, "ERROR: complex result");
		stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);

	return NO_ERROR;
};

char* f_r_cos::name(){return "R_COS";};
char* f_r_cos::description(){return "COSine";};
function* f_r_cos::clone(){return new f_r_cos();};
f_r_cos::f_r_cos(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_cos::~f_r_cos(){};
int f_r_cos::step(){
	var->var->set_real(cos((heap->pop())->var->get_real()));
	heap->push(var);
	return NO_ERROR;
};

char* f_r_sin::name(){return "R_SIN";};
char* f_r_sin::description(){return "SINe";};
function* f_r_sin::clone(){return new f_r_sin();};
f_r_sin::f_r_sin(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_sin::~f_r_sin(){};
int f_r_sin::step(){
	var->var->set_real(sin((heap->pop())->var->get_real()));
	heap->push(var);
	return NO_ERROR;
};

char* f_r_tan::name(){return "R_TAN";};
char* f_r_tan::description(){return "TANgent";};
function* f_r_tan::clone(){return new f_r_tan();};
f_r_tan::f_r_tan(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_tan::~f_r_tan(){};
int f_r_tan::step(){
	var->var->set_real(tan((heap->pop())->var->get_real()));
	heap->push(var);
	return NO_ERROR;
};

char* f_r_acs::name(){return "R_ACS";};
char* f_r_acs::description(){return "ArcCoSine - inverse cosine";};
function* f_r_acs::clone(){return new f_r_acs();};
f_r_acs::f_r_acs(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_acs::~f_r_acs(){};
int f_r_acs::step(){
	variable	*var1;
	var1 = heap->pop();
	if(fabs(var1->var->get_real()) <= 1.0){
		var->var->set_real(acos(var1->var->get_real()));
	} else {
		error_log(MINOR, "ERROR: ACS value outside +1 to -1 range");
			stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_asn::name(){return "R_ASN";};
char* f_r_asn::description(){return "ArcCoSine - inverse cosine";};
function* f_r_asn::clone(){return new f_r_asn();};
f_r_asn::f_r_asn(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_asn::~f_r_asn(){};
int f_r_asn::step(){
	variable	*var1;
	var1 = heap->pop();
	if(fabs(var1->var->get_real()) <= 1.0){
		var->var->set_real(asin(var1->var->get_real()));
	} else {
		error_log(MINOR, "ERROR: ASN value outside +1 to -1 range");
			stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_atn::name(){return "R_ATN";};
char* f_r_atn::description(){return "ArcCoSine - inverse cosine";};
function* f_r_atn::clone(){return new f_r_atn();};
f_r_atn::f_r_atn(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_atn::~f_r_atn(){};
int f_r_atn::step(){
	variable	*var1;
	var1 = heap->pop();
	if(fabs(var1->var->get_real()) <= 1.0){
		var->var->set_real(atan(var1->var->get_real()));
	} else {
		error_log(MINOR, "ERROR: ATN value outside +1 to -1 range");
			stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_ln::name(){return "R_LN";};
char* f_r_ln::description(){return "Log Natural - base e";};
function* f_r_ln::clone(){return new f_r_ln();};
f_r_ln::f_r_ln(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_ln::~f_r_ln(){};
int f_r_ln::step(){
	variable	*var1;
	var1 = heap->pop();
	if(fabs(var1->var->get_real()) > 0.0){
		var->var->set_real(log(var1->var->get_real()));
	} else {
		error_log(MINOR, "ERROR: can't get a loge of a negative number");
			stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_log::name(){return "R_LOG";};
char* f_r_log::description(){return "Log Natural - base e";};
function* f_r_log::clone(){return new f_r_log();};
f_r_log::f_r_log(){var=mem->add(NULL,NULL,NULL,NULL,_REAL);arg_cnt=1;return_type=_REAL;};
f_r_log::~f_r_log(){};
int f_r_log::step(){
	variable	*var1;
	var1 = heap->pop();
	if(fabs(var1->var->get_real()) > 0.0){
		var->var->set_real(log10(var1->var->get_real()));
	} else {
		error_log(MINOR, "ERROR: can't get a log10 of a negative number");
			stat->error->var->set_bit(ERROR_MATH, 1);
	}
	heap->push(var);
	return NO_ERROR;
};




/////////////////////////////////////////////////////////////////////
//
// Comparison Functions WITHOUT return values
//
/////////////////////////////////////////////////////////////////////

char* f_eq::name(){return "EQ";};
char* f_eq::description(){return "compare EQual";};
function* f_eq::clone(){return new f_eq();};
f_eq::f_eq(){arg_cnt=2;return_type=_VOID;};
f_eq::~f_eq(){};
int f_eq::step(){
	int	val;
	val = truth->pop();
	if(val != FALSE){
		if((heap->pop())->var->get_real() == (heap->pop())->var->get_real()){ val = TRUE;
		} else {val = FALSE;}
	}
	truth->push(val);
	return NO_ERROR;
};

char* f_lt::name(){return "LT";};
char* f_lt::description(){return "compare Less Than";};
function* f_lt::clone(){return new f_lt();};
f_lt::f_lt(){arg_cnt=2;return_type=_VOID;};
f_lt::~f_lt(){};
int f_lt::step(){
	int	val;
	val = truth->pop();
	if(val != FALSE){
		if((heap->pop())->var->get_real() > (heap->pop())->var->get_real()){ val = TRUE;
		} else {val = FALSE;}
	}
	truth->push(val);
	return NO_ERROR;
};

char* f_gt::name(){return "GT";};
char* f_gt::description(){return "compare EQual";};
function* f_gt::clone(){return new f_gt();};
f_gt::f_gt(){arg_cnt=2;return_type=_VOID;};
f_gt::~f_gt(){};
int f_gt::step(){
	int	val;
	val = truth->pop();
	if(val != FALSE){
		if((heap->pop())->var->get_real() < (heap->pop())->var->get_real()){ val = TRUE;
		} else {val = FALSE;}
	}
	truth->push(val);
	return NO_ERROR;
};

char* f_leq::name(){return "LEQ";};
char* f_leq::description(){return "compare Less than or EQual";};
function* f_leq::clone(){return new f_leq();};
f_leq::f_leq(){arg_cnt=2;return_type=_VOID;};
f_leq::~f_leq(){};
int f_leq::step(){
	int	val;
	val = truth->pop();
	if(val != FALSE){
		if((heap->pop())->var->get_real() >= (heap->pop())->var->get_real()){val = TRUE;
		} else {val = FALSE;}
	}
	truth->push(val);
	return NO_ERROR;
};

char* f_geq::name(){return "GEQ";};
char* f_geq::description(){return "compare Greater than or EQual";};
function* f_geq::clone(){return new f_geq();};
f_geq::f_geq(){arg_cnt=2;return_type=_VOID;};
f_geq::~f_geq(){};
int f_geq::step(){
	int	val;
	val = truth->pop();
	if(val != FALSE){
		if((heap->pop())->var->get_real() <= (heap->pop())->var->get_real()){val = TRUE;
		} else {val = FALSE;}
	}
	truth->push(val);
	return NO_ERROR;
};

char* f_ne::name(){return "NE";};
char* f_ne::description(){return "compare Not Equal";};
function* f_ne::clone(){return new f_ne();};
f_ne::f_ne(){arg_cnt=2;return_type=_VOID;};
f_ne::~f_ne(){};
int f_ne::step(){
	int	val;
	val = truth->pop();
	if(val != FALSE){
		if((heap->pop())->var->get_real() != (heap->pop())->var->get_real()){val = TRUE;
		} else {val = FALSE;}
	}
	truth->push(val);
	return NO_ERROR;
};

char* f_lim::name(){return "LIM";};
char* f_lim::description(){return "compare LIMits";};
function* f_lim::clone(){return new f_lim();};
f_lim::f_lim(){arg_cnt=3;return_type=_VOID;};
f_lim::~f_lim(){};
int f_lim::step(){
	double	val1, val2, val3;
	int val;
	val = truth->pop();
	val1 = heap->pop()->var->get_real();
	val2 = heap->pop()->var->get_real();
	val3 = heap->pop()->var->get_real();
	if(val != FALSE){
		if(val3 <= val1){
			if((val2 >= val3) && (val2 <= val1)){ val = TRUE;
			} else { val = FALSE;}
		} else {
			if((val2 < val3) && (val2 > val1)){ val = TRUE;
			} else { val = FALSE;}
		}
	}
	truth->push(val);
	return NO_ERROR;
};





/////////////////////////////////////////////////////////////////////
//
// Comparison Functions WITH return values
//
/////////////////////////////////////////////////////////////////////

char* f_r_eq::name(){return "R_EQ";};
char* f_r_eq::description(){return "compare EQual";};
function* f_r_eq::clone(){return new f_r_eq();};
f_r_eq::f_r_eq(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=2;return_type=_BIT;};
f_r_eq::~f_r_eq(){};
int f_r_eq::step(){
	if((heap->pop())->var->get_real() == (heap->pop())->var->get_real()){
		var->var->set_bit(1);
	} else {var->var->set_bit(0);}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_lt::name(){return "R_LT";};
char* f_r_lt::description(){return "compare Less Than";};
function* f_r_lt::clone(){return new f_r_lt();};
f_r_lt::f_r_lt(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=2;return_type=_BIT;};
f_r_lt::~f_r_lt(){};
int f_r_lt::step(){
	if((heap->pop())->var->get_real() > (heap->pop())->var->get_real()){
		var->var->set_bit(1);
	} else {var->var->set_bit(0);}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_gt::name(){return "R_GT";};
char* f_r_gt::description(){return "compare EQual";};
function* f_r_gt::clone(){return new f_r_gt();};
f_r_gt::f_r_gt(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=2;return_type=_BIT;};
f_r_gt::~f_r_gt(){};
int f_r_gt::step(){
	if((heap->pop())->var->get_real() < (heap->pop())->var->get_real()){
		var->var->set_bit(1);
	} else {var->var->set_bit(0);}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_leq::name(){return "R_LEQ";};
char* f_r_leq::description(){return "compare Less than or EQual";};
function* f_r_leq::clone(){return new f_r_leq();};
f_r_leq::f_r_leq(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=2;return_type=_BIT;};
f_r_leq::~f_r_leq(){};
int f_r_leq::step(){
	if((heap->pop())->var->get_real() >= (heap->pop())->var->get_real()){
		var->var->set_bit(1);
	} else {var->var->set_bit(0);}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_geq::name(){return "R_GEQ";};
char* f_r_geq::description(){return "compare Greater than or EQual";};
function* f_r_geq::clone(){return new f_r_geq();};
f_r_geq::f_r_geq(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=2;return_type=_BIT;};
f_r_geq::~f_r_geq(){};
int f_r_geq::step(){
	if((heap->pop())->var->get_real() <= (heap->pop())->var->get_real()){
		var->var->set_bit(1);
	} else {var->var->set_bit(0);}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_ne::name(){return "R_NE";};
char* f_r_ne::description(){return "compare Not Equal";};
function* f_r_ne::clone(){return new f_r_ne();};
f_r_ne::f_r_ne(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=2;return_type=_BIT;};
f_r_ne::~f_r_ne(){};
int f_r_ne::step(){
	if((heap->pop())->var->get_real() != (heap->pop())->var->get_real()){
		var->var->set_bit(1);
	} else {var->var->set_bit(0);}
	heap->push(var);
	return NO_ERROR;
};

char* f_r_lim::name(){return "R_LIM";};
char* f_r_lim::description(){return "compare LIMits";};
function* f_r_lim::clone(){return new f_r_lim();};
f_r_lim::f_r_lim(){var=mem->add(NULL,NULL,NULL,NULL,_BIT);arg_cnt=3;return_type=_BIT;};
f_r_lim::~f_r_lim(){};
int f_r_lim::step(){
	double	val1, val2, val3;
	val1 = heap->pop()->var->get_real();
	val2 = heap->pop()->var->get_real();
	val3 = heap->pop()->var->get_real();
	if(val3 <= val1){
		if((val2 >= val3) && (val2 <= val1)){ var->var->set_bit(1);
		} else { var->var->set_bit(0);}
	} else {
		if((val2 < val3) && (val2 > val1)){ var->var->set_bit(1);
		} else { var->var->set_bit(0);}
	}
	heap->push(var);
	return NO_ERROR;
};




/////////////////////////////////////////////////////////////////
//
// Higher level functions
//
/////////////////////////////////////////////////////////////////


char* f_msg::name(){return "MSG";};
char* f_msg::description(){return "MeSsaGe send";};
function* f_msg::clone(){return new f_msg();};
f_msg::f_msg(){arg_cnt=3;return_type=_BIT;};
f_msg::~f_msg(){};
int f_msg::step(){
	variable *var1, *var2, *var3;
//	if(estack[stack_pointer].ladder_flag == 1) push(int_val);
	var1 = heap->pop(); // message source
	var2 = heap->pop(); // status
	var3 = heap->pop(); // slot
//	_en = var2->var->get_bit(_EN);
//	_dn = var2->var->get_bit(_DN);
//	_er = var2->var->get_bit(_ER);

	return NO_ERROR;
};


//	} else if(_type == IL_MSG){
//printf("MSG\n");
//		pull(&int_val);
//		if(estack[stack_pointer].ladder_flag == 1) push(int_val);
//		var1 = vstack->pop(); // message source
//		var2 = vstack->pop(); // status
//		var3 = vstack->pop(); // slot
//		_en = var2->var->get_bit(_EN);
//		_dn = var2->var->get_bit(_DN);
//		_er = var2->var->get_bit(_ER);

//		if(first_scan == TRUE){
//			_en = 0; _dn = 0; _er = 0;
//		}
//		if((int_val != 0) && (_en == 0)){
//			// error = mem->memory_get(_file, _word, text);
//			number = communications->add(var1->var->get_string());
//printf("Message is [%s] \n", var1->var->get_string());
//			_en = 1; _dn = 0; _er = 0;
//			communications->status(number, _WAITING);
//			var3->var->set_int(number);
//			// error = mem->memory_set(_file3, _word3, _POSITION, number);
//		} else if((_en != 0) && (_dn == 0)){
//			slot = var3->var->get_int();
//			// error = mem->memory_get(_file3, _word3, _POSITION, &slot);
//			state = communications->status(slot);
//			if(state == _DONE){
//				_dn = 1;
//				communications->release(slot);
//			} else if(state == _ERROR){
//				_dn = 1; _er = 1;
//				communications->release(slot);
//			}
//		}
//		if(_dn == 1) _en = int_val;
//		var2->var->set_bit(_EN, _en);
//		var2->var->set_bit(_DN, _dn);
//		var2->var->set_bit(_ER, _er);





/////////////////////////////////////////////////////////////////
//
// Timer Counter functions
//
/////////////////////////////////////////////////////////////////



char* f_ton::name(){return "TON";};
char* f_ton::description(){return "Timer ON delay";};
function* f_ton::clone(){return new f_ton();};
f_ton::f_ton(){arg_cnt=3;return_type=_VOID;};
f_ton::~f_ton(){};
int f_ton::step(){
	variable	*var1, *var2, *var3;
	int		_en, _dn, _tt;
	double		_preset, _accumulator;

	var3 = heap->pop(); // accumulator
	var2 = heap->pop(); // delay
	var1 = heap->pop(); // status

	_preset = var2->var->get_real();
	_accumulator = var3->var->get_real();
	_en = var1->var->get_bit(_EN);
	_dn = var1->var->get_bit(_DN);
	_tt = var1->var->get_bit(_TT);

//printf("TON-B  %f   %f   %d   %d   %d   %d \n", _preset, _accumulator, _en, _dn, _tt, truth->top());
	if(truth->top() != FALSE){
		_en = 1;
		_accumulator += stat->seconds; //  / _time_base;
		// (int)(milliseconds/(1000.0 * _time_base))-
		// (int)(milliseconds_last/(1000.0 * _time_base));
		//_accumulator += time_diff;
		if(_accumulator >= _preset){
//if(_dn != 1) printf("DONE------- %f ----\n", _accumulator);
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
		_accumulator = 0;
		_dn = 0;
	}
//printf("TON-A  %f   %f   %d   %d   %d   %d \n", _preset, _accumulator, _en, _dn, _tt, truth->top());

	var1->var->set_bit(_EN, _en);
	var1->var->set_bit(_DN, _dn);
	var1->var->set_bit(_TT, _tt);
	var3->var->set_real(_accumulator);

	return NO_ERROR;
};




char* f_rto::name(){return "RTO";};
char* f_rto::description(){return "Timer ON delay";};
function* f_rto::clone(){return new f_rto();};
f_rto::f_rto(){arg_cnt=3;return_type=_VOID;};
f_rto::~f_rto(){};
int f_rto::step(){
	variable	*var1, *var2, *var3;
	int		_en, _dn, _tt;
	double		_preset, _accumulator;

	var3 = heap->pop(); // accumulator
	var2 = heap->pop(); // delay
	var1 = heap->pop(); // status

	_preset = var2->var->get_real();
	_accumulator = var3->var->get_real();
	_en = var1->var->get_bit(_EN);
	_dn = var1->var->get_bit(_DN);
	_tt = var1->var->get_bit(_TT);

	if(truth->top() != FALSE){
		_en = 1;
		_accumulator += stat->seconds; //  / _time_base;
		// (int)(milliseconds/(1000.0 * _time_base))-
		// (int)(milliseconds_last/(1000.0 * _time_base));
		//_accumulator += time_diff;
		if(_accumulator >= _preset){
//if(_dn != 1) printf("DONE------- %f ----\n", _accumulator);
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
	}

	var1->var->set_bit(_EN, _en);
	var1->var->set_bit(_DN, _dn);
	var1->var->set_bit(_TT, _tt);
	var3->var->set_real(_accumulator);

	return NO_ERROR;
};



char* f_tof::name(){return "TOF";};
char* f_tof::description(){return "Timer OFf delay";};
function* f_tof::clone(){return new f_ton();};
f_tof::f_tof(){arg_cnt=3;return_type=_VOID;};
f_tof::~f_tof(){};
int f_tof::step(){
	variable	*var1, *var2, *var3;
	int		_en, _dn, _tt;
	double		_preset, _accumulator;

	var3 = heap->pop(); // accumulator
	var2 = heap->pop(); // delay
	var1 = heap->pop(); // status

	_preset = var2->var->get_real();
	_accumulator = var3->var->get_real();
	_en = var1->var->get_bit(_EN);
	_dn = var1->var->get_bit(_DN);
	_tt = var1->var->get_bit(_TT);

//printf("TON-B  %f   %f   %d   %d   %d   %d \n", _preset, _accumulator, _en, _dn, _tt, truth->top());
	if(truth->top() != FALSE){
		_tt = 0;
		_en = 1;
		_accumulator = 0;
		_dn = 1;
	} else {
		_en = 0;
		if(_dn == 1){
			_accumulator += stat->seconds; // (int)(seconds / _time_base);
			// time_diff = (int)(milliseconds/(1000.0*_time_base))-(int)(milliseconds_last/(1000.0*_time_base));
			// _accumulator += time_diff;
			if(_accumulator >= _preset){
				_accumulator = 0;
				_tt = 0;
				_dn = 0;
			}
//printf("TON-A  %f   %f   %d   %d   %d   %d \n", _preset, _accumulator, _en, _dn, _tt, truth->top());
		}
	}

	var1->var->set_bit(_EN, _en);
	var1->var->set_bit(_DN, _dn);
	var1->var->set_bit(_TT, _tt);
	var3->var->set_real(_accumulator);

	return NO_ERROR;
};


char* f_res::name(){return "RES";};
char* f_res::description(){return "RESet timer or counter";};
function* f_res::clone(){return new f_res();};
f_res::f_res(){arg_cnt=2;return_type=_VOID;};
f_res::~f_res(){};
int f_res::step(){
	variable	*var1, *var2;
	var1 = heap->pop();
	var2 = heap->pop();
	if(truth->top() != 0){
		var2->var->set_bit(_DN, 0);
		var2->var->set_bit(_TT, 0);
		// var1->var->set_bit(_EN, 0);
		var1->var->set_real(0.0);
	}
	return NO_ERROR;
};





#ifdef __cplusplus
}
#endif






