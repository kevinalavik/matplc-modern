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


/*
 * GMM Manager - setup and shutdown functions
 *
 * This file implements the routines in gmm_setup.h
 *
 * In general, it should only be called by plc_setup() and plc_shutdown()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>

#include <misc/mutex_util.h>
#include "gmm_setup.h"
#include "gmm_private.h"
#include <cmm/cmm.h>
#include <misc/shmem_util.h>
#include <misc/string_util.h>
#include "plc.h"

static const int debug = 0;


/*************************************/
/*                                   */
/*         F U N C T I O N S         */
/*                                   */
/*************************************/


/* forward declaration */
static int gmm_load_confmap(void);


int gmm_setup(const char *module_name)
/* returns 0 if successful               */
/* returns -1 on error                   */
/*
 *       All other values are obtained from the config file.
 */
/*
 * This function should either setup all the memory areas and semaphores
 * correctly or not do anything at all.
 */
{
  gmm_t *gmm_shm;
  int *global_mem;
  int globalmap_key;
  unsigned int globalmap_size;

  if (debug)
    printf("gmm_setup(): module_name = %s\n", module_name);

  /* parse the config file for parameters */
  gmm_parse_conf( NULL, /* we are not interested in location parameter */
                  NULL, /* we are not interested in privmap_key        */
                  &globalmap_key, &globalmap_size,
                  1 /* return the default if parm. is left unspecified */);

/* TODO:                                                   */
/* should do some checking to see if values are consistent */

  /* create configuration shared memory */
  gmm_shm = cmm_block_alloc(GMM_T_BLOCK_TYPE,
                            GMM_T_BLOCK_NAME,
                            sizeof(gmm_t));
  if (gmm_shm == NULL) {
    if (debug) printf("gmm_setup(): could not alloc config memory block.\n");
    goto error_exit_0;
  }

  /* create global shared memory */
  if (globalmap_key == 0)
    global_mem = create_shmem_rand(&globalmap_key, globalmap_size);
  else
    global_mem = create_shmem(globalmap_key, globalmap_size);
  if (!global_mem)
    goto error_exit_1;
  /* create the sempahores... */
  if(plcmutex_create(&(gmm_shm->mutexid)) < 0)
    goto error_exit_2;

  /* initialize the conf mem area */
  gmm_shm->magic = GMM_MAGIC;
  gmm_shm->globalmap_key = globalmap_key;
  gmm_shm->globalmap_ofs = 0;
  gmm_shm->globalmap_size = globalmap_size;

  /* initialize the access to the gmm */
  if (gmm_init(module_name, loc_local, 0 /* use malloc for private maps */) < 0)
    goto error_exit_3;

  /* Load the confmap */
  if (gmm_load_confmap() < 0) {
    if (debug) printf("gmm_setup(): error loading confmap...\n");
    goto error_exit_4;
  }

  /* "the shared map is now open for business" */
  return 0;

  /* undo anything done up until the error */
error_exit_4:
  gmm_done();

error_exit_3:
  plcmutex_destroy(&(gmm_shm->mutexid));

error_exit_2:
  delete_shmem(globalmap_key);

error_exit_1:
  cmm_block_free(gmm_shm);

error_exit_0:
  return -1;
}


int gmm_shutdown(void)
{
  u32 size;
  int globalmap_key;
  int result = 0;
  int conffile_globalmap_key;
  gmm_t *gmm_shm;

  if (debug)
    printf("gmm_shutdown(): ...\n");

  /* parse the config file for parameters */
  conffile_globalmap_key = -1;

  gmm_parse_conf( NULL, /* we are not interested in location parameter */
                  NULL, /* we are not interested in localmap_key       */
                  &conffile_globalmap_key,
                  NULL, /* we are not interested in size */
                   /* don't change our parm. values if they have not */
                   /* been ecplicitly specified in the config file   */
                  /* use_defaults */ 0);
    /* if we were to use a random key (==0), then set it to -1
     * because we still don't know which key was used!
     */
  if (conffile_globalmap_key <= 0) conffile_globalmap_key = -1;

  /* get hold of conf map shared memmory */
  gmm_shm = cmm_block_find(GMM_T_BLOCK_TYPE,
                           GMM_T_BLOCK_NAME,
                           &size);

  if (size != sizeof(gmm_t))
    gmm_shm = NULL;

  /* TODO: do a more intelligent shutdown:
   *      - if we cannot get access to the conf mem area, should ask
   *        the user if we should try to delete the resources with config
   *        specified keys...
   */

  /* assume unknown keys */
  globalmap_key = -1;

  /* determine global map shared memory key */
  if (gmm_shm != NULL) {
    globalmap_key = gmm_shm->globalmap_key;

    /* Decide which of the globalmap_key we will use! */
    if ((globalmap_key != conffile_globalmap_key) &&
       (globalmap_key > 0) && (conffile_globalmap_key > 0))
      fprintf(stderr,
              "Warning: PLC globalmap keys from config file (%d) and "
              "configuration memory area (%d) do not match. Using %d.\n",
              conffile_globalmap_key, globalmap_key, globalmap_key);
  } else {
    /* Try to shutdown using info from the config file            */
    /* This should probably be protected with a force option/flag */

    /* semid = ...*/
    /*
    if (conffile_globalmap_key > 0)
      globalmap_key = conffile_globalmap_key;
    */
  }

  /* remove globalmap */
  if (globalmap_key != -1)
    if (delete_shmem(globalmap_key) < 0)
      result = -1;

  /* remove semaphores */
  if (gmm_shm != NULL)
    if (plcmutex_destroy(&(gmm_shm->mutexid)) < 0)
      result = -1;

  /* delete the cmm block used by the gmm */
  if (cmm_block_free(gmm_shm) < 0)
    result = -1;

/*
  gmm_done();
*/

  return result;
}


