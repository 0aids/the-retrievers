#include "netinet/in.h"
#include "loraImpl.h"
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
// Global State
loraImpl_globalState_t loraImpl_globalState_g = {
    .onTxDone               = NULL,
    .onRxDone               = NULL,
    .onRxError              = NULL,
    .onRxTimeout            = NULL,
    .onTxTimeout            = NULL,
    .dataBuffer             = {},
    .dataCurrentContentSize = {},
    .rssi                   = {},
    .snr                    = {},
};

void (*loraImpl_onRxDoneCallback)(uint8_t* payload, uint16_t size,
                                  int16_t rssi, int8_t snr);

void (*loraImpl_sendCallback)(uint8_t* payload, uint16_t size);

void loraImpl_init(void) {}

void loraImpl_deinit(void) {}

void loraImpl_setCallbacks(void (*onTxDone)(void),
                           void (*onRxDone)(uint8_t* payload,
                                            uint16_t size,
                                            int16_t rssi, int8_t snr),
                           void (*onTxTimeout)(void),
                           void (*onRxTimeout)(void),
                           void (*onRxError)(void))
{
    loraImpl_onRxDoneCallback = onRxDone;
}

void loraImpl_send(uint8_t* payload, uint16_t payloadSize)
{
    loraImpl_sendCallback(payload, payloadSize);
}

void loraImpl_setSendCallback(void (*sendCallback)(uint8_t* payload,
                                                   uint16_t size))
{
    loraImpl_sendCallback = sendCallback;
}

void loraImpl_irqProcess(void) {}

void loraImpl_setRx(uint32_t milliseconds) {}

void loraImpl_setIdle() {}

void loraImpl_queryState(void)
{
    // noop
}
