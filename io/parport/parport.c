/*
 * (c) 2000 Mario de Sousa
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
 * this file implements the parport - Parallel Port IO Driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>

#include "parport.h"
#include "parport_util.h"


#include "../lib/misc/string_util.h"
#include "../lib/io/io_hw.h"

/* should it dump the config after reading it in? */
#undef debug_dump

/* definitions */

static const int debug = 0;

/********************/
/* global variables */
/********************/
ppt_regdir_t Cdir_ = regdir_none;
ppt_regdir_t Ddir_ = regdir_none;
ppt_regdir_t Sdir_ = regdir_in;

u32 base_addr_ = 0;
const char *dev_filename_ = NULL;


const char *IO_MODULE_DEFAULT_NAME = "parport";

/*************************/
/* global variable types */
/*************************/

 /* The following eneum must have the registers defined
  * in the following order:
  *     D, S, C, anything else...
  *
  * This is because int reg_arrays[reg_t] are initialised
  * in the parport.h file assuming this order!!!
  */
typedef enum {reg_D, reg_S, reg_C, reg_none} reg_t;

/* our interpretation of the generic io_addr_t */
typedef struct {
	  reg_t reg;
	  u8    bit_shift;
	  u8    mask;
	} io_addr_parport_t;





int io_hw_read(io_addr_t *io_addr, u32 *value)
{
  u8 Byte = 0;

  switch (((io_addr_parport_t *)io_addr)->reg) {
    case(reg_D): ppt_get_D(&Byte);
                 break;
    case(reg_C): ppt_get_C(&Byte);
                 break;
    case(reg_S): ppt_get_S(&Byte);
                 break;
    default:     return -1;
  } /* switch() */

  *value = (Byte & ((io_addr_parport_t *)io_addr)->mask)
           >> ((io_addr_parport_t *)io_addr)->bit_shift;

  return 0;
}


int io_hw_write(io_addr_t *io_addr, u32 value)
{
  u8 Byte = 0;
  u8 mask = ((io_addr_parport_t *)io_addr)->mask;

  switch (((io_addr_parport_t *)io_addr)->reg) {
    case(reg_D): ppt_get_D(&Byte);
                 ppt_set_D((Byte & ~mask) |
                           ((value << ((io_addr_parport_t *)io_addr)->bit_shift) & mask));
                 break;
    case(reg_C): ppt_get_C(&Byte);
                 ppt_set_C((Byte & ~mask) |
                           ((value << ((io_addr_parport_t *)io_addr)->bit_shift) & mask));
                 break;
    case(reg_S):
    default:     return -1;
  } /* switch() */

  return 0;
}


int io_hw_read_end(void) {
  return 0;
}


int io_hw_write_end(void) {
  return 0;
}


static const char *parport_reg2str(reg_t reg)
{
  if (reg == reg_C) return PARPORT_CREG;
  if (reg == reg_D) return PARPORT_DREG;
  if (reg == reg_S) return PARPORT_SREG;

  return NULL;
}


/* parse the io address */
/* accepted syntax:
 *   <reg>[.<bit>]
 *
 *   <reg> : {C | D | S}
 *   <bit> : defaults to MIN_REG_BIT
 */
