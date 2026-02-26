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


#ifndef	PARSER_HEADER_DEF

#include "global.h"

// An early attempt at dynamic expansion
//typedef char* bloto;


#define MAX_NUMBER_TOKENS	50


class parser {
	char	*tokens[MAX_NUMBER_TOKENS];
	int	end_of_list;
	char	*tokens_string;
	int	new_flag;
	int	pnt_end_list;
    public:
		parser();
		~parser();
	int	parse(char *, char);
	int	parse_xml(char *);
	char*	token(int);
	char*	token_string(int, int);
	char*	token_string(int, char*);
	int	counter();
};


#define PARSER_HEADER_DEF
#endif
