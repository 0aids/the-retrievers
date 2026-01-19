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

// Testing configuration
#include "loraImpl_testing_config.h"

// --- loraImpl.c Test Configuration Defaults ---
bool loraImpl_testing_forceTxTimeout = false;
bool loraImpl_testing_forceRxError = false;
uint8_t loraImpl_testing_packetLossPercent = 0;
uint32_t loraImpl_testing_txDelayMs = 0;
uint32_t loraImpl_testing_dropPacketNumber = 0; // 0 is disabled
loraImpl_testing_manglingType_e loraImpl_testing_manglingType = loraImpl_manglingType_none;

// Internal counter for deterministic packet dropping
static uint32_t sendCounter = 0;

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

// Configuration
#define SOCK_PORT_D      50037
#define SOCK_BUFFER_SIZE (2 << 10)

// Unified Global Variables (Refactored)
static int                mySockFd = -1;
static struct sockaddr_in myAddr =
    {}; // Address this process binds to
static struct sockaddr_in peerAddr =
    {}; // Address this process sends to
static char    rxBuffer[SOCK_BUFFER_SIZE] = {};

static int64_t lastSetRxTime = 0;
static int64_t rxDuration = 0;

// Set externally (e.g., via command line args in main.c)
extern int8_t isServer;
extern uint16_t interPacketDelayMS;

// Function to reset the send counter, exposed via test_helpers
void loraImpl_resetSendCounter() {
    sendCounter = 0;
}

// --- Helper Macros ---
#define runCallback(callback, ...)                                   \
    {                                                                \
        if (callback != NULL)                                        \
        {                                                            \
            callback(__VA_ARGS__);                                   \
        }                                                            \
    }

