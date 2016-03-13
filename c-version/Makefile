PROJ=microhttpserver

CC=cc
INCLUDES=-I.
CFLAGS=$(INCLUDES) -Os
SRCS=server.c

all:
	$(CC) $(SRCS) $(CFLAGS) -o $(PROJ)

clean:
	rm -rf *.out *.bin *.exe *.o *.a *.so test build $(PROJ)
