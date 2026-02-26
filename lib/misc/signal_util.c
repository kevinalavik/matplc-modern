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
 * Signal utility routines
 *
 * This file implements the routines in signal_util.h
 *
 * Apart from the above functions, this file should include
 * routines to provide a POSIX compatible interface to
 * signals, in case we are compiling under a system that does not provide them.
 *
 * At the moment, we are only compiling under systems that do support them, so
 * those functions are un-necessary.
 *
 */


#include <stdlib.h>

#include "signal_util.h"


int signal_action(int signum,
                  struct sigaction *new_action,
                  struct sigaction *old_action) {
  return sigaction(signum, new_action, old_action);
};


int signal_sethandler(int signum, signal_handler_t sighandler,
                      struct sigaction *old_action) {

  struct sigaction action;

     /* setup signal handler... */
  action.sa_handler = sighandler;
     /* do not block any signals while running the signal handler... */
  sigemptyset(&action.sa_mask);
     /* ...except signum itself, ofcourse.                           */
  action.sa_flags = 0;

  return sigaction(signum, &action, old_action);
};



int signal_gethandler(int signum, signal_handler_t *sighandler_ptr) {
  int err;
  struct sigaction action;

     /* get signal handler... */
  if ((err = sigaction(signum, NULL, &action /*old_action*/)) < 0)
    return err;

  if (sighandler_ptr != NULL)
    *sighandler_ptr = action.sa_handler;

  return 0;
};



int signal_getfreesignal(void) {

  signal_handler_t sighandler;

  if (signal_gethandler(SIGUSR1, &sighandler) >= 0)
    if (sighandler == SIG_DFL)
      return SIGUSR1;

  if (signal_gethandler(SIGUSR2, &sighandler) >= 0)
    if (sighandler == SIG_DFL)
      return SIGUSR2;

    /* no free signal found... */
  return -1;
};

