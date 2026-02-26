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



/***************************************************/
/*                                                 */
/*        The    G M M                             */
/*                                                 */
/*        Global Memory Manager                    */
/*                                                 */
/***************************************************/


#ifndef __GMM_PRIVATE_H
#define __GMM_PRIVATE_H

#include "plc_private.h"
#include "gmm_local.h"
#include "gmm_isolate.h"
#include "gmm_shared.h"
#include "gmm.h"
#include <cmm/cmm.h>  /* needed for CMM_NAME_MAX_LEN */
#include <misc/mutex_util.h>  /* needed for plcmutex_id_t */


#define GMM_NAME_MAX_LEN  CMM_NAME_MAX_LEN
/* A prefix to add to the module name in order to generate the
 * new module name of the proxy process.
 */
#define GMM_PROXY_MODULE_NAME_PREF "_P"
#define GMM_OWNER_MAX_LEN GMM_NAME_MAX_LEN



/* This file contains the private 'interface' of the gmm       */
/* i.e. it contains the variables and type definitions that    */
/* are used internally by the gmm, and should not be           */
/* visible to outside users.                                   */



/****************************************************************/
/*****************                              *****************/
/*****************      Global (to the gmm)     *****************/
/*****************      Variables and Type      *****************/
/*****************   Definitions/Declarations   *****************/
/*****************                              *****************/
/****************************************************************/

/* name of this module, as given to plc_init() or from the command line */
extern const char *gmm_module_name_;


/* layout of a plc point handle (plc_pt_t) */
/* Note: must be compatible with plc_pt_t defined in gmm.h */
typedef struct {
    i16 magic;

    i32 ofs; /* offset into the data area */
    u32 mask; /* mask */
    u8  bit_shift; /* the number of bits the mask has to be
                    *  shifted right (>>) so that all bits set
                    *  to 1 are right justified.
                    */
    u8  length;    /* number of valid bits in the mask */
} plc_pt_priv_t;


/* layout of info stored in the cmm_block for a point */
typedef struct {
  char owner[GMM_OWNER_MAX_LEN+1];
  i32  offset;
  u8   bit;
  u8   length;
} gmm_pt_conf_t;



/* 
 * Modularity - for local vs. isolate vs. shared access to the shared map.
 *
 * A variant is characterised by the "init" function, which then sets the
 * other fields to the correct values.
 *
 * This is actually a way of implementing C++ virtual class methods in C
 */
typedef struct {
  int (*init) (int);
  int (*pt_count) (void);
  plc_pt_t (*pt_by_name) (const char *);
  plc_pt_t (*pt_by_index) (int, char **, char **);
  plc_pt_t (*pt_by_loc) (i32 offset, u8 bit, u8 length);
  int (*update) (void);
  int (*update_pt) (plc_pt_t);
  int (*update_pts) (plc_pt_t[], int);
  int (*done) (void);
} gmm_mod_t;

/* global variable - which module is in use? */
/* The only variable of gmm_mod_t type.      */
extern gmm_mod_t gmm_mod_;


/* layout of shared memory                                 */
/*                                                         */
/* NOTE:                                                   */
/* There are two shared memory areas for a matplc.       */
/* The globalmap stores the plc state, i.e. points, ...    */
/* The configuration area stores the plc's config.         */
/*                                                         */
/* The gmm only manages the globalmap, and other memory    */
/* areas that are private (to the module) and also related */
/* to the module's private copy of the plc state. Some of  */
/* these private memory areas may or may not be shared,    */
/* depending on the location (--PLClocal, ...) the module  */
/* is configured to use.                                   */
/* The configuration memory area is managed by the cmm.    */
/*                                                         */
/* The globalmap is merely a collection of bytes and bits  */
/*  whose definition is stored in the confmap. No special  */
/*  registers or magic ints are stored there               */
/*  (we should probably put a magic there?)                */
/*  This memory area may be referred to as the globmap.    */
/*                                                         */
/* The above memory area is created and destroyed by       */
/* the setup functions defined in gmm_setup.h              */
/*                                                         */
/* A second and third memory areas are created by          */
/* gmm_init(). Each module will have it's own private      */
/* copy of these memory areas.                             */
/*                                                         */
/* The Second memory area is a private copy of the plc     */
/* state (i.e. points, ...). This copy is sincronized with */
/* the globalmap by plc_update(). This memory area is      */
/* known as the privatemap or privmap. As this memory area */
/* is merely a local copy of the globalmap, it is also     */
/* just a collection of bytes and bits. No special         */
/* registers or magic ints are stored there.               */
/*                                                         */
/* The Third memory area is a mask that contains which     */
/* bits in the globalmap the module has write access to.   */
/* This memory area is known as the mapmask  and also has  */
/* no special registers or magic ints stored there.        */
typedef struct {
  i32 magic;
  int globalmap_key; /* key to global map shared memory */
  int globalmap_ofs; /* offset of global map in shared memory */
  u32 globalmap_size; /* The size in bytes of the global map in shared memory */
  plcmutex_id_t mutexid; /* the key of the (GMM-use) semaphore set */
/* we should probably reserve space for future upgrades without */
/* breaking old versions...                                     */
} gmm_t;

