
//
//    step.cpp - a memory integration class
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
//    Last Modified: April 3, 2001
//


#include "step.h"
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif


step_t::step_t(){
	label = NULL;
	comment = NULL;
	next = NULL;
	previous = NULL;
	child = NULL;
	parent = NULL;
	var = NULL;
	func = NULL;
	type = _STEP_UNDEFINED;
}

step_t::~step_t(){
	if(label != NULL) delete label;
	if(comment != NULL) delete comment;
	if(var != NULL) delete var;
//	if(func != NULL) delete func; // don't forget to delete these at a higher level
}

int step_t::set_label(char *_label){
	int	error;

	error = NO_ERROR;
	if(label != NULL) delete label;
	label = new char[strlen(_label)+1];
	strcpy(label, _label);

	return error;
}

int step_t::set_comment(char *_comment){
	int	error;

	error = NO_ERROR;
	if(comment != NULL) delete comment;
	comment = new char[strlen(_comment)+1];
	strcpy(comment, _comment);

	return error;
}







program::program(){
	root = new step_t();
	root->set_label("root");
	root->set_comment("a global root node");
}

program::~program(){
	delete_step(root);
}

step_t *program::append_child(step_t *parent){
	step_t	*temp;

	if(parent->child == NULL){
		temp = new step_t();
		temp->parent = parent;
		parent->child = temp;
	} else {
		temp = parent->child;
		for(temp = parent->child; temp->next != NULL; temp = temp->next){}
		temp->next = new step_t();
		temp->next->previous = temp;
		temp->next->parent = parent;
		temp = temp->next;
	}

	return temp;
}

step_t *program::insert_child(step_t *parent){
	step_t	* temp;

	if(parent->child == NULL){
		temp = new step_t();
		parent->child = temp;
		temp->parent = parent;
	} else {
		temp = new step_t();
		temp->parent = parent;
		temp->next = parent->child;
		temp->next->previous = temp;
		parent->child = temp;
	}

	return temp;
}

step_t *program::append_next(step_t *current){
	step_t	*temp;

	if(current->next == NULL){
		temp = new step_t();
		current->next = temp;
		temp->previous = current;
		temp->parent = current->parent;
	} else {
		for(temp = current; temp->next != NULL; temp = temp->next){}
		temp->next = new step_t();
		temp->next->previous = temp;
		temp->next->parent = temp->parent;
		temp = temp->next;
	}

	return temp;
}

step_t *program::insert_next(step_t *current){
	step_t	*temp;

	if(current->previous == NULL){
		temp = new step_t();
		temp->next = current;
		temp->parent = current->parent;
		current->previous = temp;
	} else {
		temp = new step_t();
		temp->next = current;
		temp->previous = current->previous;
		temp->parent = current->parent;
		temp->next->previous = temp;
		temp->previous->next = temp;
	}

	return temp;
}


int program::delete_step(step_t *current){
	int	error;

	error = NO_ERROR;
	for(; current->child != NULL;){delete_step(current->child);}
	if(current->previous == NULL){
		if(current->next != NULL){
			current->next->previous = NULL;
			current->parent->child = current->next;
		} else {
			if(current->parent != NULL) current->parent->child = NULL;
		}
	} else {
		if(current->next == NULL){
			current->previous->next = NULL;
		} else {
			current->previous->next = current->next;
			current->next->previous = current->previous;
		}
	}
	// delete current;

	return error;
}


step_t *program::find(step_t *start, char *label, int depth){
	int	error;
	step_t	*stack[20];
	int	stack_pnt;

	error = NO_ERROR;
	stack_pnt = 0;
	if(start == NULL) start = root;
	stack[stack_pnt] = start;
	while(stack_pnt >= 0){
		if(stack[stack_pnt] == NULL){
			stack_pnt--;
		} else {
			if(stack[stack_pnt]->label != NULL){
				if(strcmp(stack[stack_pnt]->label, label) == 0){
					return stack[stack_pnt];
				}
			}

			if(stack[stack_pnt]->child == NULL){
				stack[stack_pnt] = stack[stack_pnt]->next;
			} else {
				stack[stack_pnt+1] = stack[stack_pnt]->child;
				stack[stack_pnt] = stack[stack_pnt]->next;
				if((depth == 0) || (stack_pnt < depth)) stack_pnt++;
			}
		}
	}

	return NULL;
}


