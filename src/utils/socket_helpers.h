/**
 * Helper functions for dealing with sockets
 */
#ifndef __SOCKET_HELPERS_H__
#define __SOCKET_HELPERS_H__

#include <sys/types.h>
#include <sys/socket.h>

int setnonblock(int fd);
char *get_ip_str(struct sockaddr *sa, char *s, ssize_t maxlen);

#endif // __SOCKET_HELPERS_H__
