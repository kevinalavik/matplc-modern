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
 * plctest utility.
 *
 * This is a very simple utility to exercise the plc
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <limits.h>

#include "plc.h"
#include <misc/string_util.h>


static const char *const DEF_MODULE_NAME = "test";

static int quiet = 0;


typedef enum {
        ft_int,
        ft_uint,
        ft_float
    } format_t;

typedef union {
        u32 u32;
        i32 i32;
        f32 f32;
    } value_t;

typedef struct {
        value_t  val;
        format_t format;
    } pt_val_t;


void termination_handler (int sig)
{
  struct sigaction new_action;

  /* must call plc_done() */
  plc_done();

  /* restore default handling */
  new_action.sa_handler = SIG_DFL;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  if (sigaction (sig, &new_action, NULL) < 0)
    exit (EXIT_FAILURE);

  /* re-raise signal */
  raise (sig);
}


static inline int register_handler(const int SIGNUM,
				    struct sigaction *new_action)
  /* register SIGNUM interrupt handler */
{
  struct sigaction query_action;
  if (sigaction(SIGNUM, NULL, &query_action) < 0)
    return -1;
  if (query_action.sa_handler != SIG_IGN)
    if (sigaction(SIGNUM, new_action, NULL) < 0)
      return -1;
  return 0;
}


int register_termination_handler(void)
{
  int res = 0;
  struct sigaction new_action;

  new_action.sa_handler = termination_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  /* register interrupt handler for SIGINT, SIGHUP and SIGTERM */
  res |= register_handler(SIGINT,&new_action);
  res |= register_handler(SIGTERM,&new_action);
  res |= register_handler(SIGHUP,&new_action);

  return res;
}


int plctest_dump(void)
{
  typedef struct {
    plc_pt_t handle;
    char     *name;
    } pt_t;

  int pt_index = 0;
  int numb_pts = 0;
  pt_t *pts;

  if ((numb_pts = plc_pt_count()) < 0)
    return -1;

  if ((pts = malloc(numb_pts * sizeof(pt_t))) == NULL)
    return -1;

  for (pt_index = 0; pt_index < numb_pts; pt_index++) {
    pts[pt_index].handle = plc_pt_by_index(pt_index,
                                           &(pts[pt_index].name),
                                           NULL);
    if (pts[pt_index].handle.valid == 0)
      return -1;
   }

  printf("\033[H\033[J"); /* clear screen (not needed any more) */

  while (1)
   {
    if (plc_update() < 0) return -1;

    printf("\033[H"); /* return cursor to top left position */

    for (pt_index = 0; pt_index < numb_pts; pt_index++) {
      printf("%12s: %X ", pts[pt_index].name, plc_get(pts[pt_index].handle));

      if (((pt_index + 1) % 4) == 0)
	printf("\033[K\n"); /* clear to eol and newline */
     }

   printf("\033[K\n\033[J"); /* clear to eol, newline, clear to eos */

   usleep(100000); /* 0.1 sleep */
   }

  return -1;  /* should never get here */
}

int plctest_dump_html(void)
{
  typedef struct {
    plc_pt_t handle;
    char *name;
  } pt_t;

  int pt_index = 0;
  int numb_pts = 0;
  pt_t *pts;

  if ((numb_pts = plc_pt_count()) < 0)
    return -1;

  if ((pts = malloc(numb_pts * sizeof(pt_t))) == NULL)
    return -1;

  for (pt_index = 0; pt_index < numb_pts; pt_index++) {
    pts[pt_index].handle = plc_pt_by_index(pt_index,
					   &(pts[pt_index].name), NULL);
    if (pts[pt_index].handle.valid == 0)
      return -1;
  }

  printf("<table border>\n");

  if (plc_update() < 0)
    return -1;

  for (pt_index = 0; pt_index < numb_pts; pt_index++) {
    printf("<tr><th>%s</th><td>%X</td></tr>\n",
	   pts[pt_index].name, plc_get(pts[pt_index].handle));
  }

  printf("</table>\n");

  return 0;
}

int plctest_dump_xml(void)
{
  typedef struct {
    plc_pt_t handle;
    char *name;
  } pt_t;

  int pt_index = 0;
  int numb_pts = 0;
  pt_t *pts;

  if ((numb_pts = plc_pt_count()) < 0)
    return -1;

  if ((pts = malloc(numb_pts * sizeof(pt_t))) == NULL)
    return -1;

  for (pt_index = 0; pt_index < numb_pts; pt_index++) {
    pts[pt_index].handle = plc_pt_by_index(pt_index,
					   &(pts[pt_index].name), NULL);
    if (pts[pt_index].handle.valid == 0)
      return -1;
  }

  printf("<MatPLC>\n");

  if (plc_update() < 0)
    return -1;

  for (pt_index = 0; pt_index < numb_pts; pt_index++) {
    printf("  <point name=\"%s\" value=\"%X\"/>\n",
	   pts[pt_index].name, plc_get(pts[pt_index].handle));
  }

  printf("</MatPLC>\n");

  return 0;
}