static int gmm_get_pa_by_index(int index,
                               char **alias_id,
                               char **full_name,
                               char **org_id,
                               u8 *bit, u8 *length)
/*
 point_alias syntax:
    point_alias alias_ID "full name" org_ID [bit [length]]

    Bit and Length are used to configure sub_aliases.
    Bit is optional.
      Defaults to 0
    Length is optional.
      Defaults to 1 if bit given,
      or to length of org_ID otherwise (i.e. returns 0 to calling function)
*/
{
  int  rowlen;
  u32  tmpu32;

  if (debug)
    printf("gmm_get_pa_by_index(%d,...): \n", index);

  if ((alias_id == NULL) || (full_name == NULL) || (org_id == NULL) ||
      (bit == NULL) || (length == NULL))
    return -1;

  *alias_id = *full_name = *org_id = NULL;

  /*
   * check that there's that many points and that the point in question has
   * the right number of fields
   */

  rowlen = conffile_get_table_rowlen_sec(CONF_POINT_ALIAS_SECTION,
                                         CONF_POINT_ALIAS_NAME,
                                         index);

  if ((rowlen < 3) || (rowlen > 5)) {
    plc_log_errmsg(1, "Wrong number of parameters configuring %s %s",
                   CONF_POINT_ALIAS_NAME,
                   conffile_get_table_sec(CONF_POINT_ALIAS_SECTION,
                                          CONF_POINT_ALIAS_NAME,
                                          index, 0)
                   );
    return -1;
  }

  if (debug)
    printf("%s record exists ok\n", CONF_POINT_ALIAS_NAME);

  /* first three fields are alias_ID, full name and org_ID */
  *alias_id  = conffile_get_table_sec(CONF_POINT_ALIAS_SECTION,
                                      CONF_POINT_ALIAS_NAME, index, 0);
  *full_name = conffile_get_table_sec(CONF_POINT_ALIAS_SECTION,
                                      CONF_POINT_ALIAS_NAME, index, 1);
  *org_id    = conffile_get_table_sec(CONF_POINT_ALIAS_SECTION,
                                      CONF_POINT_ALIAS_NAME, index, 2);

  /* set the default values */
  *bit = 0;
  if (rowlen == 3)
    *length = 0;   /* default length if bit position not given */
  else
    *length = 1;    /* default length if bit position given */

  /* parse the bit field */
  if (rowlen > 3) {
    if (conffile_get_table_sec_u32(CONF_POINT_ALIAS_SECTION,
                                   CONF_POINT_ALIAS_NAME,
                                   index, 3,
                                   &tmpu32, 0, 8*sizeof(u32) - 1, *bit)
        < 0) {
      plc_log_errmsg(1, "Invalid format for bit of %s %s",
                     CONF_POINT_ALIAS_NAME, *alias_id);
      goto err_exit;
    }
    *bit = tmpu32;
  }

  /* parse the length in the last field */
  if (rowlen > 4) {
    if (conffile_get_table_sec_u32(CONF_POINT_ALIAS_SECTION,
                                   CONF_POINT_ALIAS_NAME,
                                   index, 4,
                                   &tmpu32, 1, 8*sizeof(u32), *length)
        < 0) {
      plc_log_wrnmsg(1, "Invalid format for length of %s %s,"
                        " defaulting to %d bits.",
                        CONF_POINT_ALIAS_NAME,
                        *alias_id, *length);
      tmpu32 = *length;
    }
    *length = tmpu32;
  }

  if (debug)
    printf("gmm_get_pa_by_index: success!\n");

  /* got through everything - success! */
  return 0;

 err_exit:
  free(*alias_id);     /* <=> no-op if (alias_id == NULL) */
  free(*full_name);    /* ...                             */
  free(*org_id);       /* ...                             */

  *alias_id = *full_name = *org_id = NULL;
  return -1;
}



