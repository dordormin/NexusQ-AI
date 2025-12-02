/*
 * NexusQ-AI Module 3 Test: LedgerFS & Integrity
 * Scenario: Rootkit Detection Simulation
 * Compile: gcc -I ./include tests/test_ledgerfs.c kernel/fs/ledgerfs.c
 * kernel/memory/kalloc.c -o test_lfs
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/kalloc.h>
#include <sys/ledgerfs.h>

int main() {
  printf("=== NexusQ-AI Module 3: LedgerFS Security Test ===\n\n");

  kalloc_init();
  lfs_init();

  // --- ÉTAPE 1 : DÉPLOIEMENT SYSTÈME ---
  printf("\n[SCENARIO] 1. Deploying Critical System Binary...\n");

  // Code binaire fictif pour "/bin/auth_daemon"
  const char *binary_code = "\xDE\xAD\xBE\xEF\x00\x01\x02\x03";

  // Fixed: Added "SYSTEM" as owner
  lfs_inode_t *sys_bin = lfs_create_file(
      "/bin/auth_daemon", (const uint8_t *)binary_code, 8, 0, "SYSTEM");
  assert(sys_bin != NULL);

  // --- ÉTAPE 2 : ANCRAGE BLOCKCHAIN ---
  printf("\n[SCENARIO] 2. Anchoring to Blockchain (Sealing)...\n");
  lfs_seal(sys_bin);

  // Vérification initiale : Doit être OK
  printf(" -> Initial Verification: ");
  if (lfs_verify_integrity(sys_bin) == 0)
    printf("OK\n");
  else {
    printf("FAIL\n");
    return 1;
  }

  // --- ÉTAPE 3 : TENTATIVE D'ÉCRITURE LÉGITIME (Doit échouer) ---
  printf("\n[SCENARIO] 3. Attempting overwrite via API...\n");
  const char *update = "\xFF\xFF\xFF\xFF";
  int status = lfs_write(sys_bin, (const uint8_t *)update, 4);

  if (status == -1) {
    printf("[PASS] LedgerFS correctly blocked write access to sealed file.\n");
  } else {
    printf("[FAIL] LedgerFS allowed write to sealed file!\n");
    return 1;
  }

  // --- ÉTAPE 4 : ATTAQUE ROOTKIT (Memory Corruption) ---
  printf(
      "\n[SCENARIO] 4. Simulating ROOTKIT ATTACK (Direct Memory Access)...\n");

  // Le hacker contourne l'API lfs_write et tape directement dans la RAM
  // C'est ce que font les Rootkits ou les attaques Rowhammer.
  // Fixed: data_ptr -> data
  printf(" -> HACKER: Injecting payload into memory address %p...\n",
         sys_bin->data);

  // Modification d'un seul octet (Bit Flip)
  // Fixed: data_ptr -> data, cast to uint8_t*
  ((uint8_t *)sys_bin->data)[0] = 0x66; // Était 0xDE

  printf(" -> HACKER: Injection Complete. Binary is now corrupted.\n");

  // --- ÉTAPE 5 : EXÉCUTION & DÉTECTION ---
  printf("\n[SCENARIO] 5. System attempts to execute binary...\n");

  // Le kernel appelle verify avant exec()
  int integrity = lfs_verify_integrity(sys_bin);

  if (integrity == -1) {
    printf("\n[SUCCESS] LedgerFS DETECTED the corruption!\n");
    printf("[SYSTEM] Execution Aborted. Security Alert Triggered.\n");
  } else {
    printf(
        "\n[FAIL] LedgerFS failed to detect the change. System compromised.\n");
    return 1;
  }

  printf("\n=== LedgerFS Validation Complete ===\n");
  return 0;
}
