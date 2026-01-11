#include "loraImpl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BASE_PORT 9000

// Context
static int                sockfd = -1;
static struct sockaddr_in peer_addr;
static int                my_id = 0;

loraImpl_globalState_t    loraImpl_globalState_g = {
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

void loraImpl_init(void)
{
    // Determine ID from ENV to decide ports (0 or 1)
    char* id_str = getenv("NODE_ID");
    my_id        = id_str ? atoi(id_str) : 0;
    int peer_id  = (my_id == 0) ? 1 : 0;

    // Create Socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Bind My Port
    struct sockaddr_in my_addr = {0};
    my_addr.sin_family         = AF_INET;
    my_addr.sin_addr.s_addr    = htonl(INADDR_LOOPBACK); // 127.0.0.1
    my_addr.sin_port           = htons(BASE_PORT + my_id);

    if (bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Set Peer Address (Where we send to)
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family      = AF_INET;
    peer_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    peer_addr.sin_port        = htons(BASE_PORT + peer_id);

    // Set Non-Blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    printf("[Node %d] Init on Port %d (Peer: %d)\n", my_id,
           BASE_PORT + my_id, BASE_PORT + peer_id);
}

void loraImpl_deinit(void)
{
    if (sockfd >= 0)
        close(sockfd);
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
    // Send immediately via UDP
    sendto(sockfd, payload, payloadSize, 0,
           (struct sockaddr*)&peer_addr, sizeof(peer_addr));

    // In this simplified version, we assume instant success
    if (loraImpl_globalState_g.onTXDone)
        loraImpl_globalState_g.onTXDone();
}

void loraImpl_IRQProcess(void)
{
    uint8_t            buf[2048];
    struct sockaddr_in src;
    socklen_t          len = sizeof(src);

    // Poll socket
    ssize_t n = recvfrom(sockfd, buf, sizeof(buf), 0,
                         (struct sockaddr*)&src, &len);

    if (n > 0)
    {
        // Packet Received
        printf("A packet was received!\n");
        if (loraImpl_globalState_g.onRXDone)
        {
            printf("Running impl RxDone!\n");
            loraImpl_globalState_g.onRXDone(buf, (uint16_t)n, -50,
                      10); // Hardcoded RSSI/SNR
        }
    }
}

// Minimal implementation of state controls
void loraImpl_SetRX(uint32_t milliseconds)
{ 
}
void loraImpl_SetIdle()
{ /* No-op for UDP sim */
}
void loraImpl_queryState(void)
{ /* No-op */
}
