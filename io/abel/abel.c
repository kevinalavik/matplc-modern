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
 * this file interfaces to Ron Gage's ABEL library to talk to the AB PLC5
 * and similar
 */
#include "../lib/io/io_hw.h"
#include "libabplc5.h"

const char *IO_MODULE_DEFAULT_NAME = "abel";

struct _data data;
struct _comm comm;

int type;

typedef struct {
  char *reg;
  char longflag; /* this indicates that point is more than 16 bits (ie, 17-32) */
} my_io_addr_t;

#define my_io_addr ((my_io_addr_t*)io_addr)

int io_hw_parse_config(void) {
  if (sizeof(my_io_addr_t)>sizeof(io_addr_t)) {
    plc_log_errmsg(1,"Datatype size problem.");
    return -1;
  }

  return 0;
}

int io_hw_init(void) {
  char *ip;
  struct plc5stat status;
  ip = conffile_get_value("IP");
  comm = attach(ip,FALSE);
  free(ip);
  if (comm.error != 0) {
    plc_log_errmsg(1,"Could not attach to plc.");
    return -1;
  }
  status = getstatus (comm, FALSE);
  if (status.type == 0xde)
    type = PLC5250;
  else if (status.type == 0xee)
    type = SLC;
  else
    type = PLC5;

  comm.tns += 4;
  return 0;
}

int io_hw_parse_io_addr(io_addr_t *io_addr, const char *addr_stri, dir_t dir, int pt_len) {
  my_io_addr->reg = strdup(addr_stri);
  my_io_addr->longflag = (pt_len>16);
  if (my_io_addr->reg)
    return 0;
  else
    return -1;
}

int io_hw_read (io_addr_t *io_addr, u32 *value) {
  data = word_read(comm, my_io_addr->reg,1,type,FALSE);
  if (data.len < 0) {
    plc_log_errmsg(2,"Could not read data from PLC.");
    return -1;
  }
  else if (data.len == 0) {
    plc_log_errmsg(3,"Got zero-length data from PLC.");
    return -1;
  }
  else if (data.len == 1) {
    *value = data.data[0];
  } else {
    ((unsigned short*)value)[0] = data.data[1];
    ((unsigned short*)value)[1] = data.data[0];
    /* if it's longer than that, it gets thrown away - tough */
  }
  return 0;
}

int io_hw_read_end (void) { return 0; }

int io_hw_write(io_addr_t *io_addr, u32 value) {
  if (my_io_addr->longflag) {
    data.data[1] = ((unsigned short*)&value)[0];
    data.data[0] = ((unsigned short*)&value)[1];
  } else {
    data.data[0] = ((unsigned short*)&value)[0];
    data.data[1] = 0;
  }
  data = word_write (comm, my_io_addr->reg, 1, data, type, TRUE);
  if (data.len < 0) {
    plc_log_errmsg(2,"Could not write data to the PLC.");
    return -1;
  }
  return 0;
}

int io_hw_write_end(void) { return 0; }

int io_hw_done(void) {
  /* 
   * This is *so* incomplete (the memory leak!), but at least close the pipe.
   * It's not like this fn will ever get called...
   */
  close(comm.handle);
  return 0;
}

