CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -Wconversion -std=c11 -pedantic
LIBS=

rasm: ./rasm.c ./rasm.h
	$(CC) $(CFLAGS) -o rasm ./rasm.c $(LIBS)
