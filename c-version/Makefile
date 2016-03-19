PROJ=microhttpserver

CC=cc
INCLUDES=-I.
CFLAGS=$(INCLUDES) -Os -D_PARSE_SIGNAL_ -D_PARSE_SIGNAL_INT_
SRCS=main.c server.c middleware.c

all:
	$(CC) $(SRCS) $(CFLAGS) -o $(PROJ)

clean:
	rm -rf *.out *.bin *.exe *.o *.a *.so test build $(PROJ)
