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
 * PLC Manager - setup and shutdown functions
 *
 * This file implements the routines in plc_setup.h
 *
 * In general, it should only be called by smm-mgr.c or other code closely
 * related to the PLC.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <errno.h>
#include <limits.h>


#include <misc/string_util.h>

#include "log/log_setup.h"
#include "conffile/conffile_setup.h"
#include "cmm/cmm_setup.h"
#include "gmm/gmm_setup.h"
#include "rt/rt_setup.h"
#include "synch/synch_setup.h"
#include "period/period_setup.h"
#include "state/state_setup.h"


#include "plc_private.h"
#include "plc.h"

static const int debug = 0;


/************************************************************/
/*****************                          *****************/
/*****************      Local auxiliary     *****************/
/*****************         functions        *****************/
/*****************                          *****************/
/************************************************************/

/* helper function for plc_parse_conf() */
static void state_parse_conf_i32(const char *section, const char *name,
                                 i32 *var, i32 min, i32 max, i32 def,
                                 int use_default)
{
  int res = 0;

  if (!use_default)
    def = *var;

  if (section == NULL)
    res = conffile_get_value_i32(name, var, min, max, def);
  else
    res = conffile_get_value_sec_i32(section, name, var, min, max, def);

  if ((res < 0) && (use_default)) {
    plc_log_wrnmsg(1, "cannot understand %s %s,"
                   " or value out of bounds [%d...%d]. Using default %d.\n",
                   name, conffile_get_value(name),
                   min, max, *var);
  }
}


/* parse the config file for global PLC related values */
void plc_parse_conf(int *module_max_count,
                     /*
                      * flag indicating if we should return the default values
                      * if the parameter has been left unspecified in the
                      * config file.
                      * 1 - yes, init to the default value if left unspecified
                      * 0 - no, leave parm. value unchanged if left unspecified
                      */
                     int use_default)
{
  if (module_max_count != NULL)
    state_parse_conf_i32(PLC_MAXMODULECOUNT_SECTION, PLC_MAXMODULECOUNT_NAME,
                         module_max_count,
                         PLC_MAXMODULECOUNT_MIN, PLC_MAXMODULECOUNT_MAX,
                         PLC_MAXMODULECOUNT_DEF,
                         use_default);
}


  /* stop all the running modules... */
static int plc_stop_modules(void) {
  return plc_module_shutdown_all();
}

  /* launch the modules specified in the config file... */
  /*
   * Accepted syntax for config file:
   *
   * module <module_name> <execution_file> [<parameter>] ...
   *
   *
   * NOTE: We need to know which conffile is being used, so the
   *       launched modules will use the same one. We currently
   *       do this by expecting to be passed the config file name,
   *       but it would probably be cleaner to get the info directly
   *       from the conffile library. We will need to add it to the
   *       conffile library's public interface though...
   */
