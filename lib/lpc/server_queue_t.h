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

/*
 * server side FIFO messaging system
 *
 * Last Revised: April 18, 2002
 */

#ifndef SERVER_QUEUE_T_HEADER
#define SERVER_QUEUE_T_HEADER


#include "message_t.h"


class	server_queue_t {
	protected:
		char	*my_name;
		int		fd;
		int		my_pid;
	public:
				server_queue_t(char*);
				~server_queue_t();
		char	*read_text();
};

#endif
