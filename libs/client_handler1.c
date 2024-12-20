#include "user.h"
#include "client_handler.h"
#include "utils.h"
#include "user_manager.h"
#include "user.h"
#include "message_handler.h"
#include "common.h"
#include <stdint.h>
#include "room_manager.h"

int decode_message(char *buffer, char *out_payload, size_t *out_len)
{

    uint8_t command = buffer[0] & 0xFF;
    const char *payload = buffer + 1;
    if (out_payload != NULL)
    {
        strncpy(out_payload, payload, *out_len - 1);
        out_payload[*out_len - 1] = '\0';
    }
    *out_len = strlen(payload);
    return command;
}

size_t encode_response(uint8_t status, const char *message, char *out_buffer) {
    out_buffer[0] = status;
    size_t message_length = strlen(message);
    strncpy(out_buffer + 1, message, BUFFER_SIZE - 1);
    out_buffer[message_length + 1] = '\0';
    return message_length + 1;
}


void init_clients()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].socket = -1;
        clients[i].is_online = 0;
    }
}

int add_client(int client_sock, int id, const char *username, const char *password)
{
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS)
    {
        clients[id].id = id;
        clients[id].is_online = 1;
        // strcpy(clients[id].password, password);
        // strcpy(clients[id].username, username);
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
        int opcode = decode_websocket_message(buffer, decoded_message, &decoded_length);
        if (opcode < 0)
        {
            send_websocket_message(client_sock, "Failed to decode message.\n", strlen("Failed to decode message.\n"), 0);
            continue;
        }
        switch (opcode)
        {
        case CMD_REGISTER:
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(decoded_message, "%s %s", username, password);

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
                    user_id = new_id;
                    strncpy(clients[user_id].username, username, BUFFER_SIZE);
                    strncpy(clients[user_id].password, password, BUFFER_SIZE);
                    clients[user_id].id = new_id;
                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;

                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "User ID: %d", new_id);
                    send_websocket_message(client_sock, response, strlen(response), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Registration failed.\n", strlen("Registration failed.\n"), 0);
                }
            }
            break;
        }

        case CMD_LOGIN:
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(decoded_message, "%s %s", username, password);

            char info_file[BUFFER_SIZE];
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
                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;

                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "User ID: %d", user_id);
                    send_websocket_message(client_sock, response, strlen(response), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Incorrect password.\n", strlen("Incorrect password.\n"), 0);
                }
            }
            break;
        }

        case CMD_CHAT:
        {
            char receiver_id_str[BUFFER_SIZE], message[BUFFER_SIZE];
            sscanf(decoded_message, "%s %[^\n]", receiver_id_str, message);

            int receiver_id = atoi(receiver_id_str);
            send_private_message(user_id, receiver_id, message);
            break;
        }

        case CMD_ADDFR:
        {
            char friend_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", friend_id_str);

            if (is_number(friend_id_str))
            {
                int friend_id = atoi(friend_id_str);
                pthread_mutex_lock(&clients_mutex);

                if (friend_id >= 0 && friend_id < MAX_CLIENTS)
                {
                    int result = send_friend_request(&clients[user_id], &clients[friend_id]);
                    pthread_mutex_unlock(&clients_mutex);

                    if (result == 1)
                    {
                        send_websocket_message(client_sock, "Friend request sent successfully.\n", strlen("Friend request sent successfully.\n"), 0);
                    }
                    else
                    {
                        send_websocket_message(client_sock, "Friend request failed.\n", strlen("Friend request failed.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send_websocket_message(client_sock, "Invalid user ID.\n", strlen("Invalid user ID.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
            break;
        } case CMD_ACCEPT:
        {
            char friend_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", friend_id_str);

            if (is_number(friend_id_str))
            {
                int friend_id = atoi(friend_id_str);
                pthread_mutex_lock(&clients_mutex);

                int result = accept_friend_request(&clients[user_id], &clients[friend_id]);
                pthread_mutex_unlock(&clients_mutex);

                if (result == 1)
                {
                    send_websocket_message(client_sock, "Friend request accepted successfully.\n", strlen("Friend request accepted successfully.\n"), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Failed to accept friend request.\n", strlen("Failed to accept friend request.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid friend ID format.\n", strlen("Invalid friend ID format.\n"), 0);
            }
            break;
        }

        case CMD_DECLINE:
        {
            char friend_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", friend_id_str);

            if (is_number(friend_id_str))
            {
                int friend_id = atoi(friend_id_str);
                pthread_mutex_lock(&clients_mutex);

                int result = decline_friend_request(&clients[user_id], friend_id);
                pthread_mutex_unlock(&clients_mutex);

                if (result == 1)
                {
                    send_websocket_message(client_sock, "Friend request declined successfully.\n", strlen("Friend request declined successfully.\n"), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Failed to decline friend request.\n", strlen("Failed to decline friend request.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid friend ID format.\n", strlen("Invalid friend ID format.\n"), 0);
            }
            break;
        }

        case CMD_LISTFR:
        {
            pthread_mutex_lock(&clients_mutex);
            char *friends_list = get_friends(clients[user_id]);
            pthread_mutex_unlock(&clients_mutex);

            send_websocket_message(client_sock, friends_list, strlen(friends_list), 0);
            break;
        }

        case CMD_CANCEL:
        {
            char friend_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", friend_id_str);

            if (is_number(friend_id_str))
            {
                int friend_id = atoi(friend_id_str);
                pthread_mutex_lock(&clients_mutex);

                int result = cancel_friend_request(&clients[user_id], &clients[friend_id]);
                pthread_mutex_unlock(&clients_mutex);

                if (result == 1)
                {
                    send_websocket_message(client_sock, "Friend request canceled successfully.\n", strlen("Friend request canceled successfully.\n"), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Failed to cancel friend request.\n", strlen("Failed to cancel friend request.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid friend ID format.\n", strlen("Invalid friend ID format.\n"), 0);
            }
            break;
        }

        case CMD_LISTREQ:
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
            break;
        }

        case CMD_REMOVE:
        {
            char friend_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", friend_id_str);

            if (is_number(friend_id_str))
            {
                int friend_id = atoi(friend_id_str);
                pthread_mutex_lock(&clients_mutex);

                int result = remove_friend(&clients[user_id], &clients[friend_id]);
                pthread_mutex_unlock(&clients_mutex);

                if (result == 1)
                {
                    send_websocket_message(client_sock, "Friend removed successfully.\n", strlen("Friend removed successfully.\n"), 0);
                }
                else
                {
                    send_websocket_message(client_sock, "Failed to remove friend.\n", strlen("Failed to remove friend.\n"), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Invalid friend ID format.\n", strlen("Invalid friend ID format.\n"), 0);
            }
            break;
        }

        case CMD_CREATE_ROOM:
        {
            char room_name[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(decoded_message, "%s %s", room_name, password);

            int room_id = create_room(room_name, user_id);
            if (room_id != -1)
            {
                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response), "Room created with ID: %d\n", room_id);
                send_websocket_message(client_sock, response, strlen(response), 0);
            }
            else
            {
                send_websocket_message(client_sock, "Failed to create room.\n", strlen("Failed to create room.\n"), 0);
            }
            break;
        }

        case CMD_JOIN_ROOM:
        {
            char room_id_str[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(decoded_message, "%s %s", room_id_str, password);

            int room_id = atoi(room_id_str);
            if (join_room(room_id, user_id))
            {
                send_websocket_message(client_sock, "Joined room successfully.\n", strlen("Joined room successfully.\n"), 0);
            }
            else
            {
                send_websocket_message(client_sock, "Failed to join room.\n", strlen("Failed to join room.\n"), 0);
            }
            break;
        }

        case CMD_ROOM_MESSAGE:
        {
            char room_id_str[BUFFER_SIZE], message[BUFFER_SIZE];
            sscanf(decoded_message, "%s %[^\n]", room_id_str, message);

            int room_id = atoi(room_id_str);
            if (room_message(room_id, user_id, message))
            {
                send_websocket_message(client_sock, "Message sent to room.\n", strlen("Message sent to room.\n"), 0);
            }
            else
            {
                send_websocket_message(client_sock, "Failed to send message to room.\n", strlen("Failed to send message to room.\n"), 0);
            }
            break;
        }

        case CMD_LEAVE_ROOM:
        {
            char room_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", room_id_str);

            int room_id = atoi(room_id_str);
            if (leave_room(room_id, user_id))
            {
                send_websocket_message(client_sock, "Left room successfully.\n", strlen("Left room successfully.\n"), 0);
            }
            else
            {
                send_websocket_message(client_sock, "Failed to leave room.\n", strlen("Failed to leave room.\n"), 0);
            }
            break;
        }

        case CMD_LIST_ROOMS:
        {
            char user_rooms[BUFFER_SIZE];
            get_user_rooms(user_id, user_rooms);

            if (strlen(user_rooms) > 0)
            {
                send_websocket_message(client_sock, user_rooms, strlen(user_rooms), 0);
            }
            else
            {
                send_websocket_message(client_sock, "No rooms joined.\n", strlen("No rooms joined.\n"), 0);
            }
            break;
        } case CMD_ADD_TO_ROOM:
        {
            char room_id_str[BUFFER_SIZE], target_user_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s %s", room_id_str, target_user_id_str);

            int room_id = atoi(room_id_str);
            int target_user_id = atoi(target_user_id_str);

            if (add_user_to_room(room_id, target_user_id))
            {
                send_websocket_message(client_sock, "User added to room successfully.\n", strlen("User added to room successfully.\n"), 0);

                // Thông báo đến người dùng được thêm
                if (clients[target_user_id].is_online)
                {
                    char notify_message[BUFFER_SIZE];
                    snprintf(notify_message, sizeof(notify_message),
                            "You have been added to room %d by %s.\n",
                            room_id, clients[user_id].username);
                    send(clients[target_user_id].socket, notify_message, strlen(notify_message), 0);
                }
            }
            else
            {
                send_websocket_message(client_sock, "Failed to add user to room.\n", strlen("Failed to add user to room.\n"), 0);
            }
            break;
        }

        case CMD_REMOVE_USER:
        {
            char room_id_str[BUFFER_SIZE], user_id_to_remove_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s %s", room_id_str, user_id_to_remove_str);

            int room_id = atoi(room_id_str);
            int user_id_to_remove = atoi(user_id_to_remove_str);

            if (remove_user_from_room(room_id, user_id, user_id_to_remove))
            {
                send_websocket_message(client_sock, "User removed from room successfully.\n", strlen("User removed from room successfully.\n"), 0);
            }
            else
            {
                send_websocket_message(client_sock, "Failed to remove user from room.\n", strlen("Failed to remove user from room.\n"), 0);
            }
            break;
        }

        case CMD_CHAT_OFFLINE:
        {
            char receiver_id_str[BUFFER_SIZE], message[BUFFER_SIZE];
            sscanf(decoded_message, "%s %[^\n]", receiver_id_str, message);

            int receiver_id = atoi(receiver_id_str);
            send_offline_message(user_id, receiver_id, message);
            send_websocket_message(client_sock, "Offline message sent.\n", strlen("Offline message sent.\n"), 0);
            break;
        }

        case CMD_RETRIEVE:
        {

            char receiver_id_str[BUFFER_SIZE];
            sscanf(decoded_message, "%s", receiver_id_str);

            int receiver_id = atoi(receiver_id_str);
            retrieve_message(user_id, receiver_id);
            send_websocket_message(client_sock, "Message retrieval complete.\n", strlen("Message retrieval complete.\n"), 0);
            break;
        }

        case CMD_LOGOUT:
        {
            logout(Client *user);
            send_websocket_message(client_sock, "Client logout.\n", strlen("Client logout.\n"), 0);
        }

        default:
            send_websocket_message(client_sock, "Invalid command.\n", strlen("Invalid command.\n"), 0);
            break;
        }
    }


    log_message("Client disconnected: ID %d", user_id);
    remove_client(user_id);
    close(client_sock);
    free(socket_desc);

    return NULL;
}
