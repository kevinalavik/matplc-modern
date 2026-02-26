/*
 * (c) 2000 Mario de Sousa
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
  parport_util_dir.c

  Utility functions to controll the parallel port I/O

  These functions assume a PC hardware compatible parallel port, and
  controls it by accessing it's io mapped registers directly [inb(), outb()]

  Processes need to be root, or run setuid-root, in order to run this,
  even with the call to ioperm().

*/

/* 
  This code is based on the parport.c code of the EMC project.
  http://www.isd.cme.nist.gov/projects/emc/emcsoft.html
*/


/*
#define MAIN
*/

#ifdef MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/*
  Because of a limitation in gcc (present at least in 2.7.2.1 and below), you
  _have to_ compile any source code that uses these routines with optimisation
  turned on (gcc -O1 or higher), or alternatively #define extern to be
  empty before #including <asm/io.h>.
  */
#include <unistd.h>		/* ioperm()  for libc5 */
#include <sys/io.h>		/* ioperm()  for glibc */
#define extern 
/*#include <asm/io.h>*/		/* outb, inb */

/* file access functions... */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



#include "parport_util.h"		/* these decls */


/******************************/
/* global status variables... */
/******************************/

/* initialized status of library... */
static int __ppt_status = 0;

/* input or output mode the C and D registers are using */
static ppt_regdir_t _Ddir = regdir_none;
static ppt_regdir_t _Cdir = regdir_none;


/* The low-level parallel port access functions virtual table type */
typedef struct {
	int  (*ppt_done)(void);
	u8   (*in )(unsigned short reg);
	void (*out)(u8  value, unsigned short reg);
	} ppt_vt_t;

/* The low-level parallel port access functions virtual table */
static ppt_vt_t *_ppt_vt = NULL;



/*********************/
/* utility functions */
/*********************/


#define __ppt_fully_initialized {   \
          __ppt_status = 1;         \
        };

#define __ppt_fully_shutdown {      \
          __ppt_status = 0;         \
        };

#define __assert_ppt_not_initialized(ret_val) {   \
          if (__ppt_status != 0)                  \
            return ret_val;                       \
        };

#define __assert_ppt_fully_initialized(ret_val) { \
          if (__ppt_status != 1)                  \
            return ret_val;                       \
        };


#define _D_reg (_parport_io_address)
#define _S_reg (_parport_io_address + 1)
#define _C_reg (_parport_io_address + 2)



/***************************/
/* Direct Access functions */
/***************************/

/*
  Digital I/O

  Bitmap:
  Data In/Out     Base+0      D0-D7   8 LS TTL outputs/inputs (programable)
  Status In       Base+1      S3-S7   5 LS TTL inputs
  Control In/Out  Base+2      C0-C3   4 TTL Open Collector outputs/inputs

  NOTE: C0-C3 are usually used solely as outputs, but since they are 
        open collector, they can be put into high impedance state by writing
        1 to every output. In this state, these lines may be used as inputs!
        That is why we inhibit any writes to this register while it is
        in input mode.

  NOTE: Some of the S and C bits are inverted by the parallel port hardware.
        We re-invert all these bits so that this library presents a coherent
        positive logic interface.
*/

#define PARPORT_IO_WIDTH 3	/* number of io addresses used by parport */

/* the IO address of the board; Set by ppt_init_dir()... */
static unsigned long int _parport_io_address;

/* forward declaration */
static int ppt_done_dir(void);

static ppt_vt_t ppt_vt_dir = {
		ppt_done_dir,
		inb,
		outb
		};


/**********************************/
/* Kernel Module Access functions */
/**********************************/

/* the handle to the device file */
int _dev_file = 0;

static void ppt_out_krn(u8 value, unsigned short reg)
{
  lseek(_dev_file, reg - _parport_io_address, SEEK_SET);
  write(_dev_file, &value, 1)?value:0;
}

static u8 ppt_in_krn(unsigned short reg)
{
  u8 tmp_val;

  lseek(_dev_file, reg - _parport_io_address, SEEK_SET);
  read(_dev_file, &tmp_val, 1);
  return tmp_val;
}

/* forward declaration */
static int ppt_done_krn(void);

static ppt_vt_t ppt_vt_krn = {
		ppt_done_krn,
		ppt_in_krn,
		ppt_out_krn
		};



/*********************/
/* Library functions */
/*********************/

/* Set up the parallel port, using direct access ( inb(), outb() )
 * with data bits D0-D7 using direction Ddir 
 * and data bits C0-C3 using direction Cdir
 */
