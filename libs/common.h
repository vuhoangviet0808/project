#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include "protocol.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define RESPONSE_SIZE (BUFFER_SIZE * MAX_MESSAGES + 1)
#define MAX_MESSAGE_LENGTH 512
#define MAX_MESSAGES 1000
#define BASE_DIR "user_data"
#define ID_FILE "user_data/next_id.txt"
#define U_FILE "user_data/username.txt"
#define MAX_CLIENTS 1000
#define MAX_FRIENDS 100
#define MAX_REQUESTS 100
#define MAX_ROOMS 100

typedef struct
{
    int socket;
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    int id;
    int is_online;


    int friends[MAX_FRIENDS];
    int friend_count;

    int add_friend_requests[MAX_REQUESTS];
    int request_count;
} Client;

typedef struct
{
    int id;
    int creator_id; // ID của người tạo nhóm
    char name[BUFFER_SIZE];
    int members[MAX_CLIENTS];
    int member_count;

} ChatRoom;

extern ChatRoom rooms[MAX_ROOMS];
extern pthread_mutex_t rooms_mutex;
extern Client clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

#endif