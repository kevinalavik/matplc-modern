/*
 * (c) 2001 Mario de Sousa
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
 * Daemon process utility routines
 *
 * This file implements the routines in daemon_util.h
 *
 *
 */


#include <sys/types.h>  /* for umask() */
#include <sys/stat.h>   /* for umask() */

#include "daemon_util.h"



 /* change the current process into a daemon */
int daemon_init(void) {

  pid_t pid;

   /* launch a new process and kill off the original */
   /* This guarantees that we are no longer a process group leader */
   if ((pid = fork()) < 0)
     /* error creating new process */
     return -1;

   if (pid != 0)
     /* kill off original process */
     exit(0);

   /* New process continues, and becomes the daemon... */

   setsid();    /* become session leader         */
   chdir("/");  /* Change working directory      */
   umask(0);    /* Clear file mode creation mask */

   return 0;
}
