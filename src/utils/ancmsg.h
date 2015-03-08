/**
 * Functions to send socket as ancillary data over AF_UNIX socket.
 */
#ifndef __ANCMSG_H__
#define __ANCMSG_H__

#include <unistd.h>

ssize_t sock_fd_write(int sock, const char *msg, ssize_t msg_size, int fd);
ssize_t sock_fd_read(int sock, char *msg, ssize_t msg_size, int *fd);

#endif  // __ANCMSG_H__
