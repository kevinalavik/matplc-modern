
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


#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>

#include "das08_io.h"
//#include "../include/process.h"
//#include <module_library.h>
#include <global.h>


int	bits[]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};




das08::das08(){
	base = CARDBASE;	// default cardbase
	chan0 = BIP10VOLTS;	// default AD ranges
	chan1 = BIP10VOLTS;
	portA = DIGITALIN;
	portB = DIGITALIN;
	portCH = DIGITALIN;
	portCL = DIGITALIN;

	data_portA_OUT = 0;
	data_portB_OUT = 0;
	data_portCL_OUT = 0;
	data_portCH_OUT = 0;
	data_portXO = 0;
	data_AO0 = 0;
	data_AO1 = 0;
}



das08::~das08(){
}


int das08::configure(char *file_name){
	int	error;
	FILE	*fp_in;
	char	params[200];

	error = NO_ERROR;
	if((fp_in = fopen(file_name, "r")) != NULL){
		fgets(params, 200, fp_in);
		while(feof(fp_in) == 0){
			if((params[0] != '#') && (strlen(params) > 3)){
				if(params[0] == 'B'){
					base = atoi(&(params[1]));
				} else if(strncmp("A0", params, 2) == 0){
					if(strncmp("BIP10VOLTS", &(params[2]), 10) == 0){ chan0 = BIP10VOLTS;
					} else if(strncmp("BIP5VOLTS", &(params[2]), 9) == 0){ chan0 = BIP5VOLTS;
					} else if(strncmp("BIP2PT5VOLTS", &(params[2]), 12) == 0){ chan0 = BIP2PT5VOLTS;
					} else if(strncmp("BIP1PT25VOLTS", &(params[2]), 13) == 0){ chan0 = BIP1PT25VOLTS;
					} else if(strncmp("BIPPT625VOLTS", &(params[2]), 13) == 0){ chan0 = BIPPT625VOLTS;
					} else if(strncmp("UNI10VOLTS", &(params[2]), 10) == 0){ chan0 = UNI10VOLTS;
					} else if(strncmp("UNI5VOLTS", &(params[2]), 9) == 0){ chan0 = UNI5VOLTS;
					} else if(strncmp("UNI2PT5VOLTS", &(params[2]), 12) == 0){ chan0 = UNI2PT5VOLTS;
					} else if(strncmp("UNI1PT25VOLTS", &(params[2]), 13) == 0){ chan0 = UNI1PT25VOLTS;
					} else {
						error_log(MINOR, "Unrecognized DAS08 analog A0 output range");
						error = ERROR;
					}
				} else if(strncmp("A1", params, 2) == 0){
					if(strncmp("BIP10VOLTS", &(params[2]), 10) == 0){ chan1 = BIP10VOLTS;
					} else if(strncmp("BIP5VOLTS", &(params[2]), 9) == 0){ chan1 = BIP5VOLTS;
					} else if(strncmp("BIP2PT5VOLTS", &(params[2]), 12) == 0){ chan1 = BIP2PT5VOLTS;
					} else if(strncmp("BIP1PT25VOLTS", &(params[2]), 13) == 0){ chan1 = BIP1PT25VOLTS;
					} else if(strncmp("BIPPT625VOLTS", &(params[2]), 13) == 0){ chan1 = BIPPT625VOLTS;
					} else if(strncmp("UNI10VOLTS", &(params[2]), 10) == 0){ chan1 = UNI10VOLTS;
					} else if(strncmp("UNI5VOLTS", &(params[2]), 9) == 0){ chan1 = UNI5VOLTS;
					} else if(strncmp("UNI2PT5VOLTS", &(params[2]), 12) == 0){ chan1 = UNI2PT5VOLTS;
					} else if(strncmp("UNI1PT25VOLTS", &(params[2]), 13) == 0){ chan1 = UNI1PT25VOLTS;
					} else {
						error_log(MINOR, "Unrecognized DAS08 analog A1 output range");
						error = ERROR;
					}
				} else if(strncmp("PAI", params, 3) == 0){ portA = DIGITALIN;
				} else if(strncmp("PAO", params, 3) == 0){ portA = DIGITALOUT;
				} else if(strncmp("PBI", params, 3) == 0){ portB = DIGITALIN;
				} else if(strncmp("PBO", params, 3) == 0){ portB = DIGITALOUT;
				} else if(strncmp("PCLI", params, 4) == 0){ portCL = DIGITALIN;
				} else if(strncmp("PCLO", params, 4) == 0){ portCL = DIGITALOUT;
				} else if(strncmp("PCHI", params, 4) == 0){ portCH = DIGITALIN;
				} else if(strncmp("PCHO", params, 4) == 0){ portCH = DIGITALOUT;
				} else if(strncmp("PCO", params, 3) == 0){ portCH = DIGITALOUT; portCL = DIGITALOUT;
				} else if(strncmp("PCI", params, 3) == 0){ portCH = DIGITALIN; portCL = DIGITALIN;
				} else {
					error_log(MINOR, "DAS08 argument not recognized");
					error = ERROR;
				}
			}
			fgets(params, 200, fp_in);
		}
		fclose(fp_in);
	}

	return error;
}



