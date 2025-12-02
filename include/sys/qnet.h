#ifndef _SYS_QNET_H_
#define _SYS_QNET_H_

#include <stdint.h>
// #include "qproc.h" // This might be needed, but let's see where it is.
// The user script had #include "qproc.h".
// qproc.h is likely in kernel/core/include or similar?
// Metadata didn't show qproc.h location explicitly but qproc.c is in
// kernel/core. Let's assume for now we might need to fix this include too. But
// for the header file, I'll stick to what was given, but maybe comment out
// qproc.h if it causes issues, or find it. Actually, let's look for qproc.h
// first.

// Types de liens quantiques
typedef enum {
  LINK_LOCAL_EPR,    // Intrication directe (Fibre optique)
  LINK_VIRTUAL_SWAP, // Intrication routée (via Répéteur)
} link_type_t;

// Une Paire EPR (Ressource Réseau)
typedef struct {
  uint32_t pair_id;
  uint32_t remote_node_id;
  double fidelity;     // Doit être > 0.95 pour être utile
  uint32_t qubit_idx;  // L'index du qubit physique local (Changed from
                       // q_register_t to uint32_t to avoid dependency issues if
                       // qproc.h is missing)
  uint8_t is_purified; // 1 si passé par le protocole de distillation
} epr_pair_t;

// --- API Noyau ---

// Initialise la pile QNet
void qnet_init(void);

// Demande d'intrication (Le "Connect" quantique)
// Bloquant jusqu'à ce que la fidélité requise soit atteinte (via purification)
// epr_pair_t *qnet_request_entanglement(uint32_t target_node,
//                                       double min_fidelity);

// Protocole de Purification (Distillation)
// Sacrifie 'aux_pair' pour augmenter la fidélité de 'target_pair'
int qnet_purify(epr_pair_t *target_pair, epr_pair_t *aux_pair);

// Routing (Entanglement Swapping)
// Connecte deux paires (A-B et B-C) pour faire (A-C)
void qnet_perform_swap(epr_pair_t *pair_left, epr_pair_t *pair_right);

#endif