/* NOTE: the semid above is the semaphore set used internally by the GMM.  */

/* variables refering to the memory areas that need to be common */
/* Note: only the local version of the gmm has direct access to  */
/*       the globalmap, so the gmm_globmap_ pointer is defined   */
/*       in the gmm_local.c file                                 */
extern gmm_t *gmm_shm_; /* gmm configuration memory area */
extern u32 *gmm_privmap_; /* private map                   */
extern u32 *gmm_mapmask_; /* read/write mask for the map   */

extern i32 gmm_globalmap_len_;  /* in u32's */
extern u32 gmm_globalmap_size_; /* in bytes */


/*
 * Get the type-checker to help us catch bugs; unfortunately then we can't
 * combine bits (easily), but we aren't doing that now, and when need it,
 * we can easily just go back to using u16 as the type for status_
 */
typedef struct {u16 s;} status_t;

/* the current status of the gmm library (how far did init get?) */
extern status_t gmm_status_;

static const status_t gmm_fully_initialized      = {0x0001};
static const status_t attached_globmap           = {0x0002};
static const status_t attached_privmap           = {0x0004};
static const status_t created_privmap            = {0x0008};
static const status_t malloced_privmap           = {0x0010};
static const status_t malloced_mapmask           = {0x0020};
static const status_t launched_proxy_process     = {0x0040};
static const status_t initialized_proxy          = {0x0080};
static const status_t attached_conf_struct       = {0x0100};
static const status_t opened_globsem       = {0x0200};



/**************************************************************/
/*****************                             ****************/
/*****************  Global (to gmm) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

/* access functions for the status_ variable */
int gmm_get_status(const status_t stat_bit);
void gmm_set_status(const status_t stat_bit);
void gmm_rst_status(const status_t stat_bit);

/* error exit function for the gmm_init_xxx() functions */
int gmm_init_error(status_t status_bit);

/* gmm_t gmm_shm_ struct initializer function */
int gmm_init_conf_struct(void);

/* return number of points stored in the config memory area */
int gmm_config_pt_numb(void);

/* store point config in config memory area */
int gmm_insert_pt_cmm(const char *name,
                      const char *owner,
                      i32 offset,
                      u8 bit,
                      u8 length);


/* remove point config from config memory area */
int gmm_remove_pt_cmm(const char *name);


/* get point config from the config memory area */
/* returns 0 if succesfull, -1 otherwise. */
int gmm_config_pt_by_name(const char*name,
                          i32 *offset,
                          u8  *bit,
                          u8  *length);


/* get point config from the config memory area */
/* returns 0 if succesfull, -1 otherwise. */
int gmm_config_pt_by_index(i16 pt_index,
                           char **name,
                           char **owner,
                           i32  *offset,
                           u8   *bit,
                           u8   *length);


/* and a r/w mapmask area init function */
/* Uses the owner to determine whether we have write acces to a point or not */
int gmm_init_mapmask(const char *module, u32 *mapmask, int mapsize);


/* get write acces to a point, regardless of it's owner module.        */
/*  This is used by gmm_setup() to set the initial value of a plc_pt.  */
int gmm_set_pt_w_perm(plc_pt_t p);


/* Function to obtain the size, in bytes, of the global map. */
/* Returns 0 on success, -1 on failure.                      */
int gmm_globalmap_size(u32 *size);


/* Function to obtain the length, in sizeof(u32), of the global map. */
/* Returns 0 on success, -1 on failure.                              */
int gmm_globalmap_len(i32 *len);


/* function to parse the config file for GMM related values */
/*
 * Note: this function will only really look for the privmap key
 *       if this variable has been initialized to something < 0 (e.g. -1).
 *       The same holds for the location parameter, it is only read from the
 *       config file if it has been initialized to loc_default.
 */