int das08::connect(){
	int	error;

	error = NO_ERROR;
	if(ioperm(base, 16, 1) == 0){
		DConfigPort(PORTA, portA);
		DConfigPort(PORTB, portB);
		DConfigPort(PORTCL, portCL);
		DConfigPort(PORTCH, portCH);
	} else {
		error = ERROR;
		error_log(MINOR, "Could not connect to DAS08 board - memory is probably in use");
	}

	return error;
}



int das08::scan(){
	int	error;

	error = NO_ERROR;
	// update digital ports
	if(portA == DIGITALIN){DIn(PORTA, &data_portA_IN);
	} else {DOut(PORTA, data_portA_OUT);}
	if(portB == DIGITALIN){DIn(PORTB, &data_portB_IN);
	} else {DOut(PORTB, data_portB_OUT);}
	if(portCL == DIGITALIN){DIn(PORTCL, &data_portCL_IN);
	} else {DOut(PORTCL, data_portCL_OUT);}
	if(portCH == DIGITALIN){DIn(PORTCH, &data_portCH_IN);
	} else {DOut(PORTCH, data_portCH_OUT);}
	DOut(PORTAUX, data_portXO);
	DIn(PORTAUX, &data_portXI);


	// Update analog inputs
	AIn(0, &data_AI0);
	AIn(1, &data_AI1);
	AIn(2, &data_AI2);
	AIn(3, &data_AI3);
	AIn(4, &data_AI4);
	AIn(5, &data_AI5);
	AIn(6, &data_AI6);
	AIn(7, &data_AI7);
	// Update analog outputs
	AOut(0, data_AO0);
	AOut(1, data_AO1);

	return error;
}


int das08::disconnect(){
	int	error;

	error = NO_ERROR;
	if(ioperm(base, 16, 0) != 0){
		error = ERROR;
		error_log(MINOR, "Could not release the DAS08 board - memory is probably in use");
	}

	return error;
}



int das08::DConfigPort(int Port, int Direction){
	//  This command configures a port as an input or output.
	//  The Direction field can be either DIGITALIN or DIGITALOUT
	//  depending on whether the port is to be configured as an
	//  input or output.  Valid ports are PORTA, PORTB, PORTCL and
	//  PORTCH.  Direction bit can be either DIGITALIN or DIGITALOUT.
	int	error,
		mask, OldByte, NewByte;

	//printf("Configuring port %d with direction %d \n", Port, Direction);
	error = NO_ERROR;
	OldByte = inb(DCONFIGPORT + base); /*read the current register*/
	if(Direction == DIGITALIN){ /* determine mask for DIGITALIN */
			if(Port == PORTA){ mask = 0x10;
			} else if(Port == PORTB){ mask = 0x02;
			} else if(Port == PORTC){ mask = 0x09;
			} else if(Port == PORTCL){ mask = 0x01; Port = PORTC;
			} else if(Port == PORTCH){ mask = 0x08; Port = PORTC;
			} else {
				error_log(MINOR, "Digital port must be PORTA, PORTB, PORTC, PORTCL or PORTCH");
				error = ERROR;
				mask = 0;
			}
			NewByte = OldByte | mask; /* new data for register */
	} else if(Direction == DIGITALOUT){ /* determine mask for DIGITALOUT */
			if(Port == PORTA){ mask = 0xef;
			} else if(Port == PORTB){ mask = 0xfd;
			} else if(Port == PORTC){ mask = 0xf6;
			} else if(Port == PORTCL){ mask = 0xfe; Port = PORTC;
			} else if(Port == PORTCH){ mask = 0xf7; Port = PORTC;
			} else {
				error_log(MINOR, "Digital port must be PORTA, PORTB, PORTC, PORTCL or PORTCH");
				error = ERROR;
			}
			NewByte = OldByte & mask; /* new value for register */
	} else {
			error_log(MINOR, "Direction must be set to DIGITALIN or DIGITALOUT");
			error = ERROR;
	}
	if(error == NO_ERROR){
	//printf("port thingy %d  %d  \n", NewByte, DCONFIGPORT);
		outb(NewByte, DCONFIGPORT + base); /* write config data to register */
	}

	return error; /* no errors detected */
}





