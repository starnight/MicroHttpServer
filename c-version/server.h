#ifndef __MICRO_HTTP_SERVER_H__
#define __MICRO_HTTP_SERVER_H__

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

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

typedef struct _HTTPHeaderField {
	char *key;
	char *value;
} HTTPHeaderField;

#ifndef MAX_HEADER_FIELDS
#define MAX_HEADER_FIELDS  20
#endif

typedef struct _HTTPReqHeader {
	char *Method;
	char *URI;
	char *Version;
	HTTPHeaderField Fields[MAX_HEADER_FIELDS];
	unsigned int Amount;
} HTTPReqHeader;

typedef struct _HTTPReqMessage {
	HTTPReqHeader Header;
	char *Body;
	uint8_t *_buf;
	uint16_t _index;
} HTTPReqMessage;

typedef struct _HTTPResHeader {
	char *Version;
	char *StatusCode;
	char *Description;
	HTTPHeaderField Fields[MAX_HEADER_FIELDS];
	unsigned int Amount;
} HTTPResHeader;

typedef struct _HTTPResMessage {
	HTTPResHeader Header;
	char *Body;
	uint8_t *_buf;
	uint16_t _index;
} HTTPResMessage;

typedef void (*HTTPREQ_CALLBACK)(HTTPReqMessage *, HTTPResMessage *);

void HTTPServerInit(HTTPServer *, uint16_t);
void HTTPServerListen(HTTPServer *, HTTPREQ_CALLBACK);
void HTTPServerClose(HTTPServer *);

#ifdef DEBUG_MSG
#include <stdio.h>
#define DebugMsg(...) (printf(__VA_ARGS__))
#else
#define DebugMsg(...)
#endif

#endif