int io_hw_parse_io_addr(io_addr_t *io_addr,
		 	const char *addr_str,
			dir_t dir,
			int pt_len)
{
  /* Variables used to track which bits have already been used/allocated
   * for outputing a PLC point value.
   * We shouldn't have more than one point being output to the same bit!
   *
   * As this function is called once for every io_addr, these variables have to be static.
   */
  static u8 used_bits[3] = {0x00, 0x00, 0x00};

  const u8 mask_all_one = ~0;/* make sure we have the correct length of ones */
  u32          tmp_u32;
  ppt_regdir_t *reg_dir;
  io_addr_parport_t *io_addr_ptr = (io_addr_parport_t *)io_addr;

  /* read the parport register (C, D or S) */
  if (strncmp(addr_str, PARPORT_CREG, strlen(PARPORT_CREG)) == 0) {
    io_addr_ptr->reg = reg_C;
    reg_dir = &Cdir_;
    addr_str += strlen(PARPORT_CREG);
  } else {
  if (strncmp(addr_str, PARPORT_DREG, strlen(PARPORT_DREG)) == 0) {
    io_addr_ptr->reg = reg_D;
    reg_dir = &Ddir_;
    addr_str += strlen(PARPORT_DREG);
  } else {
  if (strncmp(addr_str, PARPORT_SREG, strlen(PARPORT_SREG)) == 0) {
    io_addr_ptr->reg = reg_S;
    reg_dir = &Sdir_;
    addr_str += strlen(PARPORT_SREG);
  } else {
    plc_log_wrnmsg(1, "Invalid io_addr %s. "
                      "Could not figure out which register to use.",
                   addr_str);
    return -1;
  }
  }
  }

  /* Check if direction is correct for this register */
  if (((dir == dir_in ) && (*reg_dir == regdir_out)) ||
      ((dir == dir_out) && (*reg_dir == regdir_in ))) {
    plc_log_wrnmsg(1, "Register %s has been previously used with/set to "
                      "opposite direction. ",
                   parport_reg2str(io_addr_ptr->reg));
    return -1;
  }
  /* if the direction for this register is not yet set, then set it now */
  if (dir == dir_in ) *reg_dir = regdir_in;
  if (dir == dir_out) *reg_dir = regdir_out;

  /* parse the bit [.<bit>] */
  tmp_u32 = PARPORT_REG_DEF_BIT[io_addr_ptr->reg];

  if (addr_str[0] != '\0') {
     if (addr_str[0] != '.') {
       plc_log_wrnmsg(1, "io_addr %s has invalid syntax. "
                         "Correct syntax is {C | D | S}[.<bit>]. ",
                      parport_reg2str(io_addr_ptr->reg));
       return -1;
     }

     /* convert the bit index */
     ++addr_str; /* skip the '.' */
     if (string_str_to_u32(addr_str, &tmp_u32,
			   PARPORT_REG_MIN_BIT[io_addr_ptr->reg],
			   PARPORT_REG_MAX_BIT[io_addr_ptr->reg])
	 < 0) {
       plc_log_wrnmsg(1, "Bit %s for register %s "
                         "should be in [%d..%d]. ",
                      addr_str, parport_reg2str(io_addr_ptr->reg),
	              PARPORT_REG_MIN_BIT[io_addr_ptr->reg],
		      PARPORT_REG_MAX_BIT[io_addr_ptr->reg]);
       return -1;
    }
  }
  io_addr_ptr->bit_shift = tmp_u32;

  /* set the mask variable */
  io_addr_ptr->mask = (~(mask_all_one << pt_len)) << io_addr_ptr->bit_shift;
   /* NOTE: the following condition is required because it seems that the
    *       the compiler (or something else) is optimizing the code, and
    *       not doing the shift left (<<) when length has the maximum value.
    *       Has anybody a better explanation for what is occuring ?
    */
  if (pt_len == 8*sizeof(io_addr_ptr->mask))
    io_addr_ptr->mask = mask_all_one << io_addr_ptr->bit_shift;

  /* check if valid length */
  if (pt_len-1 + io_addr_ptr->bit_shift > PARPORT_REG_MAX_BIT[io_addr_ptr->reg]) {
    plc_log_wrnmsg(1, "PLC point has more bits (%d) than can fit in the "
                      "input/output register %s starting at bit %d.",
                   pt_len, parport_reg2str(io_addr_ptr->reg), io_addr_ptr->bit_shift);
  }

  /* check if it doesn't overlay any previously mapped points... */
  if (dir == dir_out) {
    if ((used_bits[io_addr_ptr->reg] & io_addr_ptr->mask) != 0x00) {
      plc_log_wrnmsg(1, "PLC point will be overwriting bits of the %s register "
                        "already being written to by another plc point.",
                     parport_reg2str(io_addr_ptr->reg));
    }
    /* reserve the points we are using for future checks... */
    used_bits[io_addr_ptr->reg] |= io_addr_ptr->mask;
  }


  return 0;
}





/* get hardware access method from the config file */
static int get_config(u32 * base_addr,
		      const char **dev_file_name,
		      ppt_regdir_t * Cdir,
		      ppt_regdir_t * Ddir)
{
  char *tmp_str;
  ppt_regdir_t Cdir_var = regdir_none;
  ppt_regdir_t Ddir_var = regdir_none;

  if (debug)
    printf("get_config(): ...\n");

  /* get the parallel port IO base address */
  if (base_addr != NULL) {
    if (conffile_get_value_u32(PARPORT_BASEADDR_NAME,
	  		       base_addr,
			       PARPORT_BASEADDR_MIN,
			       PARPORT_BASEADDR_MAX,
			       PARPORT_BASEADDR_DEF )
        < 0) {
      tmp_str = conffile_get_value(PARPORT_BASEADDR_NAME);
      plc_log_wrnmsg(1, "Invalid parallel port base address %s. "
                        "Using default 0x%X.",
                     tmp_str, PARPORT_BASEADDR_DEF);
      free(tmp_str);
    }
  }

  /* get the plc_parport parallel port device file name */
  if (dev_file_name != NULL) {
    if ((*dev_file_name = conffile_get_value(PARPORT_DEVFILE_NAME)) == NULL) {
	/* no need to log any message, this config. is optional... */
    }
  }

  /* Get the Direction of the D register */
  if ((tmp_str = conffile_get_value(PARPORT_DDIR_NAME)) != NULL) {
    if (strcmp(tmp_str, PARPORT_DIR_OUT) == 0) {
      Ddir_var = regdir_out;
    } else {
    if (strcmp(tmp_str, PARPORT_DIR_IN) == 0) {
      Ddir_var = regdir_in;
    } else {
      plc_log_wrnmsg(1, "Syntax error specifying direction for D register (%s). "
                        "Will use direction of mapped points to figure out "
                        "what direction to use for this register.",
                     tmp_str);
    }
    }
    free(tmp_str);
  }
  if (Ddir != NULL)
    *Ddir = Ddir_var;

  /* Get the Direction of the C register */
  if ((tmp_str = conffile_get_value(PARPORT_CDIR_NAME)) != NULL) {
    if (strcmp(tmp_str, PARPORT_DIR_OUT) == 0) {
      Cdir_var = regdir_out;
    } else {
    if (strcmp(tmp_str, PARPORT_DIR_IN) == 0) {
      Cdir_var = regdir_in;
    } else {
      plc_log_wrnmsg(1, "Syntax error specifying direction for C register (%s). "
                        "Will use direction of mapped points to figure out "
                        "what direction to use for this register.",
                     tmp_str);
    }
    }
    free(tmp_str);
  }
  if (Cdir != NULL)
    *Cdir = Cdir_var;

  /* We do not read the direction for the S register. It can only work as in. */

  return 0;
}



