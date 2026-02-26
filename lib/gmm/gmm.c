/*
 * (c) 2000 Jiri Baum
 *          Mario de Sousa
 *
 * (c) 2001 Juan Carlos Orozco
 *          (added plc_get_f32() and plc_set_f32() )
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
 * GMM Manager - access library implementation
 *
 * This file implements the routines in gmm.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>
#include <time.h>  /* random seed generator */
#include <ctype.h>

#include "plc.h"

#include <misc/mutex_util.h>
#include "gmm_private.h"
#include "gmm_setup.h"
#include <cmm/cmm.h>
#include <conffile/conffile.h>
#include <log/log.h>
#include "protocol.h"
#include "plcproxy.h"
#include <misc/shmem_util.h>
#include <misc/string_util.h>
#include "../plc_private.h"



static int debug = 0;



/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************       gmm_private.h      *****************/
/*****************                          *****************/
/************************************************************/


/* name of this module, as given to plc_init or from the command line */
const char *gmm_module_name_ = NULL;

/* the current status of the gmm library (how far did init get?) */
status_t gmm_status_ = {0};

/* access functions for the status_ variable */
inline int gmm_get_status(const status_t stat_bit) {
  return (gmm_status_.s & stat_bit.s) != 0;
}

inline void gmm_set_status(const status_t stat_bit) {
  gmm_status_.s |= stat_bit.s;
}

inline void gmm_rst_status(const status_t stat_bit) {
  gmm_status_.s &= (~stat_bit.s);
}

int plc_magic_bit_aliases = 0;

gmm_t *gmm_shm_   = NULL;
u32 *gmm_privmap_ = NULL; /* private map */
u32 *gmm_mapmask_ = NULL; /* read/write mask for the map */

i32 gmm_globalmap_len_  = 0; /* in integers */
u32 gmm_globalmap_size_ = 0; /* in bytes */


#define __assert_gmm_initialized(ret_val)                                   \
        { if (!gmm_get_status(gmm_fully_initialized))                       \
            {if (debug)                                                     \
               printf ("Global Memory Manager not initialized at %s:%d.\n", \
                      __FILE__, __LINE__);                                  \
             return ret_val;                                                \
            };                                                              \
        };

#define __assert_gmm_not_initialized(ret_val)                         \
        { if (gmm_get_status(gmm_fully_initialized))                  \
            return ret_val;                                           \
        };


/*
 * Modularity - for local vs. isolate vs. shared access to the global shared map
 *
 * They are characterised by the "init" function. The other function
 * pointers will be set to the correct values by the relevant init fn.
 *
 */


/*
 * This is the ``pure virtual'' function. Technically, we aren't allowed to
 * typecast it like this (the behaviour is undefined, see comment at
 * plc_get_f32()), but it works and it'll probably work everywhere.
 */
static void plc_variant_error(void)
{
  printf("PLC not initialized! (invalid function pointer)\n");
  exit(1);
}

gmm_mod_t gmm_mod_ = {
  init: (void *) plc_variant_error,
  pt_count:(void *) plc_variant_error,
  pt_by_index:(void *) plc_variant_error,
  pt_by_name:(void *) plc_variant_error,
  pt_by_loc:(void *) plc_variant_error,
  update:(void *) plc_variant_error,
  update_pt:(void *) plc_variant_error,
  update_pts:(void *) plc_variant_error,
  done:(void *) plc_variant_error,
};

/*
 * wrapper for one of the two-variant functions - user doesn't know
 * anything about modules. See other comment on ``two-variant''.
 *
 * This is here so it can be inlined - others are further on.
 */
inline plc_pt_t plc_pt_by_loc(i32 offset, u8 bit, u8 length)
{
  return gmm_mod_.pt_by_loc(offset, bit, length);
}


