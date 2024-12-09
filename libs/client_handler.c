#include "user.h"
#include "client_handler.h"
#include "utils.h"
#include "user_manager.h"
#include "user.h"
#include "message_handler.h"
#include "common.h"
#include <stdint.h>

// Function to decode WebSocket frames
int decode_websocket_message(char *buffer, char *out, size_t *out_len) {
    uint8_t *p = (uint8_t *)buffer;
    size_t payload_len;
    size_t mask_offset = 0;
    size_t i;

    // First byte contains the FIN bit and opcode
    uint8_t first_byte = p[0];
    uint8_t opcode = first_byte & 0x0F; // Extract the opcode (lower 4 bits)

    // Second byte contains the masking bit and payload length
    uint8_t second_byte = p[1];
    int mask = (second_byte & 0x80) >> 7; // Extract masking bit
    payload_len = second_byte & 0x7F; // Extract payload length

    // Check for extended payload length (if >= 126)
    if (payload_len == 126) {
        payload_len = (p[2] << 8) | p[3]; // 2 bytes for extended payload length
        mask_offset = 4; // Move to the masking key
    } else if (payload_len == 127) {
        // For simplicity, we won't handle this case (8-byte length)
        return -1; // Unsupported frame size
    } else {
        mask_offset = 2; // Move to the masking key
    }

    // Get the masking key if it's a masked frame
    uint8_t masking_key[4] = {0};
    if (mask) {
        for (i = 0; i < 4; i++) {
            masking_key[i] = p[mask_offset + i];
        }
    }

    // Decode the payload
    *out_len = payload_len;
    for (i = 0; i < payload_len; i++) {
        out[i] = mask ? (p[mask_offset + 4 + i] ^ masking_key[i % 4]) : p[mask_offset + 4 + i];
    }
    out[payload_len] = '\0'; // Null-terminate the output

    return opcode; // Return the opcode of the message
}


void init_clients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].socket = -1;
        clients[i].is_online = 0;
    }
}

int add_client(int client_sock, int id, const char *username, const char * password)
{
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS)
    {
        clients[id].id = id;
        clients[id].is_online = 1;
        //strcpy(clients[id].password, password);
        //strcpy(clients[id].username, username);
        clients[id].socket = client_sock;
        strncpy(clients[id].username, username, BUFFER_SIZE);
        strncpy(clients[id].password, password, BUFFER_SIZE);
    }
    pthread_mutex_unlock(&clients_mutex);
    return id;
}

