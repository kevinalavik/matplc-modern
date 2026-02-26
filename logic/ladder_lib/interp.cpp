/////////////////////////////////////////////////////////////////////////////////
//
// A main client for the ladder logic engine
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
//
//    Copyright (C) 2000-2 by Hugh Jack <jackh@gvsu.edu>
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

//#include "../include/process.h"


//////////////////////////////////////////////////////////////////////////////////
//
// Functions specific to this program follow. i.e. you write them
//
// note:	Write the step_function() with care, it should run
//		once in a fraction of a second.
//
// required functions:
//		init() - called when the program is first loaded
//		deinit() - called before the program is unloaded
//		step() - called each scan - it should run once - fast!
//



#include <plc.h>
#include <module_library.h>

#include "ladder.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <string.h>



//char	text[100];
//char	arg_val[10][20];
//int	arg_cnt;
//int	arg_case;

//int	*state;

//char	*sender;
//char	*receive;


//unsigned char *tmp;

char	file_name[40];
char	main_name[40];
ladder	*plc;
int	state;

#define		PLC_RUN		600
#define		PLC_HALT	601
#define		PLC_STEP	602		// not used yet, but good for later when debugging

int	init(){
	int	error;

	error = NO_ERROR;
	//if(args[0] != NULL){
	//	strcpy(file_name, args[0]);
	//	if(args[1] != NULL){
	//		strcpy(main_name, args[1]);
	//	} else {
	//		strcpy(main_name, "main");
	//	}
	//} else {
		strcpy(file_name, "default.plc");
		strcpy(main_name, "main");
	//}

	plc = new ladder();
	if((error = plc->load(file_name)) == ERROR){
		printf("COULD NOT OPEN FILE %s \n", file_name);
	}
	state = PLC_RUN;

	return error;
}




int	deinit(){
	return NO_ERROR;
	delete plc;
}


int	io_scan(int type){
//
// Later forces can be added into this routine - but this is a low priority
//
//
	int	error;

	error = NO_ERROR;
	if(type == IO_INPUT){
		io_point	*pnt;
		plc->io->scan_first(IO_INPUT);
		for(pnt = plc->io->next(); pnt != NULL; pnt = plc->io->next()){
			if(!pnt->var_remote.valid){
				//printf("LADDER: getting input variable: %s\n", pnt->remote_name);
// not allowed by mat		create_global_variable(pnt->remote_name, pnt->var->var->get_type(), _VOID);
				pnt->var_remote = plc_pt_by_name(pnt->remote_name);
				if(pnt->var->var->get_type() == _INTEGER){
					plc_set(pnt->var_remote, pnt->var->var->get_int());
				} else if(pnt->var->var->get_type() == _BIT){
					plc_set(pnt->var_remote, pnt->var->var->get_bit());
				} else if(pnt->var->var->get_type() == _REAL){
					plc_set_f32(pnt->var_remote, pnt->var->var->get_real());
				} else {
					error_log(MAJOR, "Input data type not recognized");
				}
			}
// mat only allows one owner if(pnt->type == IO_INPUTOUTPUT){
//				while(pnt->var_remote->lock == LOCKED){
//				};
//				pnt->var_remote->lock == LOCKED;
//			}
			if(pnt->var->var->get_type() == _INTEGER){
				//printf("LADDER: getting integer %d\n", plc_get(pnt->var_remote));
				pnt->var->var->set_int(plc_get(pnt->var_remote));
			} else if(pnt->var->var->get_type() == _BIT){
				//printf("LADDER: getting bit %d\n", plc_get(pnt->var_remote));
				pnt->var->var->set_bit(plc_get(pnt->var_remote));
			} else if(pnt->var->var->get_type() == _REAL){
				//printf("LADDER: getting float %f\n", plc_get_f32(pnt->var_remote));
				pnt->var->var->set_real(plc_get_f32(pnt->var_remote));
			} else {
				error_log(MAJOR, "Input data type not known");
			}
		}
	} else if(type == IO_OUTPUT){
		io_point	*pnt;
		plc->io->scan_first(IO_OUTPUT);
		for(pnt = plc->io->next(); pnt != NULL; pnt = plc->io->next()){
			if(!pnt->var_remote.valid){
				//printf("LADDER: getting output variable: %s\n", pnt->remote_name);
//				create_global_variable(pnt->remote_name, pnt->var->var->get_type(), _VOID);
				pnt->var_remote = plc_pt_by_name(pnt->remote_name);
				if(pnt->var->var->get_type() == _INTEGER){
					plc_set(pnt->var_remote, pnt->var->var->get_int());
				} else if(pnt->var->var->get_type() == _BIT){
					plc_set(pnt->var_remote, pnt->var->var->get_bit());
				} else if(pnt->var->var->get_type() == _REAL){
					plc_set_f32(pnt->var_remote, pnt->var->var->get_real());
				} else {
					error_log(MAJOR, "Ouptut data type not recognized");
				}
			}

			if(pnt->var->var->get_type() == _INTEGER){
				plc_set(pnt->var_remote, pnt->var->var->get_int());
			} else if(pnt->var->var->get_type() == _BIT){
				plc_set(pnt->var_remote, pnt->var->var->get_bit());
			} else if(pnt->var->var->get_type() == _REAL){
				plc_set_f32(pnt->var_remote, pnt->var->var->get_real());
			} else {
				error_log(MAJOR, "Output data type not known");
			}
//			if(pnt->type == IO_INPUTOUTPUT){
//				pnt->var_remote->lock == UNLOCKED;
//			}
		}
	}

	return error;
}



int	message_receive(char* message){
	int	error;

	error = NO_ERROR;
	if(strncmp(message, "LOAD ", 5) == 0){
		error = plc->load(&(message[5]));
	} else if(strncmp(message, "HALT", 4) == 0){
		state = PLC_HALT;
	} else if(strncmp(message, "RUN", 3) == 0){
		state = PLC_RUN;
	} else if(strncmp(message, "RESTART", 5) == 0){
		delete plc;
		plc = new ladder();
		state = PLC_HALT;
	} else {
		error_log(MINOR, "Message not recognized");
	}

	return error;
}


int	step_run(){
	int	error;

	if(state == PLC_RUN){
		if((error = io_scan(IO_INPUT)) == NO_ERROR){
			if((error = plc->cpu->scan(main_name)) == NO_ERROR){
				if((error = io_scan(IO_OUTPUT)) == NO_ERROR){
					// Do nothing, it worked
//sleep(1);
				} else {
					error_log(MINOR, "Could not scan outputs");
				}
			} else {
				error_log(MINOR, "Ladder logic scan error");
			}
		
		} else {
			error_log(MINOR, "Could not scan inputs");
		}
	}
	
	// catch any incoming messages
//	if(this_process->message != NULL){
//		this_process->message_lock = TRUE;
//		interpret_message((char*)this_process->message->message);
//		parent->messages->release(this_process->message);
//	}

	return error;
}


#ifdef __cplusplus
}
#endif


