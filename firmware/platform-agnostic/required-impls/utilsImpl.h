#ifndef shared_utils_h_INCLUDED
#define shared_utils_h_INCLUDED
// Anything that is required by a shared logic
// This is mainly sleeping functions and logging functions

// Defined in the utils.c of your implementation.
#include <stdint.h>
#include <stdarg.h>
extern void utils_sleepMs(uint16_t milliseconds);

extern void utils_log(const char* format, ...);

#endif // shared_utils_h_INCLUDED