/* helper function to parse the
 * [x[.y]] parameter coming after the 'at' keyword
 */
static int gmm_parse_at_parm(char *at_parm,
                             i32 *x, u8 *y, u8 *length,
                             char *pt_name)
{
  i32  tmp_i32;
  int  tmp_int;
  char *tmp_str = at_parm + strcspn(at_parm, ".");

  if (tmp_str[0] != '.')
    *length = 32;   /* default length if no bit position given */
  else
    *length = 1;    /* default length if bit position given */

  tmp_str[0] = '\0'; tmp_str += 1;

  if (string_str_to_i32(at_parm, &tmp_i32, 0, i32_MAX) < 0) {
    plc_log_errmsg(1, "Invalid format for offset of %s %s",
                   CONF_POINT_NAME, *pt_name);
    goto err_exit;
  }
  *x = tmp_i32;

  if (*length == 32) {
    *y = 0;
  } else {
    if (string_str_to_int(tmp_str, &tmp_int, 0, 8*sizeof(u32) - 1) < 0) {
      plc_log_errmsg(1, "Invalid format for bit of %s %s",
                     CONF_POINT_NAME, *pt_name);
      goto err_exit;
    }
    *y = tmp_int;
  }

  return 0;

err_exit:
  return -1;
}



/* parse the point definition line in a configuration file */
/*
 * returns 0 on success, -1 on error
 *
 * NOTE: valid_xy_flag is set depending on whether the point
 *       location is explicilty specified or not.
 *       set to 0: when no location (at x[.y]) is specified
 *                 in this case, the *x and *y values will
 *                 be undefined.
 *       set to 1: when a valid location (at x.[y]) was specified
 *                 in this case the *x and *y will be set to the
 *                 user specified values
 */
static int gmm_get_pt_by_index(int index,
                               char **id,
                               char **full_name,
                               char **owner,
                               int *valid_xy_flag,
                               i32 *x, u8 *y, u8 *length,
                               u32 *init_value)
