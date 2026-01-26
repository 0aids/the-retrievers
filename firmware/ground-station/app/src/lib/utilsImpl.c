#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

void utils_sleepMs(uint16_t milliseconds)
{
    usleep(1000 * milliseconds);
}

void utils_log(const char* format, ...)
{
    va_list list;
    va_start(list, format);
    vprintf(format, list);
    va_end(list);
}