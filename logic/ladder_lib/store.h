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
// Last Modified: December 15, 2000
//


#include "global.h"
//#include "../include/global.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>




#ifndef __STORE_HEADER
#define __STORE_HEADER

	class variable;

	#define		_LITERAL		6000
	#define		_VARIABLE		6001

	class data{
			char	*name;
			char	*symbol;
			char	*comment;
		public:
			int		type;					// to specify literals or variables
			// virtual	int		bit;

							data();
			virtual			~data();
			virtual int		get_type();
			virtual char*	get_symbol();
			virtual void	set_symbol(char*);
			virtual char*	get_name();
			virtual void	set_name(char*);
			virtual char*	get_comment();
			virtual void	set_comment(char*);

			virtual int		get_int(){return 0;};		// integers
			virtual void	set_int(int){};
			virtual double	get_real(){return 0;};		// reals
			virtual void	set_real(double){};
			virtual char	*get_string(){return 0;};	// strings
			virtual void	set_string(char *){};
			virtual int		get_bit(){return 0;};		// bits
			virtual int		get_bit_number(){return 0;};		// bits
			virtual int		get_bit(int){return 0;};
			virtual void	set_bit(int, int){};
			virtual void	set_bit(int){};
			virtual	void	set_var(int, variable*){};
			virtual void	set_size(int){};
			virtual	int		get_size(){return 1;};
			virtual variable*get_variable(int){return NULL;};
	};


	class array: public data{
			variable	**list;
			int			size;
		public:
							array();
			virtual 		~array();
			virtual int		get_type();
			virtual int		get_int();
			virtual double	get_real();
			virtual char	*get_string();
			virtual void	set_int(int);
			virtual void	set_real(double);
			virtual void	set_string(char *);
			virtual int		get_bit();
			virtual int		get_bit(int);
			virtual void	set_bit(int, int);
			virtual void	set_bit(int);
			virtual	void	set_var(int, variable*);
			virtual void	set_size(int);
			virtual int		get_size();

			virtual variable*get_variable(int);
	};

	class integer: public data{
			int		value;
		public:
							integer();
			virtual 		~integer();
			virtual int		get_type();
			virtual int		get_int();
			virtual double	get_real();
			virtual char	*get_string();
			virtual void	set_int(int);
			virtual void	set_real(double);
			virtual void	set_string(char *);
			virtual int		get_bit();
			virtual int		get_bit(int);
			virtual void	set_bit(int, int);
			virtual void	set_bit(int);
			virtual	void	set_var(int, variable*);
			virtual void	set_size(int);
			virtual variable*get_variable(int);
	};

	class bit: public data{
			variable	*var;
			int			_bit;
		public:
							bit();
			virtual			~bit();
			virtual int		get_type();
			virtual int		get_int();
			virtual double	get_real();
			virtual char	*get_string();
			virtual void	set_int(int);
			virtual void	set_real(double);
			virtual void	set_string(char *);
			virtual int		get_bit();
			virtual int		get_bit_number();
			virtual int		get_bit(int);
			virtual void	set_bit(int, int);
			virtual void	set_bit(int);
			virtual	void	set_var(int, variable*);
			virtual void	set_size(int);
			virtual variable*get_variable(int);
	};

	class real: public data{
			double	value;
		public:
							real();
			virtual 		~real();
			virtual int		get_type();
			virtual int		get_int();
			virtual double	get_real();
			virtual char	*get_string();
			virtual void	set_int(int);
			virtual void	set_real(double);
			virtual void	set_string(char *);
			virtual int		get_bit();
			virtual int		get_bit(int);
			virtual void	set_bit(int, int);
			virtual void	set_bit(int);
			virtual	void	set_var(int, variable*);
			virtual void	set_size(int);
			virtual variable*get_variable(int);
	};

	class string: public data{
			char	*value;
		public:
							string();
			virtual			~string();
			virtual int		get_type();
			virtual int		get_int();
			virtual double	get_real();
			virtual char	*get_string();
			virtual void	set_int(int);
			virtual void	set_real(double);
			virtual void	set_string(char *);
			virtual int		get_bit();
			virtual int		get_bit(int);
			virtual void	set_bit(int, int);
			virtual void	set_bit(int);
			virtual	void	set_var(int, variable*);
			virtual void	set_size(int);
			virtual variable*get_variable(int);
	};


	class variable{
		public:
			data			*var;
			int				type;
			variable		*child;
			variable		*next;
	};

	class memory{
		public:
			variable	*list;
						memory();
						~memory();
			variable	*add(variable*, char*, char*, char*, int);
			variable	*find(variable*, char*);
			variable	*find(char*);
			variable	*find_bit(variable*, int);
			void		dump_all();
	};

#endif
