
//
//    io.cpp - Lowlevel IO Routines
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


#include "io.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


io::io(){
	io_count = 0;

//	{
//		int	i;
//		for(i = 0; i < MAXIMUM_IO_LIST; i++){
//			io_list[i].type = _IO_UNDEFINED;
//			io_list[i].com = NULL;
//			io_list[i].net = NULL;
//			io_list[i].daq = NULL;
//			io_list[i].file = NULL;
//		}
//	}
}


io::~io(){
	int	i,
		error;
//for(i = 0; i < XXXXXX; i++){
//	if(io_list[i].type == _IO_NETWORK){
//		error = io_list[i].net->end_write_connection();
//		error = io_list[i].net->deinit_write();
//	}
//}
	for(i = 0; i < io_count; i++) error = remove(i);
}

int io::remove(int position){
	int	error;

	error = NO_ERROR;
/*
	if((position >= 0) && (position < io_count)){
		if(io_list[position].type == _IO_SERIAL){
			delete io_list[position].com;
		} else if(io_list[position].type == _IO_NETWORK){
			io_list[position].net->end_read_connection();
			io_list[position].net->deinit_read();
			delete io_list[position].net;
		} else if(io_list[position].type == _IO_DAS08){
			delete io_list[position].daq;
		} else if(io_list[position].type == _IO_STREAM){
			close(io_list[position].file);
		}
		io_list[position].type = _IO_UNDEFINED;
		if(position == (io_count-1)){
			io_count--;
		}
	} else {
		error_log("ERROR: IO resource outside range");
		error = ERROR;
	}
*/
	return error;
}

int io::add(int type, int arg1, int arg2, int arg3, char *arg4){
/*
	int error;

	if(type == _IO_SERIAL){
		io_list[io_count].type = type;
		io_list[io_count].com = new serial_io(arg4);
		io_count++;
	} else if(type == _IO_NETWORK){
		io_list[io_count].type = type;
		io_list[io_count].net = new network;
		if(arg1 == _IO_SYNC){
			error = io_list[io_count].net->set_local_host(arg2);
			if(error == NO_ERROR) error = io_list[io_count].net->init_read();
			if(error == ERROR){
				error_log("ERROR: could not initialize synchronous network connection");
			}
		} else {
			if(arg1 != _IO_ASYNC){
				error_log("WARNING: Network type must be synchronous or async - nonsync assumed");
			}
			error = io_list[io_count].net->set_local_host(arg2);
			if(error == NO_ERROR) error = io_list[io_count].net->init_read();
			if(error == ERROR){
				error_log("ERROR: could not initialize async network connection");
			}
		}
		io_count++;
	} else if(type == _IO_DAS08){
		io_list[io_count].type = type;
		io_list[io_count].daq = new das08(arg1, arg2, arg3);
		io_count++;
	} else if(type == _IO_STREAM){
		io_list[io_count].type = type;
		if((io_list[io_count].file = open(arg4, O_RDWR+O_CREAT+O_NONBLOCK)) == -1){
			error_log("ERROR: Could not open output stream");
printf(" : File name was <%s> \n", arg4);
		} else {
			io_count++;
		}
	} else {
		error_log("WARNING: IO type not recognized");
		io_list[io_count].type = type;
		io_count++;
	}
*/
	return io_count - 1;
}

