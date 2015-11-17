/*
 * jos.c -- starting point for student's os.
 * 
 */
#define KOS_C
#include "kos.h"


KOS()
{
	//initialize all the vars
	start_timer(10);
	writeok = make_kt_sem(0);
	readok = make_kt_sem(0);
	writers = make_kt_sem(1);
	nelem = make_kt_sem(0);
	readers = make_kt_sem(1);
	consoleWait = make_kt_sem(0);
	verifyUser = make_kt_sem(1);
	pidTree = make_rb();
	//printf("pidTree not initialized to a root? %d\n", !(pidTree->s.head));
	numValidPids = 0;
	bzero(memSlotRecords, 8*4);
	nextPID = 1;

	initScheduler();
	initCRB();
	kt_fork(readFromConsole, NULL);
	kt_fork(initUserProcess, NULL);
	schedResumeUser();	
	// int registers[NumTotalRegs];
	// int i;
	
}





