#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include "server.h"

typedef void (*SOCKET_CALLBACK)(void *);

#define NOTWORK_SOCKET      0
#define READING_SOCKET      1
#define READEND_SOCKET      2
#define WRITING_SOCKET      4
#define WRITEEND_SOCKET     8
#define CLOSE_SOCKET        128
#define IsReqReading(s)     (s == READING_SOCKET)
#define IsReqWriting(s)     (s == WRITING_SOCKET)
#define IsReqReadEnd(s)     (s == READEND_SOCKET)
#define IsReqWriteEnd(s)    (s == WRITEEND_SOCKET)
#define IsReqClose(s)       (s == CLOSE_SOCKET)

typedef struct _HTTPReq {
	SOCKET clisock;
	HTTPReqMessage req;
	HTTPResMessage res;
	SOCKET_CALLBACK OnRead;
	SOCKET_CALLBACK EndRead;
	SOCKET_CALLBACK OnWrite;
	SOCKET_CALLBACK EndWrite;
	size_t rindex;
	size_t windex;
	uint8_t work_state;
} HTTPReq;

HTTPReq http_req[MAX_HTTP_CLIENT];
uint8_t req_buf[MAX_HTTP_CLIENT][MAX_HEADER_SIZE + MAX_BODY_SIZE];
uint8_t res_buf[MAX_HTTP_CLIENT][MAX_HEADER_SIZE + MAX_BODY_SIZE];

int HTTPServerInit(HTTPServer *srv, uint16_t port) {
	struct sockaddr_in srv_addr;
	unsigned int i;

	/* Have a server socket. */
	srv->sock = socket(AF_INET, SOCK_STREAM, 0);
	if(srv->sock < 0)
		return -1;
	/* Set server address. */
	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	/* Set the server socket can reuse the address. */
	setsockopt(srv->sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	/* Bind the server socket with the server address. */
	if(bind(srv->sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) == -1) {
		HTTPServerClose(srv);
		return -1;
	}
	/* Set the server socket non-blocking. */
	fcntl(srv->sock, F_SETFL, O_NONBLOCK);

	/* Start server socket listening. */
	DebugMsg("Listening\n");
	listen(srv->sock, MAX_HTTP_CLIENT);

	/* Append server socket to the master socket queue. */
	FD_ZERO(&(srv->_read_sock_pool));
	FD_ZERO(&(srv->_write_sock_pool));
	FD_SET(srv->sock, &(srv->_read_sock_pool));
	/* The server socket's FD is max in the master socket queue for now. */
	srv->_max_sock = srv->sock;

	/* Prepare the HTTP client requests pool. */
	for(i=0; i<MAX_HTTP_CLIENT; i++) {
		http_req[i].req._buf = req_buf[i];
		http_req[i].res._buf = res_buf[i];
		http_req[i].clisock = -1;
		http_req[i].work_state = NOTWORK_SOCKET;
	}
	return 0;
}

void _HTTPServerAccept(HTTPServer *srv) {
	struct sockaddr_in cli_addr;
	socklen_t sockaddr_len = sizeof(cli_addr);
	SOCKET clisock;
	unsigned int i;

	/* Have the client socket and append it to the master socket queue. */
	clisock = accept(srv->sock, (struct sockaddr*) &cli_addr, &sockaddr_len);
	if(clisock != -1) {
		FD_SET(clisock, &(srv->_read_sock_pool));
		/* Set the client socket non-blocking. */
		//fcntl(clisock, F_SETFL, O_NONBLOCK);
		/* Set the max socket file descriptor. */
		if(clisock > srv->_max_sock) srv->_max_sock = clisock;
		DebugMsg("Accept 1 client.  %s:%d\n",
					inet_ntoa(cli_addr.sin_addr),
					(int)ntohs(cli_addr.sin_port));
		/* Add into HTTP client requests pool. */
		for(i=0; i<MAX_HTTP_CLIENT; i++) {
			if(http_req[i].clisock == -1) {
				http_req[i].clisock = clisock;
				http_req[i].req.Header.Amount = 0;
				http_req[i].res.Header.Amount = 0;
				http_req[i].rindex = 0;
				http_req[i].windex = 0;
				break;
			}
		}
	}
}

int _CheckLine(char *buf) {
	int i = 0;

	if(buf[i] == '\n') {
		if(buf[i - 1] == '\r')
			i = 2;
		else
			i = 1;
	}

	return i;
}

int _CheckFieldSep(char *buf) {
	int i = 0;

	if((buf[i - 1] == ':') && (buf[i] == ' ')) {
		i = 2;
	}

	return i;
}

/* Max length of HTTP method adds 1 for space */
#define MAX_HTTP_METHOD_LEN	(6 + 1)

HTTPMethod HaveMethod(char *method) {
	HTTPMethod m;

	if(memcmp(method, "GET", 3) == 0)
		m = HTTP_GET;
	else if(memcmp(method, "POST", 4) == 0)
		m = HTTP_POST;
	else if(memcmp(method, "PUT", 3) == 0)
		m = HTTP_PUT;
	else if(memcmp(method, "DELETE", 6) == 0)
		m = HTTP_DELETE;
	else
		m = HTTP_GET;

	return m;
}

void WriteSock(HTTPReq *hr) {
	ssize_t n;

	n = send(hr->clisock,
				hr->res._buf + hr->windex,
				hr->res._index - hr->windex,
				MSG_DONTWAIT);
	if(n > 0) {
		/* Send some bytes and send left next loop. */
		hr->windex += n;
		if(hr->res._index > hr->windex)
			hr->work_state = WRITING_SOCKET;
		else
			hr->work_state = WRITEEND_SOCKET;
	}
	else if(n == 0) {
		/* Writing is finished. */
		hr->work_state = WRITEEND_SOCKET;
	}
	else if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
		/* Send with non-blocking socket. */
		hr->windex += hr->res._index - hr->windex;
		hr->work_state = WRITING_SOCKET;
	}
	else {
		/* Send with error. */
		hr->work_state = CLOSE_SOCKET;
	}
}

