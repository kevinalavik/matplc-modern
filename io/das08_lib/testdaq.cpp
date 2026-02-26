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
// Last Modified: March 11, 2001
//



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "das08_io.h"



int ChooseCounter();
int ChooseConfig();
int ChooseDir(int DirectNum);

#define QUERY			350
#define	CHOOSE_PORT		351
#define	CHOOSE_COUNTER	352
#define CHOOSE_CONFIG	353
#define	CHOOSE_DIRECTION 354


int	query(int, char*, int);





int main(){
	int	choice;
	das08	*A;
	int	value;

	A = new das08();
	A->configure("das08.conf");
	A->connect();

	do{
		printf("\n\n------------ DAS08 Test Harness Menu --------------\n");

		printf("1.  Digital  Configure\n");
		printf("2.  Digital  Input   Bit\n");
		printf("3.  Digital  Input   Word\n");
		printf("4.  Digital  Output  Bit\n");
		printf("5.  Digital  Output  Word\n\n");
		printf("6.  Counter  Configure\n");
		printf("7.  Counter  Load    Value\n");
		printf("8.  Counter  Input   Value\n\n");
		printf("9.  Analog   Input   Value\n");
		printf("10. Analog   Output  Value\n\n");
		printf("11. Quit\n\n");
		printf("Select: ");
		scanf("%d", &choice);
		if(choice == 1){
			A->DConfigPort(	query(CHOOSE_PORT, NULL, 0),
							query(CHOOSE_DIRECTION, NULL, 0));
		} else if(choice == 2){
			A->DBitIn(	query(CHOOSE_PORT, NULL, 0),
						query(QUERY, "Choose a bit (0-7): ", 0), &value);
			printf("The Bit Value is [%d] \n", value);
		} else if(choice == 3){
			A->DIn(		query(CHOOSE_PORT, NULL, 0), &value);
			printf("The Value is [%d] or [%d]hex\n", value, value);
		} else if(choice == 4){
			A->DBitOut(	query(CHOOSE_PORT, NULL, 0),
						query(QUERY, "Choose a bit (0-7): ", 0),
						query(QUERY, "Choose a value (0 or 1): ", 0));
		} else if(choice == 5){
			A->DOut(	query(CHOOSE_PORT, NULL, 0),
						query(QUERY, "Choose a value (-128 to 127): ", 0));
		} else if(choice == 6){
			A->C8254Config(	query(CHOOSE_COUNTER, NULL, 0),
							query(CHOOSE_CONFIG, NULL, 0));
		} else if(choice == 7){
			A->CLoad(	query(CHOOSE_COUNTER, NULL, 0),
						query(QUERY, "Enter a value in the form 0x____ : ", 0));
		} else if(choice == 8){
			A->CIn(		query(CHOOSE_COUNTER, NULL, 0), &value);
			printf("The Counter value was [%d]\n", value);
		} else if(choice == 9){
			A->AIn(		query(QUERY, "Enter Channel Number (0-7): ", 0), &value);
			printf("The value is [%d]\n", value);
		} else if(choice == 10){
			A->AOut(	query(QUERY, "Enter Channel Number (0-1): ", 0),
						query(QUERY, "Enter Value (0- 4095): ", 0));
		} else if(choice == 11){
		} else {
			printf("ERROR: Choice not recognized\n");
		}
	} while(choice != 11);
	A->disconnect();
	delete A;
}



int	query(int type, char *text, int def){
	char	work[20];
	int	value;

	if(type == QUERY){
		printf("%s [%d]: ", text, def);
		scanf("%s", work);
printf("<%s>\n", work);
		if(strlen(work) == 0){
			return def;
		} else {
			return atoi(work);
		}
	} else if(type == CHOOSE_PORT){
		printf("Which port (1=A, 2=B, 3=C, 4=CH, 5=CL, 6=AUX): ");               
    		scanf("%d", &value);
		if(value == 1) return PORTA;
		if(value == 2) return PORTB;
		if(value == 3) return PORTC;
		if(value == 4) return PORTCL;
		if(value == 5) return PORTCH;
		if(value == 6) return PORTAUX;
		return ERROR;
	} else if(type == CHOOSE_COUNTER){
		printf("Which counter (1, 2, 3): ");               
    		scanf("%d", &value);
		if((value >= 1) || (value <= 3)) return value;
		return ERROR;
	} else if(type == CHOOSE_CONFIG){
		printf("Which mode (1=HighOnLastCount, 2=OneShot, 3=RateGenerator, 4=SquareWave, 5=SoftwareStrobe, 6=HardwareStrobe): ");
    		scanf("%d", &value);
		if(value == 1) return HIGHONLASTCOUNT;
		if(value == 2) return ONESHOT;
		if(value == 3) return RATEGENERATOR;
		if(value == 4) return SQUAREWAVE;
		if(value == 5) return SOFTWARESTROBE;
		if(value == 6) return HARDWARESTROBE;
		return ERROR;
	} else if(type == CHOOSE_DIRECTION){
		printf("Which direction (1=In, 2=Out): ");               
    		scanf("%d", &value);
		if(value == 1) return DIGITALIN;
		if(value == 2) return DIGITALOUT;
		return ERROR;
	} else {
		return ERROR;
	}
}



void	error_log(int code, char *string){
	printf("ERROR %d: %s \n", code, string);
}