int program::dump(step_t *start){
	int	error;
	step_t	*stack[20];
	int	stack_pnt;
	int	i;

	error = NO_ERROR;
	stack_pnt = 0;
	stack[stack_pnt] = start;
	while(stack_pnt >= 0){
		// printf("beep %d   %d  \n", stack_pnt, stack[stack_pnt]);
		if(stack[stack_pnt] == NULL){
			stack_pnt--;
		} else {
			for(i = 0; i < stack_pnt; i++) printf("  ");
			if(stack[stack_pnt]->type == _STEP_UNDEFINED) 	printf("Undefined:   ");
			if(stack[stack_pnt]->type == _STEP_INSTRUCTION) printf("Instruction: ");
			if(stack[stack_pnt]->type == _STEP_VARIABLE){
			 	printf("Variable:    [ ");
				if(stack[stack_pnt]->var->var->get_symbol() != NULL)
					printf("symbol=%s ", stack[stack_pnt]->var->var->get_symbol());
				if(stack[stack_pnt]->var->var->get_name() != NULL)
					printf("name=%s ", stack[stack_pnt]->var->var->get_name());
				if(stack[stack_pnt]->var->var->get_comment() != NULL)
					printf("comment=%s ", stack[stack_pnt]->var->var->get_comment());
				printf("] ");
			}
			if(stack[stack_pnt]->type == _STEP_LABEL) 		printf("Label:       ");
			if(stack[stack_pnt]->label != NULL) printf("label=[%s] ", stack[stack_pnt]->label);
			if(stack[stack_pnt]->comment != NULL) printf("comment=[%s] ", stack[stack_pnt]->comment);
			printf("\n");

			if(stack[stack_pnt]->child == NULL){
				stack[stack_pnt] = stack[stack_pnt]->next;
			} else {
				stack[stack_pnt+1] = stack[stack_pnt]->child;
				stack[stack_pnt] = stack[stack_pnt]->next;
				stack_pnt++;
			}
		}
	}

	return error;
}


int test_main(){
	program		*A;
	step_t		*step1, *step2, *step3;

	printf("-------- 0 ----------\n");
	A = new program();
	step1 = A->append_child(A->root); step1->type = _STEP_INSTRUCTION; step1->set_label("A"); step1->set_comment("a");
		step2 = A->append_child(step1); step2->type = _STEP_INSTRUCTION; step2->set_label("AA"); step2->set_comment("aa");
		step2 = A->append_next(step2); step2->type = _STEP_INSTRUCTION; step2->set_label("AB"); step2->set_comment("ab");
	A->dump(A->root);

	printf("-------- 1 ----------\n");
		step2 = A->append_next(step2); step2->type = _STEP_INSTRUCTION; step2->set_label("AC"); step2->set_comment("ac");
			step3 = A->append_child(step2); step3->type = _STEP_INSTRUCTION; step3->set_label("ACA"); step3->set_comment("aca");
			step3 = A->append_next(step3); step3->type = _STEP_INSTRUCTION; step3->set_label("ACB"); step3->set_comment("acb");
	A->dump(A->root);

	printf("-------- 2 ----------\n");
	step1 = A->append_child(A->root); step1->type = _STEP_INSTRUCTION; step1->set_label("B"); step1->set_comment("b");
		step2 = A->append_child(step1); step2->type = _STEP_INSTRUCTION; step2->set_label("BA"); step2->set_comment("ba");
		step2 = A->append_next(step2); step2->type = _STEP_INSTRUCTION; step2->set_label("BB"); step2->set_comment("bb");
	A->dump(A->root);
	printf("-------- 3 ----------\n");

	A->dump(A->find(A->root, "AC", 3));
	printf("-------- 4 ----------\n");

	A->delete_step(A->find(A->root, "AC", 3));
	printf("-------- 5 ----------\n");

	A->dump(A->root);
	printf("-------- 6 ----------\n");

	return (0);
}





#ifdef __cplusplus
}
#endif











