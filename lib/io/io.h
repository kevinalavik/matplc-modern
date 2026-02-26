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
  io.h

  The functions declared in this file will be used by the implementations
  of the funtions whose prototype is defined in the io_hw.h file.
*/


#ifndef PLC_IO_H
#define PLC_IO_H

#include "../../lib/plc.h"
#include "io_hw.h"

#ifdef __cplusplus
extern "C" {
#endif


static inline u32  io_get_value(int count, void *opaque_ptr)
{
  return 0;
};

static inline void io_set_value(int count, void *opaque_ptr, u32 value)
{
};

/*
 * Change this variable during startup to over-ride the default scan cycle.
 * The new function should return a negative value on error. It is not
 * expected to return on success, but if it does, the value should be
 * non-negative.
 *
 * The parameter is an internal structure, access to which has not been
 * thought through at this point :-) We apologise for the inconvenience.
 */
extern int(*run_loop)(void*);


/*
 * Call this function when you need to get the plc_pt to which an
 * io_add has been mapped.
 * Make sure that the io_addr you pass is identical to the io_addr
 * generated when parsing the config file. The un-used must also match!
 * One way of doing this is by setting all the bytes to zero (memset())
 * before filling in the relevant data.
 *
 * NOTE: plc_pt_null() is returned if the io_addr is not found!
 */
plc_pt_t ioaddr_to_plcpt(io_addr_t *io_addr);

int slave_mapped_points(void);

int get_map_entry_by_index(int n, io_addr_t **addr, plc_pt_t *point);

#ifdef __cplusplus
}
#endif

#endif  /* PLC_IO_H */
