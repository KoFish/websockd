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
