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
  cif_util.i

  The inlined functions...

*/



#ifndef CIF_UTIL_I
#define CIF_UTIL_I

extern unsigned short cif_util_BoardId_;


inline int cif_read_in(unsigned short data_adr,  /* where to get the data from */
                       unsigned short data_cnt,
                       u8             *data_ptr,
                       unsigned long  time_out)
{
/*  __assert_cif_initialized(-1); */

  return DevExchangeIO(cif_util_BoardId_,
                       0,               /* send offset */
                       0,               /* send size   */
                       NULL,            /* send buffer */
                       data_adr,        /* recv offset */
                       data_cnt,        /* recv size   */
                       data_ptr,        /* recv buffer */
                       time_out);    /* ulTimeout   */
}


inline int cif_read_out(unsigned short data_adr,  /* where to get the data from */
                        unsigned short data_cnt,
                        u8             *data_ptr
                       )
{
/*  __assert_cif_initialized(-1); */

  return DevReadSendData(cif_util_BoardId_,
                         data_adr,        /* recv offset */
                         data_cnt,        /* recv size   */
                         data_ptr         /* recv buffer */
                        );
}


inline int cif_write_out(unsigned short data_adr,  /* where to put the data */
                         unsigned short data_cnt,
                         u8             *data_ptr,
                         unsigned long  time_out)
{
/*  __assert_cif_initialized(-1); */

  return DevExchangeIO(cif_util_BoardId_,
                       data_adr,        /* send offset */
                       data_cnt,        /* send size   */
                       data_ptr,        /* send buffer */
                       0,               /* recv offset */
                       0,               /* recv size   */
                       NULL,            /* recv buffer */
                       time_out);    /* ulTimeout   */
}



inline int cif_read_write(unsigned short w_data_adr,    /* where to put the data */
                          unsigned short w_data_cnt,
                          u8             *w_data_ptr,
                          unsigned short r_data_adr,    /* where to get the data */
                          unsigned short r_data_cnt,
                          u8             *r_data_ptr,
                          unsigned long  time_out)
{
/*  __assert_cif_initialized(-1); */

  return DevExchangeIO(cif_util_BoardId_,
                       w_data_adr,        /* send offset */
                       w_data_cnt,        /* send size   */
                       w_data_ptr,        /* send buffer */
                       r_data_adr,        /* recv offset */
                       r_data_cnt,        /* recv size   */
                       r_data_ptr,        /* recv buffer */
                       time_out);    /* ulTimeout   */
}

#endif /* CIF_UTIL_I */