static int plc_launch_modules(
        const char *conffile, 
        int plcid,
        const char *log_file
        )
{
  int error_launch_flag;
  int modules_count, max_modules, modules_launched;
  int module_argc, next_argc, argc_count;
  pid_t pid;
  char *exec_command = NULL;
  char *module_name  = NULL;
  char **module_argv = NULL;


  max_modules = conffile_get_table_rows_sec(START_MODULES_TABLE_SEC,
                                            START_MODULES_TABLE_NAME);

  for (modules_count = 0, modules_launched = 0;
       modules_count < max_modules;
       modules_count++) {

    /* assume error */
    error_launch_flag = -1;
    module_name = conffile_get_table_sec(START_MODULES_TABLE_SEC,
                                         START_MODULES_TABLE_NAME,
                                         modules_count, 0);
    if (module_name == NULL)
      continue;

    exec_command = conffile_get_table_sec(START_MODULES_TABLE_SEC,
                                          START_MODULES_TABLE_NAME,
                                          modules_count, 1);
    if (exec_command == NULL) {
      plc_log_errmsg(1,
                     "No command was specified to launch module %s. "
                     "Skipping this module",
                     module_name);
      goto error_launch_0;
    }

    /* get the number of argv[] arguments... */
    module_argc = conffile_get_table_rowlen_sec(START_MODULES_TABLE_SEC,
                                                START_MODULES_TABLE_NAME,
                                                modules_count);

      /* The number of arguments we copy directly from the user.
       * Note that <module_name> and <execution_file> do not count here
       * so we need to subtract 2.
       */
    module_argc -= 2;
    next_argc = module_argc;

      /* we need an extra 6.
       *  - for the <execution_command> which gets passed in argv[0]
       *  - for the <module_name> which gets passed in argv[module_argc+1]
       *        as --PLCmodule=<module_name>
       *  - for the <conffile> which gets passed in argv[module_argc+2]
       *        as --PLCconf=<conffile>
       *  - for the <log_file> which gets passed in argv[module_argc+3]
       *        as --PLClogfile=<log_file>
       *  - for the plc ID which gets passed in argv[module_argc+4]
       *        as --PLCplc_id=<ID>
       *  - for the NULL pointer at the end...
       */
    module_argc += 6;
    module_argv = malloc((module_argc) * sizeof(char *));
    if (module_argv == NULL) {
      plc_log_errmsg(1,
                     "Not enough memory to create argv list for module %s. "
                     "Skipping this module",
                     module_name);
      goto error_launch_1;
    }
    /* to be on the safe side, we init every pointer to NULL
     * because of the cleanup code
     */
    for (argc_count = 0; argc_count < module_argc; argc_count++)
      module_argv[argc_count] = NULL;

    /* Copy the arguments... */
    /* Note: we copy the name of the command onto argv[0],
     *       so we start parsing at column numb. 1, and go on for
     *       an extra argument!
     */
    for (argc_count = 0; argc_count < next_argc + 1; argc_count++) {

      module_argv[argc_count] =
        conffile_get_table_sec(START_MODULES_TABLE_SEC,
                               START_MODULES_TABLE_NAME,
                               modules_count,
                               argc_count + 1);
      if (module_argv[argc_count] == NULL) {
        plc_log_errmsg(1,
                       "Error reading argv list for module %s. "
                       "Skipping this module",
                       module_name);
        goto error_launch_2;
      }
    }

    /* We now add --PLCmodule=<module_name> as the next argument */
    next_argc++;
    module_argv[next_argc] = strdup2 (CLO_module_name, module_name);
    if (module_argv[next_argc] == NULL) {
      plc_log_errmsg(1,
                     "Error creating argv list for module %s. "
                     "Skipping this module",
                     module_name);
      goto error_launch_2;
    }

    /* We now add --PLCconf=<conffile> as the next argument */
    if (conffile != NULL) {
      next_argc++;
      module_argv[next_argc] = strdup2 (CLO_config_file, conffile);
      if (module_argv[next_argc] == NULL) {
        plc_log_errmsg(1,
                       "Error creating argv list for module %s. "
                       "Skipping this module",
                       module_name);
        goto error_launch_2;
      }
    }

    /* We now add --PLClogfile=<log_file> as the next argument */
    if (log_file != NULL) {
      next_argc++;
      module_argv[next_argc] = strdup2 (CLO_log_file, log_file);
      if (module_argv[next_argc] == NULL) {
        plc_log_errmsg(1,
                       "Error creating argv list for module %s. "
                       "Skipping this module",
                       module_name);
        goto error_launch_2;
      }
    }

    /* We now add --PLCplc_id=<ID> as the last argument */
    if (plcid >= 0) {
      int str_size;
      char *plcid_str;

       /* the following function will return the number of characters
        * required to write the number into the string.
        * NOTE: on older versions of glibc, this will return -1, so we abort!
        */
      plcid_str = NULL;
      str_size = snprintf(plcid_str, 0, "%d", plcid);
      if (str_size <= 0)
        goto error_launch_2;

      str_size += 1; /* for the '\0' */
      plcid_str = (char *)malloc(str_size);
      if (plcid_str == NULL)
        goto error_launch_2;

      str_size = snprintf(plcid_str, str_size, "%d", plcid);
      if (str_size <= 0) {
        free(plcid_str);
        goto error_launch_2;
      }

      next_argc++;
      module_argv[next_argc] = strdup2 (CLO_plc_id, plcid_str);
      free(plcid_str);
      if (module_argv[next_argc] == NULL) {
        plc_log_errmsg(1,
                       "Error creating argv list for module %s. "
                       "Skipping this module",
                       module_name);
        goto error_launch_2;
      }
    }

    /* The last is a NULL pointer... */
    next_argc++;
    module_argv[next_argc] = NULL;

    pid = fork();

    if (pid < 0) {
      plc_log_errmsg(1, "Could not create process to run %s.", module_name);
      goto error_launch_2;
    }

    if (pid == 0) {
      /* child process */
      /* In principle, we call plc_done() in order to close down access to open files,
       * shared memory, and semaphores.
       * Note that files remain open, and shared memory remains attached, after
       * a fork. Nevertheless, after a call to exec(), access to shared memory
       * is closed, and any open files are closed if fcntl()'s FD_CLOEXEC flag is set
       * (which seems to be set by default - have to confirm this!!).
       * I (Mario) don't know what happens to POSIX semaphores that have been
        * sem_open()'d though... Note that SysV semaphores do not have to be closed()!!
       * Since I don't know what happens to the POSIX semaphores, it would probably be 
       *  prudent to call plc_done() here to make sure everything is closed down before 
       * calling exec(). Note that this means that plc_done may be called multiple times 
       * after a single call to plc_init(). Once for the parent process, and once for each 
       * child process.
       * Unfortunately, cmm_block_free() may not be called multiple times to free the 
       * same memory block. This may happen if a MatPLC subsystem (gmm, synch, etc...)
       * allocates a cmm_block during xxx_init(), and frees it during xxx_done().
       * Currently, the SysV semaphore emulation based on POSIX sempahores, which
       * are used by the synch sub-system, does the above, which is why I commented
       * out the call to plc_open().
       * Hopefully, everything will work out all right. If we find that POSIX semaphores
       * remain open after an exec() call, then I will probably have to change the SysV
       * semaphore emulation so that it does not use a cmm_block for each MatPLC
       * module, and insert the call to plc_done() once again...
       *
       * Note that not calling plc_done() also has the advantage of not having multiple
       * 'file close' notices in the logging files.
       */
      /*plc_done();*/  

      /* now load and execute the module binary file... */
      if (execvp(exec_command, module_argv) < 0) {
        perror("Error executing command.");
        exit(EXIT_FAILURE);
      } else {
        /* the execvp() function is guranteed to never return success,
         * so this code NEVER gets run!
         */
        /*
        exit(EXIT_SUCCESS);
        */
      }
    } else {
      // parent process //
      plc_log_trcmsg(2, "Launched %s with pid %d", module_name, pid);
    }

    modules_launched++;

    /* everything went through OK */
    error_launch_flag = 0;

    /* fall through for cleanup code */

   error_launch_2:
    for (argc_count =  0; argc_count < module_argc; argc_count++) {
      free(module_argv[argc_count]);
    }
    free(module_argv);
  error_launch_1:
    free(exec_command);
    exec_command = NULL;
  error_launch_0:
    free(module_name);
    module_name = NULL;

    if (error_launch_flag < 0)
      goto error_exit_0;
    /*
    else
      continue;
    */
  } /* for (module_count...) */

  plc_log_trcmsg(1, "Launched %d modules.", modules_launched);
  return 0;

error_exit_0:
  plc_stop_modules();
  return -1;
}