/* A r/w mapmask area init function */
/* Uses the owner to determine whether we have write acces to a point or not */
int gmm_init_mapmask(const char *module, u32 *mapmask, int mapsize)
{
  int index;
  char *id, *owner;
  i32 ofs;
  u8 bit, length;
  plc_pt_t p;

  if (debug) printf("gmm_init_mapmask(): ...\n");

  if ((module == NULL) || (mapmask == NULL) || (mapsize < 0))
    return -1;

  memset(mapmask, 0, mapsize);

  index = 0;
  while (gmm_config_pt_by_index(index++, &id, &owner,
                                &ofs, &bit, &length) == 0) {
    if ((owner[0] != '\0') && (strcmp(module, owner) == 0)) {
      p = plc_pt_by_loc(ofs, bit, length);
      if (p.valid != 0) {
        plc_log_trcmsg(9, "Setting mask=%x for point %s",
                        (*((plc_pt_priv_t *)(&p))).mask, id);
        mapmask[ofs] |= (*((plc_pt_priv_t *)(&p))).mask;
      }
      else
        /* TODO: we should not really just give up */
        /* but rather log an error and continue */
        /* requires that plc_log_init() be called before gmm_init() */
        /* return -1;*/
        continue;
    }
  }

  return 0;
}


/* get write acces to a point, regardless of it's owner module.             */
/*  This is also used by gmm_setup() to set the initial value of a plc_pt.  */
int gmm_set_pt_w_perm(plc_pt_t p) {
  __assert_gmm_initialized(-1);

  gmm_mapmask_[((plc_pt_priv_t *)(&p))->ofs] |= ((plc_pt_priv_t *)(&p))->mask;

  return 0;
}


/* Function to obtain the size, in bytes, of the global map. */
/* Returns 0 on success, -1 on failure.                      */
int gmm_globalmap_size(u32 *size)
{
  __assert_gmm_initialized(-1);

  if (size != NULL)
    *size = gmm_globalmap_size_;

  return 0;
}



/* Function to obtain the length, in sizeof(u32), of the global map. */
/* Returns 0 on success, -1 on failure.                              */
int gmm_globalmap_len(i32 *len)
{
  __assert_gmm_initialized(-1);

  if (len != NULL)
    *len = gmm_globalmap_len_;

  return 0;
}



/* initialise a memory block in the cmm
 * with the gmm structure
 */
int gmm_init_conf_struct(void)
{
  u32 size = 0;

  /* do the consistency checks */
  if (sizeof(plc_pt_t) != sizeof(plc_pt_priv_t)) {
    fprintf(stderr,
	    "PLC ERROR - private (%d) and public (%d) point handle size mismatch!\n",
            sizeof(plc_pt_priv_t), sizeof(plc_pt_t));
    return -1;
  }

  /* get pointer to the gmm_t struct */
  gmm_shm_ = cmm_block_find(GMM_T_BLOCK_TYPE,
                            GMM_T_BLOCK_NAME,
                            &size);
  if ((gmm_shm_ == NULL) || (size != sizeof(gmm_t)))
    return -1;

  /* initialize global variables */
  gmm_globalmap_size_ = gmm_shm_->globalmap_size;
  gmm_globalmap_len_ = gmm_globalmap_size_ / sizeof(u32);

  return 0;
}


/*
 * Error exit function for all gmm_init_xxxxx() functions
 */
int gmm_init_error(status_t status_bit)
{
  gmm_rst_status(status_bit);
  gmm_rst_status(gmm_fully_initialized);
  gmm_done();
  return -1;
}


/* return number of points stored in the config memory area */
inline int gmm_config_pt_numb(void)
{
  return cmm_block_count(GMM_PT_CONF_BLOCK_TYPE);
}


/* store point config in config memory area */
int gmm_insert_pt_cmm(const char *name,
                      const char *owner,
                      i32 offset,
                      u8 bit,
                      u8 length)
{
  gmm_pt_conf_t *pt_conf;

  if (name == NULL) return -1;

  pt_conf = cmm_block_alloc(GMM_PT_CONF_BLOCK_TYPE,
                            name, sizeof(gmm_pt_conf_t));

  if (pt_conf == NULL)
    return -1;

  strncpy(pt_conf->owner, owner, GMM_OWNER_MAX_LEN);
  pt_conf->offset = offset;
  pt_conf->bit = bit;
  pt_conf->length = length;

  return 0;
}


