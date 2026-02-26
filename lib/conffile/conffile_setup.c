/*
 * (c) 2000  Mario de Sousa
 *           Jiri Baum
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
 * Configuration File Reader - access library implementation
 *
 * This file implements the routines in conffile_setup.h
 * 
 */

#include "conffile.h"
#include "conffile_setup.h"

int conffile_setup(const char *conffile, const char *mod_name)
{
  return conffile_init(conffile, mod_name);
}

int conffile_shutdown(void)
{
  return 0;
}
