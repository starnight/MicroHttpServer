PROJ=microhttpserver

CC=cc
INCLUDES=-Ilib
CFLAGS=$(INCLUDES) -Os -D_PARSE_SIGNAL_ -D_PARSE_SIGNAL_INT_ -DDEBUG_MSG -Wall
SRCS=main.c app.c lib/server.c lib/middleware.c

all:
	$(CC) $(SRCS) $(CFLAGS) -o $(PROJ)

clean:
	rm -rf *.out *.bin *.exe *.o *.a *.so *.list *.img test build $(PROJ)
