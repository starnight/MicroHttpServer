/* This file defines the server application functions (SAFs). */

#include <string.h>
#include <stdlib.h>
#include "app.h"

void HelloPage(HTTPReqMessage *req, HTTPResMessage *res) {
	int n, i = 0, j;
	char *p;
	char header[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n"
					"Content-Type: text/html; charset=UTF-8\r\n\r\n";
	char body1[] = "<html><body>Hello!<br>許功蓋<br>";
	char body2[] = "</body></html>";

	/* Build header. */
	p = (char *)res->_buf;
	n = strlen(header);
	memcpy(p, header, n);
	p += n;
	i += n;

	/* Build body. */
	n = strlen(body1);
	memcpy(p, body1, n);
	p += n;
	i += n;

	/* Echo request header into body. */
	n = strlen((char *)req->_buf);
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

char *itoa(int n, char *s, int b) {
	char digits[] = "0123456789ABCDEF";
	uint8_t i = 0;
	int shift;
	char *p;

	p = s;

	/* Deal the sign. */
	if(n < 0) {
		s[0] = '-';
		n = -n;
		p += 1;
	}

	/* Convert integer to characters. */
	shift = n;
	if(shift == 0)
		i = 1;
	else
		for(; shift > 0; i++)
			shift /= b;

	p[i] = '\0';

	do {
		i--;
		p[i] = digits[n % b];
	} while((n /= b) > 0);

	return s;
}

int fibnacci(int l) {
	int sum = 0;
	int ppre = 0, pre = 1;

	if(l == 2)
		sum = 1;
	else {
		for(l -= 2; l > 0; l--) {
			sum = ppre + pre;
			ppre = pre;
			pre = sum;
		}
	}

	return sum;
}

void Fib(HTTPReqMessage *req, HTTPResMessage *res) {
	int n, i = 0;
	char *p;
	char header[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n"
					"Content-Type: text/text; charset=UTF-8\r\n\r\n";
	int l;
	char *str;

	/* Build header. */
	p = (char *)res->_buf;
	n = strlen(header);
	memcpy(p, header, n);
	p += n;
	i += n;

	/* Build body. */
	str = strstr((char *)req->Body, "Level=");
	if(str != NULL) {
		l = atoi(str + 6);
		n = strlen(itoa(fibnacci(l), p, 10));
		i += n;
	}
	else {
		memcpy(p, "Wrong", 6);
		i += 6;
	}

	res->_index = i;
}
