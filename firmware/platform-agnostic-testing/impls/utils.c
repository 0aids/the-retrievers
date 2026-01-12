#include "utilsImpl.h"
#include <unistd.h>

void utils_sleepMs(uint16_t milliseconds) 
{
    usleep(milliseconds * 1000);
}

void utils_log()
{
    // noop
}
