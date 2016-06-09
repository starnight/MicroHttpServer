#ifndef __APP_H__
#define __APP_H__

#include "server.h"

void HelloPage(HTTPReqMessage *, HTTPResMessage *);
void Fib(HTTPReqMessage *, HTTPResMessage *);
void LED(HTTPReqMessage *req, HTTPResMessage *res);

#endif
