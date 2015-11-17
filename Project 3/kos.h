#ifndef __KOS_H
#define __KOS_H


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "simulator_lab2.h"
#include "dllist.h"
#include "kt.h"
//#include "rb.h"

#include "scheduler.h"
#include "console_buf.h"
#include "memory.h"
#include "syscall.h"

#ifdef KOS_C
#define EXTERN 
#else
#define EXTERN extern
#endif


//macros to give back the start and end of a slot
#define userMemStart(slot) (((slot) * (MemorySize / 8 - PageSize)))
#define userMemEnd(slot) (userMemStart((slot)+1))
#define userMemSize (MemorySize / 8 - PageSize)




EXTERN kt_sem writeok; 
EXTERN kt_sem writers;
EXTERN kt_sem nelem; 
EXTERN kt_sem readers;
EXTERN kt_sem readok;
EXTERN kt_sem consoleWait;
EXTERN kt_sem verifyUser;
EXTERN PCB* lastUserPCB;
EXTERN Rb_node pidTree;
EXTERN int numValidPids;
//extern CRB* crb; 


//memory slots records
EXTERN int memSlotRecords[8];


#endif
