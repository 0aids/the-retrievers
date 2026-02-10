#include <stddef.h>
#include "states.h"
#include "on_entry_handlers.h"
#include "esp_log.h"
#include "on_exit_handlers.h"
#include "state_handlers.h"

static const char*     TAG = "PSAT_STATE";

static psatFsm_state_t stateTable[] = {
    [psatFSM_state_start] = {.state = psatFSM_state_start,
                             .defaultNextState =
                                 psatFSM_state_prelaunch, .onStateExit = psatFSM_startExitHandler,
                             .stateHandler =
                                 psatFSM_startStateHandler},
    [psatFSM_state_prelaunch] =
        {.state            = psatFSM_state_prelaunch,
                             .defaultNextState = psatFSM_state_ascent,
                             .onStateEntry     = psatFSM_prelaunchEntryHandler,
                             .onStateExit      = psatFSM_prelaunchExitHandler,
                             .stateHandler     = psatFSM_prelaunchStateHandler},
    [psatFSM_state_ascent] =
        {.state            = psatFSM_state_ascent,
                             .defaultNextState = psatFSM_state_deployPending,
                             .onStateEntry     = psatFSM_ascentEntryHandler,
                             .onStateExit      = psatFSM_ascentExitHandler,
                             .stateHandler     = psatFSM_ascentStateHandler},
    [psatFSM_state_deployPending] =
        {.state            = psatFSM_state_deployPending,
                             .defaultNextState = psatFSM_state_deployed,
                             .onStateEntry     = psatFSM_deployPendingEntryHandler,
                             .onStateExit      = psatFSM_deployPendingExitHandler,
                             .stateHandler     = psatFSM_deployPendingStateHandler},
    [psatFSM_state_deployed] =
        {.state            = psatFSM_state_deployed,
                             .defaultNextState = psatFSM_state_descent,
                             .onStateEntry     = psatFSM_deployedEntryHandler,
                             .onStateExit      = psatFSM_deployedExitHandler,
                             .stateHandler     = psatFSM_deployedStateHandler},
    [psatFSM_state_descent] =
        {.state            = psatFSM_state_descent,
                             .defaultNextState = psatFSM_state_landing,
                             .onStateEntry     = psatFSM_descentEntryHandler,
                             .onStateExit      = psatFSM_descentExitHandler,
                             .stateHandler     = psatFSM_descentStateHandler},
    [psatFSM_state_landing] =
        {.state            = psatFSM_state_landing,
                             .defaultNextState = psatFSM_state_recovery,
                             .onStateEntry     = psatFSM_landingEntryHandler,
                             .onStateExit      = psatFSM_landingExitHandler,
                             .stateHandler     = psatFSM_landingStateHandler},
    [psatFSM_state_recovery] =
        {.state            = psatFSM_state_recovery,
                             .defaultNextState = psatFSM_state_lowPower,
                             .onStateEntry     = psatFSM_recoveryEntryHandler,
                             .onStateExit      = psatFSM_recoveryExitHandler,
                             .stateHandler     = psatFSM_recoveryStateHandler},
    [psatFSM_state_lowPower] =
        {.state            = psatFSM_state_lowPower,
                             .defaultNextState = psatFSM_state_error,
                             .onStateEntry     = psatFSM_lowPowerEntryHandler,
                             .onStateExit      = psatFSM_lowPowerExitHandler,
                             .stateHandler     = psatFSM_lowPowerStateHandler},
    [psatFSM_state_error] = {.state            = psatFSM_state_error,
                             .defaultNextState = psatFSM_state_error,
                             .onStateEntry =
                                 psatFSM_errorEntryHandler, .onStateExit = psatFSM_errorExitHandler,
                             .stateHandler =
                                 psatFSM_errorStateHandler},
    [psatFSM_state_permanentError] =
        {.state            = psatFSM_state_error,
                             .defaultNextState = psatFSM_state_permanentError,
                             .onStateEntry     = psatFSM_permanentErrorEntryHandler,
                             .onStateExit      = psatFSM_permanentErrorExitHandler,
                             .stateHandler     = psatFSM_permanentErrorStateHandler},
};

psatFsm_state_t* psatFSM_getState(psatFSM_state_e state)
{
    if (state < 0 || state >= psatFSM_state__COUNT)
    {
        ESP_LOGW(TAG, "Invalid state id Provided (%i)", state);
        return NULL;
    }

    return &stateTable[state];
}