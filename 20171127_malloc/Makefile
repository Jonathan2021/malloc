CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -D_GNU_SOURCE -fPIC -fvisibility=hidden -g -fno-builtin
LDFLAGS = -shared -fno-builtin
TARGET_LIB = libmalloc.so
OBJS = malloc.o
BIN = test
CC ?= gcc


.PHONY: all ${TARGET_LIB} clean

$(BIN): test_malloc.c
	$(CC) $(CFLAGS) -o $(@) $(<) 

${TARGET_LIB}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ $^

all: ${TARGET_LIB}

clean:
	${RM} ${TARGET_LIB} ${OBJS}
