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
 * this file implements the modbus IO Driver
 */


/*
 *
 *   NOTE:
 *
 *         This file will get compiled three times, each time with
 *         a different #define out of the following list:
 *
 *           MODBUS_M_RTU
 *           MODBUS_M_ASCII
 *           MODBUS_M_TCP
 *
 *
 *         Each time it gets compiled, a new modbus_m_xxx.o
 *         gets created. This .o file will later get linked with
 *         the apropriate files that implement the protocol
 *         in question.
 */

/* Check whether we are being compiled with one, and only one, of the
 * above mentioned #define's
 */
#ifndef MODBUS_M_RTU
#ifndef MODBUS_M_TCP
#ifndef MODBUS_M_ASCII
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_M_RTU -DMODBUS_M_TCP -DMODBUS_M_ASCII)
#endif
#endif
#endif

#ifdef MODBUS_M_RTU
#ifdef MODBUS_M_ASCII
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_M_RTU -DMODBUS_M_TCP -DMODBUS_M_ASCII)
#endif
#endif

#ifdef MODBUS_M_RTU
#ifdef MODBUS_M_TCP
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_M_RTU -DMODBUS_M_TCP -DMODBUS_M_ASCII)
#endif
#endif

#ifdef MODBUS_M_TCP
#ifdef MODBUS_M_ASCII
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_M_RTU -DMODBUS_M_TCP -DMODBUS_M_ASCII)
#endif
#endif


#undef MODBUS_M_SERIAL
#ifdef MODBUS_M_RTU
#define MODBUS_M_SERIAL
#endif
#ifdef MODBUS_M_ASCII
#define MODBUS_M_SERIAL
#endif




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <time.h>

#include "modbus_m.h"
#include "mb_master.h"


#include "misc/string_util.h"
#include "io/io_hw.h"

/* should it dump the config after reading it in? */
#undef debug_dump

/* definitions */

static const int debug = 0;



/*************************/
/* global variable types */
/*************************/

/* an entry in the network array */
typedef struct {
        const char      *name;
        node_addr_t     addr;
        int             nd; /* the node descriptor... */
        struct timespec timeout;
        u32             send_retries;
        } network_t;

typedef struct {
        network_t *net;
        int count;
        } network_array_t;

/* our interpretation of the generic io_addr_t */
typedef struct {
	  u8  function_code;
	    /* NOTE: function code contains the modbus frame function code
	     *       to be used.
	     *       Modbus function codes always have bit 6 set to 0, so just
	     *       like the modbus spec, we re-use that bit to indicate that
	     *       the last communication attempt produced an error.
	     *       (i.e. if we had an error, bit 6 will be set, and therefore
	     *        function_code & 0x80 will be != 0)
	     *       This info will be used to determine the log level of
	     *       subsequent error messages.
	     *       Normally, it will have bit 6 set to 0, i.e.
	     *       function_code & 0x80 will be == 0.
	     */
          u8  slave;
	  u16 address;
	  u16 count;
          u16 nw_array_ofs;
	} io_addr_modbus_t;


/***************************/
/* global static variables */
/***************************/

const char *IO_MODULE_DEFAULT_NAME = DEF_MODULE_NAME;

static network_array_t nw_array_ = {NULL, 0};


/************************************/
/**                                **/
/** Miscelaneous Utility functions **/
/**                                **/
/************************************/

/* Function to load a struct timeval correctly
 * from a double.
 */
static inline struct timespec d_to_timespec(double time) {
  struct timespec tmp;

  tmp.tv_sec  = time;
  tmp.tv_nsec = 1e9*(time - tmp.tv_sec);
  return tmp;
}



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****     C o n f i g.   D u m p    F u n c t i o n s      ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


#ifdef debug_dump
int io_hw_dump_config(int debug_level) {

  /* TODO: Dump the nw_array_ to plc_log_trcmag()... */
  return 0;
}


char *io_hw_ioaddr2str(io_addr_t *io_addr) {
  char *tmp_str = NULL;
  int res, str_size = 0;
  io_addr_modbus_t *io_addr_modbus = (io_addr_modbus_t *)io_addr;

  str_size = snprintf(tmp_str, str_size,
                      "function:%d slave:%d address:%d count:%d array_ofs:%d",
                      io_addr_modbus->function_code,
                      io_addr_modbus->slave,
                      io_addr_modbus->address,
                      io_addr_modbus->count,
                      io_addr_modbus->nw_array_ofs);

  if (str_size <= 0)
    return NULL;

  if ((tmp_str = malloc(str_size + 1)) == NULL)
    return NULL;

  res = snprintf(tmp_str, str_size+1,
                 "function:%d slave:%d address:%d count:%d array_ofs:%d",
                 io_addr_modbus->function_code,
                 io_addr_modbus->slave,
                 io_addr_modbus->address,
                 io_addr_modbus->count,
                 io_addr_modbus->nw_array_ofs);

  if (res != str_size) {
    free(tmp_str);
    return NULL;
  }
  tmp_str[str_size] = '\0';

  return tmp_str;
}
#endif


