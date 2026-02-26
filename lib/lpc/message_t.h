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

#ifndef MESSAGE_T_HEADER
#define MESSAGE_T_HEADER



#define FIFO_PERMISSIONS	0700	// same process owner only

//
// definitions for message headers
//
#define	MSG_MAGIC_NUMBER_FIRST	69	// an arbitrary start of message
#define MSG_MAGIC_NUMBER_SECOND	96	
#define	MSG_VERSION			1		// a message versioning number
#define MSG_T_TEXT			0		// the body message is text
#define MSG_T_BINARY		1		// the body of the message is a binary structure

struct message_header_t {
	char	version;
	pid_t	source;
	pid_t	destination;
	char	type;
	short int	size;
};


#endif

