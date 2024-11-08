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

// Hàm tạo thư mục
void create_user_directory(const char *username, const char *password) {
    char user_dir[BUFFER_SIZE];
    snprintf(user_dir, sizeof(user_dir), "%s/%s", BASE_DIR, username);
    
    if (mkdir(user_dir, 0700) == 0) {
        char password_file[BUFFER_SIZE];
        snprintf(password_file, sizeof(password_file), "%s/password.txt", user_dir);
        
        FILE *file = fopen(password_file, "w");
        if (file) {
            fprintf(file, "%s", password);
            fclose(file);
        }
    }
}

// Hàm xử lý client
void *client_handler(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        // Nhận dữ liệu từ client
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
                create_user_directory(username, password);
                send(client_sock, "Registration successful.\n", strlen("Registration successful.\n"), 0);
            }
        } else if (strcmp(command, "login") == 0) {
            char password_file[BUFFER_SIZE];
            snprintf(password_file, sizeof(password_file), "%s/%s/password.txt", BASE_DIR, username);
            
            if (access(password_file, F_OK) == -1) {
                send(client_sock, "Account does not exist.\n", strlen("Account does not exist.\n"), 0);
            } else {
                FILE *file = fopen(password_file, "r");
                char stored_password[BUFFER_SIZE];
                fscanf(file, "%s", stored_password);
                fclose(file);

                if (strcmp(stored_password, password) == 0) {
                    send(client_sock, "Login successful.\n", strlen("Login successful.\n"), 0);
                } else {
                    send(client_sock, "Incorrect password.\n", strlen("Incorrect password.\n"), 0);
                }
            }
        } else {
            send(client_sock, "Invalid command.\n", strlen("Invalid command.\n"), 0);
        }
    }

    close(client_sock);
    free(socket_desc);
    return NULL;
}

int main() {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // Tạo thư mục gốc để lưu thông tin người dùng
    mkdir(BASE_DIR, 0700);

    // Tạo socket server
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Gắn socket với địa chỉ và cổng
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
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