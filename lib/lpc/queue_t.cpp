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

/*
 * Queue (message) basic functions
 *
 * These function deal with basic process management.
 *
 * Last Revised: May 3, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "queue_t.h"
#include "global.h"





queue_t::queue_t(){
	first = NULL;
	last = NULL;
	last_out = NULL;
}



queue_t::~queue_t(){
	char	*temp;
	while(first != NULL){
		temp = pull();
	}
	if(last_out != NULL) delete last_out;
}



int queue_t::push(char *message){
	if(first == NULL){	// the list should be empty
		first = new queue_atom();
		last = first;
		first->message = new char[strlen(message) + 1];
		strcpy(first->message, message);
		first->next = NULL;
	} else {	// append onto the list
		queue_atom *temp = new queue_atom();
		temp->message = new char[strlen(message) + 1];
		strcpy(temp->message, message);
		temp->next = NULL;
		last->next = temp;
		last = temp;
	}

	return NO_ERROR;
}



char *queue_t::pull(){
	queue_atom	*temp;
	if(first != NULL){

		//printf("POPPED_X [%d]  [%d]\n", last_out, first->message);
		// printf("POPPED_A [%s]  [%s]\n", last_out, first->message);
		//
		// The line below is commented out to keep things working.
		// Something is broken that is causing the last out pointer to also point
		// to a currently alive message element. I assume there is some sort
		// of pointer rewritten. For now this fix will allow things to
		// work, but at the cost of a memory leak.
		//
		//if(last_out != NULL) delete last_out;
		last_out = first->message;
		//printf("POPPED_B [%s]  [%s]\n", last_out, first->message);
		if(last == first) last = NULL;
		temp = first;
		first = temp->next;
		delete temp;
		return last_out;
	} else {
		return NULL;
	}
}



void queue_t::dump(){
	queue_atom *focus;

	printf("   message queue dump;\n");
	for(focus = first; focus != NULL; focus = focus->next){
		printf("       message string [%s] %s\n", focus->message, focus->message);
	}
}
