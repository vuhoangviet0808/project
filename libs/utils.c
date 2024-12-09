#include "utils.h"
#include <stdarg.h>
#include <stdio.h>

void log_message(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Send a WebSocket message with the specified parameters
void send_websocket_message(int socket, const char *message, size_t message_length, int flag) {
    unsigned char frame[BUFFER_SIZE];

    // First byte: FIN bit set, opcode for text frame (1)
    frame[0] = 0x81; // FIN = 1, opcode = 1 (text frame)

    // Determine payload length and set the second byte
    if (message_length < 126) {
        frame[1] = (unsigned char)message_length; // Payload length
        memcpy(frame + 2, message, message_length); // Copy message after the header
        send(socket, frame, 2 + message_length, 0); // Send the frame
    } else if (message_length <= 65535) {
        frame[1] = 126; // Extended payload length
        frame[2] = (message_length >> 8) & 0xFF; // First byte of extended length
        frame[3] = message_length & 0xFF; // Second byte of extended length
        memcpy(frame + 4, message, message_length); // Copy message
        send(socket, frame, 4 + message_length, 0); // Send the frame
    } else {
        // Handle larger messages if necessary (not implemented here)
        fprintf(stderr, "Message too long to send.\n");
    }
}