int ppt_init_dir(int parport_io_address, 
 	         ppt_regdir_t Cdir,
	         ppt_regdir_t Ddir)
{
  int switched_ids;
  u8 CByte;

  __assert_ppt_not_initialized(-1);

  /* make sure we are running with efective user id as root          */
  /* otherwise, the ioperm call will produce a segmentation fault... */
  switched_ids = 0;
  if (geteuid() != 0) {
    /* try switching effective and real user id */
    setreuid (geteuid (), getuid ());
    switched_ids = 1;
    if (geteuid() != 0) {
      /* if still not root, then switch back and return error... */
      setreuid (geteuid (), getuid ());
      return -1;
    }
  }

  /* turn on port access */
  if (ioperm(parport_io_address, PARPORT_IO_WIDTH, 1) < 0)
    return -1;

  /* revert to previous effective and real user id */
  if (switched_ids == 1) {
    setreuid (geteuid (), getuid ());
  }

  /* set the global variable giving the base io addr */
  _parport_io_address = parport_io_address;

  /* Initialize the virtual table... */
  _ppt_vt = &ppt_vt_dir;

  /* disable use of IRQ by parallel port - set C4 to 0 */
  CByte = _ppt_vt->in(_C_reg);
  CByte &= ~0x10;
  _ppt_vt->out(CByte, _C_reg);

  ppt_set_Cdir(Cdir);
  ppt_set_Ddir(Ddir);

  __ppt_fully_initialized;  

  return 0;
}


/* Set up the parallel port, accessing through kernel module,
 * with data bits D0-D7 using direction Ddir
 * and data bits C0-C3 using direction Cdir
 */
int ppt_init_krn(const char *dev_file_name, 
 	         ppt_regdir_t Cdir,
	         ppt_regdir_t Ddir)
{
  int dev_file;
  u8 CByte;

  __assert_ppt_not_initialized(-1);

  /* open the device file */
  if ((dev_file = open(dev_file_name, O_RDWR)) < 0)
    /* error openning dev_file... */
    return -1;

  /* set the global variable giving the dev_file handle */
  _dev_file = dev_file;

  /* Initialize the virtual table... */
  _ppt_vt = &ppt_vt_krn;

  /* disable use of IRQ by parallel port - set C4 to 0 */
  CByte = _ppt_vt->in(_C_reg);
  CByte &= ~0x10;
  _ppt_vt->out(CByte, _C_reg);

  ppt_set_Cdir(Cdir);
  ppt_set_Ddir(Ddir);

  __ppt_fully_initialized;  

  return 0;
}


static int ppt_done_dir()
{
  int switched_ids;

  /* make sure we are running with efective user id as root          */
  /* otherwise, the ioperm call will produce a segmentation fault... */
  switched_ids = 0;
  if (geteuid() != 0) {
    /* try switching effective and real user id */
    setreuid (geteuid (), getuid ());
    switched_ids = 1;
    if (geteuid() != 0) {
      /* if still not root, then switch back and return error... */
      setreuid (geteuid (), getuid ());
      return -1;
    }
  }

  /* turn off port access */
  ioperm(_parport_io_address, PARPORT_IO_WIDTH, 0);

  /* revert to previous effective and real user id */
  if (switched_ids == 1) {
    setreuid (geteuid (), getuid ());
  }

  return 0;
}


static int ppt_done_krn()
{
  return close(_dev_file);
}


int ppt_done()
{
  __assert_ppt_fully_initialized(-1);

  _ppt_vt->ppt_done();

  _ppt_vt = NULL;

  __ppt_fully_shutdown;

  return 0;
}



int ppt_set_Cdir(ppt_regdir_t Cdir)
{
  __assert_ppt_fully_initialized(-1);

  _Cdir = Cdir;

  if (Cdir == regdir_out)
    return 0;

  /* set the C outputs to high impedance state = set to 1 */
  return ppt_set_C(0x0F);
};


int ppt_set_Ddir(ppt_regdir_t Ddir)
{
  u8 CByte;

  __assert_ppt_fully_initialized(-1);

  _Ddir = Ddir;

  /*
   *  C5 = 0 -> D0-D7 outputs
   *  C5 = 1 -> D0-D7 inputs (high impedance)
   */

  CByte = _ppt_vt->in(_C_reg);

  if (Ddir == regdir_out)
    /* reset C5 bit */
    CByte &= ~0x20;
  else
    /* set C5 bit */
    CByte |= 0x20;

  _ppt_vt->out(CByte, _C_reg);

  return 0;
}


ppt_regdir_t ppt_get_Ddir(void) {
  return _Ddir;
};

ppt_regdir_t ppt_get_Cdir(void) {
  return _Cdir;
};

ppt_regdir_t ppt_get_Sdir(void) {
  return regdir_in;
};


