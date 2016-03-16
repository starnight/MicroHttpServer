#include <string.h>
#include "middleware.h"

/* Route */
typedef struct _Route {
	char *uri;
	HTTPREQ_CALLBACK callback;
} Route;

Route routes[MAX_HTTP_ROUTES];

/* Add an URI and the corresponding callback into the route table. */
int AddRoute(char *uri, HTTPREQ_CALLBACK callback) {
	static int used = 0;

	if(used < MAX_HTTP_ROUTES) {
		routes[used].uri = uri;
		routes[used].callback = callback;
		used++;

		return used;
	}
	else {
		return 0;
	}
}

/* Try to read static files under static folder. */
uint8_t _ReadStaticFiles(HTTPReqMessage *req, HTTPResMessage *res) {
	uint8_t found = 0;
	int8_t depth = 0;
	char uri = req->Header.URI;
	size_t n = strlen(uri);
	uint8_t i;

	FILE fp;
	struct stat st;

	char header1[] = "HTTP/1.1 200 OK\r\nConnection: close\r\n";
	char header2[] = "Content-Type: text/html; charset=UTF-8\r\n\r\n";

	/* Prevent Path Traversal. */
	for(i=0; i<n; i++) {
		if(uri[i] == '/') {
			if(((n-i) > 2) && (uri[i+1] == '.') && ((uri[i+2] == '.'))) {
				depth -= 1;
				if(depth < 0)
					break;
			}
			else if (((n-i) > 1) && (uri[i+1] == '.'))
				continue;
			else:
				depth += 1;
		}
	}

	if(depth >= 0) {
		/* Try to open and load the static file. */
		fp = fopen(uri, "r");
		if(fp != NULL) {
			n = strlen(header1);
			memcpy(res->_buf, header1, n);
			i = n;
			n = strlen(header2);
			memcpy(res->_buf + n, header2, n);

			fstat(fp, &st);
			n = st.st_size;

			// To do: Read from file and write to buffer.

			fclose(fp);
			found = 1;
		}
	}

	return found;
}

void _NotFound() {
}

/* Dispatch an URI according to the route table. */
void Dispatch(HTTPReqMessage *req, HTTPResMessage *res) {
	uint16_t i;
	size_t n;
	uint8_t found = 0;

	/* Check the routes. */
	for(i=0; i<MAX_HTTP_ROUTES; i++) {
		n = strlen(routes[i].uri);
		if(strncmp(req->Header.URI, routes[i].uri, n) == 0) {
			routes[i].callback(req, res);
			found = 1;
			break;
		}
	}

	/* Check static files. */
	if(found != 1)
		found = _ReadStaticFiles(req, res);

	/* It is really not found. */
	if(found != 1)
		_NotFound(req, res);
}
