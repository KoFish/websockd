#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include "worker_fork.h"
#include "client.h"
#include "utils/utils.h"
#include "config.h"

#define LOGF(format, ...) fprintf(stderr, "[worker %i] " format, worker_id, __VA_ARGS__)
#define LOG(format) fprintf(stderr, "[worker %i] " format, worker_id)

#define MATCHES(a, b) (strncmp(a, b, strlen(b)) == 0)

static int worker_id; // This will be set when starting the fork and indicates
					  // the identity of the worker compared to other workers
					  // it is mainly used for debugging.
static client_t *clients; // This is a linked list of all clients currently
						  // connected to the worker.
static ev_io ctrl_sock_w; // This is a watcher for the control socket used
						  // to send new client sockets and commands to the
						  // worker by the main process.

static int read_client(EV_P_ client_t*);
static int write_client(EV_P_ client_t*);
static void send_client(EV_P_ client_t*, char*, ssize_t);
static ssize_t count_clients(void);
static void remove_worker_client(client_t*);
static void client_cb(EV_P_ ev_io*, int);
static void worker_ctrl_cb(EV_P_ ev_io*, int);

// run worker
//
// The entry point of the process, this is the setup functionality which
// actually starts the event loop.
void run_worker(EV_P_ int w, int ctrlfd) {
	worker_id = w;
	clients = NULL;
	ev_io_init(&ctrl_sock_w, worker_ctrl_cb, ctrlfd, EV_READ);
	ev_io_start(EV_A_ &ctrl_sock_w);
	ev_run(EV_A_ 0);
	while (clients != NULL) {
		remove_worker_client(clients);
	}
}

// on client connected
static void client_connected(EV_P_ client_t *client) {
	LOG("client connected\n");
}

static void client_disconnected(EV_P_ client_t *client) {
	LOG("client disconnected\n");
}

static void client_input(EV_P_ client_t *client, char *buff, ssize_t bufflen) {
	send_client(EV_A_ client, strcat(buff, "\n"), bufflen + 1);
}

static void client_output(EV_P_ client_t *client, char *buff, ssize_t bufflen) {
	send_client(EV_A_ client, buff, bufflen);
}

static void disconnect_client(EV_P_ client_t *client) {
	ev_io_stop(EV_A_ &client->write_w);
	ev_io_stop(EV_A_ &client->read_w);
	client_disconnected(EV_A_ client);
	remove_worker_client(client);
}

// client callback
//
// Called whenever a client socket signals that it's ready to read or write
// data.
static void client_cb(EV_P_ ev_io *w, int revents) {
	client_t *client = clients;
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
		if (read_client(EV_A_ client) < 0) {
			disconnect_client(EV_A_ client);
			return;
		}
	}
	if (revents & EV_WRITE) {
		if (write_client(EV_A_ client) < 0) {
			disconnect_client(EV_A_ client);
			return;
		}
	}
	if ((client != NULL) && (client->write_buffer != NULL)) {
		ev_io_start(EV_A_ &client->write_w);
	} else {
		ev_io_stop(EV_A_ &client->write_w);
	}
}

// worker control callback
//
// Called whenever the control command socket signals that it is ready to be
// read.
static void worker_ctrl_cb(EV_P_ ev_io *w, int revents) {
	char buff[CTRL_BUFF_LEN];
	int nfd;
	if (revents & EV_READ) {
		ssize_t c = sock_fd_read(w->fd, buff, CTRL_BUFF_LEN, &nfd);
		if (c == -1) {
			perror("could not read from control socket");
			exit(EXIT_FAILURE);
		}
		buff[c] = '\0';
		if MATCHES(buff, WCMD_END) {
			LOG("got end, terminating");
			ev_break(EV_A_ EVBREAK_ALL);
		} else if MATCHES(buff, WCMD_ADD_CLIENT) {
			setnonblock(nfd);
			client_t *client;
			new_client(&client);
			client->sock = nfd;
			ev_io_init(&(client->read_w), client_cb, nfd, EV_READ);
			ev_io_init(&(client->write_w), client_cb, nfd, EV_WRITE);
			add_client(&clients, client);
			ev_io_start(EV_A_ &(client->read_w));
			client_connected(EV_A_ client);
		} else {
			LOGF("got cmd message, \"%s\" %d\n", buff, nfd);
		}
	} else if (revents & EV_WRITE) {
		LOG("there are no need to respect signal to write to the control "
		    "socket\n");
		ev_io_stop(EV_A_ w);
		ev_io_set(w, w->fd, EV_READ);
		ev_io_start(EV_A_ w);
	}
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

// remove worker client
//
// Removes the client from the workers list of clients and signals to the main
// application that the worker controls one less client.
static void remove_worker_client(client_t *client) {
	client_t *ct = NULL;
	close(client->sock);
	client->sock = -1;
	sock_fd_write(ctrl_sock_w.fd, WCMD_CLIENT_DISCONNECT, strlen(WCMD_CLIENT_DISCONNECT), -1);
	ct = clients;
	if (client == ct) {
		free_client(&clients);
	} else {
		bool removed = false;
		while (ct != NULL) {
			if (ct->next == client) {
				free_client(&ct->next);
				removed = true;
				break;
			}
			ct = ct->next;
		}
		if (!removed) {
			LOG("the client could not be found in the list of clients\n");
			exit(EXIT_FAILURE);
		}
	}
	LOGF("freed client, %i clients left\n", (int)count_clients());
}

static void send_client(EV_P_ client_t *client, char *buff, ssize_t bufflen) {
	add_client_write(client, ">>> ", 4);
	add_client_write(client, buff, bufflen);
	ev_io_start(EV_A_ &client->write_w);
}

static int read_client(EV_P_ client_t *client) {
	char buff[CLIENT_BUFF_LEN];
	ssize_t c = recv(client->sock, buff, CLIENT_BUFF_LEN, 0);
	if (c == -1) {
		perror("could not read from client");
		return 0;
	}
	buff[(c > 0 && buff[c-1] == '\n') ? c-1 : c] = '\0'; // Remove the newline if
													   // the received buff ends
													   // in one.
	LOGF("Receieved message from client: %s\n", buff);
	if (c == 0) {
		// This usually means that the connection has been closed or is sending
		// us empty data. We will be treating it as if the client must have
		// disconnected.
		return -1;
	}
	client_input(EV_A_ client, buff, c);
	return c;
}

static int write_client(EV_P_ client_t *client) {
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