/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****   H a r d w a r e   A c c e s s   F u n c t i o n s  ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

static const char *function_code_2_str(u8 function_code) {
  switch(function_code) {
    case MODBUS_READ_INBIT_CODE   : return(MODBUS_INBIT);
    case MODBUS_READ_INWORD_CODE  : return(MODBUS_INWORD);
    case MODBUS_WRITE_OUTBIT_CODE : return(MODBUS_OUTBIT);
    case MODBUS_READ_OUTBIT_CODE  : return(MODBUS_OUTBIT);
    case MODBUS_READ_OUTWORD_CODE : return(MODBUS_OUTWORD);
    case MODBUS_WRITE_OUTWORD_CODE: return(MODBUS_OUTWORD);
    default  : return NULL;
  }

  return NULL;
}


static inline const char *error_code_2_str(u8 error_code) {
  if (error_code <= ERROR_CODE_MSG_MAX)
    return ERROR_CODE_MSG[error_code];

  return UNKNOWN_ERROR_MSG;
}


static int log_error_msg(io_addr_modbus_t *io_addr_modbus,
			 int res,
			 u8 error_code) {
  int log_level;
  const char *msg = NULL;
  const char *error_msg = NULL;

  if (res >= 0) {
    /* Comms was succesfull... */
    if ((io_addr_modbus->function_code & 0x80) != 0) {
      /* we had an error previously, so we warn that connection is back up... */
      plc_log_errmsg(FIRST_ERROR_LOG_LEVEL,
    		     "Comunications to %s.%d.%s.%d is back up",
  	 	     nw_array_.net[io_addr_modbus->nw_array_ofs].name,
		     io_addr_modbus->slave,
		     function_code_2_str(io_addr_modbus->function_code & ~0x80),
		     io_addr_modbus->address);
    }

    /* now reset the error flags... */
    io_addr_modbus->function_code &= ~0x80;

    /* then return success */
    return 0;
  }

  /* If we reach this point, its because we had a comms error... */
  if ((io_addr_modbus->function_code & 0x80) == 0)
    /* this was the first error */
    log_level = FIRST_ERROR_LOG_LEVEL;
  else
    log_level = SUBSEQUENT_ERROR_LOG_LEVEL;

  /* remember an error occured! */
  io_addr_modbus->function_code |= 0x80;

  switch(res) {
    case PORT_FAILURE:
      msg = "Error accessing comunications port, or other internal error while "
	    "connecting to %s.%d.%s.%d";
      break;
    case TIMEOUT:
      msg = "Timeout while attempting to connect to %s.%d.%s.%d";
      break;
    case INVALID_FRAME:
      msg = "Received an invalid frame in response to connection to %s.%d.%s.%d";
      break;
    case MODBUS_ERROR:
      msg = "Attempt to connect to %s.%d.%s.%d resulted in a reply from the slave "
            "with error code %d (%s)";
      error_msg=error_code_2_str(error_code);
      break;
    default:
      msg = "Unknown error while connecting to %s.%d.%s.%d";
  } /* switch() */

  plc_log_errmsg(log_level, msg,
  	 	 nw_array_.net[io_addr_modbus->nw_array_ofs].name,
		 io_addr_modbus->slave,
		 function_code_2_str(io_addr_modbus->function_code & ~0x80),
		 io_addr_modbus->address,
		 error_code, error_msg);

  return -1;
}


#define mb_master_call8(f) f(io_addr_modbus->slave,                                   \
                             io_addr_modbus->address,                                 \
                             io_addr_modbus->count,                                   \
                             value,                                                   \
                             nw_array_.net[io_addr_modbus->nw_array_ofs].nd,          \
                             nw_array_.net[io_addr_modbus->nw_array_ofs].send_retries,\
			     &error_code,                                             \
			     &nw_array_.net[io_addr_modbus->nw_array_ofs].timeout)


int io_hw_read(io_addr_t *io_addr, u32 *value) {
  int res;
  u8 error_code;
  io_addr_modbus_t *io_addr_modbus = (io_addr_modbus_t *)io_addr;

  switch(io_addr_modbus->function_code & ~0x80) {
    case MODBUS_READ_OUTBIT_CODE:  res=mb_master_call8(read_output_bits_u32); break;
    case MODBUS_READ_INBIT_CODE:   res=mb_master_call8(read_input_bits_u32);  break;
    case MODBUS_READ_OUTWORD_CODE: res=mb_master_call8(read_output_words_u32);break;
    case MODBUS_READ_INWORD_CODE:  res=mb_master_call8(read_input_words_u32); break;
    /* Does not make sense to write when a read is requested...
     * The following should never occur, as it is checked for
     * in parse_io_hw_addr()...
     */
    /*
    case MODBUS_WRITE_OUTBIT_CODE:  res=mb_master_call8(write_output_bits_u32); break;
    case MODBUS_WRITE_OUTWORD_CODE: res=mb_master_call8(write_output_words_u32);break;
    */
    default  : return -1;
  }

  return log_error_msg(io_addr_modbus, res, error_code);
}