int _ParseHeader(HTTPReq *hr) {
	SOCKET clisock = hr->clisock;
	HTTPReqMessage *req = &(hr->req);
	int n;
	int l, end;
	int i = 0;
	char *p;

	DebugMsg("\tParse Header\n");
	p = (char *)req->_buf;
	memset(p, 0, MAX_HEADER_SIZE + MAX_BODY_SIZE);
	req->_index = 0;

	/* GET, PUT ... and a white space are 3 charaters. */
	n = recv(clisock, p, 3, 0);
	if(n == 3) {
		/* Parse method. */
		for(i = 3; n>0; i++) {
			n = recv(clisock, p + i, 1, 0);
			if(n == -EAGAIN || n == -EWOULDBLOCK) {
				n = 1;
				i--;
				continue;
			} else if (n <= 0) {
				return 0;
			}

			if(p[i] == ' ') {
				p[i] = '\0';
				i += 1;
				req->_index = i;
				break;
			}

			if(i > MAX_HTTP_METHOD_LEN - 2)
				return 0;
		}
		req->Header.Method = HaveMethod(p);

		/* Parse URI. */
		p += i;
		req->Header.URI = p;
		for(i = 0; n>0; i++) {
			n = recv(clisock, p + i, 1, 0);
			if(n == -EAGAIN || n == -EWOULDBLOCK) {
				n = 1;
				i--;
				continue;
			} else if (n <= 0) {
				return 0;
			}

			if(p[i] == ' ') {
				p[i] = '\0';
				i += 1;
				req->_index += i;
				break;
			}

			if(i > MAX_HTTP_URI_LEN - 2) {
				strncpy(req->Header.URI, "notfound", 9);
				req->_index += 9;
				return req->_index;
			}
		}

		/* Parse HTTP version. */
		p += i;
		req->Header.Version = p;
		for(i = 0; n>0; i++) {
			n = recv(clisock, p + i, 1, 0);
			if(n == -EAGAIN || n == -EWOULDBLOCK) {
				n = 1;
				i--;
				continue;
			} else if (n <= 0) {
				return 0;
			}

			/* HTTP/1.1 has 8 charaters */
			if(i >= 8 && (l = _CheckLine(p + i))) {
				p[i] = '\0';
				i += 1;
				if(l == 2) {
					p[i - 1] = '\0';
					i -= 1;
				}
				req->_index += i;
				break;
			}

			/* 8 charaters adds 2 characters for break line */
			if(i > 8 + 2 - 2)
				return 0;
		}

		/* Parse other fields. */
		p += i;
		req->Header.Fields[req->Header.Amount].key = p;
		end = 0;
		for(i = 0; (n>0) && (req->_index+i < MAX_HEADER_SIZE) && (req->Header.Amount < MAX_HEADER_FIELDS); i++) {
			n = recv(clisock, p + i, 1, 0);
			/* Check field key name end. */
			if((l = _CheckFieldSep(p + i))) {
				p[i - 1] = '\0';
				req->Header.Fields[req->Header.Amount].value = p + i + 1;
			}

			/* Check header end. */
			if((l = _CheckLine(p + i))) {
				if(end == 0) {
					p[i] = '\0';
					if(l == 2) {
						p[i - 1] = '\0';
						i -= 1;
					}

					/* CRLF have 2 characters, so check 2 times new line. */
					end = 2;

					/* Go to parse next header field. */
					req->Header.Amount += 1;
					req->Header.Fields[req->Header.Amount].key = p + i + 1;
				}
				else {
					/* Requset message header finished. */
					break;
				}
			}
			else {
				if(end > 0) end -= 1;
			}
		}
		req->_index += i;
	}
	if(n < 0) {
		hr->work_state = CLOSE_SOCKET;
	}

	return req->_index;
}

