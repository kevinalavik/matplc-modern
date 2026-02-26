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
//    Last Modified: March 30, 2001
//


#include "store.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#ifndef __STEP_FOR_PROGRAM_HEADER
#define __STEP_FOR_PROGRAM_HEADER

#define		_STEP_UNDEFINED			7000
#define		_STEP_INSTRUCTION		7001
#define		_STEP_VARIABLE			7002
#define		_STEP_LABEL				7003


class function;


class step_t {
	public:
		int			type;

		int			opcode;
		function	*func;
		variable	*var;
		char		*label;
		char		*comment;

		step_t		*next;
		step_t		*child;
		step_t		*previous;
		step_t		*parent;

		step_t();
		~step_t();
	int	set_label(char *);
	int	set_comment(char *);
};



class program {
	public:
		step_t	*root;

				program();
				~program();

		step_t	*append_child(step_t*);
		step_t	*insert_child(step_t*);
		step_t	*append_next(step_t*);
		step_t	*insert_next(step_t*);
		int		delete_step(step_t*);
		step_t	*find(step_t*, char*, int);
		int		dump(step_t*);
		int		test();
};



#endif

