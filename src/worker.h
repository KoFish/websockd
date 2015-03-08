#ifndef __WORKER_H__
#define __WORKER_H__

#include <ev.h>
#include <sys/types.h>

struct worker_in {
	int ctrl_sock;
	pid_t pid;
	unsigned int clients;
	ev_io ctrl_w;
	ev_child child_w;
};

typedef struct worker_in worker_t;

void init_workers(EV_P);
void stop_workers(EV_P);

void print_workers(void);

void add_client_socket(EV_P_ int);

#endif  // __WORKER_H__
