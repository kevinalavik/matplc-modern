/*
 * (c) 2000
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
  parport_util.h

  Processes need to be root, or run setuid-root, in order to run this,
  even with the call to ioperm().
*/


/*
  This code is based on the parport.c code of the EMC project.
  http://www.isd.cme.nist.gov/projects/emc/emcsoft.html
*/


#ifndef PARPORT_UTIL_H
#define PARPORT_UTIL_H

#include "../../lib/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_PARPORT_IO_ADDRESS 0x378
/* #define DEFAULT_PARPORT_IO_ADDRESS 0x278 */

typedef enum {regdir_out, regdir_in, regdir_none} ppt_regdir_t;

extern int ppt_init_dir(int parport_io_address,
	                ppt_regdir_t C_dir,
	                ppt_regdir_t D_dir);
extern int ppt_init_krn(const char *deb_file_name,
	                ppt_regdir_t C_dir,
	                ppt_regdir_t D_dir);
extern int ppt_done(void);

/* sets the direction (in/out mode) the register will use */
extern int ppt_set_Ddir(ppt_regdir_t dir);
extern int ppt_set_Cdir(ppt_regdir_t dir);
/* Sdir does not make sense, it can only de read, and not written to
extern int ppt_set_Sdir(ppt_regdir_t dir);
*/

/* gets the direction (in/out mode) the register is using */
extern ppt_regdir_t ppt_get_Cdir(void);
extern ppt_regdir_t ppt_get_Ddir(void);
extern ppt_regdir_t ppt_get_Sdir(void); /* will always return regdir_in */

/* writes value of digital output */
/* Will only work if register previously set to output mode */
/* returns 0 on success, -1 on error */
extern int ppt_set_C(u8 value);
extern int ppt_set_D(u8 value);

/* writes value of digital output bit referenced bu index */
/* Will only work if register previously set to output mode */
/* returns 0 on success, -1 on error */
extern int ppt_set_Cbit(int index, u8 value);
extern int ppt_set_Dbit(int index, u8 value);

/* reads value of digital input, stores result in *value */
/* Will work in both input and output modes */
/* returns 0 on success, -1 on error */
extern int ppt_get_D(u8 *value);
extern int ppt_get_C(u8 *value);
extern int ppt_get_S(u8 *value);

/* reads value of digital input bit referenced by index,and 
 * stores result in *value 
 */
/* Will work in both input and output modes */
/* returns 0/1 on success, -1 on error */
extern int ppt_get_Dbit(int index);
extern int ppt_get_Cbit(int index);
extern int ppt_get_Sbit(int index);


#ifdef __cplusplus
}
#endif

#endif  /* PARPORT_UTIL_H */