int io_hw_write(io_addr_t *io_addr, u32 val) {
  int res;
  u8 error_code;
  io_addr_modbus_t *io_addr_modbus = (io_addr_modbus_t *)io_addr;
  u32 *value = &val;

  switch(io_addr_modbus->function_code & ~0x80) {
    /* Does not make sense to read when a write is requested...
     * The following should never occur, as it is checked for
     * in parse_io_hw_addr()...
     */
    /*
    case MODBUS_READ_OUTBIT_CODE:  res=mb_master_call8(read_output_bits_u32); break;
    case MODBUS_READ_INBIT_CODE:   res=mb_master_call8(read_input_bits_u32);  break;
    case MODBUS_READ_OUTWORD_CODE: res=mb_master_call8(read_output_words_u32);break;
    case MODBUS_READ_INWORD_CODE:  res=mb_master_call8(read_input_words_u32); break;
    */
    case MODBUS_WRITE_OUTBIT_CODE:  res=mb_master_call8(write_output_bits_u32); break;
    case MODBUS_WRITE_OUTWORD_CODE: res=mb_master_call8(write_output_words_u32);break;
    default  : return -1;
  }

  return log_error_msg(io_addr_modbus, res, error_code);
}


int io_hw_read_end(void) {
  return 0;
}


int io_hw_write_end(void) {
  return mb_master_silence_init();
}




/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****          P a r s e    I O    A d d r e s s           ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

/* Search through the nw_array_ for a network, and return it's ntework id. */
static int get_network_id(const char *nw_name) {
  int count;

  for(count = 0; count < nw_array_.count; count++) {
    if ((nw_name != NULL) && (nw_array_.net[count].name != NULL)) {
      if (strcmp(nw_name, nw_array_.net[count].name) == 0)
        return count;
    }
  }

  /* not found */
  return -1;
}


/* Search through the slave addr (a.k.a. node address) table,
 * for the address of the *slave_name node.
 *
 * accepted syntax:
 *
 * node <slave> <network_name> <slave_addr>
 *
 *   <slave>      : the name of the slave node
 *   <slave_addr> : 0..255
 */
static int get_slave_addr(char *slave_name, u16 *nw_id, u8 *slave_addr) {
  char *row_name, *nw_name;
  int row_count, tmp_int;
  u32 tmp_u32;
  int row_max = conffile_get_table_rows(MODBUS_SLAVE_TABLE);

  if (slave_name == NULL)
    return -1;

  for(row_count = 0; row_count < row_max; row_count++) {
    row_name = conffile_get_table(MODBUS_SLAVE_TABLE, row_count, MODBUS_SLAVE_NAME_OFS);
    if (row_name != NULL) {
      if (strcmp(slave_name, row_name) == 0) {
        /* Found it!!  */

        /* once we get inside this if(), then
         * we will surely return,
         * if not success, then an error,
         * so we must not forget to free the memory...
         */
        free(row_name);

        /* Let's read the network name... */
        nw_name = conffile_get_table(MODBUS_SLAVE_TABLE, row_count, MODBUS_SLAVE_NETW_OFS);
        if (nw_name == NULL) {
          plc_log_errmsg(1, "Invalid ...");
          return -1;
        }

        /* Let's search for the network data... */
        tmp_int = get_network_id(nw_name);
        free(nw_name);
        if (tmp_int < 0) {
          return -1;
        }
        *nw_id = tmp_int;

        /* Let's read the slave address... */
        tmp_u32 = MODBUS_SLAVE_ADDR_DEF;
        if (conffile_get_table_u32(MODBUS_SLAVE_TABLE,
                                   row_count, MODBUS_SLAVE_ADDR_OFS,
                                   &tmp_u32,
                                   MODBUS_SLAVE_ADDR_MIN, MODBUS_SLAVE_ADDR_MAX,
                                   0 /* We do accept default value! */)
            < 0) {
          plc_log_errmsg(1, "Slave %s id should be in [%d..%d]. "
                            "Ignoring this slave...",
                         slave_name, MODBUS_SLAVE_ADDR_MIN, MODBUS_SLAVE_ADDR_MAX);
          return -1;
        }
        *slave_addr = tmp_u32;

        /* success */
        return 0;
      }
      free(row_name);
    }
  }

  /* not found...*/
  return -1;
}


