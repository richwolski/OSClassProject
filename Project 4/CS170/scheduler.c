#include "kos.h"

//PCB* currUserPCB;
static Dllist readyQueue;
static int userRunning = FALSE;

void runUser(PCB *pcbToRun);

void initScheduler() {
    readyQueue = new_dllist();
}

//This function is responsible for resuming the user process
void schedResumeUser() {
    DEBUG('s', "We are checking if there are any user programs to run\n");
    kt_joinall();
    if (numValidPids == 0)
        SYSHalt();
    if (dll_empty(readyQueue)) {
        //printf("noop is running\n");
        //DEBUG('s', "no user info to read\n");
        lastUserPCB = NULL;
        noop();
    }
    else {
        DEBUG('1', "found a waiting user process\n");
        Dllist nextDLItem = dll_first(readyQueue);
        PCB *next = (PCB *) nextDLItem->val.v;
        dll_delete_node(nextDLItem);
        runUser(next);
    }
}

void runUser(PCB *pcbToRun) {
    userRunning = TRUE;
    int *regs = pcbToRun->registers;
    lastUserPCB = pcbToRun;
    //now we need set the correct values for the memory start and end
    assert(pcbToRun->memSlot > -1);
    User_Base = userMemStart(pcbToRun->memSlot);
    User_Limit = userMemEnd(pcbToRun->memSlot) - User_Base;
    //printf("\nGoing into PCReg: %d nextPCReg: %d\n", lastUserPCB->registers[PCReg],
    //        lastUserPCB->registers[NextPCReg]);
    //printf("User_Base is chosen to be %dwhen starting process %i\n",User_Base, pcbToRun->pid);
    //free(pcbToRun);
    //printf("%d\n", regs[NextPCReg]);
    //TODO: check whether the regs gets freed by run_user_code
    DEBUG('f', "going back to user land!\n");
    run_user_code(regs);
}

void addUserToSched(PCB *pcbToAdd) {
    Jval newJval = new_jval_v((void *) pcbToAdd);
    dll_append(readyQueue, newJval);

}

//checks whether there was just a user program running
//Should be called every time we get an interrupt or exception
int checkUserRunning() {
    int tempUserRunningInfo = userRunning;
    userRunning = FALSE;
    return tempUserRunningInfo;
}




