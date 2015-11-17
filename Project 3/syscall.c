#include "kos.h"

int hitEOF = 0;

void syscall_return(PCB* pcb, int retval){
	pcb->registers[2] = retval;
	pcb->registers[PCReg] = pcb->registers[NextPCReg];
	if(pcb != NULL){ //only add it if we got back from a user
        DEBUG('s', "sticking the user back into the schedule\n");
		addUserToSched(pcb);
    }
	kt_exit();
}


void* do_getpagesize(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	syscall_return(newPCB, PageSize);
}

void* do_stat_buf_fill(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	if(newPCB->registers[5] == 0)
		stat_buf_fill(userToKernel(newPCB,(char*)newPCB->registers[6]), 1);
	else if(newPCB->registers[5] == 1 || newPCB->registers[5] == 2)
		stat_buf_fill(userToKernel(newPCB,(char*)newPCB->registers[6]), 256);
	syscall_return(newPCB, 0);
}

void* do_ioctl_console_fill(void* pcb){
	PCB* newPCB = (PCB*) pcb;

	if(newPCB->registers[5] != 1 || newPCB->registers[6] != JOS_TCGETP)
		syscall_return(newPCB, -EINVAL);
	else{
		ioctl_console_fill(userToKernel(newPCB,(char*)newPCB->registers[7]));
		syscall_return(newPCB, 0);
	}
}

void* do_read(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	assert(newPCB->memSlot > -1);
	//printf("do_read\n");
	if(hitEOF)
		syscall_return(newPCB, 0);

	if(newPCB->registers[5] != 0){
		//printf("read first case fail: %d \n", newPCB->registers[5]);
		syscall_return(newPCB,-EBADF);
	}
	if(newPCB->registers[6] < 0 || newPCB->registers[6] > MemorySize -24){
		//printf("read second case fail\n");
		syscall_return(newPCB,-EFAULT);
	}

	if(newPCB->registers[7] < 0){
		syscall_return(newPCB, -EINVAL);
	}

	else{
		//P_kt_sem(readers);
		char* sysAddress = (main_memory + userMemStart(newPCB->memSlot) +newPCB->registers[6]);
		char consoleChar;
		int i = 0; 
		//printf("do_read: %d\n", newPCB->registers[7]);
		P_kt_sem(readers);
		while(i < newPCB->registers[7]){
			P_kt_sem(nelem);
			CRBRead(&consoleChar);
			//printf("in do_read: %c\n", consoleChar);
			if((int)consoleChar == -1){
				hitEOF = 1;
				V_kt_sem(readers);
				syscall_return(newPCB, i);
			}
			else{
				sysAddress[i] = (char)consoleChar;
				i++;
			}
		}
		
		V_kt_sem(readers);
		syscall_return(newPCB, i);
	}
}


void* do_write(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	assert(newPCB->memSlot > -1);
	//printf("do_write\n");
	if(newPCB->registers[5] != 1 && newPCB->registers[5] != 2){
		//printf("write first case fail %d \n", newPCB->registers[5]);
		syscall_return(newPCB,-EBADF);
	}
	if(newPCB->registers[6] < 0 || newPCB->registers[6] > MemorySize -24){
		//printf("write second case fail\n");
		syscall_return(newPCB,-EFAULT);
	}

	if(newPCB->registers[7] < 0){
		syscall_return(newPCB, -EINVAL);
	}

	else{
		//printf("ok write is 1 or 2: %d \n", newPCB->registers[5]);
		char* sysAddress = (main_memory + userMemStart(newPCB->memSlot) + newPCB->registers[6]);
		int i = 0;
		//printf("do_write: %d\n", newPCB->registers[7]);
		P_kt_sem(writers);
		//printf("\n process %i has entered WRITE critical section\n", newPCB->pid );
		while(sysAddress[i] != '\0' && i < newPCB->registers[7]){
			//printf("in do_write: %c blahblah \n", sysAddress[i]);
			console_write(sysAddress[i]);
			i++;
			P_kt_sem(writeok);
		}
		//printf("\n process %i has exited WRITE critical section\n", newPCB->pid );
		V_kt_sem(writers);
		syscall_return(newPCB, newPCB->registers[7]);
	}
}

void* do_close(void* pcb){
	PCB* newPCB = (PCB*) pcb;
    syscall_return(newPCB,-EBADF);
}

void* sbrk_call(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	
    int prevBreak = newPCB->dataBreak;
    newPCB->dataBreak += newPCB->registers[5];
    if(!isMemValid(newPCB->dataBreak, newPCB)){
        DEBUG('e', "Invalid memory address for sbrk");
        syscall_return(newPCB,-1);
    }
    else{
        //printf("Sbrk new location is %i\n", newPCB->dataBreak);
        syscall_return(newPCB,prevBreak);
    }
}

void* getppid_call(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	if(newPCB->parent == NULL) // if the parent is init
		syscall_return(newPCB,0);
    syscall_return(newPCB,((PCB*)newPCB->parent)->pid);
}

