

//
// Core logic test program
//


#include "ladder.h"
#include <unistd.h>


int	test1();
int	test2();


int main(){

	return test1();



}


int	test1(){
	ladder	*A;

	printf("-------------------------TEST1  0 \n");
	A = new ladder();
	printf("-------------------------TEST2  0 \n");
	A->load("default.plc");
	A->dump();
	printf("-------------------------TEST3  0 \n");
	A->cpu->scan("main");
	A->dump();
sleep(1);
	printf("-------------------------TEST4  0 \n");
	A->cpu->scan("main");
	A->dump();
	printf("-------------------------TEST5  0 \n");

	delete A;
	return NO_ERROR;
}



int	test2(){
	core	*PLC;
	step_t	*step1, *step2, *step3;
	variable	*temp1;
	//variable	*temp2;
	//variable	*temp3;

	PLC = new core();
	printf("-------- 0 ----------\n");

	printf("Adding values");
	temp1 = mem->add(NULL, "A", "A", "input", _INTEGER);	temp1->var->set_int(5);
	temp1 = mem->add(NULL, "X", "X", "output", _INTEGER);	temp1->var->set_int(100);
	PLC->dump();
	printf("-------- 1 ----------\n");

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_LABEL, "main", "the main PLC program", NULL, NULL);
	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "start of new rung", library->search("SOR"), NULL);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "first input", library->search("XIC"), NULL);
		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, NULL, "bit fetch", library->search("BIT"), NULL);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, mem->find("A"));
			temp1 = mem->add(NULL, NULL, NULL, NULL, _INTEGER);	temp1->var->set_int(0);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, temp1);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "start of branch", library->search("BST"), NULL);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "first input", library->search("XIO"), NULL);
		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, NULL, "bit fetch", library->search("BIT"), NULL);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, mem->find("A"));
			temp1 = mem->add(NULL, NULL, NULL, NULL, _INTEGER);	temp1->var->set_int(1);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, temp1);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "next branch", library->search("NXB"), NULL);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "first input", library->search("XIC"), NULL);
		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, NULL, "bit fetch", library->search("BIT"), NULL);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, mem->find("A"));
			temp1 = mem->add(NULL, NULL, NULL, NULL, _INTEGER);	temp1->var->set_int(2);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, temp1);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "next branch", library->search("NXB"), NULL);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "first input", library->search("XIC"), NULL);
		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, NULL, "bit fetch", library->search("BIT"), NULL);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, mem->find("A"));
			temp1 = mem->add(NULL, NULL, NULL, NULL, _INTEGER);	temp1->var->set_int(3);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, temp1);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "end of branch", library->search("BND"), NULL);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "first output", library->search("OTE"), NULL);
		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, NULL, "bit fetch", library->search("BIT"), NULL);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "output", NULL, mem->find("X"));
			temp1 = mem->add(NULL, NULL, NULL, NULL, _INTEGER);	temp1->var->set_int(0);
			step3 = PLC->add_to_prog(_CHILD, step2, _STEP_VARIABLE, NULL, "input", NULL, temp1);

	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, NULL, "end of new rung", library->search("EOR"), NULL);

//		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_VARIABLE, "AB", "ab", NULL, mem->find("F8:1"));
//	step1 = PLC->add_to_prog(_CHILD, NULL, _STEP_INSTRUCTION, "B", "b", NULL, NULL);
//		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, "BA", "ba", NULL, NULL);
//		step2 = PLC->add_to_prog(_CHILD, step1, _STEP_INSTRUCTION, "BB", "bb", NULL, NULL);
	PLC->dump();
	printf("-------- 2 ----------\n");

	PLC->scan("main");
	PLC->dump();
	printf("-------- 3 ----------\n");

	// PLC->scan(NULL);
	// PLC->dump();
	// printf("-------- 4 ----------\n");

	delete PLC;

	return 0;
}


/*

SOR
	XIC ( BIT ( A ) ( 3 ) )
	BST
		XIC ( BIT A 4 )
	NXB
		XIO ( BIT A 5 )
	BND
	OTE ( BIT X 0 )
EOR

*/




	#define	_LOG_TO_SCREEN		1
	#define	_LOG_TO_FILE		2

void error_log(int code, char *text){
	int	__log_flag = _LOG_TO_SCREEN;
	FILE	*fp_out;
	time_t	now;

	time(&now);
	if((__log_flag & _LOG_TO_SCREEN) > 0){
		printf("%d: %s: %s", code, text, ctime(&now));
	}
	if((__log_flag & _LOG_TO_FILE) > 0){
		if((fp_out = fopen("plc.log", "a")) != NULL){
			fprintf(fp_out, "%d: %s: %s", code, text, ctime(&now));
			fclose(fp_out);
		} else {
			printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX Error log write failed\n");
		}
	}
}
