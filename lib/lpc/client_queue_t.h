/*
 * (c) 2002 Hugh Jack
 *
 * Offered to the public under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * This code is made available on the understanding that it will not be
 * used in safety-critical situations without a full and competent review.
 */

#ifndef CLIENT_QUEUE_T_HEADER
#define CLIENT_QUEUE_T_HEADER

#include "message_t.h"


class	client_queue_t {
	protected:
		int			fd;
		char		*my_name;
		int			my_pid;
		int			remote_pid;
		message_header_t	header;
		int			header_size;
	public:
		int		status;
		client_queue_t	*next;
		char		*remote_name;
				client_queue_t(char*, char*);
				~client_queue_t();
		void	writer(char *);
};

#endif
