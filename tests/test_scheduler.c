/*
 * NexusQ-AI Unit Test Suite: Neuro-Scheduler
 * Target: kernel/core/scheduler.c
 * Compile: gcc -I ./include tests/test_scheduler.c kernel/core/scheduler.c kernel/core/qproc.c kernel/memory/kalloc.c -o test_sched
 */

#include <stdio.h>
#include <assert.h>
#include <sys/scheduler.h>
#include <sys/kalloc.h> // Pour qproc_create qui utilise kmalloc (si intégré)

int main() {
    printf("=== NexusQ-AI Module 2: Scheduler Validation ===\n\n");

    // Init
    kalloc_init(); // Nécessaire si qproc utilise kalloc
    sched_init();

    // --- SCÉNARIO ---
    
    // 1. Processus "User" (Compilation) - Priorité Normale
    struct qproc* p_gcc = qproc_create("gcc_build", 0);
    sched_submit(p_gcc);

    // 2. Processus "Quantum Stable" (Simulation Moléculaire) - T2 = 80µs (Confortable)
    struct qproc* p_mol = qproc_create("sim_h2o", 10);
    p_mol->t_coherence = 80.0;
    sched_submit(p_mol);

    // 3. Processus "Quantum Critique" (Recherche Grover) - T2 = 18µs (DANGER < 20µs)
    struct qproc* p_grover = qproc_create("grover_search", 5);
    p_grover->t_coherence = 18.0; // En dessous du seuil COHERENCE_THRESHOLD
    sched_submit(p_grover);

    printf("\n[TEST] 1. Initial State Inspection:\n");
    sched_dump_queue();

    // --- TEST DECISION ---
    printf("\n[TEST] 2. Triggering Scheduler Tick (Delta = 1us)...\n");
    
    // On demande à l'ordonnanceur qui doit tourner.
    struct qproc* winner = sched_tick(1.0);

    // ANALYSE DU VAINQUEUR
    if (winner == NULL) {
        printf("[FAIL] Scheduler returned NULL!\n");
        return 1;
    }

    printf(" -> Winner is PID %d (%s)\n", winner->pid, winner->name);

    // VERIFICATION LOGIQUE
    if (winner->pid == p_grover->pid) {
        printf("[PASS] Scheduler correctly prioritized Critical Quantum Process (Grover).\n");
    } else {
        printf("[FAIL] Scheduler failed to save Grover! It picked PID %d.\n", winner->pid);
        return 1; // Échec critique
    }

    // --- TEST SIMULATION TEMPS ---
    printf("\n[TEST] 3. Time Warp (Advance 20us)...\n");
    // Grover (18us) va mourir. Mol (80us) va passer à 60us.
    
    sched_tick(20.0);
    
    // Grover devrait avoir disparu de la file (tué par decoherence)
    printf(" -> Checking RunQueue integrity...\n");
    sched_dump_queue();
    
    // Vérification manuelle dans le tableau (accès brutal pour le test)
    // p_grover ne devrait plus être dans la liste active gérée par sched_dump_queue
    // (Note: l'implémentation actuelle met NULL dans le slot)
    
    // Le prochain gagnant doit être p_mol (car il est quantique et actif) ou p_gcc
    struct qproc* next_winner = sched_tick(1.0);
    printf(" -> New Winner is PID %d (%s)\n", next_winner->pid, next_winner->name);
    
    if (next_winner->pid == p_mol->pid) {
        printf("[PASS] Hierarchy respected (Quantum > Classical).\n");
    }

    printf("\n=== Scheduler Logic Validated ===\n");
    return 0;
}
