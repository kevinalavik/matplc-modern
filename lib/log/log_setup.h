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


#ifndef __PLC_LOG_SETUP_H
#define __PLC_LOG_SETUP_H


/*
 * This file declares the functions required to setup the logging service
 */ 


int plc_log_setup(int debug_level,
                  const char *mod_name,   /* module name */
                  const char *trc_fname,
                  const char *wrn_fname,
                  const char *err_fname);

int plc_log_shutdown(void);


#endif /* __PLC_LOG_SETUP_H */

