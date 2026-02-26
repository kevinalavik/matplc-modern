/*
 * (c) 2002 Hugh Jack
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
 * a simple test program for the DAS08 DAQ board
 *
 * Last Revised: April 22, 2002
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


plc_pt_t	data_portA_IN;	// hooks to global values
plc_pt_t	data_portA_OUT;
plc_pt_t	data_portB_IN;
plc_pt_t	data_portB_OUT;
plc_pt_t	data_portCL_IN;
plc_pt_t	data_portCL_OUT;
plc_pt_t	data_portCH_IN;
plc_pt_t	data_portCH_OUT;
plc_pt_t	data_portXI;
plc_pt_t	data_portXO;
plc_pt_t	data_AI0;
plc_pt_t	data_AI1;
plc_pt_t	data_AI2;
plc_pt_t	data_AI3;
plc_pt_t	data_AI4;
plc_pt_t	data_AI5;
plc_pt_t	data_AI6;
plc_pt_t	data_AI7;
plc_pt_t	data_AO0;
plc_pt_t	data_AO1;


int	count;


int init(){
	data_portA_IN = plc_pt_by_name("das08portA_IN");
	data_portA_OUT = plc_pt_by_name("das08portA_OUT");
	data_portB_IN = plc_pt_by_name("das08portB_IN");
	data_portB_OUT = plc_pt_by_name("das08portB_OUT");
	data_portCL_IN = plc_pt_by_name("das08portCL_IN");
	data_portCL_OUT = plc_pt_by_name("das08portCL_OUT");
	data_portCH_IN = plc_pt_by_name("das08portCH_IN");
	data_portCH_OUT = plc_pt_by_name("das08portCH_OUT");
	data_portXI = plc_pt_by_name("das08portXI");
	data_portXO = plc_pt_by_name("das08portXO");
	data_AI0 = plc_pt_by_name("das08AI0");
	data_AI1 = plc_pt_by_name("das08AI1");
	data_AI2 = plc_pt_by_name("das08AI2");
	data_AI3 = plc_pt_by_name("das08AI3");
	data_AI4 = plc_pt_by_name("das08AI4");
	data_AI5 = plc_pt_by_name("das08AI5");
	data_AI6 = plc_pt_by_name("das08AI6");
	data_AI7 = plc_pt_by_name("das08AI7");
	data_AO0 = plc_pt_by_name("das08AO0");
	data_AO1 = plc_pt_by_name("das08AO1");

	count = 0;

	return NO_ERROR;
}



int step_run(){
	count++;
	if(count == 20) send_message("QUIT");

	plc_set(data_portA_OUT, plc_get(data_portA_OUT) + 1);

	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif

