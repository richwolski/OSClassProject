#include "memory.h"
#include "simulator_lab2.h"


unsigned short get_new_pid(){
	int found;
	Rb_node tmp;
	while(1){
		tmp = rb_find_ikey_n(pidTree, nextPID, &found);
		if(found == 1){
			if(nextPID == 65535)
				nextPID = 1;
			else
				nextPID++;
		}
		else{
			tmp = rb_inserti(pidTree, nextPID, NULL);
			numValidPids ++;
			//printf("chose new PID: %i", nextPID);
			return nextPID;
		}
	}
}

void destroy_pid(int pid){
	int found; 
	Rb_node tmp;
	//printf("trying to delete PID %i \n", pid);
	tmp = rb_find_ikey_n(pidTree, pid, &found);
	if(found == 1){
		//printf("deleting pid: %i \n", tmp->k.ikey);
		rb_delete_node(tmp);
		numValidPids --;
	}
	else
		DEBUG('e', "destroy_pid called on invalid pid\n");
}


int get_mem_chunk(){
	int memSlot;

	DEBUG('n', "we have started making a new process\n");
	for (memSlot = 0; memSlot < 8; memSlot++){
		if (memSlotRecords[memSlot] == 0)
		{
			memSlotRecords[memSlot] = 1;
			return memSlot;
		}
	}
	return -1;
}


void* initUserProcess() {
    // this is a array of strings
	PCB* firstProcess;
	int memSlot; 
	memSlot = get_mem_chunk();
	firstProcess = createNewProcess(memSlot);
    	do_execve(firstProcess,kos_argv[0],kos_argv, FALSE);
}

PCB* createNewProcess(int memSlot){
	

	PCB* newPCB = (PCB*)malloc(sizeof(PCB));

	//now, we will set up the pcb vars
	newPCB->pid = get_new_pid();
	newPCB->parent = NULL;
	newPCB->memSlot = memSlot;
	newPCB->isDead = 0;
	newPCB->numChildren = 0;

	newPCB->child_death_wait = make_kt_sem(0);
	newPCB->parent_wait = make_kt_sem(0);
	newPCB->children = make_rb();
	// newPCB->children

	DEBUG('n', "we have made a new process\n");
	return newPCB;
}

void do_execve(PCB* curPCB, char* fileName, char** initArgv, int freeMem){
    	int memSlot = curPCB->memSlot;
	int memStart = userMemStart(memSlot);
	int memEnd = userMemEnd(memSlot);
	User_Base = memStart;
	User_Limit = userMemSize;
	bzero(memStart + main_memory, User_Limit);
    //printf("User_Base is chosen to be %i when intializing",User_Base);

	DEBUG('n', "started execve main function\n");
    	int codeSize = load_user_program(fileName);
	if (codeSize < 0) {
		fprintf(stderr,"Can't load program.\n");
		exit(1);
	}
	//first we need to figre out how big the args are
	int argC = 0;
	while(initArgv[argC] != NULL){
		argC++;
	}
    //printf("argC: %d\n", argC); 
	//this is the system's copy of the Argv
	char ** systemArgV = malloc(argC * sizeof(char**));
	
	int curStrLen;

	//we will be building the argvs strings backwards
	char* memHeadLoc = (char*)((int)main_memory + memEnd - 16);
	int argNum;
	for (argNum = argC -1; 0 <= argNum; argNum--)
	{
		curStrLen = strlen(initArgv[argNum])+ 1; //because we need to remember the last position
		memHeadLoc = memHeadLoc - curStrLen;
		strncpy(memHeadLoc, initArgv[argNum], curStrLen);
		systemArgV[argNum] = (char*)(memHeadLoc - main_memory - memStart);
        		if(freeMem)
            		free(initArgv[argNum]);
	}
    
	//now you need to word align the data
	int* memIntHeadLoc = (int*)(((int) memHeadLoc >> 2) << 2);
	memIntHeadLoc--;
	*memIntHeadLoc = 0;
	memIntHeadLoc--;

	for(argNum = argC - 1; 0 <= argNum; argNum--){
		*memIntHeadLoc = (int)systemArgV[argNum];
		memIntHeadLoc--;
	}
	int* argvLoc = memIntHeadLoc + 1;

	//add the envp
	*memIntHeadLoc = 0;
	memIntHeadLoc--;

	//add the argV
	*memIntHeadLoc = (int)kernelToUser(curPCB, (char*)argvLoc);
    //printf("argV is at %i\n", (int)kernelToUser(curPCB, (char*)memIntHeadLoc));
	memIntHeadLoc--;

	//add the argC
	*memIntHeadLoc = argC;
    //printf("argC is at %i\n", (int)kernelToUser(curPCB, (char*)memIntHeadLoc));

	//Now we will set up the args
	int* stackHead = memIntHeadLoc - 3;
	int i;

	for (i=0; i < NumTotalRegs; i++)
		curPCB->registers[i] = 0;

    //curPCB->dataBreak = (int)kernelToUser(curPCB, (char*)codeSize);
    //printf("the databreak is %i\n", codeSize);
    curPCB->dataBreak = codeSize;
	curPCB->registers[PCReg] = 0;
	curPCB->registers[NextPCReg] = 4;
	curPCB->registers[StackReg] = (int)kernelToUser(curPCB, (char*)stackHead);
    //printf("stackHead is at %i\n", curPCB->registers[StackReg]);
    
    if(freeMem){
        free(fileName);
        free(initArgv);
    }
	DEBUG('n', "execed a program \n");
	addUserToSched(curPCB);
	kt_exit();
}

void deletePCB(PCB *toDelete) {
	//place the pid free routine here
	//printf("PID free routine not implemented yet (memory.c)\n");
	destroy_pid(toDelete->pid);
	rb_free_tree(toDelete->children);
	free(toDelete);
}

int isMemValid(int memAddress, PCB* process){
    //checks if the memory address is valid for the particular PCB
    //This assumes that it is being passed user space addresses
    //returns a 1 if it is valid, zero otherwise

    int isAfter = memAddress > userMemEnd(process->memSlot) - userMemStart(process->memSlot);
    int isBefore = memAddress < 0;
    return !(isAfter || isBefore); 
}

char* userToKernel(PCB* pcb, char* address){
    return (char*)((int)address + (int)main_memory + (int)userMemStart(pcb->memSlot));
}

char* kernelToUser(PCB* pcb, char* address){
    return (char*)((int)address - (int)main_memory - (int)userMemStart(pcb->memSlot));
}


