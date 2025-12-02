/*
 * NexusQ-AI - Quantum Virtual Machine (QVM)
 * File: modules/quantum/include/qvm.h
 */

#ifndef _QVM_H_
#define _QVM_H_

#include <complex.h>
#include <stdint.h>

#define QVM_MAX_QUBITS 16
#define QVM_MAX_GATES 256
#define QVM_STATE_SIZE (1 << QVM_MAX_QUBITS) // 2^n states

// Quantum gate types
typedef enum {
  GATE_H,      // Hadamard
  GATE_X,      // Pauli-X (NOT)
  GATE_Y,      // Pauli-Y
  GATE_Z,      // Pauli-Z
  GATE_T,      // T gate (π/8)
  GATE_S,      // S gate (π/4)
  GATE_CNOT,   // Controlled-NOT
  GATE_CZ,     // Controlled-Z
  GATE_SWAP,   // SWAP
  GATE_MEASURE // Measurement
} qvm_gate_type_t;

// Gate operation
typedef struct {
  qvm_gate_type_t type;
  int target;  // Target qubit
  int control; // Control qubit (-1 if not used)
} qvm_gate_t;

// Quantum state (statevector simulation)
typedef struct {
  int num_qubits;
  double _Complex *amplitudes;  // 2^n complex amplitudes
  int measured[QVM_MAX_QUBITS]; // Measurement results
} qvm_state_t;

// Quantum circuit
typedef struct {
  int num_qubits;
  int num_gates;
  qvm_gate_t gates[QVM_MAX_GATES];
} qvm_circuit_t;

// QVM API
void qvm_init(qvm_state_t *state, int num_qubits);
void qvm_free(qvm_state_t *state);
void qvm_apply_gate(qvm_state_t *state, qvm_gate_t *gate);
void qvm_measure(qvm_state_t *state, int qubit);
void qvm_execute_circuit(qvm_state_t *state, qvm_circuit_t *circuit);
int qvm_parse_circuit(const char *circuit_text, qvm_circuit_t *circuit);
void qvm_print_state(qvm_state_t *state);

// Userspace helper
void qvm_execute_from_text(const char *circuit_text);

#endif // _QVM_H_
