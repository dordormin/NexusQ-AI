/*
 * NexusQ-AI - QVM Unit Tests
 * File: tests/test_qvm_unit.c
 *
 * Comprehensive unit tests for Quantum Virtual Machine
 */

#include "../modules/quantum/include/qvm.h"
#include <complex.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPSILON 0.0001
#define TEST_PASS "\033[32m✓\033[0m"
#define TEST_FAIL "\033[31m✗\033[0m"

int tests_passed = 0;
int tests_failed = 0;

// Helper: Check if two complex numbers are equal
int complex_equal(double _Complex a, double _Complex b) {
  return (fabs(creal(a) - creal(b)) < EPSILON &&
          fabs(cimag(a) - cimag(b)) < EPSILON);
}

// Helper: Check probability
int prob_equal(double a, double b) { return fabs(a - b) < EPSILON; }

// Test 1: Initialization
void test_qvm_init() {
  printf("\n[TEST] QVM Initialization... ");

  qvm_state_t state;
  qvm_init(&state, 2);

  int size = 1 << 2; // 4 states

  // Check |00> has amplitude 1
  if (!complex_equal(state.amplitudes[0], 1.0 + 0.0 * I)) {
    printf("%s FAIL: |00> amplitude should be 1\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  // Check all other amplitudes are 0
  for (int i = 1; i < size; i++) {
    if (!complex_equal(state.amplitudes[i], 0.0 + 0.0 * I)) {
      printf("%s FAIL: |%d> amplitude should be 0\n", TEST_FAIL, i);
      tests_failed++;
      qvm_free(&state);
      return;
    }
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Test 2: Hadamard Gate
void test_hadamard() {
  printf("[TEST] Hadamard Gate... ");

  qvm_state_t state;
  qvm_init(&state, 1);

  qvm_gate_t h_gate = {.type = GATE_H, .target = 0, .control = -1};
  qvm_apply_gate(&state, &h_gate);

  // After H on |0>, should be (|0> + |1>)/sqrt(2)
  double expected = 1.0 / sqrt(2);

  if (!prob_equal(cabs(state.amplitudes[0]), expected) ||
      !prob_equal(cabs(state.amplitudes[1]), expected)) {
    printf("%s FAIL: Amplitudes incorrect\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Test 3: X (NOT) Gate
void test_x_gate() {
  printf("[TEST] X (NOT) Gate... ");

  qvm_state_t state;
  qvm_init(&state, 1);

  qvm_gate_t x_gate = {.type = GATE_X, .target = 0, .control = -1};
  qvm_apply_gate(&state, &x_gate);

  // After X on |0>, should be |1>
  if (!complex_equal(state.amplitudes[0], 0.0 + 0.0 * I) ||
      !complex_equal(state.amplitudes[1], 1.0 + 0.0 * I)) {
    printf("%s FAIL: State should be |1>\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Test 4: CNOT Gate (Entanglement)
void test_cnot() {
  printf("[TEST] CNOT Gate (Entanglement)... ");

  qvm_state_t state;
  qvm_init(&state, 2);

  // Create superposition on qubit 0
  qvm_gate_t h_gate = {.type = GATE_H, .target = 0, .control = -1};
  qvm_apply_gate(&state, &h_gate);

  // Apply CNOT
  qvm_gate_t cnot = {.type = GATE_CNOT, .control = 0, .target = 1};
  qvm_apply_gate(&state, &cnot);

  // Should create Bell state (|00> + |11>)/sqrt(2)
  double expected = 1.0 / sqrt(2);

  if (!prob_equal(cabs(state.amplitudes[0]), expected) ||
      !prob_equal(cabs(state.amplitudes[3]), expected) ||
      !complex_equal(state.amplitudes[1], 0.0 + 0.0 * I) ||
      !complex_equal(state.amplitudes[2], 0.0 + 0.0 * I)) {
    printf("%s FAIL: Bell state incorrect\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Test 5: Z Gate
void test_z_gate() {
  printf("[TEST] Z Gate... ");

  qvm_state_t state;
  qvm_init(&state, 1);

  // Create superposition
  qvm_gate_t h_gate = {.type = GATE_H, .target = 0, .control = -1};
  qvm_apply_gate(&state, &h_gate);

  // Apply Z
  qvm_gate_t z_gate = {.type = GATE_Z, .target = 0, .control = -1};
  qvm_apply_gate(&state, &z_gate);

  // Z should add phase: (|0> - |1>)/sqrt(2)
  double expected = 1.0 / sqrt(2);

  if (!prob_equal(cabs(state.amplitudes[0]), expected) ||
      !prob_equal(cabs(state.amplitudes[1]), expected)) {
    printf("%s FAIL: Z gate amplitude incorrect\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  // Check phase (sign change on |1>)
  if (creal(state.amplitudes[1]) > -expected + EPSILON) {
    printf("%s FAIL: Z gate phase incorrect\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Test 6: Circuit Parser
void test_parser() {
  printf("[TEST] Circuit Parser... ");

  const char *circuit = "QUBITS 2\n"
                        "H 0\n"
                        "CNOT 0 1\n"
                        "MEASURE 0\n";

  qvm_circuit_t parsed;
  if (qvm_parse_circuit(circuit, &parsed) != 0) {
    printf("%s FAIL: Parse failed\n", TEST_FAIL);
    tests_failed++;
    return;
  }

  if (parsed.num_qubits != 2) {
    printf("%s FAIL: Wrong qubit count\n", TEST_FAIL);
    tests_failed++;
    return;
  }

  if (parsed.num_gates != 3) {
    printf("%s FAIL: Wrong gate count\n", TEST_FAIL);
    tests_failed++;
    return;
  }

  if (parsed.gates[0].type != GATE_H || parsed.gates[1].type != GATE_CNOT ||
      parsed.gates[2].type != GATE_MEASURE) {
    printf("%s FAIL: Wrong gate types\n", TEST_FAIL);
    tests_failed++;
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
}

// Test 7: Normalization
void test_normalization() {
  printf("[TEST] State Normalization... ");

  qvm_state_t state;
  qvm_init(&state, 2);

  // Apply some gates
  qvm_gate_t h0 = {.type = GATE_H, .target = 0, .control = -1};
  qvm_gate_t h1 = {.type = GATE_H, .target = 1, .control = -1};
  qvm_apply_gate(&state, &h0);
  qvm_apply_gate(&state, &h1);

  // Check total probability = 1
  double total_prob = 0.0;
  int size = 1 << state.num_qubits;
  for (int i = 0; i < size; i++) {
    total_prob += cabs(state.amplitudes[i]) * cabs(state.amplitudes[i]);
  }

  if (!prob_equal(total_prob, 1.0)) {
    printf("%s FAIL: Total probability = %f (should be 1.0)\n", TEST_FAIL,
           total_prob);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Test 8: Multiple Gates
void test_multigate_circuit() {
  printf("[TEST] Multi-Gate Circuit... ");

  const char *circuit = "QUBITS 2\n"
                        "H 0\n"
                        "H 1\n"
                        "CNOT 0 1\n"
                        "H 0\n";

  qvm_circuit_t parsed;
  qvm_state_t state;

  if (qvm_parse_circuit(circuit, &parsed) != 0) {
    printf("%s FAIL: Parse failed\n", TEST_FAIL);
    tests_failed++;
    return;
  }

  qvm_init(&state, parsed.num_qubits);
  qvm_execute_circuit(&state, &parsed);

  // Check normalization after execution
  double total_prob = 0.0;
  int size = 1 << state.num_qubits;
  for (int i = 0; i < size; i++) {
    total_prob += cabs(state.amplitudes[i]) * cabs(state.amplitudes[i]);
  }

  if (!prob_equal(total_prob, 1.0)) {
    printf("%s FAIL: Not normalized after circuit\n", TEST_FAIL);
    tests_failed++;
    qvm_free(&state);
    return;
  }

  printf("%s PASS\n", TEST_PASS);
  tests_passed++;
  qvm_free(&state);
}

// Run all tests
void run_all_tests() {
  printf("\n╔═══════════════════════════════════╗\n");
  printf("║     QVM Unit Test Suite v1.0      ║\n");
  printf("╚═══════════════════════════════════╝\n");

  test_qvm_init();
  test_hadamard();
  test_x_gate();
  test_cnot();
  test_z_gate();
  test_parser();
  test_normalization();
  test_multigate_circuit();

  printf("\n┌─── Test Results ───┐\n");
  printf("│ Passed: %d\n", tests_passed);
  printf("│ Failed: %d\n", tests_failed);
  printf("│ Total:  %d\n", tests_passed + tests_failed);
  printf("└────────────────────┘\n");

  if (tests_failed == 0) {
    printf("\n🎉 All tests passed!\n");
  } else {
    printf("\n⚠️  Some tests failed.\n");
  }
}

int main() {
  run_all_tests();
  return (tests_failed == 0) ? 0 : 1;
}
