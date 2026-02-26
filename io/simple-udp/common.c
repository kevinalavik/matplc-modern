#include "common.h"

/* retrieve the list of points from the named table */
pt_list_t *get_point_list(const char *tablename)
{
  int rows, columns, r, c, i, totalcount = 0;
  pt_list_t *res;
  char *name=NULL;

  rows = conffile_get_table_rows(tablename);

  for (r = 0; r < rows; r++)
    totalcount += conffile_get_table_rowlen(tablename, r);

  res =
      (pt_list_t *) malloc(sizeof(pt_list_t) +
			   totalcount * sizeof(plc_pt_t));
  if (!res)
    return NULL;

  res->count = totalcount;

  i = 0;
  for (r = 0; r < rows; r++) {
    columns = conffile_get_table_rowlen(tablename, r);
    for (c = 0; c < columns; c++) {
      name = conffile_get_table(tablename, r, c);
      res->pt[i]=plc_pt_by_name(name);
      if (!res->pt[i].valid) {
	printf("Couldn't get handle to point %s!!!\n", name);
	res->pt[i]=plc_pt_null();
      }
      if (name)
	free(name);
      i++;
    }
  }

  return res;
}
