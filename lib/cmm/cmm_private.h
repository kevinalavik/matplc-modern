/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
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


#ifndef __CMM_PRIVATE_H
#define __CMM_PRIVATE_H

#include <misc/mutex_util.h>
#include "plc_private.h"
#include "cmm.h"


/* This file contains the private 'interface' of the cmm     */
/* i.e. it contains the variables and type definitions that  */
/* are used internally by the cmm, and should not be visible */
/* to outside users.                                         */



/************************************************************/
/*****************                          *****************/
/*****************    Global (to the cmm)   *****************/
/*****************    Variables and Type    *****************/
/***************** Definitions/Declarations *****************/
/*****************                          *****************/
/************************************************************/




/* layout of a cmm_block_t */

typedef struct {
    cmm_block_type_t type;
    char             name[CMM_NAME_MAX_LEN];

    u32 next; /* offset into config mem. of next block in list */
    u32 prev; /* offset into config mem. of previous block in list */

    u32 size; /* size in bytes */
} cmm_block_t;


/***********************************************************/
/*                                                         */
/* layout of Configuration Shared Memory                   */
/*                                                         */
/* NOTE:                                                   */
/* There are two shared memory areas for a matplc.       */
/* The globalmap stores the plc state, i.e. points, ...    */
/* The configuration area stores the plc's config.         */
/*                                                         */
/* The CMM (Configuration Memory Manager) ony manages the  */
/* configuration shared memory area.                       */
/*                                                         */
/* The configuration area starts with a cmm_t structure.   */
/* This structure is imediately followed by the confmap.   */
/* The confmap is a doubly linked list of cmm_block_t, in  */
/* which each cmm_block_t is imediately followed by a      */
/* char [cmm_block_t.size array] of bytes. The structure   */
/* of the data stored within these bytes is the sole       */
/* responsibility of the function/lib that requested it's  */
/* allocation.                                             */
/*                                                         */
/*
 * The whole confmap is at first initialised with a single
 * cmm_block_t of free memory (block_type_free). New
 * blocks are created by dividing a block of free memory
 * into two, one being the new block, and the other a block
 * with the remaining free memory.
 * This means that all of the confmap memory is always
 * allocated to one cmm_block_t, even though the memory
 * may be free.
 * When a block is freed, it is first merged with the
 * following block it it, too, is free memory. We then
 * merge the resulting block with the previous block if
 * it, too, is free. With this algorithm we are guaranteed
 * that we will never have two consecutive blocks of free
 * memory.
 */
/*                                                         */
/* The above memory area is created and destroyed by       */
/* the setup functions defined in cmm_setup.h              */
/*                                                         */
typedef struct {
  i32 magic;
  u32 confmap_size; /* how many bytes for the conf map in shared memory */
  plcmutex_id_t mutexid; /* the key of the (CMM-use) semaphore set */

  u32 first_block;
  u32 last_block;

/* we should probably reserve space for future upgrades without */
/* breaking old versions...                                     */
} cmm_t;

/* NOTE: the mutexid above is the semaphore used internally by the CMM.  */
/* Other libraries will allocate the semaphores they use themselves.      */

/* variables refering to the memory areas that need to be common */
extern cmm_t *cmm_shm_;   /* plc configuration memory area */


/**************************************************************/
/*****************                             ****************/
/*****************  Global (to cmm) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

/* function to parse the config file for CMM related values */
/*
 * Note: this function will only really look for the confmap
 *       if this variable has been initialized to something < 0 (e.g. -1).
 */
void cmm_parse_conf(int *confmap_key,
                    u32 *confmap_size,
                     /*
                      * flag indicating if it should return the default values
                      * if the parameter has been left unspecified in the
                      * config file.
                      * 1 - yes, init to the default value if not specified
                      * 0 - no, leave parm. value unchanged if not specified
                      */
                    int use_defaults);


/* return the cmm sem_key being used */
plcmutex_id_t *cmm_mutex_id_ptr(void);




/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* magic value stored in cmm_t.magic */
static const i32 CMM_MAGIC = 0x3c9bd4b1;

/* Default semaphore key */
/*  Use 0 for random key */
#define CMM_DEF_SEM_KEY 0

/* Default confmap shmem key */
/*  Use 0 for random key */
#define CMM_DEF_CONFMAP_KEY DEF_PLC_ID

/* Default number of bytes for confmap */
/* Must be > 0 */
#define CMM_DEF_CONFMAP_PG (8*1024)  /* default of 8 kBytes */


/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/

/* confmap shmem key */
#define CONF_CK_NAME "confmap_key"     /* name to search for in conf file    */
#define CONF_CK_SECTION "PLC"          /* section to search for in conf file */
#define CONF_CK_MIN  0                 /* minimum acceptable value           */
#define CONF_CK_MAX  INT_MAX           /* maximum acceptable value           */

/* num. of bytes for confmap */
#define CONF_CP_NAME "confmap_size"
#define CONF_CP_SECTION "PLC"
#define CONF_CP_MIN  1
#define CONF_CP_MAX  UINT_MAX

#endif /* __CMM_PRIVATE_H */
