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


/*
 * matplc utility
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plc.h>
#include <plc_setup.h>
#include <plc_private.h> /* required for DEF_CONFIG_FILE */

static const int debug = 0;


/********************************************/
/*                                          */
/* The functions that do the actual work... */
/*                                          */
/********************************************/

static int setup(int argc, char **argv, const char *conffile)
{
  if (plc_setup(argc, argv, conffile) < 0) {
    printf ("Error initializing the plc.\n");
    return -1;
  }

  return 0;
}


static int shutdown(int argc, char **argv)
{
  int res;

  res = plc_shutdown(argc, argv);

  if (res < 0)
    printf ("Error closing down the plc.\n");

  return res;
}




/* small auxiliary function to dump() */
static const char *mode2str(int mode) {
  if (mode == 1)
    return "RUN";
  if (mode == 0)
    return "STOP";
  return "?";
}


static int dump(int argc, char **argv)
{
  int index = 0;
  char *name = NULL;

  if (plc_init("SMM-MGR", argc, argv) != 0) {
    printf("MatPLC not running or not found.\n");
    return -1;
  }

  /* print points */
  {
    int length = 0;
    int bit = 0;
    int offset = 0;
    char *owner = NULL;
    plc_pt_t pt_handle;

    for (index = 0;
         (pt_handle = plc_pt_by_index(index, &name, &owner)).valid != 0;
         index++) {
      plc_pt_details(pt_handle, NULL, &length, &offset, &bit, NULL);
      printf("point name=%s, owner=%s, offset=%d, bit=%d, length=%d, ",
             name, owner, offset, bit, length);
      pt_handle = plc_pt_by_name(name);
      if (pt_handle.valid != 0) {
        printf("state=%d", plc_get(pt_handle));
      }
      printf("\n");
    } /* for(;;) */
  }

  /* print transitions */
  {
    plc_transition_t transition_handle;
    for (index = 0;
         plc_transition_is_valid(transition_handle = plc_transition_by_index(index, &name));
         index++) {
      int place_count;
      plc_transition_swvalue_t *wait_values;
      plc_transition_swvalue_t *signal_values;

      printf("transition name=%s\n", name);

      if (plc_transition_details(transition_handle,
                                 &place_count,
                                 &wait_values,
                                 &signal_values)
          >= 0) {
        int place_index;

        printf(" wait values: ");
        for (place_index = 0; place_index < place_count; place_index++)
          printf(" %d", wait_values[place_index]);
        printf("\n");

        printf(" signal values: ");
        for (place_index = 0; place_index < place_count; place_index++)
          printf(" %d", signal_values[place_index]);
        printf("\n");
      }
    } /* for(;;) */
  }

  /* print module details... */
 {
    pid_t pid;     /* the module's pid                     */
    plc_synchplace_t place;
    int   active; /* does the process seem to be running ? */
    int   mode;   /* current RUN / STOP mode               */
    int   module_count = plc_module_count();

    for (index = 0; index < module_count; index++) {
      if (plc_module_by_index(index, &name) >= 0)
        if(plc_module_details(name, &pid, &place, &mode, &active) >= 0)
          printf("module name=%s, pid=%d, run/stop place=%d, current mode=%s, active=%d\n",
                 name, pid, place, mode2str(mode), active);
    } /* for(;;) */
  }

  return 0;
}


static int check_conffile(const char *conffile)
{
  if (conffile == NULL)
    conffile = DEF_CONFIG_FILE; /* !! Hardcoded matplc.conf !! */

  conffile_init(conffile,""); /* module name is indiferent */
  conffile_dump();
  conffile_done();
  return 0;
}



static int module_run(int argc, char **argv, const char *module_name) {

  int res = 0;

  if (plc_init("SMM-MGR", argc, argv) != 0) {
    printf("MatPLC not running or not found.\n");
    return -1;
  }

  if (module_name != NULL) {
    if (plc_module_run(module_name) < 0) {
      printf("Error placing %s in RUN mode.\n", module_name);
      res = -1;
    }
  } else {
    /* module_name == NULL */
    if (plc_run() < 0) {
      printf("Error placing PLC in RUN mode.\n");
      res = -1;
    }
  }

  plc_done();

  return res;
}



static int module_stop(int argc, char **argv, const char *module_name) {

  int res = 0;

  if (plc_init("SMM-MGR", argc, argv) != 0) {
    printf("MatPLC not running or not found.\n");
    return -1;
  }

  if (module_name != NULL) {
    if (plc_module_stop(module_name) < 0) {
      printf("Error placing %s in STOP mode.\n", module_name);
      res = -1;
    }
  } else {
    /* module_name == NULL */
    if (plc_stop() < 0) {
      printf("Error placing PLC in STOP mode.\n");
      res = -1;
    }
  }

  plc_done();

  return res;
}



/********************************************/
/*                                          */
/* Parsing command line parameters...       */
/*                                          */
/********************************************/

