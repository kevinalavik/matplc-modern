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
 *   state_setup.h
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "state_private.h"
#include "state_setup.h"

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


/* Setup all common resources required for the state library. */
/* In this case, allocate the state_shared_state_t block in
 * the cmm, and allocate the plc_synchplace_t used for the MatPLC
 * wide RUN/STOP mode.
 */
int state_setup(const char *module_name)
{
  state_shared_data_t *state_shared_shm;

  if (debug)
    printf("state_setup(): module_name = %s\n", module_name);

  /* create configuration shared memory block */
  state_shared_shm = cmm_block_alloc(STATE_SHARED_T_BLOCK_TYPE,
                                     STATE_SHARED_T_BLOCK_NAME,
                                     sizeof(state_shared_data_t));

  if (state_shared_shm == NULL) {
    plc_log_errmsg(9,"state_setup(): could not alloc config memory block.\n");
    goto error_exit_0;
  }

  /* allocate the plc_synchplace_t place for theMatPLC wide RUN/STOP mode */
  state_shared_shm->run_stop_place = plc_synchplace_add();
  if (!plc_synchplace_is_valid(state_shared_shm->run_stop_place))
    goto error_exit_1;

  /* initialize access to the state library */
  /* NOTE: The libraries setup after the state library do not
   *       require the state library to have been initialized,
   *       so we do not call the state_init() function.
   */
  /*
  return state_init(module_name);
  */

  return 0;

/*
error_exit_2:
  plc_synchplace_del(state_shared_shm->run_stop_place);
*/
error_exit_1:
  cmm_block_free(state_shared_shm);
error_exit_0:
  return -1;
}



/* delete all resources used by the state library */
int state_shutdown(void)
{
  int res;
  state_shared_data_t *state_shared_shm;
  u32 size;

  /* get hold of configuration shared memory block */
  state_shared_shm = cmm_block_find(STATE_SHARED_T_BLOCK_TYPE,
                                    STATE_SHARED_T_BLOCK_NAME,
                                    &size);

  if (state_shared_shm == NULL)
    return -1;

  /* If the following condition is false, we canot safely
   * access the state_shared_shm->... variables!!
   */
  if (size < sizeof(state_shared_data_t)) {
    cmm_block_free(state_shared_shm);
    return -1;
  }

  /* assume success... */
  res = 0;

  /* free the MatPLC wide RUN/STOP place */
  if (plc_synchplace_del(state_shared_shm->run_stop_place) < 0)
    res = -1;

  /* free the shared cmm block... */
  if (cmm_block_free(state_shared_shm) < 0)
    res = -1;

  return res;
}


/**************************************************************/
/**************************************************************/
/****                                                      ****/
/****        Local Auxiliary Funcitons.                    ****/
/****                                                      ****/
/**************************************************************/
/**************************************************************/


