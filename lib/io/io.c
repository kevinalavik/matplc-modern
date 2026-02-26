/*
 * (c) 2001 Mario de Sousa
 *           Created the 'new' io library, inheriting Jiri's previous
 *           io library with the single io_status_pt() function.
 *
 * (c) 2000 Jiri Baum
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
 * this file implements the generic io module. It requires an implementation of the
 *  functions in io_hardware.h to generate a complete IO module.
 *
 * The generic IO module may work in one of two modes: master or slave.
 * By default it will work in the master mode, running the run_loop_master() function.
 * To run in slave mode, this function needs to be over-riden by another version
 * of run_loop().
 *
 * The master mode runs a scan cycle. In each scan it will write to every
 * io_addr in the io_write array, and read from every io_addr in the io_read
 * array.
 *
 * The slave mode does whatever the new run_loop() function implements.
 * It is expected that this new run_loop() function will work in a passive mode,
 * waiting for requests from external sources, and replying to these
 * requests. It may, or may not, run a scan cycle...
 * The new run_loop() function is expected to use the
 * ioaddr_to_plcpt() function to obtain the plc_pt_t to which the io_addr
 * is mapped.
 * The ioaddr_to_plcpt() function uses io_slave array to map io_addr to plc_pts
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>

#include "../misc/string_util.h"

#include "io_private.h" /* global declarations private to this library's functions */
#include "io_hw.h"	/* the hardware access functions, different implementation for each I/O module */
#include "io.h"         /* the functions we publicly export to be used by the hardware access functions */



/* should it dump the config after reading it in? */
#undef debug_dump
/* #define debug_dump */

/* definitions */
static const int debug = 0;



/* methods that can be over-ridden by user */
int(*run_loop)(void*) = 0;



/* global variable types */

typedef struct {
	  u8        inv_mask;  /* 0x00 - not invert; 0x01 - invert */
	  plc_pt_t  plc_pt;
	  io_addr_t io_addr;
	} pt_map_entry_t;


typedef struct {
	  pt_map_entry_t *pts;
	  int         num_pts;
	} pt_map_t;

static inline void pt_map_init(pt_map_t *pt_map) {
  pt_map->pts     = NULL;
  pt_map->num_pts = 0;
}

static inline int pt_map_add(pt_map_t *pt_map, pt_map_entry_t pt_map_entry) {
  pt_map_entry_t *tmp_ptr;

  tmp_ptr = realloc(pt_map->pts, (1+pt_map->num_pts) * sizeof(pt_map_entry_t));
  if (tmp_ptr == NULL)
    return -1;

  pt_map->num_pts++;
  pt_map->pts = tmp_ptr;
  pt_map->pts[pt_map->num_pts - 1] = pt_map_entry;

  return 0;
}


typedef struct {
          /* Used by modules in master mode... */
          /* points to which this module will write to in each scan... */
	pt_map_t io_write;
	pt_map_t io_write_array;   /* not yet implemented... */
          /* Used by modules in master mode... */
          /* points from which this module will read from in each scan... */
	pt_map_t io_read;
	pt_map_t io_read_array;    /* not yet implemented... */
          /* Used by modules in slave mode... */
	pt_map_t io_slave;
	pt_map_t io_slave_array;   /* not yet implemented... */
	} pt_map_lists_t;


static inline void pt_map_lists_init(pt_map_lists_t *pt_map_lists) {
  pt_map_init(&(pt_map_lists->io_write));
  pt_map_init(&(pt_map_lists->io_write_array));
  pt_map_init(&(pt_map_lists->io_read));
  pt_map_init(&(pt_map_lists->io_read_array));
  pt_map_init(&(pt_map_lists->io_slave));
  pt_map_init(&(pt_map_lists->io_slave_array));
}


/* We have a single global variable of this type... */
pt_map_lists_t *map_lists = NULL;



/***********************************/
static int io_read_inputs(pt_map_lists_t *map_lists)
{
  int count;
  u32 value;
  pt_map_entry_t  *pt_map_ptr;

  for (count = 0, pt_map_ptr = map_lists->io_read.pts;
       count < map_lists->io_read.num_pts;
       count++, pt_map_ptr++) {

    if (io_hw_read(&(pt_map_ptr->io_addr), &value) < 0) {
      continue;
    }

    if (pt_map_ptr->inv_mask == 0x00)
      plc_set(pt_map_ptr->plc_pt, value);
    else
      plc_seti(pt_map_ptr->plc_pt, value);
  } /* for(;;) */

  io_hw_read_end();

  return 0;
}


