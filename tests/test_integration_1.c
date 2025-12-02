// /*
//  * NexusQ-AI Integration Test 1: Process & Memory
//  * Compile: gcc tests/test_integration_1.c kernel/core/qproc.c kernel/memory/kalloc.c -I include -o os_test
//  */
//
// #include <stdio.h>
// #include <assert.h>
// #include "../include/sys/qproc.h"
// #include "../include/sys/kalloc.h"
//
// // On doit modifier qproc.c légèrement pour utiliser kalloc au lieu de malloc standard
// // Pour ce test, on suppose que qproc_create a été patché pour appeler kmalloc/qmalloc.
// // Voici le Patch Virtuel pour le test :
// // Dans qproc_create :
// //   remplacer malloc() par kmalloc()
// //   ajouter p->q_regs_ptr = qmalloc(qubits_needed);
//
// int main() {
//     printf("=== NexusQ-AI Integration Level 1 ===\n");
//
//     // 1. Boot du système mémoire
//     kalloc_init();
//
//     // 2. Allocation manuelle pour simuler ce que qproc fera
//     printf("\n[TEST] Simulating Kernel Process Allocation...\n");
//
//     // Allocation de la structure proc en RAM noyau
//     struct qproc* p_shor = (struct qproc*)kmalloc(sizeof(struct qproc));
//     assert(p_shor != NULL);
//
//     // Allocation des registres quantiques
//     uint16_t needed = 5;
//     q_register_t q_regs = qmalloc(needed);
//
//     if (q_regs != (q_register_t)-1) {
//         p_shor->pid = 100;
//         p_shor->q_regs_ptr = q_regs;
//         p_shor->num_qubits = needed;
//         printf("Process allocated at QPU Address: %ld\n", q_regs);
//     } else {
//         printf("Allocation Failed!\n");
//         return 1;
//     }
//
//     // 3. Vérification de l'état mémoire
//     kalloc_dump_stats();
//
//     // 4. Test de saturation
//     printf("\n[TEST] Stress Testing QPU...\n");
//     // On essaie d'allouer 120 qubits (il en reste 128-5 = 123, ça devrait passer)
//     q_register_t big_chunk = qmalloc(120);
//     assert(big_chunk != -1);
//
//     // On essaie d'en allouer 10 de plus (Impossible, QPU plein)
//     q_register_t fail_chunk = qmalloc(10);
//     assert(fail_chunk == -1);
//     printf("Saturation logic works: Allocation correctly rejected.\n");
//
//     // 5. Libération
//     qfree(q_regs, needed);
//     qfree(big_chunk, 120);
//
//     kalloc_dump_stats(); // Devrait être vide (FF partout ou 00 selon logique)
//
//     printf("\n=== Integration SUCCESS ===\n");
//     return 0;
// }
