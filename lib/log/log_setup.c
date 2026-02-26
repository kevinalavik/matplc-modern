/*
 * (c) 2000 Mario de Sousa
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
 * This file implements the functions declared in log_setup.h
 */ 


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "log_setup.h"


int plc_log_setup(int debug_level, 
                  const char *mod_name,   /* module name */
                  const char *trc_fname,
                  const char *wrn_fname,
                  const char *err_fname)
{
  return plc_log_init(debug_level, 
                      mod_name,   /* module name */
                      trc_fname,
                      wrn_fname,
                      err_fname);
}


int plc_log_shutdown(void)
{
  return 0;
}
