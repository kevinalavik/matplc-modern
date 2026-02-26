/*
 * Copyright (C) 1995  Sam Moore, Warren Jasper
 * Copyright (C) 1997  Tim Edwards
 * All rights reserved.
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

/*----------------------------------------------------------------*/
/* ciodio48.c							  */
/*----------------------------------------------------------------*/

#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/malloc.h>
#include <linux/string.h>
#include <linux/signal.h>
#include <asm/io.h>
#include <asm/segment.h>
#include <asm/system.h>
#include <stdlib.h>
#include "dio48.h"

/***************************************************************************
 *
 * Prototype of pubic and private functions.
 *
 ***************************************************************************/

#define BYTE unsigned char
#define LONG unsigned long

static int dio_read(struct inode *iNode, struct file *filePtr, char *buf, int count);
static int dio_write(struct inode *iNode, struct file *filePtr, char *buf, int count);
static int dio_open(struct inode *iNode, struct file *filePtr);
static void dio_close(struct inode *iNode, struct file *filePtr);
static int dio_ioctl(struct inode *iNode, struct file *filePtr, unsigned int cmd, LONG arg);

int init_module(void);
void cleanup_module(void) ;

/***************************************************************************
 *
 * Global data.
 *
 ***************************************************************************/

/* Default device major number */

#ifndef DEFAULT_MAJOR_DEV
#define DEFAULT_MAJOR_DEV  31    /* Default Major Device Number */
#endif

/* Base I/O Address */

#ifndef BASE_REG
#define BASE_REG         0x300   /* Default base address for CIO-DIO48 */
#endif

/* Number of channels */

#ifndef CHANNELS
#define CHANNELS	2
#endif

#define PORT_OFFSET	4
#define A_OFFSET	0
#define B_OFFSET	1
#define C_OFFSET	2
#define CTRL_OFFSET     3

#define A_MASK		0xff
#define B_MASK		0xff00
#define C_MASK		0xff0000

typedef struct CHANNEL_REC {
   int           open;        /* for lockout of other processes */
   unsigned char mode;        /* for readback of 82C55 mode */
} ChanRec;

static ChanRec Chan[CHANNELS];              /* Channel specific information   */
static int MajorNumber = DEFAULT_MAJOR_DEV; /* Major number compiled in       */
/* char kernel_version[] = UTS_RELEASE; */  /* Required by module             */

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/***************************************************************************
 *
 * read() service function
 *
 ***************************************************************************/

static int dio_read(struct inode *iNode, struct file *filePtr, char *buf, int count)
{
  long  value = 0L;
  int   minor = MINOR(iNode->i_rdev);
  int   portbase = BASE_REG + (minor * PORT_OFFSET);

  if (minor < CHANNELS) {
    value = inb(portbase + A_OFFSET);
    if (count > 1) value += inb(portbase + B_OFFSET) * 256;
    if (count > 2) value += inb(portbase + C_OFFSET) * 65536;

    cli();
    put_fs_long(value, buf);
    sti();
    return count;
  }

  printk("dio_read: Incorrect minor number (%d).\n", minor);
  return -1;
}

/***************************************************************************
 *
 * write() service function
 *
 ***************************************************************************/

static int dio_write(struct inode *iNode, struct file *filePtr, char *buf, int count)
{
    int  minor = MINOR(iNode->i_rdev);
    int  portbase = BASE_REG + (minor * PORT_OFFSET);
    long value;

    if ( minor >= CHANNELS ) {
        printk("dio_write: bad minor number = %d\n", minor );
        return -1;
    }
    cli();
    value = get_fs_long(buf) & 0xffffff;
    outb_p( (BYTE) (value & A_MASK),        portbase + A_OFFSET);
    outb_p( (BYTE)((value & B_MASK) >> 8),  portbase + B_OFFSET); 
    outb_p( (BYTE)((value & C_MASK) >> 16), portbase + C_OFFSET); 
    sti();
    #ifdef DEBUG
        printk("DIO set to %#x\n", value & 0xf);
    #endif
    return 3;
}


/***************************************************************************
 *
 * open() service handler
 *
 ***************************************************************************/

