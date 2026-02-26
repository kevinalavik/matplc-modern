/*
 * (c) 2004 Mario de Sousa
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

/* The Real-Time section...
 * 
 * This section attempts to guarantee that the MatPLc module works
 * under Real-Time constraints. Whether these constraints are hard or soft
 * will mostly depend on the underlying OS. 
 * In any event, the MatPLC simply tries to lock all memory into RAM
 * (if memlock = 1 is chosen in the config file), and
 * to set the apropriate scheduling priority, along with the SCHED_FIFO
 * scheduling policy if a priority > PRIORITY_MIN is chosen.
 */

#include <plc.h>
#include <rt/rt.h>
#include <rt/rt_setup.h>



int rt_setup(const char *module_name) {

  /* We don't have any seting up to do, other than calling rt_init()
   * Note that rt_init() will try to lock all the memory into RAM if
   * memlock option is set to 1, so by calling rt_init() here we are 
   * essentially checking whether the OS we are currently using
   * supports the mlockall() function, and if not, abort the MatPLC
   * setup process right now, rather than let the seting up process
   * continue to completion, and have the modules produce an error
   * as they start running!
   */
   return rt_init(module_name);
}


int rt_shutdown(void) {
  return rt_done();
}