/********************************/
static int io_write_outputs(pt_map_lists_t *map_lists)
{
  int count;
  pt_map_entry_t  *pt_map_ptr;

  for (count = 0, pt_map_ptr = map_lists->io_write.pts;
       count < map_lists->io_write.num_pts;
       count++, pt_map_ptr++) {

    if (io_hw_write(&(pt_map_ptr->io_addr),
		    (pt_map_ptr->inv_mask == 0)?
                        plc_get(pt_map_ptr->plc_pt):plc_geti(pt_map_ptr->plc_pt))
        < 0) {
      continue;
    }
  } /* for(;;) */

  io_hw_write_end();

  return 0;
}


/************************************************/
static int run_loop_master(pt_map_lists_t *map_lists)
{
 while (1) {
  plc_scan_beg();

  io_read_inputs(map_lists);
  plc_update();
  io_write_outputs(map_lists);

  plc_scan_end();
 } /* while (1) */

 return -1;
}


/************************************************/
/* Do the mapping between ioaddr and corresponding plc_pt
 * Used by the slave modules...
 *
 * NOTE: we do a linear search as the list is most probably not very long.
 *       A future version will probably use much speedier hashing functions...
 */

plc_pt_t ioaddr_to_plcpt(io_addr_t *io_addr) {
  pt_map_entry_t *pt_map_entry;
  int i;

  if (map_lists == NULL)
    return plc_pt_null();

  pt_map_entry = map_lists->io_slave.pts;
  for (i = 0; i < map_lists->io_slave.num_pts; i++, pt_map_entry++) {
    if (memcmp(&(pt_map_entry->io_addr), io_addr, sizeof(io_addr_t)) == 0)
      return pt_map_entry->plc_pt;
  }

  /* not found! */
  return plc_pt_null();
}

int get_map_entry_by_index(int n, io_addr_t **addr, plc_pt_t *point)
{
   if (map_lists == NULL)
       return -1;
   if (n>=map_lists->io_slave.num_pts)
      return -1;
   *addr = &(map_lists->io_slave.pts[n].io_addr);
   *point = map_lists->io_slave.pts[n].plc_pt;
   return 0;
}

int slave_mapped_points(void) {
  if (map_lists == NULL)
    return -1;
  return map_lists->io_slave.num_pts;
}

/* get the plc point mappings from the config */
/* Hardware specific configurations will be read by the
 * io_init() function...
 */
static int parse_config(pt_map_lists_t *map_lists)
{
  char *tmp_str;
  int max_num_pts, row_count, col_count;
  int pt_len;
  dir_t pt_dir;

  pt_map_entry_t pt_map_tmp;
  pt_map_t *pt_map_ptr;

  if (debug)
    printf("parse_config(): ...\n");

  if (map_lists == NULL)
    return 0;

  /* get the in/out points */
  /* accepted syntax:
   *   map [inv | invert] {in | out} <io_addr> <matplc_point> [<matplc_point> ...]
   *
   *   inv | invert : invert the value read from the input/MatPLCpoint before storing it in the MatPLCpoint/output
   *
   *   in           : read hardware input and store value in MatPLCpoint
   *   out          : read MatPLCpoint and store value to hardware output
   *
   *   <io_addr>    : hardware input/output address - hardware specific format...
   *
   *   <matplc_point> : MatPLCpoint, or sequence of points
   *
   */
  max_num_pts = conffile_get_table_rows(IO_PTSTABLE_NAME);

  for (row_count = 0; row_count < max_num_pts; row_count++) {
    col_count = 0;

    /* check if value must be inverted */
    pt_map_tmp.inv_mask = 0x00;
    tmp_str = conffile_get_table(IO_PTSTABLE_NAME, row_count, col_count);
    if (tmp_str == NULL) {
      continue;
    } else {
      if ((strcmp(tmp_str, IO_PTSTABLE_INV1) == 0) ||
          (strcmp(tmp_str, IO_PTSTABLE_INV2) == 0)) {
	pt_map_tmp.inv_mask = 0x01;
	col_count++;
      }
    }
    free (tmp_str);

    /* check whether in | out */
    if ((tmp_str = conffile_get_table(IO_PTSTABLE_NAME, row_count, col_count++))
        == NULL)
      continue;

    if (strcmp(tmp_str, IO_PTSTABLE_OUT) == 0) {
      pt_dir = dir_out;
      pt_map_ptr = &(map_lists->io_write);
    } else
    if (strcmp(tmp_str, IO_PTSTABLE_IN) == 0) {
      pt_dir = dir_in;
      pt_map_ptr = &(map_lists->io_read);
    } else 
    if (strcmp(tmp_str, IO_PTSTABLE_SLAVE) == 0) {
      pt_dir = dir_none;
      pt_map_ptr = &(map_lists->io_slave);
    }
    else {
      /* If it is neither 'in' not 'out' not 'slave' ... */
      pt_dir = dir_none;
      pt_map_ptr = &(map_lists->io_slave_array);
      /* ...then it is something else that will need to be parsed later on! */
      col_count--;
    }
    free (tmp_str);

    /* skip the io_addr */
    col_count++;

    /* read the matplc point */
    if ((tmp_str = conffile_get_table(IO_PTSTABLE_NAME, row_count, col_count++))
        == NULL)
      continue;

    pt_map_tmp.plc_pt = plc_pt_by_name(tmp_str);

    if (pt_map_tmp.plc_pt.valid == 0) {
      plc_log_wrnmsg(1, "Invalid matplc point %s in point mapping %d. "
                        "Skipping this point.",
                     tmp_str, row_count);
      free (tmp_str);
      continue;
    }
    free (tmp_str);

    /* get the matplc point length */
    pt_len = plc_pt_len(pt_map_tmp.plc_pt);
    if (pt_len < 0) {
      plc_log_wrnmsg(1, "Could not obtain length of matplc point at "
                        "point mapping %d. "
                        "Skipping this point.",
                     row_count);
      continue;
    }

    /* read the io_addr */
    col_count-=2;
    tmp_str = conffile_get_table(IO_PTSTABLE_NAME, row_count, col_count++);
    if (io_hw_parse_io_addr(&pt_map_tmp.io_addr, tmp_str, pt_dir, pt_len) < 0) {
      plc_log_wrnmsg(1, "Invalid register %s in point mapping %d. "
                        "Skipping this point.",
                     tmp_str, row_count);
      free (tmp_str);
      continue;
    }
    free (tmp_str);

    /* add the mapping to the apropriate list... */
    if (pt_map_add(pt_map_ptr, pt_map_tmp) < 0) {
      plc_log_errmsg(1, "Not enough memory for mapping %d. "
                        "Aborting!",
                     row_count);
      return -1;
    }

  } /* for */

  return 0;
}



