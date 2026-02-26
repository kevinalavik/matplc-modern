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


/*
 * This file implements the functions in
 *   period_setup.h
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "period.h"
#include "period_private.h"
#include "period_setup.h"

#include <misc/string_util.h>
#include <conffile/conffile.h>
#include <cmm/cmm.h>
#include <log/log.h>


static int debug = 0;



/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****        The main setup and shutdown functions.        ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


/* Setup all common resources required for the period library. */
/* In this case, do nothing!                                   */
/*
 * NOTE: At first glance it may seem that we should be reading
 *       the period from the configuration file and seting up
 *       the period memroy block in the cmm.
 *       The above would be WRONG!
 *       This function is called by the plc_setup() function which
 *       is in turn called by the matplc utility to setup any
 *       common resources used throught the PLC. The period a module
 *       uses is module specific, so it is the module that should
 *       initialise the cmm memory block, and not the matplc utility.
 *       Note that there will be one period memory block for each
 *       running module! These blocks are initiliased by the module
 *       itself in the period_init() function.
 */
int period_setup(const char *module_name)
{
  if (debug)
    printf("period_setup(): module_name = %s\n", module_name);

  /* initialize access to the period library */
  /* NOTE 1: At the moment plc setup does not require that the period
   *         library be initialised. In other words, the setup of library
   *         modules setup after the period library do not need to use
   *         the resources/facilities provided by the period library.
   *         Because of the above, we do not call period_init().
   *
   * NOTE 2: For some reason, it seems that calling timer_create() with
   *         an event type of SIGEV_SIGNAL (i.e. warn of expiring timers
   *         using an OS signal) followed by a fork(), will cause further calls
   *         to execvp() by the child process to block. Something to do with
   *         signal handling by child processes??
   *         Anyway, this is another reason for not calling period_init().
   *         period_init() makes a call to timer_create using SIGEV_SIGNAL.
   *         plc_setup() will later call fork() and execvp() in the child
   *         process to launcht modules. If we call period_init() then
   *         modules will npot be launched succesfully.
   *
   */
  /*
  return period_init(module_name);
  */

  return 0;
}


/* delete all resources used by the period library */
int period_shutdown(void)
{
  /* we have no OS resources (semaphores, shared memory areas, etc
   * to delete!
   */
  return 0;
}


/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****        Local Auxiliary Funcitons.                    ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


