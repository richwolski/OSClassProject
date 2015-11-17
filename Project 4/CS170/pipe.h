#ifndef _PIPE_H
#define _PIPE_H

#define NUM_FD 256


struct pipe {
    int *buf;
    int head;
    int tail;
    int size;
    int readers;
    int writers;
    int eof;
    kt_sem pipeReadSem;
    kt_sem pipeWriteSem;
    kt_sem pipe_num_ele;
    kt_sem pipe_num_till_full;
} typedef Pipe;
//for the pipes reading and writing

Pipe *initPipe();

void removeReader(Pipe * thePipe);
void removeWriter(Pipe * thePipe);

void pipeRead(char *c, Pipe *thePipe);

void pipeWrite(char *c, Pipe *thePipe);

void removePipe(Pipe * thePipe);

// A file descriptor for a single file
// if you want a copy of the descriptor call duplicateFD instead of
// just getting a copy of the pointer

struct fileDesc {
    int fileType;    // the type is 0 for IO buffers, and 1 for pipes
    Pipe *pipe;
    int ioStreamNum; // the stream that this corresponds to (0 for stdin, 1 for stdout , etc.)
    int accessCode; // 4 for read, 2 for write (add together for read and write)
} typedef FileDesc;

FileDesc *newIO_FD(int streamNum);

FileDesc *newPipeFD(Pipe *pipe, int accessCode);

FileDesc *duplicateFD(FileDesc * oldFileDesc);

void deleteFD(FileDesc* toDelete);

#endif

