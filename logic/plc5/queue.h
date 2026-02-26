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


#ifndef __QUEUE
#define __QUEUE


	class communication_atom{
		public:
			#define		_UNUSED				600
			#define		_WAITING			601
			#define		_DONE				602
			#define		_ERROR				603
			int			state;		// status info

			char		*message;
	};
	#define MAXIMUM_QUEUE_LENGTH	40


	class queue {
		private:
			communication_atom	*list;
			int			maximum_size;
		public:
						queue(int);
						~queue();
			int			add(char*);
			int			scan(int, char**);
			int			update(int, char*);
			int			append(int, char*);
			int			status(int);
			int			status(int, int);
			int			release(int);
			int			size(){return maximum_size;};
			void		dump();
	};

#endif
