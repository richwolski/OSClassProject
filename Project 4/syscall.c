#include "kos.h"
#include "console_buf.h"
#include "pipe.h"

int hitEOF = 0;


void syscall_return(PCB *pcb, int retval) {
    pcb->registers[2] = retval;
    pcb->registers[PCReg] = pcb->registers[NextPCReg];
    if (pcb != NULL) { //only add it if we got back from a user
        DEBUG('s', "sticking the user back into the schedule\n");
        addUserToSched(pcb);
    }
    kt_exit();
}


void *do_getpagesize(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    syscall_return(newPCB, PageSize);
}

void *do_stat_buf_fill(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    if (newPCB->registers[5] == 0)
        stat_buf_fill(userToKernel(newPCB, (char *) newPCB->registers[6]), 1);
    else if (newPCB->registers[5] == 1 || newPCB->registers[5] == 2)
        stat_buf_fill(userToKernel(newPCB, (char *) newPCB->registers[6]), 256);
    syscall_return(newPCB, 0);
}

void *do_pipe(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    int readEnd, writeEnd;

    readEnd = get_next_FD(newPCB->fileDesc);
    newPCB->fileDesc[readEnd] = (FileDesc*)1;       //temp assignment
    writeEnd = get_next_FD(newPCB->fileDesc);
    if (writeEnd > 255){
        newPCB->fileDesc[readEnd] = NULL;   //need to undo the temp assignment  
        syscall_return(newPCB, -1);
    }

    // create new pipe
    Pipe *pipe = initPipe();

    // create new FD's for the pipe
    FileDesc *fdR = newPipeFD(pipe, 4);
    FileDesc *fdW = newPipeFD(pipe, 2);
    newPCB->fileDesc[readEnd] = fdR;
    newPCB->fileDesc[writeEnd] = fdW;
    char *fdAddress = userToKernel(newPCB, (char*) newPCB->registers[5]);
    *((int *) fdAddress) = readEnd;
    *(((int *) fdAddress) + 1) = writeEnd;

    //printFDs(newPCB);
    syscall_return(newPCB, 0);

}


void *do_ioctl_console_fill(void *pcb) {
    PCB *newPCB = (PCB *) pcb;

    if (newPCB->registers[5] != 1 || newPCB->registers[6] != JOS_TCGETP)
        syscall_return(newPCB, -EINVAL);
    else {
        ioctl_console_fill(userToKernel(newPCB, (char *) newPCB->registers[7]));
        syscall_return(newPCB, 0);
    }
}

void *do_read(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    int pcbFD = newPCB->registers[5];
    FileDesc *readFD = newPCB->fileDesc[pcbFD];
    assert(newPCB->memSlot > -1);

    if(NULL == readFD)
        syscall_return(newPCB, -EBADF);

    if (pcbFD == 1 || pcbFD == 2)
        syscall_return(newPCB, -EBADF);

    //printf("do_read\n");
    if (hitEOF)
        syscall_return(newPCB, 0);

    if (!isMemValid(newPCB->registers[6], newPCB)) {
        //printf("read second case fail\n");
        syscall_return(newPCB, -EFAULT);
    }

    if (newPCB->registers[7] < 0) {
        syscall_return(newPCB, -EINVAL);
    }

    else {
        if (readFD->ioStreamNum == 0) {
            char *sysAddress = userToKernel(newPCB, (char*) newPCB->registers[6]);
            char consoleChar;
            int i = 0;
            //printf("do_read: %d\n", newPCB->registers[7]);
            P_kt_sem(readers);
            while (i < newPCB->registers[7]) {
                P_kt_sem(nelem);
                CRBRead(&consoleChar);
                //printf("in do_read: %c\n", consoleChar);
                if (consoleChar == (char) -1) {
                    hitEOF = 1;
                    V_kt_sem(readers);
                    syscall_return(newPCB, i);
                }
                else {
                    sysAddress[i] = (char) consoleChar;
                    i++;
                }
            }

            V_kt_sem(readers);
            syscall_return(newPCB, i);
        }
        else if (readFD->ioStreamNum == -1) {
            if(readFD->accessCode != 4)
                syscall_return(newPCB, -EBADF);
            char *sysAddress = userToKernel(newPCB, (char*) newPCB->registers[6]);
            int i = 0;
            P_kt_sem(readFD->pipe->pipeReadSem);
            while (i < newPCB->registers[7]) {
                if(1 == readFD->pipe->eof && 1 == kt_getval(
                        readFD->pipe->pipe_num_ele)){
                    V_kt_sem(readFD->pipe->pipeReadSem);
                    syscall_return(newPCB,i);
                }
                if(0 < i && 0 == kt_getval(
                        readFD->pipe->pipe_num_ele)){
                    V_kt_sem(readFD->pipe->pipeReadSem);
                    syscall_return(newPCB,i);
                }

                P_kt_sem(readFD->pipe->pipe_num_ele);
                pipeRead(&(sysAddress[i]), readFD->pipe);
                V_kt_sem(readFD->pipe->pipe_num_till_full);
                i++;
            }
            V_kt_sem(readFD->pipe->pipeReadSem);
            syscall_return(newPCB, i);
        

        }
    }
}


