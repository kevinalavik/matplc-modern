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
// Last Modified: April 10, 2001
//


#include "store.h"
////#include "../include/variables.h"

#ifndef __IO_LIST_HEADER
#define __IO_LIST_HEADER


	class io_point	{
		public:
			#define	IO_INPUT			1
			#define	IO_OUTPUT			2
			#define	IO_INPUTOUTPUT		3	// bits 1 and 2 set
			int			type;
			variable	*var;
			io_point	*next;
			io_point	*prev;
			char		*remote_name;
			plc_pt_t	var_remote;
			// global_variable		*var_remote;
	};


	class io_list {
		public:
			io_point	*first;
			io_point	*scan_focus;
			int			scan_type;

						io_list();
						~io_list();
			int			remove(io_point*);
			int			add(int, variable*, char*);
			int			scan_first(int);
			io_point	*next();
			int			dump();
	};

#endif
