/* 
 * This code copyright (c) 2000 by Curt Wuollet, Wide Open Technologies  
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sched.h>
#include <plc.h>
#include "dio.h"

#define TRUE 1
#define BYTE unsigned char
#define WORD unsigned short
#define LONG unsigned long

/*
 * This is a quick-n-dirty conversion of Curt's DIO48H code to connect it
 * to the PLC.
 *
 * It is now configured from matplc.conf (default section "dio48").
 *
 * It reads two tables and seven variables.
 *
 * Tables:
 *     input <point> <port> <bit>
 *         Map a point as input. <port> is the channel number followed by
 *         A, B or C; <bit> is a single digit between 0 and 7.
 *
 *     output <point> <port> <bit>
 *         Map a point as output.
 *
 * Variables:
 *     file_prefix
 *         Specify the filename prefix. Default "/dev/dio48H_"
 *
 *     file_1A
 *     file_1B
 *     file_1C
 *     file_2A
 *     file_2B
 *     file_2C
 *         Specify the filenames to be used for the six ports individually.
 *         Defaults: <file_prefix> and <port> concatenated
 *
 *
 * Limitations:
 *     This module can only map each input to one PLC point.
 *
 *     The number of channels is hardcoded at 2.
 */

/*
 * An enumerated type for keeping track of which ports/halfports are INPUT
 * and which are OUTPUT. "unused" means that no points are mapped into that
 * port/halfport - unused ports are ioctl()'d to INPUT but not read.
 *
 * This is what other comments mean by "direction".
 */
typedef enum { unused, in, out } dir_t;

int fd_1A, fd_1B, fd_1C;
int fd_2A, fd_2B, fd_2C;

/* point handles */
plc_pt_t pt[48];

/* point names */
const char *ptname[48];

/* the direction of each port (1A, 1B, 1Cl, 1Ch, 2A, 2B, 2Cl, 2Ch) */
dir_t dirs[8];

/* filenames of the port files (1A, 1B, 1C, 2A, 2B, 2C) */
const char *fnames[6];


/* Forward Declarations */

void get_config(void);
int get_cfg_points(const char*, dir_t);
void get_cfg_fnames(void);
void dump_config(void);
void setup_io(void);
void setup_pts(void);
void close_io(void);
void do_io(dir_t);

int main(int argc,char *argv[])
{
    if (plc_init("dio48",argc,argv)<0) {
	printf("Error connecting to PLC.\n");
	exit(1);
    }
    get_config();
    setup_io();
    setup_pts();

    while (TRUE) {
	plc_scan_beg();

        do_io(in);
	plc_update();
        do_io(out);

	plc_scan_end();
        /*sched_yield(); *//* probably no longer required... */
    }
}

/* get everything from the config */
void get_config(void)
{
    int i, inputpts, outputpts;

    /* zero out the ptname[] and dirs[] arrays */
    for (i = 0; i < 48; i++)
	ptname[i] = NULL;
    for (i = 0; i < 8; i++)
	dirs[i] = unused;

    /* get the input and output mappings */
    inputpts = get_cfg_points("input", in);
    outputpts = get_cfg_points("output", out);

    if (inputpts + outputpts == 0) {
	printf("No inputs or outputs configured.\n");
	exit(1);
    }

    /* get the filenames */
    get_cfg_fnames();

    /* dump and exit (for debugging) */
    /* dump_config(); */
}

