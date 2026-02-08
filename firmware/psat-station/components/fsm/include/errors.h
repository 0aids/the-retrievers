#include "shared_state.h"
#include "sm.h"

#define NUMBER_ERRORS_STORED 10

typedef struct
{
    int head;
    int count;
} circularBuffer_t;

void psatErr_bufferInsert(psatErr_error_t error);

void psatErr_bufferCopyOrdered(
    psatErr_error_t errorOrderedBuffer[NUMBER_ERRORS_STORED]);

psatErr_error_t* psatErr_getMostRecentError();
psatErr_error_t* psatErr_getErrorById(int id);