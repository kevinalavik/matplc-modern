//
//    serial.cpp - This is a driver for serial communication
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
// Last Modified: March 8, 2001
//

#include <stdio.h>   
#include <stdlib.h>
#include <string.h>  
#include <unistd.h>  
#include <fcntl.h>   
#include <errno.h>   
#include <termios.h> 
#include <ctype.h>

#include "rs232c_io.h"


// port name - probably "/dev/ttyS0" - com1
// baud rate - "9600"
// parity - "N" - NOT IMPLEMENTED
// data bits - "8"
// top bits - "1" - NOT IMPLEMENTED

char	*param_file_name;
int	param_baud;
int	param_parity;	// not implemented yet
int	param_size;
int	param_flow;		// not implemented yet

int	connected;



serial_io::serial_io(){
	param_file_name = NULL;
	param_baud = B9600;	// set defaults
	param_size = CS8;
	fd = -1;;
}


serial_io::~serial_io(void)
{
	if(fd != -1) close(fd);
	fd = -1;
	if(param_file_name != NULL) delete param_file_name;
}


int serial_io::set_param(int param_type, int param_val_int, char *param_val_char){
	int	error;

	error = NO_ERROR;
	if(param_type == BAUD_RATE){
		if(param_val_int == 9600) param_baud = B9600;
		if(param_val_int == 2400) param_baud = B2400;
		if(param_val_int == 1200) param_baud = B1200;
	} else if(param_type == DATA_BITS){
		if(param_val_int == 8) param_size = CS8;
		if(param_val_int == 7) param_size = CS7;
	} else if(param_type == PORT_FILE){
		if(param_file_name != NULL) delete param_file_name;
		param_file_name = new char[strlen(param_val_char) + 1];
		strcpy(param_file_name, param_val_char);
	} else {
		error_log(MINOR, "Serial port parameter not recognized");
		error = ERROR;
	}

	return error;
}




int serial_io::connect(){
	struct	termios
		options;
//	int	i;

//	char	temp[200];
//	int	len, last, cnt;

	if(fd != -1){
		close(fd);
		fd = -1;
	}
	if(param_file_name != NULL){
		if((fd = open(param_file_name /*args[0] port "/dev/ttyS0"*/, O_RDWR | O_NOCTTY | O_NDELAY)) < 0){
			error_log(MAJOR, "Unable to open serial port file device");
			error_log(MAJOR, param_file_name);
			fd = -1;
		} else {
			fcntl(fd, F_SETFL, FNDELAY);   /* Configure port reading */
			tcgetattr(fd, &options);	   /* Get the current options for the port */

			cfsetispeed(&options, param_baud);  /* Set the baud rates to 9600 */
			cfsetospeed(&options, param_baud);

			// O_RDWR -------> Reading and writing.
			// O_NOCTTY -----> Program will not be the controlling entity.
			// O_NDELAY -----> DCD line ignored.
			// Baud ---------> 9600
			// Data Bits ----> 8
			// Parity -------> None
			// Flow Control -> None
			options.c_cflag |= (CLOCAL | CREAD);// enable receiver and set local mode
			options.c_cflag &= ~PARENB;		// Mask the character size to 8 bits, no parity
			options.c_cflag &= ~CSTOPB;

			options.c_cflag &= ~CSIZE;
			// set data size
			options.c_cflag |=  param_size;		// set number of data bits

//			options.c_cflag &= ~CRTSCTS;	// Disable hardware flow control
// 			options.c_lflag &= ~(ICANON | ECHO | ISIG);// process as raw input
			options.c_oflag |= OPOST;

			// Update settings
			tcsetattr(fd, TCSANOW, &options);
		}
	} else {
		fd = -1;
	}

	if(fd == -1) return ERROR;
	return NO_ERROR;
}





int serial_io::reader(char *text, int max){
	int	char_read,
		error,
		i, j;

	error = ERROR;
	if(fd >= 0){
		char_read = read(fd, text, max-1);
//printf("reader <%s> %d = %d \n", text, max, char_read);
		if (char_read > 0){
			text[char_read] = 0;
			error = NO_ERROR;
			for(i = 0; i < char_read;){
				if((text[i] == 10 /*CR*/) || (text[i] == '\n')){
					for(j = i+1; j <= char_read; j++){
						text[j-1] = text[j];
					}
					char_read--;
				} else if(text[i] == '\t'){
					text[i] = ' ';
					i++;
				} else {
					i++;
				}
			}
		} else {
			text[0] = 0;
		}
	} else {
		error_log(MAJOR, "Serial port is not initialized");
	}

	return error;
}


int serial_io::writer(char *text)
{
	int	error,
		length = 0,
		count = 0;

	error = NO_ERROR;
	if(fd >= 0){
		length = strlen(text);
// printf("%s, %d \n",out_string, length);
		for(count = 0; count < length; count++){
			write(fd, &(text[count]), 1);
		}
	} else {
		error_log(MAJOR, "Serial port not initialized");
		error = ERROR;
	}

	return error;
}

