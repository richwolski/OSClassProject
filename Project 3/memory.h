#include "kos.h"
#include "simulator_lab2.h"

unsigned short nextPID;
unsigned short get_new_pid();
void destroy_pid();
void* initUserProcess();
int get_mem_chunk();
PCB* createNewProcess(int memSlot);
void do_execve(PCB* curPCB, char* fileName, char** initArgv, int freeMem);


// memory related helper functions

int isMemValid(int memAddress, PCB* process);
char* kernelToUser(PCB* pcb, char* address);
char* userToKernel(PCB* pcb, char* address);
