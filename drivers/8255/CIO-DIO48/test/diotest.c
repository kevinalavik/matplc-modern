/*
 * Copyright (C) 1995  Sam Moore, Warren Jasper
 * All rights reserved.
 * Modified by Tim Edwards, April, 1997 for CIO-DIO48 board
 *
 * This software may be freely copied, modified, and redistributed
 * provided that this copyright notice is preserved on all copies.
 *
 * You may not distribute this software, in whole or in part, as part of
 * any commercial product without the express consent of the authors.
 *
 * There is no warranty or other guarantee of fitness of this software
 * for any purpose.  It is provided solely "as is".
 *
 */

/***************************************************************************
 *
 *  diotest.c
 *
 *  This program is used to test the CIO-DIO48
 *  Linux loadable module (dio48).
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../dio48.h"

/***************************************************************************
 *
 *  Global Data
 *
 ***************************************************************************/

#define LONG unsigned long
#define BYTE unsigned char

char *DevName = "/dev/dio48/dio0";

/***************************************************************************
 *
 *  Main
 *
 ***************************************************************************/

int main(int argc, char **argv)
{
  int	fd;
  long  dataval;
  int   rval;

  /* Open the device file for the counter */

  if ((fd = open("/dev/dio48/dio0", O_RDWR)) < 0) {
    printf("error opening DIO48 device\n");
    exit(2);
  }
  printf("Setting mode of 82C55 to all ports output.\n");
  ioctl(fd, DIO_SET_MODE, (BYTE)0x80);

  printf("sleep before read\n");
  sleep(1);
  
  /* infinite loop; stop by exiting program */

  for (;;) {
    dataval = (LONG)0xffffff;
    if (write(fd, &dataval, 3) != 3 ) {
      printf("main() in test.c: error on write.\n");
      exit(1);
    }
    if ((rval = read(fd, &dataval, 3)) != 3 ) {
      printf("main() in test.c: error on readback.\n");
      printf("   read() returned %d, expected 3\n", rval); 
      exit(1);
    }
    else printf("Data expected: 0xffffff, Data read: %lx\n", dataval);
    sleep(5);
    dataval = (LONG)0x00;
    if (write(fd, &dataval, 3) != 3 ) {
      printf("main() in test.c: error on write.\n");
      exit(1);
    }
    if (read(fd, &dataval, 3) != 3 ) {
      printf("main() in test.c: error on readback.\n");
      exit(1);
    }
    else printf("Data expected: 0x000000, Data read: %lx\n", dataval);
    sleep(5);
  }

  close(fd);
  return 0;
}
	
