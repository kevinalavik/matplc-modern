/*
 * (c) 2003 Jiri Baum
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

#ifndef __comedi_syntax_h
#define __comedi_syntax_h

typedef struct {
  char *device; /* dynamically allocated */
  int subdev;
  int chan;
  int range; /* not valid for digital addresses */
  int aref; /* not valid for digital addresses */
  signed char type; /* either a bit-field, or small integers */
} comedi_addr;

/* constants for the bitfield */

static const char COMEDI_ADDR_OUT = 1;
static const char COMEDI_ADDR_PHYS = 2;
static const char COMEDI_ADDR_DIG = 4;

/* these mean that the types are:
 *   0: raw analog input
 *   1: raw analog output
 *   2: physical analog input
 *   3: physical analog output
 *   4: digital input
 *   5: digital output
 * and a special value
 *  -1: error
 */

#define COMEDI_ADDR_RAWIN 0
#define COMEDI_ADDR_RAWOUT 1
#define COMEDI_ADDR_PHIN 2
#define COMEDI_ADDR_PHOUT 3
#define COMEDI_ADDR_DIN 4
#define COMEDI_ADDR_DOUT 5
#define COMEDI_ADDR_ERR (-1)

/* parse a comedi address from a (user-supplied) string
 *
 * On success, device will be non-null and type non-negative.
 * The device string is allocated on the heap; it's up to the caller to
 * free(3) it.
 *
 * On failure, device will be null and type will be -1.
 */
comedi_addr parse_comedi_addr(const char *s);

#endif /* __comedi_syntax_h */
