#ifndef __DB_WRAP_H
#define __DB_WRAP_H

#define MAX_POINT_NAME_SIZE 80

#endif /* __DB_WRAP_H */


void log_error(MYSQL *conn, const char *message);

MYSQL *db_connect(const char *host_name, const char *user_name, const char *password, const char *db_name,
        unsigned int port_num, const char *socket_name, unsigned int flags);

void db_disconnect(MYSQL *conn);

int table_exist(MYSQL *conn, char *table);

int compare_structure(MYSQL *conn, char *table, int npoints, char point_names[][]);

int create_table(MYSQL *conn, char *table, int npoints, char point_names[][]);

int add_row(MYSQL *conn, char *table, int npoints, char point_names[][], int values[], long sec, long usec);