int io::command(int position, int command_type, int arg1, int arg2, int arg3, char *arg4, double arg5, int *arg_int, char *arg_text, int arg_text_size, double *arg_float){
	int	error;

	error = ERROR;
/*
	if((position >= 0) && (position < io_count)){
		if(io_list[position].type == _IO_SERIAL){
			error = NO_ERROR;
			if((command_type == _IO_READ) || (command_type == _IO_CONNECTION_READ)){
				if(arg1 == _STRING){
					error = io_list[position].com->reader(arg_text, arg_text_size);
//printf("Getting serial input <%s> %d \n", arg_text, error);
					// if((error == NO_ERROR) && (strlen(arg_text) <= 0)) error = ERROR;
				} else {
					error_log("ERROR: can only read a string from serial IO");
				}
			} else if((command_type == _IO_WRITE) || (command_type == _IO_CONNECTION_WRITE)){
				if(arg1 == _STRING){
					error = io_list[position].com->writer(arg4);
				} else {
					error_log("ERROR: can only write string with serial IO");
				}
			} else if(command_type == _IO_WRITE_INIT){
			} else if(command_type == _IO_WRITE_END){
			} else if(command_type == _IO_READ_INIT){
			} else if(command_type == _IO_CONNECTION_WAIT){
			} else if(command_type == _IO_CONNECTION_END){
			} else if(command_type == _IO_IS_CONNECTED){
			} else { error_log("ERROR: Serial command not recognized");}

		} else if(io_list[position].type == _IO_NETWORK){
			if(command_type == _IO_READ){
				if(arg1 == _STRING){
					error = io_list[position].net->reader(arg_text, arg_text_size);
					if((error == NO_ERROR) && (strlen(arg_text) <= 0)) error = ERROR;
				} else {
					error_log("ERROR: Can only read strings with network IO");
				}
			} else if(command_type == _IO_WRITE){
				if(arg1 == _STRING){
					error = io_list[position].net->write_to_connection(arg_text);
				} else {
					error_log("ERROR: can only write strings with network IO");
				}
			} else if(command_type == _IO_WRITE_INIT){
				if((error = io_list[position].net->set_remote_host(arg_text, arg1)) == NO_ERROR){
					error = io_list[position].net->init_write();
				}
				if(error == NO_ERROR) error = io_list[position].net->open_write_connection();
			} else if(command_type == _IO_WRITE_END){
				error = io_list[position].net->write_stuff_done();
				if(error == NO_ERROR){
 					error = io_list[position].net->end_write_connection();
					error = io_list[position].net->deinit_write();
				}
			} else if(command_type == _IO_READ_INIT){
				error = io_list[position].net->set_local_host(arg1);
				if(error == NO_ERROR) error = io_list[position].net->init_read();
			} else if(command_type == _IO_CONNECTION_WAIT){
				error = io_list[position].net->wait_read_connection();
			} else if(command_type == _IO_CONNECTION_READ){
				if(arg1 == _STRING){
					error = io_list[position].net->read_from_connection(arg_text, arg_text_size);
				} else {
					error_log("ERROR: Can only read strings over network IO");
				}
			} else if(command_type == _IO_CONNECTION_WRITE){
				if(arg1 == _STRING){
					error = io_list[position].net->write_to_connection(arg4);
				} else {
					error_log("ERROR: Can only write strings over network IO");
				}
			} else if(command_type == _IO_CONNECTION_END){
				error = io_list[position].net->end_read_connection();
			} else if(command_type == _IO_IS_CONNECTED){
				error = io_list[position].net->check_connection();
			} else if(command_type == _IO_IS_WAITING){
				error = io_list[position].net->read_stuff_waiting();
			} else { error_log("ERROR: Network command not recognized");}

		} else if(io_list[position].type == _IO_DAS08){
			if(command_type == _IO_READ){
				error = NO_ERROR;
				if(arg2 == _D_PORT_A){
					io_list[position].daq->DConfigPort(PORTA, DIGITALIN);
					io_list[position].daq->DIn(PORTA, arg_int);
				} else if(arg2 == _D_PORT_B){
					io_list[position].daq->DConfigPort(PORTB, DIGITALIN);
					io_list[position].daq->DIn(PORTB, arg_int);
				} else if(arg2 == _D_PORT_C){
					io_list[position].daq->DConfigPort(PORTC, DIGITALIN);
					io_list[position].daq->DIn(PORTC, arg_int);
				} else if(arg2 == _D_PORT_D){
					io_list[position].daq->DIn(PORTAUX, arg_int);
					arg_int[0] = arg_int[0] / 16;
				} else if(arg2 == _A_IN_0){ io_list[position].daq->AIn(0, arg_int);
				} else if(arg2 == _A_IN_1){ io_list[position].daq->AIn(1, arg_int);
				} else if(arg2 == _A_IN_2){ io_list[position].daq->AIn(2, arg_int);
				} else if(arg2 == _A_IN_3){ io_list[position].daq->AIn(3, arg_int);
				} else if(arg2 == _A_IN_4){ io_list[position].daq->AIn(4, arg_int);
				} else if(arg2 == _A_IN_5){ io_list[position].daq->AIn(5, arg_int);
				} else if(arg2 == _A_IN_6){ io_list[position].daq->AIn(6, arg_int);
				} else if(arg2 == _A_IN_7){ io_list[position].daq->AIn(7, arg_int);
				} else { error_log("ERROR: That port is not available yet"); error = ERROR;}
			} else if(command_type == _IO_WRITE){ //error = io_list[position].com->writer(arg_text);
				error = NO_ERROR;
				if(arg3 == _D_PORT_A){
					io_list[position].daq->DConfigPort(PORTA, DIGITALOUT);
					io_list[position].daq->DOut(PORTA, arg2);
				} else if(arg3 == _D_PORT_B){
					io_list[position].daq->DConfigPort(PORTB, DIGITALOUT);
					io_list[position].daq->DOut(PORTB, arg2);
				} else if(arg3 == _D_PORT_C){
					io_list[position].daq->DConfigPort(PORTC, DIGITALOUT);
					io_list[position].daq->DOut(PORTC, arg2);
				} else if(arg3 == _D_PORT_D){ io_list[position].daq->DOut(PORTAUX, arg2);
				} else if(arg3 == _A_OUT_0){ io_list[position].daq->AOut(0, arg2);
				} else if(arg3 == _A_OUT_1){ io_list[position].daq->AOut(1, arg2);
				} else {error_log("ERROR: That port is not available yet"); error = ERROR;}
			} else if(command_type == _IO_WRITE_INIT){
			} else if(command_type == _IO_DECODE){
				error = NO_ERROR;
				if(strcmp(arg4, "A") == 0){arg_int[0] = _D_PORT_A;
				} else if(strcmp(arg4, "B") == 0){arg_int[0] = _D_PORT_B;
				} else if(strcmp(arg4, "C") == 0){arg_int[0] = _D_PORT_C;
				} else if(strcmp(arg4, "AUX") == 0){arg_int[0] = _D_PORT_D;
				} else if(strcmp(arg4, "AI0") == 0){arg_int[0] = _A_IN_0;
				} else if(strcmp(arg4, "AI1") == 0){arg_int[0] = _A_IN_1;
				} else if(strcmp(arg4, "AI2") == 0){arg_int[0] = _A_IN_2;
				} else if(strcmp(arg4, "AI3") == 0){arg_int[0] = _A_IN_3;
				} else if(strcmp(arg4, "AI4") == 0){arg_int[0] = _A_IN_4;
				} else if(strcmp(arg4, "AI5") == 0){arg_int[0] = _A_IN_5;
				} else if(strcmp(arg4, "AI6") == 0){arg_int[0] = _A_IN_6;
				} else if(strcmp(arg4, "AI7") == 0){arg_int[0] = _A_IN_7;
				} else if(strcmp(arg4, "AO0") == 0){arg_int[0] = _A_OUT_0;
				} else if(strcmp(arg4, "AO1") == 0){arg_int[0] = _A_OUT_1;
				} else { error_log("ERROR: Argument not recognized"); error = ERROR;}
			} else if(command_type == _IO_WRITE_END){
			} else if(command_type == _IO_READ_INIT){
			} else if(command_type == _IO_CONNECTION_WAIT){
			} else if(command_type == _IO_CONNECTION_READ){
			} else if(command_type == _IO_CONNECTION_END){
			} else { error_log("ERROR: Serial command not recognized");}

		} else if(io_list[position].type == _IO_STREAM){
			error = NO_ERROR;
			if(command_type == _IO_READ){
				read(io_list[position].file, arg_text, arg_text_size);
			} else if(command_type == _IO_WRITE){
printf("write 0   %d   <%s>  %d \n", io_list[position].file, arg4, strlen(arg4));
				error = write(io_list[position].file, arg4, strlen(arg4));
			} else if(command_type == _IO_WRITE_INIT){
			} else if(command_type == _IO_WRITE_END){
			} else if(command_type == _IO_READ_INIT){
			} else if(command_type == _IO_CONNECTION_WAIT){
			} else if(command_type == _IO_CONNECTION_READ){
				read(io_list[position].file, arg_text, arg_text_size);
				// add stuff in later to allow all kinds of streams here
			} else if(command_type == _IO_CONNECTION_WRITE){
printf("write 0   %d   <%s>  %d \n", io_list[position].file, arg4, strlen(arg4));
				error = write(io_list[position].file, arg4, strlen(arg4));
			} else if(command_type == _IO_CONNECTION_END){
			} else if(command_type == _IO_IS_CONNECTED){
			} else { error_log("ERROR: Stream command not recognized");}

		} else {
			error_log("ERROR: can't read from unrecognized IO type");
		}
	} else {
		error_log("ERROR: IO item out of list range");
	}
*/
	return error;
}




