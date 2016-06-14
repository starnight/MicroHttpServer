#ifndef __MICRO_HTTP_SERVER_H__
#define __MICRO_HTTP_SERVER_H__

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_HEADER_SIZE  1024
#define MAX_BODY_SIZE    2048
#ifndef MHS_PORT
#define MHS_PORT         8001
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
	fd_set _read_sock_pool;
	fd_set _write_sock_pool;
} HTTPServer;

typedef struct _HTTPHeaderField {
	char *key;
	char *value;
} HTTPHeaderField;

#ifndef MAX_HEADER_FIELDS
#define MAX_HEADER_FIELDS  20
#endif

typedef enum {
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_NUM_METHOD
} HTTPMethod;

typedef struct _HTTPReqHeader {
	HTTPMethod Method;
	char *URI;
	char *Version;
	HTTPHeaderField Fields[MAX_HEADER_FIELDS];
	unsigned int Amount;
} HTTPReqHeader;

typedef struct _HTTPReqMessage {
	HTTPReqHeader Header;
	uint8_t *Body;
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
	uint8_t *Body;
	uint8_t *_buf;
	uint16_t _index;
} HTTPResMessage;

typedef void (*HTTPREQ_CALLBACK)(HTTPReqMessage *, HTTPResMessage *);

void HTTPServerInit(HTTPServer *, uint16_t);
void HTTPServerRun(HTTPServer *, HTTPREQ_CALLBACK);
#define HTTPServerRunLoop(srv, callback) { \
	while(1) { \
		HTTPServerRun(srv, callback); \
	}}
void HTTPServerClose(HTTPServer *);

#ifdef DEBUG_MSG
#include <stdio.h>
#define DebugMsg(...) (printf(__VA_ARGS__))
#else
#define DebugMsg(...)
#endif

#endif
