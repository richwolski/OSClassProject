


// from syscall.c
void syscall_return(PCB* pcb, int retval);
void* do_write(void* pcb);
void* do_read(void* pcb);
void* do_close(void* pcb);
void* do_ioctl_console_fill(void* pcb);
void* do_stat_buf_fill(void* pcb);
void* do_getpagesize(void* pcb);
void* sbrk_call(void* pcb);
void* fork_call(void* pcb);
void* exit_call(void* pcb);
void* execve_call(void* pcb);
void* getppid_call(void* pcb);
void* getpid_call(void* pcb);
void* dtable_call(void* pcb);
void* wait_call(void* pcb);

//this is a helper function
char* strAllocCpy(char* src);

