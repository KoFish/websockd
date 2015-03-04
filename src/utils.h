#ifndef __UTILS_H__
#define __UTILS_H__

#include <sys/types.h>
#include <sys/socket.h>

int setnonblock(int fd);
char *get_ip_str(struct sockaddr *sa, char *s, ssize_t maxlen);

#endif // __UTILS_H__
