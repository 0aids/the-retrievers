#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define defaultBufferSize_d 256
static char buffer[defaultBufferSize_d] = {0};

void printw(const char* fmt, ...)
{
    strncpy(buffer + 1, fmt, defaultBufferSize_d);
    buffer[0] = 0x11;
    va_list args;
    va_start(args, fmt); 
    vprintf(buffer, args);
    va_end(args);
}
