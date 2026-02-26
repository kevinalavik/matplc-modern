/*
 * (c) 2000 Jiri Baum
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
 * light chaser - sample code 
 */

#include <stdio.h>
#include <math.h>

#include <plc.h>
#include <logic/timer.h>

const int num_lights = 4; /* how many lights in the chase, max 9 */ 

/* 
 * Get a handle to a point, checking that it's valid. Abort if the handle
 * could not be obtained.
 */
plc_pt_t get_pt(const char *pt_name)
{
  plc_pt_t pt_handle;

  pt_handle = plc_pt_by_name(pt_name);

  if (!pt_handle.valid) {
    plc_log_errmsg(1, "Could not get handle to %s, aborting.", pt_name);
    printf("Could not get valid handle to %s.\n", pt_name);
    exit(1);
  }

  return pt_handle;
}

/* Obtain the chasing delay from the config, default 0.5 seconds. */
double get_delay(void)
{
  double res;

  /* 
   * Get the value "delay", of type double, into res.
   * Minimum 0, maximum HUGE_VAL (ie, no maximum), default 0.5
   */
  if (conffile_get_value_d("delay", &res, 0, HUGE_VAL, 0.5) == 0) {
    /* a return value of 0 means OK - return the value obtained */
    return res;
  }

  /* some sort of problem - log it, and use half a second */
  plc_log_wrnmsg(1, "Could not get delay from config, using 0.5s");

  return 0.5;
}

void chase(void)
{
  plc_pt_t left, right, L[num_lights];
  char Lname[] = "Ln";
  int i, dir=1, cur=num_lights-1;
  double delay;
  plc_timer_t timer;

  /* get the point handles */
  left = get_pt("left");
  right = get_pt("right");
  for(i=0;i<num_lights;i++) {
    Lname[1] = '1'+i;
    L[i]=get_pt(Lname);
  }
  
  /* get the delay from the config */
  delay = get_delay();
  /* start the timer ticking */
  plc_timer_start(timer);

  /* now chase! */
  while (1) {
    /* beginning of scan */
    plc_scan_beg();
    plc_update();

    /* check for change of direction */
    if (plc_get(left) && !plc_get(right)) {
      dir = -1;
    } else if (plc_get(right)) {
      dir = 1;
    }

    /* has delay elapsed? */
    if (plc_timer_done(timer, delay)) {
      /* move the light along */
      plc_set(L[cur],0);
      cur=(cur+dir+num_lights) % num_lights; /* be careful about -1 % n */
      plc_set(L[cur],1);

      /* set timer back by delay; this avoids loss of precision */
      plc_timer_add(timer, -delay);
    }

    /* end of scan */
    plc_update();
    plc_scan_end();
  }
}

int main(int argc, char **argv)
{
  /* initialise the MatPLC library */
  if (plc_init("Chaser", argc, argv) < 0) {
    printf("Error initializing PLC\n");
    return -1;
  }

  chase();

  return 0;
}