/* remove point config from config memory area */
int gmm_remove_pt_cmm(const char *name) {
  return cmm_block_free(cmm_block_find(GMM_PT_CONF_BLOCK_TYPE, name, NULL /*size*/));
}


/* get point config from the config memory area */
int gmm_config_pt_by_name(const char*name,
                          i32 *offset,
                          u8  *bit,
                          u8  *length)
{
  u32 size;
  gmm_pt_conf_t *pt_conf;

  if (name == NULL) return -1;

  pt_conf = cmm_block_find(GMM_PT_CONF_BLOCK_TYPE,
                           name, &size);

  if ((pt_conf == NULL) || (size != sizeof(gmm_pt_conf_t)))
    return -1;

  if (offset != NULL) *offset = pt_conf->offset;
  if (bit    != NULL) *bit    = pt_conf->bit;
  if (length != NULL) *length = pt_conf->length;

  return 0;
}



/* get point config from the config memory area */
/* returns 0 if succesfull, -1 otherwise. */
int gmm_config_pt_by_index(i16 pt_index,
                           char **name,
                           char **owner,
                           i32  *offset,
                           u8   *bit,
                           u8   *length)
{
  u32 size;
  gmm_pt_conf_t *pt_conf;

  if (debug) printf("gmm_config_pt_by_index(): ...\n");

  pt_conf = cmm_block_get(GMM_PT_CONF_BLOCK_TYPE,
                          pt_index,
                          &size,
                          name);

  if ((pt_conf == NULL) || (size != sizeof(gmm_pt_conf_t)))
    return -1;

  if (owner  != NULL) *owner  = pt_conf->owner;
  if (offset != NULL) *offset = pt_conf->offset;
  if (bit    != NULL) *bit    = pt_conf->bit;
  if (length != NULL) *length = pt_conf->length;

  if (debug)
    printf("gmm_pt_by_index(): "
           "point no. %d, name=%s, owner=%s, offset=%d, bit=%d, length=%d\n",
           pt_index, *name, *owner, *offset, *bit, *length);

  return 0;
}


/* helper function for gmm_parse_conf() */
static void gmm_parse_conf_i32(const char *section, const char *name,
                               i32 *var, i32 min, i32 max, i32 def,
                               int use_default)
{
  int res = 0;

  if (!use_default)
    def = *var;

  if (section == NULL)
    res = conffile_get_value_i32(name, var, min, max, def);
  else
    res = conffile_get_value_sec_i32(section, name, var, min, max, def);

  if ((res < 0) && (use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s = %s,"
                   " or value out of bounds [%d...%d]. Using default %d",
                   name, conffile_get_value(name), min, max, *var);
  }

  if ((res < 0) && (!use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s = %s,"
                   " or value out of bounds [%d...%d].",
                   name, conffile_get_value(name), min, max);
  }
}


/* helper function for gmm_parse_conf() */
static void gmm_parse_conf_u32(const char *section, const char *name,
                               u32 *var, u32 min, u32 max, u32 def,
                               int use_default)
{
  int res = 0;

  if (!use_default)
    def = *var;

  if (section == NULL)
    res = conffile_get_value_u32(name, var, min, max, def);
  else
    res = conffile_get_value_sec_u32(section, name, var, min, max, def);

  if ((res < 0) && (use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s = %s,"
                   " or value out of bounds [%d...%d]. Using default %d",
                   name, conffile_get_value(name), min, max, *var);
  }

  if ((res < 0) && (!use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s = %s,"
                   " or value out of bounds [%d...%d].",
                   name, conffile_get_value(name), min, max);
  }
}


/* parse the config file for gmm related values */
/*
 * Note: this function will only really look for the privmap keys
 *       if this variable has been initialized to something < 0 (e.g. -1).
 *       The same holds for the location parameter, it is only read from the
 *       config file if it has been initialized to loc_default.
 */
