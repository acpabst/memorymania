CC ?= gcc
CFLAGS ?= -g -pthread
LDFLAGS ?= -lm


all: clean memorymania.o memorymania

memorymania: memorymania.o
	$(CC) $(CFLAGS) memorymania.o -o memorymania $(LDFLAGS)

memorymania.o: main.c
	$(CC) $(CFLAGS) -c main.c -o memorymania.o $(LDFLAGS)

clean:
	rm -f memorymania memorymania.o