/* read the mapping for a particular direction (input or output) */
int get_cfg_points(const char* dirname, dir_t dir)
{
    int i, rows;
    char *port, *bit;

    /* how many mappings in this direction ? */
    rows = conffile_get_table_rows(dirname);

    for (i = 0; i < rows; i++) {
	if (conffile_get_table_rowlen(dirname, i) != 3) {
	    printf("Error - all %s specifications must have 3 fields\n",
		   dirname);
	    exit(1);
	}

	/* get port and bit from the 2nd and 3rd column */
	port = conffile_get_table(dirname, i, 1);
	bit = conffile_get_table(dirname, i, 2);

	/* check whether port and bit are valid */
	if (((port[0] == '1') || (port[0] == '2'))
	    && ((port[1] == 'A') || (port[1] == 'B') || (port[1] == 'C'))
	    && (port[2] == '\0')
	    && (bit[0] >= '0') && (bit[0] <= '7')
	    && (bit[1] == '\0')) {

	    /* valid specification, now what does it correspond to ? */
	    int ptnumber = (port[0]-'1')*24 + (port[1]-'A')*8 + bit[0]-'0';
	    int dirnumber = (port[0]-'1')*4 + port[1]-'A';
	    if ((port[1] == 'C') && (bit[0] > '3'))
		dirnumber++;

	    /* check for duplicates */
	    if (ptname[ptnumber] != NULL) {
	        char *newname = conffile_get_table(dirname, i, 0);
		if (strcmp(ptname[ptnumber], newname)) {
		    printf("Points %s and %s mapped to the same port/bit\n",
			   ptname[ptnumber], newname);
		    exit(1);
		}
		/* duplication is OK as long as it's identical */
		free(newname);
	    } else {
		/* OK, now record the name */
		ptname[ptnumber] = conffile_get_table(dirname, i, 0);
	    }

	    /* record the direction */
	    if (dirs[dirnumber] == unused)
		dirs[dirnumber] = dir;

	    /* check for port/halfport being both input and output */
	    if (dirs[dirnumber] != dir) {
		printf("Invalid combination of inputs and outputs\n");
		exit(1);
	    }
	} else {
	    printf("Invalid port/bit specification\n");
	    exit(1);
	}
	free(port);
	free(bit);
    }
    return rows;		/* how many points did we read */
}

/* concatenate two strings, malloc()ing the result */ 
static inline char *strdup2(const char *a, const char *b)
{
    char *res = malloc(strlen(a) + strlen(b) + 1);
    return strcat(strcpy(res, a), b);
}

/*
 * Either get the device filenames from the config or set them to the
 * default values.
 */
void get_cfg_fnames(void)
{
    /*            0123456 */
    char var[] = "file_XX";
    char *varvalue;
    const char *file_prefix="/dev/dio48H_";
    int i=0;

    /* get the file_prefix from the config, if it's there */
    varvalue = conffile_get_value("file_prefix");
    if (varvalue != NULL)
        file_prefix = varvalue;

    /* go through all the ports */
    for (var[5] = '1'; var[5] <= '2'; var[5]++) {
	for (var[6] = 'A'; var[6] <= 'C'; var[6]++) {

	    varvalue = conffile_get_value(var);
	    if (varvalue != NULL)
	        /* use value from config */
		fnames[i] = varvalue;
	    else
	        /* construct the default value */
	        fnames[i] = strdup2(file_prefix, var+5);
	    i++;
	}
    }
    /* memory leak: file_prefix, if set from the config */
}

/* for debugging - dump the config and exit */
void dump_config(void)
{
    int i;
    printf("Point names:");
    for (i = 0; i < 48; i++)
	printf(" %s%s", i % 8 ? "" : "\n    ", ptname[i]);
    printf("\nDirections:");
    for (i = 0; i < 8; i++)
	printf(" %d", dirs[i]);
    printf("\nFilenames:\n");
    for (i = 0; i < 6; i++)
	printf("    %s\n", fnames[i]);

    exit(0);
}

static inline int setup_io_byte(const char*fname)
{
    int fd;
    if ((fd = open(fname, O_RDWR )) < 0) 
    {
        perror(fname);
        printf("error opening device %s\n", fname);
        exit(2);
    }
    return fd;
}

