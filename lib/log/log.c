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
 * This file implements the functions declared in log.h
 */ 


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "log.h"


static int debug = 0;

static const char *const TRC_HEADER = "TRACE";
static const char *const ERR_HEADER = "ERROR";
static const char *const WRN_HEADER = "WARNING";


static int log_fully_initialized_ = 0;

static int debug_level_ = 1;

static const char *mod_name_ = NULL;

static FILE *trcfile = NULL;
static FILE *wrnfile = NULL;
static FILE *errfile = NULL;


static int open_log_file(FILE **file, 
                         const char *fname)
{
 int res = 0;

 if (fname != NULL)
   *file = fopen(fname, "a");

 if (*file == NULL) {
   res = -1;
   *file = stderr;
   }

 if (res < 0)
   plc_log_wrnmsg(1, 
                  "Cannot append to log file %s. Writing to stderr instead.",
                  fname);

 return res;
}


int plc_log_init(int debug_level, 
                 const char *mod_name,   /* module name */
                 const char *trc_fname,
                 const char *wrn_fname,
                 const char *err_fname)
{
 if (debug)
   printf("plc_log_init(): mod_name = %s, trc_fname = %s, wrn_fname = %s, "
          " err_fname = %s\n", 
          mod_name, trc_fname, wrn_fname, err_fname);
      
 if (log_fully_initialized_ == 1)
   return -1;

 debug_level_ = debug_level;
 if ((mod_name_ = strdup(mod_name)) == NULL)
   return -1;

  /* 
   * Must begin with warning file. 
   * The open_log_file() function will log to it in case of error.
   */
 open_log_file(&wrnfile, wrn_fname);

 if (strcmp(wrn_fname, err_fname) == 0)
   errfile = wrnfile;
 else
   open_log_file(&errfile, err_fname);

 if (strcmp(trc_fname, err_fname) == 0)
   trcfile = wrnfile;
 else
   open_log_file(&trcfile, trc_fname);

 log_fully_initialized_ = 1;

 plc_log_trcmsg(9, "This trace log file initialized.");
 plc_log_errmsg(9, "This error log file initialized.");
 plc_log_wrnmsg(9, "This warning log file initialized.");

 return 0;
}

/* 
 * Make this function robust and re-entrant, i.e. may be called multiple times,
 * and even when the log system is only partially setup.
 */
int plc_log_done(void)
{
 int res = 0;

 if (debug)
   printf("plc_log_done(): ...\n");

 plc_log_trcmsg(9, "Closing this trace log file.");
 plc_log_errmsg(9, "Closing this error log file.");
 plc_log_wrnmsg(9, "Closing this warning log file.");

 if (mod_name_ != NULL)
   free((void *)mod_name_);
 mod_name_ = NULL;

 if (trcfile != NULL) {
   if (fclose(trcfile) != 0) res = -1; 
   if (wrnfile == trcfile) wrnfile = NULL;
   if (errfile == trcfile) errfile = NULL;
   trcfile = NULL;
 }  

 if (wrnfile != NULL) {
   if (fclose(wrnfile) != 0) res = -1; 
   if (errfile == wrnfile) errfile = NULL;
   wrnfile = NULL;
 }  

 if (errfile != NULL) {
   if (fclose(errfile) != 0) res = -1; 
   errfile = NULL;
 }  

 log_fully_initialized_ = 0;

 return 0;
}


/* 
 * This function is the implementation of the three plc_log_xxxmsg() functions
 * and should therefore be protected agaisnt being called before plc_log_init()
 */
static int log_msg(FILE *file,
                   const char *header,
                   int min_debug_level,
                   const char *format,
                   va_list arg_list)
{
 char      char_time[32];
 time_t    compact_time;

 if (debug)
   printf("log_msg(): file = %s msg = %s\n", (char *)file, format);

 if (log_fully_initialized_ != 1)
   return -1;

 if (debug_level_ < min_debug_level)
   return 0;
/*
 * We should really lock the file here so no other 
 * process may interrupt our log record.
 * Not doing it at the moment because I don't think
 * that the log function should introduce any
 * bigger delay than strictly necessary. For the
 * moment let's just hope for the best and see how
 * it goes. 
 */

 if (time (&compact_time) != (time_t)(-1))
   if (ctime_r (&compact_time, char_time) == char_time)
     fprintf (file, char_time);

 fprintf(file, " - %s - %s - ", header, mod_name_);
 vfprintf(file, format, arg_list);
 fprintf(file, "\n");
 fflush(file);

 return 0;
}



void plc_log_trcmsg(int min_debug_level,
                    const char *format,
                    ...)
{
 va_list arg_list;

 va_start(arg_list,format);
 log_msg( trcfile, TRC_HEADER, min_debug_level, format, arg_list);
 va_end(arg_list);
}


void plc_log_wrnmsg(int min_debug_level,
                    const char *format,
                    ...)
{
 va_list arg_list;

 va_start(arg_list,format);
 log_msg(wrnfile, WRN_HEADER, min_debug_level, format, arg_list);
 va_end(arg_list);
}


void plc_log_errmsg(int min_debug_level,
                    const char *format,
                    ...)
{
 va_list arg_list;

 va_start(arg_list,format);
 log_msg(errfile, ERR_HEADER, min_debug_level, format, arg_list);
 va_end(arg_list);
}
