#ifndef _CONSOLE_BUF_H
#define _CONSOLE_BUF_H

#define NUM_FD 256

struct ConReadBuf {
    int *buf;
    int head;
    int tail;
    int size;

} typedef CRB;

// for console read buffer
void *readFromConsole();

void initCRB();

void writeCRB(char *c);

int CRBisFull();

int CRBisEmpty();

void CRBRead(char *c);

void freeCRB();

#endif
