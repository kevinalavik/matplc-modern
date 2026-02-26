//
//	program_il.h - for storing instruction list programs
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


// #include "memory_address.h"
#include "store.h"
#include "comments.h"


#ifndef __PROGRAM_IL
#define __PROGRAM_IL

	#define _VALUE_UNDEFINED	1005

	class operand_il {
		public:
			operand_il		*next;
			variable		*address;
			comments_list	*comments;
	};

	class opcode_il {
		public:
			int				opcode;
			opcode_il		*next;
			operand_il		*first;
			comments_list	*comments;
	};

	#define	LABEL_LIST_SIZE		20
	class label_list {
		public:
			int				used;
			int				instruction;
	};

	class program_il{
		protected:
			opcode_il		*first; 
			opcode_il		*focus_in,
							*focus_out;
			operand_il		*operand_in,
							*operand_out;
			label_list		*labels;
		public:
			comments_list	*comments;
							program_il();
							~program_il();
			int 			append_instruction();
			int 			insert_instruction(int);
			int				delete_operands(opcode_il*);
			int				delete_instruction(int);
			int				get_instruction(int); 
			int				get_next_instruction();
			int				create_opcode(int);
			int				append_operand(variable *);
			int				get_opcode(int*);
			int				get_operand_type();
			variable		*get_operand();

			void			dump_all();

			#define	__OPERAND_COMMENT	200
			#define	__OPCODE_COMMENT	201
			#define	__PROGRAM_COMMENT	202
			int				get_comment(int, int, char*);
			int				set_comment(int, int, int, char*);

			#define		LABEL_CLEAR_ALL		900
			#define		LABEL_GET_VALUE		901
			#define		LABEL_SET_VALUE		902
			int				label(int, int, int, int*);
	};

#endif
