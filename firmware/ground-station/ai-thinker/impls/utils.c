#include <stdint.h>
#include <utilsImpl.h>
#include <delay.h>
#include <stdio.h>

void utils_sleepMs(uint16_t ms) 
{
    DelayMs(ms);
}

void utils_log(const char* format, ...)
{
    va_list args;
    va_start(args ,format);
    vprintf(format, args);
    va_end(args);
}

