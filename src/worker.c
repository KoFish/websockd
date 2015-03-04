#include <ev.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "ancmsg.h"
#include "utils.h"
#include "worker.h"
#include "client.h"
#include "worker_fork.h"

#define LOG(format) fprintf(stderr, "[worker] " format)
#define LOGF(format, ...) fprintf(stderr, "[worker] " format, __VA_ARGS__)

static worker_t workers[MAX_PROCS];
static int worker_counter;
static bool keep_running;
static ev_check check_w;

static int start_worker(EV_P);
static void stop_worker(int);
// static void any_child_cb(EV_P_ ev_child *w, int revents);

static void check_cb(EV_P_ ev_check *w, int revents) {
	LOGF("workers: %i%s\n", worker_counter, keep_running ? "" : " (stopping)");
	if (!keep_running && (worker_counter == 0)) {
		ev_check_stop(EV_A_ w);
		ev_break(EV_A_ EVBREAK_ONE);
	}
}

static void dismiss_client(int fd, char *msg) {
	send(fd, msg, strlen(msg), 0);
	close(fd);
}

void init_workers(EV_P) {
	keep_running = true;
	worker_counter = 0;
	memset((void*)workers, 0, sizeof(worker_t) * MAX_PROCS);
	ev_check_init(&check_w, check_cb);
	ev_check_start(EV_A_ &check_w);
	for (int w = 0 ; w < MAX_PROCS ; w++) {
		start_worker(EV_A);
	}
	LOG("initiated workers:");
	print_workers();
}

void add_client_socket(EV_P_ int cfd) {
	int wrk = -1;
	if (!keep_running) {
		dismiss_client(cfd, "Shutting down");
		return;
	}
	if (worker_counter < MAX_PROCS) {
		if ((wrk = start_worker(EV_A)) == -1) {
			LOG("could not start new worker");
			dismiss_client(cfd, "Could not start worker");
			return;
		}
	}
	if (wrk == -1) {
		int cmin = INT_MAX;
		for (int w = 0 ; w < MAX_PROCS ; w++) {
			if (workers[w].pid != 0) {
				if (workers[w].clients < cmin) {
					cmin = workers[w].clients;
					wrk = w;
				}
			}
		}
	}
	if (wrk >= 0) {
		worker_t *worker = &workers[wrk];
		worker->clients += 1;
		sock_fd_write(worker->ctrl_sock, WCMD_ADD_CLIENT, strlen(WCMD_ADD_CLIENT), cfd);
	}
}

static void print_worker(worker_t *w) {
	fprintf(stderr, "-- WORKER %2lu --\nSOCK    : %5i\nPID     : %5i\nCLIENTS : %5i\n", ((long)w - (long)&workers) / sizeof(worker_t), w->ctrl_sock, w->pid, w->clients);
}

void stop_workers(EV_P) {
	for (int w = 0 ; w < MAX_PROCS ; w++) {
		stop_worker(w);
	}
	keep_running = false;
}

void print_workers(void) {
	LOG("finished holding\n");
	for (int w = 0 ; w < MAX_PROCS ; w++) {
		print_worker(&workers[w]);
	}
}

static void stop_worker(int w) {
	if (workers[w].pid != 0) {
		LOGF("stop worker %d\n", w);
		worker_t *worker = &workers[w];
		sock_fd_write(worker->ctrl_sock, WCMD_END, strlen(WCMD_END), -1);
	}
}

static void worker_child_cb(EV_P_ ev_child *childw, int revents) {
	worker_t *worker;
	ev_child_stop(EV_A_ childw);
	worker_counter -= 1;
	for (int w = 0 ; w < MAX_PROCS ; w++) {
		if (childw->rpid == workers[w].pid) {
			worker = &workers[w];
			close(worker->ctrl_sock);
			ev_io_stop(EV_A_ &worker->ctrl_w);
			worker->pid = 0;
			worker->ctrl_sock = 0;
		}
	}
	LOGF("process %d exited with status %x, %d left\n", childw->rpid, childw->rstatus, worker_counter);
}

static void dworker_ctrl_cb(EV_P_ ev_io *iow, int revents) {
	worker_t *worker = NULL;
	for (int w = 0 ; w < MAX_PROCS ; w++) {
		if (workers[w].ctrl_sock == iow->fd) {
			worker = &workers[w];
			break;
		}
	}
	if (revents & EV_READ) {
		char buff[500];
		ssize_t c = recv(worker->ctrl_sock, buff, 500, 0);
		buff[c] = '\0';
		if (strncmp(buff, WCMD_CLIENT_DISCONNECT, strlen(WCMD_CLIENT_DISCONNECT)) == 0) {
			worker->clients -= 1;
		}
	}
	print_workers();
}

static int start_worker(EV_P) {
	int w;
	worker_t *worker = NULL;
	for (w = 0 ; w < MAX_PROCS ; w++) {
		if (workers[w].pid == 0) {
			worker = &workers[w];
			break;
		}
	}
	if (worker != NULL) {
		pid_t cpid;
		int ctrlsocks[2];
		if (socketpair(AF_UNIX, SOCK_DGRAM, 0, ctrlsocks) == -1) {
			perror("start_worker, socketpair");
			exit(EXIT_FAILURE);
		}
		if ((cpid = fork()) == 0) {
			// ev_loop_fork(EV_A);
			close(ctrlsocks[0]);
			struct ev_loop *subloop = ev_loop_new(EVFLAG_AUTO | EVFLAG_NOENV | EVFLAG_FORKCHECK);
			run_worker(subloop, w, ctrlsocks[1]);
			exit(EXIT_SUCCESS);
		} else {
			LOGF("child %d started\n", cpid);
			close(ctrlsocks[1]);
			worker->pid = cpid;
			worker->ctrl_sock = ctrlsocks[0];
			worker->clients = 0;
			worker_counter += 1;
			ev_io_init(&worker->ctrl_w, dworker_ctrl_cb, worker->ctrl_sock, EV_READ);
			ev_child_init(&worker->child_w, worker_child_cb, worker->pid, 0);
			ev_child_start(EV_A_ &worker->child_w);
			ev_io_start(EV_A_ &worker->ctrl_w);
		}
	}
	return w;
}
