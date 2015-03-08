#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "client.h"

void new_client(client_t **c) {
	client_t *client = malloc(sizeof(client_t));
	client->sock = -1;
	client->write_buffer = NULL;
	client->next = NULL;
	(*c) = client;
}

void add_client(client_t **list, client_t *client) {
	while ((*list) != NULL) list = &((*list)->next);
	client->next = NULL;
	(*list) = client;
}

void free_client(client_t **c) {
	client_t *client = (*c);
	client_t *next = client->next;
	close(client->sock);
	free_lbuff(&(client->write_buffer));
	free(client);
	(*c) = next;
}

void add_lbuff(lbuff_t **b, char *cbuff, ssize_t bufflen) {
	if ((*b) == NULL) {
		char *buff = malloc(sizeof(char) * bufflen);
		strncpy(buff, cbuff, bufflen);
		(*b) = malloc(sizeof(lbuff_t));
		(*b)->buff = buff;
		(*b)->bufflen = bufflen;
	} else {
		add_lbuff(&((*b)->next), cbuff, bufflen);
	}
}

void free_lbuff(lbuff_t **b) {
	if ((*b) == NULL) { return; }
	free_lbuff(&((*b)->next));
	if ((*b)->buff != NULL) { free((*b)->buff); }
	free((*b));
	(*b) = NULL;
}

void add_client_write(client_t *client, char *buff, ssize_t bufflen) {
	add_lbuff(&(client->write_buffer), buff, bufflen);
}

void remove_written(lbuff_t *b, ssize_t count) {
	ssize_t newbufflen = b->bufflen - count;
	char *newbuff = malloc(sizeof(char) * newbufflen);
	strncpy(newbuff, b->buff + count, newbufflen);
	free(b->buff);
	b->buff = newbuff;
	b->bufflen = newbufflen;
}
