/*
 * (c) 2003 Stefan Staedtler, Heiko Patz
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

/* =====================================================================
 * $Id: io.c,v 1.3 2003/07/08 20:03:26 jorozco Exp $
 *
 * $Log: io.c,v $
 * Revision 1.3  2003/07/08 20:03:26  jorozco
 * Staedtler and Patz changes
 *
 *
 * ===================================================================== 
 * Funktionen:
 * ===================================================================== 
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include <string.h>
#include <signal.h>

#include <plc.h>
#include <logic/timer.h>

enum t_boolean {false=0,true=1};

plc_pt_t get_pt(const char *pt_name)
{
  plc_pt_t pt_handle;

  pt_handle = plc_pt_by_name(pt_name);

  if (!pt_handle.valid) {
    printf("Could not get valid handle to %s.\n", pt_name);
    exit(1);
  }

  return pt_handle;
}


/*
========================================================================
	M A I N
========================================================================
*/

int main(int argc, char **argv) 
{

	// Allgemeines
    //const char 		*author = "Patz/Staedtler";
    int 			i;
    int 			count=0;
    int 			rotcount=0;
    double 			x1=0;
	enum t_boolean 		start=false;

    plc_pt_t in[4];
    plc_pt_t out_io[4];
    plc_pt_t xcoordinate1;
    plc_pt_t y1coordinate;
    plc_pt_t startplot;
    plc_pt_t stopplot;
    plc_pt_t resetplot;
    plc_pt_t rotation;
	
	plc_pt_t in4,out4f,out4u,out4i;
	
	char 	 ptname[40];
	u32		 u32_tmp;
	f32		 f32_tmp;
	i32      i32_tmp;
	
#define DEBUG
#undef DEBUG

	// ---------------------------------------------------
	// ---------------------------------------------------

	const char *module_name = "io";

	/* printf("Initializing.\n"); */
	if (plc_init(module_name, argc, argv) < 0) {
		printf("Error initializing PLC\n");
		return -1;
	}
	
	// ---------------------------------------------------
	// ---------------------------------------------------

#ifdef DEBUG
    printf("Start ...\n");
#endif

	for (i=0; i<3; i++) {
		sprintf(ptname,"in%d",i+1);
		in[i]=get_pt(ptname);
		
		sprintf(ptname,"out%d_io",i+1);
		out_io[i]=get_pt(ptname);
	}
	
	in4=get_pt("in4");
	out4f=get_pt("out4f_io");
	out4u=get_pt("out4u_io");
	out4i=get_pt("out4i_io");
	xcoordinate1=get_pt("xcoord1");
	y1coordinate=get_pt("sinus");
	startplot=get_pt("startplot");
	stopplot=get_pt("stopplot");
	resetplot=get_pt("resetplot");
	rotation=get_pt("rotation");

	start=false;
	count=0;
	x1=0;
	f32_tmp=0;			
	plc_set_f32(y1coordinate,(sin(x1)+1));
	plc_set_f32(xcoordinate1,x1);
		
	while (1) {
		/* begin of scan */
		plc_scan_beg();
		plc_update();

		for (i=0; i<3; i++) {
				u32_tmp=plc_get(in[i]);
				plc_set(out_io[i],u32_tmp);
		}

		u32_tmp=plc_get(in4);
		f32_tmp=*(f32*)&u32_tmp;
		
		u32_tmp=f32_tmp;
		i32_tmp=f32_tmp;
		
		plc_set(out4f,*(u32*)&f32_tmp);
		plc_set(out4u,*(u32*)&u32_tmp);
		plc_set(out4i,*(u32*)&i32_tmp);

		if( (plc_get(startplot)) && (start==false)){
			start=true;
			#ifdef DEBUG
				printf("io.c: Startplot made\n");
			#endif
		}

		if( (plc_get(stopplot)) && (start==true)){
			#ifdef DEBUG
				printf("io.c: Stopplot made\n");
			#endif
			start=false;
		}
		
		if( plc_get(resetplot)){
			start=false;
			count=0;
			x1=0;
			plc_set_f32(y1coordinate,(sin(x1)+1));
			plc_set_f32(xcoordinate1,x1);
			#ifdef DEBUG
				printf("io.c: Resetplot made\n");
			#endif
		}
		
		if(start==true) count++;

		if(count>10){
			x1+=0.25;
			plc_set_f32(y1coordinate,(sin(x1)+1));
			plc_set_f32(xcoordinate1,x1);
			count=0;
			
		}

		if (((count % 4) == 0 ) && (start==true)) {
			plc_set(rotation,*(u32 *)&rotcount);
			rotcount++;
			if (rotcount>3) rotcount=0;
		}
		
		/* end of scan */
		plc_update();
		plc_scan_end();
	}
}
