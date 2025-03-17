CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -Wconversion -Wno-missing-braces -fno-strict-aliasing -ggdb -std=c11 -pedantic
LIBS=

.PHONY: all
all: rasm rme derasm

rasm: ./rasm.c ./sv.h ./rasm.h
	$(CC) $(CFLAGS) -o rasm ./rasm.c $(LIBS)

rme: ./rme.c ./sv.h ./rasm.h
	$(CC) $(CFLAGS) -o rme ./rme.c $(LIBS)

derasm: ./derasm.c ./sv.h ./rasm.h
	$(CC) $(CFLAGS) -o derasm ./derasm.c $(LIBS)
