#include "kos.h"

//PCB* currUserPCB;
static Dllist readyQueue;
static int userRunning = FALSE;

void runUser(PCB* pcbToRun); 

void initScheduler(){
    readyQueue = new_dllist();
}

//This function is responsible for resuming the user process
void schedResumeUser(){
    DEBUG('s', "We are checking if there are any user programs to run\n");
    kt_joinall();
    if(dll_empty(readyQueue)){
        //printf("noop is running\n");
        //DEBUG('s', "no user info to read\n");
        curPCB = NULL;
        noop();
    }
    else
    {
        DEBUG('1', "found a waiting user process\n");
        Dllist nextDLItem = dll_first(readyQueue);
        PCB* next = (PCB*) nextDLItem->val.v;
        curPCB = next;
        dll_delete_node(nextDLItem);
        runUser(next);
    }
}

void runUser(PCB* pcbToRun){
    userRunning = TRUE;
    int* regs = pcbToRun->registers;
    //free(pcbToRun);
    //printf("%d\n", regs[NextPCReg]);
    //TODO: check whether the regs gets freed by run_user_code
    run_user_code(regs);
}

void addUserToSched(PCB* pcbToAdd){
    Jval newJval = new_jval_v((void*)pcbToAdd);
    dll_append(readyQueue, newJval);

}

//checks whether there was just a user program running
//Should be called every time we get an interrupt or exception
int checkUserRunning(){
    int tempUserRunningInfo = userRunning;
    userRunning = FALSE;
    return tempUserRunningInfo;
}




