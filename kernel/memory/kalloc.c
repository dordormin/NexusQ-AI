/*
 * NexusQ-AI Kernel - Hybrid Memory Manager Implementation
 * File: kernel/memory/kalloc.c
 */

#include "include/sys/kalloc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // Utilisé UNIQUEMENT pour simuler la RAM physique du host
#include <string.h>

// --- Variables Globales (État du Système) ---
static uint8_t kernel_heap[KERNEL_HEAP_SIZE]; // La RAM simulée du noyau
static size_t heap_offset = 0; // Allocateur linéaire simple (bump pointer)

static qpu_map_t qpu_state; // L'état des registres quantiques

// Memory statistics tracking
static size_t total_allocated_ram = 0;
static size_t total_free_ram = KERNEL_HEAP_SIZE;

// --- Helpers Bitwise pour le QPU ---
static void set_bit(int bit) { qpu_state.bitmap[bit / 8] |= (1 << (bit % 8)); }

static void clear_bit(int bit) {
  qpu_state.bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static int test_bit(int bit) {
  return qpu_state.bitmap[bit / 8] & (1 << (bit % 8));
}

/**
 * Initialisation du Gestionnaire
 */
void kalloc_init(void) {
  // Reset RAM Allocator
  heap_offset = 0;

  // Reset QPU (Tous les qubits libres)
  memset(qpu_state.bitmap, 0, sizeof(qpu_state.bitmap));
  qpu_state.free_qubits = QPU_MAX_QUBITS;

  printf("[MEMORY] Initialized. RAM: %d MB | QPU: %d Qubits\n",
         KERNEL_HEAP_SIZE / (1024 * 1024), QPU_MAX_QUBITS);
}

/**
 * Allocateur Classique (Très simplifié pour ce stade)
 * Note: Dans un vrai OS, on utiliserait un Slab Allocator ou Buddy System.
 */
void *kmalloc(size_t size) {
  if (heap_offset + size > KERNEL_HEAP_SIZE) {
    printf("[MEMORY] PANIC: Kernel Out of Memory (OOM)\n");
    return NULL;
  }

  void *ptr = &kernel_heap[heap_offset];
  heap_offset += size;
  // Alignement mémoire (optionnel mais recommandé)
  if (heap_offset % 8 != 0)
    heap_offset += (8 - (heap_offset % 8));

  // Update statistics
  total_allocated_ram = heap_offset;
  total_free_ram = KERNEL_HEAP_SIZE - heap_offset;

  return ptr;
}

void kfree(void *ptr) {
  // Non implémenté dans un allocateur linéaire simple (Bump Pointer)
  // Dans 4.4BSD, cela remettrait le bloc dans la free-list.
}

/**
 * Allocateur Quantique (qmalloc)
 * Algorithme: First-Fit contigu.
 * Cherche une séquence de 'num_qubits' libres dans le bitmap.
 */
q_register_t qmalloc(uint16_t num_qubits) {
  if (num_qubits == 0)
    return 0;
  if (num_qubits > qpu_state.free_qubits) {
    printf("[MEMORY] QPU Full. Requested: %d, Available: %d\n", num_qubits,
           qpu_state.free_qubits);
    return -1; // Erreur
  }

  int run_length = 0;
  int start_index = -1;

  // Scan du registre
  for (int i = 0; i < QPU_MAX_QUBITS; i++) {
    if (!test_bit(i)) {
      // Bit libre
      if (run_length == 0)
        start_index = i;
      run_length++;

      if (run_length == num_qubits) {
        // Séquence trouvée ! On marque les bits comme occupés.
        for (int j = 0; j < num_qubits; j++) {
          set_bit(start_index + j);
        }
        qpu_state.free_qubits -= num_qubits;
        // Retourne l'index physique (simulé par un offset adresse)
        return (q_register_t)start_index;
      }
    } else {
      // Bit occupé, on reset la séquence
      run_length = 0;
      start_index = -1;
    }
  }

  printf(
      "[MEMORY] QPU Fragmentation Error. No contiguous block of %d qubits.\n",
      num_qubits);
  return -1;
}

/**
 * Libérateur Quantique (qfree)
 */
void qfree(q_register_t base_addr, uint16_t num_qubits) {
  int start_index = (int)base_addr;

  // Vérification de sécurité (Bounds Check)
  if (start_index < 0 || start_index + num_qubits > QPU_MAX_QUBITS) {
    printf("[MEMORY] SEGFAULT: Attempt to free invalid Qubit range.\n");
    return;
  }

  // Libération et Reset
  for (int i = 0; i < num_qubits; i++) {
    clear_bit(start_index + i);
    // TODO: Envoyer signal hardware "Active Reset" au driver QPU ici
  }

  qpu_state.free_qubits += num_qubits;
  printf("[MEMORY] Freed Qubits [%d - %d]\n", start_index,
         start_index + num_qubits - 1);
}

void kalloc_dump_stats(void) {
  printf("--- Memory Stats ---\n");
  printf("RAM Usage: %zu / %d bytes\n", heap_offset, KERNEL_HEAP_SIZE);
  printf("QPU Usage: %d / %d qubits free\n", qpu_state.free_qubits,
         QPU_MAX_QUBITS);
  printf("Bitmap: ");
  for (int i = 0; i < 16; i++)
    printf("%02X ", qpu_state.bitmap[i]);
  printf("\n--------------------\n");
}

void qpu_load_context(int pid, int active_reg) {
  // Simulation of Control Line Multiplexing
  // In a real Cryostat, this switches the DACs to the specific Qubit lines.
  if (active_reg >= 0) {
    printf("[HAL] QPU Context Switch -> PID %d (Active Reg: %d)\n", pid,
           active_reg);
    printf("[HAL] Calibrating Control Pulses for Qubits [%d-%d]...\n",
           active_reg, active_reg + 4); // Assuming 5 qubits per reg for demo
  }
}

/**
 * Get memory statistics
 */
void kalloc_get_stats(size_t *ram_used, size_t *ram_free, int *qubits_used,
                      int *qubits_free) {
  if (ram_used)
    *ram_used = total_allocated_ram;
  if (ram_free)
    *ram_free = total_free_ram;
  if (qubits_used)
    *qubits_used = QPU_MAX_QUBITS - qpu_state.free_qubits;
  if (qubits_free)
    *qubits_free = qpu_state.free_qubits;
}
