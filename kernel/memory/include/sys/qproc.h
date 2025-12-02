#ifndef _SYS_QPROC_H_
#define _SYS_QPROC_H_

#include "../../../../include/sys/config.h"
#include <stdint.h>

typedef int32_t q_register_t;

// Quantum States
#define QSTATE_IDLE 0
#define QSTATE_RUNNING 1
#define QSTATE_DECOHERED 2
#define QSTATE_SUPERPOSED 3

// Process Structure
struct qproc {
  int pid;
  char name[32];
  int priority;

  // Quantum Info
  int num_qubits;
  int q_state; // État quantique (RUNNING, SUPERPOSITION, COLLAPSED)

  // Superposition Info (Level 34)
  int branch_id;   // Which branch of the multiverse is this?
  float amplitude; // Probability amplitude (0.0 - 1.0)

  // Resource Management (Deep Integration)
  // #define MAX_Q_REGS 8 // Defined in config.h
  q_register_t allocated_regs[MAX_Q_REGS]; // Handles to physical qubit blocks
  int reg_count;                           // Number of active allocations
  uint32_t quantum_pid;                    // Hardware QPU ID
  uint64_t cpu_ticks;                      // CPU Usage Counter
  int nice_value;          // Process Priority Adjustment (-20 to 19)
  q_register_t active_reg; // Currently addressed register (Context Switch)
  // Registers (Simplified Context)

  double t_coherence;     // Temps de cohérence restant (en us)
  float burst_prediction; // Prédiction de l'IA (0.0 - 1.0)

  // Identity & Security
  char owner_pubkey[64]; // Clé publique du propriétaire (Wallet)

  struct qproc *next; // Liste chaînée
};

// API
struct qproc *qproc_create(const char *name, int qubits);
void qproc_destroy(int pid);
void qproc_update_coherence(struct qproc *p, double delta_time);

#endif // _SYS_QPROC_H_
