/*
 * NexusQ-AI - Quantum Circuit Debugger
 * File: modules/quantum/qdbg.c
 *
 * Interactive step-by-step quantum circuit debugger
 */

#include "include/qvm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Debugger state
typedef struct {
  qvm_circuit_t circuit;
  qvm_state_t state;
  int current_gate;
  int breakpoints[QVM_MAX_GATES];
  int num_breakpoints;
  int paused;
} qdbg_session_t;

static qdbg_session_t dbg_session;

// Print current state in readable format
void qdbg_print_state(qvm_state_t *state) {
  int size = 1 << state->num_qubits;
  printf("\n┌─── Quantum State ───┐\n");

  int shown = 0;
  for (int i = 0; i < size && shown < 8; i++) {
    double prob = cabs(state->amplitudes[i]) * cabs(state->amplitudes[i]);
    if (prob > 0.001) {
      printf("│ |");
      for (int j = state->num_qubits - 1; j >= 0; j--) {
        printf("%d", (i >> j) & 1);
      }
      double _Complex amp = state->amplitudes[i];
      printf("> : %.4f + %.4fi  (P=%.3f)\n", creal(amp), cimag(amp), prob);
      shown++;
    }
  }
  printf("└────────────────────┘\n");
}

// Print gate information
void qdbg_print_gate(qvm_gate_t *gate, int index) {
  printf("\n[Gate %d] ", index);

  switch (gate->type) {
  case GATE_H:
    printf("H (Hadamard) on qubit %d\n", gate->target);
    break;
  case GATE_X:
    printf("X (NOT) on qubit %d\n", gate->target);
    break;
  case GATE_Y:
    printf("Y on qubit %d\n", gate->target);
    break;
  case GATE_Z:
    printf("Z on qubit %d\n", gate->target);
    break;
  case GATE_T:
    printf("T (π/8) on qubit %d\n", gate->target);
    break;
  case GATE_S:
    printf("S (π/4) on qubit %d\n", gate->target);
    break;
  case GATE_CNOT:
    printf("CNOT: control=%d, target=%d\n", gate->control, gate->target);
    break;
  case GATE_MEASURE:
    printf("MEASURE qubit %d\n", gate->target);
    break;
  default:
    printf("Unknown gate\n");
  }
}

// Show help
void qdbg_help() {
  printf("\n┌─── QDebug Commands ───┐\n");
  printf("│ n, next    - Execute next gate\n");
  printf("│ c, cont    - Continue to end/breakpoint\n");
  printf("│ s, state   - Show current quantum state\n");
  printf("│ g, gates   - List all gates\n");
  printf("│ b <num>    - Set breakpoint at gate <num>\n");
  printf("│ r, restart - Restart from beginning\n");
  printf("│ h, help    - Show this help\n");
  printf("│ q, quit    - Exit debugger\n");
  printf("└───────────────────────┘\n");
}

// Initialize debugger session
int qdbg_init(const char *circuit_text) {
  // Parse circuit
  if (qvm_parse_circuit(circuit_text, &dbg_session.circuit) != 0) {
    printf("[QDBG] Failed to parse circuit\n");
    return -1;
  }

  // Initialize state
  qvm_init(&dbg_session.state, dbg_session.circuit.num_qubits);

  dbg_session.current_gate = 0;
  dbg_session.num_breakpoints = 0;
  dbg_session.paused = 0;

  printf("\n╔═══════════════════════════════════╗\n");
  printf("║  Quantum Circuit Debugger (QDbg)  ║\n");
  printf("╚═══════════════════════════════════╝\n");
  printf("\nLoaded circuit: %d gates, %d qubits\n",
         dbg_session.circuit.num_gates, dbg_session.circuit.num_qubits);
  printf("Type 'h' for help\n");

  return 0;
}

// Execute next gate
void qdbg_step() {
  if (dbg_session.current_gate >= dbg_session.circuit.num_gates) {
    printf("\n[QDBG] Circuit execution complete!\n");
    qdbg_print_state(&dbg_session.state);
    return;
  }

  qvm_gate_t *gate = &dbg_session.circuit.gates[dbg_session.current_gate];

  printf("\n>>> Executing gate %d/%d:\n", dbg_session.current_gate + 1,
         dbg_session.circuit.num_gates);
  qdbg_print_gate(gate, dbg_session.current_gate);

  // Execute gate
  qvm_apply_gate(&dbg_session.state, gate);

  dbg_session.current_gate++;

  // Show state after execution
  qdbg_print_state(&dbg_session.state);
}

