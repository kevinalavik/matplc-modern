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


#ifndef SIGNAL_UTIL_H
#define SIGNAL_UTIL_H



 /*

    Signal handling utility functions.

    The current implementation assumes POSIX signal handling functions are available.

    If compiling under a different system that does not support POSIX signal functions,
    then this file should be changed to one that emulates POSIX signals.
  */


#include <signal.h>


typedef void (* signal_handler_t)(int);


int signal_action(int signum,
                  struct sigaction *new_action,
                  struct sigaction *old_action);

int signal_sethandler(int signum, signal_handler_t sighandler,
                      struct sigaction *old_action);

int signal_gethandler(int signum, signal_handler_t *sighandler_ptr);


  /* the the number of a signal that still has the default behaviour... */
int signal_getfreesignal(void);



#endif    /* SIGNAL_UTIL_H */