static int64_t getTimeMs()
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
    // Seed random number generator for packet loss simulation
    srand(time(NULL) ^ getpid());
    loraImpl_resetSendCounter();

    // 1. Create the socket
    if ((mySockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
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
    }
    else if (isServer == 0)
    {
        // Client: Bind to Localhost, Send to Localhost (Server)
        myAddr.sin_addr.s_addr   = inet_addr("127.0.0.1");
        peerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    else
    {
        fprintf(stderr,
                "loraImpl: Error - isServer must be 0 or 1\n");
        exit(EXIT_FAILURE);
    }

    // 3. Bind the socket
    int opt = 1;
    setsockopt(mySockFd, SOL_SOCKET, SO_REUSEADDR, &opt,
               sizeof(opt));

    if (bind(mySockFd, (struct sockaddr*)&myAddr, sizeof(myAddr)) <
        0)
    {
        perror("loraImpl: Failed to bind socket");
        exit(EXIT_FAILURE);
    }

    // 4. Set socket to non-blocking
    int flags = fcntl(mySockFd, F_GETFL, 0);
    fcntl(mySockFd, F_SETFL, flags | O_NONBLOCK);

    // Clear buffer
    memset(rxBuffer, 0, sizeof(rxBuffer));
}

void loraImpl_deinit(void)
{
    if (mySockFd >= 0)
    {
        close(mySockFd);
        mySockFd = -1;
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
    loraImpl_globalState_g.onTxDone    = onTxDone;
    loraImpl_globalState_g.onRxDone    = onRxDone;
    loraImpl_globalState_g.onTxTimeout = onTxTimeout;
    loraImpl_globalState_g.onRxTimeout = onRxTimeout;
    loraImpl_globalState_g.onRxError   = onRxError;
}

void loraImpl_send(uint8_t* payload, uint16_t payloadSize)
{
    if (mySockFd < 0)
        return;
    
    sendCounter++;

    // --- EDGE CASE: TX Timeout ---
    if (loraImpl_testing_forceTxTimeout)
    {
        loraImpl_testing_forceTxTimeout = false; // Reset flag
        runCallback(loraImpl_globalState_g.onTxTimeout);
        return;
    }

    // --- EDGE CASE: TX Latency/Delay ---
    if (loraImpl_testing_txDelayMs > 0)
    {
        usleep(loraImpl_testing_txDelayMs * 1000);
    }

    // --- EDGE CASE: Deterministic Packet Loss ---
    if (loraImpl_testing_dropPacketNumber > 0 && sendCounter == loraImpl_testing_dropPacketNumber) {
        runCallback(loraImpl_globalState_g.onTxDone); // Pretend it sent
        return;
    }

    // --- EDGE CASE: Percentage Packet Loss ---
    if (loraImpl_testing_packetLossPercent > 0)
    {
        if ((rand() % 100) < loraImpl_testing_packetLossPercent)
        {
            runCallback(loraImpl_globalState_g.onTxDone); // Pretend it sent
            return;
        }
    }

    uint16_t finalPayloadSize = payloadSize;
    uint8_t mangledPayload[payloadSize];
    memcpy(mangledPayload, payload, payloadSize);

    // --- EDGE CASE: Packet Mangling ---
    if (loraImpl_testing_manglingType != loraImpl_manglingType_none)
    {
        switch (loraImpl_testing_manglingType)
        {
            case loraImpl_manglingType_badPreamble:
                if (finalPayloadSize > 0) {
                    mangledPayload[0] = ~mangledPayload[0]; // Flip the bits of the preamble
                }
                break;
            case loraImpl_manglingType_badSize:
                if (finalPayloadSize > 0) {
                    finalPayloadSize -= 1; // Send one less byte
                }
                break;
            default:
                break;
        }
        loraImpl_testing_manglingType = loraImpl_manglingType_none; // Reset flag
    }


    // Unified send function using the pre-calculated peerAddr
    ssize_t sent =
        sendto(mySockFd, mangledPayload, finalPayloadSize, 0,
               (struct sockaddr*)&peerAddr, sizeof(peerAddr));
    usleep(5000);

    if (sent < 0)
    {
        perror("loraImpl: Send failed");
    }
    else
    {
        // Simulate immediate TX completion
        runCallback(loraImpl_globalState_g.onTxDone);
    }
}

static void flushRecvBuffer()
{
    while (1)
    {
        ssize_t n = recvfrom(mySockFd, rxBuffer, sizeof(rxBuffer),
                             MSG_DONTWAIT, NULL, NULL);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // buffer empty
            perror("recvfrom");
        }
    }
}

void loraImpl_irqProcess(void)
{
    if (lastSetRxTime == 0)
    {
        flushRecvBuffer();
    }
    else
    {
        int64_t timeSinceSetRx = getTimeMs() - lastSetRxTime;
        if (rxDuration > 0 && timeSinceSetRx > rxDuration)
        {
            flushRecvBuffer();
            runCallback(loraImpl_globalState_g.onRxTimeout);
            lastSetRxTime = 0;
        } 
        else {
            struct sockaddr_in senderAddr;
            socklen_t          addrLen = sizeof(senderAddr);

            ssize_t len = recvfrom(mySockFd, rxBuffer, SOCK_BUFFER_SIZE, 0,
                                   (struct sockaddr*)&senderAddr, &addrLen);
            flushRecvBuffer();

            if (len > 0)
            {
                if (loraImpl_testing_forceRxError) {
                    loraImpl_testing_forceRxError = false; // Reset flag
                    runCallback(loraImpl_globalState_g.onRxError);
                    return;
                }

                int16_t fakeRssi = -40;
                int8_t  fakeSnr  = 10;

                runCallback(loraImpl_globalState_g.onRxDone,
                            (uint8_t*)rxBuffer, (uint16_t)len, fakeRssi,
                            fakeSnr);
            }
        }
    }
}

void loraImpl_setRx(uint32_t milliseconds)
{
    lastSetRxTime = getTimeMs();
    rxDuration    = milliseconds;
}

void loraImpl_setIdle()
{
    lastSetRxTime = 0;
}

void loraImpl_queryState(void)
{
    // noop
}
