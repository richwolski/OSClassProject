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
    PCB* tempPCB;

	//P_kt_sem(verifyUser);
	examine_registers(buf);
	DEBUG('e', "examining registers\n");
	//PCB* curUserPCB = (PCB*)malloc(sizeof(PCB));
	memcpy(lastUserPCB->registers, buf, NumTotalRegs*sizeof(int) );
	type = buf[4];
	r5 = buf[5];
	r6 = buf[6];
	r7 = buf[7];
    //printf("\n PCReg: %d nextPCReg: %d\n", lastUserPCB->registers[PCReg],
    //        lastUserPCB->registers[NextPCReg]);
	newPC = buf[NextPCReg];
	lastUserPCB->registers[PCReg] = lastUserPCB->registers[NextPCReg];
    tempPCB = lastUserPCB;
    lastUserPCB = NULL;
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
            break;
		case SYS_exit:
			/* this is the _exit() system call */
			DEBUG('e', "SYS_exit() system call\n");
			//freeCRB()
            kt_fork(exit_call,(void *)tempPCB);
            break;
			//printf("Program exited with value %d.\n", r5);
		case SYS_read:
			kt_fork(do_read,(void *)tempPCB);
			DEBUG('e', "SYS_read system call\n");
			break;
		case SYS_write:
			// fork off a thread
			kt_fork(do_write, (void*)tempPCB);
			DEBUG('e', "SYS_write system call\n");
			break;

		//lab 2 sys calls
		case SYS_close:
			kt_fork(do_close,(void *)tempPCB);
			DEBUG('e', "SYS_close system call\n");
			break;
		case SYS_execve:
		    kt_fork(execve_call,(void*)tempPCB);
		    DEBUG('e', "SYS_execve system call\n");
		    break;
		case SYS_fstat: 
			kt_fork(do_stat_buf_fill, (void*)tempPCB);
			DEBUG('e', "SYS_fstat system call\n");
			break;
        case SYS_fork:
            //printf("\nprocess %d is forking", tempPCB->pid);
            kt_fork(fork_call, (void*)tempPCB);
            DEBUG('e', "SYS_fork system call\n");
            break;
        case SYS_getdtablesize:
            kt_fork(dtable_call,(void*)tempPCB);
            DEBUG('e', "SYS_sbrk system call\n");
            break;
		case SYS_getpagesize:
			kt_fork(do_getpagesize, (void*)tempPCB);
			DEBUG('e', "SYS_getpagesize system call\n");
			break;
		case SYS_getpid:
		    kt_fork(getpid_call,(void*)tempPCB);
				DEBUG('e', "SYS_getpid system call\n");
		    break;
		case SYS_getppid:
		    kt_fork(getppid_call,(void*)tempPCB);
				DEBUG('e', "SYS_getppid system call\n");
		    break;
		case SYS_ioctl:
			kt_fork(do_ioctl_console_fill, (void*)tempPCB);
			DEBUG('e', "SYS_ioctl system call\n");
			break;
		case SYS_sbrk:
		    kt_fork(sbrk_call,(void*)tempPCB);
            DEBUG('e', "SYS_sbrk system call\n");
		    break;
        case SYS_wait:
            kt_fork(wait_call,(void*)tempPCB);
            DEBUG('e', "SYS_sbrk system call\n");
            break;
		default:
			DEBUG('e', "Unknown system call\n");
			printf("the system error is: %i\n", type);
			SYSHalt();
			break;
        }
        break;
	case PageFaultException:
		DEBUG('e', "Exception PageFaultException\n");
		break;
	case BusErrorException:
		DEBUG('e', "Exception BusErrorException\n");
		kt_fork(exit_call,(void *)tempPCB);
		break;
	case AddressErrorException:
		DEBUG('e', "Exception AddressErrorException\n");
		kt_fork(exit_call,(void *)tempPCB);
		break;
	case OverflowException:
		DEBUG('e', "Exception OverflowException\n");
		break;
	case IllegalInstrException:
		DEBUG('e', "Exception IllegalInstrException\n");
		kt_fork(exit_call,(void *)tempPCB);
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
	if(lastUserPCB != NULL){
		memcpy(lastUserPCB->registers, buf, NumTotalRegs*sizeof(int));
        //printf("\n PCReg: %d nextPCReg: %d\n", lastUserPCB->registers[PCReg],
        //        lastUserPCB->registers[NextPCReg]);
		addUserToSched(lastUserPCB);
		lastUserPCB = NULL;
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
	case TimerInt:
		DEBUG('t', "TimerInt interrupt\n");
		break;
	default:
		DEBUG('e', "Unknown interrupt\n");
		break;
	}
	kt_joinall();
	schedResumeUser();
}
