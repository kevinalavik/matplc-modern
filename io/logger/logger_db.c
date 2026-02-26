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
#include "logger_db.h"

#include <mysql.h>

#include "db_wrap.h"

MYSQL   *conn;
char    *table;

static const int debug = 0;

/* Future change change to dynamic memory allocation */
plc_pt_t points[MAX_POINTS];
char point_names[MAX_POINTS][MAX_POINT_NAME_SIZE];
int npoints;
char *file_name;
FILE *log_file;

/* Logger loop */
int run_loop(void)
{
  struct timeval time;
  int i, val;
  int values[MAX_POINTS];

  /* start logging */
  while (1) {
    plc_scan_beg();
    plc_update();

    /* Log points */
    gettimeofday(&time, NULL);
    if(debug) printf("Log points %ld %ld ", time.tv_sec, time.tv_usec);
    if(log_file) fprintf(log_file, "%ld %ld ", time.tv_sec, time.tv_usec);
    for(i=0; i<npoints; i++){
      val = plc_get(points[i]);
      values[i] = val;
      if(debug) printf("%d ", val);
    }
    if(log_file) fprintf(log_file, "\n");
    if(debug) printf("\n");

    add_row(conn, table, npoints, point_names, values, time.tv_sec, time.tv_usec);

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
        strcpy(point_names[npoints], point);
        points[npoints] = get_pt(point);
        npoints++;
      }
    }
  }
  return 0;
}

int connect_to_db(void){
  char *host_name;
  char *user;
  char *password;
  char *port;
  unsigned int port_num;
  char *socket_name;
  char *db_name;

  host_name = conffile_get_value("host_name");
  if(debug) printf("host name %s\n", host_name);
  user = conffile_get_value("user");
  if(debug) printf("user %s\n", user);
  password = conffile_get_value("password");
  if(debug) printf("password %s\n", password);
  port = conffile_get_value("port");
  if(port){
        port_num = atoi(port);
  }
  else{
        port_num = 0;
  }
  if(debug) printf("port %d\n", port_num);
  socket_name = conffile_get_value("socket_name");
  if(debug) printf("socket name %s\n", socket_name);
  db_name = conffile_get_value("db_name");
  if(debug) printf("db name %s\n", db_name);
  table = conffile_get_value("table");
  if(debug) printf("table %s\n", table);

  conn = db_connect(host_name, user, password, db_name,
    port_num, socket_name, 0);
  if (conn == NULL){
      db_disconnect (conn);
      return -1;
  }
  if(table_exist(conn, table) == 0){
    if(compare_structure(conn, table, npoints, point_names) == 0){
      /* Structure OK */
    }
    else{
      db_disconnect (conn);
      return -2;
    }
  }
  else{
    plc_log_wrnmsg(1, "CREATE TABLE");
    if(create_table(conn, table, npoints, point_names) != 0){
      db_disconnect (conn);
      return -3;
    }
  }

  /* Where should I disconnect the database? */
  /* db_disconnect (conn); */

  return 0;
}

int main(int argc,char *argv[])
{
  if (plc_init("LOGGER_DB",argc,argv) < 0) {
    printf("Error connecting to PLC.\n");
    exit(EXIT_FAILURE);
  }
  if (get_config() < 0)
    exit(EXIT_FAILURE);
  if (connect_to_db() < 0)
    exit(EXIT_FAILURE);
  if (run_loop() < 0)
    exit(EXIT_FAILURE);

  return EXIT_SUCCESS;
}

