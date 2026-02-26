//
//    function.h - a definition of the functions in the logic core
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



#ifndef __FUNCTIONS_HEADER__
#define __FUNCTIONS_HEADER__

	#define		_VOID		10000
	#define		_BIT		10001
	#define		_STRING		10002
	#define		_INTEGER	10003
	#define		_REAL		10004
	#define		_ARRAY		10005
	#define		_UNION		10006


	#include "step.h"
	#include <time.h>
	#include <sys/timeb.h>
	

	class	processor_status {
		public:
			variable	*first_scan;
			variable	*error_routine;

			variable	*error;
			// bit numbers for errors
			#define		ERROR_MATH		0	// For div by zero, etc.
			#define		ERROR_STACK		1	// For stack underflow/overflow
			#define		ERROR_EXPR		2	// Invalid expression/variable

			variable	*core_flags;
			#define		FLAG_IGNORE_ERRORS		0	// while set, errors will be ignored

			timeb			time_now;
			timeb			time_last;
			// timeb		time_start;
			double			seconds;
			//double		milliseconds_last;
			//double		milliseconds_start;

			processor_status();
			~processor_status();
	};


	/////////////////////////////////////////////////////////////
	//
	// Basic execution stack definitions
	//
	/////////////////////////////////////////////////////////////

	class execution_stack{
		public:
			int		ladder_flag;
			step_t	*instruction;
			int		end_flag;
			int		arg_count;
			int		done_flag;
			int		if_or_similar;
	};
	#define		MAXIMUM_STACK_SIZE		20	




	class	truth_stack {
		protected:
			int			size;
			int			stack[100]; // At this point I have made this a limit of 100, later it should be variable
			int			stack_pnt;
		public:
						truth_stack(int _size){size = 100; stack_pnt = 0; /*stack = new *variable[size];*/};
						// ~truth_stack(delete stack;);
			void		clear(){stack_pnt = 0;};
			void		push(int val){if(stack_pnt < size){stack[stack_pnt]=val; stack_pnt++;}else{exit(1);}};
			int			pop(){if(stack_pnt > 0){stack_pnt--; return stack[stack_pnt];} else {return FALSE;}};
			int			top(){if(stack_pnt > 0){return stack[stack_pnt-1];} else {return FALSE;}};
			void		dump(){
							printf("Stack dump: ");
							for(int i = 0; i < stack_pnt; i++){
								printf(" [%d:%d] ", i, stack[i]);
							}
							printf("\n");
						}
	};



	class	variable_stack {
		protected:
			int			size;
			variable	*stack[100]; // At this point I have made this a limit of 100, later it should be variable
			int			stack_pnt;
		public:
						variable_stack(int _size){size = 100; stack_pnt = 0; /*stack = new *variable[size];*/};
						// ~variable_stack(delete stack;);
			void		clear(){stack_pnt = 0;};
			void		push(variable *var){if(stack_pnt < size){stack[stack_pnt]=var; stack_pnt++;}else{exit(1);}};
			variable	*pop(){if(stack_pnt > 0){stack_pnt--; return stack[stack_pnt];} else {return NULL;}};
			variable	*top(){if(stack_pnt > 0){return stack[stack_pnt-1];} else {return NULL;}};
			void		dump(){
							printf("Stack dump: ");
							for(int i = 0; i < stack_pnt; i++){
								// printf(" [%s] ", stack[i]->var->get_string());
								printf(" [%s:%s] ", stack[i]->var->get_string(), stack[i]->var->get_name());
							}
							printf("\n");
						}
	};



	extern truth_stack		*truth;
	extern execution_stack	*estack;
	extern variable_stack	*heap;
	extern int				stack_pointer;
	extern memory			*mem;
	extern program			*prog;
	extern processor_status	*stat;


	//////////////////////////////////////////////////////////////
	//
	// The basic function prototype
	//
	//////////////////////////////////////////////////////////////

	class function{
		public:
			// char	*description;
			int		arg_cnt;
			int		return_type;
			variable		*var;
							function();
			virtual			~function();
			virtual	int		pre_step();
			virtual int		step();
			virtual	char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};



	/////////////////////////////////////////////////////////////////
	//
	// The function library that holds all function code
	//
	/////////////////////////////////////////////////////////////////


	class function_library{
		public:
			function	*list[100];
			int			list_cnt;
						function_library();
						~function_library();
			function*	search(char*);
	};
	extern function_library	*library;



	//////////////////////////////////////////////////////////////
	//
	// Basic Logic function prototypes
	//
	//////////////////////////////////////////////////////////////

	class f_sor: public function{
		public:
							f_sor();
			virtual				~f_sor();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_eor: public function{
		public:
							f_eor();
			virtual				~f_eor();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_xic: public function{
		public:
							f_xic();
							~f_xic();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_xio: public function{
		public:
							f_xio();
							~f_xio();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_ote: public function{
		public:
							f_ote();
							~f_ote();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_otl: public function{
		public:
							f_otl();
							~f_otl();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_otu: public function{
		public:
							f_otu();
							~f_otu();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_bit: public function{
		public:
							f_bit();
							~f_bit();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_bst: public function{
		public:
							f_bst();
							~f_bst();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_nxb: public function{
		public:
							f_nxb();
							~f_nxb();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_bnd: public function{
		public:
							f_bnd();
							~f_bnd();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_lbl: public function{
		public:
							f_lbl();
							~f_lbl();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	///////////////////////////////////////////////////////////////
	//
	// Basic program structure prototypes
	//
	///////////////////////////////////////////////////////////////

	class f_end: public function{
		public:
							f_end();
							~f_end();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	///////////////////////////////////////////////////////////////
	//
	// Basic Data oriented function prototypes
	//
	///////////////////////////////////////////////////////////////

	class f_mov: public function{
		public:
							f_mov();
							~f_mov();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_clr: public function{
		public:
							f_clr();
							~f_clr();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};


	///////////////////////////////////////////////////////////////
	//
	// Boolean word manipulation function prototypes WITHOUT return values
	//
	///////////////////////////////////////////////////////////////

	class f_and: public function{
		public:
							f_and();
							~f_and();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_or: public function{
		public:
							f_or();
							~f_or();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_xor: public function{
		public:
							f_xor();
							~f_xor();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_not: public function{
		public:
							f_not();
							~f_not();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};



	///////////////////////////////////////////////////////////////
	//
	// Boolean word manipulation function prototypes WITH return values
	//
	///////////////////////////////////////////////////////////////

	class f_r_and: public function{
		public:
							f_r_and();
							~f_r_and();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_or: public function{
		public:
							f_r_or();
							~f_r_or();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_xor: public function{
		public:
							f_r_xor();
							~f_r_xor();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_not: public function{
		public:
							f_r_not();
							~f_r_not();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};


	///////////////////////////////////////////////////////////////
	//
	// Basic Math function prototypes WITHOUT return values
	//
	///////////////////////////////////////////////////////////////

	class f_add: public function{
		public:
							f_add();
							~f_add();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_sub: public function{
		public:
							f_sub();
							~f_sub();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_mul: public function{
		public:
							f_mul();
							~f_mul();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_div: public function{
		public:
							f_div();
							~f_div();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_xpy: public function{
		public:
							f_xpy();
							~f_xpy();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_neg: public function{
		public:
							f_neg();
							~f_neg();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_sqr: public function{
		public:
							f_sqr();
							~f_sqr();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_cos: public function{
		public:
							f_cos();
							~f_cos();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_sin: public function{
		public:
							f_sin();
							~f_sin();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_tan: public function{
		public:
							f_tan();
							~f_tan();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_acs: public function{
		public:
							f_acs();
							~f_acs();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_asn: public function{
		public:
							f_asn();
							~f_asn();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_atn: public function{
		public:
							f_atn();
							~f_atn();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_ln: public function{
		public:
							f_ln();
							~f_ln();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_log: public function{
		public:
							f_log();
							~f_log();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};



	///////////////////////////////////////////////////////////////
	//
	// Basic Math function prototypes WITH return values
	//
	///////////////////////////////////////////////////////////////

	class f_r_add: public function{
		public:
							f_r_add();
							~f_r_add();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_sub: public function{
		public:
							f_r_sub();
							~f_r_sub();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_mul: public function{
		public:
							f_r_mul();
							~f_r_mul();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_div: public function{
		public:
							f_r_div();
							~f_r_div();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_xpy: public function{
		public:
							f_r_xpy();
							~f_r_xpy();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_neg: public function{
		public:
							f_r_neg();
							~f_r_neg();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_sqr: public function{
		public:
							f_r_sqr();
							~f_r_sqr();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_cos: public function{
		public:
							f_r_cos();
							~f_r_cos();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_sin: public function{
		public:
							f_r_sin();
							~f_r_sin();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_tan: public function{
		public:
							f_r_tan();
							~f_r_tan();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_acs: public function{
		public:
							f_r_acs();
							~f_r_acs();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_asn: public function{
		public:
							f_r_asn();
							~f_r_asn();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_atn: public function{
		public:
							f_r_atn();
							~f_r_atn();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_ln: public function{
		public:
							f_r_ln();
							~f_r_ln();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_log: public function{
		public:
							f_r_log();
							~f_r_log();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};


	///////////////////////////////////////////////////////////////
	//
	// Basic Comparison function prototypes WITHOUT return values
	//
	///////////////////////////////////////////////////////////////

	class f_eq: public function{
		public:
							f_eq();
							~f_eq();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_gt: public function{
		public:
							f_gt();
							~f_gt();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_lt: public function{
		public:
							f_lt();
							~f_lt();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_geq: public function{
		public:
							f_geq();
							~f_geq();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_leq: public function{
		public:
							f_leq();
							~f_leq();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_ne: public function{
		public:
							f_ne();
							~f_ne();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_lim: public function{
		public:
							f_lim();
							~f_lim();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};




	///////////////////////////////////////////////////////////////
	//
	// Basic Comparison function prototypes WITH return values
	//
	///////////////////////////////////////////////////////////////

	class f_r_eq: public function{
		public:
							f_r_eq();
							~f_r_eq();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_gt: public function{
		public:
							f_r_gt();
							~f_r_gt();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_lt: public function{
		public:
							f_r_lt();
							~f_r_lt();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_geq: public function{
		public:
							f_r_geq();
							~f_r_geq();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_leq: public function{
		public:
							f_r_leq();
							~f_r_leq();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_ne: public function{
		public:
							f_r_ne();
							~f_r_ne();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_r_lim: public function{
		public:
							f_r_lim();
							~f_r_lim();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	///////////////////////////////////////////////////////////////
	//
	// High level function prototypes
	//
	///////////////////////////////////////////////////////////////


	class f_msg: public function{
		public:
							f_msg();
							~f_msg();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};


	///////////////////////////////////////////////////////////////
	//
	// Timer counter function prototypes
	//
	///////////////////////////////////////////////////////////////

	#define		_EN		15
	#define		_TT		14
	#define		_DN		13

	class f_ton: public function{
		public:
							f_ton();
							~f_ton();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_rto: public function{
		public:
							f_rto();
							~f_rto();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_tof: public function{
		public:
							f_tof();
							~f_tof();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	class f_res: public function{
		public:
							f_res();
							~f_res();
			virtual int		step();
			virtual char*	name();
			virtual	char*	description();
			virtual	function* clone();
	};

	///////////////////////////////////////////////////////////////
	//
	// Basic User function prototypes - add yours here so that they
	//		are easy to find later.
	//
	///////////////////////////////////////////////////////////////






#endif
