#include "../../../include/sys/qnet.h"
#include "../../memory/include/sys/kalloc.h"
#include <stdio.h>
#include <stdlib.h>

// Simulation : Augmente la fidélité selon la formule de Bennett
// F_new = (F^2 + (1-F)^2 / 9) / Normalisation ... (Simplifié pour le kernel)
static double simulate_distillation_math(double f1, double f2) {
  if (f1 != f2)
    return f1; // Simplification: paires identiques requises
  double num = f1 * f1 + ((1.0 - f1) / 3.0) * ((1.0 - f1) / 3.0);
  double den = num + (2.0 * f1 * (1.0 - f1) / 3.0) +
               (5.0 * ((1.0 - f1) / 3.0) * ((1.0 - f1) / 3.0));
  return num / den;
}

int qnet_purify(epr_pair_t *target, epr_pair_t *aux) {
  printf("[QNET] PURIFICATION: Sacrificing Pair #%d (F=%.3f) to boost Pair #%d "
         "(F=%.3f)\n",
         aux->pair_id, aux->fidelity, target->pair_id, target->fidelity);

  // 1. CNOT Gate (Local) : Target -> Aux
  // Dans un vrai QPU: q_driver_cnot(target->qubit_idx, aux->qubit_idx);

  // 2. Mesure de l'Auxiliaire
  // Dans BBPSSW, si les résultats de mesure coincident, on garde la paire.
  // Simulation : On assume succès probabiliste pour le test.

  double new_fid = simulate_distillation_math(target->fidelity, aux->fidelity);

  if (new_fid > target->fidelity) {
    printf("[QNET] SUCCESS: Fidelity increased %.3f -> %.3f\n",
           target->fidelity, new_fid);
    target->fidelity = new_fid;
    target->is_purified = 1;

    // Libération de la ressource sacrifiée (Module Kalloc)
    // qfree(aux->qubit_idx, 1);
    // free(aux);
    return 0;
  } else {
    printf("[QNET] FAILURE: Distillation failed. Discarding both.\n");
    return -1;
  }
}
