/*
 * (c) 2002 Mario de Sousa
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


#ifndef MODBUS_LAYER2_H
#define MODBUS_LAYER2_H

#include <plc.h>  /* get the plc data types */
#include <time.h> /* struct timespec data type */



typedef enum {optimize_speed, optimize_size} optimization_t;


typedef enum {
        naf_ascii,
        naf_rtu,
        naf_tcp,
  } node_addr_family_t;

typedef struct {
        const char *host;
        const char *service;
        int         close_on_silence;  
  } node_addr_tcp_t;

typedef struct {
        const char *device;
        int         baud;       /* plain baud rate, eg 2400; zero for the default 9600 */
        int         parity;     /* 0 for none, 1 for odd, 2 for even                   */
        int         data_bits;
        int         stop_bits;
        int         ignore_echo; /* 1 => ignore echo; 0 => do not ignore echo */
  } node_addr_rtu_t;

typedef node_addr_rtu_t node_addr_ascii_t;

typedef union {
        node_addr_ascii_t ascii;
        node_addr_rtu_t   rtu;
        node_addr_tcp_t   tcp;
  } node_addr_common_t;

typedef struct {
        node_addr_family_t  naf;
        node_addr_common_t  addr;
  } node_addr_t;

#endif  /* MODBUS_LAYER2_H */








