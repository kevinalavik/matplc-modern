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
/*          The Local Version of the GMM           */
/*                                                 */
/***************************************************/


#ifndef __GMM_LOCAL_H
#define __GMM_LOCAL_H

#include "gmm_private.h"
#include "gmm.h"


/* This file contains the private 'interface' of the local version */
/* of the gmm, i.e. it contains the variables and type definitions */
/* that are used internally by the local version of the gmm, and   */
/* should not be visible to outside users.                         */



/**************************************************************/
/*****************                             ****************/
/*****************  Global (to gmm) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

/* here is the init function (to be called by gmm_init() only) */
int gmm_init_local(int privmap_key);

/* these functions are common to local and isolate             */
int      plc_pt_count_local(void);
plc_pt_t plc_pt_by_index_local(int index, char **name, char **owner);
plc_pt_t plc_pt_by_name_local(const char *name);
plc_pt_t plc_pt_by_loc_local(i32 offset, u8 bit, u8 length);


#endif /* __GMM_LOCAL_H */
