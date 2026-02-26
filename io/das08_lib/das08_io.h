//
//    Copyright (C) 2000-1 by Hugh Jack <jackh@gvsu.edu>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//
// Last Modified: April 22, 2002
//

#include <global.h>


#ifndef _DAS08__
#define _DAS08__

	#define	CARDBASE	0x300
	
	#define ADCHIGH		0	// AD Data Registers
	#define ADCLOW		1

	/* A/D Status and Control Register */
	#define ADCSTATUS	2

	/* Auxiliary port on analog bus */
	#define PORTAUX         2

	/* Programmable Gain Register */
	#define GAIN		3

	/* Counter Load & Read Registers */
	#define LOADREAD1	4
	#define LOADREAD2	5
	#define LOADREAD3	6

	#define CCONFIGPORT	7	// Counter Control Register

	/* D/A 0 Control Registers */
	#define DAC0LOW		8
	#define DAC0HIGH	9

	/* D/A 1 Control Registers */
	#define DAC1LOW		10
	#define DAC1HIGH	11

	/* 82C55 Digital I/O Registers */
	#define PORTA		12
	#define PORTB		13
	#define PORTC		14
	#define PORTCL		12345	/* real port is 0x30e bits 0-3 */
	#define PORTCH		6789	/* real port is 0x30e bits 4-7 */

	/* 82C55 Control Register */
	#define DCONFIGPORT	15

	#define DIGITALOUT      1
	#define DIGITALIN       2
	#define HIGHONLASTCOUNT 0
	#define ONESHOT		1
	#define RATEGENERATOR	2
	#define SQUAREWAVE	3
	#define SOFTWARESTROBE	4
	#define HARDWARESTROBE	5

	/* Range Codes */
	#define BIP10VOLTS	0x08
	#define BIP5VOLTS	0x00
	#define BIP2PT5VOLTS	0x02
	#define BIP1PT25VOLTS	0x04
	#define BIPPT625VOLTS	0x06
	#define UNI10VOLTS	0x01
	#define UNI5VOLTS	0x03
	#define UNI2PT5VOLTS	0x05
	#define UNI1PT25VOLTS	0x07



	class das08{
		protected:
		public:
			int		base;	// card setup information
			int		chan0;
			int		chan1;

			int		portA;	// port data directions
			int		portB;
			int		portCL;
			int		portCH;

			int		data_portA_IN;	// hooks to global values
			int		data_portA_OUT;
			int		data_portB_IN;
			int		data_portB_OUT;
			int		data_portCL_IN;
			int		data_portCL_OUT;
			int		data_portCH_IN;
			int		data_portCH_OUT;
			int		data_portXI;
			int		data_portXO;
			int		data_AI0;
			int		data_AI1;
			int		data_AI2;
			int		data_AI3;
			int		data_AI4;
			int		data_AI5;
			int		data_AI6;
			int		data_AI7;
			int		data_AO0;
			int		data_AO1;

					das08();
					~das08();

			int		configure(char*);
			int		connect();
			int		scan();
			int		disconnect();
			int		DConfigPort(int, int);
			int		DIn(int, int*);
			int		DBitIn(int, int, int*);
			int		DOut(int, int);
			int		DBitOut(int, int, int);

			int		C8254Config(int, int);
			int		CLoad(int, int);
			int		CIn(int, int*);

			int		AIn(int, int*);
			int		AOut(int, int);
	};

#endif
