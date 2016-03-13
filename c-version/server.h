#ifndef __MICRO_HTTP_SERVER_H__
#define __MICRO_HTTP_SERVER_H__

#define MAX_HEADER_SIZE  2048
#define MAX_BODY_SIZE    4096
#ifndef MTS_PORT
#define MTS_PORT         8001
#endif
#ifndef MAX_HTTP_CLIENT
#define MAX_HTTP_CLIENT  5
#endif
#ifndef HTTP_SERVER
#define HTTP_SERVER      "Micro CHTTP Server"
#endif

typedef int SOCKET;

typedef struct _HTTPServer {
	SOCKET sock;
	SOCKET _max_sock;
	fd_set _sock_pool;
} HTTPServer;

typedef struct _HTTPHeader {
	char *Method;
	char *URI;
	char *Version;
	char *ContentType;
} HTTPHeader;

typedef struct _HTTPMessage {
	HTTPHeader Header;
	char *Body;
	uint8_t *_buf;
	uint16_t _index;
} HTTPMessage;

typedef void (*HTTPREQ_CALLBACK)(HTTPMessage *, HTTPMessage *);

#endif
