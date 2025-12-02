/**/
/*
 * NexusQ-AI Kernel - Unified Memory Allocator Interface
 * File: include/sys/kalloc.h
 */

#ifndef _SYS_KALLOC_H_
#define _SYS_KALLOC_H_

#include "qproc.h" // Nécessaire pour q_register_t
#include <stddef.h>
#include <stdint.h>

// --- Constantes Physiques ---
#define KERNEL_HEAP_SIZE (1024 * 1024 * 16) // 16 MB pour le noyau (Simulation)
#define QPU_MAX_QUBITS 128 // Taille du registre quantique physique

// --- Structures de Gestion ---

// Bitmap pour suivre l'occupation des qubits physiques
// 128 qubits / 8 bits = 16 octets
typedef struct {
  uint8_t bitmap[QPU_MAX_QUBITS / 8];
  uint32_t free_qubits;
} qpu_map_t;

// --- API Publique ---

// 1. Initialisation du sous-système mémoire
void kalloc_init(void);

// 2. Allocation Classique (Wrapper kmalloc style BSD)
void *kmalloc(size_t size);
void kfree(void *ptr);

// 3. Allocation Quantique (Le "Moteur" NexusQ)
// Cherche 'n' qubits contigus (ou topologiquement liés) et retourne l'adresse
// de base.
q_register_t qmalloc(uint16_t num_qubits);

// Libère les qubits et force un RESET (Active Reset) pour vider l'énergie.
void qfree(q_register_t base_addr, uint16_t num_qubits);

// Debug / Inspection
void kalloc_dump_stats(void);
void kalloc_get_stats(size_t *ram_used, size_t *ram_free, int *qubits_used,
                      int *qubits_free);

// Context Switching
void qpu_load_context(int pid, int active_reg);

#endif // _SYS_KALLOC_H_