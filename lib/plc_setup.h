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

#ifndef __PLC_SETUP_H
#define __PLC_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

/* returns 0 if successful */
/* returns -1 on error     */
int plc_shutdown(int argc, char **argv);


/* returns 0 if successful */
/* returns -1 on error     */
int plc_setup(int argc, char **argv, const char *conffile);


#ifdef __cplusplus
}
#endif

#endif				/* __PLC_SETUP_H */
