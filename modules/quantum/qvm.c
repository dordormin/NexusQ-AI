/*
 * NexusQ-AI - Quantum Virtual Machine Implementation
 * File: modules/quantum/qvm.c
 */

#include "include/qvm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Initialize quantum state to |0...0>
void qvm_init(qvm_state_t *state, int num_qubits) {
  if (num_qubits > QVM_MAX_QUBITS) {
    printf("[QVM] Error: Max %d qubits supported\n", QVM_MAX_QUBITS);
    return;
  }

  state->num_qubits = num_qubits;
  int size = 1 << num_qubits; // 2^n
  state->amplitudes = (double _Complex *)calloc(size, sizeof(double _Complex));

  // Initialize to |0...0> state
  state->amplitudes[0] = 1.0 + 0.0 * I;

  // Clear measurements
  for (int i = 0; i < QVM_MAX_QUBITS; i++) {
    state->measured[i] = -1;
  }

  printf("[QVM] Initialized %d-qubit state\n", num_qubits);
}

void qvm_free(qvm_state_t *state) {
  if (state->amplitudes) {
    free(state->amplitudes);
    state->amplitudes = NULL;
  }
}

// Apply single-qubit gate
static void apply_single_gate(qvm_state_t *state, int target,
                              double _Complex matrix[2][2]) {
  int size = 1 << state->num_qubits;
  double _Complex *new_amps =
      (double _Complex *)calloc(size, sizeof(double _Complex));

  for (int i = 0; i < size; i++) {
    int bit = (i >> target) & 1;
    int i0 = i & ~(1 << target); // Clear target bit
    int i1 = i | (1 << target);  // Set target bit

    if (bit == 0) {
      new_amps[i] = matrix[0][0] * state->amplitudes[i0] +
                    matrix[0][1] * state->amplitudes[i1];
    } else {
      new_amps[i] = matrix[1][0] * state->amplitudes[i0] +
                    matrix[1][1] * state->amplitudes[i1];
    }
  }

  memcpy(state->amplitudes, new_amps, size * sizeof(double _Complex));
  free(new_amps);
}

// Apply controlled gate
static void apply_cnot(qvm_state_t *state, int control, int target) {
  int size = 1 << state->num_qubits;

  for (int i = 0; i < size; i++) {
    int ctrl_bit = (i >> control) & 1;
    if (ctrl_bit == 1) {
      // Flip target bit
      int j = i ^ (1 << target);
      if (i < j) {
        double _Complex temp = state->amplitudes[i];
        state->amplitudes[i] = state->amplitudes[j];
        state->amplitudes[j] = temp;
      }
    }
  }
}

void qvm_apply_gate(qvm_state_t *state, qvm_gate_t *gate) {
  double _Complex H[2][2] = {{1.0 / sqrt(2), 1.0 / sqrt(2)},
                             {1.0 / sqrt(2), -1.0 / sqrt(2)}};
  double _Complex X[2][2] = {{0, 1}, {1, 0}};
  double _Complex Y[2][2] = {{0, -I}, {I, 0}};
  double _Complex Z[2][2] = {{1, 0}, {0, -1}};
  double _Complex T[2][2] = {{1, 0}, {0, cexp(I * M_PI / 4)}};
  double _Complex S[2][2] = {{1, 0}, {0, I}};

  switch (gate->type) {
  case GATE_H:
    apply_single_gate(state, gate->target, H);
    break;
  case GATE_X:
    apply_single_gate(state, gate->target, X);
    break;
  case GATE_Y:
    apply_single_gate(state, gate->target, Y);
    break;
  case GATE_Z:
    apply_single_gate(state, gate->target, Z);
    break;
  case GATE_T:
    apply_single_gate(state, gate->target, T);
    break;
  case GATE_S:
    apply_single_gate(state, gate->target, S);
    break;
  case GATE_CNOT:
    apply_cnot(state, gate->control, gate->target);
    break;
  case GATE_MEASURE:
    qvm_measure(state, gate->target);
    break;
  default:
    printf("[QVM] Unknown gate type %d\n", gate->type);
  }

  // Apply Noise (if enabled)
  extern void qnoise_apply(void *ctx, int qubit_idx);
  if (gate->type != GATE_MEASURE) {
    qnoise_apply(NULL, gate->target);
    if (gate->type == GATE_CNOT) {
      qnoise_apply(NULL, gate->control);
    }
  }

  // QMonitor Telemetry
  extern void qmonitor_record_gate(int gate_type);
  qmonitor_record_gate(gate->type);
}

