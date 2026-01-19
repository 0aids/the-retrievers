// AI Disclosure - Written primarily using AI (underlying implementation written by Aidan, testing functionality and tests
//                                             written by Gemini 2.5)
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <string.h>

#include "shared_lora.h"
#include "test_helpers.h"

// Define the global variables required by loraImpl and the test helpers
int8_t isServer = -1;
uint16_t interPacketDelayMS = 5;

// Keep track of which callback was last fired
volatile int callbackFired = 0; // 0=none, 1=RXDone, 2=TXDone, 3=RXTimeout, 4=TXTimeout, 5=RXError

const char serverSendMsg[] = "This message will fail to send.";
const char* whoami = "[Serv]";

#define dprint(...) {printf("%s ", whoami); printf(__VA_ARGS__);}

void rxDoneCallback(uint8_t* p, uint16_t s, int16_t r, int8_t snr) { callbackFired = 1; }
void txDoneCallback(void) { callbackFired = 2; }
void rxTimeoutCallback(void) { callbackFired = 3; }
void txTimeoutCallback(void) { callbackFired = 4; }
void rxErrorCallback(void) { callbackFired = 5; }

// Client process (receiver)
void runClient()
{
    isServer = 0;
    whoami   = "[Clnt]";
    lora_init();
    lora_setCallbacks(txDoneCallback, rxDoneCallback, txTimeoutCallback, rxTimeoutCallback, rxErrorCallback);
    
    // Set a 2 second timeout. The server should fail to send, so we expect this to fire.
    lora_setRx(2000);

    dprint("Waiting for a message that should never arrive...\n");
    while(callbackFired == 0) {
        lora_irqProcess();
        usleep(10000);
    }

    lora_deinit();

    if (callbackFired == 3) {
        dprint("SUCCESS: rxTimeoutCallback fired as expected.\n");
        _exit(0); // Success
    } else {
        dprint("FAIL: Incorrect callback fired. Expected rxTimeoutCallback (3), got %d\n", callbackFired);
        _exit(1);
    }
}

// Server process (sender)
void runServer()
{
    isServer = 1;
    lora_init();
    lora_setCallbacks(txDoneCallback, rxDoneCallback, txTimeoutCallback, rxTimeoutCallback, rxErrorCallback);
    
    sleep(1);

    dprint("Attempting to send a message, but forcing a TX timeout...\n");
    
    // Set the test configuration to force a TX timeout on the next send
    testHelpers_forceTxTimeout(true);
    
    lora_send((uint8_t*)serverSendMsg, sizeof(serverSendMsg));

    lora_deinit();

    if (callbackFired == 4) {
        dprint("SUCCESS: txTimeoutCallback fired as expected.\n");
        // This is the expected outcome for the server
    } else {
        dprint("FAIL: Incorrect callback fired. Expected txTimeoutCallback (4), got %d\n", callbackFired);
        exit(EXIT_FAILURE); // Make the whole test fail if server behaves incorrectly
    }
}

int main()
{
    printf("\n--- Running Test 002: TX Timeout ---\n");
    testHelpers_resetAllConfigs();

    pid_t clientProcess = fork();
    if (clientProcess < 0)
    {
        perror("Failed to fork client process!");
        exit(EXIT_FAILURE);
    }
    if (clientProcess == 0)
    {
        if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1) { perror("prctl failed"); exit(EXIT_FAILURE); }
        runClient();
        _exit(127);
    }

    runServer();

    int status;
    waitpid(clientProcess, &status, 0);

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
        {
            printf("--- Test 002 PASSED ---\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("--- Test 002 FAILED (Client exit code: %d) ---\n", exit_code);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("--- Test 002 FAILED (Client did not exit normally) ---\n");
        exit(EXIT_FAILURE);
    }
}
