/* 		modbus_rtu.h

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


 */


#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

#include <plc.h>  /* get the plc data types */
#include <time.h> /* struct timespec data structure */

#include "mb_addr.h"  /* get definition of common variable types */



/***********************************************************************

	 Note: All functions used for sending or receiving data via
	       modbus return these return values.


	Returns:	string_length if OK
			-1 on internal error or port failure
			-2 on timeout
			-3 if a valid yet un-expected frame is received!
			-4 for modbus exception errors
			   (in this case exception code is returned in *error_code)

***********************************************************************/

#define PORT_FAILURE   -1
#define INTERNAL_ERROR -1
#define TIMEOUT        -2
#define INVALID_FRAME  -3
#define MODBUS_ERROR   -4


/* Error codes defined in modbus specification... */
#define ILLEGAL_FUNCTION -1
#define ILLEGAL_DATA_ADDRESS -2
#define ILLEGAL_DATA_VALUE -3
#define SLAVE_DEVICE_FAILURE -4
#define ACKNOWLEDGE -5
#define SLAVE_DEVICE_BUSY -6
#define NEGATIVE_ACKNOWLEDGE -7
#define MEMORY_PARITY_ERROR -8




/************************************************************************

	read_coil_status()

	reads the boolean status of coils and sets the array elements
	in the destination to TRUE or FALSE.

*************************************************************************/
inline int read_output_bits(u8  slave,
                            u16 start_addr,
                            u16 count,
                            int *dest,
                            int dest_size,
                            int fd,
                            int send_retries,
                            u8  *error_code,
                            const struct timespec *response_timeout);

#define read_coil_status(p1,p2,p3,p4,p5,p6,p7,p8,p9) \
        read_output_bits(p1,p2,p3,p4,p5,p6,p7,p8,p9)


inline int read_output_bits_u32(u8  slave,
                                u16 start_addr,
                                u16 count,
                                u32 *dest,
                                int fd,
                                int send_retries,
                                u8  *error_code,
                                const struct timespec *response_timeout);

#define read_coil_status_u32(p1,p2,p3,p4,p5,p6,p7,p8) \
        read_output_bits_u32(p1,p2,p3,p4,p5,p6,p7,p8)




/************************************************************************

	read_input_status()

	same as read_coil_status but reads the slaves input table.

************************************************************************/

inline int read_input_bits(u8  slave,
                           u16 start_addr,
                           u16 count,
                           int *dest,
                           int dest_size,
                           int fd,
                           int send_retries,
                           u8  *error_code,
                           const struct timespec *response_timeout);

#define read_input_status(p1,p2,p3,p4,p5,p6,p7,p8,p9) \
        read_input_bits  (p1,p2,p3,p4,p5,p6,p7,p8,p9)


inline int read_input_bits_u32(u8  slave,
                               u16 start_addr,
                               u16 count,
                               u32 *dest,
                               int fd,
                               int send_retries,
                               u8  *error_code,
                               const struct timespec *response_timeout);

#define read_input_status_u32(p1,p2,p3,p4,p5,p6,p7,p8) \
        read_input_bits_u32  (p1,p2,p3,p4,p5,p6,p7,p8)


/***********************************************************************

	read_holding_registers()

	Read the holding registers in a slave and put the data into
	an array.

************************************************************************/

#define MAX_READ_REGS 100

inline int read_output_words(u8  slave,
                             u16 start_addr,
                             u16 count,
                             u16 *dest,
                             int dest_size,
                             int fd,
                             int send_retries,
                             u8  *error_code,
                             const struct timespec *response_timeout);

#define read_holding_registers(p1,p2,p3,p4,p5,p6,p7,p8,p9) \
        read_output_words     (p1,p2,p3,p4,p5,p6,p7,p8,p9)


inline int read_output_words_u32(u8  slave,
                                 u16 start_addr,
                                 u16 count,
                                 u32 *dest,
                                 int fd,
                                 int send_retries,
                                 u8  *error_code,
                                 const struct timespec *response_timeout);

#define read_holding_registers_u32(p1,p2,p3,p4,p5,p6,p7,p8) \
        read_output_words_u32     (p1,p2,p3,p4,p5,p6,p7,p8)


/***********************************************************************

	read_input_registers()

	Read the inputg registers in a slave and put the data into
	an array.

***********************************************************************/

#define MAX_INPUT_REGS 100

inline int read_input_words(u8  slave,
                            u16 start_addr,
                            u16 count,
                            u16 *dest,
                            int dest_size,
                            int fd,
                            int send_retries,
                            u8  *error_code,
                            const struct timespec *response_timeout);

