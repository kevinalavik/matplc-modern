/*
 *
 * Copyright (C) 2000 Mario de Sousa
 *
 * This code adapted from:
 * Parallel port DCF77 radio clock driver for Linux
 *
 * Original copyrights:
 * Copyright (C) 1999-2000 Andreas Voegele
 * Copyright (C) 1997-1999 Joachim Koenig
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program in the file COPYING; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA.
 */

/*
 * This driver is a simple passthrough driver to the parallel port.
 * Writing to a device controlled by this driver will write the bytes directly
 * to the parallel port. Reading from the device will read directly from
 * the parallel port.
 *
 * If the driver is built into the kernel, you can configure it using the
 * kernel command-line.  For example:
 *
 *   plc_parport=parport0,none,parport1 (bind the first plc_parport device to
 #                                       the first parallel port, disable the
 #                                       second device and bind the third
 #                                       device to the second parallel port)
 *
 *   plc_parport=0                      (disable the driver entirely)
 *
 * If the driver is loaded as a module, similar functionality is available
 * using module parameters.  The equivalent of the above command would be:
 *
 *	# insmod plc_parport.o parport=0,none,1
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#include <linux/devfs_fs_kernel.h>
#endif
#include <linux/string.h>
#include <linux/parport.h>

#include <asm/uaccess.h>

/*
#define DEBUG
*/


#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)
#error This kernel module does not support kernel versions previous to 2.2.0
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#ifndef PLC_PARPORT_IGNORE_KERNEL_VERSION
#error This kernel module does not directly support kernel versions later than 2.4.x If you are sure your kernel version is backward compatible, then compile this module with -DPLC_PARPORT_IGNORE_KERNEL_VERSION
#endif
#endif


#ifndef PLC_PARPORT_MAJOR
#define PLC_PARPORT_MAJOR 240
  /* Currently, major devices 240-254 are reserved for experimental use.
   * Please check http://www.kernel.org/pub/linux/docs/device-list/devices.txt
   * for an updated list of standardized major device numbers...
   */
#endif

/* Maximum number of devices supported by this driver */
#define PLC_PARPORT_MAX_NO 3

#define PLC_PARPORT_UNSPEC_PORT -3	/* port was not specified */
#define PLC_PARPORT_OFF         -2	/* turn the module off    */
#define PLC_PARPORT_NO_PORT     -1	/* do not use port        */

struct plc_parport_struct {
     /* pointer to the parport device to use */
  struct pardevice *dev;

     /* flag indicating if the device has been opened, i.e. if we have claimed
      * the parport. 0 -> device not open; 1 -> device open.
      */
  unsigned int dev_open;  /* flag indicating if the device has been opened */

     /* Current file offset position  */
  loff_t offset;
};

static struct plc_parport_struct plc_parport_table[PLC_PARPORT_MAX_NO];


/*********************************************/
/*                                           */
/*  Local Utility functions                  */
/*                                           */
/*********************************************/

static inline void plc_parport_write_pos(struct parport *port,
					 unsigned int offset,
					 u8 val)
{
  switch (offset) {
    case 0:
      parport_write_data(port, val);
      break;

    case 1:
/*    parport_write_status(port, val);*/
      break;

    case 2:
      parport_write_control(port, val);
      break;

    default:
  } /* switch */
}


static inline u8 plc_parport_read_pos(struct parport *port,
				      unsigned int offset)
{
  switch (offset) {
    case 0:
      return parport_read_data(port);

    case 1:
      return parport_read_status(port);

    case 2:
      return parport_read_control(port);

    default:
  } /* switch */

  return 0;
}


/*********************************************/
/*                                           */
/*  The functions required by a char device  */
/*                                           */
/*********************************************/