/*
  ppt_get_S() maps S3-S7 onto 00011111, S3 on 00000001
  providing 5 digital inputs
 
  Note that S7 is reverse polarity, but we correct it here!
*/
/* returns 0 on success, -1 on error */
int ppt_get_S(u8 *value)
{
  u8 inByte;

  __assert_ppt_fully_initialized(-1);

  /* latch input */
  inByte = _ppt_vt->in(_S_reg);

  /* S3-S7 are at 11111000, so we must shift right 3 bits */
  /* S3     is at 00001000, so we must shift right 3 bits */
  inByte >>= 3;

  /* invert S7 - previously inverted by parallel port hardware */
  inByte ^= 0x10;

  /* make sure empty bits are set to 0 */
  inByte &= 0x1F;

  if (value != NULL)
    *value = inByte;
  
  return 0;
}

/*
  ppt_get_Sbit(int index) maps 5 index values 3-7 onto S3-S7, respectively

  Note that S7 is reverse polarity, but we correct it here!
*/
/* returns 0/1 on success, -1 on error */
int ppt_get_Sbit(int index)
{
  u8 inByte, mask;

  __assert_ppt_fully_initialized(-1);

  if ((index < 3) || (index > 7))
    return -1;

  index -= 3;

  if (ppt_get_S(&inByte) < 0)
    return -1;

  mask = (0x01 << index);

  return (inByte & mask) >> index;
}


/*
  ppt_get_D() maps 8 index values 0-7 onto D0-D7, respectively
*/
/* returns 0 on success, -1 on error */
int ppt_get_D(u8 *value)
{
  __assert_ppt_fully_initialized(-1);

  if (value != NULL)
    *value = _ppt_vt->in(_D_reg);

  return 0;
}


/*
  ppt_get_Dbit(int index) maps 8 index values 0-7 onto D0-D7, respectively
*/
/* returns 0/1 on success, -1 on error */
int ppt_get_Dbit(int index)
{
  u8 inByte, mask;

  __assert_ppt_fully_initialized(-1);

  if ((index < 0) || (index > 7))
    return -1;

  if (ppt_get_D(&inByte) < 0)
    return -1;

  mask = (0x01 << index);

  return (inByte & mask) >> index;
}


/*
  ppt_get_C() maps C0-C3 onto 00001111

  Note that C0, C1, and C3 are inverted outputs; 'value' will return
  what was written to these, so checking a 1 means a 1 was last written
  and the output is 5V, as these routines correct the inversion.
*/
/* returns 0 on success, -1 on error */
int ppt_get_C(u8 *value)
{
  unsigned char inByte;

  __assert_ppt_fully_initialized(-1);

  inByte = _ppt_vt->in(_C_reg);
  inByte &= 0x0F;

  /* invert inverted outputs */
  inByte ^= 0xB; /* (1 + 2 + 8) */

  if (value != NULL)
    *value = inByte;

  return 0;
}

/*
  ppt_get_Cbit(int index) maps 4 index values 0-3 onto C0-C3, respectively

  Note that C0, C1, and C3 are inverted outputs; 'value' will return
  what was written to these, so checking a 1 means a 1 was last written
  and the output is 5V, as these routines correct the inversion.
*/
/* returns 0/1 on success, -1 on error */
int ppt_get_Cbit(int index)
{
  u8 inByte, mask;

  __assert_ppt_fully_initialized(-1);

  if ((index < 0) || (index > 3))
    return -1;

  if (ppt_get_C(&inByte) < 0)
    return -1;

  mask = (0x01 << index);

  return (inByte & mask) >> index;
}


/*
  ppt_set_D() maps D0-D7
*/
/* returns 0 on success, -1 on error */
int ppt_set_D(u8 value)
{
  __assert_ppt_fully_initialized(-1);

  if (_Ddir == regdir_in)
    return -1;

  _ppt_vt->out(value, _D_reg);
  return 0;
}


/*
  ppt_set_Dbit(int index, u8 value) maps D0-D7
*/
/* returns 0 on success, -1 on error */
int ppt_set_Dbit(int index, u8 value)
{
  u8 outByte, mask;

  __assert_ppt_fully_initialized(-1);

  if (_Ddir == regdir_in)
    return -1;

  if ((index < 0) || (index > 7))
    return -1;

  if (ppt_get_D(&outByte) < 0)
    return -1;

  mask = 0x01 << index;
  outByte = (outByte & ~mask) | ((value << index) & mask);

  return ppt_set_D(outByte);
}

/*
  ppt_set_C() maps C0-C3 onto 00001111, providing 4 digital outputs.

  Note that C0, C1, and C3 are inverted outputs, but we correct that here!
  */
