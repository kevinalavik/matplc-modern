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


#ifndef __NONLINEAR_T_H
#define __NONLINEAR_T_H

/********************************************************/
/*****************                      *****************/
/*****************    Global (to dsp)   *****************/
/*****************   Type Definitions   *****************/
/*****************                      *****************/
/********************************************************/


typedef struct {
    /* Input plc point */
  plc_pt_t  in_pt;

    /* Output plc point */
  plc_pt_t out_pt;

    /* Algorithm Parameters */
  f32 deadband_bot, deadband_top, deadband_out;
  f32 cutoff_bot, cutoff_top;
  f32 abs_gain;

} fb_nonlinear_t;


/*
  This function block implements a nonlinear block supporting both
  a deadband function centered around a configured offset, and a limiter.
  In addition, it also allows for a linear gain and offset top be applied
  the output of it's nonlinear function. These last two are essentially to
  reduce the number of function blocks eventually required to implement
  a specific global user function.



  The nonlinear part, implements the following function:


     out = nl_f(in):
			   out
			    ^
			    |
			    |
		     co_top |.............................--------
			    |                            /
			    |                           /
			    |                          /
			    |                         /
			    |                        /
			    |                       /
			    |                      /
			    |                     / inclination = 1
			    |                    /
			    |                   /
		     db_out |...----------------
			    |  /.              .
			    | / .              .
			    |/  .              .
			    |   .              .
		   	   /|   .              .
		          / |   .              .
		         /  |   .              .
		        /   |   .              .
  <--------------------------------------------------------------> in
		      /     | db_bot         db_top
		     /      |
		    /       |
		   /        |
   ----------------.........| co_bot
			    |
			    |
			    |
			    v

	(co = cutoff      db = deadband)


  The linear part implements the following function:
   out = l_f(in) = in * gain


  The output of the complete nonlinear block is:
   out =  nl_f( in * gain )
*/

/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/*
 * Identifier used in the first column of the matplc.conf fblock
 * table that identifies the row as defining a pid block
 */
#define NONLINEAR_TABLE_ID     "nonlinear"


#endif /* __NONLINEAR_T_H */
