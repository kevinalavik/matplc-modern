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

#ifndef QUEUE_T_HEADER
#define QUEUE_T_HEADER




class	queue_atom {
	public:
		char		*message;
		queue_atom	*next;
};


class	queue_t {
	protected:
		char		*last_out;
		queue_atom	*first;
		queue_atom	*last;
	public:
				queue_t();
				~queue_t();
		int		push(char *);
		char	*pull();
		void	dump();
};

#endif
