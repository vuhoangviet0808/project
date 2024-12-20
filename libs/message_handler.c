#include "message_handler.h"
#include "common.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

extern Client clients[MAX_CLIENTS];

void generate_fixed_id(char *buffer) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < CONVERSATION_ID_LENGTH; ++i) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buffer[CONVERSATION_ID_LENGTH] = '\0';
}

void ensure_directory_exists(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir(path, 0755);
    }
}

char *get_or_create_shared_conversation_id(const char *sender_file, const char *receiver_file, char *buffer) {
    FILE *file;

    // Try to read sender's file first
    file = fopen(sender_file, "r");
    if (file) {
        if (fgets(buffer, CONVERSATION_ID_LENGTH + 2, file)) { // +2 for newline and null terminator
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
            fclose(file);
            return buffer;
        }
        fclose(file);
    }

    // If sender's file doesn't have an ID, check receiver's file
    file = fopen(receiver_file, "r");
    if (file) {
        if (fgets(buffer, CONVERSATION_ID_LENGTH + 2, file)) { // +2 for newline and null terminator
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
            fclose(file);

            // Write this ID to sender's file to ensure consistency
            file = fopen(sender_file, "w");
            if (file) {
                fprintf(file, "%s\n", buffer);
                fclose(file);
            }
            return buffer;
        }
        fclose(file);
    }

    // If no ID exists, generate a new one
    generate_fixed_id(buffer);
    file = fopen(sender_file, "w");
    if (file) {
        fprintf(file, "%s\n", buffer);
        fclose(file);
    }
    file = fopen(receiver_file, "w");
    if (file) {
        fprintf(file, "%s\n", buffer);
        fclose(file);
    }

    return buffer;
}

void store_message(int sender_id, int receiver_id, const char *message) {
    char sender_folder[BUFFER_SIZE], receiver_folder[BUFFER_SIZE];
    char sender_conversation_file[BUFFER_SIZE], receiver_conversation_file[BUFFER_SIZE];
    char conversation_folder[BUFFER_SIZE] = "conversation_data";
    char conversation_file[BUFFER_SIZE], conversation_id[CONVERSATION_ID_LENGTH + 1] = {0};

    // Ensure user_data/{username}/conversations directories exist
    snprintf(sender_folder, BUFFER_SIZE, "user_data/%s/conversations", clients[sender_id].username);
    snprintf(receiver_folder, BUFFER_SIZE, "user_data/%s/conversations", clients[receiver_id].username);
    ensure_directory_exists(sender_folder);
    ensure_directory_exists(receiver_folder);

    // Paths to individual conversation files
    snprintf(sender_conversation_file, BUFFER_SIZE, "%s/%s.txt", sender_folder, clients[receiver_id].username);
    snprintf(receiver_conversation_file, BUFFER_SIZE, "%s/%s.txt", receiver_folder, clients[sender_id].username);

    // Get or create shared conversation ID
    get_or_create_shared_conversation_id(sender_conversation_file, receiver_conversation_file, conversation_id);

    // Ensure conversation_data folder exists
    ensure_directory_exists(conversation_folder);

    // Path to the conversation file
    snprintf(conversation_file, BUFFER_SIZE, "%s/%s.txt", conversation_folder, conversation_id);

    // Append the message to the conversation file
    FILE *file = fopen(conversation_file, "a");
    if (file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

        fprintf(file, "[%s] %s: %s\n", timestamp, clients[sender_id].username, message);
        fclose(file);
    }
}

void send_private_message(int sender_id, int receiver_id, const char *message) {
    printf("%d %d", sender_id, receiver_id);
    if (!message)
    {
        fprintf(stderr, "No message to send\n");
        const char *response = "No message to send";
        send_websocket_message(clients[sender_id].socket, response, strlen(response), 0);
        return;
    }

    if (sender_id < 0 || sender_id >= MAX_CLIENTS || receiver_id < 0 || receiver_id >= MAX_CLIENTS) {
        fprintf(stderr, "Invalid sender or receiver ID\n");
        const char *response = "Invalid sender or receiver ID";
        send_websocket_message(clients[sender_id].socket, response, strlen(response), 0);
        return;
    }

    char message_buffer[BUFFER_SIZE];
    char final_response[BUFFER_SIZE];
    snprintf(message_buffer, BUFFER_SIZE, "%s:%s", clients[sender_id].username, message);

    if (clients[receiver_id].is_online && clients[receiver_id].socket != -1) {
        add_response_header(final_response, RESPONSE_CHAT, message_buffer, strlen(message_buffer));
        send_websocket_message(clients[receiver_id].socket, final_response, strlen(message_buffer), 0);
        log_message("User %s sent private message to %s", clients[sender_id].username, clients[receiver_id].username);
        // Store the message
        store_message(sender_id, receiver_id, message);
        log_message("Stored private message from %s to %s", clients[sender_id].username, clients[receiver_id].username);
    } else {
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "User %s is not online.", clients[receiver_id].username);
        printf("User %s is not online.\n", clients[receiver_id].username);
        send_websocket_message(clients[sender_id].socket, response, strlen(response), 0);
    }
}

