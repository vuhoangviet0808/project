#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int isLoggedIn = 0;

    // Tạo socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    // Login or register
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

    // Chat mode
    while (1) {
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

        memset(buffer, 0, BUFFER_SIZE);
        if (recv(sock, buffer, BUFFER_SIZE, 0) < 0) {
            perror("Receive failed");
            break;
        }

        printf("Server: %s\n", buffer);
    }

    close(sock);
    return 0;
}
