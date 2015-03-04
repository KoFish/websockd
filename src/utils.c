#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "utils.h"

// Simply adds O_NONBLOCK to the file descriptor of choice
int setnonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
}

char *get_ip_str(struct sockaddr *sa, char **ps)
{
	char *s = malloc(sizeof(char) * INET6_ADDRSTRLEN);
	switch(sa->sa_family) {
		case AF_INET:
		inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
		          s, INET6_ADDRSTRLEN);
		break;

		case AF_INET6:
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
		          s, INET6_ADDRSTRLEN);
		break;

		default:
		sprintf(s, "Unknown AF %i", sa->sa_family);
		// strncpy(s, "Unknown AF", maxlen);
		return NULL;
	}

	(*ps) = s;

	return s;
}
