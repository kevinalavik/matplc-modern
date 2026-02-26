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


#ifndef MODBUS_MASTER_PRIVATE_H
#define MODBUS_MASTER_PRIVATE_H

#include <plc.h>  /* get the plc data types */
#include "mb_util.h"
#include "mb_master.h"


#define QUERY_BUFFER_SIZE       MAX_L2_FRAME_LENGTH

#define DEF_LAYER2_SEND_RETRIES 1

#define DEF_IGNORE_ECHO         0

#define DEF_OPTIMIZATION        optimize_speed


#endif  /* MODBUS_MASTER_PRIVATE_H */








