#ifndef __MICRO_HTTP_MIDDLEWARE_H__
#define __MICRO_HTTP_MIDDLEWARE_H__

#include "server.h"

/* Route */
#ifndef MAX_HTTP_ROUTES
#define MAX_HTTP_ROUTES  10
#endif
#if (ENABLE_STATIC_FILE == 1) && !(defined STATIC_FILE_FOLDER)
#define STATIC_FILE_FOLDER "static/"
#endif

/* Data type of server application function */
typedef HTTPREQ_CALLBACK SAF;

int AddRoute(HTTPMethod, char *, SAF);
void Dispatch(HTTPReqMessage *, HTTPResMessage *);

#endif
