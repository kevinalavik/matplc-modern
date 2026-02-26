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
  io_hw.h

  These are the declarations of the functions to access IO hardware.
  Their implementation is hardware specific. Each different implementation,
  linked to the io library, will generate a new IO module.
*/

/*
  The io.o library is guaranteed to call the io_hw library functions
  in the following order:

  io_hw_parse_config();   -> called once
  io_hw_parse_io_addr();  -> called once for every io_addr in the map table
  io_hw_init();           -> called once
  while (1)
    io_hw_read_XXX();
    io_hw_write_XXX();
  io_hw_done();           -> called once

  Additionally, all functions may assume that plc_init() has been previously
  called, and therefore the plc is correctly initialised...
*/

#ifndef PLC_IO_HW_H
#define PLC_IO_HW_H

#include "../../lib/plc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u8 opaque[8];
	} io_addr_t;

typedef enum {
	dir_in,
	dir_out,
        dir_none
	} dir_t;



extern int io_hw_write_end(void);
extern int io_hw_read_end (void);

  /* NOTE:
   *       the following functions are for reading/writing a single
   *       point at a time.
   *
   *       They should send/store the result in the 'value' variable
   */
extern int io_hw_write(io_addr_t *io_addr, u32  value);
extern int io_hw_read (io_addr_t *io_addr, u32 *value);

  /* NOTE:
   *       the following functions are for reading/writing an array
   *       of single plc points.
   *
   *       For each single point (i = 0 .. count), they should get/set the value using the
   *       u32  io_get_value(count, opaque_ptr);
   *       void io_set_value(count, opaque_ptr, value);
   *       WARNING: the above two functions are not protected against 'count' overflows,
   *                and may result in memory access violations when passed an out of bounds
   *                count value!
   *       WARNING: NOT YET IMPLEMENTED!!!
   */
extern int io_hw_write_array(io_addr_t *io_addr, int count, void *opaque_ptr);
extern int io_hw_read_array (io_addr_t *io_addr, int count, void *opaque_ptr);

  /* NOTE:
   *       the following functions are for reading/writing
   *       a plc point array.
   *
   */
/*
extern int io_hw_write_array(io_addr_t *io_addr, int count, plc_pt_t plc_pt);
extern int io_hw_read_array (io_addr_t *io_addr, int count, plc_pt_t plc_pt);
*/

  /* This function must parse the io address described in the string, and
   * store it in the io_addr struct.
   * It may use the direction, pt_len values for
   * consistency checking.
   * Should return 0 on success, -1 on failure.
   */
extern int io_hw_parse_io_addr(io_addr_t *io_addr,
                               const char *addr_stri,
                               dir_t dir,
                               int pt_len);

 /* This function is called so the io_hw library gets a chance to parse
  * any specific configuration before the io_hw_init() functions gets
  * called. */
extern int io_hw_parse_config(void);

 /* connect to hardware, etc... */
extern int io_hw_init(void);

 /* terminate connection to hardware... */
extern int io_hw_done(void);

 /* the default name of the IO module */
extern const char *IO_MODULE_DEFAULT_NAME;

 /* Should dump to plc_log_trcmsg(debug_level, ...) any configuration
  * parameters it will use. Used only for debugging purposes.
  * Please use the debug_level specified.
  */
extern int io_hw_dump_config(int debug_level);

 /* Should return a string description of
  * the io_addr. Memory for the string must be
  * malloc()'d, and will be free()'d by the
  * calling function in the io library.
  */
extern char *io_hw_ioaddr2str(io_addr_t *io_addr);

#ifdef __cplusplus
}
#endif

#endif  /* PLC_IO_HW_H */
