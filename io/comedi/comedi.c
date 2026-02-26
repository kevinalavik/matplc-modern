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

/*
 * this file interfaces to the comedi library to talk to various DAQ cards
 */
#include <string.h>
#include <io/io_hw.h>
#include <comedilib.h>
#include "comedi_syntax.h"

const char *IO_MODULE_DEFAULT_NAME = "comedi";

typedef struct {
  unsigned char dev;
  char phys_init;
  int maxdata;
  comedi_range *rng;
} misc_t;

typedef struct {
  comedi_addr *reg;
  misc_t *misc;
} my_io_addr_t;

#define my_io_addr ((my_io_addr_t*)io_addr)
#define c_addr (*(my_io_addr->reg))
#define c_misc (*(my_io_addr->misc))
#define c_dev (c_misc.dev)

/* individual field access */
#define c_f (devs[c_dev] .f)

typedef struct {
  char *filename;
  comedi_t *f;
} device_t;

device_t *devs = 0;
unsigned char numdevs = 0;

int io_hw_parse_config(void) {
  if (sizeof(my_io_addr_t)>sizeof(io_addr_t)) {
    plc_log_errmsg(1,"Datatype size problem.");
    return -1;
  }

  return 0;
}

int io_hw_parse_io_addr(io_addr_t *io_addr, const char *addr_stri, dir_t dir, int pt_len) {
  int i;

  my_io_addr->reg=malloc(sizeof(comedi_addr));
  my_io_addr->misc=malloc(sizeof(misc_t));
  if ((!my_io_addr->reg) || (!my_io_addr->misc)) {
    plc_log_errmsg(1,"Couldn't allocate memory");
    return -1;
  }
  c_addr = parse_comedi_addr(addr_stri);
  if ((!c_addr.device) || (c_addr.type<0)) {
    plc_log_errmsg(1,"Couldn't understand address: %s", addr_stri);
    return -1;
  }

  c_misc.phys_init = 0;

  /* has this device already been used? */
  for (i=0;i<numdevs;i++) {
    if (!strcmp(devs[i].filename, c_addr.device)) {
      /* found it! */
      c_dev=i;
      return 0;
    }
  }

  /* device never before encountered */
  c_dev=numdevs++;
  devs=realloc(devs, numdevs*sizeof(device_t));
  devs[c_dev].filename = c_addr.device;

  return 0;
}

int io_hw_init(void) {
  int i, res=0;
  for (i=0; i<numdevs; i++) {
    devs[i].f = comedi_open(devs[i].filename);
    if (!devs[i].f) {
      plc_log_errmsg(1,"Couldn't open device %s", devs[i].filename);
      res=-1;
    }
  }

  return res;
}

int io_hw_read (io_addr_t *io_addr, u32 *value) {
  lsampl_t data;
  unsigned int bit;
  union {
    f32 f;
    u32 u;
  } tmp;

  switch (c_addr.type) {
    case COMEDI_ADDR_RAWIN:
      if (comedi_data_read(c_f, c_addr.subdev, c_addr.chan,
	               c_addr.range, c_addr.aref, &data)<0) {
	return -1;
      }
      *value = data;
      return 0;

    case COMEDI_ADDR_PHIN:
      if (comedi_data_read(c_f, c_addr.subdev, c_addr.chan,
	               c_addr.range, c_addr.aref, &data)<0) {
	return -1;
      }
      if (!c_misc.phys_init) {
        /* init rng and maxdata */
	c_misc.rng = comedi_get_range(c_f, c_addr.subdev,
	                              c_addr.chan, c_addr.range);
	c_misc.maxdata = comedi_get_maxdata(c_f, c_addr.subdev,
	                                    c_addr.chan);
	c_misc.phys_init = 1;
      }
      tmp.f = comedi_to_phys(data,c_misc.rng,c_misc.maxdata);
      *value = tmp.u;
      return 0;

    case COMEDI_ADDR_DIN:
      comedi_dio_read(c_f, c_addr.subdev, c_addr.chan, &bit);
      *value = bit;
      return 0;

    case COMEDI_ADDR_ERR:
      return -1;

    case COMEDI_ADDR_RAWOUT:
    case COMEDI_ADDR_PHOUT:
    case COMEDI_ADDR_DOUT:
      /* should never happen */
      *value = 0;
      return 0;
    default:
      return -1;
  }
}

int io_hw_read_end (void) {return 0;}

int io_hw_write(io_addr_t *io_addr, u32 value) {
  lsampl_t data;
  unsigned int bit;
  union {
    f32 f;
    u32 u;
  } tmp;

  switch (c_addr.type) {
    case COMEDI_ADDR_RAWOUT:
      data = value;
      if (comedi_data_write(c_f, c_addr.subdev, c_addr.chan,
	               c_addr.range, c_addr.aref, data)<0) {
	return -1;
      }
      return 0;

    case COMEDI_ADDR_PHOUT:
      if (!c_misc.phys_init) {
        /* init rng and maxdata */
	c_misc.rng = comedi_get_range(c_f, c_addr.subdev,
	                              c_addr.chan, c_addr.range);
	c_misc.maxdata = comedi_get_maxdata(c_f, c_addr.subdev,
	                                    c_addr.chan);
	c_misc.phys_init = 1;
      }
      tmp.u = value;
      data = comedi_from_phys(tmp.f, c_misc.rng, c_misc.maxdata);
      if (comedi_data_write(c_f, c_addr.subdev, c_addr.chan,
	               c_addr.range, c_addr.aref, data)<0) {
	return -1;
      }
      return 0;

    case COMEDI_ADDR_DOUT:
      bit = value;
      comedi_dio_write(c_f, c_addr.subdev, c_addr.chan, bit);
      return 0;

    case COMEDI_ADDR_ERR:
      return -1;

    case COMEDI_ADDR_RAWIN:
    case COMEDI_ADDR_PHIN:
    case COMEDI_ADDR_DIN:
      /* should never happen */
      return 0;

    default:
      return -1;
  }
}

int io_hw_write_end(void) {return 0;}

int io_hw_done(void) {
  int i, res=0;
  for (i=0; i<numdevs; i++) {
    if (devs[i].f)
      if (comedi_close(devs[i].f)<0)
	res=-1;
  }

  /* should also free(3) all the strings malloc(3)d by parse_comedi_addr(),
   * but never mind.
   */

  return res;
}

