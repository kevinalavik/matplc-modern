/*
 * (c) 2004 Jiri Baum
 * (c) 2004 Mario de Sousa
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


#ifndef __RT_H
#define __RT_H

#include "../types.h"

/*** GENERAL ***/

int rt_init(const char *module_name);
int rt_done(void);

int rt_scan_beg(void);
int rt_scan_end(void);

/* returns the requested priority for this MatPLC module */
/* returns -1 on error */
int rt_getpriority(void);


#endif /* __RT_H */