/* parse the io address */
/* accepted syntax:
 *   <slave>.<reg_type>.<address>
 *
 *   <slave>     : The name of the slave as it appears in the node table.
 *   <reg_type>  : in_bit | out_bit | in_word | out_word
 *   <address>   : 1..10000
 */
int io_hw_parse_io_addr(io_addr_t *io_addr,
		 	const char *const_addr_str,
			dir_t dir,
			int pt_len)
{
  /* Variables used to track which bits have already been used/allocated
   * for outputing a PLC point value.
   * We shouldn't have more than one point being output to the same bit!
   *
   * As this function is called once for every io_addr, these variables have to be static.
   */
  /*
     NOTE: we do not yet iomplement this protection against user errors...
           let's keep it simple for now.

  static u16 used_bits[...] = {0x00, 0x00, 0x00, ...};
  */

  int    modbus_pt_len = 0;
  u32    tmp_u32;
  char  *tmp_str, *ptrptr, *addr_str, *org_addr_str;
  dir_t  reg_dir;
  io_addr_modbus_t *io_addr_ptr = (io_addr_modbus_t *)io_addr;

  if (const_addr_str == NULL)
    return -1;

  if (const_addr_str[0] == '\0')
    return -1;

 /* We will be calling strtok_r() which will corrupt the addr_str string.
   * Since addr_str is a const char *, we must therefore make a copy
   * of this string and work on the copy...
   */
  addr_str = strdup(const_addr_str);
  org_addr_str = addr_str;
  if (addr_str == NULL) {
    plc_log_errmsg(1, "Not enough memory to duplicate a string. Aborting...");
    goto error_exit_0;
  }

  /*********************/
  /* parse the <slave> */
  /*********************/
  if ((tmp_str = strtok_r(addr_str, ".", &ptrptr)) == NULL) {
    /* <slave> not found! */
    plc_log_wrnmsg(1, "Invalid modbus address %s. "
                      "Should be <slave>.<location>.<address>",
                      addr_str);
    goto error_exit_1;
  }

    /* search for the <slave> in the slave address table */
  if (get_slave_addr(tmp_str, &io_addr_ptr->nw_array_ofs, &io_addr_ptr->slave) < 0) {
    /* Not on the slave address table. */
    plc_log_wrnmsg(1, "Unknown slave %s.", tmp_str);
    goto error_exit_1;
  }


  /************************/
  /* parse the <location> */
  /************************/
  if ((tmp_str = strtok_r(NULL, ".", &ptrptr)) == NULL) {
    /* <location> not found! */
    plc_log_wrnmsg(1, "Invalid modbus address %s. "
                      "Should be <slave>.<location>.<address>",
                      addr_str);
    goto error_exit_1;
  }

  /* stop the compiler from throwing out a warning... */
  reg_dir = dir_out;

  if (strcmp(tmp_str, MODBUS_INBIT) == 0) {
     /* INBITs can only be read. If the user is trying to write to it,
        we catch the error later on...
      */
    io_addr_ptr->function_code = MODBUS_READ_INBIT_CODE;
    modbus_pt_len = 1;
    reg_dir = MODBUS_INBIT_DIR;
  } else {
  if (strcmp(tmp_str, MODBUS_OUTBIT) == 0) {
    if (dir == dir_in)
      io_addr_ptr->function_code = MODBUS_READ_OUTBIT_CODE;
    else
      io_addr_ptr->function_code = MODBUS_WRITE_OUTBIT_CODE;

    modbus_pt_len = 1;
    reg_dir = MODBUS_OUTBIT_DIR;
  } else {
  if (strcmp(tmp_str, MODBUS_INWORD) == 0) {
     /* INBITs can only be read. If the user is trying to write to it,
        we catch the error later on...
      */
    io_addr_ptr->function_code = MODBUS_READ_INWORD_CODE;
    modbus_pt_len = 16;
    reg_dir = MODBUS_INWORD_DIR;
  } else {
  if (strcmp(tmp_str, MODBUS_OUTWORD) == 0) {
    if (dir == dir_in)
      io_addr_ptr->function_code = MODBUS_READ_OUTWORD_CODE;
    else
      io_addr_ptr->function_code = MODBUS_WRITE_OUTWORD_CODE;

    modbus_pt_len = 16;
    reg_dir = MODBUS_OUTWORD_DIR;
  } else {
    plc_log_wrnmsg(1, "Invalid io_addr %s. "
                      "Could not figure out which register to use.",
                   addr_str);
    goto error_exit_1;
  }
  }
  }
  }

  /* Check if direction is correct for this register */
  if ((dir == dir_out) && (reg_dir == dir_in )) {
    /* trying to write to an input!! */
    plc_log_wrnmsg(1, "Cannot write to an input! (%s)",
                   addr_str);
    goto error_exit_1;
  }

  /************************/
  /* parse the .<address> */
  /************************/
  if ((tmp_str = strtok_r(NULL, ".", &ptrptr)) == NULL) {
    /* <address> not found! */
    plc_log_wrnmsg(1, "Invalid modbus address %s. "
                      "Should be <slave>.<location>.<address>",
                      addr_str);
    goto error_exit_1;
  }

    /* convert the <address> string */
  if (string_str_to_u32(tmp_str, &tmp_u32,
			MODBUS_REG_ADDR_MIN,
			MODBUS_REG_ADDR_MAX)
	 < 0) {
    plc_log_wrnmsg(1, "Address %s should be in [%d..%d]. ",
                      tmp_str, MODBUS_REG_ADDR_MIN, MODBUS_REG_ADDR_MAX);
    goto error_exit_1;
  }
  io_addr_ptr->address = tmp_u32;

  /* Make sure there are no more characters to parse... */
  if ((tmp_str = strtok_r(NULL, "", &ptrptr)) != NULL) {
    plc_log_wrnmsg(1, "Invalid modbus address %s. "
                      "Should be <slave>.<location>.<address>",
                      addr_str);
    goto error_exit_1;
  }

  /* Determine the number of bits/words that need to be transferred... */
  /* NOTE: we are using integer division! */
  io_addr_ptr->count = ((pt_len-1) / modbus_pt_len) + 1;

  return 0;

error_exit_1:
  free(org_addr_str);
error_exit_0:
  return -1;
}