/* returns 0 on success, -1 on error */
int ppt_set_C(u8 value)
{
  __assert_ppt_fully_initialized(-1);

  if (_Cdir == regdir_in)
    return -1;

  /* read current outputs */
  value = (value & 0x0F) | (_ppt_vt->in(_C_reg) & 0xF0);

  /* invert inverted outputs... */
  value ^= 0x0B; /*  (1 + 2 + 8) */

  _ppt_vt->out(value, _C_reg);
  return 0;
}


/*
  ppt_set_Cbit() maps C0-C3 onto 00001111, providing 4 digital outputs.

  Note that C0, C1, and C3 are inverted outputs, but we correct that here!
*/
/* returns 0 on success, -1 on error */
int ppt_set_Cbit(int index, u8 value)
{
  u8 outByte, mask;

  __assert_ppt_fully_initialized(-1);

  if (_Cdir == regdir_in)
    return -1;

  if ((index < 0) || (index > 3))
    return -1;

  if (ppt_get_C(&outByte) < 0)
    return -1;
 
  mask = 0x01 << index;
  outByte = (outByte & ~mask) | ((value << index) & mask);

  return ppt_set_C(value);
}




#ifdef MAIN

/* Used for testing the routines... */

static void fillIn(int *ins)
{
  int t;

  for (t = 0; t < 5; t++) {
    ins[t] = ppt_get_Sbit(3 + t);
  }
}

static void printIn(int *ins)
{
  int t;

  for (t = 0; t < 5; t++) {
    printf("%d ", ins[t]);
  }
  printf("\n");
}

static void fillOut(int *outs)
{
  int t;

  for (t = 0; t < 8; t++) {
    outs[t] = ppt_get_Dbit(t);
  }
}

static void printOut(int *outs)
{
  int t;

  for (t = 0; t < 8; t++) {
    printf("%d ", outs[t]);
  }
  printf("\n");
}

void print_help(void)
{
  printf("help/?           print this help\n");
  printf("s <#>            set bit #\n");
  printf("c <#>            clear bit #\n");
  printf("k                check outputs\n");
  printf("ENTER            print inputs\n");
  printf("q                quit\n");
}

int main(int argc, char *argv[])
{
  int t;
#define INPUTLEN 256
  char input[INPUTLEN];
  char cmd[INPUTLEN];
  int ins[5];
  int outs[8];
  int index;
  u32 base_addr;

  base_addr = DEFAULT_PARPORT_IO_ADDRESS;

  /* check -addr 0x378 */
  for (t = 1; t < argc; t++) {
    if (!strcmp(argv[t], "-addr")) {
      if (t == argc - 1) {
	fprintf(stderr, "no address specified\n");
	exit(1);
      } else if (1 != sscanf(argv[t + 1], "%i", &base_addr)) {
	fprintf(stderr, "bad address specified: %s\n", argv[t + 1]);
	exit(1);
      } else {
	t++;			/* step over address */
	continue;
      }
    } else {
      fprintf(stderr, "syntax: %s -addr <io address>\n", argv[0]);
      exit(1);
    }
  }

  if (ppt_init_dir(base_addr, regdir_out, regdir_out) < 0) {
    fprintf(stderr, "error initializing direct access to parallel port.\n");
    exit(1);
  }

  printf("initialized ppt module...\n");
  print_help();

  while (!feof(stdin)) {
    printf("ppt> ");
    fflush(stdout);

    printf("waiting for command...\n");
    if (NULL == fgets(input, INPUTLEN, stdin)) {
      /* EOF */
      break;
    }

    if (1 != sscanf(input, "%s", cmd)) {
      /* blank line-- print input */
      fillIn(ins);
      printIn(ins);
      continue;
    }

    if (!strcmp(cmd, "help") || !strcmp(cmd, "?")) {
      print_help();
    } else if (!strcmp(cmd, "s")) {
      /* set */
      if (1 != sscanf(input, "%*s %d", &index)) {
	printf("bad input: %s\n", input);
	continue;
      }

      if (0 != ppt_set_Dbit(index, 1)) {
	printf("index out of range\n");
      }
    } else if (!strcmp(cmd, "c")) {
      /* reset */
      if (1 != sscanf(input, "%*s %d", &index)) {
	printf("bad input: %s\n", input);
	continue;
      }

      if (0 != ppt_set_Dbit(index, 0)) {
	printf("index out of range\n");
      }
    } else if (!strcmp(cmd, "k")) {
      /* check outputs */
      fillOut(outs);
      printOut(outs);
    } else if (!strcmp(cmd, "q")) {
      break;
    } else {
      printf("?\n");
    }
  }

  ppt_done();

  exit(0);
}

#endif
