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
// Last Modified: October 12, 2000
//

#include "io.h"
#include "plc5.h"


#ifndef __CONTROLLER
#define __CONTROLLER

	#include <time.h>

	#define		_RUNNING		1100
	#define		_STEPPING		1101
	#define		_IDLE			1102
	#define		_QUITTING		1103
	#define		_FAULTED		1104

	#define		NETWORK_TIMEOUT	60


	struct user_data	{	int	user_number;
						int write_level;
						int	read_level;
						int	admin_level;};

	class controller{
		private:
			parser	*p;
			#define	MAXIMUM_SCAN_LIST_SIZE		10
			struct {	int			number;
						int			type;
						int			data;
						variable	*var;
						int			position;
						int			length; 
						int			channel;	} scan_list[MAXIMUM_SCAN_LIST_SIZE];
			int		scan_list_count;

			#define MAXIMUM_PROGRAM_LIST_SIZE	10
			int		program_list[MAXIMUM_PROGRAM_LIST_SIZE];

			#define MAXIMUM_WORM_LIST_SIZE	10
			int		worm_list[MAXIMUM_WORM_LIST_SIZE];

			#define MAXIMUM_SYNCHRONOUS_LIST_SIZE	10
			#define	_SYNC_NOT_CONNECTED		1900
			#define	_SYNC_WAITING		1901
			#define	_SYNC_RECEIVING			1902
			#define	_SYNC_SENDING			1903
			struct {	int 	number;
						//int		active;
						int		ack;
						int		fp;
						FILE	*fp_in;
						time_t	last_time;
						int		secure;
						char	incoming_string[200];
						int		incoming_string_buffer;
						user_data	user;
						int		status;	}	synchronous_list[MAXIMUM_SYNCHRONOUS_LIST_SIZE];
			int		synchronous_list_count;

			int		scan_count;
			int		scan_count2;

		public:
			int		status;

			plc5	*cpu;
			io		*in_out;

			int	add_scan(int, int, int, variable *, int, int, int);
			#define	_SCAN_INPUTS	1600
			#define	_SCAN_OUTPUTS	1601
			int	scan_list_scan(int);
			int	add_worm(int, int);
			int worm_list_number(int);
			int	worm_list_scan();
			int	add_synchronous(int);
			int	synchronous_list_scan();
			int	add_program(int, int);
			int	program_list_scan(int);

			int init();
			int load_plc_file(const char *);
			// int load_io_config(char *);
			// int load_program(char *);
			// int load_memory(char *);
			int update_inputs();
			int scan();
			int communication_update(int);
			int	string_extract(char*, int*, int*, int*);
			int update_outputs();
			#define	_SAVE_MEMORY	1
			#define _SAVE_IO		2
			#define	_SAVE_PROGRAM	4
			#define _SAVE_ALL		255
			int save_plc_file(const char *, int, const char*);
			int save_io_config(FILE *);
			int save_program(FILE*);
			int save_memory(FILE *, const char *);
			int	check_filter(const char*, const char*);
			int	password_check(char*, char*, user_data*);
			int	password_change(user_data*, char*);

			int shutdown();
	};

#endif