int das08::DBitIn(int Port, int BitNum, int *BitData){
	//  This function determines whether a bit within the
	//  requested port is set.  The value (1 or 0) is returned
	//  in the variable pointer sent to the function.  Port may
	//  be PORTA, PORTB, PORTCL or PORTCH.  BitNum must be in the
	//  range 0-7.
	int	error,
		mask = 0, data;

	error = NO_ERROR;
	if((Port == PORTCL) || (Port == PORTCH)){ data = inb(PORTC + base);
	} else { data = inb(Port + base);}
	//printf("GOT   %d   %d  %d   %d  \n", Port, data, BitNum, BitData[0]);
	if((Port == PORTA) || (Port == PORTB) || (Port == PORTC)){
		if((BitNum >= 0) && (BitNum <= 7)){
			mask = bits[BitNum];
		} else {
			error_log(MINOR, "Bit numbers should be between 0 and 7");
			error = ERROR;
		}
	} else if((Port == PORTCL) || (Port == PORTAUX)) {
		if((BitNum >= 0) && (BitNum <= 3)){
			mask = bits[BitNum];
		} else {
			error_log(MINOR, "Bit numbers should be between 0 and 3");
			error = ERROR;
		}
	} else if(Port == PORTCH) {
		if((BitNum >= 4) && (BitNum <= 7)){
			mask = bits[BitNum];
		} else {
			error_log(MINOR, "Bit numbers should be between 4 and 7");
			error = ERROR;
		}
	} else if(Port == DCONFIGPORT) {
		mask = bits[BitNum];
	} else {
		error_log(MINOR, "Input port not recognized");
		error = ERROR;
	}

	if(error == NO_ERROR){
		BitData[0] = 0;
		if((mask & data) != 0) BitData[0] = 1;
	}

	return error;
}



int das08::DBitOut(int Port, int BitNum, int BitValue){
	//  This function sets a bit of the requested port to either
	//  a zero or a one.  Port may be PORTA, PORTB, PORTCL or
	//  PORTCH.  BitNum must be in the range 0 - 7.  BitValue
	//  must be 1 or 0.
	int	error,
		mask, NewByte, OldByte;

	error = NO_ERROR;
	if((Port == PORTCL) || (Port == PORTCH)){
		OldByte = inb(PORTC + base);
	} else {
		OldByte = inb(Port + base);
	}

	if((Port == PORTAUX) && (BitValue == 1)){
		mask = bits[BitNum+4];
		NewByte = OldByte | mask;
		//printf("ddo  %x   %x \n", mask, OldByte);
	} else if((Port == PORTAUX) && (BitValue == 0)) {
		mask = bits[BitNum+4];
		NewByte = OldByte & ~mask;
    	} else if(((Port==PORTA) || (Port==PORTB) || (Port == PORTC)) && (BitValue==1)){
		mask = bits[BitNum];
		NewByte = OldByte | mask;
	}else if(((Port==PORTA) || (Port==PORTB) || (Port==PORTC)) && (BitValue == 0)){
		mask = bits[BitNum];
		NewByte = OldByte & ~mask;
	} else if((Port == PORTCL) && (BitValue == 1)){
		mask = bits[BitNum];
		NewByte = OldByte | mask;
	} else if((Port == PORTCL) && (BitValue == 0)){
		mask = bits[BitNum];
		NewByte = OldByte & ~mask;
	} else if((Port == PORTCH) && (BitValue == 1)){
		mask = bits[BitNum];
		NewByte = OldByte | mask;
	} else if((Port == PORTCH) && (BitValue == 0)){
		mask = bits[BitNum];
		NewByte = OldByte & ~mask;
	} else {
		error = ERROR;
	}
	if((Port == PORTCL) || (Port == PORTCH))
		Port = PORTC;
		//printf("OUT %d %d\n", NewByte, Port + base);
	if(error == NO_ERROR) outb(NewByte, Port + base);
    
    	return error;
}




