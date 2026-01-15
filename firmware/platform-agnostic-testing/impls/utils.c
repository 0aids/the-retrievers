#include "utilsImpl.h"
#include <unistd.h>

void utils_sleepms(uint16_t ms) 
{
    usleep(ms * 1000);
}

void utils_log()
{
    // noop
}
