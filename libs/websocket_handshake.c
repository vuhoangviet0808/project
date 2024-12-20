#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include "../libs/common.h"
#include "../libs/utils.h"
#include "websocket_handshake.h"

#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

// Base64 encoding function
char *base64_encode(const unsigned char *input, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    char *output = malloc(bufferPtr->length + 1);
    if (output) {
        memcpy(output, bufferPtr->data, bufferPtr->length);
        output[bufferPtr->length] = '\0'; // Null-terminate the string
    }
    return output;
}

// Generate the Sec-WebSocket-Accept key
void generate_accept_key(const char *client_key, char *accept_key) {
    char buffer[BUFFER_SIZE];
    unsigned char hash[SHA_DIGEST_LENGTH];

    snprintf(buffer, sizeof(buffer), "%s%s", client_key, GUID);
    SHA1((unsigned char *)buffer, strlen(buffer), hash);

    char *encoded = base64_encode(hash, SHA_DIGEST_LENGTH);
    strcpy(accept_key, encoded);
    free(encoded);
}

// Handle WebSocket handshake
int handle_websocket_handshake(int client_sock) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    char client_key[128], accept_key[128];

    // Read the client's handshake request
    if (read(client_sock, buffer, sizeof(buffer)) <= 0) {
        log_message("Failed to read WebSocket handshake request");
        return -1;
    }

    // Extract the Sec-WebSocket-Key
    char *key_start = strstr(buffer, "Sec-WebSocket-Key: ");
    if (!key_start) {
        log_message("Missing Sec-WebSocket-Key in WebSocket handshake");
        return -1;
    }
    sscanf(key_start, "Sec-WebSocket-Key: %s", client_key);

    // Generate Sec-WebSocket-Accept
    generate_accept_key(client_key, accept_key);

    // Build handshake response
    snprintf(response, sizeof(response),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n\r\n",
             accept_key);

    // Send handshake response
    if (send(client_sock, response, strlen(response), 0) <= 0) {
        log_message("Failed to send WebSocket handshake response");
        return -1;
    }

    log_message("WebSocket handshake completed successfully");
    return 0;
}
