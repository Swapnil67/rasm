CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -Wconversion -ggdb -std=c11 -pedantic
LIBS=

.PHONY: all
all: rasm rme

rasm: ./rasm.c ./sv.h ./rasm.h
	$(CC) $(CFLAGS) -o rasm ./rasm.c $(LIBS)

rme: ./rme.c ./sv.h ./rasm.h
	$(CC) $(CFLAGS) -o rme ./rme.c $(LIBS)
