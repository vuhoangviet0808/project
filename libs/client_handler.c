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

int add_client(int client_sock, int id, const char *username)
{
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS)
    {
        clients[id].socket = client_sock;
        clients[id].id = id;
        clients[id].is_online = 1;
        strncpy(clients[id].username, username, BUFFER_SIZE);
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
        sscanf(buffer, "%s %s %s", command, username, password); // Nhận lệnh từ client (login, register, chat):

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
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "Registration successful");
                    send(client_sock, response, strlen(response), 0);
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
                    add_client(client_sock, user_id, username); // Thêm client vào mảng
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "Login successful");
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