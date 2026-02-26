#include <stdio.h>
#include <string.h>
#include <plc.h>
#include <mysql.h>

#include "db_wrap.h"

static const int debug = 0;

void log_error(MYSQL *conn, const char *message)
{
  if(debug) printf("%s\n", message);
  plc_log_wrnmsg(1, message);
}

MYSQL *db_connect(const char *host_name, const char *user_name, const char *password, const char *db_name,
        unsigned int port_num, const char *socket_name, unsigned int flags)
{
  MYSQL *ret_conn;

  ret_conn = mysql_init(NULL);
  if(ret_conn == NULL){
    log_error(NULL, "mysql_init() failed.");
    return(NULL);
  }
  if(mysql_real_connect(ret_conn, host_name, user_name, password, db_name, port_num, socket_name, flags) == NULL){
    log_error(ret_conn, "mysql_real_connect() failed");
    return(NULL);
  }
  if(db_name != NULL)
    if (mysql_select_db (ret_conn, db_name) != 0){
      log_error (ret_conn, "mysql_select_db() failed");
      mysql_close (ret_conn);
      return (NULL);
  }

  return (ret_conn);
}

void db_disconnect(MYSQL *conn)
{
  mysql_close(conn);
}

int table_exist(MYSQL *conn, char *table)
{
  MYSQL_RES *result;
  int num_rows;
  
  result = mysql_list_tables(conn, table);
  if(result){
    num_rows = mysql_num_rows(result);
    mysql_free_result(result);
    if(num_rows < 1){
      log_error(conn, "Could not find table.");
      return -1;
    }
  }
  else{
    log_error(conn, "Could not find table.");
    return -1;
  }
  return 0;
}

/* Check types using enum_field_types */
int compare_structure(MYSQL *conn, char *table, int npoints, char point_names[][]){
  char str[100];
  int i;
  MYSQL_RES *result;
  MYSQL_ROW row;
  int num_fields;

  snprintf(str, 100, "DESCRIBE %s", table);
  /* log_error(NULL, str); */

  if(mysql_query(conn, str) != 0){
    log_error(conn, "Different structure. 1");
    return -1;
  }
  result = mysql_store_result(conn);

  if(result){
    num_fields = mysql_num_fields(result);
    i = 0;
    while ((row = mysql_fetch_row(result))){
      unsigned long *lengths;
      lengths = mysql_fetch_lengths(result);
      snprintf(str, 100, "%.*s", (int) lengths[0], row[0] ? row[0] : "NULL");
      if(i == 0){
        if(strcmp("datetime", str)!=0){
          mysql_free_result(result);
          log_error(conn, str);
          log_error(conn, "Different structure. 2");
          return -1;
        }
      }
      else if(i == 1){
        if(strcmp("sec", str)!=0){
          mysql_free_result(result);
          log_error(conn, str);
          log_error(conn, "Different structure. 3");
          return -1;
        }
      }
      else if(i == 2){
        if(strcmp("usec", str)!=0){
          mysql_free_result(result);
          log_error(conn, str);
          log_error(conn, "Different structure. 4");
          return -1;
        }
      }
      else if(strcmp((char *)point_names+((i-3)*MAX_POINT_NAME_SIZE), str)!=0){
        mysql_free_result(result);
        log_error(conn, str);
        log_error(conn, "Different structure. 5");
        return -1;
      }
      else{
        /* Check type */
      }
      i++;
    }
  }
  return 0;
}

int create_table(MYSQL *conn, char *table, int npoints, char point_names[][]){
  char str[2000], str2[200];
  int i;

  snprintf(str, 1000, "create table %s (datetime TIMESTAMP(14), sec BIGINT, usec BIGINT", table);
  for(i=0; i<npoints; i++){
        snprintf(str2, 200, ", %s INT", (char *)point_names+(i*MAX_POINT_NAME_SIZE));
        strcat(str, str2);
  }
  strcat(str, ")");

  /* log_error(NULL, str); */
  if(mysql_query(conn, str) != 0){
    log_error(conn, "Could not create table.");
    return -1;
  }
  return 0;
}

int add_row(MYSQL *conn, char *table, int npoints, char point_names[][], int values[], long sec, long nsec){
  char str[2000], str2[200];
  int i;

  snprintf(str, 1000, "INSERT INTO %s (sec, usec", table);
  for(i=0; i<npoints; i++){
        snprintf(str2, 200, ", %s", (char *)point_names+(i*MAX_POINT_NAME_SIZE));
        strcat(str, str2);
  }
  snprintf(str2, 200, ") VALUES ( %ld, %ld", sec, nsec);
  strcat(str, str2);
  for(i=0; i<npoints; i++){
        snprintf(str2, 200, ", %d", values[i]);
        strcat(str, str2);
  }
  strcat(str, ")");

  /* log_error(NULL, str); */
  if(mysql_query(conn, str) != 0){
    log_error(conn, "Could not create table.");
    return -1;
  }
  return 0;
}

