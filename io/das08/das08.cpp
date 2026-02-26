/////////////////////////////////////////////////////////////////////////////////
//
// A test program for interfacing with the serial port
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
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
// Last Modified: April 23, 2002
//


#include <unistd.h>
#include <stdio.h>
#include <string.h>


#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


#include "das08_io.h"



das08		*board;


plc_pt_t	data_portA_IN;	// hooks to global values
plc_pt_t	data_portA_OUT;
plc_pt_t	data_portB_IN;
plc_pt_t	data_portB_OUT;
plc_pt_t	data_portCL_IN;
plc_pt_t	data_portCL_OUT;
plc_pt_t	data_portCH_IN;
plc_pt_t	data_portCH_OUT;
plc_pt_t	data_portXI;
plc_pt_t	data_portXO;
plc_pt_t	data_AI0;
plc_pt_t	data_AI1;
plc_pt_t	data_AI2;
plc_pt_t	data_AI3;
plc_pt_t	data_AI4;
plc_pt_t	data_AI5;
plc_pt_t	data_AI6;
plc_pt_t	data_AI7;
plc_pt_t	data_AO0;
plc_pt_t	data_AO1;




plc_pt_t get_point(char *mod_name, char *name){
	char		*temp;
	plc_pt_t	pt;

	temp = new char[strlen(mod_name) + strlen(name) + 1];
	sprintf(temp, "%s%s", mod_name, name);
	pt = plc_pt_by_name(temp);
	delete temp;

	return pt;
}



int	init(){
	char	*temp;

	// create an instance of the controller board
	board = new das08();

	temp = get_argument(0, "CONFIG_FILE");
	if(temp != NULL){
		if(board->configure(temp) == ERROR) return ERROR;
	} else {
		if(board->configure("das08.conf") == ERROR) return ERROR;
	}

	//
	// get the variable names
	//
	temp = get_argument(0, "MODULE_NAME");
	if(temp != NULL){
		data_portA_IN = get_point(temp, "portA_IN");
		if(!data_portA_IN.valid){error_log(MAJOR, "portA_IN not defined"); return ERROR;}
		data_portA_OUT = get_point(temp, "portA_OUT");
		if(!data_portA_OUT.valid){error_log(MAJOR, "portA_OUT not defined"); return ERROR;}
		data_portB_IN = get_point(temp, "portB_IN");
		if(!data_portB_IN.valid){error_log(MAJOR, "portB_IN not defined"); return ERROR;}
		data_portB_OUT = get_point(temp, "portB_OUT");
		if(!data_portB_OUT.valid){error_log(MAJOR, "portB_OUT not defined"); return ERROR;}
		data_portCL_IN = get_point(temp, "portCL_IN");
		if(!data_portCL_IN.valid){error_log(MAJOR, "portCL_IN not defined"); return ERROR;}
		data_portCL_OUT = get_point(temp, "portCL_OUT");
		if(!data_portCL_OUT.valid){error_log(MAJOR, "portCL_OUT not defined"); return ERROR;}
		data_portCH_IN = get_point(temp, "portCH_IN");
		if(!data_portCH_IN.valid){error_log(MAJOR, "portCH_IN not defined"); return ERROR;}
		data_portCH_OUT = get_point(temp, "portCH_OUT");
		if(!data_portCH_OUT.valid){error_log(MAJOR, "portCH_OUT not defined"); return ERROR;}
		data_portXI = get_point(temp, "portXI");
		if(!data_portXI.valid){error_log(MAJOR, "portXI not defined"); return ERROR;}
		data_portXO = get_point(temp, "portXO");
		if(!data_portXO.valid){error_log(MAJOR, "portX0 not defined"); return ERROR;}
		data_AI0 = get_point(temp, "AI0");
		if(!data_AI0.valid){error_log(MAJOR, "portAI0 not defined"); return ERROR;}
		data_AI1 = get_point(temp, "AI1");
		if(!data_AI1.valid){error_log(MAJOR, "portAI1 not defined"); return ERROR;}
		data_AI2 = get_point(temp, "AI2");
		if(!data_AI2.valid){error_log(MAJOR, "portAI2 not defined"); return ERROR;}
		data_AI3 = get_point(temp, "AI3");
		if(!data_AI3.valid){error_log(MAJOR, "portAI3 not defined"); return ERROR;}
		data_AI4 = get_point(temp, "AI4");
		if(!data_AI4.valid){error_log(MAJOR, "portAI4 not defined"); return ERROR;}
		data_AI5 = get_point(temp, "AI5");
		if(!data_AI5.valid){error_log(MAJOR, "portAI5 not defined"); return ERROR;}
		data_AI6 = get_point(temp, "AI6");
		if(!data_AI6.valid){error_log(MAJOR, "portAI6 not defined"); return ERROR;}
		data_AI7 = get_point(temp, "AI7");
		if(!data_AI7.valid){error_log(MAJOR, "portAI7 not defined"); return ERROR;}
		data_AO0 = get_point(temp, "AO0");
		if(!data_AO0.valid){error_log(MAJOR, "portAO0 not defined"); return ERROR;}
		data_AO1 = get_point(temp, "AO1");
		if(!data_AO1.valid){error_log(MAJOR, "portAO1 not defined"); return ERROR;}
	} else {
		error_log(MAJOR, "The DAS08 module name was not defined");
		return ERROR;
	}

	return NO_ERROR;
}



int	deinit(){
	board->disconnect();
	if(board != NULL) delete board;
	return NO_ERROR;
}



int step_run(){

	// update outputs
	board->data_portA_OUT = plc_get(data_portA_OUT);
	board->data_portB_OUT = plc_get(data_portB_OUT);
	board->data_portCL_OUT = plc_get(data_portCL_OUT);
	board->data_portCH_OUT = plc_get(data_portCH_OUT);
	board->data_portXO = plc_get(data_portXO);
	board->data_AO0 = plc_get(data_AO0);
	board->data_AO1 = plc_get(data_AO1);

	board->scan();

	// update inputs
	plc_set(data_portA_IN, board->data_portA_IN);
	plc_set(data_portB_IN, board->data_portB_IN);
	plc_set(data_portCL_IN, board->data_portCL_IN);
	plc_set(data_portCH_IN, board->data_portCH_IN);
	plc_set(data_portXI, board->data_portXI);
	plc_set(data_AI0, board->data_AI0);
	plc_set(data_AI1, board->data_AI1);
	plc_set(data_AI2, board->data_AI2);
	plc_set(data_AI3, board->data_AI3);
	plc_set(data_AI4, board->data_AI4);
	plc_set(data_AI5, board->data_AI5);
	plc_set(data_AI6, board->data_AI6);
	plc_set(data_AI7, board->data_AI7);

	return NO_ERROR;
}

#ifdef __cplusplus
}
#endif