plc_pt_t get_handle(char *pt_name)
{
  plc_pt_t pt_handle;

  pt_handle = plc_pt_by_name(pt_name);

  if (pt_handle.valid == 0) {
    printf("Could not get valid handle to %s.\n", pt_name);
    exit(EXIT_FAILURE);
  };

  if (!quiet)
    printf("Got valid handle to %s - %d-bit %s point.\n", pt_name,
	 plc_pt_len(pt_handle), plc_pt_rw_s(pt_handle));

  return pt_handle;
}


void print_pt_val(char *pt_name, plc_pt_t pt_handle, format_t format) {
  if (!quiet)
    printf("%s = ",  pt_name);

  if (format == ft_int)
    printf("%d\n", plc_get(pt_handle));
  if (format == ft_uint)
    printf("%u\n", plc_get(pt_handle));
  if (format == ft_float)
    printf("%f\n", plc_get_f32(pt_handle));
}


int plctest_get(char *pt_name, format_t format)
{
  plc_pt_t pt_handle = get_handle(pt_name);
  plc_update();
  print_pt_val(pt_name, pt_handle, format);
  return 0;
}


int plctest_set(char *pt_name, pt_val_t pt_val)
{
  plc_pt_t pt_handle = get_handle(pt_name);

  switch (pt_val.format) {
    case ft_int:   plc_set    (pt_handle, pt_val.val.i32); break;
    case ft_uint:  plc_set    (pt_handle, pt_val.val.u32); break;
    case ft_float: plc_set_f32(pt_handle, pt_val.val.f32); break;
  }
  plc_update();

  if (!quiet) {
    if (plc_pt_rw(pt_handle) != 2)
      printf("Setting probably didn't work, %s is not read-write.\n",
	     pt_name);

    /* print the current point value... */
    print_pt_val(pt_name, pt_handle, pt_val.format);
  }
  return 0;
}

int plctest_until(char *pt_name)
{
  u32 pt_val;
  plc_pt_t pt_handle = get_handle(pt_name);

  do {
    plc_scan_beg();
    plc_update();

    pt_val = plc_get(pt_handle);

    /* plc_update(); --- not required, not setting anything */
    plc_scan_end();
  } while (!pt_val);

  return 0;
}




int plctest_wait(char *transition_name)
{
  plc_transition_t transition_handle;
  plc_synchpt_t synchpt;

  transition_handle = plc_transition_by_name(transition_name);

  if (!plc_transition_is_valid(transition_handle)) {
    printf("Could not get valid handle to %s.\n", transition_name);
    exit(EXIT_FAILURE);
  };

  synchpt = plc_synchpt_new(transition_handle, 0, plc_synchpt_blocking);

  if (!plc_synchpt_is_valid(synchpt)) {
    printf("Could not create new synchpt from transition %s.\n", transition_name);
    exit(EXIT_FAILURE);
  };

  if (!quiet)
    printf("Waiting on transition %s...\n", transition_name);
  plc_synch(synchpt);
  if (!quiet)
    printf("transition %s returned.\n", transition_name);

  return 0;
}


int plctest_scan_loop(void)
{
  int scan_count = 1;

  while(1) {
  if (!quiet)
    printf("Synching on scan %d begin...\n", scan_count);
  plc_scan_beg();
  if (!quiet)
    printf("Inside scan %d...\n", scan_count);
  if (!quiet)
    printf("Synching on scan %d end...\n", scan_count);
  plc_scan_end();
  scan_count++;
  }

  return 0;
}


void print_usage(char **argv)
{
  printf("usage: %s { [-f|i] -g <pt_name> | [-f|i] -s <pt_name> [-v <value>] | -d[h|x] | "
         "-w <synchpt_name>| -l | -u <pt_name>} [-q]\n",
         argv[0]);
  printf("       g: get value of point pt_name.\n");
  printf("       s: set value of point pt_name.\n");
  printf("       f: use values in floating point format .\n");
  printf("       i: use values in integer format .\n");
  printf("       u: use values in unsigned integer format .\n");
  printf("       v: value to store in point pt_name (defaults to 0).\n");
  printf("       d: dump the state of the plc points.\n");
  printf("       dh: dump the state of the plc points as HTML.\n");
  printf("       dx: dump the state of the plc points as XML.\n");
  printf("       w: wait on a synchpt synchpt_name.\n");
  printf("       l: run an empty scan loop.\n");
  printf("       h: run a scan loop until point pt_name is on.\n");
  printf("       q: quiet (print only errors and requested data)\n");

  plc_print_usage(stdout);
};



