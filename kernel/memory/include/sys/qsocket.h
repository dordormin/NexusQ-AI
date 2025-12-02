/*
 * NexusQ-AI Kernel - Quantum Network Interface (QSocket)
 * Based on "Quantum Internet: A Vision for the Road Ahead" (Wehner et al.)
 * and RFC 9583.
 *
 * File: include/sys/qsocket.h
 */

#ifndef _SYS_QSOCKET_H_
#define _SYS_QSOCKET_H_

#include "../../../../include/sys/config.h"
#include "qproc.h" // For nexus_pid_t
#include <stdint.h>

// --- Quantum Network Layer Definitions ---

// Node Addressing (IPvQ?)
// For now, a simple 32-bit ID. In a real Q-Internet, this would be a
// hierarchical address.
typedef uint32_t qnet_node_id_t;

// Entanglement Fidelity Thresholds
// Defined in config.h
// #define QNET_FIDELITY_LOW 0.80
// #define QNET_FIDELITY_HIGH 0.95
// #define QNET_FIDELITY_ULTRA 0.99

// Protocols
#define QNET_PROTO_BB84 1     // Quantum Key Distribution (Prepare & Measure)
#define QNET_PROTO_E91 2      // Entanglement-based QKD
#define QNET_PROTO_TELEPORT 3 // State Teleportation
#define QNET_PROTO_SWAP 4     // Entanglement Swapping (Repeater)

// --- Structures ---

// Handle for an Entangled Pair (EPR Pair)
// Represents a resource stored in the "Entanglement Bank" (Link Layer)
typedef struct {
  int pair_id;               // Unique ID of the pair in the local QPU
  qnet_node_id_t partner_id; // Who is the other half with?
  double fidelity;           // Estimated fidelity (via Tomography)
  uint64_t creation_time;    // Timestamp (for decoherence tracking)
  int is_valid;              // 1 if available, 0 if consumed/expired
} qnet_entanglement_t;

// QSocket Address
typedef struct {
  qnet_node_id_t node_id;
  int port; // Classical port for control signaling
} qnet_addr_t;

// --- Kernel API (Internal) ---

void qnet_init(void);

// Link Layer: Request Entanglement Generation
// Returns a handle (pair_id) or -1 on failure.
// Blocks until entanglement is established or timeout.
int qnet_request_entanglement(qnet_node_id_t target, double min_fidelity,
                              int timeout_ms);

// Transport Layer: Teleport a Qubit State
// Consumes an entangled pair (pair_id) to transport the state of 'qubit_id'.
// Returns 0 on success.
int qnet_teleport_state(int qubit_id, int pair_id);

// Application Layer: QKD
// Generates a shared secret key using E91 protocol.
// Returns 0 on success, fills 'key_buffer'.
int qnet_exchange_key(qnet_node_id_t target, uint8_t *key_buffer, int key_len,
                      int proto_id);

// Verification: Link Tomography
// Verifies the quality of the link with a neighbor.
double qnet_verify_link(qnet_node_id_t target);

#endif // _SYS_QSOCKET_H_
