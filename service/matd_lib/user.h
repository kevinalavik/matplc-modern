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
// Last Modified: May 10, 2002
//


#ifndef USER_HEADER_FILE
#define	USER_HEADER_FILE

	#define	PASSWORD_FILE		"passwd"
	#define	OLD_PASSWORD_FILE	"passwd.old"
	#define	TEMP_PASSWORD_FILE	"passwd.tmp"

	#define	PERMISSION_READ		100
	#define	PERMISSION_WRITE	101
	#define	PERMISSION_ADMIN	102

	#define	LEVEL_NONE			0
	#define	LEVEL_LOW			1
	#define	LEVEL_MEDIUM		2
	#define	LEVEL_HIGH			3

	class	 user_data	{
		public:
			int		write_level;
			int		read_level;
			int		admin_level;
			char	*passwd;
			char	*id;
			char	*comments;

			user_data	*next;
			user_data	*prev;

			user_data();
			~user_data();
			int		password(char*);
			int		encode_password(char*);
			char 	*password();
			int		identity(char*);
			char 	*identity();
			int		comment(char*);
			char 	*comment();
	};

	class	user_t {
		public:
			user_data	*first;

			user_t();
			~user_t();
			user_data 	*add_user(char*);
			user_data	*find(char*);
			int 	delete_user(user_data*);
			int 	load();
			int 	save();
			int 	verify_password(user_data*, char*);
			int		dump();
			int		dump(user_data*);
	};


#endif
