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


#ifndef __STRING_UTIL_H
#define __STRING_UTIL_H

#include "../types.h"

/* 
 * convert a string to an (int, i32, u32 or f32) between min and max; 
 * if successful, the result is placed in *numb. On failure, *numb 
 * is guaranteed unchanged.
 *
 * Note that the returned value is a success/fail code (0 for OK).
 */
int string_str_to_int(const char *str, int *numb, int min, int max);
int string_str_to_i32(const char *str, i32 *numb, i32 min, i32 max);
int string_str_to_u32(const char *str, u32 *numb, u32 min, u32 max);
int string_str_to_f32(const char *str, f32 *numb, f32 min, f32 max);
int string_str_to_d  (const char *str, double *numb, double min, double max);

/*
 * convert a string to 7-bit ASCII. Non-ascii characters (0x80-0x7FFFFFFF)
 * are replaced by `?'. The result is allocated using malloc. The input
 * string is not checked for validity.
 *
 * Buglet: the space allocated may be excessive (it will be strlen(src)+1).
 */
char *string_u_to_a(const char*src);


/*
 * Join two/three strings together. Allocate space with malloc(3).
 */
char *strdup2(const char *a, const char *b);
char *strdup3(const char *a, const char *b, const char *c);

#endif /* __STRING_UTIL_H */
