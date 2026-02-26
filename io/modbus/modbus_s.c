/*
 * (c) 2001 Mario de Sousa
 * (c) 2003 Andrey Romanenko
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
#ifndef MODBUS_S_RTU
#ifndef MODBUS_S_TCP
#ifndef MODBUS_S_ASCII
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_S_RTU -DMODBUS_S_TCP -DMODBUS_S_ASCII)
#endif
#endif
#endif

#ifdef MODBUS_S_RTU
#ifdef MODBUS_S_ASCII
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_S_RTU -DMODBUS_S_TCP -DMODBUS_S_ASCII)
#endif
#endif

#ifdef MODBUS_S_RTU
#ifdef MODBUS_S_TCP
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_S_RTU -DMODBUS_S_TCP -DMODBUS_S_ASCII)
#endif
#endif

#ifdef MODBUS_S_TCP
#ifdef MODBUS_S_ASCII
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_S_RTU -DMODBUS_S_TCP -DMODBUS_S_ASCII)
#endif
#endif


#undef MODBUS_S_SERIAL
#ifdef MODBUS_S_RTU
#define MODBUS_S_SERIAL
#endif
#ifdef MODBUS_S_ASCII
#define MODBUS_S_SERIAL
#endif




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <time.h>
#include <netinet/in.h>

#include "plc.h"
#include "modbus_s.h"
#include "mb_slave.h"
#include "mb_layer1.h"


#include "misc/string_util.h"
#include "io/io.h"

/* should it dump the config after reading it in? */
//#define debug_dump

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
        int             slave_address;
        int             connections;
        int             mode;
        } network_t;

typedef struct {
        network_t *net;
        int count;
        } network_array_t;