void* getpid_call(void* pcb){
	PCB* newPCB = (PCB*) pcb;
    syscall_return(newPCB,newPCB->pid);
}

void* dtable_call(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	syscall_return(newPCB, 64);
}

void* execve_call(void* pcb){
	PCB* newPCB = (PCB*) pcb;
	if(!isMemValid(newPCB->registers[6], newPCB) || !isMemValid(newPCB->registers[5], newPCB)  ){
        // one of the addresses is invalid
		syscall_return(newPCB,-1);
	}
    char* user_filename;
    char* filename;

    int numArgV = 0;
    char** user_argv;
    char** argv;
    int strNum;

    user_filename = userToKernel(newPCB, (char*)newPCB->registers[5]);
    filename = strAllocCpy(user_filename);

    user_argv = (char**) userToKernel(newPCB, (char*)newPCB->registers[6]);
	while(user_argv[numArgV] != NULL){
		numArgV++;
	}
	numArgV++;
    	argv = (char**) malloc(numArgV *sizeof(char*));
    
    for(strNum = 0; strNum < numArgV -1; strNum++){
        char* curUserStr = userToKernel(newPCB, (char*)user_argv[strNum]);
        argv[strNum] = strAllocCpy(curUserStr);
    }
    argv[numArgV -1] = NULL;
    do_execve(pcb, filename, argv, TRUE);
}

void* exit_call(void* pcb){
	Rb_node iterNode;
	PCB *childPCB, *parentPCB;
	PCB *newPCB = (PCB*) pcb;
	memSlotRecords[newPCB->memSlot] = 0;
	//newPCB->memSlot = -1;
	newPCB->isDead = 1;
	//change the parents of alive children to init
	if(0 != newPCB->numChildren){
		rb_traverse(iterNode,newPCB->children) //yes, this function is a for loop in disguise....
		{
			childPCB = (PCB*)rb_val(iterNode);
			if(isAlive(childPCB)){
				childPCB->parent = NULL;
			}
			else{
				deletePCB(childPCB);
			}
		}
	}

	if(newPCB->parent == NULL){
		deletePCB(newPCB);
	}
	else{
		parentPCB = (PCB*) newPCB->parent;
		V_kt_sem(parentPCB->parent_wait);
	}
	kt_exit();
}

//only used by the fork function
void* add_fork_child(void* pcb){
	PCB* childPCB = (PCB*) pcb;
	syscall_return(childPCB, 0);
}

void* fork_call(void* pcb){

    	int i, memSlot;
	PCB* childPCB;
	PCB* parentPCB = (PCB*) pcb;
	//printf("parent pid is: %d\n", parentPCB->pid);
	DEBUG('s', "started the fork call\n");
	assert(parentPCB->memSlot > -1);

	//create a child PCB
	memSlot = get_mem_chunk();
	if(memSlot < 0)
		syscall_return(childPCB, -EAGAIN);
	childPCB = createNewProcess(memSlot);
	childPCB->parent = parentPCB;

	//Project 4: add the file descriptor copying here

	//copy all of the data
	childPCB->dataBreak = parentPCB->dataBreak;
    for(i =0; i < NumTotalRegs;i++){
        childPCB->registers[i] = parentPCB->registers[i];
    }
	//memcpy(childPCB->registers,parentPCB->registers, sizeof(int) * NumTotalRegs);
	memcpy(userToKernel(childPCB, (char*)0), userToKernel(parentPCB, (char*)0), userMemSize);

	rb_inserti(parentPCB->children, childPCB->pid, (char*)childPCB);
	parentPCB->numChildren++;
	//DEBUG('s', "about to stick the child on the ready queue\n");
	kt_fork(add_fork_child, childPCB);
	//DEBUG('s', "about to stick the parent on the ready queue\n");
	syscall_return(parentPCB, childPCB->pid);
}

void* wait_call(void* pcb){
	PCB* childPCB, *iterPCB;
	Rb_node iterNode;
	int returnValue;
	PCB* parent = (PCB*) pcb;
	if(0 == parent->numChildren)
	{
		printf("waiting parent has no children\n");
		SYSHalt();
	}
	P_kt_sem(parent->parent_wait);

	childPCB = NULL;
	rb_traverse(iterNode, parent->children) //yes, this function is a for loop in disguise....
	{
		iterPCB = (PCB*)iterNode->v.val;
		if(!isAlive(iterPCB)){
			childPCB = iterPCB;
			break;
		}
	}
	if(childPCB == NULL){
		printf("No waiting child PCBs found even though parent had availabe child\n");
		SYSHalt();
	}
	parent->registers[6] = childPCB->registers[5];
	returnValue = childPCB->pid;
	rb_delete_node(iterNode);
	deletePCB(childPCB);
	parent->numChildren--;

	syscall_return(parent, returnValue);
}

char* strAllocCpy(char* src){
    int stringLen;
    char* newStr;
    
    stringLen = strlen(src)+1; // to account for terminating /0
    newStr = malloc(stringLen*sizeof(char));
    strncpy(newStr,src,stringLen);
    return newStr;
}
