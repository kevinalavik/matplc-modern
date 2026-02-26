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


#ifndef __PERIOD_PRIVATE_H
#define __PERIOD_PRIVATE_H


/* This file contains the private 'interface' of the period  */
/* sub-module, i.e. it contains the variables and type       */
/* definitions that are used internally by the period        */
/* sub-module, and should not be visible to outside users.   */


/************************************************************/
/*****************                          *****************/
/*****************   Global (to the period) *****************/
/*****************    Variables and Type    *****************/
/***************** Definitions/Declarations *****************/
/*****************                          *****************/
/************************************************************/


/* layout of info stored in the period_block   */
/* one period_block for each running module... */
/*
 * NOTE: We could have defined the following variables as global
 *       variables with access only to this library
 *       (e.g. static u32 period; ...), but this would not
 *       support online changes to a module.
 *       Online change to a module will consist of killing off
 *       the process executing the module with the old parameters,
 *       and starting a new process with the new parameters. This means
 *       any module state that has to outlive the process' life needs
 *       to be placed in shared memory, and cannot be go into the
 *       process' stack, as global variables do.
 *       The period needs to outlive the process' life, as it may be
 *       changed at any moment, and so it may no longer
 *       be loaded from the config file once the module
 *       has been started at least once.
 */
typedef struct {
  u32  period_sec;    /* period in seconds.
                       * with u32 we can go up to more than 136 years.
                       */
  u32  period_nsec;   /* period in nano seconds.
                       * complete period will be the sum of the two period variables..
                       */
  i8 first_scan;      /* 1 -> this is the first scan. skip the sleep() function */
                      /* 0 -> not the first scan. Must do the sleep() function  */
} period_t;


/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

/* the current status of the period library */
extern status_t period_status;

static const status_t period_fully_initialized = {0x0001};



/*****************************************************************/
/*****************                                ****************/
/*****************  Global (to period) Functions  ****************/
/*****************                                ****************/
/*****************************************************************/



/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* The block type used to store period_period_t struct */
#define PERIOD_T_BLOCK_TYPE block_type_period

/* Default Period */
/*  Use 0 for no delay */
#define DEF_SCAN_PERIOD  0


/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/

#define SCAN_PERIOD_NAME   "scan_period"     /* name to search for in conf file    */
#define SCAN_PERIOD_DEF     DEF_SCAN_PERIOD  /* default value for parameter        */
#define SCAN_PERIOD_MIN     0                /* minimum valid value for parameter  */
#define SCAN_PERIOD_MAX     f32_MAX          /* maximum valid value for parameter  */

#endif /* __PERIOD_PRIVATE_H */
