#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

void *client_handler(void *socket_desc);
void init_clients();
int is_number(const char *str);
void init_user(Client *user, const char *username, const char *password, int id);


int login(Client *user, const char *password);

void logout(Client *user);


int send_friend_request(Client *sender, Client *receiver);
int accept_friend_request(Client *sender, Client *receiver);


int decline_friend_request(Client *user, int sender_id);
int cancel_friend_request(Client *sender, Client *receiver);

char* get_friends(Client user);
void get_friend_requests(Client *user, int request_list[], int *request_count);

#endif