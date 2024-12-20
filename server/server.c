#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "../libs/common.h"
#include "../libs/utils.h"
#include "../libs/client_handler.h"
#include "../libs/user_manager.h"
#include "../libs/websocket_handshake.h"
#include "../libs/room_manager.h"

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int number_client;

int main() {
    number_client = load_user_name(clients, MAX_CLIENTS);
    for (int i = 0; i < number_client; i++) {
        printf("%d %s %s\n", clients[i].id, clients[i].username, clients[i].password);
    }

    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    mkdir(BASE_DIR, 0700);           // Tạo thư mục lưu dữ liệu người dùng
    mkdir("server/room_data", 0700); // Tạo thư mục lưu dữ liệu phòng chat
    init_clients();

    init_rooms(); // Khởi tạo danh sách phòng
    load_next_id();
    load_next_room_id(); // Tải ID phòng tiếp theo
    load_rooms();        // Tải danh sách phòng từ file
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        log_message("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_message("setsockopt(SO_REUSEADDR) failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        log_message("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        log_message("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    log_message("Server is running on port %d...", PORT);

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size))) {
        log_message("Client connected: %s", inet_ntoa(client_addr.sin_addr));

        // Perform WebSocket handshake
        if (handle_websocket_handshake(client_sock) < 0) {
            log_message("WebSocket handshake failed. Closing connection.");
            close(client_sock);
            continue;
        }

        // WebSocket handshake successful, start client handler thread
        pthread_t client_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&client_thread, NULL, client_handler, (void *)new_sock) < 0) {
            log_message("Could not create thread");
            close(client_sock);
            free(new_sock);
        }
    }
    // 123

    if (client_sock < 0) {
        log_message("Accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return 0;
}
