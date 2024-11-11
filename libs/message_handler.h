#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "common.h"

void send_private_message(const int *sender_id, const int *receiver_id, const char *message);

#endif