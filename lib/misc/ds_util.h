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

#ifndef __DS_UTIL_H
#define __DS_UTIL_H

/*
 * Misc data structures used by the confmap (and perhaps others). This is
 * internal stuff, folks, and may change without warning. If you want to
 * use it, cut'n'paste.
 */

#include <stdlib.h>

/* A simple variable-sized array. */

typedef struct {
  int count;
  void **data;
} list;

static inline list *list_new(void)
{
  list *lst = malloc(sizeof(list));
  if (lst == NULL)
    return NULL;

  lst->count = 0;
  lst->data = NULL;
  return lst;
}

static inline void list_delete(list *lst)
{
  if (lst == NULL)
    return;
  free(lst->data);
  free(lst);
}

static inline int list_append(list * lst, void *item)
{
  void **tmp_data;

  if (lst == NULL)
    return -1;

  tmp_data = realloc(lst->data, (lst->count+1) * sizeof(void *));
  if (tmp_data == NULL)
    return -1;

  lst->data = tmp_data;
  lst->count++;
  lst->data[lst->count - 1] = item;

  return 0;
}

static inline int list_len(list * lst)
{
  if (lst == NULL)
    return -1;
  return lst->count;
}

static inline void *list_item(list * lst, int index)
{
  if (lst == NULL)
    return NULL;
  if ((index < 0) || (index >= lst->count))
    return NULL;
  return lst->data[index];
}


/*
 * A simple dictionary type. Can't use hsearch(3) because we want to be
 * able to iterate through the thing, so use qsort(3) and bsearch(3).
 */

#include <search.h>

typedef struct {
  int count;
  ENTRY *data;
} dict;

static inline dict *dict_new(void)
{
  dict *dct = malloc(sizeof(dict));
  if (dct == NULL)
    return NULL;
  dct->count = 0;
  dct->data = NULL;
  return dct;
}

static inline void dict_delete(dict *dct)
{
  if (dct == NULL)
    return;
  free(dct->data);
  free(dct);
}

static int dict_compar_(const void*a, const void*b) {
  return strcmp(
      ((ENTRY*)a)->key,
      ((ENTRY*)b)->key);
}

static inline int dict_add(dict * dct, char *key, void *data)
{
  ENTRY *tmp_data;

  if (dct == NULL)
    return -1;

  tmp_data = realloc(dct->data, (dct->count+1) * sizeof(ENTRY));
  if (tmp_data == NULL)
    return -1;

  dct->data = tmp_data;
  dct->count++;
  dct->data[dct->count - 1].key=key;
  dct->data[dct->count - 1].data=data;
  qsort(dct->data, dct->count, sizeof(ENTRY), dict_compar_);

  return 0;
}

static inline int dict_len(dict * dct)
{
  if (dct == NULL)
    return -1;
  return dct->count;
}

static inline char *dict_key(dict * dct, int index)
{
  if (dct == NULL)
    return NULL;
  if ((index < 0) || (index >= dct->count))
    return NULL;
  return dct->data[index].key;
}

static inline void *dict_value(dict * dct, int index)
{
  if (dct == NULL)
    return NULL;
  if ((index < 0) || (index >= dct->count))
    return NULL;
  return dct->data[index].data;
}

static inline void *dict_get(dict * dct, const char * key)
{
  ENTRY e = {(char*)key, NULL}; ENTRY *res;

  if (dct == NULL)
    return NULL;

  res=bsearch(&e, dct->data, dct->count, sizeof(ENTRY),dict_compar_);

  if (res)
    return res->data;
  else
    return NULL;
}

#endif				/* __DS_UTIL_H */
