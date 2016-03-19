#include <string.h>
#include "server.h"
#include "middleware.h"

void HelloPage(HTTPReqMessage *req, HTTPResMessage *res) {
	int n, i = 0, j;
	char *p;
	char header1[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	char header2[] = "Content-Type: text/html; charset=UTF-8\r\n\r\n";
	char body1[] = "<html><body>Hello!<br>許功蓋<br>";
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
	n = strlen(req->Header.Method);
	memcpy(p, req->Header.Method, n);
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
	AddRoute("/index.html", HelloPage);
	AddRoute("/", HelloPage);
	HTTPServerInit(&srv, MTS_PORT);
	HTTPServerListen(&srv, Dispatch);
	HTTPServerClose(&srv);
	return 0;
}
