/*
 * (c) 2001 Mario de Sousa
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


#ifndef __STATE_SETUP_H
#define __STATE_SETUP_H

#include "state.h"

/*** GENERAL ***/

int state_setup(const char *module_name);

/* Shut down the state */
/* NOTE: Assumes that plc_init() has previously been called
 *       and that the following sub libraries are still up and running:
 *        - cmm
 *        - log
 */
int state_shutdown(void);

#endif /* __STATE_SETUP_H */
