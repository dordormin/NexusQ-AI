/*
 * NexusQ-AI - Quantum Teleportation Protocol
 * File: modules/quantum/teleport.c
 *
 * Implements the standard Quantum Teleportation protocol to transfer
 * a qubit state from Alice to Bob using shared entanglement.
 */

#include "include/qvm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Teleportation State
typedef struct {
  qvm_state_t state;
  int alice_payload; // Qubit 0
  int alice_ancilla; // Qubit 1
  int bob_target;    // Qubit 2
} teleport_ctx_t;

static teleport_ctx_t ctx;

// Initialize Teleportation Network (3 Qubits)
void teleport_init() {
  // 3 Qubits: 0 (Payload), 1 (Alice's half), 2 (Bob's half)
  qvm_init(&ctx.state, 3);
  ctx.alice_payload = 0;
  ctx.alice_ancilla = 1;
  ctx.bob_target = 2;

  printf("[TELEPORT] Network Initialized (3 Qubits).\n");
}

// Prepare Payload State |psi> = alpha|0> + beta|1> on Qubit 0
void teleport_prepare_payload(double alpha, double beta) {
  // Reset state first (simplified by re-init for demo)
  qvm_free(&ctx.state);
  qvm_init(&ctx.state, 3);

  // Apply rotation to create |psi>
  // We approximate by applying Ry(theta) where alpha = cos(theta/2)
  double theta = 2.0 * acos(alpha);

  // Custom rotation gate (using standard gates sequence or direct manipulation)
  // For this demo, we'll just set the amplitudes directly for Qubit 0
  // Since qvm_init sets |000> = 1.0
  // We want (alpha|0> + beta|1>) x |00>
  // |000> = alpha, |001> = beta (Wait, qubit 0 is LSB or MSB?)
  // In our QVM, index i = q_n ... q_0. So qubit 0 is LSB.
  // |000> (0) -> alpha
  // |001> (1) -> beta

  ctx.state.amplitudes[0] = alpha;
  ctx.state.amplitudes[1] = beta;

  printf("[TELEPORT] Alice prepared payload |psi> = %.2f|0> + %.2f|1>\n", alpha,
         beta);
}

// Execute Teleportation Protocol
void teleport_exec() {
  printf("[TELEPORT] Step 1: Distributing Entanglement (Bell Pair)...\n");
  // Create Bell Pair (|Phi+>) between Qubit 1 (Alice) and Qubit 2 (Bob)
  // H on 1, CNOT 1->2
  qvm_gate_t h_gate = {GATE_H, ctx.alice_ancilla, -1};
  qvm_apply_gate(&ctx.state, &h_gate);

  qvm_gate_t cnot_gate = {GATE_CNOT, ctx.bob_target,
                          ctx.alice_ancilla}; // Control, Target
  // Wait, struct is {type, target, control} usually?
  // Let's check qvm.h or usage. In qvm.c: apply_cnot(state, gate->control,
  // gate->target) Struct definition: int type; int target; int control;
  cnot_gate.control = ctx.alice_ancilla;
  cnot_gate.target = ctx.bob_target;
  qvm_apply_gate(&ctx.state, &cnot_gate);

  printf(
      "[TELEPORT] Entanglement established between Alice (Q1) and Bob (Q2).\n");

  printf("[TELEPORT] Step 2: Alice performs Bell Measurement...\n");
  // Alice entangles Payload (Q0) with her Ancilla (Q1)
  // CNOT 0->1
  cnot_gate.control = ctx.alice_payload;
  cnot_gate.target = ctx.alice_ancilla;
  qvm_apply_gate(&ctx.state, &cnot_gate);

  // H on 0
  h_gate.target = ctx.alice_payload;
  qvm_apply_gate(&ctx.state, &h_gate);

  // Measure Q0 and Q1
  qvm_measure(&ctx.state, ctx.alice_payload);
  qvm_measure(&ctx.state, ctx.alice_ancilla);

  int m1 = ctx.state.measured[ctx.alice_payload];
  int m2 = ctx.state.measured[ctx.alice_ancilla];

  printf("[TELEPORT] Alice Measured: M1=%d, M2=%d\n", m1, m2);
  printf("[TELEPORT] Sending classical bits to Bob...\n");

  printf("[TELEPORT] Step 3: Bob applies corrections...\n");
  // If M2=1, apply X to Q2
  if (m2 == 1) {
    printf("[TELEPORT] Bob applies X gate.\n");
    qvm_gate_t x_gate = {GATE_X, ctx.bob_target, -1};
    qvm_apply_gate(&ctx.state, &x_gate);
  }

  // If M1=1, apply Z to Q2
  if (m1 == 1) {
    printf("[TELEPORT] Bob applies Z gate.\n");
    qvm_gate_t z_gate = {GATE_Z, ctx.bob_target, -1};
    qvm_apply_gate(&ctx.state, &z_gate);
  }

  printf("[TELEPORT] Teleportation Complete!\n");
}

// Verify Result
void teleport_verify(double expected_alpha, double expected_beta) {
  // Check Bob's Qubit (Q2)
  // Since Q0 and Q1 are collapsed, the state should be product state |m1 m2
  // psi> We need to check the amplitudes where q0=m1, q1=m2

  int m1 = ctx.state.measured[ctx.alice_payload];
  int m2 = ctx.state.measured[ctx.alice_ancilla];

  // Index base = (m2 << 1) | m1  (Wait, q1 is bit 1, q0 is bit 0)
  int base_idx = (m2 << 1) | m1;

  // Bob's qubit is bit 2 (MSB in our 3-qubit system? No, usually q2 is 4)
  // Let's assume standard little-endian: |q2 q1 q0>
  // Index = q2*4 + q1*2 + q0

  // We want to check amplitudes for Bob=0 and Bob=1 given Alice's measurements
  int idx_0 = (0 << 2) | (m2 << 1) | m1;
  int idx_1 = (1 << 2) | (m2 << 1) | m1;

  double _Complex amp0 = ctx.state.amplitudes[idx_0];
  double _Complex amp1 = ctx.state.amplitudes[idx_1];

  // Calculate Fidelity
  // |<psi|psi_actual>|^2
  // psi = alpha|0> + beta|1>
  // psi_actual = amp0|0> + amp1|1> (normalized?)

  // Normalize actual
  double norm = sqrt(cabs(amp0) * cabs(amp0) + cabs(amp1) * cabs(amp1));
  amp0 /= norm;
  amp1 /= norm;

  double _Complex overlap =
      conj(expected_alpha) * amp0 + conj(expected_beta) * amp1;
  double fidelity = cabs(overlap) * cabs(overlap);

  printf("\n[TELEPORT] Verification:\n");
  printf("  Expected: %.2f|0> + %.2f|1>\n", expected_alpha, expected_beta);
  printf("  Actual:   %.2f|0> + %.2f|1>\n", creal(amp0), creal(amp1));
  printf("  Fidelity: %.4f\n", fidelity);

  if (fidelity > 0.99) {
    printf("[TELEPORT] SUCCESS: State teleported perfectly.\n");
  } else {
    printf("[TELEPORT] WARNING: Fidelity loss detected.\n");
  }

  qvm_free(&ctx.state);
}

// Demo Wrapper
void teleport_run_demo() {
  teleport_init();

  // Teleport |+> = 1/sqrt(2)(|0> + |1>)
  double alpha = 0.70710678;
  double beta = 0.70710678;

  teleport_prepare_payload(alpha, beta);
  teleport_exec();
  teleport_verify(alpha, beta);
}
