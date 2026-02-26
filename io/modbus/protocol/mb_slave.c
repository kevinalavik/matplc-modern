/* mb_slave.c

   By P.Costigan email: phil@pcscada.com.au http://pcscada.com.au

   These library of functions are designed to enable a program send and
   receive data from a device that communicates using the Modbus protocol.

   Copyright (C) 2000 Philip Costigan  P.C. SCADA LINK PTY. LTD.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   The functions included here have been derived from the
   Modicon Modbus Protocol Reference Guide
   which can be obtained from Schneider at www.schneiderautomation.com.

   This code has its origins with
   paul@pmcrae.freeserve.co.uk (http://www.pmcrae.freeserve.co.uk)
   who wrote a small program to read 100 registers from a modbus slave.

   I have used his code as a catalist to produce this more functional set
   of functions. Thanks paul.



   10/2001 - Mario de Sousa
		Slightly re-organized the code.
                Miscelaneous cleanups
                Removed layer1 functions (i.e. write and read to /dev/ttySx)
                Added support for transaction id

                TODO
                add retries at layer2 (i.e. in this file)
                check response frame for correctness

*/

#include <fcntl.h>	/* File control definitions */
#include <stdio.h>	/* Standard input/output */
#include <string.h>
#include <stdlib.h>
#include <termio.h>	/* POSIX terminal control definitions */
#include <sys/time.h>	/* Time structures for select() */
#include <unistd.h>	/* POSIX Symbolic Constants */
#include <errno.h>	/* Error definitions */

#include <netinet/in.h> /* required for htons() and ntohs() */
#include "mb_layer1.h"
#include "mb_master.h"
#include "mb_master_private.h"

//#define DEBUG 		/* uncomment to see the data sent and received */


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/******************************************/
/******************************************/
/**                                      **/
/**         Global Variables...          **/
/**                                      **/
/******************************************/
/******************************************/

static u8 *query_buffer_ = NULL;



/******************************************/
/******************************************/
/**                                      **/
/**       Local Utility functions...     **/
/**                                      **/
/******************************************/
/******************************************/


/*
 * Function to determine next transaction id.
 *
 * We use a library wide transaction id, which means that we
 * use a new transaction id no matter what slave to which we will
 * be sending the request...
 */
static inline u16 next_transaction_id(void) {

  static u16 next_id = 0;

  return next_id++;
}


/*
 * Functions to convert u16 variables
 * between network and host byte order
 *
 * NOTE: Modbus uses MSByte first, just like
 *       tcp/ip, so we use the htons() and
 *       ntoh() functions to guarantee
 *       code portability.
 */

static inline u16 mb_hton(u16 h_value) {
/*  return h_value; */
  return htons(h_value);
}

static inline u16 mb_ntoh(u16 m_value) {
/*  return m_value; */
  return ntohs(m_value);
}



static inline u8 msb(u16 value) {
/*  return Most Significant Byte of value; */
  return (value >> 8) & 0xFF;
}

static inline u8 lsb(u16 value) {
/*  return Least Significant Byte of value; */
  return value & 0xFF;
}

#define u16_v(char_ptr)  (*((u16 *)(&(char_ptr))))









/***********************************************************************

	The following functions construct the required query into
	a modbus query packet.

***********************************************************************/

static inline int build_query_packet(u8  slave,
                                     u8  function,
                                     u16 start_addr,
                                     u16 count,
                                     u8 *packet)
{
        packet[ 0 ] = slave,
        packet[ 1 ] = function;
          /* NOTE:
           *  Modbus uses high level addressing starting off from 1, but
           *  this is sent as 0 on the wire!
           */
        u16_v(packet[2]) = mb_hton(start_addr - 1);
        u16_v(packet[4]) = mb_hton(count);

        return 6;
}



/************************************************************************

	initialise / shutdown the library

	These functions sets up/shut down the library state
        (allocate memory for buffers, initialise data strcutures, etc)

**************************************************************************/

int mb_slave_init(int nd_count) {
        int extra_bytes;

#ifdef DEBUG
	fprintf( stderr, "mb_slave_init()\n");
	fprintf( stderr, "creating %d nodes\n", nd_count);
#endif

            /* initialise layer 1 library */
        if (modbus_init(nd_count, DEF_OPTIMIZATION, &extra_bytes)
            < 0)
          goto error_exit_0;

            /* initialise send buffer */
        query_buffer_ = (u8 *)malloc(QUERY_BUFFER_SIZE + extra_bytes);
        if (query_buffer_ == NULL)
          goto error_exit_1;

        return 0;

error_exit_1:
        modbus_done();
error_exit_0:
        return -1;
}




int mb_slave_done(void) {
        free(query_buffer_);
        query_buffer_ = NULL;

        return modbus_done();
}




/************************************************************************

	open/close slave connection

	This function sets up/destroys a receiver for connections 
   from the remote master.

**************************************************************************/

int mb_slave_connect(node_addr_t node_addr) {
#ifdef DEBUG
	fprintf( stderr, "mb_slave_connect()\n");
#endif
          /* call layer 1 library */
        return modbus_listen(node_addr);
}



int mb_slave_close(int nd) {
#ifdef DEBUG
	fprintf( stderr, "mb_slave_close(): nd = %d\n", nd);
#endif
          /* call layer 1 library */
        return modbus_close(nd);
}






/***************************************************************************

	Tell the library that we will probably not be issuing any more
        comunication requests for some (small) time to come.

	This will allow the library to free up any resources that it may be
        reserving, but won't be needed for some time to come.

        In reality, the RTU and ASCII versions ignore this function, and
        the TCP version merely closes all the open tcp connections to the
        slaves. Note that each connection will be automatically
        re-established the next time an IO function to the slave in question
        is required.
        To be more precise, the TCP version makes an estimate of how long
        the silence will be based on previous invocations to this exact same
        function, and will only close the connections if this silence is
        expected to be longer than 1 second!


***************************************************************************/
int mb_master_silence_init(void) {
#ifdef DEBUG
	fprintf( stderr, "mb_master_silence_init():\n");
#endif
          /* call layer 1 library */
        return modbus_silence_init();
}




/***************************************************************************

	Get minimum acceptable timeout (in seconds).

***************************************************************************/
double mb_get_min_timeout(int baud,
                          int parity,
                          int data_bits,
                          int stop_bits) {
  return modbus_get_min_timeout(baud, parity, data_bits, stop_bits);
};









