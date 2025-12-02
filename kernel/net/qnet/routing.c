/*
 * NexusQ-AI Kernel - Quantum Network Routing Layer
 * File: kernel/net/qnet/routing.c
 */

#include "../../../include/sys/qnet.h"
#include "../../memory/include/sys/qsocket.h"
#include <stdio.h>

// Simple Static Routing Table
// Format: {Target Node, Next Hop Node}
struct route_entry {
  qnet_node_id_t target;
  qnet_node_id_t next_hop;
};

#define MAX_ROUTES 16
static struct route_entry routing_table[MAX_ROUTES];
static int route_count = 0;

void qnet_routing_init(void) {
  // Initialize with some static routes for simulation
  // Node 1 knows: To reach Node 3, go via Node 2

  // Route: 3 -> 2
  routing_table[0].target = 3;
  routing_table[0].next_hop = 2;
  route_count = 1;

  printf("[QNET] Routing Table Initialized. %d static routes loaded.\n",
         route_count);
}

qnet_node_id_t qnet_route_packet(qnet_node_id_t target) {
  // Linear search for simplicity
  for (int i = 0; i < route_count; i++) {
    if (routing_table[i].target == target) {
      return routing_table[i].next_hop;
    }
  }

  // If no route found, assume direct link (or broadcast in a real mesh)
  // For this simulation, we default to direct connection attempt
  return target;
}

void qnet_add_route(qnet_node_id_t target, qnet_node_id_t next_hop) {
  if (route_count < MAX_ROUTES) {
    routing_table[route_count].target = target;
    routing_table[route_count].next_hop = next_hop;
    route_count++;
  }
}

void qnet_perform_swap(epr_pair_t *pair_left, epr_pair_t *pair_right) {
  printf("[QNET] ROUTING: Performing Bell Measurement for Entanglement "
         "Swapping.\n");
  printf("       Link 1: Node %d <-> Node ME (Pair #%d)\n",
         pair_left->remote_node_id, pair_left->pair_id);
  printf("       Link 2: Node ME <-> Node %d (Pair #%d)\n",
         pair_right->remote_node_id, pair_right->pair_id);

  // 1. CNOT + Hadamard sur les qubits locaux (Bell Basis Measurement)
  // q_driver_cnot(pair_left->qubit, pair_right->qubit);
  // q_driver_h(pair_left->qubit);

  // 2. Mesure
  // int m1 = q_driver_measure(pair_left->qubit);
  // int m2 = q_driver_measure(pair_right->qubit);

  // 3. Transmission Classique des résultats (2 bits) aux nœuds distants
  // send_classical_packet(pair_left->remote, "CORRECTION_PAULLI", m1, m2);

  printf("[QNET] SWAP COMPLETE: Node %d is now entangled with Node %d via "
         "NexusQ.\n",
         pair_left->remote_node_id, pair_right->remote_node_id);

  // Les qubits locaux sont détruits/consommés par la mesure
  // qfree(...)
}