int _IsLengthHeader(char *key) {
	const char *len_header = "content-length";

	return !strncasecmp(key, len_header, strlen(len_header));
}

int _GetBody(HTTPReq *hr) {
	SOCKET clisock = hr->clisock;
	HTTPReqMessage *req = &(hr->req);
	int n = 1;
	unsigned int i = 0;
	int c = 0, len = 0;
	uint8_t *p;

	DebugMsg("\tParse body\n");
	req->Body = req->_buf + req->_index;

	if(req->Header.Method == HTTP_POST) {
		for(i=0; i<req->Header.Amount; i++) {
			if(_IsLengthHeader(req->Header.Fields[i].key)) {
				len = atoi(req->Header.Fields[i].value);
				break;
			}
		}
		p = req->Body;
		if(len > MAX_BODY_SIZE) len = MAX_BODY_SIZE;
		for(c=0; (n>0) && (c<len); c+=n) {
			n = recv(clisock, p + c, (len-c), MSG_PEEK);
		}
	}

	req->Body[c] = '\0';

	return (n < 0) ? -1 : c;
}

void _HTTPServerRequest(HTTPReq *hr, HTTPREQ_CALLBACK callback) {
	int n;

	hr->work_state = READING_SOCKET;
	n = _ParseHeader(hr);
	if(n > 0) {
		n = _GetBody(hr);
		if(n >= 0) {
			callback(&(hr->req), &(hr->res));
			/* Write all response. */
			hr->work_state = WRITING_SOCKET;
		}
		else {
			hr->work_state = CLOSE_SOCKET;
		}
	}
	else {
		hr->work_state = CLOSE_SOCKET;
	}
}