#define read_input_registers(p1,p2,p3,p4,p5,p6,p7,p8,p9) \
        read_input_words    (p1,p2,p3,p4,p5,p6,p7,p8,p9)



inline int read_input_words_u32(u8  slave,
                                u16 start_addr,
                                u16 count,
                                u32 *dest,
                                int fd,
                                int send_retries,
                                u8  *error_code,
                                const struct timespec *response_timeout);

#define read_input_registers_u32(p1,p2,p3,p4,p5,p6,p7,p8) \
        read_input_words_u32    (p1,p2,p3,p4,p5,p6,p7,p8)





/************************************************************************

	force_single_coil()

	turn on or off a single coil on the slave device.

************************************************************************/

inline int write_output_bit(u8  slave,
                            u16 coil_addr,
                            u16 state,
                            int fd,
                            int send_retries,
                            u8  *error_code,
                            const struct timespec *response_timeout);

#define force_single_coil(p1,p2,p3,p4,p5,p6,p7) \
        write_output_bit (p1,p2,p3,p4,p5,p6,p7)






/*************************************************************************

	preset_single_register()

	sets a value in one holding register in the slave device.

*************************************************************************/

inline int write_output_word(u8  slave,
                             u16 reg_addr,
                             u16 value,
                             int fd,
                             int send_retries,
                             u8  *error_code,
                             const struct timespec *response_timeout);

#define preset_single_register(p1,p2,p3,p4,p5,p6,p7) \
        write_output_word     (p1,p2,p3,p4,p5,p6,p7)





/*************************************************************************

	set_multiple_coils()

	Takes an array of ints and sets or resets the coils on a slave
	appropriatly.

**************************************************************************/

#define MAX_WRITE_COILS 800

int write_output_bits(u8  slave,
                      u16 start_addr,
                      u16 coil_count,
                      int *data,
                      int fd,
                      int send_retries,
                      u8  *error_code,
                      const struct timespec *response_timeout);

#define set_multiple_coils(p1,p2,p3,p4,p5,p6,p7,p8) \
        write_output_bits (p1,p2,p3,p4,p5,p6,p7,p8)


int write_output_bits_u32(u8  slave,
                          u16 start_addr,
                          u16 coil_count,
                          u32 *data,
                          int fd,
                          int send_retries,
                          u8  *error_code,
                          const struct timespec *response_timeout);

#define set_multiple_coils_u32(p1,p2,p3,p4,p5,p6,p7,p8) \
        write_output_bits_u32 (p1,p2,p3,p4,p5,p6,p7,p8)



/*************************************************************************

	preset_multiple_registers()

	copy the values in an array to an array on the slave.

*************************************************************************/

#define MAX_WRITE_REGS 100

int write_output_words(u8  slave,
                       u16 start_addr,
                       u16 reg_count,
                       u16 *data,
                       int fd,
                       int send_retries,
                       u8  *error_code,
                       const struct timespec *response_timeout);

#define preset_multiple_registers(p1,p2,p3,p4,p5,p6,p7,p8) \
        write_output_words       (p1,p2,p3,p4,p5,p6,p7,p8)



int write_output_words_u32(u8  slave,
                           u16 start_addr,
                           u16 reg_count,
                           u32 *data,
                           int fd,
                           int send_retries,
                           u8  *error_code,
                           const struct timespec *response_timeout);

#define preset_multiple_registers_u32(p1,p2,p3,p4,p5,p6,p7,p8) \
        write_output_words_u32       (p1,p2,p3,p4,p5,p6,p7,p8)






/************************************************************************

	initialise

	This function sets up the libraries
        (allocate memory for buffers, initialise data strcutures, etc)

**************************************************************************/
int mb_master_init(int nd_count);

int mb_master_done(void);




/***************************************************************************

	set_up_comms

	This function sets up (closes) a connection to a remote modbus
	slave.


***************************************************************************/
int mb_master_connect(node_addr_t node_addr);

int mb_master_close(int nd);




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
int mb_master_silence_init(void);



/***************************************************************************

	Get minimum acceptable timeout (in seconds).

	When using a serial connection, there may be a significant time
        lapse between the beginning and the end of a modbus frame.
        If the timeout given to write_*() and/or read_*() functions is
        smaller than this time lapse, then the function will abort in the
        middle of a possibly correct frame.

        This function returns the minimum timeout that is guaranteed
        to give the library enough time to read a correct frame. In
        essence, this function returns the time it takes to transmit
        the largest valid modbus frame, using the specified baud/...
        serial parameters.

        The TCP version of this function simply returns 0.


***************************************************************************/
double mb_get_min_timeout(int baud,
                          int parity,
                          int data_bits,
                          int stop_bits);




#endif  /* MODBUS_MASTER_H */








