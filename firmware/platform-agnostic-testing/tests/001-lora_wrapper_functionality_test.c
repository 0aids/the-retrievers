#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "loraImpl.h"
#include "shared_lora.h"
#include <inttypes.h>
#include <sys/prctl.h>
#include <string.h>

int8_t     isServer = -1;
const char serverSendMsg[] =
    "This is a long message being sent by the server";
const char  servermsg[]  = "[Server]";
const char  clientmsg[]  = "[Client]";
const char* whoami       = servermsg;
char        buffer[2049] = {};

uint16_t interPacketDelayMS = 5;

#define dprint(...) {printf("%s ", whoami); printf(__VA_ARGS__);}

void RXDoneCallback(uint8_t* payload, uint16_t size, int16_t rssi,
                    int8_t snr)
{
    memcpy(buffer, payload, size);
    buffer[size] = 0;
    dprint("Received: %s\n", buffer);
    dprint("Rssi: %" PRId16 ", snr: %" PRId8 "\n", rssi, snr);
}

void TXDoneCallback(void)
{
    dprint("RXDone!\n");
}
void RXTimeoutCallback(void)
{
    dprint("RxTimeout!\n");
}
void TXTimeoutCallback(void)
{
    dprint("TXTimeout!\n");
}
void RXErrorCallback(void)
{
    dprint("RXError!\n");
}

// Run on separate forks.
void runServer()
{
    isServer = 1;
    lora_init();
    lora_setCallbacks(TXDoneCallback, RXDoneCallback,
                      TXTimeoutCallback, RXTimeoutCallback,
                      RXErrorCallback);
    sleep(3);
    dprint("Sending message!\n");
    lora_send((uint8_t*)serverSendMsg, sizeof(serverSendMsg));
    lora_IRQProcess();
    sleep(6);
    lora_deinit();
}

// Run on separate forks.
void runClient()
{
    isServer = 0;
    whoami   = clientmsg;
    lora_init();
    lora_setCallbacks(TXDoneCallback, RXDoneCallback,
                      TXTimeoutCallback, RXTimeoutCallback,
                      RXErrorCallback);
    lora_setRX(0);
    sleep(2);
    dprint("Waiting for msg!\n");
    lora_IRQProcess();
    sleep(1);
    lora_IRQProcess();
    sleep(1);
    lora_IRQProcess();
    sleep(1);
    lora_deinit();
}

int main()
{
    pid_t clientProcess = fork();
    if (clientProcess < 0)
    {
        perror("Failed to fork client process!");
    }
    if (clientProcess == 0)
    {
        if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1)
        {
            perror("prctl failed");
            exit(EXIT_FAILURE);
        }

        runClient();
        _exit(127);
    }

    runServer();
}