void HTTPServerRun(HTTPServer *srv, HTTPREQ_CALLBACK callback) {
	fd_set readable, writeable;
	struct timeval timeout = {0, 0};
	uint16_t i;

	if (srv->sock < 0)
		return;

	/* Copy master socket queue to readable, writeable socket queue. */
	readable = srv->_read_sock_pool;
	writeable = srv->_write_sock_pool;
	/* Wait the flag of any socket in readable socket queue. */
	select(srv->_max_sock+1, &readable, &writeable, NULL, &timeout);
	/* Check server socket is readable. */
	if(FD_ISSET(srv->sock, &readable)) {
		/* Accept when server socket has been connected. */
		_HTTPServerAccept(srv);
	}
	/* Check sockets in HTTP client requests pool are readable. */
	for(i=0; i<MAX_HTTP_CLIENT; i++) {
		if(http_req[i].clisock != -1) {
			//s = &(http_req[i].clisock);
			if(FD_ISSET(http_req[i].clisock, &readable)) {
				/* Deal the request from the client socket. */
				_HTTPServerRequest(&(http_req[i]), callback);
				FD_SET(http_req[i].clisock, &(srv->_write_sock_pool));
				FD_CLR(http_req[i].clisock, &(srv->_read_sock_pool));
			}
			if(IsReqWriting(http_req[i].work_state)
				&& FD_ISSET(http_req[i].clisock, &writeable)) {
				WriteSock(http_req + i);
			}
			if(IsReqWriteEnd(http_req[i].work_state)) {
				http_req[i].work_state = CLOSE_SOCKET;
			}
			if(IsReqClose(http_req[i].work_state)) {
				shutdown(http_req[i].clisock, SHUT_RDWR);
				close(http_req[i].clisock);
				FD_CLR(http_req[i].clisock, &(srv->_write_sock_pool));
				if(http_req[i].clisock >= srv->_max_sock)
					srv->_max_sock -= 1;
				http_req[i].clisock = -1;
				http_req[i].work_state = NOTWORK_SOCKET;
			}
		}
	}
}

void HTTPServerClose(HTTPServer *srv) {
	if (srv->sock < 0)
		return;
	shutdown(srv->sock, SHUT_RDWR);
	close((srv)->sock);
	srv->sock = -1;
}

#ifdef MICRO_HTTP_SERVER_EXAMPLE
/* This is exmaple. */
void _HelloPage(HTTPReqMessage *req, HTTPResMessage *res) {
	int n, i = 0, j;
	char *p;
	char header1[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	char header2[] = "Content-Type: text/html; charset=UTF-8\r\n\r\n";
	char body1[] = "<html><body>許功蓋 Hello <br>";
	char body2[] = "</body></html>";

	/* Build header. */
	p = res->_buf;	
	n = strlen(header1);
	memcpy(p, header1, n);
	p += n;
	i += n;
	
	n = strlen(header2);
	memcpy(p, header2, n);
	p += n;
	i += n;
	
	/* Build body. */
	n = strlen(body1);
	memcpy(p, body1, n);
	p += n;
	i += n;

	/* Echo request header into body. */
	n = strlen(req->_buf);
	memcpy(p, req->_buf, n);
	p += n;
	i += n;

	n = strlen("<br>");
	memcpy(p, "<br>", n);
	p += n;
	i += n;

	n = strlen(req->Header.URI);
	memcpy(p, req->Header.URI, n);
	p += n;
	i += n;

	n = strlen("<br>");
	memcpy(p, "<br>", n);
	p += n;
	i += n;

	n = strlen(req->Header.Version);
	memcpy(p, req->Header.Version, n);
	p += n;
	i += n;

	for(j=0; j<req->Header.Amount; j++) {
		n = strlen("<br>");
		memcpy(p, "<br>", n);
		p += n;
		i += n;

		n = strlen(req->Header.Fields[j].key);
		memcpy(p, req->Header.Fields[j].key, n);
		p += n;
		i += n;

		p[0] = ':'; p[1] = ' ';
		p += 2;
		i += 2;

		n = strlen(req->Header.Fields[j].value);
		memcpy(p, req->Header.Fields[j].value, n);
		p += n;
		i += n;
	}

	n = strlen(body2);
	memcpy(p, body2, n);
	i += n;

	res->_index = i;
}

int main(void) {
	HTTPServer srv;
	HTTPServerInit(&srv, MHS_PORT);
	while(1) { HTTPServerRun(&srv, _HelloPage); }
	HTTPServerClose(&srv);
	return 0;
}

#endif