// Continue execution
void qdbg_continue() {
  while (dbg_session.current_gate < dbg_session.circuit.num_gates) {
    // Check breakpoints
    for (int i = 0; i < dbg_session.num_breakpoints; i++) {
      if (dbg_session.breakpoints[i] == dbg_session.current_gate) {
        printf("\n[QDBG] Hit breakpoint at gate %d\n",
               dbg_session.current_gate);
        qdbg_print_gate(&dbg_session.circuit.gates[dbg_session.current_gate],
                        dbg_session.current_gate);
        return;
      }
    }

    qvm_apply_gate(&dbg_session.state,
                   &dbg_session.circuit.gates[dbg_session.current_gate]);
    dbg_session.current_gate++;
  }

  printf("\n[QDBG] Circuit execution complete!\n");
  qdbg_print_state(&dbg_session.state);
}

// List all gates
void qdbg_list_gates() {
  printf("\n┌─── Circuit Gates ───┐\n");
  for (int i = 0; i < dbg_session.circuit.num_gates; i++) {
    printf("│ ");
    if (i == dbg_session.current_gate) {
      printf(">>> ");
    } else {
      printf("    ");
    }
    qdbg_print_gate(&dbg_session.circuit.gates[i], i);
  }
  printf("└────────────────────┘\n");
}

// Add breakpoint
void qdbg_add_breakpoint(int gate_num) {
  if (gate_num < 0 || gate_num >= dbg_session.circuit.num_gates) {
    printf("[QDBG] Invalid gate number\n");
    return;
  }

  if (dbg_session.num_breakpoints >= QVM_MAX_GATES) {
    printf("[QDBG] Too many breakpoints\n");
    return;
  }

  dbg_session.breakpoints[dbg_session.num_breakpoints++] = gate_num;
  printf("[QDBG] Breakpoint set at gate %d\n", gate_num);
}

// Restart debugging session
void qdbg_restart() {
  qvm_free(&dbg_session.state);
  qvm_init(&dbg_session.state, dbg_session.circuit.num_qubits);
  dbg_session.current_gate = 0;
  printf("[QDBG] Restarted. Circuit reset to initial state.\n");
  qdbg_print_state(&dbg_session.state);
}

// Main debugger loop
void qdbg_run() {
  char cmd[64];

  // Show initial state
  qdbg_print_state(&dbg_session.state);

  while (1) {
    printf("\nqdbg> ");
    if (fgets(cmd, sizeof(cmd), stdin) == NULL)
      break;

    cmd[strcspn(cmd, "\n")] = 0;

    if (strlen(cmd) == 0)
      continue;

    if (strcmp(cmd, "n") == 0 || strcmp(cmd, "next") == 0) {
      qdbg_step();
    } else if (strcmp(cmd, "c") == 0 || strcmp(cmd, "cont") == 0) {
      qdbg_continue();
    } else if (strcmp(cmd, "s") == 0 || strcmp(cmd, "state") == 0) {
      qdbg_print_state(&dbg_session.state);
    } else if (strcmp(cmd, "g") == 0 || strcmp(cmd, "gates") == 0) {
      qdbg_list_gates();
    } else if (strncmp(cmd, "b ", 2) == 0) {
      int gate_num = atoi(cmd + 2);
      qdbg_add_breakpoint(gate_num);
    } else if (strcmp(cmd, "r") == 0 || strcmp(cmd, "restart") == 0) {
      qdbg_restart();
    } else if (strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) {
      qdbg_help();
    } else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) {
      break;
    } else {
      printf("Unknown command '%s'. Type 'h' for help.\n", cmd);
    }
  }

  qvm_free(&dbg_session.state);
  printf("\n[QDBG] Debugger session ended.\n");
}

// Entry point for debugger
void qdbg_main(const char *circuit_text) {
  if (qdbg_init(circuit_text) == 0) {
    qdbg_run();
  }
}