/************************************************************/
/*****************                          *****************/
/*****************    Things declared in    *****************/
/*****************       plc_setup.h        *****************/
/*****************                          *****************/
/************************************************************/



int plc_setup(int argc, char **argv, const char *conffile_prm1)
/* returns 0 if succesful   */
/* returns -1 on error      */
{
  int plc_id, log_level, max_module_count;
  const char *conffile;
  const char *conffile_prm2;
  const char *log_file;

  if (debug) printf("plc_setup():...\n");

  log_level = DEF_LOG_LEVEL;
  log_file = DEF_LOG_FILE;
  conffile_prm2 = NULL;
  plc_id = -1;

  plc_parse_args(argc, argv,
                 NULL,       /* not interested in location      */
                 NULL,       /* not interested in local shm key */
                 &plc_id,
                 &conffile_prm2,
                 NULL,       /* not interested in module_name   */
                 &log_level,
                 &log_file,
                 NULL,       /* not interested in force-init    */
                 0           /* do not remove parsed args...    */
                );

  if ((conffile_prm1 != NULL) && (conffile_prm2 != NULL))
    if (strcmp(conffile_prm1, conffile_prm2) == 0)
      /* we were told to use two different config files! */
      /* Give up! */
      return -1;

  /* Use whichever parameter is != NULL, or default if none! */
  conffile = DEF_CONFIG_FILE;
  if (conffile_prm1 != NULL)
    conffile = conffile_prm1;
  if (conffile_prm2 != NULL)
    conffile = conffile_prm1;

  if (plc_log_setup(log_level,
                    "plc_setup",
                    log_file,
                    log_file,
                    log_file)
      < 0) {
    printf("Error setting up the logging service...\n");
    return -1;
  }
  plc_log_trcmsg(9, "Succesfully started the logging service.\n");

  if (conffile_setup(conffile, "") < 0) {
    plc_log_errmsg(1, "Error setting up the config file reading service...\n");
    goto error_exit_0;
  }
  plc_log_trcmsg(9, "Succesfully started the config file reading service.\n");

  /* parse config file for global PLC related values... */
  plc_parse_conf(&max_module_count, 1 /* yes, use defaults... */);

  if (cmm_setup(plc_id) < 0) {
    plc_log_errmsg(1, "Error setting up the configuration memory manager ...\n");
    goto error_exit_1;
  }
  plc_log_trcmsg(9, "Succesfully started the configuration memory manager.\n");

  if (gmm_setup("") < 0) {
    plc_log_errmsg(1, "Error setting up the global memory manager ...\n");
    goto error_exit_2;
  }
  plc_log_trcmsg(9, "Succesfully started the global memory manager.\n");
  
  if (rt_setup("") < 0) {
    plc_log_errmsg(1, "Error setting up the real-time manager...\n");
    goto error_exit_3;
  }
  plc_log_trcmsg(9, "Succesfully started the real-time manager.\n");

  if (synch_setup("", max_module_count) < 0) {
    plc_log_errmsg(1, "Error setting up the synchronisation manager ...\n");
    goto error_exit_4;
  }
  plc_log_trcmsg(9, "Succesfully started the synchronisation manager.\n");

  if (period_setup("") < 0) {
    plc_log_errmsg(1, "Error setting up the period manager ...\n");
    goto error_exit_5;
  }
  plc_log_trcmsg(9, "Succesfully started the period manager.\n");

  if (state_setup("") < 0) {
    plc_log_errmsg(1, "Error setting up the state manager ...\n");
    goto error_exit_6;
  }
  plc_log_trcmsg(9, "Succesfully started the state manager.\n");


    /* Now that the PLC is fully setup, we launch the modules... */
  if (plc_launch_modules(conffile, plc_id, log_file) < 0) {
    plc_log_errmsg(1, "Error launching the modules ...\n");
    goto error_exit_7;
  }
  plc_log_trcmsg(9, "Succesfully started all the modules.\n");

  /* place the PLC in RUN mode... */
  plc_run();

  state_done();
  period_done();
  synch_done();
  rt_done();
  gmm_done();
  cmm_done();
  conffile_done();
  plc_log_done();

  return 0;

/*
error_exit_8:
  plc_stop_modules();
*/
error_exit_7:
  state_shutdown();

error_exit_6:
  period_shutdown();

error_exit_5:
  synch_shutdown();

error_exit_4:
  rt_shutdown();

error_exit_3:
  gmm_shutdown();

error_exit_2:
  cmm_shutdown();

error_exit_1:
  conffile_shutdown();

error_exit_0:
  plc_log_shutdown();

  return -1;
}


int plc_shutdown(int argc, char **argv)
/* returns 0 if succesful   */
/* returns -1 on error      */
{
  int res = 0;

  res |= plc_init_try("plc_shutdown", argc, argv);

  res |= plc_stop_modules();

  res |= state_shutdown();
  res |= period_shutdown();
  res |= synch_shutdown();
  res |= rt_shutdown();
  res |= gmm_shutdown();
  res |= cmm_shutdown();
  res |= conffile_shutdown();
  res |= plc_log_shutdown();

  res |= plc_done();

  if (res != 0) res = -1;
  return res;
}


