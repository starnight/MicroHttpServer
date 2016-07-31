/* This file is reference from files of Linux's header files. */

#ifndef __SELECT_H__
#define __SELECT_H__

#include <stdint.h>
#include <sys/types.h>
#include "bits/socket.h"

struct timeval {
	long tv_sec;	/* second */
	long tv_usec;	/* microsecond */
};

typedef uint64_t fd_set;

#define FD_ZERO(__pset)			(*(__pset) = 0)
#define FD_SET(__fd, __pset)	(*(__pset) |= (fd_set)(1 << __fd))
#define FD_CLR(__fd, __pset)	(*(__pset) &= ~((fd_set)(1 << __fd)))
#define FD_ISSET(__fd, __pset)	((*(__pset) & (fd_set)(1 << __fd)) > 0)

int select(SOCKET nfds, fd_set *__readfds, fd_set *__writefds,
			fd_set *__exceptfds, struct timeval *__timeout);

#endif
