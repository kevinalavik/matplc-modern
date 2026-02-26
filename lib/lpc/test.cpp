


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


#include "server_queue_t.h"
#include "client_queue_t.h"


void client_A(){
	char	*msg;
	server_queue_t *temp = new server_queue_t("client_A");
	for(;;){
		msg = temp->read_text();
		if(msg != NULL){
			printf("Got Message: [%s]\n", msg);
			if(msg[0] == 'E') break;
		}
	}
	sleep(1);

	delete temp;
}


void client_B(){
	char	msg[100];
	client_queue_t *temp = new client_queue_t("client_B", "client_A");
	if(temp->status == 0){
		for(;;){
			gets(msg);
			temp->writer(msg);
			if(msg[0] == 'E') break;
		}
	} else {
		printf("client open failed \n");
	}
	sleep(1);

	delete temp;
}





main(int argc, char**argv){
	if(argc >= 2){
		if(argv[1][0] == '1'){
			client_A();
		} else if(argv[1][0] == '2'){
			client_B();
		} else {
			printf("The choices are 1-2\n");
		}
	} else {
		printf("Add the arguments 1-2\n");
	}
}
