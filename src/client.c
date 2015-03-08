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
