CC ?= gcc
CFLAGS ?= -g -pthread
LDFLAGS ?= -lncurses

ifeq ($(CROSS_COMPILE),)
 $(info Compiling natively.)
else
 CROSS_COMPILE_FULL = aarch64-none-linux-gnu-$(CROSS_COMPILE)
 CC = $(CROSS_COMPILE_FULL)
 $(info Cross compiling: $(CROSS_COMPILE_FULL))
endif

all: clean memorymania.o memorymania

memorymania: memorymania.o
	$(CC) $(CFLAGS) memorymania.o -o memorymania $(LDFLAGS)

memorymania.o: main.c
	$(CC) $(CFLAGS) -c main.c -o memorymania.o $(LDFLAGS)

clean:
	rm -f memorymania memorymania.o
