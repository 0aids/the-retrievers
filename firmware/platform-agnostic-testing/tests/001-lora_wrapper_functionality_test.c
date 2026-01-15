#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shared_lora.h"
#include <sys/prctl.h>

// ---------------------------------------------
// Test Payload
// ---------------------------------------------
// Large enough to force fragmentation (assuming LoRa packet limit < 100)
char test_msg[] = "This is a long message that should be fragmented into multiple packets to test the reconstruction logic of the shared_lora driver.";

// ---------------------------------------------
// Sender Logic (Parent)
// ---------------------------------------------
void run_sender() {
    setenv("NODE_ID", "0", 1);
    printf("[Sender] Starting...\n");

    // Initialize YOUR driver
    // Note: You might need to call lora_init() if you have a wrapper, 
    // but here we call the impl init and assume you manually setup the logic
    lora_init(); 
    
    // We need to wait a moment for the receiver to spin up
    usleep(100000); 

    // Trigger the send using YOUR high-level logic
    // (Assuming lora_send calls loraImpl_send underneath)
    printf("[Sender] Sending %lu bytes...\n", sizeof(test_msg));
    lora_send((uint8_t*)test_msg, sizeof(test_msg));

    // Pump the loop to handle states
    for (int i = 0; i < 50; i++) {
        lora_IRQProcess(); 
        usleep(10000); // 10ms tick
    }
    
    lora_deinit();
}

// ---------------------------------------------
// Receiver Logic (Child)
// ---------------------------------------------
// We need to hook into the high-level callbacks to see if it worked
void on_app_rx_complete(uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr) {
    printf("\n[Receiver] SUCCESS! Data Reconstructed:\n");
    printf(">> %.*s\n", size, payload);
    exit(0); // Exit child on success
}

void run_receiver() {
    setenv("NODE_ID", "1", 1);
    printf("[Receiver] Listening...\n");

    lora_init();
    
    // Hook up High Level Callbacks
    lora_setCallbacks(NULL, on_app_rx_complete, NULL, NULL, NULL);

    // Run loop indefinitely until we get data
    while(1) {
        lora_IRQProcess();
        usleep(1000); // 1ms tick
    }
}

// ---------------------------------------------
// Main Fork Entry
// ---------------------------------------------
int main() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        return 1;
    }

    if (pid == 0) {
        if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1)
        {
            perror("prctl failed");
            exit(EXIT_FAILURE);
        }
        run_receiver();
    } else {
        run_sender();
    }

    return 0;
}
