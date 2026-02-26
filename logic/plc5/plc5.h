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

#ifndef __PLC5
#define __PLC5

	#include "store.h"
	#include "program.h"
	#include "parser.h"
	#include "instructions.h"
	#include "queue.h"

	#include <time.h>
	#include <sys/timeb.h>

	#define	MAXIMUM_STRING_LENGTH	83

	#define		_RUN		20
	#define		_FAULT		21
	#define		_PROGRAM	22


		

	class execution_stack{
		public:
			int		ladder_flag;
			int		prog_file;
			int		instruction;
			int		end_flag;
	};
	#define		MAXIMUM_STACK_SIZE		20	


	class plc5{
		protected:
			parser		*p;
//			plc5_commands 	*comms;

		public:
			execution_stack	*stack_exec;
			int				stack_pointer;
			queue			*communications;
			int	status;
			memory	*mem;
			program	*prog;
			int	focus_program;
			int	first_scan;
			timeb	time_now;
			timeb	time_last;
			// timeb	time_start;
			double	seconds;
			//double	milliseconds_last;
			//double	milliseconds_start;

			#define	STACK_SIZE	15
			struct{	int		list[STACK_SIZE];
				int		instruction[STACK_SIZE];
				int		count;
			} stack;
				plc5();
				~plc5();

			void array_memory_make(char*, char*, int, int);
			int	make_program(int);
			int	parse_line(char*);
			int	parse_line(int, int, char*);
			int	update_labels(int);
			int	find_function(char*, int*, int*);
			int	decode_argument(char*);
			int	decode_address(char*);
			variable	*create_address(char*);
			float	get_float_from_memory();
			int	set_float_in_memory(float);

			int	scan(int);
			int	scan_step();
			int instructions_basic_logic(int);
			int instructions_timer_counter(int);
			int instructions_math(int);
			int instructions_data(int);
			int instructions_program_control(int);
			int instructions_fancy(int);
			int instructions_ascii(int);
			int	push(int);
			int	pull(int*);
			void	dump_all();

			int	add_memory(int, int, int);
			int	set_memory_value(int, int, int, char*);
			int	set_memory_value(char*, char*);
			int	get_memory_value(int, int, int, int*);
			int	get_memory_value(int, int, float*);
			int	get_memory_value(int, int, char*);
			int	program_info(int, int*, int*);
			int	memory_info(int, int*, int*, int*);

			int	program_line(int, int, char*);
			int	opcode_text(char*);
	};



#endif