/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****     P a r s e    H a r d w a r e    C o n f i g.     ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


/* not used by the TCP version */
#ifndef MODBUS_M_TCP
/* small helper function for get_config() */
static void get_config_i32(i32 *value, i32 min, i32 max,
                           const char *name,
                           const char *value_str) {
  /* get the ??? */
  if (value != NULL) {
    if (string_str_to_i32(value_str, value, min, max) < 0)
      plc_log_wrnmsg(1, "Parameter %s has invalid value %s. "
                        "Reverting to default %d.",
                     name, value_str, *value);
  }
}
#endif


/* small helper function for get_config() */
static void get_config_u32(u32 *value, u32 min, u32 max,
                           const char *name,
                           const char *value_str) {
  /* get the ??? */
  if (value != NULL) {
    if (string_str_to_u32(value_str, value, min, max) < 0)
      plc_log_wrnmsg(1, "Parameter %s has invalid value %s. "
                        "Reverting to default %d.",
                     name, value_str, *value);
  }
}


/* small helper function for get_config() */
static void get_config_d(double *value, double min, double max,
                         const char *name,
                         const char *value_str) {
  /* get the ??? */
  if (value != NULL) {
    if (string_str_to_d(value_str, value, min, max) < 0)
      plc_log_wrnmsg(1, "Parameter %s has invalid value %s. "
                        "Reverting to default %f.",
                     name, value_str, *value);
  }
}


/* small helper function for get_config() */
static void get_config_str(i32 *value, int (*f)(const char *),
                           const char *def,
                           const char *name,
                           const char *value_str) {
  /* get the ??? */
  if (value != NULL) {
    if (f(value_str) < 0)
      plc_log_wrnmsg(1, "Parameter %s has invalid value %s. "
                        "Reverting to default %s.",
                     name, value_str, def);
    else
      *value = f(value_str);
  }
}


/*************************************************/
/**                                             **/
/**   Parse Serial Network Config (RTU/Ascii)   **/
/**                                             **/
/*************************************************/

#ifdef MODBUS_M_SERIAL
/* small helper function for get_config() */
static int parity_str2int(const char *str) {

       if (strcasecmp(str, MODBUS_PARITY_NONE) == 0) return 0;
  else if (strcasecmp(str, MODBUS_PARITY_ODD ) == 0) return 1;
  else if (strcasecmp(str, MODBUS_PARITY_EVEN) == 0) return 2;

  return -1;
}


/* small helper function for get_config() */
static int ignore_echo_str2int(const char *str) {

       if (strcasecmp(str, MODBUS_IGNORE_ECHO_FALSE) == 0) return 0;
  else if (strcasecmp(str, MODBUS_IGNORE_ECHO_TRUE ) == 0) return 1;

  return -1;
}


/* Parse the one line of the network configuration table */
/*
 * accepted syntax:
 *   network <nw_name> {rtu | ascii} [options...]
 *
 *   options:
 *      [device <file_name>]
 *      [baudrate <xx>]
 *      [parity {none | odd | even}]
 *      [data_bits {7 | 8}]
 *      [stop_bits {1 | 2}]
 */
