#ifndef LORAIMPL_TESTING_CONFIG_H
#define LORAIMPL_TESTING_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// --- loraImpl.c Test Configuration ---
// These variables must be defined in the test file that includes this header.
// They allow for the simulation of various network and hardware edge cases.

// If true, the next call to loraImpl_send() will immediately trigger the
// onTXTimeout callback instead of sending a packet.
// The flag is automatically reset to false after being triggered.
extern bool loraImpl_testing_force_tx_timeout;

// If true, the next valid received packet in loraImpl_IRQProcess() will be
// ignored and the onRXError callback will be triggered instead.
// The flag is automatically reset to false after being triggered.
extern bool loraImpl_testing_force_rx_error;

// Sets a percentage chance (0-100) that a packet will be "lost" (i.e., not
// sent) during a call to loraImpl_send().
extern uint8_t loraImpl_testing_packet_loss_percent;

// Artificially adds a delay (in milliseconds) before each packet is sent
// via loraImpl_send(), simulating processing latency on the sender.
extern uint32_t loraImpl_testing_tx_delay_ms;

// If non-zero, the Nth packet sent via loraImpl_send will be dropped.
// This is more deterministic than the percentage-based loss.
// A reset function should be called to reset the internal packet counter.
extern uint32_t loraImpl_testing_drop_packet_number;

// Specifies a type of corruption to apply to the next outgoing packet.
typedef enum {
    LORAIMPL_MANGLING_NONE = 0,
    LORAIMPL_MANGLING_BAD_PREAMBLE, // Changes the first byte of the payload.
    LORAIMPL_MANGLING_BAD_SIZE,     // Sends one byte less than the payloadSize.
} loraImpl_testing_mangling_type_e;

// If set to a value other than NONE, the next sent packet will be mangled
// according to the specified type.
// The flag is automatically reset to LORAIMPL_MANGLING_NONE after being triggered.
extern loraImpl_testing_mangling_type_e loraImpl_testing_mangling_type;


#endif // LORAIMPL_TESTING_CONFIG_H
