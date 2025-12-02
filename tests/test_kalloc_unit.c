/*
 * NexusQ-AI Unit Test Suite: Hybrid Memory Manager
 * Target: kernel/memory/kalloc.c
 * Compile: gcc tests/test_kalloc_unit.c kernel/memory/kalloc.c -I include -o test_kalloc
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/sys/kalloc.h"

// Macros pour des tests lisibles
#define ASSERT_EQ(val, expected) if ((val) != (expected)) { printf(" [FAIL] Expected %ld, got %ld\n", (long)(expected), (long)(val)); return 1; }
#define TEST_PASS(name) printf("[PASS] %s\n", name)

int main() {
    printf("=== NexusQ-AI Module 1: Memory Manager Validation ===\n\n");

    // --- TEST 1: Initialisation ---
    kalloc_init();
    printf("[TEST] System Initialized.\n");
    kalloc_dump_stats();

    // --- TEST 2: RAM Allocation (Classique) ---
    printf("\n[TEST] 2. RAM Allocation Linear check...\n");
    void* ptr1 = kmalloc(1024); // 1KB
    void* ptr2 = kmalloc(2048); // 2KB
    
    // Vérification: l'allocateur linéaire doit avancer les adresses
    if (ptr2 > ptr1) {
        TEST_PASS("RAM pointers are monotonic (Heap grows up)");
    } else {
        printf("[FAIL] RAM Allocator logic error.\n");
        return 1;
    }

    // --- TEST 3: QPU Allocation (Quantique Simple) ---
    printf("\n[TEST] 3. QPU Simple Allocation...\n");
    // Allouer 5 qubits. Doit commencer à 0.
    q_register_t q1 = qmalloc(5);
    ASSERT_EQ(q1, 0); 
    TEST_PASS("Allocated 5 Qubits at index 0");

    // Allouer 10 qubits. Doit commencer à 5.
    q_register_t q2 = qmalloc(10);
    ASSERT_EQ(q2, 5); 
    TEST_PASS("Allocated 10 Qubits at index 5");

    // --- TEST 4: QPU Fragmentation & Reuse (Complexe) ---
    printf("\n[TEST] 4. QPU Fragmentation Handling...\n");
    
    // État actuel: [0-4 Occupé (q1)] [5-14 Occupé (q2)] [15... Libre]
    
    // On libère q1 (0-4). Le début devient libre.
    qfree(q1, 5);
    printf(" -> Freed q1 (0-4).\n");
    
    // On demande 3 qubits. L'allocateur doit être assez malin pour combler le trou [0-4]
    q_register_t q3 = qmalloc(3);
    ASSERT_EQ(q3, 0);
    TEST_PASS("Smart Allocator filled the hole at index 0");
    
    // On demande 100 qubits. Le trou restant (3-4) est trop petit (2 qubits).
    // Il doit sauter q2 (5-14) et aller chercher après (15...).
    q_register_t q4 = qmalloc(100);
    ASSERT_EQ(q4, 15);
    TEST_PASS("Allocator skipped occupied blocks correctly for large request");

    // --- TEST 5: Boundary & Error Handling ---
    printf("\n[TEST] 5. Stress & Bounds...\n");
    
    // Il reste très peu de place. Essayons d'allouer plus que disponible.
    // Total 128. Utilisé: 3 (q3) + 10 (q2) + 100 (q4) = 113. Reste ~15.
    q_register_t q_fail = qmalloc(50);
    ASSERT_EQ(q_fail, -1);
    TEST_PASS("OOM (Out of Qubits) correctly detected");

    // Double Free simulation (Libérer une zone invalide)
    // qfree ne crashe pas mais affiche une erreur console (test manuel visuel)
    printf(" -> Triggering Invalid Free (Check logs below)...\n");
    qfree(200, 5); // Index 200 est hors limites

    printf("\n=== All Memory Unit Tests Passed ===\n");
    kalloc_dump_stats();
    
    return 0;
}
