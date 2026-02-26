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
 * parport.h
 */

#ifndef __PARPORT_H
#define __PARPORT_H

#include <plc.h>
#include "parport_util.h"




/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

#define PARPORT_DEVFILE_NAME "dev_file"

#define PARPORT_BASEADDR_NAME "io_addr"
#define PARPORT_BASEADDR_MIN   0x0000
#define PARPORT_BASEADDR_MAX   0x03FF  /* maximum values accepted by ioperm() */
#define PARPORT_BASEADDR_DEF   DEFAULT_PARPORT_IO_ADDRESS


#define PARPORT_CDIR_NAME "Cdir"
#define PARPORT_DDIR_NAME "Ddir"
#define PARPORT_DIR_IN    "in"
#define PARPORT_DIR_OUT   "out"

#define PARPORT_CREG "C"
#define PARPORT_CREG_MIN_BIT  0
#define PARPORT_CREG_MAX_BIT  3
#define PARPORT_CREG_DEF_BIT  PARPORT_CREG_MIN_BIT
#define PARPORT_DREG "D"
#define PARPORT_DREG_MIN_BIT  0
#define PARPORT_DREG_MAX_BIT  7
#define PARPORT_DREG_DEF_BIT  PARPORT_DREG_MIN_BIT
#define PARPORT_SREG "S"
#define PARPORT_SREG_MIN_BIT  0 /* the bits   3..7 of the S register get         */
#define PARPORT_SREG_MAX_BIT  4 /* shifted to 0..4 by the parport_util.c library */
#define PARPORT_SREG_DEF_BIT  PARPORT_SREG_MIN_BIT

static int PARPORT_REG_MIN_BIT[3] = {
		PARPORT_DREG_MIN_BIT,
		PARPORT_SREG_MIN_BIT,
		PARPORT_CREG_MIN_BIT
	   };

static int PARPORT_REG_MAX_BIT[3] = {
		PARPORT_DREG_MAX_BIT,
		PARPORT_SREG_MAX_BIT,
		PARPORT_CREG_MAX_BIT
	   };

static int PARPORT_REG_DEF_BIT[3] = {
		PARPORT_DREG_DEF_BIT,
		PARPORT_SREG_DEF_BIT,
		PARPORT_CREG_DEF_BIT
	   };

#endif /* __PARPORT_H */
