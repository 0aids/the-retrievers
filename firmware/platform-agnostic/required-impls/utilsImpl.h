#ifndef shared_utils_h_INCLUDED
#define shared_utils_h_INCLUDED
// Anything that is required by a shared logic
// This is mainly sleeping functions and logging functions

// Defined in the utils.c of your implementation.
#include <stdint.h>
extern void utils_sleepms(uint16_t ms);

// Unimplemented for now.
extern void utils_log();

#endif // shared_utils_h_INCLUDED
