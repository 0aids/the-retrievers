#include "shared_state.h"

psatGlobal_state_t psat_globalState = {.currentFSMState = psatFSM_state_start};

const char* printErrorType(psatErrStates_e err)
{
    switch (err)
    {
#define X(errType)                                                   \
    case errType: return #errType; break;
        allError_xmacro
#undef X
    }
}