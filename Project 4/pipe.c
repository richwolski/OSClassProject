#include "kos.h"
#define PipeSize 256


Pipe *initPipe() {
    Pipe *pipe = (Pipe *) malloc(sizeof(Pipe));
    pipe->buf = (int *) calloc( PipeSize, sizeof(int));;
    pipe->head = 0;
    pipe->tail = 0;
    pipe->size = PipeSize;
    pipe->eof = 0;
    pipe->readers = 0;
    pipe->writers = 0;
    pipe->pipeReadSem = make_kt_sem(1);
    pipe->pipeWriteSem = make_kt_sem(1);
    pipe->pipe_num_ele = make_kt_sem(0);
    pipe->pipe_num_till_full = make_kt_sem(PipeSize);
    return pipe;
}


void pipeRead(char * c, Pipe* thePipe){
    *c = thePipe->buf[thePipe->tail];
    thePipe->tail = (thePipe->tail + 1 )%PipeSize;
}

void pipeWrite(char * c, Pipe* thePipe){
    thePipe->buf[thePipe->head]  = *c;
    if((char) -1== *c){
        //might need more stuff here to make everything work
        thePipe->eof = 1;
    }
    thePipe->head = (thePipe->head + 1 )%PipeSize;
}

void removeReader(Pipe *thePipe) {
    thePipe->readers--;
    if (thePipe->readers == 0) {
        V_kt_sem(thePipe->pipe_num_till_full);
        if(thePipe->readers == 0 && thePipe->writers == 0){
            removePipe(thePipe);
        }
    }
}

void removeWriter(Pipe *thePipe) {
    thePipe->writers--;
    if (thePipe->writers == 0) {
        V_kt_sem(thePipe->pipe_num_ele);
        thePipe->eof = 1;
        if(thePipe->readers == 0 && thePipe->writers == 0){
            removePipe(thePipe);
        }
    }

    
}

void removePipe(Pipe *thePipe) {
    free(thePipe->buf);
    free(thePipe);
}

FileDesc *newPipeFD(Pipe *pipe, int accessCode) {
    FileDesc *fd = (FileDesc *) malloc(sizeof(FileDesc));
    fd->fileType = 1;
    fd->ioStreamNum = -1;
    fd->pipe = pipe;
    fd->accessCode = accessCode;
    if (accessCode == 4)
        fd->pipe->readers++;
    if (accessCode == 2)
        fd->pipe->writers++;
    return fd;
}

FileDesc *newIO_FD(int streamNum) {
    FileDesc *fd = (FileDesc *) malloc(sizeof(FileDesc));
    fd->fileType = 0;
    fd->ioStreamNum = streamNum;
    return fd;
}

FileDesc *duplicateFD(FileDesc *oldFileDesc) {
    FileDesc *newFD = (FileDesc *) malloc(sizeof(FileDesc));
    newFD->fileType = oldFileDesc->fileType;
    newFD->pipe = oldFileDesc->pipe;
    newFD->accessCode = oldFileDesc->accessCode;
    if (oldFileDesc->fileType == 1) {
        if (oldFileDesc->accessCode == 4)
            oldFileDesc->pipe->readers++;
        if (oldFileDesc->accessCode == 2)
            oldFileDesc->pipe->writers++;
    }
    newFD->ioStreamNum = oldFileDesc->ioStreamNum;
    return newFD;
}

void deleteFD(FileDesc* toDelete) {
    //printf("deleting file descriptor: %d\n", toDelete->ioStreamNum);
    if (toDelete->fileType == 1) {
        if (toDelete->accessCode == 4)
            removeReader(toDelete->pipe);
        else
            removeWriter(toDelete->pipe);
    }
    free(toDelete);
}