void setup_io(void)
{
    fd_1A = setup_io_byte(fnames[0]);
    fd_1B = setup_io_byte(fnames[1]);
    fd_1C = setup_io_byte(fnames[2]);
    fd_2A = setup_io_byte(fnames[3]);
    fd_2B = setup_io_byte(fnames[4]);
    fd_2C = setup_io_byte(fnames[5]);

    ioctl(fd_1A, DIO_SET_DIRECTION,
	  dirs[0] == out ? PORT_OUTPUT : PORT_INPUT);
    ioctl(fd_1B, DIO_SET_DIRECTION,
	  dirs[1] == out ? PORT_OUTPUT : PORT_INPUT);
    ioctl(fd_1C, DIO_SET_DIRECTION,
	  dirs[2] == out ? LOW_PORT_OUTPUT : LOW_PORT_INPUT);
    ioctl(fd_1C, DIO_SET_DIRECTION,
	  dirs[3] == out ? HIGH_PORT_OUTPUT : HIGH_PORT_INPUT);
    ioctl(fd_2A, DIO_SET_DIRECTION,
	  dirs[4] == out ? PORT_OUTPUT : PORT_INPUT);
    ioctl(fd_2B, DIO_SET_DIRECTION,
	  dirs[5] == out ? PORT_OUTPUT : PORT_INPUT);
    ioctl(fd_2C, DIO_SET_DIRECTION,
	  dirs[6] == out ? LOW_PORT_OUTPUT : LOW_PORT_INPUT);
    ioctl(fd_2C, DIO_SET_DIRECTION,
	  dirs[7] == out ? HIGH_PORT_OUTPUT : HIGH_PORT_INPUT);
}

void setup_pts(void)
{
  int i;

  for(i=0;i<48;i++)
  {
    if (ptname[i]) {
      pt[i]=plc_pt_by_name(ptname[i]);
      if (!pt[i].valid)
      {
	printf("Couldn't get handle to point %s.\n",ptname[i]);
	exit(1);
      }
    } else {
      /* not mapped to anything - so note in the array */
      pt[i].valid=0;
    }
  }
}

void close_io(void)
{
    close(fd_1A);
    close(fd_1B);
    close(fd_1C);
    close(fd_2A);
    close(fd_2B);
    close(fd_2C);
}

static inline void write_io_byte(int fd, plc_pt_t *handles)
{
  BYTE buf=0; int i;
  for(i=0;i<8;i++) {
    buf <<= 1;
    if (handles[i].valid)
      buf |= plc_get(handles[i]);
  }
  write(fd,&buf,1) ;
}

static inline void read_io_byte(int fd, plc_pt_t *handles)
{
  BYTE buf=0; int i;
  read(fd_2A,&buf,1) ;
  for(i=0;i<8;i++) {
    if (handles[i].valid)
      plc_set(handles[i], buf&1);
    buf >>= 1;
  }
}

static inline void do_io_byte(dir_t dir, int fd, plc_pt_t * handles)
{
    if (dir == out)
	write_io_byte(fd, handles);
    else if (dir == in)
	read_io_byte(fd, handles);
    /* do nothing when dir == unused */
}

/* do all the I/O in a particular direction */
void do_io(dir_t dir)
{
    /* these are the simple ones */
    if (dir==dirs[0])
        do_io_byte(dir,fd_1A, pt);
    if (dir==dirs[1])
        do_io_byte(dir,fd_1B, pt + 8);

    /*
     * I (Jiri) am not sure whether this is the correct way to do it. If
     * the C port is mixed, is it right to both write and read it?
     */
    if ((dir == out) && ((dirs[2] == out) || (dirs[3] == out)))
	write_io_byte(fd_1C, pt + 16);
    if ((dir == in) && ((dirs[2] == in) || (dirs[3] == in)))
	read_io_byte(fd_1C, pt + 16);

    /* now the same thing for the other three ports */
    if (dir==dirs[4])
        do_io_byte(dir,fd_2A, pt+24);
    if (dir==dirs[5])
        do_io_byte(dir,fd_2B, pt + 32);
    if ((dir == out) && ((dirs[6] == out) || (dirs[7] == out)))
	write_io_byte(fd_2C, pt + 40);
    if ((dir == in) && ((dirs[6] == in) || (dirs[7] == in)))
	read_io_byte(fd_2C, pt + 40);
}
