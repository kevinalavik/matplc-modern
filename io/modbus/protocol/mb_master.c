/* mb_master.c

   By:
       - P.Costigan email: phil@pcscada.com.au http://pcscada.com.au
       - Mario de Sousa (msousa@fe.up.pt)

   These library of functions are designed to enable a program send and
   receive data from a device that communicates using the Modbus protocol.

   Copyright (C) 2000 Philip Costigan  P.C. SCADA LINK PTY. LTD.
   Copyright (C) 2002 Mario de Sousa

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
		Added mb_ntoh() and mb_hton()
   11/2002 - Mario de Sousa
		Added retries at layer2 (i.e. in this file)
		Added the mb_transaction() function.
                Added checks to response frame for correctness
   03/2003 - Mario de Sousa
   		Added *_u32 functions (functions taking as argument arrays of u32)

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

/* #define DEBUG */		/* uncomment to see the data sent and received */


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



/***********************************************************************

	A Master/Slave transaction...

***********************************************************************/
/* RETURN: 0..2 -> these values will never be returned!
 *         <0   -> error codes
 *         >2   -> frame length
 */
static int mb_transaction(u8  *packet,
			  int query_length,
			  u8  **data,
                          int ttyfd,
                          int send_retries,
                          u8  *error_code,
                          const struct timespec *response_timeout) {

  int error = INTERNAL_ERROR;
  int response_length = INTERNAL_ERROR;
  u16 send_transaction_id, recv_transaction_id;

  for (send_retries++; send_retries > 0; send_retries--) {

    /* We must also initialize the recv_transaction_id with the same value,
     * since some layer 1 protocols do not support transaction id's, so
     * simply return the recv_transaction_id variable without any changes...
     */
    send_transaction_id = recv_transaction_id = next_transaction_id();


    if (modbus_write(ttyfd, packet, query_length, send_transaction_id) < 0)
      {error = PORT_FAILURE; continue;}

    response_length = modbus_read(&ttyfd, data, &recv_transaction_id,
                                  packet, query_length,
                                  response_timeout);

    if(response_length == -2)
      return TIMEOUT;
    if(response_length < 0)
      {error = PORT_FAILURE; continue;}
    if( response_length < 3 )
      /* This should never occur! Modbus_read() should only return valid frames! */
      return INTERNAL_ERROR;

    /* first check whether we have correct transaction id */
    if (send_transaction_id != recv_transaction_id)
      {error = INVALID_FRAME; continue;}

    /* NOTE: no need to check whether (*data)[0] = slave!              */
    /*       This has already been done by the modbus_read() function! */

    /* Check whether the response frame is a response to _our_ query */
    if (((*data)[1] & ~0x80) != packet[1])
      {error = INVALID_FRAME; continue;}

    /* Now check whether we received a Modbus Exception frame */
    if (((*data)[1] & 0x80) != 0) {
      /* we have an exception frame! */
      if (error_code != NULL)
        /* NOTE: we have already checked above that data[2] exists! */
        *error_code = (*data)[2];
      return MODBUS_ERROR;
    }

    /* everything seems to be OK. Let's get out of the send retry loop... */
    /* success! */
    return response_length;
  }

  /* reached the end of the retries... */
  return error;
}


/************************************************************************

	read_IO_status

	read_coil_stat_query and read_coil_stat_response interigate
	a modbus slave to get coil status. An array of coils shall be
	set to TRUE or FALSE according to the response from the slave.

*************************************************************************/

/* called by:
 *
 *  read_input_bits()
 *  read_output_bits()
 *
 */