/* our interpretation of the generic io_addr_t */
typedef struct {
	  u8  function_code;
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

extern int (*run_loop)(void*);
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

/* Slave specific loop function replacing run_loop_default in the IO 
 * library
 */

int modbus_slave_loop(void *foo) 
{
  int nd=-1;
  int i=0;
  u8 *buff;
  u16 trId=0;
  int nbytes;
  u8 reply[255];
  int npoints;
  int count;
  u16 start_addr;
  u16 quantity;
  plc_pt_t point;
  u32 value;
  io_addr_modbus_t *mb_addr;
  io_addr_t *mentry_addr;

  while(1) {
    //plc_scan_beg();
    plc_update();
    nbytes=modbus_read(&nd, &buff, &trId, NULL, 0, NULL);
    if(nbytes<0) {
       nd=-1;
       continue;
    } 
    // Check if the slave address in the packet matches one
    // of our slave addresses
    for (count = 0; count < nw_array_.count; count ++) {
       if(buff[0]==nw_array_.net[count].slave_address) {
          break;
       }
    }
    if(count==nw_array_.count) {
       plc_log_errmsg(1, "Request to an unsupported slave address, dropped");
       modbus_close(nd);
       nd=-1;
       continue;
    }

    // Determine the length of the state map
    
    npoints=slave_mapped_points();

    if(buff) {
       count = 0;
       reply[0]=buff[0]; //SLAVE ADDRESS
       reply[1]=buff[1]; //CODE OF OPERATION;
       reply[2]=0; //REPLY SIZE OR EXCEPTION CODE

       start_addr=(*(u16 *)&(buff[2]));
       start_addr=ntohs(start_addr);
       quantity=(*(u16 *)&(buff[4]));
       quantity=ntohs(quantity);

       switch(buff[1]) {
          /*
          case 0x01: //READ COILS

             if(((start_addr+quantity)/32 > npoints) || (quantity > 2000) || (quantity < 1)) {
                plc_log_errmsg(1, "Request READ COILS out of range, start: %d, quantity: %d\n", start_addr, quantity);
                reply[1]+=0x80;
                reply[2]=2;
                count = 3;
             } else {

                bitshift = (start_addr % sizeof(u32))-1;
                reply[3]=0;
                for(i=start_addr/sizeof(u32);i<(start_addr+quantity)/sizeof(u32);i++) {
                   value = plc_get(plc_pt_by_index(i, NULL, NULL));
                   value = value >> bitshift;
                   if(bitshift && i<(start_addr+quantity)/sizeof(u32)-1) {
                      value1=plc_get(plc_pt_by_index(i+1, NULL, NULL));
                      value1=value1 << (sizeof(u32)-bitshift);
                      value=value|value1;
                   }
                   memcpy(&reply[3+(i-start_addr/sizeof(u32))*sizeof(u32)], &value, sizeof(u32));
                   reply[2]+=sizeof(u32);
                }
                count = reply[2]+3;
             }
                break;
                */
          case 0x04: //READ INPUT REGISTERS
          case 0x03: //READ HOLDING REGISTERS
                if(((start_addr+quantity) > npoints) || (quantity > 0x7D) || (quantity < 1)) {
                   plc_log_errmsg(1, "Request READ INPUT REGISTERS out of range, start: %d, quantity: %d\n", start_addr, quantity);
                   reply[1]+=0x80;
                   reply[2]=2;
                   count = 3;
                   break;
                } else {
                   for(i=start_addr;i<(start_addr+quantity);i++) {

                      get_map_entry_by_index(i, &mentry_addr, &point);

                      mb_addr=(io_addr_modbus_t *)mentry_addr;

                      if(mb_addr && point.valid) {
                         value=plc_get(point);
                         value=htons(value >> ((*mb_addr).count));
                         memcpy(&reply[3+(i-start_addr)*2], &value, 2);
                         reply[2]+=2;
                      } else {
                         plc_log_errmsg(1, "READ INPUT REGISTERS, invalid point at address %d\n", i);
                         reply[1]+=0x80;
                         reply[2]=4; //SLAVE DEVICE FAILURE
                         count = 3;
                         break;
                      }
                   } 
                }
                count=reply[2]+3;
                break;
          case 0x06: //WRITE SINGLE REGISTER
                if(start_addr >= npoints) {
                   plc_log_errmsg(1, "Request WRITE SINGLE REGISTER out of range, address: %d\n", start_addr);
                   reply[1]+=0x80;
                   reply[2]=2;
                   count = 3;
                   break;
                } else {
                   get_map_entry_by_index(start_addr, &mentry_addr, &point);

                   mb_addr=(io_addr_modbus_t *)mentry_addr;

                   if(mb_addr && point.valid) { 
                      if((plc_pt_rw(point)==2) && plc_pt_len(point)>15) {
                         value=plc_get(point);
                         value=(value & ~(0x0000ffff << (*mb_addr).count)) | ((u32)quantity << (*mb_addr).count);
                         plc_set(point, value);
                         plc_update();
                         value=plc_get(point);
                         value=htons(value >> ((*mb_addr).count));
                         memcpy(&reply[4], &value, 2);
                         value=htons(start_addr);
                         memcpy(&reply[2], &value, 2);
                         count=6;
                      } else {
                         plc_log_errmsg(1, "WRITE SINGLE  REGISTER, read-only point at address %d\n", start_addr);
                         reply[1]+=0x80;
                         reply[2]=4;
                         count = 3;
                      }
                   } else {
                         plc_log_errmsg(1, "WRITE SINGLE  REGISTER, wrong point at address %d\n", i);
                         reply[1]+=0x80;
                         reply[2]=3;
                         count = 3;
                   }
                }
                break;
          default:
             reply[1]+=0x80;
             reply[2]=01;
             count = 3;
             break;
       }
       modbus_write(nd, reply, count, trId);
    }
    plc_update();
    //plc_scan_end();
  }
  return 0;
}

/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****   H a r d w a r e   A c c e s s   F u n c t i o n s  ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

#define mb_master_call7(f) f(io_addr_modbus->slave,                                   \
                             io_addr_modbus->address,                                 \
                             io_addr_modbus->count,                                   \
                             value,                                                   \
                             nw_array_.net[io_addr_modbus->nw_array_ofs].nd,          \
                             nw_array_.net[io_addr_modbus->nw_array_ofs].send_retries,\
                             &nw_array_.net[io_addr_modbus->nw_array_ofs].timeout)

#define mb_master_call8(f) f(io_addr_modbus->slave,                                   \
                             io_addr_modbus->address,                                 \
                             io_addr_modbus->count,                                   \
                             value,                                                   \
                             io_addr_modbus->count,                                   \
                             nw_array_.net[io_addr_modbus->nw_array_ofs].nd,          \
                             nw_array_.net[io_addr_modbus->nw_array_ofs].send_retries,\
                             &nw_array_.net[io_addr_modbus->nw_array_ofs].timeout)

/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****                                                      ****/
/****          P a r s e    I O    A d d r e s s           ****/
/****                                                      ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/

  /* Andrey: please delete this if it is not required! */ 
#if 0
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
#endif

