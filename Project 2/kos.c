/*
 * jos.c -- starting point for student's os.
 * 
 */
#define KOS_C
#include "kos.h"


void* initUserProcess(){
		// this is a array of strings
	
	/* need to back off from top of memory */
	/* 12 for argc, argv, envp */
	/* 12 for stack frame */

	int memSlot;
	for (memSlot = 0; memSlot < 8 memSlot++){
		if (memSlotRecords[memSlot] == 0)
		{
			memSlotRecords[memSlot] = 1;
			break;
		}
	}
	if (memSlot >= 8)
	{
		//throw an error because we have filled all the mem slots
		printf("All memory slots are filled. System exiting. \n");
		SYSHalt();
	}

	int memStart = userMemStart(memSlot);
	int memEnd = userMemEnd(memSlot);

	bzero(memStart, memEnd - memStart);
	//first we need to figre out how big the args are
	int argC = 0;
	//printf("kos_argv[%d] = %c\n", argC, kos_argv[argC]);
	while(kos_argv[argC] != NULL){
		//printf("kos_argv[%d] = %s\n", argC, kos_argv[argC]);
		argC++;
	}
	//printf("argC: %d\n", argC);
	//argC++
	//now we have the correct number of args

	//this is the system's copy of the Argv
	char ** systemArgV = malloc(argC * sizeof(char**));
	
	int curStrLen;

	//we will be building the argvs strings backwards
	char* memHeadLoc = main_memory + MemorySize - 16;
	int argNum;
	for (argNum = argC -1; 0 <= argNum; argNum--)
	{
		curStrLen = strlen(kos_argv[argNum])+ 1; //because we need to remember the last position
		// memHeadLoc = ((memHeadLoc - curStrLen) >> 2) << 2 // not needed
		memHeadLoc = memHeadLoc - curStrLen;
		//printf("%s\n", kos_argv[argNum]);
		strncpy(memHeadLoc, kos_argv[argNum], curStrLen);
		systemArgV[argNum] = memHeadLoc - (int)main_memory;
	}
	//now you need to word align the data

	int* memIntHeadLoc = (int*)(((int) memHeadLoc >> 2) << 2);
	//printf("memIntHeadLoc 1st: %d\n", memIntHeadLoc);
	memIntHeadLoc--;
	*memIntHeadLoc = 0;
	memIntHeadLoc--;
	//printf("memIntHeadLoc: %d\n", (int)memIntHeadLoc - (int)main_memory);

	for(argNum = argC - 1; 0 <= argNum; argNum--){
		*memIntHeadLoc = (int)systemArgV[argNum];
		memIntHeadLoc--;
		//printf("memIntHeadLoc: %d\n", (int)memIntHeadLoc - (int)main_memory);
	}
	int* argvLoc = memIntHeadLoc + 1;
	//printf("argvLoc: %d\n", (int)argvLoc - (int)main_memory);

	//add the envp
	*memIntHeadLoc = 0;
	//printf("envp: %d with value %d\n", memIntHeadLoc, *memIntHeadLoc);
	memIntHeadLoc--;

	//add the argV
	*memIntHeadLoc = (int)argvLoc - (int)main_memory;
	//printf("argV: %d with value %d\n", (int)memIntHeadLoc - (int)main_memory, *memIntHeadLoc);
	memIntHeadLoc--;

	//add the argC
	*memIntHeadLoc = argC;
	//printf("argC: %d with value %d\n", (int)memIntHeadLoc - (int)main_memory, *memIntHeadLoc);

	//Now we will set up the args


	int* stackHead = memIntHeadLoc - 3;
	PCB* curPCB = (PCB*)malloc(sizeof(PCB));
	int i;

	if (load_user_program(kos_argv[0]) < 0) {
		fprintf(stderr,"Can't load program.\n");
		exit(1);
	}

	for (i=0; i < NumTotalRegs; i++)
		curPCB->registers[i] = 0;

	curPCB->registers[PCReg] = 0;
	curPCB->registers[NextPCReg] = 4;
	curPCB->registers[StackReg] = (int)stackHead - (int)main_memory;
	//printf("stackHead: %d\n", (int)stackHead - (int)main_memory);

	// put newPCB at the end of the readyQueue

	addUserToSched(curPCB);
	kt_exit();
	
}



KOS()
{
	//initialize all the vars
	writeok = make_kt_sem(0);
	readok = make_kt_sem(0);
	writers = make_kt_sem(1);
	nelem = make_kt_sem(0);
	readers = make_kt_sem(1);
	consoleWait = make_kt_sem(0);
	verifyUser = make_kt_sem(1);

	bzero(memSlotRecords, 8*4);

	initScheduler();
	initCRB();
	kt_fork(readFromConsole, NULL);
	kt_fork(initUserProcess, NULL);
	// change this when getting to semaphores
	schedResumeUser();	
	// int registers[NumTotalRegs];
	// int i;
	
}





