CC=gcc
CFLAGS=-g

all:	matrix-mult

matrix-mult:	matrix-mult.c
	${CC}	${CFLAGS}	-o	matrix-mult	matrix-mult.c	-lpthread

clean:
	rm	-f	matrix-mult