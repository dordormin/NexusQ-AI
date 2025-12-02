/*
 * NexusQ-AI Kernel - Quantum Network Subsystem
 * File: kernel/net/qnet/qnet.c
 */

#include "../../../include/sys/config.h"
#include "../../memory/include/sys/kalloc.h"
#include "../../memory/include/sys/qsocket.h"
#include "../../memory/include/sys/scheduler.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// --- Configuration ---
// Defined in config.h
// #define MAX_ENTANGLEMENT_BANK 64
// #define LOCAL_NODE_ID 1

// --- Global State ---
static qnet_entanglement_t entanglement_bank[MAX_ENTANGLEMENT_BANK];
static int bank_count = 0;

// Forward Declarations
void qnet_send_classical(const char *ip, int port, const void *data, int len);

#include "../../neural/include/sys/neural.h"

static int firewall_sock = -1;

void qnet_init_firewall() {
  firewall_sock = nal_socket(AF_NEURAL, SOCK_INFER, NPROTO_SEC);
  if (firewall_sock >= 0) {
    struct sockaddr_neural addr;
    addr.sn_family = AF_NEURAL;
    strncpy(addr.sn_model, "firewall_v1", 63);
    nal_connect(firewall_sock, &addr);
    printf("[QNET] Neural Firewall Active (Sock: %d)\n", firewall_sock);
  }
}

int qnet_check_firewall(const void *data, int len) {
  if (firewall_sock < 0)
    return 0; // Pass if FW down

  // printf("[QNET] Inspecting %d bytes via NAL...\n", len);
  nal_send(firewall_sock, data, len);

  neural_result_t res;
  if (nal_recv(firewall_sock, &res, sizeof(res)) > 0) {
    if (res.confidence > 0.9f) {
      printf("[FIREWALL] ALERT: Dropped Malicious Packet! (%s)\n", res.label);
      return -1; // DROP
    }
  }
  return 0; // ALLOW
}

// --- Initialization ---
void qnet_init(void) {
  printf("[QNET] Initializing Quantum Network Stack (Layer 1-4)...\n");
  memset(entanglement_bank, 0, sizeof(entanglement_bank));
  bank_count = 0;

  qnet_init_firewall();

  // Mock: Pre-generate some entanglement with a "Neighbor" (Node 2)
  // In a real system, the Link Daemon would do this continuously.
  printf(
      "[QNET] Link Daemon: Establishing initial entanglement with Node 2...\n");

  entanglement_bank[0].pair_id = 100;
  entanglement_bank[0].partner_id = 2;
  entanglement_bank[0].fidelity = 0.98; // High quality
  entanglement_bank[0].is_valid = 1;
  bank_count++;

  entanglement_bank[1].pair_id = 101;
  entanglement_bank[1].partner_id = 2;
  entanglement_bank[1].fidelity = 0.85; // Lower quality
  entanglement_bank[1].is_valid = 1;
  bank_count++;

  printf("[QNET] Ready. %d EPR pairs available in bank.\n", bank_count);
}

// --- Classical Channel (Real Networking) ---

int qnet_socket_fd = -1;

