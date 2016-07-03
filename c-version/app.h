/* This file declares the server application functions (SAFs). */

#ifndef __APP_H__
#define __APP_H__

#include "server.h"

void HelloPage(HTTPReqMessage *, HTTPResMessage *);
void Fib(HTTPReqMessage *, HTTPResMessage *);

#endif
