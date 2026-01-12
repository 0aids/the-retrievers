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
volatile int callback_fired = 0; // 0=none, 1=RXDone, 2=TXDone, 3=RXTimeout, 4=TXTimeout, 5=RXError

const char serverSendMsg[] = "This is a long message where one packet will be lost.";
const char* whoami = "[Serv]";

#define dprint(...) {printf("%s ", whoami); printf(__VA_ARGS__);}

void RXDoneCallback(uint8_t* p, uint16_t s, int16_t r, int8_t snr) { callback_fired = 1; }
void TXDoneCallback(void) { /* This will be fired multiple times, not useful for this test */ }
void RXTimeoutCallback(void) { callback_fired = 3; }
void TXTimeoutCallback(void) { callback_fired = 4; }
void RXErrorCallback(void) { callback_fired = 5; }

// Client process (receiver)
void runClient()
{
    isServer = 0;
    whoami   = "[Clnt]";
    lora_init();
    lora_setCallbacks(TXDoneCallback, RXDoneCallback, TXTimeoutCallback, RXTimeoutCallback, RXErrorCallback);
    
    // Set a 2 second overall timeout. We expect the inter-packet timeout to fire first.
    lora_setRX(2000);

    dprint("Waiting for a partial message...\n");
    // The lora_IRQProcess will call the backend RXDone, which will set the 
    // inter-packet timeout. We just need to wait for it to fire.
    while(callback_fired == 0) {
        lora_IRQProcess();
        usleep(10000);
    }

    lora_deinit();

    if (callback_fired == 3) {
        dprint("SUCCESS: RXTimeout callback fired as expected after packet loss.\n");
        _exit(0); // Success
    } else {
        dprint("FAIL: Incorrect callback fired. Expected RXTimeout (3), got %d\n", callback_fired);
        _exit(1);
    }
}

// Server process (sender)
void runServer()
{
    isServer = 1;
    lora_init();
    lora_setCallbacks(TXDoneCallback, RXDoneCallback, TXTimeoutCallback, RXTimeoutCallback, RXErrorCallback);
    
    sleep(1);

    dprint("Sending a message, but dropping the 2nd packet (first data packet).\n");
    
    // Set the test configuration to drop the 2nd packet sent by lora_send
    test_helpers_drop_packet_number(2);
    
    lora_send((uint8_t*)serverSendMsg, sizeof(serverSendMsg));
    dprint("Message sequence sent (with one packet dropped).\n");

    lora_deinit();
}

int main()
{
    printf("\n--- Running Test 003: RX Packet Loss ---\n");
    test_helpers_reset_all_configs();

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
            printf("--- Test 003 PASSED ---\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("--- Test 003 FAILED (Client exit code: %d) ---\n", exit_code);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("--- Test 003 FAILED (Client did not exit normally) ---\n");
        exit(EXIT_FAILURE);
    }
}
