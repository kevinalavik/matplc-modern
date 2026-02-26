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
// Last Modified: May 31, 2002
//


#ifndef EMAIL_HEADER
#define EMAIL_HEADER

	#include "../network_io/network_io.h"

	#define	_EMAIL_UNUSED		9000
	#define	_EMAIL_CONNECTING	9001
	#define	_EMAIL_HELO			9002
	#define	_EMAIL_FROM			9003
	#define	_EMAIL_TO			9004
	#define	_EMAIL_DATA			9005
	#define	_EMAIL_BODY			9006
	#define	_EMAIL_QUIT			9007
	#define	_EMAIL_DONE			9008

	#define	MAXIMUM_MAIL_LIST_SIZE	10
	struct	{	int		status;
			time_t	start;
			char	*destination;
			char	*subject;
			char	*message;
			network_io	*net;}	mail_list[MAXIMUM_MAIL_LIST_SIZE];

	int email_parse(char*);
	int email_add(char*, char*, char*);
	int email_send();

	#define		NETWORK_TIMEOUT	60


#endif
