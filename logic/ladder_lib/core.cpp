
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
// Last Modified: March 30, 2001
//

#include "core.h"


truth_stack	*truth;		// keeps a stack of truth values for logic functions
variable_stack	*heap;		// for keeping variables
execution_stack	*estack;	// for keeping execution history
int		stack_pointer;
memory		*mem;
program		*prog;
function_library	*library;
processor_status	*stat;



