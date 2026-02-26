/*
 * (c) 2000 Jiri Baum
 *     2001 Juan Carlos Orozco
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
  unsigned char ch; 
  plc_pt_t quit, Automatic, Start, TFTempSP, GAValveSP, 
    BeltMotorSpdSP, Z1TempSP, Z2TempSP, Z3TempSP, 
    Z1TWV_Position, Z2TWV_Position, Z3TWV_Position;

  quit = get_pt("quit");
  Automatic = get_pt("Automatic");
  Start = get_pt("Start");
  TFTempSP = get_pt("TFTempSP");
  GAValveSP = get_pt("GAValveSP");

  BeltMotorSpdSP = get_pt("BeltMotorSpdSP");
  Z1TempSP = get_pt("Z1TempSP");
  Z2TempSP = get_pt("Z2TempSP");
  Z3TempSP = get_pt("Z3TempSP");
  Z1TWV_Position = get_pt("Z1TWV_Position");
  Z2TWV_Position = get_pt("Z2TWV_Position");
  Z3TWV_Position = get_pt("Z3TWV_Position");

  setcbreak();
  while (1) {
    plc_scan_beg();
    plc_update();

    ch = getchar();
    switch(ch){
    case 'q': case 'Q': case 3: /* Ctrl-C */
      plc_set(quit, !plc_get(quit));
      break;
    case 'M': case 'm':
      plc_set(Automatic, !plc_get(Automatic));
      break;
    case 'A': case 'a':
      plc_set(Start, 1);
      break;
    case 'Z': case 'z':
      plc_set(Start, 0);
      break;
    case 'S': case 's':
      if(plc_get(Automatic))
	plc_set_f32(TFTempSP, plc_get_f32(TFTempSP)+1.0);
      else
	plc_set_f32(GAValveSP, plc_get_f32(GAValveSP)+1.0);
      break;
    case 'X': case 'x':
      if(plc_get(Automatic))
	plc_set_f32(TFTempSP, plc_get_f32(TFTempSP)-1.0);
      else
	plc_set_f32(GAValveSP, plc_get_f32(GAValveSP)-1.0);
      break;
    case 'D': case 'd':
      plc_set_f32(BeltMotorSpdSP, plc_get_f32(BeltMotorSpdSP)+1.0);
      break;
    case 'C': case 'c':
      plc_set_f32(BeltMotorSpdSP, plc_get_f32(BeltMotorSpdSP)-1.0);
      break;
    case 'F': case 'f':
      if(plc_get(Automatic))
	plc_set_f32(Z1TempSP, plc_get_f32(Z1TempSP)+1.0);
      else
	plc_set_f32(Z1TWV_Position, plc_get_f32(Z1TWV_Position)+1.0);
      break;
    case 'V': case 'v':
      if(plc_get(Automatic))
	plc_set_f32(Z1TempSP, plc_get_f32(Z1TempSP)-1.0);
      else
	plc_set_f32(Z1TWV_Position, plc_get_f32(Z1TWV_Position)-1.0);
      break;
    case 'G': case 'g':
      if(plc_get(Automatic))
	plc_set_f32(Z2TempSP, plc_get_f32(Z2TempSP)+1.0);
      else
	plc_set_f32(Z2TWV_Position, plc_get_f32(Z2TWV_Position)+1.0);
      break;
    case 'B': case 'b':
      if(plc_get(Automatic))
	plc_set_f32(Z2TempSP, plc_get_f32(Z2TempSP)-1.0);
      else
	plc_set_f32(Z2TWV_Position, plc_get_f32(Z2TWV_Position)-1.0);
      break;
    case 'H': case 'h':
      if(plc_get(Automatic))
	plc_set_f32(Z3TempSP, plc_get_f32(Z3TempSP)+1.0);
      else
	plc_set_f32(Z3TWV_Position, plc_get_f32(Z3TWV_Position)+1.0);
      break;
    case 'N': case 'n':
      if(plc_get(Automatic))
	plc_set_f32(Z3TempSP, plc_get_f32(Z3TempSP)-1.0);
      else
	plc_set_f32(Z3TWV_Position, plc_get_f32(Z3TWV_Position)-1.0);
      break;
     
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
