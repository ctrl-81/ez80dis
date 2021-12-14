# minimal Makefile
all: ez80dis

CC=cc
CFLAGS=-std=gnu99 -Wall -Wno-format

OBJS= \
src/dis.o \
src/main.o \
src/opcode.o \
src/sym.o

DEPS= \
src/dis.h \
src/opcode.h \
src/sym.h

ez80dis: $(OBJS)
	$(CC) $(CFLAGS) -o ez80dis $(OBJS)

$(OBJS): %.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm src/*.o
