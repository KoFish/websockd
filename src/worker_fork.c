#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "worker_fork.h"
#include "client.h"
#include "ancmsg.h"
#include "utils.h"

static int worker_id;
static client_t *clients;
static ev_io ctrl_sock_w;

#define LOGF(format, ...) fprintf(stderr, "[worker %i] " format, worker_id, __VA_ARGS__)
#define LOG(format) fprintf(stderr, "[worker %i] " format, worker_id)

#define MATCHES(a, b) (strncmp(a, b, strlen(b)) == 0)

static ssize_t count_clients(void);
static void remove_client(client_t**);
static int read_client(client_t*);
static int write_client(client_t*);

static void client_cb(EV_P_ ev_io *w, int revents) {
	client_t *client = clients;
	int client_flags = EV_READ;
	while (client != NULL) {
		if (client->sock == w->fd) {
			break;
		}
		client = client->next;
	}
	if (client == NULL) {
		ev_io_stop(EV_A_ w);
		LOGF("client_cb: got callback from from unknown client %i\n", w->fd);
	}
	if (revents & EV_READ) {
		if (read_client(client) < 0) {
			ev_io_stop(EV_A_ w);
			remove_client(&client);
			return;
		}
	}
	if (revents & EV_WRITE) {
		if (write_client(client) < 0) {
			ev_io_stop(EV_A_ w);
			remove_client(&client);
			return;
		}
	}
	if ((client != NULL) && (client->write_buffer != NULL)) {
		client_flags = EV_READ | EV_WRITE;
	}
	ev_io_stop(EV_A_ w);
	ev_io_set(w, client->sock, client_flags);
	ev_io_start(EV_A_ w);
}

static void worker_ctrl_cb(EV_P_ ev_io *w, int revents) {
	char buff[500];
	int nfd;
	ssize_t c = sock_fd_read(w->fd, buff, 500, &nfd);
	buff[c] = '\0';
	if MATCHES(buff, WCMD_END) {
		LOG("got end, terminating");
		ev_break(EV_A_ EVBREAK_ALL);
	} else if MATCHES(buff, WCMD_ADD_CLIENT) {
		setnonblock(nfd);
		client_t *client;
		new_client(&client);
		client->sock = nfd;
		ev_io_init(&(client->w), client_cb, nfd, EV_READ);
		add_client(&clients, client);
		ev_io_start(EV_A_ &(client->w));
	} else {
		LOGF("got cmd message, \"%s\" %d\n", buff, nfd);
	}
}

void run_worker(EV_P_ int w, int ctrlfd) {
	worker_id = w;
	clients = NULL;
	ev_io_init(&ctrl_sock_w, worker_ctrl_cb, ctrlfd, EV_READ);
	ev_io_start(EV_A_ &ctrl_sock_w);
	LOGF("worker-%d starting loop\n", worker_id);
	ev_run(EV_A_ 0);
	while (clients != NULL) {
		remove_client(&clients);
	}
	LOGF("worker-%d finished\n", worker_id);
}

static ssize_t count_clients(void) {
	client_t *cc = clients;
	ssize_t cnt = 0;
	while (cc != NULL) {
		cnt += 1;
		cc = cc->next;
	}
	return cnt;
}

static void remove_client(client_t **client) {
	client_t *ct = NULL;
	close((*client)->sock);
	(*client)->sock = -1;
	sock_fd_write(ctrl_sock_w.fd, WCMD_CLIENT_DISCONNECT, strlen(WCMD_CLIENT_DISCONNECT), -1);
	ct = clients;
	while (ct != NULL) {
		if (ct->next == (*client)) {
			free_client(&ct->next);
			break;
		}
		ct = ct->next;
	}
	LOGF("freed client, %i clients left\n", (int)count_clients());
	(*client) = NULL;
}

static int read_client(client_t *client) {
	char buff[500];
	ssize_t c = recv(client->sock, buff, 500, 0);
	buff[c > 0 ? c-1 : c] = '\0';
	LOGF("Receieved message from client: %s\n", buff);
	if (c == 0) {
		return -1;
	}
	add_write(client, buff, c);
	return c;
}

static int write_client(client_t *client) {
	if (client->write_buffer != NULL) {
		char *b = client->write_buffer->buff;
		ssize_t blen = client->write_buffer->bufflen;
		ssize_t c = send(client->sock, b, blen, 0);
		if (blen > c) {
			remove_written(client->write_buffer, c);
		} else {
			client->write_buffer = client->write_buffer->next;
		}
		return client->write_buffer == NULL ? 0 : 1;
	} else {
		return -1;
	}
}
