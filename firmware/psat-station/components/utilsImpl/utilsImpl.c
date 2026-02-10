#include "stdio.h"
#include "utilsImpl.h"
#include <unistd.h>

void utils_sleepMs(uint16_t milliseconds)
{
    usleep(milliseconds * 1000);
}

void utils_log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
