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
 * keyboard input - sample code 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <plc.h>

plc_pt_t get_pt(const char *pt_name)
{
  plc_pt_t pt_handle;

  pt_handle = plc_pt_by_name(pt_name);

  if (!pt_handle.valid) {
    printf("Could not get valid handle to %s.\n", pt_name);
    exit(1);
  }

  return pt_handle;
}

#include <termios.h>
#include <unistd.h>

/* not sure if this is right... */
void setcbreak(void)
{
  struct termios t;

  tcgetattr ( 0, &t);
  t.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP);
  t.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG );

  tcsetattr ( 0, TCSANOW, &t);
}

void keyboard(void) {
  unsigned char ch; plc_pt_t p[256];
  memset(p, 0, sizeof(p)); /* reset all the .valid fields */

  p['I']=get_pt("in");
  p['q']=get_pt("quit");
  p['Q']=get_pt("quit");
  p[3]=get_pt("quit"); /* Ctrl-C */

  setcbreak();
  while (1) {
    plc_scan_beg();
    plc_update();

    ch = getchar();
    if (p[ch].valid) {
      plc_set(p[ch], !plc_get(p[ch]));
    }


    if ((ch >= '0') && (ch <= '9')) {
     plc_set(p['I'], (i32)(ch - '0'));
    }

    /* end of scan */
    plc_update();
    plc_scan_end();
  }
}

int main(int argc, char **argv)
{
  const char *module_name = "Kbd";

  /* printf("Initializing.\n"); */
  if (plc_init(module_name, argc, argv) < 0) {
    printf("Error initializing PLC\n");
    return -1;
  }

  keyboard();

  return 0;
}