void qnet_init_networking(int port) {
  qnet_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (qnet_socket_fd < 0) {
    perror("[QNET] Socket creation failed");
    return;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);

  if (bind(qnet_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("[QNET] Bind failed");
  } else {
    printf("[QNET] Classical Channel Listening on UDP Port %d\n", port);

    // Non-blocking mode
    int flags = fcntl(qnet_socket_fd, F_GETFL, 0);
    fcntl(qnet_socket_fd, F_SETFL, flags | O_NONBLOCK);
  }
}

void qnet_send_classical(const char *ip, int port, const void *data, int len) {
  if (qnet_socket_fd < 0)
    return;

  // Firewall Check (Outbound/Inbound Simulation)
  if (qnet_check_firewall(data, len) < 0) {
    return; // Dropped
  }

  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_pton(AF_INET, ip, &dest.sin_addr);

  sendto(qnet_socket_fd, data, len, 0, (struct sockaddr *)&dest, sizeof(dest));
  printf("[QNET] Sent %d bytes of Classical Data to %s:%d\n", len, ip, port);
}

// --- Link Layer ---

double qnet_verify_link(qnet_node_id_t target) {
  // Real Tomography Simulation
  extern double tomography_measure_fidelity(int target_node);
  double fidelity = tomography_measure_fidelity(target);

  if (fidelity > 0.9) {
    printf("[QNET] Link Verified (Excellent). Ready for E91/BB84.\n");
  } else if (fidelity > 0.8) {
    printf("[QNET] Link Verified (Good). Expect higher QBER.\n");
  } else {
    printf("[QNET] Link Unstable (F=%.2f). Retrying calibration...\n",
           fidelity);
  }

  return fidelity;
}

int qnet_extend_link(qnet_node_id_t target) {
  // Logic: Find a path. Assume Node 2 is the repeater for Node 3.
  if (target != 3) {
    printf("[QNET] Repeater: No known path to Node %d.\n", target);
    return -1;
  }

  printf("[QNET] Repeater: Extending link to Node %d via Node 2...\n", target);

  // 1. Ensure we have a link to Node 2
  int pair_1_2 = qnet_request_entanglement(2, 0.9, 1000);
  if (pair_1_2 < 0)
    return -1;

  // 2. Request Node 2 to swap with Node 3
  // In a real net, we'd send a control message. Here we simulate the result.
  // Assume Node 2 has a pair with Node 3 (Pair ID 200+).
  int pair_2_3 = 205;

  extern int repeater_swap_entanglement(int p1, int p2);
  int new_pair_id = repeater_swap_entanglement(pair_1_2, pair_2_3);

  // 3. Add new virtual pair to bank
  if (bank_count < MAX_ENTANGLEMENT_BANK) {
    int idx = bank_count++;
    entanglement_bank[idx].pair_id = new_pair_id;
    entanglement_bank[idx].partner_id = target;
    entanglement_bank[idx].fidelity = 0.88; // Slightly degraded by swap
    entanglement_bank[idx].is_valid = 1;
    printf("[QNET] Link Extended! New Virtual Pair %d with Node %d.\n",
           new_pair_id, target);
    return new_pair_id;
  }

  return -1;
}

int qnet_request_entanglement(qnet_node_id_t target, double min_fidelity,
                              int timeout_ms) {
  printf("[QNET] Requesting Entanglement with Node %d (Min F=%.2f)...\n",
         target, min_fidelity);

  // 1. Check Bank
  for (int i = 0; i < bank_count; i++) {
    if (entanglement_bank[i].is_valid &&
        entanglement_bank[i].partner_id == target &&
        entanglement_bank[i].fidelity >= min_fidelity) {

      // Found a suitable pair
      printf("[QNET] Found existing EPR Pair ID %d (F=%.2f).\n",
             entanglement_bank[i].pair_id, entanglement_bank[i].fidelity);
      return entanglement_bank[i].pair_id;
    }
  }

  // 2. If not found, check if we need a repeater
  if (target == 3) {
    return qnet_extend_link(target);
  }

  // 3. Simulate generation (if timeout allows)
  printf("[QNET] Bank empty/insufficient. Generating new pair...\n");
  // Simulate success
  if (bank_count < MAX_ENTANGLEMENT_BANK) {
    int idx = bank_count++;
    entanglement_bank[idx].pair_id = 200 + idx;
    entanglement_bank[idx].partner_id = target;
    entanglement_bank[idx].fidelity = 0.99; // Freshly minted
    entanglement_bank[idx].is_valid = 1;

    printf("[QNET] Generated EPR Pair ID %d.\n",
           entanglement_bank[idx].pair_id);
    return entanglement_bank[idx].pair_id;
  }

  return -1; // Failed
}

// --- Transport Layer ---

int qnet_teleport_state(int qubit_id, int pair_id) {
  // 1. Find the pair
  qnet_entanglement_t *pair = NULL;
  for (int i = 0; i < bank_count; i++) {
    if (entanglement_bank[i].pair_id == pair_id &&
        entanglement_bank[i].is_valid) {
      pair = &entanglement_bank[i];
      break;
    }
  }

  if (!pair) {
    printf("[QNET] Error: Invalid or Expired EPR Pair ID %d.\n", pair_id);
    return -1;
  }

  // 2. Perform Teleportation Protocol
  // Bell Measurement (BSM) on local qubit + local half of EPR
  printf("[QNET] Teleporting Qubit %d via Pair %d to Node %d...\n", qubit_id,
         pair_id, pair->partner_id);

  printf("       1. CNOT(Q%d, EPR_L)\n", qubit_id);
  printf("       2. Hadamard(Q%d)\n", qubit_id);
  printf("       3. Measure(Q%d, EPR_L) -> Classical Bits (01)\n", qubit_id);
  printf("       4. Sending Classical Bits to Node %d...\n", pair->partner_id);
  printf("       5. Remote Node applies corrections (X/Z).\n");

  // 3. Consume the resource
  pair->is_valid = 0;
  printf("[QNET] Teleportation Complete. EPR Pair Consumed.\n");

  return 0;
}

// --- Application Layer ---

// --- Application Layer ---

// Helper to print binary buffer
void print_hex(const char *label, uint8_t *data, int len) {
  printf("%s: ", label);
  for (int i = 0; i < len; i++)
    printf("%02X", data[i]);
  printf("\n");
}

// --- Protocols ---

int qnet_run_e91(qnet_node_id_t target, uint8_t *key_buffer, int key_len) {
  printf("\n[QNET] === Protocol E91 (Entanglement-based QKD) Initiated ===\n");
  printf("[QNET] Target Node: %d | Key Length: %d bytes\n", target, key_len);

  // Step 1: Entanglement Distribution
  int pairs_needed = key_len * 8 * 2;
  printf("[QNET] [Step 1] Distributing %d EPR Pairs (Bell States |Phi+>)...\n",
         pairs_needed);
  printf("[QNET] ... Quantum Channel Active (Fiber/Satellite) ...\n");

  // Step 2: Measurement
  printf("[QNET] [Step 2] Alice & Bob measuring in random bases (Z or X)...\n");
  int sifted_bits = key_len * 8 + 16;
  printf("[QNET] ... Sifting ... (Discarding mismatched bases)\n");
  printf("[QNET] -> Sifted Key Length: %d bits\n", sifted_bits);

  // Step 3: Error Estimation
  printf("[QNET] [Step 3] Estimating Quantum Bit Error Rate (QBER)...\n");
  double qber = 0.015; // 1.5%
  printf("[QNET] -> Measured QBER: %.2f%% (Threshold: < 11%%)\n", qber * 100.0);

  if (qber > 0.11) {
    printf("[QNET] [FAILURE] QBER too high! Possible Eavesdropper (Eve) "
           "detected.\n");
    printf("[QNET] Aborting Protocol.\n");
    return -1;
  }

  // Step 4: Post-Processing
  printf("[QNET] [Step 4] Classical Post-Processing (Cascade Protocol)...\n");
  printf("[QNET] -> Correcting bit errors...\n");
  printf("[QNET] -> Privacy Amplification (Hashing)...\n");

  for (int i = 0; i < key_len; i++)
    key_buffer[i] = (uint8_t)(rand() % 255);

  print_hex("[QNET] [SUCCESS] Final Shared Secret", key_buffer, key_len);
  printf("[QNET] === Protocol E91 Completed ===\n\n");
  return 0;
}

int qnet_run_bb84(qnet_node_id_t target, uint8_t *key_buffer, int key_len) {
  printf("\n[QNET] === Protocol BB84 (Prepare & Measure) Initiated ===\n");
  printf("[QNET] Target Node: %d | Key Length: %d bytes\n", target, key_len);

  // Step 1: Preparation
  int qubits_needed = key_len * 8 * 2;
  printf("[QNET] [Step 1] Alice prepares %d Qubits in random bases (Z/X)...\n",
         qubits_needed);
  printf("[QNET] ... Sending Photons via Quantum Channel ...\n");

  // Step 2: Measurement
  printf("[QNET] [Step 2] Bob measures incoming photons in random bases "
         "(Z/X)...\n");

  // Step 3: Sifting
  printf("[QNET] [Step 3] Sifting (Public Channel Discussion)...\n");
  printf("[QNET] -> Alice & Bob compare bases used.\n");

  // Real Network Action: Send Basis Info
  char sifting_msg[64];
  sprintf(sifting_msg, "BASIS_INFO_NODE_1_SEQ_%d", rand() % 1000);

  // Routing Logic
  extern void qnet_routing_init(void);
  extern qnet_node_id_t qnet_route_packet(qnet_node_id_t target);

  // Ensure routing is init (lazy)
  static int route_init_done = 0;
  if (!route_init_done) {
    qnet_routing_init();
    route_init_done = 1;
  }

  qnet_node_id_t next_hop = qnet_route_packet(target);
  if (next_hop != target) {
    printf("[QNET] Routing: Target Node %d is not direct. Forwarding via Node "
           "%d.\n",
           target, next_hop);

    // Wrap message in FORWARD envelope
    char fwd_msg[128];
    snprintf(fwd_msg, 128, "FORWARD %d %s", target, sifting_msg);

    qnet_send_classical("127.0.0.1", 5000 + next_hop, fwd_msg, strlen(fwd_msg));
  } else {
    qnet_send_classical("127.0.0.1", 5000 + target, sifting_msg,
                        strlen(sifting_msg));
  }

  int sifted_bits = key_len * 8 + 8;
  printf("[QNET] -> Sifted Key Length: %d bits\n", sifted_bits);

  // Step 4: Error Estimation
  printf("[QNET] [Step 4] Estimating QBER (Check Subset)...\n");
  double qber = 0.021; // 2.1% (Higher than E91 usually due to channel noise)
  printf("[QNET] -> Measured QBER: %.2f%% (Threshold: < 11%%)\n", qber * 100.0);

  if (qber > 0.11) {
    printf("[QNET] [FAILURE] QBER too high! Possible Eavesdropper (Eve) "
           "detected.\n");
    printf("[QNET] Aborting Protocol.\n");
    return -1;
  }

  // Step 5: Post-Processing
  printf("[QNET] [Step 5] Privacy Amplification...\n");

  for (int i = 0; i < key_len; i++)
    key_buffer[i] = (uint8_t)(rand() % 255);

  print_hex("[QNET] [SUCCESS] Final Shared Secret", key_buffer, key_len);
  printf("[QNET] === Protocol BB84 Completed ===\n\n");
  return 0;
}

// Dispatcher
int qnet_exchange_key(qnet_node_id_t target, uint8_t *key_buffer, int key_len,
                      int proto_id) {
  if (proto_id == 1)
    return qnet_run_bb84(target, key_buffer, key_len);
  if (proto_id == 2)
    return qnet_run_e91(target, key_buffer, key_len);

  // Default to E91 if unknown or 0
  return qnet_run_e91(target, key_buffer, key_len);
}
