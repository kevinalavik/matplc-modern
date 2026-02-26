//
//    functions.cpp - the functions for the controller
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




#ifndef LADDER_HEADER
#define LADDER_HEADER


#include "core.h"
#include "io_list.h"


	class ladder {
		public:
			core	*cpu;
			io_list	*io;
					ladder();
					~ladder();
			int		load(char *);
			int		parse_program_line(char*);
			int		init_program_line(step_t*);
			int		dump();
	};


#endif

