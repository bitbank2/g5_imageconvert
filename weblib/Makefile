
all: g5lib.a
CFLAGS = -D__LINUX__ -Wall -O2

g5lib.a: weblib.o
	ar -rc g5lib.a weblib.o

weblib.o: weblib.c Makefile ../Group5.h
	$(CC) $(CFLAGS) -c weblib.c

clean:
	rm -rf *.o g5lib.a
