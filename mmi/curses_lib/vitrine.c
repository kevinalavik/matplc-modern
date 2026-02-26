/*
 * (c) 2000 Jiri Baum
 * (c) 2001 Mario de Sousa
 * (c) 2002 Hugh Jack
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

/* use GNU extensions - in particular the getline() function */

/*
 * An HMI front end for screen display
 *
 * this version has been adapted from the original version for the library
 * module format.
 *
 * Last Revised: April 16, 2002
 */


#define _GNU_SOURCE

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <sched.h>

#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


char *bkg=NULL;

/* The strings in the config file used to define what format the data is in */
#define  f32_TYPE "f32"
#define  i32_TYPE "i32"
#define  u32_TYPE "u32"
#define  i16_TYPE "i16"
#define  u16_TYPE "u16"
#define   i8_TYPE "i8"
#define   u8_TYPE "u8"
#define bool_TYPE "bool"

typedef enum {bool_dt, i32_dt, u32_dt, i16_dt, u16_dt, i8_dt, u8_dt, f32_dt} data_type_t;


typedef struct {
  plc_pt_t pt;		 /* the point to be displayed */
  data_type_t data_type; /* how to interpret the data stored in the point */
} data_t;


/* data required to display data as values */
typedef struct {
  data_t data;
  int x, y;       /* position on screen */
  chtype on, off; /* markers for on and off when data type is bool */
  /* other stuff to be added later */
  /*
  int numb_digits;
  int numb_digits_exp: // number of digits for the exponent //
  int numb_digits_ ...
  */
} disp_value_t;



/* data required to define a graph 'window' */
typedef struct {
  char *name;
  int max_x, min_x; /* position on screen for horizontal axis */
  int max_y, min_y; /* position on screen for vertical   axis */
  f32 max_x_val, min_x_val;  /* range of values for horizontal axis */
  f32 max_y_val, min_y_val;  /* range of values for vertical   axis */
  chtype back_marker;        /* character used for the backgorund */
} graph_win_t;



typedef enum {bar_ft, point_ft} fill_mode_t;

/* data required to display data in a graph graph */
typedef struct {
  graph_win_t *graph;
  data_t data_x;
  data_t data_y;
  int old_x, old_y;  /* used to store previous location of marker */
  chtype marker;    /* defaults to '*' */
  /* fill_mode_t fill_mode; */
} disp_graph_t;



disp_value_t *disp_value = NULL;
disp_graph_t *disp_graph = NULL;
graph_win_t  *graph_win  = NULL;
int disp_value_items = 0;
int disp_graph_items = 0;
int graph_win_items  = 0;




int get_value_type(const char *table_name, int row, int *col, data_type_t *dt)
{
  char * s;

  s = conffile_get_table(table_name,row,*col);
    *dt = i32_dt;
    (*col)++;
         if (strcmp(s,  f32_TYPE) == 0) *dt =  f32_dt;
    else if (strcmp(s,  i32_TYPE) == 0) *dt =  i32_dt;
    else if (strcmp(s,  u32_TYPE) == 0) *dt =  u32_dt;
    else if (strcmp(s,  i16_TYPE) == 0) *dt =  i16_dt;
    else if (strcmp(s,  u16_TYPE) == 0) *dt =  u16_dt;
    else if (strcmp(s,   i8_TYPE) == 0) *dt =   i8_dt;
    else if (strcmp(s,   u8_TYPE) == 0) *dt =   u8_dt;
    else if (strcmp(s, bool_TYPE) == 0) *dt = bool_dt;
    else (*col)--;
    free(s);

  return 0;
}






/* normal terminal size is 23, but we want to support larger terminals... */
/* ncurses initializes the LINES and COL variables with the current size of the scrren */
/* We need to call initscr() to get them initialized...			  */
#define MAX_CURSES_WIN_Y LINES
#define MAX_CURSES_WIN_X COLS


#define DRAW_TABLE "draw"
#define PLOT_TABLE "plot"

