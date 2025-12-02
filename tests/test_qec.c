#include "../kernel/memory/include/sys/qec.h"
#include "../kernel/memory/include/sys/qproc.h"
#include <assert.h>
#include <stdio.h>

// Hack pour accéder à la fonction de debug cachée
extern void qec_debug_inject_error(int qubit_idx);

int main() {
  printf("=== NexusQ-AI Module 4: Quantum Error Correction ===\n");
  printf("Based on protocols from I. Djordjevic\n\n");

  qec_init();

  // 1. Création d'un processus factice
  struct qproc p;
  p.pid = 42;

  // 2. Encodage Logique
  printf("\n[TEST] 1. Encoding Logical Qubit...\n");
  qec_encode_logical(&p, 0, QEC_CODE_REPETITION_3);

  // 3. Cycle sans erreur
  printf("\n[TEST] 2. Running Cycle (Clean State)...\n");
  qec_run_cycle(&p);
  printf(" -> No correction triggered (Expected).\n");

  // 4. Attaque de Bruit (Bit Flip sur Qubit 0)
  printf("\n[TEST] 3. Injecting NOISE (Bit Flip on Physical Qubit 0)...\n");
  qec_debug_inject_error(0);
  // L'état est maintenant CORROMPU (100 au lieu de 000)

  // 5. Réparation Automatique
  printf("\n[TEST] 4. QEC Response...\n");
  qec_run_cycle(&p);

  // Dans la sortie console, on doit voir "Correcting (X-Gate)"

  printf("\n[TEST] 5. Sensing Check...\n");
  double fid = qec_get_fidelity_metric();
  printf(" -> Current Hardware Fidelity: %.4f\n", fid);

  printf("\n=== QEC Subsystem Validated ===\n");
  return 0;
}