int das08::DIn(int Port, int *Value){
	//  This function reads the byte value of the specified port
	//  and returns the result in the variable pointer sent to the
	//  function.  Valid ports are PORTA, PORTB, PORTCL and PORTCH.
	int	error;
//	int	result;
//	int	BitData;
	int	temp;

	error = NO_ERROR;
//	if(Port == PORTA){
//		result = DBitIn(DCONFIGPORT, 4, &BitData);
//	} else if(Port == PORTB){
//		result = DBitIn(DCONFIGPORT, 1, &BitData);
//	} else if(Port == PORTC){
//		result = DBitIn(DCONFIGPORT, 0, &BitData)
//				+ DBitIn(DCONFIGPORT, 3, &BitData);
//	} else if(Port == PORTCL){
//		result = DBitIn(DCONFIGPORT, 0, &BitData);
//	} else if(Port == PORTCH){
//		result = DBitIn(DCONFIGPORT, 3, &BitData);
//	} else if(Port == PORTAUX){
//	} else {
//		error_log(MINOR, "ERROR: Port not recognized");
//		error = ERROR;
//	}

//////////////
//printf("sss %d  %d \n", Port, result);
//	if((error == NO_ERROR) && (BitData == 0)){
//		error_log("ERROR: Port not configured for read");
//		error = ERROR;
//	}
	if(error == NO_ERROR){
		if(Port == PORTCL){
			temp = inb(PORTC + base);	/* read the port data */
			Value[0] = (temp & 0x0f);	/* mask off the high bits */
		} else if(Port == PORTCH){
			temp = inb(PORTC + base);	/* read the port data */
			Value[0] = (temp & 0xf0);	/* mask off the low bits */
		} else if(Port == PORTAUX){
			Value[0] = 0x7 & (int)((inb(Port + base) / 16));
		} else {
			Value[0] = 0xff & inb(Port + base);	/* read the port data */
		}
	}

	return error;
}


	

int das08::DOut(int Port, int ByteValue){
	//  This function writes the byte value to the specified port.
	//  Valid ports are PORTA, PORTB, PORTCL and PORTCH.
	int	error;
	
	error = NO_ERROR;
	if(Port == PORTAUX){
		ByteValue = /* (0xf0 & inb(Port+base)) | */ 0xf0 & (ByteValue * 16);
	}
	if((ByteValue > 255) || (ByteValue < 0)){
		error = ERROR;
	}
	//printf("Writing byte %d to port %d\n", ByteValue, Port);
	if(error == NO_ERROR){
		if(Port == PORTCL){
			outb((ByteValue & 0x0f), PORTC + base);
		} else if(Port == PORTCH){
			outb((ByteValue & 0xf0), PORTC + base);
		} else {
			outb(ByteValue, Port + base); /* write the port data */
		}
	}

	return error; /* no errors detected */
}



