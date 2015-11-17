#ifndef __KOS_H
#define __KOS_H

#ifdef KOS_C
#define EXTERN 
#else
#define EXTERN extern
#endif


//macros to give back the start and end of a slot
#define userMemStart(slot) ( slot * (MemorySize / 8 - 8) + 32)
#define userMemEnd(slot) (memStart(slot+1) - 4)

#include "simulator.h"
#include "dllist.h"
#include <assert.h>
#include "kt.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

struct ProcessControlBlock { 
	// contains information about the user process
	int registers[NumTotalRegs];
	int pid; 
	int memSlot; // if -1 it's not in memory


} typedef PCB;

struct ConReadBuf{
	int* buf;
	int head; 
	int tail; 
	int size;  

} typedef CRB;


EXTERN kt_sem writeok; 
EXTERN kt_sem writers;
EXTERN kt_sem nelem; 
EXTERN kt_sem readers;
EXTERN kt_sem readok;
EXTERN kt_sem consoleWait;
EXTERN kt_sem verifyUser;
EXTERN PCB* curPCB;
//extern CRB* crb; 


//memory slots records
EXTERN memSlotRecords[8];

// from scheduler.c 
void schedResumeUser();
void initScheduler();
int checkUserRunning();
void addUserToSched(PCB* pcbToAdd);
void runUser(PCB* pcbToRun);

// from syscall.c
void syscall_return(PCB* pcb, int retval);
void* do_write(void* pcb);
void* do_read(void* pcb);

// from console_buf.c
void* readFromConsole();
void initCRB();
void writeCRB( char* c);
int CRBisFull();
int CRBisEmpty();
void CRBRead( char* c);
void freeCRB();


#endif