static loff_t plc_parport_llseek(struct file *filp, loff_t offset,
				 int origin)
{
  unsigned int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
  struct plc_parport_struct *plc_parport = &plc_parport_table[minor];

#ifdef DEBUG
  printk("plc_parport: llseek() called with origin=%d, offset=%Ld\n",
         origin, offset);
#endif

  if (plc_parport->dev_open == 0)
    /* file is not open... */
    return -EBADF;

  switch(origin) {
    case SEEK_SET:
      break;

    case SEEK_CUR:
      offset += plc_parport->offset;
      break;

    case SEEK_END:
      offset += 3;
      break;

    default:
    /* invalid Whence (origin) value */
    return -EINVAL;
  } /* switch */

  if (offset > 2)
    return -ESPIPE;

  return (plc_parport->offset = offset);
}


/* returns:
    register D in first byte;
    register C in second byte;
    register S in third byte;
*/
static ssize_t plc_parport_read(struct file *filp, char *user_buf,
				size_t length, loff_t * ppos)
{
  unsigned char buf[3];
  unsigned int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
  unsigned int count;
  struct plc_parport_struct *plc_parport = &plc_parport_table[minor];

#ifdef DEBUG
  printk("plc_parport: read() called with offset=%Ld\n",
         *ppos);
#endif

  if (plc_parport->dev_open == 0)
    /* file is not open... */
    return -EBADF;

  count = 0;
  while((plc_parport->offset < 3) && (count < length)) {
    buf[count] = plc_parport_read_pos(plc_parport->dev->port,
				      plc_parport->offset);
    (plc_parport->offset)++;
    count++;
  }

  if (copy_to_user(user_buf, buf, count))
    return -EFAULT;

  return count; /* returns the number of bytes read and passed to caller */
}


/* writes:
    first byte to register D;
    second byte to register C;
    third byte to register S;
*/
static ssize_t plc_parport_write(struct file *filp, const char *user_buf,
			         size_t length, loff_t * ppos)
{
  unsigned char buf[3];
  unsigned int minor = MINOR(filp->f_dentry->d_inode->i_rdev);
  unsigned int count;
  struct plc_parport_struct *plc_parport = &plc_parport_table[minor];

#ifdef DEBUG
  printk("plc_parport: write() called with offset=%Ld\n",
         *ppos);
#endif

  if (plc_parport->dev_open == 0)
    /* file is not open... */
    return -EBADF;

  if (length == 0)
    return 0; 

  if (plc_parport->offset > 2)
    /* file would become larger than what we can accomodate... */
    return -EFBIG;

  if (length > 3)
    length = 3; 

  if (copy_from_user(buf, user_buf, length))
    return -EFAULT;

  count = 0;
  while((plc_parport->offset < 3) && (count < length)) {
    plc_parport_write_pos(plc_parport->dev->port,
			  plc_parport->offset,
			  buf[count]);
    (plc_parport->offset)++;
    count++;
  }

  return count; /* returns the number of bytes written */
}


static int plc_parport_open(struct inode *inode, struct file *filp)
{
  unsigned int minor = MINOR(inode->i_rdev);
  struct plc_parport_struct *plc_parport = &plc_parport_table[minor];

  if (minor >= PLC_PARPORT_MAX_NO)
    return -ENXIO;

  if (!plc_parport->dev)
    return -ENXIO;

  /* 
   * If we want to protect against multiple processes using this device
   * simultaneously, this is where to control it...
   * For the moment, we will only allow one processes at a time access to read
   * and write to a parport device.
   */
  /* NOTE: checking this variable un-atomically is safe since only one process
   *       can be in kernel mode at a time, i.e. we are in the kernel, so
   *       we are protected against context switches.
   *       This is all very nice in a sinlge processor box, but what happens
   *       in an SMP environment? I don't know, and the author of the book I 
   *       am following didn't seem to know either at the time he wrote the
   *       book. I'll have to look this up somewhere else at a later date...
   *       The code I am basing this on also doesn't seem to be concerned about
   *       this effect either. Another hint sugesting it might just be OK
   *       afterall...???
   */
  if (plc_parport->dev_open == 1)
    return -EBUSY;

  /* This device driver supports sharing of the parallel port, although
   * how this can come in useful for the MatPLC, I don't know...
   * This is where we tell the parport kernel module we want to start using
   * this port.
   */
  if (parport_claim(plc_parport->dev) < 0)
    return -EBUSY;

  plc_parport->dev_open = 1;
  plc_parport->offset   = 0;
  MOD_INC_USE_COUNT;

  return 0;
}