void gmm_parse_conf(gmm_loc_t *location,
                    int *privmap_key,
                    int *globalmap_key,
                    unsigned int *globalmap_size,
                     /*
                      * flag indicating if we should return the default values
                      * if the parameter has been left unspecified in the
                      * config file.
                      * 1 - yes, init to the default value if left unspecified
                      * 0 - no, leave parm. value unchanged if left unspecified
                      */
                    int use_default)
{
  char *location_tmp;

  if (privmap_key != NULL)
  if (*privmap_key < 0)
    /* NOTE: only use the value in config file if this parameter is not
     *       set on the command line arguments...
     */
    gmm_parse_conf_i32(CONF_PK_SECTION, CONF_PK_NAME, privmap_key,
                       CONF_PK_MIN, CONF_PK_MAX, GMM_DEF_PRIVMAP_KEY,
                       use_default);

  if (location != NULL)
  if (*location == loc_default) {
    /* NOTE: same as above note... */
    if (CONF_LOC_SECTION != NULL)
      location_tmp = conffile_get_value_sec(CONF_LOC_SECTION, CONF_LOC_NAME);
    else
      location_tmp = conffile_get_value(CONF_LOC_NAME);

    if (location_tmp == NULL)
      {if (use_default) *location = GMM_DEF_PLC_LOCATION;}
    else if (strcmp(location_tmp, CONF_LOC_LOCAL) == 0)
           *location = loc_local;
    else if (strcmp(location_tmp, CONF_LOC_ISOLATE) == 0)
           *location = loc_isolate;
    else if (strcmp(location_tmp, CONF_LOC_SHARED) == 0)
           *location = loc_shared;
  }

  if (globalmap_key != NULL)
    gmm_parse_conf_i32(CONF_GK_SECTION, CONF_GK_NAME, globalmap_key,
                       CONF_GK_MIN, CONF_GK_MAX, GMM_DEF_GLOBALMAP_KEY,
                       use_default);

  if (globalmap_size != NULL)
    gmm_parse_conf_u32(CONF_GP_SECTION, CONF_GP_NAME, globalmap_size,
                       CONF_GP_MIN, CONF_GP_MAX, GMM_DEF_GLOBALMAP_PG,
                       use_default);
}




/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************          gmm.h           *****************/
/*****************                          *****************/
/************************************************************/


/********************************************************/
/*                                                      */
/* Init and Done functions                              */
/*                                                      */
/********************************************************/

int gmm_init(const char *mod_name,
             gmm_loc_t location,
             int privmap_key)
{
  int globalmap_key = 0;
  unsigned int globalmap_pg  = 0;

  if (debug)
    printf("gmm_init(...): ...\n");

  __assert_gmm_not_initialized(-1);

  gmm_module_name_ = mod_name;

  /*
   * get the magic_bit_aliases setting
   * Order of setting:
   * 	default (0 = OFF)
   * 	module-specific default (set plc_magic_bit_aliases before init)
   * 	PLC-wide setting (magic_bit_aliases in [PLC] section)
   * 	per-module setting (magic_bit_aliases in module's section)
   * 	module forcing (set plc_magic_bit_aliases after init)
   */
  conffile_get_value_sec_i32("PLC","magic_bit_aliases",
      &plc_magic_bit_aliases, 0, 1, plc_magic_bit_aliases);
  conffile_get_value_i32("magic_bit_aliases",
      &plc_magic_bit_aliases, 0, 1, plc_magic_bit_aliases);

  /* read the values from the config file */
  gmm_parse_conf(&location, &privmap_key,
                 &globalmap_key, &globalmap_pg,
                 1 /* return the default values if parm. is left unspecified*/);

  switch (location) {
   case loc_local  : {gmm_mod_.init = gmm_init_local;   break;}
   case loc_isolate: {gmm_mod_.init = gmm_init_isolate; break;}
   case loc_shared : {gmm_mod_.init = gmm_init_shared;  break;}
   default         : {/* invalid location-only reached in case of bug in prog.*/
                      plc_log_errmsg(1,"Invalid location for gmm sub-system.");
                      return -1;
                     }
  } /* switch() */

  /* call the specialised init function */
  /* TODO: we should really pass all the above parsed parameters to the
   * init function. Even though it does not need them, they could be used
   * to do some checking and output some warning messages...
   */
  if (gmm_mod_.init(privmap_key) < 0)
    return -1;

  return 0;
}