void send_offline_message(int sender_id, int receiver_id, const char *message) {
    if (!message) {
        fprintf(stderr, "No message to send\n");
        const char *response = "No message to send";
        send_websocket_message(clients[sender_id].socket, response, strlen(response), 0);
        return;
    }

    if (sender_id < 0 || sender_id >= MAX_CLIENTS || receiver_id < 0 || receiver_id >= MAX_CLIENTS) {
        fprintf(stderr, "Invalid sender or receiver ID\n");
        const char *response = "Invalid sender or receiver ID";
        send_websocket_message(clients[sender_id].socket, response, strlen(response), 0);
        return;
    }


    char message_buffer[BUFFER_SIZE];
    snprintf(message_buffer, BUFFER_SIZE, "%s: %s", clients[sender_id].username, message);
    log_message("User %s sent offline message to %s", clients[sender_id].username, clients[receiver_id].username);
    // Store the message
    store_message(sender_id, receiver_id, message);
    log_message("Stored offline message from %s to %s", clients[sender_id].username, clients[receiver_id].username);
}

void retrieve_message(int client_sock, int sender_id, int receiver_id) {
    if (sender_id < 0 || sender_id >= MAX_CLIENTS || receiver_id < 0 || receiver_id >= MAX_CLIENTS) {
        fprintf(stderr, "Invalid sender or receiver ID\n");
        const char *response = "Invalid sender or receiver ID";
        send_websocket_message(clients[sender_id].socket, response, strlen(response), 0);
        return;
    }

    char sender_conversation_file[BUFFER_SIZE];
    char conversation_id[CONVERSATION_ID_LENGTH + 1] = {0};
    MessageList *message_list = malloc(sizeof(MessageList));
    message_list->count = 0;

    char line[BUFFER_SIZE];
    char *response_content = malloc(RESPONSE_SIZE);
    size_t response_size = 0;

    snprintf(sender_conversation_file, BUFFER_SIZE, "user_data/%s/conversations/%s.txt",
             clients[sender_id].username, clients[receiver_id].username);

    FILE *file = fopen(sender_conversation_file, "r");
    if (file == NULL) {
        free(response_content);
        free(message_list);
        return;
    }

    if (fgets(conversation_id, CONVERSATION_ID_LENGTH + 2, file)) {
        conversation_id[strcspn(conversation_id, "\n")] = '\0';
    }
    fclose(file);

    char conversation_file[BUFFER_SIZE];
    snprintf(conversation_file, BUFFER_SIZE, "conversation_data/%s.txt", conversation_id);

    file = fopen(conversation_file, "r");
    if (file == NULL) {
        free(response_content);
        free(message_list);
        return;
    }

    while (fgets(line, sizeof(line), file) && message_list->count < MAX_MESSAGES) {
        Message *msg = &message_list->messages[message_list->count++];
        if (sscanf(line, "[%19[^]]] %[^:]: %[^\n]", msg->timestamp, msg->username, msg->message) == 3) {
            // Valid line parsed
        } else {
            --message_list->count; // Revert count if line is invalid
        }
    }
    fclose(file);

    for (int i = 0; i < message_list->count; i++) {
        Message *msg = &message_list->messages[i];
        int written = snprintf(response_content + response_size, RESPONSE_SIZE - response_size,
                               "[%s] %s: %s\n", msg->timestamp, msg->username, msg->message);
        if (written < 0 || written >= RESPONSE_SIZE - response_size) {
            break;
        }
        response_size += written;
    }

    char *final_response = malloc(RESPONSE_SIZE);
    if (!final_response) {
        free(response_content);
        free(message_list);
        perror("Failed to allocate memory for final response");
        return;
    }

    add_response_header(final_response, RESPONSE_RETRIEVE, response_content, response_size);

    send_websocket_message(client_sock, final_response, strlen(final_response), 0);

    printf("Final Response: %s\n", final_response);

    free(response_content);
    free(final_response);
    free(message_list);
}
