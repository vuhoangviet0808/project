#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H
#define CONVERSATION_ID_LENGTH 12


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