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

#ifndef __GMM_SETUP_H
#define __GMM_SETUP_H

#include "gmm.h"


int gmm_setup(const char *module_name);
/* returns 0 if successful               */
/* returns -1 on error                   */
/*
 *       All other values are obtained from the config file.
 */


/* Shut down the gmm */
/* NOTE: Assumes that plc_init() has previously been called
 *       and that the following sub libraries are still up and running:
 *        - cmm
 *        - conffile
 *        - log
 */
int gmm_shutdown(void);
/* returns 0 if successful               */
/* returns -1 on error                   */


#endif				/* __GMM_SETUP_H */