/* Useful function that the io_hw_parse_config() function might
 * want to use...
 *
 * Copied from Jiri's old version of the io library.
 */
plc_pt_t io_status_pt(const char *base, const char *suffix, int loglevel)
{
  char *ret_name = strdup3(base, ".", suffix);
  plc_pt_t ret = plc_pt_by_name(ret_name);

  if (!ret.valid) {
    plc_log_wrnmsg(loglevel,
                   "%1$s not found (ignored)\n"
                   "if required add %3$s:point %1$s \"...\" %2$s",
                   ret_name, plc_module_name(),
                   IO_CONF_POINT_SECTION);
    ret = plc_pt_null();
  }
  free(ret_name);  /* if (ret_name == NULL) then free() does a no-operation */
  return ret;
}





/*****************************/
#ifdef debug_dump
static int io_print_pts(pt_map_t *pts)
{
  int count;
  char *tmp_str;

  for (count = 0; count < pts->num_pts; count++) {
    plc_log_trcmsg(2, "%s %s",
                   tmp_str = io_hw_ioaddr2str(&(pts->pts[count].io_addr)),
                   (pts->pts[count].inv_mask != 0)?"inverted":"");
    free(tmp_str);
  }

  return 0;
}
#endif



#ifdef debug_dump
static int dump_config(pt_map_lists_t *map_lists)
{

  io_hw_dump_config(2);

  if (map_lists == NULL)
    return 0;

  if (map_lists->io_read.pts != NULL) {
    plc_log_trcmsg(2, "in points...(%d)\n", map_lists->io_read.num_pts);
    io_print_pts(&(map_lists->io_read));
  }
  printf("\n");

  if (map_lists->io_write.pts != NULL) {
    plc_log_trcmsg(2, "out points...(%d)\n", map_lists->io_write.num_pts);
    io_print_pts(&(map_lists->io_write));
  }
  printf("\n");

  return 0;
}
#endif




int main(int argc,char *argv[])
{
int res = 0;

  run_loop = (int(*)(void*))run_loop_master;

  if ((map_lists = malloc(sizeof(pt_map_lists_t))) == NULL) {
      printf("Out of memory. Could not allocate a measly %d bytes!\n",
             sizeof(pt_map_lists_t));
      exit(EXIT_FAILURE);
  }
  pt_map_lists_init(map_lists);

  if (plc_init(IO_MODULE_DEFAULT_NAME, argc, argv) < 0) {
      printf("Error connecting to PLC.\n");
      exit(EXIT_FAILURE);
  }

  if (io_hw_parse_config() < 0)
    exit (EXIT_FAILURE);

  if (parse_config(map_lists) < 0)
    exit (EXIT_FAILURE);

#ifdef debug_dump
  if (dump_config(map_lists) < 0)
    exit (EXIT_FAILURE);
#endif

  if (io_hw_init() < 0)
    exit (EXIT_FAILURE);

  if (run_loop(map_lists) < 0)
    res = -1;

  if (io_hw_done() < 0)
    res = -1;

  return res;
}

