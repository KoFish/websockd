#ifndef __WORKER_FORK_H__
#define __WORKER_FORK_H__

#include <stdlib.h>
#include <unistd.h>
#include <ev.h>

static const char WCMD_CLIENT_DISCONNECT[] = "CLIENT DISCONNECTED";
static const char WCMD_ADD_CLIENT[] = "ADD CLIENT";
static const char WCMD_END[] = "END";

void run_worker(EV_P_ int, int);

#endif
