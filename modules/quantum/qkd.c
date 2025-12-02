/*
 * NexusQ-AI - Quantum Key Distribution (BB84 Protocol)
 * File: modules/quantum/qkd.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define QKD_BASIS_Z 0 // Standard Basis (|0>, |1>)
#define QKD_BASIS_X 1 // Hadamard Basis (|+>, |->)

// Alice's State
static int *alice_bases = NULL;
static int num_bits_total = 0;

// Bob's State
static int *bob_bases = NULL;
static int *bob_measurements = NULL;

// Final Key
static int *final_key = NULL;
static int final_key_len = 0;

// Stats
static int total_keys_generated = 0;
static float last_qber = 0.0f;

// Get QKD Statistics
void qkd_get_stats(int *keys, float *qber) {
  *keys = total_keys_generated;
  *qber = last_qber;
}

// Initialize QKD Session
void qkd_init(int n_bits) {
  if (alice_bases)
    free(alice_bases);
  if (bob_bases)
    free(bob_bases);
  if (bob_measurements)
    free(bob_measurements);
  if (final_key)
    free(final_key);

  num_bits_total = n_bits;
  alice_bases = (int *)malloc(n_bits * sizeof(int));
  bob_bases = (int *)malloc(n_bits * sizeof(int));
  bob_measurements = (int *)malloc(n_bits * sizeof(int));
  final_key = (int *)malloc(n_bits * sizeof(int));

  srand(time(NULL));
  printf("[QKD] Session Initialized for %d bits.\n", n_bits);
}

// Run BB84 Protocol
void qkd_run_bb84(int n_bits, int eavesdrop) {
  qkd_init(n_bits);

  // Alice's original bits (immutable record)
  int *alice_sent_bits = (int *)malloc(n_bits * sizeof(int));

  printf("\n--- Step 1: Alice Prepares Qubits ---\n");
  for (int i = 0; i < n_bits; i++) {
    alice_sent_bits[i] = rand() % 2;
    alice_bases[i] = rand() % 2;
    // The 'qubit' state is now (alice_sent_bits[i], alice_bases[i])
  }
  printf("[Alice] Prepared %d qubits.\n", n_bits);

  printf("\n--- Step 2: Transmission (Quantum Channel) ---\n");
  // We simulate the qubit state as it travels
  int *qubits_in_flight = (int *)malloc(n_bits * sizeof(int));
  memcpy(qubits_in_flight, alice_sent_bits, n_bits * sizeof(int));

  if (eavesdrop) {
    printf("[Eve] \033[1;31mEAVESDROPPING DETECTED!\033[0m\n");
    for (int i = 0; i < n_bits; i++) {
      int eve_basis = rand() % 2;
      if (eve_basis != alice_bases[i]) {
        // Eve chose wrong basis -> 50% chance to flip the bit state
        if (rand() % 2 == 0) {
          qubits_in_flight[i] = 1 - qubits_in_flight[i];
        }
      }
    }
  } else {
    printf("[Channel] Secure transmission...\n");
  }

  printf("\n--- Step 3: Bob Measures ---\n");
  for (int i = 0; i < n_bits; i++) {
    bob_bases[i] = rand() % 2;

    if (bob_bases[i] == alice_bases[i]) {
      // Bases match: Measurement should match qubit state
      bob_measurements[i] = qubits_in_flight[i];
    } else {
      // Bases mismatch: Measurement is random (50/50)
      bob_measurements[i] = rand() % 2;
    }
  }
  printf("[Bob] Measured all qubits.\n");

  printf("\n--- Step 4: Sifting (Classical Communication) ---\n");
  final_key_len = 0;
  int errors = 0;
  int sifted_bits = 0;

  printf("Idx | Alice (Bit/Bas) | Bob (Bas/Res) | Key | Check\n");
  printf("-----------------------------------------------------\n");

  for (int i = 0; i < n_bits; i++) {
    if (alice_bases[i] == bob_bases[i]) {
      sifted_bits++;
      int bit = bob_measurements[i];
      final_key[final_key_len++] = bit;

      // Check for error (Alice sent vs Bob measured)
      char status[10] = "OK";
      if (bit != alice_sent_bits[i]) {
        errors++;
        strcpy(status, "ERROR");
      }

      if (sifted_bits <= 10) { // Show first 10 matches
        printf("%03d |   %d   /   %c   |  %c  /   %d   |  %d  | %s\n", i,
               alice_sent_bits[i], (alice_bases[i] ? 'X' : 'Z'),
               (bob_bases[i] ? 'X' : 'Z'), bob_measurements[i], bit, status);
      }
    }
  }

  if (sifted_bits > 10)
    printf("... (remaining bits hidden)\n");

  printf("\n--- Step 5: Results ---\n");
  printf("Total Qubits: %d\n", n_bits);
  printf("Sifted Key Length: %d (approx 50%%)\n", final_key_len);

  float qber = 0.0f;
  if (final_key_len > 0) {
    qber = (float)errors / (float)final_key_len * 100.0f;
  }

  printf("Errors Detected: %d\n", errors);
  printf("QBER (Quantum Bit Error Rate): %.2f%%\n", qber);

  last_qber = qber;

  if (qber > 10.0f) {
    printf("[Security] \033[1;31mHIGH QBER DETECTED! Eavesdropper present. Key "
           "discarded.\033[0m\n");
  } else {
    printf("[Security] \033[1;32mChannel Secure. Key Accepted.\033[0m\n");
    total_keys_generated += final_key_len;
    printf("Final Key (Hex): ");
    for (int i = 0; i < final_key_len / 4 && i < 16; i++) {
      int val = 0;
      if (i * 4 + 0 < final_key_len)
        val |= final_key[i * 4 + 0] << 3;
      if (i * 4 + 1 < final_key_len)
        val |= final_key[i * 4 + 1] << 2;
      if (i * 4 + 2 < final_key_len)
        val |= final_key[i * 4 + 2] << 1;
      if (i * 4 + 3 < final_key_len)
        val |= final_key[i * 4 + 3] << 0;
      printf("%X", val);
    }
    printf("...\n");
  }

  free(alice_sent_bits);
  free(qubits_in_flight);
}
