#include "include/qvm.h"
#include <stdio.h>

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
  qvm_execute_circuit(&state, &circuit);

  // Print results
  qvm_print_state(&state);

  // Cleanup
  qvm_free(&state);
}
