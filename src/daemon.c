/*
 *  Copyright 2015 Krister Svanlund <krister.svanlund@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */


#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ev.h>

#include "config.h"
#include "worker.h"
#include "utils/utils.h"

#define LOG(format) fprintf(stderr, "[main] " format)
#define LOGF(format, ...) fprintf(stderr, "[main] " format, __VA_ARGS__)


static int setup_accept_socket(EV_P) {
	int sock, s, yes=1;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	s = getaddrinfo(WS_HOST, WS_PORT, &hints, &result);
	if (s != 0) {
		perror("getaddrinfo");
		exit(EXIT_FAILURE);
	}
	for (rp = result; rp != NULL ; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol);
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != -1) {
			if (sock == -1) continue;
			if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;
		}
		close(sock);
	}
	if (listen(sock, 2) == -1) {
		perror("listen");
	}
	if (rp == NULL) {
		LOGF("Could not bind to (%s %s)\n", WS_HOST, WS_PORT);
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	return sock;
}

static void accept_cb(EV_P_ ev_io *w, int revents) {
	struct sockaddr_in client;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	int sock = accept(w->fd, (struct sockaddr*)&client, &addrlen);
	char ip_addr[INET6_ADDRSTRLEN];
	int remote_port;
	if (get_ip_str((struct sockaddr*)&client, ip_addr, INET6_ADDRSTRLEN) == NULL) {
		perror("accept_cb");
		close(sock);
	}
	remote_port = ntohs(client.sin_port);
	LOGF("got connection from %s:%i\n", ip_addr, remote_port);
	add_client_socket(EV_A_ sock);
}

static void int_cb(EV_P_ ev_signal *w, int revents) {
	LOG("ctrl-c\n");
	stop_workers(EV_A);
	ev_signal_stop(EV_A_ w);
}

int main(int argc, char *argv[]) {
	struct ev_loop *loop = ev_default_loop(EVFLAG_NOENV | EVFLAG_FORKCHECK);
	init_workers(EV_A);
	int sock = setup_accept_socket(EV_A);
	ev_signal int_w;
	ev_signal_init(&int_w, int_cb, SIGINT);
	ev_signal_start(EV_A_ &int_w);
	ev_io accept_w;
	ev_io_init(&accept_w, accept_cb, sock, EV_READ);
	ev_io_start(EV_A_ &accept_w);
	ev_run(EV_A_ 0);
	print_workers();
	return 0;
}
