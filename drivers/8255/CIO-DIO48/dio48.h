/*-------------------------------------------------------*/
/* dio48.h			    			 */
/*							 */
/* Written April 1997 for CIO-DIO48 board by Tim Edwards */
/*-------------------------------------------------------*/

/* ioctl() calls */

#define DIO_SET_MODE	   1
#define DIO_GET_MODE	   2
#define DIO_SETA	   3
#define DIO_SETB	   4
#define DIO_SETC	   5
#define DIO_GETA	   6
#define DIO_GETB	   7
#define DIO_GETC	   8

/*---------------------------------------------------------------*/
/* Control Register values for use with DIO_SET_MODE ioctl call. */
/*---------------------------------------------------------------*/

#define OUTPUT		0x0

/* input modes */

#define CNTL_A		0x10
#define CNTL_B		0x02
#define CNTL_CL		0x01
#define CNTL_CH		0x08
#define CNTL_C		0x09	/* CNTL_C = CNTL_CL | CNTL_CH */
#define MODE_SET	0x80	/* set by driver---shouldn't be needed */

/*--------------------------------------------------------------*/
/* 82C55 modes 							*/
/* GRPA controls mode of ports A and CU 			*/
/* GRPB controls mode of ports B and CL 			*/
/* Default setting on opening the device is Mode 0 (Normal I/O) */
/*      for both groups 					*/
/*--------------------------------------------------------------*/

#define GRPA_MODE0 0x0		/* Normal I/O 		*/
#define GRPA_MODE1 0x20		/* Strobed I/O 		*/
#define GRPA_MODE2 0x40		/* Bi-directional Bus 	*/
#define GRPB_MODE0 0x0		/* Normal I/O 		*/
#define GRPB_MODE1 0x04		/* Strobed I/O		*/
