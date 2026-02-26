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


#ifndef __GMM_H
#define __GMM_H

#include "../types.h"

/*** GENERAL ***/

typedef enum {
              loc_default,
              loc_local,
              loc_isolate,
              loc_shared
             } gmm_loc_t;

int gmm_init(const char *module_name,
             gmm_loc_t location,
             int privmap_shm_key);

int gmm_done(void);

/*
 * point handle (to be considered opaque except for valid: will be non-zero
 * for valid, zero for invalid)
 */
typedef struct {
  i16 valid;
  u8  opaque[14];
} plc_pt_t;

/*
 * Get a point handle. (This is mostly a function of the CMM rather
 * than the GMM proper, but the GMM gets a say, too.)
 *
 * If the point doesn't exist, the `valid' field of the result will be 0.
 *
 * If plc_magic_bit_aliases is non-zero, the point is not found, and its
 * name ends in a .bit specification (.0 to .31), the function will look
 * for the point without the .bit and return a subpt handle to that bit.
 * For instance, if "foo.3" doesn't exist it'll return a subpt of "foo".
 */
plc_pt_t plc_pt_by_name(const char *name);

/*
 * Whether or not to allow the point.bit syntax - sometimes useful, but can
 * lead to confusion if you forget it's there. Default to OFF, but the
 * default can be changed by setting this to 1 *before* calling init.
 */
extern int plc_magic_bit_aliases;

/*
 * Get a point handle. (This is mostly a function of the CMM rather
 * than the GMM proper, but the GMM gets a say, too.)
 *
 * If the point doesn't exist, the `valid' field of the result will be 0.
 *
 * If (*name  != NULL) must deallocate memory with free(*name)
 * If (*owner != NULL) must deallocate memory with free(*owner)
 */
plc_pt_t plc_pt_by_index(int index, char **name, char **owner);

/*
 * Get the number of configured points
 */
int plc_pt_count(void);

/*
 * Create a sub_point handle.
 *
 * This functions creates a handle to an anonymous point corresponding to
 * 'length' bits of point 'parent_pt', starting from 'bit' of that
 * 'parent_pt'
 *
 * If the point doesn't exist, the `valid' field of the result will be 0.
 */
plc_pt_t plc_subpt(plc_pt_t parent_pt, u8 bit, u8 length);

/*
 * Create a handle to a null point.
 *
 * This function creates a handle to a null anonymous point. This is like
 * /dev/null - anything written to it is discarded, on reading it returns
 * zero.
 */
plc_pt_t plc_pt_null(void);


/*
 * Create a point handle.
 *
 * USE AT YOUR OWN DISCRETION
 *
 * Point handles obtained through this function may not be sensible !!
 *
 * NOTE: This function builds a point handle from information passed to it.
 *       A valid handle may be returned that does not correspond to any
 *       existing configured point. The point handle returned may overlap
 *       or partially overlap, one or more than one configured point
 *       simultaneously. The returned handle may even not overlap any
 *       configured point at all.
 *
 * If the values given are invalid, the `valid' field of the result will be 0.
 */
plc_pt_t plc_pt_by_loc(i32 offset, u8 bit, u8 length);

/*
 * Get the length (number of bits) of a point.
 *
 * returns -1 on error.
 */
int plc_pt_len(plc_pt_t p);

/*
 * Get the read/write status of a point, as an integer or constant string.
 *
 * returns:
 *    1 = "read-only"
 *    2 = "read-write" (actually write-only, but that'd be confusing)
 *    3 = "mixed" (some read, some write)
 *    0 = "null" (neither read nor write)
 *   -1 on error = NULL
 */
int plc_pt_rw(plc_pt_t p);
const char *plc_pt_rw_s(plc_pt_t p);

/*
 * Get various details about a point.
 *
 * null pointers are skipped. For invalid point handles, only the *valid
 * variable is guaranteed to be set - others may or may not be. Similarly
 * for a null point - the other variables may or may not be set.
 *
 * NOTE: this function is intended to be rarely-used and then only when
 * intimate details from the GMM are required. As such, from time to time,
 * new parameters may be added to this function (at the end of the argument
 * list), to prevent the proliferation of numerous barely-useful functions
 * for each new detail.
 *
 * If you need any of this information in an ordinary I/O or logic module,
 * ask us for a function returning that particular information only.
 */
void plc_pt_details(plc_pt_t p,	/* the point handle - compulsory */
		    int *valid,	/* is the point valid? */
		    int *length,	/* length in bits */
		    i32 *offset,	/* word address in memory area */
		    int *bit,	/* bit number 0-31 of LSB of point */
		    int *rw	/* read/write (like plc_pt_rw) */
    );

/*
 * UPDATE the inputs/outputs/coils (all or specified); each of these update
 * operations is atomic with respect to any other task.
 */
int plc_update(void);
int plc_update_pt(plc_pt_t p);
int plc_update_pts(plc_pt_t p[], int count);


/*
 * ACCESS functions; for points narrower than 32 bits, the data will be /
 * should be placed in the least-significant bits, with remaining bits 0.
 */
u32 plc_get(plc_pt_t p);
int plc_set(plc_pt_t p, u32 val);

/*
 * ACCESS functions; similar to the previous, but the returned/stored value
 * will be inverted.
 */
u32 plc_geti(plc_pt_t p);
int plc_seti(plc_pt_t p, u32 val);

/*
 * ACCESS functions; Interpret stored value as a 32 bit float (f32).
 */
f32 plc_get_f32(plc_pt_t p);
int plc_set_f32(plc_pt_t p, f32 val);


#endif /* __GMM_H */
