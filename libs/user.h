#ifndef USER_MANAGER_H
#define USER_MANAGER_H

//#include "stdbool.h"

#define MAX_FRIENDS 100
#define MAX_REQUESTS 100


typedef struct {
    char username[50];  
    char password[50];  
    int id;             
    int is_online;    


    int friends[MAX_FRIENDS];  
    int friend_count;              

    int add_friend_requests[MAX_REQUESTS];
    int request_count;              
} User;


void init_user(User *user, const char *username, const char *password, int id);


bool login(User *user, const char *password);

void logout(User *user);


int send_friend_request(User *sender, User *receiver);
int accept_friend_request(User *sender, User *receiver);


int decline_friend_request(User *user, int sender_id);
int cancel_friend_request(User *sender, User *receiver);

void get_friends(User *user, int friend_list[], int *friend_count);
void get_friend_requests(User *user, int request_list[], int *request_count);

#endif // USER_MANAGER_H