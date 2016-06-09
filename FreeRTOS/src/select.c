#include "sys/select.h"
#include "bits/mac_esp8266.h"

#include <stdio.h>
#include "usart.h"

int select(SOCKET nfds, fd_set *__readfds, fd_set *__writefds,
			fd_set *__exceptfds, struct timeval *__timeout) {
	SOCKET i;
	int c;

	c = 0;
	/* Go through interested sockets. */
	for(i = SOCKET_BASE; i < nfds; i++) {
		if((__readfds != NULL) && FD_ISSET(i, __readfds)) {
			if(IsSocketReady2Read(i)) {
				/* The interested socket is ready to be read. */
				c++;
			}
			else {
				/* The interested socket is not ready to be read. */
				FD_CLR(i, __readfds);
			}
		}
		if((__writefds != NULL) && FD_ISSET(i, __writefds)) {
			if(IsSocketReady2Write(i)) {
				/* The interested socket is ready to be written. */
				c++;
			}
			else {
				/* The interested socket is not ready to be written. */
				FD_CLR(i, __writefds);
			}
		}
		if((__exceptfds != NULL) && FD_ISSET(i, __exceptfds)) {
			// To do: List exception sockets.
		}
	}

	return c;
}
