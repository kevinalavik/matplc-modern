//
//    Copyright (C) 2000-2 by Hugh Jack <jackh@gvsu.edu>
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


#ifndef LPCD_HEADER
#define LPCD_HEADER

#include "user.h"

	#define		NETWORK_TIMEOUT	60

	#define	_SYNC_NOT_CONNECTED		1900
	#define	_SYNC_WAITING			1901
	#define	_SYNC_RECEIVING			1902
	#define	_SYNC_SENDING			1903
	#define	_SYNC_STROBING			1904


	class strobe_list_item	{
		public:
			strobe_list_item	*next;
			strobe_list_item	*prev;
			plc_pt_t			var;
			char				*name;
	};

	class strobe_list	{
		public:
			strobe_list_item	*first;
			strobe_list_item	*scan;
			strobe_list();
			~strobe_list();
			int		add(char*);
			strobe_list_item	*find(char*);
			int		remove(strobe_list_item*);
			int		scan_init();
			strobe_list_item		*scan_next();
	};


	struct {	//int 	number;
			//int		active;
			int		ack;
			int		fp;
			FILE	*fp_in;
			time_t	last_time;
			int		secure;
			char	incoming_string[200];
			int		incoming_string_buffer;
			user_data	user;
			int		status;
			strobe_list	*strobe;	}	synchronous_list;



	int sync_init();
	int sync_deinit();
	int sync_scan(char*);
	int sync_send();
	int sync_receive(char*);
	int sync_strobe();
	int sync_message(char*);
	int filename_is_secure(char*);

#endif
