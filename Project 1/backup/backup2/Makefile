CC=gcc
CFLAGS=-g

all:	mmult	print-rand-matrix	mmult-extra

mmult:	mmult.c
	${CC}	${CFLAGS}	-o	mmult	mmult.c	-lpthread

print-rand-matrix:	print-rand-matrix.c
	${CC}	${CFLAGS}	-o	print-rand-matrix	print-rand-matrix.c

mmult-extra:	mmult-extra.c
	${CC}	${CFLAGS}	-o	mmult-extra	mmult-extra.c	-lpthread

clean:
	rm -f mmult	print-rand-matrix	mmult-extra