static int dio_open(struct inode *iNode, struct file *filePtr)
{
    int minor = MINOR(iNode->i_rdev);
    int portbase = BASE_REG + (minor * PORT_OFFSET);

    /* 
       check if device is already open: only one process may read from a
       port at a time.  There is still the possibility of two processes 
       reading from two different channels messing things up. However,
       the overhead to check for this may not be worth it.
    */

    if ( Chan[minor].open == TRUE ) {
        return -EBUSY;
    }

    Chan[minor].open = TRUE;              /* The device is open */
    Chan[minor].mode = 0;
    outb_p(0x0, portbase + CTRL_OFFSET);

    #ifdef DEBUG
        printk("%s: open(): minor %d mode %d.\n", ADAPTER_ID, minor, Chan[minor].mode);
    #endif
    return 0;   
}

/***************************************************************************
 *
 * close() service handler
 *
 ***************************************************************************/

static void dio_close(struct inode *iNode, struct file *filePtr)
{
    int minor = MINOR(iNode->i_rdev);

    #ifdef DEBUG
        printk("%s: close() of minor number %d.\n", ADAPTER_ID, minor);
    #endif

    Chan[minor].open = FALSE;
}

/***************************************************************************
 *
 * iotctl() service handler
 *
 ***************************************************************************/

/* Note:
    Remember that FIOCLEX, FIONCLEX, FIONBIO, and FIOASYN are reserved ioctl cmd numbers
*/

static int dio_ioctl(struct inode *iNode, struct file *filePtr, unsigned int cmd, LONG arg)
{
  int minor = MINOR(iNode->i_rdev);
  int portbase = BASE_REG + (minor * PORT_OFFSET);

  switch (cmd) {
    case DIO_SET_MODE:
      Chan[minor].mode = (BYTE)arg;
      outb_p((BYTE)(MODE_SET | arg), portbase + CTRL_OFFSET); break;
      break;

    case DIO_GET_MODE:
      put_fs_long(Chan[minor].mode, (long*) arg);
      break;

    case DIO_SETA:
      outb_p((BYTE)arg, portbase + A_OFFSET); break;
      break;

    case DIO_SETB:
      outb_p((BYTE)arg, portbase + B_OFFSET); break; 
      break;

    case DIO_SETC:
      outb_p((BYTE)arg, portbase + C_OFFSET); break;
      break;

    case DIO_GETA:
      put_fs_long(inb(portbase + A_OFFSET), (long*)arg);
      break;

    case DIO_GETB:
      put_fs_long(inb(portbase + B_OFFSET), (long*)arg);
      break;

    case DIO_GETC:
      put_fs_long(inb(portbase + C_OFFSET), (long*)arg);
      break;

    default:
      return(-EINVAL);
      break;
  }
  return 0;
}

/***************************************************************************
 *
 * See /usr/include/linux/fs.h  for more information
 *
 ***************************************************************************/

static struct file_operations dio_fops = {
	NULL,           /* lseek */
	dio_read,	/* read */
	dio_write,      /* write */
	NULL,		/* readdir */
	NULL,		/* select */
	dio_ioctl,	/* ioctl */
	NULL,		/* mmap */
	dio_open,	/* open */
	dio_close,	/* release */
	NULL,		/* fsync */
	NULL,		/* fasync */
	NULL,		/* check_media_change */
	NULL		/* revalidate */
};

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 *
 * Loads driver. Called when "insmod dio48.o" is invoked on the command line.
 *
 ***************************************************************************/

int init_module(void) 
{
  int i;

  /* Register as a device with kernel.  */

  if (register_chrdev(MajorNumber, "dio48", &dio_fops)) {
    printk("%s: Failure to load module.\n", ADAPTER_ID);
    return -EIO;
  } else {
    printk("%s: address=%#x.",ADAPTER_ID, BASE_REG);
    printk(" 4/24/97 tim@bach.ece.jhu.edu\n");
  }
  cli();

  /* Set all channel structures to show nothing active/open */

  for (i = 0; i < CHANNELS; i++)
    Chan[i].open = FALSE;

  sti();
  return 0;
}

/***************************************************************************
 *
 * Remove driver. Called when "rmmod dio48" is run on the command line.
 *
 ***************************************************************************/

void cleanup_module(void) 
{
    if (MOD_IN_USE) {
      printk("%s: device busy, remove delayed\n", ADAPTER_ID);
    }

    if (unregister_chrdev(MajorNumber, "dio48") != 0) {
      printk("%s: cleanup_module failed\n", ADAPTER_ID);
    } else {

    #ifdef DEBUG
        printk("%s: module removed.\n", ADAPTER_ID);
    #endif

    }    
}

#ifdef __cplusplus
}
#endif

