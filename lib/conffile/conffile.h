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


#ifndef __CONFFILE_H
#define __CONFFILE_H

#include "../types.h"


/*
 * note: conffile_open() and conffile_close() have been replaced by
 * conffile_init() and conffile_done() respectively. Since they are called
 * by plc_init() and plc_done(), you probably won't need to call them
 * explicitly.
 *
 * The config is now parsed and read into memory by conffile_init().
 * Instead of the module argument, it now uses the global variable
 * plc_module_name as the default section [NOTE - this doesn't appear to be
 * true - Jiri 30.8.00]; any module can read any section.
 *
 * conffile_done() should throw the config out of the memory; it doesn't do
 * that yet, but you should call it anyway when appropriate.
 */
int conffile_init(const char *conffile, const char *mod_name);
int conffile_done(void);

/*
 * This will return a global variable name by number/positon in the dict.
 * added by Hugh Jack Apr., 30, 2002
 */
char *conffile_var_by_index(int);

/*
 * Dump the config (as currently in memory). Mostly for debugging (both of
 * this module and of users' matplc.conf files)
 */
void conffile_dump(void);

/*
 * Get the value of the given config variable as a string, allocated with
 * malloc(). Returns NULL if the variable is not found.
 *
 * The first form uses the module name as the section, the second has
 * explicit section name (eg for getting the PLC settings).
 */
char *conffile_get_value(const char *name);
int conffile_get_value_i32(const char *name,
                           i32 *val, i32 min_val, i32 max_val, i32 def_val);
int conffile_get_value_u32(const char *name,
                           u32 *val, u32 min_val, u32 max_val, u32 def_val);
int conffile_get_value_f32(const char *name,
                           f32 *val, f32 min_val, f32 max_val, f32 def_val);
int conffile_get_value_d  (const char *name,
                           double *val, double min_val,
                           double max_val, double def_val);

/* As above but with explicit section. */
char *conffile_get_value_sec(const char*section, const char *name);
int conffile_get_value_sec_i32(const char *section, const char *name,
                               i32 *val, i32 min_val, i32 max_val, i32 def_val);
int conffile_get_value_sec_u32(const char *section, const char *name,
                               u32 *val, u32 min_val, u32 max_val, u32 def_val);
int conffile_get_value_sec_f32(const char *section, const char *name,
                               f32 *val, f32 min_val, f32 max_val, f32 def_val);
int conffile_get_value_sec_d  (const char *section, const char *name,
                               double *val, double min_val,
                               double max_val, double def_val);

/*
 * Get the value of from a given config table as a string, allocated with
 * malloc(). Row and columns are zero-based. Returns NULL if the table is
 * not found or doesn't have that many rows or the row doesn't have that
 * many columns.
 */
char *conffile_get_table(const char *name, int row, int col);
int conffile_get_table_i32(const char *name, int row, int col, 
                           i32 *val, i32 min_val, i32 max_val, i32 def_val);
int conffile_get_table_u32(const char *name, int row, int col,
                           u32 *val, u32 min_val, u32 max_val, u32 def_val);
int conffile_get_table_f32(const char *name, int row, int col,
                           f32 *val, f32 min_val, f32 max_val, f32 def_val);
/* Get number of rows, or number of columns in a given row. 0 if not found. */
int conffile_get_table_rows(const char *name);
int conffile_get_table_rowlen(const char *name, int row);

/* As above but with explicit section. */
char *conffile_get_table_sec(const char *section, const char *name,
			     int row, int col);
int conffile_get_table_sec_i32(const char *section, const char *name, 
                               int row, int col, 
                               i32 *val, i32 min_val, i32 max_val, i32 def_val);
int conffile_get_table_sec_u32(const char *section, const char *name, 
                               int row, int col, 
                               u32 *val, u32 min_val, u32 max_val, u32 def_val);
int conffile_get_table_sec_f32(const char *section, const char *name, 
                               int row, int col, 
                               f32 *val, f32 min_val, f32 max_val, f32 def_val);
int conffile_get_table_rows_sec(const char *section, const char *name);
int conffile_get_table_rowlen_sec(const char *section, const char *name,
				   int row);


void conffile_parse_i32(const char *section, const char *name,
                        i32 *var, i32 min, i32 max, i32 def,
                        int use_default);


#endif /* __CONFFILE_H */
