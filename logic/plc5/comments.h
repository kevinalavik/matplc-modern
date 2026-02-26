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

#include "global.h"


#ifndef __COMMENTS
#define __COMMENTS

	class comment_element {
		public:
			char		*text;
			comment_element	*next;
	};

	class comments_list{
		public:
			comment_element	*first; 
				comments_list();
				~comments_list();
			int	delete_comment(int);
			int 	append_comment(char*);
			int 	insert_comment(int, char*);
			int	get_comment(int, char*);

			#define	__APPEND_COMMENT	203
			#define	__REPLACE_COMMENT	204
			#define	__INSERT_COMMENT	205
			#define	__DELETE_COMMENT	206
			int	set_comment(int, int, char*);
			int	find_comment(char*, int*);
	};

#endif