static int read_IO_status(u8  function,
                          u8  slave,
                          u16 start_addr,
                          u16 count,
                          int *dest,
                          int dest_size,
                          int ttyfd,
                          int send_retries,
                          u8  *error_code,
                          const struct timespec *response_timeout)
{
        u8 *packet = query_buffer_;
        u8 *data;
	int response_length, query_length;
	int temp, i, bit, dest_pos = 0;
	int coils_processed = 0;

	query_length = build_query_packet(slave, function,
                                          start_addr, count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	response_length = mb_transaction(packet, query_length, &data,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	/* NOTE: Integer division. This is equivalent of determining the ceil(count/8) */
	if (response_length != 3 + (count+7)/8)
	  return INVALID_FRAME;

	if (data[2] != (count+7)/8)
	  return INVALID_FRAME;

	for( i = 0; (i < data[2]) && (i < dest_size); i++ ) {
	  /* shift reg hi_byte to temp */
	  temp = data[ 3 + i ] ;
	  for( bit = 0x01; bit & 0xff && coils_processed < count; ) {
	    dest[dest_pos] = (temp & bit)?TRUE:FALSE;
	    coils_processed++;
	    dest_pos++;
	    bit = bit << 1;
	  }
	}

	return response_length;
}



/************************************************************************

	read_IO_status_u32

	Similar to read_IO_status, but with a different format for data
	input!

	The count bits read will be packed into an array of u32, i.e. each
	element of the array will contain the result of 32 bits!
	The last element in the array will have unused bits set to 0.

*************************************************************************/

/* called by:
 *
 *  read_input_bits_u32()
 *  read_output_bits_32()
 *
 */

static int read_IO_status_u32(u8  function,
                              u8  slave,
                              u16 start_addr,
                              u16 count, /* number of bits !! */
                              u32 *dest,
                              int ttyfd,
                              int send_retries,
                              u8  *error_code,
                              const struct timespec *response_timeout)
{
        u8 *packet = query_buffer_;
        u8 *data;
	int response_length, query_length;
	int byte_count, i, dest_pos = 0;

	query_length = build_query_packet(slave, function,
                                          start_addr, count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	response_length = mb_transaction(packet, query_length, &data,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	/* NOTE: Integer division. This is equivalent of determining the ceil(count/8) */
	if (response_length != 3 + (count+7)/8)
	  return INVALID_FRAME;

	if (data[2] != (count+7)/8)
	  return INVALID_FRAME;

	byte_count = data[2];
	data += 3;
	/* handle groups of 4 bytes... */
	for(i = 0, dest_pos = 0; i + 3 < byte_count; i += 4, dest_pos++)
	  dest[dest_pos] = data[i] + data[i+1]*0x100 + data[i+2]*0x10000 + data[i+3]*0x1000000;
	/* handle any remaining bytes... begining with the last! */
	if (i < byte_count) dest[dest_pos] = 0;
	for(byte_count--; i <= byte_count; byte_count--)
	  dest[dest_pos] = dest[dest_pos]*0x100 + data[byte_count];

	return response_length;
}



/************************************************************************

	read_coil_status

	reads the boolean status of coils and sets the array elements
	in the destination to TRUE or FALSE

*************************************************************************/

inline int read_output_bits(u8  slave,
                            u16 start_addr,
                            u16 count,
                            int *dest,
                            int dest_size,
                            int ttyfd,
                            int send_retries,
                            u8  *error_code,
                            const struct timespec *response_timeout)
{
	return read_IO_status(0x01 /* function */,
                              slave, start_addr, count,
	                      dest, dest_size,
                              ttyfd, send_retries,
			      error_code, response_timeout);
}



/************************************************************************

	read_coil_status_u32

	Similar to read_coil_status, but with a different format for data
	input!

	The count bits read will be packed into an array of u32, i.e. each
	element of the array will contain the result of 32 bits!
	The last element in the array will have unused bits set to 0.

*************************************************************************/

inline int read_output_bits_u32(u8  slave,
                                u16 start_addr,
                                u16 count,
                                u32 *dest,
                                int ttyfd,
                                int send_retries,
                                u8  *error_code,
                                const struct timespec *response_timeout)
{
	return read_IO_status_u32(0x01 /* function */,
                                  slave, start_addr, count,
	                          dest,
                                  ttyfd, send_retries,
                                  error_code, response_timeout);
}


/************************************************************************

	read_input_bits

	same as read_coil_status but reads the slaves input table.

************************************************************************/

inline int read_input_bits(u8  slave,
                           u16 start_addr,
                           u16 count,
                           int *dest,
                           int dest_size,
                           int ttyfd,
                           int send_retries,
                           u8  *error_code,
                           const struct timespec *response_timeout)
{
	return read_IO_status(0x02 /* function */,
                              slave, start_addr, count,
	                      dest, dest_size,
                              ttyfd, send_retries,
			      error_code, response_timeout);
}



/************************************************************************

	read_input_bits_u32

	Similar to read_input_bits, but with a different format for data
	input!

	The count bits read will be packed into an array of u32, i.e. each
	element of the array will contain the result of 32 bits!
	The last element in the array will have unused bits set to 0.

************************************************************************/

inline int read_input_bits_u32(u8  slave,
                               u16 start_addr,
                               u16 count,
                               u32 *dest,
                               int ttyfd,
                               int send_retries,
                               u8  *error_code,
                               const struct timespec *response_timeout)
{
	return read_IO_status_u32(0x02 /* function */,
                                   slave, start_addr, count,
                                   dest,
                                   ttyfd, send_retries,
                                   error_code, response_timeout);
}






/************************************************************************

	read_registers

	read the data from a modbus slave and put that data into an array.

************************************************************************/

/* called by:
 *
 * read_input_words()
 * read_output_words()
 */

static int read_registers(u8  function,
                          u8  slave,
                          u16 start_addr,
                          u16 count,
                          u16 *dest,
                          int dest_size,
                          int ttyfd,
                          int send_retries,
                          u8  *error_code,
                          const struct timespec *response_timeout)
{
	u8 *data;
	u8 *packet = query_buffer_;
	int response_length;
        int query_length;
        int temp,i;

        query_length = build_query_packet(slave, function,
                                          start_addr, count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	response_length = mb_transaction(packet, query_length, &data,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 3 + 2*count)
	  return INVALID_FRAME;

	if (data[2] != 2*count)
	  return INVALID_FRAME;

	for(i = 0; (i < (data[2]*2)) && (i < dest_size); i++ ) {
	  /* shift reg hi_byte to temp */
	  temp = data[ 3 + i *2 ] << 8;
	  /* OR with lo_byte           */
	  temp = temp | data[ 4 + i * 2 ];
	  dest[i] = temp;
	}

	return response_length;
}




/************************************************************************

	read_registers_u32

	read u16 registers from a modbus slave and put that data into an
	array of u32. Unused bits of the last element of the u32 array
	are set to 0.

************************************************************************/

/* called by:
 *
 * read_input_words_u32()
 * read_output_words_u32()
 */

static int read_registers_u32(u8  function,
                              u8  slave,
                              u16 start_addr,
                              u16 count,
                              u32 *dest,
                              int ttyfd,
                              int send_retries,
                              u8  *error_code,
                              const struct timespec *response_timeout)
{
	u8 *data;
	u8 *packet = query_buffer_;
	int response_length;
        int query_length;
        int i, byte_count, dest_pos;

        query_length = build_query_packet(slave, function,
                                          start_addr, count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	response_length = mb_transaction(packet, query_length, &data,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 3 + 2*count)
	  return INVALID_FRAME;

	if (data[2] != 2*count)
	  return INVALID_FRAME;

	byte_count = data[2];
	data += 3;
	/* handle groups of 4 bytes... */
	for(i = 0, dest_pos = 0; i + 3 < byte_count; i += 4, dest_pos++)
	  dest[dest_pos] = mb_ntoh(*(u16 *)(data+i)) + 0x10000*mb_ntoh(*(u16 *)(data+i+2));
	/* handle any remaining bytes...
	 * since byte_count is supposed to be multiple of 2,
	 * (and has already been verified above 'if (data[2] != 2*count)')
	 * this will be either 2, or none at all!
	 */
	if (i + 1 < byte_count)
	  dest[dest_pos] = mb_ntoh(*(u16 *)(data+i));

	return response_length;
}







/************************************************************************

	read_holding_registers

	Read the holding registers in a slave and put the data into
	an array.

*************************************************************************/

inline int read_output_words(u8  slave,
                             u16 start_addr,
                             u16 count,
                             u16 *dest,
                             int dest_size,
                             int ttyfd,
                             int send_retries,
                             u8  *error_code,
                             const struct timespec *response_timeout)
{
	if( count > MAX_READ_REGS ) {
	  count = MAX_READ_REGS;
#ifdef DEBUG
	  fprintf( stderr, "Too many registers requested.\n" );
#endif
	}

	return read_registers(0x03 /* function */,
                              slave, start_addr, count,
			      dest, dest_size,
                              ttyfd, send_retries,
			      error_code, response_timeout);
}




/************************************************************************

	read_holding_registers_u32

	Read the holding registers in a slave and put the data into
	an u32 array.

*************************************************************************/

inline int read_output_words_u32(u8  slave,
                                 u16 start_addr,
                                 u16 count,
                                 u32 *dest,
                                 int ttyfd,
                                 int send_retries,
                                 u8  *error_code,
                                 const struct timespec *response_timeout)
{
	if( count > MAX_READ_REGS ) {
	  count = MAX_READ_REGS;
#ifdef DEBUG
	  fprintf( stderr, "Too many registers requested.\n" );
#endif
	}

	return read_registers_u32(0x03 /* function */,
                                   slave, start_addr, count,
                                   dest,
                                   ttyfd, send_retries,
                                   error_code, response_timeout);
}




/************************************************************************

	read_input_registers

	Read the inputg registers in a slave and put the data into
	an array.

*************************************************************************/

inline int read_input_words(u8  slave,
                            u16 start_addr,
                            u16 count,
                            u16 *dest,
                            int dest_size,
                            int ttyfd,
                            int send_retries,
                            u8  *error_code,
                            const struct timespec *response_timeout)
{
	if( count > MAX_INPUT_REGS ) {
	  count = MAX_INPUT_REGS;
#ifdef DEBUG
	  fprintf( stderr, "Too many input registers requested.\n" );
#endif
	}

        return read_registers(0x04 /* function */,
                              slave, start_addr, count,
			      dest, dest_size,
                              ttyfd, send_retries,
			      error_code, response_timeout);
}


/************************************************************************

	read_input_registers_u32

	Read the inputg registers in a slave and put the data into
	an u32 array.

*************************************************************************/

inline int read_input_words_u32(u8  slave,
                                u16 start_addr,
                                u16 count,
                                u32 *dest,
                                int ttyfd,
                                int send_retries,
                                u8  *error_code,
                                const struct timespec *response_timeout)
{
	if( count > MAX_INPUT_REGS ) {
	  count = MAX_INPUT_REGS;
#ifdef DEBUG
	  fprintf( stderr, "Too many input registers requested.\n" );
#endif
	}

        return read_registers_u32(0x04 /* function */,
                                  slave, start_addr, count,
                                  dest,
                                  ttyfd, send_retries,
                                  error_code, response_timeout);
}




/*************************************************************************

	set_single

	sends a value to a register in a slave.

**************************************************************************/

/* called by:
 *
 * write_output_bit()
 * write_output_word()
 */

static int set_single(u8  function,
                      u8  slave,
                      u16 addr,
                      u16 value,
                      int ttyfd,
                      int send_retries,
                      u8  *error_code,
                      const struct timespec *response_timeout)
{
        u8 *packet = query_buffer_;
        u8 *data;
        int query_length, response_length;

        query_length = build_query_packet(slave, function,
                                          addr, value,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	response_length = mb_transaction(packet, query_length, &data,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 6)
	  return INVALID_FRAME;

	if ((data[2] != packet[2]) || (data[3] != packet[3]) ||
	    (data[4] != packet[4]) || (data[5] != packet[5]))
	  return INVALID_FRAME;

        return response_length;
}






/*************************************************************************

	force_single_coil

	turn on or off a single coil on the slave device

*************************************************************************/

inline int write_output_bit(u8  slave,
                            u16 coil_addr,
                            u16 state,
                            int fd,
                            int send_retries,
                            u8  *error_code,
                            const struct timespec *response_timeout)
{
	if (state)
          state = 0xFF00;

	return set_single(0x05 /* function */, slave, coil_addr, state,
                          fd, send_retries,
			  error_code, response_timeout);
}





/*************************************************************************

	preset_single_register

	sets a value in one holding register in the slave device

*************************************************************************/

inline int write_output_word(u8  slave,
                             u16 reg_addr,
                             u16 value,
                             int fd,
                             int send_retries,
                             u8  *error_code,
                             const struct timespec *response_timeout)
{
	return set_single(0x06 /* function */, slave, reg_addr, value,
                          fd, send_retries,
			  error_code, response_timeout);
}





/************************************************************************

	set_multiple_coils

	Takes an array of ints and sets or resets the coils on a slave
	appropriatly.

*************************************************************************/

#define PRESET_QUERY_SIZE 210

int write_output_bits(u8  slave,
                      u16 start_addr,
                      u16 coil_count,
                      int *data,
                      int ttyfd,
                      int send_retries,
                      u8  *error_code,
                      const struct timespec *response_timeout)
{
	int byte_count, i;
	u8  bit;
	int coil_check = 0;
	int data_array_pos = 0;
        int query_length, response_length;
	u8  *packet = query_buffer_;
	u8  *rdata;

	if( coil_count > MAX_WRITE_COILS ) {
	  coil_count = MAX_WRITE_COILS;
#ifdef DEBUG
	  fprintf( stderr, "Writing to too many coils.\n" );
#endif
	}

        query_length = build_query_packet(slave, 0x0F /* function */,
                                          start_addr, coil_count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	/* NOTE: Integer division. This is equivalent of determining the ceil(count/8) */
	byte_count = (coil_count+7)/8;
        packet[query_length] = byte_count;

	bit = 0x01;

	for( i = 0; i < byte_count; i++) {
	  packet[++query_length] = 0;
	  while( bit & 0xFF && coil_check++ < coil_count ) {
	    if( data[ data_array_pos++ ] ) {
	      packet[ query_length ] |= bit;
	    } else {
	      packet[ query_length ] &=~ bit;
	    }
	    bit <<= 1;
	  }
	  bit = 0x01;
	}

	response_length = mb_transaction(packet, ++query_length, &rdata,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 6)
	  return INVALID_FRAME;

	if ((rdata[2] != packet[2]) || (rdata[3] != packet[3]) ||
	    (rdata[4] != packet[4]) || (rdata[5] != packet[5]))
	  return INVALID_FRAME;

        return response_length;
}



/************************************************************************

	set_multiple_coils_u32

	Similar to set_multiple_coils, but takes an array of u32,
	and sets or resets the coils on a slave appropriatly.
	Unused bits should be set to 0!

*************************************************************************/

#define PRESET_QUERY_SIZE 210

int write_output_bits_u32(u8  slave,
                          u16 start_addr,
                          u16 coil_count,
                          u32 *data,
                          int ttyfd,
                          int send_retries,
                          u8  *error_code,
                          const struct timespec *response_timeout)
{
	int org_pos, byte_count, i;
        int query_length, response_length;
	u8  *packet = query_buffer_;
	u8  *rdata;

	if( coil_count > MAX_WRITE_COILS ) {
	  coil_count = MAX_WRITE_COILS;
#ifdef DEBUG
	  fprintf( stderr, "Writing to too many coils.\n" );
#endif
	}

        query_length = build_query_packet(slave, 0x0F /* function */,
                                          start_addr, coil_count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	/* NOTE: Integer division. This is equivalent of determining the ceil(count/8) */
	byte_count = (coil_count+7)/8;
        packet[query_length] = byte_count;

	/* handle groups of 4 bytes... */
	for(i = 0, org_pos = 0; i + 3 < byte_count; i += 4, org_pos++) {
	  packet[++query_length] = data[org_pos] & 0xFF; data[org_pos] >>= 8;
	  packet[++query_length] = data[org_pos] & 0xFF; data[org_pos] >>= 8;
	  packet[++query_length] = data[org_pos] & 0xFF; data[org_pos] >>= 8;
	  packet[++query_length] = data[org_pos] & 0xFF;
	}
	/* handle any remaining bytes... */
	for(; i < byte_count; i++) {
	  packet[++query_length] = data[org_pos] & 0xFF; data[org_pos] >>= 8;
	}

	response_length = mb_transaction(packet, ++query_length, &rdata,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 6)
	  return INVALID_FRAME;

	if ((rdata[2] != packet[2]) || (rdata[3] != packet[3]) ||
	    (rdata[4] != packet[4]) || (rdata[5] != packet[5]))
	  return INVALID_FRAME;

        return response_length;
}


/*************************************************************************

	preset_multiple_registers

	copy the values in an array to an array on the slave.

***************************************************************************/

int write_output_words(u8  slave,
                       u16 start_addr,
                       u16 reg_count,
                       u16 *data,
                       int ttyfd,
                       int send_retries,
                       u8  *error_code,
                       const struct timespec *response_timeout)
{
	u8  byte_count;
        int i, query_length, response_length;
	u8  *packet = query_buffer_;
	u8  *rdata;

        if( reg_count > MAX_WRITE_REGS ) {
	  reg_count = MAX_WRITE_REGS;
#ifdef DEBUG
	  fprintf( stderr, "Trying to write to too many registers.\n" );
#endif
	}

        query_length = build_query_packet(slave, 0x10 /* function */,
                                          start_addr, reg_count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	byte_count = reg_count*2;
	packet[query_length] = byte_count;

	for( i = 0; i < reg_count; i++ ) {
	  packet[++query_length] = data[i] >> 8;
	  packet[++query_length] = data[i] & 0x00FF;
	}

	response_length = mb_transaction(packet, ++query_length, &rdata,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 6)
	  return INVALID_FRAME;

	if ((rdata[2] != packet[2]) || (rdata[3] != packet[3]) ||
	    (rdata[4] != packet[4]) || (rdata[5] != packet[5]))
	  return INVALID_FRAME;

	return response_length;
}




/*************************************************************************

	preset_multiple_registers_u32

	copy the values in an u32 array to an array on the slave.

***************************************************************************/

int write_output_words_u32(u8  slave,
                           u16 start_addr,
			     /* number of 16 bit registers packed in the u32 array! */
                           u16 reg_count,
                           u32 *data,
                           int ttyfd,
                           int send_retries,
                           u8  *error_code,
                           const struct timespec *response_timeout)
{
	u8  byte_count;
        int i, query_length, response_length;
	u8  *packet = query_buffer_;
	u8  *rdata;

        if( reg_count > MAX_WRITE_REGS ) {
	  reg_count = MAX_WRITE_REGS;
#ifdef DEBUG
	  fprintf( stderr, "Trying to write to too many registers.\n" );
#endif
	}

        query_length = build_query_packet(slave, 0x10 /* function */,
                                          start_addr, reg_count,
                                          packet);
	if (query_length < 0)
	  return INTERNAL_ERROR;

	byte_count = reg_count*2;
	packet[query_length] = byte_count;
	/* handle groups of 4 bytes... */
	for(i = 0; i + 3 < byte_count; i += 4) {
	  *((u16 *)(packet+(++query_length))) = mb_hton(data[i]);
	  ++query_length;
	  *((u16 *)(packet+(++query_length))) = mb_hton(data[i] >> 16);
	  ++query_length;
	}
	/* handle any remaining bytes...
	 * since byte_count is supposed to be multiple of 2,
	 * (and has already been verified above 'if (data[2] != 2*count)')
	 * this will be either 2, or none at all!
	 */
	if (i + 1 < byte_count) {
	  *((u16 *)(packet+(++query_length))) = mb_hton(data[i]);
	  ++query_length;
	}

	response_length = mb_transaction(packet, ++query_length, &rdata,
					 ttyfd, send_retries,
					 error_code, response_timeout);

	if (response_length < 0)
	  return response_length;

	if (response_length != 6)
	  return INVALID_FRAME;

	if ((rdata[2] != packet[2]) || (rdata[3] != packet[3]) ||
	    (rdata[4] != packet[4]) || (rdata[5] != packet[5]))
	  return INVALID_FRAME;

	return response_length;
}








/************************************************************************

	initialise / shutdown the library

	These functions sets up/shut down the library state
        (allocate memory for buffers, initialise data strcutures, etc)

**************************************************************************/

int mb_master_init(int nd_count) {
        int extra_bytes;

#ifdef DEBUG
	fprintf( stderr, "mb_master_init()\n");
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




int mb_master_done(void) {
        free(query_buffer_);
        query_buffer_ = NULL;

        return modbus_done();
}




/************************************************************************

	open/close master connection

	This function opens/closes a connection to the remote slave.

**************************************************************************/

int mb_master_connect(node_addr_t node_addr) {
#ifdef DEBUG
	fprintf( stderr, "mb_master_connect()\n");
#endif
          /* call layer 1 library */
        return modbus_connect(node_addr);
}



int mb_master_close(int nd) {
#ifdef DEBUG
	fprintf( stderr, "mb_master_close(): nd = %d\n", nd);
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









