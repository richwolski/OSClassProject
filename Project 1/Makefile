LD_DEBUG=all	make

CC=gcc
CFLAGS=-g

all:	mmult	print-rand-matrix	mmult-extra	mmult-extra-backup

mmult:	mmult.c
	${CC}	${CFLAGS}	-o	mmult	mmult.c	-lpthread

print-rand-matrix:	print-rand-matrix.c
	${CC}	${CFLAGS}	-o	print-rand-matrix	print-rand-matrix.c

mmult-extra:	mmult-extra.c
	${CC}	${CFLAGS}	-o	mmult-extra	mmult-extra.c	-lm	-lpthread

mmult-extra-backup:	mmult-extra-backup.c
	${CC}	${CFLAGS}	-o	mmult-extra-backup	mmult-extra-backup.c	-lm	-lpthread

clean:
	rm -f mmult	print-rand-matrix	mmult-extra	mmult-extra-backup
