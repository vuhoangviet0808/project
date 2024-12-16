#ifndef UTILS_H
#define UTILS_H

#include "common.h"

void log_message(const char *format, ...);
void send_websocket_message(int socket, const char *message, size_t message_length, int flag);

#endif