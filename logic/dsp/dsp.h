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
 * dsp.h
 */

#ifndef __DSP_H
#define __DSP_H


#include "dsp_t.h"


/* struct used by the list of currently supported function block types */
typedef struct {
  const char *conftable_id;
  int (*parse_parm_func) (const char *table_name,
                          int table_index,
                          fb_parm_t *parm,
		          f32 T /* period this dsp module will work with */);
  int (*step_func) (unsigned int step_count,
                    dsp_time_t *dsp_time,
                    fb_parm_t *parm);

} fb_types_t;





/* Function Block type */
typedef struct {
  int (*fb_step) (unsigned int step_count,
                  dsp_time_t *dsp_time,
                  fb_parm_t *parm);
  fb_parm_t fb_parm;

} fb_t;


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/

/* Default minimum period */
#define DEF_MIN_DELTA_T (1000)  /* in useconds */


#define FBLOCK_TABLE_ID "fblock"

#endif /* __DSP_H */

