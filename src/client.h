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
