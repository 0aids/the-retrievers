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

const char serverSendMsg[] = "This is a long message being sent by the server to test the happy path.";
const char* whoami = "[Serv]";
char received_buffer[2049] = {};

#define dprint(...) {printf("%s ", whoami); printf(__VA_ARGS__);}

void RXDoneCallback(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr)
{
    memcpy(received_buffer, payload, size);
    received_buffer[size] = 0;
    callback_fired = 1;
}

void TXDoneCallback(void)
{
    callback_fired = 2;
}
void RXTimeoutCallback(void)
{
    callback_fired = 3;
}
void TXTimeoutCallback(void)
{
    callback_fired = 4;
}
void RXErrorCallback(void)
{
    callback_fired = 5;
}

// Client process (receiver)
void runClient()
{
    isServer = 0;
    whoami   = "[Clnt]";
    lora_init();
    lora_setCallbacks(TXDoneCallback, RXDoneCallback, TXTimeoutCallback, RXTimeoutCallback, RXErrorCallback);
    
    // Listen for a message for up to 5 seconds
    lora_setRX(5000);

    dprint("Waiting for message...\n");
    while(callback_fired == 0) {
        lora_IRQProcess();
        usleep(10000); // Poll every 10ms
    }

    lora_deinit();

    if (callback_fired != 1) {
        dprint("FAIL: Incorrect callback fired. Expected RXDone (1), got %d\n", callback_fired);
        _exit(1);
    }
    
    if (strcmp(received_buffer, serverSendMsg) != 0) {
        dprint("FAIL: Received message does not match sent message.\n");
        dprint("   Sent:     %s\n", serverSendMsg);
        dprint("   Received: %s\n", received_buffer);
        _exit(2);
    }

    dprint("SUCCESS: Message received correctly.\n");
    _exit(0); // Success
}

// Server process (sender)
void runServer()
{
    isServer = 1;
    lora_init();
    lora_setCallbacks(TXDoneCallback, RXDoneCallback, TXTimeoutCallback, RXTimeoutCallback, RXErrorCallback);
    
    // Give the client time to start listening
    sleep(1);

    dprint("Sending message...\n");
    lora_send((uint8_t*)serverSendMsg, sizeof(serverSendMsg));
    dprint("Message sent.\n");

    lora_deinit();
}

int main()
{
    printf("\n--- Running Test 001: Happy Path ---\n");
    // Reset all testing configurations to ensure a clean run
    test_helpers_reset_all_configs();

    pid_t clientProcess = fork();
    if (clientProcess < 0)
    {
        perror("Failed to fork client process!");
        exit(EXIT_FAILURE);
    }
    if (clientProcess == 0)
    {
        // Child process: run the client
        // Ensure child process is killed if parent dies
        if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1)
        {
            perror("prctl failed");
            exit(EXIT_FAILURE);
        }
        runClient();
        _exit(127); // Should not be reached
    }

    // Parent process: run the server
    runServer();

    // Wait for the client to finish and check its exit code
    int status;
    waitpid(clientProcess, &status, 0);

    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
        {
            printf("--- Test 001 PASSED ---\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("--- Test 001 FAILED (Client exit code: %d) ---\n", exit_code);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("--- Test 001 FAILED (Client did not exit normally) ---\n");
        exit(EXIT_FAILURE);
    }
}
