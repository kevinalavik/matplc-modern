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


#ifndef MB_UTIL_H
#define MB_UTIL_H

/* This file has constants related to the modbus protocol */
/*
 * Some of these constants are specific to the layer two protocols
 * (i.e. master and slave), while others are specific of the
 * layer one protocols (i.e. rtu, ascii, tcp).
 *
 * a) Unfortunately, due to the nature of the modbus protocol, that does not
 * include a frame size field in the layer 1 frame (see note 1), and the
 * fact that we are implementing it at the user level, the implementation
 * of some layer 1 protocols need to know the content of the layer 2 protocol
 * in order to determine the size of the frame.
 *
 * b) The layer two message formats are in fact the same, just reversing the role
 * being played (master or slave).
 *
 * Bothe a) and b) mean we need the same modbus protocol constants in several files.
 * It ends up making more sense to put them all together in a single file, which
 * makes updating easier, even though we are trying to strictly seperate the layer 1
 * and layer 2 protocols.
 *
 *
 *
 * Notes:
 *  (1) There is no layer 1 field with the frame size, nevertheless this
 *      size can be determined indirectly due to timing restrictions on the rtu
 *      protocol. Unfortunately, due to the fact that we are implementing
 *      it at the user level, we are not guaranteed to detect these timings
 *      correctly, and therefore have to rely on layer 2 protocol info to
 *      determine the frame size.
 *      For the ascii protocol, the frame size is determined indirectly by
 *      a frame header and tail, so we do not use layer 2 protocol info.
 */


 /* Layer 2 Frame Structure...                */
 /* Valid for both master and slave protocols */
#define L2_FRAME_HEADER_LENGTH    6
#define L2_FRAME_BYTECOUNT_LENGTH 1
#define L2_FRAME_DATABYTES_LENGTH 255
#define MAX_L2_FRAME_LENGTH (L2_FRAME_HEADER_LENGTH + L2_FRAME_BYTECOUNT_LENGTH +    \
                             L2_FRAME_DATABYTES_LENGTH)

#define L2_FRAME_SLAVEID_OFS    0
#define L2_FRAME_FUNCTION_OFS   1

 /* Layer 1 - Ascii Frame sizes... */
#define L2_TO_ASC_CODING        2 /* number of ascii bytes used to code a Layer 2 frame byte */
#define ASC_FRAME_HEADER_LENGTH 1
#define ASC_FRAME_HEADER        ':'
#define ASC_FRAME_TAIL_LENGTH   2
#define ASC_FRAME_TAIL_0        '\13' /* 'CR' */
#define ASC_FRAME_TAIL_1        '\10' /* 'LF' */
#define ASC_FRAME_LRC_LENGTH    2

 /* Layer 1 - RTU Frame sizes... */
#define RTU_FRAME_CRC_LENGTH    2

 /* Layer 1 - TCP Frame sizes... */
#define TCP_HEADER_LENGTH       6

 /* Global Frame sizes */
#define MAX_RTU_FRAME_LENGTH MAX_L2_FRAME_LENGTH + RTU_FRAME_CRC_LENGTH
#define MAX_ASC_FRAME_LENGTH ((MAX_L2_FRAME_LENGTH * L2_TO_ASC_CODING) +             \
                              ASC_FRAME_HEADER_LENGTH + ASC_FRAME_TAIL_LENGTH +      \
                              ASC_FRAME_LRC_LENGTH)

#endif  /* MB_UTIL_H */