void remove_client(int id)
{
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS)
    {
        clients[id].socket = -1;
        clients[id].is_online = 0;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_handler(void *socket_desc)
{
    int client_sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    char decoded_message[BUFFER_SIZE];
    size_t decoded_length;
    int user_id = -1;
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int read_size = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (read_size <= 0)
            break;

        // decode
        int opcode = decode_websocket_message(buffer, decoded_message, &decoded_length);
        if (opcode < 0) {
            send_websocket_message(client_sock, "Failed to decode message.\n", strlen("Failed to decode message.\n"), 0);
            continue;
        }

        ///

        char command[BUFFER_SIZE], payload[BUFFER_SIZE];

        sscanf(decoded_message, "%s %[^\n]", command, payload);
        // sscanf(buffer, "%s %[^\n]", command, payload);

        printf("Debug: command %s, payload %s\n", command, payload);

        if (strcmp(command, "register") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            char user_dir[BUFFER_SIZE];
            snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);

            if (access(user_dir, F_OK) != -1)
            {
                send_websocket_message(client_sock, "Account already exists.\n", strlen("Account already exists.\n"), 0);
            }
            else
            {
                int new_id = create_user_directory(username, password);

                if (new_id != -1)
                {
                    //int id = add_client(client_sock, new_id, username, password);
                    //printf("%d\n", id);
                    user_id = new_id;
                    strncpy(clients[user_id].username, username, BUFFER_SIZE);
                    strncpy(clients[user_id].password, password, BUFFER_SIZE);
                    clients[user_id].id = new_id;
                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "Registration successful");
                    send_websocket_message(client_sock, response, strlen(response), 0);
                    printf("%d %d %d %s %s\n", user_id,clients[user_id].id, clients[user_id].is_online,clients[user_id].username, clients[user_id].password);
                }
                else
                {
                    send_websocket_message(client_sock, "Registration failed.\n", strlen("Registration failed.\n"), 0);
                }
            }
        }
        else if (strcmp(command, "login") == 0)
        {
            char info_file[BUFFER_SIZE];
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            snprintf(info_file, sizeof(info_file), "%s/%s/info.txt", BASE_DIR, username);

            if (access(info_file, F_OK) == -1)
            {
                send_websocket_message(client_sock, "Account does not exist.\n", strlen("Account does not exist.\n"), 0);
            }
            else
            {
                FILE *file = fopen(info_file, "r");
                int stored_id;
                char stored_password[BUFFER_SIZE];
                fscanf(file, "ID:%d\nPASSWORD:%s", &stored_id, stored_password);
                fclose(file);

                if (strcmp(stored_password, password) == 0)
                {
                    user_id = stored_id;
                    //add_client(client_sock, user_id, username, password); // Thêm client vào mảng

                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "Login successful");
                    send_websocket_message(client_sock, response, strlen(response), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Incorrect password.\n", strlen("Incorrect password.\n"), 0);
                }
            }
        }
        else if (strcmp(command, "chat") == 0)
        {
            char receiver_id[BUFFER_SIZE];
            char message[BUFFER_SIZE];
            int sender_id = user_id;
            sscanf(payload, "%s %[^\n]", receiver_id, message);
            send_private_message(sender_id, atoi(receiver_id), message);
        }
        else if (strcmp(command, "addfr") == 0)
        {
            char username[BUFFER_SIZE];
            sscanf(payload, "%s", username);
            if (is_number(username))
            {
                int receiver_id = atoi(username);
                printf("%d\n", receiver_id);
                pthread_mutex_lock(&clients_mutex);

                if (receiver_id >= 0 && receiver_id < MAX_CLIENTS)
                {
                    int result = send_friend_request(&clients[user_id], &clients[receiver_id]);
                    pthread_mutex_unlock(&clients_mutex);

                    if (result == 1)
                    {
                        send_websocket_message(client_sock, "Friend request sent successfully.\n", strlen("Friend request sent successfully.\n"), 0);
                    }
                    else
                    {
                        send_websocket_message(client_sock, "Friend request failed. Maybe already sent or full.\n", strlen("Friend request failed. Maybe already sent or full.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send_websocket_message(client_sock, "Invalid user ID or user is offline.\n", strlen("Invalid user ID or user is offline.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        } else if (strcmp(command, "accept") == 0)
        {
            char username[BUFFER_SIZE];
            sscanf(payload, "%s", username);
            if (is_number(username))
            {
                int sender_id = atoi(username);
                pthread_mutex_lock(&clients_mutex);

                if (sender_id >= 0 && sender_id < MAX_CLIENTS )
                {
                    int result = accept_friend_request(&clients[user_id], &clients[sender_id]);
                    pthread_mutex_unlock(&clients_mutex);

                    if (result == 1)
                    {
                        send_websocket_message(client_sock, "Friend request accepted successfully.\n", strlen("Friend request accepted successfully.\n"), 0);
                    }
                    else
                    {
                        send_websocket_message(client_sock, "Failed to accept friend request. Maybe list full.\n", strlen("Failed to accept friend request. Maybe list full.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send_websocket_message(client_sock, "Invalid user ID or user is offline.\n", strlen("Invalid user ID or user is offline.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        } else if (strcmp(command, "decline") == 0)
        {
            char username[BUFFER_SIZE];
            sscanf(payload, "%s", username);
            if (is_number(username))
            {
                int sender_id = atoi(username);
                pthread_mutex_lock(&clients_mutex);

                int result = decline_friend_request(&clients[user_id], sender_id);
                pthread_mutex_unlock(&clients_mutex);

                if (result == 1)
                {
                    send_websocket_message(client_sock, "Friend request declined successfully.\n", strlen("Friend request declined successfully.\n"), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Failed to decline friend request. Request not found.\n", strlen("Failed to decline friend request. Request not found.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "listfr") == 0)
        {
            pthread_mutex_lock(&clients_mutex);

            char *friends_list = get_friends(clients[user_id]);
            pthread_mutex_unlock(&clients_mutex);

            send_websocket_message(client_sock, friends_list, strlen(friends_list), 0);
        }
        else if (strcmp(command, "cancel") == 0)
        {
            char username[BUFFER_SIZE];
            sscanf(payload, "%s", username);
            if (is_number(username))
            {
                int receiver_id = atoi(username);
                pthread_mutex_lock(&clients_mutex);

                if (receiver_id >= 0 && receiver_id < MAX_CLIENTS && clients[receiver_id].is_online)
                {
                    int result = cancel_friend_request(&clients[user_id], &clients[receiver_id]);
                    pthread_mutex_unlock(&clients_mutex);

                    if (result == 1)
                    {
                        send_websocket_message(client_sock, "Friend request canceled successfully.\n", strlen("Friend request canceled successfully.\n"), 0);
                    }
                    else
                    {
                        send_websocket_message(client_sock, "Failed to cancel friend request. Request not found.\n", strlen("Failed to cancel friend request. Request not found.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send_websocket_message(client_sock, "Invalid user ID or user is offline.\n", strlen("Invalid user ID or user is offline.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        } else if (strcmp(command, "listreq") == 0)
        {
            pthread_mutex_lock(&clients_mutex);

            int request_list[MAX_REQUESTS];
            int request_count = 0;

            get_friend_requests(&clients[user_id], request_list, &request_count);

            pthread_mutex_unlock(&clients_mutex);

            if (request_count > 0)
            {
                char response[BUFFER_SIZE] = "Friend requests from: ";
                for (int i = 0; i < request_count; i++)
                {
                    char temp[BUFFER_SIZE];
                    snprintf(temp, sizeof(temp), "%d", request_list[i]);
                    strcat(response, temp);
                    if (i < request_count - 1)
                        strcat(response, ", ");
                }
                strcat(response, "\n");
                send_websocket_message(client_sock, response, strlen(response), 0);
            }
            else
            {
                send_websocket_message(client_sock, "No friend requests received.\n", strlen("No friend requests received.\n"), 0);
            }
        } else if(strcmp(command, "remove") == 0){
            char username[BUFFER_SIZE];
            sscanf(payload, "%s", username);
            if (is_number(username))
            {
                int sender_id = atoi(username);
                pthread_mutex_lock(&clients_mutex);

                int result = remove_friend(&clients[user_id], &clients[sender_id]);
                pthread_mutex_unlock(&clients_mutex);

                if (result == 1)
                {
                    send_websocket_message(client_sock, "Friend remove successfully.\n", strlen("Friend request declined successfully.\n"), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Failed to remove friend. Request not found.\n", strlen("Failed to decline friend request. Request not found.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "chat_offline") == 0)
        {
            char receiver_id[BUFFER_SIZE];
            char message[BUFFER_SIZE];
            int sender_id = user_id;
            sscanf(payload, "%s %[^\n]", receiver_id, message);
            send_offline_message(sender_id, atoi(receiver_id), message);
        }
        else if (strcmp(command, "retrieve") == 0)
        {
            char receiver_id[BUFFER_SIZE];
            int sender_id = user_id;
            sscanf(payload, "%s %[^\n]", receiver_id);
            retrieve_message(sender_id, atoi(receiver_id));
        }
        else
        {
            send_websocket_message(client_sock, "Invalid command.\n", strlen("Invalid command.\n"), 0);
        }
    }

    log_message("Client disconnected: ID %d", user_id);
    remove_client(user_id);
    close(client_sock);
    free(socket_desc);
    return NULL;
}


