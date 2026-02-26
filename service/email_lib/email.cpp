/////////////////////////////////////////////////////////////////////////////////
//
// A program to bridge TCP/IP networks to the LPC message interface protocol
//
// The top section of this program must be defined for all client tasks - I suggest
// using this as a template.
//
//
//    Copyright (C) 2000-2 by Hugh Jack <jackh@gvsu.edu>
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
// Last Modified: May 31, 2002
//


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include <plc.h>
#include <module_library.h>
#ifdef __cplusplus
extern "C" {
#endif


#include "email.h" // the low level network functions



int	init(){
	int	error;
	error = NO_ERROR;
	for(int i = 0; i < MAXIMUM_MAIL_LIST_SIZE; i++){
		mail_list[i].status = _EMAIL_UNUSED;
	}

	return error;
}


int	deinit(){
	for(int i = 0; i < MAXIMUM_MAIL_LIST_SIZE; i++){
		if(mail_list[i].status != _EMAIL_UNUSED){
			printf("WARNING: Unsent email %s  %s  %s \n",
				mail_list[i].destination,
				mail_list[i].subject,
				mail_list[i].message);
			delete mail_list[i].destination;
			delete mail_list[i].subject;
			delete mail_list[i].message;
			delete mail_list[i].net;
		}
	}
	return NO_ERROR;
}




int	step_run(){
	return email_send();
}


int message_receive(char*message){
	return email_parse(message);
}




int email_parse(char*message){
	int	error;
	int	i, j;
	int	len;

	error = NO_ERROR;
	len = strlen(message);
printf("disecting [%s]\n", message);
	for(i = 0; (i < len) && (message[i] != '~'); i++){}
	for(j = i+1; (j < len) && (message[j] != '~'); j++){}
	if(j < len){
		message[i] = 0;
		message[j] = 0;
		error = email_add(message, &(message[i+1]), &(message[j+1]));
	} else {
		error = ERROR;
		error_log(MINOR, "email message not well formed 'destination:subject:file'");
	}

	return error;
}



int email_add(char *destination, char *subject, char *message){
	int	error;
	int	i, j;

	error = NO_ERROR;
	for(i = 0; i < MAXIMUM_MAIL_LIST_SIZE; i++){
		if(mail_list[i].status == _EMAIL_UNUSED) break;
	}
	if(i < MAXIMUM_MAIL_LIST_SIZE){
		for(j = 0; j < (int) strlen(destination); j++){
			if(destination[j] == '@') break;
		}
		if(j < (int)strlen(destination)){
			mail_list[i].destination = new char[strlen(destination)+1];
			strcpy(mail_list[i].destination, destination);
			mail_list[i].subject = new char[strlen(subject)+1];
			strcpy(mail_list[i].subject, subject);
			mail_list[i].message = new char[strlen(message)+1];
			strcpy(mail_list[i].message, message);
			mail_list[i].net = new network_io();
			mail_list[i].net->set_remote_host(&(destination[j+1]), 25);
			mail_list[i].net->init_write();
			mail_list[i].status = _EMAIL_CONNECTING;
			time(&(mail_list[i].start));
		} else {
			error_log(MINOR, "Email address not properly formed");
			error = ERROR;
		}
	} else {
		error = ERROR;
		error_log(MAJOR, "ERROR: email log is full");
	}

	return error;
}


int email_send(){
	int	error;
	int	i;
	char	work[200];
	time_t	now;

	error = NO_ERROR;
	time(&now);
	for(i = 0; i < MAXIMUM_MAIL_LIST_SIZE; i++){
		if(mail_list[i].status == _EMAIL_UNUSED){
			// Do nothing
		} else {
printf("scan %d  %s \n", mail_list[i].status, mail_list[i].destination);
			if(mail_list[i].status == _EMAIL_CONNECTING){
				if(mail_list[i].net->open_write_connection() == NO_ERROR){
					mail_list[i].status = _EMAIL_HELO;
				}
			} else if(mail_list[i].status == _EMAIL_HELO){
				if((mail_list[i].net->read_from_connection(work, 200) == NO_ERROR) && (strlen(work) > 0)){
					if(atoi(work) == 220){
						sprintf(work, "helo [%s]\n", mail_list[i].net->get_address());
						error = mail_list[i].net->write_to_connection(work);
						mail_list[i].status = _EMAIL_FROM;
					} else {
						error = ERROR;
						error_log(MINOR, "ERROR: SMTP connection not right");
					}
				}
			} else if(mail_list[i].status == _EMAIL_FROM){
				if((mail_list[i].net->read_from_connection(work, 200) == NO_ERROR) && (strlen(work) > 0)){
					if(atoi(work) == 250){
						// THE NEXT LINE CREATES PROBLEMS FOR SOME MAIL SYSTEMS
						sprintf(work, "mail from: mat_plc@%s\n", mail_list[i].net->get_address());
						//printf("SENDING [[%s]]\n", work);
						// THE FOLLOWING LINE IS A QUICK FIX
						sprintf(work, "mail from: mat_plc_email_test@mat.sourceforge.net\n");
						error = mail_list[i].net->write_to_connection(work);
						mail_list[i].status = _EMAIL_TO;
					} else {
						error = ERROR;
						error_log(MINOR, "ERROR: HELO not right");
					}
				}
			} else if(mail_list[i].status == _EMAIL_TO){
				if((mail_list[i].net->read_from_connection(work, 200) == NO_ERROR) && (strlen(work) > 0)){
					if(atoi(work) == 250){
						sprintf(work, "rcpt to: %s\n", mail_list[i].destination);
						error = mail_list[i].net->write_to_connection(work);
						mail_list[i].status = _EMAIL_DATA;
					} else {
						error = ERROR;
						error_log(MINOR, "ERROR: MAIL FROM not right");
					}
				}
			} else if(mail_list[i].status == _EMAIL_DATA){
				if((mail_list[i].net->read_from_connection(work, 200) == NO_ERROR) && (strlen(work) > 0)){
					if(atoi(work) == 250){
						error = mail_list[i].net->write_to_connection("data\n");
						mail_list[i].status = _EMAIL_BODY;
					} else {
						error = ERROR;
						error_log(MINOR, "ERROR: RCPT TO not right");
					}
				}
			} else if(mail_list[i].status == _EMAIL_BODY){
				if((mail_list[i].net->read_from_connection(work, 200) == NO_ERROR) && (strlen(work) > 0)){
					if(atoi(work) == 354){
						sprintf(work, "Subject: %s\n", mail_list[i].subject);
						error = mail_list[i].net->write_to_connection(work);
//printf("Here to send mail body \n");
						FILE *fp_in;
						if((fp_in = fopen(mail_list[i].message, "r")) != NULL){
							char	text[1001];
							fgets(text, 1000, fp_in);
							while((feof(fp_in) == 0) && (error == NO_ERROR)){
								text[1000] = 0;
								error = mail_list[i].net->write_to_connection(text);
								fgets(text, 1000, fp_in);
							}
							fclose(fp_in);
						} else {
							error = mail_list[i].net->write_to_connection(mail_list[i].message);
						}
						error = mail_list[i].net->write_to_connection("\n.\n");
						mail_list[i].status = _EMAIL_QUIT;
					} else {
						error = ERROR;
						error_log(MINOR, "ERROR: DATA not right");
					}
				}
			} else if(mail_list[i].status == _EMAIL_QUIT){
				if((mail_list[i].net->read_from_connection(work, 200) == NO_ERROR) && (strlen(work) > 0)){
					if(atoi(work) == 250){
						error = mail_list[i].net->write_to_connection("quit\n");
						mail_list[i].status = _EMAIL_DONE;
					} else {
						error = ERROR;
						error_log(MINOR, "ERROR: BODY not right");
					}
				}
			}
			if((mail_list[i].status == _EMAIL_DONE) || (difftime(mail_list[i].start, now) > 60) || (error == ERROR)){
				if(difftime(mail_list[i].start, now) > 60){
					error_log(MINOR, "ERROR: email write connection timed out");
				}
				delete mail_list[i].destination;
				delete mail_list[i].subject;
				delete mail_list[i].message;
				mail_list[i].net->end_write_connection();
				mail_list[i].net->deinit_write();
				delete mail_list[i].net;
				mail_list[i].status = _EMAIL_UNUSED;
			}
		}

	}

	return error;
}


#ifdef __cplusplus
}
#endif