/*
 point syntax:
    point ID "full name" owner [at x[.y]] [length] [init zzz]

    at x.y is optional. Default is to find an empty slot for the point.
    y      is optional. Defaults to 0.

    Length is optional. Defaults to 1 if at x.y is not specified.
    Length is optional. Defaults to 1 if it has both x.y
    Length is optional. Defaults to 32 if it has only the x

    zzz is the initial value the point should have... Defaults to 0.
*/
{
  typedef enum {
      vtype_any,
      vtype_u,
      vtype_i,
      vtype_f}
    init_value_t;

  char *id_local, *full_name_local, *owner_local;
  i32  x_local;
  u8   y_local, length_local;
  u32  init_value_local;
  int  valid_xy_flag_local;

  init_value_t init_value_type = vtype_any;
  int  at_already_parsed;
  int  length_already_parsed;
  int  init_value_already_parsed;
  int  rowlen, field;
  union {u32 u; f32 f;} tmp;
#define tmp_u32 tmp.u
  char *tmp_str = NULL, *tmp_init_str = NULL, *tmp_len_str = NULL;

  if (debug)
    printf("gmm_get_pt_by_index(%d,...): \n", index);

  if (id            == NULL)            id = &id_local;
  if (full_name     == NULL)     full_name = &full_name_local;
  if (owner         == NULL)         owner = &owner_local;
  if (valid_xy_flag == NULL) valid_xy_flag = &valid_xy_flag_local;
  if (x             == NULL)             x = &x_local;
  if (y             == NULL)             y = &y_local;
  if (length        == NULL)        length = &length_local;
  if (init_value    == NULL)    init_value = &init_value_local;

  *id = *full_name = *owner = NULL;

  /*
   * check that there's that many points and that the point in question has
   * the right number of fields
   */

  rowlen = conffile_get_table_rowlen_sec(CONF_POINT_SECTION,
                                         CONF_POINT_NAME,
                                         index);

  if ((rowlen < 3) || (rowlen > 8)) {
    plc_log_errmsg(1, "Wrong number of parameters configuring %s %s",
                   CONF_POINT_NAME,
                   conffile_get_table_sec(CONF_POINT_SECTION,
                                          CONF_POINT_NAME, index, 0)
                   );
    return -1;
  }

  if (debug)
    printf("%s record exists ok\n", CONF_POINT_NAME);

  /* first three fields are ID, full name and owner */
  *id        = conffile_get_table_sec(CONF_POINT_SECTION,
                                      CONF_POINT_NAME, index, 0);
  *full_name = conffile_get_table_sec(CONF_POINT_SECTION,
                                      CONF_POINT_NAME, index, 1);
  *owner     = conffile_get_table_sec(CONF_POINT_SECTION,
                                      CONF_POINT_NAME, index, 2);

  /* This should never occur. Just checking consistency... */
  if ((*id == NULL) || (*full_name == NULL) || (*owner == NULL)) {
    plc_log_errmsg(1, "Internal consistency error at %s:%d "
                      "while parsing the config file. ",
                      __FILE__, __LINE__);
    goto err_exit;
  }


  /****************************************************************************/
  /* From the fourth field onwards, we have optional parameters, so we use a  */
  /*  field variable and loop though the user parameters...                   */
  /****************************************************************************/
    /* the field currently being parsed...*/
  field = 3;  /* first field is indexed as 0! */
    /* flags indicating we have not yet encountered a field specifying the       */
    /* length/init_value parameter.                                              */
    /*  These flags are required because we do not have a keyword before the     */
    /*  length field, and to make sure multiple copies of the same parameter are */
    /*  not parsed without giving out a warning...                               */
  at_already_parsed = 0;
  length_already_parsed = 0;
  init_value_already_parsed = 0;
    /* The length specification can include a hint on how to interpret the   */
    /* initial value to store in the plc point (float, unsigned or integer)  */
    /* When the length is parsed, it sets this flag, which will then be used */
    /* by the code parsing the init_value as a hint on how to interpret that */
    /* value                                                                 */
  init_value_type = vtype_any;
    /* Start off by initialising parameters to default values... */
  *valid_xy_flag = 0;
  *init_value = CONF_POINT_INIT_DEF;
  *length = CONF_POINT_LENGTH_DEF;
    /* Note: The default value of the length parameter may still be changed
             in the code that parses the 'at' parameters. This is because the default
             value of the length parameter depends on how thw 'at' parameters are
             specified
     */
  *y = 0;
    /* NOTE: Please search for the comment with the reference COMMENT_REF_1,
     *       which contains the reason why the variable must be initialised here!
     */
   /* OK. Now we get to do the loop... */
  while ((tmp_str = conffile_get_table_sec(CONF_POINT_SECTION,
                                CONF_POINT_NAME, index, field++))
         != NULL) {

    /* See if we have the "at" keyword... */
    if (strcmp(tmp_str, "at") == 0) {
      /*********************************/
      /* parse the  at x.[y] parameter */
      /*********************************/

      /* read in [word[.bit]] in the following field */
      if ((tmp_init_str = conffile_get_table_sec(CONF_POINT_SECTION,
                                                 CONF_POINT_NAME, index, field++))
           == NULL) {
          /* ERROR! No parameter given after the at keyword! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "no parameter given after 'at' keyword. "
                          "Ignoring this field.",
                          CONF_POINT_NAME, *id,
                          tmp_str);
        field--;
        goto loop_end;
      }

      /*
       *  But if the 'at' 'init' or length has already been specified, then this
       *  is an error...
       */
      if (at_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field and it's parameter %s.",
                          tmp_str, CONF_POINT_NAME, *id, tmp_init_str);
        free(tmp_init_str); tmp_init_str = NULL;
        goto loop_end;
      }

      /* NOTE: Please search for the comment with the reference COMMENT_REF_1,
       *       which contains the reason why the 'at' option must be specified
       *       before the 'length' option!
       */
      if ((init_value_already_parsed != 0) || (length_already_parsed != 0)){
        plc_log_wrnmsg(1, "In the %s %s spefication, the location (at %s) is being "
                          "specified after the length or the initial value. "
                          "Correct order is the reverse. "
                          "Ignoring this field.",
                          CONF_POINT_NAME, *id, tmp_init_str);
        free(tmp_init_str); tmp_init_str = NULL;
        goto loop_end;
      }

      at_already_parsed = 1;

      /* now really parse the x.[y] value, and set the default value of length*/
      if (gmm_parse_at_parm(tmp_init_str, x, y, length, *id) < 0) {
          /* NO! -> ERROR! We cannot parse the location value! */
        *valid_xy_flag = 0;
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "error parsing the parameter %s "
                          "given after '%s' keyword. "
                          "Ignoring this field.",
                          CONF_POINT_NAME, *id,
                          tmp_init_str, tmp_str);
      }

      *valid_xy_flag = 1;
      free(tmp_init_str); tmp_init_str = NULL;
      goto loop_end;
    } /* parse at x.[y] parameter */


    /* See if we have the "init" keyword... */
    if (strcmp(tmp_str, CONF_POINT_INIT) == 0) {
      /******************************/
      /* parse the initial value... */
      /******************************/

      if ((tmp_init_str = conffile_get_table_sec(CONF_POINT_SECTION,
                                                 CONF_POINT_NAME, index, field++))
          == NULL) {
          /* ERROR! No parameter given after the init keyword! */
        plc_log_wrnmsg(1, "In configuration line of %s %s, "
                          "no parameter given after '%s' keyword. "
                          "Ignoring this field.",
                          CONF_POINT_NAME, *id,
                          tmp_str);
        field--;
        goto loop_end;
      }

      /*
       *  But if the init value has already been specified, then this
       *  is an error...
       */
      if (init_value_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field and it's parameter %s.",
                          tmp_str, CONF_POINT_NAME, *id, tmp_init_str);
        free(tmp_init_str);
        goto loop_end;
      }

      init_value_already_parsed = 1;

        /* Start off by initialising to current value... */
      tmp_u32 = *init_value;

        /* NOTE: string_str_to_xxx() are guaranteed not to change the value
         *       stored in tmp_u32 if they cannot parse the string correctly!
         */
      { int parse_success = 0;
          /* for some strange reason, the << doesn't work for (*length == 32) */
        u32 bit_mask = (*length != 8*sizeof(u32)) ? ~((~0) << *length) : ~0;

          /* Is it 'FALSE' ? */
        if ((init_value_type == vtype_any) &&
            (strcmp(tmp_init_str, CONF_POINT_INIT_FALSE) == 0)) {

          tmp_u32 = 0;
          parse_success = 1;
        }

        if (parse_success == 0)
          /* No. Is it 'TRUE' ? */
          if ((init_value_type == vtype_any) &&
              (strcmp(tmp_init_str, CONF_POINT_INIT_TRUE) == 0)) {

            tmp_u32 = bit_mask;
            parse_success = 1;
          }

          /* NOTE: We only allow 32 bit signed/unsigned ints and floats
                   because we can't support other size variables portably.
                   Who is to say what binary representation is being used?

                   We might be able to support signed and unsigned integers
                   of 8 and 16 bits, but how do we place them in a u32 in order
                   to pass to plc_pt_set()? We must make sure that the 8/16 bits
                   being used must be in the low order bits. Can we portably do
                   this using a union? Casts are out of the question. How can we
                   cast an 8 bit variable onto a 32 bit variable, and make sure
                   that they get mapped onto the low order bits?
          */

        if (parse_success == 0)
          /* No. Is it an unsigned int? */
            /* note: interpret as signed if it has a leading '+' */
          if ((((init_value_type == vtype_any) && (tmp_str[0] != '+')) ||
               (init_value_type == vtype_u)) &&
              /* only allow 32 bit unsigned ints! */
              (*length == 32))
            if (string_str_to_u32(tmp_init_str, (u32 *)&tmp_u32, u32_MIN, u32_MAX) >= 0)
              parse_success = 1;

        if (parse_success == 0)
          /* No. Is it a signed int? */
          if (((init_value_type == vtype_any) || (init_value_type == vtype_i)) &&
              /* only allow 32 bit signed ints! */
              (*length == 32))
            if (string_str_to_i32(tmp_init_str, (i32 *)&tmp_u32, i32_MIN, i32_MAX) >= 0)
              parse_success = 1;

        if (parse_success == 0)
          /* No. Is it a float? */
          if (((init_value_type == vtype_any) || (init_value_type == vtype_f)) &&
              /* only allow 32 bit floats! */
              (*length == 32))
	    /* note that tmp.f is overlaid with tmp_u32 */
            if (string_str_to_f32(tmp_init_str, & tmp.f,-f32_MAX, f32_MAX) >= 0)
              parse_success = 1;

        if (parse_success == 0)
          /* NO! -> ERROR! We cannot parse the initial value! */
          plc_log_wrnmsg(1, "In configuration line of %s %s, "
                            "error parsing the parameter %s "
                            "given after '%s' keyword. "
                            "Using default value %d (0x%X) instead.",
                            CONF_POINT_NAME, *id,
                            tmp_init_str, tmp_str,
                            tmp_u32, tmp_u32);
      }

      *init_value = tmp_u32;

      free(tmp_init_str);
      goto loop_end;
    }  /* parse init value */

    /********************/
    /* Parse the length */
    /********************/
    {
      /* if none of the above, then it must be the length...
       *  But if the length or the init value has already been specified, then this
       *  is an error...
       */
      if (length_already_parsed != 0) {
        plc_log_wrnmsg(1, "Duplicate field %s in %s %s spefication. "
                          "Ignoring this field.",
                          tmp_str, CONF_POINT_NAME, *id);
        goto loop_end;
      }

      if (init_value_already_parsed != 0) {
        plc_log_wrnmsg(1, "In the %s %s spefication, the length (%s) is being "
                          "specified after the initial value. "
                          "Correct order is the reverse. "
                          "Ignoring this field.",
                          CONF_POINT_NAME, *id, tmp_str);
        goto loop_end;
      }

      /*  If none of the above, then it _must_ be a length specification... */

      length_already_parsed = 1;
      tmp_len_str = tmp_str;

      if (tmp_len_str[0] == CONF_POINT_INIT_FTYPE) {
        tmp_len_str++;
        init_value_type = vtype_f;
      }
      if (tmp_len_str[0] == CONF_POINT_INIT_UTYPE) {
        tmp_len_str++;
        init_value_type = vtype_u;
      }
      if (tmp_len_str[0] == CONF_POINT_INIT_ITYPE) {
        tmp_len_str++;
        init_value_type = vtype_i;
      }

        /* default length has already been set when parsing 'at' parameters */
        /* Do not change it if we cannot parse this length parameter.       */
      tmp_u32 = *length;
      if (string_str_to_u32(tmp_len_str, &tmp_u32, 1, 8*sizeof(u32)) < 0) {
        plc_log_wrnmsg(1, "Invalid format for length of %s %s,"
                          " defaulting to %d bits.",
                          CONF_POINT_NAME,
                          *id, *length);
      };

      *length = tmp_u32;

      /* We can only do the following test if the *y variable has already
       * been initialised... which is why we initialise it to 0 before we
       * start up the while() loop.
       * Don't worry, it can't be set to a different value with the 'at' option
       * after the 'length' has already been set, so it is correct to test
       * the length here!
       *
       * Do not remove the following line. It is a method of other
       * comments to reference the comment easily...
       * COMMENT_REF_1
       */
      if ((*length + *y) > (8*sizeof(u32))) {
        *length = 8*sizeof(u32) - *y;
        plc_log_wrnmsg(1, "%s %s overflows into next offset position."
                          " Using new length of %d.",
                          CONF_POINT_NAME,
                          *id, *length);
      }

      goto loop_end;
    }  /* parse length */

    loop_end:
    free(tmp_str); tmp_str = NULL;

  } /* while() -> parsing 6th field, onwards... */

  if (debug) {
    if (*id)
      printf("gmm_get_pt_by_index: success on %s!\n", *id);
    else
      printf("gmm_get_pt_by_index: success!\n");
  }

  /* got through everything - success! */
  return 0;

 err_exit:
  free(*id);          /* <=> no-op if (id == NULL) */
  free(*full_name);   /* ...                       */
  free(*owner);       /* ...                       */
  free(tmp_str);      /* ...                       */

  *id = *full_name = *owner = NULL;
  return -1;
