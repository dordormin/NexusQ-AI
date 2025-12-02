/*
 * NexusQ-AI - Quantum Hardware Abstraction Layer (QHAL)
 * File: modules/quantum/qhal.c
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define QHAL_ROWS 4
#define QHAL_COLS 4
#define QHAL_NUM_QUBITS (QHAL_ROWS * QHAL_COLS)

// Physical Qubit ID: 0..15
// Grid Layout:
//  0 -- 1 -- 2 -- 3
//  |    |    |    |
//  4 -- 5 -- 6 -- 7
//  |    |    |    |
//  8 -- 9 -- 10-- 11
//  |    |    |    |
// 12 -- 13 -- 14-- 15

typedef struct {
  int id;
  int row;
  int col;
} PhysicalQubit;

static PhysicalQubit grid[QHAL_NUM_QUBITS];
static bool initialized = false;

void qhal_init() {
  if (initialized)
    return;

  for (int r = 0; r < QHAL_ROWS; r++) {
    for (int c = 0; c < QHAL_COLS; c++) {
      int id = r * QHAL_COLS + c;
      grid[id].id = id;
      grid[id].row = r;
      grid[id].col = c;
    }
  }
  initialized = true;
  printf("[QHAL] Initialized 4x4 Superconducting Grid Topology.\n");
}

// Check if two physical qubits are connected (neighbors)
bool qhal_is_connected(int p1, int p2) {
  if (!initialized)
    qhal_init();
  if (p1 < 0 || p1 >= QHAL_NUM_QUBITS || p2 < 0 || p2 >= QHAL_NUM_QUBITS)
    return false;

  int r1 = grid[p1].row;
  int c1 = grid[p1].col;
  int r2 = grid[p2].row;
  int c2 = grid[p2].col;

  // Manhattan distance must be 1
  int dist = abs(r1 - r2) + abs(c1 - c2);
  return (dist == 1);
}

int qhal_get_num_qubits() { return QHAL_NUM_QUBITS; }

void qhal_print_topology() {
  if (!initialized)
    qhal_init();
  printf("Physical Topology (4x4 Grid):\n");
  for (int r = 0; r < QHAL_ROWS; r++) {
    for (int c = 0; c < QHAL_COLS; c++) {
      printf(" %02d ", r * QHAL_COLS + c);
      if (c < QHAL_COLS - 1)
        printf("--");
    }
    printf("\n");
    if (r < QHAL_ROWS - 1) {
      for (int c = 0; c < QHAL_COLS; c++)
        printf("  |   ");
      printf("\n");
    }
  }
}
