#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "socket_helpers.h"

// Simply adds O_NONBLOCK to the file descriptor of choice
int setnonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
}

static void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*)sa)->sin_addr);
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char *get_ip_str(struct sockaddr *sa, char *s, ssize_t maxlen) {
	if (inet_ntop(sa->sa_family, get_in_addr((struct sockaddr *)sa), s, maxlen) == NULL) {
		return NULL;
	}
	return s;
}