#undef tmp_u32
}



/* Find a location in which to store a point with a length of 'length' */
/*
 * !!! NOTE !!!
 *      This function requires out-of-band information to be passed to it,
 *      i.e. it requires that info is passed to it through variables not
 *      defined in the function prototype.
 *      The out-of-band data used:
 *       - the local copy of the global map will represent the memory
 *         locations that are occupied by other points by having those
 *         bits set to 1. Al other locations are set to 0.
 *         NOTE: this function _does_not_ change the state of the
 *         local copy of the global map in any way. It is up to the
 *         calling function to decide whether or not to use the location
 *         found by this function, and therefore the bits should only
 *         be set to 1 (or not) by the calling function.
 */
static int gmm_find_location_for_pt(i32 *ofs, u8 *bit, u8 length)
{
  i32 ofs_scan;
  i32 ofs_max;
  u8  bit_scan;
  u8  bit_max;
  plc_pt_t p;


  if (gmm_globalmap_len(&ofs_max) < 0)
    return -1;

  if (length > (sizeof(u32)*8))
    return -1;
  bit_max = (8*sizeof(u32)) - length + 1;

  for (ofs_scan = 0; ofs_scan < ofs_max; ofs_scan++) {
    for (bit_scan = 0; bit_scan < bit_max; bit_scan++) {

      if ((p = plc_pt_by_loc(ofs_scan, bit_scan, length)).valid != 0)
        if (plc_get(p) == 0) {
          if (ofs != NULL)
            *ofs = ofs_scan;
          if (bit != NULL)
            *bit = bit_scan;
          return 0;
        }

    } /* for(bit_scan ... ;;) */
  }   /* for(ofs_scan ... ;;) */

  /* reached end without finding a suitable location... */
  return -1;
}




  /* This function will load the confmap with the plc point definitions */
  /*   but not the pt alias definitions.                                */
  /* It assumes that gmm_init() has already been called...              */
  /*
   * NOTE: fixed_location_flag
   *       - If set to 0, the function will only load the plc points
   *         that do NOT have a user specified location, i.e. points that
   *         do not have the 'at x.y' parameter set.
   *       - If set to 1, then the function will only load the plc points
   *         that HAVE a user specified location, i.e. points that
   *         do have the 'at x.y' parameter set.
   */
