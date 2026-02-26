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

/*
 * cif.h
 */

#ifndef __CIF_H
#define __CIF_H

#include <plc.h>
#include "cif_util.h"


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

#define CIF_BOARDID_NAME "BoardID"
#define CIF_BOARDID_MIN  0
#define CIF_BOARDID_MAX  (MAX_DEV_BOARDS - 1) /* defined in cif_user.h */
#define CIF_BOARDID_DEF  0

 /* timeout trying to access the cif card... */
#define CIF_TIMEOUT_NAME "timeout"
#define CIF_TIMEOUT_MIN  0          /* 0 = no timeout */
#define CIF_TIMEOUT_MAX  u32_MAX
#define CIF_TIMEOUT_DEF  (100L)   /* 100 ms */


/*
 * The size of the DPM (Dual Port Memory) of the cif card being used.
 * The module uses this paramter to determine the maximum
 * size in bytes of each of the Receive and Send Process Data
 * buffers in the DPM of the cif card.
 * The documentation I have at the moment specifies:
 *   512 bytes for cards with 2kBytes
 *  3584 bytes for cards with 8kBytes
 *
 * This parameter is not really very important. It is only used
 * during configure time to verify the user is not configuring:
 *  - more than one plc point to be written to the same location;
 *  - plc point to be written to an out-of-bounds location.
 * In such a case only a warning is issued, and the module will
 * run as configured, even if wrongly.
 *
 * At runtime the module will verify if the cif card that will
 * be used really does have the DPM with the specified size...
 *
 */
#define CIF_MAX_DPM_SIZE_NAME "DPMsize"
#define CIF_MAX_DPM_SIZE_MIN  0
#define CIF_MAX_DPM_SIZE_MAX  u32_MAX
#define CIF_MAX_DPM_SIZE_DEF  8   /* size in kBytes */


#define CIF_IOADDR_DEF_BIT 0

#endif /* __CIF_H */
