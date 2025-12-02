#ifndef _SYS_QEC_H_
#define _SYS_QEC_H_

#include "qproc.h"
#include <stdint.h>

// Types d'erreurs détectables (Syndrome)
typedef enum {
  QERR_NONE = 0,
  QERR_BIT_FLIP_X,   // Erreur Pauli-X (0->1)
  QERR_PHASE_FLIP_Z, // Erreur Pauli-Z (+ -> -)
  QERR_DECOHERENCE_FATAL
} q_error_t;

// Codes de Correction Supportés
typedef enum {
  QEC_CODE_NONE = 0,
  QEC_CODE_REPETITION_3, // Le code "3-qubit bit flip" classique
  QEC_CODE_SHOR_9,       // Code de Shor (X et Z protection)
  QEC_CODE_SURFACE       // Code topologique (Futur)
} qec_algo_t;

// --- API Noyau ---

// Initialise le démon de correction d'erreurs
void qec_init(void);

// "Encode" un qubit logique à partir d'un qubit physique
// Alloue les qubits ancillaires nécessaires via kalloc
int qec_encode_logical(struct qproc *p, uint16_t logical_idx, qec_algo_t algo);

// Cycle de Syndrome (Heartbeat)
// Mesure les stabilisateurs et applique les corrections sans effondrer l'état
void qec_run_cycle(struct qproc *p);

// Métrologie (Sensing)
// Récupère le taux d'erreur estimé actuel (Fidélité)
double qec_get_fidelity_metric(void);

#endif
