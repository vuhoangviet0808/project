#ifndef USER_H
#define USER_H

//#include "stdbool.h"
#include "common.h"




int is_number(const char *str);
void init_user(Client *user, const char *username, const char *password, int id);


int login(Client *user, const char *password);

void logout(Client *user);


int send_friend_request(Client *sender, Client *receiver);
int accept_friend_request(Client *sender, Client *receiver);


int decline_friend_request(Client *user, int sender_id);
int cancel_friend_request(Client *sender, Client *receiver);

char* get_friends(Client user);
char* get_friend_requests(Client user);

int remove_friend(Client* sender, Client* receiver);
void write_friend_list(Client *client);
void write_friend_request(Client *receiver);



#endif // USER_MANAGER_H