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
/*        The Isolate Version of the GMM           */
/*                                                 */
/***************************************************/


#ifndef __GMM_ISOLATE_H
#define __GMM_ISOLATE_H

#include "gmm_private.h"
#include "gmm.h"
/* this version uses some functions from the local verson */
#include "gmm_local.h"

/* This file contains the private 'interface' of the isolate version */
/* of the gmm, i.e. it contains the variables and type definitions   */
/* that are used internally by the isolate version of the gmm, and   */
/* should not be visible to outside users.                           */



/**************************************************************/
/*****************                             ****************/
/*****************  Global (to gmm) Functions  ****************/
/*****************                             ****************/
/**************************************************************/

/* here is the init function (to be called by gmm_init() only) */
int gmm_init_isolate(int privmap_key);



#endif /* __GMM_ISOLATE_H */