int das08::C8254Config(int CounterNum, int Config){
	int	error,
		NewByte,
//		TempByte,
		BCD, mask, counter;
//	int	temp;	

	error = NO_ERROR;
	/* BCD = 0xfe - 16-bit binary count
	   BCD = 0xf1 - 4 decade Binary Coded Decimal */
	BCD = 0xfe;
	switch (Config){
		case HIGHONLASTCOUNT:	mask = 0xf1; break;
		case ONESHOT:			mask = 0xf3; break;
		case RATEGENERATOR:		mask = 0xf5; break;
		case SQUAREWAVE:		mask = 0xf7; break;
		case SOFTWARESTROBE:	mask = 0xf9; break;
		case HARDWARESTROBE:	mask = 0xfb; break;
		default: error = ERROR;; break;
	}

	switch (CounterNum){
		case 1: counter = 0x3f; break;
		case 2: counter = 0x7f; break;
		case 3: counter = 0xbf; break;
		default: error = ERROR; break;
	}
	if(error == NO_ERROR){
		NewByte = (BCD & mask) & counter;
		//printf("The value of TempByte & mask is --> %x.\n", NewByte);
		outb(NewByte, CCONFIGPORT + base);
	}

	return error;
}



int das08::CLoad(int CounterNum, int value)
{
	char	LoadValue[6];
	int	error;
	int TempByte, TempByte1, Register, CounterMask;
	int WriteLowByteMask1 = 0x20;	/* RL1 | */
	int WriteLowByteMask2 = 0xef;	/* RL0 & */
	int WriteHighByteMask1 = 0xdf;	/* RL1 & */
	int WriteHighByteMask2 = 0x10;	/* RL0 | */
	char LowByte[5];
	char HighByte[5];
	long HighByteValue, LowByteValue;
	int test;

	error = NO_ERROR;
	switch (CounterNum){
		case 1: Register = LOADREAD1; CounterMask = 0x3f; break;
		case 2: Register = LOADREAD2; CounterMask = 0x7f; break;
		case 3: Register = LOADREAD3; CounterMask = 0xbf; break;
		default: error = ERROR; break;
	}

	HighByte[0] = LoadValue[0];
	HighByte[1] = LoadValue[1];
	HighByte[2] = LoadValue[2];
	HighByte[3] = LoadValue[3];

	LowByte[0] = '0';
	LowByte[1] = 'x';
	LowByte[2] = LoadValue[4];
	LowByte[3] = LoadValue[5];

	if(error == NO_ERROR){
		HighByteValue = (int)strtol(HighByte, NULL, 0);
		LowByteValue = (int)strtol(LowByte, NULL, 0);
		TempByte = (CounterMask | WriteLowByteMask1) & WriteLowByteMask2;
		TempByte1 = TempByte & 0xf0;
		//printf("The value in config low is --> %x.\n", TempByte1);
		outb(TempByte1, CCONFIGPORT + base);
		outb(LowByteValue, Register + base);
		//printf("The register chosen is --> %x.\n", Register);
		test = inb(Register + base);
		//printf("The value read in counter low is --> %x.\n", test);
		TempByte = (0x30 & WriteHighByteMask1) | WriteHighByteMask2;
		//printf("The value in config high is --> %x.\n", TempByte);
		outb(TempByte, CCONFIGPORT + base);
		outb(HighByteValue, Register + base);
		outb(TempByte, CCONFIGPORT + base);
		test = inb(Register + base);
		//printf("The value in counter high is --> %x.\n", test);
	}

	return error;
}	


int das08::CIn(int CounterNum, int *CountValue){
	int	error;
	int TempByte, Register;
	int ReadLowByteMask1 = 0x20;	/* RL1 | */
	int ReadLowByteMask2 = 0xef;	/* RL0 & */
	int ReadHighByteMask1 = 0xdf;	/* RL1 & */
	int ReadHighByteMask2 = 0x10;	/* RL0 | */
	int CountValue1, CountValue2;	
		
	error = NO_ERROR;
	switch (CounterNum){
		case 1: Register = LOADREAD1; break;
		case 2: Register = LOADREAD2; break;
		case 3: Register = LOADREAD3; break;
		default: error = ERROR; break;
	}

	if(error == NO_ERROR){
		TempByte = (0x3f | ReadLowByteMask1) & ReadLowByteMask2;
		outb(TempByte, CCONFIGPORT + base);
		CountValue1 = inb(Register + base);
		//printf("The low value is --> %x.\n", CountValue1);
		TempByte = (0x3f & ReadHighByteMask1) | ReadHighByteMask2;
		outb(TempByte, CCONFIGPORT + base);
		CountValue2 = inb(Register + base);
		//printf("The high value is --> %x.\n", CountValue2);
	}

	return error;
}





