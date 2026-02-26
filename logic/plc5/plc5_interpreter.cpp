//
//    embedded.cpp - This is program for embedded applications.
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
// Changes to connect to matplc made by Mario de Sousa
//


#include <unistd.h>  // reuired for sleep()
#include <plc.h>
#include "controller.h"


#define PLC5_FILE_NAME "load_file"

void execute(controller *);


int main(int argc, char **argv){
	controller	*A;
	char *plc_file_name;

	if (plc_init("plc5", argc, argv) < 0) {
          printf("Error initializing the plc.\n");
          return EXIT_FAILURE;
        }

 	if ((plc_file_name = conffile_get_value(PLC5_FILE_NAME)) == NULL){
	  plc_log_errmsg(1, "Could not determine which file to load...");
          return EXIT_FAILURE;
        }

	A = new controller;
	A->init();
	A->load_plc_file(plc_file_name);
	execute(A);
	A->save_plc_file(plc_file_name, _SAVE_ALL, NULL);
	A->shutdown();
	delete A;

        plc_done();

	return EXIT_SUCCESS;
}



void execute(controller *A){
//	int	i;

	for(; A->status != _QUITTING;){
		plc_scan_beg();
                plc_update();
		A->scan();
                plc_update();
		plc_scan_end();
	}
}

