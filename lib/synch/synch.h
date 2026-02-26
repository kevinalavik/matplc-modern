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


#ifndef __SYNCH_H
#define __SYNCH_H

#include "../types.h"

/*******************/
/*                 */
/*   DATA TYPES    */
/*                 */
/*******************/

/* transition handle
 *  will be == INVALID_TRANSITION when invalid...
 */
typedef void * plc_transition_t;

#define INVALID_TRANSITION NULL
#define plc_transition_is_valid(transition) ((transition) != INVALID_TRANSITION)


/* synch point handle
 *  will be == INVALID_SYNCHPT when invalid...
 */
typedef void * plc_synchpt_t;

#define INVALID_SYNCHPT NULL
#define plc_synchpt_is_valid(synchpt) ((synchpt) != INVALID_SYNCHPT)


/* synch place handle
 *  will be == INVALID_PLACE when invalid...
 */
typedef int plc_synchplace_t;

#define INVALID_SYNCHPLACE -1
#define plc_synchplace_is_valid(place) ((place) != INVALID_SYNCHPLACE)



/*******************/
/*                 */
/*     GENERAL     */
/*                 */
/*******************/

int synch_init(const char *module_name);
int synch_done(void);

int synch_scan_beg(void);
int synch_scan_end(void);

/* SYNCHRONISATION function */
int plc_synch(plc_synchpt_t synchpt);


/*******************/
/*                 */
/*   TRANSITIONs   */
/*                 */
/*******************/

/* Get a transition handle. (This is mostly a function of the Confmap rather
 * than the SYNCH proper, but the SYNCH gets a say, too.)
 *
 * If the transition doesn't exist, INVALID_TRANSITION will be returned.
 */
plc_transition_t plc_transition_by_name(const char *name);

/* Get a transition handle. (This is mostly a function of the Confmap rather
 * than the SYNCH proper, but the SYNCH gets a say, too.)
 *
 * If the transition doesn't exist, INVALID_TRANSITION will be returned.
 *
 * If (*name  != NULL), the caller must deallocate memory with free(*name)
 */
plc_transition_t plc_transition_by_index(int transition_index, char **name);

/* Create a handle to a null transition.
 *
 * This function creates a handle to a null transition.
 * Calls to plc_synch with a synchpt created from this transition
 * will return imediately.
 */
plc_transition_t plc_transition_null(void);

/* Get the number of configured transitions */
int plc_transition_count(void);

/* Get the details of a transition... */
/* This is intended to be used mostly by control programs (i.e. matplc utility.
 * and not by general modules.
 */
typedef i16 plc_transition_swvalue_t;
int plc_transition_details(plc_transition_t transition,
                             /* number of entries in each of the following arrays... */
                           int *place_count,
                           plc_transition_swvalue_t *wait_values[],
                           plc_transition_swvalue_t *signal_values[]);


/*******************/
/*                 */
/*     PLACEs      */
/*                 */
/*******************/

/* Create a place */
plc_synchplace_t plc_synchplace_add(void);

/* Delete a place */
int plc_synchplace_del(plc_synchplace_t synchplace);



/*******************/
/*                 */
/*    SYNCHPTs     */
/*                 */
/*******************/

/* Create a new synchpt.
 *
 * extra_arcs -> maximum number of arcs it will be possible to add to the synchpt
 *               later on by calling plc_synchpt_add_arc()
 *
 * In case of errors when creating the synchpt, an
 * INVALID_SYNCHPT will be returned.
 */
typedef enum {plc_synchpt_blocking, plc_synchpt_nonblocking}
  plc_synchpt_blocking_t;

plc_synchpt_t plc_synchpt_new(plc_transition_t transition,
                              unsigned int extra_arcs,
                              plc_synchpt_blocking_t blocking_opt);

/* De-allocate memory reserved for a synchpt */
void plc_synchpt_free(plc_synchpt_t synchpt);

/* Add an arc to a synchpt... */
int plc_synchpt_add_arc(plc_synchpt_t synchpt, int weight, plc_synchplace_t place);

/* Returns handle to the synchpt being used for scan_beg... */
/* Should probably only be used by the state library!!      */
plc_synchpt_t plc_synchpt_scan_beg(void);

#endif /* __SYNCH_H */
