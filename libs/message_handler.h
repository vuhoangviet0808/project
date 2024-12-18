#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H
#define MAX_MESSAGE_LENGTH 512
#define CONVERSATION_ID_LENGTH 12
#define MAX_MESSAGES 1000
#define RESPONSE_SIZE (BUFFER_SIZE * MAX_MESSAGES)

#include "common.h"

typedef struct {
    char timestamp[20];
    char username[BUFFER_SIZE];
    char message[MAX_MESSAGE_LENGTH];
} Message;

typedef struct {
    Message messages[MAX_MESSAGES];
    int count;
} MessageList;

void send_private_message(int sender_id, int receiver_id, const char *message);
void send_offline_message(int sender_id, int receiver_id, const char *message);
void store_message(int sender_id, int receiver_id, const char *message);
void retrieve_message(int client_sock, int sender_id, int receiver_id);

#endif