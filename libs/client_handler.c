#include "user.h"
#include "client_handler.h"
#include "utils.h"
#include "user_manager.h"
#include "message_handler.h"
#include "common.h"
#include "room_manager.h"
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
    int user_id = -1;
    while (1)
    {

        memset(buffer, 0, BUFFER_SIZE);

        int read_size = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (read_size <= 0)
            break;

        char command[BUFFER_SIZE], payload[BUFFER_SIZE];
        sscanf(buffer, "%s %[^\n]", command, payload);

        printf("Debug: command %s, payload %s\n", command, payload);

        if (strcmp(command, "register") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            char user_dir[BUFFER_SIZE];
            snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);

            if (access(user_dir, F_OK) != -1)
            {
                send(client_sock, "Account already exists.\n", strlen("Account already exists.\n"), 0);
            }
            else
            {
                int new_id = create_user_directory(username, password);

                if (new_id != -1)
                {
                    // int id = add_client(client_sock, new_id, username, password);
                    // printf("%d\n", id);
                    user_id = new_id;
                    strncpy(clients[user_id].username, username, BUFFER_SIZE);
                    strncpy(clients[user_id].password, password, BUFFER_SIZE);
                    clients[user_id].id = new_id;
                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "%d", new_id);
                    send(client_sock, response, strlen(response), 0);
                    printf("%d %d %d %s %s\n", user_id, clients[user_id].id, clients[user_id].is_online, clients[user_id].username, clients[user_id].password);
                }
                else
                {
                    send(client_sock, "Registration failed.\n", strlen("Registration failed.\n"), 0);
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
                send(client_sock, "Account does not exist.\n", strlen("Account does not exist.\n"), 0);
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
                    // add_client(client_sock, user_id, username, password); // Thêm client vào mảng

                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "%d", user_id);
                    send(client_sock, response, strlen(response), 0);
                }
                else
                {
                    send(client_sock, "Incorrect password.\n", strlen("Incorrect password.\n"), 0);
                }
            }
        }
        else if (strcmp(command, "chat") == 0)
        {
            char recver[BUFFER_SIZE];
            char message[BUFFER_SIZE];
            int sender_id = user_id, receiver_id = -1;

            sscanf(payload, "%s %[^\n]", recver, message);

            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].is_online && strcmp(clients[i].username, recver) == 0)
                {
                    receiver_id = i;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            if (receiver_id != -1)
            {
                send_private_message(sender_id, receiver_id, message);
                log_message("User %s sent message to %s", clients[sender_id].username, recver);
            }
            else
            {
                char error_response[BUFFER_SIZE];
                snprintf(error_response, BUFFER_SIZE, "User %s is not online or does not exist.", recver);
                send(client_sock, error_response, strlen(error_response), 0);
            }
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
                        send(client_sock, "Friend request sent successfully.\n", strlen("Friend request sent successfully.\n"), 0);
                    }
                    else
                    {
                        send(client_sock, "Friend request failed. Maybe already sent or full.\n", strlen("Friend request failed. Maybe already sent or full.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send(client_sock, "Invalid user ID or user is offline.\n", strlen("Invalid user ID or user is offline.\n"), 0);
                }
            }
            else
            {
                send(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "accept") == 0)
        {
            char username[BUFFER_SIZE];
            sscanf(payload, "%s", username);
            if (is_number(username))
            {
                int sender_id = atoi(username);
                pthread_mutex_lock(&clients_mutex);

                if (sender_id >= 0 && sender_id < MAX_CLIENTS)
                {
                    int result = accept_friend_request(&clients[user_id], &clients[sender_id]);
                    pthread_mutex_unlock(&clients_mutex);

                    if (result == 1)
                    {
                        send(client_sock, "Friend request accepted successfully.\n", strlen("Friend request accepted successfully.\n"), 0);
                    }
                    else
                    {
                        send(client_sock, "Failed to accept friend request. Maybe list full.\n", strlen("Failed to accept friend request. Maybe list full.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send(client_sock, "Invalid user ID or user is offline.\n", strlen("Invalid user ID or user is offline.\n"), 0);
                }
            }
            else
            {
                send(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "decline") == 0)
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
                    send(client_sock, "Friend request declined successfully.\n", strlen("Friend request declined successfully.\n"), 0);
                }
                else
                {
                    send(client_sock, "Failed to decline friend request. Request not found.\n", strlen("Failed to decline friend request. Request not found.\n"), 0);
                }
            }
            else
            {
                send(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "listfr") == 0)
        {
            pthread_mutex_lock(&clients_mutex);

            char *friends_list = get_friends(clients[user_id]);
            pthread_mutex_unlock(&clients_mutex);

            send(client_sock, friends_list, strlen(friends_list), 0);
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
                        send(client_sock, "Friend request canceled successfully.\n", strlen("Friend request canceled successfully.\n"), 0);
                    }
                    else
                    {
                        send(client_sock, "Failed to cancel friend request. Request not found.\n", strlen("Failed to cancel friend request. Request not found.\n"), 0);
                    }
                }
                else
                {
                    pthread_mutex_unlock(&clients_mutex);
                    send(client_sock, "Invalid user ID or user is offline.\n", strlen("Invalid user ID or user is offline.\n"), 0);
                }
            }
            else
            {
                send(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "listreq") == 0)
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
                send(client_sock, response, strlen(response), 0);
            }
            else
            {
                send(client_sock, "No friend requests received.\n", strlen("No friend requests received.\n"), 0);
            }
        }
        else if (strcmp(command, "remove") == 0)
        {
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
                    send(client_sock, "Friend remove successfully.\n", strlen("Friend request declined successfully.\n"), 0);
                }
                else
                {
                    send(client_sock, "Failed to remove friend. Request not found.\n", strlen("Failed to decline friend request. Request not found.\n"), 0);
                }
            }
            else
            {
                send(client_sock, "Invalid user ID format.\n", strlen("Invalid user ID format.\n"), 0);
            }
        }
        else if (strcmp(command, "chat_offline") == 0)
        {
            char recver[BUFFER_SIZE];
            char message[BUFFER_SIZE];
            int sender_id = user_id, receiver_id = -1;

            sscanf(payload, "%s %[^\n]", recver, message);

            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (strcmp(clients[i].username, recver) == 0)
                {
                    receiver_id = i;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            if (receiver_id != -1)
            {
                send_offline_message(sender_id, receiver_id, message);
                log_message("User %s sent offline message to %s", clients[sender_id].username, recver);
            }
            else
            {
                char error_response[BUFFER_SIZE];
                snprintf(error_response, BUFFER_SIZE, "User %s does not exist.", recver);
                send(client_sock, error_response, strlen(error_response), 0);
            }
        }
        else if (strcmp(command, "retrieve") == 0)
        {
            char recver[BUFFER_SIZE];
            int sender_id = user_id, receiver_id = -1;

            sscanf(payload, "%s %[^\n]", recver);

            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (strcmp(clients[i].username, recver) == 0)
                {
                    receiver_id = i;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);

            if (receiver_id != -1)
            {
                retrieve_message(sender_id, receiver_id);
            }
            else
            {
                char error_response[BUFFER_SIZE];
                snprintf(error_response, BUFFER_SIZE, "User %s does not exist.", recver);
                send(client_sock, error_response, strlen(error_response), 0);
            }
        }
        else if (strcmp(command, "create_room") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            int room_id = create_room(username, user_id);
            if (room_id != -1)
            {
                char response[BUFFER_SIZE];
                snprintf(response, BUFFER_SIZE, "Room created with ID %d\n", room_id);
                send(client_sock, response, strlen(response), 0);
            }
            else
            {
                send(client_sock, "Failed to create room 123.\n", strlen("Failed to create room 123.\n"), 0);
            }
        }
        else if (strcmp(command, "join_room") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            int room_id = atoi(username); // Tìm room_id từ tên
            if (room_id == -1)
            {
                send(client_sock, "Room does not exist.\n", strlen("Room does not exist.\n"), 0);
            }
            else if (join_room(room_id, user_id))
            {
                send(client_sock, "Joined room successfully.\n", strlen("Joined room successfully.\n"), 0);
            }
            else
            {
                send(client_sock, "Failed to join room.\n", strlen("Failed to join room.\n"), 0);
            }
        }
        else if (strcmp(command, "room_message") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            int room_id;
            char message[BUFFER_SIZE];

            // Phân tích cú pháp để lấy room_id và phần còn lại là tin nhắn
            char *token = strtok(buffer + strlen(command) + 1, " ");
            if (token)
            {
                room_id = atoi(token);
                char *message_start = buffer + strlen(command) + strlen(token) + 2;
                strncpy(message, message_start, BUFFER_SIZE - 1);
                message[BUFFER_SIZE - 1] = '\0';

                if (room_message(room_id, user_id, message))
                {
                    send(client_sock, "Message sent to room.\n", strlen("Message sent to room.\n"), 0);
                }
                else
                {
                    send(client_sock, "Failed to send message to room.\n", strlen("Failed to send message to room.\n"), 0);
                }
            }
            else
            {
                send(client_sock, "Invalid room_message format.\n", strlen("Invalid room_message format.\n"), 0);
            }
        }
        else if (strcmp(command, "add_to_room") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            int room_id, target_user_id;
            sscanf(buffer + strlen(command) + 1, "%d %d", &room_id, &target_user_id);

            if (add_user_to_room(room_id, target_user_id))
            {
                send(client_sock, "User added to room successfully.\n", strlen("User added to room successfully.\n"), 0);

                // Gửi thông báo đến người dùng được thêm
                // if (clients[target_user_id].is_online)
                // {
                char notify_message[BUFFER_SIZE];
                snprintf(notify_message, sizeof(notify_message),
                         "You have been added to room %d by %s.\n",
                         room_id, clients[user_id].username);
                send(clients[target_user_id].socket, notify_message, strlen(notify_message), 0);
                // }
            }
            else
            {
                send(client_sock, "Failed to add user to room.\n", strlen("Failed to add user to room.\n"), 0);
            }
        }
        else if (strcmp(command, "leave_room") == 0)
        {
            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            int room_id = atoi(username); // Room ID được truyền trong trường username
            if (leave_room(room_id, user_id))
            {
                send(client_sock, "You have leave the room successfully.\n", strlen("You have leave the room successfully.\n"), 0);
            }
            else
            {
                send(client_sock, "Failed to leave the room.\n", strlen("Failed to leave the room.\n"), 0);
            }
        }
        else if (strcmp(command, "remove_user") == 0)
        {

            char username[BUFFER_SIZE], password[BUFFER_SIZE];
            sscanf(payload, "%s %s", username, password);
            int room_id = atoi(username);           // Room ID truyền ở phần username
            int user_id_to_remove = atoi(password); // User ID cần xóa truyền ở phần password

            if (remove_user_from_room(room_id, user_id, user_id_to_remove))
            {
                send(client_sock, "User removed from room successfully.\n", strlen("User removed from room successfully.\n"), 0);
            }
            else
            {
                send(client_sock, "Failed to remove user from room.\n", strlen("Failed to remove user from room.\n"), 0);
            }
        }

        else
        {
            send(client_sock, "Invalid command.\n", strlen("Invalid command.\n"), 0);
        }
    }

    log_message("Client disconnected: ID %d", user_id);
    remove_client(user_id);
    close(client_sock);
    free(socket_desc);
    return NULL;
}
