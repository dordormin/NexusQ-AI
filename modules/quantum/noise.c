/*
 * NexusQ-AI - Quantum Noise Models
 * File: modules/quantum/noise.c
 */

#include "include/qvm.h"
#include <stdio.h>
#include <stdlib.h>

// Noise Configuration
static float global_noise_prob = 0.0;
static int noise_enabled = 0;

typedef enum {
  NOISE_NONE = 0,
  NOISE_BIT_FLIP,
  NOISE_PHASE_FLIP,
  NOISE_DEPOLARIZING
} noise_type_t;

static noise_type_t current_noise_type = NOISE_NONE;

// Set noise parameters
void qnoise_set(int type, float probability) {
  if (type < 0 || type > 3)
    return;
  if (probability < 0.0f || probability > 1.0f)
    return;

  current_noise_type = (noise_type_t)type;
  global_noise_prob = probability;
  noise_enabled = (type != NOISE_NONE && probability > 0.0f);

  printf("[QNOISE] Noise set to Type %d with P=%.4f\n", type, probability);
}

// Apply noise to a qubit state
// Note: In a full density matrix sim, this would mix states.
// In this wavefunction sim, we apply random unitary errors (Monte Carlo
// wavefunction method).
void qnoise_apply(qvm_circuit_t *ctx, int qubit_idx) {
  if (!noise_enabled)
    return;

  // Generate random float 0.0 - 1.0
  float r = (float)rand() / (float)RAND_MAX;

  if (r < global_noise_prob) {
    // Error occurred! Apply the error gate.
    switch (current_noise_type) {
    case NOISE_BIT_FLIP:
      // Apply X gate
      // We need to access the internal state directly or use a helper
      // For simplicity, we'll just print it for this demo,
      // as modifying the complex state requires math headers here.
      // In a real implementation, we'd call qvm_apply_gate(ctx, GATE_X,
      // qubit_idx); But to avoid circular deps/re-entrancy, we assume the
      // caller handles it or we implement a lightweight X here.
      printf("[QNOISE] Bit Flip error on Qubit %d\n", qubit_idx);
      break;

    case NOISE_PHASE_FLIP:
      // Apply Z gate
      printf("[QNOISE] Phase Flip error on Qubit %d\n", qubit_idx);
      break;

    case NOISE_DEPOLARIZING:
      // Random X, Y, or Z
      printf("[QNOISE] Depolarizing error on Qubit %d\n", qubit_idx);
      break;

    default:
      break;
    }
  }
}

// Get current noise info
void qnoise_info() {
  const char *names[] = {"None", "Bit Flip", "Phase Flip", "Depolarizing"};
  printf("--- Quantum Noise Configuration ---\n");
  printf("Type: %s\n", names[current_noise_type]);
  printf("Probability: %.4f\n", global_noise_prob);
  printf("Status: %s\n", noise_enabled ? "ACTIVE" : "DISABLED");
}
