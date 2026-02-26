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
 * String Handling Utility Routines
 *
 * This file implements the routines in string_util.h
 *
 * These routines merely make life simpler when working with
 * strings
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>    /* required for isspace() */
#include <errno.h>
#include <limits.h>


#include "types.h"


static const int string_debug = 0;

#define _strtod_(str,err_str)  strtod ((str), (err_str))
#define _strtol_(str,err_str)  strtol ((str), (err_str), 0)
#define _strtoul_(str,err_str) strtoul((str), (err_str), 0)

/* returns 0 if successful */
/* returns -1 on error     */
#define __string_util_str_to_type(type,conv_func,str,numb,min,max)   \
{                                                                    \
 type tmp_numb;                                                      \
 char *error_char;                                                   \
                                                                     \
 if ((str) == NULL) return -1;                                       \
 tmp_numb = conv_func((str), &error_char);                           \
                                                                     \
 if ((errno == ERANGE) ||                                            \
     ((*error_char) != 0) ||                                         \
     (error_char == str) ||                                          \
     (tmp_numb < (min)) || (tmp_numb > (max)))                       \
   return -1;                                                        \
                                                                     \
 *(numb) = tmp_numb;                                                 \
 return 0;                                                           \
}


/* return 0 if successful */
/* return -1 on error     */
int string_str_to_int(const char *str, int *numb, int min, int max)
{
  __string_util_str_to_type(int, _strtol_, str, numb, min, max);
}

int string_str_to_i32(const char *str, i32 * numb, i32 min, i32 max)
{
  __string_util_str_to_type(i32, _strtol_, str, numb, min, max);
}

int string_str_to_u32(const char *str, u32 * numb, u32 min, u32 max)
{
  /* working around a bug in strtoul that does not detect an error
     if a negative number is given!
   */
  if (str != NULL) {
    while (isspace(*str))
      str++;
    if (str[0] == '-')
      return -1;
  }

  __string_util_str_to_type(u32, _strtoul_, str, numb, min, max);
}

int string_str_to_f32(const char *str, f32 * numb, f32 min, f32 max)
{
  __string_util_str_to_type(f32, _strtod_, str, numb, min, max);
}

int string_str_to_d(const char *str, double * numb, double min, double max)
{
  __string_util_str_to_type(double, _strtod_, str, numb, min, max);
}


/* UTF-8 to ASCII */
char *string_u_to_a(const char *src)
{
  char *res;
  int i = 0;
  if (!src)			/* don't crash if src is null */
    return NULL;
  res = malloc(strlen(src)+1);
  if (!res)			/* malloc failed */
    return NULL;

  for (; *src; src++) {
    switch (*src & 0xC0) {
    case 0xC0:			/* UTF-8 begin multibyte char */
      res[i++] = '?';
      break;
    case 0x80:			/* UTF-8 continue multibyte char */
      break;
    default:			/* 7-bit ASCII */
      res[i++] = *src;
    }
  }

  res[i]='\0';
  return res;
}

/*
 * Join two strings together. Allocate space with malloc(3).
 */
char *strdup2(const char *a, const char *b)
{
  char *res = malloc(strlen(a) + strlen(b) + 1);
  if (!res)
    return 0;
  return strcat(strcpy(res, a), b);  /* safe, actually */
}

/*
 * Join three strings together. Allocate space with malloc(3).
 */
char *strdup3(const char *a, const char *b, const char *c)
{
  char *res = malloc(strlen(a) + strlen(b) + strlen(c) + 1);
  if (!res)
    return 0;
  return strcat(strcat(strcpy(res, a), b), c);  /* safe, actually */
}

