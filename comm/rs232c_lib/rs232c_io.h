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
// Last Modified: April 23, 2002
//

#include <global.h>


#ifndef __SERIAL
#define __SERIAL

	#define	BAUD_RATE	2000
	#define	PORT_FILE	2001
	#define	DATA_BITS	2002

	class serial_io {
		protected:
		public:
			int		fd;		  		/* File Descriptor Global Variable */
					serial_io();
					~serial_io();
			int		set_param(int, int, char*);
			int		connect();
			int		writer(char*);
			int		reader(char*, int);
	};

#endif
