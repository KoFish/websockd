#ifndef __ANCMSG_H__
#define __ANCMSG_H__

#include <unistd.h>

ssize_t sock_fd_write(int, const char *, ssize_t, int);
ssize_t sock_fd_read(int, char *, ssize_t, int*);

#endif  // __ANCMSG_H__
