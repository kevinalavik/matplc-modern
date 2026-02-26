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


#ifndef __STATE_H
#define __STATE_H

#include <stdlib.h>
#include <unistd.h>  /* required for pid_t */

#include "../types.h"
#include <synch/synch.h>

/*** GENERAL ***/



int state_init(const char *module_name, int force_init);

int state_done(void);



/*************************************************************************/
/*                                                                       */
/*  Static functions, that may be called _before_ the library            */
/*  has been initialised (by calling state_init())                       */
/*                                                                       */
/*************************************************************************/

/* These functions are only supposed to be used by the synch library,
 * (during it's initialisation...).
 * Since the synch library gets initialised before the state library,
 * these functions must be 'static' (as in static functions in C++ classes)
 */


/*
 * Get the number of extra arcs the state library will be adding to the
 * scanbeg synchpt...
 */
int plc_scanbeg_extra_arcs(void);



/*************************************************************************/
/*                                                                       */
/*  Normal functions, that will only work correclty after the library    */
/*  has been initialised by calling state_init() ...                     */
/*                                                                       */
/*************************************************************************/

/*
 * Get the module name by index
 */
int plc_module_by_index(int module_index, char **module_name);


/*
 *  Shutdown a module
 */
int plc_module_shutdown(char *name);


/*
 * Shutdown all the modules
 * (except the calling process, if it too is a running module)
 * The calling process can easily kill itself by simply calling exit()
 * if it so wishes.
 */
int plc_module_shutdown_all(void);



/*
 * Get the number of currently installed modules
 */
int plc_module_count(void);

/* place module in run mode */
int plc_module_run(const char *name /* the module name... */);

/* place module in stop mode */
int plc_module_stop(const char *name /* the module name... */);

/* Place the MatPLC in run mode... */
int plc_run(void);

/* Place the MatPLC in stop mode... */
int plc_stop(void);

/*
 * Obtain info on running modules...
 */
int plc_module_details(const char *name, /* the module name - compulsory          */
		        pid_t *pid,      /* the module's pid                      */
                                 /* the place being used for the RUN/STOP mode... */
                        plc_synchplace_t *place,
                        int *mode,       /* The current RUN=1 / STOP=0 mode...    */
                        int *active      /* does the process seem to be running ? */
                       );





#endif /* __STATE_H */