static int gmm_load_points_in_confmap(int fixed_location_flag)
{
  char *id=NULL, *full_name=NULL, *owner=NULL;
  i32 ofs = 0; // init to 0 so compiler does not complain!
  u8  bit;
  u8  length;
  u32 init_value;
  int index, max_points;
  int valid_ofs_flag;
  plc_pt_t p;
  const u32 mask_all_1 = ~0;

  if (debug) printf("gmm_load_points_in_confmap(): ...\n");

  if ((fixed_location_flag != 0) && (fixed_location_flag != 1))
    return -1;

  /* set to 1 all the already defined points in the plc */
  /* Note: as we don't do a global plc_update(), this state does not affect
   *       the global plc state.
   */
  index = 0;
  while ((p = plc_pt_by_index(index++, /*&name*/ NULL, /*&owner*/ NULL)).valid != 0)
    plc_set(p, mask_all_1);

  /* Now for the main loop, that will insert the point definitions in the cmm */
  max_points = conffile_get_table_rows_sec(CONF_POINT_SECTION, CONF_POINT_NAME);
  for (index = 0; index < max_points; index++) {
    /* Read a new point from the config file... */
    if (gmm_get_pt_by_index(index,
                            &id, &full_name, &owner,
                            &valid_ofs_flag,
                            &ofs, &bit, &length,
                            &init_value)
        < 0) {
      plc_log_errmsg(1, "Error parsing %s %s. Point skipped.",
                     CONF_POINT_NAME,
                     conffile_get_table_sec(CONF_POINT_SECTION,
                                            CONF_POINT_NAME, index, 0)
                     );
      continue;
    }

     /* if we should only be loading points with no user specified location   */
     /* but gmm_pt_by_index() returned a point with a user specified location */
     /* then we just ignore this point.                                       */
    if ((fixed_location_flag == 0) && (valid_ofs_flag == 1))
      continue;
     /* The exact opposite of the above (i.e. vice-versa) */
    if ((fixed_location_flag == 1) && (valid_ofs_flag == 0))
      continue;

    if (debug)
      plc_log_trcmsg(1, "gmm_load_points_in_confmap(): "
                        "result from gmm_get_pt_by_index(%d)-> "
                        "name=%s, owner=%s, valid_ofs_flag=%d, ofs=%d, bit=%d, length=%d, "
                        "init_value=0x%X ",
                        index, id, owner, valid_ofs_flag, ofs, bit, length, init_value);

     /* if we got a point with no user specified location, then we must */
     /* find a location for it ourselves.                               */
    if (valid_ofs_flag == 0)
      if (gmm_find_location_for_pt(&ofs, &bit, length) < 0) {
        plc_log_errmsg(1, "Could not find enough memory to store %s %s. "
                          "Point skipped.",
                       CONF_POINT_NAME,
                       conffile_get_table_sec(CONF_POINT_SECTION,
                                              CONF_POINT_NAME, index, 0)
                       );
        continue;
      };

      /* Check whether the plc point name is not already being used by another
       *   plc point...
       */
    p = plc_pt_by_name(id);
    if (p.valid != 0) {
      plc_log_errmsg(1, "Duplicate definition of %s %s. "
                        "This definition skipped.",
                     CONF_POINT_NAME,
                     id);
      goto point_error;
    }

      /* Let's see if we can build a point handle from the specified parameters,
       *  and at the same time chsck if they are consistent...
       */
    p = plc_pt_by_loc(ofs, bit, length);
    if (p.valid == 0) {
      plc_log_errmsg(1, "%s %s has invalid (location?) parameters."
                        " Point skipped.",
                     CONF_POINT_NAME,
                     id);
      goto point_error;
    }

      /* verify that the memory location used by this plc point does not
       *   overlap any previosly defined point's memory location.
       */
    if (plc_get(p) != 0) {
      plc_log_errmsg(1, "%s %s overlaps a previously defined %1$s."
                        " This %1$s skipped.",
                     CONF_POINT_NAME, id);
      goto point_error;
    }

      /* insert the plc point definition in the cmm */
    if (gmm_insert_pt_cmm(id, owner, ofs, bit, length) < 0) {
      plc_log_errmsg(1, "Error registering %s %s."
                        " This %s skipped.",
                     CONF_POINT_NAME,
                     id,
                     CONF_POINT_NAME);
      goto point_error;
    }

      /* initialize the point to it's initial value...
       * NOTE: we must obtain write permission on the point, otherwise the value
       *       will just be thrown away when we do plc_update()
       */
      /* We don't check whether each of the individual functions        */
      /* returns succesfully. We make a consistency check later on...   */
    gmm_set_pt_w_perm(p);
    plc_set(p, init_value);
    plc_update_pt(p);

      /* Consistency check. */
    if (plc_get(p) != init_value) {
      plc_log_errmsg(1, "Error writing initial value to %s %s."
                        " This %s skipped.",
                     CONF_POINT_NAME,
                     id,
                     CONF_POINT_NAME);
      goto point_error_1;
    }

      /* now set the point in our private copy of the gmm to all 1,
       * indicating that this location has been taken up by a plc point.
       */
    if (plc_set(p, mask_all_1) < 0) {
      plc_log_errmsg(1, "Error writing to %s %s."
                        " This %s skipped.",
                     CONF_POINT_NAME,
                     id,
                     CONF_POINT_NAME);
      goto point_error_1;
    }

     /* point inserted correctly... */
    goto point_clean_up;

    point_error_1:
     gmm_remove_pt_cmm(id);

    point_error:

    point_clean_up:
     free(id);         /* <=> no-op if (id == NULL) */
     free(full_name);  /* ...                       */
     free(owner);      /* ...                       */
  } /* for(;;) */

  return 0;
}


