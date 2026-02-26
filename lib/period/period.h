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


#ifndef __PERIOD_H
#define __PERIOD_H

#include "../types.h"

/*** GENERAL ***/

  /* WARNING: The following function is not thread safe in a non-standard way. */
  /*          Please see source code for more comments...                      */
int period_init(const char *module_name);

int period_done(void);

int period_scan_beg(void);
int period_scan_end(void);



/*
 * PERIOD functions;
 */

/*
 * Get the currently configured period
 *
 * returns -1 on error, or 0 on success.
 * Period is stored in *period_xxx on success.
 */
int plc_period_get(u32 *period_sec, u32 *period_nsec);


/*
 * Set the period
 */
int plc_period_set(u32 period_sec, u32 period_nsec);


#endif /* __PERIOD_H */
