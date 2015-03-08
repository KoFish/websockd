#include "ancmsg.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

ssize_t sock_fd_write(int sock, const char *buf, ssize_t buflen, int fd) {
	ssize_t size;
	char buff[buflen];
	strcpy(buff, buf);
	struct msghdr msg = {0};
	struct iovec iov;
	struct cmsghdr *cmsg;
	union {
		char control[CMSG_SPACE(sizeof(int))];
		struct cmsghdr align;
	} u;
	iov.iov_base = buff;
	iov.iov_len = buflen;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	if (fd != -1) {
		msg.msg_control = u.control;
		msg.msg_controllen = sizeof(u.control);
		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		*((int *) CMSG_DATA(cmsg)) = fd;
		msg.msg_controllen = cmsg->cmsg_len;
	} else {
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
	}
	size = sendmsg(sock, &msg, 0);
	if (size < 0) perror("sendmsg");
	return size;
}

ssize_t sock_fd_read(int sock, char *buf, ssize_t buflen, int *fd) {
	ssize_t size;
	if (fd) {
		struct msghdr msg;
		struct iovec iov;
		union {
			char control[CMSG_SPACE(sizeof(int))];
			struct cmsghdr align;
		} u;
		struct cmsghdr *cmsg;
		iov.iov_base = buf;
		iov.iov_len = buflen;
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = u.control;
		msg.msg_controllen = sizeof(u.control);
		size = recvmsg(sock, &msg, 0);
		if (size < 0) {
			perror("sock_fd_read");
			exit(EXIT_FAILURE);
		}
		cmsg = CMSG_FIRSTHDR(&msg);
		if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
			if (cmsg->cmsg_level != SOL_SOCKET) {
				fprintf(stderr, "invalid cmsg_level, %d\n", cmsg->cmsg_level);
				exit(EXIT_FAILURE);
			}
			if (cmsg->cmsg_type != SCM_RIGHTS) {
				fprintf(stderr, "invalid cmsg_type, %d\n", cmsg->cmsg_level);
				exit(EXIT_FAILURE);
			}

			*fd = *((int *) CMSG_DATA(cmsg));
		} else {
			*fd = -1;
		}
	} else {
		size = read(sock, buf, buflen);
		if (size < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}
	}
	return size;
}