void print_usage(char **argv)
{
  printf("usage: %s %s",argv[0],
    " {{-{c|g} [<file_name>]} | {-{r|h} [<module_name>]} | -{s|d}} [PLC options]\n"
    "          c: Check matplc config file syntax."
                 " fname defaults to matplc.conf\n"
    "          g: Go. Start the MatPLC."
                 " fname defaults to matplc.conf\n"
    "          s: Shutdown the MatPLC.\n"
    "          r: Place the MatPLC, or the <module> if specified, in RUN mode.\n"
    "          h: Place the MatPLC, or the <module> if specified, in STOP mode.\n"
    "          d: Dump the MatPLC configuration map.\n"
    "PLC options:\n");
  plc_print_usage(stdout);
}


typedef enum {
  act_none,
  act_start,
  act_stop,
  act_dump,
  act_check_conffile,
  act_module_run,
  act_module_stop
} action_t;


int parse_arg(int argc, char **argv, action_t * action,
              const char **conffile, const char **module_name)
{
  int option, argc_iter, my_argc;
  char **my_argv;

  /* remove all long options from argv, otherwise getopt gets confused */
  my_argv = malloc(argc * sizeof(char *));
  if (my_argv == NULL)
    return -1;

  for (argc_iter = 0, my_argc = 0; argc_iter < argc; argc_iter++)
    if (strncmp(argv[argc_iter], CLO_LEADER, strlen(CLO_LEADER)) != 0)
      my_argv[my_argc++] = argv[argc_iter];

  *action = act_none;
  *conffile = NULL;

  opterr = 0;
  while ((option = getopt(my_argc, my_argv, "-g::lsdc::r::h::")) != EOF) {
    /* Optional parameters (the double ':'), is a GNU extension not
     * available in QNX. This means that an error is returned if, for e.g., -g is given
     * without a parameter.
     * We work around this by catching that error and letting things carry forward...
     */
    if ((option == ':')  || (option == '?'))
      switch(optopt) {
        case 'g':
        case 'c':
        case 'r':
        case 'h':
          option = optopt;
          optarg = NULL;
          break;
        default:  
          break; /* humour the compiler... */
      }  /* switch(optopt); */
    switch (option) {
    case '?':
    case ':':
      goto error_exit;
      break;
    case  1 :
      /* if the previous option was an option that requires a conffile parameter */
      if ((*action == act_start) || (*action == act_check_conffile)) {
        /* and the conffile parameter has not yet been specified... */
        if (conffile != NULL)
          if (*conffile == NULL)
            *conffile = optarg;
        break;
      }
      /* if the previous option was an option that requires a module_name parameter */
      if ((*action == act_module_run) || (*action == act_module_stop)) {
        /* and the module_name parameter has not yet been specified... */
        if (module_name != NULL)
          if (*module_name == NULL)
          *module_name = optarg;
        break;
      }
      /* otherwise, this is an error... */
      goto error_exit;
      break;
    case 'r':
      if (*action != act_none)
        goto error_exit;
      *action = act_module_run;
      if (optarg == NULL)
	break;
      if (module_name != NULL)
        *module_name = optarg;
      break;
    case 'h':
      if (*action != act_none)
        goto error_exit;
      *action = act_module_stop;
      if (optarg == NULL)
	break;
      if (module_name != NULL)
        *module_name = optarg;
      break;
    case 'g':
      if (*action != act_none)
        goto error_exit;
      *action = act_start;
      if (optarg == NULL)
        break;
      if (conffile != NULL)
        *conffile = optarg;
      break;
    case 'c':
      if (*action != act_none)
        goto error_exit;
      *action = act_check_conffile;
      if (optarg == NULL)
	break;
      if (conffile != NULL)
        *conffile = optarg;
      break;
    case 's':
      if (*action != act_none)
        goto error_exit;
      *action = act_stop;
      break;
    case 'd':
      if (*action != act_none)
        goto error_exit;
      *action = act_dump;
      break;
    default:
      printf("error processing parameters...\n");
      goto error_exit;
      break;
    }				/* switch() */
  } /* while(getopt()) */
  free(my_argv);
  return 0;

error_exit:
  print_usage(argv);
  free(my_argv);
  return -1;
}



/********************************************/
/*                                          */
/* The main program...                      */
/*                                          */
/********************************************/

int main(int argc, char **argv)
{
  action_t action = act_none;
  const char *conffile = NULL;
  const char *module_name = NULL;

  if (parse_arg(argc, argv, &action, &conffile, &module_name) < 0)
    return -1;

  switch (action) {
    case act_start          : setup(argc, argv, conffile);          break;
    case act_stop           : shutdown(argc, argv);                 break;
    case act_dump           : dump(argc, argv);                     break;
    case act_check_conffile : check_conffile(conffile);             break;
    case act_module_run     : module_run(argc, argv, module_name);  break;
    case act_module_stop    : module_stop(argc, argv, module_name); break;
    default                 : print_usage(argv); return -1;
  } /* switch */

  return 0;
}
