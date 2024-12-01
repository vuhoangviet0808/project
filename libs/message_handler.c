#include "message_handler.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

#define CONVERSATION_ID_LENGTH 12

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
    // Try to read sender's conversation file first
    file = fopen(sender_file, "r");
    if (file) {
        if (fgets(buffer, CONVERSATION_ID_LENGTH + 2, file)) { // +2 for newline and null terminator
            buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
            fclose(file);
            return buffer;
        }
        fclose(file);
    }

    // If sender's file doesn't have an ID, check receiver's conversation file
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

    // Paths to conversations.txt files
    snprintf(sender_conversation_file, BUFFER_SIZE, "%s/conversations.txt", sender_folder);
    snprintf(receiver_conversation_file, BUFFER_SIZE, "%s/conversations.txt", receiver_folder);

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
    if (!message) {
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

    // Store the message
    store_message(sender_id, receiver_id, message);
}
