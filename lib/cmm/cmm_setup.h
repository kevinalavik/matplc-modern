/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
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
 * CMM Manager - setup and shutdown functions
 *
 */

#ifndef __CMM_SETUP_H
#define __CMM_SETUP_H

#include "../types.h"



#include "plc_private.h"
#include "cmm.h"
#include "plc.h"



int cmm_setup(int cmm_map_key);
/* returns 0 if successful               */
/* returns -1 on error                   */
/*
 * NOTE: use cmm_map_key = -1 to try get it's value from the config file or
 *       use the default.
 *       cmm_map_key = 0 will use a randomly generated key.
 *
 *       All other values are obtained from the config file.
 */


/* Shut down the cmm */
/* NOTE: Assumes that plc_init() has previously been called
 *       and that the following sub libraries are still up and running:
 *        - cmm
 *        - conffile
 *        - log
 */
int cmm_shutdown(void);

#endif /* __CMM_SETUP_H */

