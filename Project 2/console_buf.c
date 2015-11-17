#include "kos.h"

static CRB* crb;


void initCRB(){
	crb = (CRB*)malloc(sizeof(CRB));
	crb->buf = (int*)calloc(257, sizeof(int));
	crb->size = 257;
	crb->head = 0;
	crb->tail = 0;
}

void* readFromConsole(){
	while(1){
		P_kt_sem(consoleWait);
		char newChar = console_read();
		if(CRBisFull() == 0){
			writeCRB(&newChar);
			V_kt_sem(nelem);
		}
	}
	//V_kt_sem(readok);
}

void writeCRB( char* c){
	// if((crb->tail +1) % crb->size == crb->head){
	// 	//discard input if there is 256 items in bufffer already
	// 	return;
	// }
	crb->buf[crb->tail] = (int)*c;
	//printf("writeCRB: %c\n", *c);
	crb->tail = (crb->tail + 1) % crb->size;
	//printf("writeCRB: %d, %d\n", crb->head, crb->tail);
}

void freeCRB(){
	free(crb->buf);
	free(crb);
}

int CRBisFull(){
	//printf("CRBisFull: %d\n", (crb->tail + 1) % crb->size == crb->head);
	return (crb->tail + 1) % crb->size == crb->head;
}

int CRBisEmpty(){
	return(crb->tail == crb->head);
}

void CRBRead(char* c) {
    *c = crb->buf[crb->head];
    crb->head = (crb->head + 1) % crb->size;
}
