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

/*
 * typeconv.c
 *
 * This file implements the type conversion block. 
 * This block converts the format used to store data in the plc points.
 * All other blocks assume data to be stored in as an f32, so
 * if the input data is stored in a diferent format, it first has to
 * be converted into an f32.
 * The oposite hapens for output points.
 *
 * This block could also conceivably be used to limit and scale the converted 
 * value, etc..., but we'll leave that till later if it becomes necessary.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>

#include <plc.h>
#include <misc/string_util.h>
#include "typeconv.h"

/*
 * This is a type conversion block implementation.
 *
 * Limitations:
 *  Maximum number of points to convert is defined in typeconv_t.h
 */


static const int debug = 0;


int typeconv_step(unsigned int step_count, dsp_time_t *dsp_time, fb_parm_t *parm)
{
 int index;
 /* I am not sure the following would be correct 
  * in both big and little endian machines, so I won't use it for now...
  
 union {i16 i16, u16 u16, u32 u32; i32 i32; f32 f32;} tmp32;
  */ 
 union {u32 u; i32 i; f32 f;} tmp32;
 union {u16 u; i16 i;       } tmp16;
 long double tmp_ld = 0;

 if (debug)
   printf("typeconv_step():...\n");

 for (index = 0;
      index < parm->typeconv.num_pts;
      index++) {

   tmp32.u = plc_get(parm->typeconv.in_pt[index]);
   tmp16.u = (u16) (tmp32.u && 0xFFFF);

   switch (parm->typeconv.in_pt_type[index]) {
     case tc_type_u16: tmp_ld = (long double) tmp16.u; break;
     case tc_type_i16: tmp_ld = (long double) tmp16.i; break;
     case tc_type_u32: tmp_ld = (long double) tmp32.u; break;
     case tc_type_i32: tmp_ld = (long double) tmp32.i; break;
     case tc_type_f32: tmp_ld = (long double) tmp32.f; break;
     default         : continue;
   } /* switch(in_pt_type[]) */

   switch (parm->typeconv.out_pt_type[index]) {
     case tc_type_u16: tmp16.u = (u16)tmp_ld; tmp32.u = (u32)tmp16.u; break;
     case tc_type_i16: tmp16.i = (i16)tmp_ld; tmp32.u = (u32)tmp16.u; break;
     case tc_type_u32: tmp32.u = (u32)tmp_ld; break;
     case tc_type_i32: tmp32.i = (i32)tmp_ld; break;
     case tc_type_f32: tmp32.f = (f32)tmp_ld; break;
     default         : continue;
   } /* switch(in_pt_type[]) */

   plc_set (parm->typeconv.out_pt[index], tmp32.u);

   if (debug)
     printf("typeconv_step(): converted to value=%Lf, u32=%d\n", 
            tmp_ld, tmp32.u);

 } /* for */


 return 0;
}



/* helper function for parser */
static int get_type(char *in_str, tc_type_t *t_ptr)
{
  if (strcmp(in_str, TYPECONV_i16_STR) == 0) {
    *t_ptr = tc_type_i16;
    return 0;
  }

  if (strcmp(in_str, TYPECONV_u16_STR) == 0) {
    *t_ptr = tc_type_u16;
    return 0;
  }

  if (strcmp(in_str, TYPECONV_i32_STR) == 0) {
    *t_ptr = tc_type_i32;
    return 0;
  }

  if (strcmp(in_str, TYPECONV_u32_STR) == 0) {
    *t_ptr = tc_type_u32;
    return 0;
  }

  if (strcmp(in_str, TYPECONV_f32_STR) == 0) {
    *t_ptr = tc_type_f32;
    return 0;
  }

  return -1;
}


/* parses the parameters in the matplc.conf configuration table */
/* and initializes the parm variable.                             */
int typeconv_parse_parm (const char *table_name, 
                         int table_index,
                         fb_parm_t *parm,
		    	 f32 T /* period this dsp module will work with */)
/*
 * typeconv function matplc.conf syntax:
 *   fblock typeconv in_pt1 in_pt1_type out_pt1 out_pt1_type    in_pt_2 ...
 *
 * where:
 *   in_pt       : matplc point to be used as input of the type conversion
 *   in_pt_type  : format in which in_pt is being stored
 *   out_pt      : matplc point to be used as output of the type conversion
 *   out_pt_type : format in which out_pt will be stored
 *
 */
