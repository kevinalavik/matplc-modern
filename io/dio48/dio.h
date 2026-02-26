/*
 * Copyright (C) 1995  Warren Jasper
 * All rights reserved.
 *
 */

/*
 * dio.h
 */

/*  ioctl() values */

#include <linux/ioctl.h>
#define IOCTL_MAGIC 'w'

#define DIO_SET_MODE         _IO(IOCTL_MAGIC,  1)
#define DIO_SET_DIRECTION    _IO(IOCTL_MAGIC,  2)

#define IOCTL_MAXNR 2         /* maxinum ordinal number */


/* digital output */
#define DIO_0 0x1
#define DIO_1 0x2
#define DIO_2 0x4
#define DIO_3 0x8
#define DIO_4 0x10
#define DIO_5 0x20
#define DIO_6 0x40
#define DIO_7 0x80

/* Digital I/O Modes */ 
#define MODE_IO            0
#define MODE_STROBE_IO     1
#define MODE_BIDIRECTIONAL 2

/* Digital I/O Direction Settings */
#define PORT_INPUT         1
#define PORT_OUTPUT        0
#define HIGH_PORT_INPUT    2
#define HIGH_PORT_OUTPUT   3
#define LOW_PORT_INPUT     4
#define LOW_PORT_OUTPUT    5