int gmm_done(void)
{
  int res;

  gmm_module_name_ = NULL;

  __assert_gmm_initialized(0);

  res = gmm_mod_.done();

  gmm_mod_.init          = (void *)plc_variant_error;
  gmm_mod_.pt_count      = (void *)plc_variant_error;
  gmm_mod_.pt_by_index   = (void *)plc_variant_error;
  gmm_mod_.pt_by_name    = (void *)plc_variant_error;
  gmm_mod_.pt_by_loc     = (void *)plc_variant_error;
  gmm_mod_.update        = (void *)plc_variant_error;
  gmm_mod_.update_pt     = (void *)plc_variant_error;
  gmm_mod_.update_pts    = (void *)plc_variant_error;
  gmm_mod_.done          = (void *)plc_variant_error;

  return res;
}


/********************************************************/
/*                                                      */
/* two-variant declarations                             */
/*                                                      */
/********************************************************/
/*
 * Several functions exist in more than one variant, one for "local", another
 * "isolate" and another for "shared". These now live in separate modules,
 * gmm_local.c, gmm_isolate.c and gmm_shared.c
 *
 * Which module is active depends on the gmm_mod global variable, which has
 * a function pointer to each function that differs.
 *
 * gmm_mod_.init is initialized to the default module.
 * All other fields are initialized to an error function
 * and changed to the correct value by the relevant gmm_mod_.init()
 *
 * Below, function gmm_X just calls gmm_mod_.X()
 */

/*
 * wrappers for the two-variant functions - user doesn't know anything
 * about modules
 */
int plc_pt_count(void)
{
  return gmm_mod_.pt_count();
}

plc_pt_t plc_pt_by_index(int index, char **name, char **owner)
{
  return gmm_mod_.pt_by_index(index, name, owner);
}

plc_pt_t plc_pt_by_name(const char *name)
{
  /* get it from the appropriate module */
  plc_pt_t res=gmm_mod_.pt_by_name(name);

  /* if that's not defined in itself, try the name.bit magic (if enabled) */
  if (!res.valid && plc_magic_bit_aliases) {
    int l = strlen(name);

    /* must end in either .d or .dd */
    if ((l>2)
	&&
	isdigit(name[l-1])
	&&
	( ((name[l-2]=='.') && (l-=2))
		 ||
	  ((l>3) && isdigit(name[l-2]) && (name[l-3]=='.') && (l-=3)))) {
      /* at this point, l is the index of the correct '.' */

      /* try to get the parent point */
      char *n = strdup(name);
      n[l] = '\0';
      res=gmm_mod_.pt_by_name(n);
      free(n);

      if (res.valid && (plc_pt_len(res)>0)) {
	/* parent point exists and is not intentionally 0-width */
	res = plc_subpt(res, atoi(name+l+1), 1);
	/* don't need to check the range of the bit, plc_subpt() does */
      }
    }
  }

  return res;
}

int plc_update(void)
{
  return gmm_mod_.update();
}

int plc_update_pt(plc_pt_t p)
{
  return gmm_mod_.update_pt(p);
}

int plc_update_pts(plc_pt_t p[], int count)
{
  return gmm_mod_.update_pts(p, count);
}


/********************************************************/
/*                                                      */
/* Other public functions that are the same for every   */
/* gmm implementation variant.                          */
/*                                                      */
/********************************************************/

#define pp (*(plc_pt_priv_t*)&(p)) /* over-ride the type system */
#define ppp(i) (*(plc_pt_priv_t *)(p + (i)))

#define check_pt_magic_ret(p,val) {            \
  if (pp.magic != PT_MAGIC)                    \
    return (val);                              \
}

#define check_pt_magic(p) check_pt_magic2(p,__FILE__,__LINE__)
static inline void check_pt_magic2(plc_pt_t p, const char*f, int l) {
  if (pp.magic==PT_MAGIC) return;
  printf("Error - point handle has bad magic at %s:%d\n",f,l);
  exit(1);
}

plc_pt_t plc_pt_null(void)
{
  return plc_pt_by_loc(0, 0, 0);
}