void *do_write(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    assert(newPCB->memSlot > -1);
    int pcbFD = newPCB->registers[5];
    FileDesc *writeFD = newPCB->fileDesc[pcbFD];

    if(NULL == writeFD)
        syscall_return(newPCB, -EBADF);

    if (writeFD->ioStreamNum == 0)
        syscall_return(newPCB, -EBADF);

    if (!isMemValid(newPCB->registers[6], newPCB)) {
        //printf("write second case fail\n");
        syscall_return(newPCB, -EFAULT);
    }

    if (newPCB->registers[7] < 0) {
        syscall_return(newPCB, -EINVAL);
    }

    else {
        if (writeFD->ioStreamNum == 1 || writeFD->ioStreamNum == 2) {
            char *sysAddress = userToKernel(newPCB, (char*) newPCB->registers[6]);
            int i = 0;
            //printf("do_write: %d\n", newPCB->registers[7]);
            P_kt_sem(writers);
            //printf("\n process %i has entered WRITE critical section\n", newPCB->pid );
            while (sysAddress[i] != '\0' && i < newPCB->registers[7]) {
                //printf("in do_write: %c blahblah \n", sysAddress[i]);
                console_write(sysAddress[i]);
                i++;
                P_kt_sem(writeok);
            }
            //printf("\n process %i has exited WRITE critical section\n", newPCB->pid );
            V_kt_sem(writers);
            syscall_return(newPCB, newPCB->registers[7]);
        }
        else if (writeFD->ioStreamNum == -1) {
            if(writeFD->accessCode != 2)
                syscall_return(newPCB, -EBADF);
            if (writeFD->pipe->readers == 0)
                syscall_return(newPCB, -EPIPE);
            else {
                char *sysAddress = userToKernel(newPCB, (char*) newPCB->registers[6]);
                int i = 0;
                P_kt_sem(writeFD->pipe->pipeWriteSem);
                while (i < newPCB->registers[7]) {
                    P_kt_sem(writeFD->pipe->pipe_num_till_full);
                    if(writeFD->pipe->readers == 0){
                        syscall_return(newPCB,i);
                    }
                    pipeWrite(&(sysAddress[i]), writeFD->pipe);
                    V_kt_sem(writeFD->pipe->pipe_num_ele);
                    if(EOF == sysAddress[i])
                        break;
                    if(sysAddress[i] == '\0'){
                        i++;
                        break;
                    }
                    i++;
                }
                V_kt_sem(writeFD->pipe->pipeWriteSem);
                syscall_return(newPCB, i);
            }


        }
    }
}

void *do_close(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    //printFDs(newPCB);
    int pcbFD = newPCB->registers[5];
    if(newPCB->fileDesc[pcbFD] == NULL){
        //printf("got to end of close, not a good FD tho\n");
        syscall_return(newPCB, -EBADF);
    }
    else{
        deleteFD(newPCB->fileDesc[pcbFD]);
        newPCB->fileDesc[pcbFD] = NULL;
        //printf("got to the end of close\n");
        syscall_return(newPCB, 0);
    }
    
}

