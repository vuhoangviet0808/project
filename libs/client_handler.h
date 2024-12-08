#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "common.h"


void *client_handler(void *socket_desc);
void init_clients();
void remove_client(int id);
int add_client(int client_sock, int id, const char *username, const char * password);


#endif