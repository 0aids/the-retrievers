// on error
// post err (id, origin,errcode)

// create error record
// post errr to fsm queue

// when fsm recieves, transition to error state
// get most recent error or by id
// increment retry count
// run recover or init/deinit
// transition back to original state

// retry = 1 wait 5s then retry
// retry = 2 wait 10s and run on exit wait 5s then on entry
// retry = 3 transiton into permanent error state

#include "errors.h"
#include "esp_timer.h"

psatErr_error_t  errorBuffer[NUMBER_ERRORS_STORED];

circularBuffer_t errorCircularBuffer = {
    .head  = 0,
    .count = 0,
};

void psatErr_bufferInsert(psatErr_error_t error)
{
    errorBuffer[errorCircularBuffer.head] = error;
    errorCircularBuffer.head =
        (errorCircularBuffer.head + 1) % NUMBER_ERRORS_STORED;

    if (errorCircularBuffer.count < NUMBER_ERRORS_STORED)
    {
        errorCircularBuffer.count++;
    }
}

void psatErr_bufferCopyOrdered(
    psatErr_error_t errorOrderedBuffer[NUMBER_ERRORS_STORED])
{
    int index =
        (errorCircularBuffer.head - 1 + NUMBER_ERRORS_STORED) %
        NUMBER_ERRORS_STORED;
    for (int i = 0; i < errorCircularBuffer.count; i++)
    {
        errorOrderedBuffer[i] = errorBuffer[index];
        index =
            (index - 1 + NUMBER_ERRORS_STORED) % NUMBER_ERRORS_STORED;
    }
}

psatErr_error_t* psatErr_getMostRecentError()
{
    int index =
        (errorCircularBuffer.head - 1 + NUMBER_ERRORS_STORED) %
        NUMBER_ERRORS_STORED;

    return &errorBuffer[index];
}

psatErr_error_t* psatErr_getErrorById(int id)
{
    for (int i = 0; i < NUMBER_ERRORS_STORED; i++)
    {
        if (errorBuffer[i].id == id)
            return &errorBuffer[i];
    }
    return NULL;
}

static int errorCounter = 0;

void       postError(psatErr_code_e      errorCode,
                     psatFSM_component_e originComponent,
                     psatFSM_state_e     originState)
{
    psatErr_error_t error = {
        .id              = errorCounter++,
        .code            = errorCode,
        .originComponent = originComponent,
        .originState     = originState,
        .timestamp       = esp_timer_get_time(),
    };

    psatErr_bufferInsert(error);

    psatFSM_event_t event = {.global = true,
                             .type   = psatFSM_eventType_error,
                             .arg    = error.id};
    psatFSM_postEvent(&event);
}