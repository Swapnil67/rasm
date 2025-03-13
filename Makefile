CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -Wconversion -ggdb -std=c11 -pedantic
LIBS=

rasm: ./rasm.c ./sv.h ./rasm.h
	$(CC) $(CFLAGS) -o rasm ./rasm.c $(LIBS)