void *sbrk_call(void *pcb) {
    PCB *newPCB = (PCB *) pcb;

    int prevBreak = newPCB->dataBreak;
    newPCB->dataBreak += newPCB->registers[5];
    if (!isMemValid(newPCB->dataBreak, newPCB)) {
        DEBUG('e', "Invalid memory address for sbrk");
        syscall_return(newPCB, -1);
    }
    else {
        //printf("Sbrk new location is %i\n", newPCB->dataBreak);
        syscall_return(newPCB, prevBreak);
    }
}

void *getppid_call(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    if (newPCB->parent == NULL) // if the parent is init
        syscall_return(newPCB, 0);
    syscall_return(newPCB, ((PCB *) newPCB->parent)->pid);
}

void *getpid_call(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    syscall_return(newPCB, newPCB->pid);
}

void *dtable_call(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    syscall_return(newPCB, 64);
}

void *execve_call(void *pcb) {
    PCB *newPCB = (PCB *) pcb;
    if (!isMemValid(newPCB->registers[6], newPCB) || !isMemValid(newPCB->registers[5], newPCB)) {
        // one of the addresses is invalid
        syscall_return(newPCB, -1);
    }
    char *user_filename;
    char *filename;

    int numArgV = 0;
    char **user_argv;
    char **argv;
    int strNum;

    user_filename = userToKernel(newPCB, (char *) newPCB->registers[5]);
    filename = strAllocCpy(user_filename);

    user_argv = (char **) userToKernel(newPCB, (char *) newPCB->registers[6]);
    while (user_argv[numArgV] != NULL) {
        numArgV++;
    }
    numArgV++;
    argv = (char **) malloc(numArgV * sizeof(char *));

    for (strNum = 0; strNum < numArgV - 1; strNum++) {
        char *curUserStr = userToKernel(newPCB, (char *) user_argv[strNum]);
        argv[strNum] = strAllocCpy(curUserStr);
    }
    argv[numArgV - 1] = NULL;
    do_execve(pcb, filename, argv, TRUE);
}

void *exit_call(void *pcb)  {
    Rb_node iterNode;
    PCB *childPCB, *parentPCB;
    PCB *newPCB = (PCB *) pcb;
    //printFDs(newPCB);
    memSlotRecords[newPCB->memSlot] = 0;
    //newPCB->memSlot = -1;
    newPCB->isDead = 1;
    int i;
    for (i = 0; i < 256; i++) {
        if (newPCB->fileDesc[i] != NULL) {
            //printf("Deleting file descriptor %d\n",i);
            deleteFD(newPCB->fileDesc[i]);
        }
    }
    //change the parents of alive children to init
    if (0 != newPCB->numChildren) {
        rb_traverse(iterNode, newPCB->children) //yes, this function is a for loop in disguise....
        {
            childPCB = (PCB *) rb_val(iterNode);
            if (isAlive(childPCB)) {
                childPCB->parent = NULL;
            }
            else {
                deletePCB(childPCB);
            }
        }
    }

    if (newPCB->parent == NULL) {
        deletePCB(newPCB);
    }
    else {
        parentPCB = (PCB *) newPCB->parent;
        V_kt_sem(parentPCB->parent_wait);
    }
    kt_exit();
}

//only used by the fork function
void *add_fork_child(void *pcb) {
    PCB *childPCB = (PCB *) pcb;
    syscall_return(childPCB, 0);
}

void *fork_call(void *pcb) {
    int i;
    PCB *childPCB;
    PCB *parentPCB = (PCB *) pcb;
    //printf("parent pid is: %d\n", parentPCB->pid);
    DEBUG('s', "started the fork call\n");
    assert(parentPCB->memSlot > -1);

    //create a child PCB
    childPCB = createNewProcess();
    childPCB->parent = parentPCB;

    //Project 4: add the file descriptor copying here
    for (i = 0; i < 256; i++) {
        if (parentPCB->fileDesc[i] == NULL)
            childPCB->fileDesc[i] = NULL;
        else
            childPCB->fileDesc[i] = duplicateFD(parentPCB->fileDesc[i]);
    }

    //copy all of the data
    childPCB->dataBreak = parentPCB->dataBreak;
    for (i = 0; i < NumTotalRegs; i++) {
        childPCB->registers[i] = parentPCB->registers[i];
    }
    //memcpy(childPCB->registers,parentPCB->registers, sizeof(int) * NumTotalRegs);
    memcpy(userToKernel(childPCB, (char *) 0), userToKernel(parentPCB, (char *) 0), userMemSize);

    rb_inserti(parentPCB->children, childPCB->pid, (char *) childPCB);
    parentPCB->numChildren++;
    //printf("printing out the parent FDs:\n");
    //printFDs(parentPCB);
    //printf("printing out the child FDs:\n");
    //printFDs(childPCB);

    //DEBUG('s', "about to stick the child on the ready queue\n");
    kt_fork(add_fork_child, childPCB);
    //DEBUG('s', "about to stick the parent on the ready queue\n");
    syscall_return(parentPCB, childPCB->pid);
}

