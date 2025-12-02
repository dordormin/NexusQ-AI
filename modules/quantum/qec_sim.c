/*
 * NexusQ-AI - Quantum Error Correction Simulation (Repetition Code)
 * File: modules/quantum/qec_sim.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simulation state for 3 physical qubits representing 1 logical qubit
typedef struct {
  int physical_qubits[3]; // 0 or 1 (Classical simulation of basis states for
                          // demo)
  int syndrome[2];        // Parity measurements
} qec_sim_t;

static qec_sim_t qec_state;
static int total_errors_detected = 0;
static int total_errors_corrected = 0;

// Get QEC Statistics
void qec_get_stats(int *detected, int *corrected) {
  *detected = total_errors_detected;
  *corrected = total_errors_corrected;
}

// Initialize logical qubit |0>
void qec_sim_init() {
  qec_state.physical_qubits[0] = 0;
  qec_state.physical_qubits[1] = 0;
  qec_state.physical_qubits[2] = 0;
  printf("[QEC] Initialized Logical Qubit |0> (Physical: 000)\n");
}

// Encode: |0> -> |000>, |1> -> |111>
// Here we simulate encoding a state. For demo, let's assume we start with |0>
// or |1>
void qec_sim_encode(int logical_val) {
  int val = logical_val ? 1 : 0;
  qec_state.physical_qubits[0] = val;
  qec_state.physical_qubits[1] = val;
  qec_state.physical_qubits[2] = val;
  printf("[QEC] Encoded Logical |%d> -> |%d%d%d>\n", val, val, val, val);
}

// Inject Noise (Bit Flip)
void qec_sim_inject_error(int qubit_idx) {
  if (qubit_idx < 0 || qubit_idx > 2)
    return;
  qec_state.physical_qubits[qubit_idx] ^= 1; // Flip bit
  printf(
      "[QEC] \033[1;31mNOISE INJECTED\033[0m: Bit Flip on Physical Qubit %d\n",
      qubit_idx);
  printf("[QEC] Current State: |%d%d%d>\n", qec_state.physical_qubits[0],
         qec_state.physical_qubits[1], qec_state.physical_qubits[2]);
}

// Measure Syndromes (Parity checks)
// S1 = Q0 XOR Q1
// S2 = Q1 XOR Q2
void qec_sim_detect() {
  qec_state.syndrome[0] =
      qec_state.physical_qubits[0] ^ qec_state.physical_qubits[1];
  qec_state.syndrome[1] =
      qec_state.physical_qubits[1] ^ qec_state.physical_qubits[2];

  printf("[QEC] Syndrome Measurement: S1=%d, S2=%d\n", qec_state.syndrome[0],
         qec_state.syndrome[1]);
}

// Correct Error based on Syndrome
void qec_sim_correct() {
  int s1 = qec_state.syndrome[0];
  int s2 = qec_state.syndrome[1];

  if (s1 == 0 && s2 == 0) {
    printf("[QEC] No error detected.\n");
  } else if (s1 == 1 && s2 == 0) {
    // Error on Q0 (0 != 1, 1 == 2) -> 100 or 011 error pattern?
    // Wait, for repetition code:
    // 000 -> 100 (S1=1, S2=0) -> Error Q0
    printf("[QEC] \033[1;32mCORRECTING\033[0m: Error detected on Qubit 0\n");
    qec_state.physical_qubits[0] ^= 1;
    total_errors_detected++;
    total_errors_corrected++;
  } else if (s1 == 1 && s2 == 1) {
    // 000 -> 010 (S1=1, S2=1) -> Error Q1
    printf("[QEC] \033[1;32mCORRECTING\033[0m: Error detected on Qubit 1\n");
    qec_state.physical_qubits[1] ^= 1;
    total_errors_detected++;
    total_errors_corrected++;
  } else if (s1 == 0 && s2 == 1) {
    // 000 -> 001 (S1=0, S2=1) -> Error Q2
    printf("[QEC] \033[1;32mCORRECTING\033[0m: Error detected on Qubit 2\n");
    qec_state.physical_qubits[2] ^= 1;
    total_errors_detected++;
    total_errors_corrected++;
  }

  printf("[QEC] Post-Correction State: |%d%d%d>\n",
         qec_state.physical_qubits[0], qec_state.physical_qubits[1],
         qec_state.physical_qubits[2]);
}

// Full Demo
void qec_run_demo() {
  printf("\n=== Quantum Error Correction Demo (3-Qubit Repetition Code) ===\n");

  // 1. Initialize and Encode
  qec_sim_encode(0); // Encode logical |0>

  // 2. Inject Random Error
  int error_qubit = rand() % 3;
  qec_sim_inject_error(error_qubit);

  // 3. Detect
  qec_sim_detect();

  // 4. Correct
  qec_sim_correct();

  // 5. Verify
  int decoded =
      qec_state
          .physical_qubits[0]; // Majority vote or just take Q0 after correction
  if (decoded == 0) {
    printf("[QEC] \033[1;34mSUCCESS\033[0m: Logical state |0> preserved!\n");
  } else {
    printf("[QEC] FAILURE: Logical state corrupted.\n");
  }
  printf("=============================================================\n");
}
