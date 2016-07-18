PROJ=microhttpserver

CC=cc
INCLUDES=-Ilib
DEFS=-D_PARSE_SIGNAL_ -D_PARSE_SIGNAL_INT_ -DDEBUG_MSG -DENABLE_STATIC_FILE=1
CFLAGS=-Os -Wall
SRCS=main.c app.c lib/server.c lib/middleware.c

all:
	$(CC) $(SRCS) $(INCLUDES) $(DEFS) $(CFLAGS) -o $(PROJ)

clean:
	rm -rf *.out *.bin *.exe *.o *.a *.so *.list *.img test build $(PROJ)