void *wait_call(void *pcb) {
    PCB *childPCB, *iterPCB;
    Rb_node iterNode;
    int returnValue;
    PCB *parent = (PCB *) pcb;
    if (0 == parent->numChildren) {
        syscall_return(parent, -ECHILD);
    }
    P_kt_sem(parent->parent_wait);

    childPCB = NULL;
    rb_traverse(iterNode, parent->children) //yes, this function is a for loop in disguise....
    {
        iterPCB = (PCB *) iterNode->v.val;
        if (!isAlive(iterPCB)) {
            childPCB = iterPCB;
            break;
        }
    }
    if (childPCB == NULL) {
        syscall_return(parent, -ECHILD);
    }
    parent->registers[6] = childPCB->registers[5];
    returnValue = childPCB->pid;
    rb_delete_node(iterNode);
    deletePCB(childPCB);
    parent->numChildren--;

    syscall_return(parent, returnValue);
}


void *dup_call(void *pcb) {
    PCB* newPCB = (PCB*) pcb;

    int oldFDIndex = newPCB->registers[5];
    if(NUM_FD < oldFDIndex || 0 > oldFDIndex)
        syscall_return(newPCB, -EBADF);
        int i;
    for (i = 0; i < NUM_FD; i++) {
        if (newPCB->fileDesc[i] == NULL) {
                FileDesc* oldFD  = newPCB->fileDesc[oldFDIndex];
                if(NULL ==oldFD){
                    syscall_return(newPCB, -EBADF); //bad FD
                }
            newPCB->fileDesc[i] = duplicateFD(oldFD);
                    syscall_return(newPCB, i);
            }
        }
    syscall_return(newPCB, -EMFILE); //no FD available
}


void *dup2_call(void *pcb) {
    PCB* newPCB = (PCB*) pcb;
    int newFDIndex = newPCB->registers[6];
    if(NUM_FD < newFDIndex || 0 > newFDIndex)
        syscall_return(newPCB, -EBADF);

    int oldFDIndex = newPCB->registers[5];
    if(NUM_FD < oldFDIndex || 0 > oldFDIndex)
        syscall_return(newPCB, -EBADF);

    FileDesc* oldFD  = newPCB->fileDesc[oldFDIndex];
    if(NULL ==oldFD){
        syscall_return(newPCB, -EBADF); //bad FD
    }
    
    FileDesc* newFD  = newPCB->fileDesc[newFDIndex];
    
    newPCB->fileDesc[newFDIndex] = duplicateFD(oldFD);

    if(NULL != newFD)
       deleteFD(newFD); //yes, it is necessary to delete last
    
    syscall_return(newPCB,newFDIndex);
}

    


char *strAllocCpy(char *src) {
    int stringLen;
    char *newStr;

    stringLen = strlen(src) + 1; // to account for terminating /0
    newStr = malloc(stringLen * sizeof(char));
    strncpy(newStr, src, stringLen);
    return newStr;
}

int get_next_FD(FileDesc **fds) {
    int i;
    for (i = 0; i < 256; i++) {
        //printf("4938 The file descriptor value is : %d\n",fds);
        if (*fds == NULL)
            return i;
        else
            fds++;
    }
    return i;
}


void printFDs(PCB* thePCB){
    FileDesc **fds = thePCB->fileDesc;
    int i;
    for(i = 0; i < NUM_FD;i++){
        if(NULL != *fds){
            printf("File Desc: %d is not null \n", i);
            if(1 == (*fds)->fileType)
                printf("pipeFD: %x\n",(*fds)->pipe);
            else
                printf("IOFD: %d\n", (*fds)->ioStreamNum);
        }
        fds++;
    }
    printf("Finished printing file descs\n");
}