#define GRAPH_TABLE "graph"
#define DEF_MIN_X_VAL 0
#define DEF_MAX_X_VAL 1
#define DEF_MIN_Y_VAL 0
#define DEF_MAX_Y_VAL 1



void get_config(void)
{
  int i, m, error, next_table_pos;
  char *s;
  bkg = conffile_get_value("background");

  /*
   * Read the config table. This is probably an interim format.
   * TODO: there should be some way of specifying colour for each marker.
   */

  /* Format of the "show" table, used to display the value in digits */
  /* show plc_pt_name x y [data_format] [on_marker [off_marker]]     */
  /* where:                                                          */
  /*   data_format: f32 | i32 | u32 | i16 | u16 | i8 | u8 | bool     */
  /*                defaults to bool                                 */
  /*   on/off_marker: characters to use when value is true/false     */
  /*                  only valid if data_format is bool              */
  /*                  defaults to '1'/'0'                            */
  disp_value_items = conffile_get_table_rows("show");
  disp_value = malloc(disp_value_items * sizeof(disp_value_t));
  for (i = 0; i < disp_value_items; i++) {
    s = conffile_get_table("show",i,0);
    disp_value[i].data.pt = plc_pt_by_name(s);
    free(s);
    s = conffile_get_table("show",i,1);
    disp_value[i].y = atoi(s);
    free(s);
    s = conffile_get_table("show",i,2);
    disp_value[i].x = atoi(s);
    free(s);
    s = conffile_get_table("show",i,3);
    disp_value[i].data.data_type = bool_dt;
    next_table_pos = 4;
    if (strcmp(s,  f32_TYPE) == 0)
      disp_value[i].data.data_type =  f32_dt;
    else if (strcmp(s,  i32_TYPE) == 0)
      disp_value[i].data.data_type =  i32_dt;
    else if (strcmp(s,  u32_TYPE) == 0)
      disp_value[i].data.data_type =  u32_dt;
    else if (strcmp(s,  i16_TYPE) == 0)
      disp_value[i].data.data_type =  i16_dt;
    else if (strcmp(s,  u16_TYPE) == 0)
      disp_value[i].data.data_type =  u16_dt;
    else if (strcmp(s,   i8_TYPE) == 0)
      disp_value[i].data.data_type =   i8_dt;
    else if (strcmp(s,   u8_TYPE) == 0)
      disp_value[i].data.data_type =   u8_dt;
    else if (strcmp(s, bool_TYPE) == 0)
      disp_value[i].data.data_type = bool_dt;
    else next_table_pos = 3;
    free(s);

    s = conffile_get_table("show",i,next_table_pos++);
    if (s) {
      disp_value[i].on = s[0];
      free(s);
    } else
      disp_value[i].on = '1';
    s = conffile_get_table("show",i,next_table_pos);
    if (s) {
      disp_value[i].off = s[0];
      free(s);
    } else
      disp_value[i].off = '0';
  }

  /* Format of the "graph" table, used to define a graph window      */
  /* graph graph_name min_x max_x min_x_val max_x_val min_y max_y min_y_val max_y_val */
  graph_win_items = conffile_get_table_rows("graph");
  graph_win = malloc(graph_win_items * sizeof(graph_win_t));
  for (i = 0; i < graph_win_items; i++) {
    error = 0;
    next_table_pos = 0;

    s = graph_win[i].name = conffile_get_table(GRAPH_TABLE,i,next_table_pos++);
    /* must not free(s) !! */
    error += conffile_get_table_u32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].min_x),
				    0, MAX_CURSES_WIN_X, 0);
    error += conffile_get_table_u32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].max_x),
				    0, MAX_CURSES_WIN_X, MAX_CURSES_WIN_X);
    error += conffile_get_table_f32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].min_x_val),
				    -f32_MAX, f32_MAX, DEF_MIN_X_VAL);
    error += conffile_get_table_f32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].max_x_val),
				    -f32_MAX, f32_MAX, DEF_MAX_X_VAL);
    error += conffile_get_table_u32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].min_y),
				    0, MAX_CURSES_WIN_Y, 0);
    error += conffile_get_table_u32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].max_y),
				    0, MAX_CURSES_WIN_Y, MAX_CURSES_WIN_Y);
    error += conffile_get_table_f32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].min_y_val),
				    -f32_MAX, f32_MAX, DEF_MIN_Y_VAL);
    error += conffile_get_table_f32(GRAPH_TABLE,i,next_table_pos++,
                                    &(graph_win[i].max_y_val),
				    -f32_MAX, f32_MAX, DEF_MAX_Y_VAL);
    s = conffile_get_table(GRAPH_TABLE,i,next_table_pos++);
    if (s) {
      graph_win[i].back_marker = s[0];
      free(s);
    } else
      graph_win[i].back_marker = ' ';
  }

  /* Format of the "plot" table, used to plot values in graphs       */
  /* plot graph_name x_plc_pt_name [x_data_format] y_plc_pt_name [y_data_format] [marker] */
  /* where:                                                          */
  /*   data_format: f32 | i32 | u32 | i16 | u16 | i8 | u8 | bool     */
  /*                defaults to i32                                  */
  /*   marker: character to use to plot value                        */
  /*                  defaults to '*'                                */
  disp_graph_items = conffile_get_table_rows(PLOT_TABLE);
  disp_graph = malloc(disp_graph_items * sizeof(disp_graph_t));
  for (i = 0; i < disp_graph_items; i++) {
    error = 0;
    next_table_pos = 0;

    s = conffile_get_table(PLOT_TABLE,i,next_table_pos++);
    disp_graph[i].graph = NULL;
    for (m = 0; m < graph_win_items; m++)
      if (strcmp(s, graph_win[m].name) == 0)
        disp_graph[i].graph = &(graph_win[m]);
    free(s);

    s = conffile_get_table(PLOT_TABLE,i,next_table_pos++);
    disp_graph[i].data_x.pt = plc_pt_by_name(s);
    free(s);
    error += get_value_type(PLOT_TABLE, i, &next_table_pos, &(disp_graph[i].data_x.data_type));

    s = conffile_get_table(PLOT_TABLE,i,next_table_pos++);
    disp_graph[i].data_y.pt = plc_pt_by_name(s);
    free(s);
    error += get_value_type(PLOT_TABLE, i, &next_table_pos, &(disp_graph[i].data_y.data_type));

    s = conffile_get_table(PLOT_TABLE,i,next_table_pos++);
    if (s) {
      disp_graph[i].marker = s[0];
      free(s);
    } else
      disp_graph[i].marker = '*';
  }
}



