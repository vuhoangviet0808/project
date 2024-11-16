#include "message_handler.h"
#include <stdio.h>
#include <string.h>

extern Client clients[MAX_CLIENTS];

void send_private_message(int sender_id, int receiver_id, const char *message) {
    if (message == NULL) {
        fprintf(stderr, "Null pointer error in send_private_message\n");
        return;
    }

    if (sender_id < 0 || sender_id >= MAX_CLIENTS || receiver_id < 0 || receiver_id >= MAX_CLIENTS) {
        fprintf(stderr, "Invalid sender or receiver ID in send_private_message\n");
        return;
    }

    char message_buffer[BUFFER_SIZE];
    snprintf(message_buffer, BUFFER_SIZE, "%s: %s", clients[sender_id].username, message);

    if (clients[receiver_id].is_online && clients[receiver_id].socket != -1) {
        send(clients[receiver_id].socket, message_buffer, strlen(message_buffer), 0);
    } else {
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "User %s is not online.", clients[receiver_id].username);
        send(clients[sender_id].socket, response, strlen(response), 0);
    }
}