static int init_network_data(int row, network_t *nw) {
  char  *tmp_str, *tmp2_str;
  const char *tmp_cstr;
  double tmp_dbl;
  int    rowlen, index;

#ifdef MODBUS_M_RTU
  node_addr_rtu_t *addr = &(nw->addr.addr.rtu);
  nw->addr.naf = naf_rtu;
  tmp_cstr = MODBUS_NETWORK_RTU_TYPE;
#else
#ifdef MODBUS_M_ASCII
  node_addr_ascii_t *addr = &(nw->addr.addr.ascii);
  nw->addr.naf = naf_ascii;
  tmp_cstr = MODBUS_NETWORK_ASCII_TYPE;
#else
#error This program has a bug! This error should never occur...
#endif
#endif

  if (debug)
    printf("init_network_data(RTU/ASCII): ...\n");

  /* not connected yet... */
  nw->nd = -1;

  /* check whether the row has the correct network type... */
  tmp_str = conffile_get_table(MODBUS_NETWORK_TABLE, row, MODBUS_NETWORK_TYPE_OFS);
  if (strcmp(tmp_str, tmp_cstr) != 0) {
    /* strange, this should not occur... */
    free(tmp_str);
    return -1;
  }
  free(tmp_str);

  /* get the network name... */
  nw->name = conffile_get_table(MODBUS_NETWORK_TABLE, row, MODBUS_NETWORK_NAME_OFS);
  if (nw->name == NULL)
    /* This will never occur, since if we have a network type, we must surely
     * also have a network name (the name comes before the type).
     * Nevertheless, there seems little harm in checking for it anyway,
     * just to make sure that any strange bug get's caught early on, and we don't
     * let our local data structures get into an undesirable state.
     */
    return -1;

  /* setup the default parameters */
  nw->timeout       = d_to_timespec(MODBUS_TIMEOUT_DEF);
  nw->send_retries  = MODBUS_SENDRETRIES_DEF;
  addr->device      = MODBUS_DEVFILE_DEF;
  addr->baud        = MODBUS_BAUDRATE_DEF;
  addr->parity      = parity_str2int(MODBUS_PARITY_DEF);
  addr->data_bits   = MODBUS_DATABITS_DEF;
  addr->stop_bits   = MODBUS_STOPBITS_DEF;
  addr->ignore_echo = ignore_echo_str2int(MODBUS_IGNORE_ECHO_DEF);

  /*****************************/
  /* Parse the parameter pairs */
  /*****************************/
  /* NOTE: we will do this in two passes.
   *       The first pass will read the serial line configuration parameters
   *       The second pass will read thje Timeout and Send_Retries.
   *
   *       This is because we need to know the serial line parameters
   *       before we can read the timeout.
   */

  /* get the number of elements in the row... */
  rowlen = conffile_get_table_rowlen(MODBUS_NETWORK_TABLE, row);

  /* First Pass */
  for (index = 0;
       index < rowlen;
       index++) {

    tmp_str  = conffile_get_table(MODBUS_NETWORK_TABLE, row, 2 + index*2);
    tmp2_str = conffile_get_table(MODBUS_NETWORK_TABLE, row, 3 + index*2);

    if (tmp2_str == NULL) {
      if (tmp_str != NULL) {
        plc_log_wrnmsg(1, "Missing value for parameter %s.", tmp_str);
        free(tmp_str);
      }
      break;
    }

    if      (strcmp(tmp_str, MODBUS_DEVFILE_NAME)     == 0)
      addr->device = strdup(tmp2_str);
    else if (strcmp(tmp_str, MODBUS_BAUDRATE_NAME)    == 0)
      get_config_i32(&addr->baud, MODBUS_BAUDRATE_MIN, MODBUS_BAUDRATE_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_PARITY_NAME)      == 0)
      get_config_str(&addr->parity, parity_str2int,
                     MODBUS_PARITY_DEF, tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_DATABITS_NAME)    == 0)
      get_config_i32(&addr->data_bits, MODBUS_DATABITS_MIN, MODBUS_DATABITS_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_STOPBITS_NAME)    == 0)
      get_config_i32(&addr->stop_bits, MODBUS_STOPBITS_MIN, MODBUS_STOPBITS_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_IGNORE_ECHO_NAME) == 0)
      get_config_str(&addr->ignore_echo, ignore_echo_str2int,
                     MODBUS_IGNORE_ECHO_DEF, tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_SENDRETRIES_NAME) == 0) {}
    else if (strcmp(tmp_str, MODBUS_TIMEOUT_NAME) == 0) {}
    else {
      plc_log_wrnmsg(1,
                     "Invalid parameter %s, "
                     "skipping this parameter.",
                     tmp_str);
    }

    /* go to next parameter... */
    free(tmp_str);
    free(tmp2_str);
  } /* for(..) */

  /* Second Pass */
  for (index = 0;
       index < rowlen;
       index++) {

    tmp_str  = conffile_get_table(MODBUS_NETWORK_TABLE, row, 2 + index*2);
    tmp2_str = conffile_get_table(MODBUS_NETWORK_TABLE, row, 3 + index*2);

    if ((tmp2_str == NULL) || (tmp_str == NULL)) {
      free(tmp_str);  /* it's OK to free a NULL */
      free(tmp2_str); /* it's OK to free a NULL */
      break;
    }

    if      (strcmp(tmp_str, MODBUS_SENDRETRIES_NAME) == 0)
      get_config_u32(&nw->send_retries, MODBUS_SENDRETRIES_MIN, MODBUS_SENDRETRIES_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_TIMEOUT_NAME) == 0) {
      double min_timeout;
      get_config_d  (&tmp_dbl, MODBUS_TIMEOUT_MIN, MODBUS_TIMEOUT_MAX,
                     tmp_str, tmp2_str);
      /* handle the timeout value... */
      min_timeout = mb_get_min_timeout(addr->baud, addr->parity,
                                       addr->data_bits, addr->stop_bits);
      if (tmp_dbl < min_timeout) {
        plc_log_wrnmsg(1, "Configured timeout (%f) is smaller than the time required "
                          "to receive the larger modbus frames (%f), "
                          "which may therefore be lost.",
                       tmp_dbl, min_timeout);
      }
      nw->timeout = d_to_timespec(tmp_dbl);
    }

    /* go to next parameter... */
    free(tmp_str);
    free(tmp2_str);
  } /* for(..) */

  return 0;
}
#endif /* MODBUS_M_SERIAL */


