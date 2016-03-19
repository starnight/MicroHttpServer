#ifndef __MICRO_HTTP_MIDDLEWARE_H__
#define __MICRO_HTTP_MIDDLEWARE_H__

#include "server.h"

/* Route */
#ifndef MAX_HTTP_ROUTES
#define MAX_HTTP_ROUTES  10
#endif
#ifndef STATIC_FILE_FOLDER
#define STATIC_FILE_FOLDER "static/"
#endif

int AddRoute(char *, HTTPREQ_CALLBACK);
void Dispatch(HTTPReqMessage *, HTTPResMessage *);

#endif
