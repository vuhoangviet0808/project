#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BASE_DIR "user_data"
#define ID_FILE "user_data/next_id.txt"
#define MAX_CLIENTS 1000

typedef struct {
    int socket;
    int id;
    int is_online;
    char username[BUFFER_SIZE];
} Client;

Client clients[MAX_CLIENTS];  
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int next_id = 0;  

void init_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;  
        clients[i].is_online = 0;
    }
}

void load_next_id() {
    FILE *file = fopen(ID_FILE, "r");
    if (file) {
        fscanf(file, "%d", &next_id);
        fclose(file);
    } else {
        next_id = 0;  
    }
}

void save_next_id() {
    FILE *file = fopen(ID_FILE, "w");
    if (file) {
        fprintf(file, "%d", next_id);
        fclose(file);
    }
}

int create_user_directory(const char *username, const char *password) {
    char user_dir[BUFFER_SIZE];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);

    if (mkdir(user_dir, 0700) == 0) {
        int user_id = next_id++;
        save_next_id();  

        char info_file[BUFFER_SIZE];
        snprintf(info_file, sizeof(info_file), "%s/info.txt", user_dir);

        FILE *file = fopen(info_file, "w");
        if (file) {
            fprintf(file, "ID:%d\nPASSWORD:%s", user_id, password);
            fclose(file);
        }
        return user_id;
    }
    return -1;
}

int add_client(int client_sock, int id, const char *username) {
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS) {
        clients[id].socket = client_sock;
        clients[id].id = id;
        clients[id].is_online = 1;
        strncpy(clients[id].username, username, BUFFER_SIZE);
    }
    pthread_mutex_unlock(&clients_mutex);
    return id;
}

void remove_client(int id) {
    pthread_mutex_lock(&clients_mutex);
    if (id >= 0 && id < MAX_CLIENTS) {
        clients[id].socket = -1;
        clients[id].is_online = 0;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_handler(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int user_id = -1;  

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        int read_size = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (read_size <= 0) break;

        char command[BUFFER_SIZE], username[BUFFER_SIZE], password[BUFFER_SIZE];
        sscanf(buffer, "%s %s %s", command, username, password);
        
        if (strcmp(command, "register") == 0) {
            char user_dir[BUFFER_SIZE];
            snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);
            
            if (access(user_dir, F_OK) != -1) {
                send(client_sock, "Account already exists.\n", strlen("Account already exists.\n"), 0);
            } else {
                int new_id = create_user_directory(username, password);
                if (new_id != -1) {
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "Registration successful");
                    send(client_sock, response, strlen(response), 0);
                } else {
                    send(client_sock, "Registration failed.\n", strlen("Registration failed.\n"), 0);
                }
            }
        } else if (strcmp(command, "login") == 0) {
            char info_file[BUFFER_SIZE];
            snprintf(info_file, sizeof(info_file), "%s/%s/info.txt", BASE_DIR, username);
            
            if (access(info_file, F_OK) == -1) {
                send(client_sock, "Account does not exist.\n", strlen("Account does not exist.\n"), 0);
            } else {
                FILE *file = fopen(info_file, "r");
                int stored_id;
                char stored_password[BUFFER_SIZE];
                fscanf(file, "ID:%d\nPASSWORD:%s", &stored_id, stored_password);
                fclose(file);

                if (strcmp(stored_password, password) == 0) {
                    user_id = stored_id;
                    add_client(client_sock, user_id, username);  // Thêm client vào mảng
                    char response[BUFFER_SIZE];
                    snprintf(response, BUFFER_SIZE, "Login successful");
                    send(client_sock, response, strlen(response), 0);
                } else {
                    send(client_sock, "Incorrect password.\n", strlen("Incorrect password.\n"), 0);
                }
            }
        } else {
            send(client_sock, "Invalid command.\n", strlen("Invalid command.\n"), 0);
        }
    }

    printf("Client disconnected: ID %d\n", user_id);
    remove_client(user_id);  // Xóa client khỏi mảng khi ngắt kết nối
    close(client_sock);
    free(socket_desc);
    return NULL;
}

int main() {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    mkdir(BASE_DIR, 0700);
    load_next_id();
    init_clients();

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size))) {
        printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

        pthread_t client_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&client_thread, NULL, client_handler, (void *)new_sock) < 0) {
            perror("Could not create thread");
            close(client_sock);
            free(new_sock);
        }
    }

    if (client_sock < 0) {
        perror("Accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return 0;
}