int das08::AIn(int ADChannel, int *Value){
	//  This function requires three arguments to perform the
	//  analog to digital conversion.  ADChannel must be in the
	//  range 0-7 and Range must be a valid range code 
	//  i.e. BIP5VOLTS.  The value of the conversion will be
	//  returned to the address specificed through the pointer
	//  variable.  This value will be in the range 0-4095.
	int	error;
	int value1, value2, value3, curr_status, new_status, ADbusy;
	int ADCmask1, ADCmask2;
	int ADValue_low, ADValue_low1, ADValue_low2, ADValue_high;
	int EOC = 1;

	error = NO_ERROR;
	curr_status = inb(ADCSTATUS + base); /* current value in status */

	switch(ADChannel){
		case 0:ADCmask1 = 0xf8;ADCmask2 = 0x00;break;
		case 1:ADCmask1 = 0xf9;ADCmask2 = 0x01;break;
		case 2:ADCmask1 = 0xfa;ADCmask2 = 0x02;break;
		case 3:ADCmask1 = 0xfb;ADCmask2 = 0x03;break;
		case 4:ADCmask1 = 0xfc;ADCmask2 = 0x04;break;
		case 5:ADCmask1 = 0xfd;ADCmask2 = 0x05;break;
		case 6:ADCmask1 = 0xfe;ADCmask2 = 0x06;break;
		case 7:ADCmask1 = 0xff;ADCmask2 = 0x07;break;
		default:error = ERROR;; break; /* error */
	}

	if(error == NO_ERROR){
		outb(chan0, GAIN + base); /* set the gain/range value */
		new_status = (curr_status & ADCmask1) | ADCmask2;
		outb(new_status, ADCSTATUS + base); /* set the channel number */
		outb(0x00, ADCLOW + base); /* start a 12 bit A/D conversion */
	}

	while((error == NO_ERROR) && (EOC == 1)){ /* check for end of conversion */
		ADbusy = inb(ADCSTATUS + base); /* read status register */
		if(ADbusy >= 128){
			EOC = 1; /* A/D still converting */
		} else {
			EOC = 0; /* A/D done converting */
		}
	}

	if(error == NO_ERROR){
		ADValue_low = inb(ADCLOW + base); /* get the lower eight bits */
		ADValue_high = inb(ADCHIGH + base); /* get the upper four bits */

		switch(ADValue_high){
			case 0x00:value1 = 0;break;
			case 0x80:value1 = 1;break;
			case 0x40:value1 = 2;break;
			case 0xc0:value1 = 3;break;
			case 0x20:value1 = 4;break;
			case 0xa0:value1 = 5;break;
			case 0x60:value1 = 6;break;
			case 0xe0:value1 = 7;break;
			case 0x10:value1 = 8;break;
			case 0x90:value1 = 9;break;
			case 0x50:value1 = 10;break;
			case 0xd0:value1 = 11;break;
			case 0x30:value1 = 12;break;
			case 0xb0:value1 = 13;break;
			case 0x70:value1 = 14;break;
			case 0xf0:value1 = 15;break;
			default:error = ERROR;break;
		}

		ADValue_low1 = (ADValue_low & 0x0f); /* mask off bits 4-7 */
		switch(ADValue_low1){
			case 0x00:value2 = 0;break;
			case 0x01:value2 = 16;break;
			case 0x02:value2 = 32;break;
			case 0x03:value2 = 48;break;
			case 0x04:value2 = 64;break;
			case 0x05:value2 = 80;break;
			case 0x06:value2 = 96;break;
			case 0x07:value2 = 112;break;
			case 0x08:value2 = 128;break;
			case 0x09:value2 = 144;break;
			case 0x0a:value2 = 160;break;
			case 0x0b:value2 = 176;break;
			case 0x0c:value2 = 192;break;
			case 0x0d:value2 = 208;break;
			case 0x0e:value2 = 224;break;
			case 0x0f:value2 = 240;break;
			default:error = ERROR;break;
		}
		ADValue_low2 = (ADValue_low & 0xf0); /* mask off bits 0-3 */
		switch(ADValue_low2){
			case 0x00:value3 = 0;break;
			case 0x10:value3 = 256;break;
			case 0x20:value3 = 512;break;
			case 0x30:value3 = 768;break;
			case 0x40:value3 = 1024;break;
			case 0x50:value3 = 1280;break;
			case 0x60:value3 = 1536;break;
			case 0x70:value3 = 1792;break;
			case 0x80:value3 = 2048;break;
			case 0x90:value3 = 2304;break;
			case 0xa0:value3 = 2560;break;
			case 0xb0:value3 = 2816;break;
			case 0xc0:value3 = 3072;break;
			case 0xd0:value3 = 3328;break;
			case 0xe0:value3 = 3584;break;
			case 0xf0:value3 = 3840;break;
			default: error = ERROR; /* error - unknown conversion result */
		}
		*Value = value1+value2+value3; /* total value for conversion */ 
	}

	return error; /* no errors detected */
}






