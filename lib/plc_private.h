/*
 * (c) 2000 Mario de Sousa
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


#ifndef __PLC_PRIVATE_H
#define __PLC_PRIVATE_H

#include "gmm/gmm.h"   /* required for gmm_loc_t */


/* This file contains the private default values for most of
 *  the plc defaults and hardcoded constants (at least, it should...)
 */

/*
 * NOTE: This file is not included by the modules using
 *       the plc, so any defines in here do not foul up those
 *       modules' name-spaces.
 */



/************************************************************/
/*****************                          *****************/
/*****************    Global (to the plc)   *****************/
/*****************           Defaults       *****************/
/***************** Definitions/Declarations *****************/
/*****************                          *****************/
/************************************************************/


#define DEF_CONFIG_FILE "matplc.conf"

#define DEF_LOG_FILE     "matplc.log"
#define DEF_TRC_LOG_FILE DEF_LOG_FILE
#define DEF_WRN_LOG_FILE DEF_LOG_FILE
#define DEF_ERR_LOG_FILE DEF_LOG_FILE

#define DEF_LOG_LEVEL  1

/* must be >= 0 */
/* if 0, then use random number on plc_setup, and give up on plc_init */
#define DEF_PLC_ID   23

/* Default initial value of every plc point. */
/*  Note that this is only the default. The initial value of a plc point
    may be spefied by the user in the config file...
 */
#define DEF_PLC_PT_VALUE 0


/* By default, we do not force the initialisation of a new
 * module (i.e. process) if another process was found that is
 * seemingly running a PLC module with the same name.
 */
#define DEF_FORCE_INIT 0;


  /* The name of the table used to configure the modules */
#define START_MODULES_TABLE_NAME "module"
#define START_MODULES_TABLE_SEC  "PLC"

/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/
/* Configuration of maximum number of simultaneously running modules... */
#define PLC_MAXMODULECOUNT_SECTION  "PLC"
#define PLC_MAXMODULECOUNT_NAME     "max_modules"
#define PLC_MAXMODULECOUNT_MIN      1
#define PLC_MAXMODULECOUNT_MAX      INT_MAX
#define PLC_MAXMODULECOUNT_DEF      20


/**************************************************************/
/*****************                             ****************/
/*****************  Provate (to plc) Functions ****************/
/*****************                             ****************/
/**************************************************************/

int plc_parse_args(int        argc,
                   char       **argv,
                   gmm_loc_t  *location,
                   int        *local_shm_key,
                   int        *confmap_shm_key, /* a.k.a. the plc_id */
                   const char **conffile,
                   char       **mod_name,
                   int        *log_level,
                   const char **log_file,
                   int        *force_init,
                   int        remove_args  /* set to 1 if args should   */
                                           /* be removed from **argv    */
                   );


#endif /* __PLC_PRIVATE_H */
