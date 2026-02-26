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
  cif_util.h

*/



#ifndef CIF_UTIL_H
#define CIF_UTIL_H

#include "../../lib/types.h"
/*#include <cifuser.h>*/
#include <cif_user.h>


int cif_init(unsigned short BoardId);

int cif_done(void);

int cif_DPM_size(void);

int cif_read_in(unsigned short data_adr,  /* where to get the data from */
                unsigned short data_cnt,
                u8             *data_ptr,
                unsigned long  time_out);

int cif_write_out(unsigned short data_adr,  /* where to get the data from */
                  unsigned short data_cnt,
                  u8             *data_ptr,
                  unsigned long  time_out);

int cif_read_out(unsigned short data_adr,  /* where to get the data from */
                 unsigned short data_cnt,
                 u8             *data_ptr
                );

int cif_read_write(unsigned short w_data_adr,    /* where to put the data */
                   unsigned short w_data_cnt,
                   u8             *w_data_ptr,
                   unsigned short r_data_adr,    /* where to get the data */
                   unsigned short r_data_cnt,
                   u8             *r_data_ptr,
                   unsigned long  time_out);



/* include the inlined functions */
#ifndef __CIF_UTIL_C
#include "cif_util.i"
#endif /* __CIF_UTIL_C */

#endif /* CIF_UTIL_H */
