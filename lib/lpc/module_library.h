/*
 * (c) 2002 Hugh Jack
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

//
// This header is primarily intended for dynamically linked
// library components. It defines the basic functions that
// should be present.
//

#ifndef MODULE_LIBRARY_HEADER

#define	MODULE_VERSION_NUMBER	100	// increment this number with each version upgrade
#define MODULE_VERSION_DATE		"April 10, 2002" // the last date of any major revisions

#include <string.h>
#include "global.h"
#include "../plc.h"

#ifndef HANDLER_T_HEADER	// only allow these to be defined for client processes

#ifdef __cplusplus
extern "C" {
#endif

//
// These functions are all optional to be defined in the library
//
int init();	/* This routine sets up the client when first loaded */
int deinit();		// the routine prepares the process to stop
int run_step();
int idle_step();
int message_receive(char *); // a user define message handler (not implemented yet)
char *argument_descriptions(int);


//
// some local variable definitions
//
char *__module__name;

// I am breaking some conventions by including function code here,
// but it will eliminate the need for linking to distant object files


// version tracking - use for compatibility with later versions
int __module_version(){return MODULE_VERSION_NUMBER;}



//
// stuff for sending messages out of the module
//
int (*__message_function_pointer)(char*);
void __set_message_pointer(int(*_func)(char*)){__message_function_pointer = _func;};
int send_message(char *text){
	if(__message_function_pointer != NULL){
		return (*__message_function_pointer)(text);
	} else {
		printf("WARNING: message not sent, null function pointer\n");
		return 1;
	}
};



//
// stuff for getting argument values
//
char *(*__argument_function_pointer)(int, char*);
void __set_argument_pointer(char*(*_func)(int, char*)){__argument_function_pointer = _func;};
char *get_argument(int number, char *text){
	char	*temp;
	int	pos;

	if(__argument_function_pointer != NULL){
		if(number > 0){
			return (*__argument_function_pointer)(number, text);
		} else if(number < 0){
			temp = conffile_var_by_index(-1 - number);
			if(temp == NULL) return NULL;
			if(strncmp(temp, __module__name, strlen(__module__name)) == 0){
				return &(temp[strlen(__module__name) + 1]);
			} else {
				return (char*) -1;
			}
		} else if(text != NULL){
			temp = (*__argument_function_pointer)(number, text);
			if(temp == NULL){
				for(pos = 0; ; pos++){
					if(text[pos] == 0){
						pos = 0;
						break;
					}
					if(text[pos] == ':'){
						pos++;
						break;
					}
				}
				temp = conffile_get_value(&(text[pos]));
				if(temp == NULL){
					error_log(MINOR, "variable not found in local or global lists");
				}
			}
			return temp;
		} else {
			error_log(MINOR, "Improperly formed argument request");
			return NULL;
		}
	} else {
		printf("WARNING: Argument not requested, null function pointer\n");
		return NULL;
	}
};


//
// a place to catch the flow of control during initialization
//
int __library__init(){
	__module__name = get_argument(0, "MODULE_NAME");
	return init();
}

//
// The error function determines how errors are logged
//
void error_log(int code, char* text){
	//plc_log_trcmsg(code, text); /* This is the MATPLC message handler */
	if(__module__name != NULL){
		printf("MODULE=%s: ", __module__name);
	}
	printf(" ERROR:  [%d]   [%s]\n", code, text); /* this is quick and dirty */
}


#define ERROR		1
#define NO_ERROR	0

#ifdef __cplusplus
}
#endif

#endif

#define MODULE_LIBRARY_HEADER
#endif
