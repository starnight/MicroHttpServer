PROJ=microhttpserver

CC=cc
INCLUDES=-I.
CFLAGS=$(INCLUDES) -O0 -D_PARSE_SIGNAL_ -D_PARSE_SIGNAL_INT_ -DDEBUG_MSG
SRCS=main.c app.c server.c middleware.c

all:
	$(CC) $(SRCS) $(CFLAGS) -o $(PROJ)

clean:
	rm -rf *.out *.bin *.exe *.o *.a *.so *.list *.img test build $(PROJ)
