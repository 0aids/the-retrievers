#include "loraImpl.h"
#include "netinet/in.h"
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
    .onRXDone               = NULL,
    .onTXDone               = NULL,
    .onRXError              = NULL,
    .onRXTimeout            = NULL,
    .onTXTimeout            = NULL,
    .dataBuffer             = {},
    .dataCurrentContentSize = {},
    .rssi                   = {},
    .snr                    = {},
};

// Configuration
#define SOCK_PORT_D      50037
#define SOCK_BUFFER_SIZE (2 << 10)

// Unified Global Variables (Refactored)
static int                mySock_fd = -1;
static struct sockaddr_in myAddr =
    {}; // Address this process binds to
static struct sockaddr_in peerAddr =
    {}; // Address this process sends to
static char    rxBuffer[SOCK_BUFFER_SIZE] = {};

static int64_t lastSetRXTime = 0;
static int64_t rxDuration = 0;

// Set externally (e.g., via command line args in main.c)
extern int8_t isServer;
extern uint16_t interPacketDelayMS;

// --- Helper Macros ---
#define runCallback(callback, ...)                                   \
    {                                                                \
        if (callback != NULL)                                        \
        {                                                            \
            callback(__VA_ARGS__);                                   \
        }                                                            \
    }

static int64_t getTimeMS()
{
    struct timespec tms;
    if (timespec_get(&tms, TIME_UTC) == 0)
    {
        return -1; // Error
    }
    uint64_t milliseconds =
        (uint64_t)tms.tv_sec * 1000 + (uint64_t)tms.tv_nsec / 1000000;
    return milliseconds;
}

// --- Unified Implementation ---

void loraImpl_init(void)
{
    // 1. Create the socket
    if ((mySock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("loraImpl: Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // 2. Configure Address Structs based on Role
    memset(&myAddr, 0, sizeof(myAddr));
    memset(&peerAddr, 0, sizeof(peerAddr));

    myAddr.sin_family = AF_INET;
    myAddr.sin_port   = htons(SOCK_PORT_D); // Use htons for ports

    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port   = htons(SOCK_PORT_D);

    if (isServer == 1)
    {
        // Server: Bind to ANY interface, Send to Localhost (Client)
        myAddr.sin_addr.s_addr   = htonl(INADDR_ANY);
        peerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        printf("loraImpl: Initializing as SERVER (Bind: ANY, Peer: "
               "127.0.0.1)\n");
    }
    else if (isServer == 0)
    {
        // Client: Bind to Localhost, Send to Localhost (Server)
        myAddr.sin_addr.s_addr   = inet_addr("127.0.0.1");
        peerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        printf("loraImpl: Initializing as CLIENT (Bind: 127.0.0.1, "
               "Peer: 127.0.0.1)\n");
    }
    else
    {
        fprintf(stderr,
                "loraImpl: Error - isServer must be 0 or 1\n");
        exit(EXIT_FAILURE);
    }

    // 3. Bind the socket
    // Optional: Set SO_REUSEADDR to allow fast restarts or multiple bindings on some OSs
    int opt = 1;
    setsockopt(mySock_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
               sizeof(opt));

    if (bind(mySock_fd, (struct sockaddr*)&myAddr, sizeof(myAddr)) <
        0)
    {
        perror("loraImpl: Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // 4. Set socket to non-blocking (Standard for LoRa emulation loops)
    int flags = fcntl(mySock_fd, F_GETFL, 0);
    fcntl(mySock_fd, F_SETFL, flags | O_NONBLOCK);

    // Clear buffer
    memset(rxBuffer, 0, sizeof(rxBuffer));
}

void loraImpl_deinit(void)
{
    if (mySock_fd >= 0)
    {
        close(mySock_fd);
        mySock_fd = -1;
    }
    memset(&myAddr, 0, sizeof(myAddr));
    memset(&peerAddr, 0, sizeof(peerAddr));
}

void loraImpl_setCallbacks(void (*onTxDone)(void),
                           void (*onRxDone)(uint8_t* payload,
                                            uint16_t size,
                                            int16_t rssi, int8_t snr),
                           void (*onTxTimeout)(void),
                           void (*onRxTimeout)(void),
                           void (*onRxError)(void))
{
    loraImpl_globalState_g.onTXDone    = onTxDone;
    loraImpl_globalState_g.onRXDone    = onRxDone;
    loraImpl_globalState_g.onTXTimeout = onTxTimeout;
    loraImpl_globalState_g.onRXTimeout = onRxTimeout;
    loraImpl_globalState_g.onRXError   = onRxError;
}

void loraImpl_send(uint8_t* payload, uint16_t payloadSize)
{
    if (mySock_fd < 0)
        return;

    // Unified send function using the pre-calculated peerAddr
    ssize_t sent =
        sendto(mySock_fd, payload, payloadSize, 0,
               (struct sockaddr*)&peerAddr, sizeof(peerAddr));
    usleep(5000);

    if (sent < 0)
    {
        perror("loraImpl: Send failed");
    }
    else
    {
        // Simulate immediate TX completion
        runCallback(loraImpl_globalState_g.onTXDone);
    }
}

static void flushRecvBuffer()
{
    while (1)
    {
        // Flush the buffer by reading.
        ssize_t n = recvfrom(mySock_fd, rxBuffer, sizeof(rxBuffer),
                             MSG_DONTWAIT, NULL, NULL);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // buffer empty
            perror("recvfrom");
        }
    }
}

void loraImpl_IRQProcess(void)
{
    // If we haven't been put in RX mode via loraImpl_SetRX, ignore incoming data
    if (lastSetRXTime == 0)
    {
        flushRecvBuffer();
    }
    else
    {
        // Check if we timed out
        int64_t timeSinceSetRX = getTimeMS() - lastSetRXTime;
        if (rxDuration > 0 && timeSinceSetRX > rxDuration)
        {
            flushRecvBuffer();
            runCallback(loraImpl_globalState_g.onRXTimeout);
            lastSetRXTime = 0;
        } 
        else {
            struct sockaddr_in senderAddr;
            socklen_t          addrLen = sizeof(senderAddr);

            // Non-blocking receive
            ssize_t len = recvfrom(mySock_fd, rxBuffer, SOCK_BUFFER_SIZE, 0,
                                   (struct sockaddr*)&senderAddr, &addrLen);
            // Simulate losing everything else because only one packet at a time on lora.
            flushRecvBuffer();

            if (len > 0)
            {
                // Simulate RSSI/SNR values
                int16_t fakeRssi = -40;
                int8_t  fakeSnr  = 10;

                runCallback(loraImpl_globalState_g.onRXDone,
                            (uint8_t*)rxBuffer, (uint16_t)len, fakeRssi,
                            fakeSnr);
            }
        }
    }

}

// Minimal implementation of state controls
void loraImpl_setRX(uint32_t milliseconds)
{
    // In a real implementation, 'milliseconds' might set a timeout timer.
    // For now, non-zero indicates we are listening.
    // Use current system time or a flag.
    lastSetRXTime = getTimeMS();
    rxDuration    = milliseconds;
}

void loraImpl_setIdle()
{
    lastSetRXTime = 0;
}

void loraImpl_queryState(void)
{
    // noop
}
