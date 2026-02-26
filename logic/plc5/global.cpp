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

#include <time.h>




void error_log(const char *text){
	int	__log_flag = _LOG_TO_FILE;
	FILE	*fp_out;
	time_t	now;

	time(&now);
	if((__log_flag & _LOG_TO_SCREEN) > 0){
		printf("%s: %s", text, ctime(&now));
	}
	if((__log_flag & _LOG_TO_FILE) > 0){
		if((fp_out = fopen("plc.log", "a")) != NULL){
			fprintf(fp_out, "%s: %s", text, ctime(&now));
			fclose(fp_out);
		} else {
			printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX Error log write failed\n");
		}
	}
}