plc_pt_t plc_subpt(plc_pt_t p, u8 bit, u8 length)
/* bit: 0 .. p.length - 1 */
/* length: 1 .. p.length  */
{
  plc_pt_t sub_pt;

  (*(plc_pt_priv_t *)&sub_pt).magic = 0; /* assume error */

  check_pt_magic_ret(p, sub_pt);

  if (pp.length < (bit + length))
    return sub_pt;

  return plc_pt_by_loc(pp.ofs, pp.bit_shift + bit, length);
};


int plc_pt_len(plc_pt_t p)
{
  check_pt_magic_ret(p, -1);

  return pp.length;
}

int plc_pt_rw(plc_pt_t p)
{
  check_pt_magic_ret(p, -1);

  /* return it as a bitmapped number */
  return ((~gmm_mapmask_[pp.ofs] & pp.mask) ? 1 : 0) /* read bits */
       | ((gmm_mapmask_[pp.ofs] & pp.mask) ? 2 : 0); /* write bits */
}

void plc_pt_details(plc_pt_t p,	/* the point handle - compulsory */
		    int *valid,	/* is the point valid? */
		    int *length,	/* length in bits */
		    int *offset,	/* word address in memory area */
		    int *bit,	/* bit number 0-31 of LSB of point */
		    int *rw /* read/write (like plc_pt_rw) */ )
{
  if (valid)
    *valid = (pp.magic == PT_MAGIC) ? 1 : 0;

  /* try to do something sensible for invalid points */
  if (pp.magic != PT_MAGIC)
    p = plc_pt_null();

  if (length)
    *length = pp.length;
  if (offset)
    *offset = pp.ofs;
  if (bit)
    *bit = pp.bit_shift;
  if (rw)
    *rw = plc_pt_rw(p);
}

const char *plc_pt_rw_s(plc_pt_t p)
{
  switch (plc_pt_rw(p)) {
    case 1: return "read-only";
    case 2: return "read-write";
    case 3: return "mixed";
    case 0: return "null";
    case -1: return NULL;
    default: return NULL;
  }				/* switch() */
}

inline u32 plc_get(plc_pt_t p)
{
  check_pt_magic(p);
  return ((gmm_privmap_[pp.ofs] & pp.mask) >> pp.bit_shift);
}


inline int plc_set(plc_pt_t p, u32 val)
{
  check_pt_magic(p);

  gmm_privmap_[pp.ofs] =
    (gmm_privmap_[pp.ofs] & ~(pp.mask & gmm_mapmask_[pp.ofs])) |
    ((val << pp.bit_shift) & (pp.mask & gmm_mapmask_[pp.ofs]));

  return 0;
}


u32 plc_geti(plc_pt_t p)
{
  check_pt_magic(p);

  return (~(gmm_privmap_[pp.ofs] & pp.mask) >> pp.bit_shift);
}


int plc_seti(plc_pt_t p, u32 val)
{
  check_pt_magic(p);

  gmm_privmap_[pp.ofs] =
    (gmm_privmap_[pp.ofs] & ~(pp.mask & gmm_mapmask_[pp.ofs])) |
    ((~(val << pp.bit_shift)) & (pp.mask & gmm_mapmask_[pp.ofs]));

  return 0;
}

f32 plc_get_f32(plc_pt_t p)
{
/*
 * We can't use a simple type cast because the calling convention is
 * inconsistent between float(*)() and int(*)(), at least on i386.
 *
 * According to C99 paragraph 6.3.2.3/8, "A pointer to a function of one
 * type may be converted to a pointer to a function of another type and
 * back again; the result shall compare equal to the original pointer. If a
 * converted pointer is used to call a function whose type is not
 * compatible with the pointed-to type, the behavior is undefined."
 *
 * return ( ( f32(*)(plc_pt_t) ) plc_get)(p);
 *
 * Similarly, gcc 3.3 points out that it's illegal to do:
 *
 * return *((f32 *)(&tmp_u32));
 */

 const union {
   u32 u;
   f32 f;
 } tmp = {u: plc_get(p)};

 return tmp.f;
}

int plc_set_f32(plc_pt_t p, f32 val)
{
  const union {
    u32 u;
    f32 f;
  } tmp = {f: val};
  return plc_set(p, tmp.u);
}

