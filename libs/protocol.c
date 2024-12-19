#include <stdio.h>
#include <string.h>

void add_response_header(char *response, int response_header) {
    char temp[1024];
    snprintf(temp, sizeof(temp), "%d\n", response_header);
    strncat(temp, response, sizeof(temp) - strlen(temp) - 1);
    strncpy(response, temp, strlen(temp) + 1);
}
