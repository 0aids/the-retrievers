#pragma once

#include "shared_state.h"
#include "sm.h"

#define NUMBER_ERRORS_STORED 10

typedef struct
{
    int head;
    int count;
} circularBuffer_t;

void psatErr_bufferInsert(psatErr_error_t error);

int  psatErr_bufferCopyOrdered(
     psatErr_error_t errorOrderedBuffer[NUMBER_ERRORS_STORED]);

psatErr_error_t* psatErr_getMostRecentError();
psatErr_error_t* psatErr_getErrorById(int id);
void             psatErr_postError(psatErr_code_e      errorCode,
                                   psatFSM_component_e originComponent,
                                   psatFSM_state_e     originState);
bool psatErr_attemptRecovery(psatFSM_component_e componentId,
                             psatErr_error_t     error);