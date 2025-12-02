/*
 * NexusQ-AI - Quantum Topology Mapper (Transpiler)
 * File: modules/quantum/mapper.c
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Import QHAL
extern void qhal_init();
extern bool qhal_is_connected(int p1, int p2);
extern void qhal_print_topology();
extern int qhal_get_num_qubits();

// Logical -> Physical Mapping
static int *l2p_map = NULL;
static int *p2l_map = NULL;
static int num_qubits = 0;

void mapper_init() {
  qhal_init();
  num_qubits = qhal_get_num_qubits();

  if (l2p_map)
    free(l2p_map);
  if (p2l_map)
    free(p2l_map);

  l2p_map = (int *)malloc(num_qubits * sizeof(int));
  p2l_map = (int *)malloc(num_qubits * sizeof(int));

  // Initial Mapping: Trivial (0->0, 1->1...)
  for (int i = 0; i < num_qubits; i++) {
    l2p_map[i] = i;
    p2l_map[i] = i;
  }
}

// Swap two physical qubits in the mapping
void mapper_apply_swap(int p1, int p2) {
  int l1 = p2l_map[p1];
  int l2 = p2l_map[p2];

  // Update maps
  if (l1 != -1)
    l2p_map[l1] = p2;
  if (l2 != -1)
    l2p_map[l2] = p1;

  p2l_map[p1] = l2;
  p2l_map[p2] = l1;

  printf("    [SWAP] Physical %d <-> %d (Logical %d <-> %d)\n", p1, p2, l1, l2);
}

// Simple BFS to find shortest path on grid
// Returns next physical qubit to swap with to get closer
int mapper_find_next_step(int start, int target) {
  // Greedy approach for grid: minimize Manhattan distance
  int r1 = start / 4;
  int c1 = start % 4;
  int r2 = target / 4;
  int c2 = target % 4;

  if (r1 < r2)
    return (r1 + 1) * 4 + c1; // Move Down
  if (r1 > r2)
    return (r1 - 1) * 4 + c1; // Move Up
  if (c1 < c2)
    return r1 * 4 + (c1 + 1); // Move Right
  if (c1 > c2)
    return r1 * 4 + (c1 - 1); // Move Left
  return start;
}

// Route a CNOT between two logical qubits
void mapper_route_cnot(int l_ctrl, int l_target) {
  int p_ctrl = l2p_map[l_ctrl];
  int p_target = l2p_map[l_target];

  printf("  [CNOT] Logical %d -> %d (Physical %d -> %d)\n", l_ctrl, l_target,
         p_ctrl, p_target);

  if (qhal_is_connected(p_ctrl, p_target)) {
    printf("    -> Direct Connection OK.\n");
    return;
  }

  printf("    -> \033[1;33mNot Connected! Routing...\033[0m\n");

  // Move p_ctrl towards p_target until neighbors
  int curr = p_ctrl;
  while (!qhal_is_connected(curr, p_target)) {
    int next = mapper_find_next_step(curr, p_target);
    mapper_apply_swap(curr, next);
    curr = next;
  }

  // Now they are neighbors
  printf("    -> Connection Established (Physical %d -> %d)\n", curr, p_target);
}

void qmap_run_demo() {
  mapper_init();
  printf("\n=== Quantum Topology Mapper Demo ===\n");
  qhal_print_topology();

  printf("\n--- Initial Mapping ---\n");
  printf("Logical:  0  1  2 ...\n");
  printf("Physical: 0  1  2 ...\n");

  // Simulate a random circuit
  printf("\n--- Processing Circuit ---\n");

  // 1. CNOT 0 -> 1 (Connected)
  mapper_route_cnot(0, 1);

  // 2. CNOT 0 -> 5 (Connected)
  mapper_route_cnot(0,
                    5); // 0 is at (0,0), 5 is at (1,1). Not connected directly?
  // Wait, 0 is (0,0). 5 is (1,1). Neighbors of 0 are 1(0,1) and 4(1,0).
  // So 0->5 needs routing.

  // 3. CNOT 0 -> 15 (Far away!)
  mapper_route_cnot(0, 15);

  printf("\n--- Final Mapping ---\n");
  for (int i = 0; i < num_qubits; i++) {
    printf("L%d -> P%d\n", i, l2p_map[i]);
  }
}