/*************************************************/
/**                                             **/
/**    Parse TCP Network Config (RTU/Ascii)     **/
/**                                             **/
/*************************************************/

#ifdef MODBUS_M_TCP
/* small helper function for get_config() */
static int close_on_silence_str2int(const char *str) {

       if (strcasecmp(str, MODBUS_CLOSEONSILENCE_FALSE) == 0)
         return 0;
  else if (strcasecmp(str, MODBUS_CLOSEONSILENCE_TRUE ) == 0)
         return 1;

  return -1;
}


static int init_network_data(int row, network_t *nw) {
  char  *tmp_str, *tmp2_str;
  double tmp_dbl;
  int    rowlen, index;

  node_addr_tcp_t *addr = &(nw->addr.addr.tcp);
  nw->addr.naf = naf_tcp;

  if (debug)
    printf("init_network_data(TCP): ...\n");

  /* not connected yet... */
  nw->nd = -1;

  /* check whether the row has the correct network type... */
  tmp_str = conffile_get_table(MODBUS_NETWORK_TABLE, row, MODBUS_NETWORK_TYPE_OFS);
  if (strcmp(tmp_str, MODBUS_NETWORK_TCP_TYPE) != 0) {
    /* strange, this should not occur... */
    free(tmp_str);
    return -1;
  }
  free(tmp_str);

  /* get the network name... */
  nw->name = conffile_get_table(MODBUS_NETWORK_TABLE, row, MODBUS_NETWORK_NAME_OFS);
  if (nw->name == NULL)
    /* This will never occur, since if we have a network type, we must surely
     * also have a network name (the name comes before the type).
     * Nevertheless, there seems little harm in checking for it anyway,
     * just to make sure that any strange bug get's caught early on, and we don't
     * let our local data structures get into an undesirable state.
     */
    return -1;

  /* setup the default parameters */
  nw->timeout            = d_to_timespec(MODBUS_TIMEOUT_DEF);
  nw->send_retries       = MODBUS_SENDRETRIES_DEF;
  addr->host             = MODBUS_HOSTNAME_DEF;
  addr->service          = MODBUS_SERVICE_DEF;
  addr->close_on_silence = close_on_silence_str2int(MODBUS_CLOSEONSILENCE_DEF);

  /* get the number of elements in the row... */
  rowlen = conffile_get_table_rowlen(MODBUS_NETWORK_TABLE, row);

  /* First and only Pass */
  for (index = 0;
       index < rowlen;
       index++) {

    tmp_str  = conffile_get_table(MODBUS_NETWORK_TABLE, row, 2 + index*2);
    tmp2_str = conffile_get_table(MODBUS_NETWORK_TABLE, row, 3 + index*2);

    if (tmp2_str == NULL) {
      if (tmp_str != NULL) {
        plc_log_wrnmsg(1, "Missing value for parameter %s.", tmp_str);
        free(tmp_str);
      }
      break;
    }

    if      (strcmp(tmp_str, MODBUS_HOSTNAME_NAME) == 0)
      addr->host = strdup(tmp2_str);
    else if (strcmp(tmp_str, MODBUS_SERVICE_NAME) == 0)
      addr->service = strdup(tmp2_str);
    else if (strcmp(tmp_str, MODBUS_SENDRETRIES_NAME) == 0)
      get_config_u32(&nw->send_retries, MODBUS_SENDRETRIES_MIN, MODBUS_SENDRETRIES_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_TIMEOUT_NAME) == 0) {
      get_config_d  (&tmp_dbl, MODBUS_TIMEOUT_MIN, MODBUS_TIMEOUT_MAX,
                     tmp_str, tmp2_str);
      nw->timeout = d_to_timespec(tmp_dbl);
    }
    else if (strcmp(tmp_str, MODBUS_CLOSEONSILENCE_NAME) == 0)
      get_config_str(&addr->close_on_silence, close_on_silence_str2int,
                     MODBUS_CLOSEONSILENCE_DEF, tmp_str, tmp2_str);
    else {
      plc_log_wrnmsg(1,
                     "Invalid parameter %s, "
                     "skipping this parameter.",
                     tmp_str);
    }

    /* go to next parameter... */
    free(tmp_str);
    free(tmp2_str);
  } /* for(..) */

  return 0;
}

