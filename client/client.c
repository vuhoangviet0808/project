#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>


#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int isLoggedIn = 0;
    fd_set read_fds;
    int max_sd;

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    while (!isLoggedIn) {
        printf("Enter command (register/login): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Xóa ký tự xuống dòng

        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            perror("Send failed");
            close(sock);
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) < 0) {
            perror("Receive failed");
            close(sock);
            exit(EXIT_FAILURE);
        }

        printf("Server: %s\n", buffer);

        // Check for successful login or registration response
        if (strcmp(buffer, "Login successful") == 0 || strcmp(buffer, "Registration successful") == 0) {
            isLoggedIn = 1;
        } else {
            printf("Login or registration failed. Try again.\n");
        }
    }

    printf("Entering chat room...\n");

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock, &read_fds);
        max_sd = sock;

        int activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        // Check for input from stdin
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            printf("You: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline character

            // Option to exit chat
            if (strcmp(buffer, "exit") == 0) {
                printf("Exiting chat room...\n");
                break;
            }

            if (send(sock, buffer, strlen(buffer), 0) < 0) {
                perror("Send failed");
                break;
            }
        }

        

        // Check for message from server
        if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(sock, buffer, BUFFER_SIZE, 0) < 0) {
                perror("Receive failed");
                break;
            }

            printf("Server: %s\n", buffer);
        }
    }

    close(sock);
    return 0;
}
