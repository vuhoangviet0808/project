#include <stdio.h>
#include <string.h>
#include "common.h"

void add_response_header(char *final_response, int response_header, const char *response_content, size_t response_content_size) {
    snprintf(final_response, RESPONSE_SIZE, "%d\n%s", response_header, response_content);
}
