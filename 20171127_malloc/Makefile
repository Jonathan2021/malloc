# Makefile for malloc
 
CC=gcc
CFLAGS= -Wall -Wextra -std=c99 -fno-builtin -O0 -g -fPIC -D_BSD_SOURCE
LDFLAGS=
LDLIBS=
 
libmalloc.so: malloc.o
	${CC} -shared -o libmalloc.so malloc.o
 
test: test.o
	gcc -o test test.o libmalloc.so

test.o: libmalloc.so

clean:
	rm -f *~ *.o
	rm -f libmalloc.so
 
# end