void gmm_parse_conf(gmm_loc_t *location,
                    int *privmap_key,
                    int *globalmap_key,
                    unsigned int *globalmap_size,
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


/* The block type used to store gmm_pt_conf_t struct */
#define GMM_PT_CONF_BLOCK_TYPE block_type_gmm

/* The block type used to store gmm_t struct */
#define GMM_T_BLOCK_TYPE block_type_gmm_setup

/* The block name used to store gmm_t struct */
#define GMM_T_BLOCK_NAME "gmm_t"



/* magic value stored in shm_t.magic */
static const i32 GMM_MAGIC = 0x7278d2aa;

/* magic value stored in plc_pt_priv_t.magic */
static const i16 PT_MAGIC = 0x07f1; /* note: mustn't be 0 */

/* How to connect to the shared memory by default                    */
/* loc_local:   access the local and global shared memory directly   */
/*               (through shared memory).                            */
/* loc_isolate: access the local shared memory directly, and the     */
/*               global shared memory through proxy server.          */
/* loc_shared:  access the global plc memory directly (through       */
/*               shared memory), and do not use a local copy, but    */
/*               access the global memory instead!                   */
#define GMM_DEF_PLC_LOCATION loc_local

/* Default semaphore key */
/*  Use 0 for random key */
#define GMM_DEF_SEM_KEY 0

/* Default globalmap shmem key */
/*  Use 0 for random key */
#define GMM_DEF_GLOBALMAP_KEY 0

/* Default private map (privmap) shmem key */
/*  gmm_local   -> 0 implies use of malloc            */
/*  gmm_isolate -> 0 implies use random key for shmem */
#define GMM_DEF_PRIVMAP_KEY 0

/* Default size in bytes for globalmap */
/* Must be > 0 */
#define GMM_DEF_GLOBALMAP_PG (8*1024)  /* default of 8 kBytes */


/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/

/* location */
#define CONF_LOC_NAME "location"       /* name to search for in conf file    */
#define CONF_LOC_SECTION NULL          /* section to search for in conf file */
#define CONF_LOC_LOCAL   "local"
#define CONF_LOC_ISOLATE "isolate"
#define CONF_LOC_SHARED  "shared"

/* globalmap shmem key */
#define CONF_GK_NAME "globalmap_key"
#define CONF_GK_SECTION "PLC"
#define CONF_GK_MIN  0
#define CONF_GK_MAX  INT_MAX

/* privmap shmem key */
#define CONF_PK_NAME "privatemap_key"
#define CONF_PK_SECTION NULL           /* use module_name_ as section */
#define CONF_PK_MIN  0
#define CONF_PK_MAX  INT_MAX

/* num. of bytes for globalmap */
#define CONF_GP_NAME "globalmap_size"
#define CONF_GP_SECTION "PLC"
#define CONF_GP_MIN  1
#define CONF_GP_MAX  UINT_MAX


/**********************************/
/* table names of the config file */
/**********************************/

/* plc points */
#define CONF_POINT_SECTION    "PLC"   /* section to search for in conf file          */
#define CONF_POINT_NAME       "point" /* name to search for in conf file             */
#define CONF_POINT_INIT       "init"  /* keyword used before the initial point value */
#define CONF_POINT_INIT_FTYPE 'f'     /* letter used before the length value to      */
                                      /* specify that the init value should be read  */
                                      /* as a float.                                 */
#define CONF_POINT_INIT_ITYPE 'i'     /* letter used before the length value to      */
                                      /* specify that the init value should be read  */
                                      /* as a signed integer.                        */
#define CONF_POINT_INIT_UTYPE 'u'     /* letter used before the length value to      */
                                      /* specify that the init value should be read  */
                                      /* as an unsigned integer.                     */
#define CONF_POINT_INIT_TRUE  "TRUE"  /* keyword used to set initial point value to  */
                                      /* TRUE <=> all bits set to 1.                 */
#define CONF_POINT_INIT_FALSE "FALSE" /* keyword used to set initial point value to  */
                                      /* FALSE <=> all bits set to 0.                */
#define CONF_POINT_INIT_DEF DEF_PLC_PT_VALUE  /* default value to use for            */
                                              /* initialising plc points             */
#define CONF_POINT_LENGTH_DEF 1       /* default length of plc points                */

/* plc point aliases */
#define CONF_POINT_ALIAS_SECTION "PLC"         /* section to search for in conf file */
#define CONF_POINT_ALIAS_NAME    "point_alias" /* name to search for in conf file    */


#endif /* __GMM_PRIVATE_H */