typedef enum {
        act_none,
        act_get,
        act_set,
        act_dump,
        act_dump_html,
        act_dump_xml,
        act_wait,
        act_loop,
        act_until
    } action_t;


int parse_arg(int argc,
              char **argv,
              action_t  *action,
              pt_val_t  *pt_val,
              char **pt_name,
              char **synchpt_name)
{
 int option, argc_iter, my_argc;
 char **my_argv;
 int value_parsed = 0;

 /* remove all long options from argv, otherwise getopt gets confused */
 my_argv = malloc (argc * sizeof (char *));
 if (my_argv == NULL) return -1;

 for (argc_iter = 0, my_argc = 0; argc_iter < argc; argc_iter++)
   if (strncmp(argv[argc_iter], CLO_LEADER, strlen(CLO_LEADER)) != 0)
      my_argv[my_argc++] = argv[argc_iter];


 while ((option = getopt(my_argc, my_argv, "iufg:s:v:d::lw:h:q")) != EOF) {
    /* Optional parameters (the double ':'), is a GNU extension not
     * available in QNX. This means that an error is returned if, for e.g., -g is given
     * without a parameter.
     * We work around this by catching that error and letting things carry forward...
     */
    if ((option == ':')  || (option == '?'))
      switch(optopt) {
        case 'd':
          option = optopt;
          optarg = NULL;
          break;
        default:  
          break;
      }  /* switch(optopt); */
    switch (option) {
       case '?':
       case ':': {print_usage(argv); free (my_argv); return -1;}; /* ? */
       case 'l': {*action = act_loop; break;}
       case 'w': {*action = act_wait;  *synchpt_name = optarg; break;}
       case 's': {*action = act_set;   *pt_name = optarg; break;}
       case 'v': {/* assume success... */
                  value_parsed = 1;
                  if (pt_val->format == ft_int)
                    if (string_str_to_i32(optarg, &(pt_val->val.i32), i32_MIN, i32_MAX)
                        >= 0)
                      break;
                  if (pt_val->format == ft_uint)
                    if (string_str_to_u32(optarg, &(pt_val->val.u32), i32_MIN, i32_MAX)
                        >= 0)
                      break;
                  if (pt_val->format == ft_float)
                    if (string_str_to_f32(optarg, &(pt_val->val.f32), -f32_MAX, f32_MAX)
                        >= 0)
                      break;
                  /* error... */
		  printf("Cannot convert %s to a number.\n", optarg);
                  value_parsed = 0;
                  goto exit_error;
                 }
       case 'g': {*action = act_get;   *pt_name = optarg; break;}
       case 'h': {*action = act_until;   *pt_name = optarg; break;}
       case 'd': {
		  if (!optarg)
		    *action = act_dump; 
		  else if (!strcmp(optarg,"h"))
		    *action = act_dump_html; 
		  else if (!strcmp(optarg,"x"))
		    *action = act_dump_xml; 
		  else
		    goto exit_error;
		  break;
		 }
       case 'q': {quiet = 1; break;}
       case 'f': {if (value_parsed != 0)     goto exit_error;
                  pt_val->format = ft_float; break;
                 }
       case 'i': {if (value_parsed != 0)     goto exit_error;
                  pt_val->format = ft_int;   break;
                 }
       case 'u': {if (value_parsed != 0)     goto exit_error;
                  pt_val->format = ft_uint;   break;
                 }
       default:  goto exit_error;
       } /* switch */
    } /* while(getopt()) */
 free (my_argv);
 return 0;

 exit_error:
 printf("error processing parameters...\n");
 free (my_argv);
 return -1;
};




int main(int argc, char **argv)
{
  action_t action = act_none;
  char *pt_name;
  char *synchpt_name;
  pt_val_t pt_val;

  action = act_none;
  pt_val.format = ft_int;

  if (parse_arg(argc, argv, &action, &pt_val, &pt_name, &synchpt_name) < 0)
    return -1;
  if (action == act_none) {
    print_usage(argv);
    return -1;
  };

  if (register_termination_handler() < 0)
    return -1;

  if (plc_init(DEF_MODULE_NAME, argc, argv) < 0) {
    printf("Error initializing PLC\n");
    return -1;
  }

  switch (action) {
  case act_get:
    return plctest_get(pt_name, pt_val.format);
    break;
  case act_set:
    return plctest_set(pt_name, pt_val);
    break;
  case act_until:
    return plctest_until(pt_name);
    break;
  case act_dump:
    return plctest_dump();
    break;
  case act_dump_html:
    return plctest_dump_html();
    break;
  case act_dump_xml:
    return plctest_dump_xml();
    break;
  case act_wait:
    return plctest_wait(synchpt_name);
    break;
  case act_loop:
    return plctest_scan_loop();
    break;
  default:
    print_usage(argv);
    return -1;
  };				/* switch */


  return 0;
}
