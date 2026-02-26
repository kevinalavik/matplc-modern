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
// Last Modified: October 12, 2000
//


#ifndef __PROGRAM
#define __PROGRAM

	#include "program_il.h"

	#define	MAXIMUM_PROGRAM_LOCATIONS_EVER	1000

	#define		_IL_PROGRAM	100
	#define		_ST_PROGRAM	101
	#define		_SFC_PROGRAM	102
	#define		_FBD_PROGRAM	103

	class program_list{
		public:
			int			type;
			int			used;
			program_il	*_il;
	};

	class program{
		protected:
			program_list	*list;
			int				maximum_size;

		public:
						program(int);
						~program();

			int			program_add(int, int);
			int			program_delete(int);
			int			program_type(int);
			void		dump_all();
			void		dump_all_programs();

			int			get_empty(int);
			int			program_move(int, int);

			int	 		append_instruction(int);
			int 		insert_instruction(int, int);
			int 		delete_instruction(int, int);
			int 		get_instruction(int, int);
			int 		get_next_instruction(int);
			int 		create_opcode(int, int);
			int 		append_operand(int, variable*);
			int 		get_opcode(int, int *);
			variable 	*get_operand(int);
			int			get_operand_type(int, int*);
			int 		get_operand_text(int, char*);
			int 		get_comment(int, int, int, char*);
			int 		set_comment(int, int, int, int, char*);
			int			label(int, int, int, int, int*);
			int			size(){return maximum_size;};
	};

#endif