int io_hw_parse_config(void) {
  /* This is the first function of the io_hw library that gets called,
   * so it is probably the best place to make any consistency checks
   * between this and the generic io library.
   *
   * make sure our io_addr_parport_t struct fits inside an io_addr_t struct
   */
  if (sizeof(io_addr_t) < sizeof(io_addr_parport_t)) {
    plc_log_errmsg(1, "This I/O module has been compiled with an incompatible io library.");
    plc_log_errmsg(2, "sizeof(io_addr_t) defined in io.h differs from "
                      "sizeof(io_addr_parport_t) defined in parport.c.");
    return -1;
  }

  /* now lets really read the config... */
  return get_config(&base_addr_, &dev_filename_, &Cdir_, &Ddir_);
}


/***************************/
/*
static int parport_print_pts(pt_map_list_t *pts)
{
  int count;
  for (count = 0; count < pts->num_pts; count++) {
    printf("%s%d invert=%d\n",
           parport_reg2str(pts->pts[count].bit.reg),
           pts->pts[count].bit.bit_index,
           pts->pts[count].inv_mask);
  }

  return 0;
}
*/

/***************************/
/*
static const char *parport_regdir2str(ppt_regdir_t dir)
{
  switch(dir)  {
    case(dir_out ): return "out" ;
    case(dir_in  ): return "in"  ;
    case(dir_none): return "none";
  } // switch //

  return NULL;
}
*/

/***************************/
/*
static int dump_config(u32 base_addr,
		       const char *dev_file_name,
		       ppt_regdir_t Cdir,
		       ppt_regdir_t Ddir,
		       pt_map_list_t * in_pts, pt_map_list_t * out_pts)
{
  printf("base io_addr = %X\n", base_addr);
  printf("device file name = %s\n", dev_file_name);
  printf("C direction  = %s\n", parport_regdir2str(Cdir));
  printf("D direction  = %s\n", parport_regdir2str(Ddir));

  if (in_pts != NULL) {
    printf("in points...(%d)\n", in_pts->num_pts);
    parport_print_pts(in_pts);
  }

  printf("\n");

  if (out_pts != NULL) {
    printf("out points...(%d)\n", out_pts->num_pts);
    parport_print_pts(out_pts);
  }

  return 0;
}
*/



int io_hw_init(void)
{
int ppt_initialized = 0;

  /* Direct access to the parallel port requires root privileges.
   * If this program is suid to root, to be on the safe side we revert to
   * real user id during all program processing, and only switch back to
   * root privileges when strictly required.
   *
   * Note that only the ppt_init_dir() function requires root privileges, and it
   * tries to switch real and effective uids if it finds it does not have
   * root privileges already, so we don't have to mess aroung with uids
   * other than this switching right at the beginning of the program.
   */
  if (geteuid() == 0) /* if running suid to root */
    if (setreuid (geteuid (), getuid ()) < 0)
      plc_log_wrnmsg(1, "For some strange reason an error ocurred when"
                        "switching between effective and real user id. "
                        "This means that this module will run as root "
                        "even when not required. This may open a security "
                        "hole as this module is not designed to work that way. "
                        "It should nevetheless still work correctly...");

  /* OK. Now lets try accessing the parallel port... */
  ppt_initialized = 0;
  if (dev_filename_ != NULL) {
    /* try accessing the parport through the kernel... */
    if (ppt_init_krn(dev_filename_, Cdir_, Ddir_) < 0) {
      plc_log_wrnmsg(1, "Error initializing access to the parallel port through"
                        " device file %s. "
                        "Is the plc_parport driver "
                        "(i.e. kernel module) loaded ? "
			"Is this device being used by another process ? "
			"Reverting to direct access mode.",
			dev_filename_);
    } else {
    ppt_initialized = 1;
    }
  }

  if (ppt_initialized == 0) {
    /* try accessing the parport harware directly... */
    if (ppt_init_dir(base_addr_, Cdir_, Ddir_) < 0) {
      plc_log_errmsg(1, "Error initializing access to the parallel port "
	  	        "using base address 0x%X. "
                        "Note that this program requires root privileges to "
                        "have direct access to the parallel port.",
			base_addr_);
    } else {
    ppt_initialized = 1;
    }
  }

  if (ppt_initialized == 0)
     return -1;

  return 0;
}



int io_hw_done(void) {
  return ppt_done();
}
