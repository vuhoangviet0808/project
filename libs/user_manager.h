#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "common.h"

int load_next_id();
void save_next_id(int);
int create_user_directory(const char *username, const char *password);
int authenticate_user(const char *username, const char *password);
int load_user_name(Client *clients, int max_clients);

#endif