#endif /* MODBUS_M_TCP */



/*************************************************/
/**                                             **/
/**    Parse Network Table                      **/
/**                                             **/
/*************************************************/


static int init_network_array(const char *nw_type) {
  char *tmp_string;
  int row_count, nw_count;
  int row_max = conffile_get_table_rows(MODBUS_NETWORK_TABLE);

  /* first figure out how many networks of the correct type... */
  nw_count = 0;
  for(row_count = 0; row_count < row_max; row_count++) {
    tmp_string = conffile_get_table(MODBUS_NETWORK_TABLE, row_count,
                                    MODBUS_NETWORK_TYPE_OFS);
    if (tmp_string != NULL) {
      if (strcmp(tmp_string, nw_type) == 0) {
        nw_count++;
      } else {
        plc_log_wrnmsg(1, "Invalid network type %s. Ignoring this network,",
                       tmp_string);
      }
      /* don't forget to free the memory...*/
      free(tmp_string);
    }
  }

  /* allocate the array to store the parsed config data... */
  nw_array_.net = malloc(nw_count * sizeof(network_t));
  if (nw_array_.net == NULL)
    return -1;
  nw_array_.count = nw_count;

  /* parse and load the network config. data */
  nw_count = 0;
  for(row_count = 0; row_count < row_max; row_count++) {
    tmp_string = conffile_get_table(MODBUS_NETWORK_TABLE, row_count,
                                    MODBUS_NETWORK_TYPE_OFS);
    if (tmp_string != NULL) {
      if (strcmp(tmp_string, nw_type) == 0) {
        if (init_network_data(row_count, &nw_array_.net[nw_count]) >= 0)
          nw_count++;
      }
      /* don't forget to free the memory...*/
      free(tmp_string);
    }
  }
  /* the actual number of valid networks... */
  nw_array_.count = nw_count;

  return 0;
}


/*************************************************/
/**                                             **/
/**    Parse Hardware Config                    **/
/**                                             **/
/*************************************************/


int io_hw_parse_config(void) {
#ifdef MODBUS_M_TCP
  const char *nw_type = MODBUS_NETWORK_TCP_TYPE;
#else
#ifdef MODBUS_M_RTU
  const char *nw_type = MODBUS_NETWORK_RTU_TYPE;
#else
#ifdef MODBUS_M_ASCII
  const char *nw_type = MODBUS_NETWORK_ASCII_TYPE;
#else
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_M_RTU -DMODBUS_M_TCP -DMODBUS_M_ASCII)
#endif
#endif
#endif

  /* This is the first function of the io_hw library that gets called,
   * so it is probably the best place to make any consistency checks
   * between this and the generic io library.
   *
   * make sure our io_addr_modbus_t struct fits inside an io_addr_t struct
   */
  if (sizeof(io_addr_t) < sizeof(io_addr_modbus_t)) {
    plc_log_errmsg(1, "This I/O module has been compiled with an incompatible io library.");
    plc_log_errmsg(2, "sizeof(io_addr_t) defined in io.h differs from "
                      "sizeof(io_addr_modbus_t) defined in modbus.c.");
    return -1;
  }

  /* load the network address array... */
  if (init_network_array(nw_type) < 0)
    return -1;

  return 0;
}




/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****       I n i t i a l i a s e     H a r d w a r e      ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


int io_hw_init(void)
{
  int count;

  if (mb_master_init(nw_array_.count) < 0)
    return -1;

  for (count = 0; count < nw_array_.count; count ++) {
    if ((nw_array_.net[count].nd = mb_master_connect(nw_array_.net[count].addr)) < 0) {
      /* could not connect to slave! */
      plc_log_errmsg(1, "Error connecting to network %s.",
                     nw_array_.net[count].name);
    }
  }

  return 0;
}



int io_hw_done(void) {
  return mb_master_done();
}
