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

#ifndef HANDLER_T_HEADER
#define HANDLER_T_HEADER

#include "module_library.h"
#include <plc.h>
#include "queue_t.h"
#include "server_queue_t.h"
#include "client_queue_t.h"

//
// some common variable definitions
//

#define	SAFETY_LEVEL	0		// set the level from 0=do anything to 9=paranoid


/*
 * Common headers for the loader API 
 */


struct argument_t {
	char		*name;
	char		*value;
	argument_t	*next;
};



//long	update_time;	// For realtime tasks
//long	time_left;


class	mat_handler {		// define a basic container
	protected:
		int		status;
		char	*file_name;

		// plc_timer_t timer;
		long	delay;
		int		initialized;
		server_queue_t	*server;
		client_queue_t	*client_first;
	public:
		char	*module_name;

		argument_t		*arg_first;
		queue_t	*queue;
				mat_handler();
				~mat_handler();
		void	delay_set(int);
		int		version(){return MODULE_VERSION_NUMBER;};
		char	*version_date(){return (char*)MODULE_VERSION_DATE;};
		void	set_file_name(char *);
		void	set_module_name(char *);
		int		parse_command(char *, int, char**);
		int		send_remote_message(char*, char*);
		int		parse_file(char *);
		int		clean_string(char *);
		int		step(int, char**);
		void	scan();
		int		connect_to_mat();
		int		load_library();
		void	unload_library();
		int		set_argument(char*, char*);
		void	dump(int);

		// The definitions below are for the dynamically linked library
		void	*handle;	// A handle to the library itself
		int		(*__module_version)();
		void	(*__set_message_pointer)(int(*)(char*));
		void	(*__set_argument_pointer)(char*(*)(int, char*));
		int		(*init)();
		int		(*deinit)();
		int		(*step_run)();
		int		(*step_idle)();
		int		(*message_receive)(char*);
		char	*(*argument_descriptions)(int);
};



#endif

