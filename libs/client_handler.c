#include "user.h"
#include "client_handler.h"
#include "utils.h"
#include "user_manager.h"
#include "message_handler.h"
#include "common.h"


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
    int user_id = -1;
    while (1)   
    {
        memset(buffer, 0, BUFFER_SIZE);

        int read_size = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (read_size <= 0)
            break;

        char command[BUFFER_SIZE], username[BUFFER_SIZE], password[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", command, username, password);

        if (strcmp(command, "register") == 0)
        {
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
                    //int id = add_client(client_sock, new_id, username, password);
                    //printf("%d\n", id);
                    user_id = new_id;
                    strncpy(clients[user_id].username, username, BUFFER_SIZE);
                    strncpy(clients[user_id].password, password, BUFFER_SIZE);
                    clients[user_id].id = new_id;
                    clients[user_id].is_online = 1;
                    clients[user_id].socket = client_sock;
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "%d", new_id);
                    send(client_sock, response, strlen(response), 0);
                    printf("%d %d %d %s %s\n", user_id,clients[user_id].id, clients[user_id].is_online,clients[user_id].username, clients[user_id].password);
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
                    //add_client(client_sock, user_id, username, password); // Thêm client vào mảng
                    
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

            sscanf(buffer + strlen(command) + 1, "%s %[^\n]", recver, message);

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
        } else if (strcmp(command, "accept") == 0)
        {
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
        } else if (strcmp(command, "decline") == 0)
        {
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
                send(client_sock, response, strlen(response), 0);
            }
            else
            {
                send(client_sock, "No friend requests received.\n", strlen("No friend requests received.\n"), 0);
            }
        } else if(strcmp(command, "remove") == 0){
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


