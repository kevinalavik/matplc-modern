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
  io_private.h
*/


#ifndef IO_PRIVATE_H
#define IO_PRIVATE_H

#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif


   /* The following value should be identical to the
      CONF_POINT_SECTION defined in lib/gmm/gmm_private.h
      If this kind of redundancy becomes the norm
      while we try to adhere to our own rules of keeping
      the xxx_private.h interfaces private to the xxx libraries,
      then we will probably need to move thess defines
      to the /lib directory. For the moment, let's live with
      this and hope we won't be doing it again...
       (Mario)
    */
#define IO_CONF_POINT_SECTION "PLC"



#define IO_PTSTABLE_NAME "map"
#define IO_PTSTABLE_INV1 "inv"
#define IO_PTSTABLE_INV2 "invert"
#define IO_PTSTABLE_BIT  "bit"
#define IO_PTSTABLE_BYTE "byte"
#define IO_PTSTABLE_WORD "word"
#define IO_PTSTABLE_IN   "in"
#define IO_PTSTABLE_OUT  "out"
#define IO_PTSTABLE_SLAVE  "slave"


#ifdef __cplusplus
}
#endif

#endif  /* IO_PRIVATE_H */