int das08::AOut(int DAChannel, int DAValue){
	//  This function performs a digital to analog conversion
	//  routine.  The DAChannel must be either 0 or 1 and the
	//  digital value must be in the range 0-4095.
	int	error;
	int low, high, DACLOW, DACHIGH;

	error = NO_ERROR;
	switch(DAChannel){
		case 0:DACLOW = DAC0LOW;DACHIGH = DAC0HIGH;break;
		case 1:DACLOW = DAC1LOW;DACHIGH = DAC1HIGH;break;
		default:error = ERROR;break;
	}

	/* The following table converts the digital value into
	   three hex values encompassing two 8-bit registers.  The
	   layout of the registers follow:

		low  - DA7  DA6  DA5  DA4  DA3  DA2  DA1  DA0
		high -   x    x    x    x  DA11 DA10 DA9  DA8 */

	if(DAValue <= 255){
		low = DAValue;
		high = 0x00;
	} else if((DAValue >= 256) && (DAValue <= 511)){
		low = DAValue - 256;
		high = 0x01;
	} else if((DAValue >= 512) && (DAValue <= 767)) {
		low = DAValue - 512;
		high = 0x02;
	} else if((DAValue >= 768) && (DAValue <= 1023)) {
		low = DAValue - 768;
		high = 0x03;
	} else if((DAValue >= 1024) && (DAValue <= 1279)) {
		low = DAValue - 1024;
		high = 0x04;
	} else if((DAValue >= 1280) && (DAValue <= 1535)) {
		low = DAValue - 1280;
		high = 0x05;
	} else if((DAValue >= 1536) && (DAValue <= 1791)) {
		low = DAValue - 1536;
		high = 0x06;
	} else if((DAValue >= 1792) && (DAValue <= 2047)) {
		low = DAValue - 1792;
		high = 0x07;
	} else if((DAValue >= 2048) && (DAValue <= 2303)){
		low = DAValue - 2048;
		high = 0x08;
	} else if((DAValue >= 2304) && (DAValue <= 2559)){
		low = DAValue - 2304;
		high = 0x09;
	} else if((DAValue >= 2560) && (DAValue <= 2815)){
		low = DAValue - 2560;
		high = 0x0a;
	} else if((DAValue >= 2816) && (DAValue <= 3071)){
		low = DAValue - 2816;
		high = 0x0b;
	} else if((DAValue >= 3072) && (DAValue <= 3327)){
		low = DAValue - 3072;
		high = 0x0c;
	} else if((DAValue >= 3328) && (DAValue <= 3583)){
		low = DAValue - 3328;
		high = 0x0d;
	} else if((DAValue >= 3584) && (DAValue <= 3839)){
		low = DAValue - 3584;
		high = 0x0e;
	} else if((DAValue >= 3840) && (DAValue <= 4095)){
		low = DAValue - 3840;
		high = 0x0f;
	} else{
		error = ERROR; /* error - D/A value must be 0-4095 */
	}
	if(error == NO_ERROR){
		outb(low, DACLOW + base); /* write the low byte value */
		outb(high, DACHIGH + base); /* write the high byte value */
	}

	return error; /* no errors detected */
}


