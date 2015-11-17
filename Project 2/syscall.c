#include "kos.h"

int hitEOF = 0;

void syscall_return(PCB* pcb, int retval){
	pcb->registers[2] = retval;
	pcb->registers[PCReg] = pcb->registers[NextPCReg];
	if(pcb != NULL) //only add it if we got back from a user
		addUserToSched(pcb);
	kt_exit();
}


void* do_read(void* pcb){
	PCB* newPCB = (PCB*) pcb;
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
		char* sysAddress = (main_memory + newPCB->registers[6]);
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
		char* sysAddress = (main_memory + newPCB->registers[6]);
		int i = 0;
		//printf("do_write: %d\n", newPCB->registers[7]);
		P_kt_sem(writers);
		while(sysAddress[i] != '\0' && i < newPCB->registers[7]){
			//printf("in do_write: %c blahblah \n", sysAddress[i]);
			console_write(sysAddress[i]);
			i++;
			P_kt_sem(writeok);
		}
		V_kt_sem(writers);
		syscall_return(newPCB, newPCB->registers[7]);
	}
}