/* a.k.a. the close function, as it is called when fclose() is used */
static int plc_parport_release(struct inode *inode, struct file *filp)
{
  unsigned int minor = MINOR(inode->i_rdev);
  struct plc_parport_struct *plc_parport = &plc_parport_table[minor];

  if (minor >= PLC_PARPORT_MAX_NO)
    return -ENXIO;

  if (!plc_parport->dev)
    return -ENXIO;

  /* If the device had not been opened previously, then return error. */
  if (plc_parport->dev_open == 0)
    return -EBADF;

  /* We don't need the port anymore, so we release it and allow other devices
   * to use it...
   */
  parport_release(plc_parport->dev);

  plc_parport->dev_open = 0;
  MOD_DEC_USE_COUNT;

  return 0;
}

static struct file_operations plc_parport_fops = {
  llseek :plc_parport_llseek,
  read   :plc_parport_read,
  write  :plc_parport_write,
  open   :plc_parport_open,
  release:plc_parport_release,
};


/******************************************************************/
/*                                                                */
/* Define our global (to the module) variables if a kernel module */
/*                                                                */
/******************************************************************/

#ifdef MODULE

EXPORT_NO_SYMBOLS;

/* array with the parport number used by each minor device... */
static int parport_nr[PLC_PARPORT_MAX_NO] =
    {[0 ... PLC_PARPORT_MAX_NO - 1] = PLC_PARPORT_UNSPEC_PORT };

/* array to store the command line configuration parameters... */
static char *parport[PLC_PARPORT_MAX_NO] = { NULL, };

static unsigned int major = PLC_PARPORT_MAJOR;


/* 
 * define the configuration parameters accepted by the module when 
 * it is inserted into the kernel...
 */
MODULE_PARM(parport, "1-" __MODULE_STRING(PLC_PARPORT_MAX_NO) "s");
MODULE_PARM(major, "i");

#else				/* #ifdef MODULE */

/**********************************************************/
/*                                                        */
/*   if not a module... then linked into the kernel       */
/*                                                        */
/**********************************************************/

static int parport_nr[PLC_PARPORT_MAX_NO] __initdata =
    {[0 ... PLC_PARPORT_MAX_NO - 1] = PLC_PARPORT_UNSPEC_PORT };

static unsigned int major __initdata = PLC_PARPORT_MAJOR;

static int parport_ptr __initdata = 0;

void __init plc_parport_setup(char *str, int *ints)
{
	if (!str) {
		if (ints[0] == 0 || ints[1] == 0) {
			/* disable driver on "plc_parport=" or "plc_parport=0"*/
			parport_nr[0] = PLC_PARPORT_OFF;
		}
	}
	else {
		while (str && parport_ptr < PLC_PARPORT_MAX_NO) {
			if (strncmp(str, "parport", 7) == 0) {
				int r = (int) simple_strtoul(str + 7, NULL,
							     10);
				parport_nr[parport_ptr++] = r;
			} else if (strncmp(str, "none", 4) == 0) {
				parport_nr[parport_ptr++] =
					PLC_PARPORT_NO_PORT;
			}

			if ((str = strchr(str, ',')))
				++str;
		}
	}
}

#endif				/* #ifdef MODULE */





#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
static devfs_handle_t devfs_handle = NULL;
#endif


