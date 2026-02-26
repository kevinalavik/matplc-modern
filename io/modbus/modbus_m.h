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
 * modbus_m_rtu.h
 */

#ifndef MODBUS_M_RTU_H
#define MODBUS_M_RTU_H

#include <plc.h>



/***********************************************************/
/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/
/***********************************************************/

#define DEF_MODULE_NAME           "modbus_m"


/************************/
/* The Node Table...    */
/************************/

#define MODBUS_SLAVE_TABLE        "node"
#define MODBUS_SLAVE_NAME_OFS     0
#define MODBUS_SLAVE_NETW_OFS     1
#define MODBUS_SLAVE_ADDR_OFS     2
#define MODBUS_SLAVE_ADDR_MIN     0
#define MODBUS_SLAVE_ADDR_MAX     255
#define MODBUS_SLAVE_ADDR_DEF     0



/************************/
/* The Address...       */
/************************/

#define MODBUS_INBIT              "in_bit"
#define MODBUS_OUTBIT             "out_bit"
#define MODBUS_INWORD             "in_word"
#define MODBUS_OUTWORD            "out_word"
#define MODBUS_READ_INBIT_CODE    0x02
#define MODBUS_READ_OUTBIT_CODE   0x01
#define MODBUS_READ_INWORD_CODE   0x04
#define MODBUS_READ_OUTWORD_CODE  0x03
#define MODBUS_WRITE_OUTBIT_CODE  0x0F
#define MODBUS_WRITE_OUTWORD_CODE 0x10
#define MODBUS_INBIT_DIR          dir_in     /* enum type defined in io_hw.h */
#define MODBUS_OUTBIT_DIR         dir_out    /* enum type defined in io_hw.h */
#define MODBUS_INWORD_DIR         dir_in     /* enum type defined in io_hw.h */
#define MODBUS_OUTWORD_DIR        dir_out    /* enum type defined in io_hw.h */

#define MODBUS_REG_ADDR_MIN   1
#define MODBUS_REG_ADDR_MAX   10000


/************************/
/* The network Table... */
/************************/
#define MODBUS_NETWORK_TABLE       "network"

#define MODBUS_NETWORK_NAME_OFS    0
#define MODBUS_NETWORK_TYPE_OFS    1

#define MODBUS_NETWORK_TCP_TYPE    "tcp"
#define MODBUS_NETWORK_RTU_TYPE    "rtu"
#define MODBUS_NETWORK_ASCII_TYPE  "ascii"

/* TCP Specific parameters... */
/* -------------------------- */
#define MODBUS_HOSTNAME_NAME  "host"
#define MODBUS_HOSTNAME_DEF   "localhost"

#define MODBUS_SERVICE_NAME   "port"
#define MODBUS_SERVICE_DEF    "502"

  /* NOTE: only used by the TCP version.
   * Parameter to define whether the connection to each slave
   * should be closed at the end of each scan...
   */
#define MODBUS_CLOSEONSILENCE_NAME      "TCP_close"
#define MODBUS_CLOSEONSILENCE_TRUE      "true"
#define MODBUS_CLOSEONSILENCE_FALSE     "false"
#define MODBUS_CLOSEONSILENCE_DEF        MODBUS_CLOSEONSILENCE_TRUE


/* RTU/ASCII Specific parameters... */
/* -------------------------------- */
#define MODBUS_DEVFILE_NAME    "device"
#define MODBUS_DEVFILE_DEF     "/dev/ttyS0"

#define MODBUS_BAUDRATE_NAME   "baudrate"
#define MODBUS_BAUDRATE_MIN    0
#define MODBUS_BAUDRATE_MAX    115200
#define MODBUS_BAUDRATE_DEF    9600

#define MODBUS_PARITY_NAME     "parity"
#define MODBUS_PARITY_NONE     "none"
#define MODBUS_PARITY_EVEN     "even"
#define MODBUS_PARITY_ODD      "odd"
#define MODBUS_PARITY_NONE_VAL 0
#define MODBUS_PARITY_EVEN_VAL 2
#define MODBUS_PARITY_ODD_VAL  1
#define MODBUS_PARITY_DEF      MODBUS_PARITY_NONE

#define MODBUS_DATABITS_NAME   "data_bits"
#define MODBUS_DATABITS_MIN    7
#define MODBUS_DATABITS_MAX    8
#define MODBUS_DATABITS_DEF    0

#define MODBUS_STOPBITS_NAME   "stop_bits"
#define MODBUS_STOPBITS_MIN    1
#define MODBUS_STOPBITS_MAX    2
#define MODBUS_STOPBITS_DEF    1

#define MODBUS_IGNORE_ECHO_NAME  "ignore_echo"
#define MODBUS_IGNORE_ECHO_TRUE  "true"
#define MODBUS_IGNORE_ECHO_FALSE "false"
#define MODBUS_IGNORE_ECHO_DEF   MODBUS_IGNORE_ECHO_FALSE



/************************/
/* Generic Parameters...*/
/************************/

#define MODBUS_SENDRETRIES_NAME  "send_retries"
#define MODBUS_SENDRETRIES_MIN   0
#define MODBUS_SENDRETRIES_MAX   i32_MAX
#define MODBUS_SENDRETRIES_DEF   1

#define MODBUS_TIMEOUT_NAME      "timeout"
#define MODBUS_TIMEOUT_MIN       0
#define MODBUS_TIMEOUT_MAX       31536000 /* one year in seconds! */
#define MODBUS_TIMEOUT_DEF       (0.1)    /* in seconds */

#define FIRST_ERROR_LOG_LEVEL       2
#define SUBSEQUENT_ERROR_LOG_LEVEL  3


/***************************/
/* Modbus Error Messages...*/
/***************************/

#define UNKNOWN_ERROR_MSG "Not defined in modbus specification"

static const char *ERROR_CODE_MSG[] = {
 /* 0x00 */ UNKNOWN_ERROR_MSG,
 /* 0x01 */ "Illegal function",
 /* 0x02 */ "Illegal data address",
 /* 0x03 */ "Illegal data value",
 /* 0x04 */ "Illegal response length",
 /* 0x05 */ "Acknowledge",
 /* 0x06 */ "Slave device busy",
 /* 0x07 */ "Negative Acknowledge",
 /* 0x08 */ "Memory parity error",
 /* 0x09 */ UNKNOWN_ERROR_MSG,
 /* 0x0A */ "Gateway path unavailable",
 /* 0x0B */ "Target device failed to respond"
};

#define ERROR_CODE_MSG_MAX 0x0B







#endif /* MODBUS_M_RTU_H */
