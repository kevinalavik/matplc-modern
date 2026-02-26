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
 * plcshutdown utility.
 *
 * This is a very simple module used to shutdown the plc
 * when a certain plc point is set.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> /* required for strcmp() */

#include "plc.h"
#include "plc_setup.h"


/***********************************************************/
/*****************                          ****************/
/*****************      Default Values      ****************/
/*****************                          ****************/
/***********************************************************/


 /* default module name */
static const char *const DEF_MODULE_NAME = "plcshutdown";

/***********************************************/
/* name in name=value pairs of the config file */
/***********************************************/
 /* quit point name */
static const char *const QUIT_PT_NAME = "quit_pt";

/************************/
/* command line options */
/************************/
static const char *const NOW_CMD_LINE_ARG = "now";






/***********************************************************/
/*****************                          ****************/
/*****************       Program Code       ****************/
/*****************                          ****************/
/***********************************************************/

static int scan_loop(plc_pt_t quit_pt)
{
  if (quit_pt.valid == 0)
    return -1;

  for (;;) {
    plc_scan_beg();
    plc_update();

    if (plc_get(quit_pt) != 0) {
        /* plc_shutdown() will initialise acces to the plc,
         * i.e. call plc_init(), which is why it needs the (argc, argv)
         * parameters.
         * In this case the plc has already been initialised, so
         * the (argc, argv) are irrelevant. We simply pass
         * no arguments.
         */
      plc_shutdown(0, NULL);
        /* plc_done() not required.  plc_shutdown() also calls plc_done() */
      /* plc_done(); */
      return 0;
    }

    /* plc_update(); --- not required, not setting anything */
    plc_scan_end();
  };

  /* Humour the compiler...         */
  /* will never reach this point... */
  return -1;
}




static int print_usage(char **argv)
{
  printf("%1$s [now] [--PLCxxx]\n"
         " %1$s now: shutdown the plc imediately \n"
         " %1$s    : shutdown the plc when a plc point is set.\n"
         "         The plc point must be configured in the configuration file.\n",
         argv[0]
        );
  plc_print_usage(stdout);

  return 0;
}




static int parse_args(int argc, char **argv, int *now)
{
  int local_now;
  int arg_count;

  if (now == NULL)
    now = &local_now;

  *now = 0;

  for (arg_count = 1; arg_count < argc; arg_count++)
    if (strcmp(argv[arg_count], NOW_CMD_LINE_ARG) == 0)
      *now = 1;

  return 0;
}




static int parse_conf(plc_pt_t *quit_pt)
{
 char     *quit_pt_name;
 plc_pt_t local_pt;
 int      result = 0;

 if (quit_pt == NULL)
   quit_pt = &local_pt;

 if ((quit_pt_name = conffile_get_value(QUIT_PT_NAME)) != NULL)
   if ((*quit_pt = plc_pt_by_name(quit_pt_name)).valid == 0) {
     plc_log_errmsg(1, "Invalid plc point %s.", quit_pt_name);
     result = -1;
   }
 free(quit_pt_name); /* it is safe to free a NULL pointer. */

 return result;
};




int main(int argc, char **argv)
{
  int      now = 0;
  plc_pt_t quit_pt;

  if (parse_args(argc, argv, &now) < 0) {
    printf("Invalid parameters.\n");
    print_usage(argv);
    return -1;
  }

  if (now != 0) {
    /* shutdown the plc imediately... */
    /* We do not even parse the config file, because we do not
     * want to bail out if an error occurs in config file parsing...
     */
    return plc_shutdown(argc, argv);
  }

  if (plc_init(DEF_MODULE_NAME, argc, argv) < 0) {
    printf("Error initializing PLC. Is the PLC running?\n");
    return -1;
  }

  quit_pt.valid = 0;

  if (parse_conf(&quit_pt) < 0) {
    plc_log_errmsg(1, "Error processing configuration parameters. "
                      "Bailing out!");
    plc_done();
    return -1;
  };

  if (quit_pt.valid == 0) {
    /* no quit point specified */
    plc_log_errmsg(1, "No quit point specified. "
                      "Bailing out!");
    print_usage(argv);
    plc_done();
    return -1;
  }

  return scan_loop(quit_pt);
}
