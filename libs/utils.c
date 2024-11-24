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