/* Function only called by init_module() */
int __init plc_parport_init(void)
{
  int n;
  int count = 0;  /* count the number of devices we manage to register with */
  struct parport *port;
  struct pardevice *dev;

  for (n = 0; n < PLC_PARPORT_MAX_NO; ++n ) {
    plc_parport_table[n].dev = NULL;
    plc_parport_table[n].dev_open = 0;
    plc_parport_table[n].offset   = 0;
  }

  switch (parport_nr[0]) {
  case PLC_PARPORT_OFF:
    return 0;

  case PLC_PARPORT_UNSPEC_PORT:
    /* no configuration parameters given, so we register all
     * possible and existing devices, up to a maximum of PLC_PARPORT_MAX_NO 
     */
    for (port = parport_enumerate();
	 port && count < PLC_PARPORT_MAX_NO; 
	 port = port->next) {
      if ((dev = parport_register_device(port, "plc_parport",
					 NULL, NULL, NULL, 0, NULL))) {
	printk(KERN_INFO "plc_parport%d: using %s\n", count, dev->port->name);
	plc_parport_table[count].dev = dev;
	++count;
      }
    }				/* for(;;) */
    break;

  default:
    /* We were given command line configuration parameters, so we have
     * to abide by them...
     */
    for (n = 0; count < PLC_PARPORT_MAX_NO; ++n) {
      for (port = parport_enumerate(); port; port = port->next) {
	if (port->number == parport_nr[n]) {
	  if ((dev = parport_register_device(port, "plc_parport",
					     NULL, NULL, NULL, 0, NULL))) {
	    printk(KERN_INFO "plc_parport%d: using %s\n", n, dev->port->name);
	    plc_parport_table[n].dev = dev;

	    ++count;
	  }
	  break;
	}
      }				/* for(;;) */
    }				/* for(;;) */
    break;
  }				/* switch() */

  if (count) {
    int result;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
    result = devfs_register_chrdev(major, "plc_parport",
				   &plc_parport_fops);
    if (result < 0) {
      printk("plc_parport: unable to get major %u\n", major);
      return -ENODEV;
    }
    if (major == 0)
      major = result;
    devfs_handle = devfs_mk_dir(NULL, "plc_parport", NULL);
    devfs_register_series(devfs_handle, "%u", PLC_PARPORT_MAX_NO,
			  DEVFS_FL_DEFAULT, major, 0,
			  S_IFCHR | S_IRUGO, &plc_parport_fops, NULL);
#else
    result = register_chrdev(major, "plc_parport", &plc_parport_fops);
    if (result < 0) {
      printk("plc_parport: unable to get major %u\n", major);
      return -ENODEV;
    }
    if (major == 0)
      major = result;
#endif
  } else {
    printk(KERN_INFO "plc_parport: no devices found\n");
    return -ENODEV;
  }

  return 0;
}

#ifdef MODULE

MODULE_AUTHOR("Mario de Sousa");
MODULE_DESCRIPTION("Passthrough parallel port driver for the MatPLC.");

int init_module(void)
{
  if (parport[0]) {
    int n;

    /* parse the configuration arguments... */
    for (n = 0; n < PLC_PARPORT_MAX_NO && parport[n]; ++n) {
      if (strncmp(parport[n], "none", 4) == 0)
	/* do not use port... */
	parport_nr[n] = PLC_PARPORT_NO_PORT;
      else {
	char *ep;
	int r = (int) simple_strtoul(parport[n], &ep, 0);
	if (ep != parport[n])
	  parport_nr[n] = r;
	else {
	  printk(KERN_ERR "plc_parport: bad port specifier `%s'\n",
		 parport[n]);
	  return -ENODEV;
	}
      }
    }
  }

  /* now do the initialisation itself... */
  return plc_parport_init();
}

void cleanup_module(void)
{
  int n;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
  devfs_unregister(devfs_handle);
  devfs_unregister_chrdev(major, "plc_parport");
#else
  unregister_chrdev(major, "plc_parport");
#endif

  for (n = 0; n < PLC_PARPORT_MAX_NO; ++n)
    if (plc_parport_table[n].dev)
      parport_unregister_device(plc_parport_table[n].dev);
}

#endif				/* #ifdef MODULE */
