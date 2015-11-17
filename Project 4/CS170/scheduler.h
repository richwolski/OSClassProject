#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "rb.h"
#include "console_buf.h"
#include "pipe.h"


//forward declaration
typedef struct ProcessControlBlock {
	// contains information about the user process
	int registers[NumTotalRegs];
	FileDesc *fileDesc[256];
	int pid;
	void *parent; // if NULL, then the parent is INIT
	int dataBreak; // this is the location of sbrk
	int memSlot; // if -1 it's not in memory
	int isDead;
	Rb_node children;
	kt_sem parent_wait;
	kt_sem child_death_wait;
	int numChildren;
} PCB;

//these are helper functions for PCBs
#define isAlive(PCB) ((PCB)->isDead == 0)

// from scheduler.c 
void schedResumeUser();

void initScheduler();

int checkUserRunning();

void addUserToSched(PCB *pcbToAdd);

void runUser(PCB *pcbToRun);

void deletePCB(PCB *toDelete);

#endif