f32 pt_to_f32(plc_pt_t pt, data_type_t dt)
{
  u32 u32_tmp;

  u32_tmp = plc_get(pt);

  switch (dt) {
    case f32_dt: return *((f32 *)&u32_tmp); break;
    case  i8_dt: return *(( i8 *)&u32_tmp); break;
    case  u8_dt: return *(( u8 *)&u32_tmp); break;
    case i16_dt: return *((i16 *)&u32_tmp); break;
    case u16_dt: return *((u16 *)&u32_tmp); break;
    case i32_dt: return *((i32 *)&u32_tmp); break;
    case u32_dt: return *((u32 *)&u32_tmp); break;
    default : break;
  }; /* switch() */

  return 0;
}




void dump_config(void)
{
  plc_log_trcmsg(9, "%d value displays configured.", disp_value_items);
  plc_log_trcmsg(9, "%d plots configured.", disp_graph_items);
  plc_log_trcmsg(9, "%d graphs configured.", graph_win_items);
}


/*
 * Show everything. This'll get much more interesting when we get multi-bit
 * points and suddenly have to start showing integers and floats and what
 * not else.
 */
#define BUFF_SIZE 128
char str_buff[BUFF_SIZE];

void show(void)
{
  int i, x, y;
  f32 val, min_val, max_val;
  u32 u32_tmp;

  for (i = 0; i < disp_value_items; i++) {
    if (disp_value[i].data.data_type == bool_dt)
      mvaddch(disp_value[i].y, disp_value[i].x,
   	      plc_get(disp_value[i].data.pt) ? disp_value[i].on : disp_value[i].off);
    else {
      u32_tmp = plc_get(disp_value[i].data.pt);
      switch (disp_value[i].data.data_type) {
        case f32_dt: snprintf(str_buff, BUFF_SIZE, "%9f", *((f32 *)&u32_tmp)); break;
        case  i8_dt: snprintf(str_buff, BUFF_SIZE, "%d", *(( i8 *)&u32_tmp)); break;
        case  u8_dt: snprintf(str_buff, BUFF_SIZE, "%d", *(( u8 *)&u32_tmp)); break;
        case i16_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((i16 *)&u32_tmp)); break;
        case u16_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((u16 *)&u32_tmp)); break;
        case i32_dt: snprintf(str_buff, BUFF_SIZE, "%d", *((i32 *)&u32_tmp)); break;
        case u32_dt:
        default : break;
      }; /* switch() */
      mvaddstr(disp_value[i].y, disp_value[i].x, str_buff);
    }
  } /* for(;;) */

  for (i = 0; i < disp_graph_items; i++) {
    if ((disp_graph[i].data_x.pt.valid) &&
        (disp_graph[i].data_y.pt.valid) &&
        (disp_graph[i].graph != NULL  )) {
      mvaddch( disp_graph[i].old_y, disp_graph[i].old_x, graph_win[i].back_marker);
    }
  } /* for(;;) */

  for (i = 0; i < disp_graph_items; i++) {
    if ((disp_graph[i].data_x.pt.valid) &&
        (disp_graph[i].data_y.pt.valid) &&
        (disp_graph[i].graph != NULL  )) {
      val = pt_to_f32(disp_graph[i].data_x.pt, disp_graph[i].data_x.data_type);
      min_val = disp_graph[i].graph->min_x_val;
      max_val = disp_graph[i].graph->max_x_val;
      x = disp_graph[i].graph->min_x +
           (disp_graph[i].graph->max_x - disp_graph[i].graph->min_x) *
           (val - min_val) / (max_val - min_val);

      val = pt_to_f32(disp_graph[i].data_y.pt, disp_graph[i].data_y.data_type);
      min_val = disp_graph[i].graph->min_y_val;
      max_val = disp_graph[i].graph->max_y_val;
      y = disp_graph[i].graph->min_y +
           (disp_graph[i].graph->max_y - disp_graph[i].graph->min_y) *
           (val - min_val) / (max_val - min_val);

      if ((x >= disp_graph[i].graph->min_x) && (x <= disp_graph[i].graph->max_x) &&
          (y >= disp_graph[i].graph->min_y) && (y <= disp_graph[i].graph->max_y))  {
        disp_graph[i].old_x = x;
        disp_graph[i].old_y = y;
        mvaddch(y, x, disp_graph[i].marker);
      }
    }

  } /* for(;;) */

  move(24,0);
  refresh();
}

void initialize_screen(void)
{
  initscr();
  if (bkg) {
    FILE *f=fopen(bkg,"r");
    char *line=NULL;
    int line_len=0, row=0;
    while (getline(&line, &line_len, f) >= 0) {
      mvaddstr(row, 0, line);
      row++;
    }
    if (line)
      free(line);
    fclose(f);
  }
}


plc_pt_t	quit;


int init(){
//printf("vitrine init\n");

	quit = plc_pt_by_name("quit");
	if(!quit.valid) return ERROR;
	initscr();
	get_config();
	dump_config();
	initialize_screen();

	return NO_ERROR;
}

int step_run(){
//static int count = 0; printf("vitrine step %d\n", count); count++;

	if(plc_get(quit)) send_message("QUIT");
	show();
	return NO_ERROR;
}



#ifdef __cplusplus
}
#endif
