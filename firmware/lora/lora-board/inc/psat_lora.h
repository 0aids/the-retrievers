#ifndef psat_lora_h_INCLUDED
#define psat_lora_h_INCLUDED

#include "global_lora.h"
#include "gps_state.h"

void PsatRadioInit();

// Is not a loop, but is designed to be run in the control loop.
void PsatRadioMain(void);

// Pointer because we don't want a copy.

#endif  // psat_lora_h_INCLUDED
