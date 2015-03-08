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


#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <ev.h>

struct lbuff_in {
	char *buff;
	ssize_t bufflen;
	struct lbuff_in *next;
};

struct client_in {
	int sock;
	ev_io read_w;
	ev_io write_w;
	struct lbuff_in *write_buffer;
	struct client_in *next;
};

typedef struct lbuff_in lbuff_t;
typedef struct client_in client_t;

void new_client(client_t**);
void add_client(client_t**, client_t*);
void free_client(client_t**);
void add_lbuff(lbuff_t**, char*, ssize_t);
void free_lbuff(lbuff_t**);
void add_client_write(client_t*, char*, ssize_t);
void remove_written(lbuff_t*, ssize_t);

#endif