void qvm_measure(qvm_state_t *state, int qubit) {
  int size = 1 << state->num_qubits;
  double prob_0 = 0.0;

  // Calculate probability of measuring |0>
  for (int i = 0; i < size; i++) {
    if (((i >> qubit) & 1) == 0) {
      prob_0 += cabs(state->amplitudes[i]) * cabs(state->amplitudes[i]);
    }
  }

  // Random measurement
  srand(time(NULL) + qubit);
  double r = (double)rand() / RAND_MAX;
  int result = (r < prob_0) ? 0 : 1;

  // Collapse state
  double norm = 0.0;
  for (int i = 0; i < size; i++) {
    if (((i >> qubit) & 1) != result) {
      state->amplitudes[i] = 0.0;
    } else {
      norm += cabs(state->amplitudes[i]) * cabs(state->amplitudes[i]);
    }
  }

  // Normalize
  norm = sqrt(norm);
  for (int i = 0; i < size; i++) {
    state->amplitudes[i] /= norm;
  }

  state->measured[qubit] = result;
  printf("[QVM] Measured qubit %d: |%d>\n", qubit, result);
}

void qvm_execute_circuit(qvm_state_t *state, qvm_circuit_t *circuit) {
  printf("[QVM] Executing circuit with %d gates...\n", circuit->num_gates);

  for (int i = 0; i < circuit->num_gates; i++) {
    qvm_apply_gate(state, &circuit->gates[i]);
  }

  printf("[QVM] Circuit execution complete\n");
}

// Parse circuit from text format
// Format: H 0, X 1, CNOT 0 1, MEASURE 0
int qvm_parse_circuit(const char *circuit_text, qvm_circuit_t *circuit) {
  circuit->num_gates = 0;
  char line[256];
  const char *ptr = circuit_text;

  // Read circuit line by line
  while (*ptr) {
    // Skip comments and empty lines
    while (*ptr == '#' || *ptr == '\n' || *ptr == ' ') {
      if (*ptr == '#') {
        while (*ptr && *ptr != '\n')
          ptr++;
      }
      if (*ptr)
        ptr++;
    }

    if (!*ptr)
      break;

    // Read line
    int i = 0;
    while (*ptr && *ptr != '\n' && i < 255) {
      line[i++] = *ptr++;
    }
    line[i] = '\0';

    // Parse gate
    char gate_name[32];
    int target, control;

    if (sscanf(line, "QUBITS %d", &circuit->num_qubits) == 1) {
      continue;
    } else if (sscanf(line, "CNOT %d %d", &control, &target) == 2) {
      circuit->gates[circuit->num_gates].type = GATE_CNOT;
      circuit->gates[circuit->num_gates].control = control;
      circuit->gates[circuit->num_gates].target = target;
      circuit->num_gates++;
    } else if (sscanf(line, "%s %d", gate_name, &target) == 2) {
      if (strcmp(gate_name, "H") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_H;
      } else if (strcmp(gate_name, "X") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_X;
      } else if (strcmp(gate_name, "Y") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_Y;
      } else if (strcmp(gate_name, "Z") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_Z;
      } else if (strcmp(gate_name, "T") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_T;
      } else if (strcmp(gate_name, "S") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_S;
      } else if (strcmp(gate_name, "MEASURE") == 0 ||
                 strcmp(gate_name, "M") == 0) {
        circuit->gates[circuit->num_gates].type = GATE_MEASURE;
      } else {
        printf("[QVM] Unknown gate: %s\n", gate_name);
        continue;
      }
      circuit->gates[circuit->num_gates].target = target;
      circuit->gates[circuit->num_gates].control = -1;
      circuit->num_gates++;
    }
  }

  return 0;
}

void qvm_print_state(qvm_state_t *state) {
  int size = 1 << state->num_qubits;
  printf("\n--- Quantum State ---\n");

  for (int i = 0; i < size; i++) {
    double prob = cabs(state->amplitudes[i]) * cabs(state->amplitudes[i]);
    if (prob > 0.001) { // Only show significant amplitudes
      printf("|");
      for (int j = state->num_qubits - 1; j >= 0; j--) {
        printf("%d", (i >> j) & 1);
      }
      printf(">: %.4f\n", prob);
    }
  }
  printf("---------------------\n");
}

// Userspace wrapper for shell
void qvm_execute_from_text(const char *circuit_text) {
  qvm_circuit_t circuit;
  qvm_state_t state;

  // Parse circuit
  if (qvm_parse_circuit(circuit_text, &circuit) != 0) {
    printf("[QVM] Failed to parse circuit\n");
    return;
  }

  // Initialize state
  qvm_init(&state, circuit.num_qubits);

  // Execute
  clock_t start = clock();
  qvm_execute_circuit(&state, &circuit);
  clock_t end = clock();
  double time_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;

  // Print results
  qvm_print_state(&state);

  // QMonitor Telemetry
  extern void qmonitor_record_execution(const char *name, int qubits, int gates,
                                        double time_ms, int success);
  qmonitor_record_execution("shell_exec", circuit.num_qubits, circuit.num_gates,
                            time_ms, 1);

  // Cleanup
  qvm_free(&state);
}