#define priv_pt(p)  (*((plc_pt_priv_t *)&p))

  /* This function will load the confmap with the plc point alias       */
  /*   definitions, but not the pt definitions.                         */
  /* It assumes that gmm_init() has already been called...              */
static int gmm_load_pointsalias_in_confmap(void)
{
  char *alias_id=NULL, *full_name=NULL, *org_id=NULL;
  u8  bit;
  u8  length;
  int index, max_points;
  plc_pt_t p;

  if (debug) printf("gmm_load_pointsalias_in_confmap(): ...\n");

  max_points = conffile_get_table_rows_sec(CONF_POINT_ALIAS_SECTION,
                                           CONF_POINT_ALIAS_NAME);
  for (index = 0; index < max_points; index++) {
    if (gmm_get_pa_by_index(index,
                            &alias_id, &full_name, &org_id, &bit, &length)
        < 0) {
      plc_log_errmsg(1, "Error parsing %s %s. Point skipped.",
                     CONF_POINT_ALIAS_NAME,
                     conffile_get_table_sec(CONF_POINT_ALIAS_SECTION,
                                            CONF_POINT_ALIAS_NAME, index, 0)
                     );
      continue;
    }

    p = plc_pt_by_name(alias_id);
    if (p.valid != 0) {
      plc_log_errmsg(1, "Duplicate definition of %s %s. "
                        "This definition skipped.",
                     CONF_POINT_ALIAS_NAME,
                     alias_id);
      goto pointalias_error;
    }

    p = plc_pt_by_name(org_id);
    if (p.valid == 0) {
      plc_log_errmsg(1, "Original %s %s for %s %s does not exist. "
                        "This definition skipped.",
                     CONF_POINT_NAME, org_id,
                     CONF_POINT_ALIAS_NAME, alias_id);
      goto pointalias_error;
    }

    if (length == 0)
      length = priv_pt(p).length;

    p = plc_subpt(p, bit, length);
    if (p.valid == 0) {
      plc_log_errmsg(1, "%s %s has invalid parameters."
                        " This %s skipped.",
                     CONF_POINT_ALIAS_NAME,
                     alias_id,
                     CONF_POINT_ALIAS_NAME);
      goto pointalias_error;
    }

    if (gmm_insert_pt_cmm(alias_id, "",
                          priv_pt(p).ofs, priv_pt(p).bit_shift, length)
        < 0) {
      plc_log_errmsg(1, "Error creating %s %s."
                        " This %s skipped.",
                     CONF_POINT_ALIAS_NAME,
                     alias_id,
                     CONF_POINT_ALIAS_NAME);
    }

  pointalias_error:
    free(alias_id);     /* <=> no-op if (alias_id == NULL) */
    free(full_name);    /* ...                             */
    free(org_id);       /* ...                             */
  } /* for(;;) */

  return 0;
}


static int gmm_load_confmap(void)
{
    /* first load all the points with a user specified location */
  if (gmm_load_points_in_confmap(1) < 0)
    return -1;

    /* now load all the points without a user specified location */
  if (gmm_load_points_in_confmap(0) < 0)
    return -1;

    /* now load all the point aliases */
  if (gmm_load_pointsalias_in_confmap() < 0)
    return -1;

  return 0;
}
