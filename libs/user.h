#ifndef USER_MANAGER_H
#define USER_MANAGER_H

//#include "stdbool.h"
#include "common.h"





void init_user(Client *user, const char *username, const char *password, int id);


int login(Client *user, const char *password);

void logout(Client *user);


int send_friend_request(Client *sender, Client *receiver);
int accept_friend_request(Client *sender, Client *receiver);


int decline_friend_request(Client *user, int sender_id);
int cancel_friend_request(Client *sender, Client *receiver);

void get_friends(Client *user, int friend_list[], int *friend_count);
void get_friend_requests(Client *user, int request_list[], int *request_count);

#endif // USER_MANAGER_H