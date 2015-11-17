/*
 * exception.c -- stub to handle user mode exceptions, including system calls
 * 
 * Everything else core dumps.
 * 
 * Copyright (c) 1992 The Regents of the University of California. All rights
 * reserved.  See copyright.h for copyright notice and limitation of
 * liability and disclaimer of warranty provisions.
 */
#include "simulator.h"
#include "kos.h"

kt_sem writeok;

void
exceptionHandler(ExceptionType which)
{
	int             type, r5, r6, r7, newPC;
	int             buf[NumTotalRegs];
	int userRunning;

	//P_kt_sem(verifyUser);
	examine_registers(buf);
	DEBUG('e', "examining registers\n");
	//PCB* curUserPCB = (PCB*)malloc(sizeof(PCB));
	memcpy(curPCB->registers, buf, NumTotalRegs*sizeof(int) );
	type = buf[4];
	r5 = buf[5];
	r6 = buf[6];
	r7 = buf[7];
	newPC = buf[NextPCReg];
	// printf("Exception buf[NextPCReg] = %d\n", buf[NextPCReg]);
	// if (!checkUserRunning())
	// {
	// 	free(curUserPCB);
	// 	curUserPCB= NULL;
		
	// 	printf("Woah, you should never see this line of charachters 2349234980234\n");
	// 	assert(FALSE);
	// }
	// else
	// {
	// 	DEBUG('1', "we got back a user process\n");
	// 	//printf("main_memory: %d\n newPC: %d\n", (int)main_memory, newPC);
		
	// }
	//V_kt_sem(verifyUser);
	/*re 
	 * for system calls type is in r4, arg1 is in r5, arg2 is in r6, and
	 * arg3 is in r7 put result in r2 and don't forget to increment the
	 * pc beforeturning!
	 */

	switch (which) {
	case SyscallException:
		/* the numbers for system calls is in <sys/syscall.h> */
		switch (type) {
		case 0:
			/* 0 is our halt system call number */
			DEBUG('e', "Halt initiated by user program\n");
			//freeCRB();
			SYSHalt();
		case SYS_exit:
			/* this is the _exit() system call */
			DEBUG('e', "_exit() system call\n");
			//freeCRB();
			//printf("Hello World!\n");
			printf("Program exited with value %d.\n", r5);
			SYSHalt();
		case SYS_read:
			kt_fork(do_read,(void *)curPCB);
            DEBUG('e', "SYS_read system call\n");
            break;
		case SYS_write:
			// fork off a thread
			kt_fork(do_write, (void*)curPCB);
			DEBUG('e', "SYS_write system call\n");
            break;
		default:
			DEBUG('e', "Unknown system call\n");
			//printf("the system error is: %i\n", type);
			SYSHalt();
			break;
		}
		break;
	case PageFaultException:
		DEBUG('e', "Exception PageFaultException\n");
		break;
	case BusErrorException:
		DEBUG('e', "Exception BusErrorException\n");
		break;
	case AddressErrorException:
		DEBUG('e', "Exception AddressErrorException\n");
		break;
	case OverflowException:
		DEBUG('e', "Exception OverflowException\n");
		break;
	case IllegalInstrException:
		DEBUG('e', "Exception IllegalInstrException\n");
		break;
	default:
		printf("Unexpected user mode exception %d %d\n", which, type);
		SYSHalt();
		exit(1);
	}
	
	schedResumeUser();
}

void
interruptHandler(IntType which)
{
	int             type, r5, r6, r7, newPC;
	int             buf[NumTotalRegs];
	int userRunning;

	//P_kt_sem(verifyUser);
	examine_registers(buf);
	//PCB* curUserPCB = (PCB*)malloc(sizeof(PCB));
	if(curPCB != NULL){
		memcpy(curPCB->registers, buf, NumTotalRegs*sizeof(int));
		addUserToSched(curPCB);
	}
	// type = buf[4];
	// r5 = buf[5];
	// r6 = buf[6];
	// r7 = buf[7];
	// newPC = buf[PCReg];
	// (curUserPCB->registers)[NextPCReg] = newPC;
	//printf("Interrupt buf[NextPCReg] = %d\n", buf[PCReg]);

	// userRunning = checkUserRunning();
	// if (userRunning)
	// {
	// 	DEBUG('1', "we got back a user process in the interrupt\n");
	// 	//printf("main_memory: %d\n newPC: %d\n", (int)main_memory, newPC);   
	// 	addUserToSched(curUserPCB);
	// }
	// else
	// {
	// 	free(curUserPCB);
	// }
	//V_kt_sem(verifyUser);

	switch (which) {
	case ConsoleReadInt:
	//keyboard to console
		V_kt_sem(consoleWait);
		DEBUG('e', "ConsoleReadInt interrupt\n");
		break;
	case ConsoleWriteInt:
		//program to console
		V_kt_sem(writeok);
		//DEBUG('e', "ConsoleWriteInt interrupt\n");
		break;
	default:
		DEBUG('e', "Unknown interrupt\n");
		break;
	}
	kt_joinall();
	schedResumeUser();
}