/* parse the io address */
int io_hw_parse_io_addr(io_addr_t *io_addr,
		 	const char *const_addr_str,
			dir_t dir,
			int pt_len)
{
  static u16 address = 0;
  int shift=0;
  io_addr_modbus_t *io_addr_ptr = (io_addr_modbus_t *)io_addr;

  memset(io_addr_ptr, 0, sizeof(io_addr_modbus_t));
  io_addr_ptr->address = address;
  if(string_str_to_int(const_addr_str, &shift, 0, 32)) {
     if(!strncasecmp(const_addr_str, "high", 4)) {
        shift=16;
     } else {
        if(!strncasecmp(const_addr_str, "low", 3)) {
           shift=0;
        } else {
           plc_log_errmsg(1, "Map line %d: invalid bit shift", address+1);
           return -1;
        }
     }
  }
  io_addr_ptr->count = shift;
  address++;
  return 0;
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


  /* Andrey: please delete this if it is not required! */ 
#if 0
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
#endif

/*************************************************/
/**                                             **/
/**   Parse Serial Network Config (RTU/Ascii)   **/
/**                                             **/
/*************************************************/

#ifdef MODBUS_S_SERIAL
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

#ifdef MODBUS_S_RTU
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
      get_config_i32(&nw->send_retries, MODBUS_SENDRETRIES_MIN, MODBUS_SENDRETRIES_MAX,
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
#endif /* MODBUS_S_SERIAL */


/*************************************************/
/**                                             **/
/**    Parse TCP Network Config (RTU/Ascii)     **/
/**                                             **/
/*************************************************/

#ifdef MODBUS_S_TCP

static int init_network_data(int row, network_t *nw) {
  char  *tmp_str, *tmp2_str;
  /* Andrey: please delete this if it is not required! */ 
  /*double tmp_dbl;*/
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
  nw->slave_address            = MODBUS_ADDRESS_DEF;
  nw->mode               = MODBUS_MODE_DEF;
  nw->connections        = MODBUS_CONNECTIONS_DEF;
  addr->host             = MODBUS_HOSTNAME_DEF;
  addr->service          = MODBUS_SERVICE_DEF;

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
    else if (strcmp(tmp_str, MODBUS_ADDRESS_NAME) == 0)
      get_config_i32(&nw->slave_address, MODBUS_ADDRESS_MIN, MODBUS_ADDRESS_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_MODE_NAME) == 0)
      get_config_i32(&nw->mode, MODBUS_MODE_MIN, MODBUS_MODE_MAX,
                     tmp_str, tmp2_str);
    else if (strcmp(tmp_str, MODBUS_CONNECTIONS_NAME) == 0)
      get_config_i32(&nw->connections, MODBUS_CONNECTIONS_MIN, MODBUS_CONNECTIONS_MAX,
                     tmp_str, tmp2_str);
//    else if (strcmp(tmp_str, MODBUS_SENDRETRIES_NAME) == 0)
//      get_config_i32(&nw->send_retries, MODBUS_SENDRETRIES_MIN, MODBUS_SENDRETRIES_MAX,
//                     tmp_str, tmp2_str);
//    else if (strcmp(tmp_str, MODBUS_TIMEOUT_NAME) == 0) {
//      get_config_d  (&tmp_dbl, MODBUS_TIMEOUT_MIN, MODBUS_TIMEOUT_MAX,
//                     tmp_str, tmp2_str);
//      nw->timeout = d_to_timespec(tmp_dbl);
//    }
//    else if (strcmp(tmp_str, MODBUS_CLOSEONSILENCE_NAME) == 0)
//      get_config_str(&addr->close_on_silence, close_on_silence_str2int,
//                     MODBUS_CLOSEONSILENCE_DEF, tmp_str, tmp2_str);
//    else if (strcmp(tmp_str, MODBUS_SLAVEMODE_NAME) == 0)
//      get_config_str(&addr->slavemode, slavemode_str2int,
//                     MODBUS_SLAVEMODE_DEF, tmp_str, tmp2_str);
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

#endif /* MODBUS_S_TCP */



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
#ifdef MODBUS_S_TCP
  const char *nw_type = MODBUS_NETWORK_TCP_TYPE;
#else
#ifdef MODBUS_S_RTU
  const char *nw_type = MODBUS_NETWORK_RTU_TYPE;
#else
#ifdef MODBUS_S_ASCII
  const char *nw_type = MODBUS_NETWORK_ASCII_TYPE;
#else
#error This file can only be compiled with one, and only one, of the following \
       command line options (-DMODBUS_S_RTU -DMODBUS_S_TCP -DMODBUS_S_ASCII)
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
  int i = 0;

  for (count = 0; count < nw_array_.count; count ++) 
     i += nw_array_.net[count].connections;

  if (mb_slave_init(nw_array_.count+i) < 0)
    return -1;

  for (count = 0; count < nw_array_.count; count ++) {
    if ((nw_array_.net[count].nd = mb_slave_connect(nw_array_.net[count].addr)) < 0) {
      /* could not set up slave communication ! */
      plc_log_errmsg(1, "Error setting up network %s.",
                     nw_array_.net[count].name);
    }
  }
  run_loop=modbus_slave_loop;

  return 0;
}



int io_hw_done(void) {
  return mb_slave_done();
}

/* Functions that the slave doesn't need but the IO library requires
 * in order to compile cleanly
 */

int io_hw_write(io_addr_t *io_addr, u32  value)
{
   return 0;
}
int io_hw_read (io_addr_t *io_addr, u32 *value)
{
   return 0;
}
int io_hw_write_end(void)
{
   return 0;
}
int io_hw_read_end (void)
{
   return 0;
}
