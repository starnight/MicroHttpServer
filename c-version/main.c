#include "server.h"
#include "middleware.h"
#include "app.h"

/* The HTTP server of this process. */
HTTPServer srv;

#ifdef _PARSE_SIGNAL_
#include <signal.h>

#ifdef _PARSE_SIGNAL_INT_
void SigRoutine_INT(int unused) {
	HTTPServerClose(&srv);
}
#endif
#endif

int main(void) {
	/* Register the routes. */
	AddRoute(HTTP_GET, "/index.html", HelloPage);
	AddRoute(HTTP_GET, "/", HelloPage);
	AddRoute(HTTP_POST, "/fib", Fib);
	/* Initial the HTTP server and make it listening on MHS_PORT. */
	HTTPServerInit(&srv, MHS_PORT);
	/* Run the HTTP server forever. */
	/* Run the dispatch callback if there is a new request */
	HTTPServerRunLoop(&srv, Dispatch);
	HTTPServerClose(&srv);
	return 0;
}
