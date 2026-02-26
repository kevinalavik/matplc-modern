/*
 * (c) 2002 Juan Carlos Orozco
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
 * this file implements the data logger module
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>

#include <plc.h>
#include <misc/string_util.h>
#include <logic/timer.h>
#include "logger.h"

/* definitions */
static const int debug = 0;

plc_pt_t points[MAX_POINTS];
int npoints;
char *file_name;
FILE *log_file;

int run_loop(void)
{
  struct timeval time;
  int i, val;

  /* start logging */
  while (1) {
    plc_scan_beg();
    plc_update();

    if( (log_file = fopen(file_name, "a")) ){
    }
    else{
      if(debug) printf("File error: %s\n", file_name);
      plc_log_wrnmsg(1, "File error: %s", file_name);
    }

    /* Log points */
    gettimeofday(&time, NULL);
    if(debug) printf("Log points %ld %ld ", time.tv_sec, time.tv_usec);
    if(log_file) fprintf(log_file, "%ld %ld ", time.tv_sec, time.tv_usec);
    for(i=0; i<npoints; i++){
      val = plc_get(points[i]);
      if(log_file) fprintf(log_file, "%d ", val);
      if(debug) printf("%d ", val);
    }
    if(log_file) fprintf(log_file, "\n");
    if(debug) printf("\n");

    fclose(log_file);

    plc_update();
    plc_scan_end();
  } /* while (1) */

  return -1;
}

plc_pt_t get_pt(const char *pt_name)
{
  plc_pt_t pt_handle;

  pt_handle = plc_pt_by_name(pt_name);

  if (!pt_handle.valid) {
    printf("Could not get valid handle to %s.\n", pt_name);
    exit(1);
  }

  return pt_handle;
}

/* get everything from the config */
int get_config(void)
{
  int rows, cols, i, j;
  char *point;

  npoints = 0;

  file_name = conffile_get_value("file");
  if(debug) printf("file name %s\n", file_name);

  /* get points to log */
  /* Probably do this twice to alocate the points dynamically */
  rows = conffile_get_table_rows("points");
  for(i=0; i<rows; i++){
    cols = conffile_get_table_rowlen("points", i);
    for(j=0; j<cols; j++){
      point = conffile_get_table("points", i, j);
      if(debug) printf("Point %s\n", point);
      if(npoints<MAX_POINTS){
        points[npoints] = get_pt(point);
        npoints++;
      }
    }
  }

  return 0;
}

int main(int argc,char *argv[])
{
  if (plc_init("LOGGER",argc,argv) < 0) {
    printf("Error connecting to PLC.\n");
    exit(EXIT_FAILURE);
  }

  if (get_config() < 0)
    exit(EXIT_FAILURE);

  if (run_loop() < 0)
    exit(EXIT_FAILURE);

  return EXIT_SUCCESS;
}