/* NOTE: the "fblock" in the row is really the value */
/* of the FBLOCK_TABLE_ID constant. (check in dsp.h) */
/* NOTE: the "typeconv" in the row is really the     */
/* value of the TYPECONV_TABLE_ID constant.          */
{
  char *tmp_val;
  int rowlen, index, pt_count;

  if (debug)
    printf("typeconv_parse_parm(): table=%s, index=%d\n",
            table_name, table_index);

  if (parm == NULL)
    return -1;

  /*
   * check that there's that many function blocks and that the block 
   * in question has the right number of fields
   */
  rowlen = conffile_get_table_rowlen(table_name, table_index);
  if (rowlen < 5) {
    plc_log_wrnmsg(1, 
                   "fblock row %d has wrong number of parameters for"
                   " an %s block.",
                   table_index, TYPECONV_TABLE_ID);
    return -1;
  }

  /* check that the block is really a typeconv block */
  tmp_val = conffile_get_table(table_name, table_index, 0);
  if ( strcmp (tmp_val, TYPECONV_TABLE_ID) != 0) {
    plc_log_wrnmsg(1,
                   "Row %d is not configuration of a %s function block.",
                   table_index, TYPECONV_TABLE_ID);
    free (tmp_val);
    return -1;
  }
  free (tmp_val);

  /* get the conversion quadruples */
  for (pt_count = 0, index = 0;
       pt_count < __TYPECONV_MAX_NUM_PTS; 
       index++) {

    /* get the input point */
    tmp_val = conffile_get_table(table_name, table_index, 1 + index*4);
     /* check if no more conversions are configured... */
    if (tmp_val == NULL)
      break; /* exit loop */
    parm->typeconv.in_pt[pt_count] = plc_pt_by_name(tmp_val);
    if (parm->typeconv.in_pt[pt_count].valid == 0) {
      plc_log_wrnmsg(1,
                     "Invalid input point %s, skipping this conversion.",
                     tmp_val);
      free (tmp_val);
      continue;
    }
    free (tmp_val);

    /* get in_pt type */
    tmp_val = conffile_get_table(table_name, table_index, 2 + pt_count*4);
    if (tmp_val == NULL) {
      plc_log_wrnmsg(1,
                     "No type specified for input point %s, "
                     "skipping this conversion.",
                     conffile_get_table(table_name, table_index, 1 + index*4));
      continue;
    }
    if (get_type(tmp_val, &(parm->typeconv.in_pt_type[pt_count])) 
        < 0) {
      plc_log_wrnmsg(1,
                     "Invalid type for input point %s, "
                     "skipping this conversion.",
                     conffile_get_table(table_name, table_index, 1 + index*4));
      continue;
    }

    /* get the output point */
    tmp_val = conffile_get_table(table_name, table_index, 3 + index*4);
    if (tmp_val == NULL) {
      plc_log_wrnmsg(1,
                     "No output point specified for input point %s,"
                     " Skipping this conversion.",
                     tmp_val);
      free (tmp_val);
      continue;
    }
    parm->typeconv.out_pt[pt_count] = plc_pt_by_name(tmp_val);
    if (parm->typeconv.out_pt[pt_count].valid == 0) {
      plc_log_wrnmsg(1,
                     "Invalid output point %s, skipping this conversion",
                     tmp_val);
      free (tmp_val);
      continue;
    }
    free (tmp_val);

    /* get out_pt type */
    tmp_val = conffile_get_table(table_name, table_index, 4 + pt_count*4);
    if (tmp_val == NULL) {
      plc_log_wrnmsg(1,
                     "No type specified for output point %s, "
                     "skipping this conversion.",
                     conffile_get_table(table_name, table_index, 3 + index*4));
      continue;
    }
    if (get_type(tmp_val, &(parm->typeconv.out_pt_type[pt_count])) 
        < 0) {
      plc_log_wrnmsg(1,
                     "Invalid type for output point %s, "
                     "skipping this conversion.",
                     conffile_get_table(table_name, table_index, 3 + index*4));
      continue;
    }

    /* conversion quadruple read correctly, go to next conversion... */
    pt_count++;

  } /* for(..) */

  parm->typeconv.num_pts = pt_count;

  if (debug)
    typeconv_dump_parm(*parm);

  /* if no conversion was configured, then exit with error */
  if (pt_count == 0) {
    plc_log_wrnmsg(1, "typeconv block found, "
                      "but no conversion correctly configured.");
    return -1;
  }

  return 0;
}



int typeconv_dump_parm(fb_parm_t parm)
{
 int index;

 printf("typeconv_dump_parm(): num_pts = %d\n",
        parm.add.num_pts);

 for (index = 0; index < parm.typeconv.num_pts; index++) {
   /* TODO: must finish writing up this function... */
   /* printf("in_pt%d = %s\n", index, parm.typeconv.in_pt[index]); */
 }

 return 0;
}

