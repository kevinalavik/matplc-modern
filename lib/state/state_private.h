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


#ifndef __STATE_PRIVATE_H
#define __STATE_PRIVATE_H

/* This file contains the private 'interface' of the state   */
/* sub-module, i.e. it contains the variables and type       */
/* definitions that are used internally by the state         */
/* sub-module, and should not be visible to outside users.   */


#include <unistd.h>  /* required for pid_t */
#include <synch/synch.h> /* required for plc_synchplace_t, etc... */


/*
 *   T h e    S T A T E    L I B R A R Y
 *  =====================================
 *  This library uses a single struct in the cmm to store its
 *  static MatPLC wide global variables.
 *  This is the state_shared_data_t, that keeps the ids of the
 *  plc_synchplace_t used for the MatPLC wide RUN/STOP mode.
 *
 *  It also uses an extra struct in the cmm for each running module.
 *  This extra struct (state_module_data_t) keeps the id of the
 *  process currently executing the module, as well as the
 *  plc_synchplace_t used for the module's own RUN/STOP mode.
 */

/************************************************************/
/*****************                          *****************/
/*****************   Global (to the state)  *****************/
/*****************    Variables and Type    *****************/
/***************** Definitions/Declarations *****************/
/*****************                          *****************/
/************************************************************/


/* layout of info stored in the state_block   */
/* one state_block for the whole MatPLC...    */
typedef struct {
   /* place used to control the MatPLC's run/stop mode. */
  plc_synchplace_t run_stop_place;
} state_shared_data_t;


/* one state_block for each running module... */
typedef struct {
   /* pid of process running the module */
  pid_t  pid;
   /* place used to control the module's run/stop mode. */
  plc_synchplace_t run_stop_place;
} state_module_data_t;


/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

/* the current status of the state library */
extern status_t state_status;

static const status_t state_fully_initialized = {0x0001};



/*****************************************************************/
/*****************                                ****************/
/*****************  Global (to state) Functions  ****************/
/*****************                                ****************/
/*****************************************************************/

/* function to parse the config file for STATE related values */
void state_parse_conf(int *module_max_count,
                     /*
                      * flag indicating if it should return the default values
                      * if the parameter has been left unspecified in the
                      * config file.
                      * 1 - yes, init to the default value if not specified
                      * 0 - no, leave parm. value unchanged if not specified
                      */
                    int use_defaults);


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* The number of extra arcs this library will insert into
 * the scan_beg synchpt in order to implement the RUN/STOP modes...
 *
 * The synch library will create the synchpts with enough free
 * space for at least these number of arcs to be added dynamically.
 *
 * We need 4 arcs: 2 for the module RUN/STOP mode, and 2 for thr
 * MatPLC wide RUN/STOP mode.
 */
#define STATE_LIB_EXTRA_ARCS 4



/* The block type used to store state_module_data_t struct */
#define STATE_MODULE_T_BLOCK_TYPE block_type_state_module

/* The block type used to store state_shared_data_t struct */
#define STATE_SHARED_T_BLOCK_TYPE block_type_state_shared

/* The block name used to store state_shared_data_t struct */
#define STATE_SHARED_T_BLOCK_NAME "state_shared_t"



/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/


#endif /* __STATE_PRIVATE_H */
